#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctosapi.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <sqlite3.h>
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
#include "../FUNCTION/TDT.h"
#include "../FUNCTION/IPASSDT.h"
#include "../FUNCTION/ECCDT.h"
#include "../FUNCTION/ICASHDT.h"
#include "../FUNCTION/PWD.h"
#include "../COMM/Comm.h"
#include "../COMM/Ethernet.h"
#include "../COMM/WiFi.h"
#include <curl/curl.h>
#include "../COMM/Ftps.h"
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
#include "Event.h"
#include "MenuMsg.h"
#include "Menu.h"
#include "Flow.h"
#include "CustomerMenu.h"

unsigned char		guszCus123UnlockBit = VS_FALSE;
extern  int             ginEventCode;   /* 用於儲存IdleMenuKeyIn第一個鍵 */
extern	int		ginEngineerDebug;
extern	int		ginHalfLCD;
extern	int		ginDebug;

int inCUST_MENU_Test(void)
{
        inLogPrintf(AT, "inCUST_MENU_Test()");
        return (VS_SUCCESS);
}

/*
Function	:inMENU_098_MenuFlow_NEWUI
Date&Time	:2017/10/16 下午 4:43
Describe	:選擇功能Menu
*/
int inMENU_098_MenuFlow_NEWUI(EventMenuItem *srEventMenuItem)
{
	int		inRetVal = VS_SUCCESS;
	char		szKey = 0x00;
	unsigned char	uszRejudgeBit = VS_FALSE;
        
	do
	{
		uszRejudgeBit = VS_FALSE;
		
                switch (srEventMenuItem->inEventCode)
                {
                        case _NEWUI_IDLE_Touch_KEY_FUNCTIONKEY_ :
                                inRetVal = inMENU_098_FUNCTION_NEWUI(srEventMenuItem);
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
                                inRetVal = inMENU_098_FUNCTION_NEWUI(srEventMenuItem);
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
		
	}while (uszRejudgeBit == VS_TRUE);
	
        return (inRetVal);
}

/*
Function	:inMENU_098_FUNCTION_NEWUI
Date&Time	:2017/11/3 下午 3:42
Describe	:
*/
int inMENU_098_FUNCTION_NEWUI(EventMenuItem *srEventMenuItem)
{
        int			inPageLoop = _PAGE_LOOP_1_;
        int			inRetVal = VS_SUCCESS;
        int			inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
        int			inCount= 1;
        int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char			szKey = 0x00;
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
                {_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_	, _TRANS_TYPE_NULL_	, inMENU_Check_Transaction_Enable	, _ICON_HIGHTLIGHT_1_TRANSACTION_	},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_	, _TRANS_TYPE_NULL_	, inMENU_Check_Mark_Enable		, _ICON_HIGHTLIGHT_2_CUP_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_	, _TRANS_TYPE_NULL_	, inMENU_Check_Mark_Enable		, _ICON_HIGHTLIGHT_3_SMARTPAY_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_	, _TRANS_TYPE_NULL_	, inMENU_Check_Mark_Enable		, _ICON_HIGHTLIGHT_4_ETICKET_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_	, _TRANS_TYPE_NULL_	, inMENU_Check_Mark_Enable		, _ICON_HIGHTLIGHT_5_AWARD_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_	, _TRANS_TYPE_NULL_	, inMENU_Check_Mark_Enable		, _ICON_HIGHTLIGHT_6_HG_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_7_	, _TRANS_TYPE_NULL_	, inMENU_Check_SETTLE_Enable		, _ICON_HIGHTLIGHT_7_SETTLE_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_8_	, _TRANS_TYPE_NULL_	, inMENU_Check_REVIEW_SETTLE_Enable	, _ICON_HIGHTLIGHT_8_REVIEW_PRINT_	},
		{_Touch_NONE_					, _TRANS_TYPE_NULL_	, NULL					, ""					}
	};
	
        /* 初始化 */
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第1.5層顯示 LOGO */
        inDISP_PutGraphic(_TOUCH_NEWUI_FUNCTION_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);

	if (ginHalfLCD == VS_FALSE)
		inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

	/* 檢查功能開關，並顯示反白的圖 */
	inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);

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
					inRetVal = inMENU_098_NEWUI_TRANSACTION_MENU(srEventMenuItem);
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
                                case 3:
				case 4:
				case 5:
                                case 6:
                                        inRetVal = VS_SUCCESS;
                                        inPageLoop = _PAGE_LOOP_1_;
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
					inRetVal = inMENU_098_NEWUI_REVIEW_PRINT_MENU(srEventMenuItem);
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
					inRetVal = inMENU_NEWUI_SETTING_MENU(srEventMenuItem);
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
				inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

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
Function        :inMENU_098_NEWUI_TRANSACTION_MENU
Date&Time       :2017/10/25 下午 1:55
Describe        :
*/
int inMENU_098_NEWUI_TRANSACTION_MENU(EventMenuItem *srEventMenuItem)
{
	int			inPageLoop = _PAGE_LOOP_1_;
        int			inRetVal = VS_SUCCESS;
        int			inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
        int			inCount = 1;
        int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char			szKey = 0x00;
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_	, _VOID_		, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_1_1_VOID_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_	, _REFUND_		, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_1_2_REFUND_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_	, _TRANS_TYPE_NULL_	, inMENU_Check_Mark_Enable                      , _ICON_HIGHTLIGHT_1_3_INST_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_	, _TRANS_TYPE_NULL_	, inMENU_Check_Mark_Enable                      , _ICON_HIGHTLIGHT_1_4_REDEEM_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_	, _TRANS_TYPE_NULL_	, inMENU_Check_Mark_Enable                      , _ICON_HIGHTLIGHT_1_5_SALEOFFLINE_	},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_	, _TRANS_TYPE_NULL_	, inMENU_Check_Mark_Enable                      , _ICON_HIGHTLIGHT_1_6_TIP_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_7_	, _TRANS_TYPE_NULL_	, inMENU_Check_Mark_Enable                      , _ICON_HIGHTLIGHT_1_7_PREAUTH_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_8_	, _TRANS_TYPE_NULL_	, inMENU_Check_Mark_Enable                      , _ICON_HIGHTLIGHT_1_8_ADJUST_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_9_	, _TRANS_TYPE_NULL_	, inMENU_Check_Mark_Enable                      , _ICON_HIGHTLIGHT_1_9_MAILORDER_	},
		{_Touch_NONE_				, _TRANS_TYPE_NULL_	, NULL						, ""					}
	};
	
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
						inRetVal = inMENU_098_REFUND(srEventMenuItem);
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
					case 4:
					case 5:
					case 6:
					case 7:
					case 8:
					case 9:
						inRetVal = VS_SUCCESS;
						inPageLoop = _PAGE_LOOP_1_;
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
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
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
Function        :inMENU_098_NEWUI_REVIEW_PRINT_MENU
Date&Time       :2017/10/25 下午 1:55
Describe        :
*/
int inMENU_098_NEWUI_REVIEW_PRINT_MENU(EventMenuItem *srEventMenuItem)
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
		inDISP_PutGraphic(_TOUCH_NEWUI_REVIEW_PRINT_098_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);
	
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
                                        case 3:
                                        case 4:
                                        case 5:
                                        case 6:
                                                inPageLoop = _PAGE_LOOP_1_;
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
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
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
Function        :inMENU_098_REFUND
Date&Time       :2017/8/25 下午 6:07
Describe        :
*/
int inMENU_098_REFUND(EventMenuItem *srEventMenuItem)
{
	int			inRetVal = VS_SUCCESS;
	
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
		
		/* 無視HG */
                inRetVal = inMENU_098_CREDIT_REFUND(srEventMenuItem);
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
	
	return (inRetVal);
}

