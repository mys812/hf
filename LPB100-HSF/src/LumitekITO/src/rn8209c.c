/*
******************************
*Company:Lumitek
*Data:2014-12-24
*Author:Meiyusong
******************************
*/

#include "../inc/lumitekConfig.h"

#ifdef CONFIG_LUMITEK_DEVICE
#include <hsf.h>
#include <string.h>
#include <stdio.h>

#include "../inc/itoCommon.h"
#include "../inc/rn8209c.h"
#include "../inc/deviceLog.h"

#ifdef RN8209C_SUPPORT

hfthread_mutex_t g_rn8209c_mutex = NULL;



#if 0
static U32 USER_FUNC rn8209cDelay(U32 ms)
{
	U32 i=0;
	U32 j;
	U32 time1;
	U32 time2;
	U32 time3;

	time1 = hfsys_get_time();
	while(i < ms)
	{
		j = 0;
		while(j < 11500)
		{
			j++;
		}
		i++;
	}
	time2 = hfsys_get_time();
	time3 = time2 - time1;
	return time3;
}
#endif


U8 USER_FUNC rn8209cGetCheckksun(U8 cmd, U8* data, U8 dataLen)
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
	char rsp[64]={0};
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
	
	checkSun = rn8209cGetCheckksun(cmd, data, dataLen);
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
		checkSun = rn8209cGetCheckksun(addr, (U8*)readBuf, readLen);
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


static void USER_FUNC rn8209cReadMeasureData(MeasureDataInfo* pMeatureData)
{
    U32 readDataLong;
    U16 readDataShort;
	U8 addr;


	//读电流有效值
#ifdef RN8209C_SELECT_PATH_A
	addr = RN8209C_IARMS;
#elif defined(RN8209C_SELECT_PATH_B)
	addr = RN8209C_IBRMS;
#else
	#error "Please select Path !"
#endif
    readDataLong = 0;
    rn8209cReadFrame(addr, (U8*)&readDataLong, 3);
    if(readDataLong&0x800000)
    {
        pMeatureData->reco_irms = 0;
    }
	else
	{
    	pMeatureData->reco_irms = readDataLong; //*100/2220;
	}

    //读电压有效值
    readDataLong=0;
    rn8209cReadFrame(RN8209C_URMS, (U8*)&readDataLong, 3);
    if(readDataLong&0x800000)
    {
        pMeatureData->reco_urms = 0;
    }
	else
	{
    	pMeatureData->reco_urms = readDataLong; //*100/2220;
	}

    //读电压频率
    readDataShort = 0;
    rn8209cReadFrame(RN8209C_UFreq, (U8*)&readDataShort, 2);
    pMeatureData->reco_freq = (357954500/8/readDataShort + 50)/100;

    //读有功功率
#ifdef RN8209C_SELECT_PATH_A
	addr = RN8209C_PowerPA;
#elif defined(RN8209C_SELECT_PATH_B)
	addr = RN8209C_PowerPB;
#else
	#error "Please select Path !"
#endif
    readDataLong = 0;
    rn8209cReadFrame(addr, (U8*)&readDataLong, 4);
	if(readDataLong&0x80000000)
	{
		pMeatureData->reco_powerp = 0;
	}
	else
	{
    	pMeatureData->reco_powerp = readDataLong;
	}

	//读无功功率
	readDataLong = 0;
	rn8209cReadFrame(RN8209C_PowerQ, (U8*)&readDataLong, 4);
	pMeatureData->reco_powerq = readDataLong;

	//读有功能量
	readDataLong = 0;
	rn8209cReadFrame(RN8209C_EnergyP, (U8*)&readDataLong, 3);
	pMeatureData->reco_energyp = readDataLong;

	//读无功能量
	readDataLong = 0;
	rn8209cReadFrame(RN8209C_EnergyQ, (U8*)&readDataLong, 3);
	pMeatureData->reco_energyq = readDataLong;

}



static U32 rn8029cKv = (1019464/220);
static U32 rn8029cKi = (16200/46);
static U32 rn8029cKp = (504061/100); //100W


