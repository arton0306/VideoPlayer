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
    tuner.init( 2, 44100, 16 );
    tuner.setPitchShiftInSemiTones( +3 );

    vector<unsigned char> buffer(1024, 0);
    FILE * inputFile = fopen( "tools/Lelouch.mp4.pcm", "rb" );
    FILE * outputFile = fopen( "tools/tunedLelouch.pcm", "ab" );

    while ( !feof( inputFile ) )
    {
        int readBytes = fread( &buffer[0], 1, 1024, inputFile );

        if ( readBytes != 1024 )
        {
            DEBUG() << "read bytes != 1024";
            buffer.erase( buffer.begin() + readBytes, buffer.end() );
        }
        vector<unsigned char> tunedBuffer = tuner.processPitchShift( buffer );

        fwrite( &tunedBuffer[0], 1, tunedBuffer.size(), outputFile );
        DEBUG() << "write " << tunedBuffer.size() << " bytes to file";
    }
    DEBUG() << "flush left in buffer";
    vector<unsigned char> tunedBuffer = tuner.flushProcessBuffer();
    DEBUG() << "finish";
    fclose( inputFile );
    fclose( outputFile );

    return a.exec();
}
