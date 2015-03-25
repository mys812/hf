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

//UDP心跳包事件固定30秒
#define LUM_UDP_HEART_INTERVAL_30S

//socket log 添加index标识
//#define LUN_SOCKET_SHOW_INDEX

#ifdef LUMITEK_DEBUG_SWITCH
//保存发送和接收的Socket信息到Flash
//#define SAVE_LOG_TO_FLASH

//串口打印SOCKET信息
//#define LUM_UART_SOCKET_LOG

//通过UDP 打印LOG信息
#define LUM_UDP_SOCKET_LOG
#endif

#define LUMITEK_DEBUG
#if defined(LUMITEK_DEBUG) && !defined(UART_NOT_SUPPORT)

#define lumi_debug(...)	HF_Debug(DEBUG_LEVEL_USER, __VA_ARGS__)

#define lumi_error(...)	HF_Debug(DEBUG_LEVEL_USER, "========> ERROR func=%s, (line = %d)", __FUNCTION__,__LINE__); \
						HF_Debug(DEBUG_LEVEL_USER, __VA_ARGS__)

#else
#define lumi_debug(...)
#define lumi_error(...)
#endif
#endif

