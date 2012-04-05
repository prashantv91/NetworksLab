/*
 * packet.h - Structure of ALL network packets used.
 */

#ifndef PACKET_H

#define PACKET_H

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/select.h>
#include "constants.h"

struct Packet
{
    int player_id;
    pkt_type packet_type;
    char message[PKT_MSG_SIZE];
    Packet(); 
    Packet(int, pkt_type, char *); 
};

bool send_packet(int sockfd, Packet* packet);
bool recv_packet(int sockfd, Packet* packet);
#endif

