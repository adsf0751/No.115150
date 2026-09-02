/* 
 * File:   ICASHDT.h
 * Author: user
 *
 * Created on 2019/10/1 下午 4:44
 */

typedef struct
{
	char	szICASH_SAM_Slot[1 + 1];		/* 愛金卡SAM卡裝設卡槽。 */
	char	szICASH_Transaction_Function[15 + 1];	/* 交易功能參數。 */
	char	szICASH_NCCC_Agency_Code[3 + 1];	/* 特店簡碼 NCCC收單的特約機構代碼 */
	char	szICASH_Shop_ID[8 + 1];			/* 門市代碼 門市對照碼 供愛金卡系統對應使用 */
	char	szICASH_RegID[3 + 1];			/* 愛金卡端末機編號 */
	char	szICASH_Special_Agency_Code[8 + 1];	/* 特約機構編號 供愛金卡系統對應使用 */
	char	szICASH_Bank_ID[4 + 1];			/* 收單行代碼 NCCC:0956 */
} ICASHDT_REC;

/* 以欄位數決定 ex:欄位數是7個，理論上會有6個comma和兩個byte的0x0D 0X0A */
#define _SIZE_ICASHDT_COMMA_0D0A_		8
#define _SIZE_ICASHDT_REC_			(sizeof(ICASHDT_REC))			/* 一個record不含comma和0D0A的長度 */
#define _ICASHDT_FILE_NAME_			"ICASHDT.dat"				/* File name */
#define _ICASHDT_FILE_NAME_BAK_			"ICASHDT.bak"				/* Bak name */


/* Load & Save function */
int inLoadICASHDTRec(int inICASHDTRec);
int inSaveICASHDTRec(int inICASHDTRec);
int inICASHDT_Edit_ICASHDT_Table(void);

/*Get function*/
int inGetICASH_SAM_Slot(char* szICASH_SAM_Slot);
int inGetICASH_Transaction_Function(char* szICASH_Transaction_Function);
int inGetICASH_NCCC_Agency_Code(char* szICASH_NCCC_Agency_Code);
int inGetICASH_Shop_ID(char* szICASH_Shop_ID);
int inGetICASH_RegID(char* szICASH_RegID);
int inGetICASH_Special_Agency_Code(char* szICASH_Special_Agency_Code);
int inGetICASH_Bank_ID(char* szICASH_Bank_ID);

/*Set function*/
int inSetICASH_SAM_Slot(char* szICASH_SAM_Slot);
int inSetICASH_Transaction_Function(char* szICASH_Transaction_Function);
int inSetICASH_NCCC_Agency_Code(char* szICASH_NCCC_Agency_Code);
int inSetICASH_Shop_ID(char* szICASH_Shop_ID);
int inSetICASH_RegID(char* szICASH_RegID);
int inSetICASH_Special_Agency_Code(char* szICASH_Special_Agency_Code);
int inSetICASH_Bank_ID(char* szICASH_Bank_ID);

/* 
 * 扣款(1Byte)+扣款取消(1Byte)+退貨(1Byte)+自動加值(1Byte)+現金加值(1Byte)+
 * 現金加值取消(1Byte)+保留(1Byte)+餘額查詢(1 Byte)+結帳(1 Byte)+補足15個Bytes。(Byte map=Y，表示功能開啟。Byte map=N，表示功能關閉) 
 */