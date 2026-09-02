#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/AllStruct.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DisTouch.h"
#include "../DISPLAY/DispMsg.h"
#include "../PRINT/Print.h"
#include "../EVENT/MenuMsg.h"
#include "../EVENT/Menu.h"
#include "../FUNCTION/Function.h"
#include "../FUNCTION/CFGT.h"
#include "../FUNCTION/HDT.h"
#include "../FUNCTION/CPT.h"
#include "../FUNCTION/EDC.h"
#include "../FUNCTION/ECR.h"
#include "../FUNCTION/TMSIPDT.h"
#include "../FUNCTION/Utility.h"
#include "../../ETicket/Ticket.h"
#include "../../NCCC/NCCCtmsCPT.h"
#include "../../NCCC/NCCCtmsFTP.h"
#include "Comm.h"
#include "Ethernet.h"
#include "Modem.h"
#include "WiFi.h"
#include "GPRS.h"
#include "UDP.h"

COMM_OBJECT     gsrobCommunication;
extern  int     ginDebug;  /* Debug使用 extern */
extern	char	gszTermVersionID[16 + 1];
extern	int	ginFindRunTime;

/* File Descripter */
int	ginECR_ServerFd = -1;	/* 被動接收資料用這個handle，目前只有MP200的ECR用到 */
int	ginECR_ResponseFd = -1;	/* 回覆ECR用Handle */
int	ginTrans_ClientFd = -1;	/* 主動送或主動接收使用(Ex:送電文使用) */
BYTE	gbECR_UDP_Buffer[_ECR_BUFF_SIZE_] = {0};
UINT	guiECR_UDP_BufferLen = 0;
/*
Function        :inCOMM_InitCommDevice
Date&Time       :
Describe        :Get Communication Mode 並將function pointer設定到對應通訊模式的fuction
*/
int inCOMM_InitCommDevice()
{
        char    szCommmode[1 + 1] = {0};
	char	szDialBackupEnable[2 + 1] = {0};

        /* inCOMM_InitCommDevice() START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inCOMM_InitCommDevice() START！");
        }

	memset(szDialBackupEnable, 0x00, sizeof(szDialBackupEnable));
	inGetDialBackupEnable(szDialBackupEnable);
	if (memcmp(szDialBackupEnable, "Y", strlen("Y")) == 0)
	{
		inSetCommMode(_COMM_ETHERNET_MODE_);
		inSaveCFGTRec(0);
	}
	
        memset(szCommmode,0x00,sizeof(szCommmode));
        /* 取得通訊模式 */
        if (inGetCommMode(szCommmode) == VS_ERROR)
        {
		vdUtility_SYSFIN_LogMessage(AT, "通訊模式 取得失敗");
                return (VS_ERROR);
        }
        
        /* Modem Mode */
        if (memcmp(szCommmode, _COMM_MODEM_MODE_, 1) == 0)
        {
		vdUtility_SYSFIN_LogMessage(AT, "Comm Mode [MODEM]");/* [20260716 Add Log] */
		
		inCOMM_MODEM_SetFuncIndex();
		if (inCOMM_DoInitial() != VS_SUCCESS)
                {
			/* 聯合沒定義初始化錯誤的流程，只能用Log紀錄 */
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "MODEM Initial Error");
			}
			vdUtility_SYSFIN_LogMessage(AT, "MODEM Init Failed");
                        return (VS_ERROR);
                }
        }
        /* 乙太網路 Mode */
        else if (memcmp(szCommmode, _COMM_ETHERNET_MODE_, 1) == 0)
        {
			vdUtility_SYSFIN_LogMessage(AT, "Comm Mode [ETHERNET]");/* [20260716 Add Log] */
		
		inCOMM_ETHERNET_SetFuncIndex();
                if (inCOMM_DoInitial() != VS_SUCCESS)
                {
			/* 聯合沒定義初始化錯誤的流程，只能用Log紀錄 */
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "ETHERNET Initial Error");
			}
			vdUtility_SYSFIN_LogMessage(AT, "ETHERNET Init Failed");
                        return (VS_ERROR);
                }
        }
	else if (memcmp(szCommmode, _COMM_GPRS_MODE_, 1) == 0	||
		 memcmp(szCommmode, _COMM_3G_MODE_, 1) == 0	||
		 memcmp(szCommmode, _COMM_4G_MODE_, 1) == 0)
        {
			vdUtility_SYSFIN_LogMessage(AT, "Comm Mode [GPRS]");/* [20260716 Add Log] */

		inCOMM_GPRS_SetFuncIndex();
		if (inCOMM_DoInitial() != VS_SUCCESS)
                {
			/* 聯合沒定義初始化錯誤的流程，只能用Log紀錄 */
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "GPRS Initial Error");
			}
			vdUtility_SYSFIN_LogMessage(AT, "GPRS Init Failed");
                        return (VS_ERROR);
                }
		
        }
	else if (memcmp(szCommmode, _COMM_WIFI_MODE_, 1) == 0)
        {
			vdUtility_SYSFIN_LogMessage(AT, "Comm Mode [WiFi]");/* [20260716 Add Log] */
	
		inCOMM_WiFi_SetFuncIndex();
		if (inCOMM_DoInitial() != VS_SUCCESS)
                {
			/* 聯合沒定義初始化錯誤的流程，只能用Log紀錄 */
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "WiFi Initial Error");
			}
			vdUtility_SYSFIN_LogMessage(AT, "Wifi Init Failed");
                        return (VS_ERROR);
                }
        }

        /* inCOMM_InitCommDevice()_END */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inCOMM_InitCommDevice()_END");
        }
	
        return (VS_SUCCESS);
}

/*
Function        :inCOMM_ConnectStart
Date&Time       :
Describe        :執行Begin 和Check，若已連線（uszConnectionBit == VS_TRUE）則直接回傳成功
*/
int inCOMM_ConnectStart(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	int	inDHCPRetVal = VS_SUCCESS;
        char    szCommmode[1 + 1] = {0};
        char	szDebugMsg[100 + 1] = {0};
	char	szDialBackupEnable[2 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	char	szHostName[8 + 1] = {0};
	char	szDHCPMode[2 + 1] = {0};
	unsigned char	uszDialbackup = VS_FALSE;
	
#ifndef _COMMUNICATION_CAPBILITY_
	return (VS_SUCCESS);
#endif
	
        /* inCOMM_ConnectStart() START */
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inCOMM_ConnectStart() START!");
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("inCOMM_ConnectStart START");
	}
	
	memset(szHostName, 0x00, sizeof(szHostName));
	inGetHostLabel(szHostName);
	
	/* 表示已連線，不用重新連線 */
        if (pobTran->uszConnectionBit == VS_TRUE)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "pobTran->uszConnectionBit == VS_TRUE");
        
                return (VS_SUCCESS);
        }
	
	/* 連線中‧‧‧‧‧ */
	if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0)
	{
		/* 不顯示訊息 */
	}
	else if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
	{
		/* 不顯示訊息 */
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		inDISP_PutGraphic(_CONNECTING_, 0, _COORDINATE_Y_LINE_8_7_);
	}
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		return (VS_SUCCESS);
	}
	else
	{
		memset(szCommmode, 0x00, sizeof(szCommmode));
		/* 取得通訊模式 */
		if (inGetCommMode(szCommmode) == VS_ERROR)
		{
			if (ginDebug == VS_TRUE)
				inLogPrintf(AT, "inGetCommMode(szCommmode) == VS_ERROR");

			return (VS_ERROR);
		}
		
		memset(szDHCPMode, 0x00, sizeof(szDHCPMode));
		inGetDHCP_Mode(szDHCPMode);
		memset(szDialBackupEnable, 0x00, sizeof(szDialBackupEnable));
		inGetDialBackupEnable(szDialBackupEnable);

		/* Modem Mode */
		if (memcmp(szCommmode, _COMM_MODEM_MODE_, 1) == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "MODEM MODE");
			}

			/* 電子簽名上傳不能走撥接 */
			if (memcmp(szHostName, _HOST_NAME_ESC_, strlen(_HOST_NAME_ESC_)) == 0)
			{
				return (VS_ERROR);
			}
			/* 電子票證不能走撥接 */
			else if (memcmp(szHostName, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_)) == 0)
			{
				return (VS_ERROR);
			}
                        else if (!memcmp(szHostName, _HOST_NAME_TRUST_, strlen(_HOST_NAME_TRUST_)))
                        {
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_ChineseFont("此交易不支援撥接", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
                                inDISP_Wait(2000);
                        }
			
			if (gsrobCommunication.inBegin != NULL)
			{
				if (gsrobCommunication.inBegin(pobTran) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
						inLogPrintf(AT, "_COMM_MODEM_MODE_ .inBegin Error");

					return (VS_ERROR);
				}
			}
			else
			{
				return (VS_ERROR);
			}


			if (gsrobCommunication.inCheck != NULL)
			{
				if (gsrobCommunication.inCheck() != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
						inLogPrintf(AT, "_COMM_MODEM_MODE_ .inCheck Error");

					return (VS_ERROR);
				}
			}
			else
			{
				return (VS_ERROR);
			}
		}
		/* 乙太網路 Mode */
		else if (memcmp(szCommmode, _COMM_ETHERNET_MODE_, 1) == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "ETHERNET MODE");
			}

			if (gsrobCommunication.inBegin != NULL)
			{
				if (gsrobCommunication.inBegin(pobTran) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
						inLogPrintf(AT, "_COMM_ETHERNET_MODE_ .inBegin Error");

					return (VS_ERROR);
				}
			}
			else
			{
				return (VS_ERROR);
			}

			if (gsrobCommunication.inCheck != NULL)
			{
				if (gsrobCommunication.inCheck() != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
						inLogPrintf(AT, "_COMM_ETHERNET_MODE_ .inCheck Error");

					if (szDHCPMode[0] == 'Y')
					{
//						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
//						inDISP_ChineseFont("取得IP中", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
//						inDISP_ChineseFont("請稍後...", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
						inDHCPRetVal = inETHERNET_DHCP_Flow();
						if (inDHCPRetVal == VS_SUCCESS)
						{
//							inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
//							inDISP_ChineseFont("DHCP詢問成功", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
						}
						else
						{
//							inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
//							inDISP_ChineseFont("DHCP詢問失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
						}
					}
					
					/* 確認是否有撥接備援功能 */
					if (memcmp(szDialBackupEnable, "Y", strlen("Y")) == 0)
					{
						/* 電子簽名上傳不能走撥接 */
						if (memcmp(szHostName, _HOST_NAME_ESC_, strlen(_HOST_NAME_ESC_)) == 0)
						{
							
						}
						/* 電子票證不能走撥接 */
						else if (memcmp(szHostName, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_)) == 0)
						{
							
						}
                                                else if (!memcmp(szHostName, _HOST_NAME_TRUST_, strlen(_HOST_NAME_TRUST_)))
                                                {
                                                        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                                        inDISP_ChineseFont("此交易不支援撥接", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
                                                        inDISP_Wait(2000);
                                                }
						else
						{
							uszDialbackup = VS_TRUE;
						}
					}
					
					/* 只有一開始連線成功和撥接備援成功可以跳出，所以用do while來仿制go to用法 */
					do
					{
						inRetVal = gsrobCommunication.inCheck();	
						if (inRetVal == VS_SUCCESS)
						{
							break;
						}
						else
						{
							if (ginDebug == VS_TRUE)
							{
								inLogPrintf(AT, "_COMM_ETHERNET_MODE_ .inCheck Error");
							}

							if (uszDialbackup == VS_TRUE)
							{
								if (inCOMM_MODEM_DialBackUpOn(pobTran) == VS_SUCCESS)
								{
									inRetVal = VS_SUCCESS;
									break;
								}
								else
								{
									/* 回復Ethernet */
									inCOMM_MODEM_DialBackUpOff(pobTran);
								}
							}
						}
						
						return (VS_ERROR);
						break;
					}while (1);
				}
			}
			else
			{
				/* 未設定check function */
				return (VS_ERROR);
			}
		}
		else if (memcmp(szCommmode, _COMM_GPRS_MODE_, 1) == 0	||
			 memcmp(szCommmode, _COMM_3G_MODE_, 1) == 0	||
			 memcmp(szCommmode, _COMM_4G_MODE_, 1) == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "GPRS");
			}

			if (gsrobCommunication.inBegin != NULL)
			{
				if (gsrobCommunication.inBegin(pobTran) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
						inLogPrintf(AT, "_COMM_GPRS_MODE_ .inBegin Error");

					return (VS_ERROR);
				}
			}
			else
			{
				return (VS_ERROR);
			}

			if (gsrobCommunication.inCheck != NULL)
			{
				if (gsrobCommunication.inCheck() != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "_COMM_GPRS_MODE_ .inCheck Error");
					}

					return (VS_ERROR);
				}
			}
			else
			{
				return (VS_ERROR);
			}

		}
		else if (memcmp(szCommmode, _COMM_WIFI_MODE_, 1) == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "WiFi");
			}

			if (gsrobCommunication.inBegin != NULL)
			{
				if (gsrobCommunication.inBegin(pobTran) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
						inLogPrintf(AT, "_COMM_WIFI_MODE_ .inBegin Error");

					return (VS_ERROR);
				}
			}
			else
			{
				return (VS_ERROR);
			}

			if (gsrobCommunication.inCheck != NULL)
			{
				if (gsrobCommunication.inCheck() != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "_COMM_WIFI_MODE_ .inCheck Error");
					}

					return (VS_ERROR);
				}
			}
			else
			{
				return (VS_ERROR);
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "COMM_MODE : %s", szCommmode);
				inLogPrintf(AT, szDebugMsg);
			}

		}
		/* 表示已連線 */
		pobTran->uszConnectionBit = VS_TRUE;
	
		if (ginFindRunTime == VS_TRUE)
		{
			inFunc_RecordTime_Append("inCOMM_ConnectStart END");
		}
		
		/* inCOMM_ConnectStart() END! */
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inCOMM_ConnectStart() END!");

		return (VS_SUCCESS);
	}
}

/*
Function        :inCOMM_Send
Date&Time       :2016/10/4 下午 4:12
Describe        :
*/
int inCOMM_Send(unsigned char *uszSendBuff, int inSendSize, int inSendTimeout , unsigned char uszDispMsgBit)
{
	int	inRetVal;
	
        /* inCOMM_Send() START */
        if (ginDebug == VS_TRUE)
	{
                inLogPrintf(AT, "inCOMM_Send() START!");
	}

	if (uszDispMsgBit == VS_TRUE)
	{
		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_7_);
		inDISP_PutGraphic(_SEND_, 0, _COORDINATE_Y_LINE_8_7_);/* 傳送中... */
	}
		
        if (gsrobCommunication.inSend != NULL)
	{
		inRetVal = gsrobCommunication.inSend(uszSendBuff, inSendSize, inSendTimeout);
                if (inRetVal != VS_SUCCESS)
			return (inRetVal);
	}
        else
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, " gsrobCommunication.inSend == NULL !! ");
                return (VS_ERROR);
        }

        /* inCOMM_Send() END */
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inCOMM_Send() END!");

        return (VS_SUCCESS);
}

/*
Function        :inCOMM_Receive
Date&Time       :2016/10/4 下午 4:12
Describe        :
*/
int inCOMM_Receive(unsigned char *uszReceiveBuff, int inReceiveSize, int inReceiveTimeout, unsigned char uszDispMsgBit)
{
	int	inReceiveCnt;
	
        /* inCOMM_Receive() START */
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inCOMM_Receive() START!");

	if (uszDispMsgBit == VS_TRUE)
	{
		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		inDISP_PutGraphic(_RECEIVE_, 0, _COORDINATE_Y_LINE_8_7_);/* 接收中... */
	}
		
        if (gsrobCommunication.inReceive != NULL)
	{
                inReceiveCnt = gsrobCommunication.inReceive(uszReceiveBuff, inReceiveSize, inReceiveTimeout);
		/* 如果收到的資料數小於等於0(Timeout會回傳小於0)*/
		if (inReceiveCnt < 0)
			return (inReceiveCnt);
		else if (inReceiveCnt == 0)
			return (VS_ERROR);
	}
        else
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "gsrobCommunication.inReceive == NULL !!");
                return (VS_ERROR);
        }

        /* inCOMM_Receive() END */
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inCOMM_Receive() END!");

        return (VS_SUCCESS);
}

/*
Function        :inCOMM_End
Date&Time       :
Describe        :執行inEND完後，將uszConnectionBit設定成VS_ERROR
*/
int inCOMM_End(TRANSACTION_OBJECT *pobTran)
{
#ifndef _COMMUNICATION_CAPBILITY_
	return (VS_SUCCESS);
#endif
        /* inCOMM_Receive() START */
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inCOMM_End() START!");

        if (gsrobCommunication.inEnd != NULL)
                gsrobCommunication.inEnd();
        else
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, " gsrobCommunication.inEnd == NULL !! ");
                return (VS_ERROR);
        }

        /* 表示不是連線中的狀態 */
        pobTran->uszConnectionBit = VS_FALSE;

        /* inCOMM_End() END */
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inCOMM_End() END!");
	
        return (VS_SUCCESS);
}

/*
Function        :inCOMM_MODEM_Mode_Change
Date&Time       :2017/3/30 上午 9:34
Describe        :
*/
int inCOMM_MODEM_Mode_Change(TRANSACTION_OBJECT *pobTran)
{
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inCOMM_MODEM_Mode_Change() START !");
	}
	
	if (pobTran->uszDialBackup == VS_TRUE)
	{
		inCOMM_MODEM_DialBackUpOff(pobTran);
	}

	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inCOMM_MODEM_Mode_Change() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inMODEM_DialBackUpOn
