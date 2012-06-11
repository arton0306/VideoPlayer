#include <cassert>
#include <algorithm>
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
    , mIsDecoding( false )
{
}

vector<uint8> LibavWorker::convertToUint8Stream( AVFrame *aDecodedFrame, int width, int height )
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
    return mVideoFifo.getFrontFrameTime();
}

// can be called by player thread
void LibavWorker::dropNextVideoFrame()
{
    if ( mVideoFifo.getCount() > 0 )
    {
        mVideoFifo.pop();
    }
}

// can be called by player thread
void LibavWorker::stopDecoding()
{
    if ( mIsDecoding )
    {
        mIsReceiveStopSignal = true;
    }
}

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
    double const fps = av_q2d( formatCtx->streams[videoFrameIndex]->avg_frame_rate );
    //DEBUG() << "video fps:" << fps;
    //DEBUG() << "videoStreamIndex:" << videoStreamIndex << "    audioStreamIndex:" << audioStreamIndex;

    ready( AVInfo(
        fps,
        formatCtx->duration,
        audioCodecCtx->channels,
        audioCodecCtx->sample_rate,
        av_get_bytes_per_sample(audioCodecCtx->sample_fmt) * BITS_PER_BYTES
        ) );

    while ( av_read_frame( formatCtx, &packet ) >= 0 )
    {
        // determine whether be forced stop
        if ( mIsReceiveStopSignal )
        {
            mVideoFifo.clear();
            mAudioFifo.clear();
            mIsReceiveStopSignal = false;
            break;
        }

        // the index is just for debug
        ++packetIndex;

        // determine whether decoded frame is not enough
        while ( isAvFrameEnough( fps ) && !mIsReceiveStopSignal )
        {
            // Sleep::usleep( 0.1 * 1.0 / fps * 1000000 );
            Sleep::msleep( 1 );
        }

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
                convertToRGBFrame( videoCodecCtx, decodedFrame, pFrameRGB );

                // fill in our ppm buffer
                mVideoFifo.push(
                    convertToUint8Stream( pFrameRGB, videoCodecCtx->width, videoCodecCtx->height ),
                    packet.dts * av_q2d(formatCtx->streams[videoStreamIndex]->time_base )
                    );

                // Dump pts and dts for debug
                ++videoFrameIndex;
                // DEBUG() << "p ndx:" << packetIndex << "    vf ndx:" << videoFrameIndex << "     PTS:" << packet.pts << "     DTS:" << packet.dts << " TimeBase:" << av_q2d(formatCtx->streams[videoStreamIndex]->time_base) << " *dts: " << packet.dts * av_q2d(formatCtx->streams[videoStreamIndex]->time_base);
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

                        // push audio data to fifo
                        vector<uint8> decodedStream( data_size, 0 );
                        memcpy( &decodedStream[0], decodedFrame->data[0], data_size );
                        mAudioFifo.push( decodedStream, av_q2d(formatCtx->streams[audioStreamIndex]->time_base) * packet.pts );

                        // appendPcmToFile( decodedFrame->data[0], data_size, "pcm.pcm" ); // this will spend lots time, which will cause the delay in video
                        ++audioFrameIndex;
                        // DEBUG() << "p ndx:" << packetIndex << "     af ndx:" << audioFrameIndex << "     PTS:" << packet.pts << "     DTS:" << packet.dts << " TimeBase:" << av_q2d(formatCtx->streams[audioStreamIndex]->time_base) << " *dts:" << av_q2d(formatCtx->streams[audioStreamIndex]->time_base) * packet.pts;
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
void LibavWorker::appendPcmToFile( void const * aPcmBuffer, int aPcmSize, char const * aFileName )
{
    FILE * outfile = fopen( aFileName, "ab" );
    assert( outfile );
    fwrite( aPcmBuffer, 1, aPcmSize, outfile );
    fclose( outfile );
}

bool LibavWorker::isAvFrameEnough( double a_fps ) const
{
    return ( min( mVideoFifo.getCount(), mAudioFifo.getCount() ) > 1 );
}

void LibavWorker::init()
{
    mVideoFifo.clear();
    mAudioFifo.clear();
}
