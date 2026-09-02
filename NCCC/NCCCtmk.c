#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sqlite3.h>
#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/AllStruct.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DisTouch.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/APDU.h"
#include "../SOURCE/FUNCTION/Batch.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/ECR.h"
#include "../SOURCE/FUNCTION/File.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/KMS.h"
#include "../SOURCE/FUNCTION/Utility.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../FISC/NCCCfisc.h"
#include "NCCCesc.h"
#include "NCCCtmk.h"
#include "NCCCats.h"

extern  int		ginDebug;       /* Debug使用 extern */
extern  int		ginISODebug;
extern	int		ginHalfLCD;
NCCC_TMK_COMMAND	gsrTMKdata;

// 為了方便測試，定義測試資料長度（必須為 16 的倍數，例如 64, 256, 1024 等）
#define TEST_DATA_SIZE 1024 

KEY_TABLE TMK_KEY_TABLE[] =
{
	{{_MULTI_TMK_DATA_1_, 
	  _MULTI_TMK_DATA_2_,
	  _MULTI_TMK_DATA_3_,
	  _MULTI_TMK_DATA_4_,
	  _MULTI_TMK_DATA_5_,
	  _MULTI_TMK_DATA_6_,
	  _MULTI_TMK_DATA_7_,
	  _MULTI_TMK_DATA_8_,
	  _MULTI_TMK_DATA_9_,
	  _MULTI_TMK_DATA_10_,
	  _MULTI_TMK_DATA_11_,
	  _MULTI_TMK_DATA_12_,
	  _MULTI_TMK_DATA_13_,
	  _MULTI_TMK_DATA_14_,
	  _MULTI_TMK_DATA_15_}},
	 
	  {{_SELF_TRANS_MULTI_TMK_DATA_1_, 
	    _SELF_TRANS_MULTI_TMK_DATA_2_,
	    _SELF_TRANS_MULTI_TMK_DATA_3_,
	    _SELF_TRANS_MULTI_TMK_DATA_4_,
	    _SELF_TRANS_MULTI_TMK_DATA_5_,
	    _SELF_TRANS_MULTI_TMK_DATA_6_,
	    _SELF_TRANS_MULTI_TMK_DATA_7_,
	    _SELF_TRANS_MULTI_TMK_DATA_8_,
	    _SELF_TRANS_MULTI_TMK_DATA_9_,
	    _SELF_TRANS_MULTI_TMK_DATA_10_,
	    _SELF_TRANS_MULTI_TMK_DATA_11_,
	    _SELF_TRANS_MULTI_TMK_DATA_12_,
	    _SELF_TRANS_MULTI_TMK_DATA_13_,
	    _SELF_TRANS_MULTI_TMK_DATA_14_,
	    _SELF_TRANS_MULTI_TMK_DATA_15_}},
};

KCV_TABLE TMK_KCV_TABLE[] =
{
	{_MULTI_TMK_KCV_1_},
	{_MULTI_TMK_KCV_2_},
	{_MULTI_TMK_KCV_3_},
	{_MULTI_TMK_KCV_4_},
	{_MULTI_TMK_KCV_5_},
	{_MULTI_TMK_KCV_6_},
	{_MULTI_TMK_KCV_7_},
	{_MULTI_TMK_KCV_8_},
	{_MULTI_TMK_KCV_9_},
	{_MULTI_TMK_KCV_10_},
	{_MULTI_TMK_KCV_11_},
	{_MULTI_TMK_KCV_12_},
	{_MULTI_TMK_KCV_13_},
	{_MULTI_TMK_KCV_14_},
	{_MULTI_TMK_KCV_15_},
};

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
 */

/*
 Function	:inNCCC_TMK_Write_Test_TMK_By_Terminal
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
int inNCCC_TMK_Write_Test_TMK_By_Terminal()
{
	int			i;
	char			szDebugMsg[100 + 1];
	char			szDispMsg[30 + 1];
	char			szKeyData[48 + 1];
	unsigned char		uszHex[24 + 1];		/* 3DES的Key長度最長24byte，若只輸入16byte則k1,k2,k3中，k3 = k1 */
	unsigned short		usKeyLength;
	CTOS_KMS2KEYWRITE_PARA	srKeyWritePara;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "------------------------------------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_TMK_Write_Test_TerminalMasterKey START()！");
		inLogPrintf(AT, szDebugMsg);
	}
	
	memset(&srKeyWritePara, 0x00, sizeof(srKeyWritePara));
	memset(&uszHex, 0x00, sizeof(uszHex));

	/* 將Terminal Master KEY所需參數放入結構中 */
	srKeyWritePara.Version = 0x01;						
	srKeyWritePara.Info.KeySet = _TMK_KEYSET_NCCC_;
	srKeyWritePara.Info.KeyIndex = _TMK_KEYINDEX_NCCC_;
	srKeyWritePara.Info.KeyType = KMS2_KEYTYPE_3DES;
	srKeyWritePara.Info.KeyVersion = 0x01;					
	srKeyWritePara.Info.KeyAttribute = KMS2_KEYATTRIBUTE_KPK;
	srKeyWritePara.Protection.Mode = KMS2_KEYPROTECTIONMODE_PLAINTEXT;	
	
	/* 存15把key */
	for (i = 0; i < _KEY_TOTAL_COUNT_; i++)
	{
		/* 將ASCII的key轉成HEX */
		memset(szKeyData, 0x00, sizeof(szKeyData));
		memcpy(szKeyData, (char*)&TMK_KEY_TABLE[0].szKey[i][0], strlen((char*)&TMK_KEY_TABLE[0].szKey[i][0]));
		usKeyLength = strlen((char*)&TMK_KEY_TABLE[0].szKey[i][0]) / 2;
		
		inFunc_ASCII_to_BCD(uszHex, szKeyData, usKeyLength);
		srKeyWritePara.Value.pKeyData = uszHex;
		srKeyWritePara.Value.KeyLength = usKeyLength;
		
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		
		if (inKMS_Write(&srKeyWritePara) != VS_SUCCESS)
		{
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "第%d key寫入失敗.", i + 1);
			inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_LEFT_);
			inDISP_Wait(3000);
		}
		else
		{
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "第%d key寫入成功.", i + 1);
			inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_LEFT_);
		}
			
		/* 寫入後換下一個位置 */
		srKeyWritePara.Info.KeyIndex++;
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_TMK_Write_Test_TerminalMasterKey END()！");
		inLogPrintf(AT, szDebugMsg);
		inLogPrintf(AT, "------------------------------------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_Write_Test_TMK_By_Terminal_Self_Trans
Date&Time       :2018/11/30 下午 5:53
Describe        :自購機用
*/
int inNCCC_TMK_Write_Test_TMK_By_Terminal_Self_Trans()
{
	int			i;
	char			szDebugMsg[100 + 1];
	char			szDispMsg[30 + 1];
	char			szKeyData[48 + 1];
	unsigned char		uszHex[24 + 1];		/* 3DES的Key長度最長24byte，若只輸入16byte則k1,k2,k3中，k3 = k1 */
	unsigned short		usKeyLength;
	CTOS_KMS2KEYWRITE_PARA	srKeyWritePara;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "------------------------------------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_TMK_Write_Test_TerminalMasterKey START()！");
		inLogPrintf(AT, szDebugMsg);
	}
	
	memset(&srKeyWritePara, 0x00, sizeof(srKeyWritePara));
	memset(&uszHex, 0x00, sizeof(uszHex));

	/* 將Terminal Master KEY所需參數放入結構中 */
	srKeyWritePara.Version = 0x01;						
	srKeyWritePara.Info.KeySet = _TMK_KEYSET_NCCC_;
	srKeyWritePara.Info.KeyIndex = _TMK_KEYINDEX_NCCC_;
	srKeyWritePara.Info.KeyType = KMS2_KEYTYPE_3DES;
	srKeyWritePara.Info.KeyVersion = 0x01;					
	srKeyWritePara.Info.KeyAttribute = KMS2_KEYATTRIBUTE_KPK;
	srKeyWritePara.Protection.Mode = KMS2_KEYPROTECTIONMODE_PLAINTEXT;	
	
	/* 存15把key */
	for (i = 0; i < _KEY_TOTAL_COUNT_; i++)
	{
		/* 將ASCII的key轉成HEX */
		memset(szKeyData, 0x00, sizeof(szKeyData));
		memcpy(szKeyData, (char*)&TMK_KEY_TABLE[1].szKey[i][0], strlen((char*)&TMK_KEY_TABLE[1].szKey[i][0]));
		usKeyLength = strlen((char*)&TMK_KEY_TABLE[1].szKey[i][0]) / 2;
		
		inFunc_ASCII_to_BCD(uszHex, szKeyData, usKeyLength);
		srKeyWritePara.Value.pKeyData = uszHex;
		srKeyWritePara.Value.KeyLength = usKeyLength;
		
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		
		if (inKMS_Write(&srKeyWritePara) != VS_SUCCESS)
		{
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "第%d key寫入失敗.", i + 1);
			inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_LEFT_);
			inDISP_Wait(3000);
		}
		else
		{
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "第%d key寫入成功.", i + 1);
			inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_LEFT_);
		}
			
		/* 寫入後換下一個位置 */
		srKeyWritePara.Info.KeyIndex++;
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_TMK_Write_Test_TerminalMasterKey END()！");
		inLogPrintf(AT, szDebugMsg);
		inLogPrintf(AT, "------------------------------------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
 Function	:inNCCC_TMK_Write_TMK
 Date&Time	:2017/11/14 下午 5:57
 Describe	:寫入Production key
 *para.Version:			Structure Format Version，可填入0x00或0x01
 *para.Info.KeySet:		key set(一個key set只能有一種key type，EX:只能3DES or RSA)
 *para.Info.KeyIndex:		key index(從0開始)
 *para.Info.KeyType:		key之後會用在哪一種加密方法(EX:3DES or RSA......等)
 *para.Info.KeyVersion:		使用者自訂，用來管理key的版本，基本上無用
 *para.Info.KeyAttribute:	若有多個屬性，用"or"把bit on起來（KMS2_KEYATTRIBUTE_KPK表示是用來加密其他key的key）
 *para..Protection.Mode:	key寫進去時的加密狀態（KMS2_KEYPROTECTIONMODE_PLAINTEXT 表示明碼寫入）
*/
int inNCCC_TMK_Write_TMK(int inKeyCnt, char *szKeyDataAscii, int inKeyDataAsciiLen)
{
	int			inCnt = 0;
	char			szDispMsg[30 + 1];
	char			szKeyData[48 + 1];
	unsigned char		uszHex[24 + 1];		/* 3DES的Key長度最長24byte，若只輸入16byte則k1,k2,k3中，k3 = k1 */
	unsigned short		usKeyLength;
	CTOS_KMS2KEYWRITE_PARA	srKeyWritePara;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "------------------------------------------------------------------");
		inLogPrintf(AT, "inNCCC_TMK_Write_TMK START()！");
	}
	if (ginISODebug == VS_TRUE)
	{
		inPRINT_ChineseFont("inNCCC_TMK_Write_TMK START()！", _PRT_ISO_);
	}
	
	memset(&srKeyWritePara, 0x00, sizeof(srKeyWritePara));
	memset(&uszHex, 0x00, sizeof(uszHex));

	/* 將Terminal Master KEY所需參數放入結構中 */
	srKeyWritePara.Version = 0x01;						
	srKeyWritePara.Info.KeySet = _TMK_KEYSET_NCCC_;
	srKeyWritePara.Info.KeyIndex = _TMK_KEYINDEX_NCCC_ + inKeyCnt;
	srKeyWritePara.Info.KeyType = KMS2_KEYTYPE_3DES;
	srKeyWritePara.Info.KeyVersion = 0x01;					
	srKeyWritePara.Info.KeyAttribute = KMS2_KEYATTRIBUTE_KPK;
	srKeyWritePara.Protection.Mode = KMS2_KEYPROTECTIONMODE_PLAINTEXT;	
	
	/* 將ASCII的key轉成HEX */
	memset(szKeyData, 0x00, sizeof(szKeyData));
	memcpy(szKeyData, &szKeyDataAscii[inCnt], inKeyDataAsciiLen);
	usKeyLength = strlen(szKeyData) / 2;

	inFunc_ASCII_to_BCD(uszHex, szKeyData, usKeyLength);
	srKeyWritePara.Value.pKeyData = uszHex;
	srKeyWritePara.Value.KeyLength = usKeyLength;

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

	if (inKMS_Write(&srKeyWritePara) != VS_SUCCESS)
	{
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "%dth key inject fail.", inKeyCnt + 1);
		inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_RED_, _DISP_LEFT_);
		inDISP_Wait(3000);
	}
	else
	{
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "%dth key inject success.", inKeyCnt + 1);
		inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_RED_, _DISP_LEFT_);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_TMK_Write_TMK END()！");
		inLogPrintf(AT, "------------------------------------------------------------------");
	}
	if (ginISODebug == VS_TRUE)
	{
		inPRINT_ChineseFont("inNCCC_TMK_Write_TMK END()！", _PRT_ISO_);
		inPRINT_ChineseFont("------------------------------------------------------------------", _PRT_ISO_);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_Write_TMK_By_KeyCard_Flow
Date&Time       :2018/11/19 下午 3:12
Describe        :
*/
int inNCCC_TMK_Write_TMK_By_KeyCard_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	i = 0;
	int	inKeyCnt = 0;
	int	inKeyLen = 0;
	int	inRetVal = VS_ERROR;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_TMK_Write_TMK_By_KeyCard_Flow() START !");
	}
	if (ginISODebug == VS_TRUE)
	{
		inPRINT_ChineseFont("----------------------------------------", _PRT_ISO_);
		inPRINT_ChineseFont("inNCCC_TMK_Write_TMK_By_KeyCard_Flow() START !", _PRT_ISO_);
	}
	
	inKeyCnt = gsrTMKdata.uszGET_KCT[0];
	/* 取TMK DATA */
	for (i = 0; i < inKeyCnt; i++)
	{
		inKeyLen = gsrTMKdata.uszGET_KLEN[i][0] * 2;
		inRetVal = inNCCC_TMK_Write_TMK(i, (char*)&gsrTMKdata.uszTMK_DES_Ascii[i][0], inKeyLen);
		if (inRetVal != VS_SUCCESS)
		{
			break;
		}
	}
	
	
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_TMK_Write_TMK_By_KeyCard_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	if (ginISODebug == VS_TRUE)
	{
		inPRINT_ChineseFont("inNCCC_TMK_Write_TMK_By_KeyCard_Flow() END !", _PRT_ISO_);
		inPRINT_ChineseFont("----------------------------------------", _PRT_ISO_);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_Write_PINKey
Date&Time       :2016/2/16 上午 9:20
Describe        :
 *para.Version:				Structure Format Version，可填入0x00或0x01
 *para.Info.KeySet:			key set(一個key set只能有一種key type)
 *para.Info.KeyIndex:			key index
 *para.Info.KeyType:			key之後會用在哪一種加密方法
 *para.Info.KeyVersion:			使用者自訂，用來管理key的版本，基本上無用
 *para.Info.KeyAttribute:		若有多個屬性，用"or"把bit on起來
 *Para.Protection.CipherKeySet		KetProtectionKey的KeySet
 *Para.Protection.CipherKeyIndex	KetProtectionKey的KeyIndex
 *para.Protection.Mode:			key寫進去時的加密狀態（KMS2_KEYPROTECTIONMODE_KPK_ECB 表示ECB寫入）
 *usTMKindex:				用第幾把Master Key
 *usPINKeyLen:				PINKey HEX長度	
 *szPINKeyAscii				PINKey Ascii形式
 *szKeyCheckValueAscii			KeyCheckValue Ascii形式
*/
int inNCCC_TMK_Write_PINKey(unsigned short usTMKindex, unsigned short usPINKeyLen, char *szPINKeyAscii, char* szKeyCheckValueAscii)
{
	char			szDebugMsg[100 + 1];
	char			szAscii[64 + 1];
	unsigned char		uszPINKeyHex[24 + 1];		/* 3DES最長24BYTE */
	unsigned char		uszKeyCheckValueHex[4 + 1];	/* KeyCheckValue 3byte 求偶數加1byte */
	unsigned short		usKeyCheckValueLength;
	unsigned short		usReturnValue;
	CTOS_KMS2KEYWRITE_PARA	srKeyWritePara;
	
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "------------------------------------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_TMK_Write_PINKey START()！");
		inLogPrintf(AT, szDebugMsg);
	}
	
	memset(&srKeyWritePara, 0x00, sizeof(CTOS_KMS2KEYWRITE_PARA));
	memset(&uszPINKeyHex, 0x00, sizeof(uszPINKeyHex));
	memset(&usKeyCheckValueLength, 0x00, sizeof(usKeyCheckValueLength));
	memset(&uszKeyCheckValueHex, 0x00, sizeof(uszKeyCheckValueHex));

	/* 可以不帶KeyCheckValue */
	if (strlen(szKeyCheckValueAscii) > 0)
	{
		/* Key Check Value Length*/
		usKeyCheckValueLength = strlen(szKeyCheckValueAscii) / 2;							/* CheckValue 是key來加密全0 Block之後密文的前6位字母數字(3byte hex)，所以hardcode */
		/* 將key check value轉成HEX格式 */
		inFunc_ASCII_to_BCD(uszKeyCheckValueHex, szKeyCheckValueAscii, usKeyCheckValueLength);
	}
	
	/* 將key轉成HEX格式 */
	inFunc_ASCII_to_BCD(uszPINKeyHex, szPINKeyAscii, usPINKeyLen);
	
	/* 將PIN KEY所需參數放入結構中 */
	srKeyWritePara.Version = 0x01;
	srKeyWritePara.Info.KeySet = _TWK_KEYSET_NCCC_;
	srKeyWritePara.Info.KeyIndex = _TWK_KEYINDEX_NCCC_PIN_ONLINE_;
	srKeyWritePara.Info.KeyType = KMS2_KEYTYPE_3DES;
	srKeyWritePara.Info.KeyVersion = 0x01;
	srKeyWritePara.Info.KeyAttribute =  KMS2_KEYATTRIBUTE_PIN | KMS2_KEYATTRIBUTE_ENCRYPT;
	srKeyWritePara.Protection.Mode = KMS2_KEYPROTECTIONMODE_KPK_ECB;
	srKeyWritePara.Protection.CipherKeySet = _TMK_KEYSET_NCCC_;
	srKeyWritePara.Protection.CipherKeyIndex = _TMK_KEYINDEX_NCCC_ + (usTMKindex - 1);	/* 0x0000放第一把key，0x0001放第二把key，所以減一 */
	srKeyWritePara.Value.KeyLength = usPINKeyLen;
	srKeyWritePara.Value.pKeyData = uszPINKeyHex;
	
	if (usKeyCheckValueLength > 0)
	{
		srKeyWritePara.Verification.Method = KMS2_KEYVERIFICATIONMETHOD_DEFAULT;
		srKeyWritePara.Verification.KeyCheckValueLength = usKeyCheckValueLength;
		srKeyWritePara.Verification.pKeyCheckValue = uszKeyCheckValueHex;
	}
	
	/* Write PIN Key*/
	usReturnValue = inKMS_Write(&srKeyWritePara);
	
	if (usReturnValue != VS_SUCCESS)
	{	
		/* 失敗 */
		if (ginDebug == VS_TRUE)
		{
			/* 被加密的working key */
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, srKeyWritePara.Value.pKeyData, srKeyWritePara.Value.KeyLength);
			inLogPrintf(AT, "Failed Encrypted Key :");
			sprintf(szDebugMsg, "%s", szAscii);
			inLogPrintf(AT, szDebugMsg);
		
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inNCCC_TMK_Write_PINKey END()！");
			inLogPrintf(AT, szDebugMsg);
			inLogPrintf(AT, "------------------------------------------------------------------");
		}
		
		return (VS_ERROR);
	}
	else
	{
		/* 成功 */
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inNCCC_TMK_Write_PINKey END()！");
			inLogPrintf(AT, szDebugMsg);
			inLogPrintf(AT, "------------------------------------------------------------------");
		}
		
		return (VS_SUCCESS);
	}
}

