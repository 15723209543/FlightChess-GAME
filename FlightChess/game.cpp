#include "game.h"
#include "keyboard.h"
#include <algorithm>
#include <ctime>
#include <windows.h>

// 构造游戏对象：初始化所有游戏状态变量和默认设置。
game::game()
    : numplayers(4), state(STATE_SETUP), drawn(0), turnindex(0), curplayer(0),
      dicevalue(1), hasrolled(false), gotsix(false), winner(-1), quit(false),
      notice(L"请选择游戏人数和每队棋子数量。"), special(L"无"),
      jumpactive(false), jumpfrom(-1), jumpto(-1), jumpowner(-1), logfile(nullptr) {
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        piececounts[i] = 4; // piececounts：每队棋子数量，默认4个。
    }
}

// 析构游戏对象：确保日志文件被关闭。
game::~game() {
    close_log();
}

// 初始化游戏：设置随机种子并创建默认棋盘。
void game::init() {
    srand((unsigned)time(nullptr));
    gameboard.init(numplayers);
}

// 游戏主循环：不断绘制当前界面，并等待鼠标或键盘操作。
void game::run() {
    while (!quit) {
        draw();
        ExMessage msg; // msg：EasyX输入消息。
        getmessage(&msg, EM_MOUSE | EM_KEY);
        if (msg.message == WM_LBUTTONDOWN) {
            handle_click(msg.x, msg.y);
        } else if (msg.message == WM_KEYDOWN) {
            handle_keyboard(*this, msg.vkcode, msg.scancode);
        }
    }
}

// 重置游戏：回到设置界面并清空运行数据。
void game::reset() {
    close_log();
    numplayers = 4;
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        piececounts[i] = 4;
    }
    state = STATE_SETUP;
    players.clear();
    playorder.clear();
    drawvalues.clear();
    history.clear();
    drawn = 0;
    turnindex = 0;
    curplayer = 0;
    dicevalue = 1;
    hasrolled = false;
    gotsix = false;
    winner = -1;
    notice = L"请选择游戏人数和每队棋子数量。";
    special = L"无";
    jumpactive = false;
    jumpfrom = -1;
    jumpto = -1;
    jumpowner = -1;
    gameboard.init(numplayers);
}

// 保存快照：用于“返回上一步”功能。
void game::save() {
    snapshot snap; // snap：保存点击前的完整游戏状态。
    snap.numplayers = numplayers;
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        snap.piececounts[i] = piececounts[i];
    }
    snap.state = state;
    snap.players = players;
    snap.playorder = playorder;
    snap.drawvalues = drawvalues;
    snap.drawn = drawn;
    snap.turnindex = turnindex;
    snap.curplayer = curplayer;
    snap.dicevalue = dicevalue;
    snap.hasrolled = hasrolled;
    snap.gotsix = gotsix;
    snap.winner = winner;
    snap.notice = notice;
    snap.special = special;
    snap.jumpactive = jumpactive;
    snap.jumpfrom = jumpfrom;
    snap.jumpto = jumpto;
    snap.jumpowner = jumpowner;
    snap.gameboard = gameboard;
    history.push_back(snap);
    if (history.size() > 80) {
        history.erase(history.begin());
    }
}

// 返回上一步：从快照栈恢复游戏状态。
void game::undo() {
    if (history.empty()) {
        notice = L"当前没有可以返回的上一步。";
        return;
    }
    snapshot snap = history.back();
    history.pop_back();
    numplayers = snap.numplayers;
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        piececounts[i] = snap.piececounts[i];
    }
    state = snap.state;
    players = snap.players;
    playorder = snap.playorder;
    drawvalues = snap.drawvalues;
    drawn = snap.drawn;
    turnindex = snap.turnindex;
    curplayer = snap.curplayer;
    dicevalue = snap.dicevalue;
    hasrolled = snap.hasrolled;
    gotsix = snap.gotsix;
    winner = snap.winner;
    notice = L"已返回上一步。";
    special = snap.special;
    jumpactive = snap.jumpactive;
    jumpfrom = snap.jumpfrom;
    jumpto = snap.jumpto;
    jumpowner = snap.jumpowner;
    gameboard = snap.gameboard;
    log_event(L"返回上一步");
}

// 创建返回按钮：位置在骰子左侧，按钮比原来更小。
button game::back_button() const {
    button one(PANEL_X + 28, PANEL_Y + 790, 150, 40, L"返回上一步");
    one.bgcolor = RGB(92, 104, 124);
    one.enabled = !history.empty();
    return one;
}

// 创建骰子点击区域：缩小蓝色区域，避免遮挡规则说明。
button game::dice_button() const {
    return button(PANEL_X + 330, PANEL_Y + 742, 120, 145, L"");
}

