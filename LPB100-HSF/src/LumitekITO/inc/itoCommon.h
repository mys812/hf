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
#if defined(LUM_UDP_SOCKET_LOG) || defined(LUM_RN8209C_UDP_LOG)
#define UDP_SOCKET_LOG_OFFSET	19000
#endif

//lumitek.bugull.com:17531 ==> 122.227.164.112
//#define TCP_SERVER_IP		"122.227.164.112"
//#define TCP_SERVER_IP		"lumitek.bugull.com"
#ifdef BANLENCE_ADDR_HOSTNAME_SUPPORT
#define TCP_SERVER_IP		"lumitek.yunext.com"
#else
#define TCP_SERVER_IP		"52.10.198.35"
#endif


//get UTC date info
#define MAX_PING_DATA_COUNT	3
#define TCP_DATE_PORT		37
#define TCP_DATE_IP			"128.138.140.44"


//aes key define
#define DEFAULT_AES_KEY		"1234567890abcdef"
#define DEFAULT_AES_IV		"1234567890abcdef"
#define AES_KEY_LEN			16

//Modual info
#define HW_VERSION			"HW_V1.01"
#define SW_VERSION			"1.25"
#define DEFAULT_MODUAL_NAME		"LumSwitch"

#define FACTORY_TOOL_MAC		"ABCDEF"




//GPIO define
#ifdef DEEVICE_LUMITEK_P1
	#define HFGPIO_F_BUZZER		         (HFGPIO_F_USER_DEFINE+0)
	#define HFGPIO_F_SWITCH				 (HFGPIO_F_USER_DEFINE+1)
	#ifdef EXTRA_SWITCH_SUPPORT
	#define HFGPIO_F_EXTRA_SWITCH		 (HFGPIO_F_USER_DEFINE+2)
	#endif
#elif defined(DEEVICE_LUMITEK_P2)
	#define HFGPIO_F_RELAY_1		     (HFGPIO_F_USER_DEFINE+0)
	#define HFGPIO_F_RELAY_2		     (HFGPIO_F_USER_DEFINE+1)
	#define HFGPIO_F_SWITCH			     (HFGPIO_F_USER_DEFINE+2)
	#define HFGPIO_F_CF				     (HFGPIO_F_USER_DEFINE+3)
	#define HFGPIO_F_WIFI_LED		     (HFGPIO_F_USER_DEFINE+4)
	#define HFGPIO_F_RELAY_LED		     (HFGPIO_F_USER_DEFINE+5)
	#define HFGPIO_F_KEY			     (HFGPIO_F_USER_DEFINE+6)
#elif defined(DEEVICE_LUMITEK_P3) || defined(DEEVICE_LUMITEK_P6)
	#define HFGPIO_F_KEY				 (HFGPIO_F_USER_DEFINE+0)
	#define HFGPIO_F_WIFI_LED			 (HFGPIO_F_USER_DEFINE+1)
	#define HFGPIO_F_SWITCH 			 (HFGPIO_F_USER_DEFINE+2)
#elif defined(DEEVICE_LUMITEK_P4)
	#define HFGPIO_F_BUZZER 			 (HFGPIO_F_USER_DEFINE+0)
	#define HFGPIO_F_SWITCH 			 (HFGPIO_F_USER_DEFINE+1)
	#define HFGPIO_F_SWITCH_2 			 (HFGPIO_F_USER_DEFINE+2)
	#ifdef EXTRA_SWITCH_SUPPORT
	#define HFGPIO_F_EXTRA_SWITCH		 (HFGPIO_F_USER_DEFINE+3)
	#define HFGPIO_F_EXTRA_SWITCH_2		 (HFGPIO_F_USER_DEFINE+4)
	#endif
#elif defined(DEEVICE_LUMITEK_P5)
	#define HFGPIO_F_RESET 				 (HFGPIO_F_USER_DEFINE+0)
	#define HFGPIO_F_SDO_0 				 (HFGPIO_F_USER_DEFINE+1)
	#define HFGPIO_F_SDO_1 				 (HFGPIO_F_USER_DEFINE+2)
	#define HFGPIO_F_SDO_2 				 (HFGPIO_F_USER_DEFINE+3)
	#define HFGPIO_F_SWITCH 			 (HFGPIO_F_USER_DEFINE+4)
	#define HFGPIO_F_WIFI_LED			 (HFGPIO_F_USER_DEFINE+5)
	#define HFGPIO_F_KEY				 (HFGPIO_F_USER_DEFINE+6)

#else
	#error "GPIO not defined!"
#endif

//timer id define
#define SMARTLINK_TIMER_ID      	1
#define GET_UTC_TIMER_ID			2
#define HEARTBEAT_TIMER_ID			3
#define CHECK_SMARTLINK_TIMER_ID	4
#define CHECK_TIME_TIMER_ID			5
#ifdef SPECIAL_RELAY_SUPPORT
#define SPECILA_RELAY_TIMER_ID		6
#endif
#ifdef DEVICE_KEY_SUPPORT
#define DEVICE_KEY_TIMER_ID			7
#endif
#ifdef DEVICE_WIFI_LED_SUPPORT
#define WIFI_LED_TIMER_ID			8
#endif
#ifdef RN8209C_SUPPORT
#define RN8209C_READ_TIMER_ID		9
#endif
#define KEY_IRQ_DEBOUNCE_TIMER_ID	10

#ifdef RN8209C_SUPPORT
#define REPORT_ENERGY_DATA_TIMER_ID		11
#endif

#ifdef LUM_READ_ENERGY_TEST
#define READ_ENERGY_TEST_TIMER_ID		12
#endif

#define REPORT_FACTORY_RESET_TIMER_ID	13
#ifdef SX1208_433M_SUPPORT
#define STUDY_433_TIMER_ID			14
#endif

#if defined(LUM_FACTORY_TEST_SUPPORT) && defined(RN8209C_SUPPORT)
#define RN8209_CALI_TIMER_ID		15
#endif //LUM_FACTORY_TEST_SUPPORT



//device save data define
#define DEVICE_CONFIG_OFFSET_START 0x00
#define DEVICE_CONFIG_SIZE (sizeof(DEVICE_CONFIG_DATA))

//socket result define
#define OPT_RESULT_SUCCESS	0x00
#define OPT_RESULT_FAILD	0x00

//socket header data
#define SOCKET_HEADER_PV			0x01
#define SOCKET_HEADER_RESERVED		0x00
#ifdef DEEVICE_LUMITEK_P1
	#define SOCKET_HEADER_DEVICE_TYPE	0xD1
#elif defined(DEEVICE_LUMITEK_P2)
	#define SOCKET_HEADER_DEVICE_TYPE	0xDE
#elif defined(DEEVICE_LUMITEK_P3) || defined(DEEVICE_LUMITEK_P6)
	#define SOCKET_HEADER_DEVICE_TYPE	0xDF
#elif defined(DEEVICE_LUMITEK_P4)
	#define SOCKET_HEADER_DEVICE_TYPE	0xD2
#elif defined(DEEVICE_LUMITEK_P5)
	#define SOCKET_HEADER_DEVICE_TYPE	0xD3
	//#define SOCKET_HEADER_DEVICE_TYPE	0xDF
#else
	#error "Please select product type!"
#endif
#define SOCKET_HEADER_FACTORY_CODE	0xF1
#define SOCKET_HEADER_LICENSE_DATA	0xB421	//Correct data is  0x21B4

//sw flag
#define LUMITEK_SW_FLAG					0xDCBA

//Heart beat interval
#ifdef LUM_UDP_HEART_INTERVAL_30S
#define UDP_HEARTBEAT_INTERVAL			30
#else
#define MAX_HEARTBEAT_INTERVAL			20
#define MIN_HEARTBEAT_INTERVAL			10
#endif


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
#define MAX_ALARM_COUNT				32
#define MAX_ABSENCE_COUNT			10
#define MAX_COUNTDOWN_COUNT			1
#define INVALID_ALARM_FLAG			0xFF
#define MAX_USER_NAME_LEN			50
#define ONE_DAY_SECOND				86400UL

#ifdef TWO_SWITCH_SUPPORT
#define TOTAL_ALARM_COUNT			(MAX_ALARM_COUNT<<1)
#define TOTAL_ABSENCE_COUNT			(MAX_ABSENCE_COUNT<<1)
#define TOTAL_COUNTDOWN_COUNT		(MAX_COUNTDOWN_COUNT<<1)
#else
#define TOTAL_ALARM_COUNT			MAX_ALARM_COUNT
#define TOTAL_ABSENCE_COUNT			MAX_ABSENCE_COUNT
#define TOTAL_COUNTDOWN_COUNT		MAX_COUNTDOWN_COUNT
#endif

//define invalid data
#define INVALID_SN_NUM				0xFFFF
#define INVALID_SERVER_ADDR			0xFFFFFFFFU
#define INVALID_SERVER_PORT			0xFFFF

//resend socket define
#define MAX_RESEND_COUNT			4		//4次
#define MAX_FAILD_COUNT				4
#define MAX_RESEND_INTERVAL			8		//8秒

#define MAX_SOCKET_SELECT_WAIT_SECOND		20
#define MAX_UPGRADE_URL_LEN					110
#define SOFTWARE_UPGRADE_FLAG				0xEE


#define DEFAULT_AP_SSID			"moduletest"
#define DEFAULT_AP_PASSWORD		"test1234"
//#define FACTORY_TEST_SSID		"Lumlink_test"
//#define FACTORY_TEST_PASSWORD	"12340000"




//Factory flag
#define FACTORY_TEST_DATA_OFFSET		0
#define FACTORY_TEST_DATA_TOTAL_SIZE	HFFLASH_PAGE_SIZE

// energy data
#define ENERGY_DATA_OFFSET				FACTORY_TEST_DATA_TOTAL_SIZE
#define ENERGY_DATA_TOTAL_SIZE			HFFLASH_PAGE_SIZE



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


typedef enum
{
	RESET_FOR_NORMAL		= 0x00,
	RESET_FOR_SMARTLINK		= 0x01,
	RESET_FOR_SMARTLINK_OK	= 0x02,
	RESET_FOR_UPGRADE		= 0x04,
	RESET_FOR_FACTORY_TEST	= 0x08
} DEVICE_RESET_TYPE;

typedef struct
{
	BOOL localAesKeyValid;
	BOOL serverAesKeyValid;
	U8	serverKey[AES_KEY_LEN];
	U8	localKey[AES_KEY_LEN];
} AES_KEY_DATA;



typedef enum
{
	DHPC_OK_BIT,
	BALANCE_CONN_BIT,
	SERVER_ADDR_BIT,
	SERVER_CONN_BIT,
} DEVICE_CONN_TYPE;


typedef enum
{
	CLOSE_SOCKET,
	SHUTDOWN_SOCKET
} CLOSE_SOCKET_TYPE;


typedef struct
{
	U8 dhcpOK:1;
	U8 balanceOK:1;
	U8 serverAdd:1;
	U8 serverConn:1;
	U8 reserved4:1;
	U8 reserved5:1;
	U8 reserved6:1;
	U8 reserved7:1;

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
} SWITCH_STATUS;


typedef enum
{
	LIGHT_CLOSE = 0,
	LIGHT_OPEN = 1
} LIGHT_STATUS;


typedef enum
{
	BUZZER_CLOSE = 0,
	BUZZER_OPEN = 1
} BUZZER_STATUS;


typedef enum
{
	EVENT_INCATIVE = 0,
	EVENT_ACTIVE = 1
} ACTIVE_STATUS;

typedef struct
{
	ALARM_REPEAT_DATA repeatData;
	U8 startHour;
	U8 startMinute;
	U8 stopHour;
	U8 stopMinute;
	U8 reserved;
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
	U8 upgradeFlag;
	S8 urlData[MAX_UPGRADE_URL_LEN];
}SW_UPGRADE_DATA;


#ifdef RN8209C_SUPPORT
typedef struct
{
	U16	rn8209cHFCost;
	U16 rn8209cKP;
	U16 rn8209cKI;
	U16 rn8209cKV;
}RN8209C_CALI_DATA;
#endif


typedef struct
{
	U8	bLocked;	//used for check device be locked
	U8	swVersion;	//Used for upgrade check
	U8	udpLogFlag;
	U8	reserved[17]; //保证前20个字节对齐
	DEVICE_NAME_DATA deviceName;
	ALARM_DATA_INFO alarmData[TOTAL_ALARM_COUNT];
	ASBENCE_DATA_INFO absenceData[TOTAL_ABSENCE_COUNT];
	COUNTDOWN_DATA_INFO countDownData[TOTAL_COUNTDOWN_COUNT];
	SW_UPGRADE_DATA upgradeData;
#ifdef RN8209C_SUPPORT
	RN8209C_CALI_DATA rn8209cData;
#endif
	BOOL needSmartLink;
	U8 userName[MAX_USER_NAME_LEN];  //need report factory reset to server for delete user accent
	U16 lumitekFlag;
} DEVICE_CONFIG_DATA;


typedef struct
{
	U8	macAddr[DEVICE_MAC_LEN];
	U32 ipAddr;
	AES_KEY_DATA	keyData;
	U16 mallocCount;
	U16 socketSn;
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
	U8 bEncrypt;
	U8 bReback;
	MSG_ORIGIN msgOrigin;
	U16 snIndex;
	U16 bodyLen;
	AES_KEY_TYPE keyType;
	U8* bodyData;
} CREATE_SOCKET_DATA;


void USER_FUNC globalConfigDataInit(BOOL factoryReset);
//get static buf
S8* USER_FUNC getUdpRecvBuf(BOOL setZero);
S8* USER_FUNC getTcpRecvBuf(BOOL setZero);

//device connect info
void USER_FUNC setDeviceConnectInfo(DEVICE_CONN_TYPE connType, BOOL value);
BOOL USER_FUNC getDeviceConnectInfo(DEVICE_CONN_TYPE connType);
void USER_FUNC setFlagAfterDhcp(U32 ipAddr);
void USER_FUNC setFlagAfterApDisconnect(void);

//get server address
void USER_FUNC setServerAddr(SOCKET_ADDR* pSocketAddr);
void USER_FUNC getServerAddr(SOCKET_ADDR* pSocketAddr);

//AES key
void USER_FUNC clearServerAesKey(BOOL clearAddr);
void USER_FUNC setServerAesKey(U8* serverKey);
BOOL USER_FUNC lum_getServerKeyStatus(void);


//SN index
U16 USER_FUNC getSocketSn(BOOL needIncrease);

//device lock status
void USER_FUNC changeDeviceLockedStatus(BOOL bLocked);
BOOL USER_FUNC getDeviceLockedStatus(void);

//device name info
void USER_FUNC changeDeviceSwVersion(U8 swVersion);
U8 USER_FUNC getDeviceSwVersion(void);

//rn8209c
#ifdef RN8209C_SUPPORT
void USER_FUNC rn8209cClearCalibraterData(void);
RN8209C_CALI_DATA* USER_FUNC lum_rn8209cGetCaliData(void);
void USER_FUNC lum_rn8209cSaveCaliData(void);


#endif


//alarm
void USER_FUNC setAlarmData(ALARM_DATA_INFO* alarmData, U8 index);
void USER_FUNC deleteAlarmData(U8 index, BOOL needSave);
ALARM_DATA_INFO* USER_FUNC getAlarmData(U8 index);

