#include "board.h"
#include <cmath>

// 构造棋盘对象：默认先按4人棋局初始化，后续会按设置人数重新初始化。
board::board() : numplayers(4) {
    init(4);
}

// 初始化棋盘：根据玩家人数重建公共跑道、特殊格、终点列和基地位置。
void board::init(int count) {
    numplayers = count;          // numplayers：当前游戏人数，影响起点间距和颜色循环。
    build_track();               // build_track：生成正六边形公共跑道。
    build_special();             // build_special：生成黑色陷阱和起点格。
    build_lanes();               // build_lanes：生成每个玩家的终点列。
}

// 构建公共跑道：用60个格子沿正六边形六条边平均分布。
void board::build_track() {
    const double pi = 3.14159265358979323846; // pi：计算六边形顶点角度。
    const int centx = MAP_X + MAP_W / 2;      // centx：地图区中心X坐标。
    const int centy = MAP_Y + MAP_H / 2;      // centy：地图区中心Y坐标。
    const int radius = 350;                   // radius：公共跑道所在六边形半径。

    double vx[6];                             // vx：六边形6个顶点的X坐标。
    double vy[6];                             // vy：六边形6个顶点的Y坐标。
    for (int i = 0; i < 6; ++i) {             // i：顶点编号。
        double angle = -pi / 2.0 + i * pi / 3.0; // angle：当前顶点角度。
        vx[i] = centx + radius * cos(angle);
        vy[i] = centy + radius * sin(angle);
    }

    int idx = 0;                              // idx：当前写入的跑道格编号。
    for (int side = 0; side < 6; ++side) {    // side：当前六边形边编号。
        int next = (side + 1) % 6;            // next：当前边的下一个顶点编号。
        for (int step = 0; step < 10; ++step) { // step：当前边上的第几个格子。
            double t = (step + 0.5) / 10.0;   // t：在线段上的插值比例。
            cells[idx].cx = (int)(vx[side] * (1.0 - t) + vx[next] * t);
            cells[idx].cy = (int)(vy[side] * (1.0 - t) + vy[next] * t);
            cells[idx].type = CELL_NORMAL;
            cells[idx].owner = idx % numplayers;
            cells[idx].index = idx;
            ++idx;
        }
    }

    for (int i = 0; i < MAX_PLAYERS; ++i) {   // i：玩家编号。
        int start = i * TRACK_COUNT / numplayers; // start：该玩家起点格。
        if (i >= numplayers) {
            start = 0;
        }
        double dx = cells[start].cx - centx;  // dx：起点相对中心的X方向。
        double dy = cells[start].cy - centy;  // dy：起点相对中心的Y方向。
        double len = sqrt(dx * dx + dy * dy); // len：方向向量长度。
        if (len < 1.0) {
            len = 1.0;
        }

        basecenters[i].x = centx + (int)(dx / len * 395.0);
        basecenters[i].y = centy + (int)(dy / len * 395.0);

        double angle = -pi / 2.0 + i * 2.0 * pi / (numplayers > 0 ? numplayers : 1);
        homecenters[i].x = centx + (int)(cos(angle) * 48.0);
        homecenters[i].y = centy + (int)(sin(angle) * 48.0);
    }
}

// 构建特殊格：只保留黑色陷阱和每个玩家的起点。
void board::build_special() {
    trapmap.clear();                         // trapmap：黑色陷阱格，每条边一个。

    for (int i = 0; i < numplayers; ++i) {
        int start = i * TRACK_COUNT / numplayers;
        cells[start].type = CELL_START;
        cells[start].owner = i;
    }

    int offsets[] = {5, 4, 6, 3, 7};         // offsets：陷阱优先放在每条边中间，遇到起点就向旁边挪。
    for (int side = 0; side < 6; ++side) {
        int trap = -1;                       // trap：当前边最终选中的陷阱格。
        for (int i = 0; i < 5; ++i) {
            int idx = side * 10 + offsets[i];
            if (cells[idx].type != CELL_START) {
                trap = idx;
                break;
            }
        }
        if (trap >= 0) {
            trapmap[trap] = 1;
            cells[trap].type = CELL_TRAP;
            cells[trap].owner = -1;
        }
    }
}

// 构建终点列：从各玩家终点入口朝棋盘中心延伸。
void board::build_lanes() {
    const int centx = MAP_X + MAP_W / 2;      // centx：地图中心X。
    const int centy = MAP_Y + MAP_H / 2;      // centy：地图中心Y。
    for (int owner = 0; owner < MAX_PLAYERS; ++owner) {
        int start = owner * TRACK_COUNT / (numplayers > 0 ? numplayers : 1);
        if (owner >= numplayers) {
            start = 0;
        }
        int endpos = (start + TRACK_COUNT - 1) % TRACK_COUNT;
        for (int pos = 0; pos < FINISH_COUNT; ++pos) {
            double t = (pos + 1.0) / (FINISH_COUNT + 1.0);
            finishcells[owner][pos].cx = (int)(cells[endpos].cx * (1.0 - t) + centx * t);
            finishcells[owner][pos].cy = (int)(cells[endpos].cy * (1.0 - t) + centy * t);
            finishcells[owner][pos].type = CELL_NORMAL;
            finishcells[owner][pos].owner = owner;
            finishcells[owner][pos].index = pos;
        }
    }
}

