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
#include <stdio.h>
#include <string.h>
#include <httpc/httpc.h>
#include "../example.h"
#include <md5.h>

#if (EXAMPLE_USE_DEMO==UPDATE_TEST_DEMO)

#define HFGPIO_F_UPGRADE_LED		HFGPIO_F_USER_DEFINE
#define HFGPIO_F_UPGRADE_GPIO		(HFGPIO_F_USER_DEFINE+1)

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
	
	HF_M_PIN(23),	//HFGPIO_F_UART1_TX,
	HFM_NOPIN,	//HFGPIO_F_UART1_RTS,
	HF_M_PIN(8),	//HFGPIO_F_UART1_RX,
	HFM_NOPIN,	//HFGPIO_F_UART1_CTS,
	
	HF_M_PIN(43),	//HFGPIO_F_NLINK
	HFM_NOPIN,//HF_M_PIN(44),	//HFGPIO_F_NREADY
	HF_M_PIN(45),	//HFGPIO_F_NRELOAD
	HFM_NOPIN,//HF_M_PIN(7),	//HFGPIO_F_SLEEP_RQ
	HFM_NOPIN,//HF_M_PIN(8),	//HFGPIO_F_SLEEP_ON

	HFM_NOPIN,		//HFGPIO_F_RESERVE0
	HFM_NOPIN,		//HFGPIO_F_RESERVE1
	HFM_NOPIN,		//HFGPIO_F_RESERVE2
	HFM_NOPIN,		//HFGPIO_F_RESERVE3
	HFM_NOPIN,		//HFGPIO_F_RESERVE4
	HFM_NOPIN,		//HFGPIO_F_RESERVE5
	
	HF_M_PIN(44),	//HFGPIO_F_UPGRADE_LED
	HF_M_PIN(7),	//HFGPIO_F_UPGRADE_GPIO

};

static int USER_FUNC hf_atcmd_upgrade_sw(pat_session_t s,int argc,char *argv[],char *rsp,int len);
static int USER_FUNC test_update_as_http(char *purl,char *type);

const hfat_cmd_t user_define_at_cmds_table[]=
{
	{"UPGRADESW",hf_atcmd_upgrade_sw,"   AT+UPGRADESW=url\r\n",NULL},
	{NULL,NULL,NULL,NULL} //the last item must be null
};


static int USER_FUNC hf_atcmd_upgrade_sw(pat_session_t s,int argc,char *argv[],char *rsp,int len)
{	
	if(argc<2)
	{
		return -1;
	}
	
	test_update_as_http(argv[0],argv[1]);
	
	return 0;
}


static int USER_FUNC uart_recv_callback(uint32_t event,char *data,uint32_t len,uint32_t buf_len)
{
	return len;
}

void USER_FUNC update_timer_callback( hftimer_handle_t htimer )
{
	if(hfgpio_fpin_is_high(HFGPIO_F_NREADY))
		hfgpio_fset_out_low(HFGPIO_F_NREADY);
	else
		hfgpio_fset_out_high(HFGPIO_F_NREADY);
}

