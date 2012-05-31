#include "Sleep.hpp"

/* static */ void Sleep::sleep(unsigned long secs)
{
    QThread::sleep(secs);
}
/* static */ void Sleep::msleep(unsigned long msecs)
{
    QThread::msleep(msecs);
}
/* static */ void Sleep::usleep(unsigned long usecs)
{
    QThread::usleep(usecs);
}
