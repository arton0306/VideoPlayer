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

    mBufferSize = 1 * 1024 * 1024;
    mStreamBuffer = (char*)malloc( mBufferSize );
    fillDefaultSample();
    mStart = 0;
    mEnd = 0;
    mConsumedBytes = 0;

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

    mPlaySec = 0.0;
}

// so we can debug conviniently
void AudioPlayer::fillDefaultSample()
{
    double ratio = 0;
    int sampleSize = 0;

    switch ( mSampleFormat )
    {
        case paFloat32: ratio = 1.0;        sampleSize = 4;  break;
        case paInt32  : ratio = 0x7fffffff; sampleSize = 4;  break;
        case paInt16  : ratio = 0x7fff;     sampleSize = 2;  break;
        case paInt8   : ratio = 0x7f;       sampleSize = 1;  break;
        default :       ratio = 0x7fff;     sampleSize = 2;  break; // int16 is popular sample format;
    }

    /*
    printf( "ratio = %lf\n", ratio );
    printf( "sizeof(short) = %u\n", sizeof(short) );
    printf( "mSampleFormat = %d\n", mSampleFormat );
    */

    double sampleValue = 0.0;
    for ( int i = 0; i < mBufferSize / mChannel / sampleSize; ++i )
    {
        for ( int j = 0; j < mChannel; ++j )
        {
            //mStreamBuffer[i * mChannel + j] = sampleValue * ratio;
            double fitRatio = sampleValue * ratio;
            int destPos = i * mChannel * sampleSize + j * sampleSize;
            switch ( mSampleFormat )
            {
                case paFloat32: { float temp = (float)fitRatio; memcpy( &mStreamBuffer[destPos], &temp, 4 ); } break;
                case paInt32  : { int   temp =   (int)fitRatio; memcpy( &mStreamBuffer[destPos], &temp, 4 ); } break;
                case paInt16  : { short temp = (short)fitRatio; memcpy( &mStreamBuffer[destPos], &temp, 2 ); } break;
                case paInt8   : { char  temp =  (char)fitRatio; memcpy( &mStreamBuffer[destPos], &temp, 1 ); } break;
                default :   break; // we don't know how to init it with default sample
            }
        }
        // Generate simple sawtooth phaser that ranges between -1.0 and 1.0.
        sampleValue += 0.01f;
        // When signal reaches top, drop back down.
        if( sampleValue >= 1.0f ) sampleValue -= 2.0f;
    }
}

AudioPlayer::~AudioPlayer()
{
    PaError err = Pa_CloseStream( mPaStream );
    assert( err == paNoError );

    free( mStreamBuffer );
}

/* static */ bool AudioPlayer::init()
{
    PaError err = paNoError;
    if ( !sIsInit )
    {
        err = Pa_Initialize();
        assert( err == paNoError );
        sIsInit = true;
    }
    return err;
}

/* static */ bool AudioPlayer::finish()
{
    PaError err = paNoError;
    if ( sIsInit )
    {
        PaError err = Pa_Terminate();
        assert( err == paNoError );
        sIsInit = false;
    }
    return err;
}

void AudioPlayer::play()
{
    PaError err = Pa_StartStream( mPaStream );
    // printf( "in play err=%d\n", err);
    assert( err == paNoError );
}

void AudioPlayer::stop()
{
    PaError err = Pa_StopStream( mPaStream );
    // printf( "in stop err=%d\n", err);
    assert( err == paNoError );
    mStart = 0;
    mEnd = 0;
    mConsumedBytes = 0;
}

double AudioPlayer::getPlaySec() const
{
    double const latency = Pa_GetStreamInfo( mPaStream )->outputLatency;
    double const bytesPerSec = mSampleRate * Pa_GetSampleSize( mSampleFormat ) * mChannel;
    double const playtime = mConsumedBytes / bytesPerSec;
    return playtime - latency;
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
    (void) timeInfo;    /* Prevent unused variable warning. */
    (void) statusFlags; // TODO: use it!

    for( int i = 0; i < framesPerBuffer; ++i )
    {
        for ( int j = 0; j < context->mChannel; ++j )
        {
            for ( int k = 0; k < sampleBytes; ++k )
            {
                *out++ = context->mStreamBuffer[context->mStart];
                context->mStart = context->getNextIndex( context->mStart );
                context->mConsumedBytes += 1;
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
    char const * aInputStream,
    int aInputSize )
{
    int consume = 0;

    // we must save it as local because the callback will change the mStart in interrupt
    int beforeStart = getPreviousIndex( mStart );
    //int beforeStart = 0; // ok
    //int beforeStart = mBufferSize - 1; // fail
    //int beforeStart = 15000000; // fail
    //int beforeStart = 10000000; // ok
    for(;
        mEnd != beforeStart && consume < aInputSize;
        mEnd = getNextIndex( mEnd ), consume += 1 )
    {
        memcpy( &mStreamBuffer[mEnd], &aInputStream[consume], 1 );
        /*
        if ( mEnd % 1000 == 0 )
        {
            printf( "beforeStart=%d mEnd=%d consume=%d\n", beforeStart, mEnd, consume );
        }
        */
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

