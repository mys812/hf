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
#include <httpc/httpc.h>
#include "../example.h"
#include <stdio.h>

#define HFGPIO_F_TEST_TIMER			HFGPIO_F_USER_DEFINE

#define REG_TIM0_CNT_INIT   (*(volatile unsigned long *) 0x4001B008)
#define REG_TIM0_CTRL		(*(volatile unsigned long *) 0x4001B000)
#define REG_TIM0_CNT   		(*(volatile unsigned long *) 0x4001B00C)

#define TIME_CTRL_CNT_CLEAR			(0x00000001)
#define TIME_CTRL_PAUSE				(0x00000002)
#define TIME_CTRL_INTC					(0x00000004)
#define TIME_CTRL_INTE					(0x00000008)
#define TIME_CTRL_CNT_INIT_LOAD		(0x00000010)

#if (EXAMPLE_USE_DEMO==USER_TIMER_DEMO)

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
#ifdef __LPB100U__	
	HFM_NOPIN,	//HFGPIO_F_SPI_MISO
	HFM_NOPIN,	//HFGPIO_F_SPI_CLK
	HFM_NOPIN,	//HFGPIO_F_SPI_CS
	HFM_NOPIN,	//HFGPIO_F_SPI_MOSI	
#else
	HF_M_PIN(27),	//HFGPIO_F_SPI_MISO
	HF_M_PIN(28),	//HFGPIO_F_SPI_CLK
	HF_M_PIN(29),	//HFGPIO_F_SPI_CS
	HF_M_PIN(30),	//HFGPIO_F_SPI_MOSI
#endif	
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
	
	HF_M_PIN(11),	//HFGPIO_F_USER_DEFINE
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
#define TEST_TIMER2_ID		(2)
#define TEST_TIMER3_ID		(3)

hftimer_handle_t test_timer_hardware=NULL;

void USER_FUNC test_timer_callback( hftimer_handle_t htimer )
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
		else if(hftimer_get_timer_id(htimer)==TEST_TIMER2_ID)
		{
			//u_printf("TEST_TIMER_ID active\n");
			if(hfgpio_fpin_is_high(HFGPIO_F_NLINK))
			{
				hftimer_change_period(htimer,1000);
				hfgpio_fset_out_low(HFGPIO_F_NLINK);
			}
			else
			{
				hftimer_change_period(htimer,3000);
				hfgpio_fset_out_high(HFGPIO_F_NLINK);
			}
			
			//hftimer_start(htimer);		
		}
		else if(hftimer_get_timer_id(htimer)==TEST_TIMER3_ID)
		{
			if(hfgpio_fpin_is_high(HFGPIO_F_TEST_TIMER))
				hfgpio_fset_out_low(HFGPIO_F_TEST_TIMER);
			else
				hfgpio_fset_out_high(HFGPIO_F_TEST_TIMER);
		}
		else
		{
			u_printf("%p\n",htimer);
		}
}


void app_init(void)
{
	u_printf("app_init\n\n");
	
}


USER_FUNC static void test_thread_func(void* arg)
{
	int fd,id;
	int tmp=1,recv_num=0;
	char recv[32]={0};	
	//char *p;
	struct sockaddr_in addr;
	hftimer_handle_t test_timer=NULL;
	hftimer_handle_t test_timer2=NULL;
	
	id = (int)arg;
	memset((char*)&addr,0,sizeof(addr));
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(10001+id);
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	// = socket()
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	bind(fd, (struct sockaddr*)&addr, sizeof(addr));
	tmp=1;
	setsockopt(fd, SOL_SOCKET,SO_BROADCAST,&tmp,sizeof(tmp));
	
	//创建一个自动定时器，每1s钟触发一次。
	if((test_timer = hftimer_create("TEST-TIMER",1000,true,TEST_TIMER_ID,test_timer_callback,0))==NULL)
	{
		u_printf("create timer 1 fail\n");
	}
	if((test_timer2 = hftimer_create("TEST-TIMER2",1000,false,TEST_TIMER2_ID,test_timer_callback,0))==NULL)
	{
		u_printf("create timer 2 fail\n");
	}

#if 0
	//0.5ms hardware timer,硬件定时器周期以微秒计算,只能建一个硬件定时器.
	if((test_timer_hardware=hftimer_create("HDW-TIMER",500,true,TEST_TIMER3_ID,test_timer_callback,HFTIMER_FLAG_HARDWARE_TIMER))==NULL)
#else
	if((test_timer_hardware=hftimer_create("HDW-TIMER",110000,true,TEST_TIMER3_ID,test_timer_callback,HFTIMER_FLAG_HARDWARE_TIMER))==NULL)
#endif
	{
		u_printf("create  hardware timer fail\n");
	}
	//启动定时器
	hftimer_start(test_timer);
	hftimer_start(test_timer2);
	hftimer_start(test_timer_hardware);
	while(1)
	{
		REG_TIM0_CTRL |= TIME_CTRL_CNT_CLEAR;
		u_printf("cnt0=%d %08X %d\n",REG_TIM0_CNT,REG_TIM0_CTRL,REG_TIM0_CNT_INIT);
		u_printf("cnt1=%d %08X %d\n",REG_TIM0_CNT,REG_TIM0_CTRL,REG_TIM0_CNT_INIT);
		u_printf("cnt2=%d %08X %d\n",REG_TIM0_CNT,REG_TIM0_CTRL,REG_TIM0_CNT_INIT);
		REG_TIM0_CTRL |= TIME_CTRL_CNT_CLEAR;
		u_printf("cnt3=%d %08X %d\n",REG_TIM0_CNT,REG_TIM0_CTRL);
		msleep(1);
	}
	while(1)
	{
		hfthread_mutext_lock(test_lock);
		test_data=id;
		HF_Debug(DEBUG_LEVEL_LOW,"thread %d is running\n",test_data);
		msleep(3000);
		u_printf("counter:%u\n",hftimer_get_counter(test_timer_hardware));
		hfthread_mutext_unlock(test_lock);
		tmp = sizeof(addr);
		//recv_num = recvfrom(fd, recv, 32, 0, (struct sockaddr *)&addr, (socklen_t*)&tmp);
		if(recv_num>0)
		{
			HF_Debug(DEBUG_LEVEL,"thread %d recvnum=%d\n",id,recv_num);
			sprintf(recv,"thread %d\r\n",id);
			sendto(fd,recv,strlen(recv),0,(struct sockaddr *)&addr,sizeof(addr));
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
	
	hfthread_create(test_thread_func,"app_main_test",256,(void*)1,1,NULL,NULL);
	//hfthread_create(test_thread_func,"app_main_test1",256,(void*)2,1,NULL,NULL);
	
}


int USER_FUNC app_main (void)
{
	time_t now=time(NULL);
	
	HF_Debug(DEBUG_LEVEL,"sdk version(%s),the app_main start time is %d %s[TIMER DEMO]\n",hfsys_get_sdk_version(),now,ctime(&now));
	if(hfgpio_fmap_check()!=0)
	{
		while(1)
		{
			HF_Debug(DEBUG_ERROR,"gpio map file error\n");
			msleep(1000);
		}
		return 0;
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
