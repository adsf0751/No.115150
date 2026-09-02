#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sqlite3.h>
#include <unistd.h>
#include <errno.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/AllStruct.h"
#include "../EVENT/MenuMsg.h"
#include "../FUNCTION/File.h"
#include "../FUNCTION/Function.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "../../NCCC/NCCCesc.h"
#include "Sqlite.h"
#include "Batch.h"
#include "Utility.h"
#include "EDC_Para_Table_Func.h"
#include "HDPT.h"

extern int	ginDebug;		/* Debug使用 extern */
SQLITE_RESULT	gsrResult = {};		/* For特殊情況使用，若一次要Get多筆Record，只Get一次，待使用完後在Call sqlite3_free_table() */
/* 以下這兩個變數用在原先sqlite3_get_table的時機 */
sqlite3		*gsrDBConnection;	/* 建立到資料庫的connection */
sqlite3_stmt	*gsrSQLStat;
char		gszTranDBPath[100 + 1] = {_FS_DATA_PATH_ _DATA_BASE_NAME_NEXSYS_TRANSACTION_};
char		gszParamDBPath[100 + 1] = {_FS_DATA_PATH_ _DATA_BASE_NAME_NEXSYS_PARAMETER_};
char		gszReprintDBPath[100 + 1] = {_FS_DATA_PATH_ _DATA_BASE_NAME_NCCC_DUTY_FREE_REPRINT_};
int		ginEnormousNowCnt = 0;
unsigned char	guszEnormousNoNeedResetBit = VS_FALSE;	/* 避免每次都從頭找起 */

/*
	筆記:
 
	(1) var因為額外儲存地址，讀取時會先去抓資料，會比非var來的略慢
	效能：(nvarchar, varchar) < (nchar, char)
	(2) n因為1字2Byte，所以正常會花費2倍儲存空間
	儲存體：(nvarchar, nchar) >> (varchar, char)


	確認一定長度，且只會有英數字：char
	確認一定長度，且可能會用非英數以外的字元：nchar
	長度可變動，且只會有英數字：varchar
	長度可變動，且可能會用非英數以外的字元：nvarchar
 
 *	SQLite 的primary key會自動增長，若已有三筆Record，Delete 第二筆再insert新資料，則primaery key從4繼續往上加
 *	目前儲存型別皆改成BLOB，目的是要儲存0x00部份
 *	目前此份code有兩種查詢方式的程式碼並存，一種是sqlite_exec，第二種是sqlite_prepare and step 因為C語言的特性，建議盡量設計成prepare的形式
 */

/*	關於sqlite3_column_xxx(sqlite3_stmt*, int iCol);
 *	如果對該列使用了不同於該列本身類型適合的數據讀取方式，得到的數值將是轉換過得結果(所以不知道類型一律用Blob就對了)。*/

/* BLOB在SQL語句的表示法，0x01 = X'01', 0x01 0x02 = X'0102' (X大小寫都可以) */

extern SQLITE_TAG_TABLE TABLE_BATCH_TAG[];
extern SQLITE_TAG_TABLE TABLE_EMV_BATCH_TAG[];

/*Note: var1字一byte nvar1字兩byte，空白可隨意插入SQL語句，唯Column（Tag）不能重複(找錯找好久)，*/
/*------------------------------------------------------------------------------------------
 * 語句形式
*		
* static char	*szCreateTable = "
*/

/* 如果建立時不設檔案名，則設立暫時使用的資料庫(close 連結即消失)
----------------------------------下面三個選一個---------------------------------------------------------------------------
 * SQLITE_OPEN_READONLY				唯讀
 * SQLITE_OPEN_READWRITE			讀寫
 * _SQLITE_DEFAULT_FLAGS_	檔案不存在就開檔	
------------------------------------如果是單ap應該是不用考慮以下問題--------------------------------------------------------
 * SQLITE_OPEN_NOMUTEX		多線程方式，可以同時一個連結寫，多個連結讀 有可能會失敗   (啟用bCoreMutex禁用bFullMutex的鎖) 
 * SQLITE_OPEN_FULLMUTEX	串行化方式，一個連結關閉，另一個連結才能連結，default為此 (啟用bCoreMutex和bFullMutex的鎖）
 * SQLITE_OPEN_SHAREDCACHE
 */

/* 新的sqlite用法盡量使用inSqlite_Get_Data_By_External_SQL這個形式(在function外組語法和link)  */
/* 使用資料庫流程，
 * 1.將要用的tag放在SQLITE_TAG_TABLE結構
 * 2.inSqlite_Create_Table
 * 3.寫入時要寫一個link用的function(參照inSqlite_Table_Link_BRec)
 * 4.寫入用inSqlite_Insert_Record或inSqlite_Insert_Or_Replace_ByRecordID_All
 * 5.讀取用inSqlite_Get_Data_By_External_SQL或inSqlite_Get_Table_ByRecordID_All
 * 
 * p.s.:每次讀取和寫入前都要重新link，讀取前是為了把uszIsFind重置，寫入前是為了取得寫入Tag的資料長度,
 * link前srAll要清空，否則會多出多餘的Tag
 */

