/*
 * File:   Function.h
 * Author: SuLH
 *
 * Created on 2015年6月29日, 上午 9:54
 */

#define _PADDING_RIGHT_	0		/* 靠左右補字元 */
#define _PADDING_LEFT_	1		/* 靠右左補字元 */

#define _SIGNED_NONE_		0
#define	_SIGNED_MINUS_		1


typedef enum
{
	_ACCESS_FREELY_	= 0,
	_ACCESS_WITH_MANAGER_PASSWORD_,
	_ACCESS_WITH_SUPER_PASSWORD_,
	_ACCESS_WITH_CLERK_PASSWORD_,
	_ACCESS_WITH_CUSTOM_,
	_ACCESS_WITH_USER_PASSWORD_,
	_ACCESS_WITH_MERCHANT_PASSWORD_,
	_ACCESS_WITH_FUNC_PASSWORD_,
	_ACCESS_WITH_CUSTOM_RESERVED1_,
	_ACCESS_WITH_CUSTOM_RESERVED2_,
	_ACCESS_WITH_EDC_UNLOACK_PASSWORD_,
	_ACCESS_WITH_FUNC2_PASSWORD_,
	_ACCESS_WITH_DUTY_FREE_REPRINT_PASSWORD_,
	_ACCESS_WITH_DUTY_FREE_DETAIL_PASSWORD_,
}PASSWORD_LEVEL_INDEX;

/* 刷卡和插卡用的TIMER */
#define _TIMER_GET_CARD_		_TIMER_NEXSYS_1_


/* inFunc_Fs_data_Copy_To_AP_Folder使用 START */
#define _COPY_	0
#define	_MOVE_	1
/* inFunc_Fs_data_Copy_To_AP_Folder使用 END */

#define _RUNTIME_RECORD_MAX_COUNT_	200
#define _LAST_TXN_HOST_FILE_NAME_	"LAST_TXN_HOST.txt"
#define _LAST_UNIQUE_NO_NAME_           "LAST_UNIQUE.txt"

/* [20251215_BUG_MDF][UI] 新增分期交易用警示畫面 新增分期交易警示圖路徑*/
#define _INST_BMP_LEGAL_LOGO_		"./fs_data/InstBmpLEGAL.bmp"		/* 分期交易警語 */

typedef struct
{
	unsigned char    uszSecond;
	unsigned char    uszMinute;
	unsigned char    uszHour;
	unsigned char    uszDay;
	unsigned char    uszMonth;
	unsigned char    uszYear;
	unsigned char    uszDoW;	/* Day of Week （星期幾）*/
} RTC_NEXSYS;


typedef int (*GETFUNC_POINT)(char *);
typedef int (*SETFUNC_POINT)(char *);
typedef struct
{
        char		szTag[50];
        GETFUNC_POINT	inGetFunctionPoint;
	SETFUNC_POINT	inSetFunctionPoint;
}TABLE_GET_SET_TABLE;

typedef struct
{
        char		szTableName[50];
        int		(*inEditTableFlow)(void);
}TABLE_EDIT_DISPLAY_TABLE;

typedef struct _EMVconfig
{
	// Info needed for Random Online Selection
	unsigned long	floor_limit;
	unsigned long	threshold;	// Threshold Value for Biased random Selection.
	int		random_select_pc;		// Target Percentage for Random Selection.
	int		max_biased_pc;			// Maximum Target Percentage.

	unsigned char	TACdenial[5];			// Terminal Action Codes
	unsigned char	TAConline[5];
	unsigned char	TACdefault[5];

	unsigned char	DefaultTDOL[32 + 1]; // Includes length BYTE
	unsigned char	DefaultDDOL[48 + 1]; // Includes length BYTE

	unsigned char	AutoSelect; 	// not 0 => don't ask user to select application,
					  	// just take appl. with highest priority.
	unsigned char	EasyEntry;		// Is this an Easy Entry transaction?
	
	unsigned char	uszAction;
	unsigned char   uszAuthorizationCode[6 + 1];
	unsigned char   uszIssuerAuthenticationData[128 + 1];
	unsigned char   uszIssuerScript[600 + 1];
	unsigned short	usIssuerAuthenticationDataLen;
	unsigned short	usIssuerScriptLen;
	unsigned char	IssuerScriptResults[25 + 1];	// This is not config. data, but it has no EMV tag
														// so put it in global structure
} EMV_CONFIG;

