#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <ctos_qrcode.h>
#include <libxml/xpath.h>
#include <fcntl.h>		//2019.02.15 added by Hachi
#include <math.h>		//2019/2/26 下午 3:38
#include <unistd.h> 		//2019/3/4 下午 6:23
#include <sqlite3.h>
#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/AllStruct.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/PRINT/PrtMsg.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/File.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/Batch.h"
#include "../SOURCE/FUNCTION/Card.h"
#include "../SOURCE/FUNCTION/CDT.h"
#include "../SOURCE/FUNCTION/CDTX.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/Signpad.h"
#include "../SOURCE/FUNCTION/IPASSDT.h"
#include "../SOURCE/FUNCTION/ECCDT.h"
#include "../SOURCE/FUNCTION/ICASHDT.h"
#include "../SOURCE/FUNCTION/TDT.h"
#include "../SOURCE/FUNCTION/Utility.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../HG/HGsrc.h"
#include "../HG/HGiso.h"
#include "../NCCC/NCCCTicketSrc.h"
#include "../FISC/NCCCfisc.h"
#include "../NCCC/NCCCesc.h"
#include "../NCCC/NCCCdcc.h"
#include "../NCCC/NCCCloyalty.h"
#include "../NCCC/NCCCtms.h"
#include "../NCCC/NCCCsrc.h"
#include "../NCCC/NCCCTrust.h"
#include "../CTLS/CTLS.h"
#include "../ECC/ICER/stdAfx.h"
#include "../ECC/ECC.h"
#include "CreditprtByBuffer.h"

#define max(x, y) (((x) > (y)) ? (x) : (y))

extern  int		ginDebug;			/* Debug使用 extern */
extern	int		ginMachineType;
extern	char		gszTermVersionID[16 + 1];
extern	char		gszTermVersionDate[16 + 1];
extern	BMPHeight	gsrBMPHeight;			/* 圖片高度 */
extern	unsigned char	guszEnormousNoNeedResetBit;	/* 資料庫是否須從頭找 */
extern	int		inPrinttype_ByBuffer;        /* 0 = 橫式，1 = 直式 */
extern	char		gszReprintDBPath[100 + 1];
extern	TOTAL_REPORT_TABLE_BYBUFFER srTotalReport_ByBuffer[];
extern	TOTAL_REPORT_TABLE_BYBUFFER_ESVC srTotalReport_ByBuffer_ESVC[];
extern	DETAIL_REPORT_TABLE_BYBUFFER_ESVC srDetailReport_ByBuffer_ESVC[];
extern	DETAIL_REPORT_TABLE_BYBUFFER srDetailReport_ByBuffer[];
extern	unsigned short	gusPrintFontStyleRegular;
extern	unsigned short	gusBAULFontStyleRegular;
extern	TOTAL_REPORT_TABLE_BYBUFFER_TRUST srTotalReport_ByBuffer_TRUST[];
extern  char		gszTranDBPath[100 + 1];