/*
Function        :inMENU_098_CREDIT_REFUND
Date&Time       :2017/8/28 上午 11:09
Describe        :
*/
int inMENU_098_CREDIT_REFUND(EventMenuItem *srEventMenuItem)
{
	int			inRetVal = VS_SUCCESS;
	int			inChoice = 0;
	int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char			szKey = 0x00;
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	, _REFUND_		, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_1_2_1_1_REFUND_		},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_	, _REDEEM_REFUND_	, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_1_2_1_2_REDEEM_REFUND_	},
		{_Touch_NONE_			, _TRANS_TYPE_NULL_	, NULL						, ""						}
	};
		
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_MENU_REFUND_OPTION_2_1_, 0, _COORDINATE_Y_LINE_8_4_);
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
Function	:inMENU_005_FPG_MenuFlow_NEWUI
Date&Time	:2022/10/5 下午 8:16
Describe	:選擇功能Menu
*/
int inMENU_005_FPG_MenuFlow_NEWUI(EventMenuItem *srEventMenuItem)
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
					inRetVal = inMENU_005_FUNCTION_NEWUI(srEventMenuItem);
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
					inRetVal = inMENU_005_FUNCTION_NEWUI(srEventMenuItem);
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
		
	}while (uszRejudgeBit == VS_TRUE);
	
        return (inRetVal);
}

