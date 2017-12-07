/**
 * Copyright (c) 2017 Tara Keeling
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <driver/i2c.h>

#include "ina226.h"

static const int USE_THIS_I2C_PORT = 1;
static int Address = 0;

#define RangeCheck( value, min, max, retexpr ) { \
    if ( value < min || value > max ) { \
        printf( "ERROR %s: %s out of range. Got %d, expected [%d to %d]\n", __FUNCTION__, #value, value, min, max ); \
        retexpr; \
    } \
}

static bool INA226_SetRegisterPointer( INA226_Reg Register ) {
    i2c_cmd_handle_t CommandHandle = NULL;
    esp_err_t Result = ESP_FAIL;

    if ( ( CommandHandle = i2c_cmd_link_create( ) ) ) {
        i2c_master_start( CommandHandle );
            i2c_master_write_byte( CommandHandle, ( Address << 1 ) | I2C_MASTER_WRITE, true );
            i2c_master_write_byte( CommandHandle, ( uint8_t ) Register, true );
        i2c_master_stop( CommandHandle );

        Result = i2c_master_cmd_begin( USE_THIS_I2C_PORT, CommandHandle, pdMS_TO_TICKS( 1000 ) );
        i2c_cmd_link_delete( CommandHandle );
    }

    return ( Result == ESP_OK ) ? true : false;    
}

bool INA226_WriteReg( INA226_Reg Register, uint16_t Value ) {
    i2c_cmd_handle_t CommandHandle = NULL;
    esp_err_t Result = ESP_FAIL;
    
    if ( ( CommandHandle = i2c_cmd_link_create( ) ) ) {
        i2c_master_start( CommandHandle );
            i2c_master_write_byte( CommandHandle, ( Address << 1 ) | I2C_MASTER_WRITE, true );
            i2c_master_write_byte( CommandHandle, ( uint8_t ) Register, true );
            i2c_master_write_byte( CommandHandle, ( uint8_t ) ( Value >> 8 ), true );
            i2c_master_write_byte( CommandHandle, ( uint8_t ) Value, true );
        i2c_master_stop( CommandHandle );

        Result = i2c_master_cmd_begin( USE_THIS_I2C_PORT, CommandHandle, pdMS_TO_TICKS( 1000 ) );
        i2c_cmd_link_delete( CommandHandle );
    }

    return ( Result == ESP_OK ) ? true : false;
}

uint16_t INA226_ReadReg16( INA226_Reg Register ) {
    i2c_cmd_handle_t CommandHandle = NULL;
    esp_err_t Result = ESP_FAIL;
    uint8_t Value_lo = 0;
    uint8_t Value_hi = 0;

    if ( INA226_SetRegisterPointer( Register ) == true ) {
        vTaskDelay( pdMS_TO_TICKS( 1 ) );

        if ( ( CommandHandle = i2c_cmd_link_create( ) ) ) {
            i2c_master_start( CommandHandle );
                i2c_master_write_byte( CommandHandle, ( Address << 1 ) | I2C_MASTER_READ, true );
                i2c_master_read_byte( CommandHandle, &Value_hi, false );
                i2c_master_read_byte( CommandHandle, &Value_lo, false );
            i2c_master_stop( CommandHandle );

            Result = i2c_master_cmd_begin( USE_THIS_I2C_PORT, CommandHandle, pdMS_TO_TICKS( 1000 ) );
            i2c_cmd_link_delete( CommandHandle );

            if ( Result == ESP_OK ) {
                return ( ( Value_hi << 8 ) | Value_lo );
            }
        }
    }

    return 0xBAAD;
}

uint16_t INA226_GetManufacturerId( void ) {
    return INA226_ReadReg16( INA226_Reg_ManufacturerId );
}

uint16_t INA226_GetDieId( void ) {
    return INA226_ReadReg16( INA226_Reg_DieId );
}

uint16_t INA226_ReadConfig( void ) {
    return INA226_ReadReg16( INA226_Reg_Cfg );
}

void INA226_WriteConfig( uint16_t Config ) {
    INA226_WriteReg( INA226_Reg_Cfg, Config );
}

void INA226_Reset( void ) {
    INA226_WriteConfig( INA226_ReadConfig( ) | INA226_CFG_Reset );
}

INA226_AveragingMode INA226_GetAveragingMode( void ) {
    uint16_t CurrentConfig = 0;

    CurrentConfig = INA226_ReadConfig( );
    CurrentConfig>>= INA226_CFG_AveragingOffset;
    CurrentConfig &= 0x07;

    return ( INA226_AveragingMode ) CurrentConfig;
}

void INA226_SetAveragingMode( INA226_AveragingMode Mode ) {
    uint16_t CurrentConfig = 0;

    RangeCheck( Mode, 0, INA226_Num_Averages, return );

    CurrentConfig = INA226_ReadConfig( );
    CurrentConfig &= ~INA226_CFG_AveragingMask;
    CurrentConfig |= ( Mode << INA226_CFG_AveragingOffset );

    INA226_WriteReg( INA226_Reg_Cfg, CurrentConfig );
}

INA226_ConversionTime INA226_GetBusVoltageConversionTime( void ) {
    uint16_t CurrentConfig = 0;

    CurrentConfig = INA226_ReadConfig( );
    CurrentConfig>>= INA226_CFG_BusVoltageTimeOffset;
    CurrentConfig&= 0x07;

    return ( INA226_ConversionTime ) CurrentConfig;    
}

void INA226_SetBusVoltageConversionTime( INA226_ConversionTime ConversionTime ) {
    uint16_t CurrentConfig = 0;

    RangeCheck( ConversionTime, 0, INA226_Num_ConversionTimes, return );

    CurrentConfig = INA226_ReadConfig( );
    CurrentConfig &= ~INA226_CFG_BusVoltageTimeMask;
    CurrentConfig |= ( ConversionTime << INA226_CFG_BusVoltageTimeOffset );

    INA226_WriteReg( INA226_Reg_Cfg, CurrentConfig );
}

INA226_ConversionTime INA226_GetShuntVoltageConversionTime( void ) {
    uint16_t CurrentConfig = 0;

    CurrentConfig = INA226_ReadConfig( );
    CurrentConfig>>= INA226_CFG_ShuntVoltageTimeOffset;
    CurrentConfig&= 0x07;

    return ( INA226_ConversionTime ) CurrentConfig;     
}

void INA226_SetShuntVoltageConversionTime( INA226_ConversionTime ConversionTime ) {
    uint16_t CurrentConfig = 0;

    RangeCheck( ConversionTime, 0, INA226_Num_ConversionTimes, return );

    CurrentConfig = INA226_ReadConfig( );
    CurrentConfig &= ~INA226_CFG_ShuntVoltageTimeMask;
    CurrentConfig |= ( ConversionTime << INA226_CFG_ShuntVoltageTimeOffset );

    INA226_WriteReg( INA226_Reg_Cfg, CurrentConfig );
}

INA226_Mode INA226_GetOperatingMode( void ) {
    uint16_t CurrentConfig = 0;

    CurrentConfig = INA226_ReadConfig( );
    CurrentConfig&= 0x07;

    return ( INA226_Mode ) CurrentConfig;        
}

void INA226_SetOperatingMode( INA226_Mode Mode ) {
    uint16_t CurrentConfig = 0;

    RangeCheck( Mode, 0, INA226_Num_Modes, return );

    CurrentConfig = INA226_ReadConfig( );
    CurrentConfig &= ~INA226_CFG_ModeMask;
    CurrentConfig |= Mode;

    INA226_WriteReg( INA226_Reg_Cfg, CurrentConfig );    
}

uint32_t INA226_GetBusVoltage( void ) {
    uint32_t Data = 0;

    Data = INA226_ReadReg16( INA226_Reg_BusVolage );
    Data*= 125;

    return Data / 100;
}

bool INA226_Init( int I2CAddress ) {
    Address = I2CAddress;

    INA226_Reset( );
    vTaskDelay( pdMS_TO_TICKS( 10 ) );

    return true;
}
