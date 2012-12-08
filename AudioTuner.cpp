#include <cassert>
#include <cstdio>
#include <cstring>
#include "AudioTuner.hpp"
#include "debug.hpp"

using namespace std;
using namespace soundtouch;

AudioTuner::AudioTuner()
{
}

void AudioTuner::init( int aChannel, int aSampleRate, int aBitsPerSample, bool aIsSpeechMode /* = false */ )
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
    mSoundTouch.setPitchSemiTones(aDelta);
}

// the inputStream bytes must below size of mBufferForProcess
vector<uint8> AudioTuner::processPitchShift( vector<uint8> const & inputStream )
{
    // soundtouch said that it may produce more bytes than input stream
    // so we use 90% to avoid overflow
    assert( inputStream.size() < BUFFER_SIZE * 0.9 );

    DEBUG() << "read";
    read( inputStream );
    int const sampleCount = inputStream.size() / ( mBitsPerSample / 8 ) / mChannel;
    DEBUG() << "putSample in soundtouch - sampleCount:" << sampleCount;
    mSoundTouch.putSamples( mBufferForProcess, sampleCount );
    DEBUG() << "after putSample in soundtouch";

    vector<uint8> processedStream( inputStream.size() * 100, 0 );
    int processedStreamTail = 0;

    int sampleProcessedCount;
    do
    {
        static float iter = -20000.0;
        iter += 1;
        mSoundTouch.setPitchSemiTones( iter / 42638.0f * 16 );
        DEBUG() << "sound touch recevie samples";
        DEBUG() << "ready samples :" << mSoundTouch.numSamples();
        sampleProcessedCount = mSoundTouch.receiveSamples(mBufferForProcess, BUFFER_SIZE / mChannel);
        DEBUG() << "write sample processed count: " << sampleProcessedCount;
        write( processedStream, processedStreamTail, sampleProcessedCount * mChannel);
    } while (sampleProcessedCount != 0);

    DEBUG() << "erase";
    processedStream.erase( processedStream.begin() + processedStreamTail, processedStream.end() );
    DEBUG() << "after erase";
    return processedStream;
}

vector<unsigned char> AudioTuner::flushProcessBuffer()
{
    mSoundTouch.flush();
    int sampleProcessedCount;

    vector<uint8> processedStream( 1024 * 100, 0 );
    int processedStreamTail = 0;

    do
    {
        DEBUG() << "sound touch recevie samples";
        sampleProcessedCount = mSoundTouch.receiveSamples(mBufferForProcess, BUFFER_SIZE / mChannel);
        DEBUG() << "write sample processed count: " << sampleProcessedCount;
        write( processedStream, processedStreamTail, sampleProcessedCount * mChannel);
    } while (sampleProcessedCount != 0);

    DEBUG() << "erase";
    processedStream.erase( processedStream.begin() + processedStreamTail, processedStream.end() );
    DEBUG() << "after erase";
    return processedStream;
}

// mBufferForProcess ( pitch shifted stream ) => first parameter
// aProcessedStream: output stream
// aTail: output stream tail ( we append processed stream at tail index )
// aElemCount: elem count in mBufferForProcess
void AudioTuner::write( vector<uint8> & aProcessedStream, int & aTail, int aElemCount )
{
    DEBUG() << "output size: " << aProcessedStream.size() << " aTail: " << aTail << " aElemCount: " << aElemCount;
    if ( mBitsPerSample == 8 )
    {
        for ( int i = 0; i < aElemCount; ++i )
        {
            int intTemp = (int)(mBufferForProcess[i] * 32768.0f);
            // saturate
            if (intTemp < -32768) intTemp = -32768;
            if (intTemp > 32767)  intTemp = 32767;
            uint8 uint8Temp = (uint8)(intTemp >> 8);
            memcpy( &aProcessedStream[aTail], &uint8Temp, sizeof( uint8 ) );
            aTail += sizeof( uint8 );
        }
    }
    else if ( mBitsPerSample == 16 )
    {
        for ( int i = 0; i < aElemCount; ++i )
        {
            int intTemp = (int)(mBufferForProcess[i] * 32768.0f);
            // saturate
            if (intTemp < -32768) intTemp = -32768;
            if (intTemp > 32767)  intTemp = 32767;
            short shortTemp = (short)intTemp;
            memcpy( &aProcessedStream[aTail], &shortTemp, sizeof( short ) );
            aTail += sizeof( short );
        }
    }
    else
    {
        assert( false );
    }
}

// original stream => mBufferForProcess
void AudioTuner::read( vector<uint8> const & inputStream )
{
    if ( mBitsPerSample == 8 )
    {
        for ( int i = 0; i < inputStream.size(); ++i )
        {
            mBufferForProcess[i] = ((short)inputStream[i] << 8) * 1.0 / 32768.0;
        }
    }
    else if ( mBitsPerSample == 16 )
    {
        assert( inputStream.size() % 2 == 0 );
        for ( int i = 0; i < inputStream.size() / 2; ++i )
        {
            // mBufferForProcess[i] = (((short)inputStream[2*i] << 8) + inputStream[2*i+1]) * 1.0 / 32768.0;
            mBufferForProcess[i] = (*(short*)&inputStream[2*i]) * 1.0 / 32768.0;
        }
    }
    else
    {
        assert( false );
    }
}
