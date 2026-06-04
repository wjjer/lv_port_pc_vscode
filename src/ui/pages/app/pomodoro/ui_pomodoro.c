/**
 * @file ui_pomodoro.c
 * @brief 番茄时钟应用 Presenter：组合 计时控制器 + 持久化 + 视图。
 *
 * 屏幕承载：lv_layer_top() 浮层（CONSTRAINTS.md §6）。
 * 状态栏：保留模式 —— show() 中 move_foreground（§7 模式 A）。
 * 生命周期：hide() 走 120ms 异步消隐 + 置 NULL（CONSTRAINTS.md §3 链路 4）。
 */

#include "ui_pomodoro.h"
#include "ui_pomodoro_timer.h"
#include "ui_pomodoro_view.h"
#include "../../../components/ui_titlebar.h"
#include "../../../components/ui_statusbar.h"
#include "../../../ui.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

/* ---- 应用级状态（全部 static，符号隔离 CONSTRAINTS.md §10） ---- */
static lv_obj_t *           pomodoro_page = NULL;  /* layer_top 浮层根 */
static lv_obj_t *           current_view  = NULL;  /* 当前子页内容根（主页/任务页） */
static lv_obj_t *           titlebar      = NULL;
static lv_timer_t *         tick_timer    = NULL;  /* 1s 驱动 */
static lv_obj_t *           task_input_modal = NULL;
static lv_obj_t *           task_input_ta    = NULL;

static ui_pomodoro_view_t   view;                  /* 主页控件句柄 */
static ui_pomodoro_timer_t  timer;                 /* 计时状态机 */
static ui_pomodoro_store_t  store;                 /* 任务与计数 */

static int32_t              current_task  = 0;     /* 当前选中任务索引 */
static bool                 on_main_page  = true;  /* 当前是否在主计时页 */

/* ============================================================
 * 提示音接口（硬件相关，收敛于此处 stub；移植时在 hal/ 实现并替换）
 * ============================================================ */
static void ui_pomodoro_play_alert(void)
{
    /* TODO(硬件端): 调用 hal 蜂鸣/音频接口，如 hal_audio_beep(); */
}

/* ============================================================
 * 取当前"日序号"（跨日今日计数归零用）
 * ============================================================ */
static int32_t today_day_index(void)
{
    time_t raw = time(NULL);
    struct tm * t = localtime(&raw);
    if(t == NULL) return 0;
    return ui_pomodoro_day_index(t->tm_year, t->tm_yday);
}

/* ============================================================
 * 拼装任务下拉选项串（LVGL dropdown 用 '\n' 分隔）
 * ============================================================ */
static void build_task_options(char * buf, size_t buf_size)
{
    buf[0] = '\0';
    if(store.task_count == 0) {
        strncpy(buf, "无任务", buf_size - 1);
        buf[buf_size - 1] = '\0';
        return;
    }
    size_t pos = 0;
    for(int32_t i = 0; i < store.task_count; i++) {
        int n = snprintf(buf + pos, buf_size - pos, "%s%s",
                         (i == 0) ? "" : "\n", store.tasks[i].name);
        if(n < 0 || (size_t)n >= buf_size - pos) break;
        pos += (size_t)n;
    }
}

/* ============================================================
 * 把当前 timer / store 状态完整刷到主页视图
 * ============================================================ */
static void refresh_main_view(void)
{
    if(!on_main_page) return;

    bool is_work = (timer.mode == UI_POMODORO_MODE_WORK);
    ui_pomodoro_view_set_mode(&view, ui_pomodoro_timer_mode_name(timer.mode), is_work);
    ui_pomodoro_view_set_time(&view,
                              ui_pomodoro_timer_mode_seconds(timer.mode),
                              timer.remaining);
    ui_pomodoro_view_set_running(&view, timer.running);
    ui_pomodoro_view_set_today(&view, store.today_count);

    char opts[UI_POMODORO_MAX_TASKS * UI_POMODORO_TASK_NAME_LEN];
    build_task_options(opts, sizeof(opts));
    int32_t tomato = (current_task >= 0 && current_task < store.task_count)
                         ? store.tasks[current_task].tomato_count : 0;
    ui_pomodoro_view_set_task(&view, opts, current_task, tomato);
}