/*
Function        :inNCCC_TMK_Write_MACKey
Date&Time       :2016/2/16 上午 9:58
Describe        :
 *para.Version:				Structure Format Version，可填入0x00或0x01
 *para.Info.KeySet:			key set(一個key set只能有一種key type)
 *para.Info.KeyIndex:			key index
 *para.Info.KeyType:			key之後會用在哪一種加密方法
 *para.Info.KeyVersion:			使用者自訂，用來管理key的版本，基本上無用
 *para.Info.KeyAttribute:		若有多個屬性，用"or"把bit on起來
 *Para.Protection.CipherKeySet		KetProtectionKey的KeySet
 *Para.Protection.CipherKeyIndex	KetProtectionKey的KeyIndex
 *para.Protection.Mode:			key寫進去時的加密狀態（KMS2_KEYPROTECTIONMODE_KPK_ECB 表示ECB寫入）
 *usTMKindex:				用第幾把Master Key
 *usMACKeyLen:				MACKey MACKey HeX長度	
 *szMACKeyAscii				MACKey Ascii形式
 *szKeyCheckValueAscii			KeyCheckValue Ascii形式
*/
int inNCCC_TMK_Write_MACKey(unsigned short usTMKindex, unsigned short usMACKeyLen, char *szMACKeyAscii, char* szKeyCheckValueAscii)
{
	char			szDebugMsg[100 + 1];
	char			szAscii[64 + 1];
	unsigned char		uszMACKeyHex[24 + 1];		/* 3DES最長24BYTE */
	unsigned char		uszKeyCheckValueHex[4 + 1];	/* KeyCheckValue 3byte 求偶數加1byte */
	unsigned short		usKeyCheckValueLength;
	unsigned short		usReturnValue;
	CTOS_KMS2KEYWRITE_PARA	srKeyWritePara;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "------------------------------------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_TMK_Write_MACKey START()！");
		inLogPrintf(AT, szDebugMsg);
	}
	
	memset(&srKeyWritePara, 0x00, sizeof(CTOS_KMS2KEYWRITE_PARA));
	memset(&uszMACKeyHex, 0x00, sizeof(uszMACKeyHex));
	memset(&usKeyCheckValueLength, 0x00, sizeof(usKeyCheckValueLength));
	memset(uszKeyCheckValueHex, 0x00, sizeof(uszKeyCheckValueHex));
	
	if (strlen(szKeyCheckValueAscii) > 0)
	{
		/* Key Check Value Length*/
		usKeyCheckValueLength = strlen(szKeyCheckValueAscii) / 2;		/* CheckValue 是key來加密全0 Block之後密文的前6位字母數字(3byte hex)，所以hardcode */
		/* 將key check value轉成HEX格式 */
		inFunc_ASCII_to_BCD(uszKeyCheckValueHex, szKeyCheckValueAscii, usKeyCheckValueLength);
	}
	/* 將key轉成HEX格式 */
	inFunc_ASCII_to_BCD(uszMACKeyHex, szMACKeyAscii, usMACKeyLen);
	
	/* 將PIN KEY所需參數放入結構中 */
	srKeyWritePara.Version = 0x01;
	srKeyWritePara.Info.KeySet = _TWK_KEYSET_NCCC_;
	srKeyWritePara.Info.KeyIndex = _TWK_KEYINDEX_NCCC_MAC_;
	srKeyWritePara.Info.KeyType = KMS2_KEYTYPE_3DES;
	srKeyWritePara.Info.KeyVersion = 0x01;
	srKeyWritePara.Info.KeyAttribute = KMS2_KEYATTRIBUTE_MAC | KMS2_KEYATTRIBUTE_ENCRYPT;
	srKeyWritePara.Protection.Mode = KMS2_KEYPROTECTIONMODE_KPK_ECB;
	srKeyWritePara.Protection.CipherKeySet = _TMK_KEYSET_NCCC_;
	srKeyWritePara.Protection.CipherKeyIndex = _TMK_KEYINDEX_NCCC_ + (usTMKindex - 1);	/* 0x0000放第一把key，0x0001放第二把key，所以減一 */
	srKeyWritePara.Value.KeyLength = usMACKeyLen;
	srKeyWritePara.Value.pKeyData = uszMACKeyHex;
	
	if (usKeyCheckValueLength > 0)
	{
		srKeyWritePara.Verification.Method = KMS2_KEYVERIFICATIONMETHOD_DEFAULT;
		srKeyWritePara.Verification.KeyCheckValueLength = usKeyCheckValueLength;
		srKeyWritePara.Verification.pKeyCheckValue = uszKeyCheckValueHex;
	}
	
	/* Write MAC Key*/
	usReturnValue = inKMS_Write(&srKeyWritePara);
	
	if (usReturnValue != VS_SUCCESS)
	{	
		/* 失敗 */
		if (ginDebug == VS_TRUE)
		{
			/* 被加密的working key */
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, srKeyWritePara.Value.pKeyData, srKeyWritePara.Value.KeyLength);
			inLogPrintf(AT, "Failed Encrypted Key :");
			sprintf(szDebugMsg, "%s", szAscii);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inNCCC_TMK_Write_MACKey END()！");
			inLogPrintf(AT, szDebugMsg);
			inLogPrintf(AT, "------------------------------------------------------------------");
		}
		
		return (VS_ERROR);
	}
	else
	{
		/* 成功 */
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inNCCC_TMK_Write_MACKey END()！");
			inLogPrintf(AT, szDebugMsg);
			inLogPrintf(AT, "------------------------------------------------------------------");
		}
		
		return (VS_SUCCESS);
	}
	
}

