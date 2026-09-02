#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <ctos_qrcode.h>
#include <sqlite3.h>
#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/AllStruct.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DispMsg.h"
#include "../SOURCE/DISPLAY/DisTouch.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/FUNCTION/File.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/ASMC.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "NCCCsrc.h"
#include "NCCCloyalty.h"

extern	int	ginDebug;

/* 需注意V3使用unicode，長度，中文字皆需要轉 */

/*
Function        :inNCCC_Loyalty_ASM_Flag
Date&Time       :2017/2/13 下午 3:01
Describe        :是否支援優惠功能電文
*/
int inNCCC_Loyalty_ASM_Flag()
{
	int		inRetVal = VS_ERROR;
	char		szNCCCFESMode[2 + 1];
	char		szFlag[2 + 1];
	char		szStartDate[8 + 1], szEndDate[8 + 1];
	char		szNowDate[8 + 1];
	char		szDebugMsg[100 + 1];
	char		szCFESMode[2 + 1] = {0};
	RTC_NEXSYS	srRTC;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_Loyalty_ASM_Flag() START !");
	}
	
	memset(szNCCCFESMode, 0x00, sizeof(szNCCCFESMode));
	inGetNCCCFESMode(szNCCCFESMode);
	memset(szCFESMode, 0x00, sizeof(szCFESMode));
	inGetCloud_MFES(szCFESMode);
	/* 優惠兌換只支援MFES和ATS */
	/* (需求單 - 107367)優惠查核改由ATS判斷 除MFES外，其餘交由主機判斷 by Russell 2019/7/2 下午 4:27 */
	if ((memcmp(szNCCCFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0	&&
	     memcmp(szCFESMode, "Y", strlen("Y")) == 0)						|| 
	     memcmp(szNCCCFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0	||
	     memcmp(szNCCCFESMode, _NCCC_05_ATS_MODE_, strlen(_NCCC_05_ATS_MODE_)) == 0)
	{
		return(VS_SUCCESS);
	}
	else if (memcmp(szNCCCFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0	&&
		 memcmp(szCFESMode, "Y", strlen("Y")) != 0)
	{
		/* 繼續判斷日期及開關 */
	}
	else
	{
		return(VS_ERROR);
	}
	
	if (inLoadASMCRec(0) < 0) /* 優惠平台參數檔【ASMConfig.txt】 */
	{
		return (VS_ERROR);
	}
	

	/* 判斷是否支援優惠平台(含詢問電文) */
	memset(szFlag, 0x00, sizeof(szFlag));
	inGetASMFlag(szFlag);
	if (memcmp(szFlag, "Y", strlen("Y")) == 0)
	{
		/* 取得活動起迄日 */
		/* 活動起日 */
		memset(szStartDate, 0x00, sizeof(szStartDate));
		inGetASMStartDate(szStartDate);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ASMStartDate = %s", szStartDate);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 活動迄日 */
		memset(szEndDate, 0x00, sizeof(szEndDate));
		inGetASMEndDate(szEndDate);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ASMEndDate = %s", szEndDate);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 現在日期 */
		memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
		inFunc_GetSystemDateAndTime(&srRTC);
		memset(szNowDate, 0x00, sizeof(szNowDate));
		sprintf(szNowDate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NowDate = %s", szNowDate);
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 用太陽日比較 */
		if (inFunc_SunDay_Sum_Check_In_Range(szNowDate, szStartDate, szEndDate) != VS_SUCCESS)
		{
			/* 不在範圍內 */
			inRetVal = VS_ERROR;
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}

	}
	else
	{
		inRetVal = VS_ERROR;
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Loyalty_ASM_Flag() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (inRetVal);
}

/*
Function        :inNCCC_Loyalty_CreditCardFlag
Date&Time       :2017/10/30 上午 11:22
Describe        :判斷是否支援過卡兌換
*/
int inNCCC_Loyalty_CreditCardFlag(int inCode)
{
	int		inRetVal = VS_ERROR;
	char		szNCCCFESMode[2 + 1];
	char		szFlag[2 + 1];
	char		szStartDate[8 + 1], szEndDate[8 + 1];
	char		szNowDate[8 + 1];
	char		szDebugMsg[100 + 1];
	char		szCFESMode[2 + 1] = {0};
	char		szCustomIndicator[3 + 1] = {0};
	RTC_NEXSYS	srRTC;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_Loyalty_CreditCardFlag() START !");
	}
	
	memset(szNCCCFESMode, 0x00, sizeof(szNCCCFESMode));
	inGetNCCCFESMode(szNCCCFESMode);
	memset(szCFESMode, 0x00, sizeof(szCFESMode));
	inGetCloud_MFES(szCFESMode);
	memset(szCustomIndicator, 0x00, sizeof(szCustomIndicator));
	inGetCustomIndicator(szCustomIndicator);
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/5/30 下午 1:30
	 * 客製化075、103不支援優惠兌換 */
	if (!memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
	{
		return(VS_ERROR);
	}
	else
	{
		/* 優惠兌換只支援MFES和ATS */
		/* (需求單 - 107367)優惠查核改由ATS判斷，除MFES外，其餘交由主機判斷 by Russell 2019/7/2 下午 4:27 */
		if ((memcmp(szNCCCFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0	&&
		     memcmp(szCFESMode, "Y", strlen("Y")) == 0)						|| 
		     memcmp(szNCCCFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0	||
		     memcmp(szNCCCFESMode, _NCCC_05_ATS_MODE_, strlen(_NCCC_05_ATS_MODE_)) == 0)
		{
			return(VS_SUCCESS);
		}
		else if (memcmp(szNCCCFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0	&&
			 memcmp(szCFESMode, "Y", strlen("Y")) != 0)
		{
			/* 繼續判斷日期及開關 */
		}
		else
		{
			return(VS_ERROR);
		}
	}
	
	if (inLoadASMCRec(0) < 0) /* 優惠平台參數檔【ASMConfig.txt】 */
	{
		return (VS_ERROR);
	}
	

	/* 判斷是否支援過卡兌換 */
	memset(szFlag, 0x00, sizeof(szFlag));
	inGetCreditCardFlag(szFlag);
	if (memcmp(szFlag, "Y", strlen("Y")) == 0)
	{
		/* 最終結果預設為False */
		inRetVal = VS_ERROR;

		/* 取得活動起迄日 */
		/* 活動起日 */
		memset(szStartDate, 0x00, sizeof(szStartDate));
		inGetCreditCardStartDate(szStartDate);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CreditCardStartDate = %s", szStartDate);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 活動迄日 */
		memset(szEndDate, 0x00, sizeof(szEndDate));
		inGetCreditCardEndDate(szEndDate);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CreditCardEndDate = %s", szEndDate);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 現在日期 */
		memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
		inFunc_GetSystemDateAndTime(&srRTC);
		memset(szNowDate, 0x00, sizeof(szNowDate));
		sprintf(szNowDate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NowDate = %s", szNowDate);
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 用太陽日比較 */
		if (inFunc_SunDay_Sum_Check_In_Range(szNowDate, szStartDate, szEndDate) != VS_SUCCESS)
		{
			/* 不在範圍內 */
			inRetVal = VS_ERROR;
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}
		
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "ASMC_CreditCardFlag() not supported");
		}
		
		inRetVal = VS_ERROR;
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Loyalty_CreditCardFlag() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inNCCC_Loyalty_BarCodeFlag
Date&Time       :2017/10/30 上午 11:22
Describe        :判斷是否支援一維條碼兌換
*/
int inNCCC_Loyalty_BarCodeFlag(int inCode)
{
	int		inRetVal = VS_ERROR;
	char		szNCCCFESMode[2 + 1];
	char		szFlag[2 + 1];
	char		szStartDate[8 + 1], szEndDate[8 + 1];
	char		szNowDate[8 + 1];
	char		szDebugMsg[100 + 1];
	char		szCFESMode[2 + 1] = {0};
	char		szCustomIndicator[3 + 1] = {0};
	RTC_NEXSYS	srRTC;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_Loyalty_BarCodeFlag() START !");
	}
	
	memset(szNCCCFESMode, 0x00, sizeof(szNCCCFESMode));
	inGetNCCCFESMode(szNCCCFESMode);
	memset(szCFESMode, 0x00, sizeof(szCFESMode));
	inGetCloud_MFES(szCFESMode);
	memset(szCustomIndicator, 0x00, sizeof(szCustomIndicator));
	inGetCustomIndicator(szCustomIndicator);
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/5/30 下午 1:30
	 * 客製化075、103不支援優惠兌換 */
	if (!memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
	{
		return(VS_ERROR);
	}
	else
	{
		/* 優惠兌換只支援MFES和ATS */
		/* (需求單 - 107367)優惠查核改由ATS判斷，除MFES外，其餘交由主機判斷 by Russell 2019/7/2 下午 4:27 */
		if ((memcmp(szNCCCFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0	&&
		     memcmp(szCFESMode, "Y", strlen("Y")) == 0)						|| 
		     memcmp(szNCCCFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0	||
		     memcmp(szNCCCFESMode, _NCCC_05_ATS_MODE_, strlen(_NCCC_05_ATS_MODE_)) == 0)
		{
			return(VS_SUCCESS);
		}
		else if (memcmp(szNCCCFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0	&&
			 memcmp(szCFESMode, "Y", strlen("Y")) != 0)
		{
			/* 繼續判斷日期及開關 */
		}
		else
		{
			return(VS_ERROR);
		}
	}
	
	if (inLoadASMCRec(0) < 0) /* 優惠平台參數檔【ASMConfig.txt】 */
	{
		return (VS_ERROR);
	}
	
	/* 判斷是否支援條碼兌換 */
	memset(szFlag, 0x00, sizeof(szFlag));
	inGetBarCodeFlag(szFlag);
	if (memcmp(szFlag, "Y", strlen("Y")) == 0)
	{
		/* 最終結果預設為False */
		inRetVal = VS_ERROR;

		/* 取得活動起迄日 */
		/* 活動起日 */
		memset(szStartDate, 0x00, sizeof(szStartDate));
		inGetBarCodeStartDate(szStartDate);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "BarCodeStartDate = %s", szStartDate);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 活動迄日 */
		memset(szEndDate, 0x00, sizeof(szEndDate));
		inGetBarCodeEndDate(szEndDate);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "BarCodeEndDate = %s", szEndDate);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 現在日期 */
		memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
		inFunc_GetSystemDateAndTime(&srRTC);
		memset(szNowDate, 0x00, sizeof(szNowDate));
		sprintf(szNowDate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NowDate = %s", szNowDate);
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 用太陽日比較 */
		if (inFunc_SunDay_Sum_Check_In_Range(szNowDate, szStartDate, szEndDate) != VS_SUCCESS)
		{
			/* 不在範圍內 */
			inRetVal = VS_ERROR;
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "ASMC_BarCodeFlag() not supported");
		}
		
		inRetVal = VS_ERROR;
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Loyalty_BarCodeFlag() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inNCCC_Loyalty_VoidRedeemFlag
Date&Time       :2017/10/30 上午 11:51
Describe        :判斷是否支援兌換取消
*/
int inNCCC_Loyalty_VoidRedeemFlag(int inCode)
{
	int		inRetVal = VS_ERROR;
	char		szNCCCFESMode[2 + 1];
	char		szFlag[2 + 1];
	char		szStartDate[8 + 1], szEndDate[8 + 1];
	char		szNowDate[8 + 1];
	char		szDebugMsg[100 + 1];
	char		szCFESMode[2 + 1] = {0};
	char		szCustomIndicator[3 + 1] = {0};
	RTC_NEXSYS	srRTC;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_Loyalty_VoidRedeemFlag() START !");
	}
	
	memset(szNCCCFESMode, 0x00, sizeof(szNCCCFESMode));
	inGetNCCCFESMode(szNCCCFESMode);
	memset(szCFESMode, 0x00, sizeof(szCFESMode));
	inGetCloud_MFES(szCFESMode);
	memset(szCustomIndicator, 0x00, sizeof(szCustomIndicator));
	inGetCustomIndicator(szCustomIndicator);
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/5/30 下午 1:30
	 * 客製化075、103不支援優惠兌換 */
	if (!memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
	{
		return(VS_ERROR);
	}
	else
	{
		/* 優惠兌換只支援MFES和ATS */
		/* (需求單 - 107367)優惠查核改由ATS判斷，除MFES外，其餘交由主機判斷 by Russell 2019/7/2 下午 4:27 */
		if ((memcmp(szNCCCFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0	&&
		     memcmp(szCFESMode, "Y", strlen("Y")) == 0)						|| 
		     memcmp(szNCCCFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0	||
		     memcmp(szNCCCFESMode, _NCCC_05_ATS_MODE_, strlen(_NCCC_05_ATS_MODE_)) == 0)
		{
			return(VS_SUCCESS);
		}
		else if (memcmp(szNCCCFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0	&&
			 memcmp(szCFESMode, "Y", strlen("Y")) != 0)
		{
			/* 繼續判斷日期及開關 */
		}
		else
		{
			return(VS_ERROR);
		}
	}
	
	if (inLoadASMCRec(0) < 0) /* 優惠平台參數檔【ASMConfig.txt】 */
	{
		return (VS_ERROR);
	}
	
	/* 判斷是否支援兌換取消 */
	memset(szFlag, 0x00, sizeof(szFlag));
	inGetVoidRedeemFlag(szFlag);
	if (memcmp(szFlag, "Y", strlen("Y")) == 0)
	{
		/* 最終結果預設為False */
		inRetVal = VS_ERROR;

		/* 取得活動起迄日 */
		/* 活動起日 */
		memset(szStartDate, 0x00, sizeof(szStartDate));
		inGetVoidRedeemStartDate(szStartDate);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "VoidRedeemStartDate = %s", szStartDate);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 活動迄日 */
		memset(szEndDate, 0x00, sizeof(szEndDate));
		inGetVoidRedeemEndDate(szEndDate);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "VoidRedeemEndDate = %s", szEndDate);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 現在日期 */
		memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
		inFunc_GetSystemDateAndTime(&srRTC);
		memset(szNowDate, 0x00, sizeof(szNowDate));
		sprintf(szNowDate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NowDate = %s", szNowDate);
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 用太陽日比較 */
		if (inFunc_SunDay_Sum_Check_In_Range(szNowDate, szStartDate, szEndDate) != VS_SUCCESS)
		{
			/* 不在範圍內 */
			inRetVal = VS_ERROR;
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "ASMC_VoidRedeemFlag() not supported");
		}
		
		inRetVal = VS_ERROR;
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Loyalty_VoidRedeemFlag() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inNCCC_Loyalty_RefundFlag
Date&Time       :2017/11/7 下午 4:25
Describe        :判斷是否支援兌換退貨
*/
int inNCCC_Loyalty_RefundFlag(int inCode)
{
	int		inRetVal = VS_ERROR;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_Loyalty_RefundFlag() START !");
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "ASMC_VoidRedeemFlag() not supported");
	}

	inRetVal = VS_ERROR;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Loyalty_RefundFlag() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        inNCCC_Loyalty_Save_Reward_Data
Date&Time       :2017/2/14 上午 9:34
Describe        :儲存優惠兌換資訊
*/
int inNCCC_Loyalty_Save_Reward_Data(unsigned long ulDataLen, char *szFileData, char* szSaveFileName)
{
	unsigned char	uszFileNameBak[16 + 1];
	unsigned char	uszFileNameSave[16 + 1];
	unsigned long	ulHandle;
	
	memset(uszFileNameBak, 0x00, sizeof(uszFileNameBak));
	strcpy((char*)uszFileNameBak, _REWARD_BACKUP_FILE_NAME_);
	
	/* 先刪檔 */
	if (inFILE_Check_Exist(uszFileNameBak) == VS_SUCCESS)
	{
		inFILE_Delete(uszFileNameBak);
	}
	
	if (inFILE_Create(&ulHandle, uszFileNameBak) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_Func_Save_Reward_Data Create Fail");
		}
	}
	
	if (inFILE_Write(&ulHandle, (unsigned char*)szFileData, ulDataLen) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_Func_Save_Reward_Data Write Data Fail");
		}
		
		return (VS_ERROR);
	}
	
	if (inFILE_Close(&ulHandle) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_Func_Save_Reward_Data Close Fail");
		}
		
		return (VS_ERROR);
	}
	
	memset(uszFileNameSave, 0x00, sizeof(uszFileNameSave));
	strcpy((char*)uszFileNameSave, szSaveFileName);
	/* 有檔就刪檔才替換 */
	if (inFILE_Check_Exist(uszFileNameSave) == VS_SUCCESS)
	{
		inFILE_Delete(uszFileNameSave);
		inFILE_Rename(uszFileNameBak, uszFileNameSave);
	}
	else
	{
		inFILE_Rename(uszFileNameBak, uszFileNameSave);
	}
	
	return (VS_SUCCESS);
}

/*
Function        inNCCC_Loyalty_Save_Reward_Data_L5
Date&Time       :2017/5/16 下午 6:54
Describe        :儲存優惠兌換資訊
*/
int inNCCC_Loyalty_Save_Reward_Data_L5(unsigned long ulDataLen, char *szFileData, char* szSaveFileName)
{
	long		lnGzipDataLen = 0;	/* 算出未解壓縮Gzip長度 */
	long		lnASMDataLen = 0;
	unsigned char	uszASMData[_SIZE_ASM_DATA_ * _AWARD_ACTIVITY_MAX_NUM_];
	unsigned char	uszFileNameBak[16 + 1];
	unsigned char	uszFileNameSave[16 + 1];
	unsigned long	ulHandle = 0;
	unsigned long	ulASMDataHandle =0;
	
	/* 處理gzip部份 */
	/* 預算長度 (2+ 2+ 1+ 22+ 1+ 2+ 1+ 60) = 91 */
	/* 算出未解壓縮Gzip長度 原長度減前面91 */
	lnGzipDataLen = ulDataLen - 91;
	/* 有可能有沒優惠內容的情況 */
	if (lnGzipDataLen > 0)
	{
		if (inNCCC_Loyalty_Process_GzipData((unsigned long)lnGzipDataLen, szFileData + 91) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Process GZIP ASMDATA Fail");
			}
			return (VS_ERROR);
		}
		
		lnASMDataLen = lnFILE_GetSize(&ulASMDataHandle, (unsigned char*)_L5_ASMDATA_FILE_NAME_);
			
		memset(uszASMData, 0x00, sizeof(uszASMData));
		if (inNCCC_Loyalty_Get_ASMDATA(uszASMData) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Get ASMDATA Fail");
			}
			return (VS_ERROR);
		}
		
	}
	
	memset(uszFileNameBak, 0x00, sizeof(uszFileNameBak));
	memcpy(uszFileNameBak, _REWARD_BACKUP_FILE_NAME_, strlen(_REWARD_BACKUP_FILE_NAME_));
	
	/* 先刪檔 */
	if (inFILE_Check_Exist(uszFileNameBak) == VS_SUCCESS)
	{
		inFILE_Delete(uszFileNameBak);
	}
	
	if (inFILE_Create(&ulHandle, uszFileNameBak) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_Func_Save_Reward_Data Create Fail");
		}
	}
	
	/* 寫前面91 bytes */
	if (inFILE_Write(&ulHandle, (unsigned char*)szFileData, ulDataLen) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_Func_Save_Reward_Data Write 91 Data Fail");
		}
		
		return (VS_ERROR);
	}
	
	/* 寫後面的優惠兌換內容 */
	if (lnGzipDataLen > 0)
	{
		if (inFILE_Write(&ulHandle, (unsigned char*)uszASMData, lnASMDataLen) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inNCCC_Func_Save_Reward_Data Write Last Data Fail");
			}

			return (VS_ERROR);
		}
	}
	
	if (inFILE_Close(&ulHandle) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_Func_Save_Reward_Data Close Fail");
		}
		
		return (VS_ERROR);
	}
	
	memset(uszFileNameSave, 0x00, sizeof(uszFileNameSave));
	strcpy((char*)uszFileNameSave, szSaveFileName);
	/* 有檔就刪檔才替換 */
	if (inFILE_Check_Exist(uszFileNameSave) == VS_SUCCESS)
	{
		inFILE_Delete(uszFileNameSave);
		inFILE_Rename(uszFileNameBak, uszFileNameSave);
	}
	else
	{
		inFILE_Rename(uszFileNameBak, uszFileNameSave);
	}
	
	return (VS_SUCCESS);
}

/*
Function        inNCCC_Loyalty_Process_GzipData
Date&Time       :2017/9/14 下午 3:24
Describe        :處理Gzip檔案，裡面的資料名稱為ASMDATA.dat
*/
int inNCCC_Loyalty_Process_GzipData(unsigned long ulDataLen, char *szFileData)
{
	unsigned char	uszFileNameBak[16 + 1];
	unsigned long	ulHandle = 0;
	
	memset(uszFileNameBak, 0x00, sizeof(uszFileNameBak));
	memcpy(uszFileNameBak, _L5_ASMDATA_GZIP_FILE_NAME_, strlen(_L5_ASMDATA_GZIP_FILE_NAME_));
	
	/* 先刪檔 */
	if (inFILE_Check_Exist(uszFileNameBak) == VS_SUCCESS)
	{
		inFILE_Delete(uszFileNameBak);
	}
	
	if (inFILE_Create(&ulHandle, uszFileNameBak) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_Func_Save_Reward_Data Create Fail");
		}
	}
	
	if (inFILE_Write(&ulHandle, (unsigned char*)szFileData, ulDataLen) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_Func_Save_Reward_Data Write Data Fail");
		}
		
		return (VS_ERROR);
	}
	
	if (inFILE_Close(&ulHandle) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_Func_Save_Reward_Data Close Fail");
		}
		
		return (VS_ERROR);
	}
	
	
	/* 存成.gzip後，開始解壓縮 */
	if (inFunc_Data_GUnZip("", _L5_ASMDATA_GZIP_FILE_NAME_, _FS_DATA_PATH_) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "L5 unzip Fail");
		}
		
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        inNCCC_Loyalty_Get_ASMDATA
Date&Time       :2017/9/14 下午 4:33
Describe        :儲存優惠兌換資訊
*/
int inNCCC_Loyalty_Get_ASMDATA(unsigned char *uszReadData)
{
	int		i = 0;
	long		lnASMDataLen = 0;	/* 解壓縮後的長度 */
	long            lnReadLength;		/* 剩餘讀取長度 */
	unsigned long	ulFile_Handle = 0;
	/*
         * open CDT.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_L5_ASMDATA_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

	
        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnASMDataLen = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_L5_ASMDATA_FILE_NAME_);

        if (lnASMDataLen == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnASMDataLen;

                for (i = 0;; ++i)
                {
                        /* 剩餘長度大於或等於1024 */
                        if (lnReadLength >= 1024)
                        {
                                if (inFILE_Read(&ulFile_Handle, &uszReadData[1024 * i], 1024) == VS_SUCCESS)
                                {
                                        /* 一次讀1024 */
                                        lnReadLength -= 1024;

                                        /* 當剩餘長度剛好為1024，會剛好讀完 */
                                        if (lnReadLength == 0)
                                                break;
                                }
                                /* 讀失敗時 */
                                else
                                {
                                        /* Close檔案 */
                                        inFILE_Close(&ulFile_Handle);

                                        return (VS_ERROR);
                                }
                        }
                        /* 剩餘長度小於1024 */
                        else if (lnReadLength < 1024)
                        {
                                /* 就只讀剩餘長度 */
                                if (inFILE_Read(&ulFile_Handle, &uszReadData[1024*i], lnReadLength) == VS_SUCCESS)
                                {
                                        break;
                                }
                                /* 讀失敗時 */
                                else
                                {
                                        /* Close檔案 */
                                        inFILE_Close(&ulFile_Handle);

                                        return (VS_ERROR);
                                }
                        }
                }/* end for loop */
        }
        /* seek不成功時 */
        else
        {
                /* 關檔並回傳 */
                inFILE_Close(&ulFile_Handle);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }
	
	return (VS_SUCCESS);
}

