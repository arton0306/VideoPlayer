#include <cmath> // for abs, there are a int version abs in cstdlib, we must include cmath to overwrite it
#include <QThread>
#include <cassert>
#include "MultimediaWidget.hpp"
#include "debug.hpp"
#include "LibavWorker.hpp"
#include "QGLCanvas.hpp"

using namespace std;

static int debugTimer = 0;
MultimediaWidget::MultimediaWidget(QWidget *parent)
    : QWidget(parent)
    , mLibavWorker( new LibavWorker )
    , mVideoCanvas( NULL )
    , mAudioOutput( NULL )
    , mOutputDevice( NULL )
    , mAdjustMs( 0 )
    , mAudioSeekTimeMSec( 0.0 )
    , mIsDecodeDone( false )
{
    setupUi(this);
    setupConnection();

    AudioPlayer::init();

    mOutsideTime.start();

    mVideoCanvas = new QGLCanvas( this );
    mMainLayout->addWidget( mVideoCanvas );

    QThread * libavThread = new QThread;
    mLibavWorker->moveToThread( libavThread );
    libavThread->start();
    play( QString( "video/outof1000kilometer.mpg" ) );
}

void MultimediaWidget::setupConnection()
{
    connect( mLibavWorker, SIGNAL(readyToDecode( AVInfo )), this, SLOT(getReadyToDecodeSignal( AVInfo )) );
    connect( mLibavWorker, SIGNAL(decodeDone()), this, SLOT(getDecodeDoneSignal()) );
    connect( mLibavWorker, SIGNAL(seekState(bool)), this, SLOT(getSeekStateSignal(bool)) );
    connect( mLibavWorker, SIGNAL(initAVFrameReady(double)), this, SLOT(getInitAVFrameReadySignal(double)) );
    connect( &mTimer, SIGNAL(timeout()), this, SLOT(updateAV()) );
}

void MultimediaWidget::getReadyToDecodeSignal( AVInfo aAvInfo )
{
    mCurrentAvInfo = aAvInfo;
    mCurrentAvInfo.dump();

    /*
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
        mAudioSeekTimeMSec = 0;
        mTimer.start( getRenewPeriod( mCurrentAvInfo.getFps() ) );
    }
    */

    mAudioSeekTimeMSec = 0;
    //mTimer.start( getRenewPeriod( aAvInfo.getFps() ) );
    // delete mAudioPlayer; // TODO: too early to delete it ~AudioPlayer will casse crash
    mAudioPlayer = new AudioPlayer( aAvInfo.getAudioChannel(), aAvInfo.getAudioSampleFormat(), aAvInfo.getAudioSampleRate() );
    //mTimer.start( getRenewPeriod() );
}

void MultimediaWidget::getSeekStateSignal(bool aIsSuccess)
{
    DEBUG() << " =========================== seek result: " << aIsSuccess;
    if ( aIsSuccess ) // true is success
    {
        mTimer.stop();
        mAudioStreamBuffer.clear();
        //mAudioOutput->stop();
    }
}

void MultimediaWidget::getInitAVFrameReadySignal(double aFirstAudioFrameMsec)
{
    // we don't need to delete mOutputDevice by try&error, TODO survey Qt source code
    /*
    mOutputDevice = mAudioOutput->start();
    mOutsideTime.restart();
    mAdjustMs = 0;
    debugTimer = 0;
    mAudioSeekTimeMSec = aFirstAudioFrameMsec;
    DEBUG() << "libav seek from: " << aFirstAudioFrameMsec;
    mTimer.start( getRenewPeriod( mCurrentAvInfo.getFps() ) );
    */
    mOutsideTime.restart();
    mAdjustMs = 0;
    debugTimer = 0;
    mAudioSeekTimeMSec = aFirstAudioFrameMsec;
    DEBUG() << "libav seek from (ms): " << aFirstAudioFrameMsec;
    fetchAllAvailableAudioAndPush();
    mAudioPlayer->play();
    mTimer.start( getRenewPeriod() );
}

// the period is a function of fps
double MultimediaWidget::getRenewPeriod( double a_fps ) const
{
    // return 250.0 / a_fps;
    return 5;
}

double MultimediaWidget::getRenewPeriod() const
{
    return 1;
}