Date&Time       :2017/3/29 下午 5:23
Describe        :
*/
int inCOMM_MODEM_DialBackUpOn(TRANSACTION_OBJECT *pobTran)
{
	char	szDebugMsg[100 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inCOMM_MODEM_DialBackUpOn() START !");
	}
	
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inCOMM_MODEM_DialBackUpOn() END !");
			inLogPrintf(AT, "----------------------------------------");
		}

		return (VS_SUCCESS);
	}
	else
	{
	
		/* 先預設沒撥接備援成功 */
		pobTran->uszDialBackup = VS_FALSE;


		/* 切換通訊模式為【撥接】 */
		if (inLoadCFGTRec(0) != VS_SUCCESS)
		{
			return (VS_ERROR);		/* 共用參數檔【Config.txt】 */
		}

		inSetCommMode(_COMM_MODEM_MODE_);

		if (inSaveCFGTRec(0) != VS_SUCCESS)
		{
			return (VS_ERROR);		/* 共用參數檔【Config.txt】 */
		}
		
		inCOMM_MODEM_SetFuncIndex();
		if (inCOMM_DoInitial() != VS_SUCCESS)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_INIT_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, "MODEM");
			srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);

			return (VS_ERROR);
		}

		pobTran->uszDialBackup = VS_TRUE;

		if (gsrobCommunication.inBegin(pobTran) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "DialBackup Begin Fail");
				inLogPrintf(AT, szDebugMsg);
			}
			return (VS_COMM_ERROR);
		}

		if (gsrobCommunication.inCheck() != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "DialBackup Check Fail");
				inLogPrintf(AT, szDebugMsg);
			}
			return (VS_COMM_ERROR);
		}

		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inCOMM_MODEM_DialBackUpOn() END !");
			inLogPrintf(AT, "----------------------------------------");
		}

		return (VS_SUCCESS);
	}
}

/*
Function        :inCOMM_MODEM_DialBackUpOff
Date&Time       :2017/3/29 下午 6:02
Describe        :把撥接切回原模式
*/
int inCOMM_MODEM_DialBackUpOff(TRANSACTION_OBJECT *pobTran)
{
	char	szDialBackupEnable[2 + 1] = {0};
	char	szCommMode[2 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inCOMM_MODEM_DialBackUpOff() START !");
	}

	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inCOMM_MODEM_DialBackUpOff() END !");
			inLogPrintf(AT, "----------------------------------------");
		}

		return (VS_SUCCESS);
	}
	else
	{
		/* 如果有DialBackup */
		memset(szDialBackupEnable, 0x00, sizeof(szDialBackupEnable));
		inGetDialBackupEnable(szDialBackupEnable);
		memset(szCommMode, 0x00, sizeof(szCommMode));
		inGetCommMode(szCommMode);		

		if (memcmp(szDialBackupEnable, "Y", strlen("Y")) == 0 && (pobTran->uszDialBackup == VS_TRUE	||
									  memcmp(szCommMode, _COMM_MODEM_MODE_, strlen(_COMM_MODEM_MODE_)) == 0))
		{
			inCOMM_End(pobTran); /* 有可能沒有斷線，先斷線 */

			/* 切換通訊模式為【ETHERNET】 */
			if (inLoadCFGTRec(0) != VS_SUCCESS)
			{
				return (VS_ERROR);		/* 共用參數檔【Config.txt】 */
			}

			inSetCommMode(_COMM_ETHERNET_MODE_);

			if (inSaveCFGTRec(0) != VS_SUCCESS)
			{
				return (VS_ERROR);		/* 共用參數檔【Config.txt】 */
			}
			
			inCOMM_ETHERNET_SetFuncIndex();
			if (inCOMM_DoInitial() != VS_SUCCESS)
			{
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, _ERR_INIT_);
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
				srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
				strcpy(srDispMsgObj.szErrMsg1, "ETHERNET");
				srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
				srDispMsgObj.inBeepTimes = 1;
				srDispMsgObj.inBeepInterval = 0;
				inDISP_Msg_BMP(&srDispMsgObj);

				return (VS_ERROR);
			}

		}

		pobTran->uszDialBackup = VS_FALSE;

		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inCOMM_MODEM_DialBackUpOff() END !");
			inLogPrintf(AT, "----------------------------------------");
		}

		return (VS_SUCCESS);
	}
}

/*
Function        :inCOMM_CreditPredialDisconnect
Date&Time       :
Describe        :
*/
int inCOMM_CreditPredialDisconnect(TRANSACTION_OBJECT *pobTran)
{
        /* inCOMM_CreditPredialDisconnect() START */
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inCOMM_CreditPredialDisconnect() START!");

        /* inCOMM_CreditPredialDisconnect() END */
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inCOMM_CreditPredialDisconnect() END!");

        return (VS_SUCCESS);
}

/*
Function        :inCOMM_DoInitial
Date&Time       :2018/9/20 下午 4:53
Describe        :執行設定後的Initial Function
*/
int inCOMM_DoInitial(void)
{
        int     inRetVal = VS_SUCCESS;

        /* inCOMM_DoInitial() START */
        if (ginDebug == VS_TRUE)
	{
                inLogPrintf(AT, "inCOMM_DoInitial() START!");
	}

	if (gsrobCommunication.inInitialize != NULL)
	{
		inRetVal = gsrobCommunication.inInitialize();
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Initial Function 尚未設定");
		}
		
		return (VS_ERROR);
	}

        /* inCOMM_ETHERNET_SetFuncIndex() END */
        if (ginDebug == VS_TRUE)
	{
                inLogPrintf(AT, "inCOMM_DoInitial() END!");
	}

        return (inRetVal);
}

/*
Function        :inCOMM_ETHERNET_SetFuncIndex
Date&Time       :
Describe        :將function pointer設定成對應乙太網路模式的fuction
*/
int inCOMM_ETHERNET_SetFuncIndex(void)
{
        int     inRetVal = VS_SUCCESS;

        /* inCOMM_ETHERNET_SetFuncIndex() START */
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inCOMM_ETHERNET_SetFuncIndex() START!");

        gsrobCommunication.inInitialize = inETHERNET_Initial;
        gsrobCommunication.inBegin = inETHERNET_Begin;
        gsrobCommunication.inCheck = inETHERNET_SetConfig;
        gsrobCommunication.inSend = inETHERNET_Send;
        gsrobCommunication.inReceive = inETHERNET_Receive;
        gsrobCommunication.inEnd = inETHERNET_END;
        gsrobCommunication.inFlush = inETHERNET_Flush;
	gsrobCommunication.inDeinitialize = inETHERNET_DeInitial;

        /* inCOMM_ETHERNET_SetFuncIndex() END */
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inCOMM_ETHERNET_SetFuncIndex() END!");

        return (inRetVal);
}

/*
Function        :inCOMM_MODEM_SetFuncIndex
Date&Time       :2016/6/3 上午 11:29
Describe        :將function pointer設定成對應撥接模式的fuction
*/
int inCOMM_MODEM_SetFuncIndex(void)
{
	int     inRetVal = VS_SUCCESS;

        /* inCOMM_MODEM_SetFuncIndex() START */
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inCOMM_MODEM_SetFuncIndex() START!");

        gsrobCommunication.inInitialize = inModem_Initial;
        gsrobCommunication.inBegin = inModem_Begin;
        gsrobCommunication.inCheck = inModem_Connect;
        gsrobCommunication.inSend = inModem_Send;
        gsrobCommunication.inReceive = inModem_Receive;
        gsrobCommunication.inEnd = inModem_END;
        gsrobCommunication.inFlush = inModem_Flush;
	gsrobCommunication.inDeinitialize = inModem_DeInitial;

        /* inCOMM_MODEM_SetFuncIndex() END */
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inCOMM_MODEM_SetFuncIndex() END!");

        return (inRetVal);
}

/*
Function        :inCOMM_WiFi_SetFuncIndex
Date&Time       :2017/7/21 下午 2:44
Describe        :將function pointer設定成對應WiFi模式的fuction
*/
int inCOMM_WiFi_SetFuncIndex(void)
{
	int     inRetVal = VS_SUCCESS;

        /* inCOMM_WiFi_SetFuncIndex() START */
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inCOMM_WiFi_SetFuncIndex() START!");

        gsrobCommunication.inInitialize = inWiFi_Initial;
	gsrobCommunication.inBegin = inWiFi_Begin;
        gsrobCommunication.inCheck = inWiFi_SetConfig;
        gsrobCommunication.inSend = inETHERNET_Send;		/* Send和Receive可和Ethernet共用 */
        gsrobCommunication.inReceive = inETHERNET_Receive;
        gsrobCommunication.inEnd = inWiFi_END;
        gsrobCommunication.inFlush = inWiFi_Flush;
	gsrobCommunication.inDeinitialize = inWiFi_DeInitial;

        /* inCOMM_WiFi_SetFuncIndex() END */
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inCOMM_WiFi_SetFuncIndex() END!");

        return (inRetVal);
}

/*
Function        :inCOMM_GPRS_SetFuncIndex
Date&Time       :2018/3/2 上午 11:37
Describe        :將function pointer設定成對應GPRS模式的fuction
*/
int inCOMM_GPRS_SetFuncIndex(void)
{
	int     inRetVal = VS_SUCCESS;

        /* inCOMM_GPRS_SetFuncIndex() START */
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inCOMM_GPRS_SetFuncIndex() START!");

	gsrobCommunication.inInitialize = inGPRS_Initial;
	gsrobCommunication.inBegin = inGPRS_Begin;
	gsrobCommunication.inCheck = inGPRS_SetConfig;
	gsrobCommunication.inSend = inGPRS_Send;		/* Send和Receive可和Ethernet共用 */
	gsrobCommunication.inReceive = inGPRS_Receive;
	gsrobCommunication.inEnd = inGPRS_END;
	gsrobCommunication.inFlush = inGPRS_Flush;
	gsrobCommunication.inDeinitialize = inGPRS_DeInitial;

        /* inCOMM_GPRS_SetFuncIndex() END */
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inCOMM_GPRS_SetFuncIndex() END!");

        return (inRetVal);
}

/*
Function        :inCOMM_Fun3_SetCommWay
Date&Time       :2016/6/3 上午 11:29
Describe        :功能3設定通訊模式
*/
int inCOMM_Fun3_SetCommWay(void)
{
	int	inPage = 1;
	int	inRetVal = VS_SUCCESS;
	
	while (inPage != 0)
	{
		if (inPage == 1)
		{
			inRetVal = inCOMM_Fun3_SetCommWay_Page1();
			if (inRetVal == VS_NEXT_PAGE)
			{
				inPage = 2;
			}
			else
			{
				inPage = 0;
			}
		}
		else if (inPage == 2)
		{
			inRetVal = inCOMM_Fun3_SetCommWay_Page2();
			if (inRetVal == VS_PREVIOUS_PAGE)
			{
				inPage = 1;
			}
			else if (inRetVal == VS_NEXT_PAGE)
			{
				inPage = 3;
			}
			else
			{
				inPage = 0;
			}
		}
		else if (inPage == 3)
		{
			inRetVal = inCOMM_Fun3_SetCommWay_Page3();
			if (inRetVal == VS_PREVIOUS_PAGE)
			{
				inPage = 2;
			}
			else
			{
				inPage = 0;
			}
		}
	}
	
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
        
        return (inRetVal);
}

