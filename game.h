/*
 * game.h - The Game structure. Eacn client and the server has one.
 */

#ifndef GAME_H

#define GAME_H

#include "map.h"
#include "player.h"

struct Game
{
    bool running;
    int num_players;
    Map map;
    Player players[MAX_PLAYERS];  
    int time;
};

#endif

