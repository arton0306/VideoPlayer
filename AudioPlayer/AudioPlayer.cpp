#include "stdlib.h"
#include "stdio.h"
#include <cassert>
#include <cstring>

#include "AudioPlayer.hpp"

/* static */ bool AudioPlayer::sIsInit = false;

AudioPlayer::AudioPlayer
(
    int aChannel,
    SampleFormat aSampleFormat,
    double aSampleRate,
    int aDebugSize /* 30 * 1024 * 1024 */
)
{
    assert( sIsInit );

    mChannel = aChannel;
    mSampleFormat = getPaSampleFormat( aSampleFormat );
    mSampleRate = aSampleRate;

    mBufferSize = 16 * 1024;
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
    memset( &mCallbackContext, 0, sizeof( struct CallbackContext ) );

    initDebugBuffer( aDebugSize );
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

void AudioPlayer::initDebugBuffer( int aDebugSize )
{
    mDebugBuffer.mDebugSize = aDebugSize;
    mDebugBuffer.mPlayStream = (char*)malloc( aDebugSize );
    memset( &mDebugBuffer.mPlayStream[0], 0, aDebugSize );
    mDebugBuffer.mWriteStream = (char*)malloc( aDebugSize );
    memset( &mDebugBuffer.mWriteStream[0], 0, aDebugSize );
    mDebugBuffer.mPlayStreamIndex = 0;
    mDebugBuffer.mWriteStreamIndex = 0;
}

AudioPlayer::~AudioPlayer()
{
    PaError err = Pa_CloseStream( mPaStream );
    assert( err == paNoError );

    printf("~AudioPlayer\n");
    FILE * f = fopen( "portaudio_playing.raw", "wb" );
    for ( int i = 0; i < mDebugBuffer.mDebugSize; ++i )
        fwrite(&mDebugBuffer.mPlayStream[i], 1, 1, f);
    fclose(f);

    free( mStreamBuffer ); mStreamBuffer = 0;
    free( mDebugBuffer.mPlayStream ); mDebugBuffer.mPlayStream = 0;
    free( mDebugBuffer.mWriteStream ); mDebugBuffer.mWriteStream = 0;
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

    for( int i = 0; i < framesPerBuffer; ++i )
    {
        for ( int j = 0; j < context->mChannel; ++j )
        {
            for ( int k = 0; k < sampleBytes; ++k )
            {
                // write to portaudio output
                char playingByte = context->mStreamBuffer[context->mStart];
                *out++ = playingByte;
                context->mStart = context->getNextIndex( context->mStart );
                context->mConsumedBytes += 1;

                // debug info
                if ( context->mDebugBuffer.mPlayStreamIndex < context->mDebugBuffer.mDebugSize )
                    context->mDebugBuffer.mPlayStream[context->mDebugBuffer.mPlayStreamIndex++] = playingByte;
            }
        }
    }

    //context->recordStatusFlags( statusFlags );
    //context->dumpCallbackContext();

    //printf( "1st sample to output: %10.5lf , cur playing time:%10.5lf, diff: %10.5lf\n", timeInfo->outputBufferDacTime , context->getPlaySec(), timeInfo->outputBufferDacTime - context->getPlaySec() );

    return 0;
}

void AudioPlayer::recordStatusFlags( PaStreamCallbackFlags flags )
{
    mCallbackContext.mCount += 1;
    if( flags & paOutputUnderflow ) mCallbackContext.mOutputUnderflowCount += 1;
    if( flags & paOutputOverflow )  mCallbackContext.mOutputOverflowCount += 1;
    if( flags & paPrimingOutput )   mCallbackContext.mPrimingCount += 1;

    //if( flags & paOutputUnderflow ) dumpCallbackContext();
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
    for(;
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

void AudioPlayer::dumpCallbackContext() const
{
    printf( "count = %d\n", mCallbackContext.mCount );
    printf( "output underflow count = %d\n", mCallbackContext.mOutputUnderflowCount );
    printf( "output overflow count = %d\n", mCallbackContext.mOutputOverflowCount );
    printf( "priming count = %d\n", mCallbackContext.mPrimingCount );
}
