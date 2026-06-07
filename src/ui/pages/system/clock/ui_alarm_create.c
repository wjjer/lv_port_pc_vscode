#include "ui_alarm_create.h"
#include "../../../components/ui_titlebar.h"
#include "../../../ui.h"
#include <string.h>

// =========================
// 极简 iOS 风格主题
// =========================
#define COLOR_BG lv_color_hex(0xF5F5F7)
#define COLOR_SURFACE lv_color_hex(0xFAFAFC)
#define COLOR_TEXT_MAIN lv_color_hex(0x111111)
#define COLOR_TEXT_SECOND lv_color_hex(0x8E8E93)
#define COLOR_ACCENT lv_color_hex(0x007AFF)
#define COLOR_GREEN lv_color_hex(0x34C759)

// =========================
// 静态变量
// =========================
static lv_obj_t *create_page = NULL;
static lv_obj_t *hour_label = NULL;
static lv_obj_t *minute_label = NULL;

static uint8_t new_hour = 8;
static uint8_t new_minute = 0;

static bool weekdays[7] = {false};

static char new_label[64] = "";
static char new_ringtone[64] = "默认铃声";

static alarm_data_t saved_data;
static bool has_new_data = false;

static lv_obj_t *weekday_btns[7];
static lv_obj_t *ringtone_btn = NULL;
static lv_obj_t *ringtone_label = NULL;
static lv_obj_t *ringtone_page = NULL;

static alarm_saved_callback_t saved_callback = NULL;

static const char *ringtone_list[] = {
    "默认铃声",
    "清晨鸟鸣",
    "海浪声",
    "雨滴声",
    "风铃声",
    "电子音",
    "钢琴曲",
    NULL};


static void textarea_focus_cb(lv_event_t *e);
static void kb_event_cb(lv_event_t *e);

// =========================
// 更新时间显示
// =========================
static void update_time_display(void)
{
    if (hour_label)
        lv_label_set_text_fmt(hour_label, "%02d", new_hour);

    if (minute_label)
        lv_label_set_text_fmt(minute_label, "%02d", new_minute);
}

// =========================
// 时间调整
// =========================
static void time_up_cb(lv_event_t *e)
{
    uintptr_t which = (uintptr_t)lv_event_get_user_data(e);

    if (which == 0)
    {
        new_hour = (new_hour >= 23) ? 0 : new_hour + 1;
    }
    else
    {
        new_minute = (new_minute >= 59) ? 0 : new_minute + 1;
    }

    update_time_display();
}

static void time_down_cb(lv_event_t *e)
{
    uintptr_t which = (uintptr_t)lv_event_get_user_data(e);

    if (which == 0)
    {
        new_hour = (new_hour == 0) ? 23 : new_hour - 1;
    }
    else
    {
        new_minute = (new_minute == 0) ? 59 : new_minute - 1;
    }

    update_time_display();
}

// =========================
// 星期按钮
// =========================
static void weekday_btn_cb(lv_event_t *e)
{
    int day = (int)(uintptr_t)lv_event_get_user_data(e);

    weekdays[day] = !weekdays[day];

    lv_obj_t *btn = weekday_btns[day];
    lv_obj_t *lbl = lv_obj_get_child(btn, 0);

    if (weekdays[day])
    {

        lv_obj_set_style_bg_color(
            btn,
            COLOR_ACCENT,
            0);

        lv_obj_set_style_bg_opa(
            btn,
            LV_OPA_COVER,
            0);

        if (lbl)
            lv_obj_set_style_text_color(
                lbl,
                lv_color_white(),
                0);
    }
    else
    {

        lv_obj_set_style_bg_opa(
            btn,
            0,
            0);

        if (lbl)
            lv_obj_set_style_text_color(
                lbl,
                COLOR_TEXT_SECOND,
                0);
    }
}

// =========================
// 输入框回调
// =========================
static void label_input_cb(lv_event_t *e)
{
    lv_obj_t *ta = lv_event_get_target(e);

    const char *txt =
        lv_textarea_get_text(ta);

    strncpy(new_label,
            txt,
            sizeof(new_label) - 1);

    new_label[sizeof(new_label) - 1] = '\0';
}

