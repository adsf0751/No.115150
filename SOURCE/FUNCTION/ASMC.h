typedef struct
{
        char szCreditCardFlag[1 + 1];           /* 支援刷卡兌換 */
        char szCreditCardStartDate[8 + 1];      /* 刷卡兌換起日參數 */
        char szCreditCardEndDate[8 + 1];        /* 刷卡兌換迄日參數 */
        char szBarCodeFlag[1 + 1];              /* 支援條碼兑換 */
        char szBarCodeStartDate[8 + 1];         /* 條碼兑換起日參數 */
        char szBarCodeEndDate[8 + 1];           /* 條碼兑換迄日參數 */
        char szVoidRedeemFlag[1 + 1];           /* 支援條碼兌換取消 */
        char szVoidRedeemStartDate[8 + 1];      /* 條碼兌換取消起日參數 */
        char szVoidRedeemEndDate[8 + 1];        /* 條碼兌換取消迄日參數 */
        char szASMFlag[1 + 1];                  /* 支援優惠(含請求電文)功能(優惠取消請求電文不另管控) */
        char szASMStartDate[8 + 1];             /* 優惠起日參數 */
        char szASMEndDate[8 + 1];               /* 優惠迄日參數 */
} ASMC_REC;


/* 以欄位數決定 ex:欄位數是12個，理論上會有11個comma和兩個byte的0x0D 0X0A */
#define _SIZE_ASMC_COMMA_0D0A_           13
#define _SIZE_ASMC_REC_                  (sizeof(ASMC_REC))
#define _ASMC_FILE_NAME_                 "ASMC.dat"
#define _ASMC_FILE_NAME_BAK_             "ASMC.bak"


/* Load & Save function */
int inLoadASMCRec(int inASMCRec);
int inSaveASMCRec(int inASMCRec);

/* Set function */
int inSetCreditCardFlag(char* szCreditCardFlag);
int inSetCreditCardStartDate(char* szCreditCardStartDate);
int inSetCreditCardEndDate(char* szCreditCardEndDate);
int inSetBarCodeFlag(char* szBarCodeFlag);
int inSetBarCodeStartDate(char* szBarCodeStartDate);
int inSetBarCodeEndDate(char* szBarCodeEndDate);
int inSetVoidRedeemFlag(char* szVoidRedeemFlag);
int inSetVoidRedeemStartDate(char* szVoidRedeemStartDate);
int inSetVoidRedeemEndDate(char* szVoidRedeemEndDate);
int inSetASMFlag(char* szASMFlag);
int inSetASMStartDate(char* szASMStartDate);
int inSetASMEndDate(char* szASMEndDate);

/* Get function */
int inGetCreditCardFlag(char* szCreditCardFlag);
int inGetCreditCardStartDate(char* szCreditCardStartDate);
int inGetCreditCardEndDate(char* szCreditCardEndDate);
int inGetBarCodeFlag(char* szBarCodeFlag);
int inGetBarCodeStartDate(char* szBarCodeStartDate);
int inGetBarCodeEndDate(char* szBarCodeEndDate);
int inGetVoidRedeemFlag(char* szVoidRedeemFlag);
int inGetVoidRedeemStartDate(char* szVoidRedeemStartDate);
int inGetVoidRedeemEndDate(char* szVoidRedeemEndDate);
int inGetASMFlag(char* szASMFlag);
int inGetASMStartDate(char* szASMStartDate);
int inGetASMEndDate(char* szASMEndDate);

