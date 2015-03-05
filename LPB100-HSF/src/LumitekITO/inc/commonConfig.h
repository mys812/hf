/*
******************************
*Company:Lumitek
*Data:2011-01-03
*Author:Meiyusong
******************************
*/

#ifndef __LUMITEK_COMMON_CONFIG_H__
#define __LUMITEK_COMMON_CONFIG_H__

//软件调试开关
#define LUMITEK_DEBUG_SWITCH

//软件升级支持断点续传
//#define HTTP_DOWNLOAD_SUPPORT_RESUMING

//通过下载BIN文件升级
#define DEVICE_UPGRADE_BY_DOWNLOAD_BIN

//通过下载Config文件升级
//#define DEVICE_UPGRADE_BY_CONFIG

//通过设置特殊闹钟升级
//#define ENTER_UPGRADE_BY_AMARM

#ifdef LUMITEK_DEBUG_SWITCH
//保存发送和接收的Socket信息到Flash
#define SAVE_LOG_TO_FLASH

//通过UDP 打印LOG信息
#define SEND_LOG_BY_UDP
#endif

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

