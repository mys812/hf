#ifndef __DEVICE_LOG_H__
#define __DEVICE_LOG_H__

#include <hsf.h>




#ifdef SAVE_LOG_TO_FLASH

typedef struct
{
	U32 flag;
	U32 bakLogLen;
}FLASH_LOG_INFO;


void USER_FUNC saveSocketData(BOOL bRecive, MSG_ORIGIN socketFrom, U8* socketData, U32 dataLen);
void USER_FUNC readFlashLog(void);
void USER_FUNC initFlashLog(void);
void USER_FUNC saveNormalLogData(const char *format, ...);

#endif //SAVE_LOG_TO_FLASH

#endif

