#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctosapi.h>
#include <errno.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <sqlite3.h>
#include "../../SOURCE/INCLUDES/Define_1.h"
#include "../../SOURCE/INCLUDES/Define_2.h"
#include "../../SOURCE/INCLUDES/TransType.h"
#include "../../SOURCE/INCLUDES/Transaction.h"
#include "../../SOURCE/INCLUDES/AllStruct.h"
#include "../../SOURCE/DISPLAY/Display.h"
#include "../../SOURCE/DISPLAY/DispMsg.h"
#include "../../SOURCE/PRINT/Print.h"
#include "../../SOURCE/PRINT/PrtMsg.h"
#include "../../SOURCE/FUNCTION/Sqlite.h"
#include "../../SOURCE/FUNCTION/Function.h"
#include "../../SOURCE/FUNCTION/FuncTable.h"
#include "../../SOURCE/FUNCTION/HDT.h"
#include "../../SOURCE/FUNCTION/CPT.h"
#include "../../SOURCE/FUNCTION/CFGT.h"
#include "../../SOURCE/FUNCTION/EDC.h"
#include "../../SOURCE/FUNCTION/HDPT.h"
#include "../../SOURCE/FUNCTION/File.h"
#include "../../SOURCE/EVENT/MenuMsg.h"
#include "../../SOURCE/EVENT/Flow.h"
#include "../../SOURCE/COMM/Comm.h"
#include "../../SOURCE/COMM/Ethernet.h"
#include "../../SOURCE/COMM/Modem.h"
#include "../../CREDIT/Creditfunc.h"
#include "../../EMVSRC/EMVxml.h"
#include "../../NCCC/NCCCtmsCPT.h"
#include "../../NCCC/NCCCtmsSCT.h"
#include "../../NCCC/NCCCtmsiso.h"
#include "../../NCCC/NCCCtmsFTP.h"
#include "../../NCCC/NCCCtms.h"
#include "TLS.h"
#include "Ftps.h"

extern  int     ginDebug;		/* Debug使用 extern */
extern  int	ginMachineType;
CURL		*gpCurl;		/* FTPS使用 */
char		gszDisp[60 + 1];	/* 顯示檔名用 */


static size_t my_fwrite(void *buffer, size_t size, size_t nmemb,
                        void *stream)
{
	struct FtpFile *out = (struct FtpFile *)stream;
	
	if (out && !out->pStream) 
	{
	       /* open file for writing */ 
	      out->pStream = fopen(out->szFilename, "wb");
              
	      if (!out->pStream)
	      {
		      return -1; /* failure, can't open file to write */ 
	      }
	}
	
	return fwrite(buffer, size, nmemb, out->pStream);
}

static int xferinfo(void *p,
                    curl_off_t dltotal, curl_off_t dlnow,
                    curl_off_t ultotal, curl_off_t ulnow)
{
	char	szDispBar[100 + 1];
	char	szDispPer[100 + 1];
	int	inMode = 0;
        int     inPercent = 0;
        unsigned long ulColor;
//  double curtime = 0;
 
//  curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &curtime);
 
  /* under certain circumstances it may be desirable for certain functionality
     to only run every N seconds, in order to do this the transaction time can
     be used */ 
//  if((curtime - myp->lastruntime) >= MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL) {
//    myp->lastruntime = curtime;
//    fprintf(stderr, "TOTAL TIME: %f \r\n", curtime);
//  }
// 
  
  
//	fprintf(stderr, "UP: %" CURL_FORMAT_CURL_OFF_T " of %" CURL_FORMAT_CURL_OFF_T
//		"  DOWN: %" CURL_FORMAT_CURL_OFF_T " of %" CURL_FORMAT_CURL_OFF_T
//		"\r\n",
//		ulnow, ultotal, dlnow, dltotal);
 
//	if(dlnow > STOP_DOWNLOAD_AFTER_THIS_MANY_BYTES)
//		return 1;

	/* 顯示檔名及下載進度 */
//	inDISP_EnglishFont(gszDisp, _FONESIZE_8X22_, _LINE_8_5_);
	
        /* 判斷上傳還下載用 聯合無 */
//	inGetFTP_Mode(&inMode);
        inMode = 1;
	memset(szDispBar, 0x00, sizeof(szDispBar));
	memset(szDispPer, 0x00, sizeof(szDispPer));
	
	switch (inMode)
	{
		case 1 :
			inPercent = inFunc_Disp_ProgressBar(szDispBar, szDispPer, dltotal, dlnow);
			break;
		case 2 :
			memset(szDispBar, 0x00, sizeof(szDispBar));
			sprintf(szDispPer, "%lld%% ", ulnow);			
			break;
	}
        
        if ((inPercent > 0) && (inPercent < 5))
                inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);

        if (inPercent >= 100)
                ulColor = _COLOR_GREEN_;
        else if (inPercent == 50)
                ulColor = _COLOR_YELLOW_;
        else if (inPercent < 50)
                ulColor = _COLOR_RED_ + (0x00000500 * inPercent);
        else
                ulColor = _COLOR_YELLOW_ - (0x00000005 * (inPercent - 50));

	/* 至少每一秒顯示一次，不然會拖慢下載速度 */
	/* 100%時也顯示一次 */
	if (inTimerGet(_TIMER_NEXSYS_4_) == VS_SUCCESS	||
	    dlnow == dltotal)
	{
		if (strlen(szDispBar) > 0)
		{
			inDISP_ChineseFont_Color(szDispBar, _FONTSIZE_8X22_, _LINE_8_6_, ulColor, _DISP_LEFT_);
		}
		if (strlen(szDispPer) > 0)
		{
			inDISP_EnglishFont_Color(szDispPer, _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_RED_, _DISP_RIGHT_);
		}
		
		inDISP_Timer_Start(_TIMER_NEXSYS_4_, 1);
	}
	
	return 0;
}

/* for libcurl older than 7.32.0 (CURLOPT_PROGRESSFUNCTION) */ 
static int older_progress(void *p, double dltotal, double dlnow, double ultotal, double ulnow)
{
	return xferinfo(p, dltotal, dlnow, ultotal, ulnow);
}

/*
Function        :inFTPS_Initial
Date&Time       :2017/8/3 下午 2:53
Describe        :
*/
int inFTPS_Initial(TRANSACTION_OBJECT *pobTran)
{
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inFTPS_Initial() START !!");
              
        /* init libcurl */
	/* 描述： 这个函数只能用一次。(其实在调用curl_global_cleanup 函数后仍然可再用) 
	 * 如果这个函数在curl_easy_init函数调用时还没调用，它讲由libcurl库自动完成 
	 * 
	 * CURL_GLOBAL_ALL	初始化所有的可能的调用。 
	 * CURL_GLOBAL_SSL   	初始化支持 安全套接字层。 
	 * CURL_GLOBAL_WIN32 	初始化win32套接字库。 
	 * CURL_GLOBAL_NOTHING	没有额外的初始化。
	 * CURL_GLOBAL_DEFAULT == CURL_GLOBAL_ALL
	 */
        curl_global_init(CURL_GLOBAL_DEFAULT);
        /* init the curl session
	 * 描述: curl_easy_init用来初始化一个CURL的指针(有些像返回FILE类型的指针一样). 
	 * 相应的在调用结束时要用curl_easy_cleanup函数清理. 一般curl_easy_init意味着一个会话的开始. 
	 * 它的返回值一般都用在easy系列的函数中.  
	 */ 
        gpCurl = curl_easy_init();
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inFTPS_Initial() END !!");
        
        return (VS_SUCCESS);
}

