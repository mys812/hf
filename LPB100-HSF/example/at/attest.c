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

#if (EXAMPLE_USE_DEMO==USER_AT_CMD_DEMO)

#define HFGPIO_F_ADC_CHANNEL1		HFGPIO_F_USER_DEFINE
#define HFGPIO_F_ADC_CHANNEL2		(HFGPIO_F_USER_DEFINE+1)
#define HFGPIO_F_ADC_CHANNEL3		(HFGPIO_F_USER_DEFINE+2)

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
	
	HF_M_PIN(11),	//HFGPIO_F_ADC_CHANNEL1
	HF_M_PIN(12),	//HFGPIO_F_ADC_CHANNEL2
	HF_M_PIN(23), //HFGPIO_F_ADC_CHANNEL3
};

static int hf_atcmd_myatcmd(pat_session_t s,int argc,char *argv[],char *rsp,int len);
static int hf_atcmd_adctest(pat_session_t s,int argc,char *argv[],char *rsp,int len);

static int USER_FUNC test_httpc_get(char *purl);
static int USER_FUNC test_httpc_post(char *purl);

const hfat_cmd_t user_define_at_cmds_table[]=
{
	{"UMYATCMD",hf_atcmd_myatcmd,"   AT+UMYATCMD=code\r\n",NULL},
	{"ADCTEST",hf_atcmd_adctest,"   AT+ADCTEST=code,channel\r\n",NULL},	
	{NULL,NULL,NULL,NULL} //the last item must be null
};

static uint32_t adc_fid =0;

static int hf_atcmd_adctest(pat_session_t s,int argc,char *argv[],char *rsp,int len)
{
	int code=0;
	uint16_t value;
	
	if(argc<2)
		return -1;

	code = atoi(argv[0]);
	adc_fid = atoi(argv[1]);

	switch(code)
	{
		case 0:
			hfgpio_adc_enable(adc_fid);
			return 0;
		case 1:
			value = hfgpio_adc_get_value(adc_fid);
			break;
		case 2:
			value = hfgpio_adc_get_voltage(adc_fid);
			break;
		default:
			return -1;
	}

	sprintf(rsp,"=%d",value);
	
	return 0;
	
}


static int USER_FUNC hf_atcmd_myatcmd(pat_session_t s,int argc,char *argv[],char *rsp,int len)
{
	static int test_code=0;
	
	if(argc<2)
	{
		return -1;
	}
	
	if(argc==0)
		sprintf(rsp,"=%d",test_code);
	else
	{
		test_code=atoi(argv[0]);
	}
	switch(test_code)
	{
		case 1:
			test_httpc_get(argv[1]);
			break;
		case 2:
			test_httpc_post(argv[1]);
			break;
		default:
			break;
	}
	
	return 0;
}

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

void USER_FUNC update_timer_callback( hftimer_handle_t htimer )
{
	if(hfgpio_fpin_is_high(HFGPIO_F_NREADY))
		hfgpio_fset_out_low(HFGPIO_F_NREADY);
	else
		hfgpio_fset_out_high(HFGPIO_F_NREADY);
}

static int USER_FUNC test_httpc_get(char *purl)
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
	
	bzero(&http_req,sizeof(http_req));
	http_req.type = HTTP_GET;
	http_req.version=HTTP_VER_1_1;
	
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
	hfupdate_start(HFUPDATE_SW);
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
		hfupdate_write_file(HFUPDATE_SW, total_size,content_data, read_size);
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
	
	if(hfupdate_complete(HFUPDATE_SW,total_size)!=HF_SUCCESS)
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

static int USER_FUNC test_httpc_post(char *purl)
{
	httpc_req_t  http_req;
	char content_data[34];
	char *temp_buf=NULL;
	parsed_url_t url={0};
	http_session_t hhttp=0;
	int total_size,read_size=0;
	int rv=0;
	tls_init_config_t  *tls_cfg=NULL;
	char *test_url=purl;
	
	bzero(&http_req,sizeof(http_req));
	http_req.type = HTTP_POST;
	http_req.version=HTTP_VER_1_1;
	
	if((temp_buf = (char*)hfmem_malloc(256))==NULL)
	{
		u_printf("no memory\n");
		return -HF_E_NOMEM;
	}
	
	bzero(temp_buf,sizeof(temp_buf));
	if((rv=hfhttp_parse_URL(test_url,temp_buf , 256, &url))!=HF_SUCCESS)
	{
		hfmem_free(temp_buf);
		return rv;
	}
	
	if((rv=hfhttp_open_session(&hhttp,test_url,0,tls_cfg,3))!=HF_SUCCESS)
	{
		u_printf("http open fail\n");
		hfmem_free(temp_buf);
		return rv;
	}
	
	http_req.resource = url.resource;
	http_req.content="POST TEST DATA\r\n";
	http_req.content_len = strlen(http_req.content);
	http_prepare_req(hhttp,&http_req,HDR_ADD_CONN_CLOSE);
	if((rv=hfhttp_send_request(hhttp,&http_req))!=HF_SUCCESS)
	{
		u_printf("http send request fail\n");
		hfmem_free(temp_buf);
		http_close_session(&hhttp);
		return rv;
	}
	total_size=0;
	bzero(content_data,sizeof(content_data));
	while((read_size=hfhttp_read_content(hhttp,content_data,32))>0)
	{
		total_size+=read_size;
		u_printf("%s",content_data);
	}
	
	u_printf("read_size:%d\n",total_size);
	hfmem_free(temp_buf);
	hfhttp_close_session(&hhttp);
	return total_size;
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
	
	{
		char *words[6]={NULL};
		char rsp[64]={0};
		hfat_send_cmd("AT+WANN\r\n",sizeof("AT+WANN\r\n"),rsp,64);
		if(hfat_get_words(rsp,words, 6)>0)
		{
			u_printf("\nresult:%s\nmode:%s\nIP:%s\nMASK:%s\nGW:%s\n",words[0],words[1],words[2],words[3],words[4]);
		}
	}
	adc_fid = HFGPIO_F_ADC_CHANNEL1;
	hfgpio_adc_enable(adc_fid);
	while(1)
	{
		u_printf("[%u]%u %u\n",adc_fid,hfgpio_adc_get_value(adc_fid),hfgpio_adc_get_voltage(adc_fid));
		//u_printf("[7]%u %u\n",SarAdcChannelGetValue(7),SarAdcGetGpioVoltage(7));
		msleep(500);
	}
	
	return 1;
	
}

#endif