// 创建“不走”按钮：和返回按钮在同一行。
button game::skip_button() const {
    button one(PANEL_X + 190, PANEL_Y + 790, 120, 40, L"不走");
    one.bgcolor = RGB(206, 126, 54);
    return one;
}

// 按当前状态绘制界面。
void game::draw() {
    BeginBatchDraw();
    switch (state) {
    case STATE_SETUP:
        draw_setup();
        break;
    case STATE_LOT:
        draw_lot();
        break;
    case STATE_ROLL:
    case STATE_SELECT:
        draw_play();
        break;
    case STATE_OVER:
        draw_over();
        break;
    }
    EndBatchDraw();
}

// 绘制设置界面。
void game::draw_setup() {
    clear_screen(COLOR_BG);
    gameboard.draw_board();
    draw_panel_base(L"游戏设置");
    draw_panel_setup();
}

// 绘制抽签界面。
void game::draw_lot() {
    clear_screen(COLOR_BG);
    gameboard.draw_board();
    gameboard.draw_pieces(players, -1, false);
    draw_panel_base(L"抽签顺序");
    draw_panel_lot();
}

// 绘制游戏进行界面。
void game::draw_play() {
    clear_screen(COLOR_BG);
    gameboard.draw_board();
    draw_jump_path();
    gameboard.draw_pieces(players, curplayer, state == STATE_SELECT);
    if (state == STATE_SELECT) {
        draw_select_marks();
    }
    draw_panel_base(L"游戏信息");
    draw_panel_play();
}

// 绘制游戏结束界面。
void game::draw_over() {
    clear_screen(COLOR_BG);
    gameboard.draw_board();
    gameboard.draw_pieces(players, -1, false);
    draw_panel_base(L"游戏结束");
    draw_panel_over();
}

// 绘制右侧面板基础外框和标题。
void game::draw_panel_base(const std::wstring& title) {
    setfillcolor(COLOR_PANEL);
    setlinecolor(RGB(182, 190, 204));
    fillrectangle(PANEL_X, PANEL_Y, PANEL_X + PANEL_W, PANEL_Y + PANEL_H);
    rectangle(PANEL_X, PANEL_Y, PANEL_X + PANEL_W, PANEL_Y + PANEL_H);
    draw_text_left(PANEL_X + 28, PANEL_Y + 18, title, RGB(38, 67, 122), 26);
    setlinecolor(RGB(214, 220, 230));
    line(PANEL_X + 24, PANEL_Y + 720, PANEL_X + PANEL_W - 24, PANEL_Y + 720);
}

// 绘制设置面板：人数、棋子数量和开始按钮。
void game::draw_panel_setup() {
    int x = PANEL_X + 30; // x：右侧内容左边距。
    int y = PANEL_Y + 76; // y：当前绘制行Y坐标。
    draw_text_left(x, y, L"选择玩家人数", COLOR_TEXT, 18);
    y += 36;
    for (int i = 0; i < 5; ++i) {
        int val = i + 2;
        button one(x + i * 68, y, 54, 38, std::to_wstring(val));
        one.bgcolor = val == numplayers ? COLOR_GOOD : RGB(76, 125, 209);
        one.draw();
    }

    y += 66;
    draw_text_left(x, y, L"每队棋子数量", COLOR_TEXT, 18);
    y += 34;
    for (int i = 0; i < numplayers; ++i) {
        int rowy = y + i * 44;
        setfillcolor(PLAYER_COLORS[i]);
        setlinecolor(PLAYER_COLORS[i]);
        solidrectangle(x, rowy + 8, x + 24, rowy + 32);
        draw_text_left(x + 34, rowy + 8, player_name(i), PLAYER_COLORS[i], 16);
        button minus(x + 190, rowy, 36, 32, L"-");
        minus.bgcolor = COLOR_DANGER;
        minus.enabled = piececounts[i] > 2;
        minus.draw();
        draw_text_center(x + 232, rowy, 44, 32, std::to_wstring(piececounts[i]), COLOR_TEXT, 17);
        button plus(x + 282, rowy, 36, 32, L"+");
        plus.bgcolor = COLOR_GOOD;
        plus.enabled = piececounts[i] < 6;
        plus.draw();
    }

    button start(x, PANEL_Y + 700, PANEL_W - 60, 48, L"开始抽签");
    start.bgcolor = RGB(206, 66, 66);
    start.draw();
    draw_back_button();
    draw_text_wrap(x, PANEL_Y + 760, PANEL_W - 60, notice, COLOR_MUTED, 14, 18, 2);
}

