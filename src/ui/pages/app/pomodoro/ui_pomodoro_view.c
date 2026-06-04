/**
 * @file ui_pomodoro_view.c
 * @brief 番茄时钟视图层实现（纯渲染 + 事件转发到 Presenter）。
 */

#include "ui_pomodoro_view.h"
#include "ui_pomodoro.h"
#include "../../../components/ui_titlebar.h"
#include "../../../components/ui_statusbar.h"
#include <stdio.h>
#include <string.h>

/* iOS 极简主题色（CONSTRAINTS.md §2） */
#define COLOR_BG          lv_color_hex(0xF5F5F7)
#define COLOR_SURFACE     lv_color_hex(0xFAFAFC)
#define COLOR_TEXT_MAIN   lv_color_hex(0x111111)
#define COLOR_TEXT_SECOND lv_color_hex(0x8E8E93)
#define COLOR_ACCENT      lv_color_hex(0x007AFF)
#define COLOR_WORK        lv_color_hex(0xFF3B30) /* 工作：番茄红 */
#define COLOR_BREAK       lv_color_hex(0x34C759) /* 休息：绿色   */
#define COLOR_DIVIDER     lv_color_hex(0xE5E5EA)

#define UI_RADIUS         12
#define BTN_RADIUS        10
#define TASK_ITEM_HEIGHT  52

/* ============================================================
 * 事件转发器：把 LVGL 事件翻译成 Presenter 的业务动作
 * ============================================================ */
static void on_start_pause(lv_event_t * e)
{
    LV_UNUSED(e);
    ui_pomodoro_action_start_pause();
}

static void on_reset(lv_event_t * e)
{
    LV_UNUSED(e);
    ui_pomodoro_action_reset();
}

static void on_toggle_mode(lv_event_t * e)
{
    LV_UNUSED(e);
    ui_pomodoro_action_toggle_mode();
}

static void on_task_changed(lv_event_t * e)
{
    lv_obj_t * dd = lv_event_get_target(e);
    int32_t idx = (int32_t)lv_dropdown_get_selected(dd);
    ui_pomodoro_action_select_task(idx);
}

static void on_open_tasks(lv_event_t * e)
{
    LV_UNUSED(e);
    ui_pomodoro_route_to_tasks();
}

/* ============================================================
 * 主计时页面（项 1.1）
 * ============================================================ */
