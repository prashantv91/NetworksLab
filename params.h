/*
 * params.h - Structure to pass parameters while threading.
 */

#ifndef PARAMS_H
#define PARAMS_H

struct Params
{
    int sockfd_game;
    int sockfd_chat;
    int player_id;
    pthread_t game_thread, chat_thread, keyboard_thread, timer_thread;
};

#endif

