typedef struct
{
        char szProductCodeIndex[2 + 1];         /* 管理號碼索引 */
        char szKeyMap[1 + 1];                   /* 端末機對應之按鍵(Hot Key) */
        char szProductScript[32 + 1];           /* 產品代碼及產品說明 */
} PCD_REC;


/* 以欄位數決定 ex:欄位數是3個，理論上會有2個comma和兩個byte的0x0D 0X0A */
#define _SIZE_PCD_COMMA_0D0A_                   4
#define _SIZE_PCD_REC_				(sizeof(PCD_REC))		/*一個record不含comma和0D0A的長度*/
#define _PCD_FILE_NAME_                         "PCD.dat"			/*file name*/
#define _PCD_FILE_NAME_BAK_			"PCD.bak"			/*bak name*/


/* Load & Save function */
int inLoadPCDRec(int inPCDRec);
int inSavePCDRec(int inPCDRec);

/*Set function*/
int inSetProductCodeIndex(char* szProductCodeIndex);
int inSetKeyMap(char* szKeyMap);
int inSetProductScript(char* szProductScript);

/*Get function*/
int inGetProductCodeIndex(char* szProductCodeIndex);
int inGetKeyMap(char* szKeyMap);
int inGetProductScript(char* szProductScript);
