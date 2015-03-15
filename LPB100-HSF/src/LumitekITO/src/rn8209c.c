/*
******************************
*Company:Lumitek
*Data:2014-12-24
*Author:Meiyusong
******************************
*/

#include "../inc/lumitekConfig.h"

#ifdef CONFIG_LUMITEK_DEVICE

#ifdef RN8209C_SUPPORT

#include <hsf.h>
#include <string.h>
#include <stdio.h>

#include "../inc/itoCommon.h"
#include "../inc/rn8209c.h"
#include "../inc/lumLog.h"
#include "../inc/asyncMessage.h"
#include "../inc/deviceGpio.h"


hfthread_mutex_t g_rn8209c_mutex = NULL;
RN8209C_CALI_DATA* g_pRn8209cCaliData;
ENERGY_DATA_INFO	g_energyData;




static void USER_FUNC lum_rn8209cClearEnergyData(void)
{
	U32 energyFlag;


	energyFlag = ENERGY_DATA_FLAG;
	hfuflash_erase_page(ENERGY_DATA_OFFSET, ENERGY_DATA_TOTAL_SIZE/HFFLASH_PAGE_SIZE);
	hfuflash_write(ENERGY_DATA_OFFSET, (S8*)(&energyFlag), ENERGY_DATA_SIZE);
	g_energyData.energyData = 0;
	g_energyData.energyOffset = ENERGY_DATA_SIZE;
	g_energyData.energyCurData = 0;
}


static void USER_FUNC lum_rn8209cInitEnergyData(void)
{
	U32 readBuf[ENERGY_PER_DEAD_LEN>>2];
	U32 energyFlag;
	U32 readOffset;
	S32 readSize;
	U16 i;
	U16 count;


	count = ENERGY_PER_DEAD_LEN>>2;
	hfuflash_read(ENERGY_DATA_OFFSET, (S8*)(&energyFlag), ENERGY_DATA_SIZE);
	if(energyFlag != ENERGY_DATA_FLAG)
	{
		lum_rn8209cClearEnergyData();
	}
	else
	{
		readOffset = 0;
		while(readOffset < ENERGY_DATA_TOTAL_SIZE)
		{
			memset(readBuf, 0, ENERGY_PER_DEAD_LEN);
			readSize = hfuflash_read((readOffset+ENERGY_DATA_OFFSET), (S8*)readBuf, ENERGY_PER_DEAD_LEN);
			if(readSize <= 0)
			{
				break;
			}
			if(readBuf[count-1] == 0xFFFFFFFF)
			{
				for(i=0; i<count; i++)
				{
					if(readBuf[i] == 0xFFFFFFFF)
					{
						g_energyData.energyOffset = readOffset + i<<2;
						if(g_energyData.energyOffset == (ENERGY_DATA_OFFSET + ENERGY_DATA_SIZE))
						{
							g_energyData.energyData = 0;
						}
						else
						{
							hfuflash_read((g_energyData.energyOffset + ENERGY_DATA_OFFSET - ENERGY_DATA_SIZE), (S8*)readBuf, ENERGY_DATA_SIZE);
							g_energyData.energyData = readBuf[0];
						}
						break;
					}
				}
			}
			else
			{
				readOffset += readSize;
			}
		}
	}
	lumi_debug("energyData=%d, energyOffset=0x%X\n", g_energyData.energyData, g_energyData.energyOffset);
}


void USER_FUNC lum_rn8209cAddEnergyData(void)
{
	g_energyData.energyCurData++;
	if(g_energyData.energyCurData >= ENERGY_SAVE_DATA_GAP)
	{
		g_energyData.energyData += g_energyData.energyCurData;
		g_energyData.energyCurData = 0;

		hfuflash_write((ENERGY_DATA_OFFSET+g_energyData.energyOffset), (S8*)(&g_energyData.energyData), ENERGY_DATA_SIZE);
		g_energyData.energyOffset += ENERGY_DATA_SIZE;
	}
}


U32 USER_FUNC lum_rn8209cGetUData(void)
{
	U32 energyData;

	energyData = g_energyData.energyData + g_energyData.energyCurData;
	energyData = energyData*100/RN8209C_DEFAULT_EC;
	return energyData;
}



static U8 USER_FUNC rn8209cGetChecksun(U8 cmd, U8* data, U8 dataLen)
{
	U32 totalSun;
	U8 checkSun;
	U8 i;


	if(dataLen > 0xFF || dataLen == 0)
	{
		return 0;
	}
	totalSun = cmd;
	for(i=0; i<dataLen; i++)
	{
		totalSun += data[i];
	}
	checkSun = (U8)(~(totalSun&0xFF));
	return checkSun;
}


