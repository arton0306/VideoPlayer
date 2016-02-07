#include <cassert>
#include <cstdio>
#include <algorithm>
#include <fstream>
#include <string>
#include <QThread>
extern "C" {
    #include <libavutil/imgutils.h>
}
#include "DecodeWorker.hpp"
#include "QtSleepHacker.hpp"
#include "debug.hpp"
#include "Sleep.hpp"

#define BITS_PER_BYTES 8

using namespace std;

static void interleave(uint8_t **data, uint8_t *outbuf, int channels,
    enum AVSampleFormat sample_fmt, int data_bytes)
{
    assert(av_sample_fmt_is_planar(sample_fmt));
    int sample_bytes = av_get_bytes_per_sample(sample_fmt);
    assert(data_bytes % (channels*sample_bytes) == 0);
    int chn_bytes = data_bytes / channels;
    for (int i = 0; i < chn_bytes; i+=sample_bytes) {
        for (int chn = 0; chn < channels; chn++) {
            memcpy(outbuf, data[chn]+i, sample_bytes);
            outbuf += sample_bytes;
        }
    }
}

DecodeWorker::DecodeWorker(QObject *parent)
    : QObject(parent)
    , mFirstAudioFrameTime( 0.0 )
    , mIsReceiveStopSignal( false )
    , mIsReceiveSeekSignal( false )
    , mSeekMSec( 0 )
    , mIsDecoding( false )
    , mDoDumpAV( false )
    , mIsSpeechMode( false )
    , mSemiTonesDelta( 0 )
{
}

vector<uint8> DecodeWorker::genFramePPM( AVFrame *aDecodedFrame, int width, int height )
{
    // Write ppm header to a temp buffer
    char ppmHeader[30];
    int const headerSize = sprintf( ppmHeader, "P6\n%d %d\n255\n", width, height );

    // Write ppm totally
    int const contentSize = height * width * 3; // =aDecodedFrame->linesize[0]
    int const ppmSize = headerSize + contentSize;
    assert( ppmSize != 0 );
    vector<uint8> decodedStream( ppmSize, 0 );
    memcpy( &decodedStream[0], ppmHeader, headerSize );
    memcpy( &decodedStream[0] + headerSize, aDecodedFrame->data[0], contentSize );

    return decodedStream;
}

// can be called by player thread
vector<uint8> DecodeWorker::popAllAudioStream()
{
    int frameCount = mAudioFifo.getCount();

    vector<uint8> result;
    while ( frameCount-- )
    {
        vector<uint8> oneAudioStream = mAudioFifo.pop();
        result.insert( result.end(), oneAudioStream.begin(), oneAudioStream.end() );
    }

    return result;
}

// can be called by player thread
vector<uint8> DecodeWorker::popNextVideoFrame()
{
    if ( mVideoFifo.getCount() > 0 )
    {
        return mVideoFifo.pop();
    }
    else
    {
        vector<uint8> empty;
        return empty;
    }
}

// can be called by player thread
double DecodeWorker::getNextVideoFrameSecond() const
{
    return mVideoFifo.getFrontFrameSecond();
}

// can be called by player thread
void DecodeWorker::dropNextVideoFrame()
{
    if ( mVideoFifo.getCount() > 0 )
    {
        mVideoFifo.pop();
    }
}

// can be called by other thread
void DecodeWorker::seek( int aMSec )
{
    mSeekMSec = aMSec;
    mIsReceiveSeekSignal = true;
}

void DecodeWorker::dumpAVStream()
{
    mDoDumpAV = true;
}

// can be called by player thread
// decode thread will check mIsReceiveStopSignal in each decoding round,
// if the flag is true, it clear all av buffer and break out of the decoding loop.
void DecodeWorker::stopDecoding()
{
    if ( mIsDecoding ) {
        mIsReceiveStopSignal = true;
    }
}

// can be called by player thread
// audio effect
void DecodeWorker::setSpeechMode( bool aIsSpeechMode ) { mIsSpeechMode = aIsSpeechMode; }
void DecodeWorker::setPitchSemiTones( int aDelta /* -60 ~ +60 */ ) { mSemiTonesDelta = aDelta; }

