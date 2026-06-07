#include "ui_calendar.h"
#include "../../../components/ui_titlebar.h"
#include "../../../ui.h"
#include <time.h>
#include <string.h>

// ==================== 🎨 iOS 极简风格配色 ====================
#define COLOR_PAGE_BG         lv_color_hex(0xF5F5F7)  // 页面背景：清爽浅灰
#define COLOR_CALENDAR_BG     lv_color_white()        // 日历背景：纯白

#define COLOR_TODAY_BG        lv_color_hex(0x007AFF)  // 今天：iOS蓝
#define COLOR_TODAY_TEXT      lv_color_white()        // 今天文字：白色

#define COLOR_TEXT_MAIN       lv_color_hex(0x1C1C1E)  // 当月日期：深黑
#define COLOR_TEXT_LUNAR      lv_color_hex(0x8E8E93)  // 农历文字：中灰
#define COLOR_TEXT_WEEKDAY    lv_color_hex(0x8E8E93)  // 星期标题：中灰
#define COLOR_TEXT_INACTIVE   lv_color_hex(0xC7C7CC)  // 非当月日期：极淡灰

#define COLOR_HEADER_BG       lv_color_white()        // 月份头部背景
#define COLOR_HEADER_TEXT     lv_color_hex(0x1C1C1E)  // 月份头部文字
#define COLOR_ARROW_COLOR     lv_color_hex(0x007AFF)  // 箭头颜色

#define UI_RADIUS             12                       // 圆角半径

static const char * day_names_cn[] = {"一", "二", "三", "四", "五", "六", "日"};
static const char * month_names_cn[] = {
    "一月", "二月", "三月", "四月", "五月", "六月",
    "七月", "八月", "九月", "十月", "十一月", "十二月"
};

static lv_obj_t * calendar_page;
static lv_obj_t * calendar;
static lv_obj_t * calendar_header;
static lv_obj_t * month_label;

static void get_today_date(lv_calendar_date_t * today) {
    time_t now = time(NULL);
    struct tm * t = localtime(&now);
    today->year = t->tm_year + 1900;
    today->month = t->tm_mon + 1;
    today->day = t->tm_mday;
}

// 获取指定日期是星期几（0=周一，6=周日）
static uint8_t get_day_of_week(uint32_t year, uint32_t month, uint32_t day) {
    if(month < 3) {
        month += 12;
        year--;
    }
    uint32_t century = year / 100;
    uint32_t year_of_century = year % 100;
    uint32_t w = (day + (13 * (month + 1)) / 5 + year_of_century + year_of_century / 4 + century / 4 - 2 * century) % 7;
    return (w + 6) % 7;  // 转换为0=周一的格式
}

// 月份切换回调
static void month_prev_click_cb(lv_event_t * e) {
    lv_obj_t * btn = lv_event_get_target(e);
    lv_obj_t * header = lv_obj_get_parent(btn);
    lv_obj_t * calendar_container = lv_obj_get_parent(header);

    // 找到日历对象（第二个子对象，第一个是header_container）
    uint32_t child_count = lv_obj_get_child_cnt(calendar_container);
    lv_obj_t * cal = NULL;
    for(uint32_t i = 0; i < child_count; i++) {
        lv_obj_t * child = lv_obj_get_child(calendar_container, i);
        if(lv_obj_check_type(child, &lv_calendar_class)) {
            cal = child;
            break;
        }
    }

    if(cal) {
        const lv_calendar_date_t * date = lv_calendar_get_showed_date(cal);
        uint32_t month = date->month - 1;
        uint32_t year = date->year;

        if(month == 0) {
            month = 12;
            year--;
        }

        lv_calendar_set_month_shown(cal, year, month);

        // 更新月份标签
        char month_text[32];
        snprintf(month_text, sizeof(month_text), "%d年 %s", year, month_names_cn[month - 1]);
        lv_label_set_text(month_label, month_text);
    }
}

static void month_next_click_cb(lv_event_t * e) {
    lv_obj_t * btn = lv_event_get_target(e);
    lv_obj_t * header = lv_obj_get_parent(btn);
    lv_obj_t * calendar_container = lv_obj_get_parent(header);

    // 找到日历对象
    uint32_t child_count = lv_obj_get_child_cnt(calendar_container);
    lv_obj_t * cal = NULL;
    for(uint32_t i = 0; i < child_count; i++) {
        lv_obj_t * child = lv_obj_get_child(calendar_container, i);
        if(lv_obj_check_type(child, &lv_calendar_class)) {
            cal = child;
            break;
        }
    }

    if(cal) {
        const lv_calendar_date_t * date = lv_calendar_get_showed_date(cal);
        uint32_t month = date->month + 1;
        uint32_t year = date->year;

        if(month == 13) {
            month = 1;
            year++;
        }

        lv_calendar_set_month_shown(cal, year, month);

        // 更新月份标签
        char month_text[32];
        snprintf(month_text, sizeof(month_text), "%d年 %s", year, month_names_cn[month - 1]);
        lv_label_set_text(month_label, month_text);
    }
}

