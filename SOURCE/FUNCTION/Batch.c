#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <unistd.h>
#include <sqlite3.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/AllStruct.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DisTouch.h"
#include "../EVENT/Flow.h"
#include "../EVENT/MenuMsg.h"
#include "Sqlite.h"
#include "Function.h"
#include "FuncTable.h"
#include "Accum.h"
#include "File.h"
#include "HDT.h"
#include "Batch.h"
#include "CDT.h"
#include "CFGT.h"
#include "HDPT.h"
#include "EDC.h"
#include "Card.h"
#include "Signpad.h"
#include "Utility.h"
#include "../../NCCC/NCCCsrc.h"
#include "../../NCCC/NCCCTicketSrc.h"
#include "../../NCCC/NCCCesc.h"
#include "../../NCCC/NCCCloyalty.h"
#include "../../NCCC/NCCCTrust.h"
#include "ECR.h"
#include "TDT.h"

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))

extern	int		ginDebug;  /* Debug使用 extern */
extern	int		ginFindRunTime;
extern	int		ginHalfLCD;
extern	char		gszTranDBPath[100 + 1];
extern	char		gszReprintDBPath[100 + 1];
extern	unsigned char	guszEnormousNoNeedResetBit;	/* 資料庫是否須從頭找 */
extern	ECR_TABLE	gsrECROb;
unsigned long		gulADVHandle, gulBATHandle, gulBKEYHandle;
unsigned char		guszBATFileName[20 + 1], guszBKEYFileName[20 + 1], guszADVFileName[20 + 1];

/* 2023/5/31 上午 9:33 uszUpdated BLOB DEFAULT 0 這個組合有點弔詭，下SQL指令僅能判別初始狀態的uszUpdated = 0(O)uszUpdated = x'30'(X)，但若嚐試改動他的值，
  比方說改成1，uszUpdated = 1(X)uszUpdated = x'31'(X)，但其他沒有Default 0的欄位似乎沒有這種特性，建議修改 */

SQLITE_TAG_TABLE TABLE_BATCH_TAG[] ={
	{"inTableID", "INTEGER", "PRIMARY KEY", ""}, /* Table ID Primary key, sqlite table專用避免PRIMARY KEY重複 */
	{"inCode", "INTEGER", "", ""}, /* Trans Code */
	{"inOrgCode", "INTEGER", "", ""}, /* Original Trans Code  */
	{"inPrintOption", "INTEGER", "", ""}, /* Print Option Flag */
	{"inHDTIndex", "INTEGER", "", ""}, /* 紀錄HDTindex */
	{"inCDTIndex", "INTEGER", "", ""}, /* 紀錄CDTindex */
	{"inCPTIndex", "INTEGER", "", ""}, /* 紀錄CPTindex */
	{"inTxnResult", "INTEGER", "", ""}, /* 紀錄交易結果 */
	{"inChipStatus", "INTEGER", "", ""}, /* 0 NOT_USING_CHIP, 1 EMV_CARD, 2 EMV_EASY_ENTRY_CARD */
	{"inFiscIssuerIDLength", "INTEGER", "", ""}, /* 金融卡發卡單位代號長度 */
	{"inFiscCardCommentLength", "INTEGER", "", ""}, /* 金融卡備註欄長度 */
	{"inFiscAccountLength", "INTEGER", "", ""}, /* 金融卡帳號長度 */
	{"inFiscSTANLength", "INTEGER", "", ""}, /* 金融卡交易序號長度 */
	{"inESCTransactionCode", "INTEGER", "", ""}, /* ESC組ISO使用 重新上傳使用 Transaction Code沒辦法存在Batch */
	{"inESCUploadMode", "INTEGER", "", ""}, /* 標示支不支援ESC */
	{"inESCUploadStatus", "INTEGER", "", ""}, /* 標示ESC上傳狀態 */
	{"inSignStatus", "INTEGER", "", ""}, /* 簽名檔狀態(有 免簽 或 Bypass) ESC電文使用 */
	{"inHGCreditHostIndex", "INTEGER", "", ""}, /* 聯合_HAPPY_GO_信用卡主機 */
	{"inHGCode", "INTEGER", "", ""}, /* 聯合_HAPPY_GO_交易碼 */
	{"lnTxnAmount", "INTEGER", "", ""}, /* The transaction amount, such as a SALE */
	{"lnOrgTxnAmount", "INTEGER", "", ""}, /* The ORG transaction amount, such as a SALE */
	{"lnTipTxnAmount", "INTEGER", "", ""}, /* The transaction amount, such as a TIP */
	{"lnAdjustTxnAmount", "INTEGER", "", ""}, /* The transaction amount, such as a ADJUST */
	{"lnTotalTxnAmount", "INTEGER", "", ""}, /* The transaction amount, such as a TOTAL */
	{"lnOilAmount", "INTEGER", "", ""}, /* 一般交易使用的加油金 */
	{"lnInvNum", "INTEGER", "", ""}, /* 調閱編號  */
	{"lnOrgInvNum", "INTEGER", "", ""}, /* Original 調閱編號  */
	{"lnBatchNum", "INTEGER", "", ""}, /* Batch Number */
	{"lnOrgBatchNum", "INTEGER", "", ""}, /* Original Batch Number */
	{"lnSTANNum", "INTEGER", "", ""}, /* Stan Number */
	{"lnOrgSTANNum", "INTEGER", "", ""}, /* Original Stan Number */
	{"lnInstallmentPeriod", "INTEGER", "", ""}, /* 分期付款_期數 */
	{"lnInstallmentDownPayment", "INTEGER", "", ""}, /* 分期付款_頭期款 */
	{"lnInstallmentPayment", "INTEGER", "", ""}, /* 分期付款_每期款 */
	{"lnInstallmentFormalityFee", "INTEGER", "", ""}, /* 分期付款_手續費 */
	{"lnRedemptionPoints", "INTEGER", "", ""}, /* 紅利扣抵_扣抵紅利點數 */
	{"lnRedemptionPointsBalance", "INTEGER", "", ""}, /* 紅利扣抵_剩餘紅利點數 */
	{"lnRedemptionPaidCreditAmount", "INTEGER", "", ""}, /* 紅利扣抵_支付金額 */
	{"lnHGTransactionType", "INTEGER", "", ""}, /* 聯合_HAPPY GO_交易類別 */
	{"lnHGPaymentType", "INTEGER", "", ""}, /* 聯合_HAPPY_GO_支付工具 */
	{"lnHGPaymentTeam", "INTEGER", "", ""}, /* 聯合_HAPPY_GO_支付工具_主機回_*/
	{"lnHGBalancePoint", "INTEGER", "", ""}, /* 聯合_HAPPY_GO_剩餘點數 */
	{"lnHGTransactionPoint", "INTEGER", "", ""}, /* 聯合_HAPPY_GO_交易點數  合計 */
	{"lnHGAmount", "INTEGER", "", ""}, /* 聯合_HAPPY_GO_扣抵後金額  (商品金額 = lnHGAmount + lnHGRedeemAmt) */
	{"lnHGRedeemAmount", "INTEGER", "", ""}, /* 聯合_HAPPY_GO_扣抵金額 */
	{"lnHGRefundLackPoint", "INTEGER", "", ""}, /* 聯合_HAPPY_GO_不足點數 */
	{"lnHGBatchIndex", "INTEGER", "", ""}, /* 聯合_HAPPY_GO_主機當下批次號碼 */
	{"lnHG_SPDH_OrgInvNum", "INTEGER", "", ""}, /* HAPPY_GO取消用INV */
	{"lnHGSTAN", "INTEGER", "", ""}, /* HAPPY_GO STAN */
	{"lnCUPUPlanDiscountedAmount", "INTEGER", "", ""}, /* 銀聯優計畫折價後金額 */
	{"lnCUPUPlanPreferentialAmount", "INTEGER", "", ""}, /* 銀聯優計畫優惠金額 */
	{"szAuthCode", "BLOB", "", ""}, /* Auth Code */
	{"szMPASAuthCode", "BLOB", "", ""}, /* MPAS Auth Code */
	{"szRespCode", "BLOB", "", ""}, /* Response Code */
	{"szStoreID", "BLOB", "", ""}, /* StoreID */
	{"szCardLabel", "BLOB", "", ""}, /* 卡別  */
	{"szPAN", "BLOB", "", ""}, /* 卡號  */
	{"szDate", "BLOB", "", ""}, /* YYYYMMDD */
	{"szOrgDate", "BLOB", "", ""}, /* YYYYMMDD */
	{"szTime", "BLOB", "", ""}, /* HHMMSS */
	{"szOrgTime", "BLOB", "", ""}, /* HHMMSS */
	{"szCardTime", "BLOB", "", ""}, /* 晶片卡讀卡時間 , YYYYMMDDHHMMSS */
	{"szRefNo", "BLOB", "", ""}, /* 序號  */
	{"szExpDate", "BLOB", "", ""}, /* Expiration date */
	{"szServiceCode", "BLOB", "", ""}, /* Service code from track */
	{"szCardHolder", "BLOB", "", ""}, /* 持卡人名字 */
	{"szAMEX4DBC", "BLOB", "", ""},
	{"szFiscIssuerID", "BLOB", "", ""}, /* 發卡單位代號 */
	{"szFiscCardComment", "BLOB", "", ""}, /* 金融卡備註欄 */
	{"szFiscAccount", "BLOB", "", ""}, /* 金融卡帳號 */
	{"szFiscOutAccount", "BLOB", "", ""}, /* 金融卡轉出帳號 */
	{"szFiscSTAN", "BLOB", "", ""}, /* 金融卡交易序號 */
	{"szFiscTacLength", "BLOB", "", ""}, /* 金融卡Tac長度 */
	{"szFiscTac", "BLOB", "", ""}, /* 金融卡Tac */
	{"szFiscTCC", "BLOB", "", ""}, /* 端末設備查核碼 */
	{"szFiscMCC", "BLOB", "", ""}, /* 金融卡MCC */
	{"szFiscRRN", "BLOB", "", ""}, /* 金融卡調單編號 */
	{"szFiscRefundDate", "BLOB", "", ""}, /* 金融卡退貨原始交易日期(YYYYMMDD) */
	{"szFiscDateTime", "BLOB", "", ""}, /* 計算TAC(S2)的交易日期時間 */
	{"szFiscPayDevice", "BLOB", "", ""}, /* 金融卡付款裝置 1 = 手機 2 = 卡片 */
	{"szFiscMobileDevice", "BLOB", "", ""}, /* SE 類型，0x05：雲端卡片(Cloud-Based) */
	{"szFiscMobileNFType", "BLOB", "", ""}, /* 行動金融卡是否需輸入密碼 00不需要 01視情況 02一定要 */
	{"szFiscMobileNFSetting", "BLOB", "", ""}, /* 近端交易類型設定 0x00：Single Issuer Wallet 0x01：國內Third-Party Wallet 0x02~9：保留 0x0A：其他 */
	{"szInstallmentIndicator", "BLOB", "", ""},
	{"szRedeemIndicator", "BLOB", "", ""},
	{"szRedeemSignOfBalance", "BLOB", "", ""},
	{"szHGCardLabel", "BLOB", "", ""}, /* HAPPY_GO 卡別 */
	{"szHGPAN", "BLOB", "", ""}, /* HAPPY_GO Account number */
	{"szHGAuthCode", "BLOB", "", ""}, /* HAPPY_GO 授權碼 */
	{"szHGRefNo", "BLOB", "", ""}, /* HAPPY_GO Reference Number */
	{"szHGRespCode", "BLOB", "", ""}, /* HG Response Code */
	{"szCUP_TN", "BLOB", "", ""}, /* CUP Trace Number (TN) */
	{"szCUP_TD", "BLOB", "", ""}, /* CUP Transaction Date (MMDD) */
	{"szCUP_TT", "BLOB", "", ""}, /* CUP Transaction Time (hhmmss) */
	{"szCUP_RRN", "BLOB", "", ""}, /* CUP Retrieve Reference Number (CRRN) */
	{"szCUP_STD", "BLOB", "", ""}, /* CUP Settlement Date(MMDD) Of Host Response */
	{"szCUP_EMVAID", "BLOB", "", ""}, /* CUP晶片交易存AID帳單列印使用 */
	{"szTranAbbrev", "BLOB", "", ""}, /* Tran abbrev for reports */
	{"szIssueNumber", "BLOB", "", ""},
	{"szStore_DREAM_MALL", "BLOB", "", ""}, /* 存Dream_Mall Account Number And Member ID*/
	{"szDCC_FCNFR", "BLOB", "", ""}, /* Foreign Currency No. For Rate */
	{"szDCC_AC", "BLOB", "", ""}, /* Action Code */
	{"szDCC_FCN", "BLOB", "", ""}, /* Foreign Currency Number */
	{"szDCC_FCA", "BLOB", "", ""}, /* Foreign Currency Amount */
	{"szDCC_FCMU", "BLOB", "", ""}, /* Foreign Currency Minor Unit */
	{"szDCC_FCAC", "BLOB", "", ""}, /* Foreign currcncy Alphabetic Code */
	{"szDCC_ERMU", "BLOB", "", ""}, /* Exchange Rate Minor Unit */
	{"szDCC_ERV", "BLOB", "", ""}, /* Exchange Rate Value */
	{"szDCC_IRMU", "BLOB", "", ""}, /* Inverted Rate Minor Unit */
	{"szDCC_IRV", "BLOB", "", ""}, /* Inverted Rate Value */
	{"szDCC_IRDU", "BLOB", "", ""}, /* Inverted Rate Display Unit */
	{"szDCC_MPV", "BLOB", "", ""}, /* Markup Percentage Value */
	{"szDCC_MPDP", "BLOB", "", ""}, /* Markup Percentage Decimal Point */
	{"szDCC_CVCN", "BLOB", "", ""}, /* Commissino Value Currency Number */
	{"szDCC_CVCA", "BLOB", "", ""}, /* Commission Value Currency Amount */
	{"szDCC_CVCMU", "BLOB", "", ""}, /* Commission Value Currency Minor Unit */
	{"szDCC_TIPFCA", "BLOB", "", ""}, /* Tip Foreign Currency Amount */
	{"szDCC_OTD", "BLOB", "", ""}, /* Original Transaction Date & Time (MMDD) */
	{"szDCC_OTA", "BLOB", "", ""}, /* Original Transaction Amount */
	{"szProductCode", "BLOB", "", ""}, /* 產品代碼 */
	{"szAwardNum", "BLOB", "", ""}, /* 優惠個數 */
	{"szAwardSN", "BLOB", "", ""}, /* 優惠序號(Award S/N) TID(8Bytes)+YYYYMMDDhhmmss(16 Bytes)，共22Bytes */
	{"szTxnNo", "BLOB", "", ""}, /* 交易編號 */
	{"szMCP_BANKID", "BLOB", "", ""}, /* 行動支付標記 金融機構代碼 */
	{"szPayItemCode", "BLOB", "", ""}, /* 繳費項目代碼 */
	{"szTableTD_Data", "BLOB", "", ""}, /* Table TD的資料 */
	{"szDFSTraceNum", "BLOB", "", ""}, /* DFS交易系統追蹤號 */
	{"szCheckNO", "BLOB", "", ""}, /* 檢查碼，目前FOR SDK使用 */
	{"szEIVI_BANKID", "BLOB", "", ""}, /* E Invoice vehicle I ssue Bank ID 支援電子發票載具之發卡行代碼 */
	{"szBarCodeData", "BLOB", "", ""}, /* 掃碼交易資料內容 */
	{"szUnyTransCode", "BLOB", "", ""}, /* Uny交易碼 */
	{"szUnyMaskedCardNo", "BLOB", "", ""}, /* 遮掩交易卡號資料(非完整卡號，有遮掩，故另設欄位) */
	{"szEInvoiceHASH", "BLOB", "", ""}, /* 電子發票HASH 值（發卡銀行若無參加電子發票載具，則此欄位回覆值 為空白 SPACE 。） */
	{"szUnyCardLabelLen", "BLOB", "", ""}, /* 卡別名稱長度，szMCP_BANKID為Twin卡專用，故另設欄位 */
	{"szUnyCardLabel", "BLOB", "", ""}, /* 卡別名稱，szMCP_BANKID為Twin卡專用，故另設欄位 */
	{"szFPG_FTC_Invoice", "BLOB", "", ""}, /* 台塑生醫生技客製化專屬【005】　台亞福懋加油站客製化專屬【006】發票號碼 */
	{"szUPlan_Coupon", "BLOB", "", ""}, /* 銀聯優計畫的優惠訊息 */
	{"szUPlan_RemarksInformation", "BLOB", "", ""}, /* 銀聯優計畫-備註訊息 */
	{"szFlightTicketTransBit", "BLOB", "", ""}, /* 判斷是否是機票交易 不設定為空值 機票交易為1 非機票交易為2 */
	{"szFlightTicketPDS0523", "BLOB", "", ""}, /* 出發地機場代碼（PDS 0523）左靠右補空白 */
	{"szFlightTicketPDS0524", "BLOB", "", ""}, /* 目的地機場代碼（PDS 0524）左靠右補空白 */
	{"szFlightTicketPDS0530", "BLOB", "", ""}, /* 航班號碼（PDS 0530） */
	{"uszWAVESchemeID", "BLOB", "", ""}, /* WAVE 使用用於組電文 Field_22 */
	{"uszVOIDBit", "BLOB", "", ""}, /* 負向交易 */
	{"uszUpload1Bit", "BLOB", "", ""}, /* Offline交易使用 (原交易advice是否已上傳)*/
	{"uszUpload2Bit", "BLOB", "", ""}, /* Offline交易使用 (當前交易是否為advice)*/
	{"uszUpload3Bit", "BLOB", "", ""}, /* Offline交易使用 */
	{"uszReferralBit", "BLOB", "", ""}, /* ISO Response Code 【01】【02】使用 */
	{"uszOfflineBit", "BLOB", "", ""}, /* 離線交易 */
	{"uszManualBit", "BLOB", "", ""}, /* Manual Keyin */
	{"uszNoSignatureBit", "BLOB", "", ""}, /* 免簽名使用 (免簽名則為true)*/
	{"uszCUPTransBit", "BLOB", "", ""}, /* 是否為CUP */
	{"uszFiscTransBit", "BLOB", "", ""}, /* SmartPay交易，是否為金融卡 */
	{"uszContactlessBit", "BLOB", "", ""}, /* 是否為非接觸式 */
	{"uszEMVFallBackBit", "BLOB", "", ""}, /* 是否要啟動fallback */
	{"uszInstallmentbit", "BLOB", "", ""}, /* Installment */
	{"uszRedeemBit", "BLOB", "", ""}, /* Redemption */
	{"uszForceOnlineBit", "BLOB", "", ""}, /* 組電文使用 Field_25 Point of Service Condition Code */
	{"uszMail_OrderBit", "BLOB", "", ""}, /* 組電文使用 Field_25 Point of Service Condition Code */
	{"uszDCCTransBit", "BLOB", "", ""}, /* 是否為DCC交易 */
	{"uszNCCCDCCRateBit", "BLOB", "", ""},
	{"uszCVV2Bit", "BLOB", "", ""},
	{"uszRewardSuspendBit", "BLOB", "", ""},
	{"uszRewardL1Bit", "BLOB", "", ""}, /* 要印L1 */
	{"uszRewardL2Bit", "BLOB", "", ""}, /* 要印L2 */
	{"uszRewardL5Bit", "BLOB", "", ""}, /* 要印L5 */
	{"uszField24NPSBit", "BLOB", "", ""},
	{"uszVEPS_SignatureBit", "BLOB", "", ""}, /* VEPS 免簽名是否成立 */
	{"uszTCUploadBit", "BLOB", "", ""}, /* TCUpload是否已上傳 */
	{"uszFiscConfirmBit", "BLOB", "", ""}, /* SmartPay 0220 是否已上傳 */
	{"uszFiscVoidConfirmBit", "BLOB", "", ""}, /* SmartPay Void 0220 是否已上傳 */
	{"uszPinEnterBit", "BLOB", "", ""}, /* 此筆交易是否有鍵入密碼(只能確認原交易，若該筆之後的調整或取消不會將此Bit Off) */
	{"uszL2PrintADBit", "BLOB", "", ""}, /* L2是否印AD，因L2原交易取消要判斷，只好增加 */
	{"uszInstallment", "BLOB", "", ""}, /* HappyGo分期交易 */
	{"uszRedemption", "BLOB", "", ""}, /* HappyGo點數兌換 */
	{"uszHappyGoSingle", "BLOB", "", ""}, /* HappyGo交易 */
	{"uszHappyGoMulti", "BLOB", "", ""}, /* HappyGo混合交易 */
	{"uszCLSBatchBit", "BLOB", "", ""}, /* 是否已接續上傳 */
	{"uszTxNoCheckBit", "BLOB", "", ""}, /* 商店自存聯卡號遮掩開關 */
	{"uszSpecial00Bit", "BLOB", "", ""}, /* 特殊卡別參數檔，活動代碼00表示免簽(只紀錄，主要看uszNoSignatureBit) */
	{"uszSpecial01Bit", "BLOB", "", ""}, /* 特殊卡別參數檔，活動代碼01表示ECR回傳明碼(先決條件ECR卡號遮掩有開才做判斷) */
	{"uszRefundCTLSBit", "BLOB", "", ""}, /* 用在簽單印(W) 因為送電文contactless bit已OFF轉Manual Key in */
	{"uszMPASTransBit", "BLOB", "", ""}, /* 標示為小額交易 */
	{"uszMPASReprintBit", "BLOB", "", ""}, /* 標示該小額交易是否可重印 */
	{"uszMobilePayBit", "BLOB", "", ""}, /* 判斷是不是行動支付 Table NC */
	{"uszUCARDTransBit", "BLOB", "", ""}, /* 判斷是否是自由品牌卡 */
	{"uszUnyTransBit", "BLOB", "", ""}, /* 是否是Uny交易 */
	{"uszESCOrgUploadBit", "BLOB", "", ""}, /* 用於標明原交易的ESC上傳狀態 */
	{"uszESCTipUploadBit", "BLOB", "", ""}, /* 用於標明Tip的ESC上傳狀態 */
	{"uszTakaTransBit", "BLOB", "", ""}, /* 客製化026、021 Taka社員卡交易 */
	{"uszUPlanECRBit", "BLOB", "", ""}, /* 判斷是否為優計畫ECR規格(用以決定是否回傳hash value) */
	{"uszCUPEMVQRCodeBit", "BLOB", "", ""}, /* 判斷是否使用銀聯QRCode支付 */
	{"uszUPlanTransBit", "BLOB", "", ""}, /* 判斷是否為優計畫交易(使用銀聯優惠券) */
	{"uszUpdated", "BLOB", "", "DEFAULT 0"}, /* For SQLite使用，pobTran中不存，若設為1則代表該紀錄已不用，初始值設為0 */
	{""},
};

SQLITE_TAG_TABLE TABLE_EMV_BATCH_TAG[] = 
{
	{"inTableID"					,"INTEGER"	,"PRIMARY KEY"	,""},	/* Table ID Primary key, sqlite table專用避免PRIMARY KEY重複 */
	{"inBatchTableID"				,"INTEGER"	,""		,""},	/* Table ID FOREIGN  key, sqlite table專用，用來對應Batch Table的FOREIGN Key */
	{"inEMVCardDecision"				,"INTEGER"	,""		,""},
	{"in50_APLabelLen"				,"INTEGER"	,""		,""},
	{"in5A_ApplPanLen"				,"INTEGER"	,""		,""},
	{"in5F20_CardholderNameLen"			,"INTEGER"	,""		,""},
	{"in5F24_ExpireDateLen"				,"INTEGER"	,""		,""},
	{"in5F28_IssuerCountryCodeLen"			,"INTEGER"	,""		,""},
	{"in5F2A_TransCurrCodeLen"			,"INTEGER"	,""		,""},
	{"in5F34_ApplPanSeqnumLen"			,"INTEGER"	,""		,""},
	{"in71_IssuerScript1Len"			,"INTEGER"	,""		,""},
	{"in72_IssuerScript2Len"			,"INTEGER"	,""		,""},
	{"in82_AIPLen"					,"INTEGER"	,""		,""},
	{"in84_DFNameLen"				,"INTEGER"	,""		,""},
	{"in8A_AuthRespCodeLen"				,"INTEGER"	,""		,""},
	{"in91_IssuerAuthDataLen"			,"INTEGER"	,""		,""},
	{"in95_TVRLen"					,"INTEGER"	,""		,""},
	{"in9A_TranDateLen"				,"INTEGER"	,""		,""},
	{"in9B_TSILen"					,"INTEGER"	,""		,""},
	{"in9C_TranTypeLen"				,"INTEGER"	,""		,""},
	{"in9F02_AmtAuthNumLen"				,"INTEGER"	,""		,""},
	{"in9F03_AmtOtherNumLen"			,"INTEGER"	,""		,""},
	{"in9F08_AppVerNumICCLen"			,"INTEGER"	,""		,""},
	{"in9F09_TermVerNumLen"				,"INTEGER"	,""		,""},
	{"in9F10_IssuerAppDataLen"			,"INTEGER"	,""		,""},
	{"in9F18_IssuerScriptIDLen"			,"INTEGER"	,""		,""},
	{"in9F1A_TermCountryCodeLen"			,"INTEGER"	,""		,""},
	{"in9F1E_IFDNumLen"				,"INTEGER"	,""		,""},
	{"in9F26_ApplCryptogramLen"			,"INTEGER"	,""		,""},
	{"in9F27_CIDLen"				,"INTEGER"	,""		,""},
	{"in9F33_TermCapabilitiesLen"			,"INTEGER"	,""		,""},
	{"in9F34_CVMLen"				,"INTEGER"	,""		,""},
	{"in9F35_TermTypeLen"				,"INTEGER"	,""		,""},
	{"in9F36_ATCLen"				,"INTEGER"	,""		,""},
	{"in9F37_UnpredictNumLen"			,"INTEGER"	,""		,""},
	{"in9F41_TransSeqCounterLen"			,"INTEGER"	,""		,""},
	{"in9F42_ApplicationCurrencyCodeLen"		,"INTEGER"	,""		,""},
	{"in9F5A_Application_Program_IdentifierLen"	,"INTEGER"	,""		,""},
	{"in9F5B_ISRLen"				,"INTEGER"	,""		,""},
        {"in9F60_CouponInformationLen"                  ,"INTEGER"	,""		,""},
	{"in9F63_CardProductLabelInformationLen"	,"INTEGER"	,""		,""},
	{"in9F66_QualifiersLen"				,"INTEGER"	,""		,""},
	{"in9F6C_Card_Transaction_QualifiersLen"	,"INTEGER"	,""		,""},
	{"in9F6E_From_Factor_IndicatorLen"		,"INTEGER"	,""		,""},
	{"in9F74_TLVLen"				,"INTEGER"	,""		,""},
	{"in9F7C_PartnerDiscretionaryDataLen"		,"INTEGER"	,""		,""},
	{"inDF69_NewJspeedyModeLen"			,"INTEGER"	,""		,""},
	{"inDF8F4F_TransactionResultLen"		,"INTEGER"	,""		,""},
	{"inDFEC_FallBackIndicatorLen"			,"INTEGER"	,""		,""},
	{"inDFED_ChipConditionCodeLen"			,"INTEGER"	,""		,""},
	{"inDFEE_TerEntryCapLen"			,"INTEGER"	,""		,""},
	{"inDFEF_ReasonOnlineCodeLen"			,"INTEGER"	,""		,""},
	{"usz50_APLabel"				,"BLOB"		,""		,""},
	{"usz5A_ApplPan"				,"BLOB"		,""		,""},
	{"usz5F20_CardholderName"			,"BLOB"		,""		,""},
	{"usz5F24_ExpireDate"				,"BLOB"		,""		,""},
	{"usz5F2A_TransCurrCode"			,"BLOB"		,""		,""},
	{"usz5F28_IssuerCountryCode"			,"BLOB"		,""		,""},
	{"usz5F34_ApplPanSeqnum"			,"BLOB"		,""		,""},
	{"usz71_IssuerScript1"				,"BLOB"		,""		,""},
	{"usz72_IssuerScript2"				,"BLOB"		,""		,""},
	{"usz82_AIP"					,"BLOB"		,""		,""},
	{"usz84_DF_NAME"				,"BLOB"		,""		,""},
	{"usz8A_AuthRespCode"				,"BLOB"		,""		,""},
	{"usz91_IssuerAuthData"				,"BLOB"		,""		,""},
	{"usz95_TVR"					,"BLOB"		,""		,""},
	{"usz9A_TranDate"				,"BLOB"		,""		,""},
	{"usz9B_TSI"					,"BLOB"		,""		,""},
	{"usz9C_TranType"				,"BLOB"		,""		,""},
	{"usz9F02_AmtAuthNum"				,"BLOB"		,""		,""},
	{"usz9F03_AmtOtherNum"				,"BLOB"		,""		,""},
	{"usz9F08_AppVerNumICC"				,"BLOB"		,""		,""},
	{"usz9F09_TermVerNum"				,"BLOB"		,""		,""},
	{"usz9F10_IssuerAppData"			,"BLOB"		,""		,""},
	{"usz9F18_IssuerScriptID"			,"BLOB"		,""		,""},
	{"usz9F1A_TermCountryCode"			,"BLOB"		,""		,""},
	{"usz9F1E_IFDNum"				,"BLOB"		,""		,""},
	{"usz9F26_ApplCryptogram"			,"BLOB"		,""		,""},
	{"usz9F27_CID"					,"BLOB"		,""		,""},
	{"usz9F33_TermCapabilities"			,"BLOB"		,""		,""},
	{"usz9F34_CVM"					,"BLOB"		,""		,""},
	{"usz9F35_TermType"				,"BLOB"		,""		,""},
	{"usz9F36_ATC"					,"BLOB"		,""		,""},
	{"usz9F37_UnpredictNum"				,"BLOB"		,""		,""},
	{"usz9F41_TransSeqCounter"			,"BLOB"		,""		,""},
	{"usz9F42_ApplicationCurrencyCode"		,"BLOB"		,""		,""},
	{"usz9F5A_Application_Program_Identifier"	,"BLOB"		,""		,""},
	{"usz9F5B_ISR"					,"BLOB"		,""		,""},
        {"usz9F60_CouponInformation"			,"BLOB"		,""		,""},
	{"usz9F63_CardProductLabelInformation"		,"BLOB"		,""		,""},
	{"usz9F66_Qualifiers"				,"BLOB"		,""		,""},
	{"usz9F6C_Card_Transaction_Qualifiers"		,"BLOB"		,""		,""},
	{"usz9F7C_PartnerDiscretionaryData"		,"BLOB"		,""		,""},
	{"usz9F6E_From_Factor_Indicator"		,"BLOB"		,""		,""},
	{"usz9F74_TLV"					,"BLOB"		,""		,""},
	{"uszDF69_NewJspeedyMode"			,"BLOB"		,""		,""},
	{"uszDF8F4F_TransactionResult"			,"BLOB"		,""		,""},
	{"uszDFEC_FallBackIndicator"			,"BLOB"		,""		,""},
	{"uszDFED_ChipConditionCode"			,"BLOB"		,""		,""},
	{"uszDFEE_TerEntryCap"				,"BLOB"		,""		,""},
	{"uszDFEF_ReasonOnlineCode"			,"BLOB"		,""		,""},
	{""},
};

SQLITE_TAG_TABLE TABLE_REPRINT_DATA_TAG[] = 
{
	{"inTableID"			,"INTEGER"	,"PRIMARY KEY"	,""},	/* Table ID Primary key, sqlite table專用避免PRIMARY KEY重複 */
	{"szINV_Data"			,"TEXT"		,""		,""},	/* *INV: %06d */
	{"szAmount"			,"TEXT"		,""		,""},	/* 2.Print Amount */
	{"szTransType"			,"TEXT"		,""		,""},	/* 3.Transaction Type */
	{"szPrintCardType"		,"TEXT"		,""		,""},	/* 4.Print Card Type */
	{"szPrintPAN"			,"TEXT"		,""		,""},	/* 5.Print Card Number */
	{"szDate"			,"TEXT"		,""		,""},	/* Data */
	{"szTime"			,"TEXT"		,""		,""},	/* Time */
	{"szAuthCode"			,"TEXT"		,""		,""},	/* Approved No.*/
	{"szChekNo"			,"TEXT"		,""		,""},	/* Check No.*/
	{"szReponseCode"		,"TEXT"		,""		,""},	/* RESPONSE CODE */
	{"szStoreID"			,"TEXT"		,""		,""},	/* Store ID */
	{"szReprintDate"		,"TEXT"		,""		,""},	/* 列印時間(執行重印的時間) */
	{"szReprintTime"		,"TEXT"		,""		,""},	/* 列印時間(執行重印的時間) */
	{""},
};

SQLITE_TAG_TABLE TABLE_REPRINT_TITLE_TAG[] = 
{
	{"inTableID"			,"INTEGER"	,"PRIMARY KEY"	,""},	/* Table ID Primary key, sqlite table專用避免PRIMARY KEY重複 */
	{"szSettleReprintNCCCEnable"	,"TEXT"		,""		,"DEFAULT 'N'"},
	{"szSettleReprintDCCEnable"	,"TEXT"		,""		,"DEFAULT 'N'"},
	{"szNCCCReprintTitleTime"	,"TEXT"		,""		,""},
	{"szDCCReprintTitleTime"	,"TEXT"		,""		,""},
	{"szNCCCReprintBatchNum"	,"TEXT"		,""		,""},
	{"szDCCReprintBatchNum"		,"TEXT"		,""		,""},
	{"szTMSUpdateSuccessNum"	,"TEXT"		,""		,""},
	{"szTMSUpdateFailNum"		,"TEXT"		,""		,""},
	{""},
};

