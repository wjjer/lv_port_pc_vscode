#ifndef UI_CALCULATOR_H
#define UI_CALCULATOR_H

#include "lvgl.h"
#include "../../../ui_config.h"


extern const lv_font_t font;

// 使用 ui_config.h 中的统一定义
#define STATUSBAR_HEIGHT  UI_STATUSBAR_HEIGHT
#define TITLEBAR_HEIGHT   UI_TITLEBAR_HEIGHT

// 计算器页面显示/隐藏
void ui_calculator_show(void);
void ui_calculator_hide(void);

#endif
