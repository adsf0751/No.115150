#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sqlite3.h>
#include <ctosapi.h>
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/AllStruct.h"
#include "../SOURCE/Display/Display.h"
#include "../SOURCE/Display/DispMsg.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/EVENT/Menu.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/Batch.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/SKM.h"
#include "../SOURCE/FUNCTION/TDT.h"
#include "../SOURCE/FUNCTION/Utility.h"
#include "../SOURCE/Print/Print.h"
#include "NCCCats.h"
#include "NCCCTicketSrc.h"
#include "NCCCtSAM.h"

extern  int     ginDebug;  /* Debug使用 extern */
unsigned char	guszSlot1InBit = VS_FALSE;
unsigned char	guszSlot2InBit = VS_FALSE;
unsigned char	guszSlot3InBit = VS_FALSE;
unsigned char	guszSlot4InBit = VS_FALSE;

/*
Function        :inNCCC_tSAM_APDUTransmit
Date&Time       :2016/1/6 下午 1:21
Describe        :此function將傳送進來的APDU command送給smart card
*/
int inNCCC_tSAM_APDUTransmit(unsigned char uszSlot, unsigned char *uszSendBuf, unsigned short usSendLen, unsigned char *uszReceBuf, int *usReceLen)
{
	int		i = 0;
	int		inDebugLen = 0;
	char		szTemplate[256 + 1] = {0};			/* 用來輸出debug訊息(debug訊息要放得下轉換後的訊息，所以至少要APDU長度的兩倍以上） */
	char		szDebugMsg[100 + 1] = {0};
	char		szLogSendAPDU[256 + 1] = {0};			/* 用來紀錄Log用 */
	char		szLogReceAPDU[256 + 1] = {0};			/* 用來紀錄Log用 */
	unsigned char 	uszSAPDU[128 + 1] = {0}, uszRAPDU[128 + 1] = {0};	/* 用來放ADPU COMMAND */
	unsigned short 	usSLen = 0, usRLen = 0, usRetVal = 0;
	

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "-------------------------------------");
                inLogPrintf(AT, "inNCCC_tSAM_APDUTransmit() START!");
		
		/* 列印tSAM APDU Command資料 (START) */
		inLogPrintf(AT, "-------------- START -----------------");
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "APDU Send Data: %d", usSendLen);
		inLogPrintf(AT, szTemplate);
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inFunc_BCD_to_ASCII(szTemplate, uszSendBuf, usSendLen);
		inDebugLen = 2 * usSendLen;
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		/* ADPU command 太長 */
		for (i = 0; inDebugLen > 0; i++)
		{
			if (inDebugLen > 40)
			{
				memcpy(szDebugMsg, &szTemplate[i * 40], 40);
				inLogPrintf(AT, szDebugMsg);
				memset(szDebugMsg, 0x00, 40);
				inDebugLen -= 40;
			}
			else
			{
				memcpy(szDebugMsg, &szTemplate[i * 40], inDebugLen);
				inLogPrintf(AT, szDebugMsg);
				memset(szDebugMsg, 0x00, inDebugLen);
				inDebugLen -= inDebugLen;
			}
			
		}
	}

	/* 將要傳送的長度及資料塞進send Buffer */
	memset(uszSAPDU, 0x00, sizeof(uszSAPDU));
	memset(uszRAPDU, 0x00, sizeof(uszRAPDU));
	memcpy(&uszSAPDU[0], &uszSendBuf[0], usSendLen);
	usSLen = usSendLen;

	usRLen = sizeof(uszRAPDU);
	usRetVal = CTOS_SCSendAPDU(uszSlot, uszSAPDU, usSLen, uszRAPDU, &usRLen);
	if (usRetVal  == d_SC_NOT_ACTIVATED)
	{
		/* 沒power ON */
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_tSAM_SCSendAPDU_ERROR!!");
			inLogPrintf(AT, "d_SC_NOT_ACTIVATED");
		}
		inUtility_StoreTraceLog_OneStep("inNCCC_tSAM_SCSendAPDU_ERROR");
		inUtility_StoreTraceLog_OneStep("d_SC_NOT_ACTIVATED");
	}
	else if (usRetVal  == d_SC_MUTE)
	{
		/* SAM卡無反應(Power On後，再拔Sam卡會有此反應) */
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_tSAM_SCSendAPDU_ERROR!!");
			inLogPrintf(AT, "d_SC_MUTE");
		}
		inUtility_StoreTraceLog_OneStep("inNCCC_tSAM_SCSendAPDU_ERROR");
		inUtility_StoreTraceLog_OneStep("d_SC_MUTE");
	}
	else if (usRetVal  != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_tSAM_SCSendAPDU_ERROR!!");
			
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "錯誤代碼： 0x%04x", usRetVal);
			inLogPrintf(AT, szTemplate);
		}
		inUtility_StoreTraceLog_OneStep("inNCCC_tSAM_SCSendAPDU_ERROR");
		inUtility_StoreTraceLog_OneStep("錯誤代碼： 0x%04x", usRetVal);
                
                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_tSAM_SCSendAPDU_ERROR 錯誤代碼： 0x%04x", usRetVal);
	}
	
	if (usRetVal  != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "APDU Rece Data: %d", *usReceLen);
			inLogPrintf(AT, szTemplate);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			inFunc_BCD_to_ASCII(szTemplate, uszReceBuf, *usReceLen);

			inDebugLen = 2 * *usReceLen;

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			/* ADPU command 太長 */
			for (i = 0; inDebugLen > 0; i++)
			{
				if (inDebugLen > 40)
				{
					memcpy(szDebugMsg, &szTemplate[i * 40], 40);
					inLogPrintf(AT, szDebugMsg);
					memset(szDebugMsg, 0x00, 40);
					inDebugLen -= 40;
				}
				else
				{
					memcpy(szDebugMsg, &szTemplate[i * 40], inDebugLen);
					inLogPrintf(AT, szDebugMsg);
					memset(szDebugMsg, 0x00, inDebugLen);
					inDebugLen -= inDebugLen;
				}

			}
			/* 列印tSAM APDU Command資料 (END) */
			inLogPrintf(AT, "-------------- END -----------------");
		}
		
		memset(szLogSendAPDU, 0x00, sizeof(szLogSendAPDU));
		inFunc_BCD_to_ASCII(szLogSendAPDU, uszSendBuf, usSendLen);
		
		memset(szLogReceAPDU, 0x00, sizeof(szLogReceAPDU));
		inFunc_BCD_to_ASCII(szLogReceAPDU, uszReceBuf, *usReceLen);
		
		inUtility_StoreTraceLog_OneStep("APDU Send Data: %d", usSendLen);
		inUtility_StoreTraceLog_OneStep("%s", szLogSendAPDU);
		
		inUtility_StoreTraceLog_OneStep("APDU Rece Data: %d", *usReceLen);
		inUtility_StoreTraceLog_OneStep("%s", szLogReceAPDU);
		
		return(VS_ERROR);
	}

	/* 將APDU回的長度及資料塞回Receive Buffer */
	*usReceLen = usRLen;
	memcpy(&uszReceBuf[0], &uszRAPDU[0], *usReceLen);

	if (ginDebug == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "APDU Rece Data: %d", *usReceLen);
		inLogPrintf(AT, szTemplate);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inFunc_BCD_to_ASCII(szTemplate, uszReceBuf, *usReceLen);
		
		inDebugLen = 2 * *usReceLen;
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		/* ADPU command 太長 */
		for (i = 0; inDebugLen > 0; i++)
		{
			if (inDebugLen > 40)
			{
				memcpy(szDebugMsg, &szTemplate[i * 40], 40);
				inLogPrintf(AT, szDebugMsg);
				memset(szDebugMsg, 0x00, 40);
				inDebugLen -= 40;
			}
			else
			{
				memcpy(szDebugMsg, &szTemplate[i * 40], inDebugLen);
				inLogPrintf(AT, szDebugMsg);
				memset(szDebugMsg, 0x00, inDebugLen);
				inDebugLen -= inDebugLen;
			}
			
		}
		/* 列印tSAM APDU Command資料 (END) */
		inLogPrintf(AT, "-------------- END -----------------");
	}
	
	/* 判斷回應碼是否為9000 */
	if (memcmp(&uszReceBuf[*usReceLen - 2], "\x90\x00", 2))
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inFunc_BCD_to_ASCII(szTemplate, &uszReceBuf[*usReceLen - 2], 2);
			inLogPrintf(AT, "APDU response as below");
			inLogPrintf(AT, szTemplate);
		}
                
                memset(szTemplate, 0x00, sizeof(szTemplate));
		inFunc_BCD_to_ASCII(szTemplate, uszReceBuf, *usReceLen);
                vdUtility_SYSFIN_LogMessage(AT,"APDU Rece %s ", szTemplate);
                
		return(VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
                inLogPrintf(AT, "inNCCC_tSAM_APDUTransmit() END!!");
		inLogPrintf(AT, "---------------------------------------");
	}
	
  	return(VS_SUCCESS);
}