/*
Function        :inSqlite_Initial
Date&Time       :2017/3/14 上午 10:50
Describe        :
*/
int inSqlite_Initial()
{
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inSqlite_Initial() START !");
	}
	
	/* 設定*/
	inSqlite_Initial_Setting();
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inSqlite_Initial() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Initial_Setting
Date&Time       :2017/3/14 上午 10:33
Describe        :有一些設定要改
*/
int inSqlite_Initial_Setting()
{
	int		inRetVal;
	int		inSwitch;
	int		inState;
	char		szDebugMsg[100 + 1];
	sqlite3		*srDBConnection;		/* 建立到資料庫的connection */
	
	/* 開啟DataBase檔 */
	inRetVal = inSqlite_DB_Open_Or_Create(gszTranDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	if (inRetVal != VS_SUCCESS)
	{	
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* SQLlite 外鍵預設為關閉，改成開啟(不然用外鍵就沒意義了) (1代表開啟 0代表關閉)*/
	inSwitch = 1;
	inRetVal = sqlite3_db_config(srDBConnection, SQLITE_DBCONFIG_ENABLE_FKEY, inSwitch, &inState);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "SQLITE_DBCONFIG_ENABLE_FKEY Switch :%d ERROR Num%d", inSwitch, inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
                vdUtility_SYSFIN_LogMessage(AT, "SQLITE_DBCONFIG_ENABLE_FKEY Switch :%d ERROR Num%d", inSwitch, inRetVal);
                
		inSqlite_DB_Close(&srDBConnection);
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "SQLITE_DBCONFIG_ENABLE_FKEY Switch :%d OK, State: %d", inSwitch, inState);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	inRetVal = inSqlite_DB_Close(&srDBConnection);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "sqlite3_close Fail ErrNum :%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_CreateBatchTable
Date&Time       :2016/3/28 上午 10:47
Describe        :Open Database檔 建立table 這function只負責建batch的table
*/
int inSqlite_Create_Table(char* szDBName, char* szTableName, SQLITE_TAG_TABLE* pobSQLTag)
{
	int		i;
	int		inRetVal;
	int		inSqlLength = 0;
	char		szSqlPrefix[100 + 2];		/* CREATE TABLE	szTableName( */
	char		szSqlSuffix[10 + 2];		/* ); */
	char		szDebugMsg[100 + 1];
	char		*szCreateSql;			/* 因為會很長且隨table浮動，所以用pointer */
	char		*szErrorMessage = NULL;
	sqlite3		*srDBConnection;		/* 建立到資料庫的connection */
		
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Create_BatchTable()_START");
        }
	
	/* 開啟DataBase檔 */
	inRetVal = inSqlite_DB_Open_Or_Create(szDBName, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}

	/* 塞入SQL語句 */
	/* 為了使table name可變動，所以拉出來組 */
	/* 加入了if not exists字串，若已建立不會重複建立 */
	/* 前綴 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "CREATE TABLE if not exists %s(", szTableName);
	/*  "CREATE TABLE	szTableName("的長度 */
	inSqlLength += strlen(szSqlPrefix);
	
	
	/* 計算要分配的記憶體長度 */
	for (i = 0;; i ++)
	{
		/* 碰到Table底部 */
		if (strlen((char*)&pobSQLTag[i].szTag) == 0)
		{
			break;
		}
		
		/* 第一行前面不加逗號，其他都要 */
		if (i > 0)
		{
			inSqlLength += strlen(",");
		}
		
		/* Tag Name */
		inSqlLength += strlen((char*)&pobSQLTag[i].szTag);
		/* Tag 型別 */
		if (strlen((char*)&pobSQLTag[i].szType) > 0)
		{
			inSqlLength += strlen(" ");
			inSqlLength += strlen((char*)&pobSQLTag[i].szType);
		}
		/* Tag 屬性1 */
		if (strlen((char*)&pobSQLTag[i].szAttribute1) > 0)
		{
			inSqlLength += strlen(" ");
			inSqlLength += strlen((char*)&pobSQLTag[i].szAttribute1);
		}
		/* Tag 屬性2 */
		if (strlen((char*)&pobSQLTag[i].szAttribute2) > 0)
		{
			inSqlLength += strlen(" ");
			inSqlLength += strlen((char*)&pobSQLTag[i].szAttribute2);
		}
	}
	
	/* 後綴 */
	memset(szSqlSuffix, 0x00, sizeof(szSqlSuffix));
	sprintf(szSqlSuffix, ");");
	inSqlLength += strlen(szSqlSuffix);
        
	/* inSqlLength: */
	if (ginDebug == VS_TRUE) 
        {
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inSqlLength: %d", inSqlLength);
		inLogPrintf(AT, szDebugMsg);
	}
	/* 配置記憶體(一定要+1，超級重要，不然會overflow) */
	szCreateSql = malloc(inSqlLength + 1);
	memset(szCreateSql, 0x00, inSqlLength);
	
	/* 先丟前綴Table Name */
	strcat(szCreateSql, szSqlPrefix);
	
	/* table要哪些tag */
	for (i = 0 ;; i ++)
        {
		/* 碰到Table底部 */
		if (strlen((char*)&pobSQLTag[i].szTag) == 0)
		{
			break;
		}
		
		/* 第一行前面不加逗號，其他都要 */
		if (i > 0)
		{
			strcat(szCreateSql , ",");
		}
		
		/* Tag Name */
		strcat(szCreateSql, (char*)&pobSQLTag[i].szTag);
		/* Tag 型別 */
		if (strlen((char*)&pobSQLTag[i].szType) > 0)
		{
			strcat(szCreateSql, " ");
			strcat(szCreateSql, (char*)&pobSQLTag[i].szType);
		}
		/* Tag 屬性1 */
		if (strlen((char*)&pobSQLTag[i].szAttribute1) > 0)
		{
			strcat(szCreateSql, " ");
			strcat(szCreateSql, (char*)&pobSQLTag[i].szAttribute1);
		}
		/* Tag 屬性2 */
		if (strlen((char*)&pobSQLTag[i].szAttribute2) > 0)
		{
			strcat(szCreateSql, " ");
			strcat(szCreateSql, (char*)&pobSQLTag[i].szAttribute2);
		}
        }
	
	/* 後綴 */
	strcat(szCreateSql, szSqlSuffix);
	
	/* 建立 Table */
	inRetVal = sqlite3_exec(srDBConnection, szCreateSql, 0, 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Create Table ERROR Num:%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason:%s", szErrorMessage);
			inLogPrintf(AT, szDebugMsg);
		}
		
		free(szCreateSql);
		if (szErrorMessage != NULL)
		{
			sqlite3_free(szErrorMessage);
		}
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Create Table OK");
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 釋放記憶體 */
		free(szCreateSql);
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}

	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_CreateBatchTable()_END");
                inLogPrintf(AT, "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Create_BatchTable_EMV
Date&Time       :2017/3/14 下午 4:48
Describe        :Open Database檔 建立table 這function只負責建EMV的table
 *		szTableName2:用來設定外鍵
*/
int inSqlite_Create_BatchTable_EMV(char* szDBName, char* szTableName1, char* szTableName2, SQLITE_TAG_TABLE* pobSQLTag)
{
	int		i;
	int		inRetVal;
	int		inSqlLength = 0;
	char		szSqlPrefix[100 + 2] = {0};		/* CREATE TABLE	szTableName( */
	char		szSqlSuffix[10 + 2] = {0};		/* ); */
	char		szAdditionalData[300 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		*szCreateSql;			/* 因為會很長且隨table浮動，所以用pointer */
	char		*szErrorMessage = NULL;
	sqlite3		*srDBConnection;		/* 建立到資料庫的connection */
		
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Create_BatchTable_EMV()_START");
        }
	
	/* 開啟DataBase檔 */
	inRetVal = inSqlite_DB_Open_Or_Create(szDBName, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}

	/* 塞入SQL語句 */
	/* 為了使table name可變動，所以拉出來組 */
	/* 加入了if not exists字串，若已建立不會重複建立 */
	/* 前綴 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "CREATE TABLE if not exists %s(", szTableName1);
	/*  "CREATE TABLE	szTableName("的長度 */
	inSqlLength += strlen(szSqlPrefix);
	
	
	/* 計算要分配的記憶體長度 */
	for (i = 0;; i ++)
	{
		/* 碰到Table底部 */
		if (strlen((char*)&pobSQLTag[i].szTag) == 0)
		{
			break;
		}
		
		/* 第一行前面不加逗號，其他都要 */
		if (i > 0)
		{
			inSqlLength += strlen(",");
		}
		
		/* Tag Name */
		inSqlLength += strlen((char*)&pobSQLTag[i].szTag);
		/* Tag 型別 */
		if (strlen((char*)&pobSQLTag[i].szType) > 0)
		{
			inSqlLength += strlen(" ");
			inSqlLength += strlen((char*)&pobSQLTag[i].szType);
		}
		/* Tag 屬性1 */
		if (strlen((char*)&pobSQLTag[i].szAttribute1) > 0)
		{
			inSqlLength += strlen(" ");
			inSqlLength += strlen((char*)&pobSQLTag[i].szAttribute1);
		}
		/* Tag 屬性2 */
		if (strlen((char*)&pobSQLTag[i].szAttribute2) > 0)
		{
			inSqlLength += strlen(" ");
			inSqlLength += strlen((char*)&pobSQLTag[i].szAttribute2);
		}
	}
	/* 外鍵 ",FOREIGN KEY(inBatchTableID) REFERENCES", szTableName2(inTableID)" */
	memset(szAdditionalData, 0x00, sizeof(szAdditionalData));
	sprintf(szAdditionalData, ",FOREIGN KEY(inBatchTableID) REFERENCES %s(inTableID) ON DELETE CASCADE ON UPDATE CASCADE", szTableName2);
	inSqlLength += strlen(szAdditionalData);
	
	/* 後綴 */
	memset(szSqlSuffix, 0x00, sizeof(szSqlSuffix));
	sprintf(szSqlSuffix, ");");
	inSqlLength += strlen(szSqlSuffix);
        
	/* inSqlLength: */
	if (ginDebug == VS_TRUE) 
        {
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inSqlLength: %d", inSqlLength);
		inLogPrintf(AT, szDebugMsg);
	}
	/* 配置記憶體(一定要+1，超級重要，不然會overflow) */
	szCreateSql = malloc(inSqlLength + 1);
	memset(szCreateSql, 0x00, inSqlLength);
	
	/* 先丟前綴Table Name */
	strcat(szCreateSql, szSqlPrefix);
	
	/* table要哪些tag */
	for (i = 0 ;; i ++)
        {
		/* 碰到Table底部 */
		if (strlen((char*)&pobSQLTag[i].szTag) == 0)
		{
			break;
		}
		
		/* 第一行前面不加逗號，其他都要 */
		if (i > 0)
		{
			strcat(szCreateSql , ",");
		}
		
		/* Tag Name */
		strcat(szCreateSql, (char*)&pobSQLTag[i].szTag);
		/* Tag 型別 */
		if (strlen((char*)&pobSQLTag[i].szType) > 0)
		{
			strcat(szCreateSql, " ");
			strcat(szCreateSql, (char*)&pobSQLTag[i].szType);
		}
		/* Tag 屬性1 */
		if (strlen((char*)&pobSQLTag[i].szAttribute1) > 0)
		{
			strcat(szCreateSql, " ");
			strcat(szCreateSql, (char*)&pobSQLTag[i].szAttribute1);
		}
		/* Tag 屬性2 */
		if (strlen((char*)&pobSQLTag[i].szAttribute2) > 0)
		{
			strcat(szCreateSql, " ");
			strcat(szCreateSql, (char*)&pobSQLTag[i].szAttribute2);
		}
        }
	
	/* 設定外鍵 */
	strcat(szCreateSql, szAdditionalData);
	
	/* 後綴 */
	strcat(szCreateSql, szSqlSuffix);
	
	/* 建立 Table */
	inRetVal = sqlite3_exec(srDBConnection, szCreateSql, 0, 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Create Table ERROR Num:%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason:%s", szErrorMessage);
			inLogPrintf(AT, szDebugMsg);
			
			inLogPrintf(AT, szCreateSql);
		}
		
		/* 釋放記憶體 */
		free(szCreateSql);
		if (szErrorMessage != NULL)
		{
			sqlite3_free(szErrorMessage);
		}
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Create Table OK");
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 釋放記憶體 */
		free(szCreateSql);

		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Create_BatchTable_EMV()_END");
                inLogPrintf(AT, "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Drop_Table_Flow
Date&Time       :2017/3/14 下午 3:00
Describe        :在這邊決定名稱並分流
*/
int inSqlite_Drop_Table_Flow(TRANSACTION_OBJECT *pobTran, int inTableType)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	if (pobTran->uszFileNameNoNeedNumBit == VS_TRUE)
	{
		inFunc_ComposeFileName(pobTran, szTableName, "", 0);
	}
	else
	{
		inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	}
	
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_TICKET_:
                case _TN_BATCH_TABLE_TRUST_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_TEMP_:
			strcat(szTableName, _BATCH_TABLE_ESC_TEMP_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_ADVICE_:
			strcat(szTableName, _BATCH_TABLE_ESC_ADVICE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_ADVICE_EMV_:
			strcat(szTableName, _BATCH_TABLE_ESC_ADVICE_EMV_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			strcat(szTableName, _BATCH_TABLE_ADVICE_SUFFIX_);
		default :
			break;
	}
	
	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_BATCH_TABLE_ESC_TEMP_:
		case _TN_EMV_TABLE_:
		case _TN_BATCH_TABLE_TICKET_:
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
		case _TN_BATCH_TABLE_ESC_ADVICE_:
		case _TN_BATCH_TABLE_ESC_ADVICE_EMV_:
                case _TN_BATCH_TABLE_TRUST_:
			inRetVal = inSqlite_Drop_Table(gszTranDBPath, szTableName);
			break;
		default :
			return (VS_ERROR);
			break;
	}
	
	return (inRetVal);
}

/*
Function        :inSqlite_Drop_Table
Date&Time       :2016/4/11 下午 3:25
Describe        :delete 該batch的table
*/
int inSqlite_Drop_Table(char* szDBName, char* szTableName)
{
	int		inRetVal;
	char		szDebugMsg[84 + 1];
	char		szDeleteSql[_SQL_SENTENCE_MAX_ + 1];	/* INSERT INTO	szTableName( */
	char		*szErrorMessage = NULL;
	sqlite3		*srDBConnection;	/* 建立到資料庫的connection */
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Drop_Table()_START");
        }
	
	inRetVal = inSqlite_DB_Open_Or_Create(szDBName, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != VS_SUCCESS)
	{	
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szDeleteSql, 0x00, sizeof(szDeleteSql));
	sprintf(szDeleteSql, "DROP TABLE %s", szTableName);
	
	/* Delete */
	inRetVal = sqlite3_exec(srDBConnection, szDeleteSql, 0, 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Drop Table ERROR Num:%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason:%s", szErrorMessage);
			inLogPrintf(AT, szDebugMsg);
		}
		
		if (szErrorMessage != NULL)
		{
			sqlite3_free(szErrorMessage);
		}
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Drop OK");
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
	}

	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Drop_Table()_END");
                inLogPrintf(AT, "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Table_Link_BRec
Date&Time       :2017/3/13 上午 10:59
Describe        :將pobTran變數pointer位置放到Table中(用以解決每一個function都要放一個table的問題)
 *		 這邊直接把pobTran的pointer直接指到srAll(之後可能要考慮給動態記憶體)，TagName因為是寫在這個Function內的Table，所以要給實體位置儲存
*/
int inSqlite_Table_Link_BRec(TRANSACTION_OBJECT *pobTran, SQLITE_ALL_TABLE *srAll, int inLinkState)
{
	SQLITE_INT32T_TABLE TABLE_BATCH_INT[] = 
	{
		{0	,"inTableID"			,&pobTran->inTableID				},	/* inTableID */
		{0	,"inCode"			,&pobTran->srBRec.inCode			},	/* Trans Code */
		{0	,"inOrgCode"			,&pobTran->srBRec.inOrgCode			},	/* Original Trans Code  */
		{0	,"inPrintOption"		,&pobTran->srBRec.inPrintOption			},	/* Print Option Flag */
		{0	,"inHDTIndex"			,&pobTran->srBRec.inHDTIndex			},	/* 紀錄HDTindex */
		{0	,"inCDTIndex"			,&pobTran->srBRec.inCDTIndex			},	/* 紀錄CDTindex */
		{0	,"inCPTIndex"			,&pobTran->srBRec.inCPTIndex			},	/* 紀錄CPTindex */
		{0	,"inTxnResult"			,&pobTran->srBRec.inTxnResult			},	/* 紀錄交易結果 */
		{0	,"inChipStatus"			,&pobTran->srBRec.inChipStatus			},	/* 0 NOT_USING_CHIP, 1 EMV_CARD, 2 EMV_EASY_ENTRY_CARD */
		{0	,"inFiscIssuerIDLength"		,&pobTran->srBRec.inFiscIssuerIDLength		},	/* 金融卡發卡單位代號長度 */
		{0	,"inFiscCardCommentLength"	,&pobTran->srBRec.inFiscCardCommentLength	},	/* 金融卡備註欄長度 */
		{0	,"inFiscAccountLength"		,&pobTran->srBRec.inFiscAccountLength		},	/* 金融卡帳號長度 */
		{0	,"inFiscSTANLength"		,&pobTran->srBRec.inFiscSTANLength		},	/* 金融卡交易序號長度 */
		{0	,"inESCTransactionCode"		,&pobTran->srBRec.inESCTransactionCode		},	/* ESC組ISO使用 重新上傳使用 Transaction Code沒辦法存在Batch */
		{0	,"inESCUploadMode"		,&pobTran->srBRec.inESCUploadMode		},	/* 標示支不支援ESC */
		{0	,"inESCUploadStatus"		,&pobTran->srBRec.inESCUploadStatus		},	/* 標示ESC上傳狀態 */
		{0	,"inSignStatus"			,&pobTran->srBRec.inSignStatus			},	/* 簽名檔狀態(有 免簽 或 Bypass) ESC電文使用 */
		{0	,"inHGCreditHostIndex"		,&pobTran->srBRec.inHGCreditHostIndex		},	/* 聯合_HAPPY_GO_信用卡主機 */
		{0	,"inHGCode"			,&pobTran->srBRec.inHGCode			},	/* 聯合_HAPPY_GO_交易碼 */
		
		{0	,"lnTxnAmount"			,&pobTran->srBRec.lnTxnAmount			},	/* The transaction amount, such as a SALE */
		{0	,"lnOrgTxnAmount"		,&pobTran->srBRec.lnOrgTxnAmount		},	/* The ORG transaction amount, such as a SALE */
		{0	,"lnTipTxnAmount"		,&pobTran->srBRec.lnTipTxnAmount		},	/* The transaction amount, such as a TIP */
		{0	,"lnAdjustTxnAmount"		,&pobTran->srBRec.lnAdjustTxnAmount		},	/* The transaction amount, such as a ADJUST */
		{0	,"lnTotalTxnAmount"		,&pobTran->srBRec.lnTotalTxnAmount		},	/* The transaction amount, such as a TOTAL */
		{0	,"lnInvNum"			,&pobTran->srBRec.lnInvNum			},	/* 調閱編號  */
		{0	,"lnOrgInvNum"			,&pobTran->srBRec.lnOrgInvNum			},	/* Original 調閱編號  */
		{0	,"lnBatchNum"			,&pobTran->srBRec.lnBatchNum			},	/* Batch Number */
		{0	,"lnOrgBatchNum"		,&pobTran->srBRec.lnOrgBatchNum			},	/* Original Batch Number */
		{0	,"lnSTANNum"			,&pobTran->srBRec.lnSTANNum			},	/* Stan Number */
		{0	,"lnOrgSTANNum"			,&pobTran->srBRec.lnOrgSTANNum			},	/* Original Stan Number */
		{0	,"lnInstallmentPeriod"		,&pobTran->srBRec.lnInstallmentPeriod		},	/* 分期付款_期數 */
		{0	,"lnInstallmentDownPayment"	,&pobTran->srBRec.lnInstallmentDownPayment	},	/* 分期付款_頭期款 */
		{0	,"lnInstallmentPayment"		,&pobTran->srBRec.lnInstallmentPayment		},	/* 分期付款_每期款 */
		{0	,"lnInstallmentFormalityFee"	,&pobTran->srBRec.lnInstallmentFormalityFee	},	/* 分期付款_手續費 */
		{0	,"lnRedemptionPoints"		,&pobTran->srBRec.lnRedemptionPoints		},	/* 紅利扣抵_扣抵紅利點數 */
		{0	,"lnRedemptionPointsBalance"	,&pobTran->srBRec.lnRedemptionPointsBalance	},	/* 紅利扣抵_剩餘紅利點數 */
		{0	,"lnRedemptionPaidCreditAmount"	,&pobTran->srBRec.lnRedemptionPaidCreditAmount	},	/* 紅利扣抵_支付金額 */
		{0	,"lnHGTransactionType"		,&pobTran->srBRec.lnHGTransactionType		},	/* 聯合_HAPPY GO_交易類別 */
		{0	,"lnHGPaymentType"		,&pobTran->srBRec.lnHGPaymentType		},	/* 聯合_HAPPY_GO_支付工具 */
		{0	,"lnHGPaymentTeam"		,&pobTran->srBRec.lnHGPaymentTeam		},	/* 聯合_HAPPY_GO_支付工具_主機回_*/
		{0	,"lnHGBalancePoint"		,&pobTran->srBRec.lnHGBalancePoint		},	/* 聯合_HAPPY_GO_剩餘點數 */
		{0	,"lnHGTransactionPoint"		,&pobTran->srBRec.lnHGTransactionPoint		},	/* 聯合_HAPPY_GO_交易點數  合計 */
		{0	,"lnHGAmount"			,&pobTran->srBRec.lnHGAmount			},	/* 聯合_HAPPY_GO_扣抵後金額  (商品金額 = lnHGAmount + lnHGRedeemAmt) */
		{0	,"lnHGRedeemAmount"		,&pobTran->srBRec.lnHGRedeemAmount		},	/* 聯合_HAPPY_GO_扣抵金額 */
		{0	,"lnHGRefundLackPoint"		,&pobTran->srBRec.lnHGRefundLackPoint		},	/* 聯合_HAPPY_GO_不足點數 */
		{0	,"lnHGBatchIndex"		,&pobTran->srBRec.lnHGBatchIndex		},	/* 聯合_HAPPY_GO_主機當下批次號碼 */
		{0	,"lnHG_SPDH_OrgInvNum"		,&pobTran->srBRec.lnHG_SPDH_OrgInvNum		},	/* HAPPY_GO取消用INV */
		{0	,"lnHGSTAN"			,&pobTran->srBRec.lnHGSTAN			},	/* HAPPY_GO STAN */
		{0	,"lnCUPUPlanDiscountedAmount"	,&pobTran->srBRec.lnCUPUPlanDiscountedAmount	},	/* 銀聯優計畫折價後金額 */
		{0	,"lnCUPUPlanPreferentialAmount"	,&pobTran->srBRec.lnCUPUPlanPreferentialAmount	},	/* 銀聯優計畫優惠金額 */
		
		{0	,""				,NULL						}	/* 這行用Null用來知道尾端在哪 */
		
	};
	
	SQLITE_INT32T_TABLE TABLE_BATCH_INT64T[] = 
	{
		{0	,""				,NULL						}	/* 這行用Null用來知道尾端在哪 */
	};
	
	SQLITE_CHAR_TABLE TABLE_BATCH_CHAR[] ={
		{0, "szAuthCode", pobTran->srBRec.szAuthCode, strlen(pobTran->srBRec.szAuthCode)}, /* Auth Code */
		{0, "szMPASAuthCode", pobTran->srBRec.szMPASAuthCode, strlen(pobTran->srBRec.szMPASAuthCode)}, /* MPAS Auth Code */
		{0, "szRespCode", pobTran->srBRec.szRespCode, strlen(pobTran->srBRec.szRespCode)}, /* Response Code */
		{0, "szStoreID", pobTran->srBRec.szStoreID, strlen(pobTran->srBRec.szStoreID)}, /* StoreID */
		{0, "szCardLabel", pobTran->srBRec.szCardLabel, strlen(pobTran->srBRec.szCardLabel)}, /* 卡別  */
		{0, "szPAN", pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN)}, /* 卡號  */
		{0, "szDate", pobTran->srBRec.szDate, strlen(pobTran->srBRec.szDate)}, /* YYYYMMDD */
		{0, "szOrgDate", pobTran->srBRec.szOrgDate, strlen(pobTran->srBRec.szOrgDate)}, /* YYYYMMDD */
		{0, "szTime", pobTran->srBRec.szTime, strlen(pobTran->srBRec.szTime)}, /* HHMMSS */
		{0, "szOrgTime", pobTran->srBRec.szOrgTime, strlen(pobTran->srBRec.szOrgTime)}, /* HHMMSS */
		{0, "szCardTime", pobTran->srBRec.szCardTime, strlen(pobTran->srBRec.szCardTime)}, /* 晶片卡讀卡時間 , YYYYMMDDHHMMSS */
		{0, "szRefNo", pobTran->srBRec.szRefNo, strlen(pobTran->srBRec.szRefNo)}, /* 序號  */
		{0, "szExpDate", pobTran->srBRec.szExpDate, strlen(pobTran->srBRec.szExpDate)}, /* Expiration date */
		{0, "szServiceCode", pobTran->srBRec.szServiceCode, strlen(pobTran->srBRec.szServiceCode)}, /* Service code from track */
		{0, "szCardHolder", pobTran->srBRec.szCardHolder, strlen(pobTran->srBRec.szCardHolder)}, /* 持卡人名字 */
		{0, "szAMEX4DBC", pobTran->srBRec.szAMEX4DBC, strlen(pobTran->srBRec.szAMEX4DBC)},
		{0, "szFiscIssuerID", pobTran->srBRec.szFiscIssuerID, strlen(pobTran->srBRec.szFiscIssuerID)}, /* 發卡單位代號 */
		{0, "szFiscCardComment", pobTran->srBRec.szFiscCardComment, strlen(pobTran->srBRec.szFiscCardComment)}, /* 金融卡備註欄 */
		{0, "szFiscAccount", pobTran->srBRec.szFiscAccount, strlen(pobTran->srBRec.szFiscAccount)}, /* 金融卡帳號 */
		{0, "szFiscOutAccount", pobTran->srBRec.szFiscOutAccount, strlen(pobTran->srBRec.szFiscOutAccount)}, /* 金融卡轉出帳號 */
		{0, "szFiscSTAN", pobTran->srBRec.szFiscSTAN, strlen(pobTran->srBRec.szFiscSTAN)}, /* 金融卡交易序號 */
		{0, "szFiscTacLength", pobTran->srBRec.szFiscTacLength, strlen(pobTran->srBRec.szFiscTacLength)}, /* 金融卡Tac長度 */
		{0, "szFiscTac", pobTran->srBRec.szFiscTac, strlen(pobTran->srBRec.szFiscTac)}, /* 金融卡Tac */
		{0, "szFiscTCC", pobTran->srBRec.szFiscTCC, strlen(pobTran->srBRec.szFiscTCC)}, /* 端末設備查核碼 */
		{0, "szFiscMCC", pobTran->srBRec.szFiscMCC, strlen(pobTran->srBRec.szFiscMCC)}, /* 金融卡MCC */
		{0, "szFiscRRN", pobTran->srBRec.szFiscRRN, strlen(pobTran->srBRec.szFiscRRN)}, /* 金融卡調單編號 */
		{0, "szFiscRefundDate", pobTran->srBRec.szFiscRefundDate, strlen(pobTran->srBRec.szFiscRefundDate)}, /* 金融卡退貨原始交易日期(YYYYMMDD) */
		{0, "szFiscDateTime", pobTran->srBRec.szFiscDateTime, strlen(pobTran->srBRec.szFiscDateTime)}, /* 計算TAC(S2)的交易日期時間 */
		{0, "szFiscPayDevice", pobTran->srBRec.szFiscPayDevice, strlen(pobTran->srBRec.szFiscPayDevice)}, /* 金融卡付款裝置 1 = 手機 2 = 卡片 */
		{0, "szFiscMobileDevice", pobTran->srBRec.szFiscMobileDevice, strlen(pobTran->srBRec.szFiscMobileDevice)}, /* SE 類型，0x05：雲端卡片(Cloud-Based) */
		{0, "szFiscMobileNFType", pobTran->srBRec.szFiscMobileNFType, strlen(pobTran->srBRec.szFiscMobileNFType)}, /* 行動金融卡是否需輸入密碼 00不需要 01視情況 02一定要 */
		{0, "szFiscMobileNFSetting", pobTran->srBRec.szFiscMobileNFSetting, strlen(pobTran->srBRec.szFiscMobileNFSetting)}, /* 近端交易類型設定 0x00：Single Issuer Wallet 0x01：國內Third-Party Wallet 0x02~9：保留 0x0A：其他 */
		{0, "szInstallmentIndicator", pobTran->srBRec.szInstallmentIndicator, strlen(pobTran->srBRec.szInstallmentIndicator)},
		{0, "szRedeemIndicator", pobTran->srBRec.szRedeemIndicator, strlen(pobTran->srBRec.szRedeemIndicator)},
		{0, "szRedeemSignOfBalance", pobTran->srBRec.szRedeemSignOfBalance, strlen(pobTran->srBRec.szRedeemSignOfBalance)},
		{0, "szHGCardLabel", pobTran->srBRec.szHGCardLabel, strlen(pobTran->srBRec.szHGCardLabel)}, /* HAPPY_GO 卡別 */
		{0, "szHGPAN", pobTran->srBRec.szHGPAN, strlen(pobTran->srBRec.szHGPAN)}, /* HAPPY_GO Account number */
		{0, "szHGAuthCode", pobTran->srBRec.szHGAuthCode, strlen(pobTran->srBRec.szHGAuthCode)}, /* HAPPY_GO 授權碼 */
		{0, "szHGRefNo", pobTran->srBRec.szHGRefNo, strlen(pobTran->srBRec.szHGRefNo)}, /* HAPPY_GO Reference Number */
		{0, "szHGRespCode", pobTran->srBRec.szHGRespCode, strlen(pobTran->srBRec.szHGRespCode)}, /* HG Response Code */
		{0, "szCUP_TN", pobTran->srBRec.szCUP_TN, strlen(pobTran->srBRec.szCUP_TN)}, /* CUP Trace Number (TN) */
		{0, "szCUP_TD", pobTran->srBRec.szCUP_TD, strlen(pobTran->srBRec.szCUP_TD)}, /* CUP Transaction Date (MMDD) */
		{0, "szCUP_TT", pobTran->srBRec.szCUP_TT, strlen(pobTran->srBRec.szCUP_TT)}, /* CUP Transaction Time (hhmmss) */
		{0, "szCUP_RRN", pobTran->srBRec.szCUP_RRN, strlen(pobTran->srBRec.szCUP_RRN)}, /* CUP Retrieve Reference Number (CRRN) */
		{0, "szCUP_STD", pobTran->srBRec.szCUP_STD, strlen(pobTran->srBRec.szCUP_STD)}, /* CUP Settlement Date(MMDD) Of Host Response */
		{0, "szCUP_EMVAID", pobTran->srBRec.szCUP_EMVAID, strlen(pobTran->srBRec.szCUP_EMVAID)}, /* CUP晶片交易存AID帳單列印使用 */
		{0, "szTranAbbrev", pobTran->srBRec.szTranAbbrev, strlen(pobTran->srBRec.szTranAbbrev)}, /* Tran abbrev for reports */
		{0, "szIssueNumber", pobTran->srBRec.szIssueNumber, strlen(pobTran->srBRec.szIssueNumber)},
		{0, "szStore_DREAM_MALL", pobTran->srBRec.szStore_DREAM_MALL, strlen(pobTran->srBRec.szStore_DREAM_MALL)}, /* 存Dream_Mall Account Number And Member ID*/
		{0, "szDCC_FCNFR", pobTran->srBRec.szDCC_FCNFR, strlen(pobTran->srBRec.szDCC_FCNFR)}, /* Foreign Currency No. For Rate */
		{0, "szDCC_AC", pobTran->srBRec.szDCC_AC, strlen(pobTran->srBRec.szDCC_AC)}, /* Action Code */
		{0, "szDCC_FCN", pobTran->srBRec.szDCC_FCN, strlen(pobTran->srBRec.szDCC_FCN)}, /* Foreign Currency Number */
		{0, "szDCC_FCA", pobTran->srBRec.szDCC_FCA, strlen(pobTran->srBRec.szDCC_FCA)}, /* Foreign Currency Amount */
		{0, "szDCC_FCMU", pobTran->srBRec.szDCC_FCMU, strlen(pobTran->srBRec.szDCC_FCMU)}, /* Foreign Currency Minor Unit */
		{0, "szDCC_FCAC", pobTran->srBRec.szDCC_FCAC, strlen(pobTran->srBRec.szDCC_FCAC)}, /* Foreign currcncy Alphabetic Code */
		{0, "szDCC_ERMU", pobTran->srBRec.szDCC_ERMU, strlen(pobTran->srBRec.szDCC_ERMU)}, /* Exchange Rate Minor Unit */
		{0, "szDCC_ERV", pobTran->srBRec.szDCC_ERV, strlen(pobTran->srBRec.szDCC_ERV)}, /* Exchange Rate Value */
		{0, "szDCC_IRMU", pobTran->srBRec.szDCC_IRMU, strlen(pobTran->srBRec.szDCC_IRMU)}, /* Inverted Rate Minor Unit */
		{0, "szDCC_IRV", pobTran->srBRec.szDCC_IRV, strlen(pobTran->srBRec.szDCC_IRV)}, /* Inverted Rate Value */
		{0, "szDCC_IRDU", pobTran->srBRec.szDCC_IRDU, strlen(pobTran->srBRec.szDCC_IRDU)}, /* Inverted Rate Display Unit */
		{0, "szDCC_MPV", pobTran->srBRec.szDCC_MPV, strlen(pobTran->srBRec.szDCC_MPV)}, /* Markup Percentage Value */
		{0, "szDCC_MPDP", pobTran->srBRec.szDCC_MPDP, strlen(pobTran->srBRec.szDCC_MPDP)}, /* Markup Percentage Decimal Point */
		{0, "szDCC_CVCN", pobTran->srBRec.szDCC_CVCN, strlen(pobTran->srBRec.szDCC_CVCN)}, /* Commissino Value Currency Number */
		{0, "szDCC_CVCA", pobTran->srBRec.szDCC_CVCA, strlen(pobTran->srBRec.szDCC_CVCA)}, /* Commission Value Currency Amount */
		{0, "szDCC_CVCMU", pobTran->srBRec.szDCC_CVCMU, strlen(pobTran->srBRec.szDCC_CVCMU)}, /* Commission Value Currency Minor Unit */
		{0, "szDCC_TIPFCA", pobTran->srBRec.szDCC_TIPFCA, strlen(pobTran->srBRec.szDCC_TIPFCA)}, /* Tip Foreign Currency Amount */
		{0, "szDCC_OTD", pobTran->srBRec.szDCC_OTD, strlen(pobTran->srBRec.szDCC_OTD)}, /* Original Transaction Date & Time (MMDD) */
		{0, "szDCC_OTA", pobTran->srBRec.szDCC_OTA, strlen(pobTran->srBRec.szDCC_OTA)}, /* Original Transaction Amount */
		{0, "szProductCode", pobTran->srBRec.szProductCode, strlen(pobTran->srBRec.szProductCode)}, /* 產品代碼 */
		{0, "szAwardNum", pobTran->srBRec.szAwardNum, strlen(pobTran->srBRec.szAwardNum)}, /* 優惠個數 */
		{0, "szAwardSN", pobTran->srBRec.szAwardSN, strlen(pobTran->srBRec.szAwardSN)}, /* 優惠序號(Award S/N) TID(8Bytes)+YYYYMMDDhhmmss(16 Bytes)，共22Bytes */
		{0, "szTxnNo", pobTran->srBRec.szTxnNo, strlen(pobTran->srBRec.szTxnNo)}, /* 交易編號 */
		{0, "szMCP_BANKID", pobTran->srBRec.szMCP_BANKID, strlen(pobTran->srBRec.szMCP_BANKID)}, /* 行動支付標記 金融機構代碼 */
		{0, "szPayItemCode", pobTran->srBRec.szPayItemCode, strlen(pobTran->srBRec.szPayItemCode)}, /* 繳費項目代碼 */
		{0, "szTableTD_Data", pobTran->srBRec.szTableTD_Data, strlen(pobTran->srBRec.szTableTD_Data)}, /* Table TD的資料， */
		{0, "szDFSTraceNum", pobTran->srBRec.szDFSTraceNum, strlen(pobTran->srBRec.szDFSTraceNum)}, /* DFS交易系統追蹤號 */
		{0, "szCheckNO", pobTran->srBRec.szCheckNO, strlen(pobTran->srBRec.szCheckNO)}, /* 檢查碼，目前FOR SDK使用 */
		{0, "szEIVI_BANKID", pobTran->srBRec.szEIVI_BANKID, strlen(pobTran->srBRec.szEIVI_BANKID)}, /* E Invoice vehicle I ssue Bank ID 支援電子發票載具之發卡行代碼 */
		{0, "szBarCodeData", pobTran->srBRec.szBarCodeData, strlen(pobTran->srBRec.szBarCodeData)}, /* 掃碼交易資料內容 */
		{0, "szUnyTransCode", pobTran->srBRec.szUnyTransCode, strlen(pobTran->srBRec.szUnyTransCode)}, /* Uny交易碼 */
		{0, "szUnyMaskedCardNo", pobTran->srBRec.szUnyMaskedCardNo, strlen(pobTran->srBRec.szUnyMaskedCardNo)}, /* 遮掩交易卡號資料(非完整卡號，有遮掩，故另設欄位) */
		{0, "szEInvoiceHASH", pobTran->srBRec.szEInvoiceHASH, strlen(pobTran->srBRec.szEInvoiceHASH)}, /* 電子發票HASH 值（發卡銀行若無參加電子發票載具，則此欄位回覆值 為空白 SPACE 。） */
		{0, "szUnyCardLabelLen", pobTran->srBRec.szUnyCardLabelLen, strlen(pobTran->srBRec.szUnyCardLabelLen)}, /* 卡別名稱長度，szMCP_BANKID為Twin卡專用，故另設欄位 */
		{0, "szUnyCardLabel", pobTran->srBRec.szUnyCardLabel, strlen(pobTran->srBRec.szUnyCardLabel)}, /* 卡別名稱，szMCP_BANKID為Twin卡專用，故另設欄位 */
		{0, "szFPG_FTC_Invoice", pobTran->srBRec.szFPG_FTC_Invoice, strlen(pobTran->srBRec.szFPG_FTC_Invoice)}, /* 台塑生醫生技客製化專屬【005】　台亞福懋加油站客製化專屬【006】發票號碼 */
		{0, "szUPlan_Coupon", pobTran->srBRec.szUPlan_Coupon, strlen(pobTran->srBRec.szUPlan_Coupon)}, /* 銀聯優計畫的優惠訊息 */
		{0, "szUPlan_RemarksInformation", pobTran->srBRec.szUPlan_RemarksInformation, strlen(pobTran->srBRec.szUPlan_RemarksInformation)}, /* 銀聯優計畫-備註訊息 */
		{0, "szFlightTicketTransBit", pobTran->srBRec.szFlightTicketTransBit, strlen(pobTran->srBRec.szFlightTicketTransBit)}, /* 判斷是否是機票交易 不設定為空值 機票交易為1 非機票交易為2 */
		{0, "szFlightTicketPDS0523", pobTran->srBRec.szFlightTicketPDS0523, strlen(pobTran->srBRec.szFlightTicketPDS0523)}, /* 出發地機場代碼（PDS 0523）左靠右補空白 */
		{0, "szFlightTicketPDS0524", pobTran->srBRec.szFlightTicketPDS0524, strlen(pobTran->srBRec.szFlightTicketPDS0524)}, /* 目的地機場代碼（PDS 0524）左靠右補空白 */
		{0, "szFlightTicketPDS0530", pobTran->srBRec.szFlightTicketPDS0530, strlen(pobTran->srBRec.szFlightTicketPDS0530)}, /* 航班號碼（PDS 0530） */
		
		{0, "uszWAVESchemeID", &pobTran->srBRec.uszWAVESchemeID, 1}, /* WAVE 使用用於組電文 Field_22 */
		{0, "uszVOIDBit", &pobTran->srBRec.uszVOIDBit, 1}, /* 負向交易 */
		{0, "uszUpload1Bit", &pobTran->srBRec.uszUpload1Bit, 1}, /* Offline交易使用 (原交易advice是否已上傳)*/
		{0, "uszUpload2Bit", &pobTran->srBRec.uszUpload2Bit, 1}, /* Offline交易使用 (當前交易是否為advice)*/
		{0, "uszUpload3Bit", &pobTran->srBRec.uszUpload3Bit, 1}, /* Offline交易使用 */
		{0, "uszReferralBit", &pobTran->srBRec.uszReferralBit, 1}, /* ISO Response Code 【01】【02】使用 */
		{0, "uszOfflineBit", &pobTran->srBRec.uszOfflineBit, 1}, /* 離線交易 */
		{0, "uszManualBit", &pobTran->srBRec.uszManualBit, 1}, /* Manual Keyin */
		{0, "uszNoSignatureBit", &pobTran->srBRec.uszNoSignatureBit, 1}, /* 免簽名使用 (免簽名則為true)*/
		{0, "uszCUPTransBit", &pobTran->srBRec.uszCUPTransBit, 1}, /* 是否為CUP */
		{0, "uszFiscTransBit", &pobTran->srBRec.uszFiscTransBit, 1}, /* SmartPay交易，是否為金融卡 */
		{0, "uszContactlessBit", &pobTran->srBRec.uszContactlessBit, 1}, /* 是否為非接觸式 */
		{0, "uszEMVFallBackBit", &pobTran->srBRec.uszEMVFallBackBit, 1}, /* 是否要啟動fallback */
		{0, "uszInstallmentbit", &pobTran->srBRec.uszInstallmentBit, 1}, /* Installment */
		{0, "uszRedeemBit", &pobTran->srBRec.uszRedeemBit, 1}, /* Redemption */
		{0, "uszForceOnlineBit", &pobTran->srBRec.uszForceOnlineBit, 1}, /* 組電文使用 Field_25 Point of Service Condition Code */
		{0, "uszMail_OrderBit", &pobTran->srBRec.uszMail_OrderBit, 1}, /* 組電文使用 Field_25 Point of Service Condition Code */
		{0, "uszDCCTransBit", &pobTran->srBRec.uszDCCTransBit, 1}, /* 是否為DCC交易 */
		{0, "uszNCCCDCCRateBit", &pobTran->srBRec.uszNCCCDCCRateBit, 1},
		{0, "uszCVV2Bit", &pobTran->srBRec.uszCVV2Bit, 1},
		{0, "uszRewardSuspendBit", &pobTran->srBRec.uszRewardSuspendBit, 1},
		{0, "uszRewardL1Bit", &pobTran->srBRec.uszRewardL1Bit, 1},
		{0, "uszRewardL2Bit", &pobTran->srBRec.uszRewardL2Bit, 1},
		{0, "uszRewardL5Bit", &pobTran->srBRec.uszRewardL5Bit, 1}, /* 要印L5 */
		{0, "uszField24NPSBit", &pobTran->srBRec.uszField24NPSBit, 1},
		{0, "uszVEPS_SignatureBit", &pobTran->srBRec.uszVEPS_SignatureBit, 1}, /* VEPS 免簽名是否成立 */
		{0, "uszTCUploadBit", &pobTran->srBRec.uszTCUploadBit, 1}, /* TCUpload是否已上傳 */
		{0, "uszFiscConfirmBit", &pobTran->srBRec.uszFiscConfirmBit, 1}, /* SmartPay 0220 是否已上傳 */
		{0, "uszFiscVoidConfirmBit", &pobTran->srBRec.uszFiscVoidConfirmBit, 1}, /* SmartPay Void 0220 是否已上傳 */
		{0, "uszPinEnterBit", &pobTran->srBRec.uszPinEnterBit, 1}, /* 此筆交易是否有鍵入密碼(只能確認原交易，若該筆之後的調整或取消不會將此Bit Off) */
		{0, "uszL2PrintADBit", &pobTran->srBRec.uszL2PrintADBit, 1}, /* L2是否印AD，因L2原交易取消要判斷，只好增加 */
		{0, "uszInstallment", &pobTran->srBRec.uszInstallment, 1}, /* HappyGo分期交易 */
		{0, "uszRedemption", &pobTran->srBRec.uszRedemption, 1}, /* HappyGo點數兌換 */
		{0, "uszHappyGoSingle", &pobTran->srBRec.uszHappyGoSingle, 1}, /* HappyGo交易 */
		{0, "uszHappyGoMulti", &pobTran->srBRec.uszHappyGoMulti, 1}, /* HappyGo混合交易 */
		{0, "uszCLSBatchBit", &pobTran->srBRec.uszCLSBatchBit, 1}, /* 是否已接續上傳 */
		{0, "uszTxNoCheckBit", &pobTran->srBRec.uszTxNoCheckBit, 1}, /* 商店自存聯卡號遮掩開關 */
		{0, "uszSpecial00Bit", &pobTran->srBRec.uszSpecial00Bit, 1}, /* 特殊卡別參數檔，活動代碼00表示免簽(只紀錄，主要看uszNoSignatureBit) */
		{0, "uszSpecial01Bit", &pobTran->srBRec.uszSpecial01Bit, 1}, /* 特殊卡別參數檔，活動代碼01表示ECR回傳明碼(先決條件ECR卡號遮掩有開才做判斷) */
		{0, "uszRefundCTLSBit", &pobTran->srBRec.uszRefundCTLSBit, 1}, /* 用在簽單印(W) 因為送電文contactless bit已OFF轉Manual Key in */
		{0, "uszMPASTransBit", &pobTran->srBRec.uszMPASTransBit, 1}, /* 標示為小額交易 */
		{0, "uszMPASReprintBit", &pobTran->srBRec.uszMPASReprintBit, 1}, /* 標示該小額交易是否可重印 */
		{0, "uszMobilePayBit", &pobTran->srBRec.uszMobilePayBit, 1}, /* 判斷是不是行動支付 Table NC */
		{0, "uszUCARDTransBit", &pobTran->srBRec.uszUCARDTransBit, 1}, /* 判斷是否是自由品牌卡 */
		{0, "uszUnyTransBit", &pobTran->srBRec.uszUnyTransBit, 1}, /* 是否是Uny交易 */
		{0, "uszESCOrgUploadBit", &pobTran->srBRec.uszESCOrgUploadBit, 1}, /* 用於標明原交易的ESC上傳狀態 */
		{0, "uszESCTipUploadBit", &pobTran->srBRec.uszESCTipUploadBit, 1}, /* 用於標明Tip的ESC上傳狀態 */
		{0, "uszTakaTransBit", &pobTran->srBRec.uszTakaTransBit, 1}, /* 客製化026、021 Taka社員卡交易 */
		{0, "uszUPlanECRBit", &pobTran->srBRec.uszUPlanECRBit, 1}, /* 判斷是否為優計畫ECR規格(用以決定是否回傳hash value) */
		{0, "uszCUPEMVQRCodeBit", &pobTran->srBRec.uszCUPEMVQRCodeBit, 1}, /* 判斷是否使用銀聯QRCode支付 */
		{0, "uszUPlanTransBit", &pobTran->srBRec.uszUPlanTransBit, 1}, /* 判斷是否為優計畫交易(使用銀聯優惠券) */
		{0, "", NULL, 0} /* 這行用Null用來知道尾端在哪 */
	};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inSqlite_Table_Link_BRec()_START");
	}

	SQLITE_LINK_TABLE srLink;
	memset(&srLink, 0x00, sizeof (SQLITE_LINK_TABLE));
	srLink.psrInt = TABLE_BATCH_INT;
	srLink.psrInt64t = TABLE_BATCH_INT64T;
	srLink.psrChar = TABLE_BATCH_CHAR;
	srLink.psrText = NULL;

	inSqlite_Table_Link(srAll, inLinkState, &srLink);

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inSqlite_Table_Link_BRec()_END");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Table_Link_EMVRec
Date&Time       :2017/3/15 上午 9:49
Describe        :將pobTran變數pointer位置放到Table中(用以解決每一個function都要放一個table的問題)
 *		 這邊直接把pobTran的pointer直接指到srAll(之後可能要考慮給動態記憶體)，TagName因為是寫在這個Function內的Table，所以要給實體位置儲存
*/
int inSqlite_Table_Link_EMVRec(TRANSACTION_OBJECT *pobTran, SQLITE_ALL_TABLE *srAll, int inLinkState)
{
	SQLITE_INT32T_TABLE TABLE_BATCH_INT[] = 
	{
		{0	,"inBatchTableID"				,&pobTran->inTableID						},	/* Table ID FOREIGN  key, sqlite table專用，用來對應Batch Table的FOREIGN Key */
		{0	,"inEMVCardDecision"				,&pobTran->srEMVRec.inEMVCardDecision				},
		{0	,"in50_APLabelLen"				,&pobTran->srEMVRec.in50_APLabelLen				},
		{0	,"in5A_ApplPanLen"				,&pobTran->srEMVRec.in5A_ApplPanLen				},
		{0	,"in5F20_CardholderNameLen"			,&pobTran->srEMVRec.in5F20_CardholderNameLen			},
		{0	,"in5F24_ExpireDateLen"				,&pobTran->srEMVRec.in5F24_ExpireDateLen			},
		{0	,"in5F2A_TransCurrCodeLen"			,&pobTran->srEMVRec.in5F2A_TransCurrCodeLen			},
		{0	,"in5F28_IssuerCountryCodeLen"			,&pobTran->srEMVRec.in5F28_IssuerCountryCodeLen			},
		{0	,"in5F34_ApplPanSeqnumLen"			,&pobTran->srEMVRec.in5F34_ApplPanSeqnumLen			},
		{0	,"in71_IssuerScript1Len"			,&pobTran->srEMVRec.in71_IssuerScript1Len			},
		{0	,"in72_IssuerScript2Len"			,&pobTran->srEMVRec.in72_IssuerScript2Len			},
		{0	,"in82_AIPLen"					,&pobTran->srEMVRec.in82_AIPLen					},
		{0	,"in84_DFNameLen"				,&pobTran->srEMVRec.in84_DFNameLen				},
		{0	,"in8A_AuthRespCodeLen"				,&pobTran->srEMVRec.in8A_AuthRespCodeLen			},
		{0	,"in91_IssuerAuthDataLen"			,&pobTran->srEMVRec.in91_IssuerAuthDataLen			},
		{0	,"in95_TVRLen"					,&pobTran->srEMVRec.in95_TVRLen					},
		{0	,"in9A_TranDateLen"				,&pobTran->srEMVRec.in9A_TranDateLen				},
		{0	,"in9B_TSILen"					,&pobTran->srEMVRec.in9B_TSILen					},
		{0	,"in9C_TranTypeLen"				,&pobTran->srEMVRec.in9C_TranTypeLen				},
		{0	,"in9F02_AmtAuthNumLen"				,&pobTran->srEMVRec.in9F02_AmtAuthNumLen			},
		{0	,"in9F03_AmtOtherNumLen"			,&pobTran->srEMVRec.in9F03_AmtOtherNumLen			},
		{0	,"in9F08_AppVerNumICCLen"			,&pobTran->srEMVRec.in9F08_AppVerNumICCLen			},
		{0	,"in9F09_TermVerNumLen"				,&pobTran->srEMVRec.in9F09_TermVerNumLen			},
		{0	,"in9F10_IssuerAppDataLen"			,&pobTran->srEMVRec.in9F10_IssuerAppDataLen			},
		{0	,"in9F18_IssuerScriptIDLen"			,&pobTran->srEMVRec.in9F18_IssuerScriptIDLen			},
		{0	,"in9F1A_TermCountryCodeLen"			,&pobTran->srEMVRec.in9F1A_TermCountryCodeLen			},
		{0	,"in9F1E_IFDNumLen"				,&pobTran->srEMVRec.in9F1E_IFDNumLen				},
		{0	,"in9F26_ApplCryptogramLen"			,&pobTran->srEMVRec.in9F26_ApplCryptogramLen			},
		{0	,"in9F27_CIDLen"				,&pobTran->srEMVRec.in9F27_CIDLen				},
		{0	,"in9F33_TermCapabilitiesLen"			,&pobTran->srEMVRec.in9F33_TermCapabilitiesLen			},
		{0	,"in9F34_CVMLen"				,&pobTran->srEMVRec.in9F34_CVMLen				},
		{0	,"in9F35_TermTypeLen"				,&pobTran->srEMVRec.in9F35_TermTypeLen				},
		{0	,"in9F36_ATCLen"				,&pobTran->srEMVRec.in9F36_ATCLen				},
		{0	,"in9F37_UnpredictNumLen"			,&pobTran->srEMVRec.in9F37_UnpredictNumLen			},
		{0	,"in9F41_TransSeqCounterLen"			,&pobTran->srEMVRec.in9F41_TransSeqCounterLen			},
		{0	,"in9F42_ApplicationCurrencyCodeLen"		,&pobTran->srEMVRec.in9F42_ApplicationCurrencyCodeLen		},
		{0	,"in9F5A_Application_Program_IdentifierLen"	,&pobTran->srEMVRec.in9F5A_Application_Program_IdentifierLen	},
		{0	,"in9F5B_ISRLen"				,&pobTran->srEMVRec.in9F5B_ISRLen				},
                {0	,"in9F60_CouponInformationLen"			,&pobTran->srEMVRec.in9F60_CouponInformationLen			},
		{0	,"in9F63_CardProductLabelInformationLen"	,&pobTran->srEMVRec.in9F63_CardProductLabelInformationLen	},
		{0	,"in9F66_QualifiersLen"				,&pobTran->srEMVRec.in9F66_QualifiersLen			},
		{0	,"in9F6C_Card_Transaction_QualifiersLen"	,&pobTran->srEMVRec.in9F6C_Card_Transaction_QualifiersLen	},
		{0	,"in9F6E_From_Factor_IndicatorLen"		,&pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen		},
		{0	,"in9F74_TLVLen"				,&pobTran->srEMVRec.in9F74_TLVLen				},
		{0	,"in9F7C_PartnerDiscretionaryDataLen"		,&pobTran->srEMVRec.in9F7C_PartnerDiscretionaryDataLen		},
		{0	,"inDF69_NewJspeedyModeLen"			,&pobTran->srEMVRec.inDF69_NewJspeedyModeLen			},
		{0	,"inDF8F4F_TransactionResultLen"		,&pobTran->srEMVRec.inDF8F4F_TransactionResultLen		},
		{0	,"inDFEC_FallBackIndicatorLen"			,&pobTran->srEMVRec.inDFEC_FallBackIndicatorLen			},
		{0	,"inDFED_ChipConditionCodeLen"			,&pobTran->srEMVRec.inDFED_ChipConditionCodeLen			},
		{0	,"inDFEE_TerEntryCapLen"			,&pobTran->srEMVRec.inDFEE_TerEntryCapLen			},
		{0	,"inDFEF_ReasonOnlineCodeLen"			,&pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen			},
		{0	,""						,NULL								}/* 這行用Null用來知道尾端在哪 */
		
	};
	
	SQLITE_INT32T_TABLE TABLE_BATCH_INT64T[] = 
	{
		{0	,""						,NULL								}/* 這行用Null用來知道尾端在哪 */
	};
	
	SQLITE_CHAR_TABLE TABLE_BATCH_CHAR[] =
	{
		{0	,"usz50_APLabel"				,pobTran->srEMVRec.usz50_APLabel				,17	},
		{0	,"usz5A_ApplPan"				,pobTran->srEMVRec.usz5A_ApplPan				,11	},
		{0	,"usz5F20_CardholderName"			,pobTran->srEMVRec.usz5F20_CardholderName			,27	},
		{0	,"usz5F24_ExpireDate"				,pobTran->srEMVRec.usz5F24_ExpireDate				,5	},
		{0	,"usz5F28_IssuerCountryCode"			,pobTran->srEMVRec.usz5F28_IssuerCountryCode			,2	},
		{0	,"usz5F2A_TransCurrCode"			,pobTran->srEMVRec.usz5F2A_TransCurrCode			,3	},
		{0	,"usz5F34_ApplPanSeqnum"			,pobTran->srEMVRec.usz5F34_ApplPanSeqnum			,3	},
		{0	,"usz71_IssuerScript1"				,pobTran->srEMVRec.usz71_IssuerScript1				,263	},
		{0	,"usz72_IssuerScript2"				,pobTran->srEMVRec.usz72_IssuerScript2				,263	},
		{0	,"usz82_AIP"					,pobTran->srEMVRec.usz82_AIP					,3	},
		{0	,"usz84_DF_NAME"				,pobTran->srEMVRec.usz84_DF_NAME				,17	},
		{0	,"usz8A_AuthRespCode"				,pobTran->srEMVRec.usz8A_AuthRespCode				,3	},
		{0	,"usz91_IssuerAuthData"				,pobTran->srEMVRec.usz91_IssuerAuthData				,17	},
		{0	,"usz95_TVR"					,pobTran->srEMVRec.usz95_TVR					,7	},
		{0	,"usz9A_TranDate"				,pobTran->srEMVRec.usz9A_TranDate				,5	},
		{0	,"usz9B_TSI"					,pobTran->srEMVRec.usz9B_TSI					,3	},
		{0	,"usz9C_TranType"				,pobTran->srEMVRec.usz9C_TranType				,3	},
		{0	,"usz9F02_AmtAuthNum"				,pobTran->srEMVRec.usz9F02_AmtAuthNum				,7	},
		{0	,"usz9F03_AmtOtherNum"				,pobTran->srEMVRec.usz9F03_AmtOtherNum				,7	},
		{0	,"usz9F08_AppVerNumICC"				,pobTran->srEMVRec.usz9F08_AppVerNumICC				,3	},
		{0	,"usz9F09_TermVerNum"				,pobTran->srEMVRec.usz9F09_TermVerNum				,3	},
		{0	,"usz9F10_IssuerAppData"			,pobTran->srEMVRec.usz9F10_IssuerAppData			,33	},
		{0	,"usz9F18_IssuerScriptID"			,pobTran->srEMVRec.usz9F18_IssuerScriptID			,5	},
		{0	,"usz9F1A_TermCountryCode"			,pobTran->srEMVRec.usz9F1A_TermCountryCode			,3	},
		{0	,"usz9F1E_IFDNum"				,pobTran->srEMVRec.usz9F1E_IFDNum				,9	},
		{0	,"usz9F26_ApplCryptogram"			,pobTran->srEMVRec.usz9F26_ApplCryptogram			,9	},
		{0	,"usz9F27_CID"					,pobTran->srEMVRec.usz9F27_CID					,3	},
		{0	,"usz9F33_TermCapabilities"			,pobTran->srEMVRec.usz9F33_TermCapabilities			,5	},
		{0	,"usz9F34_CVM"					,pobTran->srEMVRec.usz9F34_CVM					,5	},
		{0	,"usz9F35_TermType"				,pobTran->srEMVRec.usz9F35_TermType				,3	},
		{0	,"usz9F36_ATC"					,pobTran->srEMVRec.usz9F36_ATC					,3	},
		{0	,"usz9F37_UnpredictNum"				,pobTran->srEMVRec.usz9F37_UnpredictNum				,5	},
		{0	,"usz9F41_TransSeqCounter"			,pobTran->srEMVRec.usz9F41_TransSeqCounter			,5	},
		{0	,"usz9F42_ApplicationCurrencyCode"		,pobTran->srEMVRec.usz9F42_ApplicationCurrencyCode		,2	},
		{0	,"usz9F5A_Application_Program_Identifier"	,pobTran->srEMVRec.usz9F5A_Application_Program_Identifier	,33	},
		{0	,"usz9F5B_ISR"					,pobTran->srEMVRec.usz9F5B_ISR					,7	},
                {0	,"usz9F60_CouponInformation"                    ,pobTran->srEMVRec.usz9F60_CouponInformation                    ,30	},
		{0	,"usz9F63_CardProductLabelInformation"		,pobTran->srEMVRec.usz9F63_CardProductLabelInformation		,17	},
		{0	,"usz9F66_Qualifiers"				,pobTran->srEMVRec.usz9F66_Qualifiers				,5	},
		{0	,"usz9F6C_Card_Transaction_Qualifiers"		,pobTran->srEMVRec.usz9F6C_Card_Transaction_Qualifiers		,3	},
		{0	,"usz9F6E_From_Factor_Indicator"		,pobTran->srEMVRec.usz9F6E_From_Factor_Indicator		,33	},
		{0	,"usz9F74_TLV"					,pobTran->srEMVRec.usz9F74_TLV					,7	},
		{0	,"usz9F7C_PartnerDiscretionaryData"		,pobTran->srEMVRec.usz9F7C_PartnerDiscretionaryData		,33	},
		{0	,"uszDF69_NewJspeedyMode"			,pobTran->srEMVRec.uszDF69_NewJspeedyMode			,3	},
		{0	,"uszDF8F4F_TransactionResult"			,pobTran->srEMVRec.uszDF8F4F_TransactionResult			,3	},
		{0	,"uszDFEC_FallBackIndicator"			,pobTran->srEMVRec.uszDFEC_FallBackIndicator			,3	},
		{0	,"uszDFED_ChipConditionCode"			,pobTran->srEMVRec.uszDFED_ChipConditionCode			,3	},
		{0	,"uszDFEE_TerEntryCap"				,pobTran->srEMVRec.uszDFEE_TerEntryCap				,3	},
		{0	,"uszDFEF_ReasonOnlineCode"			,pobTran->srEMVRec.uszDFEF_ReasonOnlineCode			,5	},
		
		{0	,""						,NULL								,0	}/* 這行用Null用來知道尾端在哪 */
	};
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Table_Link_EMVRec()_START");
        }
	
	SQLITE_LINK_TABLE	srLink;
	memset(&srLink, 0x00, sizeof(SQLITE_LINK_TABLE));
	srLink.psrInt = TABLE_BATCH_INT;
	srLink.psrInt64t = TABLE_BATCH_INT64T;
	srLink.psrChar = TABLE_BATCH_CHAR;
	srLink.psrText = NULL;
	
	inSqlite_Table_Link(srAll, inLinkState, &srLink);
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Table_Link_EMVRec()_END");
                inLogPrintf(AT, "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_TagValue_ByInvoiceNumber_Flow
Date&Time       :2017/3/20 下午 5:57
Describe        :在這邊決定名稱並分流
*/
int inSqlite_Get_TagValue_ByInvoiceNumber_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inInvoiceNumber, char *szTagName, char *szTagValue)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		default :
			break;
	}
	
	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_EMV_TABLE_:
			inRetVal = inSqlite_Get_TagValue_ByInvoiceNumber(pobTran, szTableName, inInvoiceNumber, szTagName, szTagValue);
			break;
		default :
			return (VS_ERROR);
			break;
	}
	
	return (inRetVal);
}

