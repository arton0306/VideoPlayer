#include <cassert>
#include <QtGui/QTextEdit>
#include <QtGui/QMessageBox>
#include <QDir>
#include <QTime>
#include "debug.hpp"

namespace debug
{
    static QTime * programTimer = NULL;
    static QTextEdit * textEdit = NULL;

    void setProgramTimer()
    {
        programTimer = new QTime( 0, 0, 0 );  // it last to end and we won't delete it for laziness
        programTimer->start();
    }

    void initDebugWindow()
    {
        textEdit = new QTextEdit();           // it last to end and we won't delete it for laziness
        textEdit->setWindowTitle("Debug window");
        textEdit->resize( 1200, 800 );
        textEdit->show();
    }

    QString getElapseTime()
    {
        int elapsedMSec = programTimer->elapsed();
        return QTime( 0, 0, 0 ).addMSecs( elapsedMSec ).toString( "hh:mm:ss.zzz" );
    }

    void debugWinMsgHandler(QtMsgType type, const char *msg)
    {
        assert( programTimer != NULL );
        assert( textEdit != NULL );
        switch (type)
        {
            case QtDebugMsg:
                textEdit->append(QString("\%1 <b>Debug:</b> \%2").arg(getElapseTime()).arg(msg));
                break;
            case QtWarningMsg:
                textEdit->append(QString("\%1 <b>Warning:</b> \%2").arg(getElapseTime()).arg(msg));
                break;
            case QtCriticalMsg:
                textEdit->append(QString("\%1 <font color=\"red\"><b>Critical:</b></font> \%2").arg(getElapseTime()).arg(msg));
                break;
            case QtFatalMsg:
                QMessageBox::critical(0, "Debug - Fatal", msg);
                abort();
        }
    }
}