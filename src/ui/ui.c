#include "lvgl.h"
#include <stdio.h>
#include <time.h>
#include "ui.h"
#include <string.h>
#include "pages/system/settings/ui_settings.h"
#include "pages/system/clock/ui_clock.h"
#include "pages/system/calendar/ui_calendar.h"
#include "pages/system/weather/ui_weather.h"
#include "pages/system/reminder/ui_reminder.h"
#include "pages/system/calculator/ui_calculator.h"
#include "pages/games/airplane/flygame.h"
#include "pages/games/pvz/pvz.h"
#include "pages/app/poetry/ui_poetry.h"
#include "pages/app/pomodoro/ui_pomodoro.h"
// 样式宏定义
#define IOS_ICON_RADIUS 12
#define COLOR_TEXT_DIM  lv_color_hex(0x94A3B8)
#define COLOR_SURFACE   lv_color_hex(0x1E293B)
#define COLOR_BG_DARK   lv_color_hex(0x0F172A)
#define COLOR_ACCENT    lv_color_hex(0x6366F1)
// 计算图标总数
#define ICON_COUNT (sizeof(icon_names) / sizeof(icon_names[0]))
// 每页最大图标数 (3列 x 2行)
#define ICONS_PER_PAGE 6
// Dock 图标数
#define DOCK_ICON_COUNT 4

// 定义设置图标
LV_IMAGE_DECLARE(icon_settings);
LV_IMAGE_DECLARE(icon_clock);
LV_IMAGE_DECLARE(icon_ai);
LV_IMAGE_DECLARE(icon_calendar);
LV_IMAGE_DECLARE(icon_weather);
LV_IMAGE_DECLARE(icon_reminder);
LV_IMAGE_DECLARE(icon_calculator);

// Dock 图标数据
const char * dock_icon_names[] = {
    "豆包", "时钟", "天气", "日历"
};

const void * dock_icons[] = {
    &icon_ai,
    &icon_clock,
    &icon_weather,
    &icon_calendar
};

// 模拟图标数据
const char * icon_names[] = {
   "设置", "时钟", "豆包", "日历", "天气", "提醒", "计算器", "诗词园地", "飞机游戏", "pvz游戏", "番茄时钟"
};

// 这里的数组存放的是图片的地址
const void * ios_icons[] = {
    &icon_settings,
    &icon_clock,
    &icon_ai,
    &icon_calendar,
    &icon_weather,
    &icon_reminder,
    &icon_calculator,
    NULL,  // 诗词园地：图标暂不显示
    NULL,  // 飞机游戏：图标暂不显示
    NULL,  // pvz游戏：图标暂不显示
    NULL   // 番茄时钟：图标暂不显示
};

// 页面指示器圆点
static lv_obj_t * dots[3];

// 翻页时钟
typedef struct {
    lv_obj_t * h1;
    lv_obj_t * h2;
    lv_obj_t * m1;
    lv_obj_t * m2;
    lv_obj_t * s1;
    lv_obj_t * s2;
} flip_clock_t;

static flip_clock_t g_flip_clock;
static int g_last_h = -1;
static int g_last_m = -1;
static int g_last_s = -1;

// APP 启动回调
static app_launch_callback_t app_launch_cb = NULL;

void ui_set_app_launch_callback(app_launch_callback_t cb) {
    app_launch_cb = cb;
}

// 图标点击回调
static void icon_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        lv_obj_t * obj = lv_event_get_target(e);
        const char * name = lv_label_get_text(lv_obj_get_child(obj, 1));
        if(strcmp(name, "设置") == 0) {
            ui_settings_show();
        } else if(strcmp(name, "时钟") == 0) {
            ui_clock_show();
        } else if(strcmp(name, "AI") == 0) {
            // AI 助手：通过回调通知外部启动
            if(app_launch_cb) {
                app_launch_cb("ai_assistant");
            }
        } else if(strcmp(name, "日历") == 0) {
            ui_calendar_show();
        } else if(strcmp(name, "天气") == 0) {
            ui_weather_show();
        } else if(strcmp(name, "提醒") == 0) {
            ui_reminder_show();
        } else if(strcmp(name, "计算器") == 0) {
            ui_calculator_show();
        } else if(strcmp(name, "飞机游戏") == 0) {
            flygame_toggle();
        } else if(strcmp(name, "pvz游戏") == 0) {
            pvz_toggle();
        } else if(strcmp(name, "诗词园地") == 0) {
            ui_poetry_show();
        } else if(strcmp(name, "番茄时钟") == 0) {
            ui_pomodoro_show();
        }
    }
}