/*
Function        :inSqlite_Get_TagValue_ByInvoiceNumber
Date&Time       :2016/4/11 下午 3:25
Describe        :利用調閱標號取得指定的Tag，輸入當筆invoiceNumber、Tag名稱的string，然後輸入一個陣列szTagValue來接值
*/
int inSqlite_Get_TagValue_ByInvoiceNumber(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber, char *szTagName, char *szTagValue)
{
	int		j = 0;
	int		inRetVal = VS_ERROR;
	int		inDataLen = 0;
	char		szDebugMsg[84 + 1];
	char		szQuerySql[_SQL_SENTENCE_MAX_ + 1];	/* INSERT INTO	szTableName( */
	sqlite3_stmt	*srSQLStat;
	sqlite3		*srDBConnection;	/* 建立到資料庫的connection */
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Get_TagValue_ByInvoiceNumber()_START");
        }
	
	inRetVal = inSqlite_DB_Open_Or_Create(gszTranDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "SELECT %s FROM %s WHERE lnOrgInvNum = %d", szTagName, szTableName, inInvoiceNumber);
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
			inLogPrintf(AT, szDebugMsg);
		}
			
	}
	
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inLogPrintf(AT, szDebugMsg);
		}
		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		inSqlite_DB_Close(&srDBConnection);
		
		return (VS_NO_RECORD);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
			inLogPrintf(AT, szDebugMsg);

			inLogPrintf(AT, szQuerySql);
		}
		
		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);

		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}

		return (VS_ERROR);
	}
	
	/* 若是成功，將值丟到輸入的位置。 */
	/* 應該只會抓到一列 */
	j = 0;
	
	inDataLen = sqlite3_column_bytes(srSQLStat, j);
	memcpy(szTagValue, sqlite3_column_blob(srSQLStat, j), inDataLen);
	
	/* 釋放事務 */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Get_TagValue_ByInvoiceNumber()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_Table_Flow
Date&Time       :2017/3/14 下午 3:27
Describe        :
*/
int inSqlite_Get_Table_Count_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int *inTableCount)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	if (pobTran->uszFileNameNoNeedNumBit == VS_TRUE)
	{
		inFunc_ComposeFileName(pobTran, szTableName, "", 0);
	}
	else
	{
		inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	}
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_TICKET_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_TEMP_:
			strcat(szTableName, _BATCH_TABLE_ESC_TEMP_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_ADVICE_:
			strcat(szTableName, _BATCH_TABLE_ESC_ADVICE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			strcat(szTableName, _BATCH_TABLE_ADVICE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_EMV_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_EMV_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_EMV_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_EMV_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_TEMP_EMV_:
			strcat(szTableName, _BATCH_TABLE_ESC_TEMP_EMV_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_ADVICE_EMV_:
			strcat(szTableName, _BATCH_TABLE_ESC_ADVICE_EMV_SUFFIX_);
			break;
		default :
			break;
	}
	
	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_BATCH_TABLE_ESC_TEMP_:
		case _TN_BATCH_TABLE_ESC_ADVICE_:
		case _TN_BATCH_TABLE_TICKET_:
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
		case _TN_EMV_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_EMV_:
		case _TN_BATCH_TABLE_ESC_FAIL_EMV_:
		case _TN_BATCH_TABLE_ESC_TEMP_EMV_:
		case _TN_BATCH_TABLE_ESC_ADVICE_EMV_:
			inRetVal = inSqlite_Get_Table_Count(gszTranDBPath, szTableName, inTableCount);
			break;
		default :
			return (VS_ERROR);
			break;
	}
	
	return (inRetVal);
}

/*
Function        :inSqlite_Get_Table_Count
Date&Time       :2016/4/29 下午 1:17
Describe        :可以取得table有幾筆資料
*/
int inSqlite_Get_Table_Count(char* szDBPath, char *szTableName, int *inTableCount)
{
	int		j = 0;
	int		inRetVal;
	char		szDebugMsg[128 + 1] = {0};
	char		szQuerySql[_SQL_SENTENCE_MAX_ + 1] = {0};	/* INSERT INTO	szTableName( */
	char		szErrorMessage[100 + 1] = {0};
	sqlite3		*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt	*srSQLStat;
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Get_Table_Count()_START");
        }
	
	inRetVal = inSqlite_DB_Open_Or_Create(szDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));

	/* SQLite COUNT 計算資料庫中 table的行數 */
	sprintf(szQuerySql, "SELECT count(*) FROM %s", szTableName);
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		inSqlite_DB_Close(&srDBConnection);
		
		return (VS_NO_RECORD);
	}
	else
	{
		memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
		strcpy(szErrorMessage, sqlite3_errmsg(srDBConnection));
			
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", szErrorMessage);
			inLogPrintf(AT, szDebugMsg);

			inLogPrintf(AT, szQuerySql);
		}
		
		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);

		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}

		/* 因為直接get 不存在的table回傳值是-1，只有在Error Msg才能得知錯誤原因 */
		if (memcmp(szErrorMessage, "no such table", strlen("no such table")) == 0)
		{
			return (VS_NO_RECORD);
		}
		else
		{
			return (VS_ERROR);
		}
	}
	
	/* 若是成功，將值丟到輸入的位置。 */
	j = 0;
	*inTableCount = sqlite3_column_int(srSQLStat, j);

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Get count(*) OK count:%d", *inTableCount);
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* 釋放事務 */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Get_Table_Count()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_Batch_ByInvNum
Date&Time       :2016/4/12 下午 2:53
Describe        :利用調閱標號來將該筆資料全塞回pobTran中的BRec、會取最新狀態(如取消、調帳)
*/
int inSqlite_Get_Batch_ByInvNum(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber)
{
	int			inRetVal = VS_SUCCESS;
	char			szDebugMsg[128 + 1];
	char			szQuerySql[_SQL_SENTENCE_MAX_ + 1];	/* INSERT INTO	szTableName( */
	char			szErrorMessage[100 + 1];
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	SQLITE_ALL_TABLE	srAll;
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Get_Batch_ByInvNum()_START");
        }
	
	inRetVal = inSqlite_DB_Open_Or_Create(gszTranDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inSqlite_Table_Link_BRec(pobTran, &srAll, _LS_READ_);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Link 失敗");
			inLogPrintf(AT, szDebugMsg);
		}
		inSqlite_DB_Close(&srDBConnection);
		
		return (VS_ERROR);
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	if (inInvoiceNumber > 0)
	{
		sprintf(szQuerySql, "SELECT * FROM %s WHERE lnOrgInvNum = %d ORDER BY inTableID DESC LIMIT 1", szTableName, inInvoiceNumber);
	}
	else if (inInvoiceNumber == _BATCH_LAST_RECORD_)
	{
		/* sqlite3_last_insert_rowid 只有在同一connection才有用 所以這邊的邏輯是最後一筆理論上invoiceNumber會最大 
		   若是同一筆，可能有調帳等操作，加上用max(inTableID)來判斷 */

		
		/* 再重新組查詢語句，把剛剛查到的invoiceNumber放進去 第一列第0行是所查的值 */
		memset(szQuerySql, 0x00, sizeof(szQuerySql));
		sprintf(szQuerySql, "SELECT * FROM %s WHERE lnOrgInvNum = (SELECT MAX(lnOrgInvNum) FROM %s) ORDER BY inTableID DESC LIMIT 1", szTableName, szTableName);
	}
	else
	{
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
	
		return (VS_ERROR);
	}
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inLogPrintf(AT, szDebugMsg);
		}
		/* 替換資料前先清空srBRec */
		memset(&pobTran->srBRec, 0x00, sizeof(pobTran->srBRec));
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		inSqlite_DB_Close(&srDBConnection);
		
		return (VS_NO_RECORD);
	}
	else
	{
		memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
		strcpy(szErrorMessage, sqlite3_errmsg(srDBConnection));
			
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", szErrorMessage);
			inLogPrintf(AT, szDebugMsg);

			inLogPrintf(AT, szQuerySql);
		}
		
		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);

		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}

		/* 因為直接get 不存在的table回傳值是-1，只有在Error Msg才能得知錯誤原因 */
		if (memcmp(szErrorMessage, "no such table", strlen("no such table")) == 0)
		{
			return (VS_NO_RECORD);
		}
		else
		{
			return (VS_ERROR);
		}
	}
	
	/* binding 取得值 */
	inSqlite_Get_Binding_Value(&srSQLStat, &srAll);
	
	/* 釋放事務 */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Get_Batch_ByInvNum()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_EMV_ByInvNum
Date&Time       :2017/3/20 下午 12:02
Describe        :利用調閱標號來將該筆資料全塞回pobTran中的BRec、會取最新狀態(如取消、調帳)
*/
int inSqlite_Get_EMV_ByInvNum(TRANSACTION_OBJECT *pobTran, char* szTableName)
{
	int			inRetVal = VS_SUCCESS;
	char			szDebugMsg[128 + 1];
	char			szQuerySql[_SQL_SENTENCE_MAX_ + 1];	/* INSERT INTO	szTableName( */
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	SQLITE_ALL_TABLE	srAll;
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Get_EMV_ByInvNum()_START");
        }
	
	inRetVal = inSqlite_DB_Open_Or_Create(gszTranDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inSqlite_Table_Link_EMVRec(pobTran, &srAll, _LS_READ_);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Link 失敗");
			inLogPrintf(AT, szDebugMsg);
		}
		inSqlite_DB_Close(&srDBConnection);
		
		return (VS_ERROR);
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	if (pobTran->inTableID >= 0)
	{
		sprintf(szQuerySql, "SELECT * FROM %s WHERE (inBatchTableID = %d)", szTableName, pobTran->inTableID);
	}
	else
	{
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
	
		return (VS_ERROR);
	}
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inLogPrintf(AT, szDebugMsg);
		}
		/* 替換資料前先清空srBRec */
		memset(&pobTran->srEMVRec, 0x00, sizeof(pobTran->srEMVRec));
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		inSqlite_DB_Close(&srDBConnection);
		
		return (VS_NO_RECORD);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
			inLogPrintf(AT, szDebugMsg);

			inLogPrintf(AT, szQuerySql);
		}

		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}

		return (VS_ERROR);
	}
	
	/* binding 取得值 */
	inSqlite_Get_Binding_Value(&srSQLStat, &srAll);
	
	/* 釋放事務 */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Get_EMV_ByInvNum()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Update_ByInvoiceNumber_All
Date&Time       :2016/4/13 上午 10:42
Describe        :利用調閱標號來更新資料庫中的所有欄位，注意:update keytag不能重複 這隻function暫時用不到
 *		因為要保存每次交易狀態，而不是只紀錄帳
