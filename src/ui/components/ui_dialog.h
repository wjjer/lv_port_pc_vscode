#ifndef UI_DIALOG_H
#define UI_DIALOG_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 弹窗完成回调函数签名
 * @param user_data 用户自定义数据
 * @param result_text 结果文本（输入弹窗为用户输入内容，确认弹窗为 "ok"/"cancel"）
 */
typedef void (*ui_dialog_done_cb_t)(void * user_data, const char * result_text);

/**
 * @brief 显示文字输入弹窗
 * @param parent 父容器（NULL 则使用 lv_layer_top）
 * @param title 弹窗标题
 * @param placeholder 输入框占位符
 * @param initial_text 输入框初始内容
 * @param ok_text 确认按钮文字
 * @param cancel_text 取消按钮文字
 * @param done_cb 完成回调
 * @param user_data 回调自定义参数
 */
void ui_dialog_text_input_show(lv_obj_t * parent,
    const char * title,
    const char * placeholder,
    const char * initial_text,
    const char * ok_text,
    const char * cancel_text,
    ui_dialog_done_cb_t done_cb,
    void * user_data);

/**
 * @brief 显示确认取消弹窗
 * @param parent 父容器（NULL 则使用 lv_layer_top）
 * @param title 弹窗标题
 * @param message 提示内容
 * @param ok_text 确认按钮文字
 * @param cancel_text 取消按钮文字
 * @param done_cb 完成回调
 * @param user_data 回调自定义参数
 */
void ui_dialog_confirm_show(lv_obj_t * parent,
    const char * title,
    const char * message,
    const char * ok_text,
    const char * cancel_text,
    ui_dialog_done_cb_t done_cb,
    void * user_data);

#ifdef __cplusplus
}
#endif

#endif