void DecodeWorker::setFileName( QString aFileName )
{
    mFileName = aFileName;
}

int DecodeWorker::getStreamIndex( AVFormatContext * aFormatCtx, AVMediaType aMediaType )
{
    for ( unsigned i = 0; i < aFormatCtx->nb_streams; ++i ) {
        if ( aFormatCtx->streams[i]->codec->codec_type == aMediaType ) {
            return i;
        }
    }
    return -1;
}

AVCodecContext * DecodeWorker::getCodecCtx( AVFormatContext * aFormatCtx, int aStreamIndex )
{
    AVCodecContext * codecCtx = aFormatCtx->streams[aStreamIndex]->codec;
    AVCodec * codec = avcodec_find_decoder( codecCtx->codec_id );
    assert(codec);
    int err;
    err = avcodec_open2( codecCtx, codec, NULL );
    assert(err >= 0);

    return codecCtx;
}

void DecodeWorker::decodeAudioVideo( QString aFileName )
{
    assert( mVideoFifo.getCount() == 0 );
    assert( mAudioFifo.getCount() == 0 );

    mIsDecoding = true;
    setFileName( aFileName );

    int err;

    av_register_all();
    AVFormatContext * formatCtx = NULL;
    err = avformat_open_input( &formatCtx, mFileName.toStdString().c_str(), NULL, NULL );
    assert(err == 0);
    err = avformat_find_stream_info( formatCtx, NULL );
    assert(err >= 0);

    int const videoStreamIndex = getStreamIndex( formatCtx, AVMEDIA_TYPE_VIDEO );
    int const audioStreamIndex = getStreamIndex( formatCtx, AVMEDIA_TYPE_AUDIO );
    double videoTimeBase = av_q2d(formatCtx->streams[videoStreamIndex]->time_base);
    double audioTimeBase = av_q2d(formatCtx->streams[audioStreamIndex]->time_base);
    AVCodecContext * videoCodecCtx = getCodecCtx( formatCtx, videoStreamIndex );
    AVCodecContext * audioCodecCtx = getCodecCtx( formatCtx, audioStreamIndex );
    DEBUG() << "video time base:" << videoTimeBase;
    DEBUG() << "audio time base:" << audioTimeBase;

    AVFrame *decodedFrame = av_frame_alloc();
    AVFrame *pFrameRGB = av_frame_alloc();
    assert( decodedFrame && pFrameRGB );

    uint8_t * buffer = (uint8_t *)av_malloc( av_image_get_buffer_size( AV_PIX_FMT_RGB24, videoCodecCtx->width, videoCodecCtx->height, 1 ) );

    avpicture_fill( (AVPicture *)pFrameRGB, buffer, AV_PIX_FMT_RGB24,
        videoCodecCtx->width, videoCodecCtx->height );

    int frameFinished;
    AVPacket packet;

    int videoFrameIndex = 0;
    int audioFrameIndex = 0;
    int pktIndex = 0;
    double const fps = av_q2d( formatCtx->streams[videoStreamIndex]->avg_frame_rate );
    //DEBUG() << "video fps:" << fps;
    //DEBUG() << "videoStreamIndex:" << videoStreamIndex << "    audioStreamIndex:" << audioStreamIndex;

    audioCodecCtx->sample_fmt;
    DEBUG() << audioCodecCtx->sample_fmt << endl;

    mAudioTuner.init(
        audioCodecCtx->channels,
        audioCodecCtx->sample_rate,
        av_get_bytes_per_sample(audioCodecCtx->sample_fmt) * BITS_PER_BYTES
        );
    mAudioTuner.flush();

    AVInfo avInfo(
        fps,
        formatCtx->duration,
        audioCodecCtx->channels,
        audioCodecCtx->sample_fmt,
        audioCodecCtx->sample_rate,
        av_get_bytes_per_sample( audioCodecCtx->sample_fmt ) * BITS_PER_BYTES
        );
    if ( mDoDumpAV ) saveAVInfoToFile( avInfo, (mFileName + ".avinfo.txt").toStdString().c_str());
    readyToDecode( avInfo );

    bool is_seek_or_new_play = true;

    while ( true )
    {
        bool stop_flag = false;

        if ( mIsReceiveStopSignal )
        {
            mVideoFifo.clear();
            mAudioFifo.clear();
            mIsReceiveStopSignal = false;
            stop_flag = true;
        }

        // determine whether seek or not
        if ( mIsReceiveSeekSignal )
        {
            int ret = avformat_seek_file( formatCtx, -1,
                    INT64_MIN, (double)mSeekMSec / 1000 * AV_TIME_BASE, INT64_MAX, 0);
            DEBUG() << "seek " << (double)mSeekMSec / 1000
                    << ", result:" << ret;
            if ( ret < 0 )
            {
                seekState( false );
                is_seek_or_new_play = false;
                DEBUG() << "seek failed";
            }
            else
            {
                seekState( true );
                mAudioTuner.flush();
                is_seek_or_new_play = true;
                mVideoFifo.clear();
                mAudioFifo.clear();
            }
            mIsReceiveSeekSignal = false;
        }

        if ( av_read_frame( formatCtx, &packet ) < 0 || stop_flag )
        {
            // there may some audio samples in soundtouch internal buffer, but we ignore them
            break;
        }

        ++pktIndex;

        if ( packet.stream_index == videoStreamIndex )
        {
            int const bytesUsed = avcodec_decode_video2( videoCodecCtx, decodedFrame, &frameFinished, &packet );

            if ( frameFinished )
            {
                double const dtsSec = packet.dts * videoTimeBase;
                convertToRGBFrame( videoCodecCtx, decodedFrame, pFrameRGB );
                vector<uint8> ppmFrame =
                    genFramePPM( pFrameRGB, videoCodecCtx->width, videoCodecCtx->height );
                if ( mDoDumpAV ) {
                    genFilePPM(&ppmFrame[0], ppmFrame.size(), dtsSec);
                }

                mVideoFifo.push( ppmFrame, dtsSec );

                DEBUG() << "p ndx:" << pktIndex
                        << "\t video frame ndx:" << videoFrameIndex
                        << "\t PTS:" << packet.pts
                        << "\t DTS:" << packet.dts
                        << "\t *dts: " << dtsSec;

                ++videoFrameIndex;
            }
            else
            {
                DEBUG() << "p ndx:" << pktIndex << "    (unfinished video packet)";
            }

            av_free_packet( &packet );
        }
        else if ( packet.stream_index == audioStreamIndex )
        {
            av_frame_unref( decodedFrame );
            uint8_t * const packetDataHead = packet.data;
            while ( packet.size > 0 )
            {
                int const bytesUsed = avcodec_decode_audio4( audioCodecCtx, decodedFrame, &frameFinished, &packet );
                if ( bytesUsed < 0 )
                {
                    fprintf( stderr, "Error while decoding audio!\n" );
                    assert( false );
                }
                else
                {
                    if ( frameFinished )
                    {
                        int const data_size = av_samples_get_buffer_size(NULL, audioCodecCtx->channels,
                            decodedFrame->nb_samples,
                            audioCodecCtx->sample_fmt, 1);

                        vector<uint8> decodedStream( data_size, 0 );
                        interleave(decodedFrame->data, &decodedStream[0],
                            audioCodecCtx->channels, audioCodecCtx->sample_fmt, data_size);
                        setAudioEffect( audioCodecCtx->channels );

                        if ( mDoDumpAV ) {
                            appendAudioPcmToFile( &decodedStream[0], data_size, (mFileName + ".pcm").toStdString().c_str() );
                        }

                        if ( mAudioFifo.isEmpty() )
                            mFirstAudioFrameTime = audioTimeBase * packet.pts;

                        bool do_pitch_shift = true;
                        if (do_pitch_shift) {
                            decodedStream = mAudioTuner.process( decodedStream );
                        }
                        if ( decodedStream.size() != 0 ) {
                            mAudioFifo.push( decodedStream, 0.0 /* dummy */ );
                        }

                        DEBUG() << "p ndx:" << pktIndex
                                << "\t audio frame ndx:" << audioFrameIndex
                                << "\t PTS:" << packet.pts
                                << "\t DTS:" << packet.dts
                                << "\t *dts:" << audioTimeBase * packet.pts;

                        ++audioFrameIndex;
                    }
                    else
                    {
                        DEBUG() << "p ndx:" << pktIndex << "    (unfinished audio packet)";
                    }
                    packet.data += bytesUsed;
                    packet.size -= bytesUsed;
                }
            }
            packet.data = packetDataHead;
            if (packet.data)
            {
                av_free_packet( &packet );
            }
        }
        else
        {
            av_free_packet( &packet );
        }

        // determine whether decoded frame is enough and determine whether interrupt signal received
        while ( isAvFrameEnough( fps ) && !mIsReceiveStopSignal && !mIsReceiveSeekSignal )
        {
            if ( is_seek_or_new_play )
            {
                initAVFrameReady( mFirstAudioFrameTime * 1000);
                is_seek_or_new_play = false;
            }
            Sleep::msleep( 1 );
        }
    }

    mIsDecoding = false;
    decodeDone();

    av_free( buffer );
    av_free( pFrameRGB );
    av_free( decodedFrame );
    avcodec_close( videoCodecCtx );
    avcodec_close( audioCodecCtx );
    avformat_close_input( &formatCtx );
}

