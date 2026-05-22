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

#define BT_DATA_BUFFER_SIZE 256

struct bt_data {
    size_t length;
    char data[BT_DATA_BUFFER_SIZE];
};

int bt_init (const char *device_name);
int bt_receive (struct bt_data *received_data, TickType_t delay);
int bt_send (struct bt_data received_data, TickType_t delay);

#endif