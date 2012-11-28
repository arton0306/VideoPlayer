#include <QThread>
#include <cassert>
#include "MultimediaWidget.hpp"
#include "debug.hpp"
#include "LibavWorker.hpp"
#include "QGLCanvas.hpp"

using namespace std;

MultimediaWidget::MultimediaWidget(QWidget *parent)
    : QWidget(parent)
    , mLibavWorker( new LibavWorker )
    , mVideoCanvas( NULL )
    , mAudioOutput( NULL )
    , mOutputDevice( NULL )
    , mAdjustMs( 0 )
    , mIsDecodeDone( false )
{
    setupUi(this);
    setupConnection();

    mOutsideTime.start();

    mVideoCanvas = new QGLCanvas( this );
    mMainLayout->addWidget( mVideoCanvas );

    QThread * libavThread = new QThread;
    mLibavWorker->moveToThread( libavThread );
    libavThread->start();
    play( QString( "video/Lelouch.mp4" ) );
}

void MultimediaWidget::setupConnection()
{
    connect( mLibavWorker, SIGNAL(ready( AVInfo )), this, SLOT(getDecodeReadySignal( AVInfo )) );
    connect( mLibavWorker, SIGNAL(decodeDone()), this, SLOT(getDecodeDoneSig()) );
    connect( &mTimer, SIGNAL(timeout()), this, SLOT(renew()) );
}

void MultimediaWidget::getDecodeReadySignal( AVInfo aAvInfo )
{
    mCurrentAvInfo = aAvInfo;
    mCurrentAvInfo.dump();
    QAudioFormat format = getAudioFormat( aAvInfo );

    if ( !QAudioDeviceInfo::defaultOutputDevice().isFormatSupported( format ) )
    {
        failAvFormat();
        assert( false );
    }
    else
    {
        delete mAudioOutput;
        mAudioOutput = new QAudioOutput( format ); // the Qt example has 2nd augument - this
        mOutputDevice = mAudioOutput->start();
        mTimer.start( getRenewPeriod( aAvInfo.getFps() ) );
    }
}

// the period is a function of fps
double MultimediaWidget::getRenewPeriod( double a_fps ) const
{
    // return 250.0 / a_fps;
    return 5;
}

double MultimediaWidget::getAudioPlayedSecond() const
{
    assert( mAudioOutput->state() == QAudio::ActiveState || mAudioOutput->state() == QAudio::IdleState );
    // the played-time got by Qt lib has about 40ms error
    // so we use a outside clock to sync audio&video, but we adjust the outside clock to match the played-time(by Qt) if the gap become too large
    qint64 const bytesInBuffer = mAudioOutput->bufferSize() - mAudioOutput->bytesFree();
    qint64 const usInBuffer = (qint64)(1000000) * bytesInBuffer / ( mCurrentAvInfo.getAudioChannel() * mCurrentAvInfo.getAudioBitsPerSample() / 8 ) / mCurrentAvInfo.getAudioSampleRate();
    qint64 const usPlayed = mAudioOutput->processedUSecs() - usInBuffer;

    // adjust the outside clock if needed
    int const msQtPlay = static_cast<double>( usPlayed ) / 1000.0;
    int const msSyncTime = mOutsideTime.elapsed() + mAdjustMs;

    if ( msQtPlay - msSyncTime > 100.0 )
    {
        mAdjustMs += msQtPlay - msSyncTime;
    }
    else if ( msSyncTime - msQtPlay > 100.0 )
    {
        if ( msQtPlay > 0 )
        {
            mAdjustMs -= msSyncTime - msQtPlay;
        }
        else // msQtPlay maybe negative (by Qt implement)
        {
            return 0;
        }
    }

    return ( mOutsideTime.elapsed() + mAdjustMs ) / 1000.0;
}

void MultimediaWidget::renew()
{
    // fetch all the decoded audio data and push into audio device if the buffer is empty
    if ( mAudioStreamBuffer.empty() )
    {
        vector<uint8> audioStreamFetched = mLibavWorker->popAllAudioStream();
        qint64 const writeBytes = mOutputDevice->write( reinterpret_cast<const char *>( &audioStreamFetched[0] ), audioStreamFetched.size() );
        mAudioStreamBuffer.insert( mAudioStreamBuffer.end(), audioStreamFetched.begin() + writeBytes, audioStreamFetched.end() );
    }
    else
    {
        qint64 const writeBytes = mOutputDevice->write( reinterpret_cast<const char *>( &mAudioStreamBuffer[0] ), mAudioStreamBuffer.size() );
        mAudioStreamBuffer.erase( mAudioStreamBuffer.begin(), mAudioStreamBuffer.begin() + writeBytes );
    }

    // get the time of audio played
    double const currentPlaySecond = getAudioPlayedSecond();

    // get the time of the next video frame in decoded buffer
    double const nextVideoFrameSecond = mLibavWorker->getNextVideoFrameSecond();
    if ( nextVideoFrameSecond != 0.0 )
    {
        // renew video frame if needed
        double const diff = currentPlaySecond - nextVideoFrameSecond;
        double const absdiff = ( diff > 0 ? diff : -diff );

        // if ( absdiff < 0.55 * 1.0 / mCurrentAvInfo.getFps() )
        if ( absdiff < 0.01 )
        {
            vector<uint8> videoFrameStream = mLibavWorker->popNextVideoFrame();
            // DEBUG() << "frame should be presented:" << nextVideoFrameSecond << "\t currentSount:" << currentPlaySecond;
            mVideoCanvas->renewFrame( static_cast<uint8_t const *>( &videoFrameStream[0] ), videoFrameStream.size() );
        }
        else if ( diff > 0 )
        {
            // video frame too old
            // DEBUG() << "drop a frame which should be presented at:" << nextVideoFrameSecond << "\t currentSount:" << currentPlaySecond;
            mLibavWorker->dropNextVideoFrame();
        }
    }
    else // there are no frame got
    {
        if ( mIsDecodeDone )
        {
            stop();
        }
    }
}

QAudioFormat MultimediaWidget::getAudioFormat( AVInfo const & aAvInfo ) const
{
    QAudioFormat audioFormat;
    audioFormat.setFrequency( aAvInfo.getAudioSampleRate() );
    audioFormat.setChannels( aAvInfo.getAudioChannel() );
    audioFormat.setSampleSize( aAvInfo.getAudioBitsPerSample() );
    audioFormat.setCodec( "audio/pcm" );
    audioFormat.setByteOrder(QAudioFormat::LittleEndian);
    audioFormat.setSampleType(QAudioFormat::UnSignedInt);

    return audioFormat;
}

MultimediaWidget::~MultimediaWidget()
{
    delete mAudioOutput;
}

void MultimediaWidget::play( QString aFileName )
{
    stop();
    QMetaObject::invokeMethod( mLibavWorker, "decodeAudioVideo", Qt::QueuedConnection,
        Q_ARG(QString, aFileName) );
}

void MultimediaWidget::seek( int aMSec )
{
}

void MultimediaWidget::stop()
{
    mTimer.stop();
    mVideoCanvas->clear();
    mAudioStreamBuffer.clear();
    mLibavWorker->stopDecoding();
}

void MultimediaWidget::getDecodeDoneSignal()
{
    mIsDecodeDone = true;
}

