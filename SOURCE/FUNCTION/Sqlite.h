/* 
 * File:   Sqlite.h
 * Author: user
 *
 * Created on 2016年3月10日, 上午 10:07
 */
/* Table 名稱 */
#define _BATCH_TABLE_SUFFIX_			""
#define _EMV_TABLE_SUFFIX_			"_EMV"
#define _BATCH_TABLE_ADVICE_SUFFIX_		"_ADV"
#define _BATCH_TABLE_ESC_AGAIN_SUFFIX_		"_AGAIN"	/* 重送區table */
#define _BATCH_TABLE_ESC_AGAIN_EMV_SUFFIX_	"_AGAINEMV"	/* 重送區 EMV table */
#define _BATCH_TABLE_ESC_FAIL_SUFFIX_		"_FAIL"		/* 失敗區table */
#define _BATCH_TABLE_ESC_FAIL_EMV_SUFFIX_	"_FAILEMV"	/* 失敗區 EMV table */
#define _BATCH_TABLE_ESC_TEMP_SUFFIX_		"_ESCTEMP"	/* 斷電保險機制 */
#define _BATCH_TABLE_ESC_TEMP_EMV_SUFFIX_	"_ESCTEMPEMV"	/* 斷電保險機制 EMV */
#define _BATCH_TABLE_ESC_ADVICE_SUFFIX_		"_ESCADV"	/* ESC ADVICE */
#define _BATCH_TABLE_ESC_ADVICE_EMV_SUFFIX_	"_ESCADVEMV"	/* ESC ADVICE EMV*/
#define _ACCUM_TABLE_SUFFIX_			"_ACC"
/* ESC不送F_55 */

/* Link State */
#define _LS_INSERT_	0
#define _LS_READ_	1
#define _LS_UPDATE_	2

/* inSqlite_Get_Batch_ByCnt_Enormous_Flow 使用 */
#define _BYCNT_ENORMMOUS_SEARCH_	0
#define _BYCNT_ENORMMOUS_READ_		1
#define _BYCNT_ENORMMOUS_FREE_		2

#define _SQLITE_DEFAULT_FLAGS_		SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE

typedef enum
{
	_TN_NULL_ = 0x00,
	_TN_BATCH_TABLE_,
	_TN_EMV_TABLE_,
	_TN_BATCH_TABLE_ESC_AGAIN_,
	_TN_BATCH_TABLE_ESC_AGAIN_EMV_,
	_TN_BATCH_TABLE_ESC_FAIL_,
	_TN_BATCH_TABLE_ESC_FAIL_EMV_,
	_TN_BATCH_TABLE_ESC_TEMP_,	/* 暫存用，用以判斷是否有執行上傳流程，導致有未統計到的 */
	_TN_BATCH_TABLE_ESC_TEMP_EMV_,
	_TN_BATCH_TABLE_ESC_ADVICE_,
	_TN_BATCH_TABLE_ESC_ADVICE_EMV_,
	_TN_BATCH_TABLE_TICKET_,
	_TN_BATCH_TABLE_TICKET_ADVICE_,
	_TN_BATCH_TABLE_TRUST_,
} TABLE_NAME;

#define	_DATA_BASE_NAME_NEXSYS_TRANSACTION_			"nexsys.db"
#define	_DATA_BASE_NAME_NCCC_DUTY_FREE_REPRINT_			"DutyFreeReprint.db"

/* Initial*/
int inSqlite_Initial(void);
int inSqlite_Initial_Setting(void);

/* Create */
int inSqlite_Create_BatchTable_Flow(TRANSACTION_OBJECT *pobTran, int inTableType);
int inSqlite_Create_Table(char* szDBName, char* szTableName, SQLITE_TAG_TABLE* pobSQLTag);
int inSqlite_Create_BatchTable_EMV(char* szDBName, char* szTableName, char* szTableName2, SQLITE_TAG_TABLE* pobSQLTag);

/* Delete */
int inSqlite_Drop_Table_Flow(TRANSACTION_OBJECT *pobTran, int inTableType);
int inSqlite_Drop_Table(char* szDBName, char* szTableName);

