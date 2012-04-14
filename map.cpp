/*
 * map.cpp - Implementation of Map class.
 */

#include "map.h"
#include "position.h"
#include "player.h"
#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cctype>
#include <allegro.h>
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

Map::Map(int posx, int posy)
{
        int area, covered_area = 0; 
        int sx, sy;

        h = posx, w = posy; 
        sx = (MAP_MAXX - h) / 2;
        sy = (MAP_MAXY - w) / 2;

        iter = 0; 
        init_shapes(); 
        srand(time(0)); 
        REP(i, MAP_MAXX) 
                REP(j, MAP_MAXY) 
                    map[i][j] = map_original[i][j] = MAP_wall; 

        area = posx * posy; 

        while(3*covered_area < 2*area)
                covered_area += fill_map();

        REP(i, posx)
                REP(j, posy)
                {
                        assert(0 <= (i + sx) && (i + sx) < MAP_MAXX); 
                        assert(0 <= (j + sy) && (j + sy) < MAP_MAXY); 
                        map_original[i+sx][j+sy] = map[i][j]; 
                }

        REP(i, MAP_MAXX)
                REP(j, MAP_MAXY)
                        map[i][j] = map_original[i][j];

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
    {
        return false;
    }
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

void Map::draw_map(Mask *mask, int posx, int posy)
{
    int mcw = MAP_CHAR_WIDTH;
    int map_startx = mcw, map_height = mcw*(MAP_MAXX+1);
    int map_starty = mcw, map_width = mcw*(MAP_MAXY+1);
    BITMAP *bmp = create_bitmap(SCREEN_X, SCREEN_Y);

    //rectfill(screen, 0, 0, MAP_MAXY*10, MAP_MAXX*10, makecol(0,0,0) );
    //clear_bitmap(screen);
    REP(x, MAP_MAXX)
        REP(y, MAP_MAXY)
                if((mask -> getvis(x, y)) && (map[x][y] == '.' || map[x][y] == '#' || (abs(x-posx) <= DIST_VIS && abs(y-posy) <= DIST_VIS)))
                        textprintf_ex(bmp, font, (y+1)*mcw, (x+1)*mcw, light_green, -1, "%c", map[x][y]);  
                else if(mask -> getvis(x, y))
                        textprintf_ex(bmp, font, (y+1)*mcw, (x+1)*mcw, light_green, -1, ".");  

    blit(bmp, screen, 0, 0, map_starty, map_startx, map_width, map_height);
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

void Map::init_shapes()
{
        shapes.clear(); 
        vector<pair<int, int> > shape; 

        // Horizontal line
        shape.clear(); 
        shape.push_back(make_pair(0, 0)); 
        shape.push_back(make_pair(0, 1)); 
        shape.push_back(make_pair(0, -1)); 
        shape.push_back(make_pair(0, 2)); 
        shape.push_back(make_pair(0, -2)); 
        shapes.push_back(shape); 

        // Vertical line
        shape.clear(); 
        shape.push_back(make_pair(0, 0)); 
        shape.push_back(make_pair(1, 0)); 
        shape.push_back(make_pair(-1, 0)); 
        shape.push_back(make_pair(2, 0)); 
        shape.push_back(make_pair(-2, 0)); 
        shapes.push_back(shape); 

        // Box 
        shape.clear(); 
        shape.push_back(make_pair(0, 0)); 
        shape.push_back(make_pair(0, 1)); 
        shape.push_back(make_pair(0, 2)); 
        shape.push_back(make_pair(0, 3)); 
        shape.push_back(make_pair(1, 0)); 
        shape.push_back(make_pair(1, 1)); 
        shape.push_back(make_pair(1, 2)); 
        shape.push_back(make_pair(1, 3)); 
        shape.push_back(make_pair(2, 0)); 
        shape.push_back(make_pair(2, 1)); 
        shape.push_back(make_pair(2, 2)); 
        shape.push_back(make_pair(2, 3)); 
        shapes.push_back(shape); 
}

int dx[] = {0, 1, 0, -1};
int dy[] = {1, 0, -1, 0}; 

bool Map::check(int x, int y)
{
        return (1 <= x && x < h - 1 && 1 <= y && y < w - 1);
}

void Map::choose_posn(int& x, int& y)
{
    bool f = false;
    while(!f){
            int x1 = 1 + rand() % (h-1) ; 
            int y1 = 1 + rand() % (w-1) ; 
            
            REP(k, 4){
                    int tx = x1 + dx[k];
                    int ty = y1 + dy[k]; 
                    if(!check(tx, ty)) continue; 
                    if(map[tx][ty] == '#' && iter > 0) continue; 

                    x = x1;
                    y = y1;
                    f = true; 
                    break;
            }
    }
}

int Map::fill_map()
{
        int covered_area = 0; 
        int x, y;

        choose_posn(x, y); 
        int i = rand() % ((int) shapes.size()); 
        //int i = iter % 2; 
        bool f = true; 
        REP(j, shapes[i].size())
        {
                int tx = x + shapes[i][j].first;
                int ty = y + shapes[i][j].second;  
                if(!check(tx, ty) || map[tx][ty] == '.')
                {
                        f = false;
                        break;
                }
        }
        if(f)
        {
                covered_area += shapes[i].size(); 
                //cout<<covered_area<<endl;
                iter++; 
                //cout<<x<<" "<<y<<endl;
                REP(j, shapes[i].size())
                {
                        int tx = x + shapes[i][j].first;
                        int ty = y + shapes[i][j].second; 
                        map[tx][ty] = '.'; 
                }
        }
        return covered_area; 
}

