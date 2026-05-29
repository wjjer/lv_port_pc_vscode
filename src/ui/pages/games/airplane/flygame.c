#include "lvgl.h"
#include <string.h>
#include <stdlib.h>
#include "../../../ui.h"

#define BULLET_COUNT   7
#define ENEMY_COUNT    10
#define PLAYER_LIVES   3

// 子弹结构
typedef struct {
    lv_obj_t * obj;
    int alive;
    int x;
    int y;
} bullet_t;

// 敌机结构
typedef struct {
    lv_obj_t * obj;
    int alive;
    int move_x;
    int move_y;
    int x;
    int y;
} enemy_t;

// 颜色定义
#define COLOR_GAME_BG     lv_color_hex(0x000020)
#define COLOR_UI_BG       lv_color_hex(0x1a1a2e)
#define COLOR_TEXT_CYAN   lv_color_hex(0x00ffff)
#define COLOR_TEXT_RED    lv_color_hex(0xFF6666)
#define COLOR_BTN_EXIT    lv_color_hex(0x404060)

LV_IMG_DECLARE(fly)
LV_IMG_DECLARE(boom)
LV_IMG_DECLARE(enemyboss)
LV_IMG_DECLARE(enemyfly1)
LV_IMG_DECLARE(enemyfly2)
LV_IMG_DECLARE(enemyfly3)

static const lv_img_dsc_t *enemyflyimg[] = {&enemyfly2, &enemyfly3, &enemyfly1};

// 游戏状态
static lv_obj_t * game_page;
static lv_obj_t * game_container;
static lv_obj_t * exit_btn;
static lv_obj_t * fly1;
static lv_obj_t * score_label;
static lv_obj_t * lives_label;
static lv_obj_t * boss_health_bar;

static bullet_t bullets[BULLET_COUNT];
static enemy_t enemies[ENEMY_COUNT];
static enemy_t enemy_boss;

static int score = 0;
static int boss_live_value = 100;
static int player_lives_val = PLAYER_LIVES;
static int player_invincible = 0;

// 定时器
static lv_timer_t * timer_game = NULL;
static lv_timer_t * timer_bullet = NULL;
static lv_timer_t * timer_enemy = NULL;
static int cleanup_done = 0;  // 防止重复清理

// 函数声明
static void timer_game_cb(lv_timer_t * t);
static void timer_bullet_cb(lv_timer_t * t);
static void timer_enemy_cb(lv_timer_t * t);
static void bullet_move(int step);
static void bullet_create(void);
static void bullet_display(void);
static void bullet_clear(void);
static void enemy_move(void);
static void enemy_spawn(void);
static void enemy_delete(void);
static void enemy_attacked(void);
static void player_hit(void);
static void boss_attacked(void);
static void boom_anim_exec(void * var, int32_t v);
static void boom_anim_end(lv_anim_t * a);
static void boss_move_cb(void * var, int32_t v);
static void boss_attack_anim(void * var, int32_t v);
static void boss_dead_cb(lv_anim_t * a);
static void game_over_delayed(lv_timer_t * t);
static void cleanup_delayed(lv_timer_t * t);
static void exit_game(lv_event_t * e);
static void game_cleanup(void);
static void touch_event_cb(lv_event_t * e);

