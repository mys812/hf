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



static S8 g_socket_recv_buf[NETWORK_MAXRECV_LEN];
static S8 g_socket_send_buf[NETWORK_MAXRECV_LEN];

hfthread_mutex_t g_socket_mutex;
static GLOBAL_CONFIG_DATA g_deviceConfig;


S8* USER_FUNC getSocketRecvBuf(BOOL setZero)
{
    if(setZero)
    {
        memset(g_socket_recv_buf, 0, NETWORK_MAXRECV_LEN);
    }
    return g_socket_recv_buf;
}


S8* USER_FUNC getSocketSendBuf(BOOL setZero)
{
    if(setZero)
    {
        memset(g_socket_send_buf, 0, NETWORK_MAXRECV_LEN);
    }
    return g_socket_send_buf;
}



hfthread_mutex_t USER_FUNC getSocketMutex(void)
{
    return g_socket_mutex;
}


void USER_FUNC setSocketMutex(hfthread_mutex_t socketMutex)
{
    g_socket_mutex = socketMutex;
}


U16 USER_FUNC getSocketSn(void)
{
    return g_deviceConfig.globalData.socketSn;
}


void USER_FUNC socketSnIncrease(void)
{
    g_deviceConfig.globalData.socketSn++;
    if(g_deviceConfig.globalData.socketSn >= 0xFFFF)
    {
        g_deviceConfig.globalData.socketSn = 0;
    }
}


U8* USER_FUNC mallocSocketData(size_t size)
{
    U8* ptData = NULL;

    ptData = hfmem_malloc(size);
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
}



static void USER_FUNC deviceConfigDataInit(void)
{
    memset(&g_deviceConfig, 0, DEVICE_CONFIG_SIZE);
    hffile_userbin_read(DEVICE_CONFIG_OFFSET_START, (char*)(&g_deviceConfig.deviceConfigData), DEVICE_CONFIG_SIZE);
}


void USER_FUNC saveDeviceConfigData(void)
{
    hffile_userbin_write(DEVICE_CONFIG_OFFSET_START, (char*)(&g_deviceConfig.deviceConfigData), DEVICE_CONFIG_SIZE);
}


void USER_FUNC getDeviceConfigData(DEVICE_CONFIG_DATA* configData)
{
    memcpy(configData, &g_deviceConfig.deviceConfigData, DEVICE_CONFIG_SIZE);
}


void USER_FUNC changeDeviceSwFlag(U16 swFlag)
{
    if(g_deviceConfig.deviceConfigData.swFlag!= swFlag)
    {
        g_deviceConfig.deviceConfigData.swFlag = swFlag;
        saveDeviceConfigData();
    }
}


U16 USER_FUNC getDeviceSwFlag(void)
{
    return g_deviceConfig.deviceConfigData.swFlag;
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
        g_deviceConfig.deviceConfigData.bLocked = (bLocked?1:0);
        saveDeviceConfigData();
    }
}


U8 USER_FUNC getDeviceLockedStatus(void)
{
    return g_deviceConfig.deviceConfigData.bLocked;
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




void USER_FUNC getLocalAesKeyByMac(U8* deviceMac, U8* aesKey)
{
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
}




BOOL USER_FUNC readDeviceMacAddr(void)
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
                //u_printf("meiyusong===> int mac[%d] = %X\n", i, macAddr[i]);
            }
            ret = TRUE;
        }
    }
    memcpy(g_deviceConfig.globalData.macAddr, macAddr, DEVICE_MAC_LEN);
    return ret;
}