/*
Function	:inMENU_005_FUNCTION_NEWUI
Date&Time	:2022/10/5 下午 8:08
Describe	:
*/
int inMENU_005_FUNCTION_NEWUI(EventMenuItem *srEventMenuItem)
{
        int			inPageLoop = _PAGE_LOOP_1_;
        int			inRetVal = VS_SUCCESS;
        int			inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
        int			inCount= 1;
        int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char			szKey = 0x00;
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
                {_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_	, _TRANS_TYPE_NULL_	, inMENU_Check_Mark_Enable		, _ICON_HIGHTLIGHT_1_TRANSACTION_	},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_	, _TRANS_TYPE_NULL_	, inMENU_Check_Mark_Enable		, _ICON_HIGHTLIGHT_2_CUP_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_	, _TRANS_TYPE_NULL_	, inMENU_Check_Mark_Enable		, _ICON_HIGHTLIGHT_3_SMARTPAY_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_	, _TRANS_TYPE_NULL_	, inMENU_Check_Mark_Enable		, _ICON_HIGHTLIGHT_4_ETICKET_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_	, _TRANS_TYPE_NULL_	, inMENU_Check_Mark_Enable		, _ICON_HIGHTLIGHT_5_AWARD_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_	, _TRANS_TYPE_NULL_	, inMENU_Check_Mark_Enable		, _ICON_HIGHTLIGHT_6_HG_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_7_	, _TRANS_TYPE_NULL_	, inMENU_Check_SETTLE_Enable		, _ICON_HIGHTLIGHT_7_SETTLE_		},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_8_	, _TRANS_TYPE_NULL_	, inMENU_Check_REVIEW_SETTLE_Enable	, _ICON_HIGHTLIGHT_8_REVIEW_PRINT_	},
		{_Touch_NONE_					, _TRANS_TYPE_NULL_	, NULL					, ""					}
	};
	
        /* 初始化 */
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第1.5層顯示 LOGO */
        inDISP_PutGraphic(_TOUCH_NEWUI_FUNCTION_PAGE_, 0,  _COORDINATE_Y_LINE_8_3_);

	if (ginHalfLCD == VS_FALSE)
		inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

	/* 檢查功能開關，並顯示反白的圖 */
	inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);

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
                                case 3:
				case 4:
				case 5:
                                case 6:
                                        inRetVal = VS_SUCCESS;
                                        inPageLoop = _PAGE_LOOP_1_;
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
					inRetVal = inMENU_NEWUI_SETTING_MENU(srEventMenuItem);
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
				inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

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
Function	:inMENU_041_CASH_MenuFlow_NEWUI
Date&Time	:2022/10/6 下午 7:56
Describe	:選擇功能Menu
*/
int inMENU_041_CASH_MenuFlow_NEWUI(EventMenuItem *srEventMenuItem)
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
					inRetVal = inMENU_041_FUNCTION_NEWUI(srEventMenuItem);
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
					inRetVal = inMENU_MenuKeyInAndGetAmount_041(srEventMenuItem);
					break;
				case _KEY_ENTER_:
					inRetVal = inMENU_041_FUNCTION_NEWUI(srEventMenuItem);
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
		
	}while (uszRejudgeBit == VS_TRUE);
	
        return (inRetVal);
}

