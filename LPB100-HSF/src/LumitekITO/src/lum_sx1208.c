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


#if 0
static void USER_FUNC lum_spiWriteData(U8 addr, U8* data, U8 len)
{
	U8 buf[30];
	hfspi_cs_low();
	addr |= 0x80;
	buf[0] = addr;
	memcpy((buf+1), data, len);
	hfspi_master_send_data((S8*)(&addr), (len+1));
	hfspi_cs_high();
}


static void USER_FUNC lum_spiReadData(U8 addr, U8* data, U8 len)
{
	hfspi_cs_low();
	addr &= 0x7F;
	hfspi_master_send_data((S8*)(&addr), 1);
	hfspi_master_recv_data((S8*)data, len);
	hfspi_cs_high();
}
#else
static void USER_FUNC lum_spiWriteData(U8 addr, U8 data)
{
#if 1
	U8 buf[2];


	hfspi_cs_low();
	addr |= 0x80;
	buf[0] = addr;
	buf[1] = data;
	hfspi_master_send_data((S8*)(buf), 2);
	hfspi_cs_high();
#else
addr |= 0x80;
hfspi_cs_low();
hfspi_master_send_data((S8*)(&addr), 1);
hfspi_master_send_data((S8*)(&data), 1);
hfspi_cs_high();

#endif
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

#endif

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


static void USER_FUNC lum_writeFrFrequent(U32 originFreq)
{
	U32 freq;


	freq = (U32)(originFreq/61.03515625);
	// 7109345.28
	lum_spiWriteData(REG_FRFMSB,(U8)(freq>>16));
	lum_spiWriteData(REG_FRFMID,(U8)(freq>>8));
	lum_spiWriteData(REG_FRFLSB,(U8)(freq));
}


static void USER_FUNC lum_changeToRecvMode(U32 freq)
{
	lum_spiWriteData(REG_OOKFIX, 0x50);
	lum_writeFrFrequent(freq);//433.92MHz
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
	msleep(10);
}


ORIGIN_WAVE_DATA g_waveDataInfo[2];
static U8 g_waveIndex_test;
static SEARCH_FREQ_INFO g_searchFreqData;
extern unsigned int GpioGetReg(unsigned char RegIndex);;
extern void GpioSetRegOneBit(unsigned char	RegIndex, unsigned int GpioIndex);
extern void GpioClrRegOneBit(unsigned char	RegIndex, unsigned int GpioIndex);


static void USER_FUNC lum_delay10us(U32 delayUs)
{
	U32 i;
	U32 delayCount;

	delayCount = delayUs*120; //89=10us
	for(i=0; i<delayCount; i++)
	{
		__nop();
	}
}


static void USER_FUNC lum_delayAfterStudyCallback( hftimer_handle_t htimer )
{
	hfgpio_fenable_interrupt(HFGPIO_F_SDO_2);

	lumi_debug("hfgpio_fenable_interrupt(HFGPIO_F_SDO_2); 22 \n");
}


static void USER_FUNC lum_delayAfterStudyTimer(void)
{
	static hftimer_handle_t lum_delayStudyTimer = NULL;


	if(lum_delayStudyTimer == NULL)
	{
		lum_delayStudyTimer = hftimer_create("dealyStudyTimer", 1000, false, 16, lum_delayAfterStudyCallback, 0);
	}
	hftimer_change_period(lum_delayStudyTimer, 1000);
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


//O18_GPIO_BANK_A(12),  //lpb nReload
//GpioGetReg(GPIO_A_IN)&(1<<12)

//#ifdef FORMAT_WAVE_SUPPORT
#if 0
static void USER_FUNC lum_formatWaveData(U16* waveData, U16 dataLen, WAVE_DATA_INFO* pWaveInfo)
{
	U16 i;
	U16 j;
	U16 minData;
	U16 index = 0;
	U16 dataIndex = 0;
	U8 waveLevel;
	U16 tmp;
	U16 halfMinData;
	U8 minWaveGap[100];

	memset(pWaveInfo, 0, sizeof(WAVE_DATA_INFO));
	//get min data
	minData = waveData[0];
	for(i=1; i<dataLen; i++)
	{
		if(waveData[i] < minData)
		{
			minData = waveData[i];
		}
	}

	//获取1.5CKL 范围内个数最多的为最小clk周期
	memset(minWaveGap, 0, sizeof(minWaveGap));
	halfMinData = minData>>1;
	for(i=0; i<dataLen; i++)
	{
		tmp = waveData[i] - minData;
		if(tmp < halfMinData)
		{
			minWaveGap[tmp] += 1;
		}
	}
	index = 0;
	for(i=1; i<halfMinData; i++)
	{
		if(minWaveGap[i] > minWaveGap[i-1])
		{
			index = i;
		}
	}
	minData += index;

	index = 0;
	pWaveInfo->waveClkTime = minData;
	halfMinData = minData>>1;
	for(i=0; i<dataLen; i++)
	{
		waveLevel = (i+1)%2;

		tmp = (waveData[i] + halfMinData)/minData;
		for(j=0; j<tmp; j++)
		{
			pWaveInfo->waveData[dataIndex] = (pWaveInfo->waveData[dataIndex]<<1) + waveLevel;
			index++;
			if(index == 8)
			{
				dataIndex++;
				index = 0;
			}
		}
	}
	pWaveInfo->waveDataLen = dataIndex++;
}


static BOOL USER_FUNC lum_studyWaveData(WAVE_DATA_INFO* pWaveInfo)
{
	U32 lastGpioStatus = 0;
	U32 curGpioStatus;
	BOOL afterFirstWave = FALSE;
	BOOL startSave = FALSE;
	U32 protectCount = 0;
	U16 waveIndex = 0;
	BOOL ret = FALSE;
	U16 waveData[100];



	memset(waveData, 0, sizeof(waveData));

	while(protectCount < 100000)  //1S
	{
		curGpioStatus = (GpioGetReg(GPIO_A_IN)&0x1000);
		if(!afterFirstWave)
		{
			if(curGpioStatus != lastGpioStatus)
			{
				lastGpioStatus = curGpioStatus;
				waveData[0] = 0;
			}
			else
			{
				waveData[0]++;
			}

			if(waveData[0] > MIN_WAVE_GAP_TIME) // > 3ms
			{
				afterFirstWave = TRUE;
				waveData[0] = 0;
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
					waveData[waveIndex]++;
					waveIndex++;
				}

#if 0
				if(gGpioStatus)
				{
					GpioClrRegOneBit(GPIO_B_OUT, (1<<27));
					gGpioStatus = FALSE;
				}
				else
				{
					GpioSetRegOneBit(GPIO_B_OUT, (1<<27));
					gGpioStatus = TRUE;
				}
#endif
			}
			else
			{
				if(startSave)
				{
					waveData[waveIndex]++;
					g_timing = 1;
				}

				if(waveData[waveIndex] > MIN_WAVE_GAP_TIME) // > 3ms
				{
					hfgpio_fdisable_interrupt(HFGPIO_F_SDO_2);
					lum_delayAfterStudyTimer();
					lum_formatWaveData(waveData, (waveIndex+1), pWaveInfo);
					ret = TRUE;
					break;
				}
			}
		}
		lum_delay10us(1);
		protectCount++;
	}
	return ret;
}


static void USER_FUNC lum_sendStudyWaveData(WAVE_DATA_INFO* pWaveInfo)
{
	U8 index;
	S8 bitNum;
	U16 clkCount;
	BOOL gpioHighLevel;


	for(index=0; index<pWaveInfo->waveDataLen; index++)
	{
		for(bitNum=7; bitNum>=0; bitNum--)
		{
			gpioHighLevel = ((pWaveInfo->waveData[index]&(1<<bitNum)) == 0)?FALSE:TRUE;
#if 0
			if(gpioHighLevel)
			{
				GpioSetRegOneBit(GPIO_A_OUT, 0x1000);
			}
			else
			{
				GpioClrRegOneBit(GPIO_A_OUT, 0x1000);
			}
			lum_delay10us(pWaveInfo->waveClkTime);
#else
			for(clkCount = 0; clkCount<pWaveInfo->waveClkTime; clkCount++) //only for clk timing
			{
				if(gpioHighLevel)
				{
					GpioSetRegOneBit(GPIO_B_OUT, (1<<27));
				}
				else
				{
					GpioClrRegOneBit(GPIO_B_OUT, (1<<27));
				}
				lum_delay10us(1);
			}
#endif
		}
	}
}

static void USER_FUNC lum_studyTimercallback( hftimer_handle_t htimer )
{
	WAVE_DATA_INFO waveDataInfo;
	BOOL bSucc;

	__disable_irq();
	hfthread_suspend_all();
	bSucc = lum_studyWaveData(&waveDataInfo);

	//hfgpio_fset_out_low(HFGPIO_F_SDO_2);
	lum_delay10us(200);
	lum_sendStudyWaveData(&waveDataInfo);
	hfthread_resume_all();
	__enable_irq();
	lumi_debug("433 study bSucc=%d\n", bSucc);

}
#else
static BOOL USER_FUNC lum_studyWaveData(ORIGIN_WAVE_DATA* pWaveInfo)
{
	U32 lastGpioStatus = 0;
	U32 curGpioStatus;
	BOOL afterFirstWave = FALSE;
	BOOL startSave = FALSE;
	U32 protectCount = 0;
	BOOL ret = FALSE;



	memset(pWaveInfo, 0, sizeof(ORIGIN_WAVE_DATA));
	pWaveInfo->waveFreq = g_searchFreqData.bestFreq;
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

			if(pWaveInfo->waveData[0] > MIN_WAVE_GAP_TIME) // > 3ms
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

#if 0
				if(gGpioStatus)
				{
					GpioClrRegOneBit(GPIO_B_OUT, (1<<27));
					gGpioStatus = FALSE;
				}
				else
				{
					GpioSetRegOneBit(GPIO_B_OUT, (1<<27));
					gGpioStatus = TRUE;
				}
#endif
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
		lum_delay10us(1);
		protectCount++;
	}

	hfgpio_fdisable_interrupt(HFGPIO_F_SDO_2);
	//lum_delayAfterStudyTimer();

	return ret;
}


