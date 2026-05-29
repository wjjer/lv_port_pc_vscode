#include "ui_calculator.h"
#include "../../../components/ui_titlebar.h"
#include "../../../ui.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// 颜色定义 (经典 iOS 科技风)
#define COLOR_BG            lv_color_hex(0x1C1C1E)
#define COLOR_DISPLAY_BG    lv_color_hex(0x1C1C1E)
#define COLOR_TEXT_MAIN     lv_color_white()
#define COLOR_TEXT_SECOND   lv_color_hex(0xA5A5A5)

#define COLOR_NUM_BG        lv_color_hex(0x333333)
#define COLOR_NUM_PRESSED   lv_color_hex(0x555555)
#define COLOR_OP_BG         lv_color_hex(0xFF9F0A)
#define COLOR_OP_PRESSED    lv_color_hex(0xFFB340)
#define COLOR_FUNC_BG       lv_color_hex(0x505050)
#define COLOR_FUNC_PRESSED  lv_color_hex(0x7C7C7C)

// 针对小屏幕优化字体层级
#define DISPLAY_FONT        &lv_font_montserrat_20
#define RESULT_FONT         &lv_font_montserrat_14

static lv_obj_t * calc_page;
static lv_obj_t * display_label; // 合并为一个主显示标签，防止重叠

// 计算器内部逻辑状态机
typedef struct {
    double current_value;
    double previous_value;
    char operator;
    bool waiting_for_operand;
    bool just_calculated;
} calc_state_t;

static calc_state_t state = {0};
static char display_text[32] = "0";

// 刷新显示屏内容
static void update_display(const char * override_text) {
    if (override_text != NULL) {
        lv_label_set_text(display_label, override_text);
    } else {
        lv_label_set_text(display_label, display_text);
    }
}

static void input_digit(int digit) {
    if (state.waiting_for_operand || state.just_calculated) {
        snprintf(display_text, sizeof(display_text), "%d", digit);
        state.waiting_for_operand = false;
        state.just_calculated = false;
    } else {
        if (strlen(display_text) < 10) { // 限制长度，防止小屏幕字符滑出边界
            if (strcmp(display_text, "0") == 0) {
                snprintf(display_text, sizeof(display_text), "%d", digit);
            } else {
                size_t len = strlen(display_text);
                snprintf(display_text + len, sizeof(display_text) - len, "%d", digit);
            }
        }
    }
    update_display(NULL);
}

static void input_decimal(void) {
    if (state.waiting_for_operand || state.just_calculated) {
        snprintf(display_text, sizeof(display_text), "0.");
        state.waiting_for_operand = false;
        state.just_calculated = false;
    } else {
        if (strchr(display_text, '.') == NULL && strlen(display_text) < 10) {
            size_t len = strlen(display_text);
            snprintf(display_text + len, sizeof(display_text) - len, ".");
        }
    }
    update_display(NULL);
}

static double get_display_value(void) {
    return atof(display_text);
}

static double calculate(double a, double b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return (b != 0) ? a / b : NAN;
        default: return b;
    }
}

static void input_operator(char op) {
    double current = get_display_value();

    if (state.operator != 0 && !state.waiting_for_operand) {
        double result = calculate(state.previous_value, current, state.operator);
        if (isnan(result)) {
            snprintf(display_text, sizeof(display_text), "Error");
            state.operator = 0;
            state.waiting_for_operand = true;
            update_display(NULL);
            return;
        }
        state.previous_value = result;
        snprintf(display_text, sizeof(display_text), "%.8g", result);
    } else {
        state.previous_value = current;
    }

    state.operator = op;
    state.waiting_for_operand = true;
    state.just_calculated = false;

    // 在显示屏上直观展现临时运算公式（例如 "12 /"）
    char temp_buf[32];
    snprintf(temp_buf, sizeof(temp_buf), "%.6g %c", state.previous_value, op);
    update_display(temp_buf);
}

static void input_equals(void) {
    if (state.operator == 0) return;

    double current = get_display_value();
    double result = calculate(state.previous_value, current, state.operator);

    if (isnan(result)) {
        snprintf(display_text, sizeof(display_text), "Error");
    } else {
        snprintf(display_text, sizeof(display_text), "%.8g", result);
        state.current_value = result;
    }

    state.operator = 0;
    state.waiting_for_operand = true;
    state.just_calculated = true;
    update_display(NULL);
}

