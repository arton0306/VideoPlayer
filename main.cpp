#include <QtGui/QApplication>
#include "AVInfo.hpp"
#include "VideoPlayer.hpp"
#include "debug.hpp"
#include "MultimediaWidget.hpp"
#include "SoundTouch.h"
#include "STTypes.h"
#include "AudioTuner.hpp"
#include <cstdio>
#include <vector>

using namespace std;
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

    //VideoPlayer w;
    // MultimediaWidget w;
    //w.show();
    AudioTuner tuner;
    tuner.setParameter( 2, 44100, 16 );
    tuner.setSpeechMode( false );
    tuner.setPitchShiftInSemiTones( +3 );

    vector<unsigned char> buffer(1024, 0);
    FILE * inputFile = fopen( "tools/10years.pcm", "rb" );
    FILE * outputFile = fopen( "tools/10years.3tone_inc.pcm", "ab" );

    while ( !feof( inputFile ) )
    {
        int readBytes = fread( &buffer[0], 1, 1024, inputFile );

        if ( readBytes != 1024 )
        {
            buffer.erase( buffer.begin() + readBytes, buffer.end() );
        }
        vector<unsigned char> tunedBuffer = tuner.process( buffer );

        fwrite( &tunedBuffer[0], 1, tunedBuffer.size(), outputFile );
    }
    vector<unsigned char> tunedBuffer = tuner.flush();
    fclose( inputFile );
    fclose( outputFile );

    return a.exec();
}
