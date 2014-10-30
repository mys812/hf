/*
******************************
*Company:Lumitek
*Data:2014-10-07
*Author:Meiyusong
******************************
*/

#include "../inc/lumitekConfig.h"

#ifdef CONFIG_LUMITEK_DEVICE
#include <hsf.h>
#include <string.h>
#include <stdio.h>

#include "../inc/itoCommon.h"
#include "../inc/deviceGpio.h"
#include "../inc/aes.h"



static S8 g_udp_recv_buf[NETWORK_MAXRECV_LEN];
static S8 g_tcp_recv_buf[NETWORK_MAXRECV_LEN];

static GLOBAL_CONFIG_DATA g_deviceConfig;


S8* USER_FUNC getUdpRecvBuf(BOOL setZero)
{
	if(setZero)
	{
		memset(g_udp_recv_buf, 0, NETWORK_MAXRECV_LEN);
	}
	return g_udp_recv_buf;
}


S8* USER_FUNC getTcpRecvBuf(BOOL setZero)
{
	if(setZero)
	{
		memset(g_tcp_recv_buf, 0, NETWORK_MAXRECV_LEN);
	}
	return g_tcp_recv_buf;
}



U16 USER_FUNC getSocketSn(BOOL needIncrease)
{
	if(needIncrease)
	{
		g_deviceConfig.globalData.socketSn++;
		if(g_deviceConfig.globalData.socketSn >= INVALID_SN_NUM)
		{
			g_deviceConfig.globalData.socketSn = 0;
		}
	}
	return g_deviceConfig.globalData.socketSn;
}



void USER_FUNC setDeviceConnectInfo(DEVICE_CONN_TYPE connType, BOOL value)
{
	switch (connType)
	{
	case STA_CONN_BIT:
		g_deviceConfig.globalData.connInfo.staConn = value;
		break;

	case DHPC_OK_BIT:
		g_deviceConfig.globalData.connInfo.dhcpOK = value;
		break;

	case BALANCE_CONN_BIT:
		g_deviceConfig.globalData.connInfo.balanceOK = value;
		break;

	case SERVER_ADDR_BIT:
		g_deviceConfig.globalData.connInfo.serverAdd = value;
		break;

	case SERVER_CONN_BIT:
		g_deviceConfig.globalData.connInfo.serverConn = value;
		break;

	default:
		break;
	}
}



BOOL USER_FUNC getDeviceConnectInfo(DEVICE_CONN_TYPE connType)
{
	BOOL ret;


	switch (connType)
	{
	case STA_CONN_BIT:
		ret = g_deviceConfig.globalData.connInfo.staConn;
		break;

	case DHPC_OK_BIT:
		ret = g_deviceConfig.globalData.connInfo.dhcpOK;
		break;

	case BALANCE_CONN_BIT:
		ret = g_deviceConfig.globalData.connInfo.balanceOK;
		break;

	case SERVER_ADDR_BIT:
		ret = g_deviceConfig.globalData.connInfo.serverAdd;
		break;

	case SERVER_CONN_BIT:
		ret = g_deviceConfig.globalData.connInfo.serverConn;
		break;

	default:
		ret = FALSE;
		break;
	}

	return ret;
}


void USER_FUNC setServerAddr(SOCKET_ADDR* pSocketAddr)
{
	memcpy(&g_deviceConfig.globalData.tcpServerAddr, pSocketAddr, sizeof(SOCKET_ADDR));
}


void USER_FUNC getServerAddr(SOCKET_ADDR* pSocketAddr)
{
	memcpy(pSocketAddr, &g_deviceConfig.globalData.tcpServerAddr, sizeof(SOCKET_ADDR));
}


U8* USER_FUNC mallocSocketData(size_t size)
{
	U8* ptData = NULL;

	ptData = hfmem_malloc(size + 1);
	if(ptData == NULL)
	{
		HF_Debug(DEBUG_ERROR, "meiyusong===> Malloc mallocSocketData \n");
		return NULL;
	}
	else
	{
		g_deviceConfig.globalData.mallocCount++;
		if(g_deviceConfig.globalData.mallocCount >= 0xFFFF)
		{
			g_deviceConfig.globalData.mallocCount = 100;
		}
	}
	memset(ptData, 0, size+1);
	//u_printf("meiyusong===> malloc mallocCount = %d size=%d\n", g_deviceConfig.globalData.mallocCount, size);
	return ptData;
}


void USER_FUNC FreeSocketData(U8* ptData)
{
	hfmem_free(ptData);
	if(g_deviceConfig.globalData.mallocCount == 0)
	{
		HF_Debug(DEBUG_ERROR, "meiyusong===> g_deviceConfig.globalData.mallocCount < 0 \n");
	}
	g_deviceConfig.globalData.mallocCount--;

	//HF_Debug(DEBUG_ERROR, "meiyusong===> free mallocCount = %d \n", g_deviceConfig.globalData.mallocCount);
}



