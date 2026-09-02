#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sqlite3.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/AllStruct.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DispMsg.h"
#include "../EVENT/MenuMsg.h"
#include "../PRINT/Print.h"
#include "../../FISC/NCCCfisc.h"
#include "../../CREDIT/Creditfunc.h"
#include "../../EMVSRC/EMVsrc.h"
#include "../../CTLS/CTLS.h"
#include "../FUNCTION/ECR.h"
#include "../../NCCC/NCCCsrc.h"
#include "Sqlite.h"
#include "Function.h"
#include "CDT.h"
#include "HDT.h"
#include "HDPT.h"
#include "CFGT.h"
#include "Card.h"

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))

TRACK_DATA	srGlobalTrackData;
DISPLAY_OBJECT  srDispObj;
extern  int     ginDebug;				/* Debug使用 extern */
extern  int     ginEventCode;
/*
Function	:inCARD_MSREvent
Date&Time	:2015/8/24 上午 1:56
Describe	:偵測磁條刷卡
*/
int inCARD_MSREvent(void)
{
	int     inRetVal;

        if (ginDebug == VS_TRUE)
        {
                //inLogPrintf(AT, "----------------------------------------");
                //inLogPrintf(AT, "inCARD_MSREvent()_START");
        }
	
	memset((char *)&srGlobalTrackData, 0x00, TRACK_DATA_SIZE);

        srGlobalTrackData.shTrack1Len = srGlobalTrackData.shTrack2Len = srGlobalTrackData.shTrack3Len = 128;
        inRetVal = CTOS_MSRRead((BYTE *)srGlobalTrackData.szTrack1, (unsigned short *)&srGlobalTrackData.shTrack1Len,
                                (BYTE *)srGlobalTrackData.szTrack2, (unsigned short *)&srGlobalTrackData.shTrack2Len,
                                (BYTE *)srGlobalTrackData.szTrack3, (unsigned short *)&srGlobalTrackData.shTrack3Len);

        if (inRetVal == d_MSR_NO_SWIPE )
        {
                /* 沒刷卡 */
                return(VS_ERROR);
        }
	else if (inRetVal == d_MSR_TRACK_ERROR || srGlobalTrackData.shTrack2Len == 0)
	{
		
		return (VS_SWIPE_ERROR);
	}
        else
        {
                if (ginDebug == VS_TRUE)
                {
                        //inLogPrintf(AT, "inCARD_MSREvent()_END");
                        //inLogPrintf(AT, "----------------------------------------");
                }

                return(VS_SUCCESS);
        }
}

/*
Function	:inCARD_Clean_MSR_Buffer
Date&Time	:2016/5/13 下午 6:13
Describe	:這隻函式是因為只要有過卡，就會記憶磁卡資料在裡面，若此時再插晶片卡則會跳出磁卡資料
*/
int inCARD_Clean_MSR_Buffer()
{
	char	szLastTrack1[128 + 1], szLastTrack2[128 + 1], szLastTrack3[128 + 1];
	short	shLastTrack1Len = 128, shLastTrack2Len = 128, shLastTrack3Len = 128;
	
	/* 清空抓磁條的資料，避免在不需刷卡的地方，刷卡後，再插入晶片卡，會出現磁卡的資料 */
	/* 只是清空用，不用initial */
	/* buffer只有在CTOS_MSRRead執行完之後才會清空，所以隨便放一個位置丟垃圾 */
	CTOS_MSRRead((unsigned char*)szLastTrack1, (unsigned short*)&shLastTrack1Len,
		     (unsigned char*)szLastTrack2, (unsigned short*)&shLastTrack2Len,
		     (unsigned char*)szLastTrack3, (unsigned short*)&shLastTrack3Len);
		
	return (VS_SUCCESS);
}

/*
Function        :inCARD_GetTrack123
Date&Time       :2016/11/8 上午 11:47
Describe        :
*/
int inCARD_GetTrack123(TRANSACTION_OBJECT *pobTran)
{
	int	i = 0;
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inCARD_GetTrack123()_START");
        }

        memset(pobTran->szTrack1, 0x00, sizeof(pobTran->szTrack1));
        memset(pobTran->szTrack2, 0x00, sizeof(pobTran->szTrack2));
        memset(pobTran->szTrack3, 0x00, sizeof(pobTran->szTrack3));

        /* 將讀到的Track123存到pobTran */
        pobTran->shTrack3Len = srGlobalTrackData.shTrack3Len;

	/* 因為Track1第0個byte為"%"，因此略過
	 * Track2組成<STX><Format code><PAN><^><NAME><^><Expiration date><Service code><Discretionary data><ETX><LRC>
	 * 這邊Track2不保留<STX><ETX><LRC>
	 * <STX>:";"
	 * <ETX>:"?"
	 * 詳細請看wiki ISO 7813
	 * <LRC>:1Byte的值，可用來檢查Track內的資料，目前不檢查，之後有空再做
	 * 下面這一行是舊寫法
	 *  memcpy(pobTran->szTrack1, srGlobalTrackData.szTrack1, srGlobalTrackData.shTrack1Len);
	 */
	
	if (srGlobalTrackData.shTrack1Len > 0)
	{
		
		/* 當遇到ETX或是整個Track1的長度都讀完就跳出 */
		for (i = 0; (1 + i) < srGlobalTrackData.shTrack1Len; i++)
		{
			
			if (srGlobalTrackData.szTrack1[1 + i] == '?')
				break;

			/* 因為Track2第0個byte為";"，因此略過 */
			memcpy(&pobTran->szTrack1[0 + i], &srGlobalTrackData.szTrack1[1 + i], 1);
		}
		
		pobTran->shTrack1Len = strlen(pobTran->szTrack1);
	}
	
        /* 因為Track2第0個byte為";"，因此略過
	 * Track2組成<STX><PAN><=><Expiration date><Service code><Discretionary data><ETX><LRC>
	 * 這邊Track2不保留<STX><ETX><LRC>
	 * <STX>:";"
	 * <ETX>:"?"
	 * <LRC>:1Byte的值，可用來檢查Track內的資料，目前不檢查，之後有空再做
	 * 詳細請看wiki ISO 7813
	 * 下面這一行是舊寫法
	 * memcpy(pobTran->szTrack2, &srGlobalTrackData.szTrack2[1], srGlobalTrackData.shTrack2Len); 
	 */
	
	if (srGlobalTrackData.shTrack2Len > 0)
	{
		
		/* 當遇到ETX或是整個Track2的長度都讀完就跳出 */
		for (i = 0; (1 + i) < srGlobalTrackData.shTrack2Len; i++)
		{
			
			if (srGlobalTrackData.szTrack2[1 + i] == '?')
				break;

			/* 因為Track2第0個byte為";"，因此略過 */
			memcpy(&pobTran->szTrack2[0 + i], &srGlobalTrackData.szTrack2[1 + i], 1);
		}
		
		pobTran->shTrack2Len = strlen(pobTran->szTrack2);
	}
	
        /* 有疑問，Track3第0個byte是否為";" */
        memcpy(pobTran->szTrack3, srGlobalTrackData.szTrack3, srGlobalTrackData.shTrack3Len);

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, pobTran->szTrack1);
                inLogPrintf(AT, pobTran->szTrack2);
		inLogPrintf(AT, srGlobalTrackData.szTrack2);
                inLogPrintf(AT, pobTran->szTrack3);
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inCARD_GetTrack123()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return(VS_SUCCESS);
}

