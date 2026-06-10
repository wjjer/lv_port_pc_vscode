#include "ui_color_picker.h"
#include "../ui_config.h"
#include "../ui.h"

// 配色常量
#define COLOR_ITEM_BG        lv_color_white()
#define COLOR_TEXT_MAIN      lv_color_hex(0x000000)
#define COLOR_TEXT_MUTED     lv_color_hex(0x8E8E93)
#define COLOR_IOS_BLUE       lv_color_hex(0x007AFF)

static lv_obj_t * overlay = NULL;
static lv_obj_t * card = NULL;
static lv_obj_t * preview_box = NULL;
static lv_obj_t * preview_text = NULL;
static void (*current_callback)(lv_color_t chosen_color, void * user_data) = NULL;
static void * current_user_data = NULL;
static lv_color_t current_color;

static lv_obj_t * slider_red = NULL;
static lv_obj_t * slider_green = NULL;
static lv_obj_t * slider_blue = NULL;

// 记录 card 初始宽高（用于缩放动画的基准）
static int32_t card_init_w = 0;
static int32_t card_init_h = 0;

// ==================== 动画工具 ====================

static void hide_color_picker_anim_ready_cb(lv_anim_t * a) {
    (void)a;
    if (overlay != NULL) {
        lv_obj_del(overlay);
        overlay = NULL;
    }
    card = NULL;
    preview_box = NULL;
    preview_text = NULL;
    slider_red = NULL;
    slider_green = NULL;
    slider_blue = NULL;
}

static void show_anim_exec(lv_anim_t * a, int32_t v) {
    lv_obj_t * obj = a->var;
    if (obj != card) return;
    if (v <= 0) { v = 1; }
    int32_t ratio = v * 1.0f / 255.0f;
    int32_t w = (int32_t)(card_init_w * ratio);
    int32_t h = (int32_t)(card_init_h * ratio);
    if (w <= 0) w = 1;
    if (h <= 0) h = 1;
    lv_obj_set_size(obj, w, h);
    int32_t cx = UI_SCREEN_WIDTH / 2;
    int32_t cy = UI_SCREEN_HEIGHT / 2;
    lv_obj_set_pos(obj, cx - w / 2, cy - h / 2);
}

static void show_color_picker_anim(void) {
    card_init_w = lv_obj_get_width(card);
    card_init_h = lv_obj_get_height(card);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, card);
    lv_anim_set_values(&a, 0, 255);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)show_anim_exec);
    lv_anim_set_duration(&a, 200);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);
}

// ==================== 颜色工具 ====================

static void update_color_display(void) {
    char buf[8];
    uint32_t hex = lv_color_to_int(current_color);
    lv_snprintf(buf, sizeof(buf), "#%06X", hex);
    lv_label_set_text(preview_text, buf);
    if (preview_box != NULL) {
        lv_obj_set_style_bg_color(preview_box, current_color, 0);
    }
}

static void apply_rgb_values(void) {
    uint8_t r = (uint8_t)lv_slider_get_value(slider_red);
    uint8_t g = (uint8_t)lv_slider_get_value(slider_green);
    uint8_t b = (uint8_t)lv_slider_get_value(slider_blue);
    current_color = lv_color_hex((int)((r << 16) | (g << 8) | b));
    update_color_display();
}

// ==================== 滑块事件 ====================

static void slider_red_event_cb(lv_event_t * e) {
    (void)lv_event_get_code(e);
    apply_rgb_values();
}

static void slider_green_event_cb(lv_event_t * e) {
    (void)lv_event_get_code(e);
    apply_rgb_values();
}

static void slider_blue_event_cb(lv_event_t * e) {
    (void)lv_event_get_code(e);
    apply_rgb_values();
}

// ==================== 按钮回调 ====================

static void overlay_click_cb(lv_event_t * e) {
    (void)e;
    if (current_callback) {
        current_callback(current_color, current_user_data);
        current_callback = NULL;
    }
    if (card != NULL) {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, card);
        lv_anim_set_values(&a, 255, 0);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)show_anim_exec);
        lv_anim_set_duration(&a, 150);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
        lv_anim_set_ready_cb(&a, hide_color_picker_anim_ready_cb);
        lv_anim_start(&a);
    }
}

