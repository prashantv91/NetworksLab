/*
 * position.h - The Position class which maintains indices into the map.
 */


#ifndef POSITION_H

#define POSITION_H

#include "constants.h"

struct Position
{
    int x, y;
    
    Position();
    Position(int xx, int yy);
    Position(const Position& P);
    ~Position();

    void move(direction dir);

};

#endif