static void clear_all(void) {
    memset(&state, 0, sizeof(state));
    snprintf(display_text, sizeof(display_text), "0");
    update_display(NULL);
}

static void toggle_sign(void) {
    double val = get_display_value();
    val = -val;
    if (fabs(val) < 1e-10) val = 0;
    snprintf(display_text, sizeof(display_text), "%.8g", val);
    update_display(NULL);
}

static void percentage(void) {
    double val = get_display_value();
    val = val / 100.0;
    snprintf(display_text, sizeof(display_text), "%.8g", val);
    update_display(NULL);
}

static void backspace(void) {
    if (state.waiting_for_operand || state.just_calculated) return;
    size_t len = strlen(display_text);
    if (len > 1) {
        display_text[len - 1] = '\0';
        if (strcmp(display_text, "-") == 0) {
            snprintf(display_text, sizeof(display_text), "0");
        }
    } else {
        snprintf(display_text, sizeof(display_text), "0");
    }
    update_display(NULL);
}

static void btn_event_cb(lv_event_t * e) {
    int id = (int)(uintptr_t)lv_event_get_user_data(e);
    switch (id) {
        case 0:  clear_all(); break;
        case 1:  toggle_sign(); break;
        case 2:  percentage(); break;
        case 3:  input_operator('/'); break;
        case 4:  backspace(); break;
        case 5:  input_digit(7); break;
        case 6:  input_digit(8); break;
        case 7:  input_digit(9); break;
        case 8:  input_operator('*'); break;
        case 9:  input_digit(4); break;
        case 10: input_digit(5); break;
        case 11: input_digit(6); break;
        case 12: input_operator('-'); break;
        case 13: input_digit(1); break;
        case 14: input_digit(2); break;
        case 15: input_digit(3); break;
        case 16: input_operator('+'); break;
        case 17: input_digit(0); break;
        case 18: input_decimal(); break;
        case 19: input_equals(); break;
    }
}

// 📐 重新调整 240 高度下的像素分配矩阵
static void calc_btn_dimensions(int32_t * btn_w, int32_t * btn_h, int32_t * pad, int32_t * left_pad) {
    *pad = 4;       // 列横向间距
    *left_pad = 6;  // 左边边距
    *btn_w = (UI_SCREEN_WIDTH - 2 * (*left_pad) - 4 * (*pad)) / 5; // 精确横向缩放
    *btn_h = 28;    // 🚀 将原本的 32 压缩到 28 像素，为底部留出足够空间
}

static lv_obj_t * create_calc_btn_ex(lv_obj_t * parent, int id, const char * text,
                                     int row, int col,
                                     lv_color_t bg_color, lv_color_t press_color) {
    int32_t btn_w, btn_h, pad, left_pad;
    calc_btn_dimensions(&btn_w, &btn_h, &pad, &left_pad);

    int32_t display_h = 32;
    int32_t btn_start_y = ui_get_content_y() + display_h + 4;

    lv_obj_t * btn = lv_button_create(parent);
    lv_obj_set_size(btn, btn_w, btn_h);


    lv_obj_set_pos(btn, left_pad + col * (btn_w + pad), btn_start_y + row * (btn_h + 3));

    lv_obj_set_style_bg_color(btn, bg_color, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn, press_color, LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn, 6, 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);

    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, COLOR_TEXT_MAIN, 0);
    lv_obj_set_style_text_font(label, DISPLAY_FONT, 0);
    lv_obj_center(label);

    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)id);
    return btn;
}