// ==============================
// 游戏页面显示/隐藏
// ==============================
void flygame_show(void) {
    // 重置游戏状态
    cleanup_done = 0;
    score = 0;
    player_lives_val = PLAYER_LIVES;
    player_invincible = 0;
    boss_live_value = 100;
    enemy_boss.alive = 0;
    enemy_boss.obj = NULL;

    // 重置子弹数组
    for(int i = 0; i < BULLET_COUNT; i++) {
        bullets[i].alive = 0;
        bullets[i].obj = NULL;
        bullets[i].x = 0;
        bullets[i].y = 0;
    }

    // 重置敌机数组
    for(int i = 0; i < ENEMY_COUNT; i++) {
        enemies[i].alive = 0;
        enemies[i].obj = NULL;
        enemies[i].x = 0;
        enemies[i].y = 0;
    }

    // 创建游戏页面（直接作为游戏容器）
    game_page = lv_obj_create(lv_layer_top());
    lv_obj_set_size(game_page, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(game_page, COLOR_GAME_BG, 0);
    lv_obj_set_style_border_width(game_page, 0, 0);
    lv_obj_clear_flag(game_page, LV_OBJ_FLAG_SCROLLABLE);
    game_container = game_page;  // 游戏容器就是主页面

    // 触摸控制
    lv_obj_add_event_cb(game_page, touch_event_cb, LV_EVENT_PRESSING, NULL);
    lv_obj_add_event_cb(game_page, touch_event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(game_page, touch_event_cb, LV_EVENT_RELEASED, NULL);

    // 创建退出按钮
    exit_btn = lv_btn_create(game_page);
    lv_obj_set_style_bg_color(exit_btn, COLOR_BTN_EXIT, 0);
    lv_obj_set_style_radius(exit_btn, 8, 0);
    lv_obj_align(exit_btn, LV_ALIGN_TOP_RIGHT, -5, 5);
    lv_obj_set_size(exit_btn, 60, 30);
    lv_obj_add_event_cb(exit_btn, exit_game, LV_EVENT_CLICKED, NULL);
    lv_obj_t * exit_lbl = lv_label_create(exit_btn);
    lv_label_set_text(exit_lbl, "退出");
    lv_obj_set_style_text_color(exit_lbl, lv_color_white(), 0);

    // 创建玩家飞机
    fly1 = lv_img_create(game_page);
    lv_img_set_src(fly1, &fly);
    lv_img_set_angle(fly1, 2700);
    lv_obj_set_pos(fly1, 200, LV_VER_RES - 100);

    // 创建分数标签
    score_label = lv_label_create(game_page);
    lv_label_set_text_fmt(score_label, "score: %d", score);
    lv_obj_set_style_text_color(score_label, COLOR_TEXT_CYAN, 0);
    lv_obj_set_style_text_font(score_label, &lv_font_montserrat_14, 0);
    lv_obj_align(score_label, LV_ALIGN_TOP_LEFT, 10, 5);

    // 创建生命标签
    lives_label = lv_label_create(game_page);
    lv_label_set_text_fmt(lives_label, "lives: %d", player_lives_val);
    lv_obj_set_style_text_color(lives_label, COLOR_TEXT_RED, 0);
    lv_obj_set_style_text_font(lives_label, &lv_font_montserrat_14, 0);
    lv_obj_align(lives_label, LV_ALIGN_TOP_LEFT, 10, 25);

    // 创建定时器（优化帧率）
    timer_game = lv_timer_create(timer_game_cb, 16, NULL);   // ~60fps
    timer_bullet = lv_timer_create(timer_bullet_cb, 300, NULL);  // 子弹生成
    timer_enemy = lv_timer_create(timer_enemy_cb, 1000, NULL);   // 敌机生成
}

void flygame_hide(void) {
    game_cleanup();
}

void flygame_toggle(void) {
    if(game_page) {
        flygame_hide();
    } else {
        flygame_show();
    }
}

// ==============================
// 游戏清理
// ==============================
static void game_cleanup(void) {
    if(cleanup_done) return;  // 防止重复清理
    cleanup_done = 1;

    // 删除定时器
    if(timer_game) { lv_timer_del(timer_game); timer_game = NULL; }
    if(timer_bullet) { lv_timer_del(timer_bullet); timer_bullet = NULL; }
    if(timer_enemy) { lv_timer_del(timer_enemy); timer_enemy = NULL; }

    // 删除游戏页面
    if(game_page) {
        lv_obj_del(game_page);
        game_page = NULL;
    }

    // 重置指针
    game_container = NULL;
    exit_btn = NULL;
    fly1 = NULL;
    score_label = NULL;
    lives_label = NULL;
    boss_health_bar = NULL;
    enemy_boss.obj = NULL;
}

static void exit_game(lv_event_t * e) {
    (void)e;
    game_cleanup();
}

// ==============================
// 触摸/鼠标控制
// ==============================
static void touch_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_PRESSING || code == LV_EVENT_PRESSED || code == LV_EVENT_RELEASED) {
        lv_indev_t * indev = lv_indev_get_act();
        if(indev && lv_indev_get_type(indev) == LV_INDEV_TYPE_POINTER) {
            lv_point_t p;
            lv_indev_get_point(indev, &p);
            // 限制飞机位置在游戏容器内
            int x = p.x - 25;  // 飞机宽度一半
            int y = p.y - 30;  // 飞机高度一半
            if(x < 0) x = 0;
            if(y < 0) y = 0;
            if(x > LV_HOR_RES - 50) x = LV_HOR_RES - 50;
            if(y > LV_VER_RES - 60) y = LV_VER_RES - 60;
            lv_obj_set_pos(fly1, x, y);
        }
    }
}

