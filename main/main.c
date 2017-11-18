#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <esp_timer.h>
#include <driver/i2c.h>

#include "ssd1306.h"
#include "font.h"

#define USE_THIS_I2C_PORT I2C_NUM_1

#define COM_Disable_LR_Remap 0
#define COM_Enable_LR_Remap BIT( 5 )

#define COM_Pins_Sequential 0
#define COM_Pins_Alternative BIT( 4 )

#define COM_ScanDir_LR 0
#define COM_ScanDir_RL 1

int64_t GetMillis( void );

static void EnableChargePumpRegulator( struct SSD1306_Device* DeviceHandle );
static void SetCOMPinConfiguration( struct SSD1306_Device* DeviceHandle, uint32_t RemapCFG, uint32_t PinCFG, int ScanDir );

int SSD1306_Init_I2C( struct SSD1306_Device* DeviceHandle, int Address, int Width, int Height );
int SSD1306_WriteCommand_I2C( struct SSD1306_Device* DeviceHandle, SSDCmd SSDCommand );
int SSD1306_WriteData_I2C( struct SSD1306_Device* DeviceHandle, uint8_t* Data, size_t DataLength );
int SSD1306_WriteCommand( struct SSD1306_Device* DeviceHandle, SSDCmd SSDCommand );

struct SSD1306_Device TestDevice;

int64_t GetMillis( void ) {
    return esp_timer_get_time( ) / 1000;
}

int SSD1306_WriteCommand_I2C( struct SSD1306_Device* DeviceHandle, SSDCmd SSDCommand ) {
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

    return Result;
}

int SSD1306_WriteData_I2C( struct SSD1306_Device* DeviceHandle, uint8_t* Data, size_t DataLength ) {
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

    return Result;
}

int SSD1306_WriteCommand( struct SSD1306_Device* DeviceHandle, SSDCmd SSDCommand ) {
    NullCheck( DeviceHandle, return ESP_ERR_INVALID_ARG );

    return SSD1306_WriteCommand_I2C( DeviceHandle, SSDCommand );
}

int SSD1306_WriteData( struct SSD1306_Device* DeviceHandle, uint8_t* Data, size_t DataLength ) {
    NullCheck( DeviceHandle, return ESP_ERR_INVALID_ARG );

    return SSD1306_WriteData_I2C( DeviceHandle, Data, DataLength );
}

void SSD1306_SetMuxRatio( struct SSD1306_Device* DeviceHandle, uint8_t Ratio ) {
    NullCheck( DeviceHandle, return );

    SSD1306_WriteCommand( DeviceHandle, 0xA8 );
    SSD1306_WriteCommand( DeviceHandle, Ratio );
}

void SSD1306_SetDisplayOffset( struct SSD1306_Device* DeviceHandle, uint8_t Offset ) {
    NullCheck( DeviceHandle, return );

    SSD1306_WriteCommand( DeviceHandle, 0xD3 );
    SSD1306_WriteCommand( DeviceHandle, Offset );
}

void SSD1306_SetDisplayStartLine( struct SSD1306_Device* DeviceHandle, int Line ) {
    NullCheck( DeviceHandle, return );

    SSD1306_WriteCommand( DeviceHandle, 
        SSDCmd_Set_Display_Start_Line + ( uint32_t ) ( Line & 0x1F )
    );
}

/*
 * This is all a big giant mystery that I have yet to figure out.
 * Beware all ye who enter.
 */
static void SetCOMPinConfiguration( struct SSD1306_Device* DeviceHandle, uint32_t RemapCFG, uint32_t PinCFG, int ScanDir ) {
    NullCheck( DeviceHandle, return );

    SSD1306_WriteCommand( DeviceHandle, SSDCmd_Set_COM_Pin_Config );
    SSD1306_WriteCommand( DeviceHandle, ( uint8_t ) ( RemapCFG | PinCFG | BIT( 1 ) ) );

    SSD1306_WriteCommand( DeviceHandle, 
        ( ScanDir == COM_ScanDir_LR ) ? SSDCmd_Set_Display_VFlip_Off : SSDCmd_Set_Display_VFlip_On
    );
}

void SSD1306_SetContrast( struct SSD1306_Device* DeviceHandle, uint8_t Contrast ) {
    NullCheck( DeviceHandle, return );

    SSD1306_WriteCommand( DeviceHandle, SSDCmd_Set_Contrast );
    SSD1306_WriteCommand( DeviceHandle, Contrast );
}

void SSD1306_EnableDisplayRAM( struct SSD1306_Device* DeviceHandle ) {
    NullCheck( DeviceHandle, return );
    SSD1306_WriteCommand( DeviceHandle, SSDCmd_Set_Display_Show_RAM );
}

void SSD1306_DisableDisplayRAM( struct SSD1306_Device* DeviceHandle ) {
    NullCheck( DeviceHandle, return );
    SSD1306_WriteCommand( DeviceHandle, SSDCmd_Set_Display_Ignore_RAM );
}

void SSD1306_SetInverted( struct SSD1306_Device* DeviceHandle, bool Inverted ) {
    NullCheck( DeviceHandle, return );
    SSD1306_WriteCommand( DeviceHandle, ( Inverted == true ) ? SSDCmd_Set_Inverted_Display : SSDCmd_Set_Normal_Display );
}