//Notice macString lenth must >= 18  		(2*6+5+1)
void USER_FUNC macAddrToString(U8* macAddr, S8*macString)
{
	sprintf(macString, "%X-%X-%X-%X-%X-%X", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}


void USER_FUNC itoParaInit(void)
{
    hfthread_mutex_t socketMutex;


    deviceConfigDataInit();
    readDeviceMacAddr();
    //init mutex
    if((hfthread_mutext_new(&socketMutex)!= HF_SUCCESS))
    {
        HF_Debug(DEBUG_ERROR, "failed to create socketMutex");

    }
    else
    {
        setSocketMutex(socketMutex);
    }

    KeyGpioInit();
}





void USER_FUNC PKCS5PaddingFillData(S8* inputData, U32* dataLen)
{
	U8	fillData;
	U8	encryptDataLen = *dataLen - SOCKET_HEADER_OPEN_DATA_LEN;

	fillData = AES_BLOCK_SIZE - (encryptDataLen%AES_BLOCK_SIZE);	
	memset((inputData + *dataLen), fillData, fillData);
	*dataLen += fillData;	
}


void USER_FUNC PKCS5PaddingRemoveData(S8* inputData, U32* dataLen)
{
	U8	removeData = inputData[*dataLen - 1];

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



U8* USER_FUNC socketDataAesDecrypt(S8 *data, U32 len, AES_KEY_TYPE keyType)
{
    S8 *cipher = data + SOCKET_HEADER_OPEN_DATA_LEN;
    U32 dataLen = len - SOCKET_HEADER_OPEN_DATA_LEN;
    //Aes enc;
    Aes dec;
    U8* pDecryptData = NULL;



    if(dataLen <= 0)
    {
        HF_Debug(DEBUG_ERROR, "meiyusong===> Decrypt input data Error \n");
        return NULL;
    }

    pDecryptData = mallocSocketData(len+1);
    if(pDecryptData == NULL)
    {
        return NULL;
    }

    //u_printf("meiyusong===> headerDataLen = %d, pDecryptData=0x%x, offset offset=0x%x\n", headerDataLen, pDecryptData, (pDecryptData + headerDataLen));
    if(keyType == AES_KEY_OPEN)
    {
        memcpy(pDecryptData, data, len);

    }
    else
    {
        if(keyType == AES_KEY_DEFAULT)
        {
            AesSetKey(&dec, (const byte *)(AES_KEY), AES_BLOCK_SIZE, (const byte *)(AES_IV), AES_DECRYPTION);
        }
        else if(keyType == AES_KEY_LOCAL)
        {
            AesSetKey(&dec, (const byte *)(g_deviceConfig.globalData.keyData.localKey), AES_BLOCK_SIZE,
                      (const byte *)(g_deviceConfig.globalData.keyData.localKey), AES_DECRYPTION);
        }
        else if(keyType == AES_KEY_SERVER)
        {
            AesSetKey(&dec, (const byte *)(g_deviceConfig.globalData.keyData.serverKey), AES_BLOCK_SIZE,
                      (const byte *)(g_deviceConfig.globalData.keyData.serverKey), AES_DECRYPTION);
        }
        else
        {
            FreeSocketData(pDecryptData);
            HF_Debug(DEBUG_ERROR, "meiyusong===> Decrypt keyType Error \n");
            return NULL;
        }
        AesCbcDecrypt(&dec, (byte *)(pDecryptData + SOCKET_HEADER_OPEN_DATA_LEN), (const byte *)cipher, dataLen);
        memcpy(pDecryptData, data, SOCKET_HEADER_OPEN_DATA_LEN);
    }

    return pDecryptData;
}




S32 USER_FUNC socketDataAesEncrypt(S8 *dataIn, U32 len, AES_KEY_TYPE keyType)
{
    S8 *plain= dataIn + SOCKET_HEADER_OPEN_DATA_LEN;
    U32 dataLen= len - SOCKET_HEADER_OPEN_DATA_LEN;
    S8* pDecryptData = getSocketSendBuf(TRUE);
    Aes enc;



    if(dataIn == NULL)
    {
        return -1;
    }
    if(keyType == AES_KEY_OPEN)
    {
        memcpy(pDecryptData, dataIn, len);
    }
    else
    {
        if(keyType == AES_KEY_DEFAULT)
        {
            AesSetKey(&enc, (const byte *)(AES_KEY), AES_BLOCK_SIZE, (const byte *)(AES_IV), AES_ENCRYPTION);
        }
        else if(keyType == AES_KEY_LOCAL)
        {
            AesSetKey(&enc, (const byte *)(g_deviceConfig.globalData.keyData.localKey), AES_BLOCK_SIZE,
                      (const byte *)(g_deviceConfig.globalData.keyData.localKey), AES_ENCRYPTION);
        }
        else if(keyType == AES_KEY_SERVER)
        {
            AesSetKey(&enc, (const byte *)(g_deviceConfig.globalData.keyData.serverKey), AES_BLOCK_SIZE,
                      (const byte *)(g_deviceConfig.globalData.keyData.serverKey), AES_ENCRYPTION);
        }
        else
        {
            HF_Debug(DEBUG_ERROR, "meiyusong===> Encrypt keyType Error \n");
            return -1;
        }
       // dataLen = ((dataLen + AES_BLOCK_SIZE - 1)/AES_BLOCK_SIZE)*AES_BLOCK_SIZE;
        AesCbcEncrypt(&enc, (byte *)(pDecryptData + SOCKET_HEADER_OPEN_DATA_LEN), (const byte *)plain, dataLen);
        memcpy(pDecryptData, dataIn, SOCKET_HEADER_OPEN_DATA_LEN);
    }
    return (dataLen + SOCKET_HEADER_OPEN_DATA_LEN);
}



void USER_FUNC setSocketHeaderOutsideData(SCOKET_HERADER_OUTSIDE* outsideData, BOOL bReback, U8 encryptDataLen, BOOL needEncrypt, U16 snIndex)
{
	SCOKET_HERADER_OUTSIDE tmpData;

	memset(&tmpData, 0, sizeof(SCOKET_HERADER_OUTSIDE));
	tmpData.openData.pv = SOCKET_HEADER_PV;
	tmpData.openData.flag.bEncrypt = needEncrypt?1:0;
	tmpData.openData.flag.bLocked = g_deviceConfig.deviceConfigData.bLocked;
	tmpData.openData.flag.bReback = bReback?1:0;
	memcpy(tmpData.openData.mac, g_deviceConfig.globalData.macAddr, DEVICE_MAC_LEN);
	tmpData.openData.dataLen = encryptDataLen;

	tmpData.secretData.reserved = SOCKET_HEADER_RESERVED;
	if(bReback)
	{
		socketSnIncrease();
	}
	tmpData.secretData.snIndex = bReback?snIndex:g_deviceConfig.globalData.socketSn;
	tmpData.secretData.deviceType = SOCKET_HEADER_DEVICE_TYPE;
	tmpData.secretData.factoryCode = SOCKET_HEADER_FACTORY_CODE;
	tmpData.secretData.licenseData = SOCKET_HEADER_LICENSE_DATA;

	memcpy(outsideData, &tmpData, sizeof(SCOKET_HERADER_OUTSIDE));
}

#endif