void DecodeWorker::convertToRGBFrame( AVCodecContext * videoCodecCtx, AVFrame * decodedFrame, AVFrame * pFrameRGB )
{
    struct SwsContext *pConvertedSwsCtx = NULL;

    pConvertedSwsCtx = sws_getContext(
        videoCodecCtx->width, videoCodecCtx->height, videoCodecCtx->pix_fmt,
        videoCodecCtx->width, videoCodecCtx->height, AV_PIX_FMT_RGB24,
        SWS_BILINEAR, NULL, NULL, NULL );

    sws_scale( pConvertedSwsCtx,
        (const uint8_t * const *)(decodedFrame->data), decodedFrame->linesize,
        0, videoCodecCtx->height,
        pFrameRGB->data, pFrameRGB->linesize );

    sws_freeContext( pConvertedSwsCtx );
}

void DecodeWorker::appendAudioPcmToFile( void const * aPcmBuffer, int aPcmSize, char const * aFileName )
{
    FILE * outfile = fopen( aFileName, "ab" );
    assert( outfile );
    fwrite( aPcmBuffer, 1, aPcmSize, outfile );
    fclose( outfile );
}

void DecodeWorker::genFilePPM(uint8_t *buf, int len, double dtsSec)
{
    char ts[12];
    snprintf(ts, 12, "%08d", int(dtsSec*1000));
    string filename = mFileName.toStdString() + "." + ts + ".ppm";

    FILE * outfile = fopen( filename.c_str(), "wb" );
    assert( outfile );
    fwrite( buf, 1, len, outfile );
    fclose( outfile );
}

