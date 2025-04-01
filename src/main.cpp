/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>
#include "lvgl_v8_port.h"

/**
/* To use the built-in examples and demos of LVGL uncomment the includes below respectively.
 * You also need to copy `lvgl/examples` to `lvgl/src/examples`. Similarly for the demos `lvgl/demos` to `lvgl/src/demos`.
 */
// #include <demos/lv_demos.h>
// #include <lv_ex/lv_examples.h>

using namespace esp_panel::drivers;
using namespace esp_panel::board;

void custom_init();

// Example: Create a label
static lv_obj_t *pointer;

// Add X label
static lv_obj_t *labelX;

// Add Y label
static lv_obj_t *labelY;

void setup()
{
    Serial.begin(115200);

    Serial.println("Initializing board");
    Board *board = new Board();
    board->init();
    assert(board->begin());

    Serial.println("Initializing LVGL");
    lvgl_port_init(board->getLCD(), board->getTouch());

    Serial.println("Creating UI");
    /* Lock the mutex due to the LVGL APIs are not thread-safe */
    lvgl_port_lock(-1);

    // /**
    //  * Create the simple labels
    //  */
    // lv_obj_t *label_1 = lv_label_create(lv_scr_act());
    // lv_label_set_text(label_1, "Hello World!");
    // lv_obj_set_style_text_font(label_1, &lv_font_montserrat_30, 0);
    // lv_obj_align(label_1, LV_ALIGN_CENTER, 0, -20);
    // lv_obj_t *label_2 = lv_label_create(lv_scr_act());
    // lv_label_set_text_fmt(
    //     label_2, "ESP32_Display_Panel(%d.%d.%d)",
    //     ESP_PANEL_VERSION_MAJOR, ESP_PANEL_VERSION_MINOR, ESP_PANEL_VERSION_PATCH
    // );
    // lv_obj_set_style_text_font(label_2, &lv_font_montserrat_16, 0);
    // lv_obj_align_to(label_2, label_1, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    // lv_obj_t *label_3 = lv_label_create(lv_scr_act());
    // lv_label_set_text_fmt(label_3, "LVGL(%d.%d.%d)", LVGL_VERSION_MAJOR, LVGL_VERSION_MINOR, LVGL_VERSION_PATCH);
    // lv_obj_set_style_text_font(label_3, &lv_font_montserrat_16, 0);
    // lv_obj_align_to(label_3, label_2, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    /**
     * Try an example. Don't forget to uncomment header.
     * See all the examples online: https://docs.lvgl.io/master/examples.html
     * source codes: https://github.com/lvgl/lvgl/tree/e7f88efa5853128bf871dde335c0ca8da9eb7731/examples
     */
    // lv_example_btn_1();
    custom_init();

    /**
     * Or try out a demo.
     * Don't forget to uncomment header and enable the demos in `lv_conf.h`. E.g. `LV_USE_DEMO_WIDGETS`
     */
    // lv_demo_widgets();
    // lv_demo_benchmark();
    // lv_demo_music();
    // lv_demo_stress();

    /* Release the mutex */
    lvgl_port_unlock();
}

void loop()
{
    Serial.println("IDLE loop");
    delay(1000);
}

// static void event_handler(lv_event_t * e)
// {
//     lv_event_code_t code = lv_event_get_code(e);

//     if(code == LV_EVENT_CLICKED) {
//         LV_LOG_USER("Clicked");
//     }
//     else if(code == LV_EVENT_VALUE_CHANGED) {
//         LV_LOG_USER("Toggled");
//     }
// }

// void lv_example_btn_1(void)
// {
//     lv_obj_t * label;

//     // lv_obj_t * btn1 = lv_btn_create(lv_scr_act());
//     // lv_obj_add_event_cb(btn1, event_handler, LV_EVENT_ALL, NULL);
//     // lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -40);

//     // label = lv_label_create(btn1);
//     // lv_label_set_text(label, "Button");
//     // lv_obj_center(label);

//     lv_obj_t * btn2 = lv_btn_create(lv_scr_act());
//     lv_obj_add_event_cb(btn2, event_handler, LV_EVENT_ALL, NULL);
//     lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 40);
//     lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
//     lv_obj_set_height(btn2, LV_SIZE_CONTENT);

//     label = lv_label_create(btn2);
//     lv_label_set_text(label, "Toggle");
//     lv_obj_center(label);
// }

// -------------------------------


static void sliderX_event_handler(lv_event_t * e)
{
    // Get the event code
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t * slider = lv_event_get_target(e);
        int16_t value = lv_slider_get_value(slider);
        Serial.printf("Slider value: %d\n", value);

        lv_obj_set_x(pointer, value);
        lv_label_set_text_fmt(labelX, "X=%d", value);
    }
}

static void sliderY_event_handler(lv_event_t * e)
{
    // Get the event code
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t * slider = lv_event_get_target(e);
        int16_t value = lv_slider_get_value(slider);
        Serial.printf("Slider value: %d\n", value);

        lv_obj_set_y(pointer, value);
        lv_label_set_text_fmt(labelY, "Y=%d", value);
    }
} 

void custom_init()
{
    // Custom initialization code here
    Serial.println("Custom initialization");

    pointer = lv_label_create(lv_scr_act());

    labelX = lv_label_create(lv_scr_act());

    labelY = lv_label_create(lv_scr_act());

    lv_label_set_text(pointer, "+");
    lv_obj_set_style_text_color(pointer, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_text_font(pointer, &lv_font_montserrat_30, 0);
    lv_obj_align(pointer, LV_ALIGN_CENTER, 0, -20);

    lv_label_set_text(labelX, "X=");
    lv_obj_set_style_text_font(labelX, &lv_font_montserrat_12, 0);
    lv_obj_align(labelX, LV_ALIGN_CENTER, 0, -12);

    lv_label_set_text(labelY, "Y=");
    lv_obj_set_style_text_font(labelY, &lv_font_montserrat_12, 0);
    lv_obj_align(labelY, LV_ALIGN_CENTER, 0, 12);

    //Add a sliderX
    lv_obj_t *sliderX = lv_slider_create(lv_scr_act());
    lv_obj_set_width(sliderX, 150);
    lv_slider_set_range(sliderX, -180, 180);
    lv_obj_align(sliderX, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_set_style_bg_color(sliderX, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_bg_opa(sliderX, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(sliderX, 10, 0);
    lv_obj_set_style_border_width(sliderX, 0, 0);
    lv_obj_add_event_cb(sliderX, sliderX_event_handler, LV_EVENT_ALL, NULL);

    //Add a sliderY
    lv_obj_t *sliderY = lv_slider_create(lv_scr_act());
    lv_obj_set_width(sliderY, 150);
    lv_slider_set_range(sliderY, -180, 180);
    lv_obj_align(sliderY, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_obj_set_style_bg_color(sliderY, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_bg_opa(sliderY, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(sliderY, 10, 0);
    lv_obj_set_style_border_width(sliderY, 0, 0);
    lv_obj_add_event_cb(sliderY, sliderY_event_handler, LV_EVENT_ALL, NULL);
}