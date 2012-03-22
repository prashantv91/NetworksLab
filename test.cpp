/*
 * test.cpp - Temporary file to test stuff.
 */

#include "map.h"
#include "position.h"
#include "player.h"
#include <allegro.h>
#include <iostream>
using namespace std;

int main()
{
    //allegro_init();
    //install_keyboard();

    Map M("maps/map1");
    Player P;
    M.place_player_random(&P);
    //readkey();
    //while (keypressed);
    M.print_map();
    
    /*
    while (1)
    {
        readkey();
        if (key[KEY_ESC])
            break;
        if (key[KEY_UP])
            M.move(&P, UP);
        else
        if (key[KEY_LEFT])
            M.move(&P, LEFT);
        else
        if (key[KEY_DOWN])
            M.move(&P, DOWN);
        else
        if (key[KEY_RIGHT])
            M.move(&P, RIGHT);
        M.print_map();
    }
    */
    /*
    M.move(&P, UP);
    M.print_map();
    M.move(&P, LEFT);
    M.print_map();
    M.move(&P, DOWN);
    M.print_map();
    M.move(&P, RIGHT);
    M.print_map();
    */
    return 0;   
}
