#include "ui.h"
#include <cmath>

// PLAYER_COLORS：六支队伍的主色，用来绘制棋子、基地、进度条和文字。
const COLORREF PLAYER_COLORS[MAX_PLAYERS] = {
    RGB(220, 62, 62),
    RGB(55, 114, 210),
    RGB(54, 168, 88),
    RGB(228, 178, 45),
    RGB(151, 83, 204),
    RGB(239, 128, 56)
};

// playernames：玩家队伍名称，按玩家编号从0到5对应。
static const wchar_t* playernames[MAX_PLAYERS] = {
    L"红队", L"蓝队", L"绿队", L"黄队", L"紫队", L"橙队"
};

// player_name：根据玩家编号返回队伍名称，越界时返回兜底名称。
const wchar_t* player_name(int index) {
    if (index >= 0 && index < MAX_PLAYERS) {
        return playernames[index];
    }
    return L"未知队";
}

// player_title：生成“某队的某号玩家”这种右侧提示文本。
std::wstring player_title(int index) {
    return std::wstring(player_name(index)) + L"的" + std::to_wstring(index + 1) + L"号玩家";
}

// mix_color：按百分比混合两个颜色，主要用于生成浅色基地背景。
COLORREF mix_color(COLORREF a, COLORREF b, int percent) {
    int ar = GetRValue(a); // ar/ag/ab：第一个颜色的红绿蓝分量。
    int ag = GetGValue(a);
    int ab = GetBValue(a);
    int br = GetRValue(b); // br/bg/bb：第二个颜色的红绿蓝分量。
    int bg = GetGValue(b);
    int bb = GetBValue(b);
    int r = (ar * (100 - percent) + br * percent) / 100; // r/g/bl：混合后的红绿蓝分量。
    int g = (ag * (100 - percent) + bg * percent) / 100;
    int bl = (ab * (100 - percent) + bb * percent) / 100;
    return RGB(r, g, bl);
}

// button 默认构造函数：提供一个可用的默认按钮。
button::button()
    : x(0), y(0), w(100), h(36), text(L""), bgcolor(RGB(72, 118, 210)),
      textcolor(COLOR_WHITE), bordercolor(RGB(48, 58, 78)), visible(true), enabled(true) {
}

// button 带参构造函数：按给定位置、尺寸和文字创建按钮。
button::button(int x, int y, int w, int h, const std::wstring& text)
    : x(x), y(y), w(w), h(h), text(text), bgcolor(RGB(72, 118, 210)),
      textcolor(COLOR_WHITE), bordercolor(RGB(48, 58, 78)), visible(true), enabled(true) {
}

// draw：绘制按钮，禁用状态会自动变灰。
void button::draw() const {
    if (!visible) {
        return;
    }
    COLORREF fill = enabled ? bgcolor : RGB(190, 194, 202);
    COLORREF textcol = enabled ? textcolor : RGB(110, 116, 128);
    setfillcolor(fill);
    setlinecolor(bordercolor);
    setlinestyle(PS_SOLID, 1);
    fillrectangle(x, y, x + w, y + h);
    rectangle(x, y, x + w, y + h);
    draw_text_center(x, y, w, h, text.empty() ? L"" : text, textcol, h > 44 ? 20 : 16);
}

// inside：判断鼠标坐标是否落在可见且可用的按钮范围内。
bool button::inside(int mx, int my) const {
    return visible && enabled && mx >= x && mx <= x + w && my >= y && my <= y + h;
}

// clear_screen：用指定颜色清空整个游戏窗口。
void clear_screen(COLORREF color) {
    setfillcolor(color);
    setlinecolor(color);
    solidrectangle(0, 0, WINDOW_W, WINDOW_H);
}

// draw_text_center：在指定矩形中居中绘制一行文字。
void draw_text_center(int x, int y, int w, int h, const std::wstring& text, COLORREF color, int size) {
    settextstyle(size, 0, L"微软雅黑");
    settextcolor(color);
    setbkmode(TRANSPARENT);
    int tw = textwidth(text.c_str()); // tw：文字宽度，用来计算水平居中。
    int th = textheight(text.c_str()); // th：文字高度，用来计算垂直居中。
    outtextxy(x + (w - tw) / 2, y + (h - th) / 2, text.c_str());
}

// draw_text_left：从左上角坐标开始绘制一行文字。
void draw_text_left(int x, int y, const std::wstring& text, COLORREF color, int size) {
    settextstyle(size, 0, L"微软雅黑");
    settextcolor(color);
    setbkmode(TRANSPARENT);
    outtextxy(x, y, text.c_str());
}

