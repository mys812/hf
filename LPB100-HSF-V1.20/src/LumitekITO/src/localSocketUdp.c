/*
******************************
*Company:Lumitek
*Data:2014-10-07
*Author:Meiyusong
******************************
*/

#include "../inc/lumitekConfig.h"

#ifdef CONFIG_LUMITEK_DEVICE
#include <hsf.h>
#include <string.h>
#include <stdio.h>

#include "../inc/itoCommon.h"


//define data



//global data
static int g_udp_socket_fd = -1;

int USER_FUNC getUdpSocketFd(void)
{
    return g_udp_socket_fd;
}



static void USER_FUNC udpSocketInit(void)
{
    struct sockaddr_in addr;
    S32 tmp = 1;

    while (1)
    {
        g_udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (g_udp_socket_fd < 0)
        {
            msleep(1000);
            continue;
        }
        else
        {
            break;
        }
    }
    memset(&addr, 0,  sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(UDP_SOCKET_PORT);
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    tmp=1;
    setsockopt(g_udp_socket_fd, SOL_SOCKET,SO_BROADCAST,&tmp,sizeof(tmp));
    hfnet_set_udp_broadcast_port_valid(UDP_SOCKET_PORT, UDP_SOCKET_PORT);  //SDK Must used!
    bind(g_udp_socket_fd, (struct sockaddr*)&addr, sizeof(addr));
    u_printf("meiyusong===> g_udp_socket_fd = %d \n", g_udp_socket_fd);
}



static U8 USER_FUNC udpSockSelect(void)
{
    fd_set fdR;
    struct timeval timeout;
    int ret;
    U8 sel= 0;

    FD_ZERO(&fdR);
    if (g_udp_socket_fd != -1)
    {
        FD_SET(g_udp_socket_fd,&fdR);
    }

    timeout.tv_sec= 0;							// timeout shall be in while
    timeout.tv_usec= 200000;					// 200ms
    ret= select(g_udp_socket_fd+1,&fdR,NULL,NULL,&timeout);
    //u_printf("After Select: ret= %d\n", ret);
    if (ret<= 0)
    {
        return 0;
    }
    else if (FD_ISSET(g_udp_socket_fd, &fdR))
    {
        sel = 0x01;
    }
    return sel;
}



static S32 USER_FUNC udpSocketRecvData( S8 *buffer, S32 bufferLen, S32 socketFd, struct sockaddr_in *rm_add)
{
    S32 recvCount;
    U32 fromLen;
    struct sockaddr_in rmaddr;
    hfthread_mutex_t socketMutex = getSocketMutex();

    hfthread_mutext_lock(socketMutex);
    recvCount = recvfrom(socketFd, buffer, bufferLen, 0, (struct sockaddr *)&rmaddr, &fromLen);
    hfthread_mutext_unlock(socketMutex);
    // u_printf("meiyusong===> udpSocketRecvData:count=%d port=%d, ip=%X\n", recvCount, rmaddr.sin_port, rmaddr.sin_addr.s_addr);
    memcpy(rm_add, &rmaddr, sizeof(struct sockaddr_in));
    return recvCount;
}


static void USER_FUNC showSocketOutsideData(SOCKET_HEADER_DATA* pHearderData)
{
    u_printf("pv=%d, flag=0x%x, mac=%x-%x-%x-%x-%x-%x, len=%d  snIndex=0x%x, deviceType=0x%x, factoryCode=0x%x, licenseData=0x%x\n",
             pHearderData->outsideData.openData.pv,
             pHearderData->outsideData.openData.flag,
             pHearderData->outsideData.openData.mac[0],
             pHearderData->outsideData.openData.mac[1],
             pHearderData->outsideData.openData.mac[2],
             pHearderData->outsideData.openData.mac[3],
             pHearderData->outsideData.openData.mac[4],
             pHearderData->outsideData.openData.mac[5],
             pHearderData->outsideData.openData.dataLen,
             pHearderData->outsideData.secretData.snIndex,
             pHearderData->outsideData.secretData.deviceType,
             pHearderData->outsideData.secretData.factoryCode,
             pHearderData->outsideData.secretData.licenseData);

}


static void USER_FUNC checkSocketLen(S8* pData, S32 dataLen)
{
    SOCKET_HEADER_OPEN* pOpenData = (SOCKET_HEADER_OPEN*)pData;
    u_printf("recvCount=%d, Encrypt data len=%d, struct len=%d\n",
             dataLen,
             pOpenData->dataLen,
             SOCKET_HEADER_OPEN_DATA_LEN);
}


static void USER_FUNC udpSocketGetDecryptData(void)
{
    S32 recvCount;
    S8* recvBuf = getSocketRecvBuf(TRUE);
    struct sockaddr_in addr;
    SOCKET_HEADER_DATA* pHearderData = NULL;
    U8* pDecryptData = NULL;
	SOCKET_HEADER_OPEN* pOpenData = NULL;
	U32 decryptDataLen;


    recvCount= udpSocketRecvData(recvBuf, NETWORK_MAXRECV_LEN, g_udp_socket_fd, &addr);
    if (recvCount >= 10)
    {
    	pOpenData = (SOCKET_HEADER_OPEN*)recvBuf;
        checkSocketLen(recvBuf, recvCount);
		decryptDataLen = pOpenData->dataLen + SOCKET_HEADER_OPEN_DATA_LEN;
		if(recvCount != decryptDataLen)
		{
			HF_Debug(DEBUG_ERROR, "Socket receive data len error recvDataLen=%d, data len should = %d\n", recvCount, decryptDataLen);
		}
        pDecryptData = socketDataAesDecrypt(recvBuf, decryptDataLen, AES_KEY_DEFAULT);
        if(pDecryptData == NULL)
        {
            return;
        }

        pHearderData = (SOCKET_HEADER_DATA*)pDecryptData;
        pHearderData->insideData = (S8*)(pDecryptData + sizeof(SCOKET_HERADER_OUTSIDE));
        showSocketOutsideData(pHearderData);
        FreeSocketData((U8*)pDecryptData);
    }
}



void USER_FUNC deviceLocalUdpThread(void)
{


    udpSocketInit();

    hfthread_enable_softwatchdog(NULL,30); //Start watchDog
    while(1)
    {

        //u_printf(" deviceLocalUdpThread \n");
        hfthread_reset_softwatchdog(NULL); //tick watchDog

        if(udpSockSelect() > 0)
        {
            udpSocketGetDecryptData();
        }
        msleep(100);
    }
}

#endif
