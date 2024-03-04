#include "driver/i2c.h"
#include "i2c_bus.h"
#include "I2C_Drv.h"
#include "Error.h"

#define I2C_MASTER_TX_BUF_DISABLE   0           /*!< I2C master do not need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0           /*!< I2C master do not need buffer */

#define I2C_TIMEOUT 				500 / portTICK_PERIOD_MS

I2C_Drv_BUS_handle_t I2C_DrvBUS_Init(i2c_port_t PortNum, int PinSDA, int PinSCL, uint32_t Clk, i2c_bus_handle_t *BUS )
{
	i2c_config_t conf =
	{
			.mode = I2C_MODE_MASTER,
			.sda_io_num = PinSDA,
			.sda_pullup_en = GPIO_PULLUP_ENABLE,
			.scl_io_num = PinSCL,
			.scl_pullup_en = GPIO_PULLUP_ENABLE,
			.master.clk_speed = Clk,
	};
	return ( i2c_bus_create(PortNum, &conf));

}



I2C_Drv_Device_handle_t I2C_Drv_Device_Init(i2c_bus_handle_t BUS, uint8_t Addr )
{
	return( i2c_bus_device_create(BUS, Addr, i2c_bus_get_current_clk_speed(BUS)) );
}




Error_t I2C_Drv_WriteRegister ( void *Handle,uint8_t Addr, uint8_t Reg,uint8_t DataLen,uint8_t *Data)
{

	Error_t ret;
	ret = (ESP_OK == i2c_bus_write_bytes(Handle, Reg,DataLen ,Data)  ) ? ERROR_OK : ERROR_FAIL;
	return ret;
}





Error_t I2C_Drv_ReadRegister ( void *Handle,uint8_t Addr, uint8_t Reg,uint8_t DataLen,uint8_t *Data)
{
	Error_t ret;

	ret = ( ESP_OK == i2c_bus_read_bytes(Handle, Reg,DataLen ,Data) ) ? ERROR_OK : ERROR_FAIL;
	return ret;
}


Error_t I2C_Drv_Write ( void *Handle,uint8_t Addr, uint8_t DataLen,uint8_t *Data)
{
	Error_t ret;
	ret = (ESP_OK == i2c_bus_write_bytes(Handle, NULL_I2C_MEM_ADDR,DataLen ,Data) ) ? ERROR_OK : ERROR_FAIL;
	return ret;
}

Error_t I2C_Drv_Read ( void *Handle,uint8_t Addr, uint8_t DataLen,uint8_t *Data)
{
	Error_t ret;

	ret = (ESP_OK == i2c_bus_read_bytes(Handle, NULL_I2C_MEM_ADDR,DataLen ,Data) ) ? ERROR_OK : ERROR_FAIL;
	return ret;
}


void I2C_Drv_Delay(uint32_t V)
{
	vTaskDelay(V);
};


