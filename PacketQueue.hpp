#ifndef PACKETQUEUE_HPP
#define PACKETQUEUE_HPP

#include "UINT64_C_incpp.hpp"
extern "C"{
    #include "libavformat/avformat.h"
}

#include <queue>

class PacketQueue
{
public:
    PacketQueue();
    void put( AVPacket * aPkt );
    AVPacket get();

private:
    std::queue<AVPacket> mQueue;
    int mSize;
};

#endif // PACKETQUEUE_HPP
