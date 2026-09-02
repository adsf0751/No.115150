typedef struct
{
        char    szTMSIPAddress[16 + 1];			/* TMS IP Address */
        char    szTMSPortNum[6 + 1];			/* TMS Port Number */
        char    szTMSPhoneNumber[16 + 1];		/* TMS Phone Number */
        char    szTMSEffectiveCloseBatch[2 + 1];	/* 此開關打開表示結帳後才能更新參數 */
        char    szTMSEffectiveDate[8 + 1];		/* 參數生效的日期 */
        char    szTMSEffectiveTime[6 + 1];		/* 參數生效的時間 */
	char    szTMSDownloadFlag[2 + 1];		/* 排程下載的類型 0:無需下載 1:立即下載 2:排程下載  */
        char    szTMSDownloadScope[2 + 1];		/* 排程下載的類型 1:AP下載 2:Full參數下載 3:Partial 參數下載 */
        char    szTMSScheduleDate[8 + 1];		/* 排程下載日期 */
        char    szTMSScheduleTime[6 + 1];		/* 排程下載時間 */
	char	szTMSScheduleRetry[2 + 1];		/* 排程重試次數 */
	char	szTMSAPPUpdateStatus[2 + 1];		/* 是否有AP要更新 Y:表示要更新 N:表示不用 */
	char	szTMSNII[4 + 1];			/* 因為Verifone存在外部參數，所以放這裡。這邊NII只放三碼，和CPT一樣 */
	char	szTMSEffectiveReportBit[2 + 1];		/* 是否需要參數生效回報 */
} TMSCPT_REC;

/* 以欄位數決定 ex:欄位數是14個，理論上會有13個comma和兩個byte的0x0D 0X0A */
#define _SIZE_TMSCPT_COMMA_0D0A_                  15
#define _SIZE_TMSCPT_REC_			  (sizeof(TMSCPT_REC))		/*一個record不含comma和0D0A的長度*/
#define _TMSCPT_FILE_NAME_                        "TMSCPT.dat"			/*file name*/
#define _TMSCPT_FILE_NAME_BAK_			  "TMSCPT.bak"			/*bak name*/

#define _TMS_DOWNLOAD_FLAG_NO_			"0"
#define _TMS_DOWNLOAD_FLAG_IMMEDIATE_		"1"
#define _TMS_DOWNLOAD_FLAG_SCHEDULE_		"2"

#define _TMS_DOWNLOAD_SCOPE_AP_			"1"
#define _TMS_DOWNLOAD_SCOPE_FULL_		"2"
#define _TMS_DOWNLOAD_SCOPE_PARTIAL_		"3"

/* Load & Save function */
int inLoadTMSCPTRec(int inTMSCPTRec);
int inSaveTMSCPTRec(int inTMSCPTRec);

/*Set function*/
int inSetTMSIPAddress(char* szTMSIPAddress);
int inSetTMSPortNum(char* szTMSPortNum);
int inSetTMSPhoneNumber(char* szTMSPhoneNumber);
int inSetTMSEffectiveCloseBatch(char* szTMSEffectiveCloseBatch);
int inSetTMSEffectiveDate(char* szTMSEffectiveDate);
int inSetTMSEffectiveTime(char* szTMSEffectiveTime);
int inSetTMSDownloadFlag(char* szTMSDownloadFlag);
int inSetTMSDownloadScope(char* szTMSDownloadScope);
int inSetTMSScheduleDate(char* szTMSScheduleDate);
int inSetTMSScheduleTime(char* szTMSScheduleTime);
int inSetTMSScheduleRetry(char* szTMSScheduleRetry);
int inSetTMSAPPUpdateStatus(char* szTMSAPPUpdateStatus);
int inSetTMSNII(char* szTMSNII);
int inSetTMSEffectiveReportBit(char* szTMSEffectiveReportBit);

/*Get function*/
int inGetTMSIPAddress(char* szTMSIPAddress);
int inGetTMSPortNum(char* szTMSPortNum);
int inGetTMSPhoneNumber(char* szTMSPhoneNumber);
int inGetTMSEffectiveCloseBatch(char* szTMSEffectiveCloseBatch);
int inGetTMSEffectiveDate(char* szTMSEffectiveDate);
int inGetTMSEffectiveTime(char* szTMSEffectiveTime);
int inGetTMSDownloadFlag(char* szTMSDownloadFlag);
int inGetTMSDownloadScope(char* szTMSDownloadScope);
int inGetTMSScheduleDate(char* szTMSScheduleDate);
int inGetTMSScheduleTime(char* szTMSScheduleTime);
int inGetTMSScheduleRetry(char* szTMSScheduleRetry);
int inGetTMSAPPUpdateStatus(char* szTMSAPPUpdateStatus);
int inGetTMSNII(char* szTMSNII);
int inGetTMSEffectiveReportBit(char* szTMSEffectiveReportBit);

int inResetTMSCPT_Schedule(void);
int inTMSCPT_Edit_TMSCPT_Table(void);