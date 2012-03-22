/*
 * constants.h - Various constants pertinent to the game.
 */


#ifndef CONSTANTS_H

#define CONSTANTS_H

#define FOR(a, b, c)   for (int a = int(b); a < int(c); a++)
#define RFOR(a, b, c)   for (int a = int(b)-1; a >= int(c); a--)
#define REP(a, b)  for (int a = 0; a < int(b); a++)

const int MAP_MAXX = 10;
const int MAP_MAXY = 10;

const int GIVE_UP = MAP_MAXX * MAP_MAXY;

enum direction {UP, LEFT, DOWN, RIGHT};

#define FOR_EACH_DIR(dir)  for (direction dir = UP; dir <= RIGHT; dir++)

const char MAP_plain = '.', MAP_wall = '#';

#endif

