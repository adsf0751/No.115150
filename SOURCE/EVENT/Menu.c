#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctosapi.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <sqlite3.h>
#include <curl/curl.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/AllStruct.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DisTouch.h"
#include "../../HG/HGsrc.h"
#include "../../HG/HGiso.h"
#include "../FUNCTION/Sqlite.h"
#include "../FUNCTION/Accum.h"
#include "../FUNCTION/ASMC.h"
#include "../FUNCTION/Card.h"
#include "../FUNCTION/CFGT.h"
#include "../FUNCTION/Function.h"
#include "../FUNCTION/HDT.h"
#include "../FUNCTION/HDPT.h"
#include "../FUNCTION/EDC.h"
#include "../FUNCTION/ECR.h"
#include "../FUNCTION/File.h"
#include "../FUNCTION/MultiFunc.h"
#include "../FUNCTION/Signpad.h"
#include "../FUNCTION/KMS.h"
#include "../FUNCTION/PowerManagement.h"
#include "../FUNCTION/PWD.h"
#include "../FUNCTION/TDT.h"
#include "../FUNCTION/IPASSDT.h"
#include "../FUNCTION/ECCDT.h"
#include "../FUNCTION/ICASHDT.h"
#include "../FUNCTION/FuncTable.h"
#include "../FUNCTION/XML.h"
#include "../COMM/Comm.h"
#include "../COMM/Ethernet.h"
#include "../COMM/WiFi.h"
#include "../COMM/Ftps.h"
#include "../../CTLS/CTLS.h"
#include "../../FISC/NCCCfisc.h"
#include "../../NCCC/NCCCats.h"
#include "../../NCCC/NCCCsrc.h"
#include "../../NCCC/NCCCdcc.h"
#include "../../NCCC/NCCCesc.h"
#include "../../NCCC/NCCCloyalty.h"
#include "../../NCCC/NCCCtmk.h"
#include "../../NCCC/NCCCtSAM.h"
#include "../../NCCC/NCCCtms.h"
#include "../../NCCC/NCCCtmsCPT.h"
#include "../../NCCC/NCCCTicketSrc.h"
#include "../../NCCC/NCCCTest.h"
#include "../../NCCC/NCCCescReceipt.h"
#include "../../EMVSRC/EMVsrc.h"
#include "../../EMVSRC/EMVxml.h"
#include "../../CREDIT/CreditprtByBuffer.h"
#include "../../IPASS/IPASSFunc.h"
#include "../../ECC/ICER/stdAfx.h"
#include "../../ECC/ECC.h"
#ifndef  _LOAD_KEY_AP_
	#include "../../ECC/ICER/stdAfx.h"
	#include "../../ECC/ICER/ICERAPI.h"
#endif
#include "../COMM/TLS.h"

#include "Event.h"
#include "MenuMsg.h"
#include "Menu.h"
#include "Flow.h"
#include "CustomerMenu.h"

/* ginIdleDispFlag用來控制什麼時候要display Idle圖片，因為while迴圈內一直顯示圖片會造成觸控判斷延遲，一顯示完圖片就OFF，要離開inEVENT_Responder前ON起來 */
int			ginIdleDispFlag;
int			ginIdleMSRStatus, ginIdleICCStatus, ginMenuKeyIn;
int			ginFallback;
unsigned long		gulPCI_IdleTime;	/* 從什麼時間開始進入Idle */
extern	int		ginESC_Idle_flag;	/* idle是否上傳ESC */
extern  int		ginMenuKeyIn, ginIdleMSRStatus, ginIdleICCStatus;
extern	int		ginDebug;
extern	int		ginISODebug;
extern	int		ginDisplayDebug;
extern	int		ginEngineerDebug;
extern	int		ginFindRunTime;
extern	int		ginMachineType;
extern	int		ginAPVersionType;
extern	int		ginEventCode;
extern	int		ginHalfLCD;
extern	char		gszTermVersionID[15 + 1];
extern	unsigned char	guszCTLSInitiOK;
extern	ECR_TABLE	gsrECROb;

/*
Function        :inMENU_Decide_Idle_Menu
Date&Time       :2017/9/18 下午 3:25
Describe        :這個function是idle畫面，理論上不應該跳出，所以在這function再包一層while
 *		 若其他Idle UI不用，應該註解掉
*/
int inMENU_Decide_Idle_Menu()
{
	/* 剛開機要顯示Idle圖片 */
	ginIdleDispFlag = VS_TRUE;
	/* 清空過磁卡資料 */
	inCARD_Clean_MSR_Buffer();
	
	while (1)
	{
#if defined	_LOAD_KEY_AP_	
	inMENU_Load_Key_UI();
#else
	if (ginMachineType == _CASTLE_TYPE_V3C_)
	{
		inMENU_New_UI();
	}
	else if (ginMachineType == _CASTLE_TYPE_V3M_)
	{
		inMENU_New_UI();
	}
	else if (ginMachineType == _CASTLE_TYPE_V3P_)
	{
		inMENU_New_UI();
	}
	else if (ginMachineType == _CASTLE_TYPE_V3UL_)
	{
		inMENU_Old_UI_V3UL();
	}
	else if (ginMachineType == _CASTLE_TYPE_MP200_)
	{
		inMENU_New_UI_MP200();
	}
	else if (ginMachineType == _CASTLE_TYPE_UPT1000_)
	{
		inMENU_New_UI();
	}
	else
	{
		inMENU_New_UI();
	}
#endif		
	}
		
	return (VS_SUCCESS);	
}

/*
Function        :inMENU_Old_UI_V3UL
Date&Time       :2017/10/2 下午 3:46
Describe        :舊版IDLE UI
*/
int inMENU_Old_UI_V3UL()
{
	int			inRetVal = VS_ERROR;
	char			szCTLSEnable[2 + 1];		/* 觸控是否打開 */
	unsigned char		uszKey;
	TRANSACTION_OBJECT 	pobTran;

	if (ginMachineType == _CASTLE_TYPE_V3UL_)
	{
		memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
		inGetContactlessEnable(szCTLSEnable);

		while (1)
		{
			/* 初始化 */
			ginIdleMSRStatus = VS_FALSE;	/* idle刷卡 */
			ginIdleICCStatus = VS_FALSE;	/* idle插晶片卡 */
			ginMenuKeyIn = VS_FALSE;	/* idle Keyin */

			/* 顯示IDLE畫面控制，一直Display圖片會造成觸控判斷延遲 */
			if (ginIdleDispFlag == VS_TRUE)
			{
				inNCCC_Func_Decide_Display_Idle_Image();

				/* 已顯示Idle圖，關閉flag */
				ginIdleDispFlag  = VS_FALSE; 
			}

			if (memcmp(&szCTLSEnable[0], "Y", 1) != 0 || guszCTLSInitiOK != VS_TRUE)
			{
				/* 未開感應 */
				/* 偵測磁條刷卡 */
				inRetVal = inCARD_MSREvent();

				if (inRetVal == VS_SUCCESS)
				{
					inEVENT_Responder(_SWIPE_EVENT_);
				}
				else if (inRetVal == VS_SWIPE_ERROR)
				{
					inFunc_ResetTitle(&pobTran);
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_READ_CARD_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _ENTER_KEY_MSG_;
					srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 1;
					srDispMsgObj.inBeepInterval = 0;
					
					inDISP_Msg_BMP(&srDispMsgObj);
					/* 刷卡錯誤會蓋idle圖 */
					ginIdleDispFlag = VS_TRUE;
				}

				/* 偵測晶片插卡 */
				inRetVal = inEMV_ICCEvent();
				if (inRetVal == VS_SUCCESS)
				{
					inEVENT_Responder(_EMV_DO_EVENT_);
				}

			}

			/* 偵測多接設備收到資料 */
			if (inMultiFunc_First_Receive_Check() == VS_SUCCESS)
			{
				if (ginDisplayDebug == VS_TRUE)
				{
					char	szDebugMsg[100 + 1];

					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Receive Event");
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
				}
				
				/*若有開工程師debug，只建議直接按0進入 */
				if (ginEngineerDebug == VS_TRUE)
				{

				}
				else
				{
					inEVENT_Responder(_MULTIFUNC_SLAVE_EVENT_);
				}
			}

			/* 偵測按鍵觸發 */
			uszKey = 0x00;
			uszKey = uszKBD_Key();
			if (uszKey != 0x00)
			{
				switch (uszKey)
				{
					case _KEY_1_:
					case _KEY_2_:
					case _KEY_3_:
					case _KEY_4_:
					case _KEY_5_:
					case _KEY_6_:
					case _KEY_7_:
					case _KEY_8_:
					case _KEY_9_:
					case _KEY_0_:
					case _KEY_F1_:
					case _KEY_F2_:
					case _KEY_F3_:
					case _KEY_F4_:
					case _KEY_CLEAR_:
					case _KEY_DOT_:
					case _KEY_ENTER_:
					case _KEY_FUNCTION_:
					case _KEY_CANCEL_:
						inEVENT_Responder(uszKey);
						break;
					default:
						break;
				}

			}

		}/* IDLE while迴圈 */
	}

	return (VS_SUCCESS);
}

/*
Function        :inMENU_New_UI_MP200
Date&Time       :2017/10/2 下午 3:46
Describe        :新版IDLE UI
*/
int inMENU_New_UI_MP200()
{
#if	_MACHINE_TYPE_ == _CASTLE_TYPE_MP200_
	int			inRetVal = VS_ERROR;
	char			szCTLSEnable[2 + 1];		/* 觸控是否打開 */
	unsigned char		uszKey;
	unsigned short		usLen = 0;
	TRANSACTION_OBJECT 	pobTran;
	
	/* 有觸控才要加 */
#ifdef _TOUCH_CAPBILITY_
	int			inFunc = 0;			/* 接收觸控回傳，進哪一個畫面 */
#endif
	memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
	inGetContactlessEnable(szCTLSEnable);
	
	while (1)
	{
                /* 初始化 */
                ginIdleMSRStatus = VS_FALSE;	/* idle刷卡 */
                ginIdleICCStatus = VS_FALSE;	/* idle插晶片卡 */
                ginMenuKeyIn = VS_FALSE;	/* idle Keyin */
		
                /* 顯示IDLE畫面控制，一直Display圖片會造成觸控判斷延遲 */
                if (ginIdleDispFlag == VS_TRUE)
                {
                        inNCCC_Func_Decide_Display_Idle_Image();
			                    
			/* 已顯示Idle圖，關閉flag */
                        ginIdleDispFlag  = VS_FALSE; 
                }
		
#ifdef _TOUCH_CAPBILITY_
                /* IDLE觸控畫面 */
                inFunc = inDISP_DisplayIdleMessage(); /* 回傳IDLE畫面的按鈕選擇 */

                if (inFunc == _IdleTouch_KEY_1_ || inFunc == _IdleTouch_KEY_2_ || inFunc == _IdleTouch_KEY_3_)
                {
                        inEVENT_Responder(inFunc); 
                }
#endif

                if (memcmp(&szCTLSEnable[0], "Y", 1) != 0 || guszCTLSInitiOK != VS_TRUE)
                {
                        /* 未開感應 */
                        /* 偵測磁條刷卡 */
			inRetVal = inCARD_MSREvent();
			
                        if (inRetVal == VS_SUCCESS)
                        {
                                inEVENT_Responder(_SWIPE_EVENT_);
                        }
			else if (inRetVal == VS_SWIPE_ERROR)
			{
				inFunc_ResetTitle(&pobTran);
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, _ERR_READ_CARD_);
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _ENTER_KEY_MSG_;
				srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
				strcpy(srDispMsgObj.szErrMsg1, "");
				srDispMsgObj.inErrMsgLine1 = 0;
				srDispMsgObj.inBeepTimes = 1;
				srDispMsgObj.inBeepInterval = 0;
			
				inDISP_Msg_BMP(&srDispMsgObj);
				/* 刷卡錯誤會蓋idle圖 */
				ginIdleDispFlag = VS_TRUE;
			}
			
                        /* 偵測晶片插卡 */
			inRetVal = inEMV_ICCEvent();
                        if (inRetVal == VS_SUCCESS)
                        {
                                inEVENT_Responder(_EMV_DO_EVENT_);
                        }
                       
                }

                /* 偵測ECR收到資料 */
		if (inECR_Receive_Check(&usLen) == VS_SUCCESS)
            	{
			inEVENT_Responder(_ECR_EVENT_);
		}
		
		/* 顯示狀態 */
		inFunc_Display_All_Status_By_Machine_Type();
		
		/* 偵測按鍵觸發 */
		uszKey = 0x00;
		uszKey = uszKBD_Key();
                if (uszKey != 0x00)
                {
                        switch (uszKey)
                        {
                                case _KEY_1_:
                                case _KEY_2_:
                                case _KEY_3_:
                                case _KEY_4_:
                                case _KEY_5_:
                                case _KEY_6_:
                                case _KEY_7_:
                                case _KEY_8_:
                                case _KEY_9_:
                                case _KEY_0_:
                                case _KEY_F1_:
                                case _KEY_F2_:
                                case _KEY_F3_:
                                case _KEY_F4_:
                                case _KEY_CLEAR_:
				case _KEY_UP_:
                                case _KEY_DOWN_:
                                case _KEY_ENTER_:
				case _KEY_FUNCTION_:
				case _KEY_CANCEL_:
                                        inEVENT_Responder(uszKey);
                                        break;
                                default:
                                        break;
                        }
		
                }
                
	}/* IDLE while迴圈 */
	
#endif
	return (VS_SUCCESS);
}

/*
Function        :inMENU_New_UI
Date&Time       :2017/9/18 下午 3:26
Describe        :因應 "觸控式面板端末設備交易畫面流程需求要點"，所作的新UI
*/
int inMENU_New_UI()
{
#if	(_MACHINE_TYPE_ == _CASTLE_TYPE_V3C_ || _MACHINE_TYPE_ == _CASTLE_TYPE_V3M_ || _MACHINE_TYPE_ == _CASTLE_TYPE_V3P_ || _MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_ || _MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000F_)
	int			inEvent = 0;
	int			inTriggerSecond = 0;		/* 多久檢查一次固定時間觸發事件，時間設得太小會造成觸控延遲 */
	int			inRetVal = VS_ERROR;
	char			szCTLSEnable[2 + 1];		/* 觸控是否打開 */
	char			szOrgTime[10 + 1];		/* 原時間 */
	char			szLastEventTime[10 + 1];		/* 用來避免同一分鐘執行兩次事件 */
	char			szNowTime[10 + 1];
	char			szDebugMsg[100 + 1];
	char			szKBDLock[2 + 1] = {0};
	unsigned char		uszKey = 0;
	unsigned short		usLen = 0;
	RTC_NEXSYS		srRTC;
	TRANSACTION_OBJECT 	pobTran;
	
	/* 有觸控才要加 */
#ifdef _TOUCH_CAPBILITY_
	int			inFunc = 0;			/* 接收觸控回傳，進哪一個畫面 */
#endif
	memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
	inGetContactlessEnable(szCTLSEnable);
	memset(szKBDLock, 0x00, sizeof(szKBDLock));
	inGetKBDLock(szKBDLock);
	
	/* 刷新閒置時間 */
	vdMENU_ReNew_Idle_Time();
	
	while (1)
	{
		/* 初始化 */
                ginIdleMSRStatus = VS_FALSE;	/* idle刷卡 */
                ginIdleICCStatus = VS_FALSE;	/* idle插晶片卡 */
                ginMenuKeyIn = VS_FALSE;	/* idle Keyin */
		
		/* 顯示IDLE畫面控制，一直Display圖片會造成觸控判斷延遲 */
                if (ginIdleDispFlag == VS_TRUE)
                {
			inNCCC_Func_Decide_Display_Idle_Image();
			/* 已顯示Idle圖，關閉flag */
                        ginIdleDispFlag  = VS_FALSE; 
                }
#ifdef _TOUCH_CAPBILITY_
                /* IDLE觸控畫面 */
                inFunc = inDISP_DisplayIdleMessage_NewUI(); /* 回傳IDLE畫面的按鈕選擇 */

		/* (需求單 - 107227)邦柏科技自助作業客製化 要鎖螢幕 by Russell 2018/12/8 下午 9:49 */
		if (memcmp(szKBDLock, "Y", strlen("Y")) == 0)
		{
			
		}
		else
		{
			if (inFunc == _NEWUI_IDLE_Touch_KEY_FUNCTIONKEY_)
			{
				inEVENT_Responder(inFunc);
                                /* 刷新閒置時間 */
                                vdMENU_ReNew_Idle_Time();
			}
		}
		
		/* 需要顯示畫面，重跑迴圈 */
		if (ginIdleDispFlag == VS_TRUE)
		{
			continue;
		}
#endif

                if (memcmp(&szCTLSEnable[0], "Y", 1) != 0 || guszCTLSInitiOK != VS_TRUE)
                {
                        /* 未開感應 */
                        /* 偵測磁條刷卡 */
			inRetVal = inCARD_MSREvent();
			
                        if (inRetVal == VS_SUCCESS)
                        {
                                inEVENT_Responder(_SWIPE_EVENT_);
                                /* 刷新閒置時間 */
                                vdMENU_ReNew_Idle_Time();
                        }
			else if (inRetVal == VS_SWIPE_ERROR)
			{
				inFunc_ResetTitle(&pobTran);
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, _ERR_READ_CARD_);
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _ENTER_KEY_MSG_;
				srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
				strcpy(srDispMsgObj.szErrMsg1, "");
				srDispMsgObj.inErrMsg1Line = 0;
				srDispMsgObj.inBeepTimes = 1;
				srDispMsgObj.inBeepInterval = 0;
			
				inDISP_Msg_BMP(&srDispMsgObj);
                                /* 刷新閒置時間 */
                                vdMENU_ReNew_Idle_Time();
				/* 刷卡錯誤會蓋idle圖 */
				ginIdleDispFlag = VS_TRUE;
			}
			
                        /* 偵測晶片插卡 */
			inRetVal = inEMV_ICCEvent();
                        if (inRetVal == VS_SUCCESS)
                        {
                                inEVENT_Responder(_EMV_DO_EVENT_);
                                /* 刷新閒置時間 */
                                vdMENU_ReNew_Idle_Time();
                        }
                       
                }
		
		/* 需要顯示畫面，重跑迴圈 */
		if (ginIdleDispFlag == VS_TRUE)
		{
			continue;
		}
		
		/* 偵測ECR收到資料 */
                /* 客製化098，查看移除卡片部分有無ECR資料 */
		if (inECR_Receive_Check(&usLen) == VS_SUCCESS || gsrECROb.srTransData.uszUseOrgData == VS_TRUE)
            	{
			inEVENT_Responder(_ECR_EVENT_);
                        /* 刷新閒置時間 */
                        vdMENU_ReNew_Idle_Time();
		}
		
		/* 需要顯示畫面，重跑迴圈 */
		if (ginIdleDispFlag == VS_TRUE)
		{
			continue;
		}
		
		/* 顯示狀態 */
		if (ginMachineType == _CASTLE_TYPE_V3C_ || ginMachineType == _CASTLE_TYPE_V3P_)
		{
			/* 聯合首頁不顯示時間，只顯示網路孔狀況*/
			inFunc_Display_All_Status("01000");
		}
		else
		{
			inFunc_Display_All_Status("01011");
		}
		
		if (ginMachineType == _CASTLE_TYPE_V3M_)
		{
			if (inPWM_IS_PWM_Enable() == VS_SUCCESS)
			{
				/* 如果超過idle時間，進入睡眠模式 */
				if (inTimerGet(_TIMER_POWER_MANAGEMENT_) == VS_SUCCESS)
				{
					inEVENT_Responder(_POWER_MANAGEMENT_EVENT_);
				}
			}
		}
		
		/* 需要顯示畫面，重跑迴圈 */
		if (ginIdleDispFlag == VS_TRUE)
		{
			continue;
		}
		
		/* 偵測按鍵觸發 */
		uszKey = 0x00;
		uszKey = uszKBD_Key();
                if (uszKey != 0x00)
                {
                        switch (uszKey)
                        {
                                case _KEY_1_:
                                case _KEY_2_:
                                case _KEY_3_:
                                case _KEY_4_:
                                case _KEY_5_:
                                case _KEY_6_:
                                case _KEY_7_:
                                case _KEY_8_:
                                case _KEY_9_:
                                case _KEY_0_:
                                case _KEY_F1_:
                                case _KEY_F2_:
                                case _KEY_F3_:
                                case _KEY_F4_:
                                case _KEY_CLEAR_:
                                case _KEY_DOT_:
                                case _KEY_ENTER_:
				case _KEY_FUNCTION_:
				case _KEY_CANCEL_:
                                        inEVENT_Responder(uszKey);
                                        /* 刷新閒置時間 */
                                        vdMENU_ReNew_Idle_Time();
                                        break;
                                default:
                                        break;
                        }
		
                }
                
		/* 需要顯示畫面，重跑迴圈 */
		if (ginIdleDispFlag == VS_TRUE)
		{
			continue;
		}
		
		/* 回到Idle立刻上傳ESC */
		if (ginESC_Idle_flag == _ESC_IDLE_UPLOAD_)
		{
			/* Idle上傳ESC */
			inRetVal = inEVENT_Responder(_ESC_IDLE_UPLOAD_EVENT_);
                        /* ESC上傳不刷新閒置時間，避免無法觸發PCI Reboot */
		}

		/* 需要顯示畫面，重跑迴圈 */
		if (ginIdleDispFlag == VS_TRUE)
		{
			continue;
		}
		
		/* 特定時間會觸發的事件 */
		memset(&srRTC, 0x00, sizeof(srRTC));
		if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		
		memset(szNowTime, 0x00, sizeof(szNowTime));
		sprintf(szNowTime, "%02d%02d%02d%02d%02d", srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
		/* 五秒觸發一次 */
		inTriggerSecond = 1;
		/* 如果時間相同，則代表這是同一秒內進來第二次，就不做動作（同一秒內不檢查兩次時間） */
		if (srRTC.uszSecond % inTriggerSecond == 0 && (memcmp(szOrgTime, szNowTime, 10) != 0))
		{
			/* 紀錄時間，避免一秒進來好幾次 */
			memset(szOrgTime, 0x00, sizeof(szOrgTime));
			memcpy(szOrgTime, szNowTime, 10);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Every %d Seconds Check Once", inTriggerSecond);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 檢查時間觸發的排程 */
			inFunc_IdleCheckALL_DateAndTime(&inEvent);
			
			if (inEvent == _NONE_EVENT_)
			{
				
			}
			else
			{
				/* 因目前不是每個排程都有紀錄是否做過的Bit，可能造成同一分鐘內進來執行好幾次，所以設計為同一分鐘內只執行一個排程
				 * example:DCC整點提示下載，按取消 */
				/* 更新比對至月，除非有一年一次的排程，否則應該不會出錯 */
				if (memcmp(szLastEventTime, szNowTime, 8) == 0)
				{
					/* 同一分鐘內執行第二次排程 */
				}
				else
				{
					/* 紀錄時間，因目前不是每個排程都有紀錄是否做過的Bit，可能造成同一分鐘內進來執行好幾次，所以設計為同一分鐘內只執行一個排程  */
					memcpy(szLastEventTime, szNowTime, 10);

					switch(inEvent)
					{
						/* _NONE_EVENT_代表沒事件 */
						case _NONE_EVENT_:
							break;
						default :
							inEVENT_Responder(inEvent);
                                                        if (inEvent == _ESC_IDLE_UPLOAD_EVENT_)
                                                        {
                                                                
                                                        }
                                                        else
                                                        {
                                                                /* 刷新閒置時間 */
                                                                vdMENU_ReNew_Idle_Time();
                                                        }
							break;
					}
					
				}/* 檢查同一分種內是否執行第二次排程 */
				
			}/* 是否觸發排程 */
			
		}/* 同一秒內只檢查一次排程*/
		
		/* 需要顯示畫面，重跑迴圈 */
		if (ginIdleDispFlag == VS_TRUE)
		{
			continue;
		}
	}
#endif
	
	return (VS_SUCCESS);
}

/*
Function        :inMENU_Load_Key_UI
Date&Time       :2017/11/14 下午 3:59
Describe        :
*/
int inMENU_Load_Key_UI()
{
	unsigned char	uszKey = 0;
	unsigned short	usLen = 0;
		
	while (1)
	{
		/* 顯示IDLE畫面控制，一直Display圖片會造成觸控判斷延遲 */
                if (ginIdleDispFlag == VS_TRUE)
                {
			inDISP_ClearAll();
			inDISP_ChineseFont("1.Download TMK", _FONTSIZE_8X22_, _LINE_8_1_, _DISP_CENTER_);
			inDISP_ChineseFont("2.Insert Key Card", _FONTSIZE_8X22_, _LINE_8_2_, _DISP_CENTER_);
			inDISP_ChineseFont("3.Switch ECR port", _FONTSIZE_8X22_, _LINE_8_3_, _DISP_CENTER_);
			inDISP_ChineseFont("4.Check key Status", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
			/* 已顯示Idle圖，關閉flag */
                        ginIdleDispFlag  = VS_FALSE; 
                }
		
		/* 偵測ECR收到資料 */
		if (inECR_Receive_Check(&usLen) == VS_SUCCESS)
            	{
			inEVENT_Responder(_ECR_EVENT_);
		}
		
		/* 偵測按鍵觸發 */
		uszKey = 0x00;
		uszKey = uszKBD_Key();
                if (uszKey != 0x00)
                {
                        switch (uszKey)
                        {
                                case _KEY_1_:
                                case _KEY_2_:
                                case _KEY_3_:
                                case _KEY_4_:
                                case _KEY_5_:
                                case _KEY_6_:
                                case _KEY_7_:
                                case _KEY_8_:
                                case _KEY_9_:
                                case _KEY_0_:
                                case _KEY_F1_:
                                case _KEY_F2_:
                                case _KEY_F3_:
                                case _KEY_F4_:
                                case _KEY_CLEAR_:
                                case _KEY_DOT_:
                                case _KEY_ENTER_:
				case _KEY_FUNCTION_:
				case _KEY_CANCEL_:
                                        inEVENT_Responder(uszKey);
                                        break;
                                default:
                                        break;
                        }
		
                }
                
	}

	return (VS_SUCCESS);
}

/*
Function	:inMENU_000_MenuFlow_V3UL
Date&Time	:2017/7/11 上午 10:23
Describe	:選擇功能Menu
*/
int inMENU_000_MenuFlow_V3UL(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
		
	if (ginMachineType == _CASTLE_TYPE_V3UL_)
	{
		switch (srEventMenuItem->inEventCode)
		{
			case _IdleTouch_KEY_1_ :
			case _IdleTouch_KEY_2_:
			case _IdleTouch_KEY_3_:
				break;
			case _KEY_0_:
				/* 可模擬感應命令 */
				if (ginEngineerDebug == VS_TRUE)
				{
					inEVENT_Responder(_MULTIFUNC_SLAVE_EVENT_);
				}
				break;
			case _KEY_1_:
			case _KEY_2_:
			case _KEY_3_:
			case _KEY_4_:
			case _KEY_5_:
			case _KEY_6_:
			case _KEY_7_:
			case _KEY_8_:
			case _KEY_9_:
				break;
			case _KEY_ENTER_:
				break;
			case _KEY_FUNCTION_:
			case _KEY_F1_:
				srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
				srEventMenuItem->inCode = FALSE;
				/* 輸入管理號碼 */
				if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
				{
					inRetVal = VS_ERROR;
				}
				else
				{
					inRetVal = inFunc_Edit_LOGONum();
				}
				break;
				break;
			case _KEY_F2_:
				srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
				srEventMenuItem->inCode = FALSE;
				/* 輸入管理號碼 */
				if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
				{
					inRetVal = VS_ERROR;
				}
				else
				{
					inRetVal = inMENU_NEWUI_FUNCTION1_MENU(srEventMenuItem);
				}
			case _KEY_F3_:
			case _KEY_F4_:
				break;
			case _KEY_CANCEL_:
				if (ginEngineerDebug == VS_TRUE)
				{
					inFunc_Exit_AP();
				}
				break;
			case _KEY_CLEAR_:
				if (ginEngineerDebug == VS_TRUE)
				{
					inFunc_ls("-R -l", _AP_ROOT_PATH_);
				}
				break;
			case _KEY_DOT_:
				if (ginEngineerDebug == VS_TRUE)
				{
					inMENU_Developer_Fuction_Flow(srEventMenuItem);
				}
				break;
			case _SWIPE_EVENT_ :
			case _EMV_DO_EVENT_ :
			case _ECR_EVENT_ :
				break;
			case _MULTIFUNC_SLAVE_EVENT_:
				inRetVal = inMENU_MULTIFUNC_SLAVE(srEventMenuItem);
				break;
			case _ESC_IDLE_UPLOAD_EVENT_ :
			case _DCC_SCHEDULE_EVENT_ :
			case _TMS_DCC_SCHEDULE_EVENT_ :
			case _TMS_SCHEDULE_INQUIRE_EVENT_ :
			case _TMS_SCHEDULE_DOWNLOAD_EVENT_ :
			case _TMS_PROCESS_EFFECTIVE_EVENT_ :
			case _DCC_HOUR_NOTIFY_EVENT_:
				break;
			case _BOOTING_EVENT_:
				inRetVal = inMENU_EDC_BOOTING(srEventMenuItem);
				break;
			default:
				break;
		}
	}
	
        return (inRetVal);
}

/*
Function	:inMENU_000_MenuFlow_MP200
Date&Time	:2017/10/2 下午 3:52
Describe	:選擇功能Menu
*/
int inMENU_000_MenuFlow_MP200(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
        switch (srEventMenuItem->inEventCode)
	{
		case _NEWUI_IDLE_Touch_KEY_FUNCTIONKEY_ :
			inRetVal = inMENU_FUNCTION_NEWUI_PAGE_Flow(srEventMenuItem);
			break;
		case _KEY_0_:
			break;
		case _KEY_1_:
		case _KEY_2_:
		case _KEY_3_:
		case _KEY_4_:
		case _KEY_5_:
		case _KEY_6_:
		case _KEY_7_:
		case _KEY_8_:
		case _KEY_9_:
			inRetVal = inMENU_MenuKeyInAndGetAmount(srEventMenuItem);
			break;
		case _KEY_ENTER_:
			inRetVal = inMENU_FUNCTION_NEWUI_PAGE_Flow(srEventMenuItem);
			break;
		case _KEY_F1_:
		case _KEY_F2_:
			break;
		case _KEY_F3_:
			break;
		case _KEY_F4_:
			break;
		case _KEY_CANCEL_:
			if (ginEngineerDebug == VS_TRUE)
			{
				srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
				srEventMenuItem->inCode = FALSE;
				/* 輸入管理號碼 */
				if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
				{
					inRetVal = VS_ERROR;
				}
				else
				{
					exit(0);
				}
			}
			break;
		case _KEY_UP_:
		case _KEY_UP_HALF_ :
			if (ginEngineerDebug == VS_TRUE)
			{
				inRetVal = inMENU_ECR(srEventMenuItem);
			}
			break;
		case _KEY_DOWN_:
		case _KEY_DOWN_HALF_:
			srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
			srEventMenuItem->inCode = FALSE;
			/* 輸入管理號碼 */
			if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			{
				inRetVal = VS_ERROR;
			}
			else
			{
				inRetVal = inWiFi_Test_Menu();
			}
			break;
		case _KEY_CLEAR_:
			if (ginEngineerDebug == VS_TRUE)
			{
				inFunc_ls("-R -l", _AP_ROOT_PATH_);
			}
			break;
		case _SWIPE_EVENT_ :
			inRetVal = inMENU_Swipe(srEventMenuItem);
			break;
		case _EMV_DO_EVENT_ :
			inRetVal = inMENU_ICC(srEventMenuItem);
			break;
		case _ECR_EVENT_ :
			inRetVal = inMENU_ECR(srEventMenuItem);
			break;
		case _MULTIFUNC_SLAVE_EVENT_:
			inRetVal = inMENU_MULTIFUNC_SLAVE(srEventMenuItem);
			break;
		case _ESC_IDLE_UPLOAD_EVENT_ :
			inRetVal = inMENU_ESC_IDLE_UPLOAD(srEventMenuItem);
			break;
		case _DCC_SCHEDULE_EVENT_ :
			inRetVal = inMENU_DCC_SCHEDULE(srEventMenuItem);
			break;
		case _TMS_DCC_SCHEDULE_EVENT_ :
			inRetVal = inMENU_TMS_DCC_SCHEDULE(srEventMenuItem);
			break;
		case _DCC_HOUR_NOTIFY_EVENT_ :
			inRetVal = inMENU_DCC_HOUR_NOTIFY_(srEventMenuItem);
			break;
		case _TMS_SCHEDULE_INQUIRE_EVENT_ :
			inRetVal = inMENU_TMS_SCHEDULE_INQUIRE(srEventMenuItem);
			break;
		case _TMS_SCHEDULE_DOWNLOAD_EVENT_ :
			inRetVal = inMENU_TMS_SCHEDULE_DOWNLOAD(srEventMenuItem);
			break;
		case _TMS_PROCESS_EFFECTIVE_EVENT_ :
			inRetVal = inMENU_TMS_PROCESS_EFFECTIVE(srEventMenuItem);
			break;
		case _BOOTING_EVENT_:
			inRetVal = inMENU_EDC_BOOTING(srEventMenuItem);
			break;
		case _POWER_MANAGEMENT_EVENT_:
			inRetVal = inMENU_POWER_MANAGEMENT(srEventMenuItem);
			break;
		default:
			break;
	}
	
        return (inRetVal);
}

/*
Function	:inMENU_000_MenuFlow_NEWUI
Date&Time	:2017/10/16 下午 4:43
Describe	:選擇功能Menu
*/
int inMENU_000_MenuFlow_NEWUI(EventMenuItem *srEventMenuItem)
{
	int		inRetVal = VS_SUCCESS;
	char		szKBDLock[3 + 1] = {0};
	char		szKey = 0x00;
	unsigned char	uszRejudgeBit = VS_FALSE;
	
	memset(szKBDLock, 0x00, sizeof(szKBDLock));
	inGetKBDLock(szKBDLock);
	
	do
	{
		uszRejudgeBit = VS_FALSE;
		
		/* (需求單 - 107227)邦柏科技自助作業客製化 主畫面要鎖按鍵 2018/11/28 下午 3:03 by Russell */
		if (memcmp(szKBDLock, "Y", strlen("Y")) == 0)
		{
			switch (srEventMenuItem->inEventCode)
			{
				case _NEWUI_IDLE_Touch_KEY_FUNCTIONKEY_ :
					break;
				case _KEY_0_:
					/* 壓住0後3秒內按clear */
					inDISP_Timer_Start(_TIMER_NEXSYS_4_, 3);
					do
					{
						/* 這裡使用uszKBD_Key而不用uszKBD_Key_In是為了不留在Buffer中 */
						szKey = uszKBD_Key();
					}while (szKey == 0	&&
						inTimerGet(_TIMER_NEXSYS_4_) != VS_SUCCESS);

					/* 不是按clear，不能進隱藏選單 */
					if (szKey != _KEY_CLEAR_)
					{
						uszRejudgeBit = VS_TRUE;
						srEventMenuItem->inEventCode = szKey;
						ginEventCode = szKey;
					}
					else
					{
						inFlushKBDBuffer();

						srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
						srEventMenuItem->inCode = FALSE;
						/* 輸入管理號碼 */
						inDISP_ClearAll();
						if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
						{

						}
						else
						{
							inRetVal = inMENU_NEWUI_FUNCTION1_MENU(srEventMenuItem);
							if (inRetVal == VS_USER_CANCEL)
							{

							}
							else
							{

							}
						}
					}
					break;
				case _KEY_1_:
				case _KEY_2_:
				case _KEY_3_:
				case _KEY_4_:
				case _KEY_5_:
				case _KEY_6_:
				case _KEY_7_:
				case _KEY_8_:
				case _KEY_9_:
					break;
				case _KEY_ENTER_:
					break;
				case _KEY_FUNCTION_:
					break;
				case _KEY_F1_:
				case _KEY_F2_:
					break;
				case _KEY_F3_:
					break;
				case _KEY_F4_:
					break;
				case _KEY_CANCEL_:
					if (ginEngineerDebug == VS_TRUE)
					{
						exit(0);
					}
					break;
				case _KEY_CLEAR_:
					if (ginEngineerDebug == VS_TRUE)
					{
						inFunc_ls("-R -l", _AP_ROOT_PATH_);
					}
					break;
				case _KEY_DOT_:
					if (ginEngineerDebug == VS_TRUE)
					{
						inMENU_Developer_Fuction_Flow(srEventMenuItem);
					}
					break;
				case _SWIPE_EVENT_ :
					inRetVal = inMENU_Swipe(srEventMenuItem);
					break;
				case _EMV_DO_EVENT_ :
					inRetVal = inMENU_ICC(srEventMenuItem);
					break;
				case _ECR_EVENT_ :
					inRetVal = inMENU_ECR(srEventMenuItem);
					break;
				case _MULTIFUNC_SLAVE_EVENT_:
					inRetVal = inMENU_MULTIFUNC_SLAVE(srEventMenuItem);
					break;
				case _ESC_IDLE_UPLOAD_EVENT_ :
					inRetVal = inMENU_ESC_IDLE_UPLOAD(srEventMenuItem);
					break;
				case _DCC_SCHEDULE_EVENT_ :
					inRetVal = inMENU_DCC_SCHEDULE(srEventMenuItem);
					break;
				case _TMS_DCC_SCHEDULE_EVENT_ :
					inRetVal = inMENU_TMS_DCC_SCHEDULE(srEventMenuItem);
					break;
				case _DCC_HOUR_NOTIFY_EVENT_ :
					inRetVal = inMENU_DCC_HOUR_NOTIFY_(srEventMenuItem);
					break;
				case _TMS_SCHEDULE_INQUIRE_EVENT_ :
					inRetVal = inMENU_TMS_SCHEDULE_INQUIRE(srEventMenuItem);
					break;
				case _TMS_SCHEDULE_DOWNLOAD_EVENT_ :
					inRetVal = inMENU_TMS_SCHEDULE_DOWNLOAD(srEventMenuItem);
					break;
				case _TMS_PROCESS_EFFECTIVE_EVENT_ :
					inRetVal = inMENU_TMS_PROCESS_EFFECTIVE(srEventMenuItem);
					break;
				case _BOOTING_EVENT_:
					inRetVal = inMENU_EDC_BOOTING(srEventMenuItem);
					break;
				case _POWER_MANAGEMENT_EVENT_:
					inRetVal = inMENU_POWER_MANAGEMENT(srEventMenuItem);
					break;
				case _ESVC_AUTO_SIGNON_EVENT_:
					inRetVal = inMENU_ESVC_AUTO_SIGNON(srEventMenuItem);
					break;
				default:
					break;
			}
		}
		/* 一般標準板 */
		else
		{
			switch (srEventMenuItem->inEventCode)
			{
				case _NEWUI_IDLE_Touch_KEY_FUNCTIONKEY_ :
					inRetVal = inMENU_FUNCTION_NEWUI_PAGE_Flow(srEventMenuItem);
					break;
				case _KEY_0_:
					/* 壓住0後3秒內按clear */
					inDISP_Timer_Start(_TIMER_NEXSYS_4_, 3);
					do
					{
						/* 這裡使用uszKBD_Key而不用uszKBD_Key_In是為了不留在Buffer中 */
						szKey = uszKBD_Key();
					}while (szKey == 0	&&
						inTimerGet(_TIMER_NEXSYS_4_) != VS_SUCCESS);

					/* 不是按clear，不能進隱藏選單 */
					if (szKey != _KEY_CLEAR_)
					{
						uszRejudgeBit = VS_TRUE;
						srEventMenuItem->inEventCode = szKey;
						ginEventCode = szKey;
					}
					else
					{
						inFlushKBDBuffer();

						srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
						srEventMenuItem->inCode = FALSE;
						/* 輸入管理號碼 */
						inDISP_ClearAll();
						if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
						{

						}
						else
						{
							inRetVal = inMENU_NEWUI_FUNCTION1_MENU(srEventMenuItem);
							if (inRetVal == VS_USER_CANCEL)
							{

							}
							else
							{

							}
						}
					}
					break;
				case _KEY_1_:
				case _KEY_2_:
				case _KEY_3_:
				case _KEY_4_:
				case _KEY_5_:
				case _KEY_6_:
				case _KEY_7_:
				case _KEY_8_:
				case _KEY_9_:
					inRetVal = inMENU_MenuKeyInAndGetAmount(srEventMenuItem);
					break;
				case _KEY_ENTER_:
					inRetVal = inMENU_FUNCTION_NEWUI_PAGE_Flow(srEventMenuItem);
					break;
				case _KEY_FUNCTION_:
                                        /* 壓住0後3秒內按clear */
					inDISP_Timer_Start(_TIMER_NEXSYS_4_, 3);
					do
					{
						/* 這裡使用uszKBD_Key而不用uszKBD_Key_In是為了不留在Buffer中 */
						szKey = uszKBD_Key();
					}while (szKey == 0	&&
						inTimerGet(_TIMER_NEXSYS_4_) != VS_SUCCESS);

					/* 不是按clear，不能進隱藏選單 */
					if (szKey == _KEY_CLEAR_)
                                        {
						inFlushKBDBuffer();
                                                inRetVal = inMENU_COMM_MENU(srEventMenuItem);
					}
                                        else if (szKey == _KEY_CANCEL_)
                                        {
						inFlushKBDBuffer();
                                                inRetVal = inMENU_TMS_PARAMETER_DOWNLOAD(srEventMenuItem);
					}
                                        else
					{
						uszRejudgeBit = VS_TRUE;
						srEventMenuItem->inEventCode = szKey;
						ginEventCode = szKey;
					}
					break;
				case _KEY_F1_:
				case _KEY_F2_:
					break;
				case _KEY_F3_:
					break;
				case _KEY_F4_:
					break;
				case _KEY_CANCEL_:
					if (ginEngineerDebug == VS_TRUE)
					{
						exit(0);
					}
					break;
				case _KEY_CLEAR_:
					if (ginEngineerDebug == VS_TRUE)
					{
						inFunc_ls("-R -l", _AP_ROOT_PATH_);
					}
					break;
				case _KEY_DOT_:
					if (ginEngineerDebug == VS_TRUE)
					{
						inMENU_Developer_Fuction_Flow(srEventMenuItem);
					}
                                        else
                                        {
                                                /* 壓住0後3秒內按clear */
                                                inDISP_Timer_Start(_TIMER_NEXSYS_4_, 3);
                                                do
                                                {
                                                        /* 這裡使用uszKBD_Key而不用uszKBD_Key_In是為了不留在Buffer中 */
                                                        szKey = uszKBD_Key();
                                                }while (szKey == 0	&&
                                                        inTimerGet(_TIMER_NEXSYS_4_) != VS_SUCCESS);

                                                /* 不是按clear，不能進隱藏選單 */
                                                if (szKey == _KEY_CANCEL_)
                                                {
                                                        inFlushKBDBuffer();
                                                        inRetVal = inMENU_DCC_PARAMETER_DOWNLOAD(srEventMenuItem);
                                                }
                                                else
                                                {
                                                        uszRejudgeBit = VS_TRUE;
                                                        srEventMenuItem->inEventCode = szKey;
                                                        ginEventCode = szKey;
                                                }
                                        }
					break;
				case _SWIPE_EVENT_ :
					inRetVal = inMENU_Swipe(srEventMenuItem);
					break;
				case _EMV_DO_EVENT_ :
					inRetVal = inMENU_ICC(srEventMenuItem);
					break;
				case _ECR_EVENT_ :
					inRetVal = inMENU_ECR(srEventMenuItem);
					break;
				case _MULTIFUNC_SLAVE_EVENT_:
					inRetVal = inMENU_MULTIFUNC_SLAVE(srEventMenuItem);
					break;
				case _ESC_IDLE_UPLOAD_EVENT_ :
					inRetVal = inMENU_ESC_IDLE_UPLOAD(srEventMenuItem);
					break;
				case _DCC_SCHEDULE_EVENT_ :
					inRetVal = inMENU_DCC_SCHEDULE(srEventMenuItem);
					break;
				case _TMS_DCC_SCHEDULE_EVENT_ :
					inRetVal = inMENU_TMS_DCC_SCHEDULE(srEventMenuItem);
					break;
				case _DCC_HOUR_NOTIFY_EVENT_ :
					inRetVal = inMENU_DCC_HOUR_NOTIFY_(srEventMenuItem);
					break;
				case _TMS_SCHEDULE_INQUIRE_EVENT_ :
					inRetVal = inMENU_TMS_SCHEDULE_INQUIRE(srEventMenuItem);
					break;
				case _TMS_SCHEDULE_DOWNLOAD_EVENT_ :
					inRetVal = inMENU_TMS_SCHEDULE_DOWNLOAD(srEventMenuItem);
					break;
				case _TMS_PROCESS_EFFECTIVE_EVENT_ :
					inRetVal = inMENU_TMS_PROCESS_EFFECTIVE(srEventMenuItem);
					break;
				case _BOOTING_EVENT_:
					inRetVal = inMENU_EDC_BOOTING(srEventMenuItem);
					break;
				case _POWER_MANAGEMENT_EVENT_:
					inRetVal = inMENU_POWER_MANAGEMENT(srEventMenuItem);
					break;
				case _ESVC_AUTO_SIGNON_EVENT_:
					inRetVal = inMENU_ESVC_AUTO_SIGNON(srEventMenuItem);
					break;
				default:
					break;
			}
		}
		
	}while (uszRejudgeBit == VS_TRUE);
	
        return (inRetVal);
}

/*
Function	:inMENU_000_MenuFlow_LoadKeyUI
Date&Time	:2017/11/14 下午 4:07
Describe	:選擇功能Menu
*/
int inMENU_000_MenuFlow_LoadKeyUI(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	switch (srEventMenuItem->inEventCode)
	{
		case _KEY_0_:
			break;
		case _KEY_1_:
			inRetVal = inMENU_ECR_OPERATION_LOAD_KEY_FROM_520(srEventMenuItem);
			break;
		case _KEY_2_:
			inRetVal = inMENU_ECR_OPERATION_LOAD_KEY_FROM_KEY_CARD(srEventMenuItem);
			break;
		case _KEY_3_:
			inRetVal = inFunc_ECR_Comport_Switch();
			inECR_Load_TMK_DeInitial();
			inECR_Load_TMK_Initial();
			break;
		case _KEY_4_:
			inDISP_ClearAll();
			inNCCC_TMK_GetKeyInfo_LookUp_Default();
			break;
		case _KEY_5_:
		case _KEY_6_:
		case _KEY_7_:
		case _KEY_8_:
		case _KEY_9_:
			break;
		case _KEY_ENTER_:
			inRetVal = inMENU_NEWUI_FUNCTION1_MENU(srEventMenuItem);
			break;
		case _KEY_FUNCTION_:
			break;
		case _KEY_F1_:
		case _KEY_F2_:
			break;
		case _KEY_F3_:
			break;
		case _KEY_F4_:
			break;
		case _KEY_CANCEL_:
			exit(0);
			break;
		case _KEY_CLEAR_:
			break;
		case _KEY_DOT_:
			break;
		case _ECR_EVENT_ :
			inRetVal = inMENU_ECR_OPERATION_LOAD_KEY_FROM_520(srEventMenuItem);
			break;
		case _BOOTING_EVENT_:
			inRetVal = inMENU_EDC_BOOTING(srEventMenuItem);
			break;
		default:
			break;
	}
	
        return (inRetVal);
}

/*
Function	:inMENU_SELECT_KEY_FUNC
Date&Time	:2015/10/26 下午 2:48
Describe	:功能鍵
*/
int inMENU_SELECT_KEY_FUNC(EventMenuItem *srEventMenuItem)
{
        int     inRetVal = VS_SUCCESS;
        char    szKey;
        VS_BOOL fSelectPage = VS_TRUE;

        inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
        inDISP_PutGraphic(_MENU_FUNCTION_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);

        do
        {
                szKey = uszKBD_GetKey(180);

                if (szKey == _KEY_1_)
                {
                        fSelectPage = VS_FALSE;
                        /* ReaderInit */
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
                        inDISP_PutGraphic(_MENU_FUNCTION_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);
                        srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
                        srEventMenuItem->inCode = FALSE;
                        srEventMenuItem->inRunOperationID = _OPERATION_FUN1_READERINIT_;
                        srEventMenuItem->inRunTRTID = FALSE;
                        /* 輸入管理號碼 */
                        if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
                                return (VS_ERROR);
                }
                else if (szKey == _KEY_3_)
                {
                        fSelectPage = VS_FALSE;
                        /* 通訊時間設定 */
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
                        inDISP_PutGraphic(_MENU_FUNC3_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);
                        srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
                        srEventMenuItem->inCode = FALSE;
                        /* 輸入管理號碼 */
                        if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
                                return (VS_ERROR);
                        
                        /* 功能3選單 */
                        inMENU_FUN3_COMM_TIME_SET(srEventMenuItem);
                }
                else if (szKey == _KEY_4_)
                {
                        fSelectPage = VS_FALSE;
                        /* Signpad列印測試 */
                        inDISP_ClearAll();
                        srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
                        srEventMenuItem->inCode = FALSE;
                        srEventMenuItem->inRunOperationID = _OPERATION_FUN4_PRINT_;
                        srEventMenuItem->inRunTRTID = FALSE;
                }
                else if (szKey == _KEY_5_)
                {
                        fSelectPage = VS_FALSE;
                        /* TMS參數下載 */
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
                        inDISP_PutGraphic(_MENU_FUNC5_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);
                        srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
                        srEventMenuItem->inCode = FALSE;
                        /* 輸入管理號碼 */
                        if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
                                return (VS_ERROR);
                        
                        /* 功能5選單 */
                        inMENU_FUN5_TMS(srEventMenuItem);
                }          
                else if (szKey == _KEY_6_)
                {
                        fSelectPage = VS_FALSE;
                        /* 商店可用的參數下載 */
                        inDISP_ClearAll();
                        srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
                        srEventMenuItem->inCode = FALSE;
                        /* 功能6選單 */
                        inMENU_FUN6_SELECT(srEventMenuItem);
                        
                }
                else if (szKey == _KEY_7_)
                {
                        fSelectPage = VS_FALSE;
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
                        inDISP_PutGraphic(_MENU_FUNC7_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 功能七 */
                        srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
                        srEventMenuItem->inCode = FALSE;
                        /* 輸入管理號碼 */
                        if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
                                return (VS_ERROR);

                        /* 列印參數表 */
                        srEventMenuItem->inRunOperationID = _OPERATION_FUN7_PRINT_;
                        srEventMenuItem->inRunTRTID = FALSE;
                }
		else if (szKey == _KEY_8_)
                {
//                        fSelectPage = VS_FALSE;
//                        inDISP_ClearAll();
//                        srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
//                        srEventMenuItem->inCode = FALSE;
//                        /* 輸入管理號碼 */
//                        if (inCheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
//                                return (VS_ERROR);
//
//                        /* 列印參數表 */
//                        srEventMenuItem->inRunOperationID = _OPERATION_FUN8_PRINT_;
//                        srEventMenuItem->inRunTRTID = FALSE;
                }
                else if (szKey == _KEY_9_)
                {
                        /* AP更新測試 */
                        char szTemplate[32 + 1];
                        inDISP_ClearAll();
                        inDISP_ChineseFont("AP更新中", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
                        system("test -d /home/ap/pub/install || mkdir -p /home/ap/pub/install/");
                        system("cp -vfr * /home/ap/pub/install/.");
                        system("ls /home/ap/pub/install/");
                        inRetVal = CTOS_UpdateFromMMCIEx((unsigned char *)"/home/ap/pub/install/update.mmci", &callbackFun);
                        CTOS_UpdateGetResult();
                        sprintf(szTemplate, "inRetVal = 0x%04X", inRetVal);
                        inLogPrintf(AT, szTemplate);
                        CTOS_SystemReset();
                }
                else if (szKey == _KEY_0_)
                {
                        /* xml測試 */
//                        WenswriteXML();
                        vdEMVXML_write_test();
                        inDISP_ChineseFont("XML測試", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
//                        CTOS_LCDTPrintXY(1, 1, "Any Key to Cont.");
//                        DumpAllXmlDoc("test.xml");
//                        DumpAllXmlDoc("emv_config.xml");
                        inFILE_Test();
                    
                }
                else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
                {
                        inRetVal = VS_ERROR;
                        fSelectPage = VS_FALSE;
                }
                else
                        continue;
                break;

        }while (fSelectPage == VS_TRUE);

        return (inRetVal);
}

int inMENU_SELECT_KEY_FUNC_BAK(EventMenuItem *srEventMenuItem)
{
        int     inRetVal = VS_SUCCESS;
//        char    szKey;
//        VS_BOOL fSelectPage = VS_TRUE;

//        inDISP_ClearAll();
//        inDISP_PutGraphic(_MENU_PRT_REPORT_, 0,  0);
//
//        do
//        {
//                szKey = szKBD_GetKey(180);
//
//                if (szKey == _KEY_1_)
//                {
//                        fSelectPage = VS_FALSE;
//                        inDISP_ClearAll();
//                        /* 第三層顯示 ＜總額報表＞ */
//                        inDISP_PutGraphic(_MENU_TOTAIL_TITLE_, 0,  0);
//                        /* 輸入密碼的層級 */
//                        srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
//                        srEventMenuItem->inCode = FALSE;
//
//                        /* 第一層輸入密碼 */
//                        if (inCheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
//                                return (VS_ERROR);
//
//                        srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
//                        srEventMenuItem->inCode = FALSE;
//                        srEventMenuItem->inRunOperationID = _OPERATION_DELETE_BATCH_;
//                        srEventMenuItem->inRunTRTID = FALSE;
//                }
//                else if (szKey == _KEY_2_)
//                {
//                        fSelectPage = VS_FALSE;
//                        inDISP_ClearAll();
//                        /* 第三層顯示 ＜明細報表＞ */
//                        inDISP_PutGraphic(_MENU_DETAIL_TITLE_, 0,  0);
//                        /* 輸入密碼的層級 */
//                        srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
//                        srEventMenuItem->inCode = FALSE;
//
//                        /* 第一層輸入密碼 */
//                        if (inCheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
//                                return (VS_ERROR);
//
//                        srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
//                        srEventMenuItem->inCode = FALSE;
//                        srEventMenuItem->inRunOperationID = _OPERATION_DETAIL_REPORT_;
//                        srEventMenuItem->inRunTRTID = FALSE;
//                }
//                else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
//                {
//                        inRetVal = VS_ERROR;
//                        fSelectPage = VS_FALSE;
//                }
//                else
//                        continue;
//
//                break;
//        } while (fSelectPage == VS_TRUE);

        return (inRetVal);
}

/*
Function	:inMENU_SELECT_KEY_F3
Date&Time	:2015/8/20 下午 2:48
Describe	:列印報表
*/
int inMENU_SELECT_KEY_F3(EventMenuItem *srEventMenuItem)
{
        int	inRetVal = VS_SUCCESS;
        unsigned char   uszKey;
	VS_BOOL	fSelectPage = VS_TRUE;

	inDISP_ClearAll();
 //       inDISP_PutGraphic(_MENU_PRT_REPORT_, 0,  0);

        do
        {
                uszKey = uszKBD_GetKey(180);

                if (uszKey == _KEY_1_)
                {
                        fSelectPage = VS_FALSE;
                        inDISP_ClearAll();
                        inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TOTAIL_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 總額報表 */
                        /* 輸入密碼的層級 */
                        srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
                        srEventMenuItem->inCode = FALSE;

                        /* 第一層輸入密碼 */
                        if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
                                return (VS_ERROR);

                        srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
                        srEventMenuItem->inCode = FALSE;
                        srEventMenuItem->inRunOperationID = _OPERATION_TOTAL_REPORT_;
                        srEventMenuItem->inRunTRTID = FALSE;
                }
                else if (uszKey == _KEY_2_)
                {
                        fSelectPage = VS_FALSE;
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
                        inDISP_PutGraphic(_MENU_DETAIL_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 明細報表 */
                        /* 輸入密碼的層級 */
                        srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
                        srEventMenuItem->inCode = FALSE;

                        /* 第一層輸入密碼 */
                        if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
                                return (VS_ERROR);

                        srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
                        srEventMenuItem->inCode = FALSE;
                        srEventMenuItem->inRunOperationID = _OPERATION_DETAIL_REPORT_;
                        srEventMenuItem->inRunTRTID = FALSE;
                }
                else if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
                {
                        inRetVal = VS_ERROR;
                        fSelectPage = VS_FALSE;
                }
                else
                        continue;

                break;
        } while (fSelectPage == VS_TRUE);

	return (inRetVal);
}

/*
Function	:inMENU_Swipe
Date&Time	:2015/8/24 上午 11:49
Describe	:磁條流程
*/
int inMENU_Swipe(EventMenuItem *srEventMenuItem)
{
        inDISP_ClearAll();
        inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */
        /* 輸入密碼的層級 */
        srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
        srEventMenuItem->inCode = _SALE_;

        /* 第一層輸入密碼 */
        if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
                return (VS_ERROR);

        srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
        srEventMenuItem->inCode = _SALE_;
        srEventMenuItem->inRunOperationID = _OPERATION_SALE_ICC_;
        srEventMenuItem->inRunTRTID = _TRT_SALE_;

        /* Idle 刷卡流程 */
        ginIdleMSRStatus = VS_TRUE;

        return (VS_SUCCESS);
}

/*
Function	:inMENU_MenuKeyIn
Date&Time	:2015/8/24 上午 11:49
Describe	:idle輸入
*/
int inMENU_MenuKeyInAndGetAmount(EventMenuItem *srEventMenuItem)
{
        char	szCTLSEnable[2 + 1];
        
        memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
	inGetContactlessEnable(szCTLSEnable);
        
        if (!memcmp(&szCTLSEnable[0], "Y", 1) && srEventMenuItem->inEventCode == '0')
        {
                /* 不接受金額第一位為0 */
                return (VS_ERROR);
        }

        inDISP_ClearAll();
        /* 第三層顯示 ＜一般交易＞ */
        inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */
        /* 輸入密碼的層級 */
        srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
        srEventMenuItem->inCode = _SALE_;

	/* 這邊先設定TRT是因為在抓卡號時inFunc_ResetTitle這隻函數會用到 */
        if (!memcmp(&szCTLSEnable[0], "Y", 1) && guszCTLSInitiOK == VS_TRUE)
        {
		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		
		/* Contactless功能開啟時，信用卡正向(銷售)交易管理號碼在輸入金額後 */
                srEventMenuItem->inRunOperationID = _OPERATION_SALE_CTLS_IDLE_;
                srEventMenuItem->inRunTRTID = _TRT_SALE_CTLS_;
        }
        else
        {
		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);
		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	
                srEventMenuItem->inRunOperationID = _OPERATION_SALE_ICC_;
                srEventMenuItem->inRunTRTID = _TRT_SALE_;
        }

        return (VS_SUCCESS);
}


/*
Function	:inMENU_ICC
Date&Time	:2015/8/24 上午 11:49
Describe	:晶片卡流程
*/
int inMENU_ICC(EventMenuItem *srEventMenuItem)
{
        inDISP_ClearAll();
        inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */
        /* 輸入密碼的層級 */
        srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
        srEventMenuItem->inCode = _SALE_;

        /* 第一層輸入密碼 */
        if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
                return (VS_ERROR);

        srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
        srEventMenuItem->inCode = _SALE_;
        srEventMenuItem->inRunOperationID = _OPERATION_SALE_ICC_;
        srEventMenuItem->inRunTRTID = _TRT_SALE_ICC_;

        /* Idle 插晶片卡流程 */
        ginIdleICCStatus = VS_TRUE;
        return (VS_SUCCESS);
}

/*
Function	:inMENU_FUN3_TMS
Date&Time	:2016/1/28 下午 8:14
Describe	:功能3選單 1.通訊設定 2.時間設定 3.版本查詢
*/
int inMENU_FUN3_COMM_TIME_SET(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
        int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_8X16_OPT_;
	char	szKey = 0x00;
        
        inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
        inDISP_PutGraphic(_MENU_FUNC3_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);
	
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	srEventMenuItem->inCode = FALSE;
	/* 輸入管理號碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);
	srEventMenuItem->inPasswordLevel =  _ACCESS_FREELY_;
	
	/* 功能三選單 */
	inDISP_PutGraphic(_MENU_FUNC3_OPTION_, 0,  _COORDINATE_Y_LINE_8_4_);
	
        inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
        while (1)
        {
                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                szKey = uszKBD_Key();
		
		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}
                
                if (szKey == _KEY_1_			||
		    inChoice == _OPTTouch8X16_LINE_5_)
                {
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SET_COMM_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <通訊設定> */
			srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
			srEventMenuItem->inCode = FALSE;
                        /* 通訊設定 */
                        inRetVal = inCOMM_Fun3_SetCommWay();
                        break;
                }
                else if (szKey == _KEY_2_			||
			 inChoice == _OPTTouch8X16_LINE_6_)
                {
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SET_DATE_TIME_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜時間設定＞ */
			srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
			srEventMenuItem->inCode = FALSE;
                        /* 時間設定 */
                        inRetVal = inFunc_Fun3EditDateTime();
			break;
                }
                else if (szKey == _KEY_3_			||
			 inChoice == _OPTTouch8X16_LINE_7_)
                {
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_AP_VERSION_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜版本查詢> */
			srEventMenuItem->inPasswordLevel =  _ACCESS_FREELY_;
			srEventMenuItem->inCode = FALSE;
                        /* 版本查詢 */
                        inRetVal = inFunc_Check_Version_ID();
                        break;
                }
                else if (szKey == _KEY_CANCEL_)
                {
                        inRetVal = VS_USER_CANCEL;
			break;
                }
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
        }
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
	
	return (inRetVal);
}

/*
Function	:inMENU_FUN5_TMS
Date&Time	:2016/1/4 下午 3:02
Describe	:功能5 TMS下載選單 1.參數下載 2.至現回報
*/
int inMENU_FUN5_TMS(EventMenuItem *srEventMenuItem)
{
        int	inRetVal = VS_SUCCESS;
        int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_8X16_OPT_;
	char	szKey = 0x00;
        
	/* 功能五 */
        inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
        inDISP_PutGraphic(_MENU_FUNC5_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	srEventMenuItem->inCode = FALSE;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);
			
	inDISP_PutGraphic(_MENU_FUNC5_OPTION_, 0,  _COORDINATE_Y_LINE_8_4_);
        inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
        while (1)
        {
                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                szKey = uszKBD_Key();
		
		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}
                
                if (szKey == _KEY_1_			||
		    inChoice == _OPTTouch8X16_LINE_5_)
                {
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TMS_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜TMS參數下載＞ */
			/* 輸入密碼的層級 */
			srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
			srEventMenuItem->inCode = FALSE;

			/* 第一層輸入密碼 */
			if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			{
				inRetVal = VS_ERROR;
				break;
			}

			srEventMenuItem->inRunOperationID = _OPERATION_FUN5_TMS_DOWNLOAD_;
			srEventMenuItem->inRunTRTID = FALSE;
			
                        break;
                }
                else if (szKey == _KEY_2_			||
			 inChoice == _OPTTouch8X16_LINE_6_)
                {
                        inDISP_ClearAll();
			/* 第三層顯示 ＜DCC參數下載＞ */
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_DCC_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜DCC參數下載＞ */
			/* 輸入密碼的層級 */
			srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
			srEventMenuItem->inCode = FALSE;

			/* 第一層輸入密碼 */
			if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			{
				inRetVal = VS_ERROR;
				break;
			}

			srEventMenuItem->inRunOperationID = _OPERATION_FUN6_DCC_DOWNLOAD_;
			srEventMenuItem->inRunTRTID = FALSE;
			
                        break;
                }
		else if (szKey == _KEY_3_			||
			 inChoice == _OPTTouch8X16_LINE_7_)
                {
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TMS_TASK_MENU_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜至現回報＞ */
			srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
			srEventMenuItem->inCode = FALSE;
			/* 輸入管理號碼 */
			if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			{
				inRetVal = VS_ERROR;
				break;
			}
			srEventMenuItem->inRunOperationID = _OPERATION_FUN5_TMS_TASK_REPORT_;
			srEventMenuItem->inRunTRTID = FALSE;
			
                        break;
                }
                else if (szKey == _KEY_CANCEL_)
                {
                        inRetVal = VS_USER_CANCEL;
			break;
                }
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
        }
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
	
	return (inRetVal);
}

/*
Function	:inMENU_FUN6_SELECT
Date&Time	:2016/1/4 下午 3:02
Describe	:功能6選單 1.設定管理號碼 2.資訊回報 3.參數下載 4.版本查詢
*/
int inMENU_FUN6_SELECT(EventMenuItem *srEventMenuItem)
{
        int	inRetVal = VS_SUCCESS;
        int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_8X16_OPT_;
	char	szKey = 0x00;

        inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
        inDISP_PutGraphic(_MENU_FUNC6_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 2u4*/
	inDISP_PutGraphic(_MENU_FUNC6_OPTION_, 0,  _COORDINATE_Y_LINE_8_4_);
        
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
        while (1)
        {
                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                szKey = uszKBD_Key();
		
		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}
                
                if (szKey == _KEY_1_			||
		    inChoice == _OPTTouch8X16_LINE_5_)
                {
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SET_PWD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 設定管理號碼 */
			srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
			srEventMenuItem->inCode = FALSE;
			/* 輸入管理號碼 */
			if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			{
				inRetVal = VS_ERROR;
				break;
			}
			inRetVal = inFunc_EditPWD_Flow();
			
			break;
                }
                else if (szKey == _KEY_2_			||
			 inChoice == _OPTTouch8X16_LINE_6_)
                {
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_RETURN_PARAM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜資訊回報＞ */
			srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
			srEventMenuItem->inCode = FALSE;
			/* 輸入管理號碼 */
			if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			{
				inRetVal = VS_ERROR;
				break;
			}
			srEventMenuItem->inRunOperationID = _OPERATION_FUN6_TMS_TRACELOG_UP_;
			
                        break;
                }
                else if (szKey == _KEY_3_			||
			 inChoice == _OPTTouch8X16_LINE_7_)
                {
                        /* 參數下載 */
			inRetVal = inMENU_Download_Parameter(srEventMenuItem);
			
                        break;
                }
                else if (szKey == _KEY_4_			||
			 inChoice == _OPTTouch8X16_LINE_8_)
                {
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_AP_VERSION_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜版本查詢> */
			srEventMenuItem->inPasswordLevel =  _ACCESS_FREELY_;
			srEventMenuItem->inCode = FALSE;
                        /* 版本查詢 */
                        inRetVal = inFunc_Check_Version_ID();
			
                        break;
                }
                else if (szKey == _KEY_CANCEL_)
                {
                        inRetVal = VS_USER_CANCEL;
			break;
                }
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
        }   
        /* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
	
	return (inRetVal);
}

/*
Function	:inMENU_ECR
Date&Time	:2016/6/23 上午 11:18
Describe	:ECR連線事件
*/
int inMENU_ECR(EventMenuItem *srEventMenuItem)
{
	inDISP_ClearAll();
	
        /* 第三層顯示 ＜ECR連線＞ */
	inDISP_PutGraphic(_ECR_CONNECTING_, 0, _COORDINATE_Y_LINE_8_4_);
        /* 這裡不設incode */

        /* 這裡不設incode */
        srEventMenuItem->inRunOperationID = _OPERATION_ECR_TRANSACTION_;
        /* 這裡不設TRT */
	
	/* ECR的Bit On起來，表示是收銀機發動交易 */
	srEventMenuItem->uszECRBit = VS_TRUE;

        return (VS_SUCCESS);
}

/*
Function	:inMENU_ECR_OPERATION_LOAD_KEY_FROM_520
Date&Time	:2017/11/15 上午 11:11
Describe	:ECR連線事件
*/
int inMENU_ECR_OPERATION_LOAD_KEY_FROM_520(EventMenuItem *srEventMenuItem)
{
	inDISP_ClearAll();
	
        /* 第三層顯示 ＜ECR連線＞ */
	inDISP_PutGraphic(_ECR_CONNECTING_, 0, _COORDINATE_Y_LINE_8_4_);
        /* 這裡不設incode */

        /* 這裡不設incode */
        srEventMenuItem->inRunOperationID = _OPERATION_LOAD_KEY_FROM_MASTER_;
        /* 這裡不設TRT */

        return (VS_SUCCESS);
}

/*
Function	:inMENU_ECR_OPERATION_LOAD_KEY_FROM_KEY_CARD
Date&Time	:2019/1/7 下午 5:36
Describe	:讀取TMK
*/
int inMENU_ECR_OPERATION_LOAD_KEY_FROM_KEY_CARD(EventMenuItem *srEventMenuItem)
{
	inDISP_ClearAll();
	
        /* 第三層顯示 ＜ECR連線＞ */
	inDISP_PutGraphic(_ECR_CONNECTING_, 0, _COORDINATE_Y_LINE_8_4_);
        /* 這裡不設incode */

        /* 這裡不設incode */
        srEventMenuItem->inRunOperationID = _OPERATION_LOAD_KEY_FROM_KEY_CARD_;
        /* 這裡不設TRT */

        return (VS_SUCCESS);
}

/*
Function	:inMENU_MULTIFUNC_SLAVE
Date&Time	:2017/6/30 下午 6:17
Describe	:當外接設備時
*/
int inMENU_MULTIFUNC_SLAVE(EventMenuItem *srEventMenuItem)
{
	inDISP_ClearAll();
	
        /* 第三層顯示 ＜ECR連線＞ */
//	inDISP_PutGraphic(_ECR_CONNECTING_, 0, _COORDINATE_Y_LINE_8X16_4_);
        /* 這裡不設incode */

        /* 這裡不設incode */
        srEventMenuItem->inRunOperationID = _OPERATION_MULTIFUNC_SLAVE_;
        /* 這裡不設TRT */
	
	srEventMenuItem->uszMultiFuncSlaveBit = VS_TRUE;
	
        return (VS_SUCCESS);
}

/*
Function	:inMENU_ESC_IDLE_UPLOAD
Date&Time	:2016/6/23 上午 11:18
Describe	:IDLE上傳ESC事件
*/
int inMENU_ESC_IDLE_UPLOAD(EventMenuItem *srEventMenuItem)
{
        /* 這裡不設incode */
        srEventMenuItem->inRunOperationID = _OPERATION_ESC_IDLE_UPLOAD_;
        /* 這裡不設TRT */
	
        return (VS_SUCCESS);
}

/*
Function	:inMENU_DCC_SCHEDULE
Date&Time	:2016/10/21 下午 3:18
Describe	:DCC排程下載
*/
int inMENU_DCC_SCHEDULE(EventMenuItem *srEventMenuItem)
{
        /* 這裡不設incode */
        srEventMenuItem->inRunOperationID = _OPERATION_DCC_SCHEDULE_;
        /* 這裡不設TRT */
	
        return (VS_SUCCESS);
}

/*
Function	:inMENU_TMS_DCC_SCHEDULE
Date&Time	:2016/10/21 下午 3:18
Describe	:TMS連動DCC下載
*/
int inMENU_TMS_DCC_SCHEDULE(EventMenuItem *srEventMenuItem)
{
        /* 這裡不設incode */
        srEventMenuItem->inRunOperationID = _OPERATION_TMS_DCC_SCHEDULE_;
        /* 這裡不設TRT */
	
        return (VS_SUCCESS);
}

/*
Function	:inMENU_DCC_HOUR_NOTIFY_
Date&Time	:2019/12/31 下午 2:31
Describe	:DCC整點下載提示
*/
int inMENU_DCC_HOUR_NOTIFY_(EventMenuItem *srEventMenuItem)
{
        /* 這裡不設incode */
        srEventMenuItem->inRunOperationID = _OPERATION_DCC_HOUR_NOTIFY_;
        /* 這裡不設TRT */
	
        return (VS_SUCCESS);
}

/*
Function	:inMENU_TMS_SCHEDULE_INQUIRE
Date&Time	:2016/12/6 下午 5:54
Describe	:TMS排程詢問
*/
int inMENU_TMS_SCHEDULE_INQUIRE(EventMenuItem *srEventMenuItem)
{
	/* 這裡不設incode */
        srEventMenuItem->inRunOperationID = _OPERATION_TMS_SCHEDULE_INQUIRE_;
        /* 這裡不設TRT */
	
	return (VS_SUCCESS);
}

/*
Function	:inMENU_TMS_SCHEDULE_DOWNLOAD
Date&Time	:2017/1/25 下午 3:09
Describe	:TMS排程下載
*/
int inMENU_TMS_SCHEDULE_DOWNLOAD(EventMenuItem *srEventMenuItem)
{
	/* 這裡不設incode */
        srEventMenuItem->inRunOperationID = _OPERATION_TMS_SCHEDULE_DOWNLOAD_;
        /* 這裡不設TRT */
	
	return (VS_SUCCESS);
}

/*
Function	:inMENU_TMS_PROCESS_EFFECTIVE
Date&Time	:2016/12/6 下午 5:54
Describe	:TMS IDLE檢查生效
*/
int inMENU_TMS_PROCESS_EFFECTIVE(EventMenuItem *srEventMenuItem)
{
	/* 這裡不設incode */
        srEventMenuItem->inRunOperationID = _OPERATION_TMS_PROCESS_EFFECTIVE_;
        /* 這裡不設TRT */
	
	return (VS_SUCCESS);
}

/*
Function	:inMENU_EDC_BOOTING
Date&Time	:2017/10/2 下午 5:31
Describe	:開機流程
*/
int inMENU_EDC_BOOTING(EventMenuItem *srEventMenuItem)
{
	/* 這裡不設incode */
#if defined	_LOAD_KEY_AP_
        srEventMenuItem->inRunOperationID = _OPERATION_EDC_LOAD_KEY_BOOTING_;
#else
	#if	_NEXSYS_APP_MODE_ == _NEXSYS_APP_MODE_SDK_
		srEventMenuItem->inRunOperationID = _OPERATION_EDC_SDK_INITIAL_BOOTING_;
	#else
		if (ginMachineType == _CASTLE_TYPE_V3UL_)
		{
			srEventMenuItem->inRunOperationID = _OPERATION_EDC_V3UL_BOOTING_;
		}
		else
		{
			srEventMenuItem->inRunOperationID = _OPERATION_EDC_BOOTING_;
		}
	#endif
#endif
        /* 這裡不設TRT */
	
	return (VS_SUCCESS);
}

/*
Function	:inMENU_POWER_MANAGEMENT
Date&Time	:2018/3/21 上午 11:01
Describe	:電量管理流程
*/
int inMENU_POWER_MANAGEMENT(EventMenuItem *srEventMenuItem)
{
	char	szPWMMode[2 + 1] = {0};
	
	memset(szPWMMode, 0x00, sizeof(szPWMMode));
	inGetPWMMode(szPWMMode);
	
	/* 這裡不設incode */
	if (memcmp(szPWMMode, _PWM_MODE_01_SLEEP_, strlen(_PWM_MODE_01_SLEEP_)) == 0)
	{
		srEventMenuItem->inRunOperationID = _OPERATION_POWER_SAVING_SLEEP_;
	}
	else if (memcmp(szPWMMode, _PWM_MODE_00_STANDBY_, strlen(_PWM_MODE_00_STANDBY_)) == 0)
	{
		srEventMenuItem->inRunOperationID = _OPERATION_POWER_SAVING_STANDBY_;
	}
	else
	{
		
	}
        /* 這裡不設TRT */
	
	return (VS_SUCCESS);
}

/*
Function        :inMENU_ETICKET_DEDUCT
Date&Time       :2017/10/27 下午 3:55
Describe        :
*/
int inMENU_ETICKET_DEDUCT(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	if (inNCCC_Ticket_Func_Check_Transaction_Deduct(_TICKET_DEDUCT_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_TICKET_DEDUCT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜購貨交易＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inCode = _TICKET_DEDUCT_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inRunOperationID = _OPERATION_TICKET_DEDUCT_;
		srEventMenuItem->inRunTRTID = FALSE;
		srEventMenuItem->uszESVCTransBit = VS_TRUE;
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_ETICKET_REFUND
Date&Time       :2017/12/19 下午 1:45
Describe        :
*/
int inMENU_ETICKET_REFUND(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	if (inNCCC_Ticket_Func_Check_Transaction_Refund(_TICKET_IPASS_REFUND_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_TICKET_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜退貨交易＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inCode = _TICKET_REFUND_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inRunOperationID = _OPERATION_TICKET_REFUND_;
		srEventMenuItem->inRunTRTID = FALSE;
		srEventMenuItem->uszESVCTransBit = VS_TRUE;
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_ETICKET_INQUIRY
Date&Time       :2017/10/27 下午 3:55
Describe        :
*/
int inMENU_ETICKET_INQUIRY(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	if (inNCCC_Ticket_Func_Check_Transaction_Inquiry(_TICKET_IPASS_INQUIRY_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_TICKET_INQUIRY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜餘額查詢＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inCode = _TICKET_INQUIRY_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inRunOperationID = _OPERATION_TICKET_INQUIRY_;
		srEventMenuItem->inRunTRTID = FALSE;
		srEventMenuItem->uszESVCTransBit = VS_TRUE;
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_ETICKET_TOP_UP
Date&Time       :2017/10/27 下午 3:55
Describe        :
*/
int inMENU_ETICKET_TOP_UP(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	if (inNCCC_Ticket_Func_Check_Transaction_Top_Up(_TICKET_IPASS_TOP_UP_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_TICKET_TOP_UP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜加值交易＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inCode = _TICKET_TOP_UP_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inRunOperationID = _OPERATION_TICKET_TOP_UP_;
		srEventMenuItem->inRunTRTID = FALSE;
		srEventMenuItem->uszESVCTransBit = VS_TRUE;
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_ETICKET_VOID_TOP_UP
Date&Time       :2017/12/19 下午 2:06
Describe        :
*/
int inMENU_ETICKET_VOID_TOP_UP(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	if (inNCCC_Ticket_Func_Check_Transaction_Void_Top_Up(_TICKET_IPASS_VOID_TOP_UP_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_TICKET_VOID_TOP_UP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜加值取消＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inCode = _TICKET_VOID_TOP_UP_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		srEventMenuItem->inRunOperationID = _OPERATION_TICKET_VOID_TOP_UP_;
		srEventMenuItem->inRunTRTID = FALSE;
		srEventMenuItem->uszESVCTransBit = VS_TRUE;
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_ETICKET_ACTIVATION_INQUIRY
Date&Time       :2020/6/24 下午 4:32
Describe        :
*/
int inMENU_ETICKET_ACTIVATION_INQUIRY(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);

	srEventMenuItem->inRunTRTID = FALSE;
	srEventMenuItem->uszESVCTransBit = VS_TRUE;
	
	/* 自助客製化不顯示 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, 3))
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Customer 111 don't display ESVC Activation");
		}
	}
	else
	{
		inNCCC_Ticket_Display_Inform_Activation_After_ESVC_SignOn(1,1);
	}
	
	return (inRetVal);
}

/*
Function	:inMENU_Loyalty_Redeem
Date&Time	:2017/2/2 下午 12:10
Describe	:優惠兌換
*/
int inMENU_Loyalty_Redeem(EventMenuItem *srEventMenuItem)
{
	int		inRetVal = VS_SUCCESS;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_8X16_OPT_;
	unsigned char	uszCreditCardSupport = VS_FALSE;
	unsigned char	uszBarcodeSupport = VS_FALSE;
	unsigned char	uszVoidRedeemSupport = VS_FALSE;
	unsigned char	uszASMSupport = VS_FALSE;
        unsigned char	uszKey = 0x00;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_LOYALTY_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜優惠兌換＞ */
		
	/* 判斷是否支援過卡兌換 */
	if (inNCCC_Loyalty_CreditCardFlag(_TRANS_TYPE_NULL_) == VS_SUCCESS)
	{
		uszCreditCardSupport = VS_TRUE;
	}
	else
	{
		uszCreditCardSupport = VS_FALSE;
	}
	
	/* 判斷是否支援條碼兌換 */
	if (inNCCC_Loyalty_BarCodeFlag(_TRANS_TYPE_NULL_) == VS_SUCCESS)
	{
		uszBarcodeSupport = VS_TRUE;
	}
	else
	{
		uszBarcodeSupport = VS_FALSE;
	}

	/* 判斷是否支援條碼兌換取消 */
	if (inNCCC_Loyalty_VoidRedeemFlag(_TRANS_TYPE_NULL_) == VS_SUCCESS)
	{
		uszVoidRedeemSupport = VS_TRUE;
	}
	else
	{
		uszVoidRedeemSupport = VS_FALSE;
	}
	
	/* 判斷是否支援優惠平台(含詢問電文) */
	if (inNCCC_Loyalty_ASM_Flag() == VS_SUCCESS)
	{
		uszASMSupport = VS_TRUE;
	}
	else
	{
		uszASMSupport = VS_FALSE;
	}
	
	/*	兌換方式
		‘1’=以條碼當作兌換資訊，透過收銀機條碼資訊。
		‘2’=以條碼當作兌換資訊，端末機接BarCode Reader掃描兌換(核銷)條碼。
		‘3’=以條碼當作兌換資訊，手動於端末機輸入兌換(核銷)條碼。
		‘4’=以卡號當作兌換資訊，於端末機上刷卡。
		‘5’=以卡號當作兌換資訊，於端末機上手動輸入。 
	 */
	
	/* 若全部開關沒開，直接顯示此功能以關閉 */
	if (uszCreditCardSupport != VS_TRUE && uszBarcodeSupport != VS_TRUE && uszVoidRedeemSupport != VS_TRUE && uszASMSupport != VS_TRUE)
	{
		srEventMenuItem->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;
		inRetVal = VS_USER_CANCEL;

		return (inRetVal);
	}

	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_LOYALTY_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜優惠兌換＞ */
	inDISP_PutGraphic(_MENU_LOYALTY_REDDEM_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);

	/* 設定Timeout */
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		uszKey = uszKBD_Key();
		
		/* 檢查TIMEOUT */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}

		/* 刷卡 */
		if (uszKey == _KEY_1_			||
		    inChoice == _OPTTouch8X16_LINE_5_)
		{
			if (uszCreditCardSupport != VS_TRUE)
			{
				srEventMenuItem->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;
				inRetVal = VS_ERROR;
				break;
			}
			
			inRetVal = inMENU_AWARD_SWIPE(srEventMenuItem);
			break;
		}
		/* 掃描條碼 */
		else if (uszKey == _KEY_2_			||
			 inChoice == _OPTTouch8X16_LINE_6_)
		{
			if (uszBarcodeSupport != VS_TRUE)
			{
				srEventMenuItem->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;
				inRetVal = VS_ERROR;
				break;
			}
			
			inRetVal = inMENU_AWARD_BARCODE(srEventMenuItem);
			break;
		}
		/* 兌換取消 */
		else if (uszKey == _KEY_3_			||
			 inChoice == _OPTTouch8X16_LINE_7_)
		{
			if (uszVoidRedeemSupport != VS_TRUE)
			{
				srEventMenuItem->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;
				inRetVal = VS_ERROR;
				break;
			}
			
			inRetVal = inMENU_AWARD_VOID(srEventMenuItem);
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
					
	return (inRetVal);
}

/*
Function        :inMENU_AWARD_SWIPE
Date&Time       :2017/10/30 上午 8:57
Describe        :
*/
int inMENU_AWARD_SWIPE(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	if (inNCCC_Loyalty_CreditCardFlag(_LOYALTY_REDEEM_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_LOYALTY_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜優惠兌換＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _LOYALTY_REDEEM_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}
		srEventMenuItem->inRunOperationID = _OPERATION_LOYALTY_REDEEM_CTLS_;
		srEventMenuItem->inRunTRTID = _TRT_LOYALTY_REDEEM_;
	}
				
	return (inRetVal);
}

/*
Function        :inMENU_AWARD_BARCODE
Date&Time       :2017/10/30 下午 1:19
Describe        :
*/
int inMENU_AWARD_BARCODE(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	if (inNCCC_Loyalty_BarCodeFlag(_LOYALTY_REDEEM_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_LOYALTY_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜優惠兌換＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _LOYALTY_REDEEM_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}
		srEventMenuItem->inRunOperationID = _OPERATION_BARCODE_;
		srEventMenuItem->inRunTRTID = _TRT_LOYALTY_REDEEM_;
	}
				
	return (inRetVal);
}

/*
Function        :inMENU_AWARD_VOID
Date&Time       :2017/10/30 下午 1:19
Describe        :
*/
int inMENU_AWARD_VOID(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	if (inNCCC_Loyalty_VoidRedeemFlag(_VOID_LOYALTY_REDEEM_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_VOID_LOYALTY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜兌換取消＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _VOID_LOYALTY_REDEEM_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}
		srEventMenuItem->inRunOperationID = _OPERATION_VOID_LOYALTY_REDEEM_;
		srEventMenuItem->inRunTRTID = _TRT_VOID_LOYALTY_REDEEM_;
	}
				
	return (inRetVal);
}

/*
Function        :inMENU_AWARD_REFUND
Date&Time       :2017/10/30 下午 1:19
Describe        :
*/
int inMENU_AWARD_REFUND(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	if (inNCCC_Loyalty_RefundFlag(_LOYALTY_REDEEM_REFUND_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_VOID_LOYALTY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜兌換取消＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _LOYALTY_REDEEM_REFUND_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}
		srEventMenuItem->inRunOperationID = _OPERATION_LOYALTY_REDEEM_REFUND_;
		srEventMenuItem->inRunTRTID = _TRT_LOYALTY_REDEEM_REFUND_;
	}
				
	return (inRetVal);
}

/*
Function	:inMENU_DCC_RATE
Date&Time	:2017/2/9 下午 12:01
Describe	:DCC匯率下載
*/
int inMENU_DCC_RATE(EventMenuItem *srEventMenuItem)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszKey;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_DCC_RATE_DOWNLOAD_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜DCC匯率下載＞ */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;

	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	srEventMenuItem->inRunOperationID = _OPERATION_FUN6_DCC_RATE_DOWNLOAD_;
	srEventMenuItem->inRunTRTID = FALSE;
	
	inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
	inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
	
	uszKey = 0;
	while (1)
	{

		uszKey = uszKBD_GetKey(30);

		if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (uszKey == _KEY_0_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}

	}
	
	return (inRetVal);
}

/*
Function	:inMENU_Download_Parameter
Date&Time	:2017/5/19 下午 6:34
Describe	:參數下載
*/
int inMENU_Download_Parameter(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char	szKey = 0x00;
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00 , sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_PARAM_DOWNLOAD_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 參數下載 */
	inDISP_PutGraphic(_MENU_PARA_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);
	
	/* 參數下載 */
	/* 設定Timeout */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
	}
	else
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	}

	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                szKey = uszKBD_Key();
		
		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}

		/* TMS參數下載 */
		if (szKey == _KEY_1_			||
		    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
		{
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TMS_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜TMS參數下載＞ */
			/* 輸入密碼的層級 */
			srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
			srEventMenuItem->inCode = FALSE;

			/* 第一層輸入密碼 */
			if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			{
				inRetVal = VS_ERROR;
				break;
			}

			srEventMenuItem->inRunOperationID = _OPERATION_FUN6_TMS_DOWNLOAD_;
			srEventMenuItem->inRunTRTID = FALSE;
			
			inRetVal = VS_SUCCESS;
			break;
		}
		/* DCC參數下載 */
		else if (szKey == _KEY_2_			||
		         inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
		{
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_DCC_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜DCC參數下載＞ */
			/* 輸入密碼的層級 */
			srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
			srEventMenuItem->inCode = FALSE;

			/* 第一層輸入密碼 */
			if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			{
				inRetVal = VS_ERROR;
				break;
			}

			srEventMenuItem->inRunOperationID = _OPERATION_FUN6_DCC_DOWNLOAD_;
			srEventMenuItem->inRunTRTID = FALSE;
			
			inRetVal = VS_SUCCESS;
			break;
		}
		/* DCC匯率下載 */
		else if (szKey == _KEY_3_			||
		         inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_)
		{
			inRetVal = inMENU_DCC_RATE(srEventMenuItem);
			break;
		}
		else if (szKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}

	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
		
	return (inRetVal);
}

/*
Function        :inMENU_VOID
Date&Time       :2017/10/26 上午 11:13
Describe        :
*/
int inMENU_VOID(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	if (inNCCC_Func_Check_Transaction_Function(_VOID_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _VOID_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inRunOperationID = _OPERATION_VOID_;
		srEventMenuItem->inRunTRTID = _TRT_VOID_;
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_REFUND
Date&Time       :2017/8/25 下午 6:07
Describe        :
*/
int inMENU_REFUND(EventMenuItem *srEventMenuItem)
{
	int			inRetVal = VS_SUCCESS;
	int			inChoice = 0;
	int			inTouchSensorFunc = _Touch_NONE_;
	char			szKey = 0x00;
	char			szFunEnable[2 + 1] = {0};
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_	, _TRANS_TYPE_NULL_	, inMENU_Check_HG_Enable		, _ICON_HIGHTLIGHT_1_2_2_HG_REFUND_		},
		{_Touch_NONE_				, _TRANS_TYPE_NULL_	, NULL					, ""						}
	};
	
	if (inNCCC_Func_Check_Transaction_Function(_REFUND_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		/* 第三層顯示 ＜退貨交易＞ */
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */
		
		/* HG沒開 */
		inHG_GetHG_Enable(0, szFunEnable);
		if (memcmp(szFunEnable, "Y", strlen("Y")) != 0)
		{
			inRetVal = inMENU_CREDIT_REFUND(srEventMenuItem);
			if (inRetVal == VS_USER_CANCEL)
			{
				inRetVal = VS_ERROR;
			}
			else if (inRetVal == VS_FUNC_CLOSE_ERR)
			{
				inRetVal = VS_SUCCESS;
			}
			else
			{
				/* 回傳inRetVal */
			}
		}
		/* HG有開 */
		else
		{
			inDISP_PutGraphic(_MENU_REFUND_OPTION_1_, 0, _COORDINATE_Y_LINE_8_4_);
			/* 檢查功能開關，並顯示反白的圖 */
			inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;

			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
			while (1)
			{
				inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
				szKey = uszKBD_Key();

				/* Timeout */
				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
				{
					szKey = _KEY_TIMEOUT_;
				}

				/* 信用卡退貨 */
				if (szKey == _KEY_1_			||
				    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
				{
					inRetVal = inMENU_CREDIT_REFUND(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						break;
					}
					else if (inRetVal == VS_FUNC_CLOSE_ERR)
					{
						inRetVal = VS_SUCCESS;
					}
					else
					{
						break;
					}
				}
				/* HappyGO退貨 */
				else if (szKey == _KEY_2_			||
					 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
				{
					inRetVal = inMENU_HG_REFUND(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						break;
					}
					else if (inRetVal == VS_FUNC_CLOSE_ERR)
					{
						inRetVal = VS_SUCCESS;
					}
					else
					{
						break;
					}
				}
				else if (szKey == _KEY_CANCEL_)
				{
					inRetVal = VS_USER_CANCEL;
					break;
				}
				else if (szKey == _KEY_TIMEOUT_)
				{
					inRetVal = VS_TIMEOUT;
					break;
				}

			}
			/* 清空Touch資料 */
			inDisTouch_Flush_TouchFile();
		}
		
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_CREDIT_REFUND
Date&Time       :2017/8/28 上午 11:09
Describe        :
*/
int inMENU_CREDIT_REFUND(EventMenuItem *srEventMenuItem)
{
	int			inRetVal = VS_SUCCESS;
	int			inChoice = 0;
	int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char			szKey = 0x00;
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	, _REFUND_		, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_1_2_1_1_REFUND_		},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_	, _REDEEM_REFUND_	, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_1_2_1_2_REDEEM_REFUND_	},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_	, _INST_REFUND_		, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_1_2_1_3_INST_REFUND_		},
		{_Touch_NONE_			, _TRANS_TYPE_NULL_	, NULL						, ""						}
	};
		
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_MENU_REFUND_OPTION_2_, 0, _COORDINATE_Y_LINE_8_4_);
	/* 檢查功能開關，並顯示反白的圖 */
	inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
	
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		szKey = uszKBD_Key();
		
		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}

		/* 一般退貨 */
		if (szKey == _KEY_1_			||
		    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
		{
			inRetVal = inMENU_REFUND_NORMAL(srEventMenuItem);
			if (inRetVal == VS_USER_CANCEL)
			{
				inRetVal = VS_ERROR;
				break;
			}
			else if (inRetVal == VS_FUNC_CLOSE_ERR)
			{
				inRetVal = VS_SUCCESS;
			}
			else
			{
				break;
			}
		}
		/* 紅利退貨 */
		else if (szKey == _KEY_2_			||
		         inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
		{
			inRetVal = inMENU_REFUND_REDEEM(srEventMenuItem);
			if (inRetVal == VS_USER_CANCEL)
			{
				inRetVal = VS_ERROR;
				break;
			}
			else if (inRetVal == VS_FUNC_CLOSE_ERR)
			{
				inRetVal = VS_SUCCESS;
			}
			else
			{
				break;
			}
		}
		/* 分期退貨 */
		else if (szKey == _KEY_3_			||
		         inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_)
		{
			inRetVal = inMENU_REFUND_INST(srEventMenuItem);
			if (inRetVal == VS_USER_CANCEL)
			{
				inRetVal = VS_ERROR;
				break;
			}
			else if (inRetVal == VS_FUNC_CLOSE_ERR)
			{
				inRetVal = VS_SUCCESS;
			}
			else
			{
				break;
			}
		}
		else if (szKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}

	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
	
	return (inRetVal);
}

/*
Function        :inMENU_REFUND_NORMAL
Date&Time       :2017/11/7 下午 5:29
Describe        :
*/
int inMENU_REFUND_NORMAL(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	char	szCTLSEnable[2 + 1];
	
	if (inNCCC_Func_Check_Transaction_Function(_REFUND_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
		inGetContactlessEnable(szCTLSEnable);

		inDISP_ClearAll();
		/* 第三層顯示 ＜退貨交易＞ */
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _REFUND_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}

		if (!memcmp(&szCTLSEnable[0], "Y", 1) && guszCTLSInitiOK == VS_TRUE)
		{
			srEventMenuItem->inRunOperationID = _OPERATION_REFUND_CTLS_;
			srEventMenuItem->inRunTRTID = _TRT_REFUND_;
		}
		else
		{
			srEventMenuItem->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_;
			srEventMenuItem->inRunTRTID = _TRT_REFUND_CTLS_;
		}
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_REFUND_REDEEM
Date&Time       :2017/11/7 下午 5:29
Describe        :
*/
int inMENU_REFUND_REDEEM(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	char	szCTLSEnable[2 + 1];
	
	if (inNCCC_Func_Check_Transaction_Function(_REDEEM_REFUND_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
		inGetContactlessEnable(szCTLSEnable);

		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利退貨＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _REDEEM_REFUND_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}

		if (!memcmp(&szCTLSEnable[0], "Y", 1) && guszCTLSInitiOK == VS_TRUE)
		{
			srEventMenuItem->inRunOperationID = _OPERATION_REDEEM_REFUND_CTLS_;
			srEventMenuItem->inRunTRTID = _TRT_REDEEM_REFUND_CTLS_;
		}
		else
		{
			srEventMenuItem->inRunOperationID = _OPERATION_REFUND_;
			srEventMenuItem->inRunTRTID = _TRT_REDEEM_REFUND_;
		}
		srEventMenuItem->uszRedeemBit = VS_TRUE;
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_REFUND_INST
Date&Time       :2017/11/7 下午 5:29
Describe        :
*/
int inMENU_REFUND_INST(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	char	szCTLSEnable[2 + 1];
	
	if (inNCCC_Func_Check_Transaction_Function(_INST_REFUND_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
		inGetContactlessEnable(szCTLSEnable);

		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期退貨＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _INST_REFUND_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}

		if (!memcmp(&szCTLSEnable[0], "Y", 1) && guszCTLSInitiOK == VS_TRUE)
		{
			srEventMenuItem->inRunOperationID = _OPERATION_INST_REFUND_CTLS_;
			srEventMenuItem->inRunTRTID = _TRT_INST_REFUND_CTLS_;
		}
		else
		{
			srEventMenuItem->inRunOperationID = _OPERATION_REFUND_;
			srEventMenuItem->inRunTRTID = _TRT_INST_REFUND_;
		}
		srEventMenuItem->uszInstallmentBit = VS_TRUE;
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_HG_REFUND
Date&Time       :2017/8/28 上午 11:17
Describe        :
*/
int inMENU_HG_REFUND(EventMenuItem *srEventMenuItem)
{
	int			inRetVal = VS_SUCCESS;
	int			inChoice = 0;
	int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char			szKey = 0x00;
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	, _HG_REWARD_REFUND_		, inHG_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_1_2_2_1_HG_REWARD_REFUND_	},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_	, _HG_REDEEM_REFUND_		, inHG_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_1_2_2_2_HG_REDEEM_REFUND_	},
		{_Touch_NONE_			, _TRANS_TYPE_NULL_		, NULL					, ""						}
	};
		
	if (inMENU_Check_HG_Enable(_HG_REWARD_REFUND_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_MENU_REFUND_OPTION_3_, 0, _COORDINATE_Y_LINE_8_4_);
		/* 檢查功能開關，並顯示反白的圖 */
		inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			/* 回饋退貨 */
			if (szKey == _KEY_1_			||
			    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
			{
				inRetVal = inMENU_HG_REWARD_REDUND(srEventMenuItem);
				if (inRetVal == VS_USER_CANCEL)
				{
					inRetVal = VS_ERROR;
					break;
				}
				else if (inRetVal == VS_FUNC_CLOSE_ERR)
				{
					inRetVal = VS_SUCCESS;
				}
				else
				{
					break;
				}
			}
			/* 扣抵退貨 */
			else if (szKey == _KEY_2_			||
				 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
			{
				inRetVal = inMENU_HG_REDEEM_REDUND(srEventMenuItem);
				if (inRetVal == VS_USER_CANCEL)
				{
					inRetVal = VS_ERROR;
					break;
				}
				else if (inRetVal == VS_FUNC_CLOSE_ERR)
				{
					inRetVal = VS_SUCCESS;
				}
				else
				{
					break;
				}
			}								
			else if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				break;
			}
		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_HG_REWARD_REDUND
Date&Time       :2017/11/8 下午 3:19
Describe        :
*/
int inMENU_HG_REWARD_REDUND(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	if (inHG_Func_Check_Transaction_Function(_HG_REWARD_REFUND_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡回饋退貨＞ */							

		srEventMenuItem->inCode = _HG_REWARD_REFUND_;
		srEventMenuItem->inRunOperationID = _OPERATION_HG_REFUND_;
		srEventMenuItem->lnHGTransactionType = _HG_REWARD_REFUND_;
		
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _HG_REWARD_REFUND_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_HG_REDEEM_REDUND
Date&Time       :2017/11/8 下午 3:19
Describe        :
*/
int inMENU_HG_REDEEM_REDUND(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	if (inHG_Func_Check_Transaction_Function(_HG_REDEEM_REFUND_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_HAPPYGO_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡扣抵退貨＞ */		

		srEventMenuItem->inCode = _HG_REDEEM_REFUND_;
		srEventMenuItem->inRunOperationID = _OPERATION_HG_REFUND_;
		srEventMenuItem->lnHGTransactionType = _HG_REDEEM_REFUND_;
		
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _HG_REDEEM_REFUND_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_INST_REDEEM
Date&Time       :2017/8/25 下午 6:00
Describe        :
*/
int inMENU_INST_REDEEM(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_8X16_OPT_;
	char	szKey = 0x00;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_REDEEM_INST_TITTLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期紅利＞ */
	inDISP_PutGraphic(_MENU_REDEEM_INST_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);

	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                szKey = uszKBD_Key();
		
		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}

		/* 一般紅利 */
		if (szKey == _KEY_1_			||
		    inChoice == _OPTTouch8X16_LINE_5_)
		{
			inRetVal = inMENU_REDEEM(srEventMenuItem);
			break;
		}
		/* 一般分期*/
		else if (szKey == _KEY_2_			||
		         inChoice == _OPTTouch8X16_LINE_6_)
		{
			inRetVal = inMENU_INST(srEventMenuItem);
			break;
		}
		else if (szKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}

	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
	
	return (inRetVal);
}

/*
Function        :inMENU_REDEEM
Date&Time       :2017/10/26 上午 11:36
Describe        :
*/
int inMENU_REDEEM(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	if (inNCCC_Func_Check_Transaction_Function(_REDEEM_SALE_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利扣抵＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _REDEEM_SALE_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}

		srEventMenuItem->inRunOperationID = _OPERATION_HG_I_R_;
		srEventMenuItem->inRunTRTID = _TRT_REDEEM_SALE_;
		srEventMenuItem->uszRedeemBit = VS_TRUE;
	}
			
	return (inRetVal);
}

/*
Function        :inMENU_INST
Date&Time       :2017/10/26 上午 11:36
Describe        :
*/
int inMENU_INST(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	if (inNCCC_Func_Check_Transaction_Function(_INST_SALE_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期付款＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _INST_SALE_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}

		srEventMenuItem->inRunOperationID = _OPERATION_HG_I_R_;
		srEventMenuItem->inRunTRTID = _TRT_INST_SALE_;
		srEventMenuItem->uszInstallmentBit = VS_TRUE;
	}
			
	return (inRetVal);
}

/*
Function        :inMENU_SETTLE
Date&Time       :2017/8/28 下午 1:45
Describe        :
*/
int inMENU_SETTLE(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_8X16_OPT_;
	char	szKey = 0x00;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SETTLE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜結帳交易＞ */
	/* 顯示連動結帳 OR 個別結帳 */
	inDISP_PutGraphic(_MENU_SETTLE_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);

	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                szKey = uszKBD_Key();
		
		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}
		
		/* 連動結帳 */
		if (szKey == _KEY_1_			||
		    inChoice == _OPTTouch8X16_LINE_5_)
		{
			inRetVal = inMENU_SETTLE_AUTOSETTLE(srEventMenuItem);
			break;
		}
		/* 個別結帳*/
		else if (szKey == _KEY_2_			||
			 inChoice == _OPTTouch8X16_LINE_6_)
		{
			inRetVal = inMENU_SETTLE_BY_HOST(srEventMenuItem);
			break;
		}
		else if (szKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}

	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
	
	return (inRetVal);
}

/*
Function        :inMENU_SETTLE_AUTOSETTLE
Date&Time       :2017/10/30 下午 2:14
Describe        :
*/
int inMENU_SETTLE_AUTOSETTLE(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SETTLE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜結帳交易＞ */
	
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
	srEventMenuItem->inCode = _SETTLE_;

	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
	{
		inRetVal = VS_ERROR;
	}

	srEventMenuItem->inRunOperationID = _OPERATION_SETTLE_;
	srEventMenuItem->inRunTRTID = _TRT_SETTLE_;

	/* 開啟連動結帳flag */
	srEventMenuItem->uszAutoSettleBit = VS_TRUE;
	
	return (inRetVal);
}

/*
Function        :inMENU_SETTLE_BY_HOST
Date&Time       :2017/10/30 下午 2:15
Describe        :
*/
int inMENU_SETTLE_BY_HOST(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SETTLE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜結帳交易＞ */
	
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
	srEventMenuItem->inCode = _SETTLE_;

	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
	{
		inRetVal = VS_ERROR;
	}

	srEventMenuItem->inRunOperationID = _OPERATION_SETTLE_;
	srEventMenuItem->inRunTRTID = _TRT_SETTLE_;
			
	return (inRetVal);
}

/*
Function        inMENU_SALEOFFLINE
Date&Time       :2017/10/26 上午 11:15
Describe        :
*/
int inMENU_SALEOFFLINE(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	if (inNCCC_Func_Check_Transaction_Function(_SALE_OFFLINE_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜交易補登＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _SALE_OFFLINE_;
		
		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inRunOperationID = _OPERATION_SALE_OFFLINE_;
		srEventMenuItem->inRunTRTID = _TRT_SALE_OFFLINE_;
	}
	
	return (inRetVal);
}

/*
Function        inMENU_TIP
Date&Time       :2017/10/26 上午 11:15
Describe        :
*/
int inMENU_TIP(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	

	if (inNCCC_Func_Check_Transaction_Function(_TIP_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_TIP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜小費交易＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _TIP_;
			
		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inRunOperationID = _OPERATION_TIP_;
		srEventMenuItem->inRunTRTID = _TRT_TIP_;
	}
					
	return (inRetVal);
}
/*
Function        :inMENU_PREAUTH
Date&Time       :2017/8/23 下午 5:08
Describe        :
*/
int inMENU_PREAUTH(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char	szKey = 0x00;
	char	szCTLSEnable[2 + 1];
	
	if (inNCCC_Func_Check_Transaction_Function(_PRE_AUTH_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
		inGetContactlessEnable(szCTLSEnable);

		inDISP_ClearAll();
		/* 第三層顯示 ＜預先授權＞ */
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權＞ */
		inDISP_PutGraphic(_MENU_PRE_AUTH_OPTION_1_, 0, _COORDINATE_Y_LINE_8_4_);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			/* 預先授權 */
			if (szKey == _KEY_1_			||
			    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權＞ */
				/* 輸入密碼的層級 */
				srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
				srEventMenuItem->inCode = _PRE_AUTH_;

				/* 第一層輸入密碼 */
				if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
				{
					inRetVal = VS_ERROR;
					break;
				}

				if (!memcmp(&szCTLSEnable[0], "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					srEventMenuItem->inRunOperationID = _OPERATION_PRE_AUTH_CTLS_;
					srEventMenuItem->inRunTRTID = _TRT_PRE_AUTH_CTLS_;
				}
				else
				{
					srEventMenuItem->inRunOperationID = _OPERATION_PRE_AUTH_;
					srEventMenuItem->inRunTRTID = _TRT_PRE_AUTH_;
				}

				inRetVal = VS_SUCCESS;
				break;
			}
			/* 授權完成 */
			else if (szKey == _KEY_2_			||
				 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜預先授權完成＞ */
				/* 輸入密碼的層級 */
				srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
				srEventMenuItem->inCode = _PRE_COMP_;

				/* 第一層輸入密碼 */
				if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
				{
					inRetVal = VS_ERROR;
					break;
				}

				if (!memcmp(&szCTLSEnable[0], "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					srEventMenuItem->inRunOperationID = _OPERATION_PRE_COMP_CTLS_;
					srEventMenuItem->inRunTRTID = _TRT_PRE_COMP_CTLS_;
				}
				else
				{
					srEventMenuItem->inRunOperationID = _OPERATION_PRE_COMP_;
					srEventMenuItem->inRunTRTID = _TRT_PRE_COMP_;
				}

				inRetVal = VS_SUCCESS;
				break;
			}
			else if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				break;
			}

		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_MAILORDER
Date&Time       :2017/10/26 上午 11:21
Describe        :
*/
int inMENU_MAILORDER(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	if (inNCCC_Func_Check_Transaction_Function(_MAIL_ORDER_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_MAIL_ORDER_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜郵購交易＞ */

		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _MAIL_ORDER_;
		
		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inRunOperationID = _OPERATION_MAIL_ORDER_;
		srEventMenuItem->inRunTRTID = _TRT_MAIL_ORDER_;
		srEventMenuItem->uszMailOrderBit = VS_TRUE;
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_SALE
Date&Time       :2017/10/26 上午 11:24
Describe        :
*/
int inMENU_SALE(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	/* 第三層顯示 ＜一般交易＞ */
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
	srEventMenuItem->inCode = _SALE_;

	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inRunOperationID = _OPERATION_SALE_CTLS_;
	srEventMenuItem->inRunTRTID = _TRT_SALE_;
	
	return (inRetVal);
}

/*
Function        :inMENU_ADJUST
Date&Time       :2017/8/23 下午 5:08
Describe        :
*/
int inMENU_ADJUST(EventMenuItem *srEventMenuItem)
{
	int			inRetVal = VS_SUCCESS;
	int			inChoice = 0;
	int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char			szKey= 0x00;
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	, _REDEEM_ADJUST_	, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_1_8_1_REDEEM_ADJUST_		},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_	, _INST_ADJUST_		, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_1_8_2_INST_ADJUST_		},
		{_Touch_NONE_			, _TRANS_TYPE_NULL_	, NULL						, ""						}
	};
	
	if (inNCCC_Func_Check_Transaction_Function(_INST_ADJUST_) != VS_SUCCESS	&&
	    inNCCC_Func_Check_Transaction_Function(_REDEEM_ADJUST_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_ADJUST_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜後台調帳＞ */
		inDISP_PutGraphic(_MENU_ADJUST_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);
		
		/* 檢查功能開關，並顯示反白的圖 */
		inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			/* 紅利調帳 */
			if (szKey == _KEY_1_			||
			    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
			{
				inRetVal = inMENU_REDEEM_ADJUST(srEventMenuItem);
				if (inRetVal == VS_USER_CANCEL)
				{
					inRetVal = VS_ERROR;
					break;
				}
				else if (inRetVal == VS_FUNC_CLOSE_ERR)
				{
					inRetVal = VS_SUCCESS;
				}
				else
				{
					break;
				}
			}
			/* 分期調帳 */
			else if (szKey == _KEY_2_			||
				 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
			{
				inRetVal = inMENU_INST_ADJUST(srEventMenuItem);
				if (inRetVal == VS_USER_CANCEL)
				{
					inRetVal = VS_ERROR;
					break;
				}
				else if (inRetVal == VS_FUNC_CLOSE_ERR)
				{
					inRetVal = VS_SUCCESS;
				}
				else
				{
					break;
				}
			}
			else if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				break;
			}

		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_REDEEM_ADJUST
Date&Time       :2017/11/8 上午 10:08
Describe        :
*/
int inMENU_REDEEM_ADJUST(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	if (inNCCC_Func_Check_Transaction_Function(_REDEEM_ADJUST_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REDEEM_ADJUST_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利調帳＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _REDEEM_ADJUST_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}

		srEventMenuItem->inRunOperationID = _OPERATION_REDEEM_ADJUST_;
		srEventMenuItem->inRunTRTID = _TRT_REDEEM_ADJUST_;
		srEventMenuItem->uszRedeemBit = VS_TRUE;
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_INST_ADJUST
Date&Time       :2017/11/8 上午 10:08
Describe        :
*/
int inMENU_INST_ADJUST(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	if (inNCCC_Func_Check_Transaction_Function(_INST_ADJUST_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_INST_ADJUST_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期調帳＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _INST_ADJUST_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}

		
		srEventMenuItem->inRunOperationID = _OPERATION_INST_ADJUST_;
		srEventMenuItem->inRunTRTID = _TRT_INST_ADJUST_;
		srEventMenuItem->uszInstallmentBit = VS_TRUE;
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_FISC_MENU
Date&Time       :2017/8/24 下午 3:33
Describe        :
*/
int inMENU_FISC_MENU(EventMenuItem *srEventMenuItem)
{
	int			inRetVal = VS_SUCCESS;
	int			inChoice = 0;
	int			inTouchSensorFunc = _Touch_8X16_OPT_;
	char			szKey = 0x00;
	char			szFiscFunctionEnable[2 + 1] = {0};
	char			szDemoMode[2 + 1] = {0};
	TRANSACTION_OBJECT	pobTran;			/* 只用來看有沒有安全認證，無實際用途 */
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SMARTPAY_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第一層顯示 Smartpay */

	/* 若CFGT的FiscFunctionEnable 和 MACEnable 未開，顯示此功能以關閉 */
	memset(szFiscFunctionEnable, 0x00, sizeof(szFiscFunctionEnable));
	inGetFiscFuncEnable(szFiscFunctionEnable);

	/* 沒開Fisc */
	if ((memcmp(&szFiscFunctionEnable[0], "Y", 1) != 0))
	{
		/* 此功能已關閉 */
		srEventMenuItem->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;

		return (VS_ERROR);
	}
	else
	{
		/* 教育訓練模式 */
		memset(szDemoMode, 0x00, sizeof(szDemoMode));
		inGetDemoMode(szDemoMode);
		if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
		{
			
		}
		else
		{
			/* SMARTPAY要GEN MAC來算TCC，一定要安全認證 */
			if (inKMS_CheckKey(_TWK_KEYSET_NCCC_, _TWK_KEYINDEX_NCCC_MAC_) != VS_SUCCESS)
			{
				memset(&pobTran, 0x00, sizeof(TRANSACTION_OBJECT));
				if (inNCCC_Func_CUP_PowerOn_LogOn(&pobTran) != VS_SUCCESS)
				{
					/* 安全認證失敗 */
					inRetVal = VS_ERROR;
					return (inRetVal);
				}
			}
		}
		
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_MENU_FISC_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);
	}

	
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                szKey = uszKBD_Key();
		
		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}

		/* 消費扣款 */
		if (szKey == _KEY_1_			||
		    inChoice == _OPTTouch8X16_LINE_5_)
		{
			inRetVal = inMENU_FISC_SALE(srEventMenuItem);
			break;
		}
		/* 消費扣款沖正 */
		else if (szKey == _KEY_2_			||
			 inChoice == _OPTTouch8X16_LINE_6_)
		{
			inRetVal = inMENU_FISC_VOID(srEventMenuItem);
			break;
		}
		/* 退費 */
		else if (szKey == _KEY_3_			||
			 inChoice == _OPTTouch8X16_LINE_7_)
		{
			inRetVal = inMENU_FISC_REFUND(srEventMenuItem);
			break;
		}
		else if (szKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}

	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
	
	return (inRetVal);
}

/*
Function        :inMENU_FISC_SALE
Date&Time       :2017/10/27 下午 3:13
Describe        :
*/
int inMENU_FISC_SALE(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	char	szFiscContactlessEnable[1 + 1] = {0};
	char	szCTLSEnable[1 + 1] = {0};
	
	if (inNCCC_Func_Check_Transaction_Function(_FISC_SALE_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
		inGetContactlessEnable(szCTLSEnable);
		memset(szFiscContactlessEnable, 0x00, sizeof(szFiscContactlessEnable));
		inGetSmartPayContactlessEnable(szFiscContactlessEnable);
		
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_FISC_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜消費扣款＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _FISC_SALE_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}

		if (!memcmp(szCTLSEnable, "Y", 1)	&&
		    !memcmp(szFiscContactlessEnable, "Y", 1)	&&
		    guszCTLSInitiOK == VS_TRUE)
		{
			srEventMenuItem->inRunOperationID = _OPERATION_FISC_SALE_CTLS_;
			srEventMenuItem->inRunTRTID = _TRT_FISC_SALE_CTLS_;
		}
		else
		{
			srEventMenuItem->inRunOperationID = _OPERATION_FISC_SALE_;
			srEventMenuItem->inRunTRTID = _TRT_FISC_SALE_ICC_;
		}
		
		srEventMenuItem->uszFISCTransBit = VS_TRUE;
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_FISC_VOID
Date&Time       :2017/10/27 下午 3:26
Describe        :
*/
int inMENU_FISC_VOID(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	if (inNCCC_Func_Check_Transaction_Function(_FISC_VOID_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_FISC_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜消費扣款沖正＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _FISC_VOID_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}

		srEventMenuItem->inRunOperationID = _OPERATION_VOID_;
		srEventMenuItem->inRunTRTID = _TRT_FISC_VOID_;
		srEventMenuItem->uszFISCTransBit = VS_TRUE;
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_FISC_REFUND
Date&Time       :2017/10/27 下午 3:26
Describe        :
*/
int inMENU_FISC_REFUND(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	char	szCTLSEnable[2 + 1];
	char	szFiscContactlessEnable[1 + 1] = {0};
	
	if (inNCCC_Func_Check_Transaction_Function(_FISC_REFUND_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
		inGetContactlessEnable(szCTLSEnable);
		memset(szFiscContactlessEnable, 0x00, sizeof(szFiscContactlessEnable));
		inGetSmartPayContactlessEnable(szFiscContactlessEnable);

		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_FISC_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜退費交易＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _FISC_REFUND_;
		srEventMenuItem->uszFISCTransBit = VS_TRUE;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}

		if (!memcmp(szCTLSEnable, "Y", 1)	&&
		    !memcmp(szFiscContactlessEnable, "Y", 1)	&&
		    guszCTLSInitiOK == VS_TRUE)
		{
			srEventMenuItem->inRunOperationID = _OPERATION_FISC_REFUND_CTLS_;
			srEventMenuItem->inRunTRTID = _TRT_FISC_REFUND_CTLS_;
		}
		else
		{
			srEventMenuItem->inRunOperationID = _OPERATION_FISC_REFUND_;
			srEventMenuItem->inRunTRTID = _TRT_FISC_REFUND_ICC_;
		}
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_FISC_MENU
Date&Time       :2017/8/24 下午 3:33
Describe        :
*/
int inMENU_HG_MENU(EventMenuItem *srEventMenuItem)
{
	int	inOrgIndex = 0;
	int	inRetVal = VS_SUCCESS;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_8X16_OPT_;
	char	szKey = 0x00;
	char	szHostEnable[2 + 1];
	
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_HAPPYGO_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜Happy go＞ */
	
	memset(szHostEnable, 0x00, sizeof(szHostEnable));
	inHG_GetHG_Enable(inOrgIndex, szHostEnable);
	if (memcmp(szHostEnable, "Y", strlen("Y")) != 0)
	{
		srEventMenuItem->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;
		
		return (VS_ERROR);
	}
	
	inDISP_PutGraphic(_MENU_HAPPYGO_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);

	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                szKey = uszKBD_Key();
		
		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}

		/* 紅利積點 */
		if (szKey == _KEY_1_			||
		    inChoice == _OPTTouch8X16_LINE_4_)
		{
			inRetVal = inMENU_HG_REWARD(srEventMenuItem);
			break;
		}
		/* 點數抵扣 */
		else if (szKey == _KEY_2_			||
			 inChoice == _OPTTouch8X16_LINE_5_)
		{
			inRetVal = inMENU_HG_ONLINE_REDEEM(srEventMenuItem);
			break;
		}
		/* 加價購 */
		else if (szKey == _KEY_3_			||
			 inChoice == _OPTTouch8X16_LINE_6_)
		{
			inRetVal = inMENU_HG_POINT_CERTAIN(srEventMenuItem);
			break;
		}
		/* 點數兌換 */
		else if (szKey == _KEY_4_			||
			 inChoice == _OPTTouch8X16_LINE_7_)
		{
			inRetVal = inMENU_HG_FULL_REDEMPTION(srEventMenuItem);
			break;
		}
		/* 點樹查詢 */
		else if (szKey == _KEY_5_			||
			 inChoice == _OPTTouch8X16_LINE_8_)
		{
			inRetVal = inMENU_HG_INQUIRY(srEventMenuItem);
			break;
		}
		else if (szKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}

	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();

	return (inRetVal);
}	

/*
Function        :inMENU_HG_REWARD
Date&Time       :2017/10/30 下午 1:53
Describe        :
*/
int inMENU_HG_REWARD(EventMenuItem *srEventMenuItem)
{
	char	szCustomerIndicator[3 + 1] = {0};
	int	inRetVal= VS_SUCCESS;
	
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
        inGetCustomIndicator(szCustomerIndicator);
        
        // Tusin_071
        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_071_FORCE_ONLINE_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inRetVal = VS_FUNC_CLOSE_ERR;
		}
		else if (inHG_Func_Check_Transaction_Function(_HG_REWARD_) != VS_SUCCESS)
		{
			inRetVal = VS_FUNC_CLOSE_ERR;
		}
        else
        {    
                inDISP_ClearAll();
                inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
                inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡紅利積點＞ */

                srEventMenuItem->inRunOperationID = _OPERATION_HG_;
                srEventMenuItem->lnHGTransactionType = _HG_REWARD_;
		}
        
	return (inRetVal);
}

/*
Function        :inMENU_HG_ONLINE_REDEEM
Date&Time       :2017/10/30 下午 1:53
Describe        :
*/
int inMENU_HG_ONLINE_REDEEM(EventMenuItem *srEventMenuItem)
{
	int	inRetVal= VS_SUCCESS;
	
	if (inHG_Func_Check_Transaction_Function(_HG_ONLINE_REDEEM_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_HAPPYGO_ONLINE_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡點數抵扣＞ */

		srEventMenuItem->inRunOperationID = _OPERATION_HG_;
		srEventMenuItem->lnHGTransactionType = _HG_ONLINE_REDEEM_;
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_HG_POINT_CERTAIN
Date&Time       :2017/10/30 下午 1:53
Describe        :
*/
int inMENU_HG_POINT_CERTAIN(EventMenuItem *srEventMenuItem)
{
	int	inRetVal= VS_SUCCESS;
	
	if (inHG_Func_Check_Transaction_Function(_HG_POINT_CERTAIN_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_HAPPYGO_POINT_CERTAIN_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡加價購＞ */

		srEventMenuItem->inRunOperationID = _OPERATION_HG_;
		srEventMenuItem->lnHGTransactionType = _HG_POINT_CERTAIN_;
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_HG_FULL_REDEMPTION
Date&Time       :2017/10/30 下午 1:53
Describe        :
*/
int inMENU_HG_FULL_REDEMPTION(EventMenuItem *srEventMenuItem)
{
	int	inRetVal= VS_SUCCESS;
	
	if (inHG_Func_Check_Transaction_Function(_HG_FULL_REDEMPTION_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_HAPPYGO_REDEMPTION_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡點數兌換＞ */

		srEventMenuItem->inRunOperationID = _OPERATION_HG_;
		srEventMenuItem->lnHGTransactionType = _HG_FULL_REDEMPTION_;
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_HG_INQUIRY
Date&Time       :2017/10/30 下午 1:53
Describe        :
*/
int inMENU_HG_INQUIRY(EventMenuItem *srEventMenuItem)
{
	int	inRetVal= VS_SUCCESS;
	
	if (inHG_Func_Check_Transaction_Function(_HG_INQUIRY_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_HAPPYGO_INQUIRY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡點數查詢＞ */

		srEventMenuItem->inRunOperationID = _OPERATION_HG_;
		srEventMenuItem->lnHGTransactionType = _HG_INQUIRY_;
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_CUP_Sale
Date&Time       :2017/10/26 下午 5:53
Describe        :
*/
int inMENU_CUP_SALE(EventMenuItem *srEventMenuItem)
{
	int	inRetVal= VS_SUCCESS;
	
	if (inNCCC_Func_Check_Transaction_Function(_CUP_SALE_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯一般交易＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _CUP_SALE_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inRunOperationID = _OPERATION_SALE_CTLS_;
		srEventMenuItem->inRunTRTID = _TRT_CUP_SALE_;
		/* 標示是CUP交易 */
		srEventMenuItem->uszCUPTransBit = VS_TRUE;
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_CUP_Void
Date&Time       :2017/10/26 下午 5:53
Describe        :
*/
int inMENU_CUP_VOID(EventMenuItem *srEventMenuItem)
{
	int	inRetVal= VS_SUCCESS;
	
	if (inNCCC_Func_Check_Transaction_Function(_CUP_VOID_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_CUP_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯取消＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _CUP_VOID_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inRunOperationID = _OPERATION_VOID_;
		srEventMenuItem->inRunTRTID = _TRT_CUP_VOID_;
		/* 標示是CUP交易 */
		srEventMenuItem->uszCUPTransBit = VS_TRUE;
	}
	
	return (inRetVal);
}

/*
Function	:inMENU_CUP_Refund
Date&Time	:2017/6/7 下午 1:12
Describe	:銀聯退貨
*/
int inMENU_CUP_REFUND(EventMenuItem *srEventMenuItem)
{
	int			inRetVal= VS_SUCCESS;
	int			inChoice = 0;
	int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char			szKey = 0x00;
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	, _CUP_REFUND_			, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_2_2_1_CUP_REFUND_		},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_	, _CUP_MAIL_ORDER_REFUND_	, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_2_2_2_CUP_MAILORDER_REFUND_	},
		{_Touch_NONE_					, _TRANS_TYPE_NULL_		, NULL						, ""						}
	};
	
	if (inNCCC_Func_Check_Transaction_Function(_CUP_REFUND_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_CUP_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯退貨> */
		inDISP_PutGraphic(_MENU_REFUND_OPTION_4_, 0,  _COORDINATE_Y_LINE_8_4_);
		
		/* 檢查功能開關，並顯示反白的圖 */
		inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			/* 銀聯退貨 */
			if (szKey == _KEY_1_			||
			    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
			{
				inRetVal = inMENU_CUP_REFUND_NORMAL(srEventMenuItem);
				if (inRetVal == VS_USER_CANCEL)
				{
					inRetVal = VS_ERROR;
					break;
				}
				else if (inRetVal == VS_FUNC_CLOSE_ERR)
				{
					inRetVal = VS_SUCCESS;
				}
				else
				{
					break;
				}
				
			}
			/* 銀聯郵購退貨 */
			else if (szKey == _KEY_2_			||
				 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
			{
				inRetVal = inMENU_CUP_REFUND_MAILORDER(srEventMenuItem);
				if (inRetVal == VS_USER_CANCEL)
				{
					inRetVal = VS_ERROR;
					break;
				}
				else if (inRetVal == VS_FUNC_CLOSE_ERR)
				{
					inRetVal = VS_SUCCESS;
				}
				else
				{
					break;
				}
			}
			else if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				break;
			}
			
		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();
	}

	return (inRetVal);
}

/*
Function        :inMENU_CUP_REFUND_NORMAL
Date&Time       :2017/11/7 下午 4:54
Describe        :
*/
int inMENU_CUP_REFUND_NORMAL(EventMenuItem *srEventMenuItem)
{
	int	inRetVal= VS_SUCCESS;
	char	szCTLSEnable[2 + 1];
	char	szCUPContactlessEnable[1 + 1] = {0};
	
	memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
	inGetContactlessEnable(szCTLSEnable);
	memset(szCUPContactlessEnable, 0x00, sizeof(szCUPContactlessEnable));
	inGetCUPContactlessEnable(szCUPContactlessEnable);
		
	if (inNCCC_Func_Check_Transaction_Function(_CUP_REFUND_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_CUP_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯退貨> */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _CUP_REFUND_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		if (!memcmp(szCTLSEnable, "Y", 1)		&&
		    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
		    guszCTLSInitiOK == VS_TRUE)
		{
			srEventMenuItem->inRunOperationID = _OPERATION_REFUND_CTLS_CUP_;
			srEventMenuItem->inRunTRTID = _TRT_CUP_REFUND_CTLS_;
		}
		else
		{
			srEventMenuItem->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_CUP_;
			srEventMenuItem->inRunTRTID = _TRT_CUP_REFUND_;
		}
		/* 標示是CUP交易 */
		srEventMenuItem->uszCUPTransBit = VS_TRUE;
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_CUP_REFUND_MAILORDER
Date&Time       :2017/11/7 下午 4:56
Describe        :
*/
int inMENU_CUP_REFUND_MAILORDER(EventMenuItem *srEventMenuItem)
{
	int	inRetVal= VS_SUCCESS;
	
	if (inNCCC_Func_Check_Transaction_Function(_CUP_MAIL_ORDER_REFUND_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_CUP_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯退貨> */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _CUP_MAIL_ORDER_REFUND_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inRunOperationID = _OPERATION_REFUND_;
		srEventMenuItem->inRunTRTID = _TRT_CUP_MAIL_ORDER_REFUND_;
		/* 標示是CUP交易 */
		srEventMenuItem->uszCUPTransBit = VS_TRUE;
		srEventMenuItem->uszMailOrderBit = VS_TRUE;
	}
	
	return (inRetVal);
}

/*
Function	:inMENU_CUP_PreAuth
Date&Time	:2017/6/7 下午 1:12
Describe	:銀聯預先授權
*/
int inMENU_CUP_PREAUTH(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char	szKey = 0x00;
	MENU_CHECK_2_TABLE_	srMenuChekDisplay[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_	, _CUP_PRE_AUTH_, _MENU_CHECK_FORCE_DISPLAY_NONE_	, inNCCC_Func_Check_Transaction_Function, 
		"預先",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_12_, VS_FALSE, 
		"授權  1",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_13_, VS_FALSE},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_	, _CUP_PRE_AUTH_VOID_, _MENU_CHECK_FORCE_DISPLAY_NONE_	, inNCCC_Func_Check_Transaction_Function, 
		"預先授權",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_2_, _COORDINATE_Y_LINE_24_12_, VS_FALSE, 
		"取消  2",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_2_, _COORDINATE_Y_LINE_24_13_, VS_FALSE},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_	, _CUP_PRE_COMP_, _MENU_CHECK_FORCE_DISPLAY_NONE_	, inNCCC_Func_Check_Transaction_Function, 
		"預先授權",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_3_, _COORDINATE_Y_LINE_24_12_, VS_FALSE, 
		"完成  3",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_3_, _COORDINATE_Y_LINE_24_13_, VS_FALSE},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_7_	, _CUP_PRE_COMP_VOID_, _MENU_CHECK_FORCE_DISPLAY_NONE_	, inNCCC_Func_Check_Transaction_Function, 
		"預先授權",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_16_, VS_FALSE, 
		"完成  ",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_17_, VS_FALSE,
		"取消  4",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_18_, VS_FALSE},
		{_Touch_NONE_}
	};

	if (inNCCC_Func_Check_Transaction_Function(_CUP_PRE_AUTH_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權＞ */
		/* 檢查功能開關，並顯示反白的圖 */
		inMENU_CHECK_2_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			/* 預先授權 */
			if (szKey == _KEY_1_			||
			    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
			{
				if (inNCCC_Func_Check_Transaction_Function(_CUP_PRE_AUTH_) != VS_SUCCESS)
				{
					inRetVal = VS_FUNC_CLOSE_ERR;
				}
				else
				{
					inDISP_ClearAll();
					inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
					inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權＞ */
					/* 輸入密碼的層級 */
					srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
					srEventMenuItem->inCode = _CUP_PRE_AUTH_;

					/* 第一層輸入密碼 */
					if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
					{
						inRetVal = VS_ERROR;
						break;
					}

					srEventMenuItem->inRunOperationID = _OPERATION_PRE_AUTH_CTLS_;
					srEventMenuItem->inRunTRTID = _TRT_CUP_PRE_AUTH_;
					/* 標示是CUP交易 */
					srEventMenuItem->uszCUPTransBit = VS_TRUE;

					inRetVal = VS_SUCCESS;
					break;
				}
			}
			/* 預先授權取消 */
			else if (szKey == _KEY_2_			||
				 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
			{
				if (inNCCC_Func_Check_Transaction_Function(_CUP_PRE_AUTH_VOID_) != VS_SUCCESS)
				{
					inRetVal = VS_FUNC_CLOSE_ERR;
				}
				else
				{
					inDISP_ClearAll();
					inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
					inDISP_PutGraphic(_MENU_PRE_AUTH_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權取消＞ */
					/* 輸入密碼的層級 */
					srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
					srEventMenuItem->inCode = _CUP_PRE_AUTH_VOID_;

					/* 第一層輸入密碼 */
					if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
					{
						inRetVal = VS_ERROR;
						break;
					}

					srEventMenuItem->inRunOperationID = _OPERATION_VOID_;
					srEventMenuItem->inRunTRTID = _TRT_CUP_PRE_AUTH_VOID_;
					/* 標示是CUP交易 */
					srEventMenuItem->uszCUPTransBit = VS_TRUE;

					inRetVal = VS_SUCCESS;
					break;
				}
			}
			/* 預先授權完成 */
			else if (szKey == _KEY_3_			||
				 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_)
			{
				if (inNCCC_Func_Check_Transaction_Function(_CUP_PRE_COMP_) != VS_SUCCESS)
				{
					inRetVal = VS_FUNC_CLOSE_ERR;
				}
				else
				{
					inDISP_ClearAll();
					inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
					inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜預先授權完成＞ */
					/* 輸入密碼的層級 */
					srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
					srEventMenuItem->inCode = _CUP_PRE_COMP_;

					/* 第一層輸入密碼 */
					if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
						return (VS_ERROR);

					srEventMenuItem->inRunOperationID = _OPERATION_PRE_COMP_CTLS_;
					srEventMenuItem->inRunTRTID = _TRT_CUP_PRE_COMP_;
					/* 標示是CUP交易 */
					srEventMenuItem->uszCUPTransBit = VS_TRUE;

					inRetVal = VS_SUCCESS;
					break;
				}
			}
			/* 授權完成 */
			else if (szKey == _KEY_4_			||
				 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_7_)
			{
				if (inNCCC_Func_Check_Transaction_Function(_CUP_PRE_COMP_VOID_) != VS_SUCCESS)
				{
					inRetVal = VS_FUNC_CLOSE_ERR;
				}
				else
				{
					inDISP_ClearAll();
					inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
					inDISP_PutGraphic(_MENU_PRE_COMP_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜預先授權完成取消＞ */
					/* 輸入密碼的層級 */
					srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
					srEventMenuItem->inCode = _CUP_PRE_COMP_VOID_;

					/* 第一層輸入密碼 */
					if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
					{
						inRetVal = VS_ERROR;
						break;
					}

					srEventMenuItem->inRunOperationID = _OPERATION_VOID_;
					srEventMenuItem->inRunTRTID = _TRT_CUP_PRE_COMP_VOID_;
					/* 標示是CUP交易 */
					srEventMenuItem->uszCUPTransBit = VS_TRUE;

					inRetVal = VS_SUCCESS;
					break;
				}
			}
			else if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				break;
			}

		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();
	}

        return (inRetVal);
}

/*
Function        :inMENU_CUP_MailOrder
Date&Time       :2017/10/26 下午 5:57
Describe        :
*/
int inMENU_CUP_MAILORDER(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	if (inNCCC_Func_Check_Transaction_Function(_CUP_MAIL_ORDER_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_MAIL_ORDER_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜郵購交易＞ */

		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _CUP_MAIL_ORDER_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inRunOperationID = _OPERATION_MAIL_ORDER_;
		srEventMenuItem->inRunTRTID = _TRT_CUP_MAIL_ORDER_;
		/* 標示是CUP交易 */
		srEventMenuItem->uszCUPTransBit = VS_TRUE;
		srEventMenuItem->uszMailOrderBit = VS_TRUE;
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_CUP_INST_REDEEM
Date&Time       :2017/10/26 下午 5:57
Describe        :
*/
int inMENU_CUP_INST_REDEEM(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_REDEEM_INST_TITTLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期紅利＞ */
	/* 此功能已關閉 */
	srEventMenuItem->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;
					
	return (inRetVal);
}

/*
Function        :inMENU_CUP_INST
Date&Time       :2017/10/27 下午 1:45
Describe        :
*/
int inMENU_CUP_INST(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	if (inNCCC_Func_Check_Transaction_Function(_CUP_INST_SALE_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期交易＞ */

		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _CUP_INST_SALE_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}

		srEventMenuItem->inRunOperationID = _OPERATION_INST_SALE_CTLS_;
		srEventMenuItem->inRunTRTID = _TRT_CUP_INST_SALE_CTLS_;
		srEventMenuItem->uszInstallmentBit = VS_TRUE;
		/* 標示是CUP交易 */
		srEventMenuItem->uszCUPTransBit = VS_TRUE;
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_CUP_REDEEM
Date&Time       :2017/10/27 下午 1:45
Describe        :
*/
int inMENU_CUP_REDEEM(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	if (inNCCC_Func_Check_Transaction_Function(_CUP_REDEEM_SALE_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利交易＞ */

		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _CUP_REDEEM_SALE_;
	
		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);
	
		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}
	
		srEventMenuItem->inRunOperationID = _OPERATION_REDEEM_SALE_CTLS_;
		srEventMenuItem->inRunTRTID = _TRT_CUP_REDEEM_SALE_CTLS_;
		srEventMenuItem->uszRedeemBit = VS_TRUE;
		/* 標示是CUP交易 */
		srEventMenuItem->uszCUPTransBit = VS_TRUE;
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_REPRINT
Date&Time       :2017/10/30 下午 2:39
Describe        :
*/
int inMENU_REPRINT(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_REPRINT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜重印簽單＞ */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;

	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	srEventMenuItem->inCode = FALSE;
	srEventMenuItem->inRunOperationID = _OPERATION_REPRINT_;
	srEventMenuItem->inRunTRTID = FALSE;
	
	return (inRetVal);
}

/*
Function        :inMENU_REVIEW
Date&Time       :2017/10/30 下午 4:36
Describe        :
*/
int inMENU_REVIEW(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <交易查詢> */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;

	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;
	srEventMenuItem->inRunOperationID = _OPERATION_REVIEW_;
	srEventMenuItem->inRunTRTID = FALSE;
	
	return (inRetVal);
}

/*
Function        :inMENU_TOTAL_REVIEW
Date&Time       :2017/10/30 下午 4:36
Describe        :總額查詢
*/
int inMENU_TOTAL_REVIEW(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <交易查詢> */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;

	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;
	srEventMenuItem->inRunOperationID = _OPERATION_REVIEW_TOTAL_;
	srEventMenuItem->inRunTRTID = FALSE;
	
	return (inRetVal);
}

/*
Function        :inMENU_DETAIL_REVIEW
Date&Time       :2017/10/30 下午 4:36
Describe        :明細查詢
*/
int inMENU_DETAIL_REVIEW(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <交易查詢> */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;

	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;
	srEventMenuItem->inRunOperationID = _OPERATION_REVIEW_DETAIL_;
	srEventMenuItem->inRunTRTID = FALSE;
	
	return (inRetVal);
}

/*
Function        :inMENU_TMS_PARAMETER_PRINT
Date&Time       :2017/10/30 下午 6:10
Describe        :
*/
int inMENU_TMS_PARAMETER_PRINT(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_PRINT_TMS_REPORT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜TMS參數列印＞ */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;
	/* 輸入管理號碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	/* 列印參數表 */
	srEventMenuItem->inRunOperationID = _OPERATION_FUN7_PRINT_;
	srEventMenuItem->inRunTRTID = FALSE;
	
	return (inRetVal);
}

/*
Function        :inMENU_DCC_PARAMETER_PRINT
Date&Time       :2017/10/30 下午 5:49
Describe        :
*/
int inMENU_DCC_PARAMETER_PRINT(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_PRINT_DCC_REPORT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜DCC參數列印＞ */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;
	/* 輸入管理號碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	/* 列印參數表 */
	srEventMenuItem->inRunOperationID = _OPERATION_FUN7_DCC_PRINT_;
	srEventMenuItem->inRunTRTID = FALSE;

	return (inRetVal);
}

/*
Function        :inMENU_TOTAL_REPORT
Date&Time       :2017/10/30 下午 4:30
Describe        :
*/
int inMENU_TOTAL_REPORT(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_TOTAIL_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 總額報表 */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;

	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;
	srEventMenuItem->inRunOperationID = _OPERATION_TOTAL_REPORT_;
	srEventMenuItem->inRunTRTID = FALSE;
	
	return (inRetVal);
}

/*
Function        :inMENU_DETAIL_REPORT
Date&Time       :2017/10/30 下午 4:34
Describe        :
*/
int inMENU_DETAIL_REPORT(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_DETAIL_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜明細報表＞ */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;

	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;
	srEventMenuItem->inRunOperationID = _OPERATION_DETAIL_REPORT_;
	srEventMenuItem->inRunTRTID = FALSE;
	
	return (inRetVal);
}

/*
Function        :inMENU_SAM_REGISTER
Date&Time       :2017/10/30 下午 4:53
Describe        :
*/
int inMENU_SAM_REGISTER(EventMenuItem *srEventMenuItem)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszKey = 0;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_TSAM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜註冊SAM卡＞ */

	inDISP_ChineseFont("1.NCCC TSAM註冊", _FONTSIZE_12X19_, _LINE_16_6_, _DISP_LEFT_);
	inDISP_ChineseFont("2.票證 SAM註冊", _FONTSIZE_12X19_, _LINE_16_7_, _DISP_LEFT_);

	uszKey = 0;

	while (1)
	{
		uszKey = uszKBD_GetKey(_EDC_TIMEOUT_);

		/* NCCC tSAM註冊 */
		if (uszKey == _KEY_1_)
		{
			/* 輸入密碼的層級 */
			srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
			srEventMenuItem->inCode = FALSE;

			/* 第一層輸入密碼 */
			if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
				return (VS_ERROR);

			srEventMenuItem->inRunOperationID = _OPERATION_TSAM_;
			srEventMenuItem->inRunTRTID = FALSE;
			break;
		}
		/* 票證SAM卡註冊 */
		else if (uszKey == _KEY_2_)
		{
			/* 輸入密碼的層級 */
			srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
			srEventMenuItem->inCode = FALSE;

			/* 第一層輸入密碼 */
			if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
				return (VS_ERROR);

			srEventMenuItem->inRunOperationID = _OPERATION_TICKET_SAM_REGISTER_;
			srEventMenuItem->inRunTRTID = FALSE;
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

	}
	
	return (inRetVal);
}

/*
Function        :inMENU_EDIT_PASSWORD
Date&Time       :2017/10/30 下午 5:25
Describe        :設定管理號碼
*/
int inMENU_EDIT_PASSWORD(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	char	szCustomIndicator[3 + 1] = {0};
	
	memset(szCustomIndicator, 0x00, sizeof(szCustomIndicator));
	inGetCustomIndicator(szCustomIndicator);
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SET_PWD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 設定管理號碼 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;
	/* 輸入管理號碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (!memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inRetVal = inMENU_EditPWD_Flow_Cus_075();
	}
	else
	{
		inRetVal = inFunc_EditPWD_Flow();
	}
	
	
	return (inRetVal);
}

/*
Function        :inMENU_TRACELOG_UP
Date&Time       :2017/10/30 下午 5:30
Describe        :
*/
int inMENU_TRACELOG_UP(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_RETURN_PARAM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 資訊回報 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;
	/* 輸入管理號碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);
	srEventMenuItem->inRunOperationID = _OPERATION_FUN6_TMS_TRACELOG_UP_;
	
	return (inRetVal);
}

/*
Function        :inMENU_TRACELOG_PRINT
Date&Time       :2019/3/5 下午 2:13
Describe        :
*/
int inMENU_TRACELOG_PRINT(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
        inDISP_ChineseFont("列印 NCCC Trace log", _FONTSIZE_8X33_, _LINE_8_1_, _DISP_CENTER_);
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;
	/* 輸入管理號碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);
	srEventMenuItem->inRunOperationID = _OPERATION_FUNC_TMS_TRACELOG_PRINT_;
	
	return (inRetVal);
}

/*
Function        :inMENU_CHECK_VERSION
Date&Time       :2017/10/30 下午 5:43
Describe        :
*/
int inMENU_CHECK_VERSION(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_AP_VERSION_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜版本查詢> */
	srEventMenuItem->inPasswordLevel =  _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;
	/* 輸入管理號碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);
	inRetVal = inFunc_Check_Version_ID();
	
	return (inRetVal);
}

/*
Function        :inMENU_TIME_SETTING
Date&Time       :2017/10/30 下午 6:02
Describe        :
*/
int inMENU_TIME_SETTING(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SET_DATE_TIME_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜時間設定＞ */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	srEventMenuItem->inCode = FALSE;
	/* 輸入管理號碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);
	inRetVal = inFunc_Fun3EditDateTime();
	
	return (inRetVal);
}

/*
Function        :inMENU_COMM_SETTING
Date&Time       :2017/10/30 下午 5:56
Describe        :
*/
int inMENU_COMM_SETTING(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SET_COMM_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 <通訊設定> */
	inRetVal = inCOMM_Fun3_SetCommWay();
	
	return (inRetVal);
}

/*
Function        :inMENU_UDP_SETTING
Date&Time       :2021/12/1 下午 2:35
Describe        :
*/
int inMENU_UDP_SETTING(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SET_COMM_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 <通訊設定> */
	inRetVal = inCOMM_Set_UDP();
	
	return (inRetVal);
}

/*
Function        :inMENU_TMS_PARAMETER_DOWNLOAD
Date&Time       :2017/10/30 下午 6:00
Describe        :
*/
int inMENU_TMS_PARAMETER_DOWNLOAD(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	/* TMS參數下載 */
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_TMS_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜TMS參數下載＞ */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	srEventMenuItem->inCode = FALSE;
	/* 輸入管理號碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	/* 參數下載 */
	srEventMenuItem->inRunOperationID = _OPERATION_FUN5_TMS_DOWNLOAD_;
	srEventMenuItem->inRunTRTID = FALSE;
	
	return (inRetVal);
}

/*
Function        :inMENU_DCC_PARAMETER_DOWNLOAD
Date&Time       :2017/10/30 下午 6:04
Describe        :
*/
int inMENU_DCC_PARAMETER_DOWNLOAD(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_DCC_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜DCC參數下載＞ */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	srEventMenuItem->inCode = FALSE;
	/* 輸入管理號碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	/* 參數下載 */
	srEventMenuItem->inRunOperationID = _OPERATION_FUN5_DCC_DOWNLOAD_;
	srEventMenuItem->inRunTRTID = FALSE;
	
	return (inRetVal);
}

/*
Function        :inMENU_TMS_TASK_REPORT
Date&Time       :2017/10/30 下午 6:07
Describe        :至現回報
*/
int inMENU_TMS_TASK_REPORT(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_TMS_TASK_MENU_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜至現回報＞ */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	srEventMenuItem->inCode = FALSE;
	/* 輸入管理號碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);
	srEventMenuItem->inRunOperationID = _OPERATION_FUN5_TMS_TASK_REPORT_;
	srEventMenuItem->inRunTRTID = FALSE;
	
	return (inRetVal);
}

/*
Function        :inMENU_DELETE_BATCH
Date&Time       :2018/5/22 下午 4:02
Describe        :清除批次
*/
int inMENU_DELETE_BATCH(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_RESET_BATCH_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <清除批次> */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	srEventMenuItem->inCode = FALSE;
	/* 輸入管理號碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);
	srEventMenuItem->inRunOperationID = _OPERATION_DELETE_BATCH_;
	srEventMenuItem->inRunTRTID = FALSE;
	
	return (inRetVal);
}

/*
Function        :inMENU_DEBUG_SWITCH
Date&Time       :2018/5/22 下午 4:14
Describe        :
*/
int inMENU_DEBUG_SWITCH(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_DEBUG_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜DEBUG開關＞ */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	inFunc_DebugSwitch();
	
	return (inRetVal);
}

/*
Function        :inMENU_EDIT_TMEP_VERSION_ID
Date&Time       :2018/5/22 下午 4:24
Describe        :
*/
int inMENU_EDIT_TMEP_VERSION_ID(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_EDIT_TMS_VERSION_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜修改TMS版本＞ */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);
	
	inFunc_Set_Temp_VersionID();
	
	return (inRetVal);
}

/*
Function        :inMENU_EDIT_TMSOK
Date&Time       :2018/5/22 下午 4:51
Describe        :
*/
int inMENU_EDIT_TMSOK(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_EDIT_TMS_VERSION_OK_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜修改TMS完成＞ */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_MANAGER_PASSWORD_;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	inFunc_Set_TMSOK_Flow();
	
	return (inRetVal);
}

/*
Function        :inMENU_UNLOCK_EDC
Date&Time       :2018/5/22 下午 4:56
Describe        :
*/
int inMENU_UNLOCK_EDC(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	/* 第三層顯示 ＜解鎖EDC＞ */
	inDISP_ChineseFont("解鎖EDC", _FONTSIZE_8X16_, _LINE_8_3_, _DISP_LEFT_);
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	inFunc_Unlock_EDCLock_Flow();
	
	return (inRetVal);
}

/*
Function        :inMENU_REBOOT
Date&Time       :2018/5/22 下午 5:14
Describe        :
*/
int inMENU_REBOOT(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_REBOOT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜重新開機＞ */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);
	srEventMenuItem->inRunOperationID = _OPERATION_EDC_REBOOT_;
	srEventMenuItem->inRunTRTID = FALSE;

	inFunc_Reboot();
	
	return (inRetVal);
}

/*
Function        :inMENU_KEY_FUNCTION
Date&Time       :2018/11/12 上午 11:09
Describe        :
*/
int inMENU_KEY_FUNCTION(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	char	szKey = 0x00;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_DOWNLOAD_CUP_KEY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜下載銀聯測試key＞ */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	while(1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont_Color("1.寫TMK", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
		inDISP_ChineseFont_Color("2.查詢Key", _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);
		inDISP_ChineseFont_Color("3.查看Master Key 狀態", _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_BLACK_, _DISP_LEFT_);
//		inDISP_ChineseFont_Color("", _FONTSIZE_8X22_, _LINE_8_8_, _COLOR_BLACK_, _DISP_LEFT_);
		
		szKey = uszKBD_GetKey(30);
		if (szKey == _KEY_1_)
		{
			inRetVal = inMENU_TMK_Menu(srEventMenuItem);
			if (inRetVal == VS_SUCCESS)
			{
				break;
			}
		}
		else if (szKey == _KEY_2_)
		{
			inKMS_GetKeyInfo_LookUp();
		}
		else if (szKey == _KEY_3_)
		{
			inNCCC_TMK_GetKeyInfo_LookUp_Default();
		}
		else if (szKey == _KEY_4_)
		{
			LoadAes256IpekTest();
                        DukptSpeedTestMain();
		}
		else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
		{
			break;
		}
		
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_TMK_Menu
Date&Time       :2018/11/12 上午 11:07
Describe        :
*/
int inMENU_TMK_Menu(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	char	szKey = 0x00;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_DOWNLOAD_CUP_KEY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜下載銀聯測試key＞ */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	while(1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont_Color("1.直接覆寫測試TMK", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
		inDISP_ChineseFont_Color("2.使用Key Card", _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);
		inDISP_ChineseFont_Color("3.和暫存Key交換", _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_BLACK_, _DISP_LEFT_);
		
		szKey = uszKBD_GetKey(30);
		if (szKey == _KEY_1_)
		{
			inNCCC_TMK_Write_Test_TMK_By_Terminal();
			break;
		}
		else if (szKey == _KEY_2_)
		{
			inRetVal = inMENU_TMK_By_KeyCard(srEventMenuItem);
			if (inRetVal == VS_SUCCESS)
			{
				break;
			}
		}
		else if (szKey == _KEY_3_)
		{
			inNCCC_TMK_ProductionKey_Swap_To_Temp();
		}
		else if (szKey == _KEY_4_)
		{
			
		}
		else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
		{
			break;
		}
		
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_TMK_By_KeyCard
Date&Time       :2018/11/12 上午 11:07
Describe        :
*/
int inMENU_TMK_By_KeyCard(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	char	szKey = 0x00;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_DOWNLOAD_CUP_KEY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜下載銀聯測試key＞ */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	while(1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont_Color("1.多把Key", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
		inDISP_ChineseFont_Color("2.多把Key被動接收", _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);
//		inDISP_ChineseFont_Color("", _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_BLACK_, _DISP_LEFT_);
		
		szKey = uszKBD_GetKey(30);
		if (szKey == _KEY_1_)
		{
			srEventMenuItem->inRunOperationID = _OPERATION_LOAD_KEY_FROM_KEY_CARD_;
			break;
		}
		else if (szKey == _KEY_2_)
		{
			
		}
		else if (szKey == _KEY_3_)
		{
			
		}
		else if (szKey == _KEY_4_)
		{
			
		}
		else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
		{
			break;
		}
		
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_EXIT_AP
Date&Time       :2018/5/22 下午 5:25
Describe        :
*/
int inMENU_EXIT_AP(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	/* 第三層顯示 ＜離開程式＞ */
	inDISP_ChineseFont("離開程式", _FONTSIZE_8X16_, _LINE_8_3_, _DISP_LEFT_);
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

        inXML_MemoryDump();
	inFunc_Exit_AP();
	
	return (inRetVal);
}

/*
Function        :inMENU_CHECK_FILE
Date&Time       :2018/5/22 下午 5:26
Describe        :
*/
int inMENU_CHECK_FILE(EventMenuItem *srEventMenuItem)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszKey = 0x00;
	unsigned char	uszDispBit = VS_FALSE;
	
	inDISP_ClearAll();
	/* 第三層顯示 ＜確認檔案＞ */
	inDISP_ChineseFont("AP File", _FONTSIZE_8X16_, _LINE_8_1_, _DISP_LEFT_);
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	while(1)
	{
		if (uszDispBit == VS_FALSE)
		{
			uszDispBit = VS_TRUE;
			inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
			inDISP_ChineseFont("1.From AP To SD", _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
			inDISP_ChineseFont("2.From AP To USB", _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
			inDISP_ChineseFont("3.From PUB To SD", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
			inDISP_ChineseFont("4.From PUB To USB", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		}
		
		uszKey = uszKBD_GetKey(30);
		if (uszKey == _KEY_1_)
		{
			inMENU_CHECK_FILE_In_SD(srEventMenuItem);
			uszDispBit = VS_FALSE;
		}
		else if (uszKey == _KEY_2_)
		{
			inMENU_CHECK_FILE_In_USB(srEventMenuItem);
			uszDispBit = VS_FALSE;
		}
		else if (uszKey == _KEY_3_)
		{
			inFunc_CheckFile_From_PUB_To_SD_ALL();
			uszDispBit = VS_FALSE;
		}
		else if (uszKey == _KEY_4_)
		{
			inFunc_CheckFile_From_PUB_To_USB_ALL();
			uszDispBit = VS_FALSE;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else
		{
			
		}
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_CHECK_FILE_In_SD
Date&Time       :2019/2/18 上午 9:49
Describe        :
*/
int inMENU_CHECK_FILE_In_SD(EventMenuItem *srEventMenuItem)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszKey = 0x00;
	unsigned char	uszDispBit = VS_FALSE;
	
	while(1)
	{
		if (uszDispBit == VS_FALSE)
		{
			uszDispBit = VS_TRUE;
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("SD", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
			inDISP_ChineseFont("1.Check Partial", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
			inDISP_ChineseFont("2.Check ALL", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
		}
		
		uszKey = uszKBD_GetKey(30);
		if (uszKey == _KEY_1_)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_6_);
			inFunc_CheckFile_In_SD_Partial();
			uszDispBit = VS_FALSE;
		}
		else if (uszKey == _KEY_2_)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_6_);
			inFunc_CheckFile_In_SD_ALL();
			uszDispBit = VS_FALSE;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else
		{
			
		}
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_CHECK_FILE_In_USB
Date&Time       :2019/2/18 上午 9:49
Describe        :
*/
int inMENU_CHECK_FILE_In_USB(EventMenuItem *srEventMenuItem)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszKey = 0x00;
	unsigned char	uszDispBit = VS_FALSE;
	
	while(1)
	{
		if (uszDispBit == VS_FALSE)
		{
			uszDispBit = VS_TRUE;
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("USB", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
			inDISP_ChineseFont("1.Check Partial", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
			inDISP_ChineseFont("2.Check ALL", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
		}
		
		uszKey = uszKBD_GetKey(30);
		if (uszKey == _KEY_1_)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_6_);
			inFunc_CheckFile_In_USB_Partial();
			uszDispBit = VS_FALSE;
		}
		else if (uszKey == _KEY_2_)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_6_);
			inFunc_CheckFile_In_USB_ALL();
			uszDispBit = VS_FALSE;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else
		{
			
		}
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_Engineer_Fuction
Date&Time       :2018/7/27 上午 9:31
Describe        :
*/
int inMENU_Engineer_Fuction(EventMenuItem *srEventMenuItem)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszKey = 0x00;
	
	inDISP_ClearAll();
	/* 第三層顯示 ＜確認檔案＞ */
	inDISP_ChineseFont("外勤工程師功能頁面", _FONTSIZE_8X16_, _LINE_8_1_, _DISP_LEFT_);
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	while(1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("1.Switch ECR Comport", _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
		inDISP_ChineseFont("2.Switch KBD Lock", _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
		inDISP_ChineseFont("3.Switch PWM Mode", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
	
		uszKey = uszKBD_GetKey(30);
		if (uszKey == _KEY_1_)
		{
			inFunc_ECR_Comport_Switch();
		}
		else if (uszKey == _KEY_2_)
		{
			inFunc_KBDLock_Switch();
		}
		else if (uszKey == _KEY_3_)
		{
			inFunc_PWM_Mode_Switch();
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			break;
		}
		else
		{
			
		}
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_Developer_Fuction_Flow
Date&Time       :2019/8/30 下午 4:08
Describe        :開發工程師頁面
*/
int inMENU_Developer_Fuction_Flow(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	int	inRetVal2 = VS_SUCCESS;
	int	inPage = 1;
	int	inMaxPage = 8;
	
	inDISP_ClearAll();
	inDISP_ChineseFont("開發工程師功能頁面", _FONTSIZE_8X16_, _LINE_8_1_, _DISP_LEFT_);
	
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);
	
	while (1)
	{
		
		inDISP_ClearAll();
		inDISP_ChineseFont("開發工程師功能頁面", _FONTSIZE_8X16_, _LINE_8_1_, _DISP_LEFT_);
	
		if (inPage == 1)
		{
			inRetVal2 = inMENU_Developer_Fuction(srEventMenuItem);
		}
		else if (inPage == 2)
		{
			inRetVal2 = inMENU_Developer_Fuction2(srEventMenuItem);
		}
		else if (inPage == 3)
		{
			inRetVal2 = inMENU_Developer_Fuction3(srEventMenuItem);
		}
		else if (inPage == 4)
		{
			inRetVal2 = inMENU_Developer_Fuction4(srEventMenuItem);
		}
		else if (inPage == 5)
		{
			inRetVal2 = inMENU_Developer_Fuction5(srEventMenuItem);
		}
		else if (inPage == 6)
		{
			inRetVal2 = inMENU_Developer_Fuction6(srEventMenuItem);
		}
		else if (inPage == 7)
		{
			inRetVal2 = inMENU_Developer_Fuction7(srEventMenuItem);
		}
		else if (inPage == 8)
		{
			inRetVal2 = inMENU_Developer_Fuction8(srEventMenuItem);
		}
		
		if (inRetVal2 == VS_LAST_PAGE)
		{
			if (inPage == 1)
			{
				inPage = 1;
			}
			else
			{
				inPage--;
			}
		}
		else if (inRetVal2 == VS_NEXT_PAGE)
		{
			if (inPage == inMaxPage)
			{
				inPage = inMaxPage;
			}
			else
			{
				inPage++;
			}
		}
		else
		{
			break;
		}
		
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_Developer_Fuction
Date&Time       :2019/8/6 下午 4:12
Describe        :開發工程師頁面
*/
int inMENU_Developer_Fuction(EventMenuItem *srEventMenuItem)
{
	int		inRetVal = VS_SUCCESS;
	int		inRetVal2 = VS_ERROR;
	unsigned char	uszKey = 0x00;
	
	while(1)
	{
		inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
		inDISP_ChineseFont("1.ESC上傳防呆測試", _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
		inDISP_ChineseFont("2.文件開啟上限測試", _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
		inDISP_ChineseFont("3.當前已開啟檔案數", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
		inDISP_ChineseFont("4.鎖機測試", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		inDISP_ChineseFont("5.SAM卡重新啟動", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
		inDISP_ChineseFont("6.上一頁", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
		inDISP_ChineseFont("7.下一頁", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
		uszKey = uszKBD_GetKey(30);
		if (uszKey == _KEY_1_)
		{
			inNCCC_Func_Test_ESC_Poka_yoke();
		}
		else if (uszKey == _KEY_2_)
		{
			int		i = 0;
			char		szFileName[20 + 1] = {0};
			unsigned long	ulHandle = 0;
			
			while (1)
			{
				memset(szFileName, 0x00, sizeof(szFileName));
				sprintf(szFileName, "test%04d.txt", i);
				
				inFILE_Create(&ulHandle, (unsigned char*)szFileName);
				inRetVal2 = inFILE_Open(&ulHandle, (unsigned char*)szFileName);
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, szFileName);
				}
				
				if (i >= 2048	||
				    inRetVal2 == VS_ERROR)
				{
					break;
				}
				
				i++;
			}
		}
		else if (uszKey == _KEY_3_)
		{
			int	inCnt = 0;
			
			inFile_Open_File_Cnt_Return(&inCnt);
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Open File Cnt = %d", inCnt);
			}
			
			char	szDebugMsg[100 + 1] = {0};

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open File Cnt = %d", inCnt);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
		}
		else if (uszKey == _KEY_4_)
		{
			inFunc_EDCLock(AT);
		}
		else if (uszKey == _KEY_5_)
		{
			inFunc_Menu_Sam_Slot_PowerOn();
		}
		else if (uszKey == _KEY_6_)
		{
			inRetVal = VS_LAST_PAGE;
			break;
		}
		else if (uszKey == _KEY_7_)
		{
			inRetVal = VS_NEXT_PAGE;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else
		{
			
		}
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_Developer_Fuction2
Date&Time       :2019/8/30 下午 3:23
Describe        :開發工程師頁面
*/
int inMENU_Developer_Fuction2(EventMenuItem *srEventMenuItem)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszKey = 0x00;

	while(1)
	{
		inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
		inDISP_ChineseFont("1.Select NCCC AID", _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
		inDISP_ChineseFont("2.測試AP更新", _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
		inDISP_ChineseFont("3.立即更新EMV參數", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
		inDISP_ChineseFont("4.立即更新EMVCL參數", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		inDISP_ChineseFont("5.快速設定AutoSignOn時間", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
		inDISP_ChineseFont("6.上一頁", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
		inDISP_ChineseFont("7.下一頁", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
		uszKey = uszKBD_GetKey(30);
		if (uszKey == _KEY_1_)
		{
			inFunc_Menu_Sam_Slot_Select_NCCC_AID();
			break;
		}
		else if (uszKey == _KEY_2_)
		{
			inNCCCTMS_Process_AP();
			break;
		}
		else if (uszKey == _KEY_3_)
		{
			/* 更新EMV參數 */
			inEMVXML_Update_EMV_XML(_EMV_CONFIG_FILENAME_, _EMV_EMVCL_DATA_PATH_);
			break;
		}
		else if (uszKey == _KEY_4_)
		{
			/* 更新EMVCL參數 */
			inEMVXML_Update_CTLS_XML(_EMVCL_CONFIG_FILENAME_, _EMV_EMVCL_DATA_PATH_);
			break;
		}
		else if (uszKey == _KEY_5_)
		{
			inFunc_Set_Auto_SignOn_Time_OneStep();
			break;
		}
		else if (uszKey == _KEY_6_)
		{
			inRetVal = VS_LAST_PAGE;
			break;
		}
		else if (uszKey == _KEY_7_)
		{
			inRetVal = VS_NEXT_PAGE;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else
		{
			
		}
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_Developer_Fuction3
Date&Time       :2019/12/19 下午 4:40
Describe        :開發工程師頁面
*/
int inMENU_Developer_Fuction3(EventMenuItem *srEventMenuItem)
{
	int		inRetVal = VS_SUCCESS;	
	unsigned char	uszKey = 0x00;

	while(1)
	{
		inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
		inDISP_ChineseFont("1.刪除DCC舊參數", _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
		inDISP_ChineseFont("2.電子票證API Version", _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
		inDISP_ChineseFont("3.刪除MAC Key", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
		inDISP_ChineseFont("4.顯示ECC Log容量", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		inDISP_ChineseFont("5.測試詢卡", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);		
		inDISP_ChineseFont("6.上一頁", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
		inDISP_ChineseFont("7.下一頁", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
		uszKey = uszKBD_GetKey(30);
		if (uszKey == _KEY_1_)
		{
			inNCCC_DCC_Clean_Parameter();
			break;
		}
		else if (uszKey == _KEY_2_)
		{
			inNCCC_Ticket_Get_API_Version();
			break;
		}
		else if (uszKey == _KEY_3_)
		{
			inKMS_DeleteKey(_TWK_KEYSET_NCCC_, _TWK_KEYINDEX_NCCC_MAC_);
			break;
		}
		else if (uszKey == _KEY_4_)
		{
			inECC_Check_ICER_LOG_SIZE();
			break;
		}
		else if (uszKey == _KEY_5_)
		{
			int	inTicket_Type = _TICKET_TYPE_NONE_;
			inTicketFunc_Check_Card_Flow(&inTicket_Type);
		}
		else if (uszKey == _KEY_6_)
		{
			inRetVal = VS_LAST_PAGE;
			break;
		}
		else if (uszKey == _KEY_7_)
		{
			inRetVal = VS_NEXT_PAGE;
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else
		{
			
		}
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_Developer_Fuction4
Date&Time       :2020/10/21 下午 5:48
Describe        :開發工程師頁面
*/
int inMENU_Developer_Fuction4(EventMenuItem *srEventMenuItem)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszKey = 0x00;

	while(1)
	{
		inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
		inDISP_ChineseFont("1.切換特殊情形", _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
		inDISP_ChineseFont("2.列印測試中文", _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
		inDISP_ChineseFont("3.列印測試英文", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
		inDISP_ChineseFont("4.列印測試圖片", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		inDISP_ChineseFont("5.顯示測試畫面", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);		
		inDISP_ChineseFont("6.上一頁", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
		inDISP_ChineseFont("7.下一頁", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
		uszKey = uszKBD_GetKey(30);
		if (uszKey == _KEY_1_)
		{
			inNCCC_Func_Switch_Special_Situation();
		}
		else if (uszKey == _KEY_2_)
		{
			inTEST_Print1();
		}
		else if (uszKey == _KEY_3_)
		{
			inTEST_Print2();
		}
		else if (uszKey == _KEY_4_)
		{
			inTEST_Print3();
		}
		else if (uszKey == _KEY_5_)
		{
			inTEST_DISPLAY();
		}
		else if (uszKey == _KEY_6_)
		{
			inRetVal = VS_LAST_PAGE;
			break;
		}
		else if (uszKey == _KEY_7_)
		{
			inRetVal = VS_NEXT_PAGE;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else
		{
			
		}
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_Developer_Fuction5
Date&Time       :2020/10/21 下午 5:48
Describe        :開發工程師頁面
*/
int inMENU_Developer_Fuction5(EventMenuItem *srEventMenuItem)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszKey = 0x00;

	while(1)
	{
		inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
		inDISP_ChineseFont("1.查看AP內檔案分佈", _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
		inDISP_ChineseFont("2.查看PUB內檔案分佈", _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
		inDISP_ChineseFont("3.清除PUB內內容", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
		inDISP_ChineseFont("4.清除ECC Log", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		inDISP_ChineseFont("5.EMV原廠debug", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);		
		inDISP_ChineseFont("6.上一頁", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
		inDISP_ChineseFont("7.下一頁", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
		uszKey = uszKBD_GetKey(30);
		if (uszKey == _KEY_1_)
		{
			inFunc_ls("-R -l", _AP_ROOT_PATH_);
		}
		else if (uszKey == _KEY_2_)
		{
			inFunc_ls("-R -l", _AP_PUB_PATH_);
		}
		else if (uszKey == _KEY_3_)
		{
			inFunc_Data_Delete("-r", "*", _AP_PUB_PATH_);
		}
		else if (uszKey == _KEY_4_)
		{
			inECC_Delete_Log(0, 0);
		}
		else if (uszKey == _KEY_5_)
		{
			inEMV_SetDebug();
		}
		else if (uszKey == _KEY_6_)
		{
			inRetVal = VS_LAST_PAGE;
			break;
		}
		else if (uszKey == _KEY_7_)
		{
			inRetVal = VS_NEXT_PAGE;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else
		{
			
		}
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_Developer_Fuction6
Date&Time       :2021/7/2 下午 5:14
Describe        :開發工程師頁面
*/
int inMENU_Developer_Fuction6(EventMenuItem *srEventMenuItem)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszKey = 0x00;

	while(1)
	{
		inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
		inDISP_ChineseFont("1.CTLS 原廠debug", _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
		inDISP_ChineseFont("2.立即更新ISO參數", _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
		inDISP_ChineseFont("3.立即更新FTPS參數", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
		inDISP_ChineseFont("4.解除ECR鎖定400", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		inDISP_ChineseFont("5.ECC HAL DEBUG", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);		
		inDISP_ChineseFont("6.上一頁", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
		inDISP_ChineseFont("7.下一頁", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
		uszKey = uszKBD_GetKey(30);
		if (uszKey == _KEY_1_)
		{
			inCTLS_SetDebug();
		}
		else if (uszKey == _KEY_2_)
		{
			TRANSACTION_OBJECT	pobTran;
			memset(&pobTran, 0x00, sizeof(TRANSACTION_OBJECT));
			pobTran.uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_NONE_;
			inNCCCTMS_UpdateParam_Flow(&pobTran);
			inNCCCTMS_Deal_With_Things_After_Parameter_Update(&pobTran);
			inFunc_Reboot();
		}
		else if (uszKey == _KEY_3_)
		{
			TRANSACTION_OBJECT	pobTran;
			memset(&pobTran, 0x00, sizeof(TRANSACTION_OBJECT));
			pobTran.uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_FTPS_;
			inNCCCTMS_UpdateParam_Flow(&pobTran);
			inNCCCTMS_Deal_With_Things_After_Parameter_Update(&pobTran);
			inFunc_Reboot();
		}
		else if (uszKey == _KEY_4_)
		{
			inFILE_Delete((unsigned char*)_144_To_400_CHANGED_FILE_NAME_);
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("刪除完成", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
			inDISP_Wait(1000);
		}
		else if (uszKey == _KEY_5_)
		{
			inECC_HAL_SetDebug();
		}
		else if (uszKey == _KEY_6_)
		{
			inRetVal = VS_LAST_PAGE;
			break;
		}
		else if (uszKey == _KEY_7_)
		{
			inRetVal = VS_NEXT_PAGE;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else
		{
			
		}
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_Developer_Fuction7
Date&Time       :2022/11/9 下午 2:50
Describe        :開發工程師頁面
*/
int inMENU_Developer_Fuction7(EventMenuItem *srEventMenuItem)
{
	int		inRetVal = VS_SUCCESS;
	char		szTemplate[50 + 1] = {0};
	unsigned char	uszKey = 0x00;

	while(1)
	{
		inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
		inDISP_ChineseFont("1.清除ECC ADV", _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
		inDISP_ChineseFont("2.切換ESC多送模擬功能", _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
		inDISP_ChineseFont("3.刪除ESC測試檔案", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
		inDISP_ChineseFont("4.顯示OS版本日期", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		inDISP_ChineseFont("5.sha1測試", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);		
		inDISP_ChineseFont("6.上一頁", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
		inDISP_ChineseFont("7.下一頁", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
		uszKey = uszKBD_GetKey(30);
		if (uszKey == _KEY_1_)
		{
			inFile_Unlink_File("ICERAPI_CMAS.adv", _ECC_FOLDER_PATH_);
		}
		else if (uszKey == _KEY_2_)
		{
			inNCCC_ESC_Test_Redundant_ESCPacket_Switch();
		}
		else if (uszKey == _KEY_3_)
		{
			inFile_Unlink_File(_ESC_FILE_RECEIPT_, _AP_ROOT_PATH_);
			inFile_Unlink_File(_ESC_FILE_RECEIPT_GZ_, _AP_ROOT_PATH_);
			inFile_Unlink_File(_ESC_FILE_BMP_GZ_, _AP_ROOT_PATH_);
			inFile_Unlink_File(_ESC_FILE_RECEIPT_GZ_ENCRYPTED_, _AP_ROOT_PATH_);
			inFile_Unlink_File(_ESC_FILE_BMP_GZ_ENCRYPTED_, _AP_ROOT_PATH_);
		}
		else if (uszKey == _KEY_4_)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inFunc_GetOSVersion((unsigned char*)szTemplate);
			inDISP_ClearAll();
			inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
			uszKBD_GetKey(30);
		}
		else if (uszKey == _KEY_5_)
		{
			unsigned long	ulRunTime = 0;
			unsigned long	ulSecond = 0;
			unsigned long	ulMilliSecond = 0;
			SHA_CTX         srSHA;
			unsigned char	uszHash[1000 + 1] = {0};
			char		szData[1000 + 1] = {0};
			char		szPrintData[100 + 1] = {0};
			int		inDataLen = 0;
			
			/* 600 */
			memset(uszHash, 0x00, sizeof(uszHash));
			memset(szData, 0x00, sizeof(szData));
			sprintf(szData, "UtWtxyVFI9G2xfCrs4tQf0iyE7LWKgRRDqqMzC6wuEv1NUzsWjWSaFbhcwMmsYQ6oWIZB8ljZY7k8iVTRI1Wk4c5qWOvzHuJXTKl8tQXlG3hXAEVhWhOgIkCRI63vAWNiUycAEhqxv9kEEOpVqJnIvQQ5JKGkUhF4FStaDzT8I5zCitBkQW7Jy28MSVY1yrAPMKZD5ZRmDZZve1s9sp6PKjeponR7Bum4dnllIrw1ytJjJKPng92fRo0bCAhIphWKMsTVIF1eLAffAL0Y4ZFfUYEQkkYkSOytY4VMFIVYAeEBhJw4YGDJJYPRzf00MwUgZwW1DNZhgtD6XEkNDjvzwmZUMSLXumj3h7UHwlWhTTKC5SDyMToRiPnsrf149ahtJauaXhRpiiBDNlovzlc9XvMIPzdmm5mlhRKwIqQPg58t24fPh32RicBSGewqs5BITAC4WavyI6oeU4hEcd2rdeTrP4dk2OoETHxxtrJ3blZPOaMRFEffbWCrekxOu6KOn4bYwEbKyhQnpT9CgFicvu8xtDxNKzUJiOvzWF0md1fiE1MgDW4MEG1oJhkAkQjyx1wiLTbFBChkE1Ok4vZSSun");
			inDataLen = strlen(szData);
			
			ulRunTime = ulFunc_CalculateRunTime_Start();
			/* 計算Hash */
			memset(&srSHA, 0x00, sizeof(SHA_CTX));
			/* Initialize the SHA_CTX structure and perpart for the SHA1 operation */
			CTOS_SHA1Init(&srSHA);
			/* Perform the SHA1 algorithm with the input data */
			CTOS_SHA1Update(&srSHA, (unsigned char *)szData, inDataLen);
			/* Finalize the SA1 operation and retrun the result */
			CTOS_SHA1Final(uszHash, &srSHA);
			inFunc_GetRunTime(ulRunTime, &ulSecond, &ulMilliSecond);
			
			memset(szPrintData, 0x00, sizeof(szPrintData));
			sprintf(szPrintData, "Len:%01d,time:%lu.%lu sec", inDataLen, ulSecond, ulMilliSecond);
			
			inPRINT_ChineseFont(szPrintData, _FONTSIZE_8X22_);
			
			/* 1000 */
			memset(uszHash, 0x00, sizeof(uszHash));
			memset(szData, 0x00, sizeof(szData));
			sprintf(szData, "aXKy3WjZjef8FiQY7J09nOSQY584kgcFjgkSRPDX1YSoz61aJpGeLQTpcGsQzRZAP60G9V9ym97AaheIel0noFYXb3D0FiiCTmp6duaoIUyoeeoPzZsoHqMqrJzoveRLIWl8vZw8J5yWQdjPaQXtZ2BGUY85NqIfWDMZHxixRKjhg1N2wwWchxG8cbjtSbbg4duo7hu3knsg05GqcqtYkg2oXxWtmAVMs3pVZvGYuwudyizLT71DUXB2ueS1rZsmNxUPJgdUfWurSf8SZtnMdMJ0XF1qQmL3stff3Fu6BBhvG9s0t0Ct4VZS9JVl3XQXYFsyaBoxvmOaaPfdflhkcBqWkFqYR1CiqkkFrMpmmdJl1nC1MCbbyZbcINBm9NJQhyIonuqYdwivm0s6nicdMJFbwrHVs051ON0oOZ71peQAtmIkNLJPn2kt6a773uwocwNPQ3bOEUoUNnVKPbSREyu5pyQyu6GVlgm94l5KSggXIlR69qQg0uFBoCCaQKeoDi6eNcbSWka24Xul7CgVjrVeB9dH4hou3mF9axiaV6t6SWxyxj9lGs9NhU8JM0phtJx7JZNG8BXWdUFwvGUSuTj8F9SRe6oh934vYuPSswm0aDn0RJW0FW8TYhFLcqhup5mNpkgPyukTk2FJD1mENVo7ffBGHxqTBNBtGzncmtKv3PYnmTAXPeSGGH38pyWLjdigt6u9Khs8d6oDbcp7q50saqchJ9tN1lXK8I5H6N4m2L01Ws7GDXWrB7dtVr59InTQEiqEQNGlwbRtPgfivJrAYwMlyZL0zbITMEwhGxmBCz6NrkIpZj0jMDxs5DYmRmfOlEiwf6syszCrV0kxOekmshV08VGNacKBPekZgNlUjdO55PIlOvJO48TeLIplpC5gEvlLlF4asoEoKodAYjK44cLPvOIBqZABzncX4ZaMugAKbp1WxmvPxwK6JFSmIx4ZvuMEaRXqYs40NA3NTv1xoj3ri7GjulPXxSss");
			inDataLen = strlen(szData);
			
			ulRunTime = ulFunc_CalculateRunTime_Start();
			/* 計算Hash */
			memset(&srSHA, 0x00, sizeof(SHA_CTX));
			/* Initialize the SHA_CTX structure and perpart for the SHA1 operation */
			CTOS_SHA1Init(&srSHA);
			/* Perform the SHA1 algorithm with the input data */
			CTOS_SHA1Update(&srSHA, (unsigned char *)szData, inDataLen);
			/* Finalize the SA1 operation and retrun the result */
			CTOS_SHA1Final(uszHash, &srSHA);
			inFunc_GetRunTime(ulRunTime, &ulSecond, &ulMilliSecond);
			
			memset(szPrintData, 0x00, sizeof(szPrintData));
			sprintf(szPrintData, "Len:%01d,time:%lu.%lu sec", inDataLen, ulSecond, ulMilliSecond);
			
			inPRINT_ChineseFont(szPrintData, _PRT_ISO_);
			
		}
		else if (uszKey == _KEY_6_)
		{
			inRetVal = VS_LAST_PAGE;
			break;
		}
		else if (uszKey == _KEY_7_)
		{
			inRetVal = VS_NEXT_PAGE;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else
		{
			
		}
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_Developer_Fuction8
Date&Time       :2023/7/5 下午 4:10
Describe        :開發工程師頁面
*/
int inMENU_Developer_Fuction8(EventMenuItem *srEventMenuItem)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszKey = 0x00;
		
	while(1)
	{
		inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
		inDISP_ChineseFont("1.刪除憑證檔", _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
		inDISP_ChineseFont("2.憑證資料檢查", _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
		inDISP_ChineseFont("3.設定檢查卡功能", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
		inDISP_ChineseFont("4.--", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		inDISP_ChineseFont("5.--", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);		
		inDISP_ChineseFont("6.上一頁", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
		inDISP_ChineseFont("7.下一頁", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
		uszKey = uszKBD_GetKey(30);
		if (uszKey == _KEY_1_)
		{
			#include "../COMM/TLS.h"
//			char	szPath[200 + 1] = {0};
//			memset(szPath, 0x00, sizeof(szPath));
//			sprintf(szPath, "%s/%s", _CA_DIR_NAME_, _PEM_IFES_FILE_NAME_);
//			inFILE_Delete((unsigned char*)szPath);
//			memset(szPath, 0x00, sizeof(szPath));
//			sprintf(szPath, "%s/%s", _CA_DIR_NAME_, _PEM_LDSS_FILE_NAME_);
//			inFILE_Delete((unsigned char*)szPath);
			inFunc_Data_Delete("", "*", _CA_DATA_PATH_);
		}
		else if (uszKey == _KEY_2_)
		{
			vdTLS_print_certificate_info(_CA_DATA_PATH_ _PEM_NEW_TLS_FILE_NAME_);
			vdTLS_print_certificate_info(_CA_DATA_PATH_ _PEM_PRESERVE_TLS_FILE_NAME_);			
			vdTLS_print_certificate_info(_CA_DATA_PATH_ _PEM_IFES_DEMO_FILE_NAME_);
			vdTLS_print_certificate_info(_CA_DATA_PATH_ _PEM_NEW_TLS_BACKUP_FILE_NAME_);
			
			inPRINT_SpaceLine(4);
		}
		else if (uszKey == _KEY_3_)
		{
			inFunc_SwitchCardRetentionFlag();
		}
		else if (uszKey == _KEY_4_)
		{
			
		}
		else if (uszKey == _KEY_5_)
		{
			
		}
		else if (uszKey == _KEY_6_)
		{
			inRetVal = VS_LAST_PAGE;
			break;
		}
		else if (uszKey == _KEY_7_)
		{
			inRetVal = VS_NEXT_PAGE;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else
		{
			
		}
	}
	
	return (inRetVal);
}

/*
Function	:inMENU_ESVC_AUTO_SIGNON
Date&Time	:2019/1/22 下午 5:22
Describe	:
*/
int inMENU_ESVC_AUTO_SIGNON(EventMenuItem *srEventMenuItem)
{
	/* 電子票證 */
	inDISP_ClearAll();
	inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);
	inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);
		
	srEventMenuItem->inRunOperationID = _OPERATION_ESVC_AUTO_SIGNON_;
        /* 這裡不設TRT */
	
	return (VS_SUCCESS);
}

/*
Function        :inMENU_COMM_MENU
Date&Time       :2021/11/29 上午 11:43
Describe        :
*/
int inMENU_COMM_MENU(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_NEWUI_CHOOSE_HOST_;
	char	szKey = 0x00;
	char	szLine1[20 + 1] = {0};
	char	szLine1_2[20 + 1] = {0};
	char	szLine2[20 + 1] = {0};
	char	szLine2_2[20 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	BYTE	bReDisplayBit = VS_FALSE;
	
	memset(szCustomerIndicator, 0x00 , sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SET_COMM_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 <通訊設定> */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	srEventMenuItem->inCode = FALSE;
	/* 輸入管理號碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);
	
	bReDisplayBit = VS_TRUE;
	
	while (1)
	{
		if (bReDisplayBit == VS_TRUE)
		{
			/* 顯示通訊設定 收銀機連線 */
			memset(szLine1, 0x00, sizeof(szLine1));
			memset(szLine1_2, 0x00, sizeof(szLine1_2));
			memset(szLine2, 0x00, sizeof(szLine2));
			memset(szLine2_2, 0x00, sizeof(szLine2_2));
			sprintf(szLine1, "通訊");
			sprintf(szLine1_2, "設定1");
			sprintf(szLine2, "收銀機");
			sprintf(szLine2_2, "連線2");

			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_CHOOSE_HOST_2_, 0, _COORDINATE_Y_LINE_8_4_);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine1, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_9_, VS_FALSE);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine1_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_10_, VS_FALSE);

			inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_9_, VS_FALSE);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine2_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_10_, VS_FALSE);
			/* 設定Timeout */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
			}
			else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
			}
			else
			{
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
			}
			bReDisplayBit = VS_FALSE;
		}
		
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                szKey = uszKBD_Key();
		
		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}
		
		/* 通訊設定 */
		if (szKey == _KEY_1_			||
		    inChoice == _NEWUI_CHOOSE_HOST_Touch_HOST_1_)
		{
			inRetVal = inMENU_COMM_SETTING(srEventMenuItem);
			if (inRetVal == VS_USER_CANCEL)
			{
				/* 按X可停留在這頁 */
				bReDisplayBit = VS_TRUE;
			}
			else
			{
				break;
			}
		}
		/* 收銀機連線 */
		else if (szKey == _KEY_2_			||
			 inChoice == _NEWUI_CHOOSE_HOST_Touch_HOST_2_)
		{
			inRetVal = inMENU_UDP_SETTING(srEventMenuItem);
			if (inRetVal == VS_USER_CANCEL)
			{
				/* 按X可停留在這頁 */
				bReDisplayBit = VS_TRUE;
			}
			else
			{
				break;
			}
		}
		else if (szKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}

	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
	
	return (inRetVal);
}

/*
Function        :inMENU_Check_Adjust
Date&Time       :2017/11/6 下午 5:32
Describe        :檢查是否全部調帳功能都不能用
*/
int inMENU_Check_Adjust(int inCode)
{
	char		szTransFunc[20 + 1];
	unsigned char	uszTxnEnable = VS_TRUE;
	
	/* 只檢查NCCC的功能 */
	inLoadHDTRec(0);
		
	memset(szTransFunc, 0x00, sizeof(szTransFunc));
	if (inGetTransFunc(szTransFunc) != VS_SUCCESS)
		return (VS_ERROR);

	
	if ((memcmp(&szTransFunc[9], "Y", 1) != 0)	&&
	    (memcmp(&szTransFunc[10], "Y", 1) != 0))
	{
		uszTxnEnable = VS_FALSE;
	}
	
	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inMENU_Check_Transaction_Enable
Date&Time       :2018/2/5 下午 2:42
Describe        :
*/
int inMENU_Check_Transaction_Enable(int inCode)
{
	char		szFunEnable[2 + 1];
	unsigned char	uszTxnEnable = VS_TRUE;
	
	memset(szFunEnable, 0x00, sizeof(szFunEnable));
	inGetTMSOK(szFunEnable);
	
	/* 沒下TMS */
	if (memcmp(&szFunEnable[0], "Y", 1) != 0)
	{
		uszTxnEnable = VS_FALSE;
	}
	/* 有下TMS */
	else
	{
		uszTxnEnable = VS_TRUE;
	}
	
	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inMENU_Check_CUP_Enable
Date&Time       :2017/11/3 下午 2:42
Describe        :
*/
int inMENU_Check_CUP_Enable(int inCode)
{
	char		szCUPFunctionEnable[2 + 1] = {0};
	char		szMACEnable[2 + 1] = {0};
	char		szTMSOK[2 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned char	uszTxnEnable = VS_TRUE;
	
	/* 若EDC的CUPFunctionEnable 和 MACEnable 未開，顯示此功能以關閉 */
	memset(szCUPFunctionEnable, 0x00, sizeof(szCUPFunctionEnable));
	inGetCUPFuncEnable(szCUPFunctionEnable);
	memset(szMACEnable, 0x00, sizeof(szMACEnable));
	inGetMACEnable(szMACEnable);
	
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_042_BDAU1_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
	{
		uszTxnEnable = VS_FALSE;
	}
	else
	{
		/* 沒開CUP */
		if ((memcmp(&szCUPFunctionEnable[0], "Y", 1) != 0)	|| 
		    (memcmp(szMACEnable, "Y", 1) != 0)			||
		    (memcmp(szTMSOK, "Y", 1) != 0))
		{
			uszTxnEnable = VS_FALSE;
		}
		/* 有開CUP */
		else
		{
			uszTxnEnable = VS_TRUE;
		}
	}
	
	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inMENU_Check_SMARTPAY_Enable
Date&Time       :2017/11/3 下午 2:42
Describe        :
*/
int inMENU_Check_SMARTPAY_Enable(int inCode)
{
	char		szFiscFunctionEnable[2 + 1];
	char		szTMSOK[2 + 1];
	unsigned char	uszTxnEnable = VS_TRUE;
	
	/* 若CFGT的FiscFunctionEnable 和 MACEnable 未開，顯示此功能以關閉 */
	memset(szFiscFunctionEnable, 0x00, sizeof(szFiscFunctionEnable));
	inGetFiscFuncEnable(szFiscFunctionEnable);
	
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);

	/* 沒開Fisc */
	if ((memcmp(&szFiscFunctionEnable[0], "Y", 1) != 0)	||
	    (memcmp(szTMSOK, "Y", 1) != 0))
	{
		uszTxnEnable = VS_FALSE;
	}
	else
	{
		uszTxnEnable = VS_TRUE;
	}
	
	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inMENU_Check_ETICKET_Enable
Date&Time       :2017/11/3 下午 2:42
Describe        :inCode沒有使用
*/
int inMENU_Check_ETICKET_Enable(int inCode)
{
	int		inESVCIndex = -1;
	char		szETICKETEnable[2 + 1];
	char		szTMSOK[2 + 1];
	unsigned char	uszTxnEnable = VS_FALSE;
	
	inFunc_Find_Specific_HDTindex(0, _HOST_NAME_ESVC_, &inESVCIndex);
	
	if (inESVCIndex != -1)
	{
		memset(szTMSOK, 0x00, sizeof(szTMSOK));
		inGetTMSOK(szTMSOK);

		if (memcmp(szTMSOK, "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
		else
		{
			inLoadHDTRec(inESVCIndex);
			memset(szETICKETEnable, 0x00, sizeof(szETICKETEnable));
						
			if (uszTxnEnable == VS_FALSE)
			{
				inLoadTDTRec(_TDT_INDEX_00_IPASS_);
				memset(szETICKETEnable, 0x00, sizeof(szETICKETEnable));
				inGetTicket_HostEnable(szETICKETEnable);
				if (memcmp(szETICKETEnable, "Y", strlen("Y")) == 0)
				{
					uszTxnEnable = VS_TRUE;
				}
			}
			
			if (uszTxnEnable == VS_FALSE)
			{
				inLoadTDTRec(_TDT_INDEX_01_ECC_);
				memset(szETICKETEnable, 0x00, sizeof(szETICKETEnable));
				inGetTicket_HostEnable(szETICKETEnable);
				if (memcmp(szETICKETEnable, "Y", strlen("Y")) == 0)
				{
					uszTxnEnable = VS_TRUE;
				}
			}
			
			if (uszTxnEnable == VS_FALSE)
			{
				inLoadTDTRec(_TDT_INDEX_02_ICASH_);
				memset(szETICKETEnable, 0x00, sizeof(szETICKETEnable));
				inGetTicket_HostEnable(szETICKETEnable);
				if (memcmp(szETICKETEnable, "Y", strlen("Y")) == 0)
				{
					uszTxnEnable = VS_TRUE;
				}
			}
		}
	}
	else
	{
		uszTxnEnable = VS_FALSE;
	}
	
	inLoadHDTRec(0);
	
	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inMENU_Check_AWARD_Enable
Date&Time       :2017/11/3 下午 6:09
Describe        :
*/
int inMENU_Check_AWARD_Enable(int inCode)
{
	char		szTMSOK[2 + 1];
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned char	uszTxnEnable = VS_TRUE;
	unsigned char	uszCreditCardSupport = VS_FALSE;
	unsigned char	uszBarcodeSupport = VS_FALSE;
	unsigned char	uszVoidRedeemSupport = VS_FALSE;
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	/* 判斷是否支援過卡兌換 */
	if (inNCCC_Loyalty_CreditCardFlag(0) == VS_SUCCESS)
	{
		uszCreditCardSupport = VS_TRUE;
	}
	else
	{
		uszCreditCardSupport = VS_FALSE;
	}
	
	/* 判斷是否支援條碼兌換 */
	if (inNCCC_Loyalty_BarCodeFlag(0) == VS_SUCCESS)
	{
		uszBarcodeSupport = VS_TRUE;
	}
	else
	{
		uszBarcodeSupport = VS_FALSE;
	}

	/* 判斷是否支援條碼兌換取消 */
	if (inNCCC_Loyalty_VoidRedeemFlag(0) == VS_SUCCESS)
	{
		uszVoidRedeemSupport = VS_TRUE;
	}
	else
	{
		uszVoidRedeemSupport = VS_FALSE;
	}

	/* 選單改為不判斷此開關 2019/7/12 下午 4:46 */
	/* 判斷是否支援優惠平台(含詢問電文) */
	
	/*	兌換方式
		‘1’=以條碼當作兌換資訊，透過收銀機條碼資訊。
		‘2’=以條碼當作兌換資訊，端末機接BarCode Reader掃描兌換(核銷)條碼。
		‘3’=以條碼當作兌換資訊，手動於端末機輸入兌換(核銷)條碼。
		‘4’=以卡號當作兌換資訊，於端末機上刷卡。
		‘5’=以卡號當作兌換資訊，於端末機上手動輸入。 
	 */
	
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	
	/* 若全部開關沒開，直接顯示此功能以關閉 */
	if ((uszCreditCardSupport != VS_TRUE && uszBarcodeSupport != VS_TRUE && uszVoidRedeemSupport != VS_TRUE)	||
	    (memcmp(szTMSOK, "Y", 1) != 0))
	{
		uszTxnEnable = VS_FALSE;
	}
	else
	{
		uszTxnEnable = VS_TRUE;
	}
	
	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inMENU_Check_AWARD_Enable
Date&Time       :2017/11/3 下午 6:15
Describe        :
*/
int inMENU_Check_HG_Enable(int inCode)
{
	char		szHostEnable[2 + 1];
	char		szTMSOK[2 + 1];
	unsigned char	uszTxnEnable = VS_TRUE;
	
	memset(szHostEnable, 0x00, sizeof(szHostEnable));
	inHG_GetHG_Enable(0, szHostEnable);
	
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	
	if ((memcmp(szHostEnable, "Y", strlen("Y")) != 0)	||
	    (memcmp(szTMSOK, "Y", 1) != 0))
	{
		uszTxnEnable = VS_FALSE;
	}
	else
	{
		uszTxnEnable = VS_TRUE;
	}
	
	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inMENU_Check_SETTLE_Enable
Date&Time       :2018/2/5 下午 3:30
Describe        :
*/
int inMENU_Check_SETTLE_Enable(int inCode)
{
	int		i = 0;
	char		szFunEnable[2 + 1] = {0};
	char		szTransFunc[20 + 1] = {0};
	char		szHostLabel[8 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned char	uszTxnEnable = VS_FALSE;	/* 預設為失敗，只要其中一個有開就不反白 */
	
	memset(szFunEnable, 0x00, sizeof(szFunEnable));
	inGetTMSOK(szFunEnable);
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 不論TMS是否有開結帳功能，端末機都不允許做結帳交易。 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_042_BDAU1_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
	{
		uszTxnEnable = VS_FALSE;
	}
	else
	{
		/* 沒下TMS */
		if (memcmp(szFunEnable, "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
		/* 有下TMS */
		else
		{
			/* 有下TMS才檢查開關 */
			/* 只要其中一個有開就不反白 */
			for (i = 0 ;; i ++)
			{
				if (inLoadHDTRec(i) < 0)	/* 主機參數檔【HostDef.txt】 */
					break;

				/* 沒開不檢查 */
				memset(szFunEnable, 0x00, sizeof(szFunEnable));
				inGetHostEnable(szFunEnable);
				if (szFunEnable[0] != 'Y')
				{
					continue;
				}

				memset(szHostLabel, 0x00, sizeof(szHostLabel));
				inGetHostLabel(szHostLabel);

				memset(szTransFunc, 0x00, sizeof(szTransFunc));
				inGetTransFunc(szTransFunc);

				if (memcmp(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_)) == 0)
				{
					if (szTransFunc[2] == 'Y')
					{
						uszTxnEnable = VS_TRUE;
						break;
					}
				}
				else if (memcmp(szHostLabel, _HOST_NAME_DINERS_, strlen(_HOST_NAME_DINERS_)) == 0)
				{
					if (szTransFunc[2] == 'Y')
					{
						uszTxnEnable = VS_TRUE;
						break;
					}
				}
				else if (memcmp(szHostLabel, _HOST_NAME_AMEX_, strlen(_HOST_NAME_AMEX_)) == 0)
				{
					if (szTransFunc[2] == 'Y')
					{
						uszTxnEnable = VS_TRUE;
						break;
					}
				}
				else if (memcmp(szHostLabel, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_)) == 0)
				{
					if (szTransFunc[2] == 'Y')
					{
						uszTxnEnable = VS_TRUE;
						break;
					}
				}
				else if (memcmp(szHostLabel, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_)) == 0)
				{
					/* IPASS */
					inLoadTDTRec(_TDT_INDEX_00_IPASS_);
					memset(szFunEnable, 0x00, sizeof(szFunEnable));
					inGetTicket_HostEnable(szFunEnable);

					if (memcmp(szFunEnable, "Y", strlen("Y")) == 0)
					{
						memset(szTransFunc, 0x00, sizeof(szTransFunc));
						inGetTicket_HostTransFunc(szTransFunc);

						if (szTransFunc[8] == 'Y')
						{
							uszTxnEnable = VS_TRUE;
							break;
						}
					}

					/* ECC */
					inLoadTDTRec(_TDT_INDEX_01_ECC_);
					memset(szFunEnable, 0x00, sizeof(szFunEnable));
					inGetTicket_HostEnable(szFunEnable);

					if (memcmp(szFunEnable, "Y", strlen("Y")) == 0)
					{
						memset(szTransFunc, 0x00, sizeof(szTransFunc));
						inGetTicket_HostTransFunc(szTransFunc);

						if (szTransFunc[8] == 'Y')
						{
							uszTxnEnable = VS_TRUE;
							break;
						}
					}

					/* ICASH */
					inLoadTDTRec(_TDT_INDEX_02_ICASH_);
					memset(szFunEnable, 0x00, sizeof(szFunEnable));
					inGetTicket_HostEnable(szFunEnable);

					if (memcmp(szFunEnable, "Y", strlen("Y")) == 0)
					{
						inLoadICASHDTRec(0);
						memset(szTransFunc, 0x00, sizeof(szTransFunc));
						inGetICASH_Transaction_Function(szTransFunc);

						if (szTransFunc[8] == 'Y')
						{
							uszTxnEnable = VS_TRUE;
							break;
						}
					}
				}
			}
		}
	}
	
	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inMENU_Check_REVIEW_SETTLE_Enable
Date&Time       :2018/2/5 下午 3:30
Describe        :
*/
int inMENU_Check_REVIEW_SETTLE_Enable(int inCode)
{
	char		szFunEnable[2 + 1];
	unsigned char	uszTxnEnable = VS_TRUE;
	
	memset(szFunEnable, 0x00, sizeof(szFunEnable));
	inGetTMSOK(szFunEnable);
	
	/* 沒下TMS */
	if (memcmp(szFunEnable, "Y", 1) != 0)
	{
		uszTxnEnable = VS_FALSE;
	}
	/* 有下TMS */
	else
	{
		uszTxnEnable = VS_TRUE;
	}
	
	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inMENU_Check_HG_Refund_Enable
Date&Time       :2017/11/3 下午 6:15
Describe        :
*/
int inMENU_Check_HG_Refund_Enable(int inCode)
{
	char		szHostEnable[2 + 1];
	unsigned char	uszTxnEnable = VS_TRUE;
	
	memset(szHostEnable, 0x00, sizeof(szHostEnable));
	inHG_GetHG_Enable(0, szHostEnable);
	if (memcmp(szHostEnable, "Y", strlen("Y")) != 0)
	{
		uszTxnEnable = VS_FALSE;
	}
	else
	{
		uszTxnEnable = VS_TRUE;
	}
	
	
	
	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inMENU_Check_Mark_Enable
Date&Time       :2017/11/3 下午 6:15
Describe        :
*/
int inMENU_Check_Mark_Enable(int inCode)
{
	unsigned char	uszTxnEnable = VS_TRUE;
	
	if (inCode == _TRANS_TYPE_NULL_)
	{
		uszTxnEnable = VS_FALSE;
	}
	
	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inMENU_Display_ICON
Date&Time       :2017/10/31 上午 10:01
Describe        :inKeyPostionID 用來標示是哪一個key，用以確認位置
*/
int inMENU_Display_ICON(char* szFileName, int inButtonPostionID)
{
	switch (inButtonPostionID)
	{
		case	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_1_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN1_X1_, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE1_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_2_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN2_X1_, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE1_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_3_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN3_X1_, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE1_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN1_X1_, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE2_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN2_X1_, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE2_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN3_X1_, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE2_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_7_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN1_X1_, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE3_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_8_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN2_X1_, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE3_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_9_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN3_X1_, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_LINE3_Y1_);
			break;
			
		case	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN1_X1_, _DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE1_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN2_X1_, _DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE1_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN3_X1_, _DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE1_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN1_X1_, _DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE2_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN2_X1_, _DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE2_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN3_X1_, _DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE2_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_7_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN1_X1_, _DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE3_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_8_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN2_X1_, _DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE3_Y1_);
			break;
		case	_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_9_:
			inDISP_PutGraphic(szFileName, _DisTouch_NEWUI_FUNC_3_TO_8_3X3_COLUMN3_X1_, _DisTouch_NEWUI_FUNC_3_TO_7_3X3_LINE3_Y1_);
			break;
			
		default:
			break;
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inMENU_CHECK_FUNCTION_ENABLE_DISPLAY
Date&Time       :2017/10/31 下午 2:21
Describe        :
*/
int inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(MENU_CHECK_TABLE *srMENU_CHECK_TABLE)
{
	int	i = 0;
	
	while(1)
	{
		if (srMENU_CHECK_TABLE[i].inButtonPositionID == _Touch_NONE_)
		{
			break;
		}
		
		if (srMENU_CHECK_TABLE[i].inCheckFunc == NULL)
		{
			/* 不檢核 */
		}
		else
		{
			if (srMENU_CHECK_TABLE[i].inCheckFunc(srMENU_CHECK_TABLE[i].inCode) != VS_SUCCESS)
			{
				inMENU_Display_ICON(srMENU_CHECK_TABLE[i].szFileName, srMENU_CHECK_TABLE[i].inButtonPositionID);
			}
		}
		
		i++;
	}
	
	return (VS_SUCCESS);
}

/*
Function	:inMENU_FUNCTION_NEWUI_PAGE1
Date&Time	:2017/11/3 下午 3:42
Describe	:
*/
int inMENU_FUNCTION_NEWUI_PAGE1(EventMenuItem *srEventMenuItem)
{
	int	inPageLoop = _PAGE_LOOP_1_;
	int	inRetVal = VS_SUCCESS;
	int	inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
	int	inCount= 1;
	int	inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char	szKey = 0x00;
	char	szCustomerIndicator[3 + 1] = {0};
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_	, _TRANS_TYPE_NULL_	, inMENU_Check_Transaction_Enable       , _ICON_HIGHTLIGHT_1_TRANSACTION_   },
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_	, _TRANS_TYPE_NULL_	, inMENU_Check_CUP_Enable               , _ICON_HIGHTLIGHT_2_CUP_           },
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_	, _TRANS_TYPE_NULL_	, inMENU_Check_SMARTPAY_Enable          , _ICON_HIGHTLIGHT_3_SMARTPAY_      },
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_	, _TRANS_TYPE_NULL_	, inMENU_Check_ETICKET_Enable           , _ICON_HIGHTLIGHT_4_ETICKET_       },
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_	, _TRANS_TYPE_NULL_	, inMENU_Check_AWARD_Enable             , _ICON_HIGHTLIGHT_5_AWARD_         },
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_	, _TRANS_TYPE_NULL_	, inMENU_Check_HG_Enable                , _ICON_HIGHTLIGHT_6_HG_            },
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_7_	, _TRANS_TYPE_NULL_	, inMENU_Check_SETTLE_Enable            , _ICON_HIGHTLIGHT_7_SETTLE_        },
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_8_	, _TRANS_TYPE_NULL_	, inMENU_Check_REVIEW_SETTLE_Enable     , _ICON_HIGHTLIGHT_8_REVIEW_PRINT_  },
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_9_	, _TRANS_TYPE_NULL_	, NULL             , ""         },
		{_Touch_NONE_}
	};
	
	memset(szCustomerIndicator, 0x00 , sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 初始化 */
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);	/* 第1.5層顯示 LOGO */
	inDISP_PutGraphic(_TOUCH_NEWUI_FUNCTION_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
	
        if (ginHalfLCD == VS_FALSE)
	{      
		inDISP_PutGraphic(_ICON_BUTTON_TEMP_BLUE_, _COORDINATE_X_16_10_,  _COORDINATE_Y_LINE_8_8_);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("回主畫面", _FONTSIZE_16X44_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_16_10_ + 20, _COORDINATE_Y_LINE_24_23_, VS_FALSE);
	}
    
	/* 檢查功能開關，並顯示反白的圖 */
	inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);

	/* 設定Timeout */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
	}
	else
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	}
		
	while (inPageLoop != _PAGE_LOOP_0_)
	{
		while (inPageLoop == _PAGE_LOOP_1_)
		{
			szKey = uszKBD_Key();
			inChioce1 = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc); /* 回傳MENU畫面的按鈕選擇 */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}
			
			switch (inChioce1)
			{
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_:
					inCount = 1;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_:
					inCount = 2;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_:
					inCount = 3;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_:
					inCount = 4;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_:
					inCount = 5;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_:
					inCount = 6;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_7_:
					inCount = 7;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_8_:
					inCount = 8;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_9_:
					inCount = 9;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_RETURN_IDLE_BUTTON_:
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				case _DisTouch_Slide_Down_To_Up_:
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				default:
					break;
			}

			if (szKey == _KEY_1_)
			{
				inCount = 1;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_2_)
			{
				inCount = 2;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_3_)
			{
				inCount = 3;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_4_)
			{
				inCount = 4;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_5_)
			{
				inCount = 5;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_6_)
			{
				inCount = 6;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_7_)
			{
				inCount = 7;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_8_)
			{
				inCount = 8;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_9_)
			{
				inCount = 9;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				inPageLoop = _PAGE_LOOP_0_;
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				inPageLoop = _PAGE_LOOP_0_;
				break;
			}
			
			inFunc_Display_All_Status_By_Machine_Type();
		}

		/* 判斷MENU點進去按鈕之後要做的事情 */
		if (inPageLoop == _PAGE_LOOP_2_)
		{
			switch (inCount)
			{
				case 1:
					inRetVal = inMENU_NEWUI_TRANSACTION_MENU(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else if (inRetVal == VS_FUNC_CLOSE_ERR)
					{
						inRetVal = VS_SUCCESS;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 2:
					inRetVal = inMENU_NEWUI_CUP_MENU(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else if (inRetVal == VS_FUNC_CLOSE_ERR)
					{
						inRetVal = VS_SUCCESS;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 3:
					inRetVal = inMENU_NEWUI_SMARTPAY_MENU(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else if (inRetVal == VS_FUNC_CLOSE_ERR)
					{
						inRetVal = VS_SUCCESS;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 4:
					inRetVal = inMENU_NEWUI_ETICKET_MENU(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else if (inRetVal == VS_FUNC_CLOSE_ERR)
					{
						inRetVal = VS_SUCCESS;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 5:
					inRetVal = inMENU_NEWUI_AWARD_MENU(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else if (inRetVal == VS_FUNC_CLOSE_ERR)
					{
						inRetVal = VS_SUCCESS;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 6:
					inRetVal = inMENU_NEWUI_HAPPYGO_MENU(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else if (inRetVal == VS_FUNC_CLOSE_ERR)
					{
						inRetVal = VS_SUCCESS;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 7:
					inRetVal = inMENU_NEWUI_SETTLE_MENU(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else if (inRetVal == VS_FUNC_CLOSE_ERR)
					{
						inRetVal = VS_SUCCESS;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 8:
					inRetVal = inMENU_NEWUI_REVIEW_PRINT_MENU(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else if (inRetVal == VS_FUNC_CLOSE_ERR)
					{
						inRetVal = VS_SUCCESS;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 9:
					if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
					    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
					{
						inRetVal = inMENU_026_NEWUI_SETTING_MENU(srEventMenuItem);
					}
					else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_) ||
						 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
					{
						inRetVal = inMENU_123_NEWUI_SETTING_MENU(srEventMenuItem);
					}
					else
					{
						inRetVal = inMENU_NEWUI_SETTING_MENU(srEventMenuItem);
					}
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else if (inRetVal == VS_FUNC_CLOSE_ERR)
					{
						inRetVal = VS_SUCCESS;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				default:
					inPageLoop = _PAGE_LOOP_0_;
					break;
			}
		}
		
		/* 代表回上一頁，要回復UI */
		if (inPageLoop == _PAGE_LOOP_1_		&&
		    inRetVal == VS_ERROR)
		{
			/* 初始化 */
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第1.5層顯示 LOGO */
			inDISP_PutGraphic(_TOUCH_NEWUI_FUNCTION_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
			
                        if (ginHalfLCD == VS_FALSE)
                        {      
                            inDISP_PutGraphic(_ICON_BUTTON_TEMP_BLUE_, _COORDINATE_X_16_10_,  _COORDINATE_Y_LINE_8_8_);
                            inDISP_ChineseFont_Point_Color_By_Graphic_Mode("回主畫面", _FONTSIZE_16X44_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_16_10_ + 20, _COORDINATE_Y_LINE_24_23_, VS_FALSE);
                        }
            
			inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
			/* 設定Timeout */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
			}
			else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
			}
			else
			{
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
			}
		}
		/* 功能未開而返回，不刷畫面 */
		else if (inPageLoop == _PAGE_LOOP_1_	&&
			 inRetVal == VS_SUCCESS)
		{
			
		}
	}/* _PAGE_LOOP_0_ */

	return (inRetVal);
}

/*
Function : inMENU_FUNCTION_TRUST_UI_PAGE
Date&Time: 
Describe : 
 * [113110-信託需求][UI] 畫面需依照信託功能是否開啟變動 2025/11/20 
 * 新增信託頁面,因為ICON已滿九個,後續要使用的畫面需再創另一頁面
*/
int inMENU_FUNCTION_TRUST_UI_PAGE(EventMenuItem *srEventMenuItem)
{
	int	inPageLoop = _PAGE_LOOP_1_;
	int	inRetVal = VS_SUCCESS;
	int	inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
	int	inCount= 1;
	int	inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char	szKey = 0x00;
	char	szCustomerIndicator[3 + 1] = {0};
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_	, _TRANS_TYPE_NULL_	, inMENU_Check_Transaction_Enable       , _ICON_HIGHTLIGHT_1_TRANSACTION_   },
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_	, _TRANS_TYPE_NULL_	, inMENU_Check_CUP_Enable               , _ICON_HIGHTLIGHT_2_CUP_           },
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_	, _TRANS_TYPE_NULL_	, inMENU_Check_SMARTPAY_Enable          , _ICON_HIGHTLIGHT_3_SMARTPAY_      },
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_	, _TRANS_TYPE_NULL_	, inMENU_Check_ETICKET_Enable           , _ICON_HIGHTLIGHT_4_ETICKET_       },
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_	, _TRANS_TYPE_NULL_	, inMENU_Check_AWARD_Enable             , _ICON_HIGHTLIGHT_5_AWARD_         },
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_	, _TRANS_TYPE_NULL_	, inMENU_Check_HG_Enable                , _ICON_HIGHTLIGHT_6_HG_            },
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_7_	, _TRANS_TYPE_NULL_	, inMENU_Check_SETTLE_Enable            , _ICON_HIGHTLIGHT_7_SETTLE_        },
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_8_	, _TRANS_TYPE_NULL_	, inMENU_Check_REVIEW_SETTLE_Enable     , _ICON_HIGHTLIGHT_8_REVIEW_PRINT_  },
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_9_	, _TRANS_TYPE_NULL_	, inMENU_Check_Trust_Enable             , _ICON_HIGHTLIGHT_9_TRUST_         },
		{_Touch_NONE_}
	};
	
	memset(szCustomerIndicator, 0x00 , sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
		
	/* 初始化 */
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);	/* 第1.5層顯示 LOGO */
	inDISP_PutGraphic(_TOUCH_NEWUI_TRUST_UI_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
	
	if (ginHalfLCD == VS_FALSE)
	{      
		inDISP_PutGraphic(_ICON_BUTTON_TEMP_BLUE_, _COORDINATE_X_16_3_,  _COORDINATE_Y_LINE_8_8_);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("下一頁", _FONTSIZE_16X44_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_16_3_ + 20, _COORDINATE_Y_LINE_24_23_, VS_FALSE);
		inDISP_PutGraphic(_ICON_BUTTON_TEMP_BLUE_, _COORDINATE_X_16_10_,  _COORDINATE_Y_LINE_8_8_);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("回主畫面", _FONTSIZE_16X44_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_16_10_ + 20, _COORDINATE_Y_LINE_24_23_, VS_FALSE);
	}
        
	/* 檢查功能開關，並顯示反白的圖 */
	inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);

	/* 設定Timeout */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
	}
	else
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	}
		
	while (inPageLoop != _PAGE_LOOP_0_)
	{
		while (inPageLoop == _PAGE_LOOP_1_)
		{
			szKey = uszKBD_Key();
			inChioce1 = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc); /* 回傳MENU畫面的按鈕選擇 */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}
			
			switch (inChioce1)
			{
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_:
					inCount = 1;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_:
					inCount = 2;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_:
					inCount = 3;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_:
					inCount = 4;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_:
					inCount = 5;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_:
					inCount = 6;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_7_:
					inCount = 7;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_8_:
					inCount = 8;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_9_:
					inCount = 9;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_LAST_PAGE_BUTTON_:
					inRetVal = VS_NEXT_PAGE;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				case _NEWUI_RETURN_IDLE_BUTTON_:
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				case _DisTouch_Slide_Down_To_Up_:
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				default:
					break;
			}

			if (szKey == _KEY_1_)
			{
				inCount = 1;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_2_)
			{
				inCount = 2;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_3_)
			{
				inCount = 3;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_4_)
			{
				inCount = 4;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_5_)
			{
				inCount = 5;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_6_)
			{
				inCount = 6;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_7_)
			{
				inCount = 7;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_8_)
			{
				inCount = 8;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_9_)
			{
				inCount = 9;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_DOWN_)
			{
				inRetVal = VS_NEXT_PAGE;
				inPageLoop = _PAGE_LOOP_0_;
				break;
			}
			else if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				inPageLoop = _PAGE_LOOP_0_;
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				inPageLoop = _PAGE_LOOP_0_;
				break;
			}
			
			inFunc_Display_All_Status_By_Machine_Type();
		}

		/* 判斷MENU點進去按鈕之後要做的事情 */
		if (inPageLoop == _PAGE_LOOP_2_)
		{
			switch (inCount)
			{
				case 1:
					inRetVal = inMENU_NEWUI_TRANSACTION_MENU(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else if (inRetVal == VS_FUNC_CLOSE_ERR)
					{
						inRetVal = VS_SUCCESS;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 2:
					inRetVal = inMENU_NEWUI_CUP_MENU(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else if (inRetVal == VS_FUNC_CLOSE_ERR)
					{
						inRetVal = VS_SUCCESS;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 3:
					inRetVal = inMENU_NEWUI_SMARTPAY_MENU(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else if (inRetVal == VS_FUNC_CLOSE_ERR)
					{
						inRetVal = VS_SUCCESS;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 4:
					inRetVal = inMENU_NEWUI_ETICKET_MENU(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else if (inRetVal == VS_FUNC_CLOSE_ERR)
					{
						inRetVal = VS_SUCCESS;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 5:
					inRetVal = inMENU_NEWUI_AWARD_MENU(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else if (inRetVal == VS_FUNC_CLOSE_ERR)
					{
						inRetVal = VS_SUCCESS;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 6:
					inRetVal = inMENU_NEWUI_HAPPYGO_MENU(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else if (inRetVal == VS_FUNC_CLOSE_ERR)
					{
						inRetVal = VS_SUCCESS;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 7:
					inRetVal = inMENU_NEWUI_SETTLE_MENU(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else if (inRetVal == VS_FUNC_CLOSE_ERR)
					{
						inRetVal = VS_SUCCESS;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 8:
					inRetVal = inMENU_NEWUI_REVIEW_PRINT_MENU(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else if (inRetVal == VS_FUNC_CLOSE_ERR)
					{
						inRetVal = VS_SUCCESS;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 9:
					inRetVal = inMENU_NEWUI_NEWUI_TRUST_MENU(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else if (inRetVal == VS_FUNC_CLOSE_ERR)
					{
						inRetVal = VS_SUCCESS;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				default:
					inPageLoop = _PAGE_LOOP_0_;
					break;
			}
		}
		
		/* 代表回上一頁，要回復UI */
		if (inPageLoop == _PAGE_LOOP_1_		&&
		    inRetVal == VS_ERROR)
		{
			/* 初始化 */
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第1.5層顯示 LOGO */
			inDISP_PutGraphic(_TOUCH_NEWUI_TRUST_UI_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
			
			if (ginHalfLCD == VS_FALSE)
			{
				inDISP_PutGraphic(_ICON_BUTTON_TEMP_BLUE_, _COORDINATE_X_16_3_,  _COORDINATE_Y_LINE_8_8_);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("下一頁", _FONTSIZE_16X44_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_16_3_ + 20, _COORDINATE_Y_LINE_24_23_, VS_FALSE);
				inDISP_PutGraphic(_ICON_BUTTON_TEMP_BLUE_, _COORDINATE_X_16_10_,  _COORDINATE_Y_LINE_8_8_);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("回主畫面", _FONTSIZE_16X44_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_16_10_ + 20, _COORDINATE_Y_LINE_24_23_, VS_FALSE);
			}

			inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
			/* 設定Timeout */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
			}
			else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
			}
			else
			{
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
			}
		}
		/* 功能未開而返回，不刷畫面 */
		else if (inPageLoop == _PAGE_LOOP_1_	&&
			 inRetVal == VS_SUCCESS)
		{
			
		}
	}/* _PAGE_LOOP_0_ */

	return (inRetVal);
}


/*
Function	:inMENU_FUNCTION_NEWUI_PAGE2
Date&Time	:2025/6/18 下午 1:51
Describe	:
*/
int inMENU_FUNCTION_NEWUI_PAGE2(EventMenuItem *srEventMenuItem)
{
	int	inPageLoop = _PAGE_LOOP_1_;
	int	inRetVal = VS_SUCCESS;
	int	inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
	int	inCount= 1;
	int	inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char	szKey = 0x00;
	char	szCustomerIndicator[3 + 1] = {0};
	MENU_CHECK_2_TABLE_	srMenuChekDisplay[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_	, _TRANS_TYPE_NULL_	, _MENU_CHECK_FORCE_DISPLAY_ON_	, NULL, 
		"設定",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_8_, VS_FALSE, 
		"      1",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_9_, VS_FALSE},
		{_Touch_NONE_}
	};
	
	memset(szCustomerIndicator, 0x00 , sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
        /* 初始化 */
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第1.5層顯示 LOGO */
        
	if (ginHalfLCD == VS_FALSE)
	{      
		inDISP_PutGraphic(_ICON_BUTTON_TEMP_BLUE_, _COORDINATE_X_16_3_,  _COORDINATE_Y_LINE_8_8_);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("上一頁", _FONTSIZE_16X44_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_16_3_ + 20, _COORDINATE_Y_LINE_24_23_, VS_FALSE);
		inDISP_PutGraphic(_ICON_BUTTON_TEMP_BLUE_, _COORDINATE_X_16_10_,  _COORDINATE_Y_LINE_8_8_);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("回主畫面", _FONTSIZE_16X44_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_16_10_ + 20, _COORDINATE_Y_LINE_24_23_, VS_FALSE);
	}
        
	/* 檢查功能開關，並顯示反白的圖 */
	inMENU_CHECK_2_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);

	/* 設定Timeout */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
	}
	else
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	}
		
	while (inPageLoop != _PAGE_LOOP_0_)
	{
		while (inPageLoop == _PAGE_LOOP_1_)
		{
			szKey = uszKBD_Key();
			inChioce1 = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc); /* 回傳MENU畫面的按鈕選擇 */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}
			
			switch (inChioce1)
			{
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_:
					inCount = 1;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_:
					inCount = 2;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_:
					inCount = 3;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_:
					inCount = 4;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_:
					inCount = 5;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_:
					inCount = 6;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_7_:
					inCount = 7;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_8_:
					inCount = 8;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_9_:
					inCount = 9;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_LAST_PAGE_BUTTON_:
					inRetVal = VS_LAST_PAGE;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				case _NEWUI_RETURN_IDLE_BUTTON_:
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				case _DisTouch_Slide_Down_To_Up_:
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				default:
					break;
			}

			if (szKey == _KEY_1_)
			{
				inCount = 1;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_2_)
			{
				inCount = 2;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_3_)
			{
				inCount = 3;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_4_)
			{
				inCount = 4;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_5_)
			{
				inCount = 5;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_6_)
			{
				inCount = 6;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_7_)
			{
				inCount = 7;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_8_)
			{
				inCount = 8;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_9_)
			{
				inCount = 9;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_UP_)
			{
				inRetVal = VS_LAST_PAGE;
				inPageLoop = _PAGE_LOOP_0_;
				break;
			}
			else if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				inPageLoop = _PAGE_LOOP_0_;
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				inPageLoop = _PAGE_LOOP_0_;
				break;
			}
			
			inFunc_Display_All_Status_By_Machine_Type();
		}

		/* 判斷MENU點進去按鈕之後要做的事情 */
		if (inPageLoop == _PAGE_LOOP_2_)
		{
			switch (inCount)
			{
				case 1:
					if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
					    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
					{
						inRetVal = inMENU_026_NEWUI_SETTING_MENU(srEventMenuItem);
					}
					else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_) ||
						 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
					{
						inRetVal = inMENU_123_NEWUI_SETTING_MENU(srEventMenuItem);
					}
					else
					{
						inRetVal = inMENU_NEWUI_SETTING_MENU(srEventMenuItem);
					}
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else if (inRetVal == VS_FUNC_CLOSE_ERR)
					{
						inRetVal = VS_SUCCESS;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				default:
					inPageLoop = _PAGE_LOOP_0_;
					break;
			}
		}
		
		/* 代表回上一頁，要回復UI */
		if (inPageLoop == _PAGE_LOOP_1_		&&
		    inRetVal == VS_ERROR)
		{
			/* 初始化 */
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第1.5層顯示 LOGO */
			
			if (ginHalfLCD == VS_FALSE)
                        {
				inDISP_PutGraphic(_ICON_BUTTON_TEMP_BLUE_, _COORDINATE_X_16_3_,  _COORDINATE_Y_LINE_8_8_);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("上一頁", _FONTSIZE_16X44_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_16_3_ + 20, _COORDINATE_Y_LINE_24_23_, VS_FALSE);
				inDISP_PutGraphic(_ICON_BUTTON_TEMP_BLUE_, _COORDINATE_X_16_10_,  _COORDINATE_Y_LINE_8_8_);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("回主畫面", _FONTSIZE_16X44_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_16_10_ + 20, _COORDINATE_Y_LINE_24_23_, VS_FALSE);
                        }

			inMENU_CHECK_2_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
			/* 設定Timeout */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
			}
			else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
			}
			else
			{
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
			}
		}
		/* 功能未開而返回，不刷畫面 */
		else if (inPageLoop == _PAGE_LOOP_1_	&&
			 inRetVal == VS_SUCCESS)
		{
			
		}
	}/* _PAGE_LOOP_0_ */

	return (inRetVal);
}

/*
Function        :inMENU_NEWUI_TRANSACTION_MENU
Date&Time       :2017/10/25 下午 1:55
Describe        :
*/
int inMENU_NEWUI_TRANSACTION_MENU(EventMenuItem *srEventMenuItem)
{
	int			inPageLoop = _PAGE_LOOP_1_;
        int			inRetVal = VS_SUCCESS;
        int			inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
        int			inCount = 1;
        int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char			szKey = 0x00;
	char			szCustomerIndicator[3 + 1] = {0};
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_	, _VOID_		, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_1_1_VOID_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_	, _REFUND_		, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_1_2_REFUND_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_	, _INST_SALE_		, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_1_3_INST_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_	, _REDEEM_SALE_		, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_1_4_REDEEM_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_	, _SALE_OFFLINE_	, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_1_5_SALEOFFLINE_	},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_	, _TIP_			, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_1_6_TIP_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_7_	, _PRE_AUTH_		, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_1_7_PREAUTH_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_8_	, _TRANS_TYPE_NULL_	, inMENU_Check_Adjust				, _ICON_HIGHTLIGHT_1_8_ADJUST_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_9_	, _MAIL_ORDER_		, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_1_9_MAILORDER_	},
		{_Touch_NONE_				, _TRANS_TYPE_NULL_	, NULL						, ""					}
	};
	
	memset(szCustomerIndicator, 0x00 , sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (inMENU_Check_Transaction_Enable(0) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		/* 初始化 */
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_1_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
		inDISP_PutGraphic(_TOUCH_NEWUI_TRANSACTION_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
		
		if (ginHalfLCD == VS_FALSE)
			inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

		/* 檢查功能開關，並顯示反白的圖 */
		inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);

		/* 設定Timeout */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
		}
		else
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
		}

		while (inPageLoop != _PAGE_LOOP_0_)
		{
			while (inPageLoop == _PAGE_LOOP_1_)
			{
				szKey = uszKBD_Key();
				inChioce1 = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc); /* 回傳MENU畫面的按鈕選擇 */
				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
				{
					szKey = _KEY_TIMEOUT_;
				}

				switch (inChioce1)
				{
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_:
						inCount = 1;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_:
						inCount = 2;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_:
						inCount = 3;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_:
						inCount = 4;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_:
						inCount = 5;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_:
						inCount = 6;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_7_:
						inCount = 7;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_8_:
						inCount = 8;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_9_:
						inCount = 9;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_RETURN_IDLE_BUTTON_:
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					case _NEWUI_LAST_PAGE_BUTTON_:
						inRetVal = VS_USER_CANCEL;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					case _DisTouch_Slide_Down_To_Up_:
						inRetVal = VS_USER_CANCEL;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					default:
						break;
				}

				if (szKey == _KEY_1_)
				{
					inCount = 1;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_2_)
				{
					inCount = 2;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_3_)
				{
					inCount = 3;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_4_)
				{
					inCount = 4;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_5_)
				{
					inCount = 5;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_6_)
				{
					inCount = 6;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_7_)
				{
					inCount = 7;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_8_)
				{
					inCount = 8;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_9_)
				{
					inCount = 9;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_CANCEL_)
				{
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				}
				else if (szKey == _KEY_TIMEOUT_)
				{
					inRetVal = VS_TIMEOUT;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				}

			}

			/* 判斷MENU點進去按鈕之後要做的事情 */
			if (inPageLoop == _PAGE_LOOP_2_)
			{
				switch (inCount)
				{
					case 1:
						inRetVal = inMENU_VOID(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 2:
						inRetVal = inMENU_REFUND(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 3:
						inRetVal = inMENU_INST(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 4:
						inRetVal = inMENU_REDEEM(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 5:
						inRetVal = inMENU_SALEOFFLINE(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 6:
						inRetVal = inMENU_TIP(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 7:
						inRetVal = inMENU_PREAUTH(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 8:
						inRetVal = inMENU_ADJUST(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 9:
						inRetVal = inMENU_MAILORDER(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					default:
						inPageLoop = _PAGE_LOOP_0_;
						break;
				}
			}

			/* 代表回上一頁，要回復UI */
			if (inPageLoop == _PAGE_LOOP_1_		&&
			    inRetVal == VS_ERROR)
			{
				/* 初始化 */
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_1_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
				inDISP_PutGraphic(_TOUCH_NEWUI_TRANSACTION_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
				
				if (ginHalfLCD == VS_FALSE)
					inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);
			
				inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
				/* 設定Timeout */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
				{
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
				}
				else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
				}
				else
				{
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
				}
			}
			/* 功能未開而返回，不刷畫面 */
			else if (inPageLoop == _PAGE_LOOP_1_	&&
				 inRetVal == VS_SUCCESS)
			{

			}
		}/* _PAGE_LOOP_0_ */
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_NEWUI_CUP_MENU
Date&Time       :2017/10/25 下午 1:55
Describe        :
*/
int inMENU_NEWUI_CUP_MENU(EventMenuItem *srEventMenuItem)
{
	int			inMSR_RetVal = -1;	/* 磁條事件的反應，怕和其他用到inRetVal的搞混所以獨立出來 */
	int			inEMV_RetVal = -1;	/* 晶片卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
	int			inPageLoop = _PAGE_LOOP_1_;
        int			inRetVal = VS_SUCCESS;
        int			inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
        int			inCount= 1;
        int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char			szKey = 0x00;
	char			szCustomerIndicator[3 + 1] = {0};
	TRANSACTION_OBJECT	pobTran;
	MENU_CHECK_2_TABLE_	srMenuChekDisplay[] =
	{
                {_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_	, _VOID_	, _MENU_CHECK_FORCE_DISPLAY_NONE_	, inNCCC_Func_Check_Transaction_Function, 
		"取消",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_8_, VS_FALSE, 
		"交易  1",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_9_, VS_FALSE},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_	, _REFUND_		, _MENU_CHECK_FORCE_DISPLAY_NONE_	, inNCCC_Func_Check_Transaction_Function, 
		"退貨",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_2_, _COORDINATE_Y_LINE_24_8_, VS_FALSE, 
		"交易  2",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_2_, _COORDINATE_Y_LINE_24_9_, VS_FALSE},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_	, _CUP_PRE_AUTH_	, _MENU_CHECK_FORCE_DISPLAY_NONE_	, inNCCC_Func_Check_Transaction_Function, 
		"預先",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_3_, _COORDINATE_Y_LINE_24_8_, VS_FALSE, 
		"授權  3",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_3_, _COORDINATE_Y_LINE_24_9_, VS_FALSE},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_	, _CUP_MAIL_ORDER_	, _MENU_CHECK_FORCE_DISPLAY_NONE_	, inNCCC_Func_Check_Transaction_Function, 
		"郵購",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_12_, VS_FALSE, 
		"交易  4",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_13_, VS_FALSE},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_	, _CUP_SALE_	, _MENU_CHECK_FORCE_DISPLAY_NONE_	, inNCCC_Func_Check_Transaction_Function, 
		"一般",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_2_, _COORDINATE_Y_LINE_24_12_, VS_FALSE, 
		"交易  5",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_2_, _COORDINATE_Y_LINE_24_13_, VS_FALSE},
		{_Touch_NONE_}
	};
	memset(szCustomerIndicator, 0x00 , sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (inMENU_Check_CUP_Enable(0) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		/* 有開CUP且MACEnable有開但安全認證沒過，不能執行CUP交易 */
		if (inKMS_CheckKey(_TWK_KEYSET_NCCC_, _TWK_KEYINDEX_NCCC_MAC_) != VS_SUCCESS)
		{
			memset(&pobTran, 0x00, sizeof(TRANSACTION_OBJECT));
			if (inNCCC_Func_CUP_PowerOn_LogOn(&pobTran) != VS_SUCCESS)
			{
				/* 安全認證失敗 */
				return (VS_ERROR);
			}
		}

		/* 初始化 */
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_2_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
	
		if (ginHalfLCD == VS_FALSE)
			inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);
		
		/* 檢查功能開關，並顯示反白的圖 */
		inMENU_CHECK_2_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);

		/* 設定Timeout */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
		}
		else
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
		}

		while (inPageLoop != _PAGE_LOOP_0_)
		{
			while (inPageLoop == _PAGE_LOOP_1_)
			{
				szKey = uszKBD_Key();
				inChioce1 = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc); /* 回傳MENU畫面的按鈕選擇 */
				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
				{
					szKey = _KEY_TIMEOUT_;
				}

				/* 偵測磁條刷卡 */
				inMSR_RetVal = inCARD_MSREvent();
	
				if (inMSR_RetVal == VS_SUCCESS)
				{
					ginIdleMSRStatus = VS_TRUE;
				}
	
				/* 偵測晶片插卡 */
				inEMV_RetVal = inEMV_ICCEvent();
				if (inEMV_RetVal == VS_SUCCESS)
				{
					ginIdleICCStatus = VS_TRUE;
				}
				
				switch (inChioce1)
				{
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_:
						inCount = 1;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_:
						inCount = 2;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_:
						inCount = 3;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_:
						inCount = 4;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_:
						inCount = 5;
						inPageLoop = _PAGE_LOOP_2_;
						break;
//					case _NEWUI_FUNC_LINE_3_TO_7_Touch_KEY_6_:
//						inCount = 6;
//						inPageLoop = _PAGE_LOOP_2_;
//						break;
//					case _NEWUI_FUNC_LINE_3_TO_7_Touch_KEY_7_:
//						inCount = 7;
//						inPageLoop = _PAGE_LOOP_2_;
//						break;
					case _NEWUI_RETURN_IDLE_BUTTON_:
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					case _NEWUI_LAST_PAGE_BUTTON_:
						inRetVal = VS_USER_CANCEL;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					case _DisTouch_Slide_Down_To_Up_:
						inRetVal = VS_USER_CANCEL;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					default:
						break;
				}

				if (szKey == _KEY_1_)
				{
					inCount = 1;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_2_)
				{
					inCount = 2;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_3_)
				{
					inCount = 3;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_4_)
				{
					inCount = 4;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_5_)
				{
					inCount = 5;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
//				else if (szKey == _KEY_6_)
//				{
//					inCount = 6;
//					inPageLoop = _PAGE_LOOP_2_;
//					break;
//				}
//				else if (szKey == _KEY_7_)
//				{
//					inCount = 7;
//					inPageLoop = _PAGE_LOOP_2_;
//					break;
//				}
				/* 銀聯選單插卡或刷卡，直接當銀聯一般交易 */
				else if (inMSR_RetVal == VS_SUCCESS || inEMV_RetVal == VS_SUCCESS)
				{
					inCount = 5;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_CANCEL_)
				{
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				}
				else if (szKey == _KEY_TIMEOUT_)
				{
					inRetVal = VS_TIMEOUT;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				}

			}

			/* 判斷MENU點進去按鈕之後要做的事情 */
			if (inPageLoop == _PAGE_LOOP_2_)
			{
				switch (inCount)
				{
					case 1:
						inRetVal = inMENU_CUP_VOID(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 2:
						inRetVal = inMENU_CUP_REFUND(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
//					case 3:
//						inRetVal = inMENU_CUP_INST(srEventMenuItem);
//						if (inRetVal == VS_USER_CANCEL)
//						{
//							inRetVal = VS_ERROR;
//							inPageLoop = _PAGE_LOOP_0_;
//						}
//						else if (inRetVal == VS_FUNC_CLOSE_ERR)
//						{
//							inRetVal = VS_SUCCESS;
//							inPageLoop = _PAGE_LOOP_1_;
//						}
//						else
//						{
//							inPageLoop = _PAGE_LOOP_0_;
//						}
//						break;
//					case 4:
//						inRetVal = inMENU_CUP_REDEEM(srEventMenuItem);
//						if (inRetVal == VS_USER_CANCEL)
//						{
//							inRetVal = VS_ERROR;
//							inPageLoop = _PAGE_LOOP_0_;
//						}
//						else if (inRetVal == VS_FUNC_CLOSE_ERR)
//						{
//							inRetVal = VS_SUCCESS;
//							inPageLoop = _PAGE_LOOP_1_;
//						}
//						else
//						{
//							inPageLoop = _PAGE_LOOP_0_;
//						}
//						break;
					case 3:
						inRetVal = inMENU_CUP_PREAUTH(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 4:
						inRetVal = inMENU_CUP_MAILORDER(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 5:
						inRetVal = inMENU_CUP_SALE(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					default:
						inPageLoop = _PAGE_LOOP_0_;
						break;
				}
			}

			/* 代表回上一頁，要回復UI */
			if (inPageLoop == _PAGE_LOOP_1_		&&
			    inRetVal == VS_ERROR)
			{
				/* 初始化 */
				inDISP_ClearAll();
				inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_2_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
			
				if (ginHalfLCD == VS_FALSE)
					inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);
			
				inMENU_CHECK_2_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
				/* 設定Timeout */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
				{
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
				}
				else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
				}
				else
				{
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
				}
			}
			/* 功能未開而返回，不刷畫面 */
			else if (inPageLoop == _PAGE_LOOP_1_	&&
				 inRetVal == VS_SUCCESS)
			{

			}
		}/* _PAGE_LOOP_0_ */
		
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_NEWUI_SMARTPAY_MENU
Date&Time       :2017/10/25 下午 1:55
Describe        :
*/
int inMENU_NEWUI_SMARTPAY_MENU(EventMenuItem *srEventMenuItem)
{
	int			inPageLoop = _PAGE_LOOP_1_;
        int			inRetVal = VS_SUCCESS;
        int			inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
        int			inCount = 1;
        int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char			szKey = 0x00;
	char			szDemoMode[2 + 1] = {0};
	char			szCustomerIndicator[3 + 1] = {0};
	TRANSACTION_OBJECT	pobTran;			/* 只用來看有沒有安全認證，無實際用途 */
	
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_	, _FISC_SALE_		, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_3_1_FISC_SALE_	},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_	, _FISC_VOID_		, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_3_2_FISC_VOID_	},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_	, _FISC_REFUND_		, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_3_3_FISC_REFUND_	},
		{_Touch_NONE_					, _TRANS_TYPE_NULL_	, NULL						, ""					}
	};
	
	memset(szCustomerIndicator, 0x00 , sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (inMENU_Check_SMARTPAY_Enable(0) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		/* 教育訓練模式 */
		memset(szDemoMode, 0x00, sizeof(szDemoMode));
		inGetDemoMode(szDemoMode);
		if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
		{
			
		}
		else
		{
			/* SMARTPAY要GEN MAC來算TCC，一定要安全認證 */
			if (inKMS_CheckKey(_TWK_KEYSET_NCCC_, _TWK_KEYINDEX_NCCC_MAC_) != VS_SUCCESS)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_1_);			/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_SMARTPAY_TITLE_, 0, _COORDINATE_Y_LINE_8_2_);	/* 第一層顯示 Smartpay */

				memset(&pobTran, 0x00, sizeof(TRANSACTION_OBJECT));
				if (inNCCC_Func_CUP_PowerOn_LogOn(&pobTran) != VS_SUCCESS)
				{
					/* 安全認證失敗 */
					inRetVal = VS_ERROR;
					return (inRetVal);
				}
			}
		}
		
		/* 初始化 */
		inDISP_ClearAll();
		inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_SMARTPAY_TITLE_, 0, _COORDINATE_Y_LINE_8_2_);		/* 第二層顯示 Smartpay */
		inDISP_PutGraphic(_TOUCH_NEWUI_SMARTPAY_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);	
	
		if (ginHalfLCD == VS_FALSE)
			inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);
		
		/* 檢查功能開關，並顯示反白的圖 */
		inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);

		/* 設定Timeout */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
		}
		else
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
		}

		while (inPageLoop != _PAGE_LOOP_0_)
		{
			while (inPageLoop == _PAGE_LOOP_1_)
			{
				szKey = uszKBD_Key();
				inChioce1 = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc); /* 回傳MENU畫面的按鈕選擇 */
				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
				{
					szKey = _KEY_TIMEOUT_;
				}

				switch (inChioce1)
				{
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_:
						inCount = 1;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_:
						inCount = 2;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_:
						inCount = 3;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_RETURN_IDLE_BUTTON_:
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					case _NEWUI_LAST_PAGE_BUTTON_:
						inRetVal = VS_USER_CANCEL;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					case _DisTouch_Slide_Down_To_Up_:
						inRetVal = VS_USER_CANCEL;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					default:
						break;
				}

				if (szKey == _KEY_1_)
				{
					inCount = 1;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_2_)
				{
					inCount = 2;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_3_)
				{
					inCount = 3;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_CANCEL_)
				{
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				}
				else if (szKey == _KEY_TIMEOUT_)
				{
					inRetVal = VS_TIMEOUT;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				}

			}

			/* 判斷MENU點進去按鈕之後要做的事情 */
			if (inPageLoop == _PAGE_LOOP_2_)
			{
				switch (inCount)
				{
					case 1:
						inRetVal = inMENU_FISC_SALE(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 2:
						inRetVal = inMENU_FISC_VOID(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 3:
						inRetVal = inMENU_FISC_REFUND(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					default:
						inPageLoop = _PAGE_LOOP_0_;
						break;
				}
			}
			
			/* 代表回上一頁，要回復UI */
			if (inPageLoop == _PAGE_LOOP_1_		&&
			    inRetVal == VS_ERROR)
			{
				/* 初始化 */
				inDISP_ClearAll();
				inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_SMARTPAY_TITLE_, 0, _COORDINATE_Y_LINE_8_2_);		/* 第二層顯示 Smartpay */
				inDISP_PutGraphic(_TOUCH_NEWUI_SMARTPAY_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
			
				if (ginHalfLCD == VS_FALSE)
					inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);
				
				inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
				/* 設定Timeout */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
				{
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
				}
				else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
				}
				else
				{
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
				}
			}
			/* 功能未開而返回，不刷畫面 */
			else if (inPageLoop == _PAGE_LOOP_1_	&&
				 inRetVal == VS_SUCCESS)
			{

			}

		}/* _PAGE_LOOP_0_ */
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_NEWUI_ETICKET_MENU
Date&Time       :2017/10/25 下午 1:55
Describe        :
*/
int inMENU_NEWUI_ETICKET_MENU(EventMenuItem *srEventMenuItem)
{
	int			inPageLoop = _PAGE_LOOP_1_;
        int			inRetVal = VS_SUCCESS;
        int			inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
        int			inCount= 1;
        int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char			szKey = 0x00;
	char			szCustomerIndicator[3 + 1] = {0};
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_	, _TRANS_TYPE_NULL_		, inNCCC_Ticket_Func_Check_Transaction_Deduct		, _ICON_HIGHTLIGHT_4_1_DEDUCT_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_	, _TRANS_TYPE_NULL_		, inNCCC_Ticket_Func_Check_Transaction_Refund		, _ICON_HIGHTLIGHT_4_2_REFUND_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_	, _TRANS_TYPE_NULL_		, inNCCC_Ticket_Func_Check_Transaction_Inquiry		, _ICON_HIGHTLIGHT_4_3_INQUIRY_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_	, _TRANS_TYPE_NULL_		, inNCCC_Ticket_Func_Check_Transaction_Top_Up		, _ICON_HIGHTLIGHT_4_4_TOP_UP_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_	, _TRANS_TYPE_NULL_		, inNCCC_Ticket_Func_Check_Transaction_Void_Top_Up	, _ICON_HIGHTLIGHT_4_5_VOID_TOP_UP_	},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_	, _TRANS_TYPE_NULL_		, NULL							, ""					},
		{_Touch_NONE_					, _TRANS_TYPE_NULL_		, NULL							, ""					}
	};
	
	memset(szCustomerIndicator, 0x00 , sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (inMENU_Check_ETICKET_Enable(0) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		/* 初始化 */
		inDISP_ClearAll();
		srEventMenuItem->uszESVCLogOnDispBit = VS_TRUE;
		if (inNCCC_Ticket_Logon_ShowResult_Flow() != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		
		/* 若有票證未結帳，要提示訊息，比照520 2019/8/29 下午 1:46 */
		if (inNCCC_Ticket_Func_Must_SETTLE_Show_In_Menu() != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		
		inDISP_ClearAll();
		inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);	
		inDISP_PutGraphic(_TOUCH_NEWUI_ETICKET_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
	
		if (ginHalfLCD == VS_FALSE)
			inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);
		
		/* 檢查功能開關，並顯示反白的圖 */
		inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
		
		/* 設定Timeout */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
		}
		else
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
		}

		while (inPageLoop != _PAGE_LOOP_0_)
		{
			while (inPageLoop == _PAGE_LOOP_1_)
			{
				szKey = uszKBD_Key();
				inChioce1 = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc); /* 回傳MENU畫面的按鈕選擇 */
				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
				{
					szKey = _KEY_TIMEOUT_;
				}

				switch (inChioce1)
				{
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_:
						inCount = 1;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_:
						inCount = 2;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_:
						inCount = 3;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_:
						inCount = 4;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_:
						inCount = 5;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_:
						inCount = 6;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_RETURN_IDLE_BUTTON_:
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					case _NEWUI_LAST_PAGE_BUTTON_:
						inRetVal = VS_USER_CANCEL;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					case _DisTouch_Slide_Down_To_Up_:
						inRetVal = VS_USER_CANCEL;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					default:
						break;
				}

				if (szKey == _KEY_1_)
				{
					inCount = 1;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_2_)
				{
					inCount = 2;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_3_)
				{
					inCount = 3;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_4_)
				{
					inCount = 4;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_5_)
				{
					inCount = 5;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_6_)
				{
					inCount = 6;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_CANCEL_)
				{
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				}
				else if (szKey == _KEY_TIMEOUT_)
				{
					inRetVal = VS_TIMEOUT;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				}

			}

			/* 判斷MENU點進去按鈕之後要做的事情 */
			if (inPageLoop == _PAGE_LOOP_2_)
			{
				switch (inCount)
				{
					case 1:
						inRetVal = inMENU_ETICKET_DEDUCT(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 2:
						inRetVal = inMENU_ETICKET_REFUND(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 3:
						inRetVal = inMENU_ETICKET_INQUIRY(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 4:
						inRetVal = inMENU_ETICKET_TOP_UP(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 5:
						inRetVal = inMENU_ETICKET_VOID_TOP_UP(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 6:
						inRetVal = inMENU_ETICKET_ACTIVATION_INQUIRY(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					default:
						inPageLoop = _PAGE_LOOP_0_;
						break;
				}
			}
			/* 代表回上一頁，要回復UI */
			if (inPageLoop == _PAGE_LOOP_1_		&&
			    inRetVal == VS_ERROR)
			{
				/* 初始化 */
				inDISP_ClearAll();
				inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);	
				inDISP_PutGraphic(_TOUCH_NEWUI_ETICKET_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
			
				if (ginHalfLCD == VS_FALSE)
					inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);
				
				inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
				/* 設定Timeout */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
				{
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
				}
				else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
				}
				else
				{
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
				}
			}
			/* 功能未開而返回，不刷畫面 */
			else if (inPageLoop == _PAGE_LOOP_1_	&&
				 inRetVal == VS_SUCCESS)
			{

			}
		}/* _PAGE_LOOP_0_ */
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_NEWUI_AWARD_MENU
Date&Time       :2017/10/25 下午 1:55
Describe        :
*/
int inMENU_NEWUI_AWARD_MENU(EventMenuItem *srEventMenuItem)
{
	int			inPageLoop = _PAGE_LOOP_1_;
        int			inRetVal = VS_SUCCESS;
        int			inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
        int			inCount= 1;
        int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char			szKey = 0x00;
	char			szCustomerIndicator[3 + 1] = {0};
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_	, _LOYALTY_REDEEM_		, inNCCC_Loyalty_CreditCardFlag		, _ICON_HIGHTLIGHT_5_1_AWARD_SWIPE_	},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_	, _LOYALTY_REDEEM_		, inNCCC_Loyalty_BarCodeFlag		, _ICON_HIGHTLIGHT_5_2_AWARD_BARCODE_	},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_	, _VOID_LOYALTY_REDEEM_		, inNCCC_Loyalty_VoidRedeemFlag		, _ICON_HIGHTLIGHT_5_3_AWARD_VOID_	},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_	, _LOYALTY_REDEEM_REFUND_	, inNCCC_Loyalty_RefundFlag		, _ICON_HIGHTLIGHT_5_4_AWARD_REFUND_	},
		{_Touch_NONE_					, _TRANS_TYPE_NULL_		, NULL					, ""					}
	};
	
	memset(szCustomerIndicator, 0x00 , sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (inMENU_Check_AWARD_Enable(0) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		/* 初始化 */
		inDISP_ClearAll();
		inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_5_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);	
		inDISP_PutGraphic(_TOUCH_NEWUI_AWARD_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
	
		if (ginHalfLCD == VS_FALSE)
			inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);
		
		/* 檢查功能開關，並顯示反白的圖 */
		inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
	
		/* 設定Timeout */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
		}
		else
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
		}

		while (inPageLoop != _PAGE_LOOP_0_)
		{
			while (inPageLoop == _PAGE_LOOP_1_)
			{
				szKey = uszKBD_Key();
				inChioce1 = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc); /* 回傳MENU畫面的按鈕選擇 */
				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
				{
					szKey = _KEY_TIMEOUT_;
				}

				switch (inChioce1)
				{
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_:
						inCount = 1;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_:
						inCount = 2;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_:
						inCount = 3;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_:
						inCount = 4;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_RETURN_IDLE_BUTTON_:
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					case _NEWUI_LAST_PAGE_BUTTON_:
						inRetVal = VS_USER_CANCEL;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					case _DisTouch_Slide_Down_To_Up_:
						inRetVal = VS_USER_CANCEL;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					default:
						break;
				}

				if (szKey == _KEY_1_)
				{
					inCount = 1;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_2_)
				{
					inCount = 2;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_3_)
				{
					inCount = 3;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_4_)
				{
					inCount = 4;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_CANCEL_)
				{
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				}
				else if (szKey == _KEY_TIMEOUT_)
				{
					inRetVal = VS_TIMEOUT;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				}

			}

			/* 判斷MENU點進去按鈕之後要做的事情 */
			if (inPageLoop == _PAGE_LOOP_2_)
			{
				switch (inCount)
				{
					case 1:
						inRetVal = inMENU_AWARD_SWIPE(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 2:
						inRetVal = inMENU_AWARD_BARCODE(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 3:
						inRetVal = inMENU_AWARD_VOID(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 4:
						inRetVal = inMENU_AWARD_REFUND(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					default:
						inPageLoop = _PAGE_LOOP_0_;
						break;
				}
			}
			
			/* 代表回上一頁，要回復UI */
			if (inPageLoop == _PAGE_LOOP_1_		&&
			    inRetVal == VS_ERROR)
			{
				/* 初始化 */
				inDISP_ClearAll();
				inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_5_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);	
				inDISP_PutGraphic(_TOUCH_NEWUI_AWARD_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
			
				if (ginHalfLCD == VS_FALSE)
					inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);
				
				inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
				/* 設定Timeout */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
				{
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
				}
				else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
				}
				else
				{
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
				}
			}
			/* 功能未開而返回，不刷畫面 */
			else if (inPageLoop == _PAGE_LOOP_1_	&&
				 inRetVal == VS_SUCCESS)
			{

			}
			
		}/* _PAGE_LOOP_0_ */
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_NEWUI_HAPPYGO_MENU
Date&Time       :2017/10/25 下午 1:55
Describe        :
*/
int inMENU_NEWUI_HAPPYGO_MENU(EventMenuItem *srEventMenuItem)
{
	int			inPageLoop = _PAGE_LOOP_1_;
        int			inRetVal = VS_SUCCESS;
        int			inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
        int			inCount= 1;
        int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char			szKey = 0x00;
	char			szCustomerIndicator[3 + 1] = {0};
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_	, _HG_REWARD_			, inHG_Func_Customer_Check_Transaction_Function	, _ICON_HIGHTLIGHT_6_1_HG_REWARD_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_	, _HG_ONLINE_REDEEM_		, inHG_Func_Check_Transaction_Function          , _ICON_HIGHTLIGHT_6_2_HG_ONLINE_REDEEM_	},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_	, _HG_POINT_CERTAIN_		, inHG_Func_Check_Transaction_Function          , _ICON_HIGHTLIGHT_6_3_HG_POINT_CERTAIN_	},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_	, _HG_FULL_REDEMPTION_		, inHG_Func_Check_Transaction_Function          , _ICON_HIGHTLIGHT_6_4_HG_FULL_REDEMPTION_	},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_	, _HG_INQUIRY_			, inHG_Func_Check_Transaction_Function          , _ICON_HIGHTLIGHT_6_5_HG_INQUIRY_		},
		{_Touch_NONE_			, _TRANS_TYPE_NULL_		, NULL					, ""						}
	};
	
	memset(szCustomerIndicator, 0x00 , sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (inMENU_Check_HG_Enable(0) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		/* 初始化 */
		inDISP_ClearAll();
		inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_6_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);	
		inDISP_PutGraphic(_TOUCH_NEWUI_HAPPYGO_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
		
		if (ginHalfLCD == VS_FALSE)
			inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);
                
        /* 檢查功能開關，並顯示反白的圖 */
		inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);

		/* 設定Timeout */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
		}
		else
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
		}

		while (inPageLoop != _PAGE_LOOP_0_)
		{
			while (inPageLoop == _PAGE_LOOP_1_)
			{
				szKey = uszKBD_Key();
				inChioce1 = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc); /* 回傳MENU畫面的按鈕選擇 */
				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
				{
					szKey = _KEY_TIMEOUT_;
				}

				switch (inChioce1)
				{
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_:
						inCount = 1;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_:
						inCount = 2;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_:
						inCount = 3;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_:
						inCount = 4;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_:
						inCount = 5;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_RETURN_IDLE_BUTTON_:
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					case _NEWUI_LAST_PAGE_BUTTON_:
						inRetVal = VS_USER_CANCEL;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					case _DisTouch_Slide_Down_To_Up_:
						inRetVal = VS_USER_CANCEL;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					default:
						break;
				}

				if (szKey == _KEY_1_)
				{
					inCount = 1;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_2_)
				{
					inCount = 2;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_3_)
				{
					inCount = 3;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_4_)
				{
					inCount = 4;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_5_)
				{
					inCount = 5;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_CANCEL_)
				{
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				}
				else if (szKey == _KEY_TIMEOUT_)
				{
					inRetVal = VS_TIMEOUT;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				}

			}

			/* 判斷MENU點進去按鈕之後要做的事情 */
			if (inPageLoop == _PAGE_LOOP_2_)
			{
				switch (inCount)
				{
					case 1:
						inRetVal = inMENU_HG_REWARD(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 2:
						inRetVal = inMENU_HG_ONLINE_REDEEM(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 3:
						inRetVal = inMENU_HG_POINT_CERTAIN(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 4:
						inRetVal = inMENU_HG_FULL_REDEMPTION(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 5:
						inRetVal = inMENU_HG_INQUIRY(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else if (inRetVal == VS_FUNC_CLOSE_ERR)
						{
							inRetVal = VS_SUCCESS;
							inPageLoop = _PAGE_LOOP_1_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					default:
						inPageLoop = _PAGE_LOOP_0_;
						break;
				}
			}
			
			/* 代表回上一頁，要回復UI */
			if (inPageLoop == _PAGE_LOOP_1_		&&
			    inRetVal == VS_ERROR)
			{
				/* 初始化 */
				inDISP_ClearAll();
				inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_6_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);	
				inDISP_PutGraphic(_TOUCH_NEWUI_HAPPYGO_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
			
				if (ginHalfLCD == VS_FALSE)
					inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

				
				inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
				/* 設定Timeout */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
				{
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
				}
				else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
				}
				else
				{
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
				}
			}
			/* 功能未開而返回，不刷畫面 */
			else if (inPageLoop == _PAGE_LOOP_1_	&&
				 inRetVal == VS_SUCCESS)
			{

			}

		}/* _PAGE_LOOP_0_ */
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_NEWUI_SETTLE_MENU
Date&Time       :2017/10/25 下午 1:55
Describe        :
*/
int inMENU_NEWUI_SETTLE_MENU(EventMenuItem *srEventMenuItem)
{
	int			inPageLoop = _PAGE_LOOP_1_;
        int			inRetVal = VS_SUCCESS;
        int			inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
        int			inCount= 1;
        int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char			szKey = 0x00;
	char			szCustomerIndicator[3 + 1] = {0};
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_Touch_NONE_				, _TRANS_TYPE_NULL_	, NULL						, ""					}
	};
	
	memset(szCustomerIndicator, 0x00 , sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (inMENU_Check_SETTLE_Enable(0) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		/* 初始化 */
		inDISP_ClearAll();
		inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_7_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
		inDISP_PutGraphic(_TOUCH_NEWUI_SETTLE_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);

		if (ginHalfLCD == VS_FALSE)
			inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

		/* 設定Timeout */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
		}
		else
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
		}

		while (inPageLoop != _PAGE_LOOP_0_)
		{
			while (inPageLoop == _PAGE_LOOP_1_)
			{
				szKey = uszKBD_Key();
				inChioce1 = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc); /* 回傳MENU畫面的按鈕選擇 */
				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
				{
					szKey = _KEY_TIMEOUT_;
				}

				switch (inChioce1)
				{
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_:
						inCount = 1;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_:
						inCount = 2;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_RETURN_IDLE_BUTTON_:
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					case _NEWUI_LAST_PAGE_BUTTON_:
						inRetVal = VS_USER_CANCEL;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					case _DisTouch_Slide_Down_To_Up_:
						inRetVal = VS_USER_CANCEL;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					default:
						break;
				}

				if (szKey == _KEY_1_)
				{
					inCount = 1;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_2_)
				{
					inCount = 2;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_CANCEL_)
				{
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				}
				else if (szKey == _KEY_TIMEOUT_)
				{
					inRetVal = VS_TIMEOUT;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				}

			}

			/* 判斷MENU點進去按鈕之後要做的事情 */
			if (inPageLoop == _PAGE_LOOP_2_)
			{
				switch (inCount)
				{
					case 1:
						inRetVal = inMENU_SETTLE_AUTOSETTLE(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 2:
						inRetVal = inMENU_SETTLE_BY_HOST(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					default:
						inPageLoop = _PAGE_LOOP_0_;
						break;
				}
			}

			/* 代表回上一頁，要回復UI */
			if (inPageLoop == _PAGE_LOOP_1_		&&
			    inRetVal == VS_ERROR)
			{
				/* 初始化 */
				inDISP_ClearAll();
				inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_7_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);	
				inDISP_PutGraphic(_TOUCH_NEWUI_SETTLE_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
			
				if (ginHalfLCD == VS_FALSE)
					inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

				inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
				/* 設定Timeout */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
				{
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
				}
				else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
				}
				else
				{
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
				}
			}
			/* 功能未開而返回，不刷畫面 */
			else if (inPageLoop == _PAGE_LOOP_1_	&&
				 inRetVal == VS_SUCCESS)
			{

			}

		}/* _PAGE_LOOP_0_ */
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_NEWUI_REVIEW_PRINT_MENU
Date&Time       :2017/10/25 下午 1:55
Describe        :
*/
int inMENU_NEWUI_REVIEW_PRINT_MENU(EventMenuItem *srEventMenuItem)
{
	int			inPageLoop = _PAGE_LOOP_1_;
        int			inRetVal = VS_SUCCESS;
        int			inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
        int			inCount= 1;
        int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char			szKey = 0x00;
	char			szCustomerIndicator[3 + 1] = {0};
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_Touch_NONE_				, _TRANS_TYPE_NULL_	, NULL						, ""					}
	};
	
	memset(szCustomerIndicator, 0x00 , sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (inMENU_Check_REVIEW_SETTLE_Enable(0) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		/* 初始化 */
		inDISP_ClearAll();
		inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_8_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
		inDISP_PutGraphic(_TOUCH_NEWUI_REVIEW_PRINT_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
	
		if (ginHalfLCD == VS_FALSE)
			inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

		/* 設定Timeout */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
		}
		else
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
		}

		while (inPageLoop != _PAGE_LOOP_0_)
		{
			while (inPageLoop == _PAGE_LOOP_1_)
			{
				szKey = uszKBD_Key();
				inChioce1 = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc); /* 回傳MENU畫面的按鈕選擇 */
				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
				{
					szKey = _KEY_TIMEOUT_;
				}

				switch (inChioce1)
				{
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_:
						inCount = 1;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_:
						inCount = 2;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_:
						inCount = 3;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_:
						inCount = 4;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_:
						inCount = 5;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_:
						inCount = 6;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_RETURN_IDLE_BUTTON_:
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					case _NEWUI_LAST_PAGE_BUTTON_:
						inRetVal = VS_USER_CANCEL;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					case _DisTouch_Slide_Down_To_Up_:
						inRetVal = VS_USER_CANCEL;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					default:
						break;
				}

				if (szKey == _KEY_1_)
				{
					inCount = 1;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_2_)
				{
					inCount = 2;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_3_)
				{
					inCount = 3;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_4_)
				{
					inCount = 4;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_5_)
				{
					inCount = 5;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_6_)
				{
					inCount = 6;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_CANCEL_)
				{
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				}
				else if (szKey == _KEY_TIMEOUT_)
				{
					inRetVal = VS_TIMEOUT;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				}

			}

			/* 判斷MENU點進去按鈕之後要做的事情 */
			if (inPageLoop == _PAGE_LOOP_2_)
			{
				switch (inCount)
				{
					case 1:
						inRetVal = inMENU_REPRINT(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 2:
						inRetVal = inMENU_TOTAL_REVIEW(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 3:
						inRetVal = inMENU_DETAIL_REVIEW(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 4:
						inRetVal = inMENU_DCC_RATE(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 5:
						inRetVal = inMENU_TOTAL_REPORT(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					case 6:
						inRetVal = inMENU_DETAIL_REPORT(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
						break;
					default:
						inPageLoop = _PAGE_LOOP_0_;
						break;
				}
			}

			/* 代表回上一頁，要回復UI */
			if (inPageLoop == _PAGE_LOOP_1_		&&
			    inRetVal == VS_ERROR)
			{
				/* 初始化 */
				inDISP_ClearAll();
				inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_8_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
				inDISP_PutGraphic(_TOUCH_NEWUI_REVIEW_PRINT_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);	
			
				if (ginHalfLCD == VS_FALSE)
					inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

				inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
				/* 設定Timeout */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
				{
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
				}
				else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
				}
				else
				{
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
				}
			}
			/* 功能未開而返回，不刷畫面 */
			else if (inPageLoop == _PAGE_LOOP_1_	&&
				 inRetVal == VS_SUCCESS)
			{

			}

		}/* _PAGE_LOOP_0_ */
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_NEWUI_SETTING_MENU
Date&Time       :2017/10/25 下午 1:55
Describe        :
*/
int inMENU_NEWUI_SETTING_MENU(EventMenuItem *srEventMenuItem)
{
	int			inPageLoop = _PAGE_LOOP_1_;
        int			inRetVal = VS_SUCCESS;
        int			inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
        int			inCount= 1;
        int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char			szKey = 0x00;
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_Touch_NONE_				, _TRANS_TYPE_NULL_	, NULL						, ""					}
	};
	
        /* 初始化 */
	inDISP_ClearAll();
	inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_9_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);	
	inDISP_PutGraphic(_TOUCH_NEWUI_SETTING_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);

	if (ginHalfLCD == VS_FALSE)
		inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);
        
	/* 設定Timeout */
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
		
	while (inPageLoop != _PAGE_LOOP_0_)
	{
		while (inPageLoop == _PAGE_LOOP_1_)
		{
			szKey = uszKBD_Key();
			inChioce1 = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc); /* 回傳MENU畫面的按鈕選擇 */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}
			
			switch (inChioce1)
			{
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_:
					inCount = 1;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_:
					inCount = 2;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_:
					inCount = 3;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_:
					inCount = 4;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_:
					inCount = 5;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_RETURN_IDLE_BUTTON_:
					inRetVal = VS_ERROR;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				case _NEWUI_LAST_PAGE_BUTTON_:
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				case _DisTouch_Slide_Down_To_Up_:
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				default:
					break;
			}

			if (szKey == _KEY_1_)
			{
				inCount = 1;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_2_)
			{
				inCount = 2;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_3_)
			{
				inCount = 3;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_4_)
			{
				inCount = 4;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_5_)
			{
				inCount = 5;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				inPageLoop = _PAGE_LOOP_0_;
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				inPageLoop = _PAGE_LOOP_0_;
				break;
			}
			
		}

		/* 判斷MENU點進去按鈕之後要做的事情 */
		if (inPageLoop == _PAGE_LOOP_2_)
		{
			switch (inCount)
			{
				case 1:
					inRetVal = inMENU_EDIT_PASSWORD(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_0_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 2:
					inRetVal = inMENU_TRACELOG_UP(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_0_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 3:
					inRetVal = inMENU_Download_Parameter(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_0_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 4:
					inRetVal = inMENU_CHECK_VERSION(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_0_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 5:
					inRetVal = inMENU_NEWUI_OTHER_MENU(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				default:
					inPageLoop = _PAGE_LOOP_0_;
					break;
			}
		}
		
		/* 代表回上一頁，要回復UI */
		if (inPageLoop == _PAGE_LOOP_1_		&&
		    inRetVal == VS_ERROR)
		{
			/* 初始化 */
			inDISP_ClearAll();
			inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_9_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
			inDISP_PutGraphic(_TOUCH_NEWUI_SETTING_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);

			if (ginHalfLCD == VS_FALSE)
				inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);
	
			inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
		}
		/* 功能未開而返回，不刷畫面 */
		else if (inPageLoop == _PAGE_LOOP_1_	&&
			 inRetVal == VS_SUCCESS)
		{

		}
		
	}/* _PAGE_LOOP_0_ */
	
	return (inRetVal);
}

/*
Function        :inMENU_NEWUI_OTHER_MENU
Date&Time       :2017/10/30 下午 5:44
Describe        :
*/
int inMENU_NEWUI_OTHER_MENU(EventMenuItem *srEventMenuItem)
{
	int			inPageLoop = _PAGE_LOOP_1_;
        int			inRetVal = VS_SUCCESS;
        int			inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
        int			inCount= 1;
        int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char			szKey = 0x00;
	char			szCustomerIndicator[3 + 1] = {0};
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_Touch_NONE_				, _TRANS_TYPE_NULL_	, NULL						, ""					}
	};
	
	memset(szCustomerIndicator, 0x00 , sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
        /* 初始化 */
	inDISP_ClearAll();
	inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_10_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
	inDISP_PutGraphic(_TOUCH_NEWUI_OTHER_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);	

	if (ginHalfLCD == VS_FALSE)
		inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

	/* 設定Timeout */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
	}
	else
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	}
		
	while (inPageLoop != _PAGE_LOOP_0_)
	{
		while (inPageLoop == _PAGE_LOOP_1_)
		{
			szKey = 0;
			szKey = uszKBD_Key();
			inChioce1 = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc); /* 回傳MENU畫面的按鈕選擇 */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}
			
			switch (inChioce1)
			{
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_:
					inCount = 1;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_:
					inCount = 2;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_:
					inCount = 3;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_:
					inCount = 4;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_RETURN_IDLE_BUTTON_:
					inRetVal = VS_ERROR;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				case _NEWUI_LAST_PAGE_BUTTON_:
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				case _DisTouch_Slide_Down_To_Up_:
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				default:
					break;
			}

			if (szKey == _KEY_1_)
			{
				inCount = 1;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_2_)
			{
				inCount = 2;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_3_)
			{
				inCount = 3;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_4_)
			{
				inCount = 4;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_0_)
			{
				inCount = 0;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				inPageLoop = _PAGE_LOOP_0_;
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				inPageLoop = _PAGE_LOOP_0_;
				break;
			}
			
		}

		/* 判斷MENU點進去按鈕之後要做的事情 */
		if (inPageLoop == _PAGE_LOOP_2_)
		{
			switch (inCount)
			{
				case 1:
					inRetVal = inMENU_COMM_MENU(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_0_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 2:
					inRetVal = inMENU_TMS_PARAMETER_DOWNLOAD(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_0_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 3:
					inRetVal = inMENU_DCC_PARAMETER_DOWNLOAD(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_0_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 4:
					inRetVal = inMENU_TMS_TASK_REPORT(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_0_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 0:
					/* 壓住0後3秒內按clear */
					inDISP_Timer_Start(_TIMER_NEXSYS_4_, 3);
					do
					{
						szKey = uszKBD_Key_In();
					}while (szKey == 0	&&
						inTimerGet(_TIMER_NEXSYS_4_) != VS_SUCCESS);
					
					/* 不是按clear，不能進隱藏選單 */
					if (szKey != _KEY_CLEAR_)
					{
						inRetVal = VS_SUCCESS;
						inPageLoop = _PAGE_LOOP_1_;
						break;
					}
					
					inFlushKBDBuffer();
					
					
					srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
					srEventMenuItem->inCode = FALSE;
					/* 輸入管理號碼 */
					inDISP_ClearAll();
					if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_0_;
					}
					else
					{
						inRetVal = inMENU_NEWUI_FUNCTION1_MENU(srEventMenuItem);
						if (inRetVal == VS_USER_CANCEL)
						{
							inRetVal = VS_ERROR;
							inPageLoop = _PAGE_LOOP_0_;
						}
						else
						{
							inPageLoop = _PAGE_LOOP_0_;
						}
					}
					
					
					break;
				default:
					inPageLoop = _PAGE_LOOP_0_;
					break;
			}
		}
		
		/* 代表回上一頁，要回復UI */
		if (inPageLoop == _PAGE_LOOP_1_		&&
		    inRetVal == VS_ERROR)
		{
			/* 初始化 */
			inDISP_ClearAll();
			inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_10_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);
			inDISP_PutGraphic(_TOUCH_NEWUI_OTHER_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);	
		
			if (ginHalfLCD == VS_FALSE)
				inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);
	
			inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
			/* 設定Timeout */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
			}
			else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
			}
			else
			{
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
			}
		}
		/* 功能未開而返回，不刷畫面 */
		else if (inPageLoop == _PAGE_LOOP_1_	&&
			 inRetVal == VS_SUCCESS)
		{

		}
		
	}/* _PAGE_LOOP_0_ */
	
	return (inRetVal);
}

/*
Function        :inMENU_NEWUI_FUNCTION1_MENU
Date&Time       :2018/5/21 下午 4:04
Describe        :
*/
int inMENU_NEWUI_FUNCTION1_MENU(EventMenuItem *srEventMenuItem)
{
	int			inPageLoop = _PAGE_LOOP_1_;
        int			inRetVal = VS_SUCCESS;
        int			inChioce1 = _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_1_;
        int			inCount= 1;
        int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_2_TO_7_3X4_;
	char			szKey = 0x00;
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_Touch_NONE_				, _TRANS_TYPE_NULL_	, NULL						, ""					}
	};
	
        /* 初始化 */
	inDISP_ClearAll();
	inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_TOUCH_NEWUI_ENGINEER_FUNCTION_PAGE_, 0,  _COORDINATE_Y_LINE_8_2_);

	if (ginHalfLCD == VS_FALSE)
		inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);
	
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);

	/* 設定Timeout */
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		
	while (inPageLoop != _PAGE_LOOP_0_)
	{
		while (inPageLoop == _PAGE_LOOP_1_)
		{
			szKey = 0;
			szKey = uszKBD_Key();
			inChioce1 = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc); /* 回傳MENU畫面的按鈕選擇 */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}
			
			switch (inChioce1)
			{
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_1_:
					inCount = 1;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_2_:
					inCount = 2;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_3_:
					inCount = 3;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_4_:
					inCount = 4;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_5_:
					inCount = 5;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_6_:
					inCount = 6;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_7_:
					inCount = 7;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_8_:
					inCount = 8;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_9_:
					inCount = 9;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_10_:
					inCount = 10;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_11_:
					inCount = 11;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_12_:
					inCount = 12;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_RETURN_IDLE_BUTTON_:
					inRetVal = VS_ERROR;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				case _DisTouch_Slide_Down_To_Up_:
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				default:
					break;
			}

			if (szKey == _KEY_UP_ ||
			    szKey == _KEY_UP_HALF_) /*向上按鍵*/
			{
				inCount--;

				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_3_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_3_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_3_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_11_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_11_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↓", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_3_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_Y2_, VS_FALSE);
				
				switch (inCount)
				{
					case 1:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);
						break;
					case 2:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);
						break;
					case 3:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_3_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);
						break;
					case 4:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y2_, VS_FALSE);
						break;
					case 5:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y2_, VS_FALSE);
						break;
					case 6:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_3_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y2_, VS_FALSE);
						break;
					case 7:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_Y2_, VS_FALSE);
						break;
					case 8:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_Y2_, VS_FALSE);
						break;
					case 9:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_3_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_Y2_, VS_FALSE);
						break;
					case 10:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_11_Y2_, VS_FALSE);
						break;
					case 11:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_11_Y2_, VS_FALSE);
						break;
					default:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);
						inCount = 1;
						break;
				}
			}
			else if (szKey == _KEY_DOWN_ ||
				 szKey == _KEY_DOWN_HALF_) /*向下按鍵*/
			{
				inCount++;
				
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_3_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_3_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_3_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_11_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_11_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↓", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_3_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_Y2_, VS_FALSE);
				
				switch (inCount)
				{
					case 2:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);
						break;
					case 3:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_3_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);
						break;
					case 4:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y2_, VS_FALSE);
						break;
					case 5:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y2_, VS_FALSE);
						break;
					case 6:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_3_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y2_, VS_FALSE);
						break;
					case 7:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_Y2_, VS_FALSE);
						break;
					case 8:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_Y2_, VS_FALSE);
						break;
					case 9:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_3_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_Y2_, VS_FALSE);
						break;
					case 10:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_11_Y2_, VS_FALSE);
						break;
					case 11:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_11_Y2_, VS_FALSE);
						break;
					case 12:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↓", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_3_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_Y2_, VS_FALSE);
						break;
					default:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↓", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_3_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_Y2_, VS_FALSE);
						inCount = 12;
						break;
				}
			}
			else if (szKey == _KEY_1_)
			{
				inCount = 1;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_2_)
			{
				inCount = 2;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_3_)
			{
				inCount = 3;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_4_)
			{
				inCount = 4;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_5_)
			{
				inCount = 5;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_6_)
			{
				inCount = 6;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_7_)
			{
				inCount = 7;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_8_)
			{
				inCount = 8;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_9_)
			{
				inCount = 9;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_0_)
			{
				inCount = 10;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				inPageLoop = _PAGE_LOOP_0_;
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				inPageLoop = _PAGE_LOOP_0_;
				break;
			}
			else if (szKey == _KEY_ENTER_)
			{
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			
		}

		/* 判斷MENU點進去按鈕之後要做的事情 */
		if (inPageLoop == _PAGE_LOOP_2_)
		{
			switch (inCount)
			{
				case 1:
					/* TMS參數列印 */
					inRetVal = inMENU_TMS_PARAMETER_PRINT(srEventMenuItem);
					inPageLoop = _PAGE_LOOP_0_;
					break;
				case 2:
					/* DCC參數列印 */
					inRetVal = inMENU_DCC_PARAMETER_PRINT(srEventMenuItem);
					inPageLoop = _PAGE_LOOP_0_;
					break;
				case 3:
					/* 清除批次 */
					inRetVal = inMENU_DELETE_BATCH(srEventMenuItem);
					inPageLoop = _PAGE_LOOP_0_;
					break;
				case 4:
					/* 註冊SAM卡 */
					inRetVal = inMENU_SAM_REGISTER(srEventMenuItem);
					inPageLoop = _PAGE_LOOP_0_;
					break;
				case 5:
					/* Debug開關 */
					inRetVal = inMENU_DEBUG_SWITCH(srEventMenuItem);
					/* 回上一頁 */
					inRetVal = VS_ERROR;
					inPageLoop = _PAGE_LOOP_1_;
					break;
				case 6:
					/* 時間設定 */
					inRetVal = inMENU_TIME_SETTING(srEventMenuItem);
					/* 回上一頁 */
					inRetVal = VS_ERROR;
					inPageLoop = _PAGE_LOOP_1_;
					break;
				case 7:
					/* 修改TMS版本 */
					inRetVal = inMENU_EDIT_TMEP_VERSION_ID(srEventMenuItem);
					/* 回上一頁 */
					inRetVal = VS_ERROR;
					inPageLoop = _PAGE_LOOP_1_;
					break;
				case 8:
					/* 修改TMS完成 */
					inRetVal = inMENU_EDIT_TMSOK(srEventMenuItem);
					/* 回上一頁 */
					inRetVal = VS_ERROR;
					inPageLoop = _PAGE_LOOP_1_;
					break;
				case 9:
					/* EDC解鎖 */
					inRetVal = inMENU_UNLOCK_EDC(srEventMenuItem);
					/* 回上一頁 */
					inRetVal = VS_ERROR;
					inPageLoop = _PAGE_LOOP_1_;
					break;
				case 10:
					inRetVal = inMENU_NEWUI_FUNCTION2_MENU(srEventMenuItem);
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 11:
					inRetVal = inMENU_TRACELOG_MENU(srEventMenuItem);
					/* 回上一頁 */
					inPageLoop = _PAGE_LOOP_0_;
					break;
				case 12:
					/* 修改DEMO Mode */
					inRetVal = inMENU_DEMO_SETTING(srEventMenuItem, VS_FALSE);
					/* 回上一頁 */
					inPageLoop = _PAGE_LOOP_0_;					
					break;
				default:
					inRetVal = VS_SUCCESS;
					inPageLoop = _PAGE_LOOP_1_;
					break;
			}
		}
		
		/* 代表回上一頁，要回復UI */
		if (inPageLoop == _PAGE_LOOP_1_		&&
		    inRetVal == VS_ERROR)
		{
			/* 初始化 */
			inDISP_ClearAll();
			inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_TOUCH_NEWUI_ENGINEER_FUNCTION_PAGE_, 0,  _COORDINATE_Y_LINE_8_2_);	
		
			if (ginHalfLCD == VS_FALSE)
				inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);
			
			inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
			
			inCount = 1;
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		}
		/* 功能未開而返回，不刷畫面 */
		else if (inPageLoop == _PAGE_LOOP_1_	&&
			 inRetVal == VS_SUCCESS)
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		}
		
	}/* _PAGE_LOOP_0_ */
	
	return (inRetVal);
}

/*
Function        :inMENU_NEWUI_FUNCTION2_MENU
Date&Time       :2018/5/21 下午 4:04
Describe        :
*/
int inMENU_NEWUI_FUNCTION2_MENU(EventMenuItem *srEventMenuItem)
{
	int			inPageLoop = _PAGE_LOOP_1_;
        int			inRetVal = VS_SUCCESS;
        int			inChioce1 = _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_1_;
        int			inCount= 1;
        int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_2_TO_7_3X4_;
	char			szKey = 0x00;
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_Touch_NONE_				, _TRANS_TYPE_NULL_	, NULL						, ""					}
	};
	
        /* 初始化 */
	inDISP_ClearAll();
	inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_TOUCH_NEWUI_ENGINEER_FUNCTION_PAGE2_, 0,  _COORDINATE_Y_LINE_8_2_);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode("■■■", _FONTSIZE_8X22_, _COLOR_BUTTON_GREEN_, _COLOR_BUTTON_GREEN_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_5_Y1_, VS_FALSE);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode("外勤", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_BUTTON_GREEN_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_5_Y1_, VS_FALSE);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode("工程師", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_BUTTON_GREEN_, _COORDINATE_X_CHOOSE_HOST_2_, ((_Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_5_Y1_ + _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_5_Y2_) / 2), VS_FALSE);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode("■■■", _FONTSIZE_8X22_, _COLOR_BUTTON_GREEN_, _COLOR_BUTTON_GREEN_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_8_Y1_, VS_FALSE);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode("測試", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_BUTTON_GREEN_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_8_Y1_, VS_FALSE);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode("人員", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_BUTTON_GREEN_, _COORDINATE_X_CHOOSE_HOST_2_, ((_Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_8_Y1_ + _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_8_Y2_) / 2), VS_FALSE);

	if (ginHalfLCD == VS_FALSE)
		inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

	inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);
	
	/* 設定Timeout */
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		
	while (inPageLoop != _PAGE_LOOP_0_)
	{
		while (inPageLoop == _PAGE_LOOP_1_)
		{
			szKey = 0;
			szKey = uszKBD_Key();
			inChioce1 = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc); /* 回傳MENU畫面的按鈕選擇 */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}
			
			switch (inChioce1)
			{
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_1_:
					inCount = 1;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_2_:
					inCount = 2;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_3_:
					inCount = 3;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_4_:
					inCount = 4;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_5_:
					inCount = 5;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_6_:
					inCount = 6;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_7_:
					inCount = 7;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_8_:
					inCount = 8;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				case _NEWUI_RETURN_IDLE_BUTTON_:
					inRetVal = VS_ERROR;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				case _NEWUI_LAST_PAGE_BUTTON_:
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				case _DisTouch_Slide_Down_To_Up_:
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				default:
					break;
			}

			if (szKey == _KEY_UP_ ||
		            szKey == _KEY_UP_HALF_) /*向上按鍵*/
			{
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_2_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_3_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_3_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y2_, VS_FALSE);
				
				inCount--;
				switch (inCount)
				{
					case 1:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);
						break;
					case 2:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_2_Y2_, VS_FALSE);
						break;
					case 3:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_3_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_3_Y2_, VS_FALSE);
						break;
					case 4:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y2_, VS_FALSE);
						break;
					default:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);
						inCount = 1;
						break;
				}
			}
			else if (szKey == _KEY_DOWN_ ||
				 szKey == _KEY_DOWN_HALF_) /*向下按鍵*/
			{
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_2_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_3_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_3_Y2_, VS_FALSE);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y2_, VS_FALSE);
				
				inCount++;
				switch (inCount)
				{
					case 2:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_2_Y2_, VS_FALSE);
						break;
					case 3:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_3_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_3_Y2_, VS_FALSE);
						break;
					case 4:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y2_, VS_FALSE);
						break;
					default:
						inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y2_, VS_FALSE);
						inCount = 4;
						break;
				}
			}
			else if (szKey == _KEY_1_)
			{
				inCount = 1;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_2_)
			{
				inCount = 2;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_3_)
			{
				inCount = 3;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_4_)
			{
				inCount = 4;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_5_)
			{
				inCount = 5;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_6_)
			{
				inCount = 6;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_7_)
			{
				inCount = 7;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_8_)
			{
				inCount = 8;
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			else if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				inPageLoop = _PAGE_LOOP_0_;
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				inPageLoop = _PAGE_LOOP_0_;
				break;
			}
			else if (szKey == _KEY_ENTER_)
			{
				inPageLoop = _PAGE_LOOP_2_;
				break;
			}
			
		}

		/* 判斷MENU點進去按鈕之後要做的事情 */
		if (inPageLoop == _PAGE_LOOP_2_)
		{
			switch (inCount)
			{
				case 1:
					inRetVal = inMENU_REBOOT(srEventMenuItem);			/* 重新開機 */
					inPageLoop = _PAGE_LOOP_0_;
					break;
				case 2:
					inRetVal = inMENU_KEY_FUNCTION(srEventMenuItem);	/* 下載銀聯測試Key */
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
					break;
				case 3:
					inRetVal = inMENU_EXIT_AP(srEventMenuItem);
					inRetVal = VS_ERROR;
					inPageLoop = _PAGE_LOOP_1_;
					break;
				case 4:
					inRetVal = inMENU_CHECK_FILE_Flow(srEventMenuItem);
					inRetVal = VS_ERROR;
					inPageLoop = _PAGE_LOOP_1_;
					break;
				case 5:
					inRetVal = inMENU_Engineer_Fuction(srEventMenuItem);
					inRetVal = VS_ERROR;
					inPageLoop = _PAGE_LOOP_1_;
					break;
				case 6:
					inRetVal = inMENU_Developer_Fuction_Flow(srEventMenuItem);
					inRetVal = VS_ERROR;
					inPageLoop = _PAGE_LOOP_1_;
					break;
				case 7:
					inRetVal = inMENU_DEMO_SETTING(srEventMenuItem, VS_TRUE);
					inRetVal = VS_ERROR;
					inPageLoop = _PAGE_LOOP_1_;
					break;
				case 8:
					inRetVal = inMENU_QA_Fuction_Flow(srEventMenuItem);
					inRetVal = VS_ERROR;
					inPageLoop = _PAGE_LOOP_1_;
					break;
				default:
					inRetVal = VS_SUCCESS;
					inPageLoop = _PAGE_LOOP_1_;
					break;
			}
		}
		
		/* 代表回上一頁，要回復UI */
		if (inPageLoop == _PAGE_LOOP_1_		&&
		    inRetVal == VS_ERROR)
		{
			/* 初始化 */
			inDISP_ClearAll();
			inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_TOUCH_NEWUI_ENGINEER_FUNCTION_PAGE2_, 0,  _COORDINATE_Y_LINE_8_2_);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("■■■", _FONTSIZE_8X22_, _COLOR_BUTTON_GREEN_, _COLOR_BUTTON_GREEN_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_5_Y1_, VS_FALSE);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("外勤", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_BUTTON_GREEN_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_5_Y1_, VS_FALSE);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("工程師", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_BUTTON_GREEN_, _COORDINATE_X_CHOOSE_HOST_2_, ((_Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_5_Y1_ + _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_5_Y2_) / 2), VS_FALSE);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("■■■", _FONTSIZE_8X22_, _COLOR_BUTTON_GREEN_, _COLOR_BUTTON_GREEN_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_8_Y1_, VS_FALSE);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("測試", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_BUTTON_GREEN_, _COORDINATE_X_CHOOSE_HOST_2_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_8_Y1_, VS_FALSE);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("人員", _FONTSIZE_32X22_, _COLOR_WHITE_, _COLOR_BUTTON_GREEN_, _COORDINATE_X_CHOOSE_HOST_2_, ((_Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_8_Y1_ + _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_8_Y2_) / 2), VS_FALSE);
		
			if (ginHalfLCD == VS_FALSE)
				inDISP_PutGraphic(_MSG_RETURN_LAST_OR_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

			inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
			inCount = 1;
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("↑", _FONTSIZE_32X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_CHOOSE_HOST_1_, _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_, VS_FALSE);
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		}
		/* 功能未開而返回，不刷畫面 */
		else if (inPageLoop == _PAGE_LOOP_1_	&&
			 inRetVal == VS_SUCCESS)
		{

		}
		
	}/* _PAGE_LOOP_0_ */
	
	return (inRetVal);
}

/*
Function        :inMENU_DEMO_SETTING
Date&Time       :2019/9/24 下午 1:59
Describe        :教育訓練模式開關
*/
int inMENU_DEMO_SETTING(EventMenuItem *srEventMenuItem, VS_BOOL fSuperEntry)
{
	int	inRetVal = VS_SUCCESS;
	char	szDemoMode[2 + 1] = {0};
	char	szTMSOK[2 + 1] = {0};
	char	szTemplate[32 + 1];
	DISPLAY_OBJECT  srDispObj;

	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);	/* 第一層顯示 LOGO */
	/* 隱藏功能不顯示Title */
	if (fSuperEntry == VS_TRUE)
	{
		
	}
	else
	{
		inDISP_PutGraphic(_MENU_DEMO_SETTING_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 教育訓練模式 */
	}
	
	inLoadEDCRec(0);
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	
	if (szDemoMode[0] != 'Y' && szTMSOK[0] != 'Y')
	{
		/* TMSOK是0 Demo mode沒開才往下跑 */
	}
	else if (fSuperEntry == VS_TRUE)
	{
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC2_PASSWORD_;
		srEventMenuItem->inCode = FALSE;
		/* 輸入管理號碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);		
	}
	else
	{
                /* 此功能已關閉 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_FUNC_CLOSE_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
                inDISP_Msg_BMP(&srDispMsgObj);
                return (VS_ERROR);
	}
	
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
		inDISP_ChineseFont("DEMO 開關 : ON", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
	else
		inDISP_ChineseFont("DEMO 開關 : OFF", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);

        inDISP_ChineseFont("1 = ON , 0 = OFF", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);

        while (1)
        {
                memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
                srDispObj.inMaxLen = 1;
                srDispObj.inY = _LINE_8_7_;
                srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inColor = _COLOR_RED_;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
                inRetVal = inDISP_Enter8x16(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                        return (VS_ERROR);

                if (strlen(srDispObj.szOutput) > 0)
                {
                        if (srDispObj.szOutput[0] == '0')
                        {
				/* 只設定第一個位置 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetDemoMode(szTemplate);
				szTemplate[0] = 'N';
				
				inSetDemoMode(szTemplate);
				inSaveEDCRec(0);
                                break;
                        }
                        else if (srDispObj.szOutput[0] == '1')
                        {
                                /* 只設定第一個位置 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetDemoMode(szTemplate);
				szTemplate[0] = 'Y';
				
				inSetDemoMode(szTemplate);
				inSaveEDCRec(0);
                                break;
                        }
                        else
                        {
                                inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
                                continue;
                        }
                }
                else
                {
                        break;
                }
        }	
	
	return (inRetVal);
}

/*
Function        :inMENU_QA_Fuction_Flow
Date&Time       :2022/1/3 下午 5:02
Describe        :QA工程師頁面
*/
int inMENU_QA_Fuction_Flow(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	int	inRetVal2 = VS_SUCCESS;
	int	inPage = 1;
	int	inMaxPage = 1;
	
	inDISP_ClearAll();
	inDISP_ChineseFont("測試工程師功能頁面", _FONTSIZE_8X16_, _LINE_8_1_, _DISP_LEFT_);
	
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);
	
	while (1)
	{
		
		inDISP_ClearAll();
		inDISP_ChineseFont("測試工程師功能頁面", _FONTSIZE_8X16_, _LINE_8_1_, _DISP_LEFT_);
	
		if (inPage == 1)
		{
			inRetVal2 = inMENU_QA_Fuction(srEventMenuItem);
		}
		
		if (inRetVal2 == VS_LAST_PAGE)
		{
			if (inPage == 1)
			{
				inPage = 1;
			}
			else
			{
				inPage--;
			}
		}
		else if (inRetVal2 == VS_NEXT_PAGE)
		{
			if (inPage == inMaxPage)
			{
				inPage = inMaxPage;
			}
			else
			{
				inPage++;
			}
		}
		else
		{
			break;
		}
		
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_QA_Fuction
Date&Time       :2022/1/3 下午 5:05
Describe        :測試工程師頁面
*/
int inMENU_QA_Fuction(EventMenuItem *srEventMenuItem)
{
	int		inRetVal = VS_SUCCESS;
//	int		inRetVal2 = VS_ERROR;
	unsigned char	uszKey = 0x00;
	
	while(1)
	{
		inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
		inDISP_ChineseFont("1.調整Host批號", _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
		inDISP_ChineseFont("2.尋找SAM卡TID", _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
		inDISP_ChineseFont("3.查看Mem Inv.", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
		inDISP_ChineseFont("4.查看PCI Reboot Time", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		inDISP_ChineseFont("5.------", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
		inDISP_ChineseFont("6.上一頁", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
		inDISP_ChineseFont("7.下一頁", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
		uszKey = uszKBD_GetKey(30);
		if (uszKey == _KEY_1_)
		{
			TRANSACTION_OBJECT		pobTran = {0};
			TABLE_GET_SET_TABLE		HDPT_FUNC_TABLE_QA[] =
			{
				{"szInvoiceNum"			,inGetInvoiceNum		,inSetInvoiceNum		},
				{"szBatchNum"			,inGetBatchNum			,inSetBatchNum			},
				{""},
			};
			pobTran.inRunOperationID = _OPERATION_EDIT_HOST_BATCH_NUM_;
			pobTran.srBRec.inHDTIndex = -1;
			inFunc_GetHostNum_NewUI(&pobTran);
			if (pobTran.srBRec.inHDTIndex != -1)
			{
				inLoadHDPTRec(pobTran.srBRec.inHDTIndex);
				inFunc_Edit_Table_Tag(HDPT_FUNC_TABLE_QA);
				inSaveHDPTRec(pobTran.srBRec.inHDTIndex);
			}
		}
		else if (uszKey == _KEY_2_)
		{
			inNCCC_Func_Find_SAM_TID();
		}
		else if (uszKey == _KEY_3_)
		{
			inNCCC_Func_Didp_Mem_Inv();
		}
		else if (uszKey == _KEY_4_)
		{
			inNCCC_Func_Didp_Reboot_Time();
		}
		else if (uszKey == _KEY_5_)
		{
			
		}
		else if (uszKey == _KEY_6_)
		{
			inRetVal = VS_LAST_PAGE;
			break;
		}
		else if (uszKey == _KEY_7_)
		{
			inRetVal = VS_NEXT_PAGE;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else
		{
			
		}
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_EditPWD_Flow_Cus_075
Date&Time       :2022/5/24 下午 5:42
Describe        :客製化075要加入昇恆昌管理
*/
int inMENU_EditPWD_Flow_Cus_075(void)
{
	int	inRetVal = VS_SUCCESS;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char	szKey = 0x00;
	char	szLine1[20 + 1] = {0};
	char	szLine1_2[20 + 1] = {0};
	char	szLine1_3[20 + 1] = {0};
	char	szLine2[20 + 1] = {0};
	char	szLine2_2[20 + 1] = {0};
	char	szLine2_3[20 + 1] = {0};
	char	szLine3[20 + 1] = {0};
	char	szLine3_2[20 + 1] = {0};
	char	szLine3_3[20 + 1] = {0};
        
        if (inLoadPWDRec(0) < 0)
        {
                /* 此功能已關閉 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_FUNC_CLOSE_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
                inDISP_Msg_BMP(&srDispMsgObj);
                return (VS_ERROR);
        }
        
        inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
        inDISP_PutGraphic(_MENU_SET_PWD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 設定管理號碼 */
        memset(szLine1, 0x00, sizeof(szLine1));
	memset(szLine1_2, 0x00, sizeof(szLine1_2));
	memset(szLine1_3, 0x00, sizeof(szLine1_3));
	memset(szLine2, 0x00, sizeof(szLine2));
	memset(szLine2_2, 0x00, sizeof(szLine2_2));
	memset(szLine2_3, 0x00, sizeof(szLine2_3));
	memset(szLine3, 0x00, sizeof(szLine3));
	memset(szLine3_2, 0x00, sizeof(szLine3_2));
	memset(szLine3_3, 0x00, sizeof(szLine3_3));
	sprintf(szLine1, "變更");
	sprintf(szLine1_2, "管理者");
	sprintf(szLine1_3, "號碼1");
	sprintf(szLine2, "交易");
	sprintf(szLine2_2, "功能");
	sprintf(szLine2_3, "管理2");
	sprintf(szLine3, "昇恆昌");
	sprintf(szLine3_2, "  ");
	sprintf(szLine3_3, "管理3");

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_CHOOSE_HOST_3_, 0, _COORDINATE_Y_LINE_8_4_);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine1, _FONTSIZE_24X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_24_13_, VS_FALSE);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine1_2, _FONTSIZE_24X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_24_14_, VS_FALSE);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine1_3, _FONTSIZE_24X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_24_15_, VS_FALSE);

	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine2, _FONTSIZE_24X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_24_13_, VS_FALSE);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine2_2, _FONTSIZE_24X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_24_14_, VS_FALSE);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine2_3, _FONTSIZE_24X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_24_15_, VS_FALSE);

	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine3, _FONTSIZE_24X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_3_, _COORDINATE_Y_LINE_24_13_, VS_FALSE);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine3_2, _FONTSIZE_24X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_3_, _COORDINATE_Y_LINE_24_14_, VS_FALSE);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine3_3, _FONTSIZE_24X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_3_, _COORDINATE_Y_LINE_24_15_, VS_FALSE);
        inDISP_BEEP(1, 0);
        
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
        while (1)
        {
                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		szKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}
                
		/* 變更管理號碼 */
                if (szKey == _KEY_1_			||
		    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
                {
			inRetVal = inFunc_Edit_Manager_Pwd();
			break;
                }
		/* 交易功能管理 */
                else if (szKey == _KEY_2_		||
			 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
                {
			inRetVal = inFunc_Edit_TransFuc_Pwd();
			break;
                }
		/* 昇恆昌管理 */
		else if (szKey == _KEY_3_		||
			 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_)
                {
			inRetVal = inMENU_Duty_Free_075_PWD();
			break;
                }
                else if (szKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
        }
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
        
        return (VS_SUCCESS);
}

/*
Function        :inMENU_Duty_Free_075_PWD
Date&Time       :2022/5/25 上午 9:39
Describe        :昇恆昌管理
*/
int inMENU_Duty_Free_075_PWD(void)
{
	int	inRetVal = VS_SUCCESS;
	char	szKey = 0x00;
	char	szLine1[42 + 1] = {0};
	char	szLine2[42 + 1] = {0};
	
        inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
        inDISP_PutGraphic(_MENU_SET_PWD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 設定管理號碼 */
        memset(szLine1, 0x00, sizeof(szLine1));
	memset(szLine2, 0x00, sizeof(szLine2));
	sprintf(szLine1, "1.變更重印管理碼");
	sprintf(szLine2, "2.變更明細管理碼");

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont_Point_Color(szLine1, _FONTSIZE_8X16_, _LINE_8_4_, _COLOR_BLACK_, _COLOR_WHITE_, 1);
	inDISP_ChineseFont_Point_Color(szLine2, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_BLACK_, _COLOR_WHITE_, 1);

        inDISP_BEEP(1, 0);
        
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
        while (1)
        {
		szKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}
                
		/* 變更重印管理碼 */
                if (szKey == _KEY_1_)
                {
			inRetVal = inFunc_Edit_Duty_Free_Reprint_Pwd();
			break;
                }
		/* 變更明細管理碼 */
                else if (szKey == _KEY_2_)
                {
			inRetVal = inFunc_Edit_Duty_Free_Detail_Pwd();
			break;
                }
                else if (szKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
        }
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
        
        return (VS_SUCCESS);
}

/*
Function        :inMENU_CHECK_2_FUNCTION_ENABLE_DISPLAY
Date&Time       :2022/10/7 下午 3:01
Describe        :直接以LCD控制顯示
*/
int inMENU_CHECK_2_FUNCTION_ENABLE_DISPLAY(MENU_CHECK_2_TABLE_ *srMENU_CHECK_TABLE)
{
	int		i = 0;
	int		inForeColor = 0;
	int		inBackColor = 0;
	char		szIconPath[200 + 1] = {0};
	unsigned char	uszDispBit = VS_FALSE;
	
	while(1)
	{
		uszDispBit = VS_FALSE;
		if (srMENU_CHECK_TABLE[i].inButtonPositionID == _Touch_NONE_)
		{
			break;
		}
		
		if (srMENU_CHECK_TABLE[i].inForceDisplay == _MENU_CHECK_FORCE_DISPLAY_ON_)
		{
			uszDispBit = VS_TRUE;
		}
		else if (srMENU_CHECK_TABLE[i].inForceDisplay == _MENU_CHECK_FORCE_DISPLAY_OFF_)
		{
			uszDispBit = VS_FALSE;
		}
		else
		{
			if (srMENU_CHECK_TABLE[i].inCheckFunc == NULL)
			{
				/* 不檢核 */
				uszDispBit = VS_FALSE;
			}
			else
			{
				if (srMENU_CHECK_TABLE[i].inCheckFunc(srMENU_CHECK_TABLE[i].inCode) == VS_SUCCESS)
				{
					uszDispBit = VS_TRUE;
				}
				else
				{
					uszDispBit = VS_FALSE;
				}
			}
		}
		
		if (uszDispBit == VS_TRUE)
		{
			memset(szIconPath, 0x00, sizeof(szIconPath));
			strcpy(szIconPath, _ICON_TEMPLATE_BLUE_);
			inForeColor = _COLOR_WHITE_;
			inBackColor = _COLOR_BUTTON_;
		}
		else
		{
			memset(szIconPath, 0x00, sizeof(szIconPath));
			strcpy(szIconPath, _ICON_TEMPLATE_GRAY_);
			inForeColor = _COLOR_WHITE_;
			inBackColor = _COLOR_BUTTON_GREY_;
		}
		inMENU_Display_ICON(szIconPath, srMENU_CHECK_TABLE[i].inButtonPositionID);
		
		if (srMENU_CHECK_TABLE[i].szLine1Msg != NULL)
		{
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode(srMENU_CHECK_TABLE[i].szLine1Msg, srMENU_CHECK_TABLE[i].ulLine1FontSize, inForeColor, inBackColor, srMENU_CHECK_TABLE[i].inLine1X, srMENU_CHECK_TABLE[i].inLine1Y, srMENU_CHECK_TABLE[i].uszLine1ReverseBit);
		}
		if (srMENU_CHECK_TABLE[i].szLine2Msg != NULL)
		{
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode(srMENU_CHECK_TABLE[i].szLine2Msg, srMENU_CHECK_TABLE[i].ulLine2FontSize, inForeColor, inBackColor, srMENU_CHECK_TABLE[i].inLine2X, srMENU_CHECK_TABLE[i].inLine2Y, srMENU_CHECK_TABLE[i].uszLine2ReverseBit);
		}
		if (srMENU_CHECK_TABLE[i].szLine3Msg != NULL)
		{
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode(srMENU_CHECK_TABLE[i].szLine3Msg, srMENU_CHECK_TABLE[i].ulLine3FontSize, inForeColor, inBackColor, srMENU_CHECK_TABLE[i].inLine3X, srMENU_CHECK_TABLE[i].inLine3Y, srMENU_CHECK_TABLE[i].uszLine3ReverseBit);
		}
		
		i++;
	}
	
	return (VS_SUCCESS);
}

/*
Function	:inMENU_MenuKeyInAndGetAmount_041
Date&Time	:2022/10/11 上午 11:21
Describe	:idle輸入 客製化041
*/
int inMENU_MenuKeyInAndGetAmount_041(EventMenuItem *srEventMenuItem)
{
        char	szCTLSEnable[2 + 1];
        
        memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
	inGetContactlessEnable(szCTLSEnable);
        
        if (!memcmp(&szCTLSEnable[0], "Y", 1) && srEventMenuItem->inEventCode == '0')
        {
                /* 不接受金額第一位為0 */
                return (VS_ERROR);
        }

	/* 第三層顯示 ＜預借現金＞ */
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
        
        inDISP_Display_Black_Back(0, _COORDINATE_Y_LINE_8_3_, _LCD_XSIZE_, _LCD_YSIZE_/ 8);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("                ", _FONTSIZE_8X16_, _COLOR_BLACK_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("預借現金", _FONTSIZE_8X16_, _COLOR_WHITE_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);
        /* 輸入密碼的層級 */
        srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
        srEventMenuItem->inCode = _CASH_ADVANCE_;

	/* 這邊先設定TRT是因為在抓卡號時inFunc_ResetTitle這隻函數會用到 */
        if (!memcmp(&szCTLSEnable[0], "Y", 1) && guszCTLSInitiOK == VS_TRUE)
        {
		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
		
		/* Contactless功能開啟時，信用卡正向(銷售)交易管理號碼在輸入金額後 */
                srEventMenuItem->inRunOperationID = _OPERATION_SALE_CTLS_IDLE_;
                srEventMenuItem->inRunTRTID = _TRT_CASH_ADVANCE_CTLS_;
        }
        else
        {
		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);
		srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	
                srEventMenuItem->inRunOperationID = _OPERATION_SALE_ICC_;
                srEventMenuItem->inRunTRTID = _TRT_CASH_ADVANCE_;
        }

        return (VS_SUCCESS);
}

/*
Function        :inMENU_CASH_ADVANCE
Date&Time       :2022/10/11 下午 1:28
Describe        :
*/
int inMENU_CASH_ADVANCE(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	if (inNCCC_Func_Check_Transaction_Function(_SALE_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		/* [113110-信託需求][UI] 修改抬頭黑底長度不夠問題,加入 inDISP_Display_Black_Back()  2025/11/20 */
		inDISP_Display_Black_Back(0, _COORDINATE_Y_LINE_8_3_, _LCD_XSIZE_, _LCD_YSIZE_/ 8);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("                ", _FONTSIZE_8X16_, _COLOR_BLACK_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("預借現金", _FONTSIZE_8X16_, _COLOR_WHITE_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _CASH_ADVANCE_;

		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}

		srEventMenuItem->inRunOperationID = _OPERATION_SALE_CTLS_;
		srEventMenuItem->inRunTRTID = _TRT_SALE_CTLS_;
	}
			
	return (inRetVal);
}

/*
Function        inMENU_FORCE_CASH_ADVANCE
Date&Time       :2022/10/11 下午 1:20
Describe        :
*/
int inMENU_FORCE_CASH_ADVANCE(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	if (inNCCC_Func_Check_Transaction_Function(_SALE_OFFLINE_) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜交易補登＞ */
		/* 輸入密碼的層級 */
		srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
		srEventMenuItem->inCode = _FORCE_CASH_ADVANCE_;
		
		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
			return (VS_ERROR);

		srEventMenuItem->inRunOperationID = _OPERATION_SALE_OFFLINE_;
		srEventMenuItem->inRunTRTID = _TRT_FORCE_CASH_ADVANCE_;
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_TRACELOG_MENU
Date&Time       :2024/10/22 下午 2:27
Describe        :用來選印一般log還是新版log
*/
int inMENU_TRACELOG_MENU(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
        char    szKey = 0;
	
	inDISP_ClearAll();
        inDISP_ChineseFont("列印 Trace log", _FONTSIZE_8X33_, _LINE_8_1_, _DISP_CENTER_);
        inDISP_ChineseFont("1.NCCC Trace Log", _FONTSIZE_8X33_, _LINE_8_2_, _DISP_LEFT_);
        inDISP_ChineseFont("2.NEXSYS Trace Log(未完成)", _FONTSIZE_8X33_, _LINE_8_3_, _DISP_LEFT_);
	srEventMenuItem->inCode = FALSE;

        inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
        do
        {
		szKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}
                
		/* NCCC Trace Log */
                if (szKey == _KEY_1_)
                {
			inRetVal = inMENU_TRACELOG_PRINT(srEventMenuItem);
			break;
                }
		/* NEXSYS Trace Log */
                else if (szKey == _KEY_2_)
                {
			inRetVal = VS_ERROR;
			break;
                }
                else if (szKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
        }while(1);

	
	return (inRetVal);
}

/*
Function        :inMENU_CHECK_FILE_Flow
Date&Time       :2024/11/5 下午 5:33
Describe        :確認檔案
*/
int inMENU_CHECK_FILE_Flow(EventMenuItem *srEventMenuItem)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszKey = 0x00;
	unsigned char	uszDispBit = VS_FALSE;
	
	inDISP_ClearAll();
	/* 第三層顯示 ＜確認檔案＞ */
	inDISP_ChineseFont("確認檔案", _FONTSIZE_8X16_, _LINE_8_1_, _DISP_LEFT_);
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_WITH_FUNC_PASSWORD_;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	while(1)
	{
		if (uszDispBit == VS_FALSE)
		{
			uszDispBit = VS_TRUE;
			inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
			inDISP_ChineseFont("1.Check AP File", _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
			inDISP_ChineseFont("2.Check Nexsys Log", _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
		}
		
		uszKey = uszKBD_GetKey(30);
		if (uszKey == _KEY_1_)
		{
			inMENU_CHECK_FILE(srEventMenuItem);
			uszDispBit = VS_FALSE;
		}
		else if (uszKey == _KEY_2_)
		{
			inMENU_CHECK_Nexsys_Log(srEventMenuItem);
			uszDispBit = VS_FALSE;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else
		{
			
		}
	}
	
	return (inRetVal);
}

/*
Function        :inMENU_CHECK_Nexsys_Log
Date&Time       :2024/11/5 下午 5:33
Describe        :確認檔案
*/
int inMENU_CHECK_Nexsys_Log(EventMenuItem *srEventMenuItem)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszKey = 0x00;
	unsigned char	uszDispBit = VS_FALSE;
	
	inDISP_ClearAll();
	/* 第三層顯示 ＜確認檔案＞ */
	inDISP_ChineseFont("Nexsys Log", _FONTSIZE_8X16_, _LINE_8_1_, _DISP_LEFT_);
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	while(1)
	{
		if (uszDispBit == VS_FALSE)
		{
			uszDispBit = VS_TRUE;
			inDISP_Clear_Line(_LINE_8_2_, _LINE_8_8_);
			inDISP_ChineseFont("1.To SD", _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
			inDISP_ChineseFont("2.To USB", _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
		}
		
		uszKey = uszKBD_GetKey(30);
		if (uszKey == _KEY_1_)
		{
			inFunc_Check_NexsysLog_To_SD();
			uszDispBit = VS_FALSE;
		}
		else if (uszKey == _KEY_2_)
		{
			inFunc_Check_NexsysLog_To_USB();
			uszDispBit = VS_FALSE;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else
		{
			
		}
	}
	
	return (inRetVal);
}

/*
 Function        :vdMENU_ReNew_Idle_Time
 Date&Time       :2025/4/15 下午 4:20
 Describe        :
 */
void vdMENU_ReNew_Idle_Time(void)
{
	gulPCI_IdleTime = time(NULL);
}

/*
Function	:inMENU_FUNCTION_NEWUI_PAGE_Flow
Date&Time	:2025/6/17 下午 3:57
Describe	:
*/
int inMENU_FUNCTION_NEWUI_PAGE_Flow(EventMenuItem *srEventMenuItem)
{
	int inRetVal = VS_SUCCESS;
	int inPage = 0;
	int inMINPage = 0;
	int inMAXPage = 1;
	int inTrustFunctionalEnable = VS_ERROR;

	/* [113110-信託需求][UI] 畫面需依照信託功能是否開啟變動 2025/11/20  */
	inTrustFunctionalEnable = inMENU_Check_Trust_Enable(0);
        
	do
	{
		/* [113110-信託需求][UI] 畫面需依照信託功能是否開啟變動 2025/11/20  */
		if(VS_SUCCESS == inTrustFunctionalEnable)
		{
			if (inPage == 0)
			{
				inRetVal = inMENU_FUNCTION_TRUST_UI_PAGE(srEventMenuItem);
			}
			else if (inPage == 1)
			{
				inRetVal = inMENU_FUNCTION_NEWUI_PAGE2(srEventMenuItem);
			}
		}else
		{
			inRetVal = inMENU_FUNCTION_NEWUI_PAGE1(srEventMenuItem);
		}               

		if (inRetVal == VS_NEXT_PAGE)
		{
			inPage++;

			if (inPage >= inMAXPage)
			{
				inPage = inMAXPage;
			}
		}
		else if (inRetVal == VS_LAST_PAGE)
		{
			inPage--;

			if (inPage <= inMINPage)
			{
				inPage = inMINPage;
			}
		}

		if (inRetVal == VS_NEXT_PAGE ||
			inRetVal == VS_LAST_PAGE)
		{

		}
		else
		{
			break;
		}

	}while(1);

	return (inRetVal);
}

/*
Function	:inMENU_NEWUI_NEWUI_TRUST_MENU
Date&Time	:2025/6/18 下午 3:21
Describe	:
*/
int inMENU_NEWUI_NEWUI_TRUST_MENU(EventMenuItem *srEventMenuItem)
{
        int			inPageLoop = _PAGE_LOOP_1_;
        int			inRetVal = VS_SUCCESS;
        int			inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
        int			inCount= 1;
        int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char			szKey = 0x00;
	char			szCustomerIndicator[3 + 1] = {0};
	MENU_CHECK_2_TABLE_	srMenuChekDisplay[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_	, _TRANS_TYPE_NULL_	, _MENU_CHECK_FORCE_DISPLAY_NONE_	, inMENU_Check_Transaction_Enable, 
		"信託",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_8_, VS_FALSE, 
		"交易  1",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_9_, VS_FALSE},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_	, _TRANS_TYPE_NULL_	, _MENU_CHECK_FORCE_DISPLAY_NONE_	, inMENU_Check_Transaction_Enable, 
		"信託",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_2_, _COORDINATE_Y_LINE_24_8_, VS_FALSE, 
		"交易  2",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_2_, _COORDINATE_Y_LINE_24_9_, VS_FALSE,
		"取消",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_2_, _COORDINATE_Y_LINE_24_10_, VS_FALSE},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_	, _TRANS_TYPE_NULL_	, _MENU_CHECK_FORCE_DISPLAY_NONE_	, inMENU_Check_Transaction_Enable, 
		"重印",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_3_, _COORDINATE_Y_LINE_24_8_, VS_FALSE, 
		"簽單  3",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_3_, _COORDINATE_Y_LINE_24_9_, VS_FALSE},
		{_Touch_NONE_}
	};
	
        if (inMENU_Check_Trust_Enable(0) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
                memset(szCustomerIndicator, 0x00 , sizeof(szCustomerIndicator));
                inGetCustomIndicator(szCustomerIndicator);

                /* 初始化 */
                inDISP_ClearAll();
                inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
                inDISP_Display_Black_Back(0, _COORDINATE_Y_LINE_8_2_, _LCD_XSIZE_, _LCD_YSIZE_/ 8);
                inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("信託專區", _FONTSIZE_8X16_, _COLOR_WHITE_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_2_, VS_FALSE, _DISP_CENTER_);

                if (ginHalfLCD == VS_FALSE)
                {      
                        inDISP_PutGraphic(_ICON_BUTTON_TEMP_BLUE_, _COORDINATE_X_16_3_,  _COORDINATE_Y_LINE_8_8_);
                        inDISP_ChineseFont_Point_Color_By_Graphic_Mode("上一頁", _FONTSIZE_16X44_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_16_3_ + 20, _COORDINATE_Y_LINE_24_23_, VS_FALSE);
                        inDISP_PutGraphic(_ICON_BUTTON_TEMP_BLUE_, _COORDINATE_X_16_10_,  _COORDINATE_Y_LINE_8_8_);
                        inDISP_ChineseFont_Point_Color_By_Graphic_Mode("回主畫面", _FONTSIZE_16X44_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_16_10_ + 20, _COORDINATE_Y_LINE_24_23_, VS_FALSE);
                }

                /* 檢查功能開關，並顯示反白的圖 */
                inMENU_CHECK_2_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);

                /* 設定Timeout */
                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
                {
                        inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
                }
                else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
                {
                        inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
                }
                else
                {
                        inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
                }

                while (inPageLoop != _PAGE_LOOP_0_)
                {
                        while (inPageLoop == _PAGE_LOOP_1_)
                        {
                                szKey = uszKBD_Key();
                                inChioce1 = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc); /* 回傳MENU畫面的按鈕選擇 */
                                if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
                                {
                                        szKey = _KEY_TIMEOUT_;
                                }

                                switch (inChioce1)
                                {
                                        case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_:
                                                inCount = 1;
                                                inPageLoop = _PAGE_LOOP_2_;
                                                break;
                                        case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_:
                                                inCount = 2;
                                                inPageLoop = _PAGE_LOOP_2_;
                                                break;
                                        case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_:
                                                inCount = 3;
                                                inPageLoop = _PAGE_LOOP_2_;
                                                break;
                                        case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_:
                                                inCount = 4;
                                                inPageLoop = _PAGE_LOOP_2_;
                                                break;
                                        case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_:
                                                inCount = 5;
                                                inPageLoop = _PAGE_LOOP_2_;
                                                break;
                                        case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_:
                                                inCount = 6;
                                                inPageLoop = _PAGE_LOOP_2_;
                                                break;
                                        case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_7_:
                                                inCount = 7;
                                                inPageLoop = _PAGE_LOOP_2_;
                                                break;
                                        case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_8_:
                                                inCount = 8;
                                                inPageLoop = _PAGE_LOOP_2_;
                                                break;
                                        case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_9_:
                                                inCount = 9;
                                                inPageLoop = _PAGE_LOOP_2_;
                                                break;
                                        case _NEWUI_LAST_PAGE_BUTTON_:
                                                inRetVal = VS_LAST_PAGE;
                                                inPageLoop = _PAGE_LOOP_0_;
                                                break;
                                        case _NEWUI_RETURN_IDLE_BUTTON_:
                                                inRetVal = VS_USER_CANCEL;
                                                inPageLoop = _PAGE_LOOP_0_;
                                                break;
                                        case _DisTouch_Slide_Down_To_Up_:
                                                inRetVal = VS_USER_CANCEL;
                                                inPageLoop = _PAGE_LOOP_0_;
                                                break;
                                        default:
                                                break;
                                }

                                if (szKey == _KEY_1_)
                                {
                                        inCount = 1;
                                        inPageLoop = _PAGE_LOOP_2_;
                                        break;
                                }
                                else if (szKey == _KEY_2_)
                                {
                                        inCount = 2;
                                        inPageLoop = _PAGE_LOOP_2_;
                                        break;
                                }
                                else if (szKey == _KEY_3_)
                                {
                                        inCount = 3;
                                        inPageLoop = _PAGE_LOOP_2_;
                                        break;
                                }
                                else if (szKey == _KEY_4_)
                                {
                                        inCount = 4;
                                        inPageLoop = _PAGE_LOOP_2_;
                                        break;
                                }
                                else if (szKey == _KEY_5_)
                                {
                                        inCount = 5;
                                        inPageLoop = _PAGE_LOOP_2_;
                                        break;
                                }
                                else if (szKey == _KEY_6_)
                                {
                                        inCount = 6;
                                        inPageLoop = _PAGE_LOOP_2_;
                                        break;
                                }
                                else if (szKey == _KEY_7_)
                                {
                                        inCount = 7;
                                        inPageLoop = _PAGE_LOOP_2_;
                                        break;
                                }
                                else if (szKey == _KEY_8_)
                                {
                                        inCount = 8;
                                        inPageLoop = _PAGE_LOOP_2_;
                                        break;
                                }
                                else if (szKey == _KEY_9_)
                                {
                                        inCount = 9;
                                        inPageLoop = _PAGE_LOOP_2_;
                                        break;
                                }
                                else if (szKey == _KEY_UP_)
                                {
                                        inRetVal = VS_LAST_PAGE;
                                        inPageLoop = _PAGE_LOOP_0_;
                                        break;
                                }
                                else if (szKey == _KEY_CANCEL_)
                                {
                                        inRetVal = VS_USER_CANCEL;
                                        inPageLoop = _PAGE_LOOP_0_;
                                        break;
                                }
                                else if (szKey == _KEY_TIMEOUT_)
                                {
                                        inRetVal = VS_TIMEOUT;
                                        inPageLoop = _PAGE_LOOP_0_;
                                        break;
                                }
                        }

                        /* 判斷MENU點進去按鈕之後要做的事情 */
                        if (inPageLoop == _PAGE_LOOP_2_)
                        {
                                switch (inCount)
                                {
                                        case 1:
                                                inRetVal = inMENU_Trust_Sale(srEventMenuItem);

                                                if (inRetVal == VS_USER_CANCEL)
                                                {
                                                        inRetVal = VS_ERROR;
                                                        inPageLoop = _PAGE_LOOP_1_;
                                                }
                                                else if (inRetVal == VS_FUNC_CLOSE_ERR)
                                                {
                                                        inRetVal = VS_SUCCESS;
                                                        inPageLoop = _PAGE_LOOP_1_;
                                                }
                                                else
                                                {
                                                        inPageLoop = _PAGE_LOOP_0_;
                                                }
                                                break;
                                        case 2:
                                                inRetVal = inMENU_Trust_Void(srEventMenuItem);

                                                if (inRetVal == VS_USER_CANCEL)
                                                {
                                                        inRetVal = VS_ERROR;
                                                        inPageLoop = _PAGE_LOOP_1_;
                                                }
                                                else if (inRetVal == VS_FUNC_CLOSE_ERR)
                                                {
                                                        inRetVal = VS_SUCCESS;
                                                        inPageLoop = _PAGE_LOOP_1_;
                                                }
                                                else
                                                {
                                                        inPageLoop = _PAGE_LOOP_0_;
                                                }
                                                break;
                                        case 3:
                                                inRetVal = inMENU_Trust_Reprint(srEventMenuItem);

                                                if (inRetVal == VS_USER_CANCEL)
                                                {
                                                        inRetVal = VS_ERROR;
                                                        inPageLoop = _PAGE_LOOP_1_;
                                                }
                                                else if (inRetVal == VS_FUNC_CLOSE_ERR)
                                                {
                                                        inRetVal = VS_SUCCESS;
                                                        inPageLoop = _PAGE_LOOP_1_;
                                                }
                                                else
                                                {
                                                        inPageLoop = _PAGE_LOOP_0_;
                                                }
                                                break;
                                        default:
                                                inPageLoop = _PAGE_LOOP_0_;
                                                break;
                                }
                        }

                        /* 代表回上一頁，要回復UI */
                        if (inPageLoop == _PAGE_LOOP_1_		&&
                            inRetVal == VS_ERROR)
                        {
                                /* 初始化 */
                                inDISP_ClearAll();
                                inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第1.5層顯示 LOGO */

                                if (ginHalfLCD == VS_FALSE)
                                {
                                        inDISP_PutGraphic(_ICON_BUTTON_TEMP_BLUE_, _COORDINATE_X_16_3_,  _COORDINATE_Y_LINE_8_8_);
                                        inDISP_ChineseFont_Point_Color_By_Graphic_Mode("上一頁", _FONTSIZE_16X44_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_16_3_ + 20, _COORDINATE_Y_LINE_24_23_, VS_FALSE);
                                        inDISP_PutGraphic(_ICON_BUTTON_TEMP_BLUE_, _COORDINATE_X_16_10_,  _COORDINATE_Y_LINE_8_8_);
                                        inDISP_ChineseFont_Point_Color_By_Graphic_Mode("回主畫面", _FONTSIZE_16X44_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_16_10_ + 20, _COORDINATE_Y_LINE_24_23_, VS_FALSE);
                                }

                                inMENU_CHECK_2_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
                                /* 設定Timeout */
                                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
                                {
                                        inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
                                }
                                else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
                                {
                                        inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
                                }
                                else
                                {
                                        inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
                                }
                        }
                        /* 功能未開而返回，不刷畫面 */
                        else if (inPageLoop == _PAGE_LOOP_1_	&&
                                 inRetVal == VS_SUCCESS)
                        {

                        }
                }/* _PAGE_LOOP_0_ */
        }

	return (inRetVal);
}

/*
Function	:inMENU_Trust_Sale
Date&Time	:2025/6/18 下午 5:08
Describe	:信託交易
*/
int inMENU_Trust_Sale(EventMenuItem *srEventMenuItem)
{
        inDISP_ClearAll();
        /* 第三層顯示 ＜信託交易＞ */
        inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
	inDISP_Display_Black_Back(0, _COORDINATE_Y_LINE_8_2_, _LCD_XSIZE_, _LCD_YSIZE_/ 8);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("信託交易", _FONTSIZE_8X16_, _COLOR_WHITE_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_2_, VS_FALSE, _DISP_CENTER_);
        /* 輸入密碼的層級 */
        srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
        srEventMenuItem->inCode = _TRUST_SALE_;

	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;

	/* Contactless功能開啟時，信用卡正向(銷售)交易管理號碼在輸入金額後 */
	srEventMenuItem->inRunOperationID = _OPERATION_TRUST_SALE_;
	srEventMenuItem->inRunTRTID = _TRT_TRUST_SALE_;
	
	srEventMenuItem->uszTrustTransBit = VS_TRUE;
	
        return (VS_SUCCESS);
}

/*
Function	:inMENU_Trust_Void
Date&Time	:2025/10/8 下午 2:03
Describe	:信託交易
*/
int inMENU_Trust_Void(EventMenuItem *srEventMenuItem)
{
        inDISP_ClearAll();
        /* 第三層顯示 ＜信託交易取消＞ */
        inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
	inDISP_Display_Black_Back(0, _COORDINATE_Y_LINE_8_2_, _LCD_XSIZE_, _LCD_YSIZE_/ 8);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("信託交易取消", _FONTSIZE_8X16_, _COLOR_WHITE_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_2_, VS_FALSE, _DISP_CENTER_);
        /* 輸入密碼的層級 */
        srEventMenuItem->inPasswordLevel = _ACCESS_WITH_CUSTOM_;
        srEventMenuItem->inCode = _TRUST_VOID_;

	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;

	/* Contactless功能開啟時，信用卡正向(銷售)交易管理號碼在輸入金額後 */
	srEventMenuItem->inRunOperationID = _OPERATION_TRUST_VOID_;
	srEventMenuItem->inRunTRTID = _TRT_TRUST_VOID_;
	
	srEventMenuItem->uszTrustTransBit = VS_TRUE;
	
        return (VS_SUCCESS);
}

/*
Function        :inMENU_Trust_Reprint
Date&Time       :2025/10/13 下午 4:38
Describe        :
*/
int inMENU_Trust_Reprint(EventMenuItem *srEventMenuItem)
{
	int	inRetVal = VS_SUCCESS;
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_REPRINT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜重印簽單＞ */
	/* 輸入密碼的層級 */
	srEventMenuItem->inPasswordLevel = _ACCESS_FREELY_;
	srEventMenuItem->inCode = FALSE;

	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(srEventMenuItem->inPasswordLevel, srEventMenuItem->inCode) != VS_SUCCESS)
		return (VS_ERROR);

	srEventMenuItem->inCode = FALSE;
	srEventMenuItem->inRunOperationID = _OPERATION_REPRINT_;
	srEventMenuItem->inRunTRTID = FALSE;
        
        srEventMenuItem->uszTrustTransBit = VS_TRUE;
	
	return (inRetVal);
}

/*
Function        :inMENU_Check_Trust_Enable
Date&Time       :2025/10/20 下午 3:35
Describe        :
*/
int inMENU_Check_Trust_Enable(int inCode)
{
	int		inHDTIndex = -1;
	int		inOrgHDTIndex = 0;
	char		szHostEnable[2 + 1];
	char		szTMSOK[2 + 1];
	unsigned char	uszTxnEnable = VS_TRUE;
	
	if (inFunc_Find_Specific_HDTindex(inOrgHDTIndex, _HOST_NAME_TRUST_, &inHDTIndex) != VS_SUCCESS)
	{
		memset(szHostEnable, 0x00, sizeof(szHostEnable));
		sprintf(szHostEnable, "N");
	}
	else
	{
		inLoadHDTRec(inHDTIndex);
		memset(szHostEnable, 0x00, sizeof(szHostEnable));
		inGetHostEnable(szHostEnable);
		inLoadHDTRec(inOrgHDTIndex);
	}
	
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	
	if ((memcmp(szHostEnable, "Y", strlen("Y")) != 0)	||
	    (memcmp(szTMSOK, "Y", 1) != 0))
	{
		uszTxnEnable = VS_FALSE;
	}
	else
	{
		uszTxnEnable = VS_TRUE;
	}
	
	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}