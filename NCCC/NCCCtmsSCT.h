typedef struct
{
        char szTMSInquireMode[2 + 1];           /* TMS下載詢問機制 */
        char szTMSInquireStartDate[8 + 1];      /* TMS 安排時間自動訊問 排程起始日 */
        char szTMSInquireTime[6 + 1];           /* TMS 安排時間自動訊問 指定詢問時間 */
        char szTMSInquireGap[2 + 1];            /* TMS 安排時間自動訊問 間隔幾天詢問 */
        char szTraceLogUploadMode[2 + 1];       /* Trace Log 上傳模式 */
        char szTraceLogUploadStartDate[8 + 1];  /* 參數為上傳啟始日 */
        char szTraceLogUploadEndDate[8 + 1];    /* 參數為上傳結束日 */
} TMSSCT_REC;


/* 以欄位數決定 ex:欄位數是7個，理論上會有6個comma和兩個byte的0x0D 0X0A */
#define _SIZE_TMSSCT_COMMA_0D0A_                8
#define _SIZE_TMSSCT_REC_                       (sizeof(TMSSCT_REC))		/*一個record不含comma和0D0A的長度*/
#define _TMSSCT_FILE_NAME_			"TMSSCT.dat"			/*file name*/
#define _TMSSCT_FILE_NAME_BAK_			"TMSSCT.bak"			/*bak name*/

#define _TMS_INQUIRE_00_SETTLE_			"0"
#define _TMS_INQUIRE_01_SETTLE_POWERON_		"1"
#define _TMS_INQUIRE_02_SCHEDHULE_SETTLE_	"2"

/* Load & Save function */
int inLoadTMSSCTRec(int inTMSSCTRec);
int inSaveTMSSCTRec(int inTMSSCTRec);
int inTMSSCT_Edit_TMSSCT_Table(void);

/*Set function*/
int inSetTMSInquireMode(char* szTMSInquireMode);
int inSetTMSInquireStartDate(char* szTMSInquireStartDate);
int inSetTMSInquireTime(char* szTMSInquireTime);
int inSetTMSInquireGap(char* szTMSInquireGap);
int inSetTraceLogUploadMode(char* szTraceLogUploadMode);
int inSetTraceLogUploadStartDate(char* szTraceLogUploadStartDate);
int inSetTraceLogUploadEndDate(char* szTraceLogUploadEndDate);


/*Get function*/
int inGetTMSInquireMode(char* szTMSInquireMode);
int inGetTMSInquireStartDate(char* szTMSInquireStartDate);
int inGetTMSInquireTime(char* szTMSInquireTime);
int inGetTMSInquireGap(char* szTMSInquireGap);
int inGetTraceLogUploadMode(char* szTraceLogUploadMode);
int inGetTraceLogUploadStartDate(char* szTraceLogUploadStartDate);
int inGetTraceLogUploadEndDate(char* szTraceLogUploadEndDate);