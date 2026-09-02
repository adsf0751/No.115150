#define _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_	10
#define _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_	3

#define _CUSTOMER_105_MCDONALDS_DISPLAY_TIMEOUT_	3

#define _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_	3

#define _ESVC_GET_CARD_TIMEOUT_			10
#define _CUSTOMER_005_FPG_GET_CARD_TIMEOUT_	30

#define _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_		15
#define _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_	30

#define _CUSTOMER_123_IKEA_ENTER_PASSWORD_TIMEOUT_		10
#define _CUSTOMER_124_EVER_RICH_ENTER_PASSWORD_TIMEOUT_		15

#define _CARD_INQUIRY_TYPE_NONE_		0
#define _CARD_INQUIRY_TYPE_MEG_			1
#define _CARD_INQUIRY_TYPE_CHIP_		2
#define _CARD_INQUIRY_TYPE_CTLS_		3
#define _144_To_400_CHANGED_FILE_NAME_		"144_to_400_changed"	/* 有此檔代表已切換過 */

#define	_TABLE_NAME_DUTY_FREE_PWD_		"duty_free_pwd"
#define _DUTY_FREE_REPRINT_PWD_DEFAULT_		"8940"
#define _DUTY_FREE_DETAIL_PWD_DEFAULT_		"0818"
#define _DUTY_FREE_REPRINT_TMS_NOTIFY_FILE_NAME_	"TMS_Notify"
#define	_TABLE_NAME_DUTY_FREE_FUNCTION_		"duty_free_function"

#define _CUS_042_BDAU_LOG_FILENAME_		"BDAUlog.txt"

#define _POBTRAN_TEMP_FILE_NAME_		"POBTRAN_TEMP"

#define _SETTLENMENT_RECOVER_NCCC_XML_								"Settlement_Check_NCCC.xml"
#define _SETTLENMENT_RECOVER_XML_TAG_ROOT_NODE_							"StepBool"
#define _SETTLENMENT_RECOVER_XML_TAG_FUNCTION_DELETE_BATCH_FLOW_				"_FUNCTION_DELETE_BATCH_FLOW_"
#define _SETTLENMENT_RECOVER_XML_TAG_FUNCTION_PRINT_TOTAL_REPORT_BY_BUFFER_			"_FUNCTION_PRINT_TOTAL_REPORT_BY_BUFFER_"
#define _SETTLENMENT_RECOVER_XML_TAG_FUNCTION_DELETE_ACCUM_FLOW_				"_FUNCTION_DELETE_ACCUM_FLOW_"
#define _SETTLENMENT_RECOVER_XML_TAG_FUNCTION_REST_BATCH_INV_					"_FUNCTION_REST_BATCH_INV_"
#define _SETTLENMENT_RECOVER_XML_TAG_FUNCTION_UPDATE_BATCH_NUM_					"_FUNCTION_UPDATE_BATCH_NUM_"
#define _SETTLENMENT_RECOVER_XML_TAG_NCCC_FUNCTION_DCC_SETTLE_DOWNLOAD_                         "_NCCC_FUNCTION_DCC_SETTLE_DOWNLOAD_"
#define _SETTLENMENT_RECOVER_XML_TAG_TICKET_ECC_SET_UPDATE_BATCH_FLAG_                          "_TICKET_ECC_SET_UPDATE_BATCH_FLAG_"
#define _SETTLENMENT_RECOVER_XML_TAG_ESC_DATA_SALE_UPLOAD_CNT_                                  "inSaleUploadCnt"
#define _SETTLENMENT_RECOVER_XML_TAG_ESC_DATA_REFUND_UPLOAD_CNT_                                "inRefundUploadCnt"
#define _SETTLENMENT_RECOVER_XML_TAG_ESC_DATA_SALE_PAPER_CNT_                                   "inSalePaperCnt"
#define _SETTLENMENT_RECOVER_XML_TAG_ESC_DATA_REFUND_PAPER_CNT_                                 "inRefundPaperCnt"
#define _SETTLENMENT_RECOVER_XML_TAG_ESC_DATA_SALE_UPLOAD_AMT_                                  "lnSaleUploadAmt"
#define _SETTLENMENT_RECOVER_XML_TAG_ESC_DATA_REFUND_UPLOAD_AMT_                                "lnRefundUploadAmt"
#define _SETTLENMENT_RECOVER_XML_TAG_ESC_DATA_SALE_PAPER_AMT_                                   "lnSalePaperAmt"
#define _SETTLENMENT_RECOVER_XML_TAG_ESC_DATA_REFUND_PAPER_AMT_                                 "lnRefundPaperAmt"
#define _SETTLENMENT_RECOVER_DCC_XML_								"Settlement_Check_DCC.xml"
#define _SETTLENMENT_RECOVER_ESVC_XML_								"Settlement_Check_ESVC.xml"

