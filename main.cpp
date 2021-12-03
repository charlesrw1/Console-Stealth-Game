#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "Map.h"
#include "Guard.h"
#include "Utilities.h"
#include "WindowUtilities.h"

using namespace std;

unsigned char polled_cmds[6] = { VK_ESCAPE, VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, VK_SPACE };
GameFlags GFlags;
string frame_buffer;
Vec2 player_location = { 1,1 };
vector<vector<Cell>> map_data;
vector<Guard*> game_guards;
vector<Vec2> alert_exclamations;

void DrawGameRules(int width)
{
    width += 5;
    cout << string(width, ' ') << BgColor(0x5f, 0xa9, 0x83) << FgColor(220, 220, 220) << "YOUR MISSION:\n";
    cout << Reset();
    cout << string(width, ' ') << FgColor(0xab, 0xdf, 0xe1) << "-Avoid the Guards, theyll hunt you down if you cross their sight\n";
    cout << string(width, ' ') << FgColor(0xab, 0xdf, 0xe1) << "-Use your knife, get behind the guards to take them out\n";
    cout << string(width, ' ') << FgColor(0xab, 0xdf, 0xe1) << "-Take out all the guards to win\n";
}
void InitMap()
{
    ifstream infile("Map-2.txt");
    string s;
    if (!infile)
        return;
    int y = 0;
    while (getline(infile, s))
    {
        vector<Cell> row;
        map_data.push_back(row);
        for (int i = 0; i < s.size(); i++)
        {
            Cell temp;
            temp.content = s.at(i);
            temp.is_path = false;
            temp.is_target = false;
            temp.is_watched = false;
            map_data.at(y).push_back(temp);
        }
        y++;
    }
    DrawGameRules(map_data.at(0).size());
    GetCell({ 1, 1 }).content = '@';
    game_guards.push_back(new Guard({ 18,2 }, { {2,7}, {12,1} }));
    game_guards.push_back(new Guard({ 8,5 }, { {4,2}, {15,4}, {10,7} }));
    game_guards.push_back(new Guard({ 4,13 }, { {15,3}, {24,7}, {16,9} }));
    game_guards.push_back(new Guard({ 24,13 }, { {15,3}, {10, 11}, {2,7} }));
}
void CheckGuardState()
{
    if (GFlags.alerted_flag)
    {
        for (auto i : game_guards)
            i->SetChase();

        GFlags.alerted_flag = false;

        GFlags.in_alert = true;
    }
    GFlags.in_alert = false;
    for (auto i : game_guards) {
        if (i->InChase())
            GFlags.in_alert = true;
    }
}
void DrawEndScreen()
{
    gotoxy(5, 5);
    if (GFlags.caught_flag)
        cout << BgColor(255, 0, 0) << FgColor(255, 255, 255) << "GAME OVER";
    else
        cout << BgColor(150, 90, 200) << FgColor(255, 255, 255) << "NICE JOB";

    GFlags.running = false;
    cout << Reset();
    gotoxy(0, map_data.size() + 2);
}
void DrawExclamations()
{
    for (auto i : alert_exclamations)
    {
        gotoxy(i.X, i.Y);
        cout << BgColor(255, 255, 255) << FgColor(0xec, 0x2e, 0x30) << '!' << Reset();
    }
    alert_exclamations.clear();
}
void DisplayMap()
{
    if (GFlags.caught_flag || GFlags.all_guards_dead) {
        DrawEndScreen();
        return;
    }
    frame_buffer.clear();
    for (short i = 0; i < map_data.size(); i++)
    {
        for (short j = 0; j < map_data.at(i).size(); j++)
        {
            if (GetCell({ j,i }).is_watched) 
                frame_buffer.append(BgColor(100,20,20));
            else if (GetCell({ j,i }).is_path) 
                frame_buffer.append(BgColor(0,70,0));

            if (GetCell({ j,i }).content == ' ' && GetCell({ j,i }).is_target)
                frame_buffer.append(FgColor(0, 190, 0)).push_back('X');
            else if (GetCell({ j,i }).content == '@')
                frame_buffer.append(FgColor(255, 255, 0)).push_back('@');
            else if (GetCell({ j,i }).content == 'G')
                frame_buffer.append(FgColor(0xab, 0xdf, 0xe1)).push_back('G');
            else
                frame_buffer.push_back(GetCell({ j,i }).content);

            frame_buffer += Reset();
        }
        frame_buffer += '\n';
    }
    if (GFlags.in_alert)
        frame_buffer.append(FgColor(150,0,0)).append("GUARDS ALERTED");
    else
        frame_buffer.append(FgColor(0,190,0)).append("ALL CLEAR     ");
    frame_buffer += Reset() + '\n';
    if (GFlags.backstab) {
        frame_buffer.append(FgColor(150, 90, 200)).append("BACKSTABBED");
        GFlags.backstab = false;
    }
    else
        frame_buffer.append("           ");
    frame_buffer += Reset() + '\n';
    gotoxy(0, 0);
    DrawWindow(frame_buffer);
    DrawExclamations();
    HideCursor();
}
const Guard* GetGuard(Vec2 pos)
{
    for (auto i : game_guards) {
        if (i->GetPos() == pos)
            return i;
    }
    return nullptr;
}
void DeleteGuard(const Guard* grd)
{
    for (int i = 0; i < game_guards.size(); i++) {
        if (game_guards.at(i) == grd) {
            delete game_guards.at(i);
            game_guards.erase(game_guards.begin() + i);
            return;
        }
    }
}
void TryMove(Vec2 change)
{
    if (player_location.Y + change.Y < 0 || 
        player_location.Y + change.Y >= map_data.size()) return;
    if (player_location.X + change.X < 0 ||
        player_location.X + change.X >= map_data.at(player_location.Y + change.Y).size()) return;
    if (WallCollision({ short(player_location.X + change.X), short(player_location.Y + change.Y) })) return;
    
    GetCell(player_location).content = ' ';
    Vec2 prev_pos = player_location;
    player_location = { short(player_location.X + change.X),short(player_location.Y + change.Y) };
    
    //Check for guard collision
    const Guard* grd = GetGuard(player_location);
    if (grd != nullptr && grd->BackFacing(prev_pos)) {
        GFlags.backstab = true;
        DeleteGuard(grd);
    }
    else if (grd != nullptr) {
        GFlags.caught_flag = true;
    }

    GetCell(player_location).content = '@';
}
void DoCommand(unsigned char cmd)
{
    switch (cmd)
    {
    case VK_ESCAPE: GFlags.running = false; break;
    case VK_UP: TryMove({ 0,-1 }); break;
    case VK_DOWN: TryMove({ 0,1 }); break;
    case VK_RIGHT: TryMove({ 1,0 }); break;
    case VK_LEFT: TryMove({ -1,0 }); break;
    case VK_SPACE: break; //Skip turn
    default: break;
    }
}
unsigned char PollKeyboard()
{
    for (int i = 0; i < 6; i++) {
        if (GetAsyncKeyState(polled_cmds[i]))
            return polled_cmds[i];
    }
    return 0x00;
}
void Update()
{
    for (auto i : game_guards)
        i->Update();
    CheckGuardState();
    if (game_guards.empty())
        GFlags.all_guards_dead = true;
}
int main()
{
    if (!InitConsole()) {
        printf("Failed console init");
        return -1;
    }
    system("cls");
    InitMap();
    DisplayMap();
    while (GFlags.running) {
        system("pause>nul");
        DoCommand(PollKeyboard());
        Update();
        DisplayMap();
    }
}
