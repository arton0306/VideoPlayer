#include <QtGui/QApplication>
#include <QThread>
#include "VideoPlayer.hpp"
#include "LibavWorker.hpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VideoPlayer w;
    w.show();

    QThread * libavThread = new QThread;
    LibavWorker * libavWorker = new LibavWorker;
    libavWorker->moveToThread( libavThread );
    libavThread->start();
    QMetaObject::invokeMethod( libavWorker, "doWork", Qt::QueuedConnection );

    return a.exec();
}
