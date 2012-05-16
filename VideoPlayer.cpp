#include <QThread>
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
    QMetaObject::invokeMethod( libavWorker, "decodeAudioVideo", Qt::QueuedConnection );

    connect( libavWorker,
        SIGNAL(frameReady( uint8_t const *, int )),
        videoCanvas,
        SLOT(renewFrame( uint8_t const *, int )) );
}

VideoPlayer::~VideoPlayer()
{
}