// =========================
// 铃声选择回调
// =========================
static void ringtone_select_cb(lv_event_t *e)
{
    int index = (int)(uintptr_t)lv_event_get_user_data(e);

    if (ringtone_list[index] != NULL)
    {
        strncpy(new_ringtone, ringtone_list[index], sizeof(new_ringtone) - 1);
        new_ringtone[sizeof(new_ringtone) - 1] = '\0';

        if (ringtone_label)
        {
            lv_label_set_text(ringtone_label, new_ringtone);
        }
    }

    ui_ringtone_page_hide();
}

// =========================
// 显示铃声选择页面
// =========================
static void show_ringtone_page(lv_event_t *e)
{
    LV_UNUSED(e);

    if (ringtone_page != NULL)
        lv_obj_del(ringtone_page);

    ringtone_page = lv_obj_create(lv_layer_top());
    lv_obj_set_size(ringtone_page, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(ringtone_page, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(ringtone_page, LV_OPA_50, 0);
    lv_obj_set_style_pad_all(ringtone_page, 0, 0);
    lv_obj_set_scrollbar_mode(ringtone_page, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(ringtone_page, LV_OBJ_FLAG_SCROLLABLE);

    if (status_bar)
        lv_obj_move_foreground(status_bar);

    int max_height = UI_SCREEN_HEIGHT - 100;
    int content_height = max_height;

    lv_obj_t *content = lv_obj_create(ringtone_page);
    lv_obj_set_size(content, UI_SCREEN_WIDTH - 64, content_height);
    lv_obj_center(content);
    lv_obj_set_style_radius(content, 16, 0);
    lv_obj_set_style_bg_color(content, lv_color_white(), 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_shadow_width(content, 0, 0);
    lv_obj_set_style_pad_all(content, 0, 0);
    lv_obj_set_scrollbar_mode(content, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_scroll_dir(content, LV_DIR_VER);

    lv_obj_t *title = lv_label_create(content);
    lv_label_set_text(title, "选择铃声");
    lv_obj_set_style_text_font(title, &font, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x111111), 0);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(title, lv_pct(100));
    lv_obj_set_pos(title, 0, 20);

    lv_obj_t *cancel_btn = lv_button_create(content);
    lv_obj_set_size(cancel_btn, 60, 30);
    lv_obj_set_pos(cancel_btn, 16, 16);
    lv_obj_set_style_bg_opa(cancel_btn, 0, 0);
    lv_obj_set_style_border_width(cancel_btn, 0, 0);
    lv_obj_set_style_shadow_width(cancel_btn, 0, 0);

    lv_obj_t *cancel_label = lv_label_create(cancel_btn);
    lv_label_set_text(cancel_label, "取消");
    lv_obj_set_style_text_font(cancel_label, &font, 0);
    lv_obj_set_style_text_color(cancel_label, lv_color_hex(0x007AFF), 0);
    lv_obj_center(cancel_label);

    lv_obj_add_event_cb(cancel_btn, (lv_event_cb_t)ui_ringtone_page_hide, LV_EVENT_CLICKED, NULL);

    lv_obj_t *list_container = lv_obj_create(content);
    lv_obj_set_size(list_container, lv_pct(100), content_height - 60);
    lv_obj_set_pos(list_container, 0, 60);
    lv_obj_set_style_bg_opa(list_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(list_container, 0, 0);
    lv_obj_set_style_pad_all(list_container, 0, 0);
    lv_obj_set_scrollbar_mode(list_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(list_container, LV_DIR_VER);

    int y = 0;
    for (int i = 0; ringtone_list[i] != NULL; i++)
    {
        lv_obj_t *item = lv_button_create(list_container);
        lv_obj_set_size(item, UI_SCREEN_WIDTH - 96, 44);
        lv_obj_set_pos(item, 32, y);
        lv_obj_set_style_bg_opa(item, 0, 0);
        lv_obj_set_style_border_width(item, 0, 0);
        lv_obj_set_style_shadow_width(item, 0, 0);
        lv_obj_set_style_radius(item, 0, 0);

        lv_obj_t *label = lv_label_create(item);
        lv_label_set_text(label, ringtone_list[i]);
        lv_obj_set_style_text_font(label, &font, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0x111111), 0);
        lv_obj_set_pos(label, 0, 12);

        if (strcmp(new_ringtone, ringtone_list[i]) == 0)
        {
            lv_obj_t *check = lv_label_create(item);
            lv_label_set_text(check, LV_SYMBOL_OK);
            lv_obj_set_style_text_color(check, lv_color_hex(0x34C759), 0);
            lv_obj_align(check, LV_ALIGN_RIGHT_MID, 0, 0);
        }

        lv_obj_add_event_cb(item, ringtone_select_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)i);

        y += 44;
    }

    lv_obj_fade_in(ringtone_page, 200, 0);
}

// =========================
// 隐藏铃声选择页面
// =========================
void ui_ringtone_page_hide(void)
{
    if (ringtone_page)
    {
        lv_obj_fade_out(ringtone_page, 150, 0);
        lv_obj_delete_delayed(ringtone_page, 180);
        ringtone_page = NULL;
    }
}

// =========================
// 保存
// =========================
static void save_alarm_cb(lv_event_t *e)
{
    LV_UNUSED(e);

    saved_data.hour = new_hour;
    saved_data.minute = new_minute;
    saved_data.active = true;

    // 简化：不使用weekdays数组，使用repeat标志
    saved_data.repeat = false;
    for (int i = 0; i < 7; i++)
    {
        if (weekdays[i])
        {
            saved_data.repeat = true;
            break;
        }
    }

    strncpy(saved_data.label,
            new_label,
            sizeof(saved_data.label) - 1);

    saved_data.label[sizeof(saved_data.label) - 1] = '\0';

    strncpy(saved_data.ringtone,
            new_ringtone,
            sizeof(saved_data.ringtone) - 1);

    saved_data.ringtone[sizeof(saved_data.ringtone) - 1] = '\0';

    has_new_data = true;

    ui_alarm_create_hide();

    // 调用回调函数通知时钟页面更新
    if (saved_callback != NULL)
        saved_callback();
}

// =========================
// 取消
// =========================
static void cancel_alarm_cb(lv_event_t *e)
{
    LV_UNUSED(e);

    ui_alarm_create_hide();
}

// =========================
// 显示页面
// =========================
void ui_alarm_create_show(void)
{
    if (create_page != NULL)
        lv_obj_del(create_page);

    // 重置数据
    new_hour = 8;
    new_minute = 0;

    memset(weekdays,
           0,
           sizeof(weekdays));

    strcpy(new_label, "");

    create_page = lv_obj_create(lv_layer_top());

    lv_obj_set_size(create_page,
                    UI_SCREEN_WIDTH,
                    UI_SCREEN_HEIGHT);

    lv_obj_set_style_bg_color(
        create_page,
        COLOR_BG,
        0);

    lv_obj_set_style_pad_all(
        create_page,
        0,
        0);

    lv_obj_set_scrollbar_mode(
        create_page,
        LV_SCROLLBAR_MODE_OFF);

    lv_obj_clear_flag(
        create_page,
        LV_OBJ_FLAG_SCROLLABLE);

    if (status_bar)
        lv_obj_move_foreground(status_bar);

    // =========================
    // 复用系统标题栏
    // =========================
    ui_titlebar_create(
        create_page,
        "新建闹钟",
        cancel_alarm_cb,
        NULL,
        "保存",
        save_alarm_cb,
        NULL,
        NULL,
        NULL,
        false);

    // =========================
    // 可滚动内容容器（标题栏固定）
    // =========================
    lv_obj_t *content = lv_obj_create(create_page);
    lv_obj_set_size(content, UI_SCREEN_WIDTH,
                    UI_SCREEN_HEIGHT - UI_STATUSBAR_HEIGHT - UI_TITLEBAR_HEIGHT);
    lv_obj_set_pos(content, 0, UI_STATUSBAR_HEIGHT + UI_TITLEBAR_HEIGHT);
    lv_obj_set_style_pad_all(content, 0, 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);
    lv_obj_set_scrollbar_mode(content, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(content, LV_DIR_VER);

    // =========================
    // 时间区域
    // =========================
    int content_y = 6;

    lv_obj_t *time_bg =
        lv_obj_create(content);

    lv_obj_set_size(time_bg,
                    UI_SCREEN_WIDTH - 32,
                    56);

    lv_obj_align(time_bg,
                 LV_ALIGN_TOP_MID,
                 0,
                 content_y);

    lv_obj_set_style_radius(
        time_bg,
        24,
        0);

    lv_obj_set_style_bg_color(
        time_bg,
        COLOR_SURFACE,
        0);

    lv_obj_set_style_border_width(
        time_bg,
        0,
        0);

    lv_obj_set_style_shadow_width(
        time_bg,
        0,
        0);

    lv_obj_set_scrollbar_mode(
        time_bg,
        LV_SCROLLBAR_MODE_OFF);

    lv_obj_clear_flag(
        time_bg,
        LV_OBJ_FLAG_SCROLLABLE);

    // 小时
    lv_obj_t *h_up =
        lv_button_create(time_bg);

    lv_obj_set_size(h_up,
                    28,
                    22);

    lv_obj_align(h_up,
                 LV_ALIGN_LEFT_MID,
                 36,
                 -16);

    lv_obj_set_style_bg_opa(
        h_up,
        0,
        0);

    lv_obj_set_style_shadow_opa(
        h_up,
        0,
        0);

    lv_obj_set_style_border_width(
        h_up,
        0,
        0);

    lv_obj_t *h_up_l =
        lv_label_create(h_up);

    lv_label_set_text(h_up_l,
                      LV_SYMBOL_UP);

    lv_obj_set_style_text_color(
        h_up_l,
        COLOR_ACCENT,
        0);

    lv_obj_center(h_up_l);

    lv_obj_add_event_cb(
        h_up,
        time_up_cb,
        LV_EVENT_CLICKED,
        (void *)0);

    hour_label =
        lv_label_create(time_bg);

    lv_label_set_text_fmt(
        hour_label,
        "%02d",
        new_hour);

    lv_obj_set_style_text_font(
        hour_label,
        &lv_font_montserrat_32,
        0);

    lv_obj_set_style_text_color(
        hour_label,
        COLOR_TEXT_MAIN,
        0);

    lv_obj_align(hour_label,
                 LV_ALIGN_LEFT_MID,
                 30,
                 6);

    lv_obj_t *h_dn =
        lv_button_create(time_bg);

    lv_obj_set_size(h_dn,
                    28,
                    22);

    lv_obj_align(h_dn,
                 LV_ALIGN_LEFT_MID,
                 36,
                 20);

    lv_obj_set_style_bg_opa(
        h_dn,
        0,
        0);

    lv_obj_set_style_shadow_opa(
        h_dn,
        0,
        0);

    lv_obj_set_style_border_width(
        h_dn,
        0,
        0);

    lv_obj_t *h_dn_l =
        lv_label_create(h_dn);

    lv_label_set_text(h_dn_l,
                      LV_SYMBOL_DOWN);

    lv_obj_set_style_text_color(
        h_dn_l,
        COLOR_ACCENT,
        0);

    lv_obj_center(h_dn_l);

    lv_obj_add_event_cb(
        h_dn,
        time_down_cb,
        LV_EVENT_CLICKED,
        (void *)0);

    // 冒号
    lv_obj_t *colon =
        lv_label_create(time_bg);

    lv_label_set_text(colon, ":");

    lv_obj_set_style_text_font(
        colon,
        &lv_font_montserrat_32,
        0);

    lv_obj_set_style_text_color(
        colon,
        COLOR_TEXT_SECOND,
        0);

    lv_obj_center(colon);

    // 分钟
    lv_obj_t *m_up =
        lv_button_create(time_bg);

    lv_obj_set_size(m_up,
                    28,
                    22);

    lv_obj_align(m_up,
                 LV_ALIGN_RIGHT_MID,
                 -36,
                 -16);

    lv_obj_set_style_bg_opa(
        m_up,
        0,
        0);

    lv_obj_set_style_shadow_opa(
        m_up,
        0,
        0);

    lv_obj_set_style_border_width(
        m_up,
        0,
        0);

    lv_obj_t *m_up_l =
        lv_label_create(m_up);

    lv_label_set_text(m_up_l,
                      LV_SYMBOL_UP);

    lv_obj_set_style_text_color(
        m_up_l,
        COLOR_ACCENT,
        0);

    lv_obj_center(m_up_l);

    lv_obj_add_event_cb(
        m_up,
        time_up_cb,
        LV_EVENT_CLICKED,
        (void *)1);

    minute_label =
        lv_label_create(time_bg);

    lv_label_set_text_fmt(
        minute_label,
        "%02d",
        new_minute);

    lv_obj_set_style_text_font(
        minute_label,
        &lv_font_montserrat_32,
        0);

    lv_obj_set_style_text_color(
        minute_label,
        COLOR_TEXT_MAIN,
        0);

    lv_obj_align(minute_label,
                 LV_ALIGN_RIGHT_MID,
                 -30,
                 6);

    lv_obj_t *m_dn =
        lv_button_create(time_bg);

    lv_obj_set_size(m_dn,
                    28,
                    22);

    lv_obj_align(m_dn,
                 LV_ALIGN_RIGHT_MID,
                 -36,
                 20);

    lv_obj_set_style_bg_opa(
        m_dn,
        0,
        0);

    lv_obj_set_style_shadow_opa(
        m_dn,
        0,
        0);

    lv_obj_set_style_border_width(
        m_dn,
        0,
        0);

    lv_obj_t *m_dn_l =
        lv_label_create(m_dn);

    lv_label_set_text(m_dn_l,
                      LV_SYMBOL_DOWN);

    lv_obj_set_style_text_color(
        m_dn_l,
        COLOR_ACCENT,
        0);

    lv_obj_center(m_dn_l);

    lv_obj_add_event_cb(
        m_dn,
        time_down_cb,
        LV_EVENT_CLICKED,
        (void *)1);

    // =========================
    // 重复星期
    // =========================
    lv_obj_t *repeat_title =
        lv_label_create(content);

    lv_label_set_text(repeat_title,
                      "重复");

    lv_obj_set_style_text_font(
        repeat_title,
        &font,
        0);

    lv_obj_set_style_text_color(
        repeat_title,
        COLOR_TEXT_SECOND,
        0);

    lv_obj_align(repeat_title,
                 LV_ALIGN_TOP_LEFT,
                 16,
                 content_y + 66);

    lv_obj_t *repeat_row = lv_obj_create(content);
    lv_obj_set_size(repeat_row, UI_SCREEN_WIDTH - 32, 34);
    lv_obj_align(repeat_row, LV_ALIGN_TOP_MID, 0, content_y + 88);
    lv_obj_set_style_bg_opa(repeat_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(repeat_row, 0, 0);
    lv_obj_set_style_pad_all(repeat_row, 0, 0);
    lv_obj_set_scrollbar_mode(repeat_row, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(repeat_row, LV_DIR_HOR);

    const char *days[] = {
        "一", "二", "三", "四", "五", "六", "日"};

    for (int i = 0; i < 7; i++)
    {

        weekday_btns[i] =
            lv_button_create(repeat_row);

        lv_obj_set_size(
            weekday_btns[i],
            28,
            28);

        lv_obj_set_style_radius(
            weekday_btns[i],
            14,
            0);

        lv_obj_set_style_bg_opa(
            weekday_btns[i],
            0,
            0);

        lv_obj_set_style_border_width(
            weekday_btns[i],
            0,
            0);

        lv_obj_set_style_shadow_width(
            weekday_btns[i],
            0,
            0);

        lv_obj_set_pos(
            weekday_btns[i],
            10 + i * 36,
            3);

        lv_obj_t *lbl =
            lv_label_create(weekday_btns[i]);

        lv_label_set_text(lbl,
                          days[i]);

        lv_obj_set_style_text_font(
            lbl,
            &font,
            0);

        lv_obj_set_style_text_color(
            lbl,
            COLOR_TEXT_SECOND,
            0);

        lv_obj_center(lbl);

        lv_obj_add_event_cb(
            weekday_btns[i],
            weekday_btn_cb,
            LV_EVENT_CLICKED,
            (void *)(uintptr_t)i);
    }

        // =========================
    // 标签输入
    // =========================
    lv_obj_t *label_title =
        lv_label_create(content);

    lv_label_set_text(label_title,
                      "标签");

    lv_obj_set_style_text_font(
        label_title,
        &font,
        0);

    lv_obj_set_style_text_color(
        label_title,
        COLOR_TEXT_SECOND,
        0);

    lv_obj_align(label_title,
                 LV_ALIGN_TOP_LEFT,
                 16,
                 content_y + 128);

    lv_obj_t *label_bg =
        lv_obj_create(content);

    lv_obj_set_size(label_bg,
                    UI_SCREEN_WIDTH - 32,
                    32);

    lv_obj_align(label_bg,
                 LV_ALIGN_TOP_MID,
                 0,
                 content_y + 150);

    lv_obj_set_style_radius(
        label_bg,
        18,
        0);

    lv_obj_set_style_bg_color(
        label_bg,
        COLOR_SURFACE,
        0);

    lv_obj_set_style_border_width(
        label_bg,
        0,
        0);

    lv_obj_set_style_shadow_width(
        label_bg,
        0,
        0);

    lv_obj_t *label_input =
        lv_textarea_create(label_bg);

    lv_obj_set_size(label_input,
                    lv_pct(100),
                    28);

    lv_obj_align(label_input,
                 LV_ALIGN_CENTER,
                 0,
                 0);

    lv_obj_set_style_bg_opa(
        label_input,
        0,
        0);

    lv_obj_set_style_border_width(
        label_input,
        0,
        0);

    lv_obj_set_style_text_font(
        label_input,
        &font,
        0);

    lv_obj_set_style_text_color(
        label_input,
        COLOR_TEXT_MAIN,
        0);

    lv_textarea_set_placeholder_text(label_input, "闹钟名称");
    lv_textarea_set_one_line(label_input, true);
    lv_textarea_set_max_length(label_input, 20);

    /* 关键：确保文本框可以获取焦点 */
    lv_obj_clear_flag(label_input, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(label_input, LV_OBJ_FLAG_CLICKABLE);  // 确保可点击

    /* 创建键盘（初始隐藏）*/
    lv_obj_t *kb = lv_keyboard_create(lv_layer_top());
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);

    /* 设置键盘位置（底部）*/
    lv_obj_set_align(kb, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_pos(kb, 0, 0);

    /* 创建拼音输入法插件 */
    lv_obj_t *pinyin_ime = lv_ime_pinyin_create(lv_layer_top());
    lv_ime_pinyin_set_keyboard(pinyin_ime, kb);

    /* 先将键盘与文本框关联 */
    lv_keyboard_set_textarea(kb, label_input);

    /* 添加焦点事件：点击文本框时显示键盘 */
    lv_obj_add_event_cb(label_input, textarea_focus_cb, LV_EVENT_ALL, kb);

    /* 添加键盘事件：点击完成按钮时隐藏键盘 */
    lv_obj_add_event_cb(kb, kb_event_cb, LV_EVENT_ALL, NULL);

    /* 文本框内容改变回调 */
    lv_obj_add_event_cb(label_input, label_input_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_textarea_set_one_line(
        label_input,
        true);

    lv_textarea_set_max_length(
        label_input,
        20);

    lv_obj_add_event_cb(
        label_input,
        label_input_cb,
        LV_EVENT_VALUE_CHANGED,
        NULL);

    // =========================
    // 铃声选择
    // =========================
    lv_obj_t *ringtone_title =
        lv_label_create(content);

    lv_label_set_text(ringtone_title,
                      "铃声");

    lv_obj_set_style_text_font(
        ringtone_title,
        &font,
        0);

    lv_obj_set_style_text_color(
        ringtone_title,
        COLOR_TEXT_SECOND,
        0);

    lv_obj_align(ringtone_title,
                 LV_ALIGN_TOP_LEFT,
                 16,
                 content_y + 188);

    ringtone_btn = lv_button_create(content);
    lv_obj_set_size(ringtone_btn, UI_SCREEN_WIDTH - 32, 44);
    lv_obj_align(ringtone_btn, LV_ALIGN_TOP_MID, 0, content_y + 210);
    lv_obj_set_style_radius(ringtone_btn, 22, 0);
    lv_obj_set_style_bg_color(ringtone_btn, COLOR_SURFACE, 0);
    lv_obj_set_style_border_width(ringtone_btn, 0, 0);
    lv_obj_set_style_shadow_width(ringtone_btn, 0, 0);
    lv_obj_set_style_pad_all(ringtone_btn, 0, 0);

    ringtone_label = lv_label_create(ringtone_btn);
    lv_label_set_text(ringtone_label, new_ringtone);
    lv_obj_set_style_text_font(ringtone_label, &font, 0);
    lv_obj_set_style_text_color(ringtone_label, COLOR_TEXT_MAIN, 0);
    lv_obj_set_pos(ringtone_label, 16, 12);

    lv_obj_t *arrow = lv_label_create(ringtone_btn);
    lv_label_set_text(arrow, LV_SYMBOL_RIGHT);
    lv_obj_set_style_text_color(arrow, COLOR_TEXT_SECOND, 0);
    lv_obj_align(arrow, LV_ALIGN_RIGHT_MID, -12, 0);

    lv_obj_add_event_cb(ringtone_btn, show_ringtone_page, LV_EVENT_CLICKED, NULL);

    // 动画
    lv_obj_fade_in(create_page, 150, 0);
}

// =========================
// 隐藏页面
// =========================
void ui_alarm_create_hide(void)
{
    if (create_page)
    {

        lv_obj_fade_out(
            create_page,
            100,
            0);

        lv_obj_delete_delayed(
            create_page,
            120);

        create_page = NULL;
    }

    hour_label = NULL;
    minute_label = NULL;
}

// =========================
// 数据接口
// =========================
bool ui_alarm_create_has_data(void)
{
    return has_new_data;
}

void ui_alarm_create_get_data(alarm_data_t *out)
{
    if (out == NULL)
        return;

    memcpy(out,
           &saved_data,
           sizeof(alarm_data_t));

    has_new_data = false;
}

void ui_alarm_create_set_saved_callback(alarm_saved_callback_t cb)
{
    saved_callback = cb;
}

// =========================
// 键盘焦点事件回调
// =========================
static void textarea_focus_cb(lv_event_t *e)
{
   lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);
    lv_obj_t *kb = lv_event_get_user_data(e);

    printf("textarea_focus_cb: code=%d\n", code);  // 调试打印

    if (code == LV_EVENT_FOCUSED) {
        /* 获得焦点：显示键盘并关联 */
        printf("FOCUSED: showing keyboard\n");
        lv_keyboard_set_textarea(kb, ta);
        lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
    else if (code == LV_EVENT_DEFOCUSED) {
        /* 失去焦点：隐藏键盘 */
        printf("DEFOCUSED: hiding keyboard\n");
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
}

// 可选：键盘事件回调（按完成按钮时隐藏键盘）
static void kb_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *kb = lv_event_get_target(e);

    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        /* 按下了键盘上的 ✓ 或 ✗ 按钮，隐藏键盘 */
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
}
