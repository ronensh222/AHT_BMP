
#ifndef _I2C_DRV_H_
#define _I2C_DRV_H_

#define I2C_MASTER_SCL_IO           22          /*!< gpio number for I2C master clock IO21*/
#define I2C_MASTER_SDA_IO           21          /*!< gpio number for I2C master data  IO15*/
#define I2C_MASTER_NUM              I2C_NUM_1   /*!< I2C port number for master bme280 */
#define I2C_MASTER_FREQ_HZ          100000      /*!< I2C master clock frequency */
typedef i2c_bus_handle_t 			I2C_Drv_BUS_handle_t; /*!< i2c bus handle */
typedef i2c_bus_device_handle_t 	I2C_Drv_Device_handle_t	; /*!< i2c device handle */


#ifdef __cplusplus
extern "C"
{
#endif
#include "Error.h"

I2C_Drv_BUS_handle_t I2C_DrvBUS_Init(i2c_port_t PortNum, int PinSDA, int PinSCL, uint32_t Clk, i2c_bus_handle_t *BUS );

I2C_Drv_Device_handle_t I2C_Drv_Device_Init(i2c_bus_handle_t BUS, uint8_t Addr );

Error_t I2C_Drv_WriteRegister ( void *Handle,uint8_t Addr, uint8_t Reg,uint8_t DataLen,uint8_t *Data);

Error_t I2C_Drv_ReadRegister ( void *Handle,uint8_t Addr, uint8_t Reg,uint8_t DataLen,uint8_t *Data);

Error_t I2C_Drv_Write ( void *Handle,uint8_t Addr, uint8_t DataLen,uint8_t *Data);

Error_t I2C_Drv_Read ( void *Handle,uint8_t Addr, uint8_t DataLen,uint8_t *Data);

void I2C_Drv_Delay(uint32_t V);



#ifdef __cplusplus
}
#endif

#endif