static void USER_FUNC lum_sendStudyWaveData(ORIGIN_WAVE_DATA* pWaveInfo)
{
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
			lum_delay10us(1);
			__nop();
			__nop();
			__nop();
		}
	}
	lum_enableAllIrq();
}


static void USER_FUNC lum_studyTimercallback( hftimer_handle_t htimer )
{
#if 1
	BOOL bSucc;

	lum_disableAllIrq();
	bSucc = lum_studyWaveData(&g_waveDataInfo[g_waveIndex_test]);
	lum_enableAllIrq();
	if(bSucc)
	{
		//lum_delay10us(285); // 1#
		//lum_delay10us(645); // 2#
		//lum_delay10us(137); // 3#
		//lum_delay10us(789); // 4#
		//lum_sendStudyWaveData(&g_waveDataInfo);
		lum_setRfMode(RF_STANDBY);
		hfgpio_fset_out_low(HFGPIO_F_SDO_2);
		hfgpio_fset_out_high(HFGPIO_F_WIFI_LED);
		lumi_debug("Study success ! \n");
	}
	else
	{
		lum_delayAfterStudyTimer();
	}
#else
BOOL status = FALSE;
U8 i;

__disable_irq();
for(i=0; i<50; i++)
{
	if(status)
	{
		GpioClrRegOneBit(GPIO_B_OUT, (1<<27));
		status = FALSE;
	}
	else
	{
		GpioSetRegOneBit(GPIO_B_OUT, (1<<27));
		status = TRUE;
	}
	lum_delay10us(1);
}
__enable_irq();
#endif
}

