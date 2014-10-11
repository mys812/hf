/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *-pipe -fno-strict-aliasing -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror-implicit-function-declaration -Wpointer-arith -std=gnu99 -ffunction-sections -fdata-sections -Wchar-subscripts -Wcomment -Wformat=2 -Wimplicit-int -Wmain -Wparentheses -Wsequence-point -Wreturn-type -Wswitch -Wtrigraphs -Wunused -Wuninitialized -Wunknown-pragmas -Wfloat-equal -Wundef -Wshadow -Wbad-function-cast -Wwrite-strings -Wsign-compare -Waggregate-return  -Wmissing-declarations -Wformat -Wmissing-format-attribute -Wno-deprecated-declarations -Wpacked -Wredundant-decls -Wnested-externs -Wlong-long -Wunreachable-code -Wcast-align --param max-inline-insns-single=500
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
#include <hsf.h>
#include <stdlib.h>
#include <string.h>
//#include <malloc.h>
#include <stdio.h>
#include <httpc/httpc.h>
#include "../example.h"

#if (EXAMPLE_USE_DEMO==USER_SOCKET_DEMO)

//#define TEST_UART_SELECT

const int hf_gpio_fid_to_pid_map_table[HFM_MAX_FUNC_CODE]=
{
	HF_M_PIN(2),	//HFGPIO_F_JTAG_TCK
	HF_M_PIN(3),	//HFGPIO_F_JTAG_TDO
	HF_M_PIN(4),	//HFGPIO_F_JTAG_TDI
	HF_M_PIN(5),	//HFGPIO_F_JTAG_TMS
	HFM_NOPIN,		//HFGPIO_F_USBDP
	HFM_NOPIN,		//HFGPIO_F_USBDM
	HF_M_PIN(39),	//HFGPIO_F_UART0_TX
	HF_M_PIN(40),	//HFGPIO_F_UART0_RTS
	HF_M_PIN(41),	//HFGPIO_F_UART0_RX
	HF_M_PIN(42),	//HFGPIO_F_UART0_CTS
	
	HF_M_PIN(27),	//HFGPIO_F_SPI_MISO
	HF_M_PIN(28),	//HFGPIO_F_SPI_CLK
	HF_M_PIN(29),	//HFGPIO_F_SPI_CS
	HF_M_PIN(30),	//HFGPIO_F_SPI_MOSI
	
	HFM_NOPIN,	//HFGPIO_F_UART1_TX,
	HFM_NOPIN,	//HFGPIO_F_UART1_RTS,
	HFM_NOPIN,	//HFGPIO_F_UART1_RX,
	HFM_NOPIN,	//HFGPIO_F_UART1_CTS,
	
	HF_M_PIN(43),	//HFGPIO_F_NLINK
	HF_M_PIN(44),	//HFGPIO_F_NREADY
	HF_M_PIN(45),	//HFGPIO_F_NRELOAD
	HF_M_PIN(7),	//HFGPIO_F_SLEEP_RQ
	HF_M_PIN(8),	//HFGPIO_F_SLEEP_ON

	HFM_NOPIN,		//HFGPIO_F_RESERVE0
	HFM_NOPIN,		//HFGPIO_F_RESERVE1
	HFM_NOPIN,		//HFGPIO_F_RESERVE2
	HFM_NOPIN,		//HFGPIO_F_RESERVE3
	HFM_NOPIN,		//HFGPIO_F_RESERVE4
	HFM_NOPIN,		//HFGPIO_F_RESERVE5
	
	HFM_NOPIN,	//HFGPIO_F_USER_DEFINE
};

const hfat_cmd_t user_define_at_cmds_table[]=
{
	{NULL,NULL,NULL,NULL} //the last item must be null
};

USER_FUNC int tcp_connect_server()
{
	int fd;	
	int tmp=1;
	struct sockaddr_in addr;
	
	memset((char*)&addr,0,sizeof(addr));
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(10001);
	addr.sin_addr.s_addr=inet_addr("10.10.100.150");
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd<0)
		return -1;
	
	tmp=1;
	if(setsockopt(fd, SOL_SOCKET,SO_KEEPALIVE,&tmp,sizeof(tmp))<0)
	{
		u_printf("set SO_KEEPALIVE fail\n");
	}
	tmp = 60;//60s
	if(setsockopt(fd, IPPROTO_TCP,TCP_KEEPIDLE,&tmp,sizeof(tmp))<0)
	{
		u_printf("set TCP_KEEPIDLE fail\n");
	}
	tmp = 6;
	if(setsockopt(fd, IPPROTO_TCP,TCP_KEEPINTVL,&tmp,sizeof(tmp))<0)
	{
		u_printf("set TCP_KEEPINTVL fail\n");
	}
	tmp = 5;
	if(setsockopt(fd, IPPROTO_TCP,TCP_KEEPCNT,&tmp,sizeof(tmp))<0)
	{
		u_printf("set TCP_KEEPCNT fail\n");
	}
	
	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr))< 0)
	{
		close(fd);
		return -1;
	}
	u_printf("connect ok!\n");
	
	return fd;
}

