/*
 * server.cpp - Handles all the clients
 */

#include"server.h"
#include"game.h"
#include<iostream>
#include<cassert>
#include<queue>
using namespace std;

struct Params
{
        int conn_fd;
        Game *game;
}; 

extern Game game; 
bool server_game_running, game_won;
Params chat_params[2*MAX_PLAYERS], game_params[2*MAX_PLAYERS]; 
pthread_mutex_t chatLock, gameLock;                               // locks for sending information to client
int NUM_PLAYERS, CURR_MAX_PLAYERS;

int fds[2*MAX_PLAYERS]; 
int backlog = 5*MAX_PLAYERS;                                      // number of pending connections
int finished[MAX_PLAYERS];
int num_finished; 

void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void create_socket(int *sockfd, char *port, int conn_type)
{
    int rv, yes = 1; 
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;

	//fill up hints structure 
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; 

    if(conn_type == 1)
        hints.ai_socktype = SOCK_STREAM;
    else
        hints.ai_socktype = SOCK_DGRAM;

	hints.ai_flags = AI_PASSIVE; 

	if((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1); 
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if (((*sockfd)  = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
		{
			perror("server: socket");
			continue;
		}

		if (setsockopt((*sockfd), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
		{
			perror("setsockopt");
			exit(1);
		}

		if (bind((*sockfd), p->ai_addr, p->ai_addrlen) == -1) 
		{
			close((*sockfd));
			perror("server: bind");
			continue;
		}
		break;
	}

	if (p == NULL)  
	{
		fprintf(stderr, "server: failed to bind\n");
        exit(1); 
	}

	freeaddrinfo(servinfo); 

    if(conn_type == 1)
    {
        //start listening for connections 
        if (listen((*sockfd), backlog) == -1) 
        {
            perror("listen");
            exit(1);
        }
        printf("TCP server listening on port %s...\n", port);
    }
    else
        printf("UDP server up on port %s\n", port); 

}

void check_broadcast(int sockfd)
{
    int retval; 
    char s[100]; 
    fd_set rfds; 
    Packet packet; 
    struct timeval tv; 
	struct sockaddr_storage client_addr; 
    socklen_t addrlen = sizeof(client_addr); 

    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);
    tv.tv_sec = 0;
    tv.tv_usec = 100000; 
    retval = select(sockfd + 1, &rfds, NULL, NULL, &tv);

    if(retval != 0){
        recvfrom(sockfd, 
                &packet,
                sizeof(Packet), 
                0,
                (struct sockaddr *)&client_addr, 
                &addrlen); 

        printf("UDP Server: Received query from %s\n", inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)&client_addr), s, 100));
        packet.player_id = -1; 
        packet.packet_type = TYPE_REPLY; 
        strcpy(packet.message , SERVER_TCP_PORT); 
        sendto(sockfd, 
                &packet, 
                sizeof(Packet),
                0,
                (struct sockaddr *)&client_addr,
                addrlen); 
    }
}

void check_connection(int sockfd, int *cnt)
{
    int retval, new_fd; 
    char s[100]; 
    fd_set rfds; 
    struct timeval tv; 
	struct sockaddr_storage client_addr; 
    socklen_t addrlen = sizeof(client_addr); 

    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);
    tv.tv_sec = 0;
    tv.tv_usec = 100000; 
    retval = select(sockfd + 1, &rfds, NULL, NULL, &tv);

    if(retval != 0){
		// accept connection and find get new file descriptor
		new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &addrlen);
        //cout<<"new fd: "<<new_fd<<endl;
		if (new_fd == -1) 
		{
			perror("Server: accept");
            exit(1); 
		}
		
		//display details of client 
		inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)&client_addr), s, sizeof s);
		printf("Server: Accepted connection from %s\n", s);
        fds[*cnt] = new_fd; 
        (*cnt)++; 

    }
}