extern SQLITE_TAG_TABLE TABLE_TICKET_TAG[];
extern SQLITE_TAG_TABLE TABLE_TRUST_TAG[];
/*
Function        :inBATCH_FuncUpdateTxnRecord
Date&Time       :2015/9/8 下午 16:00
Describe        :update到.bkey檔, 交易存檔檔案名稱【最多 12 Byte】 = (Host Name + Batch Number + .Bkey)
*/
int inBATCH_FuncUpdateTxnRecord(TRANSACTION_OBJECT *pobTran)
{
	unsigned char   uszBATFileName[14 + 1];			/* 檔名(含附檔名)最長可到15個字 */
        unsigned long   ulBATHandle;
	TRANS_BATCH_KEY srBKeyRec;

        if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_FuncUpdateTxnRecord()_START");
        }

	if (inFunc_ComposeFileName(pobTran, (char*)uszBATFileName, _BATCH_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
        memset(&srBKeyRec, 0x00, sizeof(srBKeyRec));

        /* Open BAT File */
        if (inFILE_Open(&ulBATHandle, uszBATFileName) == VS_ERROR)
        {
                /* 開檔失敗時，不必關檔(開檔失敗，handle回傳NULL) */
                /* 若檔案不存在時，Create檔案 */
                if (inFILE_Check_Exist(uszBATFileName) == (VS_ERROR))
                {
                        /* Create檔 */
                        if (inFILE_Create(&ulBATHandle, uszBATFileName) == (VS_ERROR))
                        {
                                /* Create檔失敗時 */
                                /* Create檔失敗時不關檔，因為create檔失敗handle回傳NULL */
                                return (VS_ERROR);
                        }
                }
                /* 檔案存在還是開檔失敗，回傳錯誤跳出 */
                else
                {
                        return (VS_ERROR);
                }

                /* create檔成功就繼續(因為create檔已經把handle指向檔案，所以不用在開檔) */
        }

        /* srBKeyRec.lnBatchRecStart儲存寫檔的起始點 */
        srBKeyRec.lnBatchRecStart = lnFILE_GetSize(&ulBATHandle, uszBATFileName);
        /* 計算要寫入的長度 */
        srBKeyRec.lnBatchRecSize = _BATCH_REC_SIZE_;

        /* 算出是否有【EMV】和【Contactless】交易 */
	if (pobTran->srBRec.inChipStatus != 0 || pobTran->srBRec.uszContactlessBit == VS_TRUE || pobTran->srBRec.uszEMVFallBackBit == VS_TRUE || pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
		srBKeyRec.lnBatchRecSize += _EMV_REC_SIZE_; /* 記憶體 -> 要存檔的 Size */

	/* 存檔 */
        /* 先seek到最後的位置，因為【.bat】永遠都是Append */
        if (inFILE_Seek(ulBATHandle, 0, _SEEK_END_) == VS_ERROR)
        {
                /* inFILE_Seek失敗時 */
                /* Seek檔失敗，所以關檔 */
                inFILE_Close(&ulBATHandle);

                /* Seek檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
                return (VS_ERROR);
        }

        if (inFILE_Write(&ulBATHandle, (unsigned char *)&pobTran->srBRec, srBKeyRec.lnBatchRecSize)== VS_ERROR)
                {
                /* 讀檔失敗 */
                /* Read檔失敗，所以關檔 */
                inFILE_Close(&ulBATHandle);

                /* Read檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
                return (VS_ERROR);
        }

        /* 關檔 */
        if (inFILE_Close(&ulBATHandle) == VS_ERROR)
        {
                /* 停1秒再關一次 */
                inDISP_Wait(1000);

                /* 若再關還是失敗，回傳VS_ERROR */
                if (inFILE_Close(&ulBATHandle) == VS_ERROR)
                        return (VS_ERROR);

        }

        /* 存TRANS_BATCH_KEY到【.bkey】檔，【.bkey】檔會存交易紀錄的記憶體位置 */
        /* 以後是透過【.bkey】檔去抓取【.bat】檔的交易紀錄 */
        /* 1. 存交易記錄的TRANS_BATCH_KEY
           2. 先取得TRANS_BATCH_KEY Index 再做存檔 */
        if (inBATCH_StoreBatchKeyFile(pobTran, &srBKeyRec) == VS_ERROR)
        {
                /* 存檔失敗 */
                /* 因為inBATCH_StoreBatchKeyFile失敗，所以回傳VS_ERROR */
                return (VS_ERROR);
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_FuncUpdateTxnRecord()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inBATCH_StoreBatchKeyFile
Date&Time       :2015/9/8 下午 16:00
Describe        :把srBKeyRec存入.bkey
*/
int inBATCH_StoreBatchKeyFile(TRANSACTION_OBJECT *pobTran, TRANS_BATCH_KEY *srBKeyRec)
{
	long            lnSearchStart, lnSearchEnd, lnSearchIndex = 0;
        long            lnOffset, lnBatchKeySize;
        long            lnRecTotalCnt = 0;		/* bkey檔案的total record number */
        long            lnBatchRecStart, lnBatchRecSize;
        long            lnRecTotalSize;			/* bkey檔案的total size */
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1]; /* debug message */
        unsigned char   uszBKEYFileName[15 + 1];      /* 檔名(含附檔名)最長可到15個字 */
        unsigned long   ulBKEYHandle;   /* File Handle */

        if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_StoreBatchKeyFile()_START");
        }

	if (inFunc_ComposeFileName(pobTran, (char*)uszBKEYFileName, _BATCH_KEY_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

        /* Open Batch Key File */
        if (inFILE_Open(&ulBKEYHandle, uszBKEYFileName) == VS_ERROR)
        {
                /* 開檔失敗時，不必關檔(開檔失敗，handle回傳NULL) */
                /* 若檔案不存在時，Create檔案 */
                if (inFILE_Check_Exist(uszBKEYFileName) == (VS_ERROR))
                {
                        if (inFILE_Create(&ulBKEYHandle, uszBKEYFileName) == VS_ERROR)
                        {
                                /* Create檔失敗時不關檔，因為create檔失敗handle回傳NULL */
                                return (VS_ERROR);
                        }
                }
                /* 檔案存在還是開檔失敗，回傳錯誤跳出 */
                else
                {
                        return (VS_ERROR);
                }

        }

        /* 從srBKeyRec->lnBatchRecStart取出記憶體起始位置 */
        lnBatchRecStart = srBKeyRec->lnBatchRecStart;
        /* 算出要存的BATCH_REC的大小 */
        lnBatchRecSize = srBKeyRec->lnBatchRecSize;
        /* 取出【.bkey】的大小 */
        lnRecTotalSize = lnFILE_GetSize(&ulBKEYHandle, uszBKEYFileName);
        /* 算出目前【.bkey】的總筆數 */
        srBKeyRec->lnSearchIndex = lnRecTotalCnt = (lnRecTotalSize / _BATCH_KEY_SIZE_);
        /* Update Invoice Number */
        srBKeyRec->lnInvoiceNo = pobTran->srBRec.lnOrgInvNum;

        /* 判斷【.bkey】是要Update還是Append */
        /* pobTran->uszUpdateBatchBit 表示是要更新記錄 */
        if (pobTran->uszUpdateBatchBit == VS_TRUE)
        {
        	lnSearchStart = 1;
	        lnSearchEnd = lnRecTotalCnt;

	        if (lnRecTotalCnt == 1)
	                lnSearchIndex = 0;
	        else
	                lnSearchIndex = (lnRecTotalCnt / 2); /* 二分法 */

		while (1)
	        {
	                lnBatchKeySize = _BATCH_KEY_SIZE_;

	                /* 表示是要 Get 最後一筆記錄 */
			if (pobTran->srBRec.lnOrgInvNum == _BATCH_LAST_RECORD_)
				lnOffset = ((lnRecTotalCnt - 1) * lnBatchKeySize);
			else
				lnOffset = lnSearchIndex * lnBatchKeySize;

                        if (inFILE_Seek(ulBKEYHandle, lnOffset, _SEEK_BEGIN_) == VS_ERROR)
                        {
                                /* inFILE_Seek失敗時 */
                                /* Seek檔失敗，所以關檔 */
                                inFILE_Close(&ulBKEYHandle);

                                /* Seek檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
                                return (VS_ERROR);
                        }

	                /* 一次讀取BATCH_KEY_SIZE的大小 */
                        if (inFILE_Read(&ulBKEYHandle, (unsigned char *)srBKeyRec, _BATCH_KEY_SIZE_) == VS_ERROR)
                        {
                                /* 讀檔失敗 */
                                /* Read檔失敗，所以關檔 */
                                inFILE_Close(&ulBKEYHandle);

                                /* Read檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
                                return (VS_ERROR);
                        }

			/* 表示是要 Get 最後一筆記錄 */
			if (pobTran->srBRec.lnOrgInvNum == _BATCH_LAST_RECORD_)
			{
				pobTran->srBRec.lnOrgInvNum = srBKeyRec->lnInvoiceNo;
				break;
			}

	                /* 判斷讀到的資料是否正確 */
	                if (pobTran->srBRec.lnOrgInvNum == srBKeyRec->lnInvoiceNo)
	                {
	                        break;
	                }
	                else
	                {
	                        if (srBKeyRec->lnInvoiceNo > pobTran->srBRec.lnOrgInvNum)
	                                lnSearchEnd = lnSearchIndex; /* 往左邊找 */
	                        else
	                                lnSearchStart = lnSearchIndex; /* 往右邊找 */
	                }

	                lnSearchIndex = (lnSearchStart + lnSearchEnd) / 2;

	                if (lnSearchIndex == 1 && lnSearchEnd == 1)
	                        lnSearchIndex = 0;

	                /* 沒有找到 Record 所以給一個假的lnOrgInvNum */
	                if (lnRecTotalCnt < 0)
	                {
	                        if (ginDebug == VS_TRUE)
	                        {
	                                inLogPrintf(AT, " BATCH_KEY SEARCH_NOT_FOUND RETURN ERR");
	                        }

				pobTran->srBRec.lnOrgInvNum = _BATCH_INVALID_RECORD_;
	                        break;
	                }

	                lnRecTotalCnt --;
	                continue;
	        }

		/* 看到錯誤在這裡修正 */
		if (srBKeyRec->lnInvoiceNo != pobTran->srBRec.lnOrgInvNum)
		{
                        inFILE_Close(&ulBKEYHandle);
			return (VS_ERROR);
		}

		/* 原交易的資料 */
		if (ginDebug == VS_TRUE)
                {
                	inLogPrintf(AT, " BATCH_KEY UPDATE");
                }

                srBKeyRec->lnBatchRecStart = lnBatchRecStart;   /* 算出 BATCH_REC 在記憶體的啟始位置 */
                srBKeyRec->lnBatchRecSize = lnBatchRecSize;     /* 算出要存 BATCH_REC 的大小 */
        }
        else
        {
                srBKeyRec->lnBatchRecStart = lnBatchRecStart;   /* 算出 BATCH_REC 在記憶體的啟始位置 */
                srBKeyRec->lnBatchRecSize = lnBatchRecSize;     /* 算出要存 BATCH_REC 的大小 */
                srBKeyRec->lnSearchIndex ++;                    /* BATCH_REC 交易的總筆數 */

                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, " BATCH_KEY APPEND");
                }
	}

	if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "InvoiceNo[BKEY : %ld]", srBKeyRec->lnInvoiceNo);
                inLogPrintf(AT, szErrorMsg);
		
		memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "[SEARCH : %ld]", pobTran->srBRec.lnOrgInvNum);
                inLogPrintf(AT, szErrorMsg);
		
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "srBKeyRec->lnBatchRecStart : [%ld]", srBKeyRec->lnBatchRecStart);
                inLogPrintf(AT, szErrorMsg);
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "srBKeyRec->lnBatchRecSize : [%ld]", srBKeyRec->lnBatchRecSize);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 要先算出 TRANS_BATCH_KEY 在記憶體的大小，因為是從【0】開始 */
	lnOffset = (srBKeyRec->lnSearchIndex - 1) * _BATCH_KEY_SIZE_;

        /* 先指到要存 TRANS_BATCH_KEY 的位置 */
        if (inFILE_Seek(ulBKEYHandle, lnOffset, _SEEK_BEGIN_) == VS_ERROR)
        {
                /* inFILE_Seek失敗時 */
                /* Seek檔失敗，所以關檔 */
                inFILE_Close(&ulBKEYHandle);

                /* Seek檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
                return (VS_ERROR);
        }

        /* 存檔到BAK */
        if (inFILE_Write(&ulBKEYHandle, (unsigned char *)srBKeyRec, _BATCH_KEY_SIZE_) == VS_ERROR)
        {
                /* 寫檔失敗時 */
                /* 關檔  */
                inFILE_Close(&ulBKEYHandle);
                /* 寫檔失敗，所以回傳error。(關檔不論成功與否都要回傳(VS_ERROR)) */
                return (VS_ERROR);
        }

        /* 關檔 */
        if (inFILE_Close(&ulBKEYHandle) == VS_ERROR)
        {
                /* 停1秒再關一次 */
                inDISP_Wait(1000);

                if (inFILE_Close(&ulBKEYHandle) == VS_ERROR)
                {
                        /* 若再關還是失敗，回傳VS_ERROR */
                        return (VS_ERROR);
                }
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_StoreBatchKeyFile()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inBATCH_SearchRecord
Date&Time       :2015/9/8 下午 16:00
Describe        :用二分法找到BAK的資料
*/
int inBATCH_SearchRecord(TRANSACTION_OBJECT *pobTran, TRANS_BATCH_KEY *srBatchKeyRec, unsigned long ulBKEYHandle, int inRecTotalCnt)
{
        long    lnSearchStart, lnSearchEnd, lnSearchIndex = 0;
        long    lnOffset, lnBatchKeySize;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_SearchRecord()_START");
        }

        lnSearchStart = 1;
        lnSearchEnd = inRecTotalCnt;

        if (inRecTotalCnt == 1)
                lnSearchIndex = 0;
        else
                lnSearchIndex = (inRecTotalCnt/2); /* 二分法 */


        while (1)
        {
                lnBatchKeySize = _BATCH_KEY_SIZE_;

                /* 表示是要 Get 最後一筆記錄 */
                if (pobTran->srBRec.lnOrgInvNum == _BATCH_LAST_RECORD_)
                {
                        lnOffset = (long) (((long) (inRecTotalCnt - 1) * lnBatchKeySize));
                }
                else
                {
                        lnOffset = lnSearchIndex * lnBatchKeySize;
                }

                /* seek .bkey */
                if (inFILE_Seek(ulBKEYHandle, lnOffset, _SEEK_BEGIN_) == VS_ERROR)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "inBATCH_SearchRecord() seek bkey error!");
                        }

                        return (VS_ERROR);
                }

                /* 一次讀取BATCH_KEY_SIZE的大小 */
                if (inFILE_Read(&ulBKEYHandle, (unsigned char *)srBatchKeyRec, _BATCH_KEY_SIZE_) == VS_ERROR)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "inBATCH_SearchRecord() read bkey error!");
                        }

                        return (VS_ERROR);
                }

                /* 表示是要 Get 最後一筆記錄 */
                if (pobTran->srBRec.lnOrgInvNum == _BATCH_LAST_RECORD_)
                {
                        pobTran->srBRec.lnOrgInvNum = srBatchKeyRec->lnInvoiceNo;
                        break;
                }

                /* 判斷讀到的資料是否正確 */
                if (pobTran->srBRec.lnOrgInvNum == srBatchKeyRec->lnInvoiceNo)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "SEARCH_BATCH_KEY_REC_OK!");
                        }

                        break;
                }
                else
                {
                        if (srBatchKeyRec->lnInvoiceNo > pobTran->srBRec.lnOrgInvNum)
                        {
                                /* 往左邊找 */
                                lnSearchEnd = lnSearchIndex;
                        }
                        else
                        {
                                /* 往右邊找 */
                                lnSearchStart = lnSearchIndex;
                        }
                }

                lnSearchIndex = (lnSearchStart + lnSearchEnd)/2;

                if (lnSearchIndex == 1 && lnSearchEnd == 1)
                        lnSearchIndex = 0;

                /* 沒有找到 Record 所以給一個假的lnOrgInvNum */
                if (inRecTotalCnt < 0)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "inBATCH_SearchRecord() ERROR!");
                        }

                        pobTran->srBRec.lnOrgInvNum = _BATCH_INVALID_RECORD_;
                        break;
                }

                inRecTotalCnt --;
        } /* end while loop */

        if (srBatchKeyRec->lnInvoiceNo != pobTran->srBRec.lnOrgInvNum)
                return (VS_ERROR);

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_SearchRecord()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}


/*
Function        :inBATCH_CheckReport
Date&Time       :2015/9/8 下午 16:00
Describe        :處理 inBATCH_GetTotalCountFromBakFile()的錯誤訊息
*/
int inBATCH_CheckReport(TRANSACTION_OBJECT *pobTran)
{
        int     inRetVal;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_CheckReport()_START");
        }

        inRetVal = inBATCH_GetTotalCountFromBakFile(pobTran);
        if (inRetVal == VS_NO_RECORD)
        {
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                /* 顯示無交易紀錄 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_RECORD_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _ENTER_KEY_MSG_;
		srDispMsgObj.inTimeout = 2;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
			
		inDISP_Msg_BMP(&srDispMsgObj);

                return (VS_ERROR);
        }
        else if (inRetVal == VS_ERROR)
        {
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                /* 顯示讀取批次檔錯誤 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_BATCH_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = 2;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);

                return (VS_ERROR);
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_CheckReport()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}


/*
Function        :inBATCH_GetTotalCountFromBakFile
Date&Time       :2015/9/8 下午 16:00
Describe        :檢查bkey有幾筆record，回傳值為筆數
*/
int inBATCH_GetTotalCountFromBakFile(TRANSACTION_OBJECT *pobTran)
{
        unsigned char   uszFileName[15 + 1];     /* 檔名(含附檔名)最長可到15個字 */
        int             inBAKTotalCnt = 0;      /* 檔案總筆數 */
        long            lnBAKTotalFileSize = 0;
        unsigned long   ulBKEYHandle;    /* File Handle */

        if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GetTotalCountFromBakFile()_START");
        }

	if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _BATCH_KEY_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

        /* ingenico 會刪除空白的檔案 */
        if (inFILE_OpenReadOnly(&ulBKEYHandle, uszFileName) == VS_ERROR)
        {
                /* 表示BAK沒有資料 */
                /* 開檔錯誤，確認是否有檔案，若有檔案仍錯誤，則可能是handle的問題 */
                if (inFILE_Check_Exist(uszFileName) != (VS_ERROR))
                {
                        /* 回傳error */
                        return (VS_ERROR);
                }
                /*如果沒檔案時，則為沒有交易紀錄，回傳NO_RECORD並印空簽單*/
                else
                {
                        return (VS_NO_RECORD);
                }
        }

        /* 算出 TRANS_BATCH_KEY 總合 */
        lnBAKTotalFileSize = lnFILE_GetSize(&ulBKEYHandle, uszFileName);
        /* 算出交易總筆數，因為lnFILE_GetSize回傳值為long，因為此函式回傳int所以強制轉型，因為筆數不會超過int大小 */
        inBAKTotalCnt = (int)(lnBAKTotalFileSize / _BATCH_KEY_SIZE_);

        /* 關檔 */
        if (inFILE_Close(&ulBKEYHandle) == VS_ERROR)
        {
                /* 沒關掉，再關一次 */
                if (inFILE_Close(&ulBKEYHandle) == VS_ERROR)
                {
                        return (VS_ERROR);
                }
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GetTotalCountFromBakFile()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (inBAKTotalCnt);
}

/*
Function        :inBATCH_OpenBatchKeyFile
Date&Time       :2015/9/8 下午 16:00
Describe        :開bak檔案(ReadOnly)，假如bak沒有資料會回傳VS_NO_RECORD，有值的話會回傳handle
*/
int inBATCH_OpenBatchKeyFile(TRANSACTION_OBJECT *pobTran, unsigned long ulBKEYHandle)
{
        unsigned char   uszFileName[15 + 1];     /* 檔名(含附檔名)最長可到15個字 */

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_OpenBatchKeyFile()_START");
        }

        if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _BATCH_KEY_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

        /* ingenico會刪除空白的檔案 */
        /* open bkey file */
        if (inFILE_OpenReadOnly(&ulBKEYHandle, uszFileName) == VS_ERROR)
        {
                /* 表示bkey沒有資料 */
                return (VS_NO_RECORD);
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_OpenBatchKeyFile()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return(VS_SUCCESS);
}


/*
Function        :inBATCH_OpenBatchRecFile
Date&Time       :2015/9/8 下午 16:00
Describe        :開bat檔案(ReadOnly)，有值的話會回傳handle
*/
int inBATCH_OpenBatchRecFile(TRANSACTION_OBJECT *pobTran, unsigned long ulBATHandle)
{
        unsigned char   uszFileName[15 + 1];     /* 檔名(含附檔名)最長可到15個字 */

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_OpenBatchRecFile()_START");
        }

	if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _BATCH_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}


        /* ingenico會刪除空白的檔案 */
        /* open bat file */
        if (inFILE_OpenReadOnly(&ulBATHandle, uszFileName) == VS_ERROR)
        {
                return (VS_ERROR);
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_OpenBatchRecFile()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}


/*
Function        :inBATCH_ReviewReport_Detail
Date&Time       :2015/9/8 下午 16:00
Describe        :交易查詢使用
*/
int inBATCH_ReviewReport_Detail(TRANSACTION_OBJECT *pobTran)
{
	int		i = 0;
	int		inFileRecCnt = 0;
	int		inReviewReportResult = VS_TRUE;
	int		inTouchPage = 0;
	char		szTemplate1[42 + 1];
	char		szTemplate2[42 + 1];
	char		szFinalPAN[_PAN_UCARD_SIZE_ + 1];
	char		szDispMsg [50 + 1];
	char		szDebugMsg[100 + 1];
	char		szKey = 0x00;
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBatch_ReviewReport()_START");
        }

	/* 算出交易總筆數 */
	inFileRecCnt = inBATCH_GetTotalCountFromBakFile(pobTran);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "筆數: %d", inFileRecCnt);
		inLogPrintf(AT, szDebugMsg);
	}

	i = 0;
	while (i < inFileRecCnt)
        {
                /* 開始讀取每一筆交易記錄 */
                if (inBATCH_GetDetailRecords(pobTran, i) != VS_SUCCESS)
                {
                        inReviewReportResult = VS_FALSE;
                        break;
                }
		
		/* DCC轉台幣的紀錄要特別處理，原先的那筆DCC一般交易不能出現 */
		if (pobTran->srBRec.inCode == _VOID_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)
		{
			if (inTouchPage == _DisTouch_Slide_Right_To_Left_ || szKey == _KEY_ENTER_ || szKey == _KEY_DOWN_ || szKey == _KEY_DOWN_HALF_)
			{
				i++;
				continue;
			}
			else if (inTouchPage == _DisTouch_Slide_Left_To_Right_ || szKey == _KEY_UP_ || szKey == _KEY_UP_HALF_)
			{
				i--;
				
				/* 交易紀錄編號不可小於0 */
				if (i < 0)
				{
					i = 0;
					/* 加這行是因為，若第一筆是DCC轉台幣會陷入無限迴圈，所以改滑動方向 */
					inTouchPage = _DisTouch_Slide_Right_To_Left_;
					szKey = _KEY_ENTER_;
				}
				continue;
			}
			/* 若第一筆就是要跳過的紀錄，會因為沒有event，而沒有跳過，所以預設為下一頁 */
			else
			{
				i++;
				continue;
			}
			
		}
		
                inDISP_ClearAll();

                if (inReviewReportResult == VS_FALSE)
                        break;

                /* Disp Card Label & Expire date*/
                memset(szDispMsg, 0x00, sizeof(szDispMsg));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetTRTFileName(szTemplate1);
		/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
		/* 金融卡沒有效期 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			sprintf(szDispMsg, " %s", pobTran->srBRec.szCardLabel);
		}
		/* 其他加密 */
		else
		{
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
			if (strlen(pobTran->srBRec.szCheckNO) > 0)
			{
				strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
			}
			else
			{
				inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
			}
			
			/* 【需求單 - 106349】自有品牌判斷需求 */
			/* unpack電文就存存CardLabel modify by LingHsiung 2020/2/14 上午 11:07 */
			sprintf(szDispMsg, " %s   %s", pobTran->srBRec.szCardLabel, szTemplate1);
		}
		
                inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_1_, _DISP_LEFT_);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "CardLabel: %s", pobTran->srBRec.szCardLabel);
			inLogPrintf(AT, szDebugMsg);
		}

                /* Disp Card Number */
                memset(szDispMsg, 0x00, sizeof(szDispMsg));
		memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
		strcpy(szFinalPAN, pobTran->srBRec.szPAN);

		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		sprintf(szDispMsg, " %s",szFinalPAN);
		inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "szPAN: %s", pobTran->srBRec.szPAN);
			inLogPrintf(AT, szDebugMsg);
		}

                /* Transaction Type */
                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                memset(szTemplate2, 0x00, sizeof(szTemplate2));
                inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
		if (strlen(szTemplate2) > 0)
		{
			sprintf(szDispMsg, " %s", szTemplate1);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_5_, _DISP_LEFT_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, " %s", szTemplate2);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_6_, _DISP_LEFT_);
		}
		else
		{
			sprintf(szDispMsg, " %s %s", szTemplate1, szTemplate2);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
		}
			
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Tran Type: %s %s", szTemplate1, szTemplate2);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 授權碼(金卡顯示調單編號) */
                memset(szDispMsg, 0x00, sizeof(szDispMsg));
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			sprintf(szDispMsg, " RRN NO.: %s", pobTran->srBRec.szFiscRRN);
		}
		else
		{
			sprintf(szDispMsg, " APPR: %s", pobTran->srBRec.szAuthCode);
		}
		inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_7_, _DISP_LEFT_);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "%s", szDispMsg);
			inLogPrintf(AT, szDebugMsg);
		}
		
                /* Reference Number */
                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                sprintf(szDispMsg, " REF: %s", pobTran->srBRec.szRefNo);
		inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_8_, _DISP_LEFT_);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "%s", szDispMsg);
			inLogPrintf(AT, szDebugMsg);
		}

                /* Invoice Number */
                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                sprintf(szDispMsg, " INV: %06ld", pobTran->srBRec.lnOrgInvNum);
		inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "%06ld", pobTran->srBRec.lnOrgInvNum);
			inLogPrintf(AT, szDebugMsg);
		}

                /* Amount */
                memset(szDispMsg, 0x00, sizeof(szDispMsg));

                if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
                {
                        switch (pobTran->srBRec.inCode)
                        {
                                case _SALE_ :
				case _INST_SALE_ :
				case _REDEEM_SALE_ :
				case _SALE_OFFLINE_ :
				case _FORCE_CASH_ADVANCE_ :
				case _PRE_COMP_:
                                case _PRE_AUTH_ :
				case _CUP_SALE_ :
				case _CUP_PRE_COMP_ :
				case _CUP_PRE_AUTH_ :
				case _INST_ADJUST_:
				case _REDEEM_ADJUST_:
				case _MAIL_ORDER_ :
				case _CUP_MAIL_ORDER_ :
				case _CASH_ADVANCE_ :
				case _FISC_SALE_ :
                                        if (pobTran->srBRec.lnTipTxnAmount != 0)
                                                sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
                                        else
                                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
					inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
                                        sprintf(szDispMsg, " %s", szTemplate1);
                                        break;
				case _ADJUST_:
					sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
					inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
                                        sprintf(szDispMsg, " %s", szTemplate1);
                                        break;
				case _TIP_ :
                                        sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
					inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
                                        sprintf(szDispMsg, " %s", szTemplate1);
                                        break;
                                case _REFUND_ :
				case _INST_REFUND_ :
				case _REDEEM_REFUND_ :
				case _CUP_REFUND_ :
				case _CUP_MAIL_ORDER_REFUND_ :
				case _FISC_REFUND_ :
                                        sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
					inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
                                        sprintf(szDispMsg, " %s", szTemplate1);
                                        break;
                                default :
                                        sprintf(szDispMsg, " AMT_ERR_(%d)", pobTran->srBRec.inCode);
                                        break;
                        } /* End switch () */
                }
                else
                {
                        switch (pobTran->srBRec.inOrgCode)
                        {
				case _SALE_ :
				case _INST_SALE_ :
				case _REDEEM_SALE_ :
				case _SALE_OFFLINE_ :
				case _FORCE_CASH_ADVANCE_ :
				case _PRE_COMP_:
                                case _PRE_AUTH_ :
				case _CUP_SALE_ :
				case _CUP_PRE_COMP_ :
				case _CUP_PRE_AUTH_ :
				case _INST_ADJUST_:
				case _REDEEM_ADJUST_:
				case _MAIL_ORDER_ :
				case _CUP_MAIL_ORDER_ :
				case _CASH_ADVANCE_ :
				case _FISC_SALE_ :
                                        sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
					inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
                                        sprintf(szDispMsg, " %s", szTemplate1);
                                        break;
				case _ADJUST_:
					sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
					inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
                                        sprintf(szDispMsg, " %s", szTemplate1);
                                        break;
				/* 目前小費沒有取消 */
                                case _REFUND_ :
				case _INST_REFUND_ :
				case _REDEEM_REFUND_ :
				case _CUP_REFUND_ :
				case _CUP_MAIL_ORDER_REFUND_ :
				/* SmartPay不能取消 */
                                        sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
					inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
                                        sprintf(szDispMsg, " %s", szTemplate1);
                                        break;
                                default :
                                        sprintf(szDispMsg, " AMT_VOID_ERR_(%d)", pobTran->srBRec.inCode);
                                        break;
                        } /* End switch () */
                }
		inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "%s", szDispMsg);
			inLogPrintf(AT, szDebugMsg);
		}

                /* Store ID */
                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                memset(szDispMsg, 0x00, sizeof(szDispMsg));

                inGetStoreIDEnable(&szTemplate1[0]);

                if ( (szTemplate1[0] == 'Y') && (strlen(pobTran->srBRec.szStoreID) > 0))
                {
                        if (strlen(pobTran->srBRec.szStoreID) > 42)
                                memcpy(&szTemplate1[0], &pobTran->srBRec.szStoreID[0], 42);
                        else
                                strcpy(szTemplate1, pobTran->srBRec.szStoreID);

                        szTemplate1[18] = 0x00;

			sprintf(szDispMsg, " STD:%s", pobTran->srBRec.szStoreID);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
                }
		else
		{
			/* 沒開櫃號功能， 不印櫃號 */
		}
		
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
                sprintf(szDispMsg, "%d / %d", i+1, inFileRecCnt);
		inDISP_ChineseFont_Point_Color(szDispMsg, _FONTSIZE_8X22_, _LINE_8_8_, _COLOR_BLACK_, _COLOR_LIGHTBLUE_, 10);
		
		/* 事件初始化 */
		szKey = 0;
		/* 設定Timeout */
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		
		while (1)
		{
			
			/* 偵測滑動 */
			inTouchPage = inDisTouch_TouchSensor_Click_Slide(_Touch_NEWUI_REVIEW_BATCH_);
			if (inTouchPage == _DisTouch_Slide_Right_To_Left_)
			{
				++i;
				break;
			}
			if (inTouchPage == _DisTouch_Slide_Left_To_Right_)
			{
				--i;
				
				/* 交易紀錄編號不可小於0 */
				if (i < 0)
				{
					i = 0;
				}
				break;
			}
			
			/* 偵測按鍵 */
			szKey = uszKBD_Key();
			
			/* TimeOut */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}
			
			if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
			{
				inReviewReportResult = VS_FALSE;
				break;
			}
			else if (szKey == _KEY_ENTER_ || szKey == _KEY_DOWN_ || szKey == _KEY_DOWN_HALF_)
			{
				++i;
				break;
			}
			else if (szKey == _KEY_UP_ || szKey == _KEY_UP_HALF_ || szKey == _KEY_FUNCTION_)
			{
				--i;
				/* 交易紀錄編號不可小於0 */
				if (i < 0)
				{
					i = 0;
				}
				break;
			}

		}
		
        } /* End for () .... */

	/* 顯示批次結束 */
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第一層顯示 <交易查詢> */
	DISPLAY_OBJECT	srDispMsgObj;
	memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
	strcpy(srDispMsgObj.szDispPic1Name, _ERR_BATCH_END_);
	srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
	srDispMsgObj.inMsgType = _ENTER_KEY_MSG_;
	srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
	strcpy(srDispMsgObj.szErrMsg1, "");
	srDispMsgObj.inErrMsg1Line = 0;
	srDispMsgObj.inBeepTimes = 1;
	srDispMsgObj.inBeepInterval = 0;
			
	inDISP_Msg_BMP(&srDispMsgObj);

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBatch_ReviewReport()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}