/*
Function        :inCARD_unPackCard
Date&Time       :2016/11/8 上午 11:47
Describe        :
*/
int inCARD_unPackCard(TRANSACTION_OBJECT *pobTran)
{
        int		i = 0, j = 0;
	char		szTagVal[128] = {0};
	char		szCheckChar = 0x00;
	unsigned char 	uszValue[128] = {0};
	unsigned short  usTagLen = 0;
	

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inCARD_unPackCard()_START");
        }

	memset(pobTran->srBRec.szServiceCode, 0x00, sizeof(pobTran->srBRec.szServiceCode));
	memset(pobTran->srBRec.szPAN, 0x00, sizeof(pobTran->srBRec.szPAN));
	memset(pobTran->srBRec.szExpDate, 0x00, sizeof(pobTran->srBRec.szExpDate));
	memset(pobTran->srBRec.szCardHolder, 0x00, sizeof(pobTran->srBRec.szCardHolder));

        if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
        {
                /* 抓Track2 Data */
                usTagLen = sizeof(uszValue);
                memset(uszValue, 0x00, sizeof(uszValue));
                inEMV_Get_Tag_Value(0x57, &usTagLen, uszValue);
                memset(szTagVal, 0x00, sizeof(szTagVal));
                inFunc_BCD_to_ASCII(szTagVal, uszValue, usTagLen);

                /* 將Track2 Data存入pobTran供組電文使用 */
                memset(pobTran->szTrack2, 0x00, sizeof(pobTran->szTrack2));
		/* 從Tag57中抓的Track2最後一位若為F，要去掉 */
		/* For ADVT case 卡號結尾會有非數字字元 */
		szCheckChar = szTagVal[(usTagLen * 2) - 1];
		if (szCheckChar < '0' || szCheckChar > '9')
			pobTran->shTrack2Len = (usTagLen * 2) - 1;
		else
			pobTran->shTrack2Len = usTagLen * 2;
                memcpy(&pobTran->szTrack2[0], &szTagVal[0], pobTran->shTrack2Len);
        
                /* 取得PAN、ExpDate、szServiceCode SRART */
                for (i = 0; i < pobTran->shTrack2Len; i ++)
                {
                        if (pobTran->szTrack2[i] != 'D')
                                memcpy(&pobTran->srBRec.szPAN[i], &pobTran->szTrack2[i], 1);
                        else if (pobTran->szTrack2[i] == 'D')
                        {
                                i ++;
                                memcpy(&pobTran->srBRec.szExpDate[0], &pobTran->szTrack2[i], 4);
                                i += 4;
                                memcpy(&pobTran->srBRec.szServiceCode[0], &pobTran->szTrack2[i], 3);
                                break;
                        }
                }
        }
	else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
	{   
		/* 取得PAN、ExpDate、szServiceCode SRART */
		for (i = 0; i < pobTran->shTrack2Len; i ++)
		{
			if ((pobTran->szTrack2[i] != '=') && (pobTran->szTrack2[i] != 'D'))
				memcpy(&pobTran->srBRec.szPAN[i], &pobTran->szTrack2[i], 1);
			else if ((pobTran->szTrack2[i] == '=') || (pobTran->szTrack2[i] == 'D'))
			{
				i ++;
				memcpy(&pobTran->srBRec.szExpDate[0], &pobTran->szTrack2[i], 4);
				i += 4;
				memcpy(&pobTran->srBRec.szServiceCode[0], &pobTran->szTrack2[i], 3);
				break;
			}
		}
	}
        else if (pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
	{
                /* 抓Track2 Data */
                memset(szTagVal, 0x00, sizeof(szTagVal));
                inFunc_BCD_to_ASCII(szTagVal, pobTran->usz57_Track2, pobTran->in57_Track2Len);
                
                /* 將Track2 Data存入pobTran供組電文使用 */
                memset(pobTran->szTrack2, 0x00, sizeof(pobTran->szTrack2));
		/* 從Tag57中抓的Track2最後一位若為F，要去掉 */
		/* For ADVT case 卡號結尾會有非數字字元 */
		szCheckChar = szTagVal[(pobTran->in57_Track2Len * 2) - 1];
		if (szCheckChar < '0' || szCheckChar > '9')
			pobTran->shTrack2Len = (pobTran->in57_Track2Len * 2) - 1;
		else
			pobTran->shTrack2Len = pobTran->in57_Track2Len * 2;
                memcpy(&pobTran->szTrack2[0], &szTagVal[0], pobTran->shTrack2Len);
                
		/* 取得PAN、ExpDate、szServiceCode SRART */
		for (i = 0; i < pobTran->shTrack2Len; i ++)
		{
			if ((pobTran->szTrack2[i] != '=') && (pobTran->szTrack2[i] != 'D'))
				memcpy(&pobTran->srBRec.szPAN[i], &pobTran->szTrack2[i], 1);
			else if ((pobTran->szTrack2[i] == '=') || (pobTran->szTrack2[i] == 'D'))
			{
				i ++;
				memcpy(&pobTran->srBRec.szExpDate[0], &pobTran->szTrack2[i], 4);
				i += 4;
				memcpy(&pobTran->srBRec.szServiceCode[0], &pobTran->szTrack2[i], 3);
				break;
			}
		}
	}
        else
        {
                /* 取得PAN、ExpDate、szServiceCode SRART */
                for (i = 0; i < pobTran->shTrack2Len; i ++)
                {
                        if ((pobTran->szTrack2[i] != '=') && (pobTran->szTrack2[i] != 'D'))
                                memcpy(&pobTran->srBRec.szPAN[i], &pobTran->szTrack2[i], 1);
                        else if ((pobTran->szTrack2[i] == '=') || (pobTran->szTrack2[i] == 'D'))
                        {
                                i ++;
                                memcpy(&pobTran->srBRec.szExpDate[0], &pobTran->szTrack2[i], 4);
                                i += 4;
                                memcpy(&pobTran->srBRec.szServiceCode[0], &pobTran->szTrack2[i], 3);
                                break;
                        }
                }
        }

        /* 卡號長度不符規定 */
	if ((strlen(pobTran->srBRec.szPAN) > _PAN_SIZE_ ) || (strlen(pobTran->srBRec.szPAN) == 0))
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "卡號過長Len: %d", strlen(pobTran->srBRec.szPAN));
		}
                /* 卡號閱讀失敗(卡號過長) */
		pobTran->inErrorMsg = _ERROR_CODE_V3_PAN_SIZE_ERROR_;
		
		return (VS_ERROR);
	}
        /* 取得PAN、ExpDate、szServiceCode END */

        if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
        {
                /* 抓CARD Holder Name */
                usTagLen = sizeof(uszValue);
                memset(uszValue, 0x00, sizeof(uszValue));
                inEMV_Get_Tag_Value(0x5F20, &usTagLen, uszValue);
                strcpy(pobTran->srBRec.szCardHolder, (char *)uszValue);
        }
        else if (pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
	{
                if (strlen(pobTran->srBRec.szCardHolder) == 0)
                {
                        strncpy(pobTran->srBRec.szCardHolder, (char*)pobTran->srEMVRec.usz5F20_CardholderName, sizeof(pobTran->srBRec.szCardHolder));
                }
        }
        else
        {
                /* 取得Card Holder Name SRART */
                for (i = 0; i < pobTran->shTrack1Len; i ++)
                {
                        if (pobTran->szTrack1[i] == '^')
                        {
                                i ++;
                                for (j = 0; j < _MAX_TRACK1_NAME_SIZE_; j ++)
                                {
                                        memcpy(&pobTran->srBRec.szCardHolder[j], &pobTran->szTrack1[i ++], 1);
                                        if (pobTran->szTrack1[i] == '^')
                                                break;
                                }

                                break;
                        }
                }

        }

        /* 檢查card holder長度不符規定 */
	if (strlen(pobTran->srBRec.szCardHolder) > _MAX_TRACK1_NAME_SIZE_)
	{
		/* 名字閱讀失敗(持卡人姓名過長)*/
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_HOLDER_SIZE_);
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
	/* 取得Card Hoder Name END */

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, pobTran->srBRec.szPAN);
                inLogPrintf(AT, pobTran->srBRec.szExpDate);
                inLogPrintf(AT, pobTran->srBRec.szServiceCode);
                inLogPrintf(AT, pobTran->srBRec.szCardHolder);
        }
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inCARD_unPackCard()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return(VS_SUCCESS);
}