void send_ids()
{
    int new_fd, cnt = 0; 
    int chat_cnt = 0; 
    Player p;
    Packet packet; 

    for(int i = 0; i < 2*NUM_PLAYERS; i++)
    {
        new_fd = fds[i]; 
        if((recv(new_fd, 
                        &packet,
                        sizeof(Packet), 
                        0)) == -1)
        {
            perror("Server: recv");
        }

        if(packet.packet_type == TYPE_GAME)
        {
            game_params[cnt].conn_fd = new_fd;
            game_params[cnt].game = &game; 

            game.players[cnt].set_id(cnt); 
            game.players[cnt].set_char(packet.message[0]); 
            game.players[cnt].set_name(packet.message + 1); 
            printf("Server: %s has joined the game with character %c and id %d\n", game.players[cnt].get_name(), game.players[cnt].get_char(), game.players[cnt].get_id()); 

            game.map.place_player_random(&game.players[cnt]); 

            packet.player_id = cnt; 
            packet.packet_type = TYPE_REPLY; 

            if((send(new_fd, 
                            &packet, 
                            sizeof(Packet),
                            0)) == -1)
            {
                perror("Server: send"); 
            }
            cnt++; 
        }
        else
        {
            assert(packet.packet_type == TYPE_CHAT); 

            chat_params[chat_cnt].conn_fd = new_fd;
            chat_params[chat_cnt].game = &game; 
            chat_cnt++; 
        }
    }
    game.map.print_map(); 
}

void* timer_fn(void *args)
{
    while (!server_game_running);

    int time = GAME_DURATION;

    while (time >= 0)
    {
        sleep(1);
        time--;
    }

    server_game_running = false;

    return NULL;
}

void *game_callback(void *args)
{
    int conn_fd, ret; 
    Game *game; 
    Packet packet; 
    Params *params;
    direction dir; 

    params = (Params *)(args);
    conn_fd = params -> conn_fd;
    game = params -> game; 

    if((ret = send(conn_fd, game, sizeof(*game), 0)) == -1)
            perror("Server: send game info");

    packet.packet_type = TYPE_START;
    send(conn_fd, &packet, sizeof(packet), 0); 
    
    game_won = false;

    while(server_game_running)
    {
            if(recv(conn_fd, &packet, sizeof(packet), 0) <= 0)
            {
                for(int i = 0 ; i < NUM_PLAYERS ; i++)
                        if(game_params[i].conn_fd == conn_fd)
                                game_params[i].conn_fd = -1; 
                break;
            }

            dir = *(direction *) packet.message; 
            printf("Server: Received keystroke from player %d saying go in direction '%d'\n", packet.player_id, dir); 
            
            if(finished[packet.player_id])
                    printf("Server: Player %d has already reached the exit. Ignoring move.\n"); 
            //cout<<(game->players[packet.player_id]).get_pos().x<<" "<<(game->players[packet.player_id]).get_pos().y<<" "<<dir<<endl;
            else if((game->map).move(&(game->players[packet.player_id]), dir))
            {
                    if(game->map.isAtExit(game->players[packet.player_id].get_pos().x, game->players[packet.player_id].get_pos().y))
                    {
                            finished[packet.player_id] = 1;
                            num_finished++; 
                            if(num_finished == NUM_PLAYERS)
                            {
                                    server_game_running = false;
                                    game_won = true;
                            }
                    }
                    cout<<"Move valid. Broadcasting...\n";
                    pthread_mutex_lock(&gameLock);
                    for(int i = 0; i < NUM_PLAYERS ; i++)
                            if(game_params[i].conn_fd != -1)
                                    send(game_params[i].conn_fd, &packet, sizeof(packet), 0); 
                    pthread_mutex_unlock(&gameLock); 
            }
            game->map.print_map(); 
    }

    packet.packet_type = TYPE_STOP;
    if (game_won)
        *(bool*)(packet.message) = true;
    else
        *(bool*)(packet.message) = false;
    pthread_mutex_lock(&gameLock);
    for(int i = 0; i < NUM_PLAYERS ; i++)
        if(game_params[i].conn_fd != -1)
            send(game_params[i].conn_fd, &packet, sizeof(packet), 0); 
    pthread_mutex_unlock(&gameLock); 



}