/*
 Function	:inNCCC_TMK_Write_ESCKey
 Date&Time	:2016/4/21 上午 9:54
 Describe	:寫ESCKey1的function
 *para.Version:			Structure Format Version，可填入0x00或0x01
 *para.Info.KeySet:		key set(一個key set只能有一種key type，EX:只能3DES or RSA)
 *para.Info.KeyIndex:		key index
 *para.Info.KeyType:		key之後會用在哪一種加密方法(EX:3DES or RSA......等)
 *para.Info.KeyVersion:		使用者自訂，用來管理key的版本，基本上無用
 *para.Info.KeyAttribute:	若有多個屬性，用"or"把bit on起來（KMS2_KEYATTRIBUTE_KPK表示是用來加密其他key的key）
 *para..Protection.Mode:	key寫進去時的加密狀態（KMS2_KEYPROTECTIONMODE_PLAINTEXT 表示明碼寫入）
 *Key1 = TID(末5) + InviceNum(3)
 *Key2 = BatchNum(2) + 交易時間 hhmm(4) + 金額有效位數前兩碼，兩碼以下左補0(2)
	註:金額欄位取輸入F_04有效位數之前2碼。F_04 未上傳本欄位預設值=00
	例：	F_04 = 000000000000  若金額為0元，右靠左補0，取得金額=00
		F_04 = 000000000100  若金額為個位數1元，右靠左補0，取得金額=01
		F_04 = 000000001200  若金額為十位數12元，取金額前2碼=12
		F_04 = 000000012300  若金額為百位數123元，取金額前2碼=12
 * Encryption Key(szKeyFull) = Key1 + Key2 + Key1
 * 
 *這裡比較特別的是key直接組成hex形式
 *例:填入的key值為139950140115214013995014
 *則原來可視的值為313339393530313430313135323134303133393935303134
*/
int inNCCC_TMK_Write_ESCKey(TRANSACTION_OBJECT *pobTran)
{
	int			inCnt = 0;		/* 記錄放到KEY的第幾位 */
	int			inCntAll = 0;		/* 記錄放到全部長度KEY的第幾位 */
	char			szDebugMsg[100 + 1];
	char			szTemplate[42 + 1];
	char			szKeyFull[24 + 1];	/* 放已組完全部長度的Key */
	char			szKeyTemp[8 + 1];
	unsigned char		uszHex[24 + 1];		/* 3DES的Key長度最長24byte，若只輸入16byte則k1,k2,k3中，k3 = k1 */
	CTOS_KMS2KEYWRITE_PARA	srKeyWritePara;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "------------------------------------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_TMK_Write_ESCKey START()！");
		inLogPrintf(AT, szDebugMsg);
	}
	
	memset(szKeyFull, 0x00, sizeof(szKeyFull));
	
	
	/* Key 1 =================================================== */
	/* 初始化 */
	inCnt = 0;
	memset(szKeyTemp, 0x00, sizeof(szKeyTemp));
	/* 1.TID (ESC Host) */
	/* 切換到ESC的TID */
	if (inNCCC_ESC_SwitchToESC_Host(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTerminalID(szTemplate);

	/* 末五碼 */
	memcpy(&szKeyTemp[inCnt], &szTemplate[strlen(szTemplate) - 5], 5);
	inCnt += 5;

	/* 回覆原Host */
        inLoadHDTRec(pobTran->srBRec.inHDTIndex);

	/* 2.Inv */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%03ld", pobTran->srBRec.lnOrgInvNum);

	memcpy(&szKeyTemp[inCnt], szTemplate, 3);
	inCnt += 3;

	/* 將key1放到Key的最前面 */
        memcpy(&szKeyFull[inCntAll], szKeyTemp, 8);
	inCntAll += 8;
	
	/* Key 2 =================================================== */
	/* 初始化 */
	inCnt = 0;
	memset(szKeyTemp, 0x00, sizeof(szKeyTemp));
	/* 3.BatchNum */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%06ld", pobTran->srBRec.lnBatchNum);

	memcpy(&szKeyTemp[inCnt], &szTemplate[4], 2);
	inCnt += 2;
	/* 4.hhmm */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s", pobTran->srBRec.szTime);

	memcpy(&szKeyTemp[inCnt], szTemplate, 4);
	inCnt += 4;

	/* 5.Amount */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (pobTran->srBRec.inCode == _TIP_)
		sprintf(szTemplate, "%02ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
	else
		sprintf(szTemplate, "%02ld", pobTran->srBRec.lnTxnAmount);

	memcpy(&szKeyTemp[inCnt], szTemplate, 2);
	inCnt += 2;
	
	/* 將key2放到Key的中間 */
        memcpy(&szKeyFull[inCntAll], szKeyTemp, 8);
	inCntAll += 8;
	
	/* Key 3 = Key 1 =================================================== */
	memcpy(&szKeyFull[inCntAll], szKeyFull, 8);
	inCnt += 8; 
	
	memset(&srKeyWritePara, 0x00, sizeof(srKeyWritePara));
	memset(&uszHex, 0x00, sizeof(uszHex));

	/* 將所需參數放入結構中 */
	srKeyWritePara.Version = 0x01;						
	srKeyWritePara.Info.KeySet = _TWK_KEYSET_NCCC_;
	srKeyWritePara.Info.KeyIndex = _TWK_KEYINDEX_NCCC_ESC_;
	srKeyWritePara.Info.KeyType = KMS2_KEYTYPE_3DES;
	srKeyWritePara.Info.KeyVersion = 0x01;					
	srKeyWritePara.Info.KeyAttribute = KMS2_KEYATTRIBUTE_ENCRYPT;		
	srKeyWritePara.Protection.Mode = KMS2_KEYPROTECTIONMODE_PLAINTEXT;	
	srKeyWritePara.Value.pKeyData = (unsigned char*)szKeyFull;
	srKeyWritePara.Value.KeyLength = strlen(szKeyFull);
	
	if (inKMS_Write(&srKeyWritePara) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_TMK_Write_ESCKey END()！");
		inLogPrintf(AT, szDebugMsg);
		inLogPrintf(AT, "------------------------------------------------------------------");
	}
	
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_ESC_3DES_Encrypt
Date&Time       :2016/4/21 上午 11:51
Describe        :以3DES ECB模式加密
 *para.Version:				Structure Format Version，可填入0x00或0x01
 *para.Protection.CipherKeySet		用來加密的KeySet
 *para.Protection.CipherKeyIndex	用來加密的KeyIndex
 *para.Protection.CipherMethod		加密的method是CBC
 *para.Protection.SK_Length		SK_Length = 0 表示不使用session key(需要用DUKPT)
 *para.ICV.Length			Initial Vector Length
 *para.ICV.pData			Initial Vector
 *para.Input.pData			要加密的資料
 *para.Output.pData			產生的MAC
 *inLength				剩下未處理的資料長度
 *inIndex				已處理的資料長度
 *szInitialVector			當次加密用的InitialVector
 *szPlaindata				被切成8bytes用來加密的資料塊
*/
int inNCCC_TMK_ESC_3DES_Encrypt(char* szInPlaindata, int inInPlaindataLen, char *szResult)
{
	int				inRetVal;
	char				szAscii[64 + 1];
	char				szDebugMsg[100 + 1];
	CTOS_KMS2DATAENCRYPT_PARA	srDataEncryptPara;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "------------------------------------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_TMK_ESC_3DES_Encrypt START()！");
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
	{
		char szDebugMsg[100 + 1];

		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char *)szInPlaindata, inInPlaindataLen);
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "%s", szAscii);
		inLogPrintf(AT, szDebugMsg);
	}
	
	memset(&srDataEncryptPara, 0x00, sizeof(CTOS_KMS2DATAENCRYPT_PARA));
	srDataEncryptPara.Version = 0x01;
	srDataEncryptPara.Protection.CipherKeySet = _TWK_KEYSET_NCCC_;
	srDataEncryptPara.Protection.CipherKeyIndex = _TWK_KEYINDEX_NCCC_ESC_;
	srDataEncryptPara.Protection.CipherMethod = KMS2_DATAENCRYPTCIPHERMETHOD_ECB;
	srDataEncryptPara.Protection.SK_Length = 0;
	
	srDataEncryptPara.Input.Length = inInPlaindataLen;
	srDataEncryptPara.Input.pData = (unsigned char*)szInPlaindata;
	srDataEncryptPara.Output.pData = (unsigned char*)szResult;
	inRetVal = inKMS_DataEncrypt(&srDataEncryptPara);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ESC_3DES_Encrypt Failed");
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ESC_3DES_Encrypt Success");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	
	if (ginDebug == VS_TRUE)
	{	
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_TMK_ESC_3DES_Encrypt END()！");
		inLogPrintf(AT, szDebugMsg);
		inLogPrintf(AT, "------------------------------------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_CalculatePINBlock
Date&Time       :2016/2/16 下午 1:35
Describe        :
 *para.Version:				Structure Format Version，可填入0x00或0x01
 *Para.PIN_Info.BlockType:		Block的type;
 *Para.PIN_Info.PINDigitMinLength:	PIN接受最短長度 只能4到12
 *Para.PIN_Info.PINDigitMaxLength:	PIN接受最長長度 只能4到12
 *Para.Protection.CipherKeySet		KetProtectionKey的KeySet
 *Para.Protection.CipherKeyIndex	KetProtectionKey的KeyIndex
 *para.Protection.Mode:			key寫進去時的加密狀態（KMS2_KEYPROTECTIONMODE_KPK_ECB 表示ECB寫入）
 *Para.AdditionalData.InLength:		This field is used as the length of PAN if BlockType is KMS2_PINBLOCKTYPE_ANSI_X9_8_ISO_0.
 *Para.AdditionalData.pInData:		This field is used as the PAN data if BlockType is KMS2_PINBLOCKTYPE_ANSI_X9_8_ISO_0.Note that for PAN data, it shall also contain the last check digit.
 *Para.PINOutput.EncryptedBlockLength:	加密後的block長度（一般為8個byte）
 *Para.PINOutput.pEncryptedBlock:	output，加密後的PINBlock
 *Para.Control.Timeout:			時間到沒輸入PIN回傳d_KMS2_GET_PIN_TIMEOUT
 *Para.Control.AsteriskPositionX:	米字出現在第幾行
 *Para.Control.AsteriskPositionY:	米字出現在第幾列
 *Para.Control.NULLPIN:			可不可以輸入空值當PIN
 *Para.Control.piTestCancel:		可加入輸入PIN時額外的call back function
*/
int inNCCC_TMK_CalculatePINBlock(TRANSACTION_OBJECT* pobTran, char *szOutputPINBlock)
{
	char			szDebugMsg[100 + 1] = {0};
	char			szASCII[42 + 1] = {0};
	char			szTimeOut[3 + 1] = {0};
	char			szAmountMsg[_DISP_MSG_SIZE_ + 1] = {0};
	char			szDemoMode[2 + 1] = {0};
	unsigned int		uiTimeOut = 0;
	unsigned short		usReturnValue = 0;
	CTOS_KMS2PINGET_PARA	srPINGetPara;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "------------------------------------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_TMK_CalculatePINBlock START()！");
		inLogPrintf(AT, szDebugMsg);
	}
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_CUP_GET_PASSWORD_IN_, 0, _COORDINATE_Y_LINE_8_4_);
	
	/* 顯示金額 */
	memset(szAmountMsg, 0x00, sizeof(szAmountMsg));
	if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && 
	    (pobTran->srBRec.inOrgCode != _REFUND_ && pobTran->srBRec.inOrgCode != _INST_REFUND_ && pobTran->srBRec.inOrgCode != _REDEEM_REFUND_ && pobTran->srBRec.inOrgCode != _CUP_REFUND_ && pobTran->srBRec.inOrgCode != _CUP_MAIL_ORDER_REFUND_))	|| 
	     pobTran->srBRec.inCode == _REFUND_			|| 
	     pobTran->srBRec.inCode == _INST_REFUND_		|| 
	     pobTran->srBRec.inCode == _REDEEM_REFUND_		||
	     pobTran->srBRec.inCode == _CUP_REFUND_		||
	     pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
	{
		sprintf(szAmountMsg, "%ld", 0 - pobTran->srBRec.lnTotalTxnAmount);
	}
	else
	{
		sprintf(szAmountMsg, "%ld", pobTran->srBRec.lnTotalTxnAmount);
	}
	inFunc_Amount_Comma(szAmountMsg, "NT$", ' ', _SIGNED_NONE_,  15, _PADDING_RIGHT_);
	inDISP_EnglishFont_Point_Color(szAmountMsg, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_RED_, _COLOR_WHITE_, 7);
		
	memset(&srPINGetPara, 0x00, sizeof(CTOS_KMS2PINGET_PARA));
	memset(szOutputPINBlock, 0x00, sizeof(szOutputPINBlock));
	
	/* 如果沒設定TimeOut，就用EDC.dat內的TimeOut */
	if (uiTimeOut <= 0)
	{
		memset(szTimeOut, 0x00, sizeof(szTimeOut));
		inGetCUPOnlinePINEntryTimeout(szTimeOut);
		uiTimeOut = atoi(szTimeOut);
	}
	
	srPINGetPara.Version = 0x01;
	srPINGetPara.PIN_Info.BlockType = KMS2_PINBLOCKTYPE_ANSI_X9_8_ISO_0;
	srPINGetPara.PIN_Info.PINDigitMinLength = 4;
	srPINGetPara.PIN_Info.PINDigitMaxLength = 12;
	
	srPINGetPara.Protection.CipherKeySet = _TWK_KEYSET_NCCC_;
	srPINGetPara.Protection.CipherKeyIndex = _TWK_KEYINDEX_NCCC_PIN_ONLINE_;
	srPINGetPara.Protection.CipherMethod = KMS2_PINCIPHERMETHOD_ECB;
	
	srPINGetPara.AdditionalData.InLength = strlen(pobTran->srBRec.szPAN);
	srPINGetPara.AdditionalData.pInData = (unsigned char*)pobTran->srBRec.szPAN;
	
	srPINGetPara.PINOutput.EncryptedBlockLength = 8;
	srPINGetPara.PINOutput.pEncryptedBlock = (unsigned char*)szOutputPINBlock;
	srPINGetPara.Control.Timeout = uiTimeOut;
	srPINGetPara.Control.AsteriskPositionX = 2;
	if (ginHalfLCD == VS_TRUE)
	{
		srPINGetPara.Control.AsteriskPositionY = 8;
	}
	else
	{
		srPINGetPara.Control.AsteriskPositionY = 15;
	}
	srPINGetPara.Control.NULLPIN = TRUE;						
	srPINGetPara.Control.piTestCancel = NULL;
	
	/* 嗶三聲 */
	inDISP_BEEP(3, 500);
	
	/* Get PIN */
	usReturnValue = CTOS_KMS2PINGet(&srPINGetPara);
	
	if (ginDebug == VS_TRUE)
	{
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		memset(szASCII, 0x00, sizeof(szASCII));
		inFunc_BCD_to_ASCII(szASCII, (unsigned char*)szOutputPINBlock, 8);
		sprintf(szDebugMsg, "PINBlock: %s", szASCII);
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		/* 輸入PIN，demo版除了取消或timeout其他都要算成功 */
		if (usReturnValue == d_KMS2_GET_PIN_TIMEOUT)
		{
			/* TimeOut */
			if (ginDebug == VS_TRUE)
			{	
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Get PINBlock TimeOut. 代碼：0x%04X", usReturnValue);
				inLogPrintf(AT, szDebugMsg);

				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "inNCCC_TMK_CalculatePINBlock END()！");
				inLogPrintf(AT, szDebugMsg);
				inLogPrintf(AT, "------------------------------------------------------------------");
			}

			return (VS_TIMEOUT);
		}
		else if (usReturnValue == d_KMS2_GET_PIN_ABORT)
		{
			/* 失敗 */
			if (ginDebug == VS_TRUE)
			{	
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Get PINBlock Canceled. 代碼：0x%04X", usReturnValue);
				inLogPrintf(AT, szDebugMsg);

				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "inNCCC_TMK_CalculatePINBlock END()！");
				inLogPrintf(AT, szDebugMsg);
				inLogPrintf(AT, "------------------------------------------------------------------");
			}

			return (VS_USER_CANCEL);
		}
		else
		{
			return (VS_SUCCESS);
		}
	}
	else
	{
		if (usReturnValue == d_KMS2_GET_PIN_TIMEOUT)
		{
			/* TimeOut */
			if (ginDebug == VS_TRUE)
			{	
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Get PINBlock TimeOut. 代碼：0x%04X", usReturnValue);
				inLogPrintf(AT, szDebugMsg);

				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "inNCCC_TMK_CalculatePINBlock END()！");
				inLogPrintf(AT, szDebugMsg);
				inLogPrintf(AT, "------------------------------------------------------------------");
			}

			return (VS_TIMEOUT);
		}
		else if ((usReturnValue == d_KMS2_GET_PIN_NULL_PIN && srPINGetPara.Control.NULLPIN == TRUE))
		{
			/* 顯示金額 */
			memset(szAmountMsg, 0x00, sizeof(szAmountMsg));
			if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && 
			    (pobTran->srBRec.inOrgCode != _REFUND_ && pobTran->srBRec.inOrgCode != _INST_REFUND_ && pobTran->srBRec.inOrgCode != _REDEEM_REFUND_ && pobTran->srBRec.inOrgCode != _CUP_REFUND_ && pobTran->srBRec.inOrgCode != _CUP_MAIL_ORDER_REFUND_))	|| 
			     pobTran->srBRec.inCode == _REFUND_			|| 
			     pobTran->srBRec.inCode == _INST_REFUND_		|| 
			     pobTran->srBRec.inCode == _REDEEM_REFUND_		||
			     pobTran->srBRec.inCode == _CUP_REFUND_		||
			     pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szAmountMsg, "%ld", 0 - pobTran->srBRec.lnTotalTxnAmount);
			}
			else
			{
				sprintf(szAmountMsg, "%ld", pobTran->srBRec.lnTotalTxnAmount);
			}
			inFunc_Amount_Comma(szAmountMsg, "NT$", ' ', _SIGNED_NONE_,  15, _PADDING_RIGHT_);
			inDISP_EnglishFont_Point_Color(szAmountMsg, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_RED_, _COLOR_WHITE_, 7);

			/* 成功 */
			if (ginDebug == VS_TRUE)
			{	
				inLogPrintf(AT, "PINBlock Bypass.");

				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "inNCCC_TMK_CalculatePINBlock END()！");
				inLogPrintf(AT, szDebugMsg);
				inLogPrintf(AT, "------------------------------------------------------------------");
			}

			return (VS_SUCCESS);
		}
		else if (usReturnValue == d_OK)
		{
			/* 顯示金額 */
			memset(szAmountMsg, 0x00, sizeof(szAmountMsg));
			if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && 
			    (pobTran->srBRec.inOrgCode != _REFUND_ && pobTran->srBRec.inOrgCode != _INST_REFUND_ && pobTran->srBRec.inOrgCode != _REDEEM_REFUND_ && pobTran->srBRec.inOrgCode != _CUP_REFUND_ && pobTran->srBRec.inOrgCode != _CUP_MAIL_ORDER_REFUND_))	|| 
			     pobTran->srBRec.inCode == _REFUND_			|| 
			     pobTran->srBRec.inCode == _INST_REFUND_		|| 
			     pobTran->srBRec.inCode == _REDEEM_REFUND_		||
			     pobTran->srBRec.inCode == _CUP_REFUND_		||
			     pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szAmountMsg, "%ld", 0 - pobTran->srBRec.lnTotalTxnAmount);
			}
			else
			{
				sprintf(szAmountMsg, "%ld", pobTran->srBRec.lnTotalTxnAmount);
			}
			inFunc_Amount_Comma(szAmountMsg, "NT$", ' ', _SIGNED_NONE_,  15, _PADDING_RIGHT_);
			inDISP_EnglishFont_Point_Color(szAmountMsg, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_RED_, _COLOR_WHITE_, 7);

			/* 成功 */
			if (ginDebug == VS_TRUE)
			{	
				inLogPrintf(AT, "Get PINBlock Success.");

				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "inNCCC_TMK_CalculatePINBlock END()！");
				inLogPrintf(AT, szDebugMsg);
				inLogPrintf(AT, "------------------------------------------------------------------");
			}

			return (VS_SUCCESS);
		}
		else
		{
			/* 失敗 */
			if (ginDebug == VS_TRUE)
			{	
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Get PINBlock Failed. 代碼：0x%04X", usReturnValue);
				inLogPrintf(AT, szDebugMsg);

				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "inNCCC_TMK_CalculatePINBlock END()！");
				inLogPrintf(AT, szDebugMsg);
				inLogPrintf(AT, "------------------------------------------------------------------");
			}

			return (VS_ERROR);
		}
	}
}

