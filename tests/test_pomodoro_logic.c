#include <assert.h>
#include <stdio.h>
#include "../src/ui/pages/app/pomodoro/ui_pomodoro_timer.h"

static int work_done_count = 0;
static int break_done_count = 0;
static ui_pomodoro_mode_t last_next_mode = UI_POMODORO_MODE_WORK;

static void test_cb(ui_pomodoro_event_t ev, ui_pomodoro_mode_t next_mode, void * user_data)
{
    (void)user_data;
    if(ev == UI_POMODORO_EVENT_WORK_DONE) work_done_count++;
    if(ev == UI_POMODORO_EVENT_BREAK_DONE) break_done_count++;
    last_next_mode = next_mode;
}

static void finish_current_segment(ui_pomodoro_timer_t * timer)
{
    timer->remaining = 1;
    ui_pomodoro_timer_tick(timer);
}

int main(int argc, char ** argv)
{
    (void)argc;
    (void)argv;

    ui_pomodoro_timer_t timer;
    ui_pomodoro_timer_init(&timer);
    ui_pomodoro_timer_set_cb(&timer, test_cb, NULL);

    assert(timer.mode == UI_POMODORO_MODE_WORK);
    assert(timer.remaining == UI_POMODORO_WORK_SECONDS);
    assert(timer.running == false);
    assert(timer.completed_work == 0);
    assert(ui_pomodoro_timer_mode_seconds(UI_POMODORO_MODE_WORK) == 25 * 60);
    assert(ui_pomodoro_timer_mode_seconds(UI_POMODORO_MODE_SHORT_BREAK) == 5 * 60);
    assert(ui_pomodoro_timer_mode_seconds(UI_POMODORO_MODE_LONG_BREAK) == 15 * 60);

    ui_pomodoro_timer_start(&timer);
    assert(timer.running == true);
    ui_pomodoro_timer_tick(&timer);
    assert(timer.remaining == UI_POMODORO_WORK_SECONDS - 1);

    ui_pomodoro_timer_pause(&timer);
    assert(timer.running == false);
    int32_t paused_remaining = timer.remaining;
    ui_pomodoro_timer_tick(&timer);
    assert(timer.remaining == paused_remaining);

    ui_pomodoro_timer_reset(&timer);
    assert(timer.running == false);
    assert(timer.remaining == UI_POMODORO_WORK_SECONDS);

    ui_pomodoro_timer_toggle_mode(&timer);
    assert(timer.mode == UI_POMODORO_MODE_SHORT_BREAK);
    assert(timer.remaining == UI_POMODORO_SHORT_BREAK_SECONDS);
    assert(timer.running == false);
    ui_pomodoro_timer_toggle_mode(&timer);
    assert(timer.mode == UI_POMODORO_MODE_WORK);
    assert(timer.remaining == UI_POMODORO_WORK_SECONDS);

    ui_pomodoro_timer_start(&timer);
    finish_current_segment(&timer);
    assert(work_done_count == 1);
    assert(last_next_mode == UI_POMODORO_MODE_SHORT_BREAK);
    assert(timer.mode == UI_POMODORO_MODE_SHORT_BREAK);
    assert(timer.running == true);
    assert(timer.remaining == UI_POMODORO_SHORT_BREAK_SECONDS);
    assert(timer.completed_work == 1);

    finish_current_segment(&timer);
    assert(break_done_count == 1);
    assert(timer.mode == UI_POMODORO_MODE_WORK);
    assert(timer.running == true);
    assert(timer.remaining == UI_POMODORO_WORK_SECONDS);

    for(int i = 0; i < 3; i++) {
        finish_current_segment(&timer);
        assert(timer.running == true);
        if(i < 2) {
            assert(timer.mode == UI_POMODORO_MODE_SHORT_BREAK);
            finish_current_segment(&timer);
            assert(timer.mode == UI_POMODORO_MODE_WORK);
        }
    }

    assert(work_done_count == 4);
    assert(timer.mode == UI_POMODORO_MODE_LONG_BREAK);
    assert(timer.remaining == UI_POMODORO_LONG_BREAK_SECONDS);
    assert(timer.completed_work == 4);

    finish_current_segment(&timer);
    assert(break_done_count == 4);
    assert(timer.mode == UI_POMODORO_MODE_WORK);
    assert(timer.remaining == UI_POMODORO_WORK_SECONDS);
    assert(timer.completed_work == 0);

    printf("test_pomodoro_logic passed\n");
    return 0;
}
