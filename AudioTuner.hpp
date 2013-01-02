#ifndef AUDIOTUNER_HPP
#define AUDIOTUNER_HPP

#include <vector>
#include "SoundTouch.h"

typedef unsigned char uint8;

class AudioTuner
{
public:
    AudioTuner();
    void init( int aChannel, int aSampleRate, int aBitsPerSample );
    std::vector<uint8> process( std::vector<uint8> const & aInputStream );
    std::vector<uint8> flush();

    // effect
    void setSpeechMode( bool aIsSpeechMode );
    void setPitchSemiTones( int aDelta /* -60 ~ +60 */ );
    void setVol( double aPercent /* 0.0 ~ 1.0 */ );
    void setLeftChanVol( double aPercent /* 0.0 ~ 1.0 */ );  // cannot be called if mono
    void setRightChanVol( double aPercent /* 0.0 ~ 1.0 */ ); // connot be called if mono

private:
    std::vector<uint8> internalProcess();
    void read( std::vector<uint8> const & aInputStream );
    void write( std::vector<uint8> & aProcessedStream, int & aTail, int aElemCount );

    soundtouch::SoundTouch mSoundTouch;

    // must be provided by users
    int mBitsPerSample;
    int mChannel;
    int mSampleRate;

    // process effect
    int mSemiTonesDelta;
    double mLeftChanVol;  // if the audio is mono, mLeftChanVol == mRightChanVol
    double mRightChanVol;

    // after souch touch processing the input samples,
    static int const OUTPUT_BUFFER_SIZE = 1024 * 100;

    // SAMPLETYPE is float, the mBufferForProcess is for input
    static int const PROCESS_BUFFER_SIZE = 1024 * 3;
    float mBufferForProcess[AudioTuner::PROCESS_BUFFER_SIZE];
};

#endif // AUDIOTUNER_HPP