typedef struct 
{
	char		szComment[50 + 1];	/* 備註 */
	unsigned long	ulSecond;
	unsigned long	ulMilliSecond;
}RUNTIME_RECORD;

typedef struct
{
	RUNTIME_RECORD	srMyRunTimeRecord[_RUNTIME_RECORD_MAX_COUNT_];
}RUNTIME_ALL_RECORD;

#define FLOOR_LIMIT       EMVGlobConfig.floor_limit
#define RS_THRESHOLD      EMVGlobConfig.threshold
#define RS_TARGET_PERCENT EMVGlobConfig.random_select_pc
#define RS_MAX_PERCENT    EMVGlobConfig.max_biased_pc
#define TACDENIAL         EMVGlobConfig.TACdenial
#define TACONLINE         EMVGlobConfig.TAConline
#define TACDEFAULT        EMVGlobConfig.TACdefault
#define DEFAULT_TDOL      EMVGlobConfig.DefaultTDOL
#define DEFAULT_DDOL      EMVGlobConfig.DefaultDDOL
#define EASY_ENTRY        EMVGlobConfig.EasyEntry
#define AUTO_SELECT       EMVGlobConfig.AutoSelect
#define ISS_SCRIPT_RES    EMVGlobConfig.IssuerScriptResults

#define     TAG_DFEC_FALLBACK_INDICATOR      	(unsigned short)0xdfec
#define     TAG_DFED_CHIP_CONDITION_CODE      	(unsigned short)0xdfed
#define     TAG_DFEE_TERMINAL_ENTRY_CAPABILITY	(unsigned short)0xdfee
#define     TAG_DFEF_REASON_ONLINE_CODE       	(unsigned short)0xdfef
#define     TAG_9F5B_ISSUER_SCRIPT_RESULTS    	(unsigned short)0x9f5b

#define	_RESET_BATCH_FILE_NAME_		"RESET_BATCH.dat"
#define	_RESET_BATCH_NONE_		"0"
#define	_RESET_BATCH_NCCC_FIRST_	"1"
#define	_RESET_BATCH_DCC_FIRST_		"2"

#define _CHECK_VALIDATION_MODE_1_       1
#define _CHECK_VALIDATION_MODE_2_       2 
#define _CHECK_VALIDATION_MODE_3_       3 
#define _CHECK_VALIDATION_MODE_4_       4 
#define _CHECK_VALIDATION_MODE_5_       5 

/* 處裡字元相關 */
int inFunc_PAD_ASCII(char *szStr_out, char *szStr_in, char szPad_char, int inPad_size, int inAlign);
int inFunc_ASCII_to_BCD(unsigned char *uszBCD, char *szASCII, int inLength);
int inFunc_BCD_to_ASCII(char *szASCII, unsigned char *uszBCD, int inLength);
int inFunc_BCD_to_INT(int *inINT, unsigned char *uszBCD, int inLength);
int inBCD_ASCII_test(void);
int inBCD_ASCII_test2(void);

/* 處理金額相關 */
int inFunc_Amount_Comma(char *szAmt, char *szCurSymbol, char szPad_char, int inSigned, int inWide, int inAlign);
int inFunc_Amount_Comma_DCC(char *szAmt, char* szCurSymbol, char szPad_char, int inSigned, int inWide, int inAlign, char *szMinorUnit, char *szCurrCode, char* szOutput);
int inFunc_Currency_Conversion_Fee(char *szMPV_In, char *szMinorUnit, char* szOutput);

/* 過卡相關 */
int inFunc_GetCardFields_Flow(TRANSACTION_OBJECT *pobTran);
int inFunc_GetCardFields(TRANSACTION_OBJECT *pobTran);
int inFunc_GetCardFields_Txno(TRANSACTION_OBJECT *pobTran);
int inFunc_GetCardFields_ICC(TRANSACTION_OBJECT *pobTran);
int inFunc_GetCardFields_CTLS(TRANSACTION_OBJECT *pobTran);
int inFunc_GetCardFields_Refund_CTLS_Flow(TRANSACTION_OBJECT *pobTran);
int inFunc_GetCardFields_Refund_CTLS(TRANSACTION_OBJECT *pobTran);
int inFunc_GetCardFields_Refund_CTLS_Txno(TRANSACTION_OBJECT *pobTran);
int inFunc_GetCardFields_FISC(TRANSACTION_OBJECT *pobTran);
int inFunc_GetCardFields_FISC_CTLS(TRANSACTION_OBJECT *pobTran);
int inFunc_GetCardFields_FISC_CTLS_Refund(TRANSACTION_OBJECT *pobTran);
int inFunc_GetCardFields_Loyalty_Redeem_Swipe(TRANSACTION_OBJECT *pobTran);
int inFunc_GetCardFields_Loyalty_Redeem_CTLS(TRANSACTION_OBJECT *pobTran);
int inFunc_GetBarCodeFlow(TRANSACTION_OBJECT *pobTran);
int inFunc_GetCardFields_HG(TRANSACTION_OBJECT *pobTran);
int inFunc_GetCreditCardFields_HG(TRANSACTION_OBJECT *pobTran);
int inFunc_GetCardFields_MailOrder(TRANSACTION_OBJECT *pobTran);