/* Link */
int inSqlite_Table_Link_BRec(TRANSACTION_OBJECT *pobTran, SQLITE_ALL_TABLE *srAll, int inLinkState);
int inSqlite_Table_Link_EMVRec(TRANSACTION_OBJECT* pobTran, SQLITE_ALL_TABLE* srAll, int inLinkState);
int inSqlite_Insert_Or_Replace_ByRecordID_All(char* szDBName /* = 0 */, char* szTableName, int inRecordID /* = 0 */, SQLITE_ALL_TABLE* srAll /* = 0 */);

/* Get Tag Value */
int inSqlite_Get_TagValue_ByInvoiceNumber_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inInvoiceNumber, char *szTagName, char *szTagValue);
int inSqlite_Get_TagValue_ByInvoiceNumber(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber, char *szTagName, char *szTagValue);

/* Table Count */
int inSqlite_Get_Table_Count_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int *inTableCount);
int inSqlite_Get_Table_Count(char* szDBPath, char *szTableName, int *inTableCount);

/* Get Batch */
int inBATCH_Get_Batch_ByInvNum_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inInvoiceNumber);
int inSqlite_Get_Batch_ByInvNum(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber);
int inSqlite_Get_EMV_ByInvNum(TRANSACTION_OBJECT *pobTran, char* szTableName);
int inSqlite_Get_Table_ByRecordID_All(char* szDBName, char* szTableName, int inRecordID, SQLITE_ALL_TABLE* srAll);


int inSqlite_Update_ByInvoiceNumber_All(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber);
int inSqlite_Update_ByRecordID_All(char* szDBName, char* szTableName, int inRecordID, SQLITE_ALL_TABLE* srAll);

/* 取最早insert的紀錄 */
int inSqlite_ESC_Get_BRec_Top_Flow(TRANSACTION_OBJECT *pobTran, int inTableType);
int inSqlite_ESC_Get_BRec_Top(TRANSACTION_OBJECT *pobTran, char* szTableName);
int inSqlite_ESC_Get_EMVRec_Top(TRANSACTION_OBJECT *pobTran, char* szTableName);

/* 刪除最早insert的紀錄 */
int inSqlite_ESC_Delete_Record_Flow(TRANSACTION_OBJECT *pobTran, int inTableType);
int inSqlite_ESC_Delete_Record(TRANSACTION_OBJECT *pobTran, char* szTableName);

/* 取最大的Primary Key值 */
int inSqlite_Get_Max_TableID_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, char *szTagValue);
int inSqlite_Get_Max_TableID(char* szDBPath, char* szTableName, char *szTagValue);

/* 更改原先紀錄的狀態 */
int inSqlite_Update_ByInvNum_TranState_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inInvoiceNumber);
int inSqlite_Update_ByInvNum_TranState(TRANSACTION_OBJECT *pobTran, char* szTableType, int inInvoiceNumber);
int inSqlite_Update_ByInvNum_TranState_Ticket(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber);
int inSqlite_Update_ByInvNum_CLS_SettleBit_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inInvoiceNumber);
int inSqlite_Update_ByInvNum_CLS_SettleBit(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber);

int inSqlite_Get_Batch_ByCnt_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inRecCnt);
int inSqlite_Get_Batch_ByCnt(TRANSACTION_OBJECT *pobTran, char* szTableType, int inRecCnt);
int inSqlite_Get_EMV_ByCnt(TRANSACTION_OBJECT *pobTran, char* szTableName);
int inSqlite_Get_Batch_Count_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int *inTableCount);
int inSqlite_Get_Batch_Count(TRANSACTION_OBJECT *pobTran, char *szTableName, int *inTableCount);
int inSqlite_Get_Batch_Upload_Count_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int *inTableCount);
int inSqlite_Get_Batch_Upload_Count(TRANSACTION_OBJECT *pobTran, char *szTableName, int *inTableCount);

/* Get大量資料用 */
int inBATCH_Get_Batch_ByCnt_Enormous_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inRecCnt, int inState);

/* 確認表是否存在 */
int inSqlite_Check_Table_Exist_Flow(TRANSACTION_OBJECT *pobTran, int inTableType);
int inSqlite_Check_Table_Exist(char* szDBPath, char *szTableName);

