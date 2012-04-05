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

void get_server_tcp_port(int udp_sockfd, char* &server_address, char* &server_port, socklen_t &addrlen, sockaddr_storage server_addr)         //Returns through the arguments.
{
//    sockaddr server_addr;
    Packet packet;
    int numbytes;
    char s[100];
    

    if ((numbytes = recvfrom(udp_sockfd, &packet, sizeof(packet), 0, (sockaddr*)&server_addr, &addrlen)) == -1) 
    {
        perror("Client::get_server_tcp_port(): recvfrom");
        return;
    }

    server_port = new char[strlen(packet.message)+1];
    strcpy(server_port, packet.message);

    inet_ntop(server_addr.ss_family, get_in_addr((sockaddr*)&server_addr), s, 100);
    server_address = new char[strlen(s)+1];
    strcpy(server_address, s);

#ifdef DEBUG
    //char s[100];
   fprintf(stderr, "Found server at %s.\nIt says \"%s\".\n", server_address, server_port);
#endif

}

void get_connections(char *server_addr, char *server_port, Params &params)
{
    addrinfo hints, *servinfo, *p;
    int rv, numbytes, broadcast;
    int sockfd;
    
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(server_addr, server_port, &hints, &servinfo)) != 0) 
    {
		fprintf(stderr, "Client::get_connections(): getaddrinfo: %s\n", gai_strerror(rv));
		return;
	}

	for(p = servinfo; p != NULL; p = p->ai_next) 
    {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
        {
			perror("Client::get_connections(): socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
        {
			close(sockfd);
			perror("Client::get_connections(): connect");
			continue;
		}
        params.sockfd_game = sockfd;

		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
        {
			perror("Client::get_connections(): socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
        {
			close(sockfd);
			perror("Client::get_connections(): connect");
			continue;
		}
        params.sockfd_chat = sockfd;
		
        break;
	}

	if (p == NULL) 
    {
		fprintf(stderr, "Client::get_connections(): failed to connect\n");
		return;
	}

	freeaddrinfo(servinfo);

#ifdef DEBUG
    char s[100];
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
	fprintf(stderr, "Connecting to %s with FDs %d, %d.\n", s, params.sockfd_game, params.sockfd_chat);
#endif

}

int first_contact(Params params, char player_char, char player_name[])
{
    Packet packet;
    int rv, sockfd;
    
    sockfd = params.sockfd_chat;
    packet.packet_type = TYPE_CHAT;
    packet.message[0] = player_char;
    strcpy(packet.message + 1, player_name);
    
    send_packet(sockfd, &packet);

#ifdef DEBUG
    fprintf(stderr, "Sent identificaiton for chat line.\n");
#endif
    
    sockfd = params.sockfd_game;
    packet.packet_type = TYPE_GAME;
    send_packet(sockfd, &packet);

#ifdef DEBUG
    fprintf(stderr, "Sent identificaiton for game line.\n");
    fprintf(stderr, "Sent character \'%c\', name \"%s\".\n", player_char, player_name);
#endif

    recv_packet(sockfd, &packet);

#ifdef DEBUG
    fprintf(stderr, "Got ID %d.\n", packet.player_id);
#endif

    return packet.player_id;    
}

Params client_init(char player_char, char player_name[])
{
    /*
     * * Send broadcast.
     * * Receive UDP replies from server.
     * * Request and establish two connections with server.
     * Get map and players from server and initialise local structures.
     */
    
    int udp_sockfd;
    Params params;
    sockaddr_storage sock;
    char *server_port, *server_addr;
    socklen_t addrlen = sizeof(sock);

    udp_sockfd = get_udp_broadcast_socket();
    broadcast_it(udp_sockfd);
    get_server_tcp_port(udp_sockfd, server_addr, server_port, addrlen, sock);
    
    get_connections(server_addr, server_port, params);

    params.player_id = first_contact(params, player_char, player_name);   
    
    












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
    
    char player_char = '@';
    char player_name[] = "gilmagunaa";

    client_init(player_char, player_name);

    return 0;
}
