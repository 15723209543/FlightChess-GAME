#pragma once
#include <graphics.h>
#include <string>

// 最大玩家数量。
const int MAX_PLAYERS = 6;
// 窗口宽度：主窗口尽量放大，保证地图和右侧信息区都有足够空间。
const int WINDOW_W = 1500;
// 窗口高度：给规则说明、按钮和骰子留出垂直空间。
const int WINDOW_H = 950;
// 地图左上角X坐标：控制左侧地图区的位置。
const int MAP_X = 20;
// 地图左上角Y坐标：控制左侧地图区的位置。
const int MAP_Y = 20;
// 地图宽度：放大棋盘，避免棋子和基地拥挤。
const int MAP_W = 960;
// 地图高度：放大棋盘，保证六边形地图完整显示。
const int MAP_H = 910;
// 右侧面板左上角X坐标：放在地图右侧。
const int PANEL_X = 1000;
// 右侧面板左上角Y坐标：和地图顶部对齐。
const int PANEL_Y = 20;
// 右侧面板宽度：放大输出框和规则说明区。
const int PANEL_W = 480;
// 右侧面板高度：和地图区域保持一致。
const int PANEL_H = 910;

const COLORREF COLOR_BG = RGB(236, 240, 246);       // 主背景色。
const COLORREF COLOR_PANEL = RGB(250, 251, 253);    // 右侧面板背景色。
const COLORREF COLOR_LINE = RGB(90, 96, 110);       // 主要线条颜色。
const COLORREF COLOR_TEXT = RGB(34, 38, 48);        // 正文文字颜色。
const COLORREF COLOR_MUTED = RGB(104, 112, 128);    // 次要文字颜色。
const COLORREF COLOR_WHITE = RGB(255, 255, 255);    // 白色。
const COLORREF COLOR_BLACK = RGB(0, 0, 0);          // 黑色。
const COLORREF COLOR_GOLD = RGB(244, 183, 64);      // 可选棋子的金色高亮。
const COLORREF COLOR_DANGER = RGB(202, 73, 64);     // 危险/减少按钮颜色。
const COLORREF COLOR_GOOD = RGB(64, 160, 99);       // 成功/增加按钮颜色。

extern const COLORREF PLAYER_COLORS[MAX_PLAYERS];   // 玩家颜色数组。

class button {
public:
    int x;                    // 按钮左上角X坐标。
    int y;                    // 按钮左上角Y坐标。
    int w;                    // 按钮宽度。
    int h;                    // 按钮高度。
    std::wstring text;        // 按钮显示文字。
    COLORREF bgcolor;         // 按钮背景色。
    COLORREF textcolor;       // 按钮文字颜色。
    COLORREF bordercolor;     // 按钮边框颜色。
    bool visible;             // 按钮是否可见。
    bool enabled;             // 按钮是否可点击。

    button();
    button(int x, int y, int w, int h, const std::wstring& text);
    void draw() const;
    bool inside(int mx, int my) const;
};

const wchar_t* player_name(int index);
std::wstring player_title(int index);
COLORREF mix_color(COLORREF a, COLORREF b, int percent);
void clear_screen(COLORREF color);
void draw_text_center(int x, int y, int w, int h, const std::wstring& text, COLORREF color, int size);
void draw_text_left(int x, int y, const std::wstring& text, COLORREF color, int size);
int draw_text_wrap(int x, int y, int w, const std::wstring& text, COLORREF color, int size, int lineheight, int maxlines);
void draw_hex(int cx, int cy, int radius, COLORREF fill, COLORREF line, int width = 1);
void draw_circle_piece(int cx, int cy, int radius, COLORREF fill, COLORREF line, int number, bool high);
void draw_dice(int x, int y, int size, int value, COLORREF fill = COLOR_WHITE);
void draw_mark(int cx, int cy, const std::wstring& text, COLORREF color);
