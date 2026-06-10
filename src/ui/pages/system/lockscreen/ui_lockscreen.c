#include "ui_lockscreen.h"
#include "../../../ui.h"
#include <stdio.h>
#include <time.h>
#include <string.h>

// 桌面 Tileview 引用（用于读取背景色）
extern lv_obj_t * _desktop_tv;

#define COLOR_LOCKSCREEN_TEXT lv_color_white()

// 持有锁屏根容器
static lv_obj_t * lockscreen_page = NULL;

// 时间标签
static lv_obj_t * lock_time_label = NULL;
static lv_obj_t * lock_date_label = NULL;
// 时间更新定时器
static lv_timer_t * lock_time_timer = NULL;

// 状态栏引用（需要隐藏/显示）
extern lv_obj_t * status_bar;

// 根据桌面背景色设置锁屏背景
static void set_lockscreen_bg_from_desktop(lv_obj_t * obj) {
    if (_desktop_tv != NULL) {
        lv_color_t bg_color = lv_obj_get_style_bg_color(_desktop_tv, 0);
        lv_obj_set_style_bg_color(obj, bg_color, 0);
    } else {
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x000000), 0);
    }
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
}

/**
 * 创建锁屏主内容：仅显示大时钟 + 日期
 */
static lv_obj_t * create_lockscreen_content(lv_obj_t * parent) {
    int32_t w = lv_display_get_horizontal_resolution(lv_display_get_default());

    // 隐藏全局状态栏（锁屏独占全屏）
    if (status_bar) {
        lv_obj_add_flag(status_bar, LV_OBJ_FLAG_HIDDEN);
    }

    // ---- 大时钟（HH:MM） ----
    lock_time_label = lv_label_create(parent);
    lv_label_set_text(lock_time_label, "10:24");
    {
        const lv_font_t * tf;
        if (w <= 240) tf = &lv_font_montserrat_48;
        else if (w <= 480) tf = &lv_font_montserrat_46;
        else tf = &lv_font_montserrat_48;
        lv_obj_set_style_text_font(lock_time_label, tf, 0);
    }
    lv_obj_set_style_text_color(lock_time_label, COLOR_LOCKSCREEN_TEXT, 0);
    lv_obj_align(lock_time_label, LV_ALIGN_CENTER, 0, -25);

    // ---- 日期 ----
    lock_date_label = lv_label_create(parent);
    lv_label_set_text(lock_date_label, "6月10日  星期二");
    lv_obj_set_style_text_font(lock_date_label, &font, 0);
    lv_obj_set_style_text_color(lock_date_label, COLOR_LOCKSCREEN_TEXT, 0);
    lv_obj_align(lock_date_label, LV_ALIGN_CENTER, 0, 25);

    return parent;
}

// 拖拽追踪状态
static int32_t drag_start_y = 0;
static int32_t drag_start_lock_y = 0;
static bool dragging = false;

/**
 * 锁屏触摸事件：追踪拖拽距离
 * 使用 PRESSING 实时响应拖拽 + RELEASED 判断是否解锁
 */
static void lockscreen_touch_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if (code == LV_EVENT_PRESSED) {
        // 记录按下时的 Y 坐标
        lv_indev_t * indev = lv_indev_get_act();
        if (!indev) return;
        lv_point_t pos;
        lv_indev_get_point(indev, &pos);
        drag_start_y = pos.y;
        drag_start_lock_y = (int32_t)lv_obj_get_y(lockscreen_page);
        dragging = true;
    }
    else if (code == LV_EVENT_PRESSING && dragging) {
        // 拖拽中：跟随手指移动
        lv_indev_t * indev = lv_indev_get_act();
        if (!indev) return;
        lv_point_t pos;
        lv_indev_get_point(indev, &pos);
        int32_t dy = pos.y - drag_start_y;
        // 只允许向上拖拽（dy < 0）
        if (dy < 0) {
            lv_obj_set_y(lockscreen_page, drag_start_lock_y + dy);
        }
    }
    else if (code == LV_EVENT_RELEASED && dragging) {
        dragging = false;
        lv_indev_t * indev = lv_indev_get_act();
        if (!indev) return;
        lv_point_t pos;
        lv_indev_get_point(indev, &pos);
        int32_t dy = pos.y - drag_start_y;

        if (dy < -60) {
            // 上滑超过 60px：触发解锁动画
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
            lv_anim_set_var(&a, lockscreen_page);
            lv_anim_set_values(&a, lv_obj_get_y(lockscreen_page), -(int32_t)UI_SCREEN_HEIGHT + 10);
            lv_anim_set_duration(&a, 350);
            lv_anim_start(&a);
            ui_lockscreen_hide();
        } else {
            // 回弹到原位
            lv_obj_set_y(lockscreen_page, 0);
        }
    }
}

