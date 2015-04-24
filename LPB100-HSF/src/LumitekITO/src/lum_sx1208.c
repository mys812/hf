/*
******************************
*Company:Lumlink
*Data:2015-04-02
*Author:Meiyusong
******************************
*/

#include "../inc/lumitekConfig.h"

#ifdef CONFIG_LUMITEK_DEVICE
#include <hsf.h>
#include <string.h>
#include <stdio.h>

#ifdef SX1208_433M_SUPPORT
#include "../inc/itoCommon.h"
#include "../inc/lum_sx1208.h"
#include "../inc/asyncMessage.h"


static ORIGIN_WAVE_DATA* g_pWaveDataInfo;
static SEARCH_FREQ_INFO g_searchFreqData;
static STUDY_SOCKET_SAVE_DATA g_socketSaveData;
static ORIGIN_WAVE_DATA g_studyWaveData;


extern unsigned int GpioGetReg(unsigned char RegIndex);;
extern void GpioSetRegOneBit(unsigned char	RegIndex, unsigned int GpioIndex);
extern void GpioClrRegOneBit(unsigned char	RegIndex, unsigned int GpioIndex);


static void USER_FUNC lum_setSdo2InteruptStatus(BOOL enableIrq);
static void USER_FUNC lum_start433StudyTimer(U32 timerGap);
static void USER_FUNC lum_enterStudyMode(U32 freq);
static void USER_FUNC lum_sendReplyStudyMessage(void);



static const _SX1231_REG RegistersCfg[] =           // !!! User can reconfigure register based on application
{
//  {addr,         val}       // RegName

	{REG_OPMODE,        0x04},  //Standby mode (STDBY)
	{REG_DATAMODUL,     0x69},  //Continuous mode without bit synchronizer,OOK,filtering with fcutoff= 2*BR
	{REG_BITRATEMSB,    0X0D},//9600bps
	{REG_BITRATELSB,    0X05},
	{REG_FDEVMSB,       0x01},// 19KDEV
	{REG_FDEVLSB,       0x37},
	{REG_FRFMSB,        0X6C}, //433.9MHz
	{REG_FRFMID,        0X79},
	{REG_FRFLSB,        0X9A},
	{REG_OSC1,          0X41},
	{REG_AFCCTRL,       0x00},
	{REG_LOWBAT,        0x02},
	{REG_LISTEN1,       0X92},
	{REG_LISTEN2,       0XF5},
	{REG_LISTEN3,       0X20},
	{REG_PALEVEL,       0x9F},
	{REG_PARAMP ,       0X09},
	{REG_OCP,           0X1B},
	{REG_SERVED14,      0x40},
	{REG_SERVED15,      0xB0},
	{REG_SERVED16,      0x7B},
	{REG_SERVED17,      0x9B},
	{REG_LNA ,          0X89},
	{REG_RXBW,          0x42}, //   120K
	{REG_AFCBW,         0X91},
	{REG_OOKPEAK,       0X3F},
	{REG_OOKAVG,        0X00},
	{REG_OOKFIX,        0X50},
	{REG_AFCFEI,        0X10},
	{REG_DIOMAPPING1,   0X00},
	{REG_DIOMAPPING2,   0X07},
	{REG_RSSITHRESH ,   0XE8},
	{REG_RXTIMEOUT1 ,   0X00},
	{REG_RXTIMEOUT2 ,   0X00},
	{REG_PREAMBLEMSB,   0X00},
	{REG_PREAMBLELSB,   0X04},
	{REG_SYNCCONFIG,    0X9A},
	{REG_SYNCVALUE1,    0XD3},
	{REG_SYNCVALUE2,    0X91},
	{REG_SYNCVALUE3,    0XD3},
	{REG_SYNCVALUE4,    0X91},
	{REG_SYNCVALUE5,    0X00},
	{REG_SYNCVALUE6,    0X00},
	{REG_SYNCVALUE7,    0X00},
	{REG_SYNCVALUE8,    0X00},
	{REG_PACKETCONFIG1, 0X90},
	{REG_PAYLOADLENGTH, 66},            // max to 66 bytes
	{REG_NODEADRS,      0X00},
	{REG_BROADCASTADRS, 0X00},
	{REG_AUTOMODES,     0X00},
	{REG_FIFOTHRESH,    0XC2},
	{REG_PACKETCONFIG2, 0X02},
	{REG_AESKEY1,       0X00},
	{REG_AESKEY2,       0X00},
	{REG_AESKEY3,       0X00},
	{REG_AESKEY4,       0X00},
	{REG_AESKEY5,       0X00},
	{REG_AESKEY6,       0X00},
	{REG_AESKEY7,       0X00},
	{REG_AESKEY8,       0X00},
	{REG_AESKEY9,       0X00},
	{REG_AESKEY10,      0X00},
	{REG_AESKEY11,      0X00},
	{REG_AESKEY12,      0X00},
	{REG_AESKEY13,      0X00},
	{REG_AESKEY14,      0X00},
	{REG_AESKEY15,      0X00},
	{REG_AESKEY16,      0X00},
	{REG_TESTLNA,       0x2D},
	{REG_TESTPA1,       0x55},
	{REG_TESTPA2,       0x70},
	{REG_TESTDAGC,      0x00},
	{REG_TESTAFC,       0x00},

};


