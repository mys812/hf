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



//global data
static int g_tcp_socket_fd = -1;
static hfthread_mutex_t g_tcp_socket_mutex;



static BOOL initTcpSockrtMutex(void)
{
	BOOL ret = TRUE;
	if((hfthread_mutext_new(&g_tcp_socket_mutex)!= HF_SUCCESS))
	{
		HF_Debug(DEBUG_ERROR, "failed to create TCP socketMutex");
		ret = FALSE;
	}
	return ret;
}


static void USER_FUNC tcpCreateSocketAddr(struct sockaddr_in* addr, SOCKET_ADDR* pSocketAddr)
{
	memset(addr, 0,  sizeof(struct sockaddr_in));
	addr->sin_family = AF_INET;
	addr->sin_port = htons(pSocketAddr->port);
	addr->sin_addr.s_addr = htonl(pSocketAddr->ipAddr);
}



static void USER_FUNC setSocketOption(S32 fd)
{
	S32 optData;

	optData = 1;
	if(setsockopt(fd, SOL_SOCKET,SO_KEEPALIVE,&optData,sizeof(optData))<0)
	{
		u_printf("set SO_KEEPALIVE fail\n");
	}
	optData = 60;//60s
	if(setsockopt(fd, IPPROTO_TCP,TCP_KEEPIDLE,&optData,sizeof(optData))<0)
	{
		u_printf("set TCP_KEEPIDLE fail\n");
	}
	optData = 6;
	if(setsockopt(fd, IPPROTO_TCP,TCP_KEEPINTVL,&optData,sizeof(optData))<0)
	{
		u_printf("set TCP_KEEPINTVL fail\n");
	}
	optData = 5;
	if(setsockopt(fd, IPPROTO_TCP,TCP_KEEPCNT,&optData,sizeof(optData))<0)
	{
		u_printf("set TCP_KEEPCNT fail\n");
	}
}