/*
Function        :inCOMM_Fun3_SetCommWay_Page1
Date&Time       :2018/3/8 下午 5:29
Describe        :
*/
int inCOMM_Fun3_SetCommWay_Page1(void)
{
	int		inRetVal = VS_SUCCESS;
	int		inSelect = 0;		/* 勾選目前的通訊模式 */
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char		szTMSOK[2 + 1] = {0};
	char		szCommMode[2 + 1] = {0};
	char		szIFESMode[2 + 1] = {0};
	char		szCFESMode[2 + 1] = {0};
	char		szDHCP_Mode[2 + 1] = {0};
	char		szDialBackupEnable[2 + 1] = {0};
	char		szNCCCFESMode[2 + 1] = {0};
	unsigned char	uszKey = 0x00;
	unsigned char	uszInitialBit = VS_FALSE;
	unsigned char	uszReturnMenuBit = VS_FALSE;
	unsigned char	uszDispMenuBit = VS_TRUE;
	unsigned char	uszSSLEnableBit = VS_FALSE;
	
	while (1)
	{
		uszInitialBit = VS_FALSE;
		uszReturnMenuBit = VS_FALSE;
		uszSSLEnableBit = VS_FALSE;
		if (uszDispMenuBit == VS_TRUE)
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
			uszDispMenuBit = VS_FALSE;
			
			memset(szTMSOK, 0x00, sizeof(szTMSOK));
			inGetTMSOK(szTMSOK);

			/* 確認現在連接模式 */
			inLoadCFGTRec(0);
			memset(szCommMode, 0x00, sizeof(szCommMode));
			inGetCommMode(szCommMode);

			/* 是否使用SSL */
			memset(szIFESMode, 0x00, sizeof(szIFESMode));
			inGetI_FES_Mode(szIFESMode);
			
			/* FES Mode */
			memset(szNCCCFESMode, 0x00, sizeof(szNCCCFESMode));
			inGetNCCCFESMode(szNCCCFESMode);
			
			/* 是否使用CFES */
			memset(szCFESMode, 0x00, sizeof(szCFESMode));
			inGetCloud_MFES(szCFESMode);
			
			if (memcmp(szIFESMode, "Y", strlen("Y")) == 0)
			{
				uszSSLEnableBit = VS_TRUE;
			}
			else if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
			{
				if (memcmp(szTMSOK, "Y", strlen("Y")) == 0)
				{
					if (memcmp(szNCCCFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0)
					{
						uszSSLEnableBit = VS_TRUE;
					}
				}
				else
				{	
					uszSSLEnableBit = VS_TRUE;
				}
			}

			/* 是否使用DHCP */
			memset(szDHCP_Mode, 0x00, sizeof(szDHCP_Mode));
			inGetDHCP_Mode(szDHCP_Mode);

			/* 選擇通訊模式 1. 撥接 2. TCP/IP */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_SET_COMM_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);

			/* Modem */
			if (memcmp(szCommMode, _COMM_MODEM_MODE_, 1) == 0)
			{
				inSelect = _COMM_SELECT_MENU_MODEM_;
				inDISP_ChineseFont_Point_Color("V", _FONTSIZE_32X22_, _LINE_32_22_, _COLOR_WHITE_, _COLOR_BUTTON_, 2);
			}
			else if (memcmp(szDHCP_Mode, "N", strlen("N")) == 0		&& 
				 memcmp(szCommMode, _COMM_ETHERNET_MODE_, 1) == 0	&& 
				 uszSSLEnableBit == VS_FALSE)
			{
				inSelect = _COMM_SELECT_MENU_ETHERNET_UCL_;
				inDISP_ChineseFont_Point_Color("V", _FONTSIZE_32X22_, _LINE_32_22_, _COLOR_WHITE_, _COLOR_BUTTON_, 9);
			}
			else if (memcmp(szDHCP_Mode, "N", strlen("N")) == 0		&& 
				 memcmp(szCommMode, _COMM_ETHERNET_MODE_, 1) == 0	&& 
				 uszSSLEnableBit == VS_TRUE)
			{
				inSelect = _COMM_SELECT_MENU_ETHERNET_SSL_;
				inDISP_ChineseFont_Point_Color("V", _FONTSIZE_32X22_, _LINE_32_22_, _COLOR_WHITE_, _COLOR_BUTTON_, 17);
			}
			/* Ping 只當功能用*/
			else if (0)
			{
				inSelect = _COMM_SELECT_MENU_ETHERNET_PING_;
			}
			else if (memcmp(szDHCP_Mode, "Y", strlen("Y")) == 0		&& 
				 memcmp(szCommMode, _COMM_ETHERNET_MODE_, 1) == 0	&& 
				 uszSSLEnableBit == VS_FALSE)
			{
				inSelect = _COMM_SELECT_MENU_ETHERNET_DHCP_UCL_;
				inDISP_ChineseFont_Point_Color("V", _FONTSIZE_32X22_, _LINE_32_30_, _COLOR_WHITE_, _COLOR_BUTTON_, 9);
			}
			else if (memcmp(szDHCP_Mode, "Y", strlen("Y")) == 0		&& 
				 memcmp(szCommMode, _COMM_ETHERNET_MODE_, 1) == 0	&& 
				 uszSSLEnableBit == VS_TRUE)
			{
				inSelect = _COMM_SELECT_MENU_ETHERNET_DHCP_SSL_;
				inDISP_ChineseFont_Point_Color("V", _FONTSIZE_32X22_, _LINE_32_30_, _COLOR_WHITE_, _COLOR_BUTTON_, 17);
			}
		}

		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		uszKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}

		if (uszKey == _KEY_ENTER_)
		{
			switch (inSelect)
			{
				case _COMM_SELECT_MENU_MODEM_:
					uszKey = _KEY_1_;
					break;
				case _COMM_SELECT_MENU_ETHERNET_UCL_ :
					uszKey = _KEY_2_;
					break;
				case _COMM_SELECT_MENU_ETHERNET_SSL_ :
					uszKey = _KEY_3_;
					break;
				case _COMM_SELECT_MENU_ETHERNET_PING_ :
					uszKey = _KEY_4_;
					break;
				case _COMM_SELECT_MENU_ETHERNET_DHCP_UCL_ :
					uszKey = _KEY_5_;
					break;
				case _COMM_SELECT_MENU_ETHERNET_DHCP_SSL_ :
					uszKey = _KEY_6_;
					break;
				default:
					break;
			}
		}

		if (uszKey == _KEY_1_			||
		    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
		{
			/* 原本不是MODEM MODE */
			if (inSelect != _COMM_SELECT_MENU_MODEM_)
			{
				memset(szDialBackupEnable, 0x00, sizeof(szDialBackupEnable));
				inGetDialBackupEnable(szDialBackupEnable);
				if (memcmp(szDialBackupEnable, "Y", strlen("Y")) == 0)
				{
					/* 無須設定 */
				}
				/* 如果下載完TMS，不能手動改成撥接 */
				else if (memcmp(szTMSOK, "Y", 1) == 0)
				{
					uszReturnMenuBit = VS_TRUE;
				}
				else
				{
					/* 設定成Modem Mode*/
					inSetCommMode(_COMM_MODEM_MODE_);
					inSaveCFGTRec(0);
					uszInitialBit = VS_TRUE;
				}
			}
			
			/* 取消設定，回設定畫面 */
			if (uszReturnMenuBit == VS_TRUE)
			{
				uszDispMenuBit = VS_TRUE;
				continue;
			}

			/* 設定PABX 數據機設定 及 授權電話 */
			if (inCOMM_Fun3_SetPhone() != VS_SUCCESS)
			{

			}
			else
			{
				uszInitialBit = VS_TRUE;
			}

			/* 設定不論成功與否一律跳回Idle */
			break;
		}
		/* Ethernet TCP/IP(UCL) */
		else if (uszKey == _KEY_2_			||
			 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
		{
			/* 原本不是ETHERNET MODE */
			if (inSelect != _COMM_SELECT_MENU_ETHERNET_UCL_)
			{
				/* 設定成Ethernet Mode*/
				inSetCommMode(_COMM_ETHERNET_MODE_);
				inSaveCFGTRec(0);
				uszInitialBit = VS_TRUE;
			}
			
			if (memcmp(szIFESMode, "Y", strlen("Y")) == 0)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("關閉SSL Mode?", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
				inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
				uszKey = 0x00;
				while (1)
				{
					uszKey = uszKBD_GetKey(30);
					if (uszKey == _KEY_TIMEOUT_ ||
					    uszKey == _KEY_CANCEL_)
					{
						uszReturnMenuBit = VS_TRUE;
						break;
					}
					else if (uszKey == _KEY_0_)
					{
						inSetI_FES_Mode("N");
						inSaveCFGTRec(0);
						uszInitialBit = VS_TRUE;
						break;
					}
				}
				
				/* 取消設定，回設定畫面 */
				if (uszReturnMenuBit == VS_TRUE)
				{
					uszDispMenuBit = VS_TRUE;
					continue;
				}
			}
			
			if (memcmp(szDHCP_Mode, "Y", strlen("Y")) == 0)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("修改為固定IP??", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
				inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
				uszKey = 0x00;
				while (1)
				{
					uszKey = uszKBD_GetKey(30);
					if (uszKey == _KEY_TIMEOUT_ ||
					    uszKey == _KEY_CANCEL_)
					{
						uszReturnMenuBit = VS_TRUE;
						break;
					}
					else if (uszKey == _KEY_0_)
					{
						inSetDHCP_Mode("N");
						inSaveCFGTRec(0);
						uszInitialBit = VS_TRUE;
						break;
					}
				}
				
				/* 取消設定，回設定畫面 */
				if (uszReturnMenuBit == VS_TRUE)
				{
					uszDispMenuBit = VS_TRUE;
					continue;
				}
			}
			
			/* 設定IP */
			if (inCOMM_Fun3_Ethernet_SetIPAddress() != VS_SUCCESS)
			{

			}
			else
			{
				uszInitialBit = VS_TRUE;
			}

			/* 設定不論成功與否一律跳回Idle */
			break;
		}
		/* Ethernet TCP/IP(SSL) */
		else if (uszKey == _KEY_3_			||
			 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_)
		{
			if (inSelect != _COMM_SELECT_MENU_ETHERNET_SSL_)
			{
				/* 設定成Ethernet Mode*/
				inSetCommMode(_COMM_ETHERNET_MODE_);
				inSaveCFGTRec(0);
				uszInitialBit = VS_TRUE;
			}
			
			if (memcmp(szIFESMode, "N", strlen("N")) == 0)
			{
				if ((memcmp(szCFESMode, "Y", strlen("Y")) == 0		&& 
				     memcmp(szNCCCFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0))
				{
					/* 避免CFES誤開IFES */
				}
				else
				{
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("開啟SSL Mode?", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
					inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
					uszKey = 0x00;
					while (1)
					{
						uszKey = uszKBD_GetKey(30);
						if (uszKey == _KEY_TIMEOUT_ ||
						    uszKey == _KEY_CANCEL_)
						{
							uszReturnMenuBit = VS_TRUE;
							break;
						}
						else if (uszKey == _KEY_0_)
						{
							inSetI_FES_Mode("Y");
							inSaveCFGTRec(0);
							uszInitialBit = VS_TRUE;
							break;
						}
					}

					/* 取消設定，回設定畫面 */
					if (uszReturnMenuBit == VS_TRUE)
					{
						uszDispMenuBit = VS_TRUE;
						continue;
					}
				}
			}
			
			if (memcmp(szDHCP_Mode, "Y", strlen("Y")) == 0)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("修改為固定IP??", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
				inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
				uszKey = 0x00;
				while (1)
				{
					uszKey = uszKBD_GetKey(30);
					if (uszKey == _KEY_TIMEOUT_ ||
					    uszKey == _KEY_CANCEL_)
					{
						uszReturnMenuBit = VS_TRUE;
						break;
					}
					else if (uszKey == _KEY_0_)
					{
						inSetDHCP_Mode("N");
						inSaveCFGTRec(0);
						uszInitialBit = VS_TRUE;
						break;
					}
				}
				
				/* 取消設定，回設定畫面 */
				if (uszReturnMenuBit == VS_TRUE)
				{
					uszDispMenuBit = VS_TRUE;
					continue;
				}
			}
			
			/* 設定IP */
			if (inCOMM_Fun3_Ethernet_SetIPAddress() != VS_SUCCESS)
			{

			}
			else
			{
				uszInitialBit = VS_TRUE;
			}

			/* 設定不論成功與否一律跳回Idle */
			break;
		}
		/* Ethernet TCP/IP(PING) */
		else if (uszKey == _KEY_4_			||
			 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_7_)
		{
			inCOMM_Ping_Test_Menu(_PING_WAY_ETHERNET_);
			/* 設定不論成功與否一律跳回Idle */
			break;
		}
		/* DHCP */
		else if (uszKey == _KEY_5_			||
			 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_8_)
		{
			/* 原本不是ETHERNET MODE */
			if (inSelect != _COMM_SELECT_MENU_ETHERNET_DHCP_UCL_)
			{
				/* 設定成Ethernet Mode*/
				inSetCommMode(_COMM_ETHERNET_MODE_);
				inSaveCFGTRec(0);
				uszInitialBit = VS_TRUE;		
			}

			if (memcmp(szIFESMode, "Y", strlen("Y")) == 0)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("關閉SSL Mode?", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
				inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
				uszKey = 0x00;
				while (1)
				{
					uszKey = uszKBD_GetKey(30);
					if (uszKey == _KEY_TIMEOUT_ ||
					    uszKey == _KEY_CANCEL_)
					{
						uszReturnMenuBit = VS_TRUE;
						break;
					}
					else if (uszKey == _KEY_0_)
					{
						inSetI_FES_Mode("N");
						inSaveCFGTRec(0);
						uszInitialBit = VS_TRUE;
						break;
					}
				}
				
				/* 取消設定，回設定畫面 */
				if (uszReturnMenuBit == VS_TRUE)
				{
					uszDispMenuBit = VS_TRUE;
					continue;
				}
			}
			
			if (memcmp(szDHCP_Mode, "N", strlen("N")) == 0)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("修改為DHCP Mode?", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
				inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
				uszKey = 0x00;
				while (1)
				{
					uszKey = uszKBD_GetKey(30);
					if (uszKey == _KEY_TIMEOUT_ ||
					    uszKey == _KEY_CANCEL_)
					{
						uszReturnMenuBit = VS_TRUE;
						break;
					}
					else if (uszKey == _KEY_0_)
					{
						inSetDHCP_Mode("Y");
						inSaveCFGTRec(0);
						uszInitialBit = VS_TRUE;
						break;
					}
				}
				
				/* 取消設定，回設定畫面 */
				if (uszReturnMenuBit == VS_TRUE)
				{
					uszDispMenuBit = VS_TRUE;
					continue;
				}
			}
			
			/* 設定IP */
			if (inCOMM_Fun3_Ethernet_SetIPAddress() != VS_SUCCESS)
			{

			}
			else
			{
				uszInitialBit = VS_TRUE;
			}

			/* 設定不論成功與否一律跳回Idle */
			break;
		}
		/* DHCP(SSL) */
		else if (uszKey == _KEY_6_			||
			 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_9_)
		{
			/* 原本不是ETHERNET MODE */
			if (inSelect != _COMM_SELECT_MENU_ETHERNET_DHCP_SSL_)
			{
				/* 設定成Ethernet Mode*/
				inSetCommMode(_COMM_ETHERNET_MODE_);
				inSaveCFGTRec(0);
				uszInitialBit = VS_TRUE;
			}
			
			if (memcmp(szIFESMode, "N", strlen("N")) == 0)
			{
				if ((memcmp(szCFESMode, "Y", strlen("Y")) == 0		&& 
				     memcmp(szNCCCFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0))
				{
					/* 避免CFES誤開IFES */
				}
				else
				{
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("開啟SSL Mode?", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
					inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
					uszKey = 0x00;
					while (1)
					{
						uszKey = uszKBD_GetKey(30);
						if (uszKey == _KEY_TIMEOUT_ ||
						    uszKey == _KEY_CANCEL_)
						{
							uszReturnMenuBit = VS_TRUE;
							break;
						}
						else if (uszKey == _KEY_0_)
						{
							inSetI_FES_Mode("Y");
							inSaveCFGTRec(0);
							uszInitialBit = VS_TRUE;
							break;
						}
					}
				
					/* 取消設定，回設定畫面 */
					if (uszReturnMenuBit == VS_TRUE)
					{
						uszDispMenuBit = VS_TRUE;
						continue;
					}
				}
			}
			
			if (memcmp(szDHCP_Mode, "N", strlen("N")) == 0)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("修改為DHCP Mode?", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
				inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
				uszKey = 0x00;
				while (1)
				{
					uszKey = uszKBD_GetKey(30);
					if (uszKey == _KEY_TIMEOUT_ ||
					    uszKey == _KEY_CANCEL_)
					{
						uszReturnMenuBit = VS_TRUE;
						break;
					}
					else if (uszKey == _KEY_0_)
					{
						inSetDHCP_Mode("Y");
						inSaveCFGTRec(0);
						uszInitialBit = VS_TRUE;
						break;
					}
				}
				
				/* 取消設定，回設定畫面 */
				if (uszReturnMenuBit == VS_TRUE)
				{
					uszDispMenuBit = VS_TRUE;
					continue;
				}
			}
			
			/* 設定IP */
			if (inCOMM_Fun3_Ethernet_SetIPAddress() != VS_SUCCESS)
			{

			}
			else
			{
				uszInitialBit = VS_TRUE;
			}

			/* 設定不論成功與否一律跳回Idle */
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (uszKey == _KEY_DOWN_			||
			 inChoice == _DisTouch_Slide_Right_To_Left_)
		{
			inRetVal = VS_NEXT_PAGE;
			break;
		}
	}
	
	/* 是否要Initial */
	if (uszInitialBit == VS_TRUE)
	{
		/* DeInitial*/
		inCOMM_DeInitCommDevice();
		/* 重開module */
		inCOMM_InitCommDevice();
	}
	
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
		
	return (inRetVal);
}

/*
Function        :inCOMM_Fun3_SetCommWay_Page2
Date&Time       :2018/3/8 下午 5:29
Describe        :
*/
int inCOMM_Fun3_SetCommWay_Page2(void)
{
	int		inRetVal = VS_SUCCESS;
	int		inSelect = 0;		/* 勾選目前的通訊模式 */
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char		uszKey = 0x00;
	char		szTMSOK[2 + 1] = {0};
	char		szCommMode[2 + 1] = {0};
	char		szIFESMode[2 + 1] = {0};
	char		szCFESMode[2 + 1] = {0};
	char		szDHCP_Mode[2 + 1] = {0};
	char		szNCCCFESMode[2 + 1] = {0};
	unsigned char	uszInitialBit = VS_FALSE;
	unsigned char	uszReturnMenuBit = VS_FALSE;
	unsigned char	uszDispMenuBit = VS_TRUE;
	unsigned char	uszSSLEnableBit = VS_FALSE;

	while (1)
	{
		uszInitialBit = VS_FALSE;
		uszReturnMenuBit = VS_FALSE;
		uszSSLEnableBit = VS_FALSE;
		if (uszDispMenuBit == VS_TRUE)
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
			uszDispMenuBit = VS_FALSE;

			memset(szTMSOK, 0x00, sizeof(szTMSOK));
			inGetTMSOK(szTMSOK);

			/* 確認現在連接模式 */
			inLoadCFGTRec(0);
			memset(szCommMode, 0x00, sizeof(szCommMode));
			inGetCommMode(szCommMode);

			/* 是否使用SSL */
			memset(szIFESMode, 0x00, sizeof(szIFESMode));
			inGetI_FES_Mode(szIFESMode);

			/* FES Mode */
			memset(szNCCCFESMode, 0x00, sizeof(szNCCCFESMode));
			inGetNCCCFESMode(szNCCCFESMode);
			
			/* 是否使用CFES */
			memset(szCFESMode, 0x00, sizeof(szCFESMode));
			inGetCloud_MFES(szCFESMode);
			
			if (memcmp(szIFESMode, "Y", strlen("Y")) == 0		||
			   (memcmp(szCFESMode, "Y", strlen("Y")) == 0		&& 
			    memcmp(szNCCCFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0))
			{
				uszSSLEnableBit = VS_TRUE;
			}
			
			/* 是否使用DHCP */
			memset(szDHCP_Mode, 0x00, sizeof(szDHCP_Mode));
			inGetDHCP_Mode(szDHCP_Mode);

			/* 選擇通訊模式 1. GPRS 2. WIFI */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inMENU_Display_ICON(_ICON_TEMPLATE_BLUE_, _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("GPRS", _FONTSIZE_24X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_14_, VS_FALSE);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("     1", _FONTSIZE_24X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_15_, VS_FALSE);
		
			inMENU_Display_ICON(_ICON_TEMPLATE_BLUE_, _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("GPRS", _FONTSIZE_24X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_MENU_2_, _COORDINATE_Y_LINE_24_14_, VS_FALSE);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("(SSL)2", _FONTSIZE_24X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_MENU_2_, _COORDINATE_Y_LINE_24_15_, VS_FALSE);
		
			inMENU_Display_ICON(_ICON_TEMPLATE_BLUE_, _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("GPRS", _FONTSIZE_24X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_MENU_3_, _COORDINATE_Y_LINE_24_14_, VS_FALSE);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("PING 3", _FONTSIZE_24X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_MENU_3_, _COORDINATE_Y_LINE_24_15_, VS_FALSE);
		
			inMENU_Display_ICON(_ICON_TEMPLATE_BLUE_, _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_7_);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("WiFi", _FONTSIZE_24X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_20_, VS_FALSE);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("    4", _FONTSIZE_24X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_21_, VS_FALSE);
			
			inMENU_Display_ICON(_ICON_TEMPLATE_BLUE_, _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_8_);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("WiFi", _FONTSIZE_24X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_MENU_2_, _COORDINATE_Y_LINE_24_20_, VS_FALSE);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("(SSL)4", _FONTSIZE_24X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_MENU_2_, _COORDINATE_Y_LINE_24_21_, VS_FALSE);
			
			inMENU_Display_ICON(_ICON_TEMPLATE_BLUE_, _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_9_);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("WiFi", _FONTSIZE_24X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_MENU_3_, _COORDINATE_Y_LINE_24_20_, VS_FALSE);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("DHCP4", _FONTSIZE_24X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_MENU_3_, _COORDINATE_Y_LINE_24_21_, VS_FALSE);

			if ((memcmp(szCommMode, _COMM_GPRS_MODE_, 1) == 0	||
			     memcmp(szCommMode, _COMM_3G_MODE_, 1) == 0		||
			     memcmp(szCommMode, _COMM_4G_MODE_, 1) == 0)	&& 
			    uszSSLEnableBit != VS_TRUE)
			{
				inSelect = _COMM_SELECT_MENU_GPRS_UCL_;
				inDISP_ChineseFont_Point_Color("V", _FONTSIZE_32X22_, _LINE_32_21_, _COLOR_WHITE_, _COLOR_BUTTON_, 2);
			}
			else if ((memcmp(szCommMode, _COMM_GPRS_MODE_, 1) == 0	||
				  memcmp(szCommMode, _COMM_3G_MODE_, 1) == 0	||
				  memcmp(szCommMode, _COMM_4G_MODE_, 1) == 0)	&& 
				 uszSSLEnableBit == VS_TRUE)
			{
				inSelect = _COMM_SELECT_MENU_GPRS_SSL_;
				inDISP_ChineseFont_Point_Color("V", _FONTSIZE_32X22_, _LINE_32_21_, _COLOR_WHITE_, _COLOR_BUTTON_, 9);
			}
			else if ((memcmp(szDHCP_Mode, "N", strlen("N")) == 0	&& 
				  memcmp(szCommMode, _COMM_WIFI_MODE_, 1) == 0)	&& 
				  uszSSLEnableBit != VS_TRUE)
			{
				inSelect = _COMM_SELECT_MENU_WIFI_UCL_;
				inDISP_ChineseFont_Point_Color("V", _FONTSIZE_32X22_, _LINE_32_25_, _COLOR_WHITE_, _COLOR_BUTTON_, 2);
			}
			else if (memcmp(szDHCP_Mode, "N", strlen("N")) == 0	&& 
				 memcmp(szCommMode, _COMM_WIFI_MODE_, 1) == 0	&& 
				 uszSSLEnableBit == VS_TRUE)
			{
				inSelect = _COMM_SELECT_MENU_WIFI_SSL_;
				inDISP_ChineseFont_Point_Color("V", _FONTSIZE_32X22_, _LINE_32_25_, _COLOR_WHITE_, _COLOR_BUTTON_, 9);
			}
			else if ((memcmp(szDHCP_Mode, "Y", strlen("Y")) == 0	&& 
				  memcmp(szCommMode, _COMM_WIFI_MODE_, 1) == 0)	&& 
				  uszSSLEnableBit != VS_TRUE)
			{
				inSelect = _COMM_SELECT_MENU_WIFI_DHCP_UCL_;
				inDISP_ChineseFont_Point_Color("V", _FONTSIZE_32X22_, _LINE_32_25_, _COLOR_WHITE_, _COLOR_BUTTON_, 16);
			}
		}

		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		uszKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}

		if (uszKey == _KEY_ENTER_)
		{
			switch (inSelect)
			{
				case _COMM_SELECT_MENU_GPRS_UCL_:
					uszKey = _KEY_1_;
					break;
				case _COMM_SELECT_MENU_GPRS_SSL_:
					uszKey = _KEY_2_;
					break;
				case _COMM_SELECT_MENU_WIFI_UCL_:
					uszKey = _KEY_4_;
					break;
				case _COMM_SELECT_MENU_WIFI_SSL_:
					uszKey = _KEY_5_;
					break;
				case _COMM_SELECT_MENU_WIFI_DHCP_UCL_:
					uszKey = _KEY_6_;
					break;
				default:
					break;
			}
		}

		if (uszKey == _KEY_1_			||
		    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
		{
			/* 原本不是GPRS MODE */
			if (memcmp(szCommMode, _COMM_GPRS_MODE_, 1) != 0 &&
			    memcmp(szCommMode, _COMM_3G_MODE_, 1) != 0 &&
			    memcmp(szCommMode, _COMM_4G_MODE_, 1) != 0)
			{
				/* 設定成Ethernet Mode*/
				inSetCommMode(_COMM_GPRS_MODE_);
				inSaveCFGTRec(0);
				uszInitialBit = VS_TRUE;
			}
			
			if (memcmp(szIFESMode, "Y", strlen("Y")) == 0)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("關閉SSL Mode?", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
				inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
				uszKey = 0x00;
				while (1)
				{
					uszKey = uszKBD_GetKey(30);
					if (uszKey == _KEY_TIMEOUT_ ||
					    uszKey == _KEY_CANCEL_)
					{
						uszReturnMenuBit = VS_TRUE;
						break;
					}
					else if (uszKey == _KEY_0_)
					{
						inSetI_FES_Mode("N");
						inSaveCFGTRec(0);
						uszInitialBit = VS_TRUE;
						break;
					}
				}
				
				/* 取消設定，回設定畫面 */
				if (uszReturnMenuBit == VS_TRUE)
				{
					uszDispMenuBit = VS_TRUE;
					continue;
				}
			}

			/* 設定IP */
			if (inCOMM_Fun3_GPRS_SetIPAddress() != VS_SUCCESS)
			{

			}
			else
			{
				uszInitialBit = VS_TRUE;
			}

			/* 設定不論成功與否一律跳回Idle */
			break;
		}
		else if (uszKey == _KEY_2_			||
			 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
		{
			/* 原本不是GPRS MODE */
			if (memcmp(szCommMode, _COMM_GPRS_MODE_, 1) != 0 &&
			    memcmp(szCommMode, _COMM_3G_MODE_, 1) != 0 &&
			    memcmp(szCommMode, _COMM_4G_MODE_, 1) != 0)
			{
				/* 設定成Ethernet Mode*/
				inSetCommMode(_COMM_GPRS_MODE_);
				inSaveCFGTRec(0);
				uszInitialBit = VS_TRUE;
			}
			
			if (memcmp(szIFESMode, "N", strlen("N")) == 0)
			{
				if ((memcmp(szCFESMode, "Y", strlen("Y")) == 0		&& 
				     memcmp(szNCCCFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0))
				{
					/* 避免CFES誤開IFES */
				}
				else
				{
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("開啟SSL Mode?", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
					inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
					uszKey = 0x00;
					while (1)
					{
						uszKey = uszKBD_GetKey(30);
						if (uszKey == _KEY_TIMEOUT_ ||
						    uszKey == _KEY_CANCEL_)
						{
							uszReturnMenuBit = VS_TRUE;
							break;
						}
						else if (uszKey == _KEY_0_)
						{
							inSetI_FES_Mode("Y");
							inSaveCFGTRec(0);
							uszInitialBit = VS_TRUE;
							break;
						}
					}

					/* 取消設定，回設定畫面 */
					if (uszReturnMenuBit == VS_TRUE)
					{
						uszDispMenuBit = VS_TRUE;
						continue;
					}
				}
			}

			/* 設定IP */
			if (inCOMM_Fun3_GPRS_SetIPAddress() != VS_SUCCESS)
			{

			}
			else
			{
				uszInitialBit = VS_TRUE;
			}

			/* 設定不論成功與否一律跳回Idle */
			break;
		}
		else if (uszKey == _KEY_3_			||
			 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_)
		{
			inCOMM_Ping_Test_Menu(_PING_WAY_GPRS_);
			/* 設定不論成功與否一律跳回Idle */
			break;
		}
		else if (uszKey == _KEY_4_			||
			 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_7_)
		{
			if (inSelect != _COMM_SELECT_MENU_WIFI_UCL_)
			{
				/* 設定成Ethernet Mode*/
				inSetCommMode(_COMM_WIFI_MODE_);
				inSaveCFGTRec(0);
				uszInitialBit = VS_TRUE;
			}
			
			if (memcmp(szIFESMode, "Y", strlen("Y")) == 0)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("關閉SSL Mode?", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
				inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
				uszKey = 0x00;
				while (1)
				{
					uszKey = uszKBD_GetKey(30);
					if (uszKey == _KEY_TIMEOUT_ ||
					    uszKey == _KEY_CANCEL_)
					{
						uszReturnMenuBit = VS_TRUE;
						break;
					}
					else if (uszKey == _KEY_0_)
					{
						inSetI_FES_Mode("N");
						inSaveCFGTRec(0);
						uszInitialBit = VS_TRUE;
						break;
					}
				}
				
				/* 取消設定，回設定畫面 */
				if (uszReturnMenuBit == VS_TRUE)
				{
					uszDispMenuBit = VS_TRUE;
					continue;
				}
			}
			
			if (memcmp(szDHCP_Mode, "Y", strlen("Y")) == 0)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("修改為固定IP??", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
				inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
				uszKey = 0x00;
				while (1)
				{
					uszKey = uszKBD_GetKey(30);
					if (uszKey == _KEY_TIMEOUT_ ||
					    uszKey == _KEY_CANCEL_)
					{
						uszReturnMenuBit = VS_TRUE;
						break;
					}
					else if (uszKey == _KEY_0_)
					{
						inSetDHCP_Mode("N");
						inSaveCFGTRec(0);
						uszInitialBit = VS_TRUE;
						break;
					}
				}
				
				/* 取消設定，回設定畫面 */
				if (uszReturnMenuBit == VS_TRUE)
				{
					uszDispMenuBit = VS_TRUE;
					continue;
				}
			}

			inWiFi_Test_Menu();
				
			/* 設定IP */
			if (inCOMM_Fun3_Ethernet_SetIPAddress() != VS_SUCCESS)
			{

			}
			else
			{
				uszInitialBit = VS_TRUE;
			}

			/* 設定不論成功與否一律跳回Idle */
			break;
		}
		else if (uszKey == _KEY_5_			||
			 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_8_)
		{
			if (inSelect != _COMM_SELECT_MENU_WIFI_SSL_)
			{
				/* 設定成Ethernet Mode*/
				inSetCommMode(_COMM_WIFI_MODE_);
				inSaveCFGTRec(0);
				uszInitialBit = VS_TRUE;
			}
			
			if (memcmp(szIFESMode, "N", strlen("N")) == 0)
			{
				if ((memcmp(szCFESMode, "Y", strlen("Y")) == 0		&& 
				     memcmp(szNCCCFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0))
				{
					/* 避免CFES誤開IFES */
				}
				else
				{
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("開啟SSL Mode?", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
					inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
					uszKey = 0x00;
					while (1)
					{
						uszKey = uszKBD_GetKey(30);
						if (uszKey == _KEY_TIMEOUT_ ||
						    uszKey == _KEY_CANCEL_)
						{
							uszReturnMenuBit = VS_TRUE;
							break;
						}
						else if (uszKey == _KEY_0_)
						{
							inSetI_FES_Mode("Y");
							inSaveCFGTRec(0);
							uszInitialBit = VS_TRUE;
							break;
						}
					}

					/* 取消設定，回設定畫面 */
					if (uszReturnMenuBit == VS_TRUE)
					{
						uszDispMenuBit = VS_TRUE;
						continue;
					}
				}
			}
			
			if (memcmp(szDHCP_Mode, "Y", strlen("Y")) == 0)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("修改為固定IP??", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
				inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
				uszKey = 0x00;
				while (1)
				{
					uszKey = uszKBD_GetKey(30);
					if (uszKey == _KEY_TIMEOUT_ ||
					    uszKey == _KEY_CANCEL_)
					{
						uszReturnMenuBit = VS_TRUE;
						break;
					}
					else if (uszKey == _KEY_0_)
					{
						inSetDHCP_Mode("N");
						inSaveCFGTRec(0);
						uszInitialBit = VS_TRUE;
						break;
					}
				}
				
				/* 取消設定，回設定畫面 */
				if (uszReturnMenuBit == VS_TRUE)
				{
					uszDispMenuBit = VS_TRUE;
					continue;
				}
			}

			inWiFi_Test_Menu();
				
			/* 設定IP */
			if (inCOMM_Fun3_Ethernet_SetIPAddress() != VS_SUCCESS)
			{

			}
			else
			{
				uszInitialBit = VS_TRUE;
			}

			/* 設定不論成功與否一律跳回Idle */
			break;
		}
		else if (uszKey == _KEY_6_			||
			 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_9_)
		{
			if (inSelect != _COMM_SELECT_MENU_WIFI_DHCP_UCL_)
			{
				/* 設定成Ethernet Mode*/
				inSetCommMode(_COMM_WIFI_MODE_);
				inSaveCFGTRec(0);
				uszInitialBit = VS_TRUE;
			}
			
			if (memcmp(szIFESMode, "Y", strlen("Y")) == 0)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("關閉SSL Mode?", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
				inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
				uszKey = 0x00;
				while (1)
				{
					uszKey = uszKBD_GetKey(30);
					if (uszKey == _KEY_TIMEOUT_ ||
					    uszKey == _KEY_CANCEL_)
					{
						uszReturnMenuBit = VS_TRUE;
						break;
					}
					else if (uszKey == _KEY_0_)
					{
						inSetI_FES_Mode("N");
						inSaveCFGTRec(0);
						uszInitialBit = VS_TRUE;
						break;
					}
				}
				
				/* 取消設定，回設定畫面 */
				if (uszReturnMenuBit == VS_TRUE)
				{
					uszDispMenuBit = VS_TRUE;
					continue;
				}
			}

			if (memcmp(szDHCP_Mode, "N", strlen("N")) == 0)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("修改為DHCP Mode?", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
				inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
				uszKey = 0x00;
				while (1)
				{
					uszKey = uszKBD_GetKey(30);
					if (uszKey == _KEY_TIMEOUT_ ||
					    uszKey == _KEY_CANCEL_)
					{
						uszReturnMenuBit = VS_TRUE;
						break;
					}
					else if (uszKey == _KEY_0_)
					{
						inSetDHCP_Mode("Y");
						inSaveCFGTRec(0);
						uszInitialBit = VS_TRUE;
						break;
					}
				}
				
				/* 取消設定，回設定畫面 */
				if (uszReturnMenuBit == VS_TRUE)
				{
					uszDispMenuBit = VS_TRUE;
					continue;
				}
			}
			
			inWiFi_Test_Menu();
				
			/* 設定IP */
			if (inCOMM_Fun3_Ethernet_SetIPAddress() != VS_SUCCESS)
			{

			}
			else
			{
				uszInitialBit = VS_TRUE;
			}

			/* 設定不論成功與否一律跳回Idle */
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (uszKey == _KEY_UP_			||
			 inChoice == _DisTouch_Slide_Left_To_Right_)
		{
			inRetVal = VS_PREVIOUS_PAGE;
			break;
		}
		else if (uszKey == _KEY_DOWN_			||
			 inChoice == _DisTouch_Slide_Right_To_Left_)
		{
			inRetVal = VS_NEXT_PAGE;
			break;
		}
	}
	
	/* 是否要Initial */
	if (uszInitialBit == VS_TRUE)
	{
		/* DeInitial*/
		inCOMM_DeInitCommDevice();
		/* 重開module */
		inCOMM_InitCommDevice();
	}
	
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
		
	return (inRetVal);
}

/*
Function        :inCOMM_Fun3_SetCommWay_Page3
Date&Time       :2023/9/5 下午 6:50
Describe        :
*/
int inCOMM_Fun3_SetCommWay_Page3(void)
{
	int		inRetVal = VS_SUCCESS;
	int		inSelect = 0;		/* 勾選目前的通訊模式 */
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char		uszKey = 0x00;
	char		szTMSOK[2 + 1] = {0};
	char		szCommMode[2 + 1] = {0};
	char		szIFESMode[2 + 1] = {0};
	char		szCFESMode[2 + 1] = {0};
	char		szDHCP_Mode[2 + 1] = {0};
	char		szNCCCFESMode[2 + 1] = {0};
	unsigned char	uszInitialBit = VS_FALSE;
	unsigned char	uszReturnMenuBit = VS_FALSE;
	unsigned char	uszDispMenuBit = VS_TRUE;
	unsigned char	uszSSLEnableBit = VS_FALSE;

	while (1)
	{
		uszInitialBit = VS_FALSE;
		uszReturnMenuBit = VS_FALSE;
		uszSSLEnableBit = VS_FALSE;
		if (uszDispMenuBit == VS_TRUE)
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
			uszDispMenuBit = VS_FALSE;

			memset(szTMSOK, 0x00, sizeof(szTMSOK));
			inGetTMSOK(szTMSOK);

			/* 確認現在連接模式 */
			inLoadCFGTRec(0);
			memset(szCommMode, 0x00, sizeof(szCommMode));
			inGetCommMode(szCommMode);

			/* 是否使用SSL */
			memset(szIFESMode, 0x00, sizeof(szIFESMode));
			inGetI_FES_Mode(szIFESMode);

			/* FES Mode */
			memset(szNCCCFESMode, 0x00, sizeof(szNCCCFESMode));
			inGetNCCCFESMode(szNCCCFESMode);
			
			/* 是否使用CFES */
			memset(szCFESMode, 0x00, sizeof(szCFESMode));
			inGetCloud_MFES(szCFESMode);
			
			if (memcmp(szIFESMode, "Y", strlen("Y")) == 0		||
			   (memcmp(szCFESMode, "Y", strlen("Y")) == 0		&& 
			    memcmp(szNCCCFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0))
			{
				uszSSLEnableBit = VS_TRUE;
			}
			
			/* 是否使用DHCP */
			memset(szDHCP_Mode, 0x00, sizeof(szDHCP_Mode));
			inGetDHCP_Mode(szDHCP_Mode);

			/* 選擇通訊模式 1. GPRS 2. WIFI */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inMENU_Display_ICON(_ICON_TEMPLATE_BLUE_, _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("WiFi", _FONTSIZE_24X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_14_, VS_FALSE);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("DHCP 1", _FONTSIZE_24X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_15_, VS_FALSE);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("(SSL)", _FONTSIZE_24X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_16_, VS_FALSE);

			if (memcmp(szDHCP_Mode, "Y", strlen("Y")) == 0	&& 
			    memcmp(szCommMode, _COMM_WIFI_MODE_, 1) == 0 &&
			    uszSSLEnableBit == VS_TRUE)
			{
				inSelect = _COMM_SELECT_MENU_WIFI_DHCP_SSL_;
				inDISP_ChineseFont_Point_Color("V", _FONTSIZE_32X22_, _LINE_32_21_, _COLOR_WHITE_, _COLOR_BUTTON_, 2);
			}
		}

		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		uszKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}

		if (uszKey == _KEY_ENTER_)
		{
			switch (inSelect)
			{
				case _COMM_SELECT_MENU_WIFI_DHCP_SSL_:
					uszKey = _KEY_1_;
					break;
				default:
					break;
			}
		}

		if (uszKey == _KEY_1_			||
		    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
		{
			if (memcmp(szIFESMode, "N", strlen("N")) == 0)
			{
				if ((memcmp(szCFESMode, "Y", strlen("Y")) == 0		&& 
				     memcmp(szNCCCFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0))
				{
					/* 避免CFES誤開IFES */
				}
				else
				{
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("開啟SSL Mode?", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
					inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
					uszKey = 0x00;
					while (1)
					{
						uszKey = uszKBD_GetKey(30);
						if (uszKey == _KEY_TIMEOUT_ ||
						    uszKey == _KEY_CANCEL_)
						{
							uszReturnMenuBit = VS_TRUE;
							break;
						}
						else if (uszKey == _KEY_0_)
						{
							inSetI_FES_Mode("Y");
							inSaveCFGTRec(0);
							uszInitialBit = VS_TRUE;
							break;
						}
					}
				
					/* 取消設定，回設定畫面 */
					if (uszReturnMenuBit == VS_TRUE)
					{
						uszDispMenuBit = VS_TRUE;
						continue;
					}
				}
			}
			
			if (memcmp(szDHCP_Mode, "N", strlen("N")) == 0)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("修改為DHCP Mode?", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
				inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
				uszKey = 0x00;
				while (1)
				{
					uszKey = uszKBD_GetKey(30);
					if (uszKey == _KEY_TIMEOUT_ ||
					    uszKey == _KEY_CANCEL_)
					{
						uszReturnMenuBit = VS_TRUE;
						break;
					}
					else if (uszKey == _KEY_0_)
					{
						inSetDHCP_Mode("Y");
						inSaveCFGTRec(0);
						uszInitialBit = VS_TRUE;
						break;
					}
				}
				
				/* 取消設定，回設定畫面 */
				if (uszReturnMenuBit == VS_TRUE)
				{
					uszDispMenuBit = VS_TRUE;
					continue;
				}
			}
			
			inWiFi_Test_Menu();
				
			/* 設定IP */
			if (inCOMM_Fun3_Ethernet_SetIPAddress() != VS_SUCCESS)
			{

			}
			else
			{
				uszInitialBit = VS_TRUE;
			}

			/* 設定不論成功與否一律跳回Idle */
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (uszKey == _KEY_UP_			||
			 inChoice == _DisTouch_Slide_Left_To_Right_)
		{
			inRetVal = VS_PREVIOUS_PAGE;
			break;
		}
	}
	
	/* 是否要Initial */
	if (uszInitialBit == VS_TRUE)
	{
		/* DeInitial*/
		inCOMM_DeInitCommDevice();
		/* 重開module */
		inCOMM_InitCommDevice();
	}
	
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
		
	return (inRetVal);
}

/*
Function        :inCOMM_Fun3_SetPhone
Date&Time       :2016/6/3 上午 11:29
Describe        :功能3設定Modem Phone
*/
int inCOMM_Fun3_SetPhone(void)
{
	int		inRetVal = 0;
	int		i = 0;
	char		szDispMsg[16 + 1];
	unsigned char   uszKey;
	DISPLAY_OBJECT  srDispObj;
	
	/* Load EDC Record */
        inLoadEDCRec(0);
	
	/* 是否撥外線 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_CHECK_PABX_CODE_, 0, _COORDINATE_Y_LINE_8_4_);
	inDISP_BEEP(1, 0);

	while (1)
	{
		uszKey = uszKBD_GetKey(30);

		if (uszKey == _KEY_0_)
		{    
			/* 輸入PABX CODE */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_EDIT_PABX_CODE_, 0, _COORDINATE_Y_LINE_8_4_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			inGetPABXCode(szDispMsg);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

			while (1)
			{
				memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
				srDispObj.inMaxLen = 12;
				srDispObj.inY = _LINE_8_7_;
				srDispObj.inR_L = _DISP_RIGHT_;
				srDispObj.inColor = _COLOR_RED_;

				memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
				srDispObj.inOutputLen = 0;
	
				inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

				if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
					return (VS_ERROR);

				if (strlen(srDispObj.szOutput) >= 0)
				{
					/* 自動將,轉p，避免壞檔 by Russell 2021/6/3 下午 5:34 */
					for (i = 0; i < srDispObj.inOutputLen; i++)
					{
						if (srDispObj.szOutput[i] == ',')
						{
							srDispObj.szOutput[i] = 'p';
						}
					}
					
					inSetPABXCode(srDispObj.szOutput);
					inSaveEDCRec(0);
					break;
				}

				break;
			}
	
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			return (VS_ERROR);
		}
		else
		{
			continue;
		}
		
	}
	
	/* 是否修改數據機 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_CHECK_MODEM_, 0, _COORDINATE_Y_LINE_8_4_);
	inDISP_BEEP(1, 0);

	while (1)
	{
		uszKey = uszKBD_GetKey(30);

		if (uszKey == _KEY_0_)
		{            
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			return (VS_ERROR);
		}
		else
		{
			continue;
		}
	}
	
	/* 下面放各機型可調整數據機設定 */
	
	return (VS_SUCCESS);
}

/*
Function        :inCOMM_Fun3_SetPhoneIPAddress
Date&Time       :2016/6/3 上午 11:29
Describe        :功能3設定Ethernet IP
*/
int inCOMM_Fun3_Ethernet_SetIPAddress(void)
{
	int		inCPTIndex = -1;
	int		inRetVal = 0, inDot = 0, i = 0, j = 0;
        char		szDispMsg[16 + 1] = {0};
        char		szTemplate[16 + 1] = {0};
	char		szTemplate2[16 + 1] = {0};
	char		szCOMMIndex[2 + 1] = {0};
	char		szHostName[10 + 1] = {0};
	char		szIFESMode[2 + 1] = {0};
	char		szNCCCIP1[16 + 1] = {0};
	char		szNCCCPort1[5 + 1] = {0};
	char		szNCCCIP2[16 + 1] = {0};
	char		szNCCCPort2[5 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	char		szCloudFesMode[1 + 1] = {0};
	char		szNCCCFesMode[2 + 1] = {0};
	unsigned char   uszKey;
	unsigned char	uszChange = VS_FALSE;	/* 預設沒改變，若改變EDC IP相關，bit on起來並在結尾重新initial裝置 */
	unsigned char	uszCancel = VS_FALSE;	/* 跳出Ethernet設定迴圈 */
	unsigned char	uszDHCPBit = VS_FALSE;
	unsigned char	uszLen = 0;
	unsigned char	uszNCCCIP1ChangeBit = VS_FALSE;
	unsigned char	uszNCCCPort1ChangeBit = VS_FALSE;
	unsigned char	uszNCCCIP2ChangeBit = VS_FALSE;
	unsigned char	uszNCCCPort2ChangeBit = VS_FALSE;
	unsigned char	uszCus026ReqBit = VS_FALSE;
	DISPLAY_OBJECT  srDispObj;
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	memset(szNCCCFesMode, 0x00 , sizeof(szNCCCFesMode));
	inGetNCCCFESMode(szNCCCFesMode);
	
	memset(szCloudFesMode, 0x00 , sizeof(szCloudFesMode));
	inGetCloud_MFES(szCloudFesMode);
	
	/* IFES才能輸入第二組 */
	memset(szIFESMode, 0x00, sizeof(szIFESMode));
	inGetI_FES_Mode(szIFESMode);
	
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
	{
		if (!memcmp(szNCCCFesMode, _NCCC_03_MFES_MODE_, 2) &&
		    !memcmp(szCloudFesMode, "Y", 1))
		{
			uszCus026ReqBit = VS_TRUE;
		}
	}
	
	/* 看是否是DHCP模式 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetDHCP_Mode(szTemplate);
	if (memcmp(szTemplate, "Y", strlen("Y")) == 0)
	{
		uszDHCPBit = VS_TRUE;
	}
	
	/* 修改TCP IP請按0確認 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_CHECK_EDIT_TCPIP_, 0, _COORDINATE_Y_LINE_8_4_);
	inDISP_BEEP(1, 0);

	while (1)
	{
		uszKey = uszKBD_GetKey(30);

		if (uszKey == _KEY_0_)
		{            
			break;
		}
		else if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
		{
			return (VS_ERROR);
		}
		else
		{
			continue;
		}
	}
			
	/* Load EDC Record */
        inLoadEDCRec(0);
        
        /* Load TMS CPT Record */
        inLoadTMSIPDTRec(0);
        
	while(1)
	{
		/* 輸入EDC IP */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_EDIT_EDC_IP_, 0, _COORDINATE_Y_LINE_8_4_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		if (uszDHCPBit == VS_TRUE)
		{
			uszLen = sizeof(szDispMsg);
			inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_IP, (unsigned char*)szDispMsg, &uszLen);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
			
			inSetTermIPAddress(szDispMsg);
			inSaveEDCRec(0);
		}
		else
		{
			inGetTermIPAddress(szDispMsg);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
		}

		if (uszDHCPBit == VS_TRUE)
		{
			while (1)
			{
				uszKey = uszKBD_GetKey(30);
				if (uszKey == _KEY_ENTER_)
				{
					break;
				}
				else if (uszKey == _KEY_CANCEL_	||
					 uszKey == _KEY_TIMEOUT_)
				{
					uszCancel = VS_TRUE;
					break;
				}
				else
				{
					continue;
				}
			}
		}
		else
		{
			while (1)
			{
				memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
				srDispObj.inMaxLen = 15;
				srDispObj.inY = _LINE_8_7_;
				srDispObj.inR_L = _DISP_RIGHT_;
				srDispObj.inColor = _COLOR_RED_;

				memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
				srDispObj.inOutputLen = 0;

				inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

				if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				{
					uszCancel = VS_TRUE;
					break;
				}

				if (strlen(srDispObj.szOutput) > 0)
				{
					inDot = 0;

					for (j = 0 ;; j ++)
					{
						if (srDispObj.szOutput[j] == '.')
							inDot ++;
						else if (srDispObj.szOutput[j] == 0x00)
							break;
					}

					if (inDot == 3 && (strlen(srDispObj.szOutput) > 6))
					{
						inSetTermIPAddress(srDispObj.szOutput);
						inSaveEDCRec(0);
						/* 有做更改 */
						uszChange = VS_TRUE;
						break;
					}
					else
					{
						inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
						inDISP_EnglishFont("!!  IP ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
						inDISP_BEEP(2, 500);
						inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
						continue;
					}
				}
				else
				{
					break;
				}
			}
		}
		
		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}

		/* 輸入EDC SUB MASK */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_EDIT_EDC_SUB_MASK_, 0, _COORDINATE_Y_LINE_8_4_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		if (uszDHCPBit == VS_TRUE)
		{
			uszLen = sizeof(szDispMsg);
			inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_MASK, (unsigned char*)szDispMsg, &uszLen);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
			
			inSetTermMASKAddress(szDispMsg);
			inSaveEDCRec(0);
		}
		else
		{
			inGetTermMASKAddress(szDispMsg);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
		}

		if (uszDHCPBit == VS_TRUE)
		{
			while (1)
			{
				uszKey = uszKBD_GetKey(30);
				if (uszKey == _KEY_ENTER_)
				{
					break;
				}
				else if (uszKey == _KEY_CANCEL_	||
					 uszKey == _KEY_TIMEOUT_)
				{
					uszCancel = VS_TRUE;
					break;
				}
				else
				{
					continue;
				}
			}
		}
		else
		{
			while (1)
			{
				memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
				srDispObj.inMaxLen = 15;
				srDispObj.inY = _LINE_8_7_;
				srDispObj.inR_L = _DISP_RIGHT_;
				srDispObj.inColor = _COLOR_RED_;

				memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
				srDispObj.inOutputLen = 0;

				inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

				if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				{
					uszCancel = VS_TRUE;
					break;
				}


				if (strlen(srDispObj.szOutput) > 0)
				{
					inDot = 0;

					for (j = 0 ;; j ++)
					{
						if (srDispObj.szOutput[j] == '.')
							inDot ++;
						else if (srDispObj.szOutput[j] == 0x00)
							break;
					}

					if (inDot == 3 && (strlen(srDispObj.szOutput) > 6))
					{
						inSetTermMASKAddress(srDispObj.szOutput);
						inSaveEDCRec(0);
						/* 有做更改 */
						uszChange = VS_TRUE;
						break;
					}
					else
					{
						inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
						inDISP_EnglishFont("!!  IP ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
						inDISP_BEEP(2, 500);
						inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
						continue;
					}
				}
				else
				{
					break;
				}
			}
		}
		
		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}

		/* 輸入EDC Geteway */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_EDIT_EDC_GATEWAY_, 0, _COORDINATE_Y_LINE_8_4_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		if (uszDHCPBit == VS_TRUE)
		{
			uszLen = sizeof(szDispMsg);
			inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_GATEWAY, (unsigned char*)szDispMsg, &uszLen);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
			
			inSetTermGetewayAddress(szDispMsg);
			inSaveEDCRec(0);
		}
		else
		{
			inGetTermGetewayAddress(szDispMsg);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
		}

		if (uszDHCPBit == VS_TRUE)
		{
			while (1)
			{
				uszKey = uszKBD_GetKey(30);
				if (uszKey == _KEY_ENTER_)
				{
					break;
				}
				else if (uszKey == _KEY_CANCEL_	||
					 uszKey == _KEY_TIMEOUT_)
				{
					uszCancel = VS_TRUE;
					break;
				}
				else
				{
					continue;
				}
			}
		}
		else
		{
			while (1)
			{
				memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
				srDispObj.inMaxLen = 15;
				srDispObj.inY = _LINE_8_7_;
				srDispObj.inR_L = _DISP_RIGHT_;
				srDispObj.inColor = _COLOR_RED_;

				memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
				srDispObj.inOutputLen = 0;

				inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

				if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				{
					uszCancel = VS_TRUE;
					break;
				}


				if (strlen(srDispObj.szOutput) > 0)
				{
					inDot = 0;

					for (j = 0 ;; j ++)
					{
						if (srDispObj.szOutput[j] == '.')
							inDot ++;
						else if (srDispObj.szOutput[j] == 0x00)
							break;
					}

					if (inDot == 3 && (strlen(srDispObj.szOutput) > 6))
					{
						inSetTermGetewayAddress(srDispObj.szOutput);
						inSaveEDCRec(0);
						/* 有做更改 */
						uszChange = VS_TRUE;
						break;
					}
					else
					{
						inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
						inDISP_EnglishFont("!!  IP ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
						inDISP_BEEP(2, 500);
						inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
						continue;
					}
				}
				else
				{
					break;
				}
			}
		}
		
		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}

		/* 輸入TMS IP */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_EDIT_TMS_IP, 0, _COORDINATE_Y_LINE_8_4_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		inGetTMS_IP_Primary(szDispMsg);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

		while (1)
		{
			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			srDispObj.inMaxLen = 15;
			srDispObj.inY = _LINE_8_7_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inColor = _COLOR_RED_;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;
	
			inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			{
				uszCancel = VS_TRUE;
				break;
			}


			if (strlen(srDispObj.szOutput) > 0)
			{
				inDot = 0;

				for (j = 0 ;; j ++)
				{
					if (srDispObj.szOutput[j] == '.')
						inDot ++;
					else if (srDispObj.szOutput[j] == 0x00)
						break;
				}

				if (inDot == 3 && (strlen(srDispObj.szOutput) > 6))
				{
					inSetTMS_IP_Primary(srDispObj.szOutput);
					inSaveTMSIPDTRec(0);
					break;
				}
				else
				{
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					inDISP_EnglishFont("!!  IP ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
					inDISP_BEEP(2, 500);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					continue;
				}
			}

			break;
		}

		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}
		
		/* 輸入TMS PORT */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_EDIT_TMS_PORT_, 0, _COORDINATE_Y_LINE_8_4_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		inGetTMS_PortNo_Primary(szDispMsg);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

		while (1)
		{
			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			srDispObj.inMaxLen = 5;
			srDispObj.inY = _LINE_8_7_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inColor = _COLOR_RED_;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;
	
			inRetVal = inDISP_Enter8x16(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			{
				uszCancel = VS_TRUE;
				break;
			}


			if (strlen(srDispObj.szOutput) > 0)
			{
				inSetTMS_PortNo_Primary(srDispObj.szOutput);
				inSaveTMSIPDTRec(0);
				break;
			}
			
			break;
		}
		
		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}
		
		/* IFES 要兩組TMS IP*/
		if (memcmp(szIFESMode, "Y", strlen("Y")) == 0)
		{
			/* 輸入TMS IP2 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont_Color("TMS IP2?", _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_BLACK_, _DISP_CENTER_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			inGetTMS_IP_Second(szDispMsg);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

			while (1)
			{
				memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
				srDispObj.inMaxLen = 15;
				srDispObj.inY = _LINE_8_7_;
				srDispObj.inR_L = _DISP_RIGHT_;
				srDispObj.inColor = _COLOR_RED_;

				memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
				srDispObj.inOutputLen = 0;

				inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

				if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				{
					uszCancel = VS_TRUE;
					break;
				}


				if (strlen(srDispObj.szOutput) > 0)
				{
					inDot = 0;

					for (j = 0 ;; j ++)
					{
						if (srDispObj.szOutput[j] == '.')
							inDot ++;
						else if (srDispObj.szOutput[j] == 0x00)
							break;
					}

					if (inDot == 3 && (strlen(srDispObj.szOutput) > 6))
					{
						inSetTMS_IP_Second(srDispObj.szOutput);
						inSaveTMSIPDTRec(0);
						break;
					}
					else
					{
						inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
						inDISP_EnglishFont("!!  IP ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
						inDISP_BEEP(2, 500);
						inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
						continue;
					}
				}

				break;
			}

			/* 跳出最大的迴圈*/
			if (uszCancel == VS_TRUE)
			{
				break;
			}

			/* 輸入TMS PORT */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont_Color("TMS PORT Num2?", _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_BLACK_, _DISP_CENTER_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			inGetTMS_PortNo_Second(szDispMsg);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

			while (1)
			{
				memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
				srDispObj.inMaxLen = 5;
				srDispObj.inY = _LINE_8_7_;
				srDispObj.inR_L = _DISP_RIGHT_;
				srDispObj.inColor = _COLOR_RED_;

				memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
				srDispObj.inOutputLen = 0;

				inRetVal = inDISP_Enter8x16(&srDispObj);

				if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				{
					uszCancel = VS_TRUE;
					break;
				}


				if (strlen(srDispObj.szOutput) > 0)
				{
					inSetTMS_PortNo_Second(srDispObj.szOutput);
					inSaveTMSIPDTRec(0);
					break;
				}

				break;
			}

			/* 跳出最大的迴圈*/
			if (uszCancel == VS_TRUE)
			{
				break;
			}
			/* 跳出最大的迴圈*/
			if (uszCancel == VS_TRUE)
			{
				break;
			}
		}
		
		/* 輸入 FTP IP */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont(" FTP HOST IP?", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		inGetTMS_IP_Primary(szDispMsg);
		inFunc_PAD_ASCII(szDispMsg, szDispMsg, ' ', 16, _PADDING_LEFT_);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

		while (1)
		{
			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			srDispObj.inMaxLen = 15;
			srDispObj.inY = _LINE_8_6_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inColor = _COLOR_RED_;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;

			inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			{
				uszCancel = VS_TRUE;
				break;
			}


			if (strlen(srDispObj.szOutput) > 0)
			{
				inSetTMS_IP_Primary(srDispObj.szOutput);
				inSaveTMSIPDTRec(0);
			}

			break;
		}
                
                /* IFES 要兩組TMS IP*/
		if (memcmp(szIFESMode, "Y", strlen("Y")) == 0)
		{
			/* 輸入FTP IP2 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont_Color("FTP IP2?", _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_BLACK_, _DISP_CENTER_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			inGetTMS_IP_Second(szDispMsg);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

			while (1)
			{
				memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
				srDispObj.inMaxLen = 15;
				srDispObj.inY = _LINE_8_7_;
				srDispObj.inR_L = _DISP_RIGHT_;
				srDispObj.inColor = _COLOR_RED_;

				memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
				srDispObj.inOutputLen = 0;

				inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

				if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				{
					uszCancel = VS_TRUE;
					break;
				}


				if (strlen(srDispObj.szOutput) > 0)
				{
					inDot = 0;

					for (j = 0 ;; j ++)
					{
						if (srDispObj.szOutput[j] == '.')
							inDot ++;
						else if (srDispObj.szOutput[j] == 0x00)
							break;
					}

					if (inDot == 3 && (strlen(srDispObj.szOutput) > 6))
					{
						inSetTMS_IP_Second(srDispObj.szOutput);
						inSaveTMSIPDTRec(0);
						break;
					}
					else
					{
						inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
						inDISP_EnglishFont("!!  IP ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
						inDISP_BEEP(2, 500);
						inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
						continue;
					}
				}

				break;
			}
		}
		
		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}

		/* 輸入HOST IP & PORT */
		for (i = 0 ;; i ++)
		{
			/* 客製化026需求，CFES要TAKAIP和NCCC同步 START */
			if (uszNCCCIP1ChangeBit == VS_TRUE)
			{
				uszNCCCIP1ChangeBit = VS_FALSE;
				inLoadCPTRec(atoi(_CPT_TAKA_INDEX_) - 1);
				inSetHostIPPrimary(szNCCCIP1);
				inSaveCPTRec(atoi(_CPT_TAKA_INDEX_) - 1);
			}
			if (uszNCCCPort1ChangeBit == VS_TRUE)
			{
				uszNCCCPort1ChangeBit = VS_FALSE;
				inLoadCPTRec(atoi(_CPT_TAKA_INDEX_) - 1);
				inSetHostIPPrimary(szNCCCPort1);
				inSaveCPTRec(atoi(_CPT_TAKA_INDEX_) - 1);
			}
			if (uszNCCCIP2ChangeBit == VS_TRUE)
			{
				uszNCCCIP2ChangeBit = VS_FALSE;
				inLoadCPTRec(atoi(_CPT_TAKA_INDEX_) - 1);
				inSetHostIPSecond(szNCCCIP2);
				inSaveCPTRec(atoi(_CPT_TAKA_INDEX_) - 1);
			}
			if (uszNCCCPort2ChangeBit == VS_TRUE)
			{
				uszNCCCPort2ChangeBit = VS_FALSE;
				inLoadCPTRec(atoi(_CPT_TAKA_INDEX_) - 1);
				inSetHostIPPrimary(szNCCCPort2);
				inSaveCPTRec(atoi(_CPT_TAKA_INDEX_) - 1);
			}
			/* 客製化026需求，CFES要TAKAIP和NCCC同步 END */
			
			if (inLoadHDTRec(i) < 0)
				break;

			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetHostEnable(szTemplate);

			if (!memcmp(&szTemplate[0], "Y", 1))
			{
				memset(szCOMMIndex, 0x00, sizeof(szCOMMIndex));
				inGetCommunicationIndex(szCOMMIndex);
				inCPTIndex = atoi(szCOMMIndex) - 1;
				/* HDT對應到CPT */
				if (inLoadCPTRec(inCPTIndex) < 0)
					break;

				memset(szHostName, 0x00, sizeof(szHostName));
				inGetHostLabel(szHostName);
				inFunc_DiscardSpace(szHostName);
				
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				/* 輸入HOST IP */
				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				sprintf(szDispMsg, " %s HOST IP?", szHostName);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);

				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				inGetHostIPPrimary(szDispMsg);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

				while (1)
				{
					memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
					srDispObj.inMaxLen = 15;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;
	
					inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
					{
						uszCancel = VS_TRUE;
						break;
					}


					if (strlen(srDispObj.szOutput) > 0)
					{
						inDot = 0;

						for (j = 0 ;; j ++)
						{
							if (srDispObj.szOutput[j] == '.')
								inDot ++;
							else if (srDispObj.szOutput[j] == 0x00)
								break;
						}

						if (inDot == 3 && (strlen(srDispObj.szOutput) > 6))
						{
							inSetHostIPPrimary(srDispObj.szOutput);
							inSaveCPTRec(inCPTIndex);
							/* 客製化026*/
							if (uszCus026ReqBit == VS_TRUE)
							{
								if (!memcmp(szHostName, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_)))
								{
									uszNCCCIP1ChangeBit = VS_TRUE;
									sprintf(szNCCCIP1, srDispObj.szOutput);
								}
							}
							break;
						}
						else
						{
							inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
							inDISP_EnglishFont("!!  IP ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
							inDISP_BEEP(2, 500);
							inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
							continue;
						}
					}

					break;
				}
				
				/* 跳出最大的迴圈*/
				if (uszCancel == VS_TRUE)
				{
					break;
				}

				/* 輸入HOST Port */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				sprintf(szDispMsg, " %s PORT NUM?", szHostName);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);

				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetHostPortNoPrimary(szTemplate);
				
				memset(szTemplate2, 0x00, sizeof(szTemplate2));
				inGetTCPHeadFormat(szTemplate2);
				
				sprintf(szDispMsg, "%s (%s)", szTemplate, szTemplate2);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

				while (1)
				{
					memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
					srDispObj.inMaxLen = 15;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;
	
					inRetVal = inDISP_Enter8x16(&srDispObj);

					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
					{
						uszCancel = VS_TRUE;
						break;
					}

					if (strlen(srDispObj.szOutput) > 0)
					{
						inSetHostPortNoPrimary(srDispObj.szOutput);
						inSaveCPTRec(inCPTIndex);
						/* 客製化026*/
						if (uszCus026ReqBit == VS_TRUE)
						{
							if (!memcmp(szHostName, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_)))
							{
								uszNCCCPort1ChangeBit = VS_TRUE;
								sprintf(szNCCCPort1, srDispObj.szOutput);
							}
						}
						break;
					}
					
					break;
				}
				
				/* 跳出最大的迴圈*/
				if (uszCancel == VS_TRUE)
				{
					break;
				}
				
				/* IFES才能輸入第二組 */
				if (memcmp(szIFESMode, "Y", strlen("Y")) == 0)
				{
					/* 這裡輸入第二組IP */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					/* 輸入HOST IP */
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, " %s HOST IP2?", szHostName);
					inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);

					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					inGetHostIPSecond(szDispMsg);
					inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

					while (1)
					{
						memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
						srDispObj.inMaxLen = 15;
						srDispObj.inY = _LINE_8_7_;
						srDispObj.inR_L = _DISP_RIGHT_;
						srDispObj.inColor = _COLOR_RED_;

						memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
						srDispObj.inOutputLen = 0;

						inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

						if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						{
							uszCancel = VS_TRUE;
							break;
						}


						if (strlen(srDispObj.szOutput) > 0)
						{
							inDot = 0;

							for (j = 0 ;; j ++)
							{
								if (srDispObj.szOutput[j] == '.')
									inDot ++;
								else if (srDispObj.szOutput[j] == 0x00)
									break;
							}

							if (inDot == 3 && (strlen(srDispObj.szOutput) > 6))
							{
								inSetHostIPSecond(srDispObj.szOutput);
								inSaveCPTRec(inCPTIndex);
								/* 客製化026*/
								if (uszCus026ReqBit == VS_TRUE)
								{
									if (!memcmp(szHostName, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_)))
									{
										uszNCCCIP2ChangeBit = VS_TRUE;
										sprintf(szNCCCIP2, srDispObj.szOutput);
									}
								}
								break;
							}
							else
							{
								inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
								inDISP_EnglishFont("!!  IP ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
								inDISP_BEEP(2, 500);
								inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
								continue;
							}
						}

						break;
					}

					/* 跳出最大的迴圈*/
					if (uszCancel == VS_TRUE)
					{
						break;
					}

					/* 輸入HOST Port */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, " %s PORT NUM2?", szHostName);
					inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);

					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					memset(szTemplate, 0x00, sizeof(szTemplate));
					inGetHostPortNoSecond(szTemplate);

					memset(szTemplate2, 0x00, sizeof(szTemplate2));
					inGetTCPHeadFormat(szTemplate2);

					sprintf(szDispMsg, "%s (%s)", szTemplate, szTemplate2);
					inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

					while (1)
					{
						memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
						srDispObj.inMaxLen = 15;
						srDispObj.inY = _LINE_8_7_;
						srDispObj.inR_L = _DISP_RIGHT_;
						srDispObj.inColor = _COLOR_RED_;

						memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
						srDispObj.inOutputLen = 0;

						inRetVal = inDISP_Enter8x16(&srDispObj);

						if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						{
							uszCancel = VS_TRUE;
							break;
						}

						if (strlen(srDispObj.szOutput) > 0)
						{
							inSetHostPortNoSecond(srDispObj.szOutput);
							inSaveCPTRec(inCPTIndex);
							/* 客製化026*/
							if (uszCus026ReqBit == VS_TRUE)
							{
								if (!memcmp(szHostName, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_)))
								{
									uszNCCCPort2ChangeBit = VS_TRUE;
									sprintf(szNCCCPort2, srDispObj.szOutput);
								}
							}
							break;
						}
						break;
					}

					/* 跳出最大的迴圈*/
					if (uszCancel == VS_TRUE)
					{
						break;
					}
				}
			}
			else
			{
				continue;
			}

		}
		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}
		
	}
	
	/* 客製化026需求，CFES要TAKAIP和NCCC同步 */
	if (uszNCCCIP1ChangeBit == VS_TRUE)
	{
		uszNCCCIP1ChangeBit = VS_FALSE;
		inLoadCPTRec(atoi(_CPT_TAKA_INDEX_) - 1);
		inSetHostIPPrimary(szNCCCIP1);
		inSaveCPTRec(atoi(_CPT_TAKA_INDEX_) - 1);
	}
	if (uszNCCCPort1ChangeBit == VS_TRUE)
	{
		uszNCCCPort1ChangeBit = VS_FALSE;
		inLoadCPTRec(atoi(_CPT_TAKA_INDEX_) - 1);
		inSetHostIPPrimary(szNCCCPort1);
		inSaveCPTRec(atoi(_CPT_TAKA_INDEX_) - 1);
	}
	if (uszNCCCIP2ChangeBit == VS_TRUE)
	{
		uszNCCCIP2ChangeBit = VS_FALSE;
		inLoadCPTRec(atoi(_CPT_TAKA_INDEX_) - 1);
		inSetHostIPSecond(szNCCCIP2);
		inSaveCPTRec(atoi(_CPT_TAKA_INDEX_) - 1);
	}
	if (uszNCCCPort2ChangeBit == VS_TRUE)
	{
		uszNCCCPort2ChangeBit = VS_FALSE;
		inLoadCPTRec(atoi(_CPT_TAKA_INDEX_) - 1);
		inSetHostIPPrimary(szNCCCPort2);
		inSaveCPTRec(atoi(_CPT_TAKA_INDEX_) - 1);
	}
	
	/* 如果有更改EDC IP相關，重新initial裝置 重open */
	if (uszChange == VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
	
	
}

/*
Function        :inCOMM_Fun3_GPRS_SetIPAddress
Date&Time       :2018/3/2 下午 2:59
Describe        :功能3設定Ethernet IP
*/
int inCOMM_Fun3_GPRS_SetIPAddress(void)
{
	int		inCPTIndex = -1;
	int		inRetVal = 0, inDot = 0, i = 0, j = 0;
        char		szDispMsg[16 + 1] = {0};
        char		szTemplate[16 + 1] = {0};
	char		szTemplate2[16 + 1] = {0};
	char		szCOMMIndex[2 + 1] = {0};
	char		szHostName[10 + 1] = {0};
	char		szIFESMode[2 + 1] = {0};
	unsigned char   uszKey;
	unsigned char	uszChange = VS_FALSE;	/* 預設沒改變，若改變EDC IP相關，bit on起來並在結尾重新initial裝置 */
	unsigned char	uszCancel = VS_FALSE;	/* 跳出Ethernet設定迴圈 */
	unsigned char	uszTemplate[16 + 1] = {0};
	DISPLAY_OBJECT  srDispObj;

	/* 修改TCP IP請按0確認 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_CHECK_EDIT_TCPIP_, 0, _COORDINATE_Y_LINE_8_4_);
	inDISP_BEEP(1, 0);

	while (1)
	{
		uszKey = uszKBD_GetKey(30);

		if (uszKey == _KEY_0_)
		{            
			break;
		}
		else if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
		{
			return (VS_ERROR);
		}
		else
		{
			continue;
		}
	}
			
	/* Load EDC Record */
        inLoadEDCRec(0);
        
        /* Load TMS CPT Record */
        inLoadTMSIPDTRec(0);
        
	while(1)
	{
		/* 修改GPRS APN */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont_Color("修改APN", _FONTSIZE_8X16_, _LINE_8_4_, _COLOR_BLACK_, _DISP_LEFT_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		inGetGPRS_APN(szDispMsg);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

		while (1)
		{
			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			srDispObj.inMaxLen = 100;
			srDispObj.inY = _LINE_8_7_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inColor = _COLOR_RED_;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;

			inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			{
				uszCancel = VS_TRUE;
				break;
			}

			if (strlen(srDispObj.szOutput) > 0)
			{
				inSetGPRS_APN(srDispObj.szOutput);
				inSaveEDCRec(0);
				/* 有做更改 */
				uszChange = VS_TRUE;
				break;
			}
			
			break;
		}
		
		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}
	
		/* 輸入EDC IP */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_EDIT_EDC_IP_, 0, _COORDINATE_Y_LINE_8_4_);
		
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		memset(uszTemplate, 0x00, sizeof(uszTemplate));
		inGPRS_Get_IP(uszTemplate);
		sprintf(szDispMsg, "%u.%u.%u.%u", (unsigned int)uszTemplate[0], (unsigned int)uszTemplate[1], (unsigned int)uszTemplate[2], (unsigned int)uszTemplate[3]);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

		inSetTermIPAddress(szDispMsg);
		inSaveEDCRec(0);

		
		while (1)
		{
			uszKey = uszKBD_GetKey(30);
			if (uszKey == _KEY_ENTER_)
			{
				break;
			}
			else if (uszKey == _KEY_CANCEL_	||
				 uszKey == _KEY_TIMEOUT_)
			{
				uszCancel = VS_TRUE;
				break;
			}
			else
			{
				continue;
			}
		}
		
		
		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}
		
		/* 輸入TMS IP */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_EDIT_TMS_IP, 0, _COORDINATE_Y_LINE_8_4_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		inGetTMS_IP_Primary(szDispMsg);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

		while (1)
		{
			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			srDispObj.inMaxLen = 15;
			srDispObj.inY = _LINE_8_7_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inColor = _COLOR_RED_;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;
	
			inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			{
				uszCancel = VS_TRUE;
				break;
			}


			if (strlen(srDispObj.szOutput) > 0)
			{
				inDot = 0;

				for (j = 0 ;; j ++)
				{
					if (srDispObj.szOutput[j] == '.')
						inDot ++;
					else if (srDispObj.szOutput[j] == 0x00)
						break;
				}

				if (inDot == 3 && (strlen(srDispObj.szOutput) > 6))
				{
					inSetTMS_IP_Primary(srDispObj.szOutput);
					inSaveTMSIPDTRec(0);
					break;
				}
				else
				{
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					inDISP_EnglishFont("!!  IP ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
					inDISP_BEEP(2, 500);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					continue;
				}
			}

			break;
		}

		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}
		
		/* 輸入TMS PORT */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_EDIT_TMS_PORT_, 0, _COORDINATE_Y_LINE_8_4_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		inGetTMS_PortNo_Primary(szDispMsg);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

		while (1)
		{
			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			srDispObj.inMaxLen = 5;
			srDispObj.inY = _LINE_8_7_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inColor = _COLOR_RED_;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;
	
			inRetVal = inDISP_Enter8x16(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			{
				uszCancel = VS_TRUE;
				break;
			}


			if (strlen(srDispObj.szOutput) > 0)
			{
				inSetTMS_PortNo_Primary(srDispObj.szOutput);
				inSaveTMSIPDTRec(0);
			}
			
			break;
		}
		
		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}
		
		/* IFES 要兩組TMS IP*/
		if (memcmp(szIFESMode, "Y", strlen("Y")) == 0)
		{
			/* 輸入TMS IP2 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont_Color("TMS IP2?", _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_BLACK_, _DISP_CENTER_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			inGetTMS_IP_Second(szDispMsg);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

			while (1)
			{
				memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
				srDispObj.inMaxLen = 15;
				srDispObj.inY = _LINE_8_7_;
				srDispObj.inR_L = _DISP_RIGHT_;
				srDispObj.inColor = _COLOR_RED_;

				memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
				srDispObj.inOutputLen = 0;

				inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

				if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				{
					uszCancel = VS_TRUE;
					break;
				}


				if (strlen(srDispObj.szOutput) > 0)
				{
					inDot = 0;

					for (j = 0 ;; j ++)
					{
						if (srDispObj.szOutput[j] == '.')
							inDot ++;
						else if (srDispObj.szOutput[j] == 0x00)
							break;
					}

					if (inDot == 3 && (strlen(srDispObj.szOutput) > 6))
					{
						inSetTMS_IP_Second(srDispObj.szOutput);
						inSaveTMSIPDTRec(0);
						break;
					}
					else
					{
						inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
						inDISP_EnglishFont("!!  IP ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
						inDISP_BEEP(2, 500);
						inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
						continue;
					}
				}

				break;
			}

			/* 跳出最大的迴圈*/
			if (uszCancel == VS_TRUE)
			{
				break;
			}

			/* 輸入TMS PORT */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont_Color("TMS PORT Num2?", _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_BLACK_, _DISP_CENTER_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			inGetTMS_PortNo_Second(szDispMsg);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

			while (1)
			{
				memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
				srDispObj.inMaxLen = 5;
				srDispObj.inY = _LINE_8_7_;
				srDispObj.inR_L = _DISP_RIGHT_;
				srDispObj.inColor = _COLOR_RED_;

				memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
				srDispObj.inOutputLen = 0;

				inRetVal = inDISP_Enter8x16(&srDispObj);

				if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				{
					uszCancel = VS_TRUE;
					break;
				}


				if (strlen(srDispObj.szOutput) > 0)
				{
					inSetTMS_PortNo_Second(srDispObj.szOutput);
					inSaveTMSIPDTRec(0);
					break;
				}

				break;
			}

			/* 跳出最大的迴圈*/
			if (uszCancel == VS_TRUE)
			{
				break;
			}
		}
		
		/* 輸入 FTP IP */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTMSDownloadMode(szTemplate);

		if (memcmp(szTemplate, _TMS_DLMODE_FTPS_, strlen(_TMS_DLMODE_FTPS_)) == 0)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("FTP HOST IP?", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			inGetTMS_IP_Primary(szDispMsg);
			inFunc_PAD_ASCII(szDispMsg, szDispMsg, ' ', 16, _PADDING_LEFT_);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

			while (1)
			{
				memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
				srDispObj.inMaxLen = 15;
				srDispObj.inY = _LINE_8_6_;
				srDispObj.inR_L = _DISP_RIGHT_;
				srDispObj.inColor = _COLOR_RED_;

				memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
				srDispObj.inOutputLen = 0;

				inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

				if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				{
					uszCancel = VS_TRUE;
					break;
				}


				if (strlen(srDispObj.szOutput) > 0)
				{
					inSetTMS_IP_Primary(srDispObj.szOutput);
					inSaveTMSIPDTRec(0);
				}

				break;
			}
		}
		
		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}

		/* 輸入HOST IP & PORT */
		for (i = 0 ;; i ++)
		{
			if (inLoadHDTRec(i) < 0)
				break;

			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetHostEnable(szTemplate);

			if (!memcmp(&szTemplate[0], "Y", 1))
			{
				memset(szCOMMIndex, 0x00, sizeof(szCOMMIndex));
				inGetCommunicationIndex(szCOMMIndex);
				inCPTIndex = atoi(szCOMMIndex) - 1;
				/* HDT對應到CPT */
				if (inLoadCPTRec(inCPTIndex) < 0)
					break;

				memset(szHostName, 0x00, sizeof(szHostName));
				inGetHostLabel(szHostName);
				inFunc_DiscardSpace(szHostName);
				
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				/* 輸入HOST IP */
				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				sprintf(szDispMsg, " %s HOST IP?", szHostName);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);

				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				inGetHostIPPrimary(szDispMsg);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

				while (1)
				{
					memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
					srDispObj.inMaxLen = 15;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;
	
					inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
					{
						uszCancel = VS_TRUE;
						break;
					}


					if (strlen(srDispObj.szOutput) > 0)
					{
						inDot = 0;

						for (j = 0 ;; j ++)
						{
							if (srDispObj.szOutput[j] == '.')
								inDot ++;
							else if (srDispObj.szOutput[j] == 0x00)
								break;
						}

						if (inDot == 3 && (strlen(srDispObj.szOutput) > 6))
						{
							inSetHostIPPrimary(srDispObj.szOutput);
							inSaveCPTRec(inCPTIndex);
							break;
						}
						else
						{
							inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
							inDISP_EnglishFont("!!  IP ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
							inDISP_BEEP(2, 500);
							inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
							continue;
						}
					}

					break;
				}
				
				/* 跳出最大的迴圈*/
				if (uszCancel == VS_TRUE)
				{
					break;
				}

				/* 輸入HOST Port */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				sprintf(szDispMsg, " %s PORT NUM?", szHostName);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);

				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetHostPortNoPrimary(szTemplate);
				
				memset(szTemplate2, 0x00, sizeof(szTemplate2));
				inGetTCPHeadFormat(szTemplate2);
				
				sprintf(szDispMsg, "%s (%s)", szTemplate, szTemplate2);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

				while (1)
				{
					memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
					srDispObj.inMaxLen = 15;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;
	
					inRetVal = inDISP_Enter8x16(&srDispObj);

					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
					{
						uszCancel = VS_TRUE;
						break;
					}

					if (strlen(srDispObj.szOutput) > 0)
					{
						inSetHostPortNoPrimary(srDispObj.szOutput);
						inSaveCPTRec(inCPTIndex);
						break;
					}
					
					break;
				}
				
				/* 跳出最大的迴圈*/
				if (uszCancel == VS_TRUE)
				{
					break;
				}
				
				/* IFES才能輸入第二組 */
				memset(szIFESMode, 0x00, sizeof(szIFESMode));
				inGetI_FES_Mode(szIFESMode);
				if (memcmp(szIFESMode, "Y", strlen("Y")) == 0)
				{
					/* 這裡輸入第二組IP */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					/* 輸入HOST IP */
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, " %s HOST IP2?", szHostName);
					inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);

					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					inGetHostIPSecond(szDispMsg);
					inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

					while (1)
					{
						memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
						srDispObj.inMaxLen = 15;
						srDispObj.inY = _LINE_8_7_;
						srDispObj.inR_L = _DISP_RIGHT_;
						srDispObj.inColor = _COLOR_RED_;

						memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
						srDispObj.inOutputLen = 0;

						inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

						if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						{
							uszCancel = VS_TRUE;
							break;
						}


						if (strlen(srDispObj.szOutput) > 0)
						{
							inDot = 0;

							for (j = 0 ;; j ++)
							{
								if (srDispObj.szOutput[j] == '.')
									inDot ++;
								else if (srDispObj.szOutput[j] == 0x00)
									break;
							}

							if (inDot == 3 && (strlen(srDispObj.szOutput) > 6))
							{
								inSetHostIPSecond(srDispObj.szOutput);
								inSaveCPTRec(inCPTIndex);
								break;
							}
							else
							{
								inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
								inDISP_EnglishFont("!!  IP ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
								inDISP_BEEP(2, 500);
								inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
								continue;
							}
						}

						break;
					}

					/* 跳出最大的迴圈*/
					if (uszCancel == VS_TRUE)
					{
						break;
					}

					/* 輸入HOST Port */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, " %s PORT NUM2?", szHostName);
					inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);

					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					memset(szTemplate, 0x00, sizeof(szTemplate));
					inGetHostPortNoSecond(szTemplate);

					memset(szTemplate2, 0x00, sizeof(szTemplate2));
					inGetTCPHeadFormat(szTemplate2);

					sprintf(szDispMsg, "%s (%s)", szTemplate, szTemplate2);
					inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

					while (1)
					{
						memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
						srDispObj.inMaxLen = 15;
						srDispObj.inY = _LINE_8_7_;
						srDispObj.inR_L = _DISP_RIGHT_;
						srDispObj.inColor = _COLOR_RED_;

						memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
						srDispObj.inOutputLen = 0;

						inRetVal = inDISP_Enter8x16(&srDispObj);

						if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						{
							uszCancel = VS_TRUE;
							break;
						}

						if (strlen(srDispObj.szOutput) > 0)
						{
							inSetHostPortNoSecond(srDispObj.szOutput);
							inSaveCPTRec(inCPTIndex);
							break;
						}

						break;
					}

					/* 跳出最大的迴圈*/
					if (uszCancel == VS_TRUE)
					{
						break;
					}
				}
			}
			else
			{
				continue;
			}

		}
		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}
		
	}
	
	/* 如果有更改EDC IP相關，重新initial裝置 重open */
	if (uszChange == VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
	
	
}

/*
Function        :inCOMM_DeInitCommDevice
Date&Time       :2016/8/9 下午 4:05
Describe        :關閉通訊裝置來重新Initial
*/
int inCOMM_DeInitCommDevice()
{
	int	inRetVal = VS_SUCCESS;
        char    szCommmode[1 + 1];

        /* inCOMM_InitCommDevice() START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inCOMM_DeInitCommDevice() START！");
        }

        memset(szCommmode,0x00,sizeof(szCommmode));
        /* 取得通訊模式 */
        if (inGetCommMode(szCommmode) == VS_ERROR)
        {
                return (VS_ERROR);
        }
        
	if (gsrobCommunication.inDeinitialize != NULL)
	{
                inRetVal = gsrobCommunication.inDeinitialize();
	}
        else
        {
		inRetVal = VS_ERROR;
                /* debug */
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, " gsrobCommunication.inDeinitialize == NULL !! ");
        }
	
        if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "_COMM_ .inDeinitialize Error");

		return (VS_ERROR);
	}
	else
	{
		gsrobCommunication.inInitialize = NULL;
		gsrobCommunication.inBegin = NULL;
		gsrobCommunication.inCheck = NULL;
		gsrobCommunication.inSend = NULL;
		gsrobCommunication.inReceive = NULL;
		gsrobCommunication.inEnd = NULL;
		gsrobCommunication.inFlush = NULL;
		gsrobCommunication.inDeinitialize = NULL;
	}

        /* inCOMM_DeInitCommDevice()_END */
        if (ginDebug == VS_TRUE)
        {
		inLogPrintf(AT, "inCOMM_DeInitCommDevice()_END");
                inLogPrintf(AT, "----------------------------------------");
        }
	
        return (VS_SUCCESS);
}

