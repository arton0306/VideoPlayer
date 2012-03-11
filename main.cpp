#include <QtGui/QApplication>
#include "VideoPlayer.hpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VideoPlayer w;
    w.show();

    return a.exec();
}
