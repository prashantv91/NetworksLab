/*
 * player.h - The Player class which contains information for each player.
 */

class Player
{
    int player_id;
    char player_char;

    public:

    Player();
    Player(int pl_id, char pl_ch);
    Player(const Player & P);
    ~Player();

    int get_id() const;
    char get_char() const;

};
