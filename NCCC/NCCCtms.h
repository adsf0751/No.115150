typedef struct
{
        char    szManagementCode[6 + 1];                /* 下載密碼 */
        char    szDownloadScope[1 + 1];                 /* 下載範圍 */
        char    szBatchNumber[8 + 1];                   /* 作業批號 */
        char    szTerminalVersionID[16 + 1];            /* 版本名稱 */
        char    szTerminalAPVersionDate[8 + 1];         /* 版本日期 */
        char    szTSAMIndicator[8 + 1];                 /* TSAM Indicator */
        char    szTerminalPacketSize[4 + 1];            /* 端末機可下載Bytes數 */
        char    szTerminalSN[16 + 1];                   /* 端末機上Serial Number */
        char    szTSAMSLOTSN1[16 + 1];                  /* TSAM SLOT SN1 */
        char    szTSAMSLOTSN2[16 + 1];                  /* TSAM SLOT SN2 */
        char    szTSAMSLOTSN3[16 + 1];                  /* TSAM SLOT SN3 */
        char    szTSAMSLOTSN4[16 + 1];                  /* TSAM SLOT SN4 */
        char    szTerminalOSVersion[16 + 1];            /* 端末機上OS的版本 */
        char    szEMVApprovalNumber[32 + 1];            /* EMV Approval Number */
        char    szEMVApplicationKernel[32 + 1];         /* EMV Application Kernel */
        char    szIPAddress[16 + 1];                    /* EDC IP Address */
        char    szGatewayAddress[16 + 1];               /* EDC GateWay Address */
        char    szMASKAddress[16 + 1];                  /* EDC MASK Address */
        char    szPABX[10 + 1];                         /* PABX */
        char    szOnsiteReportFlag[2 + 1];              /* 至現回報flag */
        char    szOnsiteReportData[60 + 1];             /* 至現回報問券 */
} TMS_Field58_REC;

typedef struct
{
	short 	shFTPDownloadFileCount;		        /* 需下載之檔案總數*/
	int	inHeader_length;                        
	char	szIndex[2 + 1];				/* 該record 是在 TermInfo.txt 第幾筆 record */
	char	szFile_Attribute[1 + 1];		/* 該 record 的屬性 分 H(頭定義) , A(AP) , P(參數) */
	char	szHeader_TID[8 + 1]; 			/* TID 資料 */
	char	szHeader_MID[15 + 1]; 			/* MID 資料 */
	char	szHeader_MFES_ID[4 + 1]; 		/* MFES ID 資料 */
	char	szHeader_AutoDownloadFlag[1 + 1]; 	/* 可否自動下載 */
	char	szHeader_StartDownloadDate[12 + 1]; 	/* EDC可以開始下載的日期 */
	char	szHeader_TermApVersion[15 + 1];		/* 下載AP 的 AP ID */
	char	szHeader_TermApVersionDate[15 + 1];	/* 下載AP 的 AP 日期 */
	char	szHeader_TermParmDateTime[12 + 1];	/* EDC 參數異動日期時間 */
	char 	szHeader_CloseBatchFlag[1 + 1];		/* EDC是否須檢查有無帳務才可更新 */
	char	szHeader_BatchNumber[8 + 1];		/* 作業批號 */
} FTP_OBJECT;

#define _TMS_DEFAULT_DOWNLOAD_			0		/* 剛出機時的狀態 */
#define _TMS_AUTO_DOWNLOAD_			1		/* 自動下載 表示不用印簽單 */
#define _TMS_MANUAL_DOWNLOAD_			2

#define _TMS_DOWNLOAD_SCOPE_FULL_AP_			"1"
#define _TMS_DOWNLOAD_SCOPE_FULL_PARAMETER_		"2"
#define _TMS_DOWNLOAD_SCOPE_PARTIAL_PARAMETER_		"3"
#define _TMS_DOWNLOAD_SCOPE_RETURN_TASK_REPORT_		"4"
#define _TMS_DOWNLOAD_SCOPE_TRACE_LOG_UPLOAD_		"5"
#define _TMS_DOWNLOAD_SCOPE_CLM_			"6"
#define _TMS_DOWNLOAD_SCOPE_PARAMETER_EFFECT_REPORT_	"7"
#define _TMS_DOWNLOAD_SCOPE_046_REPRINT_REPORT_		"8"

/* 自動排程提示單用 START */
#define _TMS_PRT_SCHEDULE_NOFTFY_	1		/* 表示要印自動排程時間 */
#define _TMS_PRT_SCHEDULE_SUCCESS_	2		/* 表示要印自動排程下載完成 */
/* 自動排程提示單用 END */

#define _TMS_DOWNLOAD_SECURE_MODE_NONE_		0	/* 不加密 */
#define _TMS_DOWNLOAD_SECURE_MODE_FTPS_		1	/* FTPS */
#define _TMS_DOWNLOAD_SECURE_MODE_SFTP_		2	/* SFTP */

