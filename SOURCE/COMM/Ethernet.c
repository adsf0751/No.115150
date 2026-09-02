#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <curl/curl.h>
#include <sqlite3.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/AllStruct.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DisTouch.h"
#include "../DISPLAY/DispMsg.h"
#include "../PRINT/Print.h"
#include "../FUNCTION/Function.h"
#include "../EVENT/MenuMsg.h"
#include "../FUNCTION/Sqlite.h"
#include "../FUNCTION/CFGT.h"
#include "../FUNCTION/HDT.h"
#include "../FUNCTION/HDPT.h"
#include "../FUNCTION/CPT.h"
#include "../FUNCTION/EDC.h"
#include "../FUNCTION/Utility.h"
#include "../FUNCTION/TMSIPDT.h"
#include "../FUNCTION/File.h"
#include "../../NCCC/NCCCtmsCPT.h"
#include "../../NCCC/NCCCtmsFTP.h"
#include "Comm.h"
#include "TLS.h"
#include "Ethernet.h"

extern int ginDebug; /* Debug使用 extern */
extern int ginISODebug;
extern int ginTrans_ClientFd;
extern unsigned int guiCTX_ID; /* TLS Context ID */
extern unsigned int guiSSL_ID; /* TLS Session ID */
extern SSL_CTX* gCtx;
extern SSL* gSsl;
extern int ginDisplayDebug;
extern char gszCACertPath[100 + 1];

int ginEthernetFlow = -1; /* (1:TLS 2:Native 3:CTOS)紀錄Ethernet走哪個流程，用這個紀錄是因為下TMS有可能修改到CFGT，所以IFES Mode可能會被改到，導致無法斷線 */

/*
Function        :inETHERNET_Open
Date&Time       :2017/7/18 下午 3:36
Describe        :
 */
int inETHERNET_Open()
{
	char szDebugMsg[100 + 1];
	unsigned short usRetVal;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inETHERNET_Open() START !");
	}

	usRetVal = CTOS_EthernetOpen();
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "ETHERNET_OPEN OK!");
		}
	} else if (usRetVal == d_ETHERNET_ALREADY_OPENED)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "ETHERNET_ALREADY_OPENED");
			inLogPrintf(AT, szDebugMsg);
		}
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "CTOS_EthernetOpen Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		vdUtility_SYSFIN_LogMessage(AT, "CTOS_EthernetOpen Err :0x%04X", usRetVal);
		return (VS_ERROR);
	}


	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inETHERNET_Open() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Close
Date&Time       :2017/7/19 上午 11:21
Describe        :
 */
int inETHERNET_Close()
{
	char szDebugMsg[100 + 1];
	unsigned short usRetVal;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inETHERNET_Close() START !");
	}

	usRetVal = CTOS_EthernetClose();
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "ETHERNET_CLOSE OK!");
		}
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "CTOS_EthernetClose Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}


	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inETHERNET_Close() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Cofig_Set
Date&Time       :2017/7/18 下午 3:45
Describe        :
 */
int inETHERNET_Cofig_Set(unsigned char uszTag, unsigned char *uszValue, unsigned char uszLen)
{
	char szDebugMsg[100 + 1];
	unsigned short usRetVal = 0x00;

	usRetVal = CTOS_EthernetConfigSet(uszTag, uszValue, uszLen);
	if (usRetVal == d_OK)
	{

	} else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			snprintf(szDebugMsg, sizeof(szDebugMsg), "CTOS_EthernetConfigSet Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			snprintf(szDebugMsg, sizeof(szDebugMsg), "Tag: %d Len:%d Value:%s", uszTag, uszLen, uszValue);
			inLogPrintf(AT, szDebugMsg);
		}

		vdUtility_SYSFIN_LogMessage(AT, "CTOS_EthernetConfigSet Err :0x%04X", usRetVal);
		vdUtility_SYSFIN_LogMessage(AT, "Tag: %d Len:%d Value:%s", uszTag, uszLen, uszValue);
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Cofig_Get
Date&Time       :2018/5/15 下午 5:26
Describe        :
 */
int inETHERNET_Cofig_Get(unsigned char uszTag, unsigned char *uszValue, unsigned char *uszLen)
{
	char szDebugMsg[100 + 1] = {0};
	unsigned short usRetVal;

	usRetVal = CTOS_EthernetConfigGet(uszTag, uszValue, uszLen);
	if (usRetVal == d_OK)
	{

	} else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			snprintf(szDebugMsg, sizeof(szDebugMsg), "CTOS_EthernetConfigGet Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			snprintf(szDebugMsg, sizeof(szDebugMsg), "Tag: %d Len:%d Value:%s", uszTag, *uszLen, uszValue);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Connect_Flow
Date&Time       :2017/7/18 下午 4:53
Describe        :分流使用CTOS還是使用原生C的Connect
 */
int inETHERNET_Connect_Flow(char *szHostIP, char *szPort)
{
	int inRetVal = VS_ERROR;
	char szI_FES_Mode[2 + 1] = {0};
	char szTRTFileName[12 + 1] = {0};
	char szDebugMsg[500 + 1] = {0};
	char szCFESMode[2 + 1] = {0};

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inETHERNET_Connect_Flow() START !");

		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "IP:%s Port:%s", szHostIP, szPort);
		inLogPrintf(AT, szDebugMsg);

	}

	if (ginISODebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "IP:%s Port:%s", szHostIP, szPort);
		inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
	}
	vdUtility_SYSFIN_LogMessage(AT, "IP:%s Port:%s", szHostIP, szPort);

	/* 一律先重置Handle */
	ginTrans_ClientFd = -1;

	/* 用原生C Socket */
	memset(szI_FES_Mode, 0x00, sizeof (szI_FES_Mode));
	inGetI_FES_Mode(szI_FES_Mode);
	memset(szCFESMode, 0x00, sizeof (szCFESMode));
	inGetCloud_MFES(szCFESMode);
	memset(szTRTFileName, 0x00, sizeof (szTRTFileName));
	inGetTRTFileName(szTRTFileName);

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "IFES: %s,CFES: %s,%s", szI_FES_Mode, szCFESMode, szTRTFileName);
		inLogPrintf(AT, szDebugMsg);
	}

	/* 判斷是IFES及非大來，其他全跑IFES */
	/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
	/* CFES也要跑TLS */
	if (memcmp(szI_FES_Mode, "Y", strlen("Y")) == 0 ||
		memcmp(szCFESMode, "Y", strlen("Y")) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Connect Flow: TLS");
			inLogPrintf(AT, szDebugMsg);
		}

		inRetVal = inETHERNET_Connect_TLS_Flow(szHostIP, szPort);
		/* 標示現在連線方式，斷線後重新恢復成_ETHERNET_FLOW_CTOS_ */
		ginEthernetFlow = _ETHERNET_FLOW_IFES_;
	}		/* 用native方式建Socket，之後要優化再考慮使用(優點自己決定Timeout等等細微設定) */
	else if (1)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Connect Flow: Native");
			inLogPrintf(AT, szDebugMsg);
		}

		inRetVal = inETHERNET_Connect_By_Native(szHostIP, szPort);
		/* 標示現在連線方式，斷線後重新恢復成_ETHERNET_FLOW_CTOS_ */
		ginEthernetFlow = _ETHERNET_FLOW_NATIVE_;

	}		/* 虹堡API */
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Connect Flow: CTOS");
			inLogPrintf(AT, szDebugMsg);
		}

		inRetVal = inETHERNET_Connect();
		/* 標示現在連線方式，斷線後重新恢復成_ETHERNET_FLOW_CTOS_ */
		ginEthernetFlow = _ETHERNET_FLOW_CTOS_;
	}

	unsigned int uiStatus = 0;
	/* Get the status of the Ethernet */
	inETHERNET_Get_Status(&uiStatus);
	memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
	if (uiStatus & d_STATUS_ETHERNET_CONNECTED)
	{
		strcat(szDebugMsg, "Connected ");
	} else
	{
		strcat(szDebugMsg, "NotConnected ");
	}

	if (uiStatus & d_STATUS_ETHERNET_PHYICAL_ONLINE)
	{
		strcat(szDebugMsg, "Phyical_Online ");
	} else
	{
		strcat(szDebugMsg, "NotPhyical_Online ");
	}

	if (uiStatus & d_STATUS_ETHERNET_RX_READY)
	{
		strcat(szDebugMsg, "RxReady ");
	} else
	{
		strcat(szDebugMsg, "NotRxReady ");
	}

	if (uiStatus & d_STATUS_ETHERNET_TX_BUSY)
	{
		strcat(szDebugMsg, "TxBusy ");
	} else
	{
		strcat(szDebugMsg, "NotTxBusy ");
	}
	vdUtility_SYSFIN_LogMessage(AT, "(%08X),%s", uiStatus, szDebugMsg);
	vdUtility_SYSFIN_LogMessage(AT, "Ethernet Cconect:(way:%d)", ginEthernetFlow);

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inETHERNET_Connect_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (inRetVal);
}

/*
Function        :inETHERNET_Connect
Date&Time       :2017/7/18 下午 4:53
Describe        :ctos api
 */
int inETHERNET_Connect()
{
	char szDebugMsg[100 + 1];
	unsigned short usRetVal = 0;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inETHERNET_Connect() START !");
	}

	usRetVal = CTOS_EthernetConnect();
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CTOS_EthernetConnect OK!");
		}
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "CTOS_EthernetConnect Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}


	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inETHERNET_Connect() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Connect_By_Native
Date&Time       :2017/7/31 下午 4:31
Describe        :原生C Socket連線
 */
