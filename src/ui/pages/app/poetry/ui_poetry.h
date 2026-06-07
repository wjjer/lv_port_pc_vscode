#ifndef UI_POETRY_H
#define UI_POETRY_H

#include "lvgl.h"
#include <stdint.h>

// 导出全局核心容器（供 View 层构建页面时作为父物体）
extern lv_obj_t *app_screen;
extern lv_obj_t *current_page;

// 导出公共样式（彻底解决 View 层的 undefined reference 错误）
extern lv_style_t style_btn_classic;

// 暴露给主系统或外部导航框架的生命周期接口
void ui_poetry_show(void);
void ui_poetry_hide(void);

// 业务层提供的页面跳转路由（供 View 层的事件回调调用）
void ui_poetry_route_to_home(void);
void ui_poetry_route_to_list(const char *stage_title);
void ui_poetry_route_to_detail(void);

// 业务逻辑行为
void ui_poetry_toggle_play(lv_obj_t *btn_label);
void ui_poetry_toggle_notes(lv_obj_t *notes_panel);
void ui_poetry_set_current_poem_index(uint16_t index);

#endif // UI_POETRY_H
