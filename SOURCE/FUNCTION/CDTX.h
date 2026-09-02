typedef struct
{
        char szCardIndex[2 + 1];                /* 卡別索引 */
        char szCardLabel[20 + 1];               /* 卡別名稱(VISA,MASTERCARD,JCB,U CARD,AMEX,DISCOVER,SMARTPAY) */
} CDTX_REC;


/* 以欄位數決定 ex:欄位數是2個，理論上會有1個comma和兩個byte的0x0D 0X0A */
#define _SIZE_CDTX_COMMA_0D0A_                   3
#define _SIZE_CDTX_REC_				(sizeof(CDTX_REC))			/* 一個record不含comma和0D0A的長度 */
#define _CDTX_FILE_NAME_			"CDTX.dat"				/* File name */
#define _CDTX_FILE_NAME_BAK_			"CDTX.bak"				/* Bak name */


/* Load & Save function */
int inLoadCDTXRec(int inCDTXRec);
int inSaveCDTXRec(int inCDTXRec);
int inCDTX_Edit_CDTX_Table(void);

/*Set function*/
int inSetUnknownCardIndex(char* szCardIndex);
int inSetUnknownCardLabel(char* szCardLabel);

/*Get function*/
int inGetUnknownCardIndex(char* szCardIndex);
int inGetUnknownCardLabel(char* szCardLabel);

int inCDTX_Test (void);