#define _FLIGHT_TICKET_TRANS_NONE_								'\x00'
#define _FLIGHT_TICKET_TRANS_YES_								'1'
#define _FLIGHT_TICKET_TRANS_NOT_								'2'
#define _FLIGHT_TICKET_PDS0523_1_                                                               "TPE"
#define _FLIGHT_TICKET_PDS0523_2_                                                               "TSA"
#define _FLIGHT_TICKET_PDS0523_3_                                                               "KHH"
#define _FLIGHT_TICKET_PDS0523_4_                                                               "RMQ"
#define _FLIGHT_TICKET_PDS0523_5_                                                               "TNN"
#define _FLIGHT_TICKET_PDS0523_6_                                                               "HUN"
#define _FLIGHT_TICKET_PDS0523_7_                                                               "TTT"
#define _FLIGHT_TICKET_PDS0523_8_                                                               "MZG"
#define _FLIGHT_TICKET_PDS0523_9_                                                               "KNH"
#define _FLIGHT_TICKET_TXN_CATEGORY_DISPLAY_UPGRADE_						"Upgrade"
#define _FLIGHT_TICKET_TXN_CATEGORY_DISPLAY_CLUB_FEE_						"Club fee"
#define _FLIGHT_TICKET_TXN_CATEGORY_DISPLAY_CARGO_						"Cargo"
#define _FLIGHT_TICKET_TXN_CATEGORY_DISPLAY_DUTY_FREE_						"Duty free"
#define _FLIGHT_TICKET_TXN_CATEGORY_DISPLAY_BAGGAGE_CHARGE_					"Baggage charge"
#define _FLIGHT_TICKET_TXN_CATEGORY_DISPLAY_PREPAID_TICKET_					"Prepaid ticket"
#define _FLIGHT_TICKET_TXN_CATEGORY_DISPLAY_DELIVERY_						"Delivery"
#define _FLIGHT_TICKET_TXN_CATEGORY_DISPLAY_PET_CARRIER_					"Pet carrier"
#define _FLIGHT_TICKET_TXN_CATEGORY_DISPLAY_TICKET_REISSUE_					"Ticket reissue"
#define _FLIGHT_TICKET_TXN_CATEGORY_DISPLAY_TOUR_ORDER_						"Tour order"
#define _FLIGHT_TICKET_TXN_CATEGORY_DISPLAY_MISCELLANEOUS_CHARGE_                               "Others"
#define _FLIGHT_TICKET_TXN_CATEGORY_DISPLAY_SOUVENIR_						"Souvenir"
#define _FLIGHT_TICKET_TXN_CATEGORY_DISPLAY_TICKET_PURCHASE_                                    "Ticket"

#define _FLIGHT_TICKET_TXN_CATEGORY_PRINT_UPGRADE_						"Upgrade"
#define _FLIGHT_TICKET_TXN_CATEGORY_PRINT_CLUB_FEE_						"Club fee"
#define _FLIGHT_TICKET_TXN_CATEGORY_PRINT_CARGO_						"Cargo"
#define _FLIGHT_TICKET_TXN_CATEGORY_PRINT_DUTY_FREE_						"Duty free"
#define _FLIGHT_TICKET_TXN_CATEGORY_PRINT_BAGGAGE_CHARGE_					"Baggage charge"
#define _FLIGHT_TICKET_TXN_CATEGORY_PRINT_PREPAID_TICKET_					"Prepaid ticket"
#define _FLIGHT_TICKET_TXN_CATEGORY_PRINT_DELIVERY_						"Delivery"
#define _FLIGHT_TICKET_TXN_CATEGORY_PRINT_PET_CARRIER_                                          "Pet carrier"
#define _FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_REISSUE_					"Ticket reissue"
#define _FLIGHT_TICKET_TXN_CATEGORY_PRINT_TOUR_ORDER_						"Tour order"
#define _FLIGHT_TICKET_TXN_CATEGORY_PRINT_MISCELLANEOUS_CHARGE_                                 "Miscellaneous charge"
#define _FLIGHT_TICKET_TXN_CATEGORY_PRINT_SOUVENIR_						"Souvenir"
#define _FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_PURCHASE_                                      "Ticket"

