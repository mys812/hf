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

#if (EXAMPLE_USE_DEMO==USER_CALLBACK_DEMO)

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

static hftimer_handle_t hnlink_timer=NULL;
#define NLINK_FALSH_TIMER_ID		(1)

void USER_FUNC nlink_falsh_timer_callback( hftimer_handle_t htimer )
{
	if(hftimer_get_timer_id(htimer)==NLINK_FALSH_TIMER_ID)
	{
		if(hfgpio_fpin_is_high(HFGPIO_F_NLINK))
			hfgpio_fset_out_low(HFGPIO_F_NLINK);
		else
			hfgpio_fset_out_high(HFGPIO_F_NLINK);
		//hftimer_start(htimer);//如果create的时候auto_reload设置为false，手动再次启动timer
	}
}

static int USER_FUNC assis_ex_recv_callback(uint32_t event,char *data,uint32_t len,uint32_t buf_len)
{
	if(event == HFNET_ASSIS_DATA_READY)
	{
		char tmp[64]={0};
		char rsp[64]={0};
		char *ip[6]={0};
		char *mac[3]={0};
		char response[40]={0};

		uint32_t ip_addr;
		uint16_t port;
 	
		MEMCPY(&ip_addr, data+len, sizeof(struct ip_addr));
		Memcpy(&port, data+len+sizeof(struct ip_addr), sizeof(port));
		u_printf("ip:%s, port:%d\n", inet_ntoa(ip_addr),port);
		
		MEMCPY(tmp, data, len);
		if(strcasecmp("HF-A11ASSISTHREAD", tmp)==0)
		{	
			hfat_send_cmd("AT+WANN\r\n", sizeof("AT+WANN\r\n"), rsp, 64);
			if(hfat_get_words(rsp, ip, 6)>0)
			{
				u_printf("local ip:%s\n", ip[2]);
				sprintf(response,"%s,",ip[2]);
			}

			memset(rsp, 0, sizeof(rsp));
			hfat_send_cmd("AT+WSMAC\r\n", sizeof("AT+WSMC\r\n"), rsp, 64);
			u_printf("AT+WSMAC's response:%s\n",rsp);
			if(hfat_get_words(rsp, mac, 3)>0)
			{
				u_printf("local mac:%s\n", mac[1]);
				strcat(response,mac[1]);
			}

			hfnet_assis_write(response, sizeof(response), ip_addr, port);
			
			return 0;
		}
	}
	return len;
}

static int USER_FUNC socketa_recv_callback(uint32_t event,char *data,uint32_t len,uint32_t buf_len)
{
	if(event==HFNET_SOCKETA_CONNECTED)
	{
		hfnet_socketa_send("CONNECT OK",sizeof("CONNECT OK")-1,1000);
	}
	else if(event==HFNET_SOCKETA_DISCONNECTED)
	{
		hfuart_send(HFUART0,"TCP DISCONNECTED\r\n",sizeof("TCP DISCONNECTED\r\n")-1,1000);
	}
	else if(event==HFNET_SOCKETA_DATA_READY)
	{
		if(len>128)
		{
			hfnet_socketa_send("INVALID PACKET\n",sizeof("INVALID PACKET\n")-1,1000);
		}
		data[len]=0;
		if(strcasecmp("GPIO NLINK LOW",data)==0)
		{
			hftimer_stop(hnlink_timer);
			hfgpio_fset_out_high(HFGPIO_F_NLINK);
		}
		else if(strcasecmp("GPIO NLINK HIGH",data)==0)
		{
			hftimer_stop(hnlink_timer);
			hfgpio_fset_out_low(HFGPIO_F_NLINK);
		}
		else if(strcasecmp("GPIO NLINK FALSH",data)==0)
		{
			hftimer_start(hnlink_timer);
		}
		else
		{
			hfuart_send(HFUART0,data,len,1000);
		}
		HF_Debug(DEBUG_LEVEL_LOW,"[%d]socketa recv %d bytes data %d %c\n",event,len,buf_len,data[0]);
	}
	//返回0，HSF透传机制不再向串口发数据
	return 0;
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

static int hfsys_event_callback( uint32_t event_id,void * param)
{
	switch(event_id)
	{
		case HFE_WIFI_STA_CONNECTED:
			u_printf("wifi sta connected!!\n");
			break;
		case HFE_WIFI_STA_DISCONNECTED:
			u_printf("wifi sta disconnected!!\n");
			break;
		case HFE_DHCP_OK:
		{
			uint32_t *p_ip;
			p_ip = (uint32_t*)param;
			u_printf("dhcp ok %08X!\n",*p_ip);
		}
			break;
		case HFE_SMTLK_OK:
			u_printf("smtlk ok!\n");
			break;
		case HFE_CONFIG_RELOAD:
			u_printf("reload!\n");
			break;
		default:
			break;
	}
	return 0;
}

static void show_reset_reason(void)
{
	uint32_t reset_reason=0;
	
	
	reset_reason = hfsys_get_reset_reason();
	
	if(reset_reason&HFSYS_RESET_REASON_ERESET)
	{
		u_printf("HFSYS_RESET_REASON_ERESET\n");
	}
	if(reset_reason&HFSYS_RESET_REASON_IRESET0)
	{
		u_printf("HFSYS_RESET_REASON_IRESET0\n");
	}
	if(reset_reason&HFSYS_RESET_REASON_IRESET1)
	{
		u_printf("HFSYS_RESET_REASON_IRESET1\n");
	}
	if(reset_reason==HFSYS_RESET_REASON_NORMAL)
	{
		u_printf("HFSYS_RESET_REASON_NORMAL\n");
	}
	if(reset_reason&HFSYS_RESET_REASON_WPS)
	{
		u_printf("HFSYS_RESET_REASON_WPS\n");
	}
	if(reset_reason&HFSYS_RESET_REASON_SMARTLINK_START)
	{
		u_printf("HFSYS_RESET_REASON_SMARTLINK_START\n");
	}
	if(reset_reason&HFSYS_RESET_REASON_SMARTLINK_OK)
	{
		u_printf("HFSYS_RESET_REASON_SMARTLINK_OK\n");
	}
	
	return;
}

int USER_FUNC app_main (void)
{
	time_t now=time(NULL);
	
	u_printf("[CALLBACK DEMO]sdk version(%s),the app_main start time is %d %s\n",hfsys_get_sdk_version(),now,ctime(&now));
	
	if(hfsys_register_system_event((hfsys_event_callback_t)hfsys_event_callback)!=HF_SUCCESS)
	{
		u_printf("register system event fail\n");
	}
	
	if(hfgpio_fmap_check()!=0)
	{
		while(1)
		{
			HF_Debug(DEBUG_ERROR,"gpio map file error\n");
			msleep(1000);
		}
		return 0;
	}
	
	show_reset_reason();
	
	while(!hfnet_wifi_is_active())
	{
		msleep(50);
	}
	
	//if(hfnet_start_assis(ASSIS_PORT)!=HF_SUCCESS)
	if(hfnet_start_assis_ex(ASSIS_PORT,(hfnet_callback_t)assis_ex_recv_callback)!=HF_SUCCESS)
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
	
	//创建一个自动定时器，每1s钟触发一次。
	if((hnlink_timer = hftimer_create("NLINK-FALSH-TIMER",1000,true,1,nlink_falsh_timer_callback,0))==NULL)
	{
		u_printf("create timer fail\n");
	}
	
	return 1;
}

#endif