/* Vacumm 指令 */
int inSqlite_Vacuum_Flow(TRANSACTION_OBJECT *pobTran, int inTableType);
int inSqlite_Vacuum(TRANSACTION_OBJECT *pobTran, char *szTableName1);

int inSqlite_Check_Escape_Cnt(char* szString);
int inSqlite_Process_Escape(char* szOldString, char* szNewString);

/* Test Function */
int inSqlite_Batch_To_DB_Test(void);
int inSqlite_Get_Table_Count_Test(void);
int inSqlite_Vacuum_Flow_Test(void);

/* 算筆數使用 */
int inSqlite_Get_ESC_Sale_Upload_Count(TRANSACTION_OBJECT *pobTran, char *szTableName, int inTxnType, unsigned char uszPaperBit, int *inTxnTotalCnt, long *lnTxnTotalAmt);

int inSqlite_Get_Data_By_External_SQL(char* szDBName, SQLITE_ALL_TABLE* srAll, char* szSQLStatement);
int inSqlite_Get_Binding_Value(sqlite3_stmt** srSQLStat, SQLITE_ALL_TABLE* srAll);
int inSqlite_DB_Open_Or_Create(char* szDBPath, sqlite3** srDBConnection, int inFlags, char* szVfs);
int inSqlite_Copy_Table_Data(char* szDBPath, char* szOldTableName, char* szNewTableName);
int inSqlite_Rename_Table(char* szDBPath, char* szOldTableName, char* szNewTableName);
int inSqlite_Table_Relist_SQLite(char *szDBPath, char *szTableName, int inOldIndex, int inNewIndex);
int inSqlite_Delete_Table_Data_Flow(TRANSACTION_OBJECT *pobTran, int inTableType);
int inSqlite_Delete_Table_Data(char* szDBPath, char* szTableName);
int inSqlite_Update_ByInvNum_SignState(TRANSACTION_OBJECT * pobTran, char* szDBPath, char* szTableName, int inInvoiceNumber);
int inSqlite_DB_Close(sqlite3** srDBConnection);
int inSqlite_SQL_Finalize(sqlite3_stmt **srSQLStat);
int inSqlite_Insert_Record(char* szDBPath, char* szTableName, SQLITE_ALL_TABLE* srAll);
int inSqlite_Table_Link(SQLITE_ALL_TABLE *srAll, int inLinkState, SQLITE_LINK_TABLE* srLink);
int inSqlite_Get_Data_Enormous_Search(char* szDBPath, char* szQuerySql);
int inSqlite_Get_Data_Enormous_Get(SQLITE_ALL_TABLE *srAll, int inRecCnt);
int inSqlite_Get_Data_Enormous_Free(void);
int inSqlite_Delete_Record_By_External_SQL(char* szDBName, char* szTableName, char* szSQLStatement);
int inSqlite_Calculate_Insert_SQLLength(SQLITE_SQL_CALCULATE_TABLE* srSQLCal, SQLITE_ALL_TABLE* srAll, int* inSqlLength);
int inSqlite_Calculate_Update_SQLLength(SQLITE_SQL_CALCULATE_TABLE* srSQLCal, SQLITE_ALL_TABLE* srAll, int* inSqlLength);
int inSqlite_Gernerate_InsertSQL(SQLITE_SQL_CALCULATE_TABLE* srSQLCal, SQLITE_ALL_TABLE* srAll, char* szSQL);
int inSqlite_Gernerate_UpdateSQL(SQLITE_SQL_CALCULATE_TABLE* srSQLCal, SQLITE_ALL_TABLE* srAll, char* szSQL);
int inSqlite_Binding_Values(SQLITE_ALL_TABLE* srAll, sqlite3_stmt** srSQLStat);
int inSqlite_Fsync_TranDB(void);
int inSqlite_Table_Link_Addition_Int(SQLITE_ALL_TABLE *srAll, char *szTag, void *vdInt);
int inSqlite_Table_Link_Addition_Int64(SQLITE_ALL_TABLE *srAll, char *szTag, void *vdInt);
int inSqlite_Reset_Find_State(SQLITE_ALL_TABLE* srAll);
int inSqlite_CheckPrepareCommStatus(int inStuts);