#endif

static void USER_FUNC lum_startStudyTimer(void)
{
	static hftimer_handle_t lum_studyTimer = NULL;



	if(lum_studyTimer == NULL)
	{
		lum_studyTimer = hftimer_create("433StudyTimer", 5, false, 15, lum_studyTimercallback, 0);
	}
	hftimer_change_period(lum_studyTimer, 5);
}


static void USER_FUNC lum_irqCallback(U32 arg1,U32 arg2)
{
	//lumi_debug("go into lum_irqCallback bInSearch=%d bSdo2High=%d searchAgain=%d\n", g_searchFreqData.bInSearch, g_searchFreqData.bSdo2High, g_searchFreqData.searchAgain);
		
	if(g_searchFreqData.bInSearch != 0)
	{
		if(g_searchFreqData.searchAgain == 0)
		{
			g_searchFreqData.searchAgain = 1;
			g_searchFreqData.curFreq = MIN_SEARCH_FREQUENT;
			g_searchFreqData.bestFreq = 0;
			g_searchFreqData.maxRssi = 0;
		}
		else
		{
			g_searchFreqData.bSdo2High = 1;
			g_searchFreqData.disableSearchIrq = 1;
		}
	}
	else
	{
		lum_startStudyTimer();
	}
}


static void USER_FUNC lum_startStudyIrq(void)
{
	U32 irqFlag;

	irqFlag = HFM_IO_TYPE_INPUT | HFPIO_IT_RISE_EDGE;
	//hfgpio_fset_out_low(HFGPIO_F_SDO_0);

	if(hfgpio_configure_fpin_interrupt(HFGPIO_F_SDO_2, irqFlag , lum_irqCallback, 1)!= HF_SUCCESS)
	{
		lumi_debug("configure HFGPIO_F_KEY fail\n");
		return;
	}
}


