#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <ctos_tls2.h>
#include <openssl/err.h>
#include <sqlite3.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/asn1.h>
#include <dirent.h>

#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/AllStruct.h"
#include "../EVENT/MenuMsg.h"
#include "../FUNCTION/Sqlite.h"
#include "../FUNCTION/File.h"
#include "../FUNCTION/Function.h"
#include "../FUNCTION/HDT.h"
#include "../FUNCTION/HDPT.h"
#include "../FUNCTION/Utility.h"
#include "../FUNCTION/EDC.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "../COMM/Comm.h"
#include "../COMM/Ethernet.h"

#include "TLS.h"

extern int ginDebug;
extern int ginTrans_ClientFd;
extern int ginAPVersionType;
extern int ginDisplayDebug;
extern int ginMachineType;

unsigned int guiCTX_ID = 0; /* Context Object ID */
unsigned int guiSSL_ID = 0; /* SSL Object ID */
unsigned int guiSSL_ConnectBit = 0;
SSL_CTX* gCtx = NULL;
SSL* gSsl = NULL;
char gszCACertPath[100 + 1] = {0};
int ginSSLErrCode = 0;

/* TLS 流程參考(講的滿清楚的，所以我把表格複製下來了)
 * (HTTPS到底是个啥玩意儿？ https://mp.weixin.qq.com/s?__biz=MzA3MDExNzcyNA==&mid=402053009&idx=1&sn=ea531fc21a07d33f8a0408e5206c60f3)
 * 
 * 	Client				|		Server
 * 	Client  Hello			|
 * 	你好！				|
 * ----------------------------------------------------------------------------------------
 *					|		Server Hello
 *					|		嗯，你好！
 *-----------------------------------------------------------------------------------------
 *					|		Certificate
 *					|		我的证书给你，验证我吧
 *-----------------------------------------------------------------------------------------
 *					|		Server Key Exchange
 *					|		这是我给你的加密密钥相关的东东
 *-----------------------------------------------------------------------------------------
 *					|		Server Hello Done
 *					|		好，我说完了
 *-----------------------------------------------------------------------------------------
 * 	Client Key Exchange		|		
 *	这是我给你的加密密钥相关的东东	|		
 *-----------------------------------------------------------------------------------------
 *	Change Cipher Spec		|		
 *	准备转换成密文了哦		|		
 *-----------------------------------------------------------------------------------------
 *	Encrypted  Handshake Message	|		
 *	%……&*4 （密文思密达）		|		
 *-----------------------------------------------------------------------------------------
 *					|		Change Cipher Spec
 *					|		我也转换密文了
 *-----------------------------------------------------------------------------------------
 *					|		Encrypted  Handshake Message
 *					|		#%&……* （密文思密达）
 *-----------------------------------------------------------------------------------------
 *	Application  Data		|		
 *	%&￥&%*……（HTTP密文数据）	|		
 *------------------------------------------------------------------------------------------
 *					|		Application  Data
 *					|		**……&%（HTTP密文数据）
 *------------------------------------------------------------------------------------------
 *	Encrypted  Alert		|
 *	警告（实际就是说完了，拜拜~）	|
 */

/* 幾個SSL相關的函式，
 * SSL_CTX_set_verify();針對使用此context的連接是否要驗證
 * SSL_set_verify();針對使用此SSL連結的是否要驗證，此設定會覆蓋SSL_CTX_set_verify()，若同時使用則只看SSL_set_verify()的結果
 * 若需要特定驗結果可以通過，ex:自驗證書可以通過，則需要在callback裡面修改 
 * SSL_get_verify_result():若SSL_CTX_set_verify和SSL_set_verify都沒有設置驗證，則此函式回傳結果不具意義，因為沒有進行驗證
 * 設置SSL_CTX_set_verify、SSL_set_verify，若驗證有問題，則會在SSL_connect失敗。
 * 那SSL_get_verify_result()主要功能是，若你使用了自定義的驗證流程，則你可以在SSL_conect不會回傳錯誤，且可以藉由SSL_get_verify_result來取得真實驗證結果 */

/*
Function        :inTLS_Init
Date&Time       :2017/7/28 下午 5:07
Describe        :在使用TLS之前至少call過一次
 */
int inTLS_Init()
{
	if (_TLS_API_WAY_ == _TLS_API_WAY_OPENSSL_)
	{
		inTLS_OPENSSL_TLS_Init();
	} else
	{
		/* 不使用CTOS TLS API，於憑證過期或憑證錯誤會有問題 */
		inTLS_CTOS_TLS_Init();
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_Context_New
Date&Time       :2017/7/28 下午 4:58
Describe        :建立一個新的SSL Context for library使用
 */
int inTLS_CTX_New(int inMethod, unsigned int *inCTX_ID)
{
	unsigned short usRetVal = 0;

	usRetVal = CTOS_TLS2_CTX_New(inMethod, inCTX_ID);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TLS2 New context OK CTX: %u", *inCTX_ID);
		}
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TLS2 New context Err :0x%04X", usRetVal);
		}

		inUtility_StoreTraceLog_OneStep("TLS2 New context Err :0x%04X", usRetVal);

		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_CTX_LoadCACertificationFromFile
Date&Time       :2017/7/31 上午 11:03
Describe        :指定檔案讀取CA憑證
 */
int inTLS_CTX_LoadCACertificationFromFile(unsigned int uiCTX_ID, unsigned char *uszFileName)
{
	unsigned short usRetVal = 0;

	usRetVal = CTOS_TLS2_CTX_LoadCACertificationFromFile(uiCTX_ID, uszFileName);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CTX Load CA Cer OK");
		}
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CA FileName:%s", uszFileName);
			inLogPrintf(AT, "CTX Load CA Cer Err :0x%04X", usRetVal);
		}

		inUtility_StoreTraceLog_OneStep("CA FileName:%s", uszFileName);
		inUtility_StoreTraceLog_OneStep("CTX Load CA Cer Err :0x%04X", usRetVal);

		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_CTX_LoadCACertificationFromFile
Date&Time       :2017/7/31 上午 11:03
Describe        :指定檔案讀取憑證(Client)
 */
int inTLS_CTX_LoadCertificationFromFile(unsigned int uiCTX_ID, unsigned char *uszFileName, int inFiletype)
{
	unsigned short usRetVal = 0;

	usRetVal = CTOS_TLS2_CTX_LoadCertificateFromFile(uiCTX_ID, uszFileName, inFiletype);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CTX load Cer OK");
		}
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CA FileName:%s", uszFileName);
			inLogPrintf(AT, "CTX load Cer Err :0x%04X", usRetVal);
		}
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_CTX_LoadCACertificationFromFile
Date&Time       :2017/7/31 上午 11:03
Describe        :指定檔案讀取kry(Clinet)
 */
int inTLS_CTX_LoadPrivateKeyFromFile(unsigned int uiCTX_ID, unsigned char *uszFileName, int inFiletype)
{
	unsigned short usRetVal = 0;

	usRetVal = CTOS_TLS2_CTX_LoadPrivateKeyFromFile(uiCTX_ID, uszFileName, inFiletype);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CTX load key OK");
		}
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CA FileName:%s", uszFileName);
			inLogPrintf(AT, "CTX load key Err :0x%04X", usRetVal);
		}
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_CTX_SetVerificationMode
Date&Time       :2017/7/31 上午 11:42
Describe        :設定SSL Context的驗證方式
 */
int inTLS_CTX_SetVerificationMode(unsigned int uiCTX_ID, unsigned int uiMode)
{
	unsigned short usRetVal = 0;

	usRetVal = CTOS_TLS2_CTX_SetVerificationMode(uiCTX_ID, uiMode);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CTX Set Verify Mode OK");
		}
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CTX Set Verify Mode Err :0x%04X", usRetVal);
		}
		inUtility_StoreTraceLog_OneStep("CTX Set Verify Mode Err :0x%04X", usRetVal);

		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_TLS2_CTX_Free_
Date&Time       :2017/8/2 下午 3:39
Describe        :Free TLS Context Object
 */
int inTLS_CTX_Free(unsigned int *uiCTX_ID)
{
	unsigned short usRetVal = 0;

	usRetVal = CTOS_TLS2_CTX_Free(*uiCTX_ID);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TLS_CTX Free OK CTXID: %u", *uiCTX_ID);
		}
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TLS_CTX Free Err :0x%04X", usRetVal);
		}
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_TLS_New
Date&Time       :2017/7/31 上午 11:55
Describe        :創一個TLS連線
 */
