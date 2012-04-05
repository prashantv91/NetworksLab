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


void client_main()
{
    Params params;
    GAME_RUNNING = true;
    keyboard(&params);
}
