/*
 * server.cpp - Handles all the clients
 */

#include"server.h"

int backlog = 20;  // number of pending connections

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
        printf("Game server listening on port %s...\n", port);
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

int check_connection(int sockfd)
{
    return 0;
}

void receive_players()
{
    int sockfd_udp, sockfd_tcp, cnt = 0, val; 
    create_socket(&sockfd_udp, SERVER_UDP_PORT, 0); 
    create_socket(&sockfd_tcp, SERVER_TCP_PORT, 1); 

    while(1)
    {
        check_broadcast(sockfd_udp); 
        val = check_connection(sockfd_tcp); 
        cnt += val; 
        if(cnt == NUM_PLAYERS)
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

int main()
{
    server_init(); 
    receive_players(); 
    return 0; 
}