// ==============================
// 游戏循环
// ==============================
static void timer_game_cb(lv_timer_t * t) {
    (void)t;
    if(game_container == NULL || fly1 == NULL) return;

    // 移动子弹
    bullet_move(-5);
    bullet_display();
    bullet_clear();

    // 敌机逻辑
    enemy_move();
    enemy_delete();
    enemy_attacked();
    boss_attacked();
    player_hit();
}

static void timer_bullet_cb(lv_timer_t * t) {
    (void)t;
    if(game_container == NULL || fly1 == NULL) return;
    bullet_create();
}

static void timer_enemy_cb(lv_timer_t * t) {
    (void)t;
    if(game_container == NULL || fly1 == NULL) return;
    enemy_spawn();

    // 生成Boss
    if(score > 5000 && enemy_boss.alive == 0) {
        boss_live_value = 100;
        enemy_boss.alive = 1;
        enemy_boss.obj = lv_img_create(game_container);
        lv_img_set_src(enemy_boss.obj, &enemyboss);
        lv_img_set_angle(enemy_boss.obj, 900);
        lv_obj_set_pos(enemy_boss.obj, -100, 50);

        boss_health_bar = lv_bar_create(enemy_boss.obj);
        lv_obj_set_style_bg_color(boss_health_bar, lv_color_hex(0xff0000), LV_PART_INDICATOR);
        lv_bar_set_range(boss_health_bar, 0, 100);
        lv_bar_set_value(boss_health_bar, boss_live_value, LV_ANIM_OFF);
        lv_obj_set_size(boss_health_bar, 6, 50);
        lv_obj_align(boss_health_bar, LV_ALIGN_LEFT_MID, 20, 0);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, &enemy_boss);
        lv_anim_set_exec_cb(&a, boss_move_cb);
        lv_anim_set_time(&a, 5000);
        lv_anim_set_values(&a, -100, 50);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
        lv_anim_start(&a);
    }
}

// ==============================
// 子弹控制
// ==============================
static void bullet_create(void) {
    for(int i = 0; i < BULLET_COUNT; i++) {
        if(bullets[i].alive == 0) {
            bullets[i].alive = 1;
            bullets[i].x = lv_obj_get_x(fly1);
            bullets[i].y = lv_obj_get_y(fly1);

            bullets[i].obj = lv_btn_create(game_container);
            lv_obj_set_pos(bullets[i].obj, bullets[i].x + 20, bullets[i].y - 10);
            lv_obj_set_size(bullets[i].obj, 10, 20);
            lv_obj_set_style_bg_color(bullets[i].obj, lv_color_hex(0xFFFF00), 0);
            lv_obj_set_style_border_width(bullets[i].obj, 0, 0);
            return;
        }
    }
}

static void bullet_move(int step) {
    for(int i = 0; i < BULLET_COUNT; i++) {
        if(bullets[i].alive == 1) {
            bullets[i].x += step;
        }
    }
}

static void bullet_display(void) {
    for(int i = 0; i < BULLET_COUNT; i++) {
        if(bullets[i].alive == 1) {
            lv_obj_set_pos(bullets[i].obj, bullets[i].x, bullets[i].y);
        }
    }
}

static void bullet_clear(void) {
    for(int i = 0; i < BULLET_COUNT; i++) {
        if(bullets[i].alive == 1 && bullets[i].x < 0) {
            lv_obj_del(bullets[i].obj);
            bullets[i].alive = 0;
            bullets[i].obj = NULL;
        }
    }
}

// ==============================
// 敌机控制
// ==============================
static void enemy_move(void) {
    for(int i = 0; i < ENEMY_COUNT; i++) {
        if(enemies[i].alive == 1) {
            enemies[i].x += enemies[i].move_x;
            enemies[i].y += enemies[i].move_y;
            lv_obj_set_pos(enemies[i].obj, enemies[i].x, enemies[i].y);
        }
    }
}

static void enemy_delete(void) {
    for(int i = 0; i < ENEMY_COUNT; i++) {
        if(enemies[i].alive == 1) {
            if(enemies[i].x > 550 || enemies[i].y > 340 || enemies[i].y < -30) {
                lv_obj_del(enemies[i].obj);
                enemies[i].alive = 0;
            }
        }
    }
}