/*
Function        :inBATCH_GetDetailRecords
Date&Time       :2015/9/8 下午 16:00
Describe        :這邊的抓法是根據Bkey內的紀錄，但因為同步InvNum的關係，第一筆不一定是InvNum = 1
*/
int inBATCH_GetDetailRecords(TRANSACTION_OBJECT *pobTran, int inStartCnt)
{
	int		i;
        long            lnOffset, lnBKEYSize;
	long            lnReadLength;					/* 記錄剩餘要從Brec讀多長的資料 */
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];		/* debug message */
        unsigned char   uszBATFileName[16 + 1], uszBKEYFileName[16 + 1];
        unsigned long   ulBKEYHandle;					/* bkey Handle */
        unsigned long   ulBATHandle;					/* BAT Handle */
        TRANS_BATCH_KEY srBKEYRec;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GetDetailRecords()_START");
        }

	if (inFunc_ComposeFileName(pobTran, (char*)uszBATFileName, _BATCH_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	if (inFunc_ComposeFileName(pobTran, (char*)uszBKEYFileName, _BATCH_KEY_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

        memset((char *)&srBKEYRec, 0x00, sizeof(srBKEYRec));

        /* 【STEP 1】先找 TRANS_BATCH_KEY 裡的 lnInvoiceNo */
        lnBKEYSize = _BATCH_KEY_SIZE_;
        lnOffset = (long) (((long) inStartCnt * lnBKEYSize)); /* 第幾筆 */
	
        if (inFILE_OpenReadOnly(&ulBKEYHandle, uszBKEYFileName) == VS_ERROR)
        {
                return (VS_ERROR);
        }
        else
        {
                if (inFILE_Seek(ulBKEYHandle, lnOffset, _SEEK_BEGIN_) == VS_ERROR)
                {
                        inFILE_Close(&ulBKEYHandle);
			
                        return (VS_ERROR);
                }
                else
                {
			 if (ginDebug == VS_TRUE)
			{
				 memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				 sprintf(szErrorMsg, "%lu,%lu", lnBKEYSize, lnFILE_GetSize(&ulBKEYHandle, uszBKEYFileName));
				 inLogPrintf(AT, szErrorMsg);
			}

                        if (inFILE_Read(&ulBKEYHandle, (unsigned char *)&srBKEYRec, sizeof(srBKEYRec)) == VS_ERROR)
                        {
                                inFILE_Close(&ulBKEYHandle);
				
                                return (VS_ERROR);
                        }
                }
        }

        if (inFILE_Close(&ulBKEYHandle) == VS_ERROR)
        {
                return (VS_ERROR);
        }

        /* 【STEP 2】開始搜尋相對應的 BATCH_REC 交易記錄 */
        lnOffset = srBKEYRec.lnBatchRecStart;

        if (inFILE_OpenReadOnly(&ulBATHandle, uszBATFileName) == VS_ERROR)
        {
                return (VS_ERROR);
        }

        if (inFILE_Seek(ulBATHandle, lnOffset, _SEEK_BEGIN_) == VS_ERROR)
        {
                inFILE_Close(&ulBATHandle);
		
                return (VS_ERROR);
        }
	
	lnReadLength = srBKEYRec.lnBatchRecSize;
	
	/* 要從0開始算，所以要用i++，而不是++i */
	for (i = 0;; i++)
	{
		/* 剩餘長度大於或等於inMaxsize */
		if (lnReadLength >= 1024)
		{
			/* read bat，讀的數量為srBKRec.lnBatchRecSize，因為bat每一筆資料長度是浮動的 */
			if (inFILE_Read(&ulBATHandle, (unsigned char *)&pobTran->srBRec + (1024 * i), 1024) == VS_ERROR)
			{
				inFILE_Close(&ulBATHandle);
				
				return (VS_ERROR);
			}
			else
			{
				/* 一次讀1024 */
				lnReadLength -= 1024;

				/* 當剩餘長度剛好為1024，會剛好讀完 */
				if (lnReadLength == 0)
					break;
			}

		}
		/* 剩餘長度小於1024 */
		else if (lnReadLength < 1024)
		{
			/* 就只讀剩餘長度 */
			if (inFILE_Read(&ulBATHandle, (unsigned char *)&pobTran->srBRec + (1024 * i), lnReadLength) == VS_ERROR)
			{
				/* Close檔案 */
				inFILE_Close(&ulBATHandle);
				
				return (VS_ERROR);
			}
			else
			{
				lnReadLength -= lnReadLength;
				
				if (lnReadLength == 0)
					break;
			}
		}
	}/* end for loop */

        /* 檢查讀出來的srBRec是否為正確的 */
        if (pobTran->srBRec.lnOrgInvNum != srBKEYRec.lnInvoiceNo)
        {
		if (ginDebug == VS_TRUE)
		{
			 memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
			 sprintf(szErrorMsg, "OrgInv:%lu, BkeyInvoiceNo:%lu", pobTran->srBRec.lnOrgInvNum, srBKEYRec.lnInvoiceNo);
			 inLogPrintf(AT, szErrorMsg);
		}
		
                inFILE_Close(&ulBATHandle);
		
                return (VS_ERROR);
        }

        if (inFILE_Close(&ulBATHandle) == VS_ERROR)
        {
                return (VS_ERROR);
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GetDetailRecords()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inBATCH_FuncUserChoice
Date&Time       :2015/9/8 下午 16:00
Describe        :
*/
int inBATCH_FuncUserChoice(TRANSACTION_OBJECT *pobTran)
{
        int	inRetVal = VS_ERROR;
	int	inDCCIndex = -1;
	int	inNCCCIndex = -1;
	int	inOrgIndex = -1;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_FuncUserChoice()_START");
        }
	
	/* NCCC DCC的特殊處理，當端末機收到收銀機要取消DCC HOST交易的電文時，
	 * 如果在DCC的帳務中查不到這一筆交易(表示交易改為台幣支付)端末機就自動在去找NCCC的帳務來解決銀行別錯誤而取消不到的問題。 START
	 */
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_DCC_, &inDCCIndex);
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inNCCCIndex);
	/* NCCC DCC的特殊處理，當端末機收到收銀機要取消DCC HOST交易的電文時，
	 * 如果在DCC的帳務中查不到這一筆交易(表示交易改為台幣支付)端末機就自動在去找NCCC的帳務來解決銀行別錯誤而取消不到的問題。 END
	 */
	
        while (1)
        {
                /* 如果lnOrgInvNum有值，就不需要做輸入的動作(ECR發動) */
		if (pobTran->uszECRBit == VS_TRUE)
		{
			if (pobTran->srBRec.lnOrgInvNum <= 0)
			{
				return (VS_ERROR);
			}
		}
		else
		{
			if (pobTran->srBRec.lnOrgInvNum <= 0)
			{
				if (inBATCH_GetInvoiceNumber(pobTran) == VS_ERROR)
				{
					return (VS_ERROR);
				}

				/* 因為EDC如果按確認鍵，等於調出上一筆記錄 */
				if (pobTran->srBRec.lnOrgInvNum == 0)
					pobTran->srBRec.lnOrgInvNum = _BATCH_LAST_RECORD_; /* 表示是最後一筆 */
			}
		}

                /* Load Batch Record */
                inRetVal = inBATCH_GetTransRecord(pobTran);

		if (inRetVal == VS_SUCCESS)
		{
			/* NCCC DCC的特殊處理，當端末機收到收銀機要取消DCC HOST交易的電文時，
			 * 如果在DCC的帳務中查不到這一筆交易(表示交易改為台幣支付)端末機就自動在去找NCCC的帳務來解決銀行別錯誤而取消不到的問題。 START
			 */
			if (pobTran->srBRec.inHDTIndex == inDCCIndex && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)
			{
				/* 這個情形只有在用POS機時才會出現 */
				if (pobTran->uszECRBit == VS_TRUE)
				{
					inOrgIndex = pobTran->srBRec.inHDTIndex;
					pobTran->srBRec.inHDTIndex = inNCCCIndex;
					/* Load Batch Record */
					inRetVal = inBATCH_GetTransRecord(pobTran);

					if (inRetVal == VS_SUCCESS)
					{
						break;
					}
					else if (inRetVal == VS_NO_RECORD)
					{
						/* 找不到，還原index並顯示無交易紀錄*/
						pobTran->srBRec.inHDTIndex = inOrgIndex;

						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						/* 顯示無交易紀錄 */
						inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

						inDISP_BEEP(3, 500);
						pobTran->srBRec.lnOrgInvNum = -1;

						continue;
					}
					else
					{
						return (VS_ERROR);
					}
					/* NCCC DCC的特殊處理，當端末機收到收銀機要取消DCC HOST交易的電文時，
					* 如果在DCC的帳務中查不到這一筆交易(表示交易改為台幣支付)端末機就自動在去找NCCC的帳務來解決銀行別錯誤而取消不到的問題。 END
					*/
				}
				else
				{
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					/* 顯示無交易紀錄 */
					inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);
					
					inDISP_BEEP(3, 500);
					pobTran->srBRec.lnOrgInvNum = -1;

					continue;
				}
				
			}
			/* 找到紀錄 */
			else
			{
				break;
			}
			
		}
		/* 找不到 */
		else if (inRetVal == VS_NO_RECORD)
                {
                        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			/* 顯示無交易紀錄 */
			inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

			inDISP_BEEP(3, 500);
			pobTran->srBRec.lnOrgInvNum = -1;

                        continue;
                }
                else
                {
                        return (VS_ERROR);
                }

        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_FuncUserChoice()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}


/*
Function        :inBATCH_GetInvoiceNumber
Date&Time       :2015/9/8 下午 16:00
Describe        :輸入調閱編號
*/
int inBATCH_GetInvoiceNumber(TRANSACTION_OBJECT *pobTran)
{
        int             inRetVal = VS_ERROR, i = 0;
        char            szTemplate[_DISP_MSG_SIZE_ + 1];
        DISPLAY_OBJECT  srDispObj;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GetInvoiceNumber()_START");
        }

        /* 初始化 */
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        while (1)
        {
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                inDISP_PutGraphic(_GET_INVOICE_NUM_, 0, _COORDINATE_Y_LINE_8_4_);
                
                srDispObj.inY = _LINE_8_6_;
                srDispObj.inR_L = _DISP_LEFT_;
                srDispObj.inMaxLen = 3;
                srDispObj.inMask = VS_FALSE;
		srDispObj.inColor = _COLOR_RED_;
		srDispObj.inTouchSensorFunc = _Touch_CUP_LOGON_;
                strcpy(srDispObj.szPromptMsg, "REF#:");

                strcpy(szTemplate, "REF#:");
                inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _COLOR_RED_, _DISP_LEFT_);
		
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
                inRetVal = inDISP_Enter8x16_Character_Mask_And_DisTouch(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                {
                        return (VS_ERROR);
                }
                else if (inRetVal == 0)
                {
                        pobTran->srBRec.lnOrgInvNum = _BATCH_LAST_RECORD_;
                        break;
                }
                else if (inRetVal > 0)
                {
                        for (i = 0; i < inRetVal; i ++)
                        {
                                if ((srDispObj.szOutput[i] >= '0') && (srDispObj.szOutput[i] <= '9'))
                                        continue;
                                else
                                {
                                        memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
                                        break;
                                }
                        }

                        if (strlen(srDispObj.szOutput) == 0)
                                continue;

                        pobTran->srBRec.lnOrgInvNum = atol(srDispObj.szOutput);

                        if (pobTran->srBRec.lnOrgInvNum == 0)
                        {
                                if (ginDebug == VS_TRUE)
                                {
                                        inLogPrintf(AT, "inBATCH_GetInvoiceNumber No OrgInvNum");
                                }

                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                /* 顯示無交易紀錄 */
                                inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);
                                inDISP_Wait(3000);

                                continue;
                        }
                }

                break;
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GetInvoiceNumber()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}


/*
Function        :inBATCH_GetTransRecord
Date&Time       :2015/9/8 下午 16:00
Describe        :用inBATCH_SearchRecord找到想要的bkey記錄，再找bkey對應的bat記錄
*/
int inBATCH_GetTransRecord(TRANSACTION_OBJECT *pobTran)
{
        long            lnBAKFileSize;
	long		lnReadLength;
        int             inRetVal, inBAKTotalCnt,i;
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1]; /* debug message */
	unsigned char   uszBATFileName[15 + 1], uszBKEYFileName[14 + 1];        /* 檔名(含附檔名)最長可到15個字 */
        unsigned long   ulBKEYHandle;
        unsigned long   ulBATHandle;
        TRANS_BATCH_KEY srBKRec;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GetTransRecord()_START");
        }

	memset(uszBKEYFileName, 0x00, sizeof(uszBKEYFileName));
	memset(uszBATFileName, 0x00, sizeof(uszBATFileName));
	
        if (inFunc_ComposeFileName(pobTran, (char*)uszBKEYFileName, _BATCH_KEY_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	if (inFunc_ComposeFileName(pobTran, (char*)uszBATFileName, _BATCH_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

        /* ingenico會刪除空白的檔案 */
        /* open bkey file */
        if (inFILE_OpenReadOnly(&ulBKEYHandle, uszBKEYFileName) == VS_ERROR)
        {
               /* 表示bkey沒有資料 */
               return (VS_NO_RECORD);
        }

        /* 檢查bkey總長度 */
        lnBAKFileSize = lnFILE_GetSize(&ulBKEYHandle, uszBKEYFileName);

        if (lnBAKFileSize > 0)
        {
                inBAKTotalCnt = (int)(lnBAKFileSize/ _BATCH_KEY_SIZE_);
        }
        else
        {
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inBATCH_GetTransRecord lnBAKFileSize = %ld", lnBAKFileSize);
                        inLogPrintf(AT, szErrorMsg);
                }

                inFILE_Close(&ulBKEYHandle);

                return (VS_ERROR);
        }

        /* 找到符合pobTran->srBRec.lnOrgInvNum的bkey記錄  */
        memset(&srBKRec, 0x00, sizeof(srBKRec));
        if (inBATCH_SearchRecord(pobTran, &srBKRec, ulBKEYHandle, inBAKTotalCnt) == VS_ERROR)
        {
                inFILE_Close(&ulBKEYHandle);
                return (VS_NO_RECORD);
        }

        /* ingenico會刪除空白的檔案 */
        /* open bat file */
        if (inFILE_OpenReadOnly(&ulBATHandle, uszBATFileName) == VS_ERROR)
        {
               /* 表示BAT沒有資料 */
               inFILE_Close(&ulBKEYHandle);
               return (VS_ERROR);
        }

        /* 開始搜尋相對應的 BATCH_REC 交易記錄 */
        if (inFILE_Seek(ulBATHandle, srBKRec.lnBatchRecStart, _SEEK_BEGIN_) == VS_ERROR)
        {
                inFILE_Close(&ulBKEYHandle);
                inFILE_Close(&ulBATHandle);
                return (VS_ERROR);
        }

        lnReadLength = srBKRec.lnBatchRecSize;
		
	for (i = 0;; ++i)
	{
		/* 剩餘長度大於或等於inMaxsize */
		if (lnReadLength >= 1024)
		{
			/* read bat，讀的數量為srBKRec.lnBatchRecSize，因為bat每一筆資料長度是浮動的 */
			if ((inRetVal = inFILE_Read(&ulBATHandle, (unsigned char *)&pobTran->srBRec + (1024 * i), 1024)) == VS_ERROR)
			{
				inFILE_Close(&ulBATHandle);
				
				return (VS_ERROR);
			}
			else
			{
				/* 一次讀1024 */
				lnReadLength -= 1024;

				/* 當剩餘長度剛好為1024，會剛好讀完 */
				if (lnReadLength == 0)
					break;
			}

		}
		/* 剩餘長度小於1024 */
		else if (lnReadLength < 1024)
		{
			/* 就只讀剩餘長度 */
			if ((inRetVal = inFILE_Read(&ulBATHandle, (unsigned char *)&pobTran->srBRec + (1024 * i), lnReadLength)) == VS_ERROR)
			{
				/* Close檔案 */
				inFILE_Close(&ulBATHandle);
				
				return (VS_ERROR);
			}
			else
			{
				lnReadLength -= lnReadLength;
				
				if (lnReadLength == 0)
					break;
			}
		}
	}/* end for loop */
	
        if (inRetVal == VS_ERROR || pobTran->srBRec.lnOrgInvNum != srBKRec.lnInvoiceNo)
        {
                inFILE_Close(&ulBKEYHandle);
                inFILE_Close(&ulBATHandle);
                return (VS_ERROR);
        }

        inFILE_Close(&ulBKEYHandle);
        inFILE_Close(&ulBATHandle);

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GetTransRecord()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inBATCH_GetAdviceDetailRecord
Date&Time       :
Describe        :
*/
int inBATCH_GetAdviceDetailRecord(TRANSACTION_OBJECT *pobTran, int inADVCnt)
{
	int		i;
        int             inRetVal;
        long	        lnBKEYSize, lnBKEYTotalCnt, lnOffset;
        long            lnSearchStart, lnSearchEnd, lnSearchIndex = 0;
	long		lnReadLength = 0;
        unsigned char   uszGetAdviceInvoiceNum[6 + 1];
        TRANS_BATCH_KEY srBKEYRec;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GetAdviceDetailRecord()_START");
        }

        /* 讀【.adv】檔 */
        if (inFILE_Seek(gulADVHandle, 0, _SEEK_BEGIN_) == VS_ERROR)
        {
                return (VS_ERROR);
        }
        else
        {
                memset(uszGetAdviceInvoiceNum, 0x00, sizeof(uszGetAdviceInvoiceNum));
                if ((inRetVal = inFILE_Read(&gulADVHandle, &uszGetAdviceInvoiceNum[0], _ADV_FIELD_SIZE_)) == VS_ERROR)
                {
                        return (VS_ERROR);
                }
                else
                {
                        pobTran->srBRec.lnOrgInvNum = atol((char *)uszGetAdviceInvoiceNum);
                }
        }

        /* 讀【.bkey】檔，用Invoice Number比對後，取得Batch在記憶體的位置 */
        lnBKEYTotalCnt = lnFILE_GetSize(&gulBKEYHandle, guszBKEYFileName);
        lnBKEYTotalCnt = (lnBKEYTotalCnt / _BATCH_KEY_SIZE_);

        lnSearchStart = 1;
        lnSearchEnd = lnBKEYTotalCnt;

        if (lnBKEYTotalCnt == 1)
                lnSearchIndex = 0;
        else
                lnSearchIndex = (lnBKEYTotalCnt / 2); /* 二分法 */

        while (1)
        {
                lnBKEYSize = _BATCH_KEY_SIZE_;
                lnOffset = lnSearchIndex * lnBKEYSize;

                if (inFILE_Seek(gulBKEYHandle, lnOffset, _SEEK_BEGIN_) == VS_ERROR)
                {
                        return (VS_ERROR);
                }
                else
                {
			lnReadLength = lnBKEYSize;
			/* 要從0開始算，所以要用i++，而不是++i */
			for (i = 0;; i++)
			{
				/* 剩餘長度大於或等於inMaxsize */
				if (lnReadLength >= 1024)
				{
					/* read bat，讀的數量為srBKRec.lnBatchRecSize，因為bat每一筆資料長度是浮動的 */
					if (inFILE_Read(&gulBKEYHandle, (unsigned char *)&srBKEYRec + (1024 * i), 1024) == VS_ERROR)
					{
						inFILE_Close(&gulBKEYHandle);

						return (VS_ERROR);
					}
					else
					{
						/* 一次讀1024 */
						lnReadLength -= 1024;

						/* 當剩餘長度剛好為1024，會剛好讀完 */
						if (lnReadLength == 0)
							break;
					}

				}
				/* 剩餘長度小於1024 */
				else if (lnReadLength < 1024)
				{
					/* 就只讀剩餘長度 */
					if (inFILE_Read(&gulBKEYHandle, (unsigned char *)&srBKEYRec + (1024 * i), lnReadLength) == VS_ERROR)
					{
						/* Close檔案 */
						inFILE_Close(&gulBKEYHandle);

						return (VS_ERROR);
					}
					else
					{
						lnReadLength -= lnBKEYSize;

						if (lnReadLength == 0)
							break;
					}
				}
			}/* end for loop */
                       
                }

                /* 判斷讀到的資料是否正確 */
                if (pobTran->srBRec.lnOrgInvNum == srBKEYRec.lnInvoiceNo)
                {
                        break;
                }
                else
                {
                        if (srBKEYRec.lnInvoiceNo > pobTran->srBRec.lnOrgInvNum)
                                lnSearchEnd = lnSearchIndex; /* 往左邊找 */
                        else
                                lnSearchStart = lnSearchIndex; /* 往右邊找 */
                }

                lnSearchIndex = (lnSearchStart + lnSearchEnd) / 2;

                if (lnSearchIndex == 1 && lnSearchEnd == 1)
                        lnSearchIndex = 0;

                /* 沒有找到TRANS_BATCH_KEY */
                if (lnBKEYTotalCnt < 0)
                {
                        return (VS_ERROR);
                }

                lnBKEYTotalCnt --;
                continue;
        }

        /* 讀【.bat】檔BATCH_REC 交易記錄 */
        lnOffset = srBKEYRec.lnBatchRecStart;

        if (inFILE_Seek(gulBATHandle, srBKEYRec.lnBatchRecStart, _SEEK_BEGIN_) == VS_ERROR)
        {
                return (VS_ERROR);
        }
        else
        {
		lnReadLength = srBKEYRec.lnBatchRecSize;
                /* 讀的數量為srBKRec.lnBatchRecSize，因為【.bat】每一筆資料長度是浮動的 */

		/* 要從0開始算，所以要用i++，而不是++i */
		for (i = 0;; i++)
		{
			/* 剩餘長度大於或等於inMaxsize */
			if (lnReadLength >= 1024)
			{
				/* read bat，讀的數量為srBKRec.lnBatchRecSize，因為bat每一筆資料長度是浮動的 */
				if (inFILE_Read(&gulBATHandle, (unsigned char *)&pobTran->srBRec + (1024 * i), 1024) == VS_ERROR)
				{
					inFILE_Close(&gulBATHandle);

					return (VS_ERROR);
				}
				else
				{
					/* 一次讀1024 */
					lnReadLength -= 1024;

					/* 當剩餘長度剛好為1024，會剛好讀完 */
					if (lnReadLength == 0)
						break;
				}

			}
			/* 剩餘長度小於1024 */
			else if (lnReadLength < 1024)
			{
				/* 就只讀剩餘長度 */
				if (inFILE_Read(&gulBATHandle, (unsigned char *)&pobTran->srBRec + (1024 * i), lnReadLength) == VS_ERROR)
				{
					/* Close檔案 */
					inFILE_Close(&gulBATHandle);

					return (VS_ERROR);
				}
				else
				{
					lnReadLength -= lnReadLength;

					if (lnReadLength == 0)
						break;
				}
			}
		}/* end for loop */
		
        }

        /* 檢查讀出來的srBRec是否為正確的 */
        if (pobTran->srBRec.lnOrgInvNum != srBKEYRec.lnInvoiceNo)
        {
                return (VS_ERROR);
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GetAdviceDetailRecord()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inBATCH_ESC_Save_Advice
Date&Time       :2016/12/19 下午 3:57
Describe        :存ESC advice
*/
int inBATCH_ESC_Save_Advice(TRANSACTION_OBJECT *pobTran)
{
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inBATCH_ESC_Save_Advice() START !");
	}
	
	inADVICE_ESC_SaveAppend(pobTran, pobTran->srBRec.lnOrgInvNum);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inBATCH_ESC_Save_Advice() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inBATCH_Update_Sign_Status
Date&Time       :2017/3/22 下午 3:46
Describe        :更新簽名狀態
*/
int inBATCH_Update_Sign_Status(TRANSACTION_OBJECT *pobTran)
{
	pobTran->uszUpdateBatchBit = VS_TRUE;
	inBATCH_FuncUpdateTxnRecord(pobTran);
	
	return (VS_SUCCESS);
}


/*
Function        :inADVICE_SaveAppend
Date&Time       :
Describe        :
*/
int inADVICE_SaveAppend(TRANSACTION_OBJECT *pobTran, long lnInvNum)
{
        int             inRetVal;
	char            szAdviceInvNum[10 + 1];
        unsigned char   uszFileName[16 + 1];
        unsigned long   ulFHandle;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inADVICE_SaveAppend()_START");
        }

	if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _ADVICE_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

        inRetVal = inFILE_Open(&ulFHandle, uszFileName);
        if (inRetVal == VS_ERROR)
        {
                /* 檔案不存在 */
                inRetVal = inFILE_Create(&ulFHandle, uszFileName);
                if (inRetVal == VS_ERROR)
                {
                        return (VS_ERROR);
                }
        }

        inRetVal = inFILE_Seek(ulFHandle, 0, _SEEK_END_); /* 指到最後第一個 BYTE */
        if (inRetVal == VS_ERROR)
        {
                inFILE_Close(&ulFHandle);
                return (VS_ERROR);
        }

        memset(szAdviceInvNum, 0x00, sizeof(szAdviceInvNum));
        sprintf(szAdviceInvNum, "%06ld", lnInvNum);
        inRetVal = inFILE_Write(&ulFHandle, (unsigned char *) &szAdviceInvNum[0], _ADV_FIELD_SIZE_);
        if (inRetVal == VS_ERROR)
        {
                inFILE_Close(&ulFHandle);
                return (VS_ERROR);
        }

        inFILE_Close(&ulFHandle);

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inADVICE_SaveAppend()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inADVICE_SaveTop
Date&Time       :2016/9/20 上午 9:53
Describe        :TC Upload 使用
*/
int inADVICE_SaveTop(TRANSACTION_OBJECT *pobTran, long lnInvNum)
{
        int             inRetVal;
        long            lnADVCnt;
	char            szAdviceInvNum[10 + 1];
        unsigned long   ulADVFHandle;
        unsigned char   *uszADVMemory, uszADVFileName[16 + 1];

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inADVICE_SaveTop()_START");
        }

        if (inFunc_ComposeFileName(pobTran, (char*)uszADVFileName, _ADVICE_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

        inRetVal = inFILE_Open(&ulADVFHandle, uszADVFileName);

        if (inRetVal == VS_ERROR)
        {
                /* 檔案不存在 */
                inRetVal = inFILE_Create(&ulADVFHandle, uszADVFileName);
                if (inRetVal == VS_ERROR)
                {
                        return (VS_ERROR);
                }
        }

        /* 取得 Advice 檔案大小 */
        lnADVCnt = lnFILE_GetSize(&ulADVFHandle, uszADVFileName);
        if (lnADVCnt == 0)
        {
                /* 表示是第一筆資料 */
                inRetVal = inFILE_Seek(ulADVFHandle, 0, _SEEK_END_); /* 指到最後第一個 BYTE */
                if (inRetVal == VS_ERROR)
                {
                        inFILE_Close(&ulADVFHandle);
                        return (VS_ERROR);
                }

                memset(szAdviceInvNum, 0x00, sizeof(szAdviceInvNum));
                sprintf(szAdviceInvNum, "%06ld", lnInvNum);
                inRetVal = inFILE_Write(&ulADVFHandle, (unsigned char *)&szAdviceInvNum[0], _ADV_FIELD_SIZE_);
                if (inRetVal == VS_ERROR)
                {
                        inFILE_Close(&ulADVFHandle);
                        return (VS_ERROR);
                }

                inFILE_Close(&ulADVFHandle);
        }
        else
        {
                /* 記憶體 */
                uszADVMemory = malloc(lnADVCnt + 12);
                memset(uszADVMemory, 0x00, (lnADVCnt + 12));
                /* 讀 Advice 檔案資料 */
                inRetVal = inFILE_Seek(ulADVFHandle, 0, _SEEK_BEGIN_); /* 指到第一個 BYTE */
                if (inRetVal == VS_ERROR)
                {
                        inFILE_Close(&ulADVFHandle);
                        free(uszADVMemory);
                        return (VS_ERROR);
                }

                inRetVal = inFILE_Read(&ulADVFHandle, &uszADVMemory[0], lnADVCnt);

                if (inRetVal == VS_ERROR)
                {
                        inFILE_Close(&ulADVFHandle);
                        free(uszADVMemory);
                        return (VS_ERROR);
                }

                /* 關檔案 */
                inFILE_Close(&ulADVFHandle);
                /* 刪除檔案 */
                inFILE_Delete(uszADVFileName);

                /* 重新建立 Advice 檔 */
                inRetVal = inFILE_Create(&ulADVFHandle, uszADVFileName);
                if (inRetVal == VS_ERROR)
                {
                        free(uszADVMemory);
                        return (VS_ERROR);
                }

                inRetVal = inFILE_Seek(ulADVFHandle, 0, _SEEK_END_); /* 指到最後第一個 BYTE */
                if (inRetVal == VS_ERROR)
                {
                        inFILE_Close(&ulADVFHandle);
                        free(uszADVMemory);
                        return (VS_ERROR);
                }

                memset(szAdviceInvNum, 0x00, sizeof(szAdviceInvNum));
                sprintf(szAdviceInvNum, "%06ld", lnInvNum);
                inRetVal = inFILE_Write(&ulADVFHandle, (unsigned char *)&szAdviceInvNum[0], _ADV_FIELD_SIZE_);
                if (inRetVal == VS_ERROR)
                {
                        inFILE_Close(&ulADVFHandle);
                        free(uszADVMemory);
                        return (VS_ERROR);
                }

                inRetVal = inFILE_Seek(ulADVFHandle, 0, _SEEK_END_); /* 指到最後第一個 BYTE */
                if (inRetVal == VS_ERROR)
                {
                        inFILE_Close(&ulADVFHandle);
                        free(uszADVMemory);
                        return (VS_ERROR);
                }

                inRetVal = inFILE_Write(&ulADVFHandle, &uszADVMemory[0], lnADVCnt);
                if (inRetVal == VS_ERROR)
                {
                        inFILE_Close(&ulADVFHandle);
                        free(uszADVMemory);
                        return (VS_ERROR);
                }

                inFILE_Close(&ulADVFHandle);
                free(uszADVMemory);
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inADVICE_SaveTop()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inADVICE_DeleteRecordFlow
Date&Time       :2016/9/20 上午 9:57
Describe        :檢查 lnInvNum 是否在 Advice 檔案中，若是則將 lnInvNum 從 Advice 檔案中刪除
*/
int inADVICE_DeleteRecordFlow(TRANSACTION_OBJECT *pobTran, long lnInvNum)
{
	int	inDeleteAdvIndex = 0;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inADVICE_DeleteRecordFlow() START !");
	}
	
	if (inADVICE_SearchRecord_Index(pobTran, lnInvNum, &inDeleteAdvIndex) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	/* 刪除中間的值 */
	if (inDeleteAdvIndex >= 0)
	{
		if (inADVICE_DeleteRecored(pobTran, inDeleteAdvIndex) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock(AT);
			
			return (VS_ERROR);
		}
	}


	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inADVICE_DeleteRecordFlow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inADVICE_SearchRecord_Index
Date&Time       :2016/9/20 下午 2:08
Describe        :
*/
int inADVICE_SearchRecord_Index(TRANSACTION_OBJECT *pobTran, long lnInvNum, int *inADVIndex)
{
	int		inRecCnt = 0;
	int		inRetVal;
	char		szDebugMsg[100 + 1];
	char		szReadBuf[_ADV_FIELD_SIZE_ + 1];
	unsigned char   uszADVFileName[16 + 1];
	unsigned long	ulHandle;
	
	if (inFunc_ComposeFileName(pobTran, (char*)uszADVFileName, _ADVICE_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* 找到該advice index */
	if (inFILE_OpenReadOnly(&ulHandle, uszADVFileName) != VS_SUCCESS)
		return (VS_ERROR);
	
	inRecCnt = (int)(lnFILE_GetSize(&ulHandle, uszADVFileName) / _ADV_FIELD_SIZE_);
	(*inADVIndex) = 0;
	
	/* 從頭開始搜索 */
	if (inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		if (inFILE_Close(&ulHandle) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock(AT);
		}
		
		return (VS_ERROR);
	}
	
	do
	{
		
		memset(szReadBuf, 0x00, _ADV_FIELD_SIZE_ + 1);
		inRetVal = inFILE_Read(&ulHandle, (unsigned char*)szReadBuf, _ADV_FIELD_SIZE_);

		if (inRetVal != VS_SUCCESS)
		{
			inFILE_Close(&ulHandle);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "inADVICE_SearchRecord_Index ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			
			return (VS_ERROR);
		}
		else
		{
			if (lnInvNum == atol(szReadBuf))
			{
				break;
			}

			/* index + 1 */
			(*inADVIndex) ++;
		}

	} while (*inADVIndex <= inRecCnt);
	
	if (inFILE_Close(&ulHandle) != VS_SUCCESS)
		return (VS_ERROR);
	
	return (VS_SUCCESS);
}

/*
Function        :inADVICE_DeleteRecored
Date&Time       :2016/9/20 下午 1:56
Describe        :刪除ADV檔中的特定invoice number的紀錄
*/
int inADVICE_DeleteRecored(TRANSACTION_OBJECT *pobTran, int inAdvIndex)
{
	int		i;
	int		inAdvSize;					/* ADV檔總長度 */
	int		inRemainLength;					/* 後半段需寫入的長度 */
	long		lnReadLength;					/* 剩餘讀取長度 */
	char		szDebugMsg[100 + 1];
	unsigned char	*uszAdvBuf;
	unsigned char   uszADVFileName[16 + 1], uszADBFileName[16 + 1];
	unsigned long	ulADVHandle, ulADBHandle;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inADVICE_DeleteRecored() START !");
	}
	
	if (inFunc_ComposeFileName(pobTran, (char*)uszADVFileName, _ADVICE_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	if (inFunc_ComposeFileName(pobTran, (char*)uszADBFileName, _ADVICE_BACKUP_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	/* 原始檔案 */
	if (inFILE_OpenReadOnly(&ulADVHandle, uszADVFileName) != VS_SUCCESS)
	{
		inFILE_Close(&ulADVHandle);
		
		/* 鎖機 */
		inFunc_EDCLock(AT);
		
		return (VS_ERROR);
	}

	/* 抓檔案長度 */
	inAdvSize = (int)lnFILE_GetSize(&ulADVHandle, uszADVFileName);
	
	/* 表示只有一筆記錄，直接刪檔即可 */
	if (inAdvSize == _ADV_FIELD_SIZE_)
	{
		
		if (inFILE_Close(&ulADVHandle) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock(AT);
		}

		if (inFILE_Delete(uszADVFileName) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock(AT);
		}
		
		return (VS_SUCCESS);
	}
	
	/* 若有多筆紀錄，從頭開始搜索 */
	if (inFILE_Seek(ulADVHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		if (inFILE_Close(&ulADVHandle) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock(AT);
		}
		
		return (VS_ERROR);
	}
	
	uszAdvBuf = malloc(inAdvSize + 1);
	/* 剩餘讀取長度 */
	lnReadLength = inAdvSize;

	/* 先讀出所有adv內容 */
	for (i = 0;; ++i)
	{
		memset(uszAdvBuf, 0x00, inAdvSize + 1);
		inFILE_Read(&ulADVHandle, uszAdvBuf, 1024);
		/* 剩餘長度大於或等於1024 */
		if (lnReadLength >= 1024)
		{
			if (inFILE_Read(&ulADVHandle, &uszAdvBuf[1024 * i], 1024) == VS_SUCCESS)
			{
				/* 一次讀1024 */
				lnReadLength -= 1024;

				/* 當剩餘長度剛好為1024，會剛好讀完 */
				if (lnReadLength == 0)
					break;
			}
			/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulADVHandle);

				/* Free pointer */
				free(uszAdvBuf);
				
				/* 鎖機 */
				inFunc_EDCLock(AT);
				
				return (VS_ERROR);
			}
		}
		/* 剩餘長度小於1024 */
		else if (lnReadLength < 1024)
		{
			/* 就只讀剩餘長度 */
			if (inFILE_Read(&ulADVHandle, &uszAdvBuf[1024*i], lnReadLength) == VS_SUCCESS)
			{
				break;
			}
			/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulADVHandle);

				/* Free pointer */
				free(uszAdvBuf);
				
				/* 鎖機 */
				inFunc_EDCLock(AT);

				return (VS_ERROR);
			}
		}
		
	};
	
	/* 關檔 */
	if (inFILE_Close(&ulADVHandle) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock(AT);
	}
	
	/* 防呆先刪除ADB */
        inFILE_Delete((unsigned char *)uszADBFileName);
	/* 建新的備份檔案 */
	if (inFILE_Create(&ulADBHandle, uszADBFileName) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock(AT);
		
		return (VS_ERROR);
	}
	
	/* 若有多筆紀錄，從頭開始搜索 */
	if (inFILE_Seek(ulADBHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		if (inFILE_Close(&ulADBHandle) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock(AT);
		}
		
		return (VS_ERROR);
	}
	
	/* 寫入前半段，inAdvIndex為0則代表前半段沒資料需要寫 */
	if (inAdvIndex != 0)
	{
		if (inFILE_Write(&ulADBHandle, uszAdvBuf, _ADV_FIELD_SIZE_ * inAdvIndex) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock(AT);
			
			return (VS_ERROR);
		}
		
	}
	
	/* 跳過那一筆不寫，並移handle到最後面 */
	if (inFILE_Seek(ulADBHandle, 0, _SEEK_END_) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock(AT);

		return (VS_ERROR);
	}
	
	/* 寫入後半段，若剩餘資料長度大於0，代表後面需要寫 */
	inRemainLength = inAdvSize - (_ADV_FIELD_SIZE_ * (inAdvIndex + 1));
	if (inRemainLength > 0)
	{
		if (inFILE_Write(&ulADBHandle, &uszAdvBuf[(_ADV_FIELD_SIZE_ * (inAdvIndex + 1))], inRemainLength) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock(AT);
			
			return (VS_ERROR);
		}
	}
	
	/* 關檔 */
	if (inFILE_Close(&ulADBHandle) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock(AT);
	}
	
	free(uszAdvBuf);

	if (inFILE_Delete(uszADVFileName) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock(AT);
	}
	
	if (inFILE_Rename(uszADBFileName, uszADVFileName) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock(AT);
	}
	
	if (inFILE_Delete(uszADBFileName) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock(AT);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inADVICE_DeleteRecored()_END");
		inLogPrintf(AT, szDebugMsg);
	}

	return (VS_SUCCESS);
}


/*
Function        :inADVICE_DeleteAll
Date&Time       :2016/9/20 上午 9:54
Describe        :刪掉該Host的adv檔
*/
int inADVICE_DeleteAll(TRANSACTION_OBJECT *pobTran)
{
        unsigned char   uszFileName[16 + 1];

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inADVICE_DeleteAll()_START");
        }

	if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _ADVICE_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
        inFILE_Delete(uszFileName);

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inADVICE_DeleteAll()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inADVICE_Update
Date&Time       :
Describe        :因gulADVHandle 是全域變數, 在外面一起關Handle
*/
int inADVICE_Update(TRANSACTION_OBJECT *pobTran)
{
        int 	        inRetVal;
        long	        lnADVCnt;
        unsigned char   *uszADVMemory;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inADVICE_Update()_START");
        }

        /* 取得 Advice 檔案大小 */
        lnADVCnt = lnFILE_GetSize(&gulADVHandle, guszADVFileName);
        if (lnADVCnt == 0)
        {
                return (VS_SUCCESS);
        }
        else
        {
                /* 記憶體 */
                uszADVMemory = malloc(lnADVCnt + 12);
                memset(uszADVMemory, 0x00, (lnADVCnt + 12));
                /* 讀 Advice 檔案資料，指到第二個 Advice */
                inRetVal = inFILE_Seek(gulADVHandle, _ADV_FIELD_SIZE_, _SEEK_BEGIN_); /* 指到第二個 Advice */
                if (inRetVal == VS_ERROR)
                {
                        free(uszADVMemory);
                        return (VS_ERROR);
                }

                lnADVCnt = lnADVCnt - 6;

                if (lnADVCnt == 0)
                {
			/* 如果lnADVCnt等於0，表示最後一筆 */
                        /* 關檔案 */
                        inFILE_Close(&gulADVHandle);
                        /* 刪除檔案 */
                        inFILE_Delete(guszADVFileName);
                }
                else
                {
                        inRetVal = inFILE_Read(&gulADVHandle, &uszADVMemory[0], lnADVCnt);

                        if (inRetVal == VS_ERROR)
                        {
                                inFILE_Close(&gulADVHandle);
                                inFILE_Delete(guszADVFileName);
                                free(uszADVMemory);
                                if (inFILE_Create(&gulADVHandle, guszADVFileName)!=VS_SUCCESS)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inFILE_Create gulADVHandle fail-1");
                                        }
                                }

                                return (VS_ERROR);
                        }

                        /* 關檔案 */
                        inFILE_Close(&gulADVHandle);
                        /* 刪除檔案 */
                        inFILE_Delete(guszADVFileName);

                        /* 重新建立 Advice 檔 */
                        inRetVal = inFILE_Create(&gulADVHandle, guszADVFileName);
                        if (inRetVal == VS_ERROR)
                        {
                                free(uszADVMemory);
                                return (VS_ERROR);
                        }

                        inRetVal = inFILE_Seek(gulADVHandle, 0, _SEEK_END_); /* 指到最後第一個 BYTE */
                        if (inRetVal == VS_ERROR)
                        {
                                free(uszADVMemory);
                                return (VS_ERROR);
                        }

                        inRetVal = inFILE_Write(&gulADVHandle, &uszADVMemory[0], lnADVCnt);
                        if (inRetVal == VS_ERROR)
                        {
                                free(uszADVMemory);
                                return (VS_ERROR);
                        }
        	}
				
			free(uszADVMemory);
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inADVICE_Update()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inADVICE_GetTotalCount
Date&Time       :
Describe        :
*/
int inADVICE_GetTotalCount(TRANSACTION_OBJECT *pobTran)
{
        int 	        inRetVal;
        long	        lnADVCnt;
	char		szDebugMsg[100 + 1];
        unsigned long   ulADVFHandle;
        unsigned char   uszADVFileName[16 + 1];

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inADVICE_GetTotalCount()_START");
        }
	if (inFunc_ComposeFileName(pobTran, (char*)uszADVFileName, _ADVICE_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
        inRetVal = inFILE_OpenReadOnly(&ulADVFHandle, uszADVFileName);
        if (inRetVal == VS_ERROR)
        {
                return (VS_NO_RECORD);
        }

        /* 取得 Advice 檔案大小 */
        lnADVCnt = lnFILE_GetSize(&ulADVFHandle, uszADVFileName);
        if (lnADVCnt == 0)
        {
                inFILE_Close(&ulADVFHandle);
                inFILE_Delete(uszADVFileName);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inADVICE_GetTotalCount(%ld)_END", lnADVCnt);
			inLogPrintf(AT, szDebugMsg);
			inLogPrintf(AT, "----------------------------------------");
		}
                return (VS_NO_RECORD);
        }
        else
        {
                lnADVCnt = (lnADVCnt / _ADV_FIELD_SIZE_);
                inFILE_Close(&ulADVFHandle);
        }

        if (ginDebug == VS_TRUE)
        {
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inADVICE_GetTotalCount(%ld)_END", lnADVCnt);
		inLogPrintf(AT, szDebugMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (lnADVCnt);
}

/*
Function        :inADVICE_GetInvoiceNum
Date&Time       :
Describe        :
*/
int inADVICE_GetInvoiceNum(TRANSACTION_OBJECT *pobTran, char *szGetInvoiceNum)
{
        int             inRetVal;
        unsigned long   ulADVFHandle;
        unsigned char   uszADVFileName[16 + 1];

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inADVICE_GetInvoiceNum()_START");
        }

        if (inFunc_ComposeFileName(pobTran, (char*)uszADVFileName, _ADVICE_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

        inRetVal = inFILE_OpenReadOnly(&ulADVFHandle, uszADVFileName);
        if (inRetVal == VS_ERROR)
        {
                return (VS_NO_RECORD);
        }

        inRetVal = inFILE_Read(&ulADVFHandle, (unsigned char *)&szGetInvoiceNum[0], _ADV_FIELD_SIZE_);

        if (inRetVal == VS_ERROR)
        {
                inFILE_Close(&ulADVFHandle);
                return (VS_ERROR);
        }

        inFILE_Close(&ulADVFHandle);

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inADVICE_GetInvoiceNum()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inADVICE_ESC_SaveAppend
Date&Time       :2017/3/29 上午 10:23
Describe        :For ESC 專用，附檔名為ADVE
*/
int inADVICE_ESC_SaveAppend(TRANSACTION_OBJECT *pobTran, long lnInvNum)
{
        int             inRetVal;
	char            szAdviceInvNum[10 + 1];
        unsigned char   uszFileName[16 + 1] = {0};
        unsigned long   ulFHandle;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inADVICE_ESC_SaveAppend()_START");
        }

	memset(uszFileName, 0x00, sizeof(uszFileName));
	if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _ADVICE_ESC_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

        inRetVal = inFILE_Open(&ulFHandle, uszFileName);
        if (inRetVal == VS_ERROR)
        {
                /* 檔案不存在 */
                inRetVal = inFILE_Create(&ulFHandle, uszFileName);
                if (inRetVal == VS_ERROR)
                {
                        return (VS_ERROR);
                }
        }

        inRetVal = inFILE_Seek(ulFHandle, 0, _SEEK_END_); /* 指到最後第一個 BYTE */
        if (inRetVal == VS_ERROR)
        {
                inFILE_Close(&ulFHandle);
                return (VS_ERROR);
        }

        memset(szAdviceInvNum, 0x00, sizeof(szAdviceInvNum));
        sprintf(szAdviceInvNum, "%06ld", lnInvNum);
        inRetVal = inFILE_Write(&ulFHandle, (unsigned char *) &szAdviceInvNum[0], _ADV_FIELD_SIZE_);
        if (inRetVal == VS_ERROR)
        {
                inFILE_Close(&ulFHandle);
                return (VS_ERROR);
        }

        inFILE_Close(&ulFHandle);

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inADVICE_ESC_SaveAppend()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inADVICE_ESC_SaveAppend_By_Sqlite
Date&Time       :2019/5/6 上午 9:55
Describe        :For ESC 專用，以資料庫儲存
*/
int inADVICE_ESC_SaveAppend_By_Sqlite(TRANSACTION_OBJECT *pobTran, long lnInvNum)
{
	int inRetVal = VS_ERROR;
	char szTableID[12 + 1] = {0};
	char szDebugMsg[100 + 1] = {0};

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inADVICE_ESC_SaveAppend_By_Sqlite()_START");
	}

	inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_BATCH_TABLE_ESC_ADVICE_);
	if (inRetVal != VS_SUCCESS)
	{
		inFunc_EDCLock(AT);
		return (inRetVal);
	}
	
	inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_BATCH_TABLE_ESC_ADVICE_EMV_);
	if (inRetVal != VS_SUCCESS)
	{
		inFunc_EDCLock(AT);
	}
	
	inRetVal = inBATCH_Insert_All_Flow(pobTran, _TN_BATCH_TABLE_ESC_ADVICE_);
	if (inRetVal != VS_SUCCESS)
	{
		inFunc_EDCLock(AT);
		return (inRetVal);
	}
	
	memset(szTableID, 0x00, sizeof(szTableID));
	/* 抓出TableID */
	inRetVal = inSqlite_Get_Max_TableID_Flow(pobTran, _TN_BATCH_TABLE_ESC_ADVICE_, szTableID);
	if (inRetVal != VS_SUCCESS)
	{
		inFunc_EDCLock(AT);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inTableID : %d", atoi(szTableID));
			inLogPrintf(AT, szDebugMsg);
		}
		pobTran->inTableID = atoi(szTableID);
	}
	
	/* 算出是否有【EMV】和【Contactless】交易 */
	if (pobTran->srBRec.inChipStatus != 0 || 
	    pobTran->srBRec.uszContactlessBit == VS_TRUE || 
	    pobTran->srBRec.uszEMVFallBackBit == VS_TRUE ||
	    pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
	{
		inRetVal = inBATCH_Insert_All_Flow(pobTran, _TN_BATCH_TABLE_ESC_ADVICE_EMV_);
		if (inRetVal != VS_SUCCESS)
		{
			inFunc_EDCLock(AT);
		}
	}
	/*[260318_BUG] 因使用原廠API不需再進行同步,移除檔案同步函式 */
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inADVICE_ESC_SaveAppend_By_Sqlite()_END");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inADVICE_ESC_GetTotalCount
Date&Time       :2017/3/29 上午 11:01
Describe        :
*/
int inADVICE_ESC_GetTotalCount(TRANSACTION_OBJECT *pobTran)
{
        int 	        inRetVal;
        long	        lnADVCnt;
	char		szDebugMsg[100 + 1];
        unsigned long   ulADVFHandle;
        unsigned char   uszADVFileName[16 + 1];

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inADVICE_ESC_GetTotalCount()_START");
        }

	if (inFunc_ComposeFileName(pobTran, (char*)uszADVFileName, _ADVICE_ESC_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
        inRetVal = inFILE_OpenReadOnly(&ulADVFHandle, uszADVFileName);
        if (inRetVal == VS_ERROR)
        {
                return (VS_NO_RECORD);
        }

        /* 取得 Advice 檔案大小 */
        lnADVCnt = lnFILE_GetSize(&ulADVFHandle, uszADVFileName);
        if (lnADVCnt == 0)
        {
                inFILE_Close(&ulADVFHandle);
                inFILE_Delete(uszADVFileName);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inADVICE_ESC_GetTotalCount(%ld)_END", lnADVCnt);
			inLogPrintf(AT, szDebugMsg);
			inLogPrintf(AT, "----------------------------------------");
		}
                return (VS_NO_RECORD);
        }
        else
        {
                lnADVCnt = (lnADVCnt / _ADV_FIELD_SIZE_);
                inFILE_Close(&ulADVFHandle);
        }

        if (ginDebug == VS_TRUE)
        {
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inADVICE_ESC_GetTotalCount(%ld)_END", lnADVCnt);
		inLogPrintf(AT, szDebugMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (lnADVCnt);
}

/*
Function        :inADVICE_ESC_GetTotalCount_By_Sqlite
Date&Time       :2019/5/6 下午 5:09
Describe        :
*/
int inADVICE_ESC_GetTotalCount_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
        int	inRetVal = VS_ERROR;
	int	inADVCnt = 0;
        long	lnADVCnt = 0;
	char	szDebugMsg[100 + 1];

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inADVICE_ESC_GetTotalCount_By_Sqlite()_START");
        }

	inRetVal = inSqlite_Get_Batch_Count_Flow(pobTran, _TN_BATCH_TABLE_ESC_ADVICE_, &inADVCnt);
	if (inRetVal == VS_SUCCESS)
	{
		lnADVCnt = inADVCnt;
	}
	else
	{
		lnADVCnt = 0;
	}
		

        if (ginDebug == VS_TRUE)
        {
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inADVICE_ESC_GetTotalCount_By_Sqlite(%ld)_END", lnADVCnt);
		inLogPrintf(AT, szDebugMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (lnADVCnt);
}

/*
Function        :inADVICE_ESC_DeleteRecord
Date&Time       :2016/9/20 上午 9:57
Describe        :檢查 lnInvNum 是否在 Advice 檔案中，若是則將 lnInvNum 從 Advice 檔案中刪除
*/
int inADVICE_ESC_DeleteRecord(TRANSACTION_OBJECT *pobTran, long lnInvNum)
{
	int	inDeleteAdvIndex = 0;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inADVICE_ESC_DeleteRecord() START !");
	}
	
	if (inADVICE_ESC_SearchRecord_Index(pobTran, lnInvNum, &inDeleteAdvIndex) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	/* 刪除中間的值 */
	if (inDeleteAdvIndex >= 0)
	{
		if (inADVICE_ESC_DeleteRecored(pobTran, inDeleteAdvIndex) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock(AT);
			
			return (VS_ERROR);
		}
	}


	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inADVICE_ESC_DeleteRecord() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inADVICE_ESC_DeleteRecord_By_Sqlite
Date&Time       :2019/5/6 下午 7:24
Describe        :刪除最上面一筆且同invoice Number的紀錄
*/
int inADVICE_ESC_DeleteRecord_By_Sqlite(TRANSACTION_OBJECT *pobTran, long lnInvNum)
{
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inADVICE_ESC_DeleteRecord_By_Sqlite() START !");
	}
	
	inSqlite_ESC_Delete_Record_Flow(pobTran, _TN_BATCH_TABLE_ESC_ADVICE_);

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inADVICE_ESC_DeleteRecord_By_Sqlite() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inADVICE_ESC_SearchRecord_Index
Date&Time       :2017/3/29 下午 1:23
Describe        :ESC專用
*/
int inADVICE_ESC_SearchRecord_Index(TRANSACTION_OBJECT *pobTran, long lnInvNum, int *inADVIndex)
{
	int		inRecCnt = 0;
	int		inRetVal;
	char		szDebugMsg[100 + 1];
	char		szReadBuf[_ADV_FIELD_SIZE_ + 1];
	unsigned char   uszADVFileName[16 + 1];
	unsigned long	ulHandle;
	
	if (inFunc_ComposeFileName(pobTran, (char*)uszADVFileName, _ADVICE_ESC_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* 找到該advice index */
	if (inFILE_OpenReadOnly(&ulHandle, uszADVFileName) != VS_SUCCESS)
		return (VS_ERROR);
	
	inRecCnt = (int)(lnFILE_GetSize(&ulHandle, uszADVFileName) / _ADV_FIELD_SIZE_);
	
	/* 從頭開始搜索 */
	if (inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		if (inFILE_Close(&ulHandle) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock(AT);
		}
		
		return (VS_ERROR);
	}
	
	do
	{
		memset(szReadBuf, 0x00, _ADV_FIELD_SIZE_ + 1);
		inRetVal = inFILE_Read(&ulHandle, (unsigned char*)szReadBuf, _ADV_FIELD_SIZE_);

		if (inRetVal != VS_SUCCESS)
		{
			inFILE_Close(&ulHandle);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "inADVICE_ESC_SearchRecord_Index ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			
			return (VS_ERROR);
		}
		else
		{
			if (lnInvNum == atol(szReadBuf))
			{
				break;
			}

			/* index + 1 */
			*inADVIndex += 1;
		}

	} while (*inADVIndex <= inRecCnt);
	
	if (inFILE_Close(&ulHandle) != VS_SUCCESS)
		return (VS_ERROR);
	
	return (VS_SUCCESS);
}

/*
Function        :inADVICE_ESC_DeleteRecored
Date&Time       :2017/3/29 下午 1:24
Describe        :ESC專用，刪除ADV檔中的特定invoice number的紀錄
*/
int inADVICE_ESC_DeleteRecored(TRANSACTION_OBJECT *pobTran, int inAdvIndex)
{
	int		i;
	int		inAdvSize;					/* ADV檔總長度 */
	int		inRemainLength;					/* 後半段需寫入的長度 */
	long		lnReadLength;					/* 剩餘讀取長度 */
	char		szDebugMsg[100 + 1];
	unsigned char	*uszAdvBuf;
	unsigned char   uszADVFileName[16 + 1], uszADBFileName[16 + 1];
	unsigned long	ulADVHandle, ulADBHandle;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inADVICE_ESC_DeleteRecored() START !");
	}
	
	if (inFunc_ComposeFileName(pobTran, (char*)uszADVFileName, _ADVICE_ESC_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	if (inFunc_ComposeFileName(pobTran, (char*)uszADBFileName, _ADVICE_ESC_BACKUP_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	/* 原始檔案 */
	if (inFILE_OpenReadOnly(&ulADVHandle, uszADVFileName) != VS_SUCCESS)
	{
		inFILE_Close(&ulADVHandle);
		
		/* 鎖機 */
		inFunc_EDCLock(AT);
		
		return (VS_ERROR);
	}

	/* 抓檔案長度 */
	inAdvSize = (int)lnFILE_GetSize(&ulADVHandle, uszADVFileName);
	
	/* 表示只有一筆記錄，直接刪檔即可 */
	if (inAdvSize == _ADV_FIELD_SIZE_)
	{
		
		if (inFILE_Close(&ulADVHandle) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock(AT);
		}

		if (inFILE_Delete(uszADVFileName) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock(AT);
		}
		
		return (VS_SUCCESS);
	}
	
	/* 若有多筆紀錄，從頭開始搜索 */
	if (inFILE_Seek(ulADVHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		if (inFILE_Close(&ulADVHandle) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock(AT);
		}
		
		return (VS_ERROR);
	}
	
	uszAdvBuf = malloc(inAdvSize + 1);
	/* 剩餘讀取長度 */
	lnReadLength = inAdvSize;

	/* 先讀出所有adv內容 */
	for (i = 0;; ++i)
	{
		memset(uszAdvBuf, 0x00, inAdvSize + 1);
		inFILE_Read(&ulADVHandle, uszAdvBuf, 1024);
		/* 剩餘長度大於或等於1024 */
		if (lnReadLength >= 1024)
		{
			if (inFILE_Read(&ulADVHandle, &uszAdvBuf[1024 * i], 1024) == VS_SUCCESS)
			{
				/* 一次讀1024 */
				lnReadLength -= 1024;

				/* 當剩餘長度剛好為1024，會剛好讀完 */
				if (lnReadLength == 0)
					break;
			}
			/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulADVHandle);

				/* Free pointer */
				free(uszAdvBuf);
				
				/* 鎖機 */
				inFunc_EDCLock(AT);
				
				return (VS_ERROR);
			}
		}
		/* 剩餘長度小於1024 */
		else if (lnReadLength < 1024)
		{
			/* 就只讀剩餘長度 */
			if (inFILE_Read(&ulADVHandle, &uszAdvBuf[1024*i], lnReadLength) == VS_SUCCESS)
			{
				break;
			}
			/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulADVHandle);

				/* Free pointer */
				free(uszAdvBuf);
				
				/* 鎖機 */
				inFunc_EDCLock(AT);

				return (VS_ERROR);
			}
		}
		
	};
	
	/* 關檔 */
	if (inFILE_Close(&ulADVHandle) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock(AT);
	}
	
	/* 防呆先刪除ADB */
        inFILE_Delete((unsigned char *)uszADBFileName);
	/* 建新的備份檔案 */
	if (inFILE_Create(&ulADBHandle, uszADBFileName) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock(AT);
		
		return (VS_ERROR);
	}
	
	/* 若有多筆紀錄，從頭開始搜索 */
	if (inFILE_Seek(ulADBHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		if (inFILE_Close(&ulADBHandle) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock(AT);
		}
		
		return (VS_ERROR);
	}
	
	/* 寫入前半段，inAdvIndex為0則代表前半段沒資料需要寫 */
	if (inAdvIndex != 0)
	{
		if (inFILE_Write(&ulADBHandle, uszAdvBuf, _ADV_FIELD_SIZE_ * inAdvIndex) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock(AT);
			
			return (VS_ERROR);
		}
		
	}
	
	/* 跳過那一筆不寫，並移handle到最後面 */
	if (inFILE_Seek(ulADBHandle, 0, _SEEK_END_) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock(AT);

		return (VS_ERROR);
	}
	
	/* 寫入後半段，若剩餘資料長度大於0，代表後面需要寫 */
	inRemainLength = inAdvSize - (_ADV_FIELD_SIZE_ * (inAdvIndex + 1));
	if (inRemainLength > 0)
	{
		if (inFILE_Write(&ulADBHandle, &uszAdvBuf[(_ADV_FIELD_SIZE_ * (inAdvIndex + 1))], inRemainLength) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock(AT);
			
			return (VS_ERROR);
		}
	}
	
	/* 關檔 */
	if (inFILE_Close(&ulADBHandle) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock(AT);
	}
	
	free(uszAdvBuf);

	if (inFILE_Delete(uszADVFileName) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock(AT);
	}
	
	if (inFILE_Rename(uszADBFileName, uszADVFileName) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock(AT);
	}
	
	if (inFILE_Delete(uszADBFileName) != VS_SUCCESS)
	{
		/* 鎖機 */
		inFunc_EDCLock(AT);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inADVICE_ESC_DeleteRecored()_END");
		inLogPrintf(AT, szDebugMsg);
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_FuncUpdateTxnRecord_By_Sqlite
Date&Time       :2017/3/14 下午 5:33
Describe        :insert資料到資料庫
*/
int inBATCH_FuncUpdateTxnRecord_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char szTableID[12 + 1] = {0};
	char szDebugMsg[100 + 1] = {0};
	char szCustomerIndicator[3 + 1] = {0};

	vdUtility_SYSFIN_LogMessage(AT, "inBATCH_FuncUpdateTxnRecord_By_Sqlite START!");

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inBATCH_FuncUpdateTxnRecord_By_Sqlite()_START");
	}
	/* 若斷電導致批次無法正確儲存時方便解釋 */
	inUtility_StoreTraceLog_OneStep("_UPDATE_BATCH_START_($:%ld)", pobTran->srBRec.lnTxnAmount);
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 如果是update，不用再試圖創Table */
	if (pobTran->uszUpdateBatchBit == VS_TRUE)
	{
		inRetVal = inSqlite_Check_Table_Exist_Flow(pobTran, _TN_BATCH_TABLE_);
		/* 如果要update卻沒有table，直接跳出 */
		if (inRetVal != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inBATCH_FuncUpdateTxnRecord_By_Sqlite _TN_BATCH_TABLE_not_exist");
			inFunc_EDCLock(AT);
		}
	}
	else
	{
		inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_BATCH_TABLE_);
		if (inRetVal != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inBATCH_FuncUpdateTxnRecord_By_Sqlite _TN_BATCH_TABLE_create_fail");
			inFunc_EDCLock(AT);
		}
		inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_EMV_TABLE_);
		if (inRetVal != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inBATCH_FuncUpdateTxnRecord_By_Sqlite _TN_EMV_TABLE__create_fail");
			inFunc_EDCLock(AT);
		}
	}

	/* 如果uszUpdate設為1 */
	if (pobTran->uszUpdateBatchBit == VS_TRUE)
	{
		inSqlite_Update_ByInvNum_TranState_Flow(pobTran, _TN_BATCH_TABLE_, pobTran->srBRec.lnOrgInvNum);
	}

	/* Insert批次 */
	inRetVal = inBATCH_Insert_All_Flow(pobTran, _TN_BATCH_TABLE_);
	if (inRetVal != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inBATCH_FuncUpdateTxnRecord_By_Sqlite _TN_BATCH_TABLE__insert_failed");
		inFunc_EDCLock(AT);
	}

	memset(szTableID, 0x00, sizeof(szTableID));
	/* 抓出TableID */
	inRetVal = inSqlite_Get_Max_TableID_Flow(pobTran, _TN_BATCH_TABLE_, szTableID);
	if (inRetVal != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inBATCH_FuncUpdateTxnRecord_By_Sqlite _TN_BATCH_TABLE_get_max_ID_failed");
		inFunc_EDCLock(AT);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inTableID : %d", atoi(szTableID));
			inLogPrintf(AT, szDebugMsg);
		}
		/* 用在Insert EMV Record上? */
		pobTran->inTableID = atoi(szTableID);
	}
	
	/* 算出是否有【EMV】和【Contactless】交易 */
	if (pobTran->srBRec.inChipStatus != 0 || 
	    pobTran->srBRec.uszContactlessBit == VS_TRUE || 
	    pobTran->srBRec.uszEMVFallBackBit == VS_TRUE ||
	    pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
	{
		inRetVal = inBATCH_Insert_All_Flow(pobTran, _TN_EMV_TABLE_);
		if (inRetVal != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inBATCH_FuncUpdateTxnRecord_By_Sqlite _TN_EMV_TABLE_insert_failed");
			inFunc_EDCLock(AT);
		}
	}
	
	/*[260318_BUG] 因使用原廠API不需再進行同步,移除檔案同步函式 */

	/* 【需求單 - 108128】	單機重印前筆簽單流程新增畫面 by Russell 2019/8/15 下午 2:59 */
	inFLOW_RunFunction(pobTran, _FUNCTION_SAVE_LAST_TXN_HOST_);

	/* 客製化098，儲存最後一筆成功唯一碼 */
	inFLOW_RunFunction(pobTran, _FUNCTION_SAVE_LAST_UNIQUE_NO_);
	
	/* 【需求單 - 105196】限制每日同一卡號不得連續刷卡，且端末機交易金額累積不得超過15萬元 2022/9/28 下午 8:32 */
	/* 存完Batch接著存卡號和交易金額 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inNCCC_Func_Store_Amount_by_Card(pobTran);
	}
	
	/* 若斷電導致批次無法正確儲存時方便解釋 */
	inUtility_StoreTraceLog_OneStep("_UPDATE_BATCH_END_($:%ld)", pobTran->srBRec.lnTxnAmount);

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inBATCH_FuncUpdateTxnRecord_By_Sqlite()_END");
		inLogPrintf(AT, "----------------------------------------");
	}

	vdUtility_SYSFIN_LogMessage(AT, "inBATCH_FuncUpdateTxnRecord_By_Sqlite END!");

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_CheckReport_By_Sqlite
Date&Time       :2017/3/14 下午 5:34
Describe        :處理 inBATCH_GetTotalCountFromBakFile()的錯誤訊息
*/
int inBATCH_CheckReport_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
        int     inRetCnt = -1;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_CheckReport_By_Sqlite()_START");
        }

        inRetCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);
        if (inRetCnt == 0)
        {
                /* 顯示無交易紀錄 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_RECORD_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _ENTER_KEY_MSG_;
		srDispMsgObj.inTimeout = 2;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
			
		inDISP_Msg_BMP(&srDispMsgObj);

                return (VS_ERROR);
        }
        else if (inRetCnt == VS_ERROR)
        {
                /* 顯示讀取批次檔錯誤 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_BATCH_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = 2;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);

                return (VS_ERROR);
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_CheckReport_By_Sqlite()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inBATCH_GetTotalCountFromBakFile_By_Sqlite
Date&Time       :2017/3/15 下午 5:49
Describe        :有幾筆record，回傳值為筆數
*/
int inBATCH_GetTotalCountFromBakFile_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	int	inCnt = -1;

        if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GetTotalCountFromBakFile_By_Sqlite()_START");
        }

	inRetVal = inSqlite_Get_Batch_Count_Flow(pobTran, _TN_BATCH_TABLE_, &inCnt);
	if (inRetVal == VS_NO_RECORD)
	{
		return (0);
	}
	else if (inRetVal != VS_SUCCESS)
	{
		return (-1);
	}

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GetTotalCountFromBakFile_By_Sqlite()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (inCnt);
}

/*
Function        :inBATCH_GetTotalCountFromBakFile_By_Sqlite_ESVC
Date&Time       :2018/1/31 下午 1:20
Describe        :有幾筆record，回傳值為筆數
*/
int inBATCH_GetTotalCountFromBakFile_By_Sqlite_ESVC(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	int	inCnt = -1;

        if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GetTotalCountFromBakFile_By_Sqlite_ESVC()_START");
        }

	inRetVal = inSqlite_Get_Batch_Count_Flow(pobTran, _TN_BATCH_TABLE_TICKET_, &inCnt);
	if (inRetVal == VS_NO_RECORD)
	{
		return (VS_NO_RECORD);
	}
	else if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GetTotalCountFromBakFile_By_Sqlite_ESVC()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (inCnt);
}

/*
Function        :inBATCH_ReviewReport_Detail_Flow_By_Sqlite
Date&Time       :2017/6/12 下午 5:42
Describe        :交易查詢分流
*/
int inBATCH_ReviewReport_Detail_Flow_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szFunEnable[2 + 1];
	
	memset(szFunEnable, 0x00, sizeof(szFunEnable));
	inGetStore_Stub_CardNo_Truncate_Enable(szFunEnable);
	
	if (memcmp(szFunEnable, "Y", strlen("Y")) == 0)
	{
		inRetVal = inBATCH_ReviewReport_Detail_Txno_By_Sqlite(pobTran);
	}
	else
	{
		inRetVal = inBATCH_ReviewReport_Detail_By_Sqlite(pobTran);
	}
	
	return (inRetVal);
}

/*
Function        :inBATCH_ReviewReport_Detail_By_Sqlite
Date&Time       :2017/3/14 下午 5:48
Describe        :交易查詢使用
*/
int inBATCH_ReviewReport_Detail_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int		inNowindex = 0;
	int		inFileRecCnt = -1;
	int		inTouchPage = 0;
	char		szTemplate1[42 + 1];
	char		szTemplate2[42 + 1];
	char		szFinalPAN[_PAN_UCARD_SIZE_ + 1];
	char		szDispMsg [50 + 1];
	char		szDebugMsg[100 + 1];
	char		szKey = 0x00;
	char		szTRTFileName[16 + 1];
	unsigned char	uszBatchEndBit = VS_FALSE;
	unsigned char	uszBatchCancelBit = VS_FALSE;
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_ReviewReport_Detail_By_Sqlite()_START");
        }

	/* 算出交易總筆數 */
	inFileRecCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "筆數: %d", inFileRecCnt);
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* 沒交易紀錄 */
	if (inFileRecCnt == 0)
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第一層顯示 <交易查詢> */
		return (VS_NO_RECORD);
	}

	/* 先抓出總資料 */
	inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(pobTran);
	/* 預設為無須重找，但往回翻時要取消 */
	guszEnormousNoNeedResetBit = VS_TRUE;
	
	while (inNowindex < inFileRecCnt)
        {
                /* 開始讀取每一筆交易記錄 */
                if (inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(pobTran, inNowindex) != VS_SUCCESS)
                {
                        uszBatchEndBit = VS_TRUE;
                        break;
                }
		
		inDISP_ClearAll();

                if (uszBatchEndBit == VS_TRUE	||
		    uszBatchCancelBit == VS_TRUE)
		{
                        break;
		}
		
		inGetTRTFileName(szTRTFileName);
		
		/* DCC轉台幣的紀錄要特別處理，原先的那筆DCC一般交易不能出現 */
		if (pobTran->srBRec.inCode == _VOID_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)
		{
			if (inTouchPage == _DisTouch_Slide_Right_To_Left_ || szKey == _KEY_ENTER_ || szKey == _KEY_DOWN_ || szKey == _KEY_DOWN_HALF_)
			{
				inNowindex++;
				guszEnormousNoNeedResetBit = VS_TRUE;
				continue;
			}
			else if (inTouchPage == _DisTouch_Slide_Left_To_Right_ || szKey == _KEY_UP_ || szKey == _KEY_UP_HALF_)
			{
				inNowindex--;
				guszEnormousNoNeedResetBit = VS_FALSE;
				/* 交易紀錄編號不可小於0 */
				if (inNowindex < 0)
				{
					inNowindex = 0;
					/* 加這行是因為，若第一筆是DCC轉台幣會陷入無限迴圈，所以改滑動方向 */
					inTouchPage = _DisTouch_Slide_Right_To_Left_;
					szKey = _KEY_ENTER_;
				}
				continue;
			}
			/* 若第一筆就是要跳過的紀錄，會因為沒有event，而沒有跳過，所以預設為下一頁 */
			else
			{
				inNowindex++;
				guszEnormousNoNeedResetBit = VS_TRUE;
				continue;
			}
			
		}
		
		/* 優惠兌換的顯示比較特別 */
                if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_		||
		    pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
		{
			/* Invoice Number */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_)
			{
				sprintf(szDispMsg, "優惠兌換     %03ld", pobTran->srBRec.lnOrgInvNum);
			}
			else if (pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
			{
				sprintf(szDispMsg, "兌換取消     %03ld", pobTran->srBRec.lnOrgInvNum);
			}
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_1_, _DISP_LEFT_);

			/* Data & Time */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "%.2s/%.2s/%.2s", &pobTran->srBRec.szDate[2], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_2_, _DISP_LEFT_);
			
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "%.2s:%.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2], &pobTran->srBRec.szTime[4]);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_3_, _DISP_LEFT_);
		}
		else
		{
		

			/* Disp Card Label & Expire date*/
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			inGetTRTFileName(szTemplate1);
			/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
			/* 金融卡沒有效期 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				sprintf(szDispMsg, " %s", pobTran->srBRec.szCardLabel);
			}
			/* 其他加密 */
			else
			{
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
				if (strlen(pobTran->srBRec.szCheckNO) > 0)
				{
					strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
				}
				else
				{
					inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
				}
				/* 【需求單 - 106349】自有品牌判斷需求 */
				/* unpack電文就存存CardLabel modify by LingHsiung 2020/2/14 上午 11:07 */
				sprintf(szDispMsg, " %s   %s", pobTran->srBRec.szCardLabel, szTemplate1);
			}

			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_1_, _DISP_LEFT_);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "CardLabel: %s", pobTran->srBRec.szCardLabel);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Disp Card Number */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
			strcpy(szFinalPAN, pobTran->srBRec.szPAN);

			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			sprintf(szDispMsg, " %s",szFinalPAN);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "szPAN: %s", pobTran->srBRec.szPAN);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Transaction Type */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memset(szTemplate2, 0x00, sizeof(szTemplate2));
			inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
			if (strlen(szTemplate2) > 0)
			{
				sprintf(szDispMsg, " %s", szTemplate1);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_5_, _DISP_LEFT_);
				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				sprintf(szDispMsg, " %s", szTemplate2);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_6_, _DISP_LEFT_);
			}
			else
			{
				sprintf(szDispMsg, " %s %s", szTemplate1, szTemplate2);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
			}


			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Tran Type: %s %s", szTemplate1, szTemplate2);
				inLogPrintf(AT, szDebugMsg);
			}

			/* 授權碼(金卡顯示調單編號) */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				sprintf(szDispMsg, " RRN NO.: %s", pobTran->srBRec.szFiscRRN);
			}
			else
			{
				sprintf(szDispMsg, " APPR: %s", pobTran->srBRec.szAuthCode);
			}
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_7_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Reference Number */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, " REF: %s", pobTran->srBRec.szRefNo);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_8_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Invoice Number */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, " INV: %06ld", pobTran->srBRec.lnOrgInvNum);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%06ld", pobTran->srBRec.lnOrgInvNum);
				inLogPrintf(AT, szDebugMsg);
			}

			/* 是否有優惠兌換資訊 */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			if (pobTran->srBRec.uszVOIDBit == VS_FALSE	&&
			    atoi(pobTran->srBRec.szAwardNum) > 0)
			{
				strcpy(szDispMsg, "award");
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_RIGHT_);
			}
			else if (pobTran->srBRec.uszVOIDBit == VS_TRUE &&
				 atoi(pobTran->srBRec.szAwardNum) > 0)
			{
				strcpy(szDispMsg, "void awd");
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_RIGHT_);
			}

			/* Amount */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));

			if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
			{
				switch (pobTran->srBRec.inCode)
				{
					case _SALE_ :
					case _INST_SALE_ :
					case _REDEEM_SALE_ :
					case _SALE_OFFLINE_ :
					case _FORCE_CASH_ADVANCE_ :
					case _PRE_COMP_:
					case _PRE_AUTH_ :
					case _CUP_SALE_ :
					case _CUP_PRE_COMP_ :
					case _CUP_PRE_AUTH_ :
					case _INST_ADJUST_:
					case _REDEEM_ADJUST_:
					case _MAIL_ORDER_ :
					case _CUP_MAIL_ORDER_ :
					case _CASH_ADVANCE_ :
					case _FISC_SALE_ :
						if (pobTran->srBRec.lnTipTxnAmount != 0)
							sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
						else
							sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					case _ADJUST_:
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					case _TIP_ :
						sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					case _REFUND_ :
					case _INST_REFUND_ :
					case _REDEEM_REFUND_ :
					case _CUP_REFUND_ :
					case _CUP_MAIL_ORDER_REFUND_ :
					case _FISC_REFUND_ :
						sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					default :
						sprintf(szDispMsg, " AMT_ERR_(%d)", pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			else
			{
				switch (pobTran->srBRec.inOrgCode)
				{
					case _SALE_ :
					case _INST_SALE_ :
					case _REDEEM_SALE_ :
					case _SALE_OFFLINE_ :
					case _FORCE_CASH_ADVANCE_ :
					case _PRE_COMP_:
					case _PRE_AUTH_ :
					case _CUP_SALE_ :
					case _CUP_PRE_COMP_ :
					case _CUP_PRE_AUTH_ :
					case _INST_ADJUST_:
					case _REDEEM_ADJUST_:
					case _MAIL_ORDER_ :
					case _CUP_MAIL_ORDER_ :
					case _CASH_ADVANCE_ :
					case _FISC_SALE_ :
						sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					case _ADJUST_:
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					/* 目前小費沒有取消 */
					case _REFUND_ :
					case _INST_REFUND_ :
					case _REDEEM_REFUND_ :
					case _CUP_REFUND_ :
					case _CUP_MAIL_ORDER_REFUND_ :
					/* SmartPay不能取消 */
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					default :
						sprintf(szDispMsg, " AMT_VOID_ERR_(%d)", pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Store ID */
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memset(szDispMsg, 0x00, sizeof(szDispMsg));

			inGetStoreIDEnable(&szTemplate1[0]);

			if ( (szTemplate1[0] == 'Y') && (strlen(pobTran->srBRec.szStoreID) > 0))
			{
				if (strlen(pobTran->srBRec.szStoreID) > 42)
					memcpy(&szTemplate1[0], &pobTran->srBRec.szStoreID[0], 42);
				else
					strcpy(szTemplate1, pobTran->srBRec.szStoreID);

				szTemplate1[18] = 0x00;

				sprintf(szDispMsg, " STD:%s", pobTran->srBRec.szStoreID);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
			}
			else
			{
				/* 沒開櫃號功能， 不印櫃號 */
			}

			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "%d / %d", inNowindex + 1, inFileRecCnt);
			inDISP_ChineseFont_Point_Color(szDispMsg, _FONTSIZE_8X22_, _LINE_8_8_, _COLOR_BLACK_, _COLOR_LIGHTBLUE_, 10);
		}
		
		/* 事件初始化 */
		szKey = 0;
		/* 設定Timeout */
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		
		while (1)
		{
			
			/* 偵測滑動 */
			inTouchPage = inDisTouch_TouchSensor_Click_Slide(_Touch_NEWUI_REVIEW_BATCH_);
			if (inTouchPage == _DisTouch_Slide_Right_To_Left_)
			{
				++inNowindex;
				break;
			}
			if (inTouchPage == _DisTouch_Slide_Left_To_Right_)
			{
				--inNowindex;
				
				/* 交易紀錄編號不可小於0 */
				if (inNowindex < 0)
				{
					inNowindex = 0;
				}
				break;
			}
			
			/* 偵測按鍵 */
			szKey = uszKBD_Key();
			
			/* TimeOut */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}
			
			if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
			{
				uszBatchCancelBit = VS_TRUE;
				break;
			}
			else if (szKey == _KEY_ENTER_ || szKey == _KEY_DOWN_ || szKey == _KEY_DOWN_HALF_)
			{
				++inNowindex;
				break;
			}
			else if (szKey == _KEY_UP_ || szKey == _KEY_UP_HALF_ || szKey == _KEY_FUNCTION_)
			{
				--inNowindex;
				/* 交易紀錄編號不可小於0 */
				if (inNowindex < 0)
				{
					inNowindex = 0;
				}
				break;
			}

		}
		
        } /* End for () .... */
	
	inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(pobTran);

	if (uszBatchEndBit == VS_TRUE)
	{
		/* 顯示批次結束 */
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);			/* 第一層顯示 <交易查詢> */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_BATCH_END_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _ENTER_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
			
		inDISP_Msg_BMP(&srDispMsgObj);
	}

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_ReviewReport_Detail_By_Sqlite()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inBATCH_ReviewReport_Detail_Txno_By_Sqlite
Date&Time       :2017/6/12 下午 5:54
Describe        :交易查詢使用，for卡號遮掩
*/
int inBATCH_ReviewReport_Detail_Txno_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int		i = 0;
	int		inNowindex = 0;
	int		inFileRecCnt = -1;
	int		inTouchPage = 0;
	char		szTemplate1[42 + 1];
	char		szTemplate2[42 + 1];
	char		szFinalPAN[_PAN_UCARD_SIZE_ + 1];
	char		szDispMsg [50 + 1];
	char		szDebugMsg[100 + 1];
	char		szKey = 0x00;
	char		szTRTFileName[16 + 1];
	unsigned char	uszBatchEndBit = VS_FALSE;
	unsigned char	uszBatchCancelBit = VS_FALSE;
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_ReviewReport_Detail_Txno_By_Sqlite()_START");
        }

	/* 算出交易總筆數 */
	inFileRecCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "筆數: %d", inFileRecCnt);
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* 沒交易紀錄 */
	if (inFileRecCnt == 0)
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第一層顯示 <交易查詢> */
		return (VS_NO_RECORD);
	}

	/* 先抓出總資料 */
	inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(pobTran);
	/* 預設為無須重找，但往回翻時要取消 */
	guszEnormousNoNeedResetBit = VS_TRUE;
	
	while (inNowindex < inFileRecCnt)
        {
                /* 開始讀取每一筆交易記錄 */
                if (inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(pobTran, inNowindex) != VS_SUCCESS)
                {
			uszBatchEndBit = VS_TRUE;
                        break;
                }
		
		inDISP_ClearAll();

                if (uszBatchEndBit == VS_TRUE	||
		    uszBatchCancelBit == VS_TRUE)
                        break;
		
		inGetTRTFileName(szTRTFileName);
		
		/* DCC轉台幣的紀錄要特別處理，原先的那筆DCC一般交易不能出現 */
		if (pobTran->srBRec.inCode == _VOID_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)
		{
			if (inTouchPage == _DisTouch_Slide_Right_To_Left_ || szKey == _KEY_ENTER_ || szKey == _KEY_DOWN_ || szKey == _KEY_DOWN_HALF_)
			{
				inNowindex++;
				guszEnormousNoNeedResetBit = VS_TRUE;
				continue;
			}
			else if (inTouchPage == _DisTouch_Slide_Left_To_Right_ || szKey == _KEY_UP_ || szKey == _KEY_UP_HALF_)
			{
				inNowindex--;
				guszEnormousNoNeedResetBit = VS_FALSE;
				/* 交易紀錄編號不可小於0 */
				if (i < 0)
				{
					i = 0;
					/* 加這行是因為，若第一筆是DCC轉台幣會陷入無限迴圈，所以改滑動方向 */
					inTouchPage = _DisTouch_Slide_Right_To_Left_;
					szKey = _KEY_ENTER_;
				}
				continue;
			}
			/* 若第一筆就是要跳過的紀錄，會因為沒有event，而沒有跳過，所以預設為下一頁 */
			else
			{
				inNowindex++;
				guszEnormousNoNeedResetBit = VS_TRUE;
				continue;
			}
			
		}
		
		/* 優惠兌換的顯示比較特別 */
                if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_		||
		    pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
		{
			/* Invoice Number */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_)
			{
				sprintf(szDispMsg, "優惠兌換     %03ld", pobTran->srBRec.lnOrgInvNum);
			}
			else if (pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
			{
				sprintf(szDispMsg, "兌換取消     %03ld", pobTran->srBRec.lnOrgInvNum);
			}
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_1_, _DISP_LEFT_);

			/* Data & Time */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "%.2s/%.2s/%.2s", &pobTran->srBRec.szDate[2], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_2_, _DISP_LEFT_);
			
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "%.2s:%.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2], &pobTran->srBRec.szTime[4]);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_3_, _DISP_LEFT_);
		}
		else
		{
			/* Invoice Number */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, " INV# %06ld", pobTran->srBRec.lnOrgInvNum);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_1_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%06ld", pobTran->srBRec.lnOrgInvNum);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* Transaction Type */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memset(szTemplate2, 0x00, sizeof(szTemplate2));
			inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
			if (strlen(szTemplate2) > 0)
			{
				sprintf(szDispMsg, " %s", szTemplate1);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_3_, _DISP_LEFT_);
				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				sprintf(szDispMsg, " %s", szTemplate2);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_4_, _DISP_LEFT_);
			}
			else
			{
				sprintf(szDispMsg, " %s %s", szTemplate1, szTemplate2);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
			}


			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Tran Type: %s %s", szTemplate1, szTemplate2);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* Disp Card Number */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
			strcpy(szFinalPAN, pobTran->srBRec.szPAN);

			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			/* 卡號遮掩 */
			/* DFS需求不再使用大來主機 2018/5/8 下午 2:10 by Russell */
			for (i = 6; i < (strlen(szFinalPAN) - 4); i ++)
				szFinalPAN[i] = 0x2A;
			
			sprintf(szDispMsg, " %s",szFinalPAN);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "szPAN: %s", pobTran->srBRec.szPAN);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* Check Number or Exp Date*/
        		memset(szTemplate1, 0x00, sizeof(szTemplate1));
        		memset(szDispMsg, 0x00, sizeof(szDispMsg));
			inGetTRTFileName(szTemplate1);
        
			/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
        		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				inGetCardLabel(szTemplate1);
        			sprintf(szDispMsg, "%s", szTemplate1);
        		}
        		else
        		{
        			/* Check Number */
        			memset(szTemplate1, 0x00, sizeof(szTemplate1));
        			/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
				if (strlen(pobTran->srBRec.szCheckNO) > 0)
				{
					strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
				}
				else
				{
					inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
				}
        			sprintf(szDispMsg, " Check# %s", szTemplate1);
        		}
        		inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
			
			/* Data & Time */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, " 20%.2s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[2], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
					
			/* 授權碼(金卡顯示調單編號) */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				sprintf(szDispMsg, " RRN NO.: %s", pobTran->srBRec.szFiscRRN);
			}
			else
			{
				sprintf(szDispMsg, " APP# %s", pobTran->srBRec.szAuthCode);
			}
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);

			/* 是否有優惠兌換資訊 */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			if (pobTran->srBRec.uszVOIDBit == VS_FALSE	&&
			    atoi(pobTran->srBRec.szAwardNum) > 0)
			{
				strcpy(szDispMsg, "award ");
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_6_, _DISP_RIGHT_);
			}
			else if (pobTran->srBRec.uszVOIDBit == VS_TRUE &&
				 atoi(pobTran->srBRec.szAwardNum) > 0)
			{
				strcpy(szDispMsg, "void awd ");
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_6_, _DISP_RIGHT_);
			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}
	
			/* Amount */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));

			if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
			{
				switch (pobTran->srBRec.inCode)
				{
					case _SALE_ :
					case _INST_SALE_ :
					case _REDEEM_SALE_ :
					case _SALE_OFFLINE_ :
					case _FORCE_CASH_ADVANCE_ :
					case _PRE_COMP_:
					case _PRE_AUTH_ :
					case _CUP_SALE_ :
					case _CUP_PRE_COMP_ :
					case _CUP_PRE_AUTH_ :
					case _INST_ADJUST_:
					case _REDEEM_ADJUST_:
					case _MAIL_ORDER_ :
					case _CUP_MAIL_ORDER_ :
					case _CASH_ADVANCE_ :
					case _FISC_SALE_ :
						if (pobTran->srBRec.lnTipTxnAmount != 0)
							sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
						else
							sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "AMT $ ", ' ', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					case _ADJUST_:
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "AMT $ ", ' ', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					case _TIP_ :
						sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
						inFunc_Amount_Comma(szTemplate1, "AMT $ ", ' ', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					case _REFUND_ :
					case _INST_REFUND_ :
					case _REDEEM_REFUND_ :
					case _CUP_REFUND_ :
					case _CUP_MAIL_ORDER_REFUND_ :
					case _FISC_REFUND_ :
						sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
						inFunc_Amount_Comma(szTemplate1, "AMT $ ", ' ', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					default :
						sprintf(szDispMsg, " AMT_ERR_(%d)", pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			else
			{
				switch (pobTran->srBRec.inOrgCode)
				{
					case _SALE_ :
					case _INST_SALE_ :
					case _REDEEM_SALE_ :
					case _SALE_OFFLINE_ :
					case _FORCE_CASH_ADVANCE_ :
					case _PRE_COMP_:
					case _PRE_AUTH_ :
					case _CUP_SALE_ :
					case _CUP_PRE_COMP_ :
					case _CUP_PRE_AUTH_ :
					case _INST_ADJUST_:
					case _REDEEM_ADJUST_:
					case _MAIL_ORDER_ :
					case _CUP_MAIL_ORDER_ :
					case _CASH_ADVANCE_ :
					case _FISC_SALE_ :
						sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
						inFunc_Amount_Comma(szTemplate1, "AMT $ ", ' ', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					case _ADJUST_:
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "AMT $ ", ' ', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					/* 目前小費沒有取消 */
					case _REFUND_ :
					case _INST_REFUND_ :
					case _REDEEM_REFUND_ :
					case _CUP_REFUND_ :
					case _CUP_MAIL_ORDER_REFUND_ :
					/* SmartPay不能取消 */
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "AMT $ ", ' ', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s", szTemplate1);
						break;
					default :
						sprintf(szDispMsg, " AMT_VOID_ERR_(%d)", pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Store ID */
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memset(szDispMsg, 0x00, sizeof(szDispMsg));

			inGetStoreIDEnable(&szTemplate1[0]);

			if ( (szTemplate1[0] == 'Y') && (strlen(pobTran->srBRec.szStoreID) > 0))
			{
				if (strlen(pobTran->srBRec.szStoreID) > 42)
					memcpy(&szTemplate1[0], &pobTran->srBRec.szStoreID[0], 42);
				else
					strcpy(szTemplate1, pobTran->srBRec.szStoreID);

				szTemplate1[18] = 0x00;

				sprintf(szDispMsg, " STD:%s", pobTran->srBRec.szStoreID);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
			}
			else
			{
				/* 沒開櫃號功能， 不印櫃號 */
			}

//			memset(szDispMsg, 0x00, sizeof(szDispMsg));
//			sprintf(szDispMsg, "%d / %d", inNowindex + 1, inFileRecCnt);
//			inDISP_ChineseFont_Point_Color(szDispMsg, _FONESIZE_8X22_, _LINE_8X22_8_, _COLOR_BLACK_, _COLOR_LIGHTBLUE_, 10);
		}
		
		/* 事件初始化 */
		szKey = 0;
		/* 設定Timeout */
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		
		while (1)
		{
			
			/* 偵測滑動 */
			inTouchPage = inDisTouch_TouchSensor_Click_Slide(_Touch_NEWUI_REVIEW_BATCH_);
			if (inTouchPage == _DisTouch_Slide_Right_To_Left_)
			{
				++inNowindex;
				break;
			}
			if (inTouchPage == _DisTouch_Slide_Left_To_Right_)
			{
				--inNowindex;
				
				/* 交易紀錄編號不可小於0 */
				if (inNowindex < 0)
				{
					inNowindex = 0;
				}
				break;
			}
			
			/* 偵測按鍵 */
			szKey = uszKBD_Key();
			
			/* TimeOut */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}
			
			if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
			{
				uszBatchCancelBit = VS_TRUE;
				break;
			}
			else if (szKey == _KEY_ENTER_ || szKey == _KEY_DOWN_ || szKey == _KEY_DOWN_HALF_)
			{
				++inNowindex;
				break;
			}
			else if (szKey == _KEY_UP_ || szKey == _KEY_UP_HALF_ || szKey == _KEY_FUNCTION_)
			{
				--inNowindex;
				/* 交易紀錄編號不可小於0 */
				if (inNowindex < 0)
				{
					inNowindex = 0;
				}
				break;
			}

		}
		
        } /* End for () .... */
	
	inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(pobTran);

	/* 顯示批次結束 */
	if (uszBatchEndBit == VS_TRUE)
	{
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);			/* 第一層顯示 <交易查詢> */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_BATCH_END_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _ENTER_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
			
		inDISP_Msg_BMP(&srDispMsgObj);
	}

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_ReviewReport_Detail_Txno_By_Sqlite()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inBATCH_ReviewReport_Detail_NEWUI_Flow_By_Sqlite
Date&Time       :2017/6/12 下午 5:42
Describe        :交易查詢分流
*/
int inBATCH_ReviewReport_Detail_NEWUI_Flow_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szFunEnable[2 + 1];
        
        vdUtility_SYSFIN_LogMessage(AT, "inBATCH_ReviewReport_Detail_NEWUI_Flow_By_Sqlite START!");
	
	memset(szFunEnable, 0x00, sizeof(szFunEnable));
	inGetStore_Stub_CardNo_Truncate_Enable(szFunEnable);
	
	if (memcmp(szFunEnable, "Y", strlen("Y")) == 0)
	{
		inRetVal = inBATCH_ReviewReport_Detail_Txno_NEWUI_By_Sqlite(pobTran);
	}
	else
	{
		inRetVal = inBATCH_ReviewReport_Detail_NEWUI_By_Sqlite(pobTran);
	}
	
	if (inRetVal != VS_SUCCESS)
	{
		inBATCH_CheckReport_By_Sqlite(pobTran);
	}
	
	return (inRetVal);
}

/*
Function        :inBATCH_ReviewReport_Detail_NEWUI_By_Sqlite_NEWUI
Date&Time       :2017/3/14 下午 5:48
Describe        :交易查詢使用
*/
int inBATCH_ReviewReport_Detail_NEWUI_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int		inLen = 0;
	int		inESVCIndex = -1;
	int		inRetVal = VS_SUCCESS;
	int		inNowindex = 0;
	int		inFileRecCnt = -1;
	int		inTouchPage = 0;
	char		szTemplate1[42 + 1] = {0};
	char		szTemplate2[42 + 1] = {0};
	char		szFinalPAN[_PAN_UCARD_SIZE_ + 1] = {0};
	char		szDispMsg [50 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		szKey = 0x00;
	char		szTRTFileName[16 + 1] = {0};
	char		szFuncEnable[2 + 1] = {0};
        char            szCustomerIndicator[3 + 1] = {0};
	unsigned char	uszBatchEndBit = VS_FALSE;
	unsigned char	uszBatchCancelBit = VS_FALSE;
	
        vdUtility_SYSFIN_LogMessage(AT, "inBATCH_ReviewReport_Detail_NEWUI_By_Sqlite START!");
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_ReviewReport_Detail_NEWUI_By_Sqlite_NEWUI()_START");
        }
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	/* 算出交易總筆數 */
	inFileRecCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "筆數: %d", inFileRecCnt);
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* 沒交易紀錄 */
	if (inFileRecCnt == 0)
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第一層顯示 <交易查詢> */
		return (VS_NO_RECORD);
	}

	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_ESVC_, &inESVCIndex);
	
	/* 先抓出總資料 */
	if (pobTran->srBRec.inHDTIndex == inESVCIndex)
	{
		inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_START(pobTran);
	}
	else
	{
		inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(pobTran);
	}
	/* 預設為無須重找，但往回翻時要取消 */
	guszEnormousNoNeedResetBit = VS_TRUE;
	
	inDISP_ClearAll();
	inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_1_);		/* 第一層顯示 <交易查詢> */

	if (ginHalfLCD == VS_FALSE)
		inDISP_PutGraphic(_REVIEW_DETAIL_, 0,  _COORDINATE_Y_LINE_8_8_);		/* 上一筆 下一筆 */

	while (1)
        {
		if (inNowindex >= inFileRecCnt)
		{
			uszBatchEndBit = VS_TRUE;
		}
		
		if (uszBatchEndBit == VS_TRUE	||
		    uszBatchCancelBit == VS_TRUE)
		{
                        break;
		}
		
                /* 開始讀取每一筆交易記錄 */
		if (pobTran->srBRec.inHDTIndex == inESVCIndex)
		{
			if (inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_Read(pobTran, inNowindex) != VS_SUCCESS)
			{
				uszBatchEndBit = VS_TRUE;
				break;
			}
		}
		else
		{
			if (inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(pobTran, inNowindex) != VS_SUCCESS)
			{
				uszBatchEndBit = VS_TRUE;
				break;
			}
		}
		
		inDISP_Clear_Line(_LINE_8_2_, _LINE_8_7_);

                
		
		inGetTRTFileName(szTRTFileName);
		
		/* DCC轉台幣的紀錄要特別處理，原先的那筆DCC一般交易不能出現 */
		if (pobTran->srBRec.inCode == _VOID_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)
		{
			if (inTouchPage == _DisTouch_Slide_Right_To_Left_ || szKey == _KEY_ENTER_ || szKey == _KEY_DOWN_ || szKey == _KEY_DOWN_HALF_)
			{
				inNowindex++;
				guszEnormousNoNeedResetBit = VS_TRUE;
				continue;
			}
			else if (inTouchPage == _DisTouch_Slide_Left_To_Right_ || szKey == _KEY_UP_ || szKey == _KEY_UP_HALF_)
			{
				inNowindex--;
				guszEnormousNoNeedResetBit = VS_FALSE;
				
				/* 交易紀錄編號不可小於0 */
				if (inNowindex < 0)
				{
					inNowindex = 0;
					/* 加這行是因為，若第一筆是DCC轉台幣會陷入無限迴圈，所以改滑動方向 */
					inTouchPage = _DisTouch_Slide_Right_To_Left_;
					szKey = _KEY_ENTER_;
				}
				continue;
			}
			/* 若第一筆就是要跳過的紀錄，會因為沒有event，而沒有跳過，所以預設為下一頁 */
			else
			{
				inNowindex++;
				guszEnormousNoNeedResetBit = VS_TRUE;
				continue;
			}
			
		}
		
		/* 優惠兌換的顯示比較特別 */
                if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_		||
		    pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
		{
			/* Invoice Number */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_)
			{
				sprintf(szDispMsg, "優惠兌換     %03ld", pobTran->srBRec.lnOrgInvNum);
			}
			else if (pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
			{
				sprintf(szDispMsg, "兌換取消     %03ld", pobTran->srBRec.lnOrgInvNum);
			}
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_2_, _DISP_LEFT_);

			/* Data & Time */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "%.2s/%.2s/%.2s", &pobTran->srBRec.szDate[2], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_3_, _DISP_LEFT_);
			
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "%.2s:%.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2], &pobTran->srBRec.szTime[4]);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
		}
		else if (pobTran->srBRec.inHDTIndex == inESVCIndex)
		{
			/* Disp Card Label & Expire date*/
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
			{
				strcpy(szDispMsg, " 一卡通");
			}
			else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
			{
				strcpy(szDispMsg, " 悠遊卡");
			}
			else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
			{
				strcpy(szDispMsg, " 愛金卡");
			}
			
			inDISP_ChineseFont_Point_Color(szDispMsg, _FONTSIZE_8X22_, _LINE_8_2_, _COLOR_BLACK_, _COLOR_WHITE_, 1);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "CardLabel: %s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Disp Card Number */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
			/* 卡號值 */
			if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
			{
				memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
				inLen = strlen(pobTran->srTRec.szUID);
				memcpy(szFinalPAN, pobTran->srTRec.szUID, inLen);
				szFinalPAN[inLen - 1] = 0x2A;
				szFinalPAN[inLen - 2] = 0x2A;
			}
			else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
			{
				memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
				inLen = strlen(pobTran->srTRec.szUID);
				memcpy(szFinalPAN, pobTran->srTRec.szUID, inLen);
				
                                /* 客製化098，悠遊卡因為使用T0200，加密遮掩為倒數5、6、7 */
                                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))  
                                {
                                        szFinalPAN[inLen - 5] = 0x2A;
                                        szFinalPAN[inLen - 6] = 0x2A;
                                        szFinalPAN[inLen - 7] = 0x2A;
                                }
                                else
                                {
                                        if (atoi(pobTran->srTRec.srECCRec.szPurseVersionNumber) == 0)
                                        {
                                                szFinalPAN[inLen - 5] = 0x2A;
                                                szFinalPAN[inLen - 6] = 0x2A;
                                                szFinalPAN[inLen - 7] = 0x2A;
                                        }
                                        else
                                        {
                                                /* 全部16 第6-11隱碼 */
                                                szFinalPAN[inLen - 6] = 0x2A;
                                                szFinalPAN[inLen - 7] = 0x2A;
                                                szFinalPAN[inLen - 8] = 0x2A;
                                                szFinalPAN[inLen - 9] = 0x2A;
                                                szFinalPAN[inLen - 10] = 0x2A;
                                                szFinalPAN[inLen - 11] = 0x2A;
                                        }
                                }    
			}
			else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
			{
				memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
				inLen = strlen(pobTran->srTRec.szUID);
				memcpy(szFinalPAN, pobTran->srTRec.szUID, inLen);
				/* 愛金卡：遮掩9-12 碼，遮掩字元為"*" */
				szFinalPAN[8] = 0x2A;
				szFinalPAN[9] = 0x2A;
				szFinalPAN[10] = 0x2A;
				szFinalPAN[11] = 0x2A;	
			}
			
			sprintf(szDispMsg, " %s",szFinalPAN);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "szUID: %s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Transaction Type */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			if (pobTran->srTRec.inCode == _TICKET_IPASS_DEDUCT_ || 
			    pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_ ||
			    pobTran->srTRec.inCode == _TICKET_ICASH_DEDUCT_)
				strcpy(szTemplate1, "購貨　　");
			else if (pobTran->srTRec.inCode == _TICKET_IPASS_REFUND_ || 
				 pobTran->srTRec.inCode == _TICKET_EASYCARD_REFUND_ ||
				 pobTran->srTRec.inCode == _TICKET_ICASH_REFUND_)
				strcpy(szTemplate1, "退貨　　");
			else if (pobTran->srTRec.inCode == _TICKET_IPASS_TOP_UP_ || 
				 pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_ ||
				 pobTran->srTRec.inCode == _TICKET_ICASH_TOP_UP_)
				strcpy(szTemplate1, "現金加值");  
			else if (pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_ || 
				 pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_ ||
				 pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_)
				strcpy(szTemplate1, "加值取消");              
			else if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_)
				strcpy(szTemplate1, "餘額查詢");  
			else if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_ || 
				 pobTran->srTRec.inCode == _TICKET_EASYCARD_AUTO_TOP_UP_ ||
				 pobTran->srTRec.inCode == _TICKET_ICASH_AUTO_TOP_UP_)
				strcpy(szTemplate1, "自動加值");         
			else
				strcpy(szTemplate1, "　　");

			sprintf(szDispMsg, " %s", szTemplate1);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Tran Type: %s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Data & Time */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, " 20%.2s/%.2s/%.2s %.2s:%.2s", &pobTran->srTRec.szDate[0], &pobTran->srTRec.szDate[2], &pobTran->srTRec.szDate[4], &pobTran->srTRec.szTime[0], &pobTran->srTRec.szTime[2]);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_9_, _DISP_LEFT_);
			
			/* RF序號 */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, " RF序號: %s", pobTran->srTRec.szTicketRefundCode);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_10_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Reference Number */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, " REF: %s", pobTran->srTRec.szRefNo);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_11_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Invoice Number */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, " INV: %06ld", pobTran->srTRec.lnInvNum);			
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_12_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}
			
			if ((pobTran->srTRec.uszRewardL1Bit == VS_TRUE	||
			     pobTran->srTRec.uszRewardL2Bit == VS_TRUE	||
			     pobTran->srTRec.uszRewardL5Bit == VS_TRUE)	&&
			     pobTran->srTRec.uszRewardSuspendBit != VS_TRUE)
			{
				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				sprintf(szDispMsg, "award");			
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_12_, _DISP_RIGHT_);
			}

			/* Amount */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			switch (pobTran->srTRec.inCode)
			{
				case _TICKET_IPASS_DEDUCT_ :
				case _TICKET_EASYCARD_DEDUCT_ :
				case _TICKET_ICASH_DEDUCT_ :
				case _TICKET_IPASS_REFUND_ :
				case _TICKET_EASYCARD_REFUND_:
				case _TICKET_ICASH_REFUND_ :
				case _TICKET_IPASS_TOP_UP_:
				case _TICKET_EASYCARD_TOP_UP_:
				case _TICKET_ICASH_TOP_UP_ :
				case _TICKET_IPASS_VOID_TOP_UP_:
				case _TICKET_EASYCARD_VOID_TOP_UP_:
				case _TICKET_ICASH_VOID_TOP_UP_ :
				case _TICKET_IPASS_INQUIRY_:
				case _TICKET_IPASS_AUTO_TOP_UP_:
				case _TICKET_EASYCARD_AUTO_TOP_UP_:
				case _TICKET_ICASH_AUTO_TOP_UP_ :
					sprintf(szTemplate1, "%ld", pobTran->srTRec.lnTxnAmount);
					inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
					sprintf(szDispMsg, " %s", szTemplate1);
					break;
				default :
					sprintf(szDispMsg, " AMT_VOID_ERR_(%d)", pobTran->srTRec.inCode);
					break;
			} /* End switch () */
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_13_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Store ID */
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memset(szDispMsg, 0x00, sizeof(szDispMsg));

			inGetStoreIDEnable(&szTemplate1[0]);

			if ((szTemplate1[0] == 'Y') && (strlen(pobTran->srTRec.szStoreID) > 0))
			{
				if (strlen(pobTran->srTRec.szStoreID) > 42)
					memcpy(&szTemplate1[0], &pobTran->srTRec.szStoreID[0], 42);
				else
					strcpy(szTemplate1, pobTran->srTRec.szStoreID);

				szTemplate1[18] = 0x00;

				sprintf(szDispMsg, " STD:%s", pobTran->srTRec.szStoreID);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_14_, _DISP_LEFT_);
			}
			else
			{
				/* 沒開櫃號功能， 不印櫃號 */
			}
		}
		else
		{
			/* Disp Card Label & Expire date */
			/* Disp Card Label */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			/* 【需求單 - 106349】自有品牌判斷需求 */
			/* unpack電文就存存CardLabel modify by LingHsiung 2020/2/14 上午 11:07 */
			sprintf(szDispMsg, " %s", pobTran->srBRec.szCardLabel);
			inDISP_ChineseFont_Point_Color(szDispMsg, _FONTSIZE_16X22_, _LINE_16_3_, _COLOR_BLACK_, _COLOR_WHITE_, 1);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "CardLabel: %s", pobTran->srBRec.szCardLabel);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* Expire date */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
			/* 金融卡沒有效期 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				
			}
			/* 大高卡顯示有效期 */
			else if (pobTran->srBRec.uszTakaTransBit == VS_TRUE)
			{
				sprintf(szDispMsg, "%.2s/'%.2s", pobTran->srBRec.szExpDate + 2, pobTran->srBRec.szExpDate);
			}
			/* 其他加密 */
			else
			{
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
				if (strlen(pobTran->srBRec.szCheckNO) > 0)
				{
					strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
				}
				else
				{
					inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
				}
				sprintf(szDispMsg, "%s", szTemplate1);
			}
			inDISP_ChineseFont_Point_Color(szDispMsg, _FONTSIZE_16X22_, _LINE_16_3_, _COLOR_BLACK_, _COLOR_WHITE_, 13);

			
			/* Disp Card Number */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
			strcpy(szFinalPAN, pobTran->srBRec.szPAN);

			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			sprintf(szDispMsg, " %s",szFinalPAN);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_8X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_4_, VS_FALSE, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "szPAN: %s", pobTran->srBRec.szPAN);
				inLogPrintf(AT, szDebugMsg);
			}

			
			/* Transaction Type */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memset(szTemplate2, 0x00, sizeof(szTemplate2));
			inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
			if (strlen(szTemplate2) > 0)
			{
				sprintf(szDispMsg, " %s", szTemplate1);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_6_, VS_FALSE, _DISP_LEFT_);
				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				sprintf(szDispMsg, " %s", szTemplate2);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_7_, VS_FALSE, _DISP_LEFT_);
			}
			else
			{
				sprintf(szDispMsg, " %s %s", szTemplate1, szTemplate2);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_8X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_6_, VS_FALSE, _DISP_LEFT_);
			}


			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Tran Type: %s %s", szTemplate1, szTemplate2);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Data & Time */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, " 20%.2s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[2], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_8_, VS_FALSE, _DISP_LEFT_);
			
			/* 授權碼(金卡顯示調單編號) */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				sprintf(szDispMsg, " 交易序號: %s", pobTran->srBRec.szFiscRRN);
			}
			else
			{
				sprintf(szDispMsg, " 授權碼: %s", pobTran->srBRec.szAuthCode);
			}
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_9_, VS_FALSE, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Reference Number */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, " 序號: %s", pobTran->srBRec.szRefNo);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_10_, VS_FALSE, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Invoice Number */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "  調閱編號: %06ld", pobTran->srBRec.lnOrgInvNum);
			
			/* 【需求單 - 105259】總額明細報表及總額明細查詢補強機制 商店聯出紙本要印<P> */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetESCMode(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0 && pobTran->srBRec.inESCUploadStatus == _ESC_UPLOAD_STATUS_PAPER_)
			{
				strcat(szDispMsg, " <P>");
			}
			
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_11_, VS_FALSE, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 顯示是否有優惠資訊 */
			if (inNCCC_Loyalty_ASM_Flag() != VS_SUCCESS)
			{
				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				sprintf(szDispMsg, " ");
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_11_, VS_FALSE, _DISP_RIGHT_);
			}
			/* 昇恆昌客製化，不顯示優惠資訊 */
			/* 保險公司客製化，不顯示優惠資訊 */
			else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_046_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
				 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
				 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
				 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
				 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_042_BDAU1_, _CUSTOMER_INDICATOR_SIZE_)	||
				 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
			{
				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				sprintf(szDispMsg, " ");
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_11_, VS_FALSE, _DISP_RIGHT_);
			}
			else
			{
				/* 原交易有優惠資訊，取消交易後，非小費及暫停優惠服務，則列印取消優惠資訊 */
				if (pobTran->srBRec.uszVOIDBit == VS_TRUE	&&
				   (pobTran->srBRec.uszRewardL1Bit == VS_TRUE	|| 
				    pobTran->srBRec.uszRewardL2Bit == VS_TRUE	||
				    pobTran->srBRec.uszRewardL5Bit == VS_TRUE)	&&
				    atoi(pobTran->srBRec.szAwardNum) > 0	&&
				    pobTran->srBRec.uszRewardSuspendBit != VS_TRUE)
				{
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, "void awd ");
					inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_11_, VS_FALSE, _DISP_RIGHT_);
				}
				else if (pobTran->srBRec.uszVOIDBit != VS_TRUE		&&
					(pobTran->srBRec.uszRewardL1Bit == VS_TRUE	|| 
					 pobTran->srBRec.uszRewardL2Bit == VS_TRUE	||
					 pobTran->srBRec.uszRewardL5Bit == VS_TRUE))
				{
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, "award ");
					inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_11_, VS_FALSE, _DISP_RIGHT_);
				}
				else
				{
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, " ");
					inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_11_, VS_FALSE, _DISP_RIGHT_);
				}
			
			}
			
			/* Amount */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));

			if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
			{
				switch (pobTran->srBRec.inCode)
				{
					case _SALE_ :
					case _INST_SALE_ :
					case _REDEEM_SALE_ :
					case _SALE_OFFLINE_ :
					case _FORCE_CASH_ADVANCE_ :
					case _PRE_COMP_:
					case _PRE_AUTH_ :
					case _CUP_SALE_ :
					case _CUP_PRE_COMP_ :
					case _CUP_PRE_AUTH_ :
					case _INST_ADJUST_:
					case _REDEEM_ADJUST_:
					case _MAIL_ORDER_ :
					case _CUP_MAIL_ORDER_ :
					case _CASH_ADVANCE_ :
					case _FISC_SALE_ :
						if (pobTran->srBRec.lnTipTxnAmount != 0)
							sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
						else
							sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 13, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s %s", "金額", szTemplate1);
						break;
					case _ADJUST_:
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 13, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s %s", "金額", szTemplate1);
						break;
					case _TIP_ :
						sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 13, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s %s", "金額", szTemplate1);
						break;
					case _REFUND_ :
					case _INST_REFUND_ :
					case _REDEEM_REFUND_ :
					case _CUP_REFUND_ :
					case _CUP_MAIL_ORDER_REFUND_ :
					case _FISC_REFUND_ :
						sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 13, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s %s", "金額", szTemplate1);
						break;
					default :
						sprintf(szDispMsg, " AMT_ERR_(%d)", pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			else
			{
				switch (pobTran->srBRec.inOrgCode)
				{
					case _SALE_ :
					case _INST_SALE_ :
					case _REDEEM_SALE_ :
					case _SALE_OFFLINE_ :
					case _FORCE_CASH_ADVANCE_ :
					case _PRE_COMP_:
					case _PRE_AUTH_ :
					case _CUP_SALE_ :
					case _CUP_PRE_COMP_ :
					case _CUP_PRE_AUTH_ :
					case _INST_ADJUST_:
					case _REDEEM_ADJUST_:
					case _MAIL_ORDER_ :
					case _CUP_MAIL_ORDER_ :
					case _CASH_ADVANCE_ :
					case _FISC_SALE_ :
						sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 13, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s %s", "金額", szTemplate1);
						break;
					case _ADJUST_:
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 13, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s %s", "金額", szTemplate1);
						break;
					/* 目前小費沒有取消 */
					case _REFUND_ :
					case _INST_REFUND_ :
					case _REDEEM_REFUND_ :
					case _CUP_REFUND_ :
					case _CUP_MAIL_ORDER_REFUND_ :
					/* SmartPay不能取消 */
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 13, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s %s", "金額", szTemplate1);
						break;
					default :
						sprintf(szDispMsg, " AMT_VOID_ERR_(%d)", pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_12_, VS_FALSE, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Store ID */
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memset(szDispMsg, 0x00, sizeof(szDispMsg));

			inGetStoreIDEnable(&szTemplate1[0]);

			if ( (szTemplate1[0] == 'Y') && (strlen(pobTran->srBRec.szStoreID) > 0))
			{
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
				{
					sprintf(szDispMsg, " 品群碼:");
					inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_13_, VS_FALSE, _DISP_LEFT_);
					sprintf(szDispMsg, "%s", pobTran->srBRec.szStoreID);
					inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szDispMsg, _FONTSIZE_16X33_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_REVIEW_STORE_ID_CUS_026_, _COORDINATE_Y_LINE_16_13_, VS_FALSE);
				}
				else
				{
					sprintf(szDispMsg, " 櫃號:");
					inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_13_, VS_FALSE, _DISP_LEFT_);
					sprintf(szDispMsg, "%s", pobTran->srBRec.szStoreID);
					inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szDispMsg, _FONTSIZE_16X33_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_REVIEW_STORE_ID_, _COORDINATE_Y_LINE_16_13_, VS_FALSE);
				}
			}
			else
			{
				/* 沒開櫃號功能， 不印櫃號 */
			}
			
			/* 【需求單-113172】免簽商店存根聯參數及短簽單調整需求 by Russell 2025/5/15 下午 2:41 */
			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
			{
				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				sprintf(szDispMsg, "  免簽名");
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_14_, VS_FALSE, _DISP_LEFT_);
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s", szDispMsg);
					inLogPrintf(AT, szDebugMsg);
				}
			}
		}
		
		/* 事件初始化 */
		szKey = 0;
		/* 設定Timeout */
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		
		while (1)
		{
			/* 偵測滑動 */
			inTouchPage = inDisTouch_TouchSensor_Click_Slide(_Touch_NEWUI_REVIEW_BATCH_);
			
			/* 偵測按鍵 */
			szKey = uszKBD_Key();
			
			/* TimeOut */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}
			
			if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_SUCCESS;
				uszBatchCancelBit = VS_TRUE;
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_SUCCESS;
				uszBatchCancelBit = VS_TRUE;
				break;
			}
			else if (inTouchPage == _DisTouch_Slide_Right_To_Left_			||
				 inTouchPage == _NEWUI_REVIEW_DETAIL_Touch_NEXT_DATA_BUTTON_	||
				 szKey == _KEY_ENTER_						|| 
				 szKey == _KEY_DOWN_						||
				 szKey == _KEY_DOWN_HALF_)
			{
				++inNowindex;
				guszEnormousNoNeedResetBit = VS_TRUE;
				break;
			}
			else if (inTouchPage == _DisTouch_Slide_Left_To_Right_			||
				 inTouchPage == _NEWUI_REVIEW_DETAIL_Touch_LAST_DATA_BUTTON_	||
				 szKey == _KEY_UP_						|| 
				 szKey == _KEY_UP_HALF_						||
				 szKey == _KEY_FUNCTION_)
			{
				--inNowindex;
				guszEnormousNoNeedResetBit = VS_FALSE;
				/* 交易紀錄編號不可小於0 */
				if (inNowindex < 0)
				{
					inNowindex = 0;
				}
				break;
			}

		}
		
        } /* End for () .... */
	
	if (pobTran->srBRec.inHDTIndex == inESVCIndex)
	{
		inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_END(pobTran);
	}
	else
	{
		inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(pobTran);
	}

	if (uszBatchEndBit == VS_TRUE)
	{
		/* 顯示批次結束 */
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第一層顯示 <交易查詢> */
		/* 清下排 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_ERR_BATCH_END_, 0, _COORDINATE_Y_LINE_8_6_);
		inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);
		inDISP_BEEP(1, 0);

		/* 事件初始化 */
		szKey = 0;
		/* 設定Timeout */
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);

		while (1)
		{
			/* 偵測滑動、觸控 */
			inTouchPage = inDisTouch_TouchSensor_Click_Slide(_Touch_BATCH_END_);
			/* 按鍵 */
			szKey = uszKBD_Key();

			/* TimeOut */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (inTouchPage == _BATCH_END_Touch_ENTER_BUTTON_	||
			    szKey == _KEY_ENTER_)
			{
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				break;
			}
			else
			{
				continue;
			}
		}
	}

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_ReviewReport_Detail_NEWUI_By_Sqlite_NEWUI()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (inRetVal);
}