// 查询公共跑道格中心坐标。
void board::track_center(int pos, int& cx, int& cy) const {
    int idx = (pos % TRACK_COUNT + TRACK_COUNT) % TRACK_COUNT;
    cx = cells[idx].cx;
    cy = cells[idx].cy;
}

// 查询终点列格中心坐标。
void board::finish_center(int owner, int pos, int& cx, int& cy) const {
    int idx = pos;
    if (idx < 0) {
        idx = 0;
    }
    if (idx >= FINISH_COUNT) {
        idx = FINISH_COUNT - 1;
    }
    cx = finishcells[owner][idx].cx;
    cy = finishcells[owner][idx].cy;
}

// 查询基地内棋子坐标：按2列或3列排布。
void board::base_piece_center(int owner, int id, int total, int& cx, int& cy) const {
    int cols = total <= 4 ? 2 : 3;            // cols：基地棋子列数。
    int rows = (total + cols - 1) / cols;     // rows：基地棋子行数。
    int col = id % cols;                      // col：棋子所在列。
    int row = id / cols;                      // row：棋子所在行。
    int space = 28;                           // space：棋子间距。
    int ox = (int)((col - (cols - 1) / 2.0) * space);
    int oy = (int)((row - (rows - 1) / 2.0) * space);
    cx = basecenters[owner].x + ox;
    cy = basecenters[owner].y + oy + 8;
}

// 查询停车区内棋子坐标。
void board::home_piece_center(int owner, int id, int total, int& cx, int& cy) const {
    int cols = total <= 4 ? 2 : 3;
    int row = id / cols;
    int col = id % cols;
    int space = 18;
    cx = homecenters[owner].x + (int)((col - (cols - 1) / 2.0) * space);
    cy = homecenters[owner].y + (int)((row - 0.5) * space);
}

// 绘制完整棋盘：背景、外框、终点列、跑道、基地。
void board::draw_board() const {
    setfillcolor(RGB(226, 232, 241));
    setlinecolor(RGB(188, 197, 211));
    fillrectangle(MAP_X, MAP_Y, MAP_X + MAP_W, MAP_Y + MAP_H);
    rectangle(MAP_X, MAP_Y, MAP_X + MAP_W, MAP_Y + MAP_H);

    draw_hex(MAP_X + MAP_W / 2, MAP_Y + MAP_H / 2, 405, RGB(247, 249, 252), RGB(72, 84, 104), 3);
    draw_home();

    for (int owner = 0; owner < numplayers; ++owner) {
        draw_finish_lane(owner);
    }
    for (int i = 0; i < TRACK_COUNT; ++i) {
        draw_one_cell(i);
    }
    for (int owner = 0; owner < numplayers; ++owner) {
        draw_base(owner);
    }
}

// 绘制单个跑道格：按类型决定颜色和标记。
void board::draw_one_cell(int index) const {
    const cell& one = cells[index];
    COLORREF fill = COLOR_WHITE;
    COLORREF line = RGB(98, 106, 124);

    if (one.type == CELL_TRAP) {
        fill = RGB(235, 238, 244);
    } else if (one.owner >= 0 && one.owner < numplayers) {
        int light = (index % 2 == 0) ? 68 : 78;
        fill = mix_color(PLAYER_COLORS[one.owner], COLOR_WHITE, light);
    }

    if (one.type == CELL_START && one.owner >= 0) {
        fill = mix_color(PLAYER_COLORS[one.owner], COLOR_WHITE, 25);
        line = PLAYER_COLORS[one.owner];
    }

    draw_hex(one.cx, one.cy, 18, fill, line, one.type == CELL_START ? 2 : 1);

    if (one.type == CELL_TRAP) {
        setfillcolor(RGB(20, 24, 32));
        setlinecolor(RGB(20, 24, 32));
        fillcircle(one.cx, one.cy, 11);
        draw_text_center(one.cx - 10, one.cy - 9, 20, 18, L"陷", COLOR_WHITE, 11);
    } else if (one.type == CELL_START) {
        draw_text_center(one.cx - 12, one.cy - 10, 24, 20, L"起", COLOR_WHITE, 13);
    }
}

// 绘制某个玩家的终点列。
void board::draw_finish_lane(int owner) const {
    for (int pos = 0; pos < FINISH_COUNT; ++pos) {
        COLORREF fill = mix_color(PLAYER_COLORS[owner], COLOR_WHITE, 55 - pos * 5);
        draw_hex(finishcells[owner][pos].cx, finishcells[owner][pos].cy, 15, fill, PLAYER_COLORS[owner], 1);
        if (pos == FINISH_COUNT - 1) {
            draw_text_center(finishcells[owner][pos].cx - 14, finishcells[owner][pos].cy - 10, 28, 20, L"停", COLOR_WHITE, 13);
        }
    }
}

