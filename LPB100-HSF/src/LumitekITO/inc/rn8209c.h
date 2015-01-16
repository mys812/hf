#ifndef __DEVICE_RN8209C_H__
#define __DEVICE_RN8209C_H__

#include <hsf.h>

#ifdef RN8209C_SUPPORT


// user 100W to calibrater
#define RN8209C_CALI_POWER_BY_100W

//Uart define
#define RN8209C_UART_NO			0
#define RN8209C_UART_TIMEOUT	5000
#define RN9029C_MAX_DATA_LEN	100

//#define RN8209C_READ_DATA_TIMEOUT	8000
#define RN8209C_READ_DATA_TIMEOUT	5000


/**********************************************************************************
100W  224.1V   0.454A
07:52:56 === reco_irms=16295 reco_urms=1037976 reco_freq=50 reco_powerp=516089 reco_powerq=-4693 reco_energyp=10 reco_energyq=0
07:53:00 === reco_irms=16320 reco_urms=1037973 reco_freq=50 reco_powerp=516461 reco_powerq=-5881 reco_energyp=10 reco_energyq=0
07:53:04 === reco_irms=16327 reco_urms=1037972 reco_freq=50 reco_powerp=516681 reco_powerq=-6174 reco_energyp=11 reco_energyq=0
07:53:07 === reco_irms=16345 reco_urms=1037965 reco_freq=50 reco_powerp=517268 reco_powerq=-4895 reco_energyp=11 reco_energyq=0
07:53:11 === reco_irms=16317 reco_urms=1037935 reco_freq=50 reco_powerp=516362 reco_powerq=-5571 reco_energyp=12 reco_energyq=0
07:53:15 === reco_irms=16315 reco_urms=1038011 reco_freq=50 reco_powerp=516381 reco_powerq=-5082 reco_energyp=12 reco_energyq=0
07:53:19 === reco_irms=16320 reco_urms=1037996 reco_freq=50 reco_powerp=516474 reco_powerq=-4779 reco_energyp=12 reco_energyq=0
07:53:23 === reco_irms=16333 reco_urms=1037961 reco_freq=50 reco_powerp=517106 reco_powerq=-5509 reco_energyp=13 reco_energyq=0

200W 224.1V  0.897A
07:55:38 === reco_irms=32492 reco_urms=1036638 reco_freq=50 reco_powerp=1028417 reco_powerq=-8074 reco_energyp=27 reco_energyq=0
07:55:42 === reco_irms=32502 reco_urms=1036641 reco_freq=50 reco_powerp=1027951 reco_powerq=-7718 reco_energyp=28 reco_energyq=0
07:55:46 === reco_irms=32493 reco_urms=1036638 reco_freq=50 reco_powerp=1027663 reco_powerq=-6589 reco_energyp=29 reco_energyq=0
07:55:50 === reco_irms=32482 reco_urms=1036628 reco_freq=50 reco_powerp=1027444 reco_powerq=-7497 reco_energyp=30 reco_energyq=0
07:55:53 === reco_irms=32475 reco_urms=1036626 reco_freq=50 reco_powerp=1027165 reco_powerq=-7498 reco_energyp=31 reco_energyq=0
07:55:57 === reco_irms=32514 reco_urms=1036644 reco_freq=50 reco_powerp=1028060 reco_powerq=-7643 reco_energyp=31 reco_energyq=0
07:56:01 === reco_irms=32523 reco_urms=1036511 reco_freq=50 reco_powerp=1028483 reco_powerq=-7967 reco_energyp=32 reco_energyq=0
07:56:05 === reco_irms=32492 reco_urms=1036548 reco_freq=50 reco_powerp=1027546 reco_powerq=-8009 reco_energyp=33 reco_energyq=0

**********************************************************************************/


