#ifndef PACKETQUEUE_HPP
#define PACKETQUEUE_HPP

#include "UINT64_C_incpp.hpp"
extern "C"{
    #include "libavformat/avformat.h"
    #include "SDL/SDL.h"
    #include "SDL/SDL_thread.h"
}

class PacketQueue
{
public:
    PacketQueue();
    void put( AVPacket * aPkt );
    AVPacket get();

private:
    typedef AVPacketList AVPacketNode;
    void init();

    AVPacketNode *mFirstPacketNode, *mLastPacketNode;
    int mPacketsCount;
    int mSize;
    SDL_mutex *mMutex;
    SDL_cond *mCond;
};

#endif // PACKETQUEUE_HPP