/*
Function        :inCARD_ValidTrack2_ExpDate
Date&Time       :
Describe        :
*/
int inCARD_ValidTrack2_ExpDate(TRANSACTION_OBJECT* pobTran)
{
        int	inCardYear, inCardMon;
	int	inSYSYear = 0, inSYSMon = 0;
	char	szTempDate[_DATE_SIZE_ + 1];
	char	szYYMMDD[6 + 1];
        char	szCustomerIndicator[3 + 1] = {0};
	char	szFuncEnable[2 + 1] = {0};
	char	szBIN_CHECK[2 + 1] = {0};
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inCARD_ValidTrack2_ExpDate()_START");
        }
	
	/* ATS Check 開啟時，必定不檢核card bin */
	/* 銀聯預先授權 + 人工輸入 無視開關設定 */
	if (pobTran->srBRec.inCode == _CUP_PRE_AUTH_ && pobTran->srBRec.uszManualBit == VS_TRUE)
	{
		memset(szBIN_CHECK, 0x00, sizeof(szBIN_CHECK));
		inGetBIN_CHECK(szBIN_CHECK);
		if (memcmp(szBIN_CHECK, "A", strlen("A")) == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "BIN CHECK 為A，不檢核有效期 ");
			}
			
			return (VS_SUCCESS);
		}
	}
	else
	{
		/* BIN_CHECK為A時，會設定CDT第一行為不檢核有效期，所以當兩個檢核同時存在時，可以省略 */
		
		
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetExpiredDateCheck(szFuncEnable);
		/* 檢核開關沒開，要跳過 */
		if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "ExpiredDateCheck未開，不檢核有效期 ");
			}
			
			return (VS_SUCCESS);
		}
	}
	
	/* 銀聯卡不檢核有效期 */
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		/* 銀聯預先授權 + 人工輸入 要檢核*/
		if (pobTran->srBRec.inCode == _CUP_PRE_AUTH_ && pobTran->srBRec.uszManualBit == VS_TRUE)
		{
			
		}
		else
		{
			return (VS_SUCCESS);
		}
	}
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	memset(szTempDate, 0x00, sizeof(szTempDate));
	memcpy(szTempDate, &pobTran->srBRec.szExpDate[2], 2);
	inCardMon = atoi(szTempDate);

        /* 月份不合法 */
	if ((inCardMon > _MAX_MONTHS_IN_YEAR_) || (inCardMon < _MIN_MONTHS_IN_YEAR_))
	{
                /* 顯示有效期錯誤 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_EXP_DATE_NOT_VALID_;
		
		return (VS_ERROR);
	}
        
        memset(szYYMMDD, 0x00, sizeof(szYYMMDD));
        memcpy(&szYYMMDD[0], &pobTran->srBRec.szDate[2], 6);

	memset(szTempDate, 0x00, sizeof(szTempDate));
	memcpy(&szTempDate[0], &szYYMMDD[0], 2);
	inSYSYear = atoi(szTempDate);

	memset(szTempDate, 0x00, sizeof(szTempDate));
	memcpy(&szTempDate[0], &szYYMMDD[2], 2);
	inSYSMon = atoi(szTempDate);

	memset(szTempDate, 0x00, sizeof(szTempDate));
        memcpy(&szTempDate[0], &pobTran->srBRec.szExpDate[0], 2);
	inCardYear = atoi(szTempDate);

        /* 檢查卡片是否過期 */
	if (inCardYear < inSYSYear )
	{
                if (ginEventCode == _SENSOR_EVENT_)
                {
                        /* Mirror Message */
                        if (pobTran->uszECRBit == VS_TRUE)
                        {
                                inECR_SendMirror(pobTran, _MIRROR_MSG_CTLS_ERROR_);
                        }
                }
                        
		/* 卡片過期 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, "");
		srDispMsgObj.inDispPic1YPosition = 0;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
                
                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)) 
                {
                        srDispMsgObj.inTimeout = _CUSTOMER_105_MCDONALDS_DISPLAY_TIMEOUT_;
                }
                else
                {
                        srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
                }
                
		strcpy(srDispMsgObj.szErrMsg1, "卡片過期");
		srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);
		
		return (VS_ERROR);
	}

        /* 檢查卡片是否過期 */
	if ( (inCardYear == inSYSYear) && (inCardMon < inSYSMon) )
	{
                if (ginEventCode == _SENSOR_EVENT_)
                {
                        /* Mirror Message */
                        if (pobTran->uszECRBit == VS_TRUE)
                        {
                                inECR_SendMirror(pobTran, _MIRROR_MSG_CTLS_ERROR_);
                        }
                }
                
                /* 卡片過期 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, "");
		srDispMsgObj.inDispPic1YPosition = 0;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		
                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))
                {
                        srDispMsgObj.inTimeout = _CUSTOMER_105_MCDONALDS_DISPLAY_TIMEOUT_;
                }
                else
                {
                        srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
                }
                
		strcpy(srDispMsgObj.szErrMsg1, "卡片過期");
		srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);
		
		return (VS_ERROR);
	}

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inCARD_ValidTrack2_ExpDate()_END");
        }

	return (VS_SUCCESS);
}

/*
Function        :inCARD_ValidTrack2_PAN
Date&Time       :2016/12/15 下午 3:43
Describe        :
*/
int inCARD_ValidTrack2_PAN(TRANSACTION_OBJECT * pobTran)
{
        int     i;
    	int     inSum, inWeight, inTmp = 0;
    	int     inLen;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inCARD_ValidTrack2_PAN()_START");
        }
	
    	inSum = 0;
    	inWeight = 2;

    	inLen = strlen(pobTran->srBRec.szPAN);

    	for (i = inLen - 2; i >= 0; i --)
    	{
        	inTmp = inWeight * (pobTran->srBRec.szPAN[i] - '0');
        	inSum += inTmp / 10;
        	inSum += inTmp % 10;
        	if (inWeight == 2)
            		inWeight = 1;
        	else
            		inWeight = 2;
    	}

    	inTmp = pobTran->srBRec.szPAN[inLen - 1] - '0';

        if ((inTmp + inSum) % 10 == 0)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "inCARD_ValidTrack2_PAN()_END");
                        inLogPrintf(AT, "----------------------------------------");
                }

                return(VS_SUCCESS);
        }
    	else
        {
		if (pobTran->uszInputTxnoBit == VS_TRUE)
		{
                        /* Mirror Message */
                        if (pobTran->uszECRBit == VS_TRUE)
                        {
                                inECR_SendMirror(pobTran, _MIRROR_MSG_CHECK_CARD_ERROR_);
                        }
                        
			/* 交易編號錯誤 請按清除鍵 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_TXNO_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			/* 顯示卡號錯誤 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CARD_NO_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}

        	return(VS_ERROR);
        }
}

/*
Function        :inCARD_ValidTrack2_HGPAN
Date&Time       :2017/5/17 下午 6:32
Describe        :
*/
int inCARD_ValidTrack2_HGPAN(TRANSACTION_OBJECT * pobTran)
{
        int     i;
    	int     inSum, inWeight, inTmp = 0;
    	int     inLen;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inCARD_ValidTrack2_HGPAN()_START");
        }
	
    	inSum = 0;
    	inWeight = 2;

    	inLen = strlen(pobTran->srBRec.szHGPAN);

    	for (i = inLen - 2; i >= 0; i --)
    	{
        	inTmp = inWeight * (pobTran->srBRec.szHGPAN[i] - '0');
        	inSum += inTmp / 10;
        	inSum += inTmp % 10;
        	if (inWeight == 2)
            		inWeight = 1;
        	else
            		inWeight = 2;
    	}

    	inTmp = pobTran->srBRec.szHGPAN[inLen - 1] - '0';

        if ((inTmp + inSum) % 10 == 0)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "inCARD_ValidTrack2_HGPAN()_END");
                        inLogPrintf(AT, "----------------------------------------");
                }

                return(VS_SUCCESS);
        }
    	else
        {
                /* 顯示卡號錯誤 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_CARD_NO_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
                inDISP_Msg_BMP(&srDispMsgObj);

        	return(VS_ERROR);
        }
}

/*
Function        :inCARD_ValidTrack2_UCard_PAN
Date&Time       :
Describe        :
*/
int inCARD_ValidTrack2_UCard_PAN(char *szCardNo)
{
        int	inWeight_for_ncc_card[10] = {7, 3, 5, 6, 7, 9, 8, 5, 9, 7};
	int	inTotal1, inTotal2, inTotal3, i;
        char    szUCardPAN[11 + 1], szKey;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inCARD_ValidTrack2_UCard_PAN()_START");
        }

	inTotal1 = 0 ;

        /* 去掉卡號4000 */
        memcpy(&szUCardPAN[0], &szCardNo[4], 11);
	for (i = 0; i < 10; i ++)
        {
		inTotal1 += (szUCardPAN[i] - '0' ) * inWeight_for_ncc_card[i];
	}

	inTotal2 = (inTotal1 / 100) * 5;
	inTotal1 %= 100 ;
	inTotal2 += (inTotal1 / 10) * 9;
	inTotal2 += (inTotal1 % 10) * 7;
	inTotal3 = inTotal2 % 10 + 3;
	if (((inTotal3 % 10 + (szUCardPAN[10] - '0')) % 10) == 0)
		return (VS_SUCCESS);
	else
        {
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                /* 顯示卡號錯誤 */
                inDISP_PutGraphic(_ERR_CARD_NO_, 0, _COORDINATE_Y_LINE_8_4_);

                while (1)
                {
                        szKey = uszKBD_GetKey(180);
                        if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
        			return(VS_ERROR);
        	}
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inCARD_ValidTrack2_UCard_PAN()_END");
        }
}