/* ============================================================
 * 计时控制器"段完成"回调：写存储 + 提示音 + 浮窗
 * ============================================================ */
static void on_timer_event(ui_pomodoro_event_t ev, ui_pomodoro_mode_t next_mode, void * user_data)
{
    LV_UNUSED(next_mode);
    LV_UNUSED(user_data);

    if(ev == UI_POMODORO_EVENT_WORK_DONE) {
        /* 完成一个工作番茄：今日 +1，当前任务 +1，落盘 */
        ui_pomodoro_store_record_tomato(&store, current_task);
        ui_pomodoro_storage_save(&store, today_day_index());
    }

    ui_pomodoro_play_alert();
    ui_pomodoro_view_show_toast("时间到", 1500);
}

/* ============================================================
 * 1 秒 tick：推进控制器并刷新视图
 * ============================================================ */
static void tick_cb(lv_timer_t * tmr)
{
    LV_UNUSED(tmr);
    ui_pomodoro_timer_tick(&timer);
    refresh_main_view();
}

/* ============================================================
 * 子页路由：销毁旧内容根，渲染新页
 * ============================================================ */
static void clean_current_view(void)
{
    if(current_view && lv_obj_is_valid(current_view)) {
        lv_obj_delete(current_view);
    }
    current_view = NULL;
    /* 主页句柄随之失效，清零防野指针 */
    memset(&view, 0, sizeof(view));
}

static void titlebar_back_cb(lv_event_t * e);

void ui_pomodoro_route_to_main(void)
{
    clean_current_view();
    on_main_page = true;

    if(titlebar && lv_obj_is_valid(titlebar)) {
        lv_obj_delete(titlebar);
        titlebar = NULL;
    }
    titlebar = ui_titlebar_create(
        pomodoro_page, "番茄时钟",
        titlebar_back_cb, NULL,
        NULL, NULL, NULL,
        NULL, NULL, false);

    current_view = ui_pomodoro_view_create_main(pomodoro_page, &view);
    refresh_main_view();
}

static void ui_pomodoro_close_task_input(void)
{
    task_input_ta = NULL;
    if(task_input_modal && lv_obj_is_valid(task_input_modal)) {
        lv_obj_delete(task_input_modal);
    }
    task_input_modal = NULL;
}

static void task_input_cancel_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    ui_pomodoro_close_task_input();
}

static void task_input_confirm_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    if(task_input_ta && lv_obj_is_valid(task_input_ta)) {
        const char * name = lv_textarea_get_text(task_input_ta);
        ui_pomodoro_action_add_task(name);
    }
    ui_pomodoro_close_task_input();
}