/*
Function        :inNCCC_TMK_CalculateMac
Date&Time       :2016/2/16 下午 4:46
Describe        :根據CBC的加密方式，需要將加密的資料分塊的第一塊與initial vector做加密，所得值再當作下一區塊的initial vector
 *para.Version:				Structure Format Version，可填入0x00或0x01
 *para.Protection.CipherKeySet		用來加密的KeySet
 *para.Protection.CipherKeyIndex	用來加密的KeyIndex
 *para.Protection.CipherMethod		加密的method是CBC
 *para.Protection.SK_Length		SK_Length = 0 表示不使用session key(需要用DUKPT)
 *para.ICV.Length			Initial Vector Length
 *para.ICV.pData			Initial Vector
 *para.Input.pData			要加密的資料
 *para.Output.pData			產生的MAC
 *inLength				剩下未處理的資料長度
 *inIndex				已處理的資料長度
 *szInitialVector			當次加密用的InitialVector
 *szPlaindata				被切成8bytes用來加密的資料塊
*/
int inNCCC_TMK_CalculateMac(char* szInPlaindata, char *szMACdata)
{
	int			i;
	int			inLength;
	int			inIndex;
	int			inDebugLen;
	char			szInitialVector[8 + 1];
	char			szPlaindata[8 + 1];
	char			szDebugMsg[100 + 1];
	char			szASCII[42 + 1];
	unsigned short		usReturnValue;
	CTOS_KMS2MAC_PARA	srMACpara;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "------------------------------------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_TMK_CalculateMac START()！");
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inDebugLen = strlen(szInPlaindata);
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "GenMACDATA: lenth: %d", inDebugLen);
		inLogPrintf(AT, szDebugMsg);
		
		/* ADPU command 太長 */
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		for (i = 0; inDebugLen > 0; i++)
		{
			if (inDebugLen > 40)
			{
				memcpy(szDebugMsg, &szInPlaindata[i * 40], 40);
				inLogPrintf(AT, szDebugMsg);
				memset(szDebugMsg, 0x00, 40);
				inDebugLen -= 40;
			}
			else
			{
				memcpy(szDebugMsg, &szInPlaindata[i * 40], inDebugLen);
				inLogPrintf(AT, szDebugMsg);
				memset(szDebugMsg, 0x00, inDebugLen);
				inDebugLen -= inDebugLen;
			}
			
		}
	}
	
	memset(szInitialVector, 0x00, sizeof(szInitialVector));
	memset(szPlaindata, 0x00, sizeof(szPlaindata));
	memset(&srMACpara, 0x00, sizeof(CTOS_KMS2MAC_PARA));
	
	/*未處理資料長度*/
	inLength = strlen(szInPlaindata);
	/*已處理資料長度*/
	inIndex = 0;
	
	srMACpara.Version = 0x01;
	srMACpara.Protection.CipherKeySet = _TWK_KEYSET_NCCC_;
	srMACpara.Protection.CipherKeyIndex = _TWK_KEYINDEX_NCCC_MAC_;
	srMACpara.Protection.CipherMethod = KMS2_MACMETHOD_CBC;
	srMACpara.Protection.SK_Length = 0;
	srMACpara.ICV.Length = 8;
	srMACpara.ICV.pData = (unsigned char*)szInitialVector;
	
	srMACpara.Input.pData = (unsigned char*)szPlaindata;
	srMACpara.Output.pData = (unsigned char*)szMACdata;
	
	/* 只要還有沒處理的資料，就繼續做 */
	while (inLength > 0)
	{
		/* 最後一次長度不足8時，補0x00至8位，所以最後一次還是抓8個bytes */
		memcpy(szPlaindata, &szInPlaindata[inIndex], 8);
		srMACpara.Input.Length = strlen(szPlaindata);
		
		/* calculate MAC*/
		usReturnValue = CTOS_KMS2MAC(&srMACpara);
		
		inIndex += strlen(szPlaindata);
		inLength -= strlen(szPlaindata);
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "------------------------------------------------------------------");
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			memset(szASCII, 0x00, sizeof(szASCII));
			inFunc_BCD_to_ASCII(szASCII, (unsigned char*)szInitialVector, 8);
			sprintf(szDebugMsg, "InitialVector: %s", szASCII);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "PlainDATA: %s", szPlaindata);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "PlainDATALength: %d", strlen(szPlaindata));
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			memset(szASCII, 0x00, sizeof(szASCII));
			inFunc_BCD_to_ASCII(szASCII, (unsigned char*)szMACdata, 8);
			sprintf(szDebugMsg, "MACDATA: %s", szASCII);
			inLogPrintf(AT, szDebugMsg);
			
			inLogPrintf(AT, "------------------------------------------------------------------");
		}
		
		if (usReturnValue != d_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Calculate MAC failed ret = 0x%04X", usReturnValue);
				inLogPrintf(AT, szDebugMsg);
				
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "inNCCC_TMK_CalculateMac END()！");
				inLogPrintf(AT, szDebugMsg);
				inLogPrintf(AT, "------------------------------------------------------------------");
			}
			
			return (VS_ERROR);
		}
		else
		{
			/* 繼續做 */
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Calculate MAC Success");
				inLogPrintf(AT, szDebugMsg);
				
			}
		}
		
		/* 每加密一次就把產生的值，當成新一次加密的Initial vector */
		memcpy(szInitialVector, szMACdata, 8);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		memset(szASCII, 0x00, sizeof(szASCII));
		inFunc_BCD_to_ASCII(szASCII, (unsigned char*)szMACdata, 8);
		sprintf(szDebugMsg, "MACDATA: %s", szASCII);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
	{	
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_TMK_CalculateMac END()！");
		inLogPrintf(AT, szDebugMsg);
		inLogPrintf(AT, "------------------------------------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_GetKeyInfo_LookUp_Default
Date&Time       :2017/3/28 下午 1:54
Describe        :查看現存15把MasterKey狀態
*usKeySet	:要調查的keyset
*usKeyIndex	:要調查的KeyIndex
*inCVLen	:Check Value Length	不用的話填0 Only used for KeyType 3DES/3DES-DUKPT/AES
*inHashAlgorithm:Hash 演算法		不用的話填0 Only used for KeyType RSA
*/
int inNCCC_TMK_GetKeyInfo_LookUp_Default()
{	
	int				inRetVal;
	int				inCVLen;
	int				inHashAlgorithm;
	char				szDebugMsg[100 + 1];
	char				szAscii[100 + 1];
	char				szDispMsg[50 + 1];
	char				szTemplate[50 + 1];
	char				szKeyCheckValue[6 + 1];
	char				szTest;
	unsigned short			usKeySet;
	unsigned short			usKeyIndex;
	CTOS_KMS2KEYGETINFO_PARA	srPara;
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	
	/* 預設Key 位置 */
	usKeySet = _TMK_KEYSET_NCCC_;
	usKeyIndex = _TMK_KEYINDEX_NCCC_;
	
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
	
	for (usKeyIndex  = _TMK_KEYINDEX_NCCC_;usKeyIndex < _TMK_KEYINDEX_NCCC_ + _KEY_TOTAL_COUNT_; usKeyIndex++)
	{
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
				inDISP_ChineseFont_Color("This is Test Key", _FONTSIZE_8X22_, _LINE_8_8_, _COLOR_RED_, _DISP_LEFT_);
			}

		}
		else if (inRetVal == d_KMS2_KEY_NOT_EXIST)
		{
			inDISP_ChineseFont_Color("Key Not Exist", _FONTSIZE_8X22_, _LINE_8_8_, _COLOR_RED_, _DISP_LEFT_);
		}
		else
		{
			/* 失敗 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		}
		
		uszKBD_GetKey(30);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_ProductionKey_Swap_To_Temp
Date&Time       :2017/1/13 下午 1:30
Describe        :將Production Key塞到後面的位置暫存
 */
int inNCCC_TMK_ProductionKey_Swap_To_Temp()
{
	int	i;
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	
	/* 因為Swap一定要兩個位置都有Key，所以檢查沒有Key就塞入全0預設Key */
	for (i = 0; i < _KEY_TOTAL_COUNT_; i++)
	{
		if (inKMS_CheckKey(_TMK_KEYSET_NCCC_, _TMK_KEYINDEX_NCCC_ + i) != VS_SUCCESS)
		{
			inKMS_Write_NULL_Key(_TMK_KEYSET_NCCC_, _TMK_KEYINDEX_NCCC_ + i);
		}
		
	}
	
	for (i = 0; i < _KEY_TOTAL_COUNT_; i++)
	{
		if (inKMS_CheckKey(_TMK_TEMP_KEYSET_NCCC_, _TMK_TEMP_KEYINDEX_NCCC_ + i) != VS_SUCCESS)
		{
			inKMS_Write_NULL_Key(_TMK_TEMP_KEYSET_NCCC_, _TMK_TEMP_KEYINDEX_NCCC_ + i);
		}
		
	}
	
	for (i = 0; i < _KEY_TOTAL_COUNT_; i++)
	{
		if (inKMS_Key_Swap(_TMK_KEYSET_NCCC_, _TMK_KEYINDEX_NCCC_ + i, _TMK_TEMP_KEYSET_NCCC_, _TMK_TEMP_KEYINDEX_NCCC_ + i) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		
	}
	
	inDISP_ChineseFont_Color("Key 交換完成", _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_RED_, _DISP_CENTER_);
	
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_Check_Test_Key
Date&Time       :2017/1/13 上午 11:30
Describe        :確認是否是測試Key，是測試Key則szTest填'Y'，不是測試Key填'N'，沒有key則為空(0x00)
*/
int inNCCC_TMK_Check_Test_Key(unsigned short usKeySet, unsigned short usKeyIndex, char *szTest)
{
	int				i;
	int				inRetVal;
	int				inCVLen = 6;
	int				inHashAlgorithm = 0;
	char				szKeyCheckValue[6 + 1];
	char				szAscii[12 + 1];
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
	*szTest = 0x00;
	
	inRetVal = inKMS_GetKeyInfo(&srPara);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	memset(szAscii, 0x00, sizeof(szAscii));
	inFunc_BCD_to_ASCII(szAscii, srPara.Output.pCV, srPara.Input.CVLen);
	
	for (i = 0; i < _KCV_TOTAL_COUNT_; i++)
	{
		if (memcmp(szAscii, TMK_KCV_TABLE[i].szKCV, strlen(szAscii)) == 0)
		{
			*szTest = 'Y';
			
			return (VS_SUCCESS);
		}
	}
	
	*szTest = 'N';
			
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_Select_AID
Date&Time       :2018/11/9 下午 4:07
Describe        :
*/
int inNCCC_TMK_Select_AID(TRANSACTION_OBJECT *pobTran)
{
	char		szDemoMode[2 + 1] = {0};
	unsigned char	uszInBuf[16 + 1] = {0};
	APDU_COMMAND	srAPDUData;
	
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		return (VS_SUCCESS);
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_TMK_Select_AID() START !");
	}
	if (ginISODebug == VS_TRUE)
	{
		inPRINT_ChineseFont("----------------------------------------", _PRT_ISO_);
		inPRINT_ChineseFont("inNCCC_TMK_Select_AID() START !", _PRT_ISO_);
	}
	
	memset(&srAPDUData, 0x00, sizeof(APDU_COMMAND));
	inFISC_PowerON(pobTran);

	memset(uszInBuf, 0x00, sizeof(uszInBuf));
	memcpy(uszInBuf, _TMK_KEY_CARD_AID_, 8);
	
	srAPDUData.uszCommandCLAData[0] = _NCCC_TMK_KEY_CARD_SELECT_AID_CLA_COMMAND_;	/* CLA */
	srAPDUData.uszCommandINSData[0] = _NCCC_TMK_KEY_CARD_SELECT_AID_INS_COMMAND_;	/* INS */
	srAPDUData.uszCommandP1Data[0] = _NCCC_TMK_KEY_CARD_SELECT_AID_P1_COMMAND_;	/* P1 */
	srAPDUData.uszCommandP2Data[0] = _NCCC_TMK_KEY_CARD_SELECT_AID_P2_COMMAND_;	/* P2 */
	memcpy(&srAPDUData.uszCommandData[0], _TMK_KEY_CARD_AID_, _NCCC_TMK_KEY_CARD_SELECT_AID_LC_SIZE_);
	srAPDUData.inCommandDataLen = _NCCC_TMK_KEY_CARD_SELECT_AID_LC_SIZE_;

	if (inAPDU_BuildAPDU(&srAPDUData) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "------------------------------------------");
			inLogPrintf(AT, "inNCCC_TMK_Select_AID_ERROR");
			inLogPrintf(AT, " Read Fail ");
			inLogPrintf(AT, "------------------------------------------");
		}
		if (ginISODebug == VS_TRUE)
		{
			inPRINT_ChineseFont("------------------------------------------", _PRT_ISO_);
			inPRINT_ChineseFont("inNCCC_TMK_Select_AID_ERROR", _PRT_ISO_);
			inPRINT_ChineseFont(" Read Fail ", _PRT_ISO_);
			inPRINT_ChineseFont("------------------------------------------", _PRT_ISO_);
		}
		
		inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);
		
		return (VS_ERROR);
        }

	inAPDU_APDUTransmit_Flow(pobTran, &srAPDUData);
	if (srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2] == 0x90	&&
	    srAPDUData.uszRecevData[srAPDUData.inRecevLen - 1] == 0x00)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "APDU ");
			inLogPrintf(AT, "[%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X]",	srAPDUData.uszRecevData[0],
													srAPDUData.uszRecevData[1],
													srAPDUData.uszRecevData[2],
													srAPDUData.uszRecevData[3],
													srAPDUData.uszRecevData[4],
													srAPDUData.uszRecevData[5],
													srAPDUData.uszRecevData[6],
													srAPDUData.uszRecevData[7],
													srAPDUData.uszRecevData[8],
													srAPDUData.uszRecevData[9],
													srAPDUData.uszRecevData[10]);
			inLogPrintf(AT, "[%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X]",	srAPDUData.uszRecevData[11],
													srAPDUData.uszRecevData[12],
													srAPDUData.uszRecevData[13],
													srAPDUData.uszRecevData[14],
													srAPDUData.uszRecevData[15],
													srAPDUData.uszRecevData[16],
													srAPDUData.uszRecevData[17],
													srAPDUData.uszRecevData[18],
													srAPDUData.uszRecevData[19],
													srAPDUData.uszRecevData[20],
													srAPDUData.uszRecevData[21]);
		}
		if (ginISODebug == VS_TRUE)
		{
			char	szDebugMsg[100 + 1] = {0};
			
			inPRINT_ChineseFont("APDU ", _PRT_ISO_);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "[%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X]",	srAPDUData.uszRecevData[0],
													srAPDUData.uszRecevData[1],
													srAPDUData.uszRecevData[2],
													srAPDUData.uszRecevData[3],
													srAPDUData.uszRecevData[4],
													srAPDUData.uszRecevData[5],
													srAPDUData.uszRecevData[6],
													srAPDUData.uszRecevData[7],
													srAPDUData.uszRecevData[8],
													srAPDUData.uszRecevData[9],
													srAPDUData.uszRecevData[10]);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "[%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X]",	srAPDUData.uszRecevData[11],
													srAPDUData.uszRecevData[12],
													srAPDUData.uszRecevData[13],
													srAPDUData.uszRecevData[14],
													srAPDUData.uszRecevData[15],
													srAPDUData.uszRecevData[16],
													srAPDUData.uszRecevData[17],
													srAPDUData.uszRecevData[18],
													srAPDUData.uszRecevData[19],
													srAPDUData.uszRecevData[20],
													srAPDUData.uszRecevData[21]);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
			
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "------------------------------------------");
			inLogPrintf(AT, "inNCCC_TMK_SelectAID()");
			inLogPrintf(AT, "ERR LOG [%02X][%02X]", srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2], srAPDUData.uszRecevData[srAPDUData.inRecevLen - 1]);
			inLogPrintf(AT, " AID error ");
			inLogPrintf(AT, "------------------------------------------");
		}
		if (ginISODebug == VS_TRUE)
		{
			char	szDebugMsg[100 + 1] = {0};
			
			inPRINT_ChineseFont("------------------------------------------", _PRT_ISO_);
			inPRINT_ChineseFont("inNCCC_TMK_SelectAID()", _PRT_ISO_);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ERR LOG [%02X][%02X]", srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2], srAPDUData.uszRecevData[srAPDUData.inRecevLen - 1]);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
			inPRINT_ChineseFont(" AID error ", _PRT_ISO_);
			inPRINT_ChineseFont("------------------------------------------", _PRT_ISO_);
		}

		inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);
        	return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_TMK_SelectAID() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	if (ginISODebug == VS_TRUE)
	{
		inPRINT_ChineseFont("inNCCC_TMK_SelectAID() END !", _PRT_ISO_);
		inPRINT_ChineseFont("----------------------------------------", _PRT_ISO_);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_GetAppletInfo
Date&Time       :2018/11/12 下午 1:27
Describe        :
*/
int inNCCC_TMK_GetAppletInfo(TRANSACTION_OBJECT *pobTran)
{
	char		szDemoMode[2 + 1] = {0};
	APDU_COMMAND	srAPDUData;
	
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		return (VS_SUCCESS);
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_TMK_GetAppletInfo() START !");
	}
	if (ginISODebug == VS_TRUE)
	{
		inPRINT_ChineseFont("----------------------------------------", _PRT_ISO_);
		inPRINT_ChineseFont("inNCCC_TMK_GetAppletInfo() START !", _PRT_ISO_);
	}
	
	memset(&srAPDUData, 0x00, sizeof(APDU_COMMAND));

	srAPDUData.uszCommandCLAData[0] = _NCCC_TMK_KEY_CARD_GET_APPLET_INFO_CLA_COMMAND_;	/* INS */
	srAPDUData.uszCommandINSData[0] = _NCCC_TMK_KEY_CARD_GET_APPLET_INFO_INS_COMMAND_;	/* INS */
	srAPDUData.uszCommandP1Data[0] = _NCCC_TMK_KEY_CARD_GET_APPLET_INFO_P1_COMMAND_;	/* P1 */
	srAPDUData.uszCommandP2Data[0] = _NCCC_TMK_KEY_CARD_GET_APPLET_INFO_P2_COMMAND_;	/* P2 */
	srAPDUData.inCommandDataLen = 0;

	if (inAPDU_BuildAPDU(&srAPDUData) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "------------------------------------------");
			inLogPrintf(AT, "inNCCC_TMK_GetAppletInfo_ERROR");
			inLogPrintf(AT, " Read Fail ");
			inLogPrintf(AT, "------------------------------------------");
		}
		if (ginISODebug == VS_TRUE)
		{
			inPRINT_ChineseFont("------------------------------------------", _PRT_ISO_);
			inPRINT_ChineseFont("inNCCC_TMK_GetAppletInfo_ERROR", _PRT_ISO_);
			inPRINT_ChineseFont(" Read Fail ", _PRT_ISO_);
			inPRINT_ChineseFont("------------------------------------------", _PRT_ISO_);
		}
		
		inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);
		
		return (VS_ERROR);
        }

	inAPDU_APDUTransmit_Flow(pobTran, &srAPDUData);
	if (srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2] == 0x90	&&
	    srAPDUData.uszRecevData[srAPDUData.inRecevLen - 1] == 0x00)
	{
		
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "------------------------------------------");
			inLogPrintf(AT, "inNCCC_TMK_GetAppletInfo()");
			inLogPrintf(AT, "ERR LOG [%02X][%02X]", srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2], srAPDUData.uszRecevData[srAPDUData.inRecevLen - 1]);
			inLogPrintf(AT, " GAI error ");
			inLogPrintf(AT, "------------------------------------------");
		}
		if (ginISODebug == VS_TRUE)
		{
			char	szDebugMsg[100 + 1] = {0};

			inPRINT_ChineseFont("------------------------------------------", _PRT_ISO_);
			inPRINT_ChineseFont("inNCCC_TMK_GetAppletInfo()", _PRT_ISO_);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ERR LOG [%02X][%02X]", srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2], srAPDUData.uszRecevData[srAPDUData.inRecevLen - 1]);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
			inPRINT_ChineseFont(" GAI error ", _PRT_ISO_);
			inPRINT_ChineseFont("------------------------------------------", _PRT_ISO_);
		}

		inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);
        	return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_TMK_GetAppletInfo() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	if (ginISODebug == VS_TRUE)
	{
		inPRINT_ChineseFont("inNCCC_TMK_GetAppletInfo() END !", _PRT_ISO_);
		inPRINT_ChineseFont("----------------------------------------", _PRT_ISO_);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_GetKeyInfo_MultiKey
Date&Time       :2018/11/12 下午 1:27
Describe        :
*/
int inNCCC_TMK_GetKeyInfo_MultiKey(TRANSACTION_OBJECT *pobTran)
{
	int		i = 0;
	int		inCnt = 0;
	int		inKeyCnt = 0;
	int		inTMK_KLC = 0;
	int		inTMK_KUC = 0;
	char		szDemoMode[2 + 1] = {0};
	APDU_COMMAND	srAPDUData;
	
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		return (VS_SUCCESS);
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_TMK_GetKeyInfo_MultiKey() START !");
	}
	if (ginISODebug == VS_TRUE)
	{
		inPRINT_ChineseFont("----------------------------------------", _PRT_ISO_);
		inPRINT_ChineseFont("inNCCC_TMK_GetKeyInfo_MultiKey() START !", _PRT_ISO_);
	}
	
	memset(&srAPDUData, 0x00, sizeof(APDU_COMMAND));

	srAPDUData.uszCommandCLAData[0] = _NCCC_TMK_KEY_CARD_GET_KEY_INFO_CLA_COMMAND_;			/* CLA */
	srAPDUData.uszCommandINSData[0] = _NCCC_TMK_KEY_CARD_GET_KEY_INFO_INS_COMMAND_;			/* INS */
	srAPDUData.uszCommandP1Data[0] = _NCCC_TMK_KEY_CARD_GET_KEY_INFO_P1_COMMAND_;			/* P1 */
	srAPDUData.uszCommandP2Data[0] = _NCCC_TMK_KEY_CARD_GET_KEY_INFO_MULTI_KEY_P2_COMMAND_;		/* P2 */
	srAPDUData.inCommandDataLen = 0;

	if (inAPDU_BuildAPDU(&srAPDUData) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "------------------------------------------");
			inLogPrintf(AT, "inNCCC_TMK_GetKeyInfo_MultiKey_ERROR");
			inLogPrintf(AT, " Read Fail ");
			inLogPrintf(AT, "------------------------------------------");
		}
		if (ginISODebug == VS_TRUE)
		{
			inPRINT_ChineseFont("------------------------------------------", _PRT_ISO_);
			inPRINT_ChineseFont("inNCCC_TMK_GetKeyInfo_MultiKey_ERROR", _PRT_ISO_);
			inPRINT_ChineseFont(" Read Fail ", _PRT_ISO_);
			inPRINT_ChineseFont("------------------------------------------", _PRT_ISO_);
		}
		inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);
		
		return (VS_ERROR);
        }

	inAPDU_APDUTransmit_Flow(pobTran, &srAPDUData);
	if (srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2] == 0x90	&&
	    srAPDUData.uszRecevData[srAPDUData.inRecevLen - 1] == 0x00)
	{
		inCnt = 0;
		memcpy(&gsrTMKdata.uszTMK_KGI[0], &srAPDUData.uszRecevData[inCnt], 1);
		inCnt++;
		memcpy(&gsrTMKdata.uszTMK_KCT[0], &srAPDUData.uszRecevData[inCnt], 1);
		inCnt++;
		inKeyCnt = gsrTMKdata.uszTMK_KCT[0];
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TMK_KGI : [%02X]", gsrTMKdata.uszTMK_KGI[0]);
			inLogPrintf(AT, "TMK_KCT : [%02X]", gsrTMKdata.uszTMK_KCT[0]);
		}
		if (ginISODebug == VS_TRUE)
		{
			char	szDebugMsg[100 + 1] = {0};
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "TMK_KGI : [%02X]", gsrTMKdata.uszTMK_KGI[0]);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "TMK_KCT : [%02X]", gsrTMKdata.uszTMK_KCT[0]);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
		}

		for (i = 0; i < inKeyCnt; i++)
		{
			memcpy(&gsrTMKdata.uszTMK_KID[i][0], &srAPDUData.uszRecevData[inCnt], 1);
			inCnt++;
			memcpy(&gsrTMKdata.uszTMK_KCC[i][0], &srAPDUData.uszRecevData[inCnt], 2);
			inCnt += 2;
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "TMK_KCC(%02d) :[%02X][%02X]", gsrTMKdata.uszTMK_KID[i][0], gsrTMKdata.uszTMK_KCC[i][0], gsrTMKdata.uszTMK_KCC[i][1]);
			}
			if (ginISODebug == VS_TRUE)
			{
				char	szDebugMsg[100 + 1] = {0};

				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "TMK_KCC(%02d) :[%02X][%02X]", gsrTMKdata.uszTMK_KID[i][0], gsrTMKdata.uszTMK_KCC[i][0], gsrTMKdata.uszTMK_KCC[i][1]);
				inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
			}
		}

		memcpy(&gsrTMKdata.uszTMK_KLC[0], &srAPDUData.uszRecevData[inCnt], 2);
		inCnt += 2;
		memcpy(&gsrTMKdata.uszTMK_KUC[0], &srAPDUData.uszRecevData[inCnt], 2);
		inCnt += 2;
		memcpy(&gsrTMKdata.uszOTP_KCC[0], &srAPDUData.uszRecevData[inCnt], 2);
		inCnt += 2;
		memcpy(&gsrTMKdata.uszOTP_KTC[0], &srAPDUData.uszRecevData[inCnt], 1);
		inCnt++;

		inTMK_KLC = gsrTMKdata.uszTMK_KLC[0] * 256 + gsrTMKdata.uszTMK_KLC[1];
		inTMK_KUC = gsrTMKdata.uszTMK_KUC[0] * 256 + gsrTMKdata.uszTMK_KUC[1];
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TMK_KLC : [%02X][%02X](%d)", gsrTMKdata.uszTMK_KLC[0], gsrTMKdata.uszTMK_KLC[1], inTMK_KLC);
			inLogPrintf(AT, "TMK_KUC : [%02X][%02X](%d)", gsrTMKdata.uszTMK_KUC[0], gsrTMKdata.uszTMK_KUC[1], inTMK_KUC);
			inLogPrintf(AT, "OTP_KCC : [%02X][%02X]", gsrTMKdata.uszOTP_KCC[0], gsrTMKdata.uszOTP_KCC[1]);
			inLogPrintf(AT, "OTP_KTC : [%02X]", gsrTMKdata.uszOTP_KTC[0]);
		}
		if (ginISODebug == VS_TRUE)
		{
			char	szDebugMsg[100 + 1] = {0};

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "TMK_KLC : [%02X][%02X](%d)", gsrTMKdata.uszTMK_KLC[0], gsrTMKdata.uszTMK_KLC[1], inTMK_KLC);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "TMK_KUC : [%02X][%02X](%d)", gsrTMKdata.uszTMK_KUC[0], gsrTMKdata.uszTMK_KUC[1], inTMK_KUC);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "OTP_KCC : [%02X][%02X]", gsrTMKdata.uszOTP_KCC[0], gsrTMKdata.uszOTP_KCC[1]);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "OTP_KTC : [%02X]", gsrTMKdata.uszOTP_KTC[0]);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
		}

