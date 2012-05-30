#ifndef AV_INFO_HPP
#define AV_INFO_HPP

class AVInfo
{
public:
    AVInfo
        (
        double aFps = 0.0,
        double aUsecs = 0.0,
        unsigned aAudioChannel = 0,
        unsigned aAudioSampleRate = 0,
        unsigned aAudioBitsPerSample = 0
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
