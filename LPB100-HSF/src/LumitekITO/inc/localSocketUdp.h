#ifndef __LOCAL_SOCKET_UPD_H__
#define __LOCAL_SOCKET_UPD_H__

#include <hsf.h>


S32 USER_FUNC getUdpSocketFd(void);
void USER_FUNC deviceLocalUdpThread(void *arg);
BOOL USER_FUNC sendUdpData(U8* sendBuf, U32 dataLen, U32 socketIp);

#if defined(LUM_UDP_SOCKET_LOG) || defined(LUM_RN8209C_UDP_LOG)
void USER_FUNC lum_createUdpLogSocket(void);
BOOL USER_FUNC lum_sendUdpLog(U8* sendBuf, U32 dataLen);
#endif //#if defined(LUM_UDP_SOCKET_LOG) || defined(LUM_RN8209C_UDP_LOG)

#endif