/*
Function        :inFTPS_End
Date&Time       :2017/8/3 下午 4:01
Describe        :和inFTPS_Initial搭配使用
*/
int inFTPS_Deinitial(TRANSACTION_OBJECT *pobTran)
{
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inFTPS_End() START !!");
        
        /* 
	 * cleanup curl stuff
	 * 在结束libcurl使用的时候，用来对curl_global_init做的工作清理。类似于close的函数。
	 */ 
        curl_easy_cleanup(gpCurl);
	
	/* 这个调用用来结束一个会话.与curl_easy_init配合着用. */
        curl_global_cleanup();
          
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inFTPS_End() END !!");
        
        return (VS_SUCCESS);
}

/*
Function        :inFTPS_Download
Date&Time       :2017/8/4 下午 1:59
Describe        :
*/
int inFTPS_Download(FTPS_REC *srFtpsObj)
{
        int		inRetVal = 0;		/* return value，來判斷是否回傳error */
	int		inRetVal2 = 0;
	int		inTimeOut = 0;
        char		szTemplate[128 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	double		dbVal = 0;
        unsigned long   ulVal = 0;
        FTP_FILE	srFile = {};		/* name to store the file as if succesful(data.txt) */
        
	/* 設定要下載的檔案名稱和檔案流指標初始化 */
	memcpy(srFile.szFilename, srFtpsObj->szFtpsFileName, strlen(srFtpsObj->szFtpsFileName));
	srFile.pStream = NULL;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Filename = %s", srFtpsObj->szFtpsFileName);
                inLogPrintf(AT, "inFTPS_Download() START !!");
                inLogPrintf(AT, szDebugMsg);
	}
	
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_PARAM_DOWNLOAD_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 參數下載 */
	
	if (gpCurl)
	{
		/* Debug用 Switch on full protocol/debug output */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (ginDebug == VS_TRUE)
		{
			szTemplate[0] = 1;
		}
		else
		{
			szTemplate[0] = 0;
		}
		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_VERBOSE, szTemplate[0]);
		if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg, "CURLOPT_VERBOSE Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }
		
                /* 設定FTPS URL */
                inRetVal = curl_easy_setopt(gpCurl, CURLOPT_URL, srFtpsObj->szFtpsURL);
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg, "CURLOPT_URL Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                        inLogPrintf(AT, srFtpsObj->szFtpsURL);
                }
                
		/* 設定FTPS PORT */
		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_PORT, srFtpsObj->inFtpsPort);
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg, "CURLOPT_PORT Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
		}
		
		/* 設定USER ID及PW */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szTemplate, "%s:%s", srFtpsObj->szFtpsID, srFtpsObj->szFtpsPW);
                inRetVal = curl_easy_setopt(gpCurl, CURLOPT_USERPWD, szTemplate);
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg, "CURLOPT_USERPWD Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }
				
		/* 是否用SSL
		 * Enable SSL/TLS for FTP, pick one of:
		   CURLUSESSL_TRY     - try using SSL, proceed anyway otherwise
		   CURLUSESSL_CONTROL - SSL for the control connection or fail
		   CURLUSESSL_ALL     - SSL for all communication or fail
		*/
		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
		if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg, "CURLOPT_USE_SSL Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }
		
		/* SSL VERSION */
		/* openssl 1.1.1不能以TLS1.2連1.0，所以這邊改設為1.0 */
		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_0);
		if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg, "CURLOPT_SSL_VERSION Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }
		
		/* SSL 加密列表 */
		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_SSL_CIPHER_LIST, "DEFAULT");
		if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg, "CURLOPT_SSL_CIPHER_LIST Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }
			
		/* 設定行為優先跑TLS */
		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_USE_SSL, CURLFTPAUTH_SSL);
		if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg, "CURLOPT_USE_SSL Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }
		
		/* CCC */
		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_FTP_SSL_CCC, CURLFTPSSL_CCC_NONE);
		if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg, "CURLOPT_FTP_SSL_CCC Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }
		
		/* (是否驗伺服器憑證)不驗憑證，設定為0
		 * 當 CURLOPT_SSL_VERIFYPEER 為 FALSE 時，其他諸如 CURLOPT_SSL_VERIFYHOST, CURLOPT_CAINFO, CURLOPT_CAPATH 等設定，都不具任何意義。
		 */
		/* CURLOPT_SSL_VERIFYPEER 表示要不要驗 */
                /* V3C，不驗憑證，V3CT3適用PCI5，要驗憑證 */
                ulVal = 0;
                if (ginMachineType == _CASTLE_TYPE_V3C_)
                {
                        /* V3C> SN開頭為118 V3CT3> SN開頭為167 */
                        if (inFunc_Check_PCI_6_0() != VS_TRUE)
                        {
                                ulVal = 0;
                        }
                        else
                        {
                                ulVal = 1;
                        }
                }
                else
                {
                        if (inFunc_Check_PCI_6_0() != VS_TRUE)
                        {
                                ulVal = 0;
                        }
                        else
                        {
                                ulVal = 1;
                        }
                }
		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_SSL_VERIFYPEER, ulVal);
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg, "CURLOPT_SSL_VERIFYPEER Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }
		
		/* CURLOPT_SSL_VERIFYHOST 表示怎麼驗 0表示不驗 1:是否存在域名 2:是否和現在連接伺服器符合 */
		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_SSL_VERIFYHOST, 0);
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg, "CURLOPT_SSL_VERIFYHOST Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }
		
                /* 設定憑證PEM檔案名稱 */
		if (strlen(srFtpsObj->szCACertFileName) > 0)
		{
			inRetVal = curl_easy_setopt(gpCurl, CURLOPT_CAINFO, srFtpsObj->szCACertFileName);
			if (ginDebug == VS_TRUE)
			{
				sprintf(szDebugMsg, "CURLOPT_CAINFO Ret = %d", inRetVal);
				inLogPrintf(AT, szDebugMsg);
				inLogPrintf(AT, srFtpsObj->szCACertFileName);
			}
		}
		
		/* 設定憑證PEM檔案路徑資料夾(會自動搜尋資料夾內所有憑證，即，) */
		if (strlen(srFtpsObj->szCACertFilePath) > 0)
		{
			inRetVal = curl_easy_setopt(gpCurl, CURLOPT_CAPATH, srFtpsObj->szCACertFilePath);
			if (ginDebug == VS_TRUE)
			{
				sprintf(szDebugMsg, "CURLOPT_CAPATH Ret = %d", inRetVal);
				inLogPrintf(AT, szDebugMsg);
				inLogPrintf(AT, srFtpsObj->szCACertFilePath);
			}
		}
		
