#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
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
#include "../DISPLAY/DispMsg.h"
#include "../PRINT/Print.h"
#include "../FUNCTION/Sqlite.h"
#include "../FUNCTION/Function.h"
#include "../EVENT/MenuMsg.h"
#include "../FUNCTION/CFGT.h"
#include "../FUNCTION/HDT.h"
#include "../FUNCTION/CPT.h"
#include "../FUNCTION/EDC.h"
#include "../FUNCTION/HDPT.h"
#include "../FUNCTION/Utility.h"
#include "../FUNCTION/TMSIPDT.h"
#include "../FUNCTION/File.h"
#include "../COMM/TLS.h"
#include "../../NCCC/NCCCtmsCPT.h"
#include "../../NCCC/NCCCtmsFTP.h"
#include "Comm.h"
#include "GPRS.h"

extern	int		ginTrans_ClientFd;
extern	unsigned int	guiCTX_ID;		/* TLS Context ID */
extern	unsigned int	guiSSL_ID;		/* TLS Session ID */
extern	char		gszCACertPath[100 + 1];
extern  SSL_CTX*        gCtx;
extern  SSL*		gSsl;

extern  int     ginDebug;  /* Debug使用 extern */
extern  int     ginISODebug;
extern	int	ginDisplayDebug;
int	ginGPRSFlow = -1;			/* (-1:NORMAL 1:IFES)紀錄GPRS走哪個流程，用這個紀錄是因為下TMS有可能修改到CFGT，所以IFES Mode可能會被改到，導致無法斷線 */

/*
Function        :inGPRS_Open
Date&Time       :2018/3/1 上午 11:58
Describe        :When baIP is "\x00\x00\x00\x00", the local IP will be assigned by GPRS Server.
*/
int inGPRS_Open(unsigned char *uszIP, unsigned char *uszAPNName, unsigned char *uszID, unsigned char *uszPW)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inGPRS_Open() START !");
	}
	
	CTOS_PPP_SetTO(2000);
	
	usRetVal = CTOS_TCP_GPRSOpen(uszIP, uszAPNName, uszID, uszPW);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "GPRS_OPEN OK!");
		}
	}
	else if (usRetVal == d_TCP_IO_PROCESSING)
	{
//		if (ginDebug == VS_TRUE)
//		{
//			inLogPrintf(AT, "GPRS_OPEN IO PROCESSING!");
//		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_TCP_GPRSOpen Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		vdUtility_SYSFIN_LogMessage(AT, "CTOS_TCP_GPRSOpen Err :0x%04X", usRetVal);
		return (VS_ERROR);
	}
	
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inGPRS_Open() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_Close
Date&Time       :2018/3/1 下午 1:41
Describe        :
*/
int inGPRS_Close(void)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = 0;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inGPRS_Close() START !");
	}
	
	usRetVal = CTOS_TCP_GPRSClose();
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CTOS_TCP_GPRSClose OK!");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_TCP_GPRSClose Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inGPRS_Close() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_Close_Asynchronous
Date&Time       :2018/3/1 下午 1:46
Describe        :Please call CTOS_TCP_GPRSStatus() to check whether this action is finished.(非同步，所以要用CTOS_TCP_GPRSStatus來確認狀態是否完成)
*/
int inGPRS_Close_Asynchronous(void)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inGPRS_Close_Asynchronous() START !");
	}
	
	usRetVal = CTOS_TCP_GPRSClose_A();
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CTOS_TCP_GPRSClose_A OK!");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_TCP_GPRSClose_A Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inGPRS_Close_Asynchronous() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_Connect_Flow
Date&Time       :2018/3/1 下午 3:22
Describe        :分流使用CTOS還是使用原生C的Connect
*/
int inGPRS_Connect_Flow(char *szHostIP, char *szPort)
{
	int		inRetVal = VS_ERROR;
	char		szI_FES_Mode[2 + 1] = {0};
	char		szTRTFileName[12 + 1] = {0};
	char		szDebugMsg[500 + 1] = {0};
	char		szCFESMode[2 + 1] = {0};
	unsigned short	usPort = 0;
	unsigned char	uszIP[4 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inGPRS_Connect_Flow() START !");

		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "IP:%s Port:%s", szHostIP, szPort);
		inLogPrintf(AT, szDebugMsg);

	}
	
	if (ginISODebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "IP:%s Port:%s", szHostIP, szPort);
		inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
	}
	vdUtility_SYSFIN_LogMessage(AT, "IP:%s Port:%s", szHostIP, szPort);
	
	/* 一律先重置Handle */
	ginTrans_ClientFd = -1;
	
	/* 用原生C Socket */
	memset(szI_FES_Mode, 0x00, sizeof(szI_FES_Mode));
	inGetI_FES_Mode(szI_FES_Mode);
	memset(szCFESMode, 0x00, sizeof(szCFESMode));
	inGetCloud_MFES(szCFESMode);
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "IFES: %s, %s", szI_FES_Mode, szTRTFileName);
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* 確認是否斷網，斷了要重啟GPRS */
	if (inGPRS_Check_Network_Status() != VS_SUCCESS)
	{
		inGPRS_DeInitial();
		inGPRS_Initial();
	}
	
	/* 判斷是IFES及非大來，其他全跑IFES */
	/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
	if (memcmp(szI_FES_Mode, "Y", strlen("Y")) == 0	||
	    memcmp(szCFESMode, "Y", strlen("Y")) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Connect Flow: TLS");
			inLogPrintf(AT, szDebugMsg);
		}
		
		inRetVal = inGPRS_Connect_TLS_Flow(szHostIP, szPort);
		/* 標示現在連線方式，斷線後重新恢復成_ETHERNET_FLOW_CTOS_ */
		ginGPRSFlow = _GPRS_FLOW_IFES_;
	}
	/* 用native方式建Socket，之後要優化再考慮使用(優點自己決定Timeout等等細微設定) */
	else if (1)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Connect Flow: Native");
			inLogPrintf(AT, szDebugMsg);
		}
		
		inRetVal = inGPRS_Connect_By_Native(szHostIP, szPort);
		/* 標示現在連線方式，斷線後重新恢復成_GPRS_FLOW_CTOS_ */
		ginGPRSFlow = _GPRS_FLOW_NATIVE_;

	}
	/* 虹堡API */
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Connect Flow: NORMAL");
			inLogPrintf(AT, szDebugMsg);
		}
		
		/*  轉換IP表示形式 */
		inFunc_IP_Transform(szHostIP, (char*)uszIP);
		usPort = atoi(szPort);	
		inGPRS_Connect_Ex((unsigned char*)&ginTrans_ClientFd, uszIP, usPort);
		inRetVal = inGPRS_IsConnected();
		/* 標示現在連線方式，斷線後重新恢復成_GPRS_FLOW_CTOS_ */
		ginGPRSFlow = _GPRS_FLOW_CTOS_;
	}
	
	unsigned int	uiStatus = 0;
	/* Get the status of the Ethernet */
	inGPRS_GetStatus(&uiStatus);
	memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
	if (uiStatus & TCP_GPRS_STATE_ESTABLISHED)
	{
		strcat(szDebugMsg, "Established ");
	}
	else
	{
		 strcat(szDebugMsg, "NotEstablished ");
	}
	
	if (uiStatus & TCP_GPRS_STATE_CONNECTED)
	{
		strcat(szDebugMsg, "Connected ");
	}
	else
	{
		 strcat(szDebugMsg, "NotConnected ");
	}
		
	if (uiStatus & TCP_GPRS_STATE_ESTABLISHING)
	{
		strcat(szDebugMsg, "Establishing ");
	}
	else
	{
		 strcat(szDebugMsg, "NotEstablishing ");
	}
	
	if (uiStatus & TCP_GPRS_STATE_CONNECTING)
	{
		strcat(szDebugMsg, "Connecting ");
	}
	else
	{
		 strcat(szDebugMsg, "NotConnecting ");
	}
	
	if (uiStatus & TCP_GPRS_STATE_SENDING)
	{
		strcat(szDebugMsg, "Sending ");
	}
	else
	{
		 strcat(szDebugMsg, "NotSending ");
	}
	
	if (uiStatus & TCP_GPRS_STATE_RECEIVING)
	{
		strcat(szDebugMsg, "Receiving ");
	}
	else
	{
		 strcat(szDebugMsg, "NotReceiving ");
	}
	
	if (uiStatus & TCP_GPRS_STATE_DISCONNECTING)
	{
		strcat(szDebugMsg, "Disconnecting ");
	}
	else
	{
		 strcat(szDebugMsg, "NotDisconnecting ");
	}
	
	if (uiStatus & TCP_GPRS_STATE_ONHOOKING)
	{
		strcat(szDebugMsg, "Onhooking ");
	}
	else
	{
		 strcat(szDebugMsg, "NotOnhooking ");
	}
	
	vdUtility_SYSFIN_LogMessage(AT, "(%08X),%s", uiStatus, szDebugMsg);
	vdUtility_SYSFIN_LogMessage(AT, "GPRS Cconect:(way:%d)", ginGPRSFlow);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inGPRS_Connect_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
	return (inRetVal);
}

