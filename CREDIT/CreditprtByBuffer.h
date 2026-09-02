/* 
 * File:   CreditprtByBuffer.h
 * Author: user
 *
 * Created on 2016年1月30日, 上午 11:55
 */

/* 命名先_客製化再_簽單類別 暫時不溯及既往 */
typedef enum
{
	_REPORT_INDEX_NORMAL_ = 0,
	_REPORT_INDEX_NORMAL_DCC_FOR_SALE_,
	_REPORT_INDEX_NORMAL_DCC_NOT_FOR_SALE_,
	_REPORT_INDEX_NORMAL_DCC_CHANGE_TWD_,
	_REPORT_INDEX_NORMAL_LOYALTY_REDEEM_,
	_REPORT_INDEX_NORMAL_VOID_LOYALTY_REDEEM_,
	_REPORT_INDEX_NORMAL_FISC_,
	_REPORT_INDEX_NORMAL_HG_SINGLE_,
	_REPORT_INDEX_NORMAL_HG_MULTIPLE_,
	_REPORT_INDEX_NORMAL_ESVC_IPASS_,
	_REPORT_INDEX_NORMAL_ESVC_ECC_,				
	_REPORT_INDEX_NORMAL_ESVC_ICASH_,
	_REPORT_INDEX_NORMAL_SMALL_,
	_REPORT_INDEX_NORMAL_DCC_FOR_SALE_SMALL_,
	_REPORT_INDEX_NORMAL_DCC_NOT_FOR_SALE_SMALL_,
	_REPORT_INDEX_NORMAL_DCC_CHANGE_TWD_SMALL_,
	_REPORT_INDEX_NORMAL_FISC_SMALL_,
	_REPORT_INDEX_NORMAL_HG_MULTIPLE_SMALL_,
	_REPORT_INDEX_NORMAL_046_,
	_REPORT_INDEX_NORMAL_DCC_FOR_SALE_046_,
	_REPORT_INDEX_NORMAL_DCC_NOT_FOR_SALE_046_,
	_REPORT_INDEX_NORMAL_DCC_CHANGE_TWD_046_,
	_REPORT_INDEX_NORMAL_FISC_046_,
	_REPORT_INDEX_NORMAL_HG_SINGLE_046_,
	_REPORT_INDEX_NORMAL_HG_MULTIPLE_046_,
	_REPORT_INDEX_NORMAL_ESVC_IPASS_046_,			
	_REPORT_INDEX_NORMAL_ESVC_ECC_046_,
	_REPORT_INDEX_NORMAL_ESVC_ICASH_046_,
	_REPORT_INDEX_NORMAL_027_,
	_REPORT_INDEX_NORMAL_FISC_027_,
	_REPORT_INDEX_NORMAL_HG_MULTIPLE_027_,
	_REPORT_INDEX_NORMAL_DCC_FOR_SALE_061_,
	_REPORT_INDEX_NORMAL_DCC_NOT_FOR_SALE_061_,
	_REPORT_INDEX_NORMAL_DCC_CHANGE_TWD_061_,
	_REPORT_INDEX_NORMAL_041_043_,
	_REPORT_INDEX_NORMAL_005_,
	_REPORT_INDEX_NORMAL_HG_SINGLE_005_,
	_REPORT_INDEX_NORMAL_HG_MULTIPLE_005_,
	_REPORT_INDEX_NORMAL_SMALL_S_,
	_REPORT_INDEX_NORMAL_DCC_FOR_SALE_SMALL_S_,
	_REPORT_INDEX_NORMAL_DCC_NOT_FOR_SALE_SMALL_S_,
	_REPORT_INDEX_NORMAL_DCC_CHANGE_TWD_SMALL_S_,
	_REPORT_INDEX_NORMAL_FISC_SMALL_S_,
	_REPORT_INDEX_NORMAL_HG_MULTIPLE_SMALL_S_,
	_REPORT_INDEX_NORMAL_041_043_SMALL_,
	_REPORT_INDEX_NORMAL_041_043_SMALL_S_,
        _REPORT_INDEX_TRUST_SALE_,
} RECEIPT_INDEX;

