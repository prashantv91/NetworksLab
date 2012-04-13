#include "chat.h"
#include <cstdio>
#include <iostream>
#include <allegro.h>
using namespace std;

extern Game game;
extern bool game_running;

void pack_chat(char **chatroll, int &top, char player_char, char player_name[], char message[], int num_chars_per_line, int num_lines)
{
    int ind = 0;
    char *new_message = new char[3 + strlen(player_name) + 2 + strlen(message) + 1];

    new_message[0] = '(';
    new_message[1] = player_char;
    new_message[2] = ')';
    strcpy(new_message+3, player_name);
    new_message[strlen(player_name)+3] = ':';
    new_message[strlen(player_name)+4] = ' ';
    strcpy(new_message+strlen(player_name)+5, message);

    FOR(i, 0, strlen(new_message))
    {
        chatroll[top][ind] = new_message[i];
        ind++;
        if (ind >= num_chars_per_line)
        {
            ind = 0;
            top++;
            if (top >= num_lines)
                top = 0;
        }
    }
    
    if (ind != 0)
    {
        chatroll[top][ind] = '\0';
        top++;
    }
    if (top >= num_lines)
        top = 0;

}

void draw_chat(char **chatroll, int top, int chat_startx, int chat_starty, int num_chars_per_line, int num_lines)
{
    int cw = CHAR_WIDTH;
    BITMAP *bmp = create_bitmap(SCREEN_X, SCREEN_Y);

    FOR(i, 0, num_lines)
        FOR(j, 0, num_chars_per_line)
        {
            if (!chatroll[(top+i)%num_lines][j])
                break;
            textprintf_ex(bmp, font, chat_startx + cw*(j+2), chat_starty + cw*(i+1), red, -1, "%c", chatroll[(top+i)%num_lines][j]);  //x and y interchanged due to differing conventions.
        }
    
    blit(bmp, screen, chat_startx + 2*cw, chat_starty + cw, chat_startx + cw + cw/2, chat_starty + cw, cw*num_chars_per_line, cw*num_lines);
}

void* chat_fn(void *args)
{
    Params params = *(Params*)(args);
    int sockfd = params.sockfd_chat;
    Player player;
    Packet packet;
    
    int cw = CHAR_WIDTH, mcw = MAP_CHAR_WIDTH, pcl = PLAYER_CHAT_LINES;
    int chat_startx = mcw*(MAP_MAXX + 4), chat_endx = SCREEN_X;
    int chat_starty = 0, chat_endy = SCREEN_Y;
        
    int num_chars_per_line = (chat_endx - chat_startx - 2*cw) / cw;
    int num_lines = (chat_endy - chat_starty - (pcl+4)*cw) / cw;

    char **chatroll = new char*[num_lines];
    int roll_top = 0;
    
    FOR(i, 0, num_lines)
    {
        chatroll[i] = new char[num_chars_per_line + 1];
        chatroll[i][0] = chatroll[i][num_chars_per_line] = '\0';
    }
    
    while (recv_packet(sockfd, &packet))
    {
        //pthread_mutex_lock(&lock);
        if (!game_running)
            break;
        //pthread_mutex_unlock(&lock);

        if (packet.packet_type == TYPE_CHAT)
        {
            player = game.players[packet.player_id];
            pack_chat(chatroll, roll_top, player.get_char(), player.get_name(), packet.message, num_chars_per_line, num_lines);
            draw_chat(chatroll, roll_top, chat_startx, chat_starty, num_chars_per_line, num_lines);
            printf("---CHAT--- %c%s: %s\n", player.get_char(), player.get_name(), packet.message);
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