/*
Function        :inGPRS_Connect_Ex
Date&Time       :2018/3/1 下午 2:06
Describe        :GPRS連線(不含SSL)未測試
*/
int inGPRS_Connect_Ex(unsigned char *uszSocket, unsigned char *uszHostIP, unsigned short usPort)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "理論上GPRS要搭配SSL，所以此function未完整測試");
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inGPRS_Connect_Ex() START !");
	}
	
	/* 連線Timout 2秒 */
	inCOMM_TCP_SetConnectTO(30000);
	inCOMM_TCP_SetRetryCounter(10);
	
	usRetVal = CTOS_TCP_GPRSConnectEx(uszSocket, uszHostIP, usPort);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CTOS_TCP_GPRSConnectEx OK!");
		}
	}
	else if (usRetVal == d_TCP_IO_PROCESSING)
	{
//		if (ginDebug == VS_TRUE)
//		{
//			inLogPrintf(AT, "CTOS_TCP_GPRSConnectEx IO PROCESSING!");
//		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_TCP_GPRSConnectEx Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inGPRS_Connect_Ex() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_Connect_By_Native
Date&Time       :2018/3/8 下午 4:21
Describe        :原生C Socket連線
*/
int inGPRS_Connect_By_Native(char *szHostIP, char *szPort)
{
	int			inRetVal;
	int			inOption = 0;
	int			inFlags = 0;
	char			szDebugMsg[100 + 1];
	struct sockaddr_in	stAddr;			/* 放Host address的結構 */
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inGPRS_Connect_By_Native() START !");
	}
	
	/* 初始化address */
	/* clear all zero. */
	memset(&stAddr, 0x00, sizeof (stAddr));
	stAddr.sin_family = AF_INET;
	stAddr.sin_addr.s_addr = inet_addr(szHostIP);
	stAddr.sin_port = htons(atoi(szPort));

	ginTrans_ClientFd = socket(AF_INET, SOCK_STREAM, 0);
	if (ginTrans_ClientFd == -1)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Create Socket Fail");
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		inFile_Open_File_Cnt_Increase();
	}
	
	/* 綁定GPRS */
	CTOS_TCP_BindToDevice(ginTrans_ClientFd, d_TCP_DEVICE_GPRS);
	
	/* 系统預設的狀態發送與接收一次為8688 Bytes(約為8.5K) 應該夠用，所以不必特別調整buffer大小 */
	
	
	/* 開關 ON */
	inOption = 1;
	/* 允許重用本地位址和埠 */
	setsockopt(ginTrans_ClientFd, SOL_SOCKET, SO_REUSEADDR, &inOption, sizeof(inOption));

	/* Linux内核中对connect的超时时间限制是75s， Soliris 9是几分钟，因此通常认为是75s到几分钟不等*/
	/* 師爺給我翻譯翻譯：不用非阻塞Timeut就是75秒，所以一定要用非阻塞 */
	/* 先獲得現在Handle的開關狀態 */
	inFlags = fcntl(ginTrans_ClientFd, F_GETFL, 0);
	/* 把非阻塞開關On起來(做or運算)，並設定回去 */
	fcntl(ginTrans_ClientFd, F_SETFL, inFlags | O_NONBLOCK);
	
	/* 連線Timout 2秒 */
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 10);

	do
	{
		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "connect Errno: %d", errno);
				inLogPrintf(AT, szDebugMsg);
				
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Connect Timeout");
				inLogPrintf(AT, szDebugMsg);
			}
			
			return (VS_TIMEOUT);
		}
		
		inRetVal = connect(ginTrans_ClientFd, (void*)&stAddr, sizeof(stAddr));
		/* 通常只有同一台機器內Clinet連server，才有可能馬上成功 */
		if (inRetVal == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Connect Succeed imediately");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else
		{
			
		}
		
	} while (inRetVal != 0);

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inGPRS_Connect_By_Native() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_Connect_TLS_Flow
Date&Time       :2017/8/2 下午 4:10
Describe        :
*/
int inGPRS_Connect_TLS_Flow(char *szHostIP, char *szPort)
{
	int	inRetVal = VS_ERROR;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inGPRS_Connect_TLS_Flow() START !");
	}
	
	/* 1.建socket */
	inRetVal = inGPRS_Connect_By_Native(szHostIP, szPort);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	/* 先用新憑證 */
	memset(gszCACertPath, 0x00, sizeof(gszCACertPath));
		
	if (strlen(_CA_DATA_PATH_) > 0)
	{
		strcat(gszCACertPath, _CA_DATA_PATH_);
	}
	if (strlen(_PEM_NEW_TLS_FILE_NAME_) > 0)
	{
		strcat(gszCACertPath, _PEM_NEW_TLS_FILE_NAME_);
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
				inGPRS_DisConnect_By_Native();
				break;
			}
                }
                else
                {
			inRetVal = inTLS_Process_CTX_Flow(&guiCTX_ID);
			if (inRetVal != VS_SUCCESS)
			{
				inTLS_CTX_Free(&guiCTX_ID);
				inGPRS_DisConnect_By_Native();
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
					inLogPrintf(AT, "inGPRS_Connect_TLS_Flow() END !");
					inLogPrintf(AT, "----------------------------------------");
				}

				return (inRetVal);
			}
			else
			{
				inTLS_OPENSSL_TLS_Disconnect(&gSsl);
				vdTLS_OPENSSL_TLS_Free(&gSsl);
				vdTLS_OPENSSL_CTX_Free(&gCtx);
				inGPRS_DisConnect_By_Native();
			}
                }
                else
                {
			inRetVal = inTLS_Process_TLS_Flow(&guiCTX_ID, &guiSSL_ID);
			if (inRetVal == VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "First CER Success");
					inLogPrintf(AT, "inGPRS_Connect_TLS_Flow() END !");
					inLogPrintf(AT, "----------------------------------------");
				}

				return (inRetVal);
			}
			else
			{
				inTLS_TLS2_Disconnect(guiSSL_ID, 0);
				inTLS_TLS2_Free(&guiSSL_ID);
				inTLS_CTX_Free(&guiCTX_ID);
				inGPRS_DisConnect_By_Native();
			}
		}
		
		break;
	}while (1);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "First CER Fail");
	}
	
	/* 換舊憑證 */
	memset(gszCACertPath, 0x00, sizeof(gszCACertPath));	
	if (strlen(_CA_DATA_PATH_) > 0)
	{
		strcat(gszCACertPath, _CA_DATA_PATH_);
	}
	if (strlen(_PEM_PRESERVE_TLS_FILE_NAME_) > 0)
	{
		strcat(gszCACertPath, _PEM_PRESERVE_TLS_FILE_NAME_);
	}
	
	
	do
	{
		/* 2.處理CTX */
		inRetVal = inTLS_Process_CTX_Flow(&guiCTX_ID);
		if (_TLS_API_WAY_ == _TLS_API_WAY_OPENSSL_)
                {
                    inRetVal = inTLS_OPENSSL_Process_CTX_Flow(&gCtx);
                    if (inRetVal != VS_SUCCESS)
                    {
                            vdTLS_OPENSSL_CTX_Free(&gCtx);
			    inGPRS_DisConnect_By_Native();
                            break;
                    }
                }
                else
                {
                    inRetVal = inTLS_Process_CTX_Flow(&guiCTX_ID);
                    if (inRetVal != VS_SUCCESS)
                    {
                            inTLS_CTX_Free(&guiCTX_ID);
			    inGPRS_DisConnect_By_Native();
                            break;
                    }
                }

		/* 3,處理TLS */
		inRetVal = inTLS_Process_TLS_Flow(&guiCTX_ID, &guiSSL_ID);
		if (_TLS_API_WAY_ == _TLS_API_WAY_OPENSSL_)
                {
			inRetVal = inTLS_OPENSSL_Process_SSL_Flow(&gCtx, &gSsl);
			if (inRetVal == VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "Second CER Success");
					inLogPrintf(AT, "inGPRS_Connect_TLS_Flow() END !");
					inLogPrintf(AT, "----------------------------------------");
				}

				return (inRetVal);
			}
			else
			{
				inTLS_OPENSSL_TLS_Disconnect(&gSsl);
				vdTLS_OPENSSL_TLS_Free(&gSsl);
				vdTLS_OPENSSL_CTX_Free(&gCtx);
				inGPRS_DisConnect_By_Native();
			}
                }
                else
                {
			inRetVal = inTLS_Process_TLS_Flow(&guiCTX_ID, &guiSSL_ID);
			if (inRetVal == VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "Second CER Success");
					inLogPrintf(AT, "inGPRS_Connect_TLS_Flow() END !");
					inLogPrintf(AT, "----------------------------------------");
				}

				return (inRetVal);
			}
			else
			{
				inTLS_TLS2_Disconnect(guiSSL_ID, 0);
				inTLS_TLS2_Free(&guiSSL_ID);
				inTLS_CTX_Free(&guiCTX_ID);
				inGPRS_DisConnect_By_Native();
			}
		}
		
		break;
	}while(1);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "Second CER Fail");
		inLogPrintf(AT, "inGPRS_Connect_TLS_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
				
	return (inRetVal);
}

