/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>
#include "lvgl_v8_port.h"

#include <WiFi.h>
#include <WebServer.h>

#include <NimBLEDevice.h>

const char* ssid = "Beelight";
const char* password = "password";
const char *NORDIC_UART_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
const char *NORDIC_UART_SERVICE_UUID_TX = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
const char *NORDIC_UART_SERVICE_UUID_RX = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";


/**
/* To use the built-in examples and demos of LVGL uncomment the includes below respectively.
 * You also need to copy `lvgl/examples` to `lvgl/src/examples`. Similarly for the demos `lvgl/demos` to `lvgl/src/demos`.
 */
// #include <demos/lv_demos.h>
// #include <lv_ex/lv_examples.h>

using namespace esp_panel::drivers;
using namespace esp_panel::board;


/* ==================== DEfS ==================== */
void ui_init();
void server_init();
void ble_init();

// Example: Create a label
static lv_obj_t *pointer;

// Add X label
static lv_obj_t *labelX;

// Add Y label
static lv_obj_t *labelY;

static lv_obj_t *labelRemoteMessage;

WebServer server(80);


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


    /**
     * Try an example. Don't forget to uncomment header.
     * See all the examples online: https://docs.lvgl.io/master/examples.html
     * source codes: https://github.com/lvgl/lvgl/tree/e7f88efa5853128bf871dde335c0ca8da9eb7731/examples
     */
    // lv_example_btn_1();
    ui_init();
    // server_init();
    ble_init();


    /* Release the mutex */
    lvgl_port_unlock();
}

void loop()
{
    server.handleClient();    
    delay(1000);
}

static void server_handle_root()
{
    server.send(200, "text/html", "<h1>Hello from ESP32</h1>");
    switch (server.method())
    {
        case HTTP_POST:
            Serial.println("Received POST request");
            // Handle POST request
            lv_label_set_text_fmt(labelRemoteMessage, "%s", server.arg(0));
            break;
        case HTTP_GET:
            Serial.println("Received GET request");
            break;

        default:
            break;
    }
}

static void server_handle_not_found()
{
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";

    for (uint8_t i = 0; i < server.args(); i++) {
        message += " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
    }

    server.send(404, "text/plain", message);
}


static void server_handler_picture()
{ 
    // Display the picture received from the client
    if (server.method() == HTTP_POST) {
        if (server.args() > 0) {
            String payload = server.arg(0);
            Serial.println("Received POST payload:");
            Serial.println(payload);
            lv_label_set_text_fmt(labelRemoteMessage, payload.c_str());
        } else {
            Serial.println("No plain text payload received in POST request.");
            lv_label_set_text_fmt(labelRemoteMessage, "Request received with no content!");
        }
    }
}

static void sliderX_event_handler(lv_event_t * e)
{
    // Get the event code
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t * slider = lv_event_get_target(e);
        int16_t value = lv_slider_get_value(slider);
        // Serial.printf("Slider value: %d\n", value);

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
        // Serial.printf("Slider value: %d\n", value);

        lv_obj_set_y(pointer, value);
        lv_label_set_text_fmt(labelY, "Y=%d", value);
    }
} 

void server_init()
{
    WiFi.softAP(ssid, password);
    Serial.println("WiFi AP started with credentials:");
    Serial.printf("SSID: %s\n", ssid);
    Serial.printf("Password: %s\n", password);
    Serial.printf("IP Address: %s\n", WiFi.softAPIP().toString().c_str());
    Serial.println("Starting server...");

    server.on("/", server_handle_root);
    server.onNotFound(server_handle_not_found);
    server.on("/pic", server_handler_picture);
    server.on("/text", HTTP_GET, []() {
        server.send(200, "text/plain", "Hello from ESP32");
        Serial.println("Received POST /text");
    });

    server.begin();
    Serial.println("HTTP server started");
}

void ble_init()
{
    NimBLEDevice::init("BeeLight"); // Name of the device

    NimBLEServer *pServer = NimBLEDevice::createServer();
    NimBLEService *pService = pServer->createService(NORDIC_UART_SERVICE_UUID);

    NimBLECharacteristic *pCharacteristic = pService->createCharacteristic(
        NORDIC_UART_SERVICE_UUID_RX,  // RX
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
    );

    // callback for receiving text
    class TextCallback : public NimBLECharacteristicCallbacks {
        void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo)
        {
            std::string value = pCharacteristic->getValue();
            Serial.print("Texte reçu : ");
            Serial.println(value.c_str());
            // Print on display
            lv_label_set_text_fmt(labelRemoteMessage, "%s", value.c_str());
        }
    };

    pCharacteristic->setCallbacks(new TextCallback());

    pService->start();
    Serial.println("BLE Service started");
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();

    // Advertising packet configuration
    pAdvertising->setName("BeeLight");
    pAdvertising->enableScanResponse(true);
    pAdvertising->setAppearance(0x1442);
    pAdvertising->addServiceUUID(NORDIC_UART_SERVICE_UUID);
    pAdvertising->setPreferredParams(0x06, 0x12);

    pAdvertising->start();
    Serial.println("BLE Advertising started");

    lv_label_set_text_fmt(labelRemoteMessage, "%s", NimBLEDevice::getAddress().toString().c_str());
}

void ui_init()
{
    // Custom initialization code here
    Serial.println("Custom initialization");

    pointer = lv_label_create(lv_scr_act());
    lv_label_set_text(pointer, "+");
    lv_obj_set_style_text_color(pointer, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_text_font(pointer, &lv_font_montserrat_30, 0);
    lv_obj_align(pointer, LV_ALIGN_CENTER, 0, -20);

    labelX = lv_label_create(lv_scr_act());
    lv_label_set_text(labelX, "X=");
    lv_obj_set_style_text_font(labelX, &lv_font_montserrat_12, 0);
    lv_obj_align(labelX, LV_ALIGN_CENTER, 0, -12);

    labelY = lv_label_create(lv_scr_act());
    lv_label_set_text(labelY, "Y=");
    lv_obj_set_style_text_font(labelY, &lv_font_montserrat_12, 0);
    lv_obj_align(labelY, LV_ALIGN_CENTER, 0, 12);

    labelRemoteMessage = lv_label_create(lv_scr_act());
    lv_label_set_text(labelRemoteMessage, "placeholder");
    lv_obj_set_style_text_font(labelRemoteMessage, &lv_font_montserrat_20, 0);
    lv_obj_align(labelRemoteMessage, LV_ALIGN_CENTER, 0, -50);

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