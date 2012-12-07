#include <cassert>
#include "AudioTuner.hpp"

using namespace std;
using namespace soundtouch;

AudioTuner::AudioTuner()
{
}

void AudioTuner::init( int aChannel, int aSampleRate, int aBitsPerSample, bool aIsSpeechMode /* = false */ )
{
    mChannel = aChannel;
    mSampleRate = aSampleRate;

    mSoundTouch.setSampleRate(aSampleRate);
    mSoundTouch.setChannels(aChannel);

    mSoundTouch.setTempoChange(0);
    mSoundTouch.setPitchSemiTones(0);
    mSoundTouch.setRateChange(0);

    mSoundTouch.setSetting(SETTING_USE_QUICKSEEK, 0);  // disable quickseek
    mSoundTouch.setSetting(SETTING_USE_AA_FILTER, 1);  // enable anti alias

    if ( aIsSpeechMode )
    {
        // use settings for speech processing
        mSoundTouch.setSetting(SETTING_SEQUENCE_MS, 40);
        mSoundTouch.setSetting(SETTING_SEEKWINDOW_MS, 15);
        mSoundTouch.setSetting(SETTING_OVERLAP_MS, 8);
    }
}

void AudioTuner::setPitchShiftInSemiTones( int aDelta /* -60 ~ +60 */ )
{
    mSemiTonesDelta = aDelta;
}

vector<uint8> AudioTuner::processPitchShift( vector<uint8> const & inputStream )
{
    vector<uint8> processedStream;

    read( inputStream );

    int const sampleCount = inputStream.size() / ( mBitsPerSample / 8 ) / mChannel;
    mSoundTouch.putSamples( mBufferForProcess, sampleCount );

    do
    {
        int sampleProcessedCount = mSoundTouch->receiveSamples(mBufferForProcess, BUFFER_SIZE / mChannel);
        write(*processedStream, sampleProcessedCount * nChannels);
    } while (sampleProcessedCount != 0);
}

void AudioTuner::write( vector<uint8> & aProcessedStream, int size )
{
    
}

void AudioTuner::read( vector<uint8> const & inputStream )
{
    if ( mBitsPerSample == 8 )
    {
        for ( int i = 0; i < inputStream.size(); ++i )
        {
            mBufferForProcess[i] = (double)( (short)inputStream[i] << 8 ) * 1.0 / 32768.0;
        }
    }
    else if ( mBitsPerSample == 16 )
    {
        assert( inputStream.size() % 2 == 0 );
        for ( int i = 0; i < inputStream.size() / 2; ++i )
        {
            mBufferForProcess[i] = (((short)inputStream[2*i] << 8) + inputStream[2*i+1]) * 1.0 / 32768.0;
        }
    }
    else
    {
        assert( false );
    }
}