/*
Function        :inGPRS_DisConnect_Flow
Date&Time       :2018/3/1 下午 6:24
Describe        :分流
*/
int inGPRS_DisConnect_Flow(void)
{
	int	inRetVal = VS_ERROR;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inGPRS_DisConnect_Flow() START !");
	}
	
	if (ginGPRSFlow == _GPRS_FLOW_IFES_)
	{
		inRetVal = inGPRS_DisConnect_TLS_Flow();
		if (inRetVal == VS_SUCCESS)
		{
			ginGPRSFlow = _GPRS_FLOW_CTOS_;
		}
		else
		{
			ginGPRSFlow = _GPRS_FLOW_CTOS_;
			return (VS_ERROR);
		}
	}
	/* 用native方式建Socket，之後要優化再考慮使用(優點自己決定Timeout等等細微設定) */
	else if (ginGPRSFlow == _GPRS_FLOW_NATIVE_)
	{
		inRetVal = inGPRS_DisConnect_By_Native();
		if (inRetVal == VS_SUCCESS)
		{
			ginGPRSFlow = _GPRS_FLOW_CTOS_;
		}
		else
		{
			return (VS_ERROR);
		}
	}
	/* 虹堡API */
	else
	{
		inRetVal = inGPRS_Disconnect((unsigned char)ginTrans_ClientFd);
	}
	
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inGPRS_DisConnect_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_Disconnect
Date&Time       :2018/3/1 下午 2:06
Describe        :
*/
int inGPRS_Disconnect(unsigned char uszSocket)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inGPRS_Disconnect() START !");
	}
	
	usRetVal = CTOS_TCP_GPRSDisconnect(uszSocket);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CTOS_TCP_GPRSDisconnect OK!");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_TCP_GPRSDisconnect Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inGPRS_Disconnect() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_DisConnect_By_Native