/*
Function	:inMENU_041_FUNCTION_NEWUI
Date&Time	:2022/10/6 下午 7:56
Describe	:
*/
int inMENU_041_FUNCTION_NEWUI(EventMenuItem *srEventMenuItem)
{
        int			inPageLoop = _PAGE_LOOP_1_;
        int			inRetVal = VS_SUCCESS;
        int			inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
        int			inCount= 1;
        int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char			szKey = 0x00;
	MENU_CHECK_2_TABLE_	srMenuChekDisplay[] =
	{
                {_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_	, _CASH_ADVANCE_	, _MENU_CHECK_FORCE_DISPLAY_NONE_	, inNCCC_Func_Check_Transaction_Function, 
		"預借",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_8_, VS_FALSE, 
		"現金  1",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_9_, VS_FALSE},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_	, _VOID_		, _MENU_CHECK_FORCE_DISPLAY_NONE_	, inNCCC_Func_Check_Transaction_Function, 
		"取消",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_2_, _COORDINATE_Y_LINE_24_8_, VS_FALSE, 
		"交易  2",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_2_, _COORDINATE_Y_LINE_24_9_, VS_FALSE},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_	, _SALE_OFFLINE_	, _MENU_CHECK_FORCE_DISPLAY_NONE_	, inNCCC_Func_Check_Transaction_Function, 
		"交易",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_3_, _COORDINATE_Y_LINE_24_8_, VS_FALSE, 
		"補登  3",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_3_, _COORDINATE_Y_LINE_24_9_, VS_FALSE},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_	, _TRANS_TYPE_NULL_	, _MENU_CHECK_FORCE_DISPLAY_NONE_	, inMENU_Check_SETTLE_Enable, 
		"結帳",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_12_, VS_FALSE, 
		"      4",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_13_, VS_FALSE},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_	, _TRANS_TYPE_NULL_	, _MENU_CHECK_FORCE_DISPLAY_NONE_	, inMENU_Check_REVIEW_SETTLE_Enable, 
		"查詢",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_2_, _COORDINATE_Y_LINE_24_12_, VS_FALSE, 
		"列印  5",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_2_, _COORDINATE_Y_LINE_24_13_, VS_FALSE},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_	, _TRANS_TYPE_NULL_	, _MENU_CHECK_FORCE_DISPLAY_ON_		, NULL, 
		"設定",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_3_, _COORDINATE_Y_LINE_24_12_, VS_FALSE, 
		"      6",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_3_, _COORDINATE_Y_LINE_24_13_, VS_FALSE},
		{_Touch_NONE_}
	};
	
        /* 初始化 */
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第1.5層顯示 LOGO */
	/* 檢查功能開關，並顯示反白的圖 */
	inMENU_CHECK_2_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);

	if (ginHalfLCD == VS_FALSE)
		inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

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
				case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_:
					inCount = 6;
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
					inRetVal = inMENU_CASH_ADVANCE(srEventMenuItem);
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
                                case 3:
					inRetVal = inMENU_FORCE_CASH_ADVANCE(srEventMenuItem);
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
				case 5:
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
				case 6:
					inRetVal = inMENU_NEWUI_SETTING_MENU(srEventMenuItem);
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
				inDISP_PutGraphic(_MSG_RETURN_IDLE_BTN_, 0,  _COORDINATE_Y_LINE_8_8_);

			inMENU_CHECK_2_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
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
Function        :inMENU_026_NEWUI_SETTING_MENU
Date&Time       :2022/10/17 下午 8:34
Describe        :多了設定POS ID
*/
int inMENU_026_NEWUI_SETTING_MENU(EventMenuItem *srEventMenuItem)
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
	inMENU_Display_ICON(_ICON_TEMPLATE_BLUE_, _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode("POS", _FONTSIZE_24X33_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_MENU_3_, _COORDINATE_Y_LINE_24_12_, VS_FALSE);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode("ID    6", _FONTSIZE_24X33_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_MENU_3_, _COORDINATE_Y_LINE_24_13_, VS_FALSE);

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
				case 6:
					inRetVal = inNCCC_Func_Get_POS_ID();
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
			inMENU_Display_ICON(_ICON_TEMPLATE_BLUE_, _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("POS", _FONTSIZE_24X33_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_MENU_3_, _COORDINATE_Y_LINE_24_12_, VS_FALSE);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("ID", _FONTSIZE_24X33_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_MENU_3_, _COORDINATE_Y_LINE_24_13_, VS_FALSE);
	
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
Function	:inMENU_123_MenuFlow_NEWUI
Date&Time	:2023/6/26 下午 3:39
Describe	:選擇功能Menu
*/
int inMENU_123_MenuFlow_NEWUI(EventMenuItem *srEventMenuItem)
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
					/* (需求單-112101)-IKEA新增客製化參數需 by Russell 2023/6/20 下午 3:44 */
					if (guszCus123UnlockBit != VS_TRUE)
					{
						break;
					}
					else
					{
						guszCus123UnlockBit = VS_FALSE;
					}
					
					inRetVal = inMENU_FUNCTION_NEWUI_PAGE_Flow(srEventMenuItem);
					break;
				case _KEY_0_:
					/* (需求單-112101)-IKEA新增客製化參數需 by Russell 2023/6/20 下午 3:44 */
					if (guszCus123UnlockBit != VS_TRUE)
					{
						if (inMENU_123_Check_Password() == VS_TRUE)
						{
							guszCus123UnlockBit = VS_TRUE;
						}
						
						inNCCC_Func_Decide_Display_Idle_Image();
					}
					else
					{
						guszCus123UnlockBit = VS_FALSE;
					}
					
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
					/* (需求單-112101)-IKEA新增客製化參數需 by Russell 2023/6/20 下午 3:44 */
					if (guszCus123UnlockBit != VS_TRUE)
					{
						if (inMENU_123_Check_Password() == VS_TRUE)
						{
							guszCus123UnlockBit = VS_TRUE;
						}
						break;
					}
					else
					{
						guszCus123UnlockBit = VS_FALSE;
					}
					
					inRetVal = inMENU_MenuKeyInAndGetAmount(srEventMenuItem);
					break;
				case _KEY_ENTER_:
					/* (需求單-112101)-IKEA新增客製化參數需 by Russell 2023/6/20 下午 3:44 */
					if (guszCus123UnlockBit != VS_TRUE)
					{
						if (inMENU_123_Check_Password() == VS_TRUE)
						{
							guszCus123UnlockBit = VS_TRUE;
						}
						break;
					}
					else
					{
						guszCus123UnlockBit = VS_FALSE;
					}
					
					inRetVal = inMENU_FUNCTION_NEWUI_PAGE_Flow(srEventMenuItem);
					break;
				case _KEY_FUNCTION_:
					/* (需求單-112101)-IKEA新增客製化參數需 by Russell 2023/6/20 下午 3:44 */
					if (guszCus123UnlockBit != VS_TRUE)
					{
						if (inMENU_123_Check_Password() == VS_TRUE)
						{
							guszCus123UnlockBit = VS_TRUE;
						}
					}
					else
					{
						guszCus123UnlockBit = VS_FALSE;
					}
					
					break;
				case _KEY_F1_:
					/* (需求單-112101)-IKEA新增客製化參數需 by Russell 2023/6/20 下午 3:44 */
					if (guszCus123UnlockBit != VS_TRUE)
					{
						if (inMENU_123_Check_Password() == VS_TRUE)
						{
							guszCus123UnlockBit = VS_TRUE;
						}
					}
					else
					{
						guszCus123UnlockBit = VS_FALSE;
					}
					break;
				case _KEY_F2_:
					/* (需求單-112101)-IKEA新增客製化參數需 by Russell 2023/6/20 下午 3:44 */
					if (guszCus123UnlockBit != VS_TRUE)
					{
						if (inMENU_123_Check_Password() == VS_TRUE)
						{
							guszCus123UnlockBit = VS_TRUE;
						}
					}
					else
					{
						guszCus123UnlockBit = VS_FALSE;
					}
					break;
				case _KEY_F3_:
					/* (需求單-112101)-IKEA新增客製化參數需 by Russell 2023/6/20 下午 3:44 */
					if (guszCus123UnlockBit != VS_TRUE)
					{
						if (inMENU_123_Check_Password() == VS_TRUE)
						{
							guszCus123UnlockBit = VS_TRUE;
						}
					}
					else
					{
						guszCus123UnlockBit = VS_FALSE;
					}
					break;
				case _KEY_F4_:
					/* (需求單-112101)-IKEA新增客製化參數需 by Russell 2023/6/20 下午 3:44 */
					if (guszCus123UnlockBit != VS_TRUE)
					{
						if (inMENU_123_Check_Password() == VS_TRUE)
						{
							guszCus123UnlockBit = VS_TRUE;
						}
					}
					else
					{
						guszCus123UnlockBit = VS_FALSE;
					}
					break;
				case _KEY_CANCEL_:
					/* (需求單-112101)-IKEA新增客製化參數需 by Russell 2023/6/20 下午 3:44 */
					if (guszCus123UnlockBit != VS_TRUE)
					{
						if (inMENU_123_Check_Password() == VS_TRUE)
						{
							guszCus123UnlockBit = VS_TRUE;
						}
					}
					else
					{
						guszCus123UnlockBit = VS_FALSE;
					}
					
					if (ginEngineerDebug == VS_TRUE)
					{
						exit(0);
					}
					break;
				case _KEY_CLEAR_:
					/* (需求單-112101)-IKEA新增客製化參數需 by Russell 2023/6/20 下午 3:44 */
					if (guszCus123UnlockBit != VS_TRUE)
					{
						if (inMENU_123_Check_Password() == VS_TRUE)
						{
							guszCus123UnlockBit = VS_TRUE;
						}
					}
					else
					{
						guszCus123UnlockBit = VS_FALSE;
					}
					
					if (ginEngineerDebug == VS_TRUE)
					{
						inFunc_ls("-R -l", _AP_ROOT_PATH_);
					}
					break;
				case _KEY_DOT_:
					/* (需求單-112101)-IKEA新增客製化參數需 by Russell 2023/6/20 下午 3:44 */
					if (guszCus123UnlockBit != VS_TRUE)
					{
						if (inMENU_123_Check_Password() == VS_TRUE)
						{
							guszCus123UnlockBit = VS_TRUE;
						}
					}
					else
					{
						guszCus123UnlockBit = VS_FALSE;
					}
					
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
		
	}while (uszRejudgeBit == VS_TRUE);
	
        return (inRetVal);
}