int inETHERNET_Connect_By_Native(char *szHostIP, char *szPort)
{
	int inRetVal = 0;
	int inOption = 0;
	int inFlags = 0;
	char szDebugMsg[100 + 1];
	struct sockaddr_in stAddr; /* 放Host address的結構 */

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inETHERNET_Connect_By_Native() START !");
	}

	/* 初始化address */
	memset(&stAddr, 0x00, sizeof (stAddr)); //clear all zero.
	stAddr.sin_family = AF_INET;
	stAddr.sin_addr.s_addr = inet_addr(szHostIP);
	stAddr.sin_port = htons(atoi(szPort));

	ginTrans_ClientFd = socket(AF_INET, SOCK_STREAM, 0);
	if (ginTrans_ClientFd == -1)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Create Socket Fail");
			inLogPrintf(AT, szDebugMsg);
		}
		/* [BUG] 移除檔案同步API SYNC()指令
		 * [20260506] 修改記錄LOG,不使用 inUtility_StoreTraceLog_OneStep() 
		 * 轉而使用 vdUtility_SYSFIN_LogMessage() */
		vdUtility_SYSFIN_LogMessage(AT,"INC_Cont_ER Create Socket Fail, errno:%d", errno);

		return (VS_ERROR);
	} else
	{
		inFile_Open_File_Cnt_Increase();
	}

	/* 系统預設的狀態發送與接收一次為8688 Bytes(約為8.5K) 應該夠用，所以不必特別調整buffer大小 */


	/* 開關 ON */
	inOption = 1;
	/* 允許重用本地位址和埠 */
	setsockopt(ginTrans_ClientFd, SOL_SOCKET, SO_REUSEADDR, &inOption, sizeof (inOption));
	/* 設定MSS(maximum segment size) 預設為536，和520同步解決宏遠電信問題，含header設為1400，扣掉header(54)應該設為1346 */
	inOption = 1346;
	setsockopt(ginTrans_ClientFd, SOL_TCP, TCP_MAXSEG, &inOption, sizeof (inOption));

	/* Linux内核中对connect的超时时间限制是75s， Soliris 9是几分钟，因此通常认为是75s到几分钟不等*/
	/* 師爺給我翻譯翻譯：不用非阻塞Timeut就是75秒，所以一定要用非阻塞 */
	/* 先獲得現在Handle的開關狀態 */
	inFlags = fcntl(ginTrans_ClientFd, F_GETFL, 0);
	/* 把非阻塞開關On起來(做or運算)，並設定回去 */
	fcntl(ginTrans_ClientFd, F_SETFL, inFlags | O_NONBLOCK);

	/* 連線Timout 2秒 */
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 2);

	do
	{
		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				snprintf(szDebugMsg, sizeof(szDebugMsg), "connect Errno: %d", errno);
				inLogPrintf(AT, szDebugMsg);

				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				snprintf(szDebugMsg, sizeof(szDebugMsg), "Connect Timeout");
				inLogPrintf(AT, szDebugMsg);
			}
			/* [BUG] 移除檔案同步API SYNC()指令
			* [20260506] 修改記錄LOG,不使用 inUtility_StoreTraceLog_OneStep() 
			* 轉而使用 vdUtility_SYSFIN_LogMessage() */
			vdUtility_SYSFIN_LogMessage(AT,"Ethernet Connect Timeout Errno:%d inRetVal:%d", errno, inRetVal);
			close(ginTrans_ClientFd);
			inFile_Open_File_Cnt_Decrease();
			ginTrans_ClientFd = -1;

			return (VS_TIMEOUT);
		}

		inRetVal = connect(ginTrans_ClientFd, (void*) &stAddr, sizeof (stAddr));
		/* 通常只有同一台機器內Clinet連server，才有可能馬上成功 */
		if (inRetVal == 0 || errno == EISCONN)
		{
			inRetVal = 0;
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				snprintf(szDebugMsg, sizeof(szDebugMsg), "Connect Succeed imediately");
				inLogPrintf(AT, szDebugMsg);
			}
		} else
		{
			/* 釋放 CPU，休眠 50ms (避免原先 100% 狂刷) */
			CTOS_Delay(50);
		}

	} while (inRetVal != 0);

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inETHERNET_Connect_By_Native() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Connect_TLS_Flow
Date&Time       :2017/8/2 下午 4:10
Describe        :
 */
int inETHERNET_Connect_TLS_Flow(char *szHostIP, char *szPort)
{
	int inRetVal = VS_ERROR;
	char szDebugMsg[64] = {0};

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inETHERNET_Connect_TLS_Flow() START !");
	}

	/* 1.建socket */
	inRetVal = inETHERNET_Connect_By_Native(szHostIP, szPort);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}

	/* 先用新憑證 */
	memset(gszCACertPath, 0x00, sizeof (gszCACertPath));

	if (strlen(_CA_DATA_PATH_) > 0)
	{
		strncat(gszCACertPath, _CA_DATA_PATH_, sizeof(gszCACertPath) - strlen(gszCACertPath) - 1);
	}
	if (strlen(_PEM_NEW_TLS_FILE_NAME_) > 0)
	{
		strncat(gszCACertPath, _PEM_NEW_TLS_FILE_NAME_, sizeof(gszCACertPath) - strlen(gszCACertPath) - 1);
	}

	if (ginISODebug == VS_TRUE)
	{
		FILE* fp = fopen(gszCACertPath, "r");
		if (!fp) {
			inPRINT_ChineseFont("無法開啟憑證檔案",_PRT_ISO_);
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			snprintf(szDebugMsg, sizeof(szDebugMsg), " TMS PEM(1):%s ", gszCACertPath);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
		}else
		{
			// 1. 取得檔案總長度 (Total Length)
			fseek(fp, 0, SEEK_END);
			long file_size = ftell(fp);
			fclose(fp);

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			snprintf(szDebugMsg, sizeof(szDebugMsg), "PEM(1):%s ", gszCACertPath);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			snprintf(szDebugMsg, sizeof(szDebugMsg), "PEM Size[%ld] ", file_size);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
		}
	}
	
	do
	{
		/* 2.處理CTX */
		if (_TLS_API_WAY_ == _TLS_API_WAY_OPENSSL_)
		{
			inRetVal = inTLS_OPENSSL_Process_CTX_Flow(&gCtx);
			if (inRetVal != VS_SUCCESS)
			{
				vdTLS_OPENSSL_CTX_Free(&gCtx);
				inETHERNET_DisConnect_By_Native();
				break;
			}
		} else
		{
			inRetVal = inTLS_Process_CTX_Flow(&guiCTX_ID);
			if (inRetVal != VS_SUCCESS)
			{
				inTLS_CTX_Free(&guiCTX_ID);
				inETHERNET_DisConnect_By_Native();
				break;
			}
		}

		/* 3,處理TLS */
		if (_TLS_API_WAY_ == _TLS_API_WAY_OPENSSL_)
		{
			inRetVal = inTLS_OPENSSL_Process_SSL_Flow(&gCtx, &gSsl);
			if (inRetVal == VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "First CER Success");
					inLogPrintf(AT, "inETHERNET_Connect_TLS_Flow() END !");
					inLogPrintf(AT, "----------------------------------------");
				}

				return (inRetVal);
			} else
			{
				inTLS_OPENSSL_TLS_Disconnect(&gSsl);
				vdTLS_OPENSSL_TLS_Free(&gSsl);
				vdTLS_OPENSSL_CTX_Free(&gCtx);
				inETHERNET_DisConnect_By_Native();
			}
		} else
		{
			inRetVal = inTLS_Process_TLS_Flow(&guiCTX_ID, &guiSSL_ID);
			if (inRetVal == VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "First CER Success");
					inLogPrintf(AT, "inETHERNET_Connect_TLS_Flow() END !");
					inLogPrintf(AT, "----------------------------------------");
				}

				return (inRetVal);
			} else
			{
				inTLS_TLS2_Disconnect(guiSSL_ID, 0);
				inTLS_TLS2_Free(&guiSSL_ID);
				inTLS_CTX_Free(&guiCTX_ID);
				inETHERNET_DisConnect_By_Native();
			}
		}

		break;
	} while (1);

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "First CER Fail");
	}

	/* 重建socket */
	inRetVal = inETHERNET_Connect_By_Native(szHostIP, szPort);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}

	/* 換舊憑證 */
	memset(gszCACertPath, 0x00, sizeof (gszCACertPath));
	if (strlen(_CA_DATA_PATH_) > 0)
	{
		strncat(gszCACertPath, _CA_DATA_PATH_, sizeof(gszCACertPath) - strlen(gszCACertPath) - 1);
	}
	if (strlen(_PEM_PRESERVE_TLS_FILE_NAME_) > 0)
	{
		strncat(gszCACertPath, _PEM_PRESERVE_TLS_FILE_NAME_, sizeof(gszCACertPath) - strlen(gszCACertPath) - 1);
	}

	if (ginISODebug == VS_TRUE)
	{
		FILE* fp = fopen(gszCACertPath, "r");
		if (!fp) {
			inPRINT_ChineseFont("無法開啟憑證檔案",_PRT_ISO_);
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			snprintf(szDebugMsg, sizeof(szDebugMsg), " TMS PEM(2):%s ", gszCACertPath);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
		}else
		{
			// 1. 取得檔案總長度 (Total Length)
			fseek(fp, 0, SEEK_END);
			long file_size = ftell(fp);
			fclose(fp);

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			snprintf(szDebugMsg, sizeof(szDebugMsg), "PEM(2):%s ", gszCACertPath);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			snprintf(szDebugMsg, sizeof(szDebugMsg), "PEM Size[%ld] ", file_size);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
		}
	}
        
	do
	{
		/* 2.處理CTX */
		if (_TLS_API_WAY_ == _TLS_API_WAY_OPENSSL_)
		{
			inRetVal = inTLS_OPENSSL_Process_CTX_Flow(&gCtx);
			if (inRetVal != VS_SUCCESS)
			{
				vdTLS_OPENSSL_CTX_Free(&gCtx);
				inETHERNET_DisConnect_By_Native();
				break;
			}
		} else
		{
			inRetVal = inTLS_Process_CTX_Flow(&guiCTX_ID);
			if (inRetVal != VS_SUCCESS)
			{
				inTLS_CTX_Free(&guiCTX_ID);
				inETHERNET_DisConnect_By_Native();
				break;
			}
		}

		/* 3,處理TLS */
		if (_TLS_API_WAY_ == _TLS_API_WAY_OPENSSL_)
		{
			inRetVal = inTLS_OPENSSL_Process_SSL_Flow(&gCtx, &gSsl);
			if (inRetVal == VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "Second CER Success");
					inLogPrintf(AT, "inETHERNET_Connect_TLS_Flow() END !");
					inLogPrintf(AT, "----------------------------------------");
				}

				return (inRetVal);
			} else
			{
				inTLS_OPENSSL_TLS_Disconnect(&gSsl);
				vdTLS_OPENSSL_TLS_Free(&gSsl);
				vdTLS_OPENSSL_CTX_Free(&gCtx);
				inETHERNET_DisConnect_By_Native();
			}
		} else
		{
			inRetVal = inTLS_Process_TLS_Flow(&guiCTX_ID, &guiSSL_ID);
			if (inRetVal == VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "Second CER Success");
					inLogPrintf(AT, "inETHERNET_Connect_TLS_Flow() END !");
					inLogPrintf(AT, "----------------------------------------");
				}

				return (inRetVal);
			} else
			{
				inTLS_TLS2_Disconnect(guiSSL_ID, 0);
				inTLS_TLS2_Free(&guiSSL_ID);
				inTLS_CTX_Free(&guiCTX_ID);
				inETHERNET_DisConnect_By_Native();
			}
		}

		break;
	} while (1);

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "Second CER Fail");
		inLogPrintf(AT, "inETHERNET_Connect_TLS_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (inRetVal);
}