static BOOL USER_FUNC connectServerSocket(SOCKET_ADDR* pSocketAddr)
{
	struct sockaddr_in socketAddrIn;


	tcpCreateSocketAddr(&socketAddrIn, pSocketAddr);
	if(connect(g_tcp_socket_fd, (struct sockaddr *)&socketAddrIn, sizeof(socketAddrIn)) < 0)
	{
		u_printf("meiyusong===> tcp connect faild\n");
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}


void USER_FUNC closeSocketFd(CLOSE_SOCKET_TYPE closeType)
{
	if(closeType == CLOSE_SOCKET)
	{
		close(g_tcp_socket_fd);
	}
	else if(closeType == SHUTDOWN_SOCKET)
	{
		shutdown(g_tcp_socket_fd, SHUT_RDWR);
	}
	g_tcp_socket_fd = -1;
}


static void USER_FUNC tcpSocketInit(SOCKET_ADDR* pSocketAddr)
{
	struct sockaddr_in socketAddrIn;


	while (1)
	{
		g_tcp_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (g_tcp_socket_fd < 0)
		{
			msleep(1000);
			continue;
		}
		else
		{
			break;
		}
	}
	tcpCreateSocketAddr(&socketAddrIn, pSocketAddr);
	setSocketOption(g_tcp_socket_fd);
	//connect(g_tcp_socket_fd, (struct sockaddr *)&socketAddrIn, sizeof(socketAddrIn));
	u_printf("meiyusong===> g_tcp_socket_fd = %d \n", g_tcp_socket_fd);
}



void USER_FUNC tcpSocketServerInit(SOCKET_ADDR* pSocketAddr)
{
	if(g_tcp_socket_fd != -1)
	{
		close(g_tcp_socket_fd);
		g_tcp_socket_fd = -1;
	}
	tcpSocketInit(pSocketAddr);
}


static U8 USER_FUNC tcpSockSelect(struct timeval* pTimeout)
{
	fd_set fdR;
	S32 ret;
	U8 sel= 0;

	FD_ZERO(&fdR);
	if (g_tcp_socket_fd != -1)
	{
		FD_SET(g_tcp_socket_fd,&fdR);
	}
	ret= select((g_tcp_socket_fd + 1), &fdR,NULL,NULL, pTimeout);
	if (ret <= 0)
	{
		return 0;
	}
	else if (FD_ISSET(g_tcp_socket_fd, &fdR))
	{
		sel = 0x01;
	}
	return sel;
}



static S32 USER_FUNC tcpSocketRecvData( S8 *buffer, S32 bufferLen, S32 socketFd)
{
	S32 recvCount;

	hfthread_mutext_lock(g_tcp_socket_mutex);
	recvCount = recv(socketFd, buffer, bufferLen, 0);
	hfthread_mutext_unlock(g_tcp_socket_mutex);
	return recvCount;
}



static S32 USER_FUNC tcpSocketSendData(U8 *SocketData, S32 bufferLen, S32 socketFd)
{
	int sendCount;

	hfthread_mutext_lock(g_tcp_socket_mutex);
	sendCount = send(socketFd, SocketData, bufferLen, 0);
	hfthread_mutext_unlock(g_tcp_socket_mutex);
	return sendCount;
}



static S8* USER_FUNC recvTcpData(U32* recvCount)
{
	S8* recvBuf;

	recvBuf = getTcpRecvBuf(TRUE);
	*recvCount= (U32)tcpSocketRecvData(recvBuf, NETWORK_MAXRECV_LEN, g_tcp_socket_fd);
	if(!checkRecvSocketData(*recvCount, recvBuf))
	{
		return NULL;
	}
	return recvBuf;
}



U32 USER_FUNC sendTcpData(U8* sendBuf, U32 dataLen)
{
	return tcpSocketSendData(sendBuf, (S32)dataLen, g_tcp_socket_fd);
}




void USER_FUNC deviceServerTcpThread(void)
{
	U32 recvCount;
	S8* recvBuf;
	struct timeval timeout;
	SOCKET_ADDR socketAddr;

	initTcpSockrtMutex();

	timeout.tv_sec = 3;
	timeout.tv_usec = 0;
	socketAddr.ipAddr = TCP_SERVER_IP;
	socketAddr.port = TCP_SOCKET_PORT;

	tcpSocketInit(&socketAddr);

	hfthread_enable_softwatchdog(NULL, 30); //Start watchDog
	while(1)
	{

		//u_printf(" deviceServerTcpThread \n");
		hfthread_reset_softwatchdog(NULL); //tick watchDog

		if(getDeviceConnectInfo(DHPC_OK_BIT) == 0) //not got IP from AP
		{
			msleep(3000);
			continue;
		}
		if(getDeviceConnectInfo(BALANCE_CONN_BIT) == 0) // balance server disconnect
		{
			if(connectServerSocket(&socketAddr))
			{
				setDeviceConnectInfo(BALANCE_CONN_BIT, 1);
				insertLocalMsgToList(MSG_LOCAL_EVENT, NULL, 0, MSG_CMD_GET_SERVER_ADDR);
			}
			else
			{
				msleep(3000);
			}
			continue;
		}
		if(getDeviceConnectInfo(SERVER_ADDR_BIT) == 0) // not got actual server addr
		{
			msleep(1000);
			continue;
		}
		if(getDeviceConnectInfo(SERVER_CONN_BIT) == 0) // actual server disconnect
		{
			getServerAddr(&socketAddr);
			if(connectServerSocket(&socketAddr))
			{
				setDeviceConnectInfo(SERVER_CONN_BIT, 1);
				insertLocalMsgToList(MSG_LOCAL_EVENT, NULL, 0, MSG_CMD_REQUST_CONNECT);
			}
			else
			{
				msleep(3000);
			}
			continue;
		}
		if(tcpSockSelect(&timeout) > 0) //check socket buf
		{
			recvBuf = recvTcpData(&recvCount);
			if(recvBuf != NULL)
			{
				insertSocketMsgToList(MSG_FROM_TCP, (U8*)recvBuf, recvCount, 0); // insert msg to msg list
			}
		}
		msleep(100);
	}
}

#endif
