#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <libxml/parser.h>
#include <sqlite3.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/AllStruct.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DisTouch.h"
#include "../EVENT/Flow.h"
#include "../COMM/Comm.h"
#include "../COMM/Modem.h"
#include "Accum.h"
#include "Sqlite.h"
#include "Function.h"
#include "../../HG//HGsrc.h"
#include "../../HG//HGiso.h"
#include "../../CREDIT/CreditFunc.h"
#include "../../CREDIT/CreditprtByBuffer.h"
#include "../../CREDIT/CreditCard.h"
#include "../../AMEX/AMEXsrc.h"
#include "../../AMEX/AMEXiso.h"
#include "../../DINERS/DINERSsrc.h"
#include "../../DINERS/DINERSiso.h"
#include "../../NCCC/NCCCsrc.h"
#include "../../NCCC/NCCCats.h"
#include "../../NCCC/NCCCmfes.h"
#include "../../NCCC/NCCCdcc.h"
#include "../../NCCC/NCCCesc.h"
#include "../../NCCC/NCCCtSAM.h"
#include "../../NCCC/NCCCtms.h"
#include "../../NCCC/NCCCtmk.h"
#include "../../NCCC/NCCCTicketSrc.h"
#include "../../NCCC/NCCCTicketIso.h"
#include "../../NCCC/TAKAsrc.h"
#include "../../NCCC/TAKAiso.h"
#include "../../NCCC/NCCCEWiso.h"
#include "../../NCCC/NCCCEWsrc.h"
#include "../../NCCC/NCCCTrust.h"
#include "../../FISC/NCCCfisc.h"
#include "../../EMVSRC/EMVsrc.h"
#include "../../CTLS/CTLS.h"
#include "../../ETicket/Ticket.h"
#include "../../IPASS/IPASSFunc.h"
#include "../../ECC/ICER/stdAfx.h"
#include "../../ECC/ECC.h"
#include "../../ICASH/ICASHFunc.h"
#include "../EVENT/Event.h"
#include "FuncTable.h"
#include "Card.h"
#include "Batch.h"
#include "Signpad.h"
#include "ECR.h"
#include "MultiFunc.h"
#include "PowerManagement.h"
#include "NexsysSDK.h"


