#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "../FUNCTION/EDC.h"
#include "../FUNCTION/File.h"
#include "../FUNCTION/Function.h"
#include "../EVENT/MenuMsg.h"
#include "../../NCCC/NCCCtmk.h"
#include "KMS.h"

extern  int     ginDebug;       /* Debug使用 extern */
extern  int	ginISODebug;
extern	int	ginDisplayDebug;

/*	Common KeySets（shared key 不因砍程式後消失）
 * 	KMS2_COMMON_KEY_SETS_START				0xC000
 * 	KMS2_COMMON_KEY_SETS_END				0xCFFF
 * 
 *	不能使用以下位置	
 * 	Reserved Keys for System
 * 	KMS2_RESERVED_KEY_SETS_0000				0x0000
 * 	KMS2_RESERVED_KEY_SETS_START				0xFF00
 * 	KMS2_RESERVED_KEY_SETS_END				0xFFFF
 *
 *	KMS筆記: keyset:0xC000 ~ 0xCFFF 是shared keyset 不會因為程式刪除而消失，且不能使用delete key fuction刪除
 *	    (delete all key只砍該owner的所有key，但0xC001 ~ 0xCFFF沒有owner (shared key是共用的()
 * 
 *	KMS筆記:KMS2_KEYATTRIBUTE_ENCRYPT 和 KMS2_KEYATTRIBUTE_DECRYPT 是互斥的，不可同時設定
 */

/*
Function        :inKMS_Initial
Date&Time       :2016/1/5 上午 9:25
Describe        :就只是initial KMS-II Library，原廠文件表示執行此fuction不會把key刪掉，但是在用其他KMS fuction前應該先call這一fuction
*/
int inKMS_Initial (void)
{
	CTOS_KMS2Init();
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_Write
Date&Time       :2016/2/16 上午 11:08
Describe        :
 *將傳進的structure PARA寫進TERMINAL
*/
int inKMS_Write(CTOS_KMS2KEYWRITE_PARA *srKeyWritePara)
{
	char		szDebugMsg[100 + 1];
	char		szAscii[100 + 1];
	unsigned short	usReturnValue;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "------------------------------------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inKMS_Write START()！");
		inLogPrintf(AT, szDebugMsg);
	}
	
	usReturnValue = CTOS_KMS2KeyWrite(srKeyWritePara);
	
	if (srKeyWritePara->Verification.KeyCheckValueLength > 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, srKeyWritePara->Verification.pKeyCheckValue, srKeyWritePara->Verification.KeyCheckValueLength);
			sprintf(szDebugMsg, "檢查碼:%s",szAscii);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (usReturnValue != d_OK)
	{
		/* 失敗 */
		if (ginDebug == VS_TRUE)
		{	
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "key injection failed. 代碼：0x%04X", usReturnValue);
			inLogPrintf(AT, szDebugMsg);
			
			/* Test用 平時註解掉 否則會有PCI Promblem */
			{
				char szDebugMsg[100 + 1];

				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				memset(szAscii, 0x00, sizeof(szAscii));
				inFunc_BCD_to_ASCII(szAscii, srKeyWritePara->Value.pKeyData, srKeyWritePara->Value.KeyLength);
				sprintf(szDebugMsg, "key: %s", szAscii);
				inLogPrintf(AT, szDebugMsg);
			}
			
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inKMS_Write END()！");
			inLogPrintf(AT, szDebugMsg);
			inLogPrintf(AT, "------------------------------------------------------------------");
		}
		if (ginISODebug == VS_TRUE)
		{
			char	szDebugMsg[100 + 1] = {0};

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "key injection failed. 代碼：0x%04X", usReturnValue);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
		}
		
		return (VS_ERROR);
	}
	else
	{
		/* 成功 */
		if (ginDebug == VS_TRUE)
		{	
			inLogPrintf(AT, "key injection success.");
			
			/* Test用 平時註解掉 否則會有PCI Promblem */
			{
				char szDebugMsg[100 + 1];

				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				memset(szAscii, 0x00, sizeof(szAscii));
				inFunc_BCD_to_ASCII(szAscii, srKeyWritePara->Value.pKeyData, srKeyWritePara->Value.KeyLength);
				sprintf(szDebugMsg, "key: %s", szAscii);
				inLogPrintf(AT, szDebugMsg);
			}
			
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inKMS_Write END()！");
			inLogPrintf(AT, szDebugMsg);
			inLogPrintf(AT, "------------------------------------------------------------------");
			
		}
		
		return (VS_SUCCESS);
	}
	
}