typedef enum
{
	_TOTAL_REPORT_INDEX_NORMAL_ = 0,
	_TOTAL_REPORT_INDEX_NCCC_,
	_TOTAL_REPORT_INDEX_046_NCCC,
	_TOTAL_REPORT_INDEX_TAKA_,
	_TOTAL_REPORT_INDEX_041_NCCC,
} RECEIPT_TOTAL_INDEX;

typedef enum
{
	_DETAIL_REPORT_INDEX_NORMAL_ = 0,
	_DETAIL_REPORT_INDEX_NCCC_,
	_DETAIL_REPORT_INDEX_DCC_,
	_DETAIL_REPORT_INDEX_HG_,
	_DETAIL_REPORT_INDEX_046_NCCC_,
	_DETAIL_REPORT_INDEX_046_DCC_,
	_DETAIL_REPORT_INDEX_046_HG_,
	_DETAIL_REPORT_INDEX_TAKA_,
	_DETAIL_REPORT_INDEX_041_NCCC_,
} RECEIPT_DETAIL_INDEX;

typedef enum
{
	_TOTAL_REPORT_INDEX_ESVC_ = 0,
	_TOTAL_REPORT_INDEX_ESVC_SETTLE_,
        _TOTAL_REPORT_INDEX_098_ESVC_,
	_TOTAL_REPORT_INDEX_098_ESVC_SETTLE_,
	_TOTAL_REPORT_INDEX_046_ESVC_,
	_TOTAL_REPORT_INDEX_046_ESVC_SETTLE_,
} RECEIPT_TOTAL_ESVC_INDEX;

typedef enum
{
	_DETAIL_REPORT_INDEX_ESVC_ = 0,
        _DETAIL_REPORT_INDEX_098_ESVC_,
	_DETAIL_REPORT_INDEX_046_ESVC_,
} RECEIPT_DETAIL_ESVC_INDEX;

typedef enum
{
	_DETAIL_REPORT_DUTYFREE_REPRINT_NCCC_INDEX_ = 0,
} RECEIPT_DETAIL_DUTYFREE_INDEX;

typedef enum
{
	_TOTAL_REPORT_INDEX_TRUST_ = 0,
} RECEIPT_TOTAL_TRUST_INDEX;

/* 列印帳單 (START) */
typedef struct
{
        int (*inLogo)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* LOGO */
        int (*inTop)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* TID and MID */
        int (*inData)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* DTAT */
        int (*inCUPAmount)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);	/* CUP_AMOUNT */
        int (*inAmount)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* AMOUNT */
        int (*inInstallment)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);	/* 分期資料 */
        int (*inRedemption)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);	/* 紅利資料 */
        int (*inOther)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* OTHER資料 */
        int (*inEnd)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* 簽名欄 */
} PRINT_RECEIPT_TYPE_TABLE_BYBUFFER;
/* 列印帳單 (END) */

#define _FONT_SIZE_SHORT_RECEIPT_U_TXNO_                _PRT_HEIGHT_
#define _FONT_SIZE_SHORT_RECEIPT_U_NCCC_LOGO_TEXT_      _PRT_12X30_

