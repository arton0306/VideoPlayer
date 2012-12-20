#ifndef AV_INFO_HPP
#define AV_INFO_HPP

extern "C" {
#ifdef __cplusplus
    #define __STDC_CONSTANT_MACROS
    #ifdef _STDINT_H
        #undef _STDINT_H
    #endif
    #include <stdint.h>
#endif
#include "libavutil/samplefmt.h"
}
#include "AudioPlayer.hpp"

class AVInfo
{
public:
    AVInfo
        (
        double aFps = 0.0,
        double aUsecs = 0.0,
        unsigned aAudioChannel = 0,
        AVSampleFormat aSampleFormat = AV_SAMPLE_FMT_NONE,
        unsigned aAudioSampleRate = 0,
        unsigned aAudioBitsPerSample = 0
        );
    double getFps() const;
    double getUsecs() const;
    unsigned getAudioChannel() const;
    AudioPlayer::SampleFormat getAudioSampleFormat() const;
    unsigned getAudioSampleRate() const;
    unsigned getAudioBitsPerSample() const;
    void dump() const;

private:
    AudioPlayer::SampleFormat turnLibavSampleFormat( AVSampleFormat aFormat );

    double mFps;
    double mUsecs;
    unsigned mAudioChannel;
    AudioPlayer::SampleFormat mAudioSampleFormat;
    unsigned mAudioSampleRate;
    unsigned mAudioBitsPerSample;
};

#endif // AV_INFO_HPP
