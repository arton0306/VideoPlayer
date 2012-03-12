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

private:
    AVPacketList *first_pkt, *last_pkt;
    int nb_packets;
    int size;
    SDL_mutex *mutex;
    SDL_cond *cond;
};

#endif // PACKETQUEUE_HPP
