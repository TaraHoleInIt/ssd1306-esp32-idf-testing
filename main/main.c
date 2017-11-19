#include <stdio.h>
#include <string.h>
#include <esp_timer.h>
#include <driver/i2c.h>

#include "ssd1306.h"
#include "font.h"

#include "iface_esp32_i2c.h"

int64_t GetMillis( void );

struct SSD1306_Device TestDevice;

int64_t GetMillis( void ) {
    return esp_timer_get_time( ) / 1000;
}

int InitI2CMaster( int SDA, int SCL ) {
    i2c_config_t Config;

    memset( &Config, 0, sizeof( i2c_config_t ) );

    Config.mode = I2C_MODE_MASTER;
    Config.sda_io_num = SDA;
    Config.sda_pullup_en = GPIO_PULLUP_ENABLE;
    Config.scl_io_num = SCL;
    Config.scl_pullup_en = GPIO_PULLUP_ENABLE;
    Config.master.clk_speed = 1000000;   // 1MHz

    if ( i2c_param_config( USE_THIS_I2C_PORT, &Config ) == ESP_OK ) {
        return i2c_driver_install( USE_THIS_I2C_PORT, Config.mode, 0, 0, 0 ) == ESP_OK ? 1 : 0;
    }

    return 0;
}

void app_main( void ) {
    printf( "Ready...\n" );

    if ( InitI2CMaster( 18, 19 ) ) {
        printf( "I2C Master Init OK.\n" );

        if ( SSD1306_Init_I2C( &TestDevice, 128, 64, 0x3C, ESP32_WriteCommand_I2C, ESP32_WriteData_I2C ) == 1 ) {
            printf( "SSD1306 Init OK.\n" );

            /* SSD1306_SetInverted( &TestDevice, true ); */
            /* SSD1306_SetFont( &TestDevice, &Font_Liberation_Sans_15x16 ); */
            /* SSD1306_SetFont( &TestDevice, &Font_Liberation_Serif_19x19 ); */
            /* SSD1306_SetFont( &TestDevice, &Font_Ubuntu_Mono_6x10 ); */
            SSD1306_SetFont( &TestDevice, &Font_Comic_Neue_25x28 );

            /*
            FontDrawAnchoredString( &TestDevice, "NE", TextAnchor_NorthEast, true );
            FontDrawAnchoredString( &TestDevice, "NW", TextAnchor_NorthWest, true );
            FontDrawAnchoredString( &TestDevice, "N", TextAnchor_North, true );
            FontDrawAnchoredString( &TestDevice, "E", TextAnchor_East, true );
            FontDrawAnchoredString( &TestDevice, "W", TextAnchor_West, true );
            FontDrawAnchoredString( &TestDevice, "SE", TextAnchor_SouthEast, true );
            FontDrawAnchoredString( &TestDevice, "SW", TextAnchor_SouthWest, true );
            FontDrawAnchoredString( &TestDevice, "S", TextAnchor_South, true );
            FontDrawAnchoredString( &TestDevice, "C", TextAnchor_Center, true );
            */

            FontDrawAnchoredString( &TestDevice, "Smile!", TextAnchor_North, true );
            FontDrawAnchoredString( &TestDevice, "Okay.", TextAnchor_South, true );

            SSD1306_Update( &TestDevice );                                  
        }
    }
}