lv_obj_t * ui_pomodoro_view_create_main(lv_obj_t * parent, ui_pomodoro_view_t * v)
{
    memset(v, 0, sizeof(*v));

    /* 内容根：避开状态栏+标题栏 */
    lv_obj_t * root = lv_obj_create(parent);
    v->root = root;
    lv_obj_set_size(root, UI_SCREEN_WIDTH, ui_get_content_height());
    lv_obj_set_pos(root, 0, ui_get_content_y());
    lv_obj_set_style_bg_color(root, COLOR_BG, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_pad_all(root, 0, 0);
    lv_obj_set_style_radius(root, 0, 0);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);

    /* ---- 顶部信息卡：左侧表盘，右侧状态与任务 ---- */
    lv_obj_t * hero = lv_obj_create(root);
    lv_obj_set_size(hero, UI_SCREEN_WIDTH - 24, 112);
    lv_obj_set_pos(hero, 12, 6);
    lv_obj_set_style_bg_color(hero, COLOR_SURFACE, 0);
    lv_obj_set_style_bg_opa(hero, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(hero, UI_RADIUS, 0);
    lv_obj_set_style_border_width(hero, 1, 0);
    lv_obj_set_style_border_color(hero, COLOR_DIVIDER, 0);
    lv_obj_set_style_shadow_width(hero, 10, 0);
    lv_obj_set_style_shadow_opa(hero, LV_OPA_10, 0);
    lv_obj_set_style_pad_all(hero, 0, 0);
    lv_obj_clear_flag(hero, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * timer_slot = lv_obj_create(hero);
    lv_obj_set_size(timer_slot, 104, 104);
    lv_obj_set_pos(timer_slot, 8, 4);
    lv_obj_set_style_bg_opa(timer_slot, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(timer_slot, 0, 0);
    lv_obj_set_style_pad_all(timer_slot, 0, 0);
    lv_obj_clear_flag(timer_slot, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * arc = lv_arc_create(timer_slot);
    v->arc = arc;
    lv_obj_set_size(arc, 98, 98);
    lv_obj_center(arc);
    lv_arc_set_rotation(arc, 270);
    lv_arc_set_bg_angles(arc, 0, 360);
    lv_arc_set_range(arc, 0, 1000);
    lv_arc_set_value(arc, 1000);
    lv_obj_remove_flag(arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_width(arc, 8, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, COLOR_DIVIDER, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc, 8, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc, COLOR_WORK, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(arc, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_pad_all(arc, 0, LV_PART_KNOB);

    lv_obj_t * time_lbl = lv_label_create(timer_slot);
    v->time_label = time_lbl;
    lv_label_set_text(time_lbl, "25:00");
    lv_obj_set_style_text_font(time_lbl, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(time_lbl, COLOR_TEXT_MAIN, 0);
    lv_obj_center(time_lbl);

    lv_obj_t * mode_lbl = lv_label_create(hero);
    v->mode_label = mode_lbl;
    lv_label_set_text(mode_lbl, "工作中");
    lv_obj_set_style_text_font(mode_lbl, &font, 0);
    lv_obj_set_style_text_color(mode_lbl, COLOR_WORK, 0);
    lv_obj_set_pos(mode_lbl, 124, 10);

    lv_obj_t * today_txt = lv_label_create(hero);
    lv_label_set_text(today_txt, "今日番茄");
    lv_obj_set_style_text_font(today_txt, &font, 0);
    lv_obj_set_style_text_color(today_txt, COLOR_TEXT_SECOND, 0);
    lv_obj_set_pos(today_txt, 124, 36);

    lv_obj_t * today_num = lv_label_create(hero);
    v->today_num = today_num;
    lv_label_set_text(today_num, "0");
    lv_obj_set_style_text_font(today_num, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(today_num, COLOR_WORK, 0);
    lv_obj_set_pos(today_num, 194, 36);

    lv_obj_t * tomato_num = lv_label_create(hero);
    v->task_tomato = tomato_num;
    lv_label_set_text(tomato_num, "x0");
    lv_obj_set_style_text_font(tomato_num, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(tomato_num, COLOR_WORK, 0);
    lv_obj_set_pos(tomato_num, 230, 36);

    lv_obj_t * task_dd = lv_dropdown_create(hero);
    v->task_dd = task_dd;
    lv_obj_set_size(task_dd, 106, 30);
    lv_obj_set_pos(task_dd, 124, 70);
    lv_obj_set_style_radius(task_dd, BTN_RADIUS, 0);
    lv_obj_set_style_bg_color(task_dd, lv_color_white(), 0);
    lv_obj_set_style_border_color(task_dd, COLOR_DIVIDER, 0);
    lv_obj_set_style_border_width(task_dd, 1, 0);
    lv_obj_set_style_text_font(task_dd, &font, 0);
    lv_obj_set_style_text_color(task_dd, COLOR_TEXT_MAIN, 0);
    lv_dropdown_set_options(task_dd, "无任务");
    lv_obj_add_event_cb(task_dd, on_task_changed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_t * dd_list = lv_dropdown_get_list(task_dd);
    if(dd_list) lv_obj_set_style_text_font(dd_list, &font, 0);

    lv_obj_t * task_btn = lv_button_create(hero);
    lv_obj_set_size(task_btn, 48, 30);
    lv_obj_set_pos(task_btn, 236, 70);
    lv_obj_set_style_radius(task_btn, BTN_RADIUS, 0);
    lv_obj_set_style_bg_color(task_btn, COLOR_ACCENT, 0);
    lv_obj_set_style_shadow_width(task_btn, 0, 0);
    lv_obj_add_event_cb(task_btn, on_open_tasks, LV_EVENT_CLICKED, NULL);
    lv_obj_t * task_lbl = lv_label_create(task_btn);
    lv_label_set_text(task_lbl, "任务");
    lv_obj_set_style_text_font(task_lbl, &font, 0);
    lv_obj_set_style_text_color(task_lbl, lv_color_white(), 0);
    lv_obj_center(task_lbl);

    /* ---- 底部操作区：三个按钮独立于表盘，避免遮挡 ---- */
    lv_obj_t * btn_bar = lv_obj_create(root);
    lv_obj_set_size(btn_bar, UI_SCREEN_WIDTH, 44);
    lv_obj_set_pos(btn_bar, 0, 126);
    lv_obj_set_style_bg_opa(btn_bar, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_bar, 0, 0);
    lv_obj_set_style_pad_all(btn_bar, 0, 0);
    lv_obj_clear_flag(btn_bar, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * btn_start = lv_button_create(btn_bar);
    v->btn_start = btn_start;
    lv_obj_set_size(btn_start, 104, 38);
    lv_obj_set_pos(btn_start, 16, 2);
    lv_obj_set_style_radius(btn_start, BTN_RADIUS, 0);
    lv_obj_set_style_bg_color(btn_start, COLOR_WORK, 0);
    lv_obj_set_style_shadow_width(btn_start, 0, 0);
    lv_obj_add_event_cb(btn_start, on_start_pause, LV_EVENT_CLICKED, NULL);
    lv_obj_t * lbl_start = lv_label_create(btn_start);
    v->lbl_start = lbl_start;
    lv_label_set_text(lbl_start, "开始");
    lv_obj_set_style_text_font(lbl_start, &font, 0);
    lv_obj_set_style_text_color(lbl_start, lv_color_white(), 0);
    lv_obj_center(lbl_start);

    lv_obj_t * btn_reset = lv_button_create(btn_bar);
    v->btn_reset = btn_reset;
    lv_obj_set_size(btn_reset, 82, 38);
    lv_obj_set_pos(btn_reset, 128, 2);
    lv_obj_set_style_radius(btn_reset, BTN_RADIUS, 0);
    lv_obj_set_style_bg_color(btn_reset, lv_color_white(), 0);
    lv_obj_set_style_border_width(btn_reset, 1, 0);
    lv_obj_set_style_border_color(btn_reset, COLOR_DIVIDER, 0);
    lv_obj_set_style_shadow_width(btn_reset, 0, 0);
    lv_obj_add_event_cb(btn_reset, on_reset, LV_EVENT_CLICKED, NULL);
    lv_obj_t * lbl_reset = lv_label_create(btn_reset);
    lv_label_set_text(lbl_reset, "重置");
    lv_obj_set_style_text_font(lbl_reset, &font, 0);
    lv_obj_set_style_text_color(lbl_reset, COLOR_TEXT_MAIN, 0);
    lv_obj_center(lbl_reset);

    lv_obj_t * toggle_btn = lv_button_create(btn_bar);
    v->btn_toggle = toggle_btn;
    lv_obj_set_size(toggle_btn, 82, 38);
    lv_obj_set_pos(toggle_btn, 222, 2);
    lv_obj_set_style_radius(toggle_btn, BTN_RADIUS, 0);
    lv_obj_set_style_bg_color(toggle_btn, lv_color_white(), 0);
    lv_obj_set_style_shadow_width(toggle_btn, 0, 0);
    lv_obj_set_style_border_width(toggle_btn, 1, 0);
    lv_obj_set_style_border_color(toggle_btn, COLOR_DIVIDER, 0);
    lv_obj_add_event_cb(toggle_btn, on_toggle_mode, LV_EVENT_CLICKED, NULL);
    lv_obj_t * toggle_lbl = lv_label_create(toggle_btn);
    lv_label_set_text(toggle_lbl, "切换");
    lv_obj_set_style_text_font(toggle_lbl, &font, 0);
    lv_obj_set_style_text_color(toggle_lbl, COLOR_ACCENT, 0);
    lv_obj_center(toggle_lbl);

    return root;
}

void ui_pomodoro_view_set_mode(ui_pomodoro_view_t * v, const char * mode_name, bool is_work)
{
    if(v == NULL) return;
    lv_color_t c = is_work ? COLOR_WORK : COLOR_BREAK;
    if(v->mode_label && lv_obj_is_valid(v->mode_label)) {
        lv_label_set_text(v->mode_label, mode_name);
        lv_obj_set_style_text_color(v->mode_label, c, 0);
    }
    if(v->arc && lv_obj_is_valid(v->arc)) {
        lv_obj_set_style_arc_color(v->arc, c, LV_PART_INDICATOR);
    }
    if(v->btn_start && lv_obj_is_valid(v->btn_start)) {
        lv_obj_set_style_bg_color(v->btn_start, c, 0);
    }
}

void ui_pomodoro_view_set_time(ui_pomodoro_view_t * v, int32_t total_seconds, int32_t remaining_seconds)
{
    if(v == NULL) return;
    if(remaining_seconds < 0) remaining_seconds = 0;
    int32_t mm = remaining_seconds / 60;
    int32_t ss = remaining_seconds % 60;
    if(v->time_label && lv_obj_is_valid(v->time_label)) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%02d:%02d", (int)mm, (int)ss);
        lv_label_set_text(v->time_label, buf);
    }
    if(v->arc && lv_obj_is_valid(v->arc) && total_seconds > 0) {
        int32_t val = (int32_t)(((int64_t)remaining_seconds * 1000) / total_seconds);
        lv_arc_set_value(v->arc, val);
    }
}

void ui_pomodoro_view_set_running(ui_pomodoro_view_t * v, bool running)
{
    if(v == NULL) return;
    if(v->lbl_start && lv_obj_is_valid(v->lbl_start)) {
        lv_label_set_text(v->lbl_start, running ? "暂停" : "开始");
    }
}

void ui_pomodoro_view_set_today(ui_pomodoro_view_t * v, int32_t today_count)
{
    if(v == NULL) return;
    if(v->today_num && lv_obj_is_valid(v->today_num)) {
        char buf[12];
        snprintf(buf, sizeof(buf), "%d", (int)today_count);
        lv_label_set_text(v->today_num, buf);
    }
}

void ui_pomodoro_view_set_task(ui_pomodoro_view_t * v, const char * options,
                               int32_t selected, int32_t tomato_count)
{
    if(v == NULL) return;
    if(v->task_dd && lv_obj_is_valid(v->task_dd)) {
        lv_dropdown_set_options(v->task_dd, options ? options : "无任务");
        uint32_t cnt = lv_dropdown_get_option_count(v->task_dd);
        if(selected < 0) selected = 0;
        if((uint32_t)selected >= cnt && cnt > 0) selected = (int32_t)cnt - 1;
        lv_dropdown_set_selected(v->task_dd, (uint32_t)selected);
    }
    if(v->task_tomato && lv_obj_is_valid(v->task_tomato)) {
        char buf[12];
        snprintf(buf, sizeof(buf), "x%d", (int)tomato_count);
        lv_label_set_text(v->task_tomato, buf);
    }
}

/* ============================================================
 * 任务列表页面（项 1.3 UI）
 * ============================================================ */
static void on_task_check(lv_event_t * e)
{
    lv_obj_t * cb = lv_event_get_target(e);
    int32_t idx = (int32_t)(intptr_t)lv_event_get_user_data(e);
    LV_UNUSED(cb);
    ui_pomodoro_action_toggle_task(idx);
}

static void on_task_delete(lv_event_t * e)
{
    int32_t idx = (int32_t)(intptr_t)lv_event_get_user_data(e);
    ui_pomodoro_action_delete_task(idx);
}

static void on_task_add(lv_event_t * e)
{
    LV_UNUSED(e);
    /* 用递增编号生成默认任务名（避免引入键盘输入复杂度，保证可移植与可测） */
    const ui_pomodoro_store_t * st = ui_pomodoro_get_store();
    char name[UI_POMODORO_TASK_NAME_LEN];
    snprintf(name, sizeof(name), "任务%d", (int)(st ? st->task_count + 1 : 1));
    ui_pomodoro_action_add_task(name);
}

lv_obj_t * ui_pomodoro_view_create_tasks(lv_obj_t * parent)
{
    /* 内容根 */
    lv_obj_t * root = lv_obj_create(parent);
    lv_obj_set_size(root, UI_SCREEN_WIDTH, ui_get_content_height());
    lv_obj_set_pos(root, 0, ui_get_content_y());
    lv_obj_set_style_bg_color(root, COLOR_BG, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_pad_top(root, 6, 0);
    lv_obj_set_style_pad_left(root, 0, 0);
    lv_obj_set_style_pad_right(root, 0, 0);
    lv_obj_set_style_pad_bottom(root, 0, 0);
    lv_obj_set_style_radius(root, 0, 0);
    lv_obj_set_scroll_dir(root, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(root, LV_SCROLLBAR_MODE_OFF);

    const ui_pomodoro_store_t * st = ui_pomodoro_get_store();
    if(st == NULL) return root;

    if(st->task_count == 0) {
        lv_obj_t * empty = lv_label_create(root);
        lv_label_set_text(empty, "暂无任务，点击右上角添加");
        lv_obj_set_style_text_font(empty, &font, 0);
        lv_obj_set_style_text_color(empty, COLOR_TEXT_SECOND, 0);
        lv_obj_align(empty, LV_ALIGN_TOP_MID, 0, 20);
        return root;
    }

    for(int32_t i = 0; i < st->task_count; i++) {
        const ui_pomodoro_task_t * task = &st->tasks[i];

        lv_obj_t * item = lv_obj_create(root);
        lv_obj_set_size(item, UI_SCREEN_WIDTH - 20, TASK_ITEM_HEIGHT);
        lv_obj_set_pos(item, 10, i * (TASK_ITEM_HEIGHT + 8));
        lv_obj_set_style_bg_color(item, COLOR_SURFACE, 0);
        lv_obj_set_style_radius(item, UI_RADIUS, 0);
        lv_obj_set_style_border_width(item, 1, 0);
        lv_obj_set_style_border_color(item, COLOR_DIVIDER, 0);
        lv_obj_set_style_pad_all(item, 0, 0);
        lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);

        /* 勾选框 + 任务名（完成时删除线 + 灰字） */
        lv_obj_t * cb = lv_checkbox_create(item);
        lv_checkbox_set_text(cb, task->name);
        lv_obj_set_style_text_font(cb, &font, 0);
        lv_obj_set_style_text_color(cb, task->completed ? COLOR_TEXT_SECOND : COLOR_TEXT_MAIN, 0);
        if(task->completed) {
            lv_obj_add_state(cb, LV_STATE_CHECKED);
            lv_obj_set_style_text_decor(cb, LV_TEXT_DECOR_STRIKETHROUGH, 0);
        }
        lv_obj_align(cb, LV_ALIGN_LEFT_MID, 12, 0);
        lv_obj_add_event_cb(cb, on_task_check, LV_EVENT_VALUE_CHANGED, (void *)(intptr_t)i);

        /* 累计番茄数（montserrat 数字） */
        lv_obj_t * cnt = lv_label_create(item);
        char cbuf[16];
        snprintf(cbuf, sizeof(cbuf), "x%d", (int)task->tomato_count);
        lv_label_set_text(cnt, cbuf);
        lv_obj_set_style_text_font(cnt, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(cnt, COLOR_WORK, 0);
        lv_obj_align(cnt, LV_ALIGN_RIGHT_MID, -52, 0);

        /* 删除按钮 */
        lv_obj_t * del = lv_button_create(item);
        lv_obj_set_size(del, 34, 34);
        lv_obj_align(del, LV_ALIGN_RIGHT_MID, -8, 0);
        lv_obj_set_style_radius(del, BTN_RADIUS, 0);
        lv_obj_set_style_bg_color(del, lv_color_hex(0xFF3B30), 0);
        lv_obj_set_style_shadow_width(del, 0, 0);
        lv_obj_add_event_cb(del, on_task_delete, LV_EVENT_CLICKED, (void *)(intptr_t)i);
        lv_obj_t * del_lbl = lv_label_create(del);
        lv_label_set_text(del_lbl, LV_SYMBOL_TRASH);
        lv_obj_set_style_text_color(del_lbl, lv_color_white(), 0);
        lv_obj_center(del_lbl);
    }

    return root;
}

/* ============================================================
 * "时间到" 轻提示浮窗
 * ============================================================ */
void ui_pomodoro_view_show_toast(const char * text, uint32_t duration_ms)
{
    lv_obj_t * toast = lv_obj_create(lv_layer_top());
    lv_obj_set_size(toast, 160, 60);
    lv_obj_center(toast);
    lv_obj_set_style_bg_color(toast, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_bg_opa(toast, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(toast, UI_RADIUS, 0);
    lv_obj_set_style_border_width(toast, 0, 0);
    lv_obj_set_style_shadow_width(toast, 18, 0);
    lv_obj_set_style_shadow_opa(toast, LV_OPA_30, 0);
    lv_obj_clear_flag(toast, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * lbl = lv_label_create(toast);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_font(lbl, &font, 0);
    lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
    lv_obj_center(lbl);

    lv_obj_fade_in(toast, 150, 0);
    lv_obj_fade_out(toast, 200, duration_ms);
    lv_obj_delete_delayed(toast, duration_ms + 220);
}
