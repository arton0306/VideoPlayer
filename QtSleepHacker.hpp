#ifndef QTSLEEPHACKER_HPP
#define QTSLEEPHACKER_HPP

#include <QThread>

class SleepThread : public QThread{
     Q_OBJECT
     // private run
     void run (){}

     public :
     static void usleep(long iSleepTime){
          QThread::usleep(iSleepTime);
     }
     static void sleep(long iSleepTime){
          QThread::sleep(iSleepTime);
     }
     static void msleep(long iSleepTime){
          QThread::msleep(iSleepTime);
     }
};

#endif // QTSLEEPHACKER_HPP