/*
Function        :inKMS_DataEncrypt
Date&Time       :2016/4/21 上午 11:33
Describe        :
 *將傳進的structure PARA寫進TERMINAL來加密
*/
int inKMS_DataEncrypt(CTOS_KMS2DATAENCRYPT_PARA *srDataEncryptPara)
{
	char		szDebugMsg[100 + 1];
	char		szAscii[100 + 1];
	unsigned short	usReturnValue;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "------------------------------------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inKMS_DataEncrypt START()！");
		inLogPrintf(AT, szDebugMsg);
	}
	
	usReturnValue = CTOS_KMS2DataEncrypt(srDataEncryptPara);
	
	if (usReturnValue != d_OK)
	{
		/* 失敗 */
		if (ginDebug == VS_TRUE)
		{	
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "data encryption failed. 代碼：0x%04X", usReturnValue);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inKMS_DataEncrypt END()！");
			inLogPrintf(AT, szDebugMsg);
			inLogPrintf(AT, "------------------------------------------------------------------");
		}
		
		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Enc Fail :0x%04X", usReturnValue);
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
		}
		
		return (VS_ERROR);
	}
	else
	{
		/* 成功 */
		if (ginDebug == VS_TRUE)
		{	
			inLogPrintf(AT, "data encryption success.");
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, srDataEncryptPara->Output.pData, srDataEncryptPara->Output.Length);
			sprintf(szDebugMsg, "OutPut(Ascii): %s", szAscii);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inKMS_DataEncrypt END()！");
			inLogPrintf(AT, szDebugMsg);
			inLogPrintf(AT, "------------------------------------------------------------------");
		}
		
		return (VS_SUCCESS);
	}
}

/*
Function        :inKMS_DeleteKey
Date&Time       :2016/2/16 下午 4:46
Describe        :Delete specifed Key
*/
int inKMS_DeleteKey(unsigned short usKeySet,unsigned short usKeyIndex)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usReturnValue;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "------------------------------------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inKMS_DeleteKey START()！");
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* Delete Specifed Key */
	usReturnValue = CTOS_KMS2KeyDelete(usKeySet, usKeyIndex);

	if (usReturnValue != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Set:%x Index:%x Delete failed 代碼 = 0x%04X", usKeySet, usKeyIndex, usReturnValue);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Set:%x Index:%x Delete OK", usKeySet, usKeyIndex);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_SUCCESS);
	}
	
	if (ginDebug == VS_TRUE)
	{	
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inKMS_DeleteKey END()！");
		inLogPrintf(AT, szDebugMsg);
		inLogPrintf(AT, "------------------------------------------------------------------");
	}
}

/*
Function        :inKMS_DeleteAllKey
Date&Time       :2016/2/16 下午 4:48
Describe        :Delete owner's ALL Key
*/
int inKMS_DeleteAllKey(void)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usReturnValue;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "------------------------------------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inKMS_DeleteAllKey START()！");
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* Delete All Key */
	usReturnValue = CTOS_KMS2KeyDeleteAll();
	
	if (usReturnValue != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Key Delete failed 代碼 = 0x%04X", usReturnValue);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ALL Key Delete OK");
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_SUCCESS);
	}
	
	if (ginDebug == VS_TRUE)
	{	
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inKMS_DeleteAllKey END()！");
		inLogPrintf(AT, szDebugMsg);
		inLogPrintf(AT, "------------------------------------------------------------------");
	}
}