#define _FLIGHT_TICKET_TXN_CATEGORY_UPGRADE_                                                    "XUP"
#define _FLIGHT_TICKET_TXN_CATEGORY_CLUB_FEE_							"XAF"
#define _FLIGHT_TICKET_TXN_CATEGORY_CARGO_							"XCA"
#define _FLIGHT_TICKET_TXN_CATEGORY_DUTY_FREE_							"XDF"
#define _FLIGHT_TICKET_TXN_CATEGORY_BAGGAGE_CHARGE_						"XAE"
#define _FLIGHT_TICKET_TXN_CATEGORY_PREPAID_TICKET_						"XAO"
#define _FLIGHT_TICKET_TXN_CATEGORY_DELIVERY_							"XTD"
#define _FLIGHT_TICKET_TXN_CATEGORY_PET_CARRIER_						"XPC"
#define _FLIGHT_TICKET_TXN_CATEGORY_TICKET_REISSUE_						"XPE"
#define _FLIGHT_TICKET_TXN_CATEGORY_TOUR_ORDER_							"XOT"
#define _FLIGHT_TICKET_TXN_CATEGORY_OTHERS_							"XAA"
#define _FLIGHT_TICKET_PDS0523_SOUVENIR_							"777"

#define _EMV_MANUAL_CARD_LIST_TIMEOUT_		20
#define _EMV_ECR_CARD_LIST_TIMEOUT_		13
#define _EMV_ECR_ESG_CARD_LIST_TIMEOUT_	6

