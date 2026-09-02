/* TMS File List */
typedef struct
{
        char szFTPFileIndex[2 + 1];             /* 檔案索引 */
        char szFTPFileAttribute[2 + 1];         /* 檔案屬性 */
        char szFTPFilePath[60 + 1];             /* 檔案路徑 */
        char szFTPFileName[26 + 1];             /* 檔案名稱 */
        char szFTPFileSize[10 + 1];             /* 檔案大小 */
} FTPFLT_REC;


/* 以欄位數決定 ex:欄位數是5個，理論上會有4個comma和兩個byte的0x0D 0X0A */
#define _SIZE_FTPFLT_COMMA_0D0A_                6
#define _SIZE_FTPFLT_REC_			(sizeof(FTPFLT_REC))		/*一個record不含comma和0D0A的長度*/
#define _FTPFLT_FILE_NAME_			"FTPFLT.dat"			/*file name*/
#define _FTPFLT_FILE_NAME_BAK_			"FTPFLT.bak"			/*bak name*/


/* Load & Save function */
int inLoadFTPFLTRec(int inFTPFLTRec);
int inSaveFTPFLTRec(int inFTPFLTRec);

/*Set function*/
int inSetFTPFileIndex(char* szFTPFileIndex);
int inSetFTPFileAttribute(char* szFTPFileAttribute);
int inSetFTPFilePath(char* szFTPFilePath);
int inSetFTPFileName(char* szFTPFileName);
int inSetFTPFileSize(char* szFTPFileSize);


/*Get function*/
int inGetFTPFileIndex(char* szFTPFileIndex);
int inGetFTPFileAttribute(char* szFTPFileAttribute);
int inGetFTPFilePath(char* szFTPFilePath);
int inGetFTPFileName(char* szFTPFileName);
int inGetFTPFileSize(char* szFTPFileSize);