static void USER_FUNC saveDeviceConfigData(void)
{
	hffile_userbin_write(DEVICE_CONFIG_OFFSET_START, (char*)(&g_deviceConfig.deviceConfigData), DEVICE_CONFIG_SIZE);
}


void USER_FUNC setDeviceName(DEVICE_NAME_DATA* nameData)
{
	memcpy(&g_deviceConfig.deviceConfigData.deviceName, nameData, sizeof(DEVICE_NAME_DATA));
	saveDeviceConfigData();
}


DEVICE_NAME_DATA* USER_FUNC getDeviceName(void)
{
	return &g_deviceConfig.deviceConfigData.deviceName;
}


void USER_FUNC setAlarmData(ALARM_DATA_INFO* alarmData, U8 index)
{
	if(index >= MAX_ALARM_COUNT)
	{
		return;
	}

	memcpy(&g_deviceConfig.deviceConfigData.alarmData[index], alarmData, sizeof(ALARM_DATA_INFO));
	saveDeviceConfigData();


	u_printf("meiyusong===>AlarmData m=%d T=%d W=%d T=%d F=%d S=%d Sun=%d active=%d hour=%d, minute=%d action=%d size=%d\n",
	         g_deviceConfig.deviceConfigData.alarmData[index].repeatData.monday,
	         g_deviceConfig.deviceConfigData.alarmData[index].repeatData.tuesday,
	         g_deviceConfig.deviceConfigData.alarmData[index].repeatData.wednesday,
	         g_deviceConfig.deviceConfigData.alarmData[index].repeatData.thursday,
	         g_deviceConfig.deviceConfigData.alarmData[index].repeatData.firday,
	         g_deviceConfig.deviceConfigData.alarmData[index].repeatData.saturday,
	         g_deviceConfig.deviceConfigData.alarmData[index].repeatData.sunday,
	         g_deviceConfig.deviceConfigData.alarmData[index].repeatData.bActive,
	         g_deviceConfig.deviceConfigData.alarmData[index].hourData,
	         g_deviceConfig.deviceConfigData.alarmData[index].minuteData,
	         g_deviceConfig.deviceConfigData.alarmData[index].action,
	         sizeof(ALARM_DATA_INFO));
}


void USER_FUNC deleteAlarmData(U8 index)
{
	U8 i;

	if(index >= MAX_ALARM_COUNT)
	{
		return;
	}

	for(i=index; i<MAX_ALARM_COUNT; i++)
	{
		if(i == (MAX_ALARM_COUNT - 1) || g_deviceConfig.deviceConfigData.alarmData[i+1].hourData == 0xFF)
		{
			memset(&g_deviceConfig.deviceConfigData.alarmData[i], 0, sizeof(ALARM_DATA_INFO));
			g_deviceConfig.deviceConfigData.alarmData[i].hourData = 0xFF;
			g_deviceConfig.deviceConfigData.alarmData[i].minuteData= 0xFF;
			break;
		}
		else
		{
			memcpy(&g_deviceConfig.deviceConfigData.alarmData[i], &g_deviceConfig.deviceConfigData.alarmData[i+1], sizeof(ALARM_DATA_INFO));
		}
	}
	saveDeviceConfigData();
}



ALARM_DATA_INFO* USER_FUNC getAlarmData(U8 index)
{
	if(index >= MAX_ALARM_COUNT)
	{
		return NULL;
	}
	else
	{
		return &g_deviceConfig.deviceConfigData.alarmData[index];
	}
}



static void USER_FUNC initAlarmData(void)
{
	U8 i;


	for(i=0; i<MAX_ALARM_COUNT; i++)
	{
		g_deviceConfig.deviceConfigData.alarmData[i].hourData = 0xFF;
		g_deviceConfig.deviceConfigData.alarmData[i].minuteData= 0xFF;
	}
}



static void USER_FUNC initAbsenceData(void)
{
	U8 i;


	for(i=0; i<MAX_ABSENCE_COUNT; i++)
	{
		g_deviceConfig.deviceConfigData.absenceData[i].startHour = 0xFF;
	}
}



