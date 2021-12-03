#ifndef UTILITIES_H
#define UTILITIES_H
#include <Windows.h>
#include <string>
#define WIN32_LEAN_AND_MEAN

struct Vec2
{
    short X, Y;
};
inline bool operator != (Vec2 c1, Vec2 c2)
{
    return !((c1.X == c2.X) && (c1.Y == c2.Y));
}
inline bool operator == (Vec2 c1, Vec2 c2)
{
    return (c1.X == c2.X) && (c1.Y == c2.Y);
}
inline Vec2 operator += (Vec2 c1, Vec2 c2)
{
    return { short(c1.X + c2.X), short(c1.Y + c2.Y) };
}
inline Vec2 operator + (Vec2 c1, Vec2 c2)
{
    return { short(c1.X + c2.X), short(c1.Y + c2.Y) };
}
inline Vec2 operator*(Vec2 c1, int n)
{
    return { short(c1.X * n), short(c1.Y * n) };
}
inline float Distance(Vec2 c1, Vec2 c2)
{
    return pow(c1.X + c2.X, 2) + pow(c1.Y + c2.Y, 2);
}
inline wchar_t* charToWChar(const char* text)
{
    const size_t size = strlen(text) + 1;
    wchar_t* wText = new wchar_t[size];
#pragma warning(disable:4996)
    mbstowcs(wText, text, size);
    return wText;
}
inline bool InitConsole()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTitle(charToWChar("Stealth Game"));
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode))
        return false;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode))
        return false;
    CONSOLE_CURSOR_INFO cursor;
    cursor.bVisible = false;
    SetConsoleCursorInfo(hOut, &cursor);

    return true;
}

#endif // !UTILITIES_H
