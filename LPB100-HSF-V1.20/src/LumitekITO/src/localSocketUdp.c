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
#include "../inc/asyncMessage.h"
#include "../inc/socketSendList.h"




//define data



//global data
static int g_udp_socket_fd = -1;
static hfthread_mutex_t g_udp_socket_mutex;


S32 USER_FUNC getUdpSocketFd(void)
{
	return g_udp_socket_fd;
}



static BOOL initUdpSockrtMutex(void)
{
	BOOL ret = TRUE;
	if((hfthread_mutext_new(&g_udp_socket_mutex)!= HF_SUCCESS))
	{
		lumi_error("failed to create socketMutex");
		ret = FALSE;
	}
	return ret;
}


static void USER_FUNC udpCreateSocketAddr(struct sockaddr_in* addr, U32 socketIp)
{
	memset(addr, 0,  sizeof(struct sockaddr_in));
	addr->sin_family = AF_INET;
	addr->sin_port = htons(UDP_SOCKET_PORT);
	addr->sin_addr.s_addr = socketIp;
}



static void USER_FUNC udpSocketInit(void)
{
	struct sockaddr_in socketAddr;
	U32 socketIp;

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
	socketIp = htonl(INADDR_ANY);
	udpCreateSocketAddr(&socketAddr, socketIp);
	bind(g_udp_socket_fd, (struct sockaddr*)&socketAddr, sizeof(socketAddr));
	hfnet_set_udp_broadcast_port_valid(UDP_SOCKET_PORT-1, UDP_SOCKET_PORT);  //SDK Must used!
	lumi_debug("g_udp_socket_fd = %d \n", g_udp_socket_fd);
}


static S32 USER_FUNC udpSocketRecvData( S8 *buffer, S32 bufferLen, S32 socketFd, struct sockaddr_in *rm_add)
{
	S32 recvCount;
	U32 fromLen = sizeof(struct sockaddr);

	hfthread_mutext_lock(g_udp_socket_mutex);
	recvCount = recvfrom(socketFd, buffer, bufferLen, 0, (struct sockaddr *)rm_add, &fromLen);
	hfthread_mutext_unlock(g_udp_socket_mutex);
	//lumi_debug("udpSocketRecvData:count=%d port=%d, ip=%X fromLen=%d\n", recvCount, rm_add->sin_port, rm_add->sin_addr.s_addr, fromLen);
	return recvCount;
}



static S32 USER_FUNC udpSocketSendData(U8 *SocketData, S32 bufferLen, S32 socketFd, struct sockaddr_in *tx_add)
{
	S32 sendCount;

	hfthread_mutext_lock(g_udp_socket_mutex);
	sendCount = sendto(socketFd, SocketData, bufferLen, 0, (struct sockaddr*)tx_add, sizeof(struct sockaddr));
	hfthread_mutext_unlock(g_udp_socket_mutex);
	return(sendCount);
}



static S8* USER_FUNC recvUdpData(U32* recvCount, struct sockaddr_in* pSocketAddr)
{
	S8* recvBuf;

	recvBuf = getUdpRecvBuf(TRUE);
	*recvCount= (U32)udpSocketRecvData(recvBuf, NETWORK_MAXRECV_LEN, g_udp_socket_fd, pSocketAddr);
	if(!checkRecvSocketData(*recvCount, recvBuf))
	{
		return NULL;
	}
	return recvBuf;
}



BOOL USER_FUNC sendUdpData(U8* sendBuf, U32 dataLen, U32 socketIp)
{
	struct sockaddr_in socketAddr;


	udpCreateSocketAddr(&socketAddr, socketIp);
	udpSocketSendData(sendBuf, (S32)dataLen, g_udp_socket_fd, &socketAddr);
	return TRUE;
}


#ifdef SEND_LOG_BY_UDP
static void USER_FUNC sendStrByUdp(MSG_ORIGIN socketFrom, U8 cmdData, U8* sendBuf, U32 dataLen, U32 socketIp)
{
	S8* sendStr;
	U32 sendLen;
	U32 i;
	U32 index = 0;


	if(dataLen> 150)
	{
		S8 sendStrTmp[50];


		memset(sendStrTmp, 0, sizeof(sendStrTmp));
		sprintf(sendStrTmp, "%s len=%d cmd=%02X \n", getMsgComeFrom(socketFrom), dataLen, cmdData);
		sendLen = strlen(sendStrTmp);
		sendUdpData((U8*)sendStrTmp, sendLen, socketIp);
		return;
	}
	sendLen = (dataLen<<1) + 50;
	sendStr = (S8*)mallocSocketData(sendLen);

	if(sendStr == NULL)
	{
		return;
	}
	else
	{
		memset(sendStr, 0, sendLen);
	}
	if(cmdData != 0)
	{
		
		sprintf(sendStr, "%s len=%d cmd=%02X ", getMsgComeFrom(socketFrom), dataLen, cmdData);
	}
	index = strlen(sendStr);
	for(i=0; i<dataLen; i++)
	{
		sprintf(sendStr+index, "%02X", sendBuf[i]);
		index += 2;

		if(i%4 == 3)
		{
			sendStr[index] = ' ';
			index++;
		}
	}
	sendStr[index] = '\n';
	index++;
	
	sendUdpData((U8*)sendStr, index, socketIp);
	FreeSocketData((U8*)sendStr);
}

void USER_FUNC sendLogByUdp(MSG_ORIGIN socketFrom, U8 cmdData, U8* sendBuf, U32 dataLen)
{
	U32 sendIp;


	sendIp = inet_addr(SEND_LOG_IP);
	sendStrByUdp(socketFrom, cmdData, sendBuf, dataLen, sendIp);
}

#endif


void USER_FUNC deviceLocalUdpThread(void *arg)
{
	U32 recvCount;
	S8* recvBuf;
	struct sockaddr_in socketAddr;
	U8 selectRet;


	initUdpSockrtMutex();
	while(!getDeviceConnectInfo(DHPC_OK_BIT))
	{
		msleep(1000);
	}
	udpSocketInit();
	memset(&socketAddr, 0, sizeof(struct sockaddr_in));

	hfthread_enable_softwatchdog(NULL, 30); //Start watchDog
	while(1)
	{

		//lumi_debug(" deviceLocalUdpThread \n");
		hfthread_reset_softwatchdog(NULL); //tick watchDog
		
		selectRet = socketSelectRead(g_udp_socket_fd, MAX_SOCKET_SELECT_WAIT_SECOND);
		if((selectRet&SOCKET_READ_ENABLE) != 0)
		{
			recvBuf = recvUdpData(&recvCount, &socketAddr);
			if(recvBuf != NULL)
			{
				insertSocketMsgToList(MSG_FROM_UDP, (U8*)recvBuf, recvCount, socketAddr.sin_addr.s_addr);
			}
		}
		msleep(50);
	}
}

#endif
