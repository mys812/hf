/*
******************************
*Company:Lumitek
*Data:2014-10-07
*Author:Meiyusong
******************************
*/

#ifndef __DEVICE_GPIO_H__
#define __DEVICE_GPIO_H__

#include <hsf.h>
#include "itoCommon.h"


#ifdef LPB100_DEVLOPMENT_BOARD
//Switch status
void USER_FUNC keyGpioInit(void);

//light status
void USER_FUNC switchLightStatus(void);
#endif

//switch status
SWITCH_STATUS USER_FUNC getSwitchStatus(void);
void USER_FUNC setSwitchStatus(SWITCH_STATUS action);

//buzzer status
#ifdef DEEVICE_LUMITEK_P1
void USER_FUNC switchBuzzerStatus(void);

#ifdef EXTRA_SWITCH_SUPPORT
void USER_FUNC extraSwitchInit(void);
#endif //EXTRA_SWITCH_SUPPORT

#endif //DEEVICE_LUMITEK_P1

#endif

