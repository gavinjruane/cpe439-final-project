#ifndef BLUETOOTH_H
#define BLUETOOTH_H

/*
    Bluetooth abstraction layer

    Author:     Gavin Ruane
                @gavinjruane
*/

#include "esp_gap_bt_api.h"
#include "esp_spp_api.h"

#define BT_TAG "BLUETOOTH"
#define SPP_SERVER "SPP_SERVER"

void bt_spp_callback (esp_spp_cb_event_t event, esp_spp_cb_param_t *param); 
void bt_gap_callback (esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);

int bt_init (const char *device_name);

#endif