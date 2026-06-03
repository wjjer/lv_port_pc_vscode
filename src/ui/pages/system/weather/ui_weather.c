#include "ui_weather.h"
#include "../../../components/ui_titlebar.h"
#include "../../../ui.h"
#include <time.h>
#include <string.h>
#include "lvgl.h"
#include <stdlib.h>

static lv_obj_t * weather_page = NULL;
static lv_obj_t * location_dialog = NULL;
static lv_obj_t * title_bar_obj = NULL;
static char current_location[32] = "北京";

// === 新增：方案A 搜索相关控件指针 ===
static lv_obj_t * search_ta = NULL; // 搜索输入框
static lv_obj_t * kb = NULL;        // 虚拟键盘
static lv_obj_t * result_list = NULL; // 搜索结果列表

// 声明外部接口：这里你需要自己实现一个向 API 发送拼音并返回结果的函数
extern void network_weather_update_request(const char * city_name);
// 声明外部接口：触发搜索的函数
extern void network_city_search_request(const char * query_str);

static void ui_weather_screen_init(void);
static void location_select_event(lv_event_t * e);
static void location_dialog_close(void);
// 提取出“仅更新数据和界面文本”的函数
void ui_weather_update_location_display(const char * city)
{
    if(city == NULL) return;

    strncpy(current_location, city, sizeof(current_location) - 1);
    current_location[sizeof(current_location) - 1] = '\0';

    if(title_bar_obj != NULL) {
        lv_obj_t * title_label = lv_obj_get_child(title_bar_obj, 1);
        if(title_label && lv_obj_check_type(title_label, &lv_label_class)) {
            lv_label_set_text(title_label, current_location);
        }
    }
    // 当位置改变后，调用网络请求更新天气数据
    // network_weather_update_request(current_location);
}

// 供外部（网络回调）调用的接口：用于往列表中动态添加搜索出来的城市
// 参数说明: city_name="济南", city_id="101120101" (可选，如果你需要存ID的话)
void ui_weather_add_search_result(const char * city_name, const char * city_id) {
    if(result_list == NULL) return;

    // 分配内存保存城市名，防止局部变量失效（记得在销毁列表时清理，或由系统统一管理）
    // 为了简单，这里假定 city_name 的生命周期由调用者管理，或者像之前一样只是临时展示
    // 在真实项目中，你可能需要 malloc 一段内存存入 user_data，并在 btn 删除时 free
    char * allocated_name = strdup(city_name);

    lv_obj_t * btn = lv_list_add_btn(result_list, LV_SYMBOL_GPS, city_name);
    lv_obj_set_style_text_font(btn, &font, 0);
    lv_obj_set_style_text_color(btn, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, 0);

    // 把分配的字符串指针存入 btn
    lv_obj_set_user_data(btn, (void *)allocated_name);

    // 复用之前的选中回调
    lv_obj_add_event_cb(btn, location_select_event, LV_EVENT_CLICKED, NULL);
}

// 清空当前的搜索结果列表
static void clear_search_results(void) {
    if(result_list != NULL) {
        // 注意：如果你在添加btn时 malloc 了内存，这里需要遍历 free 掉 user_data
        uint32_t child_cnt = lv_obj_get_child_cnt(result_list);
        for(uint32_t i = 0; i < child_cnt; i++) {
            lv_obj_t * child = lv_obj_get_child(result_list, i);
            void * user_data = lv_obj_get_user_data(child);
            if(user_data) free(user_data);
        }
        lv_obj_clean(result_list);
    }
}

// =========================================================
// 以下为原有的 Forecast Item 创建等函数，保持不变
// =========================================================

