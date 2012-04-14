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

extern bool game_running;
extern Game game;
bool game_won;
Mask mask[NUM_PLAYERS]; 
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
    fprintf(stderr, "Sent identification for chat line.\n");
#endif
    
    sockfd = params.sockfd_game;
    packet.packet_type = TYPE_GAME;
    send_packet(sockfd, &packet);

#ifdef DEBUG
    fprintf(stderr, "Sent identification for game line.\n");
    fprintf(stderr, "Sent character \'%c\', name \"%s\".\n", player_char, player_name);
#endif

    recv_packet(sockfd, &packet);

#ifdef DEBUG
    fprintf(stderr, "Got ID %d.\n", packet.player_id);
#endif

    return packet.player_id;    
}

void draw_outline()
{
    BITMAP *bmp = create_bitmap(SCREEN_X, SCREEN_Y);

    int mcw = MAP_CHAR_WIDTH, cw = CHAR_WIDTH;
    int chat_startx = mcw*(MAP_MAXX + 4), chat_endx = SCREEN_X;
    int chat_starty = 0, chat_endy = SCREEN_Y;
    int pcl = PLAYER_CHAT_LINES;

    cout<<"WHITE: "<<white<<' '<<makecol(128,0,128)<<endl;
    

    rectfill(bmp, 0, 0, cw, SCREEN_Y, white);
    rectfill(bmp, 0, 0, SCREEN_X, cw, white);
    rectfill(bmp, 0, SCREEN_Y-cw, SCREEN_X, SCREEN_Y, white);
    rectfill(bmp, SCREEN_X-cw, 0, SCREEN_X, SCREEN_Y, white);
        
    rectfill(bmp, chat_startx, chat_starty, chat_startx+cw, chat_endy, white);
    rectfill(bmp, chat_startx, chat_endy-cw*(pcl+3), chat_endx+cw, chat_endy-cw*(pcl+2), white);
    rectfill(bmp, 0, SCREEN_Y - cw*5, chat_startx, SCREEN_Y - cw*4, white);

    blit(bmp, screen, 0, 0, 0, 0, bmp->w, bmp->h);
    
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
    
    draw_outline();

    return params;

}

void* timer_fn(void *args)
{
    Params params = *(Params*)args;
    
    while (!game_running);

    int cw = CHAR_WIDTH;
    int time = game.time;
    char message[] = "Time till manatees";
    int x = cw*(strlen(message) + 4);
    int y = SCREEN_Y - 2*cw - cw/2;

    textprintf_ex(screen, font, cw*2, y, green, -1, "%s: ", message);

    while (time >= 0 && game_running)
    {
        rectfill(screen, x, y, x + cw*10, y + cw, black);
        textprintf_ex(screen, font, x, y, red, -1, "%d", time);

        sleep(1);
        time--;
    }

    
    //pthread_cancel(params.chat_thread);
    pthread_cancel(params.game_thread);
    //pthread_cancel(params.chat_thread);

    return NULL;

}

