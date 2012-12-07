#include <QtGui/QApplication>
#include "AVInfo.hpp"
#include "VideoPlayer.hpp"
#include "debug.hpp"
#include "MultimediaWidget.hpp"
#include "SoundTouch.h"
#include "STTypes.h"

using namespace soundtouch;

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
    // MultimediaWidget w;
    w.show();
    return a.exec();
}
