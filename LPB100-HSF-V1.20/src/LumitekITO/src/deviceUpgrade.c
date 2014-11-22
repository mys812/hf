/*
******************************
*Company:Lumitek
*Data:2014-11-09
*Author:Meiyusong
******************************
*/

#include "../inc/lumitekConfig.h"

#ifdef CONFIG_LUMITEK_DEVICE
#include <hsf.h>
#include <string.h>
#include <stdio.h>
#include "md5.h"


#include "../inc/itoCommon.h"
#include "../inc/deviceUpgrade.h"



void USER_FUNC setUpgradeType(S8* url)
{
	setSoftwareUpgradeUrl(url);
	msleep(300);
	hfsys_reset();
}



#if 1
static int USER_FUNC softwareUpgrade(S8* urlData)
{
	httpc_req_t  http_req;
	char *content_data=NULL;
	char *temp_buf=NULL;
	parsed_url_t url={0};
	http_session_t hhttp=0;
	S32 total_size = 0;
	S32 read_size = 0;
	int rv = 0;
	tls_init_config_t  *tls_cfg=NULL;
	struct MD5Context md5_ctx;
	U8 digest[16]={0};
	
	bzero(&http_req,sizeof(http_req));
	http_req.type = HTTP_GET;
	http_req.version=HTTP_VER_1_1;
	

	lumi_debug("urlData=%s\n", urlData);
	if((temp_buf = (char*)hfmem_malloc(MAX_RECEIVE_BUF_SIZE))==NULL)
	{
		u_printf("no memory\n");
		rv = -HF_E_NOMEM;
		goto exit;
	}
	
	bzero(temp_buf,sizeof(temp_buf));
	if((rv = hfhttp_parse_URL(urlData,temp_buf , MAX_RECEIVE_BUF_SIZE, &url)) != HF_SUCCESS)
	{
		goto exit;
	}

	if((rv = hfhttp_open_session(&hhttp,urlData,0,tls_cfg,3)) != HF_SUCCESS)
	{
		u_printf("http open fail\n");
		goto exit;
	}

	hfsys_disable_all_soft_watchdogs();
	hfupdate_start(HFUPDATE_SW);
	http_req.resource = url.resource;
	hfhttp_prepare_req(hhttp,&http_req, HDR_ADD_CONN_CLOSE);
	hfhttp_add_header(hhttp,"Range","bytes=0");
	if((rv = hfhttp_send_request(hhttp,&http_req))!=HF_SUCCESS)
	{
		u_printf("http send request fail\n");
		goto exit;
	}
	
	content_data = (char*)hfmem_malloc(MAX_RECEIVE_BUF_SIZE);
	if(content_data==NULL)
	{
		rv = -HF_E_NOMEM;
		goto exit;
	}
	total_size=0;
	bzero(content_data,MAX_RECEIVE_BUF_SIZE);

	MD5Init(&md5_ctx);
	while(1)
	{
		read_size = hfhttp_read_content(hhttp,content_data,MAX_RECEIVE_BUF_SIZE);
		if(read_size > 0)
		{
			hfupdate_write_file(HFUPDATE_SW, total_size,content_data, read_size);
			MD5Update(&md5_ctx,(U8*)content_data,read_size);
			total_size += read_size;
			u_printf("download file:[%d] [%d]\n",total_size,read_size);
		}
		else
		{
			u_printf("faild read_size:%d  total_size=%d \n",read_size, total_size);
			break;
		}
	}
	MD5Final(digest,&md5_ctx);
	u_printf("read_size:%d digest is ",total_size);
	u_printf("%02x%02x%02x%02x",digest[0],digest[1],digest[2],digest[3]);
	u_printf("%02x%02x%02x%02x",digest[4],digest[5],digest[6],digest[7]);
	u_printf("%02x%02x%02x%02x",digest[8],digest[9],digest[10],digest[11]);
	u_printf("%02x%02x%02x%02x\n",digest[12],digest[13],digest[14],digest[15]);
	
	if(hfupdate_complete(HFUPDATE_SW,total_size) != HF_SUCCESS)
	{
		u_printf("update software fail\n");
	}
exit:
	if(temp_buf!=NULL)	
		hfmem_free(temp_buf);
	if(content_data!=NULL)
		hfmem_free(content_data);
	if(hhttp!=0)
		hfhttp_close_session(&hhttp);
	//hfgpio_fset_out_low(HFGPIO_F_NREADY);
	hfsys_enable_all_soft_watchdogs();
	lumi_debug("rv = %s\n", rv);
	return rv;
}



static void USER_FUNC deviceEnterSwUpgrade(void)
{
	SW_UPGRADE_DATA* pUpgradeData;

	pUpgradeData = getSoftwareUpgradeData();
	clearSoftwareUpgradeFlag();
	softwareUpgrade(pUpgradeData->urlData);
	msleep(1000);
}


#else

static void USER_FUNC deviceEnterSwUpgrade(void)
{
	SW_UPGRADE_DATA* pUpgradeData;
	S8 rsp[100]={0};
	S8 sendCmd[120];
	S8 cmdlen;
	U16 i = 0;


	
	pUpgradeData = getSoftwareUpgradeData();
	clearSoftwareUpgradeFlag();
	
	memset(sendCmd, 0, sizeof(sendCmd));
	cmdlen = strlen(pUpgradeData->urlData);
	if(cmdlen > 100)
	{
		return;
	}
	sprintf(sendCmd, "AT+UPURL=%s", pUpgradeData->urlData);
	cmdlen = strlen(sendCmd);

	lumi_debug("sendCmd = %s\n", sendCmd);
	hfat_send_cmd(sendCmd, cmdlen, rsp, 100);
	lumi_debug("rsp = %s\n", rsp);
	while(1)
	{
		lumi_debug("i=%d\n", i);
		i++;
		msleep(1000);
	}
}

#endif



static BOOL checkNetworkConnect(void)
{
	S32 pingRet;
	BOOL ret = FALSE;
	
	pingRet = ping(TCP_SERVER_IP);
	lumi_debug("pingRet=%d\n", pingRet);
	if(pingRet == 0)
	{
		ret = TRUE;
	}
	return ret;
}


static void USER_FUNC deviceUpgradeThread(void)
{
	while(1)
	{
		if(checkNetworkConnect())
		{
			deviceEnterSwUpgrade();
			hfsys_reset();
		}
		
		msleep(1000);
	}
}



void USER_FUNC enterUpgradeThread(void)
{
	if(hfthread_create((PHFTHREAD_START_ROUTINE)deviceUpgradeThread, "IOT_Upgrade_C", 512, NULL, HFTHREAD_PRIORITIES_LOW,NULL,NULL)!= HF_SUCCESS)
	{
		lumi_error("Create IOT_Upgrade_C thread failed!\n");
	}
	lumi_debug("go into enterUpgradeThread\n");
}


#endif

