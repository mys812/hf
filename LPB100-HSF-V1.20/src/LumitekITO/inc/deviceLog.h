#ifndef __DEVICE_LOG_H__
#define __DEVICE_LOG_H__

#include <hsf.h>




#ifdef SAVE_LOG_TO_FLASH
void USER_FUNC initFlashLog(void);
void USER_FUNC saveFlashLog(S8* saveData, U32 lenth);
void USER_FUNC readFlashLog(void);
void USER_FUNC clearFlashLog(void);
#endif

#ifdef SEND_LOG_BY_UDP
void USER_FUNC sendLogByUdp(BOOL bRecive, MSG_ORIGIN socketFrom, U8 cmdData, U8* sendBuf, U32 dataLen);
#endif

#endif

