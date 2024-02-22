#include <stdio.h>
#include <stdbool.h>
#include "AHT_BMP.h"
/* ------------------------------------AHTx ------------------------------- */
#define CALIB_RTRY  				5 
#define READ_RTRY					5		

#define AHTx_AHTx_ID (0x60)

#define AHTx_REGISTER_SOFTRESET		0xBA       /*!< Soft reset register */
#define AHTx_REGISTER_STATUS		0x71
#define AHT2X_REGISTER_CALIBRATE    0xBE  /*!<initialization register, for AHT2x only */
#define AHTx_REGISTER_TRIGGER 		0xAC       /*!< Trigger reading command */

#define AHTX0_STATUS_BUSY 			0x80      /*!< Status bit for busy */
#define AHTX0_STATUS_CALIBRATED 	0x08 /*!< Status bit for calibrated */
/* ---------------------------------------------------------------------- */

/* ---------------------------------BMP280---------------------------- */
#define BMP280_ID					0x58
#define BMP280_REG_T1				0x88
#define BMP280_REG_T2				0x8A
#define BMP280_REG_T3				0x8C

#define BMP280_REG_P1				0x8E
#define BMP280_REG_P2				0x90
#define BMP280_REG_P3				0x92
#define BMP280_REG_P4				0x94
#define BMP280_REG_P5				0x96
#define BMP280_REG_P6				0x98
#define BMP280_REG_P7				0x9A
#define BMP280_REG_P8				0x9C
#define BMP280_REG_P9				0x9E


#define BMP280_REG_CHIPID			0xD0
#define BMP280_REG_VERSION			0xD1
#define BMP280_REG_SOFTRESET		0xE0

#define BMP280_REG_CAL26			0xE1  // R calibration stored in 0xE1-0xF0

#define BMP280_REG_CONTROLHUMID     0xF2
#define BMP280_REG_STATUS           0XF3
#define BMP280_REG_CONTROL			0xF4
#define BMP280_REG_CONFIG			0xF5
#define BMP280_REG_PRESSUREDATA		0xF7
#define BMP280_REG_TEMPDATA			0xFA


typedef struct
{
    uint16_t t1;
    int16_t t2;
    int16_t t3;

    uint16_t p1;
    int16_t	p2;
    int16_t p3;
    int16_t p4;
    int16_t p5;
    int16_t p6;
    int16_t p7;
    int16_t p8;
    int16_t p9;


} BMP280_Coffdata_t;

typedef enum {
    BME280_SAMPLING_NONE = 0b000,
    BME280_SAMPLING_X1 = 0b001,
    BME280_SAMPLING_X2 = 0b010,
    BME280_SAMPLING_X4 = 0b011,
    BME280_SAMPLING_X8 = 0b100,
    BME280_SAMPLING_X16 = 0b101
} BMP280_SensorSamp_t;

typedef enum {
    BME280_MODE_SLEEP 	= 0b00,
    BME280_MODE_FORCED 	= 0b01,
    BME280_MODE_NORMAL 	= 0b11
} BMP280_SensorMode_t;

typedef enum {
    BME280_FILTER_OFF 	= 0b000,
    BME280_FILTER_X2 	= 0b001,
    BME280_FILTER_X4	= 0b010,
    BME280_FILTER_X8 	= 0b011,
    BME280_FILTER_X16 	= 0b100
} BMP280_SensorFilter_t;

// standby durations in ms
typedef enum {
    BME280_STANDBY_MS_0_5 = 0b000,
    BME280_STANDBY_MS_10 = 0b110,
    BME280_STANDBY_MS_20 = 0b111,
    BME280_STANDBY_MS_62_5 = 0b001,
    BME280_STANDBY_MS_125 = 0b010,
    BME280_STANDBY_MS_250 = 0b011,
    BME280_STANDBY_MS_500 = 0b100,
    BME280_STANDBY_MS_1000 = 0b101
} BMP280_Sdandby_t;