#define _TMS_TRACE_LOG_			_EDC_TRACE_LOG_FILE_NAME_
#define _APPL_NAME_			"NCCCAPPL.zip"
#define _MMCI_FILE_NAME_		"MMCI_NAME.dat"
#define _APP_UPDATE_DIR_NAME_		"nexsysap"
#define _APP_UPDATE_PATH_		"./fs_data/nexsysap/"
#define _FTP_INFO_2_			"TermInfo2.txt"
#define _APPL_UPDATE_CHECK_NAME_	"APPLOK"
#define _APPL_UPDATE_CHECK_DIR_NAME_	"APPLCheck"
#define _APPL_UPDATE_CHECK_PATH_	"./fs_data/APPLCheck/"

#define _TMS_TerminalPacketSize_	"0998"

#define _UI_PICTURE_FULL_		"UI_FULL.zip"
#define _UI_PICTURE_HALF_		"UI_HALF.zip"
#define _UI_UPDATE_DIR_NAME_		"nexsysui"
#define _UI_UPDATE_PATH_		"./fs_data/nexsysui/"

#define	_TMS_PRESERVE_XML_FILE_NAME_		"TMS_PRESERVE.xml"
#define _TMS_PRESERVE_TAG_NEXSYS_		"NEXSYS"
#define _TMS_PRESERVE_TAG_NEXSYS_LEN_		6
#define	_TMS_PRESERVE_TAG_EDC_IP_		"EDC_IP"
#define _TMS_PRESERVE_TAG_EDC_IP_LEN_		6
#define _TMS_PRESERVE_TAG_MASK_IP_		"MASK_IP"
#define _TMS_PRESERVE_TAG_MASK_IP_LEN_		7
#define _TMS_PRESERVE_TAG_GATEWAY_IP_		"GATEWAY_IP"
#define _TMS_PRESERVE_TAG_GATEWAY_IP_LEN_	10
#define _TMS_PRESERVE_TAG_FTP_IP_		"FTP_IP"
#define _TMS_PRESERVE_TAG_FTP_IP_LEN_		6
#define _TMS_PRESERVE_TAG_FTP_PORT_		"FTP_PORT"
#define _TMS_PRESERVE_TAG_FTP_PORT_LEN_		8
#define _TMS_PRESERVE_TAG_SUP_ECR_UDP_		"SUP_ECR_UDP"
#define _TMS_PRESERVE_TAG_SUP_ECR_UDP_LEN_	11
#define _TMS_PRESERVE_TAG_POS_IP_		"POS_IP"
#define _TMS_PRESERVE_TAG_POS_IP_LEN_		6
#define _TMS_PRESERVE_TAG_POS_ID_		"POS_ID"
#define _TMS_PRESERVE_TAG_POS_ID_LEN_		6
#define _TMS_PRESERVE_TAG_GPRS_APN_		"GPRS_APN"
#define _TMS_PRESERVE_TAG_GPRS_APN_LEN_		8
#define _TMS_PRESERVE_TAG_SIGNPAD_BEEP_INTERVAL_	"SIGNPAD_BEEP_INTERVAL"
#define _TMS_PRESERVE_TAG_SIGNPAD_BEEP_INTERVAL_LEN_	21
#define _TMS_PRESERVE_TAG_PWM_ENABLE_		"PWM_ENABLE"
#define _TMS_PRESERVE_TAG_PWM_ENABLE_LEN_	10
#define _TMS_PRESERVE_TAG_PWM_MODE_		"PWM_MODE"
#define _TMS_PRESERVE_TAG_PWM_MODE_LEN_		8
#define _TMS_PRESERVE_TAG_PWM_IDLE_TIMEOUT_	"PWM_IDLE_TIMEOUT"
#define _TMS_PRESERVE_TAG_PWM_IDLE_TIMEOUT_LEN_	16

/*	 MEMO 
		Trace Log上傳(資訊回報)為端末機上傳異常紀錄之資訊。
		參數生效狀態回報為端末機上傳參數是否已生效之資訊。
		至現回報、Trace Log上傳(資訊回報)及參數生效狀態回報無需發送訊問訊息，直接以Logon Request啟動上傳作業。
 */
