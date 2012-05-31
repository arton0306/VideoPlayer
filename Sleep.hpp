#ifndef SLEEP_HPP
#define SLEEP_HPP

#include <QThread>

class Sleep : public QThread
{
    public:
        static void sleep(unsigned long secs);
        static void msleep(unsigned long msecs);
        static void usleep(unsigned long usecs);
};

#endif // SLEEP_HPP