static void confirm_color_cb(lv_event_t * e) {
    (void)e;
    if (current_callback) {
        current_callback(current_color, current_user_data);
        current_callback = NULL;
    }
    if (card != NULL) {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, card);
        lv_anim_set_values(&a, 255, 0);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)show_anim_exec);
        lv_anim_set_duration(&a, 150);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
        lv_anim_set_ready_cb(&a, hide_color_picker_anim_ready_cb);
        lv_anim_start(&a);
    }
}

// ==================== 公开接口 ====================

void ui_color_picker_show(lv_color_t initial_color,
    void (*callback)(lv_color_t chosen_color, void * user_data),
    void * user_data) {
    if (overlay != NULL) return;

    current_callback = callback;
    current_user_data = user_data;
    current_color = initial_color;

    // 遮罩层
    overlay = lv_obj_create(lv_layer_top());
    lv_obj_set_size(overlay, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(overlay, LV_OPA_50, 0);
    lv_obj_set_style_border_width(overlay, 0, 0);
    lv_obj_set_style_pad_all(overlay, 0, 0);
    lv_obj_clear_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(overlay, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(overlay, overlay_click_cb, LV_EVENT_CLICKED, NULL);

    // 卡片
    card = lv_obj_create(overlay);
    lv_obj_set_size(card, 280, 250);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, COLOR_ITEM_BG, 0);
    lv_obj_set_style_radius(card, 16, 0);
    lv_obj_set_style_clip_corner(card, true, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_shadow_width(card, 14, 0);
    lv_obj_set_style_shadow_color(card, lv_color_black(), 0);
    lv_obj_set_style_shadow_opa(card, LV_OPA_10, 0);
    lv_obj_set_style_shadow_ofs_y(card, 6, 0);
    lv_obj_set_style_pad_hor(card, 20, 0);
    lv_obj_set_style_pad_top(card, 24, 0);
    lv_obj_set_style_pad_bottom(card, 24, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(card, 12, 0);

    // 标题
    lv_obj_t * title_label = lv_label_create(card);
    lv_label_set_text(title_label, "桌面背景色");
    lv_obj_set_style_text_font(title_label, &font, 0);
    lv_obj_set_style_text_color(title_label, COLOR_TEXT_MAIN, 0);
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(title_label, 240);

    // 预览区域
    lv_obj_t * preview_row = lv_obj_create(card);
    lv_obj_set_size(preview_row, LV_PCT(100), 50);
    lv_obj_set_style_bg_opa(preview_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(preview_row, 0, 0);
    lv_obj_set_style_pad_all(preview_row, 0, 0);
    lv_obj_set_flex_flow(preview_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(preview_row, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    preview_box = lv_obj_create(preview_row);
    lv_obj_set_size(preview_box, 50, 50);
    lv_obj_set_style_bg_color(preview_box, current_color, 0);
    lv_obj_set_style_radius(preview_box, 8, 0);
    lv_obj_set_style_border_width(preview_box, 2, 0);
    lv_obj_set_style_border_color(preview_box, lv_color_hex(0xCCCCCC), 0);

    preview_text = lv_label_create(preview_row);
    {
        uint32_t hex = lv_color_to_int(current_color);
        char buf[8];
        lv_snprintf(buf, sizeof(buf), "#%06X", hex);
        lv_label_set_text(preview_text, buf);
    }
    lv_obj_set_style_text_font(preview_text, &font, 0);
    lv_obj_set_style_text_color(preview_text, COLOR_TEXT_MUTED, 0);

    // ---- RGB 滑块组 ----
    lv_obj_t * sliders_cont = lv_obj_create(card);
    lv_obj_set_size(sliders_cont, LV_PCT(100), 70);
    lv_obj_set_style_bg_opa(sliders_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(sliders_cont, 0, 0);
    lv_obj_set_style_pad_all(sliders_cont, 0, 0);
    lv_obj_set_style_pad_row(sliders_cont, 4, 0);
    lv_obj_set_flex_flow(sliders_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(sliders_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 红色滑块
    lv_obj_t * r_cont = lv_obj_create(sliders_cont);
    lv_obj_set_size(r_cont, LV_PCT(100), 18);
    lv_obj_set_style_bg_opa(r_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(r_cont, 0, 0);
    lv_obj_set_style_pad_all(r_cont, 0, 0);
    lv_obj_set_flex_flow(r_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(r_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(r_cont, 6, 0);

    lv_obj_t * r_label = lv_label_create(r_cont);
    lv_label_set_text(r_label, "R");
    lv_obj_set_style_text_font(r_label, &font, 0);
    lv_obj_set_style_text_color(r_label, lv_color_hex(0xFF0000), 0);
    lv_obj_set_width(r_label, 14);

    slider_red = lv_slider_create(r_cont);
    lv_obj_set_size(slider_red, LV_PCT(85), 16);
    lv_slider_set_range(slider_red, 0, 255);
    lv_slider_set_value(slider_red, (uint32_t)((lv_color_to_int(current_color) >> 16) & 0xFF), LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider_red, lv_color_hex(0x000000), LV_PART_KNOB);
    lv_obj_set_style_bg_color(slider_red, lv_color_hex(0xFF0000), LV_PART_INDICATOR);
    lv_obj_add_event_cb(slider_red, slider_red_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // 绿色滑块
    lv_obj_t * g_cont = lv_obj_create(sliders_cont);
    lv_obj_set_size(g_cont, LV_PCT(100), 18);
    lv_obj_set_style_bg_opa(g_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(g_cont, 0, 0);
    lv_obj_set_style_pad_all(g_cont, 0, 0);
    lv_obj_set_flex_flow(g_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(g_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(g_cont, 6, 0);

    lv_obj_t * g_label = lv_label_create(g_cont);
    lv_label_set_text(g_label, "G");
    lv_obj_set_style_text_font(g_label, &font, 0);
    lv_obj_set_style_text_color(g_label, lv_color_hex(0x00AA00), 0);
    lv_obj_set_width(g_label, 14);

    slider_green = lv_slider_create(g_cont);
    lv_obj_set_size(slider_green, LV_PCT(85), 16);
    lv_slider_set_range(slider_green, 0, 255);
    lv_slider_set_value(slider_green, (uint32_t)((lv_color_to_int(current_color) >> 8) & 0xFF), LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider_green, lv_color_hex(0x000000), LV_PART_KNOB);
    lv_obj_set_style_bg_color(slider_green, lv_color_hex(0x00FF00), LV_PART_INDICATOR);
    lv_obj_add_event_cb(slider_green, slider_green_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // 蓝色滑块
    lv_obj_t * b_cont = lv_obj_create(sliders_cont);
    lv_obj_set_size(b_cont, LV_PCT(100), 18);
    lv_obj_set_style_bg_opa(b_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(b_cont, 0, 0);
    lv_obj_set_style_pad_all(b_cont, 0, 0);
    lv_obj_set_flex_flow(b_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(b_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(b_cont, 6, 0);

    lv_obj_t * b_label = lv_label_create(b_cont);
    lv_label_set_text(b_label, "B");
    lv_obj_set_style_text_font(b_label, &font, 0);
    lv_obj_set_style_text_color(b_label, lv_color_hex(0x0000FF), 0);
    lv_obj_set_width(b_label, 14);

    slider_blue = lv_slider_create(b_cont);
    lv_obj_set_size(slider_blue, LV_PCT(85), 16);
    lv_slider_set_range(slider_blue, 0, 255);
    lv_slider_set_value(slider_blue, (uint32_t)(lv_color_to_int(current_color) & 0xFF), LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider_blue, lv_color_hex(0x000000), LV_PART_KNOB);
    lv_obj_set_style_bg_color(slider_blue, lv_color_hex(0x0000FF), LV_PART_INDICATOR);
    lv_obj_add_event_cb(slider_blue, slider_blue_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // 应用按钮行
    lv_obj_t * btn_row = lv_obj_create(card);
    lv_obj_set_size(btn_row, LV_PCT(100), 40);
    lv_obj_set_style_bg_opa(btn_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_row, 0, 0);
    lv_obj_set_style_pad_all(btn_row, 0, 0);
    lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * btn_apply = lv_obj_create(btn_row);
    lv_obj_set_size(btn_apply, 100, 38);
    lv_obj_set_style_bg_color(btn_apply, COLOR_IOS_BLUE, 0);
    lv_obj_set_style_radius(btn_apply, 10, 0);
    lv_obj_add_flag(btn_apply, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t * lbl_apply = lv_label_create(btn_apply);
    lv_label_set_text(lbl_apply, "应用");
    lv_obj_set_style_text_font(lbl_apply, &font, 0);
    lv_obj_center(lbl_apply);
    lv_obj_add_event_cb(btn_apply, confirm_color_cb, LV_EVENT_CLICKED, NULL);

    // 显示动画
    show_color_picker_anim();
}
