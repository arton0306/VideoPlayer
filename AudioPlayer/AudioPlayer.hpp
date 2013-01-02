#ifndef AUDIO_PLAYER_HPP
#define AUDIO_PLAYER_HPP

#include "portaudio.h"

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
        double aSampleRate,
        int aDebugSize = 1 * 1024 * 1024
        );
    ~AudioPlayer();
    int pushStream
        (
        char const * aInputStream,  /* input audio stream */
        int aInputSize /* input audio stream size */
        );
    void play();
    void stop();
    double getPlaySec() const;

private:
    static bool sIsInit;
    void resetBufferInfo();
    PaSampleFormat getPaSampleFormat( SampleFormat aSampleFormat ) const;
    int getPreviousIndex( int pos ) const;
    int getNextIndex( int pos ) const;
    int getUsedSize() const;
    int getAvailableSize() const;
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

    // TODO: check volatile is needed or not
    // audio buffer data
    char * mStreamBuffer;
    int mBufferSize;
    int mStart; // the callback will read from this index
    int mEnd;   // the index where the users push from, the element on this index is always empty
    volatile long long mPlayByteCount;
    volatile long long mWriteByteCount;
    unsigned long mFramesWriteToBufferInCallback;
    double mConsumedBytes; // we use this to estimate the playing time

    // port audio data
    PaStream * mPaStream;
    struct CallbackContext
    {
        int mCount;
        int mOutputUnderflowCount;
        int mOutputOverflowCount;
        int mPrimingCount;
    } mCallbackContext;

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
