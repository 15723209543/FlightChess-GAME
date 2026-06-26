#include "player.h"

// piece 构造函数：初始化单个棋子的默认状态和计数。
piece::piece()
    : id(0), owner(0), state(PIECE_BASE), trackpos(0), finishpos(-1),
      skipturns(0), trapcount(0), extralaps(0) {
}

// reset：把棋子放回基地，并清空暂停、陷阱和额外圈数记录。
void piece::reset() {
    state = PIECE_BASE;
    trackpos = 0;
    finishpos = -1;
    skipturns = 0;
    trapcount = 0;
    extralaps = 0;
}

// inbase：判断棋子是否仍停在玩家基地里。
bool piece::inbase() const {
    return state == PIECE_BASE;
}

// home：判断棋子是否已经进入最终停车区。
bool piece::home() const {
    return state == PIECE_HOME;
}

// player 构造函数：初始化玩家编号、颜色、起终点和抽签点数。
player::player()
    : id(0), piececount(0), color(COLOR_WHITE), name(L""), startpos(0), endpos(0), drawvalue(0) {
}

// init：按照玩家编号和棋子数量创建本队所有棋子。
void player::init(int pid, int count, COLORREF pcolor) {
    id = pid;
    piececount = count;
    color = pcolor;
    name = player_name(pid);
    drawvalue = 0;
    pieces.clear();
    for (int i = 0; i < piececount; ++i) {
        piece one; // one：当前正在创建的一枚棋子。
        one.id = i;
        one.owner = pid;
        one.reset();
        pieces.push_back(one);
    }
}

// resetpieces：重开或撤回时统一重置本玩家的全部棋子。
void player::resetpieces() {
    for (size_t i = 0; i < pieces.size(); ++i) {
        pieces[i].reset();
    }
}

// homecount：统计已经进入停车区的棋子数量。
int player::homecount() const {
    int count = 0; // count：已到达停车区的棋子计数。
    for (size_t i = 0; i < pieces.size(); ++i) {
        if (pieces[i].home()) {
            ++count;
        }
    }
    return count;
}

// allhome：判断玩家全部棋子是否都进入停车区。
bool player::allhome() const {
    if (pieces.empty()) {
        return false;
    }
    for (size_t i = 0; i < pieces.size(); ++i) {
        if (!pieces[i].home()) {
            return false;
        }
    }
    return true;
}

// arrivedcount：统计进入终点列或停车区的棋子数量，用作胜利进度。
int player::arrivedcount() const {
    int count = 0; // count：已经完成任务的棋子计数。
    for (size_t i = 0; i < pieces.size(); ++i) {
        if (pieces[i].state == PIECE_FINISH || pieces[i].state == PIECE_HOME) {
            ++count;
        }
    }
    return count;
}

// allarrived：胜利判断，只要全部棋子进入终点列或停车区即完成。
bool player::allarrived() const {
    if (pieces.empty()) {
        return false;
    }
    for (size_t i = 0; i < pieces.size(); ++i) {
        if (pieces[i].state != PIECE_FINISH && pieces[i].state != PIECE_HOME) {
            return false;
        }
    }
    return true;
}