// Dock 图标点击回调
static void dock_icon_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        lv_obj_t * obj = lv_event_get_target(e);
        int32_t idx = (int32_t)lv_obj_get_user_data(obj);
        const char * name = dock_icon_names[idx];
        if(strcmp(name, "天气") == 0) {
            ui_weather_show();
        } else if(strcmp(name, "日历") == 0) {
            ui_calendar_show();
        } else if(strcmp(name, "AI") == 0) {
            // AI功能待实现
        } else if(strcmp(name, "时钟") == 0) {
            ui_clock_show();
        }
    }
}

/**
 * Tileview 滑动事件回调：更新指示器状态
 */
static void tileview_event_cb(lv_event_t * e) {
    lv_obj_t * tv = lv_event_get_target(e);
    lv_obj_t * active_tile = lv_tileview_get_tile_active(tv);

    // LVGL 9.x 获取坐标的标准方式
    int32_t x = lv_obj_get_x(active_tile);

    // 使用屏幕宽度代替硬编码 320
    int32_t index = x / UI_SCREEN_WIDTH;

    // 限制索引范围防止数组越界（保险措施）
    if(index < 0) index = 0;
    if(index > 2) index = 2;

    // 更新圆点状态
    for(int i = 0; i < 3; i++) {
        if(i == index) {
            lv_obj_set_style_bg_opa(dots[i], LV_OPA_COVER, 0); // 选中的点
        } else {
            lv_obj_set_style_bg_opa(dots[i], LV_OPA_40, 0);    // 未选中的点
        }
    }
}

/**
 * 创建页面指示器
 */
