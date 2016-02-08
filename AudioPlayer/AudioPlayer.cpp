#include "stdlib.h"
#include "stdio.h"
#include <cassert>
#include <cstring>
#include "AudioPlayer.hpp"

#ifdef AudioPlayerTest
    #include <iostream>
    using namespace std;
    #define LOG cout
#else
    #include "debug.hpp"
    #define LOG DEBUG()
#endif

#define uint8_t char

/* static */ bool AudioPlayer::sIsInit = false;

AudioPlayer::AudioPlayer
(
    int aChannel,
    SampleFormat aSampleFormat,
    double aSampleRate,
    int aDebugSize
)
{
    assert( sIsInit );

    mChannel = aChannel;
    mSampleFormat = getPaSampleFormat( aSampleFormat );
    mSampleRate = aSampleRate;

    mBufferSize = 32 * 1024;
    mStreamBuffer = (char*)malloc( mBufferSize );
    resetBufferInfo();

    LOG << "open stream, sample format:" << mSampleFormat << endl;

    PaError err;
    err = Pa_OpenDefaultStream( &mPaStream,
                                0,                              /* no input channels */
                                mChannel,
                                mSampleFormat,
                                mSampleRate,
                                paFramesPerBufferUnspecified,   /* frames per buffer */
                                AudioPlayer::callback,
                                this );
    if (err != paNoError) {
        LOG << "Pa_OpenDefaultStream result:" << err << " (" << Pa_GetErrorText(err) << ")" << endl;
        assert(false);
    }

    memset( &mCBCtx, 0, sizeof( struct CallbackContext ) );
}

void AudioPlayer::resetBufferInfo()
{
    // fillDefaultSample();
    mStart = 0;
    mEnd = 0;
    mPlayByteCount = 0;
    mWriteByteCount = 0;
    mFramesWriteToBufferInCallback = 0;
    mConsumedBytes = 0;
}

void AudioPlayer::fillTestSample()
{
    fillDefaultSample();
}

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

    assert(sizeof(short)==2);
    // printf( "ratio = %lf\n", ratio );
    // printf( "mSampleFormat = %d\n", mSampleFormat );

    double sampleValue = 0.0;
    int sampleCnt = mBufferSize / mChannel / sampleSize;
    for ( int i = 0; i < sampleCnt; ++i )
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
    /*
    mDebugBuffer.mDebugSize = aDebugSize;
    mDebugBuffer.mPlayStream = (char*)malloc( aDebugSize );
    memset( &mDebugBuffer.mPlayStream[0], 0, aDebugSize );
    mDebugBuffer.mWriteStream = (char*)malloc( aDebugSize );
    memset( &mDebugBuffer.mWriteStream[0], 0, aDebugSize );
    mDebugBuffer.mPlayStreamIndex = 0;
    mDebugBuffer.mWriteStreamIndex = 0;
    */
}

AudioPlayer::~AudioPlayer()
{
    PaError err = Pa_CloseStream( mPaStream );
    assert( err == paNoError );    

    printf("~AudioPlayer\n");

    /*
    FILE * f = fopen( "portaudio_playing.raw", "wb" );
    for ( int i = 0; i < mDebugBuffer.mDebugSize; ++i )
        fwrite(&mDebugBuffer.mPlayStream[i], 1, 1, f);
    fclose(f);

    free( mStreamBuffer ); mStreamBuffer = 0;
    free( mDebugBuffer.mPlayStream ); mDebugBuffer.mPlayStream = 0;
    free( mDebugBuffer.mWriteStream ); mDebugBuffer.mWriteStream = 0;
    */
}

/* static */ bool AudioPlayer::init()
{
    PaError err = paNotInitialized;
    if ( !sIsInit )
    {
        err = Pa_Initialize();
        if (err != paNoError) {
            LOG << "Pa_Initialize result:" << err << " (" << Pa_GetErrorText(err) << ")" << endl;
            assert(false);
        }
        sIsInit = true;
    }
    assert(err == paNoError);
    assert(Pa_GetDeviceCount() > 0);

    return err;
}

/* static */ bool AudioPlayer::finish()
{
    PaError err = paNotInitialized;
    if ( sIsInit )
    {
        PaError err = Pa_Terminate();
        if( err != paNoError ) {
            printf(  "Pa_Terminate error: %s\n", Pa_GetErrorText( err ) );
            assert(false);
        }
        sIsInit = false;
    }
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
    resetBufferInfo();
}

double AudioPlayer::getPlaySec() const
{
    double const deviceLatency = Pa_GetStreamInfo( mPaStream )->outputLatency;
    double const bufferLatency = mFramesWriteToBufferInCallback / mSampleRate;
    double const bytesPerSec = mSampleRate * Pa_GetSampleSize( mSampleFormat ) * mChannel;
    double const playtime = mConsumedBytes / bytesPerSec;
    return playtime - deviceLatency - bufferLatency;
}

/* static */ int AudioPlayer::callback
    (
    const void *inBuf,
    void *outBuf,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData
    )
{
    AudioPlayer * ctx = (AudioPlayer*)userData;
    int const sampleBytes = Pa_GetSampleSize( ctx->mSampleFormat );

    char *out = (char*)outBuf;
    (void) inBuf;       /* Prevent unused variable warning. */
    (void) timeInfo;    /* Prevent unused variable warning. */

    ctx->mFramesWriteToBufferInCallback = 0;
    for( int i = 0; i < framesPerBuffer; ++i )
    {
        int const frameByteCount = ctx->mChannel * sampleBytes;
        if ( ctx->mWriteByteCount >= ctx->mPlayByteCount + frameByteCount )
        {
            for ( int j = 0; j < frameByteCount; ++j )
            {
                uint8_t b = ctx->mStreamBuffer[ctx->mStart];
                *out++ = b;
                ctx->mStart = ctx->getNextIndex( ctx->mStart );
                ctx->mConsumedBytes += 1;
                ++ctx->mPlayByteCount;
            }
            ++ctx->mFramesWriteToBufferInCallback;
        }
        else
        {
            for ( int j = 0; j < frameByteCount; ++j )
            {
                *out++ = 0;
            }
        }
    }

    return 0;
}

void AudioPlayer::recordStatusFlags( PaStreamCallbackFlags flags )
{
    mCBCtx.mCount += 1;
    if( flags & paOutputUnderflow ) mCBCtx.mOutputUnderflowCount += 1;
    if( flags & paOutputOverflow )  mCBCtx.mOutputOverflowCount += 1;
    if( flags & paPrimingOutput )   mCBCtx.mPrimingCount += 1;
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
        ++mWriteByteCount;
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
    printf( "count = %d\n", mCBCtx.mCount );
    printf( "output underflow count = %d\n", mCBCtx.mOutputUnderflowCount );
    printf( "output overflow count = %d\n", mCBCtx.mOutputOverflowCount );
    printf( "priming count = %d\n", mCBCtx.mPrimingCount );
}