Date&Time       :2017/8/2 下午 5:37
Describe        :
*/
int inGPRS_DisConnect_By_Native(void)
{
	int		inRetVal = VS_ERROR;
	char		szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inGPRS_DisConnect_By_Native() START !");
	}
	
	inRetVal = close(ginTrans_ClientFd);
	if (inRetVal == 0)
	{
		inRetVal= VS_SUCCESS;
		inFile_Open_File_Cnt_Decrease();
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Disconnect Errno: %d", errno);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inGPRS_DisConnect_By_Native() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_DisConnect_TLS_Flow
Date&Time       :2017/8/2 下午 5:45
Describe        :
*/
int inGPRS_DisConnect_TLS_Flow(void)
{
	int	inRetVal = VS_ERROR;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inGPRS_DisConnect_TLS_Flow() START !");
	}
	
	inTLS_TLS2_Disconnect(guiSSL_ID, 0);
	
	inRetVal = inTLS_TLS2_Free(&guiSSL_ID);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	inRetVal = inTLS_CTX_Free(&guiCTX_ID);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	inRetVal = inGPRS_DisConnect_By_Native();
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inGPRS_DisConnect_TLS_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_Send_Ready_Flow
Date&Time       :2017/8/1 下午 2:19
Describe        :分流
*/
int inGPRS_Send_Ready_Flow()
{
	int	inRetVal = VS_ERROR;
	
	if (ginGPRSFlow == _GPRS_FLOW_IFES_)
	{
		inRetVal = inGPRS_Send_Ready_TLS_CTOS(guiSSL_ID);
	}
	/* 用native方式建Socket，之後要優化再考慮使用(優點自己決定Timeout等等細微設定) */
	else if (ginGPRSFlow == _GPRS_FLOW_NATIVE_)
	{
		inRetVal = inGPRS_Send_Ready_By_Native(ginTrans_ClientFd);
	}
	else
	{
		inRetVal = inGPRS_Send_Ready();
	}
	
	return (inRetVal);
}

/*
Function        :inGPRS_Send_Ready
Date&Time       :2017/7/18 下午 5:28
Describe        :表示現在GPRS可以傳送資料
*/
int inGPRS_Send_Ready()
{
	unsigned int	uiStatus = 0;
	unsigned char	uszEnable = VS_TRUE;
	
	if (inGPRS_GetSocketStatus(&uiStatus, (unsigned char)ginTrans_ClientFd) != VS_SUCCESS)
		return (VS_ERROR);
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_ESTABLISHED)
	{
		
	}
	else
	{
		inLogPrintf(AT, "SendReady : Not ESTABLISHED");
		uszEnable = VS_TRUE;
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_CONNECTED)
	{
		
	}
	else
	{
		inLogPrintf(AT, "SendReady : Not CONNECTED");
		uszEnable = VS_TRUE;
	}
		
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_SENDING)
	{
		inLogPrintf(AT, "SendReady : SENDING");
		uszEnable = VS_TRUE;
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_RECEIVING)
	{
		inLogPrintf(AT, "SendReady : RECEIVING");
		uszEnable = VS_TRUE;
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_DISCONNECTING)
	{
		inLogPrintf(AT, "SendReady : DISCONNECTING");
		uszEnable = VS_TRUE;
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_ONHOOKING)
	{
		
	}
	else
	{
		inLogPrintf(AT, "SendReady : ONHOOKING");
		uszEnable = VS_TRUE;
	}
	
	if (uszEnable == VS_TRUE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inGPRS_Send_Ready_By_Native
Date&Time       :2023/4/18 下午 6:00
Describe        :
*/
int inGPRS_Send_Ready_By_Native(int inFileHandle)
{
	fd_set		stWfd;			/* 用來判斷是否可以寫的Fd */
	struct timeval	stTimeVal;
	
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
		
	}
	else
	{
		inUtility_StoreTraceLog_OneStep("inGPRS_Send_Ready_By_Native Fail, errno:%d", errno);
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_Send_Ready_TLS
Date&Time       :2017/8/2 下午 4:56
Describe        :表示現在TLS可以傳送資料
*/
int inGPRS_Send_Ready_TLS_CTOS(unsigned int uiSSL_ID)
{
	int	inRetVal = VS_ERROR;
	
	inRetVal = inTLS_TLS2_GetSession(uiSSL_ID);
	
	return (inRetVal);
}

/*
Function        :inGPRS_Send_Ready_TLS_OPENSSL
Date&Time       :2024/8/20 下午 3:11
Describe        :表示現在TLS可以傳送資料
*/
int inGPRS_Send_Ready_TLS_OPENSSL(SSL **ssl)
{
	int	inRetVal = 0;
	
	inRetVal = SSL_is_init_finished(*ssl);
	if (inRetVal == 1)
        {
                return (VS_SUCCESS);
        }
        else
	{
		
                return (VS_ERROR);
	}
}

/*
Function        :inGPRS_Send_Data_Flow
Date&Time       :2017/8/1 下午 2:19
Describe        :分流
*/
int inGPRS_Send_Data_Flow(unsigned char* uszData, unsigned short usLen)
{
	int	inRetVal = VS_ERROR;
	
	if (ginGPRSFlow == _GPRS_FLOW_IFES_)
	{
		if (_TLS_API_WAY_ == _TLS_API_WAY_OPENSSL_)
                {
			inRetVal = inGPRS_Send_Data_TLS_OPENSSL(&gSsl, uszData, usLen);
                }
                else
                {
			inRetVal = inGPRS_Send_Data_TLS_CTOS(guiSSL_ID, uszData, usLen);
		}
	}
	/* 用native方式建Socket，之後要優化再考慮使用(優點自己決定Timeout等等細微設定) */
	else if (ginGPRSFlow == _GPRS_FLOW_NATIVE_)
	{
		inRetVal = inGPRS_Send_Data_By_Native(ginTrans_ClientFd, uszData, usLen);
	}
	else
	{
		inRetVal = inGPRS_Send_Data((unsigned char)ginTrans_ClientFd, uszData, usLen);
	}
	
	return (inRetVal);
}

/*
Function        :inGPRS_Send_Data
Date&Time       :2017/7/18 下午 5:28
Describe        :傳送資料
*/
int inGPRS_Send_Data(unsigned char uszSocket, unsigned char* uszData, unsigned short usLen)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	usRetVal = CTOS_TCP_GPRSTx(uszSocket, uszData, usLen);
	if (usRetVal == d_OK)
	{
		
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_GPRSTxData Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_Send_Data_By_Native
Date&Time       :2023/4/18 下午 6:02
Describe        :傳送資料
*/
int inGPRS_Send_Data_By_Native(int inFileHandle, unsigned char* uszData, unsigned short usLen)
{
	int	inTempLen = 0;
	char	szDebugMsg[100 + 1];
	
	/* Send message */  
	inTempLen = send(inFileHandle, uszData, usLen, 0);
	
	if (inTempLen >= 0)
	{
		return (VS_SUCCESS);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "send Errno: %d", errno);
			inLogPrintf(AT, szDebugMsg);
		}
		inUtility_StoreTraceLog_OneStep("inGPRS_Send_Data_By_Native Fail, Errno: %d", errno);
		
		return (VS_ERROR);
	}
	
}

/*
Function        :inGPRS_Send_Data_TLS
Date&Time       :2017/8/2 下午 5:03
Describe        :傳送資料
*/
int inGPRS_Send_Data_TLS_CTOS(unsigned int uiSSL_ID, unsigned char* uszData, unsigned short usLen)
{
	int	inRetVal = VS_ERROR;
	int	inDataLen = 0;
	
	inDataLen = usLen;
	
	inRetVal = inTLS_Send_Data(uiSSL_ID, uszData, &inDataLen);
	
	usLen = (unsigned short)inDataLen;
	
	return (inRetVal);
}

/*
Function        :inGPRS_Send_Data_OPENSSL_TLS
Date&Time       :2024/8/20 下午 3:18
Describe        :傳送資料
*/
int inGPRS_Send_Data_TLS_OPENSSL(SSL **ssl, unsigned char *uszData, unsigned short usLen)
{
	int	inRetVal = VS_ERROR;
        int	inDataLen = 0;
	
	inDataLen = usLen;
	
	inRetVal = inTLS_OPENSSL_SSL_Send_Data(ssl, uszData, &inDataLen);
	
	usLen = (unsigned short)inDataLen;
        
	return (inRetVal);
}

/*
Function        :inGPRS_Receive_Ready_Flow
Date&Time       :2018/3/2 上午 11:19
Describe        :目前只有虹堡API才有receive check的功能，用native沒辦法實做，所以決定不使用
 */
int inGPRS_Receive_Ready_Flow(unsigned short *usLen)
{
	int		inRetVal;
	unsigned short	usBufferMaxLen = 0;

	if (ginGPRSFlow == _GPRS_FLOW_IFES_)
	{
		if (_TLS_API_WAY_ == _TLS_API_WAY_OPENSSL_)
                {
                        inRetVal = inGPRS_Receive_Ready_TLS_OPENSSL(&gSsl, usLen);
                }
                else
                {
			inRetVal = inGPRS_Receive_Ready_TLS_CTOS(guiSSL_ID, usLen);
		}
	}
	/* 用native方式建Socket，之後要優化再考慮使用(優點自己決定Timeout等等細微設定) */
	else if (ginGPRSFlow == _GPRS_FLOW_NATIVE_)
	{
		inRetVal = inGPRS_Receive_Ready_By_Native(ginTrans_ClientFd, usLen);
	}
	else
	{
		usBufferMaxLen = *usLen;
		inRetVal = inGPRS_Receive_Ready(usLen);
		if (*usLen > usBufferMaxLen)
		{
			*usLen = usBufferMaxLen;
		}
	}
	
	return (inRetVal);
}

/*
Function        :inGPRS_Receive_Ready
Date&Time       :2017/7/18 下午 5:57
Describe        :表示現在GPRS可以接收資料
*/
int inGPRS_Receive_Ready(unsigned short *usLen)
{
	unsigned int	uiStatus = 0;
	unsigned char	uszEnable = VS_TRUE;
	
	if (inGPRS_GetSocketStatus(&uiStatus, (unsigned char)ginTrans_ClientFd) != VS_SUCCESS)
		return (VS_ERROR);
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_ESTABLISHED)
	{
		
	}
	else
	{
		inLogPrintf(AT, "SendReady : Not ESTABLISHED");
		uszEnable = VS_TRUE;
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_CONNECTED)
	{
		
	}
	else
	{
		inLogPrintf(AT, "SendReady : Not CONNECTED");
		uszEnable = VS_TRUE;
	}
		
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_SENDING)
	{
		inLogPrintf(AT, "SendReady : SENDING");
		uszEnable = VS_TRUE;
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_RECEIVING)
	{
		inLogPrintf(AT, "SendReady : RECEIVING");
		uszEnable = VS_TRUE;
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_DISCONNECTING)
	{
		inLogPrintf(AT, "SendReady : DISCONNECTING");
		uszEnable = VS_TRUE;
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_ONHOOKING)
	{
		
	}
	else
	{
		inLogPrintf(AT, "SendReady : ONHOOKING");
		uszEnable = VS_TRUE;
	}
	
	if (uszEnable == VS_TRUE)
	{
		return (VS_ERROR);
	}
	else
	{
		/* Native沒有check功能，所以直接設大一點，等到read時再根據實際回傳讀取 */
		*usLen = _COMM_RECEIVE_MAX_LENGTH_;
		
		return (VS_SUCCESS);
	}
}