// The ctrl_meas register
typedef struct
{
    // temperature oversampling
    // 000 = skipped
    // 001 = x1
    // 010 = x2
    // 011 = x4
    // 100 = x8
    // 101 and above = x16
    unsigned int osrs_t : 3;

    // pressure oversampling
    // 000 = skipped
    // 001 = x1
    // 010 = x2
    // 011 = x4
    // 100 = x8
    // 101 and above = x16
    unsigned int osrs_p : 3;

    // device mode
    // 00       = sleep
    // 01 or 10 = forced
    // 11       = normal
    unsigned int mode : 2;
} BMP280_CtrlMeas_t;

typedef struct
{
    // inactive duration (standby time) in normal mode
    // 000 = 0.5 ms
    // 001 = 62.5 ms
    // 010 = 125 ms
    // 011 = 250 ms
    // 100 = 500 ms
    // 101 = 1000 ms
    // 110 = 10 ms
    // 111 = 20 ms
    unsigned int t_sb : 3;

    // filter settings
    // 000 = filter off
    // 001 = 2x filter
    // 010 = 4x filter
    // 011 = 8x filter
    // 100 and above = 16x filter
    unsigned int filter : 3;

    // unused - don't set
    unsigned int none : 1;
    unsigned int spi3w_en : 1;
} BMP280_Config_t;

/* ---------------------------------------------------------------------- */

 
typedef struct
{
 void *AHThandle;
 void *BMPhandle;
 uint8_t AHTaddr;
 uint8_t BMPaddr;
 I2C_WriteRegisterFunc I2C_WriteRegister;
 I2C_ReadRegisterFunc  I2C_ReadRegister;
 
 I2C_WriteFunc I2C_Write;
 I2C_ReadFunc  I2C_Read;
 DelayFunc		Delay;
}AHT_BMP_dev_t;	
 

static BMP280_Coffdata_t	BMP280_Coffdata;
static AHT_BMP_dev_t 		AHT_BMP_dev;
static BMP280_CtrlMeas_t   	BMP280_CtrlMeas;
static BMP280_Config_t 	   	BMP280_Config;




static  AHT_BMP_Error_t AHT_BMP__BMPRead16Reg(bool Le, uint8_t addr, uint16_t *data)
{
	uint8_t data0, data1;
	AHT_BMP_Error_t rc =AHT_BMP_FAIL;
	if ( AHT_BMP_FAIL ==  AHT_BMP_dev.I2C_ReadRegister(AHT_BMP_dev.BMPhandle,AHT_BMP_dev.BMPaddr, addr, 1,  &data0))
	{
		return rc;
	}
	if ( AHT_BMP_FAIL == AHT_BMP_dev.I2C_ReadRegister(AHT_BMP_dev.BMPhandle,AHT_BMP_dev.BMPaddr, addr+1, 1, &data1))
	{
		return rc;
	}
	rc = AHT_BMP_SUCESS;

		if(Le)
	{
		*data = (data1 << 8) | data0;
	}
	else
	{
		*data = (data0 << 8) | data1;
	}

   return rc;
}

static bool AHT_BMP__BMP_Is_reading_calibration(void)
{
	uint8_t rstatus = 0;
    if (AHT_BMP_dev.I2C_ReadRegister(AHT_BMP_dev.BMPhandle,AHT_BMP_dev.BMPaddr, BMP280_REG_STATUS,1,&rstatus)  != AHT_BMP_SUCESS )
		{
        return false;
    }
    return (rstatus & (1 << 0)) != 0;
}