static void USER_FUNC lum_spiWriteData(U8 addr, U8 data)
{
	U8 buf[2];


	hfspi_cs_low();
	addr |= 0x80;
	buf[0] = addr;
	buf[1] = data;
	hfspi_master_send_data((S8*)(buf), 2);
	hfspi_cs_high();
}


static U8 USER_FUNC lum_spiReadData(U8 addr)
{
	U8 data;


	hfspi_cs_low();
	addr &= 0x7F;
	hfspi_master_send_data((S8*)(&addr), 1);
	hfspi_master_recv_data((S8*)(&data), 1);
	hfspi_cs_high();
	return data;
}


static void USER_FUNC lum_spiInit(void)
{
	hfspi_master_init(HFSPI_MODE_CPOL0_CPHA0, HFSPIM_CLK_DIV_1M5);
}


static void USER_FUNC lum_sx1208HwReset(void)
{
	hfgpio_fset_out_high(HFGPIO_F_RESET);
	msleep(15);
	hfgpio_fset_out_low(HFGPIO_F_RESET);
	msleep(15);
}

static void USER_FUNC lum_rfDIOsInit(void)
{
	// do nothing now
}

static U8 lum_setRfMode(unsigned char mode)
{
	static unsigned char pre_mode = RF_STANDBY;

	if(mode != pre_mode)
	{
		if(mode == RF_TRANSMITTER)
		{
			lum_spiWriteData(REG_OPMODE, (REG_OPMODE_PRESET & 0xE3) | RF_TRANSMITTER);
			//SpiWriteReg(REG_OPMODE,0x0c
			while ((lum_spiReadData(REG_IRQFLAGS1) & RF_IRQFLAGS1_TXREADY) == 0x00); // Wait for TxReady
			pre_mode = RF_TRANSMITTER;
		}

		else if(mode == RF_RECEIVER)
		{
			lum_spiWriteData(REG_OPMODE, (REG_OPMODE_PRESET & 0xE3) | RF_RECEIVER);
			//while ((SpiReadReg(REG_IRQFLAGS1) & RF_IRQFLAGS1_RXREADY) == 0x00); // Wait for RxReady
			pre_mode = RF_RECEIVER;
		}

		else if(mode == RF_SYNTHESIZER)
		{
			lum_spiWriteData(REG_OPMODE, (REG_OPMODE_PRESET & 0xE3) | RF_SYNTHESIZER);
			while ((lum_spiReadData(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00); // Wait for ModeReady
			pre_mode = RF_SYNTHESIZER;
		}

		else if(mode == RF_STANDBY)
		{
			lum_spiWriteData(REG_OPMODE, (REG_OPMODE_PRESET & 0xE3) | RF_STANDBY);
			while ((lum_spiReadData(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00); // Wait for ModeReady
			pre_mode = RF_STANDBY;
		}

		else  // mode == RF_SLEEP
		{
			lum_spiWriteData(REG_OPMODE, (REG_OPMODE_PRESET & 0xE3) | RF_SLEEP);
			while ((lum_spiReadData(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00); // Wait for ModeReady
			pre_mode = RF_SLEEP;
		}
		return MODE_CHANGED;
	}
	else
	{
		return MODE_NOCHANGED;
	}
}


static void USER_FUNC lum_writeInitPara(void)
{
	U8 i;
	U8 paraCount;


	paraCount = sizeof(RegistersCfg)/sizeof(_SX1231_REG);
	for(i=0; i<paraCount; i++)
	{
		lum_spiWriteData(RegistersCfg[i].addr, RegistersCfg[i].val);
	}
}


static void USER_FUNC lum_testSpi(void)
{
	U8 data;


	while(1)
	{
		lum_spiWriteData(REG_SYNCVALUE5, SPI_TEST_DATA);
		msleep(10);
		data = lum_spiReadData(REG_SYNCVALUE5);
		if(data != SPI_TEST_DATA)
		{
			msleep(10);
		}
		else
		{
			break;
		}
	}
}



void USER_FUNC lum_sx1208ChipInit(void)
{
	lum_spiInit();
	msleep(100);
	lum_sx1208HwReset();
	lum_testSpi();
	lum_rfDIOsInit();
	lum_writeInitPara();
	lum_setRfMode(RF_SLEEP);
}


static void USER_FUNC lum_writeFrFrequent(U32 originFreq)
{
	U32 freq;


	freq = (U32)(originFreq/61.03515625); //433.92 ==>7109345.28
	lum_spiWriteData(REG_FRFMSB,(U8)(freq>>16));
	lum_spiWriteData(REG_FRFMID,(U8)(freq>>8));
	lum_spiWriteData(REG_FRFLSB,(U8)(freq));
}


static void USER_FUNC lum_changeToRecvMode(U32 freq)
{
	lum_spiWriteData(REG_OOKFIX, 0x50);
	lum_writeFrFrequent(freq);
	lum_setRfMode(RF_RECEIVER);
	msleep(100);
}


static void USER_FUNC lum_changeToSendMode(U32 freq)
{
	lum_writeFrFrequent(freq);//433.92MHz
	lum_setRfMode(RF_TRANSMITTER);
	msleep(100);
}


static void USER_FUNC lum_changeToSearchFreqMode(void)
{
	lum_setRfMode(RF_RECEIVER);
	msleep(10);
	lum_spiWriteData(REG_RSSICONFIG, 0x01);
	lum_spiWriteData(REG_OOKFIX, 0x60);		//set RSSI threshold
	msleep(100);
}


static void USER_FUNC lum_delay15us(U32 delayUs)
{
	U32 i;
	U32 delayCount;

	delayCount = delayUs*134; //89=10us  
	for(i=0; i<delayCount; i++)
	{
		__nop();
	}
}


static void USER_FUNC lum_disableAllIrq(void)
{
	__disable_irq();
	hfthread_suspend_all();
}


static void USER_FUNC lum_enableAllIrq(void)
{
	hfthread_resume_all();
	__enable_irq();
}


static void USER_FUNC lum_searchNextFreq(void)
{
	lum_setSdo2InteruptStatus(FALSE);  //切换频点时会产生高电平，因此屏蔽该中断
	if(g_searchFreqData.curFreq == MIN_SEARCH_FREQUENT)
	{
		g_searchFreqData.foundIrq = 0;
	}
	lum_setRfMode(RF_STANDBY);
	lum_writeFrFrequent(g_searchFreqData.curFreq);
	lum_start433StudyTimer(MAX_SEARCH_FREQ_TIMER_GAP);
	lum_setRfMode(RF_RECEIVER);
	if(g_searchFreqData.foundIrq == 0)
	{
		msleep(2);
		lum_setSdo2InteruptStatus(TRUE);
	}
}


static void USER_FUNC lum_searchFreqCallback(void)
{
	U8 rssiData = 0;
	U32 readCount = 0;
	U32 beginTime = 0;
	U32 curTime = 0;
	U32 totalRssi = 0;
	U32 waitTime = MIN_SEARCH_FREQ_WAIT_TIME;
	static U8 g_bLastFound = 0;
	U32 test = 0;
	U32 lowCount = 0;



	if(g_searchFreqData.foundIrq)
	{
		lum_setSdo2InteruptStatus(FALSE);
		beginTime = hfsys_get_time();
		while(1)
		{
			test++;
			if((GpioGetReg(GPIO_C_IN)&0x04) != 0)
			{
				rssiData = lum_spiReadData(REG_RSSIVALUE);
				if(waitTime != MAX_SEARCH_FREQ_WAIT_TIME)
				{
					waitTime = MAX_SEARCH_FREQ_WAIT_TIME;
				}
				if(rssiData != 0x80)
				{
					readCount++;
					totalRssi += rssiData;
					if(readCount >= MAX_READ_RSSI_COUNT)
					{
						totalRssi /=  readCount;
						if(totalRssi > g_searchFreqData.maxRssi)
						{
							if(g_bLastFound != 0)
							{
								g_searchFreqData.maxRssi = totalRssi;
								g_searchFreqData.bestFreq = g_searchFreqData.curFreq;
							}
							else
							{
								totalRssi = 0;
								g_bLastFound = 1;
							}
						}
						curTime = hfsys_get_time();
						break;
					}
				}
			}
			else
			{
				lowCount++;
				curTime = hfsys_get_time();
				if(curTime < beginTime)
				{
					beginTime = 0;
				}
				if((curTime - beginTime) > waitTime)
				{
					totalRssi = 0;
					g_bLastFound = 0;
					break;
				}
			}
		}
	}

	lumi_debug("curFreq=%ld RSSI=%d bSdo2High=%d readCount=%d timegap=%d lowCount=%d test=%d\n", g_searchFreqData.curFreq, totalRssi, g_searchFreqData.foundIrq, readCount, (curTime - beginTime), lowCount, test);
	

	if(g_searchFreqData.curFreq < MAX_SEARCH_FREQUENT)
	{
		if(g_searchFreqData.foundIrq)
		{
			g_searchFreqData.curFreq += MIN_SEARCH_FREQ_GAP;
		}
		else
		{
			g_searchFreqData.curFreq += MAX_SEARCH_FREQ_GAP;
		}
		if(readCount == 0)
		{
			g_searchFreqData.foundIrq = 0;
		}
		lum_searchNextFreq();
	}
	else if(g_searchFreqData.maxRssi < MIN_SEARCH_FREQ_RSSI)
	{
		curTime = hfsys_get_time();
		if(curTime < MAX_STUDY_TIME_WAIT || (curTime - g_searchFreqData.timeout) < MAX_STUDY_TIME_WAIT)
		{
			memset(&g_searchFreqData, 0, sizeof(SEARCH_FREQ_INFO));
			g_searchFreqData.chipStatus = SX1208_SEARCHING;
			g_searchFreqData.curFreq = MIN_SEARCH_FREQUENT;
			g_searchFreqData.timeout = curTime;
			lum_searchNextFreq();
		}
		else
		{
			//Search Faild;
			lum_setSdo2InteruptStatus(FALSE);
			g_searchFreqData.chipStatus = SX1208_IDLE;
			lum_setRfMode(RF_SLEEP);
			lum_sendReplyStudyMessage();
			lumi_debug("search frequent timeout time1=%ld time2=%ld\n", g_searchFreqData.timeout, curTime);
		}
	}
	else
	{
		lum_setSdo2InteruptStatus(FALSE);
		lum_enterStudyMode(g_searchFreqData.bestFreq);
	}
}


static BOOL USER_FUNC lum_studyWaveData(ORIGIN_WAVE_DATA* pWaveInfo)
{
	U32 lastGpioStatus = 0;
	U32 curGpioStatus;
	BOOL afterFirstWave = FALSE;
	BOOL startSave = FALSE;
	U32 protectCount = 0;
	BOOL ret = FALSE;


	lum_setSdo2InteruptStatus(FALSE); //中断服务程序内部不能禁止中断，故放此处
	lum_disableAllIrq();
	while(protectCount < 100000)  //1S
	{
		curGpioStatus = (GpioGetReg(GPIO_C_IN)&0x04);
		if(!afterFirstWave)
		{
			if(curGpioStatus != lastGpioStatus)
			{
				lastGpioStatus = curGpioStatus;
				pWaveInfo->waveData[0] = 0;
			}
			else
			{
				pWaveInfo->waveData[0]++;
			}

			if(pWaveInfo->waveData[0] >= MIN_WAVE_GAP_TIME) // > 3ms
			{
				afterFirstWave = TRUE;
				pWaveInfo->waveData[0] = 0;
			}
		}
		else
		{
			if(curGpioStatus != lastGpioStatus)
			{
				lastGpioStatus = curGpioStatus;
				if(!startSave)
				{
					startSave = TRUE;
				}
				else
				{
					pWaveInfo->waveData[pWaveInfo->waveCount]++;
					pWaveInfo->waveCount++;
					if(pWaveInfo->waveCount >= MAX_WAVE_DATA_COUNT)
					{
						break;
					}
				}
			}
			else
			{
				if(startSave)
				{
					//g_timing = 0;
					pWaveInfo->waveData[pWaveInfo->waveCount]++;
					__nop();
					__nop();
					__nop();
				}

				if(pWaveInfo->waveData[pWaveInfo->waveCount] >= MIN_WAVE_GAP_TIME) // > 3ms
				{
					pWaveInfo->waveCount++;
					if(pWaveInfo->waveCount > 20)
					{
						ret = TRUE;
					}
					break;
				}
			}
		}
		lum_delay15us(1);
		protectCount++;
	}
	lum_enableAllIrq();
	return ret;
}


static void USER_FUNC lum_sendStudyWaveData(ORIGIN_WAVE_DATA* pWaveInfo)
{
	static U8 g_sendCount = 0;
	U8 index;
	U16 clkCount;
	BOOL gpioHighLevel;


	lum_disableAllIrq();
	for(index=0; index<pWaveInfo->waveCount; index++)
	{
		gpioHighLevel = ((index%2) == 0)?TRUE:FALSE;
		for(clkCount = 0; clkCount<pWaveInfo->waveData[index]; clkCount++) //only for clk timing
		{
			if(gpioHighLevel)
			{
				GpioSetRegOneBit(GPIO_C_OUT, (1<<2));
			}
			else
			{
				GpioClrRegOneBit(GPIO_C_OUT, (1<<2));
			}
			lum_delay15us(1);
			__nop();
			__nop();
			__nop();
		}
	}
	lum_enableAllIrq();

	g_sendCount++;
	if(g_sendCount < MAX_WAVE_RESEND_COUNT)
	{
		lum_start433StudyTimer(MAX_SEND_WAVE_TIME_DELAY);
	}
	else
	{
		g_sendCount = 0;
		lum_setRfMode(RF_SLEEP);
		g_searchFreqData.chipStatus = SX1208_IDLE;
	}
}


static void USER_FUNC lum_433StudyTimerCallback( hftimer_handle_t htimer )
{
	if(g_searchFreqData.chipStatus == SX1208_SEARCHING || g_searchFreqData.chipStatus == SX1208_SEARCH_AGAIN) //扫频状态
	{
		lum_searchFreqCallback();
	}
	else if(g_searchFreqData.chipStatus == SX1208_STUDYING) //学习状态
	{
		BOOL studyStatus;


		g_pWaveDataInfo->waveFreq = g_searchFreqData.bestFreq;
		studyStatus = lum_studyWaveData(g_pWaveDataInfo);
		if(studyStatus)
		{
			g_socketSaveData.pWaveData = g_pWaveDataInfo;
		}
		lum_setRfMode(RF_STANDBY);
		hfgpio_fset_out_high(HFGPIO_F_WIFI_LED); //close wifi LED
		g_searchFreqData.chipStatus = SX1208_IDLE;
		lum_sendReplyStudyMessage();
	}
	else if(g_searchFreqData.chipStatus == SX1208_SENDING)
	{
		lum_sendStudyWaveData(g_pWaveDataInfo);
	}
	else
	{
		// Do nothing
	}
}


static void USER_FUNC lum_start433StudyTimer(U32 timerGap)
{
	static hftimer_handle_t g_433StudyTimer = NULL;

	
	if(g_433StudyTimer == NULL)
	{
		g_433StudyTimer = hftimer_create("dealyStudyTimer", timerGap, false, STUDY_433_TIMER_ID, lum_433StudyTimerCallback, 0);
	}
	hftimer_change_period(g_433StudyTimer, timerGap);
}


static void USER_FUNC lum_sdo2IrqCallback(U32 arg1,U32 arg2)
{
	if(g_searchFreqData.chipStatus == SX1208_STUDYING)
	{
		lum_start433StudyTimer(5);
	}
	else
	{
		if(g_searchFreqData.chipStatus == SX1208_SEARCHING)
		{
			g_searchFreqData.chipStatus = SX1208_SEARCH_AGAIN;
			g_searchFreqData.curFreq = MIN_SEARCH_FREQUENT;
		}
		else if(g_searchFreqData.chipStatus == SX1208_SEARCH_AGAIN)
		{
			g_searchFreqData.foundIrq = TRUE;
		}
		else
		{
			// Do nothing
		}
	}
}


static void USER_FUNC lum_setSdo2InteruptStatus(BOOL enableIrq)
{
	if(enableIrq)
	{
		hfgpio_fenable_interrupt(HFGPIO_F_SDO_2);
		hfgpio_fset_out_high(HFGPIO_F_SDO_1);
	}
	else
	{
		hfgpio_fdisable_interrupt(HFGPIO_F_SDO_2);
		hfgpio_fset_out_low(HFGPIO_F_SDO_1);
	}
}


static void USER_FUNC lum_initSdo2Interupt(void)
{
	if(hfgpio_configure_fpin_interrupt(HFGPIO_F_SDO_2, (HFM_IO_TYPE_INPUT | HFPIO_IT_RISE_EDGE) , lum_sdo2IrqCallback, 0)!= HF_SUCCESS)
	{
		lumi_debug("configure HFGPIO_F_KEY fail\n");
		return;
	}
	lum_setSdo2InteruptStatus(FALSE);
	//切换频点时会产生高电平，因此屏蔽该中断
}


static void USER_FUNC lum_enterStudyMode(U32 freq)
{
	lumi_debug("=================> go into Study freq = %ld\n", freq);
	g_searchFreqData.chipStatus = SX1208_STUDYING;
	lum_changeToRecvMode(freq);
	lum_setSdo2InteruptStatus(TRUE);
}


static void USER_FUNC lum_enterSearchFreqMode(ORIGIN_WAVE_DATA* pWaveDataInfo)
{
	g_pWaveDataInfo = pWaveDataInfo;
	hfgpio_fset_out_low(HFGPIO_F_WIFI_LED); //Open LED
	memset(&g_searchFreqData, 0, sizeof(SEARCH_FREQ_INFO));
	memset(pWaveDataInfo, 0, sizeof(ORIGIN_WAVE_DATA));
	g_searchFreqData.chipStatus = SX1208_SEARCHING;
	g_searchFreqData.curFreq = MIN_SEARCH_FREQUENT;
	g_searchFreqData.timeout = hfsys_get_time();
	lum_changeToSearchFreqMode();
	lum_initSdo2Interupt();
	lum_searchNextFreq();
	lum_start433StudyTimer(MAX_SEARCH_FREQ_TIMER_GAP);
}


static void USER_FUNC lum_enterSendMode(ORIGIN_WAVE_DATA* pWaveDataInfo)
{
	lum_setRfMode(RF_STANDBY);
	msleep(100);
	hfgpio_fset_out_low(HFGPIO_F_SDO_2);
	lum_changeToSendMode(pWaveDataInfo->waveFreq);
	g_pWaveDataInfo = pWaveDataInfo;
	lum_start433StudyTimer(MAX_SEND_WAVE_TIME_DELAY);
	g_searchFreqData.chipStatus = SX1208_SENDING;
}


void USER_FUNC lum_saveStudySocketData(STUDY_SOCKET_SAVE_DATA* pSaveData)
{
	memcpy(&g_socketSaveData, pSaveData, sizeof(STUDY_SOCKET_SAVE_DATA));
}


STUDY_SOCKET_SAVE_DATA* USER_FUNC lum_getStudySocketData(void)
{
	return &g_socketSaveData;
}


static void USER_FUNC lum_sendReplyStudyMessage(void)
{
	insertLocalMsgToList(MSG_LOCAL_EVENT, NULL, 0, MSG_CMD_433M_REPLY_STUDY_STATUS);
}


void USER_FUNC lum_study433Wave(void)
{
	lum_enterSearchFreqMode(&g_studyWaveData);
}


void USER_FUNC lum_send433Wave(ORIGIN_WAVE_DATA* pWaveDataInfo)
{
	memcpy(&g_studyWaveData, pWaveDataInfo, sizeof(ORIGIN_WAVE_DATA));
	lum_enterSendMode(&g_studyWaveData);
}


#ifdef SX1208_433M_TEST
static ORIGIN_WAVE_DATA studyWaveData[2];

void USER_FUNC lum_studyWaveTest(U16 index)
{
	lum_enterSearchFreqMode(&studyWaveData[index]);
}


void USER_FUNC lum_sendWaveTest(U8 index)
{
	lumi_debug("freq=%ld, len=%d\n", studyWaveData[index].waveFreq, studyWaveData[index].waveCount);
	lum_enterSendMode(&studyWaveData[index]);
}

#endif //SX1208_433M_TEST

#endif //SX1208_433M_SUPPORT
#endif

