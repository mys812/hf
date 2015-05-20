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

#define RSP_BUFFER_LEN		200
#define MAX_RECEIVE_BUF_SIZE	512
#define MAX_TRY_COUNT_WHILE_FAILD	10

void USER_FUNC resetForUpgrade(void);
void USER_FUNC enterUpgradeThread(void);


#endif
