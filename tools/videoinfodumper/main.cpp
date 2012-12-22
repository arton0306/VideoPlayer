#include "UINT64_C_incpp.hpp"
extern "C"{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libswscale/swscale.h"
}

int getStreamIndex( AVFormatContext * aFormatCtx, AVMediaType aMediaType );
AVCodecContext * getCodecCtx( AVFormatContext * aFormatCtx, int aStreamIndex );
void dumper( char const * filename );

int main()
{
    return 0;
}

void dumper( char const * filename )
{
    // Register all codec
    av_register_all();

    // Declare a decode context
    AVFormatContext * formatCtx = NULL;

    // readHeader
    if ( avformat_open_input( formatCtx, filename, NULL, NULL ) != 0 )
        assert( false );

    // Retrieve stream information
    if ( avformat_find_stream_info( formatCtx, NULL ) < 0 )
        assert( false );

    // Get video/audio stream index
    int const videoStreamIndex = getStreamIndex( formatCtx, AVMEDIA_TYPE_VIDEO );
    int const audioStreamIndex = getStreamIndex( formatCtx, AVMEDIA_TYPE_AUDIO );

    // Set video/audio decoder
    AVCodecContext * videoCodecCtx = getCodecCtx( formatCtx, videoStreamIndex );
    AVCodecContext * audioCodecCtx = getCodecCtx( formatCtx, audioStreamIndex );

    // info to dump
    double video_time_length = formatCtx->duration;
    double const fps = av_q2d( formatCtx->streams[videoStreamIndex]->avg_frame_rate );
    int video_width = videoCodecCtx->width;
    int video_height = videoCodecCtx->height;
    char const * video_codec_name = videoCodecCtx->codec_name;
    char const * audio_codec_name = audioCodecCtx->codec_name;
    int audio_channel_count = audioCodecCtx->channels;
    int audio_sample_rate = audioCodecCtx->sample_rate;
    char const * audio_sample_format = av_get_sample_fmt_name( audioCodecCtx->sample_fmt );
    int audio_sample_bytes = av_get_bytes_per_sample(audioCodecCtx->sample_fmt);

    // Close the codec
    avcodec_close( videoCodecCtx );
    avcodec_close( audioCodecCtx );

    // Close format context
    avformat_close_input( &formatCtx );

    return 0;
}

int getStreamIndex( AVFormatContext * aFormatCtx, AVMediaType aMediaType )
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

AVCodecContext * getCodecCtx( AVFormatContext * aFormatCtx, int aStreamIndex )
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
