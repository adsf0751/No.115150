#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <signature.h>
#include <epad.h>
#include <sqlite3.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/AllStruct.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DisTouch.h"
#include "../DISPLAY/DispMsg.h"
#include "../EVENT/MenuMsg.h"
#include "../EVENT/Menu.h"
#include "../EVENT/Flow.h"
#include "../../FISC/NCCCfisc.h"
#include "../../NCCC/NCCCesc.h"
#include "Sqlite.h"
#include "Function.h"
#include "Batch.h"
#include "Card.h"
#include "CFGT.h"
#include "HDT.h"
#include "HDPT.h"
#include "EDC.h"
#include "File.h"
#include "Signpad.h"
#include "FuncTable.h"
#include "Utility.h"

SIGNPAD_OBJECT	gsrSignpad = {_SIGNPAD_LEFT_, VS_FALSE, 0};
extern int	ginDebug;
extern int	ginFindRunTime;
extern int	ginTouchEnable;
extern int	ginISODebug;

/*
Function        :inSIGN_TouchSignature_Start
Date&Time       :2017/7/24 上午 11:27
Describe        :簽名底層API開始
*/
int inSIGN_TouchSignature_Start(unsigned int uiX ,unsigned int uiY,unsigned int uiWidth ,unsigned int uiHeight,unsigned char *uszBMPFileName,unsigned long ulTimeout)
{

#ifndef	_LOAD_KEY_AP_

	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;

	usRetVal = CTOS_TouchSignatureStart(uiX, uiY, uiWidth, uiHeight , uszBMPFileName, ulTimeout);
	if (usRetVal == d_OK)
	{
		
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_TouchSignatureStart Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}

#endif
	
	return (VS_SUCCESS);
}

