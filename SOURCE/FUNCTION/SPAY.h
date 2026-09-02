typedef struct
{
        char    szSPAID[16 + 1];                        /* SmartPay應用程式ID */
        char    szSPCVMRequireLimit[12 + 1];            /* SmartPay感應卡驗證卡人身分限額(不含) */
        char    szSPContactlessFloorLimit[12 + 1];      /* SmartPay感應卡離線授權限額(不含) */
} SPAY_REC;


/* 以欄位數決定 ex:欄位數是3個，理論上會有2個comma和兩個byte的0x0D 0X0A */
#define _SIZE_SPAY_COMMA_0D0A_                   4
#define _SIZE_SPAY_REC_				(sizeof(SPAY_REC))		/*一個record不含comma和0D0A的長度*/
#define _SPAY_FILE_NAME_                        "SPAY.dat"			/*file name*/
#define _SPAY_FILE_NAME_BAK_			"SPAY.bak"			/*bak name*/


/* Load & Save function */
int inLoadSPAYRec(int inSPAYRec);
int inSaveSPAYRec(int inSPAYRec);

/*Set function*/
int inSetSPAID(char* szSPAID);
int inSetSPCVMRequireLimit(char* szSPCVMRequireLimit);
int inSetSPContactlessFloorLimit(char* szSPContactlessFloorLimit);

/*Get function*/
int inGetSPAID(char* szSPAID);
int inGetSPCVMRequireLimit(char* szSPCVMRequireLimit);
int inGetSPContactlessFloorLimit(char* szSPContactlessFloorLimit);