void USER_FUNC setAbsenceData(ASBENCE_DATA_INFO* absenceData, U8 index)
{
	if(index >= MAX_ABSENCE_COUNT)
	{
		return;
	}
	memcpy(&g_deviceConfig.deviceConfigData.absenceData[index], absenceData, sizeof(ASBENCE_DATA_INFO));
	saveDeviceConfigData();

	u_printf("meiyusong===>AbsenceData  m=%d T=%d W=%d T=%d F=%d S=%d Sun=%d active=%d Shour=%d, Sminute=%d Ehour=%d, Eminute=%d time=%d size=%d\n",
	         g_deviceConfig.deviceConfigData.absenceData[index].repeatData.monday,
	         g_deviceConfig.deviceConfigData.absenceData[index].repeatData.tuesday,
	         g_deviceConfig.deviceConfigData.absenceData[index].repeatData.wednesday,
	         g_deviceConfig.deviceConfigData.absenceData[index].repeatData.thursday,
	         g_deviceConfig.deviceConfigData.absenceData[index].repeatData.firday,
	         g_deviceConfig.deviceConfigData.absenceData[index].repeatData.saturday,
	         g_deviceConfig.deviceConfigData.absenceData[index].repeatData.sunday,
	         g_deviceConfig.deviceConfigData.absenceData[index].repeatData.bActive,
	         g_deviceConfig.deviceConfigData.absenceData[index].startHour,
	         g_deviceConfig.deviceConfigData.absenceData[index].startMinute,
	         g_deviceConfig.deviceConfigData.absenceData[index].endHour,
	         g_deviceConfig.deviceConfigData.absenceData[index].endMinute,
	         g_deviceConfig.deviceConfigData.absenceData[index].timeData,
	         sizeof(ASBENCE_DATA_INFO));

}



void USER_FUNC deleteAbsenceData(U8 index)
{
	U8 i;

	if(index >= MAX_ABSENCE_COUNT)
	{
		return;
	}

	for(i=index; i<MAX_ABSENCE_COUNT; i++)
	{
		if(i == (MAX_ABSENCE_COUNT - 1) || g_deviceConfig.deviceConfigData.absenceData[i+1].startHour == 0xFF)
		{
			memset(&g_deviceConfig.deviceConfigData.absenceData[i], 0, sizeof(ASBENCE_DATA_INFO));
			g_deviceConfig.deviceConfigData.absenceData[i].startHour= 0xFF;
			break;
		}
		else
		{
			memcpy(&g_deviceConfig.deviceConfigData.absenceData[i], &g_deviceConfig.deviceConfigData.absenceData[i+1], sizeof(ASBENCE_DATA_INFO));
		}
	}
	saveDeviceConfigData();
}




ASBENCE_DATA_INFO* USER_FUNC getAbsenceData(U8 index)
{
	if(index >= MAX_ABSENCE_COUNT)
	{
		return NULL;
	}
	else
	{
		return &g_deviceConfig.deviceConfigData.absenceData[index];
	}
}



static void USER_FUNC initCountDownData(void)
{
	U8 i;

	for(i=0; i<MAX_COUNTDOWN_COUNT; i++)
	{
		g_deviceConfig.deviceConfigData.countDownData[0].count = 0;
	}
}



void USER_FUNC setCountDownData(COUNTDOWN_DATA_INFO* countDownData, U8 index)
{
	if(index >= MAX_COUNTDOWN_COUNT)
	{
		return;
	}

	memcpy(&g_deviceConfig.deviceConfigData.countDownData[index], countDownData, sizeof(COUNTDOWN_DATA_INFO));
	saveDeviceConfigData();

	u_printf("meiyusong==> countDownData active=%d, action=%d, count=%0xX\n", countDownData->flag.bActive,
	         countDownData->action, countDownData->count);
}



void USER_FUNC deleteCountDownData(U8 index)
{
	U8 i;

	if(index >= MAX_COUNTDOWN_COUNT)
	{
		return;
	}

	for(i=index; i<MAX_COUNTDOWN_COUNT; i++)
	{
		if(i == (MAX_COUNTDOWN_COUNT - 1) || g_deviceConfig.deviceConfigData.countDownData[i+1].count == 0)
		{
			memset(&g_deviceConfig.deviceConfigData.countDownData[i], 0, sizeof(COUNTDOWN_DATA_INFO));
			g_deviceConfig.deviceConfigData.countDownData[i].count= 0;
			break;
		}
		else
		{
			memcpy(&g_deviceConfig.deviceConfigData.countDownData[i], &g_deviceConfig.deviceConfigData.countDownData[i+1], sizeof(COUNTDOWN_DATA_INFO));
		}
	}
	saveDeviceConfigData();
}



COUNTDOWN_DATA_INFO* USER_FUNC getCountDownData(U8 index)
{
	if(index >= MAX_COUNTDOWN_COUNT)
	{
		return NULL;
	}
	else
	{
		return &g_deviceConfig.deviceConfigData.countDownData[index];
	}
}



static void USER_FUNC globalConfigDataInit(void)
{
	memset(&g_deviceConfig, 0, sizeof(GLOBAL_CONFIG_DATA));
	hffile_userbin_read(DEVICE_CONFIG_OFFSET_START, (char*)(&g_deviceConfig.deviceConfigData), DEVICE_CONFIG_SIZE);
	if(g_deviceConfig.deviceConfigData.lumitekFlag != LUMITEK_SW_FLAG)
	{
		U8 defaultNameLen = strlen(DEFAULT_MODUAL_NAME);

		//Device  first power on flag
		memset(&g_deviceConfig, 0, sizeof(GLOBAL_CONFIG_DATA));
		g_deviceConfig.deviceConfigData.lumitekFlag = LUMITEK_SW_FLAG;

		//Device name init
		memcpy(g_deviceConfig.deviceConfigData.deviceName.nameData, DEFAULT_MODUAL_NAME, defaultNameLen);
		g_deviceConfig.deviceConfigData.deviceName.nameLen = defaultNameLen;

		initAlarmData();
		initAbsenceData();
		saveDeviceConfigData();
		initCountDownData();
	}
}