/*
Function        :inCARD_ValidTrack2_UCard_PAN_MenuKeyIn
Date&Time       :2017/1/20 下午 3:23
Describe        :
*/
int inCARD_ValidTrack2_UCard_PAN_MenuKeyIn(char *szCardNo)
{
        int	inWeight_for_ncc_card[10] = {7, 3, 5, 6, 7, 9, 8, 5, 9, 7};
	int	inTotal1, inTotal2, inTotal3, i;
        char    szKey;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inCARD_ValidTrack2_UCard_PAN()_START");
        }

	inTotal1 = 0 ;

	for (i = 0; i < 10; i ++)
        {
		inTotal1 += (szCardNo[i] - '0' ) * inWeight_for_ncc_card[i];
	}

	inTotal2 = (inTotal1 / 100) * 5;
	inTotal1 %= 100 ;
	inTotal2 += (inTotal1 / 10) * 9;
	inTotal2 += (inTotal1 % 10) * 7;
	inTotal3 = inTotal2 % 10 + 3;
	if (((inTotal3 % 10 + (szCardNo[10] - '0')) % 10) == 0)
		return (VS_SUCCESS);
	else
        {
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                /* 顯示卡號錯誤 */
                inDISP_PutGraphic(_ERR_CARD_NO_, 0, _COORDINATE_Y_LINE_8_4_);

                while (1)
                {
                        szKey = uszKBD_GetKey(180);
                        if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
        			return(VS_ERROR);
        	}
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inCARD_ValidTrack2_UCard_PAN()_END");
        }
}

