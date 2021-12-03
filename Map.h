#include <vector>
#include <string>
#include "Utilities.h"
#ifndef MAP_H
#define MAP_H
using namespace std;
struct Cell
{
    unsigned char content;
    bool is_watched : 1;
    bool is_path : 1;
    bool is_target : 1;
};
extern vector<Vec2> alert_exclamations;
extern vector<vector<Cell>> map_data;
extern Vec2 player_location;
struct GameFlags {
    bool running = true;
    bool caught_flag = false;
    bool all_guards_dead = false;
    bool alerted_flag = false;
    bool in_alert = false;
    bool backstab = false;
};
extern GameFlags GFlags;
inline Cell& GetCell(Vec2 pos) 
{ 
    return map_data.at(pos.Y).at(pos.X); 
}
inline bool MoveCollision(Vec2 pos)
{
    return GetCell(pos).content != ' ';
}
inline bool CheckCollision(Vec2 pos) 
{ 
    return GetCell(pos).content == '#' || GetCell(pos).content == 'G';
}
inline bool WallCollision(Vec2 pos)
{
    return GetCell(pos).content == '#';
}

#endif // !MAP_H