static AHT_BMP_Error_t AHT_BMP__BMP_CoffRead(void)
 {
	AHT_BMP_Error_t rc = AHT_BMP_SUCESS;
	uint16_t data16;

	rc |= AHT_BMP__BMPRead16Reg(true, BMP280_REG_T1, &data16);
	BMP280_Coffdata.t1 = data16;
	rc |= AHT_BMP__BMPRead16Reg(true, BMP280_REG_T2, &data16);
	BMP280_Coffdata.t2 = data16;
	rc |= AHT_BMP__BMPRead16Reg(true, BMP280_REG_T3, &data16);
	BMP280_Coffdata.t3 = data16;

	rc |= AHT_BMP__BMPRead16Reg(true, BMP280_REG_P1, &data16);
	BMP280_Coffdata.p1 = data16;
	rc |= AHT_BMP__BMPRead16Reg(true, BMP280_REG_P2, &data16);
	BMP280_Coffdata.p2 = data16;
	rc |= AHT_BMP__BMPRead16Reg(true, BMP280_REG_P3, &data16);
	BMP280_Coffdata.p3 = data16;
	rc |= AHT_BMP__BMPRead16Reg(true, BMP280_REG_P4, &data16);
	BMP280_Coffdata.p4 = data16;
	rc |= AHT_BMP__BMPRead16Reg(true, BMP280_REG_P5, &data16);
	BMP280_Coffdata.p5 = data16;
	rc |= AHT_BMP__BMPRead16Reg(true, BMP280_REG_P6, &data16);
	BMP280_Coffdata.p6 = data16;
	rc |= AHT_BMP__BMPRead16Reg(true, BMP280_REG_P7, &data16);
	BMP280_Coffdata.p7 = data16;
	rc |= AHT_BMP__BMPRead16Reg(true, BMP280_REG_P8, &data16);
	BMP280_Coffdata.p8 = data16;
	rc |= AHT_BMP__BMPRead16Reg(true, BMP280_REG_P9, &data16);
	BMP280_Coffdata.p9 = data16;
	return rc;
}

 static uint8_t AHT_BMP__BMP_GetConfig(BMP280_Config_t *pConfig)
 {
	 if (NULL == pConfig)
	 {
		 return 0;
	 }

		 return ( (pConfig->t_sb << 5) | (pConfig->filter << 3) |pConfig->spi3w_en);
 }

 static uint8_t AHT_BMP__BMP_CtrlMeas(BMP280_CtrlMeas_t *pMeas)
  {
 	 if (NULL == pMeas)
 	 {
 		 return 0;
 	 }

 	return ( (pMeas->osrs_t << 5) | (pMeas->osrs_p << 3) | pMeas->mode);
  }

 AHT_BMP_Error_t AHT_BMP__BMP_SetSamp(BMP280_SensorMode_t Mode,BMP280_SensorSamp_t TemSamp,BMP280_SensorSamp_t PressSamp,BMP280_SensorFilter_t Filter,BMP280_Sdandby_t Standby)
 {
	 AHT_BMP_Error_t rc 	= AHT_BMP_SUCESS;
	 uint8_t Cfg,Meas;
	 BMP280_CtrlMeas.mode 	= Mode;
	 BMP280_CtrlMeas.osrs_t = TemSamp;
	 BMP280_CtrlMeas.osrs_p = PressSamp;
	 BMP280_Config.filter   =Filter;
	 BMP280_Config.t_sb		= Standby;


	 Cfg = AHT_BMP__BMP_GetConfig(&BMP280_Config);
	 Meas = AHT_BMP__BMP_CtrlMeas(&BMP280_CtrlMeas);
	 AHT_BMP_dev.I2C_WriteRegister(AHT_BMP_dev.BMPhandle,AHT_BMP_dev.BMPaddr, BMP280_REG_SOFTRESET,1,&Cfg);
	 AHT_BMP_dev.I2C_WriteRegister(AHT_BMP_dev.BMPhandle,AHT_BMP_dev.BMPaddr, BMP280_REG_CONTROL,1,&Meas);



	 return rc;
 }



 AHT_BMP_Error_t AHT_BMP__take_forced_measurement(BMP280_CtrlMeas_t *pMeas)
 {
	 AHT_BMP_Error_t rc = AHT_BMP_FAIL;
	 uint8_t data = 0;
	 uint8_t Meas;
	 if (NULL == pMeas)
	 {
		 return rc;
	 }


	 if (pMeas->mode == BME280_MODE_FORCED)
	 {
		 // set to forced mode, i.e. "take next measurement"
		 Meas = AHT_BMP__BMP_CtrlMeas(&BMP280_CtrlMeas);
		 rc = AHT_BMP_dev.I2C_WriteRegister(AHT_BMP_dev.BMPhandle,AHT_BMP_dev.BMPaddr, BMP280_REG_CONTROL,1,&Meas);
		 if(AHT_BMP_SUCESS == rc )
		 {
			 do
			 {
				 rc = AHT_BMP_dev.I2C_ReadRegister(AHT_BMP_dev.BMPhandle,AHT_BMP_dev.BMPaddr, BMP280_REG_STATUS,1,&data);
				 AHT_BMP_dev.Delay(10);

			 }
			 while ((AHT_BMP_SUCESS == rc )  && (data & 0x08) );
		 }
	 }
	 return rc;
 }