static void ui_pomodoro_open_task_input(void)
{
    ui_pomodoro_close_task_input();

    task_input_modal = lv_obj_create(lv_layer_top());
    lv_obj_set_size(task_input_modal, 260, 148);
    lv_obj_center(task_input_modal);
    lv_obj_set_style_bg_color(task_input_modal, lv_color_hex(0xFAFAFC), 0);
    lv_obj_set_style_bg_opa(task_input_modal, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(task_input_modal, 12, 0);
    lv_obj_set_style_border_width(task_input_modal, 1, 0);
    lv_obj_set_style_border_color(task_input_modal, lv_color_hex(0xE5E5EA), 0);
    lv_obj_set_style_shadow_width(task_input_modal, 18, 0);
    lv_obj_set_style_shadow_opa(task_input_modal, LV_OPA_20, 0);
    lv_obj_set_style_pad_all(task_input_modal, 12, 0);
    lv_obj_remove_flag(task_input_modal, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * title = lv_label_create(task_input_modal);
    lv_label_set_text(title, "新增任务");
    lv_obj_set_style_text_font(title, &font, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x111111), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 0);

    task_input_ta = lv_textarea_create(task_input_modal);
    lv_obj_set_size(task_input_ta, 226, 38);
    lv_obj_align(task_input_ta, LV_ALIGN_TOP_MID, 0, 30);
    lv_textarea_set_one_line(task_input_ta, true);
    lv_textarea_set_max_length(task_input_ta, UI_POMODORO_TASK_NAME_LEN - 1);
    lv_textarea_set_placeholder_text(task_input_ta, "输入任务名称");
    lv_obj_set_style_text_font(task_input_ta, &font, 0);
    lv_obj_set_style_radius(task_input_ta, 10, 0);

    lv_obj_t * cancel = lv_button_create(task_input_modal);
    lv_obj_set_size(cancel, 94, 34);
    lv_obj_align(cancel, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_radius(cancel, 10, 0);
    lv_obj_set_style_bg_color(cancel, lv_color_white(), 0);
    lv_obj_set_style_border_width(cancel, 1, 0);
    lv_obj_set_style_border_color(cancel, lv_color_hex(0xE5E5EA), 0);
    lv_obj_set_style_shadow_width(cancel, 0, 0);
    lv_obj_add_event_cb(cancel, task_input_cancel_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * cancel_lbl = lv_label_create(cancel);
    lv_label_set_text(cancel_lbl, "取消");
    lv_obj_set_style_text_font(cancel_lbl, &font, 0);
    lv_obj_set_style_text_color(cancel_lbl, lv_color_hex(0x111111), 0);
    lv_obj_center(cancel_lbl);

    lv_obj_t * confirm = lv_button_create(task_input_modal);
    lv_obj_set_size(confirm, 94, 34);
    lv_obj_align(confirm, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_style_radius(confirm, 10, 0);
    lv_obj_set_style_bg_color(confirm, lv_color_hex(0x007AFF), 0);
    lv_obj_set_style_shadow_width(confirm, 0, 0);
    lv_obj_add_event_cb(confirm, task_input_confirm_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * confirm_lbl = lv_label_create(confirm);
    lv_label_set_text(confirm_lbl, "确认");
    lv_obj_set_style_text_font(confirm_lbl, &font, 0);
    lv_obj_set_style_text_color(confirm_lbl, lv_color_white(), 0);
    lv_obj_center(confirm_lbl);
}

/* 任务页"添加"按钮回调 */
static void tasks_add_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    ui_pomodoro_open_task_input();
}

void ui_pomodoro_route_to_tasks(void)
{
    clean_current_view();
    on_main_page = false;

    if(titlebar && lv_obj_is_valid(titlebar)) {
        lv_obj_delete(titlebar);
        titlebar = NULL;
    }
    titlebar = ui_titlebar_create(
        pomodoro_page, "任务管理",
        titlebar_back_cb, NULL,
        LV_SYMBOL_PLUS, tasks_add_cb, NULL,
        NULL, NULL, false);

    current_view = ui_pomodoro_view_create_tasks(pomodoro_page);
}

static void titlebar_back_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    if(on_main_page) {
        ui_pomodoro_hide();
    } else {
        ui_pomodoro_route_to_main();
    }
}

/* ============================================================
 * 业务动作（View 回调）
 * ============================================================ */
void ui_pomodoro_action_start_pause(void)
{
    if(timer.running) {
        ui_pomodoro_timer_pause(&timer);
    } else {
        ui_pomodoro_timer_start(&timer);
    }
    refresh_main_view();
}

void ui_pomodoro_action_reset(void)
{
    ui_pomodoro_timer_reset(&timer);
    refresh_main_view();
}

void ui_pomodoro_action_toggle_mode(void)
{
    ui_pomodoro_timer_toggle_mode(&timer);
    refresh_main_view();
}

void ui_pomodoro_action_select_task(int32_t idx)
{
    if(idx < 0) idx = 0;
    if(idx >= store.task_count) idx = store.task_count - 1;
    current_task = idx;
    refresh_main_view();
}

void ui_pomodoro_action_add_task(const char * name)
{
    if(ui_pomodoro_store_add_task(&store, name) >= 0) {
        ui_pomodoro_storage_save(&store, today_day_index());
        if(!on_main_page) {
            ui_pomodoro_route_to_tasks(); /* 重渲染列表 */
        }
    }
}

void ui_pomodoro_action_delete_task(int32_t idx)
{
    if(ui_pomodoro_store_delete_task(&store, idx)) {
        /* 维护当前选中索引 */
        if(current_task >= store.task_count) {
            current_task = store.task_count > 0 ? store.task_count - 1 : 0;
        }
        ui_pomodoro_storage_save(&store, today_day_index());
        if(!on_main_page) {
            ui_pomodoro_route_to_tasks();
        }
    }
}

void ui_pomodoro_action_toggle_task(int32_t idx)
{
    if(ui_pomodoro_store_toggle_task(&store, idx)) {
        ui_pomodoro_storage_save(&store, today_day_index());
        if(!on_main_page) {
            ui_pomodoro_route_to_tasks();
        }
    }
}

/* ============================================================
 * 数据读取
 * ============================================================ */
const ui_pomodoro_store_t * ui_pomodoro_get_store(void)
{
    return &store;
}

int32_t ui_pomodoro_get_current_task(void)
{
    return current_task;
}

#ifdef UI_POMODORO_TEST
lv_obj_t * ui_pomodoro_test_get_page(void)
{
    return pomodoro_page;
}

lv_timer_t * ui_pomodoro_test_get_tick_timer(void)
{
    return tick_timer;
}

lv_obj_t * ui_pomodoro_test_get_task_input(void)
{
    return task_input_modal;
}

void ui_pomodoro_test_submit_task_name(const char * name)
{
    ui_pomodoro_open_task_input();
    if(task_input_ta && lv_obj_is_valid(task_input_ta)) {
        lv_textarea_set_text(task_input_ta, name ? name : "");
    }
    ui_pomodoro_action_add_task(lv_textarea_get_text(task_input_ta));
    ui_pomodoro_close_task_input();
}
#endif

/* ============================================================
 * 生命周期
 * ============================================================ */
void ui_pomodoro_show(void)
{
    ui_pomodoro_hide(); /* 入口自清理（CONSTRAINTS.md §5） */

    /* 控制器与数据初始化 */
    ui_pomodoro_timer_init(&timer);
    ui_pomodoro_timer_set_cb(&timer, on_timer_event, NULL);
    ui_pomodoro_storage_load(&store, today_day_index());
    current_task = 0;
    on_main_page = true;

    /* layer_top 浮层根 */
    pomodoro_page = lv_obj_create(lv_layer_top());
    lv_obj_set_size(pomodoro_page, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(pomodoro_page, lv_color_hex(0xF5F5F7), 0);
    lv_obj_set_style_pad_all(pomodoro_page, 0, 0);
    lv_obj_set_style_border_width(pomodoro_page, 0, 0);
    lv_obj_set_style_radius(pomodoro_page, 0, 0);
    lv_obj_set_scrollbar_mode(pomodoro_page, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(pomodoro_page, LV_OBJ_FLAG_SCROLLABLE);

    /* 保留状态栏：顶到浮层之上（§7 模式 A） */
    if(status_bar) lv_obj_move_foreground(status_bar);

    /* 主页 */
    ui_pomodoro_route_to_main();

    /* 1 秒驱动 */
    tick_timer = lv_timer_create(tick_cb, 1000, NULL);

    lv_obj_fade_in(pomodoro_page, 180, 0);
}

void ui_pomodoro_hide(void)
{
    /* 先停驱动，避免 tick 触到正在销毁的视图 */
    if(tick_timer) {
        lv_timer_delete(tick_timer);
        tick_timer = NULL;
    }

    ui_pomodoro_close_task_input();

    /* 句柄先失效，再异步销毁根（120ms 消隐后置 NULL） */
    titlebar     = NULL;
    current_view = NULL;
    memset(&view, 0, sizeof(view));

    if(pomodoro_page && lv_obj_is_valid(pomodoro_page)) {
        lv_obj_fade_out(pomodoro_page, 100, 0);
        lv_obj_delete_delayed(pomodoro_page, 120);
    }
    pomodoro_page = NULL;
}
