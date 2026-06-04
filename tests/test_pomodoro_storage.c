#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/ui/pages/app/pomodoro/ui_pomodoro_storage.h"

int main(int argc, char ** argv)
{
    (void)argc;
    (void)argv;

    const char * path = "tests_pomodoro_storage_tmp.dat";
    remove(path);
    ui_pomodoro_storage_set_path(path);

    ui_pomodoro_store_t store;
    bool loaded = ui_pomodoro_storage_load(&store, ui_pomodoro_day_index(126, 10));
    assert(loaded == false);
    assert(store.task_count == 2);
    assert(store.today_count == 0);
    assert(strcmp(store.tasks[0].name, "学习") == 0);

    int32_t idx = ui_pomodoro_store_add_task(&store, "数学作业");
    assert(idx == 2);
    assert(store.task_count == 3);
    assert(strcmp(store.tasks[idx].name, "数学作业") == 0);

    ui_pomodoro_store_record_tomato(&store, idx);
    assert(store.today_count == 1);
    assert(store.tasks[idx].tomato_count == 1);

    assert(ui_pomodoro_store_toggle_task(&store, idx) == true);
    assert(store.tasks[idx].completed == true);
    assert(ui_pomodoro_storage_save(&store, ui_pomodoro_day_index(126, 10)) == true);

    ui_pomodoro_store_t reloaded;
    loaded = ui_pomodoro_storage_load(&reloaded, ui_pomodoro_day_index(126, 10));
    assert(loaded == true);
    assert(reloaded.task_count == 3);
    assert(reloaded.today_count == 1);
    assert(reloaded.tasks[idx].tomato_count == 1);
    assert(reloaded.tasks[idx].completed == true);

    ui_pomodoro_store_t next_day;
    loaded = ui_pomodoro_storage_load(&next_day, ui_pomodoro_day_index(126, 11));
    assert(loaded == true);
    assert(next_day.today_count == 0);
    assert(next_day.tasks[idx].tomato_count == 1);

    assert(ui_pomodoro_store_delete_task(&next_day, 1) == true);
    assert(next_day.task_count == 2);
    assert(strcmp(next_day.tasks[1].name, "数学作业") == 0);

    remove(path);
    ui_pomodoro_storage_set_path(NULL);

    printf("test_pomodoro_storage passed\n");
    return 0;
}
