#ifndef _INA226_H_
#define _INA226_H_

#if ! defined BIT
#define BIT( n ) ( 1 << n )
#endif

typedef enum {
    INA226_Reg_Cfg = 0x00,
    INA226_Reg_ShuntVoltage,
    INA226_Reg_BusVolage,
    INA226_Reg_AvgPower,
    INA226_Reg_AvgCurrent,
    INA226_Reg_Calibration,
    INA226_Reg_AlertMask,
    INA226_Reg_AlertLimit,
    INA226_Reg_ManufacturerId = 0xFE,
    INA226_Reg_DieId
} INA226_Reg;

typedef enum {
    INA226_Averages_1 = 0,
    INA226_Averages_4,
    INA226_Averages_16,
    INA226_Averages_64,
    INA226_Averages_128,
    INA226_Averages_256,
    INA226_Averages_512,
    INA226_Averages_1024,
    INA226_Num_Averages = 7
} INA226_AveragingMode;

typedef enum {
    INA226_ConversionTime_140us = 0,
    INA226_ConversionTime_204us,
    INA226_ConversionTime_332us,
    INA226_ConversionTime_588us,
    INA226_ConversionTime_1_1ms,
    INA226_ConversionTime_2_116ms,
    INA226_ConversionTime_4_156ms,
    INA226_ConversionTime_8_244ms,
    INA226_Num_ConversionTimes = 7
} INA226_ConversionTime;

typedef enum {
    INA226_Mode_Shutdown = 0,
    INA226_Mode_ShuntVoltage_Triggered,
    INA226_Mode_BusVoltage_Triggered,
    INA226_Mode_ShuntAndBus_Triggered,
    INA226_Mode_Shutdown2,
    INA226_Mode_ShuntVoltage_Continuous,
    INA226_Mode_BusVoltage_Continuous,
    INA226_Mode_ShuntAndBus_Continuous,
    INA226_Num_Modes = 7
} INA226_Mode;

#define INA226_CFG_Reset BIT( 15 )

#define INA226_CFG_AveragingMask ( BIT( 9 ) | BIT( 10 ) | BIT( 11 ) )
#define INA226_CFG_AveragingOffset 9

#define INA226_CFG_BusVoltageTimeMask ( BIT( 6 ) | BIT( 7 ) | BIT( 8 ) )
#define INA226_CFG_BusVoltageTimeOffset 6

#define INA226_CFG_ShuntVoltageTimeMask ( BIT( 3 ) | BIT( 4 ) | BIT( 5 ) )
#define INA226_CFG_ShuntVoltageTimeOffset 3

#define INA226_CFG_ModeMask ( BIT( 0 ) | BIT( 1 ) | BIT( 2 ) )

bool INA226_Init( int Address );
bool INA226_WriteReg( INA226_Reg Register, uint16_t Value );
uint16_t INA226_ReadReg16( INA226_Reg Register );

uint16_t INA226_GetManufacturerId( void );
uint16_t INA226_GetDieId( void );
uint16_t INA226_ReadCfg( void );

void INA226_Reset( void );

INA226_AveragingMode INA226_GetAveragingMode( void );
void INA226_SetAveragingMode( INA226_AveragingMode Mode );

INA226_ConversionTime INA226_GetBusVoltageConversionTime( void );
void INA226_SetBusVoltageConversionTime( INA226_ConversionTime ConversionTime );

INA226_ConversionTime INA226_GetShuntVoltageConversionTime( void );
void INA226_SetShuntVoltageConversionTime( INA226_ConversionTime ConversionTime );

INA226_Mode INA226_GetOperatingMode( void );
void INA226_SetOperatingMode( INA226_Mode Mode );

uint32_t INA226_GetBusVoltage( void );

#endif
