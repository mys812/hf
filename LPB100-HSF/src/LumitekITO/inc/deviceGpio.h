/*
******************************
*Company:Lumitek
*Data:2014-10-07
*Author:Meiyusong
******************************
*/



#if 0
GpioGetReg can get GPIO register. But this is not recommended. See the following for usage
 
#define GPIO_A_IN (0x00) /**<GPIOA input data register macro*/
#define GPIO_A_OUT (0x01) /**<GPIOA output data register macro*/
#define GPIO_B_IN (0x0A) /**<GPIOB input data register macro*/
#define GPIO_B_OUT (0x0B) /**<GPIOB output data register macro*/
#define GPIO_C_IN (0x14) /**<GPIOC input data register macro*/
#define GPIO_C_OUT (0x15) /**<GPIOC output data register macro*/
 
 GpioGetReg(GPIO_A_IN)&(1<<gpio_index)                ;gpio_index is the corresponding GPIOX pin number.  If GPIOA_25, then gpio_index should be 25.
 
The GPIO mapping information is as following.
CHIP_NOPIN, //lpb GND
CHIP_NOPIN, //lpb SWCLK
CHIP_NOPIN, //lpb pin3 NC
CHIP_NOPIN, //lpb pin4 NC
CHIP_NOPIN, //lpb SWD 
CHIP_NOPIN, //lpb pin6 NC
O18_GPIO_BANK_B(5),     //lpb SLEEP_RQ GPIO7
O18_GPIO_BANK_B(7),     //lpb SLEEP_ON GPIO8
CHIP_NOPIN, //lpb DVDD
CHIP_NOPIN, //lpb pin10 NC
O18_GPIO_BANK_B(24),  //lpb GPIO11 LPB100_PWM1
O18_GPIO_BANK_B(25),  //lpb GPIO12 LPB100_PWM2
O18_GPIO_BANK_C(0),  //lpb GPIO13
CHIP_NOPIN, //lpb pin14 NC
O18_GPIO_BANK_C(2),     //lpb GPIO15 wps
CHIP_NOPIN,  //lpb pin16 NC
CHIP_NOPIN, //lpb pin17 GND
O18_GPIO_BANK_B(26), //lpb100 GPIO18 LPB100_PWM3
CHIP_NOPIN, //lpb100 pin19 NC
O18_GPIO_BANK_B(27), //lpb100 GPIO20  LPB100_PWM4
CHIP_NOPIN, //lpb100 pin21 NC
CHIP_NOPIN, //lpb100 pin22 NC
O18_GPIO_BANK_B(6),     //lpb100 GPIO23/PWM5
CHIP_NOPIN, //lpb pin24 NC
CHIP_NOPIN, //lpb pin25 PWR_SW
CHIP_NOPIN,  //lpb pin26 NC
O18_GPIO_BANK_B(22),  //lpb SPI_MISO
O18_GPIO_BANK_B(21),  //lpb SPI_CLK
O18_GPIO_BANK_B(23),  //lpb SPI_CS
O18_GPIO_BANK_B(20),  //lpb SPI_MOSI
CHIP_NOPIN,   //lpb DVDD
CHIP_NOPIN,   //lpb GND
CHIP_NOPIN,   //lpb pin33 NC
CHIP_NOPIN,   //lpb DVDD
CHIP_NOPIN,  //lpb pin35 NC
CHIP_NOPIN,   //lpb pin36 NC
CHIP_NOPIN,   //lpb pin37 NC
CHIP_NOPIN,   //lpb pin38 NC
O18_GPIO_BANK_B(28),  //lpb UART0_TX
O18_GPIO_BANK_B(31),  //lpb UART0_RTS
O18_GPIO_BANK_B(29),  //lpb UART0_RX
O18_GPIO_BANK_B(30),  //lpb UART0_CTS
O18_GPIO_BANK_A(25),  //lpb nLink
O18_GPIO_BANK_A(11),   //lpb nReady
O18_GPIO_BANK_A(12),  //lpb nReload
CHIP_NOPIN,   //lpb pin46 NC
CHIP_NOPIN,    //lpb EXT_RESETn
CHIP_NOPIN,    //lpb pin48 GND

#endif

#ifndef __DEVICE_GPIO_H__
#define __DEVICE_GPIO_H__

#include <hsf.h>
#include "itoCommon.h"


typedef struct
{
	U8 maxRingTimes;
	U16 ringPeriod;
	U16 stopPeriod;
	const U16* pRindPeriod;
}BUZZER_RING_DATA;



