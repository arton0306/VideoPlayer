#include <QThread>
#include "LibavWorker.hpp"
#include "VideoPlayer.hpp"
#include "QGLCanvas.hpp"

#include "PacketQueue.hpp"

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
    QMetaObject::invokeMethod( libavWorker, "doWork", Qt::QueuedConnection );

    connect( libavWorker,
        SIGNAL(frameReady( uint8_t const *, int )),
        videoCanvas,
        SLOT(renewFrame( uint8_t const *, int )) );

    PacketQueue test;
}

VideoPlayer::~VideoPlayer()
{
}
