#include "ui_dialog.h"
#include "../ui_config.h"
#include "../ui.h"

// 配色常量（与 ui_settings.c 保持一致）
#define COLOR_ITEM_BG        lv_color_white()
#define COLOR_TEXT_MAIN      lv_color_hex(0x000000)
#define COLOR_TEXT_MUTED     lv_color_hex(0x8E8E93)
#define COLOR_IOS_BLUE       lv_color_hex(0x007AFF)

static lv_obj_t * overlay = NULL;
static lv_obj_t * card = NULL;
static ui_dialog_done_cb_t current_done_cb = NULL;
static void * current_user_data = NULL;

// 记录 card 初始宽高（用于缩放动画的基准）
static int32_t card_init_w = 0;
static int32_t card_init_h = 0;

// 文字弹窗的输入框引用（用于 OK 回调）
static lv_obj_t * current_textarea = NULL;

// ==================== 工具函数 ====================

static void hide_dialog_anim_ready_cb(lv_anim_t * a) {
    (void)a;
    if (overlay != NULL) {
        lv_obj_del(overlay);
        overlay = NULL;
    }
    card = NULL;
}

static void show_anim_exec(lv_anim_t * a, int32_t v) {
    lv_obj_t * obj = a->var;
    if (obj != card) return;
    if (v <= 0) {
        // 动画刚开始，尺寸设为 1 避免为 0
        v = 1;
    }
    int32_t ratio = v * 1.0f / 255.0f;
    int32_t w = (int32_t)(card_init_w * ratio);
    int32_t h = (int32_t)(card_init_h * ratio);
    if (w <= 0) w = 1;
    if (h <= 0) h = 1;
    lv_obj_set_size(obj, w, h);
    // 居中：让中心坐标始终不变
    int32_t cx = UI_SCREEN_WIDTH / 2;
    int32_t cy = UI_SCREEN_HEIGHT / 2;
    lv_obj_set_pos(obj, cx - w / 2, cy - h / 2);
}

static void show_dialog_anim(void) {
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

static void hide_anim_exec(lv_anim_t * a, int32_t v) {
    lv_obj_t * obj = a->var;
    if (obj != card) return;
    if (v <= 0) {
        v = 1;
    }
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

static void hide_dialog(void) {
    if (card == NULL) return;

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, card);
    lv_anim_set_values(&a, 255, 0);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)hide_anim_exec);
    lv_anim_set_duration(&a, 150);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
    lv_anim_set_ready_cb(&a, hide_dialog_anim_ready_cb);
    lv_anim_start(&a);
}

// 点击遮罩层关闭
static void overlay_click_cb(lv_event_t * e) {
    (void)e;
    if (current_done_cb) {
        current_done_cb(current_user_data, "cancel");
        current_done_cb = NULL;
    }
    hide_dialog();
}

// 输入弹窗 OK 按钮回调
static void text_input_ok_cb(lv_event_t * e) {
    (void)e;
    if (current_done_cb && current_textarea != NULL) {
        const char * text = lv_textarea_get_text(current_textarea);
        current_done_cb(current_user_data, text);
        current_done_cb = NULL;
        current_textarea = NULL;
    }
    hide_dialog();
}

// 确认弹窗 OK 按钮回调
static void confirm_ok_cb(lv_event_t * e) {
    (void)e;
    if (current_done_cb) {
        current_done_cb(current_user_data, "ok");
        current_done_cb = NULL;
    }
    hide_dialog();
}

// ==================== 通用创建流程 ====================

