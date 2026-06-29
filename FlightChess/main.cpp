#include <graphics.h>
#include <windows.h>
#include "game.h"

// main：程序入口，负责创建 EasyX 窗口、启动游戏主循环并在退出时关闭图形窗口。
int main() {
    // initgraph：按照 UI 常量打开尽可能大的游戏窗口，并保留控制台方便调试。
    initgraph(WINDOW_W, WINDOW_H, SHOWCONSOLE);
    setbkmode(TRANSPARENT);
    SetWindowText(GetHWnd(), L"飞行棋游戏");

    game app; // app：整局飞行棋游戏对象，包含棋盘、玩家、输入和日志。
    app.init();
    app.run();

    // closegraph：释放 EasyX 图形资源。
    closegraph();
    return 0;
}
