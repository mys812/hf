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
#include "hfmsgq.h"

#if (EXAMPLE_USE_DEMO==USER_THREAD_DEMO)

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

static int test_data=0;
hfthread_mutex_t test_lock=NULL_MUTEX;
void test_thread_start(void);

#define PRINTF(...)	HF_Debug(DEBUG_LEVEL,__VA_ARGS__)

USER_FUNC void display_mallinfo(void)
{
//	struct mallinfo mi;

//	mi = mallinfo();

//	PRINTF("Total non-mmapped bytes (arena):       %d\n", mi.arena);
//	PRINTF("# of free chunks (ordblks):            %d\n", mi.ordblks);
//	PRINTF("# of free fastbin blocks (smblks):     %d\n", mi.smblks);
//	PRINTF("# of mapped regions (hblks):           %d\n", mi.hblks);
//	PRINTF("Bytes in mapped regions (hblkhd):      %d\n", mi.hblkhd);
//	PRINTF("Max. total allocated space (usmblks):  %d\n", mi.usmblks);
//	PRINTF("Free bytes held in fastbins (fsmblks): %d\n", mi.fsmblks);
//	PRINTF("Total allocated space (uordblks):      %d\n", mi.uordblks);
//	PRINTF("Total free space (fordblks):           %d\n", mi.fordblks);
//	PRINTF("Topmost releasable block (keepcost):   %d\n", mi.keepcost);
}

#define TEST_TIMER_ID		(1)

void USER_FUNC test_timer_callback1( hftimer_handle_t htimer )
{
		if(hftimer_get_timer_id(htimer)==TEST_TIMER_ID)
		{
			//u_printf("TEST_TIMER_ID active\n");
			if(hfgpio_fpin_is_high(HFGPIO_F_NREADY))
				hfgpio_fset_out_low(HFGPIO_F_NREADY);
			else
				hfgpio_fset_out_high(HFGPIO_F_NREADY);
			//hftimer_start(htimer);//如果create的时候auto_reload设置为false，手动再次启动timer	
		}
}

static hftimer_handle_t test_timer=NULL;
static HFMSGQ_HANDLE test_msgq=NULL;

USER_FUNC static void test_thread_func(void* arg)
{
	int fd,id,ret;
	int tmp=1,recv_num=0;
	char recv[32]={0};	
	fd_set rset;
	struct timeval timeout;
	struct sockaddr_in addr;
	
	id = (int)arg;
	memset((char*)&addr,0,sizeof(addr));
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(10001+id);
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	
	tmp=1;
	setsockopt(fd, SOL_SOCKET,SO_BROADCAST,&tmp,sizeof(tmp));
	hfnet_set_udp_broadcast_port_valid(10001,10001+id);
	bind(fd, (struct sockaddr*)&addr, sizeof(addr));
	hftimer_start(test_timer);
	FD_ZERO(&rset);
	//使能当前线程的看门狗，30秒
	hfthread_enable_softwatchdog(NULL,30);
	//启动定时器
	while(1)
	{
		void *msg;
		hfthread_mutext_lock(test_lock);
		//把下面代码注释调，模块会30s后自动重启
		hfthread_reset_softwatchdog(NULL);
		test_data=id;
		HF_Debug(DEBUG_LEVEL,"thread %d is running\n",test_data);
		hfthread_mutext_unlock(test_lock);
		
		if(hfmsgq_recv(test_msgq,&msg,10,0)==HF_SUCCESS)
		{
			u_printf("recv msg %p\n",msg);
		}
		FD_SET(fd,&rset);
		timeout.tv_sec= 3;
		timeout.tv_usec= 0;
		ret = select(fd+1, &rset, NULL, NULL, &timeout);
		if(ret<=0)
			continue;
		
		if (FD_ISSET(fd, &rset))
		{
			tmp = sizeof(addr);
			recv_num = recvfrom(fd, recv, 32, 0, (struct sockaddr *)&addr, (socklen_t*)&tmp);
			if(recv_num>0)
			{
				HF_Debug(DEBUG_LEVEL,"thread %d recvnum=%d\n",id,recv_num);
				sprintf(recv,"thread %d\r\n",id);
				sendto(fd,recv,strlen(recv),0,(struct sockaddr *)&addr,sizeof(addr));
			}
		}	

	}
	
	hftimer_delete(test_timer);
}

USER_FUNC void test_thread_and_timer_start(void)
{
	if(hfthread_mutext_new(&test_lock)!=0)
	{
		HF_Debug(DEBUG_LEVEL,"create mutex fail\n");
		return;
	}
	//创建一个自动定时器，每1s钟触发一次。
	if((test_timer = hftimer_create("TEST-TIMER",1000,true,TEST_TIMER_ID,test_timer_callback1,0))==NULL)
	{
		u_printf("create timer fail\n");
	}
	test_msgq = hfmsgq_create(10,4);
	if(test_msgq==NULL)
	{
		u_printf("create msgq fail\n");
	}
	
	hfthread_create(test_thread_func,"app_main_test",256,(void*)1,1,NULL,NULL);
	//hfthread_create(test_thread_func,"app_main_test1",256,(void*)2,1,NULL,NULL);
}


int USER_FUNC app_main (void)
{
	time_t now=time(NULL);
	
	HF_Debug(DEBUG_LEVEL,"[THREAD DEMO]sdk version(%s),the app_main start time is %d %s\n",hfsys_get_sdk_version(),now,ctime(&now));
	if(hfgpio_fmap_check(HFM_TYPE_LPB100)!=0)
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
	if(hfnet_start_uart(HFTHREAD_PRIORITIES_LOW,NULL)!=HF_SUCCESS)
	{
		HF_Debug(DEBUG_WARN,"start uart fail!\n");
	}
	if(hfnet_start_socketa(HFTHREAD_PRIORITIES_LOW,(hfnet_callback_t)NULL)!=HF_SUCCESS)
	{
		HF_Debug(DEBUG_WARN,"start socketa fail\n");
	}
	if(hfnet_start_socketb(HFTHREAD_PRIORITIES_LOW,(hfnet_callback_t)NULL)!=HF_SUCCESS)
	{
		HF_Debug(DEBUG_WARN,"start socketb fail\n");
	}
	
	test_thread_and_timer_start();
	
	return 1;
	
}

#endif
