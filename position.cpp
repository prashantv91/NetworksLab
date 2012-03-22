/*
 * position.cpp - Implements the Position class.
 */

#include "position.h"

/* Constructors */

Position::Position()
{
    x = y = 0;
}

Position::Position(int xx, int yy)
{
    x = xx;
    y = yy;
}

Position::Position(const Position& P)
{
    x = P.x;
    y = P.y;
}

Position::~Position()
{
}


/* Functions */

void Position::move(direction dir)
{
    if (dir == UP)
        x--;
    if (dir == LEFT)
        y--;
    if (dir == DOWN)
        x++;
    if (dir == RIGHT)
        y++;
}

