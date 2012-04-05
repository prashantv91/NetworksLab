/*
 * map.h - The Map class which maintains the map, handles movement and draws the map, etc.
 */

#ifndef MAP_H

#define MAP_H

#include "constants.h"
#include "position.h"
//        y
//    *--->
//    |
//    |
//  x v
// 

class Player;

class Map
{
    char map_original[MAP_MAXX][MAP_MAXY];           
    char map[MAP_MAXX][MAP_MAXY];           

    bool safe(Position pos);
    bool check_map(char m[MAP_MAXX][MAP_MAXY]);

    public:
    
    Map();
    Map(const char m[MAP_MAXX][MAP_MAXY]);
    Map(const Map & M);
    Map(char filename[]);
    ~Map();


    bool move(Player* player, direction dir);
    void print_map();
    void draw_map();
    void place_player(Player* pl, Position pos);
    void place_player_random(Player* pl);

};
#endif