/* 帳單相關 */
int inFunc_PrintReceipt_ByBuffer_Flow(TRANSACTION_OBJECT *pobTran);
int inFunc_PrintReceipt_ByBuffer(TRANSACTION_OBJECT *pobTran);
int inFunc_NCCC_PrintReceipt_ByBuffer_Loyalty_Redeem(TRANSACTION_OBJECT *pobTran);
int inFunc_NCCC_PrintReceipt_ByBuffer(TRANSACTION_OBJECT *pobTran);
int inFunc_NCCC_PrintReceipt_ByBuffer_ESC(TRANSACTION_OBJECT *pobTran);
int inFunc_NCCC_PrintReceipt_ByBuffer_HappyGo_Single(TRANSACTION_OBJECT *pobTran);
int inFunc_NCCC_PrintReceipt_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran);
int inFunc_NCCC_PrintReceipt_ByBuffer_098_ESVC(TRANSACTION_OBJECT *pobTran);
int inFunc_RePrintReceipt_ByBuffer_Flow(TRANSACTION_OBJECT *pobTran);
int inFunc_NCCC_RePrintReceipt_ByBuffer_ESC(TRANSACTION_OBJECT *pobTran);
int inFunc_GetTransType(TRANSACTION_OBJECT *pobTran, char *szPrintBuf1, char* szPrintBuf2);
int inFunc_GetTransType_ESVC(TRANSACTION_OBJECT *pobTran, char *szPrintBuf);
int inFunc_PrePrintReport_ByBuffer(TRANSACTION_OBJECT *pobTran);
int inFunc_PrePrintReport_Auto_ByBuffer(TRANSACTION_OBJECT *pobTran);
int inFunc_PrintTotalReport_ByBuffer(TRANSACTION_OBJECT *pobTran);
int inFunc_PrintDetailReport_ByBuffer(TRANSACTION_OBJECT *pobTran);
int inFunc_PrintReceipt_ByBuffer_Trust(TRANSACTION_OBJECT *pobTran);

/* 批次、invoice處理相關 */
int inFunc_UpdateInvNum(TRANSACTION_OBJECT *pobTran);
int inFunc_UpdateBatchNum(TRANSACTION_OBJECT *pobTran);
int inFunc_DeleteBatch(TRANSACTION_OBJECT *pobTran);
int inFunc_DeleteBatch_Flow(TRANSACTION_OBJECT *pobTran);
int inFunc_DeleteAccum(TRANSACTION_OBJECT *pobTran);
int inFunc_DeleteAccum_Flow(TRANSACTION_OBJECT *pobTran);
int inFunc_CreateAllBatchTable(void);
int inFunc_DeleteAllBatchTable(void);
int inFunc_CreateNewBatchTable(TRANSACTION_OBJECT *pobTran);
int inFunc_DeleteBatchTable(TRANSACTION_OBJECT *pobTran);
int inFunc_ResetBatchInvNum(TRANSACTION_OBJECT *pobTran);

/* 時間設定相關 */
int inFunc_GetSystemDateAndTime(RTC_NEXSYS *srRTC);
int inFunc_SyncPobTran_Date_Include_Year(char *szDate,int inTimeBufferLen, RTC_NEXSYS *srRTC);
int inFunc_SyncPobTran_Date(char *szDate,int inTimeBufferLen, RTC_NEXSYS *srRTC);
int inFunc_SyncPobTran_Time(char *szTime,int inTimeBufferLen, RTC_NEXSYS *srRTC);
int inFunc_Sync_BRec_Date_Time(TRANSACTION_OBJECT *pobTran, RTC_NEXSYS *srRTC);
int inFunc_Sync_TRec_Date_Time(TRANSACTION_OBJECT *pobTran, RTC_NEXSYS *srRTC);
int inFunc_SetEDCDateTime(char *uszDate, char *uszTime);
int inFunc_Fun3EditDateTime(void);
int inFunc_CheckValidDate_Include_Year(char *szValidDate);
int inFunc_CheckValidOriDate(char *szValidDate);
int inFunc_CheckValidTime(char *szTime);