void DecodeWorker::saveAVInfoToFile( AVInfo const & aAVInfo, char const * aFileName )
{
    fstream fp;
    fp.open( aFileName, ios::out );
    assert( fp );
    fp << "fps:" << aAVInfo.getFps() << endl;
    fp << "audio channel:" << aAVInfo.getAudioChannel() << endl;
    fp << "audio sample rate:" << aAVInfo.getAudioSampleRate() << endl;
    fp << "audio bits per sample:" << aAVInfo.getAudioBitsPerSample() << endl;
    fp << "av length in msec:" << fixed << aAVInfo.getUsecs() / 1000.0 << endl;
    fp.close();
}

bool DecodeWorker::isAvFrameEnough( double a_fps ) const
{
    // video: at least 2 frames
    // audio: 16KB data, in 48000Hz, 2ch, s16, can play 0.085 second
    // this condition will consume lots mem because we'll read many video frame in order to get audio frames
    // this change due to soundtouch output stream at one go, and portaudio's callback need data ASAP

    return mVideoFifo.getCount()>1 && mAudioFifo.getBytes()>16*1024;
    // return mVideoFifo.getCount()>1 && mAudioFifo.getCount()>1;

}

void DecodeWorker::init()
{
    mVideoFifo.clear();
    mAudioFifo.clear();
}

void DecodeWorker::setAudioEffect( int aChannel )
{
    mAudioTuner.setSpeechMode( mIsSpeechMode );
    mAudioTuner.setPitchSemiTones( mSemiTonesDelta );
}