//		/* client憑證 */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		sprintf(szTemplate, "%s%s", _CLIENT_CER_DATA_PATH_, _PEM_CLIENT_CRT_FILE_NAME_);
//		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_SSLCERT, szTemplate);
//		if (ginDebug == VS_TRUE)
//		{
//			sprintf(szDebugMsg, "CURLOPT_SSLCERT Ret = %d", inRetVal);
//			inLogPrintf(AT, szDebugMsg);
//			inLogPrintf(AT, szTemplate);
//		}
//		
//		/* client憑證密碼 */
//		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_SSLCERTPASSWD, "password");
//		if (ginDebug == VS_TRUE)
//		{
//			sprintf(szDebugMsg, "CURLOPT_SSLCERTPASSWD Ret = %d", inRetVal);
//			inLogPrintf(AT, szDebugMsg);
//			inLogPrintf(AT, "password");
//		}
//		
//		/* client憑證格式 */
//		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_SSLCERTTYPE, "PEM");
//		if (ginDebug == VS_TRUE)
//		{
//			sprintf(szDebugMsg, "CURLOPT_SSLCERTTYPE Ret = %d", inRetVal);
//			inLogPrintf(AT, szDebugMsg);
//			inLogPrintf(AT, "PEM");
//		}
//		
//		/* client private key */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		sprintf(szTemplate, "%s%s", _CLIENT_CER_DATA_PATH_, _PEM_CLIENT_PRIVATE_KEY_FILE_NAME_);
//		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_SSLKEY, szTemplate);
//		if (ginDebug == VS_TRUE)
//		{
//			sprintf(szDebugMsg, "CURLOPT_SSLKEY Ret = %d", inRetVal);
//			inLogPrintf(AT, szDebugMsg);
//			inLogPrintf(AT, szTemplate);
//		}
//		
//		/* client private key 密碼 */
//		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_SSLKEYPASSWD, "password");
//		if (ginDebug == VS_TRUE)
//		{
//			sprintf(szDebugMsg, "CURLOPT_SSLKEYPASSWD Ret = %d", inRetVal);
//			inLogPrintf(AT, szDebugMsg);
//			inLogPrintf(AT, "password");
//		}
//		
//		/* client private key格式 */
//		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_SSLKEYTYPE, "PEM");
//		if (ginDebug == VS_TRUE)
//		{
//			sprintf(szDebugMsg, "CURLOPT_SSLKEYTYPE Ret = %d", inRetVal);
//			inLogPrintf(AT, szDebugMsg);
//			inLogPrintf(AT, "PEM");
//		}
		
		
		/* 最長允許Timeout */
		/* 最長允許3分鐘 */
		inTimeOut = 180;
		/* 如果填0會變成無限等候，直到回應，CURLOPT_TIMEOUT表示秒，CURLOPT_TIMEOUT_MS表示毫秒 */
		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_TIMEOUT, inTimeOut);
		if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg, "CURLOPT_TIMEOUT Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }
		
		/* 連接Timeout */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetCarrierTimeOut(szTemplate);
		inTimeOut = atoi(szTemplate);
		
		/* 如果填0會變成無限等候，直到回應，CURLOPT_TIMEOUT表示秒，CURLOPT_TIMEOUT_MS表示毫秒 */
		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_CONNECTTIMEOUT, inTimeOut);
		if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg, "CURLOPT_CONNECTTIMEOUT Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }
		
		/* Response Timeout */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetHostResponseTimeOut(szTemplate);
		inTimeOut = atoi(szTemplate);
		
		/* 如果填0會變成無限等候，直到回應，CURLOPT_TIMEOUT表示秒，CURLOPT_TIMEOUT_MS表示毫秒 */
		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_FTP_RESPONSE_TIMEOUT, inTimeOut);
		if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg, "CURLOPT_FTP_RESPONSE_TIMEOUT Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }
		
		/* Define our callback to get called when there's data to be written */
                /* 寫檔Function */
		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_WRITEFUNCTION, my_fwrite);
                if (ginDebug == VS_TRUE)
                {
			sprintf(szDebugMsg, "CURLOPT_WRITEFUNCTION Ret = %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
                }
                
		/* Set a pointer to our struct to pass to the callback */
                /* 寫檔callback */
		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_WRITEDATA, &srFile);
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg, "CURLOPT_WRITEDATA Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }

		/* 抓取上傳或下載時擷取資訊 */
		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_PROGRESSFUNCTION, older_progress);
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg, "CURLOPT_PROGRESSFUNCTION Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }
		
		/* 停止傳輸function */
		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_NOPROGRESS, FALSE);
		if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg, "CURLOPT_NOPROGRESS Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }
				
		if (srFtpsObj->inFtpRetryTimes > 0)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_4_);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "重試下載次數: %d", srFtpsObj->inFtpRetryTimes);
			inDISP_ChineseFont(szTemplate, _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
		}
		inDISP_Clear_Line(_LINE_8_5_, _LINE_8_8_);
		inDISP_EnglishFont(srFtpsObj->szFtpsFileName, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_); /* 顯示檔名 */
                inDISP_PutGraphic(_CONNECTING_, 0, _COORDINATE_Y_LINE_8_7_); /* 連線中 */
				
                /* 連線下載 */
		inRetVal = curl_easy_perform(gpCurl);
                
		if (inRetVal == CURLE_OK)
		{
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, srFtpsObj->szFtpsFileName);
                                sprintf(szDebugMsg, "curl_easy_perform Ret = %d", inRetVal);
                                inLogPrintf(AT, szDebugMsg);
                                
                                /* check for bytes downloaded */ 
                                inRetVal = curl_easy_getinfo(gpCurl, CURLINFO_SIZE_DOWNLOAD, &dbVal);
                                
                                if((CURLE_OK == inRetVal) && (dbVal > 0))
                                {
                                        sprintf(szDebugMsg, "Data downloaded: %0.0f bytes.", dbVal);
                                        inLogPrintf(AT, szDebugMsg);
                                }
                                
                                /* check for total download time */ 
                                inRetVal = curl_easy_getinfo(gpCurl, CURLINFO_TOTAL_TIME, &dbVal);
                                
                                if((CURLE_OK == inRetVal) && (dbVal > 0))
                                {
                                        sprintf(szDebugMsg, "Total download time: %0.3f sec.", dbVal);
                                        inLogPrintf(AT, szDebugMsg);
                                }
                                
                                /* check for average download speed */ 
                                inRetVal = curl_easy_getinfo(gpCurl, CURLINFO_SPEED_DOWNLOAD, &dbVal);
                                
                                if((CURLE_OK == inRetVal) && (dbVal > 0))
                                {
                                        sprintf(szDebugMsg, "Average download speed: %0.3f kbyte/sec.", dbVal / 1024);
                                        inLogPrintf(AT, szDebugMsg);
                                }
                                
                                /* check for name resolution time */ 
                                inRetVal = curl_easy_getinfo(gpCurl, CURLINFO_NAMELOOKUP_TIME, &dbVal);
                                
                                if((CURLE_OK == inRetVal) && (dbVal > 0))
                                {
                                        sprintf(szDebugMsg, "Name lookup time: %0.3f sec.", dbVal);
                                        inLogPrintf(AT, szDebugMsg);
                                }
                                
                                /* check for connect time */ 
                                inRetVal = curl_easy_getinfo(gpCurl, CURLINFO_CONNECT_TIME, &dbVal);
                                
                                if((CURLE_OK == inRetVal) && (dbVal > 0))
                                {
                                        sprintf(szDebugMsg, "Connect time: %0.3f sec.", dbVal);
                                        inLogPrintf(AT, szDebugMsg);
                                }
                        }
		}
		else
		{
			if (inRetVal == CURLE_REMOTE_ACCESS_DENIED)
			{
				/* 
				 * FTP找不到目錄或檔案 
				 * CURLE_REMOTE_ACCESS_DENIED 被主機拒絕(無此目錄) 
				 */
				inSetFTPInquiryResponseCode(_FTP_INQUIRY_REPORT_CAN_NOT_FIND_DIR_);
				inSetFTPDownloadResponseCode(_FTP_DOWNLOAD_REPORT_CAN_NOT_FIND_DIR_);
				inSaveTMSFTPRec(0);
			}
			else if (inRetVal == CURLE_REMOTE_FILE_NOT_FOUND)
			{
				/* FTP找不到目錄或檔案 
				 * CURLE_REMOTE_FILE_NOT_FOUND 無此檔案 
				 */
				inSetFTPInquiryResponseCode(_FTP_INQUIRY_REPORT_CAN_NOT_FIND_DIR_);
				inSetFTPDownloadResponseCode(_FTP_DOWNLOAD_REPORT_CAN_NOT_FIND_DIR_);
				inSaveTMSFTPRec(0);
			}
			else if (inRetVal == CURLE_COULDNT_CONNECT)
			{
				/* FTP連線失敗 */
				inSetFTPInquiryResponseCode(_FTP_INQUIRY_REPORT_FTP_SERVER_CONNECT_FAILED_);
				inSetFTPDownloadResponseCode(_FTP_DOWNLOAD_REPORT_FTP_SERVER_CONNECT_FAILED_);
				inSaveTMSFTPRec(0);
			}
			else
			{
				/* FTP重試失敗 */
				if (srFtpsObj->inFtpRetryTimes > 0)
				{
					inSetFTPDownloadResponseCode(_FTP_DOWNLOAD_REPORT_DOWNLOAD_RETRY_FAILED_);
					inSaveTMSFTPRec(0);
				}
			}
			
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, srFtpsObj->szFtpsFileName);
                                sprintf(szDebugMsg, "curl_easy_perform ERROR Ret = %d", inRetVal);
                                inLogPrintf(AT, szDebugMsg);
				
                                /* check for bytes downloaded */ 
                                inRetVal2 = curl_easy_getinfo(gpCurl, CURLINFO_SIZE_DOWNLOAD, &dbVal);
                                
                                if((CURLE_OK == inRetVal2) && (dbVal > 0))
                                {
                                        sprintf(szDebugMsg, "Data downloaded: %0.0f bytes.", dbVal);
                                        inLogPrintf(AT, szDebugMsg);
                                }
                                
                                /* check for total download time */ 
                                inRetVal2 = curl_easy_getinfo(gpCurl, CURLINFO_TOTAL_TIME, &dbVal);
                                
                                if((CURLE_OK == inRetVal2) && (dbVal > 0))
                                {
                                        sprintf(szDebugMsg, "Total download time: %0.3f sec.", dbVal);
                                        inLogPrintf(AT, szDebugMsg);
                                }
                                
                                /* check for average download speed */ 
                                inRetVal2 = curl_easy_getinfo(gpCurl, CURLINFO_SPEED_DOWNLOAD, &dbVal);
                                
                                if((CURLE_OK == inRetVal2) && (dbVal > 0))
                                {
                                        sprintf(szDebugMsg, "Average download speed: %0.3f kbyte/sec.", dbVal / 1024);
                                        inLogPrintf(AT, szDebugMsg);
                                }
                                
                                /* check for name resolution time */ 
                                inRetVal2 = curl_easy_getinfo(gpCurl, CURLINFO_NAMELOOKUP_TIME, &dbVal);
                                
                                if((CURLE_OK == inRetVal2) && (dbVal > 0))
                                {
                                        sprintf(szDebugMsg, "Name lookup time: %0.3f sec.", dbVal);
                                        inLogPrintf(AT, szDebugMsg);
                                }
                                
                                /* check for connect time */ 
                                inRetVal2 = curl_easy_getinfo(gpCurl, CURLINFO_CONNECT_TIME, &dbVal);
                                
                                if((CURLE_OK == inRetVal2) && (dbVal > 0))
                                {
                                        sprintf(szDebugMsg, "Connect time: %0.3f sec.", dbVal);
                                        inLogPrintf(AT, szDebugMsg);
                                }
                        }
			
                        if (srFile.pStream)
                                fclose(srFile.pStream); /* close the local file */

                        return (inRetVal);
		}
	}
        else
        {
		/* X:未定義之錯誤狀態 */
		inSetFTPInquiryResponseCode(_FTP_INQUIRY_REPORT_UNKNOWN_);
		inSetFTPDownloadResponseCode(_FTP_DOWNLOAD_REPORT_UNKNOWN_);
		inSaveTMSFTPRec(0);

                /* curl_easy_init 失敗 */
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "curl_easy_init Error!!");
                
                if (srFile.pStream)
                        fclose(srFile.pStream); /* close the local file */

                return (VS_ERROR);
        }

	if (srFile.pStream)
		fclose(srFile.pStream); /* close the local file */

	if (inRetVal == CURLE_OK)
	{
                /* 將FTPS下載下來的檔案移動到fs_data資料夾 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inRetVal = inFile_Move_File(srFtpsObj->szFtpsFileName, _AP_ROOT_PATH_, "", _FS_DATA_PATH_);
                
                /* 檔案移失敗 */
                if (inRetVal != VS_SUCCESS)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                sprintf(szDebugMsg, "inFunc_Move_Data Ret = %d", inRetVal);
                                inLogPrintf(AT, szDebugMsg);
                        }
                        
                        return (VS_ERROR);
                }
                
                /* ======test 用====== */
