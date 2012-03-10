#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include "ui_VideoPlayer.h"
#include <QMainWindow>

class VideoPlayer : public QMainWindow, private Ui::VideoPlayer
{
    Q_OBJECT
    
public:
    explicit VideoPlayer(QWidget *parent = 0);
    ~VideoPlayer();
    
private:

};

#endif // VIDEOPLAYER_H
