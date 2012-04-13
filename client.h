/*
 * client.h - Functions for the client.
 */

#ifndef CLIENT_H

#define CLIENT_H

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
#include <allegro.h>
#include "packet.h"
#include "game.h"


struct Params
{
    int sockfd_game;
    int sockfd_chat;
    int player_id;
};

Params client_init();
void recv_map();
void recv_players();

void* game_fn(void *);
void* chat_fn(void *);
void* keyboard_fn(void *);
int main(int, char **);

#endif