*/
int inSqlite_Update_ByInvoiceNumber_All(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber)
{
	int			inRetVal = VS_SUCCESS;
	int			inSqlLength = 0;
	char			szDebugMsg[84 + 1];
	char			szSqlPrefix[100 + 1];	/* Update INTO	szTableName( */
	char			szSqlSuffix[40 + 1];	/* VALUES ( */
	char			szSqlSuffix2[200 + 1];	/* ); */
	char			*szUpdateSql;
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	SQLITE_ALL_TABLE	srAll;
	SQLITE_SQL_CALCULATE_TABLE	srSQLCal;
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Update_ByInvoiceNumber_All()_START");
        }
	
	pobTran->srBRec.lnOrgInvNum = inInvoiceNumber;
	
	inRetVal = inSqlite_DB_Open_Or_Create(gszTranDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inSqlite_Table_Link_BRec(pobTran, &srAll, _LS_UPDATE_);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Link 失敗");
			inLogPrintf(AT, szDebugMsg);
		}
		inSqlite_DB_Close(&srDBConnection);
		
		return (VS_ERROR);
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "UPDATE %s", szTableName);
	
	memset(szSqlSuffix, 0x00, sizeof(szSqlSuffix));
	sprintf(szSqlSuffix, " SET ");
	
	memset(szSqlSuffix2, 0x00, sizeof(szSqlSuffix2));
	sprintf(szSqlSuffix2, " WHERE (lnOrgInvNum = %d) AND (inTableID = (SELECT MAX(inTableID) FROM %s))", inInvoiceNumber, szTableName);
	
	/* 算要配置多少記憶體 */
	memset(&srSQLCal, 0x00, sizeof(SQLITE_SQL_CALCULATE_TABLE));
	srSQLCal.pSqlPrefix = szSqlPrefix;
	srSQLCal.pSqlSuffix = szSqlSuffix;
	srSQLCal.pSqlSuffix2 = szSqlSuffix2;
	
	inSqlite_Calculate_Update_SQLLength(&srSQLCal, &srAll, &inSqlLength);
	
	/* 配置記憶體 */
	szUpdateSql = malloc(inSqlLength + 1);
	memset(szUpdateSql, 0x00, inSqlLength);
	
	inSqlite_Gernerate_UpdateSQL(&srSQLCal, &srAll, szUpdateSql);
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szUpdateSql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	
	/* Binding變數 */
	inSqlite_Binding_Values(&srAll, &srSQLStat);
	
	/* Update */
	do
	{
		/* Update */
		inRetVal = sqlite3_step(srSQLStat);
		if (inRetVal == SQLITE_ROW	||
		    inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Update OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Update ERROR Num:%d", inRetVal);
				inLogPrintf(AT, szDebugMsg);
				
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
				inLogPrintf(AT, szDebugMsg);
				
				inLogPrintf(AT, szUpdateSql);
			}

		}
		
	}while (inRetVal == SQLITE_ROW);
	
	/* 釋放事務，若要重用則用sqlite3_reset */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	free(szUpdateSql);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Update_ByInvoiceNumber_All()_END");
                inLogPrintf(AT, "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Delete_Record_ByInvoiceNumber
Date&Time       :2016/4/11 下午 3:25
Describe        :利用調閱標號刪除Record
*/
int inSqlite_Delete_Record_ByInvoiceNumber(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber)
{
	int		inRetVal;
	char		szDebugMsg[84 + 1];
	char		szQuerySql[_SQL_SENTENCE_MAX_ + 1];	/* INSERT INTO	szTableName( */
	char		*szErrorMessage = NULL;
	sqlite3		*srDBConnection;	/* 建立到資料庫的connection */
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Delete_Record_ByInvoiceNumber()_START");
        }
	
	inRetVal = inSqlite_DB_Open_Or_Create(gszTranDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			inLogPrintf(AT, szQuerySql);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "DELETE FROM %s WHERE lnOrgInvNum = %d", szTableName, inInvoiceNumber);
	
	/* 取得 database 裡所有的資料 */
	inRetVal = sqlite3_exec(srDBConnection , szQuerySql, 0 , 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Delete Record ERROR Num:%d,", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason:%s", szErrorMessage);
			inLogPrintf(AT, szDebugMsg);
		}
		
		if (szErrorMessage != NULL)
		{
			sqlite3_free(szErrorMessage);
		}
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Delete Record OK");
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Delete_Record_ByInvoiceNumber()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inSqlite_ESC_Get_BRec_Top_Flow
Date&Time       :2017/3/14 下午 3:49
Describe        :
*/
int inSqlite_ESC_Get_BRec_Top_Flow(TRANSACTION_OBJECT *pobTran, int inTableType)
{
	int	inRetVal = VS_SUCCESS;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	if (pobTran->uszFileNameNoNeedNumBit == VS_TRUE)
	{
		inFunc_ComposeFileName(pobTran, szTableName, "", 0);
	}
	else
	{
		inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	}
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_TICKET_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_TEMP_:
			strcat(szTableName, _BATCH_TABLE_ESC_TEMP_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_ADVICE_:
			strcat(szTableName, _BATCH_TABLE_ESC_ADVICE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			strcat(szTableName, _BATCH_TABLE_ADVICE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_EMV_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_EMV_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_EMV_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_EMV_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_TEMP_EMV_:
			strcat(szTableName, _BATCH_TABLE_ESC_TEMP_EMV_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_ADVICE_EMV_:
			strcat(szTableName, _BATCH_TABLE_ESC_ADVICE_EMV_SUFFIX_);
			break;
		default :
			break;
	}
	
	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_BATCH_TABLE_ESC_TEMP_:
		case _TN_BATCH_TABLE_ESC_ADVICE_:
			inRetVal = inSqlite_ESC_Get_BRec_Top(pobTran, szTableName);
			break;
		case _TN_EMV_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_EMV_:
		case _TN_BATCH_TABLE_ESC_FAIL_EMV_:
		case _TN_BATCH_TABLE_ESC_TEMP_EMV_:
		case _TN_BATCH_TABLE_ESC_ADVICE_EMV_:
			inRetVal = inSqlite_ESC_Get_EMVRec_Top(pobTran, szTableName);
			break;
		case _TN_BATCH_TABLE_TICKET_:
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			inRetVal = VS_ERROR;
			break;
		default :
			return (VS_ERROR);
			break;
	}
	
	return (inRetVal);
}

/*
Function        :inSqlite_ESC_Get_BRec_Top
Date&Time       :2016/4/12 下午 2:53
Describe        :ESC會取table中Primary Key最小的值
*/
int inSqlite_ESC_Get_BRec_Top(TRANSACTION_OBJECT *pobTran, char* szTableName)
{
	int			inRetVal = VS_SUCCESS;
	char			szDebugMsg[128 + 1];
	char			szQuerySql[_SQL_SENTENCE_MAX_ + 1];	/* INSERT INTO	szTableName( */
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	SQLITE_ALL_TABLE	srAll;
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_ESC_Get_BRec_Top()_START");
        }
	
	inRetVal = inSqlite_DB_Open_Or_Create(gszTranDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
        if (inRetVal != SQLITE_OK)
	{
            return (VS_ERROR);
        }
    // 呼叫inSqlite_DB_Open_Or_Create() 已經有紀錄log，所以可以不顯示
    /*
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
    */	
	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inSqlite_Table_Link_BRec(pobTran, &srAll, _LS_READ_);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Link 失敗");
			inLogPrintf(AT, szDebugMsg);
		}
		inSqlite_DB_Close(&srDBConnection);
		
		return (VS_ERROR);
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "SELECT * FROM %s WHERE (inTableID = (SELECT MIN(inTableID) FROM %s))", szTableName, szTableName);

	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inLogPrintf(AT, szDebugMsg);
		}
		/* 替換資料前先清空srBRec */
		memset(&pobTran->srBRec, 0x00, sizeof(pobTran->srBRec));
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		inSqlite_DB_Close(&srDBConnection);
		
		return (VS_NO_RECORD);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
			inLogPrintf(AT, szDebugMsg);

			inLogPrintf(AT, szQuerySql);
		}

		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}

		return (VS_ERROR);
	}
	
	/* binding 取得值 */
	inSqlite_Get_Binding_Value(&srSQLStat, &srAll);
	
	/* 釋放事務 */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_ESC_Get_BRec_Top()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inSqlite_ESC_Get_EMVRec_Top
Date&Time       :2019/9/20 上午 11:55
Describe        :ESC會取table中Primary Key最小的值
*/
int inSqlite_ESC_Get_EMVRec_Top(TRANSACTION_OBJECT *pobTran, char* szTableName)
{
	int			inRetVal = VS_SUCCESS;
	char			szDebugMsg[128 + 1];
	char			szQuerySql[_SQL_SENTENCE_MAX_ + 1];	/* INSERT INTO	szTableName( */
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	SQLITE_ALL_TABLE	srAll;
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_ESC_Get_BRec_Top()_START");
        }
	
	inRetVal = inSqlite_DB_Open_Or_Create(gszTranDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inSqlite_Table_Link_EMVRec(pobTran, &srAll, _LS_READ_);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Link 失敗");
			inLogPrintf(AT, szDebugMsg);
		}
		inSqlite_DB_Close(&srDBConnection);
		
		return (VS_ERROR);
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	if (pobTran->inTableID >= 0)
	{
		sprintf(szQuerySql, "SELECT * FROM %s WHERE (inBatchTableID = %d)", szTableName, pobTran->inTableID);
	}
	else
	{
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
	
		return (VS_ERROR);
	}

	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inLogPrintf(AT, szDebugMsg);
		}
		/* 替換資料前先清空srBRec */
		memset(&pobTran->srEMVRec, 0x00, sizeof(pobTran->srEMVRec));
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inLogPrintf(AT, szDebugMsg);
		}
		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		inSqlite_DB_Close(&srDBConnection);
		
		return (VS_NO_RECORD);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
			inLogPrintf(AT, szDebugMsg);

			inLogPrintf(AT, szQuerySql);
		}

		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}

		return (VS_ERROR);
	}
	
	/* binding 取得值 */
	inSqlite_Get_Binding_Value(&srSQLStat, &srAll);
	
	/* 釋放事務 */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_ESC_Get_BRec_Top()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inSqlite_ESC_Delete_Record_Flow
Date&Time       :2017/3/14 下午 3:57
Describe        :
*/
int inSqlite_ESC_Delete_Record_Flow(TRANSACTION_OBJECT *pobTran, int inTableType)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	if (pobTran->uszFileNameNoNeedNumBit == VS_TRUE)
	{
		inFunc_ComposeFileName(pobTran, szTableName, "", 0);
	}
	else
	{
		inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	}
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_TICKET_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_TEMP_:
			strcat(szTableName, _BATCH_TABLE_ESC_TEMP_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_ADVICE_:
			strcat(szTableName, _BATCH_TABLE_ESC_ADVICE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			strcat(szTableName, _BATCH_TABLE_ADVICE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_EMV_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_EMV_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_EMV_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_EMV_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_TEMP_EMV_:
			strcat(szTableName, _BATCH_TABLE_ESC_TEMP_EMV_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_ADVICE_EMV_:
			strcat(szTableName, _BATCH_TABLE_ESC_ADVICE_EMV_SUFFIX_);
			break;
		default :
			break;
	}
	
	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_BATCH_TABLE_ESC_TEMP_:
		case _TN_BATCH_TABLE_ESC_ADVICE_:
		case _TN_BATCH_TABLE_TICKET_:
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
		case _TN_EMV_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_EMV_:
		case _TN_BATCH_TABLE_ESC_FAIL_EMV_:
		case _TN_BATCH_TABLE_ESC_TEMP_EMV_:
		case _TN_BATCH_TABLE_ESC_ADVICE_EMV_:
			inRetVal = inSqlite_ESC_Delete_Record(pobTran, szTableName);
			break;
		default :
			return (VS_ERROR);
			break;
	}
	
	/*[260318_BUG] 因使用原廠API不需再進行同步,移除檔案同步函式 */
	
	return (inRetVal);
}

/*
Function        :inSqlite_ESC_Delete_Record
Date&Time       :2016/4/11 下午 3:25
Describe        :ESC會刪除table中Primary Key最小的值
*/
int inSqlite_ESC_Delete_Record(TRANSACTION_OBJECT *pobTran, char* szTableName)
{
	int		inRetVal;
	char		szDebugMsg[84 + 1];
	char		szQuerySql[_SQL_SENTENCE_MAX_ + 1];	/* INSERT INTO	szTableName( */
	char		*szErrorMessage = NULL;
	sqlite3		*srDBConnection;	/* 建立到資料庫的connection */
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_ESC_Delete_Record()_START");
        }
	
	inRetVal = inSqlite_DB_Open_Or_Create(gszTranDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "DELETE FROM %s WHERE inTableID = (SELECT MIN(inTableID) FROM %s)", szTableName, szTableName);
	
	/* 取得 database 裡所有的資料 */
	inRetVal = sqlite3_exec(srDBConnection , szQuerySql, 0 , 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Delete Record ERROR Num:%d,", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason:%s", szErrorMessage);
			inLogPrintf(AT, szDebugMsg);
			inLogPrintf(AT, szQuerySql);
		}
		
		if (szErrorMessage != NULL)
		{
			sqlite3_free(szErrorMessage);
		}
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Delete Record OK");
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_ESC_Delete_Record()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_Max_TableID_Flow
Date&Time       :2017/3/15 下午 2:43
Describe        :
*/
int inSqlite_Get_Max_TableID_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, char *szTagValue)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_EMV_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_EMV_SUFFIX_);
			break;	
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_EMV_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_EMV_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_ADVICE_:
			strcat(szTableName, _BATCH_TABLE_ESC_ADVICE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_ADVICE_EMV_:
			strcat(szTableName, _BATCH_TABLE_ESC_ADVICE_EMV_SUFFIX_);
			break;
		default :
			break;
	}
	
	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_BATCH_TABLE_ESC_ADVICE_:
		case _TN_EMV_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_EMV_:
		case _TN_BATCH_TABLE_ESC_FAIL_EMV_:
		case _TN_BATCH_TABLE_ESC_ADVICE_EMV_:
			inRetVal = inSqlite_Get_Max_TableID(gszTranDBPath, szTableName, szTagValue);
			break;
		default :
			return (VS_ERROR);
			break;
	}
	
	return (inRetVal);
}

/*
Function        :inSqlite_Get_Max_TableID
Date&Time       :2017/3/15 下午 1:29
Describe        :抓最大的Primary Key值
*/
int inSqlite_Get_Max_TableID(char* szDBPath, char* szTableName, char *szTagValue)
{
	int		j = 0;
	int		inDataLen = 0;
	int		inRetVal = VS_ERROR;
	char		szDebugMsg[84 + 1];
	char		szQuerySql[_SQL_SENTENCE_MAX_ + 1];	/* INSERT INTO	szTableName( */
	sqlite3		*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt	*srSQLStat;
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Get_Max_TableID()_START");
        }
	
	inRetVal = inSqlite_DB_Open_Or_Create(szDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "SELECT MAX(%s) FROM %s", "inTableID", szTableName);
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		inSqlite_DB_Close(&srDBConnection);
		
		return (VS_NO_RECORD);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
			inLogPrintf(AT, szDebugMsg);

			inLogPrintf(AT, szQuerySql);
		}

		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}

		return (VS_ERROR);
	}
	
	/* 若是成功，將值丟到輸入的位置。 */
	j = 0;
	inDataLen = sqlite3_column_bytes(srSQLStat, j);
	memcpy(szTagValue, sqlite3_column_blob(srSQLStat, j), inDataLen);
	
	/* 釋放事務 */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Get_Max_TableID()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Update_ByInvoiceNumber_TranState_Flow
Date&Time       :2017/3/15 下午 2:43
Describe        :用來更新批次狀態時，把舊的紀錄作廢
*/
int inSqlite_Update_ByInvNum_TranState_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inInvoiceNumber)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_TICKET_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		default :
			break;
	}
	
	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_EMV_TABLE_:
			inRetVal = inSqlite_Update_ByInvNum_TranState(pobTran, szTableName, inInvoiceNumber);
			break;
		case _TN_BATCH_TABLE_TICKET_:
			inRetVal = inSqlite_Update_ByInvNum_TranState_Ticket(pobTran, szTableName, inInvoiceNumber);
			break;
		default :
			return (VS_ERROR);
			break;
	}
	
	return (inRetVal);
}

/*
Function        :inSqlite_Update_ByInvNum_TranState
Date&Time       :2017/3/15 下午 2:46
Describe        :UPDATE table_name SET column1 = value1, column2 = value2...., columnN = valueN WHERE [condition];
 *
*/
int inSqlite_Update_ByInvNum_TranState(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber)
{
	int			inRetVal;
	int			inSqlLength = 0;
	char			szDebugMsg[84 + 1];
	char			szSqlPrefix[100 + 1];	/* Update INTO	szTableName( */
	char			szSqlSuffix[40 + 1];	/* VALUES ( */
	char			szSqlSuffix2[200 + 1];	/* ); */
	char			szTag[_TAG_WIDTH_ + 1];
	char			szTagValue[2 + 1];
	char			*szUpdateSql;
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	SQLITE_ALL_TABLE	srAll;
	SQLITE_SQL_CALCULATE_TABLE	srSQLCal;
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Update_ByInvNum_TranState()_START");
        }
	
	inRetVal = inSqlite_DB_Open_Or_Create(gszTranDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "UPDATE %s", szTableName);
	
	memset(szSqlSuffix, 0x00, sizeof(szSqlSuffix));
	sprintf(szSqlSuffix, " SET ");
	
	memset(szSqlSuffix2, 0x00, sizeof(szSqlSuffix2));
	sprintf(szSqlSuffix2, " WHERE inTableID IN (SELECT inTableID FROM %s WHERE lnOrgInvNum = %d ORDER BY inTableID DESC LIMIT 1)", szTableName, inInvoiceNumber);
	
	memset(szTag, 0x00, sizeof(szTag));
	sprintf(szTag, "uszUpdated");
	
	memset(szTagValue, 0x00, sizeof(szTagValue));
	sprintf(szTagValue, "1");
	
	memset(&srAll, 0x00, sizeof(SQLITE_ALL_TABLE));
	srAll.inCharNum = 1;
	strcpy(srAll.srChar[0].szTag, szTag);
	srAll.srChar[0].pCharVariable = szTagValue;
	srAll.srChar[0].inTagValueLen = strlen(srAll.srChar[0].pCharVariable);
	
	/* 算要配置多少記憶體 */
	memset(&srSQLCal, 0x00, sizeof(SQLITE_SQL_CALCULATE_TABLE));
	srSQLCal.pSqlPrefix = szSqlPrefix;
	srSQLCal.pSqlSuffix = szSqlSuffix;
	srSQLCal.pSqlSuffix2 = szSqlSuffix2;
	
	inSqlite_Calculate_Update_SQLLength(&srSQLCal, &srAll, &inSqlLength);
	
	/* 配置記憶體 */
	szUpdateSql = malloc(inSqlLength + 1);
	memset(szUpdateSql, 0x00, inSqlLength);
	
	inSqlite_Gernerate_UpdateSQL(&srSQLCal, &srAll, szUpdateSql);
	
	/* Update */
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szUpdateSql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* Binding變數 */
	inSqlite_Binding_Values(&srAll, &srSQLStat);
	
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	do
	{
		inRetVal = sqlite3_step(srSQLStat);
		if (inRetVal == SQLITE_ROW)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "UPDATE Table OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else if (inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "UPDATE Table OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "UPDATE Table ERROR Num:%d", inRetVal);
				inLogPrintf(AT, szDebugMsg);

				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
				inLogPrintf(AT, szDebugMsg);

				inLogPrintf(AT, szUpdateSql);
			}

			/* 釋放事務 */
			inSqlite_SQL_Finalize(&srSQLStat);
			
			free(szUpdateSql);
			
			/* 關閉 database, close null pointer 是NOP(No Operation) */
			if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
			{
				/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
				return (VS_ERROR);
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "Close Database OK");
					inLogPrintf(AT, szDebugMsg);
				}
			}

			return (VS_ERROR);
		}
	
	}while (inRetVal != SQLITE_DONE);
	
	/* 釋放事務 */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	free(szUpdateSql);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Update_ByInvNum_TranState()_END");
                inLogPrintf(AT, "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Update_ByInvNum_TranState_Ticket
Date&Time       :2018/1/12 下午 5:05
Describe        :UPDATE table_name SET column1 = value1, column2 = value2...., columnN = valueN WHERE [condition];
 *		 用於更新batch時，需把上一筆紀錄作廢
 *
*/
int inSqlite_Update_ByInvNum_TranState_Ticket(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber)
{
	int			inRetVal;
	int			inSqlLength = 0;
	char			szDebugMsg[84 + 1];
	char			szSqlPrefix[100 + 1];	/* Update INTO	szTableName( */
	char			szSqlSuffix[40 + 1];	/* VALUES ( */
	char			szSqlSuffix2[200 + 1];	/* ); */
	char			szTag[_TAG_WIDTH_ + 1];
	char			szTagValue[2 + 1];
	char			*szUpdateSql;
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	SQLITE_ALL_TABLE	srAll;
	SQLITE_SQL_CALCULATE_TABLE	srSQLCal;
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Update_ByInvNum_TranState_Ticket()_START");
        }
	
	inRetVal = inSqlite_DB_Open_Or_Create(gszTranDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "UPDATE %s", szTableName);
	
	memset(szSqlSuffix, 0x00, sizeof(szSqlSuffix));
	sprintf(szSqlSuffix, " SET ");
	
	memset(szSqlSuffix2, 0x00, sizeof(szSqlSuffix2));
	sprintf(szSqlSuffix2, " WHERE inTableID IN (SELECT inTableID FROM %s WHERE lnInvNum = %d ORDER BY inTableID DESC LIMIT 1)", szTableName, inInvoiceNumber);
	
	memset(szTag, 0x00, sizeof(szTag));
	sprintf(szTag, "uszUpdated");
	
	memset(szTagValue, 0x00, sizeof(szTagValue));
	sprintf(szTagValue, "1");
	
	memset(&srAll, 0x00, sizeof(SQLITE_ALL_TABLE));
	srAll.inCharNum = 1;
	strcpy(srAll.srChar[0].szTag, szTag);
	srAll.srChar[0].pCharVariable = szTagValue;
	srAll.srChar[0].inTagValueLen = strlen(srAll.srChar[0].pCharVariable);
	
	/* 算要配置多少記憶體 */
	memset(&srSQLCal, 0x00, sizeof(SQLITE_SQL_CALCULATE_TABLE));
	srSQLCal.pSqlPrefix = szSqlPrefix;
	srSQLCal.pSqlSuffix = szSqlSuffix;
	srSQLCal.pSqlSuffix2 = szSqlSuffix2;
	
	inSqlite_Calculate_Update_SQLLength(&srSQLCal, &srAll, &inSqlLength);
	
	/* 配置記憶體 */
	szUpdateSql = malloc(inSqlLength + 1);
	memset(szUpdateSql, 0x00, inSqlLength);
	
	inSqlite_Gernerate_UpdateSQL(&srSQLCal, &srAll, szUpdateSql);
	
	/* Update */
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szUpdateSql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* Binding變數 */
	inSqlite_Binding_Values(&srAll, &srSQLStat);
	
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	do
	{
		inRetVal = sqlite3_step(srSQLStat);
		if (inRetVal == SQLITE_ROW)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "UPDATE Table OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else if (inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "UPDATE Table OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "UPDATE Table ERROR Num:%d", inRetVal);
				inLogPrintf(AT, szDebugMsg);

				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
				inLogPrintf(AT, szDebugMsg);

				inLogPrintf(AT, szUpdateSql);
			}

			/* 釋放事務 */
			inSqlite_SQL_Finalize(&srSQLStat);
			
			free(szUpdateSql);
			
			/* 關閉 database, close null pointer 是NOP(No Operation) */
			if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
			{
				/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
				return (VS_ERROR);
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "Close Database OK");
					inLogPrintf(AT, szDebugMsg);
				}
			}

			return (VS_ERROR);
		}
	
	}while (inRetVal != SQLITE_DONE);
	
	/* 釋放事務 */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	free(szUpdateSql);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Update_ByInvNum_TranState_Ticket()_END");
                inLogPrintf(AT, "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Update_ByInvoiceNumber_CLS_SettleBit_Flow
Date&Time       :2017/3/15 下午 2:43
Describe        :
*/
int inSqlite_Update_ByInvNum_CLS_SettleBit_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inInvoiceNumber)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		default :
			break;
	}
	
	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_EMV_TABLE_:
			inRetVal = inSqlite_Update_ByInvNum_CLS_SettleBit(pobTran, szTableName, inInvoiceNumber);
			break;
		default :
			return (VS_ERROR);
			break;
	}
	
	return (inRetVal);
}

/*
Function        :inSqlite_Update_ByInvNum_CLS_SettleBit
Date&Time       :2017/3/15 下午 2:46
Describe        :UPDATE table_name SET column1 = value1, column2 = value2...., columnN = valueN WHERE [condition];
 *
*/
int inSqlite_Update_ByInvNum_CLS_SettleBit(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber)
{
	int			inRetVal;
	int			inSqlLength = 0;
	char			szDebugMsg[500 + 1];
	char			szSqlPrefix[100 + 1];	/* Update INTO	szTableName( */
	char			szSqlSuffix[40 + 1];	/* VALUES ( */
	char			szSqlSuffix2[200 + 1];	/* ); */
	char			szTag[_TAG_WIDTH_ + 1];
	char			szTagValue[10 + 1];
	char			*szUpdateSql;
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	SQLITE_ALL_TABLE	srAll;
	SQLITE_SQL_CALCULATE_TABLE	srSQLCal;
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Update_ByInvNum_CLS_SettleBit()_START");
        }
	
	pobTran->srBRec.lnOrgInvNum = inInvoiceNumber;
	
	inRetVal = inSqlite_DB_Open_Or_Create(gszTranDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "UPDATE %s", szTableName);
	
	memset(szSqlSuffix, 0x00, sizeof(szSqlSuffix));
	sprintf(szSqlSuffix, " SET ");
	
	memset(szSqlSuffix2, 0x00, sizeof(szSqlSuffix2));
	sprintf(szSqlSuffix2, " WHERE inTableID IN (SELECT inTableID FROM %s WHERE lnOrgInvNum = %d ORDER BY inTableID DESC LIMIT 1)", szTableName, inInvoiceNumber);
	
	memset(szTag, 0x00, sizeof(szTag));
	sprintf(szTag, "uszCLSBatchBit");
	
	memset(szTagValue, 0x00, sizeof(szTagValue));
	sprintf(szTagValue, "\x01");
	
	memset(&srAll, 0x00, sizeof(SQLITE_ALL_TABLE));
	srAll.inCharNum = 1;
	strcpy(srAll.srChar[0].szTag, szTag);
	srAll.srChar[0].pCharVariable = szTagValue;
	srAll.srChar[0].inTagValueLen = strlen(srAll.srChar[0].pCharVariable);
	
	/* 算要配置多少記憶體 */
	memset(&srSQLCal, 0x00, sizeof(SQLITE_SQL_CALCULATE_TABLE));
	srSQLCal.pSqlPrefix = szSqlPrefix;
	srSQLCal.pSqlSuffix = szSqlSuffix;
	srSQLCal.pSqlSuffix2 = szSqlSuffix2;
	
	inSqlite_Calculate_Update_SQLLength(&srSQLCal, &srAll, &inSqlLength);
	
	/* 配置記憶體 */
	szUpdateSql = malloc(inSqlLength + 1);
	memset(szUpdateSql, 0x00, inSqlLength);
	
	inSqlite_Gernerate_UpdateSQL(&srSQLCal, &srAll, szUpdateSql);
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szUpdateSql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	
	/* Binding變數 */
	inSqlite_Binding_Values(&srAll, &srSQLStat);
	
	/* Update */
	do
	{
		/* Update */
		inRetVal = sqlite3_step(srSQLStat);
		if (inRetVal == SQLITE_ROW	||
		    inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Update OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Update ERROR Num:%d", inRetVal);
				inLogPrintf(AT, szDebugMsg);
				
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
				inLogPrintf(AT, szDebugMsg);
				
				inLogPrintf(AT, szUpdateSql);
			}

		}
		
	}while (inRetVal == SQLITE_ROW);
	
	/* 釋放事務，若要重用則用sqlite3_reset */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	free(szUpdateSql);
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Update_ByInvNum_CLS_SettleBit()_END");
                inLogPrintf(AT, "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_Batch_ByCnt_Flow
Date&Time       :2017/3/15 下午 4:03
Describe        :
*/
int inSqlite_Get_Batch_ByCnt_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inRecCnt)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		default :
			break;
	}
	
	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
			inRetVal = inSqlite_Get_Batch_ByCnt(pobTran, szTableName, inRecCnt);
			break;
		case _TN_EMV_TABLE_:
			inRetVal = inSqlite_Get_EMV_ByCnt(pobTran, szTableName);
			break;
		default :
			return (VS_ERROR);
			break;
	}
	
	return (inRetVal);
}

/*
Function        :inSqlite_Get_Batch_ByCnt
Date&Time       :2017/3/15 下午 4:02
Describe        :利用調閱標號來將該筆資料全塞回pobTran中的BRec、會取最新狀態(如取消、調帳)，只抓最後一筆，
 *		因為原本API inRecCnt當成offset，所以這邊做成一樣。
*/
int inSqlite_Get_Batch_ByCnt(TRANSACTION_OBJECT *pobTran, char* szTableName, int inRecCnt)
{
	int			i = 0;
	int			inRetVal = VS_SUCCESS;
	char			szDebugMsg[128 + 1];
	char			szQuerySql[_SQL_SENTENCE_MAX_ + 1];	/* INSERT INTO	szTableName( */
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	SQLITE_ALL_TABLE	srAll;
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Get_Batch_ByCnt()_START");
        }
	
	inRetVal = inSqlite_DB_Open_Or_Create(gszTranDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inSqlite_Table_Link_BRec(pobTran, &srAll, _LS_READ_);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Link 失敗");
			inLogPrintf(AT, szDebugMsg);
		}		
		inSqlite_DB_Close(&srDBConnection);
		
		return (VS_ERROR);
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "SELECT * FROM %s WHERE (uszUpdated = 0)", szTableName);

	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	
	/* 取得 database 裡所有的資料 */
	
	/* 跳到要選的那一筆 */
	for (i = 0; i < inRecCnt; i++)
	{
		inRetVal = sqlite3_step(srSQLStat);
		if (inRetVal == SQLITE_ROW)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Get Table OK");
				inLogPrintf(AT, szDebugMsg);
			}

		}
		else if (inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "NO DATA");
				inLogPrintf(AT, szDebugMsg);
			}
			/* 釋放事務 */
			inSqlite_SQL_Finalize(&srSQLStat);
			inSqlite_DB_Close(&srDBConnection);
			
			return (VS_NO_RECORD);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
				inLogPrintf(AT, szDebugMsg);

				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
				inLogPrintf(AT, szDebugMsg);

				inLogPrintf(AT, szQuerySql);
			}
			
			/* 釋放事務 */
			inSqlite_SQL_Finalize(&srSQLStat);
			
			/* 關閉 database, close null pointer 是NOP(No Operation) */
			if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
			{
				/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
				return (VS_ERROR);
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "Close Database OK");
					inLogPrintf(AT, szDebugMsg);
				}
			}

			return (VS_ERROR);
		}
	}
	/* 替換資料前先清空srBRec */
	memset(&pobTran->srBRec, 0x00, sizeof(pobTran->srBRec));
	
	/* binding 取得值 */
	inSqlite_Get_Binding_Value(&srSQLStat, &srAll);
	
	/* 釋放事務 */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Get_Batch_ByCnt()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_EMV_ByCnt