/* 功能性function*/
int inFunc_SunDay_Sum(char *szDate);
int inFunc_SunDay_Sum_Check_In_Range(char *szDate1, char *szDate2, char *szDate3);
int inFuncGetTimeToUnix(RTC_NEXSYS *srRTC_In, char *szOutput, int inSrcTimeZone, int inDestTimeZone);
int inFuncGetUnixTimeToLocalTime(RTC_NEXSYS *srRTC_Out, char *szInput, int inSrcTimeZone, int inDestTimeZone);
int inFunc_String_Dec_to_Hex_Little2Little(char *szInput, char *szOutput);
int inFunc_String_Hex_to_Dec_Big2Little(char *szInput, char *szOutput, int inInputLen);
int inFunc_String_Hex_to_Dec_Little2Little(char *szInput, char *szOutput, int inInputLen /* = 0 */);
int inFunc_Big5toUTF8(char *szUTF8, char *szBIG5);
int inFunc_UTF8toBig5(char *szBIG5, char *szUTF8);
void callbackFun(unsigned int uszTotalProgress, unsigned int uiCapProgress, unsigned char *uszCurMCI, unsigned char *uszCurCAP);
int inFunc_CalculateRunTimeGlobal_Start(void);
int inFunc_GetRunTimeGlobal(unsigned long *ulSecond, unsigned long *ulMilliSecond);
unsigned long ulFunc_CalculateRunTime_Start(void);
int inFunc_GetRunTime(unsigned long ulRunTime, unsigned long *ulSecond, unsigned long *ulMilliSecond);
int inFunc_WatchRunTime(void);
int inFunc_RecordTime_ResetAll(void);
int inFunc_RecordTime_Append(char* szComment, ...);
int inFunc_RecordTime_WatchAll(void);
int inFunc_RecordTime_Save(int inIndex, char* szComment);
int inFunc_RecordTime_Watch(int inIndex);
int inFunc_How_Many_Not_Ascii(char *szData);
int inFunc_Start_Display_Time_Thread(void);
int inFunc_Display_All_Status(char* szFunEnable);
int inFunc_Display_All_Status_By_Machine_Type(void);
int inFunc_Display_Time(void);
void vdFunc_Display_Ethernet_Status(void);
void vdFunc_Display_Battery_Status(void);
void vdFunc_Display_WiFi_Quality(void);
void vdFunc_Display_SIM_Quality(void);

/* Copy 檔案或檔案處理相關 */
int inFunc_ChooseLoadFileWay(void);
int inFunc_SDLoadFile(void);
int inFunc_Data_Move(char *szSrcFileName, char* szSource, char* szDesFileName, char* szDestination);
int inFunc_Dir_Make(char *szDirName, char* szSource);
int inFunc_Data_Delete(char* szParameter, char *szFileName, char* szSource);
int inFunc_Data_Copy(char *szSrcFileName, char* szSource, char* szDesFileName, char* szDestination);
int inFunc_Data_Rename(char *szOldFileName, char* szSource, char *szNewFileName, char* szDestination);
int inFunc_Data_GZip(char* szParameter, char *szFileName, char* szSource);
int inFunc_Data_GUnZip(char* szParameter, char *szFileName, char* szSource);
int inFunc_Unzip(char* szParameter1, char* szOldFileName, char* szSource, char* szParameter2, char* szDestination);
int inFunc_ls(char* szParameter1, char* szSource);
int inFunc_Data_Chmod(char* szParameter, char *szFileName, char* szSource);
int inFunc_Data_Dir_Get_Size(char* szParameter, char *szFileName, char* szSource, char* szSize);
int inFunc_CheckFile_In_SD_Partial(void);
int inFunc_CheckFile_In_SD_ALL(void);
int inFunc_CheckFile_In_USB_Partial(void);
int inFunc_CheckFile_In_USB_ALL(void);
int inFunc_ShellCommand_System(char *szCommand);
int inFunc_ShellCommand_Popen(char *szCommand, unsigned char uszSaveBit);
int inFunc_ShellCommand_TextFile_UTF8toBig5(char *szUTF8FileName, char *szBIG5FileName);
int inFunc_ClearAP(char *szFileName);
int inFunc_Clear_AP_Dump(void);
int inFunc_CheckFile_From_PUB_To_SD_ALL(void);
int inFunc_CheckFile_From_PUB_To_USB_ALL(void);

