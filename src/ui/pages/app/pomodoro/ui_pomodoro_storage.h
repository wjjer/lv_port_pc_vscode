#ifndef UI_POMODORO_STORAGE_H
#define UI_POMODORO_STORAGE_H

/**
 * @file ui_pomodoro_storage.h
 * @brief 番茄时钟任务管理与持久化（纯 C 文件存储，零 LVGL 依赖）
 *
 * 职责（见 PROGRESS.md 项 1.3）：
 *  - 任务列表：名称 / 各任务累计番茄数 / 完成态，支持增删改。
 *  - 今日番茄计数：每完成一个工作番茄 +1；跨日自动归零。
 *  - 持久化：load/save 到本地文件，重启不丢失。
 *
 * 移植说明：使用标准 C stdio。PC 端写工作目录下的 .dat 文件；
 * 移植到 ESP32-S3 时只需把 UI_POMODORO_STORAGE_PATH 指向挂载的
 * SPIFFS / 文件系统路径即可，逻辑一行不改（CONSTRAINTS.md §11）。
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UI_POMODORO_MAX_TASKS     16   /* 任务上限 */
#define UI_POMODORO_TASK_NAME_LEN 32   /* 任务名最大字节数（含结尾 \0） */

/* 单条任务 */
typedef struct {
    char    name[UI_POMODORO_TASK_NAME_LEN];
    int32_t tomato_count; /* 该任务累计完成的番茄数 */
    bool    completed;    /* 是否已勾选完成 */
} ui_pomodoro_task_t;

/* 整体持久化数据 */
typedef struct {
    ui_pomodoro_task_t tasks[UI_POMODORO_MAX_TASKS];
    int32_t            task_count;
    int32_t            today_count; /* 今日番茄计数 */
    int32_t            last_day;    /* 上次写入对应的"日序号"（用于跨日归零判定） */
} ui_pomodoro_store_t;

/**
 * @brief 覆盖默认存储文件路径（主要给测试用，传 NULL 恢复默认）。
 */
void ui_pomodoro_storage_set_path(const char * path);

/**
 * @brief 从文件加载到 out。文件不存在或损坏时填入默认值（含两条示例任务）并返回 false。
 * @param today_day 当前"日序号"（如 年*366+一年中的天）。与 last_day 不同则今日计数归零。
 */
bool ui_pomodoro_storage_load(ui_pomodoro_store_t * out, int32_t today_day);

/**
 * @brief 将 store 写入文件。成功返回 true。
 */
bool ui_pomodoro_storage_save(const ui_pomodoro_store_t * store, int32_t today_day);

/**
 * @brief 追加一条任务（番茄数 0、未完成）。成功返回索引，失败（满/非法）返回 -1。
 */
int32_t ui_pomodoro_store_add_task(ui_pomodoro_store_t * store, const char * name);

/**
 * @brief 删除指定索引任务（后续任务前移）。成功返回 true。
 */
bool ui_pomodoro_store_delete_task(ui_pomodoro_store_t * store, int32_t index);

/**
 * @brief 切换指定任务完成态。成功返回 true。
 */
bool ui_pomodoro_store_toggle_task(ui_pomodoro_store_t * store, int32_t index);

/**
 * @brief 记录"完成一个工作番茄"：今日计数 +1，且 task_index 任务番茄数 +1。
 *        task_index < 0 或越界时只增今日计数（无关联任务）。
 */
void ui_pomodoro_store_record_tomato(ui_pomodoro_store_t * store, int32_t task_index);

/**
 * @brief 计算"日序号"。用 struct tm 的年与年内天数合成单调递增整数，便于跨日比较。
 *        参数为标准 localtime 拆出的 tm_year(自1900) 与 tm_yday(0..365)。
 */
int32_t ui_pomodoro_day_index(int32_t tm_year, int32_t tm_yday);

#ifdef __cplusplus
}
#endif

#endif /* UI_POMODORO_STORAGE_H */
