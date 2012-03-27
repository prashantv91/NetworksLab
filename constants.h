/*
 * constants.h - Various constants pertinent to the game.
 */


#ifndef CONSTANTS_H

#define CONSTANTS_H

#define DEBUG

#define FOR(a, b, c)   for (int a = int(b); a < int(c); a++)
#define RFOR(a, b, c)   for (int a = int(b)-1; a >= int(c); a--)
#define REP(a, b)  for (int a = 0; a < int(b); a++)
#define debug(x) cerr<<#x

const int SCREEN_X = 640;
const int SCREEN_Y = 480;

const int MAP_MAXX = 10;
const int MAP_MAXY = 10;
const int MAX_PLAYERS = 10;

const int GIVE_UP = MAP_MAXX * MAP_MAXY;

enum direction {UP, LEFT, DOWN, RIGHT};
enum pkt_type {TYPE_GAME, TYPE_CHAT, TYPE_BROADCAST, TYPE_REPLY, TYPE_START, TYPE_STOP, TYPE_MAP};
const int PKT_MSG_SIZE = 101;

#define FOR_EACH_DIR(dir)  for (direction dir = UP; dir <= RIGHT; dir++)

const char MAP_plain = '.', MAP_wall = '#';

#endif

