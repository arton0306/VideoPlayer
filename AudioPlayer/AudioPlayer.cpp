#include "stdlib.h"
#include "stdio.h"
#include <cassert>
#include <cstring>

#include "AudioPlayer.hpp"

/* static */ bool AudioPlayer::sIsInit = false;

AudioPlayer::AudioPlayer( int aChannel, SampleFormat aSampleFormat, double aSampleRate )
{
    assert( sIsInit );

    mChannel = aChannel;
    mSampleFormat = getPaSampleFormat( aSampleFormat );
    mSampleRate = aSampleRate;

    mBufferSize = 20 * 1024 * 1024;
    mStreamBuffer = (char*)malloc( mBufferSize );
    mStart = 0;
    mEnd = 0;

    PaError err;
    err = Pa_OpenDefaultStream( &mPaStream,
                                0,                              /* no input channels */
                                mChannel,
                                mSampleFormat,
                                mSampleRate,
                                paFramesPerBufferUnspecified,   /* frames per buffer */
                                AudioPlayer::callback,
                                this );
    assert( err == paNoError );
}

AudioPlayer::~AudioPlayer()
{
    PaError err = Pa_CloseStream( mPaStream );
    assert( err == paNoError );

    free( mStreamBuffer );
}

/* static */ bool AudioPlayer::init()
{
    PaError err = Pa_Initialize();
    assert( err == paNoError );
    sIsInit = true;
    return err;
}

/* static */ bool AudioPlayer::finish()
{
    PaError err = Pa_Terminate();
    assert( err == paNoError );
    sIsInit = false;
    return err;
}

void AudioPlayer::play()
{
    PaError err = Pa_StartStream( mPaStream );
    assert( err == paNoError );
}

void AudioPlayer::stop()
{
    PaError err = Pa_StopStream( mPaStream );
    assert( err == paNoError );
}

/* static */ int AudioPlayer::callback
    (
    const void *inputBuffer,
    void *outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData
    )
{
    AudioPlayer * context = (AudioPlayer*)userData;
    int const sampleBytes = Pa_GetSampleSize( context->mSampleFormat );

    char *out = (char*)outputBuffer;
    (void) inputBuffer; /* Prevent unused variable warning. */
    (void) timeInfo;    // TODO: use it!
    (void) statusFlags; // TODO: use it!

    for( int i = 0; i < framesPerBuffer; ++i )
    {
        for ( int j = 0; j < context->mChannel; ++j )
        {
            for ( int k = 0; k < sampleBytes; ++k )
            {
                *out++ = context->mStreamBuffer[context->mStart];
                context->mStart = context->getNextIndex( context->mStart );
            }
        }
    }
    return 0;
}

PaSampleFormat AudioPlayer::getPaSampleFormat( SampleFormat aSampleFormat ) const
{
    switch ( aSampleFormat )
    {
        case Float32: return paFloat32;
        case Int32:   return paInt32;
        case Int24:   return paInt24;
        case Int16:   return paInt16;
        case Int8:    return paInt8;
        case UInt8:   return paUInt8;
        default:
            assert( false );
            return Error;
    }
}

// return consume size
int AudioPlayer::pushStream(
    char * aInputStream,
    int aInputSize )
{
    int consume = 0;

    // we must save it as local because the callback will change the mStart in interrupt
    int beforeStart = getPreviousIndex( mStart );
    for( ;
         mEnd != beforeStart && consume < aInputSize;
         mEnd = getNextIndex( mEnd ), consume += 1 )
    {
        memcpy( &mStreamBuffer[mEnd], &aInputStream[consume], 1 );
    }

    return consume;
}

int AudioPlayer::getPreviousIndex( int pos ) const
{
    return ( pos + mBufferSize - 1 ) % mBufferSize;
}

int AudioPlayer::getNextIndex( int pos ) const
{
    return ( pos + 1 ) % mBufferSize;
}

int AudioPlayer::getUsedSize() const
{
    return ( mEnd + mBufferSize - mStart ) % mBufferSize;
}

int AudioPlayer::getAvailableSize() const
{
    // mEnd is always empty, so minus 1
    return mBufferSize - getUsedSize() - 1;
}

