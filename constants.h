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

#define SCREEN_X 800
#define SCREEN_Y 600

#define CHAR_WIDTH 9
#define MAP_CHAR_WIDTH 10
#define PLAYER_CHAT_LINES 4

#define GAME_DURATION 10
#define MAP_MAXX 50
#define MAP_MAXY 50
#define MAX_PLAYERS 10
#define DIST_VIS 2

#define GIVE_UP 10000

enum direction {UP, LEFT, DOWN, RIGHT};
enum pkt_type {TYPE_GAME, TYPE_CHAT, TYPE_BROADCAST, TYPE_REPLY, TYPE_START, TYPE_STOP, TYPE_MAP};
#define PKT_MSG_SIZE 101
#define FOR_EACH_DIR(dir)  for (direction dir = UP; dir <= RIGHT; dir++)


/* Default values used by server */
#define SERVER_UDP_PORT "2718" 
#define SERVER_TCP_PORT "3142" 

#define NUM_PLAYERS 2

#define MAP_plain '.'
#define MAP_wall '#'

#define CLIENT_PORT "3141"
#define BROADCAST_ADDRESS "255.255.255.255"

#define PLAYER_NAME_SIZE 21

#define black       makecol( 0,   0,   0   )
#define grey        makecol( 128, 128, 128 )
#define white       makecol( 254, 254, 254 )
#define red         makecol( 255, 0,   0   )
#define green       makecol( 0,   255, 0   )
#define blue        makecol( 0,   0,   255 )
#define yellow      makecol( 255, 255, 0   )
#define orange      makecol( 255, 128, 0   )
#define cyan        makecol( 0,   255, 255 )
#define purple      makecol( 255, 0,   255 )
#define light_blue  makecol( 128, 128, 255 )
#define pink        makecol( 255, 128, 128 )
#define light_green makecol( 128, 255, 128 )
#define dark_purple makecol( 128, 0,   128 )

#endif

