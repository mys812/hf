/*
******************************
*Company:Lumitek
*Data:2014-11-09
*Author:Meiyusong
******************************
*/

#ifndef __DEVICE_UPGRADE_H__
#define __DEVICE_UPGRADE_H__

#include <hsf.h>

#define MAX_RECEIVE_BUF_SIZE	256

void USER_FUNC setUpgradeType(S8* url);
void USER_FUNC enterUpgradeThread(void);


#endif