/*
Function        :inKMS_CheckKey
Date&Time       :2016/2/16 下午 5:05
Describe        :Check if the specified key exists or not.
*/
int inKMS_CheckKey(unsigned short usKeySet, unsigned short usKeyIndex)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usReturnValue;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "------------------------------------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inKMS_CheckKey START()！");
		inLogPrintf(AT, szDebugMsg);
	}
	
	usReturnValue = CTOS_KMS2KeyCheck(usKeySet, usKeyIndex);
	
	if (usReturnValue != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Set:%04X Index:%04X Check failed 代碼 = 0x%04X", usKeySet, usKeyIndex, usReturnValue);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Set:%X Index:%04X Check OK", usKeySet, usKeyIndex);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_SUCCESS);
	}
	
	if (ginDebug == VS_TRUE)
	{	
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inKMS_CheckKey END()！");
		inLogPrintf(AT, szDebugMsg);
		inLogPrintf(AT, "------------------------------------------------------------------");
	}
}

/*
Function        :inKMS_GetKeyInfo
Date&Time       :2016/5/11 上午 11:53
Describe        :
*usKeySet	:要調查的keyset
*usKeyIndex	:要調查的KeyIndex
*inCVLen	:Check Value Length	不用的話填0 Only used for KeyType 3DES/3DES-DUKPT/AES
*inHashAlgorithm:Hash 演算法		不用的話填0 Only used for KeyType RSA
*/
int inKMS_GetKeyInfo(CTOS_KMS2KEYGETINFO_PARA *srPara)
{	
	char				szDebugMsg[100 + 1];
	char				szAscii[100 + 1];
	unsigned short			usReturnValue;
	
	usReturnValue = CTOS_KMS2KeyGetInfo(srPara);
	
	if (usReturnValue == d_OK)
	{
		/* 成功 */
		if (ginDebug == VS_TRUE)
		{	
			inLogPrintf(AT, "Key get info success.");
			
			inLogPrintf(AT, "Key type :");
			switch (srPara->Output.KeyType)
			{
				case KMS2_KEYTYPE_3DES :
					inLogPrintf(AT, "3DES");
					break;
				case KMS2_KEYTYPE_3DES_DUKPT :
					inLogPrintf(AT, "3DES_DUKPT");
					break;
				case KMS2_KEYTYPE_AES :
					inLogPrintf(AT, "AES");
					break;
				case KMS2_KEYTYPE_RSA :
					inLogPrintf(AT, "RSA");
					break;
				case KMS2_KEYTYPE_DES_DUKPT :
					inLogPrintf(AT, "DES_DUKPT");
					break;
				default :
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "key type not found, value : %d", srPara->Output.KeyType);
					inLogPrintf(AT, szDebugMsg);
					break;
			}
			
			inLogPrintf(AT, "Key Attribute :");
			if (srPara->Output.KeyAttribute & KMS2_KEYATTRIBUTE_PIN)
				inLogPrintf(AT, "KEYATTRIBUTE_PIN");
			if (srPara->Output.KeyAttribute & KMS2_KEYATTRIBUTE_ENCRYPT)
				inLogPrintf(AT, "KEYATTRIBUTE_ENCRYPT");
			if (srPara->Output.KeyAttribute & KMS2_KEYATTRIBUTE_MAC)
				inLogPrintf(AT, "KEYATTRIBUTE_MAC");
			if (srPara->Output.KeyAttribute & KMS2_KEYATTRIBUTE_KPK)
				inLogPrintf(AT, "KEYATTRIBUTE_KPK");
			if (srPara->Output.KeyAttribute & KMS2_KEYATTRIBUTE_DECRYPT)
				inLogPrintf(AT, "KEYATTRIBUTE_DECRYPT");
			if (srPara->Output.KeyAttribute & KMS2_KEYATTRIBUTE_KBPK)
				inLogPrintf(AT, "KEYATTRIBUTE_KBPK");
			if (srPara->Output.KeyAttribute & KMS2_KEYATTRIBUTE_SK_ENCRYPT)
				inLogPrintf(AT, "KEYATTRIBUTE_SK_ENCRYPT");
			if (srPara->Output.KeyAttribute & KMS2_KEYATTRIBUTE_INTERMEDIATE)
				inLogPrintf(AT, "KEYATTRIBUTE_INTERMEDIATE");
			if (srPara->Output.KeyAttribute & KMS2_KEYATTRIBUTE_FREEZE_RSA_WRITE_KEY_BY_CERTIFICATE)
				inLogPrintf(AT, "KEYATTRIBUTE_FREEZE_RSA_WRITE_KEY_BY_CERTIFICATE");
			if (srPara->Output.KeyAttribute & KMS2_KEYATTRIBUTE_FREEZE_RSA_ENCRYPT)
				inLogPrintf(AT, "KEYATTRIBUTE_FREEZE_RSA_ENCRYPT");
			if (srPara->Output.KeyAttribute & KMS2_KEYATTRIBUTE_CONSIDER_INVALID_BIT_AS_VALID_FOR_KEY_VALUE_UNIQUE)
				inLogPrintf(AT, "KEYATTRIBUTE_CONSIDER_INVALID_BIT_AS_VALID_FOR_KEY_VALUE_UNIQUE");
			if (srPara->Output.KeyAttribute & KMS2_KEYATTRIBUTE_VALUE_UNIQUE)
				inLogPrintf(AT, "KEYATTRIBUTE_VALUE_UNIQUE");
			if (srPara->Output.KeyAttribute & KMS2_KEYATTRIBUTE_PROTECTED)
				inLogPrintf(AT, "KEYATTRIBUTE_PROTECTED");
			

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "KeyLength : %u", srPara->Output.KeyLength);
			inLogPrintf(AT, szDebugMsg);

			if (srPara->Input.CVLen != 0)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				memset(szAscii, 0x00, sizeof(szAscii));
				inFunc_BCD_to_ASCII(szAscii, (unsigned char*)srPara->Output.pCV, srPara->Input.CVLen);
				sprintf(szDebugMsg, "Key Check Value : %s", szAscii);
				inLogPrintf(AT, szDebugMsg);
			}
			
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inKMS_GetKeyInfo END()！");
			inLogPrintf(AT, szDebugMsg);
			inLogPrintf(AT, "------------------------------------------------------------------");
		}
		
		return (VS_SUCCESS);
	}
	else if (usReturnValue != d_KMS2_KEY_NOT_EXIST)
	{
		/* 失敗 */
		if (ginDebug == VS_TRUE)
		{	
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Key get info failed. 代碼：0x%04X", usReturnValue);
			inLogPrintf(AT, szDebugMsg);
			
			inLogPrintf(AT, "Key not exist.");
			
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inKMS_GetKeyInfo END()！");
			inLogPrintf(AT, szDebugMsg);
			inLogPrintf(AT, "------------------------------------------------------------------");
		}
		
		return (usReturnValue);
	}
	else
	{
		/* 失敗 */
		if (ginDebug == VS_TRUE)
		{	
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Key get info failed. 代碼：0x%04X", usReturnValue);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inKMS_GetKeyInfo END()！");
			inLogPrintf(AT, szDebugMsg);
			inLogPrintf(AT, "------------------------------------------------------------------");
		}
		
		return (VS_ERROR);
	}
	
}