/*
Function        :inSIGN_TouchSignature_Internal_END
Date&Time       :2017/4/20 上午 11:56
Describe        :
*/
int inSIGN_TouchSignature_Internal_END(void)
{

#ifndef	_LOAD_KEY_AP_

	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	usRetVal = CTOS_TouchSignatureTerminate();
	if (usRetVal == d_OK)
	{
		
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, " Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}

#endif
	
	return (VS_SUCCESS);
}

/*
Function        :inSIGN_GetSignatureStatus
Date&Time       :2017/7/24 上午 10:34
Describe        :
		Signature Status
		d_SCP_STATUS_NO_DATA	  0x00000001    未碰到螢幕
		d_SCP_STATUS_IDLE	  0x00000002    有碰到螢幕後手離開螢幕
		d_SCP_STATUS_SIGNING      0x00000003    手在螢幕上
*/
int inSIGN_GetSignatureStatus(unsigned long *ulStatus , unsigned long *ulDuration)
{

#ifndef	_LOAD_KEY_AP_

	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = VS_ERROR;
	
	usRetVal = CTOS_GetSignatureStatus(ulStatus, ulDuration);
	if (usRetVal == d_OK)
	{
		
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_GetSignatureStatus Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}

#endif
	
	return (VS_SUCCESS);
}

/*
Function        :inSIGN_IsSigned
Date&Time       :2017/7/24 上午 10:49
Describe        :確認是否有被簽名過
*/
int inSIGN_IsSigned(SIGNPAD_OBJECT *srSignpad)
{
	int		inRetVal = VS_ERROR;
	unsigned long	ulStatus = 0;
	unsigned long	ulDuration = 0;
	
	inRetVal = inSIGN_GetSignatureStatus(&ulStatus, &ulDuration);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	if (ulStatus == d_SCP_STATUS_NO_DATA || srSignpad->inSigned != VS_TRUE)
	{
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inSIGN_BMPConverter_Left
Date&Time       :2017/7/24 上午 11:56
Describe        :
*/
int inSIGN_BMPConverter_Left(unsigned char *uszInputBMPFileName, unsigned char *uszOutputBMPFileName)
{

#ifndef	_LOAD_KEY_AP_

	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;

	usRetVal = CTOS_BMPConverter(uszInputBMPFileName, uszOutputBMPFileName, d_BMP_CONVERT_LEFT_ROTATE);
	if (usRetVal == d_OK)
	{
		
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BMPConverter Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
#endif	

	
	return (VS_SUCCESS);
}

/*
Function        :inSIGN_BMPConverter_Right
Date&Time       :2017/7/24 下午 1:17
Describe        :
*/
int inSIGN_BMPConverter_Right(unsigned char *uszInputBMPFileName, unsigned char *uszOutputBMPFileName)
{

#ifndef	_LOAD_KEY_AP_

	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;

	usRetVal = CTOS_BMPConverter(uszInputBMPFileName, uszOutputBMPFileName, d_BMP_CONVERT_RIGHT_ROTATE);
	if (usRetVal == d_OK)
	{
		
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BMPConverter Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
#endif
	
	return (VS_SUCCESS);
}

/*
Function        :inSIGN_BMPConverter_OneColor
Date&Time       :2017/7/24 下午 1:17
Describe        :轉成單色
*/
int inSIGN_BMPConverter_OneColor(unsigned char *uszInputBMPFileName, unsigned char *uszOutputBMPFileName)
{
#ifndef	_LOAD_KEY_AP_

	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;

	usRetVal = CTOS_BMPConverter(uszInputBMPFileName, uszOutputBMPFileName, d_BMP_CONVERT_ONE_BIT_COLOR);
	if (usRetVal == d_OK)
	{
		
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BMPConverter Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
#endif
	
	return (VS_SUCCESS);
}

/*
Function        :inSIGN_Text_To_BMP
Date&Time       :2017/7/24 下午 1:53
Describe        :
*/
int inSIGN_Text_To_BMP(unsigned long ulwidth, unsigned long ulHeight, unsigned short usX, unsigned short usY, unsigned char *uszString,unsigned short uszFontSize,char *szBMPFileName)
{
#ifndef	_LOAD_KEY_AP_
	vdCTOSS_TextBufferToBMP(ulwidth, ulHeight, usX, usY, uszString, uszFontSize, szBMPFileName);
#endif
		
	return (VS_SUCCESS);
}

/*
Function        :inSIGN_Text_To_BMPEx
Date&Time       :2017/7/24 下午 1:53
Describe        :
*/
int inSIGN_Text_To_BMPEx(unsigned short usX, unsigned short usY, unsigned char *uszString,unsigned short uszFontSize,char *szBMPFileName)
{
#ifndef	_LOAD_KEY_AP_
	vdCTOSS_TextBufferToBMPEx(usX, usY, uszString, uszFontSize, szBMPFileName);
#endif
		
	return (VS_SUCCESS);
}

/*
Function        :inSIGN_Rotate_TextBMP
Date&Time       :2017/7/24 下午 2:07
Describe        :用vdCTOSS_TextBufferToBMP產生的BMP只能用這個轉，有可能0是左轉，1是右轉之類的定義
*/
int inSIGN_Rotate_TextBMP(char *szFileName, int inRotate)
{
#ifndef	_LOAD_KEY_AP_
	vdCTOSS_RotateBMPFileEx(szFileName, inRotate);
#endif
	
	return (VS_SUCCESS);
}

/*
Function        :inSIGN_TouchSignature_Flow
Date&Time       :2015/12/29 下午 3:55
Describe        :SignPad觸控簽名功能
*/
int inSIGN_TouchSignature_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
        char	szSignPadMode[2 + 1];
	char	szDebugMsg[100 + 1];
	char	szCustomerIndicator[3 + 1] = {0};
        
        vdUtility_SYSFIN_LogMessage(AT, "inSIGN_TouchSignature_Flow START!");
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inSIGN_TouchSignature_Flow() START !");
	}
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
        /* CFGT的Signpad開關判斷 */
        memset(szSignPadMode , 0x00, sizeof(szSignPadMode));
        inGetSignPadMode(szSignPadMode);
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 修改免簽條件的地方，現在修改到_CREDIT_FUNCTION_CHECKRESULT_內 2024/9/25 下午 1:53 */
        
	while (1)
	{
		/* 不開signpad或免簽 */
		if (!memcmp(szSignPadMode, _SIGNPAD_MODE_0_NO_, 1))
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "不支援簽名版");
				inLogPrintf(AT, szDebugMsg);
			}

			/* 更新簽名狀態 */
			pobTran->srBRec.inSignStatus = _SIGN_BYPASS_;
			inBATCH_Update_Sign_Status_By_Sqlite(pobTran);
			
			if (ginFindRunTime == VS_TRUE)
			{
				inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
			}
			
			/* 更新ESC暫存檔狀態 */
			inFLOW_RunFunction(pobTran, _NCCC_FUNCTION_ESC_FUNCPROCESS_);

			break;
		}
		else if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "達成免簽條件");
				inLogPrintf(AT, szDebugMsg);
			}

			/* 更新簽名狀態 */
			pobTran->srBRec.inSignStatus = _SIGN_NO_NEED_SIGN_;
			inBATCH_Update_Sign_Status_By_Sqlite(pobTran);
			
			if (ginFindRunTime == VS_TRUE)
			{
				inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
			}
			
			/* 更新ESC暫存檔狀態 */
			inFLOW_RunFunction(pobTran, _NCCC_FUNCTION_ESC_FUNCPROCESS_);

			break;
		}
		/* 使用內建簽名板 */
		else if (!memcmp(szSignPadMode, _SIGNPAD_MODE_1_INTERNAL_, strlen(_SIGNPAD_MODE_1_INTERNAL_)))
		{
			if (ginTouchEnable != VS_TRUE)
                        {
                                vdUtility_SYSFIN_LogMessage(AT, "inSIGN_TouchSignature_Flow No_Touch_Pass END!");
				return (VS_SUCCESS);
                        }

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "使用內建簽名板internal");
				inLogPrintf(AT, szDebugMsg);
			}

			inSIGN_TouchSignature_Internal(pobTran);
		}
		else if (!memcmp(szSignPadMode, _SIGNPAD_MODE_2_EXTERNAL_, strlen(_SIGNPAD_MODE_2_EXTERNAL_)))
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "使用外接簽名板external");
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 更新簽名狀態 */
			pobTran->srBRec.inSignStatus = _SIGN_BYPASS_;
			inBATCH_Update_Sign_Status_By_Sqlite(pobTran);
			
			if (ginFindRunTime == VS_TRUE)
			{
				inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
			}
			
			/* 更新ESC暫存檔狀態 */
			inFLOW_RunFunction(pobTran, _NCCC_FUNCTION_ESC_FUNCPROCESS_);
			
			break;
		}
		
		/* 客製化123不需要確認畫面 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_) ||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			break;
		}
		else
		{
			/* 有簽名就要，確認簽名畫面 */
			if (pobTran->srBRec.inSignStatus == _SIGN_SIGNED_)
			{
				/* 確認OK跳出 */
				inRetVal = inSIGN_CheckSignature(pobTran);
				if (inRetVal == VS_SUCCESS)
				{
					break;
				}
				/* 確認不OK，重簽 */
				else
				{
					continue;
				}
			}
			/* 應簽而沒簽名要再確認一次是否確定不要用簽名板 */
			else if (pobTran->srBRec.inSignStatus == _SIGN_BYPASS_)
			{
				/* 確認OK跳出 */
				inRetVal = inSIGN_Check_NOSignature(pobTran);
				if (inRetVal == VS_SUCCESS)
				{
					vdUtility_SYSFIN_LogMessage(AT, "簽名板出紙本");
					break;
				}
				/* 確認不OK，重簽 */
				else
				{
					continue;
				}
			}
			else
			{
				break;
			}
		}
		
	}
	
	/* 恢復UI */
	inFunc_ResetTitle(pobTran);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inSIGN_TouchSignature_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
        
        vdUtility_SYSFIN_LogMessage(AT, "inSIGN_TouchSignature_Flow END!");
        
	return (VS_SUCCESS);
}

