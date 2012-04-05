/*
 * client.cpp - Implemets client functions.
 */

#include "client.h"
#include <iostream>
#include <cstdio>
using namespace std;

Game game;

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int get_udp_broadcast_socket()
{
    addrinfo hints, *servinfo, *p;
    int rv, sockfd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    
    if ((rv = getaddrinfo(NULL, SERVER_UDP_PORT, &hints, &servinfo)) != 0) 
    {
        fprintf(stderr, "Client::get_udp_socket(): getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }
    
    for(p = servinfo; p != NULL; p = p->ai_next) 
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
        {
            perror("Client::get_udp_socket(): socket");
            continue;
        }
        break;
    }
    freeaddrinfo(servinfo);
    
    if (p == NULL) 
    {
        fprintf(stderr, "Client::get_udp_socket(): failed to bind socket\n");
        return -2;
    }

#ifdef DEBUG
    fprintf(stderr, "Obtaining socket on port %s.\n", "<how to find port?>");
#endif

    return sockfd;
}


int broadcast_it(int sockfd_broadcast)
{
    addrinfo hints, *servinfo;
    int rv, numbytes, broadcast;
    Packet packet;
    
    if (setsockopt(sockfd_broadcast, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1) 
    {
        perror("setsockopt (SO_BROADCAST)");
        return -1;
    }
        
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    
    if ((rv = getaddrinfo(BROADCAST_ADDRESS, SERVER_UDP_PORT, &hints, &servinfo)) != 0) 
    {
        fprintf(stderr, "Client::broadcast_it(): getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }
    
    packet.player_id = -1;
    packet.packet_type = TYPE_BROADCAST;

    if ((numbytes = sendto(sockfd_broadcast, (char*)&packet, sizeof(packet), 0, servinfo->ai_addr, servinfo->ai_addrlen)) == -1) 
    {
        perror("Client::broadcast_it(): sendto");
        return -1;
    }

    freeaddrinfo(servinfo);

#ifdef DEBUG
    fprintf(stderr, "Broadcasting to port %s.\n", SERVER_UDP_PORT);
#endif

    return 0;

}

void get_server_tcp_port(int udp_sockfd, sockaddr &server_addr, char* server_port, socklen_t &addrlen)         //Returns through the arguments.
{
    Packet packet;
    int numbytes;
    

    if ((numbytes = recvfrom(udp_sockfd, &packet, sizeof(packet), 0, &server_addr, &addrlen)) == -1) 
    {
        perror("Client::get_server_tcp_port(): recvfrom");
        return;
    }
    server_port = new char[strlen(packet.message)];
    strcpy(server_port, packet.message);

#ifdef DEBUG
    char s[100];
    fprintf(stderr, "Found server at %s.\nIt says \"%s\".\n", inet_ntop(server_addr.sa_family, get_in_addr((sockaddr*)&server_addr), s, 100), server_port);
#endif

}

Params client_init()
{
    /*
     * Send broadcast.
     * Receive UDP replies from server.
     * Request and establish two connections with server.
     * Get map and players from server and initialise local structures.
     */
    
    int udp_sockfd;
    Params params;
    sockaddr server_addr;
    char* server_port;
    socklen_t addrlen;

    udp_sockfd = get_udp_broadcast_socket();
    broadcast_it(udp_sockfd);
    get_server_tcp_port(udp_sockfd, server_addr, server_port, addrlen);









}

void game_thread(void *args)
{
    return;

}

void chat_thread(void *args)
{
}

void keyboard(void *args)
{
    Params params = *(Params*)(args);
    Packet game_pkt, chat_pkt;
    int chat_ind = 0;
    char ch, s[10*PKT_MSG_SIZE+1];

    game_pkt.player_id = params.player_id;
    game_pkt.packet_type = TYPE_GAME;

    chat_pkt.player_id = params.player_id;
    chat_pkt.packet_type = TYPE_CHAT;


    //while (GAME_RUNNING)
    //for (int i = 0 ; i < 20; i++)
    while (1)
    {
        while (!keypressed());
        ch = readkey() % 256;
        if (ch > 0 && ch < 128)
        {
            if (ch !='\n' && ch != '\r' && (chat_ind % PKT_MSG_SIZE != PKT_MSG_SIZE-1))
            {
                s[chat_ind] = ch;
                chat_ind++;
#ifdef DEBUG
                textprintf_ex(screen, font, chat_ind*8, 0, makecol(255, 100, 200), -1, "%c", ch);  //x and y interchanged due to differing conventions.
                //cerr<<s<<endl;
#endif
            }
            else
            {
                s[chat_ind] = 0;
                chat_ind++;
                if (chat_ind == 10*PKT_MSG_SIZE)
                    chat_ind = 0;
                /**/break;
            }

        }
        
    }
}


int main()
{
    Params params;
    //GAME_RUNNING = true;
    //keyboard(&params);
    client_init();

    return 0;
}
