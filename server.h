/*
 * server.h - Functions for the server
 */

#ifndef SERVER_H

#define SERVER_H

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <allegro.h>
#include <signal.h>
#include <fcntl.h>
#include "packet.h"

#endif

void sigchld_handler(int);
void server_init(); 
int main(); 
void create_socket(int *, char *, int); 
void receive_players(); 
void check_broadcast(); 
void check_connection(); 
