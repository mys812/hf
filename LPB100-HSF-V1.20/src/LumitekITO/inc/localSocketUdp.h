#ifndef __LOCAL_SOCKET_UPD_H__
#define __LOCAL_SOCKET_UPD_H__

#include <hsf.h>


void USER_FUNC deviceLocalUdpThread(void);
BOOL USER_FUNC sendUdpData(U8* sendBuf, U32 dataLen, U32 socketIp);

#endif