void ui_calculator_show(void) {
    if (calc_page != NULL) {
        lv_obj_del(calc_page);
    }

    calc_page = lv_obj_create(lv_layer_top());
    lv_obj_set_size(calc_page, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(calc_page, COLOR_BG, 0);
    lv_obj_set_style_pad_all(calc_page, 0, 0);
    lv_obj_set_style_border_width(calc_page, 0, 0);
    lv_obj_clear_flag(calc_page, LV_OBJ_FLAG_SCROLLABLE);

    if(status_bar) lv_obj_add_flag(status_bar, LV_OBJ_FLAG_HIDDEN);

    ui_titlebar_create(calc_page, "计算器", (lv_event_cb_t)ui_calculator_hide, NULL, NULL, NULL, NULL, NULL, NULL, true);

    int32_t display_h = 32;
    lv_obj_t * display_bg = lv_obj_create(calc_page);
    lv_obj_set_size(display_bg, UI_SCREEN_WIDTH, display_h);
    lv_obj_set_pos(display_bg, 0, ui_get_content_y());
    lv_obj_set_style_bg_color(display_bg, COLOR_DISPLAY_BG, 0);
    lv_obj_set_style_border_width(display_bg, 0, 0);
    lv_obj_clear_flag(display_bg, LV_OBJ_FLAG_SCROLLABLE);

    display_label = lv_label_create(display_bg);
    lv_label_set_text(display_label, "0");
    lv_obj_set_style_text_color(display_label, COLOR_TEXT_MAIN, 0);
    lv_obj_set_style_text_font(display_label, DISPLAY_FONT, 0);
    lv_obj_align(display_label, LV_ALIGN_RIGHT_MID, -12, 0);

    clear_all();

    // 行 0
    create_calc_btn_ex(calc_page, 0, "C",   0, 0, COLOR_FUNC_BG, COLOR_FUNC_PRESSED);
    create_calc_btn_ex(calc_page, 1, "+/-", 0, 1, COLOR_FUNC_BG, COLOR_FUNC_PRESSED);
    create_calc_btn_ex(calc_page, 2, "%",   0, 2, COLOR_FUNC_BG, COLOR_FUNC_PRESSED);
    create_calc_btn_ex(calc_page, 3, "/",   0, 3, COLOR_OP_BG,   COLOR_OP_PRESSED);
    create_calc_btn_ex(calc_page, 4, "DEL", 0, 4, COLOR_OP_BG,   COLOR_OP_PRESSED);

    // 行 1
    create_calc_btn_ex(calc_page, 5, "7",   1, 0, COLOR_NUM_BG,  COLOR_NUM_PRESSED);
    create_calc_btn_ex(calc_page, 6, "8",   1, 1, COLOR_NUM_BG,  COLOR_NUM_PRESSED);
    create_calc_btn_ex(calc_page, 7, "9",   1, 2, COLOR_NUM_BG,  COLOR_NUM_PRESSED);
    create_calc_btn_ex(calc_page, 8, "*",   1, 3, COLOR_OP_BG,   COLOR_OP_PRESSED);
    create_calc_btn_ex(calc_page, 12, "-",  1, 4, COLOR_OP_BG,   COLOR_OP_PRESSED);

    // 行 2
    create_calc_btn_ex(calc_page, 9, "4",   2, 0, COLOR_NUM_BG,  COLOR_NUM_PRESSED);
    create_calc_btn_ex(calc_page, 10,"5",   2, 1, COLOR_NUM_BG,  COLOR_NUM_PRESSED);
    create_calc_btn_ex(calc_page, 11,"6",   2, 2, COLOR_NUM_BG,  COLOR_NUM_PRESSED);
    create_calc_btn_ex(calc_page, 16,"+",   2, 3, COLOR_OP_BG,   COLOR_OP_PRESSED);
    create_calc_btn_ex(calc_page, 19,"=",   2, 4, COLOR_OP_BG,   COLOR_OP_PRESSED);

    // 行 3
    create_calc_btn_ex(calc_page, 13,"1",   3, 0, COLOR_NUM_BG,  COLOR_NUM_PRESSED);
    create_calc_btn_ex(calc_page, 14,"2",   3, 1, COLOR_NUM_BG,  COLOR_NUM_PRESSED);
    create_calc_btn_ex(calc_page, 15,"3",   3, 2, COLOR_NUM_BG,  COLOR_NUM_PRESSED);
    create_calc_btn_ex(calc_page, 17,"0",   3, 3, COLOR_NUM_BG,  COLOR_NUM_PRESSED);
    create_calc_btn_ex(calc_page, 18,".",   3, 4, COLOR_NUM_BG,  COLOR_NUM_PRESSED);
}

void ui_calculator_hide(void) {
    if (calc_page != NULL) {
        // 退出计算器时恢复系统状态栏可见性
        if(status_bar) lv_obj_clear_flag(status_bar, LV_OBJ_FLAG_HIDDEN);
        lv_obj_del(calc_page);
        calc_page = NULL;
    }
}