//		if (ginDebug == VS_TRUE)
//		{
//			long		lnFileSize = 0;
//			unsigned long	ulFileHandle = 0;
//			
//			inRetVal = inFILE_Open(&ulFileHandle, (unsigned char *)srFtpsObj->szFtpsFileName);
//			sprintf(szDebugMsg, "inFILE_Open = %d", inRetVal);
//			inLogPrintf(AT, szDebugMsg);
//
//			lnFileSize = lnFILE_GetSize(&ulFileHandle, (unsigned char *)srFtpsObj->szFtpsFileName);
//			sprintf(szDebugMsg, "lnFILE_GetSize = %ld", lnFileSize);
//			inLogPrintf(AT, szDebugMsg);	
//
//			inFILE_Close(&ulFileHandle);
//		}
                /* ======test 用====== */
	}
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inFTPS_Download() END !!");
        
	return (VS_SUCCESS);
}

/* 聯合目前沒用到上傳 */
int inFTPS_Upload(FTPS_REC *srFtpsObj)
{
//        int     inRetVal; /* return value，來判斷是否回傳error */
//        long    lnFileSize;
//        char    szTemplate[128 + 1];
	
//        /* FTPS使用 */
//	CURL		*curl;
//	CURLcode	res;
//	FILE		*hd_src;
//	curl_off_t	fsize;	
//	struct stat	file_info;
//	static char	*pCACertFile = "FTPS.pem";
//        /* FTPS使用 */
//        
//        if (ginDebug == VS_TRUE)
//                inLogPrintf(AT, "inFTPS_Upload() START !!");
//        
//	inDISP_PutGraphic(_PARAM_DOWNLOAD_TITLE_, 0, 0);
//	
//	/* get the file size of the local file */ 
//	if(stat(srFtpsObj->szFtpsFileName, &file_info)) 
//	{
//                if (ginDebug == VS_TRUE)
//                {		
//			memset(szTemplate, 0x00, sizeof(szTemplate));
//			sprintf(szTemplate, "Couldnt open '%s': %s", srFtpsObj->szFtpsFileName, strerror(errno));
//                        inLogPrintf(AT, szTemplate);
//		}
//
//		return 1;
//	}
//
//	fsize = (curl_off_t)file_info.st_size;
//
//	if (ginDebug == VS_TRUE)
//	{
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		sprintf(szTemplate, "Local file size: %lld bytes. ", fsize);
//		inLogPrintf(AT, szTemplate);
//	}
//
//	/* get a FILE * of the same file */
//	hd_src = fopen(srFtpsObj->szFtpsFileName, "rb");
//	
//	if (!hd_src)
//	{
//		if (ginDebug == VS_TRUE)
//		{
//			memset(szTemplate, 0x00, sizeof(szTemplate));
//			sprintf(szTemplate, "Local file size: %lld bytes. ", fsize);
//			inLogPrintf(AT, szTemplate);
//		}
//		
//		return(VS_ERROR);
//	}
//
////	curl = curl_easy_init();
//
//	if (curl)
//	{
//                /* 設定FTPS URL */
//                res = curl_easy_setopt(curl, CURLOPT_URL, srFtpsObj->szFtpsURL);
//
//                if (ginDebug == VS_TRUE)
//                {
//                        sprintf(szTemplate, "CURLOPT_URL = %d", res);
//                        inLogPrintf(AT, szTemplate);
//                        inLogPrintf(AT, srFtpsObj->szFtpsURL);
//                }
//                
//		/* 設定上傳模式 */ 
//		res = curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
//
//                if (ginDebug == VS_TRUE)
//                {
//			memset(szTemplate, 0x00, sizeof(szTemplate));
//                        sprintf(szTemplate, "CURLOPT_UPLOAD = %d", res);
//                        inLogPrintf(AT, szTemplate);
//                }
//
//		/* 設定上傳的檔案 */ 
//		res = curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);
//
//                if (ginDebug == VS_TRUE)
//                {
//			memset(szTemplate, 0x00, sizeof(szTemplate));
//			sprintf(szTemplate, "CURLOPT_READDATA = %d", res);
//                        inLogPrintf(AT, szTemplate);
//                }
//
//		/* 設定上傳檔案大小 */ 
//		res = curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);
//
//                if (ginDebug == VS_TRUE)
//                {
//			memset(szTemplate, 0x00, sizeof(szTemplate));
//			sprintf(szTemplate, "CURLOPT_INFILESIZE_LARGE = %d", res);
//                        inLogPrintf(AT, szTemplate);
//                }
//
//		/* enable verbose for easier tracing */ 
//		res = curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
//
//		if (ginDebug == VS_TRUE)
//                {
//			memset(szTemplate, 0x00, sizeof(szTemplate));
//			sprintf(szTemplate, "CURLOPT_VERBOSE = %d", res);
//                        inLogPrintf(AT, szTemplate);
//                }	
//
//                /* 設定憑證PEM檔案 */
//		res = curl_easy_setopt(curl, CURLOPT_CAINFO, pCACertFile);
//                
//                if (ginDebug == VS_TRUE)
//                {
//			memset(szTemplate, 0x00, sizeof(szTemplate));
//                        sprintf(szTemplate, "CURLOPT_CAINFO = %d", res);
//                        inLogPrintf(AT, szTemplate);
//                }
//                
//                /* 設定FTPS PORT */
//		res = curl_easy_setopt(curl, CURLOPT_PORT, srFtpsObj->inFtpsPort);
//                
//                if (ginDebug == VS_TRUE)
//                {
//                        sprintf(szTemplate, "CURLOPT_PORT = %d", res);
//                        inLogPrintf(AT, szTemplate);
//		}
//                
//                /* 設定USER ID及PW */
//                memset(szTemplate, 0x00, sizeof(szTemplate));
//                sprintf(szTemplate, "%s:%s", srFtpsObj->szFtpsID, srFtpsObj->szFtpsPW);
//                res = curl_easy_setopt(curl, CURLOPT_USERPWD, szTemplate);
//                
//                if (ginDebug == VS_TRUE)
//                {
//                        sprintf(szTemplate, "CURLOPT_USERPWD = %d", res);
//                        inLogPrintf(AT, szTemplate);
//                }
//                
//		/* 抓取上傳或下載時擷取資訊 */
//		res = curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, older_progress);
//                
//                if (ginDebug == VS_TRUE)
//                {
//                        sprintf(szTemplate, "CURLOPT_PROGRESSFUNCTION = %d", res);
//                        inLogPrintf(AT, szTemplate);
//                        inLogPrintf(AT, srFtpsObj->szFtpsFileName);
//                }		
//		
//		memset(gszDisp, 0x00, sizeof(gszDisp));
//		strcpy(gszDisp, srFtpsObj->szFtpsFileName);
//		
//                /* 連線下載 */
//		res = curl_easy_perform(curl);
//                memset(gszDisp, 0x00, sizeof(gszDisp));
//                
//		if (res == CURLE_OK)
//		{
//                        if (ginDebug == VS_TRUE)
//                        {
//                                sprintf(szTemplate, "curl_easy_perform = %d", res);
//                                inLogPrintf(AT, szTemplate);
//                        }
//		}
//		else
//		{
//                        if (ginDebug == VS_TRUE)
//                        {
//                                sprintf(szTemplate, "curl_easy_perform ERROR = %d", res);
//                                inLogPrintf(AT, szTemplate);
//                        }
//
//			fclose(hd_src); /* close the local file */ 
//                        return (res);
//		}
//	}
//        else
//        {
//                /* curl_easy_init 失敗 */
//                if (ginDebug == VS_TRUE)
//                        inLogPrintf(AT, "curl_easy_init Error!!");
//                
//		fclose(hd_src); /* close the local file */ 
//                return (VS_ERROR);
//        }
//
//	curl_global_cleanup();
//        
//        if (ginDebug == VS_TRUE)
//                inLogPrintf(AT, "inFTPS_Upload() END !!");
//        
	return (VS_SUCCESS);
}

