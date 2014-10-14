#ifndef __LOCAL_SOCKET_UPD_H__
#define __LOCAL_SOCKET_UPD_H__

#include <hsf.h>


void USER_FUNC deviceLocalUdpThread(void);
U32 USER_FUNC udpSocketSendData(U8* sendBuf, U32 dataLen);


#endif
