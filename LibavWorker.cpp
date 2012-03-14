#include "LibavWorker.hpp"
#include "QtSleepHacker.hpp"
#include "debug.hpp"

extern "C"{
    #include "SDL/SDL.h"
}

void audio_callback(void *userdata, Uint8 *stream, int len);
int audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf, int buf_size);
const int SDL_AUDIO_BUFFER_SIZE = 1024;

LibavWorker::LibavWorker(QObject *parent) :
    QObject(parent)
{
}

void LibavWorker::doWork()
{
    libav();
}

void LibavWorker::saveFrame(AVFrame *pFrame, int width, int height, int iFrame)
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

void LibavWorker::saveFrame( int aFrame )
{
    FILE *pFile;
    char szFilename[32];

    // Open file
    sprintf(szFilename, "frame%d.ppm", aFrame);
    pFile=fopen(szFilename, "wb");
    if(pFile==NULL)
        return;

    // Write file by buffer
    fwrite( (char *)mPpmBuffer, 1, mPpmSize, pFile );

    // Close file
    fclose( pFile );
}

void LibavWorker::fillPpmBuffer( AVFrame *pFrame, int width, int height )
{
    // Write ppm header
    int const headLength = sprintf( (char *)mPpmBuffer, "P6\n%d %d\n255\n", width, height );

    // Write pixel data
    int const horizontalLineBytes = width * 3;
    for( int rowIndex = 0; rowIndex < height; ++rowIndex )
    {
        memcpy( mPpmBuffer + headLength + rowIndex * horizontalLineBytes,
                pFrame->data[0] + rowIndex * pFrame->linesize[0],
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
    if ( avformat_open_input( &pFormatCtx, "video/ktv.mpg", NULL, NULL ) != 0 )
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

    // fill SDL_AudioSpec
    SDL_AudioSpec wanted_spec;
    wanted_spec.freq = audioCodecCtx->sample_rate;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = audioCodecCtx->channels;
    wanted_spec.silence = 0;
    wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
    wanted_spec.callback = audio_callback;
    wanted_spec.userdata = audioCodecCtx;

    // Open audio codec
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

    SDL_PauseAudio(0);

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

    i = 0;
    while ( av_read_frame( pFormatCtx, &packet ) >= 0 )
    {
        // Is this packet from the video stream?
        if ( packet.stream_index == videoStream )
        {
            // Decode video frame
            int bytesUsed = avcodec_decode_video2( videoCodecCtx, pFrame, &frameFinished, &packet );
            if ( bytesUsed != packet.size )
            {
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
                    (const uint8_t * const *)(pFrame->data), pFrame->linesize,
                    0, videoCodecCtx->height,
                    pFrameRGB->data, pFrameRGB->linesize );

                // Release SwsContext
                sws_freeContext( pConvertedSwsCtx );

                // fill in our ppm buffer
                fillPpmBuffer( pFrameRGB, videoCodecCtx->width, videoCodecCtx->height );
            }

            // Free the packet that was allocated by av_read_frame
            av_free_packet( &packet );
        }
        else if ( packet.stream_index == audioStream )
        {
            mAudioQueue.put( &packet );

            // Notice: we will free the packet in the get function
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
    av_free( pFrame );

    // Close the codec
    avcodec_close( videoCodecCtx );

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

void audio_callback(void *userdata, Uint8 *stream, int len) {

    AVCodecContext *aCodecCtx = (AVCodecContext *)userdata;
    int len1, audio_size;

    static uint8_t audio_buf[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
    static unsigned int audio_buf_size = 0;
    static unsigned int audio_buf_index = 0;

    while(len > 0) {
        if(audio_buf_index >= audio_buf_size) {
            /* We have already sent all our data; get more */
            audio_size = audio_decode_frame(aCodecCtx, audio_buf, sizeof(audio_buf));
            if(audio_size < 0) {
                /* If error, output silence */
                audio_buf_size = 1024;
                memset(audio_buf, 0, audio_buf_size);
            } else {
                audio_buf_size = audio_size;
            }
            audio_buf_index = 0;
        }
        len1 = audio_buf_size - audio_buf_index;
        if(len1 > len)
            len1 = len;
        memcpy(stream, (uint8_t *)audio_buf + audio_buf_index, len1);
        len -= len1;
        stream += len1;
        audio_buf_index += len1;
    }
}

int audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf,
        int buf_size) {

    static AVPacket pkt;
    static uint8_t *audio_pkt_data = NULL;
    static int audio_pkt_size = 0;

    int len1, data_size;

    for(;;) {
        while(audio_pkt_size > 0) {
            data_size = buf_size;
            len1 = avcodec_decode_audio4(aCodecCtx, (int16_t *)audio_buf, &data_size, 
                    audio_pkt_data, audio_pkt_size);
            if(len1 < 0) {
                /* if error, skip frame */
                audio_pkt_size = 0;
                break;
            }
            audio_pkt_data += len1;
            audio_pkt_size -= len1;
            if(data_size <= 0) {
                /* No data yet, get more frames */
                continue;
            }
            /* We have data, return it and come back for more later */
            return data_size;
        }
        if(pkt.data)
            av_free_packet(&pkt);

        if(quit) {
            return -1;
        }

        pkt = mAudioQueue.get();
        audio_pkt_data = pkt.data;
        audio_pkt_size = pkt.size;
    }
}

