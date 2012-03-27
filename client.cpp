/*
 * client.cpp - Implemets client functions.
 */

#include "client.h"
#include <iostream>
using namespace std;

bool GAME_RUNNING = false;

void game_thread(void *args)
{
    return;

}

void chat_thread(void *args){}


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
    for (int i = 0 ; i < 20; i++)
    {
        while (!keypressed());
        ch = readkey() % 256;
        if (ch > 0 && ch < 128)
        {
            if (ch !='\n' && (chat_ind % PKT_MSG_SIZE != PKT_MSG_SIZE-1))
            {
                s[chat_ind] = ch;
                chat_ind++;
            }
            else
            {
                s[chat_ind] = 0;
                chat_ind++;
                if (chat_ind == 10*PKT_MSG_SIZE)
                    chat_ind = 0;
#ifdef DEBUG
                cerr<<s<<endl;
#endif
            }

        }
        
    }
}


void client_main()
{
    GAME_RUNNING = true;
    keyboard(NULL);
}
