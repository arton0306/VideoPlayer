#ifndef AUDIOTUNER_HPP
#define AUDIOTUNER_HPP

#include <vector>
#include "SoundTouch.h"

typedef unsigned char uint8;

class AudioTuner
{
public:
    AudioTuner();
    void setParameter( int aChannel, int aSampleRate, int aBitsPerSample );
    void setSpeechMode( bool aIsSpeechMode );
    void setPitchShiftInSemiTones( int aDelta /* -60 ~ +60 */ );
    std::vector<uint8> process( std::vector<uint8> const & aInputStream );
    std::vector<unsigned char> flush();

private:
    std::vector<uint8> internalProcess();
    void read( std::vector<uint8> const & aInputStream );
    void write( std::vector<uint8> & aProcessedStream, int & aTail, int aElemCount );

    soundtouch::SoundTouch mSoundTouch;
    int mSemiTonesDelta;

    int mBitsPerSample;
    int mChannel;
    int mSampleRate;

    // after souch touch processing the input samples,
    // it output the stream at a few round which is totally less then 20k (by experiences)
    // we use 30k to tolerate more
    static int const OUTPUT_BUFFER_SIZE = 1024 * 30;

    // SAMPLETYPE is float, the mBufferForProcess is for input
    static int const PROCESS_BUFFER_SIZE = 1024 * 3;
    float mBufferForProcess[AudioTuner::PROCESS_BUFFER_SIZE];
};

#endif // AUDIOTUNER_HPP