static BOOL USER_FUNC rn8209cSetUartBaudrate(void)
{
	char rsp[64]= {0};
	BOOL ret = FALSE;
	S8* sensBuf1 = "AT+UART=0\r\n";
	S8* sendBuf = "AT+UART=4800,8,1,EVEN,NFC\r\n";
	S8* cmpBuf = "+ok=4800";


	memset(rsp, 0, sizeof(rsp));
	hfat_send_cmd(sensBuf1, strlen(sensBuf1),rsp,64);
	if(((rsp[0]=='+')&&(rsp[1]=='o')&&(rsp[2]=='k')))
	{
		if(memcmp(rsp, cmpBuf, strlen(cmpBuf)) != 0)
		{
			memset(rsp, 0, sizeof(rsp));
			hfat_send_cmd(sendBuf, strlen(sendBuf),rsp,64);
			if(((rsp[0]=='+')&&(rsp[1]=='o')&&(rsp[2]=='k')))
			{
				ret = TRUE;
			}
		}
		else
		{
			ret = TRUE;
		}
	}
	return ret;
}


static void USER_FUNC rn8209cWriteFrame(U8 addr, U8* data, U8 dataLen)
{
	U8 sendData[RN9029C_MAX_DATA_LEN];
	U8 sendLen;
	U8 cmd;
	U8 checkSun;
	U8 i;


	memset(sendData, 0, sizeof(sendData));
	cmd = addr|0x80;
	sendData[0] = cmd;
	//memcpy((sendData + 1), data, dataLen);
	for(i=1; i<=dataLen; i++)
	{
		sendData[i] = data[dataLen-i];
	}

	checkSun = rn8209cGetChecksun(cmd, data, dataLen);
	sendData[dataLen+1] = checkSun;
	sendLen = dataLen+2;

	hfuart_send(HFUART0, (S8*)sendData, sendLen, RN8209C_UART_TIMEOUT);

}


static BOOL USER_FUNC rn8209cReadFrame(U8 addr, U8* data, U8 readLen)
{
	U8 cmd;
	S32 recvLen = 0;
	S32 tmp;
	S8 readBuf[RN9029C_MAX_DATA_LEN];
	U8 checkSun;
	BOOL ret = FALSE;
	U8 i;
	U8 totalRead;
	U8 recvCount = 0;


	totalRead = readLen + 1;
	cmd = addr&0x7F;
	memset(readBuf, 0, sizeof(readBuf));
	hfthread_mutext_lock(g_rn8209c_mutex);
	hfuart_recv(HFUART0, readBuf, RN9029C_MAX_DATA_LEN, 1);
	hfuart_send(HFUART0, (S8*)(&cmd), 1, 50);
	memset(readBuf, 0, sizeof(readBuf));
	while(recvLen < totalRead && recvCount < 10)
	{
		tmp = hfuart_recv(HFUART0, (readBuf + recvLen), (totalRead-recvLen), 5);
		if(tmp > 0)
		{
			recvLen += tmp;
		}
		recvCount++;
	}
	hfthread_mutext_unlock(g_rn8209c_mutex);
	if(recvCount < 10)
	{
		checkSun = rn8209cGetChecksun(addr, (U8*)readBuf, readLen);
		if(checkSun == (U8)readBuf[readLen])
		{
			for(i=0; i<readLen; i++)
			{
				data[readLen-i-1] = readBuf[i];
			}
			ret = TRUE;
		}
	}
	return ret;
}


static void USER_FUNC lum_rn8209cReadIVPData(MeasureDataInfo* meatureInfo)
{
	U32 readDataLong;


	//读电流有效值
	readDataLong = 0;
	rn8209cReadFrame(RN8209C_IARMS, (U8*)&readDataLong, 3);
	if(readDataLong&0x800000)
	{
		meatureInfo->reco_irms = 0;
	}
	else
	{
		meatureInfo->reco_irms = readDataLong;
	}

	//读电压有效值
	readDataLong=0;
	rn8209cReadFrame(RN8209C_URMS, (U8*)&readDataLong, 3);
	if(readDataLong&0x800000)
	{
		meatureInfo->reco_urms = 0;
	}
	else
	{
		meatureInfo->reco_urms = readDataLong;
	}

	//读有功功率
	readDataLong = 0;
	rn8209cReadFrame(RN8209C_PowerPA, (U8*)&readDataLong, 4);
	if(readDataLong&0x80000000)
	{
		meatureInfo->reco_powerp = 0;
	}
	else
	{
		meatureInfo->reco_powerp = readDataLong;
	}
#ifdef LUM_RN8209C_UDP_LOG
	saveNormalLogData("RAW DATA irms=%d, urms=%d, powerp=%d\n", meatureInfo->reco_irms, meatureInfo->reco_urms, meatureInfo->reco_powerp);
#endif
}