// draw_text_wrap：在固定宽度内自动换行绘制文字，并返回绘制结束后的Y坐标。
int draw_text_wrap(int x, int y, int w, const std::wstring& text, COLORREF color, int size, int lineheight, int maxlines) {
    settextstyle(size, 0, L"微软雅黑");
    settextcolor(color);
    setbkmode(TRANSPARENT);
    std::wstring line; // line：当前正在累计的一行文字。
    int lines = 0; // lines：已经绘制的行数，防止超出最大行数。
    for (size_t i = 0; i < text.size(); ++i) {
        wchar_t ch = text[i]; // ch：当前处理的字符。
        if (ch == L'\r') {
            continue;
        }
        if (ch == L'\n') {
            outtextxy(x, y + lines * lineheight, line.c_str());
            line.clear();
            ++lines;
            if (lines >= maxlines) {
                return y + lines * lineheight;
            }
            continue;
        }
        std::wstring test = line + ch; // test：尝试加入当前字符后的测试行。
        if (!line.empty() && textwidth(test.c_str()) > w) {
            outtextxy(x, y + lines * lineheight, line.c_str());
            line.clear();
            ++lines;
            if (lines >= maxlines) {
                return y + lines * lineheight;
            }
        }
        line += ch;
    }
    if (!line.empty() && lines < maxlines) {
        outtextxy(x, y + lines * lineheight, line.c_str());
        ++lines;
    }
    return y + lines * lineheight;
}

// draw_hex：绘制一个正六边形，用于棋盘格、基地和终点格。
void draw_hex(int cx, int cy, int radius, COLORREF fill, COLORREF line, int width) {
    POINT pts[6]; // pts：六边形的六个顶点坐标。
    const double pi = 3.14159265358979323846; // pi：角度换算用圆周率。
    for (int i = 0; i < 6; ++i) {
        double angle = -pi / 2.0 + i * pi / 3.0; // angle：当前顶点相对圆心的角度。
        pts[i].x = cx + (int)(radius * cos(angle));
        pts[i].y = cy + (int)(radius * sin(angle));
    }
    setfillcolor(fill);
    setlinecolor(line);
    setlinestyle(PS_SOLID, width);
    fillpolygon(pts, 6);
    polygon(pts, 6);
    setlinestyle(PS_SOLID, 1);
}

// draw_circle_piece：绘制圆形棋子，高亮时额外画金色外圈。
void draw_circle_piece(int cx, int cy, int radius, COLORREF fill, COLORREF line, int number, bool high) {
    setfillcolor(RGB(72, 78, 92));
    setlinecolor(RGB(72, 78, 92));
    solidcircle(cx + 2, cy + 3, radius);
    setfillcolor(fill);
    setlinecolor(high ? COLOR_GOLD : line);
    setlinestyle(PS_SOLID, high ? 3 : 2);
    fillcircle(cx, cy, radius);
    circle(cx, cy, radius);
    setlinestyle(PS_SOLID, 1);
    if (high) {
        setlinecolor(COLOR_GOLD);
        circle(cx, cy, radius + 5);
    }

    wchar_t buf[8]; // buf：棋子编号的临时文本缓冲区。
    swprintf_s(buf, L"%d", number);
    draw_text_center(cx - radius, cy - radius, radius * 2, radius * 2, buf, COLOR_WHITE, 14);
}

// draw_dice：按点数绘制骰子图案。
void draw_dice(int x, int y, int size, int value, COLORREF fill) {
    setfillcolor(fill);
    setlinecolor(RGB(52, 58, 70));
    setlinestyle(PS_SOLID, 2);
    fillrectangle(x, y, x + size, y + size);
    rectangle(x, y, x + size, y + size);
    setlinestyle(PS_SOLID, 1);

    int dot = size / 9; // dot：骰子点的半径。
    int cx = x + size / 2; // cx/cy：骰子中心坐标。
    int cy = y + size / 2;
    int off = size / 4; // off：角落点距离中心的偏移量。
    setfillcolor(RGB(32, 36, 46));
    setlinecolor(RGB(32, 36, 46));

    auto pip = [&](int dx, int dy) {
        solidcircle(cx + dx, cy + dy, dot);
    };

    switch (value) {
    case 1:
        pip(0, 0);
        break;
    case 2:
        pip(-off, -off);
        pip(off, off);
        break;
    case 3:
        pip(-off, -off);
        pip(0, 0);
        pip(off, off);
        break;
    case 4:
        pip(-off, -off);
        pip(off, -off);
        pip(-off, off);
        pip(off, off);
        break;
    case 5:
        pip(-off, -off);
        pip(off, -off);
        pip(0, 0);
        pip(-off, off);
        pip(off, off);
        break;
    case 6:
        pip(-off, -off);
        pip(off, -off);
        pip(-off, 0);
        pip(off, 0);
        pip(-off, off);
        pip(off, off);
        break;
    default:
        break;
    }
}

// draw_mark：绘制奖励、惩罚等小标记。
void draw_mark(int cx, int cy, const std::wstring& text, COLORREF color) {
    setlinecolor(color);
    setfillcolor(COLOR_WHITE);
    fillcircle(cx, cy, 10);
    circle(cx, cy, 10);
    draw_text_center(cx - 10, cy - 10, 20, 20, text, color, 13);
}
