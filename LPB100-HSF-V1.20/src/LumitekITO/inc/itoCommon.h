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
//lumitek.bugull.com:17531 ==> 122.227.164.112
#define TCP_SERVER_IP		"122.227.164.112"
//#define TCP_SERVER_IP		"192.168.1.105"


//aes key define
#define DEFAULT_AES_KEY		"1234567890abcdef"
#define DEFAULT_AES_IV		"1234567890abcdef"
#define AES_KEY_LEN			16

//Modual info
#define HW_VERSION			"HW_V1.00_01"
#define SW_VERSION			"1.00.01"
#define DEFAULT_MODUAL_NAME		"Lumitek switch"


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

//Heart beat interval
#define MAX_HEARTBEAT_INTERVAL			30
#define MIN_HEARTBEAT_INTERVAL			20


//other data define define
#define MAX_SOCKEY_DATA_LEN				256
#define SOCKET_HEADER_OPEN_DATA_LEN		sizeof(SOCKET_HEADER_OPEN)
#define SOCKET_HEADER_SECRET_DATA_LEN	(SOCKET_HEADER_LEN - SOCKET_HEADER_OPEN_DATA_LEN)
#define NETWORK_MAXRECV_LEN				(MAX_SOCKEY_DATA_LEN + SOCKET_HEADER_OPEN_DATA_LEN + 1)
#define SOCKET_HEADER_LEN				sizeof(SCOKET_HERADER_OUTSIDE)
#define DEVICE_MAC_LEN					6
#define SOCKET_IP_LEN					4
#define SOCKET_MAC_ADDR_OFFSET			2
#define SOCKET_CMD_OFFSET				SOCKET_HEADER_LEN
#define DEVICE_NAME_LEN					20
#define MAX_ALARM_COUNT				16
#define MAX_ABSENCE_COUNT			10
#define MAX_COUNTDOWN_COUNT			1

//define invalid data
#define INVALID_SN_NUM				0xFFFF
#define INVALID_SERVER_ADDR			0xFFFFFFFFU
#define INVALID_SERVER_PORT			0xFFFF

//resend socket define
#define MAX_RESEND_COUNT			4		//4¥Œ
#define MAX_RESEND_INTERVAL			8		//8√Î


typedef enum
{
	MSG_LOCAL_EVENT	= 0,
	MSG_FROM_UDP	= 1,
	MSG_FROM_TCP	= 2
} MSG_ORIGIN;



typedef enum
{
	AES_KEY_DEFAULT,
	AES_KEY_LOCAL,
	AES_KEY_SERVER,
	AES_KEY_OPEN
} AES_KEY_TYPE;


typedef struct
{
	BOOL localAesKeyValid;
	BOOL serverAesKeyValid;
	U8	serverKey[AES_KEY_LEN];
	U8	localKey[AES_KEY_LEN];
} AES_KEY_DATA;



typedef enum
{
	STA_CONN_BIT,
	DHPC_OK_BIT,
	BALANCE_CONN_BIT,
	SERVER_ADDR_BIT,
	SERVER_CONN_BIT,
	RESERVED_BIT
} DEVICE_CONN_TYPE;


typedef enum
{
	CLOSE_SOCKET,
	SHUTDOWN_SOCKET
} CLOSE_SOCKET_TYPE;


typedef struct
{
	U8 staConn:1;
	U8 dhcpOK:1;
	U8 balanceOK:1;
	U8 serverAdd:1;
	U8 serverConn:1;
	U8 reserved4:1;
	U8 reserved5:1;
	U8 reserved6:1;

} DEVICE_CONN_INFO;


//ALARM data
typedef struct
{
	U8 monday:1;
	U8 tuesday:1;
	U8 wednesday:1;
	U8 thursday:1;
	U8 firday:1;
	U8 saturday:1;
	U8 sunday:1;
	U8 bActive:1;
} ALARM_REPEAT_DATA;


typedef enum
{
	SWITCH_CLOSE = 0,
	SWITCH_OPEN = 1
} SWITCH_ACTION;

typedef struct
{
	ALARM_REPEAT_DATA repeatData;
	U8 hourData;
	U8 minuteData;
	U8 action;
} ALARM_DATA_INFO;



typedef struct
{
	ALARM_REPEAT_DATA repeatData;
	U8 startHour;
	U8 startMinute;
	U8 endHour;
	U8 endMinute;
	U8 timeData;
} ASBENCE_DATA_INFO;


typedef struct
{
	U8 reserved0:1;
	U8 reserved1:1;
	U8 reserved2:1;
	U8 reserved3:1;
	U8 reserved4:1;
	U8 reserved5:1;
	U8 reserved6:1;
	U8 bActive:1;
} COUNTDOWN_FLAG;



typedef struct
{
	COUNTDOWN_FLAG flag;
	U8		action;
	U32		count;
} COUNTDOWN_DATA_INFO;


typedef struct
{
	U8	nameLen;
	U8	nameData[DEVICE_NAME_LEN];
} DEVICE_NAME_DATA;


typedef struct
{
	U16 port;
	U32 ipAddr;
} SOCKET_ADDR;


