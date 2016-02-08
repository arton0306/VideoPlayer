#ifndef AUDIO_PLAYER_HPP
#define AUDIO_PLAYER_HPP

#include "portaudio.h"

class CirBuf;

class AudioPlayer
{
public:
    enum SampleFormat
    {
        Float32,
        Int32,
        Int24,
        Int16,
        Int8,
        UInt8,

        Error
    };

    static bool init();
    static bool finish();

    AudioPlayer(
        int aChannel,
        SampleFormat aSampleFormat,
        double aSampleRate
        );
    ~AudioPlayer();

    int pushStream(
        char const * buf,  /* input audio stream */
        int len            /* input audio stream size */
        );
    void play();
    void stop();
    double getPlaySec() const;
    void fillTestSample();

private:
    static bool sIsInit;
    void resetBufferInfo();
    PaSampleFormat getPaSampleFormat( SampleFormat aSampleFormat ) const;
    void fillDefaultSample(); // for debug
    void initDebugBuffer( int aDebugSize ); // for debug
    void dumpCallbackContext() const;
    void recordStatusFlags( PaStreamCallbackFlags flags );
    static int callback
        (
        const void *inputBuffer,
        void *outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void *userData
        );

    // stream info
    int mChannel;
    PaSampleFormat mSampleFormat;
    double mSampleRate;

    // audio buffer data
    CirBuf *mBuffer;
    unsigned long mFramesWriteToBufferInCallback;
    long long mConsumedBytes;   // we use this to estimate the playing time

    // port audio data
    PaStream * mPaStream;
    struct CallbackContext
    {
        int mCount;
        int mOutputUnderflowCount;
        int mOutputOverflowCount;
        int mPrimingCount;
    } mCBCtx;

    // debug info, we will check the mPlayStream and mWriteStream is the same
    struct DebugBuffer
    {
        int mDebugSize;
        char * mPlayStream;
        char * mWriteStream;
        int mPlayStreamIndex;
        int mWriteStreamIndex;
    } mDebugBuffer;
};

#endif
