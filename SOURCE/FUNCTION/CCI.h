/* 
 * File:   CCI.h
 * Author: user
 *
 * Created on 2016年8月22日, 下午 1:16
 */

typedef struct TagCCI_REC
{
	char	szDCCCurrencyIndex[2 + 1];	/* 貨幣索引 */
	char	szDCCCurrencyName[3 + 1];	/* 貨幣名稱 */
	char	szDCCCurrencyCode[3 + 1];	/* 幣別碼 */
}CCI_REC;

/* 以欄位數決定 ex:欄位數是3個，理論上會有2個comma和兩個byte的0x0D 0X0A */
#define _SIZE_CCI_COMMA_0D0A_                   4
#define _SIZE_CCI_REC_				(sizeof(CCI_REC))				/* 一個Record不含Comma和0D0A的長度 */
#define _CCI_FILE_NAME_				"CCI_V.dat"					/* File Name *//* 因為VISA和MASTERCARD會調成一樣，所以用VISA的就好了 */
#define _CCI_FILE_NAME_BAK_			"CCI_V.bak"					/* Bak Name */


/* Load & Save function */
int inLoadCCIRec(int inCCIRec);
int inSaveCCIRec(int inCCIRec);

/* Set function */
int inSetDCCCurrencyIndex(char* szDCCCurrencyIndex);
int inSetDCCCurrencyName(char* szDCCCurrencyName);
int inSetDCCCurrencyCode(char* szDCCCurrencyCode);

/* Get function */
int inGetDCCCurrencyIndex(char* szDCCCurrencyIndex);
int inGetDCCCurrencyName(char* szDCCCurrencyName);
int inGetDCCCurrencyCode(char* szDCCCurrencyCode);