static void enemy_spawn(void) {
    int select = rand() % 3;
    for(int i = 0; i < ENEMY_COUNT; i++) {
        if(enemies[i].alive == 0) {
            enemies[i].alive = 1;
            enemies[i].x = -50;
            // 在屏幕上方区域内随机生成Y坐标
            enemies[i].y = rand() % (LV_VER_RES - 100);
            enemies[i].move_x = rand() % 6 + 1;
            enemies[i].move_y = rand() % 3 - 1;
            enemies[i].obj = lv_img_create(game_container);
            lv_img_set_src(enemies[i].obj, enemyflyimg[select]);
            lv_img_set_angle(enemies[i].obj, 900);
            lv_obj_set_pos(enemies[i].obj, enemies[i].x, enemies[i].y);
            return;
        }
    }
}

// ==============================
// 碰撞检测
// ==============================
static void enemy_attacked(void) {
    if(game_container == NULL) return;

    for(int i = 0; i < BULLET_COUNT; i++) {
        if(bullets[i].alive != 1) continue;

        for(int j = 0; j < ENEMY_COUNT; j++) {
            if(enemies[j].alive != 1) continue;

            if((bullets[i].x - enemies[j].x < 40 && bullets[i].x - enemies[j].x > -20) &&
               (bullets[i].y - enemies[j].y < 20 && bullets[i].y - enemies[j].y > -30)) {

                lv_obj_del(bullets[i].obj);
                bullets[i].alive = 0;

                lv_obj_del(enemies[j].obj);
                enemies[j].alive = 0;

                // 爆炸效果
                lv_obj_t * boomx = lv_img_create(game_container);
                lv_img_set_src(boomx, &boom);
                lv_obj_set_pos(boomx, enemies[j].x, enemies[j].y);

                lv_anim_t a;
                lv_anim_init(&a);
                lv_anim_set_var(&a, boomx);
                lv_anim_set_exec_cb(&a, boom_anim_exec);
                lv_anim_set_time(&a, 300);
                lv_anim_set_values(&a, 1, 5);
                lv_anim_set_ready_cb(&a, boom_anim_end);
                lv_anim_start(&a);

                score += 100;
                if(score_label) lv_label_set_text_fmt(score_label, "score: %d", score);
                break;
            }
        }
    }
}

static void player_hit(void) {
    if(fly1 == NULL) return;

    if(player_invincible > 0) {
        player_invincible--;
        if(player_invincible == 0 && fly1 != NULL) {
            lv_obj_set_style_img_opa(fly1, 255, 0);
        }
        return;
    }

    if(player_lives_val <= 0) return;

    int player_x = lv_obj_get_x(fly1);
    int player_y = lv_obj_get_y(fly1);
    int player_w = 50;
    int player_h = 60;

    for(int i = 0; i < ENEMY_COUNT; i++) {
        if(enemies[i].alive != 1) continue;

        int enemy_x = enemies[i].x;
        int enemy_y = enemies[i].y;

        if(player_x < enemy_x + 40 && player_x + player_w > enemy_x - 10 &&
           player_y < enemy_y + 40 && player_y + player_h > enemy_y - 10) {

            player_lives_val--;
            if(lives_label) lv_label_set_text_fmt(lives_label, "lives: %d", player_lives_val);

            lv_obj_del(enemies[i].obj);
            enemies[i].alive = 0;

            // 爆炸效果
            lv_obj_t * boomx = lv_img_create(game_container);
            lv_img_set_src(boomx, &boom);
            lv_obj_set_pos(boomx, enemy_x, enemy_y);

            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, boomx);
            lv_anim_set_exec_cb(&a, boom_anim_exec);
            lv_anim_set_time(&a, 300);
            lv_anim_set_values(&a, 1, 5);
            lv_anim_set_ready_cb(&a, boom_anim_end);
            lv_anim_start(&a);

            player_invincible = 50;
            lv_obj_set_style_img_opa(fly1, 128, 0);

            if(player_lives_val <= 0) {
                lv_obj_t * gameover = lv_label_create(game_container);
                lv_label_set_text(gameover, "游戏结束!");
                lv_obj_set_style_text_color(gameover, lv_color_hex(0xFF0000), 0);
                lv_obj_set_style_text_font(gameover, &lv_font_montserrat_24, 0);
                lv_obj_center(gameover);

                if(timer_game) lv_timer_pause(timer_game);
                if(timer_bullet) lv_timer_pause(timer_bullet);
                if(timer_enemy) lv_timer_pause(timer_enemy);
                lv_timer_create(game_over_delayed, 2000, NULL);
            }
            return;
        }
    }
}

