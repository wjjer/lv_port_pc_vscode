#ifndef UI_TITLEBAR_H
#define UI_TITLEBAR_H

#include "lvgl.h"
#include "../ui_config.h"

extern const lv_font_t font;

#define STATUSBAR_HEIGHT  UI_STATUSBAR_HEIGHT
#define TITLEBAR_HEIGHT   UI_TITLEBAR_HEIGHT

/**
 * @brief 创建通用标题栏（会自动偏移避开状态栏）
 * @param parent 父对象
 * @param title 标题文字
 * @param back_cb 返回按钮回调（可为空）
 * @param back_user_data 返回回调参数
 * @param right_text 右侧按钮文字（可为空）
 * @param right_cb 右侧按钮回调（可为空）
 * @param right_user_data 右侧回调参数
 * @param bg_color 背景颜色（设为 NULL 则自动从父对象获取）
 * @param text_color 文字颜色（设为 NULL 则自动适配背景）
 * @return 标题栏容器对象
 */
lv_obj_t * ui_titlebar_create(lv_obj_t * parent, const char * title,
                              lv_event_cb_t back_cb, void * back_user_data,
                              const char * right_text, lv_event_cb_t right_cb, void * right_user_data,
                              const lv_color_t * bg_color, const lv_color_t * text_color,
                              bool transparent_bg);

/**
 * @brief 获取状态栏高度常量
 */
int ui_titlebar_get_statusbar_height(void);

/**
 * @brief 获取标题栏高度常量
 */
int ui_titlebar_get_titlebar_height(void);

#endif
