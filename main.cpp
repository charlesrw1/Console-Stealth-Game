#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <wchar.h>
#include <windows.h>
#include <vector>
#include "Map.h"
#include "Guard.h"

using namespace std;

unsigned char polled_cmds[5] = { VK_ESCAPE, VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT };
bool game_running = true;
wstring frame_buffer;
COORD player_location = { 1,1 };
vector<vector<Cell>> map_data;
vector<Guard*> game_guards;


BOOL gotoxy(const WORD x, const WORD y) {
    COORD xy;
    xy.X = x;
    xy.Y = y;
    return SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), xy);
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
        //game_map.push_back(s);
        y++;
    }
    GetCell({ 1, 1 }).content = '@';
    GetCell({ 18, 2 }).content = 'G';
    GetCell({ 8, 5 }).content = 'G';
    game_guards.push_back(new Guard({ 18,2 }, { {2,7}, {12,1} }));
    game_guards.push_back(new Guard({ 8,5 }, { {4,2}, {15,4}, {10,7} }));
    game_guards.push_back(new Guard({ 4,13 }, { {15,3}, {24,7}, {16,9} }));
    game_guards.push_back(new Guard({ 24,13 }, { {15,3}, {10, 11}, {2,7} }));
}
//If one guard is alerted, they all are
bool CheckGuardState()
{
    for (auto i : game_guards) {
        if (i->InChase()) {
            for (auto i : game_guards)
                i->SetChase();
            return true;
        }
    }
    return false;
}
void DisplayMap()
{
    frame_buffer.clear();
    for (short i = 0; i < map_data.size(); i++)
    {
        for (short j = 0; j < map_data.at(i).size(); j++)
        {
            if (GetCell({ j,i }).is_watched) frame_buffer.append(L"\x1b[48;2;150;0;0m");
            else if (GetCell({ j,i }).is_path) frame_buffer.append(L"\x1b[48;2;0;100;0m");

            if(GetCell({ j,i }).content == ' ' && GetCell({ j,i }).is_target)frame_buffer.append(L"\x1b[38;2;0;190;0mX");
            else if (GetCell({ j,i }).content == '@') frame_buffer.append(L"\x1b[38;2;255;255;0m@");
            else frame_buffer.push_back(GetCell({ j,i }).content);

            frame_buffer.append(L"\x1b[!p");
        }
        frame_buffer.push_back('\n');
    }
    if (CheckGuardState())
        frame_buffer.append(L"\x1b[38;2;150;0;0mGUARDS ALERTED\x1b[!p");
    else
        frame_buffer.append(L"\x1b[38;2;0;190;0mALL CLEAR     \x1b[!p");
    gotoxy(0, 0);
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), frame_buffer.c_str(), frame_buffer.size(), nullptr, nullptr);
}
void TryMove(COORD change)
{
    if (player_location.Y + change.Y < 0 || 
        player_location.Y + change.Y >= map_data.size()) return;
    if (player_location.X + change.X < 0 ||
        player_location.X + change.X >= map_data.at(player_location.Y + change.Y).size()) return;
    if (CheckCollision({ short(player_location.X + change.X), short(player_location.Y + change.Y) })) return;
    GetCell(player_location).content = ' ';
    player_location = { short(player_location.X + change.X),short(player_location.Y + change.Y) };
    GetCell(player_location).content = '@';
}
void DoCommand(unsigned char cmd)
{
    switch (cmd)
    {
    case VK_ESCAPE: game_running = false; break;
    case VK_UP: TryMove({ 0,-1 }); break;
    case VK_DOWN: TryMove({ 0,1 }); break;
    case VK_RIGHT: TryMove({ 1,0 }); break;
    case VK_LEFT: TryMove({ -1,0 }); break;
    default: break;
    }
}
unsigned char PollKeyboard()
{
    for (int i = 0; i < 5; i++) {
        if (GetAsyncKeyState(polled_cmds[i]))
            return polled_cmds[i];
    }
    return 0x00;
}
static wchar_t* charToWChar(const char* text)
{
    const size_t size = strlen(text) + 1;
    wchar_t* wText = new wchar_t[size];
    #pragma warning(disable:4996)
    mbstowcs(wText, text, size);
    return wText;
}
bool InitConsole()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTitle(charToWChar("Space-Rouge"));
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode))
        return false;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode))
        return false;

    return true;
}
int main()
{
    if (!InitConsole()) {
        printf("Failed console init");
        return -1;
    }
    InitMap();
    while (game_running) {
        //WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), string.c_str(),
        //  string.length(), nullptr, nullptr);
        DisplayMap();
        system("pause>nul");
        DoCommand(PollKeyboard());
        for (auto i : game_guards)
            i->Update();
    }
}
