#ifndef UI_POMODORO_H
#define UI_POMODORO_H

/**
 * @file ui_pomodoro.h
 * @brief 番茄时钟应用（Presenter）对外接口。
 *
 * 仅暴露标准生命周期对 + 供 View 回调的业务动作 + 数据读取接口。
 * 内部组合三块：
 *   - ui_pomodoro_timer  : 纯逻辑计时状态机（项 1.2）
 *   - ui_pomodoro_storage: 任务与今日计数持久化（项 1.3）
 *   - ui_pomodoro_view   : 纯渲染（项 1.1）
 */

#include "lvgl.h"
#include "ui_pomodoro_storage.h"

#ifdef __cplusplus
extern "C" {
#endif

/* —— 标准生命周期（CONSTRAINTS.md §5），由 Launcher(ui.c) 分发调用 —— */
void ui_pomodoro_show(void);
void ui_pomodoro_hide(void);

/* —— 页面路由 —— */
void ui_pomodoro_route_to_main(void);   /* 主计时页 */
void ui_pomodoro_route_to_tasks(void);  /* 任务列表页 */

/* —— View 回调的业务动作 —— */
void ui_pomodoro_action_start_pause(void);        /* 开始/暂停 */
void ui_pomodoro_action_reset(void);              /* 重置当前模式 */
void ui_pomodoro_action_toggle_mode(void);        /* 工作/休息手动切换 */
void ui_pomodoro_action_select_task(int32_t idx); /* 选中当前任务 */
void ui_pomodoro_action_add_task(const char * name);  /* 新增任务 */
void ui_pomodoro_action_delete_task(int32_t idx);     /* 删除任务 */
void ui_pomodoro_action_toggle_task(int32_t idx);     /* 勾选/取消完成 */

/* —— 数据读取（供任务页渲染） —— */
const ui_pomodoro_store_t * ui_pomodoro_get_store(void);
int32_t ui_pomodoro_get_current_task(void);

#ifdef UI_POMODORO_TEST
lv_obj_t * ui_pomodoro_test_get_page(void);
lv_timer_t * ui_pomodoro_test_get_tick_timer(void);
lv_obj_t * ui_pomodoro_test_get_task_input(void);
void ui_pomodoro_test_submit_task_name(const char * name);
#endif

#ifdef __cplusplus
}
#endif

#endif /* UI_POMODORO_H */
