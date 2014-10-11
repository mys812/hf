/**
brief:	
		You can input "10.10.100.254/AT+WSSSID=?" to query the wsssid.
		Input "10.10.100.254/AT+WSSSID=abcd" to set wsssid of module.
		Tips:10.10.100.254 is module's ip address.
		
Author:	Cyrus.xu
Data:	2014/01/13
 **/

 
#include <hsf.h>
#include <hfir.h>
#include <stdlib.h>
#include <string.h>
//#include <malloc.h>
#include <stdio.h>
#include <httpc/httpc.h>
#include "../example.h"

#if (EXAMPLE_USE_DEMO==USER_URL_DEMO)

static char user_define = -1;

const int hf_gpio_fid_to_pid_map_table[HFM_MAX_FUNC_CODE]=
{
	HF_M_PIN(2),	//HFGPIO_F_JTAG_TCK
	HFM_NOPIN,	//HFGPIO_F_JTAG_TDO
	HFM_NOPIN,	//HFGPIO_F_JTAG_TDI
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
		
	HF_M_PIN(15),		//HFGPIO_F_WPS
	HFM_NOPIN,		//HFGPIO_F_RESERVE1
	HFM_NOPIN,		//HFGPIO_F_RESERVE2
	HFM_NOPIN,		//HFGPIO_F_RESERVE3
	HFM_NOPIN,		//HFGPIO_F_RESERVE4
	HFM_NOPIN,		//HFGPIO_F_RESERVE5
	
	HFM_NOPIN,	//HFGPIO_F_USER_DEFINE
};
static int web_user_define(pat_session_t s,int argc,char *argv[],char *rsp,int len);

const hfat_cmd_t user_define_at_cmds_table[]=
{
	{"URT", web_user_define, "   AT+URT=Set/Get the return value of user defined webpage.\r\n", NULL},
	{NULL,NULL,NULL,NULL} //the last item must be null
};

static int USER_FUNC socketa_recv_callback(uint32_t event,char *data,uint32_t len,uint32_t buf_len)
{	
	return len;
}

static int USER_FUNC socketb_recv_callback(uint32_t event,char *data,uint32_t len,uint32_t buf_len)
{			
	return len;
}

static int USER_FUNC uart_recv_callback(uint32_t event,char *data,uint32_t len,uint32_t buf_len)
{
	return len;
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

static int web_user_define(pat_session_t s,int argc,char *argv[],char *rsp,int len)
{
	if(argc == 0)
		sprintf(rsp, "=%d", user_define);
	else
		user_define = atoi(argv[0]);
	return 0;
}

static char *strnstr(const char *s, const char *find, size_t slen)
{
	int i,flen;
	
	flen = strlen(find);
	if(flen>slen)
		return NULL;
	
	for(i=0;i<=slen-flen;i++)
	{
		if(s[i]==*find)
		{
			if(strncmp(s+i,find,slen-i)==0)
				return (char*)(s+i);
		}
	}
	
	return NULL;
}
static int http_get_value(char *str, char *value)
{
	char *tmp = str;
	char i = 0;
	
	while((*tmp != '&') && (*tmp != '\0'))
	{
		i++;
		tmp++;
	}
	
	strncpy(value, str, i);
	if(value[0] == '?')
		return 0;
	else
		return 1;
}
/*

User defined parameter supports a maximum of 70 bytes.
Rsp supports a maximum of 1400 bytes.

*/
static int hfhttpd_url_callback_test(char *url, char *rsp)
{

	
	char i = 0;
	char *p1, *fname_tmp;
	char value[30] = {0};
	char at_rsp[64] = {0};	
	char at_cmd[50] = {0};
	char ret = -1;
	
	fname_tmp = url;


	if((p1 = strnstr(fname_tmp, "/AT+WSSSID=", strlen("/AT+WSSSID="))) != NULL)
	{
		ret = http_get_value(p1+strlen("/AT+WSSSID="), value);		
		
		if(ret == 0)
		{
			hfat_send_cmd("AT+WSSSID\r\n", strlen("AT+WSSSID\r\n"), at_rsp, sizeof(at_rsp));	
			sprintf(rsp, at_rsp);
		}
		else
		{
			sprintf(at_cmd, "%s%s\r\n", "AT+WSSSID=", value);
			hfat_send_cmd(at_cmd, strlen(at_cmd), at_rsp, sizeof(at_rsp));	
			if(0 == strcmp("+ok", at_rsp))
				sprintf(rsp, "set ok.\r\n");
			else
				sprintf(rsp, "set fail.\r\n");
		}
		return 0;
	}
	else if((p1 = strnstr(fname_tmp, "/AT+WSKEY=", strlen("/AT+WSKEY="))) != NULL)
	{
		ret = http_get_value(p1+strlen("/AT+WSKEY="), value);		
		
		if(ret == 0)
		{
			hfat_send_cmd("AT+WSKEY\r\n", strlen("AT+WSKEY\r\n"), at_rsp, sizeof(at_rsp));	
			sprintf(rsp, at_rsp);
		}
		else
		{
			sprintf(at_cmd, "%s%s\r\n", "AT+WSKEY=", value);
			hfat_send_cmd(at_cmd, strlen(at_cmd), at_rsp, sizeof(at_rsp));	
			if(0 == strcmp("+ok", at_rsp))
				sprintf(rsp, "set ok.\r\n");
			else
				sprintf(rsp, "set fail.\r\n");
		}
		return 0;
	}
	else if((p1 = strnstr(fname_tmp, "/AT+WMODE=", strlen("/AT+WMODE="))) != NULL)
	{
		ret = http_get_value(p1+strlen("/AT+WMODE="), value);		
		
		if(ret == 0)
		{
			hfat_send_cmd("AT+WMODE\r\n", strlen("AT+WMODE\r\n"), at_rsp, sizeof(at_rsp));	
			sprintf(rsp, at_rsp);
		}
		else
		{
			sprintf(at_cmd, "%s%s\r\n", "AT+WMODE=", value);
			hfat_send_cmd(at_cmd, strlen(at_cmd), at_rsp, sizeof(at_rsp));	
			if(0 == strcmp("+ok", at_rsp))
				sprintf(rsp, "set ok.\r\n");
			else
				sprintf(rsp, "set fail.\r\n");
		}
		
		return 0;
	}
	else if((p1 = strnstr(fname_tmp, "/iweb.html", strlen("/iweb.html"))) != NULL)
	{
		/***************************************	
		"iweb.html" is our internal upgrade webpage.
		If you want to replace it ,please return 0;
		If not, just return -1.
		***************************************/
		if( 0 == user_define)
			sprintf(rsp, "user define page.");
		return user_define;
	}

	return -1;
}

int USER_FUNC app_main (void)
{
	time_t now=time(NULL);
	
	HF_Debug(DEBUG_LEVEL,"[URL_CALLBACK DEMO]sdk version(%s),the app_main start time is %d %s\n",hfsys_get_sdk_version(),now,ctime(&now));
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
	if(hfnet_start_assis(ASSIS_PORT)!=HF_SUCCESS)
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
	if(hfnet_start_httpd(HFTHREAD_PRIORITIES_MID)!=HF_SUCCESS)
	{
		HF_Debug(DEBUG_WARN,"start httpd fail\n");
	}

	if(HF_SUCCESS != (hfhttpd_url_callback_register(hfhttpd_url_callback_test, 0)))
	{
		HF_Debug(DEBUG_LEVEL, "register url callback fail\r\n");
	}
	
	return 1;
}

#endif

