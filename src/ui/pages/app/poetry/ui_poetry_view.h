#ifndef UI_POETRY_VIEW_H
#define UI_POETRY_VIEW_H

// 声明纯粹的界面渲染接口，不暴露业务状态
void ui_poetry_view_render_home(void);
void ui_poetry_view_render_list(const char *stage_title);
void ui_poetry_view_render_detail(const char *title, const char *author, const char *content, const char *notes);

#endif // UI_POETRY_VIEW_H
