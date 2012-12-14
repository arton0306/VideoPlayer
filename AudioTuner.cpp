#include <cassert>
#include <cstdio>
#include <cstring>
#include "AudioTuner.hpp"
#include "debug.hpp"

using namespace std;
using namespace soundtouch;

AudioTuner::AudioTuner()
    : mLeftChanVol(  1.0 )
    , mRightChanVol( 1.0 )
{
}

void AudioTuner::init( int aChannel, int aSampleRate, int aBitsPerSample )
{
    mChannel = aChannel;
    mSampleRate = aSampleRate;
    mBitsPerSample = aBitsPerSample;

    mSoundTouch.setSampleRate(aSampleRate);
    mSoundTouch.setChannels(aChannel);

    mSoundTouch.setTempoChange(0);
    mSoundTouch.setPitchSemiTones(0);
    mSoundTouch.setRateChange(0);

    mSoundTouch.setSetting(SETTING_USE_QUICKSEEK, 0);  // disable quickseek
    mSoundTouch.setSetting(SETTING_USE_AA_FILTER, 1);  // enable anti alias
}

void AudioTuner::setSpeechMode( bool aIsSpeechMode )
{
    /* output: 0 0 8
    DEBUG() << mSoundTouch.getSetting(SETTING_SEQUENCE_MS)
            << mSoundTouch.getSetting(SETTING_SEEKWINDOW_MS)
            << mSoundTouch.getSetting(SETTING_OVERLAP_MS);
    */

    if ( aIsSpeechMode )
    {
        // reference sound stretch main.cpp
        mSoundTouch.setSetting(SETTING_SEQUENCE_MS, 40);
        mSoundTouch.setSetting(SETTING_SEEKWINDOW_MS, 15);
        mSoundTouch.setSetting(SETTING_OVERLAP_MS, 8);
    }
    else
    {
        mSoundTouch.setSetting(SETTING_SEQUENCE_MS, 0);
        mSoundTouch.setSetting(SETTING_SEEKWINDOW_MS, 0);
        mSoundTouch.setSetting(SETTING_OVERLAP_MS, 8);
    }
}

// in sound touch, it will bound it automatically if aDelta out of range
void AudioTuner::setPitchShiftInSemiTones( int aDelta /* -60 ~ +60 */ )
{
    mSemiTonesDelta = aDelta;
    mSoundTouch.setPitchSemiTones(aDelta);
}

vector<uint8> AudioTuner::process( vector<uint8> const & aInputStream )
{
    read( aInputStream );
    int const sampleCount = aInputStream.size() / ( mBitsPerSample / 8 ) / mChannel;
    mSoundTouch.putSamples( mBufferForProcess, sampleCount );
    return internalProcess();
}

vector<uint8> AudioTuner::internalProcess()
{
    vector<uint8> processedStream( OUTPUT_BUFFER_SIZE, 0 );
    int processedStreamTail = 0;

    int sampleProcessedCount;
    do
    {
        sampleProcessedCount = mSoundTouch.receiveSamples(mBufferForProcess, PROCESS_BUFFER_SIZE / mChannel);
        write( processedStream, processedStreamTail, sampleProcessedCount * mChannel);
    } while (sampleProcessedCount != 0);

    processedStream.erase( processedStream.begin() + processedStreamTail, processedStream.end() );
    return processedStream;
}

// some sample may left in sound touch internal buffer, we flush it and get them
// take main.c in soundtouch for reference
vector<uint8> AudioTuner::flush()
{
    mSoundTouch.flush();
    return internalProcess();
}

