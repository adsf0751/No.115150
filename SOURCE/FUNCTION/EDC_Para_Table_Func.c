#include <string.h>
#include <stdio.h>
#include <sqlite3.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/AllStruct.h"
#include "../DISPLAY/Display.h"
#include "Sqlite.h"
#include "Batch.h"
#include "Utility.h"
#include "EDC_Para_Table_Func.h"

extern	int		ginDebug;		/* Debug使用 extern */
extern	char		gszParamDBPath[100 + 1];
/*
Function        :inEDCPara_Create_Table_Flow
Date&Time       :2019/4/23 下午 1:57
Describe        :在這邊決定名稱並分流
*/
int inEDCPara_Create_Table_Flow(char* szDBName, char* szTableName, SQLITE_TAG_TABLE* pobSQLTag)
{
	int	inRetVal = VS_SUCCESS;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inEDCPara_Create_Table_Flow() START !");
		inLogPrintf(AT, "DBName:%s", szDBName);
		inLogPrintf(AT, "szTableName:%s", szTableName);
	}
	
	/* DBName不合法 */
	if (szDBName == NULL	||
	    strlen(szDBName) == 0)
	{
		if (szDBName == NULL)
		{
			inUtility_StoreTraceLog_OneStep("szDBName null pointer");
		}
		else if (strlen(szDBName) == 0)
		{
			inUtility_StoreTraceLog_OneStep("szDBName Length = 0");
		}
		
		return (VS_ERROR);
	}
	
	/* TableName 不合法 */
	if (szTableName == NULL	||
	    strlen(szTableName) == 0)
	{
		if (szTableName == NULL)
		{
			inUtility_StoreTraceLog_OneStep("szTableName null pointer");
		}
		else if (strlen(szTableName) == 0)
		{
			inUtility_StoreTraceLog_OneStep("szDBName Length = 0");
		}
		
		return (VS_ERROR);
	}
		
	inRetVal = inSqlite_Create_Table(gszParamDBPath, szTableName, pobSQLTag);
	
	return (inRetVal);
}