int inNCCCTMS_TMS_Func5SelectFlow(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_TID_GET(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_MID_GET(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_Version_Check(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_TMS_SetCommParm(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_Download_PWD_GET(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_ConnectToServer(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_DisConnect_From_Server(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_FuncSendReceive_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_FuncSendReceive(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_FuncSendReceive_FTPS(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_FuncResultHandle(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_Field58_Initial(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_IdleUpdate(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_UpdateParam_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_UpdateParam(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_Func6TMSDownload(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_CheckTMSOK(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_Schedule_Effective_Date_Time_Check(TRANSACTION_OBJECT * pobTran);
int inNCCCTMS_Schedule_Inquire_Date_Time_Check(void);
int inNCCCTMS_Schedule_Download_Date_Time_Check(TRANSACTION_OBJECT * pobTran);
int inNCCCTMS_Inquire(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_Schedule_Download(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_Download_Settle(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_TMS_Return_Report(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_Func6TraceLog_Upload(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_Func6TraceLog_Result(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_Settle_Check(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_Func5ReturnTaskReport(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_CheckAllDownloadFile_Flow(TRANSACTION_OBJECT * pobTran);
int inNCCCTMS_CheckAllDownloadFile_ISO8583(void);
int inNCCCTMS_CheckAllDownloadFile_FTPS(void);
int inNCCCTMS_DeleteAllDownloadFile_Flow(TRANSACTION_OBJECT * pobTran);
int inNCCCTMS_DeleteAllDownloadFile_ISO8583(void);
int inNCCCTMS_DeleteAllDownloadFile_FTPS(void);
int inNCCCTMS_Initial_HDPT(void);
int inNCCCTMS_Initial_EDC(void);
int inNCCCTMS_HDT_AND_HDPT_Relist(char* szLeadHostName, char* szFollowHostName, char* szLeadTRTName, char* szFollowTRTName);
int inNCCCTMS_HDPT_Relist_Flow(char* szLeadTRTName, char* szFollowTRTName);
int inNCCCTMS_HDPT_Relist(char* szLeadTRTName, char* szFollowTRTName);
int inNCCCTMS_HDPT_Relist_SQLite(char* szLeadTRTName, char* szFollowTRTName);
int inNCCCTMS_Table_Relist(char *szFileName, int inOldIndex, int inNewIndex);
int inNCCCTMS_PRINT_ScheduleMessage(TRANSACTION_OBJECT *pobTran, int inStatus);
int inNCCCTMS_PRINT_Return_Task(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_Check_FileList_Flow(TRANSACTION_OBJECT * pobTran);
int inNCCCTMS_Delete_FileList_Flow(TRANSACTION_OBJECT * pobTran);
int inNCCCTMS_Process_AP(void);
int inNCCCTMS_Backup_CPT_Parameter(void);
int inNCCCTMS_Recover_CPT_Parameter(void);
int inNCCCTMS_Sync_MCCCode(void);
int inNCCCTMS_Initial_Ticket(void);
int inNCCCTMS_Initial_IPASS(void);
int inNCCCTMS_Initial_ECC(void);
int inNCCCTMS_Initial_ICASH(void);
int inNCCCTMS_Reboot(TRANSACTION_OBJECT * pobTran);
int inNCCCTMS_Deal_With_Things_After_APPL_Update(TRANSACTION_OBJECT * pobTran);
int inNCCCTMS_Deal_With_Things_After_Parameter_Update(TRANSACTION_OBJECT * pobTran);
int inNCCCTMS_Change_AP_Filesize_To_0(void);
int inNCCCTMS_FTPS_Update_TermParaTime(void);
int inNCCCTMS_Get_Filelist_Name(char* szFileName);
int inNCCCTMS_CDT_Customize(void);
int inNCCCTMS_CDTX_initial(void);
int inNCCCTMS_DUTY_FREE_ReturnTxnReceipt(TRANSACTION_OBJECT* pobTran);
int inNCCCTMS_PRINT_ScheduleMessage_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_FuncSendReceive_Only_Packet(TRANSACTION_OBJECT *pobTran);

/* FTPS使用 */
int inNCCCTMS_FTPS_TermInfo_Download(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_FTPS_TermInfo_Analyze(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_FTPS_Auto_Download_Check(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_FTPS_Manual_Download_Check(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_FTPS_Check_TID_MID(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_FTPS_APPARM_Download(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_FTPS_FuncResultHandle(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_FTPS_UpdateParam(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_FTPFLT_Delete_AP_Attribute_A(char *szFileName);
int inNCCCTMS_FTPFLT_Delete_AP_Attribute_R(char *szFileName);
int inNCCCTMS_IFES_GET_FTP_IDPW(TRANSACTION_OBJECT * pobTran);
int inNCCCTMS_FTP_TermInfo_Disp_Flow(TRANSACTION_OBJECT * pobTran /* = pobTran */, FTPS_REC* srFtpsObj);
int inNCCCTMS_FTP_APPARM_Disp_Flow(TRANSACTION_OBJECT * pobTran /* = pobTran */, FTPS_REC* srFtpsObj);

/* DCC參數下載 */
int inNCCCTMS_DCC_FuncDownloadParameter(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_DCC_PRINT_AllDownloadMessage(TRANSACTION_OBJECT *pobTran);