/*
Function        :inETHERNET_DisConnect_Flow
Date&Time       :2017/8/2 下午 5:35
Describe        :分流
 */
int inETHERNET_DisConnect_Flow()
{
	int inRetVal = VS_ERROR;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inETHERNET_DisConnect_Flow() START !");
	}
	vdUtility_SYSFIN_LogMessage(AT, "Ethernet Disconect:(way:%d)", ginEthernetFlow);

	if (ginEthernetFlow == _ETHERNET_FLOW_IFES_)
	{
		inRetVal = inETHERNET_DisConnect_TLS_Flow();
		if (inRetVal == VS_SUCCESS)
		{
			ginEthernetFlow = _ETHERNET_FLOW_CTOS_;
		}
	}		/* 用native方式建Socket，之後要優化再考慮使用(優點自己決定Timeout等等細微設定) */
	else if (ginEthernetFlow == _ETHERNET_FLOW_NATIVE_)
	{
		inRetVal = inETHERNET_DisConnect_By_Native();
		if (inRetVal == VS_SUCCESS)
		{
			ginEthernetFlow = _ETHERNET_FLOW_CTOS_;
		}
	}		/* 虹堡API */
	else
	{
		inRetVal = inETHERNET_DisConnect();
		if (inRetVal == VS_SUCCESS)
		{
			ginEthernetFlow = _ETHERNET_FLOW_CTOS_;
		}
	}


	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inETHERNET_DisConnect_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	if (inRetVal == VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "Ethernet Disconect Success");
		return (VS_SUCCESS);
	} else
	{
		vdUtility_SYSFIN_LogMessage(AT, "Ethernet Disconect Fail");
		return (VS_ERROR);
	}
}

/*
Function        :inETHERNET_DisConnect
Date&Time       :2017/7/19 上午 11:30
Describe        :
 */
int inETHERNET_DisConnect()
{
	char szDebugMsg[100 + 1];
	unsigned short usRetVal;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inETHERNET_DisConnect() START !");
	}

	usRetVal = CTOS_EthernetDisconnect();
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CTOS_EthernetDisconnect OK!");
		}
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "CTOS_EthernetDisconnect Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}


	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inETHERNET_DisConnect() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_DisConnect_By_Native
Date&Time       :2017/8/2 下午 5:37
Describe        :
 */
