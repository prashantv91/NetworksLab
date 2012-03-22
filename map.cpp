/*
 * map.cpp - Implementation of Map class.
 */

#include "map.h"
#include "position.h"
#include "player.h"
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cctype>
using namespace std;

/* Constructors */

Map::Map()
{
    REP(x, MAP_MAXX)
        REP(y, MAP_MAXY)
        {
            map_original[x][y] = MAP_plain;
            map[x][y] = MAP_plain;
        }
}

Map::Map(const char m[MAP_MAXX][MAP_MAXY])
{
    REP(x, MAP_MAXX)
        REP(y, MAP_MAXY)
        {
            map_original[x][y] = m[x][y];
            map[x][y] = m[x][y];
        }
}

Map::Map(const Map& M)
{
    REP(x, MAP_MAXX)
        REP(y, MAP_MAXY)
        {
            map_original[x][y] = M.map_original[x][y];
            map[x][y] = M.map[x][y];
        }
}

Map::Map(char filename[])
{
    char temp_map[MAP_MAXX][MAP_MAXY];
    char ch;
    fstream f;
    f.open(filename, ios::in);
    int x, y;

    x = y = 0;

    while (!f.eof())
    {
        ch = f.get();
        if (isspace(ch))
            continue;
        
        /*
        if (ch =='\n')
        {
            cout<<'n';
            x++;
            y = 0;

            if (x >= MAP_MAXX){cout<<"XX";
                break;}

            continue;
           
            while (isspace(ch))
                ch = f.get();
                
        }
        */
        temp_map[x][y] = ch;
        y++;
        if (y >= MAP_MAXY)
        {
            x++;
            y = 0;
        }
        if (x >= MAP_MAXX)
            break;

    }

    if (check_map(temp_map))
    {
        REP(i, MAP_MAXX)
            REP(j, MAP_MAXY)
                map_original[i][j] = map[i][j] = temp_map[i][j];
    }
    else
    {
        fprintf(stderr, "Map::Map(filename[]): Invalid map file. Loading blank map.\n");
        REP(x, MAP_MAXX)
            REP(y, MAP_MAXY)
            {
                map_original[x][y] = MAP_plain;
                map[x][y] = MAP_plain;
            }
    }

}

Map::~Map()
{
}

/* Functions */

bool Map::safe(Position pos)
{
    if (pos.x < 0 || pos.x >= MAP_MAXX)
        return false;
    if (pos.y < 0 || pos.y >= MAP_MAXY)
        return false;
    if (map[pos.x][pos.y] != MAP_plain)
        return false;

    return true;
}

bool Map::check_map(char m[MAP_MAXX][MAP_MAXY])
{
    REP(x, MAP_MAXX)
        REP(y, MAP_MAXY)
            if (m[x][y] != MAP_plain && m[x][y] != MAP_wall)
                return false;

    return true;
}

bool Map::move(Player* player, direction dir)
{
    Position pos = player->get_pos(), new_pos;
    char pl_char = player->get_char();

    if (map[pos.x][pos.y] != pl_char)          //If player object is lying about its position, abort.
        return false;

    new_pos = pos;
    new_pos.move(dir);
    if (!safe(new_pos))
        return false;

    map[pos.x][pos.y] = map_original[pos.x][pos.y];
    map[new_pos.x][new_pos.y] = pl_char;
    player->move(dir);

    return true;
}

void Map::print_map()
{
    REP(x, MAP_MAXX)
    {
        REP(y, MAP_MAXY)
            cout<<map[x][y];
        cout<<endl;
    }
    cout<<endl;
}

void Map::draw_map()
{
    return;
}

void Map::place_player(Player* pl, Position pos)
{
    if (!safe(pos))
    {
        fprintf(stderr, "Map::place_player: Invalid position specified. Not placing player.\n");
        return;
    }

    map[pos.x][pos.y] = pl->get_char();
    pl->set_pos(pos);

}

void Map::place_player_random(Player* pl)
{
    Position pos;
    int give_up_count = GIVE_UP;

    while (1)
    {
        pos.x = random()%MAP_MAXX;
        pos.y = random()%MAP_MAXY;
        if (safe(pos))
            break;
        give_up_count--;
        if (give_up_count == 0)
        {
            fprintf(stderr, "Map::place_player_random: Couldn't find place for player. Not placing player.\n");
            return;
        }

    }

    place_player(pl, pos);

}