/*
Function        :inBATCH_ReviewReport_Detail_Txno_NEWUI_By_Sqlite
Date&Time       :2017/6/12 下午 5:54
Describe        :交易查詢使用，for卡號遮掩
*/
int inBATCH_ReviewReport_Detail_Txno_NEWUI_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int		inLen = 0;
	int		inESVCIndex = -1;
	int		inRetVal = VS_SUCCESS;
	int		i = 0;
	int		inNowindex = 0;
	int		inFileRecCnt = -1;
	int		inTouchPage = 0;
	char		szTemplate1[42 + 1] = {0};
	char		szTemplate2[42 + 1] = {0};
	char		szFinalPAN[_PAN_UCARD_SIZE_ + 1] = {0};
	char		szDispMsg [50 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		szKey = 0x00;
	char		szTRTFileName[16 + 1] = {0};
	char		szFuncEnable[2 + 1] = {0};
        char            szCustomerIndicator[3 + 1] = {0};
	unsigned char	uszBatchEndBit = VS_FALSE;
	unsigned char	uszBatchCancelBit = VS_FALSE;
	
        vdUtility_SYSFIN_LogMessage(AT, "inBATCH_ReviewReport_Detail_Txno_NEWUI_By_Sqlite START!");
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_ReviewReport_Detail_Txno_NEWUI_By_Sqlite()_START");
        }
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	/* 算出交易總筆數 */
	inFileRecCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "筆數: %d", inFileRecCnt);
		inLogPrintf(AT, szDebugMsg);
	}

	/* 沒交易紀錄 */
	if (inFileRecCnt == 0)
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第一層顯示 <交易查詢> */
		return (VS_NO_RECORD);
	}
	
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_ESVC_, &inESVCIndex);
	
	/* 先抓出總資料 */
	if (pobTran->srBRec.inHDTIndex == inESVCIndex)
	{
		inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_START(pobTran);
	}
	else
	{
		inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(pobTran);
	}
	/* 預設為無須重找，但往回翻時要取消 */
	guszEnormousNoNeedResetBit = VS_TRUE;
	
	inDISP_ClearAll();
	inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_1_);		/* 第一層顯示 <交易查詢> */

	if (ginHalfLCD == VS_FALSE)
		inDISP_PutGraphic(_REVIEW_DETAIL_, 0,  _COORDINATE_Y_LINE_8_8_);		/* 上一筆 下一筆 */
	
	while (1)
        {
		if (inNowindex >= inFileRecCnt)
		{
			uszBatchEndBit = VS_TRUE;
		}
		
		if (uszBatchEndBit == VS_TRUE	||
		    uszBatchCancelBit == VS_TRUE)
		{
                        break;
		}
		
                /* 開始讀取每一筆交易記錄 */
		if (pobTran->srBRec.inHDTIndex == inESVCIndex)
		{
			if (inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_Read(pobTran, inNowindex) != VS_SUCCESS)
			{
				uszBatchEndBit = VS_TRUE;
				break;
			}
		}
		else
		{
			if (inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(pobTran, inNowindex) != VS_SUCCESS)
			{
				uszBatchEndBit = VS_TRUE;
				break;
			}
		}
		
		inDISP_Clear_Line(_LINE_8_2_, _LINE_8_7_);

		inGetTRTFileName(szTRTFileName);
		
		/* DCC轉台幣的紀錄要特別處理，原先的那筆DCC一般交易不能出現 */
		if (pobTran->srBRec.inCode == _VOID_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)
		{
			if (inTouchPage == _DisTouch_Slide_Right_To_Left_ || szKey == _KEY_ENTER_ || szKey == _KEY_DOWN_ || szKey == _KEY_DOWN_HALF_)
			{
				inNowindex++;
				guszEnormousNoNeedResetBit = VS_TRUE;
				continue;
			}
			else if (inTouchPage == _DisTouch_Slide_Left_To_Right_ || szKey == _KEY_UP_ || szKey == _KEY_UP_HALF_)
			{
				inNowindex--;
				guszEnormousNoNeedResetBit = VS_FALSE;
				/* 交易紀錄編號不可小於0 */
				if (i < 0)
				{
					i = 0;
					/* 加這行是因為，若第一筆是DCC轉台幣會陷入無限迴圈，所以改滑動方向 */
					inTouchPage = _DisTouch_Slide_Right_To_Left_;
					szKey = _KEY_ENTER_;
				}
				continue;
			}
			/* 若第一筆就是要跳過的紀錄，會因為沒有event，而沒有跳過，所以預設為下一頁 */
			else
			{
				inNowindex++;
				guszEnormousNoNeedResetBit = VS_TRUE;
				continue;
			}
			
		}
		
		/* 優惠兌換的顯示比較特別 */
                if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_		||
		    pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
		{
			/* Invoice Number */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_)
			{
				sprintf(szDispMsg, "優惠兌換     %03ld", pobTran->srBRec.lnOrgInvNum);
			}
			else if (pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
			{
				sprintf(szDispMsg, "兌換取消     %03ld", pobTran->srBRec.lnOrgInvNum);
			}
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_1_, _DISP_LEFT_);

			/* Data & Time */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "%.2s/%.2s/%.2s", &pobTran->srBRec.szDate[2], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_2_, _DISP_LEFT_);
			
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "%.2s:%.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2], &pobTran->srBRec.szTime[4]);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_3_, _DISP_LEFT_);
		}
		else if (pobTran->srBRec.inHDTIndex == inESVCIndex)
		{
			/* Disp Card Label & Expire date*/
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
			{
				strcpy(szDispMsg, " 一卡通");
			}
			else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
			{
				strcpy(szDispMsg, " 悠遊卡");
			}
			else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
			{
				strcpy(szDispMsg, " 愛金卡");
			}

			inDISP_ChineseFont_Point_Color(szDispMsg, _FONTSIZE_8X22_, _LINE_8_2_, _COLOR_BLACK_, _COLOR_WHITE_, 1);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "CardLabel: %s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Disp Card Number */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
			/* 卡號值 */
			if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
			{
				memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
				inLen = strlen(pobTran->srTRec.szUID);
				memcpy(szFinalPAN, pobTran->srTRec.szUID, inLen);
				szFinalPAN[inLen - 1] = 0x2A;
				szFinalPAN[inLen - 2] = 0x2A;
			}
			else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
			{
				memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
				inLen = strlen(pobTran->srTRec.szUID);
				memcpy(szFinalPAN, pobTran->srTRec.szUID, inLen);
				
                                /* 客製化098，悠遊卡因為使用T0200，加密遮掩為倒數5、6、7 */
                                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))  
                                {
                                        szFinalPAN[inLen - 5] = 0x2A;
                                        szFinalPAN[inLen - 6] = 0x2A;
                                        szFinalPAN[inLen - 7] = 0x2A;
                                }
                                else
                                {
                                        if (atoi(pobTran->srTRec.srECCRec.szPurseVersionNumber) == 0)
                                        {
                                                szFinalPAN[inLen - 5] = 0x2A;
                                                szFinalPAN[inLen - 6] = 0x2A;
                                                szFinalPAN[inLen - 7] = 0x2A;
                                        }
                                        else
                                        {
                                                /* 全部16 第6-11隱碼 */
                                                szFinalPAN[inLen - 6] = 0x2A;
                                                szFinalPAN[inLen - 7] = 0x2A;
                                                szFinalPAN[inLen - 8] = 0x2A;
                                                szFinalPAN[inLen - 9] = 0x2A;
                                                szFinalPAN[inLen - 10] = 0x2A;
                                                szFinalPAN[inLen - 11] = 0x2A;
                                        }
                                }    
			}
			else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
			{
				memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
				inLen = strlen(pobTran->srTRec.szUID);
				memcpy(szFinalPAN, pobTran->srTRec.szUID, inLen);
				/* 愛金卡：遮掩9-12 碼，遮掩字元為"*" */
				szFinalPAN[8] = 0x2A;
				szFinalPAN[9] = 0x2A;
				szFinalPAN[10] = 0x2A;
				szFinalPAN[11] = 0x2A;	
			}
			
			sprintf(szDispMsg, " %s",szFinalPAN);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "szUID: %s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Transaction Type */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			if (pobTran->srTRec.inCode == _TICKET_IPASS_DEDUCT_ || 
			    pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_ ||
			    pobTran->srTRec.inCode == _TICKET_ICASH_DEDUCT_)
				strcpy(szTemplate1, "購貨　　");
			else if (pobTran->srTRec.inCode == _TICKET_IPASS_REFUND_ || 
				 pobTran->srTRec.inCode == _TICKET_EASYCARD_REFUND_ ||
				 pobTran->srTRec.inCode == _TICKET_ICASH_REFUND_)
				strcpy(szTemplate1, "退貨　　");
			else if (pobTran->srTRec.inCode == _TICKET_IPASS_TOP_UP_ || 
				 pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_ ||
				 pobTran->srTRec.inCode == _TICKET_ICASH_TOP_UP_)
				strcpy(szTemplate1, "現金加值");  
			else if (pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_ || 
				 pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_ ||
				 pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_)
				strcpy(szTemplate1, "加值取消");              
			else if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_)
				strcpy(szTemplate1, "餘額查詢");  
			else if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_ || 
				 pobTran->srTRec.inCode == _TICKET_EASYCARD_AUTO_TOP_UP_ ||
				 pobTran->srTRec.inCode == _TICKET_ICASH_AUTO_TOP_UP_)
				strcpy(szTemplate1, "自動加值");         
			else
				strcpy(szTemplate1, "　　");

			sprintf(szDispMsg, " %s", szTemplate1);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Tran Type: %s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Data & Time */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, " 20%.2s/%.2s/%.2s %.2s:%.2s", &pobTran->srTRec.szDate[0], &pobTran->srTRec.szDate[2], &pobTran->srTRec.szDate[4], &pobTran->srTRec.szTime[0], &pobTran->srTRec.szTime[2]);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_9_, _DISP_LEFT_);
			
			/* RF序號 */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, " RF序號: %s", pobTran->srTRec.szTicketRefundCode);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_10_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Reference Number */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, " REF: %s", pobTran->srTRec.szRefNo);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_11_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Invoice Number */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, " INV: %06ld", pobTran->srTRec.lnInvNum);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_12_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Amount */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			switch (pobTran->srTRec.inCode)
			{
				case _TICKET_IPASS_DEDUCT_ :
				case _TICKET_EASYCARD_DEDUCT_ :
				case _TICKET_ICASH_DEDUCT_ :
				case _TICKET_IPASS_REFUND_ :
				case _TICKET_EASYCARD_REFUND_:
				case _TICKET_ICASH_REFUND_:
				case _TICKET_IPASS_TOP_UP_:
				case _TICKET_EASYCARD_TOP_UP_:
				case _TICKET_ICASH_TOP_UP_:
				case _TICKET_IPASS_VOID_TOP_UP_:
				case _TICKET_EASYCARD_VOID_TOP_UP_:
				case _TICKET_ICASH_VOID_TOP_UP_:
				case _TICKET_IPASS_INQUIRY_:
				case _TICKET_IPASS_AUTO_TOP_UP_:
				case _TICKET_EASYCARD_AUTO_TOP_UP_:
				case _TICKET_ICASH_AUTO_TOP_UP_:
					sprintf(szTemplate1, "%ld", pobTran->srTRec.lnTxnAmount);
					inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
					sprintf(szDispMsg, " %s", szTemplate1);
					break;
				default :
					sprintf(szDispMsg, " AMT_VOID_ERR_(%d)", pobTran->srTRec.inCode);
					break;
			} /* End switch () */
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_13_, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Store ID */
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memset(szDispMsg, 0x00, sizeof(szDispMsg));

			inGetStoreIDEnable(&szTemplate1[0]);

			if ( (szTemplate1[0] == 'Y') && (strlen(pobTran->srTRec.szStoreID) > 0))
			{
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
				{
					sprintf(szDispMsg, " 品群碼:");
					inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_14_, _DISP_LEFT_);
					sprintf(szDispMsg, "%s", pobTran->srBRec.szStoreID);
					inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szDispMsg, _FONTSIZE_16X33_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_REVIEW_STORE_ID_CUS_026_, _COORDINATE_Y_LINE_16_14_, VS_FALSE);
				}
				else
				{
					sprintf(szDispMsg, " 櫃號:");
					inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_14_, _DISP_LEFT_);
					sprintf(szDispMsg, "%s", pobTran->srBRec.szStoreID);
					inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szDispMsg, _FONTSIZE_16X33_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_REVIEW_STORE_ID_, _COORDINATE_Y_LINE_16_14_, VS_FALSE);
				}
			}
			else
			{
				/* 沒開櫃號功能， 不印櫃號 */
			}
		}
		else
		{
			/* Disp Card Label & Expire date */
			/* Disp Card Label */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			/* 【需求單 - 106349】自有品牌判斷需求 */
			/* unpack電文就存存CardLabel modify by LingHsiung 2020/2/14 上午 11:07 */
			sprintf(szDispMsg, " %s", pobTran->srBRec.szCardLabel);
			inDISP_ChineseFont_Point_Color(szDispMsg, _FONTSIZE_16X22_, _LINE_16_3_, _COLOR_BLACK_, _COLOR_WHITE_, 1);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "CardLabel: %s", pobTran->srBRec.szCardLabel);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* Expire date */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
			/* 金融卡沒有效期 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				
			}
			/* 大高卡顯示有效期 */
			else if (pobTran->srBRec.uszTakaTransBit == VS_TRUE)
			{
				sprintf(szDispMsg, "%.2s/'%.2s", pobTran->srBRec.szExpDate + 2, pobTran->srBRec.szExpDate);
			}
			else
			{
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
				if (strlen(pobTran->srBRec.szCheckNO) > 0)
				{
					strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
				}
				else
				{
					inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
				}
				sprintf(szDispMsg, "%s", szTemplate1);
			}
			inDISP_ChineseFont_Point_Color(szDispMsg, _FONTSIZE_16X22_, _LINE_16_3_, _COLOR_BLACK_, _COLOR_WHITE_, 13);

			/* Disp Card Number */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
			strcpy(szFinalPAN, pobTran->srBRec.szPAN);

			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			/* 卡號遮掩 */
			/* DFS需求不再使用大來主機 2018/5/8 下午 2:10 by Rusell */
			for (i = 6; i < (strlen(szFinalPAN) - 4); i ++)
				szFinalPAN[i] = 0x2A;
			
			sprintf(szDispMsg, " %s",szFinalPAN);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_8X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_4_, VS_FALSE, _DISP_LEFT_);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "szPAN: %s", pobTran->srBRec.szPAN);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Transaction Type */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memset(szTemplate2, 0x00, sizeof(szTemplate2));
			inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
			if (strlen(szTemplate2) > 0)
			{
				sprintf(szDispMsg, " %s", szTemplate1);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_6_, VS_FALSE, _DISP_LEFT_);
				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				sprintf(szDispMsg, " %s", szTemplate2);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_7_, VS_FALSE, _DISP_LEFT_);
			}
			else
			{
				sprintf(szDispMsg, " %s %s", szTemplate1, szTemplate2);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_8X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_6_, VS_FALSE, _DISP_LEFT_);
			}


			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Tran Type: %s %s", szTemplate1, szTemplate2);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Data & Time */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, " 20%.2s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[2], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_8_, VS_FALSE, _DISP_LEFT_);
			
			/* 授權碼(金卡顯示調單編號) */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				sprintf(szDispMsg, " 交易序號: %s", pobTran->srBRec.szFiscRRN);
			}
			else
			{
				sprintf(szDispMsg, " 授權碼: %s", pobTran->srBRec.szAuthCode);
			}
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_9_, VS_FALSE, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Reference Number */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, " 序號: %s", pobTran->srBRec.szRefNo);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_10_, VS_FALSE, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Invoice Number */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "  調閱編號: %06ld", pobTran->srBRec.lnOrgInvNum);
			
			/* 【需求單 - 105259】總額明細報表及總額明細查詢補強機制 商店聯出紙本要印<P> */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetESCMode(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0 && pobTran->srBRec.inESCUploadStatus == _ESC_UPLOAD_STATUS_PAPER_)
			{
				strcat(szDispMsg, " <P>");
			}
			
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_11_, VS_FALSE, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 顯示是否有優惠資訊 */
			if (inNCCC_Loyalty_ASM_Flag() != VS_SUCCESS)
			{
				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				sprintf(szDispMsg, " ");
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_11_, VS_FALSE, _DISP_RIGHT_);
			}
			/* 昇恆昌客製化，不顯示優惠資訊 */
			/* 保險公司客製化，不顯示優惠資訊 */
			else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_046_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
				 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
				 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
				 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
				 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_042_BDAU1_, _CUSTOMER_INDICATOR_SIZE_)	||
				 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
			{
				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				sprintf(szDispMsg, " ");
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_11_, VS_FALSE, _DISP_RIGHT_);
			}
			else
			{
				/* 原交易有優惠資訊，取消交易後，非小費及暫停優惠服務，則列印取消優惠資訊 */
				if (pobTran->srBRec.uszVOIDBit == VS_TRUE	&&
				   (pobTran->srBRec.uszRewardL1Bit == VS_TRUE	|| 
				    pobTran->srBRec.uszRewardL2Bit == VS_TRUE	||
				    pobTran->srBRec.uszRewardL5Bit == VS_TRUE)	&&
				    atoi(pobTran->srBRec.szAwardNum) > 0	&&
				    pobTran->srBRec.uszRewardSuspendBit != VS_TRUE)
				{
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, "void awd ");
					inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_11_, VS_FALSE, _DISP_RIGHT_);
				}
				else if (pobTran->srBRec.uszVOIDBit != VS_TRUE		&&
					(pobTran->srBRec.uszRewardL1Bit == VS_TRUE	|| 
					 pobTran->srBRec.uszRewardL2Bit == VS_TRUE	||
					 pobTran->srBRec.uszRewardL5Bit == VS_TRUE))
				{
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, "award ");
					inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_11_, VS_FALSE, _DISP_RIGHT_);
				}
				else
				{
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, " ");
					inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_11_, VS_FALSE, _DISP_RIGHT_);
				}
			
			}
			
			/* Amount */
			memset(szDispMsg, 0x00, sizeof(szDispMsg));

			if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
			{
				switch (pobTran->srBRec.inCode)
				{
					case _SALE_ :
					case _INST_SALE_ :
					case _REDEEM_SALE_ :
					case _SALE_OFFLINE_ :
					case _FORCE_CASH_ADVANCE_ :
					case _PRE_COMP_:
					case _PRE_AUTH_ :
					case _CUP_SALE_ :
					case _CUP_PRE_COMP_ :
					case _CUP_PRE_AUTH_ :
					case _INST_ADJUST_:
					case _REDEEM_ADJUST_:
					case _MAIL_ORDER_ :
					case _CUP_MAIL_ORDER_ :
					case _CASH_ADVANCE_ :
					case _FISC_SALE_ :
						if (pobTran->srBRec.lnTipTxnAmount != 0)
							sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
						else
							sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 13, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s %s", "金額", szTemplate1);
						break;
					case _ADJUST_:
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 13, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s %s", "金額", szTemplate1);
						break;
					case _TIP_ :
						sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 13, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s %s", "金額", szTemplate1);
						break;
					case _REFUND_ :
					case _INST_REFUND_ :
					case _REDEEM_REFUND_ :
					case _CUP_REFUND_ :
					case _CUP_MAIL_ORDER_REFUND_ :
					case _FISC_REFUND_ :
						sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 13, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s %s", "金額", szTemplate1);
						break;
					default :
						sprintf(szDispMsg, " AMT_ERR_(%d)", pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			else
			{
				switch (pobTran->srBRec.inOrgCode)
				{
					case _SALE_ :
					case _INST_SALE_ :
					case _REDEEM_SALE_ :
					case _SALE_OFFLINE_ :
					case _FORCE_CASH_ADVANCE_ :
					case _PRE_COMP_:
					case _PRE_AUTH_ :
					case _CUP_SALE_ :
					case _CUP_PRE_COMP_ :
					case _CUP_PRE_AUTH_ :
					case _INST_ADJUST_:
					case _REDEEM_ADJUST_:
					case _MAIL_ORDER_ :
					case _CUP_MAIL_ORDER_ :
					case _CASH_ADVANCE_ :
					case _FISC_SALE_ :
						sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 13, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s %s", "金額", szTemplate1);
						break;
					case _ADJUST_:
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 13, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s %s", "金額", szTemplate1);
						break;
					/* 目前小費沒有取消 */
					case _REFUND_ :
					case _INST_REFUND_ :
					case _REDEEM_REFUND_ :
					case _CUP_REFUND_ :
					case _CUP_MAIL_ORDER_REFUND_ :
					/* SmartPay不能取消 */
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 13, _PADDING_RIGHT_);
						sprintf(szDispMsg, " %s %s", "金額", szTemplate1);
						break;
					default :
						sprintf(szDispMsg, " AMT_VOID_ERR_(%d)", pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_12_, VS_FALSE, _DISP_LEFT_);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%s", szDispMsg);
				inLogPrintf(AT, szDebugMsg);
			}

			/* Store ID */
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memset(szDispMsg, 0x00, sizeof(szDispMsg));

			inGetStoreIDEnable(&szTemplate1[0]);

			if ( (szTemplate1[0] == 'Y') && (strlen(pobTran->srBRec.szStoreID) > 0))
			{
				strcpy(szTemplate1, pobTran->srBRec.szStoreID);
				szTemplate1[18] = 0x00;
				
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
				{
					sprintf(szDispMsg, " 品群碼:");
					inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_13_, VS_FALSE, _DISP_LEFT_);
					sprintf(szDispMsg, "%s", pobTran->srBRec.szStoreID);
					inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szDispMsg, _FONTSIZE_16X33_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_REVIEW_STORE_ID_CUS_026_, _COORDINATE_Y_LINE_16_13_, VS_FALSE);
				}
				else
				{
					sprintf(szDispMsg, " 櫃號:");
					inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_13_, VS_FALSE, _DISP_LEFT_);
					sprintf(szDispMsg, "%s", pobTran->srBRec.szStoreID);
					inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szDispMsg, _FONTSIZE_16X33_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_REVIEW_STORE_ID_, _COORDINATE_Y_LINE_16_13_, VS_FALSE);
				}
			}
			else
			{
				/* 沒開櫃號功能， 不印櫃號 */
			}
			
			/* 【需求單-113172】免簽商店存根聯參數及短簽單調整需求 by Russell 2025/5/15 下午 2:41 */
			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
			{
				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				sprintf(szDispMsg, "  免簽名");
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(szDispMsg, _FONTSIZE_16X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_16_14_, VS_FALSE, _DISP_LEFT_);
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s", szDispMsg);
					inLogPrintf(AT, szDebugMsg);
				}
			}
		}
		
		/* 事件初始化 */
		szKey = 0;
		/* 設定Timeout */
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		
		while (1)
		{
			
			/* 偵測滑動 */
			inTouchPage = inDisTouch_TouchSensor_Click_Slide(_Touch_NEWUI_REVIEW_BATCH_);
			
			/* 偵測按鍵 */
			szKey = uszKBD_Key();
			
			/* TimeOut */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}
			
			if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_SUCCESS;
				uszBatchCancelBit = VS_TRUE;
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_SUCCESS;
				uszBatchCancelBit = VS_TRUE;
				break;
			}
			else if (inTouchPage == _DisTouch_Slide_Right_To_Left_			||
				 inTouchPage == _NEWUI_REVIEW_DETAIL_Touch_NEXT_DATA_BUTTON_	||
				 szKey == _KEY_ENTER_						|| 
				 szKey == _KEY_DOWN_						||
				 szKey == _KEY_DOWN_HALF_)
			{
				++inNowindex;
				guszEnormousNoNeedResetBit = VS_TRUE;
				break;
			}
			else if (inTouchPage == _DisTouch_Slide_Left_To_Right_			||
				 inTouchPage == _NEWUI_REVIEW_DETAIL_Touch_LAST_DATA_BUTTON_	||
				 szKey == _KEY_UP_						|| 
				 szKey == _KEY_UP_HALF_						||
				 szKey == _KEY_FUNCTION_)
			{
				--inNowindex;
				guszEnormousNoNeedResetBit = VS_FALSE;
				/* 交易紀錄編號不可小於0 */
				if (inNowindex < 0)
				{
					inNowindex = 0;
				}
				break;
			}
		}
		
        } /* End for () .... */
	
	/* 開始讀取每一筆交易記錄 */
	if (pobTran->srBRec.inHDTIndex == inESVCIndex)
	{
		inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_END(pobTran);
	}
	else
	{
		inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(pobTran);
	}
	
	
	if (uszBatchEndBit == VS_TRUE)
	{
		/* 顯示批次結束 */
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第一層顯示 <交易查詢> */
		/* 清下排 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_ERR_BATCH_END_, 0, _COORDINATE_Y_LINE_8_6_);
		inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);
		inDISP_BEEP(1, 0);

		/* 事件初始化 */
		szKey = 0;
		/* 設定Timeout */
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);

		while (1)
		{
			/* 偵測滑動、觸控 */
			inTouchPage = inDisTouch_TouchSensor_Click_Slide(_Touch_BATCH_END_);
			/* 按鍵 */
			szKey = uszKBD_Key();

			/* TimeOut */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (inTouchPage == _BATCH_END_Touch_ENTER_BUTTON_	||
			    szKey == _KEY_ENTER_)
			{
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				break;
			}
			else
			{
				continue;
			}
		}
	}

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_ReviewReport_Detail_Txno_NEWUI_By_Sqlite()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}


