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


#if (EXAMPLE_USE_DEMO==USER_IR_DEMO)

#define HFGPIO_F_IRTRNSMITTER			HFGPIO_F_USER_DEFINE
#define HFGPIO_F_IR_KEY0				(HFGPIO_F_USER_DEFINE+1)
#define HFGPIO_F_IR_LED					(HFGPIO_F_USER_DEFINE+2)

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
	HFM_NOPIN,//HF_M_PIN(7),	//HFGPIO_F_SLEEP_RQ
	HF_M_PIN(8),	//HFGPIO_F_SLEEP_ON

	HFM_NOPIN,		//HFGPIO_F_WPS
	HF_M_PIN(15),		//HFGPIO_F_IR
	HFM_NOPIN,		//HFGPIO_F_RESERVE2
	HFM_NOPIN,		//HFGPIO_F_RESERVE3
	HFM_NOPIN,		//HFGPIO_F_RESERVE4
	HFM_NOPIN,		//HFGPIO_F_RESERVE5
	HF_M_PIN(11),	//HFGPIO_F_USER_DEFINE
	HF_M_PIN(7),     // HFGPIO_F_IR_KEY0
	HF_M_PIN(13),
};

static int ir_recv_counter=0;
static int ir_recv_timer_stop=1;
static hftimer_handle_t test_timer_hardware=NULL;
static int record_save_to_file=0;

#define RECORD_FILE_HDR_SIZE		(16)
static char RECORD_FILE_HDR[RECORD_FILE_HDR_SIZE]="ir-records-file";
#define ONE_KEY_RECORD_SIZE		(512)
#define MAX_IR_KEY					(6)

typedef struct _IR_RECV_RECORD
{
	//uint32_t recv_time;
	//uint8_t   io_state;
	uint32_t  data;
}IR_RECV_RECORD,*PIR_RECV_RECORD;

#define RECORD_SET_IO_STATE_HI(__p_record)	(__p_record).data |= 0x80000000
#define RECORD_SET_IO_STATE_LO(__p_record)	(__p_record).data &= 0x7FFFFFFF
#define RECORD_GET_IO_STATE(__p_record)	(((__p_record).data>>31)&0x01)
#define RECORD_GET_RECV_TIME(__p_record)	((__p_record).data&0x7FFFFFFF)
#define RECORD_SET_RECV_TIME(__p_record,_recv_time)   ((__p_record).data= ((__p_record).data&0x80000000)|(_recv_time&0x7FFFFFFF))


static void USER_FUNC ir_transfer_key(PIR_RECV_RECORD p_key_records,int cnt);
static int hf_atcmd_irsnd(pat_session_t s,int argc,char *argv[],char *rsp,int len);
static int ir_press_key_id =-1;

const hfat_cmd_t user_define_at_cmds_table[]=
{
	{"IRSND",hf_atcmd_irsnd,"   AT+IRSND=code\r\n",NULL},	
	{NULL,NULL,NULL,NULL} //the last item must be null
};

struct _IR_RECV_RECORD   ir_recv_record[200]={0};

void USER_FUNC save_key_record_to_file(PIR_RECV_RECORD p_record,int cnt)
{
	int key_cnt=0;
	int offset;

	if(!record_save_to_file)
		return;
	
	if(cnt<16)
		return;
	
	hffile_userbin_read(RECORD_FILE_HDR_SIZE,(char*)&key_cnt,4);
	if(key_cnt>=MAX_IR_KEY)
	{
		u_printf("record file is full!\n");
		return;
	}

	if(cnt*sizeof(IR_RECV_RECORD)>ONE_KEY_RECORD_SIZE)
		return;
	
	offset=RECORD_FILE_HDR_SIZE+4+key_cnt*ONE_KEY_RECORD_SIZE;
	hffile_userbin_write(offset,(char*)&cnt,4);
	hffile_userbin_write(offset+4,(char*)p_record,cnt*sizeof(IR_RECV_RECORD));
	key_cnt++;
	hffile_userbin_write(RECORD_FILE_HDR_SIZE,(char*)&key_cnt,4);
	ir_press_key_id = key_cnt-1;
	
	return;
}

