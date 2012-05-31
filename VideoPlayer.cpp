#include <QThread>
#include <cassert>
#include "LibavWorker.hpp"
#include "VideoPlayer.hpp"
#include "QGLCanvas.hpp"

using namespace std;

VideoPlayer::VideoPlayer(QWidget *parent)
    : QMainWindow(parent)
    , mLibavWorker( new LibavWorker )
    , videoCanvas( NULL )
    , mAudioOutput( NULL )
    , mOutputDevice( NULL )
{
    setupUi(this);
    setupConnection();

    videoCanvas = new QGLCanvas( this );
    mVideoLayout->addWidget( videoCanvas );

    QThread * libavThread = new QThread;
    mLibavWorker->moveToThread( libavThread );
    libavThread->start();
    QMetaObject::invokeMethod( mLibavWorker, "decodeAudioVideo", Qt::QueuedConnection,
        Q_ARG(QString, "video/Lelouch.mp4") );
}

void VideoPlayer::setupConnection()
{
    connect( mLibavWorker, SIGNAL(ready( AVInfo )), this, SLOT(startPlay( AVInfo )) );
    connect( &mTimer, SIGNAL(timeout()), this, SLOT(fetchAndPlay()) );
}

void VideoPlayer::startPlay( AVInfo aAvInfo )
{
    mCurrentAvInfo = aAvInfo;
    QAudioFormat format = getAudioFormat( aAvInfo );

    if ( !QAudioDeviceInfo::defaultOutputDevice().isFormatSupported( format ) )
    {
        failAvFormat();
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
double VideoPlayer::getRenewPeriod( double a_fps ) const
{
    return 0.1 * 1.0 / a_fps;
}

double VideoPlayer::getAudioPlayedSecond() const
{
    assert( mAudioOutput->state() == QAudio::ActiveState || mAudioOutput->state() == QAudio::IdleState );
    qint64 bytesInBuffer = mAudioOutput->bufferSize() - mAudioOutput->bytesFree();
    qint64 usInBuffer = (qint64)(1000000) * bytesInBuffer / ( mCurrentAvInfo.getAudioChannel() * mCurrentAvInfo.getAudioBitsPerSample() / 8 ) / mCurrentAvInfo.getAudioSampleRate();
    qint64 usPlayed = mAudioOutput->processedUSecs() - usInBuffer;

    return static_cast<double>( usPlayed ) / 1000000.0;
}

void VideoPlayer::fetchAndPlay()
{
    // fetch all the decoded audio data and push into audio device
    vector<uint8> audioStream = mLibavWorker->popAllAudioStream();
    if ( !audioStream.empty() )
    {
        qint64 writeBytes = mOutputDevice->write( reinterpret_cast<const char *>( &audioStream[0] ), audioStream.size() );
        assert( writeBytes == audioStream.size() );
    }

    // get the time of audio played
    double const currentPlaySecond = getAudioPlayedSecond();

    // get the time of the next video frame in decoded buffer
    double const nextVideoFrameSecond = mLibavWorker->getNextVideoFrameSecond();

    // renew video frame if needed
    double const diff = currentPlaySecond - nextVideoFrameSecond;
    if ( diff < 1.5 * mCurrentAvInfo.getFps() )
    {
        // video frame too old
        mLibavWorker->dropNextVideoFrame();
    }
    else if ( diff < getRenewPeriod( mCurrentAvInfo.getFps() ) )
    {
        vector<uint8> videoFrameStream = mLibavWorker->popNextVideoFrame();
        videoCanvas->renewFrame( static_cast<uint8_t const *>( &videoFrameStream[0] ), videoFrameStream.size() );
    }
}

QAudioFormat VideoPlayer::getAudioFormat( AVInfo const & aAvInfo ) const
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

VideoPlayer::~VideoPlayer()
{
    delete mAudioOutput;
}