int inTLS_TLS_New(unsigned int uiCTX, unsigned int *uiSSLID)
{
	unsigned short usRetVal = 0;

	usRetVal = CTOS_TLS2_New(uiCTX, uiSSLID);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TLS New OK CTX: %u,SSLID: %u", uiCTX, *uiSSLID);
		}
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TLS New Err :0x%04X", usRetVal);
		}
		inUtility_StoreTraceLog_OneStep("TLS New Err :0x%04X", usRetVal);

		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_SetVerificationMode
Date&Time       :2017/7/31 下午 1:15
Describe        :設定SSL連線的驗證方式
 * This function sets the verification mode of SSL session for remote peers
 * 
 * use either SSL_VERIFY_NONE or SSL_VERIFY_PEER, the last 2 options
 * are 'ored' with SSL_VERIFY_PEER if they are desired 
 */
int inTLS_SetVerificationMode(unsigned int uiSSLID, unsigned int uiMode)
{
	unsigned short usRetVal = 0;

	usRetVal = CTOS_TLS2_SetVerificationMode(uiSSLID, uiMode);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TLS Set Verify Mode OK");
			inLogPrintf(AT, "SSLID:%u,Mode:%u。", uiSSLID, uiMode);
		}
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TLS Set Verify Mode Err :0x%04X", usRetVal);
		}
		inUtility_StoreTraceLog_OneStep("TLS Set Verify Mode Err :0x%04X", usRetVal);

		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_SetCipherList
Date&Time       :2017/7/31 下午 3:49
Describe        :加密算法套裝列表：客戶端支持的加密-簽名算法的列表，讓伺服器去選擇
 *	szList :The null-terminated text string and a colon-delimited list of cipher suites to use with the specified SSL session.
 *	szList可以填"ALL"
 */
int inTLS_SetCipherList(unsigned int uiSSLID, unsigned char *uszList)
{
	unsigned short usRetVal = 0;

	usRetVal = CTOS_TLS2_SetCipherList(uiSSLID, uszList);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TLS Set CipherList OK");
		}
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TLS Set CipherList Err :0x%04X", usRetVal);
		}
		inUtility_StoreTraceLog_OneStep("TLS Set CipherList Err :0x%04X", usRetVal);

		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_SetSocket
Date&Time       :2017/8/1 下午 5:49
Describe        :
 */
int inTLS_SetSocket(unsigned int uiSSL_ID, int inSocketHandle)
{
	unsigned short usRetVal = 0;

	usRetVal = CTOS_TLS2_SetSocket(uiSSL_ID, inSocketHandle);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TLS Set Socket OK");
		}
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TLS Set Socket Err :0x%04X", usRetVal);
		}
		inUtility_StoreTraceLog_OneStep("TLS Set Socket Err :0x%04X", usRetVal);

		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_SetProtocolVersion
Date&Time       :2017/8/1 下午 5:52
Describe        :
 */
int inTLS_SetProtocolVersion(unsigned int uiSSL_ID, unsigned int uiProtocolVersion)
{
	unsigned short usRetVal = 0;

	usRetVal = CTOS_TLS2_SetProtocolVersion(uiSSL_ID, uiProtocolVersion);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TLS Set Protocol Version OK");
		}
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TLS Set Protocol Version Err :0x%04X", usRetVal);
		}
		inUtility_StoreTraceLog_OneStep("TLS Set Protocol Version Err :0x%04X", usRetVal);

		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_TLS2_Connect
Date&Time       :2017/8/1 下午 5:52
Describe        :
 */
int inTLS_TLS2_Connect(unsigned int uiSSL_ID)
{
	unsigned short usRetVal = 0;

	usRetVal = CTOS_TLS2_Connect(uiSSL_ID);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TLS Connect OK");
		}
	} else
	{
		//		if (ginDebug == VS_TRUE)
		//		{
		//			inLogPrintf(AT, "TLS Connect Err :0x%04X", usRetVal);
		//		}
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_Send_Data
Date&Time       :2017/8/2 上午 9:36
Describe        :Send Data的function
 */
int inTLS_Send_Data(unsigned int uiSSL_ID, unsigned char *uszData, int *inDataLen)
{
	int inTempLen = 0;
	unsigned short usRetVal = 0;

	inTempLen = *inDataLen;

	usRetVal = CTOS_TLS2_Write(uiSSL_ID, uszData, inDataLen);
	/* 若沒全部傳送完也算失敗 */
	if (usRetVal == d_OK && *inDataLen == inTempLen)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TLS Send_Data OK");
		}
	} else if (usRetVal == d_TLS2_WRITE_FAIL)
	{
		inUtility_StoreTraceLog_OneStep("inTLS_Send_Data Fail, retval: 0x%04X", usRetVal);

		return (VS_ERROR);
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TLS Send_Data Err :0x%04X", usRetVal);
		}
		inUtility_StoreTraceLog_OneStep("inTLS_Send_Data Fail, retval: 0x%04X", usRetVal);

		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_Read_Data
Date&Time       :2017/8/2 上午 11:28
Describe        :
 */
int inTLS_Read_Data(unsigned int uiSSL_ID, unsigned char *uszData, int *inDataLen)
{
	unsigned short usRetVal = 0;

	usRetVal = CTOS_TLS2_Read(uiSSL_ID, uszData, inDataLen);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TLS Read_Data OK");
		}
	} else if (usRetVal == d_TLS2_READ_FAIL)
	{
		return (VS_ERROR);
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TLS Read_Data Err :0x%04X", usRetVal);
		}
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_TLS_Disconnect
Date&Time       :2017/8/2 下午 3:27
Describe        :TLS斷線
 * uiFlag:RFU, this field should be 0.
 */
int inTLS_TLS2_Disconnect(unsigned int uiSSL_ID, unsigned int uiFlag)
{
	//	int		inRetVal = VS_ERROR;
	char szDebugMsg[100 + 1];
	//	unsigned char	uszData[1 + 1];
	//	unsigned short	usLen = 0;
	unsigned short usRetVal = 0;

	/* 保留欄位 等API文件更新再開放使用 */
	uiFlag = 0;

	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 1);
	do
	{
		usRetVal = CTOS_TLS2_Disconnect(uiSSL_ID, uiFlag);
		if (usRetVal == d_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				snprintf(szDebugMsg, sizeof (szDebugMsg), "TLS2 Disconnect OK SSL_ID: %u", uiSSL_ID);
				inLogPrintf(AT, szDebugMsg);
			}
		} else
		{
			//			do
			//			{
			//				usLen = 1;
			//				inRetVal = inETHERNET_Receive_Data_TLS(uiSSL_ID, uszData, &usLen);
			//			} while (inRetVal == VS_SUCCESS);
		}

	} while (usRetVal != d_OK && inTimerGet(_TIMER_NEXSYS_1_) != VS_SUCCESS);

	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			snprintf(szDebugMsg, sizeof (szDebugMsg), "TLS2 Disconnect Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			CTOS_TLS2_GetErrorMessage(uiSSL_ID, (unsigned char*) szDebugMsg);
			inLogPrintf(AT, szDebugMsg);
		}

		return (VS_ERROR);
	} else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inTLS_TLS2_Free
Date&Time       :2017/8/2 下午 3:35
Describe        :釋放TLSObject
 */
int inTLS_TLS2_Free(unsigned int *uiSSL_ID)
{
	unsigned short usRetVal = 0;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inTLS_TLS2_Free() START ! SSLID:%u", *uiSSL_ID);
	}

	usRetVal = CTOS_TLS2_Free(*uiSSL_ID);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TLS2 Free OK SSLID: %u", *uiSSL_ID);
		}
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TLS2 Free Err :0x%04X", usRetVal);
		}
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_TLS2_GetSession
Date&Time       :2017/8/2 下午 4:57
Describe        :確認現在Session狀態
 */
int inTLS_TLS2_GetSession(unsigned int uiSSL_ID)
{
	unsigned short usRetVal = 0;

	usRetVal = CTOS_TLS2_GetSession(uiSSL_ID);
	if (usRetVal == d_OK)
	{
		//		if (ginDebug == VS_TRUE)
		//		{
		//			inLogPrintf(AT, "TLS2 Session OK");
		//		}
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TLS2 Session Err :0x%04X", usRetVal);
		}
		inUtility_StoreTraceLog_OneStep("TLS2 Session Err :0x%04X", usRetVal);

		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_Process_CTX_Flow
Date&Time       :2017/8/2 下午 4:26
Describe        :
 */
int inTLS_Process_CTX_Flow(unsigned int *uiCTX_ID)
{
	int inRetVal = VS_ERROR;
	char szClientCertPath[100 + 1] = {0};
	char szPrivateKeyPath[100 + 1] = {0};
	char szDemoMode[2 + 1] = {0};

	memset(szDemoMode, 0x00, sizeof (szDemoMode));
	inGetDemoMode(szDemoMode);

	/* 處理context */
	/* 新建一個CTX */
	inRetVal = inTLS_CTX_New(d_TLS2_METHOD_CLIENT_TLS_V1_2, uiCTX_ID);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDisplayDebug == VS_TRUE)
		{
			/* TLS Timeout */
			unsigned char uszKey = 0;

			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("CTX New error", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, 2);
			do
			{
				uszKey = uszKBD_Key();

				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
					break;
				if (uszKey != 0)
					break;
			} while (1);
		}
		return (inRetVal);
	}

	/* 載入CA憑證 */
	if (strlen(gszCACertPath) > 0)
	{

		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "載入CA憑證 [%s]", gszCACertPath);
		}

		inRetVal = inTLS_CTX_LoadCACertificationFromFile(*uiCTX_ID, (unsigned char*) gszCACertPath);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDisplayDebug == VS_TRUE)
			{
				/* TLS Timeout */
				unsigned char uszKey = 0;

				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("CTX Load CA cer error", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
				inDISP_ChineseFont(gszCACertPath, _FONTSIZE_8X44_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, 2);
				do
				{
					uszKey = uszKBD_Key();

					if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
						break;
					if (uszKey != 0)
						break;
				} while (1);
			}
			return (inRetVal);
		}
	}

	/* 載入client憑證 */
	memset(szClientCertPath, 0x00, sizeof (szClientCertPath));
	//	if (strlen(_CLIENT_CER_DATA_PATH_) > 0)
	//	{
	//		strcat(szClientCertPath, _CLIENT_CER_DATA_PATH_);
	//	}
	//	if (strlen(_PEM_CLIENT_CRT_FILE_NAME_) > 0)
	//	{
	//		strcat(szClientCertPath, _PEM_CLIENT_CRT_FILE_NAME_);
	//	}

	if (strlen(szClientCertPath) > 0)
	{
		/* [LOG顯示][20260211] 記錄TLS使用哪個憑證  */
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, " 載入Client憑證 [%s]", szClientCertPath);
		}
		inRetVal = inTLS_CTX_LoadCertificationFromFile(*uiCTX_ID, (unsigned char*) szClientCertPath, d_TLS2_FILETYPE_PEM);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDisplayDebug == VS_TRUE)
			{
				/* TLS Timeout */
				unsigned char uszKey = 0;

				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("CTX Load cer error", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
				inDISP_ChineseFont(szClientCertPath, _FONTSIZE_8X44_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, 2);
				do
				{
					uszKey = uszKBD_Key();

					if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
						break;
					if (uszKey != 0)
						break;
				} while (1);
			}

			return (inRetVal);
		}
	}

	/* 載入client key */
	memset(szPrivateKeyPath, 0x00, sizeof (szPrivateKeyPath));
	//	if (strlen(_CLIENT_CER_DATA_PATH_) > 0)
	//	{
	//		strcat(szPrivateKeyPath, _CLIENT_CER_DATA_PATH_);
	//	}
	//	if (strlen(_PEM_CLIENT_PRIVATE_KEY_FILE_NAME_) > 0)
	//	{
	//		strcat(szPrivateKeyPath, _PEM_CLIENT_PRIVATE_KEY_FILE_NAME_);
	//	}

	if (strlen(szPrivateKeyPath) > 0)
	{
		/* [LOG顯示][20260211] 記錄TLS使用哪個憑證  */
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "載入私鑰[%s]", szPrivateKeyPath);
		}
		inRetVal = inTLS_CTX_LoadPrivateKeyFromFile(*uiCTX_ID, (unsigned char*) szPrivateKeyPath, d_TLS2_FILETYPE_PEM);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDisplayDebug == VS_TRUE)
			{
				/* TLS Timeout */
				unsigned char uszKey = 0;

				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("CTX Load private key error", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
				inDISP_ChineseFont(szPrivateKeyPath, _FONTSIZE_8X44_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, 2);
				do
				{
					uszKey = uszKBD_Key();

					if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
						break;
					if (uszKey != 0)
						break;
				} while (1);
			}

			return (inRetVal);
		}
	}

	if (ginMachineType == _CASTLE_TYPE_V3C_)
	{
		/* V3C> SN開頭為118 V3CT3> SN開頭為167 */
		if (inFunc_Check_PCI_6_0() != VS_TRUE)
		{
			/* [LOG顯示][20260211] 記錄TLS使用哪個憑證  */
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "V3C 不進行憑證驗證");
			}
			inRetVal = inTLS_CTX_SetVerificationMode(*uiCTX_ID, d_TLS2_VERIFY_NONE);
		} else
		{
			/* [LOG顯示][20260211] 記錄TLS使用哪個憑證  */
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "V3C 進行憑證驗證");
			}
			inRetVal = inTLS_CTX_SetVerificationMode(*uiCTX_ID, d_TLS2_VERIFY_PEER);
		}
	} else
	{
		if (inFunc_Check_PCI_6_0() != VS_TRUE)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "非V3C 不進行憑證驗證");
			}
			inRetVal = inTLS_CTX_SetVerificationMode(*uiCTX_ID, d_TLS2_VERIFY_NONE);
		} else
		{
			/* [LOG顯示][20260211] 記錄TLS使用哪個憑證  */
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "非V3C 進行憑證驗證");
			}
			inRetVal = inTLS_CTX_SetVerificationMode(*uiCTX_ID, d_TLS2_VERIFY_PEER);
		}
	}

	if (inRetVal != VS_SUCCESS)
	{
		if (ginDisplayDebug == VS_TRUE)
		{
			/* TLS Timeout */
			unsigned char uszKey = 0;

			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("SetVerificationMode error", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, 2);
			do
			{
				uszKey = uszKBD_Key();

				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
					break;
				if (uszKey != 0)
					break;
			} while (1);
		}
		return (inRetVal);
	}

	return (inRetVal);
}

/*
Function        :inTLS_Process_TLS_Flow
Date&Time       :2017/8/2 下午 4:26
Describe        :
 */
int inTLS_Process_TLS_Flow(unsigned int *uiCTX_ID, unsigned int *uiSSLID)
{
	int inRetVal = VS_ERROR;
	char szDebugMsg[100 + 1];

	/* 處理TLS */
	/* 建立新TLS Object */
	inRetVal = inTLS_TLS_New(*uiCTX_ID, uiSSLID);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}

	/* 設定驗證方式 */
	if (ginMachineType == _CASTLE_TYPE_V3C_)
	{
		/* V3C> SN開頭為118 V3CT3> SN開頭為167 */
		if (inFunc_Check_PCI_6_0() != VS_TRUE)
		{
			inRetVal = inTLS_SetVerificationMode(*uiSSLID, d_TLS2_VERIFY_NONE);
		} else
		{
			inRetVal = inTLS_SetVerificationMode(*uiSSLID, d_TLS2_VERIFY_PEER);
		}
	} else
	{
		if (inFunc_Check_PCI_6_0() != VS_TRUE)
		{
			inRetVal = inTLS_SetVerificationMode(*uiSSLID, d_TLS2_VERIFY_NONE);
		} else
		{
			inRetVal = inTLS_SetVerificationMode(*uiSSLID, d_TLS2_VERIFY_PEER);
		}
	}

	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}

	/* 設定加密列表 */
	inRetVal = inTLS_SetCipherList(*uiSSLID, (unsigned char*) "ALL");
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}

	/* 設定socket */
	inRetVal = inTLS_SetSocket(*uiSSLID, ginTrans_ClientFd);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}

	/* 設定協議標準 */
	if (0)
	{

	} else
	{
		inRetVal = inTLS_SetProtocolVersion(*uiSSLID, d_TLS2_VERSION_TLSV1_2);
	}

	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}

	/* 2秒內要連線 */
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 2);
	do
	{
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				snprintf(szDebugMsg, sizeof (szDebugMsg), "TLS Connect Timeout");
				inLogPrintf(AT, szDebugMsg);

				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				CTOS_TLS2_GetErrorMessage(*uiSSLID, (unsigned char*) szDebugMsg);
				inLogPrintf(AT, szDebugMsg);

				inUtility_StoreTraceLog_OneStep("TLS Connect timeout");
				inUtility_StoreTraceLog_OneStep(szDebugMsg);
			}

			return (VS_TIMEOUT);
		}

		inRetVal = inTLS_TLS2_Connect(*uiSSLID);

	} while (inRetVal != VS_SUCCESS);

	return (inRetVal);
}

/*
Function        :inTLS_TLS2_GetErrorMessage
Date&Time       :2023/11/7 下午 2:42
Describe        :
 */
int inTLS_TLS2_GetErrorMessage(unsigned int* uiSSLID, unsigned char* uszDebugMsg)
{
	CTOS_TLS2_GetErrorMessage(*uiSSLID, (unsigned char*) uszDebugMsg);

	return (VS_SUCCESS);
}

/*
Function        :inTLS_Test
Date&Time       :2017/7/28 下午 5:04
Describe        :用來做簡易測試
 */
int inTLS_Test()
{
	int i;
	int inRetVal = VS_ERROR;
	int inDataLen = 0;
	char szReadData[1024 + 1];
	unsigned int uiCTX_ID;
	unsigned int uiSSLID;
	TRANSACTION_OBJECT pobTran;

	memset(&pobTran, 0x00, sizeof (pobTran));
	inCOMM_InitCommDevice();

	pobTran.srBRec.inHDTIndex = 0;

	inCOMM_ConnectStart(&pobTran);

	/* 處理context */
	inTLS_CTX_New(d_TLS2_METHOD_CLIENT_TLS_V1_2, &uiCTX_ID);
	inTLS_CTX_LoadCACertificationFromFile(uiCTX_ID, (unsigned char*) "ca.crt");
	inTLS_CTX_LoadCertificationFromFile(uiCTX_ID, (unsigned char*) "client.csr.pem", d_TLS2_FILETYPE_PEM);
	inTLS_CTX_LoadPrivateKeyFromFile(uiCTX_ID, (unsigned char*) "client.key.pem", d_TLS2_FILETYPE_PEM);
	inTLS_CTX_SetVerificationMode(uiCTX_ID, d_TLS2_VERIFY_PEER);

	/* 處理TLS */
	inTLS_TLS_New(uiCTX_ID, &uiSSLID);
	inTLS_SetVerificationMode(uiSSLID, d_TLS2_VERIFY_NONE);
	inTLS_SetCipherList(uiSSLID, (unsigned char*) "ALL");
	inTLS_SetSocket(uiSSLID, ginTrans_ClientFd);
	inTLS_SetProtocolVersion(uiSSLID, d_TLS2_VERSION_TLSV1);

	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);

	do
	{
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			return (VS_TIMEOUT);
		}

		inRetVal = inTLS_TLS2_Connect(uiSSLID);

	} while (inRetVal != VS_SUCCESS);

	for (i = 0; i < 5; i++)
	{
		inDataLen = strlen("12345TEST");
		inTLS_Send_Data(uiSSLID, (unsigned char*) "12345TEST", &inDataLen);
		uszKBD_GetKey(30);
	}

	for (i = 0; i < 5; i++)
	{
		memset(szReadData, 0x00, sizeof (szReadData));
		inDataLen = sizeof (szReadData);
		inTLS_Read_Data(uiSSLID, (unsigned char*) szReadData, &inDataLen);
		uszKBD_GetKey(30);
	}

	inTLS_TLS2_Disconnect(uiSSLID, 0);
	inTLS_TLS2_Free(&uiSSLID);
	inTLS_CTX_Free(&uiCTX_ID);

	inCOMM_End(&pobTran);

	return (VS_SUCCESS);
}

/*
 Function        :inTLS_OPENSSL_Init
 Date&Time       :2024/8/12 下午 5:27
 Describe        :1.1.1
 */
int inTLS_OPENSSL_TLS_Init(void)
{
	int inRetVal = VS_SUCCESS;

#if OPENSSL_VERSION_NUMBER < 0x10100000L
	inRetVal = SSL_library_init();
	if (inRetVal == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "SSL_library_init Failed");
		}
		inUtility_StoreTraceLog_OneStep("SSL_library_init Failed");

		return (VS_ERROR);
	}
	SSL_load_error_strings();
	OpenSSL_add_ssl_algorithms();
#else
	// 對於 OpenSSL 1.1.0 及更高版本
	inRetVal = OPENSSL_init_ssl(0, NULL);
	if (inRetVal == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "OPENSSL_init_ssl Failed");
		}
		inUtility_StoreTraceLog_OneStep("OPENSSL_init_ssl Failed");

		return (VS_ERROR);
	}
#endif


	return (VS_SUCCESS);
}

/*
 Function        :inTLS_CTOS_TLS_Init
 Date&Time       :2024/8/13 下午 4:15
 Describe        :
 */
int inTLS_CTOS_TLS_Init(void)
{
	unsigned short usRetVal = 0;

	usRetVal = CTOS_TLS2_Init();
	;
	if (usRetVal == d_OK)
	{

	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, " Err :0x%04X", usRetVal);
		}
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_OPENSSL_Process_CTX_Flow
Date&Time       :2024/8/13 下午 5:56
Describe        :
 */
int inTLS_OPENSSL_Process_CTX_Flow(SSL_CTX **ctx)
{
	int inRetVal = VS_ERROR;
	char szCerFileFolderPath[100 + 1] = {0}; /* CAPTH使用，需指向資料夾，裡面放hash值為名稱的連結 */
	char szErr_string[256] = {0};
	char szDebugMsg[400 + 1] = {0};
	char szClientCertPath[100 + 1] = {0};
	char szPrivateKeyPath[100 + 1] = {0};
	char szDemoMode[2 + 1] = {0};
	unsigned long ulErr_code = 0;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inTLS_OPENSSL_Process_CTX_Flow() START !");
	}

	memset(szDemoMode, 0x00, sizeof (szDemoMode));
	inGetDemoMode(szDemoMode);

	/* 處理context */
	/* 新建一個CTX */
	inRetVal = inTLS_OPENSSL_CTX_New(ctx);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDisplayDebug == VS_TRUE)
		{
			/* TLS Timeout */
			unsigned char uszKey = 0;

			ulErr_code = ERR_get_error();
			ERR_error_string_n(ulErr_code, szErr_string, sizeof (szErr_string));

			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("CTX New error", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			snprintf(szDebugMsg, sizeof (szDebugMsg), "ErrCode:%lu", ulErr_code);
			inDISP_ChineseFont(szDebugMsg, _FONTSIZE_8X44_, _LINE_8_5_, _DISP_CENTER_);
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			snprintf(szDebugMsg, sizeof (szDebugMsg), "Reason:%s", szErr_string);
			inDISP_ChineseFont(szDebugMsg, _FONTSIZE_8X44_, _LINE_8_6_, _DISP_CENTER_);
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, 2);
			do
			{
				uszKey = uszKBD_Key();

				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
					break;
				if (uszKey != 0)
					break;
			} while (1);
		}
		return (inRetVal);
	}

	/* debug使用 */
	if (ginDebug == VS_TRUE)
	{
		SSL_CTX_set_info_callback(*ctx, vdTLS_OPENSSL_SSL_INFO_CALLBACK);
	}

	/* 載入CA憑證 */
	/* CAPATH */
	memset(szCerFileFolderPath, 0x00, sizeof (szCerFileFolderPath));

	if (strlen(gszCACertPath) > 0)
	{
		inRetVal = inTLS_OPENSSL_CTX_LoadCACertificationFromFile(ctx, gszCACertPath, szCerFileFolderPath);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDisplayDebug == VS_TRUE)
			{
				/* TLS Timeout */
				unsigned char uszKey = 0;

				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("CTX Load CA cer error", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				snprintf(szDebugMsg, sizeof (szDebugMsg), "ErrCode:%lu", ulErr_code);
				inDISP_ChineseFont(szDebugMsg, _FONTSIZE_8X44_, _LINE_8_5_, _DISP_CENTER_);
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				snprintf(szDebugMsg, sizeof (szDebugMsg), "Reason:%s", szErr_string);
				inDISP_ChineseFont(szDebugMsg, _FONTSIZE_8X44_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, 2);
				do
				{
					uszKey = uszKBD_Key();

					if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
						break;
					if (uszKey != 0)
						break;
				} while (1);
			}
			return (inRetVal);
		}
	}

	/* 載入client憑證 */
	memset(szClientCertPath, 0x00, sizeof (szClientCertPath));
	//	if (strlen(_CLIENT_CER_DATA_PATH_) > 0)
	//	{
	//		strcat(szClientCertPath, _CLIENT_CER_DATA_PATH_);
	//	}
	//	if (strlen(_PEM_CLIENT_CRT_FILE_NAME_) > 0)
	//	{
	//		strcat(szClientCertPath, _PEM_CLIENT_CRT_FILE_NAME_);
	//	}

	if (strlen(szClientCertPath) > 0)
	{
		inRetVal = inTLS_OPENSSL_CTX_LoadCertificationFromFile(ctx, szClientCertPath, SSL_FILETYPE_PEM);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDisplayDebug == VS_TRUE)
			{
				/* TLS Timeout */
				unsigned char uszKey = 0;

				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("CTX Load client cer error", _FONTSIZE_8X44_, _LINE_8_4_, _DISP_CENTER_);
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				snprintf(szDebugMsg, sizeof (szDebugMsg), "ErrCode:%lu", ulErr_code);
				inDISP_ChineseFont(szDebugMsg, _FONTSIZE_8X44_, _LINE_8_5_, _DISP_CENTER_);
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				snprintf(szDebugMsg, sizeof (szDebugMsg), "Reason:%s", szErr_string);
				inDISP_ChineseFont(szDebugMsg, _FONTSIZE_8X44_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, 2);
				do
				{
					uszKey = uszKBD_Key();

					if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
						break;
					if (uszKey != 0)
						break;
				} while (1);
			}

			return (inRetVal);
		}
	}

	/* 載入client key */
	memset(szPrivateKeyPath, 0x00, sizeof (szPrivateKeyPath));
	//	if (strlen(_CLIENT_CER_DATA_PATH_) > 0)
	//	{
	//		strcat(szPrivateKeyPath, _CLIENT_CER_DATA_PATH_);
	//	}
	//	if (strlen(_PEM_CLIENT_PRIVATE_KEY_FILE_NAME_) > 0)
	//	{
	//		strcat(szPrivateKeyPath, _PEM_CLIENT_PRIVATE_KEY_FILE_NAME_);
	//	}

	if (strlen(szPrivateKeyPath) > 0)
	{
		inRetVal = inTLS_OPENSSL_CTX_LoadPrivateKeyFromFile(ctx, szPrivateKeyPath, SSL_FILETYPE_PEM);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDisplayDebug == VS_TRUE)
			{
				/* TLS Timeout */
				unsigned char uszKey = 0;

				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("CTX Load private key error", _FONTSIZE_8X44_, _LINE_8_4_, _DISP_CENTER_);
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				snprintf(szDebugMsg, sizeof (szDebugMsg), "ErrCode:%lu", ulErr_code);
				inDISP_ChineseFont(szDebugMsg, _FONTSIZE_8X44_, _LINE_8_5_, _DISP_CENTER_);
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				snprintf(szDebugMsg, sizeof (szDebugMsg), "Reason:%s", szErr_string);
				inDISP_ChineseFont(szDebugMsg, _FONTSIZE_8X44_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, 2);
				do
				{
					uszKey = uszKBD_Key();

					if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
						break;
					if (uszKey != 0)
						break;
				} while (1);
			}

			return (inRetVal);
		}
	}

	/* V3C，不驗憑證，V3CT3適用PCI5，要驗憑證 */
	if (ginMachineType == _CASTLE_TYPE_V3C_)
	{
		/* V3C> SN開頭為118 V3CT3> SN開頭為167 */
		if (inFunc_Check_PCI_6_0() != VS_TRUE)
		{
			vdTLS_OPENSSL_CTX_SetVerificationMode(ctx, SSL_VERIFY_NONE, NULL);
		} else
		{
			vdTLS_OPENSSL_CTX_SetVerificationMode(ctx, SSL_VERIFY_PEER, NULL);
		}
	} else
	{
		if (inFunc_Check_PCI_6_0() != VS_TRUE)
		{
			vdTLS_OPENSSL_CTX_SetVerificationMode(ctx, SSL_VERIFY_NONE, NULL);
		} else
		{
			vdTLS_OPENSSL_CTX_SetVerificationMode(ctx, SSL_VERIFY_PEER, NULL);
		}
	}

	//	/* 設定加密列表 */
	//        /* "DEFAULT"、"ALL" */
	//        /* 安全性和兼容性 "DHE-RSA-AES256-SHA:DHE-DSS-AES256-SHA:AES256-SHA:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA:AES128-SHA" */
	//        /* TLS1.2 "ECDHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256" */
	//        /* TLS1.3 高安全性套件"TLS_AES_128_GCM_SHA256:TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256" */
	//        inFunc_ShellCommand_System("openssl ciphers -v");
	//	inRetVal = inTLS_OPENSSL_CTX_SetCipherList(ctx, "DHE-RSA-AES256-SHA:DHE-DSS-AES256-SHA:AES256-SHA:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA:AES128-SHA");
	//	if (inRetVal != VS_SUCCESS)
	//	{
	//		return (inRetVal);
	//	}

	/* 設定協議標準 */
	inRetVal = inTLS_OPENSSL_CTX_Set_Max_Protocol_Version(ctx, TLS_MAX_VERSION);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}

	inRetVal = inTLS_OPENSSL_CTX_Set_Min_Protocol_Version(ctx, TLS1_VERSION);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}

	/* 設定timeout，預設為7200秒，設定為7200秒 */
	inRetVal = inTLS_OPENSSL_CTX_Set_Timeout(ctx, 7200);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}

	return (inRetVal);
}

/*
Function        :inTLS_OPENSSL_CTX_New
Date&Time       :2024/8/14 上午 9:48
Describe        :建立一個新的SSL Context for library使用
 */
int inTLS_OPENSSL_CTX_New(SSL_CTX **ctx)
{
	const SSL_METHOD *method = TLS_client_method();
	*ctx = SSL_CTX_new(method);
	if (*ctx == NULL)
	{
		vdTLS_OPENSSL_SSL_Get_Error(AT, NULL, NULL);

		return (VS_ERROR);
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "SSL_CTX_new OK");
		}
	}


	return (VS_SUCCESS);
}

/*
Function        :inTLS_OPENSSL_CTX_LoadCACertificationFromFile
Date&Time       :2024/8/14 上午 10:43
Describe        :指定檔案讀取CA憑證
 */
int inTLS_OPENSSL_CTX_LoadCACertificationFromFile(SSL_CTX **ctx, char* szFileName, char* szCAPATH)
{
	int inRetVal = 0;
	char *szCATempPath = NULL;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inTLS_OPENSSL_CTX_LoadCACertificationFromFile() START !");

		inLogPrintf(AT, "連線憑證 CA FileName:%s", szFileName);
		inLogPrintf(AT, "憑證路徑 CA Path:%s", szCAPATH);
	}

	if (strlen(szCAPATH) > 0)
	{
		szCATempPath = szCAPATH;
	}

	inRetVal = SSL_CTX_load_verify_locations(*ctx, szFileName, szCATempPath);
	if (inRetVal == 0)
	{
		vdTLS_OPENSSL_SSL_Get_Error(AT, NULL, NULL);
		return (VS_ERROR);
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CTX Load CA Cer OK");
		}
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_OPENSSL_CTX_LoadCertificationFromFile
Date&Time       :2024/8/14 下午 3:28
Describe        :指定檔案讀取憑證(Client)
 *               SSL_FILETYPE_PEM
 *               SSL_FILETYPE_ASN1
 */
int inTLS_OPENSSL_CTX_LoadCertificationFromFile(SSL_CTX **ctx, char* szFileName, int inType)
{
	int inRetVal = 0;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inTLS_OPENSSL_CTX_LoadCertificationFromFile() START !");

		inLogPrintf(AT, "客戶端驗證 CA FileName:%s", szFileName);
	}

	inRetVal = SSL_CTX_use_certificate_file(*ctx, szFileName, inType);
	if (inRetVal <= 0)
	{
		vdTLS_OPENSSL_SSL_Get_Error(AT, NULL, NULL);

		return (VS_ERROR);
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CTX load Client Cer OK");
		}
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_OPENSSL_CTX_LoadPrivateKeyFromFile
Date&Time       :2024/8/14 下午 5:35
Describe        :指定檔案讀取kry(Clinet)
 */
int inTLS_OPENSSL_CTX_LoadPrivateKeyFromFile(SSL_CTX **ctx, char* szFileName, int inType)
{
	int inRetVal = 0;

	inRetVal = SSL_CTX_use_PrivateKey_file(*ctx, szFileName, inType);
	if (inRetVal <= 0)
	{
		vdTLS_OPENSSL_SSL_Get_Error(AT, NULL, NULL);

		return (VS_ERROR);
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CTX load Client Key OK");
		}
	}


	return (VS_SUCCESS);
}

/*
Function        :vdTLS_OPENSSL_CTX_SetVerificationMode
Date&Time       :2024/8/15 下午 4:10
Describe        :設定SSL Context的驗證方式
 */
void vdTLS_OPENSSL_CTX_SetVerificationMode(SSL_CTX **ctx, int inMode, SSL_verify_cb callback)
{
	/* void */
	SSL_CTX_set_verify(*ctx, inMode, callback);
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "CTX Set Verify Mode OK");
		inLogPrintf(AT, "Mode = %d", inMode);
		if (callback != NULL)
		{
			inLogPrintf(AT, "callback != NULL");
		} else
		{
			inLogPrintf(AT, "callback == NULL");
		}
	}
}

/*
Function        :inTLS_OPENSSL_CTX_Free
Date&Time       :2024/8/15 下午 5:55
Describe        :Free TLS Context Object
 */
void vdTLS_OPENSSL_CTX_Free(SSL_CTX **ctx)
{
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "vdTLS_OPENSSL_CTX_Free() START !");
	}

	/* void */
	/* 如果 ctx 為 NULL，調用 SSL_CTX_free() 不會有任何作用 */
	SSL_CTX_free(*ctx);
	*ctx = NULL;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "vdTLS_OPENSSL_CTX_Free() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
}

/*
Function        :inTLS_OPENSSL_CTX_SetCipherList
Date&Time       :2024/8/16 下午 3:29
Describe        :加密算法套裝列表：客戶端支持的加密-簽名算法的列表，讓伺服器去選擇
 *	szList :The null-terminated text string and a colon-delimited list of cipher suites to use with the specified SSL session.
 *	szList可以填"ALL"
 */
int inTLS_OPENSSL_CTX_SetCipherList(SSL_CTX **ctx, char *szList)
{
	int inRetVal = 0;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inTLS_OPENSSL_CTX_SetCipherList(%s) START !", szList);
	}

	inRetVal = SSL_CTX_set_ciphersuites(*ctx, szList);
	if (inRetVal != 1)
	{
		vdTLS_OPENSSL_SSL_Get_Error(AT, NULL, NULL);

		return (VS_ERROR);
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "SSL_set_cipher_list OK");
		}
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inTLS_OPENSSL_CTX_SetCipherList() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_OPENSSL_CTX_Set_Max_Protocol_Version
Date&Time       :2024/8/16 下午 3:52
Describe        :TLS1_2_VERSION、TLS_MAX_VERSION
 */
int inTLS_OPENSSL_CTX_Set_Max_Protocol_Version(SSL_CTX **ctx, int inVersion)
{
	int inRetVal = 0;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inTLS_OPENSSL_CTX_Set_Max_Protocol_Version(%04X) START !", inVersion);
	}

	inRetVal = SSL_CTX_set_max_proto_version(*ctx, inVersion);
	if (inRetVal != 1)
	{
		vdTLS_OPENSSL_SSL_Get_Error(AT, NULL, NULL);

		return (VS_ERROR);
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "SSL_CTX_set_max_proto_version OK");
		}
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inTLS_OPENSSL_CTX_Set_Max_Protocol_Version() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_OPENSSL_CTX_Set_Min_Protocol_Version
Date&Time       :2024/8/16 下午 3:52
Describe        :TLS1_2_VERSION、TLS_MAX_VERSION
 */
int inTLS_OPENSSL_CTX_Set_Min_Protocol_Version(SSL_CTX **ctx, int inVersion)
{
	int inRetVal = 0;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inTLS_OPENSSL_CTX_Set_Min_Protocol_Version(%04X) START !", inVersion);
	}

	inRetVal = SSL_CTX_set_min_proto_version(*ctx, inVersion);
	if (inRetVal != 1)
	{
		vdTLS_OPENSSL_SSL_Get_Error(AT, NULL, NULL);

		return (VS_ERROR);
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "SSL_CTX_set_min_proto_version OK");
		}
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inTLS_OPENSSL_CTX_Set_Min_Protocol_Version() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_OPENSSL_CTX_Set_Timeout
Date&Time       :2024/8/19 上午 10:50
Describe        :t: 要設置的超時時間（以秒為單位）。這個時間是會話在緩存中保持有效的時間。
 */
int inTLS_OPENSSL_CTX_Set_Timeout(SSL_CTX **ctx, int inTimeout)
{
	int inOldTimeout = 0;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inTLS_OPENSSL_CTX_Set_Timeout(%d) START !", inTimeout);
	}

	inOldTimeout = SSL_CTX_set_timeout(*ctx, inTimeout);

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "SSL_CTX_set_timeout OK");
		inLogPrintf(AT, "old timeout %d", inOldTimeout);
	}


	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "SSL_CTX_set_timeout() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_OPENSSL_Process_SSL_Flow
Date&Time       :2024/8/15 下午 4:38
Describe        :
 */
int inTLS_OPENSSL_Process_SSL_Flow(SSL_CTX **ctx, SSL **ssl)
{
	int inRetVal = VS_ERROR;

	/* 處理TLS */
	/* 建立新TLS Object */
	inRetVal = inTLS_OPENSSL_SSL_New(ctx, ssl);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}

	/* 設定驗證方式 */
	/* 於ctx處設置，SSL就不再設置，避免混亂 */
	//	if (ginMachineType == _CASTLE_TYPE_V3C_)
	//	{
	//		/* V3C> SN開頭為118 V3CT3> SN開頭為167 */
	//		if (inFunc_Check_PCI_6_0() != VS_TRUE)
	//		{
	//			vdTLS_OPENSSL_SSL_SetVerificationMode(*ssl, SSL_VERIFY_NONE, NULL);
	//		}
	//		else
	//		{
	//			vdTLS_OPENSSL_SSL_SetVerificationMode(*ssl, SSL_VERIFY_PEER, NULL);
	//		}
	//	}
	//	else
	//	{
	//		if (inFunc_Check_PCI_6_0() != VS_TRUE)
	//		{
	//			vdTLS_OPENSSL_SSL_SetVerificationMode(*ssl, SSL_VERIFY_NONE, NULL);
	//		}
	//		else
	//		{
	//			vdTLS_OPENSSL_SSL_SetVerificationMode(*ssl, SSL_VERIFY_PEER, NULL);
	//		}
	//	}


	/* 設定加密列表 */
	/* 統一在CTX控制 */
	//	inRetVal = inTLS_OPENSSL_SetCipherList(*ssl, "ALL");
	//	if (inRetVal != VS_SUCCESS)
	//	{
	//		return (inRetVal);
	//	}

	/* 設定socket */
	inRetVal = inTLS_OPENSSL_SSL_SetSocket(ssl, ginTrans_ClientFd);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}

	/* 2秒內要連線 */
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 2);
	do
	{
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			vdTLS_OPENSSL_SSL_Get_Error(AT, ssl, NULL);

			return (VS_TIMEOUT);
		}

		inRetVal = inTLS_OPENSSL_SSL_Connect(ssl);

		if(inRetVal == SSL_ERROR_WANT_READ || inRetVal == SSL_ERROR_WANT_WRITE)
			CTOS_Delay(50);
		
	} while (inRetVal != VS_SUCCESS);

	return (inRetVal);
}

/*
Function        :inTLS_OPENSSL_SSL_New
Date&Time       :2024/8/16 下午 1:23
Describe        :創一個TLS連線
 */
int inTLS_OPENSSL_SSL_New(SSL_CTX **ctx, SSL **ssl)
{
	*ssl = SSL_new(*ctx);
	if (*ssl == NULL)
	{
		vdTLS_OPENSSL_SSL_Get_Error(AT, ssl, NULL);

		return (VS_ERROR);
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "SSL_new OK CTX");
		}
	}


	return (VS_SUCCESS);
}

/*
Function        :vdTLS_OPENSSL_SSL_SetVerificationMode
Date&Time       :2024/8/16 下午 2:02
Describe        :設定SSL連線的驗證方式

 */
void vdTLS_OPENSSL_SSL_SetVerificationMode(SSL **ssl, int inMode, SSL_verify_cb callback)
{
	/* void */
	SSL_set_verify(*ssl, inMode, callback);
	/* void */
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "SSL Set Verify Mode OK");
		inLogPrintf(AT, "Mode = %d", inMode);
		if (callback != NULL)
		{
			inLogPrintf(AT, "callback != NULL");
		} else
		{
			inLogPrintf(AT, "callback == NULL");
		}
	}
}

/*
Function        :inTLS_OPENSSL_SSL_SetCipherList
Date&Time       :2024/8/16 下午 2:56
Describe        :加密算法套裝列表：客戶端支持的加密-簽名算法的列表，讓伺服器去選擇
 *	szList :The null-terminated text string and a colon-delimited list of cipher suites to use with the specified SSL session.
 *	szList可以填"ALL"
 */
int inTLS_OPENSSL_SSL_SetCipherList(SSL **ssl, char *szList)
{
	int inRetVal = 0;

	inRetVal = SSL_set_cipher_list(*ssl, szList);
	if (inRetVal != 1)
	{
		vdTLS_OPENSSL_SSL_Get_Error(AT, ssl, &inRetVal);

		return (VS_ERROR);
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "SSL_set_cipher_list OK");
		}
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_OPENSSL_SSL_SetSocket
Date&Time       :2024/8/16 下午 3:34
Describe        :
 */
int inTLS_OPENSSL_SSL_SetSocket(SSL **ssl, int inSocketHandle)
{
	int inRetVal = 0;

	inRetVal = SSL_set_fd(*ssl, inSocketHandle);
	if (inRetVal != 1)
	{
		vdTLS_OPENSSL_SSL_Get_Error(AT, ssl, &inRetVal);

		return (VS_ERROR);
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "SSL_set_fd OK");
		}
	}

	return (VS_SUCCESS);
}

/*
Function        :inTLS_OPENSSL_SSL_Connect
Date&Time       :2024/8/19 下午 1:14
Describe        :
 */
int inTLS_OPENSSL_SSL_Connect(SSL **ssl)
{
	int inRetVal = VS_ERROR;

	/* 避免回傳太多log */
	//        if (ginDebug == VS_TRUE)
	//        {
	//                inLogPrintf(AT, "----------------------------------------");
	//                inLogPrintf(AT, "inTLS_OPENSSL_SSL_Connect() START !");
	//        }

	inRetVal = SSL_connect(*ssl);
	if (inRetVal == 1)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "SSL_connect OK");
		}
		guiSSL_ConnectBit = VS_TRUE;
	} else
	{
		/* 避免回傳太多log，timeout時再抓 */
		//                vdTLS_OPENSSL_SSL_Get_Error(AT, ssl, inRetVal);

		return (VS_ERROR);
	}

	/* 避免回傳太多log */
	//        if (ginDebug == VS_TRUE)
	//        {
	//                inLogPrintf(AT, "inTLS_OPENSSL_SSL_Connect() END !");
	//                inLogPrintf(AT, "----------------------------------------");
	//        }

	return (VS_SUCCESS);
}

/*
Function        :inTLS_OPENSSL_Send_Data
Date&Time       :2024/8/19 下午 2:00
Describe        :Send Data的function
 */
int inTLS_OPENSSL_SSL_Send_Data(SSL **ssl, unsigned char *uszData, int *inDataLen)
{
	int inTempSentLen = 0;
	int inAlreadySentLen = 0;
	int inSSL_Err = 0;

	do
	{
		inTempSentLen = 0;
		inTempSentLen = SSL_write(*ssl, uszData + inAlreadySentLen, *inDataLen - inAlreadySentLen);
		/* 若沒全部傳送完也算失敗 */
		if (inTempSentLen > 0)
		{
			inAlreadySentLen += inTempSentLen;
			if (inTempSentLen > 0)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "SSL Send_Data_Temp(%d)", inTempSentLen);
				}
			}

			if (inAlreadySentLen >= *inDataLen)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "SSL Send_Data(%d)all OK", inAlreadySentLen);
				}
				break;
			}
		} else
		{
			inSSL_Err = SSL_get_error(*ssl, inTempSentLen);
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SSL_write error");
			}

			if (inSSL_Err == SSL_ERROR_WANT_READ || inSSL_Err == SSL_ERROR_WANT_WRITE)
			{
				continue;
			} else
			{
				vdTLS_OPENSSL_SSL_Get_Error(AT, ssl, &inTempSentLen);
				break;
			}
		}

		/* 直接使用外面的計時器 */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SSL Send_Data(%d)Timeout", inAlreadySentLen);
			}
			vdTLS_OPENSSL_SSL_Get_Error(AT, ssl, &inTempSentLen);
			break;
		}
	} while (1);

	if (inAlreadySentLen >= *inDataLen)
	{
		*inDataLen = inAlreadySentLen;

		return (VS_SUCCESS);
	} else
	{
		*inDataLen = inAlreadySentLen;

		return (VS_ERROR);
	}
}

/*
Function        :inTLS_OPENSSL_SSL_Read_Data
Date&Time       :2024/8/19 下午 3:47
Describe        :
 */
int inTLS_OPENSSL_SSL_Read_Data(SSL **ssl, unsigned char *uszData, int *inDataLen)
{
	int inTempReadLen = 0;
	int inAlreadyReadLen = 0;
	int inSSL_Err = 0;

	do
	{
		inTempReadLen = 0;
		inTempReadLen = SSL_read(*ssl, uszData + inAlreadyReadLen, *inDataLen - inAlreadyReadLen);
		/* 若沒全部傳送完也算失敗 */
		if (inTempReadLen > 0)
		{
			inAlreadyReadLen += inTempReadLen;
			if (inTempReadLen > 0)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "SSL_read_Temp(%d)", inTempReadLen);
				}
			}

			if (inAlreadyReadLen >= *inDataLen)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "SSL_read(%d)all OK", inAlreadyReadLen);
				}
				break;
			}
		} else
		{
			inSSL_Err = SSL_get_error(*ssl, inTempReadLen);

			if (inSSL_Err == SSL_ERROR_WANT_READ || inSSL_Err == SSL_ERROR_WANT_WRITE)
			{
				continue;
			} else
			{
				vdTLS_OPENSSL_SSL_Get_Error(AT, ssl, &inTempReadLen);
				break;
			}
		}

		/* 直接使用外面的計時器 */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			vdTLS_OPENSSL_SSL_Get_Error(AT, ssl, &inTempReadLen);
			break;
		}
	} while (1);

	if (inAlreadyReadLen >= *inDataLen)
	{
		*inDataLen = inAlreadyReadLen;

		return (VS_SUCCESS);
	} else
	{
		*inDataLen = inAlreadyReadLen;

		return (VS_ERROR);
	}
}

/*
Function        :inTLS_OPENSSL_TLS2_Disconnect
Date&Time       :2024/8/16 下午 4:48
Describe        :TLS斷線
 * 雙向關閉: SSL_shutdown() 可能需要被調用兩次。第一次調用後，可能只完成了一半的關閉，這時候會返回 0，表示還需要再調用一次 SSL_shutdown() 來完成完整的雙向關閉。
 * 返回值
 * 0: 返回 0 表示關閉過程只完成了一半，當前的連接已經接收到 "close_notify" 的通知，但還未發送 "close_notify"。
 * 1: 返回 1 表示關閉過程已經完全完成，雙方都收到了對方的 "close_notify" 通知。
 * -1: 返回 -1 表示關閉過程中出現錯誤，可以使用 SSL_get_error() 來獲取具體的錯誤信息。
 */
int inTLS_OPENSSL_TLS_Disconnect(SSL **ssl)
{
	int inRetVal = VS_ERROR;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inTLS_OPENSSL_TLS2_Disconnect() START !");
	}

	if (guiSSL_ConnectBit != VS_TRUE)
	{

		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "No need to disconnect");
		}
		return (VS_SUCCESS);
	}

	inRetVal = SSL_shutdown(*ssl);
	if (inRetVal == 1)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "SSL_shutdown OK");
		}
		guiSSL_ConnectBit = VS_FALSE;
	}		/* 回傳0要再關一次*/
	else if (inRetVal == 0)
	{
		inRetVal = SSL_shutdown(*ssl);
		if (inRetVal <= 0)
		{
			vdTLS_OPENSSL_SSL_Get_Error(AT, ssl, &inRetVal);
			guiSSL_ConnectBit = VS_FALSE;

			return (VS_ERROR);
		} else
		{
			guiSSL_ConnectBit = VS_FALSE;
		}
	} else
	{
		vdTLS_OPENSSL_SSL_Get_Error(AT, ssl, &inRetVal);
		guiSSL_ConnectBit = VS_FALSE;

		return (VS_ERROR);
	}


	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inTLS_OPENSSL_TLS2_Disconnect() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :vdTLS_OPENSSL_TLS2_Free
Date&Time       :2024/8/16 下午 5:14
Describe        :釋放TLSObject
 */
void vdTLS_OPENSSL_TLS_Free(SSL **ssl)
{
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "vdTLS_OPENSSL_TLS2_Free() START !");
	}

	/* void */
	SSL_free(*ssl);
	*ssl = NULL;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "TLS2 Free OK");
	}
}

/*
 Function        :vdTLS_OPENSSL_SSL_INFO_CALLBACK
 Date&Time       :2024/8/20 上午 10:34
 Describe        :
 */
