/**
 * Copyright (c) 2017 Tara Keeling
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include <stdio.h>
#include <string.h>
#include <esp_timer.h>
#include <driver/spi_master.h>
#include <driver/i2c.h>

#include "ssd1306.h"
#include "font.h"

#include "iface_esp32_i2c.h"
#include "iface_esp32_spi.h"

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

void FBShiftLeft( struct SSD1306_Device* DeviceHandle, uint8_t* ShiftIn, uint8_t* ShiftOut ) {
    uint8_t* Framebuffer = NULL;
    int Width = 0;
    int Height = 0;
    int y = 0;
    int x = 0;

    NullCheck( DeviceHandle, return );

    Framebuffer = DeviceHandle->Framebuffer;
    Width = DeviceHandle->Width;
    Height = DeviceHandle->Height;

    /* Clear out the first and last rows */
    for ( y = 0; y < ( Width / 8 ); y++ ) {
        /* Copy the column to be destroyed out if a buffer was passed in to hold it */
        if ( ShiftOut != NULL ) {
            ShiftOut[ y ] = Framebuffer[ y * Width ];
        }

        Framebuffer[ y * Width ] = 0;

        /* If the caller passes a buffer of pixels it wants shifted in, use that instead of clearing it */
        Framebuffer[ ( y * Width ) + ( Width - 1 ) ] = ( ShiftIn != NULL ) ? ShiftIn[ y ] : 0;
    }

    /* Shift every column of pixels one column to the left */
    for ( x = 0; x < ( Width - 1 ); x++ ) {
        for ( y = 0; y < ( Height / 8 ); y++ ) {
            Framebuffer[ x + ( y * Width ) ] = Framebuffer[ 1 + x + ( y * Width ) ]; 
        }
    }
}

void DrawPixelInColumn( uint8_t* Column, int y, bool Color ) {
    uint32_t Pixel = ( y & 0x07 );
    uint32_t Page = ( y >> 3 );

    NullCheck( Column, return );

    Column[ Page ] = ( Color == true ) ? Column[ Page ] | BIT( Pixel ) : Column[ Page ] & ~BIT( Pixel );
}

void ShiftTask( void* Param ) {
    uint8_t In[ 8 ];
    int dy = 1;
    int y = 0;

    while ( true ) {
        memset( In, 0, sizeof( In ) );

        y+= dy;

        if ( y >= 63 ) {
            dy = -1;
        } else if ( y <= 0 ) {
            dy = 1;
        }

        DrawPixelInColumn( In, y, true );

        FBShiftLeft( &TestDevice, In, NULL );
        SSD1306_Update( &TestDevice );

        vTaskDelay( 50 / portTICK_PERIOD_MS );
    }
}

const int RSTPin = 5;
const int DCPin = 16;
const int CSPin = 4;

void app_main( void ) {
    printf( "Ready...\n" );

    /*if ( InitI2CMaster( 18, 19 ) ) {*/
    if ( ESP32_InitSPIMaster( DCPin ) ) {
        /*printf( "I2C Master Init OK.\n" );*/
        printf( "SPI Master Init OK.\n" );

        /*if ( SSD1306_Init_I2C( &TestDevice, 128, 64, 0x3C, ESP32_WriteCommand_I2C, ESP32_WriteData_I2C ) == 1 ) {*/
        if ( ESP32_AddDevice_SPI( &TestDevice, 128, 64, CSPin, RSTPin ) == 1 ) {
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

            //xTaskCreate( ShiftTask, "ShiftTask", 4096, NULL, 5, NULL );                             
        }
    }
}