typedef struct
{
	U8 ringPeriodIndex;
	U8 curTimes;
	BOOL ringStop;
	U32 startTime;
	const BUZZER_RING_DATA* pRingData;
}BUZZER_RING_INFO;


typedef enum
{
	SWITCH_PIN_1	= 0,
	SWITCH_PIN_2	= 1,
	SWITCH_PIN_3	= 2,
	SWITCH_PIN_4	= 3,
}SWITCH_PIN_FLAG;


typedef struct
{
	SWITCH_STATUS	action;
	SWITCH_PIN_FLAG pinFlag;
} GPIO_CHANGE_REPORT;



#ifdef LIGHT_CHENGE_SUPPORT
#define MAX_LIGHT_LEVEL			7				//7档
#define LIGHT_DIM_BASE_TIME		1000			//2ms
#define LIGHT_DIM_SHUTDOWN_TIME	1000			//1ms
#define LIGHT_DIM_LEVEL_GAP		1200			//1.2ms
#define LIGHT_KEY_DEBOUNCE		30				//30ms去抖


#define GPIO_A_IN (0x00) /**<GPIOA input data register macro*/
#define GPIO_A_OUT (0x01) /**<GPIOA output data register macro*/
#define GPIO_B_IN (0x0A) /**<GPIOB input data register macro*/
#define GPIO_B_OUT (0x0B) /**<GPIOB output data register macro*/
#define GPIO_C_IN (0x14) /**<GPIOC input data register macro*/
#define GPIO_C_OUT (0x15) /**<GPIOC output data register macro*/


typedef enum
{
	GET_AC_FREQ,		//获取市电频率
	ZERO_DETECT,		//过零检测
	SHUT_DOWN_DIM,		//关断 DIM
}LIGHT_DIM_STATUS;
#endif


#ifdef CHANGE_BRIGHTNESS_SUPPORT

#define SMARTLINK_BRIGHT_LEVEL		5


typedef enum
{
	LIGHT_LED_CLOSE,		 //灯关闭
	LIGHT_LED_OPEN,		 	//灯开启
	LIGHT_NOT_UNCONNECT, //wifi未连接
	LIGHT_SMARTLINK,	//进入配置模式
}LIGHT_STATUS_INDICATION;


typedef struct
{
	U8 ledOpen;
	LIGHT_STATUS_INDICATION ledStatus;
}LIGHT_LET_INFO;
#endif

void USER_FUNC initDevicePin(void);

//switch status
SWITCH_STATUS USER_FUNC getSwitchStatus(SWITCH_PIN_FLAG switchFlag);
void USER_FUNC setSwitchStatus(SWITCH_STATUS action, SWITCH_PIN_FLAG switchFlag);
void USER_FUNC changeSwitchStatus(SWITCH_PIN_FLAG switchFlag);


//buzzer status
#ifdef BUZZER_RING_SUPPORT
void USER_FUNC switchBuzzerStatus(void);
S32 USER_FUNC getBuzzerRingPeriod(const BUZZER_RING_DATA* initRingData);
void USER_FUNC setBuzzerStatus(BUZZER_STATUS buzzerStatus);
void USER_FUNC buzzerRingNotice(S32 ringPeriod, S32 stopPeriod, S32 ringTims);
#endif


#ifdef DEVICE_KEY_SUPPORT
void USER_FUNC initKeyGpio(void);
#ifdef LUM_FACTORY_TEST_SUPPORT
void USER_FUNC lum_DisableKeyInterrupt(void);
void USER_FUNC lum_EnableKeyInterrupt(void);
#endif //LUM_FACTORY_TEST_SUPPORT

#endif //DEVICE_KEY_SUPPORT

#ifdef DEVICE_WIFI_LED_SUPPORT
void USER_FUNC changeWifiLedStatus(BOOL needClose);
#endif


#ifdef RN8209C_SUPPORT
void USER_FUNC lum_rn8209cInitCfPin(void);
#endif

#ifdef LIGHT_CHENGE_SUPPORT
void USER_FUNC lum_lightChangeIRQInit(void);
void USER_FUNC lum_lightChangeLevel(U8 level);
#endif

#ifdef CHANGE_BRIGHTNESS_SUPPORT
void USER_FUNC lum_setLedLightStatus(LIGHT_STATUS_INDICATION letStatus);
#endif

#endif

