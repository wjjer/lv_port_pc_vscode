/**
 * @file ui_pomodoro_storage.c
 * @brief 番茄时钟任务管理与持久化实现（纯 C，标准 stdio）
 */

#include "ui_pomodoro_storage.h"
#include <stdio.h>
#include <string.h>

#define UI_POMODORO_STORAGE_DEFAULT_PATH "pomodoro_data.dat"
#define UI_POMODORO_STORAGE_MAGIC        0x504D4431u /* "PMD1" */

static char s_storage_path[260] = UI_POMODORO_STORAGE_DEFAULT_PATH;

/* 文件落盘格式（紧凑、定长，便于嵌入式 FS 读写） */
typedef struct {
    uint32_t            magic;
    int32_t             task_count;
    int32_t             today_count;
    int32_t             last_day;
    ui_pomodoro_task_t  tasks[UI_POMODORO_MAX_TASKS];
} ui_pomodoro_file_t;

void ui_pomodoro_storage_set_path(const char * path)
{
    if(path == NULL) {
        strncpy(s_storage_path, UI_POMODORO_STORAGE_DEFAULT_PATH, sizeof(s_storage_path) - 1);
    } else {
        strncpy(s_storage_path, path, sizeof(s_storage_path) - 1);
    }
    s_storage_path[sizeof(s_storage_path) - 1] = '\0';
}

int32_t ui_pomodoro_day_index(int32_t tm_year, int32_t tm_yday)
{
    /* 年内天数最多 366，用 400 作基数保证单调递增且不溢出 */
    return tm_year * 400 + tm_yday;
}

static void fill_defaults(ui_pomodoro_store_t * s, int32_t today_day)
{
    memset(s, 0, sizeof(*s));
    /* 两条示例任务，便于首启动即可演示 */
    strncpy(s->tasks[0].name, "学习", UI_POMODORO_TASK_NAME_LEN - 1);
    strncpy(s->tasks[1].name, "阅读", UI_POMODORO_TASK_NAME_LEN - 1);
    s->task_count  = 2;
    s->today_count = 0;
    s->last_day    = today_day;
}

bool ui_pomodoro_storage_load(ui_pomodoro_store_t * out, int32_t today_day)
{
    if(out == NULL) return false;

    FILE * f = fopen(s_storage_path, "rb");
    if(f == NULL) {
        fill_defaults(out, today_day);
        return false;
    }

    ui_pomodoro_file_t fb;
    size_t n = fread(&fb, sizeof(fb), 1, f);
    fclose(f);

    if(n != 1 || fb.magic != UI_POMODORO_STORAGE_MAGIC ||
       fb.task_count < 0 || fb.task_count > UI_POMODORO_MAX_TASKS) {
        fill_defaults(out, today_day);
        return false;
    }

    memset(out, 0, sizeof(*out));
    out->task_count  = fb.task_count;
    out->today_count = fb.today_count;
    out->last_day    = fb.last_day;
    memcpy(out->tasks, fb.tasks, sizeof(out->tasks));

    /* 跨日：今日计数归零，并更新 last_day */
    if(out->last_day != today_day) {
        out->today_count = 0;
        out->last_day    = today_day;
    }
    return true;
}

bool ui_pomodoro_storage_save(const ui_pomodoro_store_t * store, int32_t today_day)
{
    if(store == NULL) return false;

    ui_pomodoro_file_t fb;
    memset(&fb, 0, sizeof(fb));
    fb.magic       = UI_POMODORO_STORAGE_MAGIC;
    fb.task_count  = store->task_count;
    fb.today_count = store->today_count;
    fb.last_day    = today_day;
    memcpy(fb.tasks, store->tasks, sizeof(fb.tasks));

    FILE * f = fopen(s_storage_path, "wb");
    if(f == NULL) return false;
    size_t n = fwrite(&fb, sizeof(fb), 1, f);
    fclose(f);
    return n == 1;
}

int32_t ui_pomodoro_store_add_task(ui_pomodoro_store_t * store, const char * name)
{
    if(store == NULL || name == NULL) return -1;
    if(store->task_count >= UI_POMODORO_MAX_TASKS) return -1;

    int32_t idx = store->task_count;
    memset(&store->tasks[idx], 0, sizeof(store->tasks[idx]));
    strncpy(store->tasks[idx].name, name, UI_POMODORO_TASK_NAME_LEN - 1);
    store->tasks[idx].name[UI_POMODORO_TASK_NAME_LEN - 1] = '\0';
    store->tasks[idx].tomato_count = 0;
    store->tasks[idx].completed    = false;
    store->task_count++;
    return idx;
}

bool ui_pomodoro_store_delete_task(ui_pomodoro_store_t * store, int32_t index)
{
    if(store == NULL || index < 0 || index >= store->task_count) return false;

    for(int32_t i = index; i < store->task_count - 1; i++) {
        store->tasks[i] = store->tasks[i + 1];
    }
    store->task_count--;
    memset(&store->tasks[store->task_count], 0, sizeof(store->tasks[store->task_count]));
    return true;
}

bool ui_pomodoro_store_toggle_task(ui_pomodoro_store_t * store, int32_t index)
{
    if(store == NULL || index < 0 || index >= store->task_count) return false;
    store->tasks[index].completed = !store->tasks[index].completed;
    return true;
}

void ui_pomodoro_store_record_tomato(ui_pomodoro_store_t * store, int32_t task_index)
{
    if(store == NULL) return;
    store->today_count++;
    if(task_index >= 0 && task_index < store->task_count) {
        store->tasks[task_index].tomato_count++;
    }
}