int USER_FUNC read_key_record_from_file(int key_id,PIR_RECV_RECORD p_record,int size)
{
	int key_cnt=0;
	int records_cnt=0;
	int offset=RECORD_FILE_HDR_SIZE+4+key_id*ONE_KEY_RECORD_SIZE;

	hffile_userbin_read(RECORD_FILE_HDR_SIZE,(char*)&key_cnt,4);
	if(key_id>=key_cnt)
		return -1;
	
	hffile_userbin_read(offset,(char*)&records_cnt,4);
	hffile_userbin_read(offset+4, (char*)p_record, size*sizeof(IR_RECV_RECORD));

	return records_cnt;
}

static void dump_ir_key_record(PIR_RECV_RECORD p_key_records,int cnt)
{
	int i;
	
	for(i=0;i<cnt;i++)
	{
		u_printf("[%d]-->%d %u\n",i,RECORD_GET_IO_STATE(p_key_records[i]),RECORD_GET_RECV_TIME(p_key_records[i]));
	}

	return;
}

static IR_RECV_RECORD ir_key_records[200]={0};


static int hf_atcmd_irsnd(pat_session_t s,int argc,char *argv[],char *rsp,int len)
{
	int key_id=0;
	int code=0;
	int key_records_cnt=0;
	
	if(argc<2)
		return -1;

	code = atoi(argv[0]);
	key_id = atoi(argv[1]);
	
	if(code==3)
	{
		int i;
		key_records_cnt=100;
		for(i=0;i<100;i++)
		{
			RECORD_SET_RECV_TIME(ir_key_records[i],i*1000);
			if(i%2)
				RECORD_SET_IO_STATE_HI(ir_key_records[i]);
			else
				RECORD_SET_IO_STATE_LO(ir_key_records[i]);
		}
		ir_transfer_key(ir_key_records,key_records_cnt);
		return 0;
	}
	else if(code==2)
	{
		int key_cnt=0;
		record_save_to_file=1;
		hffile_userbin_write(RECORD_FILE_HDR_SIZE,(char*)&key_cnt,4);
		return 0;
	}
	
	if((key_records_cnt=read_key_record_from_file(key_id,ir_key_records,200))<=0)
	{
		return -1;
	}

	if(code==0)
	{
		dump_ir_key_record(ir_key_records,key_records_cnt);
	}
	else if(code==1)
	{
		ir_transfer_key(ir_key_records,key_records_cnt);
	}
	
	return 0;
		
}

void USER_FUNC test_timer_callback( hftimer_handle_t htimer )
{	
	ir_recv_timer_stop = 1;
	hftimer_stop(test_timer_hardware);
	
	if(ir_recv_counter>0)
	{
		dump_ir_key_record(ir_recv_record,ir_recv_counter);
		save_key_record_to_file(ir_recv_record,ir_recv_counter);
	}
	ir_recv_counter=0;	
}

static void USER_FUNC do_user_ir(uint32_t arg1,uint32_t arg2)
{
	time_t now=time(NULL);
	uint32_t recv_time;
	
	if(ir_recv_timer_stop)
	{
		hftimer_start(test_timer_hardware);
		ir_recv_timer_stop=0;
	}

	recv_time = hftimer_get_counter(test_timer_hardware);
	if(hfgpio_fpin_is_high(HFGPIO_F_IR)) 
	{
		//ir_recv_record[ir_recv_counter].recv_time=hftimer_get_counter(test_timer_hardware);
		//ir_recv_record[ir_recv_counter].io_state=1;
		RECORD_SET_IO_STATE_HI(ir_recv_record[ir_recv_counter]);
		if(hfir_is_key_come())//ENC
		{
			u_printf("key code=%08X\n",hfir_get_key_code());
		}
		//hfgpio_fset_out_high(HFGPIO_F_IR_LED);
	}
	else
	{
		//ir_recv_record[ir_recv_counter].recv_time=hftimer_get_counter(test_timer_hardware);
		//ir_recv_record[ir_recv_counter].io_state=0;
		RECORD_SET_IO_STATE_LO(ir_recv_record[ir_recv_counter]);
		//hfgpio_fset_out_low(HFGPIO_F_IR_LED);
	}
	
	RECORD_SET_RECV_TIME(ir_recv_record[ir_recv_counter],recv_time);
	
	ir_recv_counter ++;	
	
}

