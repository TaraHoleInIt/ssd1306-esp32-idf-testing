#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <driver/i2c.h>

#include "ssd1306.h"
#include "iface_esp32_i2c.h"

int ESP32_WriteCommand_I2C( struct SSD1306_Device* DeviceHandle, SSDCmd SSDCommand ) {
    i2c_cmd_handle_t CommandHandle = NULL;
    esp_err_t Result = ESP_FAIL;

    if ( ( CommandHandle = i2c_cmd_link_create( ) ) ) {
        i2c_master_start( CommandHandle );
            i2c_master_write_byte( CommandHandle, ( DeviceHandle->Address << 1 ) | I2C_MASTER_WRITE, 1 );
            i2c_master_write_byte( CommandHandle, 0x80, true );
            i2c_master_write_byte( CommandHandle, ( uint8_t ) SSDCommand, true );
        i2c_master_stop( CommandHandle );

        Result = i2c_master_cmd_begin( USE_THIS_I2C_PORT, CommandHandle, 1000 / portTICK_PERIOD_MS );
        i2c_cmd_link_delete( CommandHandle );
    }

    return ( Result == ESP_OK ) ? 1 : 0;
}

int ESP32_WriteData_I2C( struct SSD1306_Device* DeviceHandle, uint8_t* Data, size_t DataLength ) {
    i2c_cmd_handle_t CommandHandle = NULL;
    esp_err_t Result = ESP_FAIL;

    if ( ( CommandHandle = i2c_cmd_link_create( ) ) ) {
        i2c_master_start( CommandHandle );
            i2c_master_write_byte( CommandHandle, ( DeviceHandle->Address << 1 ) | I2C_MASTER_WRITE, true );
            i2c_master_write_byte( CommandHandle, 0x40, true );
            i2c_master_write( CommandHandle, Data, DataLength, true );
        i2c_master_stop( CommandHandle );

        Result = i2c_master_cmd_begin( USE_THIS_I2C_PORT, CommandHandle, 1000 / portTICK_PERIOD_MS );
        i2c_cmd_link_delete( CommandHandle );
    }

    return ( Result == ESP_OK ) ? 1 : 0;
}