GLOBAL_CONFIG_DATA* USER_FUNC getGlobalConfigData(void)
{
	return &g_deviceConfig;
}


void USER_FUNC changeDeviceSwVersion(U8 swVersion)
{
	if(g_deviceConfig.deviceConfigData.swVersion!= swVersion)
	{
		g_deviceConfig.deviceConfigData.swVersion= swVersion;
		saveDeviceConfigData();
	}
}


U8 USER_FUNC getDeviceSwVersion(void)
{
	return g_deviceConfig.deviceConfigData.swVersion;
}



void USER_FUNC changeDeviceLockedStatus(BOOL bLocked)
{
	if(g_deviceConfig.deviceConfigData.bLocked != bLocked)
	{
#if 0
		g_deviceConfig.deviceConfigData.bLocked = (bLocked?1:0);
		saveDeviceConfigData();
#endif
		u_printf("meiyusong===> LOCK Device\n");
	}
}


U8 USER_FUNC getDeviceLockedStatus(void)
{
	return g_deviceConfig.deviceConfigData.bLocked;
}



BOOL USER_FUNC checkSmartlinkStatus(void)
{
	S32	start_reason = hfsys_get_reset_reason();
	BOOL ret = FALSE;


	if(start_reason&HFSYS_RESET_REASON_SMARTLINK_START)
	{
		hftimer_handle_t smartlinkTimer;


		globalConfigDataInit();
		changeDeviceLockedStatus(FALSE);

		if((smartlinkTimer = hftimer_create("SMARTLINK_TIMER", 300, true, SMARTLINK_TIMER_ID, smartlinkTimerCallback, 0)) == NULL)
		{

			u_printf("create smartlinkTimer fail\n");
		}
		else
		{
			hftimer_start(smartlinkTimer);
			u_printf("meiyusong===> go into SmartLink time = %d\n", time(NULL));
		}
		ret = TRUE;
	}

	return ret;
}




static U8 USER_FUNC macAtoi(S8 c)
{
	U8 ret = 0;


	if(c >= 'A' && c <= 'F')
	{
		ret = c - 'A' + 10;
	}
	else if(c >= 'a' && c <= 'f')
	{
		ret = c - 'a' + 10;
	}
	else if(c >= '0' && c <= '9')
	{
		ret = c - '0';
	}
	return ret;
}


static BOOL USER_FUNC readDeviceMacAddr(void)
{
	S8 *words[3]= {NULL};
	S8 rsp[64]= {0};
	BOOL ret = FALSE;
	U8 macAddr[DEVICE_MAC_LEN+1];
	U8 i;


	memset(macAddr, 0, sizeof(macAddr));
	hfat_send_cmd("AT+WSMAC\r\n",sizeof("AT+WSMAC\r\n"),rsp,64);

	if(hfat_get_words(rsp,words, 2)>0)
	{
		if((rsp[0]=='+')&&(rsp[1]=='o')&&(rsp[2]=='k'))
		{
			u_printf("meiyusong===> sting mac = %s\n", words[1]);
			for(i=0; i<DEVICE_MAC_LEN; i++)
			{
				macAddr[i] = macAtoi(words[1][(i<<1)]);
				macAddr[i] = (macAddr[i]<<4) + macAtoi(words[1][(i<<1)+1]);
				u_printf("meiyusong===> int mac[%d] = %X\n", i, macAddr[i]);
			}
			ret = TRUE;
		}
	}
	memcpy(g_deviceConfig.globalData.macAddr, macAddr, DEVICE_MAC_LEN);
	return ret;
}


U8* USER_FUNC getDeviceMacAddr(U8* devMac)
{
	if(devMac != NULL)
	{
		memcpy(devMac, g_deviceConfig.globalData.macAddr, DEVICE_MAC_LEN);
	}
	return g_deviceConfig.globalData.macAddr;
}


static void USER_FUNC CreateLocalAesKey(void)
{
#if 0
	U8 i;
	U8 index = 0;

	for(i=0; i<DEVICE_MAC_LEN; i++)
	{
		if(i < (DEVICE_MAC_LEN-1))
		{
			aesKey[index++] = ((deviceMac[i]&0x2f)>>2);
			aesKey[index++] = ((deviceMac[i]&0x78)>>3);
			aesKey[index++] = ((deviceMac[i]&0x1E)>>1);
		}
		else
		{
			aesKey[index++] = ((deviceMac[i]&0x2f)>>2);
		}
	}
#else
	g_deviceConfig.globalData.keyData.localAesKeyValid = TRUE;
	memcpy(g_deviceConfig.globalData.keyData.localKey, DEFAULT_AES_KEY, AES_KEY_LEN);
#endif
}



