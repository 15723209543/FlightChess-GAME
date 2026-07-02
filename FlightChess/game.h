#pragma once
#include "board.h"
#include <cstdio>

enum gamestate {
    STATE_SETUP,   // 设置人数和棋子数。
    STATE_LOT,     // 抽签决定出发顺序。
    STATE_ROLL,    // 等待当前玩家投骰子。
    STATE_SELECT,  // 等待当前玩家选择棋子或不走。
    STATE_OVER     // 游戏结束。
};

struct snapshot {
    int numplayers;                       // 快照中的玩家人数。
    int piececounts[MAX_PLAYERS];         // 快照中的每队棋子数量。
    gamestate state;                      // 快照中的游戏状态。
    std::vector<player> players;          // 快照中的玩家和棋子数据。
    std::vector<int> playorder;           // 快照中的行动顺序。
    std::vector<int> drawvalues;          // 快照中的抽签点数。
    int drawn;                            // 快照中已经抽签的人数。
    int turnindex;                        // 快照中当前行动顺序下标。
    int curplayer;                        // 快照中当前玩家编号。
    int dicevalue;                        // 快照中骰子点数。
    bool hasrolled;                       // 快照中是否已经投骰。
    bool gotsix;                          // 快照中是否投出6。
    int winner;                           // 快照中胜利玩家编号。
    std::wstring notice;                  // 快照中右侧输出信息。
    std::wstring special;                 // 快照中特殊操作说明。
    bool jumpactive;                      // 快照中是否显示飞跃虚线。
    int jumpfrom;                         // 快照中飞跃起点格。
    int jumpto;                           // 快照中飞跃终点格。
    int jumpowner;                        // 快照中飞跃所属玩家。
    board gameboard;                      // 快照中的棋盘对象。
};

class game {
public:
    int numplayers;                       // 当前玩家人数。
    int piececounts[MAX_PLAYERS];         // 每队棋子数量。
    gamestate state;                      // 当前游戏状态。
    std::vector<player> players;          // 所有玩家数据。
    std::vector<int> playorder;           // 正式游戏中的行动顺序。
    std::vector<int> drawvalues;          // 抽签点数。
    int drawn;                            // 已经完成抽签的人数。
    int turnindex;                        // 当前行动顺序下标。
    int curplayer;                        // 当前操作玩家编号。
    int dicevalue;                        // 当前骰子点数。
    bool hasrolled;                       // 当前回合是否已投骰。
    bool gotsix;                          // 当前投骰是否为6。
    int winner;                           // 获胜玩家编号；-1表示未分胜负。
    bool quit;                            // 是否退出主循环。
    std::wstring notice;                  // 右侧输出框文字。
    std::wstring special;                 // 右侧特殊操作文字。
    bool jumpactive;                      // 是否显示同色飞跃虚线。
    int jumpfrom;                         // 同色飞跃起点格。
    int jumpto;                           // 同色飞跃终点格。
    int jumpowner;                        // 同色飞跃所属玩家。
    board gameboard;                      // 棋盘对象。

    game();
    ~game();
    void init();
    void run();
    void handle_key(int key, int scancode);

private:
    std::vector<snapshot> history;        // 返回上一步用的历史快照栈。
    FILE* logfile;                        // 当前日志文件指针。
    std::wstring logpath;                 // 当前日志文件完整路径。

    void reset();
    void save();
    void undo();
    void draw();
    void handle_click(int mx, int my);
    void draw_setup();
    void draw_lot();
    void draw_play();
    void draw_over();
    void draw_panel_base(const std::wstring& title);
    void draw_panel_setup();
    void draw_panel_lot();
    void draw_panel_play();
    void draw_panel_over();
    void draw_back_button();
    void draw_dice_box(bool active);
    void draw_progress(int x, int y);
    void draw_rules(int x, int y);
    void draw_select_marks();
    void draw_jump_path();
    button back_button() const;
    button dice_button() const;
    button skip_button() const;
    void click_setup(int mx, int my);
    void click_lot(int mx, int my);
    void click_play(int mx, int my);
    void click_over(int mx, int my);
    void start_game();
    void lot_roll();
    void start_play();
    void roll_dice();
    bool any_move() const;
    bool can_move(int index) const;
    void move_piece(int index);
    void skip_move();
    void finish_action();
    void next_turn(const std::wstring& last = L"");
    void reduce_skips(int owner);
    std::wstring apply_special(piece& one, int owner);
    std::wstring check_hit(piece& one, int owner);
    bool check_win();
    void start_log();
    void close_log();
    void log_event(const std::wstring& text);
    std::wstring root_path() const;
    std::string gbk_text(const std::wstring& text) const;
};
