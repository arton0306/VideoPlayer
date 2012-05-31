#include "AVInfo.hpp"

AVInfo::AVInfo
    (
    double aFps /* = 0.0 */,
    double aUsecs /* = 0.0 */,
    unsigned aAudioChannel /* = 0.0 */,
    unsigned aAudioSampleRate /* = 0.0 */,
    unsigned aAudioBitsPerSample /* = 0.0 */
    )
    : mFps( aFps )
    , mUsecs( aUsecs )
    , mAudioChannel( aAudioChannel )
    , mAudioSampleRate( aAudioSampleRate )
    , mAudioBitsPerSample( aAudioBitsPerSample )
{}

double AVInfo::getFps() const
{
    return mFps;
}

double AVInfo::getUsecs() const
{
    return mUsecs;
}

unsigned AVInfo::getAudioChannel() const
{
    return mAudioChannel;
}

unsigned AVInfo::getAudioSampleRate() const
{
    return mAudioSampleRate;
}

unsigned AVInfo::getAudioBitsPerSample() const
{
    return mAudioBitsPerSample;
}

/*
AVInfo( AVInfo const & aAvInfo )
{
    mFps = aAvInfo.mFps;
    mUsecs = aAvInfo.mUsecs;
}
*/