/* 加工要顯示字串相關 */
int inFunc_DiscardSpace(char *szTemplate);
int inFunc_Align_Center(char *szTemplate);
int inFunc_CheckFullSpace(char* szString);
int inFunc_CheckFullSpaceWithLen(char* szString, int inLen);

/* 組檔名相關 */
int inFunc_HostName_DecideByTRT(TRANSACTION_OBJECT * pobTran, char *szHostName);
int inFunc_ComposeFileName(TRANSACTION_OBJECT *pobTran, char *szFileName, char *szFileExtension, int inBatchNumWidth);
int inFunc_ComposeFileName_InvoiceNumber(TRANSACTION_OBJECT *pobTran, char *szFileName, char *szFileExtension, int inInvNumWidth);

/* Referral相關 */
int inFunc_REFERRAL_GetManualApproval(TRANSACTION_OBJECT *pobTran);
int inFunc_REFERRAL_DisplayPan(TRANSACTION_OBJECT *pobTran);
void vdFunc_FilterTel(char *tel);

/* Terminal 危機處理相關 or 系統相關 */
int inFunc_DebugSwitch(void);
int inFunc_EDCLock(char* szlocation);
int inFunc_Check_EDCLock(void);
int inFunc_Unlock_EDCLock_Flow(void);
int inFunc_Unlock_EDCLock(void);
int inFunc_Unlock_EDCLock_And_Delete_Batch(void);
int inFunc_Set_TMSOK_Flow(void);
int inFunc_Set_TMSOK(void);
int inFunc_Edit_Table(void);
int inFunc_Edit_Table_Tag(TABLE_GET_SET_TABLE* srTable);
int inFunc_Display_Option(TABLE_EDIT_DISPLAY_TABLE *srTable, int inPage);
int inFunc_Reboot(void);
int inFunc_Exit_AP(void);
int inFunc_GetSeriaNumber(char* szSerialNumber);
int inFunc_GetSystemInfo(unsigned char uszID, unsigned char *uszBuffer);
int inFunc_GetOSVersion(unsigned char *uszBuffer);
int inFunc_Sync_Debug_Switch(void);
int inFunc_Sync_ISODebug_Switch(void);
int inFunc_Sync_DisplayDebug_Switch(void);
int inFunc_Sync_EngineerDebug_Switch(void);
int inFunc_Sync_ESCDebug_Switch(void);
int inFunc_Is_Portable_Type(void);
int inFunc_Is_Cradle_Attached(void);

/* 搜尋Record 相關 */
int inFunc_Find_Specific_HDTindex(int inOrgIndex, char *szHostName, int *inHostIndex);
int inFunc_Find_Specific_HDTindex_ByCDTIndex(int inOrgIndex, char *szHDTIndex, int *inHostIndex);
int inFunc_Find_Specific_HDPTindex_Flow(int inOrgIndex, char *szTRTName, int *inTRTIndex);
int inFunc_Find_Specific_HDPTindex_SQLite(int inOrgIndex, char *szTRTName, int *inTRTIndex);
int inFunc_Find_Specific_HDPTindex(int inOrgIndex, char *szTRTName, int *inTRTIndex);
int inFunc_Find_Specific_CDTindex(int inOrgIndex, char *szCardLabel, int *inCDTIndex);
int inFunc_Get_HDPT_General_Data(TRANSACTION_OBJECT * pobTran);
int inFunc_Check_Linkage_Function_Enable(TRANSACTION_OBJECT * pobTran);

