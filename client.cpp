/*
 * client.cpp - Implemets client functions.
 */

#include "client.h"
#include <iostream>
#include <cstdio>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <bits/pthreadtypes.h>
using namespace std;

bool game_running;
Game game;
pthread_mutex_t lock;

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
     * * Get map and players from server and initialise local structures.
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
    
    return params;

}

void* game_fn(void *args)
{
    Params params = *(Params*)(args);
    int sockfd = params.sockfd_game;
    int numbytes;
    direction dir;
    Player *player;
    Packet packet;

    if ((numbytes = recv(sockfd, &game, sizeof(game), 0)) < 0)
    {
        perror("Client::game_fn(): recv");
        return NULL;
    }
for (int i = 0; i < game.num_players; i++)
    cerr<<game.players[i].get_pos().x<<' '<<game.players[i].get_pos().y<<endl;
#ifdef DEBUG
    fprintf(stderr, "Received game. %d bytes. Number of players: %d.\n", numbytes, game.num_players);
#endif

    do
    {
        recv_packet(sockfd, &packet);
    } while (packet.packet_type != TYPE_START);

    game.map.draw_map();

    //pthread_mutex_lock(&lock);
    game_running = true;
    //pthread_mutex_lock(&lock);

    while (recv_packet(sockfd, &packet))
    {cerr<<"GAME PACKET\n";
        //pthread_mutex_lock(&lock);
        if (!game_running)
            break;
        //pthread_mutex_unlock(&lock);

        if (packet.packet_type == TYPE_GAME)
        {
            dir = *(direction*)(packet.message);
            player = &(game.players[packet.player_id]);
            game.map.move(player, dir);
            game.map.draw_map();
            //game.map.print_map();
        }
        else
        if (packet.packet_type == TYPE_STOP)
            break;
    }

    //pthread_mutex_lock(&lock);
    game_running = false;
    //pthread_mutex_unlock(&lock);

#ifdef DEBUG
    fprintf(stderr, "Game thread exiting.\n");
#endif

    return NULL;
}

void* chat_fn(void *args)
{
    Params params = *(Params*)(args);
    int sockfd = params.sockfd_chat;
    Player player;
    Packet packet;
    int chat_startx = SCREEN_X - 8*MAP_MAXX - 16, chat_width = (SCREEN_X - chat_startx)/8;
    int chat_starty = SCREEN_Y - 8*MAP_MAXY - 16, chat_height = (SCREEN_Y - chat_starty)/8;
    char **chatroll;
    
    

    while (recv_packet(sockfd, &packet))
    {
        //pthread_mutex_lock(&lock);
        if (!game_running)
            break;
        //pthread_mutex_unlock(&lock);

        if (packet.packet_type == TYPE_CHAT)
        {
            player = game.players[packet.player_id];
            /**/printf("---CHAT--- %c%s: %s\n", player.get_char(), player.get_name(), packet.message);
        }
    }

    //pthread_mutex_lock(&lock);
    game_running = false;
    //pthread_mutex_unlock(&lock);

#ifdef DEBUG
    fprintf(stderr, "Chat thread exiting.\n");
#endif

    return NULL;
}

void* keyboard_fn(void *args)
{

    //pthread_mutex_lock(&lock);
    while (!game_running)
    {
        //pthread_mutex_unlock(&lock);
        sleep(1);
        //pthread_mutex_lock(&lock);
    }
    //pthread_mutex_unlock(&lock);


    Params params = *(Params*)(args);
    Packet game_pkt, chat_pkt;

    int chat_ind = 0;
    char ch;
    /**/Player P = game.players[params.player_id];

    game_pkt.player_id = params.player_id;
    game_pkt.packet_type = TYPE_GAME;

    chat_pkt.player_id = params.player_id;
    chat_pkt.packet_type = TYPE_CHAT;
    
    while (game_running)
    {
        while (!keypressed());
        ch = readkey() % 256;
        if (ch > 0 && ch < 128)
        {

            if (ch !='\n' && ch != '\r' && (chat_ind < PKT_MSG_SIZE-1))
            {
                chat_pkt.message[chat_ind] = ch;
                chat_ind++;
#ifdef DEBUG
                textprintf_ex(screen, font, chat_ind*8, 0, makecol(255, 100, 200), -1, "%c", ch);  //x and y interchanged due to differing conventions.
                //cerr<<s<<endl;
#endif
            }
            else
            {
                chat_pkt.message[chat_ind] = 0;
                chat_ind = 0;

                send_packet(params.sockfd_chat, &chat_pkt);
            }
            
        }

        if (key[KEY_ESC])
            break;
        
        if (key[KEY_UP])
        {
            *(direction*)(game_pkt.message) = UP;
            send_packet(params.sockfd_game, &game_pkt);
        }
        else
        if (key[KEY_LEFT])
        {
            *(direction*)(game_pkt.message) = LEFT;
            send_packet(params.sockfd_game, &game_pkt);
        }
        else
        if (key[KEY_DOWN])
        {
            *(direction*)(game_pkt.message) = DOWN;
            send_packet(params.sockfd_game, &game_pkt);
        }
        else
        if (key[KEY_RIGHT])
        {
            *(direction*)(game_pkt.message) = RIGHT;
            send_packet(params.sockfd_game, &game_pkt);
        }
        
        //game.map.draw_map();
    }
    
    //pthread_mutex_lock(&lock);
    game_running = false;
    //pthread_mutex_unlock(&lock);

#ifdef DEBUG
    fprintf(stderr, "Keyboard thread exiting.\n");
#endif
    
    return NULL;
}

void start_allegro()
{
    allegro_init();
    install_keyboard();

    if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, SCREEN_X, SCREEN_Y, 0, 0)) 
    {
        allegro_message("Video Error: %s.\n", allegro_error);
        exit(1);
    }
}

int main()
{
    Params params;
    char player_char = '@';
    char player_name[] = "gilmagunaa";
    
    start_allegro();    
        
    game_running = false;

    params = client_init(player_char, player_name);

    pthread_t game_thread, chat_thread, keyboard_thread;
    
    pthread_create(&game_thread, NULL, &game_fn, (void*)&params);
    pthread_create(&chat_thread, NULL, &chat_fn, (void*)&params);
    pthread_create(&keyboard_thread, NULL, &keyboard_fn, (void*)&params);

    pthread_join(game_thread, NULL);
    pthread_join(chat_thread, NULL);
    pthread_join(keyboard_thread, NULL);

    fprintf(stderr, "EXITING.\n");

    return 0;
}
