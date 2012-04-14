/*
 * map.h - The Map class which maintains the map, handles movement and draws the map, etc.  */ 
#ifndef MAP_H 
#define MAP_H 
#include <vector>
#include "constants.h"
#include "position.h"
#include "mask.h"
//using namespace std;
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

    int iter; 
    std::vector<std::vector<std::pair<int, int> > >shapes; 
    int h, w; 

    public:
    
    Map();
    Map(int, int);
    Map(const char m[MAP_MAXX][MAP_MAXY]);
    Map(const Map & M);
    Map(char filename[]);
    ~Map();

    bool check(int, int); 
    bool move(Player* player, direction dir);
    bool safe(Position pos);
    bool isAtExit(int, int); 
    bool check_map(char m[MAP_MAXX][MAP_MAXY]);

    int fill_map(); 
    void init_shapes();
    void print_map();
    void choose_posn(int&, int&); 
    void draw_map(Mask *, int, int);
    void place_player(Player* pl, Position pos);
    void place_player_random(Player* pl);
    void place_exit(); 

};
#endif
