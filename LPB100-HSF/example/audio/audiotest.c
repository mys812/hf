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

#ifdef __LPB100U__

#if (EXAMPLE_USE_DEMO==AUDIO_OUT_DEMO)

#define AUDIO_FILE_FLASH_OFFSET		(WEB_ADDRESS)
#define AUDIO_FILE_MAX_SIZE			(1024*1024)

#define HFGPIO_F_PLAY_AUDIO_KEY		HFGPIO_F_USER_DEFINE

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
	
	HFM_NOPIN,	//HFGPIO_F_SPI_MISO
	HFM_NOPIN,	//HFGPIO_F_SPI_CLK
	HFM_NOPIN,	//HFGPIO_F_SPI_CS
	HFM_NOPIN,	//HFGPIO_F_SPI_MOSI
	
	HF_M_PIN(29),	//HFGPIO_F_UART1_TX,
	HFM_NOPIN,	//HFGPIO_F_UART1_RTS,
	HF_M_PIN(30),	//HFGPIO_F_UART1_RX,
	HFM_NOPIN,	//HFGPIO_F_UART1_CTS,
	
	HF_M_PIN(43),	//HFGPIO_F_NLINK
	HFM_NOPIN,//HF_M_PIN(44),	//HFGPIO_F_NREADY
	HFM_NOPIN,//HF_M_PIN(45),	//HFGPIO_F_NRELOAD
	HFM_NOPIN,//HF_M_PIN(7),	//HFGPIO_F_SLEEP_RQ
	HFM_NOPIN,//HF_M_PIN(8),	//HFGPIO_F_SLEEP_ON

	HFM_NOPIN,		//HFGPIO_F_RESERVE0
	HFM_NOPIN,		//HFGPIO_F_RESERVE1
	HFM_NOPIN,		//HFGPIO_F_RESERVE2
	HFM_NOPIN,		//HFGPIO_F_RESERVE3
	HFM_NOPIN,		//HFGPIO_F_RESERVE4
	HFM_NOPIN,		//HFGPIO_F_RESERVE5
	
	HF_M_PIN(45),	//HFGPIO_F_PLAY_AUDIO_KEY
	HF_M_PIN(7),	//HFGPIO_F_UPGRADE_GPIO

};

#define MAX_AUDIO_FILE_NUMBER			(10)

typedef struct AUDIO_FILE
{
	uint32_t size;
	uint32_t start_addr;
}AUDIO_FILE;

AUDIO_FILE  audio_files[MAX_AUDIO_FILE_NUMBER]={0};

static int play_audio_file=-1;
static uint32_t audio_item_number=0;
static int USER_FUNC hf_atcmd_download_audio_file(pat_session_t s,int argc,char *argv[],char *rsp,int len);
static int USER_FUNC download_audio_file(char *purl);

const hfat_cmd_t user_define_at_cmds_table[]=
{
	{"DLAUDIOFILE",hf_atcmd_download_audio_file,"   AT+DLAUDIOFILE=url\r\n",NULL},
	{NULL,NULL,NULL,NULL} //the last item must be null
};


static int USER_FUNC hf_atcmd_download_audio_file(pat_session_t s,int argc,char *argv[],char *rsp,int len)
{	
	if(argc<1)
	{
		return -1;
	}
	
	if(argc==0)
		return -1;
	else
	{
		download_audio_file(argv[0]);
	}
	
	return 0;
}

static int USER_FUNC uart_recv_callback(uint32_t event,char *data,uint32_t len,uint32_t buf_len)
{
	return len;
}

void app_init(void)
{
	u_printf("ex app_init \n");
	hfaudio_enable();
}

static int parse_audio_image_file(void)
{
	uint8_t *p_data = (uint8_t*)AUDIO_FILE_FLASH_OFFSET;
	int i;
	
	if(memcmp(p_data,"Audio Image File System",sizeof("Audio Image File System")-1)!=0)
	{
		u_printf("invalid audio image file!\n");
		return -1;
	}

	audio_item_number = (p_data[32]<<24)|(p_data[33]<<16)|(p_data[34]<<8)|(p_data[35]);

	for(i=0;i<audio_item_number ;i++)
	{
		audio_files[i].start_addr= (p_data[64+i*32]<<24)|(p_data[65+i*32]<<16)|(p_data[66+i*32]<<8)|(p_data[67+i*32]);
		audio_files[i].start_addr += AUDIO_FILE_FLASH_OFFSET;
		audio_files[i].size = (p_data[68+i*32]<<24)|(p_data[69+i*32]<<16)|(p_data[70+i*32]<<8)|(p_data[71+i*32]);
	}
	
	return 0;
}

