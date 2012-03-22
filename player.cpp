/*
 * player.cpp - Implements the Player class.
 */

#include "player.h"

/* Constructors */

Player::Player()
{
    player_id = 0;
    player_char = '0';
    player_pos.x = player_pos.y = 0;
}

Player::Player(int pl_id, char pl_ch, int x, int y)
{
    player_id = pl_id;
    player_char = pl_ch;
    player_pos.x = x;
    player_pos.y = y;
}

Player::Player(int pl_id, char pl_ch, Position pos)
{
    player_id = pl_id;
    player_char = pl_ch;
    player_pos.x = pos.x;
    player_pos.y = pos.y;
}

Player::Player(const Player& P)
{
    player_id = P.player_id;
    player_char = P.player_char;
    player_pos.x = P.player_pos.x;
    player_pos.y = P.player_pos.y;
}

Player::~Player()
{
}

/* Functions */

int Player::get_id() const
{
    return player_id;
}

char Player::get_char() const
{
    return player_char;
}

Position Player::get_pos() const
{
    return player_pos;
}

void Player::set_pos(const Position& pos)
{
    player_pos = pos;
}

void Player::move(direction dir)
{
    player_pos.move(dir);
}

