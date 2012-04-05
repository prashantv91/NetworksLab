/*
 * player.h - The Player class which contains information for each player.
 */

#ifndef PLAYER_H

#define PLAYER_H

#include "constants.h"
#include "position.h"

class Player
{
    int player_id;
    char player_char;
    char player_name[PLAYER_NAME_SIZE];
    Position player_pos;
    //bool map_mask[MAP_MAXX][MAP_MAXX];

    public:

    Player();
    Player(int pl_id, char pl_ch, int x, int y);
    Player(int pl_id, char pl_ch, Position pos);
    Player(const Player & P);
    ~Player();

    int get_id() const;
    char get_char() const;
    char *get_name(); 
    Position get_pos() const;
    void set_pos(const Position& pos);
    void move(direction dir);
    void look();
    void set_id(int);
    void set_char(char);
    void set_name(char *); 
};

#endif

