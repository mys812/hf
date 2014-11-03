#ifndef __SERVER_SOCKET_TCP_H__
#define __SERVER_SOCKET_TCP_H__

#include <hsf.h>


S32 USER_FUNC getTcpSocketFd(void);
BOOL USER_FUNC sendTcpData(U8* sendBuf, U32 dataLen);
void USER_FUNC afterGetServerAddr(SOCKET_ADDR* socketAddr);

void USER_FUNC deviceServerTcpThread(void);

#endif