// 绘制抽签面板：抽签点数和出发顺序分区显示。
void game::draw_panel_lot() {
    int x = PANEL_X + 30;
    int y = PANEL_Y + 76;
    if (drawn < numplayers) {
        draw_text_left(x, y, L"当前操作是：" + player_title(drawn), COLOR_TEXT, 17);
        draw_text_left(x, y + 34, L"点击下方骰子抽签，点数大的先出发。", COLOR_MUTED, 14);
    } else {
        draw_text_left(x, y, L"抽签完成。", COLOR_TEXT, 17);
    }

    y += 78;
    draw_text_left(x, y, L"抽签点数", COLOR_TEXT, 16);
    y += 26;
    for (int i = 0; i < numplayers; ++i) {
        int rowy = y + i * 30;
        setfillcolor(PLAYER_COLORS[i]);
        setlinecolor(PLAYER_COLORS[i]);
        solidrectangle(x, rowy + 5, x + 20, rowy + 25);
        std::wstring text = player_title(i);
        text += drawvalues[i] > 0 ? L"  点数：" + std::to_wstring(drawvalues[i]) : L"  等待抽签";
        draw_text_left(x + 30, rowy + 5, text, COLOR_TEXT, 15);
    }

    if (drawn >= numplayers) {
        std::vector<int> order(numplayers);
        for (int i = 0; i < numplayers; ++i) {
            order[i] = i;
        }
        std::sort(order.begin(), order.end(), [&](int a, int b) {
            return drawvalues[a] == drawvalues[b] ? a < b : drawvalues[a] > drawvalues[b];
        });

        int oy = PANEL_Y + 395;
        draw_text_left(x, oy - 28, L"出发顺序", COLOR_TEXT, 16);
        for (int i = 0; i < numplayers; ++i) {
            std::wstring text = std::to_wstring(i + 1) + L". " + player_title(order[i]);
            draw_text_left(x, oy + i * 28, text, PLAYER_COLORS[order[i]], 15);
        }
        button start(x, PANEL_Y + 700, PANEL_W - 60, 48, L"进入游戏");
        start.bgcolor = COLOR_GOOD;
        start.draw();
    }

    draw_back_button();
    if (drawn < numplayers) {
        draw_dice_box(true);
    }
    draw_text_wrap(x, PANEL_Y + 760, PANEL_W - 60, notice, COLOR_MUTED, 14, 18, 2);
}

// 绘制游戏面板：操作说明、输出框、进度、规则、按钮和骰子。
void game::draw_panel_play() {
    int x = PANEL_X + 30;
    int y = PANEL_Y + 76;
    if (!players.empty()) {
        draw_text_left(x, y, L"当前操作是：" + player_title(curplayer), COLOR_TEXT, 18);
    }
    y += 38;
    if (hasrolled) {
        draw_text_left(x, y, L"当前玩家投掷出的点数是：" + std::to_wstring(dicevalue), COLOR_TEXT, 16);
    } else {
        draw_text_left(x, y, L"请点击右下骰子投掷。", COLOR_TEXT, 16);
    }
    y += 34;
    draw_text_left(x, y, L"特殊操作：" + special, special == L"无" ? COLOR_MUTED : RGB(169, 91, 25), 15);

    int outy = PANEL_Y + 182; // outy：输出框上边界，上移以减少右侧空白。
    setfillcolor(RGB(246, 248, 252));
    setlinecolor(RGB(80, 160, 220));
    fillrectangle(x - 4, outy, PANEL_X + PANEL_W - 30, outy + 92);
    rectangle(x - 4, outy, PANEL_X + PANEL_W - 30, outy + 92);
    draw_text_wrap(x + 10, outy + 12, PANEL_W - 76, notice, COLOR_MUTED, 15, 18, 4);

    draw_progress(x, PANEL_Y + 300);
    draw_rules(x, PANEL_Y + 550);
    draw_back_button();
    if (state == STATE_SELECT) {
        button skip = skip_button();
        skip.draw();
    }
    draw_dice_box(state == STATE_ROLL);
}

// 绘制游戏结束面板。
void game::draw_panel_over() {
    int x = PANEL_X + 30;
    int y = PANEL_Y + 82;
    if (winner >= 0) {
        draw_text_left(x, y, player_title(winner) + L" 获胜！", PLAYER_COLORS[winner], 25);
    }
    draw_text_left(x, y + 48, L"本局操作记录已写入 result 文件夹。", COLOR_TEXT, 15);
    if (!logpath.empty()) {
        std::wstring showpath = logpath;
        size_t pos = showpath.find_last_of(L"\\/");
        if (pos != std::wstring::npos) {
            showpath = showpath.substr(pos + 1);
        }
        draw_text_left(x, y + 76, L"文件：" + showpath, COLOR_MUTED, 13);
    }
    draw_progress(x, PANEL_Y + 300);
    draw_rules(x, PANEL_Y + 550);
    draw_back_button();

    button again(x, PANEL_Y + 842, 150, 42, L"重新开始");
    again.bgcolor = COLOR_GOOD;
    again.draw();
    button exitbtn(x + 170, PANEL_Y + 842, 150, 42, L"退出游戏");
    exitbtn.bgcolor = COLOR_DANGER;
    exitbtn.draw();
}