//		/* 修改為不檢核機型 by Russell 2019/1/7 下午 5:55 */
//		if (gsrTMKdata.uszTMK_KGI[0] != _KEY_GROUP_ID_CEE_)
//		{
//			inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);
//
//			inLogPrintf(AT, "------------------------------------------");
//			inLogPrintf(AT, " 卡片與機型不符 ");
//			inLogPrintf(AT, "------------------------------------------");
//			
//			return (VS_ERROR);
//		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "------------------------------------------");
			inLogPrintf(AT, "inNCCC_TMK_GetKeyInfo_MultiKey()");
			inLogPrintf(AT, "ERR LOG [%02X][%02X]", srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2], srAPDUData.uszRecevData[srAPDUData.inRecevLen - 1]);
			inLogPrintf(AT, " GKI error ");
			inLogPrintf(AT, "------------------------------------------");
		}
		if (ginISODebug == VS_TRUE)
		{
			char	szDebugMsg[100 + 1] = {0};

			inPRINT_ChineseFont("------------------------------------------", _PRT_ISO_);
			inPRINT_ChineseFont("inNCCC_TMK_GetKeyInfo_MultiKey()", _PRT_ISO_);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ERR LOG [%02X][%02X]", srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2], srAPDUData.uszRecevData[srAPDUData.inRecevLen - 1]);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
			
			inPRINT_ChineseFont(" GKI error ", _PRT_ISO_);
			inPRINT_ChineseFont("------------------------------------------", _PRT_ISO_);
		}

		inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);
        	return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_TMK_GetKeyInfo_MultiKey() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	if (ginISODebug == VS_TRUE)
	{
		inPRINT_ChineseFont("inNCCC_TMK_GetKeyInfo_MultiKey() END !", _PRT_ISO_);
		inPRINT_ChineseFont("----------------------------------------", _PRT_ISO_);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_GenerateRandomNumber
Date&Time       :2018/11/12 下午 3:32
Describe        :
*/
int inNCCC_TMK_GenerateRandomNumber(TRANSACTION_OBJECT *pobTran)
{
	char		szDemoMode[2 + 1] = {0};
	APDU_COMMAND	srAPDUData;
	
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		return (VS_SUCCESS);
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_TMK_GenerateRandomNumber() START !");
	}
	if (ginISODebug == VS_TRUE)
	{
		inPRINT_ChineseFont("----------------------------------------", _PRT_ISO_);
		inPRINT_ChineseFont("inNCCC_TMK_GenerateRandomNumber() START !", _PRT_ISO_);
	}
	
	memset(&srAPDUData, 0x00, sizeof(APDU_COMMAND));

	srAPDUData.uszCommandCLAData[0] = _NCCC_TMK_KEY_CARD_GEN_RANDOM_NUM_CLA_COMMAND_;	/* CLA */
	srAPDUData.uszCommandINSData[0] = _NCCC_TMK_KEY_CARD_GEN_RANDOM_NUM_INS_COMMAND_;	/* INS */
	srAPDUData.uszCommandP1Data[0] = _NCCC_TMK_KEY_CARD_GEN_RANDOM_NUM_P1_COMMAND_;		/* P1 */
	srAPDUData.uszCommandP2Data[0] = _NCCC_TMK_KEY_CARD_GEN_RANDOM_NUM_P2_COMMAND_;		/* P2 */
	srAPDUData.inCommandDataLen = 0;

	if (inAPDU_BuildAPDU(&srAPDUData) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "------------------------------------------");
			inLogPrintf(AT, "inNCCC_TMK_GenerateRandomNumber_ERROR");
			inLogPrintf(AT, " Read Fail ");
			inLogPrintf(AT, "------------------------------------------");
		}
		if (ginISODebug == VS_TRUE)
		{
			inPRINT_ChineseFont("------------------------------------------", _PRT_ISO_);
			inPRINT_ChineseFont("inNCCC_TMK_GenerateRandomNumber_ERROR", _PRT_ISO_);
			inPRINT_ChineseFont(" Read Fail ", _PRT_ISO_);
			inPRINT_ChineseFont("------------------------------------------", _PRT_ISO_);
		}
		inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);
		
		return (VS_ERROR);
        }
	
	inAPDU_APDUTransmit_Flow(pobTran, &srAPDUData);
	if (srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2] == 0x90	&&
	    srAPDUData.uszRecevData[srAPDUData.inRecevLen - 1] == 0x00)
	{
		memset(gsrTMKdata.uszTMK_CRN, 0x00, sizeof(gsrTMKdata.uszTMK_CRN));
		memcpy(&gsrTMKdata.uszTMK_CRN[0], &srAPDUData.uszRecevData[0], 8);

		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TMK_CRN : [%02X %02X %02X %02X %02X %02X %02X %02X]", gsrTMKdata.uszTMK_CRN[0],
												   gsrTMKdata.uszTMK_CRN[1],
												   gsrTMKdata.uszTMK_CRN[2],
												   gsrTMKdata.uszTMK_CRN[3],
												   gsrTMKdata.uszTMK_CRN[4],
												   gsrTMKdata.uszTMK_CRN[5],
												   gsrTMKdata.uszTMK_CRN[6],
												   gsrTMKdata.uszTMK_CRN[7]);
		}
		if (ginISODebug == VS_TRUE)
		{
			char	szDebugMsg[100 + 1] = {0};

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "TMK_CRN : [%02X %02X %02X %02X %02X %02X %02X %02X]", gsrTMKdata.uszTMK_CRN[0],
												   gsrTMKdata.uszTMK_CRN[1],
												   gsrTMKdata.uszTMK_CRN[2],
												   gsrTMKdata.uszTMK_CRN[3],
												   gsrTMKdata.uszTMK_CRN[4],
												   gsrTMKdata.uszTMK_CRN[5],
												   gsrTMKdata.uszTMK_CRN[6],
												   gsrTMKdata.uszTMK_CRN[7]);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "------------------------------------------");
			inLogPrintf(AT, "inNCCC_TMK_GenerateRandomNumber()");
			inLogPrintf(AT, "ERR LOG [%02X][%02X]", srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2], srAPDUData.uszRecevData[srAPDUData.inRecevLen - 1]);
			inLogPrintf(AT, " GRN error ");
			inLogPrintf(AT, "------------------------------------------");
		}
		if (ginISODebug == VS_TRUE)
		{
			char	szDebugMsg[100 + 1] = {0};

			inPRINT_ChineseFont("------------------------------------------", _PRT_ISO_);
			inPRINT_ChineseFont("inNCCC_TMK_GenerateRandomNumber()", _PRT_ISO_);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ERR LOG [%02X][%02X]", srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2], srAPDUData.uszRecevData[srAPDUData.inRecevLen - 1]);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
			inPRINT_ChineseFont(" GRN error ", _PRT_ISO_);
			inPRINT_ChineseFont("------------------------------------------", _PRT_ISO_);
		}

		inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);
        	return (VS_ERROR);
	}
		
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_TMK_GenerateRandomNumber() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	if (ginISODebug == VS_TRUE)
	{
		inPRINT_ChineseFont("inNCCC_TMK_GenerateRandomNumber() END !", _PRT_ISO_);
		inPRINT_ChineseFont("----------------------------------------", _PRT_ISO_);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_VerifyOTP
Date&Time       :2018/11/12 下午 3:32
Describe        :
*/
int inNCCC_TMK_VerifyOTP(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	char		szDemoMode[2 + 1] = {0};
	char		szDispMsg[100 + 1] = {0};
	unsigned char	uszVerifyOTPBit = VS_FALSE;
	DISPLAY_OBJECT	srDispObj;
	APDU_COMMAND	srAPDUData;
	
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
//		return (VS_SUCCESS);
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_TMK_VerifyOTP() START !");
	}
	if (ginISODebug == VS_TRUE)
	{
		inPRINT_ChineseFont("----------------------------------------", _PRT_ISO_);
		inPRINT_ChineseFont("inNCCC_TMK_VerifyOTP() START !", _PRT_ISO_);
	}
	
	do
	{
		uszVerifyOTPBit = VS_FALSE;

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		memset(szDispMsg, 0x00, strlen(szDispMsg));
		sprintf(szDispMsg, "TMK CRN %s", gsrTMKdata.uszTMK_CRN);
		inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
		inDISP_ChineseFont("Pls Call", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		inDISP_ChineseFont("Pls Enter OTP", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
		
		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMaxLen = 8;
		srDispObj.inColor = _COLOR_RED_;
		srDispObj.inTouchSensorFunc = _Touch_OX_LINE8_8_;
		srDispObj.inTimeout = 300;
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
		
		if (inRetVal == VS_USER_CANCEL)
		{
			inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);
			
			return (inRetVal);
		}
		
		/* OTP未達8碼不合法 */
		if (strlen(srDispObj.szOutput) != 8)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "OTP under8 not valid");
			}
			if (ginISODebug == VS_TRUE)
			{
				inPRINT_ChineseFont("OTP under8 not valid", _PRT_ISO_);
			}
			
			uszVerifyOTPBit = VS_FALSE;
			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;
			inDISP_ChineseFont("not enough 8， pls ReEnter", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
			inDISP_Wait(1000);
			continue;
		}
		else
		{
			
		}

		memset(&srAPDUData, 0x00, sizeof(APDU_COMMAND));
		srAPDUData.uszCommandCLAData[0] = _NCCC_TMK_KEY_CARD_VERIFY_OTP_CLA_COMMAND_;	/* CLA */
		srAPDUData.uszCommandINSData[0] = _NCCC_TMK_KEY_CARD_VERIFY_OTP_INS_COMMAND_;	/* INS */
		srAPDUData.uszCommandP1Data[0] = _NCCC_TMK_KEY_CARD_VERIFY_OTP_P1_COMMAND_;	/* P1 */
		srAPDUData.uszCommandP2Data[0] = _NCCC_TMK_KEY_CARD_VERIFY_OTP_P2_COMMAND_;	/* P2 */
		memcpy(&srAPDUData.uszCommandData[0], srDispObj.szOutput, _NCCC_TMK_KEY_CARD_VERIFY_OTP_LC_SIZE_);
		srAPDUData.inCommandDataLen = _NCCC_TMK_KEY_CARD_VERIFY_OTP_LC_SIZE_;
		
		if (inAPDU_BuildAPDU(&srAPDUData) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "------------------------------------------");
				inLogPrintf(AT, "inNCCC_TMK_VerifyOTP_ERROR");
				inLogPrintf(AT, " Read Fail ");
				inLogPrintf(AT, "------------------------------------------");
			}
			if (ginISODebug == VS_TRUE)
			{
				inPRINT_ChineseFont("------------------------------------------", _PRT_ISO_);
				inPRINT_ChineseFont("inNCCC_TMK_VerifyOTP_ERROR", _PRT_ISO_);
				inPRINT_ChineseFont(" Read Fail ", _PRT_ISO_);
				inPRINT_ChineseFont("------------------------------------------", _PRT_ISO_);
			}
			inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);

			return (VS_ERROR);
		}
		
		inAPDU_APDUTransmit_Flow(pobTran, &srAPDUData);
		if (srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2] == 0x90	&&
		    srAPDUData.uszRecevData[srAPDUData.inRecevLen - 1] == 0x00)
		{
			uszVerifyOTPBit = VS_TRUE;
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "------------------------------------------");
				inLogPrintf(AT, "inNCCC_TMK_VerifyOTP()");
				inLogPrintf(AT, "ERR LOG [%02X][%02X]", srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2], srAPDUData.uszRecevData[srAPDUData.inRecevLen - 1]);
				inLogPrintf(AT, "------------------------------------------");
			}
			if (ginISODebug == VS_TRUE)
			{
				char	szDebugMsg[100 + 1] = {0};

				inPRINT_ChineseFont("------------------------------------------", _PRT_ISO_);
				inPRINT_ChineseFont("inNCCC_TMK_VerifyOTP()", _PRT_ISO_);
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "ERR LOG [%02X][%02X]", srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2], srAPDUData.uszRecevData[srAPDUData.inRecevLen - 1]);
				inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
				inPRINT_ChineseFont("------------------------------------------", _PRT_ISO_);
			}
			
			uszVerifyOTPBit = VS_FALSE;
			/* 以下在判斷驗證錯誤的次數 */
			if (srAPDUData.uszRecevData[0] == 0x66 && (srAPDUData.uszRecevData[1] == 0xCE ||
								    srAPDUData.uszRecevData[1] == 0xCD ||
								    srAPDUData.uszRecevData[1] == 0xCC ||
								    srAPDUData.uszRecevData[1] == 0xCB ||
								    srAPDUData.uszRecevData[1] == 0xCA ||
								    srAPDUData.uszRecevData[1] == 0xC9 ||
								    srAPDUData.uszRecevData[1] == 0xC8 ||
								    srAPDUData.uszRecevData[1] == 0xC7 ||
							       	    srAPDUData.uszRecevData[1] == 0xC6 ||
								    srAPDUData.uszRecevData[1] == 0xC5 ||
								    srAPDUData.uszRecevData[1] == 0xC4 ||
								    srAPDUData.uszRecevData[1] == 0xC3 ||
								    srAPDUData.uszRecevData[1] == 0xC2 ||
								    srAPDUData.uszRecevData[1] == 0xC1 ||
								    srAPDUData.uszRecevData[1] == 0xC0))
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("Verify fail", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
				inDISP_ChineseFont("Pls ReEnter", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
				inDISP_BEEP(3, 1000);
			}

		        if (srAPDUData.uszRecevData[0] == 0x66 && srAPDUData.uszRecevData[1] == 0x20)
		        {
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "Verify OverLimit");
					inLogPrintf(AT, "Lock Key Card");
				}
				if (ginISODebug == VS_TRUE)
				{
					inPRINT_ChineseFont("Verify OverLimit", _PRT_ISO_);
					inPRINT_ChineseFont("Lock Key Card", _PRT_ISO_);
				}
				
				inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);
				return (VS_ERROR);
			}
		}
		
		
		/* 如果驗證錯誤，驗證次數還沒歸【0】的時候，讓他重跑【OPT.txt】的流程 */
		if (uszVerifyOTPBit == VS_FALSE)
		{
			if (inFLOW_RunFunction(pobTran, _NCCC_TMK_SELECT_AID_) != VS_SUCCESS)
			{
				inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);
				return (VS_ERROR);
			}

			if (inFLOW_RunFunction(pobTran, _NCCC_TMK_GET_APPLET_INFO_) != VS_SUCCESS)
			{
				inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);
				return (VS_ERROR);
			}

			if (inFLOW_RunFunction(pobTran, _NCCC_TMK_GET_KEY_INFO_MULTI_KEY_) != VS_SUCCESS)
			{
				inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);
				return (VS_ERROR);
			}

			if (inFLOW_RunFunction(pobTran, _NCCC_TMK_GENERATE_RANDOM_NUMBER_) != VS_SUCCESS)
			{
				inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);
				return (VS_ERROR);
			}
		}
		
	} while (uszVerifyOTPBit == VS_FALSE);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_TMK_VerifyOTP() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	if (ginISODebug == VS_TRUE)
	{
		inPRINT_ChineseFont("inNCCC_TMK_VerifyOTP() END !", _PRT_ISO_);
		inPRINT_ChineseFont("----------------------------------------", _PRT_ISO_);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_GetTMK_Multi
Date&Time       :2018/11/16 下午 5:38
Describe        :
*/
int inNCCC_TMK_GetTMK_Multi(TRANSACTION_OBJECT *pobTran)
{
	int		i = 0;
	int		inCnt = 0;
	int		inKeyCnt = 0;
	int		inKeyLen = 0;
	int		inGET_KFC = 0;
	char		szDemoMode[2 + 1] = {0};
	APDU_COMMAND	srAPDUData;
	
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		gsrTMKdata.uszGET_KCT[0] = 15;
		
		i = 0;
		gsrTMKdata.uszGET_KID[i][0] = i + 1;
		gsrTMKdata.uszGET_KLEN[i][0] = 16;
		memcpy(&gsrTMKdata.uszTMK_DES_Ascii[i][0], _MULTI_TMK_DATA_1_, strlen(_MULTI_TMK_DATA_1_));
		
		i = 1;
		gsrTMKdata.uszGET_KID[i][0] = i + 1;
		gsrTMKdata.uszGET_KLEN[i][0] = 16;
		memcpy(&gsrTMKdata.uszTMK_DES_Ascii[i][0], _MULTI_TMK_DATA_2_, strlen(_MULTI_TMK_DATA_2_));
		
		i = 2;
		gsrTMKdata.uszGET_KID[i][0] = i + 1;
		gsrTMKdata.uszGET_KLEN[i][0] = 16;
		memcpy(&gsrTMKdata.uszTMK_DES_Ascii[i][0], _MULTI_TMK_DATA_3_, strlen(_MULTI_TMK_DATA_3_));
		
		i = 3;
		gsrTMKdata.uszGET_KID[i][0] = i + 1;
		gsrTMKdata.uszGET_KLEN[i][0] = 16;
		memcpy(&gsrTMKdata.uszTMK_DES_Ascii[i][0], _MULTI_TMK_DATA_4_, strlen(_MULTI_TMK_DATA_4_));
		
		i = 4;
		gsrTMKdata.uszGET_KID[i][0] = i + 1;
		gsrTMKdata.uszGET_KLEN[i][0] = 16;
		memcpy(&gsrTMKdata.uszTMK_DES_Ascii[i][0], _MULTI_TMK_DATA_5_, strlen(_MULTI_TMK_DATA_5_));
		
		i = 5;
		gsrTMKdata.uszGET_KID[i][0] = i + 1;
		gsrTMKdata.uszGET_KLEN[i][0] = 16;
		memcpy(&gsrTMKdata.uszTMK_DES_Ascii[i][0], _MULTI_TMK_DATA_6_, strlen(_MULTI_TMK_DATA_6_));
		
		i = 6;
		gsrTMKdata.uszGET_KID[i][0] = i + 1;
		gsrTMKdata.uszGET_KLEN[i][0] = 16;
		memcpy(&gsrTMKdata.uszTMK_DES_Ascii[i][0], _MULTI_TMK_DATA_7_, strlen(_MULTI_TMK_DATA_7_));
		
		i = 7;
		gsrTMKdata.uszGET_KID[i][0] = i + 1;
		gsrTMKdata.uszGET_KLEN[i][0] = 16;
		memcpy(&gsrTMKdata.uszTMK_DES_Ascii[i][0], _MULTI_TMK_DATA_8_, strlen(_MULTI_TMK_DATA_8_));
		
		i = 8;
		gsrTMKdata.uszGET_KID[i][0] = i + 1;
		gsrTMKdata.uszGET_KLEN[i][0] = 16;
		memcpy(&gsrTMKdata.uszTMK_DES_Ascii[i][0], _MULTI_TMK_DATA_9_, strlen(_MULTI_TMK_DATA_9_));
		
		i = 9;
		gsrTMKdata.uszGET_KID[i][0] = i + 1;
		gsrTMKdata.uszGET_KLEN[i][0] = 16;
		memcpy(&gsrTMKdata.uszTMK_DES_Ascii[i][0], _MULTI_TMK_DATA_10_, strlen(_MULTI_TMK_DATA_10_));
		
		i = 10;
		gsrTMKdata.uszGET_KID[i][0] = i + 1;
		gsrTMKdata.uszGET_KLEN[i][0] = 16;
		memcpy(&gsrTMKdata.uszTMK_DES_Ascii[i][0], _MULTI_TMK_DATA_11_, strlen(_MULTI_TMK_DATA_11_));
		
		i = 11;
		gsrTMKdata.uszGET_KID[i][0] = i + 1;
		gsrTMKdata.uszGET_KLEN[i][0] = 16;
		memcpy(&gsrTMKdata.uszTMK_DES_Ascii[i][0], _MULTI_TMK_DATA_12_, strlen(_MULTI_TMK_DATA_12_));
		
		i = 12;
		gsrTMKdata.uszGET_KID[i][0] = i + 1;
		gsrTMKdata.uszGET_KLEN[i][0] = 16;
		memcpy(&gsrTMKdata.uszTMK_DES_Ascii[i][0], _MULTI_TMK_DATA_13_, strlen(_MULTI_TMK_DATA_13_));
		
		i = 13;
		gsrTMKdata.uszGET_KID[i][0] = i + 1;
		gsrTMKdata.uszGET_KLEN[i][0] = 16;
		memcpy(&gsrTMKdata.uszTMK_DES_Ascii[i][0], _MULTI_TMK_DATA_14_, strlen(_MULTI_TMK_DATA_14_));
		
		i = 14;
		gsrTMKdata.uszGET_KID[i][0] = i + 1;
		gsrTMKdata.uszGET_KLEN[i][0] = 16;
		memcpy(&gsrTMKdata.uszTMK_DES_Ascii[i][0], _MULTI_TMK_DATA_15_, strlen(_MULTI_TMK_DATA_15_));
		
		return (VS_SUCCESS);
	}

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont("GET TMK ... ", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
	memset(&srAPDUData, 0x00, sizeof(APDU_COMMAND));
	srAPDUData.uszCommandCLAData[0] = _NCCC_TMK_KEY_CARD_GET_TMK_CLA_COMMAND_;		/* CLA */
	srAPDUData.uszCommandINSData[0] = _NCCC_TMK_KEY_CARD_GET_TMK_INS_COMMAND_;		/* INS */
	srAPDUData.uszCommandP1Data[0] = _NCCC_TMK_KEY_CARD_GET_TMK_P1_COMMAND_;		/* P1 */
	srAPDUData.uszCommandP2Data[0] = _NCCC_TMK_KEY_CARD_GET_TMK_MULTI_KEY_P2_COMMAND_;	/* P2 */
	srAPDUData.inCommandDataLen = 0;
	
	if (inAPDU_BuildAPDU(&srAPDUData) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "------------------------------------------");
			inLogPrintf(AT, "inNCCC_TMK_GetTMK_ERROR");
			inLogPrintf(AT, " Read Fail ");
			inLogPrintf(AT, "------------------------------------------");
		}
		if (ginISODebug == VS_TRUE)
		{
			inPRINT_ChineseFont("------------------------------------------", _PRT_ISO_);
			inPRINT_ChineseFont("inNCCC_TMK_GetTMK_ERROR", _PRT_ISO_);
			inPRINT_ChineseFont(" Read Fail ", _PRT_ISO_);
			inPRINT_ChineseFont("------------------------------------------", _PRT_ISO_);
		}
		inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);

		return (VS_ERROR);
	}
	
	/* 對卡片取得【TMK】 */
	inAPDU_APDUTransmit_Flow(pobTran, &srAPDUData);
	if (srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2] == 0x90	&&
	    srAPDUData.uszRecevData[srAPDUData.inRecevLen - 1] == 0x00)
	{
		inCnt = 0;
		memcpy(&gsrTMKdata.uszGET_KGI[0], &srAPDUData.uszRecevData[inCnt], 1);
		inCnt++;
		
		memcpy(&gsrTMKdata.uszGET_KCT[0], &srAPDUData.uszRecevData[inCnt], 1);
		inCnt++;
		inKeyCnt = gsrTMKdata.uszGET_KCT[0];
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_TMK_GetTMK()_KGI : [%02X]", gsrTMKdata.uszGET_KGI[0]);
		}
		if (ginISODebug == VS_TRUE)
		{
			char	szDebugMsg[100 + 1] = {0};

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inNCCC_TMK_GetTMK()_KGI : [%02X]", gsrTMKdata.uszGET_KGI[0]);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
		}
		
//		/* 修改為不檢核機型 by Russell 2019/1/7 下午 5:55 */
//		if (gsrTMKdata.uszGET_KGI[0] == _KEY_GROUP_ID_CEE_)
//		{
			memset(&gsrTMKdata.uszTMK_DES[0][0], 0x00, sizeof(gsrTMKdata.uszTMK_DES));
			
			for (i = 0; i < inKeyCnt; i++)
			{
				/* TMK Index */
				memcpy(&gsrTMKdata.uszGET_KID[i][0], &srAPDUData.uszRecevData[inCnt], 1);
				inCnt++;
				
				/* TMK KeyLength */
				memcpy((char*)&gsrTMKdata.uszGET_KLEN[i][0], &srAPDUData.uszRecevData[inCnt], 1);
				inKeyLen = gsrTMKdata.uszGET_KLEN[i][0];
				inCnt++;
				
				/* TMK DES */
				memcpy(&gsrTMKdata.uszTMK_DES[i][0], &srAPDUData.uszRecevData[inCnt], inKeyLen);
				inFunc_BCD_to_ASCII((char*)&gsrTMKdata.uszTMK_DES_Ascii[i][0], &gsrTMKdata.uszTMK_DES[i][0], inKeyLen);
				inCnt += inKeyLen;
				
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "TMK_KID(%02d): ", gsrTMKdata.uszGET_KID[i][0]);
					inLogPrintf(AT, "TMK_KLEN(%02d): ", gsrTMKdata.uszGET_KLEN[i][0]);
					inLogPrintf(AT, "TMK_DES_01-04 [%02X][%02X][%02X][%02X]", gsrTMKdata.uszTMK_DES[i][0], gsrTMKdata.uszTMK_DES[i][1], gsrTMKdata.uszTMK_DES[i][2], gsrTMKdata.uszTMK_DES[i][3]);
					inLogPrintf(AT, "TMK_DES_05-08 [%02X][%02X][%02X][%02X]", gsrTMKdata.uszTMK_DES[i][4], gsrTMKdata.uszTMK_DES[i][5], gsrTMKdata.uszTMK_DES[i][6], gsrTMKdata.uszTMK_DES[i][7]);
					
					if (inKeyLen > 8)
					{
						inLogPrintf(AT, "TMK_DES_09-12 [%02X][%02X][%02X][%02X]", gsrTMKdata.uszTMK_DES[i][8], gsrTMKdata.uszTMK_DES[i][9], gsrTMKdata.uszTMK_DES[i][10], gsrTMKdata.uszTMK_DES[i][11]);
						inLogPrintf(AT, "TMK_DES_13-16 [%02X][%02X][%02X][%02X]", gsrTMKdata.uszTMK_DES[i][12], gsrTMKdata.uszTMK_DES[i][13], gsrTMKdata.uszTMK_DES[i][14], gsrTMKdata.uszTMK_DES[i][15]);
					}
					if (inKeyLen > 16)
					{
						inLogPrintf(AT, "TMK_DES_17-20 [%02X][%02X][%02X][%02X]", gsrTMKdata.uszTMK_DES[i][16], gsrTMKdata.uszTMK_DES[i][17], gsrTMKdata.uszTMK_DES[i][18], gsrTMKdata.uszTMK_DES[i][19]);
						inLogPrintf(AT, "TMK_DES_21-24 [%02X][%02X][%02X][%02X]", gsrTMKdata.uszTMK_DES[i][20], gsrTMKdata.uszTMK_DES[i][21], gsrTMKdata.uszTMK_DES[i][22], gsrTMKdata.uszTMK_DES[i][23]);
					}
				}
				if (ginISODebug == VS_TRUE)
				{
					char	szDebugMsg[100 + 1] = {0};

					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "TMK_KID(%02d): ", gsrTMKdata.uszGET_KID[i][0]);
					inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
					
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "TMK_KLEN(%02d): ", gsrTMKdata.uszGET_KLEN[i][0]);
					inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
					
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "TMK_DES_01-04 [%02X][%02X][%02X][%02X]", gsrTMKdata.uszTMK_DES[i][0], gsrTMKdata.uszTMK_DES[i][1], gsrTMKdata.uszTMK_DES[i][2], gsrTMKdata.uszTMK_DES[i][3]);
					inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
					
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "TMK_DES_05-08 [%02X][%02X][%02X][%02X]", gsrTMKdata.uszTMK_DES[i][4], gsrTMKdata.uszTMK_DES[i][5], gsrTMKdata.uszTMK_DES[i][6], gsrTMKdata.uszTMK_DES[i][7]);
					inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
					
					if (inKeyLen > 8)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "TMK_DES_09-12 [%02X][%02X][%02X][%02X]", gsrTMKdata.uszTMK_DES[i][8], gsrTMKdata.uszTMK_DES[i][9], gsrTMKdata.uszTMK_DES[i][10], gsrTMKdata.uszTMK_DES[i][11]);
						inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
						
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "TMK_DES_13-16 [%02X][%02X][%02X][%02X]", gsrTMKdata.uszTMK_DES[i][12], gsrTMKdata.uszTMK_DES[i][13], gsrTMKdata.uszTMK_DES[i][14], gsrTMKdata.uszTMK_DES[i][15]);
						inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
					}
					if (inKeyLen > 16)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "TMK_DES_17-20 [%02X][%02X][%02X][%02X]", gsrTMKdata.uszTMK_DES[i][16], gsrTMKdata.uszTMK_DES[i][17], gsrTMKdata.uszTMK_DES[i][18], gsrTMKdata.uszTMK_DES[i][19]);
						inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
						
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "TMK_DES_21-24 [%02X][%02X][%02X][%02X]", gsrTMKdata.uszTMK_DES[i][20], gsrTMKdata.uszTMK_DES[i][21], gsrTMKdata.uszTMK_DES[i][22], gsrTMKdata.uszTMK_DES[i][23]);
						inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
					}
				}
			}

			/* 剩餘讀取次數 */
			memcpy(&gsrTMKdata.uszGET_KFC[0], &srAPDUData.uszRecevData[inCnt], 2);
			inGET_KFC = gsrTMKdata.uszGET_KFC[0] * 256 + gsrTMKdata.uszGET_KFC[1];
			inCnt += 2;
	
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inNCCC_TMK_GetTMK()_KFC : [%02X][%02X](%d)", gsrTMKdata.uszGET_KFC[0], gsrTMKdata.uszGET_KFC[1], inGET_KFC);
			}
			if (ginISODebug == VS_TRUE)
			{
				char	szDebugMsg[100 + 1] = {0};

				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "inNCCC_TMK_GetTMK()_KFC : [%02X][%02X](%d)", gsrTMKdata.uszGET_KFC[0], gsrTMKdata.uszGET_KFC[1], inGET_KFC);
				inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
			}
	
			memcpy(&gsrTMKdata.uszStatusWord[0], &srAPDUData.uszRecevData[inCnt], 2);
	
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inNCCC_TMK_GetTMK_Multi()_StatusWord : [%02X][%02X]", gsrTMKdata.uszStatusWord[0], gsrTMKdata.uszStatusWord[1]);
			}
			if (ginISODebug == VS_TRUE)
			{
				char	szDebugMsg[100 + 1] = {0};

				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "inNCCC_TMK_GetTMK_Multi()_StatusWord : [%02X][%02X]", gsrTMKdata.uszStatusWord[0], gsrTMKdata.uszStatusWord[1]);
				inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
			}
