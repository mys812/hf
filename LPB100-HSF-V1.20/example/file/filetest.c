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
#include <hfir.h>
#include <stdlib.h>
#include <string.h>
//#include <malloc.h>
#include <stdio.h>
#include <httpc/httpc.h>
#include "../example.h"

#if (EXAMPLE_USE_DEMO==USER_FILE_DEMO)

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

	HFM_NOPIN,		//HFGPIO_F_WPS
	HF_M_PIN(15),		//HFGPIO_F_IR
	HFM_NOPIN,		//HFGPIO_F_RESERVE2
	HFM_NOPIN,		//HFGPIO_F_RESERVE3
	HFM_NOPIN,		//HFGPIO_F_RESERVE4
	HFM_NOPIN,		//HFGPIO_F_RESERVE5
	
	HFM_NOPIN,	//HFGPIO_F_USER_DEFINE
};
static int USER_FUNC hf_atcmd_filetest(pat_session_t s,int argc,char *argv[],char *rsp,int len);
const hfat_cmd_t user_define_at_cmds_table[]=
{
	{"FTEST",hf_atcmd_filetest,"   AT+FTEST=code,offset,value\r\n",NULL},
	{NULL,NULL,NULL,NULL} //the last item must be null
};

#define PRINTF(...)	HF_Debug(DEBUG_LEVEL,__VA_ARGS__)

static int USER_FUNC hf_atcmd_filetest(pat_session_t s,int argc,char *argv[],char *rsp,int len)
{
	int code,offset,rlen,i;
	uint32_t temp;
	char cc;
	uint32_t file_size;
		
	if(argc!=3)
		return -1;
	
	code = atoi(argv[0]);
	offset = atoi(argv[1]);
	file_size = hffile_userbin_size();
	if(code==0)
	{
		sprintf(rsp,"=%d",file_size);
	}
	else if(code==1||code==5)
	{	
		rlen = atoi(argv[2]);
		for(i=0;i<rlen;i++)
		{
			hffile_userbin_read(offset+i,&cc,1);
			if(code==1)
				u_printf("%c",cc);
			else
				u_printf("%02X",(uint8_t)cc);
		}
	}
	else if(code==2)
	{
		hffile_userbin_write(offset,argv[2],strlen(argv[2]));
	}
	else if(code==3)
	{
		u_printf("userbin file size=%d\n",file_size);
		for(i=0;i<file_size;)
		{
			hffile_userbin_write(i,(char*)&i,sizeof(i));
			i+=sizeof(i);
		}
		for(i=0;i<file_size;)
		{
			hffile_userbin_read(i,(char*)&temp,sizeof(temp));
			//u_printf("file[%d]=%08x\n",i,temp);
			if(temp!=i)
			{
				u_printf("test fail\n");
			}
			i+=sizeof(temp);
		}
	}
	else if(code==4)
	{
		hffile_userbin_zero();
	}
	
	return 0;
}

#define CFG_HDR_FILE_OFFSET			0
#define CFG_BRMID_FILE_OFFSET		32
#define CFG_BRMADDR_FILE_OFFSET		64
#define CFG_BRMPORT_FILE_OFFSET		96

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


int hfhttpd_user_nvset( char * cfg_name,int name_len,char* value,int val_len)
{
	char temp[20];
		
	bzero(temp,sizeof(temp));
	if(val_len>=20)
	{
		return 0;
	}
	memcpy(temp,value,val_len);
	
	if(strnstr(cfg_name,"CFG_BRMID",name_len)!=NULL)
	{
		hffile_userbin_write(CFG_BRMID_FILE_OFFSET,temp,sizeof(temp));
		return 0;
	}
	else if(strnstr(cfg_name,"CFG_BRMADDR",name_len)!=NULL)
	{
		hffile_userbin_write(CFG_BRMADDR_FILE_OFFSET,temp,sizeof(temp));
		return 0;
	}
	else if(strnstr(cfg_name,"CFG_BRMPORT",name_len)!=NULL)
	{
		hffile_userbin_write(CFG_BRMPORT_FILE_OFFSET,temp,sizeof(temp));
		return 0;
	}
	
	return -1;
}

int hfhttpd_user_nvget( char *cfg_name,int name_len,char *value,int val_len)
{
	char temp[20];
	
	bzero(temp,sizeof(temp));
	if(strnstr(cfg_name,"CFG_BRMID",name_len)!=NULL)
	{
		hffile_userbin_read(CFG_BRMID_FILE_OFFSET,temp,19);
		strcpy(value,temp);
		return 0;
	}
	else if(strnstr(cfg_name,"CFG_BRMADDR",name_len)!=NULL)
	{
		hffile_userbin_read(CFG_BRMADDR_FILE_OFFSET,temp,19);
		strcpy(value,temp);
		return 0;
	}
	else if(strnstr(cfg_name,"CFG_BRMPORT",name_len)!=NULL)
	{
		hffile_userbin_read(CFG_BRMPORT_FILE_OFFSET,temp,19);
		strcpy(value,temp);
		return 0;
	}
	
	return -1;
}

USER_FUNC void test_file_thread_start(void)
{
	uint8_t hdr[8]={0};


	if(hffile_userbin_read(CFG_HDR_FILE_OFFSET,(char*)hdr,8)>0)
	{
		if(hdr[0]!=0x00||hdr[1]!=0x01)
		{
			bzero(hdr,8);
			hdr[0]=0x00;
			hdr[1]=0x01;
			hffile_userbin_zero();
			hffile_userbin_write(CFG_HDR_FILE_OFFSET,(char*)hdr,8);
		}
	}
	
	hfnet_httpd_set_get_nvram_callback(hfhttpd_user_nvset,hfhttpd_user_nvget);
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
	
	//if(hfnet_start_httpd(HFTHREAD_PRIORITIES_MID)!=HF_SUCCESS)
	//{
	//	HF_Debug(DEBUG_WARN,"start httpd fail\n");
	//}
	if(hfnet_start_uart(HFTHREAD_PRIORITIES_LOW,NULL)!=HF_SUCCESS)
	{
		HF_Debug(DEBUG_WARN,"start uart fail!\n");
	}
	if(hfnet_start_socketa(HFTHREAD_PRIORITIES_LOW,(hfnet_callback_t)NULL)!=HF_SUCCESS)
	{
		HF_Debug(DEBUG_WARN,"start socketa fail\n");
	}
	//if(hfnet_start_socketb(HFTHREAD_PRIORITIES_LOW,(hfnet_callback_t)NULL)!=HF_SUCCESS)
	//{
	//	HF_Debug(DEBUG_WARN,"start socketb fail\n");
	//}
	
	test_file_thread_start();
	
	return 1;
	
}

#endif
