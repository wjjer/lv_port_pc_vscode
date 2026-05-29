#include "ui_weather.h"
#include "../../../components/ui_titlebar.h"
#include "../../../ui.h"
#include <time.h>
#include <string.h>
#include "lvgl.h"
#include <stdlib.h>

/**
 * @brief 创建天气预报单个小卡片
 */
static lv_obj_t * weather_page = NULL;

static lv_obj_t * create_forecast_item(lv_obj_t * parent, const char * day, const char * temp, const void * img_src)
{
    lv_obj_t * obj = lv_obj_create(parent);
    lv_obj_set_size(obj, 90, 75);
    lv_obj_set_style_bg_color(obj, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_20, 0); // 半透明背景
    lv_obj_set_style_border_width(obj, 1, 0);
    lv_obj_set_style_border_color(obj, lv_color_white(), 0);
    lv_obj_set_style_border_opa(obj, LV_OPA_30, 0);
    lv_obj_set_style_radius(obj, 12, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    // 设置卡片内部布局为垂直居中
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(obj, 4, 0);
    lv_obj_set_style_pad_gap(obj, 4, 0);

    // 日期标签
    lv_obj_t * label_day = lv_label_create(obj);
    lv_label_set_text(label_day, day);
    lv_obj_set_style_text_font(label_day, &font, 0);
    lv_obj_set_style_text_color(label_day, lv_color_white(), 0);
    lv_obj_set_style_opa(label_day, LV_OPA_70, 0);

    // 天气图标
    if(img_src != NULL) {
        lv_obj_t * icon = lv_img_create(obj);
        lv_img_set_src(icon, img_src);
        lv_obj_set_size(icon, 24, 24);
    } else {
        // 无图片时用色块占位
        lv_obj_t * icon = lv_obj_create(obj);
        lv_obj_set_size(icon, 24, 24);
        lv_obj_set_style_bg_color(icon, lv_palette_main(LV_PALETTE_YELLOW), 0);
        lv_obj_set_style_radius(icon, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(icon, 0, 0);
    }

    // 温度标签
    lv_obj_t * label_temp = lv_label_create(obj);
    lv_label_set_text(label_temp, temp);
    lv_obj_set_style_text_font(label_temp, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(label_temp, lv_color_white(), 0);

    return obj;
}

static void ui_weather_screen_init(void)
{
    if(weather_page != NULL) {
        lv_obj_del(weather_page);
    }

    int32_t scr_w = UI_SCREEN_WIDTH;
    int32_t scr_h = UI_SCREEN_HEIGHT;

    weather_page = lv_obj_create(lv_layer_top());
    lv_obj_set_size(weather_page, scr_w, scr_h);
    lv_obj_set_style_pad_all(weather_page, 0, 0);
    lv_obj_set_style_border_side(weather_page, LV_BORDER_SIDE_NONE, 0);
    lv_obj_clear_flag(weather_page, LV_OBJ_FLAG_SCROLLABLE);

    // 1. 背景渐变
    lv_obj_set_style_bg_grad_dir(weather_page, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_main_stop(weather_page, 0, 0);
    lv_obj_set_style_bg_grad_stop(weather_page, 255, 0);
    lv_obj_set_style_bg_color(weather_page, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_bg_grad_color(weather_page, lv_color_hex(0x2563EB), 0);

    if(status_bar) lv_obj_add_flag(status_bar, LV_OBJ_FLAG_HIDDEN);

    // 2. 顶部标题栏（复用系统通用标题栏，自动避开状态栏）
    ui_titlebar_create(weather_page, "北京", (lv_event_cb_t)ui_weather_hide, NULL, NULL, NULL, NULL, NULL, NULL, true);

    // 3. 当前天气核心区
    lv_obj_t * temp_val = lv_label_create(weather_page);
    lv_label_set_text(temp_val, "25");
    lv_obj_set_style_text_font(temp_val, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(temp_val, lv_color_white(), 0);
    lv_obj_align(temp_val, LV_ALIGN_TOP_LEFT, 20, 50);

    lv_obj_t * temp_unit = lv_label_create(weather_page);
    lv_label_set_text(temp_unit, "°");
    lv_obj_set_style_text_font(temp_unit, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(temp_unit, lv_color_white(), 0);
    lv_obj_align_to(temp_unit, temp_val, LV_ALIGN_OUT_RIGHT_TOP, 2, 8);

    lv_obj_t * cond_label = lv_label_create(weather_page);
    lv_label_set_text(cond_label, "晴");
    lv_obj_set_style_text_font(cond_label, &font, 0);
    lv_obj_set_style_text_color(cond_label, lv_color_white(), 0);
    lv_obj_align_to(cond_label, temp_val, LV_ALIGN_OUT_BOTTOM_LEFT, 0, -5);

    lv_obj_t * aqi_label = lv_label_create(weather_page);
    lv_label_set_text(aqi_label, "AQI 32");
    lv_obj_set_style_text_font(aqi_label, &lv_font_montserrat_10, 0);
    lv_obj_set_style_bg_color(aqi_label, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(aqi_label, LV_OPA_30, 0);
    lv_obj_set_style_border_width(aqi_label, 1, 0);
    lv_obj_set_style_border_color(aqi_label, lv_color_white(), 0);
    lv_obj_set_style_border_opa(aqi_label, LV_OPA_20, 0);
    lv_obj_set_style_pad_hor(aqi_label, 4, 0);
    lv_obj_set_style_radius(aqi_label, 4, 0);
    lv_obj_align_to(aqi_label, cond_label, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

    // 右侧大图标占位
    lv_obj_t * big_sun = lv_obj_create(weather_page);
    lv_obj_set_size(big_sun, 60, 60);
    lv_obj_set_style_bg_color(big_sun, lv_palette_main(LV_PALETTE_YELLOW), 0);
    lv_obj_set_style_radius(big_sun, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(big_sun, 0, 0);
    lv_obj_align(big_sun, LV_ALIGN_TOP_RIGHT, -30, 60);

    // 4. 详细参数行
    lv_obj_t * info_row = lv_label_create(weather_page);
    lv_label_set_text_fmt(info_row, "%s 45%%    28°-18°", LV_SYMBOL_CHARGE, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_font(info_row, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(info_row, lv_color_white(), 0);
    lv_obj_set_style_opa(info_row, LV_OPA_80, 0);
    lv_obj_align(info_row, LV_ALIGN_TOP_LEFT, 20, 130);

    // 5. 底部预报容器
    lv_obj_t * forecast_cont = lv_obj_create(weather_page);
    lv_obj_set_size(forecast_cont, scr_w - 20, 85);
    lv_obj_set_style_bg_opa(forecast_cont, 0, 0);
    lv_obj_set_style_border_width(forecast_cont, 0, 0);
    lv_obj_set_style_pad_all(forecast_cont, 0, 0);
    lv_obj_align(forecast_cont, LV_ALIGN_BOTTOM_MID, 0, -10);

    lv_obj_set_flex_flow(forecast_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(forecast_cont, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    create_forecast_item(forecast_cont, "明天", "28/18°", NULL);
    create_forecast_item(forecast_cont, "后天", "26/17°", NULL);
    create_forecast_item(forecast_cont, "周六", "30/20°", NULL);
}

void ui_weather_show(void) {
    ui_weather_screen_init();
}

void ui_weather_hide(void) {
    if(weather_page != NULL) {
        lv_obj_del(weather_page);
        weather_page = NULL;
    }
    if(status_bar) lv_obj_remove_flag(status_bar, LV_OBJ_FLAG_HIDDEN);
}