/*
Function        :inSIGN_TouchSignature_Internal
Date&Time       :2017/4/20 上午 11:44
Describe        :SignPad內建觸控簽名功能
*/
int inSIGN_TouchSignature_Internal(TRANSACTION_OBJECT *pobTran)
{
	int		inChoice;
        int		inTouchSensorFunc;
	char		szMemoSignBMPFile[32 + 1];
	char		szCustomerIndicator[3 + 1] = {0};
	char		szTemplate[10 + 1] = {0};
	char		szBeepTimeInterval[3 + 1] = {0};
	long		lnBeepTimeInterval = 0;
	unsigned char   uszKey = 0x00;
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 校正touch IC */
	/* inSIGN_TouchSignature_Internal_Calibration一定要在inSIGN_TouchSignature_Internal_START 之前 */
	inSIGN_TouchSignature_Internal_Calibration();
	
	/* 初始化結構 */
	memset(&gsrSignpad, 0x00, sizeof(gsrSignpad));
	
	/* 預設為左邊 */
	gsrSignpad.inPosition = _SIGNPAD_RIGHT_;    
        inTouchSensorFunc = _Touch_SIGNATURE_;
        inSIGN_TouchSignature_Internal_START(pobTran, &gsrSignpad);

        /* 初始化 用Invoice Number來命名 */
        memset(szMemoSignBMPFile, 0x00, sizeof(szMemoSignBMPFile));
	inFunc_ComposeFileName_InvoiceNumber(pobTran, szMemoSignBMPFile, _PICTURE_FILE_EXTENSION_, 6);
	/* 先刪檔避免沒簽名用到上個批次漏刪的簽名圖檔(保險起見) */
	inFILE_Delete((unsigned char*)szMemoSignBMPFile);
	
        /* 清鍵盤buffer */
        inFlushKBDBuffer();
	/* 清空觸控事件 */
	inDisTouch_Flush_TouchFile();
	
	/* 客製化123，簽名時設為最亮 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		memset(szBeepTimeInterval, 0x00 , sizeof(szBeepTimeInterval));
		inGetSignpadBeepInterval(szBeepTimeInterval);
		if (atoi(szBeepTimeInterval) >= atoi(_SIGNPAD_BEEP_INTERVAL_CUS_123_MIN_) && 
		    atoi(szBeepTimeInterval) <= atoi(_SIGNPAD_BEEP_INTERVAL_CUS_123_MAX_) &&
		    strlen(szBeepTimeInterval) > 0)
		{
			lnBeepTimeInterval = atol(szBeepTimeInterval);
		}
		else
		{
			/* 預設值 */
			lnBeepTimeInterval = 100;
		}
		
		inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(_SCREEN_BRIGHTNESS_MAX_));
		inTimerStart(_TIMER_NEXSYS_1_, 0);
	}
	
        while (1)
        {
                uszKey = uszKBD_Key();
                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		
		/* 判斷是否在簽名板下筆 */
		if (inChoice == _SignTouch_Signpad_)
		{
			gsrSignpad.inSigned = VS_TRUE;
			/* 簽名板，於落筆簽名後，應重新計算timeout時間。 */
			inSIGN_TimeoutStart(&gsrSignpad.ulSignTimeStart);
		}
                
		/* 確認TimeOut並顯示 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_) ||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				inDISP_BEEP(1, 0);
				inTimerStart_MicroSecond(_TIMER_NEXSYS_1_, lnBeepTimeInterval);
			}
		}
		else
		{
			/* 進入簽名板流程後，應於1分鐘後開始提示聲 */
			if (inSIGN_TimeoutCheck(&gsrSignpad, 60, VS_FALSE) == VS_TIMEOUT)
			{
				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
				{
					inDISP_BEEP(1, 0);
					inTimerStart(_TIMER_NEXSYS_1_, 2);
				}
			}
			/* 1分30秒後Timeout。 */
			if (inSIGN_TimeoutCheck(&gsrSignpad, 90, VS_FALSE) == VS_TIMEOUT)
			{
				uszKey = _KEY_TIMEOUT_;
			}
		}
                
                if (inChoice == _SignTouch_Clear_ || uszKey == _KEY_CANCEL_)
                {
                        inSIGN_TouchSignature_Internal_END();
                        inSIGN_TouchSignature_Internal_START(pobTran, &gsrSignpad);
                }
                else if (inChoice == _SignTouch_Ok_ || uszKey == _KEY_ENTER_ || uszKey == _KEY_TIMEOUT_)
                {
			/* 客製化123，沒簽不給過 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
			    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
			{
				if (inSIGN_IsSigned(&gsrSignpad) != VS_SUCCESS)
				{
					continue;
				}
			}
			
                        /* 判斷簽名狀態 沒下過筆直接按確認則不存圖片 */
                        if (inSIGN_IsSigned(&gsrSignpad) != VS_SUCCESS)
                        {
                                memset(szMemoSignBMPFile, 0x00, sizeof(szMemoSignBMPFile));
                                inSIGN_TouchSignature_Internal_END();
				/* 刪檔 */
				inFile_Unlink_File(SignBMPFile, _AP_ROOT_PATH_);
				
				/* 沒簽名，當筆要出紙本 */
				/* 更新簽名狀態 */
				pobTran->srBRec.inSignStatus = _SIGN_BYPASS_;
				inBATCH_Update_Sign_Status_By_Sqlite(pobTran);
				/* 更新ESC暫存檔狀態 */
				inFLOW_RunFunction(pobTran, _NCCC_FUNCTION_ESC_FUNCPROCESS_);
                        }
                        else
                        {
                                inSIGN_TouchSignature_Internal_END();
                                /* 圖片旋轉 */
                                if (gsrSignpad.inPosition == _SIGNPAD_LEFT_)
				{
                                        inSIGN_BMPConverter_Left((unsigned char *)SignBMPFile, (unsigned char *)SignBMPFile);
				}
                                else if (gsrSignpad.inPosition == _SIGNPAD_RIGHT_)
				{
					inSIGN_BMPConverter_Right((unsigned char *)SignBMPFile, (unsigned char *)SignBMPFile);
				}

                                /* 圖片轉換成單色 */
                                inSIGN_BMPConverter_OneColor((unsigned char *)SignBMPFile, (unsigned char *)szMemoSignBMPFile);
				/* 放到fs_data內 */
                                inFile_Move_File(szMemoSignBMPFile, _AP_ROOT_PATH_, "", _FS_DATA_PATH_);
				
				/* 更新簽名狀態 */
				pobTran->srBRec.inSignStatus = _SIGN_SIGNED_;
				inBATCH_Update_Sign_Status_By_Sqlite(pobTran);
				/* 更新ESC暫存檔狀態 */
				inFLOW_RunFunction(pobTran, _NCCC_FUNCTION_ESC_FUNCPROCESS_);
                        }

                        break;
                }
                else if (inChoice == _SignTouch_Rotate_ || uszKey == d_KBD_DOT)
                {
                        
                        /* 判斷是否已經下筆 有下筆則擋住旋轉，（因為判斷整個畫面，所以按畫面上的按鈕也算下筆） */
			/* 現在將條件從d_SCP_STATUS_SIGNING && Duration為 0改為 Duration為 0，可以讓按一下或一直押著都旋轉，但仍無法判斷 */
                        if (inSIGN_IsSigned(&gsrSignpad) != VS_SUCCESS)
                        {
                                /* Signature Status為整個螢幕檢測 使用觸控旋轉的話 結果會是d_SCP_STATUS_SIGNING  Duration為 0 */
                        }
                        else
                        {
                                continue;
                        }
                        
                        inSIGN_TouchSignature_Internal_END();
                        
                        if (gsrSignpad.inPosition == _SIGNPAD_LEFT_)
                        {
                                gsrSignpad.inPosition = _SIGNPAD_RIGHT_;
                                inSIGN_TouchSignature_Internal_START(pobTran, &gsrSignpad);
                        }
                        else if (gsrSignpad.inPosition == _SIGNPAD_RIGHT_)
                        {
                                gsrSignpad.inPosition = _SIGNPAD_LEFT_;
                                inSIGN_TouchSignature_Internal_START(pobTran, &gsrSignpad);
                        } 
			
                }
                else
                {
                        continue;
                }
		
        }
	
	/* 客製化123，簽完時設為較暗 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetScreenBrightness(szTemplate);
		if (strlen(szTemplate) > 0)
		{
			inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(szTemplate));
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inSIGN_TouchSignature_START
Date&Time       :2015/12/29 下午 3:55
Describe        :用來判斷左邊簽名還是右邊簽名 並啟動簽名功能
*/
int inSIGN_TouchSignature_Internal_START(TRANSACTION_OBJECT *pobTran, SIGNPAD_OBJECT *srSignpad)
{
	int	inSigned = 0;
        char    szDispBuffer[64 + 1];
        char    szAmount[20 + 1];
	char	szOutputAmount[20 + 1];
        
        memset(szAmount, 0x00, sizeof(szAmount));
        /* 要含小費 */
	if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && 
	    (pobTran->srBRec.inOrgCode != _REFUND_ && pobTran->srBRec.inOrgCode != _INST_REFUND_ && pobTran->srBRec.inOrgCode != _REDEEM_REFUND_ && pobTran->srBRec.inOrgCode != _CUP_REFUND_ && pobTran->srBRec.inOrgCode != _CUP_MAIL_ORDER_REFUND_))	|| 
	     pobTran->srBRec.inCode == _REFUND_			|| 
	     pobTran->srBRec.inCode == _INST_REFUND_		|| 
	     pobTran->srBRec.inCode == _REDEEM_REFUND_		||
	     pobTran->srBRec.inCode == _CUP_REFUND_		||
	     pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
	{
		inSigned = _SIGNED_MINUS_;
		if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
		{
			sprintf(szAmount, "%ld", atol(pobTran->srBRec.szDCC_FCA) + atol(pobTran->srBRec.szDCC_TIPFCA));
		}
		else
		{
			sprintf(szAmount, "%ld", 0 - pobTran->srBRec.lnTotalTxnAmount);
		}
	}
	else
	{
		inSigned = _SIGNED_NONE_;
		if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
		{
			sprintf(szAmount, "%ld", atol(pobTran->srBRec.szDCC_FCA) + atol(pobTran->srBRec.szDCC_TIPFCA));
		}
		else
		{
			sprintf(szAmount, "%ld", pobTran->srBRec.lnTotalTxnAmount);
		}
	}
	
	/* DCC要加入含小費的金額 */
	memset(szOutputAmount, 0x00, sizeof(szOutputAmount));
	if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
		inFunc_Amount_Comma_DCC(szAmount, "", 0x00, inSigned, 18, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szOutputAmount);
	else
		inFunc_Amount_Comma_DCC(szAmount, "NT$", 0x00, inSigned, 18, _PADDING_RIGHT_, "0", "", szOutputAmount);
        
        memset(szDispBuffer, 0x00, sizeof(szDispBuffer));
        sprintf(szDispBuffer, "金額：%s", szOutputAmount);
        
        if (srSignpad->inPosition == _SIGNPAD_LEFT_)
        {
		inDISP_ClearAll();
		
                /* 簽名底圖 */
                inDISP_PutGraphic(_SIGNATURE_BACKGROUND_LEFT_, 0, _COORDINATE_Y_LINE_8_1_);
                
                /* 顯示金額 */
                inSIGN_Text_To_BMPEx(1, 5, (unsigned char *)szDispBuffer, _CHINESE_FONT_12X19_, AmountBMPFile);
                inDISP_PutGraphic(AmountBMPFile, 0,  _COORDINATE_Y_LINE_8_1_);
                
                /* 簽名區塊 & 初始化觸控區 */
                inDISP_Display_Black_Back(_SIGNEDPAD_OUT_LEFT_X1_, _SIGNEDPAD_OUT_LEFT_Y1_, _SIGNEDPAD_OUT_WIDTH_, _SIGNEDPAD_OUT_LENGTH_);
                inSIGN_TouchSignature_Start(_SIGNEDPAD_LEFT_X1_, _SIGNEDPAD_LEFT_Y1_, _SIGNEDPAD_WIDTH_, _SIGNEDPAD_LENGTH_, (unsigned char *)SignBMPFile, 0);
        }
        else
        {
		inDISP_ClearAll();
		
                /* 簽名底圖 */
		inDISP_PutGraphic(_SIGNATURE_BACKGROUND_RIGHT_, 0, _COORDINATE_Y_LINE_8_1_);
                
                /* 顯示金額 */
                inSIGN_Text_To_BMPEx(1, 1, (unsigned char *)szDispBuffer, _CHINESE_FONT_12X19_, AmountBMPFile);
		/* 旋轉兩次朝另一個方向 */
                inSIGN_Rotate_TextBMP(AmountBMPFile, 0);
                inSIGN_Rotate_TextBMP(AmountBMPFile, 0);
                inDISP_PutGraphic(AmountBMPFile, 260, 90);
                
                /* 簽名區塊 & 初始化觸控區 */
                inDISP_Display_Black_Back(_SIGNEDPAD_OUT_RIGHT_X1_, _SIGNEDPAD_OUT_RIGHT_Y1_, _SIGNEDPAD_OUT_WIDTH_, _SIGNEDPAD_OUT_LENGTH_);
                inSIGN_TouchSignature_Start(_SIGNEDPAD_RIGHT_X1_, _SIGNEDPAD_RIGHT_Y1_, _SIGNEDPAD_WIDTH_, _SIGNEDPAD_LENGTH_, (unsigned char *)SignBMPFile, 0);
        }
	
	/* 重置是否已下筆狀態 */
	srSignpad->inSigned = VS_FALSE;
	/* 重置TimeOut */
	/* 進入簽名板流程後，應於1分鐘後開始提示聲，1分30秒後Timeout。 */
	inSIGN_TimeoutStart(&srSignpad->ulSignTimeStart);

        return (VS_SUCCESS);
}