/*
Function        :inBATCH_GetDetailRecords_By_Sqlite
Date&Time       :2017/3/15 下午 3:20
Describe        :抓資料庫中uszUpdate為0的資料
*/
int inBATCH_GetDetailRecords_By_Sqlite(TRANSACTION_OBJECT *pobTran, int inStartCnt)
{
	int	inRetVal;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GetDetailRecords_By_Sqlite()_START");
        }

	
	inRetVal = inSqlite_Get_Batch_ByCnt_Flow(pobTran, _TN_BATCH_TABLE_, inStartCnt);
	if (inRetVal != VS_SUCCESS)
		return (inRetVal);
	
	if (pobTran->srBRec.inChipStatus != 0			|| 
	    pobTran->srBRec.uszContactlessBit == VS_TRUE	|| 
	    pobTran->srBRec.uszEMVFallBackBit == VS_TRUE        ||
            pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
	{
		inRetVal = inSqlite_Get_Batch_ByCnt_Flow(pobTran, _TN_EMV_TABLE_, pobTran->srBRec.lnOrgInvNum);
		if (inRetVal != VS_SUCCESS)
			return (inRetVal);
	}
	

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GetDetailRecords_By_Sqlite()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (inRetVal);
}

/*
Function        :inBATCH_Get_Batch_ByCnt_Enormous_Flow
Date&Time       :2017/3/21 下午 1:18
Describe        :查詢大量紀錄使用，最後一定要call free
*/
int inBATCH_Get_Batch_ByCnt_Enormous_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inRecCnt, int inState)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	char	szSql[300 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};
	char	szCustomIndicator[3 + 1] = {0};
	SQLITE_ALL_TABLE	srAll;
	
	memset(szCustomIndicator, 0x00, sizeof(szCustomIndicator));
	inGetCustomIndicator(szCustomIndicator);
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_TICKET_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		default :
			break;
	}
	
	if (inState == _BYCNT_ENORMMOUS_READ_)
	{
		switch (inTableType)
		{
			case _TN_BATCH_TABLE_:
			case _TN_BATCH_TABLE_ESC_AGAIN_:
			case _TN_BATCH_TABLE_ESC_FAIL_:
				/* 將pobTran變數pointer位置放到Table中 */
				memset(&srAll, 0x00, sizeof(srAll));
				inRetVal = inSqlite_Table_Link_BRec(pobTran, &srAll, _LS_READ_);
				if (inRetVal != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "Table Link 失敗");
						inLogPrintf(AT, szDebugMsg);
					}

					return (VS_ERROR);
				}

				/* 替換資料前先清空srBRec */
				memset(&pobTran->srBRec, 0x00, sizeof(pobTran->srBRec));
				break;
			case _TN_BATCH_TABLE_TICKET_:
				/* 將pobTran變數pointer位置放到Table中 */
				memset(&srAll, 0x00, sizeof(srAll));
				inRetVal = inNCCC_Ticket_Table_Link_TRec(pobTran, &srAll, _LS_READ_);
				if (inRetVal != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "Table Link 失敗");
						inLogPrintf(AT, szDebugMsg);
					}

					return (VS_ERROR);
				}

				/* 替換資料前先清空srTRec */
				memset(&pobTran->srTRec, 0x00, sizeof(pobTran->srTRec));
				break;
			default :
				return (VS_ERROR);
				break;
		}
	}
	
	if (inState == _BYCNT_ENORMMOUS_SEARCH_)
	{
		switch (inTableType)
		{
			case _TN_BATCH_TABLE_:
			case _TN_BATCH_TABLE_ESC_AGAIN_:
			case _TN_BATCH_TABLE_ESC_FAIL_:
				/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/5/20 下午 2:26
				* 客製化075交易查詢要倒著 */
				if (!memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
				   !memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
				{
				       if (pobTran->inRunOperationID == _OPERATION_REVIEW_DETAIL_)
				       {
					       memset(szSql, 0x00, sizeof(szSql));
					       sprintf(szSql, "SELECT * FROM %s WHERE (uszUpdated = 0)ORDER BY lnOrgInvNum DESC", szTableName);
					       inRetVal = inSqlite_Get_Data_Enormous_Search(gszTranDBPath, szSql);
				       }
				       else
				       {
					       memset(szSql, 0x00, sizeof(szSql));
					       sprintf(szSql, "SELECT * FROM %s WHERE (uszUpdated = 0)ORDER BY lnOrgInvNum ASC", szTableName);
					       inRetVal = inSqlite_Get_Data_Enormous_Search(gszTranDBPath, szSql);
				       }
				}
				else
				{
					memset(szSql, 0x00, sizeof(szSql));
					sprintf(szSql, "SELECT * FROM %s WHERE (uszUpdated = 0)ORDER BY lnOrgInvNum ASC", szTableName);
					inRetVal = inSqlite_Get_Data_Enormous_Search(gszTranDBPath, szSql);
				}
				break;
			case _TN_BATCH_TABLE_TICKET_:
				/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/5/20 下午 2:26
				* 客製化075交易查詢要倒著 */
				if (!memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
				   !memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
				{
				       if (pobTran->inRunOperationID == _OPERATION_REVIEW_DETAIL_)
				       {
					       memset(szSql, 0x00, sizeof(szSql));
					       sprintf(szSql, "SELECT * FROM %s WHERE (uszUpdated = 0)ORDER BY lnInvNum DESC", szTableName);
					       inRetVal = inSqlite_Get_Data_Enormous_Search(gszTranDBPath, szSql);
				       }
				       else
				       {
					       memset(szSql, 0x00, sizeof(szSql));
					       sprintf(szSql, "SELECT * FROM %s WHERE (uszUpdated = 0)ORDER BY lnInvNum ASC", szTableName);
					       inRetVal = inSqlite_Get_Data_Enormous_Search(gszTranDBPath, szSql);
				       }
				}
				else
				{
					memset(szSql, 0x00, sizeof(szSql));
					sprintf(szSql, "SELECT * FROM %s WHERE (uszUpdated = 0)ORDER BY lnInvNum ASC", szTableName);
					inRetVal = inSqlite_Get_Data_Enormous_Search(gszTranDBPath, szSql);
				}
				break;
			default :
				return (VS_ERROR);
				break;
		}
	}
	else if (inState == _BYCNT_ENORMMOUS_READ_)
	{
		inRetVal = inSqlite_Get_Data_Enormous_Get(&srAll, inRecCnt);
	}
	else if (inState == _BYCNT_ENORMMOUS_FREE_)
	{
		inRetVal = inSqlite_Get_Data_Enormous_Free();
	}
	else
	{
		return (VS_ERROR);
	}
	
	return (inRetVal);
}

/*
Function        :inBATCH_GetDetailRecords_By_Sqlite_Enormous_START
Date&Time       :2017/3/21 下午 5:16
Describe        :抓資料庫中uszUpdate為0的資料
*/
int inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GetDetailRecords_By_Sqlite_Enormous_START()_START");
        }
	
	inRetVal = inBATCH_Get_Batch_ByCnt_Enormous_Flow(pobTran, _TN_BATCH_TABLE_, 0 ,_BYCNT_ENORMMOUS_SEARCH_);
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GetDetailRecords_By_Sqlite_Enormous_START()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (inRetVal);
}

