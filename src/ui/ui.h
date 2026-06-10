#ifndef UI_H
#define UI_H

#include "lvgl.h"
#include <stdlib.h>
#include <stdio.h>
#include "ui_config.h"
#include "components/ui_statusbar.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_font_t font;

// 初始化当前UI
void ui_init(void);

// 创建图标
lv_obj_t * create_icon(lv_obj_t * parent, const char * name, const void * src_img);

// Dock 栏控制接口
void ui_dock_hide(void);
void ui_dock_show(void);

// 桌面背景色设置
void ui_set_desktop_bg_color(lv_color_t color);

#ifdef __cplusplus
}
#endif

#endif
