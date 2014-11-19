#ifndef __LUMITEK_CONFIG_H__
#define __LUMITEK_CONFIG_H__


#define CONFIG_LUMITEK_DEVICE
#define LUMITEK_DEBUG_SWITCH
//#define DEEVICE_LUMITEK_P1
#define LPB100_DEVLOPMENT_BOARD

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
