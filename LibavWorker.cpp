#include <cassert>
#include "LibavWorker.hpp"
#include "QtSleepHacker.hpp"
#include "debug.hpp"

LibavWorker::LibavWorker(QObject *parent) :
    QObject(parent)
{
}

void LibavWorker::doWork()
{
    libav();
}

void LibavWorker::saveFrame(AVFrame *aDecodedFrame, int width, int height, int iFrame)
{
    FILE *pFile;
    char szFilename[32];
    int  y;

    // Open file
    sprintf(szFilename, "frame%d.ppm", iFrame);
    pFile=fopen(szFilename, "wb");
    if(pFile==NULL)
        return;

    // Write header
    fprintf( pFile, "P6\n%d %d\n255\n", width, height );

    // Write pixel data
    for( y = 0; y < height; y++)
        fwrite(aDecodedFrame->data[0]+y*aDecodedFrame->linesize[0], 1, width*3, pFile );

    // Close file
    fclose(pFile);
}

void LibavWorker::saveFrame( int aFrame )
{
    FILE *pFile;
    char szFilename[32];

    // Open file
    sprintf(szFilename, "frame%05d.ppm", aFrame);
    pFile=fopen(szFilename, "wb");
    if(pFile==NULL)
        return;

    // Write file by buffer
    fwrite( (char *)mPpmBuffer, 1, mPpmSize, pFile );

    // Close file
    fclose( pFile );
}

void LibavWorker::fillPpmBuffer( AVFrame *aDecodedFrame, int width, int height )
{
    // Write ppm header
    int const headLength = sprintf( (char *)mPpmBuffer, "P6\n%d %d\n255\n", width, height );

    // Write pixel data
    int const horizontalLineBytes = width * 3;
    for( int rowIndex = 0; rowIndex < height; ++rowIndex )
    {
        memcpy( mPpmBuffer + headLength + rowIndex * horizontalLineBytes,
                aDecodedFrame->data[0] + rowIndex * aDecodedFrame->linesize[0],
                horizontalLineBytes );
    }

    // Write buffer size
    mPpmSize = headLength + height * horizontalLineBytes;

    emit frameReady( mPpmBuffer, mPpmSize );
    SleepThread::msleep( 33 );
}

