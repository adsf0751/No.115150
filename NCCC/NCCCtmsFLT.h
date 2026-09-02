/* TMS File List */
typedef struct
{
        char szTMSFileIndex[2 + 1];             /* 檔案索引 */
        char szTMSFileAttribute[2 + 1];         /* 檔案屬性 */
        char szTMSFilePathName[60 + 1];         /* 檔案路徑及檔案名稱 */
        char szTMSFileSize[10 + 1];             /* 檔案大小 */
} TMSFLT_REC;


/* 以欄位數決定 ex:欄位數是4個，理論上會有3個comma和兩個byte的0x0D 0X0A */
#define _SIZE_TMSFLT_COMMA_0D0A_                5
#define _SIZE_TMSFLT_REC_			(sizeof(TMSFLT_REC))		/*一個record不含comma和0D0A的長度*/
#define _TMSFLT_FILE_NAME_			"TMSFLT.dat"			/*file name*/
#define _TMSFLT_FILE_NAME_BAK_			"TMSFLT.bak"			/*bak name*/


/* Load & Save function */
int inLoadTMSFLTRec(int inTMSFLTRec);
int inSaveTMSFLTRec(int inTMSFLTRec);

/*Set function*/
int inSetTMSFileIndex(char* szTMSFileIndex);
int inSetTMSFileAttribute(char* szTMSFileAttribute);
int inSetTMSFilePathName(char* szTMSFilePathName);
int inSetTMSFileSize(char* szTMSFileSize);


/*Get function*/
int inGetTMSFileIndex(char* szTMSFileIndex);
int inGetTMSFileAttribute(char* szTMSFileAttribute);
int inGetTMSFilePathName(char* szTMSFilePathName);
int inGetTMSFileSize(char* szTMSFileSize);