/*
Function        :inGPRS_Receive_Ready_By_Native
Date&Time       :2023/4/18 下午 6:03
Describe        :
*/
int inGPRS_Receive_Ready_By_Native(int inFileHandle, unsigned short *usLen)
{
	fd_set		stRfd;			/* 用來判斷是否可以讀的Fd */
	struct timeval	stTimeVal;
	
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
	}
	else
	{
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_Receive_Ready_TLS
Date&Time       :2017/8/2 下午 4:56
Describe        :表示現在TLS可以接收資料
*/
int inGPRS_Receive_Ready_TLS_CTOS(unsigned int uiSSL_ID, unsigned short *usLen)
{
	int	inRetVal = VS_ERROR;
	
	inRetVal = inTLS_TLS2_GetSession(uiSSL_ID);
	if (inRetVal == VS_SUCCESS)
	{
		*usLen = _COMM_RECEIVE_MAX_LENGTH_;
	}
	else
	{
		*usLen = 0;
	}
	
	return (inRetVal);
}

/*
Function        :inGPRS_Receive_Ready_TLS_OPENSSL
Date&Time       :2024/8/20 下午 3:19
Describe        :表示現在TLS可以接收資料
 *		經測試SSL_peek可用，SSL_pending不可用
*/
int inGPRS_Receive_Ready_TLS_OPENSSL(SSL **ssl, unsigned short *usLen)
{
	int	inRetVal = 0;
	char	buffer[_COMM_RECEIVE_MAX_LENGTH_] = {0};  // 用來呼叫 SSL_peek，實際上我們不會使用這個緩衝區

	inRetVal = SSL_peek(*ssl, buffer, sizeof(buffer));
	if (inRetVal > 0)
	{
		*usLen = inRetVal;
                
                return (VS_SUCCESS);
	}
	else
	{
		*usLen = 0;
                
                return (VS_ERROR);
	}
}

/*
Function        :inGPRS_Receive_Data_Flow
Date&Time       :2017/8/1 下午 3:43
Describe        :分流
*/
int inGPRS_Receive_Data_Flow(unsigned char* uszData, unsigned short *usLen)
{
	int	inRetVal = VS_ERROR;
	
	if (ginGPRSFlow == _GPRS_FLOW_IFES_)
	{
		if (_TLS_API_WAY_ == _TLS_API_WAY_OPENSSL_)
                {
			inRetVal = inGPRS_Receive_Data_TLS_OPENSSL(&gSsl, uszData, usLen);
                }
                else
                {
			inRetVal = inGPRS_Receive_Data_TLS_CTOS(guiSSL_ID, uszData, usLen);
		}
	}
	/* 用native方式建Socket，之後要優化再考慮使用(優點自己決定Timeout等等細微設定) */
	else if (ginGPRSFlow == _GPRS_FLOW_NATIVE_)
	{
		inRetVal = inGPRS_Receive_Data_By_Native(ginTrans_ClientFd, uszData, usLen);
	}
	else
	{
		inRetVal = inGPRS_Receive_Data((unsigned char)ginTrans_ClientFd, uszData, usLen);
	}
	
	
	return (inRetVal);
}

/*
Function        :inGPRS_Receive_Data
Date&Time       :2017/7/18 下午 6:03
Describe        :接收資料
*/
int inGPRS_Receive_Data(unsigned char uszSocket, unsigned char* uszData, unsigned short *usLen)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = 0;
	
	usRetVal = CTOS_TCP_GPRSRx(uszSocket, uszData, usLen);
	if (usRetVal == d_OK)
	{
		
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_GPRSRxData Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_Receive_Data_By_Native
Date&Time       :2017/8/1 下午 3:45
Describe        :
*/
int inGPRS_Receive_Data_By_Native(int inFileHandle, unsigned char* uszData, unsigned short *usLen)
{
	int	inTempLen = 0;
	
	
	inTempLen = recv(inFileHandle, uszData, *usLen, 0);
	
	/* 如果回傳-1會變成65535，所以要做安全檢核 */
	if (inTempLen > 0)
	{
		*usLen = (unsigned short)inTempLen;
	}
	else
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
Function        :inGPRS_Receive_Data_TLS
Date&Time       :2017/8/2 下午 5:03
Describe        :接收資料
*/
int inGPRS_Receive_Data_TLS_CTOS(unsigned int uiSSL_ID, unsigned char* uszData, unsigned short *usLen)
{
	int	inRetVal = VS_ERROR;
	
	inRetVal = inTLS_Read_Data(uiSSL_ID, uszData, (int*)usLen);
	
	return (inRetVal);
}

/*
Function        :inGPRS_Receive_Data_TLS_OPENSSL
Date&Time       :2024/8/20 下午 3:20
Describe        :接收資料
*/
int inGPRS_Receive_Data_TLS_OPENSSL(SSL **ssl, unsigned char* uszData, unsigned short *usLen)
{
	int	inRetVal = VS_ERROR;
	int	inDataLen = 0;
	
	inDataLen = *usLen;
	inRetVal = inTLS_OPENSSL_SSL_Read_Data(ssl, uszData, &inDataLen);
	*usLen = inDataLen;
	
	return (inRetVal);
}

/*
Function        :inGPRS_Initial
Date&Time       :2018/3/1 下午 3:38
Describe        :
*/
int inGPRS_Initial(void)
{
	char		szAPNName[100 + 1] = {0};
	char		szIP[15 + 1] = {0};
	unsigned char	uszTempIP[4 + 1] = {0};
	
        /* inGPRS_Initial() START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inGPRS_Initial() START！");
        }
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SET_COMM_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示<通訊設定> */
	
	memset(szAPNName, 0x00, sizeof(szAPNName));
	inGetGPRS_APN(szAPNName);
	if (strlen(szAPNName) <= 0)
	{
		strcpy(szAPNName, "internet");
	}

	inGPRS_CheckStatus();
	inGPRS_Open((unsigned char*)"", (unsigned char*)szAPNName, (unsigned char*)"", (unsigned char*)"");
	inGPRS_CheckStatus();
	
	memset(uszTempIP, 0x00, sizeof(uszTempIP));
	inGPRS_Get_IP(uszTempIP);
	memset(szIP, 0x00, sizeof(szIP));
	sprintf(szIP, "%u.%u.%u.%u", (unsigned int)uszTempIP[0], (unsigned int)uszTempIP[1], (unsigned int)uszTempIP[2], (unsigned int)uszTempIP[3]);
	inSetTermIPAddress(szIP);
	inSaveEDCRec(0);
	
        /* inGPRS_Initial()_END */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inGPRS_Initial()_END");
        }

        return (VS_SUCCESS);
}

/*
Function        :inGPRS_Begin
Date&Time       :2018/3/1 下午 3:44
Describe        :
*/
int inGPRS_Begin(TRANSACTION_OBJECT *pobtran)
{
        char    szCommIndex[2 + 1];
        char    szDebugBuf[64 + 1];

        /* inGPRS_Begin() START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inGPRS_Begin() START！");
        }

        memset(szCommIndex, 0x00, sizeof(szCommIndex));
        /* 從HDT內get CPT的index */
        if (inGetCommunicationIndex(szCommIndex) == VS_ERROR)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inGetCommunicationIndex Error!");
                
                return (VS_ERROR);
        }
        else
        {
                /* CPT index從0開始，但是CPTRec index從1開始，所以要減一  */
                pobtran->srBRec.inCPTIndex = atoi(szCommIndex) - 1;
        }

        if (inLoadCPTRec(pobtran->srBRec.inCPTIndex) == VS_ERROR)
        {
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugBuf, "inLoadCPTRec(%d) Error!", pobtran->srBRec.inCPTIndex);
                        inLogPrintf(AT, szDebugBuf);
                }
                /* Load CPT失敗 */
                return (VS_ERROR);
        }

        /* inGPRS_Begin()_END */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inGPRS_Begin()_END");
        }

        return (VS_SUCCESS);
}

/*
Function        :inGPRS_SetConfig
Date&Time       :2018/3/1 下午 3:45
Describe        :如果IP衝突會Connect會TimeOut
*/
int inGPRS_SetConfig(void)
{
	int	inRetVal = 0;
	int	inConnectMaxCnt = 1;		/* 連線最大重試次數(含第一次) */
	int	inConnectNowCnt = 0;		/* 連線目前重試次數 */
	char	szHostIPPrimary[16 + 1] = {0};
	char	szHostIPSecond[16 + 1] = {0};
	char	szHostPortNoPrimary[6 + 1] = {0};
	char	szHostPortNoSecond[6 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inGPRS_SetConfig() START!");
	
	/* Set Host IP */
        memset(szHostIPPrimary,0x00,sizeof(szHostIPPrimary));
        if (inGetHostIPPrimary(szHostIPPrimary) == VS_ERROR)
        {
                /* inGetHostIPPrimary ERROR */
                /* debug */
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inGetHostIPPrimary() ERROR!!");

                return (VS_ERROR);
        }
         
	/* Set Host Port */
        memset(szHostPortNoPrimary,0x00,sizeof(szHostPortNoPrimary));
        if (inGetHostPortNoPrimary(szHostPortNoPrimary) == VS_ERROR)
        {
                /* Get HostPortNumber Primary ERROR */
                /* debug */
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inGetHostPortNoPrimary() ERROR!!");
                
                return (VS_ERROR);
        }

	/* 重置目前重試次數 */
	inConnectNowCnt = 0;
	/* 若連線失敗看是否要重試 */
	do 
	{
		inRetVal = inGPRS_Connect_Flow(szHostIPPrimary, szHostPortNoPrimary);
		inConnectNowCnt ++;
		
	}while (inRetVal != VS_SUCCESS && inConnectNowCnt < inConnectMaxCnt);
        
	
	/* 第一授權主機連線成功 */
        if (inRetVal == VS_SUCCESS)
        {
                return (VS_SUCCESS);
        }
        else
	{
               /* 若第一授權主機連線不成功，改連第二授權主機 */ 
	}
	
	/* 看狀態 */
	inGPRS_CheckStatus();
	
	/* 第一授權IP失敗 嘗試第二IP */
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "First IP Conenct Fail");
		inLogPrintf(AT, szDebugMsg);
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Try Second IP...");
		inLogPrintf(AT, szDebugMsg);
	}
	
	inDISP_Clear_Line(_LINE_8_6_, _LINE_8_6_);
	inDISP_ChineseFont("第二組IP", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
	/* 嘗試連線第二授權主機 */
	/* 更改HostIP */
	/* Set Host IP */
        memset(szHostIPSecond,0x00,sizeof(szHostIPSecond));
        if (inGetHostIPSecond(szHostIPSecond) == VS_ERROR)
        {
                /* inGetHostIPSecond ERROR */
                /* debug */
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inGetHostIPSecond() ERROR!!");

                return (VS_ERROR);
        }
	
	/* Set Host Port */
        memset(szHostPortNoSecond, 0x00, sizeof(szHostPortNoSecond));
        if (inGetHostPortNoSecond(szHostPortNoSecond) == VS_ERROR)
        {
                /* Get HostPortNumber Primary ERROR */
                /* debug */
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inGetHostPortNoSecond() ERROR!!");
                
                return (VS_ERROR);
        }
        		
	/* 重置目前重試次數 */
	inConnectNowCnt = 0;
	/* 若連線失敗看是否要重試 */
	do 
	{
		inRetVal = inGPRS_Connect_Flow(szHostIPSecond, szHostPortNoSecond);
		inConnectNowCnt ++;
	}while (inRetVal != VS_SUCCESS && inConnectNowCnt < inConnectMaxCnt);

	/* 第二授權主機連線成功 */
        if (inRetVal == VS_SUCCESS)
        {
                return (VS_SUCCESS);
        }
        else
	{
               /* 若第二授權主機連線不成功，直接斷線 */ 
	}
	
	/* 第二授權IP失敗 */
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Second IP Conenct Fail");
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* 到最後還失敗，斷線 */
	inGPRS_END();
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inGPRS_SetConfig() END!");
        
	/* 若連線成功，中間就會Return出去，若一直失敗，則最後一定回傳失敗 */
        return (VS_ERROR);
}

/*
Function        :inGPRS_Send
Date&Time       :2018/3/2 上午 10:51
Describe        :
*/
int inGPRS_Send(unsigned char *uszSendBuff, int inSendSize, int inSendTimeout)
{
        char            szDataHead[1 + 1];
	char		szDebugMsg[100 + 1];
        unsigned char   szSendData[inSendSize + 32];

        memset(szSendData, 0x00, sizeof(szSendData));
        memset(szDataHead, 0x00, sizeof(szDataHead));

        /* inGPRS_Send() START! */
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inGPRS_Send() START!");

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
        }
        else
        {
		szSendData[0] = (inSendSize / 100) / 10 * 16 + (inSendSize / 100) % 10;
		szSendData[1] = (inSendSize % 100) / 10 * 16 + (inSendSize % 100) % 10;
        }
	
	if (ginISODebug == VS_TRUE)
	{
		inPRINT_ChineseFont("----------------------------------------",_PRT_ISO_);
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "%02X %02X :length = %d", szSendData[0], szSendData[1], inSendSize);
		inPRINT_ChineseFont_Format(szDebugMsg, "  ", 34, _PRT_ISO_);
		inPRINT_ChineseFont("----------------------------------------", _PRT_ISO_);
	}
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
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
                                        inLogPrintf(AT, "inGPRS_Send() TIMEOUT");
				}
				
				return (VS_ERROR);
			}
		}

		/* 如果可以Send就Send */
		if (inGPRS_Send_Ready_Flow()== VS_SUCCESS)
		{
			if (inGPRS_Send_Data_Flow(szSendData, inSendSize) != VS_SUCCESS)
			{
				continue;
			}
			else
				break;
		}
	}

        /* inGPRS_Send() END! */
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inGPRS_Send() END!");

        return (VS_SUCCESS);
}

/*
Function        :inGPRS_Receive
Date&Time       :
Describe        :
*/
int inGPRS_Receive(unsigned char *uszReceiveBuff, int inReceiveSize, int inReceiveTimeout)
{
	int		inRetVal = 0;
        int     	inReceivelen = 0;			/* Comport當前收到的長度 */
        int     	inDataLength = 0;			/* 收到的資料長度(不含Head) */
	int		inExpectLength = 0;
        char    	szDataHead[1 + 1];
        char    	szDebugMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        unsigned char 	uszRawBuffer[inReceiveSize + 1];
	unsigned short 	usOutputLen = 0;			/* 目前comport可以讀取的資料長度 */
	
        /* inGPRS_Receive() START! */
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inGPRS_Receive() START!");

	memset(uszRawBuffer, 0x00, sizeof(uszRawBuffer));

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
						inLogPrintf(AT, "inGPRS_Receive TimeOut 1");
					}
					
				}
                        	return (VS_TIMEOUT);
                        }
                }
		
		/* 設定為剩餘的Buffer空間大小，才符合API的用法 */
		usOutputLen = inReceiveSize - inReceivelen;
		
		if (inGPRS_Receive_Ready_Flow(&usOutputLen) == VS_SUCCESS)
		{
			inRetVal = inGPRS_Receive_Data_Flow(&uszRawBuffer[inReceivelen], &usOutputLen);
			if (inRetVal == VS_SUCCESS)
			{
				inReceivelen = inReceivelen + (int)usOutputLen;
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
        memset(szDataHead, 0x00, sizeof(szDataHead));
        if (inGetTCPHeadFormat(szDataHead) == VS_ERROR)
        {
                /* inGetTCPHeadFormat ERROR */
                /* debug */
                if (ginDebug == VS_TRUE)
                                inLogPrintf(AT, "inGetTCPHeadFormat() ERROR!!");

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
		}
		else
		{
			inExpectLength = ((uszRawBuffer[0] / 16 * 10 + uszRawBuffer[0] % 16 ) * 100) + (uszRawBuffer[1] / 16 * 10 + uszRawBuffer[1] % 16);
		}
		
		if (ginISODebug == VS_TRUE)
		{
			inPRINT_ChineseFont("----------------------------------------",_PRT_ISO_);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%02X %02X :length = %d", uszRawBuffer[0], uszRawBuffer[1], inExpectLength);
			inPRINT_ChineseFont_Format(szDebugMsg, "  ", 34, _PRT_ISO_);
			inPRINT_ChineseFont("----------------------------------------", _PRT_ISO_);
		}
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "----------------------------------------");
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
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
			memset(uszRawBuffer, 0x00, sizeof(uszRawBuffer));
			
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
								inLogPrintf(AT, "inGPRS_Receive TimeOut 2");
							}

							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "Receve Len: %d", inDataLength);
							inLogPrintf(AT, szDebugMsg);
						}
                                                return (VS_TIMEOUT);
                                        }
                                }

				/* 設定為剩餘的Buffer空間大小，才符合API的用法 */
				usOutputLen = inReceiveSize - inReceivelen;
				
				if (inGPRS_Receive_Ready_Flow(&usOutputLen) == VS_SUCCESS)
				{
					inRetVal = inGPRS_Receive_Data_Flow(&uszRawBuffer[inReceivelen], &usOutputLen);
					if (inRetVal == VS_SUCCESS)
					{
						inReceivelen = inReceivelen + (int)usOutputLen;
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

	}
	/* 連Head都沒收到 */
	else
	{
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                        sprintf(szDebugMsg, "inReceivelen ERROR!! :(inReceivelen = %d)", inReceivelen);
                        inLogPrintf(AT, szDebugMsg);
                }

		return (VS_ERROR);
	}
	     
        /* inGPRS_Receive() END! */
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inGPRS_Receive() END!");

	return (inDataLength);
}