Date&Time       :2017/3/20 下午 3:10
Describe        :利用調閱標號來將該筆資料全塞回pobTran中的BRec、會取最新狀態(如取消、調帳)，只抓最後一筆，
 *		因為原本API inRecCnt當成offset，所以這邊做成一樣。
*/
int inSqlite_Get_EMV_ByCnt(TRANSACTION_OBJECT *pobTran, char* szTableName)
{
	int			inRetVal = VS_SUCCESS;
	char			szDebugMsg[128 + 1];
	char			szQuerySql[_SQL_SENTENCE_MAX_ + 1];	/* INSERT INTO	szTableName( */
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	SQLITE_ALL_TABLE	srAll;
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Get_EMV_ByCnt()_START");
        }
	
	inRetVal = inSqlite_DB_Open_Or_Create(gszTranDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inSqlite_Table_Link_EMVRec(pobTran, &srAll, _LS_READ_);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Link 失敗");
			inLogPrintf(AT, szDebugMsg);
		}		
		inSqlite_DB_Close(&srDBConnection);

		return (VS_ERROR);
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	if (pobTran->inTableID >= 0)
	{
		sprintf(szQuerySql, "SELECT * FROM %s WHERE (inBatchTableID = %d)", szTableName, pobTran->inTableID);
	}
	else
	{
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
	
		return (VS_ERROR);
	}
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inLogPrintf(AT, szDebugMsg);
		}
		/* 替換資料前先清空srBRec */
		memset(&pobTran->srEMVRec, 0x00, sizeof(pobTran->srEMVRec));
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		inSqlite_DB_Close(&srDBConnection);
		
		return (VS_NO_RECORD);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
			inLogPrintf(AT, szDebugMsg);

			inLogPrintf(AT, szQuerySql);
		}

		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}

		return (VS_ERROR);
	}
	
	/* binding 取得值 */
	inSqlite_Get_Binding_Value(&srSQLStat, &srAll);

	/* 釋放事務 */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Get_EMV_ByCnt()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_EMV_ByCnt_Enormous_Get
Date&Time       :2017/3/21 下午 1:53
Describe        :
*/
int inSqlite_Get_EMV_ByCnt_Enormous_Get(TRANSACTION_OBJECT *pobTran, char* szTableName, int inRecCnt)
{
	int			inRetVal = 0;
	char			szDebugMsg[100 + 1] = {0};
	SQLITE_ALL_TABLE	srAll;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inSqlite_Get_EMV_ByCnt_Enormous_Get() START !");
	}
	
	/* 延續使用，不重新搜尋 */
	if (guszEnormousNoNeedResetBit == VS_TRUE)
	{
		
	}
	else
	{
		/* 重置前一次結果 */
		sqlite3_reset(gsrSQLStat);
		ginEnormousNowCnt = 0;
	}
	
	/* 取得 database 裡所有的資料 */
	for (; ginEnormousNowCnt <= inRecCnt; ginEnormousNowCnt++)
	{
		/* Qerry */
		inRetVal = sqlite3_step(gsrSQLStat);
		if (inRetVal == SQLITE_ROW)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Get Table OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else if (inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "NO DATA");
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 釋放事務 */
			inSqlite_SQL_Finalize(&gsrSQLStat);
			inSqlite_DB_Close(&gsrDBConnection);
			return (VS_NO_RECORD);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
				inLogPrintf(AT, szDebugMsg);

				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
				inLogPrintf(AT, szDebugMsg);
			}

			/* 釋放事務 */
			inSqlite_SQL_Finalize(&gsrSQLStat);
			/* 關閉 database, close null pointer 是NOP(No Operation) */
			if (inSqlite_DB_Close(&gsrDBConnection) != VS_SUCCESS)
			{
				/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
				return (VS_ERROR);
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "Close Database OK");
					inLogPrintf(AT, szDebugMsg);
				}
			}

			return (VS_ERROR);
		}
	}
	
	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inSqlite_Table_Link_EMVRec(pobTran, &srAll, _LS_READ_);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Link 失敗");
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	
	/* binding 取得值 */
	inSqlite_Get_Binding_Value(&gsrSQLStat, &srAll);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inSqlite_Get_EMV_ByCnt_Enormous_Get() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_Batch_Count_Flow
Date&Time       :2017/3/14 下午 3:27
Describe        :這邊是抓有效交易紀錄的筆數
*/
int inSqlite_Get_Batch_Count_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int *inTableCount)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_TICKET_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_ADVICE_:
			strcat(szTableName, _BATCH_TABLE_ESC_ADVICE_SUFFIX_);
			break;
		default :
			break;
	}
	
	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_EMV_TABLE_:
		case _TN_BATCH_TABLE_TICKET_:
		case _TN_BATCH_TABLE_ESC_ADVICE_:
			inRetVal = inSqlite_Get_Batch_Count(pobTran, szTableName, inTableCount);
			break;
		default :
			return (VS_ERROR);
			break;
	}
	
	return (inRetVal);
}

/*
Function        :inSqlite_Get_Batch_Count
Date&Time       :2017/3/15 下午 4:36
Describe        :這邊是抓有效交易紀錄的筆數
*/
int inSqlite_Get_Batch_Count(TRANSACTION_OBJECT *pobTran, char *szTableName, int *inTableCount)
{
	int		j = 0;
	int		inRetVal;
	char		szDebugMsg[128 + 1];
	char		szQuerySql[_SQL_SENTENCE_MAX_ + 1];	/* INSERT INTO	szTableName( */
	char		szErrorMessage[100 + 1];
	sqlite3		*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt	*srSQLStat;
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Get_Table_Count()_START");
        }
	
	inRetVal = inSqlite_DB_Open_Or_Create(gszTranDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	
	/* SQLite COUNT 計算資料庫中 table的行數 */
	sprintf(szQuerySql, "SELECT count(*) FROM %s WHERE (uszUpdated = 0)", szTableName);
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inLogPrintf(AT, szDebugMsg);
		}
		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		inSqlite_DB_Close(&srDBConnection);

		return (VS_NO_RECORD);
	}
	else
	{
		memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
		strcpy(szErrorMessage, sqlite3_errmsg(srDBConnection));
			
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", szErrorMessage);
			inLogPrintf(AT, szDebugMsg);

			inLogPrintf(AT, szQuerySql);
		}

		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}

		/* 因為直接get 不存在的table回傳值是-1，只有在Error Msg才能得知錯誤原因 */
		if (memcmp(szErrorMessage, "no such table", strlen("no such table")) == 0)
		{
			return (VS_NO_RECORD);
		}
		else
		{
			return (VS_ERROR);
		}
	}
	
	/* 若是成功，將值丟到輸入的位置。 */
	j = 0;
	*inTableCount =sqlite3_column_int(srSQLStat, j);

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Get count(*) OK count:%d", *inTableCount);
		inLogPrintf(AT, szDebugMsg);
	}

	/* 釋放事務 */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Get_Table_Count()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_Batch_Upload_Count_Flow
Date&Time       :2017/3/14 下午 3:27
Describe        :這邊是抓BathcUploaf有效交易紀錄的筆數
*/
int inSqlite_Get_Batch_Upload_Count_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int *inTableCount)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		default :
			break;
	}
	
	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_EMV_TABLE_:
			inRetVal = inSqlite_Get_Batch_Upload_Count(pobTran, szTableName, inTableCount);
			break;
		default :
			return (VS_ERROR);
			break;
	}
	
	return (inRetVal);
}

/*
Function        :inSqlite_Get_Batch_Upload_Count
Date&Time       :2017/3/15 下午 4:36
Describe        :這邊是抓有效交易紀錄的筆數
*/
int inSqlite_Get_Batch_Upload_Count(TRANSACTION_OBJECT *pobTran, char *szTableName, int *inTableCount)
{
	int		inBindingIndex = 1;
	int		inCols;
	int		inRetVal;
	char		szDebugMsg[128 + 1];
	char		szQuerySql[_SQL_SENTENCE_MAX_ + 1];	/* INSERT INTO	szTableName( */
	sqlite3_stmt	*srSQLStat;
	sqlite3		*srDBConnection;	/* 建立到資料庫的connection */
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Get_Batch_Upload_Count()_START");
        }
	
	inRetVal = inSqlite_DB_Open_Or_Create(gszTranDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));

	/* SQLite COUNT 計算資料庫中 table的行數 */
	/* 注意：X'01'這個寫法相當於0x01 */
	/* 注意：但X'00' 不等於0x00 */
	sprintf(szQuerySql, "SELECT count(*) FROM %s WHERE (uszUpdated = 0) "
		"AND (uszCLSBatchBit <> X'01') "
		"AND (uszVOIDBit <> X'01') "		
		"AND (inCode <> ?) "
		"AND (inCode <> ?) "
		"AND (inCode <> ?) "
		"AND (inCode <> ?)", szTableName);
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	
	/* index 從1開始 */
	inBindingIndex = 1;
	inRetVal = sqlite3_bind_int(srSQLStat, inBindingIndex, _PRE_AUTH_);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Binging Int Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	else
	{
		inBindingIndex++;
	}
	
	inRetVal = sqlite3_bind_int(srSQLStat, inBindingIndex, _CUP_PRE_AUTH_);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Binging Int Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	else
	{
		inBindingIndex++;
	}
	
	inRetVal = sqlite3_bind_int(srSQLStat, inBindingIndex, _LOYALTY_REDEEM_);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Binging Int Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	else
	{
		inBindingIndex++;
	}
	
	inRetVal = sqlite3_bind_int(srSQLStat, inBindingIndex, _VOID_LOYALTY_REDEEM_);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Binging Int Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	else
	{
		inBindingIndex++;
	}
	
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inLogPrintf(AT, szDebugMsg);
		}
		/* 替換資料前先清空srBRec */
		memset(&pobTran->srEMVRec, 0x00, sizeof(pobTran->srEMVRec));
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		inSqlite_DB_Close(&srDBConnection);
		
		return (VS_NO_RECORD);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
			inLogPrintf(AT, szDebugMsg);

			inLogPrintf(AT, szQuerySql);
		}

		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}

		return (VS_ERROR);
	}
	
	inCols = sqlite3_column_count(srSQLStat);
	
	/* 若是成功，將值丟到輸入的位置。 */
	/* The leftmost column of the result set has the index 0.*/
	*inTableCount = sqlite3_column_int(srSQLStat, 0);

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Get count(*) OK count:%d", *inTableCount);
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* 釋放事務，若要重用則用sqlite3_reset */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Get_Batch_Upload_Count()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Check_Table_Exist_Flow
Date&Time       :2017/3/17 上午 9:36
Describe        :By query "SELECT name FROM sqlite_master WHERE type='table' AND name='table_name';" 確認Table是否存在
*/
int inSqlite_Check_Table_Exist_Flow(TRANSACTION_OBJECT *pobTran, int inTableType)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_TRUST_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_TICKET_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		default :
			break;
	}
	
	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_EMV_TABLE_:
		case _TN_BATCH_TABLE_TICKET_:
		case _TN_BATCH_TABLE_TRUST_:
			inRetVal = inSqlite_Check_Table_Exist(gszTranDBPath, szTableName);
			break;
		default :
			return (VS_ERROR);
			break;
	}
	
	return (inRetVal);
}

/*
Function        :inSqlite_Check_Table_Exist
Date&Time       :2017/3/17 上午 10:00
Describe        :By query "SELECT name FROM sqlite_master WHERE type='table' AND name='table_name';" 確認Table是否存在
 *		sqlite_master 是 Sqlite的系統表，裡面存所有表的名稱
 *		需要另外注意的是暫存表存在sqlite_temp_master
*/
int inSqlite_Check_Table_Exist(char* szDBPath, char *szTableName)
{
	int		inRetVal = VS_ERROR;
	char		szDebugMsg[128 + 1];
	char		szQuerySql[_SQL_SENTENCE_MAX_ + 1];	/* INSERT INTO	szTableName( */
	char		szErrorMessage[100 + 1];
	sqlite3		*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt	*srSQLStat;
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Check_Table_Exist()_START");
        }
	
	inRetVal = inSqlite_DB_Open_Or_Create(szDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));

	/* SQLite COUNT 計算資料庫中 table的行數 */
	sprintf(szQuerySql, "SELECT name FROM sqlite_master WHERE type = 'table' AND name = '%s'", szTableName);
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Exist : %s", szTableName);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Not Exist : %s", szTableName);
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		inSqlite_DB_Close(&srDBConnection);
		
		return (VS_NO_RECORD);
	}
	else
	{
		memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
		strcpy(szErrorMessage, sqlite3_errmsg(srDBConnection));
			
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", szErrorMessage);
			inLogPrintf(AT, szDebugMsg);

			inLogPrintf(AT, szQuerySql);
		}

		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}

		/* 因為直接get 不存在的table回傳值是-1，只有在Error Msg才能得知錯誤原因 */
		if (memcmp(szErrorMessage, "no such table", strlen("no such table")) == 0)
		{
			return (VS_NO_RECORD);
		}
		else
		{
			return (VS_ERROR);
		}
	}
	
	/* 釋放事務 */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Check_Table_Exist()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Vacuum_Flow
Date&Time       :2017/3/17 下午 3:31
Describe        :Vacuum
*/
int inSqlite_Vacuum_Flow(TRANSACTION_OBJECT *pobTran, int inTableType)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		default :
			break;
	}
	
	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_EMV_TABLE_:
			inRetVal = inSqlite_Vacuum(pobTran, szTableName);
			break;
		default :
			return (VS_ERROR);
			break;
	}
	
	return (inRetVal);
}

/*
Function        :inSqlite_Vacuum
Date&Time       :2017/3/17 下午 3:30
Describe        :Vacuum 指令不只可以對Table使用，也能對資料庫使用
*/
int inSqlite_Vacuum(TRANSACTION_OBJECT *pobTran, char* szTableName1)
{
	int		inRetVal;
	int		inSqlLength = 0;
	char		szSqlPrefix[40 + 2];		/* CREATE TABLE	szTableName( */
	char		szDebugMsg[100 + 1];
	char		*szVacummSql;			/* 因為會很長且隨table浮動，所以用pointer */
	char		*szErrorMessage = NULL;
	sqlite3		*srDBConnection;		/* 建立到資料庫的connection */
		
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Vacuum()_START");
        }
	
	/* 開啟DataBase檔 */
	inRetVal = inSqlite_DB_Open_Or_Create(gszTranDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database ERROR Num%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}

	/* 塞入SQL語句 */
	/* 為了使table name可變動，所以拉出來組 */
	/* 加入了if not exists字串，若已建立不會重複建立 */
	/* 前綴 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "VACUUM %s;", szTableName1);
	/*  "CREATE TABLE	szTableName("的長度 */
	inSqlLength += strlen(szSqlPrefix);
	        
	/* inSqlLength: */
	if (ginDebug == VS_TRUE) 
        {
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inSqlLength: %d", inSqlLength);
		inLogPrintf(AT, szDebugMsg);
	}
	/* 配置記憶體(一定要+1，超級重要，不然會overflow) */
	szVacummSql = malloc(inSqlLength + 1);
	memset(szVacummSql, 0x00, inSqlLength);
	
	/* 先丟前綴Table Name */
	strcat(szVacummSql, szSqlPrefix);
	
	/* 建立 Table */
	inRetVal = sqlite3_exec(srDBConnection, szVacummSql, 0, 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Vacuum Table ERROR Num:%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason:%s", szErrorMessage);
			inLogPrintf(AT, szDebugMsg);
			inLogPrintf(AT, szVacummSql);
		}
		
		free(szVacummSql);
		if (szErrorMessage != NULL)
		{
			sqlite3_free(szErrorMessage);
		}
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Vacuum Table OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 釋放記憶體 */
	free(szVacummSql);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Vacuum()_END");
                inLogPrintf(AT, "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Check_Escape_Cnt
Date&Time       :2017/3/21 下午 4:23
Describe        :
*/
int inSqlite_Check_Escape_Cnt(char* szString)
{
	int	i;
	int	inCnt = 0;
	
	for (i = 0; i < strlen(szString); i++)
	{
		if (szString[i] == '\'')
		{
			inCnt++;
		}
	}
	
	
	return (inCnt);
}

/*
Function        :inSqlite_Process_Escape
Date&Time       :2017/3/21 下午 4:23
Describe        :
*/
int inSqlite_Process_Escape(char* szOldString, char* szNewString)
{
	int	i;
	int	inNewStringIndex = 0;
	
	for (i = 0; i < strlen(szOldString); i++)
	{
		szNewString[inNewStringIndex] = szOldString[i];
		inNewStringIndex++;
		
		if (szOldString[i] == '\'')
		{
			strcat(szNewString, "'");
			inNewStringIndex++;
		}
		
	}
	
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_ESC_Sale_Upload_Count
Date&Time       :2016/4/29 下午 1:17
Describe        :可以取得table有幾筆資料
*/
int inSqlite_Get_ESC_Sale_Upload_Count(TRANSACTION_OBJECT *pobTran, char *szTableName, int inTxnType, unsigned char uszPaperBit, int *inTxnTotalCnt, long *lnTxnTotalAmt)
{
	int		j = 0;
	int		inRetVal = VS_ERROR;
	int		inTempCnt = 0;
	long		lnTempAmt = 0;
	char		szDebugMsg[128 + 1] = {0};
	char		szQuerySql[_SQL_SENTENCE_MAX_ + 1] = {0};	/* INSERT INTO	szTableName( */
	char		szErrorMessage[100 + 1];
	sqlite3		*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt	*srSQLStat;
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Get_ESC_Sale_Upload_Count()_START");
        }
	
	inRetVal = inSqlite_DB_Open_Or_Create(gszTranDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	
	/* SQLite COUNT 計算資料庫中 table的行數 */
	if (inTxnType == _ESC_REINFORCE_TXNCODE_SALE_)
	{
		if (uszPaperBit == VS_TRUE)
		{
			sprintf(szQuerySql, "SELECT count(*), sum(lnTotalTxnAmount) FROM %s WHERE "
				"((uszUpdated = %d) AND "
				"(inESCUploadStatus = %d OR "
				"inESCUploadStatus = %d) AND "
				"(incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d))", 
				szTableName, 
				0, 
				_ESC_UPLOAD_STATUS_PAPER_, 
				_ESC_UPLOAD_STATUS_UPLOAD_NOT_YET_, 
				_SALE_,
				_CUP_SALE_,
				_INST_SALE_,
				_REDEEM_SALE_,
				_SALE_OFFLINE_,
				_INST_ADJUST_,
				_REDEEM_ADJUST_,
				_PRE_COMP_,
				_CUP_PRE_COMP_,
				_MAIL_ORDER_,
				_CUP_MAIL_ORDER_,
				_CASH_ADVANCE_,
				_FORCE_CASH_ADVANCE_,
				_FISC_SALE_);
		}
		else
		{
			sprintf(szQuerySql, "SELECT count(*), sum(lnTotalTxnAmount) FROM %s WHERE "
				"((uszUpdated = %d) AND "
				"(inESCUploadStatus = %d) AND "
				"(incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d))", 
				szTableName, 
				0, 
				_ESC_UPLOAD_STATUS_UPLOADED_,
				_SALE_,
				_CUP_SALE_,
				_INST_SALE_,
				_REDEEM_SALE_,
				_SALE_OFFLINE_,
				_INST_ADJUST_,
				_REDEEM_ADJUST_,
				_PRE_COMP_,
				_CUP_PRE_COMP_,
				_MAIL_ORDER_,
				_CUP_MAIL_ORDER_,
				_CASH_ADVANCE_,
				_FORCE_CASH_ADVANCE_,
				_FISC_SALE_);
		}
	}
	else if (inTxnType == _ESC_REINFORCE_TXNCODE_REFUND_)
	{
		if (uszPaperBit == VS_TRUE)
		{
			sprintf(szQuerySql, "SELECT count(*), sum(lnTotalTxnAmount) FROM %s WHERE "
				"((uszUpdated = %d) AND "
				"(inESCUploadStatus = %d OR "
				"inESCUploadStatus = %d) AND "
				"(incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d))", 
				szTableName, 
				0, 
				_ESC_UPLOAD_STATUS_PAPER_,  
				_ESC_UPLOAD_STATUS_UPLOAD_NOT_YET_,
				_REFUND_,
				_REDEEM_REFUND_,
				_INST_REFUND_,
				_CUP_REFUND_,
				_CUP_MAIL_ORDER_REFUND_,
				_FISC_REFUND_);
		}
		else
		{
			sprintf(szQuerySql, "SELECT count(*), sum(lnTotalTxnAmount) FROM %s WHERE "
				"((uszUpdated = %d) AND "
				"(inESCUploadStatus = %d) AND "
				"(incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d OR "
				"incode = %d))", 
				szTableName, 
				0, 
				_ESC_UPLOAD_STATUS_UPLOADED_,
				_REFUND_,
				_REDEEM_REFUND_,
				_INST_REFUND_,
				_CUP_REFUND_,
				_CUP_MAIL_ORDER_REFUND_,
				_FISC_REFUND_);
		}
	}
	else
	{
		inSqlite_DB_Close(&srDBConnection);

		return (VS_ERROR);
	}
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	
	/* 先設定初始值，避免沒有Table就不給值 */
	*inTxnTotalCnt = 0;
	*lnTxnTotalAmt = 0;
	
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inLogPrintf(AT, szDebugMsg);
		}
		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		inSqlite_DB_Close(&srDBConnection);
		
		return (VS_NO_RECORD);
	}
	else
	{
		memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
		strcpy(szErrorMessage, sqlite3_errmsg(srDBConnection));
			
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", szErrorMessage);
			inLogPrintf(AT, szDebugMsg);

			inLogPrintf(AT, szQuerySql);
		}

		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}

		/* 因為直接get 不存在的table回傳值是-1，只有在Error Msg才能得知錯誤原因 */
		if (memcmp(szErrorMessage, "no such table", strlen("no such table")) == 0)
		{
			return (VS_NO_RECORD);
		}
		else
		{
			return (VS_ERROR);
		}
	}
	
	/* 若是成功，將值丟到輸入的位置。 */
	j = 0;
	inTempCnt = sqlite3_column_int(srSQLStat, j);
	*inTxnTotalCnt += inTempCnt;
	j = 1;
	lnTempAmt = sqlite3_column_int(srSQLStat, j);
	*lnTxnTotalAmt += lnTempAmt;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Get count(*) OK count:%d Amt:%ld", inTempCnt, lnTempAmt);
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* 釋放事務 */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	
	/* 補計算TIP的筆數金額，分別算原交易和Tip */
	/* 先計算原交易 */
	if (inTxnType == _ESC_REINFORCE_TXNCODE_SALE_)
	{	
		memset(szQuerySql, 0x00, sizeof(szQuerySql));
		
		if (uszPaperBit == VS_TRUE)
		{
			sprintf(szQuerySql, "SELECT count(*), sum(lnTxnAmount) FROM %s WHERE "
				"((uszUpdated = %d) AND "
				"(uszESCOrgUploadBit = %s) AND "
				"(incode = %d))", 
				szTableName, 
				0, 
				"X'00'",
				_TIP_);
		}
		else
		{
			sprintf(szQuerySql, "SELECT count(*), sum(lnTxnAmount) FROM %s WHERE "
				"((uszUpdated = %d) AND "
				"(uszESCOrgUploadBit = %s) AND "
				"(incode = %d))", 
				szTableName, 
				0,
				"X'01'",
				_TIP_);
		}
		
		/* prepare語句 */
		inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
		if (inRetVal != SQLITE_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
				inLogPrintf(AT, szDebugMsg);
				
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
				inLogPrintf(AT, szDebugMsg);
			}

		}

		/* Qerry */
		inRetVal = sqlite3_step(srSQLStat);
		if (inRetVal == SQLITE_ROW)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Get Table OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else if (inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "NO DATA");
				inLogPrintf(AT, szDebugMsg);
			}
			/* 釋放事務 */
			inSqlite_SQL_Finalize(&srSQLStat);
			inSqlite_DB_Close(&srDBConnection);

			return (VS_NO_RECORD);
		}
		else
		{
			memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
			strcpy(szErrorMessage, sqlite3_errmsg(srDBConnection));

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
				inLogPrintf(AT, szDebugMsg);

				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", szErrorMessage);
				inLogPrintf(AT, szDebugMsg);

				inLogPrintf(AT, szQuerySql);
			}

			/* 釋放事務 */
			inSqlite_SQL_Finalize(&srSQLStat);
			/* 關閉 database, close null pointer 是NOP(No Operation) */
			if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
			{
				/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
				return (VS_ERROR);
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "Close Database OK");
					inLogPrintf(AT, szDebugMsg);
				}
			}

			/* 因為直接get 不存在的table回傳值是-1，只有在Error Msg才能得知錯誤原因 */
			if (memcmp(szErrorMessage, "no such table", strlen("no such table")) == 0)
			{
				return (VS_NO_RECORD);
			}
			else
			{
				return (VS_ERROR);
			}
		}

		/* 若是成功，將值丟到輸入的位置。 */
		j = 0;
		inTempCnt = sqlite3_column_int(srSQLStat, j);
		*inTxnTotalCnt += inTempCnt;
		j = 1;
		lnTempAmt = sqlite3_column_int(srSQLStat, j);
		*lnTxnTotalAmt += lnTempAmt;

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get count(*) OK count:%d Amt:%ld", inTempCnt, lnTempAmt);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		
		/* 計算小費 */
		memset(szQuerySql, 0x00, sizeof(szQuerySql));
		
		if (uszPaperBit == VS_TRUE)
		{
			sprintf(szQuerySql, "SELECT count(*), sum(lnTipTxnAmount) FROM %s WHERE "
				"((uszUpdated = %d) AND "
				"(uszESCTipUploadBit = %s) AND "
				"(incode = %d))", 
				szTableName, 
				0, 
				"X'00'",
				_TIP_);
		}
		else
		{
			sprintf(szQuerySql, "SELECT count(*), sum(lnTipTxnAmount) FROM %s WHERE "
				"((uszUpdated = %d) AND "
				"(uszESCTipUploadBit = %s) AND "
				"(incode = %d))", 
				szTableName, 
				0, 
				"X'01'",
				_TIP_);
		}
		
		/* prepare語句 */
		inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
		if (inRetVal != SQLITE_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
				inLogPrintf(AT, szDebugMsg);
				
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
				inLogPrintf(AT, szDebugMsg);
			}

		}

		/* Qerry */
		inRetVal = sqlite3_step(srSQLStat);
		if (inRetVal == SQLITE_ROW)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Get Table OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else if (inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "NO DATA");
				inLogPrintf(AT, szDebugMsg);
			}
			/* 釋放事務 */
			inSqlite_SQL_Finalize(&srSQLStat);
			inSqlite_DB_Close(&srDBConnection);

			return (VS_NO_RECORD);
		}
		else
		{
			memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
			strcpy(szErrorMessage, sqlite3_errmsg(srDBConnection));

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
				inLogPrintf(AT, szDebugMsg);

				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", szErrorMessage);
				inLogPrintf(AT, szDebugMsg);

				inLogPrintf(AT, szQuerySql);
			}

			/* 釋放事務 */
			inSqlite_SQL_Finalize(&srSQLStat);
			/* 關閉 database, close null pointer 是NOP(No Operation) */
			if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
			{
				/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
				return (VS_ERROR);
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "Close Database OK");
					inLogPrintf(AT, szDebugMsg);
				}
			}

			/* 因為直接get 不存在的table回傳值是-1，只有在Error Msg才能得知錯誤原因 */
			if (memcmp(szErrorMessage, "no such table", strlen("no such table")) == 0)
			{
				return (VS_NO_RECORD);
			}
			else
			{
				return (VS_ERROR);
			}
		}

		/* 若是成功，將值丟到輸入的位置。 */
		j = 0;
		inTempCnt = sqlite3_column_int(srSQLStat, j);
		*inTxnTotalCnt += inTempCnt;
		j = 1;
		lnTempAmt = sqlite3_column_int(srSQLStat, j);
		*lnTxnTotalAmt += lnTempAmt;

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get count(*) OK count:%d Amt:%ld", inTempCnt, lnTempAmt);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
	}
	
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Get count(*) OK count:%d Amt:%ld", *inTxnTotalCnt, *lnTxnTotalAmt);
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Get_ESC_Sale_Upload_Count()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Batch_To_DB_Test
Date&Time       :2016/4/11 下午 5:33
Describe        :將現有的batch檔轉成改以db紀錄，測試用
*/
int inSqlite_Batch_To_DB_Test(void)
{
//	int			i;
//	int			inRetVal;
//	long			lnCurrentInvoiceNumber;
//	char			szTable[16 + 1];
//	TRANSACTION_OBJECT	pobTran;
//	
//	memset(&pobTran, 0x00, sizeof(pobTran));
//	pobTran.srBRec.inHDTIndex = -1;
//	
//	inFunc_GetHostNum(&pobTran);
//	
//	pobTran.srBRec.lnOrgInvNum = -1;
//	inBATCH_FuncUserChoice_By_Sqlite(&pobTran);
//	lnCurrentInvoiceNumber = pobTran.srBRec.lnOrgInvNum;
//	
//	for (i = 1; i <= lnCurrentInvoiceNumber; i++)
//	{
//		inBATCH_GetTransRecord_By_Sqlite(&pobTran);
//		memcpy(pobTran.srBRec.szCardHolder, "'", strlen("'"));
//		inRetVal = inSqlite_Insert_All_Flow(&pobTran, _TN_BATCH_TABLE_);
//		if (inRetVal != VS_SUCCESS)
//		{
//			DISPLAY_OBJECT	srDispMsgObj;
//			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
//			strcpy(srDispMsgObj.szDispPic1Name, _ERR_BATCH_);
//			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
//			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
//			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
//			strcpy(srDispMsgObj.szErrMsg1, "");
//			srDispMsgObj.inErrMsgLine1 = 0;
//			inDISP_Msg_BMP(_ERR_BATCH_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
//			break;
//		}
//		pobTran.srBRec.lnOrgInvNum++;
//	}
	
	
//	inFunc_AP_Folder_Copy_To_SD(_DATA_BASE_NAME_NEXSYS_);
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_Table_Count_Test
Date&Time       :2016/5/4 上午 11:28
Describe        :算出table 資料筆數，測試用
*/
int inSqlite_Get_Table_Count_Test()
{
	int			inCnt = 0;
	TRANSACTION_OBJECT	pobTran;
	
	memset(&pobTran, 0x00, sizeof(pobTran));
	
	inSqlite_Get_Table_Count(gszTranDBPath, _BATCH_TABLE_ESC_AGAIN_SUFFIX_, &inCnt);
		
	if (ginDebug == VS_TRUE)
	{
		char szDebugMsg[100 + 1];

		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "%d", inCnt);
		inLogPrintf(AT, szDebugMsg);
	}
	
	inSqlite_Get_Table_Count(gszTranDBPath, _BATCH_TABLE_ESC_FAIL_SUFFIX_, &inCnt);
		
	if (ginDebug == VS_TRUE)
	{
		char szDebugMsg[100 + 1];

		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "%d", inCnt);
		inLogPrintf(AT, szDebugMsg);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Vacuum_Flow_Test
Date&Time       :2017/3/17 下午 3:40
Describe        :測試壓縮指令
*/
int inSqlite_Vacuum_Flow_Test()
{
	TRANSACTION_OBJECT	pobTran;
	
	memset(&pobTran, 0x00, sizeof(pobTran));
	pobTran.srBRec.inHDTIndex = -1;
	
	inFunc_GetHostNum(&pobTran);
	inSqlite_Vacuum_Flow(&pobTran, _TN_BATCH_TABLE_);
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Update_ByTableID_All
Date&Time       :2019/4/24 上午 9:44
Describe        :利用Table ID來更新資料庫中的所有欄位，注意:update keytag不能重複（若已沒有Record則會update失敗，改用insert or update更符合以前SaveRec的使用情況）
*/
int inSqlite_Update_ByRecordID_All(char* szDBName, char* szTableName, int inRecordID, SQLITE_ALL_TABLE* srAll)
{
	int			inRowID = 0;
	int			inRetVal = VS_SUCCESS;
	int			inSqlLength = 0;
	char			szDebugMsg[84 + 1];
	char			szSqlPrefix[100 + 1];	/* Update INTO	szTableName( */
	char			szSqlSuffix[40 + 1];	/* VALUES ( */
	char			szSqlSuffix2[40 + 1];	/* ); */
	char			*szUpdateSql;
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	SQLITE_SQL_CALCULATE_TABLE	srSQLCal;
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Update_ByInvoiceNumber_All()_START");
        }
	
	if (inRecordID < 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inRecordID不合法:%d", inRecordID);
			inLogPrintf(AT, szDebugMsg);
		}
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inRecordID不合法:%d", inRecordID);
		inUtility_StoreTraceLog_OneStep(szDebugMsg);
		
		return (VS_ERROR);
	}
	
	/* 程式內的RecorID都是從0開始，RowID從1開始，所以這邊加一 */
	inRowID = inRecordID + 1;
	
	inRetVal = inSqlite_DB_Open_Or_Create(szDBName, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "UPDATE %s", szTableName);
	
	memset(szSqlSuffix, 0x00, sizeof(szSqlSuffix));
	sprintf(szSqlSuffix, " SET ");
	
	memset(szSqlSuffix2, 0x00, sizeof(szSqlSuffix2));
	sprintf(szSqlSuffix2, " WHERE inTableID = %d FROM %s", inRowID, szTableName);
	
	/* 算要配置多少記憶體 */
	memset(&srSQLCal, 0x00, sizeof(SQLITE_SQL_CALCULATE_TABLE));
	srSQLCal.pSqlPrefix = szSqlPrefix;
	srSQLCal.pSqlSuffix = szSqlSuffix;
	srSQLCal.pSqlSuffix2 = szSqlSuffix2;
	
	inSqlite_Calculate_Update_SQLLength(&srSQLCal, srAll, &inSqlLength);
	
	/* 配置記憶體 */
	szUpdateSql = malloc(inSqlLength + 1);
	memset(szUpdateSql, 0x00, inSqlLength);
	
	inSqlite_Gernerate_UpdateSQL(&srSQLCal, srAll, szUpdateSql);
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szUpdateSql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	
	/* Binding變數 */
	inSqlite_Binding_Values(srAll, &srSQLStat);
	
	/* Update */
	do
	{
		/* Update */
		inRetVal = sqlite3_step(srSQLStat);
		if (inRetVal == SQLITE_ROW	||
		    inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Update OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Update ERROR Num:%d", inRetVal);
				inLogPrintf(AT, szDebugMsg);
				
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
				inLogPrintf(AT, szDebugMsg);
				
				inLogPrintf(AT, szUpdateSql);
			}

		}
		
	}while (inRetVal == SQLITE_ROW);
	
	/* 釋放事務，若要重用則用sqlite3_reset */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	free(szUpdateSql);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Update_ByRecordID_All()_END");
                inLogPrintf(AT, "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_Table_ByRecordID_All
Date&Time       :2019/4/24 下午 2:22
Describe        :利用RecordID、取得最新參數
*/
int inSqlite_Get_Table_ByRecordID_All(char* szDBName, char* szTableName, int inRecordID, SQLITE_ALL_TABLE* srAll)
{
	int			inRowID = 0;
	int			inRetVal = VS_SUCCESS;
	char			szDebugMsg[128 + 1] = {0};
	char			szQuerySql[_SQL_SENTENCE_MAX_ + 1];	/* INSERT INTO	szTableName( */
	char			szErrorMessage[100 + 1];
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Get_Table_ByRecordID_All(%s,%s,%d)_START", szTableName, szDBName, inRecordID);
        }
	
	if (inRecordID < 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inRecordID不合法:%d", inRecordID);
			inLogPrintf(AT, szDebugMsg);
		}
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inRecordID不合法:%d", inRecordID);
		inUtility_StoreTraceLog_OneStep(szDebugMsg);
		
		return (VS_ERROR);
	}
	
	inRowID = inRecordID + 1;
	
	inRetVal = inSqlite_DB_Open_Or_Create(szDBName, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 塞入SQL語句 */
	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "SELECT * FROM %s WHERE inTableID = %d ORDER BY inTableID DESC LIMIT 1", szTableName, inRowID);

	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inLogPrintf(AT, szDebugMsg);
		}
		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		inSqlite_DB_Close(&srDBConnection);
		
		return (VS_NO_RECORD);
	}
	else
	{
		memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
		strcpy(szErrorMessage, sqlite3_errmsg(srDBConnection));
			
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", szErrorMessage);
			inLogPrintf(AT, szDebugMsg);

			inLogPrintf(AT, szQuerySql);
		}

		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}

		/* 因為直接get 不存在的table回傳值是-1，只有在Error Msg才能得知錯誤原因 */
		if (memcmp(szErrorMessage, "no such table", strlen("no such table")) == 0)
		{
			return (VS_NO_RECORD);
		}
		else
		{
			return (VS_ERROR);
		}
	}
	
	/* binding 取得值 */
	inSqlite_Get_Binding_Value(&srSQLStat, srAll);
		
	/* 釋放事務 */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Get_Table_ByRecordID_All()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Insert_Or_Replace_ByRecordID_All
