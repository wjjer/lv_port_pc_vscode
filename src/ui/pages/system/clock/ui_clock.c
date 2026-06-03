#include "ui_clock.h"
#include "ui_alarm_create.h"
#include "../../../components/ui_titlebar.h"
#include "../../../ui.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

extern lv_obj_t *bar_time_label;

// =========================
// 极简 iOS 风格主题
// =========================
#define COLOR_BG lv_color_hex(0xF5F5F7)
#define COLOR_SURFACE lv_color_hex(0xFAFAFC)
#define COLOR_TEXT_MAIN lv_color_hex(0x111111)
#define COLOR_TEXT_SECOND lv_color_hex(0x8E8E93)
#define COLOR_DIVIDER lv_color_hex(0xE5E5EA)
#define COLOR_ACCENT lv_color_hex(0x007AFF)
#define COLOR_GREEN lv_color_hex(0x34C759)
#define COLOR_RED lv_color_hex(0xFF3B30)

#define UI_RADIUS 12
#define ITEM_HEIGHT 72

static lv_obj_t *clock_page;
static lv_obj_t *content_area;
static lv_obj_t *titlebar;
static lv_obj_t *sw_label;
static lv_obj_t *sw_min_label;
static lv_obj_t *sw_sec_label;
static lv_obj_t *sw_ms_label;
static lv_obj_t *lap_list;

static lv_timer_t *sw_timer = NULL;

static uint32_t sw_elapsed_ms = 0;
static uint32_t sw_start_ms = 0;
static uint32_t sw_lap_time = 0;

static bool sw_running = false;

#define MAX_LAPS 20
static uint32_t laps[MAX_LAPS];
static int lap_count = 0;
static int current_tab = 0;
static lv_obj_t * tab_buttons[2];

// =========================
// 闹钟结构
// =========================
#define MAX_ALARMS 10

typedef struct
{
    uint8_t hour;
    uint8_t minute;
    bool active;
    char label[64];
} alarm_t;

static alarm_t alarms[MAX_ALARMS];
static int alarm_count = 3;

// =========================
// 前置声明
// =========================
void render_alarm_view(void);
void render_stopwatch_view(void);
static void add_alarm_from_create_page(void);
static void render_lap_list(void);
static void alarm_delete_at(int index);

// 长按删除：记录当前待删除的闹钟下标
static int pending_delete_index = -1;

// =========================
// 秒表计时器
// =========================
static void sw_timer_cb(lv_timer_t *t)
{
    LV_UNUSED(t);

    if (!sw_running)
        return;

    uint32_t diff =
        (lv_tick_get() - sw_start_ms) + sw_elapsed_ms;

    int min = (int)(diff / 60000);
    int sec = (int)((diff / 1000) % 60);
    int ms = (int)((diff % 1000));

    if (sw_min_label)
        lv_label_set_text_fmt(sw_min_label, "%02d", min);
    if (sw_sec_label)
        lv_label_set_text_fmt(sw_sec_label, "%02d", sec);
    if (sw_ms_label)
        lv_label_set_text_fmt(sw_ms_label, "%03d", ms);
}

// =========================
// 秒表按钮
// =========================
static void sw_btn_event_cb(lv_event_t *e)
{
    LV_UNUSED(e);

    if (!sw_running)
    {
        sw_start_ms = lv_tick_get();
        sw_running = true;
    }
    else
    {
        sw_elapsed_ms += (lv_tick_get() - sw_start_ms);
        sw_running = false;
    }

    render_stopwatch_view();
}

// =========================
// 计次按钮
// =========================
static void sw_lap_cb(lv_event_t *e)
{
    LV_UNUSED(e);

    if (!sw_running && sw_elapsed_ms == 0)
        return;

    if (lap_count < MAX_LAPS)
    {
        uint32_t current_time = sw_running ? (lv_tick_get() - sw_start_ms) + sw_elapsed_ms : sw_elapsed_ms;

        uint32_t lap_diff = lap_count == 0 ? current_time : current_time - sw_lap_time;
        laps[lap_count] = lap_diff;
        sw_lap_time = current_time;
        lap_count++;

        render_lap_list();
    }
}

// =========================
// 重置按钮
// =========================
static void sw_reset_cb(lv_event_t *e)
{
    LV_UNUSED(e);

    sw_elapsed_ms = 0;
    sw_start_ms = 0;
    sw_lap_time = 0;
    sw_running = false;
    lap_count = 0;
    memset(laps, 0, sizeof(laps));

    render_stopwatch_view();
}