/*
Function        :inNCCC_tSAM_APDUEncrypt
Date&Time       :2016/1/14 下午 12:01
Describe        :於交易時，field2，field35，field55的tag(0x57 or 0x5a)欄位需以SAM卡加密
 		 此function依SAM卡規格下APDU Command作加密。
		uszData		-要加密欄位pack後的資料
	  	inLength	-要加密欄位pack後的長度(byte)
	  	*uszRRN		-Ref Number
	  	*uszTID		-Terminal ID
	  	*uszKeyindex	-SAM卡回傳的key index
	  	*uszCheckSum	-加密欄位的checksum
*/
int inNCCC_tSAM_APDUEncrypt(unsigned char       uszSlot,
			    int			inLength, 
			    unsigned char	*uszData,
			    unsigned char	*uszRRN,
			    unsigned char	*uszTID,
			    unsigned char	*uszKeyindex,
			    unsigned char	*uszCheckSum)
{
	int		inCnt = 0;
	int		inRetVal;
	char		szTemplate[42 + 1];
	APDU_COMMAND	srAPDUData;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "---------------------------------------");
                inLogPrintf(AT, "inNCCC_tSAM_APDUEncrypt START!!");
	}

	memset((char *) &srAPDUData, 0x00, APDU_LENGTH);

	/* 組tSAM Encrypt APDU command */
	srAPDUData.uszSendData[inCnt ++] = SAM_CLA_ENCRYPTION;		/* CLA */
	srAPDUData.uszSendData[inCnt ++] = SAM_INS_ENCRYPTION;		/* INS */
	srAPDUData.uszSendData[inCnt ++] = SAM_P1_ENCRYPTION;		/* P1 */
	srAPDUData.uszSendData[inCnt ++] = SAM_P2_ENCRYPTION;		/* P2 */
	srAPDUData.uszSendData[inCnt ++] = 15 + inLength;		/* Lc */

	/* TID:8個數字（4Bytes） */
	memcpy(&srAPDUData.uszSendData[inCnt], uszTID, TIDSIZE);	/* TID */
	inCnt += TIDSIZE;

	srAPDUData.uszSendData[inCnt++] = SAM_FUNC_ENCRYPTION;		/* FUNC */
	srAPDUData.uszSendData[inCnt++] = SAM_MODE_ENCRYPTION;		/* MODE */

	memcpy(&srAPDUData.uszSendData[inCnt], uszRRN, DIVSIZE);		/* DIV */
	inCnt += DIVSIZE;

	srAPDUData.uszSendData[inCnt] = inLength;			/* LENGTH */
	inCnt++;

	memcpy(&srAPDUData.uszSendData[inCnt], uszData, inLength);	/* Data */
	inCnt += inLength;

	srAPDUData.uszSendData[inCnt] = 6 + inLength;			/* LE */
	inCnt++;

	/* 將inCnt算的長度放入APDU結構 */
	srAPDUData.inSendLen = inCnt;

	inRetVal = inNCCC_tSAM_APDUTransmit(uszSlot, srAPDUData.uszSendData, srAPDUData.inSendLen, srAPDUData.uszRecevData, &srAPDUData.inRecevLen);
		
	if (inRetVal != VS_SUCCESS)
	{
		/* SAM卡加密失敗補救機制 */
		do
		{
			inRetVal = inNCCC_tSAM_SAM_Encrypt_Fail_Remedy(uszSlot);
			if (inRetVal != VS_SUCCESS)
			{
				break;
			}

			inRetVal = inNCCC_tSAM_APDUTransmit(uszSlot, srAPDUData.uszSendData, srAPDUData.inSendLen, srAPDUData.uszRecevData, &srAPDUData.inRecevLen);
			if (inRetVal != VS_SUCCESS)
			{
				break;
			}
			break;
		}while(1);
		
		if (inRetVal != VS_SUCCESS)
		{
			/* SAM卡加密失敗 */
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SAM卡加密失敗");
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "Encrypt Fail: SAM Response %02x%02x", srAPDUData.uszRecevData[srAPDUData.inRecevLen-2], srAPDUData.uszRecevData[srAPDUData.inRecevLen-1]);
				inLogPrintf(AT, szTemplate);
			}
			/* [BUG] 移除檔案同步API SYNC()指令     
			* [20260506] 新增APUT錯誤記錄 */
			vdUtility_SYSFIN_LogMessage(AT, "Slot[%d] Encrypt Error: SAM Response %02x%02x", uszSlot, srAPDUData.uszRecevData[srAPDUData.inRecevLen-2], 
											srAPDUData.uszRecevData[srAPDUData.inRecevLen-1]);
			return (VS_ERROR);
		}
		
	}

	memcpy(uszData, &srAPDUData.uszRecevData[2], srAPDUData.uszRecevData[1]);
	*uszKeyindex = srAPDUData.uszRecevData[0];
	memcpy(uszCheckSum, &srAPDUData.uszRecevData[1 + 1 + srAPDUData.uszRecevData[1]], CHECKSUMSIZE);
	
	if (ginDebug == VS_TRUE)
	{

		inLogPrintf(AT, "inNCCC_tSAM_APDUEncrypt() END!!");
		inLogPrintf(AT, "---------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_tSAM_Encrypt
Date&Time       :2016/1/21 下午 1:24
Describe        :因應不同SAM卡長度規格，以不同方式處理加密
		 inDataLength：要以多大資料為一區塊做加密
		 szInOutEncryptData：要加密的資料
 		 uszRRN：RRN補F補到16碼
 		 uszTID：Terminal ID
		 uszKeyIndex:tSAM key的Index
		 uszCheckSum:檢查碼
*/
int inNCCC_tSAM_Encrypt(unsigned char   uszSlot, 
			int		inDataLength,
			char		*szInOutEncryptData,
			unsigned char	*uszRRN,
			unsigned char	*uszTID,
			unsigned char	*uszKeyIndex,
			unsigned char	*uszCheckSum)
{
        int             in8TimesLen = 0;
	char		szTemplate[42 + 1];
	unsigned char	uszEncryptData[128 + 1];
	

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "---------------------------------------");
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "inNCCC_tSAM_Encrypt(DataLen = %d) START!!", inDataLength);
		inLogPrintf(AT, szTemplate);
	}
	
	
	/* 寫死，NCCC固定為1，SAM卡固定放slot1 */

	/* 判斷SAM卡狀態是否Ready */
	if (inNCCC_tSAM_Check_SAM_In_Slot_By_Global_Variable(uszSlot) != VS_SUCCESS)
	{
		/* 請檢查SAM卡 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHECK_SAM_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = 1;
		char	szHostEnable[1 + 1] = {0};
		inNCCC_Ticket_GetESVC_Enable(0, szHostEnable);
		if (szHostEnable[0] == 'N')
		{
			sprintf(srDispMsgObj.szErrMsg1, "SAM卡請放SAM1");
		}
		else
		{
			sprintf(srDispMsgObj.szErrMsg1, "SAM卡請放SAM%d", uszSlot);
		}
		srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);
	}
	
	/* SAM 卡規格不足 16 個 nibble 需補滿，並以 16 個 nibble 作加密 (1 nibble = 0.5 byte)*/
	if (inDataLength < 16)
	{
		memset(uszEncryptData, 0x00, sizeof(uszEncryptData));
		memcpy(uszEncryptData, szInOutEncryptData, 8);

		if (inNCCC_tSAM_APDUEncrypt(uszSlot, 
					    8, 
			                    uszEncryptData,
				  uszRRN,
				  uszTID,
				  uszKeyIndex,
				  uszCheckSum) == VS_ERROR)
		{
			return (VS_ERROR);
		}

		memcpy(szInOutEncryptData, uszEncryptData, 8);
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_tSAM_Encrypt_END!!! 8byte");
			inLogPrintf(AT, "---------------------------------------");
		}
		
		return (8);
	}
	else if (inDataLength < 32)
	{
		/* SAM 卡規格大於 16 個 nibble 小於 32 個 nibble，以 16 個 nibble 作加密 */
		memset(uszEncryptData, 0x00, sizeof(uszEncryptData));
		memcpy(uszEncryptData, szInOutEncryptData, 8);

		if (inNCCC_tSAM_APDUEncrypt(uszSlot, 8, uszEncryptData,
				  uszRRN,
				  uszTID,
				  uszKeyIndex,
				  uszCheckSum) == VS_ERROR)
		{
			return (VS_ERROR);
		}

		memcpy(szInOutEncryptData, uszEncryptData, 8);
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_tSAM_Encrypt_END!!! 8byte");
			inLogPrintf(AT, "---------------------------------------");
		}
		
		return (8);
	}
	else
	{
                in8TimesLen = ((inDataLength / 2) - ((inDataLength / 2) % 8));
		/* SAM 卡規格大於或等於 32 個 nibble，以 32 個 nibble 作加密 */
		memset(uszEncryptData, 0x00, sizeof(uszEncryptData));
		memcpy(uszEncryptData, szInOutEncryptData, in8TimesLen);

		if (inNCCC_tSAM_APDUEncrypt(uszSlot, in8TimesLen, uszEncryptData,
				  uszRRN,
				  uszTID,
				  uszKeyIndex,
				  uszCheckSum) == VS_ERROR)
		{
			return (VS_ERROR);
		}

		memcpy(szInOutEncryptData, uszEncryptData, in8TimesLen);
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_tSAM_Encrypt_END!!! 16byte");
			inLogPrintf(AT, "---------------------------------------");
		}
		
		return (in8TimesLen);
	}
}