int inFTPS_Test(void)
{
        TRANSACTION_OBJECT 	pobTran;
        FTPS_REC        srFtpsObj;
//        ginDebug = VS_TRUE;
        memset(&pobTran, 0x00, sizeof(TRANSACTION_OBJECT));
        memset(&srFtpsObj, 0x00, sizeof(FTPS_REC));
        
	inFLOW_RunFunction(&pobTran, _FUNCTION_TMS_CONNECT_);
	
        inFTPS_Initial(&pobTran);
        
//        strcpy(srFtpsObj.szFtpsURL, "ftps://192.168.13.106/AP/ChiaHsin/184/VX520AH-D-012/NCCCAPPL.zip");
//        srFtpsObj.inFtpsPort = 990;
//        strcpy(srFtpsObj.szFtpsID, "edcuser");
//        strcpy(srFtpsObj.szFtpsPW, "edc@123");
//        strcpy(srFtpsObj.szFtpsFileName, "NCCCAPPL.zip");
	
	strcpy(srFtpsObj.szFtpsURL, "ftp://61.220.34.44/u01/users/tmsap/AP/ChiaHsin/189/CASV3AH-D-015/NCCCAPPL.zip");
        srFtpsObj.inFtpsPort = 990;
	strcpy(srFtpsObj.szFtpsID, "EDC01001");
        strcpy(srFtpsObj.szFtpsPW, "N3cedcpw1001");
	strcpy(srFtpsObj.szFtpsFileName, "NCCCAPPL.zip");
    
        inFTPS_Download(&srFtpsObj);
        inFTPS_Deinitial(&pobTran);
	
	inFLOW_RunFunction(&pobTran, _FUNCTION_TMS_DISCONNECT_);
        
//    	ginDebug = VS_FALSE;
        return (VS_SUCCESS);
}