int inETHERNET_DisConnect_By_Native()
{
	int inRetVal = VS_ERROR;
	char szDebugMsg[100 + 1];

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inETHERNET_DisConnect_By_Native() START !");
	}

	/* 防呆，小於0會存取到錯的記憶體 */
	if (ginTrans_ClientFd > 0)
	{
		inRetVal = close(ginTrans_ClientFd);
	} else
	{
		inRetVal = VS_ERROR;
	}

	if (inRetVal == 0)
	{
		inRetVal = VS_SUCCESS;
		inFile_Open_File_Cnt_Decrease();
		ginTrans_ClientFd = -1;
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Disconnect Errno: %d", errno);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inETHERNET_DisConnect_By_Native() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_DisConnect_TLS_Flow
Date&Time       :2017/8/2 下午 5:45
Describe        :
 */
int inETHERNET_DisConnect_TLS_Flow()
{
	int inRetVal = VS_ERROR;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inETHERNET_DisConnect_TLS_Flow() START !");
	}

	if (_TLS_API_WAY_ == _TLS_API_WAY_OPENSSL_)
	{
		inTLS_OPENSSL_TLS_Disconnect(&gSsl);
		vdTLS_OPENSSL_TLS_Free(&gSsl);
		vdTLS_OPENSSL_CTX_Free(&gCtx);
		inRetVal = inETHERNET_DisConnect_By_Native();
	} else
	{
		inTLS_TLS2_Disconnect(guiSSL_ID, 0);

		inRetVal = inTLS_TLS2_Free(&guiSSL_ID);
		/* 失敗也要繼續，否則會沒清掉ctx(有建成功CTX，但還沒建TLS的狀況)*/

		inRetVal = inTLS_CTX_Free(&guiCTX_ID);

		inRetVal = inETHERNET_DisConnect_By_Native();
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inETHERNET_DisConnect_TLS_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Send_Ready_Flow
Date&Time       :2017/8/1 下午 2:19
Describe        :分流
 */
int inETHERNET_Send_Ready_Flow()
{
	int inRetVal = VS_ERROR;

	if (ginEthernetFlow == _ETHERNET_FLOW_IFES_)
	{
		if (_TLS_API_WAY_ == _TLS_API_WAY_OPENSSL_)
		{
			inRetVal = inETHERNET_Send_Ready_TLS_OPENSSL(&gSsl);
		} else
		{
			inRetVal = inETHERNET_Send_Ready_TLS_CTOS(guiSSL_ID);
		}
	}		/* 用native方式建Socket，之後要優化再考慮使用(優點自己決定Timeout等等細微設定) */
	else if (ginEthernetFlow == _ETHERNET_FLOW_NATIVE_)
	{
		inRetVal = inETHERNET_Send_Ready_By_Native(ginTrans_ClientFd);
	}		/* 虹堡API */
	else
	{
		inRetVal = inETHERNET_Send_Ready();
	}

	return (inRetVal);
}

/*
Function        :inETHERNET_Send_Ready
Date&Time       :2017/7/18 下午 5:28
Describe        :表示現在Ethernet可以傳送資料
 */
int inETHERNET_Send_Ready()
{
	char szDebugMsg[100 + 1];
	unsigned short usRetVal;

	usRetVal = CTOS_EthernetTxReady();
	if (usRetVal == d_OK)
	{

	} else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inETHERNET_Send_Ready Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		inUtility_StoreTraceLog_OneStep("inETHERNET_Send_Ready Fail, Err :0x%04X", usRetVal);

		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Send_Ready_By_Native
Date&Time       :2017/8/1 下午 1:40
Describe        :
 */
int inETHERNET_Send_Ready_By_Native(int inFileHandle)
{
	fd_set stWfd; /* 用來判斷是否可以寫的Fd */
	struct timeval stTimeVal;

	stTimeVal.tv_sec = 0;
	stTimeVal.tv_usec = 0;

	/* 將 set 整個清為零。 */
	FD_ZERO(&stWfd);
	/* 將 fd 新增到 set。 */
	FD_SET(inFileHandle, &stWfd);


	/* nfds is the highest-numbered file descriptor in any of the three sets, plus 1. FD最高位再加一 */
	/* 若Timeout仍未有Fd，則回傳0 */
	select(inFileHandle + 1, NULL, &stWfd, NULL, &stTimeVal);
	if (FD_ISSET(inFileHandle, &stWfd))
	{

	} else
	{
		inUtility_StoreTraceLog_OneStep("inETHERNET_Send_Ready_By_Native Fail, errno:%d", errno);
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Send_Ready_TLS
Date&Time       :2017/8/2 下午 4:56
Describe        :表示現在TLS可以傳送資料
 */
int inETHERNET_Send_Ready_TLS_CTOS(unsigned int uiSSL_ID)
{
	int inRetVal = VS_ERROR;

	inRetVal = inTLS_TLS2_GetSession(uiSSL_ID);
	if (inRetVal != VS_SUCCESS)
	{
		inUtility_StoreTraceLog_OneStep("inETHERNET_Send_Ready_TLS error");
	}

	return (inRetVal);
}

/*
Function        :inETHERNET_Send_Ready_TLS_OPENSSL
Date&Time       :2024/8/19 下午 3:32
Describe        :表示現在TLS可以傳送資料
 */
int inETHERNET_Send_Ready_TLS_OPENSSL(SSL **ssl)
{
	int inRetVal = 0;

	inRetVal = SSL_is_init_finished(*ssl);
	if (inRetVal == 1)
	{
		return (VS_SUCCESS);
	} else
	{

		return (VS_ERROR);
	}
}

/*
Function        :inETHERNET_Send_Data_Flow
Date&Time       :2017/8/1 下午 2:19
Describe        :分流
 */
int inETHERNET_Send_Data_Flow(unsigned char* uszData, unsigned short usLen)
{
	int inRetVal = VS_ERROR;

	if (ginEthernetFlow == _ETHERNET_FLOW_IFES_)
	{
		if (_TLS_API_WAY_ == _TLS_API_WAY_OPENSSL_)
		{
			inRetVal = inETHERNET_Send_Data_TLS_OPENSSL(&gSsl, uszData, usLen);
		} else
		{
			inRetVal = inETHERNET_Send_Data_TLS_CTOS(guiSSL_ID, uszData, usLen);
		}
	}		/* 用native方式建Socket，之後要優化再考慮使用(優點自己決定Timeout等等細微設定) */
	else if (ginEthernetFlow == _ETHERNET_FLOW_NATIVE_)
	{
		inRetVal = inETHERNET_Send_Data_By_Native(ginTrans_ClientFd, uszData, usLen);
	} else
	{
		inRetVal = inETHERNET_Send_Data(uszData, usLen);
	}

	return (inRetVal);
}

/*
Function        :inETHERNET_Send_Data
Date&Time       :2017/7/18 下午 5:28
Describe        :傳送資料
 */
int inETHERNET_Send_Data(unsigned char* uszData, unsigned short usLen)
{
	char szDebugMsg[100 + 1];
	unsigned short usRetVal;

	usRetVal = CTOS_EthernetTxData(uszData, usLen);
	if (usRetVal == d_OK)
	{

	} else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "CTOS_EthernetTxData Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		inUtility_StoreTraceLog_OneStep("inETHERNET_Send_Data Fail, Err :0x%04X", usRetVal);

		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Send_Data_By_Native
Date&Time       :2017/8/1 下午 2:28
Describe        :傳送資料
 */
int inETHERNET_Send_Data_By_Native(int inFileHandle, unsigned char* uszData, unsigned short usLen)
{
	int inTempLen = 0;
	char szDebugMsg[100 + 1];

	/* Send message */
	inTempLen = send(inFileHandle, uszData, usLen, 0);

	if (inTempLen >= 0)
	{
		return (VS_SUCCESS);
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "send Errno: %d", errno);
			inLogPrintf(AT, szDebugMsg);
		}
		inUtility_StoreTraceLog_OneStep("inETHERNET_Send_Data_By_Native Fail, Errno: %d", errno);

		return (VS_ERROR);
	}

}

/*
Function        :inETHERNET_Send_Data_TLS
Date&Time       :2017/8/2 下午 5:03
Describe        :傳送資料
 */
int inETHERNET_Send_Data_TLS_CTOS(unsigned int uiSSL_ID, unsigned char* uszData, unsigned short usLen)
{
	int inRetVal = VS_ERROR;
	int inDataLen = 0;

	inDataLen = usLen;

	inRetVal = inTLS_Send_Data(uiSSL_ID, uszData, &inDataLen);

	usLen = (unsigned short) inDataLen;

	return (inRetVal);
}

/*
Function        :inETHERNET_Send_Data_OPENSSL_TLS
Date&Time       :2024/8/19 下午 3:20
Describe        :傳送資料
 */
int inETHERNET_Send_Data_TLS_OPENSSL(SSL **ssl, unsigned char *uszData, unsigned short usLen)
{
	int inRetVal = VS_ERROR;
	int inDataLen = 0;

	inDataLen = usLen;

	inRetVal = inTLS_OPENSSL_SSL_Send_Data(ssl, uszData, &inDataLen);

	usLen = (unsigned short) inDataLen;

	return (inRetVal);
}

/*
Function        :inETHERNET_Receive_Ready_Flow
Date&Time       :2017/8/1 下午 2:47
Describe        :目前只有虹堡API才有receive check的功能，用native沒辦法實做，所以決定不使用
 */
int inETHERNET_Receive_Ready_Flow(unsigned short *usLen)
{
	int inRetVal;
	unsigned short usBufferMaxLen = 0;

	if (ginEthernetFlow == _ETHERNET_FLOW_IFES_)
	{
		if (_TLS_API_WAY_ == _TLS_API_WAY_OPENSSL_)
		{
			inRetVal = inETHERNET_Receive_Ready_TLS_OPENSSL(&gSsl, usLen);
		} else
		{
			inRetVal = inETHERNET_Receive_Ready_TLS_CTOS(guiSSL_ID, usLen);
		}
	}		/* 用native方式建Socket，之後要優化再考慮使用(優點自己決定Timeout等等細微設定) */
	else if (ginEthernetFlow == _ETHERNET_FLOW_NATIVE_)
	{
		inRetVal = inETHERNET_Receive_Ready_By_Native(ginTrans_ClientFd, usLen);
	} else
	{
		usBufferMaxLen = *usLen;
		inRetVal = inETHERNET_Receive_Ready(usLen);
		if (*usLen > usBufferMaxLen)
		{
			*usLen = usBufferMaxLen;
		}
	}

	return (inRetVal);
}

/*
Function        :inETHERNET_Receive_Ready
Date&Time       :2017/7/18 下午 5:57
Describe        :表示現在Ethernet可以接收資料
 */
int inETHERNET_Receive_Ready(unsigned short *usLen)
{
	char szDebugMsg[100 + 1];
	unsigned short usRetVal;

	usRetVal = CTOS_EthernetRxReady(usLen);
	if (usRetVal == d_OK)
	{

	} else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inETHERNET_Receive_Ready Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Receive_Ready_By_Native
Date&Time       :2017/8/1 下午 3:10
Describe        :
 */
int inETHERNET_Receive_Ready_By_Native(int inFileHandle, unsigned short *usLen)
{
	fd_set stRfd; /* 用來判斷是否可以讀的Fd */
	struct timeval stTimeVal;

	stTimeVal.tv_sec = 0;
	stTimeVal.tv_usec = 0;

	/* 將 set 整個清為零。 */
	FD_ZERO(&stRfd);
	/* 將 fd 新增到 set。 */
	FD_SET(inFileHandle, &stRfd);

	/* 初始化 */
	*usLen = 0;
	/* nfds is the highest-numbered file descriptor in any of the three sets, plus 1. FD最高位再加一 */
	/* 若Timeout仍未有Fd，則回傳0 */
	select(inFileHandle + 1, &stRfd, NULL, NULL, &stTimeVal);
	if (FD_ISSET(inFileHandle, &stRfd))
	{
		/* Native沒有check功能，所以直接設大一點，等到read時再根據實際回傳讀取 */
		*usLen = _COMM_RECEIVE_MAX_LENGTH_;
	} else
	{
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Receive_Ready_TLS_CTOS
Date&Time       :2017/8/2 下午 4:56
Describe        :表示現在TLS可以接收資料
 */
int inETHERNET_Receive_Ready_TLS_CTOS(unsigned int uiSSL_ID, unsigned short *usLen)
{
	int inRetVal = VS_ERROR;

	inRetVal = inTLS_TLS2_GetSession(uiSSL_ID);
	if (inRetVal == VS_SUCCESS)
	{
		*usLen = _COMM_RECEIVE_MAX_LENGTH_;
	} else
	{
		*usLen = 0;
	}

	return (inRetVal);
}

/*
Function        :inETHERNET_Receive_Ready_TLS_OPENSSL
Date&Time       :2024/8/19 下午 3:39
Describe        :表示現在TLS可以接收資料
 *		經測試SSL_peek可用，SSL_pending不可用
 */
int inETHERNET_Receive_Ready_TLS_OPENSSL(SSL **ssl, unsigned short *usLen)
{
	int inRetVal = 0;
	char buffer[_COMM_RECEIVE_MAX_LENGTH_] = {0}; // 用來呼叫 SSL_peek，實際上我們不會使用這個緩衝區

	inRetVal = SSL_peek(*ssl, buffer, sizeof (buffer));
	if (inRetVal > 0)
	{
		*usLen = inRetVal;

		return (VS_SUCCESS);
	} else
	{
		*usLen = 0;

		return (VS_ERROR);
	}
}

/*
Function        :inETHERNET_Receive_Data_Flow
Date&Time       :2017/8/1 下午 3:43
Describe        :分流
 */
int inETHERNET_Receive_Data_Flow(unsigned char* uszData, unsigned short *usLen)
{
	int inRetVal = VS_ERROR;

	if (ginEthernetFlow == _ETHERNET_FLOW_IFES_)
	{
		if (_TLS_API_WAY_ == _TLS_API_WAY_OPENSSL_)
		{
			inRetVal = inETHERNET_Receive_Data_TLS_OPENSSL(&gSsl, uszData, usLen);
		} else
		{
			inRetVal = inETHERNET_Receive_Data_TLS_CTOS(guiSSL_ID, uszData, usLen);
		}
	}		/* 用native方式建Socket，之後要優化再考慮使用(優點自己決定Timeout等等細微設定) */
	else if (ginEthernetFlow == _ETHERNET_FLOW_NATIVE_)
	{
		inRetVal = inETHERNET_Receive_Data_By_Native(ginTrans_ClientFd, uszData, usLen);
	} else
	{
		inRetVal = inETHERNET_Receive_Data(uszData, usLen);
	}


	return (inRetVal);
}

/*
Function        :inETHERNET_Receive_Data
Date&Time       :2017/7/18 下午 6:03
Describe        :接收資料
 */
int inETHERNET_Receive_Data(unsigned char* uszData, unsigned short *usLen)
{
	char szDebugMsg[100 + 1];
	unsigned short usRetVal;

	usRetVal = CTOS_EthernetRxData(uszData, usLen);
	if (usRetVal == d_OK)
	{

	} else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "CTOS_EthernetRxData Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Receive_Data_By_Native
Date&Time       :2017/8/1 下午 3:45
Describe        :
 */
int inETHERNET_Receive_Data_By_Native(int inFileHandle, unsigned char* uszData, unsigned short *usLen)
{
	int inTempLen = 0;


	inTempLen = recv(inFileHandle, uszData, *usLen, 0);

	/* 如果回傳-1會變成65535，所以要做安全檢核 */
	if (inTempLen > 0)
	{
		*usLen = (unsigned short) inTempLen;
	} else
	{
		*usLen = 0;

		//		char	szDebugMsg[100 + 1];
		//		if (ginDebug == VS_TRUE)
		//		{
		//			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		//			sprintf(szDebugMsg, "recv Errno: %d", errno);
		//			inLogPrintf(AT, szDebugMsg);
		//		}

		return (VS_ERROR);
	}



	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Receive_Data_TLS_CTOS
Date&Time       :2017/8/2 下午 5:03
Describe        :接收資料
 */
int inETHERNET_Receive_Data_TLS_CTOS(unsigned int uiSSL_ID, unsigned char* uszData, unsigned short *usLen)
{
	int inRetVal = VS_ERROR;
	int inDataLen = 0;

	inDataLen = *usLen;
	inRetVal = inTLS_Read_Data(uiSSL_ID, uszData, &inDataLen);
	*usLen = inDataLen;

	return (inRetVal);
}

/*
Function        :inETHERNET_Receive_Data_TLS_OPENSSL
Date&Time       :2024/8/19 下午 3:46
Describe        :接收資料
 */
int inETHERNET_Receive_Data_TLS_OPENSSL(SSL **ssl, unsigned char* uszData, unsigned short *usLen)
{
	int inRetVal = VS_ERROR;
	int inDataLen = 0;

	inDataLen = *usLen;
	inRetVal = inTLS_OPENSSL_SSL_Read_Data(ssl, uszData, &inDataLen);
	*usLen = inDataLen;

	return (inRetVal);
}

/*
Function        :inETHERNET_Flush_Rx
Date&Time       :2017/7/19 上午 11:18
Describe        :清空Comport
 */
int inETHERNET_Flush_Rx()
{
	char szDebugMsg[100 + 1];
	unsigned short usRetVal;

	usRetVal = CTOS_EthernetFlushRxData();
	if (usRetVal == d_OK)
	{

	} else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "CTOS_EthernetFlushRxData Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Initial
Date&Time       :2017/7/18 下午 3:37
Describe        :
 */
int inETHERNET_Initial(void)
{
	int inRetVal = VS_ERROR;
	char szEDCIPAddress[16 + 1];
	char szSubMask[16 + 1];
	char szGatWay[16 + 1];
	char szConfig[16 + 1];
	char szDHCPMode[2 + 1];

	/* inETHERNET_Initial() START */
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inETHERNET_Initial() START！");
	}

	inDISP_ClearAll();
	inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SET_COMM_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 <通訊設定> */

	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

	/* 打開TCP IP */
	/* Portable 機型若沒接上底座再開Ethernet會失敗 */
	/* 這邊要分是否為有底座機型，有底座要先接回才能Initial */
	inRetVal = inETHERNET_Open();

	if (inRetVal == VS_SUCCESS)
	{

	} else
	{
		vdUtility_SYSFIN_LogMessage(AT, "inETHERNET_Open Failed");
		return (VS_ERROR);
	}

	memset(szDHCPMode, 0x00, sizeof (szDHCPMode));
	inGetDHCP_Mode(szDHCPMode);
	if (memcmp(szDHCPMode, "Y", strlen("Y")) == 0)
	{
		/* 設定HDCP */
		memset(szConfig, 0x00, sizeof(szConfig));
		szConfig[0] = 0x31;
		inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_DHCP, (unsigned char*)szConfig, 1);

		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT,"Log: Etherent DHCP Config is %s", szConfig);
		}
		/*
			d_ETHERNET_CONFIG_UPDATE_EXIT
			Save the settings to the non-volatile memory in Ethernet module.      
		 */
		/* 儲存設定 */
		memset(szConfig, 0x00, sizeof (szConfig));
		if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_UPDATE_EXIT, (unsigned char*) szConfig, 0) != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "Set d_ETHERNET_CONFIG_UPDATE_EXIT *Error*");/* [20260716 Add Log] */
			return (VS_ERROR);
		}
	} else
	{
		
		/* 設定為固定IP */
		memset(szConfig, 0x00, sizeof(szConfig));
		szConfig[0] = 0x30;
		inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_DHCP, (unsigned char*)szConfig, 1);
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT,"Log: Etherent FixIp DHCP Config is %s", szConfig);
		}
		
		/* 這邊先設定IP是因為要讓Idle下可以Ping得到 */
		memset(szEDCIPAddress, 0x00, sizeof (szEDCIPAddress));
		if (inGetTermIPAddress(szEDCIPAddress) == VS_ERROR)
		{
			vdUtility_SYSFIN_LogMessage(AT, "Get TermIPAddress *Error*");/* [20260716 Add Log] */
			return (VS_ERROR);
		}

		/* Set EDC IP Address */
		if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_IP, (unsigned char*) szEDCIPAddress, strlen(szEDCIPAddress)) != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "Set d_ETHERNET_CONFIG_IP *Error*");/* [20260716 Add Log] */
			return (VS_ERROR);
		}

		memset(szGatWay, 0x00, sizeof (szGatWay));
		if (inGetTermGetewayAddress(szGatWay) == VS_ERROR)
		{
			vdUtility_SYSFIN_LogMessage(AT, "Get TermGetewayAddress *Error*");/* [20260716 Add Log] */
			return (VS_ERROR);
		}

		/* Set Geteway IP */
		if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_GATEWAY, (unsigned char*) szGatWay, strlen(szGatWay)) != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "Set d_ETHERNET_CONFIG_GATEWAY *Error*");/* [20260716 Add Log] */
			return (VS_ERROR);
		}

		memset(szSubMask, 0x00, sizeof (szSubMask));
		if (inGetTermMASKAddress(szSubMask) == VS_ERROR)
		{
			vdUtility_SYSFIN_LogMessage(AT, "Get TermMASKAddress *Error*");/* [20260716 Add Log] */
			return (VS_ERROR);
		}

		/* Set EDC SUB Mask */
		if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_MASK, (unsigned char*) szSubMask, strlen(szSubMask)) != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "Set d_ETHERNET_CONFIG_MASK *Error*");/* [20260716 Add Log] */
			return (VS_ERROR);
		}

		/*
			d_ETHERNET_CONFIG_UPDATE_EXIT
			Save the settings to the non-volatile memory in Ethernet module.      
		 */
		/* 儲存設定 */
		memset(szConfig, 0x00, sizeof (szConfig));
		if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_UPDATE_EXIT, (unsigned char*) szConfig, 0) != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "Set d_ETHERNET_CONFIG_UPDATE_EXIT *Error*");/* [20260716 Add Log] */
			return (VS_ERROR);
		}

		/* 魔法，勿動 START 
		 * 這個Bug的發生時機在當機器直接斷電時，下次開機後，
		 * 連線狀態會一直Timeout。
		 * 正規解法是把設定IP、Submask、gateway 放到inETHERNET_SetConfig 再設定
		 */
		{
			memset(szGatWay, 0x00, sizeof (szGatWay));
			if (inGetTermGetewayAddress(szGatWay) == VS_ERROR)
			{
				vdUtility_SYSFIN_LogMessage(AT, "Get TermGetewayAddress *Error*");/* [20260716 Add Log] */
				return (VS_ERROR);
			}

			/* Set Geteway IP */
			if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_GATEWAY, (unsigned char*) szGatWay, strlen(szGatWay)) != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "Set d_ETHERNET_CONFIG_GATEWAY *Error*");/* [20260716 Add Log] */
				return (VS_ERROR);
			}

		}
		/* 魔法，勿動 End */
	}

	/* inETHERNET_Initial()_END */
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inETHERNET_Initial()_END");
	}

	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Begin
Date&Time       :2017/7/18 下午 4:16
Describe        :
 */
int inETHERNET_Begin(TRANSACTION_OBJECT *pobtran)
{
	char szCommIndex[2 + 1];
	char szDebugBuf[64 + 1];

	/* inETHERNET_Begin() START */
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inETHERNET_Begin() START！");
	}

	memset(szCommIndex, 0x00, sizeof (szCommIndex));
	/* 從HDT內get CPT的index */
	if (inGetCommunicationIndex(szCommIndex) == VS_ERROR)
	{
		vdUtility_SYSFIN_LogMessage(AT, "Get GetCommunicationIndex *Error*");/* [20260716 Add Log] */
		return (VS_ERROR);
	} else
	{
		/* CPT index從0開始，但是CPTRec index從1開始，所以要減一  */
		pobtran->srBRec.inCPTIndex = atoi(szCommIndex) - 1;
	}

	if (inLoadCPTRec(pobtran->srBRec.inCPTIndex) == VS_ERROR)
	{
		vdUtility_SYSFIN_LogMessage(AT,  "inLoadCPTRec(%d) *Error*", pobtran->srBRec.inCPTIndex);/* [20260716 Add Log] */
		return (VS_ERROR);
	}

	/* inETHERNET_Begin()_END */
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inETHERNET_Begin()_END");
	}

	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_SetConfig
Date&Time       :2017/7/18 下午 3:14
Describe        :如果IP衝突會Connect會TimeOut
 */
int inETHERNET_SetConfig(void)
{
	int inRetVal = 0;
	int inConnectMaxCnt = 1; /* 連線最大重試次數(含第一次) */
	int inConnectNowCnt = 0; /* 連線目前重試次數 */
	char szHostIPPrimary[16 + 1] = {0};
	char szHostIPSecond[16 + 1] = {0};
	char szHostPortNoPrimary[6 + 1] = {0};
	char szHostPortNoSecond[6 + 1] = {0};
	char szConfig[16 + 1] = {0};
	char szDebugMsg[100 + 1] = {0};
	char szDHCPMode[2 + 1] = {0};
	char szAutoConnect[10 + 1] = {0};
	char szTemplate[50 + 1] = {0};
	char szIFESMode[2 + 1] = {0};
	unsigned char uszLen = 0;

	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inETHERNET_SetConfig() START!");

	/* Set Host IP */
	memset(szHostIPPrimary, 0x00, sizeof (szHostIPPrimary));
	if (inGetHostIPPrimary(szHostIPPrimary) == VS_ERROR)
	{
		vdUtility_SYSFIN_LogMessage(AT, "Get inGetHostIPPrimary *Error*");/* [20260716 Add Log] */
		return (VS_ERROR);
	}

	inRetVal = inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_HOSTIP, (unsigned char*) szHostIPPrimary, strlen(szHostIPPrimary));
	if (inRetVal != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "Set d_ETHERNET_CONFIG_HOSTIP *Error*");/* [20260716 Add Log] */
		return (VS_ERROR);
	}

	/* Set Host Port */
	memset(szHostPortNoPrimary, 0x00, sizeof (szHostPortNoPrimary));
	if (inGetHostPortNoPrimary(szHostPortNoPrimary) == VS_ERROR)
	{
		vdUtility_SYSFIN_LogMessage(AT, "Get inGetHostPortNoPrimary *Error*");/* [20260716 Add Log] */
		return (VS_ERROR);
	}

	inRetVal = inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_HOSTPORT, (unsigned char*) szHostPortNoPrimary, strlen(szHostPortNoPrimary));
	if (inRetVal != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "Set d_ETHERNET_CONFIG_HOSTPORT *Error*");/* [20260716 Add Log] */
		return (VS_ERROR);
	}

	/*
			d_ETHERNET_CONFIG_AUTOCON
			Set Connection Mode.
			= 0 : Auto-connect. When Auto-connect is set, the Ethernet module will automatically try to connect to the host every 5 seconds.
			= 1 : Not support
			= 2 : Manual. The connection must be established manually by calling CTOS_EthernetConnectxxx() function.
	 */
	/* 預設成2 */
	memset(szConfig, 0x00, sizeof (szConfig));
	memset(szAutoConnect, 0x00, sizeof (szAutoConnect));
	inGetAutoConnect(szAutoConnect);
	if (memcmp(&szAutoConnect[0], "1", strlen("1")) == 0)
	{
		szConfig[0] = '0';
	} else
	{
		szConfig[0] = '2';
	}
	if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_AUTOCON, (unsigned char*) szConfig, 1) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "d_ETHERNET_CONFIG_AUTOCON Error");

		return (VS_ERROR);
	}

	/*
			d_ETHERNET_CONFIG_DHCP
			Set IP configuration.
			= 0 : Static. Use the static IP set in the Ethernet configuration.
			= 1 : DHCP. Retrieve the dynamic IP from the DHCP server.
	 */
	/* DHCP預設0 */
	memset(szConfig, 0x00, sizeof (szConfig));
	memset(szDHCPMode, 0x00, sizeof (szDHCPMode));
	inGetDHCP_Mode(szDHCPMode);
	if (memcmp(szDHCPMode, "Y", strlen("Y")) == 0)
	{
		szConfig[0] = 0x31;
	} else
	{
		szConfig[0] = 0x30;
	}

	if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_DHCP, (unsigned char*) szConfig, 1) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "d_ETHERNET_CONFIG_DHCP Error");

		return (VS_ERROR);
	}

	/*
			d_ETHERNET_CONFIG_UPDATE_EXIT
			Save the settings to the non-volatile memory in Ethernet module.      
	 */
	memset(szConfig, 0x00, sizeof (szConfig));
	if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_UPDATE_EXIT, (unsigned char*) szConfig, 0) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "d_ETHERNET_CONFIG_UPDATE_EXIT Error");

		return (VS_ERROR);
	}

	/* 重置目前重試次數 */
	inConnectNowCnt = 0;
	/* 若連線失敗看是否要重試 */
	do
	{
		inRetVal = inETHERNET_Connect_Flow(szHostIPPrimary, szHostPortNoPrimary);
		inConnectNowCnt++;

	} while (inRetVal != VS_SUCCESS && inConnectNowCnt < inConnectMaxCnt);


	/* 第一授權主機連線成功 */
	if (inRetVal == VS_SUCCESS)
	{
		/* 如果是DHCP，*/
		if (memcmp(szDHCPMode, "Y", strlen("Y")) == 0)
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			uszLen = sizeof (szTemplate);
			inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_IP, (unsigned char*) szTemplate, &uszLen);
			inSetTermIPAddress(szTemplate);

			memset(szTemplate, 0x00, sizeof (szTemplate));
			uszLen = sizeof (szTemplate);
			inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_MASK, (unsigned char*) szTemplate, &uszLen);
			inSetTermMASKAddress(szTemplate);

			memset(szTemplate, 0x00, sizeof (szTemplate));
			uszLen = sizeof (szTemplate);
			inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_GATEWAY, (unsigned char*) szTemplate, &uszLen);
			inSetTermGetewayAddress(szTemplate);

			inSaveEDCRec(0);
		}

		return (VS_SUCCESS);
	} else
	{
		/* 斷線(TLS若失敗沒斷線，會沒清除資源，所以調整成失敗一律先斷線再重連) */
		inETHERNET_END();

		/* 若第一授權主機連線不成功，改連第二授權主機 */
		/* IFES情況下才連第二組 */
		memset(szIFESMode, 0x00, sizeof (szIFESMode));
		inGetI_FES_Mode(szIFESMode);
		if (memcmp(szIFESMode, "Y", strlen("Y")) != 0)
		{
			return (VS_ERROR);
		}
	}

	/* 看狀態 */
	inETHERNET_Watch_Status();

	/* 第一授權IP失敗 嘗試第二IP */
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "First IP Conenct Fail");
		inLogPrintf(AT, szDebugMsg);
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Try Second IP...");
		inLogPrintf(AT, szDebugMsg);
	}

	inDISP_Clear_Line(_LINE_8_6_, _LINE_8_6_);
	inDISP_ChineseFont("第二組IP", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
	/* 嘗試連線第二授權主機 */
	/* 更改HostIP */
	/* Set Host IP */
	memset(szHostIPSecond, 0x00, sizeof (szHostIPSecond));
	if (inGetHostIPSecond(szHostIPSecond) == VS_ERROR)
	{
		/* inGetHostIPSecond ERROR */
		/* debug */
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inGetHostIPSecond() ERROR!!");

		return (VS_ERROR);
	}

	inRetVal = inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_HOSTIP, (unsigned char*) szHostIPSecond, strlen(szHostIPSecond));
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "d_ETHERNET_CONFIG_HOSTIP Error : 0x%04x", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}

		return (VS_ERROR);
	}

	/* Set Host Port */
	memset(szHostPortNoSecond, 0x00, sizeof (szHostPortNoSecond));
	if (inGetHostPortNoSecond(szHostPortNoSecond) == VS_ERROR)
	{
		/* Get HostPortNumber Primary ERROR */
		/* debug */
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inGetHostPortNoSecond() ERROR!!");

		return (VS_ERROR);
	}

	if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_HOSTPORT, (unsigned char*) szHostPortNoSecond, strlen(szHostPortNoSecond)) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "d_ETHERNET_CONFIG_HOSTPORT Error");

		return (VS_ERROR);
	}

	/* 紀錄在記憶體內 */
	memset(szConfig, 0x00, sizeof (szConfig));
	if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_UPDATE_EXIT, (unsigned char*) szConfig, 0) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "d_ETHERNET_CONFIG_UPDATE_EXIT Error");

		return (VS_ERROR);
	}

	/* 重置目前重試次數 */
	inConnectNowCnt = 0;
	/* 若連線失敗看是否要重試 */
	do
	{
		inRetVal = inETHERNET_Connect_Flow(szHostIPSecond, szHostPortNoSecond);
		inConnectNowCnt++;
	} while (inRetVal != VS_SUCCESS && inConnectNowCnt < inConnectMaxCnt);

	/* 第二授權主機連線成功 */
	if (inRetVal == VS_SUCCESS)
	{
		return (VS_SUCCESS);
	} else
	{
		/* 若第二授權主機連線不成功，直接斷線 */
	}

	/* 第二授權IP失敗 */
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Second IP Conenct Fail");
		inLogPrintf(AT, szDebugMsg);
	}

	/* 到最後還失敗，斷線 */
	inETHERNET_END();

	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inETHERNET_SetConfig() END!");

	/* 若連線成功，中間就會Return出去，若一直失敗，則最後一定回傳失敗 */
	return (VS_ERROR);
}

/*
Function        :inETHERNET_Send
Date&Time       :2017/7/18 下午 5:25
Describe        :送電文，並根據電文格式前面加長度
 */
int inETHERNET_Send(unsigned char *uszSendBuff, int inSendSize, int inSendTimeout)
{
	char szDataHead[1 + 1];
	char szDebugMsg[100 + 1];
	unsigned char szSendData[inSendSize + 32];

	memset(szSendData, 0x00, sizeof (szSendData));
	memset(szDataHead, 0x00, sizeof (szDataHead));

	/* inETHERNET_Send() START! */
	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inETHERNET_Send() START!");

	/* Get HeadFormat */
	if (inGetTCPHeadFormat(szDataHead) == VS_ERROR)
	{
		/* inGetTCPHeadFormat ERROR */
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inGetTCPHeadFormat Error!");

		return (VS_ERROR);
	}

	/*	範例:Size = 1024
	 * 	H:0x04 0x00
	 *	B:0x10 0x24
	 */
	if (szDataHead[0] == 'H')
	{
		szSendData[0] = ((inSendSize & 0x0000FF00) >> 8);
		szSendData[1] = (inSendSize & 0x000000FF);
	} else
	{
		szSendData[0] = (inSendSize / 100) / 10 * 16 + (inSendSize / 100) % 10;
		szSendData[1] = (inSendSize % 100) / 10 * 16 + (inSendSize % 100) % 10;
	}

	if (ginISODebug == VS_TRUE)
	{
		inPRINT_ChineseFont("----------------------------------------", _PRT_ISO_);
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "%02X %02X :length = %d", szSendData[0], szSendData[1], inSendSize);
		inPRINT_ChineseFont_Format(szDebugMsg, "  ", 34, _PRT_ISO_);
		inPRINT_ChineseFont("----------------------------------------", _PRT_ISO_);
	}
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "%02X %02X :length = %d", szSendData[0], szSendData[1], inSendSize);
		inLogPrintf(AT, szDebugMsg);
		inLogPrintf(AT, "----------------------------------------");
	}

	memcpy(&szSendData[2], uszSendBuff, inSendSize);

	/* 因為多放長度所以要加2Byte */
	inSendSize += 2;

	/* 有Timeout，要計時 */
	if (inSendTimeout != 0)
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, inSendTimeout);

	while (1)
	{
		if (inSendTimeout != 0)
		{
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "inETHERNET_Send() TIMEOUT");
				}

				return (VS_ERROR);
			}
		}

		/* 如果可以Send就Send */
		if (inETHERNET_Send_Ready_Flow() == VS_SUCCESS)
		{
			if (inETHERNET_Send_Data_Flow(szSendData, inSendSize) != VS_SUCCESS)
			{
				continue;
			} else
				break;
		}
	}

	/* inETHERNET_Send() END! */
	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inETHERNET_Send() END!");

	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Receive
Date&Time       :2017/7/18 下午 5:55
Describe        :
 */
