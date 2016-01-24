#include <cassert>
//#include <QtGui/QTextEdit>
//#include <QtGui/QMessageBox>
#include <QTextEdit>
#include <QMessageBox>
#include <QDir>
#include <QTime>
#include "debug.hpp"

namespace debug
{
    static QTime * programTimer = NULL;
    static QTextEdit * textEdit = NULL;
    static char oneLineDebugMsgBuffer[1024];
    static FILE * file = NULL;

    void setProgramTimer()
    {
        programTimer = new QTime( 0, 0, 0 );  // it last to end and we won't delete it for laziness
        programTimer->start();
    }

    // debug msg output to a text edit, use in single thread
    void initDebugWindow()
    {
        textEdit = new QTextEdit();           // it last to end and we won't delete it for laziness
        textEdit->setWindowTitle("Debug window");
        textEdit->resize( 1200, 800 );
        textEdit->show();
    }

    // debug msg output to a file
    void initOutputFile()
    {
        file = fopen( "debuglog.txt", "w" );
    }

    QString getElapseTime()
    {
        int elapsedMSec = programTimer->elapsed();
        return QTime( 0, 0, 0 ).addMSecs( elapsedMSec ).toString( "hh:mm:ss.zzz" );
    }

    void debugWinMsgHandler(QtMsgType type, const char *msg)
    {
        assert( programTimer != NULL );
        switch (type)
        {
            case QtDebugMsg:
                sprintf( oneLineDebugMsgBuffer, "%s Debug: %s", getElapseTime().toStdString().c_str(), msg );
                break;
            case QtWarningMsg:
                sprintf( oneLineDebugMsgBuffer, "%s Warning: %s", getElapseTime().toStdString().c_str(), msg );
                break;
            case QtCriticalMsg:
                sprintf( oneLineDebugMsgBuffer, "%s Critical: %s", getElapseTime().toStdString().c_str(), msg );
                break;
            case QtFatalMsg:
                sprintf( oneLineDebugMsgBuffer, "%s Fatal: %s", getElapseTime().toStdString().c_str(), msg );
                abort();
            default:
                sprintf( oneLineDebugMsgBuffer, "%s Unknown %d: %s", getElapseTime().toStdString().c_str(), type, msg );
                break;
        }

        if( textEdit != NULL )
        {
            textEdit->append(QString("%1").arg( oneLineDebugMsgBuffer ) );
        }

        if( file != NULL )
        {
            fprintf( file, "%s\n", oneLineDebugMsgBuffer );
            fflush( file );
        }
    }
}
