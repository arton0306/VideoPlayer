AVInfo:AVInfo
    (
    double aFps,
    double aUsecs,
    unsigned aAudioChannel,
    unsigned aAudioSampleRate,
    unsigned aAudioBitsPerSample
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