AHT_BMP_Error_t AHT_BMP__Init(void *AHThandle, void *BMPhandle , uint8_t AHTaddr,uint8_t BMPaddr, I2C_WriteRegisterFunc WriteRegister, I2C_ReadRegisterFunc  ReadRegister, I2C_WriteFunc Write, I2C_ReadFunc Read, DelayFunc Delay)
{
	AHT_BMP_Error_t rc = AHT_BMP_FAIL;
	uint8_t Cmd[2] ={0};
	uint8_t Status;
	uint8_t BMP_ChipID;
	if( NULL == AHThandle )
	{
		return rc;
	}
	AHT_BMP_dev.AHThandle = AHThandle;

	if( NULL == BMPhandle )
		{
			return rc;
		}
	AHT_BMP_dev.BMPhandle = BMPhandle;

	AHT_BMP_dev.AHTaddr = AHTaddr;
	AHT_BMP_dev.BMPaddr = BMPaddr;

	
	if( NULL == WriteRegister ) 
	{
		return rc;
	}

	AHT_BMP_dev.I2C_WriteRegister = WriteRegister;
	
	if( NULL == ReadRegister ) 
	{
		return rc;
	}
	AHT_BMP_dev.I2C_ReadRegister  = ReadRegister;
	
	if( NULL == Write ) 
	{
		return rc;
	}
	AHT_BMP_dev.I2C_Write		  = Write;
	
	if( NULL == Read ) 
	{
		return rc;
	}
	AHT_BMP_dev.I2C_Read = Read;
	
	if( NULL == Delay )
 	{ 
		return rc;

	}		
	AHT_BMP_dev.Delay = Delay;


	/* -----------------------------AHTx settting start ------------------------ */
	AHT_BMP_dev.I2C_WriteRegister(AHT_BMP_dev.AHThandle,AHT_BMP_dev.AHTaddr, AHTx_REGISTER_SOFTRESET,0,Cmd);
	uint8_t CalibRetry = CALIB_RTRY; 	
	do
	{
		AHT_BMP_dev.I2C_ReadRegister(AHT_BMP_dev.AHThandle,AHT_BMP_dev.AHTaddr, AHTx_REGISTER_STATUS,1,&Status);
		if( 0 ==  (Status & AHTX0_STATUS_CALIBRATED ) )
		{ /* Need to calibrate */
			Cmd[0] = 0x08;
			Cmd[1] = 0x00;
			AHT_BMP_dev.I2C_WriteRegister(AHT_BMP_dev.AHThandle,AHT_BMP_dev.AHTaddr, AHT2X_REGISTER_CALIBRATE,2,Cmd);
			CalibRetry--;
		}
		AHT_BMP_dev.Delay(10);
		CalibRetry--;
		
	}while ( ( 0 ==  (Status & AHTX0_STATUS_CALIBRATED ))	&& ( CalibRetry > 0 ) );
	if(	0== (Status & AHTX0_STATUS_CALIBRATED )  )
	{
		return rc;
	}

	/* ----------------------------- AHTx settting finshed ------------------------ */

	/* ----------------------------- BMP setting  start --------------------------- */
	printf("BMP setting start ... \n");
		if ( AHT_BMP_FAIL ==  AHT_BMP_dev.I2C_ReadRegister(AHT_BMP_dev.BMPhandle,AHT_BMP_dev.BMPaddr, BMP280_REG_CHIPID,1,&BMP_ChipID) )
	    {
	    	return rc;
	    }
	    printf("BMP280 ID 0x%X\n",BMP_ChipID);
	    if(BMP_ChipID != BMP280_ID)
	    {
	    	return rc;
	    }
	    uint8_t rstv = 0xB6;
	    if ( AHT_BMP_FAIL ==  AHT_BMP_dev.I2C_WriteRegister(AHT_BMP_dev.BMPhandle,AHT_BMP_dev.BMPaddr, BMP280_REG_SOFTRESET,1,&rstv) )
	    {
	    	return rc;
	    }

	    // wait for chip to wake up.
	    AHT_BMP_dev.Delay(300);
	       // if chip is still reading calibration, delay
	       while (AHT_BMP__BMP_Is_reading_calibration())
	       {
	    	   AHT_BMP_dev.Delay(100);
	       }
	       if ( AHT_BMP_FAIL == AHT_BMP__BMP_CoffRead())
	       {
	    	   return rc;
	       }
	       if ( AHT_BMP_FAIL == AHT_BMP__BMP_SetSamp( BME280_MODE_NORMAL, BME280_SAMPLING_X16, BME280_SAMPLING_X16,  BME280_FILTER_OFF, BME280_STANDBY_MS_0_5))
	       {
	      	    	   return rc;
	       }
	/* ----------------------------- BMP settting finshed ------------------------ */
	    printf("BMP setting finished ... \n");
	    rc =  AHT_BMP_SUCESS;
	return rc;
}