/*
Function        :inCARD_GetBin
Date&Time       :2015/9/11 下午 1:22
Describe        :Get CARD Bin
*/
int inCARD_GetBin(TRANSACTION_OBJECT *pobTran)
{
	int	inCardLen, inMinPANLength, inMaxPANLength;
        char	szMinPANLength[2+1], szMaxPANLength[2+1];
        

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inCARD_GetBin()_START");
        }

        /* 判斷CDT 及 Load HDT */
        if (inCARD_LoadGetCDTIndex(pobTran) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* 檢查卡號長度 */
        inCardLen = strlen(pobTran->srBRec.szPAN);
	
        memset(szMinPANLength, 0x00, sizeof(szMinPANLength));
        inGetMinPANLength(szMinPANLength);
        inMinPANLength = atoi(szMinPANLength);
	
	memset(szMinPANLength, 0x00, sizeof(szMinPANLength));
        inGetMaxPANLength(szMaxPANLength);
        inMaxPANLength = atoi(szMaxPANLength);
	
        if (inCardLen < inMinPANLength || inCardLen > inMaxPANLength)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "CardLength Error!!");
                }
		
		/* 顯示卡號長度錯誤 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_PAN_TOO_LONG_ERROR_;

                return (VS_ERROR);
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inCARD_GetBin()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inCARD_LoadGetCDTIndex
Date&Time       :2015/9/11 下午 1:22
Describe        :判斷 CDT 及 Load HDT
*/
int inCARD_LoadGetCDTIndex(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	int	inHDTIndex;
        int	i = 0, inPANLen, inLowBinLen, inHighBinLen;
        char	szLowBinRange[11 + 1], szHighBinRange[11 + 1];
        char	szHostCDTIndex[2 + 1], szHostEnable[1 + 1], szCardLabel[20 + 1];
	char	szTemplate[42 + 1];
        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];
	char	szBinCheck[2 + 1];

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inCARD_LoadGetCDTIndex()_START");
        }

        inPANLen = strlen(pobTran->srBRec.szPAN);
        if (inPANLen < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "PANLength: %d < 0", inPANLen);
                        inLogPrintf(AT, szErrorMsg);
                }
		
		/* 顯示卡號長度錯誤 */
                pobTran->inErrorMsg = _ERROR_CODE_V3_PAN_TOO_LONG_ERROR_;

                return (VS_ERROR);
        }

        do
        {
                /* 按順序load每一個卡別的卡號範圍 */
                if (inLoadCDTRec(i) < 0)
                {
			/* debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "卡號無法對應CDT中的任一Record");
				inLogPrintf(AT, szErrorMsg);
			}
                        
                        /* Mirror Message */
                        if (pobTran->uszECRBit == VS_TRUE)
                        {
                                inECR_SendMirror(pobTran, _MIRROR_MSG_WRONG_CARD_ERROR_);
                        }
		
			/* 請依正確卡別操作 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_PLEASE_FOLLOW_RIGHT_OPT_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
			
                        return (VS_ERROR);
                }

                memset(szLowBinRange, 0x00, sizeof(szLowBinRange));
                memset(szHighBinRange, 0x00, sizeof(szHighBinRange));
                inGetLowBinRange(szLowBinRange);
                inLowBinLen = strlen (szLowBinRange);
                inGetHighBinRange(szHighBinRange);
                inHighBinLen = strlen (szHighBinRange);
		
		/* 如果是銀聯交易 */
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetCardLabel(szTemplate);
			if (!memcmp(szTemplate, _CARD_TYPE_CUP_, strlen(_CARD_TYPE_CUP_)))
			{
				/* 把CDT rec index 存起來 */
				pobTran->srBRec.inCDTIndex = i;
				/* 讀HDT */
				memset(szHostCDTIndex, 0x00,  sizeof(szHostCDTIndex));
				inGetHostCDTIndex (szHostCDTIndex);

				/* 把HDT rec index 存起來  ,HDT rec 是從0開始讀  */
				inFunc_Find_Specific_HDTindex_ByCDTIndex(pobTran->srBRec.inHDTIndex, szHostCDTIndex, &inHDTIndex);
				pobTran->srBRec.inHDTIndex = inHDTIndex;
				if (ginDebug == VS_TRUE)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate,"HDT:%d, CDT:%d", pobTran->srBRec.inHDTIndex, pobTran->srBRec.inCDTIndex);
					inLogPrintf(AT, szTemplate);
				}

				if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) < 0)
				{
					return (VS_ERROR);
				}
				else
				{
					/* 判斷HOST是否開啟  */
					memset(szHostEnable, 0x00, sizeof(szHostEnable));
					inGetHostEnable(szHostEnable);
					if (szHostEnable[0] != 'Y')
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "Host not open.");
						}
						return (VS_ERROR);
					}
					break;
				}
			}
		}
                else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
                {
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetCardLabel(szTemplate);
			if (!memcmp(szTemplate, _CARD_TYPE_SMARTPAY_, strlen(_CARD_TYPE_SMARTPAY_)))
			{
				/* 把CDT rec index 存起來 */
				pobTran->srBRec.inCDTIndex = i;
				/* 讀HDT */
				memset(szHostCDTIndex, 0x00,  sizeof(szHostCDTIndex));
				inGetHostCDTIndex(szHostCDTIndex);

				/* 把HDT rec index 存起來  ,HDT rec 是從0開始讀  */
				inFunc_Find_Specific_HDTindex_ByCDTIndex(pobTran->srBRec.inHDTIndex, szHostCDTIndex, &inHDTIndex);
				pobTran->srBRec.inHDTIndex = inHDTIndex;
				if (ginDebug == VS_TRUE)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate,"HDT:%d, CDT:%d", pobTran->srBRec.inHDTIndex, pobTran->srBRec.inCDTIndex);
					inLogPrintf(AT, szTemplate);
				}

				if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) < 0)
				{
					return (VS_ERROR);
				}
				else
				{
					/* 判斷HOST是否開啟  */
					memset(szHostEnable, 0x00, sizeof(szHostEnable));
					inGetHostEnable(szHostEnable);
                                        
					if (szHostEnable[0] != 'Y')
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "Host not open.");
						}
						return (VS_ERROR);
					}
                                        
					break;
				}
			}
                }
		else
		{
			/* 判斷卡號 位在哪個Bin Range(如果卡號大於該卡別最低卡號，而且小於最高卡號) */
			if (strncmp(pobTran->srBRec.szPAN, szLowBinRange, min(inPANLen, inLowBinLen)) >= 0	&&
			    strncmp(pobTran->srBRec.szPAN, szHighBinRange, min(inPANLen, inHighBinLen)) <= 0)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				/* 把CDT rec index 存起來 */
				pobTran->srBRec.inCDTIndex = i;
				/* 讀HDT */
				memset(szHostCDTIndex, 0x00,  sizeof(szHostCDTIndex));
				inGetHostCDTIndex (szHostCDTIndex);
				/* 把HDT rec index 存起來  ,HDT rec 是從0開始讀  */
				inFunc_Find_Specific_HDTindex_ByCDTIndex(pobTran->srBRec.inHDTIndex, szHostCDTIndex, &inHDTIndex);
				pobTran->srBRec.inHDTIndex = inHDTIndex;
				if (ginDebug == VS_TRUE)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate,"HDT:%d, CDT:%d", pobTran->srBRec.inHDTIndex, pobTran->srBRec.inCDTIndex);
					inLogPrintf(AT, szTemplate);
				}

				if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) < 0)
				{
					return (VS_ERROR);
				}
				else
				{
					/* 判斷HOST是否開啟  */
					memset(szHostEnable, 0x00, sizeof(szHostEnable));
					inGetHostEnable(szHostEnable);
					if (szHostEnable[0] != 'Y')
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "Host not open.");
						}
						return (VS_ERROR);
					}
					break;
				}
			}
			
		}

                i++;
        } while (1);
	
        inRetVal = inFunc_Get_HDPT_General_Data(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* 【需求單 - 108097】Card BIN由ATS查核 add by LingHsiung 2020/2/3 下午 5:19 */
	memset(szBinCheck, 0x00, sizeof(szBinCheck));
	inGetBIN_CHECK(szBinCheck);
	/* 由ATS主機檢核的話晶片和感應卡先塞CardLabel 後面自訂義cardbin不存CardLabel */
	if ((memcmp(szBinCheck, "A", strlen("A")) == 0) && 
	    (pobTran->srBRec.inChipStatus == _EMV_CARD_ || pobTran->srBRec.uszContactlessBit == VS_TRUE || pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE))
		return (VS_SUCCESS);

        memset(szCardLabel, 0x00, sizeof(szCardLabel));
        inGetCardLabel(szCardLabel);
        memcpy(&pobTran->srBRec.szCardLabel[0], &szCardLabel[0], strlen(szCardLabel));
	
	if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_TAKA_EMPLOYEE_, strlen(_CARD_TYPE_TAKA_EMPLOYEE_)) ||
	    !memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_TAKA_T_DAYEH_, strlen(_CARD_TYPE_TAKA_T_DAYEH_)) ||
	    !memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_TAKA_T_CARD_, strlen(_CARD_TYPE_TAKA_T_CARD_)))
	{
		pobTran->srBRec.uszTakaTransBit = VS_TRUE;
	}

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inCARD_LoadGetCDTIndex()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inCARD_GetBin_HG
Date&Time       :2017/4/5 上午 11:46
Describe        :Get CARD Bin
*/
int inCARD_GetBin_HG(TRANSACTION_OBJECT *pobTran)
{
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inCARD_GetBin_HG()_START");
        }

        if (memcmp(pobTran->srBRec.szHGPAN, "9552", strlen("9552")) == 0)
	{
		memcpy(pobTran->srBRec.szHGCardLabel, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_));
	}
	else
	{
		inCARD_LoadGetCDTIndex_HG(pobTran);
	}

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inCARD_GetBin_HG()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inCARD_LoadGetCDTIndex_HG
Date&Time       :2017/4/5 下午 3:19
Describe        :判斷 CDT
*/
int inCARD_LoadGetCDTIndex_HG(TRANSACTION_OBJECT *pobTran)
{
        int	i = 0, inPANLen, inLowBinLen, inHighBinLen;
        char	szLowBinRange[11 + 1], szHighBinRange[11 + 1];
        char	szCardLabel[20 + 1];
        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inCARD_LoadGetCDTIndex_HG()_START");
        }

        inPANLen = strlen(pobTran->srBRec.szHGPAN);
        if (inPANLen < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "PANLength: %d < 0", inPANLen);
                        inLogPrintf(AT, szErrorMsg);
                }
		
		/* 顯示卡號錯誤 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_CARD_NO_);
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

        do
        {
                /* 按順序load每一個卡別的卡號範圍 */
                if (inLoadCDTRec(i) < 0)
                {
			/* debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "卡號無法對應CDT中的任一Record");
				inLogPrintf(AT, szErrorMsg);
			}
		
                        return (VS_SUCCESS);
                }

                memset(szLowBinRange, 0x00, sizeof(szLowBinRange));
                memset(szHighBinRange, 0x00, sizeof(szHighBinRange));
                inGetLowBinRange(szLowBinRange);
                inLowBinLen = strlen (szLowBinRange);
                inGetHighBinRange(szHighBinRange);
                inHighBinLen = strlen (szHighBinRange);
		
		/* 如果是銀聯交易 */
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			memcpy(pobTran->srBRec.szHGCardLabel, _CARD_TYPE_CUP_, strlen(_CARD_TYPE_CUP_));
			
			break;
		}
                else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
                {
			memcpy(pobTran->srBRec.szHGCardLabel, _CARD_TYPE_SMARTPAY_, strlen(_CARD_TYPE_SMARTPAY_));
			
			break;
                }
		else
		{
			/* 判斷卡號 位在哪個Bin Range(如果卡號大於該卡別最低卡號，而且小於最高卡號) */
			if (strncmp(pobTran->srBRec.szHGPAN, szLowBinRange, min(inPANLen, inLowBinLen)) >=0	&&
			    strncmp(pobTran->srBRec.szHGPAN, szHighBinRange, min(inPANLen, inHighBinLen)) <=0)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				memset(szCardLabel, 0x00, sizeof(szCardLabel));
				inGetCardLabel(szCardLabel);
				memcpy(&pobTran->srBRec.szHGCardLabel[0], &szCardLabel[0], strlen(szCardLabel));
				break;
			}
			
		}

                i++;
        } while (1);
	
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inCARD_LoadGetCDTIndex_HG()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inCARD_ExpDateEncryptAndDecrypt
Date&Time       :2016/9/30 上午 10:09
Describe        :有效期轉換檢查碼
*/
int inCARD_ExpDateEncryptAndDecrypt(TRANSACTION_OBJECT *pobTran, char *szInBuf, char *szOutBuf, int inMethod)
{
	int	inLen = 0;
	int	i;
	int	inSum = 0, inKeyNo = 0;
	char	szCheckNo[5 + 1], szExpDateTmp[5 + 1];
	char	szFinalPAN[20 + 1];

	memset(szCheckNo, 0x00, sizeof(szCheckNo));
	memset(szExpDateTmp, 0x00, sizeof(szExpDateTmp));
	memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
	
	/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
	memcpy(szFinalPAN, pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));
	
	inLen = strlen(szFinalPAN);

	if (inMethod == _EXP_ENCRYPT_)
	{
		/* MMYY */
		memcpy(szExpDateTmp, &pobTran->srBRec.szExpDate[2], 2);
		memcpy(&szExpDateTmp[2], pobTran->srBRec.szExpDate, 2);

		/* 第五碼 固定卡號第 10 碼 */
		szExpDateTmp[4] = szFinalPAN[9]; 
	}
	/* Decrypt */
	else
	{
		memcpy(szExpDateTmp, szInBuf, 5);
	}

	/* 計算出 Key No. 卡號每個數字加起來 */
	/* 若總和不是一位數，則把各個位數相加，重複至只剩一位數 */
	do
	{
		inSum = 0;
		/* 將各個位數相加 */
		for (i = 0; i < inLen; i++)
		{
			inSum += szFinalPAN[i] - '0';
		}
		
		memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
		
		sprintf(szFinalPAN, "%d", inSum);
		inLen = strlen(szFinalPAN);
	}while (inLen > 1);
	
	/* 最後剩的一位數，就是keyNo */
	inKeyNo = inSum;
	
	if (inMethod == _EXP_ENCRYPT_)
	{
		for (i = 0; i < 5; i ++)
		{
			/* 偶數位 */
			if (i % 2 == 0)
			{
				/* 那位數 + KeyNo */
				inSum = (szExpDateTmp[i] - '0') + inKeyNo;
				/* 若大於1位數，則只取個位數 */
				if (inSum >= 10)
					inSum = inSum % 10;

				szCheckNo[i] = inSum + '0';
			}
			/* 奇數位 */
			else
			{
				/* 若小於KeyNo */
				if ((szExpDateTmp[i] - '0') < inKeyNo)
				{
					/* 那位數 +10 - KeyNo */
					inSum = ((szExpDateTmp[i] - '0') + 10) - inKeyNo;
					szCheckNo[i] = inSum + '0';
				}
				else
				{
					/* 那位數 - KeyNo */
					inSum = (szExpDateTmp[i] - '0') - inKeyNo;
					szCheckNo[i] = inSum + '0';
				}
			}
		}
	}
	/* Decrypt */
	else
	{
		for (i = 0; i < 5; i ++)
		{
			if (i % 2 == 0)
			{
				if ((szExpDateTmp[i] - '0') < inKeyNo)
				{
					inSum = ((szExpDateTmp[i] - '0') + 10) - inKeyNo;
					szCheckNo[i] = inSum + '0';
				}
				else
				{
					inSum = (szExpDateTmp[i] - '0') - inKeyNo;
					szCheckNo[i] = inSum + '0';
				}
			}
			else
			{
				inSum = (szExpDateTmp[i] - '0') + inKeyNo;
				if (inSum >= 10)
					inSum = inSum % 10;

				szCheckNo[i] = inSum + '0';
			}
		}
	}

	memset(szOutBuf, 0x00, 5);
	memcpy(szOutBuf, szCheckNo, strlen(szCheckNo));

	return (VS_SUCCESS);
}

