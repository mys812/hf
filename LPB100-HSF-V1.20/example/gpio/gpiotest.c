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

#if (EXAMPLE_USE_DEMO==USER_GPIO_DEMO)

#define HFGPIO_F_TCP_LINK			(HFGPIO_F_USER_DEFINE+0)
#define HFGPIO_F_USER_RELOAD		(HFGPIO_F_USER_DEFINE+1)


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
	
	HFM_NOPIN,//HF_M_PIN(43),	//HFGPIO_F_NLINK
	HF_M_PIN(44),	//HFGPIO_F_NREADY
	HFM_NOPIN,//HF_M_PIN(45),	//HFGPIO_F_NRELOAD
	HF_M_PIN(7),	//HFGPIO_F_SLEEP_RQ
	HF_M_PIN(8),	//HFGPIO_F_SLEEP_ON

	HFM_NOPIN,		//HFGPIO_F_RESERVE0
	HFM_NOPIN,		//HFGPIO_F_RESERVE1
	HFM_NOPIN,		//HFGPIO_F_RESERVE2
	HFM_NOPIN,		//HFGPIO_F_RESERVE3
	HFM_NOPIN,		//HFGPIO_F_RESERVE4
	HFM_NOPIN,		//HFGPIO_F_RESERVE5
	
	HF_M_PIN(43),	//HFGPIO_F_USER_DEFINE,HFGPIO_F_TCP_LINK
	HF_M_PIN(45)    //HFGPIO_F_USER_RELOAD
};

static void USER_FUNC do_user_reload(uint32_t arg1,uint32_t arg2);
static char at_cmd_rsp[128]={0};
static int press_reload_key=0;

static void USER_FUNC do_user_reload(uint32_t arg1,uint32_t arg2)
{
	time_t now=time(NULL);
	
	if(hfgpio_fpin_is_high(HFGPIO_F_NREADY))
	{
		hfgpio_fset_out_low(HFGPIO_F_NREADY);
	}
	else
	{
		hfgpio_fset_out_high(HFGPIO_F_NREADY);
	}
	//LPB开发板RELOD脚上拉，如果为高电平说明释放按键
	if(hfgpio_fpin_is_high(HFGPIO_F_USER_RELOAD))
	{
		press_reload_key=1;
		u_printf("release the reload button!%d %d\n",now,hfgpio_fpin_is_high(HFGPIO_F_SLEEP_RQ));
	}
	else
	{
		press_reload_key=0;
		u_printf("press the reload button!%d %d\n",now,hfgpio_fpin_is_high(HFGPIO_F_SLEEP_RQ));
	}
}


static void USER_FUNC do_user_rq(uint32_t arg1,uint32_t arg2)
{
	time_t now=time(NULL);
	
	u_printf("press the RQ button!%d\n",now);
	if(hfgpio_fpin_is_high(HFGPIO_F_NREADY))
	{
		hfgpio_fset_out_low(HFGPIO_F_NREADY);
	}
	else
	{
		hfgpio_fset_out_high(HFGPIO_F_NREADY);
	}	
}

static void USER_FUNC test_gpio_start()
{
	hfgpio_fset_out_high(HFGPIO_F_TCP_LINK);
	if(hfgpio_configure_fpin_interrupt(HFGPIO_F_USER_RELOAD,HFPIO_IT_EDGE,do_user_reload,1)!=HF_SUCCESS)
	{
		u_printf("configure HFGPIO_F_USER_RELOAD fail\n");
		return;
	}
	
	if(hfgpio_configure_fpin_interrupt(HFGPIO_F_SLEEP_RQ,HFPIO_IT_FALL_EDGE,do_user_rq,1)!=HF_SUCCESS)
	{
		u_printf("configure HFGPIO_F_SLEEP_RQ fail\n");
		return;
	}
	
	while(1)
	{
		if(press_reload_key)
		{
			hfsys_reload();
			hfat_send_cmd("AT+SMTLK\r\n",sizeof("AT+SMTLK\r\n"),at_cmd_rsp,64);
		}
		else
			msleep(100);
	}
}

const hfat_cmd_t user_define_at_cmds_table[]=
{
	{NULL,NULL,NULL,NULL} //the last item must be null
};

static int USER_FUNC socketa_recv_callback(uint32_t event,char *data,uint32_t len,uint32_t buf_len)
{
	if(event==HFNET_SOCKETA_CONNECTED)
	{
		hfgpio_fset_out_low(HFGPIO_F_TCP_LINK);
	}
	else if(event==HFNET_SOCKETA_DISCONNECTED)
	{
		hfgpio_fset_out_high(HFGPIO_F_TCP_LINK);
	}
	else if(event==HFNET_SOCKETA_DATA_READY)
	{
		HF_Debug(DEBUG_LEVEL_LOW,"[%d]socketa recv %d bytes data %d %c\n",event,len,buf_len,data[0]);
	}
	
	return len;
}

static int USER_FUNC socketb_recv_callback(uint32_t event,char *data,uint32_t len,uint32_t buf_len)
{
	if(event==HFNET_SOCKETB_CONNECTED)
	{
		u_printf("socket b connected!\n");
	}
	else if(event==HFNET_SOCKETB_DISCONNECTED)
	{
		u_printf("socket b disconnected!\n");
	}	
	else if(event==HFNET_SOCKETB_DATA_READY)
		HF_Debug(DEBUG_LEVEL_LOW,"[%d]socketb recv %d bytes data %d %c\n",event,len,buf_len,data[0]);
	
	return len;
}

static int USER_FUNC uart_recv_callback(uint32_t event,char *data,uint32_t len,uint32_t buf_len)
{
	HF_Debug(DEBUG_LEVEL_LOW,"[%d]uart recv %d bytes data %d\n",event,len,buf_len);
	return len;
}

int USER_FUNC app_main (void)
{
	time_t now=time(NULL);
	
	u_printf("[GPIO DEMO]sdk version(%s),the app_main start time is %d %s\n",hfsys_get_sdk_version(),now,ctime(&now));
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
	if(hfnet_start_uart(HFTHREAD_PRIORITIES_LOW,(hfnet_callback_t)uart_recv_callback)!=HF_SUCCESS)
	{
		HF_Debug(DEBUG_WARN,"start uart fail!\n");
	}
	if(hfnet_start_socketa(HFTHREAD_PRIORITIES_LOW,(hfnet_callback_t)socketa_recv_callback)!=HF_SUCCESS)
	{
		HF_Debug(DEBUG_WARN,"start socketa fail\n");
	}
	if(hfnet_start_socketb(HFTHREAD_PRIORITIES_LOW,(hfnet_callback_t)socketb_recv_callback)!=HF_SUCCESS)
	{
		HF_Debug(DEBUG_WARN,"start socketb fail\n");
	}

	test_gpio_start();
	
	return 1;
}

#endif

