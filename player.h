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
    Position player_pos;

    public:

    Player();
    Player(int pl_id, char pl_ch, int x, int y);
    Player(int pl_id, char pl_ch, Position pos);
    Player(const Player & P);
    ~Player();

    int get_id() const;
    char get_char() const;
    Position get_pos() const;
    void set_pos(const Position& pos);
    void move(direction dir);
    

};

#endif