void* game_fn(void *args)
{
    Params params = *(Params*)(args);
    int sockfd = params.sockfd_game;
    int myId = params.player_id; 
    int numbytes, moveId;
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
    

    mask[myId].update(game.players[myId].get_pos().x, game.players[myId].get_pos().y); 
    game.map.draw_map(&mask[myId], game.players[myId].get_pos().x, game.players[myId].get_pos().y);

    //pthread_mutex_lock(&lock);
    game_running = true;
    //pthread_mutex_lock(&lock);

    game_won = false;
    game.map.draw_map();

    while (recv_packet(sockfd, &packet))
    {
        //pthread_mutex_lock(&lock);
        if (!game_running)
            break;
        //pthread_mutex_unlock(&lock);

        if (packet.packet_type == TYPE_GAME)
        {
            dir = *(direction*)(packet.message);
            moveId = packet.player_id; 
            player = &(game.players[moveId]);

            game.map.move(player, dir);

            // update mask for player who moved
            mask[moveId].update(game.players[moveId].get_pos().x, game.players[moveId].get_pos().y); 

            // check for adjacent players and update map
            for(int i = 0 ; i < game.num_players; i++)
                    if(abs(game.players[i].get_pos().x - game.players[moveId].get_pos().x) + \
                       abs(game.players[i].get_pos().y - game.players[moveId].get_pos().y) == 1)
                            mask[moveId].exchange(&mask[i]); 


            // show my map
            game.map.draw_map(&mask[myId], game.players[myId].get_pos().x, game.players[myId].get_pos().y);
            //game.map.print_map();
        }
        else
        if (packet.packet_type == TYPE_STOP)
        {
            if (*(bool*)(packet.message))
                game_won = true;
            else
                game_won = false;
            break;
        }
    }

    //pthread_mutex_lock(&lock);
    game_running = false;
    //pthread_mutex_unlock(&lock);
    
    //pthread_cancel(params.chat_thread);

#ifdef DEBUG
    fprintf(stderr, "Game thread exiting.\n");
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

    //BITMAP *bmp = create_bitmap(SCREEN_X, SCREEN_Y);
    int cw = CHAR_WIDTH, mcw = MAP_CHAR_WIDTH, pcl = PLAYER_CHAT_LINES;
    int chat_startx = mcw*(MAP_MAXX + 4), chat_endx = SCREEN_X;
    int chat_starty = 0, chat_endy = SCREEN_Y;
        
    int num_chars_per_line = (chat_endx - chat_startx - 2*cw) / cw;
    int num_lines = (chat_endy - chat_starty - (pcl+4)*cw) / cw;
    
    int pl_chat_startx = chat_startx + cw + cw/2, pl_chat_endx = chat_endx - cw - cw/2;
    int pl_chat_starty = chat_endy - cw*(pcl+2) + cw/2, pl_chat_endy = chat_endy - cw - cw/2;
    int pl_chat_x, pl_chat_y;
    //rectfill(bmp, chat_startx, chat_endy-cw*(pcl+3), chat_endx+cw, chat_endy-cw*(pcl+2), white);

    Params params = *(Params*)(args);
    Packet game_pkt, chat_pkt;

    int chat_ind = 0;
    char ch;
    /**/Player P = game.players[params.player_id];

    game_pkt.player_id = params.player_id;
    game_pkt.packet_type = TYPE_GAME;

    chat_pkt.player_id = params.player_id;
    chat_pkt.packet_type = TYPE_CHAT;
    
    pl_chat_x = pl_chat_startx;
    pl_chat_y = pl_chat_starty;

    while (game_running)
    {
        while (!keypressed());
        ch = readkey() % 256;
        if (ch > 0 && ch < 128)
        {

            if (ch !='\n' && ch != '\r' && (chat_ind < PKT_MSG_SIZE-1))
            {
                if (ch == '\b')
                {
                    if (pl_chat_x != pl_chat_startx || pl_chat_y != pl_chat_starty)
                    {
                        if (pl_chat_x != pl_chat_startx)
                        {
                            pl_chat_x -= cw;
                        }
                        else
                        {
                            pl_chat_x = pl_chat_endx - cw;
                            pl_chat_y -= cw;
                        }
                        rectfill(screen, pl_chat_x, pl_chat_y, pl_chat_x+cw, pl_chat_y+cw, black);
                    }
                }
                else
                if (ch >= 32 && ch <= 126)
                {
                    chat_pkt.message[chat_ind] = ch;
                    chat_ind++;
                    textprintf_ex(screen, font, pl_chat_x, pl_chat_y, light_green, -1, "%c", ch);  //x and y interchanged due to differing conventions.

                    pl_chat_x += cw;
                    if (pl_chat_x >= pl_chat_endx)
                    {
                        pl_chat_x = pl_chat_startx;
                        pl_chat_y += cw;
                        if (pl_chat_y >= pl_chat_starty + pcl*cw)
                        {
                            chat_pkt.message[chat_ind] = 0;
                            chat_ind = 0;
                            pl_chat_x = pl_chat_startx;
                            pl_chat_y = pl_chat_starty;
                            rectfill(screen, pl_chat_startx, pl_chat_starty, pl_chat_endx, pl_chat_endy, black);

                            send_packet(params.sockfd_chat, &chat_pkt);

                        }
                    }
                }
                //cerr<<s<<endl;
            }
            else
            {
                chat_pkt.message[chat_ind] = 0;
                chat_ind = 0;
                pl_chat_x = pl_chat_startx;
                pl_chat_y = pl_chat_starty;
                rectfill(screen, pl_chat_startx, pl_chat_starty, pl_chat_endx, pl_chat_endy, black);
               
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

    pthread_cancel(params.chat_thread);
    pthread_cancel(params.game_thread);

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

int main(int argc, char *argv[])
{
    Params params;
    char player_char;
    char player_name[PLAYER_NAME_SIZE]; 
    
    if(argc != 3)
    {
            cout<<"Error. Usage: ./client [PLAYER_CHAR] [PLAYER_NAME]";
            return 1;
    }
    player_char = *argv[1]; 
    strcmp(player_name, argv[2]);
    
    start_allegro();    
        
    game_running = false;

    params = client_init(player_char, player_name);

    
    pthread_create(&params.chat_thread, NULL, &chat_fn, (void*)&params);
    pthread_create(&params.game_thread, NULL, &game_fn, (void*)&params);
    pthread_create(&params.keyboard_thread, NULL, &keyboard_fn, (void*)&params);
    pthread_create(&params.timer_thread, NULL, &timer_fn, (void*)&params);

    pthread_join(params.game_thread, NULL);
    
    rectfill(screen, MAP_CHAR_WIDTH, MAP_CHAR_WIDTH, MAP_CHAR_WIDTH*(MAP_MAXX+3), MAP_CHAR_WIDTH*(MAP_MAXY+3), black);
    if (game_won)
        textprintf_centre_ex(screen, font, (MAP_CHAR_WIDTH*MAP_MAXX)/2, (MAP_CHAR_WIDTH*MAP_MAXY)/2, green, -1, "BIG SUCCESS!");
    else
        textprintf_centre_ex(screen, font, (MAP_CHAR_WIDTH*MAP_MAXX)/2, (MAP_CHAR_WIDTH*MAP_MAXY)/2, red, -1, "YOU FOR MISTAKE.");

    pthread_join(params.chat_thread, NULL);
    pthread_join(params.timer_thread, NULL);
    pthread_join(params.keyboard_thread, NULL);

    
    while(!keypressed());

    fprintf(stderr, "EXITING.\n");

    return 0;
}