static void ir_transfer_key_by_id(int key_id)
{
	int key_cnt=0;
	int key_records_cnt=0;

	hffile_userbin_read(RECORD_FILE_HDR_SIZE,(char*)&key_cnt,4);
	if(key_id>=key_cnt)
	{
		return;
	}
	
	if((key_records_cnt=read_key_record_from_file(key_id,ir_key_records,200))<=0)
	{
		return ;
	}
	ir_transfer_key(ir_key_records,key_records_cnt);

	return;
}

	
static void USER_FUNC do_press_irkey0(uint32_t arg1,uint32_t arg2)
{
	int key_cnt=0;
	static int key_id=0;

	if(ir_press_key_id!=-1)
		return;
	
	hffile_userbin_read(RECORD_FILE_HDR_SIZE,(char*)&key_cnt,4);
	if(key_id>=key_cnt)
	{
		key_id = 0;
	}
	
	ir_press_key_id=key_id;
	
	key_id++;

	return;
		
}


static void ir_start_38K(void)
{
	hfgpio_pwm_enable(	HFGPIO_F_IRTRNSMITTER,40000,50);
}

static void ir_stop_38K(void)
{
	hfgpio_pwm_disable(HFGPIO_F_IRTRNSMITTER);
	hfgpio_fset_out_high(HFGPIO_F_IRTRNSMITTER);
}

static void USER_FUNC ir_transfer_key(PIR_RECV_RECORD p_key_records,int cnt)
{
	int i;
	uint32_t recv_time;
	uint32_t io_state;

	hfgpio_fdisable_interrupt(HFGPIO_F_IR);
	hfthread_suspend_all();
	ir_stop_38K();
	hftimer_stop(test_timer_hardware);
	hftimer_start(test_timer_hardware);
	
	for(i=0;i<cnt;i++)
	{
		recv_time = RECORD_GET_RECV_TIME(p_key_records[i]);
		io_state = RECORD_GET_IO_STATE(p_key_records[i]);
		
		if(io_state==0)
		{
			while(hftimer_get_counter(test_timer_hardware)<recv_time);
			ir_start_38K();
		}
		else
		{
			while(hftimer_get_counter(test_timer_hardware)<recv_time);
			ir_stop_38K();
		}
	}
	ir_stop_38K();
	hfthread_resume_all();
	hfgpio_fenable_interrupt(HFGPIO_F_IR);

	return;
}
	

void init_ir_key_records_file(void)
{
	char hdr[RECORD_FILE_HDR_SIZE]={0};

	hffile_userbin_read(0,hdr,RECORD_FILE_HDR_SIZE);
	if(memcmp(hdr,RECORD_FILE_HDR,sizeof(RECORD_FILE_HDR))!=0)
	{
		hffile_userbin_zero();
		hffile_userbin_write(0, RECORD_FILE_HDR,RECORD_FILE_HDR_SIZE);
	}

	return;
}

int USER_FUNC app_main (void)
{
	time_t now=time(NULL);
	
	HF_Debug(DEBUG_LEVEL,"[IR DEMO]sdk version(%s),the app_main start time is %d %s\n",hfsys_get_sdk_version(),now,ctime(&now));
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
	hfir_ignore_lead_header(true);

	init_ir_key_records_file();

	if(hfgpio_configure_fpin_interrupt(HFGPIO_F_IR_KEY0,HFPIO_IT_FALL_EDGE,do_press_irkey0,1)!=HF_SUCCESS)
	{
		u_printf("configure HFGPIO_F_IR_KEY0 fail\n");
		return 0;
	}
#if 1	
	if(hfgpio_configure_fpin_interrupt(HFGPIO_F_IR,HFPIO_IT_EDGE,do_user_ir,1)!=HF_SUCCESS)
	{
		u_printf("configure HFGPIO_F_IR fail\n");
		return 0;
	}
	
#else	
	while(1)
	{
		if(hfir_is_key_come())
		{
			u_printf("key code=%08X\n",hfir_get_key_code());
		}
		else
			msleep(100);
	}
#endif

	if((test_timer_hardware=hftimer_create("HDW-TIMER",110000,true,3,test_timer_callback,HFTIMER_FLAG_HARDWARE_TIMER))==NULL)
	{
		u_printf("create  hardware timer fail\n");
	}

	while(1)
	{
		if(ir_press_key_id>=0)
		{
			u_printf("press ir_key_id=%d\n",ir_press_key_id);
			ir_transfer_key_by_id(ir_press_key_id);
		}
		ir_press_key_id=-1;
		msleep(100);
	}
	return 1;
	
}

#endif
