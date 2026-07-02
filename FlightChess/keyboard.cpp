#include "keyboard.h"
#include "game.h"

// handle_keyboard：键盘模块入口，保持 game.cpp 的主循环简洁。
void handle_keyboard(game& app, int key, int scancode) {
    app.handle_key(key, scancode);
}

// handle_key：按当前游戏阶段处理键盘操作，所有非法按键只提示不改变状态。
void game::handle_key(int key, int scancode) {
    if (key >= 'a' && key <= 'z') {
        key = key - 'a' + 'A';
    }

    if (key == KEY_UNDO) {
        undo();
        return;
    }

    if (state == STATE_SETUP) {
        if (key == KEY_LEFT_PLAYER || key == KEY_NUMPAD_FIRST_PIECE + 3) {
            if (numplayers > 2) {
                save();
                --numplayers;
                gameboard.init(numplayers);
                notice = L"键盘操作：玩家人数减少为 " + std::to_wstring(numplayers) + L" 人。";
            } else {
                notice = L"玩家人数不能少于 2 人。";
            }
            return;
        }
        if (key == KEY_RIGHT_PLAYER || key == KEY_NUMPAD_LAST_PIECE) {
            if (numplayers < 6) {
                save();
                ++numplayers;
                gameboard.init(numplayers);
                notice = L"键盘操作：玩家人数增加为 " + std::to_wstring(numplayers) + L" 人。";
            } else {
                notice = L"玩家人数不能超过 6 人。";
            }
            return;
        }
        if (key == KEY_UP_PIECES || scancode == KEY_NUMPAD_SCANCODE_8) {
            bool changed = false;
            for (int i = 0; i < numplayers; ++i) {
                if (piececounts[i] < 6) {
                    changed = true;
                    break;
                }
            }
            if (changed) {
                save();
                for (int i = 0; i < numplayers; ++i) {
                    if (piececounts[i] < 6) {
                        ++piececounts[i];
                    }
                }
                notice = L"键盘操作：所有队伍棋子数量增加 1。";
            } else {
                notice = L"所有队伍棋子数量已经是 6。";
            }
            return;
        }
        if (key == KEY_DOWN_PIECES || key == KEY_NUMPAD_FIRST_PIECE + 1) {
            bool changed = false;
            for (int i = 0; i < numplayers; ++i) {
                if (piececounts[i] > 2) {
                    changed = true;
                    break;
                }
            }
            if (changed) {
                save();
                for (int i = 0; i < numplayers; ++i) {
                    if (piececounts[i] > 2) {
                        --piececounts[i];
                    }
                }
                notice = L"键盘操作：所有队伍棋子数量减少 1。";
            } else {
                notice = L"所有队伍棋子数量已经是 2。";
            }
            return;
        }
        if (key == KEY_CONFIRM) {
            save();
            start_game();
            return;
        }
        notice = L"设置界面键盘：左右调人数，上下调棋子数，回车开始，Delete返回。";
        return;
    }

    if (state == STATE_LOT && key == KEY_CONFIRM && drawn >= numplayers) {
        save();
        start_play();
        return;
    }

    if (key == KEY_DICE) {
        if (state == STATE_LOT && drawn < numplayers) {
            save();
            lot_roll();
            return;
        }
        if (state == STATE_ROLL) {
            save();
            roll_dice();
            return;
        }
        notice = L"当前不能投骰子，请按右侧提示操作。";
        return;
    }

    if (state != STATE_SELECT) {
        notice = L"当前键盘操作无效：空格投骰，能走时按0不走或按1-6选择棋子。";
        return;
    }

    if (key == KEY_SKIP || key == KEY_NUMPAD_SKIP || key == KEY_NUMLOCK_OFF_SKIP ||
        scancode == KEY_NUMPAD_SCANCODE_0) {
        save();
        skip_move();
        return;
    }

    int index = -1; // index：键盘数字对应的棋子下标。
    if (key >= KEY_FIRST_PIECE && key <= KEY_LAST_PIECE) {
        index = key - KEY_FIRST_PIECE;
    } else if (key >= KEY_NUMPAD_FIRST_PIECE && key <= KEY_NUMPAD_LAST_PIECE) {
        index = key - KEY_NUMPAD_FIRST_PIECE;
    } else if (scancode == KEY_NUMPAD_SCANCODE_1) {
        index = 0;
    } else if (scancode == KEY_NUMPAD_SCANCODE_2) {
        index = 1;
    } else if (scancode == KEY_NUMPAD_SCANCODE_3) {
        index = 2;
    } else if (scancode == KEY_NUMPAD_SCANCODE_4) {
        index = 3;
    } else if (scancode == KEY_NUMPAD_SCANCODE_5) {
        index = 4;
    } else if (scancode == KEY_NUMPAD_SCANCODE_6) {
        index = 5;
    } else if (key == KEY_NUMLOCK_OFF_1) {
        index = 0;
    } else if (key == KEY_NUMLOCK_OFF_2) {
        index = 1;
    } else if (key == KEY_NUMLOCK_OFF_3) {
        index = 2;
    } else if (key == KEY_NUMLOCK_OFF_4) {
        index = 3;
    } else if (key == KEY_NUMLOCK_OFF_5) {
        index = 4;
    } else if (key == KEY_NUMLOCK_OFF_6) {
        index = 5;
    }

    if (index >= 0) {
        if (index >= 0 && index < (int)players[curplayer].pieces.size() && can_move(index)) {
            save();
            move_piece(index);
            return;
        }
        notice = L"当前棋子不能走，请按带金色圈的棋子编号，或按0不走。";
        return;
    }

    notice = L"当前键盘操作无效：空格投骰，能走时按0不走或按1-6选择棋子。";
}
