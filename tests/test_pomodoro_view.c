#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "lvgl.h"
#include "../src/ui/pages/app/pomodoro/ui_pomodoro.h"
#include "../src/ui/pages/app/pomodoro/ui_pomodoro_storage.h"

static uint32_t fake_tick;
static uint8_t draw_buf[320 * 40 * 4];

static uint32_t tick_cb(void)
{
    return fake_tick;
}

static void flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
    (void)area;
    (void)px_map;
    lv_display_flush_ready(disp);
}

static void pump(uint32_t ms)
{
    fake_tick += ms;
    lv_timer_handler();
}

static int count_type(lv_obj_t * obj, const lv_obj_class_t * cls)
{
    int count = lv_obj_check_type(obj, cls) ? 1 : 0;
    uint32_t child_count = lv_obj_get_child_count(obj);
    for(uint32_t i = 0; i < child_count; i++) {
        count += count_type(lv_obj_get_child(obj, (int32_t)i), cls);
    }
    return count;
}

static lv_obj_t * find_label(lv_obj_t * obj, const char * text)
{
    if(lv_obj_check_type(obj, &lv_label_class)) {
        const char * current = lv_label_get_text(obj);
        if(current && strcmp(current, text) == 0) return obj;
    }
    uint32_t child_count = lv_obj_get_child_count(obj);
    for(uint32_t i = 0; i < child_count; i++) {
        lv_obj_t * found = find_label(lv_obj_get_child(obj, (int32_t)i), text);
        if(found) return found;
    }
    return NULL;
}

static void mark(const char * msg)
{
    printf("%s\n", msg);
    fflush(stdout);
}

int main(int argc, char ** argv)
{
    (void)argc;
    (void)argv;

    mark("start");
    remove("tests_pomodoro_view_tmp.dat");
    ui_pomodoro_storage_set_path("tests_pomodoro_view_tmp.dat");

    mark("lv_init");
    lv_init();
    lv_tick_set_cb(tick_cb);
    lv_display_t * disp = lv_display_create(320, 240);
    lv_display_set_flush_cb(disp, flush_cb);
    lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

    mark("show");
    ui_pomodoro_show();
    pump(20);

    mark("assert main");    lv_obj_t * page = ui_pomodoro_test_get_page();
    assert(page != NULL);
    assert(lv_obj_is_valid(page));
    assert(ui_pomodoro_test_get_tick_timer() != NULL);
    assert(find_label(page, "番茄时钟") != NULL);
    assert(find_label(page, "工作中") != NULL);
    assert(find_label(page, "25:00") != NULL);
    assert(find_label(page, "开始") != NULL);
    assert(find_label(page, "重置") != NULL);
    assert(find_label(page, "切换") != NULL);
    assert(find_label(page, "今日番茄") != NULL);
    assert(find_label(page, "任务") != NULL);
    assert(count_type(page, &lv_button_class) >= 4);
    assert(count_type(page, &lv_dropdown_class) >= 1);
    assert(count_type(page, &lv_arc_class) >= 1);

    mark("start pause");
    ui_pomodoro_action_start_pause();
    pump(20);
    assert(find_label(page, "暂停") != NULL);

    mark("reset");
    ui_pomodoro_action_reset();
    pump(20);
    assert(find_label(page, "25:00") != NULL);
    assert(find_label(page, "开始") != NULL);

    mark("tasks");
    ui_pomodoro_route_to_tasks();
    pump(20);
    page = ui_pomodoro_test_get_page();
    assert(find_label(page, "任务管理") != NULL);
    assert(count_type(page, &lv_checkbox_class) >= 2);

    ui_pomodoro_test_submit_task_name("测试任务");
    pump(20);
    page = ui_pomodoro_test_get_page();
    assert(ui_pomodoro_test_get_task_input() == NULL);
    assert(ui_pomodoro_get_store()->task_count >= 3);
    assert(strcmp(ui_pomodoro_get_store()->tasks[ui_pomodoro_get_store()->task_count - 1].name, "测试任务") == 0);
    assert(find_label(page, "测试任务") != NULL);

    ui_pomodoro_hide();
    assert(ui_pomodoro_test_get_page() == NULL);
    assert(ui_pomodoro_test_get_tick_timer() == NULL);
    pump(140);

    remove("tests_pomodoro_view_tmp.dat");
    ui_pomodoro_storage_set_path(NULL);

    printf("test_pomodoro_view passed\n");
    return 0;
}
