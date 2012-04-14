/*
 * mask.cpp - Implements visibility functions
 */

#include"mask.h"
using namespace std;

Mask::Mask()
{
        REP(i, MAP_MAXX)
                REP(j, MAP_MAXY)
                    vis[i][j] = 0; 
}

bool Mask::getvis(int x, int y)
{
        return vis[x][y];
}

void Mask::update(int posx, int posy)
{
    FOR(i, posx - DIST_VIS, posx + DIST_VIS + 1)
            FOR(j, posy - DIST_VIS, posy + DIST_VIS + 1)
                if(0 <= i && i < MAP_MAXX && 0 <= j && j < MAP_MAXY)
                    vis[i][j] = 1; 
}

void Mask::exchange(Mask *mask)
{
        REP(i, MAP_MAXX)
                REP(j, MAP_MAXY)
                    vis[i][j] = (mask -> vis[i][j]) = (vis[i][j] | (mask -> vis[i][j])); 
}

