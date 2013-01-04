#ifndef VOLUMETUNER_HPP
#define VOLUMETUNER_HPP

#include <vector>
#include "AudioPlayer.hpp"

typedef unsigned char uint8;

class VolumeTuner
{
    public:
        VolumeTuner( int ch, AudioPlayer::SampleFormat sampleFmt );
        void tune( std::vector<uint8> & stream ) const;
        void setLeftVol( double v );
        void setRightVol( double v );

    private:
        bool isFormatSupport( AudioPlayer::SampleFormat sampleFmt ) const;
        template <typename T> void tune_( std::vector<uint8> & stream ) const;

        // audio stream info
        int mChannel;
        AudioPlayer::SampleFormat mSampleFormat;

        // audio volume, if the audio is mono channel, mLeftVol is equal mRightVol
        double mLeftVol;   // 0.0 ~ 1.0
        double mRightVol;  // 0.0 ~ 1.0
};

#endif // VOLUMETUNER_HPP
