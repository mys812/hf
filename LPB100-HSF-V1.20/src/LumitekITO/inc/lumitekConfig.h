#ifndef __LUMITEK_CONFIG_H__
#define __LUMITEK_CONFIG_H__

#define CONFIG_LUMITEK_DEVICE

//软件调试开关
#define LUMITEK_DEBUG_SWITCH

//开发板
#define LPB100_DEVLOPMENT_BOARD

//墙插设备
#define DEEVICE_LUMITEK_P1


//软件升级支持断点续传
//#define HTTP_DOWNLOAD_SUPPORT_RESUMING

//通过下载BIN文件升级
#define DEVICE_UPGRADE_BY_DOWNLOAD_BIN

//通过下载Config文件升级
//#define DEVICE_UPGRADE_BY_CONFIG

//通过设置特殊闹钟升级
#define ENTER_UPGRADE_BY_AMARM

//支持外部开关
#define EXTRA_SWITCH_SUPPORT

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