static void USER_FUNC rn8209cUartInit(void)
{
	S8 readBuf[200];

	hfthread_mutext_new(&g_rn8209c_mutex);
	hfuart_open(RN8209C_UART_NO);
	msleep(10);
	rn8209cSetUartBaudrate();
	hfuart_recv(HFUART0, readBuf, 200, 10);
}


static void USER_FUNC lum_rn8209cChipInit(void)
{
	U8 writeData;
	U32 chipID;


	//uart init
	rn8209cUartInit();

	//rn8209c reset
	writeData = RN8209C_CMD_RESET;
	rn8209cWriteFrame(RN8209C_EA, &writeData, 1);
	msleep(300);

	//read chip ID
	chipID = 0;
	rn8209cReadFrame(RN8209C_DeviceID, (U8*)&chipID, 3);
#ifdef LUM_RN8209C_UDP_LOG
	saveNormalLogData("RN8209C_DeviceID =0x%x", chipID);
#endif

	//write en
	writeData = RN9208C_WRITE_EN;
	rn8209cWriteFrame(RN8209C_EA, &writeData, 1);

	//select path A
	writeData = RN8209C_PATH_A;
	rn8209cWriteFrame(RN8209C_EA, &writeData, 1);

	//write HFConst
	rn8209cWriteFrame(RN8209C_HFConst, (U8*)&g_pRn8209cCaliData->rn8209cHFCost, 2);

	//write protect
	writeData = RN8209C_WRITE_PROTECT;
	rn8209cWriteFrame(RN8209C_EA, &writeData, 1);
}


#if 0
static void USER_FUNC lum_rn8209cSetCaliData(MeatureEnergyData* meatureData)
{
	MeasureDataInfo meatureInfo;


	lum_rn8209cReadIVPData(&meatureInfo);

	g_pRn8209cCaliData->rn8209cKP = meatureInfo.reco_powerp / meatureData->powerP;
	g_pRn8209cCaliData->rn8209cKI = meatureInfo.reco_irms / meatureData->irms;
	g_pRn8209cCaliData->rn8209cKV = meatureInfo.reco_urms / meatureData->urms;
	g_pRn8209cCaliData->rn8209cHFCost = RN8209C_HF_COST_KPEC*g_pRn8209cCaliData->rn8209cKP/RN8209C_DEFAULT_EC;
	lum_rn8209cSaveCaliData();
}
#endif

static void USER_FUNC lum_rn8209cInitCaliData(void)
{
	BOOL needSave = FALSE;


	g_pRn8209cCaliData = lum_rn8209cGetCaliData();

	if(g_pRn8209cCaliData->rn8209cHFCost == 0)
	{
		g_pRn8209cCaliData->rn8209cHFCost = RN8209C_DEFAULT_HFCOST;
		needSave = TRUE;
	}
	if(g_pRn8209cCaliData->rn8209cKI == 0)
	{
		g_pRn8209cCaliData->rn8209cKI = RN8209C_DEFAULT_KI;
		needSave = TRUE;
	}
	if(g_pRn8209cCaliData->rn8209cKV == 0)
	{
		g_pRn8209cCaliData->rn8209cKV = RN8209C_DEFAULT_KV;
		needSave = TRUE;
	}
	if(g_pRn8209cCaliData->rn8209cKP == 0)
	{
		g_pRn8209cCaliData->rn8209cKP = RN8209C_DEFAULT_KP;
		needSave = TRUE;
	}

	if(needSave)
	{
		lum_rn8209cSaveCaliData();
	}
#ifdef LUM_RN8209C_UDP_LOG
	saveNormalLogData("HFCost=%d KI=%d KV=%d KP=%d\n", g_pRn8209cCaliData->rn8209cHFCost,
	                  g_pRn8209cCaliData->rn8209cKI,
	                  g_pRn8209cCaliData->rn8209cKV,
	                  g_pRn8209cCaliData->rn8209cKP);
#endif
}


void USER_FUNC lum_rn8209cInit(void)
{
	lum_rn8209cInitCaliData();
	lum_rn8209cInitEnergyData();
	lum_rn8209cChipInit();
	lum_rn8209cInitCfPin();
	insertLocalMsgToList(MSG_LOCAL_EVENT, NULL, 0, MSG_CMD_REPORT_ENERGY_DATA);
}


void USER_FUNC lum_rn8209cGetIVPData(MeatureEnergyData* meatureData)
{
	MeasureDataInfo meatureInfo;


	lum_rn8209cReadIVPData(&meatureInfo);

	meatureData->irms = meatureInfo.reco_irms/g_pRn8209cCaliData->rn8209cKI;
	meatureData->urms = meatureInfo.reco_urms/g_pRn8209cCaliData->rn8209cKV;
	meatureData->powerP = meatureInfo.reco_powerp/g_pRn8209cCaliData->rn8209cKP;
	meatureData->energyU = lum_rn8209cGetUData(); //0.01W
}


#endif /* RN8209C_SUPPORT */
#endif


