#ifndef AUDIOTUNER_HPP
#define AUDIOTUNER_HPP

#include "SoundTouch.h"

class AudioTuner
{
public:
    AudioTuner();
    void init( int aChannel, int aSampleRate, bool aIsSpeechMode = false );
    bool shiftPitch( int aSemiTones /* -60 ~ +60 */ );

private:
    soundtouch::SoundTouch mSoundTouch;
};

#endif // AUDIOTUNER_HPP