/*
Function        :inBATCH_GetDetailRecords_By_Sqlite_Enormous_START_Cus_075
Date&Time       :2022/5/24 下午 2:05
Describe        :075要倒著抓資料
*/
int inBATCH_GetDetailRecords_By_Sqlite_Enormous_START_Cus_075(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GetDetailRecords_By_Sqlite_Enormous_START_Cus_075()_START");
        }
	
	inRetVal = inBATCH_Get_Batch_ByCnt_Enormous_Flow(pobTran, _TN_BATCH_TABLE_, 0 ,_BYCNT_ENORMMOUS_SEARCH_);
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GetDetailRecords_By_Sqlite_Enormous_START_Cus_075()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (inRetVal);
}

/*
Function        :inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read
Date&Time       :2017/3/21 下午 5:16
Describe        :抓資料庫中uszUpdate為0的資料
*/
int inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(TRANSACTION_OBJECT *pobTran, int inStartCnt)
{
	int	inRetVal;
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read()_START");
        }
	
	inRetVal = inBATCH_Get_Batch_ByCnt_Enormous_Flow(pobTran, _TN_BATCH_TABLE_, inStartCnt ,_BYCNT_ENORMMOUS_READ_);
	if (inRetVal != VS_SUCCESS)
		return (inRetVal);
	
	if (pobTran->srBRec.inChipStatus != 0			|| 
	    pobTran->srBRec.uszContactlessBit == VS_TRUE	|| 
	    pobTran->srBRec.uszEMVFallBackBit == VS_TRUE        ||
            pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
	{
		inRetVal = inSqlite_Get_Batch_ByCnt_Flow(pobTran, _TN_EMV_TABLE_, 0);
		if (inRetVal != VS_SUCCESS)
			return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (inRetVal);
}

/*
Function        :inBATCH_GetDetailRecords_By_Sqlite_Enormous_END
Date&Time       :2017/3/21 下午 5:16
Describe        :抓資料庫中uszUpdate為0的資料
*/
int inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GetDetailRecords_By_Sqlite_Enormous_END()_START");
        }
	
	inRetVal = inBATCH_Get_Batch_ByCnt_Enormous_Flow(pobTran, _TN_BATCH_TABLE_, 0 ,_BYCNT_ENORMMOUS_FREE_);
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GetDetailRecords_By_Sqlite_Enormous_END()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (inRetVal);
}

/*
Function        :inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_START
Date&Time       :2017/3/21 下午 5:16
Describe        :抓資料庫中uszUpdate為0的資料
*/
int inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_START(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_START()_START");
        }
	
	inRetVal = inBATCH_Get_Batch_ByCnt_Enormous_Flow(pobTran, _TN_BATCH_TABLE_TICKET_, 0 ,_BYCNT_ENORMMOUS_SEARCH_);
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_START()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (inRetVal);
}

/*
Function        :inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_Read
Date&Time       :2017/3/21 下午 5:16
Describe        :抓資料庫中uszUpdate為0的資料
*/
int inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_Read(TRANSACTION_OBJECT *pobTran, int inStartCnt)
{
	int	inRetVal;
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_Read()_START");
        }
	
	inRetVal = inBATCH_Get_Batch_ByCnt_Enormous_Flow(pobTran, _TN_BATCH_TABLE_TICKET_, inStartCnt ,_BYCNT_ENORMMOUS_READ_);
	if (inRetVal != VS_SUCCESS)
		return (inRetVal);
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_Read()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (inRetVal);
}

/*
Function        :inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_END
Date&Time       :2017/3/21 下午 5:16
Describe        :抓資料庫中uszUpdate為0的資料
*/
int inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_END(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_END()_START");
        }
	
	inRetVal = inBATCH_Get_Batch_ByCnt_Enormous_Flow(pobTran, _TN_BATCH_TABLE_TICKET_, 0 ,_BYCNT_ENORMMOUS_FREE_);
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GetDetailRecords_By_Sqlite_ESVC_Enormous_END()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (inRetVal);
}

/*
Function        :inBATCH_FuncUserChoice_By_Sqlite_ESVC
Date&Time       :2017/3/15 下午 4:48
Describe        :
*/
int inBATCH_FuncUserChoice_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
        int	inRetVal = VS_SUCCESS;
	int	inTempRetVal = VS_SUCCESS;
	int	inDCCIndex = -1;
	int	inNCCCIndex = -1;
	int	inESVCIndex = -1;
	int	inHGIndex = -1;
        int     inTRUSTIndex = -1;
	int	inOrgIndex = -1;
	int	inMaxOrgInvNum_NCCC = -1;
	int	inMaxOrgInvNum_DCC = -1;
	int	inMaxOrgInvNum_HG = -1;
	int	inMAXOrgInvNum_Total = -1;
	int	inHDTIndex_Final = -1;
	char	szCustomerIndicator[3 + 1] = {0};
	char	szTemplate[50 + 1] = {0};
	char	szQuerySql[200 + 1] = {0};
	char	szTableName[30 + 1] = {0};
	unsigned char	uszUnionHostMode1 = VS_FALSE;	/* 是否要合併Host檢查 選DCC主機若為轉台幣要自動轉向 */
	unsigned char	uszUnionHostMode2 = VS_FALSE;	/* 是否要合併Host檢查 昇恆昌要NCCC、DCC、HG主機合併 */
	SQLITE_ALL_TABLE	srAll;
	TRANSACTION_OBJECT	pobDataTran;

        vdUtility_SYSFIN_LogMessage(AT, "inBATCH_FuncUserChoice_By_Sqlite START!");
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_FuncUserChoice_By_Sqlite()_START");
        }
	
	/* NCCC DCC的特殊處理，當端末機收到收銀機要取消DCC HOST交易的電文時，
	 * 如果在DCC的帳務中查不到這一筆交易(表示交易改為台幣支付)端末機就自動在去找NCCC的帳務來解決銀行別錯誤而取消不到的問題。 START
	 */
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_DCC_, &inDCCIndex);
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inNCCCIndex);
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_ESVC_, &inESVCIndex);
        inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_TRUST_, &inTRUSTIndex);
	/* NCCC DCC的特殊處理，當端末機收到收銀機要取消DCC HOST交易的電文時，
	 * 如果在DCC的帳務中查不到這一筆交易(表示交易改為台幣支付)端末機就自動在去找NCCC的帳務來解決銀行別錯誤而取消不到的問題。 END
	 */
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/5/20 下午 2:26
	 * 客製化075，取消點選NCCC，若該筆紀錄為DCC，要能自動切換 */
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)         ||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
	{
		uszUnionHostMode2 = VS_TRUE;
	}
	else
	{
		/* NCCC DCC的特殊處理，當端末機收到收銀機要取消DCC HOST交易的電文時，
		* 如果在DCC的帳務中查不到這一筆交易(表示交易改為台幣支付)端末機就自動在去找NCCC的帳務來解決銀行別錯誤而取消不到的問題。 START
		*/
		/* 這個情形只有在用POS機時才會出現 */
		if (pobTran->uszECRBit == VS_TRUE)
		{
			if (pobTran->srBRec.inHDTIndex == inDCCIndex)
			{
				uszUnionHostMode1 = VS_TRUE;
			}
		}
	}
	
	/* 紀錄原主機 */
	inOrgIndex = pobTran->srBRec.inHDTIndex;
	do
	{
		/* 取調閱編號流程 */
		if (pobTran->srBRec.inHDTIndex == inESVCIndex)
		{
			/* ECR若回傳不是最後一筆提示錯誤 */
			if (pobTran->uszECRBit == VS_TRUE)
			{
				/* 檢核是否為最後一筆 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetInvoiceNum(szTemplate);

				/* invoice Num減一即最後一筆 */
				if ((atoi(szTemplate) - 1) != pobTran->srBRec.lnOrgInvNum)
				{
					/* 此筆無法重印 */
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, "");
					srDispMsgObj.inDispPic1YPosition = 0;
					srDispMsgObj.inMsgType = _NO_KEY_MSG_;
					srDispMsgObj.inTimeout = 3;
					strcpy(srDispMsgObj.szErrMsg1, "只能重印最後一筆");
					srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
					inDISP_Msg_BMP(&srDispMsgObj);

					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
					inRetVal = VS_ERROR;
					break;
				}
				else
				{
					/* 最後一筆可以印 */
				}
			}
			else
			{
				pobTran->srBRec.lnOrgInvNum = _BATCH_LAST_RECORD_; /* 表示是最後一筆 */
			}
		}
                else if (pobTran->srBRec.inHDTIndex == inTRUSTIndex)
		{
			pobTran->srBRec.lnOrgInvNum = _BATCH_LAST_RECORD_; /* 表示是最後一筆 */
		}
		else
		{
			/* 如果lnOrgInvNum有值，就不需要做輸入的動作(ECR發動) */
			if (pobTran->srBRec.lnOrgInvNum == 0)
			{
				if (inBATCH_GetInvoiceNumber(pobTran) == VS_ERROR)
				{
					inRetVal = VS_ERROR;
					break;
				}

				/* 因為EDC如果按確認鍵，等於調出上一筆記錄 */
				if (pobTran->srBRec.lnOrgInvNum == 0)
					pobTran->srBRec.lnOrgInvNum = _BATCH_LAST_RECORD_; /* 表示是最後一筆 */
			}
		}
		
		/* 是否有合併Host需求 */
		/* NCCC DCC合併
		 * 找DCC如果發現是已取消，要自動轉成NCCC */
		if (uszUnionHostMode1 == VS_TRUE)
		{
			memset(&pobDataTran, 0x00, sizeof(TRANSACTION_OBJECT));
			memcpy(&pobDataTran, pobTran, sizeof(TRANSACTION_OBJECT));
			inTempRetVal = inBATCH_GetTransRecord_By_Sqlite(&pobDataTran);
			if (inTempRetVal == VS_SUCCESS)
			{
				/* 是轉台幣 */
				if (pobDataTran.srBRec.uszNCCCDCCRateBit == VS_TRUE)
				{
					pobDataTran.srBRec.inHDTIndex = inNCCCIndex;
					inTempRetVal = inBATCH_GetTransRecord_By_Sqlite(&pobDataTran);
					if (inTempRetVal == VS_SUCCESS)
					{
						/* NCCC找到有帳，轉換host */
						pobTran->srBRec.inHDTIndex = inNCCCIndex;
					}
				}
			}
			
		}
		/* NCCC DCC HG 合併 */
		else if (uszUnionHostMode2 == VS_TRUE)
		{
			if (pobTran->srBRec.lnOrgInvNum == _BATCH_LAST_RECORD_)
			{
				/* 取NCCC */
				if (inNCCCIndex >= 0)
				{
					memset(&srAll, 0x00, sizeof(SQLITE_ALL_TABLE));
					memset(&pobDataTran, 0x00, sizeof(TRANSACTION_OBJECT));
					memset(szTableName, 0x00, sizeof(szTableName));
					pobDataTran.srBRec.inHDTIndex = inNCCCIndex;
					inLoadHDTRec(pobDataTran.srBRec.inHDTIndex);
					inFunc_ComposeFileName(&pobDataTran, szTableName, "", 6);
					strcat(szTableName, _BATCH_TABLE_SUFFIX_);
					inSqlite_Table_Link_BRec(&pobDataTran, &srAll, _LS_READ_);
					/* 取得NCCC和sDCC最大的lnorgInvnum和HDTIndex */
					memset(szQuerySql, 0x00, sizeof(szQuerySql));
					sprintf(szQuerySql, "SELECT lnOrgInvNum FROM %s WHERE (uszUpdated = 0) ORDER BY lnOrgInvNum DESC LIMIT 1", szTableName);
					inTempRetVal = inSqlite_Get_Data_By_External_SQL(gszTranDBPath, &srAll, szQuerySql);
					if (inTempRetVal == VS_SUCCESS)
					{
						inMaxOrgInvNum_NCCC = pobDataTran.srBRec.lnOrgInvNum;
					}
				}
				
				/* 取DCC */
				if (inDCCIndex >= 0)
				{
					memset(&srAll, 0x00, sizeof(SQLITE_ALL_TABLE));
					memset(&pobDataTran, 0x00, sizeof(TRANSACTION_OBJECT));
					memset(szTableName, 0x00, sizeof(szTableName));
					pobDataTran.srBRec.inHDTIndex = inDCCIndex;
					inLoadHDTRec(pobDataTran.srBRec.inHDTIndex);
					inFunc_ComposeFileName(&pobDataTran, szTableName, "", 6);
					strcat(szTableName, _BATCH_TABLE_SUFFIX_);
					inSqlite_Table_Link_BRec(&pobDataTran, &srAll, _LS_READ_);
					/* 取得NCCC和sDCC最大的lnorgInvnum和HDTIndex */
					memset(szQuerySql, 0x00, sizeof(szQuerySql));
					sprintf(szQuerySql, "SELECT lnOrgInvNum FROM %s WHERE (uszUpdated = 0) ORDER BY lnOrgInvNum DESC LIMIT 1", szTableName);
					inTempRetVal = inSqlite_Get_Data_By_External_SQL(gszTranDBPath, &srAll, szQuerySql);
					if (inTempRetVal == VS_SUCCESS)
					{
						inMaxOrgInvNum_DCC = pobDataTran.srBRec.lnOrgInvNum;
					}
				}
				
				/* 取HG */
				if (inHGIndex >= 0)
				{
					memset(&srAll, 0x00, sizeof(SQLITE_ALL_TABLE));
					memset(&pobDataTran, 0x00, sizeof(TRANSACTION_OBJECT));
					memset(szTableName, 0x00, sizeof(szTableName));
					pobDataTran.srBRec.inHDTIndex = inHGIndex;
					inLoadHDTRec(pobDataTran.srBRec.inHDTIndex);
					inFunc_ComposeFileName(&pobDataTran, szTableName, "", 6);
					strcat(szTableName, _BATCH_TABLE_SUFFIX_);
					inSqlite_Table_Link_BRec(&pobDataTran, &srAll, _LS_READ_);
					/* 取得NCCC和sHG最大的lnorgInvnum和HDTIndex */
					memset(szQuerySql, 0x00, sizeof(szQuerySql));
					sprintf(szQuerySql, "SELECT lnOrgInvNum FROM %s WHERE (uszUpdated = 0) ORDER BY lnOrgInvNum DESC LIMIT 1", szTableName);
					inTempRetVal = inSqlite_Get_Data_By_External_SQL(gszTranDBPath, &srAll, szQuerySql);
					if (inTempRetVal == VS_SUCCESS)
					{
						inMaxOrgInvNum_HG = pobDataTran.srBRec.lnOrgInvNum;
					}
				}
				
				if (inMaxOrgInvNum_NCCC >= inMAXOrgInvNum_Total)
				{
					inMAXOrgInvNum_Total = inMaxOrgInvNum_NCCC;
					inHDTIndex_Final = inNCCCIndex;
				}
				
				if (inMaxOrgInvNum_DCC >= inMAXOrgInvNum_Total)
				{
					inMAXOrgInvNum_Total = inMaxOrgInvNum_DCC;
					inHDTIndex_Final = inDCCIndex;
				}
				
				if (inMaxOrgInvNum_HG >= inMAXOrgInvNum_Total)
				{
					inMAXOrgInvNum_Total = inMaxOrgInvNum_HG;
					inHDTIndex_Final = inHGIndex;
				}
				
				if (inHDTIndex_Final >= 0)
				{
					pobTran->srBRec.lnOrgInvNum = inMAXOrgInvNum_Total;
					pobTran->srBRec.inHDTIndex = inHDTIndex_Final;
					inLoadHDTRec(pobTran->srBRec.inHDTIndex);
					inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
				}
				else
				{
					pobTran->srBRec.inHDTIndex = inOrgIndex;
					inLoadHDTRec(pobTran->srBRec.inHDTIndex);
					inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
				}
			}
			else
			{
				memset(&pobDataTran, 0x00, sizeof(TRANSACTION_OBJECT));
				memcpy(&pobDataTran, pobTran, sizeof(TRANSACTION_OBJECT));
				do
				{
					/* 找NCCC */
					pobDataTran.srBRec.inHDTIndex = inNCCCIndex;
					inLoadHDTRec(pobDataTran.srBRec.inHDTIndex);
					inTempRetVal = inBATCH_GetTransRecord_By_Sqlite(&pobDataTran);
					if (inTempRetVal == VS_SUCCESS)
					{
						inHDTIndex_Final = inNCCCIndex;
						break;
					}
					/* 找DCC */
					pobDataTran.srBRec.inHDTIndex = inDCCIndex;
					inLoadHDTRec(pobDataTran.srBRec.inHDTIndex);
					inTempRetVal = inBATCH_GetTransRecord_By_Sqlite(&pobDataTran);
					if (inTempRetVal == VS_SUCCESS)
					{
						inHDTIndex_Final = inDCCIndex;
						break;
					}
					/* 找HG */
					pobDataTran.srBRec.inHDTIndex = inHGIndex;
					inLoadHDTRec(pobDataTran.srBRec.inHDTIndex);
					inTempRetVal = inBATCH_GetTransRecord_By_Sqlite(&pobDataTran);
					if (inTempRetVal == VS_SUCCESS)
					{
						inHDTIndex_Final = inHGIndex;
						break;
					}
					break;
				}while(1);
				
				if (inHDTIndex_Final >= 0)
				{
					pobTran->srBRec.inHDTIndex = inHDTIndex_Final;
					inLoadHDTRec(pobTran->srBRec.inHDTIndex);
					inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
				}
				else
				{
					pobTran->srBRec.inHDTIndex = inOrgIndex;
					inLoadHDTRec(pobTran->srBRec.inHDTIndex);
					inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
				}
			}
		}
				
		if (pobTran->srBRec.inHDTIndex == inESVCIndex)
		{
			/* Load Batch Record */
			inTempRetVal = inBATCH_GetTransRecord_By_Sqlite_ESVC(pobTran);

			if (inTempRetVal == VS_SUCCESS)
			{
				inLoadTDTRec(pobTran->srTRec.inTDTIndex);
				inRetVal = VS_SUCCESS;
				break;
			}
			/* 找不到 */
			else if (inTempRetVal == VS_NO_RECORD)
			{
				/* 因客製化107及111為無人自助機，若無人操作因直接回IDLE，不應繼續輸入流程 2019/1/15 上午 10:01*/
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					/* 顯示無交易紀錄 */
					inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

					inDISP_BEEP(3, 500);
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
					inRetVal = VS_ERROR;
					break;
				}
				else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
					 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))  
				{
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					/* 顯示無交易紀錄 */
					inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

					inDISP_BEEP(3, 500);
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_VOID_INVOICE_ERROR_;
					inRetVal = VS_ERROR;
					break;
				}
				else
				{
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					/* 顯示無交易紀錄 */
					inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

					inDISP_BEEP(3, 500);
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
					inRetVal = VS_ERROR;
					break;
				}
			}
			else
			{
				inRetVal = VS_ERROR;
				break;
			}
		}
		/* DCC有特殊處理 */
		else if (pobTran->srBRec.inHDTIndex == inDCCIndex)
		{
			/* Load Batch Record */
			inTempRetVal = inBATCH_GetTransRecord_By_Sqlite(pobTran);

			if (inTempRetVal == VS_SUCCESS)
			{
				/* NCCC DCC的特殊處理，當端末機收到收銀機要取消DCC HOST交易的電文時，
				 * 如果在DCC的帳務中查不到這一筆交易(表示交易改為台幣支付)端末機就自動在去找NCCC的帳務來解決銀行別錯誤而取消不到的問題。 START
				 */
				if (pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)
				{
					if (pobTran->srBRec.inCode == _VOID_ && pobTran->inTransactionCode == _TIP_)
					{
						/* 仿照520顯示方式 by Russell 2019/12/12 下午 1:41 */
						/* 如果不在這邊檔，就要連重印簽單都一起改，很不方便 */
						/* 顯示 取消交易 不能做小費 請按清除鍵 */
						DISPLAY_OBJECT	srDispMsgObj;
						memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
						strcpy(srDispMsgObj.szDispPic1Name, "");
						srDispMsgObj.inDispPic1YPosition = 0;
						srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
						srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
						strcpy(srDispMsgObj.szErrMsg1, "取消交易");
						srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
						strcpy(srDispMsgObj.szErrMsg2, "不能有小費");
						srDispMsgObj.inErrMsg2Line = _LINE_8_6_;
						srDispMsgObj.inBeepTimes = 1;
						srDispMsgObj.inBeepInterval = 0;

						inDISP_Msg_BMP(&srDispMsgObj);
						
						pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
						inRetVal = VS_ERROR;
						break;
					}
					else
					{
						if (pobTran->uszECRBit == VS_TRUE)
						{
							inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
							/* 顯示無交易紀錄 */
							inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

							inDISP_BEEP(3, 500);
							pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
							inRetVal = VS_ERROR;
							break;
						}
						else
						{
							inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
							/* 顯示無交易紀錄 */
							inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

							inDISP_BEEP(3, 500);
							pobTran->srBRec.lnOrgInvNum = 0;
							continue;
						}
					}
				}
				/* 找到紀錄 */
				else
				{
					inRetVal = VS_SUCCESS;
					break;
				}

			}
			/* 找不到 */
			else if (inTempRetVal == VS_NO_RECORD)
			{
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)         ||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
				{
					/* 075 重印選是最後一筆錯誤直接跳出，不能回輸入調閱編號流程，因為有密碼管控 */
					if (pobTran->inRunOperationID == _OPERATION_REPRINT_	&&
					    pobTran->srBRec.lnOrgInvNum == _BATCH_LAST_RECORD_)
					{
						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						/* 顯示無交易紀錄 */
						inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

						inDISP_BEEP(3, 500);
						pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
						inRetVal = VS_ERROR;
						break;
					}
					else
					{
						if (pobTran->uszECRBit == VS_TRUE)
						{
							inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
							/* 顯示無交易紀錄 */
							inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

							inDISP_BEEP(3, 500);
							pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
							inRetVal = VS_ERROR;
							break;
						}
						else
						{
							inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
							/* 顯示無交易紀錄 */
							inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

							inDISP_BEEP(3, 500);
							pobTran->srBRec.lnOrgInvNum = 0;
							continue;
						}
					}
				}
				/* 因客製化107及111為無人自助機，若無人操作因直接回IDLE，不應繼續輸入流程 2019/1/15 上午 10:01*/
				else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)         ||
					 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_) ||
					 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		  ||
					 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					/* 顯示無交易紀錄 */
					inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

					inDISP_BEEP(3, 500);
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
					inRetVal = VS_ERROR;
					break;
				}
				else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
					 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))  
				{
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					/* 顯示無交易紀錄 */
					inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

					inDISP_BEEP(3, 500);
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_VOID_INVOICE_ERROR_;
					inRetVal = VS_ERROR;
					break;
				}
				else
				{
					if (pobTran->uszECRBit == VS_TRUE)
					{
						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						/* 顯示無交易紀錄 */
						inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

						inDISP_BEEP(3, 500);
						pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
						inRetVal = VS_ERROR;
						break;
					}
					else
					{
						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						/* 顯示無交易紀錄 */
						inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

						inDISP_BEEP(3, 500);
						pobTran->srBRec.lnOrgInvNum = 0;
						continue;
					}
				}
			}
			else
			{
				inRetVal = VS_ERROR;
				break;
			}
		}
                else if (pobTran->srBRec.inHDTIndex == inTRUSTIndex)
                {
                        /* Load Batch Record */
			inTempRetVal = inBATCH_GetTransRecord_Trust_By_Sqlite(pobTran);
			if (inTempRetVal == VS_SUCCESS)
			{
				inRetVal = VS_SUCCESS;
				break;
			}
			/* 找不到 */
			else if (inTempRetVal == VS_NO_RECORD)
			{
				/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/5/20 下午 2:26 
				   客製化075 NCCC主機找不到要往DCC主機找 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)         ||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
				{
					/* 075 重印選是最後一筆錯誤直接跳出，不能回輸入調閱編號流程，因為有密碼管控 */
					if (pobTran->inRunOperationID == _OPERATION_REPRINT_	&&
						 pobTran->srBRec.lnOrgInvNum == _BATCH_LAST_RECORD_)
					{
						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						/* 顯示無交易紀錄 */
						inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);
						inDISP_BEEP(3, 500);
						pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
						inRetVal = VS_ERROR;
						break;
					}
					else if (pobTran->uszECRBit == VS_TRUE)
					{
						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						/* 顯示無交易紀錄 */
						inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

						inDISP_BEEP(3, 500);
						pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
						inRetVal = VS_ERROR;
						break;
					}
					else
					{
						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						/* 顯示無交易紀錄 */
						inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);
						inDISP_BEEP(3, 500);
						pobTran->srBRec.lnOrgInvNum = 0;
						inRetVal = VS_ERROR;
						break;
					}	
				}
				/* 因客製化107及111為無人自助機，若無人操作因直接回IDLE，不應繼續輸入流程 2019/1/15 上午 10:01*/
				else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)         ||
					 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_) ||
					 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		  ||
					 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					/* 顯示無交易紀錄 */
					inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

					inDISP_BEEP(3, 500);
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
					inRetVal = VS_ERROR;
					break;
				}
				else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
					 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))  
				{
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					/* 顯示無交易紀錄 */
					inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

					inDISP_BEEP(3, 500);
					pobTran->srBRec.lnOrgInvNum = 0;
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_VOID_INVOICE_ERROR_;
					inRetVal = VS_ERROR;
					break;
				}
				else
				{
					if (pobTran->uszECRBit == VS_TRUE)
					{
						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						/* 顯示無交易紀錄 */
						inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

						inDISP_BEEP(3, 500);
						pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
						inRetVal = VS_ERROR;
						break;
					}
					else
					{
						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						/* 顯示無交易紀錄 */
						inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

						inDISP_BEEP(3, 500);
						pobTran->srBRec.lnOrgInvNum = 0;
						inRetVal = VS_ERROR;
						break;
					}
				}
			}
			else
			{
				inRetVal = VS_ERROR;
				break;
			}
                }
		else
		{
			/* Load Batch Record */
			inTempRetVal = inBATCH_GetTransRecord_By_Sqlite(pobTran);
			if (inTempRetVal == VS_SUCCESS)
			{
				inRetVal = VS_SUCCESS;
				break;
			}
			/* 找不到 */
			else if (inTempRetVal == VS_NO_RECORD)
			{
				/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/5/20 下午 2:26 
				   客製化075 NCCC主機找不到要往DCC主機找 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)         ||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
				{
					/* 075 重印選是最後一筆錯誤直接跳出，不能回輸入調閱編號流程，因為有密碼管控 */
					if (pobTran->inRunOperationID == _OPERATION_REPRINT_	&&
						 pobTran->srBRec.lnOrgInvNum == _BATCH_LAST_RECORD_)
					{
						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						/* 顯示無交易紀錄 */
						inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);
						inDISP_BEEP(3, 500);
						pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
						inRetVal = VS_ERROR;
						break;
					}
					else if (pobTran->uszECRBit == VS_TRUE)
					{
						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						/* 顯示無交易紀錄 */
						inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

						inDISP_BEEP(3, 500);
						pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
						inRetVal = VS_ERROR;
						break;
					}
					else
					{
						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						/* 顯示無交易紀錄 */
						inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);
						inDISP_BEEP(3, 500);
						pobTran->srBRec.lnOrgInvNum = 0;
						continue;
					}	
				}
				/* 因客製化107及111為無人自助機，若無人操作因直接回IDLE，不應繼續輸入流程 2019/1/15 上午 10:01*/
				else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)         ||
					 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_) ||
					 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		  ||
					 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					/* 顯示無交易紀錄 */
					inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

					inDISP_BEEP(3, 500);
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
					inRetVal = VS_ERROR;
					break;
				}
				else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
					 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))  
				{
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					/* 顯示無交易紀錄 */
					inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

					inDISP_BEEP(3, 500);
					pobTran->srBRec.lnOrgInvNum = 0;
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_VOID_INVOICE_ERROR_;
					inRetVal = VS_ERROR;
					break;
				}
				else
				{
					if (pobTran->uszECRBit == VS_TRUE)
					{
						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						/* 顯示無交易紀錄 */
						inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

						inDISP_BEEP(3, 500);
						pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
						inRetVal = VS_ERROR;
						break;
					}
					else
					{
						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						/* 顯示無交易紀錄 */
						inDISP_PutGraphic(_ERR_RECORD_, 0, _COORDINATE_Y_LINE_8_6_);

						inDISP_BEEP(3, 500);
						pobTran->srBRec.lnOrgInvNum = 0;
						continue;
					}
				}
			}
			else
			{
				inRetVal = VS_ERROR;
				break;
			}
		}
		break;
	}while(1);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	/* ECR Uny交易檢核，1000不能信用卡取消,400不能uny取消 */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		if (pobTran->inTransactionCode == _VOID_    ||
                    pobTran->inTransactionCode == _CUP_VOID_)
		{
			if (gsrECROb.srTransData.szECRIndicator[0] == 'Q')
			{
				if (pobTran->srBRec.uszUnyTransBit == VS_TRUE       ||
                                    pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE   ||
                                    pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
				{
					
				}
				else
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					sprintf(pobTran->szErrorMsgBuff1, "ECR請勿用掃碼格式");
					pobTran->inECRErrorMsg = VS_ISO_PACK_ERR;
					
					/* Mirror Message */
					inECR_SendMirror(pobTran, _MIRROR_MSG_OPER_ERROR_);
					
					return (VS_ERROR);
				}
			}
			else
			{
				if (pobTran->srBRec.uszUnyTransBit == VS_TRUE       ||
                                    pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE   ||
                                    pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					sprintf(pobTran->szErrorMsgBuff1, "ECR請用掃碼格式");
					pobTran->inECRErrorMsg = VS_ISO_PACK_ERR;
					
					/* Mirror Message */
					inECR_SendMirror(pobTran, _MIRROR_MSG_OPER_ERROR_);
					
					return (VS_ERROR);
				}
				else
				{
					
				}
			}
		}
	}
	
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
	{
		if (pobTran->inTransactionCode == _TIP_ && pobTran->srBRec.uszTakaTransBit == VS_TRUE)
		{
			/* 顯示社員卡不支持小費 請按清除鍵 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, "社員卡不支持小費");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
			
			return (VS_ERROR);
		}
	}
	
	/* 簽名狀態要重置，取消、小費這種讀批次的會紀錄到上一次的狀態 */
	/* 是否輸入Pin狀態重置 */
	/* 手動輸入調閱編號的情況中，只有重印是要保留原狀態 */
	if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
	{
		
	}
	else
	{
		pobTran->srBRec.inESCUploadMode = _ESC_STATUS_NONE_;
		pobTran->srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_UPLOAD_NOT_YET_;
		pobTran->srBRec.inSignStatus = _SIGN_NONE_;
		pobTran->srBRec.uszPinEnterBit = VS_FALSE;
		
		/* 清空ResponseCode，當作一筆新的交易 */
		memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));
		
		/* 取消要帶原值，其他取原交易則要當新交易  */
		/* 取消的免簽比照原交易，小費一律要簽 */
		if (pobTran->inTransactionCode == _VOID_		||
		    pobTran->inTransactionCode == _CUP_VOID_		||
		    pobTran->inTransactionCode == _CUP_PRE_AUTH_VOID_	||
		    pobTran->inTransactionCode == _FISC_VOID_)
		{
			
		}
		else
		{
			/* 小費一律要簽，仿照520直接修改簽單判斷*/
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE;
		}
	}
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_FuncUserChoice_By_Sqlite()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inBATCH_GetTransRecord_By_Sqlite
Date&Time       :2017/3/15 下午 4:57
Describe        :讀出pobTran->srBRec.lnOrgInvNum的 pobTran->srBRec的紀錄，
 *				 如果pobTran->srBRec有EMV的資料，透過pobTran->inTableID讀出pobTran->srEMVRec的紀錄
*/
int inBATCH_GetTransRecord_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GetTransRecord_By_Sqlite()_START");
        }
	
	inRetVal = inBATCH_Get_Batch_ByInvNum_Flow(pobTran, _TN_BATCH_TABLE_, pobTran->srBRec.lnOrgInvNum);
	if (inRetVal != VS_SUCCESS)
		return (inRetVal);
	
	if (pobTran->srBRec.inChipStatus != 0			|| 
	    pobTran->srBRec.uszContactlessBit == VS_TRUE	|| 
	    pobTran->srBRec.uszEMVFallBackBit == VS_TRUE        ||
            pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
	{
		inRetVal = inBATCH_Get_Batch_ByInvNum_Flow(pobTran, _TN_EMV_TABLE_, pobTran->srBRec.lnOrgInvNum);
		if (inRetVal != VS_SUCCESS)
			return (inRetVal);
	}
	
	/* 儲存原STAN，離線交易軟加要用原STAN */
	pobTran->srBRec.lnOrgSTANNum = pobTran->srBRec.lnSTANNum;
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GetTransRecord_By_Sqlite()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (inRetVal);
}

/*
Function        :inBATCH_GetTransRecord_By_Sqlite_ESVC
Date&Time       :2018/1/30 下午 6:30
Describe        :讀出pobTran->srBRec.lnOrgInvNum的紀錄
*/
int inBATCH_GetTransRecord_By_Sqlite_ESVC(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GetTransRecord_By_Sqlite_ESVC()_START");
        }
	
	inRetVal = inBATCH_Get_Batch_ByInvNum_Flow(pobTran, _TN_BATCH_TABLE_TICKET_, pobTran->srBRec.lnOrgInvNum);
	if (inRetVal != VS_SUCCESS)
		return (inRetVal);
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GetTransRecord_By_Sqlite_ESVC()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (inRetVal);
}

/*
Function        :inBATCH_GlobalBatchHandleClose_By_Sqlite
Date&Time       :2017/3/15 下午 5:20
Describe        :
*/
int inBATCH_GlobalBatchHandleClose_By_Sqlite(void)
{
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GlobalBatchHandleClose_By_Sqlite()_START");
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GlobalBatchHandleClose_By_Sqlite()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inBATCH_AdviceHandleReadOnly_By_Sqlite
Date&Time       :2017/3/15 下午 5:19
Describe        :
*/
int inBATCH_AdviceHandleReadOnly_By_Sqlite(TRANSACTION_OBJECT* pobTran)
{
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_AdviceHandleReadOnly_By_Sqlite()_START");
        }

	if (inFunc_ComposeFileName(pobTran, (char*)guszADVFileName, _ADVICE_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	/* 困惑為什麼handler要用unsigned long，並賦予負數不會overflow? */
        /* 為了避免重複開Handle造成當機，先將handle值設為default(0x00)(因為用globalHandle無法確保是否已關，重複開會造成錯誤) */
        gulADVHandle = VS_HANDLE_NULL;

        if (inFILE_OpenReadOnly(&gulADVHandle, guszADVFileName) == VS_ERROR)
	{
		return (VS_ERROR);
	}

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_AdviceHandleReadOnly_By_Sqlite()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_GlobalAdviceHandleClose_By_Sqlite
Date&Time       :2017/3/15 下午 5:18
Describe        :
*/
int inBATCH_GlobalAdviceHandleClose_By_Sqlite(void)
{
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GlobalAdviceHandleClose_By_Sqlite()_START");
        }
        if (gulADVHandle != VS_HANDLE_NULL)
        {
                inFILE_Close(&gulADVHandle);
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GlobalAdviceHandleClose_By_Sqlite()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inBATCH_GetAdviceDetailRecord_By_Sqlite
Date&Time       :2017/3/15 下午 5:14
Describe        :
*/
int inBATCH_GetAdviceDetailRecord_By_Sqlite(TRANSACTION_OBJECT *pobTran, int inADVCnt)
{
        int             inRetVal;
        unsigned char   uszGetAdviceInvoiceNum[6 + 1];

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GetAdviceDetailRecord_By_Sqlite()_START");
        }

        /* 讀【.adv】檔 */
        if (inFILE_Seek(gulADVHandle, 0, _SEEK_BEGIN_) == VS_ERROR)
        {
                return (VS_NO_RECORD);
        }
        else
        {
                memset(uszGetAdviceInvoiceNum, 0x00, sizeof(uszGetAdviceInvoiceNum));
                if ((inRetVal = inFILE_Read(&gulADVHandle, &uszGetAdviceInvoiceNum[0], _ADV_FIELD_SIZE_)) == VS_ERROR)
                {
                        return (VS_NO_RECORD);
                }
                else
                {
                        pobTran->srBRec.lnOrgInvNum = atol((char *)uszGetAdviceInvoiceNum);
                }
        }

	inRetVal = inBATCH_GetTransRecord_By_Sqlite(pobTran);

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GetAdviceDetailRecord_By_Sqlite()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (inRetVal);
}

/*
Function        :inBATCH_Advice_ESC_HandleReadOnly
Date&Time       :2017/3/15 下午 5:19
Describe        :
*/
int inBATCH_Advice_ESC_HandleReadOnly(TRANSACTION_OBJECT* pobTran)
{
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_Advice_ESC_HandleReadOnly()_START");
        }

	if (inFunc_ComposeFileName(pobTran, (char*)guszADVFileName, _ADVICE_ESC_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
        /* 為了避免重複開Handle造成當機，先將handle值設為default(0x00)(因為用globalHandle無法確保是否已關，重複開會造成錯誤) */
        gulADVHandle = VS_HANDLE_NULL;

        if (inFILE_OpenReadOnly(&gulADVHandle, guszADVFileName) == VS_ERROR)
	{
		return (VS_ERROR);
	}

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_Advice_ESC_HandleReadOnly()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_GlobalAdvice_ESC_HandleClose
Date&Time       :2017/3/15 下午 5:18
Describe        :
*/
int inBATCH_GlobalAdvice_ESC_HandleClose(void)
{
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GlobalAdvice_ESC_HandleClose()_START");
        }

        if (gulADVHandle != VS_HANDLE_NULL)
        {
                inFILE_Close(&gulADVHandle);
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GlobalAdvice_ESC_HandleClose()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inBATCH_Advice_ESC_HandleReadOnly_By_Sqlite
Date&Time       :2019/5/6 下午 8:02
Describe        :
*/
int inBATCH_Advice_ESC_HandleReadOnly_By_Sqlite(TRANSACTION_OBJECT* pobTran)
{
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_Advice_ESC_HandleReadOnly_By_Sqlite()_START");
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_Advice_ESC_HandleReadOnly_By_Sqlite()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_GlobalAdvice_ESC_HandleClose
Date&Time       :2017/3/15 下午 5:18
Describe        :
*/
int inBATCH_GlobalAdvice_ESC_HandleClose_By_Sqlite(void)
{
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GlobalAdvice_ESC_HandleClose_By_Sqlite()_START");
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GlobalAdvice_ESC_HandleClose_By_Sqlite()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inBATCH_GetAdvice_ESC_DetailRecord
Date&Time       :2017/3/29 上午 11:18
Describe        :目前和inBATCH_GetAdvice_DetailRecord_By_Sqlite完全相同，但為了保險所以也分開
*/
int inBATCH_GetAdvice_ESC_DetailRecord(TRANSACTION_OBJECT *pobTran, int inADVCnt)
{
        int             inRetVal;
        unsigned char   uszGetAdviceInvoiceNum[6 + 1];

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GetAdvice_ESC_DetailRecord()_START");
        }

        /* 讀【.adv】檔 */
        if (inFILE_Seek(gulADVHandle, 0, _SEEK_BEGIN_) == VS_ERROR)
        {
                return (VS_ERROR);
        }
        else
        {
                memset(uszGetAdviceInvoiceNum, 0x00, sizeof(uszGetAdviceInvoiceNum));
                if ((inRetVal = inFILE_Read(&gulADVHandle, &uszGetAdviceInvoiceNum[0], _ADV_FIELD_SIZE_)) == VS_ERROR)
                {
                        return (VS_ERROR);
                }
                else
                {
                        pobTran->srBRec.lnOrgInvNum = atol((char *)uszGetAdviceInvoiceNum);
                }
        }

	inRetVal = inBATCH_GetTransRecord_By_Sqlite(pobTran);

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GetAdvice_ESC_DetailRecord()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (inRetVal);
}

/*
Function        :inBATCH_GetAdvice_ESC_DetailRecord_By_Sqlite
Date&Time       :2019/5/6 下午 8:13
Describe        :advice獨立存在一個table內
*/
int inBATCH_GetAdvice_ESC_DetailRecord_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
        int	inRetVal = VS_SUCCESS;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GetAdvice_ESC_DetailRecord_By_Sqlite()_START");
        }

	inRetVal = inSqlite_ESC_Get_BRec_Top_Flow(pobTran, _TN_BATCH_TABLE_ESC_ADVICE_);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	if (pobTran->srBRec.inChipStatus != 0			|| 
	    pobTran->srBRec.uszContactlessBit == VS_TRUE	|| 
	    pobTran->srBRec.uszEMVFallBackBit == VS_TRUE        ||
            pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
	{
		inRetVal = inSqlite_ESC_Get_BRec_Top_Flow(pobTran, _TN_BATCH_TABLE_ESC_ADVICE_EMV_);
		if (inRetVal != VS_SUCCESS)
		{
			return (inRetVal);
		}
	}
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GetAdvice_ESC_DetailRecord_By_Sqlite()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (inRetVal);
}

/*
Function        :inBATCH_ESC_Save_Advice_By_Sqlite
Date&Time       :2017/3/15 下午 5:15
Describe        :存ESC advice
*/
int inBATCH_ESC_Save_Advice_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inBATCH_ESC_Save_Advice_By_Sqlite() START !");
	}
	inADVICE_ESC_SaveAppend_By_Sqlite(pobTran, pobTran->srBRec.lnOrgInvNum);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inBATCH_ESC_Save_Advice_By_Sqlite() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inBATCH_Update_Sign_Status_By_Sqlite
Date&Time       :2017/3/22 下午 3:46
Describe        :更新簽名狀態
*/
int inBATCH_Update_Sign_Status_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	char	szTableName[50 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inBATCH_Update_Sign_Status_By_Sqlite() START !");
	}
	
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	inSqlite_Update_ByInvNum_SignState(pobTran, gszTranDBPath, szTableName, pobTran->srBRec.lnOrgInvNum);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inBATCH_Update_Sign_Status_By_Sqlite() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inBATCH_Update_MPAS_Reprint_By_Sqlite
Date&Time       :2017/9/5 下午 4:34
Describe        :更新MPAS重印狀態
*/
int inBATCH_Update_MPAS_Reprint_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inBATCH_Update_MPAS_Reprint_By_Sqlite() START !");
	}
	
	pobTran->uszUpdateBatchBit = VS_TRUE;
	inBATCH_FuncUpdateTxnRecord_By_Sqlite(pobTran);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inBATCH_Update_MPAS_Reprint_By_Sqlite() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inBATCH_Update_CLS_SettleBit_By_Sqlite
Date&Time       :2017/4/11 上午 11:15
Describe        :
*/
int inBATCH_Update_CLS_SettleBit_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inBATCH_Update_CLS_SettleBit_By_Sqlite() START !");
	}
	
	inSqlite_Update_ByInvNum_CLS_SettleBit_Flow(pobTran, _TN_BATCH_TABLE_, pobTran->srBRec.lnOrgInvNum);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inBATCH_Update_CLS_SettleBit_By_Sqlite() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inBATCH_Update_ESC_Uploaded_By_Sqlite
Date&Time       :2018/4/27 下午 5:49
Describe        :更新ESC上傳狀態
*/
int inBATCH_Update_ESC_Uploaded_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inBATCH_Update_ESC_Uploaded_By_Sqlite() START !");
	}
	
	pobTran->uszUpdateBatchBit = VS_TRUE;
	inBATCH_FuncUpdateTxnRecord_By_Sqlite(pobTran);

	/*[260318_BUG] 因使用原廠API不需再進行同步,移除檔案同步函式 */
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inBATCH_Update_ESC_Uploaded_By_Sqlite() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inBATCH_GetTotalCount_BatchUpload_By_Sqlite
Date&Time       :2017/4/11 下午 3:13
Describe        :取得BatchUpload有效筆數，以避免最後一筆為取消時沒有改processing code
*/
int inBATCH_GetTotalCount_BatchUpload_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	int	inCnt = -1;

        if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GetTotalCountFromBakFile_By_Sqlite()_START");
        }

	inRetVal = inSqlite_Get_Batch_Upload_Count_Flow(pobTran, _TN_BATCH_TABLE_, &inCnt);
	if (inRetVal == VS_NO_RECORD)
	{
		return (VS_NO_RECORD);
	}
	else if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GetTotalCountFromBakFile_By_Sqlite()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (inCnt);
}

/*
Function        :inBATCH_FuncUpdateTxnRecord_Ticket_By_Sqlite
Date&Time       :2018/1/12 下午 4:29
Describe        :insert資料到資料庫
*/
int inBATCH_FuncUpdateTxnRecord_Ticket_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inBATCH_FuncUpdateTxnRecord_Ticket_By_Sqlite()_START");
	}
	/* 若斷電導致批次無法正確儲存時方便解釋 */
	inUtility_StoreTraceLog_OneStep("_TICKET_UPDATE_BATCH_START_($:%ld)", pobTran->srTRec.lnTxnAmount);
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	/* 如果是update，不用再試圖創Table */
	if (pobTran->uszUpdateBatchBit == VS_TRUE)
	{
		inRetVal = inSqlite_Check_Table_Exist_Flow(pobTran, _TN_BATCH_TABLE_TICKET_);
		/* 如果要update卻沒有table，直接跳出 */
		if (inRetVal != VS_SUCCESS)
		{
			inFunc_EDCLock(AT);
		}
	}
	else
	{
		inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_BATCH_TABLE_TICKET_);
		if (inRetVal != VS_SUCCESS)
		{
			inFunc_EDCLock(AT);
		}
	}

	/* 如果uszUpdate設為1 */
	if (pobTran->uszUpdateBatchBit == VS_TRUE)
	{
		inSqlite_Update_ByInvNum_TranState_Flow(pobTran, _TN_BATCH_TABLE_TICKET_, pobTran->srTRec.lnInvNum);
	}
	
	/* Insert批次 */
	inRetVal = inBATCH_Insert_All_Flow(pobTran, _TN_BATCH_TABLE_TICKET_);
	if (inRetVal != VS_SUCCESS)
	{
		inFunc_EDCLock(AT);
	}
	/*[260318_BUG] 因使用原廠API不需再進行同步,移除檔案同步函式 */
	/* 【需求單 - 108128】單機重印前筆簽單流程新增畫面 by Russell 2019/8/15 下午 2:59 */
	inFLOW_RunFunction(pobTran, _FUNCTION_SAVE_LAST_TXN_HOST_);

	/* 客製化098，儲存最後一筆成功唯一碼 */
	inFLOW_RunFunction(pobTran, _FUNCTION_SAVE_LAST_UNIQUE_NO_);

	/* 若斷電導致批次無法正確儲存時方便解釋 */
	inUtility_StoreTraceLog_OneStep("_TICKET_UPDATE_BATCH_END_($:%ld)", pobTran->srTRec.lnTxnAmount);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inBATCH_FuncUpdateTxnRecord_Ticket_By_Sqlite()_END");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inBATCH_Create_BatchTable_Flow
Date&Time       :2017/3/14 下午 3:00
Describe        :在這邊決定名稱並分流
*/
int inBATCH_Create_BatchTable_Flow(TRANSACTION_OBJECT *pobTran, int inTableType)
{
	int	inRetVal = VS_SUCCESS;
	char	szTableName1[50 + 1];	/* 若傳進的TableName1為空字串，則用szTableName組TableName，理論上Table Name不限制長度 */
	char	szTableName2[50 + 1];	/* 用來設定外鍵 */
	
	/* 由function決定TableName */
	memset(szTableName1, 0x00, sizeof(szTableName1));
	if (pobTran->uszFileNameNoNeedNumBit == VS_TRUE)
	{
		inFunc_ComposeFileName(pobTran, szTableName1, "", 0);
	}
	else
	{
		inFunc_ComposeFileName(pobTran, szTableName1, "", 6);
	}
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName1, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			memset(szTableName2, 0x00, sizeof(szTableName2));
			memcpy(szTableName2, szTableName1, strlen(szTableName1));
			strcat(szTableName1, _EMV_TABLE_SUFFIX_);
			strcat(szTableName2, _BATCH_TABLE_SUFFIX_);
			break;
		/* ESC */
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName1, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_EMV_:
			memset(szTableName2, 0x00, sizeof(szTableName2));
			memcpy(szTableName2, szTableName1, strlen(szTableName1));
			strcat(szTableName1, _BATCH_TABLE_ESC_AGAIN_EMV_SUFFIX_);
			strcat(szTableName2, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName1, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_EMV_:
			memset(szTableName2, 0x00, sizeof(szTableName2));
			memcpy(szTableName2, szTableName1, strlen(szTableName1));
			strcat(szTableName1, _BATCH_TABLE_ESC_FAIL_EMV_SUFFIX_);
			strcat(szTableName2, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_TEMP_:
			strcat(szTableName1, _BATCH_TABLE_ESC_TEMP_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_TEMP_EMV_:
			memset(szTableName2, 0x00, sizeof(szTableName2));
			memcpy(szTableName2, szTableName1, strlen(szTableName1));
			strcat(szTableName1, _BATCH_TABLE_ESC_TEMP_EMV_SUFFIX_);
			strcat(szTableName2, _BATCH_TABLE_ESC_TEMP_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_ADVICE_:
			strcat(szTableName1, _BATCH_TABLE_ESC_ADVICE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_ADVICE_EMV_:
			memset(szTableName2, 0x00, sizeof(szTableName2));
			memcpy(szTableName2, szTableName1, strlen(szTableName1));
			strcat(szTableName1, _BATCH_TABLE_ESC_ADVICE_EMV_SUFFIX_);
			strcat(szTableName2, _BATCH_TABLE_ESC_ADVICE_SUFFIX_);
			break;
		/* 票證 */
		case _TN_BATCH_TABLE_TICKET_:
			strcat(szTableName1, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			strcat(szTableName1, _BATCH_TABLE_ADVICE_SUFFIX_);
			break;
                /* 信託 */
                case _TN_BATCH_TABLE_TRUST_:
			strcat(szTableName1, _BATCH_TABLE_SUFFIX_);
		default :
			break;
	}
	
	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_BATCH_TABLE_ESC_TEMP_:
		case _TN_BATCH_TABLE_ESC_ADVICE_:
			inRetVal = inSqlite_Create_Table(gszTranDBPath, szTableName1, TABLE_BATCH_TAG);
			break;
		case _TN_EMV_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_EMV_:
		case _TN_BATCH_TABLE_ESC_FAIL_EMV_:
		case _TN_BATCH_TABLE_ESC_TEMP_EMV_:
		case _TN_BATCH_TABLE_ESC_ADVICE_EMV_:
			inRetVal = inSqlite_Create_BatchTable_EMV(gszTranDBPath, szTableName1, szTableName2, TABLE_EMV_BATCH_TAG);
			break;
		case _TN_BATCH_TABLE_TICKET_:
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			inRetVal = inSqlite_Create_Table(gszTranDBPath, szTableName1, TABLE_TICKET_TAG);
			break;
		case _TN_BATCH_TABLE_TRUST_:
			inRetVal = inSqlite_Create_Table(gszTranDBPath, szTableName1, TABLE_TRUST_TAG);
			break;
		default :
			return (VS_ERROR);
			break;
	}
	
	return (inRetVal);
}

/*
Function        :inBATCH_Get_Batch_ByInvNum_Flow
Date&Time       :2018/1/31 下午 1:58
Describe        :
*/
int inBATCH_Get_Batch_ByInvNum_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inInvoiceNumber)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_TICKET_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		default :
			break;
	}
	
	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
			inRetVal = inSqlite_Get_Batch_ByInvNum(pobTran, szTableName, inInvoiceNumber);
			break;
		case _TN_EMV_TABLE_:
			inRetVal = inSqlite_Get_EMV_ByInvNum(pobTran, szTableName);
			break;
		case _TN_BATCH_TABLE_TICKET_:
			inRetVal = inNCCC_Ticket_ESVC_Get_Batch_ByInvNum(pobTran, szTableName, inInvoiceNumber);
			break;
		default :
			return (VS_ERROR);
			break;
	}
	
	return (inRetVal);
}

