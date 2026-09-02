/* 
 * File:   EDC_Para_Table_Func.h
 * Author: RussellBai
 *
 * Created on 2019年4月23日, 下午 1:53
 */

#define	_DATA_BASE_NAME_NEXSYS_PARAMETER_			"nexsys_param.db"

int inEDCPara_Create_Table_Flow(char* szDBName, char* szTableName, SQLITE_TAG_TABLE* pobSQLTag);