/*
Function        :inNCCC_tSAM_Encrypt_ESVC
Date&Time       :2020/8/24 下午 2:01
Describe        :因應不同SAM卡長度規格，以不同方式處理加密
		 inDataLength：要以多大資料為一區塊做加密
		 szInOutEncryptData：要加密的資料
 		 uszRRN：RRN補F補到16碼
 		 uszTID：Terminal ID
		 uszKeyIndex:tSAM key的Index
		 uszCheckSum:檢查碼
 *		 電票tSAM加密
*/
int inNCCC_tSAM_Encrypt_ESVC(unsigned char   uszSlot, 
			int		inDataLength,
			char		*szInOutEncryptData,
			unsigned char	*uszRRN,
			unsigned char	*uszTID,
			unsigned char	*uszKeyIndex,
			unsigned char	*uszCheckSum)
{
	char		szTemplate[100 + 1];
	unsigned char	uszEncryptData[128 + 1];
	

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "---------------------------------------");
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "inNCCC_tSAM_Encrypt_ESVC(DataLen = %d) START!!", inDataLength);
		inLogPrintf(AT, szTemplate);
	}

	/* 判斷SAM卡狀態是否Ready */
	if (inNCCC_tSAM_Check_SAM_In_Slot_By_Global_Variable(uszSlot) != VS_SUCCESS)
	{
		/* 請檢查SAM卡 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHECK_SAM_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = 1;
		sprintf(srDispMsgObj.szErrMsg1, "SAM卡請放SAM%d", uszSlot);
		srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);
	}

	memset(uszEncryptData, 0x00, sizeof(uszEncryptData));
	memcpy(uszEncryptData, szInOutEncryptData, inDataLength);

	if (inNCCC_tSAM_APDUEncrypt(uszSlot, inDataLength, uszEncryptData,
			  uszRRN,
			  uszTID,
			  uszKeyIndex,
			  uszCheckSum) == VS_ERROR)
	{
		return (VS_ERROR);
	}

	memcpy(szInOutEncryptData, uszEncryptData, inDataLength);

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_tSAM_Encrypt_ESVC_END!!! (%d)byte", inDataLength);
		inLogPrintf(AT, "---------------------------------------");
	}

	return (inDataLength);
}

/*
Function        :inNCCC_tSAM_InitialSLOT
Date&Time       :2016/1/6 上午 11:25
Describe        :
*/
int inNCCC_tSAM_InitialSLOT(TRANSACTION_OBJECT * pobTran)
{
	int		inRetVal = VS_ERROR, i = 0;
	char		szTemplate[42 + 1] = {0};
	char		szEncryptMode[2 + 1] = {0};
	char		szTSAMRegisterEnable[1 + 1] = {0};
	char		szDebugMsg[100 + 1];
	unsigned char	uszSlot = 0;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "---------------------------------------");
                inLogPrintf(AT, "inTSAM_FuncInitialSLOT() START!");
	}
	
	/* 非tsam加密 */
	memset(szEncryptMode, 0x00, sizeof(szEncryptMode));
	inGetEncryptMode(szEncryptMode);
	if (memcmp(szEncryptMode, _NCCC_ENCRYPTION_TSAM_, 1) != 0)
	{
                vdUtility_SYSFIN_LogMessage(AT,"非TSAM加密 szEncryptMode:(%s)", szEncryptMode);
		return (VS_SUCCESS);
	}
	
	/* 抓tSAM Slot */
	inRetVal = inNCCC_tSAM_Decide_tSAM_Slot(&uszSlot);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inNCCC_tSAM_Decide_tSAM_Slot Failed");
			inLogPrintf(AT, szDebugMsg);
		}
	}

	inRetVal = inNCCC_tSAM_PowerOn_Flow(&uszSlot);
	/* Power On 失敗 */
	if (inRetVal != VS_SUCCESS)
	{
                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_tSAM_PowerOn_Flow failed, uszSlot:(%d)", uszSlot);
                
		/* 請檢查SAM卡 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHECK_SAM_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = 1;
		char	szHostEnable[1 + 1] = {0};
		inNCCC_Ticket_GetESVC_Enable(pobTran->srBRec.inHDTIndex, szHostEnable);
		if (szHostEnable[0] == 'N')
		{
			sprintf(srDispMsgObj.szErrMsg1, "SAM卡請放SAM1");
		}
		else
		{
			sprintf(srDispMsgObj.szErrMsg1, "SAM卡請放SAM%d", uszSlot);
		}
		srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);
		
		return (VS_ERROR);
	}
	
	/* 純tSAM Power On完直接抓SN(先select AID反而抓不到)，之後要再PowerOn一次並Select AID 
	   MSAM Power On完要Select AID之後才能抓SN */
	
	/* 開機才抓SAM卡SN */
	if (pobTran->inRunOperationID == _OPERATION_EDC_BOOTING_	||
	    pobTran->inRunOperationID == _OPERATION_EDC_V3UL_BOOTING_	||
	    pobTran->inRunOperationID == _OPERATION_EDC_SDK_INITIAL_BOOTING_)
	{
		/* 紀錄有放TSAM的位置 */
		if (inNCCC_tSAM_Check_SAM_In_Slot(_SAM_SLOT_1_) == VS_SUCCESS)
		{
			guszSlot1InBit = VS_TRUE;
		}
		if (inNCCC_tSAM_Check_SAM_In_Slot(_SAM_SLOT_2_) == VS_SUCCESS)
		{
			guszSlot2InBit = VS_TRUE;
		}
		if (inNCCC_tSAM_Check_SAM_In_Slot(_SAM_SLOT_3_) == VS_SUCCESS)
		{
			guszSlot3InBit = VS_TRUE;
		}
		if (inNCCC_tSAM_Check_SAM_In_Slot(_SAM_SLOT_4_) == VS_SUCCESS)
		{
			guszSlot4InBit = VS_TRUE;
		}
	}
	
	/* 純tSAM多PowerOn一次 */
	inNCCC_tSAM_PowerOn_Flow(&uszSlot);
	inRetVal = inNCCC_tSAM_SelectAID_NCCC(uszSlot);
	/* selectAID 失敗 */
	if (inRetVal != VS_SUCCESS)
	{
                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_tSAM_SelectAID_NCCC failed, uszSlot(%d)", uszSlot);
                
		/* 請檢查SAM卡 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHECK_SAM_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = 1;
		char	szHostEnable[1 + 1] = {0};
		inNCCC_Ticket_GetESVC_Enable(pobTran->srBRec.inHDTIndex, szHostEnable);
		if (szHostEnable[0] == 'N')
		{
			sprintf(srDispMsgObj.szErrMsg1, "SAM卡請放SAM1");
		}
		else
		{
			sprintf(srDispMsgObj.szErrMsg1, "SAM卡請放SAM%d", uszSlot);
		}
		srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);

		return (VS_ERROR);
	}
	
	memset(szTSAMRegisterEnable, 0x00, sizeof(szTSAMRegisterEnable));
	inGetTSAMRegisterEnable(szTSAMRegisterEnable);
	
	if ((memcmp(szEncryptMode, _NCCC_ENCRYPTION_TSAM_, 1) == 0) && (memcmp(szTSAMRegisterEnable, "Y", 1) != 0))
	{
		/* 換key */
		for (i = 0 ;; i ++)
		{
			/* 密鑰檔【SKMDef.txt】 */
			if (inLoadSKMRec(i) < 0) 
				break;

			if (inNCCC_tSAM_DeleteAllKey(uszSlot) != VS_SUCCESS)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate,"tSAM_DELETE_ALL_KEYS_ERROR_(%d)", i);
				inLogPrintf(AT, szTemplate);
				
				return (VS_SUCCESS);
			}

			break;
		}

		for (i = 0 ;; i ++)
		{
			if (inLoadSKMRec(i) < 0) /* 密鑰檔【SKMDef.txt】 */
				break;

			if (inNCCC_tSAM_DownloadKey(uszSlot) != VS_SUCCESS)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate,"tSAM_DOWNLOAD_KEYS_ERROR_(%d)", i);
				inLogPrintf(AT, szTemplate);
				
				return (VS_SUCCESS);
			}

			if (inSaveSKMRec(i) < 0) /* 密鑰檔【SKMDef.txt】 */
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate,"inSaveSKMRec(%d)", i);
				inLogPrintf(AT, szTemplate);
				
				return (VS_ERROR);
			}
		}
		
		/* 如果還沒註冊，就開機時註冊*/
		inNCCC_tSAM_Register_PowerOn();

	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_tSAM_SelectAID_NCCC_Flow
Date&Time       :2018/1/12 上午 9:57
Describe        :NCCC 信用卡的AID
*/
int inNCCC_tSAM_SelectAID_NCCC_Flow()
{
	int		inRetVal = 0;
	char		szDebugMsg[100 + 1];
	unsigned char	uszSlot = 0;
	
	/* 抓tSAM Slot */
	inRetVal = inNCCC_tSAM_Decide_tSAM_Slot(&uszSlot);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inNCCC_tSAM_Decide_tSAM_Slot Failed");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	inRetVal = inNCCC_tSAM_SelectAID_NCCC(uszSlot);
	
	return (inRetVal);
}

/*
Function        :inNCCC_tSAM_SelectAID_NCCC
Date&Time       :2016/1/6 下午 1:31
Describe        :NCCC 信用卡的AID
*/
int inNCCC_tSAM_SelectAID_NCCC(unsigned char uszSlot)
{
	int		inRetVal;
	int		inCnt = 0;
	int		inSAMAidLen = SAM_LC_SELECT_NCCC_AID;
	char		szTemplate[42 + 1];
	APDU_COMMAND	srAPDUData;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "---------------------------------------");
		inLogPrintf(AT, "inNCCC_tSAM_SelectAID_NCCC() START!!");
	}
	
	memset((char *) &srAPDUData, 0x00, APDU_LENGTH);

	srAPDUData.uszSendData[inCnt ++] = SAM_CLA_SELECT_NCCC_AID;			/* CLA 0x00 */
	srAPDUData.uszSendData[inCnt ++] = SAM_INS_SELECT_NCCC_AID;			/* INS 0xA4 */
	srAPDUData.uszSendData[inCnt ++] = SAM_P1_SELECT_NCCC_AID;			/* P1  0x04 */
	srAPDUData.uszSendData[inCnt ++] = SAM_P2_SELECT_NCCC_AID;			/* P2  0x00 */
	srAPDUData.uszSendData[inCnt ++] = SAM_LC_SELECT_NCCC_AID;			/* Lc  0x08 */

	memcpy(&srAPDUData.uszSendData[inCnt], _AID_SAM_NCCC_, inSAMAidLen);	/* Lc Data */
	inCnt += inSAMAidLen;

	srAPDUData.uszSendData[inCnt ++]= inSAMAidLen + 9;			/* Le  */
	srAPDUData.inSendLen = inCnt;

	inRetVal = inNCCC_tSAM_APDUTransmit(uszSlot, srAPDUData.uszSendData, srAPDUData.inSendLen, srAPDUData.uszRecevData, &srAPDUData.inRecevLen);

	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "inNCCC_tSAM_SelectAID_NCCC ERROR(%d)!!", inRetVal);
			inLogPrintf(AT, szTemplate);
		}
		
		return(VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_tSAM_SelectAID_NCCC() END!!!");
		inLogPrintf(AT, "---------------------------------------");
	}
	
	return(VS_SUCCESS);
}

/*
Function        :inNCCC_tSAM_PowerOn_IPASS_Flow
Date&Time       :2018/3/7 下午 2:50
Describe        :
*/
int inNCCC_tSAM_PowerOn_IPASS_Flow()
{
	int		inRetVal = 0;
	char		szSAMSlot[2 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	unsigned char	uszSlot = 0;
	
	memset(szSAMSlot, 0x00, sizeof(szSAMSlot));
	inGetTicket_SAM_Slot(szSAMSlot);
	if (memcmp(szSAMSlot, "01", strlen("01")) == 0)
	{
		uszSlot = _SAM_SLOT_1_;
	}
	else if (memcmp(szSAMSlot, "02", strlen("02")) == 0)
	{
		uszSlot = _SAM_SLOT_2_;
	}
	else if (memcmp(szSAMSlot, "03", strlen("03")) == 0)
	{
		uszSlot = _SAM_SLOT_3_;
	}
	else if (memcmp(szSAMSlot, "04", strlen("04")) == 0)
	{
		uszSlot = _SAM_SLOT_4_;
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "IPASS SAM Slot is Not Set.");
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	if (inNCCC_tSAM_Status_IsActive(uszSlot) != VS_SUCCESS)
	{
		inRetVal = inNCCC_tSAM_Slot_PowerOn(uszSlot);
		if (inRetVal != VS_SUCCESS)
		{
			/* 請檢查SAM卡 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHECK_SAM_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = 1;
			sprintf(srDispMsgObj.szErrMsg1, "SAM卡請放SAM%d", atoi(szSAMSlot));
			srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else
	{
		inRetVal = VS_SUCCESS;
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "No need Power On");
		}
	}
	
	return (inRetVal);
}

/*
Function        :inNCCC_tSAM_SelectAID_IPASS_Flow
Date&Time       :2018/1/12 上午 9:58
Describe        :
*/
int inNCCC_tSAM_SelectAID_IPASS_Flow()
{
	int		inRetVal = 0;
	char		szSAMSlot[2 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		szDemoMode[2 + 1] = {0};
	unsigned char	uszSlot = 0;
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		return (VS_SUCCESS);
	}
	else
	{
		memset(szSAMSlot, 0x00, sizeof(szSAMSlot));
		inGetTicket_SAM_Slot(szSAMSlot);
		if (memcmp(szSAMSlot, "01", strlen("01")) == 0)
		{
			uszSlot = _SAM_SLOT_1_;
		}
		else if (memcmp(szSAMSlot, "02", strlen("02")) == 0)
		{
			uszSlot = _SAM_SLOT_2_;
		}
		else if (memcmp(szSAMSlot, "03", strlen("03")) == 0)
		{
			uszSlot = _SAM_SLOT_3_;
		}
		else if (memcmp(szSAMSlot, "04", strlen("04")) == 0)
		{
			uszSlot = _SAM_SLOT_4_;
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "IPASS SAM Slot is Not Set.");
				inLogPrintf(AT, szDebugMsg);
			}
			return (VS_ERROR);
		}

		inRetVal = inNCCC_tSAM_SelectAID_IPASS(uszSlot);

		return (inRetVal);
	}
}

/*
Function        :inNCCC_tSAM_SelectAID_IPASS
Date&Time       :2018/1/4 上午 10:02
Describe        :IPASS的AID
*/
int inNCCC_tSAM_SelectAID_IPASS(unsigned char uszSlot)
{
	int		inRetVal;
	int		inCnt = 0;
	int		inSAMAidLen = SAM_LC_SELECT_IPASS_AID;
	char		szTemplate[42 + 1];
	APDU_COMMAND	srAPDUData;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "---------------------------------------");
		inLogPrintf(AT, "inNCCC_tSAM_SelectAID_IPASS() START!!");
	}
	
	memset((char *) &srAPDUData, 0x00, APDU_LENGTH);

	srAPDUData.uszSendData[inCnt ++] = SAM_CLA_SELECT_IPASS_AID;		/* CLA 0x00 */
	srAPDUData.uszSendData[inCnt ++] = SAM_INS_SELECT_IPASS_AID;		/* INS 0xA4 */
	srAPDUData.uszSendData[inCnt ++] = SAM_P1_SELECT_IPASS_AID;		/* P1  0x04 */
	srAPDUData.uszSendData[inCnt ++] = SAM_P2_SELECT_IPASS_AID;		/* P2  0x00 */
	srAPDUData.uszSendData[inCnt ++] = SAM_LC_SELECT_IPASS_AID;		/* Lc  0x08 */

	memcpy(&srAPDUData.uszSendData[inCnt], _AID_SAM_IPASS_, inSAMAidLen);	/* Lc Data */
	inCnt += inSAMAidLen;

	srAPDUData.uszSendData[inCnt ++]= 0x00;		/* Le  */
	srAPDUData.inSendLen = inCnt;

	inRetVal = inNCCC_tSAM_APDUTransmit(uszSlot, srAPDUData.uszSendData, srAPDUData.inSendLen, srAPDUData.uszRecevData, &srAPDUData.inRecevLen);

	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "inNCCC_tSAM_SelectAID_IPASS ERROR(%d)!!", inRetVal);
			inLogPrintf(AT, szTemplate);
		}
		
		return(VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_tSAM_SelectAID_IPASS() END!!!");
		inLogPrintf(AT, "---------------------------------------");
	}
	
	return(VS_SUCCESS);
}