//		}
//		else
//		{
//			inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);
//			if (ginDebug == VS_TRUE)
//			{
//				inLogPrintf(AT, "卡片與機型不符");
//			}
//			return (VS_ERROR);
//		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "------------------------------------------");
			inLogPrintf(AT, "inNCCC_TMK_GetTMK_Multi");
			inLogPrintf(AT, "ERR LOG_1 [%02X][%02X]", srAPDUData.uszRecevData[0], srAPDUData.uszRecevData[1]);
			inLogPrintf(AT, "ERR LOG_2 [%02X][%02X]", srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2], srAPDUData.uszRecevData[srAPDUData.inRecevLen - 1]);
			inLogPrintf(AT, "------------------------------------------");
		}
		if (ginISODebug == VS_TRUE)
		{
			char	szDebugMsg[100 + 1] = {0};

			inPRINT_ChineseFont("------------------------------------------", _PRT_ISO_);
			inPRINT_ChineseFont("inNCCC_TMK_GetTMK_Multi", _PRT_ISO_);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ERR LOG_1 [%02X][%02X]", srAPDUData.uszRecevData[0], srAPDUData.uszRecevData[1]);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ERR LOG_2 [%02X][%02X]", srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2], srAPDUData.uszRecevData[srAPDUData.inRecevLen - 1]);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
			
			inPRINT_ChineseFont("------------------------------------------", _PRT_ISO_);
		}

//		/* 修改為不檢核機型 by Russell 2019/1/7 下午 5:55 */
//		if (gsrTMKdata.uszTMK_KGI[0] == _KEY_GROUP_ID_CEE_)
//		{
			if (srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2] == 0x66 && srAPDUData.uszRecevData[srAPDUData.inRecevLen - 1] == 0x00)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "Key Card no times");
				}
				
				if (ginISODebug == VS_TRUE)
				{
					inPRINT_ChineseFont("Key Card no times", _PRT_ISO_);
				}
				inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);
				
				return (VS_ERROR);
			}
