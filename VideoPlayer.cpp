#include <QThread>
#include <cassert>
#include "LibavWorker.hpp"
#include "VideoPlayer.hpp"
#include "QGLCanvas.hpp"

VideoPlayer::VideoPlayer(QWidget *parent)
    : QMainWindow(parent)
    , videoCanvas( NULL )
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
    // connect( libavWorker, SIGNAL(frameReady( uint8_t const *, int )),
    //          videoCanvas,   SLOT(renewFrame( uint8_t const *, int )) );

    connect( libavWorker, SIGNAL(    ready( AVInfo )),
             this       ,   SLOT(startPlay( AVInfo )) );
}

void VideoPlayer::startPlay( AVInfo aAvInfo )
{
    mCurrentAvInfo = aAvInfo;
    QAudioFormat format = getAudioFormat( aAVInfo );

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
}
