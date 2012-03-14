#ifndef DEBUG_H
#define DEBUG_H

#include <QtGlobal>
#include <QtDebug>
#include <QString>

#define DEBUG_ADD_FILE_FUNC_LINE_INFO // The macro ref: http://www.qtcentre.org/archive/index.php/t-6301.html

/*----- Flags for Debug Output -----*/
# define DEBUG_PRINT_FILEPATH
# define DEBUG_PRINT_SHORT_FN_NAME
# define DEBUG_PRINT_LINENUMBER

/*----- Adjust Length of fields Here -----*/
// FileName
# define DEBUG_FILENAME_LENGTH 22
// Function
# define DEBUG_FUNCTION_LENGTH 25
// LineNumber
# define DEBUG_LINENUMBER_LENGTH 4

/*----- Get Names -----*/
// FileNames
# if defined( DEBUG_PRINT_FILEPATH )
    # define DEBUG_FILENAME ( QString( "%1").arg( __FILE__, -DEBUG_FILENAME_LENGTH ) )
# else
    # define DEBUG_FILENAME
# endif
// Function Names
# if defined ( DEBUG_PRINT_SHORT_FN_NAME )
    # define DEBUG_FUNCTION_NAME QString( " %1").arg( __FUNCTION__, -DEBUG_FUNCTION_LENGTH )
# else
    # define DEBUG_FUNCTION_NAME
# endif
// Line numbers
# if defined ( DEBUG_PRINT_LINENUMBER )
    # define DEBUG_LINENUMBER ( QString("%1").arg( QString::number(__LINE__), DEBUG_LINENUMBER_LENGTH ) )
# else
    # define DEBUG_LINENUMBER
# endif

/*----- Debug Macro -----*/
#if defined( DEBUG_ADD_FILE_FUNC_LINE_INFO )
    # define DEBUG() qDebug() << QString( "[" + DEBUG_FILENAME + DEBUG_LINENUMBER + DEBUG_FUNCTION_NAME + "]" )
#else
    # define DEBUG() qDebug()
#endif

namespace debug                                              
{                                                            
    void setProgramTimer();                                  
    void initDebugWindow();                                  
    QString getElapseTime();                                 
    void debugWinMsgHandler(QtMsgType type, const char *msg);
};

#endif // DEBUG_H
