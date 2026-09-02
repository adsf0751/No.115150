/* 
 * File:   ECCDT.h
 * Author: user
 *
 * Created on 2018年3月20日, 下午 5:23
 */

typedef struct
{
	char	szECC_SAM_Slot[1 + 1];			/* 一卡通SAM卡裝設卡槽。 */
	char	szECC_Transaction_Function[15 + 1];	/* 交易功能參數。 */
	char	szECC_New_SP_ID[8 + 1];			/* New SP ID=TMLocationID=悠遊卡特店代號 */
	char	szECC_POS_ID[30 + 1];			/* NCCC MID */
} ECCDT_REC;


/* 以欄位數決定 ex:欄位數是4個，理論上會有3個comma和兩個byte的0x0D 0X0A */
#define _SIZE_ECCDT_COMMA_0D0A_			5
#define _SIZE_ECCDT_REC_			(sizeof(ECCDT_REC))			/* 一個record不含comma和0D0A的長度 */
#define _ECCDT_FILE_NAME_			"ECCDT.dat"				/* File name */
#define _ECCDT_FILE_NAME_BAK_			"ECCDT.bak"				/* Bak name */


/* Load & Save function */
int inLoadECCDTRec(int inECCDTRec);
int inSaveECCDTRec(int inECCDTRec);
int inECCDT_Edit_ECCDT_Table(void);

/*Get function*/
int inGetECC_SAM_Slot(char* szECC_SAM_Slot);
int inGetECC_Transaction_Function(char* szECC_Transaction_Function);
int inGetECC_New_SP_ID(char* szECC_New_SP_ID);
int inGetECC_POS_ID(char* szECC_POS_ID);

/*Set function*/
int inSetECC_SAM_Slot(char* szECC_SAM_Slot);
int inSetECC_Transaction_Function(char* szECC_Transaction_Function);
int inSetECC_New_SP_ID(char* szECC_New_SP_ID);
int inSetECC_POS_ID(char* szECC_POS_ID);

/* 
 * 扣款(1Byte)+扣款取消(1Byte)+退貨(1Byte)+自動加值(1Byte)+現金加值(1Byte)+現金加值取消(1Byte)+保留(1Byte)+餘額查詢(1 Byte)+結帳(1 Byte)+補足15個Bytes。
 * (Byte map=Y，表示功能開啟。Byte map=N，表示功能關閉) 
 */

