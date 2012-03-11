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
    QGLCanvas.cpp

HEADERS  += \
    VideoPlayer.hpp \
    LibavWorker.hpp \
    QGLCanvas.hpp

FORMS    += VideoPlayer.ui

INCLUDEPATH += "$$_PRO_FILE_PWD_/libav/include/"

LIBS += -L"$$_PRO_FILE_PWD_/libav/lib/" -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswscale
