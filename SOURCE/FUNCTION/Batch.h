#define _BATCH_LAST_RECORD_		-1
#define _BATCH_INVALID_RECORD_		-11

typedef struct
{
	int	inTableID;
	char	szINV_Data[15];		/* *INV: %06d */
	char	szAmount[20];		/* 2.Print Amount */
	char	szTransType[40];	/* 3.Transaction Type */
	char	szPrintCardType[20];	/* 4.Print Card Type */
	char	szPrintPAN[20];		/* 5.Print Card Number */
	char	szDate[20];		/* Data */
	char	szTime[20];		/* Time */
	char	szAuthCode[20];		/* Approved No.*/
	char	szChekNo[20];		/* Check No.*/
	char	szReponseCode[20];	/* RESPONSE CODE */
	char	szStoreID[60];		/* Store ID */
	char	szReprintDate[20];	/* 列印時間(執行重印的時間) */
	char	szReprintTime[20];	/* 列印時間(執行重印的時間) */
}DUTYFREE_REPRINT_DATA;

typedef struct
{
	int	inTableID;
	char	szSettleReprintNCCCEnable[1 + 1];	/* 代表可以重印NCCC結帳明細 */
	char	szSettleReprintDCCEnable[1 + 1];	/* 代表可以重印DCC結帳明細 */
	char	szNCCCReprintTitleDateTime[30 + 1];
	char	szDCCReprintTitleDateTime[30 + 1];
	char	szNCCCReprintBatchNum[6 + 1];
	char	szDCCReprintBatchNum[6 + 1];
	char	szTMSUpdateSuccessNum[3 + 1];
	char	szTMSUpdateFailNum[3 + 1];
}DUTYFREE_REPRINT_TITLE;

typedef struct
{
	int	inTableID;
}DUTYFREE_REPRINT_RETURMTMS;

#define	_TABLE_NAME_REPRINT_NCCC_	_HOST_NAME_CREDIT_NCCC_
#define	_TABLE_NAME_REPRINT_DCC_	_HOST_NAME_DCC_
#define	_TABLE_NAME_REPRINT_TITLE_	"reprint_title"

int inBATCH_FuncUpdateTxnRecord(TRANSACTION_OBJECT *pobTran);
int inBATCH_FuncUserChoice(TRANSACTION_OBJECT *pobTran);


int inBATCH_StoreBatchKeyFile(TRANSACTION_OBJECT *pobTran, TRANS_BATCH_KEY *srBKeyRec);
int inBATCH_SearchRecord(TRANSACTION_OBJECT *pobTran, TRANS_BATCH_KEY *srBatchKeyRec, unsigned long ulBAK_Handle, int inRecTotalCnt);
int inBATCH_CheckReport(TRANSACTION_OBJECT *pobTran);
int inBATCH_GetTotalCountFromBakFile(TRANSACTION_OBJECT *pobTran);
int inBATCH_OpenBatchKeyFile(TRANSACTION_OBJECT *pobTran, unsigned long ulBAK_Handle);
int inBATCH_OpenBatchRecFile(TRANSACTION_OBJECT *pobTran, unsigned long ulBAT_Handle);
int inBATCH_ReviewReport_Detail(TRANSACTION_OBJECT *pobTran);
int inBATCH_GetDetailRecords(TRANSACTION_OBJECT *pobTran, int inStartCnt);
int inBATCH_GetInvoiceNumber(TRANSACTION_OBJECT *pobTran);
int inBATCH_GetTransRecord(TRANSACTION_OBJECT *pobTran);

int inBATCH_GetAdviceDetailRecord(TRANSACTION_OBJECT *pobTran, int inADVCnt);

int inBATCH_ESC_Save_Advice(TRANSACTION_OBJECT *pobTran);
int inBATCH_Update_Sign_Status(TRANSACTION_OBJECT *pobTran);

int inADVICE_SaveAppend(TRANSACTION_OBJECT *pobTran, long lnInvNum);
int inADVICE_SaveTop(TRANSACTION_OBJECT *pobTran, long lnInvNum);
int inADVICE_DeleteRecordFlow(TRANSACTION_OBJECT *pobTran, long lnInvNum);
int inADVICE_SearchRecord_Index(TRANSACTION_OBJECT *pobTran, long lnInvNum, int *inADVIndex);
int inADVICE_DeleteRecored(TRANSACTION_OBJECT *pobTran, int inAdvIndex);
int inADVICE_DeleteAll(TRANSACTION_OBJECT *pobTran);
int inADVICE_Update(TRANSACTION_OBJECT *pobTran);
int inADVICE_GetTotalCount(TRANSACTION_OBJECT *pobTran);
int inADVICE_GetInvoiceNum(TRANSACTION_OBJECT *pobTran, char *szGetInvoiceNum);
int inADVICE_ESC_SaveAppend(TRANSACTION_OBJECT *pobTran, long lnInvNum);
int inADVICE_ESC_GetTotalCount(TRANSACTION_OBJECT *pobTran);
int inADVICE_ESC_DeleteRecord(TRANSACTION_OBJECT *pobTran, long lnInvNum);
int inADVICE_ESC_SearchRecord_Index(TRANSACTION_OBJECT *pobTran, long lnInvNum, int *inADVIndex);
int inADVICE_ESC_DeleteRecored(TRANSACTION_OBJECT *pobTran, int inAdvIndex);
int inBATCH_Advice_ESC_HandleReadOnly(TRANSACTION_OBJECT* pobTran);
int inBATCH_GlobalAdvice_ESC_HandleClose(void);
int inBATCH_GetAdvice_ESC_DetailRecord(TRANSACTION_OBJECT *pobTran, int inADVCnt);

