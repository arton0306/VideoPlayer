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
    Sleep.cpp

HEADERS += \
    VideoPlayer.hpp \
    LibavWorker.hpp \
    QGLCanvas.hpp \
    QtSleepHacker.hpp \
    UINT64_C_incpp.hpp \
    debug.hpp \
    WavOutFile.hpp \
    FrameFifo.hpp \
    AVInfo.hpp \
    Sleep.hpp

FORMS    += VideoPlayer.ui

INCLUDEPATH += "$$_PRO_FILE_PWD_/libav/include/"
INCLUDEPATH += "$$_PRO_FILE_PWD_/SDL/"

LIBS += -L"$$_PRO_FILE_PWD_/libav/lib/"  -L"$$_PRO_FILE_PWD_/SDL/lib/" -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswscale -lSDLmain -lSDL