/* 有需要再用 目前版本 7.24 */
void vdFunc_Check_libCurl_Version(void)
{
	char sztemp[100+1];
	curl_version_info_data *d = curl_version_info(CURLVERSION_NOW);

	/* compare with the 24 bit hex number in 8 bit fields */
	if(d->version_num >= 0x072100) {
	  /* this is libcurl 7.33.0 or later */
	  inDISP_ChineseFont("7.33以上", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
	}
	else {
	  inDISP_ChineseFont("7.33以下", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
	}
	
	memset(sztemp, 0x00, sizeof(sztemp));
	sprintf(sztemp, "%d", d->version_num);
	inDISP_ChineseFont(sztemp, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
}

int inFunc_Disp_ProgressBar(char *szDispBar, char *szDispPer, curl_off_t cuTotal, curl_off_t cuNow)
{
	float	flDispPer;
	int	inPer = 0, i;
	double	dbTotal, dbNow;
	
	dbTotal = Int64ToDouble(cuTotal);
	dbNow = Int64ToDouble(cuNow);
        
	if (dbTotal > 0)
	{
		flDispPer  = dbNow / dbTotal;
		inPer = flDispPer * 100;
	}
	
	memset(szDispPer, 0x00, sizeof(szDispPer));
	sprintf(szDispPer, "%d%% ", inPer);
	
	for (i = 0; i < (inPer / 5); i ++)
	{
		strcat(szDispBar, "■");
	}

        return (inPer);
}

double Int64ToDouble(int64_t  in64)   
{   
	int   flag=0;   
	double   d;   
	
	if(in64   <   0)   
	{//负数处理   
		flag   =   1;   
		in64   =   -in64;   
	}
	
	d = (unsigned long)(in64 >> 32);   
	//   直接运算1<<32会有数值溢出   
	d *=  (1<<16);   
	d *=  (1<<16); 
	d += (unsigned long)(in64 & 0xFFFFFFFF);   
	
	if(flag)   
		d = -d;   
	
	return d;
}

/*
Function        :inFTPS_Download_SFTP
Date&Time       :2019/3/11 上午 11:30
Describe        :直接沿用前綴，實際上是FTP中的SFTP
*/
int inFTPS_Download_SFTP(FTPS_REC *srFtpsObj)
{
        int		inRetVal = 0;		/* return value，來判斷是否回傳error */
	int		inRetVal2 = 0;
	int		inTimeOut = 0;
        char		szTemplate[128 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	double		dbVal = 0;
        FTP_FILE	srFile = {};		/* name to store the file as if succesful(data.txt) */
        
	/* 設定要下載的檔案名稱和檔案流指標初始化 */
	memcpy(srFile.szFilename, srFtpsObj->szFtpsFileName, strlen(srFtpsObj->szFtpsFileName));
	srFile.pStream = NULL;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Filename = %s", srFtpsObj->szFtpsFileName);
                inLogPrintf(AT, "inFTPS_Download_SFTP() START !!");
                inLogPrintf(AT, szDebugMsg);
	}
	
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_PARAM_DOWNLOAD_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 參數下載 */
	
	if (gpCurl)
	{
		/* Debug用 Switch on full protocol/debug output */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (ginDebug == VS_TRUE)
		{
			szTemplate[0] = 1;
		}
		else
		{
			szTemplate[0] = 0;
		}
		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_VERBOSE, szTemplate[0]);
		if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg, "CURLOPT_VERBOSE Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }
		
                /* 設定FTPS URL */
                inRetVal = curl_easy_setopt(gpCurl, CURLOPT_URL, srFtpsObj->szFtpsURL);
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg, "CURLOPT_URL Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                        inLogPrintf(AT, srFtpsObj->szFtpsURL);
                }
                
		/* 設定FTPS PORT */
		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_PORT, srFtpsObj->inFtpsPort);
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg, "CURLOPT_PORT Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
		}
		
		/* 設定USER ID及PW */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szTemplate, "%s:%s", srFtpsObj->szFtpsID, srFtpsObj->szFtpsPW);
                inRetVal = curl_easy_setopt(gpCurl, CURLOPT_USERPWD, szTemplate);
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg, "CURLOPT_USERPWD Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }

		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_ANY);
		if (ginDebug == VS_TRUE)
                {
			inLogPrintf(AT, "CURLOPT_SSH_AUTH_TYPES:%d", CURLSSH_AUTH_AGENT);
                        sprintf(szDebugMsg, "CURLOPT_SSH_AUTH_TYPES Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }
		
		/* 目前sftp主機僅支援password認證 */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		sprintf(szTemplate, "%s%s", _CLIENT_CER_DATA_PATH_, _SFTP_PRIVATE_KEY_FILE_NAME_);
//		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_SSH_PRIVATE_KEYFILE, szTemplate);
//		if (ginDebug == VS_TRUE)
//                {
//			inLogPrintf(AT, "CURLOPT_SSH_PRIVATE_KEYFILE:%s", szTemplate);
//                        sprintf(szDebugMsg, "CURLOPT_SSH_PRIVATE_KEYFILE Ret = %d", inRetVal);
//                        inLogPrintf(AT, szDebugMsg);
//                }
		
		/* 最長允許Timeout */
		/* 最長允許3分鐘 */
		inTimeOut = 180;
		/* 如果填0會變成無限等候，直到回應，CURLOPT_TIMEOUT表示秒，CURLOPT_TIMEOUT_MS表示毫秒 */
		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_TIMEOUT, inTimeOut);
		if (ginDebug == VS_TRUE)
                {
			inLogPrintf(AT, "CURLOPT_CONNECTTIMEOUT:%d", inTimeOut);
                        sprintf(szDebugMsg, "CURLOPT_TIMEOUT Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }
		
		/* 連接Timeout */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetCarrierTimeOut(szTemplate);
		inTimeOut = atoi(szTemplate);
		
		/* 如果填0會變成無限等候，直到回應，CURLOPT_TIMEOUT表示秒，CURLOPT_TIMEOUT_MS表示毫秒 */
		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_CONNECTTIMEOUT, inTimeOut);
		if (ginDebug == VS_TRUE)
                {
			inLogPrintf(AT, "CURLOPT_CONNECTTIMEOUT:%d", inTimeOut);
                        sprintf(szDebugMsg, "CURLOPT_CONNECTTIMEOUT Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }
		
		/* Response Timeout */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetHostResponseTimeOut(szTemplate);
		inTimeOut = atoi(szTemplate);
		
		/* 如果填0會變成無限等候，直到回應，CURLOPT_TIMEOUT表示秒，CURLOPT_TIMEOUT_MS表示毫秒 */
		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_FTP_RESPONSE_TIMEOUT, inTimeOut);
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CURLOPT_FTP_RESPONSE_TIMEOUT:%d", inTimeOut);
			sprintf(szDebugMsg, "CURLOPT_FTP_RESPONSE_TIMEOUT Ret = %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* Define our callback to get called when there's data to be written */
                /* 寫檔Function */
		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_WRITEFUNCTION, my_fwrite);
                if (ginDebug == VS_TRUE)
                {
			sprintf(szDebugMsg, "CURLOPT_WRITEFUNCTION Ret = %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
                }
                
		/* Set a pointer to our struct to pass to the callback */
                /* 寫檔callback */
		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_WRITEDATA, &srFile);
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg, "CURLOPT_WRITEDATA Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }

		/* 抓取上傳或下載時擷取資訊 */
		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_PROGRESSFUNCTION, older_progress);
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg, "CURLOPT_PROGRESSFUNCTION Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }
		
		/* 停止傳輸function */
		inRetVal = curl_easy_setopt(gpCurl, CURLOPT_NOPROGRESS, FALSE);
		if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg, "CURLOPT_NOPROGRESS Ret = %d", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }
		
		if (srFtpsObj->inFtpRetryTimes > 0)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_4_);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "重試下載次數: %d", srFtpsObj->inFtpRetryTimes);
			inDISP_ChineseFont(szTemplate, _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
		}
		inDISP_Clear_Line(_LINE_8_5_, _LINE_8_8_);
		inDISP_EnglishFont(srFtpsObj->szFtpsFileName, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_); /* 顯示檔名 */
                inDISP_PutGraphic(_CONNECTING_, 0, _COORDINATE_Y_LINE_8_7_); /* 連線中 */
				
                /* 連線下載 */
		inRetVal = curl_easy_perform(gpCurl);
                
		if (inRetVal == CURLE_OK)
		{
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, srFtpsObj->szFtpsFileName);
                                sprintf(szDebugMsg, "curl_easy_perform Ret = %d", inRetVal);
                                inLogPrintf(AT, szDebugMsg);
                                
                                /* check for bytes downloaded */ 
                                inRetVal = curl_easy_getinfo(gpCurl, CURLINFO_SIZE_DOWNLOAD, &dbVal);
                                
                                if((CURLE_OK == inRetVal) && (dbVal > 0))
                                {
                                        sprintf(szDebugMsg, "Data downloaded: %0.0f bytes.", dbVal);
                                        inLogPrintf(AT, szDebugMsg);
                                }
                                
                                /* check for total download time */ 
                                inRetVal = curl_easy_getinfo(gpCurl, CURLINFO_TOTAL_TIME, &dbVal);
                                
                                if((CURLE_OK == inRetVal) && (dbVal > 0))
                                {
                                        sprintf(szDebugMsg, "Total download time: %0.3f sec.", dbVal);
                                        inLogPrintf(AT, szDebugMsg);
                                }
                                
                                /* check for average download speed */ 
                                inRetVal = curl_easy_getinfo(gpCurl, CURLINFO_SPEED_DOWNLOAD, &dbVal);
                                
                                if((CURLE_OK == inRetVal) && (dbVal > 0))
                                {
                                        sprintf(szDebugMsg, "Average download speed: %0.3f kbyte/sec.", dbVal / 1024);
                                        inLogPrintf(AT, szDebugMsg);
                                }
                                
                                /* check for name resolution time */ 
                                inRetVal = curl_easy_getinfo(gpCurl, CURLINFO_NAMELOOKUP_TIME, &dbVal);
                                
                                if((CURLE_OK == inRetVal) && (dbVal > 0))
                                {
                                        sprintf(szDebugMsg, "Name lookup time: %0.3f sec.", dbVal);
                                        inLogPrintf(AT, szDebugMsg);
                                }
                                
                                /* check for connect time */ 
                                inRetVal = curl_easy_getinfo(gpCurl, CURLINFO_CONNECT_TIME, &dbVal);
                                
                                if((CURLE_OK == inRetVal) && (dbVal > 0))
                                {
                                        sprintf(szDebugMsg, "Connect time: %0.3f sec.", dbVal);
                                        inLogPrintf(AT, szDebugMsg);
                                }
                        }
		}
		else
		{
			if (inRetVal == CURLE_REMOTE_ACCESS_DENIED)
			{
				/* 
				 * FTP找不到目錄或檔案 
				 * CURLE_REMOTE_ACCESS_DENIED 被主機拒絕(無此目錄) 
				 */
				inSetFTPInquiryResponseCode(_FTP_INQUIRY_REPORT_CAN_NOT_FIND_DIR_);
				inSetFTPDownloadResponseCode(_FTP_DOWNLOAD_REPORT_CAN_NOT_FIND_DIR_);
				inSaveTMSFTPRec(0);
			}
			else if (inRetVal == CURLE_REMOTE_FILE_NOT_FOUND)
			{
				/* FTP找不到目錄或檔案 
				 * CURLE_REMOTE_FILE_NOT_FOUND 無此檔案 
				 */
				inSetFTPInquiryResponseCode(_FTP_INQUIRY_REPORT_CAN_NOT_FIND_DIR_);
				inSetFTPDownloadResponseCode(_FTP_DOWNLOAD_REPORT_CAN_NOT_FIND_DIR_);
				inSaveTMSFTPRec(0);
			}
			else if (inRetVal == CURLE_COULDNT_CONNECT)
			{
				/* FTP連線失敗 */
				inSetFTPInquiryResponseCode(_FTP_INQUIRY_REPORT_FTP_SERVER_CONNECT_FAILED_);
				inSetFTPDownloadResponseCode(_FTP_DOWNLOAD_REPORT_FTP_SERVER_CONNECT_FAILED_);
				inSaveTMSFTPRec(0);
			}
			else
			{
				/* FTP重試失敗 */
				if (srFtpsObj->inFtpRetryTimes > 0)
				{
					inSetFTPDownloadResponseCode(_FTP_DOWNLOAD_REPORT_DOWNLOAD_RETRY_FAILED_);
					inSaveTMSFTPRec(0);
				}
			}
			
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, srFtpsObj->szFtpsFileName);
                                sprintf(szDebugMsg, "curl_easy_perform ERROR Ret = %d", inRetVal);
                                inLogPrintf(AT, szDebugMsg);
				
                                /* check for bytes downloaded */ 
                                inRetVal2 = curl_easy_getinfo(gpCurl, CURLINFO_SIZE_DOWNLOAD, &dbVal);
                                
                                if((CURLE_OK == inRetVal2) && (dbVal > 0))
                                {
                                        sprintf(szDebugMsg, "Data downloaded: %0.0f bytes.", dbVal);
                                        inLogPrintf(AT, szDebugMsg);
                                }
                                
                                /* check for total download time */ 
                                inRetVal2 = curl_easy_getinfo(gpCurl, CURLINFO_TOTAL_TIME, &dbVal);
                                
                                if((CURLE_OK == inRetVal2) && (dbVal > 0))
                                {
                                        sprintf(szDebugMsg, "Total download time: %0.3f sec.", dbVal);
                                        inLogPrintf(AT, szDebugMsg);
                                }
                                
                                /* check for average download speed */ 
                                inRetVal2 = curl_easy_getinfo(gpCurl, CURLINFO_SPEED_DOWNLOAD, &dbVal);
                                
                                if((CURLE_OK == inRetVal2) && (dbVal > 0))
                                {
                                        sprintf(szDebugMsg, "Average download speed: %0.3f kbyte/sec.", dbVal / 1024);
                                        inLogPrintf(AT, szDebugMsg);
                                }
                                
                                /* check for name resolution time */ 
                                inRetVal2 = curl_easy_getinfo(gpCurl, CURLINFO_NAMELOOKUP_TIME, &dbVal);
                                
                                if((CURLE_OK == inRetVal2) && (dbVal > 0))
                                {
                                        sprintf(szDebugMsg, "Name lookup time: %0.3f sec.", dbVal);
                                        inLogPrintf(AT, szDebugMsg);
                                }
                                
                                /* check for connect time */ 
                                inRetVal2 = curl_easy_getinfo(gpCurl, CURLINFO_CONNECT_TIME, &dbVal);
                                
                                if((CURLE_OK == inRetVal2) && (dbVal > 0))
                                {
                                        sprintf(szDebugMsg, "Connect time: %0.3f sec.", dbVal);
                                        inLogPrintf(AT, szDebugMsg);
                                }
                        }
			
                        if (srFile.pStream)
                                fclose(srFile.pStream); /* close the local file */

                        return (inRetVal);
		}
	}
        else
        {
		/* X:未定義之錯誤狀態 */
		inSetFTPInquiryResponseCode(_FTP_INQUIRY_REPORT_UNKNOWN_);
		inSetFTPDownloadResponseCode(_FTP_DOWNLOAD_REPORT_UNKNOWN_);
		inSaveTMSFTPRec(0);

                /* curl_easy_init 失敗 */
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "curl_easy_init Error!!");
                
                if (srFile.pStream)
                        fclose(srFile.pStream); /* close the local file */

                return (VS_ERROR);
        }

	if (srFile.pStream)
		fclose(srFile.pStream); /* close the local file */

	if (inRetVal == CURLE_OK)
	{
                /* 將FTPS下載下來的檔案移動到fs_data資料夾 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inRetVal = inFile_Move_File(srFtpsObj->szFtpsFileName, _AP_ROOT_PATH_, "", _FS_DATA_PATH_);
                
                /* 檔案移失敗 */
                if (inRetVal != VS_SUCCESS)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                sprintf(szDebugMsg, "inFunc_Move_Data Ret = %d", inRetVal);
                                inLogPrintf(AT, szDebugMsg);
                        }
                        
                        return (VS_ERROR);
                }
                
                /* ======test 用====== */