// 绘制返回按钮。
void game::draw_back_button() {
    button one = back_button();
    one.draw();
}

// 绘制骰子区域：缩小蓝色外框和骰子大小。
void game::draw_dice_box(bool active) {
    button area = dice_button();
    COLORREF fill = active ? RGB(245, 248, 252) : RGB(230, 233, 239);
    setfillcolor(fill);
    setlinecolor(active ? RGB(68, 112, 188) : RGB(170, 178, 192));
    setlinestyle(PS_SOLID, active ? 3 : 1);
    fillrectangle(area.x, area.y, area.x + area.w, area.y + area.h);
    rectangle(area.x, area.y, area.x + area.w, area.y + area.h);
    setlinestyle(PS_SOLID, 1);

    int value = dicevalue;
    if (value < 1 || value > 6) {
        value = 1;
    }
    draw_dice(area.x + 25, area.y + 18, 70, value, COLOR_WHITE);
    draw_text_center(area.x, area.y + 98, area.w, 32, active ? L"点击骰子" : L"骰子等待",
                     active ? RGB(38, 67, 122) : COLOR_MUTED, 15);
}

// 绘制玩家完成任务进度：文案改成“xx队完成任务进度”。
void game::draw_progress(int x, int y) {
    draw_text_left(x, y, L"玩家任务进度", COLOR_TEXT, 18);
    y += 34;
    for (int i = 0; i < (int)players.size(); ++i) {
        int finished = players[i].arrivedcount();
        int total = (int)players[i].pieces.size();
        setfillcolor(PLAYER_COLORS[i]);
        setlinecolor(PLAYER_COLORS[i]);
        solidrectangle(x, y + 4, x + 24, y + 28);
        std::wstring text = std::wstring(player_name(i)) + L"完成任务进度  " +
                            std::to_wstring(finished) + L"/" + std::to_wstring(total);

        // statustext：显示本队棋子的当前特殊状态，罚圈全程显示，停止一次结束后消失。
        std::wstring statustext;
        int statuscount = 0; // statuscount：已写入进度行的特殊状态数量，避免文字过长。
        bool morestatus = false; // morestatus：状态过多时用“等”提示。
        for (int k = 0; k < total; ++k) {
            const piece& one = players[i].pieces[k];
            if (one.extralaps > 0) {
                if (statuscount < 2) {
                    if (!statustext.empty()) {
                        statustext += L"；";
                    }
                    statustext += std::to_wstring(one.id + 1) + L"号多跑";
                    statustext += one.extralaps == 1 ? L"一圈" : std::to_wstring(one.extralaps) + L"圈";
                } else {
                    morestatus = true;
                }
                ++statuscount;
            }
            if (one.skipturns > 0) {
                if (statuscount < 2) {
                    if (!statustext.empty()) {
                        statustext += L"；";
                    }
                    statustext += std::to_wstring(one.id + 1) + L"号停止一次";
                } else {
                    morestatus = true;
                }
                ++statuscount;
            }
        }
        if (morestatus) {
            statustext += L"等";
        }

        draw_text_left(x + 34, y + 3, text, COLOR_TEXT, statustext.empty() ? 15 : 14);
        if (!statustext.empty()) {
            draw_text_left(x + 230, y + 4, statustext, RGB(169, 91, 25), 13);
        }
        int barx = x + 34;
        int bary = y + 27;
        int barw = PANEL_W - 94;
        setfillcolor(RGB(224, 229, 238));
        setlinecolor(RGB(224, 229, 238));
        solidrectangle(barx, bary, barx + barw, bary + 8);
        if (total > 0) {
            setfillcolor(PLAYER_COLORS[i]);
            solidrectangle(barx, bary, barx + barw * finished / total, bary + 8);
        }
        y += 34;
    }
}

// 绘制游戏规则说明：字号加大，放在进度区域下面。
void game::draw_rules(int x, int y) {
    draw_text_left(x, y, L"游戏规则说明", COLOR_TEXT, 18);
    y += 28;
    draw_text_left(x, y, L"1. 掷出6可起飞，掷出6可再投一次。", COLOR_MUTED, 14);
    y += 20;
    draw_text_left(x, y, L"2. 落到本队普通色格，飞到下一同色格。", COLOR_MUTED, 14);
    y += 20;
    draw_text_left(x, y, L"3. 落到别人起点，该棋子停一个回合。", COLOR_MUTED, 14);
    y += 20;
    draw_text_left(x, y, L"4. 每条边有黑陷阱，同圈两次踩中多跑一圈。", COLOR_MUTED, 14);
    y += 20;
    draw_text_left(x, y, L"5. 同格不击退，全部棋子进终点列即胜。", COLOR_MUTED, 14);
}