typedef struct
{
	U16 lumitekFlag;
	U8	bLocked;	//used for check device be locked
	U8	swVersion;	//Used for upgrade check
	DEVICE_NAME_DATA deviceName;
	ALARM_DATA_INFO alarmData[MAX_ALARM_COUNT];
	ASBENCE_DATA_INFO absenceData[MAX_ABSENCE_COUNT];
	COUNTDOWN_DATA_INFO countDownData[MAX_COUNTDOWN_COUNT];
} DEVICE_CONFIG_DATA;


typedef struct
{
	U8	macAddr[DEVICE_MAC_LEN];
	AES_KEY_DATA	keyData;
	U16 mallocCount;
	U16 socketSn;
	time_t nextHeartTime;
	SOCKET_ADDR tcpServerAddr;
	DEVICE_CONN_INFO connInfo;
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



typedef struct
{
	U8 cmdCode;
	U8 bEncrypt;
	U8 bReback;
	U16 snIndex;
	U16 bodyLen;
	AES_KEY_TYPE keyType;
	U8* bodyData;
} CREATE_SOCKET_DATA;


BOOL USER_FUNC checkSmartlinkStatus(void);

S8* USER_FUNC getUdpRecvBuf(BOOL setZero);
S8* USER_FUNC getTcpRecvBuf(BOOL setZero);

//device connect info
void USER_FUNC setDeviceConnectInfo(DEVICE_CONN_TYPE connType, BOOL value);
BOOL USER_FUNC getDeviceConnectInfo(DEVICE_CONN_TYPE connType);

//get server address
void USER_FUNC setServerAddr(SOCKET_ADDR* pSocketAddr);
void USER_FUNC getServerAddr(SOCKET_ADDR* pSocketAddr);

//AES key
void USER_FUNC clearServerAesKey(BOOL clearAddr);
void USER_FUNC setServerAesKey(U8* serverKey);

//HeartBeat time
void setNextHeartbeatTime(U16 Interval);
time_t getNextHeartbeatTime(U16 Interval);


U16 USER_FUNC getSocketSn(BOOL needIncrease);

//device lock status
void USER_FUNC changeDeviceLockedStatus(BOOL bLocked);
U8 USER_FUNC getDeviceLockedStatus(void);

//device name info
void USER_FUNC changeDeviceSwVersion(U8 swVersion);
U8 USER_FUNC getDeviceSwVersion(void);

//alarm
void USER_FUNC setAlarmData(ALARM_DATA_INFO* alarmData, U8 index);
void USER_FUNC deleteAlarmData(U8 index);
ALARM_DATA_INFO* USER_FUNC getAlarmData(U8 index);

//Absence
void USER_FUNC setAbsenceData(ASBENCE_DATA_INFO* absenceData, U8 index);
void USER_FUNC deleteAbsenceData(U8 index);
ASBENCE_DATA_INFO* USER_FUNC getAbsenceData(U8 index);

//CountDown
void USER_FUNC setCountDownData(COUNTDOWN_DATA_INFO* countDownData, U8 index);
void USER_FUNC deleteCountDownData(U8 index);
COUNTDOWN_DATA_INFO* USER_FUNC getCountDownData(U8 index);

//Get MAC
U8* USER_FUNC getDeviceMacAddr(U8* devMac);
BOOL USER_FUNC needRebackRecvSocket(U8* macAddr, BOOL bItself);
BOOL USER_FUNC getDeviceIPAddr(U8* ipAddr);


//debug API
#ifdef LUMITEK_DEBUG_SWITCH
S8* USER_FUNC macAddrToString(U8* macAddr, S8*macString);
void USER_FUNC showHexData(S8* descript, U8* showData, U8 lenth);
void USER_FUNC debugShowSendData(MSG_ORIGIN msgOrigin, U8* pSocketData, U32 recvDataLen);
void USER_FUNC showSocketOutsideData(U8* pData);
void USER_FUNC printGlobalParaStatus(S8* discript);
U16 USER_FUNC getMallocCount(void);

#endif

//device name api
void USER_FUNC setDeviceName(DEVICE_NAME_DATA* nameData);
DEVICE_NAME_DATA* USER_FUNC getDeviceName(void);

void USER_FUNC itoParaInit(void);
GLOBAL_CONFIG_DATA* USER_FUNC getGlobalConfigData(void);

//memory function
U8* USER_FUNC mallocSocketData(size_t size);
void USER_FUNC FreeSocketData(U8* ptData);

BOOL USER_FUNC checkRecvSocketData(U32 recvCount, S8* recvBuf);
AES_KEY_TYPE USER_FUNC getSocketAesKeyType(MSG_ORIGIN msgOrigin, U8 bEncrypt);
BOOL USER_FUNC getAesKeyData(AES_KEY_TYPE keyType, U8* keyData);
BOOL USER_FUNC socketDataAesDecrypt(U8 *inData, U8* outData, U32* aesDataLen, AES_KEY_TYPE keyType);
BOOL USER_FUNC socketDataAesEncrypt(U8 *inData, U8* outData, U32* aesDataLen, AES_KEY_TYPE keyType);

//AES CBC 128 function
U8* USER_FUNC createSendSocketData(CREATE_SOCKET_DATA* createData, U32* sendSocketLen);
U8* USER_FUNC encryptRecvSocketData(MSG_ORIGIN msgOrigin, U8* pSocketData, U32* recvDataLen);



#endif