//		}
//		else
//		{
//			if (ginDebug == VS_TRUE)
//			{
//				inLogPrintf(AT, "卡片與機型不符");
//			}
//			inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);
//			
//			return (VS_ERROR);
//		}

		inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TMK Read Fail");
		}
		if (ginISODebug == VS_TRUE)
		{
			inPRINT_ChineseFont("TMK Read Fail", _PRT_ISO_);
		}
		
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_Select_Device
Date&Time       :2019/1/8 下午 3:19
Describe        :選擇key要寫到哪裡
*/
int inNCCC_TMK_Select_Device(TRANSACTION_OBJECT *pobTran)
{
	int		inRemainTimes = 0;
	char		szTemplate[50 + 1] = {0};
	char		szNowComPort[4 + 1] = {0};
	unsigned char	uszKey = 0x00;
	unsigned char	uszDispChoiceBit = VS_FALSE;
        
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_TMK_Select_Device START!");
	
	memset(szNowComPort, 0x00, sizeof(szNowComPort));
	inGetECRComPort(szNowComPort);
	
	while (1)
	{
		if (uszDispChoiceBit != VS_TRUE)
		{
			uszDispChoiceBit = VS_TRUE;
			
			inRemainTimes = gsrTMKdata.uszGET_KFC[0] * 256 + gsrTMKdata.uszGET_KFC[1];
			inDISP_ClearAll();
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "Remain Times : %d", inRemainTimes);
			inDISP_ChineseFont("1.Terminal", _FONTSIZE_8X22_, _LINE_8_1_, _DISP_CENTER_);
			inDISP_ChineseFont("2.COM1", _FONTSIZE_8X22_, _LINE_8_2_, _DISP_CENTER_);
			inDISP_ChineseFont("3.COM2", _FONTSIZE_8X22_, _LINE_8_3_, _DISP_CENTER_);
//			inDISP_ChineseFont("4.COM3", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
//			inDISP_ChineseFont("5.USB1", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);
		}
		
		uszKey = uszKBD_Key();
		if (uszKey == _KEY_1_)
		{
			inFLOW_RunFunction(pobTran, _NCCC_TMK_WRITE_TMK_BY_KEYCARD_FLOW_);
			uszDispChoiceBit = VS_FALSE;
		}
		else if (uszKey == _KEY_2_)
		{
			if (memcmp(szNowComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) != 0)
			{
				inLoadEDCRec(0);
				inSetECRComPort(_COMPORT_COM1_);
				inSaveEDCRec(0);
				inECR_Load_TMK_DeInitial();
				inECR_Load_TMK_Initial();
				memset(szNowComPort, 0x00, sizeof(szNowComPort));
				sprintf(szNowComPort, _COMPORT_COM1_);
			}
			inDISP_ChineseFont("Send TMK...", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_CENTER_);
			inECR_Send_TMK(pobTran);
			
			uszDispChoiceBit = VS_FALSE;
		}
		else if (uszKey == _KEY_3_)
		{
			if (memcmp(szNowComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) != 0)
			{
				inLoadEDCRec(0);
				inSetECRComPort(_COMPORT_COM2_);
				inSaveEDCRec(0);
				inECR_Load_TMK_DeInitial();
				inECR_Load_TMK_Initial();
				memset(szNowComPort, 0x00, sizeof(szNowComPort));
				sprintf(szNowComPort, _COMPORT_COM2_);
			}
			inDISP_ChineseFont("Send TMK...", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_CENTER_);
			inECR_Send_TMK(pobTran);
			
			uszDispChoiceBit = VS_FALSE;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			break;
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_Test
Date&Time       :2016/2/16 下午 5:03
Describe        :以第十一把TMK來解PINkey和MAC Key
*/
int inNCCC_TMK_Test()
{	
	char			szTemplate[42 + 1];
	char			szKeyCheckValue[16 + 1];
	unsigned short		usTMKindex = 0x000B;
	unsigned short		usKeyLen = 32;
	
	
	inKMS_Initial();
	inNCCC_TMK_Write_Test_TMK_By_Terminal();
	
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, "3D96384729C1C60066CCE7AEEE590BB0", 32);
		memset(szKeyCheckValue, 0x00, sizeof(szKeyCheckValue));
		memcpy(szKeyCheckValue, "936A6B", 6);
		inNCCC_TMK_Write_PINKey(usTMKindex, usKeyLen/2, szTemplate, szKeyCheckValue);
	}
	
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, "F44719377CF40775EA9CCBB7524DB9E7", 32);
		memset(szKeyCheckValue, 0x00, sizeof(szKeyCheckValue));
		memcpy(szKeyCheckValue, "B8ABA8", 6);
		inNCCC_TMK_Write_MACKey(usTMKindex, usKeyLen/2, szTemplate, szKeyCheckValue);
	}
	
	{
		char	szPINBlock[8 + 1];
		memset(szPINBlock, 0x00, sizeof(szPINBlock));

		TRANSACTION_OBJECT pobTran;
		memset(&pobTran, 0x00, sizeof(TRANSACTION_OBJECT));
		memcpy(&pobTran.srBRec.szPAN, "452001234567890301", strlen("452001234567890301"));

		inNCCC_TMK_CalculatePINBlock(&pobTran, szPINBlock);
	}

	{
		char	szPlainData[128 + 1];
		char	szMACData[128 + 1];
		
		memset(szPlainData, 0x00, sizeof(szPlainData));
		memset(szMACData, 0x00, sizeof(szMACData));

		memcpy(szPlainData,"401399402000376348129192026D1903110416522000000010000",strlen("401399402000376348129192026D1903110416522000000010000"));

		inNCCC_TMK_CalculateMac(szPlainData, szMACData);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_ESCKey_Test
Date&Time       :2016/4/21 上午 11:06
Describe        :
*/
int inNCCC_TMK_ESCKey_Test()
{	
	char			szTemplate[42 + 1];
	char			szAscii[64 + 1];
	TRANSACTION_OBJECT	pobTran;
	
	memset(&pobTran, 0x00, sizeof(pobTran));
	
//	if (inFunc_GetHostNum(&pobTran) != VS_SUCCESS)
//	{
//		return (VS_ERROR);
//	}
//	
//	if (inBATCH_FuncUserChoice_By_Sqlite(&pobTran) != VS_SUCCESS)
//	{
//		return (VS_ERROR);
//	}
//	
//	if (inBATCH_FuncUserChoice_By_Sqlite(&pobTran) != VS_SUCCESS)
//	{
//		return (VS_ERROR);
//	}
	/* TID 13995512 */
	pobTran.srBRec.lnOrgInvNum = 001;
	pobTran.srBRec.lnBatchNum = 001;
	memcpy(pobTran.srBRec.szTime, "1515", 4);
	pobTran.srBRec.lnTxnAmount = 11;
	
	if (inNCCC_TMK_Write_ESCKey(&pobTran) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	
	if (inNCCC_TMK_ESC_3DES_Encrypt("3569990010082211", 0, szTemplate) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	memset(szAscii, 0x00, sizeof(szAscii));
	inFunc_BCD_to_ASCII(szAscii, (unsigned char*)szTemplate, 8);
	/* should be 6A6F31F73B9D0FD0*/
	inLogPrintf(AT, szAscii);
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TMK_MFES_Test
Date&Time       :2017/1/3 上午 9:34
Describe        :莫名其妙MFES安全認證不過
*/
int inNCCC_TMK_MFES_Test()
{	
	char			szTemplate[42 + 1];
	char			szDebugMsg[100 + 1];
	char			szPlainData[128 + 1];
	char			szMACData[128 + 1];
	unsigned short		usTMKindex = 0x0001;
	unsigned short		usKeyLen = 32;
	
	
	inKMS_Initial();
	inNCCC_TMK_Write_Test_TMK_By_Terminal();
	
	for (usTMKindex = 1; usTMKindex <= 15; usTMKindex++)
	{
//		{
//			memset(szTemplate, 0x00, sizeof(szTemplate));
//			memcpy(szTemplate, "3D96384729C1C60066CCE7AEEE590BB0", 32);
//			inNCCC_TMK_Write_PINKey(usTMKindex, usKeyLen/2, szTemplate, szKeyCheckValue);
//		}

		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, "2A238A173E9EA6EB01F40E7AAE793B67", 32);
			inNCCC_TMK_Write_MACKey(usTMKindex, usKeyLen/2, szTemplate, "");
		}

//		{
//			char	szPINBlock[8 + 1];
//			memset(szPINBlock, 0x00, sizeof(szPINBlock));
//	
//			TRANSACTION_OBJECT pobTran;
//			memset(&pobTran, 0x00, sizeof(TRANSACTION_OBJECT));
//			memcpy(&pobTran.srBRec.szPAN, "452001234567890301", strlen("452001234567890301"));
//	
//			inNCCC_TMK_CalculatePINBlock(&pobTran, szPINBlock);
//		}

		{
			memset(szPlainData, 0x00, sizeof(szPlainData));
			memset(szMACData, 0x00, sizeof(szMACData));

			memcpy(szPlainData,"631399550200;3560500100001218D18121011697684900000?1000000000100",strlen("631399550200;3560500100001218D18121011697684900000?1000000000100"));

			inNCCC_TMK_CalculateMac(szPlainData, szMACData);
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			inFunc_BCD_to_ASCII(szDebugMsg, (unsigned char*)szMACData, 8);
			sprintf(szDebugMsg, "%s", szDebugMsg);
			inLogPrintf(AT, szDebugMsg);
			/* MAC值should be 78E1F67BC3CD2C17 */
		}
	}
	
	return (VS_SUCCESS);
}


void PrintHex(const char* label, BYTE* pData, USHORT usLen) 
{
    USHORT i;
    if (pData == NULL || usLen == 0) {
        printf("%s: (null or empty)\n", label);
        return;
    }
    
    printf("%s (Len=%d): \n", label, usLen);
    
    // 為了排版美觀，每 16 bytes 換行一次
    for (i = 0; i < usLen; i++) {
        printf("%02X ", pData[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    // 如果最後一行不滿 16 bytes，補上換行
    if (usLen % 16 != 0) {
        printf("\n");
    }
    printf("----------------------------------------\n");
}

void RunDukptTest(BYTE cipherMethod, const char* modeName)
{
    CTOS_KMS2AESDUKPTDATAENCRYPT_PARA stPara;
    USHORT usRtn;
    BYTE KsnLen;
    
    // 配置動態或靜態記憶體
    BYTE baInput[TEST_DATA_SIZE];
    BYTE baOutput[TEST_DATA_SIZE + 32]; // 加密輸出稍微留大一點緩衝
    BYTE baKSN[12];
    BYTE baTmpKSN[12];
    BYTE baICV[16];                     // AES 的固定塊大小為 16 位元組
    

    // 初始化記憶體與測試資料
    memset(&stPara, 0x00, sizeof(stPara));
    memset(baOutput, 0x00, sizeof(baOutput));
    memset(baKSN, 0x00, sizeof(baKSN));
    memset(baICV, 0x00, sizeof(baICV)); // ECB 不需要 ICV，CBC 需要
    memset(baInput, 0x5A, sizeof(baInput)); // 填入測試 dummy 資料 (0x5A)

    // 設定 KMS2 DUKPT 參數
    stPara.Version = 0x00;
    stPara.Protection.usKeySet = 0x0101;     // 請根據您實際下載的 KeySet 調整
    stPara.Protection.usKeyIndex = 0x0001;    // 請根據您實際下載的 KeyIndex 調整
    
    // 設定加密模式 (ECB 或 CBC)
    stPara.Input.bCipherMethod = cipherMethod; 
    stPara.Input.usLength = sizeof(baInput);
    stPara.Input.pData = baInput;
    
    // 當使用 CBC 模式時，AES 的 ICV (IV) 長度必須為 16 位元組
    if (cipherMethod == KMS2_DATAENCRYPTCIPHERMETHOD_CBC) {
        stPara.Input.usICVLength = 16; 
        stPara.Input.pICV = baICV;
    } else {
        stPara.Input.usICVLength = 0;
        stPara.Input.pICV = NULL;
    }

    stPara.Output.usLength = sizeof(baOutput);
    stPara.Output.pData = baOutput;
    stPara.Output.usKSNLen = sizeof(baKSN);
    stPara.Output.pKSN = baKSN;

    stPara.Additional.isUseCurrentKey = FALSE;
    stPara.Additional.bDerivedKeyUsage = KMS2_DERIVEDKEYUSAGE_DATAENCRYPTIONENCRYPT;
    
    // 關鍵修改：將原範例的 2TDEA 改為 AES256
    // 註：若編譯器報錯找不到此常數，請查閱 KMS2 文件的最新衍生金鑰型態定義（通常為 KMS2_DERIVEDKEYTYPE_AES256）
    stPara.Additional.bDerivedKeyType = KMS2_DERIVEDKEYTYPE_AES256; 

    printf("開始測試 AES-256 DUKPT [%s] 模式，資料長度: %d bytes...\n", modeName, TEST_DATA_SIZE);

    // --- 時間測試開始 ---
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    usRtn = CTOS_KMS2AESDUKPTDataEncrypt(&stPara);

    clock_gettime(CLOCK_MONOTONIC, &end);
    // --- 時間測試結束 ---

    if (usRtn == d_OK) {
        // 計算花費時間（毫秒 ms）
        double elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0 + 
                            (end.tv_nsec - start.tv_nsec) / 1000000.0;
        printf("[%s 測試成功] 耗時: %.3f ms\n", modeName, elapsed_ms);
        printf("回傳之 KSN 長度: %d, 加密後資料長度: %d\n", stPara.Output.usKSNLen, stPara.Output.usLength);
// 1. 印出加密後的資料 (密文)
        // 資料位置：stPara.Output.pData
        // 資料長度：stPara.Output.usLength
        PrintHex("Encrypted Data", stPara.Output.pData, stPara.Output.usLength);
        
        // 2. 印出當次使用的 KSN (交易計數器已累加)
        // 資料位置：stPara.Output.pKSN
        // 資料長度：stPara.Output.usKSNLen
        PrintHex("Output KSN", stPara.Output.pKSN, stPara.Output.usKSNLen);
    } else {
        printf("[%s 測試失敗] 錯誤碼: 0x%04X\n", modeName, usRtn);
    }
    memset(baTmpKSN, 0x00 , sizeof(baTmpKSN));
    CTOS_KMS2DUKPTGetKSN(stPara.Protection.usKeySet, stPara.Protection.usKeyIndex, baTmpKSN, &KsnLen);
    
     PrintHex("Output KSN", baTmpKSN, KsnLen);
     
    printf("--------------------------------------------------\n");
}

int DukptSpeedTestMain(void)
{
    // 1. 測試 ECB 模式
    RunDukptTest(KMS2_DATAENCRYPTCIPHERMETHOD_ECB, "ECB");

    // 2. 測試 CBC 模式
    RunDukptTest(KMS2_DATAENCRYPTCIPHERMETHOD_CBC, "CBC");

    return 0;
}


// 確保已包含您的 kmsext.h 及其依賴的 kms2_def.h 等

USHORT LoadAes256IpekTest(void)
{
    CTOS_KMS2KEYWRITE_PARA stWritePara;
    USHORT usRtn;

    // 模擬由銀行端衍生的 32 bytes (256 bits) AES IPEK
    BYTE baDummyIPEK[32] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
    };

    // AES DUKPT 初始 KSN (長度依銀行規範，通常為 10 或 12 bytes)
    // 這裡以 12 bytes 為例，前段為 Key ID/Device ID，後段交易計數器為 0
    BYTE baInitKSN[12] = {
        0xFF, 0xFF, 0x98, 0x76, 0x54, 0x32, 0x10, 0xE0, 0x00, 0x00, 0x00, 0x00
    };

    memset(&stWritePara, 0x00, sizeof(stWritePara));
    
    // 依據 kmsext.h 註解，Version 應為 0x00 或 0x01
    stWritePara.Version = 0x01; 

    // 1. 設定金鑰基本資訊 (Info)
    stWritePara.Info.KeySet = 0x0101;
    stWritePara.Info.KeyIndex = 0x0001;
    // 註：請確認 kms2_def.h 中關於 AES256 DUKPT Base Key 的正確巨集名稱
    stWritePara.Info.KeyType = KMS2_KEYTYPE_AES_DUKPT; 
    stWritePara.Info.KeyVersion = 0x01;
    stWritePara.Info.KeyAttribute = 0x00000000; // 測試時可設為 0，正式環境可加上保護屬性

    // 2. 設定保護模式 (Protection) - 依據 kmsext.h 定義
    stWritePara.Protection.Mode = KMS2_KEYPROTECTIONMODE_PLAINTEXT; 

    // 3. 帶入金鑰資料與長度 (Value)
    stWritePara.Value.KeyLength = sizeof(baDummyIPEK); 
    stWritePara.Value.pKeyData = baDummyIPEK;

    // 4. 設定 DUKPT 初始資料 (DUKPT_DATA)
    stWritePara.DUKPT_DATA.KSNLength = sizeof(baInitKSN);
    stWritePara.DUKPT_DATA.pKSN = baInitKSN;

    printf("正在透過 CTOS_KMS2KeyWrite 將 AES-256 IPEK 寫入 (KeySet: 0x0101)... \n");

    // 呼叫 KMS2 API
    usRtn = CTOS_KMS2KeyWrite(&stWritePara);

    if (usRtn == d_OK) {
        printf("[金鑰寫入成功] AES-256 IPEK 已成功儲存於設備安全晶片中。\n");
    } else {
        printf("[金鑰寫入失敗] 錯誤碼: 0x%04X\n", usRtn);
    }

    return usRtn;
}



USHORT LoadAes256IpekByTR31Test(void)
{
    CTOS_KMS2KEYWRITEBYTR31EX_PARA stTR31WritePara;
    USHORT usRtn;

    // 1. 模擬由銀行端下發的 TR-31 Key Block 字串
    // 實務上這會是一長串字元，標頭會定義這是一把 DUKPT Base Key (例如 Usage='D0', Algorithm='A' 代表 AES)
    // 這裡僅用假字串示意，測試時請替換為銀行提供、或您用測試工具產出的真實 TR-31 Block
    BYTE baDummyTR31Block[] = "B0096D0AE00N0000...[加密金鑰與MAC]..."; 

    // 2. DUKPT 初始 KSN (綁定這把 IPEK 使用)
    BYTE baInitKSN[12] = {
        0xFF, 0xFF, 0x98, 0x76, 0x54, 0x32, 0x10, 0xE0, 0x00, 0x00, 0x00, 0x00
    };

    memset(&stTR31WritePara, 0x00, sizeof(stTR31WritePara));

    // ==========================================
    // (A) 設定 Key Owner (Ex API 專屬)
    // ==========================================
    stTR31WritePara.KeyOwner.Length = 6;
    memcpy(stTR31WritePara.KeyOwner.Name, "BANK_A", 6);

    // ==========================================
    // (B) 設定 TR-31 寫入參數
    // ==========================================
    stTR31WritePara.KeyWriteByTR31Para.Version = 0x01; // 依您的 SDK 版本而定，通常為 0x00 或 0x01

    // [關鍵 1] 保護金鑰 (KBPK)
    // 告訴 KMS2 晶片：「請用儲存在這個位置的 KBPK，來驗證並解密這包 TR-31 資料」
    // 假設您的 KBPK 放在 KeySet 0x0002, KeyIndex 0x0001
    stTR31WritePara.KeyWriteByTR31Para.Protection.CipherKeySet = 0x0002;
    stTR31WritePara.KeyWriteByTR31Para.Protection.CipherKeyIndex = 0x0001;

    // [關鍵 2] 目的金鑰位置 (DstKey)
    // 告訴 KMS2 晶片：「解密成功後，請把這把 AES-256 IPEK 存放到這裡」
//    stTR31WritePara.KeyWriteByTR31Para.DstKey.usKeySet = 0x0101;
//    stTR31WritePara.KeyWriteByTR31Para.DstKey.usKeyIndex = 0x0001;
    // 註：有些 SDK 的 TR-31 API 會直接從 TR-31 Header (如 D0) 推斷 KeyType，
    // 若您的編譯器提示 DstKey 下需要 bKeyType，請補上 KMS2_KEYTYPE_AES256_DUKPT_BASE_KEY。

    // [關鍵 3] 帶入 TR-31 資料本身
//    stTR31WritePara.KeyWriteByTR31Para.Input.Length = strlen((char*)baDummyTR31Block);
//    stTR31WritePara.KeyWriteByTR31Para.Input.pData = baDummyTR31Block;

    // [關鍵 4] DUKPT KSN 綁定
 //   stTR31WritePara.KeyWriteByTR31Para.DUKPT_DATA.KSNLength = sizeof(baInitKSN);
//    stTR31WritePara.KeyWriteByTR31Para.DUKPT_DATA.pKSN = baInitKSN;

    printf("正在透過 TR-31 寫入 AES-256 IPEK (Dst: 0x0101)...\n");

    // 呼叫 API 執行寫入
    usRtn = CTOS_KMS2KeyWriteByTR31Ex(&stTR31WritePara);

    if (usRtn == d_OK) {
        printf("[TR-31 寫入成功] IPEK 已安全儲存！後續可無縫銜接 DUKPT 加密測試。\n");
    } else {
        printf("[TR-31 寫入失敗] 錯誤碼: 0x%04X\n", usRtn);
    }

    return usRtn;
}