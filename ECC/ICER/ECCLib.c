#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sqlite3.h>
#include "../../SOURCE/INCLUDES/Transaction.h"
#include "../../SOURCE/INCLUDES/Define_1.h"
#include "../../SOURCE/INCLUDES/TransType.h"
#include "../../SOURCE/INCLUDES/AllStruct.h"
#include "../../SOURCE/DISPLAY/Display.h"
#include "../../SOURCE/FUNCTION/Sqlite.h"
#include "../../SOURCE/FUNCTION/Function.h"
#include "../../SOURCE/FUNCTION/CFGT.h"
#include "../../SOURCE/FUNCTION/HDT.h"
#include "../../SOURCE/FUNCTION/File.h"
#include "../../NCCC/NCCCTicketSrc.h"
#include "../../NCCC/NCCCtSAM.h"
#include "../../SOURCE/FUNCTION/KMS.h"
#include "ECCLib.h"

extern	int		ginMacError;
extern	int		ginDebug;
extern	unsigned char	gusztSAMKeyIndex;
extern	unsigned char	gusztSAMCheckSum_56[4 + 1];

short ECC_NCCC_Encryption_TSAM_Lib(BYTE *bData, unsigned int *dataLen, BYTE fieldNo, BYTE *keyField37, BYTE *checksumField57, BYTE *keyIndex)
{
	int			inRetVal = 0;
	int			inUIDLen = 0;
	int			inEncryptLen = 0;
	int			inRetVal_SelectAID = VS_SUCCESS;
	int			inPacketCnt = 0;
	int			i = 0;
	char			szDebugMsg[100 + 1] = {0};
	char			szUID[50 + 1] = {0};
	char			szTemplate[50 + 1] = {0};
	char			szField37[8 + 1] = {0}, szField41[4 + 1] = {0};
	char			szAscii[100 + 1] = {0};
	char			szPacket[5000 + 1] = {0};
	char			szPath[200 + 1] = {0};
	unsigned char		uszSlot = 0;
	unsigned char		uszLRC[1 + 1] = {0};
	unsigned long		ulHandle = 0;
	TRANSACTION_OBJECT	pobTran = {0};
	
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
	
	
	if (fieldNo == 35)
	{
		memset(szUID, 0x00, sizeof(szUID));
		memcpy(szUID, (char*)bData, *dataLen);
		inUIDLen = *dataLen;
		
		/* 取加密長度，
		* 1. F_35 Data Length= 01~ 15 Bytes ，取前 8 Bytes 加密。
		* 2. F_35 Data Length= 16~ 23 Bytes ，取前 16 Bytes 加密。
		* 3. F_35 Data Length= 24~ 31 Bytes ，取前 24 Bytes 加密。
		* 4. F_35 Data Length= 32~ 39 Bytes ，取前 32 Bytes 加密。
		* 5. F_35 Data Length= 40~ 57 Bytes ，取前 40 Bytes 加密。
		*  */
	       if (inUIDLen >= 1 && inUIDLen <= 15)
	       {
		       inEncryptLen = 8;
	       }
	       else if (inUIDLen >= 16 && inUIDLen <= 23)
	       {
		       inEncryptLen = 16;
	       }
	       else if (inUIDLen >= 24 && inUIDLen <= 31)
	       {
		       inEncryptLen = 24;
	       }
	       else if (inUIDLen >= 32 && inUIDLen <= 39)
	       {
		       inEncryptLen = 32;
	       }
	       else if (inUIDLen >= 40 && inUIDLen <= 57)
	       {
		       inEncryptLen = 40;
	       }
		
		/* 組 Field_37 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, keyField37, 11);
		inFunc_PAD_ASCII(szTemplate, szTemplate, 'F', 16, _PADDING_RIGHT_); /* 補【F】 */
		memset(szField37, 0x00, sizeof(szField37));
		inFunc_ASCII_to_BCD((unsigned char*)szField37, szTemplate, 8);
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "F_37:\"%s\"", szTemplate);
		}
		/* 組 Field_41 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTerminalID(szTemplate);
		memset(szField41, 0x00, sizeof(szField41));
		inFunc_ASCII_to_BCD((unsigned char*)szField41, szTemplate, 4);

		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "F_41:\"%s\"", szTemplate);
			inLogPrintf(AT, "Before Encrypt(%d):\"%s\"", inEncryptLen, szUID);
		}
		
		/* 切回NCCC */
		inNCCC_tSAM_SelectAID_NCCC(uszSlot);
		
		/* 加密 */
		inRetVal = inNCCC_tSAM_Encrypt_ESVC(uszSlot,
				 inEncryptLen,
				 szUID,
				 (unsigned char*)&szField37,
				 (unsigned char*)&szField41,
				 keyIndex,
				 checksumField57);
		/* 切回票證AID */
		/* [20251215_BUG_MDF][TSAM] 悠遊卡tSAM使用問題 
		 * 把原本呼叫的 inNCCC_tSAM_SelectAID_ECC 修改成依照設定重選AID的 inNCCC_tSAM_SelectAID_ECC_Flow() */
		inRetVal_SelectAID = inNCCC_tSAM_SelectAID_ECC_Flow();
		
		if (inRetVal_SelectAID != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		
		if (inRetVal == VS_ERROR)
		{
			return (VS_ERROR);
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}
		
		memcpy(bData, szUID, inUIDLen);
		*dataLen = inUIDLen;
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, (unsigned char*)szUID, inUIDLen);
			sprintf(szDebugMsg, "F_35 [%d %s]", inUIDLen, szAscii);
			inLogPrintf(AT, szDebugMsg);
		}

	}
	else if (fieldNo == 56)
	{
		memset(&pobTran, 0x00, sizeof(pobTran));
		memcpy(pobTran.srTRec.szRefNo, (char*)keyField37, 11);
		pobTran.srTRec.inTicketType = _TICKET_TYPE_ECC_;
		memset(szPacket, 0x00, sizeof(szPacket));
		memcpy(szPacket, (char*)bData, *dataLen);
		inPacketCnt = *dataLen;
		
		/* 先Gzip */
		inRetVal = inNCCC_Ticket_Gen_F_56_File(szPacket, inPacketCnt);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inNCCC_Ticket_Gen_F_56_File Fail");
			}
		}

		inRetVal = inFunc_Data_GZip("", _ESVC_FILE_F_56_, _FS_DATA_PATH_);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inFunc_Data_GZip Fail");
			}
		}
		/* 加密第17~32 byte */
		/* 置換 */
		gusztSAMKeyIndex = *keyIndex;
		inRetVal = inNCCC_Ticket_Data_Compress_Encryption(&pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inNCCC_Ticket_Data_Compress_Encryption Fail");
			}
		}
		else
		{
			memcpy((char*)checksumField57, (char*)gusztSAMCheckSum_56, 4);
		}

		/* 讀出來，貼上 */
		memset(szPacket, 0x00, sizeof(szPacket));
		inPacketCnt = 0;
		memset(szPath, 0x00, sizeof(szPath));
		sprintf(szPath, "%s%s", _FS_DATA_PATH_, _ESVC_FILE_F_56_GZ_ENCRYPTED_);
		inFile_Linux_Get_FileSize_By_Stat(szPath, &inPacketCnt);
		inRetVal = inFILE_OpenReadOnly(&ulHandle, (unsigned char*)_ESVC_FILE_F_56_GZ_ENCRYPTED_);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inFILE_OpenReadOnly Fail");
			}
		}
		else
		{
			inFILE_Read(&ulHandle, (unsigned char*)szPacket, (unsigned long)inPacketCnt);

			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inPacketCnt : %d", inPacketCnt);
			}

			inFILE_Close(&ulHandle);

			/* 補上檢查碼LRC */
			memset(uszLRC, 0x00, sizeof(uszLRC));
			uszLRC[0] = 0x00;
			for (i = 0; i < inPacketCnt; i++)
			{
				uszLRC[0] = uszLRC[0] ^ szPacket[i];
			}
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "LRC : %X", uszLRC[0]);
			}

			memcpy(&szPacket[inPacketCnt], uszLRC, 1);
			inPacketCnt++;
			
			memcpy(bData, szPacket, inPacketCnt);
			*dataLen = inPacketCnt;
		}
	}
	else
	{
		inRetVal = -1;
		return (inRetVal);
	}


	return (inRetVal);
}

