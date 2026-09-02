/* 
 * File:   Sqlite_Struct.h
 * Author: RussellBai
 *
 * Created on 2022年5月26日, 下午 4:44
 */

#ifndef SQLITE_STRUCT_H
#define	SQLITE_STRUCT_H

#ifdef	__cplusplus
extern "C" {
#endif

#define _SQL_SENTENCE_MAX_		600
#define	_TAG_WIDTH_			_SQL_SENTENCE_MAX_+1	/* 目前最長的Tag Name為30 *//* exist會用查詢語句當tag，所以長度應大於查詢語句限制 */
#define _TAG_TYPE_WIDTH_		20	/* 型別 */
#define _ADDITIONAL_ATTRIBUTE_1_	20	/* 屬性1 */
#define _ADDITIONAL_ATTRIBUTE_2_	20	/* 屬性2 */

#define _TAG_MAX_LENGRH_		50	/* Tag最長長度 */
#define _TAG_INT_MAX_NUM_		100
#define _TAG_INT64T_MAX_NUM_		100
#define _TAG_CHAR_MAX_NUM_		300	/* 截至2018/1/15 下午 3:54為止，用了115個，若需要在往上調 */
#define _TAG_TEXT_MAX_NUM_		300

/* 儲存變數的Table */
typedef struct
{
	char	szTag[_TAG_WIDTH_];
	char	szType[_TAG_TYPE_WIDTH_];
	char	szAttribute1[_ADDITIONAL_ATTRIBUTE_1_];
	char	szAttribute2[_ADDITIONAL_ATTRIBUTE_2_];
}SQLITE_TAG_TABLE;

typedef struct
{
	unsigned char	uszIsFind;			/* 若讀取時已找到，設為1 */
	char		szTag[_TAG_MAX_LENGRH_];
        void		*pTagValue;
}SQLITE_INT32T_TABLE;

typedef struct
{
	unsigned char	uszIsFind;			/* 若讀取時已找到，設為1 */
	char		szTag[_TAG_MAX_LENGRH_];
        void		*pCharVariable;
	int		inTagValueLen;
}SQLITE_CHAR_TABLE;

typedef struct
{
	char	*Variable;
        int	(*inTagUpdate)(TRANSACTION_OBJECT *, unsigned char *); /* 將pobTran資料update到資料庫中 */
}SQLITE_UPDATE_TABLE;

typedef struct
{
	int			inIntNum;
	int			inInt64tNum;
	int			inCharNum;
	int			inTextNum;
	SQLITE_INT32T_TABLE	srInt[_TAG_INT_MAX_NUM_];
	SQLITE_INT32T_TABLE	srInt64t[_TAG_INT64T_MAX_NUM_];
	SQLITE_CHAR_TABLE	srChar[_TAG_CHAR_MAX_NUM_];
	SQLITE_CHAR_TABLE	srText[_TAG_TEXT_MAX_NUM_];	/* 用BLOB的API塞進去的資料無法用字串比對，所以再生一個Table，EMV參數則維持使用BLOB塞*/
}SQLITE_ALL_TABLE;

/* 避免大量查詢的方法 */
typedef struct
{
	int	inRows;
	int	inCols;
	char**	szResult;
}SQLITE_RESULT;

typedef struct
{
	SQLITE_INT32T_TABLE*	psrInt;
	SQLITE_INT32T_TABLE*	psrInt64t;
	SQLITE_CHAR_TABLE*	psrChar;
	SQLITE_CHAR_TABLE*	psrText;
}
SQLITE_LINK_TABLE;

typedef struct
{
	char*	pSqlPrefix;
	char*	pSqlSuffix;
	char*	pSqlSuffix2;
}SQLITE_SQL_CALCULATE_TABLE;

#ifdef	__cplusplus
}
#endif

#endif	/* SQLITE_STRUCT_H */