/**
 * 锁屏时间更新定时器
 */
static void lockscreen_time_update_timer(lv_timer_t * timer) {
    (void)timer;
    if (lockscreen_page == NULL || !lv_obj_is_valid(lockscreen_page)) return;
    if (lock_time_label == NULL || !lv_obj_is_valid(lock_time_label)) return;

    time_t rawtime;
    struct tm * timeinfo;
    char buf[16];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buf, sizeof(buf), "%H:%M", timeinfo);
    lv_label_set_text(lock_time_label, buf);
}

void ui_lockscreen_show(void) {
    // 自清理：防止重复创建
    ui_lockscreen_hide();

    // 创建在全屏 top layer
    lockscreen_page = lv_obj_create(lv_layer_top());

    lv_obj_set_size(lockscreen_page, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT);
    lv_obj_set_pos(lockscreen_page, 0, 0);
    set_lockscreen_bg_from_desktop(lockscreen_page);
    lv_obj_set_style_border_width(lockscreen_page, 0, 0);
    lv_obj_set_style_radius(lockscreen_page, 0, 0);
    lv_obj_clear_flag(lockscreen_page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(lockscreen_page, LV_OBJ_FLAG_CLICKABLE);

    // 创建内容
    create_lockscreen_content(lockscreen_page);

    // 绑定触摸事件（ PRESSING + RELEASED 追踪拖拽）
    lv_obj_add_event_cb(lockscreen_page, lockscreen_touch_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(lockscreen_page, lockscreen_touch_cb, LV_EVENT_PRESSING, NULL);
    lv_obj_add_event_cb(lockscreen_page, lockscreen_touch_cb, LV_EVENT_RELEASED, NULL);

    // 初始化显示时间
    time_t rawtime;
    struct tm * timeinfo;
    char buf[32];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buf, sizeof(buf), "%H:%M", timeinfo);
    lv_label_set_text(lock_time_label, buf);

    static const char * week_days[] = {"星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"};
    strftime(buf, sizeof(buf), "%m月%d日", timeinfo);
    lv_label_set_text_fmt(lock_date_label, "%s  %s", buf, week_days[timeinfo->tm_wday]);

    // 每秒更新时间的定时器
    lock_time_timer = lv_timer_create(lockscreen_time_update_timer, 1000, NULL);
}

void ui_lockscreen_hide(void) {
    // 先删除定时器，避免操作已删除的对象
    if (lock_time_timer != NULL) {
        lv_timer_delete(lock_time_timer);
        lock_time_timer = NULL;
    }
    if (lockscreen_page != NULL) {
        // 先删除所有对象
        lv_obj_add_flag(lockscreen_page, LV_OBJ_FLAG_HIDDEN);
        lv_obj_delete(lockscreen_page);
        lockscreen_page = NULL;
        lock_time_label = NULL;
        lock_date_label = NULL;
    }
    // 恢复状态栏可见
    if (status_bar) {
        lv_obj_remove_flag(status_bar, LV_OBJ_FLAG_HIDDEN);
    }
    // 重置拖拽状态
    dragging = false;
}