BOOL USER_FUNC needRebackRecvSocket(U8* macAddr, BOOL bItself)
{
	U8 i;
	BOOL ret = FALSE;



	if(strncmp((const S8* )macAddr, (const S8* )g_deviceConfig.globalData.macAddr, DEVICE_MAC_LEN) == 0)
	{
		ret = TRUE;
	}
	else if(!bItself)
	{
		for(i=0; i<DEVICE_MAC_LEN; i++)
		{
			if(macAddr[i] != 0xFF)
			{
				break;
			}
		}

		if(i == DEVICE_MAC_LEN && g_deviceConfig.deviceConfigData.bLocked == 0)
		{
			ret = TRUE;
		}
	}
	if(!ret)
	{
		u_printf("meiyusong===> mac error reve_mac = %s\n", macAddrToString(macAddr, NULL));
	}
	return ret;
}



static BOOL USER_FUNC checkSocketData(S8* pData, S32 dataLen)
{
	SCOKET_HERADER_OUTSIDE* POutsideData = (SCOKET_HERADER_OUTSIDE*)pData;
	BOOL ret = FALSE;


	if((POutsideData->openData.dataLen + SOCKET_HEADER_OPEN_DATA_LEN) != dataLen)
	{
		HF_Debug(DEBUG_ERROR, "meiyusong===> checkSocketData socket data len Error \n");
	}
	else
	{
		ret = TRUE;
	}
	return ret;
}




BOOL USER_FUNC checkRecvSocketData(U32 recvCount, S8* recvBuf)
{
	BOOL ret = TRUE;


	if (recvCount < 10)
	{
		ret = FALSE;
	}
	else if (!checkSocketData(recvBuf, (S32)recvCount)) //check socket lenth
	{
		ret = FALSE;
	}
	else if(!needRebackRecvSocket((U8*)(recvBuf + SOCKET_MAC_ADDR_OFFSET), FALSE)) //check socket mac address
	{
		ret = FALSE;
	}
	return ret;
}



#ifdef LUMITEK_DEBUG_SWITCH

