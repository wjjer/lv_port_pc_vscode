#include "ui_poetry.h"
#include "ui_poetry_view.h"
#include "ui_poetry_data.h"
#include <string.h>

// 实体定义：全局核心容器
lv_obj_t *app_screen = NULL;
lv_obj_t *current_page = NULL;

// 实体定义：公共样式
lv_style_t style_btn_classic;
static bool style_initialized = false;

// 内部业务状态
static bool is_playing = false;
static uint8_t current_stage = 0;
static uint16_t current_poem_index = 0;

// 初始化应用特有样式
static void ui_poetry_init_styles(void) {
    if (style_initialized) return;

    lv_style_init(&style_btn_classic);
    lv_style_set_bg_color(&style_btn_classic, lv_color_hex(0x2C4A3E)); // 远山黛绿
    lv_style_set_radius(&style_btn_classic, 8);
    lv_style_set_border_width(&style_btn_classic, 0);
    lv_style_set_shadow_width(&style_btn_classic, 0);

    style_initialized = true;
}

// 路由控制：销毁旧页面
static void clean_current_page(void) {
    if (current_page != NULL) {
        lv_obj_delete(current_page);
        current_page = NULL;
    }
}

// ==========================================
// 核心路由与页面切换控制 (Presenter 角色)
// ==========================================

void ui_poetry_route_to_home(void) {
    clean_current_page();
    ui_poetry_init_styles(); // 确保样式加载
    ui_poetry_view_render_home();
}

void ui_poetry_route_to_list(const char *stage_title) {
    clean_current_page();

    // 确定阶段索引
    if (strstr(stage_title, "小学")) {
        current_stage = 0;
    } else if (strstr(stage_title, "初中")) {
        current_stage = 1;
    } else if (strstr(stage_title, "高中")) {
        current_stage = 2;
    }

    ui_poetry_view_render_list(stage_title, current_stage);
}

void ui_poetry_route_to_detail(void) {
    clean_current_page();
    is_playing = false;

    // 从数据库获取当前诗词
    poem_t *poem = ui_poetry_get_stage_poem(current_stage, current_poem_index);
    if (poem == NULL) {
        // 降级到第一首诗
        poem = ui_poetry_get_stage_poem(current_stage, 0);
    }

    if (poem) {
        ui_poetry_view_render_detail(poem->title, poem->author, poem->content, poem->notes);
    }
}

// ==========================================
// 业务逻辑控制
// ==========================================

void ui_poetry_toggle_play(lv_obj_t *btn_label) {
    is_playing = !is_playing;
    if (is_playing) {
        lv_label_set_text(btn_label, " " LV_SYMBOL_PAUSE " ");
        // TODO: 在这里调用你的音频底层播放接口，如: audio_play("jingyesi.mp3");
    } else {
        lv_label_set_text(btn_label, " " LV_SYMBOL_PLAY " ");
        // TODO: 在这里调用你的音频底层暂停接口，如: audio_pause();
    }
}

void ui_poetry_toggle_notes(lv_obj_t *notes_panel) {
    if (lv_obj_has_flag(notes_panel, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_remove_flag(notes_panel, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(notes_panel, LV_OBJ_FLAG_HIDDEN);
    }
}

void ui_poetry_set_current_poem_index(uint16_t index) {
    current_poem_index = index;
}

// ==========================================
// 外部框架生命周期接口
// ==========================================

void ui_poetry_show(void) {
    ui_poetry_hide();
    ui_poetry_data_init(); // 初始化诗词数据
    app_screen = lv_obj_create(NULL);
    ui_poetry_route_to_home();
    lv_screen_load(app_screen);
}

void ui_poetry_hide(void) {
    if (app_screen != NULL) {
        lv_obj_delete(app_screen);
        app_screen = NULL;
        current_page = NULL;
        is_playing = false;
    }
}