double const MultimediaWidget::sAudioAdjustGapMs = 150.0;
double MultimediaWidget::getAudioPlayedSecond() const
{
    /*
    assert( mAudioOutput->state() == QAudio::ActiveState || mAudioOutput->state() == QAudio::IdleState );
    // the played-time got by Qt lib has about 40ms error
    // so we use a outside clock to sync audio&video, but we adjust the outside clock to match the played-time(by Qt) if the gap become too large ( >sAudioAdjustGapMs )
    qint64 const bytesInBuffer = mAudioOutput->bufferSize() - mAudioOutput->bytesFree();
    qint64 const usInBuffer = (qint64)(1000000) * bytesInBuffer / ( mCurrentAvInfo.getAudioChannel() * mCurrentAvInfo.getAudioBitsPerSample() / 8 ) / mCurrentAvInfo.getAudioSampleRate();
    qint64 const usPlayed = mAudioOutput->processedUSecs() - usInBuffer;

    // adjust the outside clock if needed
    double const msQtPlay = mAudioSeekTimeMSec + static_cast<double>( usPlayed ) / 1000.0;
    double const msSyncTime = mAudioSeekTimeMSec + mOutsideTime.elapsed() + mAdjustMs;

    if ( std::abs( msQtPlay - msSyncTime ) > MultimediaWidget::sAudioAdjustGapMs )
    {
        mAdjustMs += msQtPlay - msSyncTime;
        DEBUG() << "qt_time: " << msQtPlay << "  sync_time: " << msSyncTime;
    }

    // msQtPlay maybe negative (by Qt implement)
    return ( mAudioSeekTimeMSec + mOutsideTime.elapsed() + mAdjustMs ) / 1000.0;
    */
    double const msPortaudioTime = mAudioSeekTimeMSec + mAudioPlayer->getPlaySec() * 1000;
    double const msSyncTime = mAudioSeekTimeMSec + mOutsideTime.elapsed() + mAdjustMs;

    if ( std::abs( msPortaudioTime - msSyncTime ) > MultimediaWidget::sAudioAdjustGapMs )
    {
        mAdjustMs += msPortaudioTime - msSyncTime;
        DEBUG() << "qt_time: " << msPortaudioTime << "  sync_time: " << msSyncTime;
    }

    return ( mAudioSeekTimeMSec + mOutsideTime.elapsed() + mAdjustMs ) / 1000.0;
}

void MultimediaWidget::fetchAllAvailableAudioAndPush()
{
    vector<uint8> audioStreamFetched = mLibavWorker->popAllAudioStream();
    int const writeBytes = mAudioPlayer->pushStream( reinterpret_cast<const char *>( &audioStreamFetched[0] ), audioStreamFetched.size() );
    mAudioStreamBuffer.insert( mAudioStreamBuffer.end(), audioStreamFetched.begin() + writeBytes, audioStreamFetched.end() );
}

void MultimediaWidget::updateAV()
{
    // deubg
    if ( debugTimer == 0 )
    {
        debugTimer++;
        DEBUG() << "**************** first updateAV ************************************************************************";
        // DEBUG() << "audio processed usecs:" << mAudioOutput->processedUSecs() << " outside time elapsed:" << mOutsideTime.elapsed();
    }

    if ( mAudioStreamBuffer.empty() )
    {
        fetchAllAvailableAudioAndPush();
    }
    else
    {
        int const writeBytes = mAudioPlayer->pushStream( reinterpret_cast<const char *>( &mAudioStreamBuffer[0] ), mAudioStreamBuffer.size() );
        mAudioStreamBuffer.erase( mAudioStreamBuffer.begin(), mAudioStreamBuffer.begin() + writeBytes );
    }

    // get the time of audio played
    double const currentPlaySecond = getAudioPlayedSecond();

    // get the time of the next video frame in decoded buffer
    double const nextVideoFrameSecond = mLibavWorker->getNextVideoFrameSecond();
    if ( nextVideoFrameSecond >= 0.0 )
    {
        // updateAV video frame if needed
        double const diff = currentPlaySecond - nextVideoFrameSecond;

        if ( std::abs(diff) < 0.01 )
        {
            vector<uint8> videoFrameStream = mLibavWorker->popNextVideoFrame();
            DEBUG() << "frame be drawed on the canvas, its exact time is:" << nextVideoFrameSecond << "\t currentSound:" << currentPlaySecond << "abs_diff: " << abs(diff);
            mVideoCanvas->renewFrame( static_cast<uint8_t const *>( &videoFrameStream[0] ), videoFrameStream.size() );
        }
        else if ( diff > 0.01 )
        {
            // video frame too old
            DEBUG() << "drop a frame which should be presented at:" << nextVideoFrameSecond << "\t currentSound:" << currentPlaySecond;
            mLibavWorker->dropNextVideoFrame();
        }

        // DEBUG() << "next video frame:" << nextVideoFrameSecond << "\t currentSound:" << currentPlaySecond;
    }
    else // there are no frame got
    {
        DEBUG() << "there are no video frame";
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
    AudioPlayer::finish();
    // delete mAudioOutput;
}

void MultimediaWidget::play( QString aFileName )
{
    stop();
    QMetaObject::invokeMethod( mLibavWorker, "decodeAudioVideo", Qt::QueuedConnection,
        Q_ARG(QString, aFileName) );
}

void MultimediaWidget::seek( int aMSec )
{
    DEBUG() << "someone click seek";
    mLibavWorker->seek( aMSec );
}

void MultimediaWidget::stop()
{
    mTimer.stop();
    mVideoCanvas->clear();
    mAudioStreamBuffer.clear();
    mLibavWorker->stopDecoding();
    mAudioSeekTimeMSec = 0.0;
}

void MultimediaWidget::dumpAVStream()
{
    
}

void MultimediaWidget::getDecodeDoneSignal()
{
    mIsDecodeDone = true;
}