/*
Function        :inNCCC_tSAM_PowerOn_ECC_Flow
Date&Time       :2018/3/12 下午 5:10
Describe        :
*/
int inNCCC_tSAM_PowerOn_ECC_Flow()
{
	int		inRetVal = 0;
	char		szSAMSlot[2 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	unsigned char	uszSlot = 0;
		
	memset(szSAMSlot, 0x00, sizeof(szSAMSlot));
	inGetTicket_SAM_Slot(szSAMSlot);
	if (memcmp(szSAMSlot, "01", strlen("01")) == 0)
	{
		uszSlot = _SAM_SLOT_1_;
	}
	else if (memcmp(szSAMSlot, "02", strlen("02")) == 0)
	{
		uszSlot = _SAM_SLOT_2_;
	}
	else if (memcmp(szSAMSlot, "03", strlen("03")) == 0)
	{
		uszSlot = _SAM_SLOT_3_;
	}
	else if (memcmp(szSAMSlot, "04", strlen("04")) == 0)
	{
		uszSlot = _SAM_SLOT_4_;
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ECC SAM Slot is Not Set.");
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	if (inNCCC_tSAM_Status_IsActive(uszSlot) != VS_SUCCESS)
	{
		inRetVal = inNCCC_tSAM_Slot_PowerOn(uszSlot);
		if (inRetVal != VS_SUCCESS)
		{
			/* 請檢查SAM卡 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHECK_SAM_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = 1;
			sprintf(srDispMsgObj.szErrMsg1, "SAM卡請放SAM%d", atoi(szSAMSlot));
			srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else
	{
		inRetVal = VS_SUCCESS;
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "No need Power On");
		}
	}
	
	return (inRetVal);
}

/*
Function        :inNCCC_tSAM_SelectAID_ECC_Flow
Date&Time       :2018/3/12 下午 5:10
Describe        :
*/
int inNCCC_tSAM_SelectAID_ECC_Flow()
{
	int		inRetVal = 0;
	char		szSAMSlot[2 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		szDemoMode[2 + 1] = {0};
	unsigned char	uszSlot = 0;
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		return (VS_SUCCESS);
	}
	else
	{
		memset(szSAMSlot, 0x00, sizeof(szSAMSlot));
		inGetTicket_SAM_Slot(szSAMSlot);
		if (memcmp(szSAMSlot, "01", strlen("01")) == 0)
		{
			uszSlot = _SAM_SLOT_1_;
		}
		else if (memcmp(szSAMSlot, "02", strlen("02")) == 0)
		{
			uszSlot = _SAM_SLOT_2_;
		}
		else if (memcmp(szSAMSlot, "03", strlen("03")) == 0)
		{
			uszSlot = _SAM_SLOT_3_;
		}
		else if (memcmp(szSAMSlot, "04", strlen("04")) == 0)
		{
			uszSlot = _SAM_SLOT_4_;
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				snprintf(szDebugMsg, sizeof(szDebugMsg), "ECC SAM Slot is Not Set.");
				inLogPrintf(AT, szDebugMsg);
			}
			return (VS_ERROR);
		}
		
		/*[需求單-115011] 新規格MSAM悠遊卡載入AID調整專案
		*[20260317] 因要自動選擇二個AID，修改原本只使用 inNCCC_tSAM_SelectAID_ECC(),
		* 改成inNCCC_tSAM_PselectSamAidMulti_ECC() 自動重選AID功能 */
		inRetVal = inNCCC_tSAM_PselectSamAidMulti_ECC(uszSlot);

		return (inRetVal);
	}
}

/*
Function        :inNCCC_tSAM_SelectAID_ECC
Date&Time       :2018/1/4 上午 10:02
Describe        :ECC的AID
[需求單-115011] 新規格MSAM悠遊卡載入AID調整專案 [20260317] 
[20260317] 修改函式多傳入指定AID的參數
*/
int inNCCC_tSAM_SelectAID_ECC(unsigned char uszSlot, int inAidValue)
{
	int		inRetVal;
	int		inCnt = 0;
	int		inSAMAidLen = SAM_LC_SELECT_ECC_AID;
	char		szTemplate[42 + 1];
	APDU_COMMAND	srAPDUData;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "---------------------------------------");
		inLogPrintf(AT, "inNCCC_tSAM_SelectAID_ECC() START!!");
	}
	
	memset((char *) &srAPDUData, 0x00, APDU_LENGTH);

	srAPDUData.uszSendData[inCnt ++] = SAM_CLA_SELECT_ECC_AID;		/* CLA 0x00 */
	srAPDUData.uszSendData[inCnt ++] = SAM_INS_SELECT_ECC_AID;		/* INS 0xA4 */
	srAPDUData.uszSendData[inCnt ++] = SAM_P1_SELECT_ECC_AID;		/* P1  0x04 */
	srAPDUData.uszSendData[inCnt ++] = SAM_P2_SELECT_ECC_AID;		/* P2  0x00 */
	srAPDUData.uszSendData[inCnt ++] = SAM_LC_SELECT_ECC_AID;		/* Lc  0x08 */
	
	/*[需求單-115011] 新規格MSAM悠遊卡載入AID調整專案
	 *[20260317] 傳入參數如不為801,則使用JavaCard 0x0501 AID進行 */
	if(inAidValue == 801)
		memcpy(&srAPDUData.uszSendData[inCnt], _AID_SAM_ECC_0801_, inSAMAidLen);	/* Lc Data */
	else
		memcpy(&srAPDUData.uszSendData[inCnt], _AID_SAM_ECC_0501_, inSAMAidLen);	/* Lc Data */
	inCnt += inSAMAidLen;

	srAPDUData.uszSendData[inCnt ++]= 0x00;		/* Le  */
	srAPDUData.inSendLen = inCnt;

	inRetVal = inNCCC_tSAM_APDUTransmit(uszSlot, srAPDUData.uszSendData, srAPDUData.inSendLen, srAPDUData.uszRecevData, &srAPDUData.inRecevLen);

	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			snprintf(szTemplate, sizeof(szTemplate), "inNCCC_tSAM_SelectAID_ECC ERROR(%d)!!", inRetVal);
			inLogPrintf(AT, szTemplate);
		}
		
		return(VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_tSAM_SelectAID_ECC() END!!!");
		inLogPrintf(AT, "---------------------------------------");
	}
	
	return(VS_SUCCESS);
}

/*
Function        :inNCCC_tSAM_PowerOn_ICASH_Flow
Date&Time       :2019/11/14 下午 5:07
Describe        :
*/
int inNCCC_tSAM_PowerOn_ICASH_Flow()
{
	int		inRetVal = 0;
	char		szSAMSlot[2 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	unsigned char	uszSlot = 0;
		
	memset(szSAMSlot, 0x00, sizeof(szSAMSlot));
	inGetTicket_SAM_Slot(szSAMSlot);
	if (memcmp(szSAMSlot, "01", strlen("01")) == 0)
	{
		uszSlot = _SAM_SLOT_1_;
	}
	else if (memcmp(szSAMSlot, "02", strlen("02")) == 0)
	{
		uszSlot = _SAM_SLOT_2_;
	}
	else if (memcmp(szSAMSlot, "03", strlen("03")) == 0)
	{
		uszSlot = _SAM_SLOT_3_;
	}
	else if (memcmp(szSAMSlot, "04", strlen("04")) == 0)
	{
		uszSlot = _SAM_SLOT_4_;
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ICASH SAM Slot is Not Set.");
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	if (inNCCC_tSAM_Status_IsActive(uszSlot) != VS_SUCCESS)
	{
		inRetVal = inNCCC_tSAM_Slot_PowerOn(uszSlot);
		if (inRetVal != VS_SUCCESS)
		{
			/* 請檢查SAM卡 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHECK_SAM_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = 1;
			sprintf(srDispMsgObj.szErrMsg1, "SAM卡請放SAM%d", atoi(szSAMSlot));
			srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else
	{
		inRetVal = VS_SUCCESS;
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "No need Power On");
		}
	}
	
	return (inRetVal);
}

/*
Function        :inNCCC_tSAM_SelectAID_ICASH_Flow
Date&Time       :2019/11/14 下午 5:07
Describe        :
*/
int inNCCC_tSAM_SelectAID_ICASH_Flow()
{
	int		inRetVal = 0;
	char		szSAMSlot[2 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		szDemoMode[2 + 1] = {0};
	unsigned char	uszSlot = 0;
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		return (VS_SUCCESS);
	}
	else
	{
		memset(szSAMSlot, 0x00, sizeof(szSAMSlot));
		inGetTicket_SAM_Slot(szSAMSlot);
		if (memcmp(szSAMSlot, "01", strlen("01")) == 0)
		{
			uszSlot = _SAM_SLOT_1_;
		}
		else if (memcmp(szSAMSlot, "02", strlen("02")) == 0)
		{
			uszSlot = _SAM_SLOT_2_;
		}
		else if (memcmp(szSAMSlot, "03", strlen("03")) == 0)
		{
			uszSlot = _SAM_SLOT_3_;
		}
		else if (memcmp(szSAMSlot, "04", strlen("04")) == 0)
		{
			uszSlot = _SAM_SLOT_4_;
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "ICASH SAM Slot is Not Set.");
				inLogPrintf(AT, szDebugMsg);
			}
			return (VS_ERROR);
		}

		inRetVal = inNCCC_tSAM_SelectAID_ICASH(uszSlot);

		return (inRetVal);
	}
}

/*
Function        :inNCCC_tSAM_SelectAID_ICASH
Date&Time       :2019/11/14 下午 5:08
Describe        :ICASH的AID
*/
int inNCCC_tSAM_SelectAID_ICASH(unsigned char uszSlot)
{
	int		inRetVal;
	int		inCnt = 0;
	int		inSAMAidLen = SAM_LC_SELECT_ICASH_AID;
	char		szTemplate[42 + 1];
	APDU_COMMAND	srAPDUData;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "---------------------------------------");
		inLogPrintf(AT, "inNCCC_tSAM_SelectAID_ICASH() START!!");
	}
	
	memset((char *) &srAPDUData, 0x00, APDU_LENGTH);

	srAPDUData.uszSendData[inCnt ++] = SAM_CLA_SELECT_ICASH_AID;		/* CLA 0x00 */
	srAPDUData.uszSendData[inCnt ++] = SAM_INS_SELECT_ICASH_AID;		/* INS 0xA4 */
	srAPDUData.uszSendData[inCnt ++] = SAM_P1_SELECT_ICASH_AID;		/* P1  0x04 */
	srAPDUData.uszSendData[inCnt ++] = SAM_P2_SELECT_ICASH_AID;		/* P2  0x00 */
	srAPDUData.uszSendData[inCnt ++] = SAM_LC_SELECT_ICASH_AID;		/* Lc  0x08 */

	memcpy(&srAPDUData.uszSendData[inCnt], _AID_SAM_ICASH_, inSAMAidLen);	/* Lc Data */
	inCnt += inSAMAidLen;

	srAPDUData.uszSendData[inCnt ++]= 0x00;		/* Le  */
	srAPDUData.inSendLen = inCnt;

	inRetVal = inNCCC_tSAM_APDUTransmit(uszSlot, srAPDUData.uszSendData, srAPDUData.inSendLen, srAPDUData.uszRecevData, &srAPDUData.inRecevLen);

	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "inNCCC_tSAM_SelectAID_ICASH ERROR(%d)!!", inRetVal);
			inLogPrintf(AT, szTemplate);
		}
		
		return(VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_tSAM_SelectAID_ICASH() END!!!");
		inLogPrintf(AT, "---------------------------------------");
	}
	
	return(VS_SUCCESS);
}