static int USER_FUNC test_update_as_http(char *purl,char *type)
{
	httpc_req_t  http_req;
	char *content_data=NULL;
	char *temp_buf=NULL;
	parsed_url_t url={0};
	http_session_t hhttp=0;
	int total_size,read_size=0;
	int rv=0;
	tls_init_config_t  *tls_cfg=NULL;
	char *test_url=purl;
	hftimer_handle_t upg_timer=NULL;
	struct MD5Context md5_ctx;
	uint8_t digest[16]={0};
	HFUPDATE_TYPE_E  upg_type;
	
	bzero(&http_req,sizeof(http_req));
	http_req.type = HTTP_GET;
	http_req.version=HTTP_VER_1_1;
	
	if(strcasecmp(type,"wifi")==0)
	{
		upg_type = HFUPDATE_WIFIFW;
	}
	else 
		upg_type = HFUPDATE_SW;
	if((temp_buf = (char*)hfmem_malloc(256))==NULL)
	{
		u_printf("no memory\n");
		rv= -HF_E_NOMEM;
		goto exit;
	}	
	bzero(temp_buf,sizeof(temp_buf));
	if((rv=hfhttp_parse_URL(test_url,temp_buf , 256, &url))!=HF_SUCCESS)
	{
		goto exit;
	}

	if((rv=hfhttp_open_session(&hhttp,test_url,0,tls_cfg,3))!=HF_SUCCESS)
	{
		u_printf("http open fail\n");
		goto exit;
	}

	hfsys_disable_all_soft_watchdogs();
	hfupdate_start(upg_type);
	http_req.resource = url.resource;
	hfhttp_prepare_req(hhttp,&http_req,HDR_ADD_CONN_CLOSE);
	hfhttp_add_header(hhttp,"Range","bytes=0");
	if((rv=hfhttp_send_request(hhttp,&http_req))!=HF_SUCCESS)
	{
		u_printf("http send request fail\n");
		goto exit;
	}
	
	content_data = (char*)hfmem_malloc(256);
	if(content_data==NULL)
	{
		rv= -HF_E_NOMEM;
		goto exit;
	}
	total_size=0;
	bzero(content_data,256);

	if((upg_timer = hftimer_create("UPG-TIMER",100,true,1,update_timer_callback,0))==NULL)
	{
		u_printf("create timer 1 fail\n");
		goto exit;
	}
	
	hftimer_start(upg_timer);
	MD5Init(&md5_ctx);
	while((read_size=hfhttp_read_content(hhttp,content_data,256))>0)
	{
		hfupdate_write_file(upg_type, total_size,content_data, read_size);
		MD5Update(&md5_ctx,(uint8_t*)content_data,read_size);
		total_size+=read_size;
		u_printf("download file:[%d] [%d]\r",total_size,read_size);
	}
	MD5Final(digest,&md5_ctx);
	u_printf("read_size:%d digest is ",total_size);
	u_printf("%02x%02x%02x%02x",digest[0],digest[1],digest[2],digest[3]);
	u_printf("%02x%02x%02x%02x",digest[4],digest[5],digest[6],digest[7]);
	u_printf("%02x%02x%02x%02x",digest[8],digest[9],digest[10],digest[11]);
	u_printf("%02x%02x%02x%02x\n",digest[12],digest[13],digest[14],digest[15]);
	
	if(hfupdate_complete(upg_type,total_size)!=HF_SUCCESS)
	{
		u_printf("update software fail\n");
	}
exit:
	if(upg_timer!=NULL)
	{
		hftimer_delete(upg_timer);
		hftimer_delete(upg_timer);
	}
	if(temp_buf!=NULL)	
		hfmem_free(temp_buf);
	if(content_data!=NULL)
		hfmem_free(content_data);
	if(hhttp!=0)
		hfhttp_close_session(&hhttp);
	hfgpio_fset_out_low(HFGPIO_F_NREADY);
	hfsys_enable_all_soft_watchdogs();
	return rv;
}

void USER_FUNC user_upgrade(void)
{
	int result=0;
	
	hfgpio_configure_fpin(HFGPIO_F_UPGRADE_GPIO,HFM_IO_TYPE_INPUT|HFPIO_DEFAULT);
	msleep(10);
	
	if(hfgpio_fpin_is_high(HFGPIO_F_UPGRADE_GPIO)==0)
	{
		result = hfupdate_auto_upgrade(0x20000000);
	}
	else
	{
		result = hfupdate_auto_upgrade(0);
	}
	
	if(result<0)
	{
		u_printf("no need to upgrade\n");
		return ;
	}
	else if(result==0)
	{
		u_printf("upgrade success!\n");
		while(1)
		{
			hfgpio_fset_out_high(HFGPIO_F_UPGRADE_LED);
			msleep(200);
			hfgpio_fset_out_low(HFGPIO_F_UPGRADE_LED);
			msleep(200);
		}
	}
	else 
	{
		u_printf("upgrade fail %d\n",result);
		while(1)
		{
			hfgpio_fset_out_low(HFGPIO_F_UPGRADE_LED);
			msleep(1000);
		}
	}
}

int USER_FUNC app_main (void)
{
	time_t now=time(NULL);
	
	HF_Debug(DEBUG_LEVEL,"sdk version(%s),the app_main start time is %d %s[AT DEMO]\n",hfsys_get_sdk_version(),now,ctime(&now));
	if(hfgpio_fmap_check()!=0)
	{
		while(1)
		{
			HF_Debug(DEBUG_ERROR,"gpio map file error\n");
			msleep(1000);
		}
	}
	while(!hfnet_wifi_is_active())
	{
		msleep(50);
	}
	user_upgrade();	
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

	
	return 1;
	
}

#endif
