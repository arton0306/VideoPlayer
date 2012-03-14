#include <QtGui/QApplication>
#include "VideoPlayer.hpp"
#include "debug.hpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    debug::setProgramTimer();
    debug::initDebugWindow();
    #ifdef Q_WS_WIN
        qInstallMsgHandler( debug::debugWinMsgHandler );
    #endif

    VideoPlayer w;
    w.show();

    return a.exec();
}
