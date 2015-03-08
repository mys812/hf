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
#include "../inc/lumLog.h"

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


static void USER_FUNC rn8209cCalibratePower(void)
{
	U32 reco_powerp;
	U32 curKp;
	U32 curViVu;
	U32 curhfCost;
	U32 totalPowerP = 0;
	U8 readCount = 0;
	U8 readFaild = 0;
	U32 test[RN8209C_CALI_READ_COUNT+1];



	while(readCount < RN8209C_CALI_READ_COUNT)
	{
		reco_powerp = 0;
#ifdef RN8209C_SELECT_PATH_A
		rn8209cReadFrame(RN8209C_PowerPA, (U8*)&reco_powerp, 4);
#elif define(RN8209C_SELECT_PATH_B)
		rn8209cReadFrame(RN8209C_PowerPB, (U8*)&reco_powerp, 4);
#else
		#error "Path not select !"
#endif
		if(reco_powerp&0x80000000 || reco_powerp < RN8209C_MIN_CALI_RAW_POWER)
		{
			reco_powerp = 0;
			readFaild ++;
		}
		else
		{
			totalPowerP += reco_powerp;
			test[readCount] = reco_powerp;
			readCount++;
		}
		if(readFaild >= RN8209C_MAX_CALI_READ_FAILD)
		{
			readFaild = 0;
			totalPowerP = 0;
			readCount = 0;
		}
#ifdef LUM_RN8209C_UDP_LOG
		saveNormalLogData("reco_powerp=%d [%X] totalPowerP=%d readFaild=%d", reco_powerp, reco_powerp, totalPowerP, readFaild);
#endif
		msleep(200);
	}

	reco_powerp = totalPowerP/RN8209C_CALI_READ_COUNT;
	curKp = reco_powerp/RN8209C_CALIBRATE_POWER;
	curViVu = curKp*10000/RN8209C_Kp_ViVu_DATA;
	curhfCost = RN8209C_HF_COST_KPEC*curKp/RN8209C_DEFAULT_EC;

	rn8209cSetKpHFcost((U16)curKp, (U16)curhfCost, (U16)curViVu);
#ifdef LUM_RN8209C_UDP_LOG
	saveNormalLogData("Calibrate Kp=%d ViVu=%d hfCost=%d reco_powerp=%d %d %d %d %d %d %d %d %d %d %d %d", curKp, curViVu, curhfCost, reco_powerp,
		test[0], test[1], test[2], test[3], test[4], test[5], test[6], test[7], test[8], test[9], test[10]);
#endif
}


static U16 USER_FUNC rn8209cGetKpData(void)
{
	U16 Kp;

	rn8209cGetKpHFcost(&Kp, NULL, NULL, NULL);
	if(Kp == 0)
	{
		Kp = RN8209C_DEFAULT_KP;
	}
	return Kp;
}


static U16 USER_FUNC rn8209cGetHFcostData(BOOL rawData)
{
	U16 hfCost;

	rn8209cGetKpHFcost(NULL, &hfCost, NULL, NULL);
	if(hfCost == 0 && !rawData)
	{
		hfCost = RN8209C_DEFAULT_HFCOST;
	}
	return hfCost;
}


static U16 USER_FUNC rn8209cGetViVuData(void)
{
	U16 ViVu;

	rn8209cGetKpHFcost(NULL, NULL, &ViVu, NULL);
	if(ViVu == 0)
	{
		ViVu = RN8209C_DEFAULT_Vi_Vu;
	}
	return ViVu;
}


static U16 USER_FUNC rn8209cGetCaliFlagData(void)
{
	U16 flag;

	rn8209cGetKpHFcost(NULL, NULL, NULL, &flag);
	return flag;
}


static void USER_FUNC rn8209cChangeHFcost(void)
{
	U8 writeData;
	U16 hfCost;

	
	hfCost = rn8209cGetHFcostData(FALSE);
	
	writeData = RN9208C_WRITE_EN;
	rn8209cWriteFrame(RN8209C_EA, &writeData, 1);

    rn8209cWriteFrame(RN8209C_HFConst, (U8*)&hfCost, 2);

	writeData = RN8209C_WRITE_PROTECT;
	rn8209cWriteFrame(RN8209C_EA, &writeData, 1);
	
}


static void USER_FUNC rn8209cCoverKvip(MeasureDataInfo* pMeatureData)
{
	U16 Kp;


	Kp = rn8209cGetKpData();
	pMeatureData->reco_irms /= RN8209C_DEFAULT_KI;
	pMeatureData->reco_urms /= RN8209C_DEFAULT_KV;
	pMeatureData->reco_powerp = pMeatureData->reco_powerp*100/Kp;
	if(pMeatureData->reco_powerp < 50)  //<0.05W = 0
	{
		pMeatureData->reco_powerp = 0;
	}
}
static void USER_FUNC rn8209cReadData(void)
{
	MeasureDataInfo meatureData;

	rn8209cReadMeasureData(&meatureData);
	rn8209cCoverKvip(&meatureData);
#ifdef LUM_RN8209C_UDP_LOG
	saveNormalLogData("reco_irms=%d reco_urms=%d reco_freq=%d reco_powerp=%d reco_powerq=%d reco_energyp=%d reco_energyq=%d Kp=%d HFcost=%d, ViVu=%d flag=0x%x",
		meatureData.reco_irms, meatureData.reco_urms, meatureData.reco_freq, meatureData.reco_powerp, meatureData.reco_powerq,
		meatureData.reco_energyp, meatureData.reco_energyq, rn8209cGetKpData(), rn8209cGetHFcostData(TRUE), rn8209cGetViVuData(), rn8209cGetCaliFlagData());
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


static void USER_FUNC rn8209cInit(void)
{
	U8 writeData;
	U16 writeDataShort;
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
    saveNormalLogData("addr=%s rn8209C_ID=0x%x", "RN8209C_DeviceID", chipID);
#endif

	//write en
	writeData = RN9208C_WRITE_EN;
	rn8209cWriteFrame(RN8209C_EA, &writeData, 1);

#ifdef RN8209C_SELECT_PATH_A
    //select path A
    writeData = RN8209C_PATH_A;
    rn8209cWriteFrame(RN8209C_EA, &writeData, 1);

#elif defined(RN8209C_SELECT_PATH_B)

	//set gain 4
	writeDataShort = 0x1670;
	rn8209cWriteFrame(RN8209C_SYSCON, (U8*)&writeDataShort, 2);

    //select path B
    writeData = RN8209C_PATH_B;
    rn8209cWriteFrame(RN8209C_EA, &writeData, 1);
#else

#error "Path not select !"

#endif

    //write HFConst
    writeDataShort = rn8209cGetHFcostData(FALSE);  //INT[(14.8528*Vu*Vi*10^11 ) / (Un*Ib*Ec)]

    //writeDataShort = 0x0A27;
    rn8209cWriteFrame(RN8209C_HFConst, (U8*)&writeDataShort, 2);

	//write protect
	writeData = RN8209C_WRITE_PROTECT;
	rn8209cWriteFrame(RN8209C_EA, &writeData, 1);
}



static void USER_FUNC rn8209cReadDataThread(void *arg)
{
	rn8209cInit();

	//check calibrate status
	if(rn8209cGetCaliFlagData() != RN8209C_CALI_FALG)
	{
		rn8209cCalibratePower();
		rn8209cChangeHFcost();
		msleep(1000);
	}
	else
	{
		msleep(RN8209C_READ_DATA_TIMEOUT);
	}
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