/*
Function	:inSIGN_TimeoutCheck
Date&Time	:2017/4/27 上午 10:59
Describe	:CTLS Check是否TimeOut
*/
int inSIGN_TimeoutCheck(SIGNPAD_OBJECT *srSignpad, int inTimeOut, unsigned char uszDispTimeout)
{
	int		inRemainSecond = 0;
	char		szTemplate[10 + 1];
	unsigned long	ulSecond = 0;
	unsigned long	ulMilliSecond = 0;
        
	inFunc_GetRunTime(srSignpad->ulSignTimeStart, &ulSecond, &ulMilliSecond);
	inRemainSecond = inTimeOut - (int)ulSecond;
        
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%02d", inRemainSecond);
	
	if (uszDispTimeout == VS_TRUE)
	{
		if (srSignpad->inPosition == _SIGNPAD_LEFT_)
		{
			inSIGN_Text_To_BMP(160, 70, 1, 5, (unsigned char *)szTemplate, _CHINESE_FONT_12X19_, TimeBMPFile);
			inSIGN_Rotate_TextBMP(TimeBMPFile, 0);
			inDISP_PutGraphic(TimeBMPFile, 0,  _COORDINATE_Y_LINE_8_7_);
		}
		else
		{
			inSIGN_Text_To_BMP(384, 70, 1, 5, (unsigned char *)szTemplate, _CHINESE_FONT_12X19_, TimeBMPFile);
			inSIGN_Rotate_TextBMP(TimeBMPFile, 0);
			inSIGN_Rotate_TextBMP(TimeBMPFile, 0);
			inDISP_PutGraphic(TimeBMPFile, 260, 0);
		}
	}

	if (inRemainSecond <= 0)
		return (VS_TIMEOUT);
	else
        	return (VS_SUCCESS);
}

