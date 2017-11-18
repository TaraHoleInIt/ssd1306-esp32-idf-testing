#ifndef _SSD1306_H_
#define _SSD1306_H_

#ifndef BIT
#define BIT( n ) ( 1 << n )
#endif

#define CheckBounds( expr, retexpr ) { \
    if ( expr ) { \
        printf( "[%s:%d] %s\n", __FUNCTION__, __LINE__, #expr ); \
        retexpr; \
    } \
}

#define NullCheck( ptr, retexpr ) { \
    if ( ptr == NULL ) { \
        printf( "%s: %s == NULL\n", __FUNCTION__, #ptr ); \
        retexpr; \
    }; \
}

typedef enum {
    SSDCmd_Set_Contrast = 0x81,
    SSDCmd_Set_Display_Show_RAM = 0xA4,
    SSDCmd_Set_Display_Ignore_RAM = 0xA5,
    SSDCmd_Set_Normal_Display = 0xA6,
    SSDCmd_Set_Inverted_Display = 0xA7,
    SSDCmd_Set_Display_Off = 0xAE,
    SSDCmd_Set_Display_On = 0xAF,
    SSDCmd_Set_Memory_Addressing_Mode = 0x20,
    SSDCmd_Set_Mux_Ratio = 0xA8,
    SSDCmd_Nop = 0xE3,
    SSDCmd_Set_Display_Offset = 0xD3,
    SSDCmd_Set_Display_Start_Line = 0x40,
    SSDCmd_Set_Display_HFlip_Off = 0xA0,
    SSDCmd_Set_Display_HFlip_On = 0xA1,
    SSDCmd_Set_Display_VFlip_Off = 0xC0,
    SSDCmd_Set_Display_VFlip_On = 0xC8,
    SSDCmd_Set_COM_Pin_Config = 0xDA,
    SSDCmd_Set_Display_CLK = 0xD5,
    SSDCmd_Enable_Charge_Pump_Regulator = 0x8D
} SSDCmd;

typedef enum {
    AddressMode_Horizontal = 0,
    AddressMode_Vertical,
    AddressMode_Page,
    AddressMode_Invalid
} SSD1306_AddressMode;

typedef enum {
    SSD1306_IFACE_I2C = 1,
    SSD1306_IFACE_SPI
} SSD1306_Interface;

/*
typedef void ( *SSD1306_WriteCommandFn ) ( SSD1306_Device* DeviceHandle, SSDCmd Command );
typedef void ( *SSD1306_WriteDataFn ) ( SSD1306_Device* DeviceHandle, uint8_t* Data, size_t Length );
*/

struct FontDef;

struct SSD1306_Device {
    /* I2C Specific */
    int Address;

    /* SPI Specific */
    int RSTPin;
    int CSPin;
    int DCPin;

    /* Everything else */
    int Interface;
    int Width;
    int Height;
    int Pitch;

    uint8_t* Framebuffer;
    int FramebufferSize;
/*
    SSD1306_WriteCommandFn WriteCommand;
    SSD1306_WriteDataFn WriteData;
*/

    struct FontDef* Font;
};

void SSD1306_SetMuxRatio( struct SSD1306_Device* DeviceHandle, uint8_t Ratio );
void SSD1306_SetDisplayOffset( struct SSD1306_Device* DeviceHandle, uint8_t Offset );
void SSD1306_SetDisplayStartLines( struct SSD1306_Device* DeviceHandle );
void SSD1306_SetSegmentRemap( struct SSD1306_Device* DeviceHandle, bool Remap );
void SSD1306_SetContrast( struct SSD1306_Device* DeviceHandle, uint8_t Contrast );
void SSD1306_EnableDisplayRAM( struct SSD1306_Device* DeviceHandle );
void SSD1306_DisableDisplayRAM( struct SSD1306_Device* DeviceHandle );
void SSD1306_SetInverted( struct SSD1306_Device* DeviceHandle, bool Inverted );
void SSD1306_DisplayOn( struct SSD1306_Device* DeviceHandle );
void SSD1306_DisplayOff( struct SSD1306_Device* DeviceHandle ); 
void SSD1306_SetDisplayAddressMode( struct SSD1306_Device* DeviceHandle, SSD1306_AddressMode AddressMode );
void SSD1306_Update( struct SSD1306_Device* DeviceHandle );
void SSD1306_SetDisplayClocks( struct SSD1306_Device* DeviceHandle, uint32_t DisplayClockDivider, uint32_t OSCFrequency );
void SSD1306_WriteRawData( struct SSD1306_Device* DeviceHandle, uint8_t* Data, size_t DataLength );

void SSD1306_DrawPixel( struct SSD1306_Device* DeviceHandle, uint32_t X, uint32_t Y, bool Color );
void SSD1306_DrawHLine( struct SSD1306_Device* DeviceHandle, int x, int y, int x2, bool Color );
void SSD1306_DrawVLine( struct SSD1306_Device* DeviceHandle, int x, int y, int y2, bool Color );
void SSD1306_DrawRect( struct SSD1306_Device* DeviceHandle, int x, int y, int x2, int y2, bool Color );
void SSD1306_SetFont( struct SSD1306_Device* DeviceHandle, struct FontDef* FontHandle );

#endif
