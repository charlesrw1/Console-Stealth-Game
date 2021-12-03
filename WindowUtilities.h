#ifndef WINDOWUTILITIES_H
#define WINDOWUTILITIES_H
#include <string>
#include <Windows.h>
#define WIN32_LEAN_AND_MEAN
using namespace std;
struct Color
{
    uint8_t r=200, g=200, b=200;
};
inline string ConvertToChars(uint8_t num)
{
    string answer;
    do
    {
        uint8_t temp = num % 10;
        answer.insert(answer.begin(), '0' + temp);
        num /= 10;
    } while (num > 0);
    return answer;
}
inline string FgColor(Color color)
{
    string str = "\x1b[38;2;;;m";
    str.insert(9, ConvertToChars(color.b));
    str.insert(8, ConvertToChars(color.g));
    str.insert(7, ConvertToChars(color.r));
    return str;
}
inline string FgColor(uint8_t r, uint8_t g, uint8_t b)
{
    string str = "\x1b[38;2;;;m";
    str.insert(9, ConvertToChars(b));
    str.insert(8, ConvertToChars(g));
    str.insert(7, ConvertToChars(r));
    return str;
}
inline string BgColor(Color color)
{
    string str = "\x1b[48;2;;;m";
    str.insert(9, ConvertToChars(color.b));
    str.insert(8, ConvertToChars(color.g));
    str.insert(7, ConvertToChars(color.r));
    return str;
}
inline string BgColor(uint8_t r, uint8_t g, uint8_t b)
{
    string str = "\x1b[48;2;;;m";
    str.insert(9, ConvertToChars(b));
    str.insert(8, ConvertToChars(g));
    str.insert(7, ConvertToChars(r));
    return str;
}
inline string Reset()
{
    return "\x1b[!p";
}
struct ColorString
{
    ColorString(string s = ""): string(s) {}
    string string;
    Color fg_color;
};
struct TableBelow
{
    TableBelow() : lines(5) {}
    vector<ColorString> lines;
}BelowTable;

inline BOOL gotoxy(const WORD x, const WORD y) {
    COORD xy;
    xy.X = x;
    xy.Y = y;
    return SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), xy);
}
inline void HideCursor()
{
    cout << "\x1b[?25l";
}
inline void DrawWindow(const string& s)
{
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), s.c_str(), s.size(), nullptr, nullptr);
}

#endif // !WINDOWUTILITIES_H

