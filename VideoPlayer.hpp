#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include "ui_VideoPlayer.h"
#include <QMainWindow>

class MultimediaWidget;

class VideoPlayer : public QMainWindow, private Ui::VideoPlayer
{
    Q_OBJECT

public:
    explicit VideoPlayer(QWidget *parent = 0);
    ~VideoPlayer();

signals:

public slots:
    void openFile();
    void stop();
    void seek();

private:
    void setupConnection();

    MultimediaWidget * mMultimediaWidget;
};

#endif // VIDEOPLAYER_H
