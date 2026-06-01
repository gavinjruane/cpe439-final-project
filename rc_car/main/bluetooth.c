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

#include "bluetooth.h"

static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;
static const bool esp_spp_enable_l2cap_ertm = true;

static const esp_spp_sec_t security_mask = ESP_SPP_SEC_AUTHENTICATE;
static const esp_spp_role_t role_slave = ESP_SPP_ROLE_SLAVE;

QueueHandle_t send_queue;
QueueHandle_t receive_queue;
TaskHandle_t send_task;

static uint16_t bluetooth_handle = 0;

static void _bt_spp_callback (esp_spp_cb_event_t event, esp_spp_cb_param_t *param); 
static void _bt_gap_callback (esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);
static void _bt_send_task (void *argument);


static void _bt_send_task (void *argument) {
    struct bt_data packet;

    while (1) {
        if ( xQueueReceive(send_queue, (void *) &packet, portMAX_DELAY) != errQUEUE_EMPTY ) {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            esp_spp_write(bluetooth_handle, (int) packet.length, (uint8_t *) packet.data);
        }

        vTaskDelay(1);
    }
}


int bt_init (const char *device_name) {
    esp_err_t status = nvs_flash_init();
    esp_spp_cfg_t spp_config = {0};
    BaseType_t result = pdFALSE;

    /* --- FreeRTOS Structure Initialization --- */

    if ( (receive_queue = xQueueCreate(32, sizeof(struct bt_data))) == NULL ) {
        ESP_LOGE(BT_TAG, "failed to create receive queue.");
        return -1;
    }
    
    if ( (send_queue = xQueueCreate(32, sizeof(struct bt_data))) == NULL ) {
        ESP_LOGE(BT_TAG, "failed to create send queue.");
        return -1;
    }

    result = xTaskCreate(
        _bt_send_task,
        "Bluetooth_Send_Task",
        8192,
        NULL,
        3,
        &send_task
    );
    if ( result == pdFALSE ) {
        ESP_LOGE(BT_TAG, "failed to create send task");
        return -1;
    }

    /* --- Bluetooth Controller Initialization --- */

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t controller_config = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

    /* Initialize the Bluetooth Controller to allocate resources and tasks.
    https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/controller_vhci.html#_CPPv422esp_bt_controller_initP26esp_bt_controller_config_t */

    if ( (status = esp_bt_controller_init(&controller_config)) != ESP_OK ) {
        ESP_LOGE(BT_TAG, "failed to initialize controller.");
        return -1;
    }

    /* Enable the Bluetooth Controller */
    if ( (status = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK ) {
        ESP_LOGE(BT_TAG, "failed to enable controller, error %d.", status);
        switch ( status ) {
            case ESP_ERR_INVALID_STATE: printf("invalid state"); break;
            case ESP_ERR_INVALID_ARG: printf("invalid argument"); break;
            default: printf("something else"); break;
        }
        return -1;
    }

    /* --- Bluedroid Initialization --- */

    esp_bluedroid_config_t bluedroid_config = BT_BLUEDROID_INIT_CONFIG_DEFAULT();

    if ( (status = esp_bluedroid_init_with_cfg(&bluedroid_config)) != ESP_OK ) {
        ESP_LOGE(BT_TAG, "failed to initialize bluedroid");
        return -1;
    }

    if ( (status = esp_bluedroid_enable()) != ESP_OK ) {
        ESP_LOGE(BT_TAG, "failed to enable bluedroid");
        return -1;
    }

    /* --- GAP (Generic Access Profile) Initialization --- */

    if ( (status = esp_bt_gap_register_callback(_bt_gap_callback)) != ESP_OK ) {
        ESP_LOGE(BT_TAG, "failed to register GAP callback");
        return -1;
    }

    /* --- SPP (Serial Port Profile) Initialization --- */

    if ( (status = esp_spp_register_callback(_bt_spp_callback)) != ESP_OK ) {
        ESP_LOGE(BT_TAG, "failed to register SPP callback");
        return -1;
    }

    spp_config.mode = esp_spp_mode;
    spp_config.enable_l2cap_ertm = esp_spp_enable_l2cap_ertm;
    spp_config.tx_buffer_size = 0;
    if ( (status = esp_spp_enhanced_init(&spp_config)) != ESP_OK ) {
        ESP_LOGE(BT_TAG, "failed to initialize SPP");
        return -1;
    }

    /* --- Set device name --- */

    if ( (status = esp_bt_gap_set_device_name(device_name ? device_name : "ESP32-DEV")) != ESP_OK ) {
        ESP_LOGE(BT_TAG, "failed to set device name");
        return -1;
    }

    esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_VARIABLE;
    esp_bt_pin_code_t pin_code;
    esp_bt_gap_set_pin(pin_type, 0, pin_code);

    return 0;
}


int bt_receive (struct bt_data *received_data, TickType_t delay) {
    memset(received_data, 0, sizeof(struct bt_data));

    if ( xQueueReceive(receive_queue, (void *) received_data, delay) == errQUEUE_EMPTY ) {
        // ESP_LOGE(BT_TAG, "Queue was empty, so aborted.");
        return -1;
    }

    return 0;
}

int bt_send (struct bt_data received_data, TickType_t delay) {
    if ( xQueueSend(send_queue, (void *) &received_data, delay) == errQUEUE_FULL ) {
       // ESP_LOGE(BT_TAG, "Queue was full, so aborted.");
       return -1;
    }

    // xQueueOverwrite(send_queue, (const void *) &received_data);

    return 0;
}


static void _bt_spp_callback (esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    switch ( event ) {
        case ESP_SPP_INIT_EVT:
            // Initialization for SPP event
            if ( param->init.status == ESP_SPP_SUCCESS ) {
                ESP_LOGI(BT_TAG, "ESP_SPP_INIT_EVT");
                esp_spp_start_srv(security_mask, role_slave, 0, SPP_SERVER);
            } else {
                ESP_LOGE(BT_TAG, "ESP_SPP_INIT_EVT failed");
            }
            break;

        case ESP_SPP_START_EVT:
            // Start server success event
            if ( param->start.status == ESP_SPP_SUCCESS ) {
                ESP_LOGI(BT_TAG, "ESP_SPP_START_EVT");
                esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
            } else {
                ESP_LOGE(BT_TAG, "ESP_SPP_START_EVT failed");
            }
            break;
        
        case ESP_SPP_DATA_IND_EVT:
            // Data to receive event
            if ( param->data_ind.len < BT_DATA_BUFFER_SIZE ) {
                struct bt_data *packet = NULL;
                size_t length = param->data_ind.len;
                
                packet = malloc(sizeof(struct bt_data));
                if ( ! packet ) {
                    ESP_LOGE(BT_TAG, "malloc() error");
                }

                if ( param->data_ind.data && (param->data_ind.len > 0) ) {
                    memcpy(packet->data, param->data_ind.data, param->data_ind.len);
                    packet->data[length] = '\0';
                    packet->length = length;
                } else {
                    ESP_LOGE(BT_TAG, "Data was NULL.");
                    break;
                }

                if ( xQueueSend(receive_queue, (void *) packet, (TickType_t) 10) == errQUEUE_FULL ) {
                    ESP_LOGE(BT_TAG, "Queue was full, so could not send.");
                }

                free(packet);
            } else {
                ESP_LOGE(BT_TAG, "Packet is too large to queue.");
            }
            break;

        case ESP_SPP_SRV_OPEN_EVT:
            // When SPP server connection opens event
            xTaskNotifyGive(send_task);

            // Store handle
            bluetooth_handle = param->open.handle;
            break;

        case ESP_SPP_WRITE_EVT:
            // When a write operation completes event
            if ( param->write.status == ESP_SPP_SUCCESS ) {
                xTaskNotifyGive(send_task);
            } else {
                ESP_LOGI(BT_TAG, "congested, please wait");
            }
            break;

        case ESP_SPP_CONG_EVT:
            // When the channel is congested
            if ( param->cong.cong == 0 ) {
                ESP_LOGI(BT_TAG, "not congested");
            } else {
                ESP_LOGI(BT_TAG, "congested");
            }
            break;

        default:
            ESP_LOGI(BT_TAG, "unhandled SPP event %d", event);
            break;
    }
}

static void _bt_gap_callback (esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
    switch ( event ) {
        case ESP_BT_GAP_AUTH_CMPL_EVT:
            // Authentication complete event
            if ( param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS ) {
                ESP_LOGI(BT_TAG, "authentication success");
            } else {
                ESP_LOGE(BT_TAG, "authentication failure");
            }
            break;

        case ESP_BT_GAP_PIN_REQ_EVT:
            // Legacy PIN pairing code request event
            ESP_LOGI(BT_TAG, "ESP_BT_GAP_PIN_REQ_EVT min_16_digit:%d", param->pin_req.min_16_digit);
            if (param->pin_req.min_16_digit) {
                ESP_LOGI(BT_TAG, "Input pin code: 0000 0000 0000 0000");
                esp_bt_pin_code_t pin_code = {0};
                esp_bt_gap_pin_reply(param->pin_req.bda, true, 16, pin_code);
            } else {
                ESP_LOGI(BT_TAG, "Input pin code: 1234");
                esp_bt_pin_code_t pin_code;
                pin_code[0] = '1';
                pin_code[1] = '2';
                pin_code[2] = '3';
                pin_code[3] = '4';
                esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
            }
            break;

        case ESP_BT_GAP_MODE_CHG_EVT:
            // Mode change event
            ESP_LOGI(BT_TAG, "ESP_BT_GAP_MODE_CHG_EVT mode:%d", param->mode_chg.mode);
            break;

        default:
            ESP_LOGI(BT_TAG, "unhandled GAP event %d", event);
            break;
    }
}