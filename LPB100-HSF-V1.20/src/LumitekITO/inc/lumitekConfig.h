#ifndef __LUMITEK_CONFIG_H__
#define __LUMITEK_CONFIG_H__


#define CONFIG_LUMITEK_DEVICE
#define LUMITEK_DEBUG_SWITCH

#define LUMITEK_DEBUG
#ifdef LUMITEK_DEBUG

#define lumi_debug(...)	HF_Debug(DEBUG_LEVEL_USER, "%d========>", time(NULL)); \
						HF_Debug(DEBUG_LEVEL_USER, __VA_ARGS__)
#else
#define lumi_debug(...)
#endif

#endif
