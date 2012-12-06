#include "AudioTuner.hpp"

using namespace soundtouch;

AudioTuner::AudioTuner()
{
}

void AudioTuner::init( int aChannel, int aSampleRate, bool aIsSpeechMode /* = false */ )
{
    mSoundTouch->setSampleRate(aSampleRate);
    mSoundTouch->setChannels(aChannel);

    mSoundTouch->setTempoChange(0);
    mSoundTouch->setPitchSemiTones(0);
    mSoundTouch->setRateChange(0);

    mSoundTouch->setSetting(SETTING_USE_QUICKSEEK, 0);  // disable quickseek
    mSoundTouch->setSetting(SETTING_USE_AA_FILTER, 1);  // enable anti alias

    if ( aIsSpeechMode )
    {
        // use settings for speech processing
        mSoundTouch->setSetting(SETTING_SEQUENCE_MS, 40);
        mSoundTouch->setSetting(SETTING_SEEKWINDOW_MS, 15);
        mSoundTouch->setSetting(SETTING_OVERLAP_MS, 8);
    }
}

bool AudioTuner::shiftPitch( int aSemiTones /* -60 ~ +60 */ )
{
    
}