/*
Function        :inKMS_GetKeyInfo_LookUp
Date&Time       :2017/1/12 下午 4:42
Describe        :
*usKeySet	:要調查的keyset
*usKeyIndex	:要調查的KeyIndex
*inCVLen	:Check Value Length	不用的話填0 Only used for KeyType 3DES/3DES-DUKPT/AES
*inHashAlgorithm:Hash 演算法		不用的話填0 Only used for KeyType RSA
*/
int inKMS_GetKeyInfo_LookUp()
{	
	int				inRetVal;
	int				inCVLen;
	int				inHashAlgorithm;
	char				szDebugMsg[100 + 1];
	char				szAscii[100 + 1];
	char				szBCD[50 + 1];
	char				szDispMsg[50 + 1];
	char				szTemplate[50 + 1];
	char				szKeyCheckValue[6 + 1];
	char				szTest;
	unsigned short			usKeySet;
	unsigned short			usKeyIndex;
	DISPLAY_OBJECT			srDispObj;
	CTOS_KMS2KEYGETINFO_PARA	srPara;
	
	/* 輸入 Key Set */
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 4;
	srDispObj.inColor = _COLOR_RED_;

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	
	/* Key Set */
        inDISP_ChineseFont_Color("Key Set:", _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);
	
	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;
	
        inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (VS_ERROR);
	
	memset(szAscii, 0x00, sizeof(szAscii));
	strcpy(szAscii, "0000");
	memcpy(&szAscii[4 - srDispObj.inOutputLen], srDispObj.szOutput, srDispObj.inOutputLen);
	
	memset(szBCD, 0x00, sizeof(szBCD));
	inFunc_ASCII_to_BCD((unsigned char*)szBCD, szAscii, 2);
	usKeySet = (unsigned short)(szBCD[0] *256 + szBCD[1]);
	
	/* 輸入 Key Index */
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 4;
	srDispObj.inColor = _COLOR_RED_;

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	
	/* 輸入 Key Index */
        inDISP_ChineseFont_Color("Key Index:", _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);
	
	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;
	
        inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (VS_ERROR);
	
	memset(szAscii, 0x00, sizeof(szAscii));
	strcpy(szAscii, "0000");
	memcpy(&szAscii[4 - srDispObj.inOutputLen], srDispObj.szOutput, srDispObj.inOutputLen);
	
	memset(szBCD, 0x00, sizeof(szBCD));
	inFunc_ASCII_to_BCD((unsigned char*)szBCD, szAscii, 2);
	usKeyIndex = (unsigned short)(szBCD[0] *256 + szBCD[1]);
	
	/* CHeckValueLen
	 * Only used for KeyType 3DES/3DES-DUKPT/AES
	 */
	inCVLen = 6;
	
	/* HashAlgorithm 
	 * Only used for KeyType RSA
	 * SHA1		KMS2_KEYCERTIFICATEGENERATECIHERMETHOD_DEFAULT_WITH_SHA1 (0x00)
	 * SHA256	KMS2_KEYCERTIFICATEGENERATECIHERMETHOD_DEFAULT_WITH_SHA2 (0x01)
	 */
	inHashAlgorithm = 0x00;
	
	/* 放入結構中 */
	memset(&srPara, 0x00, sizeof(CTOS_KMS2KEYGETINFO_PARA));
	srPara.Version = 0x01;	
	srPara.Input.KeySet = usKeySet;
	srPara.Input.KeyIndex = usKeyIndex;
	srPara.Input.CVLen = inCVLen;				/* Only used for KeyType 3DES/3DES-DUKPT/AES */
	srPara.Input.HashAlgorithm = inHashAlgorithm;		/* Only used for KeyType RSA */
	srPara.Output.pCV = (unsigned char*)szKeyCheckValue;
	
	inRetVal = inKMS_GetKeyInfo(&srPara);
	
	if (inRetVal == VS_SUCCESS)
	{
		/* 成功 */
		
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		
		switch (srPara.Output.KeyType)
		{
			case KMS2_KEYTYPE_3DES :
				strcat(szTemplate, "3DES");
				break;
			case KMS2_KEYTYPE_3DES_DUKPT :
				strcat(szTemplate, "3DES_DUKPT");
				break;
			case KMS2_KEYTYPE_AES :
				strcat(szTemplate, "AES");
				break;
			case KMS2_KEYTYPE_RSA :
				strcat(szTemplate, "RSA");
				break;
			case KMS2_KEYTYPE_DES_DUKPT :
				strcat(szTemplate, "DES_DUKPT");
				break;
			default :
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Not found, value : %d", srPara.Output.KeyType);
				strcat(szTemplate, szDebugMsg);
				break;
		}
		
		sprintf(szDispMsg, "%s : %s", "Key Type", szTemplate);
		inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X22_, _LINE_8_4_, _COLOR_BLACK_, _DISP_LEFT_);
		
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		
		if (srPara.Output.KeyAttribute & KMS2_KEYATTRIBUTE_PIN)
			strcat(szTemplate, "PIN/");
		if (srPara.Output.KeyAttribute & KMS2_KEYATTRIBUTE_ENCRYPT)
			strcat(szTemplate, "ENC/");
		if (srPara.Output.KeyAttribute & KMS2_KEYATTRIBUTE_MAC)
			strcat(szTemplate, "MAC/");
		if (srPara.Output.KeyAttribute & KMS2_KEYATTRIBUTE_KPK)
			strcat(szTemplate, "KPK/");
		if (srPara.Output.KeyAttribute & KMS2_KEYATTRIBUTE_DECRYPT)
			strcat(szTemplate, "DEC/");
		if (srPara.Output.KeyAttribute & KMS2_KEYATTRIBUTE_KBPK)
			strcat(szTemplate, "KBPK/");
		if (srPara.Output.KeyAttribute & KMS2_KEYATTRIBUTE_SK_ENCRYPT)
			strcat(szTemplate, "SK ENC/");
		if (srPara.Output.KeyAttribute & KMS2_KEYATTRIBUTE_INTERMEDIATE)
			strcat(szTemplate, "INTER/");
		if (srPara.Output.KeyAttribute & KMS2_KEYATTRIBUTE_FREEZE_RSA_WRITE_KEY_BY_CERTIFICATE)
			strcat(szTemplate, "FR RSA CE/");
		if (srPara.Output.KeyAttribute & KMS2_KEYATTRIBUTE_FREEZE_RSA_ENCRYPT)
			strcat(szTemplate, "FR RSA ENC/");
		if (srPara.Output.KeyAttribute & KMS2_KEYATTRIBUTE_CONSIDER_INVALID_BIT_AS_VALID_FOR_KEY_VALUE_UNIQUE)
			strcat(szTemplate, "CON/");
		if (srPara.Output.KeyAttribute & KMS2_KEYATTRIBUTE_VALUE_UNIQUE)
			strcat(szTemplate, "UNI/");
		if (srPara.Output.KeyAttribute & KMS2_KEYATTRIBUTE_PROTECTED)
			strcat(szTemplate, "PRO/");
		
		if (szTemplate[strlen(szTemplate) - 1] == '/')
			szTemplate[strlen(szTemplate) - 1] = 0x00;
		
		sprintf(szDispMsg, "%s : %s", "Key Attr", szTemplate);
		inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
		
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "KeyLength : %u", srPara.Output.KeyLength);
		inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);
		
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char*)srPara.Output.pCV, inCVLen);
		sprintf(szDispMsg, "Key CV : %s", szAscii);
		inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_BLACK_, _DISP_LEFT_);
		
		szTest = 0x00;
		inNCCC_TMK_Check_Test_Key(srPara.Input.KeySet, srPara.Input.KeyIndex, &szTest);
		if (szTest == 'Y')
		{
			inDISP_ChineseFont_Color("這是測試Key", _FONTSIZE_8X22_, _LINE_8_8_, _COLOR_RED_, _DISP_LEFT_);
		}

		uszKBD_GetKey(30);
		
		return (VS_SUCCESS);
	}
	else if (inRetVal == d_KMS2_KEY_NOT_EXIST)
	{
		inDISP_ChineseFont_Color("該位置Key不存在", _FONTSIZE_8X22_, _LINE_8_8_, _COLOR_RED_, _DISP_LEFT_);
		uszKBD_GetKey(30);
		
		return (VS_ERROR);
	}
	else
	{
		/* 失敗 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		
		return (VS_ERROR);
	}
	
	
}

/*
Function        :inKMS_Key_Swap
Date&Time       :2017/1/13 上午 10:39
Describe        :
*/
int inKMS_Key_Swap(unsigned short usKeySet1, unsigned short usKeyIndex1, unsigned short usKeySet2, unsigned short usKeyIndex2)
{
	char			szDebugMsg[100 + 1];
	unsigned short		usRetVal;
	CTOS_KMS2KEYSWAP_PARA	srPara;
	
	memset(&srPara, 0x00, sizeof(srPara));
	srPara.Version = 0x01;
	srPara.Source1.KeySet = usKeySet1;
	srPara.Source1.KeyIndex = usKeyIndex1;
	srPara.Source2.KeySet = usKeySet2;
	srPara.Source2.KeyIndex = usKeyIndex2;
	
	usRetVal = CTOS_KMS2KeySwap(&srPara);
		
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inKMS_Key_Swap Failed Value:0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
 Function	:inKMS_Write_NULL_Key
 Date&Time	:2016/2/5 下午 2:39
 Describe	:寫測試MasterKey的function
 *para.Version:			Structure Format Version，可填入0x00或0x01
 *para.Info.KeySet:		key set(一個key set只能有一種key type，EX:只能3DES or RSA)
 *para.Info.KeyIndex:		key index
 *para.Info.KeyType:		key之後會用在哪一種加密方法(EX:3DES or RSA......等)
 *para.Info.KeyVersion:		使用者自訂，用來管理key的版本，基本上無用
 *para.Info.KeyAttribute:	若有多個屬性，用"or"把bit on起來（KMS2_KEYATTRIBUTE_KPK表示是用來加密其他key的key）
 *para..Protection.Mode:	key寫進去時的加密狀態（KMS2_KEYPROTECTIONMODE_PLAINTEXT 表示明碼寫入）
*/
int inKMS_Write_NULL_Key(unsigned short usKeySet, unsigned short usKeyIndex)
{
	char			szDebugMsg[100 + 1];
	char			szKeyData[48 + 1];
	unsigned char		uszHex[24 + 1];		/* 3DES的Key長度最長24byte，若只輸入16byte則k1,k2,k3中，k3 = k1 */
	unsigned short		usKeyLength;
	CTOS_KMS2KEYWRITE_PARA	srKeyWritePara;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "------------------------------------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inKMS_Write_NULL_Key START()！");
		inLogPrintf(AT, szDebugMsg);
	}
	
	memset(&srKeyWritePara, 0x00, sizeof(srKeyWritePara));
	memset(&uszHex, 0x00, sizeof(uszHex));

	/* 將Terminal Master KEY所需參數放入結構中 */
	srKeyWritePara.Version = 0x01;						
	srKeyWritePara.Info.KeySet = usKeySet;
	srKeyWritePara.Info.KeyIndex = usKeyIndex;
	srKeyWritePara.Info.KeyType = KMS2_KEYTYPE_3DES;
	srKeyWritePara.Info.KeyVersion = 0x01;					
	srKeyWritePara.Info.KeyAttribute = KMS2_KEYATTRIBUTE_KPK;
	srKeyWritePara.Protection.Mode = KMS2_KEYPROTECTIONMODE_PLAINTEXT;	
	
	/* 將ASCII的key轉成HEX */
	memset(szKeyData, 0x00, sizeof(szKeyData));
	memcpy(szKeyData, "00000000000000000000000000000000", strlen("00000000000000000000000000000000"));
	usKeyLength = strlen(szKeyData) / 2;
	inFunc_ASCII_to_BCD(uszHex, szKeyData, usKeyLength);
	srKeyWritePara.Value.pKeyData = uszHex;
	srKeyWritePara.Value.KeyLength = usKeyLength;
	inKMS_Write(&srKeyWritePara);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inKMS_Write_NULL_Key END()！");
		inLogPrintf(AT, szDebugMsg);
		inLogPrintf(AT, "------------------------------------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inKMS_Check_NULL_Key
Date&Time       :2017/1/13 上午 11:30
Describe        :確認是否是空白Key，是空白Key則szTest填'Y'，不是空白Key填'N'，沒有key則為空(0x00)
*/
int inKMS_Check_NULL_Key(unsigned short usKeySet, unsigned short usKeyIndex, char *szNull)
{
	int				inRetVal;
	int				inCVLen = 3;
	int				inHashAlgorithm = 0;
	char				szKeyCheckValue[6 + 1];
	char				szAscii[6 + 1];
	CTOS_KMS2KEYGETINFO_PARA	srPara;
	
	/* 放入結構中 */
	memset(szKeyCheckValue, 0x00, sizeof(szKeyCheckValue));
	memset(&srPara, 0x00, sizeof(CTOS_KMS2KEYGETINFO_PARA));
	srPara.Version = 0x01;	
	srPara.Input.KeySet = usKeySet;
	srPara.Input.KeyIndex = usKeyIndex;
	srPara.Input.CVLen = inCVLen;				/* Only used for KeyType 3DES/3DES-DUKPT/AES */
	srPara.Input.HashAlgorithm = inHashAlgorithm;		/* Only used for KeyType RSA */
	srPara.Output.pCV = (unsigned char*)szKeyCheckValue;
	*szNull = 0x00;
	
	inRetVal = inKMS_GetKeyInfo(&srPara);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	memset(szAscii, 0x00, sizeof(szAscii));
	inFunc_BCD_to_ASCII(szAscii, srPara.Output.pCV, 3);
	
	if (memcmp(szAscii, "000000", strlen("000000")) == 0)
	{
		*szNull = 'Y';

		return (VS_SUCCESS);
	}
	
	*szNull = 'N';
			
	return (VS_SUCCESS);
}