/*********************************************************************************
Kp= 3.22155*10^12/(2^32*HFConst*EC)
Kp =3.22155*10^12*Un*Ib*Ec/14.8528*Vu*Vi*10^11 *2^32*EC
Kp =3.22155*10^12*Un*Ib/14.8528*Vu*Vi*10^11 *2^32
Vu*Vi = 3.22155*10^12*Un*Ib/14.8528*10^11*2^32*Kp

HFCost = INT[(14.8528*Vu*Vi*10^11 ) / (Un*Ib*Ec)]
HFCost = 14.8528*Vu*Vi*10^11/Un*Ib*Ec
HFCost = 14.8528*10^11*(3.22155*10^12*Un*Ib/14.8528*10^11*2^32*Kp)/Un*Ib*Ec
HFCost = 14.8528*10^11*3.22155*10^12*Un*Ib/14.8528*10^11*2^32*Kp*Un*Ib*Ec
HFCost = 3.22155*10^12/2^32*Kp*Ec

*********************************************************************************/
#define RN8209C_CALI_FALG				0xBCDA
#define RN8209C_CALI_READ_COUNT			30
#define RN8209C_MAX_CALI_READ_FAILD		5

#ifdef RN8209C_CALI_POWER_BY_100W
#define RN8209C_MIN_CALI_RAW_POWER		400000U
#define RN8209C_CALIBRATE_POWER			100   //100W
#elif defined(RN8209C_CALI_POWER_BY_200W)
#define RN8209C_MIN_CALI_RAW_POWER		800000U
#define RN8209C_CALIBRATE_POWER			200   //200W
#else
#error "Please define how much power to calibrator"
#endif


#define RN8209C_MAX_CURRENT				15		//15A
#define RN8209C_DEFAULT_EC				3200
#define RN8209C_Kp_ViVu_DATA			(9000790/RN8209C_MAX_CURRENT) ////3.22155*10^12/(2^32*HFConst*EC)   //(KP/VuVi)
#define RN8209C_HF_COST_KPEC			750			//3.22155*10^12/2^32

#define RN8209C_DEFAULT_KI				36		//mA
#define RN8209C_DEFAULT_KV				4629
#define RN8209C_DEFAULT_KP				5139

#define RN8209C_DEFAULT_Vi_Vu			86		//0.06*0.12*10000
#define RN8209C_DEFAULT_HFCOST			1209

//------------------------------------------------------------------------
//				RN8209寄存器定义
//------------------------------------------------------------------------
//校表参数和计量控制
#define RN8209C_SYSCON     0x00
#define RN8209C_EMUCON     0x01
#define RN8209C_HFConst    0x02
#define RN8209C_PStart     0x03
#define RN8209C_GPQA       0x05
#define RN8209C_PhsA       0x07
#define RN8209C_APOSA      0x0A
#define RN8209C_IARMSOS    0x0E
#define RN8209C_IBGain     0x10

//计量参数和状态寄存器
#define RN8209C_PFCnt      0x20
#define RN8209C_IARMS      0x22
#define RN8209C_IBRMS      0x23
#define RN8209C_URMS       0x24
#define RN8209C_UFreq      0x25
#define RN8209C_PowerPA    0x26
#define RN8209C_PowerPB    0x27
#define RN8209C_PowerQ     0x28
#define RN8209C_EnergyP    0x29
#define RN8209C_EnergyP2   0x2A
#define RN8209C_EnergyQ    0x2B
#define RN8209C_EnergyQ2   0x2C
#define RN8209C_EMUStatus  0x2D

//中断寄存器
#define RN8209C_IE         0X40
#define RN8209C_IF         0X41
#define RN8209C_RIF        0X42

//系统状态寄存器
#define RN8209C_SysStatus  0x43
#define RN8209C_RData      0x44
#define RN8209C_WData      0x45
#define RN8209C_DeviceID   0x7F

//特殊命令
#define RN8209C_EA		   0xEA

#define RN8209C_PATH_A	0x5A
#define RN8209C_PATH_B	0xA5
#define RN9208C_WRITE_EN		0xE5
#define RN8209C_WRITE_PROTECT	0xDC
#define RN8209C_CMD_RESET		0xFA



typedef struct
{
	int reco_irms; //电流有效值
	int reco_urms; //电压有效值
	int reco_freq;//频率
	int reco_powerp;//有功功率
	int reco_powerq;//无功功率
	int reco_energyp;//有功能量
	int reco_energyq;//无功能量
} MeasureDataInfo;


void USER_FUNC rn8209cCreateThread(void);

#endif /* RN8209C_SUPPORT */

#endif