short ECC_NCCC_MAC_Lib(unsigned int msgType, BYTE *bData, unsigned int idxField3, unsigned idxField4, unsigned int idxField11, unsigned int idxField35, unsigned int idxField59, unsigned int idxField63, BYTE *MACData)
{
	int			inRetVal = 0;
	char			szTerminalID[8 + 1];
	char			szMACEnable[1 + 1] = {0};
	TICKET_NCCC_MAC_STRUCT	srMAC;
	TRANSACTION_OBJECT	pobTran = {0};
	
	/* F_64 */
	/* 預設除0400、0800外全部要送MAC，但若安全認證沒通過，信用卡交易不送F_64 */
	/* 這邊採用跟Verifone Code一樣的邏輯，安全認證時先砍Working Key，若找不到Key代表安全認證失敗 */
	/* CFGT 的 MacEnable沒On 或是 沒有Mac Key 或是 0220的交易 不送Mac */
	/* MAC換key後，回3次A0或A1 */
	memset(szMACEnable, 0x00, sizeof(szMACEnable));
	inGetMACEnable(szMACEnable);
	if ((memcmp(szMACEnable, "Y", 1) != 0)						||	/* CFGT沒開 */
	    (inKMS_CheckKey(_TWK_KEYSET_NCCC_, _TWK_KEYINDEX_NCCC_MAC_) != VS_SUCCESS)	||
	     ginMacError >= 3)
	{
		/* 沒有MAC */
		inRetVal = 0;
	}
	else
	{
		/* 判斷MTI是否要加MAC */
		if ((msgType == 0x0800)	||
		    (msgType == 0x0400))
		{
			/* 不用送 */
			inRetVal = 0;
		}
		else
		{
			memset(&pobTran, 0x00, sizeof(pobTran));
			
			if (msgType == 0x0500)
			{
				pobTran.inISOTxnCode = _SETTLE_;
			}
			
			/* 信用卡 */
			memset(szTerminalID, 0x00, sizeof(szTerminalID));
			inGetTerminalID(szTerminalID);

			memset(&srMAC, 0x00, sizeof(TICKET_NCCC_MAC_STRUCT));
			/* 電票要P Code中間兩碼，所以要再加一 */
			inFunc_BCD_to_ASCII(srMAC.szF_03, bData + idxField3 + 1, 1);
			inFunc_BCD_to_ASCII(srMAC.szF_04, bData + idxField4, 6);
			inFunc_BCD_to_ASCII(srMAC.szF_11, bData + idxField11 + 2, 1);
			memcpy(srMAC.szF_35, (char*)bData + idxField35, 22);
			memcpy(srMAC.szF_41, szTerminalID, 8);
			/* 若該筆交易電文沒有 Table ID “ 則此欄位補滿 0 */
			if (idxField59 > 0)
			{
				memcpy(srMAC.szF_59, bData + idxField59, 18);
			}
			else
			{
				strcpy(srMAC.szF_59, "000000000000000000");
			}
			/* 63要跳著撈 */
			memcpy(&srMAC.szF_63[0], (char*)bData + idxField63 + 2, 15);
			memcpy(&srMAC.szF_63[15], (char*)bData + idxField63 + 2 + (105 * 1), 15);
			memcpy(&srMAC.szF_63[30], (char*)bData + idxField63 + 2 + (105 * 2), 15);

			/* MIT = 〈0800〉 〈0400〉不用送(在Bit Map處理) */
			inNCCC_Ticket_GenMAC(&pobTran, &srMAC);

			memcpy(MACData, &pobTran.szMAC_HEX[0], 8);

			/* 有MAC */
			/* 回傳值不為0就行 */
			inRetVal = 1;
		}
	}
	
	return (inRetVal);
}