/* 雜用function (START) */
int inCREDIT_PRINT_MarchantSlogan(TRANSACTION_OBJECT *pobTran, int inPrintPosition, unsigned char *uszBuffer, BufferHandle *srBhandle);
int inCREDIT_PRINT_MerchantLogo(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, BufferHandle *srBhandle);
int inCREDIT_PRINT_MerchantName(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, BufferHandle *srBhandle);
int inCREDIT_PRINT_Notice(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, BufferHandle *srBhandle);
int inCREDIT_PRINT_Printing_Time(TRANSACTION_OBJECT *pobTran, int inFontSize, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Func7PrintDCCParamTerm_ByBuffer(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_RewardAdvertisement(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_MPAS_RewardAdvertisement(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_ESVC_RewardAdvertisement(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_AutoSettle_Failed_ByBuffer(void);
int inCREDIT_PRINT_TerminalTraceLog(TRANSACTION_OBJECT *pobTran);
/* 雜用function (END) */

/* 列印帳單 (START) */
int inCREDIT_PRINT_Receipt_ByBuffer(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_Logo_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Tidmid_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Data_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Cup_Amount_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Amount_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Inst_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Redeem_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_ReceiptEND_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
/* 列印帳單 (END) */

/* 列印帳單縮小版 (START) */
int inCREDIT_PRINT_Tidmid_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Data_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Cup_Amount_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Amount_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Inst_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Redeem_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_ReceiptEND_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_FISC_Data_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_FISC_Amount_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DCC_Data_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DCC_Amount_FOR_SALE_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DCC_Amount_NOT_FOR_SALE_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DCC_Amount_CHANGE_NTD_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DCC_ReceiptEND_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
/* 列印帳單縮小版 (END) */

int inCREDIT_PRINT_Check_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);

/* 列印總額報表(START) */
typedef struct
{
        int (*inReportCheck)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);					/* 檢查檔案是否存在 */
        int (*inReportLogo)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);					/* LOGO */
        int (*inReportTop)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);					/* TID and MID and DATA */
        int (*inAmount)(TRANSACTION_OBJECT *, void *, unsigned char *, FONT_ATTRIB *, BufferHandle *);			/* 全部金額總計 */
        int (*inAmountByCard)(TRANSACTION_OBJECT *, void *, unsigned char *, FONT_ATTRIB *, BufferHandle *);			/* 卡別金額總計 */
        int (*inAmountByInstallment)(TRANSACTION_OBJECT *, ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* 分期金額總計 */
        int (*inAmountByRedemption)(TRANSACTION_OBJECT *, ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* 紅利金額總計 */
	int (*inAmountByOther)(TRANSACTION_OBJECT *, ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* 其他總計 */
	int (*inAmountHG)(TRANSACTION_OBJECT *, HG_ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);			/* HG使用 */
        int (*inReportEnd)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);												/* 結束 */
} TOTAL_REPORT_TABLE_BYBUFFER;
/* 列印總額報表 (END) */


/* 列印總額報表 (START) */
int inCREDIT_PRINT_TotalReport_ByBuffer(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_DCC_TotalReport_ByBuffer(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_TotalReport_ByBuffer_NCCC(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_TotalReport_ByBuffer_DCC(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_TotalReport_ByBuffer_HG(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_Top_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmount_ByBuffer(TRANSACTION_OBJECT *pobTran, void *srAccumRecOrg, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmountByCard_ByBuffer(TRANSACTION_OBJECT *pobTran, void *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmountByInstllment_ByBuffer(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmountByRedemption_ByBuffer(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmountByOther(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Total_Loyalty_Redeem_ByBuffer(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Total_HG_ByBuffer(TRANSACTION_OBJECT *pobTran, HG_ACCUM_TOTAL_REC *srHGAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_End_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_ESC_Reinforce_Count_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
/* 列印總額報表 (END) */

/* 列印明細報表(START) */
typedef struct
{
        int (*inReportCheck)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);				/* 檢查檔案是否存在 */
        int (*inReportLogo)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);				/* LOGO */
        int (*inReportTop)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);				/* TID and MID and DATA */
        int (*inTotalAmount)(TRANSACTION_OBJECT *, void *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* 全部金額總計 */
        int (*inMiddle)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);					/* 明細中文 */
        int (*inBottom)(TRANSACTION_OBJECT *, int, unsigned char *, FONT_ATTRIB *, BufferHandle *);				/* 所有明細 */
        int (*inReportEnd)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);				/* 結束 */
} DETAIL_REPORT_TABLE_BYBUFFER;
/* 列印明細報表 (END) */

/* 列印明細報表(START) */
int inCREDIT_PRINT_DetailReport_ByBuffer(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_NCCC_DetailReport_ByBuffer(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_HG_DetailReport_ByBuffer(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_DetailReportMiddle_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_NCCC_DetailReportMiddle_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DetailReportBottom_ByBuffer(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
/* 列印明細報表 (END) */

/* 列印端末機參數(START) */
int inCREDIT_PRINT_Func7PrintParamTerm_ByBuffer(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_ParamLOGO_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char* uszBuffer, FONT_ATTRIB* srFont_Attrib, BufferHandle* srBhandle);
int inCREDIT_PRINT_ParamTermInformation_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_ParamHostDetailParam_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_ParamCardType_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_ParamManageNum_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_ProductCode_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_CAPublicKey_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_ParamSystemConfig_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_ParamLOGO_END_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
/* 列印端末機參數(END) */

/* DCC相關(START) */
int inCREDIT_PRINT_DCC_InquiryMessage_ByBuffer(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_DCC_Data_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DCC_Amount_FOR_SALE_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DCC_ReceiptEND_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(char *szFilename, int inFontSize, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);

int inCREDIT_PRINT_DCC_Amount_NOT_FOR_SALE_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DCC_Amount_CHANGE_NTD_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);

int inCREDIT_PRINT_DCC_DetailReportMiddle_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DCC_DetailReportBottom_ByBuffer(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
/* DCC相關 (END) */

/* SmartPay相關 */
int inCREDIT_PRINT_FISC_Data_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_FISC_Amount_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);

/* 優惠兌換相關(START) */
int inCREDIT_PRINT_LOYALTY_REDEEM_Data(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_VOID_LOYALTY_REDEEM_Data(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
/* 優惠兌換相關(END) */

/* TMS排程使用 */
int inCREDIT_PRINT_Schedule_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);

/* HappyGo相關 */
int inCREDIT_PRINT_HG_Data_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_HG_Multi_Data_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_HG_Amount_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_HG_DetailReportMiddle_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_HG_DetailReportBottom_ByBuffer(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_HG_DetailReport_HGData_ByBuffer(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_HG_ReceiptEND_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
void vdCREDIT_PRINT_HG_GetTransactionType(TRANSACTION_OBJECT *pobTran, char *szPrintBuf1, char *szPrintBuf2);

/* 票證相關 */
/* 列印票證總額報表(START) */
typedef struct
{
        int (*inReportCheck)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);					/* 檢查檔案是否存在 */
        int (*inReportLogo)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);					/* LOGO */
        int (*inReportTop)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);					/* TID and MID and DATA */
        int (*inAmount)(TRANSACTION_OBJECT *, TICKET_ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* 全部金額總計 */
        int (*inAmountByCard)(TRANSACTION_OBJECT *, TICKET_ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* 卡別金額總計 */
        int (*inAmountByInstallment)(TRANSACTION_OBJECT *, TICKET_ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);	/* 分期金額總計 */
        int (*inAmountByRedemption)(TRANSACTION_OBJECT *, TICKET_ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);	/* 紅利金額總計 */
	int (*inAmountByOther)(TRANSACTION_OBJECT *, TICKET_ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* 其他總計 */
        int (*inReportEnd)(TRANSACTION_OBJECT *, TICKET_ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);												/* 結束 */
} TOTAL_REPORT_TABLE_BYBUFFER_ESVC;
/* 列印票證總額報表 (END) */

int inCREDIT_PRINT_Data_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Amount_ByBuffer_IPASS(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Amount_ByBuffer_ECC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Amount_ByBuffer_ICASH(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_ReceiptEND_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalReport_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_Logo_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Top_ESVC_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Top_ESVC_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Top_ESVC_SETTLE_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmount_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmount_ByBuffer_ESVC_Settle(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmountByCard_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmountByCard_ByBuffer_ESVC_Settle(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_End_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC* srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);

/* 列印明細報表(START) */
typedef struct
{
        int (*inReportCheck)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);				/* 檢查檔案是否存在 */
        int (*inReportLogo)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);				/* LOGO */
        int (*inReportTop)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);				/* TID and MID and DATA */
        int (*inTotalAmount)(TRANSACTION_OBJECT *, TICKET_ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);	/* 全部金額總計 */
        int (*inMiddle)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);					/* 明細中文 */
        int (*inBottom)(TRANSACTION_OBJECT *, int, unsigned char *, FONT_ATTRIB *, BufferHandle *);				/* 所有明細 */
        int (*inReportEnd)(TRANSACTION_OBJECT *, TICKET_ACCUM_TOTAL_REC* srAccumRec, unsigned char *, FONT_ATTRIB *, BufferHandle *);				/* 結束 */
} DETAIL_REPORT_TABLE_BYBUFFER_ESVC;
/* 列印明細報表 (END) */
int inCREDIT_PRINT_DetailReport_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_Check_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DetailReportMiddle_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DetailReportBottom_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);

int inCREDIT_PRINTBYBUFFER_Receipt_Test(void);
int inCREDIT_PRINT_Test(void);

int inCREDIT_PRINT_TotalAmount_ByBuffer_098_ESVC(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmount_ByBuffer_098_ESVC_Settle(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmountByCard_ByBuffer_098_ESVC(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmountByCard_ByBuffer_098_ESVC_Settle(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Top_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DetailReportMiddle_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_NCCC_DetailReportMiddle_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DCC_DetailReportMiddle_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_HG_DetailReportMiddle_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DetailReportBottom_ByBuffer_046(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DCC_DetailReportBottom_ByBuffer_046(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_HG_DetailReportBottom_ByBuffer_046(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_End_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Top_ESVC_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmount_ByBuffer_046_ESVC(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DetailReportMiddle_ByBuffer_046_ESVC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DetailReportBottom_ByBuffer_046_ESVC(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_End_ByBuffer_ESVC_046(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC* srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmount_ByBuffer_046(TRANSACTION_OBJECT *pobTran, void *srAccumRecOrg, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmountByCard_ByBuffer_046(TRANSACTION_OBJECT *pobTran, void *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmountByInstllment_ByBuffer_046(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmountByRedemption_ByBuffer_046(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmountByOther_046(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Total_HG_ByBuffer_046(TRANSACTION_OBJECT *pobTran, HG_ACCUM_TOTAL_REC *srHGAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmount_ByBuffer_046_ESVC_Settle(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Top_ESVC_SETTLE_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmountByCard_ByBuffer_046_ESVC(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmountByCard_ByBuffer_046_ESVC_Settle(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Tidmid_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Data_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Cup_Amount_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Amount_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Inst_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Redeem_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_LEGAL_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DCC_Data_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DCC_ReceiptEND_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DCC_Disclaimer_ByBuffer_046(int inFontSize, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_FISC_Data_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_FISC_Amount_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_HG_Data_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_HG_Amount_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_HG_Multi_Data_ByBuffer_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Data_ByBuffer_ESVC_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Amount_ByBuffer_IPASS_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Amount_ByBuffer_ECC_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Amount_ByBuffer_ICASH_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);

/* 列印明細報表(START) */
typedef struct
{
        int (*inReportCheck)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);				/* 檢查檔案是否存在 */
        int (*inReportLogo)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);				/* LOGO */
        int (*inReportTop)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);				/* TID and MID and DATA */
        int (*inTotalAmount)(TRANSACTION_OBJECT *, ACCUM_TOTAL_REC *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* 全部金額總計 */
        int (*inMiddle)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);					/* 明細中文 */
        int (*inBottom)(TRANSACTION_OBJECT *, int, unsigned char *, FONT_ATTRIB *, BufferHandle *);				/* 所有明細 */
        int (*inReportEnd)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);				/* 結束 */
} DETAIL_REPORT_DUTYFREE_REPRINT_TABLE_BYBUFFER;
/* 列印明細報表 (END) */
int inCREDIT_PRINT_Dutyfree_DetailReport_Title_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char uszReprint);
int inCREDIT_PRINT_Dutyfree_NCCC_DetailReport_ByBuffer(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_Dutyfree_Reprint_Check_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Dutyfree_Reprint_Logo_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Dutyfree_Reprint_Top_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Dutyfree_Reprint_NCCC_DetailReportMiddle_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Dutyfree_Reprint_DetailReportBottom_ByBuffer(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Dutyfree_Reprint_End_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Dutyfree_ReceiptEND_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Dutyfree_Reprint_Cavas_File_ByBuffer(unsigned char* uszFileName);
int inCREDIT_PRINT_Dutyfree_Reprint_Report_NCCC_First(TRANSACTION_OBJECT *pobTran, unsigned char* uszBuffer1, int* inYLen);
int inCREDIT_PRINT_Dutyfree_Reprint_Report_DCC_First(TRANSACTION_OBJECT *pobTran, unsigned char* uszBuffer1, int* inYLen);
int inCREDIT_PRINT_Dutyfree_Reprint_Report_ESVC_First(TRANSACTION_OBJECT *pobTran, unsigned char* uszBuffer1, int* inYLen);
int inCREDIT_PRINT_ReceiptEND_ByBuffer_ESVC_046(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_ReceiptEND_ByBuffer_027(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Reversal_Print(TRANSACTION_OBJECT *pobTran);

int inCREDIT_PRINT_DCC_GRAND_HOTEL_ReceiptEND_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DCC_Disclaimer_ByBuffer_061_VISA(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DCC_Disclaimer_ByBuffer_061_MASTERCARD(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);

int inCREDIT_PRINT_Logo_ByBuffer_041_043(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Data_ByBuffer_041_043(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Amount_ByBuffer_041_043(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_ReceiptEND_ByBuffer_041_043(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);

int inCREDIT_PRINT_Data_ByBuffer_005(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_HG_Data_ByBuffer_005(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);

int inCREDIT_PRINT_TotalAmount_ByBuffer_TAKA(TRANSACTION_OBJECT *pobTran, void *srAccumRecOrg, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalReport_ByBuffer_TAKA(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_DetailReport_ByBuffer_TAKA(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_TotalAmountByCard_ByBuffer_TAKA(TRANSACTION_OBJECT *pobTran, void *srAccumRecOrg, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);

int inCREDIT_PRINT_Data_ByBuffer_Small_S(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Amount_ByBuffer_Small_S(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_ReceiptEND_ByBuffer_Small_S(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DCC_Data_ByBuffer_Small_S(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_DCC_ReceiptEND_ByBuffer_Small_S(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_FISC_Data_ByBuffer_Small_S(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);

int inCREDIT_PRINT_Data_ByBuffer_041_043_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Amount_ByBuffer_041_043_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_ReceiptEND_ByBuffer_041_043_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Data_ByBuffer_041_043_Small_S(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Amount_ByBuffer_041_043_Small_S(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_ReceiptEND_ByBuffer_041_043_Small_S(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);

/* 信託相關 */
/* 列印信託總額報表(START) */
typedef struct
{
        int (*inReportCheck)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* 檢查檔案是否存在 */
        int (*inReportLogo)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* LOGO */
        int (*inReportTop)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* TID and MID and DATA */
        int (*inAmount)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);                  /* 全部金額總計 */
        int (*inAmountByCard)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);		/* 依機構金額總計 */
        int (*inReportEnd)(TRANSACTION_OBJECT *, unsigned char *, FONT_ATTRIB *, BufferHandle *);												/* 結束 */
} TOTAL_REPORT_TABLE_BYBUFFER_TRUST;

/* 列印信託總額報表 (END) */
int inCREDIT_PRINT_Data_ByBuffer_Trust(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_Amount_ByBuffer_Trust(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_ReceiptEND_ByBuffer_Trust(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalReport_ByBuffer_TRUST(TRANSACTION_OBJECT *pobTran);
int inCREDIT_PRINT_Top_ByBuffer_TRUST(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmount_ByBuffer_TRUST(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_TotalAmountByInstitutionCode_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inCREDIT_PRINT_End_ByBuffer_TRUST(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
