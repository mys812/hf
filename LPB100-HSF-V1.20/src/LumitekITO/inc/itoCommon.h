/*
******************************
*Company:Lumitek
*Data:2014-10-07
*Author:Meiyusong
******************************
*/

#ifndef __ITO_COMMON_H__
#define __ITO_COMMON_H__


typedef unsigned int		portTickType;  //def for time

typedef   char		S8;
typedef   short int	S16;
typedef   int		S32;


typedef unsigned int U32;
typedef unsigned short U16;
typedef unsigned char U8;

typedef unsigned char BOOL;


#ifndef NULL
#define NULL 0
#endif

//socket port define
#define UDP_SOCKET_PORT		18530
#define TCP_SOCKET_PORT		17531

//aes key define
#define AES_KEY		"1234567890abcdef"
#define AES_IV		"1234567890abcdef"
#define AES_KEY_LEN			16


//GPIO define
#define HFGPIO_F_SMARTLINK           (HFGPIO_F_USER_DEFINE+0)
#define HFGPIO_F_SWITCH				 (HFGPIO_F_USER_DEFINE+1)
#define HFGPIO_F_LIGHT				 (HFGPIO_F_USER_DEFINE+2)

//timer id define
#define SMARTLINK_TIMER_ID      (1)

//device save data define
#define DEVICE_CONFIG_OFFSET_START 0x00
#define DEVICE_CONFIG_SIZE (sizeof(DEVICE_CONFIG_DATA))

//socket result define
#define OPT_RESULT_SUCCESS	0x00
#define OPT_RESULT_FAILD	0x00

//socket header data
#define SOCKET_HEADER_PV			0x01
#define SOCKET_HEADER_RESERVED		0x00
#define SOCKET_HEADER_DEVICE_TYPE	0xD1
#define SOCKET_HEADER_FACTORY_CODE	0xF1
#define SOCKET_HEADER_LICENSE_DATA	0xB421	//Correct data is  0x21B4

//sw flag
#define LUMITEK_SW_FLAG					0xDCBA

//other data define define
#define MAX_SOCKEY_DATA_LEN				256
#define SOCKET_HEADER_OPEN_DATA_LEN		sizeof(SOCKET_HEADER_OPEN)
#define SOCKET_HEADER_SECRET_DATA_LEN	(SOCKET_HEADER_LEN - SOCKET_HEADER_OPEN_DATA_LEN)
#define NETWORK_MAXRECV_LEN				(MAX_SOCKEY_DATA_LEN + SOCKET_HEADER_OPEN_DATA_LEN + 1)
#define SOCKET_HEADER_LEN				sizeof(SCOKET_HERADER_OUTSIDE)
#define DEVICE_MAC_LEN					6
#define SOCKET_IP_LEN					4


typedef enum
{
	MSG_LOCAL_EVENT	= 0,
	MSG_TO_UDP		= 1,
	MSG_FROM_UDP	= 2,
	MSG_TO_TCP		= 3,
	MSG_FROM_TCP	= 4
}MSG_ORIGIN;



typedef enum
{
    AES_KEY_DEFAULT,
    AES_KEY_LOCAL,
    AES_KEY_SERVER,
    AES_KEY_OPEN
} AES_KEY_TYPE;


typedef struct
{
    U8	serverKey[AES_KEY_LEN];
    U8	localKey[AES_KEY_LEN];
} AES_KEY_DATA;


typedef struct
{
	U16 lumitekFlag;
    U16	swFlag;		//Used for check lumitek sw type
    U8	swVersion;	//Used for upgrade check
    U8	bLocked;	//used for check device be locked
    U8	reserved[100];

} DEVICE_CONFIG_DATA;


typedef struct
{
    U8	macAddr[DEVICE_MAC_LEN];
    BOOL localAesKeyValid;
    BOOL serverAesKeyValid;
    AES_KEY_DATA	keyData;
    U16 mallocCount;
    U16 socketSn;
} GLOBAL_RUN_DATA;