static lv_obj_t * create_base_dialog(lv_obj_t * parent, const char * title) {
    if (overlay != NULL) return NULL; // 已有弹窗在显示

    lv_obj_t * root = (parent != NULL) ? parent : lv_layer_top();

    // 遮罩层
    overlay = lv_obj_create(root);
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
    lv_obj_set_size(card, 280, 160);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, COLOR_ITEM_BG, 0);
    lv_obj_set_style_radius(card, 12, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_pad_all(card, 20, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(card, 12, 0);

    // 标题
    lv_obj_t * title_label = lv_label_create(card);
    lv_label_set_text(title_label, title);
    lv_obj_set_style_text_font(title_label, &font, 0);
    lv_obj_set_style_text_color(title_label, COLOR_TEXT_MAIN, 0);
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(title_label, 240);

    return card; // 返回 card 作为内容容器
}

// 创建底部按钮行
static void create_dialog_buttons(lv_obj_t * parent, const char * ok_text, const char * cancel_text) {
    lv_obj_t * btn_row = lv_obj_create(parent);
    lv_obj_set_size(btn_row, LV_PCT(100), 40);
    lv_obj_set_style_bg_opa(btn_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_row, 0, 0);
    lv_obj_set_style_pad_all(btn_row, 0, 0);
    lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 取消按钮
    lv_obj_t * btn_cancel = lv_obj_create(btn_row);
    lv_obj_set_size(btn_cancel, 100, 38);
    lv_obj_set_style_bg_color(btn_cancel, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_radius(btn_cancel, 10, 0);
    lv_obj_add_flag(btn_cancel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(btn_cancel, overlay_click_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * lbl_cancel = lv_label_create(btn_cancel);
    lv_label_set_text(lbl_cancel, cancel_text);
    lv_obj_set_style_text_font(lbl_cancel, &font, 0);
    lv_obj_center(lbl_cancel);

    // 确认按钮
    lv_obj_t * btn_ok = lv_obj_create(btn_row);
    lv_obj_set_size(btn_ok, 100, 38);
    lv_obj_set_style_bg_color(btn_ok, COLOR_IOS_BLUE, 0);
    lv_obj_set_style_radius(btn_ok, 10, 0);
    lv_obj_add_flag(btn_ok, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t * lbl_ok = lv_label_create(btn_ok);
    lv_label_set_text(lbl_ok, ok_text);
    lv_obj_set_style_text_font(lbl_ok, &font, 0);
    lv_obj_center(lbl_ok);
}

// ==================== 文字输入弹窗 ====================

void ui_dialog_text_input_show(lv_obj_t * parent,
    const char * title,
    const char * placeholder,
    const char * initial_text,
    const char * ok_text,
    const char * cancel_text,
    ui_dialog_done_cb_t done_cb,
    void * user_data) {

    if (overlay != NULL) return;

    lv_obj_t * content = create_base_dialog(parent, title);
    current_done_cb = done_cb;
    current_user_data = user_data;

    // 输入框
    lv_obj_t * ta = lv_textarea_create(content);
    lv_obj_set_size(ta, LV_PCT(100), 40);
    lv_obj_set_style_text_font(ta, &font, 0);
    lv_obj_set_style_text_align(ta, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_bg_color(ta, lv_color_hex(0xF2F2F7), 0);
    lv_obj_set_style_radius(ta, 8, 0);
    lv_obj_set_style_border_width(ta, 0, 0);
    lv_obj_set_style_pad_all(ta, 10, 0);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_placeholder_text(ta, placeholder);

    if (initial_text) {
        lv_textarea_set_text(ta, initial_text);
    }

    // 保存当前 textarea 引用供 OK 回调使用
    current_textarea = ta;

    // 按钮行
    create_dialog_buttons(content, ok_text, cancel_text);

    // 确定按钮绑定回调（通过全局 current_textarea 获取输入）
    lv_obj_t * btns = lv_obj_get_child(content, 2);
    if (btns) {
        lv_obj_t * ok_btn = lv_obj_get_child(btns, 1);
        if (ok_btn) {
            lv_obj_add_event_cb(ok_btn, text_input_ok_cb, LV_EVENT_CLICKED, NULL);
        }
    }

    // 显示动画
    show_dialog_anim();
}

// ==================== 确认弹窗 ====================

void ui_dialog_confirm_show(lv_obj_t * parent,
    const char * title,
    const char * message,
    const char * ok_text,
    const char * cancel_text,
    ui_dialog_done_cb_t done_cb,
    void * user_data) {

    if (overlay != NULL) return;

    lv_obj_t * content = create_base_dialog(parent, title);
    current_done_cb = done_cb;
    current_user_data = user_data;

    // 消息文本
    lv_obj_t * msg_label = lv_label_create(content);
    lv_label_set_text(msg_label, message);
    lv_obj_set_style_text_font(msg_label, &font, 0);
    lv_obj_set_style_text_color(msg_label, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_align(msg_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(msg_label, 240);

    // 按钮行
    create_dialog_buttons(content, ok_text, cancel_text);

    // 确定按钮
    lv_obj_t * btns = lv_obj_get_child(content, 2);
    if (btns) {
        lv_obj_t * ok_btn = lv_obj_get_child(btns, 1);
        if (ok_btn) {
            lv_obj_add_event_cb(ok_btn, confirm_ok_cb, LV_EVENT_CLICKED, NULL);
        }
    }

    // 显示动画
    show_dialog_anim();
}
