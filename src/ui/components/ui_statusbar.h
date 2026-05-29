#ifndef UI_STATUSBAR_H
#define UI_STATUSBAR_H

#include "lvgl.h"
#include "../ui_config.h"

// 中文字体
extern const lv_font_t font;


/**
 * @brief 创建状态栏（包含快捷面板）
 * 调用此函数初始化状态栏组件
 */
void ui_statusbar_create(void);

/**
 * @brief 更新状态栏颜色（根据背景色自动取反文字和图标颜色）
 * @param bg_color 背景颜色
 */
void update_status_bar_color(lv_color_t bg_color);


// 状态栏时间标签
extern lv_obj_t * bar_time_label;
// 状态栏对象
extern lv_obj_t * status_bar;
// 状态栏右侧图标容器
extern lv_obj_t * bar_right_icons;

#endif