/*
Function	:inMENU_123_Check_Password
Date&Time	:2023/6/26 下午 3:51
Describe	:
 *		甲、	抓TMS管理號碼功能之結帳交易管理號碼
 *		乙、	當正確輸入管理號碼後，即可依單機操作簡表進行作業; 10秒內未鍵入正確管理號碼，則返回待機畫面。
 *		丙、	按清除鍵，返回待機畫面。
 *		丁、	僅單機操作需要輸入管理號碼; 收銀機連動交易時不須輸入。
 *		戊、	若未設定TMS結帳交易管理號碼，所有按鍵全鎖只能透由收銀機啟動交易。
 *		客製化124沿用，秒數調整為15

*/
int inMENU_123_Check_Password(void)
{
	int	i = 0;
	int	inRetVal = VS_SUCCESS;
	int	inRetVal2 = VS_SUCCESS;
	char	szSettlementPwdEnable[1 + 1] = {0};
	char	szSettlementPwd[4 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	DISPLAY_OBJECT srDispObj;
	
	memset(szSettlementPwdEnable, 0x00, sizeof(szSettlementPwdEnable));
	inGetSettlementPwdEnable(szSettlementPwdEnable);
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (memcmp(szSettlementPwdEnable, "Y", 1) != 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Cus-123 SettlePwd Unable");
		}
		return (VS_FALSE);
	}
	
	memset(szSettlementPwd, 0x00, sizeof(szSettlementPwd));
	inGetSettlementPwd(szSettlementPwd);
	if (strlen(szSettlementPwd) <= 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Cus-123 No Settle Pwd");
		}
		return (VS_FALSE);
	}
	
	for (i = 0; i < 4; i++)
	{
		if ((szSettlementPwd[i] - '0') < 0 || 
		    (szSettlementPwd[i] - '9') > 0)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Cus-123 illegal Pwd:'%02X, %02X, %02X, %02X'", szSettlementPwd[0], szSettlementPwd[1], szSettlementPwd[2], szSettlementPwd[3]);
			}
			return (VS_FALSE);
		}
	}
	
	inFlushKBDBuffer();

	/* 輸入管理號碼 */
	inDISP_ClearAll();
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 請輸入單機密碼 */
	inDISP_ChineseFont("請輸入單機密碼", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
	
	while (1)
	{
                inDISP_BEEP(1, 0);
		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

                /* 設定顯示變數 */
                srDispObj.inMaxLen = 4;
                srDispObj.inY = _LINE_8_7_;
                srDispObj.inR_L = _DISP_RIGHT_;
                srDispObj.inMask = VS_TRUE;
		srDispObj.inColor = _COLOR_RED_;
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			srDispObj.inTimeout = _CUSTOMER_124_EVER_RICH_ENTER_PASSWORD_TIMEOUT_;
		}
		else
		{
			srDispObj.inTimeout = _CUSTOMER_123_IKEA_ENTER_PASSWORD_TIMEOUT_;
		}

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
                inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		{
			inRetVal2 = VS_ERROR;
			break;
		}

		if (srDispObj.inOutputLen == strlen(szSettlementPwd))
		{
			if (!memcmp(szSettlementPwd, srDispObj.szOutput, strlen(szSettlementPwd)))
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inRetVal2 = VS_SUCCESS;
				break;
			}
                        else 
			{
                            inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
			}
		}
                else 
		{
                        inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		}
	}
	
	if (inRetVal2 != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Cus-123 Enter Pwd Fail");
		}
		return (VS_FALSE);
	}
	else
	{
		return (VS_TRUE);
	}
}