/*
Function        :inGPRS_END
Date&Time       :
Describe        :
*/
int inGPRS_END(void)
{
        /* inGPRS_END() START! */
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inGPRS_END() START!");
	
	if (inGPRS_DisConnect_Flow() != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inGPRS_Disconnect() == VS_ERROR");
		return (VS_ERROR);
	}

        /* inGPRS_END() END! */
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inGPRS_END() END!");

        return (VS_SUCCESS);
}

/*
Function        :inGPRS_Flush
Date&Time       :2018/3/2 上午 9:22
Describe        :
*/
int inGPRS_Flush(void)
{
        /* inGPRS_Flush() START! */
	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inGPRS_Flush() START!");
	
        /* inGPRS_Flush() END! */
	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inGPRS_Flush() END!");

        return (VS_SUCCESS);
}

/*
Function        :inGPRS_DeInitial
Date&Time       :
Describe        :
*/
int inGPRS_DeInitial(void)
{
	int	inRetval = 0;
        
        /* inGPRS_DeInitial() START! */
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inGPRS_DeInitial() START!");

	inRetval = inGPRS_Close();
	if (inRetval != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inGPRS_DeInitial Failed");
		}
	}
	
        /* inGPRS_DeInitial() END! */
	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inGPRS_DeInitial() END!");

        return (VS_SUCCESS);
}