/*
Function        inNCCC_Loyalty_Read_Reward_Data
Date&Time       :2017/2/14 上午 9:34
Describe        :讀取優惠兌換資訊，ulBufferSize讀BufferSize進來，超過會回傳錯誤
*/
int inNCCC_Loyalty_Read_Reward_Data(unsigned long *ulBufferSize, char *szFileData, char* szFileName)
{
	long		lnFileSize;
	unsigned char	uszFileNameSave[16 + 1];
	unsigned long	ulHandle;
	
	memset(uszFileNameSave, 0x00, sizeof(uszFileNameSave));
	memcpy(uszFileNameSave, szFileName, strlen(szFileName));
	
	lnFileSize = lnFILE_GetSize(&ulHandle, uszFileNameSave);
	if (lnFileSize >= *ulBufferSize)
	{
		return (VS_ERROR);
	}
	else
	{
		*ulBufferSize = lnFileSize;
		
		if (inFILE_OpenReadOnly(&ulHandle, uszFileNameSave) != VS_SUCCESS)
			return (VS_ERROR);
		
		if (inFILE_Read(&ulHandle, (unsigned char*)szFileData, lnFileSize) != VS_SUCCESS)
		{
			/* 不論成功或失敗都要close */
			inFILE_Close(&ulHandle);
			
			return (VS_ERROR);
		}
		
		/* 不論成功或失敗都要close */
		inFILE_Close(&ulHandle);
	}
	
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Loyalty_Data_Format
Date&Time       :2017/2/14 下午 2:18
Describe        :放到結構中，重點：電文送的是Big5，因V3使用Linux，所以需要轉為UTF8
*/
int inNCCC_Loyalty_Data_Format(LOYALTY_L1L2L3_OBJECT *srLoyaltyObj, int inDataLen, char *szData)
{
	int	i = 0;
	int	inCnt = 0;
	char	szBig5[300 + 1];
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_Loyalty_Data_Format() START !");
	}
	
	memset(srLoyaltyObj, 0x00, sizeof(LOYALTY_L1L2L3_OBJECT));
	
	/* Table ID */
	memcpy(srLoyaltyObj->szAwardName, &szData[inCnt], 2);
	inCnt += 2;
	/* SubTotalLen */
	memcpy(srLoyaltyObj->szSubTotalLen, &szData[inCnt], 2);
	inCnt += 2;
	
	/* 以上長度91Bytes(內含table ID SubtotalLen) */
	/* 開始分是L1、L2、L3，若是L1則填到srL1DATA，其他兩個為空，以此類推 */
	if (memcmp(srLoyaltyObj->szAwardName, "L1", strlen("L1")) == 0)
	{
		/* 列印優惠或廣告資訊之個數(如果是L3此欄位為兌換方式) */
		memcpy(srLoyaltyObj->srL1DATA.szPrintRewardNum, &szData[inCnt], 1);
		inCnt += 1;

		/* 優惠序號(Award S/N) */
		memcpy(srLoyaltyObj->srL1DATA.szAwardSN, &szData[inCnt], 22);
		inCnt += 22;

		/* 是否列印補充資訊。‘0’=不列印，’1’=列印 */
		memcpy(srLoyaltyObj->srL1DATA.szSupInfFlag, &szData[inCnt], 1);
		inCnt += 1;

		/* 補充資訊實際長度 */
		memcpy(srLoyaltyObj->srL1DATA.szSupInfLen, &szData[inCnt], 2);
		inCnt += 2;

		/* 補充資訊內容列印位置 */
		memcpy(srLoyaltyObj->srL1DATA.szSupInfLocation, &szData[inCnt], 1);
		inCnt += 1;

		/* 補充資訊內容 */
		memset(szBig5, 0x00, sizeof(szBig5));
		memcpy(szBig5, &szData[inCnt], atoi(srLoyaltyObj->srL1DATA.szSupInfLen));
		inFunc_Big5toUTF8(srLoyaltyObj->srL1DATA.szSupInfContent, szBig5);
		inCnt += 60;
		/* 這裡因應轉UTF8，也修正長度 */
		sprintf(srLoyaltyObj->srL1DATA.szSupInfLen, "%02d", strlen(srLoyaltyObj->srL1DATA.szSupInfContent));
		
		/* 防呆 */
		if (atoi(srLoyaltyObj->srL1DATA.szPrintRewardNum) < 0)
		{
			return (VS_ERROR);
		}
		
		for (i = 0; i < atoi(srLoyaltyObj->srL1DATA.szPrintRewardNum); i++)
		{
			/* 全部解析完畢 */
			if (inCnt == inDataLen)
			{
				break;
			}
			else if (inCnt > inDataLen)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "解析資料長度錯誤");
					inLogPrintf(AT, szDebugMsg);
				}
				return (VS_ERROR);
			}
			
			/* 是否列印優惠活動之標題 */
			memcpy(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].szTitleFlag, &szData[inCnt], 1);
			inCnt += 1;

			/* 優惠活動之標題長度。(最大長度限制50 Bytes，右靠左補零) */
			memcpy(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].szTitleLen, &szData[inCnt], 2);
			inCnt += 2;

			/* 優惠活動之標題內容。(端末機列印時需自動置中) */
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, &szData[inCnt], atoi(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].szTitleLen));
			inFunc_Big5toUTF8(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].szTitleContent, szBig5);
			inCnt += 50;
			/* 這裡因應轉UTF8，也修正長度 */
			sprintf(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].szTitleLen, "%02d", strlen(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].szTitleContent));

			/* 是否列印一維條碼(一) */
			memcpy(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].sz39Code1Flag, &szData[inCnt], 1);
			inCnt += 1;

			/* 一維條碼(一)長度。(最大長度限制20 Bytes，右靠左補空白) */
			memcpy(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].sz39Code1Len, &szData[inCnt], 2);
			inCnt += 2;

			/* 一維條碼(一) 補足空白至20 Bytes。 */
			memcpy(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].sz39Code1Content, &szData[inCnt], atoi(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].sz39Code1Len));
			inCnt += 20;

			/* 是否列印一維條碼(二) */
			memcpy(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].sz39Code2Flag, &szData[inCnt], 1);
			inCnt += 1;

			/* 一維條碼(二)長度。(最大長度限制20 Bytes，右靠左補空白) */
			memcpy(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].sz39Code2Len, &szData[inCnt], 2);
			inCnt += 2;

			/* 一維條碼(二) 補足空白至20 Bytes。 */
			memcpy(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].sz39Code2Content, &szData[inCnt], atoi(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].sz39Code2Len));
			inCnt += 20;

			/* 保留欄位 */
			inCnt += 1;

			/* 是否列印優惠活動之內容 1 byte */
			memcpy(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].szContentFlag, &szData[inCnt], 1);
			inCnt += 1;

			/* 優惠活動之內容長度。(最大長度限制200 Bytes，左靠右補空白) */
			memcpy(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].szContentLen, &szData[inCnt], 3);
			inCnt += 3;

			/* 優惠活動之內容 */
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, &szData[inCnt], atoi(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].szContentLen));
			inFunc_Big5toUTF8(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].szContent, szBig5);
			inCnt += 200;
			/* 這裡因應轉UTF8，也修正長度 */
			sprintf(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].szContentLen, "%02d", strlen(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].szContent));
			
			/* 檢查碼(Checksum) */
			memcpy(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].szChecksum, &szData[inCnt], 1);
			inCnt += 1;
		}
	
	}
	else if (memcmp(srLoyaltyObj->szAwardName, "L2", strlen("L2")) == 0)
	{
		/* 列印優惠或廣告資訊之個數(如果是L3此欄位為兌換方式) */
		memcpy(srLoyaltyObj->srL2DATA.szPrintRewardNum, &szData[inCnt], 1);
		inCnt += 1;

		/* 優惠序號(Award S/N) */
		memcpy(srLoyaltyObj->srL2DATA.szAwardSN, &szData[inCnt], 22);
		inCnt += 22;

		/* 是否列印補充資訊。‘0’=不列印，’1’=列印 */
		memcpy(srLoyaltyObj->srL2DATA.szSupInfFlag, &szData[inCnt], 1);
		inCnt += 1;

		/* 補充資訊實際長度 */
		memcpy(srLoyaltyObj->srL2DATA.szSupInfLen, &szData[inCnt], 2);
		inCnt += 2;

		/* 補充資訊內容列印位置 */
		memcpy(srLoyaltyObj->srL2DATA.szSupInfLocation, &szData[inCnt], 1);
		inCnt += 1;

		/* 補充資訊內容 */
		memset(szBig5, 0x00, sizeof(szBig5));
		memcpy(szBig5, &szData[inCnt], atoi(srLoyaltyObj->srL2DATA.szSupInfLen));
		inFunc_Big5toUTF8(srLoyaltyObj->srL2DATA.szSupInfContent, szBig5);
		inCnt += 60;
		/* 這裡因應轉UTF8，也修正長度 */
		sprintf(srLoyaltyObj->srL2DATA.szSupInfLen, "%02d", strlen(srLoyaltyObj->srL2DATA.szSupInfContent));
		
		/* 列印(優惠活動||累計訊息)及廣告資訊之個數。 註3.	本欄位值=’1’是指僅有一個，但仍需判斷第一段與第二段電文。*/
		if (atoi(srLoyaltyObj->srL2DATA.szPrintRewardNum) <= 0)
		{
			return (VS_ERROR);
		}
		
		for (i = 0; ; i++)
		{
			/* 全部解析完畢 */
			if (inCnt == inDataLen)
			{
				break;
			}
			else if (inCnt > inDataLen)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "解析資料長度錯誤");
					inLogPrintf(AT, szDebugMsg);
				}
				return (VS_ERROR);
			}
		
			/* 是否列印(優惠活動||累計訊息)之標題。 ‘0’=不列印，’1’=列印 */
			memcpy(srLoyaltyObj->srL2DATA.srL2_AWARD_DATA.szTitleFlag, &szData[inCnt], 1);
			inCnt += 1;

			/* (優惠活動||累計訊息)之標題長度。(右靠左補零) */
			memcpy(srLoyaltyObj->srL2DATA.srL2_AWARD_DATA.szTitleLen, &szData[inCnt], 2);
			inCnt += 2;

			/* (優惠活動||累計訊息)之標題內容。(端末機列印時需自動置中，左靠右補空白) */
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, &szData[inCnt], atoi(srLoyaltyObj->srL2DATA.srL2_AWARD_DATA.szTitleLen));
			inFunc_Big5toUTF8(srLoyaltyObj->srL2DATA.srL2_AWARD_DATA.szTitleContent, szBig5);
			inCnt += 50;
			/* 這裡因應轉UTF8，也修正長度 */
			sprintf(srLoyaltyObj->srL2DATA.srL2_AWARD_DATA.szTitleLen, "%02d", strlen(srLoyaltyObj->srL2DATA.srL2_AWARD_DATA.szTitleContent));

			/* 是否列印一維條碼(一)。’1’=列印，’0’=不列印 註：若列印累計訊息，則不須列印條碼。 */
			memcpy(srLoyaltyObj->srL2DATA.srL2_AWARD_DATA.sz39Code1Flag, &szData[inCnt], 1);
			inCnt += 1;

			/* 一維條碼(一)長度。(右靠左補零) */
			memcpy(srLoyaltyObj->srL2DATA.srL2_AWARD_DATA.sz39Code1Len, &szData[inCnt], 2);
			inCnt += 2;

			/* 一維條碼(一)。(左靠右補空白) */
			memcpy(srLoyaltyObj->srL2DATA.srL2_AWARD_DATA.sz39Code1Content, &szData[inCnt], atoi(srLoyaltyObj->srL2DATA.srL2_AWARD_DATA.sz39Code1Len));
			inCnt += 20;

			/* 是否列印一維條碼(二)。’1’=列印，’0’=不列印 註：若列印累計訊息，則不須列印條碼。*/
			memcpy(srLoyaltyObj->srL2DATA.srL2_AWARD_DATA.sz39Code2Flag, &szData[inCnt], 1);
			inCnt += 1;

			/* 一維條碼(二)長度。(右靠左補零) */
			memcpy(srLoyaltyObj->srL2DATA.srL2_AWARD_DATA.sz39Code2Len, &szData[inCnt], 2);
			inCnt += 2;

			/* 一維條碼(二)。(左靠右補空白) */
			memcpy(srLoyaltyObj->srL2DATA.srL2_AWARD_DATA.sz39Code2Content, &szData[inCnt], atoi(srLoyaltyObj->srL2DATA.srL2_AWARD_DATA.sz39Code2Len));
			inCnt += 20;

			/* 保留欄位(預設空白) */
			inCnt += 1;

			/* 是否列印(優惠活動||累計訊息)之內容。’1’=列印，’0’=不列印 */
			memcpy(srLoyaltyObj->srL2DATA.srL2_AWARD_DATA.szContentFlag, &szData[inCnt], 1);
			inCnt += 1;

			/* (優惠活動||累計訊息)之內容長度。(右靠左補零) */
			memcpy(srLoyaltyObj->srL2DATA.srL2_AWARD_DATA.szContentLen, &szData[inCnt], 3);
			inCnt += 3;

			/* (優惠活動||累計訊息)之內容。(左靠右補空白)(活動內容可支援英數字、中文、換行。需補足空白至200 Bytes。) */
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, &szData[inCnt], atoi(srLoyaltyObj->srL2DATA.srL2_AWARD_DATA.szContentLen));
			inFunc_Big5toUTF8(srLoyaltyObj->srL2DATA.srL2_AWARD_DATA.szContent, szBig5);
			inCnt += 200;
			/* 這裡因應轉UTF8，也修正長度 */
			sprintf(srLoyaltyObj->srL2DATA.srL2_AWARD_DATA.szContentLen, "%02d", strlen(srLoyaltyObj->srL2DATA.srL2_AWARD_DATA.szContent));

			/* 檢查碼(Checksum) */
			memcpy(srLoyaltyObj->srL2DATA.srL2_AWARD_DATA.szChecksum, &szData[inCnt], 1);
			inCnt += 1;

			/* 是否列印廣告資訊之標題。‘0’=不列印，’1’=列印 */
			memcpy(srLoyaltyObj->srL2DATA.srL2_AD_DATA.szADTitleFlag, &szData[inCnt], 1);
			inCnt += 1;

			/* 廣告資訊之標題長度。(右靠左補零) */
			memcpy(srLoyaltyObj->srL2DATA.srL2_AD_DATA.szADTitleLen, &szData[inCnt], 2);
			inCnt += 2;

			/* 廣告資訊之標題內容。(端末機列印時需自動置中，左靠右補空白) */
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, &szData[inCnt], atoi(srLoyaltyObj->srL2DATA.srL2_AD_DATA.szADTitleLen));
			inFunc_Big5toUTF8(srLoyaltyObj->srL2DATA.srL2_AD_DATA.szADTitleContent, szBig5);
			inCnt += 50;
			/* 這裡因應轉UTF8，也修正長度 */
			sprintf(srLoyaltyObj->srL2DATA.srL2_AD_DATA.szADTitleLen, "%02d", strlen(srLoyaltyObj->srL2DATA.srL2_AD_DATA.szADTitleContent));

			/* 是否列印廣告資訊。’1’=列印，’0’=不列印 */
			memcpy(srLoyaltyObj->srL2DATA.srL2_AD_DATA.szADFlag, &szData[inCnt], 1);
			inCnt += 1;

			/* 廣告資訊長度。(右靠左補零) */
			memcpy(srLoyaltyObj->srL2DATA.srL2_AD_DATA.szADLen, &szData[inCnt], 3);
			inCnt += 3;

			/* 廣告資訊之內容。(左靠右補空白) */
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, &szData[inCnt], atoi(srLoyaltyObj->srL2DATA.srL2_AD_DATA.szADLen));
			inFunc_Big5toUTF8(srLoyaltyObj->srL2DATA.srL2_AD_DATA.szADContent, szBig5);
			inCnt += 150;
			/* 這裡因應轉UTF8，也修正長度 */
			sprintf(srLoyaltyObj->srL2DATA.srL2_AD_DATA.szADLen, "%02d", strlen(srLoyaltyObj->srL2DATA.srL2_AD_DATA.szADContent));

			/* 是否列印URL資訊之QR Code。’1’=列印，’0’=不列印 註：將URL資訊內容轉印出QR Code。 */
			memcpy(srLoyaltyObj->srL2DATA.srL2_AD_DATA.szQRCodeFlag, &szData[inCnt], 1);
			inCnt += 1;

			/* URL資訊長度。(右靠左補零) */
			memcpy(srLoyaltyObj->srL2DATA.srL2_AD_DATA.szQRCodeLen, &szData[inCnt], 3);
			inCnt += 3;

			/* URL資訊之內容。(本欄位以QR Code列印，左靠右補空白) */
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, &szData[inCnt], atoi(srLoyaltyObj->srL2DATA.srL2_AD_DATA.szQRCodeLen));
			inFunc_Big5toUTF8(srLoyaltyObj->srL2DATA.srL2_AD_DATA.szQRCodeContent, szBig5);
			inCnt += 100;
			/* 這裡因應轉UTF8，也修正長度 */
			sprintf(srLoyaltyObj->srL2DATA.srL2_AD_DATA.szQRCodeLen, "%02d", strlen(srLoyaltyObj->srL2DATA.srL2_AD_DATA.szQRCodeContent));

			/* 檢查碼(Checksum) */
			memcpy(srLoyaltyObj->srL2DATA.srL2_AD_DATA.szChecksum, &szData[inCnt], 1);
			inCnt += 1;
		}
		
	}
	else if (memcmp(srLoyaltyObj->szAwardName, "L3", strlen("L3")) == 0)
	{
		/* 如果是L3此欄位為兌換方式 */
		memcpy(srLoyaltyObj->srL3DATA.szRewardWay, &szData[inCnt], 1);
		inCnt += 1;

		/* 優惠序號(Award S/N) */
		memcpy(srLoyaltyObj->srL3DATA.szAwardSN, &szData[inCnt], 22);
		inCnt += 22;

		/* 是否列印補充資訊。‘0’=不列印，’1’=列印 */
		memcpy(srLoyaltyObj->srL3DATA.szSupInfFlag, &szData[inCnt], 1);
		inCnt += 1;

		/* 補充資訊實際長度 */
		memcpy(srLoyaltyObj->srL3DATA.szSupInfLen, &szData[inCnt], 2);
		inCnt += 2;

		/* 補充資訊內容 */
		memset(szBig5, 0x00, sizeof(szBig5));
		memcpy(szBig5, &szData[inCnt], atoi(srLoyaltyObj->srL3DATA.szSupInfLen));
		inFunc_Big5toUTF8(srLoyaltyObj->srL3DATA.szSupInfContent, szBig5);
		inCnt += 20;
		/* 這裡因應轉UTF8，也修正長度 */
		sprintf(srLoyaltyObj->srL3DATA.szSupInfLen, "%02d", strlen(srLoyaltyObj->srL3DATA.szSupInfContent));
		
		/* 是否上傳一維條碼(一)。’1’=上傳，’0’=不上傳 */
		memcpy(srLoyaltyObj->srL3DATA.srL3_AWARD_DATA.sz39Code1Flag, &szData[inCnt], 1);
		inCnt += 1;

		/* 一維條碼(一)長度。(右靠左補零) */
		memcpy(srLoyaltyObj->srL3DATA.srL3_AWARD_DATA.sz39Code1Len, &szData[inCnt], 2);
		inCnt += 2;

		/* 一維條碼(一)。(左靠右補空白) */
		memcpy(srLoyaltyObj->srL3DATA.srL3_AWARD_DATA.sz39Code1Content, &szData[inCnt], atoi(srLoyaltyObj->srL3DATA.srL3_AWARD_DATA.sz39Code1Len));
		inCnt += 20;

		/* 是否上傳一維條碼(二)。’1’=上傳，’0’=不上傳 */
		memcpy(srLoyaltyObj->srL3DATA.srL3_AWARD_DATA.sz39Code2Flag, &szData[inCnt], 1);
		inCnt += 1;

		/* 一維條碼(二)長度。(右靠左補零) */
		memcpy(srLoyaltyObj->srL3DATA.srL3_AWARD_DATA.sz39Code2Len, &szData[inCnt], 2);
		inCnt += 2;

		/* 一維條碼(二)。(左靠右補空白) */
		memcpy(srLoyaltyObj->srL3DATA.srL3_AWARD_DATA.sz39Code2Content, &szData[inCnt], atoi(srLoyaltyObj->srL3DATA.srL3_AWARD_DATA.sz39Code2Len));
		inCnt += 20;

		for (i = 0; i < _AWARD_CANCELLATION_MAX_NUM_; i++)
		{
			/* 全部解析完畢 */
			if (inCnt == inDataLen)
			{
				break;
			}
			else if (inCnt > inDataLen)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "解析資料長度錯誤");
					inLogPrintf(AT, szDebugMsg);
				}
				return (VS_ERROR);
			}
			
			/* 是否列印兌換核銷資訊(一)。’1’=列印，’0’=不列印 */
			memcpy(srLoyaltyObj->srL3DATA.srL3_CANCEL_DATA[i].szCancelFlag, &szData[inCnt], 1);
			inCnt += 1;

			/* 兌換核銷資訊(一)長度。(右靠左補零) */
			memcpy(srLoyaltyObj->srL3DATA.srL3_CANCEL_DATA[i].szCancelLen, &szData[inCnt], 3);
			inCnt += 3;

			/* 兌換核銷資訊(一)之內容。(左靠右補空白)(內容可支援英數字、中文、換行。需補足空白至100 Bytes。) */
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, &szData[inCnt], atoi(srLoyaltyObj->srL3DATA.srL3_CANCEL_DATA[i].szCancelLen));
			inFunc_Big5toUTF8(srLoyaltyObj->srL3DATA.srL3_CANCEL_DATA[i].szCancelContent, szBig5);
			inCnt += 100;
			/* 這裡因應轉UTF8，也修正長度 */
			sprintf(srLoyaltyObj->srL3DATA.srL3_CANCEL_DATA[i].szCancelLen, "%02d", strlen(srLoyaltyObj->srL3DATA.srL3_CANCEL_DATA[i].szCancelContent));
		}
		/* 檢查碼(Checksum) */
		memcpy(srLoyaltyObj->srL3DATA.szChecksum, &szData[inCnt], 1);
		inCnt += 1;
		
	}
	/* 目前L5與L1相同，直接套用L1的 僅差在補充資訊列印位置 */
	else if (memcmp(srLoyaltyObj->szAwardName, "L5", strlen("L5")) == 0)
	{
		/* 列印優惠或廣告資訊之個數(如果是L3此欄位為兌換方式) */
		memcpy(srLoyaltyObj->srL1DATA.szPrintRewardNum, &szData[inCnt], 1);
		inCnt += 1;

		/* 優惠序號(Award S/N) */
		memcpy(srLoyaltyObj->srL1DATA.szAwardSN, &szData[inCnt], 22);
		inCnt += 22;

		/* 是否列印補充資訊。‘0’=不列印，’1’=列印 */
		memcpy(srLoyaltyObj->srL1DATA.szSupInfFlag, &szData[inCnt], 1);
		inCnt += 1;

		/* 補充資訊實際長度 */
		memcpy(srLoyaltyObj->srL1DATA.szSupInfLen, &szData[inCnt], 2);
		inCnt += 2;

		/* 補充資訊內容列印位置 */
		memcpy(srLoyaltyObj->srL1DATA.szSupInfLocation, &szData[inCnt], 1);
		inCnt += 1;

		/* 補充資訊內容 */
		memset(szBig5, 0x00, sizeof(szBig5));
		memcpy(szBig5, &szData[inCnt], atoi(srLoyaltyObj->srL1DATA.szSupInfLen));
		inFunc_Big5toUTF8(srLoyaltyObj->srL1DATA.szSupInfContent, szBig5);
		inCnt += 60;
		/* 這裡因應轉UTF8，也修正長度 */
		sprintf(srLoyaltyObj->srL1DATA.szSupInfLen, "%02d", strlen(srLoyaltyObj->srL1DATA.szSupInfContent));
		
		if (atoi(srLoyaltyObj->srL1DATA.szPrintRewardNum) < 0)
		{
			return (VS_ERROR);
		}
		
		for (i = 0; i < atoi(srLoyaltyObj->srL1DATA.szPrintRewardNum); i++)
		{
			/* 全部解析完畢 */
			if (inCnt == inDataLen)
			{
				break;
			}
			else if (inCnt > inDataLen)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "解析資料長度錯誤");
					inLogPrintf(AT, szDebugMsg);
				}
				return (VS_ERROR);
			}
		
			/* 是否列印優惠活動之標題 */
			memcpy(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].szTitleFlag, &szData[inCnt], 1);
			inCnt += 1;

			/* 優惠活動之標題長度。(最大長度限制50 Bytes，右靠左補零) */
			memcpy(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].szTitleLen, &szData[inCnt], 2);
			inCnt += 2;

			/* 優惠活動之標題內容。(端末機列印時需自動置中) */
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, &szData[inCnt], atoi(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].szTitleLen));
			inFunc_Big5toUTF8(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].szTitleContent, szBig5);
			inCnt += 50;
			/* 這裡因應轉UTF8，也修正長度 */
			sprintf(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].szTitleLen, "%02d", strlen(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].szTitleContent));

			/* 是否列印一維條碼(一) */
			memcpy(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].sz39Code1Flag, &szData[inCnt], 1);
			inCnt += 1;

			/* 一維條碼(一)長度。(最大長度限制20 Bytes，右靠左補空白) */
			memcpy(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].sz39Code1Len, &szData[inCnt], 2);
			inCnt += 2;

			/* 一維條碼(一) 補足空白至20 Bytes。 */
			memcpy(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].sz39Code1Content, &szData[inCnt], atoi(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].sz39Code1Len));
			inCnt += 20;

			/* 是否列印一維條碼(二) */
			memcpy(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].sz39Code2Flag, &szData[inCnt], 1);
			inCnt += 1;

			/* 一維條碼(二)長度。(最大長度限制20 Bytes，右靠左補空白) */
			memcpy(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].sz39Code2Len, &szData[inCnt], 2);
			inCnt += 2;

			/* 一維條碼(二) 補足空白至20 Bytes。 */
			memcpy(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].sz39Code2Content, &szData[inCnt], atoi(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].sz39Code2Len));
			inCnt += 20;

			/* 保留欄位 */
			inCnt += 1;

			/* 是否列印優惠活動之內容 1 byte */
			memcpy(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].szContentFlag, &szData[inCnt], 1);
			inCnt += 1;

			/* 優惠活動之內容長度。(最大長度限制200 Bytes，左靠右補空白) */
			memcpy(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].szContentLen, &szData[inCnt], 3);
			inCnt += 3;

			/* 優惠活動之內容 */
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, &szData[inCnt], atoi(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].szContentLen));
			inFunc_Big5toUTF8(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].szContent, szBig5);
			inCnt += 200;
			/* 這裡因應轉UTF8，也修正長度 */
			sprintf(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].szContentLen, "%02d", strlen(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].szContent));
			
			/* 檢查碼(Checksum) */
			memcpy(srLoyaltyObj->srL1DATA.srL1_AWARD_DATA[i].szChecksum, &szData[inCnt], 1);
			inCnt += 1;
			
		}
	}
	else
	{
		inLogPrintf(AT, "Award Name Not Correct");
		
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Loyalty_Data_Format() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}