/*
Function        :inBATCH_Insert_All_Flow
Date&Time       :2017/3/14 下午 3:00
Describe        :在這邊決定名稱並分流
*/
int inBATCH_Insert_All_Flow(TRANSACTION_OBJECT *pobTran, int inTableType)
{
	int			inRetVal;
	char			szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	SQLITE_ALL_TABLE	srAll;
	
	memset(&srAll, 0x00, sizeof(SQLITE_ALL_TABLE));
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	if (pobTran->uszFileNameNoNeedNumBit == VS_TRUE)
	{
		inFunc_ComposeFileName(pobTran, szTableName, "", 0);
	}
	else
	{
		inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	}
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
		/* ESC */
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_TEMP_:
			strcat(szTableName, _BATCH_TABLE_ESC_TEMP_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_ADVICE_:
			strcat(szTableName, _BATCH_TABLE_ESC_ADVICE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_AGAIN_EMV_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_EMV_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_EMV_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_EMV_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_TEMP_EMV_:
			strcat(szTableName, _BATCH_TABLE_ESC_TEMP_EMV_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_ADVICE_EMV_:
			strcat(szTableName, _BATCH_TABLE_ESC_ADVICE_EMV_SUFFIX_);
			break;
		/* 票證 */
		case _TN_BATCH_TABLE_TICKET_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			strcat(szTableName, _BATCH_TABLE_ADVICE_SUFFIX_);
			break;
                /* 信託 */
                case _TN_BATCH_TABLE_TRUST_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
                        break;
		default :
			break;
	}
	
	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_BATCH_TABLE_ESC_TEMP_:
		case _TN_BATCH_TABLE_ESC_ADVICE_:
			inSqlite_Table_Link_BRec(pobTran, &srAll, _LS_INSERT_);
			inRetVal = inSqlite_Insert_Record(gszTranDBPath, szTableName, &srAll);
			break;
		case _TN_EMV_TABLE_:
		case _TN_BATCH_TABLE_ESC_AGAIN_EMV_:
		case _TN_BATCH_TABLE_ESC_FAIL_EMV_:
		case _TN_BATCH_TABLE_ESC_TEMP_EMV_:
		case _TN_BATCH_TABLE_ESC_ADVICE_EMV_:
			inSqlite_Table_Link_EMVRec(pobTran, &srAll, _LS_INSERT_);
			inRetVal = inSqlite_Insert_Record(gszTranDBPath, szTableName, &srAll);
			break;
			
		case _TN_BATCH_TABLE_TICKET_:
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			inNCCC_Ticket_Table_Link_TRec(pobTran, &srAll, _LS_INSERT_);
			inRetVal = inSqlite_Insert_Record(gszTranDBPath, szTableName, &srAll);
			break;
                case _TN_BATCH_TABLE_TRUST_:
                        inNCCC_Trust_Table_Link_TrustRec(pobTran, &srAll, _LS_INSERT_);
			inRetVal = inSqlite_Insert_Record(gszTranDBPath, szTableName, &srAll);
                        break;
		default :
			return (VS_ERROR);
			break;
	}
	
	return (inRetVal);
}

/*
Function        :inBATCH_Get_ESC_Upload_Count_Flow
Date&Time       :2018/5/31 下午 7:07
Describe        :在這邊決定名稱並分流
 *		inTxnType:sale、refund
 *		uszPaperBit:是否已出紙本
 *		
*/
int inBATCH_Get_ESC_Upload_Count_Flow(TRANSACTION_OBJECT *pobTran, int inTableType, int inTxnType, unsigned char uszPaperBit, int *inTxnTotalCnt, long *lnTxnTotalAmt)
{
	int	inRetVal;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	/* 會長得CR000001的形式 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_EMV_TABLE_:
			strcat(szTableName, _EMV_TABLE_SUFFIX_);
			break;
			
		/* ESC */
		case _TN_BATCH_TABLE_ESC_AGAIN_:
			strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_ESC_FAIL_:
			strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
			break;
		
		/* 票證 */
		case _TN_BATCH_TABLE_TICKET_:
			strcat(szTableName, _BATCH_TABLE_SUFFIX_);
			break;
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			strcat(szTableName, _BATCH_TABLE_ADVICE_SUFFIX_);
			break;
			
		default :
			break;
	}
	
	/* Batch 和 EMV 跑不同邊 */
	switch (inTableType)
	{
		case _TN_BATCH_TABLE_:
			inRetVal = inSqlite_Get_ESC_Sale_Upload_Count(pobTran, szTableName, inTxnType, uszPaperBit, inTxnTotalCnt, lnTxnTotalAmt);
			break;
		default :
			return (VS_ERROR);
			break;
	}
	
	return (inRetVal);
}

/*
Function        :inBATCH_GetAdvice_ESVC_DetailRecord_By_Sqlite
Date&Time       :2018/1/26 下午 2:28
Describe        :目前和inBATCH_GetAdvice_DetailRecord_By_Sqlite完全相同，但為了保險所以也分開
*/
int inBATCH_GetAdvice_ESVC_DetailRecord_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
        int	inRetVal = VS_ERROR;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GetAdvice_ESC_DetailRecord_By_Sqlite()_START");
        }

	inRetVal = inNCCC_Ticket_ESVC_Get_TRec_Top_Flow(pobTran, _TN_BATCH_TABLE_TICKET_ADVICE_);

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GetAdvice_ESC_DetailRecord_By_Sqlite()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (inRetVal);
}

/*
Function        :inBATCH_Presssure_Test
Date&Time       :2017/10/17 下午 2:17
Describe        :
*/
int inBATCH_Presssure_Test()
{
	
	return (VS_SUCCESS);
}

/*
Function        :inBATCH_ESC_Save_Advice_Flow
Date&Time       :2019/5/6 上午 9:59
Describe        :存ESC advice 已方式分流
*/
int inBATCH_ESC_Save_Advice_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inBATCH_ESC_Save_Advice_Flow() START !");
	}
	
	inNCCC_Func_Get_STAN_Flow(pobTran);
	inNCCC_Func_Set_STAN_Flow(pobTran);
	
	if (_PARAMETER_SAVE_WAY_DAT_ == _PARAMETER_SAVE_WAY_DAT_SQLITE_)
	{
		inRetVal = inBATCH_ESC_Save_Advice_By_Sqlite(pobTran);
	}
	else
	{
		inRetVal = inBATCH_ESC_Save_Advice(pobTran);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inBATCH_ESC_Save_Advice_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inADVICE_ESC_GetTotalCount_Flow
Date&Time       :2019/5/6 下午 5:07
Describe        :
*/
int inADVICE_ESC_GetTotalCount_Flow(TRANSACTION_OBJECT *pobTran)
{
        int	inADVCnt = 0;
	
	if (_PARAMETER_SAVE_WAY_DAT_ == _PARAMETER_SAVE_WAY_DAT_SQLITE_)
	{
		inADVCnt = inADVICE_ESC_GetTotalCount_By_Sqlite(pobTran);
	}
	else
	{
		inADVCnt = inADVICE_ESC_GetTotalCount(pobTran);
	}

	if (inADVCnt <= 0)
	{
		return (VS_NO_RECORD);
	}
	else
	{
		return (inADVCnt);
	}
}

/*
Function        :inADVICE_ESC_DeleteRecordFlow
Date&Time       :2016/9/20 上午 9:57
Describe        :檢查 lnInvNum 是否在 Advice 檔案中，若是則將 lnInvNum 從 Advice 檔案中刪除，
 *		 sqlite預設刪最上面一筆
*/
int inADVICE_ESC_DeleteRecordFlow(TRANSACTION_OBJECT *pobTran, long lnInvNum)
{
	int	inRetVal = VS_ERROR;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inADVICE_ESC_DeleteRecordFlow() START !! Way=%d", _PARAMETER_SAVE_WAY_DAT_);
	}
	
	if (_PARAMETER_SAVE_WAY_DAT_ == _PARAMETER_SAVE_WAY_DAT_SQLITE_)
	{
		inRetVal = inADVICE_ESC_DeleteRecord_By_Sqlite(pobTran, lnInvNum);
	}
	else
	{
		inRetVal = inADVICE_ESC_DeleteRecord(pobTran, lnInvNum);
	}


	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inADVICE_ESC_DeleteRecordFlow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inBATCH_Advice_ESC_HandleReadOnly_Flow
Date&Time       :2019/5/6 下午 7:52
Describe        :
*/
int inBATCH_Advice_ESC_HandleReadOnly_Flow(TRANSACTION_OBJECT* pobTran)
{
	int	inRetVal = VS_SUCCESS;
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_Advice_ESC_HandleReadOnly_Flow()_START");
        }
	
	if (_PARAMETER_SAVE_WAY_DAT_ == _PARAMETER_SAVE_WAY_DAT_SQLITE_)
	{
		inRetVal = inBATCH_Advice_ESC_HandleReadOnly_By_Sqlite(pobTran);
	}
	else
	{
		inRetVal = inBATCH_Advice_ESC_HandleReadOnly(pobTran);
	}
	

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_Advice_ESC_HandleReadOnly_Flow()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (inRetVal);
}

/*
Function        :inBATCH_GlobalAdvice_ESC_HandleClose_Flow
Date&Time       :2019/5/6 下午 7:55
Describe        :
*/
int inBATCH_GlobalAdvice_ESC_HandleClose_Flow(void)
{
	int	inRetVal = VS_SUCCESS;
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GlobalAdvice_ESC_HandleClose_Flow()_START");
        }

        if (_PARAMETER_SAVE_WAY_DAT_ == _PARAMETER_SAVE_WAY_DAT_SQLITE_)
	{
		inRetVal = inBATCH_GlobalAdvice_ESC_HandleClose_By_Sqlite();
	}
	else
	{
		inRetVal = inBATCH_GlobalAdvice_ESC_HandleClose();
	}

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GlobalAdvice_ESC_HandleClose_Flow()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (inRetVal);
}

/*
Function        :inBATCH_GetAdvice_ESC_DetailRecord_Flow
Date&Time       :2019/5/6 下午 8:12
Describe        :目前和inBATCH_GetAdvice_DetailRecord_By_Sqlite完全相同，但為了保險所以也分開
*/
int inBATCH_GetAdvice_ESC_DetailRecord_Flow(TRANSACTION_OBJECT *pobTran, int inADVCnt)
{
        int	inRetVal = VS_SUCCESS;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GetAdvice_ESC_DetailRecord_Flow()_START");
        }

        if (_PARAMETER_SAVE_WAY_DAT_ == _PARAMETER_SAVE_WAY_DAT_SQLITE_)
	{
		inRetVal = inBATCH_GetAdvice_ESC_DetailRecord_By_Sqlite(pobTran);
	}
	else
	{
		inRetVal = inBATCH_GetAdvice_ESC_DetailRecord(pobTran, inADVCnt);
	}

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GetAdvice_ESC_DetailRecord_Flow()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (inRetVal);
}

/*
Function	:inBATCH_Find_Last_Txn_Inv_SQLite
Date&Time	:2020/5/21 下午 1:25
Describe        :抓最後一筆交易的Inv
*/
int inBATCH_Find_Last_Txn_Inv_SQLite(TRANSACTION_OBJECT *pobTran, char* szInvNum)
{
	int			inRetVal = VS_SUCCESS;
	char			szQuerySql[200 + 1] = {0};
	char			szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	char			szTRTFName[16 + 1] = {0};
	SQLITE_ALL_TABLE	srAll;
	TRANSACTION_OBJECT	pobTempTran;
	
	memset(&pobTempTran, 0x00, sizeof(pobTempTran));
	memcpy(&pobTempTran, pobTran, sizeof(TRANSACTION_OBJECT));
	
	memset(szTRTFName, 0x00, sizeof(szTRTFName));
        inGetTRTFileName(szTRTFName);
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(&pobTempTran, szTableName, "", 6);

	memset(&srAll, 0x00, sizeof(srAll));
	if (!memcmp(szTRTFName, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)))
	{
		inRetVal = inNCCC_Ticket_Table_Link_TRec(&pobTempTran, &srAll, _LS_READ_);
	}
	else
	{
		inRetVal = inSqlite_Table_Link_BRec(&pobTempTran, &srAll, _LS_READ_);
	}
	
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	if (!memcmp(szTRTFName, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)))
	{
		sprintf(szQuerySql, "SELECT lnInvNum FROM %s ORDER BY lnInvNum DESC LIMIT 1", szTableName);
	}
	else
	{
		sprintf(szQuerySql, "SELECT lnOrgInvNum FROM %s ORDER BY lnOrgInvNum DESC LIMIT 1", szTableName);
	}
	
	inRetVal = inSqlite_Get_Data_By_External_SQL(gszTranDBPath, &srAll, szQuerySql);
	
	if (inRetVal == VS_SUCCESS)
	{
		if (!memcmp(szTRTFName, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)))
		{
			sprintf(szInvNum, "%06ld", pobTempTran.srTRec.lnInvNum);
		}
		else
		{
			sprintf(szInvNum, "%06ld", pobTempTran.srBRec.lnOrgInvNum);
		}
		return (VS_SUCCESS);
	}
	else
	{
		if (!memcmp(szTRTFName, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)))
		{
			sprintf(szInvNum, "%06ld", pobTempTran.srTRec.lnInvNum);
		}
		else
		{
			sprintf(szInvNum, "%06ld", pobTempTran.srBRec.lnOrgInvNum);
		}
		return (VS_ERROR);
	}
}

/*
Function	:inBATCH_Reprint_Data_Prepare
Date&Time	:2022/5/5 下午 5:51
Describe        :將資料放進RePrintData內
*/
int inBATCH_Reprint_Data_Prepare(TRANSACTION_OBJECT *pobTran, DUTYFREE_REPRINT_DATA* srReprintData)
{
	int		i = 0, j = 0;
	int		inCardLen = 0;
	char		szPrintBuf[62 + 1] = {0};
	char		szTemplate1[42 + 1] = {0};
	char		szTemplate2[42 + 1] = {0};
	char		szDispMsg [50 + 1] = {0};
	char		szFuncEnable[1 + 1] = {0};
	RTC_NEXSYS	srRTC = {0};
	
	/* 1.Print INV */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

	if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
		sprintf(srReprintData->szINV_Data, "INV: %06ld", pobTran->srBRec.lnOrgInvNum);
	else
		sprintf(srReprintData->szINV_Data, "*INV: %06ld", pobTran->srBRec.lnOrgInvNum);

	/* 2.Print Amount */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
	{
		switch (pobTran->srBRec.inCode)
		{
			case _SALE_:
			case _INST_SALE_ :
			case _REDEEM_SALE_ :
			case _MAIL_ORDER_ :
			case _CUP_MAIL_ORDER_ :
			case _SALE_OFFLINE_ :
			case _PRE_COMP_ :
			case _PRE_AUTH_ :
			case _CUP_SALE_ :
			case _CUP_PRE_COMP_ :
			case _CUP_PRE_AUTH_ :
			case _FISC_SALE_ :
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
				break;
			case _TIP_ :
				sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
				break;
			case _REFUND_ :
			case _INST_REFUND_ :
			case _REDEEM_REFUND_ :
			case _CUP_REFUND_ :
			case _CUP_MAIL_ORDER_REFUND_ :
			case _FISC_REFUND_ :
				sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
				break;
			case _INST_ADJUST_ :
			case _REDEEM_ADJUST_ :
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
				break;
			case _ADJUST_ :
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
				break;
			default :
				break;
		} /* End switch () */
	}
	else
	{
		switch (pobTran->srBRec.inOrgCode)
		{
			 case _SALE_:
			case _INST_SALE_ :
			case _REDEEM_SALE_ :
			case _MAIL_ORDER_ :
			case _CUP_MAIL_ORDER_ :
			case _SALE_OFFLINE_ :
			case _PRE_COMP_ :
			case _PRE_AUTH_ :
			case _CUP_SALE_ :
			case _CUP_PRE_COMP_ :
			case _CUP_PRE_AUTH_ :
			case _FISC_SALE_ :
				sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
				break;
			/* NCCC小費不能取消 */
//	                                case _TIP_ :
//	                                        sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
//	                                        break;
			case _REFUND_ :
			case _INST_REFUND_ :
			case _REDEEM_REFUND_ :
			case _CUP_REFUND_ :
			case _CUP_MAIL_ORDER_REFUND_ :
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
				break;
			case _INST_ADJUST_ :
			case _REDEEM_ADJUST_ :
				sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
				break;
			case _ADJUST_ :
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
				break;
			default :
				break;
		} /* End switch () */
	}
	inFunc_Amount_Comma(szTemplate1, "NT$" , 0x00, _SIGNED_NONE_, 16, _PADDING_RIGHT_);
	strcat(szPrintBuf, szTemplate1);
	sprintf(srReprintData->szAmount, "%s", szPrintBuf);

	/* 3.Transaction Type */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
	sprintf(szPrintBuf, "%s %s", szTemplate1, szTemplate2);
	sprintf(srReprintData->szTransType, "%s", szPrintBuf);

	/* 4.Print Card Type */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "%s", pobTran->srBRec.szCardLabel);
	sprintf(srReprintData->szPrintCardType, "%s", szDispMsg);

	/* 5.Print Card Number */
	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
	{
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		strcpy(szTemplate1, pobTran->srBRec.szPAN);
		if (pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
		{
			inCardLen = strlen(szTemplate1);

			for (j = 6; j < (inCardLen - 4); j ++)
				szTemplate1[j] = 0x2A;
		}
	}
	else
	{
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		strcpy(szTemplate1, pobTran->srBRec.szPAN);

		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		/* 卡號遮掩(一般卡號前6後4，U Card前3後5) */
		if (!memcmp(szTemplate1, "9552", 4))
		{
			/* HAPPG_GO 卡不掩飾 */
		}
		else
		{
			for (i = 6; i < (strlen(szTemplate1) - 4); i ++)
			{
				szTemplate1[i] = 0x2A;
			}
		}

		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 19, _PADDING_RIGHT_);
	}
	sprintf(srReprintData->szPrintPAN, "%s", szTemplate1);

	/* Data & Time */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "DATE: %.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
	sprintf(srReprintData->szDate, "%s", szPrintBuf);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "TIME: %.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
	sprintf(srReprintData->szTime, "%s", szPrintBuf);
	
	/* Approved No. & Check No.*/
	/* SmartPay印調單編號 */
	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "RRN NO.:%s", pobTran->srBRec.szFiscRRN);
	}
	else
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "APPR: %s", pobTran->srBRec.szAuthCode);
	}
	sprintf(srReprintData->szAuthCode, "%s", szPrintBuf);
	
	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
	{
		/* SmartPay不印檢查碼 */
	}
	else
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		if (strlen(pobTran->srBRec.szCheckNO) > 0)
		{
			strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
		}
		else
		{
			inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
		}
		sprintf(srReprintData->szChekNo, "No.: %s", szTemplate1);
	}
	
	/* RESPONSE CODE */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetCUPFuncEnable(szFuncEnable);
	if (szFuncEnable[0] == 'Y')
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcat(szPrintBuf, pobTran->srBRec.szRespCode);
		sprintf(srReprintData->szReponseCode, "RESPONSE CODE: %s", szPrintBuf);
	}
	else
	{
		/* 沒開啟銀聯功能，則不印櫃號 */
	}
	
	/* Store ID */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetStoreIDEnable(szFuncEnable);
	if (szFuncEnable[0] == 'Y')
	{
		/*開啟櫃號功能*/
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "STORE ID: %s", pobTran->srBRec.szStoreID);
		sprintf(srReprintData->szStoreID, "%s", szPrintBuf);
	}
	else
	{
	       /* 沒開啟櫃號功能，則不印櫃號 */
	}

	/* 列印時間 */
	inFunc_GetSystemDateAndTime(&srRTC);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "DATE: 20%02d/%02d/%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
	sprintf(srReprintData->szReprintDate, "%s", szPrintBuf);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "TIME: %02d:%02d", srRTC.uszHour, srRTC.uszMinute);
	sprintf(srReprintData->szReprintTime, "%s", szPrintBuf);
	
	return (VS_SUCCESS);
}

/*
Function	:inBATCH_Reprint_Data_Save
Date&Time	:2022/5/5 下午 5:51
Describe        :將ReprintData結構存進資料庫內
*/
int inBATCH_Reprint_Data_Save(TRANSACTION_OBJECT *pobTran, DUTYFREE_REPRINT_DATA* srReprintData)
{
	char			szTableName[20 + 1] = {0};
	char			szTRTFileName[12 + 1] = {0};
	SQLITE_ALL_TABLE	srAll;
	
	inGetTRTFileName(szTRTFileName);
	if (!memcmp(szTRTFileName, _TRT_FILE_NAME_CREDIT_, max(strlen(szTRTFileName), strlen(_TRT_FILE_NAME_CREDIT_))))
	{
		sprintf(szTableName, _TABLE_NAME_REPRINT_NCCC_);
	}
	else if (!memcmp(szTRTFileName, _TRT_FILE_NAME_DCC_, max(strlen(szTRTFileName), strlen(_TRT_FILE_NAME_DCC_))))
	{
		sprintf(szTableName, _TABLE_NAME_REPRINT_DCC_);
	}
	else
	{
		sprintf(szTableName, _TABLE_NAME_REPRINT_NCCC_);
	}
	
	/* 建立Table */
	inSqlite_Create_Table(gszReprintDBPath, szTableName, TABLE_REPRINT_DATA_TAG);
	
	/* 存重印帳單 */
	memset(&srAll, 0x00, sizeof(SQLITE_ALL_TABLE));
	inBatch_Table_Link_Reprint_Data(srReprintData, &srAll, _LS_INSERT_);
	inSqlite_Insert_Record(gszReprintDBPath, szTableName, &srAll);
	
	return (VS_SUCCESS);
}

/*
Function        :inBatch_Table_Link_Reprint_Data
Date&Time       :2022/5/10 下午 4:01
Describe        :
*/
int inBatch_Table_Link_Reprint_Data(DUTYFREE_REPRINT_DATA* srReprintData, SQLITE_ALL_TABLE *srAll, int inLinkState)
{
	SQLITE_INT32T_TABLE TABLE_BATCH_INT[] = 
	{
		{0	,"inTableID"			,&srReprintData->inTableID				},	/* inTableID */
		{0	,""				,NULL						}	/* 這行用Null用來知道尾端在哪 */
		
	};
	
	SQLITE_INT32T_TABLE TABLE_BATCH_INT64T[] = 
	{
		{0	,""				,NULL						}	/* 這行用Null用來知道尾端在哪 */
	};
	
	SQLITE_CHAR_TABLE TABLE_BATCH_CHAR[] =
	{	
		{0	,""				,NULL						,0						}	/* 這行用Null用來知道尾端在哪 */
	};
	
	SQLITE_CHAR_TABLE TABLE_BATCH_TEXT[] =
	{
		{0	,"szINV_Data"			,srReprintData->szINV_Data			,strlen(srReprintData->szINV_Data)		},	/* *INV: %06d */
		{0	,"szAmount"			,srReprintData->szAmount			,strlen(srReprintData->szAmount)		},	/* 2.Print Amount */
		{0	,"szTransType"			,srReprintData->szTransType			,strlen(srReprintData->szTransType)		},	/* 3.Transaction Type */
		{0	,"szPrintCardType"		,srReprintData->szPrintCardType			,strlen(srReprintData->szPrintCardType)		},	/* 4.Print Card Type */
		{0	,"szPrintPAN"			,srReprintData->szPrintPAN			,strlen(srReprintData->szPrintPAN)		},	/* 5.Print Card Number */
		{0	,"szDate"			,srReprintData->szDate				,strlen(srReprintData->szDate)			},	/* Data */
		{0	,"szTime"			,srReprintData->szTime				,strlen(srReprintData->szTime)			},	/* Time */
		{0	,"szAuthCode"			,srReprintData->szAuthCode			,strlen(srReprintData->szAuthCode)		},	/* Approved No.*/
		{0	,"szChekNo"			,srReprintData->szChekNo			,strlen(srReprintData->szChekNo)		},	/* Check No.*/
		{0	,"szReponseCode"		,srReprintData->szReponseCode			,strlen(srReprintData->szReponseCode)		},	/* RESPONSE CODE */
		{0	,"szStoreID"			,srReprintData->szStoreID			,strlen(srReprintData->szStoreID)		},	/* Store ID */
		{0	,"szReprintDate"		,srReprintData->szReprintDate			,strlen(srReprintData->szReprintDate)		},	/* 列印時間(執行重印的時間) */
		{0	,"szReprintTime"		,srReprintData->szReprintTime			,strlen(srReprintData->szReprintTime)		},	/* 列印時間(執行重印的時間) */
		
		{0	,""				,NULL						,0						}	/* 這行用Null用來知道尾端在哪 */
	};
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBatch_Table_Link_Reprint_Data()_START");
        }
	
	SQLITE_LINK_TABLE	srLink;
	memset(&srLink, 0x00, sizeof(SQLITE_LINK_TABLE));
	srLink.psrInt = TABLE_BATCH_INT;
	srLink.psrInt64t = TABLE_BATCH_INT64T;
	srLink.psrChar = TABLE_BATCH_CHAR;
	srLink.psrText = TABLE_BATCH_TEXT;
	
	inSqlite_Table_Link(srAll, inLinkState, &srLink);
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBatch_Table_Link_Reprint_Data()_END");
                inLogPrintf(AT, "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inBatch_Table_Link_Reprint_Title
Date&Time       :2022/5/10 下午 4:01
Describe        :
*/
int inBatch_Table_Link_Reprint_Title(DUTYFREE_REPRINT_TITLE* srReprintTitle, SQLITE_ALL_TABLE *srAll, int inLinkState)
{
	SQLITE_INT32T_TABLE TABLE_BATCH_INT[] = 
	{
		{0	,"inTableID"			,&srReprintTitle->inTableID				},	/* inTableID */
		{0	,""				,NULL						}	/* 這行用Null用來知道尾端在哪 */
		
	};
	
	SQLITE_INT32T_TABLE TABLE_BATCH_INT64T[] = 
	{
		{0	,""				,NULL						}	/* 這行用Null用來知道尾端在哪 */
	};
	
	SQLITE_CHAR_TABLE TABLE_BATCH_CHAR[] =
	{
		{0	,""				,NULL						,0							}
	};
	
	SQLITE_CHAR_TABLE TABLE_BATCH_TEXT[] =
	{
		{0	,"szSettleReprintNCCCEnable"	,srReprintTitle->szSettleReprintNCCCEnable	,strlen(srReprintTitle->szSettleReprintNCCCEnable)	},
		{0	,"szSettleReprintDCCEnable"	,srReprintTitle->szSettleReprintDCCEnable	,strlen(srReprintTitle->szSettleReprintDCCEnable)	},
		{0	,"szNCCCReprintTitleTime"	,srReprintTitle->szNCCCReprintTitleDateTime	,strlen(srReprintTitle->szNCCCReprintTitleDateTime)	},
		{0	,"szDCCReprintTitleTime"	,srReprintTitle->szDCCReprintTitleDateTime	,strlen(srReprintTitle->szDCCReprintTitleDateTime)	},
		{0	,"szNCCCReprintBatchNum"	,srReprintTitle->szNCCCReprintBatchNum		,strlen(srReprintTitle->szNCCCReprintBatchNum)		},
		{0	,"szDCCReprintBatchNum"		,srReprintTitle->szDCCReprintBatchNum		,strlen(srReprintTitle->szDCCReprintBatchNum)		},
		{0	,"szTMSUpdateSuccessNum"	,srReprintTitle->szTMSUpdateSuccessNum		,strlen(srReprintTitle->szTMSUpdateSuccessNum)		},
		{0	,"szTMSUpdateFailNum"		,srReprintTitle->szTMSUpdateFailNum		,strlen(srReprintTitle->szTMSUpdateFailNum)		},
		{0	,""				,NULL						,0							}
	};
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBatch_Table_Link_Reprint_Title()_START");
        }
	
	SQLITE_LINK_TABLE	srLink;
	memset(&srLink, 0x00, sizeof(SQLITE_LINK_TABLE));
	srLink.psrInt = TABLE_BATCH_INT;
	srLink.psrInt64t = TABLE_BATCH_INT64T;
	srLink.psrChar = TABLE_BATCH_CHAR;
	srLink.psrText = TABLE_BATCH_TEXT;
	
	inSqlite_Table_Link(srAll, inLinkState, &srLink);
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBatch_Table_Link_Reprint_Title()_END");
                inLogPrintf(AT, "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function	:inBATCH_Get_ATS_Batch_Still_Not_Upload
Date&Time	:2023/5/30 上午 10:32
Describe        :取得尚未被標記已上傳的紀錄
*/
int inBATCH_Get_ATS_Batch_Still_Not_Upload(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = VS_SUCCESS;
	char			szQuerySql[500 + 1] = {0};
	char			szTableName[50 + 1] = {0};		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	char			szDebugMsg[100 + 1] = {0};
	SQLITE_ALL_TABLE	srAll;
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);

	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inSqlite_Table_Link_BRec(pobTran, &srAll, _LS_READ_);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Link 失敗");
			inLogPrintf(AT, szDebugMsg);
		}

		return (VS_ERROR);
	}

	/* 替換資料前先清空srBRec */
	memset(&pobTran->srBRec, 0x00, sizeof(pobTran->srBRec));
	
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "SELECT * FROM %s WHERE (uszUpdated = 0) "
		"AND (uszCLSBatchBit <> X'01')"
		"AND (uszVOIDBit <> X'01') "		
		"AND (inCode <> %d) "
		"AND (inCode <> %d) "
		"AND (inCode <> %d) "
		"AND (inCode <> %d) "
		"ORDER BY lnOrgInvNum ASC LIMIT 1", szTableName, _PRE_AUTH_, _CUP_PRE_AUTH_, _LOYALTY_REDEEM_, _VOID_LOYALTY_REDEEM_);
	inRetVal = inSqlite_Get_Data_By_External_SQL(gszTranDBPath, &srAll, szQuerySql);
	
	if (inRetVal == VS_SUCCESS)
	{
		if (pobTran->srBRec.inChipStatus != 0			|| 
		    pobTran->srBRec.uszContactlessBit == VS_TRUE	|| 
		    pobTran->srBRec.uszEMVFallBackBit == VS_TRUE        ||
                    pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
		{
			inRetVal = inSqlite_Get_Batch_ByCnt_Flow(pobTran, _TN_EMV_TABLE_, 0);
			if (inRetVal != VS_SUCCESS)
				return (inRetVal);
		}
		
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}

/*
Function        :inBATCH_Update_ESC_Upload_Status_By_Sqlite
Date&Time       :2024/2/17 下午 4:46
Describe        :更新ESC上傳狀態
*/
int inBATCH_Update_ESC_Upload_Status_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szTableName[50 + 1] = {0};
	char	szUpdateSQL[300 + 1] = {0};
	SQLITE_ALL_TABLE	srAll;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inBATCH_Update_ESC_Upload_Status_By_Sqlite() START !");
	}
	
	memset(&srAll, 0x00, sizeof(srAll));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	
	memset(szUpdateSQL, 0x00, sizeof(szUpdateSQL));
	sprintf(szUpdateSQL, "UPDATE %s SET inESCUploadStatus = %d WHERE inTableID IN (SELECT inTableID FROM %s WHERE lnOrgInvNum = %ld ORDER BY inTableID DESC LIMIT 1)", szTableName, pobTran->srBRec.inESCUploadStatus, szTableName, pobTran->srBRec.lnOrgInvNum);
	
	inRetVal = inSqlite_Get_Data_By_External_SQL(gszTranDBPath, &srAll, szUpdateSQL);
	if (inRetVal != VS_SUCCESS)
	{
		inUtility_StoreTraceLog_OneStep("inBATCH_Update_ESC_Upload_Status_By_Sqlite update failed");
		inUtility_StoreTraceLog_OneStep(szUpdateSQL);
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inBATCH_Update_ESC_Upload_Status_By_Sqlite() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inBATCH_Update_NoSignature_By_Sqlite
Date&Time       :2024/10/23 上午 11:24
Describe        :更新免簽名狀態
*/
int inBATCH_Update_NoSignature_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szTableName[50 + 1] = {0};
	char	szUpdateSQL[300 + 1] = {0};
	SQLITE_ALL_TABLE	srAll = {};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inBATCH_Update_NoSignature_By_Sqlite() START !");
	}
	
	memset(&srAll, 0x00, sizeof(srAll));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
	
	memset(szUpdateSQL, 0x00, sizeof(szUpdateSQL));
	sprintf(szUpdateSQL, "UPDATE %s SET uszNoSignatureBit = X'%02X' WHERE inTableID IN (SELECT inTableID FROM %s WHERE lnOrgInvNum = %ld ORDER BY inTableID DESC LIMIT 1)", szTableName, pobTran->srBRec.uszNoSignatureBit, szTableName, pobTran->srBRec.lnOrgInvNum);
	
	inRetVal = inSqlite_Get_Data_By_External_SQL(gszTranDBPath, &srAll, szUpdateSQL);
	if (inRetVal == VS_SUCCESS ||
	    inRetVal == VS_NO_RECORD)
	{
		/* 若update的狀態前後相同，不會出現結果，故回傳NO_Record */
	}
	else
	{
		inUtility_StoreTraceLog_OneStep("inBATCH_Update_NoSignature_By_Sqlite update failed");
		inUtility_StoreTraceLog_OneStep(szUpdateSQL);
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inBATCH_Update_NoSignature_By_Sqlite() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inBATCH_FuncUpdateTxnRecord_Trust_By_Sqlite
Date&Time       :2025/10/7 上午 11:13
Describe        :用來記錄最後一筆簽單
*/
int inBATCH_FuncUpdateTxnRecord_Trust_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inBATCH_FuncUpdateTxnRecord_Trust_By_Sqlite()_START");
	}
	
	vdUtility_SYSFIN_LogMessage(AT, "_TRUST_UPDATE_BATCH_START_($:%ld)", pobTran->srTrustRec.lnTxnAmount);
	
	inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_BATCH_TABLE_TRUST_);
	if (inRetVal != VS_SUCCESS)
	{
		inFunc_EDCLock(AT);
	}
	
	/* Insert批次 */
	inRetVal = inBATCH_Insert_All_Flow(pobTran, _TN_BATCH_TABLE_TRUST_);
	if (inRetVal != VS_SUCCESS)
	{
		inFunc_EDCLock(AT);
	}
	
	/* 移除最早一筆 */
	
	/*[260318_BUG] 因使用原廠API不需再進行同步,移除檔案同步函式 */
	/* 客製化098，儲存最後一筆成功唯一碼 */
	inFLOW_RunFunction(pobTran, _FUNCTION_SAVE_LAST_UNIQUE_NO_);

	vdUtility_SYSFIN_LogMessage(AT, "_TRUST_UPDATE_BATCH_END_($:%ld)", pobTran->srTrustRec.lnTxnAmount);

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inBATCH_FuncUpdateTxnRecord_Trust_By_Sqlite()_END");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_GetTransRecord_Trust_By_Sqlite
Date&Time       :2025/10/14 上午 10:44
Describe        :讀出pobTran->srBRec.lnOrgInvNum的紀錄
*/
int inBATCH_GetTransRecord_Trust_By_Sqlite(TRANSACTION_OBJECT *pobTran)
{
	int                     inRetVal = VS_SUCCESS;
	char			szQuerySql[200 + 1] = {0};
        char                    szTableName[50 + 1] = {0};		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	SQLITE_ALL_TABLE	srAll = {};
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inBATCH_GetTransRecord_Trust_By_Sqlite()_START");
        }
        
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inNCCC_Trust_Table_Link_TrustRec(pobTran, &srAll, _LS_READ_);
	
        /* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);
        
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "SELECT * FROM %s ORDER BY inTableID DESC LIMIT 1", szTableName);
	
	inRetVal = inSqlite_Get_Data_By_External_SQL(gszTranDBPath, &srAll, szQuerySql);
	if (inRetVal != VS_SUCCESS)
		return (inRetVal);
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inBATCH_GetTransRecord_Trust_By_Sqlite()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        return (inRetVal);
}