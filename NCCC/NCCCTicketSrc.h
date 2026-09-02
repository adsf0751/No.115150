/* 
 * File:   NCCCTicketSrc.h
 * Author: user
 *
 * Created on 2017年12月14日, 上午 11:10
 */
#define _TICKET_TYPE_NONE_		0
#define _TICKET_TYPE_IPASS_		1
#define _TICKET_TYPE_ECC_		2
#define _TICKET_TYPE_ICASH_		3

#define _TICKET_MSG_NONE_	0

#define _ESVC_FILE_F_56_		"ESVC_F_56.txt"
#define _ESVC_FILE_F_56_GZ_		"ESVC_F_56.txt.gz"
#define _ESVC_FILE_F_56_GZ_ENCRYPTED_	"ESVC_F_56_E.txt.gz"

typedef struct
{
	char	szF_03[2 + 1];	/* ISO8583 F_03 Processing Code 中間兩碼 (BCD to ASCII) */
	char	szF_04[12 + 1]; /* ISO8583 F_04 A mount 。例： NT$ 123 => 則 DATA 為 0000000 123 00 12 碼。 */
        char	szF_11[2 + 1];	/* ISO8583 F_11 STAN 後2碼 (BCD to ASCII) */
	char	szF_35[22 + 1];	/* ISO8583 F_35 電票卡號 前22碼 (ASCII)，不足22碼左靠右補 0 。*/
	char	szF_41[8 + 1];	/* ISO8583 F_41 (TID)8碼  */
	char	szF_59[18 + 1];	/* ISO8583 F _59 之 Table ID “ 電票交易訊 )18 碼。若該筆交易電文沒有 Table ID “ 則此欄位補滿 0 */
	char	szF_63[45 + 1]; /* ISO8583 F _63 Data 例：悠遊卡購貨筆數 悠遊卡購貨總額 一卡通購貨筆數 一卡通購貨總額 愛金卡購貨筆數 愛金卡購貨總額 共計 45 Bytes) */
} TICKET_NCCC_MAC_STRUCT;

int inNCCC_Ticket_RunTRT(TRANSACTION_OBJECT *pobTran, int inTRTCode);
int inNCCC_Ticket_Func_Check_Transaction_Deduct(int inCode);
int inNCCC_Ticket_Func_Check_Transaction_Refund(int inCode);
int inNCCC_Ticket_Func_Check_Transaction_Inquiry(int inCode);
int inNCCC_Ticket_Func_Check_Transaction_Top_Up(int inCode);
int inNCCC_Ticket_Func_Check_Transaction_Void_Top_Up(int inCode);
int inNCCC_Ticket_IPASS_Func_Check_Transaction_Function(int inCode);
int inNCCC_Ticket_ECC_Func_Check_Transaction_Function(int inCode);
int inNCCC_Ticket_ICASH_Func_Check_Transaction_Function(int inCode);
int inNCCC_Ticket_Func_Check_Transaction_Function_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Host_LogOn_(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Decide_Trans_Type(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Get_ParamValue(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Func_MakeRefNo(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Connect(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_DisConnect(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Check_AckHost(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Func_UpdateTermInvNum(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Func_UpdatetTicketInvNum(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Get_Card_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Get_Card(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Init_Logon_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_MFES_PowerOn_SAM_Slot(void);
int inNCCC_Ticket_Display_Transaction_Result(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Get_DAVTI_Data(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Table_Link_TRec(TRANSACTION_OBJECT *pobTran, SQLITE_ALL_TABLE *srAll, int inLinkState);
int inNCCC_Ticket_Logon_ShowResult(void);
int inNCCC_Ticket_Logon_ShowResult_Customer_107(void);
int inNCCC_Ticket_Logon_ShowResult_Flow(void);
int inNCCC_Ticket_Display_LogOn_Result(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Fast_Tap_Wait(TRANSACTION_OBJECT * pobTran, char *szUID);
int inNCCC_Ticket_Func_Display_Review_Settle(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Func_Display_Review(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Insert_Advice_Ticket_Record(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_ESVC_Get_TRec_Top_Flow(TRANSACTION_OBJECT *pobTran, int inTableType);
int inNCCC_Ticket_ESVC_Get_TRec_Top(TRANSACTION_OBJECT *pobTran, char* szTableName);
int inNCCC_Ticket_ESVC_Delete_TRec_Top_Flow(TRANSACTION_OBJECT *pobTran, int inTableType);
int inNCCC_Ticket_ESVC_Delete_TRec_Top(TRANSACTION_OBJECT *pobTran, char* szTableName);
int inNCCC_Ticket_ESVC_Get_Batch_ByInvNum(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber);
int inNCCC_Ticket_Get_Void_Top_Up_Amount_From_Batch(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_107_Bumper_Display_Transaction_Result(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_GetESVC_Enable(int inOrgHDTIndex, char *szHostEnable);
int inNCCC_Ticket_Trans_Success_Beep(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Get_Card_ECC_RETRY_FLOW(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Auto_SignOn_Time_Return(RTC_NEXSYS *srRTC);
int inNCCC_Ticket_Schedule_Auto_SignOn_Time_Check(void);
int inNCCC_Ticket_Check_ESVC_Need_SignOn_Flow(void);
int inNCCC_Ticket_Check_ESVC_Need_SignOn_By_Host(int inTDTIndex);
int inNCCC_Ticket_Auto_SignOn_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Auto_SignOn_Date_Return(char* szDate);
int inNCCC_Ticket_Top_Up_Amount_Check(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Set_CTLS_Light(int *inOrgLight, int inNewLight);
int inNCCC_Ticket_Initial_Light_State(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Set_Msg(int inTicketType, int *inOrgMsg, int inNewMsg);
int inNCCC_Ticket_Display_Fixed_Msg(int inTicketType, int inMsg);
int inNCCC_Ticket_Func_Must_SETTLE(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Func_Must_SETTLE_Show_In_Menu(void);
int inNCCC_Ticket_Demo_Type(TRANSACTION_OBJECT * pobTran, int inUnitDigit);
int inNCCC_Ticket_Find_Last_Txn_By_Ticket_Type_SQLite(TRANSACTION_OBJECT *pobTran, int inTicketType);
int inNCCC_Ticket_Get_API_Version(void);
int inNCCC_Ticket_Disp_Receipt(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Disp_Receipt_IPASS(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Disp_Receipt_ECC(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Disp_Receipt_ICASH(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Find_Last_Topup_CardType_SQLite(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Display_Inform_Activation_After_TMS_Download(void);
int inNCCC_Ticket_PrintBuffer_Inform_Activation_After_TMS_Download(void);
int inNCCC_Ticket_Display_Inform_Activation_After_ESVC_SignOn(int inDisplayFlag, int inTitleFlag);
int inNCCC_Ticket_Inquiry_Beep(TRANSACTION_OBJECT *pobTran);
int inTicketFunc_Check_Card_Flow(int *inTicketType);
int inNCCC_Ticket_InquiryCardNo_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_Decide_Ticket_Slot(unsigned char *uszSlot);

int inNCCC_Ticket_098_Mcdonalds_Display_Transaction_Result(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_098_Display_Transaction_Result(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_098_Display_Transaction_Result_End(TRANSACTION_OBJECT *pobTran);
int inNCCC_Ticket_GenMAC(TRANSACTION_OBJECT *pobTran, TICKET_NCCC_MAC_STRUCT* srMAC);
int inNCCC_Ticket_Gen_F_56_File(char* szData, int inLen);
int inNCCC_Ticket_Data_Compress_Encryption(TRANSACTION_OBJECT *pobTran);