void create_page_indicator(void) {
    // 1. 创建指示器容器
    lv_obj_t * indicator_cont = lv_obj_create(lv_layer_top());
    lv_obj_set_size(indicator_cont, 80, 20);
    lv_obj_align(indicator_cont, LV_ALIGN_BOTTOM_MID, 0, -50); // 放在 Dock 上方
    lv_obj_set_style_bg_opa(indicator_cont, 0, 0);
    lv_obj_set_style_border_side(indicator_cont, LV_BORDER_SIDE_NONE, 0);
    lv_obj_clear_flag(indicator_cont, LV_OBJ_FLAG_SCROLLABLE);

    // 2. 使用 Flex 布局横向排列圆点
    lv_obj_set_flex_flow(indicator_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(indicator_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(indicator_cont, 8, 0);

    // 3. 创建 3 个圆点
    for(int i = 0; i < 3; i++) {
        dots[i] = lv_obj_create(indicator_cont);
        lv_obj_set_size(dots[i], 6, 6); // iOS 风格的小圆点
        lv_obj_set_style_radius(dots[i], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(dots[i], lv_color_white(), 0);
        lv_obj_set_style_border_side(dots[i], LV_BORDER_SIDE_NONE, 0);

        // 初始状态：第一页亮起
        if(i == 0) lv_obj_set_style_bg_opa(dots[i], LV_OPA_COVER, 0);
        else lv_obj_set_style_bg_opa(dots[i], LV_OPA_40, 0);
    }
}


/**
 * 定时器：更新主屏时间、日期和状态栏时间
 */
static void update_time_task(lv_timer_t * timer) {
    lv_obj_t * date_lbl = (lv_obj_t *)lv_timer_get_user_data(timer);
    time_t rawtime;
    struct tm * timeinfo;
    char buf[32];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    int hour = timeinfo->tm_hour;
    int min  = timeinfo->tm_min;
    int sec  = timeinfo->tm_sec;

    // 更新翻页时钟（每秒都更新秒，只在分钟变化时更新时分）
    if(g_flip_clock.h1 && (hour != g_last_h || min != g_last_m || sec != g_last_s)) {
        if(hour != g_last_h || min != g_last_m) {
            lv_roller_set_selected(g_flip_clock.h1, hour / 10, LV_ANIM_ON);
            lv_roller_set_selected(g_flip_clock.h2, hour % 10, LV_ANIM_ON);
            lv_roller_set_selected(g_flip_clock.m1, min / 10, LV_ANIM_ON);
            lv_roller_set_selected(g_flip_clock.m2, min % 10, LV_ANIM_ON);
            g_last_h = hour;
            g_last_m = min;
        }
        lv_roller_set_selected(g_flip_clock.s1, sec / 10, LV_ANIM_ON);
        lv_roller_set_selected(g_flip_clock.s2, sec % 10, LV_ANIM_ON);
        g_last_s = sec;
    }

    // 更新日期和星期（每分钟检查一次，减少开销）
    static int last_update_day = -1;
    if(date_lbl && timeinfo->tm_mday != last_update_day) {
        const char * week_days[] = {"星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"};
        sprintf(buf, "%d月%d日  %s", timeinfo->tm_mon + 1, timeinfo->tm_mday, week_days[timeinfo->tm_wday]);
        lv_label_set_text(date_lbl, buf);
        last_update_day = timeinfo->tm_mday;
    }

    // 更新状态栏时间
    sprintf(buf, "%02d:%02d:%02d", hour, min, sec);
    if(bar_time_label) lv_label_set_text(bar_time_label, buf);
}

// 通用的图标创建函数
lv_obj_t * create_icon(lv_obj_t * parent, const char * name, const void * src_img) {
    int32_t cont_w = ui_get_icon_container_w();
    int32_t img_size = ui_get_icon_img_size();
    int32_t radius = ui_get_icon_radius();

    // 1. 创建容器，使用Flex垂直布局
    lv_obj_t * item_cont = lv_obj_create(parent);
    lv_obj_set_size(item_cont, cont_w, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(item_cont, 0, 0);
    lv_obj_set_style_border_side(item_cont, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_flex_flow(item_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(item_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(item_cont, 2, 0);  // 图标和文字间距减小
    lv_obj_clear_flag(item_cont, LV_OBJ_FLAG_SCROLLABLE);

    // 让容器可以被点击
    lv_obj_add_flag(item_cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(item_cont, icon_event_cb, LV_EVENT_CLICKED, NULL);

    // 2. 创建图片
    lv_obj_t * icon_img = lv_image_create(item_cont);
    if(src_img != NULL) {
        lv_image_set_src(icon_img, src_img);
    } else {
        lv_obj_set_style_bg_color(icon_img, lv_color_hex(0x888888), 0);
    }
    lv_obj_set_size(icon_img, img_size, img_size);
    lv_obj_set_style_radius(icon_img, radius, 0);
    lv_obj_set_style_clip_corner(icon_img, true, 0);
    lv_obj_remove_flag(icon_img, LV_OBJ_FLAG_CLICKABLE);

    // 3. 创建文字
    lv_obj_t * label = lv_label_create(item_cont);
    lv_label_set_text(label, name);
    lv_obj_set_style_text_font(label, &font, 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);

    return item_cont;
}

// 创建翻页时钟单个数字 roller
static lv_obj_t * create_digit_roller(lv_obj_t * parent, const char * options, int32_t default_sel)
{
    lv_obj_t * roller = lv_roller_create(parent);
    lv_roller_set_options(roller, options, LV_ROLLER_MODE_NORMAL);

    int32_t w = lv_display_get_horizontal_resolution(lv_display_get_default());
    int32_t roller_w = (w <= 240) ? 30 : ((w <= 480) ? 36 : 44);
    int32_t roller_h = (w <= 240) ? 42 : ((w <= 480) ? 52 : 62);

    lv_obj_set_size(roller, roller_w, roller_h);

    // 背景样式：深色卡片
    lv_obj_set_style_bg_color(roller, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_bg_opa(roller, LV_OPA_50, 0);
    lv_obj_set_style_radius(roller, 8, 0);
    lv_obj_set_style_border_width(roller, 0, 0);
    lv_obj_set_style_pad_all(roller, 0, 0);

    // 文本样式
    const lv_font_t * fnt = (w <= 240) ? &lv_font_montserrat_24 : ((w <= 480) ? &lv_font_montserrat_36 : &lv_font_montserrat_40);
    lv_obj_set_style_text_font(roller, fnt, 0);
    lv_obj_set_style_text_color(roller, lv_color_hex(0x666666), 0);
    lv_obj_set_style_text_line_space(roller, 2, 0);

    // 选中项样式
    lv_obj_set_style_text_font(roller, fnt, LV_PART_SELECTED);
    lv_obj_set_style_text_color(roller, lv_color_white(), LV_PART_SELECTED);
    lv_obj_set_style_bg_opa(roller, 0, LV_PART_SELECTED);

    lv_roller_set_selected(roller, default_sel, LV_ANIM_OFF);
    return roller;
}

    // 为单个翻页器添加中间分割线
static void add_roller_divider(lv_obj_t * cont, lv_obj_t * roller)
{
    lv_obj_t * line = lv_obj_create(cont);
    int32_t w = lv_obj_get_width(roller);
    lv_obj_set_size(line, w - 4, 2);
    lv_obj_set_style_bg_color(line, lv_color_hex(0x94A3B8), 0);
    lv_obj_set_style_bg_opa(line, LV_OPA_20, 0);
    lv_obj_set_style_border_width(line, 0, 0);
    lv_obj_set_style_radius(line, 0, 0);
    lv_obj_align_to(line, roller, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(line, LV_OBJ_FLAG_FLOATING);
    lv_obj_clear_flag(line, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
}

// 首页：翻页大时钟 + 天气卡片 + 日期
void create_home_page(lv_obj_t * parent) {
    int32_t screen_w = lv_display_get_horizontal_resolution(lv_display_get_default());
    const lv_font_t * colon_font = (screen_w <= 240) ? &lv_font_montserrat_24 : ((screen_w <= 480) ? &lv_font_montserrat_36 : &lv_font_montserrat_40);

    // ---- 右上角天气卡片 ----
    lv_obj_t * weather_card = lv_obj_create(parent);
    int32_t card_w = 70;
    int32_t card_h = 25;
    lv_obj_set_size(weather_card, card_w, card_h);
    lv_obj_align(weather_card, LV_ALIGN_TOP_RIGHT, -12, 5 + ui_get_statusbar_height());
    lv_obj_set_style_bg_color(weather_card, COLOR_SURFACE, 0);
    lv_obj_set_style_bg_opa(weather_card, LV_OPA_60, 0);
    lv_obj_set_style_radius(weather_card, 16, 0);
    lv_obj_set_style_border_width(weather_card, 0, 0);
    lv_obj_set_flex_flow(weather_card, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(weather_card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(weather_card, 6, 0);
    lv_obj_clear_flag(weather_card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * temp_lbl = lv_label_create(weather_card);
    lv_label_set_text(temp_lbl, "25°C");
    lv_obj_set_style_text_font(temp_lbl, &font, 0);
    lv_obj_set_style_text_color(temp_lbl, lv_color_hex(0xF1F5F9), 0);

    lv_obj_t * desc_lbl = lv_label_create(weather_card);
    lv_label_set_text(desc_lbl, "晴");
    lv_obj_set_style_text_font(desc_lbl, &font, 0);
    lv_obj_set_style_text_color(desc_lbl, lv_color_hex(0x94A3B8), 0);

    // ---- 翻页时钟容器 ----
    lv_obj_t * clock_cont = lv_obj_create(parent);
    lv_obj_set_size(clock_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(clock_cont, 0, 0);
    lv_obj_set_style_border_width(clock_cont, 0, 0);
    lv_obj_set_flex_flow(clock_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(clock_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(clock_cont, 4, 0);
    lv_obj_align(clock_cont, LV_ALIGN_CENTER, 0, -25);
    lv_obj_clear_flag(clock_cont, LV_OBJ_FLAG_SCROLLABLE);

    // 默认时间 00:00:00
    g_flip_clock.h1 = create_digit_roller(clock_cont, "0\n1\n2", 1);
    g_flip_clock.h2 = create_digit_roller(clock_cont, "0\n1\n2\n3\n4\n5\n6\n7\n8\n9", 0);

    lv_obj_t * colon1 = lv_label_create(clock_cont);
    lv_label_set_text(colon1, ":");
    lv_obj_set_style_text_font(colon1, colon_font, 0);
    lv_obj_set_style_text_color(colon1, lv_color_white(), 0);

    g_flip_clock.m1 = create_digit_roller(clock_cont, "0\n1\n2\n3\n4\n5", 2);
    g_flip_clock.m2 = create_digit_roller(clock_cont, "0\n1\n2\n3\n4\n5\n6\n7\n8\n9", 4);

    lv_obj_t * colon2 = lv_label_create(clock_cont);
    lv_label_set_text(colon2, ":");
    lv_obj_set_style_text_font(colon2, colon_font, 0);
    lv_obj_set_style_text_color(colon2, lv_color_white(), 0);

    g_flip_clock.s1 = create_digit_roller(clock_cont, "0\n1\n2\n3\n4\n5", 2);
    g_flip_clock.s2 = create_digit_roller(clock_cont, "0\n1\n2\n3\n4\n5\n6\n7\n8\n9", 4);

    // 强制刷新 flex 布局后再加分割线，否则坐标还未计算
    lv_obj_update_layout(clock_cont);

    add_roller_divider(clock_cont, g_flip_clock.h1);
    add_roller_divider(clock_cont, g_flip_clock.h2);
    add_roller_divider(clock_cont, g_flip_clock.m1);
    add_roller_divider(clock_cont, g_flip_clock.m2);
    add_roller_divider(clock_cont, g_flip_clock.s1);
    add_roller_divider(clock_cont, g_flip_clock.s2);

    // ---- 日期和星期 ----
    lv_obj_t * date_lbl = lv_label_create(parent);
    lv_label_set_text(date_lbl, "1月1日  星期一");
    lv_obj_set_style_text_font(date_lbl, &font, 0);
    lv_obj_set_style_text_color(date_lbl, lv_color_hex(0xCBD5E1), 0);
    lv_obj_align(date_lbl, LV_ALIGN_CENTER, 0, 20);

    // 启动时间刷新（同时更新日期）
    lv_timer_create(update_time_task, 1000, date_lbl);
}

// 桌面页：3x2 布局
void create_desktop_page(lv_obj_t * parent, int start_idx) {
    // 创建桌面容器
    lv_obj_t * container = lv_obj_create(parent);
    lv_obj_set_width(container, LV_PCT(100));
    lv_obj_set_height(container, LV_PCT(100));
    lv_obj_set_style_bg_opa(container, 0, 0);
    lv_obj_set_style_border_side(container, LV_BORDER_SIDE_NONE, 0);

    // 第一行（紧贴状态栏下方）
    lv_obj_t * row1 = lv_obj_create(container);
    lv_obj_set_width(row1, LV_PCT(100));
    lv_obj_set_height(row1, LV_SIZE_CONTENT);
    lv_obj_set_pos(row1, 0, -5);
    lv_obj_set_style_bg_opa(row1, 0, 0);
    lv_obj_set_style_border_side(row1, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_flex_flow(row1, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_left(row1, ui_get_desktop_padding(), 0);
    lv_obj_set_style_pad_column(row1, ui_get_desktop_col_gap(), 0);

    // 第二行（在row1下方，留出行间距）
    lv_obj_t * row2 = lv_obj_create(container);
    lv_obj_set_width(row2, LV_PCT(100));
    lv_obj_set_height(row2, LV_SIZE_CONTENT);
    lv_obj_set_pos(row2, 0, 65);  // 两行之间的间距
    lv_obj_set_style_bg_opa(row2, 0, 0);
    lv_obj_set_style_border_side(row2, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_flex_flow(row2, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row2, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_left(row2, ui_get_desktop_padding(), 0);
    lv_obj_set_style_pad_column(row2, ui_get_desktop_col_gap(), 0);

    // 动态循环：创建6个图标，前3个在第一行，后3个在第二行
    for(int i = 0; i < ICONS_PER_PAGE; i++) {
        int current_idx = start_idx + i;

        if (current_idx >= ICON_COUNT) {
            break;
        }

        lv_obj_t * row = (i < 3) ? row1 : row2;
        lv_obj_t * icon = create_icon(row, icon_names[current_idx], ios_icons[current_idx]);
    }
}

// 创建 Dock 栏
static lv_obj_t * dock_container;

void create_dock(void) {
    int32_t dock_h;
    int32_t w = lv_display_get_horizontal_resolution(lv_display_get_default());
    if(w <= 240) dock_h = 50;
    else if(w <= 480) dock_h = 60;
    else dock_h = 70;

    // 创建 Dock 容器（毛玻璃风格）
    dock_container = lv_obj_create(lv_layer_top());
    lv_obj_set_width(dock_container, LV_PCT(90));
    lv_obj_set_height(dock_container, dock_h);
    lv_obj_align(dock_container, LV_ALIGN_BOTTOM_MID, 0, -6);  // 微微离底，更有悬浮感

    // 背景：半透明白色 + 背景模糊（backdrop blur）
    lv_obj_set_style_bg_color(dock_container, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(dock_container, LV_OPA_30, 0);   // 30% 透明度，让底层内容隐约透出
    lv_obj_set_style_blur_backdrop(dock_container, true, 0);  // 启用背景模糊
    lv_obj_set_style_blur_radius(dock_container, 12, 0);      // 模糊半径
    lv_obj_set_style_blur_quality(dock_container, LV_BLUR_QUALITY_AUTO, 0);

    // 圆角与边框：微边框增强玻璃质感
    lv_obj_set_style_radius(dock_container, 20, 0);
    lv_obj_set_style_border_width(dock_container, 1, 0);
    lv_obj_set_style_border_color(dock_container, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_opa(dock_container, LV_OPA_30, 0);

    // 柔和阴影，增加悬浮层次
    lv_obj_set_style_shadow_width(dock_container, 20, 0);
    lv_obj_set_style_shadow_color(dock_container, lv_color_black(), 0);
    lv_obj_set_style_shadow_opa(dock_container, LV_OPA_20, 0);
    lv_obj_set_style_shadow_ofs_y(dock_container, 4, 0);

    lv_obj_clear_flag(dock_container, LV_OBJ_FLAG_SCROLLABLE);

    // Flex 布局横向排列，内部内容居中
    lv_obj_set_flex_flow(dock_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(dock_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(dock_container, ui_get_desktop_col_gap(), 0);  // 与桌面图标间距一致

    // 创建 Dock 图标
    int32_t dock_icon_size = ui_get_icon_img_size() * 0.8;
    for(int i = 0; i < DOCK_ICON_COUNT; i++) {
        lv_obj_t * item_cont = lv_obj_create(dock_container);
        lv_obj_set_size(item_cont, dock_icon_size + 10, LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(item_cont, 0, 0);
        lv_obj_set_style_border_side(item_cont, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_pad_all(item_cont, 0, 0);
        lv_obj_set_flex_flow(item_cont, LV_FLEX_FLOW_COLUMN);  // 垂直排列
        lv_obj_set_flex_align(item_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_clear_flag(item_cont, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(item_cont, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_user_data(item_cont, (void *)i);  // 存储索引
        lv_obj_add_event_cb(item_cont, dock_icon_event_cb, LV_EVENT_CLICKED, NULL);

        // 图标图片
        lv_obj_t * icon_img = lv_image_create(item_cont);
        if(dock_icons[i] != NULL) {
            lv_image_set_src(icon_img, dock_icons[i]);
            lv_obj_set_size(icon_img, dock_icon_size, dock_icon_size);
        } else {
            lv_obj_set_size(icon_img, dock_icon_size, dock_icon_size);
            lv_obj_set_style_bg_color(icon_img, lv_color_hex(0x888888), 0);
        }
        int32_t radius = ui_get_icon_radius();
        lv_obj_set_style_radius(icon_img, radius, 0);
        lv_obj_set_style_clip_corner(icon_img, true, 0);
        lv_obj_remove_flag(icon_img, LV_OBJ_FLAG_CLICKABLE);

        // 隐藏文字（只显示图标）
        // lv_obj_t * label = lv_label_create(item_cont);
        // lv_label_set_text(label, dock_icon_names[i]);
        // lv_obj_set_style_text_font(label, &font, 0);
        // lv_obj_set_style_text_color(label, lv_color_white(), 0);
    }
}

void ui_init(void) {


    // 1. 创建 Tileview (三色对角线渐变，增加中点让顶部更明亮开阔)
    lv_obj_t * tv = lv_tileview_create(lv_screen_active());

    // static lv_grad_dsc_t bg_grad;
    // lv_grad_linear_init(&bg_grad, 0, 0, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, LV_GRAD_EXTEND_PAD);
    // bg_grad.stops[0].color = lv_color_hex(0x64D2FF);  // 起点 (0,0) 靛蓝
    // bg_grad.stops[0].opa   = LV_OPA_COVER;
    // bg_grad.stops[0].frac  = 0;
    // bg_grad.stops[1].color = lv_color_hex(0x3B82F6);  // 中点 明亮蓝
    // bg_grad.stops[1].opa   = LV_OPA_COVER;
    // bg_grad.stops[1].frac  = 128;
    // bg_grad.stops[2].color = lv_color_hex(0x0369A1);  // 终点 深天蓝
    // bg_grad.stops[2].opa   = LV_OPA_COVER;
    // bg_grad.stops[2].frac  = 255;
    // bg_grad.stops_count    = 3;

    // lv_obj_set_style_bg_grad(tv, &bg_grad, 0);
    lv_obj_set_style_bg_color(tv, lv_color_hex(0x3B82F6), 0);

    lv_obj_set_scrollbar_mode(tv, LV_SCROLLBAR_MODE_OFF);

    // 绑定事件处理器
    lv_obj_add_event_cb(tv, tileview_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // 2. 第一屏 (0,0)：首页
    lv_obj_t * t1 = lv_tileview_add_tile(tv, 0, 0, LV_DIR_RIGHT);
    lv_obj_set_style_bg_opa(t1, 0, 0);
    create_home_page(t1);

    // 3. 第二屏 (1,0)：桌面第一页 (4x2, 前 8 个图标)
    lv_obj_t * t2 = lv_tileview_add_tile(tv, 1, 0, LV_DIR_HOR);
    lv_obj_set_style_bg_opa(t2, 0, 0);
    create_desktop_page(t2, 0);

    // 4. 第三屏 (2,0)：桌面第二页 (3x2, 后 6 个图标)
    lv_obj_t * t3 = lv_tileview_add_tile(tv, 2, 0, LV_DIR_LEFT);
    lv_obj_set_style_bg_opa(t3, 0, 0);
    create_desktop_page(t3, 6);

    // 创建 Dock 栏
    create_dock();

    // 最后创建指示器
    create_page_indicator();

    // 创建状态栏
    ui_statusbar_create();

    // 设置状态栏为深色（适配浅色背景）
    update_status_bar_color(lv_color_hex(0x5AC8FA));
}
