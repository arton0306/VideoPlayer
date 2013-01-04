#include "VolumeTuner.hpp"
#include <vector>
#include <cassert>

using namespace std;

VolumeTuner::VolumeTuner( int ch, AudioPlayer::SampleFormat sampleFmt )
{
    mChannel = ch;
    mSampleFormat = sampleFmt;
    mLeftVol = 1.0;
    mRightVol = 1.0;
}

void VolumeTuner::tune( vector<uint8> & stream ) const
{
    switch ( mSampleFormat )
    {
        case AudioPlayer::Float32: tune_<float>( stream ); break;
        case AudioPlayer::Int32:   tune_<long>( stream ); break;
        case AudioPlayer::Int16:   tune_<short>( stream ); break;
        case AudioPlayer::Int8:    tune_<char>( stream ); break;
        case AudioPlayer::UInt8:   tune_<unsigned char>( stream ); break;
        default:
            return;
    }
}

template <typename T>
void VolumeTuner::tune_( vector<uint8> & stream ) const
{
    T * data = reinterpret_cast<T*>( &stream[0] );
    int const sampleCount = stream.size() / sizeof(T);
    int const frameCount = sampleCount / mChannel;

    assert( stream.size() % sizeof(T) == 0 && sampleCount % mChannel == 0 );

    // if the audio is mono channel, mLeftVol is equal mRightVol
    std::vector<double> channelRatio;
    channelRatio.push_back( mLeftVol );
    if ( mChannel == 2 )
        channelRatio.push_back( mRightVol );

    for ( int i = 0; i < frameCount; ++i )
    {
        for ( int j = 0; j < mChannel; ++j )
        {
            data[ i * mChannel + j ] *= channelRatio[j];
        }
    }
}

bool VolumeTuner::isFormatSupport( AudioPlayer::SampleFormat sampleFmt ) const
{
    switch ( mSampleFormat )
    {
        case AudioPlayer::Float32:
        case AudioPlayer::Int32:
        case AudioPlayer::Int16:
        case AudioPlayer::Int8:
        case AudioPlayer::UInt8:
            return true;
        default:
            return false;
    }
}

void VolumeTuner::setLeftVol( double v )
{
    mLeftVol = v;
    if ( mLeftVol > 1.0 ) mLeftVol = 1.0;
    if ( mLeftVol < 0.0 ) mLeftVol = 0.0;
    if ( mChannel == 1 ) mRightVol = mLeftVol;
}

void VolumeTuner::setRightVol( double v )
{
    mRightVol = v;
    if ( mRightVol > 1.0 ) mRightVol = 1.0;
    if ( mRightVol < 0.0 ) mRightVol = 0.0;
    if ( mChannel == 1 ) mLeftVol = mRightVol;
}