void SSD1306_SetDisplayClocks( struct SSD1306_Device* DeviceHandle, uint32_t DisplayClockDivider, uint32_t OSCFrequency ) {
    NullCheck( DeviceHandle, return );

    DisplayClockDivider&= 0x0F;
    OSCFrequency&= 0x0F;

    SSD1306_WriteCommand( DeviceHandle, SSDCmd_Set_Display_CLK );
    SSD1306_WriteCommand( DeviceHandle, ( ( OSCFrequency << 4 ) | DisplayClockDivider ) );
}

/* There is no documentation for this command, but it is required during init. */
static void EnableChargePumpRegulator( struct SSD1306_Device* DeviceHandle ) {
    NullCheck( DeviceHandle, return );

    SSD1306_WriteCommand( DeviceHandle, SSDCmd_Enable_Charge_Pump_Regulator );
    SSD1306_WriteCommand( DeviceHandle, 0x14 ); /* MAGIC NUMBER */
}

void SSD1306_DisplayOn( struct SSD1306_Device* DeviceHandle ) {
    NullCheck( DeviceHandle, return );
    SSD1306_WriteCommand( DeviceHandle, SSDCmd_Set_Display_On );
}

void SSD1306_DisplayOff( struct SSD1306_Device* DeviceHandle ) {
    NullCheck( DeviceHandle, return );
    SSD1306_WriteCommand( DeviceHandle, SSDCmd_Set_Display_Off );
}

void SSD1306_SetDisplayAddressMode( struct SSD1306_Device* DeviceHandle, SSD1306_AddressMode AddressMode ) {
    NullCheck( DeviceHandle, return );

    SSD1306_WriteCommand( DeviceHandle, SSDCmd_Set_Memory_Addressing_Mode );
    SSD1306_WriteCommand( DeviceHandle, AddressMode );
}

void SSD1306_Update( struct SSD1306_Device* DeviceHandle ) {
    NullCheck( DeviceHandle, return );
    NullCheck( DeviceHandle->Framebuffer , return );

    SSD1306_WriteData_I2C( DeviceHandle, DeviceHandle->Framebuffer, DeviceHandle->FramebufferSize );
}

void SSD1306_WriteRawData( struct SSD1306_Device* DeviceHandle, uint8_t* Data, size_t DataLength ) {
    NullCheck( DeviceHandle, return );
    NullCheck( DeviceHandle->Framebuffer, return );
    NullCheck( Data, return );

    DataLength = DataLength > DeviceHandle->FramebufferSize ? DeviceHandle->FramebufferSize : DataLength;

    if ( DataLength > 0 ) {
        SSD1306_WriteData( DeviceHandle, Data, DataLength );
    }
}

void SSD1306_SetHFlip( struct SSD1306_Device* DeviceHandle, bool On ) {
    NullCheck( DeviceHandle, return );
    SSD1306_WriteCommand( DeviceHandle, ( On == true ) ? SSDCmd_Set_Display_HFlip_On : SSDCmd_Set_Display_HFlip_Off );
}

void SSD1306_SetVFlip( struct SSD1306_Device* DeviceHandle, bool On ) {
    NullCheck( DeviceHandle, return );
    SSD1306_WriteCommand( DeviceHandle, ( On == true ) ? SSDCmd_Set_Display_VFlip_On : SSDCmd_Set_Display_VFlip_Off );
}

void SSD1306_DrawPixel( struct SSD1306_Device* DeviceHandle, uint32_t X, uint32_t Y, bool Color ) {
    uint32_t YBit = ( Y & 0x07 );
    uint8_t* FBOffset = NULL;

    NullCheck( DeviceHandle, return );
    NullCheck( DeviceHandle->Framebuffer, return );

    /* 
     * We only need to modify the Y coordinate since the pitch
     * of the screen is the same as the width.
     * Dividing Y by 8 gives us which row the pixel is in but not
     * the bit position.
     */
    Y>>= 3;

    FBOffset = DeviceHandle->Framebuffer + ( ( Y * DeviceHandle->Width ) + X );
    *FBOffset = ( Color == true ) ? *FBOffset | BIT( YBit ) : *FBOffset & ~BIT( YBit );
}

void SSD1306_DrawHLine( struct SSD1306_Device* DeviceHandle, int x, int y, int x2, bool Color ) {
    NullCheck( DeviceHandle, return );
    NullCheck( DeviceHandle->Framebuffer, return );

    CheckBounds( x >= DeviceHandle->Width, return );
    CheckBounds( ( x2 + x ) >= DeviceHandle->Width, return );
    CheckBounds( y >= DeviceHandle->Height, return );

    for ( ; x <= x2; x++ ) {
        SSD1306_DrawPixel( DeviceHandle, x, y, Color );
    }
}

