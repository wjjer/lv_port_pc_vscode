#include "ui_calendar.h"
#include "../../../components/ui_titlebar.h"
#include "../../../ui.h"
#include <time.h>
#include <string.h>



#ifndef LV_CALENDAR_CTRL_TODAY
#define LV_CALENDAR_CTRL_TODAY      LV_BUTTONMATRIX_CTRL_CUSTOM_1
#endif
#ifndef LV_CALENDAR_CTRL_HIGHLIGHT
#define LV_CALENDAR_CTRL_HIGHLIGHT  LV_BUTTONMATRIX_CTRL_CUSTOM_2
#endif

// ==================== 🎨 适配白底黑字的高级扁平配色 ====================
#define COLOR_PAGE_BG         lv_color_hex(0xFFFFFF)  // 页面纯白背景
#define COLOR_TITLE_BG        lv_color_hex(0xFFFFFF)  // 标题栏纯白
#define COLOR_TITLE_TEXT      lv_color_hex(0x1C1C1E)  // 标题栏黑字

#define COLOR_TODAY_BG        lv_color_hex(0x007AFF)  // 今天：iOS科技蓝
#define COLOR_TODAY_TEXT      lv_color_white()        // 今天文字：白
#define COLOR_HIGHLIGHT_BG    lv_color_hex(0xF2F2F7)  // 节日格子背景：极浅灰

#define COLOR_TEXT_MAIN       lv_color_hex(0x1C1C1E)  // 当月公历数字：纯黑
#define COLOR_TEXT_LUNAR      lv_color_hex(0x8E8E93)  // ✨ 农历改用中灰色，通过颜色拉开视觉字号差
#define COLOR_TEXT_DIM        lv_color_hex(0x8E8E93)  // 星期行（一二三）：中灰
#define COLOR_TEXT_MUTE       lv_color_hex(0xE5E5EA)  // 非当月公历数字：极淡灰

static const char * day_names_cn[] = {"一", "二", "三", "四", "五", "六", "日"};
static lv_obj_t * calendar_page;
static lv_obj_t * calendar;

static void get_today_date(lv_calendar_date_t * today) {
    time_t now = time(NULL);
    struct tm * t = localtime(&now);
    today->year = t->tm_year + 1900;
    today->month = t->tm_mon + 1;
    today->day = t->tm_mday;
}

// ==================== 🛠️ 核心重绘回调 ====================


void ui_calendar_show(void) {
    calendar_page = lv_obj_create(lv_layer_top());
    lv_obj_set_size(calendar_page, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(calendar_page, COLOR_PAGE_BG, 0);
    lv_obj_set_style_pad_all(calendar_page, 0, 0);
    lv_obj_set_style_border_width(calendar_page, 0, 0);

    if(status_bar) lv_obj_add_flag(status_bar, LV_OBJ_FLAG_HIDDEN);

    // 创建纯白背景、黑字标题栏
    lv_obj_t * title_bar = ui_titlebar_create(calendar_page, "日历", (lv_event_cb_t)ui_calendar_hide, NULL, NULL, NULL, NULL, NULL, NULL, true);

    int32_t title_h = lv_obj_get_height(title_bar);
    if(title_h <= 0) title_h = 36;

    // 计算下方日历区域
    int32_t cal_w = UI_SCREEN_WIDTH;
    int32_t cal_h = UI_SCREEN_HEIGHT - title_h;

    lv_obj_t * calendar_bg = lv_obj_create(calendar_page);
    lv_obj_set_size(calendar_bg, cal_w, cal_h);
    lv_obj_set_pos(calendar_bg, 0, title_h);
    lv_obj_set_style_radius(calendar_bg, 0, 0);
    lv_obj_set_style_bg_color(calendar_bg, COLOR_PAGE_BG, 0);
    lv_obj_set_style_border_width(calendar_bg, 0, 0);
    lv_obj_set_style_shadow_width(calendar_bg, 0, 0);
    lv_obj_set_style_pad_all(calendar_bg, 0, 0);

    // 创建日历实体
    lv_obj_t * cal_obj = lv_calendar_create(calendar_bg);
    lv_obj_set_size(cal_obj, cal_w - 4, cal_h - 2);
    lv_obj_center(cal_obj);
    lv_obj_set_style_bg_opa(cal_obj, 0, 0);
    lv_obj_set_style_border_side(cal_obj, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_pad_all(cal_obj, 2, 0);

#if LV_USE_CALENDAR_CHINESE
    lv_calendar_set_chinese_mode(cal_obj, true);
#endif

    lv_obj_set_style_text_font(cal_obj, &lv_font_source_han_sans_sc_14_cjk, LV_PART_MAIN);

    calendar = cal_obj;
    lv_calendar_set_day_names(calendar, day_names_cn);

    lv_calendar_date_t today;
    get_today_date(&today);
    lv_calendar_set_today_date(calendar, today.year, today.month, today.day);
    lv_calendar_set_month_shown(calendar, today.year, today.month);

    static lv_calendar_date_t highlighted[1];
    highlighted[0] = today;
    lv_calendar_set_highlighted_dates(calendar, highlighted, 1);


    lv_obj_t * btnm = (lv_obj_t *)lv_calendar_get_btnmatrix(calendar);
    lv_obj_set_style_border_width(btnm, 0, 0);
    lv_obj_set_style_bg_opa(btnm, 0, 0);
    lv_obj_set_style_pad_all(btnm, 0, 0);

    lv_obj_set_style_pad_column(btnm, 1, 0);
    lv_obj_set_style_pad_row(btnm, 10, 0); 

    lv_obj_set_style_radius(btnm, 4, LV_PART_ITEMS);
    lv_obj_set_style_border_width(btnm, 0, LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(btnm, 0, LV_PART_ITEMS);


}

void ui_calendar_hide(void) {
    if(status_bar) lv_obj_clear_flag(status_bar, LV_OBJ_FLAG_HIDDEN);
    if(lv_obj_is_valid(calendar_page)) {
        lv_obj_del(calendar_page);
        calendar_page = NULL;
        calendar = NULL;
    }
}