Date&Time       :2019/4/26 下午 5:10
Describe        :Open Database檔 insert or replace(若有此紀錄，則更新(若該欄位未指定值，則會更新為NULL)，若無此紀錄，則插入)
 *		inRecordID傳進來會自動加一，因為SQLite的RowID從1開始
*/
int inSqlite_Insert_Or_Replace_ByRecordID_All(char* szDBName, char* szTableName, int inRecordID , SQLITE_ALL_TABLE* srAll)
{
	int			inRowID = 0;
	int			inRetVal;
	int			inSqlLength = 0;	/* 算組SQL語句的長度 */
	char			szDebugMsg[84 + 1];
	char			szSqlPrefix[100 + 1];	/* INSERT INTO	szTableName( */
	char			szSqlSuffix[20 + 1];	/* VALUES ( */
	char			szSqlSuffix2[10 + 1];	/* ); */
	char			*szInsertSql;
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	SQLITE_SQL_CALCULATE_TABLE	srSQLCal;
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Insert_Or_Replace_ByRecordID_All(%s,%s,%d)_START", szTableName, szDBName, inRecordID);
        }
	
	if (inRecordID < 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inRecordID不合法:%d", inRecordID);
			inLogPrintf(AT, szDebugMsg);
		}
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inRecordID不合法:%d", inRecordID);
		inUtility_StoreTraceLog_OneStep(szDebugMsg);
		
		return (VS_ERROR);
	}
	
	/* 程式內的RecorID都是從0開始，RowID從1開始，所以這邊加一 */
	inRowID = inRecordID + 1;
	/* 強制改成rowID的pointer */
	srAll->srInt[0].pTagValue = &inRowID;
	inRetVal = inSqlite_DB_Open_Or_Create(szDBName, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "INSERT OR REPLACE INTO %s (", szTableName);
	
	memset(szSqlSuffix, 0x00, sizeof(szSqlSuffix));
	sprintf(szSqlSuffix, ")VALUES (");
	
	memset(szSqlSuffix2, 0x00, sizeof(szSqlSuffix2));
	sprintf(szSqlSuffix2, ");");
	
	/* 算要配置多少記憶體 */
	memset(&srSQLCal, 0x00, sizeof(SQLITE_SQL_CALCULATE_TABLE));
	srSQLCal.pSqlPrefix = szSqlPrefix;
	srSQLCal.pSqlSuffix = szSqlSuffix;
	srSQLCal.pSqlSuffix2 = szSqlSuffix2;
	inSqlite_Calculate_Insert_SQLLength(&srSQLCal, srAll, &inSqlLength);
	
	/* 配置記憶體 */
	szInsertSql = malloc(inSqlLength + 100);
	memset(szInsertSql, 0x00, inSqlLength + 100);
	
	inSqlite_Gernerate_InsertSQL(&srSQLCal, srAll, szInsertSql);
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szInsertSql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	
	/* Binding變數 */
	inSqlite_Binding_Values(srAll, &srSQLStat);
	
	do
	{
		/* Insert */
		inRetVal = sqlite3_step(srSQLStat);
		if (inRetVal == SQLITE_ROW	||
		    inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Insert OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Insert ERROR Num:%d", inRetVal);
				inLogPrintf(AT, szDebugMsg);
				
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
				inLogPrintf(AT, szDebugMsg);
				
				inLogPrintf(AT, szInsertSql);
			}

		}
		
	}while (inRetVal == SQLITE_ROW);
	
	/* 釋放事務，若要重用則用sqlite3_reset */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	free(szInsertSql);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (inRetVal == SQLITE_ERROR)
	{
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Insert_Or_Replace_ByRecordID_All()_END");
                inLogPrintf(AT, "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_Data_By_External_SQL
Date&Time       :2019/5/28 下午 2:22
Describe        :嘗試不將SQL寫死達到重複使用的效果
*/
int inSqlite_Get_Data_By_External_SQL(char* szDBName, SQLITE_ALL_TABLE* srAll, char* szSQLStatement)
{
	int			inRetVal = VS_SUCCESS;
	char			szDebugMsg[128 + 1];
	char			szQuerySql[_SQL_SENTENCE_MAX_ + 1];	/* INSERT INTO	szTableName( */
	char			szErrorMessage[100 + 1];
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat = NULL; /* 初始化狀態 */
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Get_Table_ByRecordID_All()_START");
        }
	
	inRetVal = inSqlite_DB_Open_Or_Create(szDBName, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 塞入SQL語句 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	strcpy(szQuerySql, szSQLStatement);

	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "SQLITE_DONE");
			inLogPrintf(AT, szDebugMsg);
		}
		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		inSqlite_DB_Close(&srDBConnection);
		
		return (VS_NO_RECORD);
	}
	else
	{
		memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
		strcpy(szErrorMessage, sqlite3_errmsg(srDBConnection));
			
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", szErrorMessage);
			inLogPrintf(AT, szDebugMsg);

			inLogPrintf(AT, szQuerySql);
		}

		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}

		/* 因為直接get 不存在的table回傳值是-1，只有在Error Msg才能得知錯誤原因 */
		if (memcmp(szErrorMessage, "no such table", strlen("no such table")) == 0)
		{
			return (VS_NO_RECORD);
		}
		else
		{
			return (VS_ERROR);
		}
	}
	
	/* binding 取得值 */
	inSqlite_Get_Binding_Value(&srSQLStat, srAll);
	
	/* 釋放事務 */
	sqlite3_finalize(srSQLStat);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Get_Table_ByRecordID_All()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_Binding_Value
Date&Time       :2019/6/3 下午 5:19
Describe        :取得查詢後的結果
*/
int inSqlite_Get_Binding_Value(sqlite3_stmt** srSQLStat, SQLITE_ALL_TABLE* srAll)
{
	int	j = 0;
	int	inCols = 0;
	int	inFind = VS_FALSE;
	int	inIntIndex = 0, inInt64tIndex = 0, inCharIndex = 0, inTextIndex = 0;
	int	inDataLen = 0;
	char	szTagName[_TAG_WIDTH_ + 1] = {0};
	char	szDebugMsg[128 + 1] = {0};
	
	inCols = sqlite3_column_count(*srSQLStat);
	
	/* binding 取得值 */
	for (j = 0; j < inCols; j++)
	{
		inFind = VS_FALSE;
		memset(szTagName, 0x00, sizeof(szTagName));
		strcat(szTagName, sqlite3_column_name(*srSQLStat, j));
		
		
		for (inIntIndex = 0; inIntIndex < srAll->inIntNum; inIntIndex++)
		{
			if (srAll->srInt[inIntIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}
			
			
			/* 比對Tag Name */
			if (memcmp(szTagName, srAll->srInt[inIntIndex].szTag, strlen(srAll->srInt[inIntIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll->srInt[inIntIndex].szTag))
			{
				*(int32_t*)srAll->srInt[inIntIndex].pTagValue = sqlite3_column_int(*srSQLStat, j);
				srAll->srInt[inIntIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}
		
		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inInt64tIndex = 0; inInt64tIndex < srAll->inInt64tNum; inInt64tIndex++)
		{
			if (srAll->srInt64t[inInt64tIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll->srInt64t[inInt64tIndex].szTag, strlen(srAll->srInt64t[inInt64tIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll->srInt64t[inInt64tIndex].szTag))
			{
				*(int64_t*)srAll->srInt64t[inInt64tIndex].pTagValue = sqlite3_column_int64(*srSQLStat, j);
				srAll->srInt64t[inInt64tIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}
			
		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}
		
		for (inCharIndex = 0; inCharIndex < srAll->inCharNum; inCharIndex++)
		{
			if (srAll->srChar[inCharIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}
			
			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll->srChar[inCharIndex].szTag, strlen(srAll->srChar[inCharIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll->srChar[inCharIndex].szTag))
			{
				inDataLen = sqlite3_column_bytes(*srSQLStat, j);
				/* 放之前先清空 */
				memset(srAll->srChar[inCharIndex].pCharVariable, 0x00, inDataLen + 1);
				memcpy(srAll->srChar[inCharIndex].pCharVariable, sqlite3_column_blob(*srSQLStat, j), inDataLen);
				srAll->srChar[inCharIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}
		
		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inTextIndex = 0; inTextIndex < srAll->inTextNum; inTextIndex++)
		{
			if (srAll->srText[inTextIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}
			
			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll->srText[inTextIndex].szTag, strlen(srAll->srText[inTextIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll->srText[inTextIndex].szTag))
			{
				inDataLen = sqlite3_column_bytes(*srSQLStat, j);
				/* 放之前先清空 */
				memset(srAll->srText[inTextIndex].pCharVariable, 0x00, inDataLen + 1);
				memcpy(srAll->srText[inTextIndex].pCharVariable, sqlite3_column_text(*srSQLStat, j), inDataLen);
				srAll->srText[inTextIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}
		
		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Find no variable to insert:Tag: %s", szTagName);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Value: %s", (char*)sqlite3_column_blob(*srSQLStat, j));
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_DB_Open_Or_Create
Date&Time       :2019/6/3 下午 6:26
Describe        :開啟DB，」若無則創建
*/
int inSqlite_DB_Open_Or_Create(char* szDBPath, sqlite3** srDBConnection, int inFlags, char* szVfs)
{
	int	inRetVal = SQLITE_OK;
	char	szDebugMsg[84 + 1] = {0};
	if (ginDebug == VS_TRUE)
        {
            inLogPrintf(AT, "inSqlite_DB_Open_Or_Create() START");
        }
	inRetVal = sqlite3_open_v2(szDBPath, srDBConnection, inFlags, szVfs);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File Failed");
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ErrorNum:%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		vdUtility_SYSFIN_LogMessage(AT,"Open Database Failed F:%s ErrorNum:%d", szDBPath, inRetVal);
        	if (ginDebug == VS_TRUE)
                {
                    inLogPrintf(AT, "inSqlite_DB_Open_Or_Create() END");
                }
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
		inFile_Open_File_Cnt_Increase();
        	if (ginDebug == VS_TRUE)
                {
                    inLogPrintf(AT, "inSqlite_DB_Open_Or_Create() END");
                }
		return (VS_SUCCESS);
	}
	
}

/*
Function        :inSqlite_Copy_Table_Data
Date&Time       :2019/6/4 上午 10:41
Describe        :複製Table
 *		uszPreserveData:是否拋棄table內的資料
*/
int inSqlite_Copy_Table_Data(char* szDBPath, char* szOldTableName, char* szNewTableName)
{
	int		inRetVal = VS_ERROR;
	int		inSqlLength = 0;
	char		szSqlPrefix[200 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		*szCreateSql;			/* 因為會很長且隨table浮動，所以用pointer */
	char		*szErrorMessage = NULL;
	sqlite3		*srDBConnection;		/* 建立到資料庫的connection */
		
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Copy_Table_Data()_START");
        }
	
	/* 開啟DataBase檔 */
	inRetVal = inSqlite_DB_Open_Or_Create(szDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	if (inRetVal != VS_SUCCESS)
	{	
		return (VS_ERROR);
	}

	/* 塞入SQL語句 */
	/* 為了使table name可變動，所以拉出來組 */
	/* 加入了if not exists字串，若已建立不會重複建立 */
	/* 前綴 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "INSERT INTO %s SELECT * FROM %s;", szNewTableName, szOldTableName);

	/*  "CREATE TABLE	szTableName("的長度 */
	inSqlLength += strlen(szSqlPrefix);
        
	/* inSqlLength: */
	if (ginDebug == VS_TRUE) 
        {
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inSqlLength: %d", inSqlLength);
		inLogPrintf(AT, szDebugMsg);
	}
	/* 配置記憶體(一定要+1，超級重要，不然會overflow) */
	szCreateSql = malloc(inSqlLength + 1);
	memset(szCreateSql, 0x00, inSqlLength);
	
	/* 先丟前綴Table Name */
	strcat(szCreateSql, szSqlPrefix);
	
	/* 建立 Table */
	inRetVal = sqlite3_exec(srDBConnection, szCreateSql, 0, 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Copy Table ERROR Num:%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason:%s", szErrorMessage);
			inLogPrintf(AT, szDebugMsg);
		}
		
		free(szCreateSql);
		if (szErrorMessage != NULL)
		{
			sqlite3_free(szErrorMessage);
		}
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Copy Table OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 釋放記憶體 */
	free(szCreateSql);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Copy_Table_Data()_END");
                inLogPrintf(AT, "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Rename_Table
Date&Time       :2019/6/4 下午 1:13
Describe        :重新命名
*/
int inSqlite_Rename_Table(char* szDBPath, char* szOldTableName, char* szNewTableName)
{
	int		inRetVal = VS_ERROR;
	int		inSqlLength = 0;
	char		szSqlPrefix[100 + 2] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		*szCreateSql;			/* 因為會很長且隨table浮動，所以用pointer */
	char		*szErrorMessage = NULL;
	sqlite3		*srDBConnection;		/* 建立到資料庫的connection */
		
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Rename_Table()_START");
        }
	
	/* 開啟DataBase檔 */
	inRetVal = inSqlite_DB_Open_Or_Create(szDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	if (inRetVal != VS_SUCCESS)
	{	
		return (VS_ERROR);
	}

	/* 塞入SQL語句 */
	/* 為了使table name可變動，所以拉出來組 */
	/* 加入了if not exists字串，若已建立不會重複建立 */
	/* 前綴 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "ALTER TABLE %s RENAME TO %s;", szOldTableName, szNewTableName);
	/*  "CREATE TABLE	szTableName("的長度 */
	inSqlLength += strlen(szSqlPrefix);
        
	/* inSqlLength: */
	if (ginDebug == VS_TRUE) 
        {
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inSqlLength: %d", inSqlLength);
		inLogPrintf(AT, szDebugMsg);
	}
	/* 配置記憶體(一定要+1，超級重要，不然會overflow) */
	szCreateSql = malloc(inSqlLength + 1);
	memset(szCreateSql, 0x00, inSqlLength);
	
	/* 先丟前綴Table Name */
	strcat(szCreateSql, szSqlPrefix);
	
	/* 建立 Table */
	inRetVal = sqlite3_exec(srDBConnection, szCreateSql, 0, 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Rename Table ERROR Num:%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason:%s", szErrorMessage);
			inLogPrintf(AT, szDebugMsg);
		}
		
		free(szCreateSql);
		if (szErrorMessage != NULL)
		{
			sqlite3_free(szErrorMessage);
		}
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Rename Table OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 釋放記憶體 */
	free(szCreateSql);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Rename_Table()_END");
                inLogPrintf(AT, "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Table_Relist_SQLite
Date&Time       :2019/6/4 下午 1:25
Describe        :
*/
int inSqlite_Table_Relist_SQLite(char *szDBPath, char *szTableName, int inOldIndex, int inNewIndex)
{
	int			i = 0;
	int			inBindingIndex = 1;
	int			inRetVal;
	int			inTempTableID = 0;		/* 暫時最大+1 */
	int			inTableID_OldIndex = 0;
	int			inTableID_NewIndex = 0;
	char			szDebugMsg[100 + 1] = {0};
	char			szMaxTableID[10 + 1] = {0};
	char			szQuerySql[_SQL_SENTENCE_MAX_ + 1] = {0};
	sqlite3			*srDBConnection;		/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inSqlite_Table_Relist_SQLite() START !");
	}
	
	/* index不合法 */
	if (inOldIndex == -1 || inNewIndex == -1)
	{
		return (VS_ERROR);
	}
	
	/* 傳進來的是傳統的record，從0開始，TableID要加一 */
	inTableID_OldIndex = inOldIndex + 1;
	inTableID_NewIndex = inNewIndex + 1;
	
	memset(szMaxTableID, 0x00, sizeof(szMaxTableID));
	inRetVal = inSqlite_Get_Max_TableID(szDBPath, szTableName, szMaxTableID);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	else
	{
		inTempTableID = atoi(szMaxTableID) + 1;
	}
	
	/* 開啟DataBase檔 */
	inRetVal = inSqlite_DB_Open_Or_Create(szDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	if (inRetVal != VS_SUCCESS)
	{	
		return (VS_ERROR);
	}
	
	/* 第一步驟，先將要調整的RECORD塞到最後面 */
	
	/* 塞入SQL語句 */
	/* 為了使table name可變動，所以拉出來組 */
	/* 前綴 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "UPDATE %s SET inTableID = %d WHERE (inTableID = %d);", szTableName, inTempTableID, inTableID_OldIndex);
	/*  "CREATE TABLE	szTableName("的長度 */
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	do
	{
		inRetVal = sqlite3_step(srSQLStat);
		if (inRetVal == SQLITE_ROW)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "UPDATE Table OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else if (inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "UPDATE Table OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "UPDATE Table ERROR Num:%d", inRetVal);
				inLogPrintf(AT, szDebugMsg);

				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
				inLogPrintf(AT, szDebugMsg);

				inLogPrintf(AT, szQuerySql);
			}

			/* 釋放事務 */
			inSqlite_SQL_Finalize(&srSQLStat);
			/* 關閉 database, close null pointer 是NOP(No Operation) */
			if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
			{
				/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
				return (VS_ERROR);
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "Close Database OK");
					inLogPrintf(AT, szDebugMsg);
				}
			}

			return (VS_ERROR);
		}
	
	}while (inRetVal != SQLITE_DONE);
	
	/* 釋放事務 */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	/* 第二步驟，將中間的TableID加一，以空出空間 */
	/* 塞入SQL語句 */
	/* 為了使table name可變動，所以拉出來組 */
	/* 前綴 */
	/* 往前插入的狀況 */
	if (inTableID_NewIndex < inTableID_OldIndex)
	{
		memset(szQuerySql, 0x00, sizeof(szQuerySql));
		sprintf(szQuerySql, "UPDATE %s SET inTableID = inTableID + 1 WHERE inTableID = ? ;", szTableName);
		/*  "CREATE TABLE	szTableName("的長度 */

		/* prepare語句 */
		inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
		if (inRetVal != SQLITE_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
				inLogPrintf(AT, szDebugMsg);
				
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
				inLogPrintf(AT, szDebugMsg);
			}
		}
		
		for (i = (inTableID_OldIndex - 1); i >= inTableID_NewIndex; i--)
		{
			inBindingIndex = 1;
			
			inRetVal = sqlite3_bind_int(srSQLStat, inBindingIndex, i);
			if (inRetVal != SQLITE_OK)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Binging Int Fail: %d", inRetVal);
					inLogPrintf(AT, szDebugMsg);
				}
			}
			else
			{
				inBindingIndex++;
			}
			
			/* 取得 database 裡所有的資料 */
			/* Qerry */
			do
			{
				inRetVal = sqlite3_step(srSQLStat);
				if (inRetVal == SQLITE_ROW)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "UPDATE Table OK");
						inLogPrintf(AT, szDebugMsg);
					}
				}
				else if (inRetVal == SQLITE_DONE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "UPDATE Table OK");
						inLogPrintf(AT, szDebugMsg);
					}
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "UPDATE Table ERROR Num:%d", inRetVal);
						inLogPrintf(AT, szDebugMsg);

						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
						inLogPrintf(AT, szDebugMsg);

						inLogPrintf(AT, szQuerySql);
					}

					/* 釋放事務 */
					inSqlite_SQL_Finalize(&srSQLStat);
	
					/* 關閉 database, close null pointer 是NOP(No Operation) */
					if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
					{
						/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
						return (VS_ERROR);
					}
					else
					{
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "Close Database OK");
							inLogPrintf(AT, szDebugMsg);
						}
					}

					return (VS_ERROR);
				}

			}while (inRetVal != SQLITE_DONE);
			
			sqlite3_reset(srSQLStat);
		}

		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
	}
	else
	{
		memset(szQuerySql, 0x00, sizeof(szQuerySql));
		sprintf(szQuerySql, "UPDATE %s SET inTableID = inTableID + 1 WHERE inTableID = ? ;", szTableName);
		/*  "CREATE TABLE	szTableName("的長度 */

		/* prepare語句 */
		inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
		if (inRetVal != SQLITE_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
				inLogPrintf(AT, szDebugMsg);
				
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
				inLogPrintf(AT, szDebugMsg);
			}
		}
		
		for (i = (inTableID_OldIndex + 1); i <= inTableID_NewIndex; i++)
		{
			inBindingIndex = 1;
			
			inRetVal = sqlite3_bind_int(srSQLStat, inBindingIndex, i);
			if (inRetVal != SQLITE_OK)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Binging Int Fail: %d", inRetVal);
					inLogPrintf(AT, szDebugMsg);
				}
			}
			else
			{
				inBindingIndex++;
			}
			
			/* 取得 database 裡所有的資料 */
			/* Qerry */
			do
			{
				inRetVal = sqlite3_step(srSQLStat);
				if (inRetVal == SQLITE_ROW)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "UPDATE Table OK");
						inLogPrintf(AT, szDebugMsg);
					}
				}
				else if (inRetVal == SQLITE_DONE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "UPDATE Table OK");
						inLogPrintf(AT, szDebugMsg);
					}
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "UPDATE Table ERROR Num:%d", inRetVal);
						inLogPrintf(AT, szDebugMsg);

						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
						inLogPrintf(AT, szDebugMsg);

						inLogPrintf(AT, szQuerySql);
					}

					/* 釋放事務 */
					inSqlite_SQL_Finalize(&srSQLStat);
					
					/* 關閉 database, close null pointer 是NOP(No Operation) */
					if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
					{
						/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
						return (VS_ERROR);
					}
					else
					{
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "Close Database OK");
							inLogPrintf(AT, szDebugMsg);
						}
					}

					return (VS_ERROR);
				}

			}while (inRetVal != SQLITE_DONE);
			
			sqlite3_reset(srSQLStat);
		}

		/* 釋放事務 */
		inSqlite_SQL_Finalize(&srSQLStat);
	}
	
	/* 第三步驟 將Record放回去 */
	/* 塞入SQL語句 */
	/* 為了使table name可變動，所以拉出來組 */
	/* 前綴 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "UPDATE %s SET inTableID = %d WHERE (inTableID = %d);", szTableName, inTableID_NewIndex, inTempTableID);
	/*  "CREATE TABLE	szTableName("的長度 */
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	do
	{
		inRetVal = sqlite3_step(srSQLStat);
		if (inRetVal == SQLITE_ROW)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "UPDATE Table OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else if (inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "UPDATE Table OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "UPDATE Table ERROR Num:%d", inRetVal);
				inLogPrintf(AT, szDebugMsg);

				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
				inLogPrintf(AT, szDebugMsg);

				inLogPrintf(AT, szQuerySql);
			}

			/* 釋放事務 */
			inSqlite_SQL_Finalize(&srSQLStat);
			
			/* 關閉 database, close null pointer 是NOP(No Operation) */
			if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
			{
				/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
				return (VS_ERROR);
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "Close Database OK");
					inLogPrintf(AT, szDebugMsg);
				}
			}

			return (VS_ERROR);
		}
	
	}while (inRetVal != SQLITE_DONE);
	
	/* 釋放事務 */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inSqlite_Table_Relist_SQLite() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

        return (VS_SUCCESS);
}

