#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"

#define BT_TAG "BLUETOOTH"

static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;
static const bool esp_spp_enable_l2cap_ertm = true;

void bt_spp_callback (esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
void bt_gap_callback (esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);

void bt_spp_callback (esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {

}

void bt_gap_callback (esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {

}

void bt_init () {
    esp_err_t status = nvs_flash_init();
    esp_spp_cfg_t spp_config = {0};

    /* --- Bluetooth Controller Initialization --- */

    esp_bt_controller_config_t controller_config = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

    /* Initialize the Bluetooth Controller to allocate resources and tasks.
    https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/controller_vhci.html#_CPPv422esp_bt_controller_initP26esp_bt_controller_config_t */

    if ( (status = esp_bt_controller_init(&controller_config)) != ESP_OK ) {
        ESP_LOGE(BT_TAG, "failed to initialize controller.");
        return;
    }

    /* Enable the Bluetooth Controller */
    if ( (status = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK ) {
        ESP_LOGE(BT_TAG, "failed to enable controller.");
        return;
    }

    /* --- Bluedroid Initialization --- */

    esp_bluedroid_config_t bluedroid_config = BT_BLUEDROID_INIT_CONFIG_DEFAULT();

    if ( (status = esp_bluedroid_init_with_cfg(&bluedroid_config)) != ESP_OK ) {
        ESP_LOGE(BT_TAG, "failed to initialize bluedroid");
        return;
    }

    if ( (status = esp_bluedroid_enable()) != ESP_OK ) {
        ESP_LOGE(BT_TAG, "failed to enable bluedroid");
        return;
    }

    /* --- GAP (Generic Access Profile) Initialization --- */

    if ( (status = esp_bt_gap_register_callback(bt_gap_callback)) != ESP_OK ) {
        ESP_LOGE(BT_TAG, "failed to register GAP callback");
        return;
    }

    /* --- SPP (Serial Port Profile) Initialization --- */

    if ( (status = esp_spp_register_callback(bt_spp_callback)) != ESP_OK ) {
        ESP_LOGE(BT_TAG, "failed to register SPP callback");
        return;
    }

    spp_config.mode = esp_spp_mode;
    spp_config.enable_l2cap_ertm = esp_spp_enable_l2cap_ertm;
    spp_config.tx_buffer_size = 0;
    if ( (status = esp_spp_enhanced_init(&spp_config)) != ESP_OK ) {
        ESP_LOGE(BT_TAG, "failed to initialize SPP");
        return;
    }

    esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_VARIABLE;
    esp_bt_pin_code_t pin_code;
    esp_bt_gap_set_pin(pin_type, 0, pin_code);
}