AHT_BMP_Error_t AHT_BMP__AHT_TempHumidity(float *temperature, float *humidity)
{
	uint8_t Cmd[2] ={0x33, 0};
	uint8_t Status;
	uint8_t data[7];
    AHT_BMP_Error_t rc = AHT_BMP_FAIL;
	
	AHT_BMP_dev.I2C_WriteRegister(AHT_BMP_dev.AHThandle,AHT_BMP_dev.AHTaddr, AHTx_REGISTER_TRIGGER,2,Cmd);
	
	AHT_BMP_dev.Delay(80);

	int i = READ_RTRY;
	do
	{
		i--;

		AHT_BMP_dev.I2C_ReadRegister(AHT_BMP_dev.AHThandle,AHT_BMP_dev.AHTaddr, AHTx_REGISTER_STATUS,1,&Status);


	}
	while(i > 0 && (Status & AHTX0_STATUS_BUSY));
	if (0 == i)
	{
		return  rc;
	}
	
	AHT_BMP_dev.I2C_Read(AHT_BMP_dev.AHThandle, AHT_BMP_dev.AHTaddr, 6, data);

	uint32_t h = data[1];
	  h <<= 8;
	  h |= data[2];
	  h <<= 4;
	  h |= data[3] >> 4;
	  *humidity = ((float)h * 100) / 0x100000;

	  int32_t tdata = data[3] & 0x0F;
	  tdata <<= 8;
	  tdata |= data[4];
	  tdata <<= 8;
	  tdata |= data[5];
	 // _temperature = ((float)tdata * 200 / 0x100000) - 50;
	  *temperature =  ((float)tdata / 0x100000) * 200 - 50;

	return AHT_BMP_SUCESS;
}


AHT_BMP_Error_t AHT_BMP__BMP_Temp(float *temperature)
{
    int32_t var1, var2;
    uint8_t data[3] = { 0 };
    AHT_BMP_Error_t rc = AHT_BMP_FAIL;

   rc = AHT_BMP_dev.I2C_ReadRegister(AHT_BMP_dev.BMPhandle,AHT_BMP_dev.BMPaddr, BMP280_REG_TEMPDATA,3,data);

   if( AHT_BMP_SUCESS == rc )
    {
   	int32_t adc_T = (data[0] << 16) | (data[1] << 8) | data[2];


		 if (adc_T == 0x800000)
		 {      // value in case temp measurement was disabled
			 return AHT_BMP_FAIL;
		 }
		 adc_T >>= 4;

    var1 = ((((adc_T >> 3) - ((int32_t) BMP280_Coffdata.t1 << 1)))
            * ((int32_t) BMP280_Coffdata.t2)) >> 11;

    var2 = (((((adc_T >> 4) - ((int32_t) BMP280_Coffdata.t1))
              * ((adc_T >> 4) - ((int32_t) BMP280_Coffdata.t1))) >> 12)
            * ((int32_t) BMP280_Coffdata.t3)) >> 14;

    int32_t fine = var1 + var2;
    *temperature = ((fine * 5 + 128) >> 8) / 100.0;
    }
    return rc;
}

