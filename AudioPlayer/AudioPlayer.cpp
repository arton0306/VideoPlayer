#include "stdlib.h"
#include "stdio.h"
#include <cassert>
#include <cstring>
#include "AudioPlayer.hpp"
#include "CircularBuffer.hpp"

#ifdef AudioPlayerTest
    #include <iostream>
    using namespace std;
    #define LOG cout
#else
    #include "debug.hpp"
    #define LOG DEBUG()
#endif

/* static */ bool AudioPlayer::sIsInit = false;

AudioPlayer::AudioPlayer
(
    int aChannel,
    SampleFormat aSampleFormat,
    double aSampleRate
)
{
    assert( sIsInit );

    mChannel = aChannel;
    mSampleFormat = getPaSampleFormat( aSampleFormat );
    mSampleRate = aSampleRate;

    mBuffer = new CirBuf(64*1024);
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

AudioPlayer::~AudioPlayer()
{
    PaError err = Pa_CloseStream( mPaStream );
    assert( err == paNoError );

    delete(mBuffer);
    printf("~AudioPlayer\n");
}

int AudioPlayer::pushStream(char const * buf, int len)
{
    return mBuffer->addData(buf, len);
}

void AudioPlayer::resetBufferInfo()
{
    mFramesWriteToBufferInCallback = 0;
    mConsumedBytes = 0;
}

void AudioPlayer::fillTestSample()
{
    fillDefaultSample();
}

void AudioPlayer::fillDefaultSample()
{
    /*
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
        sampleValue += 0.01f;
        if( sampleValue >= 1.0f ) sampleValue -= 2.0f;
    }
    */
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
    double const playtime = (double)mConsumedBytes / (double)bytesPerSec;
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
    int const frameBytes = ctx->mChannel * sampleBytes;
    for( int i = 0; i < framesPerBuffer; ++i ) {
        if ( ctx->mBuffer->getUsed() >= frameBytes ) {
            for ( int j = 0; j < frameBytes; ++j ) {
                *out++ = ctx->mBuffer->pop();
            }
            ctx->mConsumedBytes += frameBytes;
            ctx->mFramesWriteToBufferInCallback += 1;
        } else {
            memset(out, 0, frameBytes);
            out += frameBytes;
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

void AudioPlayer::dumpCallbackContext() const
{
    printf( "count = %d\n", mCBCtx.mCount );
    printf( "output underflow count = %d\n", mCBCtx.mOutputUnderflowCount );
    printf( "output overflow count = %d\n", mCBCtx.mOutputOverflowCount );
    printf( "priming count = %d\n", mCBCtx.mPrimingCount );
}
