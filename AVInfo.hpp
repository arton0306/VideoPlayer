#ifndef AV_INFO_HPP
#define AV_INFO_HPP

class AVInfo
{
public:
    AVInfo
        (
        double aFps,
        double aUsecs,
        unsigned aAudioChannel,
        unsigned aAudioSampleRate,
        unsigned aAudioBitsPerSample
        );
    double getFps() const;
    double getUsecs() const;
    unsigned getAudioChannel() const;
    unsigned getAudioSampleRate() const;
    unsigned getAudioBitsPerSample() const;
    // AVInfo( AVInfo const & aAvInfo );

private:
    double mFps;
    double mUsecs;
    unsigned mAudioChannel;
    unsigned mAudioSampleRate;
    unsigned mAudioBitsPerSample;
};

#endif // AV_INFO_HPP