// 绘制同色飞跃路径：用玩家颜色画虚线。
void game::draw_jump_path() {
    if (!jumpactive || jumpfrom < 0 || jumpto < 0 || jumpowner < 0 || jumpowner >= numplayers) {
        return;
    }
    int x1 = 0;
    int y1 = 0;
    int x2 = 0;
    int y2 = 0;
    gameboard.track_center(jumpfrom, x1, y1);
    gameboard.track_center(jumpto, x2, y2);

    COLORREF color = PLAYER_COLORS[jumpowner];
    setlinecolor(color);
    setlinestyle(PS_DASH, 3);
    line(x1, y1, x2, y2);
    setlinestyle(PS_SOLID, 1);

    setfillcolor(COLOR_WHITE);
    setlinecolor(color);
    fillcircle(x1, y1, 9);
    circle(x1, y1, 9);
    fillcircle(x2, y2, 11);
    circle(x2, y2, 11);
}

// 绘制可移动棋子的金色选择圈。
void game::draw_select_marks() {
    for (int i = 0; i < (int)players[curplayer].pieces.size(); ++i) {
        if (!can_move(i)) {
            continue;
        }
        const piece& one = players[curplayer].pieces[i];
        int cx = 0;
        int cy = 0;
        if (one.state == PIECE_BASE) {
            gameboard.base_piece_center(curplayer, one.id, (int)players[curplayer].pieces.size(), cx, cy);
        } else if (one.state == PIECE_TRACK) {
            gameboard.track_center(one.trackpos, cx, cy);
        } else if (one.state == PIECE_FINISH) {
            gameboard.finish_center(curplayer, one.finishpos, cx, cy);
        }
        setlinecolor(COLOR_GOLD);
        setlinestyle(PS_SOLID, 3);
        circle(cx, cy, 23);
        setlinestyle(PS_SOLID, 1);
    }
}

// 处理鼠标点击：按当前状态分发给不同点击函数。
void game::handle_click(int mx, int my) {
    button back = back_button();
    if (back.inside(mx, my)) {
        undo();
        return;
    }

    switch (state) {
    case STATE_SETUP:
        click_setup(mx, my);
        break;
    case STATE_LOT:
        click_lot(mx, my);
        break;
    case STATE_ROLL:
    case STATE_SELECT:
        click_play(mx, my);
        break;
    case STATE_OVER:
        click_over(mx, my);
        break;
    }
}

// 处理设置界面点击。
void game::click_setup(int mx, int my) {
    int x = PANEL_X + 30;
    int y = PANEL_Y + 112;
    for (int i = 0; i < 5; ++i) {
        int val = i + 2;
        button one(x + i * 68, y, 54, 38, std::to_wstring(val));
        if (one.inside(mx, my)) {
            save();
            numplayers = val;
            gameboard.init(numplayers);
            notice = L"已选择 " + std::to_wstring(numplayers) + L" 人游戏。";
            return;
        }
    }

    y = PANEL_Y + 212;
    for (int i = 0; i < numplayers; ++i) {
        int rowy = y + i * 44;
        button minus(x + 190, rowy, 36, 32, L"-");
        minus.enabled = piececounts[i] > 2;
        button plus(x + 282, rowy, 36, 32, L"+");
        plus.enabled = piececounts[i] < 6;
        if (minus.inside(mx, my)) {
            save();
            --piececounts[i];
            notice = player_title(i) + L" 棋子数量减少为 " + std::to_wstring(piececounts[i]) + L"。";
            return;
        }
        if (plus.inside(mx, my)) {
            save();
            ++piececounts[i];
            notice = player_title(i) + L" 棋子数量增加为 " + std::to_wstring(piececounts[i]) + L"。";
            return;
        }
    }

    button start(x, PANEL_Y + 700, PANEL_W - 60, 48, L"开始抽签");
    if (start.inside(mx, my)) {
        save();
        start_game();
        return;
    }
    notice = L"当前点击无效，请在右侧设置区操作。";
}

// 处理抽签界面点击。
void game::click_lot(int mx, int my) {
    button dice = dice_button();
    if (drawn < numplayers && dice.inside(mx, my)) {
        save();
        lot_roll();
        return;
    }

    button start(PANEL_X + 30, PANEL_Y + 700, PANEL_W - 60, 48, L"进入游戏");
    if (drawn >= numplayers && start.inside(mx, my)) {
        save();
        start_play();
        return;
    }
    notice = L"当前点击无效，请按提示点击骰子或进入游戏。";
}

// 处理游戏进行界面点击。
void game::click_play(int mx, int my) {
    if (state == STATE_ROLL) {
        button dice = dice_button();
        if (dice.inside(mx, my)) {
            save();
            roll_dice();
            return;
        }
        notice = L"当前只能点击右下方骰子。";
        return;
    }

    button skip = skip_button();
    if (skip.inside(mx, my)) {
        save();
        skip_move();
        return;
    }

    int idx = gameboard.hit_piece(players, curplayer, mx, my);
    if (idx >= 0 && can_move(idx)) {
        save();
        move_piece(idx);
        return;
    }
    notice = L"当前点击无效，请点击带金色圈的棋子，或选择不走。";
}