/*
Function        :inSqlite_Delete_Table_Data_Flow
Date&Time       :2019/6/5 上午 11:59
Describe        :在這邊決定名稱並分流
*/
int inSqlite_Delete_Table_Data_Flow(TRANSACTION_OBJECT *pobTran, int inTableType)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	if (pobTran->uszFileNameNoNeedNumBit == VS_TRUE)
	{
		inFunc_ComposeFileName(pobTran, szTableName, "", 0);
	}
	else
	{
		inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	}
	
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_TICKET_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_TEMP_:
			strcat(szTableName, _BATCH_TABLE_ESC_TEMP_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_ADVICE_:
			strcat(szTableName, _BATCH_TABLE_ESC_ADVICE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_ADVICE_EMV_:
			strcat(szTableName, _BATCH_TABLE_ESC_ADVICE_EMV_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			strcat(szTableName, _BATCH_TABLE_ADVICE_SUFFIX_);
		default :
			break;
	}
	
	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_BATCH_TABLE_ESC_TEMP_:
		case _TN_EMV_TABLE_:
		case _TN_BATCH_TABLE_TICKET_:
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
		case _TN_BATCH_TABLE_ESC_ADVICE_:
		case _TN_BATCH_TABLE_ESC_ADVICE_EMV_:
			inRetVal = inSqlite_Delete_Table_Data(gszTranDBPath, szTableName);
			break;
		default :
			return (VS_ERROR);
			break;
	}
	
	return (inRetVal);
}

/*
Function        :inSqlite_Delete_Table_Data
Date&Time       :2019/6/5 上午 11:59
Describe        :僅刪除Data，不刪除Table
*/
int inSqlite_Delete_Table_Data(char* szDBPath, char* szTableName)
{
	int		inRetVal;
	char		szDebugMsg[84 + 1];
	char		szDeleteSql[_SQL_SENTENCE_MAX_ + 1];	/* INSERT INTO	szTableName( */
	char		*szErrorMessage = NULL;
	sqlite3		*srDBConnection;	/* 建立到資料庫的connection */
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Delete_Table_Data()_START");
        }
	
	/* 開啟DataBase檔 */
	inRetVal = inSqlite_DB_Open_Or_Create(szDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	if (inRetVal != VS_SUCCESS)
	{	
		return (VS_ERROR);
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szDeleteSql, 0x00, sizeof(szDeleteSql));
	sprintf(szDeleteSql, "DELETE FROM %s", szTableName);
	
	/* Delete */
	inRetVal = sqlite3_exec(srDBConnection, szDeleteSql, 0, 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Delete Table Data ERROR Num:%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason:%s", szErrorMessage);
			inLogPrintf(AT, szDebugMsg);
		}
		
		if (szErrorMessage != NULL)
		{
			sqlite3_free(szErrorMessage);
		}
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "DELETE TABLE DATA OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Delete_Table_Data()_END");
                inLogPrintf(AT, "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Update_ByInvNum_SignState
Date&Time       :2019/6/5 下午 3:44
Describe        :UPDATE table_name SET column1 = value1, column2 = value2...., columnN = valueN WHERE [condition];
 *
*/
int inSqlite_Update_ByInvNum_SignState(TRANSACTION_OBJECT * pobTran, char* szDBPath, char* szTableName, int inInvoiceNumber)
{
	int			inRetVal = VS_ERROR;
	int			inSqlLength = 0;
	char			szDebugMsg[84 + 1] = {0};
	char			szSqlPrefix[100 + 1] = {0};	/* Update INTO	szTableName( */
	char			szSqlSuffix[40 + 1] = {0};	/* VALUES ( */
	char			szSqlSuffix2[200 + 1] = {0};	/* ); */
	char			szTag[_TAG_WIDTH_ + 1] = {0};
	char			szTagValue[2 + 1] = {0};
	char			*szUpdateSql = NULL;
	sqlite3			*srDBConnection = NULL;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat = NULL;
	SQLITE_ALL_TABLE	srAll;
	SQLITE_SQL_CALCULATE_TABLE	srSQLCal;
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Update_ByInvNum_TranState()_START");
        }
	
	/* 開啟DataBase檔 */
	inRetVal = inSqlite_DB_Open_Or_Create(szDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	if (inRetVal != VS_SUCCESS)
	{	
		return (VS_ERROR);
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "UPDATE %s", szTableName);
	
	memset(szSqlSuffix, 0x00, sizeof(szSqlSuffix));
	sprintf(szSqlSuffix, " SET ");
	
	memset(szSqlSuffix2, 0x00, sizeof(szSqlSuffix2));
	sprintf(szSqlSuffix2, " WHERE inTableID IN (SELECT inTableID FROM %s WHERE lnOrgInvNum = %d ORDER BY inTableID DESC LIMIT 1)", szTableName, inInvoiceNumber);
	
	memset(szTag, 0x00, sizeof(szTag));
	sprintf(szTag, "inSignStatus");
	
	memset(szTagValue, 0x00, sizeof(szTagValue));
	sprintf(szTagValue, "%d", pobTran->srBRec.inSignStatus);
	
	memset(&srAll, 0x00, sizeof(SQLITE_ALL_TABLE));
	srAll.inCharNum = 1;
	strcpy(srAll.srChar[0].szTag, szTag);
	srAll.srChar[0].pCharVariable = szTagValue;
	srAll.srChar[0].inTagValueLen = strlen(srAll.srChar[0].pCharVariable);
	
	/* 算要配置多少記憶體 */
	memset(&srSQLCal, 0x00, sizeof(SQLITE_SQL_CALCULATE_TABLE));
	srSQLCal.pSqlPrefix = szSqlPrefix;
	srSQLCal.pSqlSuffix = szSqlSuffix;
	srSQLCal.pSqlSuffix2 = szSqlSuffix2;
	
	inSqlite_Calculate_Update_SQLLength(&srSQLCal, &srAll, &inSqlLength);
	
	/* 配置記憶體 */
	szUpdateSql = malloc(inSqlLength + 1);
	memset(szUpdateSql, 0x00, inSqlLength);
	
	inSqlite_Gernerate_UpdateSQL(&srSQLCal, &srAll, szUpdateSql);
	
	/* Update */
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szUpdateSql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* Binding變數 */
	inSqlite_Binding_Values(&srAll, &srSQLStat);
	
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	do
	{
		inRetVal = sqlite3_step(srSQLStat);
		if (inRetVal == SQLITE_ROW)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "UPDATE Table OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else if (inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "UPDATE Table OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "UPDATE Table ERROR Num:%d", inRetVal);
				inLogPrintf(AT, szDebugMsg);

				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
				inLogPrintf(AT, szDebugMsg);

				inLogPrintf(AT, szUpdateSql);
			}

			/* 釋放事務 */
			inSqlite_SQL_Finalize(&srSQLStat);
			
			/* free */
			free(szUpdateSql);
			
			/* 關閉 database, close null pointer 是NOP(No Operation) */
			if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
			{
				/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
				return (VS_ERROR);
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "Close Database OK");
					inLogPrintf(AT, szDebugMsg);
				}
			}

			return (VS_ERROR);
		}
	
	}while (inRetVal != SQLITE_DONE);
	
	/* 釋放事務 */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	/* free */
	free(szUpdateSql);
			
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Update_ByInvNum_TranState()_END");
                inLogPrintf(AT, "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_DB_Close
Date&Time       :2019/9/6 上午 10:00
Describe        :關閉DB
*/
int inSqlite_DB_Close(sqlite3** srDBConnection)
{
	int             inRetVal = SQLITE_OK;
	char            szDebugMsg[84 + 1] = {0};
        sqlite3_stmt    *pStmt = NULL;
	
        if (srDBConnection == NULL || *srDBConnection == NULL)
        {
                vdUtility_SYSFIN_LogMessage(AT, "inSqlite_DB_Close *srDBConnection == NULL");
                inRetVal = SQLITE_ERROR;
        }
        else
        {
                while (sqlite3_close(*srDBConnection) == SQLITE_BUSY)
                {
                        /* 強制 finalize 所有未釋放的 statement */
                        pStmt = sqlite3_next_stmt(*srDBConnection, NULL);
                        if (pStmt != NULL)
                        {
                                vdUtility_SYSFIN_LogMessage(AT, "stmt not released");
                        }
                        
                        while (pStmt)
                        {
                                if (pStmt != NULL)
                                {
                                        vdUtility_SYSFIN_LogMessage(AT, "%s", sqlite3_sql(pStmt));
                                }
                                inSqlite_SQL_Finalize(&pStmt);
                                pStmt = sqlite3_next_stmt(*srDBConnection, NULL);
                        }
                }

                *srDBConnection = NULL;
        }
        
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Close Database File Failed");
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ErrorNum:%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
                vdUtility_SYSFIN_LogMessage(AT, "Close Database File Failed ErrorNum:%d", inRetVal);
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Close Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
		inFile_Open_File_Cnt_Decrease();
		
		return (VS_SUCCESS);
	}
	
}

/*
Function        :inSqlite_SQL_Finalize
Date&Time       :2019/10/24 下午 5:55
Describe        :
*/
int inSqlite_SQL_Finalize(sqlite3_stmt **srSQLStat)
{
	int	inRetVal = SQLITE_OK;
	char	szDebugMsg[84 + 1] = {0};
	
        if (srSQLStat != NULL && *srSQLStat != NULL)
        {
                sqlite3_finalize(*srSQLStat);
                *srSQLStat = NULL;
        }
        else
        {
                vdUtility_SYSFIN_LogMessage(AT, "inSqlite_SQL_Finalize *srSQLStat == NULL");
                inRetVal = SQLITE_ERROR;
        }
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "sqlite3_finalize Failed");
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ErrorNum:%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "sqlite3_finalize OK");
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_SUCCESS);
	}
	
}

/*
Function        :inSqlite_Insert_Record
Date&Time       :2022/5/10 上午 11:08
Describe        :Insert通用
*/
int inSqlite_Insert_Record(char* szDBPath, char* szTableName, SQLITE_ALL_TABLE* srAll)
{
	int			inRetVal;
	int			inSqlLength = 0;	/* 算組SQL語句的長度 */
	char			szDebugMsg[84 + 1] = {0};
	char			szSqlPrefix[100 + 1];	/* INSERT INTO	szTableName( */
	char			szSqlSuffix[20 + 1];	/* VALUES ( */
	char			szSqlSuffix2[10 + 1];	/* ); */
	char			*szInsertSql;
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	SQLITE_SQL_CALCULATE_TABLE	srSQLCal;
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Insert_All_Batch()_START");
        }
	
	inRetVal = inSqlite_DB_Open_Or_Create(szDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	if (inRetVal != VS_SUCCESS)
	{	
		return (VS_ERROR);
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szSqlPrefix, 0x00, sizeof(szSqlPrefix));
	sprintf(szSqlPrefix, "INSERT INTO %s (", szTableName);
	
	memset(szSqlSuffix, 0x00, sizeof(szSqlSuffix));
	sprintf(szSqlSuffix, ")VALUES (");
	
	memset(szSqlSuffix2, 0x00, sizeof(szSqlSuffix2));
	sprintf(szSqlSuffix2, ");");
	
	/* 算要配置多少記憶體 */
	memset(&srSQLCal, 0x00, sizeof(SQLITE_SQL_CALCULATE_TABLE));
	srSQLCal.pSqlPrefix = szSqlPrefix;
	srSQLCal.pSqlSuffix = szSqlSuffix;
	srSQLCal.pSqlSuffix2 = szSqlSuffix2;
	inSqlite_Calculate_Insert_SQLLength(&srSQLCal, srAll, &inSqlLength);
	
	/* 配置記憶體 */
	szInsertSql = malloc(inSqlLength + 100);
	memset(szInsertSql, 0x00, inSqlLength + 100);
	
	inSqlite_Gernerate_InsertSQL(&srSQLCal, srAll, szInsertSql);
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szInsertSql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* Binding變數 */
	inSqlite_Binding_Values(srAll, &srSQLStat);
	
	do
	{
		/* Insert */
		inRetVal = sqlite3_step(srSQLStat);
		if (inRetVal == SQLITE_ROW	||
		    inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Insert OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Insert ERROR Num:%d", inRetVal);
				inLogPrintf(AT, szDebugMsg);
				
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
				inLogPrintf(AT, szDebugMsg);
				
				inLogPrintf(AT, szInsertSql);
			}

		}
		
	}while (inRetVal == SQLITE_ROW);
	
	/* 釋放事務，若要重用則用sqlite3_reset */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	free(szInsertSql);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (inRetVal == SQLITE_ERROR)
	{
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Insert_All_Batch()_END");
                inLogPrintf(AT, "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Table_Link
Date&Time       :2022/5/13 下午 5:17
Describe        :
*/
int inSqlite_Table_Link(SQLITE_ALL_TABLE *srAll, int inLinkState, SQLITE_LINK_TABLE* srLink)
{
	int		i = 0;
	char		szDebugMsg[100 + 1] = {0};
	unsigned char	uszFailBit = VS_FALSE;
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBatch_Table_Link_Reprint_Title()_START");
        }
	
	do
	{
		for (i = 0;; i++)
		{
			if (srLink->psrInt == NULL)
			{
				break;
			}
			
			/* 碰到Table底部，設定Tag數並跳出 */
			if (strlen(srLink->psrInt[i].szTag) == 0)
			{
				break;
			}

			/* pointer為空，則跳過 */
			if (srLink->psrInt[i].pTagValue == NULL)
				continue;

			/* 變數多過於原來設定的Tag數 */
			if (srAll->inIntNum == _TAG_INT_MAX_NUM_)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Int變數過多");
					inLogPrintf(AT, szDebugMsg);
				}
				return (VS_ERROR);
			}

			/* Tag 名稱過長 */
			if (strlen(srLink->psrInt[i].szTag) > _TAG_MAX_LENGRH_)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Tag 名稱過長");
					inLogPrintf(AT, szDebugMsg);
				}
				return (VS_ERROR);
			}

			/* Insert時不用塞TableID */
			if (inLinkState == _LS_INSERT_)
			{
				/* 判斷長度是因為避免相同字首比對錯誤 */
				if ((memcmp(srLink->psrInt[i].szTag, "inTableID", strlen("inTableID")) == 0) && (strlen(srLink->psrInt[i].szTag) == strlen("inTableID")))
				{
					continue;
				}
			}

			strcat(srAll->srInt[srAll->inIntNum].szTag, srLink->psrInt[i].szTag);
			srAll->srInt[srAll->inIntNum].pTagValue = srLink->psrInt[i].pTagValue;
			srAll->inIntNum++;
		}
		
		/* 有誤中止 */
		if (uszFailBit == VS_TRUE)
		{
			break;
		}

		for (i = 0;; i++)
		{
			if (srLink->psrInt64t == NULL)
			{
				break;
			}
			
			/* 碰到Table底部，設定Tag數並跳出 */
			if (strlen(srLink->psrInt64t[i].szTag) == 0)
			{
				break;
			}

			/* pointer為空，則跳過 */
			if (srLink->psrInt64t[i].pTagValue == NULL)
				continue;

			/* 變數多過於原來設定的Tag數 */
			if (srAll->inInt64tNum == _TAG_INT64T_MAX_NUM_)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Int64t變數過多");
					inLogPrintf(AT, szDebugMsg);
				}
				return (VS_ERROR);
			}

			/* Tag 名稱過長 */
			if (strlen(srLink->psrInt64t[i].szTag) > _TAG_MAX_LENGRH_)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Tag 名稱過長");
					inLogPrintf(AT, szDebugMsg);
				}
				return (VS_ERROR);
			}

			strcat(srAll->srInt64t[srAll->inInt64tNum].szTag, srLink->psrInt64t[i].szTag);
			srAll->srInt64t[srAll->inInt64tNum].pTagValue = srLink->psrInt64t[i].pTagValue;
			srAll->inInt64tNum++;
		}

		/* 有誤中止 */
		if (uszFailBit == VS_TRUE)
		{
			break;
		}
		
		for (i = 0;; i++)
		{
			if (srLink->psrChar == NULL)
			{
				break;
			}
			
			/* 碰到Table底部，設定Tag數並跳出 */
			if (strlen(srLink->psrChar[i].szTag) == 0)
			{
				break;
			}

			/* pointer為空，則跳過 */
			if (srLink->psrChar[i].pCharVariable == NULL)
				continue;

			/* 變數多過於原來設定的Tag數 */
			if (srAll->inCharNum == _TAG_CHAR_MAX_NUM_)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Char變數過多");
					inLogPrintf(AT, szDebugMsg);
				}
				return (VS_ERROR);
			}

			/* Tag 名稱過長 */
			if (strlen(srLink->psrChar[i].szTag) > _TAG_MAX_LENGRH_)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Tag 名稱過長");
					inLogPrintf(AT, szDebugMsg);
				}
				return (VS_ERROR);
			}

			strcat(srAll->srChar[srAll->inCharNum].szTag, srLink->psrChar[i].szTag);
			srAll->srChar[srAll->inCharNum].pCharVariable = srLink->psrChar[i].pCharVariable;
			srAll->srChar[srAll->inCharNum].inTagValueLen = srLink->psrChar[i].inTagValueLen;
			srAll->inCharNum++;
		}
		
		/* 有誤中止 */
		if (uszFailBit == VS_TRUE)
		{
			break;
		}
		
		for (i = 0;; i++)
		{
			if (srLink->psrText == NULL)
			{
				break;
			}
			
			/* 碰到Table底部，設定Tag數並跳出 */
			if (strlen(srLink->psrText[i].szTag) == 0)
			{
				break;
			}

			/* pointer為空，則跳過 */
			if (srLink->psrText[i].pCharVariable == NULL)
				continue;

			/* 變數多過於原來設定的Tag數 */
			if (srAll->inTextNum == _TAG_TEXT_MAX_NUM_)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "TEXT變數過多");
					inLogPrintf(AT, szDebugMsg);
				}
				return (VS_ERROR);
			}

			/* Tag 名稱過長 */
			if (strlen(srLink->psrText[i].szTag) > _TAG_MAX_LENGRH_)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Tag 名稱過長");
					inLogPrintf(AT, szDebugMsg);
				}
				return (VS_ERROR);
			}

			strcat(srAll->srText[srAll->inTextNum].szTag, srLink->psrText[i].szTag);
			srAll->srText[srAll->inTextNum].pCharVariable = srLink->psrText[i].pCharVariable;
			srAll->srText[srAll->inTextNum].inTagValueLen = srLink->psrText[i].inTagValueLen;
			srAll->inTextNum++;
		}
		
		/* 有誤中止 */
		if (uszFailBit == VS_TRUE)
		{
			break;
		}
		break;
	}while(1);
	
	
	if (uszFailBit == VS_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Link 失敗");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Link 成功");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_Data_Enormous_Search
Date&Time       :2022/5/16 上午 9:10
Describe        :
*/
int inSqlite_Get_Data_Enormous_Search(char* szDBPath, char* szQuerySql)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[1024 + 1] = {0};
        char    szErrorMessage[300 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Get_Data_Enormous_Search(%s)_START", szDBPath);
		inLogPrintf(AT, "SQL(%s)", szQuerySql);
        }
	inRetVal = inSqlite_DB_Open_Or_Create(szDBPath, &gsrDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	/* Example:"SELECT * FROM %s WHERE (uszUpdated = 0)ORDER BY lnOrgInvNum ASC" */
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(gsrDBConnection, szQuerySql, -1, &gsrSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
                memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
		strcpy(szErrorMessage, sqlite3_errmsg(gsrDBConnection));
                
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", szErrorMessage);
			inLogPrintf(AT, szDebugMsg);
		}
                
                /* 因為直接get 不存在的table回傳值是-1，只有在Error Msg才能得知錯誤原因 */
		if (memcmp(szErrorMessage, "no such table", strlen("no such table")) == 0)
		{
			return (VS_NO_RECORD);
		}
		else
		{
			return (VS_ERROR);
		}
	}
	
	/* 初始化計數 */
	ginEnormousNowCnt = 0;
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Get_Data_Enormous_Search()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_Data_Enormous_Get
Date&Time       :2022/5/16 上午 9:25
Describe        :
*/
int inSqlite_Get_Data_Enormous_Get(SQLITE_ALL_TABLE *srAll, int inRecCnt)
{
	int			inRetVal = 0;
	char			szDebugMsg[100 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inSqlite_Get_Data_Enormous_Get() START !");
	}
	
	/* 延續使用，不重新搜尋 */
	if (guszEnormousNoNeedResetBit == VS_TRUE)
	{
		
	}
	else
	{
		/* 重置前一次結果 */
		sqlite3_reset(gsrSQLStat);
		ginEnormousNowCnt = 0;
	}
	
	/* 取得 database 裡所有的資料 */
	for (; ginEnormousNowCnt <= inRecCnt; ginEnormousNowCnt++)
	{
		/* Qerry */
		inRetVal = sqlite3_step(gsrSQLStat);
		if (inRetVal == SQLITE_ROW)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Get Table OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else if (inRetVal == SQLITE_DONE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "NO DATA");
				inLogPrintf(AT, szDebugMsg);
			}
			
			return (VS_NO_RECORD);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
				inLogPrintf(AT, szDebugMsg);

				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(gsrDBConnection));
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
	}
	
	/* binding 取得值 */
	inSqlite_Get_Binding_Value(&gsrSQLStat, srAll);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inSqlite_Get_Data_Enormous_Get() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Get_Data_Enormous_Free