// 绘制玩家基地。
void board::draw_base(int owner) const {
    COLORREF color = PLAYER_COLORS[owner];
    COLORREF fill = mix_color(color, COLOR_WHITE, 75);
    draw_hex(basecenters[owner].x, basecenters[owner].y, 50, fill, color, 2);
    draw_text_center(basecenters[owner].x - 42, basecenters[owner].y - 58, 84, 22, player_name(owner), color, 17);
}

// 绘制中心停车区。
void board::draw_home() const {
    int cx = MAP_X + MAP_W / 2;
    int cy = MAP_Y + MAP_H / 2;
    draw_hex(cx, cy, 66, RGB(255, 248, 226), RGB(122, 100, 57), 2);
    draw_text_center(cx - 45, cy - 15, 90, 30, L"停车区", RGB(96, 76, 38), 18);
}

// 绘制所有棋子：同格棋子会错位显示，避免互相遮挡。
void board::draw_pieces(const std::vector<player>& players, int curplayer, bool selectmode) const {
    int offx[] = {0, -11, 11, 0, -11, 11, -18, 18, -18, 18, 0, 0};
    int offy[] = {0, -9, -9, 12, 9, 9, 0, 0, -16, -16, -18, 18};
    for (int owner = 0; owner < (int)players.size(); ++owner) {
        for (int i = 0; i < (int)players[owner].pieces.size(); ++i) {
            const piece& one = players[owner].pieces[i];
            int cx = 0;
            int cy = 0;
            if (one.state == PIECE_BASE) {
                base_piece_center(owner, one.id, (int)players[owner].pieces.size(), cx, cy);
            } else if (one.state == PIECE_TRACK) {
                track_center(one.trackpos, cx, cy);
                int stackidx = 0;
                for (int p = 0; p < owner; ++p) {
                    for (int k = 0; k < (int)players[p].pieces.size(); ++k) {
                        if (players[p].pieces[k].state == PIECE_TRACK &&
                            players[p].pieces[k].trackpos == one.trackpos) {
                            ++stackidx;
                        }
                    }
                }
                for (int k = 0; k < i; ++k) {
                    if (players[owner].pieces[k].state == PIECE_TRACK &&
                        players[owner].pieces[k].trackpos == one.trackpos) {
                        ++stackidx;
                    }
                }
                cx += offx[stackidx % 12];
                cy += offy[stackidx % 12];
            } else if (one.state == PIECE_FINISH) {
                finish_center(owner, one.finishpos, cx, cy);
            } else {
                home_piece_center(owner, one.id, (int)players[owner].pieces.size(), cx, cy);
            }
            draw_circle_piece(cx, cy, 13, players[owner].color, RGB(40, 44, 54), one.id + 1, false);
        }
    }
}

// 鼠标命中检测：判断当前玩家点击了哪个棋子。
int board::hit_piece(const std::vector<player>& players, int owner, int mx, int my) const {
    if (owner < 0 || owner >= (int)players.size()) {
        return -1;
    }
    for (int i = 0; i < (int)players[owner].pieces.size(); ++i) {
        const piece& one = players[owner].pieces[i];
        if (one.state == PIECE_HOME) {
            continue;
        }
        int cx = 0;
        int cy = 0;
        if (one.state == PIECE_BASE) {
            base_piece_center(owner, one.id, (int)players[owner].pieces.size(), cx, cy);
        } else if (one.state == PIECE_TRACK) {
            track_center(one.trackpos, cx, cy);
        } else if (one.state == PIECE_FINISH) {
            finish_center(owner, one.finishpos, cx, cy);
        }
        int dx = mx - cx;
        int dy = my - cy;
        if (dx * dx + dy * dy <= 22 * 22) {
            return i;
        }
    }
    return -1;
}

// 查找下一个同色普通格。
int board::next_color_cell(int pos, int owner) const {
    for (int step = 1; step <= TRACK_COUNT; ++step) {
        int idx = (pos + step) % TRACK_COUNT;
        if (cells[idx].type == CELL_NORMAL && cells[idx].owner == owner) {
            return idx;
        }
    }
    return -1;
}

// 计算棋子在公共跑道上的移动结果。
moveresult board::move(int curpos, int steps, int startpos) const {
    moveresult res;
    res.enterfinish = false;
    res.valid = true;
    res.finishpos = -1;
    res.newpos = curpos;

    int endpos = (startpos + TRACK_COUNT - 1) % TRACK_COUNT;
    if (curpos == endpos) {
        res.enterfinish = true;
        res.finishpos = steps - 1;
        res.valid = res.finishpos < FINISH_COUNT;
        return res;
    }

    int dist = (endpos - curpos + TRACK_COUNT) % TRACK_COUNT;
    if (steps > dist) {
        res.enterfinish = true;
        res.finishpos = steps - dist - 1;
        res.valid = res.finishpos < FINISH_COUNT;
        return res;
    }

    res.newpos = (curpos + steps) % TRACK_COUNT;
    return res;
}
