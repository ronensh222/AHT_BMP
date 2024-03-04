
#ifndef _AHT_BMP_H_
#define _AHT_BMP_H_
#include "Error.h"



#define AHTx_I2C_ADDRESS_DEFAULT   (0x38)     /*The AHT device's I2C address */
#define BMP280_I2C_ADDRESS_DEFAULT (0x77)



#ifdef __cplusplus
extern "C"
{
#endif


typedef Error_t (*I2C_WriteRegisterFunc)(void *, uint8_t, uint8_t, uint8_t, uint8_t * );  /* I2C_WriteRegisterFunc(* void Handle,uint8_t Addr, uint8_t Reg,uint8_t DataLen,uint8_t *Data) */
typedef Error_t (*I2C_ReadRegisterFunc)(void *, uint8_t, uint8_t, uint8_t, uint8_t *  );   /* I2C_ReadRegisterFunc(* void Handle,uint8_t Addr, uint8_t Reg,uint8_t DataLen,uint8_t *Data) */
typedef Error_t (*I2C_WriteFunc)(void *, uint8_t, uint8_t, uint8_t *);						/* I2C_WriteFunc(* void Handle,uint8_t Addr, uint8_t DataLen,uint8_t *Data) */
typedef Error_t (*I2C_ReadFunc)(void *,uint8_t, uint8_t, uint8_t * );						/* I2C_ReadFunc(* void Handle,uint8_t Addr, uint8_t DataLen,uint8_t *Data) */
typedef void (*DelayFunc)(uint32_t); /* Just delay function */




Error_t AHT_BMP__Init(void *AHThandle, void *BMPhandle , uint8_t AHTaddr,uint8_t BMPaddr, I2C_WriteRegisterFunc WriteRegister, I2C_ReadRegisterFunc  ReadRegister, I2C_WriteFunc Write, I2C_ReadFunc Read, DelayFunc Delay);

Error_t AHT_BMP__AHT_TempHumidity(float *temperature, float *humidity);
Error_t AHT_BMP__BMP_Temp(float *temperature);

#ifdef __cplusplus
}
#endif

#endif
