#include <cassert>
#include <algorithm>
#include <fstream>
#include <QThread>
#include "LibavWorker.hpp"
#include "QtSleepHacker.hpp"
#include "debug.hpp"
#include "Sleep.hpp"

#define BITS_PER_BYTES 8

using namespace std;

LibavWorker::LibavWorker(QObject *parent)
    : QObject(parent)
    , mIsReceiveStopSignal( false )
    , mIsReceiveSeekSignal( false )
    , mSeekMSec( 0 )
    , mIsDecoding( false )
    , isAvDumpNeeded( false )
    , mIsSpeechMode( false )
    , mSemiTonesDelta( 0 )
{
}

vector<uint8> LibavWorker::convertToPpmFrame( AVFrame *aDecodedFrame, int width, int height )
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
vector<uint8> LibavWorker::popAllAudioStream()
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
vector<uint8> LibavWorker::popNextVideoFrame()
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
double LibavWorker::getNextVideoFrameSecond() const
{
    return mVideoFifo.getFrontFrameSecond();
}

// can be called by player thread
void LibavWorker::dropNextVideoFrame()
{
    if ( mVideoFifo.getCount() > 0 )
    {
        mVideoFifo.pop();
    }
}

// can be called by other thread
void LibavWorker::seek( int aMSec )
{
    mSeekMSec = aMSec;
    mIsReceiveSeekSignal = true;
}

void LibavWorker::dumpAVStream()
{
    isAvDumpNeeded = true;
}

// can be called by player thread
// decode thread will check mIsReceiveStopSignal in each decoding round,
// if the flag is true, it clear all av buffer and break out of the decoding loop.
void LibavWorker::stopDecoding()
{
    if ( mIsDecoding )
    {
        mIsReceiveStopSignal = true;
    }
}

// can be called by player thread
// audio effect
void LibavWorker::setSpeechMode( bool aIsSpeechMode ) { mIsSpeechMode = aIsSpeechMode; }
void LibavWorker::setPitchSemiTones( int aDelta /* -60 ~ +60 */ ) { mSemiTonesDelta = aDelta; }

void LibavWorker::setFileName( QString aFileName )
{
    mFileName = aFileName;
}

int LibavWorker::readHeader( AVFormatContext ** aFormatCtx )
{
    if ( avformat_open_input( aFormatCtx, mFileName.toStdString().c_str(), NULL, NULL ) != 0 )
    {
        assert( false );
        return -1;
    }
    else
    {
        return 1;
    }
}

int LibavWorker::retrieveStreamInfo( AVFormatContext * aFormatCtx )
{
    if ( avformat_find_stream_info( aFormatCtx, NULL ) < 0 )
    {
        assert( false );
        return -1;
    }
    else
    {
        return 1;
    }
}

int LibavWorker::getStreamIndex( AVFormatContext * aFormatCtx, AVMediaType aMediaType )
{
    for ( unsigned streamIndex = 0; streamIndex < aFormatCtx->nb_streams; ++streamIndex )
    {
        if ( aFormatCtx->streams[streamIndex]->codec->codec_type == aMediaType )
        {
            return streamIndex;
        }
    }
    return -1;
}

AVCodecContext * LibavWorker::getCodecCtx( AVFormatContext * aFormatCtx, int aStreamIndex )
{
    AVCodecContext * codecCtx = aFormatCtx->streams[aStreamIndex]->codec;
    AVCodec * codec = avcodec_find_decoder( codecCtx->codec_id );
    if ( codec == NULL )
    {
        assert( false );
    }

    if ( avcodec_open2( codecCtx, codec, NULL ) < 0 )
    {
        assert( false );
    }

    return codecCtx;
}

