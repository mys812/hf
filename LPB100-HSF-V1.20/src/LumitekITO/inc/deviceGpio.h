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


void USER_FUNC smartlinkTimerCallback( hftimer_handle_t htimer );

//Switch status
void USER_FUNC setSwitchStatus(BOOL bOpen);
BOOL USER_FUNC getSwitchStatus(void);

void USER_FUNC KeyGpioInit(void);


#endif
