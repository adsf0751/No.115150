typedef struct
{
        char    szFTPIPAddress[16 + 1];			/* FTP IP Address */
        char    szFTPPortNum[6 + 1];			/* FTP Port Number */
        char    szFTPID[20 + 1];			/* FTP ID */
        char    szFTPPW[20 + 1];			/* FTP PW */
	char	szFTPAutoDownloadFlag[2 + 1];		/* FTP AutoDownloadFlag （目前無使用）*/
	char	szFTPStartDownloadDateTime[12 + 1];	/* 允許端末機下載的日期(時間) YYYYMMDDhhmm */
	char	szFTPTermParemeterDateTime[12 + 1];	/* 端末機參數異動日期時間 YYYYMMDDhhmm */
	char	szFTPEffectiveCloseBatch[2 + 1];	/* 端末機是否須檢查有無帳務才可更新 */
	char	szFTPBatchNum[8 + 1];			/* 作業批號(端末機於自動回報時上傳) */
	char	szFTPInquiryResponseCode[2 + 1];	/* 端末機開機詢問的結果 */
	char	szFTPDownloadResponseCode[2 + 1];	/* 端末機下載的狀態 */
	char	szFTPDownloadCategory[2 + 1];		/* 0:Unknown 1:AP下載(含參數) 2:參數下載 */
	char	szFTPEffectiveReportBit[2 + 1];		/* 是否需要參數生效回報 */
} TMSFTP_REC;

/* 以欄位數決定 ex:欄位數是13個，理論上會有12個comma和兩個byte的0x0D 0X0A */
#define _SIZE_TMSFTP_COMMA_0D0A_                  14 
#define _SIZE_TMSFTP_REC_			  (sizeof(TMSFTP_REC))		/*一個record不含comma和0D0A的長度*/
#define _TMSFTP_FILE_NAME_                        "TMSFTP.dat"			/*file name*/
#define _TMSFTP_FILE_NAME_BAK_			  "TMSFTP.bak"			/*bak name*/

/*
	端末機開機詢問的結果
	0 = 無可更新之端末機AP或參數檔
	1 = 參數下載
	2 = AP下載
	3 = 與Download(POS) Server連線失敗
	4 = 無法於中心TMS找MFES相對應
	    之目錄或檔案
	5 = 下載TermInfo2.txt中途失敗
	6 = 檢核TID、MID與TermInfo.txt不相符
	7 = DHCP失敗
	8 = 簽單圖檔版本日期回報
	X = 未定義之錯誤狀態

	端末機下載回報的結果
	1 = 成功
	2 = 下載成功但是端末機因為結帳兒無法生效
	3 = 與Download Server連線失敗
	4 = 無法於中心TMS找MFES相對應
	    之目錄或檔案
	5 = 檔案大小不相符
	6 = 檢核TID、MID與TermInfo.txt不相符
	7 = 下載中途失敗
	8 = 下載後參數生效失敗
	X = 未定義之錯誤狀態
*/
#define _FTP_INQUIRY_REPORT_NO_DOWNLOAD_		"0"
#define _FTP_INQUIRY_REPORT_PARAMETER_DOWNLOAD_		"1"
#define _FTP_INQUIRY_REPORT_AP_DOWNLOAD_		"2"
#define _FTP_INQUIRY_REPORT_FTP_SERVER_CONNECT_FAILED_	"3"
#define _FTP_INQUIRY_REPORT_CAN_NOT_FIND_DIR_		"4"
#define _FTP_INQUIRY_REPORT_TERMINFO_DOWNLOAD_FAILED_	"5"
#define _FTP_INQUIRY_REPORT_TID_MID_NOT_MATCHED_	"6"
#define _FTP_INQUIRY_REPORT_DHCP_FAILED_		"7"
#define _FTP_INQUIRY_REPORT_BMP_VERSION_DATE_		"8"
#define _FTP_INQUIRY_REPORT_UNKNOWN_			"X"

#define _FTP_DOWNLOAD_REPORT_EFFECT_SUCCESS_			"1"
#define _FTP_DOWNLOAD_REPORT_EFFECT_FAIED_FOR_NOT_SETTLE_	"2"
#define _FTP_DOWNLOAD_REPORT_FTP_SERVER_CONNECT_FAILED_		"3"
#define _FTP_DOWNLOAD_REPORT_CAN_NOT_FIND_DIR_			"4"
#define _FTP_DOWNLOAD_REPORT_FILE_SIZE_NOT_VAILID_		"5"
#define _FTP_DOWNLOAD_REPORT_TID_MID_NOT_MATCHED_		"6"
#define _FTP_DOWNLOAD_REPORT_DOWNLOAD_RETRY_FAILED_		"7"
#define _FTP_DOWNLOAD_REPORT_EFFECT_FAILED_			"8"
#define _FTP_DOWNLOAD_REPORT_UNKNOWN_				"X"

#define _FTP_DOWNLOAD_CATEGORY_UNKNOWN_			"0"
#define _FTP_DOWNLOAD_CATEGORY_AP_			"1"
#define _FTP_DOWNLOAD_CATEGORY_PARAMETER_		"2"

/* Load & Save function */
int inLoadTMSFTPRec(int inTMSFTPRec);
int inSaveTMSFTPRec(int inTMSFTPRec);
int inTMSFTP_Edit_TMSFTP_Table(void);

/*Set function*/
int inSetFTPIPAddress(char* szFTPIPAddress);
int inSetFTPPortNum(char* szFTPPortNum);
int inSetFTPID(char* szFTPID);
int inSetFTPPW(char* szFTPPW);
int inSetFTPAutoDownloadFlag(char* szFTPAutoDownloadFlag);
int inSetFTPStartDownloadDateTime(char* szFTPStartDownloadDateTime);
int inSetFTPTermParemeterDateTime(char* szFTPTermParemeterDateTime);
int inSetFTPEffectiveCloseBatch(char* szFTPEffectiveCloseBatch);
int inSetFTPBatchNum(char* szFTPBatchNum);
int inSetFTPInquiryResponseCode(char* szFTPInquiryResponseCode);
int inSetFTPDownloadResponseCode(char* szFTPDownloadResponseCode);
int inSetFTPDownloadCategory(char* szFTPDownloadCategory);
int inSetFTPEffectiveReportBit(char* szFTPEffectiveReportBit);

/*Get function*/
int inGetFTPIPAddress(char* szFTPIPAddress);
int inGetFTPPortNum(char* szFTPPortNum);
int inGetFTPID(char* szFTPID);
int inGetFTPPW(char* szFTPPW);
int inGetFTPAutoDownloadFlag(char* szFTPAutoDownloadFlag);
int inGetFTPStartDownloadDateTime(char* szFTPStartDownloadDateTime);
int inGetFTPTermParemeterDateTime(char* szFTPTermParemeterDateTime);
int inGetFTPEffectiveCloseBatch(char* szFTPEffectiveCloseBatch);
int inGetFTPBatchNum(char* szFTPBatchNum);
int inGetFTPInquiryResponseCode(char* szFTPInquiryResponseCode);
int inGetFTPDownloadResponseCode(char* szFTPDownloadResponseCode);
int inGetFTPDownloadCategory(char* szFTPDownloadCategory);
int inGetFTPEffectiveReportBit(char* szFTPEffectiveReportBit);