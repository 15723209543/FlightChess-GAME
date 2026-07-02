#pragma once
#include "keyboard_data.h"

class game;

// handle_keyboard：把键盘按键和扫描码分发给游戏对象处理。
void handle_keyboard(game& app, int key, int scancode);
