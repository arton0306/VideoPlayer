#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include "AudioPlayer.hpp"

using namespace std;

int main()
{
    printf( "sizeof short: %d bytes\n", sizeof( short ) );

    // audio file prepare
    // FILE * inputFile = fopen( "lulu.mp4.pcm", "rb" );
    FILE * inputFile = fopen( "5sec_lulu.wmv.pcm", "rb" );
    int const BUFFER_SIZE = 10 * 1024 * 1024;
    char * data;
    data = (char *)(malloc( BUFFER_SIZE ));

    int readBytes = fread( &data[0], 1, BUFFER_SIZE, inputFile );
    printf( "file bytes read: %d\n", readBytes );
    assert( feof( inputFile ) );

    // port audio
    /*
    AudioPlayer::init();
    AudioPlayer audioPlayer( 2, AudioPlayer::Float32, 48000 );
    audioPlayer.pushStream( data, readBytes );
    audioPlayer.play();

    Pa_Sleep(30*1000);
    */


    AudioPlayer::init();
    AudioPlayer audioPlayer( 2, AudioPlayer::Float32, 48000 );
    Pa_Sleep(3000);
    int pushbytes = 0;
    for ( int i = 0; i < 100; i++ )
    {
        pushbytes += audioPlayer.pushStream( data + pushbytes, readBytes / 100);
        if ( i == 10 ) audioPlayer.play();
        Pa_Sleep(50);
        cout << "getPlaySec " << audioPlayer.getPlaySec() << endl;
    }

    Pa_Sleep(30*1000);

    return 0;
}
