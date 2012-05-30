#include <QThread>
#include <cassert>
#include "LibavWorker.hpp"
#include "VideoPlayer.hpp"
#include "QGLCanvas.hpp"

VideoPlayer::VideoPlayer(QWidget *parent)
    : QMainWindow(parent)
    , videoCanvas( NULL )
    , mAudioOutput( NULL )
    , mOutputDevice( NULL )
{
    setupUi(this);

    videoCanvas = new QGLCanvas( this );
    mVideoLayout->addWidget( videoCanvas );

    QThread * libavThread = new QThread;
    LibavWorker * libavWorker = new LibavWorker;
    libavWorker->moveToThread( libavThread );
    libavThread->start();
    QMetaObject::invokeMethod( libavWorker, "decodeAudioVideo", Qt::QueuedConnection,
        Q_ARG(QString, "video/Lelouch.mp4") );

}

void VideoPlayer::setupConnection()
{
    connect( libavWorker, SIGNAL(ready( AVInfo )), this, SLOT(startPlay( AVInfo )) );
    connect( timer, SIGNAL(timeout()), this, SLOT(fetchDecodedAvData()));
}

void VideoPlayer::startPlay( AVInfo aAvInfo )
{
    mCurrentAvInfo = aAvInfo;
    QAudioFormat format = getAudioFormat( aAVInfo );

    if ( !QAudioDeviceInfo::defaultOutputDevice().isFormatSupported( format ) )
    {
        failAvFormat();
    }
    else
    {
        delete mAudioOutput;
        mAudioOuput = new QAudioOutput( format ); // the Qt example has 2nd augument - this
        mOutputDevice = mAudioOutput.start();
        timer.start( 1.0 / aAvInfo.getFps() * 0.1 );
    }
}

void VideoPlayer::fetchDecodedAvData()
{
    
}

QAudioFormat VideoPlayer::getAudioFormat( AVInfo const & aAvInfo )
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
    delete mAudioOuput;
}
