#pragma once
#include "player.h"
#include <map>
#include <vector>

const int TRACK_COUNT = 60;
const int FINISH_COUNT = 6;

enum celltype {
    CELL_NORMAL,   // 普通颜色格，可触发同色飞跃。
    CELL_START,    // 起点格，别人落到这里会停一回合。
    CELL_REWARD,   // 奖励格，前进若干格。
    CELL_PUNISH,   // 惩罚格，后退若干格。
    CELL_TRAP      // 黑色陷阱格，同圈第二次踩中要多跑一圈。
};

struct point2 {
    int x; // X坐标。
    int y; // Y坐标。
};

struct cell {
    int cx;         // 格子中心X坐标。
    int cy;         // 格子中心Y坐标。
    celltype type;  // 格子类型。
    int owner;      // 格子所属玩家；-1表示无所属。
    int index;      // 公共跑道格号。
};

struct moveresult {
    bool enterfinish; // 本次移动是否进入终点列。
    bool valid;       // 本次移动是否有效。
    int finishpos;    // 进入终点列后的格号。
    int newpos;       // 仍在公共跑道时的新格号。
};

class board {
public:
    int numplayers;                                      // 当前玩家人数。
    cell cells[TRACK_COUNT];                             // 公共跑道所有格子。
    cell finishcells[MAX_PLAYERS][FINISH_COUNT];         // 每个玩家的终点列格子。
    point2 basecenters[MAX_PLAYERS];                     // 每个玩家基地中心点。
    point2 homecenters[MAX_PLAYERS];                     // 每个玩家停车区棋子中心点。
    std::map<int, int> rewardmap;                        // 奖励格：格号 -> 前进步数。
    std::map<int, int> punishmap;                        // 惩罚格：格号 -> 后退步数。
    std::map<int, int> trapmap;                          // 黑色陷阱格：格号 -> 标记值。

    board();
    void init(int count);
    void draw_board() const;
    void draw_pieces(const std::vector<player>& players, int curplayer, bool selectmode) const;
    void track_center(int pos, int& cx, int& cy) const;
    void finish_center(int owner, int pos, int& cx, int& cy) const;
    void base_piece_center(int owner, int id, int total, int& cx, int& cy) const;
    void home_piece_center(int owner, int id, int total, int& cx, int& cy) const;
    int hit_piece(const std::vector<player>& players, int owner, int mx, int my) const;
    int next_color_cell(int pos, int owner) const;
    moveresult move(int curpos, int steps, int startpos) const;

private:
    void build_track();
    void build_special();
    void build_lanes();
    void draw_one_cell(int index) const;
    void draw_finish_lane(int owner) const;
    void draw_base(int owner) const;
    void draw_home() const;
};