/*
Function        :inCOMM_TCP_SetConnectTO
Date&Time       :2018/3/1 下午 2:40
Describe        :注意，這只能用在Modem和GPRS上，
 *		 The timeout value in millisecond
*/
int inCOMM_TCP_SetConnectTO(unsigned long ulTime)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = 0;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inCOMM_TCP_SetConnectTO() START !");
	}
	
	usRetVal = CTOS_TCP_SetConnectTO(ulTime);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CTOS_TCP_SetConnectTO OK!");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_TCP_SetConnectTO Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inCOMM_TCP_SetConnectTO() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inCOMM_TCP_SetRetryCounter
Date&Time       :2018/3/1 下午 2:40
Describe        :注意，這只能用在Modem和GPRS上，
 *		 The number of times in retry counter.
 *		 Please set this value to be larger than 2.(最小不可低於二)
*/
int inCOMM_TCP_SetRetryCounter(unsigned short usTime)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = 0;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inCOMM_TCP_SetRetryCounter() START !");
	}
	
	usRetVal = CTOS_TCP_SetRetryCounter(usTime);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CTOS_TCP_SetRetryCounter OK!");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_TCP_SetRetryCounter Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inCOMM_TCP_SetRetryCounter() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inCOMM_Ping_Test_Menu
Date&Time       :2018/11/2 上午 11:14
Describe        :
*/
int inCOMM_Ping_Test_Menu(int inConnectWay)
{
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_NEWUI_CHOOSE_HOST_;
	int		inRetVal = VS_SUCCESS;
	int		inKey = 0;
	char		szKey = 0;
	unsigned char	uszReDispBit = VS_FALSE;
	
	uszReDispBit = VS_TRUE;
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	while (1)
	{
		if (uszReDispBit == VS_TRUE)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_CHOOSE_HOST_2_, 0,_COORDINATE_Y_LINE_8_4_);

			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("選擇", _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_9_, VS_FALSE);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("HOST", _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_10_, VS_FALSE);

			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("輸入", _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_9_, VS_FALSE);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("IP", _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_10_, VS_FALSE);
		}
		uszReDispBit = VS_FALSE;
		
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		szKey = uszKBD_Key();

		/* 轉成數字判斷是否在inOpenHostCnt的範圍內 */
		inKey = 0;
		/* 有觸摸*/
		if (inChoice != _DisTouch_No_Event_)
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
			switch (inChoice)
			{
				case _NEWUI_CHOOSE_HOST_Touch_HOST_1_:
					inKey = 1;
					break;
				case _NEWUI_CHOOSE_HOST_Touch_HOST_2_:
					inKey = 2;
					break;
				default:
					inKey = 0;
					break;
			}
		}
		/* 有按按鍵 */
		else if (szKey != 0)
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
			switch (szKey)
			{
				case _KEY_1_:
					inKey = 1;
					break;
				case _KEY_2_:
					inKey = 2;
					break;
				default:
					inKey = 0;
					break;
			}
		}

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}

		if (szKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (inKey == 1)
		{
			inCOMM_Ping_Host_Menu(inConnectWay);
			uszReDispBit = VS_TRUE;
		}
		else if (inKey == 2)
		{
			inCOMM_Ping_IP_Menu(inConnectWay);
			uszReDispBit = VS_TRUE;
		}
	}

	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
		
	return (VS_SUCCESS);
}

