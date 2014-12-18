
/* hfsmtlk.h
 *
 * Copyright (C) 2013-2014 ShangHai High-flying Electronics Technology Co.,Ltd.
 *
 * This file is part of HSF.
 * 
 * Modify:
 * 2013-12-25 : Create by Jim
 */

#ifndef _HFSMTLK_H_H_
#define _HFSMTLK_H_H_


//#define  hfsmtlk_start()	hfat_send_cmd("AT+SMTLK\r\n", sizeof("AT+SMTLK\r\n")-1, NULL, 0)
int HSF_API  hfsmtlk_start(void);

int HSF_API  hfsmtlk_stop(void);

int HSF_API hfsmtlk_enable_recv_data_from_router(int enable);

int HSF_API  hfwps_start(int timeout);

#endif