static void USER_FUNC rn8209cCoverKvip(MeasureDataInfo* pMeatureData)
{
	pMeatureData->reco_irms /= rn8029cKi;
	pMeatureData->reco_urms /= rn8029cKv;
	pMeatureData->reco_powerp = pMeatureData->reco_powerp*100/rn8029cKp;
}
static void USER_FUNC rn8209cReadData(void)
{
	MeasureDataInfo meatureData;

	rn8209cReadMeasureData(&meatureData);
	rn8209cCoverKvip(&meatureData);
	saveNormalLogData("reco_irms=%d reco_urms=%d reco_freq=%d reco_powerp=%d reco_powerq=%d reco_energyp=%d reco_energyq=%d",
		meatureData.reco_irms, meatureData.reco_urms, meatureData.reco_freq, meatureData.reco_powerp, meatureData.reco_powerq,
		meatureData.reco_energyp, meatureData.reco_energyq);
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


static void USER_FUNC rn8209cInit(void)
{
	U8 writeData;
	U16 writeDataShort;
	U32 chipID;
#ifdef RN8209C_READ_TEST
	U16 readData;
#endif

	//uart init
	rn8209cUartInit();

	//rn8209c reset
	writeData = RN8209C_CMD_RESET;
	rn8209cWriteFrame(RN8209C_EA, &writeData, 1);
	msleep(300);
	
	//read chip ID
	chipID = 0;
	rn8209cReadFrame(RN8209C_DeviceID, (U8*)&chipID, 3);
    saveNormalLogData("addr=%s rn8209C_ID=0x%x", "RN8209C_DeviceID", chipID);

	//write en
	writeData = RN9208C_WRITE_EN;
	rn8209cWriteFrame(RN8209C_EA, &writeData, 1);
#ifdef RN8209C_READ_TEST
	readData = 0;
	rn8209cReadFrame(RN8209C_WData, (U8*)&readData, 2);
    saveNormalLogData("addr=%s writeData=0x%x readData=0x%x", "RN8209C_EA", writeData, readData);
#endif


#ifdef RN8209C_SELECT_PATH_A
    //select path A
    writeData = RN8209C_PATH_A;
    rn8209cWriteFrame(RN8209C_EA, &writeData, 1);
#ifdef RN8209C_READ_TEST
    readData = 0;
    rn8209cReadFrame(RN8209C_WData, (U8*)&readData, 2);
    saveNormalLogData("addr=%s writeData=0x%x readData=0x%x", "RN8209C_EA", writeData, readData);
#endif

#elif defined(RN8209C_SELECT_PATH_B)

	//set gain 4
	writeDataShort = 0x1670;
	rn8209cWriteFrame(RN8209C_SYSCON, (U8*)&writeDataShort, 2);
#ifdef RN8209C_READ_TEST
	readData = 0;
	rn8209cReadFrame(RN8209C_WData, (U8*)&readData, 2);
	saveNormalLogData("addr=%s writeData=0x%x readData=0x%x", "RN8209C_EA", writeData, readData);
#endif

    //select path B
    writeData = RN8209C_PATH_B;
    rn8209cWriteFrame(RN8209C_EA, &writeData, 1);
#ifdef RN8209C_READ_TEST
    readData = 0;
    rn8209cReadFrame(RN8209C_WData, (U8*)&readData, 2);
    saveNormalLogData("addr=%s writeData=0x%x readData=0x%x", "RN8209C_EA", writeData, readData);
#endif

#else

#error "Path not select !"

#endif

    //write HFConst
    //Vu = 220/(470ohm*4+10ohm*2+1ohm*1)=0.12
    //Vi = 15A*0.001ohm*4 or 16 = 0.06
    //EC = 3200
    //Un = 220V
    //Ib = 15A
    writeDataShort = 1013;  //INT[(14.8528*Vu*Vi*10^11 ) / (Un*Ib*Ec)]
    //writeDataShort = 0x0A27;
    rn8209cWriteFrame(RN8209C_HFConst, (U8*)&writeDataShort, 2);
#ifdef RN8209C_READ_TEST
    readData = 0;
    rn8209cReadFrame(RN8209C_WData, (U8*)&readData, 2);
    saveNormalLogData("addr=%s writeData=0x%x readData=0x%x", "RN8209C_HFConst", writeDataShort, readData);
#endif

	//write protect
	writeData = RN8209C_WRITE_PROTECT;
	rn8209cWriteFrame(RN8209C_EA, &writeData, 1);
#ifdef RN8209C_READ_TEST
    readData = 0;
    rn8209cReadFrame(RN8209C_WData, (U8*)&readData, 2);
    saveNormalLogData("addr=%s writeData=0x%x readData=0x%x", "RN8209C_EA", writeData, readData);
    rn8209cReadData();
#endif
}



static void USER_FUNC rn8209cReadDataThread(void *arg)
{
	rn8209cInit();
	
	msleep(RN8209C_READ_DATA_TIMEOUT);
	hfthread_enable_softwatchdog(NULL, 30); //Start watchDog
	while(1)
	{

		//lumi_debug(" rn8209cReadDataThread \n");
		hfthread_reset_softwatchdog(NULL); //tick watchDog
		rn8209cReadData();
		msleep(RN8209C_READ_DATA_TIMEOUT);
	}
}


void USER_FUNC rn8209cCreateThread(void)
{
	if(hfthread_create((PHFTHREAD_START_ROUTINE)rn8209cReadDataThread, "IOT_rn8209C", 256, NULL, HFTHREAD_PRIORITIES_LOW,NULL,NULL)!= HF_SUCCESS)
	{
		lumi_error("Create IOT_TD_M thread failed!\n");
	}
}

#endif /* RN8209C_SUPPORT */
#endif