// 处理结束界面点击。
void game::click_over(int mx, int my) {
    int x = PANEL_X + 30;
    button again(x, PANEL_Y + 842, 150, 42, L"重新开始");
    button exitbtn(x + 170, PANEL_Y + 842, 150, 42, L"退出游戏");
    if (again.inside(mx, my)) {
        reset();
        return;
    }
    if (exitbtn.inside(mx, my)) {
        quit = true;
        return;
    }
    notice = L"当前点击无效，请选择重新开始或退出游戏。";
}

// 开始游戏：创建玩家、初始化棋盘和日志。
void game::start_game() {
    players.clear();
    for (int i = 0; i < numplayers; ++i) {
        player one;
        one.init(i, piececounts[i], PLAYER_COLORS[i]);
        one.startpos = i * TRACK_COUNT / numplayers;
        one.endpos = (one.startpos + TRACK_COUNT - 1) % TRACK_COUNT;
        players.push_back(one);
    }
    gameboard.init(numplayers);
    drawvalues.assign(numplayers, 0);
    playorder.clear();
    drawn = 0;
    state = STATE_LOT;
    dicevalue = 1;
    hasrolled = false;
    special = L"无";
    jumpactive = false;
    jumpfrom = -1;
    jumpto = -1;
    jumpowner = -1;
    notice = L"开始抽签。";
    start_log();
    log_event(L"开始游戏设置：玩家数 " + std::to_wstring(numplayers));
}

// 抽签掷骰：记录当前抽签玩家点数。
void game::lot_roll() {
    dicevalue = rand() % 6 + 1;
    drawvalues[drawn] = dicevalue;
    players[drawn].drawvalue = dicevalue;
    log_event(player_title(drawn) + L" 抽签点数 " + std::to_wstring(dicevalue));
    ++drawn;
    notice = drawn < numplayers ? L"请下一位玩家点击骰子抽签。" : L"抽签完成，点击进入游戏。";
}

// 根据抽签点数进入正式游戏。
void game::start_play() {
    playorder.resize(numplayers);
    for (int i = 0; i < numplayers; ++i) {
        playorder[i] = i;
    }
    std::sort(playorder.begin(), playorder.end(), [&](int a, int b) {
        return drawvalues[a] == drawvalues[b] ? a < b : drawvalues[a] > drawvalues[b];
    });
    turnindex = 0;
    curplayer = playorder[turnindex];
    state = STATE_ROLL;
    hasrolled = false;
    gotsix = false;
    dicevalue = 1;
    special = L"无";
    notice = L"请点击骰子开始本回合。";
}

// 掷骰：随机生成点数并判断是否有棋子可走。
void game::roll_dice() {
    for (int i = 0; i < (int)players[curplayer].pieces.size(); ++i) {
        if (players[curplayer].pieces[i].skipturns > 0) {
            players[curplayer].pieces[i].skipfresh = false;
        }
    }
    hasrolled = true;
    special = L"无";
    jumpactive = false;
    jumpfrom = -1;
    jumpto = -1;
    jumpowner = -1;
    for (int i = 0; i < 12; ++i) {
        dicevalue = rand() % 6 + 1;
        notice = L"骰子转动中。";
        draw();
        Sleep(45);
    }
    dicevalue = rand() % 6 + 1;
    gotsix = dicevalue == 6;
    log_event(player_title(curplayer) + L" 投掷点数 " + std::to_wstring(dicevalue));
    state = STATE_SELECT;
    if (!any_move()) {
        special = L"无棋可走";
        notice = L"没有可移动棋子，本回合自动跳过。";
        log_event(player_title(curplayer) + L" 无棋可走，自动跳过。");
        draw();
        Sleep(700);
        if (gotsix) {
            state = STATE_ROLL;
            hasrolled = false;
            dicevalue = 1;
            gotsix = false;
            special = L"无";
            notice = L"无棋可走，但掷出 6，继续投骰子。";
        } else {
            next_turn();
        }
        return;
    }
    notice = gotsix ? L"掷出 6，可起飞或移动，完成后额外再投一次。" : L"请选择要移动的棋子，也可以不走。";
}

// 判断当前玩家是否至少有一个棋子可以移动。
bool game::any_move() const {
    for (int i = 0; i < (int)players[curplayer].pieces.size(); ++i) {
        if (can_move(i)) {
            return true;
        }
    }
    return false;
}