int inETHERNET_Receive(unsigned char *uszReceiveBuff, int inReceiveSize, int inReceiveTimeout)
{
	int inRetVal = 0;
	int inReceivelen = 0; /* Comport當前收到的長度 */
	int inDataLength = 0; /* 收到的資料長度(不含Head) */
	int inExpectLength = 0;
	char szDataHead[1 + 1];
	char szDebugMsg[_DEBUG_MESSAGE_SIZE_ + 1]; /* debug message */
	unsigned char uszRawBuffer[inReceiveSize + 1];
	unsigned short usOutputLen = 0; /* 目前comport可以讀取的資料長度 */

	/* inETHERNET_Receive() START! */
	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inETHERNET_Receive() START!");

	memset(uszRawBuffer, 0x00, sizeof (uszRawBuffer));

	/* 設定接收Timeout時間 */
	if (inReceiveTimeout != 0)
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, inReceiveTimeout);

	/* 第一段先收到comport沒資料為止，或Timeout */
	while (1)
	{
		if (inReceiveTimeout != 0)
		{
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
				{
					if (inReceiveTimeout != 0)
					{
						inLogPrintf(AT, "inETHERNET_Receive TimeOut 1");
					}

				}
				inUtility_StoreTraceLog_OneStep("inETHERNET_Receive TimeOut 1");

				return (VS_TIMEOUT);
			}
		}

		/* 設定為剩餘的Buffer空間大小，才符合API的用法 */
		usOutputLen = inReceiveSize - inReceivelen;

		if (inETHERNET_Receive_Ready_Flow(&usOutputLen) == VS_SUCCESS)
		{
			inRetVal = inETHERNET_Receive_Data_Flow(&uszRawBuffer[inReceivelen], &usOutputLen);
			if (inRetVal == VS_SUCCESS)
			{
				inReceivelen = inReceivelen + (int) usOutputLen;
			}
		}

		/* 沒有可以收的就跳出去(usOutputLen == 0代表沒有可已從buffer中讀取的資料) */
		if (inReceivelen > 0 && usOutputLen == 0)
		{
			break;
		}
	}

	/* Get HeadFormat */
	/*	範例:Size = 1024
	 * 	H:0x04 0x00
	 *	B:0x10 0x24
	 */
	memset(szDataHead, 0x00, sizeof (szDataHead));
	if (inGetTCPHeadFormat(szDataHead) == VS_ERROR)
	{
		/* inGetTCPHeadFormat ERROR */
		/* debug */
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inGetTCPHeadFormat() ERROR!!");
		}
		inUtility_StoreTraceLog_OneStep("inGetTCPHeadFormat() ERROR!!");

		return (VS_ERROR);
	}

	/* 如果有收到，就解析Head的封包長度並確認是否全收到 */
	/* 因為封包欄位佔2bytes，所以至少應該收到2byte */
	if (inReceivelen >= 2)
	{
		/* 根據Head算出預期長度 */
		if (szDataHead[0] == 'H')
		{
			inExpectLength = uszRawBuffer[0] * 256 + uszRawBuffer[1];
		} else
		{
			inExpectLength = ((uszRawBuffer[0] / 16 * 10 + uszRawBuffer[0] % 16) * 100) + (uszRawBuffer[1] / 16 * 10 + uszRawBuffer[1] % 16);
		}

		if (ginISODebug == VS_TRUE)
		{
			inPRINT_ChineseFont("----------------------------------------", _PRT_ISO_);
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "%02X %02X :length = %d", uszRawBuffer[0], uszRawBuffer[1], inExpectLength);
			inPRINT_ChineseFont_Format(szDebugMsg, "  ", 34, _PRT_ISO_);
			inPRINT_ChineseFont("----------------------------------------", _PRT_ISO_);
		}
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "----------------------------------------");
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "%02X %02X :length = %d", uszRawBuffer[0], uszRawBuffer[1], inExpectLength);
			inLogPrintf(AT, szDebugMsg);
			inLogPrintf(AT, "----------------------------------------");
		}


		memcpy(&uszReceiveBuff[inDataLength], &uszRawBuffer[2], inReceivelen - 2);
		/* inDataLength為目前收到的封包數 */
		inDataLength += inReceivelen - 2;

		/* 比對目前接收的長度(含Length)是否與主機下的長度一致(inDataLength = inExpectLength)，若小於應接收長度則繼續接收剩下的封包 */
		while (inDataLength < inExpectLength)
		{
			/* 準備收下一次，清空暫存Buffer */
			inReceivelen = 0;
			memset(uszRawBuffer, 0x00, sizeof (uszRawBuffer));

			while (1)
			{
				/* Timeout的話 */
				if (inReceiveTimeout != 0)
				{
					if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
					{
						if (ginDebug == VS_TRUE)
						{
							if (inReceiveTimeout != 0)
							{
								inLogPrintf(AT, "inETHERNET_Receive TimeOut 2");
								memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
								sprintf(szDebugMsg, "Receve Len: %d", inDataLength);
								inLogPrintf(AT, szDebugMsg);
							}

							inUtility_StoreTraceLog_OneStep("inETHERNET_Receive TimeOut 2");
						}
						return (VS_TIMEOUT);
					}
				}

				/* 設定為剩餘的Buffer空間大小，才符合API的用法 */
				usOutputLen = inReceiveSize - inReceivelen;

				if (inETHERNET_Receive_Ready_Flow(&usOutputLen) == VS_SUCCESS)
				{
					inRetVal = inETHERNET_Receive_Data_Flow(&uszRawBuffer[inReceivelen], &usOutputLen);
					if (inRetVal == VS_SUCCESS)
					{
						inReceivelen = inReceivelen + (int) usOutputLen;
					}
				}

				/* 沒有可以收的就跳出去(usOutputLen == 0代表沒有可已從buffer中讀取的資料) */
				if (inReceivelen > 0 && usOutputLen == 0)
				{
					break;
				}
			}

			memcpy(&uszReceiveBuff[inDataLength], &uszRawBuffer[0], inReceivelen);
			inDataLength += inReceivelen;
		}

	}		/* 連Head都沒收到 */
	else
	{
		/* debug */
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inReceivelen ERROR!! :(inReceivelen = %d)", inReceivelen);
			inLogPrintf(AT, szDebugMsg);
		}

		return (VS_ERROR);
	}

	/* inETHERNET_Receive() END! */
	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inETHERNET_Receive() END!");

	return (inDataLength);
}

/*
Function        :inETHERNET_END
Date&Time       :2017/7/19 上午 11:09
Describe        :斷線
 */
int inETHERNET_END(void)
{
	/* inETHERNET_END() START! */
	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inETHERNET_END() START!");

	if (inETHERNET_DisConnect_Flow() != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inETHERNET_Disconnect_Flow() == VS_ERROR");
		return (VS_ERROR);
	}

	/* inETHERNET_END() END! */
	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inETHERNET_END() END!");

	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Flush
Date&Time       :2017/7/19 上午 11:14
Describe        :清空Comport的資料
 */
int inETHERNET_Flush(void)
{
	int inRetVal = 0;
	char szDebugMsg[100 + 1]; /* debug message */

	/* inETHERNET_Flush() START! */
	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inETHERNET_Flush() START!");

	inRetVal = inETHERNET_Flush_Rx();
	if (inRetVal != VS_SUCCESS)
	{
		/* debug */
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inETHERNET_Flush Error");
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}

	/* inETHERNET_Flush() END! */
	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inETHERNET_Flush() END!");

	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_DeInitial
Date&Time       :2017/7/19 上午 11:20
Describe        :關閉Comport
 */
int inETHERNET_DeInitial(void)
{
	int inRetVal = 0;
	char szDebugMsg[100 + 1]; /* debug message */

	/* inETHERNET_DeInitial() START! */
	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inETHERNET_DeInitial() START!");

	inRetVal = inETHERNET_Close();
	if (inRetVal != VS_SUCCESS)
	{
		/* debug */
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inETHERNET_Close ERR");
			inLogPrintf(AT, szDebugMsg);
		}
	}

	/* inETHERNET_DeInitial() END! */
	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inETHERNET_DeInitial() END!");

	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Get_Status
Date&Time       :2018/3/9 下午 1:23
Describe        :
 */
int inETHERNET_Get_Status(unsigned int *uiStatus)
{
	char szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1]; /* debug message */
	unsigned short usRetval;

	if (uiStatus == NULL)
		return (VS_ERROR);

	usRetval = CTOS_EthernetStatus(uiStatus);

	if (usRetval != d_OK)
	{
		/* debug */
		if (ginDebug == VS_TRUE)
		{
			memset(szErrorMsg, 0x00, sizeof (szErrorMsg));
			sprintf(szErrorMsg, "CTOS_EthernetStatus Error : 0x%04x", usRetval);
			inLogPrintf(AT, szErrorMsg);
		}
	}

	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_NCCCTMS_Check
Date&Time       :2017/7/19 上午 11:33
Describe        :
 */
int inETHERNET_NCCCTMS_Check(void)
{
	int inRetVal = 0;
	int inConnectMaxCnt = 1; /* 連線最大重試次數(含第一次) */
	int inConnectNowCnt = 0; /* 連線目前重試次數 */
	char szTMSIPPrimary[16 + 1] = {0};
	char szTMSIPSecond[16 + 1] = {0};
	char szTMSPortNoPrimary[6 + 1] = {0};
	char szTMSPortNoSecond[6 + 1] = {0};
	char szConfig[16 + 1];
	char szDebugMsg[100 + 1] = {0};
	char szDHCPMode[2 + 1];
	char szTemplate[50 + 1] = {0};
	char szIFESMode[2 + 1] = {0};
	unsigned char uszLen = 0;

	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inETHERNET_NCCCTMS_Check() START!");

	inLoadTMSIPDTRec(0);

	/* Set TMS IP */
	memset(szTMSIPPrimary, 0x00, sizeof (szTMSIPPrimary));
	if (inGetTMS_IP_Primary(szTMSIPPrimary) == VS_ERROR)
	{
		/* inGetHostIPPrimary ERROR */
		/* debug */
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inGetTMSIPAddress() ERROR!!");

		return (VS_ERROR);
	}

	if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_HOSTIP, (unsigned char*) szTMSIPPrimary, strlen(szTMSIPPrimary)) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "d_ETHERNET_CONFIG_HOSTIP Error");

		return (VS_ERROR);
	}

	/* Set TMS Port */
	memset(szTMSPortNoPrimary, 0x00, sizeof (szTMSPortNoPrimary));

	if (inGetTMS_PortNo_Primary(szTMSPortNoPrimary) == VS_ERROR)
	{
		/* Get HostPortNumber Primary ERROR */
		/* debug */
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inGetTMSPortNum() ERROR!!");

		return (VS_ERROR);
	}

	if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_HOSTPORT, (unsigned char*) szTMSPortNoPrimary, strlen(szTMSPortNoPrimary)) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "d_ETHERNET_CONFIG_HOSTPORT Error");

		return (VS_ERROR);
	}

	/*
			d_ETHERNET_CONFIG_AUTOCON
			Set Connection Mode.
			= 0 : Auto-connect. When Auto-connect is set, the Ethernet module will automatically try to connect to the host every 5 seconds.
			= 1 : Not support
			= 2 : Manual. The connection must be established manually by calling CTOS_EthernetConnectxxx() function.
	 */
	/* 預設成2 */
	memset(szConfig, 0x00, sizeof (szConfig));
	szConfig[0] = 0x32;

	if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_AUTOCON, (unsigned char*) szConfig, 1) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "d_ETHERNET_CONFIG_AUTOCON Error");

		return (VS_ERROR);
	}

	/*
			d_ETHERNET_CONFIG_DHCP
			Set IP configuration.
			= 0 : Static. Use the static IP set in the Ethernet configuration.
			= 1 : DHCP. Retrieve the dynamic IP from the DHCP server.
	 */
	/* DHCP預設0 */
	memset(szConfig, 0x00, sizeof (szConfig));
	memset(szDHCPMode, 0x00, sizeof (szDHCPMode));
	inGetDHCP_Mode(szDHCPMode);
	if (memcmp(szDHCPMode, "Y", strlen("Y")) == 0)
	{
		szConfig[0] = 0x31;
	} else
	{
		szConfig[0] = 0x30;
	}

	if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_DHCP, (unsigned char*) szConfig, 1) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "d_ETHERNET_CONFIG_DHCP Error");

		return (VS_ERROR);
	}

	/*
			d_ETHERNET_CONFIG_UPDATE_EXIT
			Save the settings to the non-volatile memory in Ethernet module.      
	 */
	memset(szConfig, 0x00, sizeof (szConfig));

	if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_UPDATE_EXIT, (unsigned char*) szConfig, 0) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "d_ETHERNET_CONFIG_UPDATE_EXIT Error");

		return (VS_ERROR);
	}

	/* 重置目前重試次數 */
	inConnectNowCnt = 0;
	/* 若連線失敗看是否要重試 */
	do
	{
		inRetVal = inETHERNET_Connect_Flow(szTMSIPPrimary, szTMSPortNoPrimary);
		inConnectNowCnt++;

	} while (inRetVal != VS_SUCCESS && inConnectNowCnt < inConnectMaxCnt);

	if (inRetVal == VS_SUCCESS)
	{
		/* 如果是DHCP，*/
		if (memcmp(szDHCPMode, "Y", strlen("Y")) == 0)
		{
			memset(szTemplate, 0x00, sizeof (szTemplate));
			uszLen = sizeof (szTemplate);
			inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_IP, (unsigned char*) szTemplate, &uszLen);
			inSetTermIPAddress(szTemplate);

			memset(szTemplate, 0x00, sizeof (szTemplate));
			uszLen = sizeof (szTemplate);
			inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_MASK, (unsigned char*) szTemplate, &uszLen);
			inSetTermMASKAddress(szTemplate);

			memset(szTemplate, 0x00, sizeof (szTemplate));
			uszLen = sizeof (szTemplate);
			inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_GATEWAY, (unsigned char*) szTemplate, &uszLen);
			inSetTermGetewayAddress(szTemplate);

			inSaveEDCRec(0);
		}

		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inETHERNET_NCCCTMS_Check() END!");

		return (VS_SUCCESS);
	} else
	{
		/* 斷線(TLS若失敗沒斷線，會沒清除資源，所以調整成失敗一律先斷線再重連) */
		inETHERNET_END();

		/* 若第一授權主機連線不成功，改連第二授權主機 */
		/* IFES情況下才連第二組 */
		memset(szIFESMode, 0x00, sizeof (szIFESMode));
		inGetI_FES_Mode(szIFESMode);
		if (memcmp(szIFESMode, "Y", strlen("Y")) != 0)
		{
			return (VS_ERROR);
		}
	}

	/* 看狀態 */
	inETHERNET_Watch_Status();

	/* 第一授權IP失敗 嘗試第二IP */
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "First IP Conenct Fail");
		inLogPrintf(AT, szDebugMsg);
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Try Second IP...");
		inLogPrintf(AT, szDebugMsg);
	}

	inDISP_Clear_Line(_LINE_8_6_, _LINE_8_6_);
	inDISP_ChineseFont("第二組IP", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
	/* 嘗試連線第二授權主機 */
	/* 更改TMSIP */
	/* Set TMS IP */
	memset(szTMSIPSecond, 0x00, sizeof (szTMSIPSecond));
	if (inGetTMS_IP_Second(szTMSIPSecond) == VS_ERROR)
	{
		/* inGetTMSIPSecond ERROR */
		/* debug */
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inGetTMSIPSecond() ERROR!!");

		return (VS_ERROR);
	}

	inRetVal = inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_HOSTIP, (unsigned char*) szTMSIPSecond, strlen(szTMSIPSecond));
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "d_ETHERNET_CONFIG_HOSTIP Error : 0x%04x", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}

		return (VS_ERROR);
	}

	/* Set TMS Port */
	memset(szTMSPortNoSecond, 0x00, sizeof (szTMSPortNoSecond));
	if (inGetTMS_PortNo_Second(szTMSPortNoSecond) == VS_ERROR)
	{
		/* Get TMSPortNumber Primary ERROR */
		/* debug */
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inGetTMSPortNoSecond() ERROR!!");

		return (VS_ERROR);
	}

	if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_HOSTPORT, (unsigned char*) szTMSPortNoSecond, strlen(szTMSPortNoSecond)) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "d_ETHERNET_CONFIG_HOSTPORT Error");

		return (VS_ERROR);
	}

	/* 紀錄在記憶體內 */
	memset(szConfig, 0x00, sizeof (szConfig));
	if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_UPDATE_EXIT, (unsigned char*) szConfig, 0) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "d_ETHERNET_CONFIG_UPDATE_EXIT Error");

		return (VS_ERROR);
	}

	/* 重置目前重試次數 */
	inConnectNowCnt = 0;
	/* 若連線失敗看是否要重試 */
	do
	{
		inRetVal = inETHERNET_Connect_Flow(szTMSIPSecond, szTMSPortNoSecond);
		inConnectNowCnt++;
	} while (inRetVal != VS_SUCCESS && inConnectNowCnt < inConnectMaxCnt);

	/* 第二授權主機連線成功 */
	if (inRetVal == VS_SUCCESS)
	{
		return (VS_SUCCESS);
	} else
	{
		/* 若第二授權主機連線不成功，直接斷線 */
	}

	/* 第二授權IP失敗 */
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Second IP Conenct Fail");
		inLogPrintf(AT, szDebugMsg);
	}

	/* 到最後還失敗，斷線 */
	inETHERNET_END();

	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inETHERNET_SetConfig() END!");

	/* 若連線成功，中間就會Return出去，若一直失敗，則最後一定回傳失敗 */
	return (VS_ERROR);
}