static lv_obj_t * create_forecast_item(lv_obj_t * parent, const char * day, const char * temp, const void * img_src)
{
    // ... (保留你原来的代码) ...
    lv_obj_t * obj = lv_obj_create(parent);
    lv_obj_set_size(obj, 90, 75);
    lv_obj_set_style_bg_color(obj, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_20, 0);
    lv_obj_set_style_border_width(obj, 1, 0);
    lv_obj_set_style_border_color(obj, lv_color_white(), 0);
    lv_obj_set_style_border_opa(obj, LV_OPA_30, 0);
    lv_obj_set_style_radius(obj, 12, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(obj, 4, 0);
    lv_obj_set_style_pad_gap(obj, 4, 0);

    lv_obj_t * label_day = lv_label_create(obj);
    lv_label_set_text(label_day, day);
    lv_obj_set_style_text_font(label_day, &font, 0);
    lv_obj_set_style_text_color(label_day, lv_color_white(), 0);
    lv_obj_set_style_opa(label_day, LV_OPA_70, 0);

    if(img_src != NULL) {
        lv_obj_t * icon = lv_img_create(obj);
        lv_img_set_src(icon, img_src);
        lv_obj_set_size(icon, 24, 24);
    } else {
        lv_obj_t * icon = lv_obj_create(obj);
        lv_obj_set_size(icon, 24, 24);
        lv_obj_set_style_bg_color(icon, lv_palette_main(LV_PALETTE_YELLOW), 0);
        lv_obj_set_style_radius(icon, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(icon, 0, 0);
    }

    lv_obj_t * label_temp = lv_label_create(obj);
    lv_label_set_text(label_temp, temp);
    lv_obj_set_style_text_font(label_temp, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(label_temp, lv_color_white(), 0);

    return obj;
}

// 专职销毁弹窗，清除指针避免野指针
static void location_dialog_close(void) {
    if(location_dialog != NULL) {
        clear_search_results(); // 清理可能分配的内存
        lv_obj_del(location_dialog);
        location_dialog = NULL;
        search_ta = NULL;
        kb = NULL;
        result_list = NULL;
    }
}

static void cancel_btn_click_cb(lv_event_t * e) {
    location_dialog_close();
}

static void location_select_event(lv_event_t * e) {
    lv_obj_t * btn = lv_event_get_target(e);
    const char * city = lv_obj_get_user_data(btn);
    if(city != NULL) {
        ui_weather_update_location_display(city);
        location_dialog_close();
    }
}

// === 核心改动：键盘事件回调 ===
static void keyboard_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    // 如果按下了键盘上的“回车/确认”键
    if(code == LV_EVENT_READY) {
        const char * query_txt = lv_textarea_get_text(search_ta);
        if(strlen(query_txt) > 0) {
            // 1. 清空当前列表
            clear_search_results();
            // 2. 显示一个提示信息（可选）
            lv_list_add_text(result_list, "搜索中...");
            // 3. 触发网络请求去搜索拼音对应的城市 (例如: "jinan")
            // network_city_search_request(query_txt);

            // 【本地模拟测试代码，跑通网络前可打开测试UI】
            /*
            clear_search_results();
            if(strcmp(query_txt, "beijing") == 0) {
                ui_weather_add_search_result("北京市", "101010100");
                ui_weather_add_search_result("北京朝阳", "101010300");
            } else {
                lv_list_add_text(result_list, "未找到城市");
            }
            */
        }
    }
    // 如果按下了键盘上的“取消/隐藏”键
    else if(code == LV_EVENT_CANCEL) {
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN); // 隐藏键盘
        lv_obj_clear_state(search_ta, LV_STATE_FOCUSED); // 取消输入框焦点
    }
}

// 文本框点击事件：点击时弹出键盘
static void textarea_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_FOCUSED || code == LV_EVENT_CLICKED) {
        if(kb != NULL) {
            lv_obj_remove_flag(kb, LV_OBJ_FLAG_HIDDEN); // 显示键盘
        }
    }
}

