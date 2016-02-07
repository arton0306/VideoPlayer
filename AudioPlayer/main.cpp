#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include "AudioPlayer.hpp"

using namespace std;

int main()
{
    FILE * inputFile = fopen( "bensound-cute.10sec.pcm", "rb" );
    int const BUFFER_SIZE = 10 * 1024 * 1024;
    char * data;
    data = (char *)(malloc( BUFFER_SIZE ));

    int readBytes = fread( &data[0], 1, BUFFER_SIZE, inputFile );
    printf( "file bytes read: %d\n", readBytes );
    assert( feof( inputFile ) );

    if (0) {
        AudioPlayer::init();
        AudioPlayer audioPlayer( 2, AudioPlayer::Int16, 44100 );
        audioPlayer.pushStream( data, readBytes );
        audioPlayer.play();
        Pa_Sleep(15000);
    } else {
        AudioPlayer::init();
        AudioPlayer audioPlayer( 2, AudioPlayer::Int16, 44100 );
        Pa_Sleep(1000);
        int pushbytes = 0;
        for ( int i = 0; i < 100; i++ ) {
            pushbytes += audioPlayer.pushStream( data + pushbytes, readBytes / 100);
            if ( i == 10 ) audioPlayer.play();
            Pa_Sleep(50);
            cout << "getPlaySec " << audioPlayer.getPlaySec() << endl;
        }
        Pa_Sleep(15000);
    }

    free(data);
    return 0;
}