int inBATCH_FuncUpdateTxnRecord_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_CheckReport_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_GetTotalCountFromBakFile_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_GetTotalCountFromBakFile_By_Sqlite_ESVC(TRANSACTION_OBJECT *pobTran);
int inBATCH_ReviewReport_Detail_Flow_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_ReviewReport_Detail_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_ReviewReport_Detail_Txno_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_ReviewReport_Detail_NEWUI_Flow_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_ReviewReport_Detail_NEWUI_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_ReviewReport_Detail_Txno_NEWUI_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_GetDetailRecords_By_Sqlite(TRANSACTION_OBJECT *pobTran, int inStartCnt);
int inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(TRANSACTION_OBJECT *pobTran);
int inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(TRANSACTION_OBJECT *pobTran, int inStartCnt);
int inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(TRANSACTION_OBJECT *pobTran);
int inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_START(TRANSACTION_OBJECT *pobTran);
int inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_Read(TRANSACTION_OBJECT *pobTran, int inStartCnt);
int inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_END(TRANSACTION_OBJECT *pobTran);
int inBATCH_FuncUserChoice_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_GetTransRecord_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_GetTransRecord_By_Sqlite_ESVC(TRANSACTION_OBJECT *pobTran);
int inBATCH_GlobalBatchHandleReadOnly_By_Sqlite(void);
int inBATCH_GlobalBatchHandleClose_By_Sqlite(void);
int inBATCH_AdviceHandleReadOnly_By_Sqlite(TRANSACTION_OBJECT* pobTran);
int inBATCH_GlobalAdviceHandleClose_By_Sqlite(void);
int inBATCH_GetAdviceDetailRecord_By_Sqlite(TRANSACTION_OBJECT *pobTran, int inADVCnt);
int inBATCH_GetAdvice_ESC_DetailRecord_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_ESC_Save_Advice_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_Update_Sign_Status_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_Update_MPAS_Reprint_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_Update_CLS_SettleBit_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_Update_ESC_Uploaded_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_GetTotalCount_BatchUpload_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_FuncUpdateTxnRecord_Ticket_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_Create_BatchTable_Flow(TRANSACTION_OBJECT *pobTran, int inTableType);
int inBATCH_Insert_All_Flow(TRANSACTION_OBJECT *pobTran, int inTableType);
int inBATCH_Get_ESC_Upload_Count_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inTxnType, unsigned char uszPaperBit, int *inTxnTotalCnt, long *lnTxnTotalAmt);
int inBATCH_GetAdvice_ESVC_DetailRecord_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_Find_Last_Txn_Inv_SQLite(TRANSACTION_OBJECT *pobTran, char* szInvNum);
int inBATCH_Reprint_Data_Prepare(TRANSACTION_OBJECT *pobTran, DUTYFREE_REPRINT_DATA* srReprintData);
int inBATCH_Reprint_Data_Save(TRANSACTION_OBJECT *pobTran, DUTYFREE_REPRINT_DATA* srReprintData);
int inBatch_Table_Link_Reprint_Data(DUTYFREE_REPRINT_DATA* srReprintData, SQLITE_ALL_TABLE *srAll, int inLinkState);
int inBatch_Table_Link_Reprint_Title(DUTYFREE_REPRINT_TITLE* srReprintTitle, SQLITE_ALL_TABLE *srAll, int inLinkState);
int inBATCH_Get_ATS_Batch_Still_Not_Upload(TRANSACTION_OBJECT *pobTran);
int inBATCH_Update_ESC_Upload_Status_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_Update_NoSignature_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_FuncUpdateTxnRecord_Trust_By_Sqlite(TRANSACTION_OBJECT *pobTran);
int inBATCH_GetTransRecord_Trust_By_Sqlite(TRANSACTION_OBJECT *pobTran);


int inBATCH_ESC_Save_Advice_Flow(TRANSACTION_OBJECT *pobTran);
int inADVICE_ESC_GetTotalCount_Flow(TRANSACTION_OBJECT *pobTran);
int inADVICE_ESC_DeleteRecordFlow(TRANSACTION_OBJECT *pobTran, long lnInvNum);
int inBATCH_Advice_ESC_HandleReadOnly_Flow(TRANSACTION_OBJECT* pobTran);
int inBATCH_GlobalAdvice_ESC_HandleClose_Flow(void);
int inBATCH_GetAdvice_ESC_DetailRecord_Flow(TRANSACTION_OBJECT *pobTran, int inADVCnt);

int inADVICE_ESC_SaveAppend_By_Sqlite(TRANSACTION_OBJECT *pobTran, long lnInvNum);
int inADVICE_ESC_GetTotalCount_By_Sqlite(TRANSACTION_OBJECT *pobTran);


int inBATCH_Presssure_Test(void);