// === 核心改动：重构后的弹窗 UI ===
static void show_location_dialog(void) {
    if(location_dialog != NULL) {
        return;
    }

    location_dialog = lv_obj_create(weather_page ? weather_page : lv_screen_active());
    lv_obj_set_size(location_dialog, UI_SCREEN_WIDTH - 20, UI_SCREEN_HEIGHT - 40); // 弹窗稍微做大一点，给键盘留空间
    lv_obj_center(location_dialog);

    lv_obj_set_style_bg_color(location_dialog, lv_color_hex(0x0F172A), 0);
    lv_obj_set_style_bg_opa(location_dialog, LV_OPA_90, 0);
    lv_obj_set_style_border_width(location_dialog, 1, 0);
    lv_obj_set_style_border_color(location_dialog, lv_color_white(), 0);
    lv_obj_set_style_border_opa(location_dialog, LV_OPA_20, 0);
    lv_obj_set_style_radius(location_dialog, 12, 0);
    lv_obj_set_style_pad_all(location_dialog, 8, 0);

    // 1. 顶部标题和关闭按钮同行显示，节省垂直空间
    lv_obj_t * title = lv_label_create(location_dialog);
    lv_label_set_text(title, "搜索城市(拼音)");
    lv_obj_set_style_text_font(title, &font, 0);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 4, 4);

    lv_obj_t * close_btn = lv_btn_create(location_dialog);
    lv_obj_set_size(close_btn, 40, 30);
    lv_obj_align(close_btn, LV_ALIGN_TOP_RIGHT, -4, 0);
    lv_obj_set_style_bg_color(close_btn, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_t * close_label = lv_label_create(close_btn);
    lv_label_set_text(close_label, LV_SYMBOL_CLOSE);
    lv_obj_set_style_text_color(close_label, lv_color_white(), 0);
    lv_obj_center(close_label);
    lv_obj_add_event_cb(close_btn, cancel_btn_click_cb, LV_EVENT_CLICKED, NULL);

    // 2. 创建输入框 (Text Area)
    search_ta = lv_textarea_create(location_dialog);
    lv_textarea_set_one_line(search_ta, true);
    lv_textarea_set_placeholder_text(search_ta, "输入拼音如 jinan");
    lv_obj_set_width(search_ta, lv_pct(100));
    lv_obj_align(search_ta, LV_ALIGN_TOP_MID, 0, 40);
    lv_obj_add_event_cb(search_ta, textarea_event_cb, LV_EVENT_ALL, NULL);

    // 3. 创建结果列表 (List)
    result_list = lv_list_create(location_dialog);
    lv_obj_set_size(result_list, lv_pct(100), LV_SIZE_CONTENT); // 高度自适应
    lv_obj_set_style_max_height(result_list, 100, 0); // 限制最大高度，防止盖住键盘
    lv_obj_align(result_list, LV_ALIGN_TOP_MID, 0, 85);
    lv_obj_set_style_bg_opa(result_list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(result_list, 0, 0);

    // 4. 创建键盘 (Keyboard)
    kb = lv_keyboard_create(location_dialog);
    lv_keyboard_set_textarea(kb, search_ta);
    lv_obj_set_size(kb, lv_pct(100), 120); // 根据你的屏幕高度调整键盘高度
    lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_event_cb(kb, keyboard_event_cb, LV_EVENT_ALL, NULL);
}

static void location_btn_callback(lv_event_t * e) {
    show_location_dialog();
}

// =========================================================
// 以下为原有的 ui_weather_screen_init 等函数，保持不变
// =========================================================
static void ui_weather_screen_init(void)
{
    // ... (保持你原来的初始化代码，无需改动) ...
    if(weather_page != NULL) {
        return;
    }

    int32_t scr_w = UI_SCREEN_WIDTH;
    int32_t scr_h = UI_SCREEN_HEIGHT;

    weather_page = lv_obj_create(lv_layer_top());
    lv_obj_set_size(weather_page, scr_w, scr_h);
    lv_obj_set_style_pad_all(weather_page, 0, 0);
    lv_obj_set_style_border_side(weather_page, LV_BORDER_SIDE_NONE, 0);
    lv_obj_clear_flag(weather_page, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_grad_dir(weather_page, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_main_stop(weather_page, 0, 0);
    lv_obj_set_style_bg_grad_stop(weather_page, 255, 0);
    lv_obj_set_style_bg_color(weather_page, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_bg_grad_color(weather_page, lv_color_hex(0x2563EB), 0);

    if(status_bar) lv_obj_add_flag(status_bar, LV_OBJ_FLAG_HIDDEN);

    title_bar_obj = ui_titlebar_create(weather_page, current_location, (lv_event_cb_t)ui_weather_hide, NULL, "城市", location_btn_callback, NULL, NULL, NULL, true);

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
    lv_obj_set_style_text_color(aqi_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(aqi_label, &lv_font_montserrat_10, 0);
    lv_obj_set_style_bg_color(aqi_label, lv_color_hex(0x2563EB), 0);
    lv_obj_set_style_bg_opa(aqi_label, LV_OPA_30, 0);
    lv_obj_set_style_border_width(aqi_label, 1, 0);
    lv_obj_set_style_border_color(aqi_label, lv_color_white(), 0);
    lv_obj_set_style_border_opa(aqi_label, LV_OPA_20, 0);
    lv_obj_set_style_pad_hor(aqi_label, 4, 0);
    lv_obj_set_style_radius(aqi_label, 4, 0);
    lv_obj_align_to(aqi_label, cond_label, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

    lv_obj_t * big_sun = lv_obj_create(weather_page);
    lv_obj_set_size(big_sun, 60, 60);
    lv_obj_set_style_bg_color(big_sun, lv_palette_main(LV_PALETTE_YELLOW), 0);
    lv_obj_set_style_radius(big_sun, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(big_sun, 0, 0);
    lv_obj_align(big_sun, LV_ALIGN_TOP_RIGHT, -30, 60);

    lv_obj_t * info_row = lv_label_create(weather_page);
    lv_label_set_text_fmt(info_row, "%s 45%%    28°-18°", LV_SYMBOL_CHARGE, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_font(info_row, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(info_row, lv_color_white(), 0);
    lv_obj_set_style_opa(info_row, LV_OPA_80, 0);
    lv_obj_align(info_row, LV_ALIGN_TOP_LEFT, 20, 130);

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
    location_dialog_close();
    if(weather_page != NULL) {
        lv_obj_del(weather_page);
        weather_page = NULL;
        title_bar_obj = NULL;
    }
    if(status_bar) lv_obj_remove_flag(status_bar, LV_OBJ_FLAG_HIDDEN);
}
