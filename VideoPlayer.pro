#-------------------------------------------------
#
# Project created by QtCreator 2012-03-10T13:43:29
#
#-------------------------------------------------

QT       += core gui opengl multimedia

TARGET = VideoPlayer
TEMPLATE = app


SOURCES += \
    main.cpp\
    VideoPlayer.cpp \
    LibavWorker.cpp \
    QGLCanvas.cpp \
    debug.cpp \
    WavOutFile.cpp \
    FrameFifo.cpp \
    AVInfo.cpp \
    Sleep.cpp \
    MultimediaWidget.cpp \
    AudioTuner.cpp \
    VolumeTuner.cpp

HEADERS += \
    VideoPlayer.hpp \
    LibavWorker.hpp \
    QGLCanvas.hpp \
    QtSleepHacker.hpp \
    debug.hpp \
    WavOutFile.hpp \
    FrameFifo.hpp \
    AVInfo.hpp \
    Sleep.hpp \
    MultimediaWidget.hpp \
    AudioTuner.hpp \
    VolumeTuner.hpp

FORMS    += VideoPlayer.ui \
            MultimediaWidget.ui

INCLUDEPATH += "$$_PRO_FILE_PWD_/libav/include/"
INCLUDEPATH += "$$_PRO_FILE_PWD_/SDL/"
INCLUDEPATH += "$$_PRO_FILE_PWD_/SoundTouch/include"

LIBS += -L"$$_PRO_FILE_PWD_/libav/lib/"  -L"$$_PRO_FILE_PWD_/SDL/lib/" -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswscale -lSDLmain -lSDL
LIBS += -L"$$_PRO_FILE_PWD_/SoundTouch/lib/" -lSoundTouch

# AudioPlayer
SOURCES += \
    AudioPlayer/AudioPlayer.cpp
HEADERS += \
    AudioPlayer/AudioPlayer.hpp
INCLUDEPATH += "$$_PRO_FILE_PWD_/AudioPlayer/"
INCLUDEPATH += "$$_PRO_FILE_PWD_/AudioPlayer/portaudio/include/"
LIBS += -L"$$_PRO_FILE_PWD_/AudioPlayer/portaudio/lib/" -llibportaudio