/* 定義Function Table只要有跑Function Index都要來這裡抓，另外這裡放的位置要與FuncID.h放的一致 */
FUNC_TABLE FunctionTable[] ={
	/* inFuncId,                                 	inFunctionPoint */

	/* MultiFunc slave */
	{_MULTIFUNC_RC_COMMAND_, inMultiFunc_Receive_Command},
	{_MULTIFUNC_SEND_RESULT_, inMultiFunc_SendResult},

	/* Loadkey使用 */
	{_NCCC_TMK_SELECT_AID_, inNCCC_TMK_Select_AID},
	{_NCCC_TMK_GET_APPLET_INFO_, inNCCC_TMK_GetAppletInfo},
	{_NCCC_TMK_GET_KEY_INFO_MULTI_KEY_, inNCCC_TMK_GetKeyInfo_MultiKey},
	{_NCCC_TMK_GENERATE_RANDOM_NUMBER_, inNCCC_TMK_GenerateRandomNumber},
	{_NCCC_TMK_VERIFY_OTP_, inNCCC_TMK_VerifyOTP},
	{_NCCC_TMK_GET_TMK_MULTI_, inNCCC_TMK_GetTMK_Multi},
	{_NCCC_TMK_WRITE_TMK_BY_KEYCARD_FLOW_, inNCCC_TMK_Write_TMK_By_KeyCard_Flow},
	{_NCCC_TMK_SELECT_DEVICE_, inNCCC_TMK_Select_Device},

	/* 共用FLOW */
	{_FUNCTION_GET_CARD_FIELDS_FLOW_, inFunc_GetCardFields_Flow},
	{_FUNCTION_GET_CARD_FIELDS_, inFunc_GetCardFields},
	{_FUNCTION_GET_CARD_FIELDS_ICC_, inFunc_GetCardFields_ICC},
	{_FUNCTION_GET_CARD_FIELDS_CTLS_, inFunc_GetCardFields_CTLS},
	{_FUNCTION_GET_CARD_FIELDS_REFUND_CTLS_FLOW_, inFunc_GetCardFields_Refund_CTLS_Flow},
	{_FUNCTION_GET_CARD_FIELDS_REFUND_CTLS_, inFunc_GetCardFields_Refund_CTLS},
	{_FUNCTION_GET_CARD_FIELDS_FISC_, inFunc_GetCardFields_FISC},
	{_FUNCTION_GET_CARD_FIELDS_FISC_CTLS_, inFunc_GetCardFields_FISC_CTLS},
	{_FUNCTION_GET_CARD_FIELDS_FISC_REDEND_CTLS_, inFunc_GetCardFields_FISC_CTLS_Refund},
	{_FUNCTION_GET_CARD_FIELDS_LOYALTY_REDEEM_, inFunc_GetCardFields_Loyalty_Redeem_Swipe},
	{_FUNCTION_GET_CARD_FIELDS_LOYALTY_REDEEM_CTLS_, inFunc_GetCardFields_Loyalty_Redeem_CTLS},
	{_FUNCTION_GET_BARCODE_, inFunc_GetBarCodeFlow},
	{_FUNCTION_GET_CARD_FIELDS_HG_, inFunc_GetCardFields_HG},
	{_FUNCTION_GET_CREDIT_CARD_FIELDS_HG_, inFunc_GetCreditCardFields_HG},
	{_FUNCTION_GET_CREDIT_CARD_FIELDS_MAIL_ORDER_, inFunc_GetCardFields_MailOrder},
	{_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_, inFunc_PrintReceipt_ByBuffer_Flow},
	{_FUNCTION_REPRINT_RECEIPT_BY_BUFFER_, inFunc_RePrintReceipt_ByBuffer_Flow},
	{_FUNCTION_PREPRINT_REPORT_BY_BUFFER_, inFunc_PrePrintReport_ByBuffer},
	{_FUNCTION_PREPRINT_AUTO_REPORT_BY_BUFFER_, inFunc_PrePrintReport_Auto_ByBuffer},
	{_FUNCTION_PRINT_TOTAL_REPORT_BY_BUFFER_, inFunc_PrintTotalReport_ByBuffer},
	{_FUNCTION_PRINT_DETAIL_REPORT_BY_BUFFER_, inFunc_PrintDetailReport_ByBuffer},
	{_FUNCTION_REVIEW_, inFunc_ReviewReport},
	{_FUNCTION_TOTAL_REVIEW_, inACCUM_ReviewReport_Total},
	{_FUNCTION_DETAIL_REVIEW_, inBATCH_ReviewReport_Detail_Flow_By_Sqlite},
	{_FUNCTION_DETAIL_REVIEW_NEWUI_, inBATCH_ReviewReport_Detail_NEWUI_Flow_By_Sqlite},
	{_FUNCTION_GET_HOST_NUM_, inFunc_GetHostNum},
	{_FUNCTION_GET_HOST_NUM_NEWUI_, inFunc_GetHostNum_NewUI},
	{_FUNCTION_USER_CHOICE_, inBATCH_FuncUserChoice_By_Sqlite},
	{_FUNCTION_CHECK_PAN_EXPDATE_, inFunc_CheckPAN_EXP},
	{_FUNCTION_CHECK_PAN_EXPDATE_LOYALTY_REDEEM_, inFunc_CheckPAN_EXP_Loyalty_Redeem},
	{_FUNCTION_CHECK_BARCODE_LOYALTY_REDEEM_, inFunc_CheckBarcode_Loyalty_Redeem},
	{_FUNCTION_CHECK_HG_PAN_EXPDATE_, inFunc_CheckHGPAN_EXP},
	{_FUNCTION_UPDATE_INV_, inFunc_UpdateInvNum},
	{_FUNCTION_UPDATE_BATCH_NUM_, inFunc_UpdateBatchNum},
	{_FUNCTION_DELETE_BATCH_, inFunc_DeleteBatch},
	{_FUNCTION_DELETE_BATCH_FLOW_, inFunc_DeleteBatch_Flow},
	{_FUNCTION_DELETE_ACCUM_, inFunc_DeleteAccum},
	{_FUNCTION_DELETE_ACCUM_FLOW_, inFunc_DeleteAccum_Flow},
	{_FUNCTION_REST_BATCH_INV_, inFunc_ResetBatchInvNum},
	{_FUNCTION_GET_SIGNPAD_, inSIGN_TouchSignature_Flow},
	{_FUNCTION_All_HOST_MUST_SETTLE_, inFunc_All_Host_Settle_Check_Display},
	{_FUNCTION_CHECK_TERM_STATUS_, inFunc_CheckTermStatus},
	{_FUNCTION_CHECK_TERM_STATUS_TICKET_, inFunc_CheckTermStatus_Ticket},
	{_FUNCTION_DUPLICATE_CHECK_, inFunc_DuplicateCheck},
	{_FUNCTION_DUPLICATE_SAVE_, inFunc_DuplicateSave},
	{_FUNCTION_IDLE_CHECKCUSTOMPASSWORD_FLOW_, inFunc_Idle_CheckCustomPassword_Flow},
	{_FUNCTION_ECR_CHECKCUSTOMPASSWORD_FLOW_, inFunc_ECR_CheckCustomPassword_Flow},
	{_UPDATE_ACCUM_, inACCUM_UpdateFlow},
	{_UPDATE_BATCH_, inBATCH_FuncUpdateTxnRecord_By_Sqlite},
	{_COMM_MODEM_PREDIAL_, inModem_Predial},
	{_POWER_SAVING_FLOW_SLEEP_, inPWM_Enter_Sleep_Mode},
	{_POWER_SAVING_FLOW_STANDBY_, inPWM_Enter_StandBy_Mode},
	{_FUNCTION_REBOOT_, inFunc_Reboot_Function_Table},
	{_FUNCTION_SAVE_LAST_TXN_HOST_, inFunc_Save_Last_Txn_Host},
	{_FUNCTION_SAVE_LAST_UNIQUE_NO_, inFunc_Save_Last_UniqueNo},
	{_FUNCTION_CHECK_BATCH_LIMIT_, inFunc_Check_Batch_limit},
	{_FUNCTION_GET_HOST_NUM_FLOW_, inFunc_GetHostNum_Flow},
	{_FUNCTION_GET_HOST_NUM_NEWUI_CUS_075_, inFunc_GetHostNum_NewUI_Cus_075},
	{_NCCC_FUNCTION_CHESG_CHECK_, inFunc_CheckUseEsgFunction},/* [114197] 中心收單作業ESG永續發展優化 新增功能*/
	{_NCCC_FUNCTION_DISPLAY_CHESG_, inFunc_Display_CHESG}, /* [114197] 中心收單作業ESG永續發展優化 新增功能*/

	{_CREDIT_FUNCTION_GET_AMOUNT_, inCREDIT_Func_GetAmount},
	{_CREDIT_FUNCTION_GET_TIP_AMOUNT_, inCREDIT_Func_GetTipAmount},
	{_CREDIT_FUNCTION_GET_REFUND_AMOUNT_, inCREDIT_Func_GetRefundAmount},
	{_CREDIT_FUNCTION_GET_ADJUST_AMOUNT_, inCREDIT_Func_GetAdjustAmount},
	{_CREDIT_FUNCTION_GET_STOREID_, inCREDIT_Func_GetStoreID},
	{_CREDIT_FUNCTION_GET_AUTHCODE_, inCREDIT_Func_GetAuthCode},
	{_CREDIT_FUNCTION_GET_PERIOD_, inCREDIT_Func_GetPeriod},
	{_CREDIT_FUNCTION_GET_DOWNPAYMENT_, inCREDIT_Func_GetDownPayment},
	{_CREDIT_FUNCTION_GET_INSTPAYMENT_, inCREDIT_Func_GetInstPayment},
	{_CREDIT_FUNCTION_GET_INSTFEE_, inCREDIT_Func_GetInstFee},
	{_CREDIT_FUNCTION_GET_PAY_AMOUNT_, inCREDIT_Func_GetPayAmount},
	{_CREDIT_FUNCTION_GET_REDEEM_POINT_, inCREDIT_Func_GetRedeemPoint},
	{_CREDIT_FUNCTION_CHECKRESULT_, inCREDIT_Func_CheckResult},
	{_CREDIT_FUNCTION_GET_ORI_TRANSDATE_, inCREDIT_Func_GetOriTransDate},
	{_CREDIT_FUNCTION_GET_ORI_AMOUNT_, inCREDIT_Func_GetOriAmount},
	{_CREDIT_FUNCTION_GET_ORI_PREAUTH_AMOUNT_, inCREDIT_Func_GetOriPreAuthAmount}, /* 輸入原預授權金額 */
	{_CREDIT_FUNCTION_GET_PRECOMP_AMOUNT_, inCREDIT_Func_GetPreCompAmount}, /* 輸入授權完成金額 */
	{_CREDIT_FUNCTION_GET_PRODUCT_CODE_, inCREDIT_Func_GetProductCode},
	{_CREDIT_FUNCTION_CHECK_RESULT_SETTLE_, inACCUM_ReviewReport_Total_Settle},
	{_CREDIT_FUNCTION_GET_CVV2_, inCREDIT_Func_GetCVV2},
	{_CREDIT_FUNCTION_GET_DCC_ORI_TRANSDATE_, inCREDIT_Func_GetDCCOriTransDate}, /* DCC 專用(因為一般退貨和DCC退貨是同一個TRT，但流程不同) */
	{_CREDIT_FUNCTION_GET_DCC_ORI_AMOUNT_, inCREDIT_Func_GetDCCOriAmount},
	{_CREDIT_FUNCTION_GET_DCC_TIP_AMOUNT_, inCREDIT_Func_GetDCCTipAmount}, /* 輸入外幣小費 */
	{_CREDIT_FUNCTION_GET_FISC_RRN_, inCREDIT_Func_GetFiscRRN}, /* SMARTPAY專用 */
	{_CREDIT_FUNCTION_GET_FISC_REFUND_, inCREDIT_Func_GetFiscRefund},
	{_CREDIT_FUNCTION_GET_FISC_ORI_TRANSDATE_, inCREDIT_Func_GetFiscOriTransDate},
	{_CREDIT_FUNCTION_GET_RF_NUMBER_, inCREDIT_Func_Get_OPT_RFNumber}, /* 票證使用 *//* 請輸入RF序號 */
	/* IDLE FLOW */
	{_CREDIT_FUNCTION_GET_OPT_AMOUNT_, inCREDIT_Func_Get_OPT_Amount},
	{_CREDIT_FUNCTION_GET_OPT_AUTHCODE_, inCREDIT_Func_Get_OPT_AuthCode},
	{_CREDIT_FUNCTION_GET_OPT_STOREID_, inCREDIT_Func_Get_OPT_StoreID},
	{_CREDIT_FUNCTION_GET_OPT_PERIOD_, inCREDIT_Func_Get_OPT_Period},
	{_CREDIT_FUNCTION_GET_OPT_DOWNPAYMENT_, inCREDIT_Func_Get_OPT_DownPayment},
	{_CREDIT_FUNCTION_GET_OPT_INSTPAYMENT_, inCREDIT_Func_Get_OPT_InstPayment},
	{_CREDIT_FUNCTION_GET_OPT_INSTFEE_, inCREDIT_Func_Get_OPT_InstFee},
	{_CREDIT_FUNCTION_GET_OPT_PRODUCT_CODE_, inCREDIT_Func_Get_OPT_ProductCode},
	{_CREDIT_FUNCTION_GET_OPT_PAY_AMOUNT_, inCREDIT_Func_Get_OPT_PayAmount},
	{_CREDIT_FUNCTION_GET_OPT_REDEEM_POINT_, inCREDIT_Func_Get_OPT_RedeemPoint},
	{_CREDIT_FUNCTION_GET_OPT_ORI_TRANSDATE_, inCREDIT_Func_Get_OPT_OriTransDate},
	{_CREDIT_FUNCTION_GET_OPT_ORI_TRANSDATE_ESVC_, inCREDIT_Func_Get_OPT_OriTransDate_ESVC},
	{_CREDIT_FUNCTION_GET_OPT_REFUND_AMOUNT_, inCREDIT_Func_Get_OPT_RefundAmount},
	{_CREDIT_FUNCTION_GET_OPT_ORI_PREAUTH_AMOUNT_, inCREDIT_Func_Get_OPT_OriPreAuthAmount}, /* 輸入原預授權金額 */
	{_CREDIT_FUNCTION_GET_OPT_PRECOMP_AMOUNT_, inCREDIT_Func_Get_OPT_PreCompAmount}, /* 輸入授權完成金額 */
	{_CREDIT_FUNCTION_GET_OPT_FISC_RRN_, inCREDIT_Func_Get_OPT_FiscRRN},
	{_CREDIT_FUNCTION_GET_OPT_FISC_REFUND_, inCREDIT_Func_Get_OPT_FiscRefund},
	{_CREDIT_FUNCTION_GET_OPT_FISC_ORI_TRANSDATE_, inCREDIT_Func_Get_OPT_FiscOriTransDate},
	{_CREDIT_FUNCTION_GET_OPT_AMOUNT_TRUST_, inCREDIT_Func_Get_OPT_Amount_Trust},
	{_CREDIT_FUNCTION_GET_OPT_TRUST_BENEFICIARYID_, inCREDIT_Func_Get_OPT_TrustBeneficiaryId},
	{_CREDIT_FUNCTION_GET_OPT_PATIENTID_, inCREDIT_Func_Get_OPT_PatientId},
	{_CREDIT_FUNCTION_GET_OPT_TRUSTINSTITUTIONCODE_, inCREDIT_Func_Get_OPT_TrustInstitutionCode},
	{_CREDIT_FUNCTION_GET_OPT_RECONCILIATIONNO_, inCREDIT_Func_Get_OPT_ReconciliationNo},
	{_CREDIT_FUNCTION_GET_OPT_TRUST_RRN_, inCREDIT_Func_Get_OPT_TrustRRN},
	{_CREDIT_FUNCTION_GET_OPT_TRUST_VERIFICATIONCODE_, inCREDIT_Func_Get_OPT_TrustVerificationCode},
	{_CREDIT_FUNCTION_GET_OPT_ORI_TRANSDATE_TRUST_, inCREDIT_Func_Get_OPT_OriTransDate_Trust},

	{_CREDIT_FUNCTION_PRINT_PARAM_TERM_BY_BUFFER_, inCREDIT_PRINT_Func7PrintParamTerm_ByBuffer},
	{_CREDIT_FUNCTION_PRINT_DCC_PARAM_BY_BUFFER_, inCREDIT_PRINT_Func7PrintDCCParamTerm_ByBuffer},
	{_CREDIT_FUNCTION_PRINT_TERMINAL_TRACE_LOG_, inCREDIT_PRINT_TerminalTraceLog},

	/* AMEX FLOW */
	{_AMEX_FUNCTION_GET_4DBC_, inAMEX_Func_Get4DBC},
	{_AMEX_FUNCTION_SET_TXN_ONLINE_OFFLINE_, inAMEX_Func_SetTxnOnlineOffline},
	{_AMEX_FUNCTION_BUILD_AND_SEND_PACKET_, inAMEX_Func_BuildAndSendPacket},
	{_AMEX_FUNCTION_VOID_CONFIRM_, inAMEX_Func_VOID_Confirm},
	{_AMEX_FUNCTION_VOID_CHECK_, inAMEX_Func_VOID_Check},
	{_AMEX_FUNCTION_TIP_CHECK_, inAMEX_Func_TIP_Check},
	{_AMEX_FUNCTION_ADJUST_CHECK_, inAMEX_Func_ADJUST_Check},
	{_AMEX_FUNCTION_MUST_SETTLE_CHECK_, inAMEX_Func_Must_SETTLE},

	/* DINERS FLOW */
	{_DINERS_FUNCTION_SET_TXN_ONLINE_OFFLINE_, inDINERS_Func_SetTxnOnlineOffline},
	{_DINERS_FUNCTION_BUILD_AND_SEND_PACKET_, inDINERS_Func_BuildAndSendPacket},
	{_DINERS_FUNCTION_VOID_CONFIRM_, inDINERS_Func_VOID_Confirm},
	{_DINERS_FUNCTION_VOID_CHECK_, inDINERS_Func_VOID_Check},
	{_DINERS_FUNCTION_TIP_CHECK_, inDINERS_Func_TIP_Check},
	{_DINERS_FUNCTION_ADJUST_CHECK_, inDINERS_Func_ADJUST_Check},
	{_DINERS_FUNCTION_MUST_SETTLE_CHECK_, inDINERS_Func_Must_SETTLE},
	{_DINERS_FUNCTION_CHECK_TRANS_FUNCTION_, inDINERS_Func_CheckTransactionFunction},

	/* HAPPYGO FLOW */
	{_HG_FUNCTION_SELECT_PAYMENT_REFUND_, inHG_Func_SelectPaymentRefund},
	{_HG_FUNCTION_SELECT_PAYMENT_TYPE_, inHG_Func_SelectPaymentType},
	{_HG_FUNCTION_SELECT_I_R_PAYMENT_TYPE_, inHG_Func_SelectInstRedeemPaymentType},
	{_HG_FUNCTION_SELECT_TRANS_TYPE_, inHG_Func_SelectTransactionTypeInstRedeem},
	{_HG_FUNCTION_CHECK_TRANS_FUNCTION_FLOW_, inHG_Func_Check_Transaction_Function_Flow},
	{_HG_FUNCTION_MUST_SETTLE_CHECK_, inHG_Func_Must_SETTLE},
	{_HG_FUNCTION_GET_POINT_, inHG_Func_GetPoint},
	{_HG_FUNCTION_DISP_TRANS_BALANCE_POINT_, inHG_Func_DispTransBalancePoint},
	{_HG_FUNCTION_BUILD_AND_SEND_PACKET_CREDIT_, inHG_Func_BuildAndSendPacket_CREDIT},
	{_HG_FUNCTION_BUILD_AND_SEND_PACKET_HG_FLOW_, inHG_Func_BuildAndSendPacket_HG_Flow},
	{_HG_FUNCTION_SELECT_VOID_TRT_, inHG_Func_SelectVoidTRT},
	{_HG_FUNCTION_SELECT_HOST_TRT_, inHG_Func_SelectHostTRT},
	{_HG_FUNCTION_UPDATE_ACCUM_, inHG_Func_ACCUM_UpdateFlow},
	{_HG_FUNCTION_SET_HOST_CREDIT_, inHG_Func_SetHost_CREDIT},
	{_HG_FUNCTION_SET_HOST_HG_, inHG_Func_SetHost_HG},
	{_HG_FUNCTION_SET_TXN_ONLINE_OFFLINE_CREDIT_, inHG_Func_SetTxnOnlineOffline_CREDIT},
	{_HG_FUNCTION_SET_TXN_ONLINE_OFFLINE_HG_, inHG_Func_SetTxnOnlineOffline_HG},
	{_HG_FUNCTION_VOID_CHECK_, inHG_Func_VOID_Confirm},
	{_HG_FUNCTION_DISPLAY_COMPLETE_, inHG_Func_Display_Complete},
	{_HG_VOID_RUN_MULTI_FLOW_, inHG_Func_VOID_RunMultiFlow},
	{_HG_FUNCTION_ONLINE_REDEEM_CREDIT_FLOW_, inHG_Func_Online_Redeem_Credit_Flow},

	/* NCCC Source Function*/
	{_NCCC_FUNCTION_VOID_CONFIRM_, inNCCC_Func_VOID_Confirm},
	{_NCCC_FUNCTION_CUP_VOID_CONFIRM_, inNCCC_Func_CUP_VOID_Confirm},
	{_NCCC_FUNCTION_FISC_VOID_CONFIRM_, inNCCC_Func_FISC_VOID_Confirm},
	{_NCCC_FUNCTION_VOID_CHECK_, inNCCC_Func_VOID_Check},
	{_NCCC_FUNCTION_CUP_VOID_CHECK_, inNCCC_Func_CUP_VOID_Check},
	{_NCCC_FUNCTION_FISC_VOID_CHECK_, inNCCC_Func_FISC_VOID_Check},
	{_NCCC_FUNCTION_TIP_CHECK_, inNCCC_Func_TIP_Check},
	{_NCCC_FUNCTION_ADJUST_CHECK_, inNCCC_Func_ADJUST_Check},
	{_NCCC_FUNCTION_MUST_SETTLE_CHECK_, inNCCC_Func_Must_SETTLE},
	{_NCCC_FUNCTION_GET_4DBC_, inNCCC_Func_Get4DBC},
	{_NCCC_FUNCTION_MAKE_REFNO_, inNCCC_Func_MakeRefNo},
	{_NCCC_FUNCTION_MAKE_INSTALLMENT_DATA_, inNCCC_Func_MakeInstData},
	{_NCCC_FUNCTION_MAKE_REDEEM_DATA_, inNCCC_Func_MakeRedeemData},
	{_NCCC_FUNCTION_SHOW_REGISTER_MENU_, inNCCC_tSAM_Register_Menu},
	{_NCCC_FUNCTION_CHECK_TRANS_FUNCTION_FLOW_, inNCCC_Func_Check_Transaction_Function_Flow},
	{_NCCC_FUNCTION_GET_CUP_PIN, inNCCC_Func_Get_CUP_PIN},
	{_NCCC_FUNCTION_CUP_REFUND_LIMIT_CHECK_, inNCCC_Func_CUP_Refund_Limit_Check},
	{_NCCC_FUNCTION_HG_MAKE_REFNO_, inNCCC_Func_HG_MakeRefNo},
	{_NCCC_FUNCTION_GET_TRANSACTION_NO_FROM_PAN_, inNCCC_Func_Get_Transaction_No_From_PAN},
	{_NCCC_FUNCTION_CHECK_SPECIALCARD_FLOW_, inNCCC_Func_CheckSpecialCard_Flow},
	{_NCCC_FUNCTION_DISCLAIM_AUTH_, inNCCC_Func_Disclaim_Auth},
	{_NCCC_FUNCTION_DISPLAY_PLEASE_LOGON_FIRST_, inNCCC_Func_Display_Please_LOGON_First},
	{_NCCC_FUNCTION_SET_TXN_ONLINE_OFFLINE_FLOW_, inNCCC_Func_SetTxnOnlineOffline_Flow},
	{_NCCC_FUNCTION_BUILD_AND_SEND_PACKET_FLOW_, inNCCC_Func_BuildAndSendPacket_Flow},
	{_NCCC_FUNCTION_CARD_INQUERY_ISSUER_BANK_, inNCCC_Func_Card_Inquery_Issuer_Bank},
	{_NCCC_FUNCTION_CHECK_INQUIRY_ISSUER_AMOUNT_, inNCCC_Func_Check_Inquiry_Issuer_Amount},
	{_NCCC_FUNCTION_ESC_FUNCPROCESS_, inNCCC_Func_ESC_FuncProcess},
	{_NCCC_FUNCTION_CL_POWER_OFF_, inNCCC_Func_CL_Power_Off},
	{_NCCC_FUNCTION_AUTO_PRINT_TOTAL_REPORT_FLOW_, inNCCC_Func_AutoPrintTotalReport_Flow},
	{_NCCC_FUNCTION_AUTO_PRINT_DETAIL_REPORT_FLOW_, inNCCC_Func_AutoPrintDetailReport_Flow},
	{_NCCC_FUNCTION_CHECK_TXN_FUNCTION_WHEN_ECR_, inNCCC_Func_Check_Txn_Function_When_ECR},
	{_NCCC_FUNCTION_GET_PAY_ITEM_, inNCCC_Func_Get_PayItem},
	{_NCCC_FUNCTION_GET_OPT_PAY_ITEM_, inNCCC_Func_Get_OPT_PayItem},
	{_NCCC_FUNCTION_DCC_SETTLE_DOWNLOAD_, inNCCC_Func_DCC_Download_Settle},
	{_NCCC_FUNCTION_SEND_ECR_AFTER_PRINT_RECEIPT_, inNCCC_Func_Send_ECR_After_Print_Receipt},
	{_NCCC_FUNCTION_RECEIVE_EI_FLOW_, inNCCC_Func_Receive_EI_Flow},
	{_NCCC_FUNCTION_DUTY_FREE_REPRINT_, inNCCC_Func_Dutyfree_Reprint},
	{_NCCC_FUNCTION_CHECK_SPECIALCARD_IN_OPT_FLOW_, inNCCC_Func_Check_Special_Card_In_OPT_Flow},
	{_NCCC_FUNCTION_DELETE_CUS_075_FILES_, inNCCC_Func_Delete_Cus_075_Files},
	{_NCCC_FUNCTION_CUS_075_MENU_WHEN_ECR_, inNCCC_Func_CUS_075_Menu_When_ECR},
	{_NCCC_FUNCTION_REPRINT_CUSTOMER_FLOW_, inNCCC_Func_RePrint_Customer_Flow},
	{_NCCC_FUNCTION_BDAU_DELETE_ACCUM_BATCH_, inNCCC_Func_BDAU_DeleteAccumBatch},
	{_NCCC_FUNCTION_BDAU_SETTLEMENT_CHECK_, inNCCC_Func_BDAU_Settlement_Check},
	{_NCCC_FUNCTION_BDAU_DELETE_BATCH_BEFORE_TMS_, inNCCC_Func_BDAU_Delete_Batch_Before_TMS},
	{_NCCC_FUNCTION_CHECK_AMOUNT_BY_CARD_, inNCCC_Func_Check_Amount_by_Card},
	{_NCCC_FUNCTION_OPT_DELETE_BATCH_CUSTOMER_FLOW_, inNCCC_Func_OPT_DELETE_BATCH_Customer_Flow},
	{_NCCC_FUNCTION_All_HOST_MUST_SETTLE_FLOW_, inNCCC_Func_All_HOST_MUST_SETTLE_Customer_Flow},
	{_NCCC_FUNCTION_BEEP_AFTER_AUTH_, inNCCC_Func_Beep_After_Auth},
	{_NCCC_FUNCTION_CHECK_UPDATE_BATCH_NUM_, inNCCC_Func_Check_Update_Batch_Num},
	{_NCCC_FUNCTION_SAVE_TEMP_POBTRAN_FOR_ECR_MISSING_DATA_, inNCCC_Func_Save_Temp_PobTran_For_ECR_Missing_Data},
	{_EDC_BOOTING_FIX_INV_PROBLEM_AT_REBOOT_, inNCCC_Func_Fix_Inv_Problem_At_Reboot},
	{_NCCC_FUNCTION_DISPLAY_PLEASE_LOGON_FIRST_ONLY_ISO_, inNCCC_Func_Display_Please_LOGON_First_Only_ISO},
	{_NCCC_FUNCTION_UPDATE_PEM_BEFORE_TMS_MANUAL_DOWNLOAD_, inNCCC_Func_Update_PEM_Before_TMS_Manual_Download},
	{_NCCC_FUNCTION_CHECK_NOSIGNATURE_FINAL_, inNCCC_Func_Check_NoSignature_Final},
	{_NCCC_FUNCTION_SETTLEMENT_XML_NCCC_START_, inNCCC_Func_Settlement_XML_NCCC_START},
	{_NCCC_FUNCTION_SETTLEMENT_XML_NCCC_END_, inNCCC_Func_Settlement_XML_NCCC_END},
	{_NCCC_FUNCTION_CHECK_IS_TICKET_PURCHASE_, inNCCC_Func_Check_Is_Ticket_Purchase},
	{_NCCC_FUNCTION_GET_PDS0523_, inNCCC_Func_Get_PDS0523},
	{_NCCC_FUNCTION_GET_PDS0524_, inNCCC_Func_Get_PDS0524},
	{_NCCC_FUNCTION_GET_TXN_CATEGORY_, inNCCC_Func_Get_Txn_Category},
	{_NCCC_FUNCTION_GET_PDS0530_, inNCCC_Func_Get_PDS0530},

	/* NCCC_ATS FLOW */
	{_NCCC_ATS_FUNCTION_SET_TXN_ONLINE_OFFLINE_, inNCCC_ATS_Func_SetTxnOnlineOffline},
	{_NCCC_ATS_FUNCTION_BUILD_AND_SEND_PACKET_, inNCCC_ATS_Func_BuildAndSendPacket},

	/* NCCC_MFES FLOW */
	{_NCCC_MFES_FUNCTION_SET_TXN_ONLINE_OFFLINE_, inNCCC_MFES_Func_SetTxnOnlineOffline},
	{_NCCC_MFES_FUNCTION_BUILD_AND_SEND_PACKET_, inNCCC_MFES_Func_BuildAndSendPacket},

	/* NCCC_ESC */
	{_NCCC_ESC_CHECK_, inNCCC_ESC_Check},
	{_NCCC_ESC_UPLOAD_, inNCCC_ESC_Func_Upload},
	{_NCCC_ESC_UPLOAD_IDLE_, inNCCC_ESC_Func_Upload_Idle},
	{_NCCC_ESC_UPLOAD_SETTLE_, inNCCC_ESC_Func_Upload_Settle},

	/* NCCC_DCC */
	{_NCCC_DCC_CHECK_, inNCCC_DCC_CHECK},
	{_NCCC_DCC_CURRENCYOPTION_, inNCCC_DCC_CurrencyOption},
	{_NCCC_DCC_ONLINERATE_, inNCCC_DCC_OnlineRate},
	{_NCCC_DCC_FINAL_DCC_OPTION_, inNCCC_DCC_Final_DCC_Option},
	{_NCCC_DCC_FUNC_VOID_CONFIRM_, inNCCC_DCC_Func_VOID_Confirm},
	{_NCCC_DCC_FUNC_MUST_SETTLE_CHECK_, inNCCC_DCC_Func_Must_SETTLE},
	{_NCCC_DCC_TRANSACTION_ORIGIN_CHECK, inNCCC_DCC_Transacton_Origin_Check},
	{_NCCC_DCC_TRANSCTION_CHECK_, inNCCC_DCC_Transacton_Check},
	{_NCCC_DCC_FUNCTION_SET_TXN_ONLINE_OFFLINE_, inNCCC_DCC_Func_SetTxnOnlineOffline},
	{_NCCC_DCC_FUNCTION_BUILD_AND_SEND_PACKET_, inNCCC_DCC_Func_BuildAndSendPacket},
	{_NCCC_DCC_FUNCTION_EXCHANGE_RATE_DOWNLOAD_, inNCCC_DCC_Fun6_ExchangeRateDownload},
	{_NCCC_DCC_AUTO_DOWNLOAD_, inNCCC_DCC_AutoDownload},
	{_NCCC_DCC_TMS_SCHEDULE_, inNCCC_DCC_TMS_Schedule},
	{_NCCC_DCC_HOUR_NOTIFY_, inNCCC_DCC_Hour_Notify},
	{_NCCC_DCC_FUNCTION_CHECK_TRANS_FUNCTION_, inNCCC_DCC_Func_CheckTransactionFunction},

	/* EMV PROCESS FLOW */
	{_EMV_PROCESS_, inEMV_Process},

	/* FISC */
	{_FISC_CARD_PROCESS_TRAN_, inFISC_CardProcess},
	{_FISC_CARD_ENTER_PIN_, inFISC_GetUserPin},
	{_FISC_CARD_VERIFY_PIN_, inFISC_VerifyPin},
	{_FISC_CARD_POWER_ON_AND_SELECT_AID_, inFISC_Power_On_And_Select_AID},
	{_FISC_CARD_POWER_OFF_, inFISC_PowerOFF},

	/* TMS */
	{_FUNC5_TMS_SELECT_FLOW_, inNCCCTMS_TMS_Func5SelectFlow},
	{_FUNC5_TASK_REPORT_, inNCCCTMS_Func5ReturnTaskReport},
	{_FUNCTION_TMS_CONNECT_, inNCCCTMS_ConnectToServer},
	{_FUNCTION_TMS_DISCONNECT_, inNCCCTMS_DisConnect_From_Server},
	{_FUNCTION_TMS_SEND_RECEIVE_FLOW_, inNCCCTMS_FuncSendReceive_Flow},
	{_FUNCTION_TMS_SEND_RECEIVE_, inNCCCTMS_FuncSendReceive},
	{_FUNCTION_TMS_SEND_RECEIVE_FTPS_, inNCCCTMS_FuncSendReceive_FTPS},
	{_FUNCTION_TMS_RESULT_HANDLE_, inNCCCTMS_FuncResultHandle},
	{_FUN6_TMS_DOWNLOAD_, inNCCCTMS_Func6TMSDownload},
	{_FUN6_TRACELOG_UPLOAD_, inNCCCTMS_Func6TraceLog_Upload},
	{_FUNCTION_TMS_SCHEDULE_INQUIRE_, inNCCCTMS_Inquire},
	{_FUNCTION_TMS_SCHEDULE_DOWNLOAD_, inNCCCTMS_Schedule_Download},
	{_FUNCTION_TMS_DOWNLOAD_SETTLE_, inNCCCTMS_Download_Settle},
	{_FUNC5_DCC_DOWNLOAD_PARAMETER_, inNCCCTMS_DCC_FuncDownloadParameter},
	{_FUNCTION_TMS_IDLE_UPDATE_, inNCCCTMS_IdleUpdate},
	{_FUNCTION_TMS_PRINT_RETURN_TASK_, inNCCCTMS_PRINT_Return_Task},
	{_FUNCTION_TMS_REBOOT_, inNCCCTMS_Reboot},
	{_FUNCTION_TMS_SEND_RECEIVE_ONLY_PACKET_, inNCCCTMS_FuncSendReceive_Only_Packet},

	/* ECR SALE FLOW */
	{_FUNCTION_ECR_RECEIVE_TRANSACTION_REQUEST_, inECR_Receive_Transaction},
	{_FUNCTION_ECR_SEND_TRANSACTION_RESULT_, inECR_Send_Transaction_Result},
	{_FUNCTION_ECR_SEND_INQUIRY_RESULT_, inECR_Send_Inquiry_Result},
	{_FUNCTION_ECR_RECEIVE_SECOND_TRANSACTION_REQUEST_, inECR_Receive_Second_Transaction},
	{_FUNCTION_ECR_LOAD_TMK_, inECR_Load_TMK},
	{_FUNCTION_ECR_CUSTOMER_FLOW_, inECR_Customer_Flow},
	{_FUNCTION_ECR_NO_TRT_RETURN_FLOW_, inECR_No_TRT_Return_Flow},
	{_FUNCTION_ECR_CHOOSE_WAY_, inECR_Choose_Way_Flow},
	{_FUNCTION_ECR_CUS_096_IPASS_TRANSACTION_FLOW_, inECR_Cus_096_Ipass_Transaction_Flow},

	/* 票證DEMO用 */
	{_TICKET_DECIDE_TRANS_TYPE_, inNCCC_Ticket_Decide_Trans_Type},
	{_TICKET_CHECK_TRANS_ENABLE_, inNCCC_Ticket_Func_Check_Transaction_Function_Flow},
	{_TICKET_GET_PARM_, inNCCC_Ticket_Get_ParamValue},
	{_TICKET_GET_ESVC_CARD_FLOW_, inNCCC_Ticket_Get_Card_Flow},
	{_TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_, inNCCC_TICKET_Func_SetTxnOnlineOffline},
	{_TICKET_FUNCTION_BUILD_AND_SEND_PACKET_, inNCCC_TICKET_Func_BuildAndSendPacket},
	{_TICKET_CHECK_ACK_HOST_, inNCCC_Ticket_Check_AckHost},
	{_TICKET_CONNECT_, inNCCC_Ticket_Connect},
	{_TICKET_DISCONNECT_, inNCCC_Ticket_DisConnect},
	{_TICKET_GET_DAVTI_PARM_, inNCCC_Ticket_Get_DAVTI_Data},
	{_TICKET_UPDATE_ACCUM_, inACCUM_Update_Ticket_Flow},
	{_TICKET_UPDATE_BATCH_, inBATCH_FuncUpdateTxnRecord_Ticket_By_Sqlite},
	{_TICKET_UPDATE_INV_, inNCCC_Ticket_Func_UpdateTermInvNum},
	{_TICKET_IPASS_TAPCARD_FIRST_, inIPASS_First_Tap},
	{_TICKET_IPASS_TAPCARD_SECOND_, inIPASS_Second_Tap},
	{_TICKET_IPASS_QUERYING_FLOW_, inIPASS_Query_Flow},
	{_TICKET_ECC_API_FLOW_, inECC_APIflow},
	{_TICKET_ECC_SETTLE_FLOW_, inECC_Settle_Flow},
	{_TICKET_GET_VOID_TOP_UP_AMOUNT_FROM_BATCH_, inNCCC_Ticket_Get_Void_Top_Up_Amount_From_Batch},
	{_TICKET_DISPLAY_LOGON_RESULT_, inNCCC_Ticket_Display_LogOn_Result},
	{_TICKET_GET_CARD_ECC_RETRY_FLOW, inNCCC_Ticket_Get_Card_ECC_RETRY_FLOW},
	{_TICKET_AUTO_SIGN_ON_FLOW, inNCCC_Ticket_Auto_SignOn_Flow},
	{_TICKET_ECC_SET_UPDATE_BATCH_FLAG_, inECC_Set_Update_Batch_Flag},
	{_TICKET_INITIAL_LIGHT_STATE_, inNCCC_Ticket_Initial_Light_State},
	{_TICKET_FUNCTION_MUST_SETTLE_CHECK_, inNCCC_Ticket_Func_Must_SETTLE},
	{_TICKET_ICASH_DEMO_FLOW_, inICASH_DEMOflow},
	{_TICKET_ICASH_GET_CARD_AMT_, inICASH_Get_CardAmt},
	{_TICKET_ICASH_AUTO_ADD_FLOW_, inICASH_AutoAdd_Flow},
	{_TICKET_ICASH_TAPCARD_FIRST_, inICASH_First_Tap},
	{_TICKET_ICASH_TAPCARD_SECOND_, inICASH_Second_Tap},
	{_TICKET_ICASH_BUILD_SEND_RECV_PACKET_COMFIRM_, inICASH_Func_BuildAndSendPacket_Comfirm},
	{_TICKET_INQUIRY_BEEP_, inNCCC_Ticket_Inquiry_Beep},
	{_TICKET_INQUIRY_CARD_NO_FLOW_, inNCCC_Ticket_InquiryCardNo_Flow},

	/* 開機流程 */
	{_EDC_BOOTING_DISPLAY_INITIAL_, inFunc_Booting_Flow_Display_Initial},
	{_EDC_BOOTING_PRINT_INITIAL_, inFunc_Booting_Flow_Print_Initial},
	{_EDC_BOOTING_PRINT_IMAGE_INITIAL_, inFunc_Booting_Flow_Print_Image_Initial},
	{_EDC_BOOTING_CASTLE_LIBRARY_INITIAL_, inFunc_Booting_Flow_Castle_library_Initial},
	{_EDC_BOOTING_LOAD_TABLE_, inFunc_Booting_Flow_Load_Table},
	{_EDC_BOOTING_SYNC_DEBUG_, inFunc_Booting_Flow_Sync_Debug},
	{_EDC_BOOTING_COMM_INITIAL_, inFunc_Booting_Flow_Communication_Initial},
	{_EDC_BOOTING_ECR_INITIAL_, inFunc_Booting_Flow_ECR_Initial},
	{_EDC_BOOTING_EMV_INITIAL_, inFunc_Booting_Flow_EMV_Initial},
	{_EDC_BOOTING_TMSOK_CHECK_INITIAL_, inFunc_Booting_Flow_TMSOK_Check_Initial},
	{_EDC_BOOTING_ETICKET_INITIAL_, inFunc_Booting_Flow_Eticket_Initial},
	{_EDC_BOOTING_CTLS_INITIAL_, inFunc_Booting_Flow_CTLS_Initial},
	{_EDC_BOOTING_SQLITE_INITIAL_, inFunc_Booting_Flow_SQLite_Initial},
	{_EDC_BOOTING_CUP_LOGON_, inFunc_Booting_Flow_CUP_LOGON},
	{_EDC_BOOTING_TSAM_INITIAL_, inFunc_Booting_Flow_TSAM_Initial},
	{_EDC_BOOTING_TMS_INQUIRE_, inFunc_Booting_Flow_TMS_Parameter_Inquire},
	{_EDC_BOOTING_TMS_DCC_SCHEDULE_, inFunc_Booting_Flow_TMS_DCC_Schedule},
	{_EDC_BOOTING_POWER_ON_PASSWORD_, inFunc_Booting_Flow_Enter_PowerOn_Password},
	{_EDC_BOOTING_CLEAR_AP_DUMP_, inFunc_Booting_Flow_Clear_AP_Dump},
	{_EDC_BOOTING_PROCESS_CRADLE_, inFunc_Booting_Flow_Process_Cradle},
	{_EDC_BOOTING_POWER_MANAGEMENT_, inFunc_Booting_Flow_PowerManagement},
	{_EDC_BOOTING_REPRINT_POWEROFF_, inFunc_Booting_Flow_Reprint_Poweroff},
	{_EDC_BOOTING_UPDATE_PARAMETER_POWER_ON_, inFunc_Booting_Flow_Update_Parameter},
	{_EDC_BOOTING_UPDATE_SUCCESS_REPORT_, inFunc_Booting_Flow_Update_Success_Report},
	{_EDC_BOOTING_TMS_DOWNLOAD_, inFunc_Booting_Flow_TMS_Download},
	{_EDC_BOOTING_TMS_AP_UPDATE_RECHECK_OK_, inFunc_Booting_Flow_TMS_AP_Update_ReCheck_OK},
	{_EDC_BOOTING_LOAD_TMK_ECR_INITIAL_, inFunc_Booting_Flow_Load_TMK_ECR_Initial},
	{_EDC_BOOTING_SET_CASTLE_PWD_, inFunc_Booting_Flow_Set_Castle_PWD},
	{_EDC_BOOTING_GET_EDC_BOOT_TIME_, inFunc_Booting_Flow_Get_EDC_Boot_Time}, /*2019/2/27 Added by Hachi*/
	{_EDC_BOOTING_SET_SYSTEM_POWER_FUNCTION_, inFunc_Booting_Flow_Set_System_Power_Function},
	{_EDC_BOOTING_UI_INITIAL_, inFunc_Booting_Flow_UI_Initial},
	{_EDC_BOOTING_CHECK_TMS_, inNCCC_Func_Check_TMS_Booting_Flow},
	{_EDC_BOOTING_SET_BRIGHTNESS_, inFunc_Booting_Flow_Set_Brightness},
	{_EDC_BOOTING_WAIT_FOR_SWITCH_, inFunc_Booting_Flow_Wait_For_Switch},
	{_EDC_BOOTING_XML_LIB_INIT_, inFunc_Booting_Flow_XML_Lib_Init},
	{_EDC_BOOTING_CHECK_AND_RECOVER_SETTLEMENT_XML_, inFunc_Booting_Flow_Check_And_Recover_Settlement_XML},
	{_EDC_BOOTING_RENEW_AUTO_REBOOT_TIME_, inFunc_Booting_Flow_Renew_Auto_Reboot_Time},

	{_COMM_START_, inCOMM_ConnectStart},
	{_COMM_END_, inCOMM_End}, /* 目前只用來資訊回報的地方斷線 */

	{_TAKA_FUNCTION_CHECK_TRANS_FUNCTION_FLOW_, inTAKA_Func_Check_Transaction_Function_Flow},
	{_TAKA_FUNCTION_MUST_SETTLE_CHECK_, inTAKA_Func_Must_SETTLE},
	{_TAKA_FUNCTION_SET_TXN_ONLINE_OFFLINE_, inTAKA_Func_SetTxnOnlineOffline},
	{_TAKA_FUNCTION_BUILD_AND_SEND_PACKET_, inTAKA_Func_BuildAndSendPacket},

	{_SDK_COMMINITIAL_, inNexsysSDK_InitCommDevice},

	/* EW FLOW */
	{_NCCC_EW_FUNCTION_SET_TXN_ONLINE_OFFLINE_, inNCCC_EW_Func_SetTxnOnlineOffline},
	{_NCCC_EW_FUNCTION_BUILD_AND_SEND_PACKET_, inNCCC_EW_Func_BuildAndSendPacket},
	{_NCCC_EW_FUNCTION_CHECK_TRANS_FUNCTION_FLOW_, inNCCC_EW_Func_Check_Transaction_Function_Flow},

	/* 信託 */
	{_NCCC_TRUST_DEAL_TRUST_TXN_PARAMETER_OPT_, inNCCC_Trust_Deal_Trust_Txn_Parameter_OPT},
	{_TRUST_UPDATE_BATCH_, inBATCH_FuncUpdateTxnRecord_Trust_By_Sqlite},
	{_NCCC_TRUST_VOID_CONFIRM_, inNCCC_Trust_VOID_Confirm},
	{_NCCC_TRUST_CHECK_TRANS_FUNCTION_FLOW_, inNCCC_Trust_Check_Transaction_Function_Flow},
	{_NCCC_TRUST_MUST_SETTLE_CHECK_, inNCCC_Trust_Func_Must_SETTLE},
	{_NCCC_TRUST_SEND_REVERSAL_, inNCCC_Trust_SendReversal},
};


int inFUNC_TABLE_Test(void)
{
        inLogPrintf(AT, "inFUNC_TABLE_Test()");
        return (VS_SUCCESS);
}
