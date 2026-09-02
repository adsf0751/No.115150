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

/*  昇恆昌客製化075 */
/* 列印重印報表使用 (START) */
DETAIL_REPORT_DUTYFREE_REPRINT_TABLE_BYBUFFER srReprintDetail[] =
{
	{
		inCREDIT_PRINT_Dutyfree_Reprint_Check_ByBuffer,
		inCREDIT_PRINT_Dutyfree_Reprint_Logo_ByBuffer,
		inCREDIT_PRINT_Dutyfree_Reprint_Top_ByBuffer,
		_NULL_CH_,
		inCREDIT_PRINT_Dutyfree_Reprint_NCCC_DetailReportMiddle_ByBuffer,
		inCREDIT_PRINT_Dutyfree_Reprint_DetailReportBottom_ByBuffer,
		inCREDIT_PRINT_Dutyfree_Reprint_End_ByBuffer
	},
	
};
/* 列印重印報表使用 (END) */

/*
Function        :inCREDIT_PRINT_TotalAmount_ByBuffer_098_ESVC
Date&Time       :2018/1/29 下午 3:44
Describe        :列印總金額
*/
int inCREDIT_PRINT_TotalAmount_ByBuffer_098_ESVC(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szPrintBuf[84 + 1], szTemplate[84 + 1];

        inPRINT_Buffer_PutIn("總額報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
        
        inPRINT_Buffer_PutIn("       筆數           金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	/* 銷售 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "購貨 　　%03lu   NT$", srAccumRec->lnDeductTotalCount);
	sprintf(szTemplate, "%lld", srAccumRec->llDeductTotalAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 退貨 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnRefundTotalCount);
	sprintf(szTemplate, "%lld", srAccumRec->llRefundTotalAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        
        inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 交易淨值 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szPrintBuf, "交易淨值 %03lu   NT$", srAccumRec->lnDeductTotalCount + srAccumRec->lnRefundTotalCount);
        
        if (srAccumRec->llDeductTotalAmount - srAccumRec->llRefundTotalAmount < 0L)
        {
                sprintf(szTemplate, "%lld", srAccumRec->llRefundTotalAmount - srAccumRec->llDeductTotalAmount);
                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_MINUS_, 13, VS_TRUE);
        }
        else
        {
                sprintf(szTemplate, "%lld", srAccumRec->llDeductTotalAmount - srAccumRec->llRefundTotalAmount);
                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
        }
        
        strcat(szPrintBuf, szTemplate);
        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 現金加值 */
	if (srAccumRec->lnADDTotalCount > 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "現金加值 %03lu   NT$", srAccumRec->lnADDTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llADDTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	/* 加值取消 */
	if (srAccumRec->lnVoidADDTotalCount > 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "加值取消 %03lu   NT$", srAccumRec->lnVoidADDTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llVoidADDTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
		
	/* 加值淨額 */
	if (srAccumRec->lnADDTotalCount > 0L || srAccumRec->lnVoidADDTotalCount > 0L)
	{
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "加值淨額 %03lu   NT$", (srAccumRec->lnADDTotalCount + srAccumRec->lnVoidADDTotalCount));
		sprintf(szTemplate, "%lld", (srAccumRec->llADDTotalAmount - srAccumRec->llVoidADDTotalAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmount_ByBuffer_098_ESVC
Date&Time       :2018/1/29 下午 3:44
Describe        :列印總金額
*/
int inCREDIT_PRINT_TotalAmount_ByBuffer_098_ESVC_Settle(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char		szPrintBuf[84 + 1] = {0}, szTemplate[84 + 1] = {0};

        inPRINT_Buffer_PutIn("結帳報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
        
        inPRINT_Buffer_PutIn("       筆數           金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	/* 銷售 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "購貨 　　%03lu   NT$", srAccumRec->lnDeductTotalCount);
	sprintf(szTemplate, "%lld", srAccumRec->llDeductTotalAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 退貨 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnRefundTotalCount);
	sprintf(szTemplate, "%lld", srAccumRec->llRefundTotalAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
        inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 交易淨值 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szPrintBuf, "交易淨值 %03lu   NT$", srAccumRec->lnDeductTotalCount + srAccumRec->lnRefundTotalCount);
        
        if (srAccumRec->llDeductTotalAmount - srAccumRec->llRefundTotalAmount < 0L)
        {
                sprintf(szTemplate, "%lld", srAccumRec->llRefundTotalAmount - srAccumRec->llDeductTotalAmount);
                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_MINUS_, 13, VS_TRUE);
        }
        else
        {
                sprintf(szTemplate, "%lld", srAccumRec->llDeductTotalAmount - srAccumRec->llRefundTotalAmount);
                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
        }
        
        strcat(szPrintBuf, szTemplate);
        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        
	/* 現金加值 */
	if (srAccumRec->lnADDTotalCount > 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "現金加值 %03lu   NT$", srAccumRec->lnADDTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llADDTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	/* 加值取消 */
	if (srAccumRec->lnVoidADDTotalCount > 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "加值取消 %03lu   NT$", srAccumRec->lnVoidADDTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llVoidADDTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	/* 加值淨額 */
	if (srAccumRec->lnADDTotalCount > 0L || srAccumRec->lnVoidADDTotalCount > 0L)
	{
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "加值淨額 %03lu   NT$", (srAccumRec->lnADDTotalCount + srAccumRec->lnVoidADDTotalCount));
		sprintf(szTemplate, "%lld", (srAccumRec->llADDTotalAmount - srAccumRec->llVoidADDTotalAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmountByCard_ByBuffer_098_ESVC
Date&Time       :2018/1/29 下午 4:19
Describe        :依卡別列印
*/
int inCREDIT_PRINT_TotalAmountByCard_ByBuffer_098_ESVC(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int		i = 0;
	char		szPrintBuf[100 + 1] = {0}, szTemplate[42 + 1] = {0};
	char		szTxnType[20 + 1] = {0};
	char		szTicketNeedNewBatch[2 + 1] = {0};
	unsigned char	uszNeedPrintBit = VS_FALSE;	
	
	/* 先檢查是否任一票證有開 */
	for (i = 0; i < 4; i++)
	{
		if (inLoadTDTRec(i) != VS_SUCCESS)
		{
			break;
		}
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTicket_HostEnable(szTemplate);
                if (memcmp(szTemplate, "Y", strlen("Y")) != 0)
		{
                        continue;
		}
		else
		{
			uszNeedPrintBit = VS_TRUE;
		}
	}
	
	/* 代表有任一票證要印 */
	if (uszNeedPrintBit == VS_TRUE)
	{
		inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("卡別小計", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	}
	else
	{
		return (VS_SUCCESS);
	}
	
	/* 個別票證 */
        for (i = 0; i < 4; i++)
        {
                if (inLoadTDTRec(i) != VS_SUCCESS)
		{
			break;
		}
		
                memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTicket_HostEnable(szTemplate);
                if (memcmp(szTemplate, "Y", strlen("Y")) != 0)
		{
                        continue;
		}
                
                memset(szTxnType, 0x00, sizeof(szTxnType));
		inGetTicket_HostTransFunc(szTxnType);
		
                switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :
        	                inPRINT_Buffer_PutIn("卡別 一卡通", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
				
				/* 特店代號 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "特店代號%s", "");
				inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetIPASS_System_ID(szTemplate);
				memcpy(&szPrintBuf[0], szTemplate, 2);

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetIPASS_SP_ID(szTemplate);
				memcpy(&szPrintBuf[2], szTemplate, 2);

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetIPASS_Sub_Company_ID(szTemplate);
				memcpy(&szPrintBuf[4], szTemplate, 4); 
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
        	                break;
        	        case _TDT_INDEX_01_ECC_ :
        	                inPRINT_Buffer_PutIn("卡別 悠遊卡", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
          	                
				/* 特店代號 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "特店代號%s", "");
				inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetECC_New_SP_ID(szTemplate);
				strcpy(&szPrintBuf[0], szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				
				/* 二代設備編號 */
          	                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetTicket_Device2(szTemplate);
                        	sprintf(szPrintBuf, "二代設備編號　 %s", szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                        	
				/* 悠遊卡批次號碼 */
                        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTicketNeedNewBatch, 0x00, sizeof(szTicketNeedNewBatch));
				inGetTicket_NeedNewBatch(szTicketNeedNewBatch);
				if (memcmp(szTicketNeedNewBatch, "Y", strlen("Y")) == 0)
				{
					sprintf(szTemplate, "%.4s%.2s%.2s", &pobTran->srBRec.szDate[2], &pobTran->srBRec.szDate[6], _ECC_DEFAULT_BATCH_);
				}
				else
				{
					inGetTicket_Batch(szTemplate);
				}
                        	sprintf(szPrintBuf, "悠遊卡批次號碼 %s", szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
        	        case _TDT_INDEX_02_ICASH_ :
        	                inPRINT_Buffer_PutIn("卡別 愛金卡", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
				
				/* 特店代號 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "門市代碼%s", "");
				inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetICASH_Shop_ID(szTemplate);
				memcpy(&szPrintBuf[0], szTemplate, 8);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
        	                break;
        	        default :
        	                break;       
        	}

		/* 不合法 跳出 */
                if (i != _TDT_INDEX_00_IPASS_	&&
		    i != _TDT_INDEX_01_ECC_	&&
		    i != _TDT_INDEX_02_ICASH_)
		{
                        break;
		}
                
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :   
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", srAccumRec->lnIPASS_DeductTotalCount);                   
        	                sprintf(szTemplate, "%lld", srAccumRec->llIPASS_DeductTotalAmount);
        	                break;
        	        case _TDT_INDEX_01_ECC_ :   
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", srAccumRec->lnEASYCARD_DeductTotalCount);                   
        	                sprintf(szTemplate, "%lld", srAccumRec->llEASYCARD_DeductTotalAmount);
        	                break;
        	        case _TDT_INDEX_02_ICASH_ :   
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", srAccumRec->lnICASH_DeductTotalCount);                   
        	                sprintf(szTemplate, "%lld", srAccumRec->llICASH_DeductTotalAmount);
        	                break;
        	        default :
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", 0l);                   
        	                sprintf(szTemplate, "%lld", 0ll);
        	                break;        
        	}
        	
        	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
        	strcat(szPrintBuf, szTemplate);
        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	
        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", srAccumRec->lnIPASS_RefundTotalCount);                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_RefundTotalAmount));
        	                break;
        	        case _TDT_INDEX_01_ECC_ :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", srAccumRec->lnEASYCARD_RefundTotalCount);                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_RefundTotalAmount));
        	                break;
        	        case _TDT_INDEX_02_ICASH_ :   
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", srAccumRec->lnICASH_RefundTotalCount);                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llICASH_RefundTotalAmount));
        	                break;
        	        default :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", 0l);                   
        	                sprintf(szTemplate, "%lld", 0ll);
        	                break;        
        	}
		
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
        	strcat(szPrintBuf, szTemplate);
        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                
                inPRINT_Buffer_PutIn("------------------------------------------", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :
                                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", srAccumRec->lnIPASS_DeductTotalCount + srAccumRec->lnIPASS_RefundTotalCount); 
                            
                                if (srAccumRec->llIPASS_DeductTotalAmount - srAccumRec->llIPASS_RefundTotalAmount < 0L)
                                {
                                        sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_RefundTotalAmount - srAccumRec->llIPASS_DeductTotalAmount));
                                        inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_MINUS_, 13, _PADDING_LEFT_);
                                }
                                else
                                {
                                        sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_DeductTotalAmount - srAccumRec->llIPASS_RefundTotalAmount));
                                        inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
                                }
                                
        	                break;
        	        case _TDT_INDEX_01_ECC_ :
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", srAccumRec->lnEASYCARD_DeductTotalCount + srAccumRec->lnEASYCARD_RefundTotalCount);      
                                
                                if (srAccumRec->llEASYCARD_DeductTotalAmount - srAccumRec->llEASYCARD_RefundTotalAmount < 0L)
                                {
                                        sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_RefundTotalAmount - srAccumRec->llEASYCARD_DeductTotalAmount));
                                        inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_MINUS_, 13, _PADDING_LEFT_);
                                }
                                else
                                {
                                        sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_DeductTotalAmount - srAccumRec->llEASYCARD_RefundTotalAmount));
                                        inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
                                }
                                
        	                break;
        	        case _TDT_INDEX_02_ICASH_ :   
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", srAccumRec->lnICASH_DeductTotalCount + srAccumRec->lnICASH_RefundTotalCount);      
                                
                                if (srAccumRec->llICASH_DeductTotalAmount - srAccumRec->llICASH_RefundTotalAmount < 0L)
                                {
                                        sprintf(szTemplate, "%lld", (srAccumRec->llICASH_RefundTotalAmount - srAccumRec->llICASH_DeductTotalAmount));
                                        inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_MINUS_, 13, _PADDING_LEFT_);
                                }
                                else
                                {
                                        sprintf(szTemplate, "%lld", (srAccumRec->llICASH_DeductTotalAmount - srAccumRec->llICASH_RefundTotalAmount));
                                        inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
                                }
                                
        	                break;
        	        default :
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", 0l);                   
        	                sprintf(szTemplate, "%lld", 0ll);
        	                break;        
        	}
		
        	strcat(szPrintBuf, szTemplate);
        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                
                inPRINT_Buffer_PutIn("------------------------------------------", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	
		/* 加值功能有開且有交易筆數 */
        	if (szTxnType[4] == 0x59 && 
		   ((i == _TDT_INDEX_00_IPASS_ && srAccumRec->lnIPASS_ADDTotalCount > 0)	||
		    (i == _TDT_INDEX_01_ECC_ && srAccumRec->lnEASYCARD_ADDTotalCount > 0)	||
		    (i == _TDT_INDEX_02_ICASH_ && srAccumRec->lnICASH_ADDTotalCount > 0)))
        	{
        	        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	
                	switch(i)
                        {
                                case _TDT_INDEX_00_IPASS_ :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", srAccumRec->lnIPASS_ADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_ADDTotalAmount));
                	                break;
                	        case _TDT_INDEX_01_ECC_ :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", srAccumRec->lnEASYCARD_ADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_ADDTotalAmount));
                	                break;
                	        case _TDT_INDEX_02_ICASH_ :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", srAccumRec->lnICASH_ADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llICASH_ADDTotalAmount));
                	                break;
                	        default :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", 0l);                   
                	                sprintf(szTemplate, "%lld", 0ll);
                	                break;        
                	}
                	
                	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	}
        	
		/* 加值取消功能有開且有交易筆數 */
        	if (szTxnType[5] == 0x59	&& 
		   ((i == _TDT_INDEX_00_IPASS_ && srAccumRec->lnIPASS_VoidADDTotalCount > 0)	||
		    (i == _TDT_INDEX_01_ECC_ && srAccumRec->lnEASYCARD_VoidADDTotalCount > 0)   ||
		    (i == _TDT_INDEX_02_ICASH_ && srAccumRec->lnICASH_VoidADDTotalCount > 0)))
        	{
                	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                	switch(i)
                        {
                                case _TDT_INDEX_00_IPASS_ :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", srAccumRec->lnIPASS_VoidADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_VoidADDTotalAmount));
                	                break;
                	        case _TDT_INDEX_01_ECC_ :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", srAccumRec->lnEASYCARD_VoidADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_VoidADDTotalAmount));
                	                break;
                	        case _TDT_INDEX_02_ICASH_ :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", srAccumRec->lnICASH_VoidADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llICASH_VoidADDTotalAmount));
                	                break;
                	        default :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", 0l);                   
                	                sprintf(szTemplate, "%lld", 0ll);
                	                break;        
                	}
                	
                	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                }
                	
		if ((i == _TDT_INDEX_00_IPASS_ && (srAccumRec->lnIPASS_ADDTotalCount > 0 || srAccumRec->lnIPASS_VoidADDTotalCount > 0))	||
		    (i == _TDT_INDEX_01_ECC_ && (srAccumRec->lnEASYCARD_ADDTotalCount > 0 || srAccumRec->lnEASYCARD_VoidADDTotalCount > 0))	||
		    (i == _TDT_INDEX_02_ICASH_ && (srAccumRec->lnICASH_ADDTotalCount > 0 || srAccumRec->lnICASH_VoidADDTotalCount > 0)))
		{
                        inPRINT_Buffer_PutIn("------------------------------------------", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                    
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			switch(i)
			{
				case _TDT_INDEX_00_IPASS_ :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", (srAccumRec->lnIPASS_ADDTotalCount + srAccumRec->lnIPASS_VoidADDTotalCount));                   
					sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_ADDTotalAmount - srAccumRec->llIPASS_VoidADDTotalAmount));
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;
				case _TDT_INDEX_01_ECC_ :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", (srAccumRec->lnEASYCARD_ADDTotalCount + srAccumRec->lnEASYCARD_VoidADDTotalCount));                   
					sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_ADDTotalAmount - srAccumRec->llEASYCARD_VoidADDTotalAmount));
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;
				case _TDT_INDEX_02_ICASH_ :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", (srAccumRec->lnICASH_ADDTotalCount + srAccumRec->lnICASH_VoidADDTotalCount));                   
					sprintf(szTemplate, "%lld", (srAccumRec->llICASH_ADDTotalAmount - srAccumRec->llICASH_VoidADDTotalAmount));
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;
				default :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", 0l);                   
					sprintf(szTemplate, "%lld", 0ll);
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;        
			}
		}
        	
        	inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        }
        
        inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmountByCard_ByBuffer_098_ESVC_Settle
Date&Time       :2018/1/30 下午 2:32
Describe        :依卡別列印
*/
int inCREDIT_PRINT_TotalAmountByCard_ByBuffer_098_ESVC_Settle(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int		i = 0;
	char		szPrintBuf[100 + 1], szTemplate[42 + 1];
	char		szTxnType[20 + 1];
	char		szTicketNeedNewBatch[2 + 1] = {0};
	char		szSpecialShopID[50 + 1] = {0};
	unsigned char	uszNeedPrintBit = VS_FALSE;
	
	/* 先檢查是否任一票證有開 */
	for (i = 0; i < 4; i++)
	{
		if (inLoadTDTRec(i) != VS_SUCCESS)
		{
			break;
		}
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTicket_HostEnable(szTemplate);
                if (memcmp(szTemplate, "Y", strlen("Y")) != 0)
		{
                        continue;
		}
		else
		{
			uszNeedPrintBit = VS_TRUE;
		}
	}
	
	/* 代表有任一票證要印 */
	if (uszNeedPrintBit == VS_TRUE)
	{
		inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("卡別小計", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	}
	else
	{
		return (VS_SUCCESS);
	}
	
        for (i = 0; i < 4; i++)
        {
                if (inLoadTDTRec(i) != VS_SUCCESS)
		{
			break;
		}
		
                memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTicket_HostEnable(szTemplate);
                if (memcmp(szTemplate, "Y", strlen("Y")) != 0)
		{
                        continue;
		}
                
                memset(szTxnType, 0x00, sizeof(szTxnType));
		inGetTicket_HostTransFunc(szTxnType);
		        
                switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :
        	                inPRINT_Buffer_PutIn("卡別 一卡通", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
				
				memset(szSpecialShopID, 0x00, sizeof(szSpecialShopID));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetIPASS_System_ID(szTemplate);
				memcpy(&szSpecialShopID[0], szTemplate, 2);

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetIPASS_SP_ID(szTemplate);
				memcpy(&szSpecialShopID[2], szTemplate, 2);

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetIPASS_Sub_Company_ID(szTemplate);
				memcpy(&szSpecialShopID[4], szTemplate, 4);
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "特店代號%s", szSpecialShopID);
				inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_DEFINE_X_01_);
				
        	                break;
        	        case _TDT_INDEX_01_ECC_ :
        	                inPRINT_Buffer_PutIn("卡別 悠遊卡", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
          	                
				memset(szSpecialShopID, 0x00, sizeof(szSpecialShopID));
				inGetECC_New_SP_ID(szSpecialShopID);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "特店代號%s", szSpecialShopID);
				inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_DEFINE_X_01_);
				
				/* 二代設備編號 */
          	                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetTicket_Device2(szTemplate);
                        	sprintf(szPrintBuf, "二代設備編號　 %s", szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                        	
				/* 悠遊卡批次號碼 */
                        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTicketNeedNewBatch, 0x00, sizeof(szTicketNeedNewBatch));
				inGetTicket_NeedNewBatch(szTicketNeedNewBatch);
				if (memcmp(szTicketNeedNewBatch, "Y", strlen("Y")) == 0)
				{
					sprintf(szTemplate, "%.4s%.2s%.2s", &pobTran->srBRec.szDate[2], &pobTran->srBRec.szDate[6], _ECC_DEFAULT_BATCH_);
				}
				else
				{
					inGetTicket_Batch(szTemplate);
				}
                        	sprintf(szPrintBuf, "悠遊卡批次號碼 %s", szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
				/* 列印交易時間 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szPrintBuf, "交易時間　　　 %.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        	                break;
        	        case _TDT_INDEX_02_ICASH_ :
        	                inPRINT_Buffer_PutIn("卡別 愛金卡", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
				
				/* 特店代號 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "門市代碼%s", "");
				inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetICASH_Shop_ID(szTemplate);
				memcpy(&szPrintBuf[0], szTemplate, 8);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
        	                break; 
        	        default :
        	                break;       
        	}

		/* 不合法 跳出 */
                if (i != _TDT_INDEX_00_IPASS_	&&
		    i != _TDT_INDEX_01_ECC_	&&
		    i != _TDT_INDEX_02_ICASH_)
		{
                        break;
		}
                
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :   
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", srAccumRec->lnIPASS_DeductTotalCount);                   
        	                sprintf(szTemplate, "%lld", srAccumRec->llIPASS_DeductTotalAmount);
        	                break;
        	        case _TDT_INDEX_01_ECC_ :   
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", srAccumRec->lnEASYCARD_DeductTotalCount);                   
        	                sprintf(szTemplate, "%lld", srAccumRec->llEASYCARD_DeductTotalAmount);
        	                break;
        	        case _TDT_INDEX_02_ICASH_ :   
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", srAccumRec->lnICASH_DeductTotalCount);                   
        	                sprintf(szTemplate, "%lld", srAccumRec->llICASH_DeductTotalAmount);
        	                break;
        	        default :
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", 0l);                   
        	                sprintf(szTemplate, "%lld", 0ll);
        	                break;        
        	}
        	
        	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
        	strcat(szPrintBuf, szTemplate);
        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	
        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", srAccumRec->lnIPASS_RefundTotalCount);                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_RefundTotalAmount));
        	                break;
        	        case _TDT_INDEX_01_ECC_ :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", srAccumRec->lnEASYCARD_RefundTotalCount);                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_RefundTotalAmount));
        	                break;
                        case _TDT_INDEX_02_ICASH_ :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", srAccumRec->lnICASH_RefundTotalCount);                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llICASH_RefundTotalAmount));
        	                break;
        	        default :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", 0l);                   
        	                sprintf(szTemplate, "%lld", 0ll);
        	                break;        
        	}
		
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
        	strcat(szPrintBuf, szTemplate);
        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                
                inPRINT_Buffer_PutIn("------------------------------------------", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :
                                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", srAccumRec->lnIPASS_DeductTotalCount + srAccumRec->lnIPASS_RefundTotalCount); 
                            
                                if (srAccumRec->llIPASS_DeductTotalAmount - srAccumRec->llIPASS_RefundTotalAmount < 0L)
                                {
                                        sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_RefundTotalAmount - srAccumRec->llIPASS_DeductTotalAmount));
                                        inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_MINUS_, 13, _PADDING_LEFT_);
                                }
                                else
                                {
                                        sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_DeductTotalAmount - srAccumRec->llIPASS_RefundTotalAmount));
                                        inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
                                }
                                
        	                break;
        	        case _TDT_INDEX_01_ECC_ :
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", srAccumRec->lnEASYCARD_DeductTotalCount + srAccumRec->lnEASYCARD_RefundTotalCount);      
                                
                                if (srAccumRec->llEASYCARD_DeductTotalAmount - srAccumRec->llEASYCARD_RefundTotalAmount < 0L)
                                {
                                        sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_RefundTotalAmount - srAccumRec->llEASYCARD_DeductTotalAmount));
                                        inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_MINUS_, 13, _PADDING_LEFT_);
                                }
                                else
                                {
                                        sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_DeductTotalAmount - srAccumRec->llEASYCARD_RefundTotalAmount));
                                        inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
                                }
                                
        	                break;
        	        case _TDT_INDEX_02_ICASH_ :   
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", srAccumRec->lnICASH_DeductTotalCount + srAccumRec->lnICASH_RefundTotalCount);      
                                
                                if (srAccumRec->llICASH_DeductTotalAmount - srAccumRec->llICASH_RefundTotalAmount < 0L)
                                {
                                        sprintf(szTemplate, "%lld", (srAccumRec->llICASH_RefundTotalAmount - srAccumRec->llICASH_DeductTotalAmount));
                                        inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_MINUS_, 13, _PADDING_LEFT_);
                                }
                                else
                                {
                                        sprintf(szTemplate, "%lld", (srAccumRec->llICASH_DeductTotalAmount - srAccumRec->llICASH_RefundTotalAmount));
                                        inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
                                }
                                
        	                break;
        	        default :
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", 0l);                   
        	                sprintf(szTemplate, "%lld", 0ll);
        	                break;        
        	}
		
        	strcat(szPrintBuf, szTemplate);
        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                
                inPRINT_Buffer_PutIn("------------------------------------------", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	
		/* 加值功能有開且有交易筆數 */
        	if (szTxnType[4] == 0x59  && 
		   ((i == _TDT_INDEX_00_IPASS_ && srAccumRec->lnIPASS_ADDTotalCount > 0)	||
		    (i == _TDT_INDEX_01_ECC_ && srAccumRec->lnEASYCARD_ADDTotalCount > 0)	||
		    (i == _TDT_INDEX_02_ICASH_ && srAccumRec->lnICASH_ADDTotalCount > 0)))
        	{
        	        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	
                	switch(i)
                        {
                                case _TDT_INDEX_00_IPASS_ :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", srAccumRec->lnIPASS_ADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_ADDTotalAmount));
                	                break;
                	        case _TDT_INDEX_01_ECC_ :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", srAccumRec->lnEASYCARD_ADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_ADDTotalAmount));
                	                break;
                                case _TDT_INDEX_02_ICASH_ :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", srAccumRec->lnICASH_ADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llICASH_ADDTotalAmount));
                	                break;				
                	        default :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", 0l);                   
                	                sprintf(szTemplate, "%lld", 0ll);
                	                break;        
                	}
                	
                	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	}
        	
		/* 加值取消功能有開且有交易筆數 */
        	if (szTxnType[5] == 0x59  && 
		   ((i == _TDT_INDEX_00_IPASS_ && srAccumRec->lnIPASS_VoidADDTotalCount > 0)	||
		    (i == _TDT_INDEX_01_ECC_ && srAccumRec->lnEASYCARD_VoidADDTotalCount > 0)	||
		    (i == _TDT_INDEX_02_ICASH_ && srAccumRec->lnICASH_VoidADDTotalCount > 0)))
        	{
                	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                	switch(i)
                        {
                                case _TDT_INDEX_00_IPASS_ :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", srAccumRec->lnIPASS_VoidADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_VoidADDTotalAmount));
                	                break;
                	        case _TDT_INDEX_01_ECC_ :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", srAccumRec->lnEASYCARD_VoidADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_VoidADDTotalAmount));
                	                break;
                                case _TDT_INDEX_02_ICASH_ :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", srAccumRec->lnICASH_VoidADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llICASH_VoidADDTotalAmount));
                	                break;
                	        default :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", 0l);                   
                	                sprintf(szTemplate, "%lld", 0ll);
                	                break;        
                	}
                	
                	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                }

                if ((i == _TDT_INDEX_00_IPASS_ && (srAccumRec->lnIPASS_ADDTotalCount > 0 || srAccumRec->lnIPASS_VoidADDTotalCount > 0))	||
		    (i == _TDT_INDEX_01_ECC_ && (srAccumRec->lnEASYCARD_ADDTotalCount > 0 || srAccumRec->lnEASYCARD_VoidADDTotalCount > 0))	||
		    (i == _TDT_INDEX_02_ICASH_ && (srAccumRec->lnICASH_ADDTotalCount > 0 || srAccumRec->lnICASH_VoidADDTotalCount > 0)))
		{
                        inPRINT_Buffer_PutIn("------------------------------------------", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			switch(i)
			{
				case _TDT_INDEX_00_IPASS_ :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", (srAccumRec->lnIPASS_ADDTotalCount + srAccumRec->lnIPASS_VoidADDTotalCount));                   
					sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_ADDTotalAmount - srAccumRec->llIPASS_VoidADDTotalAmount));
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;
				case _TDT_INDEX_01_ECC_ :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", (srAccumRec->lnEASYCARD_ADDTotalCount + srAccumRec->lnEASYCARD_VoidADDTotalCount));                   
					sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_ADDTotalAmount - srAccumRec->llEASYCARD_VoidADDTotalAmount));
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;
				case _TDT_INDEX_02_ICASH_ :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", (srAccumRec->lnICASH_ADDTotalCount + srAccumRec->lnICASH_VoidADDTotalCount));                   
					sprintf(szTemplate, "%lld", (srAccumRec->llICASH_ADDTotalAmount - srAccumRec->llICASH_VoidADDTotalAmount));
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;
				default :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", 0l);                   
					sprintf(szTemplate, "%lld", 0ll);
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;        
			}
		}
		
        	inPRINT_Buffer_PutIn(" ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        }
        
        inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Top_ByBuffer_046
Date&Time       :2022/4/18 下午 5:12
Describe        :
*/
int inCREDIT_PRINT_Top_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];
	
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);

        /* Get商店代號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetMerchantID(szTemplate);

        /* 列印商店代號 */
        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_LEFT_);
        sprintf(szPrintBuf, "商店代號%s", szTemplate);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        /* Get端末機代號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetTerminalID(szTemplate);

        /* 列印端末機代號 */
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PADDING_LEFT_);
        sprintf(szPrintBuf, "端末機代號%s", szTemplate);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        inRetVal = inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        /* 列印日期 / 時間 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inRetVal = inPRINT_Buffer_PutIn("日期/時間(Date/Time)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	if (pobTran->uszEverRich_NoDataBit == VS_TRUE)
	{
		sprintf(szPrintBuf, "%s", " ");
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	else
	{
		sprintf(szPrintBuf, "%.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	
        /* 列印交易類別 */
	/* 結帳時預先列印總額和明細(pobTran->uszPrePrintBit == VS_TRUE)，不印交易類別*/
        if (pobTran->srBRec.inCode == _SETTLE_)
        {
		if (pobTran->uszPrePrintBit != VS_TRUE)
		{
			inRetVal = inPRINT_Buffer_PutIn("交易類別(Trans. Type)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "60 結帳 SETTLEMENT");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			if (pobTran->uszEverRich_Settle_RepeintBit == VS_TRUE)
			{
				inRetVal = inPRINT_Buffer_PutIn("(重印)", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
		}
        }

        /* 列印批次號碼 */
        inRetVal = inPRINT_Buffer_PutIn("批次號碼(Batch No.)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	
	if (pobTran->uszEverRich_NoDataBit == VS_TRUE)
	{
		sprintf(szTemplate, "%03d", 0);
		strcpy(szPrintBuf, szTemplate);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	else
	{
		sprintf(szTemplate, "%03ld", pobTran->srBRec.lnBatchNum);
		strcpy(szPrintBuf, szTemplate);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
        /* 列印主機 */
        inRetVal = inPRINT_Buffer_PutIn("主機(Host)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        inGetHostLabel(szPrintBuf);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_DetailReportMiddle_ByBuffer_046
Date&Time       :2022/4/19 下午 4:11
Describe        :
*/
int inCREDIT_PRINT_DetailReportMiddle_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szFuncEnable[1 + 1];			/* catch Y or N */

	inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);
	
        inPRINT_Buffer_PutIn("明細報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

	inPRINT_Buffer_PutIn("調閱編號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn_Specific_X_Position("交易金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

	inPRINT_Buffer_PutIn("交易類別", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("卡別", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("卡號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	inPRINT_Buffer_PutIn("交易日期", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn_Specific_X_Position("交易時間", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

	/* SmartPay要印調單編號 */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetFiscFuncEnable(szFuncEnable);
	if (szFuncEnable[0] == 'Y')
	{
		inPRINT_Buffer_PutIn("授權碼/調單編號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	}
	else
	{
		inPRINT_Buffer_PutIn("授權碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	}
	inPRINT_Buffer_PutIn_Specific_X_Position("檢查碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

	/* 櫃號功能有開才印櫃號 */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetStoreIDEnable(szFuncEnable);
	if (szFuncEnable[0] == 'Y')
		inPRINT_Buffer_PutIn("櫃號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_NCCC_DetailReportMiddle_ByBuffer_046
Date&Time       :2022/4/18 下午 6:19
Describe        :
*/
int inCREDIT_PRINT_NCCC_DetailReportMiddle_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szFuncEnable[1 + 1];			/* catch Y or N */
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
	inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);
	
	/* 商店自存聯卡號遮掩 */
	if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0)
	{
		inPRINT_Buffer_PutIn("明細報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

		inPRINT_Buffer_PutIn("調閱編號", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn_Specific_X_Position("交易金額", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		inPRINT_Buffer_PutIn("交易類別", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		/* 客製化47不印卡別 */
		if (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_) != 0)
		{
			inPRINT_Buffer_PutIn("卡別", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		inPRINT_Buffer_PutIn("卡號", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("交易編號", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		inPRINT_Buffer_PutIn("交易日期", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn_Specific_X_Position("交易時間", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		
		/* SmartPay要印調單編號 */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetFiscFuncEnable(szFuncEnable);
		if (szFuncEnable[0] == 'Y')
		{
			inPRINT_Buffer_PutIn("授權碼/調單編號", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		}
		else
		{
			inPRINT_Buffer_PutIn("授權碼", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		}
		inPRINT_Buffer_PutIn_Specific_X_Position("檢查碼", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		
		/* 銀聯功能有開才印回覆碼 */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetCUPFuncEnable(szFuncEnable);
		if (szFuncEnable[0] == 'Y')
		{
			inPRINT_Buffer_PutIn("回覆碼", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		/* 櫃號功能有開才印櫃號 */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetStoreIDEnable(szFuncEnable);
		if (szFuncEnable[0] == 'Y')
			inPRINT_Buffer_PutIn("櫃號", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutIn("==========================================", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	else
	{
		inPRINT_Buffer_PutIn("明細報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

		inPRINT_Buffer_PutIn("調閱編號", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn_Specific_X_Position("交易金額", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		inPRINT_Buffer_PutIn("交易類別", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		/* 客製化47不印卡別 */
		if (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_) != 0)
		{
			inPRINT_Buffer_PutIn("卡別", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		inPRINT_Buffer_PutIn("卡號", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		inPRINT_Buffer_PutIn("交易日期", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn_Specific_X_Position("交易時間", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		
		/* SmartPay要印調單編號 */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetFiscFuncEnable(szFuncEnable);
		if (szFuncEnable[0] == 'Y')
		{
			inPRINT_Buffer_PutIn("授權碼/調單編號", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		}
		else
		{
			inPRINT_Buffer_PutIn("授權碼", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		}
		inPRINT_Buffer_PutIn_Specific_X_Position("檢查碼", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		
		/* 銀聯功能有開才印回覆碼 */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetCUPFuncEnable(szFuncEnable);
		if (szFuncEnable[0] == 'Y')
		{
			inPRINT_Buffer_PutIn("回覆碼", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		/* 櫃號功能有開才印櫃號 */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetStoreIDEnable(szFuncEnable);
		if (szFuncEnable[0] == 'Y')
			inPRINT_Buffer_PutIn("櫃號", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutIn("==========================================", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	}
	
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
	
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_DCC_DetailReportMiddle_ByBuffer_046
Date&Time       :2022/4/18 下午 6:20
Describe        :
*/
int inCREDIT_PRINT_DCC_DetailReportMiddle_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szFuncEnable[1 + 1];			/* catch Y or N */
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
	inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);
	
	/* 商店自存聯卡號遮掩 */
	if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0)
	{
		inPRINT_Buffer_PutIn("明細報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

		inPRINT_Buffer_PutIn("調閱編號", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn_Specific_X_Position("交易金額", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		inPRINT_Buffer_PutIn("交易類別", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		/* 客製化47不印卡別 */
		if (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_) != 0)
		{
			inPRINT_Buffer_PutIn("卡別", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		inPRINT_Buffer_PutIn("卡號", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("交易編號", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		inPRINT_Buffer_PutIn("交易日期", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn_Specific_X_Position("交易時間", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		
		inPRINT_Buffer_PutIn("授權碼", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn_Specific_X_Position("檢查碼", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		/* 櫃號功能有開才印櫃號 */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetStoreIDEnable(szFuncEnable);
		if (szFuncEnable[0] == 'Y')
			inPRINT_Buffer_PutIn("櫃號", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutIn("==========================================", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	else
	{
		inPRINT_Buffer_PutIn("明細報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

		inPRINT_Buffer_PutIn("調閱編號", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn_Specific_X_Position("交易金額", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		inPRINT_Buffer_PutIn("交易類別", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		/* 客製化47不印卡別 */
		if (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_) != 0)
		{
			inPRINT_Buffer_PutIn("卡別", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		inPRINT_Buffer_PutIn("卡號", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		inPRINT_Buffer_PutIn("交易日期", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn_Specific_X_Position("交易時間", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		
		inPRINT_Buffer_PutIn("授權碼", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn_Specific_X_Position("檢查碼", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		
		/* 櫃號功能有開才印櫃號 */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetStoreIDEnable(szFuncEnable);
		if (szFuncEnable[0] == 'Y')
			inPRINT_Buffer_PutIn("櫃號", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutIn("==========================================", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	}
	
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
	
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_HG_DetailReportMiddle_ByBuffer_046
Date&Time       :2022/4/19 上午 9:54
Describe        :
*/
int inCREDIT_PRINT_HG_DetailReportMiddle_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szFuncEnable[1 + 1];			/* catch Y or N */
	
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);

        inPRINT_Buffer_PutIn("明細報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

	inPRINT_Buffer_PutIn("調閱編號", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn_Specific_X_Position("交易金額", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
	
        inPRINT_Buffer_PutIn("交易類別", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
        inPRINT_Buffer_PutIn("卡號", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	inPRINT_Buffer_PutIn("交易日期", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn_Specific_X_Position("交易時間", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
	
	inPRINT_Buffer_PutIn("授權碼", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	/* 櫃號功能有開才印櫃號 */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
        inGetStoreIDEnable(szFuncEnable);
        if (szFuncEnable[0] == 'Y')
                inPRINT_Buffer_PutIn("櫃號", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        inPRINT_Buffer_PutIn("==========================================", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
	
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_DetailReportBottom_ByBuffer_046
Date&Time       :2022/4/19 上午 10:19
Describe        :
*/
int inCREDIT_PRINT_DetailReportBottom_ByBuffer_046(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i, j;
	int	inReadCnt = 0;
	int	inCardLen = 0;
	int	inRetVal = VS_SUCCESS;
        char	szPrintBuf[62 + 1], szTemplate1[62 + 1], szTemplate2[62 + 1];
        char	szFuncEnable[1 + 1];			/* catch Y or N */
        char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
        char	szCustomerIndicator[3 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inCREDIT_PRINT_DetailReportBottom_ByBuffer_046()_START");
	}
	
	memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
	inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);
	
	/* 開始讀取 */
	inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(pobTran);
	/* 預設為無須重找 */
	guszEnormousNoNeedResetBit = VS_TRUE;

        for (inReadCnt = 0; inReadCnt < inRecordCnt; inReadCnt ++)
        {
                /*. 開始讀取每一筆交易記錄 .*/
                if (inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(pobTran, inReadCnt) != VS_SUCCESS)
                {
                        inRetVal = VS_ERROR;
                        break;
                }
		
		/* 昇恆昌沒有優惠兌換 */
		/* 商店自存聯卡號遮掩 */
		if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0)
		{
			/* Invoice Number */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			if (pobTran->srBRec.uszVOIDBit != VS_TRUE)
			{
				sprintf(szTemplate1, "INV:%06ld", pobTran->srBRec.lnOrgInvNum);
			}
			else
			{
				sprintf(szTemplate1, "*INV:%06ld", pobTran->srBRec.lnOrgInvNum);
			}

			/* 【需求單 - 105259】總額明細報表及總額明細查詢補強機制 商店聯出紙本要印<P> */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetESCMode(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0 && pobTran->srBRec.inESCUploadStatus == _ESC_UPLOAD_STATUS_PAPER_)
			{
				strcat(szTemplate1, " <P>");
			}

			strcat(szPrintBuf, szTemplate1);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* Print Amount */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));

			if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
			{
				switch (pobTran->srBRec.inCode)
				{
					case _SALE_:
					case _INST_SALE_ :
					case _REDEEM_SALE_ :
					case _MAIL_ORDER_ :
					case _CUP_MAIL_ORDER_ :
					case _SALE_OFFLINE_ :
					case _PRE_COMP_ :
					case _PRE_AUTH_ :
					case _CUP_SALE_ :
					case _CUP_PRE_COMP_ :
					case _CUP_PRE_AUTH_ :
					case _FISC_SALE_ :
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						break;
					case _TIP_ :
						sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
						break;
					case _REFUND_ :
					case _INST_REFUND_ :
					case _REDEEM_REFUND_ :
					case _CUP_REFUND_ :
					case _CUP_MAIL_ORDER_REFUND_ :
					case _FISC_REFUND_ :
						sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
						break;
					case _INST_ADJUST_ :
					case _REDEEM_ADJUST_ :
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						break;
					case _ADJUST_ :
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
						break;
					default :
						memset(szTemplate2, 0x00, sizeof(szTemplate2));
						inGetHostLabel(szTemplate2);
						sprintf(szTemplate1,"%s_AMT_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			else
			{
				switch (pobTran->srBRec.inOrgCode)
				{
					 case _SALE_:
					case _INST_SALE_ :
					case _REDEEM_SALE_ :
					case _MAIL_ORDER_ :
					case _CUP_MAIL_ORDER_ :
					case _SALE_OFFLINE_ :
					case _PRE_COMP_ :
					case _PRE_AUTH_ :
					case _CUP_SALE_ :
					case _CUP_PRE_COMP_ :
					case _CUP_PRE_AUTH_ :
					case _FISC_SALE_ :
						if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
						{
							sprintf(szTemplate1, "%ld", 0 - (pobTran->srBRec.lnCUPUPlanDiscountedAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount));
						}
						else
						{
							sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
						}
						break;
					/* NCCC小費不能取消 */
					case _REFUND_ :
					case _INST_REFUND_ :
					case _REDEEM_REFUND_ :
					case _CUP_REFUND_ :
					case _CUP_MAIL_ORDER_REFUND_ :
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						break;
					case _INST_ADJUST_ :
					case _REDEEM_ADJUST_ :
						sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
						break;
					case _ADJUST_ :
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						break;
					default :
						memset(szTemplate2, 0x00, sizeof(szTemplate2));
						inGetHostLabel(szTemplate2);
						sprintf(szTemplate1, "%s_AMT_VOID_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			inFunc_Amount_Comma(szTemplate1, "NT$" , 0x00, _SIGNED_NONE_, 16, _PADDING_RIGHT_);
			strcat(szPrintBuf, szTemplate1);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* Trans Type */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
			sprintf(szPrintBuf, "%s %s", szTemplate1, szTemplate2);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;
			
			/* 客製化47不印卡別 */
			if (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_) != 0)
			{
				/* 卡別 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 12, _PADDING_RIGHT_);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
			}
			
			/* 卡號 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				strcpy(szTemplate1, pobTran->srBRec.szPAN);
				if (pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
				{
					inCardLen = strlen(szTemplate1);

					for (j = 6; j < (inCardLen - 4); j ++)
						szTemplate1[j] = 0x2A;
				}
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				if (inRetVal != VS_SUCCESS)
					break;
			}
			else
			{
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				strcpy(szTemplate1, pobTran->srBRec.szPAN);

				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				/* 卡號遮掩(一般卡號前6後4，U Card前3後5) */
				for (i = 6; i < (strlen(szTemplate1) - 4); i ++)
				{
					szTemplate1[i] = 0x2A;
				}

				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 19, _PADDING_RIGHT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
			}

			/* 交易序號Transaction No. */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcpy(szPrintBuf, "TXN. NO: ");
			if (pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
				strcat(szPrintBuf, pobTran->srBRec.szTxnNo);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* Trans Date Time */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "DATE: %.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "TIME: %.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;
			
			/* Approved No. & RRN NO. */
			/* SmartPay印調單編號 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcpy(szPrintBuf, "RRN NO.:");
				strcat(szPrintBuf, pobTran->srBRec.szFiscRRN);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
			}
			else
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcpy(szPrintBuf, "APPR: ");
				strcat(szPrintBuf, pobTran->srBRec.szAuthCode);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
			}
			
			/* 檢查碼Check No */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				/* SmartPay不印檢查碼 */
			}
			else
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				if (strlen(pobTran->srBRec.szCheckNO) > 0)
				{
					strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
				}
				else
				{
					inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
				}
				sprintf(szPrintBuf, "No.: %s",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
				if (inRetVal != VS_SUCCESS)
					break;
			}
			
			/* RESPONSE CODE */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetCUPFuncEnable(szFuncEnable);
			if (szFuncEnable[0] == 'Y')
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcpy(szPrintBuf, "RESPONSE CODE: ");
				strcat(szPrintBuf, pobTran->srBRec.szRespCode);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

				if (inRetVal != VS_SUCCESS)
					break;
			}

			/* Store ID */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetStoreIDEnable(szFuncEnable);
			if (szFuncEnable[0] == 'Y')
			{
				/*開啟櫃號功能*/
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "STORE ID: %s", pobTran->srBRec.szStoreID);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				if (inRetVal != VS_SUCCESS)
					break;
			}
			else
			{
			       /*沒開啟櫃號功能，則不印櫃號*/
			}
		}
		else
		{
			/* Invoice Number */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			if (pobTran->srBRec.uszVOIDBit != VS_TRUE)
			{
				sprintf(szTemplate1, "INV:%06ld", pobTran->srBRec.lnOrgInvNum);
			}
			else
			{
				sprintf(szTemplate1, "*INV:%06ld", pobTran->srBRec.lnOrgInvNum);
			}

			/* 【需求單 - 105259】總額明細報表及總額明細查詢補強機制 商店聯出紙本要印<P> */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetESCMode(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0 && pobTran->srBRec.inESCUploadStatus == _ESC_UPLOAD_STATUS_PAPER_)
			{
				strcat(szTemplate1, " <P>");
			}

			strcat(szPrintBuf, szTemplate1);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				break;

			/* Print Amount */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));

			if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
			{
				switch (pobTran->srBRec.inCode)
				{
					case _SALE_:
					case _INST_SALE_ :
					case _REDEEM_SALE_ :
					case _MAIL_ORDER_ :
					case _CUP_MAIL_ORDER_ :
					case _SALE_OFFLINE_ :
					case _PRE_COMP_ :
					case _PRE_AUTH_ :
					case _CUP_SALE_ :
					case _CUP_PRE_COMP_ :
					case _CUP_PRE_AUTH_ :
					case _FISC_SALE_ :
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						break;
					case _TIP_ :
						sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
						break;
					case _REFUND_ :
					case _INST_REFUND_ :
					case _REDEEM_REFUND_ :
					case _CUP_REFUND_ :
					case _CUP_MAIL_ORDER_REFUND_ :
					case _FISC_REFUND_ :
						sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
						break;
					case _INST_ADJUST_ :
					case _REDEEM_ADJUST_ :
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						break;
					case _ADJUST_ :
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
						break;
					default :
						memset(szTemplate2, 0x00, sizeof(szTemplate2));
						inGetHostLabel(szTemplate2);
						sprintf(szTemplate1,"%s_AMT_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			else
			{
				switch (pobTran->srBRec.inOrgCode)
				{
					 case _SALE_:
					case _INST_SALE_ :
					case _REDEEM_SALE_ :
					case _MAIL_ORDER_ :
					case _CUP_MAIL_ORDER_ :
					case _SALE_OFFLINE_ :
					case _PRE_COMP_ :
					case _PRE_AUTH_ :
					case _CUP_SALE_ :
					case _CUP_PRE_COMP_ :
					case _CUP_PRE_AUTH_ :
					case _FISC_SALE_ :
						if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
						{
							sprintf(szTemplate1, "%ld", 0 - (pobTran->srBRec.lnCUPUPlanDiscountedAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount));
						}
						else
						{
							sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
						}
						break;
					/* NCCC小費不能取消 */
					case _REFUND_ :
					case _INST_REFUND_ :
					case _REDEEM_REFUND_ :
					case _CUP_REFUND_ :
					case _CUP_MAIL_ORDER_REFUND_ :
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						break;
					case _INST_ADJUST_ :
					case _REDEEM_ADJUST_ :
						sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
						break;
					case _ADJUST_ :
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						break;
					default :
						memset(szTemplate2, 0x00, sizeof(szTemplate2));
						inGetHostLabel(szTemplate2);
						sprintf(szTemplate1, "%s_AMT_VOID_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			inFunc_Amount_Comma(szTemplate1, "NT$" , 0x00, _SIGNED_NONE_, 16, _PADDING_RIGHT_);
			strcat(szPrintBuf, szTemplate1);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

			if (inRetVal != VS_SUCCESS)
				break;

			/* Trans Type */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
			sprintf(szPrintBuf, "%s %s", szTemplate1, szTemplate2);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* 客製化47不印卡別 */
			if (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_) != 0)
			{
				/* 卡別 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 12, _PADDING_RIGHT_);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
			}
			
			/* 卡號 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				/* SmartPay不印檢查碼 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcpy(szPrintBuf, pobTran->srBRec.szPAN);

				inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 25, _PADDING_RIGHT_);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
			}
			else
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcpy(szPrintBuf, pobTran->srBRec.szPAN);

				inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 19, _PADDING_RIGHT_);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
			}

			/* Trans Date Time */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "DATE: %.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "TIME: %.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* Approved No. & RRN NO. */
			/* SmartPay印調單編號 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcpy(szPrintBuf, "RRN NO.:");
				strcat(szPrintBuf, pobTran->srBRec.szFiscRRN);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
			}
			else
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcpy(szPrintBuf, "APPR: ");
				strcat(szPrintBuf, pobTran->srBRec.szAuthCode);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
			}
			
			/* 檢查碼Check No */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				/* SmartPay不印檢查碼 */
			}
			else
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				if (strlen(pobTran->srBRec.szCheckNO) > 0)
				{
					strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
				}
				else
				{
					inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
				}
				sprintf(szPrintBuf, "No.: %s",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
				if (inRetVal != VS_SUCCESS)
					break;
			}
			
			/* RESPONSE CODE */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetCUPFuncEnable(szFuncEnable);
			if (szFuncEnable[0] == 'Y')
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcpy(szPrintBuf, "RESPONSE CODE: ");
				strcat(szPrintBuf, pobTran->srBRec.szRespCode);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
			}

			/* Store ID */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetStoreIDEnable(szFuncEnable);
			if (szFuncEnable[0] == 'Y')
			{
				/*開啟櫃號功能*/
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "STORE ID: %s", pobTran->srBRec.szStoreID);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
			}
			else
			{
			       /*沒開啟櫃號功能，則不印櫃號*/
			}
		}/* 商店聯卡號遮掩 END */
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	} /* End for () .... */
	
	/* 結束讀取 */
	inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(pobTran);

	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inCREDIT_PRINT_DetailReportBottom_ByBuffer_046()_END");
	}
	
        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_DCC_DetailReportBottom_ByBuffer_046
Date&Time       :2022/4/19 上午 10:20
Describe        :
*/
int inCREDIT_PRINT_DCC_DetailReportBottom_ByBuffer_046(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
	int	inReadCnt = 0;
	int	inRetVal = VS_SUCCESS;
        char	szPrintBuf[62 + 1], szTemplate1[62 + 1], szTemplate2[62 + 1];
        char	szFuncEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];/* catch Y or N */
        char	szCustomerIndicator[3 + 1] = {0};
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inCREDIT_PRINT_DCC_DetailReportBottom_ByBuffer_046()_START");
	}
	
	memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
	inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);
	
	/* 開始讀取 */
	inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(pobTran);
	/* 預設為無須重找 */
	guszEnormousNoNeedResetBit = VS_TRUE;
	
        for (inReadCnt = 0; inReadCnt < inRecordCnt; inReadCnt ++)
        {
                /*. 開始讀取每一筆交易記錄 .*/
                if (inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(pobTran, inReadCnt) != VS_SUCCESS)
                {
                        inRetVal = VS_ERROR;
                        break;
                }
		
		/* 2012-11-07 AM 10:09:10 add by kakab 新增DCC一段式交易(當筆轉台幣，已取消的紀錄不能印明細) */
		if (pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)
	    		continue;

		/* 商店自存聯卡號遮掩 */
		if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0)
		{
			/* Invoice Number */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			if (pobTran->srBRec.uszVOIDBit != VS_TRUE)
			{
				sprintf(szTemplate1, "INV:%06ld", pobTran->srBRec.lnOrgInvNum);
			}
			else
			{
				sprintf(szTemplate1, "*INV:%06ld", pobTran->srBRec.lnOrgInvNum);
			}
			
			/* 【需求單 - 105259】總額明細報表及總額明細查詢補強機制 商店聯出紙本要印<P> */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetESCMode(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0 && pobTran->srBRec.inESCUploadStatus == _ESC_UPLOAD_STATUS_PAPER_)
			{
				strcat(szTemplate1, " <P>");
			}
				
			strcat(szPrintBuf, szTemplate1);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				break;

			/* Print Amount */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));

			if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
			{
				switch (pobTran->srBRec.inCode)
				{
					case _SALE_:
					case _SALE_OFFLINE_ :
					case _PRE_COMP_ :
					case _PRE_AUTH_ :
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						break;
					case _TIP_ :
						sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
						break;
					case _REFUND_ :
						sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
						break;
					default :
						memset(szTemplate2, 0x00, sizeof(szTemplate2));
						inGetHostLabel(szTemplate2);
						sprintf(szTemplate1,"%s_AMT_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			else
			{
				switch (pobTran->srBRec.inOrgCode)
				{
					case _SALE_:
					case _SALE_OFFLINE_ :
					case _PRE_COMP_ :
					case _PRE_AUTH_ :
						sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
						break;
					/* NCCC小費不能取消 */
					case _REFUND_ :
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						break;
					default :
						memset(szTemplate2, 0x00, sizeof(szTemplate2));
						inGetHostLabel(szTemplate2);
						sprintf(szTemplate1, "%s_AMT_VOID_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			inFunc_Amount_Comma(szTemplate1, "NT$" , 0x00, _SIGNED_NONE_, 16, _PADDING_RIGHT_);
			strcat(szPrintBuf, szTemplate1);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* Trans Type */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
			sprintf(szPrintBuf, "%s %s", szTemplate1, szTemplate2);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* 客製化47不印卡別 */
			if (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_) != 0)
			{
				/* 卡別 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 12, _PADDING_RIGHT_);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
			}
			
			/* 卡號 */
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			strcpy(szTemplate1, pobTran->srBRec.szPAN);

			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			/* 卡號遮掩(一般卡號前6後4，U Card前3後5) */
			for (i = 6; i < (strlen(szTemplate1) - 4); i ++)
				szTemplate1[i] = 0x2A;

			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 19, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s",szTemplate1);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* 交易序號Transaction No. */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcpy(szPrintBuf, "TXN. NO: ");
			if (pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
				strcat(szPrintBuf, pobTran->srBRec.szTxnNo);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* Trans Date Time */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "DATE: %.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "TIME: %.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;
			
			/* Approved No. & RRN NO. */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcpy(szPrintBuf, "APPR: ");
			strcat(szPrintBuf, pobTran->srBRec.szAuthCode);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;
			
			/* 檢查碼Check No */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			if (strlen(pobTran->srBRec.szCheckNO) > 0)
			{
				strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
			}
			else
			{
				inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
			}
			sprintf(szPrintBuf, "No.: %s",szTemplate1);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* Store ID */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetStoreIDEnable(szFuncEnable);
			if (szFuncEnable[0] == 'Y')
			{
				/*開啟櫃號功能*/
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "STORE ID: %s", pobTran->srBRec.szStoreID);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
			}
			else
			{
			       /*沒開啟櫃號功能，則不印櫃號*/
			}

		}
		else
		{
			/* Invoice Number */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			if (pobTran->srBRec.uszVOIDBit != VS_TRUE)
			{
				sprintf(szTemplate1, "INV:%06ld", pobTran->srBRec.lnOrgInvNum);
			}
			else
			{
				sprintf(szTemplate1, "*INV:%06ld", pobTran->srBRec.lnOrgInvNum);
			}
			
			/* 【需求單 - 105259】總額明細報表及總額明細查詢補強機制 商店聯出紙本要印<P> */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetESCMode(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0 && pobTran->srBRec.inESCUploadStatus == _ESC_UPLOAD_STATUS_PAPER_)
			{
				strcat(szTemplate1, " <P>");
			}
			strcat(szPrintBuf, szTemplate1);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* Print Amount */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));

			if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
			{
				switch (pobTran->srBRec.inCode)
				{
					case _SALE_:
					case _SALE_OFFLINE_ :
					case _PRE_COMP_ :
					case _PRE_AUTH_ :
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						break;
					case _TIP_ :
						sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
						break;
					case _REFUND_ :
						sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
						break;
					default :
						memset(szTemplate2, 0x00, sizeof(szTemplate2));
						inGetHostLabel(szTemplate2);
						sprintf(szTemplate1,"%s_AMT_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			else
			{
				switch (pobTran->srBRec.inOrgCode)
				{
					case _SALE_:
					case _SALE_OFFLINE_ :
					case _PRE_COMP_ :
					case _PRE_AUTH_ :
						sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
						break;
					/* NCCC小費不能取消 */
					case _REFUND_ :
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						break;
					default :
						memset(szTemplate2, 0x00, sizeof(szTemplate2));
						inGetHostLabel(szTemplate2);
						sprintf(szTemplate1, "%s_AMT_VOID_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			inFunc_Amount_Comma(szTemplate1, "NT$" , 0x00, _SIGNED_NONE_, 16, _PADDING_RIGHT_);
			strcat(szPrintBuf, szTemplate1);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* Trans Type */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
			sprintf(szPrintBuf, "%s %s", szTemplate1, szTemplate2);
			
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* 客製化47不印卡別 */
			if (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_) != 0)
			{
				/* 卡別 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 12, _PADDING_RIGHT_);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
			}
			
			/* 卡號 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcpy(szPrintBuf, pobTran->srBRec.szPAN);
			inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 19, _PADDING_RIGHT_);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* Trans Date Time */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "DATE: %.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "TIME: %.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* Approved No. & RRN NO. */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcpy(szPrintBuf, "APPR: ");
			strcat(szPrintBuf, pobTran->srBRec.szAuthCode);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;
			
			/* 檢查碼Check No */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			if (strlen(pobTran->srBRec.szCheckNO) > 0)
			{
				strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
			}
			else
			{
				inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
			}
			sprintf(szPrintBuf, "No.: %s",szTemplate1);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;
			
			/* Store ID */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetStoreIDEnable(szFuncEnable);
			if (szFuncEnable[0] == 'Y')
			{
				/*開啟櫃號功能*/
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "STORE ID:%s", pobTran->srBRec.szStoreID);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				if (inRetVal != VS_SUCCESS)
					break;
			}
			else
			{
			       /*沒開啟櫃號功能，則不印櫃號*/
			}

		}/* 商店聯卡號遮掩 END */

                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        } /* End for () .... */

	/* 結束讀取 */
	inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(pobTran);
		
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inCREDIT_PRINT_DCC_DetailReportBottom_ByBuffer_046()_END");
	}
	
        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_HG_DetailReportBottom_ByBuffer_046
Date&Time       :2022/4/19 上午 10:36
Describe        :HG用明細紀錄
*/
int inCREDIT_PRINT_HG_DetailReportBottom_ByBuffer_046(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
	int	inRetVal = VS_SUCCESS;
        char	szPrintBuf[62 + 1], szTemplate1[62 + 1], szTemplate2[62 + 1];
        char	szFuncEnable[1 + 1];			/* catch Y or N */
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inCREDIT_PRINT_HG_DetailReportBottom_ByBuffer_046()_START");
	}

	/* 開始讀取 */
	inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(pobTran);
	/* 預設為無須重找 */
	guszEnormousNoNeedResetBit = VS_TRUE;
	
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);
	
        for (i = 0; i < inRecordCnt; i ++)
        {
                /*. 開始讀取每一筆交易記錄 .*/
                if (inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(pobTran, i) != VS_SUCCESS)
                {
                        inRetVal = VS_ERROR;
                        break;
                }
		
		/* Invoice Number */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate1, 0x00, sizeof(szTemplate1));
		if (pobTran->srBRec.uszVOIDBit != VS_TRUE)
		{
			sprintf(szTemplate1, "REF:%s/%06ld", pobTran->srBRec.szHGRefNo, pobTran->srBRec.lnOrgInvNum);
		}
		else
		{
			sprintf(szTemplate1, "*REF:%s/%06ld", pobTran->srBRec.szHGRefNo, pobTran->srBRec.lnOrgInvNum);
		}
		
		/* 【需求單 - 105259】總額明細報表及總額明細查詢補強機制 商店聯出紙本要印<P> */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetESCMode(szFuncEnable);
		if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0 && pobTran->srBRec.inESCUploadStatus == _ESC_UPLOAD_STATUS_PAPER_)
		{
			strcat(szTemplate1, " <P>");
		}
				
                strcat(szPrintBuf, szTemplate1);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		
		if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		
		/* Print Amount */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate1, 0x00, sizeof(szTemplate1));

                if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
                {
                        switch (pobTran->srBRec.inHGCode)
			{
				case _HG_FULL_REDEMPTION_:	/* 點數兌換金額帶0 */
					sprintf(szTemplate1, "%d", 0);
                                        break;
				case _HG_POINT_CERTAIN_CASH_:
				case _HG_POINT_CERTAIN_GIFT_PAPER_:
				case _HG_POINT_CERTAIN_CREDIT_:
				case _HG_POINT_CERTAIN_CREDIT_INSIDE_:
				case _HG_POINT_CERTAIN_CUP_:
				case _HG_REWARD_CASH_:
				case _HG_REWARD_GIFT_PAPER_:
				case _HG_REWARD_CREDIT_:
				case _HG_REWARD_CREDIT_INSIDE_:
				case _HG_REWARD_CUP_:
					sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
                                        break;
				case _HG_ONLINE_REDEEM_CREDIT_:
				case _HG_ONLINE_REDEEM_CREDIT_INSIDE_:
				case _HG_ONLINE_REDEEM_CUP_:
				        /* 全額扣抵金額要顯示0 */
					if ((pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnHGRedeemAmount) == 0L)
						sprintf(szTemplate1, "%d", 0);
					else
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);

					break;
				case _HG_ONLINE_REDEEM_CASH_:
				case _HG_ONLINE_REDEEM_GIFT_PAPER_:
				        /* 點數扣抵 - 現金、禮卷 要顯示實際支付金額 */
					sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGAmount);
					break;
				case _HG_REWARD_REFUND_:
				case _HG_REDEEM_REFUND_:
					/* 不用印金額 */
					break;
				default :
					memset(szTemplate2, 0x00, sizeof(szTemplate2));
					inGetHostLabel(szTemplate2);
                                        sprintf(szTemplate1,"%s_AMT_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inHGCode);
					break;
			} /* End switch () */
			
		}
		else
		{
			switch (pobTran->srBRec.inHGCode)
			{
				case _HG_FULL_REDEMPTION_:
					sprintf(szTemplate1, "%d", 0);
                                        break;
				case _HG_POINT_CERTAIN_CASH_:
				case _HG_POINT_CERTAIN_GIFT_PAPER_:
				case _HG_POINT_CERTAIN_CREDIT_:
				case _HG_POINT_CERTAIN_CREDIT_INSIDE_:
				case _HG_POINT_CERTAIN_CUP_:
				case _HG_REWARD_CASH_:
				case _HG_REWARD_GIFT_PAPER_:
				case _HG_REWARD_CREDIT_:
				case _HG_REWARD_CREDIT_INSIDE_:
				case _HG_REWARD_CUP_:
					sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
					break;
				case _HG_ONLINE_REDEEM_CREDIT_:
				case _HG_ONLINE_REDEEM_CREDIT_INSIDE_:
				case _HG_ONLINE_REDEEM_CUP_:
				        /* 全額扣抵金額要顯示0 */
					if ((pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnHGRedeemAmount) == 0L)
						sprintf(szTemplate1, "%d", 0);
					else
						sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
					break;
                                case _HG_ONLINE_REDEEM_CASH_:
				case _HG_ONLINE_REDEEM_GIFT_PAPER_:
				        /* 點數扣抵 - 現金、禮卷 要顯示實際支付金額 */
					sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnHGAmount));
					break;
				case _HG_REWARD_REFUND_:
				case _HG_REDEEM_REFUND_:
					/* 不用印金額 */
					break;
				default :
					memset(szTemplate2, 0x00, sizeof(szTemplate2));
					inGetHostLabel(szTemplate2);
                                        sprintf(szTemplate1,"%s_VOID_AMT_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inHGCode);
					break;
			} /* End switch () */
				
                }
		
		/* 退貨不用印金額 */
		if (strlen(szTemplate1) > 0)
		{
			strcat(szPrintBuf, "NT$ ");
			inFunc_Amount_Comma(szTemplate1, "" , 0x00, _SIGNED_NONE_, 13, _PADDING_RIGHT_);
			strcat(szPrintBuf, szTemplate1);
		}
                
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                /* Trans Type */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                vdCREDIT_PRINT_HG_GetTransactionType(pobTran, szTemplate1, szTemplate2);
		sprintf(szPrintBuf, "%s %s", szTemplate1, szTemplate2);
		
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

                /* 卡號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcat(szPrintBuf, "PAN: ");
		strcat(szPrintBuf, pobTran->srBRec.szHGPAN);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		
                /* Trans Date Time */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "DATE: %.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
		
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "TIME: %.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
		
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
                /* Approved No. */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, "APPR:");
		strcat(szPrintBuf, pobTran->srBRec.szHGAuthCode);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

                /* Store ID */
                memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
                inGetStoreIDEnable(szFuncEnable);
                if (szFuncEnable[0] == 'Y')
                {
                        /*開啟櫃號功能*/
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "STORE ID:%s", pobTran->srBRec.szStoreID);
			
                        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
                }
                else
                {
                       /*沒開啟櫃號功能，則不印櫃號*/
                }
		
		/* HappyGo資訊 */
		inCREDIT_PRINT_HG_DetailReport_HGData_ByBuffer(pobTran, inRecordCnt, uszBuffer, srFont_Attrib, srBhandle);
		
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        } /* End for () .... */
	
	/* 結束讀取 */
	inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(pobTran);
	
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inCREDIT_PRINT_DCC_DetailReportBottom_ByBuffer_046()_END");
	}
	
	return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_End_ByBuffer_046
Date&Time       :2022/4/19 上午 10:53
Describe        :列印結尾
*/
int inCREDIT_PRINT_End_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
	
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);
	
	if (pobTran->uszEverRich_Settle_RepeintBit == VS_TRUE)
	{
		inPRINT_Buffer_PutIn("重印 REPRINT", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	}
	else
	{
		inPRINT_Buffer_PutIn("*** 列印完成 ***", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	}

        for (i = 0; i < 8; i++)
	{
		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Top_ESVC_ByBuffer_046
Date&Time       :2022/4/19 下午 3:04
Describe        :
*/
int inCREDIT_PRINT_Top_ESVC_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal = VS_SUCCESS;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];
	
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);

	do
	{
		/* Get商店代號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetMerchantID(szTemplate);

		/* 列印商店代號 */
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_LEFT_);
		sprintf(szPrintBuf, "商店代號%s", szTemplate);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;

		/* Get端末機代號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTerminalID(szTemplate);

		/* 列印端末機代號 */
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PADDING_LEFT_);
		sprintf(szPrintBuf, "端末機代號%s", szTemplate);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;

		inRetVal = inPRINT_Buffer_PutIn("================================================", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;

		/* 主機、批號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		inGetHostLabel(szTemplate);
		sprintf(szPrintBuf, "主機　　 %s", szTemplate);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "批號　　 %s", "");
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
			break;

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		inGetBatchNum(szPrintBuf);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			break;

		/* 列印日期時間 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "日期/時間 :  %.4s/%.2s/%.2s %.2s:%.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2], &pobTran->srBRec.szTime[4]);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;
		
		break;
	}while(1);
	
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
	
        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_TotalAmount_ByBuffer_ESVC_046
Date&Time       :2022/4/19 下午 3:06
Describe        :列印總金額
*/
int inCREDIT_PRINT_TotalAmount_ByBuffer_046_ESVC(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szPrintBuf[84 + 1], szTemplate[84 + 1];
	
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);

        inPRINT_Buffer_PutIn("總額報表", _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
        
        /*     筆數(CNT)      金額(AMOUNT) */
	inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
        inPRINT_Buffer_PutIn_Specific_X_Position("筆數(CNT)", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_04_);
	inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
	inPRINT_Buffer_PutIn("金額(AMOUNT)", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	
	/* 銷售 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "購貨");
	inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%03lu $", srAccumRec->lnDeductTotalCount);
	inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
	inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_06_);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%lld", srAccumRec->llDeductTotalAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	
	/* 退貨 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "退貨");
	inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%03lu $", srAccumRec->lnRefundTotalCount);
	inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
	inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_06_);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%lld", srAccumRec->llRefundTotalAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	
	/* 現金加值 */
	if (srAccumRec->lnADDTotalCount > 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "現金加值");
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%03lu $", srAccumRec->lnADDTotalCount);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_06_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%lld", srAccumRec->llADDTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	}
	
	/* 加值取消 */
	if (srAccumRec->lnVoidADDTotalCount > 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "加值取消");
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%03lu $", srAccumRec->lnVoidADDTotalCount);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_06_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%lld", srAccumRec->llVoidADDTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	}
		
	inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 交易淨額(總購貨 - 總退貨) */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "交易淨額");
	inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%03lu $", (srAccumRec->lnDeductTotalCount + srAccumRec->lnRefundTotalCount));
	inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
	inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_06_);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%lld", (srAccumRec->llDeductTotalAmount - srAccumRec->llRefundTotalAmount));
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	
	/* 加值淨額 */
	if (srAccumRec->lnADDTotalCount > 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "加值淨額");
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%03lu $", (srAccumRec->lnADDTotalCount + srAccumRec->lnVoidADDTotalCount));
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_06_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%lld", (srAccumRec->llADDTotalAmount - srAccumRec->llVoidADDTotalAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	}
	
	inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_DetailReportMiddle_ByBuffer_ESVC_046
Date&Time       :2022/4/19 下午 3:40
Describe        :
*/
int inCREDIT_PRINT_DetailReportMiddle_ByBuffer_046_ESVC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);
	
        inPRINT_Buffer_PutIn("明細報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

	inPRINT_Buffer_PutIn("調閱編號", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn_Specific_X_Position("交易金額", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
	
        inPRINT_Buffer_PutIn("交易類別", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn_Specific_X_Position("卡別", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
	
        inPRINT_Buffer_PutIn("卡號", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	inPRINT_Buffer_PutIn("交易日期", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn_Specific_X_Position("交易時間", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
	
	inPRINT_Buffer_PutIn("RF序號", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("RRN", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
        inPRINT_Buffer_PutIn("==========================================", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_DetailReportBottom_ByBuffer_046_ESVC
Date&Time       :2022/4/19 下午 3:57
Describe        :
*/
int inCREDIT_PRINT_DetailReportBottom_ByBuffer_046_ESVC(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inReadCnt = 0;
	int	inRetVal = VS_SUCCESS;
	int	inLen = 0;
        char	szPrintBuf[62 + 1], szTemplate1[62 + 1];
        char    szCustomerIndicator[3 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inCREDIT_PRINT_DetailReportBottom_ByBuffer_046_ESVC()_START");
	}
	
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 開始讀取 */
	inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_START(pobTran);
	guszEnormousNoNeedResetBit = VS_TRUE;

        for (inReadCnt = 0; inReadCnt < inRecordCnt; inReadCnt ++)
        {
                /*. 開始讀取每一筆交易記錄 .*/
                if (inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_Read(pobTran, inReadCnt) != VS_SUCCESS)
                {
                        inRetVal = VS_ERROR;
                        break;
                }
		
		/* 調閱編號 & Amount */
		/* Invoice Number */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "INV:%06ld", pobTran->srTRec.lnInvNum);
		strcat(szPrintBuf, szTemplate1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
		{
			break;
		}

		/* Print Amount */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));

		switch (pobTran->srTRec.inCode)
		{
			case _TICKET_IPASS_AUTO_TOP_UP_:
				sprintf(szTemplate1, "%ld", pobTran->srTRec.lnTotalTopUpAmount);
				break;
			default :
				sprintf(szTemplate1, "%ld", pobTran->srTRec.lnTxnAmount);
				break;
		} /* End switch () */
		
		inFunc_Amount_Comma(szTemplate1, "NT$" , 0x00, _SIGNED_NONE_, 16, _PADDING_RIGHT_);
		strcat(szPrintBuf, szTemplate1);
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		if (inRetVal != VS_SUCCESS)
		{
			break;
		}

		/* 交易類別 & 卡別 */
		/* Trans Type */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inFunc_GetTransType_ESVC(pobTran, szTemplate1);
		sprintf(szPrintBuf, "%s", szTemplate1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
		{
			break;
		}

		/* 卡別 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
		{
		        strcat(szPrintBuf, "一卡通");
		}
		else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
		{
		        strcat(szPrintBuf, "悠遊卡");
		}
		else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
		{
		        strcat(szPrintBuf, "愛金卡");
		}
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		if (inRetVal != VS_SUCCESS)
		{
			break;
		}
		
		
		/* 卡號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
		{
			inLen = strlen(pobTran->srTRec.szUID);
			memcpy(szPrintBuf, pobTran->srTRec.szUID, inLen);
			szPrintBuf[inLen - 2] = 0x2A;
        		szPrintBuf[inLen - 1] = 0x2A;
		}
		else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
		{
                        inLen = strlen(pobTran->srTRec.szUID);
                        memcpy(szPrintBuf, pobTran->srTRec.szUID, inLen);
                        
                        /* 客製化098，悠遊卡因為使用T0200，加密遮掩為倒數5、6、7 */
                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))
                        {
                                szPrintBuf[inLen - 5] = 0x2A;
                                szPrintBuf[inLen - 6] = 0x2A;
                                szPrintBuf[inLen - 7] = 0x2A;
                        }
                        else
                        {		
                                if (atoi(pobTran->srTRec.srECCRec.szPurseVersionNumber) == 0)
                                {
                                        szPrintBuf[inLen - 5] = 0x2A;
                                        szPrintBuf[inLen - 6] = 0x2A;
                                        szPrintBuf[inLen - 7] = 0x2A;
                                }
                                else
                                {
                                        /* 全部16 第6-11隱碼 */
                                        szPrintBuf[inLen - 6] = 0x2A;
                                        szPrintBuf[inLen - 7] = 0x2A;
                                        szPrintBuf[inLen - 8] = 0x2A;
                                        szPrintBuf[inLen - 9] = 0x2A;
                                        szPrintBuf[inLen - 10] = 0x2A;
                                        szPrintBuf[inLen - 11] = 0x2A;
                                }
                        }    
		}
		else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
		{
			inLen = strlen(pobTran->srTRec.szUID);
			memcpy(szPrintBuf, pobTran->srTRec.szUID, inLen);
			/* 愛金卡：遮掩9-12 碼，遮掩字元為"*" */
			szPrintBuf[8] = 0x2A;
			szPrintBuf[9] = 0x2A;
			szPrintBuf[10] = 0x2A;
			szPrintBuf[11] = 0x2A;				
		}
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
		{
			break;
		}

		/* Trans Date Time */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "DATE: 20%.4s/%.2s/%.2s", &pobTran->srTRec.szDate[0], &pobTran->srTRec.szDate[2], &pobTran->srTRec.szDate[4]);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
		{
			break;
		}

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "TIME: %.2s:%.2s",  &pobTran->srTRec.szTime[0], &pobTran->srTRec.szTime[2]);
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		if (inRetVal != VS_SUCCESS)
		{
			break;
		}

		/* RF序號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, "RF no: ");
		strcat(szPrintBuf, pobTran->srTRec.szTicketRefundCode);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
		{
			break;
		}
		
		/* 悠遊卡要多印RRN */
		if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
		{
		        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		        sprintf(szPrintBuf, "RRN : %s", pobTran->srTRec.srECCRec.szRRN);
        		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		}
				
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        } /* End for () .... */
	
	/* 結束讀取 */
	inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_END(pobTran);
	
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inCREDIT_PRINT_DetailReportBottom_ByBuffer_046_ESVC()_END");
	}
	
        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_End_ByBuffer_ESVC_046
Date&Time       :2022/4/19 下午 4:02
Describe        :列印結尾
*/
int inCREDIT_PRINT_End_ByBuffer_ESVC_046(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC* srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
	
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);
	
	if (pobTran->uszEverRich_Settle_RepeintBit == VS_TRUE)
	{
		inPRINT_Buffer_PutIn("重印 REPRINT", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	}
	else
	{
		inPRINT_Buffer_PutIn("*** 列印完成 ***", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	}

        for (i = 0; i < 8; i++)
	{
		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmount_ByBuffer_046
Date&Time       :2022/4/18 下午 5:36
Describe        :列印總金額
*/
int inCREDIT_PRINT_TotalAmount_ByBuffer_046(TRANSACTION_OBJECT *pobTran, void *srAccumRecOrg, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szPrintBuf[84 + 1] = {0}, szTemplate[84 + 1] = {0};
	ACCUM_TOTAL_REC *srAccumRec;
        
	srAccumRec = srAccumRecOrg;

        if (pobTran->inRunOperationID == _OPERATION_SETTLE_)
	{
                /* 客製化098，結帳流程，預先列印，TITLE為總額 */
                if (pobTran->uszPrePrintBit == VS_TRUE)
		{
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
                        inPRINT_Buffer_PutIn("總額報表", _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		}
                else
		{
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
                        inPRINT_Buffer_PutIn("結帳報表", _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		}
	}
	else
        {
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
                inPRINT_Buffer_PutIn("總額報表", _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
        }

	/*     筆數(CNT)      金額(AMOUNT) */
	inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
        inPRINT_Buffer_PutIn_Specific_X_Position("筆數(CNT)", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_04_);
	inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
	inPRINT_Buffer_PutIn("金額(AMOUNT)", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);

        if (srAccumRec->lnTotalCount == 0)
        {
		/* 銷售 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "銷售 Ｄ");
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%03lu $", 0L);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_05_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%lld", 0LL);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "退貨 Ｒ");
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%03lu $", 0L);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_05_);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld", 0L);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
                /* 淨額 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "淨額 Ｔ　%03lu   $  ", 0L);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%03lu $", 0L);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_05_);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szTemplate, "%ld", 0L);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "小費");
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%03lu $", 0L);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_05_);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szTemplate, "%ld", 0L);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                inPRINT_Buffer_PutIn("==========================================", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }
        else
        {
		/* 銷售 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "銷售 Ｄ");
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%03lu $", srAccumRec->lnTotalSaleCount);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_05_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%lld", (srAccumRec->llTotalSaleAmount + srAccumRec->llTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "退貨 Ｒ");
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%03lu $", srAccumRec->lnTotalRefundCount);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_05_);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
                /* 淨額 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "淨額 Ｔ");
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%03lu $", srAccumRec->lnTotalCount);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_05_);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%lld", srAccumRec->llTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "小費");
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%03lu $", srAccumRec->lnTotalTipsCount);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_05_);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szTemplate, "%lld", srAccumRec->llTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
                inPRINT_Buffer_PutIn("==========================================", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }
	
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmountByCard_ByBuffer_046
Date&Time       :2022/4/20 上午 10:05
Describe        :依卡別列印
*/
int inCREDIT_PRINT_TotalAmountByCard_ByBuffer_046(TRANSACTION_OBJECT *pobTran, void *srAccumRecOrg, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szPrintBuf[84 + 1], szTemplate[84 + 1];
	char	szUnknownCardLabel[20 + 1];
	ACCUM_TOTAL_REC *srAccumRec;
	srAccumRec = srAccumRecOrg;
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);

        inPRINT_Buffer_PutIn("卡別小計", _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

        if (srAccumRec->llUCardTotalSaleAmount != 0L || srAccumRec->llUCardTotalRefundAmount != 0L)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_TWIN_CARD_);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   $", srAccumRec->lnUCardTotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llUCardTotalSaleAmount + srAccumRec->llUCardTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   $", srAccumRec->lnUCardTotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llUCardTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   $", srAccumRec->lnUCardTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llUCardTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   $", srAccumRec->lnUCardTotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llUCardTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }

        if (srAccumRec->llVisaTotalSaleAmount != 0L || srAccumRec->llVisaTotalRefundAmount != 0L)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_VISA_);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   $", srAccumRec->lnVisaTotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llVisaTotalSaleAmount + srAccumRec->llVisaTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   $", srAccumRec->lnVisaTotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llVisaTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   $", srAccumRec->lnVisaTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llVisaTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   $", srAccumRec->lnVisaTotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llVisaTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }

        if (srAccumRec->llMasterTotalSaleAmount != 0L || srAccumRec->llMasterTotalRefundAmount != 0L)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_MASTERCARD_);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   $", srAccumRec->lnMasterTotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llMasterTotalSaleAmount + srAccumRec->llMasterTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   $", srAccumRec->lnMasterTotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llMasterTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   $", srAccumRec->lnMasterTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llMasterTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   $", srAccumRec->lnMasterTotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llMasterTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }

        if (srAccumRec->llJcbTotalSaleAmount != 0L || srAccumRec->llJcbTotalRefundAmount != 0L)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_JCB_);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   $", srAccumRec->lnJcbTotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llJcbTotalSaleAmount + srAccumRec->llJcbTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   $", srAccumRec->lnJcbTotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llJcbTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   $", srAccumRec->lnJcbTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llJcbTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   $", srAccumRec->lnJcbTotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llJcbTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }

        if (srAccumRec->llAmexTotalSaleAmount != 0L || srAccumRec->llAmexTotalRefundAmount != 0L)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_AMEX_);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   $", srAccumRec->lnAmexTotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llAmexTotalSaleAmount + srAccumRec->llAmexTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   $", srAccumRec->lnAmexTotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llAmexTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   $", srAccumRec->lnAmexTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llAmexTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   $", srAccumRec->lnAmexTotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llAmexTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }

        if (srAccumRec->llCupTotalSaleAmount != 0L || srAccumRec->llCupTotalRefundAmount != 0L)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_CUP_);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   $", srAccumRec->lnCupTotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llCupTotalSaleAmount + srAccumRec->llCupTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   $", srAccumRec->lnCupTotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llCupTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   $", srAccumRec->lnCupTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llCupTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   $", srAccumRec->lnCupTotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llCupTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }
	
	if (srAccumRec->llDinersTotalSaleAmount != 0L || srAccumRec->llDinersTotalRefundAmount != 0L)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_DINERS_);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   $", srAccumRec->lnDinersTotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llDinersTotalSaleAmount + srAccumRec->llDinersTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   $", srAccumRec->lnDinersTotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llDinersTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   $", srAccumRec->lnDinersTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llDinersTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   $", srAccumRec->lnDinersTotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llDinersTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }
	
	if (srAccumRec->llFiscTotalSaleAmount != 0L || srAccumRec->llFiscTotalRefundAmount != 0L)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_SMARTPAY_);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   $", srAccumRec->lnFiscTotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llFiscTotalSaleAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   $", srAccumRec->lnFiscTotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llFiscTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   $", srAccumRec->lnFiscTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llFiscTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		/* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   $", (unsigned long)0);
                sprintf(szTemplate, "%lld", (long long)0);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }

	/* 【需求單 - 108097】Card BIN由ATS查核 add by LingHsiung 2020/2/3 下午 5:19 */
	/* 新增未知卡別小計 add by LingHsiung 2020/7/7 下午 2:26 */
	if (srAccumRec->llX0TotalSaleAmount != 0L || srAccumRec->llX0TotalRefundAmount != 0L)
	{
		inLoadCDTXRec(0);
		memset(szUnknownCardLabel, 0x00, sizeof(szUnknownCardLabel));
		inGetUnknownCardLabel(szUnknownCardLabel);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", szUnknownCardLabel);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   $", srAccumRec->lnX0TotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llX0TotalSaleAmount + srAccumRec->llX0TotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   $", srAccumRec->lnX0TotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llX0TotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   $", srAccumRec->lnX0TotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX0TotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   $", srAccumRec->lnX0TotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX0TotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	if (srAccumRec->llX1TotalSaleAmount != 0L || srAccumRec->llX1TotalRefundAmount != 0L)
	{
		inLoadCDTXRec(1);
		memset(szUnknownCardLabel, 0x00, sizeof(szUnknownCardLabel));
		inGetUnknownCardLabel(szUnknownCardLabel);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", szUnknownCardLabel);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   $", srAccumRec->lnX1TotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llX1TotalSaleAmount + srAccumRec->llX1TotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   $", srAccumRec->lnX1TotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llX1TotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   $", srAccumRec->lnX1TotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX1TotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   $", srAccumRec->lnX1TotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX1TotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	if (srAccumRec->llX2TotalSaleAmount != 0L || srAccumRec->llX2TotalRefundAmount != 0L)
	{
		inLoadCDTXRec(2);
		memset(szUnknownCardLabel, 0x00, sizeof(szUnknownCardLabel));
		inGetUnknownCardLabel(szUnknownCardLabel);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", szUnknownCardLabel);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   $", srAccumRec->lnX2TotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llX2TotalSaleAmount + srAccumRec->llX2TotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   $", srAccumRec->lnX2TotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llX2TotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   $", srAccumRec->lnX2TotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX2TotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   $", srAccumRec->lnX2TotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX2TotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	if (srAccumRec->llX3TotalSaleAmount != 0L || srAccumRec->llX3TotalRefundAmount != 0L)
	{
		inLoadCDTXRec(3);
		memset(szUnknownCardLabel, 0x00, sizeof(szUnknownCardLabel));
		inGetUnknownCardLabel(szUnknownCardLabel);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", szUnknownCardLabel);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   $", srAccumRec->lnX3TotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llX3TotalSaleAmount + srAccumRec->llX3TotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   $", srAccumRec->lnX3TotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llX3TotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   $", srAccumRec->lnX3TotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX3TotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   $", srAccumRec->lnX3TotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX3TotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	if (srAccumRec->llX4TotalSaleAmount != 0L || srAccumRec->llX4TotalRefundAmount != 0L)
	{
		inLoadCDTXRec(4);
		memset(szUnknownCardLabel, 0x00, sizeof(szUnknownCardLabel));
		inGetUnknownCardLabel(szUnknownCardLabel);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", szUnknownCardLabel);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   $", srAccumRec->lnX4TotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llX4TotalSaleAmount + srAccumRec->llX4TotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   $", srAccumRec->lnX4TotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llX4TotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   $", srAccumRec->lnX4TotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX4TotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   $", srAccumRec->lnX4TotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX4TotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	if (srAccumRec->llX5TotalSaleAmount != 0L || srAccumRec->llX5TotalRefundAmount != 0L)
	{
		inLoadCDTXRec(5);
		memset(szUnknownCardLabel, 0x00, sizeof(szUnknownCardLabel));
		inGetUnknownCardLabel(szUnknownCardLabel);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", szUnknownCardLabel);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   $", srAccumRec->lnX5TotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llX5TotalSaleAmount + srAccumRec->llX5TotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   $", srAccumRec->lnX5TotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llX5TotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   $", srAccumRec->lnX5TotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX5TotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   $", srAccumRec->lnX5TotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX5TotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	if (srAccumRec->llX6TotalSaleAmount != 0L || srAccumRec->llX6TotalRefundAmount != 0L)
	{
		inLoadCDTXRec(6);
		memset(szUnknownCardLabel, 0x00, sizeof(szUnknownCardLabel));
		inGetUnknownCardLabel(szUnknownCardLabel);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", szUnknownCardLabel);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   $", srAccumRec->lnX6TotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llX6TotalSaleAmount + srAccumRec->llX6TotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   $", srAccumRec->lnX6TotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llX6TotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   $", srAccumRec->lnX6TotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX6TotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   $", srAccumRec->lnX6TotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX6TotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	if (srAccumRec->llX7TotalSaleAmount != 0L || srAccumRec->llX7TotalRefundAmount != 0L)
	{
		inLoadCDTXRec(7);
		memset(szUnknownCardLabel, 0x00, sizeof(szUnknownCardLabel));
		inGetUnknownCardLabel(szUnknownCardLabel);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", szUnknownCardLabel);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   $", srAccumRec->lnX7TotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llX7TotalSaleAmount + srAccumRec->llX7TotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   $", srAccumRec->lnX7TotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llX7TotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   $", srAccumRec->lnX7TotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX7TotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   $", srAccumRec->lnX7TotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX7TotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	if (srAccumRec->llX8TotalSaleAmount != 0L || srAccumRec->llX8TotalRefundAmount != 0L)
	{
		inLoadCDTXRec(8);
		memset(szUnknownCardLabel, 0x00, sizeof(szUnknownCardLabel));
		inGetUnknownCardLabel(szUnknownCardLabel);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", szUnknownCardLabel);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   $", srAccumRec->lnX8TotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llX8TotalSaleAmount + srAccumRec->llX8TotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   $", srAccumRec->lnX8TotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llX8TotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   $", srAccumRec->lnX8TotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX8TotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   $", srAccumRec->lnX8TotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX8TotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	if (srAccumRec->llX9TotalSaleAmount != 0L || srAccumRec->llX9TotalRefundAmount != 0L)
	{
		inLoadCDTXRec(9);
		memset(szUnknownCardLabel, 0x00, sizeof(szUnknownCardLabel));
		inGetUnknownCardLabel(szUnknownCardLabel);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", szUnknownCardLabel);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   $", srAccumRec->lnX9TotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llX9TotalSaleAmount + srAccumRec->llX9TotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   $", srAccumRec->lnX9TotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llX9TotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   $", srAccumRec->lnX9TotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX9TotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   $", srAccumRec->lnX9TotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX9TotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
	
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmountByInstllment_ByBuffer_046
Date&Time       :2016/2/24 下午 3:50
Describe        :
*/
int inCREDIT_PRINT_TotalAmountByInstllment_ByBuffer_046(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char    szPrintBuf[84 + 1], szTemplate[84 + 1];
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);

        inPRINT_Buffer_PutIn("分期交易總額", _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

	inPRINT_Buffer_PutIn("    筆數(CNT)      金額(AMOUNT)", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* 銷售 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "銷售 Ｄ　%03lu   NT$", srAccumRec->lnInstSaleCount);
	sprintf(szTemplate, "%lld", (srAccumRec->llInstSaleAmount + srAccumRec->llInstTipsAmount));
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 退貨 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "退貨 Ｒ　%03lu   NT$", srAccumRec->lnInstRefundCount);
	sprintf(szTemplate, "%lld", (0 - srAccumRec->llInstRefundAmount));
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 淨額 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "淨額 Ｔ　%03lu   NT$", srAccumRec->lnInstTotalCount);
	sprintf(szTemplate, "%lld", srAccumRec->llInstTotalAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 小費 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnInstTipsCount);
	sprintf(szTemplate, "%lld", srAccumRec->llInstTipsAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("==========================================", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
	
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmountByRedemption_ByBuffer_046
Date&Time       :2022/4/25 下午 3:50
Describe        :
*/
int inCREDIT_PRINT_TotalAmountByRedemption_ByBuffer_046(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char    szPrintBuf[84 + 1], szTemplate[84 + 1];
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);
	
        inPRINT_Buffer_PutIn("紅利扣抵總額", _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

	inPRINT_Buffer_PutIn("    筆數(CNT)      金額(AMOUNT)", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* 銷售 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "銷售 Ｄ　%03lu   NT$", srAccumRec->lnRedeemSaleCount);
	sprintf(szTemplate, "%lld", (srAccumRec->llRedeemSaleAmount + srAccumRec->llRedeemTipsAmount));
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 退貨 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "退貨 Ｒ　%03lu   NT$", srAccumRec->lnRedeemRefundCount);
	sprintf(szTemplate, "%lld", (0 - srAccumRec->llRedeemRefundAmount));
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 淨額 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "淨額 Ｔ　%03lu   NT$", srAccumRec->lnRedeemTotalCount);
	sprintf(szTemplate, "%lld", srAccumRec->llRedeemTotalAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 小費 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnRedeemTipsCount);
	sprintf(szTemplate, "%lld", srAccumRec->llRedeemTipsAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("-----------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* 紅利扣抵總點數 */
	/* 初始化 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

	/* 將NT$ ＋數字塞到szTemplate中來inpad */
	sprintf(szTemplate, " %ld", (srAccumRec->lnRedeemTotalPoint));
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PADDING_LEFT_);

	/* 把前面的字串和數字結合起來 */
	sprintf(szPrintBuf, "紅利扣抵總點數 :%s點", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* 結束隔線 */
	inPRINT_Buffer_PutIn("", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("==========================================", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
	
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmountByOther_046
Date&Time       :2022/4/25 下午 3:51
Describe        :
*/
int inCREDIT_PRINT_TotalAmountByOther_046(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal = VS_ERROR;
	int	inSpace = 13;
	char	szESCMode[2 + 1] = {0};
	char	szPrintBuf[100 + 1] = {0};
	char	szTRTFileName[12 + 1] = {0};
	char	szTransFunc[20 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};

	/* 若找不到ESC或ESC沒開或水位為0，不送欄位NE */
	memset(szESCMode, 0x00, sizeof(szESCMode));
	inGetESCMode(szESCMode);
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (memcmp(szESCMode, "Y", strlen("Y")) != 0)
	{
		return (VS_SUCCESS);
	}
	else
        {
		/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
		inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);
	
		memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
		inGetTRTFileName(szTRTFileName);
		
		if (!memcmp(szTRTFileName, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)) ||
		    !memcmp(szTRTFileName, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)))
		{
			/* 結帳才印已上傳筆數 */
			if (pobTran->inRunOperationID == _OPERATION_SETTLE_)
			{
				if (pobTran->uszPrePrintBit == VS_TRUE)
				{
					/* 麥當勞結帳時的總額 */
				}
				else
				{
					inRetVal = inPRINT_Buffer_PutIn("------------------------------------------", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					/* 電簽已上傳筆數 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%03lu", srAccumRec->lnESC_SuccessNum);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "電簽已上傳總筆數 = ");
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					/* 電簽已上傳金額 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%ld", (long)srAccumRec->llESC_SuccessAmount);
					inFunc_Amount_Comma(szPrintBuf, "NT$ ", ' ', _SIGNED_NONE_, inSpace, _PADDING_LEFT_);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "電簽已上傳總金額 = ");
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inRetVal = inPRINT_Buffer_PutIn("==========================================", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					/* 電簽未上傳筆數 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%03lu", srAccumRec->lnESC_FailUploadNum);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "電簽未上傳總筆數 = ");
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					/* 電簽未上傳金額 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%ld", (long)srAccumRec->llESC_FailUploadAmount);
					inFunc_Amount_Comma(szPrintBuf, "NT$ ", ' ', _SIGNED_NONE_, inSpace, _PADDING_LEFT_);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "電簽未上傳總金額 = ");
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


					/* 非電簽總筆數(BYPASS) */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%03lu", srAccumRec->lnESC_BypassNum);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "非電簽總筆數 = ");
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					/* 非電簽總金額 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%ld", (long)srAccumRec->llESC_BypassAmount);
					inFunc_Amount_Comma(szPrintBuf, "NT$ ", ' ', _SIGNED_NONE_, inSpace, _PADDING_LEFT_);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "非電簽總金額 = ");
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					inRetVal = inPRINT_Buffer_PutIn("==========================================", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					/* 紙本簽單總筆數 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%03lu", srAccumRec->lnESC_TotalFailULNum);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "紙本簽單總筆數 =");
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					/* 紙本簽單總金額 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%ld", (long)srAccumRec->llESC_TotalFailULAmount);
					inFunc_Amount_Comma(szPrintBuf, "NT$ ", ' ', _SIGNED_NONE_, inSpace, _PADDING_LEFT_);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "紙本簽單總金額 = ");
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inRetVal = inPRINT_Buffer_PutIn("------------------------------------------", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					memset(szTransFunc, 0x00, sizeof(szTransFunc));
					inGetTransFunc(szTransFunc);

					if (szTransFunc[5] == 'Y')
					{
						/* ESC預先授權列印紙本及預先授權不納入結帳總額 */
						inRetVal = inPRINT_Buffer_PutIn("＊以上統計不含預先授權交易", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						inRetVal = inPRINT_Buffer_PutIn("＊預先授權", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "　%03lu", srAccumRec->lnESC_PreAuthNum);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "　紙本簽單總筆數 = ");
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

						/* 紙本簽單總金額 */
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%ld", (long)srAccumRec->llESC_PreAuthAmount);
						inFunc_Amount_Comma(szPrintBuf, "NT$ ", ' ', _SIGNED_NONE_, inSpace, _PADDING_LEFT_);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "　紙本簽單總金額 = ");
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						inRetVal = inPRINT_Buffer_PutIn("------------------------------------------", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					}
				}
			}
                        
			/* ESC補強機制 */
			inCREDIT_PRINT_ESC_Reinforce_Count_ByBuffer(pobTran, uszBuffer, srFont_Attrib, srBhandle);
                }
        }

	inPRINT_Buffer_PutIn("", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
	
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Total_HG_ByBuffer_046
Date&Time       :2022/4/25 下午 3:49
Describe        :列印HG總額
*/
int inCREDIT_PRINT_Total_HG_ByBuffer_046(TRANSACTION_OBJECT *pobTran, HG_ACCUM_TOTAL_REC *srHGAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char    szPrintBuf[84 + 1], szTemplate[84 + 1];
	char	szTRTFileName[16 + 1];
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);
	
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	
	/* 選聯合印快樂購聯合集點卡，選HG印總額報表 */
	if (memcmp(szTRTFileName, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)) == 0)
	{
                inPRINT_Buffer_PutIn("快樂購聯合集點卡", _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	}
	else
	{
                inPRINT_Buffer_PutIn("總額報表", _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	}

	/* 紅利積點 */
	inPRINT_Buffer_PutIn("紅利積點", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 紅利積點 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "紅利積點 %03d   NT$", srHGAccumRec->inRewardCnt);
	sprintf(szTemplate, "%lu", srHGAccumRec->uslnRewardAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
	/* 點數扣抵 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "點數扣抵 %03d   NT$", srHGAccumRec->inOnlineRedeemCnt);
	sprintf(szTemplate, "%lu", srHGAccumRec->uslnOnlineRedeemAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
	/* 加價購 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "加價購   %03d   NT$", srHGAccumRec->inPointCertainCnt);
	sprintf(szTemplate, "%lu", srHGAccumRec->uslnPointCertainAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
	/* 小計 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "小計 　　      NT$");
	sprintf(szTemplate, "%lu", (srHGAccumRec->uslnRewardAmount + srHGAccumRec->uslnOnlineRedeemAmount + srHGAccumRec->uslnPointCertainAmount));
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
	
	/* 空行 */
	inPRINT_Buffer_PutIn("", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

	/* 紅利扣抵 */
	inPRINT_Buffer_PutIn("紅利扣抵", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 點數扣抵 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "點數扣抵 %03d      ", srHGAccumRec->inOnlineRedeemCnt);
	sprintf(szTemplate, "%lu　點", srHGAccumRec->uslnOnlineRedeemPoint);
	inFunc_PAD_ASCII(szTemplate, szTemplate , ' ', 16, _PADDING_LEFT_);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
	/* 加價購 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "加價購   %03d      ", srHGAccumRec->inPointCertainCnt);
	sprintf(szTemplate, "%lu　點", srHGAccumRec->uslnPointCertainPoint);
	inFunc_PAD_ASCII(szTemplate, szTemplate , ' ', 16, _PADDING_LEFT_);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
	/* 點數兌換 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "點數兌換 %03d      ", srHGAccumRec->inFullRedemptionCnt);
	sprintf(szTemplate, "%lu　點", srHGAccumRec->uslnFullRedemptionPoint);
	inFunc_PAD_ASCII(szTemplate, szTemplate , ' ', 16, _PADDING_LEFT_);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
	/* 小計 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "小計 　　         ");
	sprintf(szTemplate, "%lu　點", (srHGAccumRec->uslnOnlineRedeemPoint + srHGAccumRec->uslnPointCertainPoint + srHGAccumRec->uslnFullRedemptionPoint));
	inFunc_PAD_ASCII(szTemplate, szTemplate , ' ', 16, _PADDING_LEFT_);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
	
	/* 空行 */
	inPRINT_Buffer_PutIn("", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

	/* 回饋退貨 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "回饋退貨 %03d      ", srHGAccumRec->inRewardRefundCnt);
	sprintf(szTemplate, "%lu　點", srHGAccumRec->uslnRewardRefundPoint);
	inFunc_PAD_ASCII(szTemplate, szTemplate , ' ', 16, _PADDING_LEFT_);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
	/* 扣抵退貨 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "扣抵退貨 %03d      ", srHGAccumRec->inRedeemRefundCnt);
	sprintf(szTemplate, "%ld　點", (0 - srHGAccumRec->uslnRedeemRefundPoint));
	inFunc_PAD_ASCII(szTemplate, szTemplate , ' ', 16, _PADDING_LEFT_);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
	
	/* 空行 */
	inPRINT_Buffer_PutIn("", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
	
	/* 合計 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "合計 　　         ");
	sprintf(szTemplate, "%lu　點", (srHGAccumRec->uslnOnlineRedeemPoint + srHGAccumRec->uslnPointCertainPoint + srHGAccumRec->uslnFullRedemptionPoint + srHGAccumRec->uslnRewardRefundPoint - srHGAccumRec->uslnRedeemRefundPoint));
	inFunc_PAD_ASCII(szTemplate, szTemplate , ' ', 16, _PADDING_LEFT_);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

	/* 空行 */
	inPRINT_Buffer_PutIn("", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmount_ByBuffer_ESVC_046
Date&Time       :2022/4/26 下午 3:58
Describe        :列印總金額
*/
int inCREDIT_PRINT_TotalAmount_ByBuffer_046_ESVC_Settle(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char		szPrintBuf[84 + 1] = {0}, szTemplate[84 + 1] = {0};
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);

        inPRINT_Buffer_PutIn("結帳報表", _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
        
        inPRINT_Buffer_PutIn("       筆數           金額", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	/* 銷售 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "購貨 　　%03lu   NT$", srAccumRec->lnDeductTotalCount);
	sprintf(szTemplate, "%lld", srAccumRec->llDeductTotalAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 退貨 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnRefundTotalCount);
	sprintf(szTemplate, "%lld", srAccumRec->llRefundTotalAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	/* 現金加值 */
	if (srAccumRec->lnADDTotalCount > 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "現金加值 %03lu   NT$", srAccumRec->lnADDTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llADDTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	/* 加值取消 */
	if (srAccumRec->lnVoidADDTotalCount > 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "加值取消 %03lu   NT$", srAccumRec->lnVoidADDTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llVoidADDTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 交易淨額(總購貨 - 總退貨) */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "交易淨額 %03lu   NT$", (srAccumRec->lnDeductTotalCount + srAccumRec->lnRefundTotalCount));
	sprintf(szTemplate, "%lld", (srAccumRec->llDeductTotalAmount - srAccumRec->llRefundTotalAmount));
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	/* 加值淨額 */
	if (srAccumRec->lnADDTotalCount > 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "加值淨額 %03lu   NT$", (srAccumRec->lnADDTotalCount + srAccumRec->lnVoidADDTotalCount));
		sprintf(szTemplate, "%lld", (srAccumRec->llADDTotalAmount - srAccumRec->llVoidADDTotalAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	inPRINT_Buffer_PutIn("==========================================", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
	
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Top_ESVC_SETTLE_ByBuffer_046
Date&Time       :2022/4/26 下午 4:36
Describe        :
*/
int inCREDIT_PRINT_Top_ESVC_SETTLE_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);

        /* Get商店代號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetMerchantID(szTemplate);

        /* 列印商店代號 */
        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_LEFT_);
        sprintf(szPrintBuf, "商店代號%s", szTemplate);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        /* Get端末機代號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetTerminalID(szTemplate);

        /* 列印端末機代號 */
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PADDING_LEFT_);
        sprintf(szPrintBuf, "端末機代號%s", szTemplate);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        inRetVal = inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 交易 特店代號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	
	sprintf(szPrintBuf, "交易　　 %s", "結帳");
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	if (pobTran->uszEverRich_Settle_RepeintBit == VS_TRUE)
	{
		inRetVal = inPRINT_Buffer_PutIn("(重印)", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	
	/* 主機、批號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	
	inGetHostLabel(szTemplate);
	sprintf(szPrintBuf, "主機　　 %s", szTemplate);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "批號　　 %s", "");
	inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	if (pobTran->uszEverRich_NoDataBit == VS_TRUE)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%03d", 0);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	else
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		inGetBatchNum(szPrintBuf);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	
	/* 列印日期時間 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
	if (pobTran->uszEverRich_NoDataBit == VS_TRUE)
	{
		sprintf(szPrintBuf, "日期/時間 :  ");
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	else
	{
		sprintf(szPrintBuf, "日期/時間 :  %.4s/%.2s/%.2s %.2s:%.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2], &pobTran->srBRec.szTime[4]);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmountByCard_ByBuffer_ESVC_046
Date&Time       :2022/4/26 下午 3:53
Describe        :依卡別列印
*/
int inCREDIT_PRINT_TotalAmountByCard_ByBuffer_046_ESVC(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int		i = 0;
	char		szPrintBuf[100 + 1] = {0}, szTemplate[42 + 1] = {0};
	char		szTxnType[20 + 1] = {0};
	char		szTicketNeedNewBatch[2 + 1] = {0};
	unsigned char	uszNeedPrintBit = VS_FALSE;
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);
	
	/* 先檢查是否任一票證有開 */
	for (i = 0; i < 4; i++)
	{
		if (inLoadTDTRec(i) != VS_SUCCESS)
		{
			break;
		}
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTicket_HostEnable(szTemplate);
                if (memcmp(szTemplate, "Y", strlen("Y")) != 0)
		{
                        continue;
		}
		else
		{
			uszNeedPrintBit = VS_TRUE;
		}
	}
	
	/* 代表有任一票證要印 */
	if (uszNeedPrintBit == VS_TRUE)
	{
		inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("         卡別小計", _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	else
	{
		return (VS_SUCCESS);
	}
	
	/* 個別票證 */
        for (i = 0; i < 4; i++)
        {
                if (inLoadTDTRec(i) != VS_SUCCESS)
		{
			break;
		}
		
                memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTicket_HostEnable(szTemplate);
                if (memcmp(szTemplate, "Y", strlen("Y")) != 0)
		{
                        continue;
		}
                
                memset(szTxnType, 0x00, sizeof(szTxnType));
		inGetTicket_HostTransFunc(szTxnType);
		
                switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :
        	                inPRINT_Buffer_PutIn("卡別 一卡通", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
				
				/* 特店代號 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "特店代號%s", "");
				inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetIPASS_System_ID(szTemplate);
				memcpy(&szPrintBuf[0], szTemplate, 2);

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetIPASS_SP_ID(szTemplate);
				memcpy(&szPrintBuf[2], szTemplate, 2);

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetIPASS_Sub_Company_ID(szTemplate);
				memcpy(&szPrintBuf[4], szTemplate, 4); 
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
        	                break;
        	        case _TDT_INDEX_01_ECC_ :
        	                inPRINT_Buffer_PutIn("卡別 悠遊卡", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
          	                
				/* 特店代號 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "特店代號%s", "");
				inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetECC_New_SP_ID(szTemplate);
				strcpy(&szPrintBuf[0], szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				
				/* 二代設備編號 */
          	                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetTicket_Device2(szTemplate);
                        	sprintf(szPrintBuf, "二代設備編號　 %s", szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                        	
				/* 悠遊卡批次號碼 */
                        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTicketNeedNewBatch, 0x00, sizeof(szTicketNeedNewBatch));
				inGetTicket_NeedNewBatch(szTicketNeedNewBatch);
				if (memcmp(szTicketNeedNewBatch, "Y", strlen("Y")) == 0)
				{
					sprintf(szTemplate, "%.4s%.2s%.2s", &pobTran->srBRec.szDate[2], &pobTran->srBRec.szDate[6], _ECC_DEFAULT_BATCH_);
				}
				else
				{
					inGetTicket_Batch(szTemplate);
				}
                        	sprintf(szPrintBuf, "悠遊卡批次號碼 %s", szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
        	        case _TDT_INDEX_02_ICASH_ :
        	                inPRINT_Buffer_PutIn("卡別 愛金卡", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
				
				/* 特店代號 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "門市代碼%s", "");
				inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetICASH_Shop_ID(szTemplate);
				memcpy(&szPrintBuf[0], szTemplate, 8);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
        	                break;
        	        default :
        	                break;       
        	}

		/* 不合法 跳出 */
                if (i != _TDT_INDEX_00_IPASS_	&&
		    i != _TDT_INDEX_01_ECC_	&&
		    i != _TDT_INDEX_02_ICASH_)
		{
                        break;
		}
                
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :   
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", srAccumRec->lnIPASS_DeductTotalCount);                   
        	                sprintf(szTemplate, "%lld", srAccumRec->llIPASS_DeductTotalAmount);
        	                break;
        	        case _TDT_INDEX_01_ECC_ :   
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", srAccumRec->lnEASYCARD_DeductTotalCount);                   
        	                sprintf(szTemplate, "%lld", srAccumRec->llEASYCARD_DeductTotalAmount);
        	                break;
        	        case _TDT_INDEX_02_ICASH_ :   
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", srAccumRec->lnICASH_DeductTotalCount);                   
        	                sprintf(szTemplate, "%lld", srAccumRec->llICASH_DeductTotalAmount);
        	                break;
        	        default :
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", 0l);                   
        	                sprintf(szTemplate, "%lld", 0ll);
        	                break;        
        	}
        	
        	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
        	strcat(szPrintBuf, szTemplate);
        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		
        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", srAccumRec->lnIPASS_RefundTotalCount);                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_RefundTotalAmount));
        	                break;
        	        case _TDT_INDEX_01_ECC_ :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", srAccumRec->lnEASYCARD_RefundTotalCount);                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_RefundTotalAmount));
        	                break;
        	        case _TDT_INDEX_02_ICASH_ :   
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", srAccumRec->lnICASH_RefundTotalCount);                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llICASH_RefundTotalAmount));
        	                break;
        	        default :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", 0l);                   
        	                sprintf(szTemplate, "%lld", 0ll);
        	                break;        
        	}
		
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
        	strcat(szPrintBuf, szTemplate);
        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	
		/* 加值功能有開且有交易筆數 */
        	if (szTxnType[4] == 0x59 && 
		   ((i == _TDT_INDEX_00_IPASS_ && srAccumRec->lnIPASS_ADDTotalCount > 0)	||
		    (i == _TDT_INDEX_01_ECC_ && srAccumRec->lnEASYCARD_ADDTotalCount > 0)	||
		    (i == _TDT_INDEX_02_ICASH_ && srAccumRec->lnICASH_ADDTotalCount > 0)))
        	{
        	        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	
                	switch(i)
                        {
                                case _TDT_INDEX_00_IPASS_ :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", srAccumRec->lnIPASS_ADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_ADDTotalAmount));
                	                break;
                	        case _TDT_INDEX_01_ECC_ :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", srAccumRec->lnEASYCARD_ADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_ADDTotalAmount));
                	                break;
                	        case _TDT_INDEX_02_ICASH_ :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", srAccumRec->lnICASH_ADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llICASH_ADDTotalAmount));
                	                break;
                	        default :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", 0l);                   
                	                sprintf(szTemplate, "%lld", 0ll);
                	                break;        
                	}
                	
                	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	}
        	
		/* 加值取消功能有開且有交易筆數 */
        	if (szTxnType[5] == 0x59	&& 
		   ((i == _TDT_INDEX_00_IPASS_ && srAccumRec->lnIPASS_VoidADDTotalCount > 0)	||
		    (i == _TDT_INDEX_01_ECC_ && srAccumRec->lnEASYCARD_VoidADDTotalCount > 0)   ||
		    (i == _TDT_INDEX_02_ICASH_ && srAccumRec->lnICASH_VoidADDTotalCount > 0)))
        	{
                	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                	switch(i)
                        {
                                case _TDT_INDEX_00_IPASS_ :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", srAccumRec->lnIPASS_VoidADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_VoidADDTotalAmount));
                	                break;
                	        case _TDT_INDEX_01_ECC_ :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", srAccumRec->lnEASYCARD_VoidADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_VoidADDTotalAmount));
                	                break;
                	        case _TDT_INDEX_02_ICASH_ :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", srAccumRec->lnICASH_VoidADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llICASH_VoidADDTotalAmount));
                	                break;
                	        default :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", 0l);                   
                	                sprintf(szTemplate, "%lld", 0ll);
                	                break;        
                	}
                	
                	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                }
                	
        	inPRINT_Buffer_PutIn("------------------------------------------", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		
        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", (srAccumRec->lnIPASS_DeductTotalCount + srAccumRec->lnIPASS_RefundTotalCount));                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_DeductTotalAmount - srAccumRec->llIPASS_RefundTotalAmount));
        	                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
                        	strcat(szPrintBuf, szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
        	        case _TDT_INDEX_01_ECC_ :
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", (srAccumRec->lnEASYCARD_DeductTotalCount + srAccumRec->lnEASYCARD_RefundTotalCount));                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_DeductTotalAmount - srAccumRec->llEASYCARD_RefundTotalAmount));
        	                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
                        	strcat(szPrintBuf, szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
        	        case _TDT_INDEX_02_ICASH_ :
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", (srAccumRec->lnICASH_DeductTotalCount + srAccumRec->lnICASH_RefundTotalCount));                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llICASH_DeductTotalAmount - srAccumRec->llICASH_RefundTotalAmount));
        	                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
                        	strcat(szPrintBuf, szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
        	        default :
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", 0l);                   
        	                sprintf(szTemplate, "%lld", 0ll);
        	                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
                        	strcat(szPrintBuf, szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;        
        	}
        	
		if ((i == _TDT_INDEX_00_IPASS_ && srAccumRec->lnIPASS_ADDTotalCount > 0)	||
		    (i == _TDT_INDEX_01_ECC_ && srAccumRec->lnEASYCARD_ADDTotalCount > 0)	||
		    (i == _TDT_INDEX_02_ICASH_ && srAccumRec->lnICASH_ADDTotalCount > 0))
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			switch(i)
			{
				case _TDT_INDEX_00_IPASS_ :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", (srAccumRec->lnIPASS_ADDTotalCount + srAccumRec->lnIPASS_VoidADDTotalCount));                   
					sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_ADDTotalAmount - srAccumRec->llIPASS_VoidADDTotalAmount));
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;
				case _TDT_INDEX_01_ECC_ :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", (srAccumRec->lnEASYCARD_ADDTotalCount + srAccumRec->lnEASYCARD_VoidADDTotalCount));                   
					sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_ADDTotalAmount - srAccumRec->llEASYCARD_VoidADDTotalAmount));
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;
				case _TDT_INDEX_02_ICASH_ :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", (srAccumRec->lnICASH_ADDTotalCount + srAccumRec->lnICASH_VoidADDTotalCount));                   
					sprintf(szTemplate, "%lld", (srAccumRec->llICASH_ADDTotalAmount - srAccumRec->llICASH_VoidADDTotalAmount));
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;
				default :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", 0l);                   
					sprintf(szTemplate, "%lld", 0ll);
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;        
			}
		}
        	
        	inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        }
        
        inPRINT_Buffer_PutIn("==========================================", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
	
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmountByCard_ByBuffer_ESVC_Settle_046
Date&Time       :2022/4/26 下午 3:53
Describe        :依卡別列印
*/
int inCREDIT_PRINT_TotalAmountByCard_ByBuffer_046_ESVC_Settle(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int		i = 0;
	char		szPrintBuf[100 + 1], szTemplate[42 + 1];
	char		szTxnType[20 + 1];
	char		szTicketNeedNewBatch[2 + 1] = {0};
	char		szSpecialShopID[50 + 1] = {0};
	unsigned char	uszNeedPrintBit = VS_FALSE;
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);
	
	/* 先檢查是否任一票證有開 */
	for (i = 0; i < 4; i++)
	{
		if (inLoadTDTRec(i) != VS_SUCCESS)
		{
			break;
		}
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTicket_HostEnable(szTemplate);
                if (memcmp(szTemplate, "Y", strlen("Y")) != 0)
		{
                        continue;
		}
		else
		{
			uszNeedPrintBit = VS_TRUE;
		}
	}
	
	/* 代表有任一票證要印 */
	if (uszNeedPrintBit == VS_TRUE)
	{
		inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("         卡別小計", _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	else
	{
		return (VS_SUCCESS);
	}
	
        for (i = 0; i < 4; i++)
        {
                if (inLoadTDTRec(i) != VS_SUCCESS)
		{
			break;
		}
		
                memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTicket_HostEnable(szTemplate);
                if (memcmp(szTemplate, "Y", strlen("Y")) != 0)
		{
                        continue;
		}
                
                memset(szTxnType, 0x00, sizeof(szTxnType));
		inGetTicket_HostTransFunc(szTxnType);
		        
                switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :
        	                inPRINT_Buffer_PutIn("卡別 一卡通", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
				
				memset(szSpecialShopID, 0x00, sizeof(szSpecialShopID));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetIPASS_System_ID(szTemplate);
				memcpy(&szSpecialShopID[0], szTemplate, 2);

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetIPASS_SP_ID(szTemplate);
				memcpy(&szSpecialShopID[2], szTemplate, 2);

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetIPASS_Sub_Company_ID(szTemplate);
				memcpy(&szSpecialShopID[4], szTemplate, 4);
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "特店代號%s", szSpecialShopID);
				inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_DEFINE_X_01_);
				
        	                break;
        	        case _TDT_INDEX_01_ECC_ :
        	                inPRINT_Buffer_PutIn("卡別 悠遊卡", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
          	                
				memset(szSpecialShopID, 0x00, sizeof(szSpecialShopID));
				inGetECC_New_SP_ID(szSpecialShopID);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "特店代號%s", szSpecialShopID);
				inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_DEFINE_X_01_);
				
				/* 二代設備編號 */
          	                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetTicket_Device2(szTemplate);
                        	sprintf(szPrintBuf, "二代設備編號　 %s", szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                        	
				/* 悠遊卡批次號碼 */
                        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTicketNeedNewBatch, 0x00, sizeof(szTicketNeedNewBatch));
				inGetTicket_NeedNewBatch(szTicketNeedNewBatch);
				if (memcmp(szTicketNeedNewBatch, "Y", strlen("Y")) == 0)
				{
					sprintf(szTemplate, "%.4s%.2s%.2s", &pobTran->srBRec.szDate[2], &pobTran->srBRec.szDate[6], _ECC_DEFAULT_BATCH_);
				}
				else
				{
					inGetTicket_Batch(szTemplate);
				}
                        	sprintf(szPrintBuf, "悠遊卡批次號碼 %s", szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
				/* 列印交易時間 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szPrintBuf, "交易時間　　　 %.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        	                break;
        	        case _TDT_INDEX_02_ICASH_ :
        	                inPRINT_Buffer_PutIn("卡別 愛金卡", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
				
				/* 特店代號 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "門市代碼%s", "");
				inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetICASH_Shop_ID(szTemplate);
				memcpy(&szPrintBuf[0], szTemplate, 8);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
        	                break; 
        	        default :
        	                break;       
        	}

		/* 不合法 跳出 */
                if (i != _TDT_INDEX_00_IPASS_	&&
		    i != _TDT_INDEX_01_ECC_	&&
		    i != _TDT_INDEX_02_ICASH_)
		{
                        break;
		}
                
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :   
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", srAccumRec->lnIPASS_DeductTotalCount);                   
        	                sprintf(szTemplate, "%lld", srAccumRec->llIPASS_DeductTotalAmount);
        	                break;
        	        case _TDT_INDEX_01_ECC_ :   
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", srAccumRec->lnEASYCARD_DeductTotalCount);                   
        	                sprintf(szTemplate, "%lld", srAccumRec->llEASYCARD_DeductTotalAmount);
        	                break;
        	        case _TDT_INDEX_02_ICASH_ :   
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", srAccumRec->lnICASH_DeductTotalCount);                   
        	                sprintf(szTemplate, "%lld", srAccumRec->llICASH_DeductTotalAmount);
        	                break;
        	        default :
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", 0l);                   
        	                sprintf(szTemplate, "%lld", 0ll);
        	                break;        
        	}
        	
        	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
        	strcat(szPrintBuf, szTemplate);
        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		
        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", srAccumRec->lnIPASS_RefundTotalCount);                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_RefundTotalAmount));
        	                break;
        	        case _TDT_INDEX_01_ECC_ :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", srAccumRec->lnEASYCARD_RefundTotalCount);                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_RefundTotalAmount));
        	                break;
                        case _TDT_INDEX_02_ICASH_ :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", srAccumRec->lnICASH_RefundTotalCount);                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llICASH_RefundTotalAmount));
        	                break;
        	        default :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", 0l);                   
        	                sprintf(szTemplate, "%lld", 0ll);
        	                break;        
        	}
		
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
        	strcat(szPrintBuf, szTemplate);
        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	
		/* 加值功能有開且有交易筆數 */
        	if (szTxnType[4] == 0x59  && 
		   ((i == _TDT_INDEX_00_IPASS_ && srAccumRec->lnIPASS_ADDTotalCount > 0)	||
		    (i == _TDT_INDEX_01_ECC_ && srAccumRec->lnEASYCARD_ADDTotalCount > 0)	||
		    (i == _TDT_INDEX_02_ICASH_ && srAccumRec->lnICASH_ADDTotalCount > 0)))
        	{
        	        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	
                	switch(i)
                        {
                                case _TDT_INDEX_00_IPASS_ :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", srAccumRec->lnIPASS_ADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_ADDTotalAmount));
                	                break;
                	        case _TDT_INDEX_01_ECC_ :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", srAccumRec->lnEASYCARD_ADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_ADDTotalAmount));
                	                break;
                                case _TDT_INDEX_02_ICASH_ :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", srAccumRec->lnICASH_ADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llICASH_ADDTotalAmount));
                	                break;				
                	        default :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", 0l);                   
                	                sprintf(szTemplate, "%lld", 0ll);
                	                break;        
                	}
                	
                	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	}
        	
		/* 加值取消功能有開且有交易筆數 */
        	if (szTxnType[5] == 0x59  && 
		   ((i == _TDT_INDEX_00_IPASS_ && srAccumRec->lnIPASS_VoidADDTotalCount > 0)	||
		    (i == _TDT_INDEX_01_ECC_ && srAccumRec->lnEASYCARD_VoidADDTotalCount > 0)	||
		    (i == _TDT_INDEX_02_ICASH_ && srAccumRec->lnICASH_VoidADDTotalCount > 0)))
        	{
                	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                	switch(i)
                        {
                                case _TDT_INDEX_00_IPASS_ :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", srAccumRec->lnIPASS_VoidADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_VoidADDTotalAmount));
                	                break;
                	        case _TDT_INDEX_01_ECC_ :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", srAccumRec->lnEASYCARD_VoidADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_VoidADDTotalAmount));
                	                break;
                                case _TDT_INDEX_02_ICASH_ :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", srAccumRec->lnICASH_VoidADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llICASH_VoidADDTotalAmount));
                	                break;
                	        default :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", 0l);                   
                	                sprintf(szTemplate, "%lld", 0ll);
                	                break;        
                	}
                	
                	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                }
                	
        	inPRINT_Buffer_PutIn("------------------------------------------", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		
        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", (srAccumRec->lnIPASS_DeductTotalCount + srAccumRec->lnIPASS_RefundTotalCount));                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_DeductTotalAmount - srAccumRec->llIPASS_RefundTotalAmount));
        	                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
                        	strcat(szPrintBuf, szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
        	        case _TDT_INDEX_01_ECC_ :
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", (srAccumRec->lnEASYCARD_DeductTotalCount + srAccumRec->lnEASYCARD_RefundTotalCount));                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_DeductTotalAmount - srAccumRec->llEASYCARD_RefundTotalAmount));
        	                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
                        	strcat(szPrintBuf, szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
                        case _TDT_INDEX_02_ICASH_ :
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", (srAccumRec->lnICASH_DeductTotalCount + srAccumRec->lnICASH_RefundTotalCount));                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llICASH_DeductTotalAmount - srAccumRec->llICASH_RefundTotalAmount));
        	                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
                        	strcat(szPrintBuf, szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
        	        default :
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", 0l);                   
        	                sprintf(szTemplate, "%lld", 0ll);
        	                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
                        	strcat(szPrintBuf, szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;        
        	}
        	
		if ((i == _TDT_INDEX_00_IPASS_ && srAccumRec->lnIPASS_ADDTotalCount > 0)	||
		    (i == _TDT_INDEX_01_ECC_ && srAccumRec->lnEASYCARD_ADDTotalCount > 0)	||
		    (i == _TDT_INDEX_02_ICASH_ && srAccumRec->lnICASH_ADDTotalCount > 0))
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			switch(i)
			{
				case _TDT_INDEX_00_IPASS_ :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", (srAccumRec->lnIPASS_ADDTotalCount + srAccumRec->lnIPASS_VoidADDTotalCount));                   
					sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_ADDTotalAmount - srAccumRec->llIPASS_VoidADDTotalAmount));
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;
				case _TDT_INDEX_01_ECC_ :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", (srAccumRec->lnEASYCARD_ADDTotalCount + srAccumRec->lnEASYCARD_VoidADDTotalCount));                   
					sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_ADDTotalAmount - srAccumRec->llEASYCARD_VoidADDTotalAmount));
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;
				case _TDT_INDEX_02_ICASH_ :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", (srAccumRec->lnICASH_ADDTotalCount + srAccumRec->lnICASH_VoidADDTotalCount));                   
					sprintf(szTemplate, "%lld", (srAccumRec->llICASH_ADDTotalAmount - srAccumRec->llICASH_VoidADDTotalAmount));
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;
				default :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", 0l);                   
					sprintf(szTemplate, "%lld", 0ll);
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;        
			}
		}
		
        	inPRINT_Buffer_PutIn(" ", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        }
        
        inPRINT_Buffer_PutIn("==========================================", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
	
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TIDMID_ByBuffer_046
Date&Time       :2022/4/25 下午 4:30
Describe        :列印TID & MID
*/
int inCREDIT_PRINT_Tidmid_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        int     inRetVal;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);

	do
	{
		if (inPrinttype_ByBuffer)
		{
			/* 直式 */
			/* Get商店代號 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetMerchantID(szTemplate);

			/* 列印商店代號 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "商店代號：%s", szTemplate);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* Get端末機代號 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetTerminalID(szTemplate);

			/* 列印端末機代號 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "端末機代號：%s", szTemplate);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;
		}
		else
		{
			/* 橫式 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetMerchantID(szTemplate);

			/* 列印商店代號 */
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_LEFT_);
			sprintf(szPrintBuf, "商店代號 %s", szTemplate);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* Get端末機代號 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetTerminalID(szTemplate);

			/* 列印端末機代號 */
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 13, _PADDING_LEFT_);
			sprintf(szPrintBuf, "端末機代號 %s", szTemplate);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			inRetVal = inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;
		}
		break;
	}while(1);
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_Data_ByBuffer_046
Date&Time       :2022/4/25 下午 5:33
Describe        :列印DATA
*/
int inCREDIT_PRINT_Data_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
        int     inRetVal;
        char 	szPrintBuf[100 + 1], szPrintBuf1[42 + 1], szPrintBuf2[42 + 1], szTemplate1[42 + 1], szTemplate2[42 + 1];
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
	char	szExamBit[10 + 1] = {0};
	unsigned char	uszChangeLineBit = VS_FALSE;

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	do
	{
		if (inPrinttype_ByBuffer)
		{
			/* 直式 */
			/*卡別、卡號*/
			/* 【需求單 - 106349】自有品牌判斷需求 */
			/* unpack電文就存存CardLabel modify by LingHsiung 2020/2/14 上午 11:07 */
			sprintf(szPrintBuf, "卡別　　：%s", pobTran->srBRec.szCardLabel);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "卡號　　：%s", pobTran->srBRec.szPAN);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/*日期、時間*/
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "日期　　：%s",pobTran->srBRec.szDate);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "時間　　：%s",pobTran->srBRec.szTime);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/*調閱編號、批次號碼 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "調閱編號：%06ld",pobTran->srBRec.lnOrgInvNum);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "批次號碼：%06ld",pobTran->srBRec.lnBatchNum);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/*交易類別*/
			inFunc_GetTransType(pobTran, szPrintBuf1, szTemplate2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "交易類別：%s",szPrintBuf1);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/*授權碼、序號*/
			sprintf(szPrintBuf, "授權碼　：%s",pobTran->srBRec.szAuthCode);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "序號　　：%s",pobTran->srBRec.szRefNo);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		}
		else
		{
			/* 橫式 */

			/* 城市別(City) */
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("城市別(City)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
			inGetCityName(szPrintBuf1);
			sprintf(szPrintBuf, "%s", szPrintBuf1);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* 卡別 檢查碼 */
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("卡別(Card Type)", _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("檢查碼(Check No.)", _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			/* 卡別 */
			/* 【需求單 - 106349】自有品牌判斷需求 */
			/* unpack電文就存存CardLabel modify by LingHsiung 2020/2/14 上午 11:07 */
			inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 12, _PADDING_RIGHT_);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			/* 檢查碼 */
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
			/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
			if (strlen(pobTran->srBRec.szCheckNO) > 0)
			{
				strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
			}
			else
			{
				inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
			}
			memcpy(szPrintBuf1, szTemplate1, strlen(szTemplate1));
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* 卡號 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcpy(szPrintBuf, pobTran->srBRec.szPAN);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("卡號(Card No.)", _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcpy(szPrintBuf, pobTran->srBRec.szPAN);

			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			/* 卡號遮掩(一般卡號前6後4，U Card前3後5) */
			/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
			if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
			{
				/* Uny交易不受TMS開關限制，且獲取卡號時已遮掩 */
			}
			else
			{
				if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
				{
					/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
					for (i = 6; i < (strlen(szPrintBuf) - 4); i ++)
					{
						szPrintBuf[i] = 0x2A;
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
						for (i = 6; i < (strlen(szPrintBuf) - 4); i ++)
						{
							szPrintBuf[i] = 0x2A;
						}
					}

				}
			}

			/* 過卡方式 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
					strcat(szPrintBuf, "(W)");
				else
					strcat(szPrintBuf, "(C)");
			}
			else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
			{
				strcat(szPrintBuf, "(Q)");
			}
			else if (pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
			{
				strcat(szPrintBuf, "(Q)");
			}
			else
			{
				if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
					strcat(szPrintBuf,"(C)");
				else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
					strcat(szPrintBuf, "(T)");
				else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
					strcat(szPrintBuf, "(W)");
				else
				{
					if (pobTran->srBRec.uszManualBit == VS_TRUE)
					{
						/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
						/* 電文轉Manual Keyin但是簽單要印感應的W */
						if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
							strcat(szPrintBuf, "(W)");
						else
							strcat(szPrintBuf,"(M)");
					}
					else
						strcat(szPrintBuf,"(S)");
				}
			}
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* 主機別 & 交易別 */
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("交易類別(Trans.Type)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memset(szTemplate2, 0x00, sizeof(szTemplate2));
			inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));

			sprintf(szPrintBuf2, "%s", szTemplate1);
			sprintf(szPrintBuf, "%s", szPrintBuf2);

			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			if (strlen(szTemplate2) > 0)
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate2);
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
			}

			/* 批次號碼、授權碼 */
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("批次號碼(Batch No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("授權碼(Auth Code)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* Batch Num */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%03ld", pobTran->srBRec.lnBatchNum);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* Auth Code */
			memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
			memcpy(szPrintBuf1, pobTran->srBRec.szAuthCode, _AUTH_CODE_SIZE_);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* 回覆碼 */
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inRetVal = inPRINT_Buffer_PutIn("回覆碼(Resp. Code)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memcpy(szPrintBuf, pobTran->srBRec.szRespCode, 2);
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
			}

			/* 日期時間 */
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("日期/時間(Date/Time)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
			sprintf(szPrintBuf1, "%.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* 序號 調閱編號 */
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("序號(Ref. No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;
			
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("調閱編號(Inv.No)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;


			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			/* 雖然電文RRN送12個byte，但RRN最後一碼是0x00，所以只看到11碼 */
			inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szRefNo, ' ', 12, _PADDING_RIGHT_);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
			sprintf(szPrintBuf1, "%06ld", pobTran->srBRec.lnOrgInvNum);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* 櫃號 */
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			inGetStoreIDEnable(szTemplate1);
			if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
			{
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inRetVal = inPRINT_Buffer_PutIn("櫃號(Store ID)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;

				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szStoreID, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
			}

			/* 產品代碼 */
			inGetProductCodeEnable(szProductCodeEnable);
			if (memcmp(szProductCodeEnable, "Y", 1) == 0)
			{
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inRetVal = inPRINT_Buffer_PutIn("產品代碼(Product Code)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;

				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szProductCode, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
			}

			/* TC */
			/* 驗測開關 */
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
					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						break;
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
						inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							break;
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
						inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							break;
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
					     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE		|| 
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
							inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
							inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								break;
							
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
							inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								break;
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
								inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
								inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
								if (inRetVal != VS_SUCCESS)
									break;
							}

							/* 商店聯卡號遮掩 */
							memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
							inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
							if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
							    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
							    strlen(pobTran->srBRec.szTxnNo) > 0)
							{
								inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
								inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
								if (inRetVal != VS_SUCCESS)
									break;
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
								inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
								inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
								if (inRetVal != VS_SUCCESS)
									break;
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
										inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
										inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
										if (inRetVal != VS_SUCCESS)
											break;
										uszChangeLineBit = VS_TRUE;
									}
									else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0)
									{
										memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
										sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
										inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
										inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
										if (inRetVal != VS_SUCCESS)
											break;
										uszChangeLineBit = VS_TRUE;
									}
									/* 【需求單-109121】Mastercard交易簽單列印AID需求 by Russell 2020/7/23 下午 4:23 */
									else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
									{
										memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
										sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
										inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
										inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
										if (inRetVal != VS_SUCCESS)
											break;
										uszChangeLineBit = VS_TRUE;
									}
									/* 【需求單-111290】VISA置換Logo、DISCOVER簽單新增AID欄位、AE簽單新增AID和AP Label欄位需求 by Russell 2022/12/29 下午 5:57 */
									else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
										pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
									{
										memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
										sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
										inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
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
										inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
										inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
										if (inRetVal != VS_SUCCESS)
											break;
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
										inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
										inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
										if (inRetVal != VS_SUCCESS)
											break;
										uszChangeLineBit = VS_TRUE;
									}
									/* AE晶片要印出AID */
									else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY || 
										 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
									{
										memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
										sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
										inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
										inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
										if (inRetVal != VS_SUCCESS)
											break;
										uszChangeLineBit = VS_TRUE;
									}
									else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE || 
										 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
									{
										memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
										sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
										inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
										inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
										if (inRetVal != VS_SUCCESS)
											break;
										uszChangeLineBit = VS_TRUE;
									}
									else if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS	|| pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE) &&
										pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
									{
										memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
										sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
										inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
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
										inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
										inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
										if (inRetVal != VS_SUCCESS)
											break;
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
									inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
									inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
									if (inRetVal != VS_SUCCESS)
										break;
								}
								else
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AP Label:");
									inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
									inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
									if (inRetVal != VS_SUCCESS)
										break;
								}
							}
							/* M/C交易列印AP Lable (END) */
							else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
							{
								if (pobTran->srEMVRec.in50_APLabelLen > 0)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
									inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
									inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
								}
								else
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AP Label:");
									inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
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
								inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
								inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
								if (inRetVal != VS_SUCCESS)
									break;
							}
							/* 一定要換行 */
							else
							{
								if (uszChangeLineBit == VS_TRUE)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, " ");
									inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
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
								inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
								inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
								if (inRetVal != VS_SUCCESS)
									break;
								
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
								inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
								inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
								if (inRetVal != VS_SUCCESS)
									break;
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
						inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
						inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							break;
						
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
						inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							break;
					}
				}
			}

			/* 斷行 */
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;
		}
		break;
	}while(1);
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_CUP_AMOUNT_ByBuffer_046
Date&Time       :2022/4/25 下午 5:33
Describe        :列印銀聯AMOUNT
*/
int inCREDIT_PRINT_Cup_Amount_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
        char    szPrintBuf[84 + 1] = {0}, szTemplate[42 + 1] = {0};
	long    lnTempTxnAmt = 0;

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
	
	do
	{
		if (inPrinttype_ByBuffer)
		{
			/* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
			if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
			{
				lnTempTxnAmt = pobTran->srBRec.lnCUPUPlanDiscountedAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount;

			}
			else
			{
				lnTempTxnAmt = pobTran->srBRec.lnTxnAmount;
			}
		
			/* 直式 */
			if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
			{
				if (pobTran->srBRec.inCode == _CUP_REFUND_	||
			            pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
				{
					/* 初始化 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					sprintf(szTemplate, "NT$ %ld", lnTempTxnAmt);
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
					sprintf(szTemplate, "NT$ %ld", 0 - lnTempTxnAmt);
					inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PADDING_LEFT_ );

					/* 把前面的字串和數字結合起來 */
					sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
				}
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
				if (pobTran->srBRec.inCode == _CUP_REFUND_	||
			            pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
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
			}
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			for (i = 0; i < 2; i++)
			{
				inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}
		/* 橫式 */
		/* 負向交易 */
		else if(pobTran->srBRec.uszVOIDBit == VS_TRUE)
		{
			/* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
			if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
			{
				lnTempTxnAmt = pobTran->srBRec.lnCUPUPlanDiscountedAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount;

			}
			else
			{
				lnTempTxnAmt = pobTran->srBRec.lnTxnAmount;
			}
		
			/* 橫式 */
			/* 金額 */
			/* 取消退貨是正數 */
			if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
			    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  lnTempTxnAmt);
				inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", "總計(Total) :");
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  (0 - lnTempTxnAmt));
				inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", "總計(Total) :");
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
                        
                        /* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
                        if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
                        {
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
				    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
				{
					sprintf(szPrintBuf, "%ld", pobTran->srBRec.lnCUPUPlanDiscountedAmount);
				}
				else
				{
					sprintf(szPrintBuf, "%ld", 0 - pobTran->srBRec.lnCUPUPlanDiscountedAmount);
				}
                                inFunc_Amount_Comma(szPrintBuf, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                inPRINT_Buffer_PutIn("優惠後金額   : ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                                if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
				    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
				{
					sprintf(szPrintBuf, "%ld", pobTran->srBRec.lnCUPUPlanPreferentialAmount);
				}
				else
				{
					sprintf(szPrintBuf, "%ld", 0 - pobTran->srBRec.lnCUPUPlanPreferentialAmount);
				}
                                inFunc_Amount_Comma(szPrintBuf, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                inPRINT_Buffer_PutIn("優惠金額　   : ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                inPRINT_Buffer_PutIn("==============================================================================================================", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                                inPRINT_Buffer_PutIn("備註欄(Reference)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                inPRINT_Buffer_PutIn(pobTran->srBRec.szUPlan_RemarksInformation, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                                inPRINT_Buffer_PutIn("優惠券號(Coupon ID)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                inPRINT_Buffer_PutIn(pobTran->srBRec.szUPlan_Coupon, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        }
		}
		else
		{
			/* 初始化 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

			/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
			if (pobTran->srBRec.inCode == _CUP_REFUND_	||
			    pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
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
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Total) :");
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        
                        /* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
                        if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
                        {
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
				    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
				{
					sprintf(szPrintBuf, "%ld", 0 - (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount));
				}
				else
				{
					sprintf(szPrintBuf, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount));
				}
                                inFunc_Amount_Comma(szPrintBuf, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
                                inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
                                inPRINT_Buffer_PutIn("優惠前金額   : ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                                if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
				    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
				{
					sprintf(szPrintBuf, "%ld", 0 - pobTran->srBRec.lnCUPUPlanPreferentialAmount);
				}
				else
				{
					sprintf(szPrintBuf, "%ld", pobTran->srBRec.lnCUPUPlanPreferentialAmount);
				}
                                inFunc_Amount_Comma(szPrintBuf, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
                                inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
                                inPRINT_Buffer_PutIn("優惠金額　   : ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                                if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
				    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
				{
					sprintf(szPrintBuf, "%ld", 0 - pobTran->srBRec.lnCUPUPlanDiscountedAmount);
				}
				else
				{
					sprintf(szPrintBuf, "%ld", pobTran->srBRec.lnCUPUPlanDiscountedAmount);
				}
                                inFunc_Amount_Comma(szPrintBuf, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
                                inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
                                inPRINT_Buffer_PutIn("優惠後金額   : ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                inPRINT_Buffer_PutIn("==============================================================================================================", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                                inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
                                inPRINT_Buffer_PutIn("備註欄(Reference)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
                                inPRINT_Buffer_PutIn(pobTran->srBRec.szUPlan_RemarksInformation, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                                inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
                                inPRINT_Buffer_PutIn("優惠券號(Coupon ID)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
                                inPRINT_Buffer_PutIn(pobTran->srBRec.szUPlan_Coupon, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        }
		}

		/* 斷行 */
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 列印銀聯交易提示文字 */
		inPRINT_Buffer_PutGraphic((unsigned char*)_CUP_LEGAL_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inCupLegalHeight, _APPEND_);

		/* 斷行 */
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		break;
	}while(1);
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
	
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Amount_ByBuffer_046
Date&Time       :2022/4/26 下午 4:58
Describe        :列印AMOUNT
*/
int inCREDIT_PRINT_Amount_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
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
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


                        /* 小費 */
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTipTxnAmount);
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PADDING_LEFT_);

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "小費(Tips)  :%s", szTemplate);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


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
			  pobTran->srBRec.inCode == _REFUND_		|| 
			  pobTran->srBRec.inCode == _INST_REFUND_	|| 
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
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                
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
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
			
			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Total) :");
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
		else
		{
			/* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Total) :");
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
	}
	else
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTransFunc(szTemplate);
		if (szTemplate[6] == 'Y')	/* 檢查是否有開小費 */
		{
			/* 退貨金額為負數 */
			if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
			{
				 /* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
				inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", "總計(Total) :");
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			/* 預授不會有小費，所以拉出來 */
			else if (pobTran->srBRec.inCode == _PRE_AUTH_ || pobTran->srBRec.inCode == _PRE_COMP_)
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", "總計(Total) :");
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				/* 金額 */
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", "金額(Amount):");
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				
				/* lnTipAmount為0表示非小費 */
				if (pobTran->srBRec.lnTipTxnAmount == 0L)
				{
					/* 小費 */
					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
					inPRINT_Buffer_PutIn("小費(Tips)  :__________________________", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					/* 總計 */
					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
					inPRINT_Buffer_PutIn("總計(Total) :__________________________", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				else
				{
					/* 小費 */
					/* 初始化 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTipTxnAmount);
					inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", szTemplate);
					inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					/* 把前面的字串和數字結合起來 */
					sprintf(szPrintBuf, "小費(Tips)  :");
					inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


					/* 總計 */
					/* 初始化 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					sprintf(szTemplate, "%ld",  (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
					inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", szTemplate);
					inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", "總計(Total) :");
					inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				
			}
			
		}
		/* 小費沒開時 */
		else
		{
			 /* 初始化 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

			/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
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
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Total) :");
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		}
		
	}
	/* 斷行 */
	inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
	
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_INST_ByBuffer_046
Date&Time       :2022/4/26 下午 5:35
Describe        :列印分期
*/
int inCREDIT_PRINT_Inst_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char    szPrintBuf[84 + 1], szTemplate[42 + 1], szTemplate1[42 + 1];
	
	if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		
		/* 分期期數 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%02ld", pobTran->srBRec.lnInstallmentPeriod);
		inFunc_Amount_Comma(szTemplate1, "" , ' ', _SIGNED_NONE_, 18, _PADDING_LEFT_);
		sprintf(szTemplate, "%s  ", szTemplate1);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "期");
		inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				
		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "分期期數   :");
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 首期金額 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (pobTran->srBRec.lnTipTxnAmount > 0L)
			sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentDownPayment + pobTran->srBRec.lnTipTxnAmount));
		else
			sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentDownPayment));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, _PADDING_LEFT_);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "首期金額   :NT$ ");
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 每期金額 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentPayment));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, _PADDING_LEFT_);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "每期金額   :NT$ ");
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 分期手續費 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentFormalityFee));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 15, _PADDING_LEFT_);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "分期手續費 :NT$ ");
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		/* 斷行 */
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
		
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_REDEEM_ByBuffer_046
Date&Time       :2022/4/26 下午 5:35
Describe        :列印紅利
*/
int inCREDIT_PRINT_Redeem_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szPrintBuf[84 + 1], szTemplate[42 + 1], szTemplate1[42 + 1];
	
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
		{
			if (pobTran->srBRec.inOrgCode == _REDEEM_SALE_ || pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_)
			{
				/* 支付金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount));
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_MINUS_, 13, _PADDING_LEFT_);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "支付金額　　 :NT$");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", ((pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount)));
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_MINUS_, 13, _PADDING_LEFT_);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵金額 :NT$");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵點數 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "-%ld", (pobTran->srBRec.lnRedemptionPoints));
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 16, _PADDING_LEFT_);
				sprintf(szTemplate, "%s  ",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "點");
				inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵點數 :");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else if(pobTran->srBRec.inOrgCode == _REDEEM_REFUND_)
			{
				/* 支付金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnRedemptionPaidCreditAmount);
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "支付金額　　 :NT$");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount));
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵金額 :NT$");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵點數 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnRedemptionPoints);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 16, _PADDING_LEFT_);
				sprintf(szTemplate, "%s  ",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "點");
				inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵點數 :");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}
		else
		{
			if (pobTran->srBRec.inCode == _REDEEM_REFUND_)
			{
				/* 支付金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (pobTran->srBRec.lnRedemptionPaidCreditAmount));
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_MINUS_, 13, _PADDING_LEFT_);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "支付金額　　 :NT$");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", ((pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount)));
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_MINUS_, 13, _PADDING_LEFT_);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵金額 :NT$");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵點數 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "-%ld", (pobTran->srBRec.lnRedemptionPoints));
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 16, _PADDING_LEFT_);
				sprintf(szTemplate, "%s  ",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "點");
				inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵點數 :");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else if (pobTran->srBRec.inCode == _REDEEM_ADJUST_)
			{
				/* 支付金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnRedemptionPaidCreditAmount);
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "支付金額　　 :NT$");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount));
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵金額 :NT$");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵點數 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnRedemptionPoints);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 16, _PADDING_LEFT_);
				sprintf(szTemplate, "%s  ",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "點");
				inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵點數 :");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				/* 支付金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount);
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "支付金額　　 :NT$");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount));
				inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵金額 :NT$");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵點數 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnRedemptionPoints);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 16, _PADDING_LEFT_);
				sprintf(szTemplate, "%s  ",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "點");
				inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵點數 :");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利剩餘點數 */
				if (pobTran->inTransactionCode == _REDEEM_SALE_ || pobTran->srBRec.inCode == _REDEEM_SALE_)
				{
					/* 紅利扣抵才印 */
					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memset(szTemplate1, 0x00, sizeof(szTemplate1));
					sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnRedemptionPointsBalance));
					inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 16, _PADDING_LEFT_);
					sprintf(szTemplate, "%s  ",szTemplate1);
					inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
					inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "點");
					inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
					
					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "紅利剩餘點數 :");
					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}

			}

		}

		/* 斷行 */
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
		
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_LEGAL_046
Date&Time       :2022/6/6 下午 1:48
Describe        :借用other區快來列印警語
*/
int inCREDIT_PRINT_LEGAL_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		/* 斷行 */
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 分期警語*/
		inPRINT_Buffer_PutGraphic((unsigned char*)_LEGAL_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inInstHeight, _APPEND_);

		/* 斷行 */
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
		
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_DCC_Data_ByBuffer_046
Date&Time       :2022/4/26 下午 5:51
Describe        :
*/
int inCREDIT_PRINT_DCC_Data_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
	int     inRetVal;
        char 	szPrintBuf[84 + 1], szPrintBuf1[42 + 1], szPrintBuf2[42 + 1], szTemplate1[42 + 1], szTemplate2[42 + 1];
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
	char	szExamBit[10 + 1] = {0};
	unsigned char	uszChangeLineBit = VS_FALSE;

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	
	do
	{
		if (inPrinttype_ByBuffer)
		{
			/* 直式 */

			/*卡別、卡號*/
			sprintf(szPrintBuf, "卡別　　：%s", pobTran->srBRec.szCardLabel);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "卡號　　：%s", pobTran->srBRec.szPAN);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/*日期、時間*/
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "日期　　：%s",pobTran->srBRec.szDate);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "時間　　：%s",pobTran->srBRec.szTime);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/*調閱編號、批次號碼 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "調閱編號：%06ld",pobTran->srBRec.lnOrgInvNum);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "批次號碼：%06ld",pobTran->srBRec.lnBatchNum);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/*交易類別*/
			inFunc_GetTransType(pobTran, szPrintBuf1, szTemplate2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "交易類別：%s",szPrintBuf1);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/*授權碼、序號*/
			sprintf(szPrintBuf, "授權碼　：%s",pobTran->srBRec.szAuthCode);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "序號　　：%s",pobTran->srBRec.szRefNo);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		}
		else
		{
			/* 橫式 */

			/* 城市別(City) */
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("城市別(City)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
			inGetCityName(szPrintBuf1);
			sprintf(szPrintBuf, "%s", szPrintBuf1);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* 卡別 檢查碼 */
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("卡別(Card Type)", _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;
			
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("檢查碼(Check No.)", _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;


			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
			/* 卡別 */
			inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 12, _PADDING_RIGHT_);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;
			/* 檢查碼 */
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			if (strlen(pobTran->srBRec.szCheckNO) > 0)
			{
				strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
			}
			else
			{
				inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
			}
			memcpy(&szPrintBuf1[0], szTemplate1, strlen(szTemplate1));
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* 卡號 */
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("卡號(Card No.)", _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcpy(szPrintBuf, pobTran->srBRec.szPAN);

			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			/* 卡號遮掩(一般卡號前6後4，U Card前3後5) */
			if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				for (i = 6; i < (strlen(szPrintBuf) - 4); i ++)
					szPrintBuf[i] = 0x2A;
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
					for (i = 6; i < (strlen(szPrintBuf) - 4); i ++)
						szPrintBuf[i] = 0x2A;
				}

			}

			/* 過卡方式 */
			if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
			{
				if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
					strcat(szPrintBuf,"(C)");
				else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
					strcat(szPrintBuf, "(T)");
				else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
					strcat(szPrintBuf, "(W)");
				else
				{
					if (pobTran->srBRec.uszManualBit == VS_TRUE)
					{
						/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
						/* 電文轉Manual Keyin但是簽單要印感應的W */
						if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
							strcat(szPrintBuf, "(W)");
						else
							strcat(szPrintBuf,"(M)");
					}
					else
						strcat(szPrintBuf,"(S)");
				}

			}
			else
			{
				if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
					strcat(szPrintBuf, "(W)");
				else
					strcat(szPrintBuf, "(C)");
			}
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* 交易別 */
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("交易類別(Trans.Type)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memset(szTemplate2, 0x00, sizeof(szTemplate2));
			inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));

			sprintf(szPrintBuf2, "%s", szTemplate1);
			sprintf(szPrintBuf, "%s", szPrintBuf2);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			if (strlen(szTemplate2) > 0)
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate2);
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
			}


			/* 批次號碼、授權碼 */
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("批次號碼(Batch No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("授權碼(Auth Code)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* Batch Num */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%03ld", pobTran->srBRec.lnBatchNum);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* Auth Code */
			memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
			memcpy(szPrintBuf1, pobTran->srBRec.szAuthCode, _AUTH_CODE_SIZE_);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* 日期時間 */
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("日期/時間(Date/Time)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
			sprintf(szPrintBuf1, "%.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* 序號 調閱編號 */
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("序號(Ref. No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;
			
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("調閱編號(Inv.No)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;


			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			/* 雖然電文RRN送12個byte，但RRN最後一碼是0x00，所以只看到11碼 */
			inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szRefNo, ' ', 12, _PADDING_RIGHT_);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
			sprintf(szPrintBuf1, "%06ld", pobTran->srBRec.lnOrgInvNum);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* 櫃號 */
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			inGetStoreIDEnable(szTemplate1);
			if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
			{
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inRetVal = inPRINT_Buffer_PutIn("櫃號(Store ID)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;

				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szStoreID, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
			}

			/* 產品代碼 */
			inGetProductCodeEnable(szProductCodeEnable);
			if (memcmp(szProductCodeEnable, "Y", 1) == 0)
			{
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inRetVal = inPRINT_Buffer_PutIn("產品代碼(Product Code)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;

				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szProductCode, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
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
						inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							break;
					}

					/* 商店聯卡號遮掩 */
					memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
					inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
					if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
					    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
					    strlen(pobTran->srBRec.szTxnNo) > 0)
					{
						inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
						inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							break;
						
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
						inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							break;
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
								inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
								inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
								if (inRetVal != VS_SUCCESS)
									break;
								uszChangeLineBit = VS_TRUE;
							}
							else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
								inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
								inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
								if (inRetVal != VS_SUCCESS)
									break;
								uszChangeLineBit = VS_TRUE;
							}
							/* 【需求單-111290】VISA置換Logo、DISCOVER簽單新增AID欄位、AE簽單新增AID和AP Label欄位需求 by Russell 2022/12/29 下午 5:57 */
							else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
								pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
								inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
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
								inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
								inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
								if (inRetVal != VS_SUCCESS)
									break;
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
								inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
								inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
								if (inRetVal != VS_SUCCESS)
									break;
								uszChangeLineBit = VS_TRUE;
							}
							else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE || 
								 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
								inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
								inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
								if (inRetVal != VS_SUCCESS)
									break;
								uszChangeLineBit = VS_TRUE;
							}
							else if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS	|| pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE) &&
								pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
								inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
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
								inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
								inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
								if (inRetVal != VS_SUCCESS)
									break;
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
							inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
							inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
							if (inRetVal != VS_SUCCESS)
								break;
						}
						else
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AP Label:");
							inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
							inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
							if (inRetVal != VS_SUCCESS)
								break;
						}
					}
					/* M/C交易列印AP Lable (END) */
					else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
					{
						if (pobTran->srEMVRec.in50_APLabelLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
							inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
							inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
						else
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AP Label:");
							inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
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
						inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
						inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
						if (inRetVal != VS_SUCCESS)
							break;
					}
					/* 一定要換行 */
					else
					{
						if (uszChangeLineBit == VS_TRUE)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, " ");
							inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
							inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
					}

					/* 銀聯沒有DCC */
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
						inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
						inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							break;
						
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
						inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							break;
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
					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
					inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						break;
					
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						break;
				}
			}

			/* 斷行 */
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;
		}
		break;
	}while(1);
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_DCC_ReceiptEND_ByBuffer_046
Date&Time       :2022/5/4 下午 3:08
Describe        :列印結尾
 *		 046的結尾條文要hardcode
*/
int inCREDIT_PRINT_DCC_ReceiptEND_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
	int	inRetVal = VS_ERROR;
	char	szTemplate[42 + 1] = {0};
	char	szSignature[30 + 1] = {0};
	char	szSignaturePath[80 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	char	szCustomIndicator[3 + 1] = {0};
	
	memset(szCustomIndicator, 0x00, sizeof(szCustomIndicator));
	inGetCustomIndicator(szCustomIndicator);
	
        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                inPRINT_Buffer_PutIn("簽名欄:_____________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
		    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
                {
                        inPRINT_Buffer_PutIn("*** 商店收據 Merchant Copy ***", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_CUST_;

                }
                else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
                {
                        inPRINT_Buffer_PutIn("*** 持卡人收據 Customer Copy ***", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_MERCH_;
                }

                inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
        }
        else
        {
		/* 免責宣言上需空一行 by Russell 2020/10/29 下午 3:21 */
		inRetVal = inPRINT_Buffer_PutIn("  ", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                /* 橫式 */
                if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
                {
			
			/* Disclaimer */
			/* 轉台幣也要印 */
			if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_))) && 
			    (pobTran->srBRec.inCode == _SALE_		|| 
			     pobTran->srBRec.inCode == _TIP_		|| 
			     pobTran->srBRec.inCode == _PRE_COMP_	||
			    (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
			{
				if (pobTran->srBRec.inCode == _TIP_)
				{
					inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(_NCCC_DCC_FILE_VISA_DISCLAIMER_, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle);
				}
				else
				{
					inCREDIT_PRINT_DCC_Disclaimer_ByBuffer_046(_PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle);
				}
			}
			else if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_))) &&
				 (pobTran->srBRec.inCode == _SALE_	|| 
				  pobTran->srBRec.inCode == _TIP_	|| 
				  pobTran->srBRec.inCode == _PRE_COMP_	||
			         (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
			{
				if (pobTran->srBRec.inCode == _TIP_)
				{
					inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(_NCCC_DCC_FILE_MASTERCARD_DISCLAIMER_, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle);
				}
				else
				{
					inCREDIT_PRINT_DCC_Disclaimer_ByBuffer_046(_PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle);
				}
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
			
                        /* 簽名欄 */
			/* 免簽名 */
			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
			{
				/* 免簽名 */
				inRetVal = inPRINT_Buffer_PutIn("免      簽      名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
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
				/* 圖檔存在、有在signpad簽名、且非重印（重印不出簽名） （目前簽名狀態存不了Batch，先把&& pobTran->srBRec.inSignStatus == _SIGN_SIGNED_此條件拿掉） */
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
						inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					}
					
				}
				
			}
                        
                        if (!memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
			    !memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn("X:________________________________", _PRT_DUTY_FREE_075_X_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			}
			else
			{
				inPRINT_Buffer_PutIn("X:________________________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			/* 持卡人姓名 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%s", pobTran->srBRec.szCardHolder);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        inPRINT_Buffer_PutIn("持卡人簽名(特店存根聯)", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
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
				if (pobTran->srBRec.inCode == _TIP_)
				{
					inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(_NCCC_DCC_FILE_VISA_DISCLAIMER_, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle);
				}
				else
				{
					inCREDIT_PRINT_DCC_Disclaimer_ByBuffer_046(_PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle);
				}
			}
			else if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_))) && 
			    (pobTran->srBRec.inCode == _SALE_		|| 
			     pobTran->srBRec.inCode == _TIP_		|| 
			     pobTran->srBRec.inCode == _PRE_COMP_	||
			    (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
			{
				if (pobTran->srBRec.inCode == _TIP_)
				{
					inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(_NCCC_DCC_FILE_MASTERCARD_DISCLAIMER_, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle);
				}
				else
				{
					inCREDIT_PRINT_DCC_Disclaimer_ByBuffer_046(_PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle);
				}
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
				inRetVal = inPRINT_Buffer_PutIn("免      簽      名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        inPRINT_Buffer_PutIn("　　　　　　　　　 商店存根", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 持卡人姓名 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate,"%s",pobTran->srBRec.szCardHolder);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
				if (pobTran->srBRec.inCode == _TIP_)
				{
					inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(_NCCC_DCC_FILE_VISA_DISCLAIMER_, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle);
				}
				else
				{
					inCREDIT_PRINT_DCC_Disclaimer_ByBuffer_046(_PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle);
				}
			}
			else if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_))) && 
			    (pobTran->srBRec.inCode == _SALE_		|| 
			     pobTran->srBRec.inCode == _TIP_		|| 
			     pobTran->srBRec.inCode == _PRE_COMP_	||
			    (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
			{
				if (pobTran->srBRec.inCode == _TIP_)
				{
					inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(_NCCC_DCC_FILE_MASTERCARD_DISCLAIMER_, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle);
				}
				else
				{
					inCREDIT_PRINT_DCC_Disclaimer_ByBuffer_046(_PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle);
				}
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
				inRetVal = inPRINT_Buffer_PutIn("免      簽      名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        inPRINT_Buffer_PutIn("持卡人存根", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			inPRINT_Buffer_PutIn("Card holder stub", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			/* 持卡人姓名 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate,"%s",pobTran->srBRec.szCardHolder);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
		
		if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
		{
			inPRINT_Buffer_PutIn("重印 REPRINT", _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		}
		
                /* 警示語 */
                inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
                inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		
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
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
        }

        return (VS_SUCCESS);
	
}

/*
Function        :inCREDIT_PRINT_DCC_Disclaimer_ByBuffer_046
Date&Time       :2022/5/4 下午 3:34
Describe        :印Disclaimer
 *		046用hardcode
*/
int inCREDIT_PRINT_DCC_Disclaimer_ByBuffer_046(int inFontSize, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	inPRINT_Buffer_PutIn("I have been offered a choice of currencies and ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("agree to pay in the selected Transaction", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("Currency. Dynamic Currency Conversion (DCC) is ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("offered by the Merchant.", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_FISC_Data_ByBuffer_046
Date&Time       :2022/4/27 上午 10:54
Describe        :
*/
int inCREDIT_PRINT_FISC_Data_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
	int     inRetVal;
	char 	szPrintBuf[84 + 1], szPrintBuf1[42 + 1], szPrintBuf2[42 + 1], szTemplate1[42 + 1], szTemplate2[42 + 1];
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	
	do
	{
		/* 發卡行代碼 */
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn("發卡行代碼", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memcpy(szPrintBuf1, &pobTran->srBRec.szFiscIssuerID[0], 3);
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;


		/* 卡別 */
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn("卡別(Card Type)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		/* 卡別 */
		inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 12, _PADDING_RIGHT_);
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 卡號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, pobTran->srBRec.szPAN);
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn("卡號(Card No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, pobTran->srBRec.szPAN);
		/* 卡號遮掩 */
		if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
		{
			for (i = 6; i < (strlen(szPrintBuf) - 4); i ++)
				szPrintBuf[i] = 0x2A;
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
				for (i = 6; i < (strlen(szPrintBuf) - 4); i ++)
					szPrintBuf[i] = 0x2A;
			}

		}

		/* 過卡方式 */
		if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
		{
			if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
				strcat(szPrintBuf,"(C)");
			else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
				strcat(szPrintBuf, "(T)");
			else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf, "(W)");
			else
			{
				if (pobTran->srBRec.uszManualBit == VS_TRUE)
				{
					/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
					/* 電文轉Manual Keyin但是簽單要印感應的W */
					if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
						strcat(szPrintBuf, "(W)");
					else
						strcat(szPrintBuf,"(M)");
				}
				else
					strcat(szPrintBuf,"(S)");
			}

		}
		else
		{
			if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf, "(W)");
			else
				strcat(szPrintBuf, "(C)");
		}
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;

		/* 主機別 & 交易別 */
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn("主機別/交易類別(Host/Trans.Type)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;

		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		inGetHostLabel(szTemplate1);
		sprintf(szPrintBuf1, "%s", szTemplate1);

		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));

		sprintf(szPrintBuf2, "%s", szTemplate1);
		sprintf(szPrintBuf, "%s %s", szPrintBuf1 , szPrintBuf2);
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;

		if (strlen(szTemplate2) > 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate2);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;
		}

		/* 批次號碼 */
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn("批次號碼(Batch No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;

		/* Batch Num */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%03ld", pobTran->srBRec.lnBatchNum);
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;

		/* 日期時間 */
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn("日期/時間(Date/Time)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;

		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
			break;

		/* 序號 調閱編號 */
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn("序號(Ref. No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;
		
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("調閱編號(Inv.No)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
			break;


		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		/* 雖然電文RRN送12個byte，但RRN最後一碼是0x00，所以只看到11碼 */
		inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szRefNo, ' ', 12, _PADDING_RIGHT_);
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;

		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%06ld", pobTran->srBRec.lnOrgInvNum);
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
			break;

		/* 調單編號 */
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn("調單編號(RRN NO.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memcpy(szPrintBuf, pobTran->srBRec.szFiscRRN, _FISC_RRN_SIZE_);
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;



		/* 櫃號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetStoreIDEnable(szTemplate1);
		if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
		{
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("櫃號(Store ID)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szStoreID, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;
		}

		/* 產品代碼 */
		inGetProductCodeEnable(szProductCodeEnable);
		if (memcmp(szProductCodeEnable, "Y", 1) == 0)
		{
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("產品代碼(Product Code)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szProductCode, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;
		}

		/* 商店聯卡號遮掩 */
		memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
		inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
		if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
		    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
		    strlen(pobTran->srBRec.szTxnNo) > 0)
		{
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;
			
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;
		}
		break;
	}while(1);
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);

	return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_FISC_Amount_ByBuffer_046
Date&Time       :2022/4/27 上午 11:29
Describe        :
 */
int inCREDIT_PRINT_FISC_Amount_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
	
	/* 橫式 */
        /* 負向交易 */
	if(pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		/* 橫式 */
                /* 金額 */
                /* 取消退貨是正數 */
                if (pobTran->srBRec.inOrgCode == _FISC_REFUND_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

                        /* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Total) :");
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
		else
		{
			/* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

                        /* 把前面的字串和數字結合起來 */
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Total) :");
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
	}
	else
	{
		
		/* 初始化 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

		/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
		if (pobTran->srBRec.inCode == _FISC_REFUND_)
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
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", "總計(Total) :");
		inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
	}
	
	/* 斷行 */
	inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_HG_Data_ByBuffer_046
Date&Time       :2022/4/27 上午 11:34
Describe        :
*/
int inCREDIT_PRINT_HG_Data_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int     i;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1], szTemplate1[42 + 1];
	char	szProductCodeEnable[1 + 1];
        
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        /* HG卡號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        strcpy(szTemplate, pobTran->srBRec.szHGPAN);
        /* 卡號遮掩 */
        if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
        {
		/* HAPPG_GO 卡不掩飾 */
		if (!memcmp(&pobTran->srBRec.szHGPAN[0], "9552", 4))
		{

		}
		else
		{
			for (i = 6; i < (strlen(szTemplate) - 4); i ++)
				szTemplate[i] = 0x2A;
		}
        }
        sprintf(szPrintBuf, "ＨＧ卡號　 %s", szTemplate);
	inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        /* 主機別 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "主機　　　 HAPPY GO");
	inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        /* 交易類別 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        vdCREDIT_PRINT_HG_GetTransactionType(pobTran, szTemplate, szTemplate1);
        sprintf(szPrintBuf, "交易類別　 %s%s", szTemplate, szTemplate1);
	inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        /* 日期 & 時間 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "日期時間　 %.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
	inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        /* 調閱編號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "調閱編號　 %s/%06ld", pobTran->srBRec.szHGRefNo, pobTran->srBRec.lnOrgInvNum);
	inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        /* 櫃號 */
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
        inGetStoreIDEnable(szTemplate1);
        if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "櫃號　　　 %s", pobTran->srBRec.szStoreID);
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }
	/* 產品代碼 */
	inGetProductCodeEnable(szProductCodeEnable);
	if (memcmp(szProductCodeEnable, "Y", 1) == 0)
	{
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inPRINT_Buffer_PutIn("產品代碼(Product Code)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inPRINT_Buffer_PutIn(pobTran->srBRec.szProductCode, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
        
        /* 斷行 */
	inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
	inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
        inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
        
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_HG_Amount_ByBuffer_046
Date&Time       :2022/4/27 上午 11:40
Describe        :
*/
int inCREDIT_PRINT_HG_Amount_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szPrintBuf[84 + 1], szTemplate[42 + 1], szTemplate1[42 + 1];
        /* 
        HG_FULL_REDEMPTION 	點數兌換	HG_PAY_CREDIT		信用卡
        HG_INQUIRY		點數查詢	HG_PAY_CASH		現金
        HG_REWARD		紅利積點	HG_PAY_GIFT_PAPER	禮券
        HG_ONLINE_REDEEM	點數扣抵	HG_PAY_CREDIT_INSIDE	HGI
        HG_POINT_CERTAIN	加價購		HG_PAY_CUP		銀聯卡

        HG_REWARD_CREDIT		紅利積點 + 信用卡
        HG_REWARD_CASH			紅利積點 + 現金
        HG_REWARD_GIFT_PAPER		紅利積點 + 禮券
        HG_REWARD_CREDIT_INSIDE		紅利積點 + HGI
        HG_REWARD_CUP			紅利積點 + 銀聯卡

        HG_ONLINE_REDEEM_CREDIT		點數扣抵 + 信用卡
        HG_ONLINE_REDEEM_CASH		點數扣抵 + 現金
        HG_ONLINE_REDEEM_GIFT_PAPER	點數扣抵 + 禮券
        HG_ONLINE_REDEEM_CREDIT_INSIDE	點數扣抵 + HGI
        HG_ONLINE_REDEEM_CUP		點數扣抵 + 銀聯卡

        HG_POINT_CERTAIN_CREDIT		加價購 + 信用卡
        HG_POINT_CERTAIN_CASH		加價購 + 現金
        HG_POINT_CERTAIN_GIFT_PAPER	加價購 + 禮券
        HG_POINT_CERTAIN_CREDIT_INSIDE	加價購 + HGI
        HG_POINT_CERTAIN_CUP		加價購 + 銀聯卡
	*/
    
        if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
        {
                switch (pobTran->srBRec.inHGCode)
		{
                        /* 點數兌換 */
                        case _HG_FULL_REDEMPTION_:
                                /* 合計 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "合計　　　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 斷行 */
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 剩餘點數 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "剩餘點數　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                break;
                        /* 加價購 */
                        case _HG_POINT_CERTAIN_CASH_:
                        case _HG_POINT_CERTAIN_GIFT_PAPER_:
                                /* 合計 */                               
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate1, "NT$ ", '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "合計　　　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 斷行 */
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 扣抵點數 */                               
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "扣抵點數　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 剩餘點數 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "剩餘點數　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                break;  
                        /* 點數扣抵 */
                        case _HG_ONLINE_REDEEM_CASH_:
                        case _HG_ONLINE_REDEEM_GIFT_PAPER_:
			case _HG_ONLINE_REDEEM_CREDIT_:
			case _HG_ONLINE_REDEEM_CREDIT_INSIDE_:
			case _HG_ONLINE_REDEEM_CUP_:
                                /* 合計 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGAmount);
				inFunc_Amount_Comma(szTemplate1, "NT$ ", '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "合計　　　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 斷行 */
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 商品金額 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnHGRedeemAmount));
				inFunc_Amount_Comma(szTemplate1, "NT$ ", '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "商品金額　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 點數扣抵金額 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGRedeemAmount);
                                inFunc_Amount_Comma(szTemplate1, "NT$ ", '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "點數扣抵金額:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 扣抵點數 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "扣抵點數　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 剩餘點數 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "剩餘點數　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                break;       
                        /* 紅利積點 */
                        case _HG_REWARD_CASH_:
                        case _HG_REWARD_GIFT_PAPER_:
                                /* 合計 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
                                inFunc_Amount_Comma(szTemplate1, "NT$ ", '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "合計　　　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 斷行 */
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                break;
                        /* 回饋退貨 */
                        case _HG_REWARD_REFUND_:
                                /* 合計 */                              
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "合計　　　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 斷行 */
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 剩餘點數 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "剩餘點數　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                
                                if (pobTran->inTransactionCode != _VOID_)
                                {                                        
                                        /* 將數字塞到szTemplate中來inpad */
                                        memset(szTemplate, 0x00, sizeof(szTemplate));
                                        memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                        sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGRefundLackPoint);
                                        inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                        sprintf(szTemplate, "%s點",szTemplate1);
					inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
					inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                        /* 把前面的字串和數字結合起來 */
                                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                        sprintf(szPrintBuf, "不足點數　　:");
					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                }
                                break;
                        /* 扣抵退貨 */
                        case _HG_REDEEM_REFUND_:
                                 /* 合計 */                              
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnHGTransactionPoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "合計　　　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 斷行 */
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 剩餘點數 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "剩餘點數　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                break;                                
                        default:
                                break;
                }
        }
        else 
        {
                switch (pobTran->srBRec.inHGCode)
		{
                        /* 點數兌換 */
                        case _HG_FULL_REDEMPTION_:
                                /* 合計 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                if (pobTran->srBRec.lnHGTransactionPoint == 0L)
                                {
                                        sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
                                }
                                else
                                {
                                        sprintf(szTemplate1, "-%ld", pobTran->srBRec.lnHGTransactionPoint);
                                }
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "合計　　　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 斷行 */
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 剩餘點數 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "剩餘點數　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                break;
                        /* 加價購 */
                        case _HG_POINT_CERTAIN_CASH_:
                        case _HG_POINT_CERTAIN_GIFT_PAPER_:                                
                                /* 扣抵點數 */                               
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                if (pobTran->srBRec.lnHGTransactionPoint == 0L)
                                {
                                        sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
                                }
                                else
                                {
                                        sprintf(szTemplate1, "-%ld", pobTran->srBRec.lnHGTransactionPoint);
                                }
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "扣抵點數　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 剩餘點數 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "剩餘點數　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                break;  
                        /* 點數扣抵 */
                        case _HG_ONLINE_REDEEM_CASH_:
                        case _HG_ONLINE_REDEEM_GIFT_PAPER_:
			case _HG_ONLINE_REDEEM_CREDIT_:
			case _HG_ONLINE_REDEEM_CREDIT_INSIDE_:
			case _HG_ONLINE_REDEEM_CUP_:
                                /* 商品金額 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                if (pobTran->srBRec.uszHappyGoSingle == VS_TRUE)
                                {
                                        if (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnHGRedeemAmount == 0L)
                                        {
                                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
                                        }
                                        else
                                        {
                                                sprintf(szTemplate1, "-%ld", pobTran->srBRec.lnTxnAmount);
                                        }
                                }
                                else
                                {
                                        if (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnHGRedeemAmount == 0L)
                                        {
                                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnHGRedeemAmount);
                                        }
                                        else
                                        {
                                                sprintf(szTemplate1, "-%ld", pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnHGRedeemAmount);
                                        }
                                }
                                inFunc_Amount_Comma(szTemplate1, "NT$ ", '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "商品金額　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 點數扣抵金額 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
				if (pobTran->srBRec.lnHGRedeemAmount == 0)
				{
					sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGRedeemAmount);
				}
				else
				{
					sprintf(szTemplate1, "-%ld", pobTran->srBRec.lnHGRedeemAmount);
				}
                                inFunc_Amount_Comma(szTemplate1, "NT$ ", '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "點數扣抵金額:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 扣抵點數 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
				if (pobTran->srBRec.lnHGTransactionPoint == 0)
				{
					sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
				}
				else
				{
					sprintf(szTemplate1, "-%ld", pobTran->srBRec.lnHGTransactionPoint);
				}
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "扣抵點數　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 剩餘點數 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "剩餘點數　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);                             
                                break;       
                        /* 紅利積點 */
                        case _HG_REWARD_CASH_:
                        case _HG_REWARD_GIFT_PAPER_:
                                /* 合計 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                if (pobTran->srBRec.lnTxnAmount == 0L)
                                {
                                        sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
                                }
                                else
                                {
                                        sprintf(szTemplate1, "-%ld", pobTran->srBRec.lnTxnAmount);
                                }
                                
                                inFunc_Amount_Comma(szTemplate1, "NT$ ", '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "合計　　　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 斷行 */
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                break;
                        /* 回饋退貨 */
                        case _HG_REWARD_REFUND_:
                                /* 合計 */                              
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnHGTransactionPoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "合計　　　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 斷行 */
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 剩餘點數 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "剩餘點數　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_); 
                                
                                if (pobTran->inTransactionCode != _VOID_)
                                {                                        
                                        /* 將數字塞到szTemplate中來inpad */
                                        memset(szTemplate, 0x00, sizeof(szTemplate));
                                        memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                        sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGRefundLackPoint);
                                        inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                        sprintf(szTemplate, "%s點",szTemplate1);
					inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
					inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                        /* 把前面的字串和數字結合起來 */
                                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                        sprintf(szPrintBuf, "不足點數　　:");
					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                }
                                break;
                        /* 扣抵退貨 */
                        case _HG_REDEEM_REFUND_:
                                /* 合計 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
                                inFunc_Amount_Comma(szTemplate1, "NT$ ", '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "合計　　　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 斷行 */
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 剩餘點數 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "剩餘點數　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                break;                                
                        default:
                                break;
                }
        }
        
        /* 斷行 */
	inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
        inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
        inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
        
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_HG_Multi_Data_ByBuffer_046
Date&Time       :2022/4/27 上午 11:52
Describe        :
*/
int inCREDIT_PRINT_HG_Multi_Data_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szPrintBuf[84 + 1], szTemplate[42 + 1], szTemplate1[42 + 1];
        int     i;
	
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        
        /* 把前面的字串和數字結合起來 */
	inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
        inPRINT_Buffer_PutIn("快樂購聯合集點卡", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        /* 交易類別 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        vdCREDIT_PRINT_HG_GetTransactionType(pobTran, szTemplate, szTemplate1);
        sprintf(szPrintBuf, "%s%s", szTemplate, szTemplate1);
	inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* HG卡號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        strcpy(szTemplate, pobTran->srBRec.szHGPAN);
        /* 卡號遮掩 */
        if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
        {
		/* HAPPG_GO 卡不掩飾 */
		if (!memcmp(&pobTran->srBRec.szHGPAN[0], "9552", 4))
		{

		}
		else
		{
			for (i = 6; i < (strlen(szTemplate) - 4); i ++)
				szTemplate[i] = 0x2A;
		}
        }
        sprintf(szPrintBuf, "ＨＧ卡號　 %s", szTemplate);
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);
        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
	inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        
	if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
	{
		switch (pobTran->srBRec.inHGCode)
		{
			case _HG_POINT_CERTAIN_CREDIT_:
			case _HG_POINT_CERTAIN_CUP_:
			case _HG_POINT_CERTAIN_CREDIT_INSIDE_:
				/* 扣抵點數 */                               
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_FONT_PRINT_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "扣抵點數　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				/* 剩餘點數 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_FONT_PRINT_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "剩餘點數　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				break;
			case _HG_ONLINE_REDEEM_CREDIT_:
			case _HG_ONLINE_REDEEM_CREDIT_INSIDE_:
			case _HG_ONLINE_REDEEM_CUP_:
				/* 商品金額 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnHGRedeemAmount));
				inFunc_Amount_Comma(szTemplate1, "NT$", '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_FONT_PRINT_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "商品金額　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				/* 點數扣抵金額 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGRedeemAmount);
				inFunc_Amount_Comma(szTemplate1, "NT$", '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_FONT_PRINT_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "點數扣抵金額:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				/* 扣抵點數 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_FONT_PRINT_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "扣抵點數　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				/* 剩餘點數 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_FONT_PRINT_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "剩餘點數　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				break;

			default:
				break;
		}
	}
	/* 負向交易時 */
	else
	{
		switch (pobTran->srBRec.inHGCode)
		{
			case _HG_POINT_CERTAIN_CREDIT_:
			case _HG_POINT_CERTAIN_CUP_:
			case _HG_POINT_CERTAIN_CREDIT_INSIDE_:
				/* 扣抵點數 */                               
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "-%ld", pobTran->srBRec.lnHGTransactionPoint);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_FONT_PRINT_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "扣抵點數　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				/* 剩餘點數 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_FONT_PRINT_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "剩餘點數　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				break;
			case _HG_ONLINE_REDEEM_CREDIT_:
			case _HG_ONLINE_REDEEM_CREDIT_INSIDE_:
			case _HG_ONLINE_REDEEM_CUP_:
				/* 商品金額 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", (0 - (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnHGRedeemAmount)));
				inFunc_Amount_Comma(szTemplate1, "NT$", '\x00', _SIGNED_MINUS_, 16, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_FONT_PRINT_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "商品金額　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				/* 點數扣抵金額 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnHGRedeemAmount));
				inFunc_Amount_Comma(szTemplate1, "NT$", '\x00', _SIGNED_MINUS_, 16, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_FONT_PRINT_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "點數扣抵金額:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				/* 扣抵點數 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "-%ld", pobTran->srBRec.lnHGTransactionPoint);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_FONT_PRINT_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "扣抵點數　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				/* 剩餘點數 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_BAUL_SetFont_Style(_FONT_PRINT_BOLD_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "剩餘點數　　:");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				break;

			default:
				break;
		}
	}
        
        /* 斷行 */
	inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
	inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
        inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
        
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Data_ByBuffer_ESVC_046
Date&Time       :2022/4/27 下午 1:43
Describe        :列印DATA
*/
int inCREDIT_PRINT_Data_ByBuffer_ESVC_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inLen = 0;
	int     inRetVal = VS_SUCCESS;
	char 	szPrintBuf[84 + 1], szTemplate1[42 + 1];
	char	szTemplate[40 + 1];
	char	szProductCodeEnable[1 + 1];
	char	szTicketNeedNewBatch[2 + 1] = {0};
        char    szCustomerIndicator[3 + 1] = {0};

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	do
	{
		/* 橫式 */
		/* 卡號、卡別 */
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn("卡號", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;

		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("卡別", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
			break;

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
		{
			strcpy(szPrintBuf, "一卡通");
		}
		else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
		{
			strcpy(szPrintBuf, "悠遊卡");
		}
		else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
		{
			strcpy(szPrintBuf, "愛金卡");
		}

		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			break;

		/* 卡號值 & 批號 */
		/* 一卡通：遮掩後2 碼，遮掩字元為”*” */
		if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inLen = strlen(pobTran->srTRec.szUID);
			memcpy(szPrintBuf, pobTran->srTRec.szUID, inLen);
			szPrintBuf[inLen - 1] = 0x2A;
			szPrintBuf[inLen - 2] = 0x2A;
			strcat(szPrintBuf, "(W)");
		}
		/*	1.簽單卡號列印邏輯
			<T4800>票卡版號(Purse Version Number)=0，<T0200>列印晶片卡號(Card Id)
			<T0200>20 bytes，14th,15th,16th隱碼，17th,18th,19th,20th顯示
			<T4800>票卡版號(Purse Version Number)<>0，<T0211>列印外觀卡號(Purse Id)
			<T0211>16 bytes，6th,7th,8th,9th,10th,11th隱碼，12th,13th,14th,15th,16th顯示
			，遮掩字元為”*”
			以<T0200>悠遊卡卡號遮掩從後往前算第五碼，第六碼，第七碼遮掩
			範例:以8碼卡號為例，卡號11651733，1,6,5要遮掩，實際列印為1***1733
			1th 2th 3th 4th 5th 6th 7th 8th 9th 10th 11th 12th 13th 14th 15th 16th 17th
			0 0 0 0 0 0 0 0 0 1 1 6 5 1 7 3 3
			0 0 0 0 0 0 0 0 0 1 * * * 1 7 3 3
			範例:以10碼卡號為例，卡號6611651733，1,6,5要遮掩，實際列印為661***1733
			1th 2th 3th 4th 5th 6th 7th 8th 9th 10th 11th 12th 13th 14th 15th 16th 17th
			0 0 0 0 0 0 0 6 6 1 1 6 5 1 7 3 3
			0 0 0 0 0 0 0 6 6 1 * * * 1 7 3 3

			2. 電子發票載具卡號為T0200 (不遮掩)
		*/
		else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inLen = strlen(pobTran->srTRec.szUID);
			memcpy(szPrintBuf, pobTran->srTRec.szUID, inLen);

			/* 客製化098，悠遊卡因為使用T0200，加密遮掩為倒數5、6、7 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
			    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))
			{
				szPrintBuf[inLen - 5] = 0x2A;
				szPrintBuf[inLen - 6] = 0x2A;
				szPrintBuf[inLen - 7] = 0x2A;
			}
			else
			{
				if (atoi(pobTran->srTRec.srECCRec.szPurseVersionNumber) == 0)
				{
					szPrintBuf[inLen - 5] = 0x2A;
					szPrintBuf[inLen - 6] = 0x2A;
					szPrintBuf[inLen - 7] = 0x2A;
				}
				else
				{
					/* 全部16 第6-11隱碼 */
					szPrintBuf[inLen - 6] = 0x2A;
					szPrintBuf[inLen - 7] = 0x2A;
					szPrintBuf[inLen - 8] = 0x2A;
					szPrintBuf[inLen - 9] = 0x2A;
					szPrintBuf[inLen - 10] = 0x2A;
					szPrintBuf[inLen - 11] = 0x2A;
				}
			}    
			strcat(szPrintBuf, "(W)");
		}
		else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inLen = strlen(pobTran->srTRec.szUID);
			memcpy(szPrintBuf, pobTran->srTRec.szUID, inLen);
			/* 愛金卡：遮掩9-12 碼，遮掩字元為"*" */
			szPrintBuf[8] = 0x2A;
			szPrintBuf[9] = 0x2A;
			szPrintBuf[10] = 0x2A;
			szPrintBuf[11] = 0x2A;
			strcat(szPrintBuf, "(W)");		
		}
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "批號　　 %s", "");
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
			break;

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		inGetBatchNum(szPrintBuf);
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			break;

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		if (pobTran->srTRec.inCode == _TICKET_IPASS_DEDUCT_ || 
		    pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_ ||
		    pobTran->srTRec.inCode == _TICKET_ICASH_DEDUCT_)
			strcpy(szTemplate, "購貨　　");
		else if (pobTran->srTRec.inCode == _TICKET_IPASS_REFUND_ ||
			 pobTran->srTRec.inCode == _TICKET_EASYCARD_REFUND_ ||
			 pobTran->srTRec.inCode == _TICKET_ICASH_REFUND_)
			strcpy(szTemplate, "退貨　　");
		else if (pobTran->srTRec.inCode == _TICKET_IPASS_TOP_UP_ || 
			 pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_ ||
			 pobTran->srTRec.inCode == _TICKET_ICASH_TOP_UP_)
			strcpy(szTemplate, "現金加值");  
		else if (pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_ || 
			 pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_ ||
			 pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_)
			strcpy(szTemplate, "加值取消");              
		else if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ || 
			 pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
			 pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
			strcpy(szTemplate, "餘額查詢");  
		else if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_ || 
			 pobTran->srTRec.inCode == _TICKET_EASYCARD_AUTO_TOP_UP_ ||
			 pobTran->srTRec.inCode == _TICKET_ICASH_AUTO_TOP_UP_)
			strcpy(szTemplate, "自動加值");         
		else
			strcpy(szTemplate, "　　");

		sprintf(szPrintBuf, "交易　　 %s", szTemplate);

		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			break;

		if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_ || pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate, 0x00, sizeof(szTemplate));

			sprintf(szPrintBuf, "特店代號 %s", szTemplate1);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;

			if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetIPASS_System_ID(szTemplate);
				memcpy(&szPrintBuf[0], szTemplate, 2);

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetIPASS_SP_ID(szTemplate);
				memcpy(&szPrintBuf[2], szTemplate, 2);

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetIPASS_Sub_Company_ID(szTemplate);
				memcpy(&szPrintBuf[4], szTemplate, 4);
			}        
			else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetECC_New_SP_ID(szTemplate);
				memcpy(&szPrintBuf[0], szTemplate, 8);
			}

			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			if (inRetVal != VS_SUCCESS)
				break;
		}
		else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate, 0x00, sizeof(szTemplate));

			sprintf(szPrintBuf, "門市代碼 %s", szTemplate1);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetICASH_Shop_ID(szTemplate);
			memcpy(&szPrintBuf[0], szTemplate, 8);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			if (inRetVal != VS_SUCCESS)
				break;		
		}

		/* 主機、調閱編號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		inGetHostLabel(szTemplate);
		sprintf(szPrintBuf, "主機　　 %s", szTemplate);
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;

		if (pobTran->srTRec.inCode != _TICKET_IPASS_INQUIRY_ &&
		    pobTran->srTRec.inCode != _TICKET_EASYCARD_INQUIRY_ &&
		    pobTran->srTRec.inCode != _TICKET_ICASH_INQUIRY_)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "調閱號　 %s", "");
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%06ld", pobTran->srTRec.lnInvNum);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			if (inRetVal != VS_SUCCESS)
				break;
		}
		else
		{
			/* 現在查詢餘額不印簽單 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " ");
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;
		}

		/* 日期時間 */
		if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%.4s/%.2s/%.2s  %.2s:%.2s:%.2s", 
				&pobTran->srTRec.srECCRec.szDate[0], 
				&pobTran->srTRec.srECCRec.szDate[4], 
				&pobTran->srTRec.srECCRec.szDate[6], 
				&pobTran->srTRec.srECCRec.szTime[0], 
				&pobTran->srTRec.srECCRec.szTime[2], 
				&pobTran->srTRec.srECCRec.szTime[4]);
		}
		else 
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "20%.2s/%.2s/%.2s  %.2s:%.2s:%.2s", 
				&pobTran->srTRec.szDate[0], 
				&pobTran->srTRec.szDate[2], 
				&pobTran->srTRec.szDate[4], 
				&pobTran->srTRec.szTime[0], 
				&pobTran->srTRec.szTime[2], 
				&pobTran->srTRec.szTime[4]);
		}

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "日期/時間 %s", szTemplate);
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;

		/* 序號 & RF序號 */
		if (pobTran->srTRec.inCode != _TICKET_IPASS_INQUIRY_ &&
		    pobTran->srTRec.inCode != _TICKET_EASYCARD_INQUIRY_ &&
		    pobTran->srTRec.inCode != _TICKET_ICASH_INQUIRY_)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			/* 雖然電文RRN送12個byte，但RRN最後一碼是0x00，所以只看到11碼 */
			inFunc_PAD_ASCII(szTemplate, pobTran->srTRec.szRefNo, ' ', 12, _PADDING_RIGHT_);

			sprintf(szPrintBuf, "序號　　 %s", szTemplate);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;


			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "RF序號　 %s", "");
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memcpy(szPrintBuf, pobTran->srTRec.szTicketRefundCode, 6);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			if (inRetVal != VS_SUCCESS)
				break;
		}

		if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetTicket_Device2(szTemplate);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "二代設備編號　 %s", szTemplate);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szTicketNeedNewBatch, 0x00, sizeof(szTicketNeedNewBatch));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetTicket_NeedNewBatch(szTicketNeedNewBatch);
			if (memcmp(szTicketNeedNewBatch, "Y", strlen("Y")) == 0)
			{
				sprintf(szTemplate, "%.4s%.2s%.2s", &pobTran->srBRec.szDate[2], &pobTran->srBRec.szDate[6], _ECC_DEFAULT_BATCH_);
			}
			else
			{
				inGetTicket_Batch(szTemplate);
			}
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "悠遊卡批次號碼 %s", szTemplate);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, pobTran->srTRec.srECCRec.szRRN);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "RRN    　　　  %s", szTemplate);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;
		}

		/* 櫃號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetStoreIDEnable(szTemplate1);
		if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srTRec.szStoreID) > 0))
		{
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("櫃號(Store ID)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(pobTran->srTRec.szStoreID, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;
		}

		/* 產品代碼 */
		inGetProductCodeEnable(szProductCodeEnable);
		if (memcmp(szProductCodeEnable, "Y", 1) == 0)
		{
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("產品代碼(Product Code)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(pobTran->srTRec.szProductCode, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;
		}

		/* 斷行 */
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;
		break;
	}while(1);
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_Amount_ByBuffer_IPASS_046
Date&Time       :2022/4/27 下午 1:42
Describe        :
*/
int inCREDIT_PRINT_Amount_ByBuffer_IPASS_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal = VS_SUCCESS;
        char    szPrintBuf[42 + 1];
        
	do
	{
		if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_)
		{
			/* 自動加值金額 */
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("自動加值 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf)); 
			sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTotalTopUpAmount);
			inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			if (inRetVal != VS_SUCCESS)
				break;

			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* 交易前餘額 */
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("交易前餘額　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			if (pobTran->srTRec.lnCardRemainAmount < 0)
			{
				sprintf(szPrintBuf, "%ld", 0 - pobTran->srTRec.lnCardRemainAmount);
			}
			else
			{
				sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnCardRemainAmount);
			}

			if (pobTran->srTRec.lnCardRemainAmount < 0)
			{
				inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_MINUS_, 10, _PADDING_RIGHT_);
			}
			else
			{
				inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
			}

			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			if (inRetVal != VS_SUCCESS)
				break;

			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);   
			if (inRetVal != VS_SUCCESS)
				break;

			/* 交易後餘額 */
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("交易後餘額　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			if (pobTran->srTRec.lnCardRemainAmount < 0)
			{
				sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTotalTopUpAmount - pobTran->srTRec.lnCardRemainAmount);
			}
			else
			{
				sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTotalTopUpAmount + pobTran->srTRec.lnCardRemainAmount);
			}
			inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);

			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);      
			if (inRetVal != VS_SUCCESS)
				break;

			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;
		}
		else
		{
			if (pobTran->srTRec.inCode == _TICKET_IPASS_DEDUCT_)
			{       
				if (pobTran->srTRec.lnTotalTopUpAmount > 0L)
				{
					/* 自動加值金額 */
					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
					inRetVal = inPRINT_Buffer_PutIn("自動加值 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						break;

					memset(szPrintBuf, 0x00, sizeof(szPrintBuf)); 
					sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTotalTopUpAmount);
					inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
					if (inRetVal != VS_SUCCESS)
						break;

					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
					inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						break;
				}
			}

			/* 交易前餘額 */
			/* 交易前餘額 */
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("交易前餘額　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			if (pobTran->srTRec.lnTotalTopUpAmount > 0L)
			{       
				if (pobTran->srTRec.lnFinalBeforeAmt > 100000)
				{
					sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTotalTopUpAmount + (pobTran->srTRec.lnFinalBeforeAmt - 100000));
				}
				else
				{
					sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalBeforeAmt - pobTran->srTRec.lnTotalTopUpAmount);
				}
			}
			else      
			{      
				if (pobTran->srTRec.lnFinalBeforeAmt > 100000)
				{
					sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalBeforeAmt - 100000);
				}
				else
				{
					sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalBeforeAmt);
				}
			}

			if (pobTran->srTRec.lnFinalBeforeAmt > 100000)
			{
				inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_MINUS_, 10, _PADDING_RIGHT_);
			}
			else
			{
				inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
			}

			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			if (inRetVal != VS_SUCCESS)
				break;

			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* 交易金額 */
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("交易金額　　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTxnAmount);
			inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			if (inRetVal != VS_SUCCESS)
				break;

			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);  
			if (inRetVal != VS_SUCCESS)
				break;

			/* 交易後餘額 */
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("交易後餘額　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

			if (pobTran->srTRec.lnFinalAfterAmt > 100000)
			{     
				sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalAfterAmt - 100000);   
				inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_MINUS_, 10, _PADDING_RIGHT_);
			}
			else
			{      
				sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalAfterAmt);     
				inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
			}

			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			if (inRetVal != VS_SUCCESS)
				break;

			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;
		}
		break;
	}while(1);
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
               
        return (VS_SUCCESS);      
}

/*
Function        :inCREDIT_PRINT_Amount_ByBuffer_ECC_046
Date&Time       :2022/4/27 下午 1:42
Describe        :
*/
int inCREDIT_PRINT_Amount_ByBuffer_ECC_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal = VS_SUCCESS;
        char    szPrintBuf[42 + 1];

	do
	{
		if (pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_ && pobTran->srTRec.lnTotalTopUpAmount > 0L)
		{
			/* 自動加值金額 */
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("自動加值 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTotalTopUpAmount);
			inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			if (inRetVal != VS_SUCCESS)
				break;

			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;
		}

		/* 交易前餘額 */
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn("交易前餘額　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalBeforeAmt);
		inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);

		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_); 
		if (inRetVal != VS_SUCCESS)
			break;

		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;

		/* 交易金額 */
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn("交易金額　　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTxnAmount);
		inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);

		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			break;
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;

		/* 交易後餘額 */
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn("交易後餘額　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalAfterAmt);
		inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);

		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);        
		if (inRetVal != VS_SUCCESS)
			break;

		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;
		break;
	}while(1);
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);

        return (VS_SUCCESS);      
}

/*
Function        :inCREDIT_PRINT_Amount_ByBuffer_ICASH_046
Date&Time       :2022/4/27 下午 1:42
Describe        :
*/
int inCREDIT_PRINT_Amount_ByBuffer_ICASH_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal = VS_SUCCESS;
        char    szPrintBuf[42 + 1];

	do
	{
		if (pobTran->srTRec.inCode == _TICKET_ICASH_DEDUCT_ && pobTran->srTRec.lnTotalTopUpAmount > 0L)
		{
			/* 自動加值金額 */
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("自動加值 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTotalTopUpAmount);
			inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			if (inRetVal != VS_SUCCESS)
				break;

			inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;
		}

		/* 交易前餘額 */
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn("交易前餘額　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalBeforeAmt);
		inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_); 
		if (inRetVal != VS_SUCCESS)
			break;

		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;

		/* 交易金額 */
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn("交易金額　　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTxnAmount);
		inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);

		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			break;
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;

		/* 交易後餘額 */
		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn("交易後餘額　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalAfterAmt);
		inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);

		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);        
		if (inRetVal != VS_SUCCESS)
			break;

		inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			break;
		break;
	}while(1);
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);

        return (VS_SUCCESS);      
}

/*
Function        :inCREDIT_PRINT_Dutyfree_DetailReport_Title_ByBuffer
Date&Time       :2022/5/11 下午 2:47
Describe        :昇恆昌客製化 重印簽單的總表
*/
int inCREDIT_PRINT_Dutyfree_DetailReport_Title_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char uszReprint)
{
	int		inHostIndex = 0;
	int		inTotalCount = 0, inHostCount = 0;
	int		inNCCC_Count = 0, inDCC_Count = 0;
	int		inDUTY_FREEReturnSuccess_Cnt = 0, inDUTY_FREEReturnFail_Cnt = 0;
	int		inDUTY_FREEReturnSuccess_LastBatch_Cnt = 0, inDUTY_FREEReturnFail_LastBatch_Cnt = 0;
	int		i = 0;
	int		inRetVal = VS_SUCCESS;
	int		inNCCCIndex = 0;
	int		inOrgHDTIndex = -1;
	char		szMustSettleBit[1 + 1] = {0};
	char		szNCCCMustSettleBit[1 + 1] = {0};
	char		szHostName[8 + 1] = {0};
	char		szPrintBuf[84 + 1] = {0}, szPrintBuf1[84 + 1] = {0};
	char		szHostEnable[1 + 1] = {0};
	char		szTemplate[42 + 1] = {0};
	char		szTableName[30 + 1] = {0};
	long		lnReprintBatchNum = 0;		/* 能夠重印代表已結完帳，所以用累加過的批號減一 */
	unsigned char	uszSettle_SuccessBit = VS_FALSE;
	unsigned char	uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle	srBhandle;
	FONT_ATTRIB	srFont_Attrib;
	DUTYFREE_REPRINT_TITLE	srReprintTitle;
	SQLITE_ALL_TABLE	srALL;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inCREDIT_PRINT_Reprint_DetailReport_Title_ByBuffer() START !");
	}
	
	inOrgHDTIndex = pobTran->srBRec.inHDTIndex;
	
	memset(&srReprintTitle, 0x00, sizeof(DUTYFREE_REPRINT_TITLE));
	memset(&srALL, 0x00, sizeof(SQLITE_ALL_TABLE));
	inBatch_Table_Link_Reprint_Title(&srReprintTitle, &srALL, _LS_READ_);
	sprintf(szTableName, "%s", _TABLE_NAME_REPRINT_TITLE_);
	inSqlite_Get_Table_ByRecordID_All(gszReprintDBPath, szTableName, 0, &srALL);

	/* 連動結帳 */
	if (pobTran->uszAutoSettleBit == VS_TRUE || uszReprint == VS_TRUE)
	{
		/* 算總比數 */
		for (inHostIndex = 0; ; inHostIndex ++)
		{
			if (inLoadHDTRec(inHostIndex) < 0)
				break;
			if (inLoadHDPTRec(inHostIndex) < 0)
				break;

			memset(szHostEnable, 0x00, sizeof(szHostEnable));
			inGetHostEnable(szHostEnable);
			if (szHostEnable[0] != 'Y')
			{
				continue;
			}
			
			memset(szHostName, 0x00, sizeof(szHostName));
			inGetHostLabel(szHostName);
			if (memcmp(szHostName, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_)) == 0	||
			    memcmp(szHostName, _HOST_NAME_ESC_, strlen(_HOST_NAME_ESC_)) == 0	||
			    memcmp(szHostName, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_)) == 0)
			{
				continue;
			}

		        /* 結帳失敗就不算進去 */
			memset(szMustSettleBit, 0x00, sizeof(szMustSettleBit));
			inGetMustSettleBit(szMustSettleBit);
		        if (szMustSettleBit[0] != 'Y')
		        {
		        	/* 表示起碼有一個主機成功 */
		        	uszSettle_SuccessBit = VS_TRUE;

				if (memcmp(szHostName, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_)) == 0)
				{
					inSqlite_Get_Table_Count(gszReprintDBPath, _TABLE_NAME_REPRINT_NCCC_, &inNCCC_Count);	/*取出重印明細筆數*/
					inTotalCount += inNCCC_Count;
				}
				else if (memcmp(szHostName, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_)) == 0)
				{
					inSqlite_Get_Table_Count(gszReprintDBPath, _TABLE_NAME_REPRINT_DCC_, &inDCC_Count);	/*取出重印明細筆數*/
					inTotalCount += inDCC_Count;
				}
			}
		}
	}
	else
	{
		inLoadHDTRec(pobTran->srBRec.inHDTIndex);
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		
		memset(szHostName, 0x00, sizeof(szHostName));
		inGetHostLabel(szHostName);
		if (memcmp(szHostName, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_)) == 0	||
		    memcmp(szHostName, _HOST_NAME_ESC_, strlen(_HOST_NAME_ESC_)) == 0	||
		    memcmp(szHostName, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_)) == 0)
		{
			return (VS_SUCCESS);
		}

		/* 結帳失敗就不算進去 */
		memset(szMustSettleBit, 0x00, sizeof(szMustSettleBit));
		inGetMustSettleBit(szMustSettleBit);
		if (szMustSettleBit[0] != 'Y')
		{
			/* 表示起碼有一個主機成功 */
			uszSettle_SuccessBit = VS_TRUE;

			if (memcmp(szHostName, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_)) == 0)
			{
				inSqlite_Get_Table_Count(gszReprintDBPath, _TABLE_NAME_REPRINT_NCCC_, &inNCCC_Count);	/*取出重印明細筆數*/
				inTotalCount += inNCCC_Count;
			}
			else if (memcmp(szHostName, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_)) == 0)
			{
				inSqlite_Get_Table_Count(gszReprintDBPath, _TABLE_NAME_REPRINT_DCC_, &inDCC_Count);	/*取出重印明細筆數*/
				inTotalCount += inDCC_Count;
			}
		}
	}

	if (inTotalCount > 0)
	{
		/* 沒一個主機成功，就不印 */
		if (uszSettle_SuccessBit == VS_FALSE)
			return (VS_SUCCESS);

		/* 是否有列印功能 */
		if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
		{
			return (VS_SUCCESS);
		}
		else
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_);/* 處理中... */
			
			do
			{
				inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);
				srBhandle.uszDetailPrint = VS_TRUE;

				inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				
				/* 印NCC的LOGO */
				inRetVal = inPRINT_Buffer_PutGraphic((unsigned char*)_BANK_LOGO_, uszBuffer, &srBhandle, gsrBMPHeight.inBankLogoHeight, _APPEND_);
				if (inRetVal != VS_SUCCESS)
				{
					break;
				}
				
				/* 列印重印帳單總表*/
				/* \fr代表反白 */
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_REVERSE_);
				inPRINT_Buffer_PutIn("                                ", _PRT_046_DHW_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("重印帳單總表", _PRT_046_DHW_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				
				/* 印商店名稱 */
				if (inCREDIT_PRINT_MerchantName(pobTran, uszBuffer, &srBhandle) != VS_SUCCESS)
				{
					break;
				}
				
				/* 連動結帳用NCCC的TID MID當顯示 */
				if (pobTran->uszAutoSettleBit == VS_TRUE || uszReprint == VS_TRUE)
				{
					inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inNCCCIndex);
					if (inLoadHDTRec(inNCCCIndex) < 0)
						break;
				}

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetMerchantID(szTemplate);

				/* 列印商店代號 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "商店代號");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
				
				inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_LEFT_);
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					break;

				/* Get端末機代號 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetTerminalID(szTemplate);

				/* 列印端末機代號 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "端末機代號");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
				
				inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 13, _PADDING_LEFT_);
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					break;

				inRetVal = inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
				
				/* 日期時間 */
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inRetVal = inPRINT_Buffer_PutIn("日期/時間(Date/Time)", _PRT_046_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;

				memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
				sprintf(szPrintBuf1, "%.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
				if (inRetVal != VS_SUCCESS)
					break;
				
				/* 讀取上傳TMS成功和失敗筆數  */
				/* 讀出來 */
				inDUTY_FREEReturnSuccess_Cnt = atoi(srReprintTitle.szTMSUpdateSuccessNum);
				inDUTY_FREEReturnFail_Cnt = atoi(srReprintTitle.szTMSUpdateFailNum);
				inDUTY_FREEReturnSuccess_LastBatch_Cnt = atoi(srReprintTitle.szTMSUpdateSuccessNum);
				inDUTY_FREEReturnFail_LastBatch_Cnt = atoi(srReprintTitle.szTMSUpdateFailNum);

				memset(szHostName, 0x00, sizeof(szHostName));
				inGetHostLabel(szHostName);
				if (pobTran->uszAutoSettleBit == VS_TRUE)
				{
					inLoadHDTRec(inNCCCIndex);
					inLoadHDPTRec(inNCCCIndex);
					memset(szNCCCMustSettleBit, 0x00, sizeof(szNCCCMustSettleBit));
					inGetMustSettleBit(szNCCCMustSettleBit);
				}
				else
				{
					memset(szMustSettleBit, 0x00, sizeof(szMustSettleBit));
					inGetMustSettleBit(szMustSettleBit);
				}
				if (uszReprint == VS_TRUE)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "上傳成功");
					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						break;
					
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%03d  ", inDUTY_FREEReturnSuccess_LastBatch_Cnt);
					inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
					if (inRetVal != VS_SUCCESS)
						break;
					
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "筆");
					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
					if (inRetVal != VS_SUCCESS)
						break;
					
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "上傳失敗");
					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						break;
					
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%03d  ", inDUTY_FREEReturnFail_LastBatch_Cnt);
					inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
					if (inRetVal != VS_SUCCESS)
						break;
					
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "筆");
					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
					if (inRetVal != VS_SUCCESS)
						break;
					
					inRetVal = inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						break;
					
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "合計");
					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						break;
					
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%03d  ", inDUTY_FREEReturnSuccess_LastBatch_Cnt + inDUTY_FREEReturnFail_LastBatch_Cnt);
					inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
					if (inRetVal != VS_SUCCESS)
						break;
					
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "筆");
					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
					if (inRetVal != VS_SUCCESS)
						break;
				}
				else
				{
					/* 連動結帳f且NCCC結帳成功才印
					* 或是單結NCCC成功 */
					if ((pobTran->uszAutoSettleBit == VS_TRUE && szNCCCMustSettleBit[0] != 'Y')	||
					    (pobTran->uszAutoSettleBit != VS_TRUE && 
					     memcmp(szHostName, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_)) == 0	&&
					     szMustSettleBit[0] != 'Y'))
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "上傳成功");
						inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							break;

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%03d  ", inDUTY_FREEReturnSuccess_Cnt);
						inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
						if (inRetVal != VS_SUCCESS)
							break;

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "筆");
						inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
						if (inRetVal != VS_SUCCESS)
							break;

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "上傳失敗");
						inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							break;

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%03d  ", inDUTY_FREEReturnFail_Cnt);
						inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
						if (inRetVal != VS_SUCCESS)
							break;

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "筆");
						inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
						if (inRetVal != VS_SUCCESS)
							break;

						inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
						inRetVal = inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							break;

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "合計");
						inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							break;

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%03d  ", inDUTY_FREEReturnSuccess_Cnt + inDUTY_FREEReturnFail_Cnt);
						inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
						if (inRetVal != VS_SUCCESS)
							break;

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "筆");
						inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
						if (inRetVal != VS_SUCCESS)
							break;

						/* NCCC主機結帳成功上傳筆數歸0 */
						inDUTY_FREEReturnSuccess_LastBatch_Cnt = inDUTY_FREEReturnSuccess_Cnt;
						inDUTY_FREEReturnFail_LastBatch_Cnt = inDUTY_FREEReturnFail_Cnt;
						inDUTY_FREEReturnSuccess_Cnt = 0;
						inDUTY_FREEReturnFail_Cnt = 0;
						/* 存起來 */
					}
				}
				
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);	
				inPRINT_Buffer_PutIn("批次號碼", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);	
				inPRINT_Buffer_PutIn("主機", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("筆數", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				
				if (pobTran->uszAutoSettleBit == VS_TRUE || uszReprint == VS_TRUE)
				{
					inTotalCount = 0;
					
					/* 算總比數 */
					for (inHostIndex = 0; ; inHostIndex ++)
					{
						if (inLoadHDTRec(inHostIndex) < 0)
							break;
						if (inLoadHDPTRec(inHostIndex) < 0)
							break;

						memset(szHostEnable, 0x00, sizeof(szHostEnable));
						inGetHostEnable(szHostEnable);
						if (szHostEnable[0] != 'Y')
						{
							continue;
						}

						memset(szHostName, 0x00, sizeof(szHostName));
						inGetHostLabel(szHostName);
						if (memcmp(szHostName, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_)) == 0	||
						    memcmp(szHostName, _HOST_NAME_ESC_, strlen(_HOST_NAME_ESC_)) == 0	||
						    memcmp(szHostName, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_)) == 0)
						{
							continue;
						}

						/* 結帳失敗就不算進去 */
						memset(szMustSettleBit, 0x00, sizeof(szMustSettleBit));
						inGetMustSettleBit(szMustSettleBit);
						if (szMustSettleBit[0] != 'Y')
						{
							/* 表示起碼有一個主機成功 */
							uszSettle_SuccessBit = VS_TRUE;

							if (memcmp(szHostName, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_)) == 0)
							{
								inNCCC_Count = 0;
								inSqlite_Get_Table_Count(gszReprintDBPath, _TABLE_NAME_REPRINT_NCCC_, &inNCCC_Count);	/*取出重印明細筆數*/
								inHostCount = inNCCC_Count;
								inTotalCount += inHostCount;
								lnReprintBatchNum = atol(srReprintTitle.szNCCCReprintBatchNum);	/* 取出重印批號 */
							}
							else if (memcmp(szHostName, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_)) == 0)
							{
								inDCC_Count = 0;
								inSqlite_Get_Table_Count(gszReprintDBPath, _TABLE_NAME_REPRINT_DCC_, &inDCC_Count);	/*取出重印明細筆數*/
								inHostCount = inDCC_Count;
								inTotalCount += inHostCount;
								lnReprintBatchNum = atol(srReprintTitle.szDCCReprintBatchNum);	/* 取出重印批號 */
							}
							
							/* 列印紀錄 */
							inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "%03ld", lnReprintBatchNum);
							inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "%s", szHostName);
							inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_CENTER_);

							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "%03d", inHostCount);
							inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
						}
					}

					inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
					inPRINT_Buffer_PutIn("================================================", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					
					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
					inPRINT_Buffer_PutIn("合計", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%03d", inTotalCount);
					inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				}
				else
				{
					memset(szHostName, 0x00, sizeof(szHostName));
					inGetHostLabel(szHostName);
					/* 取出紀錄 */
					if (memcmp(szHostName, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_)) == 0)
					{
						lnReprintBatchNum = atol(srReprintTitle.szNCCCReprintBatchNum);	/* 取出重印批號 */
					}
					else if (memcmp(szHostName, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_)) == 0)
					{
						lnReprintBatchNum = atol(srReprintTitle.szDCCReprintBatchNum);	/* 取出重印批號 */
					}
					
					inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%03ld", lnReprintBatchNum);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
					
					memset(szHostName, 0x00, sizeof(szHostName));
					inGetHostLabel(szHostName);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", szHostName);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_CENTER_);
					
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%03d", inTotalCount);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		
					inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
					inPRINT_Buffer_PutIn("================================================", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					
					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
					inPRINT_Buffer_PutIn("合計", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%03d", inTotalCount);
					inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				}

				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn("================================================", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn("*** 列印完成 ***", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			
				inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);
				
				for (i = 0; i < 8; i++)
				{
					inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}

				if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer, &srBhandle)) != VS_SUCCESS)
					break;

				break;
			}while(1);
		}
	}
	else
	{
		/* 沒一個主機成功，就不印 */
		if (uszSettle_SuccessBit == VS_FALSE)
			return (VS_SUCCESS);

		/* 是否有列印功能 */
		if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
		{
			return (VS_SUCCESS);
		}
		else
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_);/* 處理中... */
			
			do
			{
				inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);
				srBhandle.uszDetailPrint = VS_TRUE;

				inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				
				/* 印NCC的LOGO */
				inRetVal = inPRINT_Buffer_PutGraphic((unsigned char*)_BANK_LOGO_, uszBuffer, &srBhandle, gsrBMPHeight.inBankLogoHeight, _APPEND_);
				if (inRetVal != VS_SUCCESS)
				{
					break;
				}
				
				/* 列印重印帳單總表*/
				/* \fr代表反白 */
				inPRINT_Buffer_PutIn("\fr                                ", _PRT_046_DHW_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("\fr重印帳單總表", _PRT_046_DHW_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

				/* 印商店名稱 */
				if (inCREDIT_PRINT_MerchantName(pobTran, uszBuffer, &srBhandle) != VS_SUCCESS)
				{
					break;
				}
				
				/* 連動結帳用NCCC的TID MID當顯示 */
				if (pobTran->uszAutoSettleBit == VS_TRUE || uszReprint == VS_TRUE)
				{
					inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inNCCCIndex);
					if (inLoadHDTRec(inNCCCIndex) < 0)
						break;
				}

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetMerchantID(szTemplate);

				/* 列印商店代號 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "商店代號");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
				
				inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_LEFT_);
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					break;

				/* Get端末機代號 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetTerminalID(szTemplate);

				/* 列印端末機代號 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "端末機代號");
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
				
				inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 13, _PADDING_LEFT_);
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					break;

				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;
				
				/* 日期時間 */
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inRetVal = inPRINT_Buffer_PutIn("日期/時間(Date/Time)", _PRT_046_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					break;

				memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
				sprintf(szPrintBuf1, "%.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
				if (inRetVal != VS_SUCCESS)
					break;
				
				/* 讀取上傳TMS成功和失敗筆數  */
				/* 讀出來 */
				inDUTY_FREEReturnSuccess_Cnt = atoi(srReprintTitle.szTMSUpdateSuccessNum);
				inDUTY_FREEReturnFail_Cnt = atoi(srReprintTitle.szTMSUpdateFailNum);
				inDUTY_FREEReturnSuccess_LastBatch_Cnt = atoi(srReprintTitle.szTMSUpdateSuccessNum);
				inDUTY_FREEReturnFail_LastBatch_Cnt = atoi(srReprintTitle.szTMSUpdateFailNum);

				memset(szHostName, 0x00, sizeof(szHostName));
				inGetHostLabel(szHostName);
				if (pobTran->uszAutoSettleBit == VS_TRUE)
				{
					inLoadHDTRec(inNCCCIndex);
					inLoadHDPTRec(inNCCCIndex);
					memset(szNCCCMustSettleBit, 0x00, sizeof(szNCCCMustSettleBit));
					inGetMustSettleBit(szNCCCMustSettleBit);
				}
				else
				{
					memset(szMustSettleBit, 0x00, sizeof(szMustSettleBit));
					inGetMustSettleBit(szMustSettleBit);
				}
				
				if (uszReprint == VS_TRUE)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "上傳成功");
					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						break;
					
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%03d  ", inDUTY_FREEReturnSuccess_LastBatch_Cnt);
					inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
					if (inRetVal != VS_SUCCESS)
						break;
					
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "筆");
					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
					if (inRetVal != VS_SUCCESS)
						break;
					
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "上傳失敗");
					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						break;
					
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%03d  ", inDUTY_FREEReturnFail_LastBatch_Cnt);
					inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
					if (inRetVal != VS_SUCCESS)
						break;
					
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "筆");
					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
					if (inRetVal != VS_SUCCESS)
						break;
					
					inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
					inRetVal = inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						break;
					
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "合計");
					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						break;
					
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%03d  ", inDUTY_FREEReturnSuccess_LastBatch_Cnt + inDUTY_FREEReturnFail_LastBatch_Cnt);
					inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
					if (inRetVal != VS_SUCCESS)
						break;
					
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "筆");
					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
					if (inRetVal != VS_SUCCESS)
						break;
				}
				else
				{
					/* 連動結帳f且NCCC結帳成功才印
					 * 或是單結NCCC成功 */
					if ((pobTran->uszAutoSettleBit == VS_TRUE && szNCCCMustSettleBit[0] != 'Y')	||
					    (pobTran->uszAutoSettleBit != VS_TRUE && 
					     memcmp(szHostName, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_)) == 0	&&
					     szMustSettleBit[0] != 'Y'))
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "上傳成功");
						inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							break;

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%03d  ", inDUTY_FREEReturnSuccess_Cnt);
						inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
						if (inRetVal != VS_SUCCESS)
							break;

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "筆");
						inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
						if (inRetVal != VS_SUCCESS)
							break;

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "上傳失敗");
						inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							break;

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%03d  ", inDUTY_FREEReturnFail_Cnt);
						inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
						if (inRetVal != VS_SUCCESS)
							break;

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "筆");
						inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
						if (inRetVal != VS_SUCCESS)
							break;

						inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
						inRetVal = inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							break;

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "合計");
						inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							break;

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%03d  ", inDUTY_FREEReturnSuccess_Cnt + inDUTY_FREEReturnFail_Cnt);
						inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
						if (inRetVal != VS_SUCCESS)
							break;

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "筆");
						inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
						if (inRetVal != VS_SUCCESS)
							break;

						/* NCCC主機結帳成功上傳筆數歸0 */
						inDUTY_FREEReturnSuccess_LastBatch_Cnt = inDUTY_FREEReturnSuccess_Cnt;
						inDUTY_FREEReturnFail_LastBatch_Cnt = inDUTY_FREEReturnFail_Cnt;
						inDUTY_FREEReturnSuccess_Cnt = 0;
						inDUTY_FREEReturnFail_Cnt = 0;
						/* 存起來 */
					}
				}
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);	
				inPRINT_Buffer_PutIn("批次號碼", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);	
				inPRINT_Buffer_PutIn("主機", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("筆數", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);

				if (pobTran->uszAutoSettleBit == VS_TRUE || uszReprint == VS_TRUE)
				{
					inTotalCount = 0;
					
					/* 算總比數 */
					for (inHostIndex = 0; ; inHostIndex ++)
					{
						if (inLoadHDTRec(inHostIndex) < 0)
							break;
						if (inLoadHDPTRec(inHostIndex) < 0)
							break;

						memset(szHostEnable, 0x00, sizeof(szHostEnable));
						inGetHostEnable(szHostEnable);
						if (szHostEnable[0] != 'Y')
						{
							continue;
						}

						memset(szHostName, 0x00, sizeof(szHostName));
						inGetHostLabel(szHostName);
						if (memcmp(szHostName, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_)) == 0	||
						    memcmp(szHostName, _HOST_NAME_ESC_, strlen(_HOST_NAME_ESC_)) == 0	||
						    memcmp(szHostName, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_)) == 0)
						{
							continue;
						}

						/* 結帳失敗就不算進去 */
						memset(szMustSettleBit, 0x00, sizeof(szMustSettleBit));
						inGetMustSettleBit(szMustSettleBit);
						if (szMustSettleBit[0] != 'Y')
						{
							/* 表示起碼有一個主機成功 */
							uszSettle_SuccessBit = VS_TRUE;

							if (memcmp(szHostName, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_)) == 0)
							{
								inNCCC_Count = 0;	/*取出重印明細筆數*/
								inHostCount = inNCCC_Count;
								inTotalCount += inHostCount;
								lnReprintBatchNum = atol(srReprintTitle.szNCCCReprintBatchNum);	/* 取出重印批號 */
							}
							else if (memcmp(szHostName, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_)) == 0)
							{
								inDCC_Count = 0;		/*取出重印明細筆數*/
								inHostCount = inDCC_Count;
								inTotalCount += inHostCount;
								lnReprintBatchNum = atol(srReprintTitle.szDCCReprintBatchNum);	/* 取出重印批號 */
							}
							
							/* 列印紀錄 */
							inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "%03ld", lnReprintBatchNum);
							inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "%s", szHostName);
							inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_CENTER_);

							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "%03d", inHostCount);
							inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
						}
					}

					inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
					inPRINT_Buffer_PutIn("================================================", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					
					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
					inPRINT_Buffer_PutIn("合計", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%03d", inTotalCount);
					inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				}
				else
				{
					memset(szHostName, 0x00, sizeof(szHostName));
					inGetHostLabel(szHostName);
					/* 取出紀錄 */
					if (memcmp(szHostName, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_)) == 0)
					{
						lnReprintBatchNum = atol(srReprintTitle.szNCCCReprintBatchNum);	/* 取出重印批號 */
					}
					else if (memcmp(szHostName, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_)) == 0)
					{
						lnReprintBatchNum = atol(srReprintTitle.szDCCReprintBatchNum);	/* 取出重印批號 */
					}
					
					inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%03ld", lnReprintBatchNum);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
					
					memset(szHostName, 0x00, sizeof(szHostName));
					inGetHostLabel(szHostName);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", szHostName);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_CENTER_);
					
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%03d", inTotalCount);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		
					inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
					inPRINT_Buffer_PutIn("================================================", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					
					inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
					inPRINT_Buffer_PutIn("合計", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%03d", inTotalCount);
					inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				}

				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn("================================================", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
				inPRINT_Buffer_PutIn("*** 列印完成 ***", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

				inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);
				
				for (i = 0; i < 8; i++)
				{
					inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}

				if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer, &srBhandle)) != VS_SUCCESS)
					break;
				break;
			}while(1);
		}/* 列印能力 */
	}/* totalcnt */
		
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
	
	if (inOrgHDTIndex >= 0)
	{
		pobTran->srBRec.inHDTIndex = inOrgHDTIndex;
		inLoadHDTRec(pobTran->srBRec.inHDTIndex);
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inCREDIT_PRINT_Reprint_DetailReport_Title_ByBuffer() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Dutyfree_NCCC_DetailReport_ByBuffer
Date&Time       :2022/5/13 下午 2:02
Describe        :昇恆昌客製化 重印簽單明細
*/
int inCREDIT_PRINT_Dutyfree_NCCC_DetailReport_ByBuffer(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = 0, inPrintIndex = 0, inRecordCnt = 0;
	char			szDebugMsg[100 + 1];
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
	else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_);/* 處理中... */

		inPrintIndex = _DETAIL_REPORT_DUTYFREE_REPRINT_NCCC_INDEX_;

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "PrintIndex : %d", inPrintIndex);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 檢查是否有帳 */
		if (srReprintDetail[inPrintIndex].inReportCheck != NULL)
		{
			if ((inRecordCnt = srReprintDetail[inPrintIndex].inReportCheck(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) == VS_ERROR)
				return (VS_ERROR); /* 表示檔案開啟失敗 */
		}

		while (1)
		{
			inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
			srBhandle1.uszDetailPrint = VS_TRUE;
			
			/* 列印LOGO */
			if (srReprintDetail[inPrintIndex].inReportLogo != NULL)
				if ((inRetVal = srReprintDetail[inPrintIndex].inReportLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 列印TID MID */
			if (srReprintDetail[inPrintIndex].inReportTop != NULL)
				if ((inRetVal = srReprintDetail[inPrintIndex].inReportTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			/* 明細規格 */
			if (srReprintDetail[inPrintIndex].inMiddle != NULL)
				if ((inRetVal = srReprintDetail[inPrintIndex].inMiddle(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 明細資料 */
			if (srReprintDetail[inPrintIndex].inBottom != NULL)
			{
				inRetVal = srReprintDetail[inPrintIndex].inBottom(pobTran, inRecordCnt, uszBuffer1, &srFont_Attrib1, &srBhandle1);
				if (inRetVal != VS_SUCCESS &&
				    inRetVal != VS_NO_RECORD)
				{
				       return (inRetVal);
				}
			}

			/* 結束 */
			if (srReprintDetail[inPrintIndex].inReportEnd != NULL)
				if ((inRetVal = srReprintDetail[inPrintIndex].inReportEnd(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);
			
			if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);

			break;
		}

		return (VS_SUCCESS);
	}
}


/*
Function        :inCREDIT_PRINT_Dutyfree_Reprint_Check_ByBuffer
Date&Time       :2022/5/13 下午 2:30
Describe        :
*/
int inCREDIT_PRINT_Dutyfree_Reprint_Check_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal = VS_SUCCESS;
        int	inRecordCnt = 0;
	char	szTableName[20 + 1] = {0};
	char	szTRTFileName[12 + 1] = {0};
	
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	if (!memcmp(szTRTFileName, _TRT_FILE_NAME_CREDIT_, max(strlen(szTRTFileName), strlen(_TRT_FILE_NAME_CREDIT_))))
	{
		sprintf(szTableName, _TABLE_NAME_REPRINT_NCCC_);
	}
	else if (!memcmp(szTRTFileName, _TRT_FILE_NAME_DCC_, max(strlen(szTRTFileName), strlen(_TRT_FILE_NAME_DCC_))))
	{
		sprintf(szTableName, _TABLE_NAME_REPRINT_DCC_);
	}
	else
	{
		sprintf(szTableName, _TABLE_NAME_REPRINT_NCCC_);
	}
	
	do
	{
		inRetVal = inSqlite_Check_Table_Exist(gszReprintDBPath, szTableName);
		if (inRetVal != VS_SUCCESS)
		{
			inRecordCnt = 0;
			break;
		}
		
		inRetVal = inSqlite_Get_Table_Count(gszReprintDBPath, szTableName, &inRecordCnt);
		if (inRetVal != VS_SUCCESS)
		{
			inRecordCnt = 0;
			break;
		}
		break;
	}while(1);
	
	
        return (inRecordCnt);
}

/*
Function        :inCREDIT_PRINT_Dutyfree_Reprint_Logo_ByBuffer
Date&Time       :2022/5/13 下午 3:04
Describe        :
*/
int inCREDIT_PRINT_Dutyfree_Reprint_Logo_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szShort_Receipt_Mode[1 + 1] = {0};
	
        memset(szShort_Receipt_Mode, 0x00, sizeof(szShort_Receipt_Mode));
        inGetShort_Receipt_Mode(szShort_Receipt_Mode);
        
         if (!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))
        {
                /* 印NCC的LOGO */
                if (inPRINT_Buffer_PutIn(_NCCC_TEXT_LOGO_, _FONT_SIZE_SHORT_RECEIPT_U_NCCC_LOGO_TEXT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_) != VS_SUCCESS)
                {
                        return (VS_ERROR);
                }
        }
        else
        {
                /* 印NCC的LOGO */
                if (inPRINT_Buffer_PutGraphic((unsigned char*)_BANK_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inBankLogoHeight, _APPEND_) != VS_SUCCESS)
                {
                        return (VS_ERROR);
                }
        }
	
	/* 列印"重印帳單明細" */
	/* \fr代表反白 */
	inPRINT_Buffer_PutIn("\fr                                ", _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_CENTER_);
	inPRINT_Buffer_PutIn("\fr重印帳單明細", _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	
	/* 印商店名稱 */
        if (inCREDIT_PRINT_MerchantName(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}


/*
Function        :inCREDIT_PRINT_Dutyfree_Reprint_Top_ByBuffer
Date&Time       :2022/5/13 下午 3:42
Describe        :
*/
int inCREDIT_PRINT_Dutyfree_Reprint_Top_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int			inRetVal = VS_SUCCESS;
        char			szPrintBuf[84 + 1] = {0}, szTemplate[42 + 1] = {0};
	char			szTableName[30 + 1] = {0};
	unsigned char		uszNCCCBit = VS_FALSE;
	unsigned char		uszDCCBit = VS_FALSE;
	DUTYFREE_REPRINT_TITLE	srReprintTitle;
	SQLITE_ALL_TABLE	srALL;
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetHostLabel(szTemplate);
	if (memcmp(szTemplate, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_)) == 0)
	{
		uszNCCCBit = VS_TRUE;
	}
	else if (memcmp(szTemplate, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_)) == 0)
	{
		uszDCCBit = VS_TRUE;
	}
	
	memset(&srReprintTitle, 0x00, sizeof(DUTYFREE_REPRINT_TITLE));
	memset(&srALL, 0x00, sizeof(SQLITE_ALL_TABLE));
	inBatch_Table_Link_Reprint_Title(&srReprintTitle, &srALL, _LS_READ_);
	sprintf(szTableName, "%s", _TABLE_NAME_REPRINT_TITLE_);
	inSqlite_Get_Table_ByRecordID_All(gszReprintDBPath, szTableName, 0, &srALL);

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

        /* 列印日期 / 時間 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inRetVal = inPRINT_Buffer_PutIn("日期/時間(Date/Time)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	if (uszNCCCBit == VS_TRUE)
	{
		sprintf(szPrintBuf, "%s", srReprintTitle.szNCCCReprintTitleDateTime);
	}
	else if (uszDCCBit == VS_TRUE)
	{
		sprintf(szPrintBuf, "%s", srReprintTitle.szDCCReprintTitleDateTime);
	}
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        /* 列印批次號碼 */
        inRetVal = inPRINT_Buffer_PutIn("批次號碼(Batch No.)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	/* 只取後3碼 */
        if (uszNCCCBit == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, &srReprintTitle.szNCCCReprintBatchNum[3], 3);
		if (atoi(szTemplate) > 0)
		{
			sprintf(szPrintBuf, "%s", szTemplate);
		}
		else
		{
			sprintf(szPrintBuf, "%03d", 0);
		}
	}
	else if (uszDCCBit == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, &srReprintTitle.szDCCReprintBatchNum[3], 3);
		if (atoi(szTemplate) > 0)
		{
			sprintf(szPrintBuf, "%s", szTemplate);
		}
		else
		{
			sprintf(szPrintBuf, "%03d", 0);
		}
	}
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
        /* 列印主機 */
        inRetVal = inPRINT_Buffer_PutIn("主機(Host)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        inGetHostLabel(szPrintBuf);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Dutyfree_Reprint_NCCC_DetailReportMiddle_ByBuffer
Date&Time       :2022/5/13 下午 6:17
Describe        :
*/
int inCREDIT_PRINT_Dutyfree_Reprint_NCCC_DetailReportMiddle_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char    szFuncEnable[1 + 1];			/* catch Y or N */
	
	inPRINT_Buffer_PutIn("明細報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

	inPRINT_Buffer_PutIn("調閱編號", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn_Specific_X_Position("交易金額", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

	inPRINT_Buffer_PutIn("交易類別", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("卡別", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("卡號", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	inPRINT_Buffer_PutIn("交易日期", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn_Specific_X_Position("交易時間", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

	/* SmartPay要印調單編號 */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetFiscFuncEnable(szFuncEnable);
	if (szFuncEnable[0] == 'Y')
	{
		inPRINT_Buffer_PutIn("授權碼/調單編號", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	}
	else
	{
		inPRINT_Buffer_PutIn("授權碼", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	}
	inPRINT_Buffer_PutIn_Specific_X_Position("檢查碼", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

	/* 銀聯功能有開才印回覆碼 */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetCUPFuncEnable(szFuncEnable);
	if (szFuncEnable[0] == 'Y')
	{
		inPRINT_Buffer_PutIn("回覆碼", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	/* 櫃號功能有開才印櫃號 */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetStoreIDEnable(szFuncEnable);
	if (szFuncEnable[0] == 'Y')
		inPRINT_Buffer_PutIn("櫃號", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	inPRINT_Buffer_PutIn("重列印時間", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Dutyfree_Reprint_DetailReportBottom_ByBuffer
Date&Time       :2022/5/13 下午 6:28
Describe        :
*/
int inCREDIT_PRINT_Dutyfree_Reprint_DetailReportBottom_ByBuffer(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inReadCnt = 0;
	int	inRetVal = VS_SUCCESS;
        char	szPrintBuf[62 + 1] = {0}, szTemplate1[62 + 1] = {0};
	char	szQuerySql[200 + 1] = {0};
	char	szTableName[30 + 1] = {0};
	char	szTRTFileName[12 + 1] = {0};
	DUTYFREE_REPRINT_DATA	srReprintData;
	SQLITE_ALL_TABLE	srAll;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inCREDIT_PRINT_Dutyfree_Reprint_DetailReportBottom_ByBuffer()_START");
	}
	
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);
	
	/* 開始讀取 */
	memset(&srReprintData, 0x00, sizeof(DUTYFREE_REPRINT_DATA));
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	memset(szTableName, 0x00, sizeof(szTableName));
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	if (!memcmp(szTRTFileName, _TRT_FILE_NAME_CREDIT_, max(strlen(szTRTFileName), strlen(_TRT_FILE_NAME_CREDIT_))))
	{
		sprintf(szTableName, _TABLE_NAME_REPRINT_NCCC_);
	}
	else if (!memcmp(szTRTFileName, _TRT_FILE_NAME_DCC_, max(strlen(szTRTFileName), strlen(_TRT_FILE_NAME_DCC_))))
	{
		sprintf(szTableName, _TABLE_NAME_REPRINT_DCC_);
	}
	
	sprintf(szQuerySql, "SELECT * FROM %s", szTableName);
	inSqlite_Get_Data_Enormous_Search(gszReprintDBPath, szQuerySql);
	/* 預設為無須重找 */
	guszEnormousNoNeedResetBit = VS_TRUE;

	if (inRecordCnt == 0)
	{
		inPRINT_Buffer_PutIn("本日無重印帳單資料", _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		inPRINT_Buffer_PutIn(" ", _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	else
	{
		for (inReadCnt = 0; inReadCnt < inRecordCnt; inReadCnt ++)
		{
			/*. 開始讀取每一筆交易記錄 .*/
			memset(&srAll, 0x00, sizeof(SQLITE_ALL_TABLE));
			inBatch_Table_Link_Reprint_Data(&srReprintData, &srAll, _LS_READ_);
			if (inSqlite_Get_Data_Enormous_Get(&srAll, inReadCnt) != VS_SUCCESS)
			{
				inRetVal = VS_ERROR;
				break;
			}

			/* 昇恆昌沒有優惠兌換 */
			/* Invoice Number */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			sprintf(szTemplate1, "%s", srReprintData.szINV_Data);
			strcat(szPrintBuf, szTemplate1);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* Print Amount */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			sprintf(szTemplate1, "%s", srReprintData.szAmount);
			strcat(szPrintBuf, szTemplate1);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* Trans Type */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", srReprintData.szTransType);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* 卡別 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", srReprintData.szPrintCardType);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* 卡號 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", srReprintData.szPrintPAN);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* Trans Date Time */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", srReprintData.szDate);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", srReprintData.szTime);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* Approved No. & RRN NO. */
			/* SmartPay印調單編號 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcat(szPrintBuf, srReprintData.szAuthCode);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* 檢查碼Check No */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s",srReprintData.szChekNo);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* RESPONSE CODE */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcat(szPrintBuf, srReprintData.szReponseCode);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* Store ID */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", srReprintData.szStoreID);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* Reprint Date Time */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", srReprintData.szReprintDate);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				break;

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", srReprintData.szReprintTime);
			inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_046_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				break;

			/* 最後一行不用印線 */
			if ((inReadCnt + 1) < inRecordCnt)
			{
				inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		} /* End for () .... */
	}
	
	/* 結束讀取 */
	inSqlite_Get_Data_Enormous_Free();

	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inCREDIT_PRINT_Dutyfree_Reprint_DetailReportBottom_ByBuffer()_END");
	}
	
        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_Dutyfree_Reprint_End_ByBuffer
Date&Time       :2022/5/13 下午 6:23
Describe        :列印結尾
*/
int inCREDIT_PRINT_Dutyfree_Reprint_End_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
	
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);
	
	inPRINT_Buffer_PutIn("==========================================", _PRT_046_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
        inPRINT_Buffer_PutIn("*** 列印完成 ***", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

        for (i = 0; i < 8; i++)
	{
		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Dutyfree_ReceiptEND_ByBuffer
Date&Time       :2022/5/23 上午 9:22
Describe        :昇恆昌使用 075要將X放大
*/
int inCREDIT_PRINT_Dutyfree_ReceiptEND_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
	int	inRetVal = VS_ERROR;
	char	szTemplate[42 + 1]= {0};
	char	szSignature[30 + 1] = {0};
	char	szSignaturePath[80 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	char	szCustomIndicator[3 + 1] = {0};
	
	memset(szCustomIndicator, 0x00, sizeof(szCustomIndicator));
	inGetCustomIndicator(szCustomIndicator);
	
        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                inPRINT_Buffer_PutIn("簽名欄:_____________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
		    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
                {
                        inPRINT_Buffer_PutIn("*** 商店收據 Merchant Copy ***", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_CUST_;

                }
                else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
                {
                        inPRINT_Buffer_PutIn("*** 持卡人收據 Customer Copy ***", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_MERCH_;
                }

                inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
				inPRINT_Buffer_PutIn("免      簽      名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
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
						inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					}
					
				}	
			}
                        
			if (!memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
			    !memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inPRINT_Buffer_BAUL_SetFont_Style(_BAUL_FONT_STYLE_BOLD_);
				inPRINT_Buffer_PutIn("X:________________________________", _PRT_DUTY_FREE_075_X_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_BAUL_SetFont_Style(gusBAULFontStyleRegular);
			}
			else
			{
				inPRINT_Buffer_PutIn("X:________________________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			/* 持卡人姓名 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%s", pobTran->srBRec.szCardHolder);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        inPRINT_Buffer_PutIn("持卡人簽名(特店存根聯)", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
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
				/* 免簽名 */
				inRetVal = inPRINT_Buffer_PutIn("免      簽      名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        inPRINT_Buffer_PutIn("　　　　　　　　　 商店存根", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 持卡人姓名 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate,"%s",pobTran->srBRec.szCardHolder);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
				/* 免簽名 */
				inRetVal = inPRINT_Buffer_PutIn("免      簽      名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        inPRINT_Buffer_PutIn("　　　　　　　　　 持卡人存根", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("　　　　　　　  Card holder stub", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 持卡人姓名 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate,"%s",pobTran->srBRec.szCardHolder);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
		
		if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
		{
			inPRINT_Buffer_PutIn("重印 REPRINT", _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		}
		
                /* 列印警示語 */
                inPRINT_Buffer_PutIn("            I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("        ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		/* Print Notice */
		if (inCREDIT_PRINT_Notice(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
			return (VS_ERROR);
		
		/* Print Slogan */
		if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
		{
			if (inCREDIT_PRINT_MarchantSlogan(pobTran, _NCCC_SLOGAN_PRINT_DOWN_, uszBuffer, srBhandle) != VS_SUCCESS)
				return (VS_ERROR);
			
			/* 昇恆昌版本不列印優惠資訊 */
		}
		
                for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
        }

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Dutyfree_Reprint_Cavas_File_ByBuffer
Date&Time       :2022/5/27 上午 11:27
Describe        :昇恆昌客製化 可重印存成檔案的Cavas Buffer
*/
int inCREDIT_PRINT_Dutyfree_Reprint_Cavas_File_ByBuffer(unsigned char* uszFileName)
{
	long			lnFileSize = 0;
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_] = {0};
	unsigned long		ulHandle = 0;
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inCREDIT_PRINT_Dutyfree_Reprint_Cavas_File_ByBuffer(%s) START !", uszFileName);
	}
	
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_);/* 處理中... */

		/* 檢查是否有帳 */
		if (inFILE_Check_Exist(uszFileName) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		else
		{
			inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
		
			/* 開啟重印buffer檔 */
			inFILE_OpenReadOnly(&ulHandle, uszFileName);

			/* 取得檔案長度 */
			lnFileSize = lnFILE_GetSize(&ulHandle, uszFileName);

			/* 讀取至buffer */
			inFILE_Read(&ulHandle, uszBuffer1, lnFileSize);

			/* 關閉檔案 */
			inFILE_Close(&ulHandle);

			srBhandle1.inYcover = lnFileSize / PB_CANVAS_X_SIZE;

			while (1)
			{
				inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

				inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1);

				break;
			}
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inCREDIT_PRINT_Dutyfree_Reprint_Cavas_File_ByBuffer() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Dutyfree_Reprint_Report_NCCC_First
Date&Time       :2022/6/14 下午 6:14
Describe        :僅用於新機沒有帳產生的重印帳單
*/
int inCREDIT_PRINT_Dutyfree_Reprint_Report_NCCC_First(TRANSACTION_OBJECT *pobTran, unsigned char* uszBuffer1, int* inYLen)
{
	int			inRetVal = 0, inPrintIndex = 0;
	int			inOrgIndex = 0;
	int			inHGIndex = -1;
	char			szFuncEnable[2 + 1] = {0};
	char			szDebugMsg[100 + 1] = {0};
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
	ACCUM_TOTAL_REC		srAccumRec;
	HG_ACCUM_TOTAL_REC	srHGAccumRec;
	
	memset(&srAccumRec, 0x00, sizeof(srAccumRec));
	memset(&srHGAccumRec, 0x00, sizeof(srHGAccumRec));
	
	inPrintIndex = _TOTAL_REPORT_INDEX_046_NCCC;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "PrintIndex : %d", inPrintIndex);
		inLogPrintf(AT, szDebugMsg);
	}

	if (pobTran->uszEverRich_NoDataBit != VS_TRUE)
	{
		/* 開啟交易總的檔案 */
		memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		inRetVal = inACCUM_GetRecord(pobTran, &srAccumRec);

		if (inRetVal == VS_NO_RECORD)
			memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		else if (inRetVal == VS_ERROR)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Get record 失敗.");	
			}

			return (VS_ERROR);
		}

		/* 開啟HappyGo交易總額檔案 */
		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_HG_, &inHGIndex);
		if (inHGIndex == -1)
		{

		}
		else
		{
			/* 轉成HG */
			inOrgIndex = pobTran->srBRec.inHDTIndex;
			pobTran->srBRec.inHDTIndex = inHGIndex;
			inLoadHDTRec(pobTran->srBRec.inHDTIndex);
			inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

			memset(&srHGAccumRec, 0x00, sizeof(srHGAccumRec));
			inRetVal = inHG_GetRecord(pobTran, &srHGAccumRec);

			/* 轉回來 */
			pobTran->srBRec.inHDTIndex = inOrgIndex;
			inLoadHDTRec(pobTran->srBRec.inHDTIndex);
			inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

			if (inRetVal == VS_NO_RECORD)
				memset(&srHGAccumRec, 0x00, sizeof(srHGAccumRec));
			else if (inRetVal == VS_ERROR)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "Get HG record 失敗.");	
				}

				return (VS_ERROR);
			}
		}
	}
	
	inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
	/* 列印LOGO */
	if (srTotalReport_ByBuffer[inPrintIndex].inReportLogo != NULL)
		if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inReportLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
			return (inRetVal);
	/* 列印TID MID */
	if (srTotalReport_ByBuffer[inPrintIndex].inReportTop != NULL)
		if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inReportTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
			return (inRetVal);
	/* 全部金額總計 */
	if (srTotalReport_ByBuffer[inPrintIndex].inAmount != NULL)
		if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmount(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
			return (inRetVal);

	/* 有金額才印 */
	if (srAccumRec.llTotalSaleAmount != 0L || srAccumRec.llTotalRefundAmount != 0L)
	{
		/* 卡別金額總計 */
		if (srTotalReport_ByBuffer[inPrintIndex].inAmountByCard != NULL)
			if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmountByCard(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);
		/* 有分期記錄才印 */
		if (srAccumRec.lnInstTotalCount > 0)
		{
			/* 分期金額總計 */
			if (srTotalReport_ByBuffer[inPrintIndex].inAmountByInstallment != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmountByInstallment(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
		}

		/* 有紅利記錄才印 */
		if (srAccumRec.lnRedeemTotalCount > 0)
		{
			/* 紅利金額總計 */
			if (srTotalReport_ByBuffer[inPrintIndex].inAmountByRedemption != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmountByRedemption(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
		}

	}

	/* 上傳電子簽單至ESC系統 */
	if (srTotalReport_ByBuffer[inPrintIndex].inAmountByOther != NULL)
		srTotalReport_ByBuffer[inPrintIndex].inAmountByOther(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1);

	/* HappyGo */
	inHG_GetHG_Enable(pobTran->srBRec.inHDTIndex, szFuncEnable);
	if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
	{
		/* 印HG總額 */
		if (srTotalReport_ByBuffer[inPrintIndex].inAmountHG != NULL)
			if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmountHG(pobTran, &srHGAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);
	}

	/* 結束 */
	if (srTotalReport_ByBuffer[inPrintIndex].inReportEnd != NULL)
		if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inReportEnd(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
			return (inRetVal);
	
	if (srBhandle1.inYcurrent >= srBhandle1.inYcover)
	{
		*inYLen = srBhandle1.inYcurrent;
	}
	else
	{
		*inYLen = srBhandle1.inYcover;
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Dutyfree_Reprint_Report_DCC_First
Date&Time       :2022/6/14 下午 6:14
Describe        :僅用於新機沒有帳產生的重印帳單
*/
int inCREDIT_PRINT_Dutyfree_Reprint_Report_DCC_First(TRANSACTION_OBJECT *pobTran, unsigned char* uszBuffer1, int* inYLen)
{
	int			inRetVal = 0, inPrintIndex = 0;
	char			szDebugMsg[100 + 1] = {0};
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
	ACCUM_TOTAL_REC		srAccumRec;
	
	memset(&srAccumRec, 0x00, sizeof(srAccumRec));
	
	inPrintIndex = _TOTAL_REPORT_INDEX_046_NCCC;

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "PrintIndex : %d", inPrintIndex);
		inLogPrintf(AT, szDebugMsg);
	}

	if (pobTran->uszEverRich_NoDataBit != VS_TRUE)
	{
		/* 開啟交易總的檔案 */
		memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		inRetVal = inACCUM_GetRecord(pobTran, &srAccumRec);

		if (inRetVal == VS_NO_RECORD)
			memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		else if (inRetVal == VS_ERROR)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Get record 失敗.");	
			}

			return (VS_ERROR);
		}
	}
		
	inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
	/* 列印LOGO */
	if (srTotalReport_ByBuffer[inPrintIndex].inReportLogo != NULL)
		srTotalReport_ByBuffer[inPrintIndex].inReportLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1);
	/* 列印TID MID */
	if (srTotalReport_ByBuffer[inPrintIndex].inReportTop != NULL)
		srTotalReport_ByBuffer[inPrintIndex].inReportTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1);
	/* 全部金額總計 */
	if (srTotalReport_ByBuffer[inPrintIndex].inAmount != NULL)
		srTotalReport_ByBuffer[inPrintIndex].inAmount(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1);

	/* 有金額才印 */
	if (srAccumRec.llTotalSaleAmount != 0L || srAccumRec.llTotalRefundAmount != 0L)
	{
		/* 卡別金額總計 */
		if (srTotalReport_ByBuffer[inPrintIndex].inAmountByCard != NULL)
			srTotalReport_ByBuffer[inPrintIndex].inAmountByCard(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1);
	}

	/* 上傳電子簽單至ESC系統 */
	if (srTotalReport_ByBuffer[inPrintIndex].inAmountByOther != NULL)
		srTotalReport_ByBuffer[inPrintIndex].inAmountByOther(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1);

	/* 結束 */
	if (srTotalReport_ByBuffer[inPrintIndex].inReportEnd != NULL)
		srTotalReport_ByBuffer[inPrintIndex].inReportEnd(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1);
	
	if (srBhandle1.inYcurrent >= srBhandle1.inYcover)
	{
		*inYLen = srBhandle1.inYcurrent;
	}
	else
	{
		*inYLen = srBhandle1.inYcover;
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Dutyfree_Reprint_Report_ESVC_First
Date&Time       :2022/6/14 下午 6:14
Describe        :僅用於新機沒有帳產生的重印帳單
*/
int inCREDIT_PRINT_Dutyfree_Reprint_Report_ESVC_First(TRANSACTION_OBJECT *pobTran, unsigned char* uszBuffer1, int* inYLen)
{
	int			inRetVal = 0, inPrintIndex = 0;
	char			szDebugMsg[100 + 1] = {0};
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
	TICKET_ACCUM_TOTAL_REC	srAccumRec;
	
	memset(&srAccumRec, 0x00, sizeof(srAccumRec));
	
	inPrintIndex = _TOTAL_REPORT_INDEX_046_ESVC_SETTLE_;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "PrintIndex : %d", inPrintIndex);
		inLogPrintf(AT, szDebugMsg);
	}

	if (pobTran->uszEverRich_NoDataBit != VS_TRUE)
	{
		/* 開啟交易總的檔案 */
		memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		inRetVal = inACCUM_GetRecord_ESVC(pobTran, &srAccumRec);
		if (inRetVal == VS_NO_RECORD)
		{
			memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		}
		else if (inRetVal == VS_ERROR)
		{
			return (VS_ERROR);
		}
	}

	inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
	/* 列印LOGO */
	if (srTotalReport_ByBuffer_ESVC[inPrintIndex].inReportLogo != NULL)
		srTotalReport_ByBuffer_ESVC[inPrintIndex].inReportLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1);
	/* 列印TID MID */
	if (srTotalReport_ByBuffer_ESVC[inPrintIndex].inReportTop != NULL)
		srTotalReport_ByBuffer_ESVC[inPrintIndex].inReportTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1);
	/* 全部金額總計 */
	if (srTotalReport_ByBuffer_ESVC[inPrintIndex].inAmount != NULL)
		srTotalReport_ByBuffer_ESVC[inPrintIndex].inAmount(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1);

	/* 卡別金額總計 */
	if (srTotalReport_ByBuffer_ESVC[inPrintIndex].inAmountByCard != NULL)
		srTotalReport_ByBuffer_ESVC[inPrintIndex].inAmountByCard(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1);

	/* 結束 */
	if (srTotalReport_ByBuffer_ESVC[inPrintIndex].inReportEnd != NULL)
		srTotalReport_ByBuffer_ESVC[inPrintIndex].inReportEnd(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1);

	
	if (srBhandle1.inYcurrent >= srBhandle1.inYcover)
	{
		*inYLen = srBhandle1.inYcurrent;
	}
	else
	{
		*inYLen = srBhandle1.inYcover;
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_ReceiptEND_ByBuffer_ESVC_046
Date&Time       :2022/7/4 下午 2:05
Describe        :昇恆昌版本不顯示優惠兌換
*/
int inCREDIT_PRINT_ReceiptEND_ByBuffer_ESVC_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
	int	inRetVal = VS_SUCCESS;
	char	szDemoMode[2 + 1] = {0};
	
        if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
        {
                inRetVal = inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                inRetVal = inPRINT_Buffer_PutIn("備註：若有疑問請洽一卡通票證公司", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                inRetVal = inPRINT_Buffer_PutIn("客服專線：(07)791-2000", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
		inRetVal = inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                inRetVal = inPRINT_Buffer_PutIn("備註：若有疑問請洽", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                inRetVal = inPRINT_Buffer_PutIn("悠遊卡公司專線：412-8880", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		inRetVal = inPRINT_Buffer_PutIn("(手機及金馬地區請加02)", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
	{
		inRetVal = inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                inRetVal = inPRINT_Buffer_PutIn("備註：若有疑問請洽愛金卡公司", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                inRetVal = inPRINT_Buffer_PutIn("客服專線：0800-233-888", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	
	/* 教育訓練模式圖案列印 */
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
	
        if (pobTran->srTRec.inPrintOption == _PRT_MERCH_)
        {
                inRetVal = inPRINT_Buffer_PutIn("------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		inRetVal = inPRINT_Buffer_PutIn("商店存根聯", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	else if (pobTran->srTRec.inPrintOption == _PRT_CUST_)
	{
	        inRetVal = inPRINT_Buffer_PutIn("------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		inRetVal = inPRINT_Buffer_PutIn("顧客收執聯", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
	{
		inRetVal = inPRINT_Buffer_PutIn("重印 REPRINT", _PRT_046_DHW_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	/* Print Notice */
	if (inCREDIT_PRINT_Notice(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	/* 票正要印下面slogan by Russell 2018/12/6 上午 10:25 */
	/* Print Slogan */
	if (pobTran->srTRec.inPrintOption == _PRT_CUST_)
	{
		if (inCREDIT_PRINT_MarchantSlogan(pobTran, _NCCC_SLOGAN_PRINT_DOWN_, uszBuffer, srBhandle) != VS_SUCCESS)
			return (VS_ERROR);
	}
	
        /* 列印空白行 */
	for (i = 0; i < 8; i++)
	{
		inRetVal = inPRINT_Buffer_PutIn(" ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

        return (VS_SUCCESS);      
}

/*
Function        :inCREDIT_PRINT_ReceiptEND_ByBuffer_027
Date&Time       :2022/7/11 上午 10:53
Describe        :中華電信要列印警語
*/
int inCREDIT_PRINT_ReceiptEND_ByBuffer_027(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
	int	inRetVal = VS_ERROR;
	char	szTemplate[42 + 1]= {0};
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
                        inPRINT_Buffer_PutIn("*** 商店收據 Merchant Copy ***", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_CUST_;

                }
                else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
                {
                        inPRINT_Buffer_PutIn("*** 持卡人收據 Customer Copy ***", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_MERCH_;
                }

                inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
        }
        else
        {
		if (pobTran->srBRec.inCode == _VOID_ || pobTran->srBRec.inCode == _CUP_VOID_ ||
		    pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ ||
		    pobTran->srBRec.inCode == _REDEEM_REFUND_ || pobTran->srBRec.inCode == _CUP_REFUND_ ||
		    pobTran->srBRec.inCode == _CUP_PRE_AUTH_VOID_ || pobTran->srBRec.inCode == _CUP_PRE_COMP_VOID_ ||
		    pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_) 
		/* 取消銀聯預授與取消銀聯預授完成不需印警語  */
		{
			
		}
		else
		{
			inPRINT_Buffer_PutIn("", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			inPRINT_Buffer_PutIn("退貨請攜帶本簽單及原交易", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			inPRINT_Buffer_PutIn("卡片與商品至營業櫃檯辦理", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			inPRINT_Buffer_PutIn("", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		}
		
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
				inPRINT_Buffer_PutIn("免      簽      名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
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
						inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					}
					
				}	
			}
                        
                        inPRINT_Buffer_PutIn("X:________________________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 持卡人姓名 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%s", pobTran->srBRec.szCardHolder);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        inPRINT_Buffer_PutIn("持卡人簽名(特店存根聯)", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
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
				/* 免簽名 */
				inRetVal = inPRINT_Buffer_PutIn("免      簽      名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        inPRINT_Buffer_PutIn("　　　　　　　　　 商店存根", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 持卡人姓名 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate,"%s",pobTran->srBRec.szCardHolder);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
				/* 免簽名 */
				inRetVal = inPRINT_Buffer_PutIn("免      簽      名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        inPRINT_Buffer_PutIn("　　　　　　　　　 持卡人存根", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("　　　　　　　  Card holder stub", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 持卡人姓名 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate,"%s",pobTran->srBRec.szCardHolder);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
		
		if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
		{
			inPRINT_Buffer_PutIn("           重印 REPRINT", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
                /* 列印警示語 */
                inPRINT_Buffer_PutIn("            I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("        ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
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
		
                for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
        }

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Reversal_Print
Date&Time       :2022/7/11 上午 10:53
Describe        :中華電信要列印警語
*/
int inCREDIT_PRINT_Reversal_Print(TRANSACTION_OBJECT *pobTran)
{
	int	i = 0;
	char	szPrintBuf[100 + 1] = {0}, szPrintBuf1[42 + 1] = {0};
	char 	szTemplate[42 + 1] = {0}, szTemplate1[42 + 1] = {0};
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_] = {0};
	BufferHandle		srBhandle;
	FONT_ATTRIB		srFont_Attrib;

	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);
		
		inCREDIT_PRINT_Logo_ByBuffer(pobTran, uszBuffer, &srFont_Attrib, &srBhandle);

		inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		inCREDIT_PRINT_Tidmid_ByBuffer(pobTran, uszBuffer, &srFont_Attrib, &srBhandle);
		
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			inPRINT_Buffer_PutIn("發卡行代碼", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			
			memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
			sprintf(szPrintBuf, "%c%c%c", pobTran->srBRec.szFiscIssuerID[0], pobTran->srBRec.szFiscIssuerID[1], pobTran->srBRec.szFiscIssuerID[2]);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			inPRINT_Buffer_PutIn("城市別(City)", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			
			memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
			inGetCityName(szPrintBuf1);
			sprintf(szPrintBuf, "%s", szPrintBuf1);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		/* 卡別 & 檢查碼 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			inPRINT_Buffer_PutIn("卡別(Card Type)", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			
			inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 12, _PADDING_RIGHT_);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			inPRINT_Buffer_PutIn("卡別(Card Type)", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn_Specific_X_Position("檢查碼(Check No.)", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			
			inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 12, _PADDING_RIGHT_);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			/* 檢查碼 */
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
			/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
			if (strlen(pobTran->srBRec.szCheckNO) > 0)
			{
				strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
			}
			else
			{
				inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
			}
			memcpy(szPrintBuf1, szTemplate1, strlen(szTemplate1));
			inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		}

		/* 卡號掩飾 NCCC 規格以為準 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                strcpy(szPrintBuf, pobTran->srBRec.szPAN);
                inPRINT_Buffer_PutIn("卡號(Card No.)", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                strcpy(szPrintBuf, pobTran->srBRec.szPAN);
		
		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		/* 卡號遮掩(一般卡號前6後4，U Card前3後5) */
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
		if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			/* Uny交易不受TMS開關限制，且獲取卡號時已遮掩 */
		}
		else
		{
			if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				for (i = 6; i < (strlen(szPrintBuf) - 4); i ++)
				{
					szPrintBuf[i] = 0x2A;
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
					for (i = 6; i < (strlen(szPrintBuf) - 4); i ++)
					{
						szPrintBuf[i] = 0x2A;
					}
				}

			}
		}
		
		/* 過卡方式 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf, "(W)");
			else
				strcat(szPrintBuf, "(C)");
		}
		else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			strcat(szPrintBuf, "(Q)");
		}
		else if (pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
		{
			strcat(szPrintBuf, "(Q)");
		}
		else
		{
			if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
				strcat(szPrintBuf,"(C)");
			else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
				strcat(szPrintBuf, "(T)");
			else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf, "(W)");
			else
			{
				if (pobTran->srBRec.uszManualBit == VS_TRUE)
				{
					/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
					/* 電文轉Manual Keyin但是簽單要印感應的W */
					if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
						strcat(szPrintBuf, "(W)");
					else
						strcat(szPrintBuf,"(M)");
				}
				else
					strcat(szPrintBuf,"(S)");
			}
		}		
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


		if(pobTran->srBRec.uszVOIDBit == VS_TRUE)
		{
			/* 橫式 */
			/* 金額 */
			/* 取消退貨是正數 */
			if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
			    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", "總計(Total) :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
				inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", "總計(Total) :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}
		else
		{
			/* 初始化 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

			/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
			if (pobTran->srBRec.inCode == _CUP_REFUND_	||
			    pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			}
			else
			{
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			}

			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Total) :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		inPRINT_Buffer_PutIn("", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("交易失敗將通知發卡行進行", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("沖銷處理。", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 欣亞電腦客製化只有一聯不用印 */
		memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
		inGetCustomIndicator(szCustomerIndicator);
		if (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_099_SINYA_, _CUSTOMER_INDICATOR_SIZE_) != 0)
		{
			if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
			    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
			{
				inPRINT_Buffer_PutIn("商店存根。", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("------------------------------------------。", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			}
			else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				inPRINT_Buffer_PutIn("持卡人存根。", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("------------------------------------------。", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			}
		}

		inPRINT_Buffer_PutIn("", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);

	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Logo_ByBuffer_ESVC
Date&Time       :2018/1/29 下午 1:57
Describe        :票證總額只印銀行LOGO
*/
int inCREDIT_PRINT_Logo_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        /* 公司內部決議比照規格，不比照520 */
        inCREDIT_PRINT_Logo_ByBuffer(pobTran, uszBuffer, srFont_Attrib, srBhandle);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Top_ESVC_ByBuffer
Date&Time       :2018/1/29 下午 2:49
Describe        :
*/
int inCREDIT_PRINT_Top_ESVC_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

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
	
	/* 主機、批號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	
	inGetHostLabel(szTemplate);
	sprintf(szPrintBuf, "主機　　 %s", szTemplate);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "批號　　 %s", "");
	inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	inGetBatchNum(szPrintBuf);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 列印日期時間 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "日期/時間 :  %.4s/%.2s/%.2s %.2s:%.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2], &pobTran->srBRec.szTime[4]);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Top_ESVC_SETTLE_ByBuffer
Date&Time       :2018/1/29 下午 2:49
Describe        :
*/
int inCREDIT_PRINT_Top_ESVC_SETTLE_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

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

	/* 交易 特店代號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	
	sprintf(szPrintBuf, "交易　　 %s", "結帳");
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* For 075 使用*/
	if (pobTran->uszEverRich_Settle_RepeintBit == VS_TRUE)
	{
		inRetVal = inPRINT_Buffer_PutIn("(重印)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	
	/* 主機、批號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	
	inGetHostLabel(szTemplate);
	sprintf(szPrintBuf, "主機　　 %s", szTemplate);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "批號　　 %s", "");
	inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	inGetBatchNum(szPrintBuf);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 列印日期時間 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "日期/時間 :  %.4s/%.2s/%.2s %.2s:%.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2], &pobTran->srBRec.szTime[4]);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmount_ByBuffer_ESVC
Date&Time       :2018/1/29 下午 3:44
Describe        :列印總金額
*/
int inCREDIT_PRINT_TotalAmount_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szPrintBuf[84 + 1], szTemplate[84 + 1];

        inPRINT_Buffer_PutIn("總額報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
        
        inPRINT_Buffer_PutIn("       筆數           金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	/* 銷售 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "購貨 　　%03lu   NT$", srAccumRec->lnDeductTotalCount);
	sprintf(szTemplate, "%lld", srAccumRec->llDeductTotalAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 退貨 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnRefundTotalCount);
	sprintf(szTemplate, "%lld", srAccumRec->llRefundTotalAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	/* 現金加值 */
	if (srAccumRec->lnADDTotalCount > 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "現金加值 %03lu   NT$", srAccumRec->lnADDTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llADDTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	/* 加值取消 */
	if (srAccumRec->lnVoidADDTotalCount > 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "加值取消 %03lu   NT$", srAccumRec->lnVoidADDTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llVoidADDTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
		
	inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 交易淨額(總購貨 - 總退貨) */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "交易淨額 %03lu   NT$", (srAccumRec->lnDeductTotalCount + srAccumRec->lnRefundTotalCount));
	sprintf(szTemplate, "%lld", (srAccumRec->llDeductTotalAmount - srAccumRec->llRefundTotalAmount));
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	/* 加值淨額 */
	if (srAccumRec->lnADDTotalCount > 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "加值淨額 %03lu   NT$", (srAccumRec->lnADDTotalCount + srAccumRec->lnVoidADDTotalCount));
		sprintf(szTemplate, "%lld", (srAccumRec->llADDTotalAmount - srAccumRec->llVoidADDTotalAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmount_ByBuffer_ESVC
Date&Time       :2018/1/29 下午 3:44
Describe        :列印總金額
*/
int inCREDIT_PRINT_TotalAmount_ByBuffer_ESVC_Settle(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char		szPrintBuf[84 + 1] = {0}, szTemplate[84 + 1] = {0};

        inPRINT_Buffer_PutIn("結帳報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
        
        inPRINT_Buffer_PutIn("       筆數           金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	/* 銷售 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "購貨 　　%03lu   NT$", srAccumRec->lnDeductTotalCount);
	sprintf(szTemplate, "%lld", srAccumRec->llDeductTotalAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 退貨 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnRefundTotalCount);
	sprintf(szTemplate, "%lld", srAccumRec->llRefundTotalAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	/* 現金加值 */
	if (srAccumRec->lnADDTotalCount > 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "現金加值 %03lu   NT$", srAccumRec->lnADDTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llADDTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	/* 加值取消 */
	if (srAccumRec->lnVoidADDTotalCount > 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "加值取消 %03lu   NT$", srAccumRec->lnVoidADDTotalCount);
		sprintf(szTemplate, "%lld", srAccumRec->llVoidADDTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 交易淨額(總購貨 - 總退貨) */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "交易淨額 %03lu   NT$", (srAccumRec->lnDeductTotalCount + srAccumRec->lnRefundTotalCount));
	sprintf(szTemplate, "%lld", (srAccumRec->llDeductTotalAmount - srAccumRec->llRefundTotalAmount));
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	/* 加值淨額 */
	if (srAccumRec->lnADDTotalCount > 0L)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szPrintBuf, "加值淨額 %03lu   NT$", (srAccumRec->lnADDTotalCount + srAccumRec->lnVoidADDTotalCount));
		sprintf(szTemplate, "%lld", (srAccumRec->llADDTotalAmount - srAccumRec->llVoidADDTotalAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmountByCard_ByBuffer_ESVC
Date&Time       :2018/1/29 下午 4:19
Describe        :依卡別列印
*/
int inCREDIT_PRINT_TotalAmountByCard_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int		i = 0;
	char		szPrintBuf[100 + 1] = {0}, szTemplate[42 + 1] = {0};
	char		szTxnType[20 + 1] = {0};
	char		szTicketNeedNewBatch[2 + 1] = {0};
	unsigned char	uszNeedPrintBit = VS_FALSE;	
	
	/* 先檢查是否任一票證有開 */
	for (i = 0; i < 4; i++)
	{
		if (inLoadTDTRec(i) != VS_SUCCESS)
		{
			break;
		}
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTicket_HostEnable(szTemplate);
                if (memcmp(szTemplate, "Y", strlen("Y")) != 0)
		{
                        continue;
		}
		else
		{
			uszNeedPrintBit = VS_TRUE;
		}
	}
	
	/* 代表有任一票證要印 */
	if (uszNeedPrintBit == VS_TRUE)
	{
		inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("         卡別小計", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	else
	{
		return (VS_SUCCESS);
	}
	
	/* 個別票證 */
        for (i = 0; i < 4; i++)
        {
                if (inLoadTDTRec(i) != VS_SUCCESS)
		{
			break;
		}
		
                memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTicket_HostEnable(szTemplate);
                if (memcmp(szTemplate, "Y", strlen("Y")) != 0)
		{
                        continue;
		}
                
                memset(szTxnType, 0x00, sizeof(szTxnType));
		inGetTicket_HostTransFunc(szTxnType);
		
                switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :
        	                inPRINT_Buffer_PutIn("卡別 一卡通", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
				
				/* 特店代號 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "特店代號%s", "");
				inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetIPASS_System_ID(szTemplate);
				memcpy(&szPrintBuf[0], szTemplate, 2);

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetIPASS_SP_ID(szTemplate);
				memcpy(&szPrintBuf[2], szTemplate, 2);

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetIPASS_Sub_Company_ID(szTemplate);
				memcpy(&szPrintBuf[4], szTemplate, 4); 
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
        	                break;
        	        case _TDT_INDEX_01_ECC_ :
        	                inPRINT_Buffer_PutIn("卡別 悠遊卡", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
          	                
				/* 特店代號 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "特店代號%s", "");
				inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetECC_New_SP_ID(szTemplate);
				strcpy(&szPrintBuf[0], szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				
				/* 二代設備編號 */
          	                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetTicket_Device2(szTemplate);
                        	sprintf(szPrintBuf, "二代設備編號　 %s", szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                        	
				/* 悠遊卡批次號碼 */
                        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTicketNeedNewBatch, 0x00, sizeof(szTicketNeedNewBatch));
				inGetTicket_NeedNewBatch(szTicketNeedNewBatch);
				if (memcmp(szTicketNeedNewBatch, "Y", strlen("Y")) == 0)
				{
					sprintf(szTemplate, "%.4s%.2s%.2s", &pobTran->srBRec.szDate[2], &pobTran->srBRec.szDate[6], _ECC_DEFAULT_BATCH_);
				}
				else
				{
					inGetTicket_Batch(szTemplate);
				}
                        	sprintf(szPrintBuf, "悠遊卡批次號碼 %s", szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
        	        case _TDT_INDEX_02_ICASH_ :
        	                inPRINT_Buffer_PutIn("卡別 愛金卡", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
				
				/* 特店代號 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "門市代碼%s", "");
				inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetICASH_Shop_ID(szTemplate);
				memcpy(&szPrintBuf[0], szTemplate, 8);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
        	                break;                      
        	        default :
        	                break;       
        	}

		/* 不合法 跳出 */
                if (i != _TDT_INDEX_00_IPASS_	&&
		    i != _TDT_INDEX_01_ECC_	&&
		    i != _TDT_INDEX_02_ICASH_)
		{
                        break;
		}
                
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :   
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", srAccumRec->lnIPASS_DeductTotalCount);                   
        	                sprintf(szTemplate, "%lld", srAccumRec->llIPASS_DeductTotalAmount);
        	                break;
        	        case _TDT_INDEX_01_ECC_ :   
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", srAccumRec->lnEASYCARD_DeductTotalCount);                   
        	                sprintf(szTemplate, "%lld", srAccumRec->llEASYCARD_DeductTotalAmount);
        	                break;
        	        case _TDT_INDEX_02_ICASH_ :   
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", srAccumRec->lnICASH_DeductTotalCount);                   
        	                sprintf(szTemplate, "%lld", srAccumRec->llICASH_DeductTotalAmount);
        	                break;
        	        default :
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", 0l);                   
        	                sprintf(szTemplate, "%lld", 0ll);
        	                break;        
        	}
        	
        	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
        	strcat(szPrintBuf, szTemplate);
        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		
        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", srAccumRec->lnIPASS_RefundTotalCount);                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_RefundTotalAmount));
        	                break;
        	        case _TDT_INDEX_01_ECC_ :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", srAccumRec->lnEASYCARD_RefundTotalCount);                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_RefundTotalAmount));
        	                break;
        	        case _TDT_INDEX_02_ICASH_ :   
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", srAccumRec->lnICASH_RefundTotalCount);                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llICASH_RefundTotalAmount));
        	                break;
        	        default :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", 0l);                   
        	                sprintf(szTemplate, "%lld", 0ll);
        	                break;        
        	}
		
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
        	strcat(szPrintBuf, szTemplate);
        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	
		/* 加值功能有開且有交易筆數 */
        	if (szTxnType[4] == 0x59 && 
		   ((i == _TDT_INDEX_00_IPASS_ && srAccumRec->lnIPASS_ADDTotalCount > 0)	||
		    (i == _TDT_INDEX_01_ECC_ && srAccumRec->lnEASYCARD_ADDTotalCount > 0)	||
		    (i == _TDT_INDEX_02_ICASH_ && srAccumRec->lnICASH_ADDTotalCount > 0)))
        	{
        	        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	
                	switch(i)
                        {
                                case _TDT_INDEX_00_IPASS_ :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", srAccumRec->lnIPASS_ADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_ADDTotalAmount));
                	                break;
                	        case _TDT_INDEX_01_ECC_ :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", srAccumRec->lnEASYCARD_ADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_ADDTotalAmount));
                	                break;
                	        case _TDT_INDEX_02_ICASH_ :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", srAccumRec->lnICASH_ADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llICASH_ADDTotalAmount));
                	                break;
                	        default :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", 0l);                   
                	                sprintf(szTemplate, "%lld", 0ll);
                	                break;        
                	}
                	
                	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	}
        	
		/* 加值取消功能有開且有交易筆數 */
        	if (szTxnType[5] == 0x59	&& 
		   ((i == _TDT_INDEX_00_IPASS_ && srAccumRec->lnIPASS_VoidADDTotalCount > 0)	||
		    (i == _TDT_INDEX_01_ECC_ && srAccumRec->lnEASYCARD_VoidADDTotalCount > 0)   ||
		    (i == _TDT_INDEX_02_ICASH_ && srAccumRec->lnICASH_VoidADDTotalCount > 0)))
        	{
                	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                	switch(i)
                        {
                                case _TDT_INDEX_00_IPASS_ :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", srAccumRec->lnIPASS_VoidADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_VoidADDTotalAmount));
                	                break;
                	        case _TDT_INDEX_01_ECC_ :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", srAccumRec->lnEASYCARD_VoidADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_VoidADDTotalAmount));
                	                break;
                	        case _TDT_INDEX_02_ICASH_ :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", srAccumRec->lnICASH_VoidADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llICASH_VoidADDTotalAmount));
                	                break;
                	        default :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", 0l);                   
                	                sprintf(szTemplate, "%lld", 0ll);
                	                break;        
                	}
                	
                	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                }
                	
        	inPRINT_Buffer_PutIn("------------------------------------------", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		
        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", (srAccumRec->lnIPASS_DeductTotalCount + srAccumRec->lnIPASS_RefundTotalCount));                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_DeductTotalAmount - srAccumRec->llIPASS_RefundTotalAmount));
        	                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
                        	strcat(szPrintBuf, szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
        	        case _TDT_INDEX_01_ECC_ :
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", (srAccumRec->lnEASYCARD_DeductTotalCount + srAccumRec->lnEASYCARD_RefundTotalCount));                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_DeductTotalAmount - srAccumRec->llEASYCARD_RefundTotalAmount));
        	                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
                        	strcat(szPrintBuf, szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
        	        case _TDT_INDEX_02_ICASH_ :
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", (srAccumRec->lnICASH_DeductTotalCount + srAccumRec->lnICASH_RefundTotalCount));                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llICASH_DeductTotalAmount - srAccumRec->llICASH_RefundTotalAmount));
        	                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
                        	strcat(szPrintBuf, szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
        	        default :
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", 0l);                   
        	                sprintf(szTemplate, "%lld", 0ll);
        	                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
                        	strcat(szPrintBuf, szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;        
        	}
        	
		if ((i == _TDT_INDEX_00_IPASS_ && srAccumRec->lnIPASS_ADDTotalCount > 0)	||
		    (i == _TDT_INDEX_01_ECC_ && srAccumRec->lnEASYCARD_ADDTotalCount > 0)	||
		    (i == _TDT_INDEX_02_ICASH_ && srAccumRec->lnICASH_ADDTotalCount > 0))
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			switch(i)
			{
				case _TDT_INDEX_00_IPASS_ :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", (srAccumRec->lnIPASS_ADDTotalCount + srAccumRec->lnIPASS_VoidADDTotalCount));                   
					sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_ADDTotalAmount - srAccumRec->llIPASS_VoidADDTotalAmount));
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;
				case _TDT_INDEX_01_ECC_ :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", (srAccumRec->lnEASYCARD_ADDTotalCount + srAccumRec->lnEASYCARD_VoidADDTotalCount));                   
					sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_ADDTotalAmount - srAccumRec->llEASYCARD_VoidADDTotalAmount));
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;
				case _TDT_INDEX_02_ICASH_ :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", (srAccumRec->lnICASH_ADDTotalCount + srAccumRec->lnICASH_VoidADDTotalCount));                   
					sprintf(szTemplate, "%lld", (srAccumRec->llICASH_ADDTotalAmount - srAccumRec->llICASH_VoidADDTotalAmount));
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;
				default :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", 0l);                   
					sprintf(szTemplate, "%lld", 0ll);
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;        
			}
		}
        	
        	inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        }
        
        inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmountByCard_ByBuffer_ESVC_Settle
Date&Time       :2018/1/30 下午 2:32
Describe        :依卡別列印
*/
int inCREDIT_PRINT_TotalAmountByCard_ByBuffer_ESVC_Settle(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int		i = 0;
	char		szPrintBuf[100 + 1], szTemplate[42 + 1];
	char		szTxnType[20 + 1];
	char		szTicketNeedNewBatch[2 + 1] = {0};
	char		szSpecialShopID[50 + 1] = {0};
	unsigned char	uszNeedPrintBit = VS_FALSE;
	
	/* 先檢查是否任一票證有開 */
	for (i = 0; i < 4; i++)
	{
		if (inLoadTDTRec(i) != VS_SUCCESS)
		{
			break;
		}
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTicket_HostEnable(szTemplate);
                if (memcmp(szTemplate, "Y", strlen("Y")) != 0)
		{
                        continue;
		}
		else
		{
			uszNeedPrintBit = VS_TRUE;
		}
	}
	
	/* 代表有任一票證要印 */
	if (uszNeedPrintBit == VS_TRUE)
	{
		inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("         卡別小計", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	else
	{
		return (VS_SUCCESS);
	}
	
        for (i = 0; i < 4; i++)
        {
                if (inLoadTDTRec(i) != VS_SUCCESS)
		{
			break;
		}
		
                memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTicket_HostEnable(szTemplate);
                if (memcmp(szTemplate, "Y", strlen("Y")) != 0)
		{
                        continue;
		}
                
                memset(szTxnType, 0x00, sizeof(szTxnType));
		inGetTicket_HostTransFunc(szTxnType);
		        
                switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :
        	                inPRINT_Buffer_PutIn("卡別 一卡通", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
				
				memset(szSpecialShopID, 0x00, sizeof(szSpecialShopID));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetIPASS_System_ID(szTemplate);
				memcpy(&szSpecialShopID[0], szTemplate, 2);

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetIPASS_SP_ID(szTemplate);
				memcpy(&szSpecialShopID[2], szTemplate, 2);

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetIPASS_Sub_Company_ID(szTemplate);
				memcpy(&szSpecialShopID[4], szTemplate, 4);
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "特店代號%s", szSpecialShopID);
				inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_DEFINE_X_01_);
				
        	                break;
        	        case _TDT_INDEX_01_ECC_ :
        	                inPRINT_Buffer_PutIn("卡別 悠遊卡", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
          	                
				memset(szSpecialShopID, 0x00, sizeof(szSpecialShopID));
				inGetECC_New_SP_ID(szSpecialShopID);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "特店代號%s", szSpecialShopID);
				inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_DEFINE_X_01_);
				
				/* 二代設備編號 */
          	                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetTicket_Device2(szTemplate);
                        	sprintf(szPrintBuf, "二代設備編號　 %s", szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                        	
				/* 悠遊卡批次號碼 */
                        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTicketNeedNewBatch, 0x00, sizeof(szTicketNeedNewBatch));
				inGetTicket_NeedNewBatch(szTicketNeedNewBatch);
				if (memcmp(szTicketNeedNewBatch, "Y", strlen("Y")) == 0)
				{
					sprintf(szTemplate, "%.4s%.2s%.2s", &pobTran->srBRec.szDate[2], &pobTran->srBRec.szDate[6], _ECC_DEFAULT_BATCH_);
				}
				else
				{
					inGetTicket_Batch(szTemplate);
				}
                        	sprintf(szPrintBuf, "悠遊卡批次號碼 %s", szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
				/* 列印交易時間 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szPrintBuf, "交易時間　　　 %.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        	                break;
        	        case _TDT_INDEX_02_ICASH_ :
        	                inPRINT_Buffer_PutIn("卡別 愛金卡", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_ , _PRINT_LEFT_);
				
				/* 特店代號 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "門市代碼%s", "");
				inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetICASH_Shop_ID(szTemplate);
				memcpy(&szPrintBuf[0], szTemplate, 8);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
        	                break; 
        	        default :
        	                break;       
        	}

		/* 不合法 跳出 */
                if (i != _TDT_INDEX_00_IPASS_	&&
		    i != _TDT_INDEX_01_ECC_	&&
		    i != _TDT_INDEX_02_ICASH_)
		{
                        break;
		}
                
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :   
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", srAccumRec->lnIPASS_DeductTotalCount);                   
        	                sprintf(szTemplate, "%lld", srAccumRec->llIPASS_DeductTotalAmount);
        	                break;
        	        case _TDT_INDEX_01_ECC_ :   
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", srAccumRec->lnEASYCARD_DeductTotalCount);                   
        	                sprintf(szTemplate, "%lld", srAccumRec->llEASYCARD_DeductTotalAmount);
        	                break;
        	        case _TDT_INDEX_02_ICASH_ :   
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", srAccumRec->lnICASH_DeductTotalCount);                   
        	                sprintf(szTemplate, "%lld", srAccumRec->llICASH_DeductTotalAmount);
        	                break;
        	        default :
        	                sprintf(szPrintBuf, "購貨　　　%03lu   NT$", 0l);                   
        	                sprintf(szTemplate, "%lld", 0ll);
        	                break;        
        	}
        	
        	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
        	strcat(szPrintBuf, szTemplate);
        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	
        	
		
        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", srAccumRec->lnIPASS_RefundTotalCount);                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_RefundTotalAmount));
        	                break;
        	        case _TDT_INDEX_01_ECC_ :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", srAccumRec->lnEASYCARD_RefundTotalCount);                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_RefundTotalAmount));
        	                break;
                        case _TDT_INDEX_02_ICASH_ :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", srAccumRec->lnICASH_RefundTotalCount);                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llICASH_RefundTotalAmount));
        	                break;
        	        default :
        	                sprintf(szPrintBuf, "退貨　　　%03lu   NT$", 0l);                   
        	                sprintf(szTemplate, "%lld", 0ll);
        	                break;        
        	}
		
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
        	strcat(szPrintBuf, szTemplate);
        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	
		/* 加值功能有開且有交易筆數 */
        	if (szTxnType[4] == 0x59  && 
		   ((i == _TDT_INDEX_00_IPASS_ && srAccumRec->lnIPASS_ADDTotalCount > 0)	||
		    (i == _TDT_INDEX_01_ECC_ && srAccumRec->lnEASYCARD_ADDTotalCount > 0)	||
		    (i == _TDT_INDEX_02_ICASH_ && srAccumRec->lnICASH_ADDTotalCount > 0)))
        	{
        	        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	
                	switch(i)
                        {
                                case _TDT_INDEX_00_IPASS_ :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", srAccumRec->lnIPASS_ADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_ADDTotalAmount));
                	                break;
                	        case _TDT_INDEX_01_ECC_ :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", srAccumRec->lnEASYCARD_ADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_ADDTotalAmount));
                	                break;
                                case _TDT_INDEX_02_ICASH_ :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", srAccumRec->lnICASH_ADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llICASH_ADDTotalAmount));
                	                break;					
                	        default :
                	                sprintf(szPrintBuf, "現金加值　%03lu   NT$", 0l);                   
                	                sprintf(szTemplate, "%lld", 0ll);
                	                break;        
                	}
                	
                	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	}
        	
		/* 加值取消功能有開且有交易筆數 */
        	if (szTxnType[5] == 0x59  && 
		   ((i == _TDT_INDEX_00_IPASS_ && srAccumRec->lnIPASS_VoidADDTotalCount > 0)	||
		    (i == _TDT_INDEX_01_ECC_ && srAccumRec->lnEASYCARD_VoidADDTotalCount > 0)	||
		    (i == _TDT_INDEX_02_ICASH_ && srAccumRec->lnICASH_VoidADDTotalCount > 0)))
        	{
                	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                	switch(i)
                        {
                                case _TDT_INDEX_00_IPASS_ :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", srAccumRec->lnIPASS_VoidADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_VoidADDTotalAmount));
                	                break;
                	        case _TDT_INDEX_01_ECC_ :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", srAccumRec->lnEASYCARD_VoidADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_VoidADDTotalAmount));
                	                break;
                                case _TDT_INDEX_02_ICASH_ :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", srAccumRec->lnICASH_VoidADDTotalCount);                   
                	                sprintf(szTemplate, "%lld", (srAccumRec->llICASH_VoidADDTotalAmount));
                	                break;
                	        default :
                	                sprintf(szPrintBuf, "加值取消　%03lu   NT$", 0l);                   
                	                sprintf(szTemplate, "%lld", 0ll);
                	                break;        
                	}
                	
                	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                }
                	
        	inPRINT_Buffer_PutIn("------------------------------------------", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		
        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	switch(i)
                {
                        case _TDT_INDEX_00_IPASS_ :
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", (srAccumRec->lnIPASS_DeductTotalCount + srAccumRec->lnIPASS_RefundTotalCount));                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_DeductTotalAmount - srAccumRec->llIPASS_RefundTotalAmount));
        	                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
                        	strcat(szPrintBuf, szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
        	        case _TDT_INDEX_01_ECC_ :
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", (srAccumRec->lnEASYCARD_DeductTotalCount + srAccumRec->lnEASYCARD_RefundTotalCount));                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_DeductTotalAmount - srAccumRec->llEASYCARD_RefundTotalAmount));
        	                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
                        	strcat(szPrintBuf, szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
                        case _TDT_INDEX_02_ICASH_ :
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", (srAccumRec->lnICASH_DeductTotalCount + srAccumRec->lnICASH_RefundTotalCount));                   
        	                sprintf(szTemplate, "%lld", (srAccumRec->llICASH_DeductTotalAmount - srAccumRec->llICASH_RefundTotalAmount));
        	                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
                        	strcat(szPrintBuf, szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;
        	        default :
        	                sprintf(szPrintBuf, "交易淨額　%03lu   NT$", 0l);                   
        	                sprintf(szTemplate, "%lld", 0ll);
        	                inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
                        	strcat(szPrintBuf, szTemplate);
                        	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        	                break;        
        	}
        	
		if ((i == _TDT_INDEX_00_IPASS_ && srAccumRec->lnIPASS_ADDTotalCount > 0)	||
		    (i == _TDT_INDEX_01_ECC_ && srAccumRec->lnEASYCARD_ADDTotalCount > 0)	||
		    (i == _TDT_INDEX_02_ICASH_ && srAccumRec->lnICASH_ADDTotalCount > 0))
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			switch(i)
			{
				case _TDT_INDEX_00_IPASS_ :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", (srAccumRec->lnIPASS_ADDTotalCount + srAccumRec->lnIPASS_VoidADDTotalCount));                   
					sprintf(szTemplate, "%lld", (srAccumRec->llIPASS_ADDTotalAmount - srAccumRec->llIPASS_VoidADDTotalAmount));
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;
				case _TDT_INDEX_01_ECC_ :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", (srAccumRec->lnEASYCARD_ADDTotalCount + srAccumRec->lnEASYCARD_VoidADDTotalCount));                   
					sprintf(szTemplate, "%lld", (srAccumRec->llEASYCARD_ADDTotalAmount - srAccumRec->llEASYCARD_VoidADDTotalAmount));
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;
				case _TDT_INDEX_02_ICASH_ :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", (srAccumRec->lnICASH_ADDTotalCount + srAccumRec->lnICASH_VoidADDTotalCount));                   
					sprintf(szTemplate, "%lld", (srAccumRec->llICASH_ADDTotalAmount - srAccumRec->llICASH_VoidADDTotalAmount));
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;
				default :
					sprintf(szPrintBuf, "加值淨額　%03lu   NT$", 0l);                   
					sprintf(szTemplate, "%lld", 0ll);
					inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
					strcat(szPrintBuf, szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
					break;        
			}
		}
		
        	inPRINT_Buffer_PutIn(" ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
        }
        
        inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_End_ByBuffer_ESVC
Date&Time       :2018/1/29 下午 4:13
Describe        :列印結尾
*/
int inCREDIT_PRINT_End_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC* srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
	char	szPrintBuf[100 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_042_BDAU1_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_)	)
	{

	}
	else
	{
		if (inNCCC_Loyalty_ASM_Flag() == VS_SUCCESS)
		{
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("優惠平台", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "　悠遊卡取得優惠　　　　　　  ");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%03lu 筆", srAccumRec->lnEASYCARD_RewardTotalCount);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "　一卡通取得優惠　　　　　　  ");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%03lu 筆", srAccumRec->lnIPASS_RewardTotalCount);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "　愛金卡取得優惠　　　　　　  ");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%03lu 筆", srAccumRec->lnICASH_RewardTotalCount);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);

			inPRINT_Buffer_PutIn("------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "　電票優惠取得小計　　　　　  ");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%03lu 筆", srAccumRec->lnEASYCARD_RewardTotalCount + srAccumRec->lnIPASS_RewardTotalCount + srAccumRec->lnICASH_RewardTotalCount + 0);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);

			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("==========================================", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
	}
	
        inPRINT_Buffer_PutIn("*** 列印完成 ***", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

        for (i = 0; i < 8; i++)
	{
		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_DetailReport_ByBuffer_ESVC
Date&Time       :2018/1/31 上午 10:16
Describe        :列印明細帳單
*/
int inCREDIT_PRINT_DetailReport_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = 0, inPrintIndex = 0, inRecordCnt = 0;
	char			szDebugMsg[100 + 1];
	char			szCustomerIndicator[3 + 1] = {0};
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
        TICKET_ACCUM_TOTAL_REC	srAccumRec;
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_);/* 處理中... */
                
                /* 客製化098，明細簽單不同 */
                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))
                {
                        inPrintIndex = _DETAIL_REPORT_INDEX_098_ESVC_;
                }
		/* (需求單-109327)-vx520客製化需求 by Russell 2022/4/18 上午 10:44
		    昇恆昌客製化明細加粗 */
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_046_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
                         !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inPrintIndex = _DETAIL_REPORT_INDEX_046_ESVC_;
		}
                else
                {
                        inPrintIndex = _DETAIL_REPORT_INDEX_ESVC_;
                }

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "PrintIndex : %d", inPrintIndex);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 檢查是否有帳 */
		if (srDetailReport_ByBuffer_ESVC[inPrintIndex].inReportCheck != NULL)
		{
			if ((inRecordCnt = srDetailReport_ByBuffer_ESVC[inPrintIndex].inReportCheck(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) == VS_ERROR)
				return (VS_ERROR); /* 表示檔案開啟失敗 */
		}

		/* 開啟交易總的檔案 */
		memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		inRetVal = inACCUM_GetRecord_ESVC(pobTran, &srAccumRec);

		if (inRetVal == VS_NO_RECORD)
		{
			memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		}
		else if (inRetVal == VS_ERROR)
		{
			inLogPrintf(AT, "Get record 失敗.");

			return (VS_ERROR);
		}

		while (1)
		{
			inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
			srBhandle1.uszDetailPrint = VS_TRUE;
			
			/* 列印LOGO */
			if (srDetailReport_ByBuffer_ESVC[inPrintIndex].inReportLogo != NULL)
				if ((inRetVal = srDetailReport_ByBuffer_ESVC[inPrintIndex].inReportLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 列印TID MID */
			if (srDetailReport_ByBuffer_ESVC[inPrintIndex].inReportTop != NULL)
				if ((inRetVal = srDetailReport_ByBuffer_ESVC[inPrintIndex].inReportTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 全部金額總計 */
			if (srDetailReport_ByBuffer_ESVC[inPrintIndex].inTotalAmount != NULL)
				if ((inRetVal = srDetailReport_ByBuffer_ESVC[inPrintIndex].inTotalAmount(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			/* 明細規格 */ 
			if (srDetailReport_ByBuffer_ESVC[inPrintIndex].inMiddle != NULL)
				if ((inRetVal = srDetailReport_ByBuffer_ESVC[inPrintIndex].inMiddle(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 明細資料 */
			if (srDetailReport_ByBuffer_ESVC[inPrintIndex].inBottom != NULL)
			{
				inRetVal = srDetailReport_ByBuffer_ESVC[inPrintIndex].inBottom(pobTran, inRecordCnt, uszBuffer1, &srFont_Attrib1, &srBhandle1);
				if (inRetVal != VS_SUCCESS	&&
				    inRetVal != VS_NO_RECORD)
				{
				       return (inRetVal);
				}
			}

			/* 結束 */
			if (srDetailReport_ByBuffer_ESVC[inPrintIndex].inReportEnd != NULL)
				if ((inRetVal = srDetailReport_ByBuffer_ESVC[inPrintIndex].inReportEnd(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			
			inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

			if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);

			break;
		}

		return (VS_SUCCESS);
	}
}

/*
Function        :inCREDIT_PRINT_Check_ByBuffer_ESVC
Date&Time       :2018/1/31 上午 11:50
Describe        :
*/
int inCREDIT_PRINT_Check_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        int     inRecordCnt;

        inRecordCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite_ESVC(pobTran);
        /* 回傳VS_ERROR(回傳 -1 )會跳出，交易筆數小於0( VS_NoRecord 會回傳 -98 )會印空白簽單 */
        /* 其餘則回傳交易筆數*/

        return (inRecordCnt);
}

/*
Function        :inCREDIT_PRINT_DetailReportMiddle_ByBuffer_ESVC
Date&Time       :2018/1/31 上午 11:27
Describe        :
*/
int inCREDIT_PRINT_DetailReportMiddle_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        inPRINT_Buffer_PutIn("明細報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

	inPRINT_Buffer_PutIn("調閱編號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn_Specific_X_Position("交易金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
	
        inPRINT_Buffer_PutIn("交易類別", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn_Specific_X_Position("卡別", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
	
        inPRINT_Buffer_PutIn("卡號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	inPRINT_Buffer_PutIn("交易日期", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn_Specific_X_Position("交易時間", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
	
	inPRINT_Buffer_PutIn("RF序號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("RRN", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("優惠類別", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
        inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_DetailReportBottom_ByBuffer_ESVC
Date&Time       :2018/1/31 上午 11:27
Describe        :
*/
int inCREDIT_PRINT_DetailReportBottom_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inReadCnt = 0;
	int	inRetVal = VS_SUCCESS;
	int	inLen = 0;
        char	szPrintBuf[62 + 1], szTemplate1[62 + 1];
        char    szCustomerIndicator[3 + 1] = {0};
	int	inFontSize_Option = 0;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inCREDIT_PRINT_DetailReportBottom_ByBuffer_ESVC()_START");
	}
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_) ||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inFontSize_Option = _PRT_CUS_123_DETAIL_;
	}
	else
	{
		inFontSize_Option = _PRT_NORMAL2_;
	}
	
	/* 開始讀取 */
	inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_START(pobTran);
	guszEnormousNoNeedResetBit = VS_TRUE;

        for (inReadCnt = 0; inReadCnt < inRecordCnt; inReadCnt ++)
        {
                /*. 開始讀取每一筆交易記錄 .*/
                if (inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_Read(pobTran, inReadCnt) != VS_SUCCESS)
                {
                        inRetVal = VS_ERROR;
                        break;
                }
		
		/* 調閱編號 & Amount */
		/* Invoice Number */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "INV:%06ld", pobTran->srTRec.lnInvNum);
		strcat(szPrintBuf, szTemplate1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
		{
			break;
		}

		/* Print Amount */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));

		switch (pobTran->srTRec.inCode)
		{
			case _TICKET_IPASS_AUTO_TOP_UP_:
				sprintf(szTemplate1, "%ld", pobTran->srTRec.lnTotalTopUpAmount);
				break;
			default :
				sprintf(szTemplate1, "%ld", pobTran->srTRec.lnTxnAmount);
				break;
		} /* End switch () */
		
		inFunc_Amount_Comma(szTemplate1, "NT$" , 0x00, _SIGNED_NONE_, 16, _PADDING_RIGHT_);
		strcat(szPrintBuf, szTemplate1);
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		if (inRetVal != VS_SUCCESS)
		{
			break;
		}

		/* 交易類別 & 卡別 */
		/* Trans Type */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inFunc_GetTransType_ESVC(pobTran, szTemplate1);
		sprintf(szPrintBuf, "%s", szTemplate1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
		{
			break;
		}

		/* 卡別 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
		{
		        strcat(szPrintBuf, "一卡通");
		}
		else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
		{
		        strcat(szPrintBuf, "悠遊卡");
		}
		else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
		{
		        strcat(szPrintBuf, "愛金卡");
		}	
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		if (inRetVal != VS_SUCCESS)
		{
			break;
		}
		
		
		/* 卡號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
		{
			inLen = strlen(pobTran->srTRec.szUID);
			memcpy(szPrintBuf, pobTran->srTRec.szUID, inLen);
			szPrintBuf[inLen - 2] = 0x2A;
        		szPrintBuf[inLen - 1] = 0x2A;
		}
		else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
		{
                        inLen = strlen(pobTran->srTRec.szUID);
                        memcpy(szPrintBuf, pobTran->srTRec.szUID, inLen);
                        
                        /* 客製化098，悠遊卡因為使用T0200，加密遮掩為倒數5、6、7 */
                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))
                        {
                                szPrintBuf[inLen - 5] = 0x2A;
                                szPrintBuf[inLen - 6] = 0x2A;
                                szPrintBuf[inLen - 7] = 0x2A;
                        }
                        else
                        {		
                                if (atoi(pobTran->srTRec.srECCRec.szPurseVersionNumber) == 0)
                                {
                                        szPrintBuf[inLen - 5] = 0x2A;
                                        szPrintBuf[inLen - 6] = 0x2A;
                                        szPrintBuf[inLen - 7] = 0x2A;
                                }
                                else
                                {
                                        /* 全部16 第6-11隱碼 */
                                        szPrintBuf[inLen - 6] = 0x2A;
                                        szPrintBuf[inLen - 7] = 0x2A;
                                        szPrintBuf[inLen - 8] = 0x2A;
                                        szPrintBuf[inLen - 9] = 0x2A;
                                        szPrintBuf[inLen - 10] = 0x2A;
                                        szPrintBuf[inLen - 11] = 0x2A;
                                }
                        }    
		}
		else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
		{
			inLen = strlen(pobTran->srTRec.szUID);
			memcpy(szPrintBuf, pobTran->srTRec.szUID, inLen);
			/* 愛金卡：遮掩9-12 碼，遮掩字元為"*" */
			szPrintBuf[8] = 0x2A;
			szPrintBuf[9] = 0x2A;
			szPrintBuf[10] = 0x2A;
			szPrintBuf[11] = 0x2A;				
		}
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
		{
			break;
		}

		/* Trans Date Time */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "DATE: 20%.4s/%.2s/%.2s", &pobTran->srTRec.szDate[0], &pobTran->srTRec.szDate[2], &pobTran->srTRec.szDate[4]);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
		{
			break;
		}

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "TIME: %.2s:%.2s",  &pobTran->srTRec.szTime[0], &pobTran->srTRec.szTime[2]);
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		if (inRetVal != VS_SUCCESS)
		{
			break;
		}

		/* RF序號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, "RF no: ");
		strcat(szPrintBuf, pobTran->srTRec.szTicketRefundCode);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
		{
			break;
		}
		
		/* 悠遊卡要多印RRN */
		if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
		{
		        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		        sprintf(szPrintBuf, "RRN : %s", pobTran->srTRec.srECCRec.szRRN);
        		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		}
		
		if ((pobTran->srTRec.uszRewardL1Bit == VS_TRUE	||
		     pobTran->srTRec.uszRewardL2Bit == VS_TRUE	||
		     pobTran->srTRec.uszRewardL5Bit == VS_TRUE)	&&
		     pobTran->srTRec.uszRewardSuspendBit != VS_TRUE)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		        sprintf(szPrintBuf, "優惠");
        		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		}
				
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        } /* End for () .... */
	
	/* 結束讀取 */
	inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_END(pobTran);

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inCREDIT_PRINT_DetailReportBottom_ByBuffer_ESVC()_END");
	}
	
        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_ESC_Reinforce_Count_ByBuffer
Date&Time       :2018/5/31 下午 5:47
Describe        :【需求單 - 105259】總額明細報表及總額明細查詢補強機制
*/
int inCREDIT_PRINT_ESC_Reinforce_Count_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal = VS_SUCCESS;
	int     inSaleUploadCnt = 0, inRefundUploadCnt = 0, inSalePaperCnt = 0, inRefundPaperCnt = 0; 
	long    lnSaleUploadAmt = 0, lnRefundUploadAmt = 0, lnSalePaperAmt = 0, lnRefundPaperAmt = 0;
	char    szPrintBuf[84 + 1] = {0}, szTemplate[84 + 1] = {0};
	char	szFuncEnable[2 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_ESC_Reinforce_Count() START !");
	}

	/* ESC沒開直接跳走 */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetESCMode(szFuncEnable);
	if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0)
	{
		return (VS_SUCCESS);
	}
	
	/* 預先列印沒有值 */
	if (pobTran->inTransactionCode == _SETTLE_	&&
	    pobTran->uszPrePrintBit != VS_TRUE)
	{
		inSaleUploadCnt = pobTran->inESC_Sale_UploadCnt;
		inRefundUploadCnt = pobTran->inESC_Refund_UploadCnt;
		inSalePaperCnt = pobTran->inESC_Sale_PaperCnt;
		inRefundPaperCnt = pobTran->inESC_Refund_PaperCnt;
		lnSaleUploadAmt = pobTran->lnESC_Sale_UploadAmt;
		lnRefundUploadAmt = pobTran->lnESC_Refund_UploadAmt;
		lnSalePaperAmt = pobTran->lnESC_Sale_PaperAmt;
		lnRefundPaperAmt = pobTran->lnESC_Refund_PaperAmt;
	}
	else
	{
		/* Sale已上傳 */
		if (inBATCH_Get_ESC_Upload_Count_Flow(pobTran, _TN_BATCH_TABLE_, _ESC_REINFORCE_TXNCODE_SALE_, VS_FALSE, &inSaleUploadCnt, &lnSaleUploadAmt) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "ESC COUNT SQLite read fail.");
			}

		}

		/* Refund已上傳 */
		if (inBATCH_Get_ESC_Upload_Count_Flow(pobTran, _TN_BATCH_TABLE_, _ESC_REINFORCE_TXNCODE_REFUND_, VS_FALSE, &inRefundUploadCnt, &lnRefundUploadAmt) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "ESC COUNT SQLite read fail.");
			}

		}

		/* Sale出紙本 */
		if (inBATCH_Get_ESC_Upload_Count_Flow(pobTran, _TN_BATCH_TABLE_, _ESC_REINFORCE_TXNCODE_SALE_, VS_TRUE, &inSalePaperCnt, &lnSalePaperAmt) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "ESC COUNT SQLite read fail.");
			}

		}

		/* Refund出紙本 */
		if (inBATCH_Get_ESC_Upload_Count_Flow(pobTran, _TN_BATCH_TABLE_, _ESC_REINFORCE_TXNCODE_REFUND_, VS_TRUE, &inRefundPaperCnt, &lnRefundPaperAmt) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "ESC COUNT SQLite read fail.");
			}

		}
	}

	inPRINT_Buffer_PutIn("==========================================", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("銷售   筆數(CNT)   金額(AMOUNT)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 銷售 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	sprintf(szPrintBuf, "已上傳   　%03d   NT$", inSaleUploadCnt);
	sprintf(szTemplate, "%ld", lnSaleUploadAmt);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 12, _PADDING_LEFT_);

	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	sprintf(szPrintBuf, "未上傳   　%03d   NT$", inSalePaperCnt);
	sprintf(szTemplate, "%ld", lnSalePaperAmt);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 12, _PADDING_LEFT_);

	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	inPRINT_Buffer_PutIn("退貨   筆數(CNT)   金額(AMOUNT)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 退貨 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	sprintf(szPrintBuf, "已上傳   　%03d   NT$", inRefundUploadCnt);
	sprintf(szTemplate, "%ld", lnRefundUploadAmt);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 12, _PADDING_LEFT_);

	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	sprintf(szPrintBuf, "未上傳   　%03d   NT$", inRefundPaperCnt);
	sprintf(szTemplate, "%ld", lnRefundPaperAmt);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 12, _PADDING_LEFT_);

	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("==========================================", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	return (inRetVal);
}

/*
Function        :inCREDIT_PRINTBYBUFFER_Receipt_Test
Date&Time       :2016/9/2 下午 4:23
Describe        :
*/
int inCREDIT_PRINTBYBUFFER_Receipt_Test(void)
{
	int			i;
	char			szPrintBuf[84 + 1], szTemplate[42 + 1], szTemplate1[42 + 1], szTransType[42 + 1], szHostLabel[8 + 1];
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	FONT_ATTRIB		srFont_Attrib;
	BufferHandle		srBhandle;
	
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(uszBuffer, 0x00, sizeof(uszBuffer));

		inPRINT_Buffer_PutIn("RIGHT", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutGraphic((unsigned char*)_BANK_LOGO_, uszBuffer, &srBhandle, 40, _APPEND_);
		inPRINT_Buffer_PutGraphic((unsigned char*)_MERCHANT_LOGO_, uszBuffer, &srBhandle, 89, _APPEND_);
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 17, _PADDING_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, "0108000237", 15);
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_LEFT_);
		sprintf(szPrintBuf, "商店代號 %s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, "13994020", 8);
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 13, _PADDING_LEFT_);
		sprintf(szPrintBuf, "端末機代號 %s", szTemplate);

		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 城市別(City) */
		inPRINT_Buffer_PutIn("城市別(City)", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, "KINMEN-LIENCHIANG   ", 20);
		sprintf(szPrintBuf, "%s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 卡別 授權碼 */
		inPRINT_Buffer_PutIn("卡別(Card Type)  授權碼(Auth Code)", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		inFunc_PAD_ASCII(szPrintBuf, "AMEX", ' ', 13, _PADDING_RIGHT_);
		memcpy(&szTemplate[0], "123456", _AUTH_CODE_SIZE_);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 卡號 */
		inPRINT_Buffer_PutIn("卡號(Card No.)", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memcpy(szPrintBuf, "376348129192026    ", 19);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 主機別 & 交易別 */
		inPRINT_Buffer_PutIn("主機別/交易類別(Host/Trans. Type)", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szTransType, 0x00, sizeof(szTransType));

		memcpy(&szTemplate, "00 一般交易 SALE", 16);

		memset(szHostLabel, 0x00, sizeof(szHostLabel));
		memcpy(&szHostLabel, "NCCC    ", 8);
		sprintf(szPrintBuf, "%s %s", szHostLabel , szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 批次號碼 */
		inPRINT_Buffer_PutIn("批次號碼(Batch No.)", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memcpy(szPrintBuf, "001", 3);
		inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, '0', 3, _PADDING_LEFT_);

		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 日期時間 */
		inPRINT_Buffer_PutIn("日期/時間(Date/Time)", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&szPrintBuf[0], "2016/01/30", 10);
		memcpy(&szPrintBuf[13], "13:38", 5);

		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 序號 調閱編號 */
		inPRINT_Buffer_PutIn("序號(Ref. No.)　 調閱編號(Inv.No)", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

		memcpy(szPrintBuf,"99402001001  000001", 19);

		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 櫃號 */
		inPRINT_Buffer_PutIn("櫃號(Store ID)", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memcpy(szPrintBuf,"                   ", 19);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 初始化 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memcpy(szTemplate, "100", 3);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 8, VS_TRUE);

		/* 把前面的字串和數字結合起來 */
		sprintf(szPrintBuf, "金額(Amount):NT$%s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 小費 */
		inPRINT_Buffer_PutIn("小費(Tips)  :__________________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 總計 */
		inPRINT_Buffer_PutIn("總計(Total) :__________________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, &srBhandle, 50, _APPEND_);

		inPRINT_Buffer_PutIn("X:________________________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 持卡人姓名 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%s", "SAM");

		inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutIn("持卡人簽名(特店存根聯)", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

		inPRINT_Buffer_PutIn("            I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutIn("        ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);

		return (VS_SUCCESS);
	}
}

/*
Function        :inCREDIT_PRINT_Test
Date&Time       :2018/3/12 上午 10:53
Describe        :
*/
int inCREDIT_PRINT_Test(void)
{
	BYTE			key;
        unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * 800];
	CTOS_FONT_ATTRIB	srFont_Attrib;
//	// TODO: Add your program here //
//	CTOS_LCDTClearDisplay();
//            
//        CTOS_LCDTPrintXY(1, 1, "Hello");
	
	srFont_Attrib.X_Zoom = 1;		/* 1, it means normal size, and 2 means double size. 0 means print nothing . */
	srFont_Attrib.Y_Zoom = 1;		/* 1, it means normal size, and 2 means double size. 0 means print nothing . */
	srFont_Attrib.X_Space = 0;		/* The space in dot to insert between each character in x coordinate. */
	srFont_Attrib.Y_Space = 0;		/* The space in dot to insert between each character in y coordinate. */
	srFont_Attrib.FontSize = d_FONT_12x24;
            
	CTOS_PrinterBufferInit(uszBuffer, 800);
	CTOS_PrinterBufferPutStringAligned(uszBuffer, 20, (unsigned char*)"123", &srFont_Attrib, d_PRINTER_ALIGNLEFT);
	CTOS_PrinterBufferPutString(uszBuffer, 160, 20, (unsigned char*)"123", &srFont_Attrib);
	CTOS_PrinterBufferPutStringAligned(uszBuffer, 20, (unsigned char*)"123", &srFont_Attrib, d_PRINTER_ALIGNRIGHT);
	
	CTOS_PrinterBufferOutput(uszBuffer, 12);
	
        CTOS_KBDGet(&key);
	
	return 0;
}

/*
Function        :inCREDIT_PRINT_TerminalTraceLog
Date&Time       :2019/2/13 
Describe        :print tracelog  設定為OPT使用，所以需要加上TRANSACTION_OBJECT*作為參數
 */
int inCREDIT_PRINT_TerminalTraceLog(TRANSACTION_OBJECT *pobTran)
{
        int		inHandle = -1, inRetVal = VS_ERROR;
        int		i = 0, j = 0, inCnt = 1;
        int		n = 0, k = 0;
	int		inReadSize = 0;
        long		lnReadSize = 0;
	char		szReadTemp[3000+1];	//2019/2/21 下午 6:21
        char		szPrintBuf[256 + 1], szTemplate[38 + 1],szTempBuf[39];
        char		szPathBuf[100];		//2019/2/19 下午 4:47
        unsigned long	ulHandle;
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_PRINT_TerminalTraceLog START!");
        
        strcpy(szPathBuf, _FS_DATA_PATH_);
        strcat(szPathBuf, _TMS_TRACE_LOG_);

        inRetVal = inFile_Linux_Open(&inHandle, szPathBuf);
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "Handle is %d, line = %d\n", inHandle, __LINE__);	 //2019/2/21 上午 9:32
	}
	
        if (inRetVal != VS_SUCCESS)
        {
                inPRINT_ChineseFont("TRACE_LOG_OPEN_FAILURE", _PRT_ISO_);
                inPRINT_ChineseFont(szPathBuf, _PRT_ISO_);
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "function = %s ,line = %d", __FUNCTION__, __LINE__);
		}
        } 
        else
        {
                inPRINT_ChineseFont("TRACE_LOG_OPEN_SUCCESS", _PRT_ISO_);//2019/2/20 上午 9:39        
  
                lnReadSize = lnFILE_GetSize(&ulHandle, (unsigned char *)_TMS_TRACE_LOG_);
                inFile_Linux_Seek(inHandle, 0L, _SEEK_BEGIN_);
		inReadSize = (int)lnReadSize;
                inFile_Linux_Read(inHandle, szReadTemp, &inReadSize);
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "lnReadSize=%ld,inRetVal=%d\n", lnReadSize, inRetVal);
		}
        
                if (inReadSize == (int)lnReadSize)
                {
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "function = %s ,line = %d",__FUNCTION__,__LINE__);
			}
                        memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
       
                        for (i = 0; i < (int)lnReadSize; i++)
                        {
                                szPrintBuf[j++] = szReadTemp[i];
                                
                                if (szReadTemp[i] == 0x0A )
                                {
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "function = %s ,line = %d", __FUNCTION__, __LINE__);
						inLogPrintf(AT, "i==%d ,j == %d", i, j);
					}
					
                                        memset(szTemplate,0x00,sizeof(szTemplate));
                                        snprintf(szTemplate,sizeof(szTemplate), "%03d. =================================", (inCnt ++));
                                        inPRINT_ChineseFont(szTemplate, _PRT_ISO_);
                                      
                                        n = (int)ceil((strlen(szPrintBuf) / 38)); /*n=字串需要印幾行*/
                                        for(k = 0; k < n+1; k++)
                                        {
                                            memset(szTempBuf,0x00,sizeof(szTempBuf));
                                            memcpy(szTempBuf,(szPrintBuf + k * 38),38);
                                            inPRINT_ChineseFont(szTempBuf, _PRT_ISO_);
                                        }
                                        szPrintBuf[j - 1] = 0x00;
                                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf)); 
                                        j = 0;
                                }       
                        }
                }
        }   
    
        inFile_Linux_Close(inHandle);
	
        return (VS_SUCCESS);
}



/*
Function        :inCREDIT_PRINT_DCC_GRAND_HOTEL_ReceiptEND_ByBuffer
Date&Time       :2022/9/5 下午 5:27
Describe        :列印結尾
*/
int inCREDIT_PRINT_DCC_GRAND_HOTEL_ReceiptEND_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
	int	inRetVal = VS_ERROR;
	char	szTemplate[42 + 1] = {0};
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
                        inPRINT_Buffer_PutIn("*** 商店收據 Merchant Copy ***", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_CUST_;

                }
                else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
                {
                        inPRINT_Buffer_PutIn("*** 持卡人收據 Customer Copy ***", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_MERCH_;
                }

                inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
        }
        else
        {
		/* 免責宣言上需空一行 by Russell 2020/10/29 下午 3:21 */
		inRetVal = inPRINT_Buffer_PutIn("  ", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                /* 橫式 */
                if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
                {
			
			/* Disclaimer */
			/* 轉台幣也要印 */
			if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_))) && 
			    (pobTran->srBRec.inCode == _SALE_		|| 
			     pobTran->srBRec.inCode == _TIP_		|| 
			     pobTran->srBRec.inCode == _PRE_COMP_	||
			    (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
			{
				if (inCREDIT_PRINT_DCC_Disclaimer_ByBuffer_061_VISA(pobTran, uszBuffer, srFont_Attrib, srBhandle) == VS_ERROR)
					return (VS_ERROR);
			}
			else if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_))) &&
				 (pobTran->srBRec.inCode == _SALE_	|| 
				  pobTran->srBRec.inCode == _TIP_	|| 
				  pobTran->srBRec.inCode == _PRE_COMP_	||
			         (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
			{
				if (inCREDIT_PRINT_DCC_Disclaimer_ByBuffer_061_MASTERCARD(pobTran, uszBuffer, srFont_Attrib, srBhandle) == VS_ERROR)
					return (VS_ERROR);
			}
			
			if (pobTran->srBRec.uszDCCTransBit == VS_TRUE && pobTran->srBRec.inCode == _TIP_)
			{
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
			
                        /* 簽名欄 */
			/* 免簽名 */
			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
			{
				/* 免簽名 */
				inRetVal = inPRINT_Buffer_PutIn("免      簽      名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
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
				/* 圖檔存在、有在signpad簽名、且非重印（重印不出簽名） （目前簽名狀態存不了Batch，先把&& pobTran->srBRec.inSignStatus == _SIGN_SIGNED_此條件拿掉） */
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
						inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					}
					
				}
				
			}
                        
                        inPRINT_Buffer_PutIn("X:________________________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 持卡人姓名 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%s", pobTran->srBRec.szCardHolder);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        inPRINT_Buffer_PutIn("持卡人簽名(特店存根聯)", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
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
				if (inCREDIT_PRINT_DCC_Disclaimer_ByBuffer_061_VISA(pobTran, uszBuffer, srFont_Attrib, srBhandle) == VS_ERROR)
					return (VS_ERROR);
			}
			else if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_))) && 
			    (pobTran->srBRec.inCode == _SALE_		|| 
			     pobTran->srBRec.inCode == _TIP_		|| 
			     pobTran->srBRec.inCode == _PRE_COMP_	||
			    (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
			{
				if (inCREDIT_PRINT_DCC_Disclaimer_ByBuffer_061_MASTERCARD(pobTran, uszBuffer, srFont_Attrib, srBhandle) == VS_ERROR)
					return (VS_ERROR);
			}
			
			if (pobTran->srBRec.uszDCCTransBit == VS_TRUE && pobTran->srBRec.inCode == _TIP_)
			{
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
				inRetVal = inPRINT_Buffer_PutIn("免      簽      名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        inPRINT_Buffer_PutIn("　　　　　　　　　 商店存根", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 持卡人姓名 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate,"%s",pobTran->srBRec.szCardHolder);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
				if (inCREDIT_PRINT_DCC_Disclaimer_ByBuffer_061_VISA(pobTran, uszBuffer, srFont_Attrib, srBhandle) == VS_ERROR)
					return (VS_ERROR);
			}
			else if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_))) && 
			    (pobTran->srBRec.inCode == _SALE_		|| 
			     pobTran->srBRec.inCode == _TIP_		|| 
			     pobTran->srBRec.inCode == _PRE_COMP_	||
			    (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
			{
				if (inCREDIT_PRINT_DCC_Disclaimer_ByBuffer_061_MASTERCARD(pobTran, uszBuffer, srFont_Attrib, srBhandle) == VS_ERROR)
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
				inRetVal = inPRINT_Buffer_PutIn("免      簽      名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        inPRINT_Buffer_PutIn("持卡人存根", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			inPRINT_Buffer_PutIn("Card holder stub", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			/* 持卡人姓名 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate,"%s",pobTran->srBRec.szCardHolder);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
		
		if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
		{
			inPRINT_Buffer_PutIn("重印 REPRINT", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		}
		
                /* 警示語 */
                inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
                inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		
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
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
        }

        return (VS_SUCCESS);
	
}

/*
Function        :inCREDIT_PRINT_DCC_Disclaimer_ByBuffer_061_VISA
Date&Time       :2022/9/5 下午 6:00
Describe        :印Disclaimer
 *		061用hardcode
*/
int inCREDIT_PRINT_DCC_Disclaimer_ByBuffer_061_VISA(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	inPRINT_Buffer_PutIn("VISA Legal Notice(Disclaimer Text): I have", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("been offered a choice of currencies and", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("agree to pay in the selected Transaction", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("Currency. Dynamic Currency Conversion", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("(DCC) is offered by the Merchant.", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("Cardholder expressly agrees to the", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
	if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ || pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
	{
		inPRINT_Buffer_PutIn("Transaction Receipt Information by marking", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("the “accept box” below.", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	else
	{
		inPRINT_Buffer_PutIn("Transaction Receipt Information.", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
				
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_DCC_Disclaimer_ByBuffer_061_MASTERCARD
Date&Time       :2022/9/5 下午 6:01
Describe        :印Disclaimer
 *		061用hardcode
*/
int inCREDIT_PRINT_DCC_Disclaimer_ByBuffer_061_MASTERCARD(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	inPRINT_Buffer_PutIn("M/C Legal Notice: I have chosen not to use", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("the MasterCard currency conversion process", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("and I will have no recourse against", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("MasterCard concerning the currency", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("conversion or its disclosure. Cardholder", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("expressly agrees to the Transaction", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ || pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
	{
		
		inPRINT_Buffer_PutIn("Receipt Information by marking the", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("“accept box” below.", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	else
	{
		inPRINT_Buffer_PutIn("Receipt Information.", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
				
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Logo_ByBuffer_041_043
Date&Time       :2022/9/29 上午 10:21
Describe        :列印LOGO
*/
int inCREDIT_PRINT_Logo_ByBuffer_041_043(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char	szCustomerIndicator[3 + 1] = {0};
        char    szShort_Receipt_Mode[1 + 1] = {0};
	
        memset(szShort_Receipt_Mode, 0x00, sizeof(szShort_Receipt_Mode));
        inGetShort_Receipt_Mode(szShort_Receipt_Mode);
        
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 印Slogan 384*180 */
	if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
	{
		if (inCREDIT_PRINT_MarchantSlogan(pobTran, _NCCC_SLOGAN_PRINT_UP_, uszBuffer, srBhandle) != VS_SUCCESS)
			return (VS_ERROR);
	}
	
        if (!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))
        {
                /* 預借現金客製化不印NCCC LOGO */
                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
                {

                }
                else
                {
                        /* 印NCC的LOGO */
                        if (inPRINT_Buffer_PutIn(_NCCC_TEXT_LOGO_, _FONT_SIZE_SHORT_RECEIPT_U_NCCC_LOGO_TEXT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_) != VS_SUCCESS)
                        {
                                return (VS_ERROR);
                        }
                }
        }
        else
        {
                /* 預借現金客製化不印NCCC LOGO */
                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
                {

                }
                else
                {
                        /* 印NCC的LOGO */
                        if (inPRINT_Buffer_PutGraphic((unsigned char*)_BANK_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inBankLogoHeight, _APPEND_) != VS_SUCCESS)
                        {
                                return (VS_ERROR);
                        }
                }
        }

        /* 印商店的LOGO */
        if (inCREDIT_PRINT_MerchantLogo(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
	{
                return (VS_ERROR);
        }
	
	/* 印商店名稱 */
        if (inCREDIT_PRINT_MerchantName(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Data_ByBuffer_041_043
Date&Time       :2022/9/29 上午 10:39
Describe        :列印DATA
*/
int inCREDIT_PRINT_Data_ByBuffer_041_043(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
        int     inRetVal;
        char 	szPrintBuf[100 + 1], szPrintBuf1[42 + 1], szPrintBuf2[42 + 1], szTemplate1[42 + 1], szTemplate2[42 + 1];
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
                inFunc_GetTransType(pobTran, szPrintBuf1, szTemplate2);
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "交易類別：%s",szPrintBuf1);
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

                /* 城市別(City) */
                inRetVal = inPRINT_Buffer_PutIn("城市別(City)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
                inGetCityName(szPrintBuf1);
                sprintf(szPrintBuf, "%s", szPrintBuf1);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 卡別 檢查碼 */
		inRetVal = inPRINT_Buffer_PutIn("卡別(Card Type)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("檢查碼(Check No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                /* 卡別 */
		/* 【需求單 - 106349】自有品牌判斷需求 */
		/* unpack電文就存存CardLabel modify by LingHsiung 2020/2/14 上午 11:07 */
		inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 12, _PADDING_RIGHT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		/* 檢查碼 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
		if (strlen(pobTran->srBRec.szCheckNO) > 0)
		{
			strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
		}
		else
		{
			inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
		}
                memcpy(szPrintBuf1, szTemplate1, strlen(szTemplate1));
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 卡號 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                strcpy(szPrintBuf, pobTran->srBRec.szPAN);
                inRetVal = inPRINT_Buffer_PutIn("卡號(Card No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                strcpy(szPrintBuf, pobTran->srBRec.szPAN);
		
		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		/* 卡號遮掩(一般卡號前6後4，U Card前3後5) */
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
		if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			/* Uny交易不受TMS開關限制，且獲取卡號時已遮掩 */
		}
		else
		{
			if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				for (i = 6; i < (strlen(szPrintBuf) - 4); i ++)
				{
					szPrintBuf[i] = 0x2A;
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
					for (i = 6; i < (strlen(szPrintBuf) - 4); i ++)
					{
						szPrintBuf[i] = 0x2A;
					}
				}

			}
		}
		
		/* 過卡方式 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf, "(W)");
			else
				strcat(szPrintBuf, "(C)");
		}
		else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			strcat(szPrintBuf, "(Q)");
		}
		else if (pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
		{
			strcat(szPrintBuf, "(Q)");
		}
		else
		{
			if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
				strcat(szPrintBuf,"(C)");
			else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
				strcat(szPrintBuf, "(T)");
			else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf, "(W)");
			else
			{
				if (pobTran->srBRec.uszManualBit == VS_TRUE)
				{
					/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
					/* 電文轉Manual Keyin但是簽單要印感應的W */
					if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
						strcat(szPrintBuf, "(W)");
					else
						strcat(szPrintBuf,"(M)");
				}
				else
					strcat(szPrintBuf,"(S)");
			}
		}
		
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 主機別 & 交易別 */
		inRetVal = inPRINT_Buffer_PutIn("主機別/交易類別(Host/Trans.Type)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		inGetHostLabel(szTemplate1);
		sprintf(szPrintBuf1, "%s", szTemplate1);

		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));

		sprintf(szPrintBuf2, "%s", szTemplate1);
		sprintf(szPrintBuf, "%s %s", szPrintBuf1 , szPrintBuf2);

		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		if (strlen(szTemplate2) > 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate2);

			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
                
		/* 批次號碼、授權碼 */
		inRetVal = inPRINT_Buffer_PutIn("批次號碼(Batch No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("授權碼(Auth Code)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* Batch Num */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "%03ld", pobTran->srBRec.lnBatchNum);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		
		if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* Auth Code */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		memcpy(szPrintBuf1, pobTran->srBRec.szAuthCode, _AUTH_CODE_SIZE_);
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 回覆碼 */
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			inRetVal = inPRINT_Buffer_PutIn("回覆碼(Resp. Code)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
			
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memcpy(szPrintBuf, pobTran->srBRec.szRespCode, 2);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}

                /* 日期時間 */
                inRetVal = inPRINT_Buffer_PutIn("日期/時間(Date/Time)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
		
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
                /* 序號 調閱編號 */
		inRetVal = inPRINT_Buffer_PutIn("序號(Ref. No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("調閱編號(Inv.No)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		/* 雖然電文RRN送12個byte，但RRN最後一碼是0x00，所以只看到11碼 */
		inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szRefNo, ' ', 12, _PADDING_RIGHT_);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%06ld", pobTran->srBRec.lnOrgInvNum);
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		
		if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 櫃號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetStoreIDEnable(szTemplate1);
		if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
		{
			inRetVal = inPRINT_Buffer_PutIn("櫃號(Store ID)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szStoreID, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
		/* 產品代碼 */
		inGetProductCodeEnable(szProductCodeEnable);
		if (memcmp(szProductCodeEnable, "Y", 1) == 0)
		{
			inRetVal = inPRINT_Buffer_PutIn("產品代碼(Product Code)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
			
			inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szProductCode, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
		/* TC */
		/* 驗測開關 */
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
				     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE		|| 
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
						inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
							inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
								/* 【需求單-109121】Mastercard交易簽單列印AID需求 by Russell 2020/7/23 下午 4:23 */
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
							inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
					inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
				inPRINT_Buffer_PutIn("      ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
		
		/* 斷行 */
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
        }

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_Amount_ByBuffer_041_043
Date&Time       :2022/9/29 上午 10:41
Describe        :列印AMOUNT
*/
int inCREDIT_PRINT_Amount_ByBuffer_041_043(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
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
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


                        /* 小費 */
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTipTxnAmount);
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PADDING_LEFT_);

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "小費(Tips)  :%s", szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


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
			  pobTran->srBRec.inCode == _REFUND_		|| 
			  pobTran->srBRec.inCode == _INST_REFUND_	|| 
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
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                
		for (i = 0; i < 2; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
        }
        /* 橫式 */
        /* 負向交易 */
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
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

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
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", "總計(Total) :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}
		else
		{
			if(pobTran->srBRec.uszVOIDBit == VS_TRUE)
			{
				/* 橫式 */
				/* 金額 */
				/* 取消退貨是正數 */
				if (pobTran->srBRec.inOrgCode == _REFUND_ || pobTran->srBRec.inOrgCode == _INST_REFUND_ || pobTran->srBRec.inOrgCode == _REDEEM_REFUND_)
				{
					/* 初始化 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
					inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

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
					sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
					inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", "總計(Total) :");
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
			}
			else
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetTransFunc(szTemplate);
				if (szTemplate[6] == 'Y')	/* 檢查是否有開小費 */
				{
					/* 退貨金額為負數 */
					if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
					{
						 /* 初始化 */
						memset(szTemplate, 0x00, sizeof(szTemplate));
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

						/* 將NT$ ＋數字塞到szTemplate中來inpad */
						sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
						inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", szTemplate);
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

						/* 把前面的字串和數字結合起來 */
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", "總計(Total) :");
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					}
					/* 預授不會有小費，所以拉出來 */
					else if (pobTran->srBRec.inCode == _PRE_AUTH_ || pobTran->srBRec.inCode == _PRE_COMP_)
					{
						/* 初始化 */
						memset(szTemplate, 0x00, sizeof(szTemplate));
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

						/* 將NT$ ＋數字塞到szTemplate中來inpad */
						sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
						inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", szTemplate);
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

						/* 把前面的字串和數字結合起來 */
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", "總計(Total) :");
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					}
					else
					{
						/* 金額 */
						/* 初始化 */
						memset(szTemplate, 0x00, sizeof(szTemplate));
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

						/* 將NT$ ＋數字塞到szTemplate中來inpad */
						sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
						inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", szTemplate);
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

						/* 把前面的字串和數字結合起來 */
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", "金額(Amount):");
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

						/* lnTipAmount為0表示非小費 */
						if (pobTran->srBRec.lnTipTxnAmount == 0L)
						{
							/* 小費 */
							inPRINT_Buffer_PutIn("小費(Tips)  :__________________________", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

							/* 總計 */
							inPRINT_Buffer_PutIn("總計(Total) :__________________________", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						}
						else
						{
							/* 小費 */
							/* 初始化 */
							memset(szTemplate, 0x00, sizeof(szTemplate));
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

							/* 將NT$ ＋數字塞到szTemplate中來inpad */
							sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTipTxnAmount);
							inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "%s", szTemplate);
							inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

							/* 把前面的字串和數字結合起來 */
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "%s", "小費(Tips)  :");
							inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


							/* 總計 */
							/* 初始化 */
							memset(szTemplate, 0x00, sizeof(szTemplate));
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

							/* 將NT$ ＋數字塞到szTemplate中來inpad */
							sprintf(szTemplate, "%ld",  (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
							inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "%s", szTemplate);
							inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

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
					 /* 初始化 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

					/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
					if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
					{
						sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
					}
					else
					{
						sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
					}

					inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					/* 把前面的字串和數字結合起來 */
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
Function        :inCREDIT_PRINT_ReceiptEND_ByBuffer_041_043
Date&Time       :2022/9/29 上午 10:44
Describe        :
*/
int inCREDIT_PRINT_ReceiptEND_ByBuffer_041_043(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
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
                        inPRINT_Buffer_PutIn("*** 商店收據 Merchant Copy ***", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_CUST_;

                }
                else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
                {
                        inPRINT_Buffer_PutIn("*** 持卡人收據 Customer Copy ***", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_MERCH_;
                }

                inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
					inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
					inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
				inPRINT_Buffer_PutIn("X:________________________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				/* 列印持卡人姓名 */
				inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("持卡人簽名(特店存根聯)", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				/* Mail 主旨: RE: 【112184】V3標準版郵購交易及客製化041、043預借現金不印免簽名調整之需求已發信問過登霖，預借現金簽單是否比照簽單規格，將Cardholder’s Signature」移除 2024/1/9 下午 3:10 */
			}
			else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
			{
				inPRINT_Buffer_PutIn("X:________________________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				/* 列印持卡人姓名 */
				inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("持卡人簽名(特店存根聯)", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("商店存根", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			}
			else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				inPRINT_Buffer_PutIn("持卡人存根", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("Card holder stub", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				/* 列印持卡人姓名 */
				inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			
			if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
			{
				inPRINT_Buffer_PutIn("重印 REPRINT", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			}

			/* 列印警示語 */
			inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

			/* 預借現金客製化專屬，於帳單最下方要印出備註標語 */
			if ((pobTran->inTransactionCode == _CASH_ADVANCE_ || pobTran->inTransactionCode == _FORCE_CASH_ADVANCE_ || pobTran->inRunOperationID == _OPERATION_REPRINT_) &&
			    pobTran->srBRec.inCode != _VOID_)
			{
				inDisclaimerFontSize = _PRT_HEIGHT_SMALL_;
				inPRINT_Buffer_PutIn("      ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
				inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
		{
			if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
			    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
			{
				inPRINT_Buffer_PutIn("X:________________________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				/* 列印持卡人姓名 */
				inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("持卡人簽名(特店存根聯)", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			}
			else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				inPRINT_Buffer_PutIn("持卡人存根", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("Card holder stub", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				/* 列印持卡人姓名 */
				inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			
			if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
			{
				inPRINT_Buffer_PutIn("重印 REPRINT", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			}

			/* 列印警示語 */
			inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

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
				inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}
		else
		{
			if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
				inPRINT_Buffer_PutIn("持卡人簽名(特店存根聯)", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
			{
				inPRINT_Buffer_PutIn("商店存根", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				/* 列印持卡人姓名 */
				inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				inPRINT_Buffer_PutIn("持卡人存根", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("Card holder stub", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				/* 列印持卡人姓名 */
				inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			
			if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
			{
				inPRINT_Buffer_PutIn("重印 REPRINT", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			}

			/* 列印警示語 */
			inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

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
				inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}
		
        }

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Data_ByBuffer_005
Date&Time       :2022/10/5 下午 8:41
Describe        :多印了發票號碼
*/
int inCREDIT_PRINT_Data_ByBuffer_005(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
        int     inRetVal;
        char 	szPrintBuf[100 + 1], szPrintBuf1[42 + 1], szPrintBuf2[42 + 1], szTemplate1[42 + 1], szTemplate2[42 + 1];
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
	char	szExamBit[10 + 1] = {0};
	unsigned char	uszChangeLineBit = VS_FALSE;

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
                inFunc_GetTransType(pobTran, szPrintBuf1, szTemplate2);
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "交易類別：%s",szPrintBuf1);
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

                /* 城市別(City) */
                inRetVal = inPRINT_Buffer_PutIn("城市別(City)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
                inGetCityName(szPrintBuf1);
                sprintf(szPrintBuf, "%s", szPrintBuf1);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 卡別 檢查碼 */
		inRetVal = inPRINT_Buffer_PutIn("卡別(Card Type)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("檢查碼(Check No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                /* 卡別 */
		/* 【需求單 - 106349】自有品牌判斷需求 */
		/* unpack電文就存存CardLabel modify by LingHsiung 2020/2/14 上午 11:07 */
		inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 12, _PADDING_RIGHT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		/* 檢查碼 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
		if (strlen(pobTran->srBRec.szCheckNO) > 0)
		{
			strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
		}
		else
		{
			inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
		}
                memcpy(szPrintBuf1, szTemplate1, strlen(szTemplate1));
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 卡號 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                strcpy(szPrintBuf, pobTran->srBRec.szPAN);
                inRetVal = inPRINT_Buffer_PutIn("卡號(Card No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                strcpy(szPrintBuf, pobTran->srBRec.szPAN);
		
		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		/* 卡號遮掩(一般卡號前6後4，U Card前3後5) */
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
		if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			/* Uny交易不受TMS開關限制，且獲取卡號時已遮掩 */
		}
		else
		{
			if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				for (i = 6; i < (strlen(szPrintBuf) - 4); i ++)
				{
					szPrintBuf[i] = 0x2A;
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
					for (i = 6; i < (strlen(szPrintBuf) - 4); i ++)
					{
						szPrintBuf[i] = 0x2A;
					}
				}

			}
		}
		
		/* 過卡方式 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf, "(W)");
			else
				strcat(szPrintBuf, "(C)");
		}
		else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			strcat(szPrintBuf, "(Q)");
		}
		else if (pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
		{
			strcat(szPrintBuf, "(Q)");
		}
		else
		{
			if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
				strcat(szPrintBuf,"(C)");
			else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
				strcat(szPrintBuf, "(T)");
			else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf, "(W)");
			else
			{
				if (pobTran->srBRec.uszManualBit == VS_TRUE)
				{
					/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
					/* 電文轉Manual Keyin但是簽單要印感應的W */
					if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
						strcat(szPrintBuf, "(W)");
					else
						strcat(szPrintBuf,"(M)");
				}
				else
					strcat(szPrintBuf,"(S)");
			}
		}
		
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 主機別 & 交易別 */
		inRetVal = inPRINT_Buffer_PutIn("主機別/交易類別(Host/Trans.Type)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		inGetHostLabel(szTemplate1);
		sprintf(szPrintBuf1, "%s", szTemplate1);

		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));

		sprintf(szPrintBuf2, "%s", szTemplate1);
		sprintf(szPrintBuf, "%s %s", szPrintBuf1 , szPrintBuf2);

		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		if (strlen(szTemplate2) > 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate2);

			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
                
		/* 批次號碼、授權碼 */
		inRetVal = inPRINT_Buffer_PutIn("批次號碼(Batch No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("授權碼(Auth Code)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* Batch Num */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "%03ld", pobTran->srBRec.lnBatchNum);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		
		if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* Auth Code */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		memcpy(szPrintBuf1, pobTran->srBRec.szAuthCode, _AUTH_CODE_SIZE_);
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 回覆碼 */
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			inRetVal = inPRINT_Buffer_PutIn("回覆碼(Resp. Code)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
			
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memcpy(szPrintBuf, pobTran->srBRec.szRespCode, 2);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}

                /* 日期時間 */
                inRetVal = inPRINT_Buffer_PutIn("日期/時間(Date/Time)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
		
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
                /* 序號 調閱編號 */
		inRetVal = inPRINT_Buffer_PutIn("序號(Ref. No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("調閱編號(Inv.No)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		/* 雖然電文RRN送12個byte，但RRN最後一碼是0x00，所以只看到11碼 */
		inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szRefNo, ' ', 12, _PADDING_RIGHT_);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%06ld", pobTran->srBRec.lnOrgInvNum);
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		
		if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 櫃號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetStoreIDEnable(szTemplate1);
		if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
		{
			inRetVal = inPRINT_Buffer_PutIn("櫃號(Store ID)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szStoreID, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
		if (strlen(pobTran->srBRec.szFPG_FTC_Invoice) > 0)
		{
			inRetVal = inPRINT_Buffer_PutIn("發票號碼", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
			inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szFPG_FTC_Invoice, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
		/* 產品代碼 */
		inGetProductCodeEnable(szProductCodeEnable);
		if (memcmp(szProductCodeEnable, "Y", 1) == 0)
		{
			inRetVal = inPRINT_Buffer_PutIn("產品代碼(Product Code)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
			
			inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szProductCode, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
		/* TC */
		/* 驗測開關 */
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
				     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE		|| 
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
						inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
							inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
								/* 【需求單-109121】Mastercard交易簽單列印AID需求 by Russell 2020/7/23 下午 4:23 */
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
							inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
					inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
			}
		}
		
		/* 斷行 */
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
        }

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_HG_Data_ByBuffer_005
Date&Time       :2022/10/5 下午 8:42
Describe        :多印了發票號碼
*/
int inCREDIT_PRINT_HG_Data_ByBuffer_005(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int     i;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1], szTemplate1[42 + 1];
	char	szProductCodeEnable[1 + 1];
        
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        /* HG卡號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        strcpy(szTemplate, pobTran->srBRec.szHGPAN);
        /* 卡號遮掩 */
        if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
        {
		/* HAPPG_GO 卡不掩飾 */
		if (!memcmp(&pobTran->srBRec.szHGPAN[0], "9552", 4))
		{

		}
		else
		{
			for (i = 6; i < (strlen(szTemplate) - 4); i ++)
				szTemplate[i] = 0x2A;
		}
        }
        sprintf(szPrintBuf, "ＨＧ卡號　 %s", szTemplate);
        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        /* 主機別 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "主機　　　 HAPPY GO");
        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        /* 交易類別 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        vdCREDIT_PRINT_HG_GetTransactionType(pobTran, szTemplate, szTemplate1);
        sprintf(szPrintBuf, "交易類別　 %s%s", szTemplate, szTemplate1);
        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        /* 日期 & 時間 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "日期時間　 %.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        /* 調閱編號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "調閱編號　 %s/%06ld", pobTran->srBRec.szHGRefNo, pobTran->srBRec.lnOrgInvNum);
        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        /* 櫃號 */
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
        inGetStoreIDEnable(szTemplate1);
        if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "櫃號　　　 %s", pobTran->srBRec.szStoreID);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }
	
	if (strlen(pobTran->srBRec.szFPG_FTC_Invoice) > 0)
	{
		inPRINT_Buffer_PutIn("發票號碼", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn(pobTran->srBRec.szFPG_FTC_Invoice, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	/* 產品代碼 */
	inGetProductCodeEnable(szProductCodeEnable);
	if (memcmp(szProductCodeEnable, "Y", 1) == 0)
	{
		inPRINT_Buffer_PutIn("產品代碼(Product Code)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutIn(pobTran->srBRec.szProductCode, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
        
        /* 斷行 */
	inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmount_ByBuffer_TAKA
Date&Time       :2022/10/17 下午 3:11
Describe        :列印總金額
*/
int inCREDIT_PRINT_TotalAmount_ByBuffer_TAKA(TRANSACTION_OBJECT *pobTran, void *srAccumRecOrg, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szPrintBuf[84 + 1], szTemplate[84 + 1];
	TAKA_ACCUM_TOTAL_REC *srAccumRec = NULL;
	
	srAccumRec = srAccumRecOrg;

        if (pobTran->inRunOperationID == _OPERATION_SETTLE_)
	{
                /* 客製化098，結帳流程，預先列印，TITLE為總額 */
                if (pobTran->uszPrePrintBit == VS_TRUE)
                        inPRINT_Buffer_PutIn("總額報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
                else
                        inPRINT_Buffer_PutIn("結帳報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	}
	else
        {
                inPRINT_Buffer_PutIn("總額報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
        }

        inPRINT_Buffer_PutIn("    筆數(CNT)      金額(AMOUNT)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        if (srAccumRec->lnTotalCount == 0)
        {
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 Ｄ　%03lu   NT$", 0L);
                sprintf(szTemplate, "%ld", 0L);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 Ｒ　%03lu   NT$", 0L);
                sprintf(szTemplate, "%ld", 0L);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 淨額 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "淨額 Ｔ　%03lu   NT$", 0L);
                sprintf(szTemplate, "%ld", 0L);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }
        else
        {
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 Ｄ　%03lu   NT$", srAccumRec->lnTotalSaleCount);
                sprintf(szTemplate, "%lld", srAccumRec->llTotalSaleAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
		strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 Ｒ　%03lu   NT$", srAccumRec->lnTotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 淨額 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "淨額 Ｔ　%03lu   NT$", srAccumRec->lnTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalReport_ByBuffer_TAKA
Date&Time       :2022/10/17 下午 3:19
Describe        :列印總額帳單
*/
int inCREDIT_PRINT_TotalReport_ByBuffer_TAKA(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = 0, inPrintIndex = 0;
	char			szDebugMsg[100 + 1] = {0};
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
        TAKA_ACCUM_TOTAL_REC	srAccumRec;
	
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

		inPrintIndex = _TOTAL_REPORT_INDEX_TAKA_;

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "PrintIndex : %d", inPrintIndex);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 開啟交易總的檔案 */
		memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		inRetVal = inACCUM_GetRecord_General(pobTran, &srAccumRec, sizeof(TAKA_ACCUM_TOTAL_REC));

		if (inRetVal == VS_NO_RECORD)
			memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		else if (inRetVal == VS_ERROR)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Get record 失敗.");	
			}

			return (VS_ERROR);
		}

		while (1)
		{
			inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
			/* 列印LOGO */
			if (srTotalReport_ByBuffer[inPrintIndex].inReportLogo != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inReportLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 列印TID MID */
			if (srTotalReport_ByBuffer[inPrintIndex].inReportTop != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inReportTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 全部金額總計 */
			if (srTotalReport_ByBuffer[inPrintIndex].inAmount != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmount(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			/* 有金額才印 */
			if (srAccumRec.llTotalSaleAmount != 0L || srAccumRec.llTotalRefundAmount != 0L)
			{
				/* 卡別金額總計 */
				if (srTotalReport_ByBuffer[inPrintIndex].inAmountByCard != NULL)
					if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmountByCard(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
						return (inRetVal);
			}

			/* 結束 */
			if (srTotalReport_ByBuffer[inPrintIndex].inReportEnd != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inReportEnd(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);
			break;
		}

		return (VS_SUCCESS);
	}
}

/*
Function        :inCREDIT_PRINT_DetailReport_ByBuffer_TAKA
Date&Time       :2022/10/17 下午 2:36
Describe        :列印明細帳單
*/
int inCREDIT_PRINT_DetailReport_ByBuffer_TAKA(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = 0, inPrintIndex = 0, inRecordCnt = 0;
	char			szDebugMsg[100 + 1];
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
        TAKA_ACCUM_TOTAL_REC	srAccumRec;
	
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_);/* 處理中... */

		inPrintIndex = _DETAIL_REPORT_INDEX_TAKA_;

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "PrintIndex : %d", inPrintIndex);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 檢查是否有帳 */
		if (srDetailReport_ByBuffer[inPrintIndex].inReportCheck != NULL)
		{
			if ((inRecordCnt = srDetailReport_ByBuffer[inPrintIndex].inReportCheck(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) == VS_ERROR)
				return (VS_ERROR); /* 表示檔案開啟失敗 */
		}

		/* 開啟交易總的檔案 */
		memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		inRetVal = inACCUM_GetRecord_General(pobTran, &srAccumRec, sizeof(TAKA_ACCUM_TOTAL_REC));

		if (inRetVal == VS_NO_RECORD)
			memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		else if (inRetVal == VS_ERROR)
		{
			inLogPrintf(AT, "Get record 失敗.");

			return (VS_ERROR);
		}

		while (1)
		{
			inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
			srBhandle1.uszDetailPrint = VS_TRUE;
			
			/* 列印LOGO */
			if (srDetailReport_ByBuffer[inPrintIndex].inReportLogo != NULL)
				if ((inRetVal = srDetailReport_ByBuffer[inPrintIndex].inReportLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 列印TID MID */
			if (srDetailReport_ByBuffer[inPrintIndex].inReportTop != NULL)
				if ((inRetVal = srDetailReport_ByBuffer[inPrintIndex].inReportTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 全部金額總計 */
			if (srDetailReport_ByBuffer[inPrintIndex].inTotalAmount != NULL)
				if ((inRetVal = srDetailReport_ByBuffer[inPrintIndex].inTotalAmount(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			/* 明細規格 */
			if (srDetailReport_ByBuffer[inPrintIndex].inMiddle != NULL)
				if ((inRetVal = srDetailReport_ByBuffer[inPrintIndex].inMiddle(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 明細資料 */
			if (srDetailReport_ByBuffer[inPrintIndex].inBottom != NULL)
			{
				inRetVal = srDetailReport_ByBuffer[inPrintIndex].inBottom(pobTran, inRecordCnt, uszBuffer1, &srFont_Attrib1, &srBhandle1);
				if (inRetVal != VS_SUCCESS &&
				    inRetVal != VS_NO_RECORD)
				{
				       return (inRetVal);
				}
			}

			/* 結束 */
			if (srDetailReport_ByBuffer[inPrintIndex].inReportEnd != NULL)
				if ((inRetVal = srDetailReport_ByBuffer[inPrintIndex].inReportEnd(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);
			
			if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);

			break;
		}

		return (VS_SUCCESS);
	}
}

/*
Function        :inCREDIT_PRINT_TotalAmountByCard_ByBuffer_TAKA
Date&Time       :2022/10/17 下午 3:26
Describe        :依卡別列印
*/
int inCREDIT_PRINT_TotalAmountByCard_ByBuffer_TAKA(TRANSACTION_OBJECT *pobTran, void *srAccumRecOrg, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szPrintBuf[84 + 1] = {0}, szTemplate[84 + 1] = {0};
	TAKA_ACCUM_TOTAL_REC *srAccumRec;
	srAccumRec = srAccumRecOrg;

        inPRINT_Buffer_PutIn("卡別小計", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

        if (srAccumRec->llTAKA_EMPLOYEE_TotalSaleAmount != 0L || srAccumRec->llTAKA_EMPLOYEE_TotalRefundAmount != 0L)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_TAKA_EMPLOYEE_);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnTAKA_EMPLOYEE_TotalSaleCount);
                sprintf(szTemplate, "%lld", srAccumRec->llTAKA_EMPLOYEE_TotalSaleAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnTAKA_EMPLOYEE_TotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llTAKA_EMPLOYEE_TotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnTAKA_EMPLOYEE_TotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llTAKA_EMPLOYEE_TotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }

        if (srAccumRec->llTAKA_T_DAYEH_TotalSaleAmount != 0L || srAccumRec->llTAKA_T_DAYEH_TotalRefundAmount != 0L)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_TAKA_T_DAYEH_);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnTAKA_T_DAYEH_TotalSaleCount);
                sprintf(szTemplate, "%lld", srAccumRec->llTAKA_T_DAYEH_TotalSaleAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnTAKA_T_DAYEH_TotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llTAKA_T_DAYEH_TotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnTAKA_T_DAYEH_TotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llTAKA_T_DAYEH_TotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }

        if (srAccumRec->llTAKA_T_CARD_TotalSaleAmount != 0L || srAccumRec->llTAKA_T_CARD_TotalRefundAmount != 0L)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_TAKA_T_CARD_);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnTAKA_T_CARD_TotalSaleCount);
                sprintf(szTemplate, "%lld", srAccumRec->llTAKA_T_CARD_TotalSaleAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnTAKA_T_CARD_TotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llTAKA_T_CARD_TotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnTAKA_T_CARD_TotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llTAKA_T_CARD_TotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }
	
        return (VS_SUCCESS);
}