int LibavWorker::libav()
{
    /******************************************
                Opening the file
    ******************************************/

    // Register all codec
    av_register_all();

    // Declare format context
    AVFormatContext *pFormatCtx = NULL;

    // Open video file
    if ( avformat_open_input( &pFormatCtx, "video/Lelouch.mp4", NULL, NULL ) != 0 )
    {
        return -1;
    }

    // Retrieve stream information
    if ( avformat_find_stream_info( pFormatCtx, NULL ) < 0 )
    {
        return -1;
    }

    // Declare codec context
    AVCodecContext *videoCodecCtx;
    AVCodecContext *audioCodecCtx;
    int videoStream = -1;
    int audioStream = -1;
    int i = 0;

    // Find the first video stream
    for ( i = 0; (unsigned)i < pFormatCtx->nb_streams; ++i )
    {
        if ( pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO && videoStream == -1 )
        {
            videoStream = i;
        }
        if ( pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && audioStream == -1 )
        {
            audioStream = i;
        }
    }

    // Didn't find a video stream
    if ( videoStream == -1 )
    {
        return -1;
    }

    // Didn't find a audio stream
    if ( audioStream == -1 )
    {
        return -1;
    }

    // Get a pointer to the codec context for the video stream
    videoCodecCtx = pFormatCtx->streams[videoStream]->codec;
    audioCodecCtx = pFormatCtx->streams[audioStream]->codec;

    // Find the decoder for the video stream
    AVCodec * videoCodec = avcodec_find_decoder( videoCodecCtx->codec_id );
    if ( videoCodec == NULL )
    {
        fprintf( stderr, "Unsupported video codec!\n" );
        return -1;
    }

    // Open video codec
    if ( avcodec_open2( videoCodecCtx, videoCodec, NULL ) < 0 )
    {
        return -1;
    }

    // Find the decoder for the audio stream
    AVCodec * audioCodec = avcodec_find_decoder( audioCodecCtx->codec_id );
    if ( audioCodec == NULL )
    {
        fprintf( stderr, "Unsupported audio codec!\n" );
        return -1;
    }

    // Open video codec
    if ( avcodec_open2( audioCodecCtx, audioCodec, NULL ) < 0 )
    {
        return -1;
    }

    /******************************************
                Storing the Data
    ******************************************/

    // Declare frame
    AVFrame *decodedFrame = avcodec_alloc_frame();
    if ( decodedFrame == NULL )
    {
        return -1;
    }
    AVFrame *pFrameRGB = avcodec_alloc_frame();
    if ( pFrameRGB == NULL )
    {
        return -1;
    }

    // Determine required buffer size and allocate buffer
    int const numBytes = avpicture_get_size( PIX_FMT_RGB24, videoCodecCtx->width, videoCodecCtx->height );

    // Create a buffer for converted frame
    uint8_t * buffer = (uint8_t *)av_malloc( numBytes * sizeof( uint8_t ) );

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

    int frameIndex = 0;
    while ( av_read_frame( pFormatCtx, &packet ) >= 0 )
    {

        // Is this packet from the video stream?
        if ( packet.stream_index == videoStream )
        {
            // Decode video frame
            int bytesUsed = avcodec_decode_video2( videoCodecCtx, decodedFrame, &frameFinished, &packet );
            if ( bytesUsed != packet.size )
            {
                // check if one packet is corresponding to one frame
                DEBUG() << bytesUsed << " " << packet.size;
            }

            // Did we get a video frame?
            if ( frameFinished )
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

                // fill in our ppm buffer
                fillPpmBuffer( pFrameRGB, videoCodecCtx->width, videoCodecCtx->height );

                // Dump pts and dts for debug
                ++frameIndex;
                //saveFrame( frameIndex );
                DEBUG() << "frame index:" << frameIndex << "     PTS:" << packet.pts << "     DTS:" << packet.dts << " @ " << av_q2d(pFormatCtx->streams[videoStream]->time_base) << packet.pts * av_q2d(pFormatCtx->streams[videoStream]->time_base);
            }

            // Free the packet that was allocated by av_read_frame
            av_free_packet( &packet );
        }
        else if ( packet.stream_index == audioStream )
        {
            avcodec_get_frame_defaults( decodedFrame );
            uint8_t * const packetDataHead = packet.data;
            while ( packet.size > 0 )
            {
                // Decod audio frame
                int bytesUsed = avcodec_decode_audio4( audioCodecCtx, decodedFrame, &frameFinished, &packet );
                if ( bytesUsed < 0 )
                {
                    fprintf( stderr, "Error while decoding audio!\n" );
                    assert( true );
                }
                else
                {
                    if ( frameFinished )
                    {
                        int data_size = av_samples_get_buffer_size(NULL, audioCodecCtx->channels,
                            decodedFrame->nb_samples,
                            audioCodecCtx->sample_fmt, 1);

                        appendPcmToFile( decodedFrame->data[0], data_size, "pcm.pcm" );
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
        }
    }

    /******************************************
                Release the Resource
    ******************************************/

    // Free the RGB image
    av_free( buffer );
    av_free( pFrameRGB );

    // Free the YUV frmae
    av_free( decodedFrame );

    // Close the codec
    avcodec_close( videoCodecCtx );
    avcodec_close( audioCodecCtx );

    // Close format context
    avformat_close_input( &pFormatCtx );

    return 0;
}

uint8_t const * LibavWorker::getPpmBuffer() const
{
    return mPpmBuffer;
}

int LibavWorker::getPpmSize() const
{
    return mPpmSize;
}

void LibavWorker::appendPcmToFile( void const * aPcmBuffer, int aPcmSize, char const * aFileName )
{
    FILE * outfile = fopen( aFileName, "ab" );
    assert( outfile );
    fwrite( aPcmBuffer, 1, aPcmSize, outfile );
    fclose( outfile );
}