static void USER_FUNC lum_enterStudyMode(U32 freq)
{
	lumi_debug("=================> go into Study freq = %ld\n", freq);
	lum_changeToRecvMode(freq);
	lum_startStudyIrq();
}


static void USER_FUNC lum_searchNextFreq(hftimer_handle_t htimer)
{
	lum_setRfMode(RF_STANDBY);
	lum_writeFrFrequent(g_searchFreqData.curFreq);
	hftimer_change_period(htimer, MAX_SEARCH_FREQ_TIMER_GAP);
	lum_setRfMode(RF_RECEIVER);
}


static void USER_FUNC lum_searchFreqCallback( hftimer_handle_t htimer )
{
	U8 rssiData = 0;
	U32 readCount = 0;
	U32 beginTime = 0;
	U32 curTime = 0;
	U32 totalRssi = 0;
	U32 waitTime = MIN_SEARCH_FREQ_WAIT_TIME;


	if(g_searchFreqData.disableSearchIrq != 0)
	{
		hfgpio_fdisable_interrupt(HFGPIO_F_SDO_2);
		g_searchFreqData.disableSearchIrq = 0;
	}
	if(g_searchFreqData.bSdo2High)
	{
		beginTime = hfsys_get_time();
		while(1)
		{
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
							g_searchFreqData.maxRssi = totalRssi;
							g_searchFreqData.bestFreq = g_searchFreqData.curFreq;
						}
						break;
					}
				}
			}
			else
			{
				curTime = hfsys_get_time();
				if(curTime < beginTime)
				{
					beginTime = 0;
				}
				if((curTime - beginTime) > waitTime)
				{
					totalRssi = 0;
					break;
				}
			}
		}
	}

	lumi_debug("curFreq=%ld RSSI=%d bSdo2High=%d readCount=%d timegap=%d\n", g_searchFreqData.curFreq, totalRssi, g_searchFreqData.bSdo2High, readCount, (curTime - beginTime));
	

	if(g_searchFreqData.curFreq < MAX_SEARCH_FREQUENT)
	{
		if(g_searchFreqData.bSdo2High)
		{
			g_searchFreqData.curFreq += MIN_SEARCH_FREQ_GAP;
		}
		else
		{
			g_searchFreqData.curFreq += MAX_SEARCH_FREQ_GAP;
		}
		lum_searchNextFreq(htimer);
	}
	else if(g_searchFreqData.maxRssi < MIN_SEARCH_FREQ_RSSI)
	{
		g_searchFreqData.curFreq = MIN_SEARCH_FREQUENT;
		g_searchFreqData.bSdo2High = 0;
		g_searchFreqData.searchAgain = 0;
		g_searchFreqData.disableSearchIrq = 0;
		hfgpio_fenable_interrupt(HFGPIO_F_SDO_2);
		//lumi_debug("hfgpio_fenable_interrupt(HFGPIO_F_SDO_2)\n");
		lum_searchNextFreq(htimer);
	}
	else
	{
		hftimer_stop(htimer);
		g_searchFreqData.bInSearch = 0;
		lum_enterStudyMode(g_searchFreqData.bestFreq);
	}
}