static int USER_FUNC download_audio_file(char *purl)
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
	flash_erase_page(AUDIO_FILE_FLASH_OFFSET,AUDIO_FILE_MAX_SIZE/HFFLASH_PAGE_SIZE);
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
	while((read_size=hfhttp_read_content(hhttp,content_data,256))>0)
	{
		hfgpio_fset_out_high(HFGPIO_F_NREADY);
		flash_write(AUDIO_FILE_FLASH_OFFSET+total_size,content_data,read_size,0);
		total_size+=read_size;
		hfgpio_fset_out_low(HFGPIO_F_NREADY);
		u_printf("download file:[%d] [%d]\r",total_size,read_size);
	}
	hfgpio_fset_out_low(HFGPIO_F_NREADY);
exit:
	hfsys_enable_all_soft_watchdogs();
	if(temp_buf!=NULL)	
		hfmem_free(temp_buf);
	if(content_data!=NULL)
		hfmem_free(content_data);
	if(hhttp!=0)
		hfhttp_close_session(&hhttp);
	hfgpio_fset_out_low(HFGPIO_F_NREADY);
	return rv;
}

//char tmp[1024];
void *audio_thread_func( void * arg)
{
	//int audio_file_size;
		
	while(1)
	{
		if(play_audio_file<0)
		{
			play_audio_file=-1;
			msleep(100);
			continue;
		}
		hfaudio_pcm_play_init(22050,1);
		//flash_read(AUDIO_FILE_FLASH_OFFSET,(char*)&audio_file_size,4,0);
		//if(audio_file_size>AUDIO_FILE_MAX_SIZE||audio_file_size<=0)
		//{
		//	u_printf("invalid audio file %d\n",audio_file_size);
		//	continue;
		//}
		//offset=1024;
		u_printf("play audio file %d\n",play_audio_file);
		hfsys_disable_all_soft_watchdogs();
		hfthread_suspend_all();
		//while(audio_file_size>0)
		//{
		//	audio_file_size-=1024;
		//	flash_read(AUDIO_FILE_FLASH_OFFSET+offset,(char*)tmp,1024,0);
		//	offset+=1024;
		//	hfaudio_pcm_play((uint32_t)tmp, 1024);
		//}
		hfaudio_pcm_play(audio_files[play_audio_file].start_addr, audio_files[play_audio_file].size);
		hfthread_resume_all();
		hfsys_enable_all_soft_watchdogs();
		play_audio_file=-1;
	}
	
	return NULL;
}

static void USER_FUNC play_audio_key_press(uint32_t arg1,uint32_t arg2)
{
	time_t now=time(NULL);
	static int play_index=0;
	if(audio_item_number<=0)
	{
		u_printf("no audio file!\n");
		return;
	}
	//LPB开发板RELOD脚上拉，如果为高电平说明释放按键
	if(hfgpio_fpin_is_high(HFGPIO_F_PLAY_AUDIO_KEY))
	{
		u_printf("release the reload button!%d\n",now);
		play_index = play_index%audio_item_number;
		play_audio_file = play_index;
		play_index++;
	}
	else
	{
		u_printf("press the reload button!%d\n",now);
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
	if(hfnet_start_assis(ASSIS_PORT)!=HF_SUCCESS)
	{
		HF_Debug(DEBUG_WARN,"start httpd fail\n");
	}	
	if(hfnet_start_uart(HFTHREAD_PRIORITIES_LOW,(hfnet_callback_t)uart_recv_callback)!=HF_SUCCESS)
	{
		HF_Debug(DEBUG_WARN,"start uart fail!\n");
	}
	parse_audio_image_file();
	if(hfgpio_configure_fpin_interrupt(HFGPIO_F_PLAY_AUDIO_KEY,HFPIO_IT_EDGE,play_audio_key_press,1)!=HF_SUCCESS)
	{
		u_printf("configure HFGPIO_F_PLAY_AUDIO_KEY fail\n");
		return 0;
	}	
	
	hfthread_create(audio_thread_func,"audio_output",256,(void*)1,1,NULL,NULL);
	
	return 1;
}

#endif

#endif