void ui_calendar_show(void) {
    // 创建全屏浮层页面
    calendar_page = lv_obj_create(lv_layer_top());
    lv_obj_set_size(calendar_page, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(calendar_page, COLOR_PAGE_BG, 0);
    lv_obj_set_style_pad_all(calendar_page, 0, 0);
    lv_obj_set_style_border_width(calendar_page, 0, 0);

    // 隐藏状态栏（全屏模式）
    if(status_bar) lv_obj_add_flag(status_bar, LV_OBJ_FLAG_HIDDEN);

    // 创建标题栏
    lv_obj_t * title_bar = ui_titlebar_create(
        calendar_page,
        "日历",
        (lv_event_cb_t)ui_calendar_hide,
        NULL,
        NULL, NULL, NULL,
        NULL, NULL,
        true  // transparent_bg
    );

    int32_t title_h = lv_obj_get_height(title_bar);
    if(title_h <= 0) title_h = UI_TITLEBAR_HEIGHT;

    // 计算内容区域
    int32_t content_y = title_h;
    int32_t content_h = UI_SCREEN_HEIGHT - title_h;
    int32_t content_w = UI_SCREEN_WIDTH;

    // 创建日历容器（无圆角无阴影，最大化空间利用）
    lv_obj_t * calendar_container = lv_obj_create(calendar_page);
    lv_obj_set_size(calendar_container, content_w, content_h);
    lv_obj_set_pos(calendar_container, 0, content_y);
    lv_obj_set_style_radius(calendar_container, 0, 0);
    lv_obj_set_style_bg_color(calendar_container, COLOR_CALENDAR_BG, 0);
    lv_obj_set_style_border_width(calendar_container, 0, 0);
    lv_obj_set_style_shadow_width(calendar_container, 0, 0);
    lv_obj_set_style_pad_all(calendar_container, 0, 0);
    lv_obj_set_flex_flow(calendar_container, LV_FLEX_FLOW_COLUMN);

    // 创建自定义月份显示容器
    lv_obj_t * header_container = lv_obj_create(calendar_container);
    lv_obj_set_width(header_container, lv_pct(100));
    lv_obj_set_height(header_container, 36);
    lv_obj_set_style_bg_color(header_container, COLOR_CALENDAR_BG, 0);
    lv_obj_set_style_border_width(header_container, 0, 0);
    lv_obj_set_style_pad_all(header_container, 0, 0);
    lv_obj_set_flex_flow(header_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 左箭头按钮
    lv_obj_t * btn_prev = lv_button_create(header_container);
    lv_obj_set_size(btn_prev, 32, 32);
    lv_obj_t * label_prev = lv_label_create(btn_prev);
    lv_label_set_text(label_prev, LV_SYMBOL_LEFT);
    lv_obj_center(label_prev);
    lv_obj_set_style_text_color(btn_prev, COLOR_TODAY_BG, 0);
    lv_obj_add_event_cb(btn_prev, month_prev_click_cb, LV_EVENT_CLICKED, NULL);

    // 月份显示标签
    month_label = lv_label_create(header_container);
    char month_text[32];
    lv_calendar_date_t today;
    get_today_date(&today);
    snprintf(month_text, sizeof(month_text), "%d年 %s",
             today.year, month_names_cn[today.month - 1]);
    lv_label_set_text(month_label, month_text);
    lv_obj_set_style_text_font(month_label, &font, 0);
    lv_obj_set_style_text_color(month_label, COLOR_HEADER_TEXT, 0);
    lv_obj_set_flex_grow(month_label, 1);

    // 右箭头按钮
    lv_obj_t * btn_next = lv_button_create(header_container);
    lv_obj_set_size(btn_next, 32, 32);
    lv_obj_t * label_next = lv_label_create(btn_next);
    lv_label_set_text(label_next, LV_SYMBOL_RIGHT);
    lv_obj_center(label_next);
    lv_obj_set_style_text_color(btn_next, COLOR_TODAY_BG, 0);
    lv_obj_add_event_cb(btn_next, month_next_click_cb, LV_EVENT_CLICKED, NULL);

    // 创建日历主体
    calendar = lv_calendar_create(calendar_container);
    lv_obj_set_width(calendar, lv_pct(100));
    lv_obj_set_flex_grow(calendar, 1);
    lv_obj_set_style_bg_opa(calendar, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(calendar, 0, 0);
    lv_obj_set_style_pad_all(calendar, 6, 0);

    // 暂时禁用中文农历模式（因为320x240屏幕空间有限，农历会导致重叠）
#if LV_USE_CALENDAR_CHINESE && 0  // 禁用农历
    lv_calendar_set_chinese_mode(calendar, true);
#endif

    // 设置中文字体（支持CJK + 数字）
    lv_obj_set_style_text_font(calendar, &lv_font_source_han_sans_sc_14_cjk, LV_PART_MAIN);

    // 设置中文星期名称
    lv_calendar_set_day_names(calendar, day_names_cn);

    // 获取并设置今天日期
    lv_calendar_set_today_date(calendar, today.year, today.month, today.day);
    lv_calendar_set_month_shown(calendar, today.year, today.month);

    // 高亮今天
    static lv_calendar_date_t highlighted[1];
    highlighted[0] = today;
    lv_calendar_set_highlighted_dates(calendar, highlighted, 1);

    // 获取内部按钮矩阵对象并设置样式
    lv_obj_t * btnm = (lv_obj_t *)lv_calendar_get_btnmatrix(calendar);

    // 按钮矩阵容器样式
    lv_obj_set_style_border_width(btnm, 0, 0);
    lv_obj_set_style_bg_opa(btnm, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(btnm, 1, 0);
    lv_obj_set_style_pad_column(btnm, 1, 0);  // 列间距
    lv_obj_set_style_pad_row(btnm, 1, 0);     // 行间距

    // 日期格子样式（默认状态）
    lv_obj_set_style_radius(btnm, 4, LV_PART_ITEMS);
    lv_obj_set_style_border_width(btnm, 0, LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(btnm, LV_OPA_TRANSP, LV_PART_ITEMS);
    lv_obj_set_style_text_color(btnm, COLOR_TEXT_MAIN, LV_PART_ITEMS);
    lv_obj_set_style_text_font(btnm, &lv_font_source_han_sans_sc_14_cjk, LV_PART_ITEMS);

    // 星期标题行样式
    lv_obj_set_style_text_color(btnm, COLOR_TEXT_WEEKDAY, LV_PART_ITEMS | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(btnm, LV_OPA_TRANSP, LV_PART_ITEMS | LV_STATE_DISABLED);

    // 非当月日期样式（使用CHECKED状态）
    lv_obj_set_style_text_color(btnm, COLOR_TEXT_INACTIVE, LV_PART_ITEMS | LV_STATE_CHECKED);

    // 今天的样式（使用PRESSED状态）
    lv_obj_set_style_bg_opa(btnm, LV_OPA_COVER, LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(btnm, COLOR_TODAY_BG, LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_set_style_text_color(btnm, COLOR_TODAY_TEXT, LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_set_style_radius(btnm, 4, LV_PART_ITEMS | LV_STATE_PRESSED);

    // 为今天的日期添加背景颜色（处理LV_CALENDAR_CTRL_TODAY状态）
    // 计算今天在日历中的按钮索引：7个星期名称 + 之前的天数 + 当前日期
    uint8_t first_day_of_week = get_day_of_week(today.year, today.month, 1);
    uint32_t today_btn_id = 7 + first_day_of_week + today.day - 1;

    // 为今天的按钮设置样式
    lv_obj_set_style_bg_opa(btnm, LV_OPA_COVER, LV_PART_ITEMS);
    // 创建样式以覆盖特定按钮
    static lv_style_t today_style;
    lv_style_init(&today_style);
    lv_style_set_bg_opa(&today_style, LV_OPA_COVER);
    lv_style_set_bg_color(&today_style, COLOR_TODAY_BG);
    lv_style_set_text_color(&today_style, COLOR_TODAY_TEXT);
    lv_style_set_radius(&today_style, 4);

    // 注意：LVGL 9.5不支持为单个按钮应用样式，所以使用FOCUSED状态作为替代
    lv_obj_set_style_bg_opa(btnm, LV_OPA_COVER, LV_PART_ITEMS | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_color(btnm, COLOR_TODAY_BG, LV_PART_ITEMS | LV_STATE_FOCUSED);
    lv_obj_set_style_text_color(btnm, COLOR_TODAY_TEXT, LV_PART_ITEMS | LV_STATE_FOCUSED);
}

void ui_calendar_hide(void) {
    // 恢复状态栏显示（9.5规范：使用remove_flag）
    if(status_bar) lv_obj_remove_flag(status_bar, LV_OBJ_FLAG_HIDDEN);

    if(lv_obj_is_valid(calendar_page)) {
        lv_obj_del(calendar_page);
        calendar_page = NULL;
        calendar = NULL;
        calendar_header = NULL;
    }
}
