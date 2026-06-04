#ifndef UI_POMODORO_TIMER_H
#define UI_POMODORO_TIMER_H

/**
 * @file ui_pomodoro_timer.h
 * @brief 番茄时钟核心计时控制器（纯逻辑状态机，零 LVGL 依赖）
 *
 * 设计原则（见 CONSTRAINTS.md §11 移植边界）：
 *  - 本控制器不引用任何 LVGL / 硬件接口，可在 PC 上独立单元测试。
 *  - UI 层每秒调用一次 ui_pomodoro_timer_tick()，并通过回调感知"模式切换"
 *    与"番茄完成"等事件，从而刷新视图、写存储、播放提示音。
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 默认时长（秒） —— 与 PROGRESS.md 项 1.2 行为描述一致
 * ============================================================ */
#define UI_POMODORO_WORK_SECONDS        (25 * 60)   /* 工作 25:00 */
#define UI_POMODORO_SHORT_BREAK_SECONDS (5 * 60)    /* 短休 5:00  */
#define UI_POMODORO_LONG_BREAK_SECONDS  (15 * 60)   /* 长休 15:00 */
#define UI_POMODORO_LONG_BREAK_EVERY    4           /* 每 4 个工作番茄进入长休 */

/* 计时模式 */
typedef enum {
    UI_POMODORO_MODE_WORK = 0,   /* 工作中 */
    UI_POMODORO_MODE_SHORT_BREAK, /* 短休息 */
    UI_POMODORO_MODE_LONG_BREAK,  /* 长休息 */
} ui_pomodoro_mode_t;

/* 计时器倒计时归零（一段计时结束）时触发的事件类型 */
typedef enum {
    UI_POMODORO_EVENT_WORK_DONE = 0, /* 一个工作番茄结束（番茄计数 +1） */
    UI_POMODORO_EVENT_BREAK_DONE,    /* 一段休息结束 */
} ui_pomodoro_event_t;

/**
 * @brief 番茄段完成回调
 * @param ev        刚刚结束的段类型（工作/休息）
 * @param next_mode 自动切换后的新模式
 * @param user_data 注册时传入的用户指针
 */
typedef void (*ui_pomodoro_timer_cb_t)(ui_pomodoro_event_t ev,
                                       ui_pomodoro_mode_t next_mode,
                                       void * user_data);

/* 控制器状态（调用方只读，请勿直接改写字段） */
typedef struct {
    ui_pomodoro_mode_t mode;       /* 当前模式 */
    int32_t            remaining;  /* 当前模式剩余秒数 */
    bool               running;    /* 是否正在倒计时 */
    int32_t            completed_work; /* 已完成工作番茄数（用于判定长休） */

    ui_pomodoro_timer_cb_t cb;     /* 段完成回调 */
    void *                 cb_user_data;
} ui_pomodoro_timer_t;

/**
 * @brief 初始化控制器：模式=工作、剩余=25:00、停止、番茄计数清零。
 */
void ui_pomodoro_timer_init(ui_pomodoro_timer_t * t);

/**
 * @brief 注册段完成回调（可传 NULL 取消）。
 */
void ui_pomodoro_timer_set_cb(ui_pomodoro_timer_t * t,
                              ui_pomodoro_timer_cb_t cb, void * user_data);

/**
 * @brief 开始 / 继续倒计时。
 */
void ui_pomodoro_timer_start(ui_pomodoro_timer_t * t);

/**
 * @brief 暂停倒计时（保留剩余时间）。
 */
void ui_pomodoro_timer_pause(ui_pomodoro_timer_t * t);

/**
 * @brief 重置：恢复当前模式默认时长并停止计时（番茄计数保持不变）。
 */
void ui_pomodoro_timer_reset(ui_pomodoro_timer_t * t);

/**
 * @brief 手动切换工作 / 休息模式（停止计时并装填目标模式默认时长）。
 *        在工作模式下调用 → 切到短休；在任意休息模式下调用 → 切回工作。
 */
void ui_pomodoro_timer_toggle_mode(ui_pomodoro_timer_t * t);

/**
 * @brief 每秒推进一格。仅在 running 时递减。
 *        归零时：触发回调 → 自动切换到下一模式 → 自动开始（保持 running）。
 *        工作归零：completed_work +1；满 UI_POMODORO_LONG_BREAK_EVERY 进长休，
 *        且长休结束切回工作后 completed_work 归零。
 */
void ui_pomodoro_timer_tick(ui_pomodoro_timer_t * t);

/**
 * @brief 取当前模式默认时长（秒）。
 */
int32_t ui_pomodoro_timer_mode_seconds(ui_pomodoro_mode_t mode);

/**
 * @brief 取模式中文名（"工作中" / "短休息" / "长休息"）。
 */
const char * ui_pomodoro_timer_mode_name(ui_pomodoro_mode_t mode);

#ifdef __cplusplus
}
#endif

#endif /* UI_POMODORO_TIMER_H */