/*
Function        :inCOMM_Ping_Host_Menu
Date&Time       :2018/11/2 下午 3:13
Describe        :
*/
int inCOMM_Ping_Host_Menu(int inConnectWay)
{
	int		inOpenHostCnt = 0;      /* 記錄有幾個Host有開 */
	int		inLine = 0;             /* 第幾行 */
	int		i = 0, j = 0;               /* j是inHostIndex陣列索引 */
	int		inHostIndex[12 + 1];    /* 記錄HostEnable為Y的HostIndex */
	int		inLine1Index = 0;       /* szLine1的index */
	int		inLine2Index = 0;       /* szLine2的index */
	int		inLine3Index = 0;       /* szLine3的index */
	int		inLine4Index = 0;
	int		inLine5Index = 0;
	int		inLine6Index = 0;
	int		inKey = 0;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_NEWUI_CHOOSE_HOST_;
	int		inRetVal = VS_SUCCESS;
	char		szKey = 0;
	char		szHostEnable[2 + 1] = {0};
	char		szHostName[42 + 1] = {0};
	char		szTemp[48 + 1] = {0};
	char		szLine1[48 + 1] = {0};		/* 存第一行要顯示的Host */	/* linux系統中文字length一個字為3，小心爆掉 */
	char		szLine2[48 + 1] = {0};		/* 存第二行要顯示的Host */
	char		szLine3[48 + 1] = {0};		/* 存第三行要顯示的Host */
	char		szLine4[48 + 1] = {0};
	char		szLine5[48 + 1] = {0};
	char		szLine6[48 + 1] = {0};
	char		szTemp2[48 + 1] = {0};
	char		szLine1_2[48 + 1] = {0};	/* 存第一行要顯示的Host */	/* linux系統中文字length一個字為3，小心爆掉 */
	char		szLine2_2[48 + 1] = {0};	/* 存第二行要顯示的Host */
	char		szLine3_2[48 + 1] = {0};	/* 存第三行要顯示的Host */
	char		szLine4_2[48 + 1] = {0};
	char		szLine5_2[48 + 1] = {0};
	char		szLine6_2[48 + 1] = {0};
	char		szTimeout[4 + 1] = {0};
	char		szDebugMsg[42 + 1] = {0};
	char		szTemplate[50 + 1] = {0};
	unsigned char	uszReDispBit = VS_FALSE;
	DISPLAY_OBJECT  srDispObj;
	
	memset(szLine1, 0x00, sizeof(szLine1));
        memset(szLine2, 0x00, sizeof(szLine2));
        memset(szLine3, 0x00, sizeof(szLine3));
        memset(szLine4, 0x00, sizeof(szLine4));
	memset(szLine5, 0x00, sizeof(szLine5));
	memset(szLine6, 0x00, sizeof(szLine6));
	memset(szLine1_2, 0x00, sizeof(szLine1_2));
        memset(szLine2_2, 0x00, sizeof(szLine2_2));
        memset(szLine3_2, 0x00, sizeof(szLine3_2));
        memset(szLine4_2, 0x00, sizeof(szLine4_2));
	memset(szLine5_2, 0x00, sizeof(szLine5_2));
	memset(szLine6_2, 0x00, sizeof(szLine6_2));
        memset(szTimeout, 0x00, sizeof(szTimeout));
        memset(inHostIndex, 0x00, sizeof(inHostIndex));
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	
	for (i = 0;; ++i)
        {
                /* 先LoadHDT */
                if (inLoadHDTRec(i) == VS_ERROR)
                {
                        /* 當找不到第i筆資料會回傳VS_ERROR */
                        break;
                }

                /* GET HOST Enable */
                memset(szHostEnable, 0x00, sizeof(szHostEnable));
                if (inGetHostEnable(szHostEnable) == VS_ERROR)
                {	
                        return (VS_ERROR);
                }
                else
                {
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%d HostEnable: %s", i, szHostEnable);
				inLogPrintf(AT, szDebugMsg);
			}
			
                        if (memcmp(szHostEnable, "Y", 1) != 0)
                        {
                                /* 如果HostEnable != Y，就continue */
                                continue;
                        }

                        inOpenHostCnt ++;       /* 記錄有幾個Host有開 */
                        inLine ++;              /* 第幾行 */

                        /* 記錄HostEnable為Y的HostIndex，減1是因為HostIndex從01開始 */
                        inHostIndex[j] = i;
                        j++;
			
			memset(szHostName, 0x00, sizeof(szHostName));
			inGetHostLabel(szHostName);
			
                        /* 用szTRTFileName來決定要顯示的Host Name */
                        memset(szTemp, 0x00, sizeof(szTemp));
			inFunc_DiscardSpace(szHostName);
			/* ESVC要顯示電子票證 */
			if (memcmp(szHostName, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_)) == 0)
			{
				sprintf(szTemp, "%s", "電子票");
				memset(szTemp2, 0x00, sizeof(szTemp2));
				sprintf(szTemp2, "證   %d", inOpenHostCnt);
			}
			else
			{
				sprintf(szTemp, "%s", szHostName);
				memset(szTemp2, 0x00, sizeof(szTemp2));
				sprintf(szTemp2, "     %d", inOpenHostCnt);
			}
			
                        /* 每一行顯示的內容先存在陣列裡 */
                        switch (inLine)
                        {
                                case 1:
                                        memcpy(&szLine1[inLine1Index], szTemp, strlen(szTemp));
                                        inLine1Index += strlen(szTemp);
					
					memcpy(szLine1_2, szTemp2, strlen(szTemp2));
                                        break;
                                case 2:
                                        memcpy(&szLine2[inLine2Index], szTemp, strlen(szTemp));
                                        inLine2Index += strlen(szTemp);
					
					memcpy(szLine2_2, szTemp2, strlen(szTemp2));
                                        break;
                                case 3:
                                        memcpy(&szLine3[inLine3Index], szTemp, strlen(szTemp));
                                        inLine3Index += strlen(szTemp);
					
					memcpy(szLine3_2, szTemp2, strlen(szTemp2));
                                        break;
                                case 4:
                                        memcpy(&szLine4[inLine4Index], szTemp, strlen(szTemp));
                                        inLine4Index += strlen(szTemp);
					
					memcpy(szLine4_2, szTemp2, strlen(szTemp2));
                                        break;
				case 5:
                                        memcpy(&szLine5[inLine5Index], szTemp, strlen(szTemp));
                                        inLine5Index += strlen(szTemp);
					
					memcpy(szLine5_2, szTemp2, strlen(szTemp2));
                                        break;
				case 6:
                                        memcpy(&szLine6[inLine6Index], szTemp, strlen(szTemp));
                                        inLine6Index += strlen(szTemp);
					
					memcpy(szLine6_2, szTemp2, strlen(szTemp2));
                                        break;
                                default:
                                        break;
                        }
                }
		
        }/* End of For loop */

	uszReDispBit = VS_TRUE;
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	while (1)
	{
		if (uszReDispBit == VS_TRUE)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

			switch (inOpenHostCnt)
			{
				case	1:
					inDISP_PutGraphic(_CHOOSE_HOST_2_, 0,_COORDINATE_Y_LINE_8_4_);
					break;
				case	2:
					inDISP_PutGraphic(_CHOOSE_HOST_2_, 0,_COORDINATE_Y_LINE_8_4_);
					break;

				case	3:
					inDISP_PutGraphic(_CHOOSE_HOST_3_, 0,_COORDINATE_Y_LINE_8_4_);
					break;

				case	4:
					inDISP_PutGraphic(_CHOOSE_HOST_4_, 0,_COORDINATE_Y_LINE_8_4_);
					break;

				case	5:
					inDISP_PutGraphic(_CHOOSE_HOST_5_, 0,_COORDINATE_Y_LINE_8_4_);
					break;

				case	6:
					inDISP_PutGraphic(_CHOOSE_HOST_6_, 0,_COORDINATE_Y_LINE_8_4_);
					break;

				default:
					inDISP_PutGraphic(_CHOOSE_HOST_6_, 0,_COORDINATE_Y_LINE_8_4_);
					break;
			}

			/*有開多個Host */
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine1, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_9_, VS_FALSE);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine1_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_10_, VS_FALSE);

			inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_9_, VS_FALSE);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine2_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_10_, VS_FALSE);

			inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine3, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_3_, _COORDINATE_Y_LINE_16_9_, VS_FALSE);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine3_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_3_, _COORDINATE_Y_LINE_16_10_, VS_FALSE);

			inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine4, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_13_, VS_FALSE);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine4_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_14_, VS_FALSE);

			inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine5, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_13_, VS_FALSE);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine5_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_14_, VS_FALSE);

			inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine6, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_3_, _COORDINATE_Y_LINE_16_13_, VS_FALSE);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine6_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_3_, _COORDINATE_Y_LINE_16_14_, VS_FALSE);

			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, szLine1);
				inLogPrintf(AT, szLine2);
				inLogPrintf(AT, szLine3);
				inLogPrintf(AT, szLine4);
				inLogPrintf(AT, szLine5);
				inLogPrintf(AT, szLine6);
			}
		}
		uszReDispBit = VS_FALSE;
		
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		szKey = uszKBD_Key();

		/* 轉成數字判斷是否在inOpenHostCnt的範圍內 */
		inKey = 0;
		/* 有觸摸*/
		if (inChoice != _DisTouch_No_Event_)
		{
			switch (inChoice)
			{
				case _NEWUI_CHOOSE_HOST_Touch_HOST_1_:
					inKey = 1;
					break;
				case _NEWUI_CHOOSE_HOST_Touch_HOST_2_:
					inKey = 2;
					break;
				case _NEWUI_CHOOSE_HOST_Touch_HOST_3_:
					inKey = 3;
					break;
				case _NEWUI_CHOOSE_HOST_Touch_HOST_4_:
					inKey = 4;
					break;
				case _NEWUI_CHOOSE_HOST_Touch_HOST_5_:
					inKey = 5;
					break;
				case _NEWUI_CHOOSE_HOST_Touch_HOST_6_:
					inKey = 6;
					break;
				default:
					inKey = 0;
					break;
			}
		}
		/* 有按按鍵 */
		else if (szKey != 0)
		{
			switch (szKey)
			{
				case _KEY_1_:
					inKey = 1;
					break;
				case _KEY_2_:
					inKey = 2;
					break;
				case _KEY_3_:
					inKey = 3;
					break;
				case _KEY_4_:
					inKey = 4;
					break;
				case _KEY_5_:
					inKey = 5;
					break;
				case _KEY_6_:
					inKey = 6;
					break;
				default:
					inKey = 0;
					break;
			}
		}

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}

		if (szKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (inKey >= 1 && inKey <= inOpenHostCnt)
		{
			if (inLoadHDTRec(inHostIndex[inKey - 1]) == VS_ERROR)
			{
				/* 主機選擇錯誤 */
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHOOSE_HOST_);
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
				srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
				strcpy(srDispMsgObj.szErrMsg1, "");
				srDispMsgObj.inErrMsg1Line = 0;
				srDispMsgObj.inBeepTimes = 1;
				srDispMsgObj.inBeepInterval = 0;
				inDISP_Msg_BMP(&srDispMsgObj);

				inRetVal = VS_ERROR;
				break;
			}
			
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetCommunicationIndex(szTemplate);
			inLoadCPTRec(atoi(szTemplate) - 1);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetHostIPPrimary(szTemplate);
			if (inConnectWay == _PING_WAY_ETHERNET_)
			{
				inRetVal = inETHERNET_Ping_IP(szTemplate);
			}
			else if (inConnectWay == _PING_WAY_ETHERNET_)
			{
				inRetVal = inGPRS_Ping_IP(szTemplate);
			}
			else
			{
				inRetVal = VS_ERROR;
			}
			
			if (inRetVal == VS_SUCCESS)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_EnglishFont("Ping Succuess", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
				inDISP_Wait(1000);
			}
			else
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_EnglishFont("Ping Failed", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
				inDISP_Wait(1000);
			}
			uszReDispBit = VS_TRUE;
		}
	}

	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
	
	return (VS_SUCCESS);
}

