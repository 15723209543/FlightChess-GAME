#pragma once
#include "ui.h"
#include <vector>

enum piecestate {
    PIECE_BASE,     // 棋子在基地，尚未起飞。
    PIECE_TRACK,    // 棋子在公共跑道上。
    PIECE_FINISH,   // 棋子已进入自己的终点列。
    PIECE_HOME      // 棋子到达停车区。
};

struct piece {
    int id;              // 棋子编号，从0开始，显示时会加1。
    int owner;           // 棋子所属玩家编号。
    piecestate state;    // 棋子当前状态：基地、跑道、终点列、停车区。
    int trackpos;        // 棋子在公共跑道上的格子编号。
    int finishpos;       // 棋子在终点列中的格子编号。
    int skipturns;       // 棋子需要暂停的剩余回合数。
    bool skipfresh;      // 是否是本回合刚获得的暂停，防止刚设置就被立即扣掉。
    int trapcount;       // 棋子在当前圈内踩黑色陷阱次数。
    int extralaps;       // 棋子因陷阱需要额外多跑的圈数。

    piece();
    void reset();
    bool inbase() const;
    bool home() const;
};

struct player {
    int id;                     // 玩家编号，从0开始。
    int piececount;             // 玩家棋子总数。
    COLORREF color;             // 玩家颜色，用于棋子、基地和路径。
    std::wstring name;          // 玩家名称，如红队、蓝队。
    std::vector<piece> pieces;  // 玩家持有的所有棋子。
    int startpos;               // 玩家在公共跑道上的起点格。
    int endpos;                 // 玩家进入终点列前的入口格。
    int drawvalue;              // 开局抽签时掷出的点数。

    player();
    void init(int pid, int count, COLORREF pcolor);
    void resetpieces();
    int homecount() const;
    bool allhome() const;
    int arrivedcount() const;
    bool allarrived() const;
};