/*
Function        :inMENU_123_NEWUI_SETTING_MENU
Date&Time       :2023/9/7 下午 5:29
Describe        :
*/
int inMENU_123_NEWUI_SETTING_MENU(EventMenuItem *srEventMenuItem)
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
                {_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_	, _TRANS_TYPE_NULL_	, _MENU_CHECK_FORCE_DISPLAY_ON_		, NULL, 
		"設定",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_8_, VS_FALSE, 
		"管理  ",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_9_, VS_FALSE,
		"號碼  1",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_10_, VS_FALSE},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_	, _TRANS_TYPE_NULL_	, _MENU_CHECK_FORCE_DISPLAY_ON_		, NULL, 
		"資訊",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_2_, _COORDINATE_Y_LINE_24_8_, VS_FALSE, 
		"回報  2",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_2_, _COORDINATE_Y_LINE_24_9_, VS_FALSE},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_	, _TRANS_TYPE_NULL_	, _MENU_CHECK_FORCE_DISPLAY_ON_		, NULL, 
		"參數",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_3_, _COORDINATE_Y_LINE_24_8_, VS_FALSE, 
		"下載  3",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_3_, _COORDINATE_Y_LINE_24_9_, VS_FALSE},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_	, _TRANS_TYPE_NULL_	, _MENU_CHECK_FORCE_DISPLAY_ON_		, NULL, 
		"版本",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_12_, VS_FALSE, 
		"查詢  4",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_13_, VS_FALSE},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_	, _TRANS_TYPE_NULL_	, _MENU_CHECK_FORCE_DISPLAY_ON_		, NULL, 
		"其他",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_2_, _COORDINATE_Y_LINE_24_12_, VS_FALSE, 
		"      5",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_2_, _COORDINATE_Y_LINE_24_13_, VS_FALSE},
		{_NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_	, _TRANS_TYPE_NULL_	, _MENU_CHECK_FORCE_DISPLAY_ON_		, NULL, 
		"設定",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_3_, _COORDINATE_Y_LINE_24_12_, VS_FALSE, 
		"提示聲6",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_3_, _COORDINATE_Y_LINE_24_13_, VS_FALSE},
		{_Touch_NONE_}
	};
	
	memset(szCustomerIndicator, 0x00 , sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
        /* 初始化 */
	inDISP_ClearAll();
	inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_9_TITLE_, 0,  _COORDINATE_Y_LINE_8_2_);	
	/* 檢查功能開關，並顯示反白的圖 */
	inMENU_CHECK_2_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);

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
				case 6:
					inRetVal = inNCCC_Func_Customer_123_Edit_Signpad_Beep_Interval();
					if (inRetVal == VS_USER_CANCEL)
					{
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_1_;
					}
					else
					{
						inPageLoop = _PAGE_LOOP_0_;
					}
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
	
			inMENU_CHECK_2_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
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