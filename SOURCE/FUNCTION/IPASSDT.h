/* 
 * File:   IPASSDT.h
 * Author: user
 *
 * Created on 2017年12月14日, 下午 5:38
 */

typedef struct
{
	char	szIPASS_SAM_Slot[1 + 1];		/* 一卡通SAM卡裝設卡槽。 */
	char	szIPASS_Transaction_Function[15 + 1];	/* 交易功能參數。 */
	char	szIPASS_AMS_TMS_IP_Address[15 + 1];	/* AMS/TMS IP Address(電文加密用) */
	char	szIPASS_BMS_IP_Address[15 + 1];		/* BMS IP Address (電文加密用) */
	char	szIPASS_System_ID[2 + 1];		/* 系統編號 */
	char	szIPASS_SP_ID[2 + 1];			/* 業者代碼(Service Provider) */
	char	szIPASS_Sub_Company_ID[4 + 1];		/* 子公司編號 */
	char	szIPASS_Shop_ID[6 + 1];			/* 店鋪編號 */
	char	szIPASS_POS_ID[2 + 1];			/* 收銀機編號，預設值"00" */
} IPASSDT_REC;


/* 以欄位數決定 ex:欄位數是9個，理論上會有8個comma和兩個byte的0x0D 0X0A */
#define _SIZE_IPASSDT_COMMA_0D0A_		10
#define _SIZE_IPASSDT_REC_			(sizeof(IPASSDT_REC))			/* 一個record不含comma和0D0A的長度 */
#define _IPASSDT_FILE_NAME_			"IPASSDT.dat"				/* File name */
#define _IPASSDT_FILE_NAME_BAK_			"IPASSDT.bak"				/* Bak name */


/* Load & Save function */
int inLoadIPASSDTRec(int inIPASSDTRec);
int inSaveIPASSDTRec(int inIPASSDTRec);
int inIPASSDT_Edit_IPASSDT_Table(void);

/*Get function*/
int inGetIPASS_SAM_Slot(char* szIPASS_SAM_Slot);
int inGetIPASS_Transaction_Function(char* szIPASS_Transaction_Function);
int inGetIPASS_AMS_TMS_IP_Address(char* szIPASS_AMS_TMS_IP_Address);
int inGetIPASS_BMS_IP_Address(char* szIPASS_BMS_IP_Address);
int inGetIPASS_System_ID(char* szIPASS_System_ID);
int inGetIPASS_SP_ID(char* szIPASS_SP_ID);
int inGetIPASS_Sub_Company_ID(char* szIPASS_Sub_Company_ID);
int inGetIPASS_Shop_ID(char* szIPASS_Shop_ID);
int inGetIPASS_POS_ID(char* szIPASS_POS_ID);

/*Set function*/
int inSetIPASS_SAM_Slot(char* szIPASS_SAM_Slot);
int inSetIPASS_Transaction_Function(char* szIPASS_Transaction_Function);
int inSetIPASS_AMS_TMS_IP_Address(char* szIPASS_AMS_TMS_IP_Address);
int inSetIPASS_BMS_IP_Address(char* szIPASS_BMS_IP_Address);
int inSetIPASS_System_ID(char* szIPASS_System_ID);
int inSetIPASS_SP_ID(char* szIPASS_SP_ID);
int inSetIPASS_Sub_Company_ID(char* szIPASS_Sub_Company_ID);
int inSetIPASS_Shop_ID(char* szIPASS_Shop_ID);
int inSetIPASS_POS_ID(char* szIPASS_POS_ID);

/* 
 * 扣款(1Byte)+扣款取消(1Byte)+退貨(1Byte)+自動加值(1Byte)+現金加值(1Byte)+
 * 現金加值取消(1Byte)+保留(1Byte)+餘額查詢(1 Byte)+結帳(1 Byte)+補足15個Bytes。(Byte map=Y，表示功能開啟。Byte map=N，表示功能關閉) 
 */