/*
Function        :inETHERNET_Watch_Status
Date&Time       :2016/10/11 下午 3:50
Describe        :
 */
int inETHERNET_Watch_Status(void)
{
	unsigned char uszTemplate[40 + 1];
	unsigned char uszLen = 0;
	unsigned int uiStatus = 0;

	/* Get the status of the Ethernet */
	inETHERNET_Get_Status(&uiStatus);

	/* if Ethernet is connected */
	if (uiStatus & d_STATUS_ETHERNET_CONNECTED)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "EthernetStatus : Connected");
		}
	}
	/* if Ethernet is command mode */
	if (uiStatus & d_STATUS_ETHERNET_COMMAND_MODE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "EthernetStatus : Command Mode");
		}
	}

	/* if Ethernet is phyical online */
	if (uiStatus & d_STATUS_ETHERNET_PHYICAL_ONLINE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "EthernetStatus : Phyical Online");
		}
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "網路線沒插好！！");
		}
	}

	/* if Ethernet is Rx readey */
	if (uiStatus & d_STATUS_ETHERNET_RX_READY)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "EthernetStatus : Rx Ready");
		}
	}

	/* if Ethernet is Tx busy */
	if (uiStatus & d_STATUS_ETHERNET_TX_BUSY)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "EthernetStatus : Tx Busy");
		}
	}

	/* IP */
	memset(uszTemplate, 0x00, sizeof (uszTemplate));
	uszLen = sizeof (uszTemplate);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_IP, uszTemplate, &uszLen);

	/* MASK */
	memset(uszTemplate, 0x00, sizeof (uszTemplate));
	uszLen = sizeof (uszTemplate);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_MASK, uszTemplate, &uszLen);

	/* GATEWAY */
	memset(uszTemplate, 0x00, sizeof (uszTemplate));
	uszLen = sizeof (uszTemplate);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_GATEWAY, uszTemplate, &uszLen);

	/* HOSTIP */
	memset(uszTemplate, 0x00, sizeof (uszTemplate));
	uszLen = sizeof (uszTemplate);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_HOSTIP, uszTemplate, &uszLen);

	/* HOST PORT */
	memset(uszTemplate, 0x00, sizeof (uszTemplate));
	uszLen = sizeof (uszTemplate);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_HOSTPORT, uszTemplate, &uszLen);

	/* CONFIG VERSION */
	memset(uszTemplate, 0x00, sizeof (uszTemplate));
	uszLen = sizeof (uszTemplate);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_VERSION, uszTemplate, &uszLen);

	/* CONFIG MAC */
	memset(uszTemplate, 0x00, sizeof (uszTemplate));
	uszLen = sizeof (uszTemplate);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_MAC, uszTemplate, &uszLen);

	/* CONFIG DHCP */
	memset(uszTemplate, 0x00, sizeof (uszTemplate));
	uszLen = sizeof (uszTemplate);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_DHCP, uszTemplate, &uszLen);

	/* CONFIG DNSIP */
	memset(uszTemplate, 0x00, sizeof (uszTemplate));
	uszLen = sizeof (uszTemplate);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_DNSIP, uszTemplate, &uszLen);

	/* CONFIG URL*/
	memset(uszTemplate, 0x00, sizeof (uszTemplate));
	uszLen = sizeof (uszTemplate);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_HOSTURL, uszTemplate, &uszLen);

	/* CONFIG AUTOCON */
	memset(uszTemplate, 0x00, sizeof (uszTemplate));
	uszLen = sizeof (uszTemplate);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_AUTOCON, uszTemplate, &uszLen);

	return (VS_SUCCESS);
}

/*
Function        inETHERNET_IsPhysicalOnine
Date&Time       :2018/3/9 下午 1:27
Describe        :回傳成功代表有插實體網路線
 */
int inETHERNET_IsPhysicalOnine()
{
	unsigned int uiStatus = 0;

	/* Get the status of the Ethernet */
	inETHERNET_Get_Status(&uiStatus);

	/* if Ethernet is phyical online */
	if (uiStatus & d_STATUS_ETHERNET_PHYICAL_ONLINE)
	{
		return (VS_SUCCESS);
	} else
	{
		return (VS_ERROR);
	}


}

/*
Function        :inETHERNET_Ping_IP
Date&Time       :2018/11/2 下午 1:23
Describe        :
 */
int inETHERNET_Ping_IP(char *szIP)
{
	int inLen = 0;
	char szDebugMsg[100 + 1] = {0};
	unsigned short usRetVal = VS_ERROR;

	inLen = strlen(szIP);

	usRetVal = CTOS_EthernetPing((unsigned char*) szIP, inLen);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "CTOS_EthernetPing Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	} else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "CTOS_EthernetPing() OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}

	return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Check_Ethernet_Config_Correct
Date&Time       :2019/10/18 上午 11:55
Describe        :ping Gateway如果成功，至少表示區域網路設置是對的，到gateway之前是通的
 *		不再使用，線上許多網路環境不接受ping gateway
 */
int inETHERNET_Check_Ethernet_Config_Correct(void)
{
	int inRetVal = VS_ERROR;
	char szGatewayIP[16 + 1] = {0};
	unsigned char uszLen = 0;

	memset(szGatewayIP, 0x00, sizeof (szGatewayIP));
	uszLen = sizeof (szGatewayIP);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_GATEWAY, (unsigned char*) szGatewayIP, &uszLen);

	inRetVal = inETHERNET_Ping_IP(szGatewayIP);
	if (inRetVal == VS_SUCCESS)
	{
		return (VS_SUCCESS);
	} else
	{
		return (VS_ERROR);
	}
}

/*
Function        :inETHERNET_DHCP_Flow
Date&Time       :2021/6/9 上午 10:48
Describe        :
 */
int inETHERNET_DHCP_Flow(void)
{
	inETHERNET_Close();
	inETHERNET_Open();

	return (VS_SUCCESS);
}
