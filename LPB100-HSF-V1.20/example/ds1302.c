/* SCLK SDA RST 需要自己定义gpio		**
** 				接口如下				**
** USER_FUNC unsigned long get_rtctime();		**
** USER_FUNC int set_rtctime(struct tm *timep);	**

*********************************************/

#include <stdio.h>
#include <hsf.h>
#include <hfsys.h>

#define SCLK 		HFGPIO_F_USER_DEFINE
#define SDA	 		(HFGPIO_F_USER_DEFINE + 1)
#define RST			(HFGPIO_F_USER_DEFINE + 2)

#define WRITE_SECOND	0x80
#define WRITE_MINUTE	0x82
#define WRITE_HOUR		0x84
#define WRITE_DAY		0x86
#define WRITE_MON		0x88
#define WRITE_WDAY		0x8a
#define WRITE_YEAR		0x8c

#define READ_SECOND		0x81
#define READ_MINUTE		0x83
#define READ_HOUR		0x85
#define READ_DAY		0x87
#define READ_MON		0x89
#define READ_WDAY		0x8b
#define READ_YEAR		0x8d

#define WRITE_PROTECT	0x8E


/* Interface */

USER_FUNC unsigned long get_rtctime();
USER_FUNC int set_rtctime(struct tm *timep);


static USER_FUNC  void delay_us(int us)
{
	int i;

	for(i=0; i<us; i++)
		NULL;
}

USER_FUNC static void Writebyte(unsigned char addr, unsigned char data)
{
	unsigned char i, tmp;

	hfthread_suspend_all();
	hfgpio_fset_out_low(RST);
	hfgpio_fset_out_low(SCLK);
	delay_us(1);
	hfgpio_fset_out_high(RST);

	for(i=0; i<8; i++)
	{
		hfgpio_fset_out_low(SCLK);
		delay_us(1);
		tmp = addr;
		if(tmp & 0x01)
			hfgpio_fset_out_high(SDA);
		else
			hfgpio_fset_out_low(SDA);
		addr >>= 1;
		hfgpio_fset_out_high(SCLK);
		delay_us(1);
	}

	for(i=0; i<8; i++)
	{
		hfgpio_fset_out_low(SCLK);
		delay_us(1);
		tmp = data;
		if(tmp & 0x01)
			hfgpio_fset_out_high(SDA);
		else
			hfgpio_fset_out_low(SDA);
		data >>= 1;
		hfgpio_fset_out_high(SCLK);
	}
	hfgpio_fset_out_low(RST);
	hfthread_resume_all();
}


USER_FUNC static unsigned char Readbyte(unsigned char addr)
{
	unsigned char i,tmp,dat1;
	unsigned char dat2[5] = {0};

	hfthread_suspend_all();
	hfgpio_fset_out_low(RST);
	hfgpio_fset_out_low(SCLK);
	delay_us(1);
	hfgpio_fset_out_high(RST);

	for(i=0; i<8; i++)
	{
		hfgpio_fset_out_low(SCLK);
		tmp = addr;
		if(tmp & 0x01)
			hfgpio_fset_out_high(SDA);
		else
			hfgpio_fset_out_low(SDA);
		addr >>= 1;
		delay_us(1);
		hfgpio_fset_out_high(SCLK);
		delay_us(1);
		
	}

	hfgpio_configure_fpin(SDA, HFM_IO_TYPE_INPUT);
	tmp = 0;
	for(i=0; i<8; i++)
	{
		tmp >>= 1;
		hfgpio_fset_out_low(SCLK);		
		if(hfgpio_fpin_is_high(SDA))
			tmp |= 0x80;
		hfgpio_fset_out_high(SCLK);
		delay_us(1);
	}
	hfgpio_fset_out_low(RST);

	sprintf(dat2, "%x", tmp);
	dat1 = atoi(dat2);
	hfthread_resume_all();
	return dat1;
}

unsigned long get_rtctime()
{
	time_t timep;
	struct tm *p;

	time(&timep);
	p = localtime(&timep);
	p->tm_sec = Readbyte(READ_SECOND);
	p->tm_min = Readbyte(READ_MINUTE);
	p->tm_hour = Readbyte(READ_HOUR);
	p->tm_mday = Readbyte(READ_DAY);
	p->tm_wday = Readbyte(READ_WDAY);
	p->tm_mon = Readbyte(READ_MON) - 1;
	p->tm_year = Readbyte(READ_YEAR) + 100;

	timep = mktime(p);

	return timep;
}
USER_FUNC void num2hex(struct tm *timep)
{
	timep->tm_sec = timep->tm_sec/10*16 + timep->tm_sec%10;
	timep->tm_min = timep->tm_min/10*16 + timep->tm_min%10;	
	timep->tm_hour = timep->tm_hour/10*16 + timep->tm_hour%10;
	timep->tm_mday = timep->tm_mday/10*16 + timep->tm_mday%10;
	timep->tm_wday = timep->tm_wday + 1;
	timep->tm_mon = timep->tm_mon/10*16 + timep->tm_mon%10 + 1;
	timep->tm_year -= 100;
	timep->tm_year = timep->tm_year/10*16 + timep->tm_year%10;
}

USER_FUNC int set_rtctime(struct tm *timep)
{
	num2hex(timep);
	Writebyte(WRITE_PROTECT, 0x00);
	Writebyte(WRITE_SECOND, timep->tm_sec);
	Writebyte(WRITE_MINUTE, timep->tm_min);
	Writebyte(WRITE_HOUR, timep->tm_hour);
	Writebyte(WRITE_DAY, timep->tm_mday);
	Writebyte(WRITE_WDAY, timep->tm_wday);
	Writebyte(WRITE_MON, timep->tm_mon);
	Writebyte(WRITE_YEAR, timep->tm_year);
	Writebyte(WRITE_PROTECT, 0x80);

	return 0;
}


#if 1
int hfat_get_time(pat_session_t s,int argc,char *argv[],char *rsp,int len)
{

	unsigned char sec;
	unsigned char min;
	unsigned char hour;
	unsigned char mday;
	unsigned char wday;
	unsigned char mon;
	unsigned char year;

	sec = Readbyte(READ_SECOND);
	min = Readbyte(READ_MINUTE);
	hour = Readbyte(READ_HOUR);
	mday = Readbyte(READ_DAY);
	wday = Readbyte(READ_WDAY);
	mon = Readbyte(READ_MON);
	year = Readbyte(READ_YEAR);

	sprintf(rsp, "=%d, 20%d-%d-%d %d:%d:%d", get_rtctime(), year, mon, mday, hour, min, sec);
	return 0;
}

int hfat_set_time(pat_session_t s,int argc,char *argv[],char *rsp,int len)
{
	struct tm *p;
	
	time_t timep;

	time(&timep);
	p = localtime(&timep);

	set_rtctime(p);
	return 0;
}
#endif