/*
Function        :inNCCC_tSAM_DeleteAllKey
Date&Time       :2016/1/6 下午 3:19
Describe        :刪除 tSAM Key
*/
int inNCCC_tSAM_DeleteAllKey(unsigned char uszSlot)
{
	int		inCnt= 0;
	APDU_COMMAND 	srAPDUData;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "---------------------------------------");
		inLogPrintf(AT, "inNCCC_tSAM_DeleteAllKey() START!!");
	}

	memset(&srAPDUData, 0x00, sizeof(srAPDUData));

	/* 
	   SAM_CLA_COMMAND		= 0x80
           SAM_INS_DELETE_ALL_KEY	= 0xEE
           SAM_P1_00			= 0x00
           SAM_P2_00			= 0x00
     	*/
	srAPDUData.uszSendData[inCnt ++] = SAM_CLA_DELETE_ALL_KEY;	/* CLA 0x80 */
	srAPDUData.uszSendData[inCnt ++] = SAM_INS_DELETE_ALL_KEY;	/* INS 0xEE */
	srAPDUData.uszSendData[inCnt ++] = SAM_P1_DELETE_ALL_KEY;	/* P1  0x00 */
	srAPDUData.uszSendData[inCnt ++] = SAM_P2_DELETE_ALL_KEY;	/* P2  0x00 */

	/* 被騙惹，520這邊都塞空字串，看Logport才知道 Start */
	/* 被騙惹，520這邊都塞空字串，看Logport才知道 End */

	/* 將inCnt算的長度放入APDU結構 */
	srAPDUData.inSendLen = inCnt;
	
	if (inNCCC_tSAM_APDUTransmit(uszSlot, srAPDUData.uszSendData, srAPDUData.inSendLen, srAPDUData.uszRecevData, &srAPDUData.inRecevLen) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_tSAM_DeleteAllKey() ERROR!!");
		}
                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_tSAM_DeleteAllKey failed");
		
		/* SAM卡讀取錯誤 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_READ_SAM_);
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

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_tSAM_DeleteAllKey() END!!!");
		inLogPrintf(AT, "---------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_tSAM_DownloadKey
Date&Time       :2016/1/6 下午 3:21
Describe        :下載 tSAM Key
*/
int inNCCC_tSAM_DownloadKey(unsigned char uszSlot)
{
	int		inCnt= 0;
	char 		szSessionData[16 + 1];
	char		szASCII[32 + 1], szBCD[16 + 1];
	APDU_COMMAND 	srAPDUData;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "---------------------------------------");
		inLogPrintf(AT, "inNCCC_tSAM_DownloadKey() START!!");
		
	}

	memset(&srAPDUData, 0x00, sizeof(srAPDUData));

	/* SAM_CLA_COMMAND = 0x80
	   SAM_INS_DOWNLOAD_KEY = 0xF0
	   SAM_P1_00 = 0x00
	   SAM_P2_00 = 0x00
	*/
	srAPDUData.uszSendData[inCnt ++] = SAM_CLA_DOWNLOAD_KEY;		/* CLA 0x80 */
  	srAPDUData.uszSendData[inCnt ++] = SAM_INS_DOWNLOAD_KEY;		/* INS 0xF0 */
  	srAPDUData.uszSendData[inCnt ++] = SAM_P1_DOWNLOAD_KEY;		/* P1 0x00 */
  	srAPDUData.uszSendData[inCnt ++] = SAM_P2_DOWNLOAD_KEY;		/* P2 0x00 */

	/* Lc */
	srAPDUData.uszSendData[inCnt ++] = SAM_LC_DOWNLOAD_KEY;
	
	/* Lc Data */
	/* Session Data */
	memset(szASCII, 0x00, sizeof(szASCII));
	memset(szSessionData, 0x00, sizeof(szSessionData));
	
	inGetSessionData(szASCII);
	inFunc_ASCII_to_BCD((unsigned char*)szSessionData, szASCII, 16);
	memcpy(&srAPDUData.uszSendData[inCnt], szSessionData, KEY_SESSION_DATA_SIZE);
	inCnt += KEY_SESSION_DATA_SIZE;
	
	/* BIN */
	memset(szASCII, 0x00, sizeof(szASCII));
	memset(szBCD, 0x00, sizeof(szBCD));
	
	inGetTsamBIN(szASCII);
	inFunc_ASCII_to_BCD((unsigned char*)szBCD, szASCII, 3);
	memcpy(&srAPDUData.uszSendData[inCnt], &szBCD[0], 3);
	inCnt += 3;
	
	/* Key Set */
	memset(szASCII, 0x00, sizeof(szASCII));
	memset(szBCD, 0x00, sizeof(szBCD));
	
	inGetKeySet(szASCII);
	inFunc_ASCII_to_BCD((unsigned char*)szBCD, szASCII, 1);
	memcpy(&srAPDUData.uszSendData[inCnt], &szBCD[0], 1);
	inCnt += 1;
	
	/* Key Index */
	memset(szASCII, 0x00, sizeof(szASCII));
	memset(szBCD, 0x00, sizeof(szBCD));
	
	inGetKeyIndex(szASCII);
	inFunc_ASCII_to_BCD((unsigned char*)szBCD, szASCII, 1);
	memcpy(&srAPDUData.uszSendData[inCnt], &szBCD[0], 1);
	inCnt += 1;
	
	/* Double Length */
	memset(&srAPDUData.uszSendData[inCnt], 0x10, 1);
	inCnt += 1;
	
	/* Key Data Value */
	memset(szASCII, 0x00, sizeof(szASCII));
	memset(szBCD, 0x00, sizeof(szBCD));
	
	inGetKeyValue(szASCII);
	inFunc_ASCII_to_BCD((unsigned char*)szBCD, szASCII, 16);
	memcpy(&srAPDUData.uszSendData[inCnt], &szBCD[0], 16);
	inCnt += 16;
	
	/* CVL */
	memset(&srAPDUData.uszSendData[inCnt], 0x03, 1);
	inCnt += 1;
	
	/* Check Value */
	memset(szASCII, 0x00, sizeof(szASCII));
	memset(szBCD, 0x00, sizeof(szBCD));
	
	inGetKeyCheckValue(szASCII);
	inFunc_ASCII_to_BCD((unsigned char*)szBCD, szASCII, 3);
	memcpy(&srAPDUData.uszSendData[inCnt], &szBCD[0], 3);
	inCnt += 3;
	
	/* Le 0x00 */
	srAPDUData.uszSendData[inCnt ++] = SAM_LE_DOWNLOAD_KEY;
	/* 將inCnt算的長度放入APDU結構 */
	srAPDUData.inSendLen = inCnt;
	
	if (inNCCC_tSAM_APDUTransmit(uszSlot, srAPDUData.uszSendData, srAPDUData.inSendLen, srAPDUData.uszRecevData, &srAPDUData.inRecevLen) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_tSAM_DownloadKey() ERROR!!");
		}
                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_tSAM_DownloadKey() ERROR!!");
		
		/* SAM卡讀取錯誤 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_READ_SAM_);
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

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_tSAM_DownloadKey() END!!!");
		inLogPrintf(AT, "---------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_tSAM_GetSlotSN1_Flow
Date&Time       :2018/1/12 上午 10:41
Describe        :
*/
int inNCCC_tSAM_GetSlotSN1_Flow()
{
	int		inRetVal = VS_SUCCESS;
	char		szSlot1SN[16 + 1];
	unsigned char	uszSlot = _SAM_SLOT_1_;
	
	memset(szSlot1SN, 0x00, sizeof(szSlot1SN));
	inRetVal = inNCCC_tSAM_GetSlotSN(uszSlot, szSlot1SN);
	if (inRetVal == VS_SUCCESS)
	{
		inSetSAMSlotSN1(szSlot1SN);
		inSaveEDCRec(0);
	}
	else
	{
		inNCCC_tSAM_Slot_PowerOn(uszSlot);
		/* 如果失敗就select 一次AID(目前發生在MSAM上) */
		inNCCC_tSAM_SelectAID_NCCC(uszSlot);
		memset(szSlot1SN, 0x00, sizeof(szSlot1SN));
		inRetVal = inNCCC_tSAM_GetSlotSN(uszSlot, szSlot1SN);
		if (inRetVal == VS_SUCCESS)
		{
			inSetSAMSlotSN1(szSlot1SN);
			inSaveEDCRec(0);
		}
		else
		{
			/* 再失敗就清空 */
			inSetSAMSlotSN1("                ");
			inSaveEDCRec(0);
		}
	}
		
	return (inRetVal);
}

/*
Function        :inNCCC_tSAM_GetSlotSN2_Flow
Date&Time       :2018/1/12 上午 10:41
Describe        :
*/
int inNCCC_tSAM_GetSlotSN2_Flow()
{
	int		inRetVal = VS_SUCCESS;
	char		szSlot2SN[16 + 1];
	unsigned char	uszSlot = _SAM_SLOT_2_;
	
	memset(szSlot2SN, 0x00, sizeof(szSlot2SN));
	inRetVal = inNCCC_tSAM_GetSlotSN(uszSlot, szSlot2SN);
	if (inRetVal == VS_SUCCESS)
	{
		inSetSAMSlotSN2(szSlot2SN);
		inSaveEDCRec(0);
	}
	else
	{
		inNCCC_tSAM_Slot_PowerOn(uszSlot);
		/* 如果失敗就select 一次AID(目前發生在MSAM上) */
		inNCCC_tSAM_SelectAID_NCCC(uszSlot);
		memset(szSlot2SN, 0x00, sizeof(szSlot2SN));
		inRetVal = inNCCC_tSAM_GetSlotSN(uszSlot, szSlot2SN);
		if (inRetVal == VS_SUCCESS)
		{
			inSetSAMSlotSN2(szSlot2SN);
			inSaveEDCRec(0);
		}
		else
		{
			/* 再失敗就清空 */
			inSetSAMSlotSN2("                ");
			inSaveEDCRec(0);
		}
	}
		
	return (inRetVal);
}

