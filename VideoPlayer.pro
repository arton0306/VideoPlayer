#-------------------------------------------------
#
# Project created by QtCreator 2012-03-10T13:43:29
#
#-------------------------------------------------

QT       += core gui

TARGET = VideoPlayer
TEMPLATE = app


SOURCES += main.cpp\
        VideoPlayer.cpp \
    LabavWorker.cpp

HEADERS  += \
    VideoPlayer.hpp \
    LabavWorker.hpp

FORMS    += VideoPlayer.ui

INCLUDEPATH += "$$_PRO_FILE_PWD_/libav/include/"

LIBS += -L"$$_PRO_FILE_PWD_/libav/lib/" -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswscale
