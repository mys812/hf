#ifndef __LUMITEK_CONFIG_H__
#define __LUMITEK_CONFIG_H__

#define CONFIG_LUMITEK_DEVICE

//¦Ì¡Â¨º??a1?
#define LUMITEK_DEBUG_SWITCH

//??2??¡ê?¨¦
#define DEEVICE_LUMITEK_P1

//?a¡¤¡é¡ã?
//#define LPB100_DEVLOPMENT_BOARD

//?¨¬??D?¡ä?
//#define HTTP_DOWNLOAD_SUPPORT_RESUMING

//????bin???t¨¦y??
#define DEVICE_UPGRADE_BY_DOWNLOAD_BIN

//????config???t¨¦y??
//#define DEVICE_UPGRADE_BY_CONFIG

//¨¦¨¨??¨¬??¡§???¨®¨¦y??
#define ENTER_UPGRADE_BY_AMARM

#define LUMITEK_DEBUG
#ifdef LUMITEK_DEBUG

#define lumi_debug(...)	HF_Debug(DEBUG_LEVEL_USER, "%d========>", time(NULL)); \
						HF_Debug(DEBUG_LEVEL_USER, __VA_ARGS__)

#define lumi_error(...)	HF_Debug(DEBUG_LEVEL_USER, "%d========> ERROR func=%s, (line = %d)", time(NULL),__FUNCTION__,__LINE__); \
						HF_Debug(DEBUG_LEVEL_USER, __VA_ARGS__)

#else
#define lumi_debug(...)
#define lumi_error(...)
#endif

#endif