void SSD1306_DrawVLine( struct SSD1306_Device* DeviceHandle, int x, int y, int y2, bool Color ) {
    NullCheck( DeviceHandle, return );
    NullCheck( DeviceHandle->Framebuffer, return );

    CheckBounds( x >= DeviceHandle->Width, return );
    CheckBounds( y >= DeviceHandle->Height, return );
    CheckBounds( ( y2 + y ) >= DeviceHandle->Height, return );

    for ( ; y <= y2; y++ ) {
        SSD1306_DrawPixel( DeviceHandle, x, y, Color );
    }
}

void SSD1306_DrawRect( struct SSD1306_Device* DeviceHandle, int x, int y, int x2, int y2, bool Color ) {
    NullCheck( DeviceHandle, return );
    NullCheck( DeviceHandle->Framebuffer, return );

    CheckBounds( x >= DeviceHandle->Width, return );
    CheckBounds( ( x2 + x ) >= DeviceHandle->Width, return );
    CheckBounds( y >= DeviceHandle->Height, return );
    CheckBounds( ( y2 + y ) >= DeviceHandle->Height, return );

    for ( ; y <= y2; y++ ) {
        SSD1306_DrawHLine( DeviceHandle, x, y, x2, Color );
    }
}

void SSD1306_SetFont( struct SSD1306_Device* DeviceHandle, struct FontDef* FontHandle ) {
    NullCheck( DeviceHandle, return );
    NullCheck( FontHandle, return );
    NullCheck( FontHandle->Data, return );

    DeviceHandle->Font = FontHandle;
}

int SSD1306_Init_I2C( struct SSD1306_Device* DeviceHandle, int Address, int Width, int Height ) {
    if ( DeviceHandle == NULL ) {
        return 0;
    }

    memset( DeviceHandle, 0, sizeof( struct SSD1306_Device ) );

    DeviceHandle->Interface = SSD1306_IFACE_I2C;
    DeviceHandle->Address = Address;
    DeviceHandle->Width = Width;
    DeviceHandle->Height = Height;
    DeviceHandle->FramebufferSize = ( DeviceHandle->Width * Height ) / 8;
    DeviceHandle->Framebuffer = ( uint8_t* ) malloc( DeviceHandle->FramebufferSize );

    if ( DeviceHandle->Framebuffer == NULL ) {
        printf( "SSD1306: Failed to allocate %d bytes for a local framebuffer.\n", DeviceHandle->FramebufferSize );
        return 0;
    }

    memset( DeviceHandle->Framebuffer, 0, DeviceHandle->FramebufferSize );

    /* Init sequence according to SSD1306.pdf */
    SSD1306_SetMuxRatio( DeviceHandle, 0x3F );
    SSD1306_SetDisplayOffset( DeviceHandle, 0x00 );
    SSD1306_SetDisplayStartLine( DeviceHandle, 0 );
    SSD1306_SetHFlip( DeviceHandle, false );
    SSD1306_SetVFlip( DeviceHandle, false );
    SetCOMPinConfiguration( DeviceHandle, COM_Disable_LR_Remap, COM_Pins_Alternative, COM_ScanDir_LR );
    SSD1306_SetContrast( DeviceHandle, 0x7F );
    SSD1306_DisableDisplayRAM( DeviceHandle );
    SSD1306_SetInverted( DeviceHandle, false );
    SSD1306_SetDisplayClocks( DeviceHandle, 0, 8 );
    EnableChargePumpRegulator( DeviceHandle );
    SSD1306_SetDisplayAddressMode( DeviceHandle, AddressMode_Horizontal );
    SSD1306_EnableDisplayRAM( DeviceHandle );
    SSD1306_DisplayOn( DeviceHandle );
    SSD1306_Update( DeviceHandle );

    /* Small delay after init, maybe not needed but added to be safe */
    vTaskDelay( 100 / portTICK_PERIOD_MS );
    return 1;
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

        if ( SSD1306_Init_I2C( &TestDevice, 0x3C, 128, 64 ) == 1 ) {
            printf( "SSD1306 Init OK.\n" );

            /* SSD1306_SetInverted( &TestDevice, true ); */
            /* SSD1306_SetFont( &TestDevice, &Font_Liberation_Sans_15x16 ); */
            SSD1306_SetFont( &TestDevice, &Font_Liberation_Serif_19x19 );
            /* SSD1306_SetFont( &TestDevice, &Font_Ubuntu_Mono_6x10 ); */

            FontDrawAnchoredString( &TestDevice, "NE", TextAnchor_NorthEast, true );
            FontDrawAnchoredString( &TestDevice, "NW", TextAnchor_NorthWest, true );
            FontDrawAnchoredString( &TestDevice, "N", TextAnchor_North, true );
            FontDrawAnchoredString( &TestDevice, "E", TextAnchor_East, true );
            FontDrawAnchoredString( &TestDevice, "W", TextAnchor_West, true );
            FontDrawAnchoredString( &TestDevice, "SE", TextAnchor_SouthEast, true );
            FontDrawAnchoredString( &TestDevice, "SW", TextAnchor_SouthWest, true );
            FontDrawAnchoredString( &TestDevice, "S", TextAnchor_South, true );
            FontDrawAnchoredString( &TestDevice, "C", TextAnchor_Center, true );

            SSD1306_Update( &TestDevice );                                  
        }
    }
}
