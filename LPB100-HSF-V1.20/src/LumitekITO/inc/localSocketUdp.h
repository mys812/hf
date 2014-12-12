#ifndef __LOCAL_SOCKET_UPD_H__
#define __LOCAL_SOCKET_UPD_H__

#include <hsf.h>


S32 USER_FUNC getUdpSocketFd(void);
void USER_FUNC deviceLocalUdpThread(void *arg);
BOOL USER_FUNC sendUdpData(U8* sendBuf, U32 dataLen, U32 socketIp);
#ifdef SEND_LOG_BY_UDP
void USER_FUNC sendLogByUdp(MSG_ORIGIN socketFrom, U8 cmdData, U8* sendBuf, U32 dataLen);
#endif // SEND_LOG_BY_UDP
#endif