// 判断指定棋子是否可以按当前骰子点数移动。
bool game::can_move(int index) const {
    if (index < 0 || index >= (int)players[curplayer].pieces.size()) {
        return false;
    }
    const piece& one = players[curplayer].pieces[index];
    if (one.skipturns > 0) {
        return false;
    }
    if (one.state == PIECE_BASE) {
        return dicevalue == 6;
    }
    if (one.state == PIECE_TRACK) {
        moveresult res = gameboard.move(one.trackpos, dicevalue, players[curplayer].startpos);
        return one.extralaps > 0 || res.valid;
    }
    if (one.state == PIECE_FINISH) {
        int need = FINISH_COUNT - 1 - one.finishpos;
        return dicevalue <= need;
    }
    return false;
}

// 移动棋子：处理起飞、普通移动、终点列移动和特殊格。
void game::move_piece(int index) {
    piece& one = players[curplayer].pieces[index];
    std::wstring text = player_title(curplayer) + L" 移动 " + std::to_wstring(one.id + 1) + L"号棋子：";
    special = L"无";

    if (one.state == PIECE_BASE) {
        one.state = PIECE_TRACK;
        one.trackpos = players[curplayer].startpos;
        text += L"起飞到起点。";
    } else if (one.state == PIECE_TRACK) {
        moveresult res = gameboard.move(one.trackpos, dicevalue, players[curplayer].startpos);
        if (res.enterfinish && one.extralaps > 0) {
            one.trackpos = (one.trackpos + dicevalue) % TRACK_COUNT;
            --one.extralaps;
            one.trapcount = 0;
            text += L"完成额外一圈的一段，落在第 " + std::to_wstring(one.trackpos) + L" 格。";
            text += apply_special(one, curplayer);
        } else if (res.enterfinish) {
            one.state = PIECE_FINISH;
            one.finishpos = res.finishpos;
            one.trapcount = 0;
            text += L"进入终点列第 " + std::to_wstring(res.finishpos + 1) + L" 格。";
            if (one.finishpos >= FINISH_COUNT - 1) {
                one.state = PIECE_HOME;
                text += L"到达停车区。";
            }
        } else {
            one.trackpos = res.newpos;
            text += L"前进到第 " + std::to_wstring(one.trackpos) + L" 格。";
            text += apply_special(one, curplayer);
        }
    } else if (one.state == PIECE_FINISH) {
        one.finishpos += dicevalue;
        text += L"在终点列前进到第 " + std::to_wstring(one.finishpos + 1) + L" 格。";
        if (one.finishpos >= FINISH_COUNT - 1) {
            one.state = PIECE_HOME;
            text += L"到达停车区。";
        }
    }

    notice = text;
    log_event(text);
    finish_action();
}

// 玩家选择不走。
void game::skip_move() {
    std::wstring text = player_title(curplayer) + L" 选择不走。";
    notice = text;
    special = L"无";
    log_event(text);
    finish_action();
}

// 完成一次行动后：判断胜利、额外回合或切换下一个玩家。
void game::finish_action() {
    if (check_win()) {
        state = STATE_OVER;
        log_event(player_title(winner) + L" 获胜，本局结束。");
        close_log();
        return;
    }
    if (gotsix) {
        state = STATE_ROLL;
        hasrolled = false;
        dicevalue = 1;
        gotsix = false;
        notice += L" 掷出 6，继续投骰子。";
        return;
    }
    next_turn(notice);
}

// 切换到下一名玩家。
void game::next_turn(const std::wstring& last) {
    reduce_skips(curplayer);
    turnindex = (turnindex + 1) % numplayers;
    curplayer = playorder[turnindex];
    state = STATE_ROLL;
    hasrolled = false;
    dicevalue = 1;
    if (last.empty()) {
        special = L"无";
        notice = L"轮到 " + player_title(curplayer) + L"，请点击骰子。";
    } else {
        notice = last + L"\n轮到 " + player_title(curplayer) + L"，请点击骰子。";
    }
}

// 减少指定玩家棋子的停回合计数。
void game::reduce_skips(int owner) {
    if (owner < 0 || owner >= (int)players.size()) {
        return;
    }
    for (int i = 0; i < (int)players[owner].pieces.size(); ++i) {
        if (players[owner].pieces[i].skipturns > 0) {
            if (players[owner].pieces[i].skipfresh) {
                players[owner].pieces[i].skipfresh = false;
                continue;
            }
            --players[owner].pieces[i].skipturns;
        }
    }
}