// ==============================
// Boss控制
// ==============================
static void boss_attacked(void) {
    if(game_container == NULL) return;

    for(int i = 0; i < BULLET_COUNT; i++) {
        if(bullets[i].alive == 1 && enemy_boss.alive == 1) {
            if((bullets[i].x - enemy_boss.x < 50 && bullets[i].x - enemy_boss.x > -50) &&
               (bullets[i].y - enemy_boss.y < 50 && bullets[i].y - enemy_boss.y > -50)) {

                lv_obj_del(bullets[i].obj);
                bullets[i].alive = 0;
                boss_live_value--;
                score += 100;
                if(score_label) lv_label_set_text_fmt(score_label, "score: %d", score);
                if(boss_health_bar) lv_bar_set_value(boss_health_bar, boss_live_value, LV_ANIM_OFF);

                lv_anim_t a;
                lv_anim_init(&a);
                lv_anim_set_var(&a, &enemy_boss);
                lv_anim_set_exec_cb(&a, boss_attack_anim);
                lv_anim_set_time(&a, 100);
                lv_anim_set_values(&a, 1, 2);
                lv_anim_set_ready_cb(&a, boss_dead_cb);
                lv_anim_start(&a);

                lv_obj_t * boomx = lv_img_create(game_container);
                lv_img_set_src(boomx, &boom);
                lv_obj_set_pos(boomx, bullets[i].x, bullets[i].y);

                lv_anim_t a2;
                lv_anim_init(&a2);
                lv_anim_set_var(&a2, boomx);
                lv_anim_set_exec_cb(&a2, boom_anim_exec);
                lv_anim_set_time(&a2, 300);
                lv_anim_set_values(&a2, 1, 5);
                lv_anim_set_ready_cb(&a2, boom_anim_end);
                lv_anim_start(&a2);
            }
        }
    }
}

static void boss_move_cb(void * var, int32_t v) {
    enemy_t * boss = (enemy_t *)var;
    if(game_container == NULL || boss == NULL) return;
    lv_obj_set_x(boss->obj, v);
    boss->x = lv_obj_get_x(boss->obj);
    boss->y = lv_obj_get_y(boss->obj);
}

static void boss_attack_anim(void * var, int32_t v) {
    enemy_t * boss = (enemy_t *)var;
    if(game_container == NULL || boss == NULL || boss->obj == NULL) return;
    lv_obj_set_style_img_opa(boss->obj, 127 * v, 0);
}

static void boss_dead_cb(lv_anim_t * a) {
    (void)a;
    if(game_container == NULL) return;

    if(boss_live_value == 0 && enemy_boss.obj != NULL) {
        lv_obj_t * boomx = lv_img_create(game_container);
        lv_img_set_src(boomx, &boom);
        lv_obj_set_pos(boomx, enemy_boss.x + 38, enemy_boss.y + 10);

        lv_anim_t a2;
        lv_anim_init(&a2);
        lv_anim_set_var(&a2, boomx);
        lv_anim_set_exec_cb(&a2, boom_anim_exec);
        lv_anim_set_time(&a2, 500);
        lv_anim_set_values(&a2, 1, 10);
        lv_anim_set_ready_cb(&a2, boom_anim_end);
        lv_anim_start(&a2);

        enemy_boss.alive = 0;
        lv_obj_del(enemy_boss.obj);
        enemy_boss.obj = NULL;

        // 通关
        if(game_container != NULL) {
            lv_obj_t * win = lv_label_create(game_container);
            lv_label_set_text(win, "恭喜通关!");
            lv_obj_set_style_text_color(win, lv_color_hex(0x00FF00), 0);
            lv_obj_set_style_text_font(win, &lv_font_montserrat_24, 0);
            lv_obj_center(win);

            if(timer_game) lv_timer_pause(timer_game);
            if(timer_bullet) lv_timer_pause(timer_bullet);
            if(timer_enemy) lv_timer_pause(timer_enemy);
            lv_timer_create(game_over_delayed, 3000, NULL);
        }
    }
}

// ==============================
// 动画回调
// ==============================
static void boom_anim_exec(void * var, int32_t v) {
    lv_obj_t * obj = (lv_obj_t *)var;
    if(game_container == NULL || obj == NULL) return;
    lv_img_set_zoom(obj, 55 + v * 50);
    lv_obj_set_style_img_opa(obj, 300 - v * 50, 0);
}

static void boom_anim_end(lv_anim_t * a) {
    lv_obj_t * obj = (lv_obj_t *)a->var;
    if(obj == NULL) return;
    lv_obj_del(obj);
}

static void game_over_delayed(lv_timer_t * t) {
    (void)t;
    game_cleanup();
}
