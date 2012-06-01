#include <QtGui/QApplication>
#include "AVInfo.hpp"
#include "VideoPlayer.hpp"
#include "debug.hpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    debug::setProgramTimer();
    // debug::initDebugWindow();
    debug::initOutputFile();
    #ifdef Q_WS_WIN
        qInstallMsgHandler( debug::debugWinMsgHandler );
    #endif

    qRegisterMetaType<AVInfo>("AVInfo");

    VideoPlayer w;
    w.show();

    return a.exec();
}