USER_FUNC void test_socket_start(void)
{
	int fd=-1;
	int recv_num=0;
	char recv[32]={0};
	uint8_t mac_addr[6]={0};
	int ufd,ret,maxfd;
	int uart_fd;
	fd_set rset;
	struct timeval timeout;
	struct sockaddr_in addr;
	int alen=sizeof(struct sockaddr_in);
	
	memset((char*)&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(10000);
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	ufd = socket(AF_INET, SOCK_DGRAM, 0);
	if(ufd<0)
	{
		u_printf("create udp socket fail\n");
	}
	bind(ufd, (struct sockaddr*)&addr, sizeof(addr));
	maxfd=ufd;
	FD_ZERO(&rset);
	fd = tcp_connect_server();
	uart_fd = (int)hfuart_open(0);
	while(1)
	{
		maxfd = ufd;
		if(maxfd<fd)
			maxfd=fd;
		FD_ZERO(&rset);
		if(fd>=0)
			FD_SET(fd,&rset);
		FD_SET(ufd,&rset);
#ifdef TEST_UART_SELECT	
		FD_SET(uart_fd,&rset);
		if(maxfd<uart_fd)
			maxfd=uart_fd;
#endif		
		timeout.tv_sec= 3;
		timeout.tv_usec= 0;
		ret = hfuart_select(maxfd+1, &rset, NULL, NULL, &timeout);
		if(ret<=0)
			continue;
		if (FD_ISSET(fd, &rset))
		{
			if((recv_num=recv(fd,recv,sizeof(recv),0))>0)
			{
				u_printf("recv data bytes:%d\n",recv_num);
			}
			else
			{
				close(fd);
				fd=-1;
				u_printf("tcp disconnectd!\n");
			}
		}
		else if(FD_ISSET(ufd,&rset))
		{	
			alen=sizeof(struct sockaddr_in);
			if((recv_num=hfnet_recvfrom(ufd,recv,sizeof(recv),0,(struct sockaddr*)&addr,(socklen_t*)&alen,(char*)mac_addr))>0)
			{
				u_printf("recv data bytes:%d mac:%02X:%02X:%02X:%02X:%02X:%02X\n",
				recv_num,
				mac_addr[0],mac_addr[1],mac_addr[2],mac_addr[3],mac_addr[4],mac_addr[5]);
			}
		}
		else if(FD_ISSET(uart_fd,&rset))
		{
			recv_num=hfuart_recv((hfuart_handle_t)uart_fd,recv,sizeof(recv)-1,0);
			if(recv_num>0)
			{
				recv[recv_num]=0;
				u_printf("recv data bytes:%d %s\n",recv_num,recv);
			}
		}
		
	}
	
	return;

}



int USER_FUNC app_main (void)
{
	time_t now=time(NULL);
	
	HF_Debug(DEBUG_LEVEL,"[FILE DEMO]sdk version(%s),the app_main start time is %d %s\n",hfsys_get_sdk_version(),now,ctime(&now));
	if(hfgpio_fmap_check()!=0)
	{
		while(1)
		{
			HF_Debug(DEBUG_ERROR,"gpio map file error\n");
			msleep(1000);
		}
		//return 0;
	}

	while(!hfnet_wifi_is_active())
	{
		msleep(50);
	}
	
	if(hfnet_start_assis(ASSIS_PORT)!=HF_SUCCESS)
	{
		HF_Debug(DEBUG_WARN,"start httpd fail\n");
	}

	if(hfnet_start_httpd(HFTHREAD_PRIORITIES_MID)!=HF_SUCCESS)
	{
		HF_Debug(DEBUG_WARN,"start httpd fail\n");
	}
#ifndef TEST_UART_SELECT		
	if(hfnet_start_uart(HFTHREAD_PRIORITIES_LOW,NULL)!=HF_SUCCESS)
	{
		HF_Debug(DEBUG_WARN,"start uart fail!\n");
	}
#endif	
	test_socket_start();

	return 1;
	
}

#endif
