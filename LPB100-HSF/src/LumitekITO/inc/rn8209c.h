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
#ifdef RN8209C_SELECT_PATH_A
#define RN8209C_MIN_CALI_RAW_POWER		800000U
#else
#define RN8209C_MIN_CALI_RAW_POWER		400000U
#endif
#define RN8209C_CALIBRATE_POWER			100   //100W
#elif defined(RN8209C_CALI_POWER_BY_200W)
#ifdef RN8209C_SELECT_PATH_A
#define RN8209C_MIN_CALI_RAW_POWER		1600000U
#else
#define RN8209C_MIN_CALI_RAW_POWER		800000U
#endif

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


typedef struct
{
	U16 urms;
	U16 irms;
	U32 powerP;
	U32 energyU;
} MeatureEnergyData;


void USER_FUNC rn8209cCreateThread(void);

#endif /* RN8209C_SUPPORT */

#endif