// 处理棋子落点上的特殊效果。
std::wstring game::apply_special(piece& one, int owner) {
    if (one.state != PIECE_TRACK) {
        return L"";
    }
    int pos = one.trackpos;

    if (gameboard.cells[pos].type == CELL_START &&
        gameboard.cells[pos].owner >= 0 &&
        gameboard.cells[pos].owner != owner) {
        if (one.skipturns < 1) {
            one.skipturns = 1;
        }
        one.skipfresh = true;
        special = L"别人起点：停一回合";
        return L" 落到" + player_title(gameboard.cells[pos].owner) + L"的起点，该棋子停一个回合。";
    }

    auto trap = gameboard.trapmap.find(pos);
    if (trap != gameboard.trapmap.end()) {
        ++one.trapcount;
        if (one.trapcount >= 2) {
            ++one.extralaps;
            one.trapcount = 0;
            special = L"黑色陷阱：多跑一圈";
            return L" 进入黑色陷阱，本圈第2次踩中，该棋子需要额外多跑一圈。";
        }
        special = L"黑色陷阱";
        return L" 进入黑色陷阱，本圈已踩中1次。";
    }

    if (gameboard.cells[pos].type == CELL_NORMAL && gameboard.cells[pos].owner == owner) {
        int next = gameboard.next_color_cell(pos, owner);
        if (next >= 0 && next != pos) {
            int from = pos;

            // 如果同色跳跃会碰到或越过本队终点入口，直接进入停车区最里面，
            // 避免终点前一格继续跳到外圈其他颜色位置。
            int endpos = (players[owner].startpos + TRACK_COUNT - 1) % TRACK_COUNT;
            int disttoend = (endpos - pos + TRACK_COUNT) % TRACK_COUNT;
            int disttonext = (next - pos + TRACK_COUNT) % TRACK_COUNT;
            if (one.extralaps <= 0 && disttoend <= disttonext) {
                one.state = PIECE_HOME;
                one.finishpos = FINISH_COUNT - 1;
                one.trapcount = 0;
                jumpactive = false;
                jumpfrom = -1;
                jumpto = -1;
                jumpowner = -1;
                special = L"终点前同色直达停车区";
                return L" 直接落到进终点前一格，改为跳到停车区最里面。";
            }

            one.trackpos = next;
            jumpactive = true;
            jumpfrom = from;
            jumpto = next;
            jumpowner = owner;
            special = L"直接同色飞跃";
            return L" 直接落到自己的颜色格，触发同色飞跃：从第 " +
                   std::to_wstring(from) + L" 格飞到第 " + std::to_wstring(next) + L" 格。";
        }
    }

    jumpactive = false;
    jumpfrom = -1;
    jumpto = -1;
    jumpowner = -1;
    special = L"无";
    return L"";
}

// 旧击退接口保留为空：当前规则为同格不击退，只错位显示。
std::wstring game::check_hit(piece& one, int owner) {
    (void)one;
    (void)owner;
    return L"";
}

// 判断胜利：所有棋子进入终点列或停车区即胜。
bool game::check_win() {
    for (int i = 0; i < (int)players.size(); ++i) {
        if (players[i].allarrived()) {
            winner = i;
            return true;
        }
    }
    return false;
}

// 开始日志文件：在result目录下按时间命名。
void game::start_log() {
    close_log();
    std::wstring root = root_path();
    std::wstring folder = root + L"\\result";
    CreateDirectoryW(folder.c_str(), nullptr);

    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t name[128];
    swprintf_s(name, L"%04d%02d%02d_%02d%02d%02d.txt",
               st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    logpath = folder + L"\\" + name;
    _wfopen_s(&logfile, logpath.c_str(), L"ab");
}

// 关闭日志文件。
void game::close_log() {
    if (logfile != nullptr) {
        fclose(logfile);
        logfile = nullptr;
    }
}

// 写入一条GBK日志。
void game::log_event(const std::wstring& text) {
    if (logfile == nullptr) {
        return;
    }
    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t prefix[64];
    swprintf_s(prefix, L"[%02d:%02d:%02d] ", st.wHour, st.wMinute, st.wSecond);
    std::wstring line = std::wstring(prefix) + text + L"\r\n";
    std::string gbk = gbk_text(line);
    fwrite(gbk.data(), 1, gbk.size(), logfile);
    fflush(logfile);
}

// 寻找解决方案根目录。
std::wstring game::root_path() const {
    wchar_t buf[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, buf);
    std::wstring cur = buf;
    for (int i = 0; i < 6; ++i) {
        std::wstring sln = cur + L"\\FlightChess.sln";
        if (GetFileAttributesW(sln.c_str()) != INVALID_FILE_ATTRIBUTES) {
            return cur;
        }
        size_t pos = cur.find_last_of(L"\\/");
        if (pos == std::wstring::npos) {
            break;
        }
        cur = cur.substr(0, pos);
    }
    return buf;
}

// 宽字符串转GBK字符串，用于日志输出。
std::string game::gbk_text(const std::wstring& text) const {
    if (text.empty()) {
        return std::string();
    }
    int size = WideCharToMultiByte(936, 0, text.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (size <= 1) {
        return std::string();
    }
    std::string out(size - 1, '\0');
    WideCharToMultiByte(936, 0, text.c_str(), -1, &out[0], size, nullptr, nullptr);
    return out;
}