/*
Function        :inCOMM_Ping_IP_Menu
Date&Time       :2018/11/2 下午 1:13
Describe        :
*/
int inCOMM_Ping_IP_Menu(int inConnectWay /* = 0 */)
{
	int		inDot = 0, j =0;
	int		inRetVal = VS_SUCCESS;
	char		szDispMsg[50 + 1] = {0};
	DISPLAY_OBJECT  srDispObj;
	
	/* 請輸入Host IP */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont("請輸入Host IP", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	while (1)
	{
		memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
		srDispObj.inMaxLen = 15;
		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inColor = _COLOR_RED_;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);
		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		{
			break;
		}

		if (strlen(srDispObj.szOutput) > 0)
		{
			inDot = 0;

			for (j = 0 ;; j ++)
			{
				if (srDispObj.szOutput[j] == '.')
					inDot ++;
				else if (srDispObj.szOutput[j] == 0x00)
					break;
			}

			if (inDot == 3 && (strlen(srDispObj.szOutput) > 6))
			{
				if (inConnectWay == _PING_WAY_ETHERNET_)
				{
					inRetVal = inETHERNET_Ping_IP(srDispObj.szOutput);
				}
				else if (inConnectWay == _PING_WAY_ETHERNET_)
				{
					inRetVal = inGPRS_Ping_IP(srDispObj.szOutput);
				}
				else
				{
					inRetVal = VS_ERROR;
				}
				
				if (inRetVal == VS_SUCCESS)
				{
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					inDISP_EnglishFont("Ping Succuess", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
					inDISP_Wait(1000);
				}
				else
				{
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					inDISP_EnglishFont("Ping Failed", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
					inDISP_Wait(1000);
				}
				break;
			}
			else
			{
				inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
				inDISP_EnglishFont("!!  IP ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
				inDISP_BEEP(2, 500);
				inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
				continue;
			}
		}
	}

	return (VS_SUCCESS);
}

/*
Function        :inCOMM_Set_UDP
Date&Time       :2021/12/1 下午 2:37
Describe        :設定UDP相關
*/
int inCOMM_Set_UDP(void)
{
	int		inRetVal = 0, inDot = 0, j = 0;
        char		szDispMsg[16 + 1] = {0};
	char		szTemplate[20 + 1] = {0};
	unsigned char	uszChange = VS_FALSE;	/* 預設沒改變，若改變EDC IP相關，bit on起來並在結尾重新initial裝置 */
	unsigned char	uszCancel = VS_FALSE;	/* 跳出Ethernet設定迴圈 */
	DISPLAY_OBJECT  srDispObj;
	
	while(1)
	{
		/* 輸入POS IP */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont_Color("POS IP", _FONTSIZE_8X16_, _LINE_8_4_, _COLOR_BLACK_, _DISP_LEFT_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		inGetPOS_IP(szDispMsg);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

		while (1)
		{
			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			srDispObj.inMaxLen = 15;
			srDispObj.inY = _LINE_8_7_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inColor = _COLOR_RED_;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;

			inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			{
				uszCancel = VS_TRUE;
				break;
			}

			if (strlen(srDispObj.szOutput) > 0)
			{
				inDot = 0;

				for (j = 0 ;; j ++)
				{
					if (srDispObj.szOutput[j] == '.')
						inDot ++;
					else if (srDispObj.szOutput[j] == 0x00)
						break;
				}

				if (inDot == 3 && (strlen(srDispObj.szOutput) > 6))
				{
					inSetPOS_IP(srDispObj.szOutput);
					if (memcmp(srDispObj.szOutput, "0.0.0.0", 7) == 0)
					{
						inSetSupECR_UDP("N");
					}
					else
					{
						inSetSupECR_UDP("Y");
					}
					inSaveEDCRec(0);
					/* 有做更改 */
					uszChange = VS_TRUE;
					break;
				}
				else
				{
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					inDISP_EnglishFont("!!  IP ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
					inDISP_BEEP(2, 500);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					continue;
				}
			}
			else
			{
				break;
			}
		}
		
		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}
	}
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetSupECR_UDP(szTemplate);
	if (uszChange == VS_TRUE && 
	    szTemplate[0] == 'Y')
	{
		/* UDP Init */
		inUDP_ECR_Initial();
	}
        
        return (inRetVal);
}

int inCOMM_Test(void)
{
        TRANSACTION_OBJECT	pobTran;
        int			inSendSize = 0;
        int			inReceiveSize = 0;
        int			i;
        char			szTemplate[1024 + 1];
	char			szReceive[1024 + 1];
        unsigned char		szSend[1024 + 1] = {"\x60\x00\x22\x80\x00\x02\x00\x30\x20\x07\x80\x20\xC1\x02\x04\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x01\x00\x52\x00\x01\x00\x22\x00\x37\x43\x11\x78\x97\x23\x94\x81\x09\xD1\x70\x42\x01\x14\x95\x01\x17\x10\x00\x0F\x31\x32\x30\x30\x34\x39\x39\x33\x30\x30\x30\x30\x30\x31\x36\x36\x33\x30\x30\x30\x30\x33\x36\x00\x14\x31\x34\x30\x30\x33\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x16\x5F\x2A\x02\x09\x01\x82\x02\x3C\x00\x95\x05\x00\x00\x00\x80\x00\x9A\x03\x15\x09\x07\x9C\x01\x00\x9F\x02\x06\x00\x00\x00\x00\x10\x00\x9F\x03\x06\x00\x00\x00\x00\x00\x00\x9F\x09\x02\x00\x8C\x9F\x10\x07\x06\x01\x0A\x03\xA0\xA0\x06\x9F\x1A\x02\x01\x58\x9F\x26\x08\xDD\xA4\xE3\x11\x68\xA6\xBE\x1B\x9F\x27\x01\x80\x9F\x33\x03\x60\x28\xD0\x9F\x34\x03\x1E\x03\x00\x9F\x35\x01\x22\x9F\x36\x02\x02\x5E\x9F\x37\x04\x16\x68\xF6\x18\x9F\x41\x04\x00\x00\x00\x00\x9F\x53\x01\x52\x00\x06\x30\x30\x30\x30\x30\x31"};
//        char    szTemplate1[4 + 1];

        inLogPrintf(AT, "inCOMM_Test()");

        inSendSize = 438;
        
/*
        if (inLoadCFGTRec(0) < 0)
        {
                return (VS_ERROR);
        }

        if (inLoadCPTRec(0) < 0)
        {
                return (VS_ERROR);
        }
*/

        inCOMM_InitCommDevice();
        inCOMM_ConnectStart(&pobTran);
        inCOMM_Send(szSend, inSendSize, 10, VS_TRUE);

        memset(szReceive,0x00,sizeof(szReceive));
        inCOMM_Receive((unsigned char *)szReceive, 1024, 30, VS_TRUE);
        inReceiveSize = atoi(&szReceive[0]);
    
        memset(szTemplate, 0x00, sizeof(szTemplate));
        int j = 0;
        for (i = 0; i < 60; i++)
        {
                sprintf(&szTemplate[j], "%02X ", szSend[i]);
                j += 3;
                
                if (j == 24)
                {
                    inLogPrintf(AT, szTemplate);
                    memset(szTemplate, 0x00, sizeof(szTemplate));
                    j = 0;
                }
                
        }
/*
  sprintf(&szTemplate[0], "%02X ", szReceive[0]);     
*/
                inLogPrintf(AT, szTemplate);

        inCOMM_End(&pobTran);
        return (VS_SUCCESS);
}