// =========================
// 渲染计次列表
// =========================
static void render_lap_list(void)
{
    if (!lap_list)
        return;

    lv_obj_clean(lap_list);

    for (int i = lap_count - 1; i >= 0; i--)
    {
        lv_obj_t *lap_item = lv_obj_create(lap_list);
        lv_obj_set_size(lap_item, lv_pct(100), 32);
        lv_obj_set_pos(lap_item, 0, (lap_count - 1 - i) * 32);
        lv_obj_set_style_bg_opa(lap_item, 0, 0);
        lv_obj_set_style_border_width(lap_item, 0, 0);
        lv_obj_set_style_pad_all(lap_item, 0, 0);
        lv_obj_set_style_pad_hor(lap_item, 15, 0);
        lv_obj_remove_flag(lap_item, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t *lap_num = lv_label_create(lap_item);
        lv_label_set_text_fmt(lap_num, "Lap %d", lap_count - i);
        lv_obj_set_style_text_font(lap_num, &font, 0);
        lv_obj_set_style_text_color(lap_num, COLOR_TEXT_SECOND, 0);
        lv_obj_align(lap_num, LV_ALIGN_LEFT_MID, 0, 0);

        lv_obj_t *lap_time = lv_label_create(lap_item);
        uint32_t lap_ms = laps[i];
        int lap_min = (int)(lap_ms / 60000);
        int lap_sec = (int)((lap_ms / 1000) % 60);
        int lap_cs = (int)((lap_ms % 1000) / 10);
        lv_label_set_text_fmt(lap_time, "%02d:%02d.%02d", lap_min, lap_sec, lap_cs);
        lv_obj_set_style_text_font(lap_time, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(lap_time, COLOR_TEXT_MAIN, 0);
        lv_obj_align(lap_time, LV_ALIGN_RIGHT_MID, 0, 0);
    }
}

// =========================
// 删除指定闹钟（数组前移保持紧凑）
// =========================
static void alarm_delete_at(int index)
{
    if (index < 0 || index >= alarm_count)
        return;

    for (int i = index; i < alarm_count - 1; i++)
        alarms[i] = alarms[i + 1];

    alarm_count--;
    memset(&alarms[alarm_count], 0, sizeof(alarm_t));

    render_alarm_view();
}

// =========================
// 删除确认弹窗回调
// =========================
static void alarm_del_confirm_cb(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    // footer button 的祖先即 msgbox 根对象
    lv_obj_t *mbox = lv_obj_get_parent(lv_obj_get_parent(btn));

    alarm_delete_at(pending_delete_index);
    pending_delete_index = -1;

    lv_msgbox_close(mbox);
}

static void alarm_del_cancel_cb(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *mbox = lv_obj_get_parent(lv_obj_get_parent(btn));

    pending_delete_index = -1;
    lv_msgbox_close(mbox);
}

// =========================
// 闹钟项长按 → 弹出删除确认
// =========================
static void alarm_item_long_press_cb(lv_event_t *e)
{
    int index = (int)(uintptr_t)lv_event_get_user_data(e);
    if (index < 0 || index >= alarm_count)
        return;

    pending_delete_index = index;

    // 9.5 标准 msgbox：parent 传 NULL 自动在 lv_layer_top() 创建模态遮罩
    lv_obj_t *mbox = lv_msgbox_create(NULL);
    lv_obj_set_style_text_font(mbox, &font, 0);
    lv_obj_set_style_radius(mbox, UI_RADIUS, 0);

    lv_msgbox_add_title(mbox, "删除闹钟");
    lv_msgbox_add_text_fmt(mbox, "确定删除 %02d:%02d%s%s ?",
                           alarms[index].hour,
                           alarms[index].minute,
                           alarms[index].label[0] ? " " : "",
                           alarms[index].label);

    lv_obj_t *btn_cancel = lv_msgbox_add_footer_button(mbox, "取消");
    lv_obj_set_style_text_font(btn_cancel, &font, 0);
    lv_obj_add_event_cb(btn_cancel, alarm_del_cancel_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn_del = lv_msgbox_add_footer_button(mbox, "删除");
    lv_obj_set_style_text_font(btn_del, &font, 0);
    lv_obj_set_style_bg_color(btn_del, COLOR_RED, 0);
    lv_obj_set_style_text_color(btn_del, lv_color_white(), 0);
    lv_obj_add_event_cb(btn_del, alarm_del_confirm_cb, LV_EVENT_CLICKED, NULL);
}

// =========================
// 闹钟开关切换 → 同步 active 状态
// =========================
static void alarm_switch_cb(lv_event_t *e)
{
    lv_obj_t *sw = lv_event_get_target(e);
    int index = (int)(uintptr_t)lv_event_get_user_data(e);
    if (index < 0 || index >= alarm_count)
        return;

    alarms[index].active = lv_obj_has_state(sw, LV_STATE_CHECKED);
}

// =========================
// 极简闹钟 Item
// =========================
static void add_alarm_item(lv_obj_t *parent,
                           int index,
                           int y)
{
    alarm_t *alarm = &alarms[index];

    lv_obj_t *obj = lv_obj_create(parent);

    lv_obj_set_size(obj, lv_pct(100), ITEM_HEIGHT);
    lv_obj_set_pos(obj, 0, y);

    lv_obj_set_style_bg_opa(obj, 0, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);

    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    // 长按删除（item 默认带 CLICKABLE，可直接收长按事件）
    lv_obj_add_event_cb(obj, alarm_item_long_press_cb, LV_EVENT_LONG_PRESSED,
                        (void *)(uintptr_t)index);

    // 时间
    lv_obj_t *time_l = lv_label_create(obj);

    lv_label_set_text_fmt(time_l,
                          "%02d:%02d",
                          alarm->hour,
                          alarm->minute);

    lv_obj_set_style_text_font(time_l,
                               &lv_font_montserrat_32,
                               0);

    lv_obj_set_style_text_color(time_l,
                                COLOR_TEXT_MAIN,
                                0);

    lv_obj_align(time_l,
                 LV_ALIGN_LEFT_MID,
                 16,
                 -8);

    // 标签
    if (alarm->label[0] != '\0')
    {

        lv_obj_t *sub = lv_label_create(obj);

        lv_label_set_text(sub, alarm->label);

        lv_obj_set_style_text_font(sub,
                                   &font,
                                   0);

        lv_obj_set_style_text_color(sub,
                                    COLOR_TEXT_SECOND,
                                    0);

        lv_obj_align(sub,
                     LV_ALIGN_LEFT_MID,
                     18,
                     18);
    }

    // 开关
    lv_obj_t *sw = lv_switch_create(obj);

    lv_obj_set_size(sw, 42, 24);

    lv_obj_align(sw,
                 LV_ALIGN_RIGHT_MID,
                 -12,
                 0);

    lv_obj_set_style_bg_color(sw,
                              COLOR_GREEN,
                              LV_PART_INDICATOR | LV_STATE_CHECKED);

    if (alarm->active)
        lv_obj_add_state(sw, LV_STATE_CHECKED);

    // 开关切换回调：把开关状态写回数据
    lv_obj_add_event_cb(sw, alarm_switch_cb, LV_EVENT_VALUE_CHANGED,
                        (void *)(uintptr_t)index);

    // 分割线
    lv_obj_t *line = lv_obj_create(obj);

    lv_obj_set_size(line, lv_pct(100), 1);

    lv_obj_align(line,
                 LV_ALIGN_BOTTOM_MID,
                 0,
                 0);

    lv_obj_set_style_bg_color(line,
                              COLOR_DIVIDER,
                              0);

    lv_obj_set_style_border_width(line, 0, 0);
    lv_obj_set_style_radius(line, 0, 0);
}

// =========================
// 添加新建闹钟按钮
// =========================
static void show_create_page_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    ui_alarm_create_show();
}

// =========================
// 检查并添加新闹钟
// =========================
static void add_alarm_from_create_page(void)
{
    if (!ui_alarm_create_has_data())
        return;

    alarm_data_t new_data;
    ui_alarm_create_get_data(&new_data);

    if (alarm_count >= MAX_ALARMS)
        return;

    alarms[alarm_count].hour = new_data.hour;
    alarms[alarm_count].minute = new_data.minute;
    alarms[alarm_count].active = new_data.active;
    strncpy(alarms[alarm_count].label, new_data.label, sizeof(alarms[0].label) - 1);
    alarms[alarm_count].label[sizeof(alarms[0].label) - 1] = '\0';
    alarm_count++;

    render_alarm_view();
}

// =========================
// 闹钟页面
// =========================
void render_alarm_view(void)
{
    if (sw_timer)
        lv_timer_pause(sw_timer);

    lv_obj_clean(content_area);

    lv_obj_set_scroll_dir(content_area,LV_DIR_VER);

    lv_obj_set_scrollbar_mode(content_area, LV_SCROLLBAR_MODE_OFF);

    lv_obj_set_style_pad_left(content_area, 14,0);

    lv_obj_set_style_pad_right(content_area,14,0);

    lv_obj_set_style_pad_top(content_area, 10, 0);

    for (int i = 0; i < alarm_count; i++)
    {
        add_alarm_item(content_area, i,i * ITEM_HEIGHT);
    }
}

// =========================
// 秒表页面
// =========================
void render_stopwatch_view(void)
{
    lv_obj_clean(content_area);
    lv_obj_set_scroll_dir(content_area,LV_DIR_VER);
    lv_obj_set_scrollbar_mode(content_area,LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_pad_all(content_area, 0, 0);
    int min = (int)(sw_elapsed_ms / 60000);
    int sec = (int)((sw_elapsed_ms / 1000) % 60);
    int ms = (int)(sw_elapsed_ms % 1000);

    lv_obj_t *time_container = lv_obj_create(content_area);
    lv_obj_set_size(time_container, UI_SCREEN_WIDTH, 80);
    lv_obj_set_pos(time_container, 0, 0);
    lv_obj_set_style_bg_opa(time_container, 0, 0);
    lv_obj_set_style_border_width(time_container, 0, 0);
    lv_obj_set_style_pad_all(time_container, 0, 0);
    lv_obj_remove_flag(time_container, LV_OBJ_FLAG_SCROLLABLE);

    int time_width = 180;
    int time_start_x = (UI_SCREEN_WIDTH - time_width) / 2;

    sw_min_label = lv_label_create(time_container);
    lv_label_set_text_fmt(sw_min_label, "%02d", min);
    lv_obj_set_style_text_font(sw_min_label, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(sw_min_label, COLOR_TEXT_MAIN, 0);
    lv_obj_set_pos(sw_min_label, time_start_x, 20);

    lv_obj_t *colon1 = lv_label_create(time_container);
    lv_label_set_text(colon1, ":");
    lv_obj_set_style_text_font(colon1, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(colon1, COLOR_TEXT_MAIN, 0);
    lv_obj_set_pos(colon1, time_start_x + 55, 20);

    sw_sec_label = lv_label_create(time_container);
    lv_label_set_text_fmt(sw_sec_label, "%02d", sec);
    lv_obj_set_style_text_font(sw_sec_label, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(sw_sec_label, COLOR_TEXT_MAIN, 0);
    lv_obj_set_pos(sw_sec_label, time_start_x + 75, 20);

    lv_obj_t *colon2 = lv_label_create(time_container);
    lv_label_set_text(colon2, ".");
    lv_obj_set_style_text_font(colon2, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(colon2, COLOR_TEXT_MAIN, 0);
    lv_obj_set_pos(colon2, time_start_x + 125, 20);

    sw_ms_label = lv_label_create(time_container);
    lv_label_set_text_fmt(sw_ms_label, "%03d", ms);
    lv_obj_set_style_text_font(sw_ms_label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(sw_ms_label, COLOR_TEXT_SECOND, 0);
    lv_obj_set_pos(sw_ms_label, time_start_x + 140, 28);

    lv_obj_t *btn_container = lv_obj_create(content_area);
    lv_obj_set_size(btn_container, UI_SCREEN_WIDTH, 45);
    lv_obj_set_pos(btn_container, 0, 75);
    lv_obj_set_style_bg_opa(btn_container, 0, 0);
    lv_obj_set_style_border_width(btn_container, 0, 0);
    lv_obj_set_style_pad_all(btn_container, 0, 0);
    lv_obj_remove_flag(btn_container, LV_OBJ_FLAG_SCROLLABLE);

    int btn_width = 65;
    int btn_gap = 10;
    int total_btn_width = btn_width * 3 + btn_gap * 2;
    int btn_start_x = (UI_SCREEN_WIDTH - total_btn_width) / 2;

    lv_obj_t *lap_btn = lv_button_create(btn_container);
    lv_obj_set_size(lap_btn, btn_width, 34);
    lv_obj_set_pos(lap_btn, btn_start_x, 5);
    lv_obj_set_style_radius(lap_btn, 17, 0);
    lv_obj_set_style_bg_color(lap_btn, COLOR_SURFACE, 0);
    lv_obj_set_style_shadow_width(lap_btn, 0, 0);
    lv_obj_set_style_border_width(lap_btn, 0, 0);

    lv_obj_t *lap_lbl = lv_label_create(lap_btn);
    lv_label_set_text(lap_lbl, "计次");
    lv_obj_set_style_text_font(lap_lbl, &font, 0);
    lv_obj_set_style_text_color(lap_lbl, COLOR_ACCENT, 0);
    lv_obj_center(lap_lbl);

    lv_obj_add_event_cb(lap_btn, sw_lap_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *main_btn = lv_button_create(btn_container);
    lv_obj_set_size(main_btn, btn_width, 34);
    lv_obj_set_pos(main_btn, btn_start_x + btn_width + btn_gap, 5);
    lv_obj_set_style_radius(main_btn, 17, 0);
    lv_obj_set_style_bg_color(main_btn, sw_running ? COLOR_RED : COLOR_GREEN, 0);
    lv_obj_set_style_shadow_width(main_btn, 0, 0);
    lv_obj_set_style_border_width(main_btn, 0, 0);

    lv_obj_t *main_lbl = lv_label_create(main_btn);
    lv_label_set_text(main_lbl, sw_running ? "停止" : "开始");
    lv_obj_set_style_text_font(main_lbl, &font, 0);
    lv_obj_set_style_text_color(main_lbl, lv_color_white(), 0);
    lv_obj_center(main_lbl);

    lv_obj_add_event_cb(main_btn, sw_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *reset_btn = lv_button_create(btn_container);
    lv_obj_set_size(reset_btn, btn_width, 34);
    lv_obj_set_pos(reset_btn, btn_start_x + (btn_width + btn_gap) * 2, 5);
    lv_obj_set_style_radius(reset_btn, 17, 0);
    lv_obj_set_style_bg_color(reset_btn, COLOR_SURFACE, 0);
    lv_obj_set_style_shadow_width(reset_btn, 0, 0);
    lv_obj_set_style_border_width(reset_btn, 0, 0);

    lv_obj_t *reset_lbl = lv_label_create(reset_btn);
    lv_label_set_text(reset_lbl, "重置");
    lv_obj_set_style_text_font(reset_lbl, &font, 0);
    lv_obj_set_style_text_color(reset_lbl, COLOR_ACCENT, 0);
    lv_obj_center(reset_lbl);

    lv_obj_add_event_cb(reset_btn, sw_reset_cb, LV_EVENT_CLICKED, NULL);

    lap_list = lv_obj_create(content_area);
    lv_obj_set_size(lap_list, UI_SCREEN_WIDTH, ui_get_content_height() - 125);
    lv_obj_set_pos(lap_list, 0, 125);
    lv_obj_set_style_bg_opa(lap_list, 0, 0);
    lv_obj_set_style_border_width(lap_list, 0, 0);
    lv_obj_set_style_pad_all(lap_list, 0, 0);
    lv_obj_set_scrollbar_mode(lap_list, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_scroll_dir(lap_list, LV_DIR_VER);

    render_lap_list();

    if (!sw_timer)
        sw_timer = lv_timer_create(sw_timer_cb,10,NULL);
    else
        lv_timer_resume(sw_timer);
}

// =========================
// 导航切换
// =========================
static void nav_event_cb(lv_event_t * e)
{
    int index =
        (int)(uintptr_t)lv_event_get_user_data(e);

    if(index == current_tab) return;

    current_tab = index;

    for(int i = 0; i < 2; i++) {
        if(tab_buttons[i]) {
            lv_obj_set_style_bg_color(tab_buttons[i], i == index ? lv_color_white() : COLOR_SURFACE, 0);
            lv_obj_t * label = lv_obj_get_child(tab_buttons[i], 0);
            if(label) {
                lv_obj_set_style_text_color(label, i == index ? COLOR_ACCENT : COLOR_TEXT_SECOND, 0);
            }
        }
    }

    if(index == 0) {
        render_alarm_view();
        if(titlebar) lv_obj_del(titlebar);
        titlebar = ui_titlebar_create(
            clock_page,
            "时钟",
            (lv_event_cb_t)ui_clock_hide,
            NULL,
            "+",
            show_create_page_cb,
            NULL,
            NULL,
            NULL,
            false);
    } else {
        render_stopwatch_view();
        if(titlebar) lv_obj_del(titlebar);
        titlebar = ui_titlebar_create(
            clock_page,
            "时钟",
            (lv_event_cb_t)ui_clock_hide,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            false);
    }
}

// =========================
// 初始化数据
// =========================
static void init_alarms(void)
{
    alarms[0].hour = 7;
    alarms[0].minute = 30;
    alarms[0].active = true;
    strcpy(alarms[0].label, "工作日");

    alarms[1].hour = 9;
    alarms[1].minute = 0;
    alarms[1].active = false;
    strcpy(alarms[1].label, "周末");

    alarms[2].hour = 12;
    alarms[2].minute = 0;
    alarms[2].active = true;
    strcpy(alarms[2].label, "午休");

    alarm_count = 3;
}

// =========================
// 显示页面
// =========================
void ui_clock_show(void)
{
    clock_page = lv_obj_create(lv_layer_top());

    lv_obj_set_size(clock_page,
                    UI_SCREEN_WIDTH,
                    UI_SCREEN_HEIGHT);

    lv_obj_set_style_bg_color(clock_page,COLOR_BG, 0);

    lv_obj_set_style_pad_all(clock_page, 0, 0);

    lv_obj_set_style_border_width(clock_page,0, 0);

    lv_obj_set_scrollbar_mode(clock_page, LV_SCROLLBAR_MODE_OFF);

    lv_obj_remove_flag(clock_page,LV_OBJ_FLAG_SCROLLABLE);

    if (status_bar)
        lv_obj_move_foreground(status_bar);

    // 标题栏
    titlebar = ui_titlebar_create(
        clock_page,
        "时钟",
        (lv_event_cb_t)ui_clock_hide,
        NULL,
        "+",
        show_create_page_cb,
        NULL,
        NULL,
        NULL,
        false);

    // 内容区域
    content_area = lv_obj_create(clock_page);
    lv_obj_set_size(content_area,UI_SCREEN_WIDTH, ui_get_content_height());
    lv_obj_set_pos(content_area, 0,ui_get_content_y());
    lv_obj_set_style_bg_opa(content_area,0, 0);
    lv_obj_set_style_border_width(content_area, 0,0);
    lv_obj_set_scrollbar_mode(content_area, LV_SCROLLBAR_MODE_OFF);

    // =========================
    // 底部导航按钮（Flex 水平居中，底部对齐）
    // =========================
    int btn_width = 80;
    int btn_height = 32;
    int btn_gap = 5;

    lv_obj_t * footer = lv_obj_create(clock_page);
    // 宽度交给内容自适应，由 Flex 把两个按钮居中排布
    lv_obj_set_size(footer, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_set_style_bg_opa(footer, 0, 0);
    lv_obj_set_style_border_width(footer, 0, 0);
    lv_obj_set_style_pad_all(footer, 0, 0);
    lv_obj_set_scrollbar_mode(footer, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(footer, LV_OBJ_FLAG_SCROLLABLE);

    // Flex 行布局：主轴/交叉轴居中，按钮间距固定
    lv_obj_set_flex_flow(footer, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(footer, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(footer, btn_gap, 0);

    const char * btns[] = {"闹钟","秒表"};

    for(int i = 0; i < 2; i++) {
        lv_obj_t * b = lv_button_create(footer);
        lv_obj_set_size(b, btn_width, btn_height);
        lv_obj_set_style_radius(b, 10, 0);
        lv_obj_set_style_bg_color(b, i == 0 ? lv_color_white() : COLOR_SURFACE, 0);
        lv_obj_set_style_shadow_width(b, 0, 0);
        lv_obj_set_style_border_width(b, 0, 0);
        lv_obj_set_style_pad_all(b, 0, 0);

        lv_obj_t * l = lv_label_create(b);
        lv_label_set_text(l, btns[i]);
        lv_obj_set_style_text_font(l, &font, 0);
        lv_obj_set_style_text_color(l, i == 0 ? COLOR_ACCENT : COLOR_TEXT_SECOND, 0);
        lv_obj_center(l);
        tab_buttons[i] = b;
        lv_obj_add_event_cb( b,nav_event_cb, LV_EVENT_CLICKED,(void *)(uintptr_t)i);
    }

    // 淡入动画
    lv_obj_fade_in(clock_page, 180, 0);
    init_alarms();
    render_alarm_view();
}

// =========================
// 隐藏页面
// =========================
void ui_clock_hide(void)
{
    if (sw_timer)
    {
        lv_timer_del(sw_timer);
        sw_timer = NULL;
    }

    lv_obj_del(clock_page);
}
