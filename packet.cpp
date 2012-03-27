/*
 * packet.cpp - Implements packet send and receive functions.
 */

#include "packet.h"
#include <iostream>
using namespace std;

bool send_packet(int sockfd, Packet* packet)
{
    if (send(sockfd, (char*)packet, sizeof(Packet), 0) == -1)
    {
        cerr<<"send_packet: Packet sending failed.\n";
        cerr<<"Exiting.\n";
        //exit(2);
        return false;
    }
#ifdef DEBUG
    cerr<<"recv_packet: Packet contains: "<<packet->player_id<<' '<<packet->packet_type<<' '<<packet->message<<endl;
#endif
    return true;
}

bool recv_packet(int sockfd, Packet* packet)
{
    if (recv(sockfd, packet, sizeof(Packet), 0) > 0)
    {
#ifdef DEBUG
        cerr<<"recv_packet: Packet contains: "<<packet->player_id<<' '<<packet->packet_type<<' '<<packet->message<<endl;
#endif
        return true;
    }
    return false;
}