//		if (ginDebug == VS_TRUE)
//		{
//			long		lnFileSize = 0;
//			unsigned long	ulFileHandle = 0;
//			
//			inRetVal = inFILE_Open(&ulFileHandle, (unsigned char *)srFtpsObj->szFtpsFileName);
//			sprintf(szDebugMsg, "inFILE_Open = %d", inRetVal);
//			inLogPrintf(AT, szDebugMsg);
//
//			lnFileSize = lnFILE_GetSize(&ulFileHandle, (unsigned char *)srFtpsObj->szFtpsFileName);
//			sprintf(szDebugMsg, "lnFILE_GetSize = %ld", lnFileSize);
//			inLogPrintf(AT, szDebugMsg);	
//
//			inFILE_Close(&ulFileHandle);
//		}
                /* ======test 用====== */
	}
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inFTPS_Download_SFTP() END !!");
        
	return (VS_SUCCESS);
}

/*
Function        :inFTPS_Check_Curl_Version
Date&Time       :2019/3/21 下午 2:42
Describe        :回傳的文字訊息無法辨認，暫不使用
*/
int inFTPS_Check_Curl_Version(void)
{
	int	inRetVal = VS_SUCCESS;
	char	szTemplate[4000 + 1] = {0};
	char	szTemplateUTF8[2 * sizeof(szTemplate) + 1] = {0};
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	strcpy(szTemplate, (char*)curl_version);
	
	strcpy(szTemplateUTF8, szTemplate);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, szTemplateUTF8);
	}
	
	return (inRetVal);
}
