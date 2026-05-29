#include "ui_poetry.h"
#include "ui_poetry_view.h"

// 实体定义：全局核心容器
lv_obj_t *app_screen = NULL;
lv_obj_t *current_page = NULL;

// 实体定义：公共样式
lv_style_t style_btn_classic;
static bool style_initialized = false;

// 内部业务状态
static bool is_playing = false;

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
    ui_poetry_view_render_list(stage_title);
}

void ui_poetry_route_to_detail(void) {
    clean_current_page();
    is_playing = false; // 进入详情页重置播放状态

    // 实际项目中，这里的数据可以从 Model（数据库/数组）中动态读取
    const char *title = "静夜思";
    const char *author = "【唐】李白";
    const char *content = "床前明月光，\n疑是地上霜。\n举头望明月，\n低头思故乡。";
    const char *notes = "【注释】\n1. 静夜思：静静的深夜产生的思念之情。\n2. 床前：一说指井栏，一说指窗前。\n\n【赏析】\n这首诗写的是游子孤身在外的深夜思乡之情，语言质朴，流传千古。";

    ui_poetry_view_render_detail(title, author, content, notes);
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

// ==========================================
// 外部框架生命周期接口
// ==========================================

void ui_poetry_show(void) {
    ui_poetry_hide(); // 安全清理
    app_screen = lv_obj_create(NULL); // 创建独立专用的 Screen
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