/*
Function        :inNCCC_tSAM_GetSlotSN
Date&Time       :2016/1/6 上午 11:24
Describe        :取得SAM卡Serier Number
*/
int inNCCC_tSAM_GetSlotSN(unsigned char uszSlot, char *szSlotSN)
{
	int		inRetVal;
	int		inCnt = 0;
	int		inSNBuffCnt = 0;
	char		szTemplate[42 + 1];
	char		szDebugMsg[100 + 1];
	APDU_COMMAND	srAPDUData;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "---------------------------------------");
		inLogPrintf(AT, "inNCCC_tSAM_GetSlotSN START!!");
	}

	memset((char *) &srAPDUData, 0x00, APDU_LENGTH);
	srAPDUData.uszSendData[inCnt ++] = SAM_CLA_GET_SN;	/* CLA 0x80 */
	srAPDUData.uszSendData[inCnt ++] = SAM_INS_GET_SN;	/* INS 0xCA */
	srAPDUData.uszSendData[inCnt ++] = SAM_P1_GET_SN;	/* P1 0x9F */
	srAPDUData.uszSendData[inCnt ++] = SAM_P2_GET_SN;	/* P2 0x7F */
	srAPDUData.uszSendData[inCnt ++] = SAM_LE_GET_SN;	/* Le 0x2D */
	srAPDUData.inSendLen = inCnt;

	inRetVal = inNCCC_tSAM_APDUTransmit(uszSlot, srAPDUData.uszSendData, srAPDUData.inSendLen, srAPDUData.uszRecevData, &srAPDUData.inRecevLen);

	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inNCCC_tSAM_GetSlotSN Fail(Slot:%d)", uszSlot);
			inLogPrintf(AT, szDebugMsg);
		}
		return(VS_ERROR);
	}
	else
	{
		/*
		 * lenth	:index
		 * 2		:0	TAG
		 * 1		:2	LENGTH
		 * 2		:3	IC Fabricator
		 * 2		:5	IC Type
		 * 2		:7	Operating System Identifier
		 * 2		:9	Operation System Release Date
		 * 2		:11	Operation System Release Level
		 * 2		:13	IC Fabrication Date (2 BYTE)
		 * 4		:15	IC Serial Number (4 BYTE)
		 * 2		:19	IC Batch Identifier (2 BYTE)

			資料組合順序
			IC Fabrication Date(4碼)
			IC Batch Identifier(4碼)
			IC Serial No.(8碼)
		*/
		inSNBuffCnt = 0;
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[inSNBuffCnt], &srAPDUData.uszRecevData[13], 2);
		inSNBuffCnt +=2;
		
		memcpy(&szTemplate[inSNBuffCnt], &srAPDUData.uszRecevData[19], 2);
		inSNBuffCnt +=2;
		
		memcpy(&szTemplate[inSNBuffCnt], &srAPDUData.uszRecevData[15], 4);
		inSNBuffCnt +=4;
		
		inFunc_BCD_to_ASCII(szSlotSN, (unsigned char*)szTemplate, 8);
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "tSAM SN = %s", szSlotSN);
		inLogPrintf(AT, szTemplate);
		inLogPrintf(AT, "inNCCC_tSAM_GetSlotSN END!!!");
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inGet_Slot_Type() END!!");
		inLogPrintf(AT, "---------------------------------------");
	}
	
	return(VS_SUCCESS);
}

/*
Function        :inNCCC_tSAM_Register_Menu
Date&Time       :2016/1/6 下午 4:23
Describe        :1. tSAM 註冊
                 2. Load CFGT Table -> inGetEncryptionMode()
                                    ->_NCCC_ENCRYPTION_tSAM_ = 1
*/
int inNCCC_tSAM_Register_Menu(TRANSACTION_OBJECT *pobTran)
{
	char		szTMSOK[2 + 1];
	char		szEncrptMode[1 + 1];
	unsigned char	uszKey;
	
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_tSAM_Register_Menu START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Func_tSAM_RegisterMenu() START!!");
		inLogPrintf(AT, "---------------------------------------");
	}

	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	if (szTMSOK[0] != 'Y')
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_tSAM_FuncF3RegisterMenu() ERROR_1");
		}
		
		/* 請執行參數下載 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_TMS_DWL_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);
		
    		return (VS_SUCCESS); /* Modify By Tsunglin 2009-12-07 PM 02:04:54 - 回傳成功可以開機到 IDLE  */
	}

	memset(szEncrptMode, 0x00, sizeof(szEncrptMode));
	inGetEncryptMode(szEncrptMode);
	
	/* 不是TSAM加密就跳出 */
	if (memcmp(szEncrptMode, _NCCC_ENCRYPTION_TSAM_, 1) != 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "加密模式非tSAM，不註冊tSAM");
		}
		pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;
		
		return (VS_SUCCESS);
	}

	/* 顯示 "tSAM註冊" */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_TSAM_REGISTER_, 0, _COORDINATE_Y_LINE_8_4_); /* tSAM註冊 */

	/* 成功則紀錄已註冊 */
	if (inNCCC_tSAM_SLOT1RegisterTID() == VS_SUCCESS)
        {
		/* 註冊SAM卡完成請按0確認 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_TSAM_REGISTER_OK_, 0, _COORDINATE_Y_LINE_8_4_);
		inDISP_BEEP(1, 0);

		while (1)
		{
			uszKey = uszKBD_GetKey(30);

			if (uszKey == _KEY_0_ || _KEY_TIMEOUT_)
			{            
				break;
			}
			else
			{
				continue;
			}

		}
		
		inSetTSAMRegisterEnable("Y");
        }
	else
	{
		/* 註冊SAM卡失敗 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_REGISTER_SAM_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);
		
		/* 註冊失敗不應該再把TSAMRegisterEnable設為0 */
	}

	if (inSaveEDCRec(0) < 0)
	{
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Func_tSAM_RegisterMenu() END!!!");
		inLogPrintf(AT, "---------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_tSAM_Register_PowerOn
Date&Time       :2016/11/14 下午 1:20
Describe        :1. tSAM 註冊
                 2. Load CFGT Table -> inGetEncryptionMode()
                                    ->_NCCC_ENCRYPTION_tSAM_ = 1
*/
int inNCCC_tSAM_Register_PowerOn()
{
	char		szTMSOK[2 + 1];
	char		szEncrptMode[1 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Func_tSAM_RegisterMenu() START!!");
		inLogPrintf(AT, "---------------------------------------");
	}

	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	if (szTMSOK[0] != 'Y')
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_tSAM_FuncF3RegisterMenu() ERROR_1");
		}
		
    		return (VS_SUCCESS); /* Modify By Tsunglin 2009-12-07 PM 02:04:54 - 回傳成功可以開機到 IDLE  */
	}

	memset(szEncrptMode, 0x00, sizeof(szEncrptMode));
	inGetEncryptMode(szEncrptMode);
	
	/* 不是TSAM加密就跳出 */
	if (memcmp(szEncrptMode, _NCCC_ENCRYPTION_TSAM_, 1) != 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "加密模式非tSAM，不註冊tSAM");
		}
		
		return (VS_SUCCESS);
	}

	/* 成功則紀錄已註冊 */
	if (inNCCC_tSAM_SLOT1RegisterTID() == VS_SUCCESS)
        {
		inSetTSAMRegisterEnable("Y");
        }
	else
	{
		/* 註冊失敗不應該再把TSAMRegisterEnable設為0 */
	}

	if (inSaveEDCRec(0) < 0)
	{
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Func_tSAM_RegisterMenu() END!!!");
		inLogPrintf(AT, "---------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_tSAM_SLOT1RegisterTID
Date&Time       :2016/1/6 下午 5:09
Describe        :tSAM註冊TID
 *		 在此function get Terminal ID 並轉成bcd格式
*/
int inNCCC_tSAM_SLOT1RegisterTID()
{
	int		inRetVal = VS_ERROR;
	char		szDebugMsg[100 + 1];
	unsigned char	uszTIDAscii[8 + 1];
	unsigned char	uszTIDBcd[4 + 1];
	unsigned char	uszSlot = 0;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "---------------------------------------");
		inLogPrintf(AT, "tSAM_SLOT1RegisterTID() START!!");
	}
	
	if (inLoadHDTRec(0) < 0)
	{	
		return (VS_ERROR);
	}

	/* Get TID */
	memset(uszTIDAscii, 0x00, sizeof(uszTIDAscii));
	inGetTerminalID((char*)uszTIDAscii);
	/* Pack TID */
	memset(uszTIDBcd, 0x00, sizeof(uszTIDBcd));
	inFunc_ASCII_to_BCD(uszTIDBcd, (char*)uszTIDAscii, 4);

	/* 抓tSAM Slot */
	inRetVal = inNCCC_tSAM_Decide_tSAM_Slot(&uszSlot);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inNCCC_tSAM_Decide_tSAM_Slot Failed");
			inLogPrintf(AT, szDebugMsg);
		}
	}
		
	if (inNCCC_tSAM_APDURegisterTID(uszSlot, uszTIDBcd) == VS_ERROR)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "tSAM_SLOT1RegisterTID() ERROR!!");
		}
		
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "tSAM_SLOT1RegisterTID() END!!!");
			inLogPrintf(AT, "---------------------------------------");
		}
		
		return (VS_ERROR);
	}
	else
	{
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "tSAM_SLOT1RegisterTID() END!!!");
			inLogPrintf(AT, "---------------------------------------");
		}

		return (VS_SUCCESS);
	}
}