// mBufferForProcess ( pitch shifted stream ) => first parameter
// aProcessedStream: output stream
// aTail: output stream tail ( we append processed stream at tail index )
// aElemCount: elem count in mBufferForProcess
void AudioTuner::write( vector<uint8> & aProcessedStream, int & aTail, int aElemCount )
{
    if ( mBitsPerSample == 8 )
    {
        for ( int i = 0; i < aElemCount; ++i )
        {
            // fit to 8 bits
            int intTemp = (int)(mBufferForProcess[i] * 32768.0f);
            if (intTemp < -32768) intTemp = -32768;
            if (intTemp > 32767)  intTemp = 32767;
            uint8 uint8Temp = (uint8)(intTemp >> 8);
            // tuned by channel vol
            uint8Temp *= ( (i%2==0) ? mLeftChanVol : mRightChanVol );
            // write
            assert( aTail + sizeof( uint8 ) < OUTPUT_BUFFER_SIZE );
            memcpy( &aProcessedStream[aTail], &uint8Temp, sizeof( uint8 ) );
            aTail += sizeof( uint8 );
        }
    }
    else if ( mBitsPerSample == 16 )
    {
        for ( int i = 0; i < aElemCount; ++i )
        {
            // fit to 16 bits
            int intTemp = (int)(mBufferForProcess[i] * 32768.0f);
            if (intTemp < -32768) intTemp = -32768;
            if (intTemp > 32767)  intTemp = 32767;
            short shortTemp = (short)intTemp;
            // tuned by channel vol
            shortTemp *= ( (i%2==0) ? mLeftChanVol : mRightChanVol );
            // write
            assert( aTail + sizeof( short ) < OUTPUT_BUFFER_SIZE );
            memcpy( &aProcessedStream[aTail], &shortTemp, sizeof( short ) );
            aTail += sizeof( short );
        }
    }
    else if ( mBitsPerSample == 32 )
    {
        for ( int i = 0; i < aElemCount; ++i )
        {
            // tuned by channel vol
            mBufferForProcess[i] *= ( (i%2==0) ? mLeftChanVol : mRightChanVol );
            // write
            assert( aTail + sizeof( float ) < OUTPUT_BUFFER_SIZE );
            memcpy( &aProcessedStream[aTail], &mBufferForProcess[i], sizeof( float ) );
            aTail += sizeof( float );
        }
    }
    else
    {
        assert( false );
    }
}

// original stream => mBufferForProcess
void AudioTuner::read( vector<uint8> const & aInputStream )
{
    if ( mBitsPerSample == 8 )
    {
        for ( int i = 0; i < aInputStream.size(); ++i )
        {
            mBufferForProcess[i] = ((short)aInputStream[i] << 8) * 1.0 / 32768.0;
        }
    }
    else if ( mBitsPerSample == 16 )
    {
        assert( aInputStream.size() % 2 == 0 );
        for ( int i = 0; i < aInputStream.size() / 2; ++i )
        {
            mBufferForProcess[i] = (*(short*)&aInputStream[2*i]) * 1.0 / 32768.0;
        }
    }
    else if ( mBitsPerSample == 32 )
    {
        assert( aInputStream.size() % 4 == 0 );
        for ( int i = 0; i < aInputStream.size() / 4; ++i )
        {
            mBufferForProcess[i] = (*(float*)&aInputStream[4*i]);
        }
    }
    else
    {
        assert( false );
    }
}

void AudioTuner::setVol( double aPercent /* 0.0 ~ 1.0 */ )
{
    if ( aPercent > 1.0 ) aPercent = 1.0;
    if ( aPercent < 0.0 ) aPercent = 0.0;
    mLeftChanVol = aPercent;
    mRightChanVol = aPercent;
}

void AudioTuner::setLeftChanVol( double aPercent /* 0.0 ~ 1.0 */ )
{
    assert( mChannel != 1 );
    if ( aPercent > 1.0 ) aPercent = 1.0;
    if ( aPercent < 0.0 ) aPercent = 0.0;
    mLeftChanVol = aPercent;
}

void AudioTuner::setRightChanVol( double aPercent /* 0.0 ~ 1.0 */ )
{
    assert( mChannel != 1 );
    if ( aPercent > 1.0 ) aPercent = 1.0;
    if ( aPercent < 0.0 ) aPercent = 0.0;
    mRightChanVol = aPercent;
}
