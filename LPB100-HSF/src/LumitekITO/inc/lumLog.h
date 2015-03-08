#ifndef __DEVICE_LOG_H__
#define __DEVICE_LOG_H__

#include <hsf.h>




#ifdef SAVE_LOG_TO_FLASH

typedef struct
{
	U32 flag;
	U32 bakLogLen;
}FLASH_LOG_INFO;

void USER_FUNC initFlashLog(void);
void USER_FUNC readFlashLog(void);

#endif //SAVE_LOG_TO_FLASH


#if defined(SAVE_LOG_TO_FLASH) || defined(LUM_UART_SOCKET_LOG) || defined(LUM_UDP_SOCKET_LOG)
void USER_FUNC saveSocketData(BOOL bRecive, MSG_ORIGIN socketFrom, U8* socketData, U32 dataLen);
#endif

#if defined(SAVE_LOG_TO_FLASH) || defined(LUM_UART_SOCKET_LOG) || defined(LUM_UDP_SOCKET_LOG) || defined(LUM_RN8209C_UDP_LOG)
void USER_FUNC saveNormalLogData(const char *format, ...);
#endif



#endif

