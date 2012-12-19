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

    AudioPlayer( int aChannel, SampleFormat aSampleFormat, double aSampleRate );
    ~AudioPlayer();
    int pushStream
        (
        char * aInputStream,  /* input audio stream */
        int aInputSize /* input audio stream size */
        );
    void play();
    void stop();

private:
    static bool sIsInit;
    PaSampleFormat getPaSampleFormat( SampleFormat aSampleFormat ) const;
    int getPreviousIndex( int pos ) const;
    int getNextIndex( int pos ) const;
    int getUsedSize() const;
    int getAvailableSize() const;
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
    char * mStreamBuffer;
    int mBufferSize;
    int mStart; // the callback will read from this index
    int mEnd;   // the index where the users push from, the index is always empty

    // port audio data
    PaStream * mPaStream;
};

#endif
