#-------------------------------------------------
#
# Project created by QtCreator 2012-03-10T13:43:29
#
#-------------------------------------------------

QT       += core gui opengl

TARGET = VideoPlayer
TEMPLATE = app


SOURCES += main.cpp\
        VideoPlayer.cpp \
        LibavWorker.cpp \
    QGLCanvas.cpp \
    PacketQueue.cpp \
    debug.cpp \
    WavOutFile.cpp

HEADERS  += \
    VideoPlayer.hpp \
    LibavWorker.hpp \
    QGLCanvas.hpp \
    QtSleepHacker.hpp \
    PacketQueue.hpp \
    UINT64_C_incpp.hpp \
    debug.hpp \
    WavOutFile.hpp

FORMS    += VideoPlayer.ui

INCLUDEPATH += "$$_PRO_FILE_PWD_/libav/include/"
INCLUDEPATH += "$$_PRO_FILE_PWD_/SDL/"

LIBS += -L"$$_PRO_FILE_PWD_/libav/lib/"  -L"$$_PRO_FILE_PWD_/SDL/lib/" -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswscale -lSDLmain -lSDL