Date&Time       :2022/5/16 上午 9:54
Describe        :用完資料Free
*/
int inSqlite_Get_Data_Enormous_Free()
{
	char	szDebugMsg[100 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inSqlite_Get_Data_Enormous_Free() START !");
	}
	
	/* 釋放事務 */
	inSqlite_SQL_Finalize(&gsrSQLStat);
	
	/* 計數歸0 */
	ginEnormousNowCnt = 0;
	guszEnormousNoNeedResetBit = VS_FALSE;
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&gsrDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inSqlite_Get_Data_Enormous_Free() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Delete_Record_By_External_SQL
Date&Time       :2022/5/18 下午 1:57
Describe        :
*/
int inSqlite_Delete_Record_By_External_SQL(char* szDBName, char* szTableName, char* szSQLStatement)
{
	int		inRetVal;
	char		szDebugMsg[84 + 1];
	char		szQuerySql[_SQL_SENTENCE_MAX_ + 1];	/* INSERT INTO	szTableName( */
	char		*szErrorMessage = NULL;
	sqlite3		*srDBConnection;	/* 建立到資料庫的connection */
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Delete_Record_By_External_SQL()_START");
        }
	
	inRetVal = inSqlite_DB_Open_Or_Create(szDBName, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	/* example: "DELETE FROM %s WHERE inTableID = (SELECT MIN(inTableID) FROM %s)" */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "DELETE FROM %s", szTableName);
	if (szSQLStatement != NULL)
	{
		strcat(szQuerySql, " ");
		strcat(szQuerySql, szSQLStatement);
	}
	
	/* 取得 database 裡所有的資料 */
	inRetVal = sqlite3_exec(srDBConnection , szQuerySql, 0, 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Delete Record ERROR Num:%d,", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason:%s", szErrorMessage);
			inLogPrintf(AT, szDebugMsg);
			inLogPrintf(AT, szQuerySql);
		}
		
		if (szErrorMessage != NULL)
		{
			sqlite3_free(szErrorMessage);
		}
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Delete Record OK");
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inSqlite_Delete_Record_By_External_SQL()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Calculate_Insert_SQLLength
Date&Time       :2022/5/19 下午 1:58
Describe        :
*/
int inSqlite_Calculate_Insert_SQLLength(SQLITE_SQL_CALCULATE_TABLE* srSQLCal, SQLITE_ALL_TABLE* srAll, int* inSqlLength)
{
	int	i = 0;
	char	szDebugMsg[84 + 1] = {0};
	
	/* 算要配置多少記憶體 */
	if (srSQLCal->pSqlPrefix != NULL)
	{
		*inSqlLength += strlen(srSQLCal->pSqlPrefix);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "COUNT");
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "IntTag: %d", srAll->inIntNum);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Int64tTag: %d", srAll->inInt64tNum);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "CharTag: %d", srAll->inCharNum);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "TextTag: %d", srAll->inTextNum);
		inLogPrintf(AT, szDebugMsg);
	}
	
	for (i = 0; i < srAll->inIntNum; i++)
	{
		*inSqlLength += strlen(srAll->srInt[i].szTag);
		/* Comma */
		*inSqlLength += 2;
	}
	
	for (i = 0; i < srAll->inInt64tNum; i++)
	{
		*inSqlLength += strlen(srAll->srInt64t[i].szTag);
		/* Comma */
		*inSqlLength += 2;
	}
	
	for (i = 0; i < srAll->inCharNum; i++)
	{
		*inSqlLength += strlen(srAll->srChar[i].szTag);
		/* Comma */
		*inSqlLength += 2;
	}
	
	for (i = 0; i < srAll->inTextNum; i++)
	{
		*inSqlLength += strlen(srAll->srText[i].szTag);
		/* Comma */
		*inSqlLength += 2;
	}
	
	if (srSQLCal->pSqlSuffix != NULL)
	{
		/* 第一行最後面的) */
		(*inSqlLength) ++;
		/* 第二行"VALUES ("的長度 */
		*inSqlLength += strlen(srSQLCal->pSqlSuffix);
	}
	
	for (i = 0; i < srAll->inIntNum; i++)
	{
		/* 用問號 */
		*inSqlLength += 1;
		/* Comma & Space */
		*inSqlLength += 2;
	}
	
	for (i = 0; i < srAll->inInt64tNum; i++)
	{
		/* 用問號 */
		*inSqlLength += 1;
		/* Comma & Space */
		*inSqlLength += 2;
	}
	
	for (i = 0; i < srAll->inCharNum; i++)
	{
		/* 用問號 */
		*inSqlLength += 1;
		/* Comma & Space & 兩個單引號 */
		*inSqlLength += 2;
	}
	
	for (i = 0; i < srAll->inTextNum; i++)
	{
		/* 用問號 */
		*inSqlLength += 1;
		/* Comma & Space & 兩個單引號 */
		*inSqlLength += 2;
	}
	
	if (srSQLCal->pSqlSuffix2 != NULL)
	{
		/* ); */
		*inSqlLength += strlen(srSQLCal->pSqlSuffix2);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inSqlLength: %d", *inSqlLength);
		inLogPrintf(AT, szDebugMsg);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Calculate_Insert_SQLLength
Date&Time       :2022/5/19 下午 1:58
Describe        :
*/
int inSqlite_Calculate_Update_SQLLength(SQLITE_SQL_CALCULATE_TABLE* srSQLCal, SQLITE_ALL_TABLE* srAll, int* inSqlLength)
{
	int	i = 0;
	char	szDebugMsg[84 + 1] = {0};
	
	/* 算要配置多少記憶體 */
	if (srSQLCal->pSqlPrefix != NULL)
	{
		*inSqlLength += strlen(srSQLCal->pSqlPrefix);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "COUNT");
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "IntTag: %d", srAll->inIntNum);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Int64tTag: %d", srAll->inInt64tNum);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "CharTag: %d", srAll->inCharNum);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "TextTag: %d", srAll->inTextNum);
		inLogPrintf(AT, szDebugMsg);
	}
	
	for (i = 0; i < srAll->inIntNum; i++)
	{
		/* Comma + space （i = 0時不放", "，但這裡只是分配空間，所以可以忽略 ） */
		*inSqlLength += 2;
		*inSqlLength += strlen(srAll->srInt[i].szTag);
		/* space + " = " + space*/
		*inSqlLength += 3;
		/* " ? " */
		*inSqlLength += 1;
	}
	
	for (i = 0; i < srAll->inInt64tNum; i++)
	{
		/* Comma + space */
		*inSqlLength += 2;
		*inSqlLength += strlen(srAll->srInt64t[i].szTag);
		/* space + " = " + space*/
		*inSqlLength += 3;
		/* " ? " */
		*inSqlLength += 1;
	}
	
	for (i = 0; i < srAll->inCharNum; i++)
	{
		/* Comma + space */
		*inSqlLength += 2;
		*inSqlLength += strlen(srAll->srChar[i].szTag);
		/* space + " = " + space*/
		*inSqlLength += 3;
		/* 根據字串長度 */
		*inSqlLength += strlen(srAll->srChar[i].pCharVariable);
		/* " ? " */
		*inSqlLength += 1;
	}
	
	for (i = 0; i < srAll->inTextNum; i++)
	{
		/* Comma + space */
		*inSqlLength += 2;
		*inSqlLength += strlen(srAll->srText[i].szTag);
		/* space + " = " + space*/
		*inSqlLength += 3;
		/* 根據字串長度 */
		*inSqlLength += strlen(srAll->srText[i].pCharVariable);
		/* " ? " */
		*inSqlLength += 1;
	}
	
	if (srSQLCal->pSqlSuffix != NULL)
	{
		/* "SET "的長度 */
		*inSqlLength += strlen(srSQLCal->pSqlSuffix);
	}
	
	if (srSQLCal->pSqlSuffix2 != NULL)
	{
		/* ); */
		*inSqlLength += strlen(srSQLCal->pSqlSuffix2);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inSqlLength: %d", *inSqlLength);
		inLogPrintf(AT, szDebugMsg);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Gernerate_InsertSQL
Date&Time       :2022/5/19 下午 1:58
Describe        :
*/
int inSqlite_Gernerate_InsertSQL(SQLITE_SQL_CALCULATE_TABLE* srSQLCal, SQLITE_ALL_TABLE* srAll, char* szSQL)
{
	int	i = 0;
	char	szTemplate[100 + 1];
	char	szDebugMsg[84 + 1] = {0};
	
	if (srSQLCal->pSqlPrefix != NULL)
	{
		/* 先丟Table Name */
		strcat(szSQL, srSQLCal->pSqlPrefix);
	}
	
	if (srAll != NULL)
	{
		/* 看上一個table是不是空的，有東西的話，第一項前面要加comma，但這已經是第一個table，所以放0 */
		if (0 > 0)
		{
			strcat(szSQL, ", ");
		}

		for (i = 0; i < srAll->inIntNum; i++)
		{
			if (i > 0)
			{
				strcat(szSQL, ", ");
			}
			strcat(szSQL, srAll->srInt[i].szTag);
		}

		/* 看上一個table是不是空的，有東西的話，第一項前面要加comma */
		if (srAll->inIntNum > 0)
		{
			strcat(szSQL, ", ");
		}

		for (i = 0; i < srAll->inInt64tNum; i++)
		{
			if (i > 0)
			{
				strcat(szSQL, ", ");
			}
			strcat(szSQL, srAll->srInt64t[i].szTag);
		}

		/* 看上一個table是不是空的，有東西的話，第一項前面要加comma */
		if (srAll->inInt64tNum > 0)
		{
			strcat(szSQL, ", ");
		}

		for (i = 0; i < srAll->inCharNum; i++)
		{

			if (i > 0)
			{
				strcat(szSQL, ", ");
			}
			strcat(szSQL, srAll->srChar[i].szTag);
		}
		
		/* 看上一個table是不是空的，有東西的話，第一項前面要加comma */
		if (srAll->inCharNum > 0)
		{
			strcat(szSQL, ", ");
		}

		for (i = 0; i < srAll->inTextNum; i++)
		{

			if (i > 0)
			{
				strcat(szSQL, ", ");
			}
			strcat(szSQL, srAll->srText[i].szTag);
		}

		/* 為了避免有空的table導致多塞, */
		if (memcmp((szSQL + strlen(szSQL) - 2), ", ", 2) == 0)
		{
			memset(szSQL + strlen(szSQL) - 2, 0x00, 2);
		}
	}
	
	if (srSQLCal->pSqlSuffix != NULL)
	{
		/* ")VALUES (" */
		strcat(szSQL, srSQLCal->pSqlSuffix);
		memset(szTemplate, 0x00, sizeof(szTemplate));
	}
	
	if (srAll != NULL)
	{
		for (i = 0; i < srAll->inIntNum; i++)
		{
			if (i == 0)
			{
				sprintf(szTemplate, "?");
			}
			else
			{
				sprintf(szTemplate, ", ?");
			}
			strcat(szSQL, szTemplate);
			/* 只清自己用過得長度 comma(1) + space(1) + ?(1) */
			memset(szTemplate, 0x00, 3);
		}

		/* 代表上一個table有東西，要加comma */
		if (srAll->inIntNum > 0)
		{
			strcat(szSQL, ", ");
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Int Insert OK");
			inLogPrintf(AT, szDebugMsg);
		}

		for (i = 0; i < srAll->inInt64tNum; i++)
		{
			if (i == 0)
			{
				sprintf(szTemplate, "?");
			}
			else
			{
				sprintf(szTemplate, ", ?");
			}
			strcat(szSQL, szTemplate);
			/* 只清自己用過得長度 comma(1) + space(1) + ?(1) */
			memset(szTemplate, 0x00, 3);
		}

		/* 代表上一個table有東西，要加comma */
		if (srAll->inInt64tNum > 0)
		{
			strcat(szSQL, ", ");
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Int64t Insert OK");
			inLogPrintf(AT, szDebugMsg);
		}

		for (i = 0; i < srAll->inCharNum; i++)
		{
			if (i == 0)
			{
				sprintf(szTemplate, "?");
			}
			else
			{
				sprintf(szTemplate, ", ?");
			}
			strcat(szSQL, szTemplate);
			/* 只清自己用過得長度 comma(1) + space(1) + ?(1) */
			memset(szTemplate, 0x00, 3);
		}

		/* 代表上一個table有東西，要加comma */
		if (srAll->inCharNum > 0)
		{
			strcat(szSQL, ", ");
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Char Insert OK");
			inLogPrintf(AT, szDebugMsg);
		}

		for (i = 0; i < srAll->inTextNum; i++)
		{
			if (i == 0)
			{
				sprintf(szTemplate, "?");
			}
			else
			{
				sprintf(szTemplate, ", ?");
			}
			strcat(szSQL, szTemplate);
			/* 只清自己用過得長度 comma(1) + space(1) + ?(1) */
			memset(szTemplate, 0x00, 3);
		}

		/* 代表上一個table有東西，要加comma */
		if (srAll->inTextNum > 0)
		{
			strcat(szSQL, ", ");
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Text Insert OK");
			inLogPrintf(AT, szDebugMsg);
		}

		/* 為了避免有空的table導致多塞, */
		if (memcmp((szSQL + strlen(szSQL) - 2), ", ", 2) == 0)
		{
			memset(szSQL + strlen(szSQL) - 2, 0x00, 2);
		}
	}
	
	if (srSQLCal->pSqlSuffix2 != NULL)
	{
		/* 最後面的); */
		strcat(szSQL, srSQLCal->pSqlSuffix2);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Gernerate_UpdateSQL
Date&Time       :2022/5/19 下午 3:08
Describe        :
*/
int inSqlite_Gernerate_UpdateSQL(SQLITE_SQL_CALCULATE_TABLE* srSQLCal, SQLITE_ALL_TABLE* srAll, char* szSQL)
{
	int	i = 0;
	char	szTemplate[100 + 1];
	
	if (srSQLCal->pSqlPrefix != NULL)
	{
		/* 先丟Table Name */
		strcat(szSQL, srSQLCal->pSqlPrefix);
	}
	
	if (srSQLCal->pSqlSuffix != NULL)
	{
		/* " SET " */
		strcat(szSQL, srSQLCal->pSqlSuffix);
		memset(szTemplate, 0x00, sizeof(szTemplate));
	}
	
	if (srAll != NULL)
	{
		/* 看上一個table是不是空的，有東西的話，第一項前面要加comma，但這已經是第一個table，所以放0 */
		if (0 > 0)
		{
			strcat(szSQL, ", ");
		}

		for (i = 0; i < srAll->inIntNum; i++)
		{
			if (i > 0)
			{
				strcat(szSQL, ", ");
			}
			strcat(szSQL, srAll->srInt[i].szTag);
			strcat(szSQL, " = ");
			strcat(szSQL, "?");
		}

		/* 看上一個table是不是空的，有東西的話，第一項前面要加comma */
		if (srAll->inIntNum > 0)
		{
			strcat(szSQL, ", ");
		}

		for (i = 0; i < srAll->inInt64tNum; i++)
		{
			if (i > 0)
			{
				strcat(szSQL, ", ");
			}
			strcat(szSQL, srAll->srInt64t[i].szTag);
			strcat(szSQL, " = ");
			strcat(szSQL, "?");
		}

		/* 看上一個table是不是空的，有東西的話，第一項前面要加comma */
		if (srAll->inInt64tNum > 0)
		{
			strcat(szSQL, ", ");
		}

		for (i = 0; i < srAll->inCharNum; i++)
		{

			if (i > 0)
			{
				strcat(szSQL, ", ");
			}
			strcat(szSQL, srAll->srChar[i].szTag);
			strcat(szSQL, " = ");
			strcat(szSQL, "?");
		}
		
		/* 看上一個table是不是空的，有東西的話，第一項前面要加comma */
		if (srAll->inCharNum > 0)
		{
			strcat(szSQL, ", ");
		}

		for (i = 0; i < srAll->inTextNum; i++)
		{

			if (i > 0)
			{
				strcat(szSQL, ", ");
			}
			strcat(szSQL, srAll->srText[i].szTag);
			strcat(szSQL, " = ");
			strcat(szSQL, "?");
		}

		/* 為了避免有空的table導致多塞, */
		if (memcmp((szSQL + strlen(szSQL) - 2), ", ", 2) == 0)
		{
			memset(szSQL + strlen(szSQL) - 2, 0x00, 2);
		}
	}
	
	if (srSQLCal->pSqlSuffix2 != NULL)
	{
		/* 最後面的); */
		strcat(szSQL, srSQLCal->pSqlSuffix2);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Binding_Values
Date&Time       :2022/5/19 下午 2:36
Describe        :
*/
int inSqlite_Binding_Values(SQLITE_ALL_TABLE* srAll, sqlite3_stmt** srSQLStat)
{
	int	i = 0;
	int	inBindingIndex = 1;	/* binding的index從1開始 */
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[84 + 1] = {0};
	
	/* Binding變數 */
	for (i = 0; i < srAll->inIntNum; i++)
	{
		inRetVal = sqlite3_bind_int(*srSQLStat, inBindingIndex, *(int32_t*)srAll->srInt[i].pTagValue);
		if (inRetVal != SQLITE_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Binging Int Fail: %d", inRetVal);
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else
		{
			inBindingIndex++;
		}
		
	}
	
	for (i = 0; i < srAll->inInt64tNum; i++)
	{
		inRetVal = sqlite3_bind_int64(*srSQLStat, inBindingIndex, *(int64_t*)srAll->srInt64t[i].pTagValue);
		if (inRetVal != SQLITE_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Binging Int64t Fail: %d", inRetVal);
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else
		{
			inBindingIndex++;
		}
	}
	
	for (i = 0; i < srAll->inCharNum; i++)
	{
		inRetVal = sqlite3_bind_blob(*srSQLStat, inBindingIndex, srAll->srChar[i].pCharVariable, srAll->srChar[i].inTagValueLen, NULL);
		if (inRetVal != SQLITE_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Binging Char Fail: %d", inRetVal);
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else
		{
			inBindingIndex++;
		}
	}
	
	for (i = 0; i < srAll->inTextNum; i++)
	{
		inRetVal = sqlite3_bind_text(*srSQLStat, inBindingIndex, srAll->srText[i].pCharVariable, srAll->srText[i].inTagValueLen, NULL);
		if (inRetVal != SQLITE_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Binging Text Fail: %d", inRetVal);
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else
		{
			inBindingIndex++;
		}
	}
	
	return (VS_SUCCESS);
}

/*
 Function        :inSqlite_fsync_retry
 Date&Time       :2025/10/7 下午 1:23
 Describe        :EINTR 安全的 fsync
 */
static int inSqlite_fsync_retry(int fd)
{
	do
	{
		if (fsync(fd) == 0)
			return (VS_SUCCESS);
		if (errno == EINTR)
			continue;
		break;
	}while(1);
	
	return (VS_ERROR);
}

/*
 Function        :inSqlite_Fsync_DB
 Date&Time       :2025/10/7 下午 1:56
 Describe        :
 */
/* 只同步交易 DB 檔案，避免全域 sync() 造成整機阻塞與誤觸 WDT */
int inSqlite_Fsync_DB(char* szFileName)
{
	int	inFd = 0;
	int	inOk = 0;
	int	inRetVal = VS_SUCCESS;
	int	inErrno = 0;
	
	if (szFileName[0] == '\0')
	{
		vdUtility_SYSFIN_LogMessage(AT, "inSqlite_Fsync_DB FileName Err(%s)", szFileName);

		return (VS_ERROR);
	}

	inRetVal = inFile_Linux_Open(&inFd, szFileName);
	if (inRetVal != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inSqlite_Fsync_DB open fail(%s), errno=%d", szFileName, errno);
		
		return (VS_ERROR);
	}

	if (inSqlite_fsync_retry(inFd) == VS_SUCCESS)
	{
		inOk = VS_SUCCESS;
	}
	
	inErrno =  errno;
	
	if (inFile_Linux_Close(inFd) != VS_SUCCESS)
	{
		inFunc_EDCLock(AT);
	}

	if (inOk != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inSqlite_Fsync_DB fsync fail (%s),errno=%d", szFileName, inErrno);
		
		return (VS_ERROR);
	}

	vdUtility_SYSFIN_LogMessage(AT, "inSqlite_Fsync_DB OK(%s)", szFileName);
	
	return (VS_SUCCESS);
}

/*
 Function        :inSqlite_Fsync_TranDB
 Date&Time       :2025/10/7 下午 1:56
 Describe        :
 */
/* 只同步交易 DB 檔案，避免全域 sync() 造成整機阻塞與誤觸 WDT */
int inSqlite_Fsync_TranDB(void)
{
	int	inRetVal = VS_SUCCESS;
	
	inRetVal = inSqlite_Fsync_DB(gszTranDBPath);
	
	return (inRetVal);
}

/*
Function        :inSqlite_Table_Link_Addition_Int
Date&Time       :2025/10/15 上午 11:01
Describe        :額外想讀出欄位，例如sum(*),Cont(*)
*/
int inSqlite_Table_Link_Addition_Int(SQLITE_ALL_TABLE *srAll, char *szTag, void *vdInt)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned char	uszFailBit = VS_FALSE;
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Table_Link_Addition()_START");
        }
	
	do
	{
                /* Tag為0錯誤 */
                if (strlen(szTag) == 0)
                {
                        uszFailBit = VS_TRUE;
                        vdUtility_SYSFIN_LogMessage(AT, "inSqlite_Table_Link_Addition No Tag Err");
                        break;
                }

                /* pointer為空，則錯誤 */
                if (vdInt == NULL)
                {
                        uszFailBit = VS_TRUE;
                        vdUtility_SYSFIN_LogMessage(AT, "inSqlite_Table_Link_Addition PTR_Null_Err");
                        break;
                }

                /* 變數多過於原來設定的Tag數 */
                if (srAll->inIntNum == _TAG_INT_MAX_NUM_)
                {
                        vdUtility_SYSFIN_LogMessage(AT, "inSqlite_Table_Link_Addition Int_Tag_Num_Too_much");
                        if (ginDebug == VS_TRUE)
                        {
                                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                sprintf(szDebugMsg, "Int變數過多");
                                inLogPrintf(AT, szDebugMsg);
                        }
                        return (VS_ERROR);
                }

                /* Tag 名稱過長 */
                if (strlen(szTag) > _TAG_MAX_LENGRH_)
                {
                        vdUtility_SYSFIN_LogMessage(AT, "inSqlite_Table_Link_Addition Int_Tag_Name_Too_long");
                        if (ginDebug == VS_TRUE)
                        {
                                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                sprintf(szDebugMsg, "Tag 名稱過長");
                                inLogPrintf(AT, szDebugMsg);
                        }
                        return (VS_ERROR);
                }

                strcat(srAll->srInt[srAll->inIntNum].szTag, szTag);
                srAll->srInt[srAll->inIntNum].pTagValue = vdInt;
                srAll->inIntNum++;
		
		/* 有誤中止 */
		if (uszFailBit == VS_TRUE)
		{
			break;
		}

		break;
	}while(1);
	
	
	if (uszFailBit == VS_TRUE)
	{
                vdUtility_SYSFIN_LogMessage(AT, "inSqlite_Table_Link_Addition Table_Link_Fail (%s)", szTag);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Link 失敗");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Link 成功");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Table_Link_Addition_Int64
Date&Time       :2025/10/15 上午 11:01
Describe        :額外想讀出欄位，例如sum(*),Cont(*)
*/
int inSqlite_Table_Link_Addition_Int64(SQLITE_ALL_TABLE *srAll, char *szTag, void *vdInt)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned char	uszFailBit = VS_FALSE;
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inSqlite_Table_Link_Addition_Int64()_START");
        }
	
	do
	{
                /* Tag為0錯誤 */
                if (strlen(szTag) == 0)
                {
                        uszFailBit = VS_TRUE;
                        vdUtility_SYSFIN_LogMessage(AT, "inSqlite_Table_Link_Addition_Int64 No Tag Err");
                        break;
                }

                /* pointer為空，則錯誤 */
                if (vdInt == NULL)
                {
                        uszFailBit = VS_TRUE;
                        vdUtility_SYSFIN_LogMessage(AT, "inSqlite_Table_Link_Addition_Int64 PTR_Null_Err");
                        break;
                }

                /* 變數多過於原來設定的Tag數 */
                if (srAll->inIntNum == _TAG_INT64T_MAX_NUM_)
                {
                        vdUtility_SYSFIN_LogMessage(AT, "inSqlite_Table_Link_Addition_Int64 Int64T_Tag_Num_Too_much");
                        if (ginDebug == VS_TRUE)
                        {
                                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                sprintf(szDebugMsg, "Int64變數過多");
                                inLogPrintf(AT, szDebugMsg);
                        }
                        return (VS_ERROR);
                }

                /* Tag 名稱過長 */
                if (strlen(szTag) > _TAG_MAX_LENGRH_)
                {
                        vdUtility_SYSFIN_LogMessage(AT, "inSqlite_Table_Link_Addition_Int64 Int_Tag_Name_Too_long");
                        if (ginDebug == VS_TRUE)
                        {
                                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                sprintf(szDebugMsg, "Tag 名稱過長");
                                inLogPrintf(AT, szDebugMsg);
                        }
                        return (VS_ERROR);
                }

                strcat(srAll->srInt64t[srAll->inInt64tNum].szTag, szTag);
                srAll->srInt64t[srAll->inInt64tNum].pTagValue = vdInt;
                srAll->inInt64tNum++;
		
		/* 有誤中止 */
		if (uszFailBit == VS_TRUE)
		{
			break;
		}

		break;
	}while(1);
	
	
	if (uszFailBit == VS_TRUE)
	{
                vdUtility_SYSFIN_LogMessage(AT, "inSqlite_Table_Link_Addition_Int64 Table_Link_Fail (%s)", szTag);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Link 失敗");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Link 成功");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inSqlite_Reset_Find_State
Date&Time       :2025/10/17 下午 2:44
Describe        :重置srAll裡面uszIsFind的狀態，uszIsFind用來表示該tag是否以被塞過值，用來加快給值的速度
*/
int inSqlite_Reset_Find_State(SQLITE_ALL_TABLE* srAll)
{
	int	inIntIndex = 0, inInt64tIndex = 0, inCharIndex = 0, inTextIndex = 0;

	/* binding 取得值 */
	for (inIntIndex = 0; inIntIndex < srAll->inIntNum; inIntIndex++)
	{
		srAll->srInt[inIntIndex].uszIsFind = VS_FALSE;
	}

        for (inInt64tIndex = 0; inInt64tIndex < srAll->inInt64tNum; inInt64tIndex++)
        {
                srAll->srInt64t[inInt64tIndex].uszIsFind = VS_FALSE;
        }

        for (inCharIndex = 0; inCharIndex < srAll->inCharNum; inCharIndex++)
        {
                srAll->srChar[inCharIndex].uszIsFind = VS_FALSE;
        }

        for (inTextIndex = 0; inTextIndex < srAll->inTextNum; inTextIndex++)
        {

		srAll->srText[inTextIndex].uszIsFind = VS_FALSE;
	}
	
	
	return (VS_SUCCESS);
}


int inSqlite_CheckPrepareCommStatus(int inStuts)
{
	int inRetVal = VS_SUCCESS;

	if (inStuts == SQLITE_SCHEMA || inStuts == SQLITE_BUSY) {
		inRetVal = VS_ESCAPE;
	} else if (inStuts == SQLITE_CORRUPT || inStuts == SQLITE_NOTADB) {
		inRetVal = VS_ERROR;
	}

	return VS_ESCAPE;
}