#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include "AudioPlayer.hpp"
#include "CircularBuffer.hpp"

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

    int test_mode = 1;
    if (test_mode == 0) {
        AudioPlayer::init();
        AudioPlayer audioPlayer( 2, AudioPlayer::Int16, 44100 );
        audioPlayer.pushStream( data, readBytes );
        audioPlayer.play();
        Pa_Sleep(15000);
    } else if (test_mode == 1) {
        AudioPlayer::init();
        AudioPlayer audioPlayer( 2, AudioPlayer::Int16, 44100 );
        Pa_Sleep(1000);
        int pushbytes = 0;
        for ( int i = 0; i < 750; i++ ) {
            pushbytes += audioPlayer.pushStream( data + pushbytes, readBytes / 250);
            if ( i == 10 ) audioPlayer.play();
            Pa_Sleep(20);
            cout << "getPlaySec " << audioPlayer.getPlaySec() << endl;
        }
        Pa_Sleep(15000);
    } else if (test_mode == 2) {
        AudioPlayer::init();
        AudioPlayer audioPlayer( 2, AudioPlayer::Int16, 44100 );
        Pa_Sleep(1000);
        for ( int i = 0; i < 100; i++ ) {
            audioPlayer.fillTestSample();
            if ( i == 10 ) audioPlayer.play();
            Pa_Sleep(50);
            cout << "getPlaySec " << audioPlayer.getPlaySec() << endl;
        }
        Pa_Sleep(5000);
    } else if (test_mode == 3) {
        CirBuf cirbuf(4096);
        char buf[256];
        for (int i = 0; i < 256; ++i) buf[i] = i;
        cirbuf.addData(buf, 256);

        for (int i = 0; i < 256; ++i) {
            printf("%d ", cirbuf.pop());
        }
    }

    free(data);
    return 0;
}
