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
#include "iface_virtual.h"

int64_t GetMillis( void ) {
    return esp_timer_get_time( ) / 1000;
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

const int RSTPin = 5;
const int DCPin = 16;
const int CSPin = 4;
const int SCLPin = 22;
const int SDAPin = 21;

struct SSD1306_Device Dev_SPI;
struct SSD1306_Device Dev_I2C;
struct SSD1306_Device Dev_Span;

void ShiftTask( void* Param ) {
    static uint8_t In[ 8 ];
    static uint8_t Out[ 8 ];
    int64_t Start = 0;
    int64_t End = 0;
    int Delay = 0;

    while ( true ) {
        Start = GetMillis( );
            FBShiftLeft( &Dev_Span, In, Out );
            memcpy( In, Out, sizeof( Out ) );

            Virt_DeviceBlit( &Dev_Span, &Dev_I2C, MakeRect( 0, 127, 0, 63 ), MakeRect( 0, 127, 0, 63 ) );
            Virt_DeviceBlit( &Dev_Span, &Dev_SPI, MakeRect( 128, 255, 0, 63 ), MakeRect( 0, 127, 0, 63 ) );   

            SSD1306_Update( &Dev_I2C );
            SSD1306_Update( &Dev_SPI );
        End = GetMillis( );

        /* Sync to 30FPS */
        Delay = 33 - ( int ) ( End - Start );

        if ( Delay <= 0 ) {
            /* More dogs for the watch dog god */
            Delay= 3;
        }

        vTaskDelay( pdMS_TO_TICKS( Delay ) );
    }
}

void app_main( void ) {
    bool Screen0 = false;
    bool Screen1 = false;

    printf( "Ready...\n" );

    if ( ESP32_InitI2CMaster( SDAPin, SCLPin ) ) {
        printf( "i2c master initialized.\n" );

        if ( SSD1306_Init_I2C( &Dev_I2C, 128, 64, 0x3C, 0, ESP32_WriteCommand_I2C, ESP32_WriteData_I2C, NULL ) == 1 ) {
            printf( "i2c display initialized.\n" );
            Screen0 = true;
            
            //SSD1306_SetFont( &Dev_I2C, &Font_Comic_Neue_25x28 );
            //FontDrawAnchoredString( &Dev_I2C, "Smile!", TextAnchor_Center, true );

            //SSD1306_Update( &Dev_I2C );
        }
    }

    if ( ESP32_InitSPIMaster( DCPin ) ) {
        printf( "SPI Master Init OK.\n" );

        if ( ESP32_AddDevice_SPI( &Dev_SPI, 128, 64, CSPin, RSTPin ) == 1 ) {
            printf( "SSD1306 Init OK.\n" );
            Screen1 = true;      

            //SSD1306_SetFont( &Dev_SPI, &Font_Comic_Neue_25x28 );
            //FontDrawAnchoredString( &Dev_SPI, "Okay.", TextAnchor_Center, true );

            //SSD1306_Update( &Dev_SPI );                            
        }
    }

    if ( Screen0 == true && Screen1 == true ) {
        if ( Virt_DeviceInit( &Dev_Span, 256, 64 ) == 1 ) {
            printf( "Span created!\n" );

            SSD1306_SetFont( &Dev_Span, &Font_Comic_Neue_25x28 );
            FontDrawAnchoredString( &Dev_Span, "lol lmao and so on.", TextAnchor_Center, true );
 
            Virt_DeviceBlit( &Dev_Span, &Dev_I2C, MakeRect( 0, 127, 0, 63 ), MakeRect( 0, 127, 0, 63 ) );
            Virt_DeviceBlit( &Dev_Span, &Dev_SPI, MakeRect( 128, 255, 0, 63 ), MakeRect( 0, 127, 0, 63 ) );

            SSD1306_Update( &Dev_I2C );
            SSD1306_Update( &Dev_SPI );

            xTaskCreate( ShiftTask, "ShiftTask", 4096, NULL, 3, NULL );
        }
    }
}
