#ifndef AUDIOTUNER_HPP
#define AUDIOTUNER_HPP

#include <vector>
#include "SoundTouch.h"

typedef unsigned char uint8;

class AudioTuner
{
public:
    AudioTuner();
    void init( int aChannel, int aSampleRate, int aBitsPerSample, bool aIsSpeechMode = false );
    void setPitchShiftInSemiTones( int aDelta /* -60 ~ +60 */ );
    std::vector<uint8> processPitchShift( std::vector<uint8> const & inputStream );
    std::vector<unsigned char> flushProcessBuffer();

private:
    void read( std::vector<uint8> const & inputStream );
    void write( std::vector<uint8> & aProcessedStream, int & aTail, int aElemCount );

    soundtouch::SoundTouch mSoundTouch;
    int mSemiTonesDelta;

    int mBitsPerSample;
    int mChannel;
    int mSampleRate;

    // SAMPLETYPE is float
    static int const BUFFER_SIZE = 1024 * 3;
    float mBufferForProcess[AudioTuner::BUFFER_SIZE];
};

#endif // AUDIOTUNER_HPP
