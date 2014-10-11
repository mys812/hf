
/* hfwifi.h
 *
 * Copyright (C) 2013-2014 ShangHai High-flying Electronics Technology Co.,Ltd.
 *
 * This file is part of HSF.
 * 
 * Modify:
 * 2013-12-31 : Create by Jim
 */

 #ifndef __HF_WIFI_H_H__
 #define __HF_WIFI_H_H__

typedef struct _WIFI_SCAN_RESULT_ITEM
 {
	uint8_t auth;
	uint8_t encry;
	uint8_t channel;
	uint8_t rssi;
	char    ssid[32+1];
	char    mac[6];
 }WIFI_SCAN_RESULT_ITEM,*PWIFI_SCAN_RESULT_ITEM;

typedef int (*hfwifi_scan_callback_t)( PWIFI_SCAN_RESULT_ITEM );


int HSF_API hfwifi_scan(hfwifi_scan_callback_t p_callback);
int HSF_API hfwifi_enable_ap_idle_auto_reset(int max_idle_time);


 #endif