/*
Function        :inNCCC_tSAM_APDURegisterTID
Date&Time       :2016/1/6 下午 5:47
Describe        :於交易時，field2，field35欄位需以SAM卡加密，此function依SAM卡規格下APDU Command註冊TID。
		 組TID註冊命令
*/
int inNCCC_tSAM_APDURegisterTID(unsigned char uszSlot, unsigned char *uszTID)
{
	int 		inCnt = 0, inRetVal;
	char		szTemplate[42 + 1];
	APDU_COMMAND 	srAPDUData;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "---------------------------------------");
		inLogPrintf(AT, "tSAM_APDURegisterTID() START!!");
	}
	
	memset((char *) &srAPDUData, 0x00, APDU_LENGTH);

	srAPDUData.uszSendData[inCnt ++] = SAM_CLA_REGISTER_TID; 	/* CLA = 0x80 */
	srAPDUData.uszSendData[inCnt ++] = SAM_INS_REGISTER_TID;		/* INS = 0xEA */
	srAPDUData.uszSendData[inCnt ++] = SAM_P1_REGISTER_TID; 		/* P1 = 0x00 */
	srAPDUData.uszSendData[inCnt ++] = SAM_P2_REGISTER_TID; 		/* P2 = 0x00 */
	srAPDUData.uszSendData[inCnt ++] = SAM_LC_REGISTER_TID; 		/* Lc = 0x04 */
	memcpy(&srAPDUData.uszSendData[inCnt], uszTID, 4);		/* Lc Data */
	inCnt += 4;
	
	/* 將inCnt算的長度放入APDU結構 */
	srAPDUData.inSendLen = inCnt;

	inRetVal = inNCCC_tSAM_APDUTransmit(uszSlot, srAPDUData.uszSendData, srAPDUData.inSendLen, srAPDUData.uszRecevData, &srAPDUData.inRecevLen);

	if (inRetVal != VS_SUCCESS)
	{
		
		/* 避免工程師重覆註冊，只要SAM卡回覆6986且APDU加密Command成功，便忽略此錯誤訊息。 */
		/* inNCCC_tSAM_CheckRegisterTID 用到的inNCCC_tSAM_Encrypt會重新selectAID 這裡不再做處理 */
		if (inNCCC_tSAM_CheckRegisterTID(uszSlot, &srAPDUData, uszTID) == VS_SUCCESS)
		{
			return (VS_SUCCESS);
		}
		
		if (ginDebug == VS_TRUE)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "Register Fail: SAM Response %02x%02x!!",  srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2], srAPDUData.uszRecevData[srAPDUData.inRecevLen - 1]);
			inLogPrintf(AT, szTemplate);
		}
		vdUtility_SYSFIN_LogMessage(AT, "Register Fail: SAM Response %02x%02x!!",  srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2], srAPDUData.uszRecevData[srAPDUData.inRecevLen - 1]);
                
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "tSAM_APDURegisterTID() END!!!");
		inLogPrintf(AT, "---------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_tSAM_CheckRegisterTID
Date&Time       :2016/1/7 上午 9:55
Describe        :避免工程師重覆註冊，只要SAM卡回覆6986且APDU加密Command成功，便忽略此錯誤訊息。
*/
int inNCCC_tSAM_CheckRegisterTID(unsigned char uszSlot, APDU_COMMAND *srAPDUData, unsigned char *uszTID)
{
	int		inRealLength = 32;
	char		szEncryptData[40 + 1];
	unsigned char 	uszRRN[8 + 1];		/* 原為RRN(11 digits)補到16碼，這邊只測試加密是否成功，所以輸入加密需最小長度8位 */
	unsigned char 	uszCheckSum[4 + 1];
	unsigned char 	uszKeyindex = 0x00;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "---------------------------------------");
		inLogPrintf(AT, "tSAM_CheckRegisterTID() START!!");
	}

	/* 隨便塞值，因不會解密故不影響加密 */
	memset(szEncryptData, 0x30, sizeof(szEncryptData));
	sprintf((char *)uszRRN, "12345678");

        if (!memcmp(&srAPDUData->uszRecevData[srAPDUData->inRecevLen - 2], "\x69\x86", 2))
	{
		/* 使用加密 APDU Command 確認是否是原本的TID */
		if (inNCCC_tSAM_Encrypt(uszSlot, 
					inRealLength,
				  	szEncryptData,
				  	uszRRN,
				  	uszTID,
				  	&uszKeyindex,
				  	uszCheckSum) == VS_ERROR)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "TSAM加密失敗");
				inLogPrintf(AT, "tSAM_CheckRegisterTID_Encrypt_ERROR!!");
			}
			
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "tSAM_CheckRegisterTID END!!");
				inLogPrintf(AT, "---------------------------------------");
			}
			
			return (VS_SUCCESS);
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "tSAM_CheckRegisterTID_ERROR!!");
		}
		
		return (VS_ERROR);
	}
}

/*
Function        :inNCCC_tSAM_Status_IsActive
Date&Time       :2016/5/20 下午 1:47
Describe        :確認該slot有沒有power on，SAM卡無法確認是否插入，只能確認是否Power On
*/
int inNCCC_tSAM_Status_IsActive(unsigned char uszSlotNum)
{
	char		szDebugMsg[100 + 1];
	unsigned char	uszStatus;
	unsigned short	usRetVal;
	
	/* 判斷SAM卡狀態是否Ready */
	usRetVal = CTOS_SCStatus(uszSlotNum, &uszStatus);
	
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_tSAM_Status_IsActive ERROR!!");
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "錯誤代碼： 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return(VS_ERROR);
	}
	
	/* d_MK_SC_ACTIVE為Ready狀態 */
	if ((uszStatus & d_MK_SC_ACTIVE) == d_MK_SC_ACTIVE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "ICC is actived");
		}
		
		return (VS_SUCCESS);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "ICC not actived");
		}
		
		return (VS_ERROR);
	}
}

/*
Function        :inNCCC_tSAM_Status_IsPresented
Date&Time       :2018/5/10 上午 10:10
Describe        :確認該slot有沒有Present，SAM卡無法確認是否插入，只能確認是否Power On
*/
int inNCCC_tSAM_Status_IsPresented(unsigned char uszSlotNum)
{
	char		szDebugMsg[100 + 1];
	unsigned char	uszStatus;
	unsigned short	usRetVal;
	
	/* 判斷SAM卡狀態是否Ready */
	usRetVal = CTOS_SCStatus(uszSlotNum, &uszStatus);
	
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_tSAM_Status_IsPresented ERROR!!");
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "錯誤代碼： 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return(VS_ERROR);
	}
	
	/* d_MK_SC_ACTIVE為Ready狀態 */
	if ((uszStatus & d_MK_SC_PRESENT) == d_MK_SC_PRESENT)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "ICC is Presented");
		}
		
		return (VS_SUCCESS);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "ICC is not Presented");
		}
		
		return (VS_ERROR);
	}
}