void vdTLS_OPENSSL_SSL_INFO_CALLBACK(const SSL *ssl, int inWhere, int inRet)
{
	int w = inWhere & ~SSL_ST_MASK;
	char szStr[100 + 1] = {0};

	if (w & SSL_ST_CONNECT)
	{
		snprintf(szStr, sizeof (szStr), "SSL_connect");
	} else if (w & SSL_ST_ACCEPT)
	{
		snprintf(szStr, sizeof (szStr), "SSL_accept");
	} else
	{
		snprintf(szStr, sizeof (szStr), "undefined");
	}

	inLogPrintf(AT, "Operation: %s, State: %s", szStr, SSL_state_string_long(ssl));

	// Handle loop states during handshake or other continuous operations
	if (inWhere & SSL_CB_LOOP)
	{
		inLogPrintf(AT, "%s: Looping in state %s", szStr, SSL_state_string_long(ssl));
	}		// Handle alert messages (warnings or errors during SSL/TLS operations)
	else if (inWhere & SSL_CB_ALERT)
	{
		const char *alertType = (inWhere & SSL_CB_READ) ? "read" : "write";
		inLogPrintf(AT, "SSL3 alert [%s]: %s : %s", alertType, SSL_alert_type_string_long(inRet), SSL_alert_desc_string_long(inRet));
	}		// Handle exit cases where operations complete or fail
	else if (inWhere & SSL_CB_EXIT)
	{
		if (inRet == 0)
		{
			inLogPrintf(AT, "%s: Failed in state %s", szStr, SSL_state_string_long(ssl));
		} else if (inRet < 0)
		{
			inLogPrintf(AT, "%s: Error occurred in state %s", szStr, SSL_state_string_long(ssl));
		} else
		{
			inLogPrintf(AT, "%s: Successfully completed state %s", szStr, SSL_state_string_long(ssl));
		}
	}		// Additional case for handling handshake start and finish
	else if (inWhere & SSL_CB_HANDSHAKE_START)
	{
		inLogPrintf(AT, "%s: Handshake started", szStr);
	} else if (inWhere & SSL_CB_HANDSHAKE_DONE)
	{
		inLogPrintf(AT, "%s: Handshake completed", szStr);
	}		// Handle unexpected cases for thorough debugging
	else
	{
		inLogPrintf(AT, "Unhandled state: %s with return code %d", SSL_state_string_long(ssl), inRet);
	}
}

/*
 Function        :vdTLS_OPENSSL_SSL_Get_Error
 Date&Time       :2024/8/20 上午 10:34
 Describe        :
 */
void vdTLS_OPENSSL_SSL_Get_Error(char* szlocation, SSL **ssl, int *inRet)
{
	int inError = 0;
	int inErr_code = 0;
	char szSSLError[256 + 1] = {0};

	if (inRet != NULL && ssl != NULL)
	{
		inError = SSL_get_error(*ssl, *inRet);
		switch (inError)
		{
			case SSL_ERROR_NONE:
				// 成功
				break;
			case SSL_ERROR_WANT_READ:
				// 當前操作需要等待更多數據
				break;
			case SSL_ERROR_WANT_WRITE:
				// 當前操作需要等待可寫事件
				break;
			case SSL_ERROR_SYSCALL:
				// 系統調用錯誤，檢查 errno 或其他系統錯誤
				perror("SSL_read failed");
				break;
			case SSL_ERROR_SSL:
				// SSL 底層錯誤，檢查具體錯誤信息
				break;
			default:
				// 處理其他錯誤
				break;
		}

		if (inError == SSL_ERROR_NONE)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SSL_ERROR_NONE");
			}
		} else if (inError == SSL_ERROR_WANT_READ)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SSL_ERROR_WANT_READ");
			}
		} else if (inError == SSL_ERROR_WANT_WRITE)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SSL_ERROR_WANT_WRITE");
			}
		} else
		{
			inErr_code = ERR_get_error();
			ERR_error_string_n(inErr_code, szSSLError, sizeof (szSSLError));

			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inError = %d", inError);
				inLogPrintf(AT, "%s, Err Code:%lu", szlocation, inErr_code);
				inLogPrintf(AT, "Reason:%s", szSSLError);
			}

			inUtility_StoreTraceLog_OneStep("inError = %d", inError);
			inUtility_StoreTraceLog_OneStep("%s, Err Code:%lu", szlocation, inErr_code);
			inUtility_StoreTraceLog_OneStep("Reason:%s", szSSLError);
			vdUtility_SYSFIN_LogMessage(AT, "inError = %d", inError);
			vdUtility_SYSFIN_LogMessage(AT, "%s, Err Code:%lu", szlocation, inErr_code);
			vdUtility_SYSFIN_LogMessage(AT, "Reason:%s", szSSLError);
		}
	} else
	{
		inErr_code = ERR_get_error();
		ERR_error_string_n(inErr_code, szSSLError, sizeof (szSSLError));

		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "%s, Err Code:%d", szlocation, inErr_code);
			inLogPrintf(AT, "Reason:%s", szSSLError);
		}

		inUtility_StoreTraceLog_OneStep("%s, Err Code:%d", szlocation, inErr_code);
		inUtility_StoreTraceLog_OneStep("Reason:%s", szSSLError);
		vdUtility_SYSFIN_LogMessage(AT, "%s, Err Code:%d", szlocation, inErr_code);
		vdUtility_SYSFIN_LogMessage(AT, "Reason:%s", szSSLError);
	}

	ginSSLErrCode = inErr_code;
}

void vdTLS_print_certificate_info(const char* cert_path) 
{
		
	char szDispMesg[512] = {0};
	X509* cert = NULL;
	int cert_count = 0;
	
    FILE* fp = fopen(cert_path, "r");
    if (!fp) {
		inPRINT_ChineseFont("無法開啟憑證檔案",_PRT_ISO_);
        return;
    }

	
	// 1. 取得檔案總長度 (Total Length)
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET); // 回到檔案開頭
	inPRINT_ChineseFont("檔案名稱",_PRT_ISO_);
	//inPRINT_ChineseFont(cert_path,1);
	memset(szDispMesg, 0x00, sizeof(szDispMesg));
	snprintf(szDispMesg,sizeof(szDispMesg), "檔案總長度: %ld Byte", file_size);
	inPRINT_ChineseFont(szDispMesg,_PRT_ISO_);
   

	while ((cert = PEM_read_X509(fp, NULL, NULL, NULL)) != NULL) 
	{
        cert_count++;
		memset(szDispMesg, 0x00, sizeof(szDispMesg));
		snprintf(szDispMesg,sizeof(szDispMesg), "[%d] 憑證資訊:", cert_count);
		inPRINT_ChineseFont(szDispMesg,_PRT_ISO_);
		inPRINT_SpaceLine(1);
        // 提取主體 (Subject)
//        char *subject = X509_NAME_oneline(X509_get_subject_name(cert), NULL, 0);
//		memset(szDispMesg, 0x00, sizeof(szDispMesg));
//		snprintf(szDispMesg, sizeof(szDispMesg), "Subject: %s", subject);
//		vdTLS_print_in_32byte_blocks(szDispMesg);

		vdTLS_print_time_32bytes_fixed(X509_get0_notBefore(cert), "Not Before");
		vdTLS_print_time_32bytes_fixed(X509_get0_notAfter(cert),  "Not After ");

//		if (subject) OPENSSL_free(subject);
        X509_free(cert); // 務必釋放當前憑證，否則會記憶體洩漏
    }

    if (cert_count == 0) 
	{
		inPRINT_ChineseFont("檔案中找不到有效的憑證", _PRT_ISO_);
    }

    fclose(fp);

}

// 核心函式：強制以 32 Bytes 為單位進行填充與輸出
void vdTLS_print_in_32byte_blocks(const char* input_str) 
{

    int total_len = strlen(input_str);
    int offset = 0;
    char block[33] = {0};

    if (total_len == 0) {
        inPRINT_ChineseFont("????????",_PRT_ISO_);
        return;
    }

    while (offset < total_len) {
        int remaining = total_len - offset;
        
        // 初始化 block 為全空白
        memset(block, ' ', 32);

        if (remaining >= 32) {
            // 剩餘長度足夠填滿一整行
            memcpy(block, input_str + offset, 32);
            offset += 32;
        } else {
            // 剩餘長度不足 32，填入剩餘部分，後面維持空白
            memcpy(block, input_str + offset, remaining);
            offset += remaining; // 結束迴圈
        }

		inPRINT_ChineseFont(block,_PRT_ISO_);
    }

}


// 輔助函式：將 ASN1_TIME 轉為固定 32 Bytes 輸出 (不依賴 ASN1_TIME_print_fp)
void vdTLS_print_time_32bytes_fixed(const ASN1_TIME *atime, const char* label) 
{

    char block[33] = {0};
    memset(block, ' ', 32); // 預填空白

	vdTLS_print_in_32byte_blocks(label);

    if (atime && atime->data) {
        // ASN1_TIME 的 data 可能是 "YYYYMMDDHHMMSSZ" 或 "YYMMDDHHMMSSZ"
        int raw_len = atime->length;
        int copy_len = (raw_len > 32) ? 32 : raw_len;
        
        memcpy(block, atime->data, copy_len);
    }

	vdTLS_print_in_32byte_blocks(block);
	inPRINT_SpaceLine(1);

}