/*
Function        :inCARD_Generate_Special_Card
Date&Time       :2017/1/20 下午 3:46
Describe        :用來組那些奇奇怪怪的卡號
*/
int inCARD_Generate_Special_Card(char* szPAN)
{
	char	szTempPAN[19 + 1];
	
	memset(szTempPAN, 0x00, sizeof(szTempPAN));
	memcpy(szTempPAN, szPAN, strlen(szPAN));
	
	if (strlen(szTempPAN) == 11)
	{
		memset(szPAN, 0x00, strlen(szPAN));
		strcpy(szPAN, "4000"); /* 組 U_CARD 卡號 */
		strcat(szPAN, szTempPAN);
		szPAN[15] = inCARD_UCardModGen(szTempPAN) + 48;
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inCARD_UCardModGen
Date&Time       :2017/1/20 下午 3:53
Describe        :產生U CARD 檢查碼用
*/
int inCARD_UCardModGen(char *szAcct)
{
	int inLoop;
	int inAuthVal[] = {2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2};
	int inRet, inSum=0;

	for (inLoop = 0; inLoop < strlen(szAcct); inLoop ++)
	{
		inRet = (szAcct[inLoop] - 48) * inAuthVal[inLoop];
		if (inRet < 10)
			inSum = inSum + inRet;
		else
		{
			inSum = inSum + inRet - 10;
			inSum++;
		}
	}

	return (10 - (inSum % 10));
}