static void USER_FUNC lum_startSearchFreqTimer(void)
{
	static hftimer_handle_t lum_searchFreqTimer = NULL;


	if(lum_searchFreqTimer == NULL)
	{
		lum_searchFreqTimer = hftimer_create("searchFreqTimer", MAX_SEARCH_FREQ_TIMER_GAP, false, 17, lum_searchFreqCallback, 0);
	}
	hftimer_change_period(lum_searchFreqTimer, MAX_SEARCH_FREQ_TIMER_GAP);
}


void USER_FUNC lum_enterSearchFreqMode(U8 index)
{
	g_waveIndex_test = index;
	hfgpio_fset_out_low(HFGPIO_F_WIFI_LED);
	memset(&g_searchFreqData, 0, sizeof(SEARCH_FREQ_INFO));
	g_searchFreqData.bInSearch = 1;
	lum_changeToSearchFreqMode();
	lum_startStudyIrq();
	g_searchFreqData.curFreq = MIN_SEARCH_FREQUENT;
	lum_writeFrFrequent(g_searchFreqData.curFreq);
	lum_startSearchFreqTimer();
}



void USER_FUNC lum_enterSendMode(U8 index)
{
	U8 i;

	lumi_debug("=================> go into Send mode index = %d waveFreq=%ld waveCount=%d\n", index, g_waveDataInfo[index].waveFreq, g_waveDataInfo[index].waveCount);
	lum_setRfMode(RF_STANDBY);
	msleep(100);
	hfgpio_fset_out_low(HFGPIO_F_SDO_2);
	lum_changeToSendMode(g_waveDataInfo[index].waveFreq);

	for(i=0; i<MAX_WAVE_RESEND_COUNT; i++)
	{
		lum_sendStudyWaveData(&g_waveDataInfo[index]);
		msleep(10);
	}
	lum_setRfMode(RF_SLEEP);
}


void USER_FUNC lum_sx1208Init(void)
{
	U8 data = 0;


	memset(&g_searchFreqData, 0, sizeof(SEARCH_FREQ_INFO));
	lum_spiInit();
	msleep(100);
	lum_sx1208HwReset();
	//lum_spiWriteData(REG_SYNCVALUE5, 0x55);
	while(data != SPI_TEST_DATA)
	{
		lum_spiWriteData(REG_SYNCVALUE5, SPI_TEST_DATA);
		msleep(10);
		data = lum_spiReadData(REG_SYNCVALUE5);
		lumi_debug("data=0x%X\n", data);
		msleep(10);
	}
	lum_rfDIOsInit();
	lum_setRfMode(RF_SLEEP);
	lum_writeInitPara();
	lum_setRfMode(RF_SLEEP);
}

#endif //SX1208_433M_SUPPORT
#endif

