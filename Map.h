#include <vector>
#include <string>
#include <Windows.h>
#ifndef MAP_H
#define MAP_H

using namespace std;
struct Color
{
    uint8_t r, g, b;
};

struct Cell
{
    unsigned char content;
    bool is_watched : 1;
    //Debugging
    bool is_path : 1;
    bool is_target : 1;
};
extern vector<vector<Cell>> map_data;
extern vector<string>game_map;
extern COORD player_location;
extern bool game_running;
inline Cell& GetCell(COORD pos) 
{ 
    return map_data.at(pos.Y).at(pos.X); 
}
inline bool MoveCollision(COORD pos)
{
    return GetCell(pos).content != ' ';
}
inline bool CheckCollision(COORD pos) 
{ 
    return GetCell(pos).content == '#' || GetCell(pos).content == 'G';
}

#endif // !MAP_H