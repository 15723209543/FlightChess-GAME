#pragma once
#include <windows.h>

// 键盘键位集中配置，方便后续修改。
const int KEY_DICE = VK_SPACE; // KEY_DICE：空格键，投骰子或抽签。
const int KEY_CONFIRM = VK_RETURN; // KEY_CONFIRM：回车键，设置界面开始抽签。
const int KEY_UNDO = VK_DELETE; // KEY_UNDO：Delete键，返回上一步。
const int KEY_LEFT_PLAYER = VK_LEFT; // KEY_LEFT_PLAYER：左方向键，设置界面减少人数。
const int KEY_RIGHT_PLAYER = VK_RIGHT; // KEY_RIGHT_PLAYER：右方向键，设置界面增加人数。
const int KEY_UP_PIECES = VK_UP; // KEY_UP_PIECES：上方向键，设置界面同时增加棋子数。
const int KEY_DOWN_PIECES = VK_DOWN; // KEY_DOWN_PIECES：下方向键，设置界面同时减少棋子数。
const int KEY_SKIP = '0';      // KEY_SKIP：数字0，选择本回合不走。
const int KEY_FIRST_PIECE = '1'; // KEY_FIRST_PIECE：数字1，选择1号棋子。
const int KEY_LAST_PIECE = '6';  // KEY_LAST_PIECE：数字6，选择6号棋子。
const int KEY_NUMPAD_SKIP = VK_NUMPAD0;      // KEY_NUMPAD_SKIP：小键盘0，选择本回合不走。
const int KEY_NUMPAD_FIRST_PIECE = VK_NUMPAD1; // KEY_NUMPAD_FIRST_PIECE：小键盘1，选择1号棋子。
const int KEY_NUMPAD_LAST_PIECE = VK_NUMPAD6;  // KEY_NUMPAD_LAST_PIECE：小键盘6，选择6号棋子。
const int KEY_NUMLOCK_OFF_SKIP = VK_INSERT; // KEY_NUMLOCK_OFF_SKIP：NumLock关闭时小键盘0。
const int KEY_NUMLOCK_OFF_1 = VK_END;       // KEY_NUMLOCK_OFF_1：NumLock关闭时小键盘1。
const int KEY_NUMLOCK_OFF_2 = VK_DOWN;      // KEY_NUMLOCK_OFF_2：NumLock关闭时小键盘2。
const int KEY_NUMLOCK_OFF_3 = VK_NEXT;      // KEY_NUMLOCK_OFF_3：NumLock关闭时小键盘3。
const int KEY_NUMLOCK_OFF_4 = VK_LEFT;      // KEY_NUMLOCK_OFF_4：NumLock关闭时小键盘4。
const int KEY_NUMLOCK_OFF_5 = VK_CLEAR;     // KEY_NUMLOCK_OFF_5：NumLock关闭时小键盘5。
const int KEY_NUMLOCK_OFF_6 = VK_RIGHT;     // KEY_NUMLOCK_OFF_6：NumLock关闭时小键盘6。
const int KEY_NUMPAD_SCANCODE_0 = 0x52; // KEY_NUMPAD_SCANCODE_0：小键盘0扫描码。
const int KEY_NUMPAD_SCANCODE_1 = 0x4F; // KEY_NUMPAD_SCANCODE_1：小键盘1扫描码。
const int KEY_NUMPAD_SCANCODE_2 = 0x50; // KEY_NUMPAD_SCANCODE_2：小键盘2扫描码。
const int KEY_NUMPAD_SCANCODE_3 = 0x51; // KEY_NUMPAD_SCANCODE_3：小键盘3扫描码。
const int KEY_NUMPAD_SCANCODE_4 = 0x4B; // KEY_NUMPAD_SCANCODE_4：小键盘4扫描码。
const int KEY_NUMPAD_SCANCODE_5 = 0x4C; // KEY_NUMPAD_SCANCODE_5：小键盘5扫描码。
const int KEY_NUMPAD_SCANCODE_6 = 0x4D; // KEY_NUMPAD_SCANCODE_6：小键盘6扫描码。
const int KEY_NUMPAD_SCANCODE_8 = 0x48; // KEY_NUMPAD_SCANCODE_8：小键盘8扫描码。