/*
Function	:inSIGN_TimeoutStart
Date&Time	:2017/7/24 上午 11:50
Describe	:取得開始時秒數
*/
int inSIGN_TimeoutStart(unsigned long *ulRunTime)
{
	*ulRunTime = ulFunc_CalculateRunTime_Start();
	
	return (VS_SUCCESS);
}

/*
Function        :inSIGN_CheckSignature
Date&Time       :2017/7/24 下午 2:24
Describe        :請收銀員核對簽名，正確請按[0]，簽名不符請按清除
*/
int inSIGN_CheckSignature(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_SIGN_CHECK_;
	char	szKey = 0x00;
	char	szMemoSignBMPFile[32 + 1];
	char	szPath[100 + 1];
	
	/* 恢復UI */
	inDISP_ClearAll();
	
	/* 初始化 用Invoice Number來命名 */
	memset(szMemoSignBMPFile, 0x00, sizeof(szMemoSignBMPFile));
	inFunc_ComposeFileName_InvoiceNumber(pobTran, szMemoSignBMPFile, _PICTURE_FILE_EXTENSION_, 6);
	/* 前面要加上路徑 */
	memset(szPath, 0x00, sizeof(szPath));
	sprintf(szPath, "./fs_data/%s", szMemoSignBMPFile);
			
	inDISP_PutGraphic(szPath, 0, _COORDINATE_Y_LINE_8_1_);
	inDISP_PutGraphic(_SIGNPAD_CHECK_SIGN_, 0, _COORDINATE_Y_LINE_8_4_);
	/* 清除keyboad Buffer */
	inFlushKBDBuffer();
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		szKey = uszKBD_Key();
		
		if (szKey == _KEY_0_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
		else if (szKey == _KEY_CANCEL_)
		{
			/* 刪檔重簽 */
			inFILE_Delete((unsigned char*)szMemoSignBMPFile);
			
			/* 初始化簽名狀態 */
			pobTran->srBRec.inSignStatus = _SIGN_NONE_;
	
			inRetVal = VS_ERROR;
			break;
		}

	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
	
	return (inRetVal);
}

/*
Function        :inSIGN_Check_NOSignature
Date&Time       :2017/7/28 下午 3:45
Describe        :確認是否真的不簽名，顯示
		"此交易非電子簽名
		 會列印紙本簽單
		 列印紙本請按【0】
		 電子簽名請按清除
		"
*/
int inSIGN_Check_NOSignature(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	int	inChoice = _DisTouch_No_Event_;
	int	inTouchSensorFunc = _Touch_NOSIGN_CHECK_;
	char	szKey = 0x00;
	char	szMemoSignBMPFile[32 + 1];
	
	/* 恢復UI */
	inDISP_ClearAll();
	
	/* 此交易非電子簽名 */
	inDISP_PutGraphic(_SIGNPAD_CHECK_NOSIGN_, 0, _COORDINATE_Y_LINE_8_4_);
	/* 清除keyboad Buffer */
	inFlushKBDBuffer();
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                szKey = uszKBD_Key();
		
		if (szKey == _KEY_0_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
		else if (szKey == _KEY_CANCEL_)
		{
			
			/* 刪檔重簽 */
			inFILE_Delete((unsigned char*)szMemoSignBMPFile);
			
			/* 初始化簽名狀態 */
			pobTran->srBRec.inSignStatus = _SIGN_NONE_;
	
			inRetVal = VS_ERROR;
			break;
		}
//		else if (szKey == _KEY_TIMEOUT_)
//		{
//			inRetVal = VS_SUCCESS;
//			break;
//		}
	}
	
	return (inRetVal);
}

/*
Function        :inSIGN_TouchSignature_Internal_Calibration
Date&Time       :2020/7/15 下午 1:46
Describe        :SignPad內建觸控校正ICs
*/
int inSIGN_TouchSignature_Internal_Calibration(void)
{
	char		szDebugMsg[100 + 1] = {0};

	CTOS_TouchSignatureCalibration();
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Touch IC Calibration");
		inLogPrintf(AT, szDebugMsg);
	}

	
	return (VS_SUCCESS);
}

/*----------------測試function------------------*/
int inSIGN_TouchSignature_Test(void)
{
	TRANSACTION_OBJECT pobTran;
	
	memset(&pobTran, 0x00, sizeof(pobTran));
	inSIGN_TouchSignature_Flow(&pobTran);
		
	return (VS_SUCCESS);
}