void LibavWorker::decodeAudioVideo( QString aFileName )
{
    assert( mVideoFifo.getCount() == 0 );
    assert( mAudioFifo.getCount() == 0 );

    mIsDecoding = true;

    /******************************************
                    Codec Init
    ******************************************/
    // set file name
    setFileName( aFileName );

    // Register all codec
    av_register_all();

    // Declare a decode context
    AVFormatContext * formatCtx = NULL;

    // readHeader
    readHeader( &formatCtx );

    // Retrieve stream information
    retrieveStreamInfo( formatCtx );

    // Get video/audio stream index
    int const videoStreamIndex = getStreamIndex( formatCtx, AVMEDIA_TYPE_VIDEO );
    int const audioStreamIndex = getStreamIndex( formatCtx, AVMEDIA_TYPE_AUDIO );

    // Set video/audio decoder
    AVCodecContext * videoCodecCtx = getCodecCtx( formatCtx, videoStreamIndex );
    AVCodecContext * audioCodecCtx = getCodecCtx( formatCtx, audioStreamIndex );

    /******************************************
                Frame & Buffer Init
    ******************************************/

    // Declare frame
    AVFrame *decodedFrame = avcodec_alloc_frame();
    AVFrame *pFrameRGB = avcodec_alloc_frame();
    assert( decodedFrame != NULL && pFrameRGB != NULL );

    // Create a buffer for converted frame
    uint8_t * buffer = (uint8_t *)av_malloc( avpicture_get_size( PIX_FMT_RGB24, videoCodecCtx->width, videoCodecCtx->height ) );

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
    // of AVPicture
    avpicture_fill( (AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24,
        videoCodecCtx->width, videoCodecCtx->height );

    /******************************************
                Reading the Data
    ******************************************/
    int frameFinished;
    AVPacket packet;

    int videoFrameIndex = 0;
    int audioFrameIndex = 0;
    int packetIndex = 0;
    double const fps = av_q2d( formatCtx->streams[videoStreamIndex]->avg_frame_rate );
    //DEBUG() << "video fps:" << fps;
    //DEBUG() << "videoStreamIndex:" << videoStreamIndex << "    audioStreamIndex:" << audioStreamIndex;

    /* libav audio sample format ( ref: libav\libavutil\samplefmt.h )
    enum AVSampleFormat {
        AV_SAMPLE_FMT_NONE = -1,
        AV_SAMPLE_FMT_U8,          ///< unsigned 8 bits
        AV_SAMPLE_FMT_S16,         ///< signed 16 bits
        AV_SAMPLE_FMT_S32,         ///< signed 32 bits
        AV_SAMPLE_FMT_FLT,         ///< float
        AV_SAMPLE_FMT_DBL,         ///< double

        AV_SAMPLE_FMT_U8P,         ///< unsigned 8 bits, planar
        AV_SAMPLE_FMT_S16P,        ///< signed 16 bits, planar
        AV_SAMPLE_FMT_S32P,        ///< signed 32 bits, planar
        AV_SAMPLE_FMT_FLTP,        ///< float, planar
        AV_SAMPLE_FMT_DBLP,        ///< double, planar

        AV_SAMPLE_FMT_NB           ///< Number of sample formats. DO NOT USE if linking dynamically
    };
    */

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
    if ( isAvDumpNeeded ) saveAVInfoToFile( avInfo, (mFileName + ".avinfo.txt").toStdString().c_str());
    readyToDecode( avInfo );

    bool is_new_decode_request = true; // true if the users want to decode from new position ( seek or new play )

    while ( true )
    {
        bool stop_flag = false;

        // determine whether be forced stop
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
            const long long INT64_MIN = (-0x7fffffffffffffffLL - 1);
            const long long INT64_MAX = (9223372036854775807LL);
            // timestamp = seconds * AV_TIME_BASE
            int ret = avformat_seek_file( formatCtx, -1, INT64_MIN, (double)mSeekMSec / 1000 * AV_TIME_BASE, INT64_MAX, 0);
            DEBUG() << "============================================================ seek " << (double)mSeekMSec / 1000 << " return:" << ret;
            if ( ret < 0 )
            {
                seekState( false );
                is_new_decode_request = false;
                DEBUG() << "============================================================ seek fail";
            }
            else
            {
                seekState( true );
                mAudioTuner.flush();
                is_new_decode_request = true;
                mVideoFifo.clear();
                mAudioFifo.clear();
            }
            mIsReceiveSeekSignal = false;
        }

        // read a frame
        if ( av_read_frame( formatCtx, &packet ) < 0 || stop_flag )
        {
            // there may some audio samples in soundtouch internal buffer, but we ignore them
            break;
        }

        // the index is just for debug
        ++packetIndex;

        // Is this packet from the video stream?
        if ( packet.stream_index == videoStreamIndex )
        {
            // Decode video frame
            int const bytesUsed = avcodec_decode_video2( videoCodecCtx, decodedFrame, &frameFinished, &packet );
            if ( bytesUsed != packet.size )
            {
                // check if one packet is corresponding to one frame
                // DEBUG() << bytesUsed << " " << packet.size;
            }

            // Did we get a video frame?
            if ( frameFinished )
            {
                double const dtsSec = packet.dts * av_q2d(formatCtx->streams[videoStreamIndex]->time_base );
                convertToRGBFrame( videoCodecCtx, decodedFrame, pFrameRGB );
                vector<uint8> ppmFrame = convertToPpmFrame( pFrameRGB, videoCodecCtx->width, videoCodecCtx->height );
                if ( isAvDumpNeeded )
                {
                    QString dtsMsecString = QString("%1").arg(int(dtsSec*1000));
                    QString ppmFileName = mFileName + "." + QString( 8 - dtsMsecString.size(), '0' ) + dtsMsecString + ".ppm";
                    saveVideoPpmToFile( ppmFrame, ppmFileName.toStdString().c_str());
                }

                // fill in our ppm buffer
                mVideoFifo.push( ppmFrame, dtsSec );

                // Dump pts and dts for debug
                DEBUG() << "p ndx:" << packetIndex << "    video frame ndx:" << videoFrameIndex << "     PTS:" << packet.pts << "     DTS:" << packet.dts << " TimeBase:" << av_q2d(formatCtx->streams[videoStreamIndex]->time_base) << " *dts: " << dtsSec;
                ++videoFrameIndex;
            }
            else
            {
                DEBUG() << "p ndx:" << packetIndex << "    (unfinished video packet)";
            }

            // Free the packet that was allocated by av_read_frame
            av_free_packet( &packet );
        }
        else if ( packet.stream_index == audioStreamIndex )
        {
            avcodec_get_frame_defaults( decodedFrame );
            uint8_t * const packetDataHead = packet.data;
            while ( packet.size > 0 )
            {
                // Decod audio frame
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

                        // apply audio effect and push audio data to fifo
                        // notice that we don't push meaningful time value with the stream into the fifo
                        vector<uint8> decodedStream( data_size, 0 );
                        memcpy( &decodedStream[0], decodedFrame->data[0], data_size );
                        setAudioEffect( audioCodecCtx->channels );

                        static double firstAudioFrameTime = 0.0;
                        if ( mAudioFifo.isEmpty() )
                            firstAudioFrameTime = av_q2d(formatCtx->streams[audioStreamIndex]->time_base) * packet.pts;

                        vector<uint8> tunedAudioStream = mAudioTuner.process( decodedStream );
                        if ( tunedAudioStream.size() != 0 )
                            mAudioFifo.push( tunedAudioStream, firstAudioFrameTime ); // the non-first audio time frame is useless
                        //mAudioFifo.push( decodedStream, firstAudioFrameTime ); // the non-first audio time frame is useless

                        if ( isAvDumpNeeded )
                            appendAudioPcmToFile( decodedFrame->data[0], data_size, (mFileName + ".pcm").toStdString().c_str() ); // this will spend lots time, which will cause the delay in video

                        DEBUG() << "p ndx:" << packetIndex << "     audio frame ndx:" << audioFrameIndex << "     PTS:" << packet.pts << "     DTS:" << packet.dts << " TimeBase:" << av_q2d(formatCtx->streams[audioStreamIndex]->time_base) << " *dts:" << av_q2d(formatCtx->streams[audioStreamIndex]->time_base) * packet.pts;
                        ++audioFrameIndex;
                    }
                    else
                    {
                        DEBUG() << "p ndx:" << packetIndex << "    (unfinished audio packet)";
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
            // Free the packet that was allocated by av_read_frame
            av_free_packet( &packet );
            // DEBUG() << "p ndx:" << packetIndex << "     packet.stream_index:" << packet.stream_index;
        }

        // determine whether decoded frame is enough and determine whether interrupt signal received
        while ( isAvFrameEnough( fps ) && !mIsReceiveStopSignal && !mIsReceiveSeekSignal )
        {
            if ( is_new_decode_request )
            {
                initAVFrameReady( mAudioFifo.getFrontFrameSecond() * 1000);
                is_new_decode_request = false;
            }
            Sleep::msleep( 1 );
        }

    }

    // av reach to the end
    mIsDecoding = false;
    decodeDone();

    /******************************************
                Release the Resource
    ******************************************/

    // Free the RGB image
    av_free( buffer );
    av_free( pFrameRGB );

    // Free the YUV frame
    av_free( decodedFrame );

    // Close the codec
    avcodec_close( videoCodecCtx );
    avcodec_close( audioCodecCtx );

    // Close format context
    avformat_close_input( &formatCtx );
}

void LibavWorker::convertToRGBFrame( AVCodecContext * videoCodecCtx, AVFrame * decodedFrame, AVFrame * pFrameRGB )
{
    // Declare SwsContext
    struct SwsContext *pConvertedSwsCtx = NULL;

    // Convert the image from its native format to RGB
    pConvertedSwsCtx = sws_getContext(
        videoCodecCtx->width, videoCodecCtx->height, videoCodecCtx->pix_fmt,
        videoCodecCtx->width, videoCodecCtx->height, PIX_FMT_RGB24,
        SWS_BILINEAR, NULL, NULL, NULL );

    sws_scale( pConvertedSwsCtx,
        (const uint8_t * const *)(decodedFrame->data), decodedFrame->linesize,
        0, videoCodecCtx->height,
        pFrameRGB->data, pFrameRGB->linesize );

    // Release SwsContext
    sws_freeContext( pConvertedSwsCtx );
}

// this will spend lots time, which will cause the delay in video
void LibavWorker::appendAudioPcmToFile( void const * aPcmBuffer, int aPcmSize, char const * aFileName )
{
    FILE * outfile = fopen( aFileName, "ab" );
    assert( outfile );
    fwrite( aPcmBuffer, 1, aPcmSize, outfile );
    fclose( outfile );
}

void LibavWorker::saveVideoPpmToFile( vector<uint8> aPpmFrame, char const * aFileName )
{
    FILE * outfile = fopen( aFileName, "wb" );
    assert( outfile );
    fwrite( &aPpmFrame[0], 1, aPpmFrame.size(), outfile );
    fclose( outfile );
}

void LibavWorker::saveAVInfoToFile( AVInfo const & aAVInfo, char const * aFileName )
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

bool LibavWorker::isAvFrameEnough( double a_fps ) const
{
    // video: at least 2 frames
    // audio: 16KB data, in 48000Hz, 2ch, int13, can play 0.085 second
    // this condition will consume lots mem because we'll read many video frame in order to get audio frames
    // this change due to soundtouch output stream at one go, and portaudio's callback need data ASAP

    if ( ( mVideoFifo.getCount() > 1 ) &&
         ( mAudioFifo.getBytes() > 16 * 1024 ) )
    {
        return true;
    }
    return false;

    // return ( mVideoFifo.getCount() > 1 ) && ( mAudioFifo.getCount() > 1 );

}

void LibavWorker::init()
{
    mVideoFifo.clear();
    mAudioFifo.clear();
}

void LibavWorker::setAudioEffect( int aChannel )
{
    mAudioTuner.setSpeechMode( mIsSpeechMode );
    mAudioTuner.setPitchSemiTones( mSemiTonesDelta );
}