bool filter(char *s)
{
        return true; 
}

void *chat_callback(void *args)
{
    Params *params = (Params *)(args);
    int conn_fd;
    Packet packet; 
    
    conn_fd = params -> conn_fd; 

    while(server_game_running)
    {
            if(recv(conn_fd, &packet, sizeof(packet), 0) <= 0)
            {
                for(int i = 0 ; i < NUM_PLAYERS ; i++)
                        if(chat_params[i].conn_fd == conn_fd)
                                chat_params[i].conn_fd = -1; 
                break;
            }

            printf("Server: Received chat from player %d saying '%s'\n", packet.player_id, packet.message); 

            if(filter(packet.message))                      
            {
                    pthread_mutex_lock(&chatLock);
                    for(int i = 0; i < NUM_PLAYERS ; i++)
                            if(chat_params[i].conn_fd != -1)
                                    send(chat_params[i].conn_fd, &packet, sizeof(packet), 0); 
                    pthread_mutex_unlock(&chatLock); 
            }
    }

}

void receive_players()
{
    int sockfd_udp, sockfd_tcp, cnt = 0;
    int prev, idle_cnt;


    create_socket(&sockfd_udp, SERVER_UDP_PORT, 0); 
    create_socket(&sockfd_tcp, SERVER_TCP_PORT, 1); 

    prev = cnt;
    idle_cnt = 0;

    while(1)
    {
        check_broadcast(sockfd_udp); 
        check_connection(sockfd_tcp, &cnt); 
        NUM_PLAYERS = cnt/2;
        
        if (prev == cnt)
            idle_cnt++;
        else
            idle_cnt = 0;
        prev = cnt;

        if (idle_cnt > SERVER_WAIT_TIME * 30)
            break;
        
        if(cnt == 2*CURR_MAX_PLAYERS)
            break;
    }
}

void server_init()
{
	struct sigaction sa;
	sa.sa_handler = sigchld_handler; 
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
		perror("sigaction");
		exit(1);
	}
}

void game_init()
{
    srand(time(0)); 
    game.map = Map(35, 35); 
    game.num_players = MAX_PLAYERS; 
    game.map.place_exit(); 
    game.time = GAME_DURATION;
    game.running = true; 
    server_game_running = true;
    num_finished = 0; 
}

void create_threads()
{
    pthread_t *threadObj; 
    pthread_t timer_thread;
    /* Create and start threads */
    threadObj = new pthread_t[2*NUM_PLAYERS];           

    for(int i = 0; i < NUM_PLAYERS ; i++)
    {
            pthread_create(&threadObj[2*i], NULL, game_callback, (void *)(&game_params[i])); 
            pthread_create(&threadObj[2*i+1], NULL, chat_callback, (void *)(&chat_params[i])); 
    }
    pthread_create(&timer_thread, NULL, &timer_fn, NULL);

    /* Wait for threads to finish */
    for(int i = 0; i < NUM_PLAYERS ; i++)
    {
            pthread_join(threadObj[2*i], NULL);
            pthread_join(threadObj[2*i+1], NULL);
    }
    pthread_join(timer_thread, NULL);

    delete[] threadObj; 
}

int main(int argc, char **argv)
{
    
    if(argc != 2)
    {
            cout<<"Usage: ./server MAX_PLAYERS\n";
            return 1;
    }

    CURR_MAX_PLAYERS = atoi(argv[1]);

    if (CURR_MAX_PLAYERS > MAX_PLAYERS)
    {
        fprintf(stderr, "Can accommodate only %d players.\n", MAX_PLAYERS);
        CURR_MAX_PLAYERS = MAX_PLAYERS;
    }

    game_init(); 
    server_init(); 
    receive_players(); 
    send_ids(); 
    create_threads(); 
    return 0; 
}