/* 流程 */
int inFunc_CheckCustomizePassword(int inPasswordLevel, int inCode);
int inFunc_GetHostNum(TRANSACTION_OBJECT *pobTran);
int inFunc_GetHostNum_NewUI(TRANSACTION_OBJECT *pobTran);
int inFunc_All_Host_Settle_Check_Display(TRANSACTION_OBJECT *pobTran);
int inFunc_All_Host_Settle_Check(TRANSACTION_OBJECT *pobTran, unsigned char uszDisplayBit);
int inFunc_Display_Error(TRANSACTION_OBJECT *pobTran);
int inFunc_IdleCheckALL_DateAndTime(int* inEvent);
int inFunc_CheckTermStatus(TRANSACTION_OBJECT *pobTran);
int inFunc_CheckTermStatus_Ticket(TRANSACTION_OBJECT *pobTran);
int inFunc_ResetTitle(TRANSACTION_OBJECT *pobTran);
int inFunc_EditPWD_Flow(void);
int inFunc_Edit_Manager_Pwd(void);
int inFunc_Edit_TransFuc_Pwd(void);
int inFunc_ReviewReport(TRANSACTION_OBJECT *pobTran);
int inFunc_ReviewReport_NEWUI(TRANSACTION_OBJECT *pobTran);
int inFunc_CheckPAN_EXP(TRANSACTION_OBJECT *pobTran);
int inFunc_CheckPAN_EXP_Loyalty_Redeem(TRANSACTION_OBJECT *pobTran);
int inFunc_CheckBarcode_Loyalty_Redeem(TRANSACTION_OBJECT *pobTran);
int inFunc_CheckHGPAN_EXP(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_Display_Initial(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_Print_Initial(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_Print_Image_Initial(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_Castle_library_Initial(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_Update_Parameter(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_Load_Table(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_Sync_Debug(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_Communication_Initial(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_ECR_Initial(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_EMV_Initial(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_TMSOK_Check_Initial(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_Eticket_Initial(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_CTLS_Initial(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_SQLite_Initial(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_TSAM_Initial(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_TMS_Parameter_Inquire(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_TMS_DCC_Schedule(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_Enter_PowerOn_Password(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_CUP_LOGON(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_Clear_AP_Dump(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_Process_Cradle(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_PowerManagement(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_Reprint_Poweroff(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_Update_Success_Report(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_TMS_Download(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_TMS_AP_Update_ReCheck_OK(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_Load_TMK_ECR_Initial(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_Set_Castle_PWD(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_Get_EDC_Boot_Time(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_RTC_Millisecond_Correction(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_Set_System_Power_Function(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_Set_Brightness(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_Wait_For_Switch(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_XML_Lib_Init(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_Check_And_Recover_Settlement_XML(TRANSACTION_OBJECT *pobTran);
int inFunc_Booting_Flow_Renew_Auto_Reboot_Time(TRANSACTION_OBJECT *pobTran);
int inFunc_Check_Validation(char* szString, int inStringLen, int inMode);
int inFunc_Check_PCI_Reboot(void);
int inFunc_Check_PCI_6_0(void);
int inFunc_Check_ID_Valid(char* szID);

int inFunc_TWNAddDataToEMVPacket(TRANSACTION_OBJECT *pobTran, unsigned short usTag, unsigned char *pbtPackBuff);
int inFunc_Set_Temp_VersionID(void);
int inFunc_Check_Version_ID(void);
int inFunc_Delete_Signature(TRANSACTION_OBJECT *pobTran);
int inFunc_DuplicateSave(TRANSACTION_OBJECT* pobTran);
int inFunc_DuplicateCheck(TRANSACTION_OBJECT* pobTran);
int inFunc_SHA256(unsigned char* uszInData, unsigned int uiInDataLen, unsigned char* uszOutData);
int inFunc_Base64_Encryption(char* szInData, int inInDataLen, char* szOutData);
int inFunc_Base64_Decryption(char *to, const char *from, int len);
int inFunc_Update_AP(char* szPathName);
int inFunc_Idle_CheckCustomPassword_Flow(TRANSACTION_OBJECT *pobTran);
int inFunc_ECR_CheckCustomPassword_Flow(TRANSACTION_OBJECT *pobTran);
int inFunc_Change_EMVForceOnline(void);
int inFunc_Check_LRC(char *szbuff, int inLen, char *szOutLRC);
int inFunc_Display_LOGO(int inX, int inY);
int inFunc_Edit_LOGONum(void);
int inFunc_IP_Transform(char* szAsciiIP, char* szDecimalIP);
int inFunc_Get_Battery_Status(unsigned int* uiStatus);
int inFunc_Is_Battery_Exist(void);
int inFunc_Is_Battery_Charging(void);
int inFunc_Get_Battery_Capacity(unsigned char* uszPercentage);
int inFunc_Device_Model_Get(unsigned char* uszModel);
int inFunc_Decide_Machine_Type(int* inType);
int inFunc_Get_Termial_Model_Name(char *szModelName);
int inFunc_Check_Print_Capability(int inType);
int inFunc_UpdateAP_Preserve(void);
int inFunc_UpdateAP_Recover(void);
int inFunc_Table_Delete_Record(char *szFileName, int inRecordIndex);
int inFunc_Check_SDCard_Mounted(void);
int inFunc_Check_USB_Mounted(void);
int inFunc_ECR_Comport_Switch(void);
int inFunc_Decide_APVersion_Type(int* inType);
int inFunc_Check_AuthCode_Validate(char* szAuthCode);
int inFunc_Log_All_Firmware_Version(void);
int inFunc_Display_Msg_Instead_For_Print_Receipt(TRANSACTION_OBJECT *pobTran);
int inFunc_KBDLock_Switch(void);
int inFunc_Reboot_Function_Table(TRANSACTION_OBJECT *pobTran);
int inFunc_Load_MVT_By_Index(char* szExpectApplicationIndex);
int inFunc_Save_Last_Txn_Host(TRANSACTION_OBJECT *pobTran);
int inFunc_Load_Last_Txn_Host(int* inHDTindex);
int inFunc_Menu_Sam_Slot_PowerOn(void);
int inFunc_Menu_Sam_Slot_Select_NCCC_AID(void);
int inFunc_Is_CONTACTLESS_SUPPORT(void);
int inFunc_Is_WiFi_Support(void);
int inFunc_PowerKeyFunGet(unsigned char *uszFunc);
int inFunc_PowerKeyFunSet(unsigned char uszFunc);
int inFunc_Set_Auto_SignOn_Time_OneStep(void);
int inFunc_Check_Screen_Size(void);
int inFunc_Booting_Flow_UI_Initial(TRANSACTION_OBJECT *pobTran);
int inFunc_Clear_UI_Dump(void);
int inFunc_Update_UI(char* szPathName);
int inFunc_Save_Last_UniqueNo(TRANSACTION_OBJECT *pobTran);
int inFunc_Get_ResetBatchNum_Switch(char* szSwitch);
int inFunc_Set_ResetBatchNum_Switch(char* szSwitch);
int inFunc_Load_Last_UniqueNo(char* szInData);
int inFunc_Check_Digit(void);
int inFunc_Check_Batch_limit(TRANSACTION_OBJECT *pobTran);
int inFunc_Check_Total_Trade(TRANSACTION_OBJECT *pobTran);
int inFunc_Check_Single_Trade(TRANSACTION_OBJECT *pobTran);
int inFunc_GetHostNum_Flow(TRANSACTION_OBJECT *pobTran);
int inFunc_GetHostNum_NewUI_Cus_075(TRANSACTION_OBJECT *pobTran);
int inFunc_Edit_Duty_Free_Reprint_Pwd(void);
int inFunc_Edit_Duty_Free_Detail_Pwd(void);
int inFunc_GetSytemInfo(BYTE bID, BYTE* bBuf);
int inFunc_Get_UpperCase_Char(char* szString);
int inFunc_PWM_Mode_Switch(void);
int inFunc_Compare_Files(char* szFilePath1, char* szFilePath2);
int inFunc_Check_NexsysLog_To_SD(void);
int inFunc_Check_NexsysLog_To_USB(void);
int inFunc_SysFin_Log_Ls(void);

int inFunc_DisplayCupWarning(TRANSACTION_OBJECT *pobTran);
int inFunc_DisplayInstallmentWarning(TRANSACTION_OBJECT *pobTran);

int inFunc_SwitchCardRetentionFlag(void);

/* import */
void vdCTOSS_PrinterTTFSelect(BYTE *baFilename, BYTE bIndex);
void vdCTOSS_RotateBMPFileEx(char *szFileName, int inRotate);

/* [114197] 中心收單作業ESG永續發展優化
* [20260515] 新增  inFunc_CheckUseEsgFunction()  inFunc_Display_CHESG() 功能 */
int inFunc_CheckUseEsgFunction(TRANSACTION_OBJECT* pobTran);
int inFunc_Display_CHESG(TRANSACTION_OBJECT* pobTran);
int inFunc_PrintEsgCustReceipt(TRANSACTION_OBJECT* pobTran);

int inFunc_GetKeyEnterOut(void);
