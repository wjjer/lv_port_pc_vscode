/**
 * @file ui_pomodoro_timer.c
 * @brief 番茄时钟核心计时控制器实现（纯逻辑，零 LVGL 依赖）
 */

#include "ui_pomodoro_timer.h"
#include <stddef.h>

int32_t ui_pomodoro_timer_mode_seconds(ui_pomodoro_mode_t mode)
{
    switch(mode) {
        case UI_POMODORO_MODE_WORK:        return UI_POMODORO_WORK_SECONDS;
        case UI_POMODORO_MODE_SHORT_BREAK: return UI_POMODORO_SHORT_BREAK_SECONDS;
        case UI_POMODORO_MODE_LONG_BREAK:  return UI_POMODORO_LONG_BREAK_SECONDS;
        default:                           return UI_POMODORO_WORK_SECONDS;
    }
}

const char * ui_pomodoro_timer_mode_name(ui_pomodoro_mode_t mode)
{
    switch(mode) {
        case UI_POMODORO_MODE_WORK:        return "工作中";
        case UI_POMODORO_MODE_SHORT_BREAK: return "短休息";
        case UI_POMODORO_MODE_LONG_BREAK:  return "长休息";
        default:                           return "工作中";
    }
}

void ui_pomodoro_timer_init(ui_pomodoro_timer_t * t)
{
    if(t == NULL) return;
    t->mode           = UI_POMODORO_MODE_WORK;
    t->remaining      = UI_POMODORO_WORK_SECONDS;
    t->running        = false;
    t->completed_work = 0;
    t->cb             = NULL;
    t->cb_user_data   = NULL;
}

void ui_pomodoro_timer_set_cb(ui_pomodoro_timer_t * t,
                              ui_pomodoro_timer_cb_t cb, void * user_data)
{
    if(t == NULL) return;
    t->cb           = cb;
    t->cb_user_data = user_data;
}

void ui_pomodoro_timer_start(ui_pomodoro_timer_t * t)
{
    if(t == NULL) return;
    if(t->remaining <= 0) {
        /* 已归零的段重新开始 → 装填当前模式默认时长 */
        t->remaining = ui_pomodoro_timer_mode_seconds(t->mode);
    }
    t->running = true;
}

void ui_pomodoro_timer_pause(ui_pomodoro_timer_t * t)
{
    if(t == NULL) return;
    t->running = false;
}

void ui_pomodoro_timer_reset(ui_pomodoro_timer_t * t)
{
    if(t == NULL) return;
    t->remaining = ui_pomodoro_timer_mode_seconds(t->mode);
    t->running   = false;
}

void ui_pomodoro_timer_toggle_mode(ui_pomodoro_timer_t * t)
{
    if(t == NULL) return;
    if(t->mode == UI_POMODORO_MODE_WORK) {
        t->mode = UI_POMODORO_MODE_SHORT_BREAK;
    } else {
        t->mode = UI_POMODORO_MODE_WORK;
    }
    t->remaining = ui_pomodoro_timer_mode_seconds(t->mode);
    t->running   = false;
}

void ui_pomodoro_timer_tick(ui_pomodoro_timer_t * t)
{
    if(t == NULL || !t->running) return;

    if(t->remaining > 0) {
        t->remaining--;
    }

    if(t->remaining > 0) {
        return; /* 尚未归零 */
    }

    /* ---- 倒计时归零：结算当前段并自动切换 ---- */
    ui_pomodoro_event_t ev;
    ui_pomodoro_mode_t  next_mode;

    if(t->mode == UI_POMODORO_MODE_WORK) {
        /* 工作番茄完成 */
        ev = UI_POMODORO_EVENT_WORK_DONE;
        t->completed_work++;

        /* 每完成 UI_POMODORO_LONG_BREAK_EVERY 个工作番茄进入长休，否则短休 */
        if(t->completed_work % UI_POMODORO_LONG_BREAK_EVERY == 0) {
            next_mode = UI_POMODORO_MODE_LONG_BREAK;
        } else {
            next_mode = UI_POMODORO_MODE_SHORT_BREAK;
        }
    } else {
        /* 休息结束 */
        ev = UI_POMODORO_EVENT_BREAK_DONE;
        next_mode = UI_POMODORO_MODE_WORK;

        /* 长休结束回到工作 → 番茄计数归零，开始新一轮 */
        if(t->mode == UI_POMODORO_MODE_LONG_BREAK) {
            t->completed_work = 0;
        }
    }

    /* 切换模式并自动开始下一段 */
    t->mode      = next_mode;
    t->remaining = ui_pomodoro_timer_mode_seconds(next_mode);
    t->running   = true; /* 归零后自动开始 */

    if(t->cb) {
        t->cb(ev, next_mode, t->cb_user_data);
    }
}