int inNCCC_RunTRT(TRANSACTION_OBJECT *pobTran, int inTRTCode);
int inNCCC_Func_Get4DBC(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_VOID_Confirm(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_VOID_Check(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_TIP_Check(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_ADJUST_Check(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Must_SETTLE(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_GenMAC(TRANSACTION_OBJECT *pobTran, char *szF_03, char *szF_04, char *szF_11, char *szF_35, char *szF_41, char *szF_63);
int inNCCC_Func_MFES_GenMAC(TRANSACTION_OBJECT *pobTran, char *szF_03, char *szF_04, char *szF_11, char *szF_35, char *szF_41, char *szF_63);
int inNCCC_CalculateMac(TRANSACTION_OBJECT *pobTran, char *szMACData, int inMACLen, int inMACIndex);
int inNCCC_Func_MakeRefNo(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_MakeInstData(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_MakeRedeemData(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Check_Transaction_Function(int inCode);
int inNCCC_Func_Check_Transaction_Function_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_CUP_PowerOn_LogOn(TRANSACTION_OBJECT* pobTran);
int inNCCC_Func_GetTxnCode(TRANSACTION_OBJECT *pobTran, char *szTxnCode);
int inNCCC_Func_FormatPAN_UCARD(char *szPAN);
int inNCCC_Func_Sync_InvoiceNumber(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Sync_Reset_InvoiceNumber(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_CardNumber_Hash(char *szInputData, char *szOutputData);
int inNCCC_Func_Get_Transaction_No_From_PAN(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Get_PAN_From_Transaction_No(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_CheckSpecialCard_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_CheckSpecialCard(TRANSACTION_OBJECT *pobTran, char* szCampaignNumber);
int inNCCC_Func_Check_MPAS_Host(void);
int inNCCC_Func_Dial_VoiceLine(unsigned char *uszNumber, unsigned short usLen);
int inNCCC_Func_Disclaim_Auth(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Display_Review_Settle(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Display_Review_Settle_DCC(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Display_Review_Settle_ESC_Reinforce(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Display_Review(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Display_Review_DCC(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Display_Review_ESC_Reinforce(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Decide_Display_Idle_Image(void);
int inNCCC_Func_Decide_MEG_TRT(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Decide_ICC_TRT(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Decide_CTLS_TRT(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Display_Please_LOGON_First(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_BuildAndSendPacket_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_BuildAndSendPacket_Retry_Flow(TRANSACTION_OBJECT *pobTran, int inRetVal);
int inNCCC_Func_SetTxnOnlineOffline_Flow(TRANSACTION_OBJECT * pobTran);
int inNCCC_Func_Card_Inquery_Issuer_Bank(TRANSACTION_OBJECT* pobTran);
int inNCCC_Func_Check_Inquiry_Issuer_Amount(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Customer_Setting(TRANSACTION_OBJECT* pobTran);
int inNCCC_Func_Customer_107_Bumper_Echo_Test(TRANSACTION_OBJECT* pobTran);
int inNCCC_Func_AutoSettle_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Bumper_Settlement_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Mcdonalds_Settlement_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Decide_DispHostRespCodeMsg(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_BuildAndSendPacket_Demo_Flow(TRANSACTION_OBJECT* pobTran);
int inNCCC_Func_DEMO_Get_DCC_F59_DATA(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Set_Terminal_Password(void);
int inNCCC_Func_CFES_Poka_Yoke_Setting(void);
int inNCCC_Func_Check_TMS_Setting_Compatible(TRANSACTION_OBJECT* pobTran);
int inNCCC_Func_ESC_FuncProcess(TRANSACTION_OBJECT* pobTran);
int inNCCC_Func_Demo_TMS_Setting(TRANSACTION_OBJECT* pobTran);
int inNCCC_Func_CL_Power_Off(TRANSACTION_OBJECT* pobTran);
int inNCCC_Func_Test_ESC_Poka_yoke(void);
int inNCCC_Func_AutoPrintTotalReport_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_AutoPrintDetailReport_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Check_Txn_Function_When_ECR(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_IDNumber_Encrypt(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Get_PayItem(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Get_OPT_PayItem(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Get_IDNumber(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_ResponseCode_Transform(int inResponseCode, char* szMsg);
int inNCCC_Func_Init_tSAM_Data(void);
int inNCCC_Func_Switch_Special_Situation(void);
int inNCCC_Func_DCC_Download_Settle(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Uny_GenMAC(TRANSACTION_OBJECT *pobTran, char *szF_03, char *szF_04, char *szF_11, char *szF_35, char *szF_41, char *szF_63);
int inNCCC_Func_Check_Card_Label_Is_Already_Have(char* szCardLabel, int* inCDTXIndex);
int inNCCC_Func_Check_Uny_Format(char* szString);
int inNCCC_Func_Disp_EchoTest(void);
int inNCCC_Func_Send_ECR_After_Print_Receipt(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Receive_EI_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Check_Card_Still_Exist_Flow(TRANSACTION_OBJECT *pobTran, int inIsError);
int inNCCC_Func_Dutyfree_Settlement_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Dutyfree_Reprint(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Everrich_Reprint_Settle_Report(TRANSACTION_OBJECT* pobTran);
int inNCCC_Func_Table_Link_Duty_Free_Pwd(SQLITE_ALL_TABLE *srAll, int inLinkState, int* inTableID, char* szReprintPwd, char* szDetailedPwd);
int inNCCC_Func_Duty_Free_Save_Total_Report_Buffer(unsigned char* uszFileName, unsigned char* uszBuffer, int inDataLen);
int inNCCC_Func_Delete_Cus_075_Files(TRANSACTION_OBJECT* pobTran);
int inNCCC_Func_Delete_Reprint_Report(int inOrgindex);
int inNCCC_Func_Check_TMS_Booting_Flow(TRANSACTION_OBJECT* pobTran);
int inNCCC_Func_Table_Link_Duty_Free_Function(SQLITE_ALL_TABLE *srAll, int inLinkState, int* inTableID, char* szEverRichSCDTEnable);
int inNCCC_Func_Check_Special_Card_In_OPT_Flow(TRANSACTION_OBJECT* pobTran);
int inNCCC_Func_CUS_075_Menu_When_ECR(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_RePrint_Customer_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_CUS_035_MIRAMAR_CINEMAS_Settlement_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Process_Reversal_Inform(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Decide_ProcessReversal(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_BDAU_DeleteAccumBatch(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_BDAU_Settlement_Check(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_BDAU_Delete_Batch_Before_TMS(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_OPT_DELETE_BATCH_Customer_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_All_HOST_MUST_SETTLE_Customer_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Check_Amount_by_Card(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Store_Amount_by_Card(TRANSACTION_OBJECT *pobTran);
void vdNCCC_Func_BDAU_GetTransType(TRANSACTION_OBJECT *pobTran, char *szPrintBuf, char *szAddMinus);
int inNCCC_Func_ICC_FPG_FTC_InsertCard(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_ICC_FPG_FTC_GetChipCardData(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_ICC_FPG_FTC_GetChipCardBalanceData(TRANSACTION_OBJECT *pobTran, char *szEFID);
int inNCCC_Func_Display_Review_Settle_TAKA(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Display_Review_TAKA(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Get_POS_ID(void);
int inNCCC_Func_Find_SAM_TID(void);
int inNCCC_Func_Beep_After_Auth(TRANSACTION_OBJECT* pobTran);
int inNCCC_Func_Get_STAN_Flow(TRANSACTION_OBJECT * pobTran);
int inNCCC_Func_Set_STAN_Flow(TRANSACTION_OBJECT * pobTran);
int inNCCC_Func_Check_Update_Batch_Num(TRANSACTION_OBJECT* pobTran);
int inNCCC_Func_Customer_123_Edit_Signpad_Beep_Interval(void);
int inNCCC_Func_Save_Temp_PobTran_For_ECR_Missing_Data(TRANSACTION_OBJECT* pobTran);
int inNCCC_Func_Get_Temp_PobTran_For_ECR_Missing_Data(TRANSACTION_OBJECT* pobTran);
int inNCCC_Func_Delete_Temp_PobTran_For_ECR_Missing_Data(TRANSACTION_OBJECT* pobTran);
int inNCCC_Func_Fix_Inv_Problem_At_Reboot(TRANSACTION_OBJECT* pobTran);
int inNCCC_Func_Check_Inv_Problem_Before_Txn(TRANSACTION_OBJECT* pobTran);
int inNCCC_Func_Display_Please_LOGON_First_Only_ISO(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Gernerate_PEM(char* szGernerateName, char* szAddFileName);
int inNCCC_Func_Update_PEM_Before_TMS_Manual_Download(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Update_PEM_After_Para_Update(void);
int inNCCC_Func_Check_CFES_Need_Change_PEM(void);
int inNCCC_Func_Is_CUP_UPLAN_ONLY(char* szString, int inStringLen);
int inNCCC_Func_Analyse_UPlan_QRCODE(TRANSACTION_OBJECT * pobTran, char* szString, int inStringLen);
int inNCCC_Func_Process_CUP_QRCODE(TRANSACTION_OBJECT *pobTran, unsigned char *szInputTLVData, int inDataSizes);
int inNCCC_Func_Check_NoSignature_Final(TRANSACTION_OBJECT *pobTran);
void vdNCCC_Func_Test_Out_Of_Handle(void);
void vdNCCC_Func_Test_Out_Of_Handle_Native(void);
void vdNCCC_Func_Test_Out_Of_Memory(void);
int inNCCC_Func_Delete_Signature_By_Shell_Command(char* szDirPath, char* szHostName);
int inNCCC_Func_Create_Settlement_XML_NCCC(TRANSACTION_OBJECT * pobTran);
int inNCCC_Func_Check_And_Recover_Settlement_XML(TRANSACTION_OBJECT *pobTran, char* szFileName, char* szPath);
int inNCCC_Func_Settlement_XML_NCCC_START(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Settlement_XML_NCCC_END(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Settlement_XML_Edit(char* szEditTag, char* szEditVal);
int inNCCC_Func_Update_Memory_Invoice(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Didp_Mem_Inv(void);
int inNCCC_Func_Card_Inquery_Issuer_Bank_Fisc(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Check_Is_Ticket_Purchase(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Get_PDS0523(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Get_PDS0523_Other(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Get_PDS0524(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Get_PDS0524_Other(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Get_Txn_Category(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Get_TXN_CATEGORY_Other_Page(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Get_PDS0530(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Get_PDS0530_Other(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Get_Flight_Ticket_Txn_Category_PrintName(TRANSACTION_OBJECT *pobTran, char* szBuffer);
int inNCCC_Func_Flight_Ticket_Allow_Print(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Flight_Ticket_ECR_Not_Allow(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inNCCC_Func_Flight_Ticket_Not_Allow_Txn_Type(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Didp_Reboot_Time(void);
int inNCCC_Func_Check_Print_Mode_4_No_Merchant_For_Txn(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Check_Print_Mode_4_No_Merchant_For_Accum(TRANSACTION_OBJECT *pobTran);

/* CUP */
int inNCCC_Func_Get_CUP_PIN(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Reset_PIN(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_CUP_VOID_Confirm(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_CUP_VOID_Check(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_CUP_Refund_Limit_Check(TRANSACTION_OBJECT *pobTran);

/* DCC */
int inNCCC_DCC_RunTRT(TRANSACTION_OBJECT *pobTran, int inTRTCode);
int inNCCC_DCC_Func_VOID_Confirm(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_Func_Must_SETTLE(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_Func_CheckTransactionFunction(TRANSACTION_OBJECT *pobTran);

/* SmartPay */
int inNCCC_Func_FISC_GenMAC(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_FISC_VOID_Confirm(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_FISC_VOID_Check(TRANSACTION_OBJECT *pobTran);

/* HappyGo */
int inNCCC_Func_HG_MakeRefNo(TRANSACTION_OBJECT *pobTran);

/* [20251219_BUG_MDF][UI] 信託主機在結帳時,需要顯示按0確認的頁面 */
int inNCCC_Func_Display_Review_Settle_TRUST(TRANSACTION_OBJECT *pobTran);
