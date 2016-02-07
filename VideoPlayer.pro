#-------------------------------------------------
#
# Project created by QtCreator 2012-03-10T13:43:29
#
#-------------------------------------------------

QT       += widgets core gui opengl multimedia

DEFINES += __STDC_CONSTANT_MACROS

TARGET = VideoPlayer
TEMPLATE = app

SOURCES += \
    main.cpp\
    VideoPlayer.cpp \
    QGLCanvas.cpp \
    debug.cpp \
    WavOutFile.cpp \
    FrameFifo.cpp \
    AVInfo.cpp \
    Sleep.cpp \
    MultimediaWidget.cpp \
    AudioTuner.cpp \
    VolumeTuner.cpp \
    # AudioPlayer/main.cpp \
    # tools/videoinfodumper/dumper.cpp \
    # tools/videoinfodumper/main.cpp \
    # tools/videoinfodumper/using_dumper.cpp \
    # tools/videoinfodumper/libav_probe.c \
    # tools/wavheader/MakeWavHeader.cpp \
    # libav_using.c
    DecodeWorker.cpp

HEADERS += \
    VideoPlayer.hpp \
    QGLCanvas.hpp \
    QtSleepHacker.hpp \
    debug.hpp \
    WavOutFile.hpp \
    FrameFifo.hpp \
    AVInfo.hpp \
    Sleep.hpp \
    MultimediaWidget.hpp \
    AudioTuner.hpp \
    VolumeTuner.hpp \
    3rdparty/ffmpeg/include/libavcodec/avcodec.h \
    3rdparty/ffmpeg/include/libavcodec/avdct.h \
    3rdparty/ffmpeg/include/libavcodec/avfft.h \
    3rdparty/ffmpeg/include/libavcodec/d3d11va.h \
    3rdparty/ffmpeg/include/libavcodec/dirac.h \
    3rdparty/ffmpeg/include/libavcodec/dv_profile.h \
    3rdparty/ffmpeg/include/libavcodec/dxva2.h \
    3rdparty/ffmpeg/include/libavcodec/qsv.h \
    3rdparty/ffmpeg/include/libavcodec/vaapi.h \
    3rdparty/ffmpeg/include/libavcodec/vda.h \
    3rdparty/ffmpeg/include/libavcodec/vdpau.h \
    3rdparty/ffmpeg/include/libavcodec/version.h \
    3rdparty/ffmpeg/include/libavcodec/videotoolbox.h \
    3rdparty/ffmpeg/include/libavcodec/vorbis_parser.h \
    3rdparty/ffmpeg/include/libavcodec/xvmc.h \
    3rdparty/ffmpeg/include/libavdevice/avdevice.h \
    3rdparty/ffmpeg/include/libavdevice/version.h \
    3rdparty/ffmpeg/include/libavfilter/avfilter.h \
    3rdparty/ffmpeg/include/libavfilter/avfiltergraph.h \
    3rdparty/ffmpeg/include/libavfilter/buffersink.h \
    3rdparty/ffmpeg/include/libavfilter/buffersrc.h \
    3rdparty/ffmpeg/include/libavfilter/version.h \
    3rdparty/ffmpeg/include/libavformat/avformat.h \
    3rdparty/ffmpeg/include/libavformat/avio.h \
    3rdparty/ffmpeg/include/libavformat/version.h \
    3rdparty/ffmpeg/include/libavutil/adler32.h \
    3rdparty/ffmpeg/include/libavutil/aes.h \
    3rdparty/ffmpeg/include/libavutil/aes_ctr.h \
    3rdparty/ffmpeg/include/libavutil/attributes.h \
    3rdparty/ffmpeg/include/libavutil/audio_fifo.h \
    3rdparty/ffmpeg/include/libavutil/avassert.h \
    3rdparty/ffmpeg/include/libavutil/avconfig.h \
    3rdparty/ffmpeg/include/libavutil/avstring.h \
    3rdparty/ffmpeg/include/libavutil/avutil.h \
    3rdparty/ffmpeg/include/libavutil/base64.h \
    3rdparty/ffmpeg/include/libavutil/blowfish.h \
    3rdparty/ffmpeg/include/libavutil/bprint.h \
    3rdparty/ffmpeg/include/libavutil/bswap.h \
    3rdparty/ffmpeg/include/libavutil/buffer.h \
    3rdparty/ffmpeg/include/libavutil/camellia.h \
    3rdparty/ffmpeg/include/libavutil/cast5.h \
    3rdparty/ffmpeg/include/libavutil/channel_layout.h \
    3rdparty/ffmpeg/include/libavutil/common.h \
    3rdparty/ffmpeg/include/libavutil/cpu.h \
    3rdparty/ffmpeg/include/libavutil/crc.h \
    3rdparty/ffmpeg/include/libavutil/des.h \
    3rdparty/ffmpeg/include/libavutil/dict.h \
    3rdparty/ffmpeg/include/libavutil/display.h \
    3rdparty/ffmpeg/include/libavutil/downmix_info.h \
    3rdparty/ffmpeg/include/libavutil/error.h \
    3rdparty/ffmpeg/include/libavutil/eval.h \
    3rdparty/ffmpeg/include/libavutil/ffversion.h \
    3rdparty/ffmpeg/include/libavutil/fifo.h \
    3rdparty/ffmpeg/include/libavutil/file.h \
    3rdparty/ffmpeg/include/libavutil/frame.h \
    3rdparty/ffmpeg/include/libavutil/hash.h \
    3rdparty/ffmpeg/include/libavutil/hmac.h \
    3rdparty/ffmpeg/include/libavutil/imgutils.h \
    3rdparty/ffmpeg/include/libavutil/intfloat.h \
    3rdparty/ffmpeg/include/libavutil/intreadwrite.h \
    3rdparty/ffmpeg/include/libavutil/lfg.h \
    3rdparty/ffmpeg/include/libavutil/log.h \
    3rdparty/ffmpeg/include/libavutil/lzo.h \
    3rdparty/ffmpeg/include/libavutil/macros.h \
    3rdparty/ffmpeg/include/libavutil/mastering_display_metadata.h \
    3rdparty/ffmpeg/include/libavutil/mathematics.h \
    3rdparty/ffmpeg/include/libavutil/md5.h \
    3rdparty/ffmpeg/include/libavutil/mem.h \
    3rdparty/ffmpeg/include/libavutil/motion_vector.h \
    3rdparty/ffmpeg/include/libavutil/murmur3.h \
    3rdparty/ffmpeg/include/libavutil/opt.h \
    3rdparty/ffmpeg/include/libavutil/parseutils.h \
    3rdparty/ffmpeg/include/libavutil/pixdesc.h \
    3rdparty/ffmpeg/include/libavutil/pixelutils.h \
    3rdparty/ffmpeg/include/libavutil/pixfmt.h \
    3rdparty/ffmpeg/include/libavutil/random_seed.h \
    3rdparty/ffmpeg/include/libavutil/rational.h \
    3rdparty/ffmpeg/include/libavutil/rc4.h \
    3rdparty/ffmpeg/include/libavutil/replaygain.h \
    3rdparty/ffmpeg/include/libavutil/ripemd.h \
    3rdparty/ffmpeg/include/libavutil/samplefmt.h \
    3rdparty/ffmpeg/include/libavutil/sha.h \
    3rdparty/ffmpeg/include/libavutil/sha512.h \
    3rdparty/ffmpeg/include/libavutil/stereo3d.h \
    3rdparty/ffmpeg/include/libavutil/tea.h \
    3rdparty/ffmpeg/include/libavutil/threadmessage.h \
    3rdparty/ffmpeg/include/libavutil/time.h \
    3rdparty/ffmpeg/include/libavutil/timecode.h \
    3rdparty/ffmpeg/include/libavutil/timestamp.h \
    3rdparty/ffmpeg/include/libavutil/tree.h \
    3rdparty/ffmpeg/include/libavutil/twofish.h \
    3rdparty/ffmpeg/include/libavutil/version.h \
    3rdparty/ffmpeg/include/libavutil/xtea.h \
    3rdparty/ffmpeg/include/libswresample/swresample.h \
    3rdparty/ffmpeg/include/libswresample/version.h \
    3rdparty/ffmpeg/include/libswscale/swscale.h \
    3rdparty/ffmpeg/include/libswscale/version.h \
    3rdparty/portaudio/include/portaudio.h \
    3rdparty/soundtouch/include/BPMDetect.h \
    3rdparty/soundtouch/include/FIFOSampleBuffer.h \
    3rdparty/soundtouch/include/FIFOSamplePipe.h \
    3rdparty/soundtouch/include/SoundTouch.h \
    3rdparty/soundtouch/include/soundtouch_config.h \
    3rdparty/soundtouch/include/STTypes.h \
    DecodeWorker.hpp

# AudioPlayer
SOURCES += \
    AudioPlayer/AudioPlayer.cpp
HEADERS += \
    AudioPlayer/AudioPlayer.hpp


FORMS    += VideoPlayer.ui \
            MultimediaWidget.ui

INCLUDEPATH += "$$_PRO_FILE_PWD_/3rdparty/ffmpeg/include"
INCLUDEPATH += "$$_PRO_FILE_PWD_/3rdparty/soundtouch/include"
INCLUDEPATH += "$$_PRO_FILE_PWD_/3rdparty/portaudio/include"

LIBS += -L"$$_PRO_FILE_PWD_/3rdparty/ffmpeg/lib" -lavformat -lavcodec -lswscale -lavutil -lavdevice -lavfilter -lswresample -lpthread -lm -lz -lrt
LIBS += -L"$$_PRO_FILE_PWD_/3rdparty/soundtouch/lib" -Bstatic -lSoundTouch
LIBS += -L"$$_PRO_FILE_PWD_/3rdparty/portaudio/lib" -lportaudio -lasound -lrt -ljack  -lSoundTouch