typedef struct
{
    DEVICE_CONFIG_DATA deviceConfigData;
    GLOBAL_RUN_DATA		globalData;

} GLOBAL_CONFIG_DATA;


typedef struct
{
    U8	reserved0:1;
    U8	bReback:1;
    U8	bLocked:1;
    U8	reserved3:1;
    U8	reserved4:1;
    U8	reserved5:1;
    U8	bEncrypt:1;
    U8	reserved7:1;

} SOCKET_HEADER_FLAG;


typedef struct
{
    U8	pv;
    SOCKET_HEADER_FLAG	flag;
    U8	mac[DEVICE_MAC_LEN];
    U8	dataLen;
} SOCKET_HEADER_OPEN;


#if 0
typedef struct
{
    U8	reserved;
    U16	snIndex;
    U8	deviceType;
    U8	factoryCode;
    U16	licenseData;
} SOCKET_HEADER_SECRET;

typedef struct
{
    SOCKET_HEADER_OPEN openData;
    SOCKET_HEADER_SECRET secretData;
} SCOKET_HERADER_OUTSIDE;

#else

//struct size is mutiply of  item's max len
typedef struct
{
    SOCKET_HEADER_OPEN openData;
    U8	reserved;
    U16	snIndex;
    U8	deviceType;
    U8	factoryCode;
    U16	licenseData;

} SCOKET_HERADER_OUTSIDE;
#endif

typedef struct
{
    SCOKET_HERADER_OUTSIDE	outsideData;
    S8*	insideData;
} SOCKET_HEADER_DATA;



typedef struct
{
	U8 bEncrypt;
	U8 bReback;
	U16 snIndex;
	U16 bodyLen;
	AES_KEY_TYPE keyType;
	U8* bodyData;	
} CREATE_SOCKET_DATA;

S8* USER_FUNC getSocketRecvBuf(BOOL setZero);
S8* USER_FUNC getSocketOriginBuf(BOOL setZero);

hfthread_mutex_t USER_FUNC getSocketMutex(void);
void USER_FUNC setSocketMutex(hfthread_mutex_t socketMutex);

U16 USER_FUNC getSocketSn(BOOL needIncrease);

void USER_FUNC changeDeviceLockedStatus(BOOL bLocked);
U8 USER_FUNC getDeviceLockedStatus(void);

void USER_FUNC changeDeviceSwFlag(U16 swFlag);
U16 USER_FUNC getDeviceSwFlag(void);

void USER_FUNC changeDeviceSwVersion(U8 swVersion);
U8 USER_FUNC getDeviceSwVersion(void);

void USER_FUNC getDeviceMacAddr(U8* devMac);
S8* USER_FUNC macAddrToString(U8* macAddr, S8*macString);
BOOL USER_FUNC getDeviceIPAddr(U8* ipAddr);
void USER_FUNC showHexData(S8* descript, U8* showData, U8 lenth);
void USER_FUNC printGlobalParaStatus(S8* discript);

void USER_FUNC itoParaInit(void);
GLOBAL_CONFIG_DATA* USER_FUNC getGlobalConfigData(void);

U8* USER_FUNC mallocSocketData(size_t size);
void USER_FUNC FreeSocketData(U8* ptData);


BOOL USER_FUNC checkSocketData(S8* pData, S32 dataLen);
AES_KEY_TYPE USER_FUNC getAesKeyType(MSG_ORIGIN msgOrigin, U8* pData);
void USER_FUNC getAesKeyData(AES_KEY_TYPE keyType, U8* keyData);
BOOL USER_FUNC socketDataAesDecrypt(U8 *inData, U8* outData, U32* aesDataLen, AES_KEY_TYPE keyType);
BOOL USER_FUNC socketDataAesEncrypt(U8 *inData, U8* outData, U32* aesDataLen, AES_KEY_TYPE keyType);

U8* USER_FUNC createSendSocketData(CREATE_SOCKET_DATA* createData, U32* sendSocketLen);
U8* USER_FUNC encryptRecvSocketData(MSG_ORIGIN msgOrigin, U8* pSocketData, U32* recvDataLen);



#endif