//Absence
void USER_FUNC setAbsenceData(ASBENCE_DATA_INFO* absenceData, U8 index);
void USER_FUNC deleteAbsenceData(U8 index, BOOL needSave);
ASBENCE_DATA_INFO* USER_FUNC getAbsenceData(U8 index);

//CountDown
void USER_FUNC setCountDownData(COUNTDOWN_DATA_INFO* countDownData, U8 index);
void USER_FUNC deleteCountDownData(U8 index);
COUNTDOWN_DATA_INFO* USER_FUNC getCountDownData(U8 index);

//Get MAC
U8* USER_FUNC getDeviceMacAddr(U8* devMac);
U32 USER_FUNC getDeviceIpAddress(void);
U32 USER_FUNC getBroadcastAddr(void);



//debug API
//#ifdef LUMITEK_DEBUG_SWITCH
S8* USER_FUNC macAddrToString(U8* macAddr, S8*macString);
void USER_FUNC showHexData(S8* descript, U8* showData, S32 lenth);
void USER_FUNC debugShowSendData(MSG_ORIGIN msgOrigin, U8* pSocketData, U32 recvDataLen);
void USER_FUNC showSocketOutsideData(U8* pData);
void USER_FUNC printGlobalParaStatus(S8* discript);
U16 USER_FUNC getMallocCount(void);

//#endif

//device name api
void USER_FUNC setDeviceName(DEVICE_NAME_DATA* nameData);
DEVICE_NAME_DATA* USER_FUNC getDeviceName(void);

void USER_FUNC itoParaInit(BOOL bFactoryTest);
GLOBAL_CONFIG_DATA* USER_FUNC getGlobalConfigData(void);

//memory function
U8* USER_FUNC mallocSocketData(size_t size);
void USER_FUNC FreeSocketData(U8* ptData);

//check socket
BOOL USER_FUNC lum_checkSocketBeforeAES(U32 recvCount, U8* recvBuf);
BOOL USER_FUNC lum_checkSocketAfterAES(U8* socketData);

AES_KEY_TYPE USER_FUNC getSocketAesKeyType(MSG_ORIGIN msgOrigin, U8 bEncrypt);
BOOL USER_FUNC getAesKeyData(AES_KEY_TYPE keyType, U8* keyData);
BOOL USER_FUNC socketDataAesDecrypt(U8 *inData, U8* outData, U32* aesDataLen, AES_KEY_TYPE keyType);
BOOL USER_FUNC socketDataAesEncrypt(U8 *inData, U8* outData, U32* aesDataLen, AES_KEY_TYPE keyType);

//AES CBC 128 function
U8* USER_FUNC createSendSocketData(CREATE_SOCKET_DATA* createData, U32* sendSocketLen);
U8* USER_FUNC encryptRecvSocketData(MSG_ORIGIN msgOrigin, U8* pSocketData, U32* recvDataLen);

//software upgrade
void USER_FUNC setSoftwareUpgradeUrl(S8* url, U8 urlLen);
void USER_FUNC clearSoftwareUpgradeFlag(void);
SW_UPGRADE_DATA* USER_FUNC getSoftwareUpgradeData(void);

DEVICE_RESET_TYPE USER_FUNC checkResetType(void);

void USER_FUNC lum_deviceFactoryReset(void);

U8* USER_FUNC lum_getUserName(void);
void USER_FUNC lum_setUserName(U8* userName);

void USER_FUNC lum_setFactorySmartlink(BOOL bCancle);
BOOL USER_FUNC lum_getFactorySmartlink(void);

void USER_FUNC lum_setUdpLogFlag(U8 bEnable);
BOOL USER_FUNC lum_getUdpLogFlag(void);

#ifndef LUM_FACTORY_TEST_SUPPORT
BOOL USER_FUNC lum_bEnterFactoryTest(void);
#endif

#endif

