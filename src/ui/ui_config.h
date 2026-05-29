#ifndef UI_CONFIG_H
#define UI_CONFIG_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==============================
 * 屏幕尺寸
 * 可用于运行时赋值，但不能用于 #if 预处理判断。
 * ============================== */
#define UI_SCREEN_WIDTH   LV_HOR_RES
#define UI_SCREEN_HEIGHT  LV_VER_RES

/* ==============================
 * 状态栏 / 标题栏高度（动态计算，根据屏幕宽度适配）
 * 小屏(<=240): 24/32, 中屏(<=480): 30/40, 大屏(>480): 36/48
 * ============================== */
static inline int32_t ui_get_statusbar_height(void) {
    int32_t w = lv_display_get_horizontal_resolution(lv_display_get_default());
    if(w <= 240) return 24;
    if(w <= 480) return 30;
    return 36;
}

static inline int32_t ui_get_titlebar_height(void) {
    int32_t w = lv_display_get_horizontal_resolution(lv_display_get_default());
    if(w <= 240) return 32;
    if(w <= 480) return 40;
    return 48;
}

#define UI_STATUSBAR_HEIGHT   ui_get_statusbar_height()
#define UI_TITLEBAR_HEIGHT    ui_get_titlebar_height()


/**
 * @brief 获取图标容器宽度
 */
static inline int32_t ui_get_icon_container_w(void) {
    int32_t w = lv_display_get_horizontal_resolution(lv_display_get_default());
    if(w <= 128) return 38;
    if(w <= 240) return 52;
    if(w <= 480) return 70;
    return 90;
}

/**
 * @brief 获取图标容器高度
 */
static inline int32_t ui_get_icon_container_h(void) {
    int32_t w = lv_display_get_horizontal_resolution(lv_display_get_default());
    if(w <= 128) return 55;
    if(w <= 240) return 72;
    if(w <= 480) return 95;
    return 120;
}

/**
 * @brief 获取图标图片尺寸（正方形）
 */
static inline int32_t ui_get_icon_img_size(void) {
    int32_t w = lv_display_get_horizontal_resolution(lv_display_get_default());
    if(w <= 128) return 30;
    if(w <= 240) return 42;
    if(w <= 480) return 55;
    return 70;
}

/**
 * @brief 获取图标圆角半径
 */
static inline int32_t ui_get_icon_radius(void) {
    int32_t w = lv_display_get_horizontal_resolution(lv_display_get_default());
    if(w <= 128) return 6;
    if(w <= 240) return 10;
    if(w <= 480) return 14;
    return 18;
}

/**
 * @brief 获取桌面图标区内边距
 */
static inline int32_t ui_get_desktop_padding(void) {
    int32_t w = lv_display_get_horizontal_resolution(lv_display_get_default());
    if(w <= 128) return 5;
    if(w <= 240) return 10;
    if(w <= 480) return 20;
    return 30;
}

/**
 * @brief 获取桌面图标行间距
 */
static inline int32_t ui_get_desktop_row_gap(void) {
    int32_t w = lv_display_get_horizontal_resolution(lv_display_get_default());
    if(w <= 128) return 8;
    if(w <= 240) return 12;
    if(w <= 480) return 18;
    return 25;
}

/**
 * @brief 获取桌面图标列间距
 */
static inline int32_t ui_get_desktop_col_gap(void) {
    int32_t w = lv_display_get_horizontal_resolution(lv_display_get_default());
    if(w <= 128) return 5;
    if(w <= 240) return 10;
    if(w <= 480) return 18;
    return 25;
}

/**
 * @brief 获取桌面图标区顶部偏移
 */
static inline int32_t ui_get_desktop_top_pad(void) {
    int32_t w = lv_display_get_horizontal_resolution(lv_display_get_default());
    if(w <= 128) return 10;
    if(w <= 240) return 20;
    if(w <= 480) return 30;
    return 40;
}

/**
 * @brief 获取内容区域可用高度（屏幕高度 - 状态栏 - 标题栏）
 */
static inline int32_t ui_get_content_height(void) {
    return UI_SCREEN_HEIGHT - UI_STATUSBAR_HEIGHT - UI_TITLEBAR_HEIGHT;
}

/**
 * @brief 获取内容区域 Y 起始坐标（状态栏 + 标题栏）
 */
static inline int32_t ui_get_content_y(void) {
    return UI_STATUSBAR_HEIGHT + UI_TITLEBAR_HEIGHT;
}

/* ==============================
 * APP 启动回调机制
 * ============================== */

/**
 * @brief APP 启动回调函数类型
 * @param app_id 应用标识符，如 "ai_assistant", "settings" 等
 */
typedef void (*app_launch_callback_t)(const char * app_id);

/**
 * @brief 设置 APP 启动回调
 * @param cb 回调函数指针
 */
void ui_set_app_launch_callback(app_launch_callback_t cb);

#ifdef __cplusplus
}
#endif

#endif /* UI_CONFIG_H */
