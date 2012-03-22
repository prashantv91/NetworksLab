/*
 * map.h - The Map class which maintains the map, handles movement and draws the map, etc.
 */

#include "constants.h"

//        y
//    *--->
//    |
//    |
//  x v
// 

class Player;

class Map
{
    char map[MAP_MAXX][MAP_MAXY];           

    public:
    
    Map();
    Map(const char m[MAP_MAXX][MAP_MAXY]);
    Map(const Map & M);
    ~Map();

    bool move(Player* player, direction dir);
    void print_map();
    void draw_map();

};