//Notice macString lenth must >= 18  		(2*6+5+1)
S8* USER_FUNC macAddrToString(U8* macAddr, S8*macString)
{
	static S8 tempMacAddr[20];
	S8* pMacAddr;

	memset(tempMacAddr, 0, sizeof(tempMacAddr));
	pMacAddr = (macString == NULL)?tempMacAddr:macString;
	sprintf(pMacAddr, "%02X-%02X-%02X-%02X-%02X-%02X", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
	return pMacAddr;
}



void USER_FUNC showHexData(S8* descript, U8* showData, U8 lenth)
{
	U8 i;
	S8 temData[250];
	U8 index = 0;


	memset(temData, 0, sizeof(temData));
	for(i=0; i<lenth; i++)
	{
		if(i == 0)
		{
			//do nothing
		}
		else if(i%8 == 0)
		{
			temData[index++] = ' ';
			temData[index++] = ' ';
		}
		else if(i%2 == 0)
		{
			temData[index++] = ',';
			temData[index++] = ' ';
		}
		sprintf(temData+index, "%02X", showData[i]);
		index += 2;
		if(index > 150)
		{
			break;
		}
	}
	if(descript != NULL)
	{
		u_printf("==>len=%d %s data=%s\n", lenth, descript, temData);
	}
	else
	{
		u_printf("==>len=%d data=%s\n", lenth, temData);
	}
}




void USER_FUNC printGlobalParaStatus(S8* discript)
{
	u_printf("meiyusong===>discript = %s lumitekFlag=0x%X bLocked=%d macAddr=%s\n", discript,
	         g_deviceConfig.deviceConfigData.lumitekFlag,
	         g_deviceConfig.deviceConfigData.bLocked,
	         macAddrToString(g_deviceConfig.globalData.macAddr, NULL));
}


void USER_FUNC debugShowSendData(MSG_ORIGIN msgOrigin, U8* pSocketData, U32 recvDataLen)
{
	U8* decrpytData;
	U32 dataLen = recvDataLen;


	decrpytData = encryptRecvSocketData(msgOrigin, pSocketData, &dataLen);
	if(decrpytData != NULL)
	{
		showHexData("After Des", decrpytData, dataLen);
		FreeSocketData(decrpytData);
	}
}


void USER_FUNC showSocketOutsideData(U8* pData)
{
	SCOKET_HERADER_OUTSIDE* pHearderData = (SCOKET_HERADER_OUTSIDE*)pData;


	u_printf("pv=%d, flag=0x%x, mac=%x-%x-%x-%x-%x-%x, len=%d  reserved=%d snIndex=0x%x, deviceType=0x%x, factoryCode=0x%x, licenseData=0x%x\n",
	         pHearderData->openData.pv,
	         pHearderData->openData.flag,
	         pHearderData->openData.mac[0],
	         pHearderData->openData.mac[1],
	         pHearderData->openData.mac[2],
	         pHearderData->openData.mac[3],
	         pHearderData->openData.mac[4],
	         pHearderData->openData.mac[5],
	         pHearderData->openData.dataLen,
	         pHearderData->reserved,
	         pHearderData->snIndex,
	         pHearderData->deviceType,
	         pHearderData->factoryCode,
	         pHearderData->licenseData);

}


U16 USER_FUNC getMallocCount(void)
{
	return g_deviceConfig.globalData.mallocCount;
}

#endif



//192.168.1.100 --->C4A80164
static void USER_FUNC coverIpToInt(S8* stringIP, U8* IntIP)
{
	U8 i = 0;
	U8 gapIndex = 0;

	memset(IntIP, 0, SOCKET_IP_LEN);
	while(1)
	{
		if(stringIP[i] == '.')
		{
			gapIndex++;
		}
		else if(stringIP[i] <= '9' && stringIP[i] >= '0')
		{
			IntIP[gapIndex] *= 10;
			IntIP[gapIndex] += stringIP[i] - '0';
		}
		else
		{
			break;
		}
		i++;
	}

	//u_printf(" coverIpToInt %d.%d.%d.%d \n", IntIP[0], IntIP[1], IntIP[2], IntIP[3]);
}




// AT+WANN
// +ok=DHCP,192.168.1.59,255.255.255.0,192.168.1.1
BOOL USER_FUNC getDeviceIPAddr(U8* ipAddr)
{
	char rsp[68]= {0};
	char *words[5]= {NULL};
	char wann_addr[20]= {0};
	BOOL ret = FALSE;



	hfat_send_cmd("AT+WANN\r\n",sizeof("AT+WANN\r\n"),rsp,68);
	if((rsp[0]!='+')&&(rsp[1]!='o')&&(rsp[2]!='k'))
	{
		memset(rsp, 0, 68);
		hfat_send_cmd("AT+WANN\r\n",sizeof("AT+WANN\r\n"),rsp,68);
	}
	if((rsp[0]=='+')&&(rsp[1]=='o')&&(rsp[2]=='k'))
	{
		if(hfat_get_words(rsp,words, 5)>0)
		{
			strcpy((char*)wann_addr,(char*)words[2]);
			//u_printf("===>IP=%s\n", wann_addr);
			if(strcmp(wann_addr,"0.0.0.0") != 0)
			{
				coverIpToInt(wann_addr, ipAddr);
				ret = TRUE;
			}
		}
	}
	return ret;
}



void USER_FUNC itoParaInit(void)
{
	globalConfigDataInit();
	readDeviceMacAddr();
	CreateLocalAesKey();
	keyGpioInit();
}



static void USER_FUNC PKCS5PaddingFillData(U8* inputData, U32* dataLen, AES_KEY_TYPE keyType)
{
	U8	fillData;

	if(keyType == AES_KEY_OPEN)
	{
		return;
	}
	fillData = AES_BLOCK_SIZE - (*dataLen%AES_BLOCK_SIZE);
	memset((inputData + *dataLen), fillData, fillData);
	*dataLen += fillData;
}



static void USER_FUNC PKCS5PaddingRemoveData(U8* inputData, U32* dataLen, AES_KEY_TYPE keyType)
{
	U8	removeData = inputData[*dataLen - 1];

	if(keyType == AES_KEY_OPEN)
	{
		return;
	}
	if(removeData != 0)
	{
		memset((inputData + *dataLen - removeData), 0, removeData);
	}
	else
	{
		HF_Debug(DEBUG_ERROR, "meiyusong===> PKCS5PaddingRemoveData Error \n");
	}
	*dataLen -= removeData;
}



AES_KEY_TYPE USER_FUNC getRecvSocketAesKeyType(MSG_ORIGIN msgOrigin, U8* pData)
{
	SOCKET_HEADER_OPEN* pOpenData = (SOCKET_HEADER_OPEN*)pData;


	return getSocketAesKeyType(msgOrigin, pOpenData->flag.bEncrypt);
}



AES_KEY_TYPE USER_FUNC getSocketAesKeyType(MSG_ORIGIN msgOrigin, U8 bEncrypt)
{
	AES_KEY_TYPE keyType = AES_KEY_OPEN ;

	if(bEncrypt == 0)
	{
		keyType = AES_KEY_OPEN;
	}
	else if(msgOrigin == MSG_FROM_UDP)
	{
		if(g_deviceConfig.globalData.keyData.localAesKeyValid)
		{
			keyType = AES_KEY_LOCAL;
		}
		else
		{
			keyType = AES_KEY_DEFAULT;
		}
	}
	else if(msgOrigin == MSG_FROM_TCP)
	{
		if(g_deviceConfig.globalData.keyData.serverAesKeyValid)
		{
			keyType = AES_KEY_SERVER;
		}
		else
		{
			keyType = AES_KEY_DEFAULT;
		}
	}
	return keyType;

}



BOOL USER_FUNC getAesKeyData(AES_KEY_TYPE keyType, U8* keyData)
{
	U8* tmpKeyData = NULL;
	BOOL needCopy = TRUE;


	if(keyType == AES_KEY_DEFAULT)
	{
		tmpKeyData = DEFAULT_AES_KEY;
	}
	else if(keyType == AES_KEY_LOCAL)
	{
		tmpKeyData = g_deviceConfig.globalData.keyData.localKey;
	}
	else if(keyType == AES_KEY_SERVER)
	{
		tmpKeyData = g_deviceConfig.globalData.keyData.serverKey;
	}
	else
	{
		needCopy = FALSE;
	}
	if(needCopy)
	{
		memcpy(keyData, tmpKeyData, AES_KEY_LEN);
	}
	return needCopy;
}



void USER_FUNC clearServerAesKey(BOOL clearAddr)
{
	g_deviceConfig.globalData.keyData.serverAesKeyValid = FALSE;
	memset(g_deviceConfig.globalData.keyData.serverKey, 0, AES_KEY_LEN);
	if(clearAddr)
	{
		g_deviceConfig.globalData.tcpServerAddr.ipAddr = INVALID_SERVER_ADDR;
		g_deviceConfig.globalData.tcpServerAddr.port = INVALID_SERVER_PORT;
	}
}


void USER_FUNC setServerAesKey(U8* serverKey)
{
	memcpy(g_deviceConfig.globalData.keyData.serverKey, serverKey, AES_KEY_LEN);
	g_deviceConfig.globalData.keyData.serverAesKeyValid = TRUE;
}


static BOOL USER_FUNC setAesKey(Aes* dec, AES_KEY_TYPE keyType, S32 aesType)
{
	U8 aesKey[AES_KEY_LEN + 1];
	BOOL ret = TRUE;


	memset(aesKey, 0, sizeof(aesKey));
	if(!getAesKeyData(keyType, aesKey))
	{
		ret = FALSE;
	}
	else
	{
		u_printf("meiyusong===> aeskey=%s keyType=%d\n", aesKey, keyType);
		AesSetKey(dec, (const byte *)aesKey, AES_BLOCK_SIZE, (const byte *)aesKey, aesType);
	}
	return ret;
}



BOOL USER_FUNC socketDataAesDecrypt(U8 *inData, U8* outData, U32* aesDataLen, AES_KEY_TYPE keyType)
{
	Aes dec;
	U32 dataLen = *aesDataLen;


	if(inData == NULL || outData == NULL)
	{
		HF_Debug(DEBUG_ERROR, "meiyusong===> socketDataAesDecrypt input data Error \n");
		return FALSE;
	}

	if(keyType == AES_KEY_OPEN)
	{
		memcpy(outData, inData, dataLen);

	}
	else
	{
		if(!setAesKey(&dec, keyType, AES_DECRYPTION))
		{
			HF_Debug(DEBUG_ERROR, "meiyusong===> Decrypt keyType Error keyType=%d \n", keyType);
			return FALSE;
		}
		AesCbcDecrypt(&dec, (byte *)(outData), (const byte *)inData, dataLen);
		PKCS5PaddingRemoveData(outData, aesDataLen, keyType);
	}
	return TRUE;
}



BOOL USER_FUNC socketDataAesEncrypt(U8 *inData, U8* outData, U32* aesDataLen, AES_KEY_TYPE keyType)
{
	Aes enc;


	if(inData == NULL || outData == NULL)
	{
		HF_Debug(DEBUG_ERROR, "meiyusong===> socketDataAesEncrypt input data Error \n");
		return FALSE;
	}
	if(keyType == AES_KEY_OPEN)
	{
		memcpy(outData, inData, *aesDataLen);
	}
	else
	{
		if(!setAesKey(&enc, keyType, AES_ENCRYPTION))
		{
			HF_Debug(DEBUG_ERROR, "meiyusong===> Encrypt keyType Error keyType=%d\n", keyType);
			return FALSE;
		}
		PKCS5PaddingFillData(inData, aesDataLen, keyType);
		AesCbcEncrypt(&enc, (byte *)outData , (const byte *)inData, *aesDataLen);
	}
	return TRUE;
}



static void USER_FUNC setSocketAesDataLen(U8* pData, U32 aesDataLen)
{
	SOCKET_HEADER_OPEN* pOpendata = (SOCKET_HEADER_OPEN*)pData;

	pOpendata->dataLen = aesDataLen;
}



static U8* USER_FUNC getEncryptDataBuf(U32 socketBodylen)
{
	U8* pData;
	U32 mallocLen;


	mallocLen = socketBodylen + SOCKET_HEADER_LEN + AES_BLOCK_SIZE + 1;
	pData = mallocSocketData(mallocLen);
	if(pData != NULL)
	{
		memset(pData, 0, mallocLen);
	}
	return pData;
}



U8* USER_FUNC createSendSocketData(CREATE_SOCKET_DATA* createData, U32* sendSocketLen)
{
	U8* originSocketBuf;
	SCOKET_HERADER_OUTSIDE* pSocketHeader;
	U8* pAesData;
	U32 aesDataLen;
	U8 openDataLen = SOCKET_HEADER_OPEN_DATA_LEN;


	originSocketBuf = getEncryptDataBuf(createData->bodyLen);
	if(originSocketBuf == NULL)
	{
		HF_Debug(DEBUG_ERROR, "meiyusong===> malloc Encrypt buf faild \n");
		return NULL;
	}
	pSocketHeader = (SCOKET_HERADER_OUTSIDE*)originSocketBuf;

	pSocketHeader->openData.pv = SOCKET_HEADER_PV;
	pSocketHeader->openData.flag.bEncrypt = createData->bEncrypt;
	pSocketHeader->openData.flag.bReback = createData->bReback;
	pSocketHeader->openData.flag.bLocked = g_deviceConfig.deviceConfigData.bLocked;
	getDeviceMacAddr(pSocketHeader->openData.mac);
	pSocketHeader->openData.dataLen = (createData->bodyLen + SOCKET_HEADER_SECRET_DATA_LEN); //aesDataLen

	pSocketHeader->reserved = SOCKET_HEADER_RESERVED;
	pSocketHeader->snIndex = htons(createData->snIndex);
	pSocketHeader->deviceType = SOCKET_HEADER_DEVICE_TYPE;
	pSocketHeader->factoryCode = SOCKET_HEADER_FACTORY_CODE;
	pSocketHeader->licenseData = SOCKET_HEADER_LICENSE_DATA;

	//fill body data
	memcpy((originSocketBuf + SOCKET_HEADER_LEN), createData->bodyData, createData->bodyLen);

	pAesData = mallocSocketData(SOCKET_HEADER_LEN + createData->bodyLen + AES_BLOCK_SIZE + 1);
	if(pAesData == NULL)
	{
		HF_Debug(DEBUG_ERROR, "meiyusong===> createSendSocketData mallic faild \n");
		FreeSocketData(originSocketBuf);
		return NULL;
	}
	memcpy(pAesData, originSocketBuf, openDataLen);
	aesDataLen = pSocketHeader->openData.dataLen;
	showHexData("before aes", originSocketBuf, (createData->bodyLen + SOCKET_HEADER_LEN));
	if(socketDataAesEncrypt((originSocketBuf + openDataLen), (pAesData + openDataLen), &aesDataLen, createData->keyType))
	{
		*sendSocketLen = aesDataLen + openDataLen;
		setSocketAesDataLen(pAesData, aesDataLen);
		//showHexData("After aes", pAesData, *sendSocketLen);
		//debugShowSendData(MSG_FROM_UDP, pAesData, *sendSocketLen);
	}
	else
	{
		FreeSocketData(pAesData);
		pAesData = NULL;
		*sendSocketLen = 0;
	}
	FreeSocketData(originSocketBuf);
	return pAesData;
}




U8* USER_FUNC encryptRecvSocketData(MSG_ORIGIN msgOrigin, U8* pSocketData, U32* recvDataLen)
{
	U8* pData;
	U8 openDataLen = SOCKET_HEADER_OPEN_DATA_LEN;
	U32 dataLen = *recvDataLen;
	U32 asDataLen = *recvDataLen - openDataLen;
	AES_KEY_TYPE keyType;


	if(pSocketData == NULL || dataLen < 10)
	{
		return NULL;
	}
	pData = mallocSocketData(dataLen);
	if(pData == NULL)
	{
		return NULL;
	}

	keyType = getRecvSocketAesKeyType(msgOrigin, pSocketData);
	memcpy(pData, pSocketData, openDataLen);

	if(socketDataAesDecrypt((pSocketData + openDataLen), (pData + openDataLen), &asDataLen, keyType))
	{
		SCOKET_HERADER_OUTSIDE* pTmpData = (SCOKET_HERADER_OUTSIDE*)pData;

		*recvDataLen = asDataLen + openDataLen;
		pTmpData->snIndex = ntohs(pTmpData->snIndex);
		pTmpData->openData.dataLen = asDataLen;
		return pData;
	}
	else
	{
		FreeSocketData(pData);
		return NULL;
	}
}

#endif