/*
Function        :inCREDIT_PRINT_Data_ByBuffer_Small_S
Date&Time       :2023/3/21 上午 10:49
Describe        :列印DATA
*/
int inCREDIT_PRINT_Data_ByBuffer_Small_S(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
        int     inRetVal;
        char 	szPrintBuf[84 + 1], szPrintBuf1[84 + 1], szPrintBuf2[84 + 1], szTemplate1[42 + 1], szTemplate2[84 + 1];
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
	char	szExamBit[10 + 1] = {0};
	char	szShort_Receipt_Mode[1 + 1] = {0};
	unsigned char	uszChangeLineBit = VS_FALSE;

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
        memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	memset(szShort_Receipt_Mode, 0x00, sizeof(szShort_Receipt_Mode));
	
	inGetShort_Receipt_Mode(szShort_Receipt_Mode);

        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                /*卡別、卡號*/
		/* 【需求單 - 106349】自有品牌判斷需求 */
		/* unpack電文就存存CardLabel modify by LingHsiung 2020/2/14 上午 11:07 */
		sprintf(szPrintBuf, "卡別　　：%s", pobTran->srBRec.szCardLabel);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡號　　：%s", pobTran->srBRec.szPAN);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*日期、時間*/
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "日期　　：%s",pobTran->srBRec.szDate);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "時間　　：%s",pobTran->srBRec.szTime);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*調閱編號、批次號碼 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "調閱編號：%06ld",pobTran->srBRec.lnOrgInvNum);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "批次號碼：%06ld",pobTran->srBRec.lnBatchNum);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*交易類別*/
                inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "交易類別：%s",szTemplate1);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*授權碼、序號*/
                sprintf(szPrintBuf, "授權碼　：%s",pobTran->srBRec.szAuthCode);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "序號　　：%s",pobTran->srBRec.szRefNo);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        }
        else
        {
                /* 橫式 */
		/* "卡號 卡別" */
		/* "卡號" */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		inFunc_PAD_ASCII(szPrintBuf, "卡號", ' ', 31, _PADDING_RIGHT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 卡別值 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		/* 【需求單 - 106349】自有品牌判斷需求 */
		/* unpack電文就存存CardLabel modify by LingHsiung 2020/2/14 上午 11:07 */
		inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 20, _PADDING_LEFT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 卡別 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", "卡別");
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 卡號值 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		
		strcpy(szPrintBuf1, pobTran->srBRec.szPAN);
		
		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		/* 卡號遮掩(一般卡號前6後4，U Card前3後5) */
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
		if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			/* Uny交易不受TMS開關限制，且獲取卡號時已遮掩 */
		}
		else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
		{
			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			for (i = 6; i < (strlen(szPrintBuf1) - 4); i ++)
			{
				szPrintBuf1[i] = 0x2A;
			}
			
		}
		else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
			 pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
		{
			/* 商店聯卡號遮掩 */
			memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
			inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0 && pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				for (i = 6; i < (strlen(szPrintBuf1) - 4); i ++)
					szPrintBuf1[i] = 0x2A;
			}
		}
                
		/* 過卡方式 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf1, "(W)");
			else
				strcat(szPrintBuf1, "(C)");
		}
		else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			strcat(szPrintBuf1, "(Q)");
		}
		else if (pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
		{
			strcat(szPrintBuf1, "(Q)");
		}
		else
		{
			if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
				strcat(szPrintBuf1,"(C)");
			else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
				strcat(szPrintBuf1, "(T)");
			else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf1, "(W)");
			else
			{
				if (pobTran->srBRec.uszManualBit == VS_TRUE)
				{
					/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
					/* 電文轉Manual Keyin但是簽單要印感應的W */
					if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
						strcat(szPrintBuf1, "(W)");
					else
						strcat(szPrintBuf1,"(M)");
				}
				else
					strcat(szPrintBuf1,"(S)");
			}
			
		}
		
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_WIDTH_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 交易別 */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s", "交易");
		
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
		
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", szTemplate1);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "%s %s", szPrintBuf1, szPrintBuf2);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		if (strlen(szTemplate2) > 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate2);

			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
                /* 城市 主機 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		
		/* 城市 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetCityName(szTemplate1);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s %s", "城市", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		/* 主機 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetHostLabel(szTemplate1);
		inFunc_DiscardSpace(szTemplate1);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PADDING_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", "主機");
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 日期時間 批號 */
		/* 日期時間 */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szTemplate1, "%.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
		sprintf(szPrintBuf1, "%s %s", "日期/時間", szTemplate1);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		
		if (!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))
		{
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		}
		else
		{
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		}
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 批號值 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%03ld", pobTran->srBRec.lnBatchNum);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PADDING_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 批號 */
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", "批號");
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);
		
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
                /* 授權碼 檢查碼 */
		/* 前半段 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szAuthCode[0], 12);
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s %s", "授權碼", szTemplate1);
		inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 32, _PADDING_RIGHT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		
		if (!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))
		{
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		}
		else
		{
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		}
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 後半段 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		if (pobTran->srBRec.uszTakaTransBit == VS_TRUE)
		{
			sprintf(szTemplate1, "%.2s/'%.2s", pobTran->srBRec.szExpDate + 2, pobTran->srBRec.szExpDate);
		}
		else
		{
			/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
			if (strlen(pobTran->srBRec.szCheckNO) > 0)
			{
				strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
			}
			else
			{
				inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
			}
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 9, _PADDING_LEFT_);
		}
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		if (pobTran->srBRec.uszTakaTransBit == VS_TRUE)
		{
			memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
			sprintf(szPrintBuf2, "%s", "有效期");
		}
		else
		{
			memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
			sprintf(szPrintBuf2, "%s", "檢查碼");
		}
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 序號 調閱號 */
		/* 序號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szRefNo[0], 12);
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s %s", "序號", szTemplate1);
		inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 29, _PADDING_RIGHT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		
		/* 調閱號值 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%06ld", pobTran->srBRec.lnOrgInvNum);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PADDING_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* "調閱號" */
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", "調閱號");
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 櫃號 回覆碼 */
		/* 櫃號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetStoreIDEnable(szTemplate1);
		if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
		{
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memcpy(szTemplate1, &pobTran->srBRec.szStoreID[0], 23);
			memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
			sprintf(szPrintBuf1, "%s %s", "櫃號", szTemplate1);
			inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 33, _PADDING_RIGHT_);
			
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szPrintBuf1);

			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* 後半段(銀聯交易才印回覆碼) */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%s", pobTran->srBRec.szRespCode);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 9, _PADDING_LEFT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate1);

				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
				sprintf(szPrintBuf2, "%s", "回覆碼");
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szPrintBuf2);

				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			else
			{
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
		}
		else
		{
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				/* 後半段(銀聯交易才印回覆碼) */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%s", pobTran->srBRec.szRespCode);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 9, _PADDING_LEFT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate1);

				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
				sprintf(szPrintBuf2, "%s", "回覆碼");
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szPrintBuf2);

				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
		}
		/* 產品代碼 */
		inGetProductCodeEnable(szProductCodeEnable);
		if (memcmp(szProductCodeEnable, "Y", 1) == 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s %s", "產品代碼", pobTran->srBRec.szProductCode);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
                /* 【需求單-113260】MasterCard機票交易需求 by Russell 2025/3/3 上午 11:55 */
                if (inNCCC_Func_Flight_Ticket_Allow_Print(pobTran) == VS_TRUE)
                {
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "%s", "出發地機場");
                        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);

                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "%s", pobTran->srBRec.szFlightTicketPDS0523);
                        inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_08_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);

                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "%s", "目的地機場");
                        inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_02_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);

                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "%s", pobTran->srBRec.szFlightTicketPDS0524);
                        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);

                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "%s", "航班號碼");
                        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);
			
                        
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        if (strlen(pobTran->srBRec.szFlightTicketPDS0530) > 0)
                        {
                                sprintf(szTemplate2, "%d", atoi(pobTran->srBRec.szFlightTicketPDS0530));
                        }
                        else
                        {
                                sprintf(szTemplate2, "%s", " ");
                        }
                        sprintf(szPrintBuf, "%s", szTemplate2);
                        
                        inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_08_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);

			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			inNCCC_Func_Get_Flight_Ticket_Txn_Category_PrintName(pobTran, szTemplate1);
			
			if (!memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_BAGGAGE_CHARGE_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_BAGGAGE_CHARGE_)))		||
			    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_PREPAID_TICKET_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_PREPAID_TICKET_)))		||
			    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_MISCELLANEOUS_CHARGE_ , max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_MISCELLANEOUS_CHARGE_)))	||
			    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_REISSUE_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_REISSUE_))))
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf)); 
				sprintf(szPrintBuf, "%s", "交易類型");
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));


				sprintf(szPrintBuf, "%s", szTemplate1);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			else
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf)); 
				sprintf(szPrintBuf, "%s", "交易類型");
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_02_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));


				sprintf(szPrintBuf, "%s", szTemplate1);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
                }
                                
		/* TC */
		memset(szExamBit, 0x00, sizeof(szExamBit));
		inGetExamBit(szExamBit);
		/* Uny交易 */
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
		if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			/* 取消或退貨，只印數字交易碼 */
			/* 最小字體 */
			if (pobTran->srBRec.uszVOIDBit == VS_TRUE ||
			    pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "交易碼 %s", pobTran->srBRec.szUnyTransCode);
				
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			/* 正向交易，要印條碼和數字 */
			else
			{
				
				if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
				{
					/* 一維條碼 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					strcpy(szPrintBuf, pobTran->srBRec.szUnyTransCode);
					inPRINT_Buffer_Barcode(szPrintBuf, uszBuffer, srBhandle, 0, VS_FALSE, _PRINT_BARCODE_TYPE_DEFAULT_, _PRINT_BARCODE_X_EXTEND_DEFAULT_, 2);
				
					/* 數字 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "交易碼(退貨請掃描) %s", pobTran->srBRec.szUnyTransCode);

					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					/* 一維條碼 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					strcpy(szPrintBuf, pobTran->srBRec.szUnyTransCode);
					inPRINT_Buffer_Barcode(szPrintBuf, uszBuffer, srBhandle, 0, VS_FALSE, _PRINT_BARCODE_TYPE_DEFAULT_, _PRINT_BARCODE_X_EXTEND_DEFAULT_, 2);
				
					/* 數字 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "交易碼 %s", pobTran->srBRec.szUnyTransCode);

					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
			}
		}
		/* 實體卡交易和掃碼交易互斥 */
		else
		{
			if (pobTran->srBRec.inChipStatus == _EMV_CARD_ || pobTran->srBRec.uszContactlessBit == VS_TRUE)
			{
				/* 感應磁條 */
				if (!memcmp(pobTran->srBRec.szAuthCode, "VLP", 3)				|| 
				    !memcmp(pobTran->srBRec.szAuthCode, "JCB", 3)				||
				     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE	|| 
				     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD		||
				     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
				{
					/* 商店聯卡號遮掩 */
					memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
					inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
					if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
					    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
					    strlen(pobTran->srBRec.szTxnNo) > 0)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
                                                if (!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))
                                                {
                                                        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _FONT_SIZE_SHORT_RECEIPT_U_TXNO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                                }
                                                else
                                                {
                                                        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                                }
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}
				}
				else 
				{
					if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
					    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
					{
						if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "TC:%02X%02X%02X%02X%02X%02X%02X%02X",
											pobTran->srEMVRec.usz9F26_ApplCryptogram[0],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[1],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[2],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[3],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[4],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[5],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[6],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[7]);
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}

						/* 商店聯卡號遮掩 */
						memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
						inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
						if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
						    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
						    strlen(pobTran->srBRec.szTxnNo) > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
                                                        if (!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))
                                                        {
                                                                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _FONT_SIZE_SHORT_RECEIPT_U_TXNO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                                        }
                                                        else
                                                        {
                                                                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                                        }
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}

						uszChangeLineBit = VS_FALSE;
						/* AID */
						if (strlen(pobTran->srBRec.szCUP_EMVAID) > 0)
						{
							if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
							{
								/* CUP晶片要印 */
								if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								/* 【需求單-111290】VISA置換Logo、DISCOVER簽單新增AID欄位、AE簽單新增AID和AP Label欄位需求 by Russell 2022/12/29 下午 5:57 */
								else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
									pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								/* 驗測要印AP Label 和 AID */
								else if (szExamBit[0] == '1')
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
							}
							else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
							{
								/* 銀聯閃付 */
								if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								/* AE晶片要印出AID */
								else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY || 
									 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE || 
									 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								else if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS	|| pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE) &&
									pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								/* 驗測要印AP Label 和 AID */
								else if (szExamBit[0] == '1')
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
							}
						}

						/* M/C交易列印AP Lable (START) */
						if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
						{
							if (pobTran->srEMVRec.in50_APLabelLen > 0)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
								inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
								if (inRetVal != VS_SUCCESS)
									return (VS_ERROR);
							}
							else
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, "AP Label:");
								inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
								if (inRetVal != VS_SUCCESS)
									return (VS_ERROR);
							}
						}
						/* M/C交易列印AP Lable (END) */
						else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
						{
							if (pobTran->srEMVRec.in50_APLabelLen > 0)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
								inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
								if (inRetVal != VS_SUCCESS)
									return (VS_ERROR);
							}
							else
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, "AP Label:");
								inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
								if (inRetVal != VS_SUCCESS)
									return (VS_ERROR);
							}
						}
						/* 驗測要印AP Label 和 AID */
						else if (szExamBit[0] == '1')
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
							inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
						/* 一定要換行 */
						else
						{
							if (uszChangeLineBit == VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, " ");
								inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
								if (inRetVal != VS_SUCCESS)
									return (VS_ERROR);
							}
						}

					}
					else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
					{
						/* 商店聯卡號遮掩 */
						/* 持卡人存根也要印 */
						memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
						inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
						if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
						    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
						    strlen(pobTran->srBRec.szTxnNo) > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
                                                        if (!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))
                                                        {
                                                                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _FONT_SIZE_SHORT_RECEIPT_U_TXNO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                                        }
                                                        else
                                                        {
                                                                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                                        }
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
					}
				}

			}
			else
			{
				/* 商店聯卡號遮掩 */
				/* 磁條卡列印交易編號 */
				memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
				inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
				if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
				    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
				    strlen(pobTran->srBRec.szTxnNo) > 0)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
                                        if (!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))
                                        {
                                                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _FONT_SIZE_SHORT_RECEIPT_U_TXNO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                        }
                                        else
                                        {
                                                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                        }
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
			}
		}
        }

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_Amount_ByBuffer_Small_S
Date&Time       :2023/3/21 下午 12:04
Describe        :列印AMOUNT
*/
int inCREDIT_PRINT_Amount_ByBuffer_Small_S(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                /* 金額 */
                if(pobTran->srBRec.inCode == _TIP_)
                {
                        /* 金額 */
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTxnAmount);
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


                        /* 小費 */
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTipTxnAmount);
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PADDING_LEFT_);

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "小費(Tips)  :%s", szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


                        /* 總計 */
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ' , 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf,"總計(Total) :%s", szTemplate);
                }
                else if ((pobTran->srBRec.uszVOIDBit == VS_TRUE	&& 
			 (pobTran->srBRec.inOrgCode != _REFUND_ && pobTran->srBRec.inOrgCode != _INST_REFUND_ && pobTran->srBRec.inOrgCode != _REDEEM_REFUND_)) ||
			  pobTran->srBRec.inCode == _REFUND_	 || 
			  pobTran->srBRec.inCode == _INST_REFUND_|| 
			  pobTran->srBRec.inCode == _REDEEM_REFUND_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", 0 - pobTran->srBRec.lnTxnAmount);
                        inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                }
                else if (pobTran->srBRec.inCode == _ADJUST_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnAdjustTxnAmount);
                        inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                }
                else
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTxnAmount);
                        inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                }
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                
		for (i = 0; i < 2; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
        }
        /* 橫式 */
        /* 負向交易 */
	else if(pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		/* 橫式 */
                /* 金額 */
                /* 取消退貨是正數 */
                if (pobTran->srBRec.inOrgCode == _REFUND_ || pobTran->srBRec.inOrgCode == _INST_REFUND_ || pobTran->srBRec.inOrgCode == _REDEEM_REFUND_)
                {
                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
			
                        /* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Total) :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
		else
		{
                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

                        /* 把前面的字串和數字結合起來 */
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Total) :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

	}
	/* 正向交易 */
	else
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTransFunc(szTemplate);
		if (szTemplate[6] == 'Y')	/* 檢查是否有開小費 */
		{
			/* 退貨金額為負數 */
			if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
			{
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
				inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", "總計(Total) :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			/* 預授不會有小費，所以拉出來 */
			else if (pobTran->srBRec.inCode == _PRE_AUTH_ || pobTran->srBRec.inCode == _PRE_COMP_)
			{
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", "總計(Total) :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				/* 金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", "金額(Amount):");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				
				/* lnTipAmount為0表示非小費 */
				if (pobTran->srBRec.lnTipTxnAmount == 0L)
				{
					/* 小費 */
					inPRINT_Buffer_PutIn("小費(Tips)  :______________________________________", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					/* 總計 */
					inPRINT_Buffer_PutIn("總計(Total) :______________________________________", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				else
				{
					/* 小費 */
					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTipTxnAmount);
					inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", "小費(Tips)  :");
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


					/* 總計 */
					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%ld",  (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
					inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", "總計(Total) :");
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				
			}
			
		}
		/* 小費沒開時 */
		else
		{
			/* 總計 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
			{
				sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			}
			else
			{
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			}
			inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
			

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Total) :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
			
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_ReceiptEND_ByBuffer_Small_S
Date&Time       :2023/3/21 上午 10:42
Describe        :列印結尾
*/
int inCREDIT_PRINT_ReceiptEND_ByBuffer_Small_S(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
	char	szSignature[30 + 1] = {0};
	char	szSignaturePath[80 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	
        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                inPRINT_Buffer_PutIn("簽名欄:_____________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
		    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
                {
                        inPRINT_Buffer_PutIn("*** 商店收據 Merchant Copy ***", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_CUST_;

                }
                else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
                {
                        inPRINT_Buffer_PutIn("*** 持卡人收據 Customer Copy ***", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_MERCH_;
                }

                inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
        }
        else
        {
                /* 橫式 */
                if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
                {
			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}
				}
			}
			
                        /* 簽名欄 */
			/* 免簽名 */
			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
			{
				
			}
			/* 要簽名 */
			else
			{
				/* 藉由TRT_FileName比對來組出bmp的檔名 */
				inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
				
				memset(szSignature, 0x00, sizeof(szSignature));
				/* 因為用invoice所以不用inFunc_ComposeFileName */
				inFunc_ComposeFileName_InvoiceNumber(pobTran, szSignature, _PICTURE_FILE_EXTENSION_, 6);
				memset(szSignaturePath, 0x00, sizeof(szSignaturePath));
				sprintf(szSignaturePath, "./fs_data/%s", szSignature);
				/* 有在signpad簽名*/
				if (inFILE_Check_Exist((unsigned char *)szSignature) == VS_SUCCESS)
				{
					/* 電子簽名 */
					inPRINT_Buffer_PutGraphic((unsigned char *)szSignaturePath, uszBuffer, srBhandle, _SIGNEDPAD_WIDTH_, _APPEND_);
				}
				/* 手簽 */
				else
				{
					inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				
				inPRINT_Buffer_PutIn("X:", _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("------------------------------------------------------", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
                        
			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
			{
				inPRINT_Buffer_PutIn("持卡人簽名(特店存根聯)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("免簽名", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			}
			else
			{
				inPRINT_Buffer_PutIn("持卡人簽名(特店存根聯)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			}
			
			if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
			    pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
			{
				/* 持卡人姓名 */
				inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
                }
		else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
                {
			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}

				}
			}
			
			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
			{
				inPRINT_Buffer_PutIn("商店存根", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("免簽名", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			}
			else
			{
				inPRINT_Buffer_PutIn("商店存根", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			}
			
			if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
			    pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
			{
				/* 持卡人姓名 */
				inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}
                else
                {
			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}

				}
			}
			
			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
			{
				inPRINT_Buffer_PutIn("持卡人存根 Card holder stub", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("免簽名", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			}
			else
			{
				inPRINT_Buffer_PutIn("持卡人存根 Card holder stub", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			}
			
			if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
			    pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
			{
				/* 持卡人姓名 */
				inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
                }
		
		if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
		{
			inPRINT_Buffer_PutIn("重印 REPRINT", _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		}
		
                /* 列印警示語 */
                inPRINT_Buffer_PutIn("            I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("        ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		/* Print Notice */
		if (inCREDIT_PRINT_Notice(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
			return (VS_ERROR);
		
		/* Print Slogan */
		if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
		{
			if (inCREDIT_PRINT_MarchantSlogan(pobTran, _NCCC_SLOGAN_PRINT_DOWN_, uszBuffer, srBhandle) != VS_SUCCESS)
				return (VS_ERROR);
			
			if (pobTran->srBRec.uszRewardL1Bit == VS_TRUE	|| 
			    pobTran->srBRec.uszRewardL2Bit == VS_TRUE	||
			    pobTran->srBRec.uszRewardL5Bit == VS_TRUE)
			{
				if (inCREDIT_PRINT_RewardAdvertisement(pobTran, uszBuffer, srFont_Attrib, srBhandle) != VS_SUCCESS)
					return (VS_ERROR);
			}
		}
		
		for (i = 0; i < 6; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
        }

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Data_ByBuffer_Small_DCC_S
Date&Time       :2023/3/21 下午 5:18
Describe        :列印DATA
*/
int inCREDIT_PRINT_DCC_Data_ByBuffer_Small_S(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
        int     inRetVal;
        char 	szPrintBuf[84 + 1], szPrintBuf1[84 + 1], szPrintBuf2[84 + 1], szTemplate1[42 + 1], szTemplate2[42 + 1];
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
	char	szExamBit[10 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	char	szShort_Receipt_Mode[1 + 1] = {0};
	unsigned char	uszChangeLineBit = VS_FALSE;
       
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
        memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	memset(szShort_Receipt_Mode, 0x00, sizeof(szShort_Receipt_Mode));
	
	inGetShort_Receipt_Mode(szShort_Receipt_Mode);
	
        if (inPrinttype_ByBuffer)
        {
                /* 直式 */

                /*卡別、卡號*/
                sprintf(szPrintBuf, "卡別　　：%s", pobTran->srBRec.szCardLabel);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡號　　：%s", pobTran->srBRec.szPAN);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*日期、時間*/
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "日期　　：%s",pobTran->srBRec.szDate);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "時間　　：%s",pobTran->srBRec.szTime);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*調閱編號、批次號碼 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "調閱編號：%06ld",pobTran->srBRec.lnOrgInvNum);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "批次號碼：%06ld",pobTran->srBRec.lnBatchNum);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*交易類別*/
                inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "交易類別：%s",szTemplate1);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*授權碼、序號*/
                sprintf(szPrintBuf, "授權碼　：%s",pobTran->srBRec.szAuthCode);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "序號　　：%s",pobTran->srBRec.szRefNo);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        }
        else
        {
                /* 橫式 */
		/* "卡號 卡別" */
		/* "卡號" */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		inFunc_PAD_ASCII(szPrintBuf, "卡號", ' ', 31, _PADDING_RIGHT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 卡別值 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 20, _PADDING_LEFT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 卡別 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", "卡別");
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 卡號值 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		
		strcpy(szPrintBuf1, pobTran->srBRec.szPAN);
		
		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		/* 卡號遮掩(一般卡號前6後4，U Card前3後5) */
		if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
		{
			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			for (i = 6; i < (strlen(szPrintBuf1) - 4); i ++)
			{
				szPrintBuf1[i] = 0x2A;
			}
			
		}
		else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
			 pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
		{
			/* 商店聯卡號遮掩 */
			memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
			inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0 && pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				for (i = 6; i < (strlen(szPrintBuf1) - 4); i ++)
					szPrintBuf1[i] = 0x2A;
			}
			
		}
                
		/* 過卡方式 */
		if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
		{
			if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
				strcat(szPrintBuf1,"(C)");
			else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
				strcat(szPrintBuf1, "(T)");
			else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf1, "(W)");
			else
			{
				if (pobTran->srBRec.uszManualBit == VS_TRUE)
				{
					/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
					/* 電文轉Manual Keyin但是簽單要印感應的W */
					if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
						strcat(szPrintBuf1, "(W)");
					else
						strcat(szPrintBuf1,"(M)");
				}
				else
					strcat(szPrintBuf1,"(S)");
			}
			
		}
		else
		{
			if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf1, "(W)");
			else
				strcat(szPrintBuf1, "(C)");
		}
		
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_WIDTH_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 交易別 */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s", "交易");
		
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
		
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", szTemplate1);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "%s %s", szPrintBuf1, szPrintBuf2);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		if (strlen(szTemplate2) > 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate2);

			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
                /* 城市 主機 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		
		/* 城市 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetCityName(szTemplate1);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s %s", "城市", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		/* 主機 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetHostLabel(szTemplate1);
		inFunc_DiscardSpace(szTemplate1);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PADDING_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", "主機");
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 日期時間 批號 */
		/* 日期時間 */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szTemplate1, "%.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
		sprintf(szPrintBuf1, "%s %s", "日期/時間", szTemplate1);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		
                if (!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))
		{
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		}
		else
		{
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		}
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 批號值 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%03ld", pobTran->srBRec.lnBatchNum);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PADDING_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 批號 */
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", "批號");
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);
		
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
                /* 授權碼 檢查碼 */
		/* 前半段 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szAuthCode[0], 12);
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s %s", "授權碼", szTemplate1);
		inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 32, _PADDING_RIGHT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		
                if (!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))
		{
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		}
		else
		{
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		}
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 後半段 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		if (strlen(pobTran->srBRec.szCheckNO) > 0)
		{
			strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
		}
		else
		{
			inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
		}
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 9, _PADDING_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", "檢查碼");
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 序號 調閱號 */
		/* 序號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szRefNo[0], 12);
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s %s", "序號", szTemplate1);
		inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 29, _PADDING_RIGHT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		
		/* 調閱號值 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%06ld", pobTran->srBRec.lnOrgInvNum);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PADDING_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* "調閱號" */
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", "調閱號");
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 櫃號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetStoreIDEnable(szTemplate1);
		if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
		{
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memcpy(szTemplate1, &pobTran->srBRec.szStoreID[0], 23);
			memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
			sprintf(szPrintBuf1, "%s %s", "櫃號", szTemplate1);
			inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 33, _PADDING_RIGHT_);
			
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szPrintBuf1);

			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
                
                 /* 【需求單-113260】MasterCard機票交易需求 by Russell 2025/3/3 上午 11:55 */
                if (inNCCC_Func_Flight_Ticket_Allow_Print(pobTran) == VS_TRUE)
                {
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "%s", "出發地機場");
                        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);

                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "%s", pobTran->srBRec.szFlightTicketPDS0523);
                        inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_08_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);

                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "%s", "目的地機場");
                        inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_02_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);

                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "%s", pobTran->srBRec.szFlightTicketPDS0524);
                        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);

                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "%s", "航班號碼");
                        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);
			
                        
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        if (strlen(pobTran->srBRec.szFlightTicketPDS0530) > 0)
                        {
                                sprintf(szTemplate2, "%d", atoi(pobTran->srBRec.szFlightTicketPDS0530));
                        }
                        else
                        {
                                sprintf(szTemplate2, "%s", " ");
                        }
                        sprintf(szPrintBuf, "%s", szTemplate2);
                        
                        inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_08_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);

			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			inNCCC_Func_Get_Flight_Ticket_Txn_Category_PrintName(pobTran, szTemplate1);
			
			if (!memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_BAGGAGE_CHARGE_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_BAGGAGE_CHARGE_)))		||
			    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_PREPAID_TICKET_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_PREPAID_TICKET_)))		||
			    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_MISCELLANEOUS_CHARGE_ , max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_MISCELLANEOUS_CHARGE_)))	||
			    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_REISSUE_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_REISSUE_))))
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf)); 
				sprintf(szPrintBuf, "%s", "交易類型");
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));


				sprintf(szPrintBuf, "%s", szTemplate1);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			else
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf)); 
				sprintf(szPrintBuf, "%s", "交易類型");
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_02_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));


				sprintf(szPrintBuf, "%s", szTemplate1);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
                }
                
		/* TC */
		memset(szExamBit, 0x00, sizeof(szExamBit));
		inGetExamBit(szExamBit);
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_ || pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
			if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
			    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
			{
				if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "TC:%02X%02X%02X%02X%02X%02X%02X%02X",
									pobTran->srEMVRec.usz9F26_ApplCryptogram[0],
									pobTran->srEMVRec.usz9F26_ApplCryptogram[1],
									pobTran->srEMVRec.usz9F26_ApplCryptogram[2],
									pobTran->srEMVRec.usz9F26_ApplCryptogram[3],
									pobTran->srEMVRec.usz9F26_ApplCryptogram[4],
									pobTran->srEMVRec.usz9F26_ApplCryptogram[5],
									pobTran->srEMVRec.usz9F26_ApplCryptogram[6],
									pobTran->srEMVRec.usz9F26_ApplCryptogram[7]);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}

				/* 商店聯卡號遮掩 */
				memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
				inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
				if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
				    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
				    strlen(pobTran->srBRec.szTxnNo) > 0)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
                                        if (!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))
                                        {
                                                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _FONT_SIZE_SHORT_RECEIPT_U_TXNO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                        }
                                        else
                                        {
                                                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                        }
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
				
				uszChangeLineBit = VS_FALSE;
				/* AID */
				if (strlen(pobTran->srBRec.szCUP_EMVAID) > 0)
				{
					if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
					{
						if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							uszChangeLineBit = VS_TRUE;
						}
						else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							uszChangeLineBit = VS_TRUE;
						}
						/* 【需求單-111290】VISA置換Logo、DISCOVER簽單新增AID欄位、AE簽單新增AID和AP Label欄位需求 by Russell 2022/12/29 下午 5:57 */
						else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
							pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							uszChangeLineBit = VS_TRUE;
						}
						/* 驗測要印AP Label 和 AID */
						else if (szExamBit[0] == '1')
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							uszChangeLineBit = VS_TRUE;
						}
					}
					else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
					{
						/* AE晶片要印出AID */
						if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY || 
						    pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							uszChangeLineBit = VS_TRUE;
						}
						else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE || 
							 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							uszChangeLineBit = VS_TRUE;
						}
						else if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS	|| pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE) &&
							pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							uszChangeLineBit = VS_TRUE;
						}
						/* 驗測要印AP Label 和 AID */
						else if (szExamBit[0] == '1')
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							uszChangeLineBit = VS_TRUE;
						}
					}
				}
				
				/* M/C交易列印AP Lable (START) */
				if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
				{
					if (pobTran->srEMVRec.in50_APLabelLen > 0)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
						inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}
					else
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "AP Label:");
						inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}
				}
				/* M/C交易列印AP Lable (END) */
				else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
				{
					if (pobTran->srEMVRec.in50_APLabelLen > 0)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
						inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}
					else
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "AP Label:");
						inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}
				}
				/* 驗測要印AP Label 和 AID */
				else if (szExamBit[0] == '1')
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
					inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
				/* 一定要換行 */
				else
				{
					if (uszChangeLineBit == VS_TRUE)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, " ");
						inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}
				}

			}
			else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				/* 商店聯卡號遮掩 */
				/* 持卡人存根也要印 */
				memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
				inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
				if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
				    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
				    strlen(pobTran->srBRec.szTxnNo) > 0)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
                                        if (!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))
                                        {
                                                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _FONT_SIZE_SHORT_RECEIPT_U_TXNO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                        }
                                        else
                                        {
                                                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                        }
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
			}
			
		}
		else
		{
			/* 商店聯卡號遮掩 */
			/* 磁條卡列印交易編號 */
			memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
			inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
			    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
			    strlen(pobTran->srBRec.szTxnNo) > 0)
			{
		 		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		 		sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
                                if (!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))
                                {
                                        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _FONT_SIZE_SHORT_RECEIPT_U_TXNO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                }
                                else
                                {
                                        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                }
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
		}
		
		/* 產品代碼 */
		inGetProductCodeEnable(szProductCodeEnable);
		if (memcmp(szProductCodeEnable, "Y", 1) == 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s %s", "產品代碼", pobTran->srBRec.szProductCode);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
        }

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_DCC_ReceiptEND_ByBuffer_Small_S
Date&Time       :2023/3/21 下午 6:26
Describe        :列印結尾
*/
int inCREDIT_PRINT_DCC_ReceiptEND_ByBuffer_Small_S(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
	int	inRetVal = VS_ERROR;
	char	szSignature[30 + 1] = {0};
	char	szSignaturePath[80 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	
        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                inPRINT_Buffer_PutIn("簽名欄:_____________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
		    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
                {
                        inPRINT_Buffer_PutIn("*** 商店收據 Merchant Copy ***", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_CUST_;

                }
                else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
                {
                        inPRINT_Buffer_PutIn("*** 持卡人收據 Customer Copy ***", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_MERCH_;
                }

                inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
        }
        else
        {
		/* 免責宣言上需空一行 by Russell 2020/10/29 下午 3:21 */
		inRetVal = inPRINT_Buffer_PutIn("  ", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                /* 橫式 */
                if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
                {
			/* Disclaimer */
			if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_))) && 
			    (pobTran->srBRec.inCode == _SALE_		|| 
			     pobTran->srBRec.inCode == _TIP_		|| 
			     pobTran->srBRec.inCode == _PRE_COMP_	||
			    (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
			{
				if (inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(_NCCC_DCC_FILE_VISA_DISCLAIMER_, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle) == VS_ERROR)
					return (VS_ERROR);
			}
			else if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_))) &&
				 (pobTran->srBRec.inCode == _SALE_	|| 
				  pobTran->srBRec.inCode == _TIP_	|| 
				  pobTran->srBRec.inCode == _PRE_COMP_	||
				 (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
			{
				if (inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(_NCCC_DCC_FILE_MASTERCARD_DISCLAIMER_, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle) == VS_ERROR)
					return (VS_ERROR);
			}
			
			if (pobTran->srBRec.uszDCCTransBit == VS_TRUE && pobTran->srBRec.inCode == _TIP_)
			{
				inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				inRetVal = inPRINT_Buffer_PutIn("Cardholder expressly agrees to the Transaction ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				inRetVal = inPRINT_Buffer_PutIn("Receipt Information by marking the “accept box”", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				inRetVal = inPRINT_Buffer_PutIn("below.", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				inRetVal = inPRINT_Buffer_PutIn("[ ] ACCEPT", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}

			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}
				}
			}

			/* 列印空白行 */
			inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
			inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

                        /* 簽名欄 */
			/* 免簽名 */
			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
			{
				inRetVal = inPRINT_Buffer_PutIn("X:      免簽名       ", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			/* 要簽名 */
			else
			{
				/* 藉由TRT_FileName比對來組出bmp的檔名 */
				inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
				
				memset(szSignature, 0x00, sizeof(szSignature));
				/* 因為用invoice所以不用inFunc_ComposeFileName */
				inFunc_ComposeFileName_InvoiceNumber(pobTran, szSignature, _PICTURE_FILE_EXTENSION_, 6);
				memset(szSignaturePath, 0x00, sizeof(szSignaturePath));
				sprintf(szSignaturePath, "./fs_data/%s", szSignature);
				/* 有在signpad簽名*/
				if (inFILE_Check_Exist((unsigned char *)szSignature) == VS_SUCCESS)
				{
					/* 電子簽名 */
					inPRINT_Buffer_PutGraphic((unsigned char *)szSignaturePath, uszBuffer, srBhandle, _SIGNEDPAD_WIDTH_, _APPEND_);
				}
				/* 手簽 */
				else
				{
					inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				
				inPRINT_Buffer_PutIn("X:", _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("------------------------------------------------------", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
                        
			inPRINT_Buffer_PutIn("持卡人簽名(特店存根聯)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
			    pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
			{
				/* 持卡人姓名 */
				inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
                }
		else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
                {
			/* Disclaimer */
			if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_))) &&
			    (pobTran->srBRec.inCode == _SALE_		|| 
			     pobTran->srBRec.inCode == _TIP_		|| 
			     pobTran->srBRec.inCode == _PRE_COMP_	||
			    (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
			{
				if (inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(_NCCC_DCC_FILE_VISA_DISCLAIMER_, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle) == VS_ERROR)
					return (VS_ERROR);
			}
			else if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_))) &&
				 (pobTran->srBRec.inCode == _SALE_		|| 
				  pobTran->srBRec.inCode == _TIP_		|| 
				  pobTran->srBRec.inCode == _PRE_COMP_		||
				 (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
			{
				if (inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(_NCCC_DCC_FILE_MASTERCARD_DISCLAIMER_, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle) == VS_ERROR)
					return (VS_ERROR);
			}
			
			if (pobTran->srBRec.uszDCCTransBit == VS_TRUE && pobTran->srBRec.inCode == _TIP_)
			{
				inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				inRetVal = inPRINT_Buffer_PutIn("Cardholder expressly agrees to the Transaction ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				inRetVal = inPRINT_Buffer_PutIn("Receipt Information by marking the “accept box”", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				inRetVal = inPRINT_Buffer_PutIn("below.", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				inRetVal = inPRINT_Buffer_PutIn("[ ] ACCEPT", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			
			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}

				}
			}
			
			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
			{
				/* 免簽名 */
				inRetVal = inPRINT_Buffer_PutIn("免簽名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			inPRINT_Buffer_PutIn("　　　　　　　　　 商店存根", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
			    pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
			{
				/* 持卡人姓名 */
				inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}
                else
                {
			/* Disclaimer */
			if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_))) &&
			    (pobTran->srBRec.inCode == _SALE_		|| 
			     pobTran->srBRec.inCode == _TIP_		|| 
			     pobTran->srBRec.inCode == _PRE_COMP_	||
			    (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
			{
				if (inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(_NCCC_DCC_FILE_VISA_DISCLAIMER_, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle) == VS_ERROR)
					return (VS_ERROR);
			}
			else if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_))) &&
				 (pobTran->srBRec.inCode == _SALE_		|| 
				  pobTran->srBRec.inCode == _TIP_		|| 
				  pobTran->srBRec.inCode == _PRE_COMP_		||
				 (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
			{
				if (inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(_NCCC_DCC_FILE_MASTERCARD_DISCLAIMER_, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle) == VS_ERROR)
					return (VS_ERROR);
			}
			
			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}
				}
			}
			
			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
			{
				/* 免簽名 */
				inRetVal = inPRINT_Buffer_PutIn("免簽名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			
			/* 持卡人存根Card holder stub 上需空一行 by Russell 2020/10/29 下午 3:18 */
			inRetVal = inPRINT_Buffer_PutIn("  ", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
	
                        inPRINT_Buffer_PutIn("　　　　　持卡人存根 Card holder stub", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
			    pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
			{
				/* 持卡人姓名 */
				inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
                }
		
		if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
		{
			inPRINT_Buffer_PutIn("重印 REPRINT", _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		}
		
                /* 列印警示語 */
                inPRINT_Buffer_PutIn("            I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("        ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		/* Print Notice */
		if (inCREDIT_PRINT_Notice(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
			return (VS_ERROR);
		
		/* Print Slogan */
		if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
		{
			if (inCREDIT_PRINT_MarchantSlogan(pobTran, _NCCC_SLOGAN_PRINT_DOWN_, uszBuffer, srBhandle) != VS_SUCCESS)
				return (VS_ERROR);
			
			if (pobTran->srBRec.uszRewardL1Bit == VS_TRUE	|| 
			    pobTran->srBRec.uszRewardL2Bit == VS_TRUE	||
			    pobTran->srBRec.uszRewardL5Bit == VS_TRUE)
			{
				if (inCREDIT_PRINT_RewardAdvertisement(pobTran, uszBuffer, srFont_Attrib, srBhandle) != VS_SUCCESS)
					return (VS_ERROR);
			}
		}
		
		for (i = 0; i < 6; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
        }

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_FISC_Data_ByBuffer_Small_S
Date&Time       :2023/3/22 下午 2:20
Describe        :列印DATA
*/
int inCREDIT_PRINT_FISC_Data_ByBuffer_Small_S(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
        int     inRetVal;
        char 	szPrintBuf[84 + 1], szPrintBuf1[84 + 1], szPrintBuf2[84 + 1], szTemplate1[42 + 1], szTemplate2[42 + 1];;
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
	char	szShort_Receipt_Mode[1 + 1] = {0};

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
        memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	memset(szShort_Receipt_Mode, 0x00, sizeof(szShort_Receipt_Mode));
	
	inGetShort_Receipt_Mode(szShort_Receipt_Mode);

        if (inPrinttype_ByBuffer)
        {
                /* 直式 */

                /*卡別、卡號*/
                sprintf(szPrintBuf, "卡別　　：%s", pobTran->srBRec.szCardLabel);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡號　　：%s", pobTran->srBRec.szPAN);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*日期、時間*/
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "日期　　：%s",pobTran->srBRec.szDate);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "時間　　：%s",pobTran->srBRec.szTime);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*調閱編號、批次號碼 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "調閱編號：%06ld",pobTran->srBRec.lnOrgInvNum);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "批次號碼：%06ld",pobTran->srBRec.lnBatchNum);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*交易類別*/
                inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "交易類別：%s",szTemplate1);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*授權碼、序號*/
                sprintf(szPrintBuf, "授權碼　：%s",pobTran->srBRec.szAuthCode);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "序號　　：%s",pobTran->srBRec.szRefNo);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        }
        else
        {
                /* 橫式 */
		/* "卡號 卡別" */
		/* "卡號" */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		inFunc_PAD_ASCII(szPrintBuf, "卡號", ' ', 31, _PADDING_RIGHT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 卡別值 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 20, _PADDING_LEFT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* "卡別" */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", "卡別");
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 卡號值 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		
		strcpy(szPrintBuf1, pobTran->srBRec.szPAN);
		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		/* 卡號遮掩(一般卡號前6後4，U Card前3後5) */
		if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
		{
			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			for (i = 6; i < (strlen(szPrintBuf1) - 4); i ++)
			{
				szPrintBuf1[i] = 0x2A;
			}
			
		}
		else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
			 pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
		{
			/* 商店聯卡號遮掩 */
			memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
			inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0 && pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				for (i = 6; i < (strlen(szPrintBuf1) - 4); i ++)
					szPrintBuf1[i] = 0x2A;
			}
		}
		
		/* 過卡方式 */
		if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
		{
			if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
				strcat(szPrintBuf1,"(C)");
			else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
				strcat(szPrintBuf1, "(T)");
			else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf1, "(W)");
			else
			{
				if (pobTran->srBRec.uszManualBit == VS_TRUE)
				{
					/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
					/* 電文轉Manual Keyin但是簽單要印感應的W */
					if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
						strcat(szPrintBuf1, "(W)");
					else
						strcat(szPrintBuf1,"(M)");
				}
				else
					strcat(szPrintBuf1,"(S)");
			}
			
		}
		else
		{
			if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf1, "(W)");
			else
				strcat(szPrintBuf1, "(C)");
		}
		
		sprintf(szPrintBuf, "%s", szPrintBuf1);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_WIDTH_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 交易別 */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s", "交易");
		
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
		
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", szTemplate1);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "%s %s", szPrintBuf1, szPrintBuf2);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		if (strlen(szTemplate2) > 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate2);

			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
                /* 發卡行代碼 主機 */
		/* 發卡行代碼 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szFiscIssuerID[0], 3);
		sprintf(szPrintBuf, "%s %s", "發卡行代碼", szTemplate1);
		inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 31, _PADDING_RIGHT_);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 主機*/
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetHostLabel(szTemplate1);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PADDING_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s %s", "主機", szTemplate1);
		
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 日期時間 批號 */
		/* 日期時間 */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szTemplate1, "%.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
		sprintf(szPrintBuf1, "%s %s", "日期/時間", szTemplate1);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		
                if (!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))
		{
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		}
		else
		{
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		}
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 批號值 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%03ld", pobTran->srBRec.lnBatchNum);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PADDING_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 批號 */
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", "批號");
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);
		
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 序號 調閱號 */
		/* 序號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szRefNo[0], 12);
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s %s", "序號", szTemplate1);
		inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 29, _PADDING_RIGHT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		
		/* 調閱號值 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%06ld", pobTran->srBRec.lnOrgInvNum);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PADDING_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* "調閱號" */
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", "調閱號");
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		
		/* 調單編號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		strcpy(szTemplate1, pobTran->srBRec.szFiscRRN);
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s %s" , "調單編號", szTemplate1);
		
		/* 合併 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "%s",  szPrintBuf1);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 櫃號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetStoreIDEnable(szTemplate1);
		if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
		{
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memcpy(szTemplate1, &pobTran->srBRec.szStoreID[0], 23);
			memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
			sprintf(szPrintBuf1, "%s %s", "櫃號", szTemplate1);
			inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 33, _PADDING_RIGHT_);
			
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szPrintBuf1);

			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
		/* 商店聯卡號遮掩 */
		memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
		inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
		if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
		    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
		    strlen(pobTran->srBRec.szTxnNo) > 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
                        if (!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))
                        {
                                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _FONT_SIZE_SHORT_RECEIPT_U_TXNO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        }
                        else
                        {
                                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        }
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
		/* 產品代碼 */
		inGetProductCodeEnable(szProductCodeEnable);
		if (memcmp(szProductCodeEnable, "Y", 1) == 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s %s", "產品代碼", pobTran->srBRec.szProductCode);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
        }

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_Data_ByBuffer_041_043_Small
Date&Time       :2023/12/21 下午 2:34
Describe        :列印DATA
*/
int inCREDIT_PRINT_Data_ByBuffer_041_043_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
        int     inRetVal;
        char 	szPrintBuf[84 + 1], szPrintBuf1[84 + 1], szPrintBuf2[84 + 1], szTemplate1[84 + 1], szTemplate2[84 + 1];
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
	char	szExamBit[10 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	unsigned char	uszChangeLineBit = VS_FALSE;
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
        memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                /*卡別、卡號*/
		/* 【需求單 - 106349】自有品牌判斷需求 */
		/* unpack電文就存存CardLabel modify by LingHsiung 2020/2/14 上午 11:07 */
		sprintf(szPrintBuf, "卡別　　：%s", pobTran->srBRec.szCardLabel);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡號　　：%s", pobTran->srBRec.szPAN);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*日期、時間*/
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "日期　　：%s",pobTran->srBRec.szDate);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "時間　　：%s",pobTran->srBRec.szTime);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*調閱編號、批次號碼 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "調閱編號：%06ld",pobTran->srBRec.lnOrgInvNum);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "批次號碼：%06ld",pobTran->srBRec.lnBatchNum);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*交易類別*/
                inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "交易類別：%s",szTemplate1);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*授權碼、序號*/
                sprintf(szPrintBuf, "授權碼　：%s",pobTran->srBRec.szAuthCode);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "序號　　：%s",pobTran->srBRec.szRefNo);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        }
        else
        {
                /* 橫式 */
		/* "卡號 卡別" */
		/* "卡號" */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		inFunc_PAD_ASCII(szPrintBuf, "卡號", ' ', 31, _PADDING_RIGHT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 卡別值 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		/* 【需求單 - 106349】自有品牌判斷需求 */
		/* unpack電文就存存CardLabel modify by LingHsiung 2020/2/14 上午 11:07 */
		inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 20, _PADDING_LEFT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 卡別 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", "卡別");
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 卡號值 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		
		strcpy(szPrintBuf1, pobTran->srBRec.szPAN);
		
		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		/* 卡號遮掩(一般卡號前6後4，U Card前3後5) */
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
		if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			/* Uny交易不受TMS開關限制，且獲取卡號時已遮掩 */
		}
		else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
		{
			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			for (i = 6; i < (strlen(szPrintBuf1) - 4); i ++)
			{
				szPrintBuf1[i] = 0x2A;
			}
			
		}
		else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
			 pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
		{
			/* 商店聯卡號遮掩 */
			memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
			inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0 && pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				for (i = 6; i < (strlen(szPrintBuf1) - 4); i ++)
					szPrintBuf1[i] = 0x2A;
			}
		}
                
		/* 過卡方式 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf1, "(W)");
			else
				strcat(szPrintBuf1, "(C)");
		}
		else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			strcat(szPrintBuf1, "(Q)");
		}
		else if (pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
		{
			strcat(szPrintBuf1, "(Q)");
		}
		else
		{
			if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
				strcat(szPrintBuf1,"(C)");
			else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
				strcat(szPrintBuf1, "(T)");
			else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf1, "(W)");
			else
			{
				if (pobTran->srBRec.uszManualBit == VS_TRUE)
				{
					/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
					/* 電文轉Manual Keyin但是簽單要印感應的W */
					if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
						strcat(szPrintBuf1, "(W)");
					else
						strcat(szPrintBuf1,"(M)");
				}
				else
					strcat(szPrintBuf1,"(S)");
			}
			
		}
		
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_WIDTH_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 交易別 */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s", "交易");
		
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
		
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", szTemplate1);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "%s %s", szPrintBuf1, szPrintBuf2);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		if (strlen(szTemplate2) > 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate2);

			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
                /* 城市 主機 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		
		/* 城市 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetCityName(szTemplate1);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s %s", "城市", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		/* 主機 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetHostLabel(szTemplate1);
		inFunc_DiscardSpace(szTemplate1);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PADDING_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", "主機");
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 日期時間 批號 */
		/* 日期時間 */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szTemplate1, "%.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
		sprintf(szPrintBuf1, "%s %s", "日期/時間", szTemplate1);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 批號值 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%03ld", pobTran->srBRec.lnBatchNum);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PADDING_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 批號 */
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", "批號");
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);
		
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
                /* 授權碼 檢查碼 */
		/* 前半段 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szAuthCode[0], 12);
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s %s", "授權碼", szTemplate1);
		inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 32, _PADDING_RIGHT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 後半段 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		if (pobTran->srBRec.uszTakaTransBit == VS_TRUE)
		{
			sprintf(szTemplate1, "%.2s/'%.2s", pobTran->srBRec.szExpDate + 2, pobTran->srBRec.szExpDate);
		}
		else
		{
			/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
			if (strlen(pobTran->srBRec.szCheckNO) > 0)
			{
				strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
			}
			else
			{
				inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
			}
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 9, _PADDING_LEFT_);
		}
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		if (pobTran->srBRec.uszTakaTransBit == VS_TRUE)
		{
			memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
			sprintf(szPrintBuf2, "%s", "有效期");
		}
		else
		{
			memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
			sprintf(szPrintBuf2, "%s", "檢查碼");
		}
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 序號 調閱號 */
		/* 序號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szRefNo[0], 12);
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s %s", "序號", szTemplate1);
		inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 29, _PADDING_RIGHT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		
		/* 調閱號值 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%06ld", pobTran->srBRec.lnOrgInvNum);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PADDING_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* "調閱號" */
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", "調閱號");
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 櫃號 回覆碼 */
		/* 櫃號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetStoreIDEnable(szTemplate1);
		if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
		{
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memcpy(szTemplate1, &pobTran->srBRec.szStoreID[0], 23);
			memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
			sprintf(szPrintBuf1, "%s %s", "櫃號", szTemplate1);
			inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 33, _PADDING_RIGHT_);
			
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szPrintBuf1);

			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* 後半段(銀聯交易才印回覆碼) */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%s", pobTran->srBRec.szRespCode);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 9, _PADDING_LEFT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate1);

				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
				sprintf(szPrintBuf2, "%s", "回覆碼");
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szPrintBuf2);

				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			else
			{
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
		}
		else
		{
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				/* 後半段(銀聯交易才印回覆碼) */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%s", pobTran->srBRec.szRespCode);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 9, _PADDING_LEFT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate1);

				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
				sprintf(szPrintBuf2, "%s", "回覆碼");
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szPrintBuf2);

				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
		}
		/* 產品代碼 */
		inGetProductCodeEnable(szProductCodeEnable);
		if (memcmp(szProductCodeEnable, "Y", 1) == 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s %s", "產品代碼", pobTran->srBRec.szProductCode);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
		/* TC */
		memset(szExamBit, 0x00, sizeof(szExamBit));
		inGetExamBit(szExamBit);
		/* Uny交易 */
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
		if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			/* 取消或退貨，只印數字交易碼 */
			/* 最小字體 */
			if (pobTran->srBRec.uszVOIDBit == VS_TRUE ||
			    pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "交易碼 %s", pobTran->srBRec.szUnyTransCode);
				
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			/* 正向交易，要印條碼和數字 */
			else
			{
				
				if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
				{
					/* 一維條碼 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					strcpy(szPrintBuf, pobTran->srBRec.szUnyTransCode);
					inPRINT_Buffer_Barcode(szPrintBuf, uszBuffer, srBhandle, 0, VS_FALSE, _PRINT_BARCODE_TYPE_DEFAULT_, _PRINT_BARCODE_X_EXTEND_DEFAULT_, _PRINT_BARCODE_Y_EXTEND_DEFAULT_);
				
					/* 數字 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "交易碼(退貨請掃描) %s", pobTran->srBRec.szUnyTransCode);

					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					/* 一維條碼 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					strcpy(szPrintBuf, pobTran->srBRec.szUnyTransCode);
					inPRINT_Buffer_Barcode(szPrintBuf, uszBuffer, srBhandle, 0, VS_FALSE, _PRINT_BARCODE_TYPE_DEFAULT_, _PRINT_BARCODE_X_EXTEND_DEFAULT_, _PRINT_BARCODE_Y_EXTEND_DEFAULT_);
				
					/* 數字 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "交易碼 %s", pobTran->srBRec.szUnyTransCode);

					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
			}
		}
		/* 實體卡交易和掃碼交易互斥 */
		else
		{
			if (pobTran->srBRec.inChipStatus == _EMV_CARD_ || pobTran->srBRec.uszContactlessBit == VS_TRUE)
			{
				/* 感應磁條 */
				if (!memcmp(pobTran->srBRec.szAuthCode, "VLP", 3)				|| 
				    !memcmp(pobTran->srBRec.szAuthCode, "JCB", 3)				||
				     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE	|| 
				     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD		||
				     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
				{
					/* 商店聯卡號遮掩 */
					memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
					inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
					if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
					    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
					    strlen(pobTran->srBRec.szTxnNo) > 0)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}
				}
				else 
				{
					if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
					    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
					{
						if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "TC:%02X%02X%02X%02X%02X%02X%02X%02X",
											pobTran->srEMVRec.usz9F26_ApplCryptogram[0],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[1],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[2],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[3],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[4],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[5],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[6],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[7]);
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}

						/* 商店聯卡號遮掩 */
						memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
						inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
						if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
						    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
						    strlen(pobTran->srBRec.szTxnNo) > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}

						uszChangeLineBit = VS_FALSE;
						/* AID */
						if (strlen(pobTran->srBRec.szCUP_EMVAID) > 0)
						{
							if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
							{
								/* CUP晶片要印 */
								if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								/* 【需求單-111290】VISA置換Logo、DISCOVER簽單新增AID欄位、AE簽單新增AID和AP Label欄位需求 by Russell 2022/12/29 下午 5:57 */
								else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
									pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								/* 驗測要印AP Label 和 AID */
								else if (szExamBit[0] == '1')
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
							}
							else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
							{
								/* 銀聯閃付 */
								if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								/* AE晶片要印出AID */
								else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY || 
									 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE || 
									 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								else if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS	|| pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE) &&
									pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								/* 驗測要印AP Label 和 AID */
								else if (szExamBit[0] == '1')
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
							}
						}

						/* M/C交易列印AP Lable (START) */
						if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
						{
							if (pobTran->srEMVRec.in50_APLabelLen > 0)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
								inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
								if (inRetVal != VS_SUCCESS)
									return (VS_ERROR);
							}
							else
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, "AP Label:");
								inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
								if (inRetVal != VS_SUCCESS)
									return (VS_ERROR);
							}
						}
						/* M/C交易列印AP Lable (END) */
						else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
						{
							if (pobTran->srEMVRec.in50_APLabelLen > 0)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
								inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
								if (inRetVal != VS_SUCCESS)
									return (VS_ERROR);
							}
							else
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, "AP Label:");
								inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
								if (inRetVal != VS_SUCCESS)
									return (VS_ERROR);
							}
						}
						/* 驗測要印AP Label 和 AID */
						else if (szExamBit[0] == '1')
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
							inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
						/* 一定要換行 */
						else
						{
							if (uszChangeLineBit == VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, " ");
								inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
								if (inRetVal != VS_SUCCESS)
									return (VS_ERROR);
							}
						}

					}
					else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
					{
						/* 商店聯卡號遮掩 */
						/* 持卡人存根也要印 */
						memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
						inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
						if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
						    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
						    strlen(pobTran->srBRec.szTxnNo) > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
					}
				}

			}
			else
			{
				/* 商店聯卡號遮掩 */
				/* 磁條卡列印交易編號 */
				memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
				inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
				if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
				    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
				    strlen(pobTran->srBRec.szTxnNo) > 0)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
			}
		}
		
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, 3))
		{
			if ((pobTran->srBRec.inPrintOption == _PRT_MERCH_  ||
			     pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_) &&
			    pobTran->srBRec.inCode != _VOID_ &&
			   (pobTran->inTransactionCode == _CASH_ADVANCE_ || pobTran->inTransactionCode == _FORCE_CASH_ADVANCE_ || pobTran->inRunOperationID == _OPERATION_REPRINT_))
			{
				inPRINT_Buffer_PutIn("四碼之識別碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("Bank Identification Number", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("__________________________________________", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("持卡人證照號碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("Cardholder’s Passport/ID Card Number", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("__________________________________________", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("證件有效期", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("Passport/ID Card Expiration Date", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("__________________________________________", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("核發證照機關", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("Issuing Authority", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("__________________________________________", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("櫃檯人員簽名", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("Cashier’s Signature", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("__________________________________________", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}
        }

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_Amount_ByBuffer_041_043_Small
Date&Time       :2023/12/21 下午 3:05
Describe        :列印AMOUNT
*/
int inCREDIT_PRINT_Amount_ByBuffer_041_043_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                /* 金額 */
                if(pobTran->srBRec.inCode == _TIP_)
                {
                        /* 金額 */
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTxnAmount);
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


                        /* 小費 */
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTipTxnAmount);
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PADDING_LEFT_);

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "小費(Tips)  :%s", szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


                        /* 總計 */
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ' , 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf,"總計(Total) :%s", szTemplate);
                }
                else if ((pobTran->srBRec.uszVOIDBit == VS_TRUE	&& 
			 (pobTran->srBRec.inOrgCode != _REFUND_ && pobTran->srBRec.inOrgCode != _INST_REFUND_ && pobTran->srBRec.inOrgCode != _REDEEM_REFUND_)) ||
			  pobTran->srBRec.inCode == _REFUND_	 || 
			  pobTran->srBRec.inCode == _INST_REFUND_|| 
			  pobTran->srBRec.inCode == _REDEEM_REFUND_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", 0 - pobTran->srBRec.lnTxnAmount);
                        inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                }
                else if (pobTran->srBRec.inCode == _ADJUST_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnAdjustTxnAmount);
                        inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                }
                else
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTxnAmount);
                        inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                }
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                
		for (i = 0; i < 2; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
        }
	else
	{
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			if (pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inCode == _VOID_)
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
				inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", "總計(Total) :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", "總計(Total) :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}
		else
		{
			/* 橫式 */
			/* 負向交易 */
			if(pobTran->srBRec.uszVOIDBit == VS_TRUE)
			{
				/* 橫式 */
				/* 金額 */
				/* 取消退貨是正數 */
				if (pobTran->srBRec.inOrgCode == _REFUND_ || pobTran->srBRec.inOrgCode == _INST_REFUND_ || pobTran->srBRec.inOrgCode == _REDEEM_REFUND_)
				{
					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
					inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", "總計(Total) :");
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				else
				{
					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
					inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", "總計(Total) :");
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}

			}
			/* 正向交易 */
			else
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetTransFunc(szTemplate);
				if (szTemplate[6] == 'Y')	/* 檢查是否有開小費 */
				{
					/* 退貨金額為負數 */
					if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
					{
						/* 將NT$ ＋數字塞到szTemplate中來inpad */
						memset(szTemplate, 0x00, sizeof(szTemplate));
						sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
						inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", szTemplate);
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

						/* 把前面的字串和數字結合起來 */
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", "總計(Total) :");
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					}
					/* 預授不會有小費，所以拉出來 */
					else if (pobTran->srBRec.inCode == _PRE_AUTH_ || pobTran->srBRec.inCode == _PRE_COMP_)
					{
						/* 將NT$ ＋數字塞到szTemplate中來inpad */
						memset(szTemplate, 0x00, sizeof(szTemplate));
						sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
						inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", szTemplate);
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

						/* 把前面的字串和數字結合起來 */
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", "總計(Total) :");
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					}
					else
					{
						/* 金額 */
						/* 將NT$ ＋數字塞到szTemplate中來inpad */
						memset(szTemplate, 0x00, sizeof(szTemplate));
						sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
						inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", szTemplate);
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

						/* 把前面的字串和數字結合起來 */
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", "金額(Amount):");
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

						/* lnTipAmount為0表示非小費 */
						if (pobTran->srBRec.lnTipTxnAmount == 0L)
						{
							/* 小費 */
							inPRINT_Buffer_PutIn("小費(Tips)  :______________________________________", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

							/* 總計 */
							inPRINT_Buffer_PutIn("總計(Total) :______________________________________", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						}
						else
						{
							/* 小費 */
							/* 將NT$ ＋數字塞到szTemplate中來inpad */
							memset(szTemplate, 0x00, sizeof(szTemplate));
							sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTipTxnAmount);
							inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "%s", szTemplate);
							inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

							/* 把前面的字串和數字結合起來 */
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "%s", "小費(Tips)  :");
							inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


							/* 總計 */
							/* 將NT$ ＋數字塞到szTemplate中來inpad */
							memset(szTemplate, 0x00, sizeof(szTemplate));
							sprintf(szTemplate, "%ld",  (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
							inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "%s", szTemplate);
							inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

							/* 把前面的字串和數字結合起來 */
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "%s", "總計(Total) :");
							inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						}

					}

				}
				/* 小費沒開時 */
				else
				{
					/* 總計 */
					/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
					{
						sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
					}
					else
					{
						sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
					}
					inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);


					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", "總計(Total) :");
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
			}
		}
	}
	
	/* 斷行 */
	inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_ReceiptEND_ByBuffer_041_043_Small
Date&Time       :2023/12/21 下午 3:46
Describe        :
*/
int inCREDIT_PRINT_ReceiptEND_ByBuffer_041_043_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
	int	inDisclaimerFontSize = 0;
	char	szSignature[30 + 1] = {0};
	char	szSignaturePath[80 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                inPRINT_Buffer_PutIn("簽名欄:_____________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
		    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
                {
                        inPRINT_Buffer_PutIn("*** 商店收據 Merchant Copy ***", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_CUST_;

                }
                else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
                {
                        inPRINT_Buffer_PutIn("*** 持卡人收據 Customer Copy ***", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_MERCH_;
                }

                inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
        }
        else
        {
                /* 橫式 */
                if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
                {
			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}

				}
			}
			
                        /* 簽名欄 */
			/* 藉由TRT_FileName比對來組出bmp的檔名 */
			inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

			memset(szSignature, 0x00, sizeof(szSignature));
			/* 因為用invoice所以不用inFunc_ComposeFileName */
			inFunc_ComposeFileName_InvoiceNumber(pobTran, szSignature, _PICTURE_FILE_EXTENSION_, 6);
			memset(szSignaturePath, 0x00, sizeof(szSignaturePath));
			sprintf(szSignaturePath, "./fs_data/%s", szSignature);
			/* 圖檔存在、有在signpad簽名、且非重印（重印不出簽名）（目前簽名狀態存不了Batch，先把&& pobTran->srBRec.inSignStatus == _SIGN_SIGNED_此條件拿掉） */
			if (inFILE_Check_Exist((unsigned char *)szSignature) == VS_SUCCESS && pobTran->inRunOperationID != _OPERATION_REPRINT_)
			{
				/* 電子簽名 */
				inPRINT_Buffer_PutGraphic((unsigned char *)szSignaturePath, uszBuffer, srBhandle, _SIGNEDPAD_WIDTH_, _APPEND_);
			}
			/* 手簽 */
			else
			{
				/* a space 2 line */
				for (i = 0; i < 2; i++)
				{
					inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}

			}	
                }
		else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
                {
			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}

				}
			}

			/* 簽名欄 */
			/* 藉由TRT_FileName比對來組出bmp的檔名 */
			inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

			memset(szSignature, 0x00, sizeof(szSignature));
			/* 因為用invoice所以不用inFunc_ComposeFileName */
			inFunc_ComposeFileName_InvoiceNumber(pobTran, szSignature, _PICTURE_FILE_EXTENSION_, 6);
			memset(szSignaturePath, 0x00, sizeof(szSignaturePath));
			sprintf(szSignaturePath, "./fs_data/%s", szSignature);
			/* 圖檔存在、有在signpad簽名、且非重印（重印不出簽名）（目前簽名狀態存不了Batch，先把&& pobTran->srBRec.inSignStatus == _SIGN_SIGNED_此條件拿掉） */
			if (inFILE_Check_Exist((unsigned char *)szSignature) == VS_SUCCESS && pobTran->inRunOperationID != _OPERATION_REPRINT_)
			{
				/* 電子簽名 */
				inPRINT_Buffer_PutGraphic((unsigned char *)szSignaturePath, uszBuffer, srBhandle, _SIGNEDPAD_WIDTH_, _APPEND_);
			}
			/* 手簽 */
			else
			{
				/* a space 2 line */
				for (i = 0; i < 2; i++)
				{
					inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}

			}
		}
                else
                {
			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}

				}
			}
                }
		
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
			{
				inPRINT_Buffer_PutIn("X:________________________________", _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("持卡人簽名(特店存根聯)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				/* 列印持卡人姓名 */
				inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				/* Mail 主旨: RE: 【112184】V3標準版郵購交易及客製化041、043預借現金不印免簽名調整之需求已發信問過登霖，預借現金簽單是否比照簽單規格，將Cardholder’s Signature」移除 2024/1/9 下午 3:10 */
			}
			else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
			{
				inPRINT_Buffer_PutIn("X:________________________________", _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("持卡人簽名(特店存根聯)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("商店存根", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				/* 列印持卡人姓名 */
				inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				inPRINT_Buffer_PutIn("持卡人存根 Card holder stub", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("------------------------------------------", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				/* 列印持卡人姓名 */
				inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			
			if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
			{
				inPRINT_Buffer_PutIn("重印 REPRINT", _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			}

			/* 列印警示語 */
			inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

			/* 預借現金客製化專屬，於帳單最下方要印出備註標語 */
			if ((pobTran->inTransactionCode == _CASH_ADVANCE_ || pobTran->inTransactionCode == _FORCE_CASH_ADVANCE_ || pobTran->inRunOperationID == _OPERATION_REPRINT_) &&
			    pobTran->srBRec.inCode != _VOID_)
			{
				inDisclaimerFontSize = _PRT_HEIGHT_SMALL_;
				inPRINT_Buffer_PutIn("備註：依據發卡機構與持卡人之約定，發卡機構得向", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("持卡人收手續費", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("Note: By the agreement of issuing bank and", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("cardholder, the Issuing bank may charge", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("handling fee to the cardholder.", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			
			/* Print Notice */
			if (inCREDIT_PRINT_Notice(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
				return (VS_ERROR);

			/* Print Slogan */
			if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				if (inCREDIT_PRINT_MarchantSlogan(pobTran, _NCCC_SLOGAN_PRINT_DOWN_, uszBuffer, srBhandle) != VS_SUCCESS)
					return (VS_ERROR);
			}

			for (i = 0; i < 8; i++)
			{
				inPRINT_Buffer_PutIn("", _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
		{
			if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
			    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
			{
				inPRINT_Buffer_PutIn("X:________________________________", _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("持卡人簽名(特店存根聯)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				/* 列印持卡人姓名 */
				inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				inPRINT_Buffer_PutIn("持卡人存根 Card holder stub", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("------------------------------------------", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				/* 列印持卡人姓名 */
				inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			
			if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
			{
				inPRINT_Buffer_PutIn("重印 REPRINT", _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			}

			/* 列印警示語 */
			inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

			/* 建設公司客製化專屬，帳單聲明書列印(START) */
			if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
			{
				inDisclaimerFontSize = _PRT_HEIGHT_SMALL_;
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("聲明書", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("本人瞭解以信用卡刷卡僅限於支付本項交易訂金金額，", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("此訂金係為向賣方取得優先購買指定房屋之權利而支付", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("，買、賣雙方於完成房屋契約簽約手續後，本人支付之", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("訂金即依約轉為房屋價款之一部分，本人支付訂金之目", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("的即已獲滿足，嗣後本人絕不再以賣方之服務未提供或", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("商品未交付為由，向信用卡發卡機構或收單機構要求扣", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("款，特立本聲明書為憑。", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				inPRINT_Buffer_PutIn("立聲明書人：＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("（請簽信用卡持卡人之本人中文姓名）", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("請款聯", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else if(pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
			{
				inDisclaimerFontSize = _PRT_HEIGHT_SMALL_;
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("聲明書", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("本人瞭解以信用卡刷卡僅限於支付本項交易訂金金額，", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("此訂金係為向賣方取得優先購買指定房屋之權利而支付", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("，買、賣雙方於完成房屋契約簽約手續後，本人支付之", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("訂金即依約轉為房屋價款之一部分，本人支付訂金之目", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("的即已獲滿足，嗣後本人絕不再以賣方之服務未提供或", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("商品未交付為由，向信用卡發卡機構或收單機構要求扣", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("款，特立本聲明書為憑。", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				inPRINT_Buffer_PutIn("立聲明書人：＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("（請簽信用卡持卡人之本人中文姓名）", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("特店存根聯", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else if(pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				inDisclaimerFontSize = _PRT_HEIGHT_SMALL_;
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("本人瞭解以信用卡刷卡僅限於支付本項交易訂金金額，", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("此訂金係為向賣方取得優先購買指定房屋之權利而支付", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("，買、賣雙方於完成房屋契約簽約手續後，本人支付之", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("訂金即依約轉為房屋價款之一部分，本人支付訂金之目", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("的即已獲滿足，嗣後本人絕不再以賣方之服務未提供或", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("商品未交付為由，向信用卡發卡機構或收單機構要求扣", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("款，特立本聲明書為憑。", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				inPRINT_Buffer_PutIn("持卡人存根聯", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			/* 建設公司客製化專屬，帳單聲明書列印(END) */
			
			/* Print Notice */
			if (inCREDIT_PRINT_Notice(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
				return (VS_ERROR);

			/* Print Slogan */
			if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				if (inCREDIT_PRINT_MarchantSlogan(pobTran, _NCCC_SLOGAN_PRINT_DOWN_, uszBuffer, srBhandle) != VS_SUCCESS)
					return (VS_ERROR);
			}

			for (i = 0; i < 8; i++)
			{
				inPRINT_Buffer_PutIn("", _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}
		else
		{
			if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
			{
				inPRINT_Buffer_PutIn("持卡人簽名(特店存根聯)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("------------------------------------------", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				/* 列印持卡人姓名 */
				inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
			{
				inPRINT_Buffer_PutIn("商店存根", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("------------------------------------------", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				/* 列印持卡人姓名 */
				inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				inPRINT_Buffer_PutIn("持卡人存根 Card holder stub", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("------------------------------------------", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				/* 列印持卡人姓名 */
				inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			
			if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
			{
				inPRINT_Buffer_PutIn("重印 REPRINT", _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			}

			/* 列印警示語 */
			inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

			/* Print Notice */
			if (inCREDIT_PRINT_Notice(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
				return (VS_ERROR);

			/* Print Slogan */
			if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				if (inCREDIT_PRINT_MarchantSlogan(pobTran, _NCCC_SLOGAN_PRINT_DOWN_, uszBuffer, srBhandle) != VS_SUCCESS)
					return (VS_ERROR);
			}

			for (i = 0; i < 8; i++)
			{
				inPRINT_Buffer_PutIn("", _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}
		
        }

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Data_ByBuffer_041_043_Small_S
Date&Time       :2023/12/25 下午 2:39
Describe        :列印DATA
*/
int inCREDIT_PRINT_Data_ByBuffer_041_043_Small_S(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
        int     inRetVal;
        char 	szPrintBuf[84 + 1], szPrintBuf1[84 + 1], szPrintBuf2[84 + 1], szTemplate1[84 + 1], szTemplate2[84 + 1];
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
	char	szExamBit[10 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
        char    szShort_Receipt_Mode[1 + 1] = {0};
	unsigned char	uszChangeLineBit = VS_FALSE;
       
        memset(szShort_Receipt_Mode, 0x00, sizeof(szShort_Receipt_Mode));
        inGetShort_Receipt_Mode(szShort_Receipt_Mode);
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
        memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                /*卡別、卡號*/
		/* 【需求單 - 106349】自有品牌判斷需求 */
		/* unpack電文就存存CardLabel modify by LingHsiung 2020/2/14 上午 11:07 */
		sprintf(szPrintBuf, "卡別　　：%s", pobTran->srBRec.szCardLabel);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡號　　：%s", pobTran->srBRec.szPAN);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*日期、時間*/
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "日期　　：%s",pobTran->srBRec.szDate);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "時間　　：%s",pobTran->srBRec.szTime);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*調閱編號、批次號碼 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "調閱編號：%06ld",pobTran->srBRec.lnOrgInvNum);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "批次號碼：%06ld",pobTran->srBRec.lnBatchNum);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*交易類別*/
                inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "交易類別：%s",szTemplate1);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*授權碼、序號*/
                sprintf(szPrintBuf, "授權碼　：%s",pobTran->srBRec.szAuthCode);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "序號　　：%s",pobTran->srBRec.szRefNo);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        }
        else
        {
                /* 橫式 */
		/* "卡號 卡別" */
		/* "卡號" */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		inFunc_PAD_ASCII(szPrintBuf, "卡號", ' ', 31, _PADDING_RIGHT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 卡別值 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		/* 【需求單 - 106349】自有品牌判斷需求 */
		/* unpack電文就存存CardLabel modify by LingHsiung 2020/2/14 上午 11:07 */
		inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 20, _PADDING_LEFT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 卡別 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", "卡別");
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 卡號值 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		
		strcpy(szPrintBuf1, pobTran->srBRec.szPAN);
		
		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		/* 卡號遮掩(一般卡號前6後4，U Card前3後5) */
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
		if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			/* Uny交易不受TMS開關限制，且獲取卡號時已遮掩 */
		}
		else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
		{
			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			for (i = 6; i < (strlen(szPrintBuf1) - 4); i ++)
			{
				szPrintBuf1[i] = 0x2A;
			}
			
		}
		else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
			 pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
		{
			/* 商店聯卡號遮掩 */
			memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
			inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0 && pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				for (i = 6; i < (strlen(szPrintBuf1) - 4); i ++)
					szPrintBuf1[i] = 0x2A;
			}
		}
                
		/* 過卡方式 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf1, "(W)");
			else
				strcat(szPrintBuf1, "(C)");
		}
		else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			strcat(szPrintBuf1, "(Q)");
		}
		else if (pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
		{
			strcat(szPrintBuf1, "(Q)");
		}
		else
		{
			if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
				strcat(szPrintBuf1,"(C)");
			else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
				strcat(szPrintBuf1, "(T)");
			else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf1, "(W)");
			else
			{
				if (pobTran->srBRec.uszManualBit == VS_TRUE)
				{
					/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
					/* 電文轉Manual Keyin但是簽單要印感應的W */
					if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
						strcat(szPrintBuf1, "(W)");
					else
						strcat(szPrintBuf1,"(M)");
				}
				else
					strcat(szPrintBuf1,"(S)");
			}
			
		}
		
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_WIDTH_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 交易別 */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s", "交易");
		
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
		
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", szTemplate1);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "%s %s", szPrintBuf1, szPrintBuf2);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		if (strlen(szTemplate2) > 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate2);

			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
                /* 城市 主機 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		
		/* 城市 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetCityName(szTemplate1);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s %s", "城市", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		/* 主機 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetHostLabel(szTemplate1);
		inFunc_DiscardSpace(szTemplate1);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PADDING_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", "主機");
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 日期時間 批號 */
		/* 日期時間 */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szTemplate1, "%.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
		sprintf(szPrintBuf1, "%s %s", "日期/時間", szTemplate1);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 批號值 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%03ld", pobTran->srBRec.lnBatchNum);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PADDING_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 批號 */
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", "批號");
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);
		
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
                /* 授權碼 檢查碼 */
		/* 前半段 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szAuthCode[0], 12);
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s %s", "授權碼", szTemplate1);
		inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 32, _PADDING_RIGHT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 後半段 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		if (pobTran->srBRec.uszTakaTransBit == VS_TRUE)
		{
			sprintf(szTemplate1, "%.2s/'%.2s", pobTran->srBRec.szExpDate + 2, pobTran->srBRec.szExpDate);
		}
		else
		{
			/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
			if (strlen(pobTran->srBRec.szCheckNO) > 0)
			{
				strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
			}
			else
			{
				inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
			}
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 9, _PADDING_LEFT_);
		}
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		if (pobTran->srBRec.uszTakaTransBit == VS_TRUE)
		{
			memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
			sprintf(szPrintBuf2, "%s", "有效期");
		}
		else
		{
			memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
			sprintf(szPrintBuf2, "%s", "檢查碼");
		}
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 序號 調閱號 */
		/* 序號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szRefNo[0], 12);
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s %s", "序號", szTemplate1);
		inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 29, _PADDING_RIGHT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		
		/* 調閱號值 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%06ld", pobTran->srBRec.lnOrgInvNum);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PADDING_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* "調閱號" */
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", "調閱號");
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 櫃號 回覆碼 */
		/* 櫃號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetStoreIDEnable(szTemplate1);
		if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
		{
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memcpy(szTemplate1, &pobTran->srBRec.szStoreID[0], 23);
			memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
			sprintf(szPrintBuf1, "%s %s", "櫃號", szTemplate1);
			inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 33, _PADDING_RIGHT_);
			
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szPrintBuf1);

			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* 後半段(銀聯交易才印回覆碼) */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%s", pobTran->srBRec.szRespCode);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 9, _PADDING_LEFT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate1);

				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
				sprintf(szPrintBuf2, "%s", "回覆碼");
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szPrintBuf2);

				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			else
			{
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
		}
		else
		{
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				/* 後半段(銀聯交易才印回覆碼) */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%s", pobTran->srBRec.szRespCode);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 9, _PADDING_LEFT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate1);

				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
				sprintf(szPrintBuf2, "%s", "回覆碼");
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szPrintBuf2);

				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
		}
		/* 產品代碼 */
		inGetProductCodeEnable(szProductCodeEnable);
		if (memcmp(szProductCodeEnable, "Y", 1) == 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s %s", "產品代碼", pobTran->srBRec.szProductCode);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
		/* TC */
		memset(szExamBit, 0x00, sizeof(szExamBit));
		inGetExamBit(szExamBit);
		/* Uny交易 */
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
		if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			/* 取消或退貨，只印數字交易碼 */
			/* 最小字體 */
			if (pobTran->srBRec.uszVOIDBit == VS_TRUE ||
			    pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "交易碼 %s", pobTran->srBRec.szUnyTransCode);
				
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			/* 正向交易，要印條碼和數字 */
			else
			{
				
				if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
				{
					/* 一維條碼 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					strcpy(szPrintBuf, pobTran->srBRec.szUnyTransCode);
					inPRINT_Buffer_Barcode(szPrintBuf, uszBuffer, srBhandle, 0, VS_FALSE, _PRINT_BARCODE_TYPE_DEFAULT_, _PRINT_BARCODE_X_EXTEND_DEFAULT_, 2);
				
					/* 數字 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "交易碼(退貨請掃描) %s", pobTran->srBRec.szUnyTransCode);

					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					/* 一維條碼 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					strcpy(szPrintBuf, pobTran->srBRec.szUnyTransCode);
					inPRINT_Buffer_Barcode(szPrintBuf, uszBuffer, srBhandle, 0, VS_FALSE, _PRINT_BARCODE_TYPE_DEFAULT_, _PRINT_BARCODE_X_EXTEND_DEFAULT_, 2);
				
					/* 數字 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "交易碼 %s", pobTran->srBRec.szUnyTransCode);

					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
			}
		}
		/* 實體卡交易和掃碼交易互斥 */
		else
		{
			if (pobTran->srBRec.inChipStatus == _EMV_CARD_ || pobTran->srBRec.uszContactlessBit == VS_TRUE)
			{
				/* 感應磁條 */
				if (!memcmp(pobTran->srBRec.szAuthCode, "VLP", 3)				|| 
				    !memcmp(pobTran->srBRec.szAuthCode, "JCB", 3)				||
				     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE	|| 
				     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD		||
				     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
				{
					/* 商店聯卡號遮掩 */
					memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
					inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
					if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
					    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
					    strlen(pobTran->srBRec.szTxnNo) > 0)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
                                                if (!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))
                                                {
                                                        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _FONT_SIZE_SHORT_RECEIPT_U_TXNO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                                }
                                                else
                                                {
                                                        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                                }
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}
				}
				else 
				{
					if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
					    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
					{
						if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "TC:%02X%02X%02X%02X%02X%02X%02X%02X",
											pobTran->srEMVRec.usz9F26_ApplCryptogram[0],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[1],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[2],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[3],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[4],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[5],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[6],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[7]);
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}

						/* 商店聯卡號遮掩 */
						memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
						inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
						if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
						    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
						    strlen(pobTran->srBRec.szTxnNo) > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
                                                        if (!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))
                                                        {
                                                                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _FONT_SIZE_SHORT_RECEIPT_U_TXNO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                                        }
                                                        else
                                                        {
                                                                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                                        }
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}

						uszChangeLineBit = VS_FALSE;
						/* AID */
						if (strlen(pobTran->srBRec.szCUP_EMVAID) > 0)
						{
							if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
							{
								/* CUP晶片要印 */
								if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								/* 【需求單-111290】VISA置換Logo、DISCOVER簽單新增AID欄位、AE簽單新增AID和AP Label欄位需求 by Russell 2022/12/29 下午 5:57 */
								else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
									pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								/* 驗測要印AP Label 和 AID */
								else if (szExamBit[0] == '1')
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
							}
							else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
							{
								/* 銀聯閃付 */
								if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								/* AE晶片要印出AID */
								else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY || 
									 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE || 
									 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								else if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS	|| pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE) &&
									pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								/* 驗測要印AP Label 和 AID */
								else if (szExamBit[0] == '1')
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
							}
						}

						/* M/C交易列印AP Lable (START) */
						if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
						{
							if (pobTran->srEMVRec.in50_APLabelLen > 0)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
								inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
								if (inRetVal != VS_SUCCESS)
									return (VS_ERROR);
							}
							else
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, "AP Label:");
								inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
								if (inRetVal != VS_SUCCESS)
									return (VS_ERROR);
							}
						}
						/* M/C交易列印AP Lable (END) */
						else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
						{
							if (pobTran->srEMVRec.in50_APLabelLen > 0)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
								inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
								if (inRetVal != VS_SUCCESS)
									return (VS_ERROR);
							}
							else
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, "AP Label:");
								inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
								if (inRetVal != VS_SUCCESS)
									return (VS_ERROR);
							}
						}
						/* 驗測要印AP Label 和 AID */
						else if (szExamBit[0] == '1')
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
							inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
						/* 一定要換行 */
						else
						{
							if (uszChangeLineBit == VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, " ");
								inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
								if (inRetVal != VS_SUCCESS)
									return (VS_ERROR);
							}
						}

					}
					else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
					{
						/* 商店聯卡號遮掩 */
						/* 持卡人存根也要印 */
						memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
						inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
						if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
						    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
						    strlen(pobTran->srBRec.szTxnNo) > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
                                                        if (!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))
                                                        {
                                                                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _FONT_SIZE_SHORT_RECEIPT_U_TXNO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                                        }
                                                        else
                                                        {
                                                                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                                        }
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
					}
				}

			}
			else
			{
				/* 商店聯卡號遮掩 */
				/* 磁條卡列印交易編號 */
				memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
				inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
				if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
				    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
				    strlen(pobTran->srBRec.szTxnNo) > 0)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
                                        if (!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))
                                        {
                                                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _FONT_SIZE_SHORT_RECEIPT_U_TXNO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                        }
                                        else
                                        {
                                                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL_S_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                        }
                                    
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
			}
		}
		
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, 3))
		{
			if ((pobTran->srBRec.inPrintOption == _PRT_MERCH_  ||
			     pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_) &&
			    pobTran->srBRec.inCode != _VOID_ &&
			   (pobTran->inTransactionCode == _CASH_ADVANCE_ || pobTran->inTransactionCode == _FORCE_CASH_ADVANCE_ || pobTran->inRunOperationID == _OPERATION_REPRINT_))
			{
				inPRINT_Buffer_PutIn("四碼之識別碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("Bank Identification Number", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("__________________________________________", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("持卡人證照號碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("Cardholder’s Passport/ID Card Number", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("__________________________________________", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("證件有效期", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("Passport/ID Card Expiration Date", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("__________________________________________", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("核發證照機關", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("Issuing Authority", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("__________________________________________", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("櫃檯人員簽名", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("Cashier’s Signature", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("__________________________________________", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}
        }

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_Amount_ByBuffer_041_043_Small_S
Date&Time       :2023/12/25 下午 6:30
Describe        :列印AMOUNT
*/
int inCREDIT_PRINT_Amount_ByBuffer_041_043_Small_S(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                /* 金額 */
                if(pobTran->srBRec.inCode == _TIP_)
                {
                        /* 金額 */
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTxnAmount);
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


                        /* 小費 */
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTipTxnAmount);
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PADDING_LEFT_);

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "小費(Tips)  :%s", szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


                        /* 總計 */
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ' , 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf,"總計(Total) :%s", szTemplate);
                }
                else if ((pobTran->srBRec.uszVOIDBit == VS_TRUE	&& 
			 (pobTran->srBRec.inOrgCode != _REFUND_ && pobTran->srBRec.inOrgCode != _INST_REFUND_ && pobTran->srBRec.inOrgCode != _REDEEM_REFUND_)) ||
			  pobTran->srBRec.inCode == _REFUND_	 || 
			  pobTran->srBRec.inCode == _INST_REFUND_|| 
			  pobTran->srBRec.inCode == _REDEEM_REFUND_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", 0 - pobTran->srBRec.lnTxnAmount);
                        inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                }
                else if (pobTran->srBRec.inCode == _ADJUST_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnAdjustTxnAmount);
                        inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                }
                else
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTxnAmount);
                        inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                }
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                
		for (i = 0; i < 2; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
        }
	else
	{
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			if (pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inCode == _VOID_)
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
				inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", "總計(Total) :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", "總計(Total) :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}
		else
		{
			/* 橫式 */
			/* 負向交易 */
			if(pobTran->srBRec.uszVOIDBit == VS_TRUE)
			{
				/* 橫式 */
				/* 金額 */
				/* 取消退貨是正數 */
				if (pobTran->srBRec.inOrgCode == _REFUND_ || pobTran->srBRec.inOrgCode == _INST_REFUND_ || pobTran->srBRec.inOrgCode == _REDEEM_REFUND_)
				{
					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
					inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", "總計(Total) :");
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				else
				{
					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
					inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", "總計(Total) :");
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}

			}
			/* 正向交易 */
			else
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetTransFunc(szTemplate);
				if (szTemplate[6] == 'Y')	/* 檢查是否有開小費 */
				{
					/* 退貨金額為負數 */
					if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
					{
						/* 將NT$ ＋數字塞到szTemplate中來inpad */
						memset(szTemplate, 0x00, sizeof(szTemplate));
						sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
						inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", szTemplate);
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

						/* 把前面的字串和數字結合起來 */
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", "總計(Total) :");
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					}
					/* 預授不會有小費，所以拉出來 */
					else if (pobTran->srBRec.inCode == _PRE_AUTH_ || pobTran->srBRec.inCode == _PRE_COMP_)
					{
						/* 將NT$ ＋數字塞到szTemplate中來inpad */
						memset(szTemplate, 0x00, sizeof(szTemplate));
						sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
						inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", szTemplate);
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

						/* 把前面的字串和數字結合起來 */
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", "總計(Total) :");
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					}
					else
					{
						/* 金額 */
						/* 將NT$ ＋數字塞到szTemplate中來inpad */
						memset(szTemplate, 0x00, sizeof(szTemplate));
						sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
						inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", szTemplate);
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

						/* 把前面的字串和數字結合起來 */
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", "金額(Amount):");
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

						/* lnTipAmount為0表示非小費 */
						if (pobTran->srBRec.lnTipTxnAmount == 0L)
						{
							/* 小費 */
							inPRINT_Buffer_PutIn("小費(Tips)  :______________________________________", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

							/* 總計 */
							inPRINT_Buffer_PutIn("總計(Total) :______________________________________", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						}
						else
						{
							/* 小費 */
							/* 將NT$ ＋數字塞到szTemplate中來inpad */
							memset(szTemplate, 0x00, sizeof(szTemplate));
							sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTipTxnAmount);
							inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "%s", szTemplate);
							inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

							/* 把前面的字串和數字結合起來 */
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "%s", "小費(Tips)  :");
							inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


							/* 總計 */
							/* 將NT$ ＋數字塞到szTemplate中來inpad */
							memset(szTemplate, 0x00, sizeof(szTemplate));
							sprintf(szTemplate, "%ld",  (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
							inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "%s", szTemplate);
							inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

							/* 把前面的字串和數字結合起來 */
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "%s", "總計(Total) :");
							inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						}

					}

				}
				/* 小費沒開時 */
				else
				{
					/* 總計 */
					/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
					{
						sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
					}
					else
					{
						sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
					}
					inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);


					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", "總計(Total) :");
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
			}
		}
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_ReceiptEND_ByBuffer_041_043_Small_S
Date&Time       :2023/12/26 上午 10:28
Describe        :
*/
int inCREDIT_PRINT_ReceiptEND_ByBuffer_041_043_Small_S(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
	int	inDisclaimerFontSize = 0;
	char	szSignature[30 + 1] = {0};
	char	szSignaturePath[80 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                inPRINT_Buffer_PutIn("簽名欄:_____________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
		    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
                {
                        inPRINT_Buffer_PutIn("*** 商店收據 Merchant Copy ***", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_CUST_;

                }
                else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
                {
                        inPRINT_Buffer_PutIn("*** 持卡人收據 Customer Copy ***", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_MERCH_;
                }

                inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
        }
        else
        {
                /* 橫式 */
                if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
                {
			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}

				}
			}
			
                        /* 簽名欄 */
			/* 藉由TRT_FileName比對來組出bmp的檔名 */
			inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

			memset(szSignature, 0x00, sizeof(szSignature));
			/* 因為用invoice所以不用inFunc_ComposeFileName */
			inFunc_ComposeFileName_InvoiceNumber(pobTran, szSignature, _PICTURE_FILE_EXTENSION_, 6);
			memset(szSignaturePath, 0x00, sizeof(szSignaturePath));
			sprintf(szSignaturePath, "./fs_data/%s", szSignature);
			/* 圖檔存在、有在signpad簽名、且非重印（重印不出簽名）（目前簽名狀態存不了Batch，先把&& pobTran->srBRec.inSignStatus == _SIGN_SIGNED_此條件拿掉） */
			if (inFILE_Check_Exist((unsigned char *)szSignature) == VS_SUCCESS && pobTran->inRunOperationID != _OPERATION_REPRINT_)
			{
				/* 電子簽名 */
				inPRINT_Buffer_PutGraphic((unsigned char *)szSignaturePath, uszBuffer, srBhandle, _SIGNEDPAD_WIDTH_, _APPEND_);
			}
			/* 手簽 */
			else
			{
				/* a space 2 line */
				for (i = 0; i < 2; i++)
				{
					inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}

			}	
                }
		else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
                {
			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}

				}
			}

			/* 簽名欄 */
			/* 藉由TRT_FileName比對來組出bmp的檔名 */
			inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

			memset(szSignature, 0x00, sizeof(szSignature));
			/* 因為用invoice所以不用inFunc_ComposeFileName */
			inFunc_ComposeFileName_InvoiceNumber(pobTran, szSignature, _PICTURE_FILE_EXTENSION_, 6);
			memset(szSignaturePath, 0x00, sizeof(szSignaturePath));
			sprintf(szSignaturePath, "./fs_data/%s", szSignature);
			/* 圖檔存在、有在signpad簽名、且非重印（重印不出簽名）（目前簽名狀態存不了Batch，先把&& pobTran->srBRec.inSignStatus == _SIGN_SIGNED_此條件拿掉） */
			if (inFILE_Check_Exist((unsigned char *)szSignature) == VS_SUCCESS && pobTran->inRunOperationID != _OPERATION_REPRINT_)
			{
				/* 電子簽名 */
				inPRINT_Buffer_PutGraphic((unsigned char *)szSignaturePath, uszBuffer, srBhandle, _SIGNEDPAD_WIDTH_, _APPEND_);
			}
			/* 手簽 */
			else
			{
				/* a space 2 line */
				for (i = 0; i < 2; i++)
				{
					inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}

			}
		}
                else
                {
			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}

				}
			}
                }
		
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
			{
				inPRINT_Buffer_PutIn("X:________________________________", _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("持卡人簽名(特店存根聯)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
				    pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
				{
					/* 列印持卡人姓名 */
					inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				/* Mail 主旨: RE: 【112184】V3標準版郵購交易及客製化041、043預借現金不印免簽名調整之需求已發信問過登霖，預借現金簽單是否比照簽單規格，將Cardholder’s Signature」移除 2024/1/9 下午 3:10 */
			}
			else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
			{
				inPRINT_Buffer_PutIn("X:________________________________", _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("持卡人簽名(特店存根聯)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("商店存根", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
				    pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
				{
					/* 列印持卡人姓名 */
					inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
			}
			else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				inPRINT_Buffer_PutIn("持卡人存根 Card holder stub", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
				    pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
				{
					inPRINT_Buffer_PutIn("------------------------------------------", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
					/* 列印持卡人姓名 */
					inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
			}
			
			if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
			{
				inPRINT_Buffer_PutIn("重印 REPRINT", _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			}

			/* 列印警示語 */
			inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

			/* 預借現金客製化專屬，於帳單最下方要印出備註標語 */
			if ((pobTran->inTransactionCode == _CASH_ADVANCE_ || pobTran->inTransactionCode == _FORCE_CASH_ADVANCE_ || pobTran->inRunOperationID == _OPERATION_REPRINT_) &&
			    pobTran->srBRec.inCode != _VOID_)
			{
				inDisclaimerFontSize = _PRT_HEIGHT_SMALL_;
				inPRINT_Buffer_PutIn("備註：依據發卡機構與持卡人之約定，發卡機構得向", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("持卡人收手續費", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("Note: By the agreement of issuing bank and", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("cardholder, the Issuing bank may charge", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("handling fee to the cardholder.", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			
			/* Print Notice */
			if (inCREDIT_PRINT_Notice(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
				return (VS_ERROR);

			/* Print Slogan */
			if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				if (inCREDIT_PRINT_MarchantSlogan(pobTran, _NCCC_SLOGAN_PRINT_DOWN_, uszBuffer, srBhandle) != VS_SUCCESS)
					return (VS_ERROR);
			}

			for (i = 0; i < 6; i++)
			{
				inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
		{
			if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
			    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
			{
				inPRINT_Buffer_PutIn("X:________________________________", _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("持卡人簽名(特店存根聯)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
				    pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
				{
					/* 列印持卡人姓名 */
					inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
			}
			else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				inPRINT_Buffer_PutIn("持卡人存根 Card holder stub", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
				    pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
				{
					inPRINT_Buffer_PutIn("------------------------------------------", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
					/* 列印持卡人姓名 */
					inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
			}
			
			if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
			{
				inPRINT_Buffer_PutIn("重印 REPRINT", _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			}

			/* 列印警示語 */
			inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

			/* 建設公司客製化專屬，帳單聲明書列印(START) */
			if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
			{
				inDisclaimerFontSize = _PRT_HEIGHT_SMALL_;
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("聲明書", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("本人瞭解以信用卡刷卡僅限於支付本項交易訂金金額，", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("此訂金係為向賣方取得優先購買指定房屋之權利而支付", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("，買、賣雙方於完成房屋契約簽約手續後，本人支付之", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("訂金即依約轉為房屋價款之一部分，本人支付訂金之目", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("的即已獲滿足，嗣後本人絕不再以賣方之服務未提供或", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("商品未交付為由，向信用卡發卡機構或收單機構要求扣", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("款，特立本聲明書為憑。", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				inPRINT_Buffer_PutIn("立聲明書人：＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("（請簽信用卡持卡人之本人中文姓名）", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("請款聯", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else if(pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
			{
				inDisclaimerFontSize = _PRT_HEIGHT_SMALL_;
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("聲明書", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("本人瞭解以信用卡刷卡僅限於支付本項交易訂金金額，", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("此訂金係為向賣方取得優先購買指定房屋之權利而支付", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("，買、賣雙方於完成房屋契約簽約手續後，本人支付之", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("訂金即依約轉為房屋價款之一部分，本人支付訂金之目", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("的即已獲滿足，嗣後本人絕不再以賣方之服務未提供或", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("商品未交付為由，向信用卡發卡機構或收單機構要求扣", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("款，特立本聲明書為憑。", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				inPRINT_Buffer_PutIn("立聲明書人：＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("（請簽信用卡持卡人之本人中文姓名）", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("特店存根聯", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else if(pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				inDisclaimerFontSize = _PRT_HEIGHT_SMALL_;
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("本人瞭解以信用卡刷卡僅限於支付本項交易訂金金額，", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("此訂金係為向賣方取得優先購買指定房屋之權利而支付", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("，買、賣雙方於完成房屋契約簽約手續後，本人支付之", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("訂金即依約轉為房屋價款之一部分，本人支付訂金之目", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("的即已獲滿足，嗣後本人絕不再以賣方之服務未提供或", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("商品未交付為由，向信用卡發卡機構或收單機構要求扣", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("款，特立本聲明書為憑。", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				inPRINT_Buffer_PutIn("持卡人存根聯", inDisclaimerFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			/* 建設公司客製化專屬，帳單聲明書列印(END) */
			
			/* Print Notice */
			if (inCREDIT_PRINT_Notice(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
				return (VS_ERROR);

			/* Print Slogan */
			if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				if (inCREDIT_PRINT_MarchantSlogan(pobTran, _NCCC_SLOGAN_PRINT_DOWN_, uszBuffer, srBhandle) != VS_SUCCESS)
					return (VS_ERROR);
			}

			for (i = 0; i < 6; i++)
			{
				inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}
		else
		{
			if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
			{
				inPRINT_Buffer_PutIn("持卡人簽名(特店存根聯)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
				    pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
				{
					/* 列印持卡人姓名 */
					inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
			}
			else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
			{
				inPRINT_Buffer_PutIn("商店存根", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
				    pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
				{
					/* 列印持卡人姓名 */
					inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
			}
			else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				inPRINT_Buffer_PutIn("持卡人存根 Card holder stub", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
				    pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
				{
					/* 列印持卡人姓名 */
					inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
			}
			
			if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
			{
				inPRINT_Buffer_PutIn("重印 REPRINT", _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			}

			/* 列印警示語 */
			inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

			/* Print Notice */
			if (inCREDIT_PRINT_Notice(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
				return (VS_ERROR);

			/* Print Slogan */
			if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				if (inCREDIT_PRINT_MarchantSlogan(pobTran, _NCCC_SLOGAN_PRINT_DOWN_, uszBuffer, srBhandle) != VS_SUCCESS)
					return (VS_ERROR);
			}

			for (i = 0; i < 6; i++)
			{
				inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}
		
        }

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Data_ByBuffer_Trust
Date&Time       :2025/7/4 下午 4:01
Describe        :列印DATA
*/
int inCREDIT_PRINT_Data_ByBuffer_Trust(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        int     inRetVal;
        char 	szPrintBuf[100 + 1], szPrintBuf1[42 + 1], szPrintBuf2[42 + 1], szTemplate1[42 + 1], szTemplate2[42 + 1];
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

        /* 列印受益人ID */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "受益人ID");
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
        if (inRetVal != VS_SUCCESS)
            return (VS_ERROR);
        
        /* 列印受益人ID值 */
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
        sprintf(szTemplate1, pobTran->srTrustRec.szMaskedBeneficiaryId);
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "%s", szTemplate1);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);

        /* 交易別 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
        sprintf(szPrintBuf1, "交易");
        sprintf(szPrintBuf, "%s", szPrintBuf1);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);
        
        /* 交易別值 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
        memset(szTemplate2, 0x00, sizeof(szTemplate2)); 
        inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
        sprintf(szPrintBuf, "%s %s", szTemplate1, szTemplate2);
        inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_011_);
        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);
        
        /* 主機 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
        sprintf(szPrintBuf1, "主機");
        sprintf(szPrintBuf, "%s", szPrintBuf1);
        inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_010_);
        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);
        
        /* 主機值 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
        inGetHostLabel(szTemplate1);
	inFunc_DiscardSpace(szTemplate1);
        sprintf(szPrintBuf, "%s", szTemplate1);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);

        /* 日期時間 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
        sprintf(szPrintBuf1, "日期/時間");
        sprintf(szPrintBuf, "%s", szPrintBuf1);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);
	
	/* 日期時間值 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
        memset(szTemplate2, 0x00, sizeof(szTemplate2));
        sprintf(szTemplate1, "%.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
        sprintf(szTemplate2, "%.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
        sprintf(szPrintBuf, "%s %s", szTemplate1, szTemplate2);
        inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_09_);
        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);

        /* 批號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
        sprintf(szPrintBuf1, "批號");
        sprintf(szPrintBuf, "%s", szPrintBuf1);
        inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_010_);
        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);
        
        /* 批號值 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
        sprintf(szTemplate1, "%03ld", pobTran->srBRec.lnBatchNum);
        sprintf(szPrintBuf, "%s", szTemplate1);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);
        
	/* 信託銀行代碼 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
        sprintf(szPrintBuf1, "信託銀行代碼");
        sprintf(szPrintBuf, "%s", szPrintBuf1);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);
	
	/* 信託銀行代碼值 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
        sprintf(szTemplate1, "%s", pobTran->srTrustRec.szTrustInstitutionCode);
        inFunc_DiscardSpace(szTemplate1);
        sprintf(szPrintBuf, "%s", szTemplate1);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);
	
        /* 平台序號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
        sprintf(szPrintBuf1, "平台序號");
	sprintf(szPrintBuf2, "%s", pobTran->srTrustRec.szExchangeTxSerialNumber);
        sprintf(szPrintBuf, "%s %s", szPrintBuf1, szPrintBuf2);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);
        
        /* 銷帳編號 */
        if (inFunc_CheckFullSpace(pobTran->srTrustRec.szReconciliationNo) != VS_TRUE)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
                memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                sprintf(szPrintBuf1, "銷帳編號");
                strcpy(szTemplate1, pobTran->srTrustRec.szReconciliationNo);
                inFunc_DiscardSpace(szTemplate1);
                sprintf(szPrintBuf2, "%s", szTemplate1);
                sprintf(szPrintBuf, "%s %s", szPrintBuf1, szPrintBuf2);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
        }
        
        /* 共用收單序號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
        sprintf(szPrintBuf1, "共用收單序號");
        sprintf(szPrintBuf, "%s", szPrintBuf1);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);
               
        /* 共用收單序號值 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
        sprintf(szPrintBuf1, pobTran->srTrustRec.szTrustRRN);
        sprintf(szPrintBuf, "%s", szPrintBuf1);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);
        
        /* 斷行 */
        inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_Amount_ByBuffer_Trust
Date&Time       :2025/9/9 下午 5:43
Describe        :列印AMOUNT
*/
int inCREDIT_PRINT_Amount_ByBuffer_Trust(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        /* 負向交易 */
	if (pobTran->srTrustRec.inCode == _TRUST_VOID_)
	{
		/* 初始化 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		sprintf(szTemplate, "%ld",  (0 - pobTran->srTrustRec.lnTxnAmount));
		inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", "金額(Total):");
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	else
	{
		/* 初始化 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szTemplate, "%ld",  pobTran->srTrustRec.lnTxnAmount);

		inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", "金額(Total):");
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	/* 斷行 */
	inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_ReceiptEND_ByBuffer_Trust
Date&Time       :2025/9/10 下午 5:26
Describe        :列印結尾
*/
int inCREDIT_PRINT_ReceiptEND_ByBuffer_Trust(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
	char	szDemoMode[2 + 1] = {0};
	
	/* 橫式 */
	if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
	{
		/* 教育訓練模式 */
		memset(szDemoMode, 0x00, sizeof(szDemoMode));
		inGetDemoMode(szDemoMode);
		if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
		{
			if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
				}

			}
		}

		if (pobTran->srTrustRec.inCode == _TRUST_VOID_)
		{
			inPRINT_Buffer_PutIn("取消使用信託帳戶給付本筆交易 (特店聯)", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		}
		else
		{
			inPRINT_Buffer_PutIn("同意使用信託帳戶給付本筆交易 (特店聯)", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		}
	}
	else
	{
		/* 教育訓練模式 */
		memset(szDemoMode, 0x00, sizeof(szDemoMode));
		inGetDemoMode(szDemoMode);
		if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
		{
			if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
				}

			}
		}

		if (pobTran->srTrustRec.inCode == _TRUST_VOID_)
		{
			inPRINT_Buffer_PutIn("取消使用信託帳戶給付本筆交易 (受益人聯)", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		}
		else
		{
			inPRINT_Buffer_PutIn("同意使用信託帳戶給付本筆交易 (受益人聯)", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		}
	}

	if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
	{
		inPRINT_Buffer_PutIn("重印", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	}

	for (i = 0; i < 8; i++)
	{
		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalReport_ByBuffer_TRUST
Date&Time       :2025/10/14 下午 4:25
Describe        :列印總額帳單
*/
int inCREDIT_PRINT_TotalReport_ByBuffer_TRUST(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = 0, inPrintIndex = 0;
	char			szDebugMsg[100 + 1] = {0};
	char			szCustomerIndicator[3 + 1] = {0};
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	/* (需求單 - 107227)邦柏科技自助作業客製化 不列印結帳條 2018/11/28 下午 3:03 by Russell */
	/* (需求單 - 107276)自助交易標準400做法 不列印結帳條 by Russell 2018/12/27 上午 11:20 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)   ||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
	{
		return (VS_SUCCESS);
	}
	else
	{
                inPrintIndex = _TOTAL_REPORT_INDEX_TRUST_;

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "PrintIndex : %d", inPrintIndex);
			inLogPrintf(AT, szDebugMsg);
		}

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

		while (1)
		{
			inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
			/* 列印LOGO */
			if (srTotalReport_ByBuffer_TRUST[inPrintIndex].inReportLogo != NULL)
				if ((inRetVal = srTotalReport_ByBuffer_TRUST[inPrintIndex].inReportLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 列印TID MID */
			if (srTotalReport_ByBuffer_TRUST[inPrintIndex].inReportTop != NULL)
				if ((inRetVal = srTotalReport_ByBuffer_TRUST[inPrintIndex].inReportTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 全部金額總計 */
			if (srTotalReport_ByBuffer_TRUST[inPrintIndex].inAmount != NULL)
				if ((inRetVal = srTotalReport_ByBuffer_TRUST[inPrintIndex].inAmount(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
                        
                        /* 機構金額總計 */
                        if (srTotalReport_ByBuffer_TRUST[inPrintIndex].inAmountByCard != NULL)
                                if ((inRetVal = srTotalReport_ByBuffer_TRUST[inPrintIndex].inAmountByCard(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
                                        return (inRetVal);
			/* 結束 */
			if (srTotalReport_ByBuffer_TRUST[inPrintIndex].inReportEnd != NULL)
				if ((inRetVal = srTotalReport_ByBuffer_TRUST[inPrintIndex].inReportEnd(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);
			break;
		}

		return (VS_SUCCESS);
	}
}

/*
Function        :inCREDIT_PRINT_Top_ByBuffer_TRUST
Date&Time       :2025/10/14 下午 4:42
Describe        :
*/
int inCREDIT_PRINT_Top_ByBuffer_TRUST(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal = VS_SUCCESS;
        char    szPrintBuf[84 + 1] = {0}, szPrintBuf1[84 + 1] = {0};
        char    szTemplate[42 + 1] = {0}, szTemplate1[42 + 1] = {0}, szTemplate2[42 + 1] = {0};

        /* Get商店代號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetMerchantID(szTemplate);

        /* 列印商店代號 */
        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_LEFT_);
        sprintf(szPrintBuf, "商店代號%s", szTemplate);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        /* Get端末機代號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetTerminalID(szTemplate);

        /* 列印端末機代號 */
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PADDING_LEFT_);
        sprintf(szPrintBuf, "端末機代號%s", szTemplate);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        inRetVal = inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        /* 日期時間 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
        sprintf(szPrintBuf1, "日期/時間");
        sprintf(szPrintBuf, "%s", szPrintBuf1);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);
	
	/* 日期時間值 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
        memset(szTemplate2, 0x00, sizeof(szTemplate2));
        sprintf(szTemplate1, "%.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
        sprintf(szTemplate2, "%.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
        sprintf(szPrintBuf, "%s %s", szTemplate1, szTemplate2);
        inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_09_);
        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);
	
        /* 交易類別 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
        sprintf(szPrintBuf1, "交易類別");
        sprintf(szPrintBuf, "%s", szPrintBuf1);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);
        
        /* 交易別值 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "結帳");
        inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_012_);
        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);

        /* 主機 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
        sprintf(szPrintBuf1, "主機");
        sprintf(szPrintBuf, "%s", szPrintBuf1);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);
        
        /* 主機值 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
        inGetHostLabel(szTemplate1);
	inFunc_DiscardSpace(szTemplate1);
        sprintf(szPrintBuf, "%s", szTemplate1);
        inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_011_);
        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);
        
        /* 批號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
        sprintf(szPrintBuf1, "批號");
        sprintf(szPrintBuf, "%s", szPrintBuf1);
        inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_010_);
        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);
        
        /* 批號值 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
        sprintf(szTemplate1, "%03ld", pobTran->srBRec.lnBatchNum);
        sprintf(szPrintBuf, "%s", szTemplate1);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmount_ByBuffer_TRUST
Date&Time       :2025/10/15 上午 10:07
Describe        :列印總金額
*/
int inCREDIT_PRINT_TotalAmount_ByBuffer_TRUST(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        int                 inRetVal = VS_SUCCESS;
        int                 inRecCnt = 0;
        char                szPrintBuf[84 + 1] = {0}, szTemplate[84 + 1] = {0};
        char                szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
        char                szSql[300 + 1] = {0};
        char                szCntTag[20 + 1] = {0};
        char                szAmtTag[20 + 1] = {0};
        char                szDebugMsg[200 + 1] = {0};
        long                lnCnt = 0;
        long long           llAmt = 0;
        SQLITE_ALL_TABLE    srAll = {};
        
        /* Table Name */
        memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
        
        sprintf(szCntTag, "TrustAllCnt");
        sprintf(szAmtTag, "TrustAllAmt");
        
        inRetVal = inSqlite_Table_Link_Addition_Int(&srAll, szCntTag, &lnCnt);
        if (inRetVal != VS_SUCCESS)
        {
                vdUtility_SYSFIN_LogMessage(AT, "Table Link Fail");
                if (ginDebug == VS_TRUE)
                {
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                        sprintf(szDebugMsg, "Table Link 失敗 %s", szCntTag);
                        inLogPrintf(AT, szDebugMsg);
                }

                return (VS_ERROR);
        }
        
        inRetVal = inSqlite_Table_Link_Addition_Int64(&srAll, szAmtTag, &llAmt);
        if (inRetVal != VS_SUCCESS)
        {
                vdUtility_SYSFIN_LogMessage(AT, "Table Link Fail");
                if (ginDebug == VS_TRUE)
                {
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                        sprintf(szDebugMsg, "Table Link 失敗 %s", szAmtTag);
                        inLogPrintf(AT, szDebugMsg);
                }

                return (VS_ERROR);
        }
        
        do
        {
                /* 預設為無須重找 */
                guszEnormousNoNeedResetBit = VS_TRUE;
                memset(szSql, 0x00, sizeof(szSql));
                snprintf(szSql, sizeof(szSql), "SELECT "
                                               "SUM(CASE WHEN incode = %d THEN 1 WHEN incode = %d THEN -1 ELSE 0 END) AS %s,"
                                               "SUM(CASE WHEN incode = %d THEN lnTxnAmount WHEN incode = %d THEN -lnTxnAmount ELSE 0 END) AS %s "
                                               "FROM %s", _TRUST_SALE_, _TRUST_VOID_, szCntTag, _TRUST_SALE_, _TRUST_VOID_, szAmtTag, szTableName);
                inRetVal = inSqlite_Get_Data_Enormous_Search(gszTranDBPath, szSql);
                if (inRetVal != VS_SUCCESS)
                {
                        if (inRetVal == VS_NO_RECORD)
                        {
                                vdUtility_SYSFIN_LogMessage(AT, "No Table");
                                if (ginDebug == VS_TRUE)
                                {
                                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                        sprintf(szDebugMsg, "No Table");
                                        inLogPrintf(AT, szDebugMsg);
                                }
                        }
                        else
                        {
                                vdUtility_SYSFIN_LogMessage(AT, "Table Search Fail");
                                if (ginDebug == VS_TRUE)
                                {
                                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                        sprintf(szDebugMsg, "Table Search Fail");
                                        inLogPrintf(AT, szDebugMsg);
                                }
                        }
                        break;
                }

                inRecCnt = 0;
                inRetVal = inSqlite_Get_Data_Enormous_Get(&srAll, inRecCnt);
                if (inRetVal != VS_SUCCESS)
                {
                        vdUtility_SYSFIN_LogMessage(AT, "Table Get Fail");
                        if (ginDebug == VS_TRUE)
                        {
                                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                sprintf(szDebugMsg, "Table Get Fail");
                                inLogPrintf(AT, szDebugMsg);
                        }
                        break;
                }
                break;
        }while(1);
        
        inSqlite_Get_Data_Enormous_Free();
        

        inPRINT_Buffer_PutIn("結帳報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
        inPRINT_Buffer_PutIn_Specific_X_Position("筆數(CNT)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_013_);
        inPRINT_Buffer_PutIn("金額(AMOUNT)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);

        /* 信託結帳 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "%s", "信託結帳");
        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
        
        /* 筆數 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        if (lnCnt < 0)
	{
	    sprintf(szTemplate, "-%03ld", labs(lnCnt));
	}
	else
	{
	    sprintf(szTemplate, "%03ld", lnCnt);
	}
        inFunc_Amount_Comma(szTemplate, "" , '\x00', _SIGNED_NONE_, 13, _PADDING_RIGHT_);
        sprintf(szPrintBuf, "%s", szTemplate);
        inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_013_);
        
        /* 金額 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szTemplate, "%lld", llAmt);
        inFunc_Amount_Comma(szTemplate, "" , '\x00', _SIGNED_NONE_, 13, _PADDING_RIGHT_);
        sprintf(szPrintBuf, "NT$ %s", szTemplate);
        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
        
        inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmountByInstitutionCode_ByBuffer
Date&Time       :2025/10/15 下午 2:35
Describe        :依機構列印
*/
int inCREDIT_PRINT_TotalAmountByInstitutionCode_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int			inRetVal = VS_SUCCESS;
        int			inRecCnt = 0;
        char			szPrintBuf[84 + 1] = {0}, szTemplate[84 + 1] = {0};
        char			szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
        char			szSql[300 + 1] = {0};
        char			szCntTag[20 + 1] = {0};
        char			szAmtTag[20 + 1] = {0};
        char			szTrustInstitutionCodeTag[100 + 1] = {0};
        char			szDebugMsg[200 + 1] = {0};
        long			lnCnt = 0;
        long long		llAmt = 0;
        SQLITE_ALL_TABLE	srAll = {};
        
        /* Table Name */
        memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
        
        sprintf(szCntTag, "TrustCnt");
        sprintf(szAmtTag, "TrustAmt");
        sprintf(szTrustInstitutionCodeTag, "szTrustInstitutionCode");
        
        /* 預設為無須重找 */
        guszEnormousNoNeedResetBit = VS_TRUE;
        memset(szSql, 0x00, sizeof(szSql));
        snprintf(szSql, sizeof(szSql), "SELECT "
                                       "%s,"
                                       "SUM(CASE WHEN incode = %d THEN 1 WHEN incode = %d THEN -1 ELSE 0 END) AS %s,"
                                       "SUM(CASE WHEN incode = %d THEN lnTxnAmount WHEN incode = %d THEN -lnTxnAmount ELSE 0 END) AS %s "
                                       "FROM %s GROUP BY %s", szTrustInstitutionCodeTag, _TRUST_SALE_, _TRUST_VOID_, szCntTag, _TRUST_SALE_, _TRUST_VOID_, szAmtTag, szTableName, szTrustInstitutionCodeTag);
        inRetVal = inSqlite_Get_Data_Enormous_Search(gszTranDBPath, szSql);
        if (inRetVal != VS_SUCCESS)
        {
                if (inRetVal == VS_NO_RECORD)
                {
                        vdUtility_SYSFIN_LogMessage(AT, "No Table");
                        if (ginDebug == VS_TRUE)
                        {
                                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                sprintf(szDebugMsg, "No Table");
                                inLogPrintf(AT, szDebugMsg);
                        }
                }
                else
                {
                        vdUtility_SYSFIN_LogMessage(AT, "Table Search Fail");
                        if (ginDebug == VS_TRUE)
                        {
                                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                sprintf(szDebugMsg, "Table Search Fail");
                                inLogPrintf(AT, szDebugMsg);
                        }
                }
        }
        
        inRetVal = inNCCC_Trust_Table_Link_TrustRec(pobTran, &srAll, _LS_READ_);
        if (inRetVal != VS_SUCCESS)
        {
                vdUtility_SYSFIN_LogMessage(AT, "Table Link Fail");
                if (ginDebug == VS_TRUE)
                {
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                        sprintf(szDebugMsg, "Table Link 失敗 %s", szCntTag);
                        inLogPrintf(AT, szDebugMsg);
                }

                return (VS_ERROR);
        }

        inRetVal = inSqlite_Table_Link_Addition_Int(&srAll, szCntTag, &lnCnt);
        if (inRetVal != VS_SUCCESS)
        {
                vdUtility_SYSFIN_LogMessage(AT, "Table Link Fail");
                if (ginDebug == VS_TRUE)
                {
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                        sprintf(szDebugMsg, "Table Link 失敗 %s", szCntTag);
                        inLogPrintf(AT, szDebugMsg);
                }

                return (VS_ERROR);
        }

        inRetVal = inSqlite_Table_Link_Addition_Int64(&srAll, szAmtTag, &llAmt);
        if (inRetVal != VS_SUCCESS)
        {
                vdUtility_SYSFIN_LogMessage(AT, "Table Link Fail");
                if (ginDebug == VS_TRUE)
                {
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                        sprintf(szDebugMsg, "Table Link 失敗 %s", szAmtTag);
                        inLogPrintf(AT, szDebugMsg);
                }

                return (VS_ERROR);
        }
        
        if (inRetVal != VS_NO_RECORD)
        {
                /* 信託銀行小計 */
                inPRINT_Buffer_PutIn("信託銀行小計", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
                inPRINT_Buffer_PutIn("信託銀行代碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn_Specific_X_Position("筆數", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_014_);
                inPRINT_Buffer_PutIn("金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
        
                inRecCnt = 0;
                do
                {
                        inSqlite_Reset_Find_State(&srAll);
                        inRetVal = inSqlite_Get_Data_Enormous_Get(&srAll, inRecCnt);
                        if (inRetVal != VS_SUCCESS)
                        {
                                if (inRetVal == VS_NO_RECORD)
                                {
                                        
                                }
                                else
                                {
                                        vdUtility_SYSFIN_LogMessage(AT, "Table Get Fail");
                                        if (ginDebug == VS_TRUE)
                                        {
                                                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                                sprintf(szDebugMsg, "Table Get Fail");
                                                inLogPrintf(AT, szDebugMsg);
                                        }
                                }
                                break;
                        }
                        else
                        {
                                /* 機構代碼 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "%s", pobTran->srTrustRec.szTrustInstitutionCode);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

                                /* 筆數 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                if (lnCnt < 0)
				{
				    sprintf(szTemplate, "-%03ld", labs(lnCnt));
				}
				else
				{
				    sprintf(szTemplate, "%03ld", lnCnt);
				}
                                inFunc_Amount_Comma(szTemplate, "" , '\x00', _SIGNED_NONE_, 13, _PADDING_RIGHT_);
                                sprintf(szPrintBuf, "%s", szTemplate);
                                inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_014_);

                                /* 金額 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szTemplate, "%lld", llAmt);
                                inFunc_Amount_Comma(szTemplate, "" , '\x00', _SIGNED_NONE_, 13, _PADDING_RIGHT_);
                                sprintf(szPrintBuf, "NT$ %s", szTemplate);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
        
                               inRecCnt++; 
                        }
                }while(1);
        
                inSqlite_Get_Data_Enormous_Free();
        }
        else
        {
                inSqlite_Get_Data_Enormous_Free();
        }

        inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_End_ByBuffer_TRUST
Date&Time       :2025/10/15 下午 4:49
Describe        :列印結尾
*/
int inCREDIT_PRINT_End_ByBuffer_TRUST(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
	
        inPRINT_Buffer_PutIn("*** 結帳完成 ***", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

        for (i = 0; i < 8; i++)
	{
		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

        return (VS_SUCCESS);
}