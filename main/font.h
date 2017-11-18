#ifndef _FONT_H_
#define _FONT_H_

#include <stdbool.h>

struct FontDef {
    const uint8_t* Data;
    int Width;  /* Width in bytes */
    int Height; /* Height in bytes */
    int StartChar;  /* Which ascii # the font starts with (usually a space or !) */
    int EndChar;    /* Ending ascii # the font ends with */
};

typedef enum {
    TextAnchor_East = 0,
    TextAnchor_West,
    TextAnchor_North,
    TextAnchor_South,
    TextAnchor_NorthEast,
    TextAnchor_NorthWest,
    TextAnchor_SouthEast,
    TextAnchor_SouthWest,
    TextAnchor_Center
} TextAnchor;

extern struct FontDef Font_Comic_Sans_MS_12x16;
extern struct FontDef Font_Impact_13x16;
extern struct FontDef Font_Terminal_8x12;

struct SSD1306_Device;

int FontGetCharHeight( struct FontDef* FontHandle, char c );
int FontMeasureString( struct FontDef* FontHandle, const char* Text );
int FontGetCharWidth( struct FontDef* FontHandle, char c );

void FontDrawChar( struct SSD1306_Device* DeviceHandle, char c, int x, int y, bool Color );
void FontDrawString( struct SSD1306_Device* DeviceHandle, const char* Text, int x, int y, bool Color );

void FontDrawCharUnaligned( struct SSD1306_Device* DeviceHandle, char c, int x, int y, bool Color );
void FontDrawStringUnaligned( struct SSD1306_Device* DeviceHandle, const char* Text, int x, int y, bool Color );
void FontDrawAnchoredString( struct SSD1306_Device* DeviceHandle, const char* Text, TextAnchor Anchor , bool Color );

#endif
