typedef struct
{
        char szPayItemIndex[2 + 1];             /* 繳費項目索引 */
        char szPayItemKeyMap[2 + 1];            /* 端末機對應之快捷鍵(Hot Key) */
        char szPayItemCode[5 + 1];              /* 繳費項目代碼 */
        char szPayItemScript[40 + 1];           /* 繳費項目說明 */
} PIT_REC;


/* 以欄位數決定 ex:欄位數是4個，理論上會有3個comma和兩個byte的0x0D 0X0A */
#define _SIZE_PIT_COMMA_0D0A_                   5
#define _SIZE_PIT_REC_				(sizeof(PIT_REC))		/*一個record不含comma和0D0A的長度*/
#define _PIT_FILE_NAME_                         "PIT.dat"			/*file name*/
#define _PIT_FILE_NAME_BAK_			"PIT.bak"			/*bak name*/


/* Load & Save function */
int inLoadPITRec(int inPITRec);
int inSavePITRec(int inPITRec);

/*Set function*/
int inSetPayItemIndex(char* szPayItemIndex);
int inSetPayItemKeyMap(char* szPayItemKeyMap);
int inSetPayItemCode(char* szPayItemCode);
int inSetPayItemScript(char* szPayItemScript);


/*Get function*/
int inGetPayItemIndex(char* szPayItemIndex);
int inGetPayItemKeyMap(char* szPayItemKeyMap);
int inGetPayItemCode(char* szPayItemCode);
int inGetPayItemScript(char* szPayItemScript);