/*
Function        :inNCCC_tSAM_Slot_PowerOn
Date&Time       :2016/5/20 下午 1:47
Describe        :Power On Slot
*/
int inNCCC_tSAM_Slot_PowerOn(unsigned char uszSlot)
{
	char		szDebugMsg[100 + 1];
	unsigned char	uszATR[128 + 1], uszATRLen, uszCardType;
	unsigned short	usRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_tSAM_Slot_PowerOn(%d) START !", uszSlot);
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* Power on the ICC and return the ATR which meets ISO-7816 specification.(Smart card) */
	uszATRLen = sizeof(uszATR);
	
	usRetVal = CTOS_SCResetISO(uszSlot, d_SC_5V, uszATR, &uszATRLen, &uszCardType);
	
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_tSAM_Slot_PowerOn ERROR!!");
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "錯誤代碼： 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		inUtility_StoreTraceLog_OneStep("%d Slot PowerOn Fail", uszSlot);
		inUtility_StoreTraceLog_OneStep("錯誤代碼： 0x%04x", usRetVal);
		
		return(VS_ERROR);
	}
	else
	{
		
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_tSAM_Slot_PowerOn() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_tSAM_Slot_PowerOff
Date&Time       :2019/6/27 下午 2:44
Describe        :Power Off Slot
*/
int inNCCC_tSAM_Slot_PowerOff(unsigned char uszSlot)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_tSAM_Slot_PowerOff(%d) START !", uszSlot);
		inLogPrintf(AT, szDebugMsg);
	}
	
	usRetVal = CTOS_SCPowerOff(uszSlot);
	
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_tSAM_Slot_PowerOff ERROR!!");
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "錯誤代碼： 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return(VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_tSAM_Slot_PowerOff() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_tSAM_Slot_PowerOn_Warm
Date&Time       :2018/1/25 下午 4:08
Describe        :Power On Slot
*/
int inNCCC_tSAM_Slot_PowerOn_Warm(unsigned char uszSlot)
{
	char		szDebugMsg[100 + 1];
	unsigned char	uszATR[128 + 1], uszATRLen, uszCardType;
	unsigned short	usRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_tSAM_Slot_PowerOn_Warm() START !");
	}
	
	/* Power on the ICC and return the ATR which meets ISO-7816 specification.(Smart card) */
	uszATRLen = sizeof(uszATR);
	
	usRetVal = CTOS_SCWarmResetISO(uszSlot, d_SC_5V, uszATR, &uszATRLen, &uszCardType);
	
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_tSAM_Slot_PowerOn_Warm ERROR!!");
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "錯誤代碼： 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return(VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_tSAM_Slot_PowerOn_Warm() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_tSAM_Check_SAM_In_Slot
Date&Time       :2018/1/11 上午 11:31
Describe        :
*/
int inNCCC_tSAM_Check_SAM_In_Slot(unsigned char uszSlot)
{
	if (inNCCC_tSAM_Status_IsActive(uszSlot) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inNCCC_tSAM_Check_SAM_In_Slot
Date&Time       :2018/1/11 上午 11:31
Describe        :
*/
int inNCCC_tSAM_Check_SAM_In_Slot_By_Global_Variable(unsigned char uszSlot)
{
	if (uszSlot == _SAM_SLOT_1_ && guszSlot1InBit == VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	else if (uszSlot == _SAM_SLOT_2_ && guszSlot2InBit == VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	else if (uszSlot == _SAM_SLOT_3_ && guszSlot3InBit == VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	else if (uszSlot == _SAM_SLOT_4_ && guszSlot4InBit == VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	else
	{
		if (inNCCC_tSAM_Check_SAM_In_Slot(uszSlot) == VS_SUCCESS)
		{
			if (uszSlot == _SAM_SLOT_1_)
			{
				guszSlot1InBit = VS_TRUE;
				return (VS_SUCCESS);
			}
			else if (uszSlot == _SAM_SLOT_2_)
			{
				guszSlot2InBit = VS_TRUE;
				return (VS_SUCCESS);
			}
			else if (uszSlot == _SAM_SLOT_3_)
			{
				guszSlot3InBit = VS_TRUE;
				return (VS_SUCCESS);
			}
			else if (uszSlot == _SAM_SLOT_4_)
			{
				guszSlot4InBit = VS_TRUE;
				return (VS_SUCCESS);
			}
			else
			{
				return (VS_ERROR);
			}
		}
		else
		{
			return (VS_ERROR);
		}
	}
}

/*
Function        :inNCCC_tSAM_Decide_tSAM_Slot
Date&Time       :2018/1/25 下午 2:27
Describe        :
*/
int inNCCC_tSAM_Decide_tSAM_Slot(unsigned char *uszSlot)
{
	int	inRetVal = VS_ERROR;
	char	szSAMSlot[2 + 1] = {0};
	
	memset(szSAMSlot, 0x00, sizeof(szSAMSlot));
	inGetHostSAMSlot(szSAMSlot);
	if (memcmp(szSAMSlot, "01", strlen("01")) == 0)
	{
		*uszSlot = _SAM_SLOT_1_;
		inRetVal = VS_SUCCESS;
	}
	else if (memcmp(szSAMSlot, "02", strlen("02")) == 0)
	{
		*uszSlot = _SAM_SLOT_2_;
		inRetVal = VS_SUCCESS;
	}
	else if (memcmp(szSAMSlot, "03", strlen("03")) == 0)
	{
		*uszSlot = _SAM_SLOT_3_;
		inRetVal = VS_SUCCESS;
	}
	else if (memcmp(szSAMSlot, "04", strlen("04")) == 0)
	{
		*uszSlot = _SAM_SLOT_4_;
		inRetVal = VS_SUCCESS;
	}
	/* 預設第一個SAM Slot，預防意外 */
	else
	{
		*uszSlot = _SAM_SLOT_1_;
		inRetVal = VS_ERROR;
                
                vdUtility_SYSFIN_LogMessage(AT,"GetHostSAMSlot Error (%s) set Default", szSAMSlot);
	}
	
	return (inRetVal);
}

/*
Function        :inNCCC_tSAM_PowerOn_Flow
Date&Time       :2018/1/25 下午 2:27
Describe        :根據現實狀況更改SamSlot位置
*/
int inNCCC_tSAM_PowerOn_Flow(unsigned char *uszSlot)
{
	int	inRetVal = VS_ERROR;
	
	/* 若有開key sound，Power On會爆音 先停一秒 */
	inDISP_Wait(1000);
	
	/* 如果是slot1就直接initial */
	if (*uszSlot == _SAM_SLOT_1_)
	{
		/* Power on the ICC and return the ATR which meets ISO-7816 specification.(Smart card) (Tsam Power On完就不Power Off) */
		inRetVal = inNCCC_tSAM_Slot_PowerOn(*uszSlot);
		if (inRetVal != VS_SUCCESS)
		{
			/* 不成功，切到slot1 */
			inSetHostSAMSlot("02");
			inSaveEDCRec(0);
			inRetVal = inNCCC_tSAM_Decide_tSAM_Slot(uszSlot);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			else
			{
				inRetVal = inNCCC_tSAM_Slot_PowerOn(*uszSlot);
				if (inRetVal != VS_SUCCESS)
				{
					/* 若再不成功，切回Slot2，並回傳錯誤 */
					inSetHostSAMSlot("01");
					inSaveEDCRec(0);
			
					return (inRetVal);
				}
			}
			
		}
		else
		{
			/* 成功，繼續做 */
		}
	}
	/* 如果是slot2，先試看看，若PowerOn失敗，切到Slot1 */
	else
	{
		inRetVal = inNCCC_tSAM_Slot_PowerOn(*uszSlot);
		if (inRetVal != VS_SUCCESS)
		{
			/* 不成功，切到slot1 */
			inSetHostSAMSlot("01");
			inSaveEDCRec(0);
			inRetVal = inNCCC_tSAM_Decide_tSAM_Slot(uszSlot);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			else
			{
				inRetVal = inNCCC_tSAM_Slot_PowerOn(*uszSlot);
				if (inRetVal != VS_SUCCESS)
				{
					/* 若再不成功，切回Slot2，並回傳錯誤 */
					inSetHostSAMSlot("02");
					inSaveEDCRec(0);
			
					return (inRetVal);
				}
			}
			
		}
		else
		{
			/* 成功，繼續做 */
		}
	}
	
	return (inRetVal);
}

int intSAMTest(void)
{
	
	char	szTemp[512];
	//Declare Local Variable //
	unsigned char	key;
	unsigned char	bStatus;
	unsigned char	baATR[128], bATRLen, CardType;
	unsigned char	uszSAPDU[128],uszRAPDU[128];
	USHORT	bSLen,usRLen;
	char	szResponce[12 + 1], szTemplate[42 + 1];

	// TODO: Add your program here //
	CTOS_BackLightSet(d_BKLIT_LCD,d_ON);
	CTOS_LCDTSelectFontSize(d_FONT_16x30);
	CTOS_LCDTClearDisplay ();
	CTOS_LCDTSetReverse(TRUE);
	CTOS_LCDTPrintXY(1,1,(unsigned char*)"Smart Card(unsigned char*)");
	CTOS_LCDTSetReverse(FALSE);
	CTOS_LCDTPrintXY(1,2,(unsigned char*)"Plz Ins Card");

	do
	{
		CTOS_KBDHit(&key);

		if (key == d_KBD_CANCEL)
			return(VS_ERROR);

		//Check the ICC status //
		CTOS_SCStatus(d_SC_SAM1,&bStatus);
	}while (!(bStatus & d_MK_SC_PRESENT)); //Break until the ICC Card is inserted //

	bATRLen = sizeof(baATR);
	CTOS_LCDTPrintXY(1,3,(unsigned char*)"Reset EMV..");

	//Power on the ICC and retrun the ATR contents metting the EMV2000 specification //
	if (CTOS_SCResetEMV(d_SC_SAM1, d_SC_5V, baATR, &bATRLen, &CardType) == d_OK)
		CTOS_LCDTPrintXY(13,3,(unsigned char*)"OK");
	else
		CTOS_LCDTPrintXY(13,3,(unsigned char*)"Fail");

	CTOS_Delay(1000);
	bATRLen = sizeof(baATR);
	CTOS_LCDTPrintXY(1,4,(unsigned char*)"Reset ISO..");

	//Power on the ICC and retrun the ATR content metting the ISO-7816 specification //
	if (CTOS_SCResetISO(d_SC_SAM1, d_SC_5V, baATR, &bATRLen, &CardType) == d_OK)
		CTOS_LCDTPrintXY(13,4,(unsigned char*)"OK");
	else
		CTOS_LCDTPrintXY(13,4,(unsigned char*)"Fail");

	CTOS_LCDTPrintXY(1,5,(unsigned char*)"Send APDU..");

	//APDU Data
//	uszSAPDU[0]=0x00; //CLA
//	uszSAPDU[1]=0xB2; //INS
//	uszSAPDU[2]=0x01; //P1
//	uszSAPDU[3]=0x0C; //P2
//	uszSAPDU[4]=0x00; //Le

//	uszSAPDU[0]=0x80; //CLA
//	uszSAPDU[1]=0xCA; //INS
//	uszSAPDU[2]=0x9F; //P1
//	uszSAPDU[3]=0x7F; //P2
//	uszSAPDU[4]=0x2D; //Le
//	bSLen = 5;

	uszSAPDU[0]=0x80; //CLA
	uszSAPDU[1]=0xEA; //INS
	uszSAPDU[2]=0x00; //P1
	uszSAPDU[3]=0x00; //P2
	uszSAPDU[4]=0x04; //Lc
	uszSAPDU[5]=0x13; //Lc
	uszSAPDU[6]=0x99; //Lc
	uszSAPDU[7]=0x91; //Lc
	uszSAPDU[8]=0x82; //Lc
	bSLen = 9;

	usRLen = sizeof(uszRAPDU);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "Receive Length： %d", (int)usRLen);
	inLogPrintf(AT, szTemplate);

	//Send out an APDU command and get the response from ICC //
	if (CTOS_SCSendAPDU(d_SC_SAM1, uszSAPDU, bSLen, uszRAPDU, &usRLen) == d_OK)
	{
		CTOS_LCDTPrintXY(13,5,(unsigned char*)"OK");
		memset(szTemp, 0x00, sizeof(szTemp));
		inFunc_BCD_to_ASCII(szTemp, uszRAPDU, (int)usRLen);
		inLogPrintf(AT, szTemp);
	}
	else
	{
		CTOS_LCDTPrintXY(13, 5, (unsigned char*)"Fail");
	}
	
	
	{
		/* 將APDU回的長度及資料塞回Rece Buffer */
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "Receive Length： %d", (int)usRLen);
		inLogPrintf(AT, szTemplate);

		/* 判斷回應碼是否為9000 */
		if (memcmp(&uszRAPDU[usRLen - 2], "\x90\x00", 2))
		{
			memset(szResponce, 0x00,  sizeof(szResponce));
			inFunc_BCD_to_ASCII(szResponce, &uszRAPDU[usRLen - 2], 2);
			inLogPrintf(AT, szResponce);
			
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "Receive Length： %d", (int)usRLen);
			inLogPrintf(AT, szTemplate);
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "APDU Rece Data:");
			}

			if (ginDebug == VS_TRUE)
			{
				memset(szTemp, 0x00, sizeof(szTemp));
				inFunc_BCD_to_ASCII(szTemp, uszRAPDU, usRLen);
				inLogPrintf(AT, szTemp);
			}

			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "-------------- END -----------------");
			}
			
			return(VS_ERROR);
		}
	}
		
		
	//Turn off the power of ICC //
	if (CTOS_SCPowerOff(d_SC_SAM1) == d_OK)
		CTOS_LCDTPrintXY(1,6,(unsigned char*)"Power off OK");
	else
		CTOS_LCDTPrintXY(1,6,(unsigned char*)"Power off Fail");

	CTOS_LCDTPrintXY(1,7,(unsigned char*)"Pls Take ICC Out");

	inDISP_Wait(3000);
	do
	{
		CTOS_SCStatus(d_SC_USER,&bStatus);
	}while (bStatus & d_MK_SC_PRESENT); //Break until the ICC Card does not exist //

//	CTOS_BackLightSet(d_BKLIT_LCD,d_OFF);
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_tSAM_SAM_Encrypt_Fail_Remedy
Date&Time       :2019/9/9 下午 2:14
Describe        :SAM卡加密失敗補救方法，只用在電票未開的情況下
*/
int inNCCC_tSAM_SAM_Encrypt_Fail_Remedy(unsigned char uszSlot)
{
	int	inRetVal = VS_SUCCESS;
	int	inRetVal2 = VS_ERROR;
	
	inUtility_StoreTraceLog_OneStep("inNCCC_tSAM_SAM_Encrypt_Fail_Remedy START");
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_tSAM_SAM_Encrypt_Fail_Remedy() START !");
	}
	
	do
	{
		/* 先檢查票證是否有開，有開則不適用此方法 */
		inRetVal2 = inMENU_Check_ETICKET_Enable(_TRANS_TYPE_NULL_);
		if (inRetVal2 == VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
			break;
		}
		
		/* 重新Power On*/
		inRetVal2 = inNCCC_tSAM_PowerOn_Flow(&uszSlot);
		if (inRetVal2 == VS_ERROR)
		{
			inRetVal = VS_ERROR;
			break;
		}
		
		/* 重新Select AID */
		inRetVal2 = inNCCC_tSAM_SelectAID_NCCC(uszSlot);
		if (inRetVal2 == VS_ERROR)
		{
			inRetVal = VS_ERROR;
			break;
		}
		
		break;
	}while(1);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_tSAM_SAM_Encrypt_Fail_Remedy() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	
	return (inRetVal);
}

/*
 * Function         : inNCCC_tSAM_PselectSamAidMulti_ECC
 * Create Date&Time : 2026/03/17 15:40
 * Describe         : ECC的AID現在有兩個，需先select 0501再select0801，這個函式一次做
 *[需求單-115011] 新規格MSAM悠遊卡載入AID調整專案
 *[20260317] 因要自動選二個AID，新增 inNCCC_tSAM_PselectSamAidMulti_ECC()
 */
int inNCCC_tSAM_PselectSamAidMulti_ECC(unsigned char uszSlot)
{
	int inRetVal = VS_SUCCESS;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "---------------------------------------");
		inLogPrintf(AT, "inNCCC_tSAM_PselectSamAidMulti_ECC() START!!");
	}
	
	/* 以下流程嘗試先使用 0x0501進行選擇，不成功後再以原本的 0x0081再試第二次 */
	inRetVal = inNCCC_tSAM_SelectAID_ECC(uszSlot,_AID_0501_);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Select ECC AID %s *Error* %d", _AID_0801_, inRetVal);
		}
		vdUtility_SYSFIN_LogMessage(AT, "Select ECC AID %s *Error* %d", _AID_0501_, inRetVal);

		inRetVal = inNCCC_tSAM_SelectAID_ECC(uszSlot,_AID_0801_);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Select ECC AID %s *Error* %d", _AID_0801_, inRetVal);
			}
			vdUtility_SYSFIN_LogMessage(AT, "Select ECC AID %s *Error* %d", _AID_0801_, inRetVal);
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_tSAM_PselectSamAidMulti_ECC() END!!!");
		inLogPrintf(AT, "---------------------------------------");
	}
	
	return (inRetVal);
}