/*
Function        :inGPRS_Status
Date&Time       :2018/3/2 上午 9:50
Describe        :
*/
int inGPRS_GetStatus(unsigned int *uiStatus)
{
	char		szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1] = {0};   /* debug message */
	unsigned short  usRetval = 0;
	
	if (uiStatus == NULL)
		return (VS_ERROR);
	
	do
	{
		usRetval = CTOS_TCP_GPRSStatus(uiStatus);
	
		if (usRetval == d_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CTOS_TCP_GPRSStatus OK");
			}
			break;
		}
		else if (usRetval == d_TCP_IO_PROCESSING)
		{
//			if (ginDebug == VS_TRUE)
//			{
//				inLogPrintf(AT, "CTOS_TCP_GPRSStatus IOProcessing");
//			}
		}
		else
		{
			/* debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "CTOS_TCP_GPRSStatus Error : 0x%04x", usRetval);
				inLogPrintf(AT, szErrorMsg);
			}
			return (VS_ERROR);
		}
		
	} while (usRetval == d_TCP_IO_PROCESSING);
	
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_CheckStatus
Date&Time       :2018/3/1 下午 3:59
Describe        :
*/
int inGPRS_CheckStatus(void)
{
	unsigned int	uiStatus = 0;

	/* Get the status of the GPRS */
	if (inGPRS_GetStatus(&uiStatus) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_ESTABLISHED)
	{
		inLogPrintf(AT, "GPRSStatus : ESTABLISHED");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_CONNECTED)
	{
		inLogPrintf(AT, "GPRSStatus : CONNECTED");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_ESTABLISHING)
	{
		inLogPrintf(AT, "GPRSStatus : ESTABLISHING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_CONNECTING)
	{
		inLogPrintf(AT, "GPRSStatus : CONNECTING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_SENDING)
	{
		inLogPrintf(AT, "GPRSStatus : SENDING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_RECEIVING)
	{
		inLogPrintf(AT, "GPRSStatus : RECEIVING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_DISCONNECTING)
	{
		inLogPrintf(AT, "GPRSStatus : DISCONNECTING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_ONHOOKING)
	{
		inLogPrintf(AT, "GPRSStatus : ONHOOKING");
	}
  
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_GetSocketStatus
Date&Time       :2018/3/2 上午 9:50
Describe        :
*/
int inGPRS_GetSocketStatus(unsigned int *uiStatus, unsigned char uszSocket)
{
	char		szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1] = {0};   /* debug message */
	unsigned short  usRetval = 0;
	
	if (uiStatus == NULL)
		return (VS_ERROR);
	
	usRetval = CTOS_TCP_GPRSSocketStatus(uszSocket, uiStatus);
	
	if (usRetval != d_OK)
	{
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "CTOS_TCP_GPRSSocketStatus Error : 0x%04x", usRetval);
                        inLogPrintf(AT, szErrorMsg);
                }
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_CheckSocketStatus
Date&Time       :2018/3/2 上午 10:04
Describe        :
*/
int inGPRS_CheckSocketStatus(unsigned char uszSocket)
{
	unsigned int	uiStatus = 0;

	/* Get the status of the GPRS */
	inGPRS_GetSocketStatus(&uiStatus, uszSocket);

	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_ESTABLISHED)
	{
		inLogPrintf(AT, "GPRSStatus : ESTABLISHED");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_CONNECTED)
	{
		inLogPrintf(AT, "GPRSStatus : CONNECTED");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_ESTABLISHING)
	{
		inLogPrintf(AT, "GPRSStatus : ESTABLISHING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_CONNECTING)
	{
		inLogPrintf(AT, "GPRSStatus : CONNECTING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_SENDING)
	{
		inLogPrintf(AT, "GPRSStatus : SENDING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_RECEIVING)
	{
		inLogPrintf(AT, "GPRSStatus : RECEIVING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_DISCONNECTING)
	{
		inLogPrintf(AT, "GPRSStatus : DISCONNECTING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_ONHOOKING)
	{
		inLogPrintf(AT, "GPRSStatus : ONHOOKING");
	}
  
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_NCCCTMS_Check
Date&Time       :2017/7/19 上午 11:33
Describe        :
*/
int inGPRS_NCCCTMS_Check(void)
{
       int		inRetVal = 0;
	int		inConnectMaxCnt = 1;		/* 連線最大重試次數(含第一次) */
	int		inConnectNowCnt = 0;		/* 連線目前重試次數 */
	char		szTMSIPPrimary[16 + 1] = {0};
	char		szTMSIPSecond[16 + 1] = {0};
	char		szTMSPortNoPrimary[6 + 1] = {0};
	char		szTMSPortNoSecond[6 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		szIFESMode[2 + 1] = {0};

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inGPRS_NCCCTMS_Check() START!");
        
	inLoadTMSIPDTRec(0);
	
	/* Set TMS IP */
        memset(szTMSIPPrimary,0x00,sizeof(szTMSIPPrimary));
        if (inGetTMS_IP_Primary(szTMSIPPrimary) == VS_ERROR)
        {
                /* inGetHostIPPrimary ERROR */
                /* debug */
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inGetTMSIPAddress() ERROR!!");

                return (VS_ERROR);
        }
        
	/* Set TMS Port */
        memset(szTMSPortNoPrimary,0x00,sizeof(szTMSPortNoPrimary));
        
        if (inGetTMS_PortNo_Primary(szTMSPortNoPrimary) == VS_ERROR)
        {
                /* Get HostPortNumber Primary ERROR */
                /* debug */
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inGetTMSPortNum() ERROR!!");
                
                return (VS_ERROR);
        }
        	
	/* 重置目前重試次數 */
	inConnectNowCnt = 0;
	/* 若連線失敗看是否要重試 */
	do 
	{
		inRetVal = inGPRS_Connect_Flow(szTMSIPPrimary, szTMSPortNoPrimary);
		inConnectNowCnt ++;
		
	}while (inRetVal != VS_SUCCESS && inConnectNowCnt < inConnectMaxCnt);

	if (inRetVal == VS_SUCCESS)
	{	
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inGPRS_NCCCTMS_Check() END!");
		
		return (VS_SUCCESS);
	}
	else
	{
		/* 斷線(TLS若失敗沒斷線，會沒清除資源，所以調整成失敗一律先斷線再重連) */
		inGPRS_END();
		
		/* 若第一授權主機連線不成功，改連第二授權主機 */
		/* IFES情況下才連第二組 */
		memset(szIFESMode, 0x00, sizeof(szIFESMode));
		inGetI_FES_Mode(szIFESMode);
		if (memcmp(szIFESMode, "Y", strlen("Y")) != 0)
		{
			return (VS_ERROR);
		}
	}
	
	/* 第一授權IP失敗 嘗試第二IP */
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "First IP Conenct Fail");
		inLogPrintf(AT, szDebugMsg);
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Try Second IP...");
		inLogPrintf(AT, szDebugMsg);
	}
	
	inDISP_Clear_Line(_LINE_8_6_, _LINE_8_6_);
	inDISP_ChineseFont("第二組IP", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
	/* 嘗試連線第二授權主機 */
	/* 更改TMSIP */
	/* Set TMS IP */
        memset(szTMSIPSecond,0x00,sizeof(szTMSIPSecond));
        if (inGetTMS_IP_Second(szTMSIPSecond) == VS_ERROR)
        {
                /* inGetTMSIPSecond ERROR */
                /* debug */
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inGetTMSIPSecond() ERROR!!");

                return (VS_ERROR);
        }
        
	/* Set TMS Port */
        memset(szTMSPortNoSecond, 0x00, sizeof(szTMSPortNoSecond));
        if (inGetTMS_PortNo_Second(szTMSPortNoSecond) == VS_ERROR)
        {
                /* Get TMSPortNumber Primary ERROR */
                /* debug */
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inGetTMSPortNoSecond() ERROR!!");
                
                return (VS_ERROR);
        }
	
	/* 重置目前重試次數 */
	inConnectNowCnt = 0;
	/* 若連線失敗看是否要重試 */
	do 
	{
		inRetVal = inGPRS_Connect_Flow(szTMSIPSecond, szTMSPortNoSecond);
		inConnectNowCnt ++;
	}while (inRetVal != VS_SUCCESS && inConnectNowCnt < inConnectMaxCnt);

	/* 第二授權主機連線成功 */
        if (inRetVal == VS_SUCCESS)
        {
                return (VS_SUCCESS);
        }
        else
	{
               /* 若第二授權主機連線不成功，直接斷線 */ 
	}
	
	/* 第二授權IP失敗 */
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Second IP Conenct Fail");
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* 到最後還失敗，斷線 */
	inGPRS_END();
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inGPRS_SetConfig() END!");
        
	/* 若連線成功，中間就會Return出去，若一直失敗，則最後一定回傳失敗 */
        return (VS_ERROR);
}

/*
Function        :inGPRS_IsConnected
Date&Time       :2018/3/2 下午 2:39
Describe        :
*/
int inGPRS_IsConnected(void)
{
	unsigned int	uiStatus = 0;
	unsigned char	uszFlag = VS_FALSE;

	/* Get the status of the GPRS */
	if (inGPRS_GetStatus(&uiStatus) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_ESTABLISHED)
	{
		inLogPrintf(AT, "GPRSStatus : ESTABLISHED");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_CONNECTED)
	{
		inLogPrintf(AT, "GPRSStatus : CONNECTED");
		uszFlag = VS_TRUE;
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_ESTABLISHING)
	{
		inLogPrintf(AT, "GPRSStatus : ESTABLISHING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_CONNECTING)
	{
		inLogPrintf(AT, "GPRSStatus : CONNECTING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_SENDING)
	{
		inLogPrintf(AT, "GPRSStatus : SENDING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_RECEIVING)
	{
		inLogPrintf(AT, "GPRSStatus : RECEIVING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_DISCONNECTING)
	{
		inLogPrintf(AT, "GPRSStatus : DISCONNECTING");
	}
	
	/* if GPRS is connected */
	if (uiStatus & TCP_GPRS_STATE_ONHOOKING)
	{
		inLogPrintf(AT, "GPRSStatus : ONHOOKING");
	}
  
	if (uszFlag == VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}

/*
Function        :
Date&Time       :
Describe        :
*/
int inGPRS_Test(void)
{
	inGPRS_Open((unsigned char*)"", (unsigned char*)"internet", (unsigned char*)"", (unsigned char*)"");
	inGPRS_CheckStatus();
	
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_Detect_Network_Status
Date&Time       :2023/4/20 上午 11:08
Describe        :
*/
int inGPRS_Detect_Network_Status(unsigned char* pbStatus)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;
	
	usRetVal = CTOS_TCP_GPRSDetectNetworkstatus(pbStatus);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_TCP_GPRSDetectNetworkstatus Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_TCP_GPRSDetectNetworkstatus() OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_Check_Network_Status
Date&Time       :2023/4/20 上午 11:08
Describe        :
*/
int inGPRS_Check_Network_Status(void)
{
	unsigned char	uszStatus[1 + 1] = {0};
	
	memset(uszStatus, 0x00, sizeof(uszStatus));
	inGPRS_Detect_Network_Status(uszStatus);
	
	if (uszStatus[0] == 1)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "GPRS available");
		}
		
		return (VS_SUCCESS);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "GPRS unavailable");
		}
		
		return (VS_ERROR);
	}
}

/*
Function        :inGPRS_Get_IP
Date&Time       :2023/9/4 下午 3:10
Describe        :
*/
int inGPRS_Get_IP(unsigned char* uszIP)
{
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_TCP_GPRSGetIP(uszIP);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CTOS_TCP_GPRSGetIP Err :0x%04X", usRetVal);
		}
		
		vdUtility_SYSFIN_LogMessage(AT, "CTOS_TCP_GPRSGetIP Err :0x%04X", usRetVal);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CTOS_TCP_GPRSGetIP(%lu.%lu.%lu.%lu) OK", (unsigned int)*(uszIP), (unsigned int)*(uszIP + 1), (unsigned int)*(uszIP + 2), (unsigned int)*(uszIP + 3));
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inGPRS_Ping_IP
Date&Time       :2023/9/5 下午 5:15
Describe        :
*/
int inGPRS_Ping_IP(char *szIP)
{
	unsigned short	usRetVal = VS_ERROR;
	
	usRetVal = CTOS_TCP_GPRSPing((unsigned char*)szIP, 5);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CTOS_TCP_GPRSPing Err :0x%04X", usRetVal);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CTOS_TCP_GPRSPing() OK");
		}
	}
	
	return (VS_SUCCESS);
}