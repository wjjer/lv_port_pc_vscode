#ifndef UI_POMODORO_VIEW_H
#define UI_POMODORO_VIEW_H

/**
 * @file ui_pomodoro_view.h
 * @brief 番茄时钟视图层（纯渲染）。Presenter(ui_pomodoro.c) 负责状态与业务，
 *        View 只负责把状态画出来、并把用户事件回调给 Presenter。
 *
 * 字体红线（CONSTRAINTS.md §8）：项目中文字库 `&font` 仅含 CJK（无数字/ASCII），
 * 因此所有"数字"一律用 montserrat，所有"中文"用 &font，二者分开成不同 label。
 */

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_font_t font;

/* 主计时页面的控件句柄集合（Presenter 持有，刷新时按需更新） */
typedef struct {
    lv_obj_t * root;        /* 主页面内容根容器 */
    lv_obj_t * mode_label;  /* 模式标识："工作中" / "短休息" / "长休息" */
    lv_obj_t * arc;         /* 倒计时进度环 */
    lv_obj_t * time_label;  /* 中央大号倒计时 MM:SS（montserrat） */
    lv_obj_t * btn_start;   /* 开始/暂停 按钮 */
    lv_obj_t * lbl_start;   /* 开始/暂停 按钮文字 */
    lv_obj_t * btn_reset;   /* 重置 按钮 */
    lv_obj_t * btn_toggle;  /* 工作/休息 手动切换按钮 */
    lv_obj_t * today_num;   /* 今日番茄数字（montserrat） */
    lv_obj_t * task_dd;     /* 当前任务下拉框 */
    lv_obj_t * task_tomato; /* 当前任务累计番茄数字（montserrat） */
} ui_pomodoro_view_t;

/**
 * @brief 在 parent 下创建主计时页面，所有句柄写入 *v。
 *        按钮/下拉事件在内部接到 Presenter 的 ui_pomodoro_action_* 上。
 * @return 内容根容器（同 v->root）
 */
lv_obj_t * ui_pomodoro_view_create_main(lv_obj_t * parent, ui_pomodoro_view_t * v);

/* —— 主页面刷新接口（Presenter 调用，内部均判空判有效，安全幂等）—— */
void ui_pomodoro_view_set_mode(ui_pomodoro_view_t * v, const char * mode_name, bool is_work);
void ui_pomodoro_view_set_time(ui_pomodoro_view_t * v, int32_t total_seconds, int32_t remaining_seconds);
void ui_pomodoro_view_set_running(ui_pomodoro_view_t * v, bool running);
void ui_pomodoro_view_set_today(ui_pomodoro_view_t * v, int32_t today_count);
void ui_pomodoro_view_set_task(ui_pomodoro_view_t * v, const char * options,
                               int32_t selected, int32_t tomato_count);

/**
 * @brief 在 parent 下创建独立任务列表页面（增/删/勾选完成）。
 *        通过 ui_pomodoro_get_store() 读数据，操作回调到 ui_pomodoro_action_*。
 * @return 任务页内容根容器
 */
lv_obj_t * ui_pomodoro_view_create_tasks(lv_obj_t * parent);

/**
 * @brief 在 layer_top 弹出"时间到"轻提示浮窗，duration_ms 后自动消失。
 */
void ui_pomodoro_view_show_toast(const char * text, uint32_t duration_ms);

#ifdef __cplusplus
}
#endif

#endif /* UI_POMODORO_VIEW_H */
