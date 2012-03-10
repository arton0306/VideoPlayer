#include "LibavWorker.hpp"

LibavWorker::LibavWorker(QObject *parent) :
    QObject(parent)
{
}

void LibavWorker::doWork()
{
    libav();
}

void LibavWorker::SaveFrame(AVFrame *pFrame, int width, int height, int iFrame)
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
        fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile );

    // Close file
    fclose(pFile);
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
    AVCodecContext *pCodecCtx;
    int videoStream = -1;
    int i = 0;

    // Find the first video stream
    for ( i = 0; i < pFormatCtx->nb_streams; ++i )
    {
        if ( pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO )
        {
            videoStream = i;
            break;
        }
    }
    if ( videoStream == -1 )
    {
        return -1;
    }

    // Get a pointer to the codec context for the video stream
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;

    // Declare codec
    AVCodec *pCodec;

    // Find the decoder for the video stream
    pCodec = avcodec_find_decoder( pCodecCtx->codec_id );
    if ( pCodec == NULL )
    {
        fprintf( stderr, "Unsupported codec!\n" );
        return -1;
    }

    // Open codec
    if ( avcodec_open2( pCodecCtx, pCodec, NULL ) < 0 )
    {
        return -1;
    }

    /******************************************
                Storing the Data
    ******************************************/

    // Declare frame
    AVFrame *pFrame = avcodec_alloc_frame();
    if ( pFrame == NULL )
    {
        return -1;
    }
    AVFrame *pFrameRGB = avcodec_alloc_frame();
    if ( pFrameRGB == NULL )
    {
        return -1;
    }

    // Declare buffer
    uint8_t *buffer;
    int numBytes;

    // Determine required buffer size and allocate buffer
    numBytes = avpicture_get_size( PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height );
    buffer = (uint8_t *)av_malloc( numBytes * sizeof( uint8_t ) );

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
    // of AVPicture
    avpicture_fill( (AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24,
        pCodecCtx->width, pCodecCtx->height );

    /******************************************
                Reading the Data
    ******************************************/
    int frameFinished;
    AVPacket packet;

    i = 0;
    while ( av_read_frame( pFormatCtx, &packet ) >= 0 )
    {
        // Is this packet from the video stream?
        if ( packet.stream_index == videoStream )
        {
            // Decode video frame
            avcodec_decode_video2( pCodecCtx, pFrame, &frameFinished, &packet );

            // Did we get a video frame?
            if ( frameFinished )
            {
                // Declare SwsContext
                struct SwsContext *pConvertedSwsCtx = NULL;

                // Convert the image from its native format to RGB
                pConvertedSwsCtx = sws_getContext(
                    pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                    pCodecCtx->width, pCodecCtx->height, PIX_FMT_RGB24,
                    SWS_BILINEAR, NULL, NULL, NULL );

                sws_scale( pConvertedSwsCtx,
                    (const uint8_t * const *)(pFrame->data), pFrame->linesize,
                    0, pCodecCtx->height,
                    pFrameRGB->data, pFrameRGB->linesize );

                // Release SwsContext
                sws_freeContext( pConvertedSwsCtx );

                // Save the frame to disk
                ++i;
                if ( 1950 <= i && i <= 1960 )
                {
                    SaveFrame( pFrameRGB, pCodecCtx->width,
                        pCodecCtx->height, i );
                }
            }
        }

        // Free the packet that was allocated by av_read_frame
        av_free_packet( &packet );
    }

    /******************************************
                Release the Resource
    ******************************************/

    // Free the RGB image
    av_free( buffer );
    av_free( pFrameRGB );

    // Free the YUV frmae
    av_free( pFrame );

    // Close the codec
    avcodec_close( pCodecCtx );

    // Close format context
    avformat_close_input( &pFormatCtx );

    return 0;
}
