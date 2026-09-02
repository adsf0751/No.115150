#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <ctos_clapi.h>
#include <sqlite3.h>
#include <libxml/parser.h>
#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/AllStruct.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DispMsg.h"
#include "../SOURCE/DISPLAY/DisTouch.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/COMM/Comm.h"
#include "../SOURCE/COMM/Modem.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/ASMC.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/Batch.h"
#include "../SOURCE/FUNCTION/File.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/CDT.h"
#include "../SOURCE/FUNCTION/Card.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/ECR.h"
#include "../SOURCE/FUNCTION/RS232.h"
#include "../SOURCE/FUNCTION/IPASSDT.h"
#include "../SOURCE/FUNCTION/ECCDT.h"
#include "../SOURCE/FUNCTION/ICASHDT.h"
#include "../SOURCE/FUNCTION/TDT.h"
#include "../SOURCE/FUNCTION/Utility.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../CREDIT/Creditfunc.h"
#include "../FISC/NCCCfisc.h"
#include "../EMVSRC/EMVsrc.h"
#include "../IPASS/IPASSFunc.h"
#include "../IPASS/IPassMicroPayment.h"
#include "../IPASS/IPassMicroPayment_ErrCode.h"
#include "../CTLS/CTLS.h"
#ifndef	_LOAD_KEY_AP_
	#include "../ECC/ICER/stdAfx.h"
	#include "../ECC/ICER/libutil.h"
#endif
#include "../ECC/ECC.h"
#include "../ICASH/ICASHFunc.h"
#include "NCCCsrc.h"
#include "NCCCtSAM.h"
#include "NCCCtmk.h"
#include "NCCCTicketIso.h"
#include "NCCCTicketSrc.h"

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))

extern  int		ginDebug;		/* Debug使用 extern */
extern  int		ginISODebug;		/* Debug使用 extern */
extern	int		ginDisplayDebug;	/* Debug使用 extern */
extern	int		ginFindRunTime;
extern	unsigned long	gulDemoTicketPoint;
extern	SQLITE_RESULT	gsrResult;		/* For特殊情況使用，若一次要Get多筆Record，只Get一次，待使用完後在Call sqlite3_free_table() */
extern	sqlite3		*gsrDBConnection;	/* 建立到資料庫的connection */
extern	sqlite3_stmt	*gsrSQLStat;
extern	unsigned char	guszSlot1InBit;
extern	unsigned char	guszSlot2InBit;
extern	unsigned char	guszSlot3InBit;
extern	unsigned char	guszSlot4InBit;
extern	char		gszTranDBPath[100 + 1];
extern	int		ginFallback;
extern	int		ginMachineType;
extern	int		ginEnormousNowCnt;
extern	int		ginHalfLCD;
extern	unsigned char	guszEnormousNoNeedResetBit;
extern	unsigned char	gusztSAMKeyIndex;
extern	unsigned char	gusztSAMCheckSum_56[4 + 1];
extern	unsigned char	guszNoChooseHostBit;

int	ginESVCHostIndex = -1;

SQLITE_TAG_TABLE TABLE_TICKET_TAG[] = 
{
	{"inTableID"			,"INTEGER"	,"PRIMARY KEY"	,""},	/* Table ID Primary key, sqlite table專用避免PRIMARY KEY重複 */
	{"inCode"			,"INTEGER"	,""		,""},	
	{"inPrintOption"		,"INTEGER"	,""		,""},	/* Print Option Flag (also in TCT) */
	{"inTicketType"			,"INTEGER"	,""		,""},	/* 票證種類 - 交易或明細使用  */
	{"inTDTIndex"			,"INTEGER"	,""		,""},	/* 存TDT的index */
	{"lnTxnAmount"			,"INTEGER"	,""		,""},	/* 交易金額 */
	{"lnTopUpAmount"		,"INTEGER"	,""		,""},	/* 加值金額(基底) */
	{"lnTotalTopUpAmount"		,"INTEGER"	,""		,""},	/* 加值金額 */
	{"lnCardRemainAmount"		,"INTEGER"	,""		,""},	/* 卡片餘額 */
	{"lnInvNum"			,"INTEGER"	,""		,""},	/* For簽單使用，簽單序號 */
	{"lnECCInvNum"			,"INTEGER"	,""		,""},	/* ECC Transaction Invoice # */
	{"lnSTAN"			,"INTEGER"	,""		,""},	/* */
	{"lnFinalBeforeAmt"		,"INTEGER"	,""		,""},	/* 最後交易結構，交易前卡片餘額 */
	{"lnFinalAfterAmt"		,"INTEGER"	,""		,""},	/* 最後交易結構，交易後卡片餘額 */
	{"lnMainInvNum"			,"INTEGER"	,""		,""},	/* Confirm use 電文序號，因電票中一筆交易可能有數筆電文，所以簽單序號和電文序號分開 */
	{"lnCountInvNum"		,"INTEGER"	,""		,""},	/* Confirm use 若有advice，要預先跳過的電文序號 */
	
	{"szUID"			,"BLOB"		,""		,""},	/* 卡號 or UID number */
	{"szDate"			,"BLOB"		,""		,""},	/* YYMMDD */
	{"szOrgDate"			,"BLOB"		,""		,""},	/* YYMMDD */
	{"szTime"			,"BLOB"		,""		,""},	/* HHMMSS */
	{"szOrgTime"			,"BLOB"		,""		,""},	/* HHMMSS */
	{"szAuthCode"			,"BLOB"		,""		,""},	/* */
	{"szECCAuthCode"		,"BLOB"		,""		,""},	
	{"szRespCode"			,"BLOB"		,""		,""},	/* Response Code */
	{"szProductCode"		,"BLOB"		,""		,""},	/* 產品代碼  */
	{"szTicketRefundCode"		,"BLOB"		,""		,""},	/* 退貨序號  */
	{"szTicketRefundDate"		,"BLOB"		,""		,""},	/* MMDD */
	{"szStoreID"			,"BLOB"		,""		,""},	/* */
	{"szRefNo"			,"BLOB"		,""		,""},	/* Reference Number(RRN) */
	{"szAwardNum"			,"BLOB"		,""		,""},	/* 優惠個數 */
	{"szAwardSN"			,"BLOB"		,""		,""},	/* 優惠序號(Award S/N) TID(8Bytes)+YYYYMMDDhhmmss(16 Bytes)，共22Bytes */
	{"uszAutoTopUpBit"		,"BLOB"		,""		,""},	/* 是否自動加值 */
	{"uszBlackListBit"		,"BLOB"		,""		,""},	/* 是否在黑名單中 */
	{"uszOfflineBit"		,"BLOB"		,""		,""},	/* 離線交易 */
	{"uszTicketConnectBit"		,"BLOB"		,""		,""},	/* 是否連線中 */
	{"uszResponseBit"		,"BLOB"		,""		,""},	/* 票值回覆用 */
	{"uszCloseAutoADDBit"		,"BLOB"		,""		,""},	/* 關閉自動加值用 */
	{"uszStopPollBit"		,"BLOB"		,""		,""},	/* Mifare Stop */
	{"uszConfirmBit"		,"BLOB"		,""		,""},	/* IPASS Confirm Inv use */
	{"uszESVCTransBit"		,"BLOB"		,""		,""},	/* 代表是電票交易 */
	{"uszRewardSuspendBit"		,"BLOB"		,""		,""},	/* 暫停優惠服務(優惠活動為0且要印補充資訊) */
	{"uszRewardL1Bit"		,"BLOB"		,""		,""},	/* 要印L1 */
	{"uszRewardL2Bit"		,"BLOB"		,""		,""},	/* 要印L2 */
	{"uszRewardL5Bit"		,"BLOB"		,""		,""},	/* 要印L5 */
	{"uszMPASTransBit"		,"BLOB"		,""		,""},	/* 是否為小額交易 */
	{"uszMPASReprintBit"		,"BLOB"		,""		,""},	/* 是否可重印 */
	{"srIPASSRec"			,"BLOB"		,""		,""},	/* IPASS結構，求快先整個存 */
	{"srECCRec"			,"BLOB"		,""		,""},	/* ECC結構，求快先整個存 */
	{"srICASHRec"			,"BLOB"		,""		,""},	/* ICASH結構，求快先整個存 */
	{"uszUpdated"			,"BLOB"		,""		,"DEFAULT 0"},	/* For SQLite使用，pobTran中不存，若設為1則代表該紀錄已不用，初始值設為0 */
	{""},
};

int IPASS_DEDUCT_TRT_TABLE[] =
{
	_TICKET_INITIAL_LIGHT_STATE_,
	_TICKET_CHECK_TRANS_ENABLE_,
	_TICKET_FUNCTION_MUST_SETTLE_CHECK_,
	_TICKET_IPASS_QUERYING_FLOW_,
	_TICKET_GET_PARM_,
	_TICKET_IPASS_TAPCARD_FIRST_,
	_TICKET_CONNECT_,
	_TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
	_TICKET_FUNCTION_BUILD_AND_SEND_PACKET_,
	_TICKET_IPASS_TAPCARD_SECOND_,
	_TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
	_TICKET_FUNCTION_BUILD_AND_SEND_PACKET_,
	_NCCC_FUNCTION_SAVE_TEMP_POBTRAN_FOR_ECR_MISSING_DATA_,
	_NCCC_FUNCTION_BEEP_AFTER_AUTH_,
	_TICKET_DISCONNECT_,
	_TICKET_GET_DAVTI_PARM_,
	_TICKET_UPDATE_ACCUM_,
	_TICKET_UPDATE_BATCH_,
	_TICKET_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_NCCC_FUNCTION_SEND_ECR_AFTER_PRINT_RECEIPT_,
	_NCCC_FUNCTION_RECEIVE_EI_FLOW_,
        _FLOW_NULL_,
};

int IPASS_REFUND_TRT_TABLE[] =
{
	_TICKET_INITIAL_LIGHT_STATE_,
	_TICKET_CHECK_TRANS_ENABLE_,
	_TICKET_FUNCTION_MUST_SETTLE_CHECK_,
	_TICKET_GET_PARM_,
	_TICKET_IPASS_TAPCARD_FIRST_,
	_TICKET_CONNECT_,
	_TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
	_TICKET_FUNCTION_BUILD_AND_SEND_PACKET_,
	_TICKET_IPASS_TAPCARD_SECOND_,
	_TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
	_TICKET_FUNCTION_BUILD_AND_SEND_PACKET_,
	_NCCC_FUNCTION_SAVE_TEMP_POBTRAN_FOR_ECR_MISSING_DATA_,
	_NCCC_FUNCTION_BEEP_AFTER_AUTH_,
	_TICKET_DISCONNECT_,
	_TICKET_GET_DAVTI_PARM_,
	_TICKET_UPDATE_ACCUM_,
	_TICKET_UPDATE_BATCH_,
	_TICKET_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_NCCC_FUNCTION_SEND_ECR_AFTER_PRINT_RECEIPT_,
	_NCCC_FUNCTION_RECEIVE_EI_FLOW_,
        _FLOW_NULL_,
};

int IPASS_INQUIRY_TRT_TABLE[] =
{
	_TICKET_INITIAL_LIGHT_STATE_,
	_TICKET_CHECK_TRANS_ENABLE_,
	_TICKET_FUNCTION_MUST_SETTLE_CHECK_,
	_TICKET_GET_PARM_,
	_TICKET_IPASS_TAPCARD_FIRST_,
	_TICKET_CHECK_ACK_HOST_,
	_TICKET_INQUIRY_BEEP_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
        _FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_NCCC_FUNCTION_SEND_ECR_AFTER_PRINT_RECEIPT_,
	_NCCC_FUNCTION_RECEIVE_EI_FLOW_,
        _FLOW_NULL_,
};

int IPASS_TOP_UP_TRT_TABLE[] =
{
	_TICKET_INITIAL_LIGHT_STATE_,
	_TICKET_CHECK_TRANS_ENABLE_,
	_TICKET_FUNCTION_MUST_SETTLE_CHECK_,
	_TICKET_GET_PARM_,
	_TICKET_IPASS_TAPCARD_FIRST_,
	_TICKET_CONNECT_,
	_TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
	_TICKET_FUNCTION_BUILD_AND_SEND_PACKET_,
	_TICKET_IPASS_TAPCARD_SECOND_,
	_TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
	_TICKET_FUNCTION_BUILD_AND_SEND_PACKET_,
	_NCCC_FUNCTION_SAVE_TEMP_POBTRAN_FOR_ECR_MISSING_DATA_,
	_NCCC_FUNCTION_BEEP_AFTER_AUTH_,
	_TICKET_DISCONNECT_,
	_TICKET_GET_DAVTI_PARM_,
	_TICKET_UPDATE_ACCUM_,
	_TICKET_UPDATE_BATCH_,
	_TICKET_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_NCCC_FUNCTION_SEND_ECR_AFTER_PRINT_RECEIPT_,
	_NCCC_FUNCTION_RECEIVE_EI_FLOW_,
        _FLOW_NULL_,
};

int IPASS_VOID_TOP_UP_TRT_TABLE[] =
{
	_TICKET_INITIAL_LIGHT_STATE_,
	_TICKET_CHECK_TRANS_ENABLE_,
	_TICKET_FUNCTION_MUST_SETTLE_CHECK_,
	_TICKET_GET_PARM_,
	_TICKET_IPASS_TAPCARD_FIRST_,
	_TICKET_CONNECT_,
	_TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
	_TICKET_FUNCTION_BUILD_AND_SEND_PACKET_,
	_TICKET_IPASS_TAPCARD_SECOND_,
	_TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
	_TICKET_FUNCTION_BUILD_AND_SEND_PACKET_,
	_NCCC_FUNCTION_SAVE_TEMP_POBTRAN_FOR_ECR_MISSING_DATA_,
	_NCCC_FUNCTION_BEEP_AFTER_AUTH_,
	_TICKET_DISCONNECT_,
	_TICKET_GET_DAVTI_PARM_,
	_TICKET_UPDATE_ACCUM_,
	_TICKET_UPDATE_BATCH_,
	_TICKET_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_NCCC_FUNCTION_SEND_ECR_AFTER_PRINT_RECEIPT_,
	_NCCC_FUNCTION_RECEIVE_EI_FLOW_,
        _FLOW_NULL_,
};

int SETTLE_TRT_TABLE[] =
{
	_TICKET_GET_PARM_,
        _FUNCTION_PREPRINT_AUTO_REPORT_BY_BUFFER_,
	_TICKET_CONNECT_,
	_TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
        _TICKET_FUNCTION_BUILD_AND_SEND_PACKET_,
        _NCCC_FUNCTION_SETTLEMENT_XML_NCCC_START_,
	_NCCC_FUNCTION_BEEP_AFTER_AUTH_,
	_TICKET_DISCONNECT_,
        _FUNCTION_DELETE_BATCH_FLOW_,
        _FUNCTION_PRINT_TOTAL_REPORT_BY_BUFFER_,
        _FUNCTION_DELETE_ACCUM_FLOW_,
        _FUNCTION_REST_BATCH_INV_,
	_FUNCTION_UPDATE_BATCH_NUM_,
        _NCCC_FUNCTION_SETTLEMENT_XML_NCCC_END_,
        _FLOW_NULL_,
};

int ECC_DEDUCT_TRT_TABLE[] =
{
	_TICKET_CHECK_TRANS_ENABLE_,
	_TICKET_FUNCTION_MUST_SETTLE_CHECK_,
	_TICKET_GET_PARM_,
	_TICKET_ECC_API_FLOW_,
	_NCCC_FUNCTION_SAVE_TEMP_POBTRAN_FOR_ECR_MISSING_DATA_,
	_NCCC_FUNCTION_BEEP_AFTER_AUTH_,
	_TICKET_UPDATE_ACCUM_,
	_TICKET_UPDATE_BATCH_,
	_TICKET_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_NCCC_FUNCTION_SEND_ECR_AFTER_PRINT_RECEIPT_,
	_NCCC_FUNCTION_RECEIVE_EI_FLOW_,
        _FLOW_NULL_,
};

int ECC_REFUND_TRT_TABLE[] =
{
	_TICKET_CHECK_TRANS_ENABLE_,
	_TICKET_FUNCTION_MUST_SETTLE_CHECK_,
	_TICKET_GET_PARM_,
	_TICKET_ECC_API_FLOW_,
	_NCCC_FUNCTION_SAVE_TEMP_POBTRAN_FOR_ECR_MISSING_DATA_,
	_NCCC_FUNCTION_BEEP_AFTER_AUTH_,
	_TICKET_UPDATE_ACCUM_,
	_TICKET_UPDATE_BATCH_,
	_TICKET_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_NCCC_FUNCTION_SEND_ECR_AFTER_PRINT_RECEIPT_,
	_NCCC_FUNCTION_RECEIVE_EI_FLOW_,
        _FLOW_NULL_,
};

int ECC_INQUIRY_TRT_TABLE[] =
{
	_TICKET_CHECK_TRANS_ENABLE_,
	_TICKET_FUNCTION_MUST_SETTLE_CHECK_,
	_TICKET_GET_PARM_,
	_TICKET_ECC_API_FLOW_,
	_NCCC_FUNCTION_SAVE_TEMP_POBTRAN_FOR_ECR_MISSING_DATA_,
	_NCCC_FUNCTION_BEEP_AFTER_AUTH_,
	_TICKET_INQUIRY_BEEP_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_NCCC_FUNCTION_SEND_ECR_AFTER_PRINT_RECEIPT_,
	_NCCC_FUNCTION_RECEIVE_EI_FLOW_,
        _FLOW_NULL_,
};

int ECC_TOP_UP_TRT_TABLE[] =
{
	_TICKET_CHECK_TRANS_ENABLE_,
	_TICKET_FUNCTION_MUST_SETTLE_CHECK_,
	_TICKET_GET_PARM_,
	_TICKET_ECC_API_FLOW_,
	_NCCC_FUNCTION_SAVE_TEMP_POBTRAN_FOR_ECR_MISSING_DATA_,
	_NCCC_FUNCTION_BEEP_AFTER_AUTH_,
	_TICKET_UPDATE_ACCUM_,
	_TICKET_UPDATE_BATCH_,
	_TICKET_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_NCCC_FUNCTION_SEND_ECR_AFTER_PRINT_RECEIPT_,
	_NCCC_FUNCTION_RECEIVE_EI_FLOW_,
        _FLOW_NULL_,
};

int ECC_VOID_TOP_UP_TRT_TABLE[] =
{
	_TICKET_CHECK_TRANS_ENABLE_,
	_TICKET_FUNCTION_MUST_SETTLE_CHECK_,
	_TICKET_GET_PARM_,
	_TICKET_ECC_API_FLOW_,
	_NCCC_FUNCTION_SAVE_TEMP_POBTRAN_FOR_ECR_MISSING_DATA_,
	_NCCC_FUNCTION_BEEP_AFTER_AUTH_,
	_TICKET_UPDATE_ACCUM_,
	_TICKET_UPDATE_BATCH_,
	_TICKET_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_NCCC_FUNCTION_SEND_ECR_AFTER_PRINT_RECEIPT_,
	_NCCC_FUNCTION_RECEIVE_EI_FLOW_,
        _FLOW_NULL_,
};

int ECC_SETTLE_TRT_TABLE[] =
{
	_TICKET_GET_PARM_,
        _FUNCTION_PREPRINT_AUTO_REPORT_BY_BUFFER_,
	_TICKET_ECC_SETTLE_FLOW_,
        _NCCC_FUNCTION_SETTLEMENT_XML_NCCC_START_,
	_NCCC_FUNCTION_BEEP_AFTER_AUTH_,
	_FUNCTION_DELETE_BATCH_FLOW_,
        _FUNCTION_PRINT_TOTAL_REPORT_BY_BUFFER_,
        _FUNCTION_DELETE_ACCUM_FLOW_,
        _FUNCTION_REST_BATCH_INV_,
	_FUNCTION_UPDATE_BATCH_NUM_,
	_TICKET_ECC_SET_UPDATE_BATCH_FLAG_,
        _NCCC_FUNCTION_SETTLEMENT_XML_NCCC_END_,
        _FLOW_NULL_,
};

int ICASH_DEDUCT_TRT_TABLE[] =
{
	_TICKET_CHECK_TRANS_ENABLE_,
	_TICKET_FUNCTION_MUST_SETTLE_CHECK_,
	_TICKET_GET_PARM_,
	_TICKET_ICASH_GET_CARD_AMT_,
	_TICKET_ICASH_AUTO_ADD_FLOW_,
	_TICKET_ICASH_TAPCARD_FIRST_,
	_TICKET_ICASH_DEMO_FLOW_,		/* DEMO流程 */
	_TICKET_CONNECT_,
	_TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
	_TICKET_FUNCTION_BUILD_AND_SEND_PACKET_,
	_TICKET_ICASH_TAPCARD_SECOND_,
	_TICKET_ICASH_BUILD_SEND_RECV_PACKET_COMFIRM_,
	_TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
	_TICKET_FUNCTION_BUILD_AND_SEND_PACKET_,
	_NCCC_FUNCTION_SAVE_TEMP_POBTRAN_FOR_ECR_MISSING_DATA_,
	_NCCC_FUNCTION_BEEP_AFTER_AUTH_,
	_TICKET_DISCONNECT_,
	_TICKET_GET_DAVTI_PARM_,
	_TICKET_UPDATE_ACCUM_,
	_TICKET_UPDATE_BATCH_,
	_TICKET_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_NCCC_FUNCTION_SEND_ECR_AFTER_PRINT_RECEIPT_,
	_NCCC_FUNCTION_RECEIVE_EI_FLOW_,
	_FLOW_NULL_,
};

int ICASH_REFUND_TRT_TABLE[] =
{
	_TICKET_CHECK_TRANS_ENABLE_,
	_TICKET_FUNCTION_MUST_SETTLE_CHECK_,
	_TICKET_GET_PARM_,
	_TICKET_ICASH_GET_CARD_AMT_,
	_TICKET_ICASH_TAPCARD_FIRST_,
	_TICKET_ICASH_DEMO_FLOW_,		/* DEMO流程 */
	_TICKET_CONNECT_,
	_TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
	_TICKET_FUNCTION_BUILD_AND_SEND_PACKET_,
	_TICKET_ICASH_TAPCARD_SECOND_,
	_TICKET_ICASH_BUILD_SEND_RECV_PACKET_COMFIRM_,
	_TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
	_TICKET_FUNCTION_BUILD_AND_SEND_PACKET_,
	_NCCC_FUNCTION_SAVE_TEMP_POBTRAN_FOR_ECR_MISSING_DATA_,
	_NCCC_FUNCTION_BEEP_AFTER_AUTH_,
	_TICKET_DISCONNECT_,
	_TICKET_GET_DAVTI_PARM_,
	_TICKET_UPDATE_ACCUM_,
	_TICKET_UPDATE_BATCH_,
	_TICKET_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_NCCC_FUNCTION_SEND_ECR_AFTER_PRINT_RECEIPT_,
	_NCCC_FUNCTION_RECEIVE_EI_FLOW_,
	_FLOW_NULL_,
};

int ICASH_INQUIRY_TRT_TABLE[] =
{
	_TICKET_CHECK_TRANS_ENABLE_,
	_TICKET_FUNCTION_MUST_SETTLE_CHECK_,
	_TICKET_GET_PARM_,
	_TICKET_ICASH_GET_CARD_AMT_,
	_TICKET_ICASH_DEMO_FLOW_,
	_TICKET_INQUIRY_BEEP_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_NCCC_FUNCTION_SEND_ECR_AFTER_PRINT_RECEIPT_,
	_NCCC_FUNCTION_RECEIVE_EI_FLOW_,
        _FLOW_NULL_,
};

int ICASH_TOP_UP_TRT_TABLE[] =
{
	_TICKET_CHECK_TRANS_ENABLE_,
	_TICKET_FUNCTION_MUST_SETTLE_CHECK_,
	_TICKET_GET_PARM_,
	_TICKET_ICASH_GET_CARD_AMT_,
	_TICKET_ICASH_TAPCARD_FIRST_,
	_TICKET_ICASH_DEMO_FLOW_,		/* DEMO流程 */
	_TICKET_CONNECT_,
	_TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
	_TICKET_FUNCTION_BUILD_AND_SEND_PACKET_,
	_TICKET_ICASH_TAPCARD_SECOND_,
	_TICKET_ICASH_BUILD_SEND_RECV_PACKET_COMFIRM_,
	_TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
	_TICKET_FUNCTION_BUILD_AND_SEND_PACKET_,
	_NCCC_FUNCTION_SAVE_TEMP_POBTRAN_FOR_ECR_MISSING_DATA_,
	_NCCC_FUNCTION_BEEP_AFTER_AUTH_,
	_TICKET_DISCONNECT_,
	_TICKET_GET_DAVTI_PARM_,
	_TICKET_UPDATE_ACCUM_,
	_TICKET_UPDATE_BATCH_,
	_TICKET_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_NCCC_FUNCTION_SEND_ECR_AFTER_PRINT_RECEIPT_,
	_NCCC_FUNCTION_RECEIVE_EI_FLOW_,
	_FLOW_NULL_,
};

int ICASH_VOID_TOP_UP_TRT_TABLE[] =
{
	_TICKET_CHECK_TRANS_ENABLE_,
	_TICKET_FUNCTION_MUST_SETTLE_CHECK_,
	_TICKET_GET_PARM_,
	_TICKET_ICASH_GET_CARD_AMT_,
	_TICKET_ICASH_TAPCARD_FIRST_,
	_TICKET_ICASH_DEMO_FLOW_,		/* DEMO流程 */
	_TICKET_CONNECT_,
	_TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
	_TICKET_FUNCTION_BUILD_AND_SEND_PACKET_,
	_TICKET_ICASH_TAPCARD_SECOND_,
	_TICKET_ICASH_BUILD_SEND_RECV_PACKET_COMFIRM_,
	_TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
	_TICKET_FUNCTION_BUILD_AND_SEND_PACKET_,
	_NCCC_FUNCTION_SAVE_TEMP_POBTRAN_FOR_ECR_MISSING_DATA_,
	_NCCC_FUNCTION_BEEP_AFTER_AUTH_,
	_TICKET_DISCONNECT_,
	_TICKET_GET_DAVTI_PARM_,
	_TICKET_UPDATE_ACCUM_,
	_TICKET_UPDATE_BATCH_,
	_TICKET_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_NCCC_FUNCTION_SEND_ECR_AFTER_PRINT_RECEIPT_,
	_NCCC_FUNCTION_RECEIVE_EI_FLOW_,
	_FLOW_NULL_,
};

TRT_TABLE TRANSACTION_NCCC_TICKET_TABLE[] =
{
	{_TRT_TICKET_IPASS_DEDUCT_,		IPASS_DEDUCT_TRT_TABLE},
	{_TRT_TICKET_IPASS_REFUND_,		IPASS_REFUND_TRT_TABLE},
	{_TRT_TICKET_IPASS_INQUIRY_,		IPASS_INQUIRY_TRT_TABLE},
	{_TRT_TICKET_IPASS_TOP_UP_,		IPASS_TOP_UP_TRT_TABLE},
	{_TRT_TICKET_IPASS_VOID_TOP_UP_,	IPASS_VOID_TOP_UP_TRT_TABLE},
	
	{_TRT_TICKET_ECC_DEDUCT_,		ECC_DEDUCT_TRT_TABLE},
	{_TRT_TICKET_ECC_REFUND_,		ECC_REFUND_TRT_TABLE},
	{_TRT_TICKET_ECC_INQUIRY_,		ECC_INQUIRY_TRT_TABLE},
	{_TRT_TICKET_ECC_TOP_UP_,		ECC_TOP_UP_TRT_TABLE},
	{_TRT_TICKET_ECC_VOID_TOP_UP_,		ECC_VOID_TOP_UP_TRT_TABLE},

	{_TRT_TICKET_ICASH_DEDUCT_,		ICASH_DEDUCT_TRT_TABLE},
	{_TRT_TICKET_ICASH_REFUND_,		ICASH_REFUND_TRT_TABLE},
	{_TRT_TICKET_ICASH_INQUIRY_,		ICASH_INQUIRY_TRT_TABLE},
	{_TRT_TICKET_ICASH_TOP_UP_,		ICASH_TOP_UP_TRT_TABLE},
	{_TRT_TICKET_ICASH_VOID_TOP_UP_,	ICASH_VOID_TOP_UP_TRT_TABLE},
	
	{_TRT_SETTLE_,				SETTLE_TRT_TABLE},
	{_TRT_TICKET_ECC_SETTLE_,		ECC_SETTLE_TRT_TABLE},
        {_FLOW_NULL_,				NULL}
};

/*
Function        :inNCCC_Ticket_RunTRT
Date&Time       :2017/12/14 下午 5:33
Describe        :
*/
int inNCCC_Ticket_RunTRT(TRANSACTION_OBJECT *pobTran, int inTRTCode)
{
        int     *inTRTID = NULL;
        int     i = 0, inRetVal = VS_ERROR ;
	int	inBatchCnt = 0;
	char	szFESMode[2 + 1] = {0};
	char	szFuncEnable[2 + 1] = {0};
	char	szCFESMode[2 + 1] = {0};
	
	memset(szFESMode, 0x00, sizeof(szFESMode));
	inGetNCCCFESMode(szFESMode);
	memset(szCFESMode, 0x00, sizeof(szCFESMode));
	inGetCloud_MFES(szCFESMode);
	
	/* 結帳時，有悠遊卡就送悠遊卡流程 */
	if (inTRTCode == _TRT_SETTLE_)
	{
		inLoadTDTRec(_TDT_INDEX_01_ECC_);
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetTicket_HostEnable(szFuncEnable);
		if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
		{
			inTRTCode = _TRT_TICKET_ECC_SETTLE_;
		}
		
		/* 如果是結帳交易，要把請先結帳的Bit On起來，直到接收到host的回覆才Off*/
		inBatchCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);
		if (inBatchCnt >= 0)
		{
			inNCCC_TICKET_SetMustSettleBit(pobTran, "Y");
		}
		else
		{
			/* 開啟失敗 */
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inBATCH_GetTotalCountFromBakFile_By_Sqlite Fail:%d", inBatchCnt);
			}
			inUtility_StoreTraceLog_OneStep("inBATCH_GetTotalCountFromBakFile_By_Sqlite Fail:%d", inBatchCnt);
			
			inFunc_EDCLock(AT);
		}
	}
	
        for (i = 0 ;; i ++)
        {
		if (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0)
		{
			if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
			{
				if (TRANSACTION_NCCC_TICKET_TABLE[i].inTRTCode == inTRTCode)
				{
					inTRTID = TRANSACTION_NCCC_TICKET_TABLE[i].inTRTID;
					break;
				}
				else if (TRANSACTION_NCCC_TICKET_TABLE[i].inTRTCode == -1)
				{
					break;
				}
			}
			else
			{
				if (TRANSACTION_NCCC_TICKET_TABLE[i].inTRTCode == inTRTCode)
				{
					inTRTID = TRANSACTION_NCCC_TICKET_TABLE[i].inTRTID;
					break;
				}
				else if (TRANSACTION_NCCC_TICKET_TABLE[i].inTRTCode == -1)
				{
					break;
				}
			}
			
		}/* 現在MPAS和ATS共用電文 */
		else if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
			 memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0)
		{
			if (TRANSACTION_NCCC_TICKET_TABLE[i].inTRTCode == inTRTCode)
			{
				inTRTID = TRANSACTION_NCCC_TICKET_TABLE[i].inTRTID;
				break;
			}
			else if (TRANSACTION_NCCC_TICKET_TABLE[i].inTRTCode == -1)
			{
				break;
			}
			
		}
		else
		{
			inTRTID = NULL;
		}
                
        }

        if (inTRTID == NULL)
        {
                return (VS_ERROR);
        }

        for (i = 0 ; ; i ++)
        {
                if (inTRTID[i] == VS_ERROR)
                        break;

                inRetVal = inFLOW_RunFunction(pobTran, inTRTID[i]);

		if (inRetVal != VS_SUCCESS)
		{
			/* 悠遊卡感應失敗不跳號 */
			if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
			{

			}
			/* 目前For iPASS(退貨主機回錯誤，但需要票值回覆時) */
			else
			{
				/* 如果因錯誤中途跳出，這裡回存TicketInv */
				inNCCC_Ticket_Func_UpdatetTicketInvNum(pobTran);
			}
			break;
		}
        }
	
	/* 不論最後成功與否都要切回NCCC */
	inNCCC_tSAM_SelectAID_NCCC_Flow();
	
	/* 斷線 */
	inCOMM_End(pobTran);
	
        return (inRetVal);
}

/*
Function        :inNCCC_Ticket_Func_Check_Transaction_Deduct
Date&Time       :2017/12/19 上午 9:42
Describe        :確認全票證扣款功能開關 主要for主頁面顯示開關使用，只要有一個開就顯示可使用
*/
int inNCCC_Ticket_Func_Check_Transaction_Deduct(int inCode)
{
	char		szTransFunc[20 + 1] = {0};
	unsigned char	uszTxnEnable = VS_FALSE;
	
	/* IPASS 扣款開關 */
	inLoadTDTRec(_TDT_INDEX_00_IPASS_);
	memset(szTransFunc, 0x00, sizeof(szTransFunc));
	if (inGetTicket_HostTransFunc(szTransFunc) != VS_SUCCESS)
		return (VS_ERROR);

	if (memcmp(&szTransFunc[0], "Y", 1) == 0)
	{
		uszTxnEnable = VS_TRUE;
	}
	
	/* 悠遊卡扣款開關 */
	inLoadTDTRec(_TDT_INDEX_01_ECC_);
	memset(szTransFunc, 0x00, sizeof(szTransFunc));
	if (inGetTicket_HostTransFunc(szTransFunc) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (memcmp(&szTransFunc[0], "Y", 1) == 0)
	{
		uszTxnEnable = VS_TRUE;
	}
	
	/* 愛金卡取消加值開關 */
	inLoadTDTRec(_TDT_INDEX_02_ICASH_);
	memset(szTransFunc, 0x00, sizeof(szTransFunc));
	if (inGetTicket_HostTransFunc(szTransFunc) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (memcmp(&szTransFunc[0], "Y", 1) == 0)
	{
		uszTxnEnable = VS_TRUE;
	}
	
	if (uszTxnEnable == VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
	
}

/*
Function        :inNCCC_Ticket_Func_Check_Transaction_Refund
Date&Time       :2017/12/19 下午 1:28
Describe        :確認全票證退貨功能開關 主要for主頁面顯示開關使用，只要有一個開就顯示可使用
*/
int inNCCC_Ticket_Func_Check_Transaction_Refund(int inCode)
{
	char		szTransFunc[20 + 1] = {0};
	unsigned char	uszTxnEnable = VS_FALSE;
		
	/* IPASS 退貨開關 */
	inLoadTDTRec(_TDT_INDEX_00_IPASS_);
	memset(szTransFunc, 0x00, sizeof(szTransFunc));
	if (inGetTicket_HostTransFunc(szTransFunc) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (memcmp(&szTransFunc[2], "Y", 1) == 0)
	{
		uszTxnEnable = VS_TRUE;
	}
	
	/* 悠遊卡退貨開關 */
	inLoadTDTRec(_TDT_INDEX_01_ECC_);
	memset(szTransFunc, 0x00, sizeof(szTransFunc));
	if (inGetTicket_HostTransFunc(szTransFunc) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (memcmp(&szTransFunc[2], "Y", 1) == 0)
	{
		uszTxnEnable = VS_TRUE;
	}
	
	/* 愛金卡取消加值開關 */
	inLoadTDTRec(_TDT_INDEX_02_ICASH_);
	memset(szTransFunc, 0x00, sizeof(szTransFunc));
	if (inGetTicket_HostTransFunc(szTransFunc) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (memcmp(&szTransFunc[2], "Y", 1) == 0)
	{
		uszTxnEnable = VS_TRUE;
	}
	
	if (uszTxnEnable == VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
	
}

/*
Function        :inNCCC_Ticket_Func_Check_Transaction_Query
Date&Time       :2017/12/19 下午 1:28
Describe        :確認全票證詢卡功能開關 主要for主頁面顯示開關使用，只要有一個開就顯示可使用
*/
int inNCCC_Ticket_Func_Check_Transaction_Inquiry(int inCode)
{
	char		szTransFunc[20 + 1] = {0};
	unsigned char	uszTxnEnable = VS_FALSE;
		
	/* IPASS 詢卡開關 */
	inLoadTDTRec(_TDT_INDEX_00_IPASS_);
	memset(szTransFunc, 0x00, sizeof(szTransFunc));
	if (inGetTicket_HostTransFunc(szTransFunc) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (memcmp(&szTransFunc[7], "Y", 1) == 0)
	{
		uszTxnEnable = VS_TRUE;
	}
		
	/* 悠遊卡詢卡開關 */
	inLoadTDTRec(_TDT_INDEX_01_ECC_);
	memset(szTransFunc, 0x00, sizeof(szTransFunc));
	if (inGetTicket_HostTransFunc(szTransFunc) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (memcmp(&szTransFunc[7], "Y", 1) == 0)
	{
		uszTxnEnable = VS_TRUE;
	}
	
	/* 愛金卡取消加值開關 */
	inLoadTDTRec(_TDT_INDEX_02_ICASH_);
	memset(szTransFunc, 0x00, sizeof(szTransFunc));
	if (inGetTicket_HostTransFunc(szTransFunc) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (memcmp(&szTransFunc[7], "Y", 1) == 0)
	{
		uszTxnEnable = VS_TRUE;
	}
	
	if (uszTxnEnable == VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
	
}

/*
Function        :inNCCC_Ticket_Func_Check_Transaction_Top_Up
Date&Time       :2017/12/19 下午 1:28
Describe        :確認全票證加值功能開關 主要for主頁面顯示開關使用，只要有一個開就顯示可使用
*/
int inNCCC_Ticket_Func_Check_Transaction_Top_Up(int inCode)
{
	char		szTransFunc[20 + 1] = {0};
	unsigned char	uszTxnEnable = VS_FALSE;
	
	/* IPASS 加值開關 */
	inLoadTDTRec(_TDT_INDEX_00_IPASS_);
	memset(szTransFunc, 0x00, sizeof(szTransFunc));
	if (inGetTicket_HostTransFunc(szTransFunc) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (memcmp(&szTransFunc[4], "Y", 1) == 0)
	{
		uszTxnEnable = VS_TRUE;
	}
	
	/* 悠遊卡加值開關 */
	inLoadTDTRec(_TDT_INDEX_01_ECC_);
	memset(szTransFunc, 0x00, sizeof(szTransFunc));
	if (inGetTicket_HostTransFunc(szTransFunc) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (memcmp(&szTransFunc[4], "Y", 1) == 0)
	{
		uszTxnEnable = VS_TRUE;
	}
	
	/* 愛金卡取消加值開關 */
	inLoadTDTRec(_TDT_INDEX_02_ICASH_);
	memset(szTransFunc, 0x00, sizeof(szTransFunc));
	if (inGetTicket_HostTransFunc(szTransFunc) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (memcmp(&szTransFunc[4], "Y", 1) == 0)
	{
		uszTxnEnable = VS_TRUE;
	}
	
	if (uszTxnEnable == VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
	
}

/*
Function        :inNCCC_Ticket_Func_Check_Transaction_Void_Top_Up
Date&Time       :2017/12/19 下午 1:28
Describe        :確認全票證取消加值功能開關 主要for主頁面顯示開關使用，只要有一個開就顯示可使用
*/
int inNCCC_Ticket_Func_Check_Transaction_Void_Top_Up(int inCode)
{
	char		szTransFunc[20 + 1] = {0};
	unsigned char	uszTxnEnable = VS_FALSE;
	
	/* IPASS 取消加值開關 */
	inLoadTDTRec(_TDT_INDEX_00_IPASS_);
	memset(szTransFunc, 0x00, sizeof(szTransFunc));
	if (inGetTicket_HostTransFunc(szTransFunc) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (memcmp(&szTransFunc[5], "Y", 1) == 0)
	{
		uszTxnEnable = VS_TRUE;
	}
	
	/* 悠遊卡取消加值開關 */
	inLoadTDTRec(_TDT_INDEX_01_ECC_);
	memset(szTransFunc, 0x00, sizeof(szTransFunc));
	if (inGetTicket_HostTransFunc(szTransFunc) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (memcmp(&szTransFunc[5], "Y", 1) == 0)
	{
		uszTxnEnable = VS_TRUE;
	}
	
	/* 愛金卡取消加值開關 */
	inLoadTDTRec(_TDT_INDEX_02_ICASH_);
	memset(szTransFunc, 0x00, sizeof(szTransFunc));
	if (inGetTicket_HostTransFunc(szTransFunc) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (memcmp(&szTransFunc[5], "Y", 1) == 0)
	{
		uszTxnEnable = VS_TRUE;
	}
	
	if (uszTxnEnable == VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
	
}

/*
Function        :inNCCC_Ticket_IPASS_Func_Check_Transaction_Function
Date&Time       :2017/12/18 下午 2:11
Describe        :確認IPASS交易功能是否打開，
*/
int inNCCC_Ticket_IPASS_Func_Check_Transaction_Function(int inCode)
{
	char		szTransFunc[20 + 1];
	unsigned char	uszTxnEnable = VS_TRUE;
	
	/* 先load IPASS 參數檔 */
	inLoadTDTRec(_TDT_INDEX_00_IPASS_);	

	memset(szTransFunc, 0x00, sizeof(szTransFunc));
	if (inGetTicket_HostTransFunc(szTransFunc) != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 扣款 */
	if (inCode == _TICKET_IPASS_DEDUCT_)
	{
		if (memcmp(&szTransFunc[0], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _TICKET_IPASS_VOID_DEDUCT_)
	{
		if (memcmp(&szTransFunc[1], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _TICKET_IPASS_REFUND_)
	{
		if (memcmp(&szTransFunc[2], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _TICKET_IPASS_AUTO_TOP_UP_)
	{
		if (memcmp(&szTransFunc[3], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _TICKET_IPASS_TOP_UP_)
	{
		if (memcmp(&szTransFunc[4], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _TICKET_IPASS_VOID_TOP_UP_)
	{
		if (memcmp(&szTransFunc[5], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _TICKET_IPASS_INQUIRY_)
	{
		if (memcmp(&szTransFunc[7], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _SETTLE_)
	{
		if (memcmp(&szTransFunc[8], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	
	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inNCCC_Ticket_ECC_Func_Check_Transaction_Function
Date&Time       :2018/3/28 下午 3:27
Describe        :確認ECC交易功能是否打開，
*/
int inNCCC_Ticket_ECC_Func_Check_Transaction_Function(int inCode)
{
	char		szTransFunc[20 + 1];
	unsigned char	uszTxnEnable = VS_TRUE;
	
	/* 先load ECC 參數檔 */
	inLoadTDTRec(_TDT_INDEX_01_ECC_);	

	memset(szTransFunc, 0x00, sizeof(szTransFunc));
	if (inGetTicket_HostTransFunc(szTransFunc) != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 扣款 */
	if (inCode == _TICKET_EASYCARD_DEDUCT_)
	{
		if (memcmp(&szTransFunc[0], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _TICKET_EASYCARD_VOID_DEDUCT_)
	{
		if (memcmp(&szTransFunc[1], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _TICKET_EASYCARD_REFUND_)
	{
		if (memcmp(&szTransFunc[2], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _TICKET_EASYCARD_AUTO_TOP_UP_)
	{
		if (memcmp(&szTransFunc[3], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _TICKET_EASYCARD_TOP_UP_)
	{
		if (memcmp(&szTransFunc[4], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _TICKET_EASYCARD_VOID_TOP_UP_)
	{
		if (memcmp(&szTransFunc[5], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _TICKET_EASYCARD_INQUIRY_)
	{
		if (memcmp(&szTransFunc[7], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _SETTLE_)
	{
		if (memcmp(&szTransFunc[8], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	
	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inNCCC_Ticket_Func_Check_Transaction_Function_Flow
Date&Time       :2017/12/20 下午 5:56
Describe        :是那個Host，並用該Host檢核
*/
int inNCCC_Ticket_Func_Check_Transaction_Function_Flow(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszTxnEnable = VS_TRUE;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_Ticket_Func_Check_Transaction_Function_Flow() START !");
	}
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
	{
		inRetVal = inNCCC_Ticket_IPASS_Func_Check_Transaction_Function(pobTran->srTRec.inCode);
		if (inRetVal != VS_SUCCESS)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
		inRetVal = inNCCC_Ticket_ECC_Func_Check_Transaction_Function(pobTran->srTRec.inCode);
		if (inRetVal != VS_SUCCESS)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
	{
		inRetVal = inNCCC_Ticket_ICASH_Func_Check_Transaction_Function(pobTran->srTRec.inCode);
		if (inRetVal != VS_SUCCESS)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else
	{
		uszTxnEnable = VS_FALSE;
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Ticket_Func_Check_Transaction_Function_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;
		
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inNCCC_Ticket_Decide_Trans_Type
Date&Time       :2017/12/20 上午 11:24
Describe        :
*/
int inNCCC_Ticket_Decide_Trans_Type(TRANSACTION_OBJECT *pobTran)
{
	char	szLogOnOK[2 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};
	char	szHostIndex[2 + 1] = {0};
	char	szFuncEnable[2 + 1] = {0};
	char	szBatchNum[6 + 1] = {0};
	
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Ticket_Decide_Trans_Type START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_Ticket_Decide_Trans_Type() START !");
	}
	
	/* 票證 TRT 分開寫 */
        if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
        {
                if (inLoadTDTRec(pobTran->srTRec.inTDTIndex) < 0)
			return (VS_ERROR);
		
		/* 切換SAM卡 */
                inNCCC_tSAM_SelectAID_IPASS_Flow();
        
                /* MFT index 紀錄 HDT index */    
		memset(szHostIndex, 0x00, sizeof(szHostIndex));
		inGetTicket_HostIndex(szHostIndex);
		pobTran->srBRec.inHDTIndex = atoi(szHostIndex);
                if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) < 0)
                        return (VS_ERROR);
		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) < 0)
                        return (VS_ERROR);
                
                if (inLoadIPASSDTRec(0) < 0)
			return (VS_ERROR);        
                
		memset(szLogOnOK, 0x00, sizeof(szLogOnOK));
		inGetTicket_LogOnOK(szLogOnOK);
                if (memcmp(szLogOnOK, "Y", strlen("Y")) == 0)
                {
                        if (pobTran->inTransactionCode == _TICKET_DEDUCT_)
                        {
                                pobTran->inTransactionCode = _TICKET_IPASS_DEDUCT_;
                                pobTran->srTRec.inCode = _TICKET_IPASS_DEDUCT_;
				pobTran->inRunTRTID = _TRT_TICKET_IPASS_DEDUCT_;
                        }  
                        else if (pobTran->inTransactionCode == _TICKET_REFUND_)
                        {
                                pobTran->inTransactionCode = _TICKET_IPASS_REFUND_;
                                pobTran->srTRec.inCode = _TICKET_IPASS_REFUND_;
				pobTran->inRunTRTID = _TRT_TICKET_IPASS_REFUND_;
                        } 
                        else if (pobTran->inTransactionCode == _TICKET_INQUIRY_)
                        {
                                pobTran->inTransactionCode = _TICKET_IPASS_INQUIRY_;
                                pobTran->srTRec.inCode = _TICKET_IPASS_INQUIRY_;
				pobTran->inRunTRTID = _TRT_TICKET_IPASS_INQUIRY_;
                                pobTran->srTRec.srIPASSRec.uszQueryBit = VS_TRUE;
                        }  
                        else if (pobTran->inTransactionCode == _TICKET_TOP_UP_)
                        {
                                pobTran->inTransactionCode = _TICKET_IPASS_TOP_UP_;
                                pobTran->srTRec.inCode = _TICKET_IPASS_TOP_UP_;
				pobTran->inRunTRTID = _TRT_TICKET_IPASS_TOP_UP_;
                        } 
                        else if (pobTran->inTransactionCode == _TICKET_VOID_TOP_UP_)
                        {
                                pobTran->inTransactionCode = _TICKET_IPASS_VOID_TOP_UP_;
                                pobTran->srTRec.inCode = _TICKET_IPASS_VOID_TOP_UP_;
				pobTran->inRunTRTID = _TRT_TICKET_IPASS_VOID_TOP_UP_;
                        }
                        else
                        {
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "IPASS 無此交易別");
					inLogPrintf(AT, szDebugMsg);
				}
                                return (VS_ERROR);   
                        }
                }
                else
                {
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "IPASS LOGON失敗");
				inLogPrintf(AT, szDebugMsg);
			}
			
			return (VS_ERROR);   
                }
	}
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
		if (inLoadTDTRec(pobTran->srTRec.inTDTIndex) < 0)
			return (VS_ERROR);
		
		/* 切換SAM卡 */
                inNCCC_tSAM_SelectAID_ECC_Flow();
        
                /* MFT index 紀錄 HDT index */    
		memset(szHostIndex, 0x00, sizeof(szHostIndex));
		inGetTicket_HostIndex(szHostIndex);
		pobTran->srBRec.inHDTIndex = atoi(szHostIndex);
                if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) < 0)
                        return (VS_ERROR);
		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) < 0)
                        return (VS_ERROR);
                
                if (inLoadECCDTRec(0) < 0)
			return (VS_ERROR);        
                
		memset(szLogOnOK, 0x00, sizeof(szLogOnOK));
		inGetTicket_LogOnOK(szLogOnOK);
                if (memcmp(szLogOnOK, "Y", strlen("Y")) == 0)
                {
                        if (pobTran->inTransactionCode == _TICKET_DEDUCT_)
                        {
                                pobTran->inTransactionCode = _TICKET_EASYCARD_DEDUCT_;
                                pobTran->srTRec.inCode = _TICKET_EASYCARD_DEDUCT_;
				pobTran->inRunTRTID = _TRT_TICKET_ECC_DEDUCT_;
                        }  
                        else if (pobTran->inTransactionCode == _TICKET_REFUND_)
                        {
                                pobTran->inTransactionCode = _TICKET_EASYCARD_REFUND_;
                                pobTran->srTRec.inCode = _TICKET_EASYCARD_REFUND_;
				pobTran->inRunTRTID = _TRT_TICKET_ECC_REFUND_;
                        } 
                        else if (pobTran->inTransactionCode == _TICKET_INQUIRY_)
                        {
                                pobTran->inTransactionCode = _TICKET_EASYCARD_INQUIRY_;
                                pobTran->srTRec.inCode = _TICKET_EASYCARD_INQUIRY_;
				pobTran->inRunTRTID = _TRT_TICKET_ECC_INQUIRY_;
                        }  
                        else if (pobTran->inTransactionCode == _TICKET_TOP_UP_)
                        {
                                pobTran->inTransactionCode = _TICKET_EASYCARD_TOP_UP_;
                                pobTran->srTRec.inCode = _TICKET_EASYCARD_TOP_UP_;
				pobTran->inRunTRTID = _TRT_TICKET_ECC_TOP_UP_;
                        } 
                        else if (pobTran->inTransactionCode == _TICKET_VOID_TOP_UP_)
                        {
                                pobTran->inTransactionCode = _TICKET_EASYCARD_VOID_TOP_UP_;
                                pobTran->srTRec.inCode = _TICKET_EASYCARD_VOID_TOP_UP_;
				pobTran->inRunTRTID = _TRT_TICKET_ECC_VOID_TOP_UP_;
                        }
                        else
                        {
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "EASYCARD 無此交易別");
					inLogPrintf(AT, szDebugMsg);
				}
                                return (VS_ERROR);   
                        }
			
			/* (需求單 - 107276)自助交易標準做法  	悠遊卡交易時，悠遊卡POS ID帶入POS櫃號數值 by Russell 2019/1/21 下午 2:02 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetStoreIDEnable(szFuncEnable);
			if (pobTran->uszECRBit == VS_TRUE		&&
			    !memcmp(szFuncEnable, "Y", strlen("Y")))
			{
				/* 重組ICERINI 用ECR帶的store id */
				inECC_PACK_ICERINI(pobTran);
			}
                }
                else
                {
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "EASYCARD LOGON失敗");
				inLogPrintf(AT, szDebugMsg);
			}
			
			return (VS_ERROR);   
                }
	}
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
	{
		if (inLoadTDTRec(pobTran->srTRec.inTDTIndex) < 0)
			return (VS_ERROR);
		
		/* 切換SAM卡 */
		inNCCC_tSAM_SelectAID_ICASH_Flow();
        
                /* MFT index 紀錄 HDT index */    
		memset(szHostIndex, 0x00, sizeof(szHostIndex));
		inGetTicket_HostIndex(szHostIndex);
		pobTran->srBRec.inHDTIndex = atoi(szHostIndex);
                if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) < 0)
                        return (VS_ERROR);
		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) < 0)
                        return (VS_ERROR);
                
                if (inLoadICASHDTRec(0) < 0)
			return (VS_ERROR);        
                
		memset(szLogOnOK, 0x00, sizeof(szLogOnOK));
		inGetTicket_LogOnOK(szLogOnOK);
                if (memcmp(szLogOnOK, "Y", strlen("Y")) == 0)
                {
                        if (pobTran->inTransactionCode == _TICKET_DEDUCT_)
                        {
                                pobTran->inTransactionCode = _TICKET_ICASH_DEDUCT_;
                                pobTran->srTRec.inCode = _TICKET_ICASH_DEDUCT_;
				pobTran->inRunTRTID = _TRT_TICKET_ICASH_DEDUCT_;
                        }  
                        else if (pobTran->inTransactionCode == _TICKET_REFUND_)
                        {
                                pobTran->inTransactionCode = _TICKET_ICASH_REFUND_;
                                pobTran->srTRec.inCode = _TICKET_ICASH_REFUND_;
				pobTran->inRunTRTID = _TRT_TICKET_ICASH_REFUND_;
                        } 
                        else if (pobTran->inTransactionCode == _TICKET_INQUIRY_)
                        {
                                pobTran->inTransactionCode = _TICKET_ICASH_INQUIRY_;
                                pobTran->srTRec.inCode = _TICKET_ICASH_INQUIRY_;
				pobTran->inRunTRTID = _TRT_TICKET_ICASH_INQUIRY_;
                        }  
                        else if (pobTran->inTransactionCode == _TICKET_TOP_UP_)
                        {
                                pobTran->inTransactionCode = _TICKET_ICASH_TOP_UP_;
                                pobTran->srTRec.inCode = _TICKET_ICASH_TOP_UP_;
				pobTran->inRunTRTID = _TRT_TICKET_ICASH_TOP_UP_;
                        } 
                        else if (pobTran->inTransactionCode == _TICKET_VOID_TOP_UP_)
                        {
                                pobTran->inTransactionCode = _TICKET_ICASH_VOID_TOP_UP_;
                                pobTran->srTRec.inCode = _TICKET_ICASH_VOID_TOP_UP_;
				pobTran->inRunTRTID = _TRT_TICKET_ICASH_VOID_TOP_UP_;
                        }
                        else
                        {
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "ICASH 無此交易別");
					inLogPrintf(AT, szDebugMsg);
				}
                                return (VS_ERROR);   
                        }
                }
                else
                {
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "ICASH LOGON失敗");
				inLogPrintf(AT, szDebugMsg);
			}
			
			return (VS_ERROR);   
                }
	}
	
	pobTran->srTRec.lnTxnAmount = pobTran->srBRec.lnTxnAmount;
	/* 批次號碼 */
	memset(szBatchNum, 0x00, sizeof(szBatchNum));
	inGetBatchNum(szBatchNum);
	pobTran->srBRec.lnBatchNum = atol(szBatchNum);
		
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Ticket_Decide_Trans_Type() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_Get_ParamValue
Date&Time       :2017/12/21 下午 1:52
Describe        :
*/
int inNCCC_Ticket_Get_ParamValue(TRANSACTION_OBJECT *pobTran)
{
	char		szInvNum[6 + 1] = {0};
	char		szTicketInvNum[6 + 1] = {0};
	char		szOutput[10 + 1] = {0};
	char		szFuncEnable[2 + 1] = {0};
	RTC_NEXSYS	srRTC;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_Ticket_Get_ParamValue() START !");
	}
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	memset(szInvNum, 0x00, sizeof(szInvNum));
	inGetInvoiceNum(szInvNum);
	
	memset(szTicketInvNum, 0x00, sizeof(szTicketInvNum));
	inGetTicket_InvNum(szTicketInvNum);
	
	pobTran->srTRec.lnInvNum = atol(szInvNum);
	pobTran->srTRec.lnMainInvNum = atol(szTicketInvNum);
	pobTran->srTRec.lnCountInvNum = pobTran->srTRec.lnMainInvNum;
	
	/* RRN */
	inNCCC_Ticket_Func_MakeRefNo(pobTran);
	
	/* Time */
	memset(&srRTC, 0x00, sizeof(srRTC));
	inFunc_GetSystemDateAndTime(&srRTC);
	
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
        {
                /* GMT Unix Time */
		memset(szOutput, 0x00, sizeof(szOutput));
                inFuncGetTimeToUnix(&srRTC, szOutput, 8, 0);
		pobTran->srTRec.srIPASSRec.lnUnixTime = atol(szOutput);
        }
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
        {
                memset(pobTran->srTRec.srICASHRec.szDate, 0x00, sizeof(pobTran->srTRec.srICASHRec.szDate));
                memset(pobTran->srTRec.srICASHRec.szTime, 0x00, sizeof(pobTran->srTRec.srICASHRec.szTime));
                sprintf(pobTran->srTRec.srICASHRec.szDate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
                sprintf(pobTran->srTRec.srICASHRec.szTime, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
        }
	
	if (inFunc_Sync_TRec_Date_Time(pobTran, &srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	/* 櫃號 */
        memset(pobTran->srTRec.szStoreID, 0x00, sizeof(pobTran->srTRec.szStoreID));
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetStoreIDEnable(szFuncEnable);
	
	if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
	{
		memcpy(pobTran->srTRec.szStoreID, pobTran->srBRec.szStoreID, 50); 
	}
                 
        /* 產品代碼 */
	memset(pobTran->srTRec.szProductCode, 0x00, sizeof(pobTran->srTRec.szProductCode));
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetProductCodeEnable(szFuncEnable);
	
	if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
	{
		memcpy(pobTran->srTRec.szProductCode, pobTran->srBRec.szProductCode, 42); 
	}
	
	
        /* 退貨碼預設為000000 */
        if (pobTran->srTRec.inCode == _TICKET_IPASS_REFUND_ || 
	    pobTran->srTRec.inCode == _TICKET_EASYCARD_REFUND_ ||
	    pobTran->srTRec.inCode == _TICKET_ICASH_REFUND_)
        {
                if (strlen(pobTran->srTRec.szTicketRefundCode) == 0)
                {
                        memset(pobTran->srTRec.szTicketRefundCode, 0x00, sizeof(pobTran->srTRec.szTicketRefundCode));        
                        memcpy(pobTran->srTRec.szTicketRefundCode, "000000", 6); 
                }    
        }   
        else    
        {
                memset(pobTran->srTRec.szTicketRefundCode, 0x00, sizeof(pobTran->srTRec.szTicketRefundCode));        
                memcpy(pobTran->srTRec.szTicketRefundCode, "000000", 6); 
        }
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Ticket_Get_ParamValue() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_Func_MakeRefNo
Date&Time       :2017/12/21 下午 3:50
Describe        :
*/
int inNCCC_Ticket_Func_MakeRefNo(TRANSACTION_OBJECT *pobTran)
{
	char	szBatchNum[6 + 1];
	char	szOrgInvoiceNum[6 + 1];
	char	szTerminalID[8 + 1];
	
	memset(pobTran->srTRec.szRefNo, 0x00, sizeof(pobTran->srTRec.szRefNo));
	strcpy(pobTran->srTRec.szRefNo, "9");
	/* Terminal ID */
	memset(szTerminalID, 0x00, sizeof(szTerminalID));
	inGetTerminalID(szTerminalID);
	memcpy(&pobTran->srTRec.szRefNo[1], &szTerminalID[3], 5);
	/* Batch Number */;
	memset(szBatchNum, 0x00, sizeof(szBatchNum));
	inGetBatchNum(szBatchNum);
	memcpy(&pobTran->srTRec.szRefNo[6], &szBatchNum[4], 2);
	/* Invoice Number */
	memset(szOrgInvoiceNum, 0x00, sizeof(szOrgInvoiceNum));
	inGetInvoiceNum(szOrgInvoiceNum);
	memcpy(&pobTran->srTRec.szRefNo[8], &szOrgInvoiceNum[3], 3);
	/* 補空白 */
	inFunc_PAD_ASCII(pobTran->srTRec.szRefNo, pobTran->srTRec.szRefNo, ' ', 12, _PADDING_RIGHT_);
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_Connect
Date&Time       :2017/12/27 下午 3:21
Describe        :票證需多段收送，所以連線、送電文、斷線分開來寫
*/
int inNCCC_Ticket_Connect(TRANSACTION_OBJECT *pobTran)
{
	char	szTemplate[20 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		return (VS_SUCCESS);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "----------------------------------------");
			inLogPrintf(AT, "inNCCC_Ticket_Connect() START !");
		}

		if (ginFindRunTime == VS_TRUE)
		{
			inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
		}
			
		if (pobTran->srTRec.uszOfflineBit == VS_FALSE)
		{
			pobTran->uszDialBackup = VS_FALSE;

			/* 票證沒撥接，自行防呆 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetCommMode(szTemplate);
			if (memcmp(szTemplate, _COMM_MODEM_MODE_, strlen(_COMM_MODEM_MODE_)) == 0)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("此交易不支援撥接", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_Wait(2000);
				return (VS_ERROR);
			}

			/* 處理 Online 交易流程 */
			if (inFLOW_RunFunction(pobTran, _COMM_START_) != VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "inNCCC_Ticket_Connect Failed");
					inLogPrintf(AT, szDebugMsg);
				}
                                
                                /* Mirror Message */
                                if (pobTran->uszECRBit == VS_TRUE)
                                {
                                        inECR_SendMirror(pobTran, _MIRROR_MSG_COMM_ERROR_);
                                }
                                
				pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;

				return (VS_ERROR);
			}
                        
                        /* Mirror Message */
                        if (pobTran->uszECRBit == VS_TRUE)
                        {
                                inECR_SendMirror(pobTran, _MIRROR_MSG_CONNECT_HOST_);
                        }
		}
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_Ticket_Connect() END !");
			inLogPrintf(AT, "----------------------------------------");
		}
		
		return (VS_SUCCESS);
	}
}

/*
Function        :inNCCC_Ticket_DisConnect
Date&Time       :2018/1/2 下午 3:35
Describe        :票證需多段收送，所以連線、送電文、斷線分開來寫
*/
int inNCCC_Ticket_DisConnect(TRANSACTION_OBJECT *pobTran)
{
	char	szDemoMode[2 + 1] = {0};
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		return (VS_SUCCESS);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "----------------------------------------");
			inLogPrintf(AT, "inNCCC_Ticket_DisConnect() START !");
		}
		
		if (ginFindRunTime == VS_TRUE)
		{
			inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
		}
		
		inFLOW_RunFunction(pobTran, _COMM_END_);
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_Ticket_DisConnect() END !");
			inLogPrintf(AT, "----------------------------------------");
		}

		return (VS_SUCCESS);
	}
}

/*
Function        :inNCCC_Ticket_Check_AckHost
Date&Time       :2017/12/27 下午 4:38
Describe        :
*/
int inNCCC_Ticket_Check_AckHost(TRANSACTION_OBJECT *pobTran)
{
	char	szDemoMode[2 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_Ticket_Check_AckHost() START !");
	}
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_Ticket_Check_AckHost() END !");
			inLogPrintf(AT, "----------------------------------------");
		}
		
		/* 一般卡詢卡時，有票值回復要同步Invnum的地方 */
		inNCCC_Ticket_Func_UpdatetTicketInvNum(pobTran);
		
		return (VS_SUCCESS);
	}
	else
	{
		if (pobTran->srTRec.srIPASSRec.uszCB_CardBit == VS_TRUE)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Combo Card要回傳Host");
			}
			
			if (inFLOW_RunFunction(pobTran, _TICKET_CONNECT_) != VS_SUCCESS)
				return (VS_ERROR);   

			if (inFLOW_RunFunction(pobTran, _TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_) != VS_SUCCESS)
				return (VS_ERROR);

			if (inFLOW_RunFunction(pobTran, _TICKET_FUNCTION_BUILD_AND_SEND_PACKET_) != VS_SUCCESS)
			{
				inFLOW_RunFunction(pobTran, _TICKET_DISCONNECT_);
				return (VS_ERROR);    
			}

			/* 拍完不響嗶聲 */
			pobTran->uszESVCTxnNotEndBit = VS_TRUE;

			if (inFLOW_RunFunction(pobTran, _TICKET_IPASS_TAPCARD_SECOND_) != VS_SUCCESS)
			{
				inFLOW_RunFunction(pobTran, _TICKET_DISCONNECT_);
				return (VS_ERROR);    
			}

			/* 回覆原來設定 */
			pobTran->uszESVCTxnNotEndBit = VS_FALSE;

			if (inFLOW_RunFunction(pobTran, _TICKET_DISCONNECT_) != VS_SUCCESS)
				return (VS_ERROR);  
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "非Combo Card");
			}
		}
		
		/* 一般卡詢卡時，有票值回復要同步Invnum的地方 */
		inNCCC_Ticket_Func_UpdatetTicketInvNum(pobTran);

		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_Ticket_Check_AckHost() END !");
			inLogPrintf(AT, "----------------------------------------");
		}

		return (VS_SUCCESS);
	}
}

/*
Function        :inNCCC_Ticket_Func_UpdateTermInvNum
Date&Time       :2017/12/27 下午 4:46
Describe        :更新簽單InvNum和電文InvNum
*/
int inNCCC_Ticket_Func_UpdateTermInvNum(TRANSACTION_OBJECT *pobTran)
{
	int		inESVC_HostIndex = -1;
        char		szInvNum[6 + 1];
	char		szTicketInvNum[6 + 1];
	char		szHostBatchNumLimit[6 + 1];
        long		lnInvNum = 0;
	long		lnTicketInvNum = 0;
	long		lnHostBatchNumLimit = 0;
	unsigned char	uszSettleBit = VS_FALSE;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_Ticket_Func_UpdateInvNum() START !");
	}
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_ESVC_, &inESVC_HostIndex);

        if (inLoadHDPTRec(inESVC_HostIndex) == VS_ERROR)
        {
		/* 鎖機 */
		inFunc_EDCLock(AT);
		
                return (VS_ERROR);
        }

	/* InvNum增加 */
	memset(szInvNum, 0x00, sizeof(szInvNum));
	inGetInvoiceNum(szInvNum);
	lnInvNum = atol(szInvNum);
	lnInvNum ++;
	
	memset(szInvNum, 0x00, sizeof(szInvNum));
	sprintf(szInvNum, "%06ld", lnInvNum);
	inSetInvoiceNum(szInvNum);
	
	memset(szHostBatchNumLimit, 0x00, sizeof(szHostBatchNumLimit));
	inGetBatchNumLimit(szHostBatchNumLimit);
	lnHostBatchNumLimit = atol(szHostBatchNumLimit);
	
	/* 超過交易筆數，表示要結帳 */
	if (lnInvNum >= lnHostBatchNumLimit)
	{
		uszSettleBit = VS_TRUE;
	}
	
	if (uszSettleBit == VS_TRUE)
	{
		inSetMustSettleBit("Y");
	}
	
	if (inLoadTDTRec(pobTran->srTRec.inTDTIndex) == VS_ERROR)
        {
		/* 鎖機 */
		inFunc_EDCLock(AT);
		
                return (VS_ERROR);
        }
	
	memset(szTicketInvNum, 0x00, sizeof(szTicketInvNum));
	inGetTicket_InvNum(szTicketInvNum);
	lnTicketInvNum = atol(szTicketInvNum);
	/* 檢核Inv是否合法(不能小於原本的值) */
	if (pobTran->srTRec.lnCountInvNum >= lnTicketInvNum)
        {
		/* 悠遊卡因為API一次吐出做完的電文Inv，故採用同步的方式 */
		if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
		{
			memset(szTicketInvNum, 0x00, sizeof(szTicketInvNum));
			sprintf(szTicketInvNum, "%06ld", pobTran->srTRec.lnCountInvNum);
			inSetTicket_InvNum(szTicketInvNum);
		}
		else
		{
			pobTran->srTRec.lnCountInvNum++;
			memset(szTicketInvNum, 0x00, sizeof(szTicketInvNum));
			sprintf(szTicketInvNum, "%06ld", pobTran->srTRec.lnCountInvNum);
			inSetTicket_InvNum(szTicketInvNum);
		}
	}
	
	/* int limit 32767 */	
	if (pobTran->srTRec.lnCountInvNum > 32000)
	{
	        inSetTicket_InvNum("000001");
	}
	
	if (inSaveHDPTRec(inESVC_HostIndex) != VS_SUCCESS)
    	{
		return (VS_ERROR);
	}
	
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
//	        /* 悠遊卡特有 */
//	        inLoadCMASTRec(0);
//	        if (lnGetCMAS_STAN() > 999990)
//	        {        
//	                vdSetCMAS_STAN(1);
//                        inSaveCMASTRec(0);
//                }
	}        

	pobTran->uszTicketADVOnBit = VS_FALSE;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Ticket_Func_UpdateInvNum() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_Func_UpdatetTicketInvNum
Date&Time       :2017/12/27 下午 4:46
Describe        :只更新電文InvNum
*/
int inNCCC_Ticket_Func_UpdatetTicketInvNum(TRANSACTION_OBJECT *pobTran)
{
	int	inESVC_HostIndex = -1;
	char	szTicketInvNum[6 + 1];
	long	lnTicketInvNum = 0;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_Ticket_Func_UpdatetTicketInvNum() START !");
	}
	
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_ESVC_, &inESVC_HostIndex);

        if (inLoadHDPTRec(inESVC_HostIndex) == VS_ERROR)
        {
		/* 鎖機 */
		inFunc_EDCLock(AT);
		
                return (VS_ERROR);
        }
	
	memset(szTicketInvNum, 0x00, sizeof(szTicketInvNum));
	inGetTicket_InvNum(szTicketInvNum);
	lnTicketInvNum = atol(szTicketInvNum);
	/* 檢核Inv是否合法(不能小於原本的值) */
	if (pobTran->srTRec.lnCountInvNum >= lnTicketInvNum)
        {
		/* 悠遊卡因為API一次吐出做完的電文Inv，故採用同步的方式 */
		if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
		{
			memset(szTicketInvNum, 0x00, sizeof(szTicketInvNum));
			sprintf(szTicketInvNum, "%06ld", pobTran->srTRec.lnCountInvNum);
			inSetTicket_InvNum(szTicketInvNum);
		}
		else
		{
			pobTran->srTRec.lnCountInvNum++;
			memset(szTicketInvNum, 0x00, sizeof(szTicketInvNum));
			sprintf(szTicketInvNum, "%06ld", pobTran->srTRec.lnCountInvNum);
			inSetTicket_InvNum(szTicketInvNum);
		}
	}
	
	/* int limit 32767 */	
	if (pobTran->srTRec.lnCountInvNum > 32000)
	{
	        inSetTicket_InvNum("000001");
	}
	
	if (inSaveHDPTRec(inESVC_HostIndex) != VS_SUCCESS)
    	{
		return (VS_ERROR);
	}
	
	pobTran->uszTicketADVOnBit = VS_FALSE;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Ticket_Func_UpdatetTicketInvNum() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_Get_Card_Flow
Date&Time       :2018/1/2 上午 11:49
Describe        :快速詢卡流程
*/
int inNCCC_Ticket_Get_Card_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1];
	
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Ticket_Get_Card_Flow START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_Ticket_Get_Card_Flow() START !");
	}
	
	inRetVal = inNCCC_Ticket_Get_Card(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inNCCC_Ticket_Get_Card_Flow Failed");
			inLogPrintf(AT, szDebugMsg);
		}
		return (inRetVal);
	}
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_ResetAll();
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Ticket_Get_Card_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_Get_Card
Date&Time       :2018/1/2 上午 11:49
Describe        :快速詢卡
*/
int inNCCC_Ticket_Get_Card(TRANSACTION_OBJECT *pobTran)
{
	int	i = 0;
	int	inRetVal = VS_ERROR;
	int	inUnitDigit;
        char    szList[10];
	char	szFuncEnable[2 + 1];
	char	szDebugMsg[100 + 1];
	char	szDemoMode[2 + 1] = {0};
        char	szCustomerIndicator[3 + 1] = {0};
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetIntegrate_Device(szFuncEnable);

        if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0)
        {
                /* ESVCPriority [IECH] = IPASS => ECC => ICASH
                        詢卡順位，不開或沒Sign On就忽略       
                */
		if (pobTran->uszCardInquirysSecondBit == VS_TRUE)
		{
			/* 走兩段式ECR流程，不用重新偵測卡 */
		}
		else
		{
                        /* Mirror Message */
                        if (pobTran->uszECRBit == VS_TRUE)
                        {
                                inECR_SendMirror(pobTran, _MIRROR_MSG_ESVC_GET_CARD_);
                        }
                    
			memset(pobTran->srTRec.szUID, 0x00, sizeof(pobTran->srTRec.szUID));
			inRetVal = inNCCC_Ticket_Fast_Tap_Wait(pobTran, pobTran->srTRec.szUID);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			else
			{
				memset(pobTran->srTRec.szAPUID, 0x00, sizeof(pobTran->srTRec.szAPUID));
				inFunc_ASCII_to_BCD((unsigned char*)pobTran->srTRec.szAPUID, pobTran->srTRec.szUID, (strlen(pobTran->srTRec.szUID) / 2));
				/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
				/* 票證只要過卡就關*/
				if (ginFallback == VS_TRUE)
				{
					inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
				}
			}
		}
                        
                memset(szList, 0x00, sizeof(szList));
		inGetESVC_Priority(szList);

		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "----------------------------------------");

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inNCCC_Ticket_Get_Card(%s) START !", szList);
			inLogPrintf(AT, szDebugMsg);
		}
		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "List: %s", szList);
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
		}
                
		/* 教育訓練模式 */
		memset(szDemoMode, 0x00, sizeof(szDemoMode));
		inGetDemoMode(szDemoMode);
	
		if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
		{
			if (pobTran->inTransactionCode == _TICKET_INQUIRY_)
				inUnitDigit = gulDemoTicketPoint % 10;
			else
				inUnitDigit = pobTran->srBRec.lnTxnAmount % 10;
			
			inRetVal = inNCCC_Ticket_Demo_Type(pobTran, inUnitDigit);
			
			switch (inRetVal)
			{
				case _TICKET_TYPE_IPASS_:
					pobTran->srTRec.inTicketType = _TICKET_TYPE_IPASS_;
					pobTran->srTRec.inTDTIndex = _TDT_INDEX_00_IPASS_;
					pobTran->srTRec.uszESVCTransBit = VS_TRUE;
					
					if (pobTran->srBRec.lnTxnAmount > 1000)
					{
						pobTran->inErrorMsg = _ERROR_CODE_V3_TICKET_AMOUNT_TOO_MUCH_IN_ONE_TRANSACTION_;
                                                
                                                /* 客製化098，0010以上的回應碼都與標準版少1 */
                                                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                                                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)) 
                                                {
                                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0011");
                                                }
                                                else
                                                {
                                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0012");
                                                }   
						return (VS_ERROR);
					}
					
					break;
				case _TICKET_TYPE_ECC_:
					pobTran->srTRec.inTicketType = _TICKET_TYPE_ECC_;
					pobTran->srTRec.inTDTIndex = _TDT_INDEX_01_ECC_;
					pobTran->srTRec.uszESVCTransBit = VS_TRUE;
					break;
				case _TICKET_TYPE_ICASH_:
					pobTran->srTRec.inTicketType = _TICKET_TYPE_ICASH_;
					pobTran->srTRec.inTDTIndex = _TDT_INDEX_02_ICASH_;
					pobTran->srTRec.uszESVCTransBit = VS_TRUE;
					
					if (pobTran->srBRec.lnTxnAmount > 10000)
					{
						pobTran->inErrorMsg = _ERROR_CODE_V3_TICKET_AMOUNT_TOO_MUCH_IN_ONE_TRANSACTION_;
                                                
						if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                                                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)) 
                                                {
                                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0011");
                                                }
                                                else
                                                {
                                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0012");
                                                }   
						return (VS_ERROR);
					}
					
					break;
				default:
					return (VS_ERROR);
			}
			
			return (VS_SUCCESS);
		}
		
                for (i = 0; i < 4; i++)
                {
                        if (szList[i] == 'I')
                        {
				inLoadTDTRec(_TDT_INDEX_00_IPASS_);
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetTicket_LogOnOK(szFuncEnable);
                                if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
                                {
                                        inNCCC_tSAM_SelectAID_IPASS_Flow();
					inRetVal = inIPASS_Fast_Tap(pobTran->srTRec.szUID, strlen(pobTran->srTRec.szUID));
                                        
					if (inRetVal == VS_SUCCESS)
					{
						pobTran->srTRec.inTicketType = _TICKET_TYPE_IPASS_;
						pobTran->srTRec.inTDTIndex = _TDT_INDEX_00_IPASS_;
						pobTran->srTRec.uszESVCTransBit = VS_TRUE;
						
						return (VS_SUCCESS);
					}
                                }       
                        } 
                        else if (szList[i] == 'E')
                        {
                                inLoadTDTRec(_TDT_INDEX_01_ECC_);
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetTicket_LogOnOK(szFuncEnable);
                                if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
                                {
					inNCCC_tSAM_SelectAID_ECC_Flow();
					inRetVal = inECC_POLL_Flow(pobTran);
                                        
                                        if (inRetVal == VS_SUCCESS)
					{
						pobTran->srTRec.inTicketType = _TICKET_TYPE_ECC_;
						pobTran->srTRec.inTDTIndex = _TDT_INDEX_01_ECC_;
						pobTran->srTRec.uszESVCTransBit = VS_TRUE;
						
                                                return (VS_SUCCESS);
					}
                                }
                        }         
                        else if (szList[i] == 'C')
                        {
				inLoadTDTRec(_TDT_INDEX_02_ICASH_);
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetTicket_LogOnOK(szFuncEnable);
                                if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
                                {
					inNCCC_tSAM_SelectAID_ICASH_Flow();
					inRetVal = inICASH_Fast_Tap(pobTran);
                                        
                                        if (inRetVal == VS_SUCCESS)
					{
						pobTran->srTRec.inTicketType = _TICKET_TYPE_ICASH_;
						pobTran->srTRec.inTDTIndex = _TDT_INDEX_02_ICASH_;
						pobTran->srTRec.uszESVCTransBit = VS_TRUE;
						
                                                return (VS_SUCCESS);
					}
                                }
                        }
                }
        } 
        else
        {
		pobTran->srTRec.inTicketType = _TICKET_TYPE_IPASS_;
		pobTran->srTRec.inTDTIndex = 0;
		pobTran->srTRec.uszESVCTransBit = VS_TRUE;
						
                return (VS_SUCCESS);
        }
        
        /* Mirror Message */
        if (pobTran->uszECRBit == VS_TRUE)
	{
                inECR_SendMirror(pobTran, _MIRROR_MSG_ESVC_GET_CARD_RETRY_);
        }
        
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inCTLS_Set_LED(_CTLS_LIGHT_RED_);
	inDISP_ChineseFont("詢卡失敗", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
	inDISP_BEEP(3, 500);
	
	pobTran->srTRec.inTicketType = _TICKET_TYPE_NONE_;
	return (VS_ERROR);
}

/*
Function        :inNCCC_Ticket_Init_Logon_Flow
Date&Time       :2018/1/3 下午 6:06
Describe        :票證初始化迴圈，依序初始化
 *		參考:inMF_Init_Logon
*/
int inNCCC_Ticket_Init_Logon_Flow(TRANSACTION_OBJECT *pobTran)
{
#ifdef	_LOAD_KEY_AP_
	return (VS_SUCCESS);
#else
	int		i = 0;
	int		inFailedCnt = 0;
	int		inRetVal = VS_ERROR;
	int		inChoice = _DisTouch_No_Event_;
	int		inTouchSensorFunc = _Touch_CUP_LOGON_;
        char		szTMSOK[2] = {0};
	char		szNCCCFesMode[2 + 1] = {0};
	char		szFuncEnable[2 + 1] = {0};
	char		szHostName[20 + 1] = {0};
	char		szKey = 0x00;
	char		szDebugMsg[100 + 1] = {0};
	char		szDemoMode[2 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned char	uszIPASSBit = VS_FALSE, uszECCBit = VS_FALSE, uszICASHBit = VS_FALSE;
	unsigned char	uszReInitBit = VS_FALSE;
        
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		for (i = 0; i < _TDT_INDEX_MAX_; i++)
		{
			if (inLoadTDTRec(i) < 0)
				break;

			/* 開機認證，各票證有開就要跑 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetTicket_HostEnable(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				inSetTicket_LogOnOK("Y");
				
				if (i == _TDT_INDEX_00_IPASS_)
				{
					inLoadIPASSDTRec(0);
				}
				else if (i == _TDT_INDEX_01_ECC_)
				{
					inLoadECCDTRec(0);
					/* 一代設備編號 */
					inSetTicket_Device1("221234567890");
					/* 二代設備編號 */
					inSetTicket_Device2("09101234567890");
					/* 悠遊卡批次號碼 */
					inSetTicket_Batch("18010101");
					/* DEMO好像不用更新 */
					inSetTicket_NeedNewBatch("N");
				}
				else if (i == _TDT_INDEX_02_ICASH_)
				{
					inLoadICASHDTRec(0);
				}				
			}
			else
			{
				inSetTicket_LogOnOK("N");
			}
			
			inSaveTDTRec(i);
		}
		return (VS_SUCCESS);
	}
	else
	{
		/* 檢查是否做過【參數下載】 */
		memset(szTMSOK, 0x00, sizeof(szTMSOK));
		inGetTMSOK(szTMSOK);
		if (szTMSOK[0] != 'Y')
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Ticket_Init_Logon_Flow TMS Not OK Failed");
			return (VS_SUCCESS);
		}

		memset(szNCCCFesMode, 0x00, sizeof(szNCCCFesMode));
		inGetNCCCFESMode(szNCCCFesMode);
		if (memcmp(szNCCCFesMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0)
		{
			/* MFES沒Tsam，要把Slot open */
			inNCCC_Ticket_MFES_PowerOn_SAM_Slot();
		}

		/* 初始化 */
		inLoadTDTRec(_TDT_INDEX_00_IPASS_);
		inSetTicket_LogOnOK("N");
		inSaveTDTRec(_TDT_INDEX_00_IPASS_);

		inLoadTDTRec(_TDT_INDEX_01_ECC_);
		inSetTicket_LogOnOK("N");
		inSaveTDTRec(_TDT_INDEX_01_ECC_);

		inLoadTDTRec(_TDT_INDEX_02_ICASH_);
		inSetTicket_LogOnOK("N");
		inSaveTDTRec(_TDT_INDEX_02_ICASH_);
		
		/* 悠遊卡硬體初始化只執行一次 */
		inLoadTDTRec(_TDT_INDEX_01_ECC_);
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetTicket_HostEnable(szFuncEnable);
		if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
		{
			inDISP_ClearAll();
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inICERAPI_InitialReader Start");
			}
#ifdef _NEW_ECC_NEXSYS_
			int	inFd = 0;
			char	szPath[100 + 1] = {0};
			memset(szPath, 0x00, sizeof(szPath));
			sprintf(szPath, "%s%s", _ECC_FOLDER_PATH_, _ECC_DEBUG_SWITCH_FILE_NAME_);
			if (inFile_Linux_Open(&inFd, szPath) == VS_SUCCESS)
			{
				ECC_Debug_On_Lib();
			}
			else
			{
				ECC_Debug_Off_Lib();
			}
#endif			
			inICERAPI_InitialReader();
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inICERAPI_InitialReader END");
			}
		}

		/* 電子票證 */
		inDISP_ClearAll();
		inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);
		inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);

		/* 迴圈開始 */
		while(1)
		{
			/* 原先一次加重試三次 */
			if (inFailedCnt >= 4)
			{
				inFlushKBDBuffer();
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

				inDISP_PutGraphic(_CHECK_TICKET_LOGON_, 0, _COORDINATE_Y_LINE_8_4_);
				inDISP_BEEP(1, 0);

				/* 設定Timeout */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_);
				}
				else
				{
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
				}

				/* 電子票證Sign on */
				/* 執行下載請按確認，不執行請按清除 */
				while (1)
				{
					inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
					szKey = uszKBD_Key();

					/* TimeOut */
					if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
					{
						szKey = _KEY_TIMEOUT_;
					}

					if (szKey == _KEY_ENTER_			|| 
					    inChoice == _CUPLogOn_Touch_KEY_1_)
					{
						inRetVal = VS_SUCCESS;
						break;
					}
					else if (szKey == _KEY_CANCEL_			|| 
						 inChoice == _CUPLogOn_Touch_KEY_2_)
					{
						inRetVal = VS_USER_CANCEL;
						break;
					}
					else if (szKey == _KEY_TIMEOUT_)
					{
						inRetVal = VS_TIMEOUT;
						break;
					}

				}
				/* 清空Touch資料 */
				inDisTouch_Flush_TouchFile();

				/* 確認是否重試 */
				if (inRetVal == VS_USER_CANCEL	||
				    inRetVal == VS_TIMEOUT)
				{
					break;
				}
				else if (inRetVal == VS_SUCCESS)
				{
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					/* 繼續LogOn */
				}
			}    

			for (i = 0; i < _TDT_INDEX_MAX_; i++)
			{
				if (inLoadTDTRec(i) < 0)
					break;
				
				/* 開機認證，各票證有開就要跑 */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetTicket_HostEnable(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
				{
					memset(szHostName, 0x00, sizeof(szHostName));
					inGetTicket_HostName(szHostName);

					if (memcmp(szHostName, _HOST_NAME_IPASS_, strlen(_HOST_NAME_IPASS_)) == 0)
					{
						if (uszIPASSBit == VS_FALSE)
						{
							inRetVal = inNCCC_tSAM_PowerOn_IPASS_Flow();
							if (inRetVal != VS_SUCCESS)
							{
								vdUtility_SYSFIN_LogMessage(AT, "IPASS tsam poweron failed");
								uszIPASSBit = VS_FALSE;
								continue;
							}

							inRetVal = inNCCC_tSAM_SelectAID_IPASS_Flow();
							if (inRetVal != VS_SUCCESS)
							{
								vdUtility_SYSFIN_LogMessage(AT, "IPASS tsam selectAID failed");
								uszIPASSBit = VS_FALSE;
								continue;
							}

							inRetVal = inIPASS_Init_Flow(pobTran, uszReInitBit);

							if (inRetVal != VS_SUCCESS)
							{
								vdUtility_SYSFIN_LogMessage(AT, "IPASS init failed");
								uszIPASSBit = VS_FALSE;
								inSetTicket_LogOnOK("N");
								inSaveTDTRec(_TDT_INDEX_00_IPASS_);
							}
							else
							{
								uszIPASSBit = VS_TRUE;
								inSetTicket_LogOnOK("Y");
								inSaveTDTRec(_TDT_INDEX_00_IPASS_);
							}
						}
					}
					else if (memcmp(szHostName, _HOST_NAME_ECC_, strlen(_HOST_NAME_ECC_)) == 0)
					{
						if (uszECCBit == VS_FALSE)
						{
							inRetVal = inNCCC_tSAM_PowerOn_ECC_Flow();
							if (inRetVal != VS_SUCCESS)
							{
								vdUtility_SYSFIN_LogMessage(AT, "ECC tsam poweron failed");
								uszECCBit = VS_FALSE;
								continue;
							}

							inRetVal = inNCCC_tSAM_SelectAID_ECC_Flow();
							if (inRetVal != VS_SUCCESS)
							{
								vdUtility_SYSFIN_LogMessage(AT, "ECC tsam selectAID failed");
								uszECCBit = VS_FALSE;
								continue;
							}

							pobTran->srTRec.inTicketType = _TICKET_TYPE_ECC_;
							inRetVal = inECC_Init_Flow(pobTran);
							
							if (inRetVal != VS_SUCCESS)
							{
								vdUtility_SYSFIN_LogMessage(AT, "ECC init failed");
								uszECCBit = VS_FALSE;
								inSetTicket_LogOnOK("N");
								inSaveTDTRec(_TDT_INDEX_01_ECC_);
							}
							else
							{
								uszECCBit = VS_TRUE;
								inSetTicket_LogOnOK("Y");
								inSaveTDTRec(_TDT_INDEX_01_ECC_);
							}
						}
					}
					else if (memcmp(szHostName, _HOST_NAME_ICASH_, strlen(_HOST_NAME_ICASH_)) == 0)
					{
						if (uszICASHBit == VS_FALSE)
						{
							inRetVal = inNCCC_tSAM_PowerOn_ICASH_Flow();
							if (inRetVal != VS_SUCCESS)
							{
								vdUtility_SYSFIN_LogMessage(AT, "ICASH tsam poweron failed");
								uszICASHBit = VS_FALSE;
								continue;
							}

							inRetVal = inNCCC_tSAM_SelectAID_ICASH_Flow();
							if (inRetVal != VS_SUCCESS)
							{
								vdUtility_SYSFIN_LogMessage(AT, "ICASH tsam selectAID failed");
								uszICASHBit = VS_FALSE;
								continue;
							}

							pobTran->srTRec.inTicketType = _TICKET_TYPE_ICASH_;
							inRetVal = inICASH_Init_Flow(pobTran);
							
							if (inRetVal != VS_SUCCESS)
							{
								vdUtility_SYSFIN_LogMessage(AT, "ICASH init failed");
								uszICASHBit = VS_FALSE;
								inSetTicket_LogOnOK("N");
								inSaveTDTRec(_TDT_INDEX_02_ICASH_);
							}
							else
							{
								uszICASHBit = VS_TRUE;
								inSetTicket_LogOnOK("Y");
								inSaveTDTRec(_TDT_INDEX_02_ICASH_);
							}
						}
					}
				}
				else
				{
					/* host沒開視為已認證過 */
					memset(szHostName, 0x00, sizeof(szHostName));
					inGetTicket_HostName(szHostName);

					if (memcmp(szHostName, _HOST_NAME_IPASS_, strlen(_HOST_NAME_IPASS_)) == 0)
					{
						uszIPASSBit = VS_TRUE;
					}
					else if (memcmp(szHostName, _HOST_NAME_ECC_, strlen(_HOST_NAME_ECC_)) == 0)
					{
						uszECCBit = VS_TRUE;
					}
					else if (memcmp(szHostName, _HOST_NAME_ICASH_, strlen(_HOST_NAME_ICASH_)) == 0)
					{
						uszICASHBit = VS_TRUE;
					}
				}                  
			}
			
			/* 所有都跑完後，若還有Host未認證過，失敗次數+1，*/
			if (uszIPASSBit == VS_FALSE || uszECCBit == VS_FALSE || uszICASHBit == VS_FALSE)
			{
				if (uszIPASSBit == VS_FALSE)
				{
					vdUtility_SYSFIN_LogMessage(AT, "IPASS Final init failed");
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "IPASS Failed");
						inLogPrintf(AT, szDebugMsg);
					}
				}

				if (uszECCBit == VS_FALSE)
				{
					vdUtility_SYSFIN_LogMessage(AT, "ECC Final init failed");
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "ECC Failed");
						inLogPrintf(AT, szDebugMsg);
					}
				}

				if (uszICASHBit == VS_FALSE)
				{
					vdUtility_SYSFIN_LogMessage(AT, "ICASH Final init failed");
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "ICASH Failed");
						inLogPrintf(AT, szDebugMsg);
					}
				}

				inFailedCnt++;
			}
			else
				break;
		}

		/* 清空錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		/* 切回NCCC */
		inNCCC_tSAM_SelectAID_NCCC_Flow();
		
		/* 自助客製化不顯示 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, 3)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, 3)		||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, 3))
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Customer 111 don't display ESVC Activation");
			}
		}
		else
		{
			/* 顯示認證訊息 */
			inNCCC_Ticket_Display_Inform_Activation_After_ESVC_SignOn(0, 0);
		}

		return (inRetVal);
	}
#endif
}

/*
Function        :inNCCC_Ticket_Logon_BeforeTxn_Flow
Date&Time       :2018/3/7 下午 4:57
Describe        :沒初始化的補初始化
 *
*/
int inNCCC_Ticket_Logon_BeforeTxn_Flow(TRANSACTION_OBJECT *pobTran)
{
	int		i = 0;
	int		inFailedCnt = 0;
	int		inRetVal = VS_ERROR;
	int		inChoice = _DisTouch_No_Event_;
	int		inTouchSensorFunc = _Touch_CUP_LOGON_;
        char		szTMSOK[2] = {0};
	char		szNCCCFesMode[2 + 1] = {0};
	char		szFuncEnable[2 + 1] = {0};
	char		szHostName[20 + 1] = {0};
	char		szKey = 0x00;
	char		szDebugMsg[100 + 1] = {0};
	char		szDemoMode[2 + 1] = {0};
	unsigned char	uszIPASSBit = VS_FALSE, uszECCBit = VS_FALSE, uszICASHBit = VS_FALSE;
	unsigned char	uszReInitBit = VS_FALSE;
        
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		return (VS_SUCCESS);
	}
	else
	{
		/* 檢查是否做過【參數下載】 */
		memset(szTMSOK, 0x00, sizeof(szTMSOK));
		inGetTMSOK(szTMSOK);
		if (szTMSOK[0] != 'Y')
			return (VS_SUCCESS);

		memset(szNCCCFesMode, 0x00, sizeof(szNCCCFesMode));
		inGetNCCCFESMode(szNCCCFesMode);
		if (memcmp(szNCCCFesMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0)
		{
			/* MFES沒Tsam，要把Slot open */
			inNCCC_Ticket_MFES_PowerOn_SAM_Slot();
		}

		/* 初始化 */
		inLoadTDTRec(_TDT_INDEX_00_IPASS_);
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetTicket_LogOnOK(szFuncEnable);
		if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
		{
			uszIPASSBit = VS_TRUE;
		}

		inLoadTDTRec(_TDT_INDEX_01_ECC_);
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetTicket_LogOnOK(szFuncEnable);
		if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
		{
			uszECCBit = VS_TRUE;
		}
		
		inLoadTDTRec(_TDT_INDEX_02_ICASH_);
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetTicket_LogOnOK(szFuncEnable);
		if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
		{
			uszICASHBit = VS_TRUE;
		}

		/* Sign on */
		inDISP_ClearAll();
		inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);
		inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);

		/* 迴圈開始 */
		while(1)
		{
			if (inFailedCnt >= 1)
			{
				inDISP_Clear_Line( _LINE_8_4_, _LINE_8_8_);

				inDISP_PutGraphic(_CHECK_TICKET_LOGON_, 0, _COORDINATE_Y_LINE_8_4_);
				inDISP_BEEP(1, 0);

				/* 設定Timeout */
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

				/* 電子票證Sign on */
				/* 執行下載請按確認，不執行請按清除 */
				while (1)
				{
					inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
					szKey = uszKBD_Key();

					/* TimeOut */
					if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
					{
						szKey = _KEY_TIMEOUT_;
					}

					if (szKey == _KEY_ENTER_			|| 
					    inChoice == _CUPLogOn_Touch_KEY_1_)
					{
						inRetVal = VS_SUCCESS;
						break;
					}
					else if (szKey == _KEY_CANCEL_			|| 
						 inChoice == _CUPLogOn_Touch_KEY_2_)
					{
						inRetVal = VS_USER_CANCEL;
						break;
					}
					else if (szKey == _KEY_TIMEOUT_)
					{
						inRetVal = VS_TIMEOUT;
						break;
					}

				}
				/* 清空Touch資料 */
				inDisTouch_Flush_TouchFile();

				/* 確認是否重試 */
				if (inRetVal == VS_USER_CANCEL	||
				    inRetVal == VS_TIMEOUT)
				{
					break;
				}
				else if (inRetVal == VS_SUCCESS)
				{
					/* 繼續LogOn */
				}
			}    

			for (i = 0; i < _TDT_INDEX_MAX_; i++)
			{
				if (inLoadTDTRec(i) < 0)
					break;

				/* 開機認證，各票證有開就要跑 */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetTicket_HostEnable(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
				{
					memset(szHostName, 0x00, sizeof(szHostName));
					inGetTicket_HostName(szHostName);

					if (memcmp(szHostName, _HOST_NAME_IPASS_, strlen(_HOST_NAME_IPASS_)) == 0)
					{
						if (uszIPASSBit == VS_FALSE)
						{
							inRetVal = inNCCC_tSAM_PowerOn_IPASS_Flow();
							if (inRetVal != VS_SUCCESS)
							{
								uszIPASSBit = VS_FALSE;
								continue;
							}

							inRetVal = inNCCC_tSAM_SelectAID_IPASS_Flow();
							if (inRetVal != VS_SUCCESS)
							{
								uszIPASSBit = VS_FALSE;
								continue;
							}

							inRetVal = inIPASS_Init_Flow(pobTran, uszReInitBit);

							if (inRetVal != VS_SUCCESS)
							{
								uszIPASSBit = VS_FALSE;
							}
							else
							{
								uszIPASSBit = VS_TRUE;
							}
						}
					}
					else if (memcmp(szHostName, _HOST_NAME_ECC_, strlen(_HOST_NAME_ECC_)) == 0)
					{
						if (uszECCBit == VS_FALSE)
						{
							inRetVal = inNCCC_tSAM_PowerOn_ECC_Flow();
							if (inRetVal != VS_SUCCESS)
							{
								uszECCBit = VS_FALSE;
								continue;
							}

							inRetVal = inNCCC_tSAM_SelectAID_ECC_Flow();
							if (inRetVal != VS_SUCCESS)
							{
								uszECCBit = VS_FALSE;
								continue;
							}

							inRetVal = inECC_Init_Flow(pobTran);

							if (inRetVal != VS_SUCCESS)
								uszECCBit = VS_FALSE;
							else
								uszECCBit = VS_TRUE;
						}
					}
					else if (memcmp(szHostName, _HOST_NAME_ICASH_, strlen(_HOST_NAME_ICASH_)) == 0)
					{
						if (uszICASHBit == VS_FALSE)
						{
							inRetVal = inNCCC_tSAM_PowerOn_ICASH_Flow();
							if (inRetVal != VS_SUCCESS)
							{
								uszICASHBit = VS_FALSE;
								continue;
							}

							inRetVal = inNCCC_tSAM_SelectAID_ICASH_Flow();
							if (inRetVal != VS_SUCCESS)
							{
								uszICASHBit = VS_FALSE;
								continue;
							}

							inRetVal = inICASH_Init_Flow(pobTran);

							if (inRetVal != VS_SUCCESS)
							{
								uszICASHBit = VS_FALSE;
							}
							else
							{
								uszICASHBit = VS_TRUE;
							}
						}
					}
				}
				else
				{
					/* host沒開視為已認證過 */
					memset(szHostName, 0x00, sizeof(szHostName));
					inGetTicket_HostName(szHostName);

					if (memcmp(szHostName, _HOST_NAME_IPASS_, strlen(_HOST_NAME_IPASS_)) == 0)
					{
						uszIPASSBit = VS_TRUE;
					}
					else if (memcmp(szHostName, _HOST_NAME_ECC_, strlen(_HOST_NAME_ECC_)) == 0)
					{
						uszECCBit = VS_TRUE;
					}
					else if (memcmp(szHostName, _HOST_NAME_ICASH_, strlen(_HOST_NAME_ICASH_)) == 0)
					{
						uszICASHBit = VS_TRUE;
					}
				}                   
			}

			/* 所有都跑完後，若還有Host未認證過，失敗次數+1，*/
			if (uszIPASSBit == VS_FALSE || uszECCBit == VS_FALSE || uszICASHBit == VS_FALSE)
			{
				if (uszIPASSBit == VS_FALSE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "IPASS Failed");
						inLogPrintf(AT, szDebugMsg);
					}
				}

				if (uszECCBit == VS_FALSE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "ECC Failed");
						inLogPrintf(AT, szDebugMsg);
					}
				}

				if (uszICASHBit == VS_FALSE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "ICASH Failed");
						inLogPrintf(AT, szDebugMsg);
					}
				}

				inFailedCnt++;
			}
			else
				break;          
		}

		/* 清空錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		inNCCC_tSAM_SelectAID_NCCC_Flow();

		return (inRetVal);
	}
}

/*
Function        :inNCCC_Ticket_MFES_PowerOn_SAM_Slot
Date&Time       :2018/1/4 上午 9:26
Describe        :MFES沒用到TSAM，要自己Power On
*/
int inNCCC_Ticket_MFES_PowerOn_SAM_Slot()
{
	int		i = 0;
	int		inRetVal = 0;
	char		szTemplate[20 + 1];
	unsigned char	uszSlot = 0;
	
	for (i = 0; i < _TDT_INDEX_MAX_; i++)
	{
		inLoadTDTRec(i);
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTicket_HostEnable(szTemplate);
		if (memcmp(szTemplate, "Y", strlen("Y")) != 0)
		{
			continue;
		}
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTicket_SAM_Slot(szTemplate);
		if (memcmp(szTemplate, "01", strlen("01")) == 0)
		{
			uszSlot = _SAM_SLOT_1_;
		}
		else if (memcmp(szTemplate, "02", strlen("02")) == 0)
		{
			uszSlot = _SAM_SLOT_2_;
		}
		else if (memcmp(szTemplate, "03", strlen("03")) == 0)
		{
			uszSlot = _SAM_SLOT_3_;
		}
		else if (memcmp(szTemplate, "04", strlen("04")) == 0)
		{
			uszSlot = _SAM_SLOT_4_;
		}

		inRetVal = inNCCC_tSAM_Slot_PowerOn(uszSlot);
		/* Power On 失敗 */
		if (inRetVal != VS_SUCCESS)
		{
			/* 請檢查SAM卡 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHECK_SAM_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = 1;
			sprintf(srDispMsgObj.szErrMsg1, "SAM卡請放SAM%d", atoi(szTemplate));
			srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);

			return (VS_ERROR);
		}
		
		/* 純tSAM Power On完直接抓SN(先select AID反而抓不到)，之後要再PowerOn一次並Select AID 
			MSAM Power On完要Select AID之後才能抓SN */

		/* 開機才抓SAM卡SN */
		/* 紀錄有放TSAM的位置 */
		if (inNCCC_tSAM_Check_SAM_In_Slot(_SAM_SLOT_1_) == VS_SUCCESS)
		{
			guszSlot1InBit = VS_TRUE;
		}
		if (inNCCC_tSAM_Check_SAM_In_Slot(_SAM_SLOT_2_) == VS_SUCCESS)
		{
			guszSlot2InBit = VS_TRUE;
		}
		if (inNCCC_tSAM_Check_SAM_In_Slot(_SAM_SLOT_3_) == VS_SUCCESS)
		{
			guszSlot3InBit = VS_TRUE;
		}
		if (inNCCC_tSAM_Check_SAM_In_Slot(_SAM_SLOT_4_) == VS_SUCCESS)
		{
			guszSlot4InBit = VS_TRUE;
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_Display_Transaction_Result
Date&Time       :2018/1/10 下午 2:04
Describe        :
*/
int inNCCC_Ticket_Display_Transaction_Result(TRANSACTION_OBJECT *pobTran)
{
	char		szDispMsg[50 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	char		szFesMode[2 + 1] = {0};
	unsigned char	uszEnterDisplayBit = VS_TRUE;		/* 顯示確認鍵 */
	
	memset(szFesMode, 0x00, sizeof(szFesMode));
	inGetNCCCFESMode(szFesMode);
	
	/* For MPAS 電票ECR 不顯示確認鍵  */
	if (memcmp(szFesMode, _NCCC_04_MPAS_MODE_, 2) == 0	&&
	    pobTran->uszECRBit == VS_TRUE)
	{
		uszEnterDisplayBit = VS_FALSE;
	}
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
        
        /* 客製化098，要顯示清除 */
        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)) 
        {
                uszEnterDisplayBit = VS_TRUE;
        }
        
	/* 此UI更新只更動V3P */
	if (ginMachineType == _CASTLE_TYPE_V3P_)
	{
		if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_	||
		    pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
		    pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
		{       
			/* 交易完成 票卡餘額 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_CHECK_TICKET_RESULT_01_, 0, _COORDINATE_Y_LINE_8_4_);

			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			if (pobTran->srTRec.lnCardRemainAmount < 0)
			{
				sprintf(szDispMsg, "-$%ld", 0 - pobTran->srTRec.lnCardRemainAmount);
			}
			else
			{
				sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnCardRemainAmount);
			}

			inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
		}
		else
		{
			inNCCC_Ticket_Disp_Receipt(pobTran);
		}
	}
	/* 小螢幕V3C */
	else if (ginMachineType == _CASTLE_TYPE_V3C_ && ginHalfLCD == VS_TRUE)
	{
		/* 客製化107.111移除請按確認鍵跟請按清除鍵 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			/* 自動加值金額大於0 */
			if (pobTran->srTRec.lnTotalTopUpAmount > 0L)
			{
				if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_)
				{
					/* 交易完成 自動加值 票卡餘額 */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_PutGraphic(_CHECK_TICKET_RESULT_02_, 0, _COORDINATE_Y_LINE_8_4_);
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount);
					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					if (pobTran->srTRec.lnCardRemainAmount < 0)
					{
						sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount - pobTran->srTRec.lnCardRemainAmount);
					}
					else
					{
						sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount + pobTran->srTRec.lnCardRemainAmount);
					}

					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
				}
				else
				{
					/* 交易完成 自動加值 票卡餘額 */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_PutGraphic(_CHECK_TICKET_RESULT_02_, 0, _COORDINATE_Y_LINE_8_4_);
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount);
					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					if (pobTran->srTRec.lnFinalAfterAmt > 100000)
					{
						sprintf(szDispMsg, "-$%ld ", pobTran->srTRec.lnFinalAfterAmt - 100000);
					}
					else
					{
						sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnFinalAfterAmt);
					}

					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
				}
			}
			else
			{
				if (pobTran->srTRec.inCode == _TICKET_IPASS_TOP_UP_ ||
				    pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_ ||
				    pobTran->srTRec.inCode == _TICKET_ICASH_TOP_UP_)
				{
					/* 加值完成 加值金額 票卡餘額 */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_PutGraphic(_CHECK_TICKET_RESULT_03_, 0, _COORDINATE_Y_LINE_8_4_);

					/* 加值金額 */
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTxnAmount);
					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

					/* 票卡餘額 */
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					if (pobTran->srTRec.lnFinalAfterAmt > 100000)
					{
						sprintf(szDispMsg, "-$%ld ", pobTran->srTRec.lnFinalAfterAmt - 100000);
					}
					else
					{
						sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnFinalAfterAmt);
					}

					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
				}
				else if (pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_ ||
					 pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_ ||
					 pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_)
				{
					/* 取消加值完成 取消金額 票卡餘額 */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_PutGraphic(_CHECK_TICKET_RESULT_04_, 0, _COORDINATE_Y_LINE_8_4_);

					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnTxnAmount);
					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					if (pobTran->srTRec.lnFinalAfterAmt > 100000)
					{
						sprintf(szDispMsg, "-$%ld", pobTran->srTRec.lnFinalAfterAmt - 100000);
					}
					else
					{
						sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnFinalAfterAmt);
					}

					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
				}        
				else if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ ||
					 pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
					 pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
				{       
					/* 交易完成 票卡餘額 */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_PutGraphic(_CHECK_TICKET_RESULT_01_, 0, _COORDINATE_Y_LINE_8_4_);

					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					if (pobTran->srTRec.lnCardRemainAmount < 0)
					{
						sprintf(szDispMsg, "-$%ld", 0 - pobTran->srTRec.lnCardRemainAmount);
					}
					else
					{
						sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnCardRemainAmount);
					}

					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
				}
				else
				{
					/* 交易完成 票卡餘額 */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_PutGraphic(_CHECK_TICKET_RESULT_01_, 0, _COORDINATE_Y_LINE_8_4_);

					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					if (pobTran->srTRec.lnFinalAfterAmt > 100000)
					{
						sprintf(szDispMsg, "-$%ld", pobTran->srTRec.lnFinalAfterAmt - 100000);
					}
					else
					{
						sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnFinalAfterAmt);
					}

					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
				}
			}
		}
		/* 一般狀況 */
		else
		{
			if (pobTran->srTRec.lnTotalTopUpAmount > 0L)
			{        
				if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_)
				{
					/* 交易完成 自動加值 票卡餘額 請按確認或清除 */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_PutGraphic(_CHECK_TICKET_RESULT_02_, 0, _COORDINATE_Y_LINE_8_4_);
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount);
					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					if (pobTran->srTRec.lnCardRemainAmount < 0)
					{
						sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount - pobTran->srTRec.lnCardRemainAmount);
					}
					else
					{
						sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount + pobTran->srTRec.lnCardRemainAmount);
					}

					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
					if (uszEnterDisplayBit == VS_TRUE)
					{
						inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_7_);
					}
				}
				else
				{
					/* 交易完成 自動加值 票卡餘額 請按確認或清除 */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_PutGraphic(_CHECK_TICKET_RESULT_02_, 0, _COORDINATE_Y_LINE_8_4_);
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount);
					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					if (pobTran->srTRec.lnFinalAfterAmt > 100000)
					{
						sprintf(szDispMsg, "-$%ld ", pobTran->srTRec.lnFinalAfterAmt - 100000);
					}
					else
					{
						sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnFinalAfterAmt);
					}

					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
					if (uszEnterDisplayBit == VS_TRUE)
					{
						inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_7_);
					}
				}
			}
			else
			{
				if (pobTran->srTRec.inCode == _TICKET_IPASS_TOP_UP_ ||
				    pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_ ||
				    pobTran->srTRec.inCode == _TICKET_ICASH_TOP_UP_)
				{
					/* 加值完成 加值金額 票卡餘額 請按確認或清除 */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_PutGraphic(_CHECK_TICKET_RESULT_03_, 0, _COORDINATE_Y_LINE_8_4_);

					/* 加值金額 */
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTxnAmount);
					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

					/* 票卡餘額 */
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					if (pobTran->srTRec.lnFinalAfterAmt > 100000)
					{
						sprintf(szDispMsg, "-$%ld ", pobTran->srTRec.lnFinalAfterAmt - 100000);
					}
					else
					{
						sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnFinalAfterAmt);
					}

					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
					if (uszEnterDisplayBit == VS_TRUE)
					{
						inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_7_);
					}
				}
				else if (pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_ ||
					 pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_ ||
					 pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_)
				{
					/* 取消加值完成 取消金額 票卡餘額 請按確認或清除 */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_PutGraphic(_CHECK_TICKET_RESULT_04_, 0, _COORDINATE_Y_LINE_8_4_);

					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnTxnAmount);
					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					if (pobTran->srTRec.lnFinalAfterAmt > 100000)
					{
						sprintf(szDispMsg, "-$%ld", pobTran->srTRec.lnFinalAfterAmt - 100000);
					}
					else
					{
						sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnFinalAfterAmt);
					}

					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
					if (uszEnterDisplayBit == VS_TRUE)
					{
						inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_7_);
					}
				}        
				else if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ ||
					 pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
					 pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
				{       
					/* 交易完成 票卡餘額 請按確認或清除 */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_PutGraphic(_CHECK_TICKET_RESULT_01_, 0, _COORDINATE_Y_LINE_8_4_);

					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					if (pobTran->srTRec.lnCardRemainAmount < 0)
					{
						sprintf(szDispMsg, "-$%ld", 0 - pobTran->srTRec.lnCardRemainAmount);
					}
					else
					{
						sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnCardRemainAmount);
					}

					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
					inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);
				}
				else
				{
					/* 交易完成 票卡餘額 請按確認或清除 */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_PutGraphic(_CHECK_TICKET_RESULT_01_, 0, _COORDINATE_Y_LINE_8_4_);

					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					if (pobTran->srTRec.lnFinalAfterAmt > 100000)
					{
						sprintf(szDispMsg, "-$%ld", pobTran->srTRec.lnFinalAfterAmt - 100000);
					}
					else
					{
						sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnFinalAfterAmt);
					}

					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
					if (uszEnterDisplayBit == VS_TRUE)
					{
						inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_7_);
					}
				}
			}
		}
	}
	/* 大螢幕V3C */
	else
	{
		/* 客製化107.111移除請按確認鍵跟請按清除鍵 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_	||
			    pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
			    pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
			{       
				/* 交易完成 票卡餘額 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_CHECK_TICKET_RESULT_01_, 0, _COORDINATE_Y_LINE_8_4_);

				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				if (pobTran->srTRec.lnCardRemainAmount < 0)
				{
					sprintf(szDispMsg, "-$%ld", 0 - pobTran->srTRec.lnCardRemainAmount);
				}
				else
				{
					sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnCardRemainAmount);
				}

				inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
			}
			else
			{
				inNCCC_Ticket_Disp_Receipt(pobTran);
			}
		}
		/* 一般狀況 */
		else
		{
			if (pobTran->srTRec.lnTotalTopUpAmount > 0L)
			{        
				if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_)
				{
					/* 交易完成 自動加值 票卡餘額 請按確認或清除 */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_PutGraphic(_CHECK_TICKET_RESULT_02_, 0, _COORDINATE_Y_LINE_8_4_);
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount);
					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					if (pobTran->srTRec.lnCardRemainAmount < 0)
					{
						sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount - pobTran->srTRec.lnCardRemainAmount);
					}
					else
					{
						sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount + pobTran->srTRec.lnCardRemainAmount);
					}

					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
					if (uszEnterDisplayBit == VS_TRUE)
					{
						inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_7_);
					}
				}
				else
				{
					/* 交易完成 自動加值 票卡餘額 請按確認或清除 */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_PutGraphic(_CHECK_TICKET_RESULT_02_, 0, _COORDINATE_Y_LINE_8_4_);
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount);
					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					if (pobTran->srTRec.lnFinalAfterAmt > 100000)
					{
						sprintf(szDispMsg, "-$%ld ", pobTran->srTRec.lnFinalAfterAmt - 100000);
					}
					else
					{
						sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnFinalAfterAmt);
					}

					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
					if (uszEnterDisplayBit == VS_TRUE)
					{
						inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_7_);
					}
				}
			}
			else
			{
				if (pobTran->srTRec.inCode == _TICKET_IPASS_TOP_UP_ ||
				    pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_ ||
				    pobTran->srTRec.inCode == _TICKET_ICASH_TOP_UP_)
				{
					/* 加值完成 加值金額 票卡餘額 請按確認或清除 */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_PutGraphic(_CHECK_TICKET_RESULT_03_, 0, _COORDINATE_Y_LINE_8_4_);

					/* 加值金額 */
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTxnAmount);
					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

					/* 票卡餘額 */
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					if (pobTran->srTRec.lnFinalAfterAmt > 100000)
					{
						sprintf(szDispMsg, "-$%ld ", pobTran->srTRec.lnFinalAfterAmt - 100000);
					}
					else
					{
						sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnFinalAfterAmt);
					}

					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
                                        
					if (uszEnterDisplayBit == VS_TRUE)
					{
						inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_7_);
					}
				}
				else if (pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_ ||
					 pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_ ||
					 pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_)
				{
					/* 取消加值完成 取消金額 票卡餘額 請按確認或清除 */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_PutGraphic(_CHECK_TICKET_RESULT_04_, 0, _COORDINATE_Y_LINE_8_4_);

					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnTxnAmount);
					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					if (pobTran->srTRec.lnFinalAfterAmt > 100000)
					{
						sprintf(szDispMsg, "-$%ld", pobTran->srTRec.lnFinalAfterAmt - 100000);
					}
					else
					{
						sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnFinalAfterAmt);
					}

					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
					if (uszEnterDisplayBit == VS_TRUE)
					{
						inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_7_);
					}
				}        
				else if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ ||
					 pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
					 pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
				{       
					/* 交易完成 票卡餘額 請按確認或清除 */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_PutGraphic(_CHECK_TICKET_RESULT_01_, 0, _COORDINATE_Y_LINE_8_4_);

					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					if (pobTran->srTRec.lnCardRemainAmount < 0)
					{
						sprintf(szDispMsg, "-$%ld", 0 - pobTran->srTRec.lnCardRemainAmount);
					}
					else
					{
						sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnCardRemainAmount);
					}

					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
					inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);
				}
				else
				{
					/* 交易完成 票卡餘額 請按確認或清除 */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_PutGraphic(_CHECK_TICKET_RESULT_01_, 0, _COORDINATE_Y_LINE_8_4_);

					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					if (pobTran->srTRec.lnFinalAfterAmt > 100000)
					{
						sprintf(szDispMsg, "-$%ld", pobTran->srTRec.lnFinalAfterAmt - 100000);
					}
					else
					{
						sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnFinalAfterAmt);
					}

					inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
					if (uszEnterDisplayBit == VS_TRUE)
					{
						inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_7_);
					}
				}
			}
		}
	}
        	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_Get_DAVTI_Data
Date&Time       :2018/1/12 下午 1:50
Describe        :
*/
int inNCCC_Ticket_Get_DAVTI_Data(TRANSACTION_OBJECT *pobTran)
{
	char    szTemplate[12 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		if (pobTran->srTRec.inCode == _TICKET_IPASS_DEDUCT_)
		{
			memset(pobTran->srTRec.szTicketRefundCode, 0x00, sizeof(pobTran->srTRec.szTicketRefundCode));
			memcpy(&pobTran->srTRec.szTicketRefundCode[0], "222222", 6); 
		}
		else
		{
			memset(pobTran->srTRec.szTicketRefundCode, 0x00, sizeof(pobTran->srTRec.szTicketRefundCode));
			memcpy(&pobTran->srTRec.szTicketRefundCode[0], "000000", 6);   
		}        

		/* 悠遊卡不跑此流程，所以不用加 */
		if (pobTran->srTRec.inCode == _TICKET_IPASS_REFUND_	|| pobTran->srTRec.inCode == _TICKET_IPASS_TOP_UP_	||
		    pobTran->srTRec.inCode == _TICKET_ICASH_REFUND_	|| pobTran->srTRec.inCode == _TICKET_ICASH_TOP_UP_)
		{        
			pobTran->srTRec.lnFinalBeforeAmt = gulDemoTicketPoint;
			pobTran->srTRec.lnFinalAfterAmt = gulDemoTicketPoint + pobTran->srTRec.lnTxnAmount;     
		}
		else
		{        
			pobTran->srTRec.lnFinalBeforeAmt = gulDemoTicketPoint;
			pobTran->srTRec.lnFinalAfterAmt = gulDemoTicketPoint - pobTran->srTRec.lnTxnAmount;     
		}

		gulDemoTicketPoint = pobTran->srTRec.lnFinalAfterAmt;    
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "----------------------------------------");
			inLogPrintf(AT, "inNCCC_Ticket_Get_DAVTI_Data() START !");
		}
		
		if (ginFindRunTime == VS_TRUE)
		{
			inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
		}
		
		/* 取得交易結果 */
		if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &pobTran->srTRec.srIPASSRec.szDAVTITxn[99], 6);
			pobTran->srTRec.lnFinalBeforeAmt = atol(szTemplate);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &pobTran->srTRec.srIPASSRec.szDAVTITxn[111], 6);
			pobTran->srTRec.lnFinalAfterAmt = atol(szTemplate);
		}
		else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
                {
                        switch (pobTran->srTRec.inCode)
                        {
                                case _TICKET_ICASH_DEDUCT_ :
                                        pobTran->srTRec.lnFinalBeforeAmt = pobTran->srTRec.lnCardRemainAmount - pobTran->srTRec.lnTotalTopUpAmount;
                                        pobTran->srTRec.lnFinalAfterAmt = pobTran->srTRec.lnCardRemainAmount - pobTran->srTRec.lnTxnAmount;
                                        break;
                                case _TICKET_ICASH_REFUND_ :
                                        pobTran->srTRec.lnFinalBeforeAmt = pobTran->srTRec.lnCardRemainAmount;
                                        pobTran->srTRec.lnFinalAfterAmt = pobTran->srTRec.lnCardRemainAmount + pobTran->srTRec.lnTxnAmount;
                                        break;
                                case _TICKET_ICASH_TOP_UP_ :
                                        pobTran->srTRec.lnFinalBeforeAmt = pobTran->srTRec.lnCardRemainAmount;
                                        pobTran->srTRec.lnFinalAfterAmt = pobTran->srTRec.lnCardRemainAmount + pobTran->srTRec.lnTxnAmount;
                                        break;
                                case _TICKET_ICASH_VOID_TOP_UP_ :
                                        pobTran->srTRec.lnFinalBeforeAmt = pobTran->srTRec.lnCardRemainAmount;
                                        pobTran->srTRec.lnFinalAfterAmt = pobTran->srTRec.lnCardRemainAmount - pobTran->srTRec.lnTxnAmount;
                                        break;
                                case _TICKET_ICASH_AUTO_TOP_UP_ :
                                        pobTran->srTRec.lnFinalBeforeAmt = pobTran->srTRec.lnCardRemainAmount - pobTran->srTRec.lnTotalTopUpAmount;
                                        pobTran->srTRec.lnFinalAfterAmt = pobTran->srTRec.lnCardRemainAmount;
                                        break;
                                default :
                                        break;
                        }

                        /* API寫入失敗，這裡就該回傳 */
                        if (pobTran->srTRec.srICASHRec.uszAPI_FailBit == VS_TRUE)
                        {
                                inFLOW_RunFunction(pobTran, _TICKET_UPDATE_INV_);
                                return (VS_ERROR);
                        }
                }
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_Ticket_Get_DAVTI_Data() END !");
			inLogPrintf(AT, "----------------------------------------");
		}
	}
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_Table_Link_TRec
Date&Time       :2018/1/15 下午 1:33
Describe        :將pobTran變數pointer位置放到Table中(用以解決每一個function都要放一個table的問題)
 *		 這邊直接把pobTran的pointer直接指到srAll(之後可能要考慮給動態記憶體)，TagName因為是寫在這個Function內的Table，所以要給實體位置儲存
*/
int inNCCC_Ticket_Table_Link_TRec(TRANSACTION_OBJECT *pobTran, SQLITE_ALL_TABLE *srAll, int inLinkState)
{
	SQLITE_INT32T_TABLE TABLE_BATCH_INT[] = 
	{
		{0	,"inTableID"			,&pobTran->inTableID			},	/* inTableID */
		{0	,"inCode"			,&pobTran->srTRec.inCode		},	
		{0	,"inPrintOption"		,&pobTran->srTRec.inPrintOption		},	/* Print Option Flag (also in TCT) */
		{0	,"inTicketType"			,&pobTran->srTRec.inTicketType		},	/* 票證種類 - 交易或明細使用  */
		{0	,"inTDTIndex"			,&pobTran->srTRec.inTDTIndex		},	/* 存TDT的index */
		{0	,"lnTxnAmount"			,&pobTran->srTRec.lnTxnAmount		},	/* 交易金額 */
		{0	,"lnTopUpAmount"		,&pobTran->srTRec.lnTopUpAmount		},	/* 加值金額(基底) */
		{0	,"lnTotalTopUpAmount"		,&pobTran->srTRec.lnTotalTopUpAmount	},	/* 加值金額 */
		{0	,"lnCardRemainAmount"		,&pobTran->srTRec.lnCardRemainAmount	},	/* 卡片餘額 */
		{0	,"lnInvNum"			,&pobTran->srTRec.lnInvNum		},	/* For簽單使用，簽單序號 */
		{0	,"lnECCInvNum"			,&pobTran->srTRec.lnECCInvNum		},
		{0	,"lnSTAN"			,&pobTran->srTRec.lnSTAN		},	/* */
		{0	,"lnFinalBeforeAmt"		,&pobTran->srTRec.lnFinalBeforeAmt	},	/* 最後交易結構，交易前卡片餘額 */
		{0	,"lnFinalAfterAmt"		,&pobTran->srTRec.lnFinalAfterAmt	},	/* 最後交易結構，交易後卡片餘額 */
		{0	,"lnMainInvNum"			,&pobTran->srTRec.lnMainInvNum		},	/* Confirm use 電文序號，因電票中一筆交易可能有數筆電文，所以簽單序號和電文序號分開 */
		{0	,"lnCountInvNum"		,&pobTran->srTRec.lnCountInvNum		},	/* Confirm use 若有advice，要預先跳過的電文序號 */
		{0	,""				,NULL					}	/* 這行用Null用來知道尾端在哪 */
		
	};
	
	SQLITE_INT32T_TABLE TABLE_BATCH_INT64T[] = 
	{

		{0	,""				,NULL				}	/* 這行用Null用來知道尾端在哪 */
	};
	
	SQLITE_CHAR_TABLE TABLE_BATCH_CHAR[] =
	{
	        {0	,"szUID"			,&pobTran->srTRec.szUID			,sizeof(pobTran->srTRec.szUID)			},	/* 卡號 or UID number */
		{0	,"szDate"			,&pobTran->srTRec.szDate		,sizeof(pobTran->srTRec.szDate)			},	/* YYMMDD */
		{0	,"szOrgDate"			,&pobTran->srTRec.szOrgDate		,sizeof(pobTran->srTRec.szOrgDate)		},	/* YYMMDD */
		{0	,"szTime"			,&pobTran->srTRec.szTime		,sizeof(pobTran->srTRec.szTime)			},	/* HHMMSS */
		{0	,"szOrgTime"			,&pobTran->srTRec.szOrgTime		,sizeof(pobTran->srTRec.szOrgTime)		},	/* HHMMSS */
		{0	,"szAuthCode"			,&pobTran->srTRec.szAuthCode		,sizeof(pobTran->srTRec.szAuthCode)		},	/* */
		{0	,"szECCAuthCode"		,&pobTran->srTRec.szECCAuthCode		,sizeof(pobTran->srTRec.szECCAuthCode)		},
		{0	,"szRespCode"			,&pobTran->srTRec.szRespCode		,sizeof(pobTran->srTRec.szRespCode)		},	/* Response Code */
		{0	,"szProductCode"		,&pobTran->srTRec.szProductCode		,sizeof(pobTran->srTRec.szProductCode)		},	/* 產品代碼  */
		{0	,"szTicketRefundCode"		,&pobTran->srTRec.szTicketRefundCode	,sizeof(pobTran->srTRec.szTicketRefundCode)	},	/* 退貨序號  */
		{0	,"szTicketRefundDate"		,&pobTran->srTRec.szTicketRefundDate	,sizeof(pobTran->srTRec.szTicketRefundDate)	},	/* MMDD */
		{0	,"szStoreID"			,&pobTran->srTRec.szStoreID		,sizeof(pobTran->srTRec.szStoreID)		},	/* */
		{0	,"szRefNo"			,&pobTran->srTRec.szRefNo		,sizeof(pobTran->srTRec.szRefNo)		},	/* Reference Number(RRN) */
		{0	,"szAwardNum"			,&pobTran->srTRec.szAwardNum		,sizeof(pobTran->srTRec.szAwardNum)		},	/* 優惠個數 */
		{0	,"szAwardSN"			,&pobTran->srTRec.szAwardSN		,sizeof(pobTran->srTRec.szAwardSN)		},	/* 優惠序號(Award S/N) TID(8Bytes)+YYYYMMDDhhmmss(16 Bytes)，共22Bytes */
		{0	,"uszAutoTopUpBit"		,&pobTran->srTRec.uszAutoTopUpBit	,sizeof(pobTran->srTRec.uszAutoTopUpBit)	},	/* 是否自動加值 */
		{0	,"uszBlackListBit"		,&pobTran->srTRec.uszBlackListBit	,sizeof(pobTran->srTRec.uszBlackListBit)	},	/* 是否在黑名單中 */
		{0	,"uszOfflineBit"		,&pobTran->srTRec.uszOfflineBit		,sizeof(pobTran->srTRec.uszOfflineBit)		},	/* 離線交易 */
		{0	,"uszTicketConnectBit"		,&pobTran->srTRec.uszTicketConnectBit	,sizeof(pobTran->srTRec.uszTicketConnectBit)	},	/* 是否連線中 */
		{0	,"uszResponseBit"		,&pobTran->srTRec.uszResponseBit	,sizeof(pobTran->srTRec.uszResponseBit)		},	/* 票值回覆用 */
		{0	,"uszCloseAutoADDBit"		,&pobTran->srTRec.uszCloseAutoTopUpBit	,sizeof(pobTran->srTRec.uszCloseAutoTopUpBit)	},	/* 關閉自動加值用 */
		{0	,"uszStopPollBit"		,&pobTran->srTRec.uszStopPollBit	,sizeof(pobTran->srTRec.uszStopPollBit)		},	/* Mifare Stop */
		{0	,"uszConfirmBit"		,&pobTran->srTRec.uszConfirmBit		,sizeof(pobTran->srTRec.uszConfirmBit)		},	/* IPASS Confirm Inv use */
		{0	,"uszESVCTransBit"		,&pobTran->srTRec.uszESVCTransBit	,sizeof(pobTran->srTRec.uszESVCTransBit)	},	/* 代表是電票交易 */
		{0	,"uszRewardSuspendBit"		,&pobTran->srTRec.uszRewardSuspendBit	,sizeof(pobTran->srTRec.uszRewardSuspendBit)	},	/* 暫停優惠服務(優惠活動為0且要印補充資訊) */
		{0	,"uszRewardL1Bit"		,&pobTran->srTRec.uszRewardL1Bit	,sizeof(pobTran->srTRec.uszRewardL1Bit)		},	/* 要印L1 */
		{0	,"uszRewardL2Bit"		,&pobTran->srTRec.uszRewardL2Bit	,sizeof(pobTran->srTRec.uszRewardL2Bit)		},	/* 要印L2 */
		{0	,"uszRewardL5Bit"		,&pobTran->srTRec.uszRewardL5Bit	,sizeof(pobTran->srTRec.uszRewardL5Bit)		},	/* 要印L5 */
		{0	,"uszMPASTransBit"		,&pobTran->srTRec.uszMPASTransBit	,sizeof(pobTran->srTRec.uszMPASTransBit)	},	/* 是否為小額交易 */
		{0	,"uszMPASReprintBit"		,&pobTran->srTRec.uszMPASReprintBit	,sizeof(pobTran->srTRec.uszMPASReprintBit)	},	/* 是否可重印 */
		{0	,"srIPASSRec"			,&pobTran->srTRec.srIPASSRec		,sizeof(pobTran->srTRec.srIPASSRec)		},	/* IPASS結構，求快先整個存 */
		{0	,"srECCRec"			,&pobTran->srTRec.srECCRec		,sizeof(pobTran->srTRec.srECCRec)		},	/* ECC結構，求快先整個存 */
		{0	,"srICASHRec"			,&pobTran->srTRec.srICASHRec		,sizeof(pobTran->srTRec.srICASHRec)		},	/* ICASH結構，求快先整個存 */
		{0	,""				,NULL					,0						}	/* 這行用Null用來知道尾端在哪 */
	};
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inNCCC_Ticket_Table_Link_TRec()_START");
        }
	
	SQLITE_LINK_TABLE	srLink;
	memset(&srLink, 0x00, sizeof(SQLITE_LINK_TABLE));
	srLink.psrInt = TABLE_BATCH_INT;
	srLink.psrInt64t = TABLE_BATCH_INT64T;
	srLink.psrChar = TABLE_BATCH_CHAR;
	srLink.psrText = NULL;
	
	inSqlite_Table_Link(srAll, inLinkState, &srLink);
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_Ticket_Table_Link_TRec()_END");
                inLogPrintf(AT, "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inTicket_Logon_ShowResult
Date&Time       :2018/1/17 下午 3:32
Describe        :
*/
int inNCCC_Ticket_Logon_ShowResult()
{
	int			i = 0;
	int			inHostOpenCnt = 0;
	int			inHostShowCnt = 0;
	int			inChoice = 0;
	int			inTouchSensorFunc = _Touch_BATCH_END_;
	int			inRetVal = VS_ERROR;
	char			szTemplate[2 + 1] = {0};
	char			szTicketEnable[4 + 1] = {0};	/* index0為IPASS 依序為ECC、ICASH */
	char			szTicketLogOn[4 + 1] = {0};
	char			szTicketFailLogOnResult[4 + 1] = {0};
	char			szHost[4][10 + 1];
	char			szCustomerIndicator[3 + 1] = {0};
	unsigned char		uszKey = 0x00;
        
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* IPASS */
        inLoadTDTRec(_TDT_INDEX_00_IPASS_);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostEnable(szTemplate);
	szTicketEnable[0] = szTemplate[0];
	
        inGetTicket_LogOnOK(szTemplate);
	szTicketLogOn[0] = szTemplate[0];
	
	/* ECC */
	inLoadTDTRec(_TDT_INDEX_01_ECC_);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostEnable(szTemplate);
	szTicketEnable[1] = szTemplate[0];
	
        inGetTicket_LogOnOK(szTemplate);
	szTicketLogOn[1] = szTemplate[0];
	
	/* IACSH */
	inLoadTDTRec(_TDT_INDEX_02_ICASH_);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostEnable(szTemplate);
	szTicketEnable[2] = szTemplate[0];
	
        inGetTicket_LogOnOK(szTemplate);
	szTicketLogOn[2] = szTemplate[0];
	
	for (i = 0; i < 4; i++)
	{
		if (szTicketEnable[i] == 'Y')
		{
			inHostOpenCnt++;
		}
		
		/* 有開但沒LogOn成功 */
		if (szTicketEnable[i] == 'Y' &&
		    szTicketLogOn[i] == 'N')
		{
			szTicketFailLogOnResult[i] = 'Y';
		}
		else
		{
			szTicketFailLogOnResult[i] = 'N';
		}
	}
	

	memset(szHost, 0x00, sizeof(szHost));
	
	if (szTicketFailLogOnResult[1] == 'Y')
	{
		strcat(&szHost[inHostShowCnt][0], _HOST_NAME_ECC_);
		inHostShowCnt++;
	}
	
	if (szTicketFailLogOnResult[0] == 'Y')
	{
		strcat(&szHost[inHostShowCnt][0], _HOST_NAME_IPASS_);
		inHostShowCnt++;
	}
	
	if (szTicketFailLogOnResult[2] == 'Y')
	{
		strcat(&szHost[inHostShowCnt][0], _HOST_NAME_ICASH_);
		inHostShowCnt++;
	}
	   
        if (inHostShowCnt > 0)
        {
		/* 客製化107、111及MPAS， 移除按確認鍵跟請按清除鍵 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			/* 根據開的電票家數不同，格式亦不同 */
			/* 全失敗的情況 */
			if (inHostOpenCnt == inHostShowCnt)
			{
				/* 電票認證失敗 請重開機 */
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);
				inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);
				inDISP_PutGraphic(_MSG_TICKET_SIGN_ON_FAILED_REBOOT_, 0, _COORDINATE_Y_LINE_8_4_);

				inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
				while (1)
				{
					uszKey = uszKBD_Key();
					
					if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
					{
						uszKey = _KEY_TIMEOUT_;
					}

					if (uszKey == _KEY_TIMEOUT_)
					{
						inRetVal = VS_TIMEOUT;
						break;
					}
					else
					{
						continue;
					}
				}
				
				return (inRetVal);
			}
			else if (inHostOpenCnt > 1	&&
				 inHostOpenCnt <= _TDT_INDEX_MAX_)
			{
				/* 電票認證失敗 請重開機 */
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);
				inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);
				for (i = 0; i < inHostShowCnt; i++)
				{
					inDISP_ChineseFont(&szHost[i][0], _FONTSIZE_8X16_, _LINE_8_4_ + i, _DISP_LEFT_);
					inDISP_ChineseFont("認證失敗", _FONTSIZE_8X16_, _LINE_8_4_ + i, _DISP_RIGHT_);
				}

				inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
				while (1)
				{
					uszKey = uszKBD_Key();

					if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
					{
						uszKey = _KEY_TIMEOUT_;
					}

					if (uszKey == _KEY_TIMEOUT_)
					{
						/* (需求單 - 107227)邦柏科技自助作業客製化 邦博客制化沒人按按鍵 必定timeout此時要能繼續做 by Russell 2018/12/7 下午 5:23 */
						/* (需求單 - 107276)自助交易標準400做法 沒人按按鍵 必定timeout此時要能繼續做 by Russell 2018/12/27 上午 11:20 */
						memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
						inGetCustomIndicator(szCustomerIndicator);
						if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
                                                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
						    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
						    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
						{
							inRetVal = VS_SUCCESS;
							break;
						}
						else
						{
							inRetVal = VS_TIMEOUT;
							break;
						}
					}
					else
					{
						continue;
					}
				}
				
				return (inRetVal);
			}
			else
			{
				/* 目前有三個票證，跑到這裡邏輯錯誤 */
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "inNCCC_Ticket_Logon_ShowResult 邏輯錯誤");
				}
				return (VS_ERROR);
			}
		}
		else
		{
			/* 根據開的電票家數不同，格式亦不同 */
			/* 全失敗的情況 */
			if (inHostOpenCnt == inHostShowCnt)
			{
				/* 電票認證失敗 請重開機 */
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);
				inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);
				inDISP_PutGraphic(_MSG_TICKET_SIGN_ON_FAILED_REBOOT_, 0, _COORDINATE_Y_LINE_8_4_);
				inDISP_PutGraphic(_ERR_CLEAR_, 0, _COORDINATE_Y_LINE_8_7_);

				inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
				while (1)
				{
					inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
					uszKey = uszKBD_Key();

					if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
					{
						uszKey = _KEY_TIMEOUT_;
					}

					if (uszKey == _KEY_CANCEL_	||
					    inChoice == _BATCH_END_Touch_ENTER_BUTTON_)
					{
						inRetVal = VS_USER_CANCEL;
						break;
					}
					else if (uszKey == _KEY_TIMEOUT_)
					{
						inRetVal = VS_TIMEOUT;
						break;
					}
					else
					{
						continue;
					}
				}
				
				inDisTouch_Flush_TouchFile();
				return (inRetVal);
			}
			else if (inHostOpenCnt > 1	&&
				 inHostOpenCnt <= _TDT_INDEX_MAX_)
			{
				/* 電票認證失敗 請重開機 */
				inDISP_ClearAll();
				inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);
				inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);
				for (i = 0; i < inHostShowCnt; i++)
				{
					inDISP_ChineseFont(&szHost[i][0], _FONTSIZE_8X16_, _LINE_8_4_ + i, _DISP_LEFT_);
					/* HAPPYCASH的H被蓋掉 中文字調小 */
					inDISP_ChineseFont("認證失敗", _FONTSIZE_8X22_, _LINE_8_4_ + i, _DISP_RIGHT_);
				}
				inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);

				inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
				while (1)
				{
					inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
					uszKey = uszKBD_Key();

					if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
					{
						uszKey = _KEY_TIMEOUT_;
					}

					if (uszKey == _KEY_ENTER_	||
					    inChoice == _BATCH_END_Touch_ENTER_BUTTON_)
					{
						inRetVal = VS_SUCCESS;
						break;
					}
					else if (uszKey == _KEY_TIMEOUT_)
					{
						/* (需求單 - 107227)邦柏科技自助作業客製化 邦博客制化沒人按按鍵 必定timeout此時要能繼續做 by Russell 2018/12/7 下午 5:23 */
						/* (需求單 - 107276)自助交易標準400做法 沒人按按鍵 必定timeout此時要能繼續做 by Russell 2018/12/27 上午 11:20 */
						memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
						inGetCustomIndicator(szCustomerIndicator);
						if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
                                                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
						    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
						    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
						{
							inRetVal = VS_SUCCESS;
							break;
						}
						else
						{
							inRetVal = VS_TIMEOUT;
							break;
						}
					}
					else
					{
						continue;
					}
				}
				
				inDisTouch_Flush_TouchFile();
				return (inRetVal);
			}
			else
			{
				/* 目前有三個票證，跑到這裡邏輯錯誤 */
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "inNCCC_Ticket_Logon_ShowResult 邏輯錯誤");
				}
				inRetVal = VS_ERROR;
				return (inRetVal);
			}
		}
        }
	else
	{
		/* 表示全成功，不顯示 */
		inRetVal = VS_SUCCESS;
		return (inRetVal);
	}
}

/*
Function        :inNCCC_Ticket_Logon_ShowResult_Customer_107
Date&Time       :2019/1/22 上午 10:43
Describe        :
*/
int inNCCC_Ticket_Logon_ShowResult_Customer_107()
{
	int			i = 0;
	int			inHostOpenCnt = 0;
	int			inHostShowCnt = 0;
	int			inRetVal = VS_ERROR;
	char			szTemplate[2 + 1] = {0};
	char			szTicketEnable[_TDT_INDEX_MAX_ + 1] = {0};	/* index0為IPASS 依序為ECC、ICASH */
	char			szTicketLogOn[_TDT_INDEX_MAX_ + 1] = {0};
	char			szTicketFailLogOnResult[_TDT_INDEX_MAX_ + 1] = {0};
	char			szHost[_TDT_INDEX_MAX_][20 + 1];
	unsigned char		uszKey = 0x00;
	
	/* IPASS */
        inLoadTDTRec(_TDT_INDEX_00_IPASS_);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostEnable(szTemplate);
	szTicketEnable[0] = szTemplate[0];
	
        inGetTicket_LogOnOK(szTemplate);
	szTicketLogOn[0] = szTemplate[0];
	
	/* ECC */
	inLoadTDTRec(_TDT_INDEX_01_ECC_);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostEnable(szTemplate);
	szTicketEnable[1] = szTemplate[0];
	
        inGetTicket_LogOnOK(szTemplate);
	szTicketLogOn[1] = szTemplate[0];
	
	/* IACSH */
	inLoadTDTRec(_TDT_INDEX_02_ICASH_);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostEnable(szTemplate);
	szTicketEnable[2] = szTemplate[0];
	
        inGetTicket_LogOnOK(szTemplate);
	szTicketLogOn[2] = szTemplate[0];
	
	for (i = 0; i < _TDT_INDEX_MAX_; i++)
	{
		if (szTicketEnable[i] == 'Y')
		{
			inHostOpenCnt++;
		}
		
		/* 有開但沒LogOn成功 */
		if (szTicketEnable[i] == 'Y' &&
		    szTicketLogOn[i] == 'N')
		{
			szTicketFailLogOnResult[i] = 'Y';
		}
		else
		{
			szTicketFailLogOnResult[i] = 'N';
		}
	}
	

	memset(szHost, 0x00, sizeof(szHost));
	
	if (szTicketFailLogOnResult[1] == 'Y')
	{
		strcat(&szHost[inHostShowCnt][0], "悠遊卡");
		inHostShowCnt++;
	}
	
	if (szTicketFailLogOnResult[0] == 'Y')
	{
		strcat(&szHost[inHostShowCnt][0], "一卡通");
		inHostShowCnt++;
	}
	
	if (szTicketFailLogOnResult[2] == 'Y')
	{
		strcat(&szHost[inHostShowCnt][0], "iCASH");
		inHostShowCnt++;
	}
	   
        if (inHostShowCnt > 0)
        {
		/* 根據開的電票家數不同，格式亦不同 */
		/* 全失敗的情況 */
		if (inHostOpenCnt == inHostShowCnt)
		{
			/* 電票認證失敗 請重開機 */
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);
			inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);
			inDISP_PutGraphic(_MSG_TICKET_SIGN_ON_FAILED_REBOOT_, 0, _COORDINATE_Y_LINE_8_4_);

			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_);
			while (1)
			{
				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
				{
					uszKey = _KEY_TIMEOUT_;
				}

				if (uszKey == _KEY_TIMEOUT_)
				{
					inRetVal = VS_TIMEOUT;
					break;
				}
				else
				{
					continue;
				}
			}

			return (inRetVal);
		}
		else if (inHostOpenCnt == 2	||
			 inHostOpenCnt == 3	||
			 inHostOpenCnt == 4)
		{
			/* OO卡 暫停服務 */
			inDISP_ClearAll();
			inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);
			inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);
			for (i = 0; i < inHostShowCnt; i++)
			{
				inDISP_ChineseFont(&szHost[i][0], _FONTSIZE_8X16_, _LINE_8_4_ + i, _DISP_LEFT_);
			}
			inDISP_ChineseFont("暫停服務", _FONTSIZE_8X16_, _LINE_8_4_ + i, _DISP_LEFT_);

			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_);
			while (1)
			{
				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
				{
					uszKey = _KEY_TIMEOUT_;
				}

				if (uszKey == _KEY_TIMEOUT_)
				{
					inRetVal = VS_SUCCESS;
					break;
				}
				else
				{
					continue;
				}
			}

			return (inRetVal);
		}
		else
		{
			/* 目前只有四個票證，跑到這裡邏輯錯誤 */
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inNCCC_Ticket_Logon_ShowResult_Customer_107 邏輯錯誤");
			}
			return (VS_ERROR);
		}
        }
	else
	{
		/* 表示全成功，不顯示 */
		inRetVal = VS_SUCCESS;
		return (inRetVal);
	}
}


/*
Function        :inNCCC_Ticket_Logon_ShowResult_Flow
Date&Time       :2019/1/22 上午 9:41
Describe        :j為了要分版本顯示，所以多做一個流程
*/
int inNCCC_Ticket_Logon_ShowResult_Flow()
{
	int	inRetVal = VS_ERROR;
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inRetVal = inNCCC_Ticket_Logon_ShowResult_Customer_107();
	}
	else
	{
		inRetVal = inNCCC_Ticket_Logon_ShowResult();
	}
	
	return (inRetVal);
}

/*
Function        :inNCCC_Ticket_Display_LogOn_Result
Date&Time       :2018/12/7 下午 5:19
Describe        :
*/
int inNCCC_Ticket_Display_LogOn_Result(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
        
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Ticket_Display_LogOn_Result START!");
	
	/* 已顯示過，就不再顯示 */
	if (pobTran->uszESVCLogOnDispBit == VS_TRUE)
	{
		inRetVal = VS_SUCCESS;
	}
	else
	{
		inRetVal = inNCCC_Ticket_Logon_ShowResult_Flow();
		pobTran->uszESVCLogOnDispBit = VS_TRUE;
	}
	
	return (inRetVal);
}

/*
Function        :inNCCC_Ticket_Fast_Tap_Wait
Date&Time       :2018/1/17 下午 5:36
Describe        :
*/
int inNCCC_Ticket_Fast_Tap_Wait(TRANSACTION_OBJECT * pobTran, char *szUID)
{
	int		inRetVal = VS_SUCCESS;
	char		szTemplate[_DISP_MSG_SIZE_ + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	long		lnTimeout = 0;
	unsigned char	uszKey = 0x00;
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	inDISP_ClearAll();
	 /* 顯示對應交易別的感應畫面 */
	inCTLS_Decide_Display_Image(pobTran);
	
	/* 顯示虛擬LED(不閃爍)，但inCTLS_ReceiveReadyForSales(V3機型)也會顯示LED（閃爍），所以沒有實質作用 */
	inCTLS_LED_Wait_Start();
        
	/* 進入畫面時先顯示金額 */
	if (pobTran->srBRec.lnTxnAmount > 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
		inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
		inDISP_ChineseFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_RIGHT_);
	}
	
	/* 客製化123，過卡時設為較最亮 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(_SCREEN_BRIGHTNESS_MAX_));
	}
	
	/* 設定Timeout */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_;
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_005_FPG_GET_CARD_TIMEOUT_;
	}
	else
	{
		if (pobTran->uszECRBit == VS_TRUE)
		{
			/* [20251219_BUG_MDF][UI] 修改ECR啟用時,電票感應時間為20秒 */
			lnTimeout = _ECR_RS232_TICKET_GET_CARD_TIMEOUT_;
			
		}
		else
		{
			lnTimeout = 30;
		}
	}
		
	/* inSecond剩餘倒數時間 */
	inDISP_TimeoutStart(lnTimeout);
	/* 立既顯示倒數秒數 */
	inDISP_LiveCountdown(lnTimeout,_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_);
	
	while (1)
	{
		uszKey = uszKBD_Key();
		
		/* 感應倒數時間 && Display Countdown */
		if (inDISP_TimeoutCheck(_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_) == VS_TIMEOUT)
		{
			/* 感應時間到Timeout */
			uszKey = _KEY_TIMEOUT_;
		}
		
		if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (inCTLS_Check_TypeACard() == VS_SUCCESS)
		{
			inCTLS_Get_TypeACardSN(szUID);
			inRetVal = VS_SUCCESS;
			break;
		}
	}
	
	/* 客製化123，過卡完時設為較暗 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetScreenBrightness(szTemplate);
		if (strlen(szTemplate) > 0)
		{
			inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(szTemplate));
		}
	}

	return (inRetVal); 
}

/*
Function        :inNCCC_Ticket_Func_Display_Review_Settle
Date&Time       :2018/1/19 下午 4:03
Describe        :
*/
int inNCCC_Ticket_Func_Display_Review_Settle(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = VS_SUCCESS;
	int			inChoice = 0;
	int			inTouchSensorFunc = _Touch_NEWUI_IDLE_;
	char			szTemplate1[44 + 1] = {0};
	char			szTemplate2[44 + 1] = {0};
	char			szDispBuffer1[44 + 1] = {0};
	char			szDispBuffer2[44 + 1] = {0};
	long long		llSum = 0;
	long long		llDeductSum = 0;
	long long		llRefundSum = 0;
	unsigned char		uszKey = 0x00;
	TICKET_ACCUM_TOTAL_REC	srAccumRec = {0};
	
	/* 讀交易資料，並放交易查詢Title */
	memset(&srAccumRec, 0x00, sizeof(TICKET_ACCUM_TOTAL_REC));
	inACCUM_GetRecord_ESVC(pobTran, &srAccumRec);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_REVIEW_SETTLE_ESVC_1_, 0, _COORDINATE_Y_LINE_8_4_);
	
	/* 購貨筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	llSum = srAccumRec.lnIPASS_DeductTotalCount + srAccumRec.lnEASYCARD_DeductTotalCount + srAccumRec.lnICASH_DeductTotalCount;

	sprintf(szTemplate1, "%03lld", llSum);
	sprintf(szDispBuffer1, "%s", szTemplate1);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_4_, _COLOR_BLACK_,_COLOR_WHITE_, 7);

	/* 購貨金額 */
	memset(szDispBuffer2, 0x00, sizeof(szDispBuffer2));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	llSum = srAccumRec.llIPASS_DeductTotalAmount + srAccumRec.llEASYCARD_DeductTotalAmount + srAccumRec.llICASH_DeductTotalAmount;
	llDeductSum = llSum;
	if (abs(llSum) < 100000000)
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 11, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X22_, _LINE_8_4_, _COLOR_BLACK_,_COLOR_WHITE_, 10);
	}
	else
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 14, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X33_, _LINE_8_4_, _COLOR_BLACK_,_COLOR_WHITE_, 13);
	}
	
	/* 退貨筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	llSum = srAccumRec.lnIPASS_RefundTotalCount + srAccumRec.lnEASYCARD_RefundTotalCount + srAccumRec.lnICASH_RefundTotalCount;

	sprintf(szTemplate1, "%03lld", llSum);
	sprintf(szDispBuffer1, "%s", szTemplate1);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_,_COLOR_WHITE_, 7);
	
	/* 退貨金額 */
	memset(szDispBuffer2, 0x00, sizeof(szDispBuffer2));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	llSum = srAccumRec.llIPASS_RefundTotalAmount + srAccumRec.llEASYCARD_RefundTotalAmount + srAccumRec.llICASH_RefundTotalAmount;
	llRefundSum = llSum;
	if (abs(llSum) < 100000000)
	{
		sprintf(szTemplate2, "%01lld", (0 - llSum));
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 11, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_,_COLOR_WHITE_, 10);
	}
	else
	{
		sprintf(szTemplate2, "%01lld", (0 - llSum));
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 14, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X33_, _LINE_8_5_, _COLOR_BLACK_,_COLOR_WHITE_, 13);
	}

	/* 加值筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	llSum = srAccumRec.lnIPASS_ADDTotalCount + srAccumRec.lnEASYCARD_ADDTotalCount + srAccumRec.lnICASH_ADDTotalCount;

	sprintf(szTemplate1, "%03lld", llSum);
	sprintf(szDispBuffer1, "%s", szTemplate1);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_,_COLOR_WHITE_, 7);

	/* 加值金額 */
	memset(szDispBuffer2, 0x00, sizeof(szDispBuffer2));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	llSum = srAccumRec.llIPASS_ADDTotalAmount + srAccumRec.llEASYCARD_ADDTotalAmount + srAccumRec.llICASH_ADDTotalAmount;
	if (abs(llSum) < 100000000)
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 11, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_,_COLOR_WHITE_, 10);
	}
	else
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 14, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X33_, _LINE_8_6_, _COLOR_BLACK_,_COLOR_WHITE_, 13);
	}
	
	/* 加值取消筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	llSum = srAccumRec.lnIPASS_VoidADDTotalCount + srAccumRec.lnEASYCARD_VoidADDTotalCount + srAccumRec.lnICASH_VoidADDTotalCount;

	sprintf(szTemplate1, "%03lld", llSum);
	sprintf(szDispBuffer1, "%s", szTemplate1);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_BLACK_,_COLOR_WHITE_, 7);
	
	/* 加值取消金額 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	llSum = srAccumRec.llIPASS_VoidADDTotalAmount + srAccumRec.llEASYCARD_VoidADDTotalAmount + srAccumRec.llICASH_VoidADDTotalAmount;
	if (abs(llSum) < 100000000)
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 11, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_BLACK_,_COLOR_WHITE_, 10);
	}
	else
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 14, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X33_, _LINE_8_7_, _COLOR_BLACK_,_COLOR_WHITE_, 13);
	}

	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	
	uszKey = 0x00;
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		uszKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}

		if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (inChoice == _NEWUI_IDLE_Touch_KEY_FUNCTIONKEY_	||
			 uszKey == _KEY_ENTER_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
	
	if (inRetVal == VS_USER_CANCEL ||
	    inRetVal == VS_TIMEOUT)
	{
		return (inRetVal);
	}
	
	/* 交易淨額 加值淨額 請按0取消 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_REVIEW_SETTLE_ESVC_2_, 0, _COORDINATE_Y_LINE_8_4_);
	
	/* 交易淨額筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	llSum = srAccumRec.lnDeductTotalCount + srAccumRec.lnRefundTotalCount;
	sprintf(szTemplate1, "%03lld", llSum);
	sprintf(szDispBuffer1, "%s", szTemplate1);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_4_, _COLOR_BLACK_,_COLOR_WHITE_, 7);
	
	/* 交易淨額金額 */
	memset(szDispBuffer2, 0x00, sizeof(szDispBuffer2));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	llSum = srAccumRec.llDeductTotalAmount - srAccumRec.llRefundTotalAmount;
	if (abs(llSum) < 100000000)
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 11, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X22_, _LINE_8_4_, _COLOR_BLACK_,_COLOR_WHITE_, 10);
	}
	else
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 14, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X33_, _LINE_8_4_, _COLOR_BLACK_,_COLOR_WHITE_, 13);
	}

	/* 加值淨額筆數 */
	llSum = srAccumRec.lnADDTotalCount + srAccumRec.lnVoidADDTotalCount;
	sprintf(szTemplate1, "%03lld", llSum);
	sprintf(szDispBuffer1, "%s", szTemplate1);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_,_COLOR_WHITE_, 7);

	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	/* 加值淨額 */
	llSum = srAccumRec.llADDTotalAmount - srAccumRec.llVoidADDTotalAmount;
	if (abs(llSum) < 100000000)
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 11, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_,_COLOR_WHITE_, 10);
	}
	else
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 14, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X33_, _LINE_8_6_, _COLOR_BLACK_,_COLOR_WHITE_, 13);
	}
	
	uszKey = 0x00;
	inTouchSensorFunc = _Touch_BATCH_END_;
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		uszKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}

		if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (inChoice == _BATCH_END_Touch_ENTER_BUTTON_	||
			 uszKey == _KEY_0_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
	
	return (inRetVal);
}

/*
Function        :inNCCC_Ticket_Func_Display_Review
Date&Time       :2018/1/26 下午 5:14
Describe        :
*/
int inNCCC_Ticket_Func_Display_Review(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = VS_SUCCESS;
	int			inChoice = 0;
	int			inTouchSensorFunc = _Touch_NEWUI_IDLE_;
	char			szTemplate1[44 + 1] = {0};
	char			szTemplate2[44 + 1] = {0};
	char			szDispBuffer1[44 + 1] = {0};
	char			szDispBuffer2[44 + 1] = {0};
	long long		llSum = 0;
	long long		llDeductSum = 0;
	long long		llRefundSum = 0;
	unsigned char		uszKey = 0x00;
	TICKET_ACCUM_TOTAL_REC	srAccumRec = {0};
	
	/* 讀交易資料，並放交易查詢Title */
	memset(&srAccumRec, 0x00, sizeof(TICKET_ACCUM_TOTAL_REC));
	inACCUM_GetRecord_ESVC(pobTran, &srAccumRec);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_REVIEW_SETTLE_ESVC_1_, 0, _COORDINATE_Y_LINE_8_4_);
	
	/* 購貨筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	llSum = srAccumRec.lnIPASS_DeductTotalCount + srAccumRec.lnEASYCARD_DeductTotalCount + srAccumRec.lnICASH_DeductTotalCount;

	sprintf(szTemplate1, "%03lld", llSum);
	sprintf(szDispBuffer1, "%s", szTemplate1);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_4_, _COLOR_BLACK_,_COLOR_WHITE_, 7);
	
	/* 購貨金額 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	llSum = srAccumRec.llIPASS_DeductTotalAmount + srAccumRec.llEASYCARD_DeductTotalAmount + srAccumRec.llICASH_DeductTotalAmount;
	llDeductSum = llSum;
	if (abs(llSum) < 100000000)
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 11, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X22_, _LINE_8_4_, _COLOR_BLACK_,_COLOR_WHITE_, 10);
	}
	else
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 14, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X33_, _LINE_8_4_, _COLOR_BLACK_,_COLOR_WHITE_, 13);
	}
	
	/* 退貨筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	llSum = srAccumRec.lnIPASS_RefundTotalCount + srAccumRec.lnEASYCARD_RefundTotalCount + srAccumRec.lnICASH_RefundTotalCount;

	sprintf(szTemplate1, "%03lld", llSum);
	sprintf(szDispBuffer1, "%s", szTemplate1);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_,_COLOR_WHITE_, 7);

	/* 退貨金額 */
	memset(szDispBuffer2, 0x00, sizeof(szDispBuffer2));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	llSum = srAccumRec.llIPASS_RefundTotalAmount + srAccumRec.llEASYCARD_RefundTotalAmount + srAccumRec.llICASH_RefundTotalAmount;
	llRefundSum = llSum;
	if (abs(llSum) < 100000000)
	{
		sprintf(szTemplate2, "%01lld", (0 - llSum));
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 11, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_,_COLOR_WHITE_, 10);
	}
	else
	{
		sprintf(szTemplate2, "%01lld", (0 - llSum));
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 14, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X33_, _LINE_8_5_, _COLOR_BLACK_,_COLOR_WHITE_, 13);
	}

	/* 加值筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	llSum = srAccumRec.lnIPASS_ADDTotalCount + srAccumRec.lnEASYCARD_ADDTotalCount + srAccumRec.lnICASH_ADDTotalCount;

	sprintf(szTemplate1, "%03lld", llSum);
	sprintf(szDispBuffer1, "%s", szTemplate1);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_,_COLOR_WHITE_, 7);

	/* 加值金額 */
	memset(szDispBuffer2, 0x00, sizeof(szDispBuffer2));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	llSum = srAccumRec.llIPASS_ADDTotalAmount + srAccumRec.llEASYCARD_ADDTotalAmount + srAccumRec.llICASH_ADDTotalAmount;
	if (abs(llSum) < 100000000)
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 11, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_,_COLOR_WHITE_, 10);
	}
	else
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 14, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X33_, _LINE_8_6_, _COLOR_BLACK_,_COLOR_WHITE_, 13);
	}
	
	/* 加值取消筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	llSum = srAccumRec.lnIPASS_VoidADDTotalCount + srAccumRec.lnEASYCARD_VoidADDTotalCount + srAccumRec.lnICASH_VoidADDTotalCount;

	sprintf(szTemplate1, "%03lld", llSum);
	sprintf(szDispBuffer1, "%s", szTemplate1);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_BLACK_,_COLOR_WHITE_, 7);

	/* 加值取消金額 */
	memset(szDispBuffer2, 0x00, sizeof(szDispBuffer2));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	llSum = srAccumRec.llIPASS_VoidADDTotalAmount + srAccumRec.llEASYCARD_VoidADDTotalAmount + srAccumRec.llICASH_VoidADDTotalAmount;
	if (abs(llSum) < 100000000)
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 11, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_BLACK_,_COLOR_WHITE_, 10);
	}
	else
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 14, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X33_, _LINE_8_7_, _COLOR_BLACK_,_COLOR_WHITE_, 13);
	}

	uszKey = 0x00;
	inTouchSensorFunc = _Touch_NEWUI_IDLE_;
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		uszKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}

		if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (inChoice == _NEWUI_IDLE_Touch_KEY_FUNCTIONKEY_	||
			 uszKey == _KEY_ENTER_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
	
	if (inRetVal == VS_USER_CANCEL ||
	    inRetVal == VS_TIMEOUT)
	{
		return (inRetVal);
	}
	
	/* 交易淨額 加值淨額 回上一頁 回主畫面 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_REVIEW_TOTAL_3_, 0, _COORDINATE_Y_LINE_8_4_);
	if (ginHalfLCD == VS_TRUE)
	{

	}
	else
	{
		if (guszNoChooseHostBit != VS_TRUE)
		{
			inDISP_PutGraphic(_ICON_BUTTON_LAST_PAGE_, _Distouch_NEWUI_LAST_PAGE_BUTTON_X1_, _Distouch_NEWUI_LAST_PAGE_BUTTON_Y1_);
		}
		inDISP_PutGraphic(_ICON_BUTTON_RETURN_IDLE_, _Distouch_NEWUI_RETURN_IDLE_BUTTON_X1_, _Distouch_NEWUI_RETURN_IDLE_BUTTON_Y1_);
	}

	/* 交易淨額筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	llSum = srAccumRec.lnDeductTotalCount + srAccumRec.lnRefundTotalCount;
	sprintf(szTemplate1, "%03lld", llSum);
	sprintf(szDispBuffer1, "%s", szTemplate1);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_4_, _COLOR_BLACK_,_COLOR_WHITE_, 7);

	/* 交易淨額金額 */
	memset(szDispBuffer2, 0x00, sizeof(szDispBuffer2));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	llSum = srAccumRec.llDeductTotalAmount - srAccumRec.llRefundTotalAmount;
	if (abs(llSum) < 100000000)
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 11, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X22_, _LINE_8_4_, _COLOR_BLACK_,_COLOR_WHITE_, 10);
	}
	else
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 14, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X33_, _LINE_8_4_, _COLOR_BLACK_,_COLOR_WHITE_, 13);
	}

	/* 加值淨額筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	
	llSum = srAccumRec.lnADDTotalCount + srAccumRec.lnVoidADDTotalCount;
	sprintf(szTemplate1, "%03lld", llSum);
	sprintf(szDispBuffer1, "%s", szTemplate1);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_,_COLOR_WHITE_, 7);
	
	/* 加值淨額 */
	memset(szDispBuffer2, 0x00, sizeof(szDispBuffer2));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	llSum = srAccumRec.llADDTotalAmount - srAccumRec.llVoidADDTotalAmount;
	if (abs(llSum) < 100000000)
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 11, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_,_COLOR_WHITE_, 10);
	}
	else
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 14, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X33_, _LINE_8_6_, _COLOR_BLACK_,_COLOR_WHITE_, 13);
	}
	
	uszKey = 0x00;
	inTouchSensorFunc = _Touch_NEWUI_REVIEW_TOTAL_;
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		uszKey = uszKBD_Key();
		
		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}

		if (inChoice == _NEWUI_REVIEW_TOTAL_Touch_RETURN_IDLE_BUTTON_	||
		    uszKey == _KEY_ENTER_					|| 
		    uszKey == _KEY_CANCEL_					||
		    uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
		else if (inChoice == _NEWUI_REVIEW_TOTAL_Touch_LAST_PAGE_BUTTON_	||
			 uszKey == _KEY_CLEAR_)
		{
			if (guszNoChooseHostBit != VS_TRUE)
			{
				inRetVal = VS_LAST_PAGE;
				pobTran->srBRec.inHDTIndex = -1;
				break;
			}
		}
	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
	
	return (inRetVal);
}

/*
Function        :inNCCC_Ticket_Insert_Advice_Ticket_Record
Date&Time       :2018/1/24 下午 12:10
Describe        :
*/
int inNCCC_Ticket_Insert_Advice_Ticket_Record(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
		
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_Ticket_Insert_Advice_Ticket_Record() START !");
	}
	
	/* 改以卡別紀錄 START */
	pobTran->uszESVCFileNameByCardBit = VS_TRUE;
	inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_BATCH_TABLE_TICKET_ADVICE_);
	/* 改以卡別紀錄 END */
	pobTran->uszESVCFileNameByCardBit = VS_FALSE;
	if (inRetVal != VS_SUCCESS)
	{
		inFunc_EDCLock(AT);
		return (inRetVal);
	}
	
	/* 改以卡別紀錄 START */
	pobTran->uszESVCFileNameByCardBit = VS_TRUE;
	inRetVal = inBATCH_Insert_All_Flow(pobTran, _TN_BATCH_TABLE_TICKET_ADVICE_);
	/* 改以卡別紀錄 END */
	pobTran->uszESVCFileNameByCardBit = VS_FALSE;
	if (inRetVal != VS_SUCCESS)
	{
		inFunc_EDCLock(AT);
		return (inRetVal);
	}
	
	/* 關閉自動加值只存一筆advice，沒還原會影響其他的Processing code */
	if (pobTran->srTRec.uszCloseAutoTopUpBit == VS_TRUE)
	{
	        /* Advice存完還原，不然此筆後都會送錯P-code */
	        inLogPrintf(AT, "inNCCC_Ticket_Insert_Advice_Ticket_Record()_uszCloseAutoTopUpBit");
	        pobTran->srTRec.uszCloseAutoTopUpBit = VS_FALSE;
	}
		
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Ticket_Insert_Advice_Ticket_Record() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inNCCC_Ticket_ESVC_Get_TRec_Top_Flow
Date&Time       :2017/3/14 下午 3:49
Describe        :
*/
int inNCCC_Ticket_ESVC_Get_TRec_Top_Flow(TRANSACTION_OBJECT *pobTran, int inTableType)
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
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_EMV_TABLE_:
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			inRetVal = inNCCC_Ticket_ESVC_Get_TRec_Top(pobTran, szTableName);
			break;
		default :
			return (VS_ERROR);
			break;
	}
	
	return (inRetVal);
}
/*
Function        :inNCCC_Ticket_ESVC_Get_TRec_Top
Date&Time       :2018/1/26 下午 2:36
Describe        :ADVICE會取table中Primary Key最小的值
*/
int inNCCC_Ticket_ESVC_Get_TRec_Top(TRANSACTION_OBJECT *pobTran, char* szTableName)
{
	int			j = 0;
	int			inIntIndex = 0, inInt64tIndex = 0, inCharIndex = 0;
	int			inCols = 0, inDataLen = 0;
	int			inRetVal = VS_SUCCESS;
	int			inFind = VS_FALSE;
	char			szDebugMsg[128 + 1];
	char			szQuerySql[300 + 1];	/* INSERT INTO	szTableName( */
	char			szTagName[_TAG_WIDTH_ + 1];
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	SQLITE_ALL_TABLE	srAll;
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inNCCC_Ticket_ESVC_Get_TRec_Top()_START");
        }
	
	inRetVal = inSqlite_DB_Open_Or_Create(gszTranDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
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
		inSqlite_DB_Close(&srDBConnection);
		
		return (VS_ERROR);
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "SELECT * FROM %s WHERE (inTableID = (SELECT MIN(inTableID) FROM %s))", szTableName, szTableName);

	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inLogPrintf(AT, szDebugMsg);
		}
		/* 替換資料前先清空srTRec */
		memset(&pobTran->srTRec, 0x00, sizeof(pobTran->srTRec));
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inLogPrintf(AT, szDebugMsg);
		}
		inSqlite_DB_Close(&srDBConnection);
		
		return (VS_NO_RECORD);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", sqlite3_errmsg(srDBConnection));
			inLogPrintf(AT, szDebugMsg);

			inLogPrintf(AT, szQuerySql);
		}

		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}

		return (VS_ERROR);
	}
	
	inCols = sqlite3_column_count(srSQLStat);
	
	/* binding 取得值 */
	for (j = 0; j < inCols; j++)
	{
		inFind = VS_FALSE;
		memset(szTagName, 0x00, sizeof(szTagName));
		strcat(szTagName, sqlite3_column_name(srSQLStat, j));
		
		
		for (inIntIndex = 0; inIntIndex < srAll.inIntNum; inIntIndex++)
		{
			if (srAll.srInt[inIntIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}
			
			
			/* 比對Tag Name */
			if (memcmp(szTagName, srAll.srInt[inIntIndex].szTag, strlen(srAll.srInt[inIntIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srInt[inIntIndex].szTag))
			{
				*(int32_t*)srAll.srInt[inIntIndex].pTagValue = sqlite3_column_int(srSQLStat, j);
				srAll.srInt[inIntIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}
		
		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		for (inInt64tIndex = 0; inInt64tIndex < srAll.inInt64tNum; inInt64tIndex++)
		{
			if (srAll.srInt64t[inInt64tIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}

			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll.srInt64t[inInt64tIndex].szTag, strlen(srAll.srInt64t[inInt64tIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srInt64t[inInt64tIndex].szTag))
			{
				*(int64_t*)srAll.srInt64t[inInt64tIndex].pTagValue = sqlite3_column_int64(srSQLStat, j);
				srAll.srInt64t[inInt64tIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}
			
		}

		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}
		
		for (inCharIndex = 0; inCharIndex < srAll.inCharNum; inCharIndex++)
		{
			if (srAll.srChar[inCharIndex].uszIsFind == VS_TRUE)
			{
				continue;
			}
			
			/* 比對Tag Name 所以列恆為0 */
			if (memcmp(szTagName, srAll.srChar[inCharIndex].szTag, strlen(srAll.srChar[inCharIndex].szTag)) == 0	&&
			    strlen(szTagName) == strlen(srAll.srChar[inCharIndex].szTag))
			{
				inDataLen = sqlite3_column_bytes(srSQLStat, j);
				memcpy(srAll.srChar[inCharIndex].pCharVariable, sqlite3_column_blob(srSQLStat, j), inDataLen);
				srAll.srChar[inCharIndex].uszIsFind = VS_TRUE;
				inFind = VS_TRUE;

				break;
			}

		}
		
		/* inFind == VS_TRUE表示找到了，跳下一回 */
		if (inFind == VS_TRUE)
		{
			continue;
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Find no variable to insert:Tag: %s", szTagName);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Value: %s", (char*)sqlite3_column_blob(srSQLStat, j));
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	
	/* 釋放事務 */
	sqlite3_finalize(srSQLStat);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_Ticket_ESVC_Get_TRec_Top()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_ESVC_Delete_TRec_Top_Flow
Date&Time       :2017/3/14 下午 3:57
Describe        :
*/
int inNCCC_Ticket_ESVC_Delete_TRec_Top_Flow(TRANSACTION_OBJECT *pobTran, int inTableType)
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
		case _TN_BATCH_TABLE_ESC_AGAIN_:
		case _TN_BATCH_TABLE_ESC_FAIL_:
		case _TN_EMV_TABLE_:
		case _TN_BATCH_TABLE_TICKET_ADVICE_:
			inRetVal = inNCCC_Ticket_ESVC_Delete_TRec_Top(pobTran, szTableName);
			break;
		default :
			return (VS_ERROR);
			break;
	}
	
	return (inRetVal);
}

/*
Function        :inNCCC_Ticket_ESVC_Delete_TRec_Top
Date&Time       :2018/1/26 下午 3:04
Describe        :adviec刪除table id最小的那一筆
*/
int inNCCC_Ticket_ESVC_Delete_TRec_Top(TRANSACTION_OBJECT *pobTran, char* szTableName)
{
	int		inRetVal;
	char		szDebugMsg[84 + 1];
	char		szQuerySql[300 + 1];	/* INSERT INTO	szTableName( */
	char		*szErrorMessage = NULL;
	sqlite3		*srDBConnection;	/* 建立到資料庫的connection */
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inNCCC_Ticket_ESVC_Delete_TRec_Top()_START");
        }
	
	inRetVal = inSqlite_DB_Open_Or_Create(gszTranDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "DELETE FROM %s WHERE inTableID = (SELECT MIN(inTableID) FROM %s)", szTableName, szTableName);
	
	/* 取得 database 裡所有的資料 */
	inRetVal = sqlite3_exec(srDBConnection , szQuerySql, 0 , 0, &szErrorMessage);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Delete Record ERROR Num:%d, ", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason:%s", szErrorMessage);
			inLogPrintf(AT, szDebugMsg);
			
			inLogPrintf(AT, szQuerySql);
		}
		
		if (szErrorMessage != NULL)
		{
			sqlite3_free(szErrorMessage);
		}
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Delete Record OK");
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_Ticket_ESVC_Delete_TRec_Top()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_ESVC_Get_Batch_ByInvNum
Date&Time       :2018/1/30 下午 6:34
Describe        :利用調閱標號來將該筆資料全塞回pobTran中的TRec、會取最新狀態(如取消、調帳)
*/
int inNCCC_Ticket_ESVC_Get_Batch_ByInvNum(TRANSACTION_OBJECT *pobTran, char* szTableName, int inInvoiceNumber)
{
	int			inRetVal = VS_SUCCESS;
	char			szDebugMsg[128 + 1];
	char			szQuerySql[200 + 1];	/* INSERT INTO	szTableName( */
	char			szErrorMessage[100 + 1];
	sqlite3			*srDBConnection;	/* 建立到資料庫的connection */
	sqlite3_stmt		*srSQLStat;
	SQLITE_ALL_TABLE	srAll;
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inNCCC_Ticket_ESVC_Get_Batch_ByInvNum()_START");
        }
	
	inRetVal = inSqlite_DB_Open_Or_Create(gszTranDBPath, &srDBConnection, _SQLITE_DEFAULT_FLAGS_, NULL);
	
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Open Database File OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
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
		inSqlite_DB_Close(&srDBConnection);
		
		return (VS_ERROR);
	}
	
	/* 塞入SQL語句 */
	
	/* 為了使table name可變動，所以拉出來組 */
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	if (inInvoiceNumber > 0)
	{
		sprintf(szQuerySql, "SELECT * FROM %s WHERE lnInvNum = %d ORDER BY inTableID DESC LIMIT 1", szTableName, inInvoiceNumber);
	}
	else if (inInvoiceNumber == _BATCH_LAST_RECORD_)
	{
		/* sqlite3_last_insert_rowid 只有在同一connection才有用 所以這邊的邏輯是最後一筆理論上invoiceNumber會最大 
		   若是同一筆，可能有調帳等操作，加上用max(inTableID)來判斷 */

		
		/* 再重新組查詢語句，把剛剛查到的invoiceNumber放進去 第一列第0行是所查的值 */
		memset(szQuerySql, 0x00, sizeof(szQuerySql));
		sprintf(szQuerySql, "SELECT * FROM %s WHERE lnInvNum = (SELECT MAX(lnInvNum) FROM %s) ORDER BY inTableID DESC LIMIT 1", szTableName, szTableName);
	}
	else
	{
		
		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}
	
		return (VS_ERROR);
	}
	
	/* prepare語句 */
	inRetVal = sqlite3_prepare_v2(srDBConnection, szQuerySql, -1, &srSQLStat, NULL);
	if (inRetVal != SQLITE_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Prepare Fail: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 取得 database 裡所有的資料 */
	/* Qerry */
	inRetVal = sqlite3_step(srSQLStat);
	if (inRetVal == SQLITE_ROW)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table OK");
			inLogPrintf(AT, szDebugMsg);
		}
		/* 替換資料前先清空srTRec */
		memset(&pobTran->srTRec, 0x00, sizeof(pobTran->srTRec));
	}
	else if (inRetVal == SQLITE_DONE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NO DATA");
			inLogPrintf(AT, szDebugMsg);
		}
		inSqlite_DB_Close(&srDBConnection);
		
		return (VS_NO_RECORD);
	}
	else
	{
		memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
		strcpy(szErrorMessage, sqlite3_errmsg(srDBConnection));
			
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Table ERROR Num:%d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reason: %s", szErrorMessage);
			inLogPrintf(AT, szDebugMsg);

			inLogPrintf(AT, szQuerySql);
		}

		/* 關閉 database, close null pointer 是NOP(No Operation) */
		if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
		{
			/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Close Database OK");
				inLogPrintf(AT, szDebugMsg);
			}
		}

		/* 因為直接get 不存在的table回傳值是-1，只有在Error Msg才能得知錯誤原因 */
		if (memcmp(szErrorMessage, "no such table", strlen("no such table")) == 0)
		{
			return (VS_NO_RECORD);
		}
		else
		{
			return (VS_ERROR);
		}
	}
	
	/* binding 取得值 */
	inSqlite_Get_Binding_Value(&srSQLStat, &srAll);
	
	/* 釋放事務 */
	inSqlite_SQL_Finalize(&srSQLStat);
	
	/* 關閉 database, close null pointer 是NOP(No Operation) */
	if (inSqlite_DB_Close(&srDBConnection) != VS_SUCCESS)
	{
		/* 如果資料還在更新就close會因為SQLITE_BUSY而失敗，而且正在更新的事務也會roll back（回復上一動）*/
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Close Database OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_Ticket_ESVC_Get_Batch_ByInvNum()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_Is_HOST_In_Priority
Date&Time       :2018/5/23 下午 2:42
Describe        :確認是否該票證Host在詢卡順序中
*/
int inNCCC_Ticket_Is_HOST_In_Priority(char* szHost)
{
	int	i = 0;
	char	szTemplate[20 + 1];
	
	if (strlen(szHost) <= 0)
	{
		return (VS_ERROR);
	}
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetESVC_Priority(szTemplate);
	for (i = 0; i < strlen(szTemplate); i++)
	{
		if (szTemplate[i] == szHost[0])
		{
			return (VS_SUCCESS);
		}
	}
	
	return (VS_ERROR);
}

/*
Function        :inNCCC_Ticket_Get_Void_Top_Up_Amount_From_Batch
Date&Time       :2018/7/18 下午 4:32
Describe        :
*/
int inNCCC_Ticket_Get_Void_Top_Up_Amount_From_Batch(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = VS_ERROR;
	int			inECRAmount;
	char			szDEMOMode[2 + 1] = {0};
	TRANSACTION_OBJECT	pobTranTemp;
        
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Ticket_Get_Void_Top_Up_Amount_From_Batch START!");
	
	/* 因為會放到RETRY的OPT所以加入交易別來判定要不要跑 by Russell 2018/12/14 上午 10:03 */
	if (pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_	||
	    pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_	||
	    pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_)
	{
		memset(&pobTranTemp, 0x00, sizeof(pobTranTemp));
		/* 先抓上一筆 */
		pobTranTemp.srBRec.inHDTIndex = pobTran->srBRec.inHDTIndex;
		pobTranTemp.srBRec.lnOrgInvNum = _BATCH_LAST_RECORD_;
		pobTranTemp.srTRec.inTicketType = pobTran->srTRec.inTicketType;
		inRetVal = inNCCC_Ticket_Find_Last_Txn_By_Ticket_Type_SQLite(&pobTranTemp, pobTranTemp.srTRec.inTicketType);
		if (inRetVal != VS_SUCCESS)
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

		/* 抓成功就直接塞金額 */
		/* 這邊流程在_TICKET_DECIDE_TRANS_TYPE_後，所以直接塞」TRec */
		pobTran->srTRec.lnTxnAmount = pobTranTemp.srTRec.lnTxnAmount;
		/* 這邊要多抓AuthCode原加值時回傳的AuthCode */
		memcpy(pobTran->srTRec.szAuthCode, pobTranTemp.srTRec.szAuthCode, strlen(pobTranTemp.srTRec.szAuthCode));
	
		/* 教育訓練版本 */
		memset(szDEMOMode, 0x00, sizeof(szDEMOMode));
		inGetDemoMode(szDEMOMode);
		if (memcmp(szDEMOMode, "Y", strlen("Y")) == 0)
		{
			if (pobTranTemp.srTRec.inCode == _TICKET_IPASS_TOP_UP_ ||
			    pobTranTemp.srTRec.inCode == _TICKET_EASYCARD_TOP_UP_ ||
			    pobTranTemp.srTRec.inCode == _TICKET_ICASH_TOP_UP_)
			{
				/* 繼續流程 */
			}
			else
			{
				/* 前一筆不是加值交易擋下 */
				/* 教育訓練版本規格寫EDC提示 原交易非加值交易 */
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, "");
				srDispMsgObj.inDispPic1YPosition = 0;
				srDispMsgObj.inMsgType = _NO_KEY_MSG_;
				srDispMsgObj.inTimeout = 2;
				strcpy(srDispMsgObj.szErrMsg1, "原交易非加值交易");
				srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
				srDispMsgObj.inBeepTimes = 3;
				srDispMsgObj.inBeepInterval = 1000;

				inDISP_Msg_BMP(&srDispMsgObj);

				return (VS_ERROR);
			}
			
			/* 檢核ECR送的交易金額 */
			if (pobTran->uszECRBit == VS_TRUE)
			{
				/* 在_TICKET_GET_ESVC_CARD_FLOW_已經分各卡別incode了
				   這邊是檢核撈出來上一筆現金加值卡是不是相同卡別 */
				inECRAmount = pobTran->srTRec.lnTxnAmount % 10;
				
				if (pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_ &&
				    (inECRAmount == 0 || inECRAmount == 1 || inECRAmount == 2))
				{
					/* 繼續流程 */
				}
				else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_ &&
				        (inECRAmount == 3 || inECRAmount == 4 || inECRAmount == 5))
				{
					/* 繼續流程 */
				}
				else if (pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_ &&
				        (inECRAmount == 6 || inECRAmount == 7))
				{
					/* 繼續流程 */
				}
				else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_ &&
				        (inECRAmount == 8 || inECRAmount == 9))
				{
					/* 繼續流程 */
				}				
				else
				{
					/* 顯示無交易紀錄 */
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_RECORD_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _NO_KEY_MSG_;
					srDispMsgObj.inTimeout = 2;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 3;
					srDispMsgObj.inBeepInterval = 1000;

					inDISP_Msg_BMP(&srDispMsgObj);

					return (VS_ERROR);
				}
			}
			
			/* 卡別也要存回 */
			pobTran->srTRec.inTicketType = pobTranTemp.srTRec.inTicketType;
		}
		else
		{
			if (pobTranTemp.srTRec.inCode == _TICKET_IPASS_TOP_UP_		||
			    pobTranTemp.srTRec.inCode == _TICKET_EASYCARD_TOP_UP_	||
			    pobTranTemp.srTRec.inCode == _TICKET_ICASH_TOP_UP_)
			{
				
			}
			else
			{
				/* 前一筆不是加值交易擋下 */
				/* 教育訓練版本規格寫EDC提示 原交易非加值交易 */
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, "");
				srDispMsgObj.inDispPic1YPosition = 0;
				srDispMsgObj.inMsgType = _NO_KEY_MSG_;
				srDispMsgObj.inTimeout = 2;
				strcpy(srDispMsgObj.szErrMsg1, "原交易非加值交易");
				srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
				srDispMsgObj.inBeepTimes = 3;
				srDispMsgObj.inBeepInterval = 1000;

				inDISP_Msg_BMP(&srDispMsgObj);

				return (VS_ERROR);
			}
		}
	}
	else
	{
		
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_Display_Transaction_Result
Date&Time       :2018/12/11 下午 7:11
Describe        :因為有需求客製107和111交易完成要嗶聲，避免誤用，這支只使用於107及1111
*/
int inNCCC_Ticket_107_Bumper_Display_Transaction_Result(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_CUST_RECEIPT_;
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned char	uszKey = 0x00;
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 為了恢復被感應燈號清掉的title */
	inFunc_ResetTitle(pobTran);
	
	inNCCC_Ticket_Display_Transaction_Result(pobTran);
			       
	inDISP_Timer_Start(_TIMER_NEXSYS_2_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
	
	/* 客製化107.111移除請按確認鍵跟請按清除鍵 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		/* (需求單 - 107276)自助交易標準做法 EDC於發動交易及完成時，需發出提示音 by Russell 2019/3/8 上午 10:46 */
		inDISP_BEEP(1, 0);
		
		while (1)
		{
			if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
			{
				uszKey = _KEY_TIMEOUT_;
			}

			if (uszKey == _KEY_TIMEOUT_)
				break;
			else
				continue;
		}
	}
	else
	{
		/* 因為查詢不印，所以只顯示確認鍵 */
		 if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ ||
		     pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
		     pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
		 {
			 inTouchSensorFunc = _Touch_BATCH_END_;

			 while (1)
			 {
				 inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
				 uszKey = uszKBD_Key();

				 if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
				 {
					 uszKey = _KEY_TIMEOUT_;
				 }

				 if (uszKey == _KEY_ENTER_			|| 
				     uszKey == _KEY_TIMEOUT_			||
				     inChoice == _BATCH_END_Touch_ENTER_BUTTON_)
					 break;
				 else
					 continue;
			 }
			 /* 清掉觸控殘值 */
			 inDisTouch_Flush_TouchFile();
		 }
		 else
		 {
			 inTouchSensorFunc = _Touch_CUST_RECEIPT_;

			 while (1)
			 {
				 inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
				 uszKey = uszKBD_Key();

				 if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
				 {
					 uszKey = _KEY_TIMEOUT_;
				 }

				 if (uszKey == _KEY_ENTER_			|| 
				     uszKey == _KEY_TIMEOUT_			||
				     inChoice == _CUSTReceipt_Touch_ENTER_)
				 {
					 inRetVal = VS_SUCCESS;
					 break;
				 }
				 else if (uszKey == _KEY_CANCEL_			||
					  inChoice == _CUSTReceipt_Touch_CANCEL_)
				 {
					 inRetVal = VS_USER_CANCEL;
					 break;
				 }
				 else
				 {
					 continue;
				 }
			 }

			 /* 清掉觸控殘值 */
			 inDisTouch_Flush_TouchFile();

			 /* 按取消不印顧客聯 */
			 if (inRetVal == VS_USER_CANCEL)
			 {
				 return (VS_SUCCESS);
			 }
		 }/* 是否為詢卡 */
	}
			       
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_GetESVC_Enable
Date&Time       :2018/12/13 下午 3:28
Describe        :傳HostEnable進來，會得到'Y' OR 'N'
*/
int inNCCC_Ticket_GetESVC_Enable(int inOrgHDTIndex, char *szHostEnable)
{
	/* 此function只用來查詢ESVC是否開，不應該切換Host */
	/* 若之前已查詢到ESVC的index為何，就可以直接使用 */
	if (ginESVCHostIndex != -1)
	{
		/* 直接使用 */
	}
	else
	{
		if (inFunc_Find_Specific_HDTindex(inOrgHDTIndex, _HOST_NAME_ESVC_, &ginESVCHostIndex) != VS_SUCCESS)
		{
			/* 找不到直接return VS_ERROR */
			return (VS_ERROR);
		}
	}
	

	/* 如果Load失敗，回復原Host並回傳VS_ERROR(理論上不會有此情況) */
	if (inLoadHDTRec(ginESVCHostIndex) < 0)
	{
		if (inOrgHDTIndex >= 0)
		{
			/* 回覆原本的Host */
			inLoadHDTRec(inOrgHDTIndex);
		}
		else
		{
			/* load回信用卡主機 */
			inLoadHDTRec(0);
		}

		return (VS_ERROR);
	}
		
	/* 判斷HOST是否開啟  */
	memset(szHostEnable, 0x00, 1);
	inGetHostEnable(szHostEnable);
	if (szHostEnable[0] != 'Y')
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "ESVC Enable not open.");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "ESVC Enable open.");
		}
	}

	if (inOrgHDTIndex >= 0)
	{
		/* 回覆原本的Host */
		inLoadHDTRec(inOrgHDTIndex);
	}
	else
	{
		/* load回信用卡主機 */
		inLoadHDTRec(0);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_Trans_Success_Beep
Date&Time       :2018/12/13 下午 7:22
Describe        :票證感應嗶聲要統一
*/
int inNCCC_Ticket_Trans_Success_Beep(TRANSACTION_OBJECT *pobTran)
{
	char	szFuncEnable[2 + 1] = {0};
	
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetIntegrate_Device(szFuncEnable);
	
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
	{
		if (pobTran->srTRec.inCode != _TICKET_IPASS_QUERY_ && pobTran->srTRec.inCode != _TICKET_IPASS_AUTO_TOP_UP_ && pobTran->uszESVCTxnNotEndBit != VS_TRUE)
		{
			/* 外接設備 */
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{

			}
			else
			{
				inNCCC_Ticket_Set_CTLS_Light(&pobTran->inCTLSLightStatus, _CTLS_LIGHT_GREEN_);
				inDISP_BEEP(2, 200);
			}
		}
	}
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
		/* 外接設備 */
		if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
		{

		}
		else
		{
			inCTLS_Set_LED(_CTLS_LIGHT_GREEN_);
			inDISP_BEEP(2, 200);
		}
	}
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
	{
		/* 外接設備 */
		if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
		{

		}
		else
		{
			inCTLS_Set_LED(_CTLS_LIGHT_GREEN_);
			inDISP_BEEP(2, 200);
		}
	}
	else
	{
		/* 外接設備 */
		if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
		{

		}
		else
		{
			inCTLS_Set_LED(_CTLS_LIGHT_GREEN_);
			inDISP_BEEP(2, 200);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_Get_Card
Date&Time       :2018/1/2 上午 11:49
Describe        :快速詢卡
*/
int inNCCC_Ticket_Get_Card_ECC_RETRY_FLOW(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	char	szFuncEnable[2 + 1];
        
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Ticket_Get_Card_ECC_RETRY_FLOW START!");
	
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetIntegrate_Device(szFuncEnable);

        if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0)
        {
                /* Mirror Message */
                if (pobTran->uszECRBit == VS_TRUE)
                {
                        inECR_SendMirror(pobTran, _MIRROR_MSG_ESVC_TAP_AGAIN_);
                }
            
                /* ESVCPriority [IECH] = IPASS => ECC => ICASH
                        詢卡順位，不開或沒Sign On就忽略       
                */
		memset(pobTran->srTRec.szUID, 0x00, sizeof(pobTran->srTRec.szUID));
		inRetVal = inNCCC_Ticket_Fast_Tap_Wait(pobTran, pobTran->srTRec.szUID);
		if (inRetVal != VS_SUCCESS)
		{
			return (inRetVal);
		}
		
		inLoadTDTRec(_TDT_INDEX_01_ECC_);
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetTicket_LogOnOK(szFuncEnable);
		if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
		{
			inNCCC_tSAM_SelectAID_ECC_Flow();
			inRetVal = inECC_POLL_Flow(pobTran);

			if (inRetVal == VS_SUCCESS)
			{
				pobTran->srTRec.inTicketType = _TICKET_TYPE_ECC_;
				pobTran->srTRec.inTDTIndex = _TDT_INDEX_01_ECC_;
				pobTran->srTRec.uszESVCTransBit = VS_TRUE;

				return (VS_SUCCESS);
			}
		}
        }        
        else
        {
		pobTran->srTRec.inTicketType = _TICKET_TYPE_ECC_;
		pobTran->srTRec.inTDTIndex = _TDT_INDEX_01_ECC_;
		pobTran->srTRec.uszESVCTransBit = VS_TRUE;
						
                return (VS_SUCCESS);
        }
        
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inCTLS_Set_LED(_CTLS_LIGHT_RED_);
	inDISP_ChineseFont("詢卡失敗", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
	inDISP_BEEP(3, 500);
	
	pobTran->srTRec.inTicketType = _TICKET_TYPE_NONE_;
	return (VS_ERROR);
}

/*
Function        :inNCCC_Ticket_Auto_SignOn_Time_Return
Date&Time       :2019/1/24 上午 10:31
Describe        :
*/
int inNCCC_Ticket_Auto_SignOn_Time_Return(RTC_NEXSYS *srRTC)
{
	int		inCheckDigit = 0;
	char		szTID[8 + 1] = {0};
	char		szTemplate[8 + 1] = {0};
	unsigned char	uszExpectHour = 0;
	unsigned char	uszExpectMinute = 0;
	unsigned char	uszExpectSecond = 0;
	
	memset(szTID, 0x00, sizeof(szTID));
	inGetTerminalID(szTID);
	
	/* 時 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &szTID[strlen(szTID) - 1], 1);
	inCheckDigit = atoi(szTemplate) % 4;
	
	switch (inCheckDigit)
	{
		case	0:
			uszExpectHour = 2;
			break;
		case	1:
			uszExpectHour = 3;
			break;
		case	2:
			uszExpectHour = 4;
			break;
		case	3:
			uszExpectHour = 5;
			break;
		default:
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Auto SignOn Time Logic Error");
			}
			return (VS_ERROR);
			break;
	}
	
	/* 分 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &szTID[strlen(szTID) - 3], 2);
	inCheckDigit = atoi(szTemplate);
	/* 若"分鐘"大於59, 自動減59 s*/
	if (inCheckDigit > 59)
	{
		inCheckDigit -= 59;
	}
	uszExpectMinute = inCheckDigit;
	
	/* 秒 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &szTID[strlen(szTID) - 1], 1);
	inCheckDigit = atoi(szTemplate);
	uszExpectSecond = inCheckDigit * 6;
	
	srRTC->uszHour = uszExpectHour;
	srRTC->uszMinute = uszExpectMinute;
	srRTC->uszSecond = uszExpectSecond;
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_Schedule_Auto_SignOn_Time_Check
Date&Time       :2019/1/22 下午 2:25
Describe        :	端末機24小時需重新sign on事宜
		1.	固定每日重新發動sign on，發動時間點依下述TID邏輯辦理，避免因端末機大量同時發動Sign On，對主機及線路都可能會發生系統資源不足
		(1)	用端末機最後一碼mod 4 
		A.	 0, 表示執行時間的"小時"是凌晨2:00
		B.	 1, 表示執行時間的"小時"是凌晨3:00
		C.	 2, 表示執行時間的"小時"是凌晨4:00
		D.	 3, 表示執行時間的"小時"是凌晨5:00
		(2)	用端末機的倒數第2,3碼當做執行的"分鐘", 若"分鐘"大於59, 自動減59
		(3)	端末機最後一碼為0*6 = 0, 00秒執行
			端末機最後一碼為1*6 = 6, 06秒執行
			以此類推，端末機最後一碼為9*6 =54, 54秒執行
		2.	如凌晨重新sign on 3:00有進行交易，則於交易結束後進行sign on
		3.	sign on時比照端末機重新開機時，嘗試3次sign on
		4.	如凌晨3:00 sign on 失敗時按電票hot key提示sign on失敗，比照開機時sign on 失敗顯示。
		5.	於上述遠鑫重新sign on作業後，其他三家票證如有sign on失敗的狀況，接續重新進行sign on，
			如沒有啟用遠鑫則直接檢核其他三家票證是否有sign on失敗，仍依TID邏輯分散於各時間點處理重新sign on作業。
*/
int inNCCC_Ticket_Schedule_Auto_SignOn_Time_Check(void)
{
	int		inRetVal = VS_ERROR;
	int		inCheckNowTime = 0;
	int		inCheckExpectTime = 0;
	int		inCheckGapTime = 0;
	unsigned char	uszExpectHour = 0;
	unsigned char	uszExpectMinute = 0;
	unsigned char	uszExpectSecond = 0;
	RTC_NEXSYS	srRTC = {};
	RTC_NEXSYS	srExpectRTC = {};
	
	memset(&srRTC, 0x00, sizeof(srRTC));
	inFunc_GetSystemDateAndTime(&srRTC);
	
	memset(&srExpectRTC, 0x00, sizeof(srExpectRTC));
	inRetVal = inNCCC_Ticket_Auto_SignOn_Time_Return(&srExpectRTC);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_Ticket_Schedule_Auto_SignOn_Time_Check Error");
		}
		
		return (VS_ERROR);
	}
	uszExpectHour = srExpectRTC.uszHour;
	uszExpectMinute = srExpectRTC.uszMinute;
	uszExpectSecond = srExpectRTC.uszSecond;
	
	inCheckNowTime = srRTC.uszHour * 3600 + srRTC.uszMinute * 60 + srRTC.uszSecond;
	inCheckExpectTime = uszExpectHour * 3600 + uszExpectMinute * 60 + uszExpectSecond;
	inCheckGapTime = inCheckNowTime - inCheckExpectTime;
	/* 30分鐘內皆可以 */
	if (inCheckGapTime >= 0	&&
	    inCheckGapTime < 30 * 60)
	{
		inRetVal = VS_SUCCESS;
	}
	else
	{
		inRetVal = VS_ERROR;
	}
	
	return (inRetVal);
}

/*
Function        :inNCCC_Ticket_Check_ESVC_Need_SignOn
Date&Time       :2019/1/22 下午 4:48
Describe        :Host有開，且有Host未SignOn成功
*/
int inNCCC_Ticket_Check_ESVC_Need_SignOn_Flow(void)
{
	int	inRetVal = VS_ERROR;
	
	/* 只要其中一個條件就執行 */
	inRetVal = inNCCC_Ticket_Check_ESVC_Need_SignOn_By_Host(_TDT_INDEX_00_IPASS_);
	if (inRetVal == VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	inRetVal = inNCCC_Ticket_Check_ESVC_Need_SignOn_By_Host(_TDT_INDEX_01_ECC_);
	if (inRetVal == VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	inRetVal = inNCCC_Ticket_Check_ESVC_Need_SignOn_By_Host(_TDT_INDEX_02_ICASH_);
	if (inRetVal == VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	return (inRetVal);
}

/*
Function        :inNCCC_Ticket_Check_ESVC_Need_SignOn_By_Host
Date&Time       :2019/1/25 下午 2:06
Describe        :Host有開，且有Host未SignOn成功
*/
int inNCCC_Ticket_Check_ESVC_Need_SignOn_By_Host(int inTDTIndex)
{
	int		inRetVal = VS_ERROR;
	char		szHostEnable[2 + 1] = {0};
	char		szLogOnOK[2 + 1] = {0};
	char		szLastAutoSignOnDate[6 + 1] = {0};
	char		szNowDate[6 + 1] = {0};
	RTC_NEXSYS	srRTC = {};
	
	memset(&srRTC, 0x00, sizeof(srRTC));
	inFunc_GetSystemDateAndTime(&srRTC);
	sprintf(szNowDate, "%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
	
	if (inTDTIndex == _TDT_INDEX_00_IPASS_)
	{
		inLoadTDTRec(inTDTIndex);
		memset(szHostEnable, 0x00, sizeof(szHostEnable));
		inGetTicket_HostEnable(szHostEnable);
		if (memcmp(szHostEnable, "Y", strlen("Y")) == 0)
		{
			/* 檢查是否SignOn成功 */
			memset(szLogOnOK, 0x00, sizeof(szLogOnOK));
			inGetTicket_LogOnOK(szLogOnOK);
			if (memcmp(szLogOnOK, "Y", strlen("Y")) == 0)
			{
				/* 不用自動SignOn */
				inRetVal = VS_ERROR;
			}
			else
			{
				/* 檢查當天是否已SignOn過 */
				memset(szLastAutoSignOnDate, 0x00, sizeof(szLastAutoSignOnDate));
				inGetTicket_LastAutoSignOnDate(szLastAutoSignOnDate);
				/* 已SignOn過，不重複SignOn */
				if (memcmp(szLastAutoSignOnDate, szNowDate, strlen(szNowDate)) == 0)
				{
					inRetVal = VS_ERROR;
				}
				/* 要SignOn */
				else
				{
					inRetVal = VS_SUCCESS;
				}
			}
		}
		else
		{
			inRetVal = VS_ERROR;
		}
		
		return (inRetVal);
	}
	else if (inTDTIndex == _TDT_INDEX_01_ECC_)
	{
		inLoadTDTRec(inTDTIndex);
		memset(szHostEnable, 0x00, sizeof(szHostEnable));
		inGetTicket_HostEnable(szHostEnable);
		if (memcmp(szHostEnable, "Y", strlen("Y")) == 0)
		{
			/* 檢查是否SignOn成功 */
			memset(szLogOnOK, 0x00, sizeof(szLogOnOK));
			inGetTicket_LogOnOK(szLogOnOK);
			if (memcmp(szLogOnOK, "Y", strlen("Y")) == 0)
			{
				/* 不用自動SignOn */
				inRetVal = VS_ERROR;
			}
			else
			{
				/* 檢查當天是否已SignOn過 */
				memset(szLastAutoSignOnDate, 0x00, sizeof(szLastAutoSignOnDate));
				inGetTicket_LastAutoSignOnDate(szLastAutoSignOnDate);
				/* 已SignOn過，不重複SignOn */
				if (memcmp(szLastAutoSignOnDate, szNowDate, strlen(szNowDate)) == 0)
				{
					inRetVal = VS_ERROR;
				}
				/* 要SignOn */
				else
				{
					inRetVal = VS_SUCCESS;
				}
			}
		}
		else
		{
			inRetVal = VS_ERROR;
		}
		
		return (inRetVal);
	}
	else if (inTDTIndex == _TDT_INDEX_02_ICASH_)
	{
		inLoadTDTRec(inTDTIndex);
		memset(szHostEnable, 0x00, sizeof(szHostEnable));
		inGetTicket_HostEnable(szHostEnable);
		if (memcmp(szHostEnable, "Y", strlen("Y")) == 0)
		{
			/* 檢查是否SignOn成功 */
			memset(szLogOnOK, 0x00, sizeof(szLogOnOK));
			inGetTicket_LogOnOK(szLogOnOK);
			if (memcmp(szLogOnOK, "Y", strlen("Y")) == 0)
			{
				/* 不用自動SignOn */
				inRetVal = VS_ERROR;
			}
			else
			{
				/* 檢查當天是否已SignOn過 */
				memset(szLastAutoSignOnDate, 0x00, sizeof(szLastAutoSignOnDate));
				inGetTicket_LastAutoSignOnDate(szLastAutoSignOnDate);
				/* 已SignOn過，不重複SignOn */
				if (memcmp(szLastAutoSignOnDate, szNowDate, strlen(szNowDate)) == 0)
				{
					inRetVal = VS_ERROR;
				}
				/* 要SignOn */
				else
				{
					inRetVal = VS_SUCCESS;
				}
			}
		}
		else
		{
			inRetVal = VS_ERROR;
		}
		
		return (inRetVal);
	}
	else
	{
		inRetVal = VS_ERROR;
		
		return (inRetVal);
	}
}

/*
Function        :inNCCC_Ticket_Auto_SignOn_Flow
Date&Time       :2019/1/22 下午 5:49
Describe        :
*/
int inNCCC_Ticket_Auto_SignOn_Flow(TRANSACTION_OBJECT *pobTran)
{
#ifdef	_LOAD_KEY_AP_
	return (VS_SUCCESS);
#else
	int		i = 0;
	int		inFailedCnt = 0;
	int		inRetVal = VS_ERROR;
	int		inChoice = _DisTouch_No_Event_;
	int		inTouchSensorFunc = _Touch_CUP_LOGON_;
	char		szHostName[20 + 1] = {0};
	char		szKey = 0x00;
	char		szDebugMsg[100 + 1] = {0};
	char		szDemoMode[2 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	char		szTemplate[20 + 1] = {0};
	char		szNowDate[6 + 1] = {0};
	unsigned char	uszIPASSBit = VS_FALSE, uszECCBit = VS_FALSE, uszICASHBit = VS_FALSE;
	unsigned char	uszIPASS_DateBit = VS_FALSE, uszECC_DateBit = VS_FALSE, uszICASH_DateBit = VS_FALSE;
	unsigned char	uszReInitBit = VS_FALSE;
	RTC_NEXSYS	srRTC = {};
	
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Ticket_Auto_SignOn_Flow START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_Ticket_Auto_SignOn_Flow() START !");
	}
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		return (VS_SUCCESS);
	}
	else
	{
		/* 電子票證 */
		inDISP_ClearAll();
		inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);
		inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);
		
		memset(&srRTC, 0x00, sizeof(srRTC));
		inFunc_GetSystemDateAndTime(&srRTC);
		sprintf(szNowDate, "%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);

		/* 迴圈開始 */
		while(1)
		{
			/* 原本一次加重試3次 */
			if (inFailedCnt >= 4)
			{
				memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
				inGetCustomIndicator(szCustomerIndicator);
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
                                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					inRetVal = VS_USER_CANCEL;
				}
				else
				{
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

					inDISP_PutGraphic(_CHECK_TICKET_LOGON_, 0, _COORDINATE_Y_LINE_8_4_);
					inDISP_BEEP(1, 0);

					/* 設定Timeout */
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

					/* 電子票證Sign on */
					/* 執行下載請按確認，不執行請按清除 */
					while (1)
					{
						inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
						szKey = uszKBD_Key();

						/* TimeOut */
						if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
						{
							szKey = _KEY_TIMEOUT_;
						}

						if (szKey == _KEY_ENTER_			|| 
						    inChoice == _CUPLogOn_Touch_KEY_1_)
						{
							inRetVal = VS_SUCCESS;
							break;
						}
						else if (szKey == _KEY_CANCEL_			|| 
							 inChoice == _CUPLogOn_Touch_KEY_2_)
						{
							inRetVal = VS_USER_CANCEL;
							break;
						}
						else if (szKey == _KEY_TIMEOUT_)
						{
							inRetVal = VS_TIMEOUT;
							break;
						}

					}
					/* 清空Touch資料 */
					inDisTouch_Flush_TouchFile();
				}

				/* 確認是否重試 */
				if (inRetVal == VS_USER_CANCEL	||
				    inRetVal == VS_TIMEOUT)
				{
					break;
				}
				else if (inRetVal == VS_SUCCESS)
				{
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					/* 繼續LogOn */
				}
			}
			
			for (i = 0; i < _TDT_INDEX_MAX_; i++)
			{	
				if (inLoadTDTRec(i) < 0)
					break;
				
				if (inNCCC_Ticket_Check_ESVC_Need_SignOn_By_Host(i) == VS_SUCCESS)
				{
					memset(szHostName, 0x00, sizeof(szHostName));
					inGetTicket_HostName(szHostName);

					if (memcmp(szHostName, _HOST_NAME_IPASS_, strlen(_HOST_NAME_IPASS_)) == 0)
					{
						if (uszIPASSBit == VS_FALSE)
						{
							uszIPASS_DateBit = VS_TRUE;
							
							inRetVal = inNCCC_tSAM_PowerOn_IPASS_Flow();
							if (inRetVal != VS_SUCCESS)
							{
								uszIPASSBit = VS_FALSE;
								continue;
							}

							inRetVal = inNCCC_tSAM_SelectAID_IPASS_Flow();
							if (inRetVal != VS_SUCCESS)
							{
								uszIPASSBit = VS_FALSE;
								continue;
							}

							inRetVal = inIPASS_Init_Flow(pobTran, uszReInitBit);

							if (inRetVal != VS_SUCCESS)
							{
								uszIPASSBit = VS_FALSE;
								inSetTicket_LogOnOK("N");
							}
							else
							{
								uszIPASSBit = VS_TRUE;
								inSetTicket_LogOnOK("Y");

							}
							inSaveTDTRec(_TDT_INDEX_00_IPASS_);
						}
					}
					else if (memcmp(szHostName, _HOST_NAME_ECC_, strlen(_HOST_NAME_ECC_)) == 0)
					{
						if (uszECCBit == VS_FALSE)
						{
							uszECC_DateBit = VS_TRUE;
							
							inRetVal = inNCCC_tSAM_PowerOn_ECC_Flow();
							if (inRetVal != VS_SUCCESS)
							{
								uszECCBit = VS_FALSE;
								continue;
							}

							inRetVal = inNCCC_tSAM_SelectAID_ECC_Flow();
							if (inRetVal != VS_SUCCESS)
							{
								uszECCBit = VS_FALSE;
								continue;
							}

							inRetVal = inECC_Init_Flow(pobTran);

							if (inRetVal != VS_SUCCESS)
							{
								uszECCBit = VS_FALSE;
								inSetTicket_LogOnOK("N");
							}
							else
							{
								uszECCBit = VS_TRUE;
								inSetTicket_LogOnOK("Y");
							}
							inSaveTDTRec(_TDT_INDEX_01_ECC_);
						}
					}
					else if (memcmp(szHostName, _HOST_NAME_ICASH_, strlen(_HOST_NAME_ICASH_)) == 0)
					{
						if (uszICASHBit == VS_FALSE)
						{
							uszICASH_DateBit = VS_TRUE;
							
							inRetVal = inNCCC_tSAM_PowerOn_ICASH_Flow();
							if (inRetVal != VS_SUCCESS)
							{
								uszICASHBit = VS_FALSE;
								continue;
							}

							inRetVal = inNCCC_tSAM_SelectAID_ICASH_Flow();
							if (inRetVal != VS_SUCCESS)
							{
								uszICASHBit = VS_FALSE;
								continue;
							}

							inRetVal = inICASH_Init_Flow(pobTran);
							
							if (inRetVal != VS_SUCCESS)
							{
								uszICASHBit = VS_FALSE;
								inSetTicket_LogOnOK("N");
							}
							else
							{
								uszICASHBit = VS_TRUE;
								inSetTicket_LogOnOK("Y");
							}
							inSaveTDTRec(_TDT_INDEX_02_ICASH_);
						}
					}
				}
				else
				{
					memset(szHostName, 0x00, sizeof(szHostName));
					inGetTicket_HostName(szHostName);

					if (memcmp(szHostName, _HOST_NAME_IPASS_, strlen(_HOST_NAME_IPASS_)) == 0)
					{
						uszIPASSBit = VS_TRUE;
					}
					else if (memcmp(szHostName, _HOST_NAME_ECC_, strlen(_HOST_NAME_ECC_)) == 0)
					{
						uszECCBit = VS_TRUE;
					}
					else if (memcmp(szHostName, _HOST_NAME_ICASH_, strlen(_HOST_NAME_ICASH_)) == 0)
					{
						uszICASHBit = VS_TRUE;
					}
				}
				         
			}
			
			/* 所有都跑完後，若還有Host未認證過，失敗次數+1，*/
			if (uszIPASSBit == VS_FALSE || uszECCBit == VS_FALSE || uszICASHBit == VS_FALSE)
			{
				if (uszIPASSBit == VS_FALSE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "IPASS Failed");
						inLogPrintf(AT, szDebugMsg);
					}
				}

				if (uszECCBit == VS_FALSE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "ECC Failed");
						inLogPrintf(AT, szDebugMsg);
					}
				}

				if (uszICASHBit == VS_FALSE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "ICASH Failed");
						inLogPrintf(AT, szDebugMsg);
					}
				}

				inFailedCnt++;
			}
			else
				break;          
		}

		if (uszIPASS_DateBit == VS_TRUE)
		{
			inLoadTDTRec(_TDT_INDEX_00_IPASS_);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inNCCC_Ticket_Auto_SignOn_Date_Return(szTemplate);
			inSetTicket_LastAutoSignOnDate(szTemplate);
			inSaveTDTRec(_TDT_INDEX_00_IPASS_);
		}
		
		if (uszECC_DateBit == VS_TRUE)
		{
			inLoadTDTRec(_TDT_INDEX_01_ECC_);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inNCCC_Ticket_Auto_SignOn_Date_Return(szTemplate);
			inSetTicket_LastAutoSignOnDate(szTemplate);
			inSaveTDTRec(_TDT_INDEX_01_ECC_);
		}
		
		if (uszICASH_DateBit == VS_TRUE)
		{
			inLoadTDTRec(_TDT_INDEX_02_ICASH_);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inNCCC_Ticket_Auto_SignOn_Date_Return(szTemplate);
			inSetTicket_LastAutoSignOnDate(szTemplate);
			inSaveTDTRec(_TDT_INDEX_02_ICASH_);
		}

		/* 清空錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		/* 切回 NCCC AID */
		inNCCC_tSAM_SelectAID_NCCC_Flow();
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_Ticket_Auto_SignOn_Flow() END !");
			inLogPrintf(AT, "----------------------------------------");
		}

		return (inRetVal);
	}
#endif
}

/*
Function        :inNCCC_Ticket_AutoSignOn_Date_Return
Date&Time       :2019/1/24 上午 10:22
Describe        :
*/
int inNCCC_Ticket_Auto_SignOn_Date_Return(char* szDate)
{
	char		szToday[6 + 1] = {0};
	RTC_NEXSYS	srRTC = {};
	RTC_NEXSYS	srExpectRTC = {};
	
	memset(&srRTC, 0x00, sizeof(srRTC));
	inFunc_GetSystemDateAndTime(&srRTC);
	
	memset(&srExpectRTC, 0x00, sizeof(srExpectRTC));
	inNCCC_Ticket_Auto_SignOn_Time_Return(&srExpectRTC);
	
	memset(szToday, 0x00, sizeof(szToday));
	sprintf(szToday, "%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
	
	strcpy(szDate, szToday);
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_Top_Up_Amount_Check
Date&Time       :2018/1/15 下午 6:00
Describe        :檢核是否可以自動加值
*/
int inNCCC_Ticket_Top_Up_Amount_Check(TRANSACTION_OBJECT *pobTran)
{
        long    lnAddAmt = 0;
        long    lnAmount, lnSubAmount, lnDeductAmount;

        lnAmount = pobTran->srTRec.lnCardRemainAmount;		/* 餘額 */
        lnSubAmount = pobTran->srTRec.lnTopUpAmount;		/* 自動加值金額 */
        lnDeductAmount = pobTran->srTRec.lnTxnAmount;		/* 扣款金額 */

        while(1)
        {
                lnAmount += lnSubAmount;
                lnAddAmt += lnSubAmount;
		
		/* 防呆 */
		/* 卡內金額不得高於10000 */
                if (lnAmount > 10000)
                        return (VS_ERROR);

                if (lnAmount >= lnDeductAmount)
                {
                        if (lnAddAmt > 1000)
                        {
                                /* 自動加值最多1000 */
                                return (VS_ERROR);
                        }
                        else
                        {
                                pobTran->srTRec.lnTotalTopUpAmount = lnAddAmt;
                                return (VS_SUCCESS);
                        }
                }
        }
}

/*
Function        :inNCCC_Ticket_Set_CTLS_Light
Date&Time       :2019/7/29 下午 4:21
Describe        :設定燈號，避免燈號重複顯示
*/
int inNCCC_Ticket_Set_CTLS_Light(int *inOrgLight, int inNewLight)
{
	int	inRetVal = VS_SUCCESS;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_Ticket_Set_CTLS_Light() START !");
	}
	
	/* 燈號一樣，不顯示 */
	if (*inOrgLight == inNewLight)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "燈號一樣，不改變");
			inLogPrintf(AT, "inNCCC_Ticket_Set_CTLS_Light() END !");
			inLogPrintf(AT, "----------------------------------------");
		}
		
		return (VS_SUCCESS);
	}
	else
	{
		inRetVal = inCTLS_Set_LED(inNewLight);
		if (inRetVal == VS_SUCCESS)
		{
			*inOrgLight = inNewLight;
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Ticket_Set_CTLS_Light() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inNCCC_Ticket_Initial_Light_State
Date&Time       :2019/7/29 下午 4:22
Describe        :顯示藍色的初始化燈號
*/
int inNCCC_Ticket_Initial_Light_State(TRANSACTION_OBJECT *pobTran)
{
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_Ticket_Initial_Light_State() START !");
	}
	
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
	{
		inDISP_ClearAll();
		pobTran->inCTLSLightStatus = _CTLS_LIGHT_NONE_;
		pobTran->inTicketMsgStatus = _TICKET_MSG_NONE_;
		inNCCC_Ticket_Set_CTLS_Light(&pobTran->inCTLSLightStatus, _CTLS_LIGHT_BLUE_);
		inNCCC_Ticket_Set_Msg(pobTran->srTRec.inTicketType, &pobTran->inTicketMsgStatus, _TICKET_MSG_IPASS_PUT_CARD_IN_AREA_);
	}
	else
	{
		inDISP_ClearAll();
		pobTran->inCTLSLightStatus = _CTLS_LIGHT_NONE_;
		inNCCC_Ticket_Set_CTLS_Light(&pobTran->inCTLSLightStatus, _CTLS_LIGHT_BLUE_);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Ticket_Initial_Light_State() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}


/*
Function        :inNCCC_Ticket_Set_Msg
Date&Time       :2019/7/29 下午 5:18
Describe        :設定訊息並紀錄，避免大量刷訊息
*/
int inNCCC_Ticket_Set_Msg(int inTicketType, int *inOrgMsg, int inNewMsg)
{
	int	inRetVal = VS_SUCCESS;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_Ticket_Set_Msg() START !");
	}
	
	/* 燈號一樣，不顯示 */
	if (*inOrgMsg == inNewMsg)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "訊息一樣，不改變");
			inLogPrintf(AT, "inNCCC_Ticket_Set_Msg() END !");
			inLogPrintf(AT, "----------------------------------------");
		}
		
		return (VS_SUCCESS);
	}
	else
	{
		if (inTicketType == _TICKET_TYPE_IPASS_)
		{
			inRetVal = inNCCC_Ticket_Display_Fixed_Msg(inTicketType, inNewMsg);
			if (inRetVal == VS_SUCCESS)
			{
				*inOrgMsg = inNewMsg;
			}
		}
		else if (inTicketType == _TICKET_TYPE_ICASH_)
		{
			inRetVal = inNCCC_Ticket_Display_Fixed_Msg(inTicketType, inNewMsg);
			if (inRetVal == VS_SUCCESS)
			{
				*inOrgMsg = inNewMsg;
			}
		}
		else
		{
			
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Ticket_Set_Msg() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inNCCC_Ticket_Display_Fixed_Msg
Date&Time       :2019/7/29 下午 5:18
Describe        :設定訊息並紀錄，避免大量刷訊息
*/
int inNCCC_Ticket_Display_Fixed_Msg(int inTicketType, int inMsg)
{
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_Ticket_Display_Fixed_Msg() START !");
	}
	
	if (inTicketType == _TICKET_TYPE_IPASS_)
	{
		if (inMsg == _TICKET_MSG_IPASS_PUT_CARD_IN_AREA_)
		{
			inDISP_Clear_Line(_LINE_8_5_, _LINE_8_6_);
			inDISP_ChineseFont("請將票卡放置", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
			inDISP_ChineseFont("感應區", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
		}
		else if (inMsg == _TICKET_MSG_IPASS_AUTO_TOP_UP_)
		{
			inDISP_Clear_Line(_LINE_8_5_, _LINE_8_6_);
			inDISP_ChineseFont("餘額不足", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
			inDISP_ChineseFont("進行自動加值", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
		}
		else if (inMsg == _TICKET_MSG_IPASS_DO_NOT_MOVE_CARD_)
		{
			inDISP_Clear_Line(_LINE_8_5_, _LINE_8_6_);
			inDISP_ChineseFont("交易進行中", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
			inDISP_ChineseFont("請勿移動票卡", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
		}
	}
	else if (inTicketType == _TICKET_TYPE_ICASH_)
	{
		if (inMsg == _TICKET_MSG_ICASH_PUT_CARD_IN_AREA_)
		{
			inDISP_Clear_Line(_LINE_8_5_, _LINE_8_6_);
			inDISP_ChineseFont("請將票卡放置", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
			inDISP_ChineseFont("感應區", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
		}
		else if (inMsg == _TICKET_MSG_ICASH_AUTO_TOP_UP_)
		{
			inDISP_Clear_Line(_LINE_8_5_, _LINE_8_6_);
			inDISP_ChineseFont("餘額不足", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
			inDISP_ChineseFont("進行自動加值", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
		}
		else if (inMsg == _TICKET_MSG_ICASH_DO_NOT_MOVE_CARD_)
		{
			inDISP_Clear_Line(_LINE_8_5_, _LINE_8_6_);
			inDISP_ChineseFont("交易進行中", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
			inDISP_ChineseFont("請勿移動票卡", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Ticket_Display_Fixed_Msg() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_Func_Must_SETTLE
Date&Time       :2019/8/23 下午 1:46
Describe        :確認是否要先結帳
*/
int inNCCC_Ticket_Func_Must_SETTLE(TRANSACTION_OBJECT *pobTran)
{
	int	inTicket_HostIndex;
        char    szMustSettleBit[2 + 1] = {0};
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	if (inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_ESVC_, &inTicket_HostIndex) != VS_SUCCESS)
	{
		/* 找不到直接return VS_ERROR */
		return (VS_ERROR);
	}
        inLoadHDPTRec(inTicket_HostIndex);
        memset(szMustSettleBit, 0x00, sizeof(szMustSettleBit));

        if (inGetMustSettleBit(szMustSettleBit) == VS_ERROR)
        {
                return (VS_ERROR);
        }

	/* Load完了要load回原來的HDPT避免意外，如果有的話 */
	inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
	
        if (!memcmp(szMustSettleBit, "Y", 1))
        {
                /* 表示要結帳 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_MUST_SETTLE_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, "電子票證");
		srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
                inDISP_Msg_BMP(&srDispMsgObj);
		
		return (VS_ERROR);
	}
        else
	{
                return (VS_SUCCESS);
	}
}

/*
Function        :inNCCC_Ticket_Func_Must_SETTLE_Show_In_Menu
Date&Time       :2019/8/29 下午 2:31
Describe        :比照520顯示是否要先結帳
*/
int inNCCC_Ticket_Func_Must_SETTLE_Show_In_Menu(void)
{
	int	inTicket_HostIndex = -1;
        char    szMustSettleBit[2 + 1] = {0};
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	if (inFunc_Find_Specific_HDTindex(0, _HOST_NAME_ESVC_, &inTicket_HostIndex) != VS_SUCCESS)
	{
		/* 找不到直接return VS_ERROR */
		return (VS_ERROR);
	}
        inLoadHDPTRec(inTicket_HostIndex);
        memset(szMustSettleBit, 0x00, sizeof(szMustSettleBit));

        if (inGetMustSettleBit(szMustSettleBit) == VS_ERROR)
        {
                return (VS_ERROR);
        }

	/* Load完了要load回原來的HDPT避免意外，如果有的話 */
	inLoadHDPTRec(0);
	
        if (!memcmp(szMustSettleBit, "Y", 1))
        {
                /* 表示要結帳 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_MUST_SETTLE_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = 1;
		strcpy(srDispMsgObj.szErrMsg1, "電子票證");
		srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
                inDISP_Msg_BMP(&srDispMsgObj);
		
		return (VS_ERROR);
	}
        else
	{
                return (VS_SUCCESS);
	}
}

/*
Function        :inNCCC_Ticket_Demo_Type
Date&Time       :2019/10/2 上午 10:54
Describe        :教育訓練模式用金額分要跑哪個票證
*/
int inNCCC_Ticket_Demo_Type(TRANSACTION_OBJECT * pobTran, int inUnitDigit)
{
        int			i, inCnt = 0;
	int			inRetVal = VS_SUCCESS;
	char			szFuncEnable[2 + 1];
        VS_BOOL			fT1 = VS_FALSE, fT2 = VS_FALSE, fT3 = VS_FALSE, fT4 = VS_FALSE;
	TRANSACTION_OBJECT	pobTranTemp;

        for (i = 0; i < _TDT_INDEX_MAX_; i++)
        {
		inLoadTDTRec(i);
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetTicket_LogOnOK(szFuncEnable);
		
		if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
		{
                        if (i == 0)
                                fT1 = VS_TRUE;

                        if (i == 1)
                                fT2 = VS_TRUE;

                        if (i == 2)
                                fT3 = VS_TRUE;

                        if (i == 3)
                                fT4 = VS_TRUE;

                        inCnt ++;
                }
        }

        /* 單開 */
        if (inCnt == 1)
        {
                if (fT2 == VS_TRUE)
                        return (_TICKET_TYPE_ECC_);
                else if (fT3 == VS_TRUE)
                        return (_TICKET_TYPE_ICASH_);
                else if (fT4 == VS_TRUE)
                        return (_TICKET_TYPE_ECC_);
                else
                        return (_TICKET_TYPE_IPASS_);
        }

	if (pobTran->srTRec.inCode == _TICKET_VOID_TOP_UP_)
	{
		if (pobTran->uszECRBit == VS_TRUE)
		{
			if (inUnitDigit == 3 || inUnitDigit == 4 || inUnitDigit == 5)
				return (_TICKET_TYPE_ECC_);
			else if (inUnitDigit == 6 || inUnitDigit == 7)
				return (_TICKET_TYPE_ICASH_);
			else if (inUnitDigit == 8 || inUnitDigit == 9)
				return (_TICKET_TYPE_ECC_);
			else
				return (_TICKET_TYPE_IPASS_);
		}
		else
		{
			memset(&pobTranTemp, 0x00, sizeof(pobTranTemp));
			/* 先抓上一筆 */
			pobTranTemp.srBRec.inHDTIndex = pobTran->srBRec.inHDTIndex;
			pobTranTemp.srBRec.lnOrgInvNum = _BATCH_LAST_RECORD_;
			inRetVal = inNCCC_Ticket_Find_Last_Topup_CardType_SQLite(&pobTranTemp);
			if (inRetVal != VS_SUCCESS)
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

				return (_TICKET_TYPE_IPASS_);
			}
			else
			{
				return (pobTranTemp.srTRec.inTicketType);
			}
		}
	}
	else
	{
		if (inUnitDigit == 3 || inUnitDigit == 4 || inUnitDigit == 5)
			return (_TICKET_TYPE_ECC_);
		else if (inUnitDigit == 6 || inUnitDigit == 7)
			return (_TICKET_TYPE_ICASH_);
		else if (inUnitDigit == 8 || inUnitDigit == 9)
			return (_TICKET_TYPE_ECC_);
		else
			return (_TICKET_TYPE_IPASS_);
	}
}

/*
Function        :inNCCC_Ticket_ICASH_Func_Check_Transaction_Function
Date&Time       :2019/10/2 下午 2:53
Describe        :確認ICASH交易功能是否打開，
*/
int inNCCC_Ticket_ICASH_Func_Check_Transaction_Function(int inCode)
{
	char		szTransFunc[20 + 1];
	unsigned char	uszTxnEnable = VS_TRUE;
	
	/* 先load ICASH 參數檔 */
	inLoadTDTRec(_TDT_INDEX_02_ICASH_);	

	memset(szTransFunc, 0x00, sizeof(szTransFunc));
	if (inGetTicket_HostTransFunc(szTransFunc) != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 扣款 */
	if (inCode == _TICKET_ICASH_DEDUCT_)
	{
		if (memcmp(&szTransFunc[0], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _TICKET_ICASH_VOID_DEDUCT_)
	{
		if (memcmp(&szTransFunc[1], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _TICKET_ICASH_REFUND_)
	{
		if (memcmp(&szTransFunc[2], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _TICKET_ICASH_AUTO_TOP_UP_)
	{
		if (memcmp(&szTransFunc[3], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _TICKET_ICASH_TOP_UP_)
	{
		if (memcmp(&szTransFunc[4], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _TICKET_ICASH_VOID_TOP_UP_)
	{
		if (memcmp(&szTransFunc[5], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _TICKET_ICASH_INQUIRY_)
	{
		if (memcmp(&szTransFunc[7], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _SETTLE_)
	{
		if (memcmp(&szTransFunc[8], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	
	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function	:inNCCC_Ticket_Find_Last_Txn_By_Ticket_Type_SQLite
Date&Time	:2019/12/4 下午 5:10
Describe        :傳進指定票證，找這個票證最後一筆交易
*/
int inNCCC_Ticket_Find_Last_Txn_By_Ticket_Type_SQLite(TRANSACTION_OBJECT *pobTran, int inTicketType)
{
	int			inRetVal = VS_SUCCESS;
	char			szQuerySql[200 + 1] = {0};
	char			szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	SQLITE_ALL_TABLE	srAll;
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);

	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inNCCC_Ticket_Table_Link_TRec(pobTran, &srAll, _LS_READ_);
	
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "SELECT * FROM %s WHERE inTicketType = %d ORDER BY inTableID DESC LIMIT 1", szTableName, inTicketType);
	
	inRetVal = inSqlite_Get_Data_By_External_SQL(gszTranDBPath, &srAll, szQuerySql);
	
	if (inRetVal == VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}

/*
Function	:inNCCC_Ticket_Get_API_Version
Date&Time	:2020/3/4 上午 11:37
Describe        :畫面顯示各票證API版本
*/
int inNCCC_Ticket_Get_API_Version(void)
{
	int		i;
	int		inChoice = _DisTouch_No_Event_;
	int		inRetVal;
	char		szDisplay[60 + 1];
	char		szTicket_API_Version[20 + 1];
	unsigned char   uszKey = 0x00;
	
	inDISP_ClearAll();
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode("  電子票證 API Version ", _FONTSIZE_8X22_, _COLOR_WHITE_, _COLOR_BLACK_, 0, _COORDINATE_Y_LINE_16_1_, VS_FALSE);
	
	for (i = 0; i < _TDT_INDEX_MAX_; i++)
	{
		inLoadTDTRec(i);
		memset(szDisplay, 0x00, sizeof(szDisplay));
		memset(szTicket_API_Version, 0x00, sizeof(szTicket_API_Version));
		inGetTicket_API_Version(szTicket_API_Version);
		
		if (i == _TDT_INDEX_00_IPASS_)
		{
			/* 一卡通 */
			sprintf(szDisplay, "IPASS: %s", szTicket_API_Version);
			inDISP_ChineseFont(szDisplay, _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);			
		}
		else if (i == _TDT_INDEX_01_ECC_)
		{
			/* 悠遊卡 */
			sprintf(szDisplay, "ECC: %s", szTicket_API_Version);
			inDISP_ChineseFont(szDisplay, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);			
			
		}
		else if (i == _TDT_INDEX_02_ICASH_)
		{
			/* 愛金卡 */
			sprintf(szDisplay, "ICASH: %s", szTicket_API_Version);
			inDISP_ChineseFont(szDisplay, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);			
		}
	}

	inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);
	inDISP_BEEP(1, 0);

	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(_Touch_BATCH_END_);
		uszKey = uszKBD_Key();
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}

		if (uszKey == _KEY_ENTER_	||
		    inChoice == _BATCH_END_Touch_ENTER_BUTTON_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
		else
		{
			continue;
		}
	}

	inDisTouch_Flush_TouchFile();
		
	return (VS_SUCCESS);
}

/*
Function	:inNCCC_Ticket_Disp_Receipt
Date&Time	:2020/6/1 下午 2:43
Describe        :
*/
int inNCCC_Ticket_Disp_Receipt(TRANSACTION_OBJECT* pobTran)
{
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
	{
		inNCCC_Ticket_Disp_Receipt_IPASS(pobTran);
	}
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
		inNCCC_Ticket_Disp_Receipt_ECC(pobTran);
	}
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
	{
		inNCCC_Ticket_Disp_Receipt_ICASH(pobTran);
	}
	
	return (VS_SUCCESS);
}

/*
Function	:inNCCC_Ticket_Disp_Receipt_ECC
Date&Time	:2020/6/1 下午 2:43
Describe        :
*/
int inNCCC_Ticket_Disp_Receipt_ECC(TRANSACTION_OBJECT* pobTran)
{
	int	inLen = 0;
	char	szTemplate[50 + 1] = {0};
	char	szDispMsg[50 + 1] = {0};
	char	szTicketNeedNewBatch[2 + 1] = {0};
        char    szCustomerIndicator[3 + 1] = {0};
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	inDISP_ClearAll();
	inDISP_ChineseFont("交易完成", _FONTSIZE_8X16_, _LINE_16_1_, _DISP_CENTER_);
	inDISP_ChineseFont("卡號", _FONTSIZE_16X22_, _LINE_16_3_, _DISP_LEFT_);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode("卡別", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_RECEPT_CENTER_, _COORDINATE_Y_LINE_16_3_, VS_FALSE);

	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	strcpy(szDispMsg, "悠遊卡");
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_3_, _DISP_RIGHT_);

	/* 卡號值 & 批號 */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	inLen = strlen(pobTran->srTRec.szUID);
	memcpy(szDispMsg, pobTran->srTRec.szUID, inLen);
	
        /* 客製化098，悠遊卡因為使用T0200，加密遮掩為倒數5、6、7 */
        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))
        {
                szDispMsg[inLen - 5] = 0x2A;
                szDispMsg[inLen - 6] = 0x2A;
                szDispMsg[inLen - 7] = 0x2A;
        }
        else
        {
                if (atoi(pobTran->srTRec.srECCRec.szPurseVersionNumber) == 0)
                {
                        szDispMsg[inLen - 5] = 0x2A;
                        szDispMsg[inLen - 6] = 0x2A;
                        szDispMsg[inLen - 7] = 0x2A;
                }
                else
                {
                        /* 全部16 第6-11隱碼 */
                        szDispMsg[inLen - 6] = 0x2A;
                        szDispMsg[inLen - 7] = 0x2A;
                        szDispMsg[inLen - 8] = 0x2A;
                        szDispMsg[inLen - 9] = 0x2A;
                        szDispMsg[inLen - 10] = 0x2A;
                        szDispMsg[inLen - 11] = 0x2A;
                }
        }    
	strcat(szDispMsg, "(W)");
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X44_, _LINE_16_4_, _DISP_LEFT_);

	/* 交易別 & 特店代號 */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	if (pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_)
		strcpy(szTemplate, "購貨　　");
	else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_REFUND_)
		strcpy(szTemplate, "退貨　　");
	else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_)
		strcpy(szTemplate, "現金加值");  
	else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_)
		strcpy(szTemplate, "加值取消");              
	else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_)
		strcpy(szTemplate, "餘額查詢");  
	else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_AUTO_TOP_UP_)
		strcpy(szTemplate, "自動加值");         
	else
		strcpy(szTemplate, "　　");
	
	sprintf(szDispMsg, szTemplate);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szDispMsg, _FONTSIZE_16X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_RECEPT_LEFT_MIDDLE_, _COORDINATE_Y_LINE_16_5_, VS_FALSE);
	
	sprintf(szDispMsg, "交易");
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X44_, _LINE_16_5_, _DISP_LEFT_);

	/* 特店代號 */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "特店代號");
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szDispMsg, _FONTSIZE_16X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_RECEPT_CENTER_, _COORDINATE_Y_LINE_16_5_, VS_FALSE);

	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetECC_New_SP_ID(szTemplate);
	memcpy(&szDispMsg[0], szTemplate, 8);
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X44_, _LINE_16_5_, _DISP_RIGHT_);
	
	/* 二代設備編號 */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "二代設備編號");
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X44_, _LINE_16_6_, _DISP_LEFT_);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_Device2(szTemplate);
	sprintf(szDispMsg, szTemplate);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szDispMsg, _FONTSIZE_16X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_RECEPT_CENTER_, _COORDINATE_Y_LINE_16_6_, VS_FALSE);
	
	/* 悠遊卡批次號碼 */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "悠遊卡批次號碼");
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X44_, _LINE_16_7_, _DISP_LEFT_);

	memset(szTicketNeedNewBatch, 0x00, sizeof(szTicketNeedNewBatch));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_NeedNewBatch(szTicketNeedNewBatch);
	if (memcmp(szTicketNeedNewBatch, "Y", strlen("Y")) == 0)
	{
		sprintf(szTemplate, "%.4s%.2s%.2s", &pobTran->srBRec.szDate[2], &pobTran->srBRec.szDate[6], _ECC_DEFAULT_BATCH_);
	}
	else
	{
		inGetTicket_Batch(szTemplate);
	}
	sprintf(szDispMsg, szTemplate);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szDispMsg, _FONTSIZE_16X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_RECEPT_CENTER_, _COORDINATE_Y_LINE_16_7_, VS_FALSE);

	/* RRN */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "RRN");
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X44_, _LINE_16_8_, _DISP_LEFT_);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	strcpy(szTemplate, pobTran->srTRec.srECCRec.szRRN);
	sprintf(szDispMsg, szTemplate);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szDispMsg, _FONTSIZE_16X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_RECEPT_CENTER_, _COORDINATE_Y_LINE_16_8_, VS_FALSE);
	
	if (pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_ && pobTran->srTRec.lnTotalTopUpAmount > 0L)
        {
                /* 自動加值金額 */
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "自動加值");
		inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		
                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                sprintf(szDispMsg, "%ld", pobTran->srTRec.lnTotalTopUpAmount);
                inFunc_Amount_Comma(szDispMsg, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
                inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_RIGHT_);
        }
        
	/* 交易前餘額 */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "交易前餘額 :");
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
	
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "%ld", pobTran->srTRec.lnFinalBeforeAmt);
	inFunc_Amount_Comma(szDispMsg, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
        inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_6_, _DISP_RIGHT_);

	/* 交易金額 */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "交易金額　 :");
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
	
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "%ld", pobTran->srTRec.lnTxnAmount);
	inFunc_Amount_Comma(szDispMsg, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
        inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_7_, _DISP_RIGHT_);

	/* 交易後餘額 */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "交易後餘額 :");
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "%ld", pobTran->srTRec.lnFinalAfterAmt);
	inFunc_Amount_Comma(szDispMsg, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_8_, _DISP_RIGHT_);
	
	return (VS_SUCCESS);
}

/*
Function	:inNCCC_Ticket_Disp_Receipt_IPASS
Date&Time	:2020/6/1 下午 2:43
Describe        :
*/
int inNCCC_Ticket_Disp_Receipt_IPASS(TRANSACTION_OBJECT* pobTran)
{
	int	inLen = 0;
	char	szTemplate[50 + 1] = {0};
	char	szDispMsg[50 + 1] = {0};
	
	inDISP_ClearAll();
	inDISP_ChineseFont("交易完成", _FONTSIZE_8X16_, _LINE_16_1_, _DISP_CENTER_);
	inDISP_ChineseFont("卡號", _FONTSIZE_16X22_, _LINE_16_3_, _DISP_LEFT_);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode("卡別", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_RECEPT_CENTER_, _COORDINATE_Y_LINE_16_3_, VS_FALSE);

	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	strcpy(szDispMsg, "一卡通");
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_3_, _DISP_RIGHT_);

	/* 卡號值 & 批號 */
	/* 一卡通：遮掩後2 碼，遮掩字元為”*” */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	inLen = strlen(pobTran->srTRec.szUID);
	memcpy(szDispMsg, pobTran->srTRec.szUID, inLen);
	szDispMsg[inLen - 1] = 0x2A;
	szDispMsg[inLen - 2] = 0x2A;
	strcat(szDispMsg, "(W)");
	/*	1.簽單卡號列印邏輯
		<T4800>票卡版號(Purse Version Number)=0，<T0200>列印晶片卡號(Card Id)
		<T0200>20 bytes，14th,15th,16th隱碼，17th,18th,19th,20th顯示
		<T4800>票卡版號(Purse Version Number)<>0，<T0211>列印外觀卡號(Purse Id)
		<T0211>16 bytes，6th,7th,8th,9th,10th,11th隱碼，12th,13th,14th,15th,16th顯示
		，遮掩字元為”*”
		以<T0200>悠遊卡卡號遮掩從後往前算第五碼，第六碼，第七碼遮掩
		範例:以8碼卡號為例，卡號11651733，1,6,5要遮掩，實際列印為1***1733
		1th 2th 3th 4th 5th 6th 7th 8th 9th 10th 11th 12th 13th 14th 15th 16th 17th
		0 0 0 0 0 0 0 0 0 1 1 6 5 1 7 3 3
		0 0 0 0 0 0 0 0 0 1 * * * 1 7 3 3
		範例:以10碼卡號為例，卡號6611651733，1,6,5要遮掩，實際列印為661***1733
		1th 2th 3th 4th 5th 6th 7th 8th 9th 10th 11th 12th 13th 14th 15th 16th 17th
		0 0 0 0 0 0 0 6 6 1 1 6 5 1 7 3 3
		0 0 0 0 0 0 0 6 6 1 * * * 1 7 3 3

		2. 電子發票載具卡號為T0200 (不遮掩)
	*/
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_4_, _DISP_LEFT_);

	/* 交易別 & 特店代號 */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	if (pobTran->srTRec.inCode == _TICKET_IPASS_DEDUCT_)
		strcpy(szTemplate, "購貨　　");
	else if (pobTran->srTRec.inCode == _TICKET_IPASS_REFUND_)
		strcpy(szTemplate, "退貨　　");
	else if (pobTran->srTRec.inCode == _TICKET_IPASS_TOP_UP_)
		strcpy(szTemplate, "現金加值");  
	else if (pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_)
		strcpy(szTemplate, "加值取消");              
	else if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_)
		strcpy(szTemplate, "餘額查詢");  
	else if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_)
		strcpy(szTemplate, "自動加值");         
	else
		strcpy(szTemplate, "　　");

	sprintf(szDispMsg, szTemplate);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szDispMsg, _FONTSIZE_16X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_RECEPT_LEFT_MIDDLE_, _COORDINATE_Y_LINE_16_5_, VS_FALSE);
	
	sprintf(szDispMsg, "交易");
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X44_, _LINE_16_5_, _DISP_LEFT_);
	
	/* 特店代號 */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "特店代號");
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szDispMsg, _FONTSIZE_16X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_RECEPT_CENTER_, _COORDINATE_Y_LINE_16_5_, VS_FALSE);

	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetIPASS_System_ID(szTemplate);
	memcpy(&szDispMsg[0], szTemplate, 2);

	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetIPASS_SP_ID(szTemplate);
	memcpy(&szDispMsg[2], szTemplate, 2);

	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetIPASS_Sub_Company_ID(szTemplate);
	memcpy(&szDispMsg[4], szTemplate, 4);
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X44_, _LINE_16_5_, _DISP_RIGHT_);
		
	if (pobTran->srTRec.inCode == _TICKET_IPASS_DEDUCT_ && pobTran->srTRec.lnTotalTopUpAmount > 0L)
	{       
		/* 自動加值金額 */
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "自動加值");
		inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);

		memset(szDispMsg, 0x00, sizeof(szDispMsg)); 
		sprintf(szDispMsg, "%ld", pobTran->srTRec.lnTotalTopUpAmount);
		inFunc_Amount_Comma(szDispMsg, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
		inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_RIGHT_);
	}

	/* 交易前餘額 */
	/* 交易前餘額 */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "交易前餘額 :");
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);

	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	if (pobTran->srTRec.lnTotalTopUpAmount > 0L)
	{       
		if (pobTran->srTRec.lnFinalBeforeAmt > 100000)
		{
			sprintf(szDispMsg, "%ld", pobTran->srTRec.lnTotalTopUpAmount + (pobTran->srTRec.lnFinalBeforeAmt - 100000));
		}
		else
		{
			sprintf(szDispMsg, "%ld", pobTran->srTRec.lnFinalBeforeAmt - pobTran->srTRec.lnTotalTopUpAmount);
		}
	}
	else      
	{      
		if (pobTran->srTRec.lnFinalBeforeAmt > 100000)
		{
			sprintf(szDispMsg, "%ld", pobTran->srTRec.lnFinalBeforeAmt - 100000);
		}
		else
		{
			sprintf(szDispMsg, "%ld", pobTran->srTRec.lnFinalBeforeAmt);
		}
	}

	if (pobTran->srTRec.lnFinalBeforeAmt > 100000)
	{
		inFunc_Amount_Comma(szDispMsg, "NT$ ", '\x00', _SIGNED_MINUS_, 10, _PADDING_RIGHT_);
	}
	else
	{
		inFunc_Amount_Comma(szDispMsg, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
	}
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_RIGHT_);

	/* 交易金額 */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "交易金額　 :");
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);

	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "%ld", pobTran->srTRec.lnTxnAmount);
	inFunc_Amount_Comma(szDispMsg, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_6_, _DISP_RIGHT_);

	/* 交易後餘額 */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "交易後餘額 :");
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);

	memset(szDispMsg, 0x00, sizeof(szDispMsg));

	if (pobTran->srTRec.lnFinalAfterAmt > 100000)
	{     
		sprintf(szDispMsg, "%ld", pobTran->srTRec.lnFinalAfterAmt - 100000);   
		inFunc_Amount_Comma(szDispMsg, "NT$ ", '\x00', _SIGNED_MINUS_, 10, _PADDING_RIGHT_);
	}
	else
	{      
		sprintf(szDispMsg, "%ld", pobTran->srTRec.lnFinalAfterAmt);     
		inFunc_Amount_Comma(szDispMsg, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
	}
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_7_, _DISP_RIGHT_);
	
	return (VS_SUCCESS);
}

/*
Function	:inNCCC_Ticket_Disp_Receipt_ICASH
Date&Time	:2020/6/1 下午 2:43
Describe        :
*/
int inNCCC_Ticket_Disp_Receipt_ICASH(TRANSACTION_OBJECT* pobTran)
{
	int	inLen = 0;
	char	szTemplate[50 + 1] = {0};
	char	szDispMsg[50 + 1] = {0};
	
	inDISP_ClearAll();
	inDISP_ChineseFont("交易完成", _FONTSIZE_8X16_, _LINE_16_1_, _DISP_CENTER_);
	inDISP_ChineseFont("卡號", _FONTSIZE_16X22_, _LINE_16_3_, _DISP_LEFT_);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode("卡別", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_RECEPT_CENTER_, _COORDINATE_Y_LINE_16_3_, VS_FALSE);

	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	strcpy(szDispMsg, "愛金卡");
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_3_, _DISP_RIGHT_);

	/* 卡號值 & 批號 */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	inLen = strlen(pobTran->srTRec.szUID);
	memcpy(szDispMsg, pobTran->srTRec.szUID, inLen);
	/* 愛金卡：遮掩9-12 碼，遮掩字元為"*" */
	szDispMsg[8] = 0x2A;
	szDispMsg[9] = 0x2A;
	szDispMsg[10] = 0x2A;
	szDispMsg[11] = 0x2A;
	strcat(szDispMsg, "(W)");
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_4_, _DISP_LEFT_);

	/* 交易別 & 特店代號 */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	if (pobTran->srTRec.inCode == _TICKET_ICASH_DEDUCT_)
		strcpy(szTemplate, "購貨　　");
	else if (pobTran->srTRec.inCode == _TICKET_ICASH_REFUND_)
		strcpy(szTemplate, "退貨　　");
	else if (pobTran->srTRec.inCode == _TICKET_ICASH_TOP_UP_)
		strcpy(szTemplate, "現金加值");  
	else if (pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_)
		strcpy(szTemplate, "加值取消");              
	else if (pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
		strcpy(szTemplate, "餘額查詢");  
	else if (pobTran->srTRec.inCode == _TICKET_ICASH_AUTO_TOP_UP_)
		strcpy(szTemplate, "自動加值");         
	else
		strcpy(szTemplate, "　　");
	
	sprintf(szDispMsg, szTemplate);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szDispMsg, _FONTSIZE_16X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_RECEPT_LEFT_MIDDLE_, _COORDINATE_Y_LINE_16_5_, VS_FALSE);
	
	sprintf(szDispMsg, "交易");
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X44_, _LINE_16_5_, _DISP_LEFT_);

	/* 門市代號 */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "門市代號");
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szDispMsg, _FONTSIZE_16X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_RECEPT_CENTER_, _COORDINATE_Y_LINE_16_5_, VS_FALSE);

	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetICASH_Shop_ID(szTemplate);
	memcpy(&szDispMsg[0], szTemplate, 8);
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X44_, _LINE_16_5_, _DISP_RIGHT_);
	
	if (pobTran->srTRec.inCode == _TICKET_ICASH_DEDUCT_ && pobTran->srTRec.lnTotalTopUpAmount > 0L)
        {
		/* 自動加值金額 */
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "自動加值");
		inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);

		memset(szDispMsg, 0x00, sizeof(szDispMsg)); 
		sprintf(szDispMsg, "%ld", pobTran->srTRec.lnTotalTopUpAmount);
		inFunc_Amount_Comma(szDispMsg, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
		inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_RIGHT_);
	}

	/* 交易前餘額 */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "交易前餘額 :");
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
	
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "%ld", pobTran->srTRec.lnFinalBeforeAmt);
	inFunc_Amount_Comma(szDispMsg, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
        inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_RIGHT_);

	/* 交易金額 */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "交易金額　 :");
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
	
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "%ld", pobTran->srTRec.lnTxnAmount);
	inFunc_Amount_Comma(szDispMsg, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
        inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_6_, _DISP_RIGHT_);

	/* 交易後餘額 */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "交易後餘額 :");
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
	
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "%ld", pobTran->srTRec.lnFinalAfterAmt);
	inFunc_Amount_Comma(szDispMsg, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_7_, _DISP_RIGHT_);
	
	return (VS_SUCCESS);
}

/*
Function	:inNCCC_Ticket_Find_Last_Void_Topup_CardType_SQLite
Date&Time	:2020/6/11 下午 3:50
Describe        :找最後一個加值交易卡別
*/
int inNCCC_Ticket_Find_Last_Topup_CardType_SQLite(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = VS_SUCCESS;
	char			szQuerySql[200 + 1] = {0};
	char			szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	SQLITE_ALL_TABLE	srAll;
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	inFunc_ComposeFileName(pobTran, szTableName, "", 6);

	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inNCCC_Ticket_Table_Link_TRec(pobTran, &srAll, _LS_READ_);
	
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "SELECT * FROM %s WHERE inCode = %d OR inCode = %d OR inCode = %d ORDER BY inTableID DESC LIMIT 1", szTableName, _TICKET_EASYCARD_TOP_UP_, _TICKET_IPASS_TOP_UP_, _TICKET_ICASH_TOP_UP_);
	
	inRetVal = inSqlite_Get_Data_By_External_SQL(gszTranDBPath, &srAll, szQuerySql);
	
	if (inRetVal == VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}

/*
Function	:inNCCC_Ticket_Display_Inform_Activation_After_TMS_Download
Date&Time	:2020/6/19 上午 10:37
Describe        :
*/
int inNCCC_Ticket_Display_Inform_Activation_After_TMS_Download(void)
{
	int		inOpenCnt = 0;
	char		szFuncEnable[2 + 1] = {0};
	char		szDispMsg[50 + 1] = {0};
	char		szQRCode[500 + 1] = {0};
	unsigned char	uszIPASSBit = VS_FALSE;
	unsigned char	uszECCBit = VS_FALSE;
	unsigned char	uszICASHBit = VS_FALSE;
	
	/* IPASS */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inLoadTDTRec(_TDT_INDEX_00_IPASS_);
	inGetTicket_HostEnable(szFuncEnable);
	if (szFuncEnable[0] == 'Y')
	{
		uszIPASSBit = VS_TRUE;
		inOpenCnt++;
	}
	
	/* ECC */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inLoadTDTRec(_TDT_INDEX_01_ECC_);
	inGetTicket_HostEnable(szFuncEnable);
	if (szFuncEnable[0] == 'Y')
	{
		uszECCBit = VS_TRUE;
		inOpenCnt++;
	}
	
	/* ICASH */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inLoadTDTRec(_TDT_INDEX_02_ICASH_);
	inGetTicket_HostEnable(szFuncEnable);
	if (szFuncEnable[0] == 'Y')
	{
		uszICASHBit = VS_TRUE;
		inOpenCnt++;
	}
	
	/* 如果沒有開電票，就不啟用 */
	if (inOpenCnt == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "No open ticket host");
		}
		
		return (VS_SUCCESS);
	}
	
	inDISP_ClearAll();
	inDISP_ChineseFont("本機已完成參數下載", _FONTSIZE_8X22_, _LINE_8_1_, _DISP_CENTER_);
	inDISP_ChineseFont("下列電票功能已啟用", _FONTSIZE_8X22_, _LINE_8_2_, _DISP_CENTER_);
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	
	if (uszIPASSBit == VS_TRUE)
	{
		strcat(szDispMsg, "一卡通");
		strcat(szDispMsg, "/");
	}
	
	if (uszECCBit == VS_TRUE)
	{
		strcat(szDispMsg, "悠遊卡");
		strcat(szDispMsg, "/");
	}
	
	if (uszICASHBit == VS_TRUE)
	{
		strcat(szDispMsg, "iCASH");
		strcat(szDispMsg, "/");
	}
	
	if (szDispMsg[strlen(szDispMsg) - 1] == '/')
	{
		szDispMsg[strlen(szDispMsg) - 1] = 0x00;
	}
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X33_, _LINE_8_3_, _DISP_CENTER_);
	
	memset(szQRCode, 0x00, sizeof(szQRCode));
	strcpy(szQRCode, "https://www.nccc.com.tw/wps/wcm/connect/zh/home/AuthorizedStoreArea/CustomerService/EducationArea");
	inDISP_Display_QRCode(szQRCode, 80, _COORDINATE_Y_LINE_8_5_);
	
	inDISP_Wait(3000);
	
	return (VS_SUCCESS);
}

/*
Function	:inNCCC_Ticket_PrintBuffer_Inform_Activation_After_TMS_Download
Date&Time	:2020/6/23 上午 11:47
Describe        :
*/
int inNCCC_Ticket_PrintBuffer_Inform_Activation_After_TMS_Download(void)
{
	int		i = 0;
	int		inOpenCnt = 0;
	int		inRetVal = VS_SUCCESS;
	char		szFuncEnable[2 + 1] = {0};
	char		szDispMsg[50 + 1] = {0};
	char		szQRCode[500 + 1] = {0};
	unsigned char	uszIPASSBit = VS_FALSE;
	unsigned char	uszECCBit = VS_FALSE;
	unsigned char	uszICASHBit = VS_FALSE;
	unsigned char	uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle	srBhandle;
	FONT_ATTRIB	srFont_Attrib;
	
	/* IPASS */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inLoadTDTRec(_TDT_INDEX_00_IPASS_);
	inGetTicket_HostEnable(szFuncEnable);
	if (szFuncEnable[0] == 'Y')
	{
		uszIPASSBit = VS_TRUE;
		inOpenCnt++;
	}
	
	/* ECC */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inLoadTDTRec(_TDT_INDEX_01_ECC_);
	inGetTicket_HostEnable(szFuncEnable);
	if (szFuncEnable[0] == 'Y')
	{
		uszECCBit = VS_TRUE;
		inOpenCnt++;
	}
	
	/* ICASH */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inLoadTDTRec(_TDT_INDEX_02_ICASH_);
	inGetTicket_HostEnable(szFuncEnable);
	if (szFuncEnable[0] == 'Y')
	{
		uszICASHBit = VS_TRUE;
		inOpenCnt++;
	}
	
	/* 如果沒有開電票，就不啟用 */
	if (inOpenCnt == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "No open ticket host");
		}
		
		return (VS_SUCCESS);
	}
	
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);
		
		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		
		inPRINT_Buffer_PutIn("本機已完成參數下載", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		inPRINT_Buffer_PutIn("下列電票功能已啟用", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));

		if (uszIPASSBit == VS_TRUE)
		{
			strcat(szDispMsg, "一卡通");
			strcat(szDispMsg, "/");
		}

		if (uszECCBit == VS_TRUE)
		{
			strcat(szDispMsg, "悠遊卡");
			strcat(szDispMsg, "/");
		}

		if (uszICASHBit == VS_TRUE)
		{
			strcat(szDispMsg, "iCASH");
			strcat(szDispMsg, "/");
		}

		if (szDispMsg[strlen(szDispMsg) - 1] == '/')
		{
			szDispMsg[strlen(szDispMsg) - 1] = 0x00;
		}
		inPRINT_Buffer_PutIn(szDispMsg, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		
		memset(szQRCode, 0x00, sizeof(szQRCode));
		strcpy(szQRCode, "https://www.nccc.com.tw/wps/wcm/connect/zh/home/AuthorizedStoreArea/CustomerService/EducationArea");
		inPRINT_Buffer_QRcode(szQRCode, uszBuffer, &srBhandle, _PRINT_DEFINE_X_03_);
		
		for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		}
	
		if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer, &srBhandle)) != VS_SUCCESS)
			return (inRetVal);
	}
	
	return (VS_SUCCESS);
}

/*
Function	:inNCCC_Ticket_Display_Inform_Activation_After_ESVC_SignOn
Date&Time	:2020/6/23 下午 3:45
Describe        :Displayflag:0表示都顯示 1表示只顯示前半段 2表示只顯示後半段
 *		 inTitleFlag:0:電票啟用結果 1:電票啟用查詢
*/
int inNCCC_Ticket_Display_Inform_Activation_After_ESVC_SignOn(int inDisplayFlag, int inTitleFlag)
{
	int		inOpenCnt = 0;
	int		inCurrentMsg = 0;
	char		szFuncEnable[2 + 1] = {0};
	char		szSignOn[2 + 1] = {0};
	char		szDispMsg[4][50 + 1] = {};
	char		szKey = 0;
	unsigned char	uszIPASSBit = VS_FALSE;
	unsigned char	uszECCBit = VS_FALSE;
	unsigned char	uszICASHBit = VS_FALSE;
	unsigned char	uszIPASS_SignOnBit = VS_FALSE;
	unsigned char	uszECC_SignOnBit = VS_FALSE;
	unsigned char	uszICASH_SignOnBit = VS_FALSE;
		
	/* IPASS */
	inLoadTDTRec(_TDT_INDEX_00_IPASS_);
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetTicket_HostEnable(szFuncEnable);
	if (szFuncEnable[0] == 'Y')
	{
		uszIPASSBit = VS_TRUE;
		inOpenCnt++;
	}
	memset(szSignOn, 0x00, sizeof(szSignOn));
	inGetTicket_LogOnOK(szSignOn);
	if (szSignOn[0] == 'Y')
	{
		uszIPASS_SignOnBit = VS_TRUE;
	}
	
	/* ECC */
	inLoadTDTRec(_TDT_INDEX_01_ECC_);
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetTicket_HostEnable(szFuncEnable);
	if (szFuncEnable[0] == 'Y')
	{
		uszECCBit = VS_TRUE;
		inOpenCnt++;
	}
	memset(szSignOn, 0x00, sizeof(szSignOn));
	inGetTicket_LogOnOK(szSignOn);
	if (szSignOn[0] == 'Y')
	{
		uszECC_SignOnBit = VS_TRUE;
	}
	
	/* ICASH */
	inLoadTDTRec(_TDT_INDEX_02_ICASH_);
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetTicket_HostEnable(szFuncEnable);
	if (szFuncEnable[0] == 'Y')
	{
		uszICASHBit = VS_TRUE;
		inOpenCnt++;
	}
	memset(szSignOn, 0x00, sizeof(szSignOn));
	inGetTicket_LogOnOK(szSignOn);
	if (szSignOn[0] == 'Y')
	{
		uszICASH_SignOnBit = VS_TRUE;
	}
	
	/* 如果沒有開電票，就不啟用 */
	if (inOpenCnt == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "No open ticket host");
		}
		
		return (VS_SUCCESS);
	}
	
	/* 電子票證 */
	inDISP_ClearAll();
	inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);
	inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);
	
	if (inDisplayFlag == 0 ||
	    inDisplayFlag == 1)
	{
		if (inTitleFlag == 1)
		{
			inDISP_ChineseFont("電票啟用查詢", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
		}
		else
		{
			inDISP_ChineseFont("電票啟用結果", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
		}

		inCurrentMsg = 0;
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		if (uszIPASSBit == VS_TRUE)
		{
			if (uszIPASS_SignOnBit == VS_TRUE)
			{
				sprintf(&szDispMsg[inCurrentMsg][0], "一卡通 ○ ");
			}
			else
			{
				sprintf(&szDispMsg[inCurrentMsg][0], "一卡通 X ");
			}
			inCurrentMsg++;
		}

		if (uszECCBit == VS_TRUE)
		{
			if (uszECC_SignOnBit == VS_TRUE)
			{
				sprintf(&szDispMsg[inCurrentMsg][0], "悠遊卡 ○ ");
			}
			else
			{
				sprintf(&szDispMsg[inCurrentMsg][0], "悠遊卡 X ");
			}
			inCurrentMsg++;
		}

		if (uszICASHBit == VS_TRUE)
		{
			if (uszICASH_SignOnBit == VS_TRUE)
			{
				sprintf(&szDispMsg[inCurrentMsg][0], "iCash ○ ");
			}
			else
			{
				sprintf(&szDispMsg[inCurrentMsg][0], "iCash X ");
			}
			inCurrentMsg++;
		}

		/* 如果開四個，第一行要顯示兩個 */
		if (inOpenCnt == 4)
		{
			inDISP_ChineseFont(&szDispMsg[0][0], _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
			inDISP_ChineseFont(&szDispMsg[1][0], _FONTSIZE_8X22_, _LINE_8_5_, _DISP_RIGHT_);
			inDISP_ChineseFont(&szDispMsg[2][0], _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
			inDISP_ChineseFont(&szDispMsg[3][0], _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
		}
		/* 小於等於3 */
		else
		{
			inDISP_ChineseFont(&szDispMsg[0][0], _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
			inDISP_ChineseFont(&szDispMsg[1][0], _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
			inDISP_ChineseFont(&szDispMsg[2][0], _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
		}

		/* 事件初始化 */
		szKey = 0;
		/* 設定Timeout */
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 10);
		while (1)
		{
			/* 偵測按鍵 */
			szKey = uszKBD_Key();

			/* TimeOut */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey != 0)
			{
				break;
			}
		}
	}
	
	if (inDisplayFlag == 0	||
	    inDisplayFlag == 2)
	{
		/* 如果有啟用失敗的情況 */
		if ((uszIPASSBit == VS_TRUE && uszIPASS_SignOnBit != VS_TRUE)	||
		    (uszECCBit == VS_TRUE && uszECC_SignOnBit != VS_TRUE)	||
		    (uszICASHBit == VS_TRUE && uszICASH_SignOnBit != VS_TRUE))
		{
			inDISP_Clear_Line(_LINE_8_5_, _LINE_8_8_);
			inDISP_ChineseFont("部份票證啟用失敗時請", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
			inDISP_ChineseFont("報修或繼續交易", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);

			/* 事件初始化 */
			szKey = 0;
			/* 設定Timeout */
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, 10);
			while (1)
			{
				/* 偵測按鍵 */
				szKey = uszKBD_Key();

				/* TimeOut */
				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
				{
					szKey = _KEY_TIMEOUT_;
				}

				if (szKey != 0)
				{
					break;
				}
			}
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_Inquiry_Beep
Date&Time       :2020/8/6 上午 11:30
Describe        :票證詢卡嗶聲統一
*/
int inNCCC_Ticket_Inquiry_Beep(TRANSACTION_OBJECT *pobTran)
{
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_Ticket_Inquiry_Beep() START !");
	}
	
	inDISP_BEEP(1, 0);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Ticket_Inquiry_Beep() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}


int inTicketFunc_Check_Card_Flow(int *inTicketType)
{
	int		inRetVal = VS_SUCCESS;
	int		inTicketPollingTime = 0;
	int		inUIDLen = 0;
	char		szTemplate[100 + 1] = {0};
	char		szUID[50 + 1] = {0};
	unsigned int	uiECCValue = 0;
	unsigned int	uiIPASSValue = 0;
	unsigned int	uiICASHValue = 0;
	unsigned int	uiOutDataLen_IPASS = 0;
	unsigned char	uszTimeoutBit = VS_FALSE;
	unsigned char	uszFindCardBit = VS_FALSE;
	unsigned char	uszAPDU[100 + 1] = {0};
	unsigned char	uszOutData_ECC[500 + 1] = {0};
	unsigned char	uszOutDataLen_ECC[2 + 1] = {0};
	unsigned char	uszOutData_IPASS[500 + 1] = {0};
	unsigned char	uszECCNotRegisterBit = VS_FALSE;
	unsigned char	uszIPASSNotRegisterBit = VS_FALSE;
	unsigned char	uszICASHNotRegisterBit = VS_FALSE;
	unsigned char	uszECCUnknownErrorBit = VS_FALSE;
	unsigned char	uszIPASSUnknownErrorBit = VS_FALSE;
	unsigned char	uszICASHUnknownErrorBit = VS_FALSE;
	unsigned char	uszIPASSDoubleCheckBit = VS_FALSE;
	
	unsigned short Ts_CheckEasyCard2(unsigned char* bpApdu, unsigned char* outData, unsigned char* RspLe);
	
	char	gszError[4 + 1];
	#define _ECR_ERROR_CODE_CHECK_CARD_ERROR_UNKNOWN_		"2000"
	#define _ECR_ERROR_CODE_CHECK_CARD_INVALID_PARAM_		"2001"
	#define _ECR_ERROR_CODE_CHECK_CARD_TIMEOUT_			"2002"
	#define _ECR_ERROR_CODE_CHECK_CARD_MULTI_CARD_			"2003"
	#define _ECR_ERROR_CODE_CHECK_CARD_UNKNOWN_CARD_		"2004"
	#define _ECR_ERROR_CODE_CHECK_CARD_APDU_LENGTH_INVALID_		"2005"
	#define _ECR_ERROR_CODE_CHECK_CARD_ALL_TICKET_REGISTER_ERROR_	"2006"

	#define PRET_CLA_INS_ERR				0x6001	// CLA, INS ERROR
	#define PRET_P1_P2_ERR					0x6002	// P1, P2 ERROR
	#define PRET_LC_LE_ERR					0x6003	// LC, LE ERROR
	#define PRET_SAM_ERR					0x6301	// SAM認證失敗
	#define PRET_RFID_ACCESS_FAIL				0x6201	// 找不到卡片
	#define PRET_RFID_READ_ERR				0x6202	// 讀卡失敗
	#define PRET_RFID_WRITE_ERR				0x6203	// 寫卡失敗
	#define PRET_RFID_ACCESS_MULTI				0x6204	// 多張卡
	#define PRET_RFID_LOAD_FAIL				0x6205	// RC531 load key失敗
	#define PRET_RFID_AUTH_FAIL				0x6206	// RC531 Auth失敗
	#define APDU_SUCCESS					0x9000
	
	/* 初始化 */
	*inTicketType = _TICKET_TYPE_NONE_;
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
//	inGetETicketPollingTime(szTemplate);
	sprintf(szTemplate, "500");
	inTicketPollingTime = atoi(szTemplate) * 10;
	inDISP_Timer_Start_MicroSecond(TIMER_ID_1, (inTicketPollingTime / 10));
	
	do
	{
		do
		{
			memset(uszOutData_ECC, 0x00, sizeof(uszOutData_ECC));
			memset(uszOutDataLen_ECC, 0x00, sizeof(uszOutDataLen_ECC));
			memset(uszOutData_IPASS, 0x00, sizeof(uszOutData_IPASS));
			uiOutDataLen_IPASS = 0;
			uszIPASSDoubleCheckBit = VS_FALSE;
			
			if (inTimerGet(TIMER_ID_1) == VS_SUCCESS)
			{
				uszTimeoutBit = VS_TRUE;
				break;
			}
			
			memset(szUID, 0x00, sizeof(szUID));
			inUIDLen = 0;

			inRetVal = inCTLS_Get_TypeACardSN(szUID);
			if (inRetVal == VS_SUCCESS)
			{
				inUIDLen = strlen(szUID);
			}

			if (inUIDLen > 0)
			{
				/* ECC */
				if (uiECCValue == PRET_SAM_ERR)
				{
					/* 如果未開機認證就不用再詢該票證 */
					uszECCNotRegisterBit = VS_TRUE;
				}
				else
				{
					inNCCC_tSAM_SelectAID_ECC_Flow();
					inRetVal = Ts_CheckEasyCard2(uszAPDU, uszOutData_ECC, uszOutDataLen_ECC);
					if (inRetVal == APDU_SUCCESS)
					{
						uszFindCardBit = VS_TRUE;
						*inTicketType = _TICKET_TYPE_ECC_;
						break;
					}
					else if (inRetVal == PRET_RFID_ACCESS_FAIL)
					{
						/* 沒偵測到卡片，繼續詢 */
					}
					else if (inRetVal == PRET_SAM_ERR)
					{
						/* 未開機認證 */
						uiECCValue = inRetVal;
					}
					else if (inRetVal == PRET_RFID_ACCESS_MULTI)
					{
						/* 多張卡 */
						uiECCValue = inRetVal;
						break;
					}
					else
					{
						uiECCValue = inRetVal;
						uszECCUnknownErrorBit = VS_TRUE;
					}
				}

				/* IPass */
				if (uiIPASSValue == IPASS_ERROR_NONE_CHECKRW)
				{
					/* 如果未開機認證就不用再詢該票證 */
					uszIPASSNotRegisterBit = VS_TRUE;
				}
				else
				{
					inNCCC_tSAM_SelectAID_IPASS_Flow();
					memset(szTemplate, 0x00, sizeof(szTemplate));
					szTemplate[0] = 1;
					inRetVal = IPassMP_RequestQueryTicket(0, (unsigned char*)szTemplate, 5,  uszOutData_IPASS, &uiOutDataLen_IPASS);
//					inRetVal = IPassMP_FastQueryTicket(0, uszOutData_IPASS, &uiOutDataLen_IPASS);

//					/* 因為悠遊卡丟進去會回傳成功，所以需要Double Check */
//					if (inRetVal == IPASS_EXECUTE_OK)
//					{
//						memset(szTemplate, 0x00, sizeof(szTemplate)); 
//						inFunc_BCD_to_ASCII(szTemplate, uszOutData_IPASS, uiOutDataLen_IPASS);
//
//						memset(uszKey, 0x00, sizeof(uszKey)); 
//						inFunc_ASCII_to_BCD(uszKey, &szTemplate[2], 6);
//						memset(uszAuthenKey, 0x00, sizeof(uszAuthenKey));
//
//						/* 轉換方式抄虹堡Sample code */
//						for (i = 0; i < 6; i++)
//						{
//							uszKeyBuffer = uszKey[i];
//							uszAuthenKey[i * 2]=(uszKeyBuffer ^ 0xF0) & 0xF0;
//							uszAuthenKey[i * 2]|= (uszKeyBuffer >> 4);
//							uszAuthenKey[i * 2 + 1]=((uszKeyBuffer ^ 0x0F) << 4);
//							uszAuthenKey[i * 2 + 1]|= (uszKeyBuffer & 0x0F);
//						}
//
//						inIPASSRetVal = inCTLS_Mifare_LoadKey(uszAuthenKey);
//
//						if (inIPASSRetVal == VS_SUCCESS)
//						{
//							/* IPASS 授權固定只取UID前四Byte */
//							inUIDLen = 8;
//							*(szUID + 8) = 0x00;
//
//							inUIDLen_BCD = inUIDLen / 2;
//							memset(uszUID_BCD, 0x00, sizeof(uszUID_BCD));
//							inFunc_ASCII_to_BCD(uszUID_BCD, szUID, inUIDLen_BCD);
//							/* 第0個Block */
//							if (uszOutData_IPASS[0] == 0x60)
//							{
//								inIPASSRetVal = inCTLS_Mifare_Auth(0x60, 0, uszUID_BCD, inUIDLen_BCD);
//							}
//							else if (uszOutData_IPASS[0] == 0x61)
//							{
//								inIPASSRetVal = inCTLS_Mifare_Auth(0x61, 0, uszUID_BCD, inUIDLen_BCD);
//							}
//							else
//							{
//								inIPASSRetVal = VS_ERROR;
//							}
//						}
//
//						if (inIPASSRetVal == VS_SUCCESS)
//						{
//							uszIPASSDoubleCheckBit = VS_TRUE;
//						}
//						else
//						{
//							uszIPASSDoubleCheckBit = VS_FALSE;
//						}
//					}

//					if (inRetVal == IPASS_EXECUTE_OK	&&
//					    uszIPASSDoubleCheckBit == VS_TRUE)
//					{
					if (inRetVal == IPASS_EXECUTE_OK)
					{
						uszFindCardBit = VS_TRUE;
						*inTicketType = _TICKET_TYPE_IPASS_;
						break;
					}
					else if (inRetVal == IPASS_ERROR_NONE_CHECKRW)
					{
						/* 未開機認證 */
						uiIPASSValue = inRetVal;
					}
					else if (inRetVal == IPASS_ERROR_MULTI_CARD)
					{
						/* 多張卡 */
						uiIPASSValue = inRetVal;
						break;
					}
					else
					{
						uiIPASSValue = inRetVal;
						uszIPASSUnknownErrorBit = VS_TRUE;
					}
				}

				/* ICASH */
				uszICASHNotRegisterBit = VS_TRUE;
			}
			
			/* 代表所有卡都確認有錯誤或未註冊，這邊要所有卡別都確認。 */
			if ((uszECCNotRegisterBit == VS_TRUE || uszECCUnknownErrorBit == VS_TRUE)	&&
			    (uszIPASSNotRegisterBit == VS_TRUE || uszIPASSUnknownErrorBit == VS_TRUE)	&&
			    (uszICASHNotRegisterBit == VS_TRUE || uszICASHUnknownErrorBit == VS_TRUE))
			{
				break;
			}
			
		}while(1);
		
		/* 偵測到或Timeout跳出 */
		if (uszFindCardBit == VS_TRUE)
		{
			break;
		}
		else if (uszTimeoutBit == VS_TRUE)
		{
			sprintf(gszError, _ECR_ERROR_CODE_CHECK_CARD_TIMEOUT_);
			break;
		}
		else
		{
			if (uiECCValue == PRET_RFID_ACCESS_MULTI	||
			    uiIPASSValue == IPASS_ERROR_MULTI_CARD)
			{
				sprintf(gszError, _ECR_ERROR_CODE_CHECK_CARD_MULTI_CARD_);
			}
			else if ((uszECCNotRegisterBit == VS_TRUE || uszECCUnknownErrorBit == VS_TRUE)	&&
				 (uszIPASSNotRegisterBit == VS_TRUE || uszIPASSUnknownErrorBit == VS_TRUE)	&&
				 (uszICASHNotRegisterBit == VS_TRUE || uszICASHUnknownErrorBit == VS_TRUE))
			{
				if (uszECCNotRegisterBit == VS_TRUE	&&
				    uszIPASSNotRegisterBit == VS_TRUE	&&
				    uszICASHNotRegisterBit == VS_TRUE)
				{
					sprintf(gszError, _ECR_ERROR_CODE_CHECK_CARD_ALL_TICKET_REGISTER_ERROR_);
				}
				else
				{
					sprintf(gszError, _ECR_ERROR_CODE_CHECK_CARD_UNKNOWN_CARD_);
				}
			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "uiECCValue:%04X", uiECCValue);
				inLogPrintf(AT, szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "uiIPASSValue:%04X", uiIPASSValue);
				inLogPrintf(AT, szTemplate);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "uiICASHValue:%04X", uiICASHValue);
				inLogPrintf(AT, szTemplate);
			}
			
			break;
		}
	}while (1);
	
	if (uszFindCardBit == VS_TRUE)
	{		
		if (*inTicketType == _TICKET_TYPE_ECC_)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "ECC card");
			}
		}
		else if (*inTicketType == _TICKET_TYPE_ICASH_)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "ICASH card");
			}
		}
		else if (*inTicketType == _TICKET_TYPE_IPASS_)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "IPASS card");
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "success, but unknown card.");
			}
		}
		
		return (VS_SUCCESS);
	}
	else
	{
		*inTicketType = _TICKET_TYPE_NONE_;
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "unknown card.");
		}
		
		return (VS_ERROR);
	}
	
	
}

/*
Function        :inNCCC_Ticket_098_Mcdonalds_Display_Transaction_Result
Date&Time       :2018/12/11 下午 7:11
Describe        :
*/
int inNCCC_Ticket_098_Mcdonalds_Display_Transaction_Result(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_CUST_RECEIPT_;
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned char	uszKey = 0x00;
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 為了恢復被感應燈號清掉的title */
	inFunc_ResetTitle(pobTran);
	
	inNCCC_Ticket_098_Display_Transaction_Result(pobTran);
			       
	inDISP_Timer_Start(_TIMER_NEXSYS_2_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
	
        /* 因為查詢不印，所以只顯示確認鍵 */
         if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ ||
             pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
             pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
         {
                 inTouchSensorFunc = _Touch_BATCH_END_;

                 while (1)
                 {
                         inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                         uszKey = uszKBD_Key();

                         if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
                         {
                                 uszKey = _KEY_TIMEOUT_;
                         }

                         if (uszKey == _KEY_ENTER_			|| 
                             uszKey == _KEY_TIMEOUT_			||
                             inChoice == _BATCH_END_Touch_ENTER_BUTTON_)
                                 break;
                         else
                                 continue;
                 }
                 /* 清掉觸控殘值 */
                 inDisTouch_Flush_TouchFile();
         }
         else
         {
                 inTouchSensorFunc = _Touch_CUST_RECEIPT_;

                 while (1)
                 {
                         inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                         uszKey = uszKBD_Key();

                         if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
                         {
                                 uszKey = _KEY_TIMEOUT_;
                         }

                         if (uszKey == _KEY_ENTER_			|| 
                             uszKey == _KEY_TIMEOUT_			||
                             inChoice == _CUSTReceipt_Touch_ENTER_)
                         {
                                 inRetVal = VS_SUCCESS;
                                 break;
                         }
                         else if (uszKey == _KEY_CANCEL_			||
                                  inChoice == _CUSTReceipt_Touch_CANCEL_)
                         {
                                 inRetVal = VS_USER_CANCEL;
                                 break;
                         }
                         else
                         {
                                 continue;
                         }
                 }

                 /* 清掉觸控殘值 */
                 inDisTouch_Flush_TouchFile();

                 /* 按取消不印顧客聯 */
                 if (inRetVal == VS_USER_CANCEL)
                 {
                         return (VS_SUCCESS);
                 }
         }/* 是否為詢卡 */
			       
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_098_Display_Transaction_Result
Date&Time       :2018/1/10 下午 2:04
Describe        :
*/
int inNCCC_Ticket_098_Display_Transaction_Result(TRANSACTION_OBJECT *pobTran)
{
	char		szDispMsg[50 + 1] = {0};
	unsigned char	uszEnterDisplayBit = VS_TRUE;		/* 顯示確認鍵 */
        
        if (pobTran->srTRec.inCode == _TICKET_IPASS_DEDUCT_ ||
            pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_ ||
            pobTran->srTRec.inCode == _TICKET_ICASH_DEDUCT_)
        {
                uszEnterDisplayBit = VS_FALSE;
        }
	
	/* 此UI更新只更動V3P */
	if (ginMachineType == _CASTLE_TYPE_V3P_)
	{
		if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_	||
		    pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
		    pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
		{       
			/* 交易完成 票卡餘額 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_CHECK_TICKET_RESULT_01_, 0, _COORDINATE_Y_LINE_8_4_);

			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			if (pobTran->srTRec.lnCardRemainAmount < 0)
			{
				sprintf(szDispMsg, "-$%ld", 0 - pobTran->srTRec.lnCardRemainAmount);
			}
			else
			{
				sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnCardRemainAmount);
			}

			inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
		}
		else
		{
			inNCCC_Ticket_Disp_Receipt(pobTran);
		}
	}
	/* 小螢幕V3C */
	else if (ginMachineType == _CASTLE_TYPE_V3C_ && ginHalfLCD == VS_TRUE)
	{
                if (pobTran->srTRec.lnTotalTopUpAmount > 0L)
                {        
                        if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_)
                        {
                                /* 交易完成 自動加值 票卡餘額 請按確認或清除 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_PutGraphic(_CHECK_TICKET_RESULT_02_, 0, _COORDINATE_Y_LINE_8_4_);
                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount);
                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                if (pobTran->srTRec.lnCardRemainAmount < 0)
                                {
                                        sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount - pobTran->srTRec.lnCardRemainAmount);
                                }
                                else
                                {
                                        sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount + pobTran->srTRec.lnCardRemainAmount);
                                }

                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
                                if (uszEnterDisplayBit == VS_TRUE)
                                {
                                        inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_7_);
                                }
                        }
                        else
                        {
                                /* 交易完成 自動加值 票卡餘額 請按確認或清除 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_PutGraphic(_CHECK_TICKET_RESULT_02_, 0, _COORDINATE_Y_LINE_8_4_);
                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount);
                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                if (pobTran->srTRec.lnFinalAfterAmt > 100000)
                                {
                                        sprintf(szDispMsg, "-$%ld ", pobTran->srTRec.lnFinalAfterAmt - 100000);
                                }
                                else
                                {
                                        sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnFinalAfterAmt);
                                }

                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
                                if (uszEnterDisplayBit == VS_TRUE)
                                {
                                        inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_7_);
                                }
                        }
                }
                else
                {
                        if (pobTran->srTRec.inCode == _TICKET_IPASS_TOP_UP_ ||
                            pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_ ||
                            pobTran->srTRec.inCode == _TICKET_ICASH_TOP_UP_)
                        {
                                /* 加值完成 加值金額 票卡餘額 請按確認或清除 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_PutGraphic(_CHECK_TICKET_RESULT_03_, 0, _COORDINATE_Y_LINE_8_4_);

                                /* 加值金額 */
                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTxnAmount);
                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

                                /* 票卡餘額 */
                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                if (pobTran->srTRec.lnFinalAfterAmt > 100000)
                                {
                                        sprintf(szDispMsg, "-$%ld ", pobTran->srTRec.lnFinalAfterAmt - 100000);
                                }
                                else
                                {
                                        sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnFinalAfterAmt);
                                }

                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
                        }
                        else if (pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_ ||
                                 pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_ ||
                                 pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_)
                        {
                                /* 取消加值完成 取消金額 票卡餘額 請按確認或清除 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_PutGraphic(_CHECK_TICKET_RESULT_04_, 0, _COORDINATE_Y_LINE_8_4_);

                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnTxnAmount);
                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                if (pobTran->srTRec.lnFinalAfterAmt > 100000)
                                {
                                        sprintf(szDispMsg, "-$%ld", pobTran->srTRec.lnFinalAfterAmt - 100000);
                                }
                                else
                                {
                                        sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnFinalAfterAmt);
                                }

                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
                                if (uszEnterDisplayBit == VS_TRUE)
                                {
                                        inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_7_);
                                }
                        }        
                        else if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ ||
                                 pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
                                 pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
                        {       
                                /* 交易完成 票卡餘額 請按確認或清除 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_PutGraphic(_CHECK_TICKET_RESULT_01_, 0, _COORDINATE_Y_LINE_8_4_);

                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                if (pobTran->srTRec.lnCardRemainAmount < 0)
                                {
                                        sprintf(szDispMsg, "-$%ld", 0 - pobTran->srTRec.lnCardRemainAmount);
                                }
                                else
                                {
                                        sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnCardRemainAmount);
                                }

                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
                                inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);
                        }
                        else
                        {
                                /* 交易完成 票卡餘額 請按確認或清除 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_PutGraphic(_CHECK_TICKET_RESULT_01_, 0, _COORDINATE_Y_LINE_8_4_);

                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                if (pobTran->srTRec.lnFinalAfterAmt > 100000)
                                {
                                        sprintf(szDispMsg, "-$%ld", pobTran->srTRec.lnFinalAfterAmt - 100000);
                                }
                                else
                                {
                                        sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnFinalAfterAmt);
                                }

                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
                                if (uszEnterDisplayBit == VS_TRUE)
                                {
                                        inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_7_);
                                }
                        }
                }
                /* 大螢幕V3C */
        }
	else
	{
                if (pobTran->srTRec.lnTotalTopUpAmount > 0L)
                {        
                        if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_)
                        {
                                /* 交易完成 自動加值 票卡餘額 請按確認或清除 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_PutGraphic(_CHECK_TICKET_RESULT_02_, 0, _COORDINATE_Y_LINE_8_4_);
                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount);
                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                if (pobTran->srTRec.lnCardRemainAmount < 0)
                                {
                                        sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount - pobTran->srTRec.lnCardRemainAmount);
                                }
                                else
                                {
                                        sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount + pobTran->srTRec.lnCardRemainAmount);
                                }

                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
                        }
                        else
                        {
                                /* 交易完成 自動加值 票卡餘額 請按確認或清除 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_PutGraphic(_CHECK_TICKET_RESULT_02_, 0, _COORDINATE_Y_LINE_8_4_);
                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount);
                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                if (pobTran->srTRec.lnFinalAfterAmt > 100000)
                                {
                                        sprintf(szDispMsg, "-$%ld ", pobTran->srTRec.lnFinalAfterAmt - 100000);
                                }
                                else
                                {
                                        sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnFinalAfterAmt);
                                }

                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
                                if (uszEnterDisplayBit == VS_TRUE)
                                {
                                        inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_7_);
                                }
                        }
                }
                else
                {
                        if (pobTran->srTRec.inCode == _TICKET_IPASS_TOP_UP_ ||
                            pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_ ||
                            pobTran->srTRec.inCode == _TICKET_ICASH_TOP_UP_)
                        {
                                /* 加值完成 加值金額 票卡餘額 請按確認或清除 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_PutGraphic(_CHECK_TICKET_RESULT_03_, 0, _COORDINATE_Y_LINE_8_4_);

                                /* 加值金額 */
                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTxnAmount);
                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

                                /* 票卡餘額 */
                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                if (pobTran->srTRec.lnFinalAfterAmt > 100000)
                                {
                                        sprintf(szDispMsg, "-$%ld ", pobTran->srTRec.lnFinalAfterAmt - 100000);
                                }
                                else
                                {
                                        sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnFinalAfterAmt);
                                }

                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
                        }
                        else if (pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_ ||
                                 pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_ ||
                                 pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_)
                        {
                                /* 取消加值完成 取消金額 票卡餘額 請按確認或清除 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_PutGraphic(_CHECK_TICKET_RESULT_04_, 0, _COORDINATE_Y_LINE_8_4_);

                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnTxnAmount);
                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                if (pobTran->srTRec.lnFinalAfterAmt > 100000)
                                {
                                        sprintf(szDispMsg, "-$%ld", pobTran->srTRec.lnFinalAfterAmt - 100000);
                                }
                                else
                                {
                                        sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnFinalAfterAmt);
                                }

                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
                        }        
                        else if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ ||
                                 pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
                                 pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
                        {       
                                /* 交易完成 票卡餘額 請按確認或清除 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_PutGraphic(_CHECK_TICKET_RESULT_01_, 0, _COORDINATE_Y_LINE_8_4_);

                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                if (pobTran->srTRec.lnCardRemainAmount < 0)
                                {
                                        sprintf(szDispMsg, "-$%ld", 0 - pobTran->srTRec.lnCardRemainAmount);
                                }
                                else
                                {
                                        sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnCardRemainAmount);
                                }

                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
                                inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);
                        }
                        else
                        {
                                /* 交易完成 票卡餘額 請按確認或清除 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_PutGraphic(_CHECK_TICKET_RESULT_01_, 0, _COORDINATE_Y_LINE_8_4_);

                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                if (pobTran->srTRec.lnFinalAfterAmt > 100000)
                                {
                                        sprintf(szDispMsg, "-$%ld", pobTran->srTRec.lnFinalAfterAmt - 100000);
                                }
                                else
                                {
                                        sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnFinalAfterAmt);
                                }

                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
                                if (uszEnterDisplayBit == VS_TRUE)
                                {
                                        inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_7_);
                                }
                        }
                }
	}
        	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_098_Display_Transaction_Result_End
Date&Time       :2018/1/10 下午 2:04
Describe        :
*/
int inNCCC_Ticket_098_Display_Transaction_Result_End(TRANSACTION_OBJECT *pobTran)
{
	char		szDispMsg[50 + 1] = {0};
	
	/* 此UI更新只更動V3P */
	if (ginMachineType == _CASTLE_TYPE_V3P_)
	{
		if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_	||
		    pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
		    pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
		{       
			/* 交易完成 票卡餘額 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_CHECK_TICKET_RESULT_01_, 0, _COORDINATE_Y_LINE_8_4_);

			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			if (pobTran->srTRec.lnCardRemainAmount < 0)
			{
				sprintf(szDispMsg, "-$%ld", 0 - pobTran->srTRec.lnCardRemainAmount);
			}
			else
			{
				sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnCardRemainAmount);
			}

			inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
		}
		else
		{
			inNCCC_Ticket_Disp_Receipt(pobTran);
		}
	}
	/* 小螢幕V3C */
	else if (ginMachineType == _CASTLE_TYPE_V3C_ && ginHalfLCD == VS_TRUE)
	{
                if (pobTran->srTRec.lnTotalTopUpAmount > 0L)
                {        
                        if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_)
                        {
                                /* 交易完成 自動加值 票卡餘額 請按確認或清除 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_PutGraphic(_CHECK_TICKET_RESULT_02_, 0, _COORDINATE_Y_LINE_8_4_);
                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount);
                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                if (pobTran->srTRec.lnCardRemainAmount < 0)
                                {
                                        sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount - pobTran->srTRec.lnCardRemainAmount);
                                }
                                else
                                {
                                        sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount + pobTran->srTRec.lnCardRemainAmount);
                                }

                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
                        }
                        else
                        {
                                /* 交易完成 自動加值 票卡餘額 請按確認或清除 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_PutGraphic(_CHECK_TICKET_RESULT_02_, 0, _COORDINATE_Y_LINE_8_4_);
                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount);
                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                if (pobTran->srTRec.lnFinalAfterAmt > 100000)
                                {
                                        sprintf(szDispMsg, "-$%ld ", pobTran->srTRec.lnFinalAfterAmt - 100000);
                                }
                                else
                                {
                                        sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnFinalAfterAmt);
                                }

                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
                        }
                }
                else
                {
                        if (pobTran->srTRec.inCode == _TICKET_IPASS_TOP_UP_ ||
                            pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_ ||
                            pobTran->srTRec.inCode == _TICKET_ICASH_TOP_UP_)
                        {
                                /* 加值完成 加值金額 票卡餘額 請按確認或清除 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_PutGraphic(_CHECK_TICKET_RESULT_03_, 0, _COORDINATE_Y_LINE_8_4_);

                                /* 加值金額 */
                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTxnAmount);
                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

                                /* 票卡餘額 */
                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                if (pobTran->srTRec.lnFinalAfterAmt > 100000)
                                {
                                        sprintf(szDispMsg, "-$%ld ", pobTran->srTRec.lnFinalAfterAmt - 100000);
                                }
                                else
                                {
                                        sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnFinalAfterAmt);
                                }

                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
                        }
                        else if (pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_ ||
                                 pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_ ||
                                 pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_)
                        {
                                /* 取消加值完成 取消金額 票卡餘額 請按確認或清除 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_PutGraphic(_CHECK_TICKET_RESULT_04_, 0, _COORDINATE_Y_LINE_8_4_);

                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnTxnAmount);
                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                if (pobTran->srTRec.lnFinalAfterAmt > 100000)
                                {
                                        sprintf(szDispMsg, "-$%ld", pobTran->srTRec.lnFinalAfterAmt - 100000);
                                }
                                else
                                {
                                        sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnFinalAfterAmt);
                                }

                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
                        }        
                        else if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ ||
                                 pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
                                 pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
                        {       
                                /* 交易完成 票卡餘額 請按確認或清除 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_PutGraphic(_CHECK_TICKET_RESULT_01_, 0, _COORDINATE_Y_LINE_8_4_);

                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                if (pobTran->srTRec.lnCardRemainAmount < 0)
                                {
                                        sprintf(szDispMsg, "-$%ld", 0 - pobTran->srTRec.lnCardRemainAmount);
                                }
                                else
                                {
                                        sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnCardRemainAmount);
                                }

                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
                                inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);
                        }
                        else
                        {
                                /* 交易完成 票卡餘額 請按確認或清除 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_PutGraphic(_CHECK_TICKET_RESULT_01_, 0, _COORDINATE_Y_LINE_8_4_);

                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                if (pobTran->srTRec.lnFinalAfterAmt > 100000)
                                {
                                        sprintf(szDispMsg, "-$%ld", pobTran->srTRec.lnFinalAfterAmt - 100000);
                                }
                                else
                                {
                                        sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnFinalAfterAmt);
                                }

                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
                        }
                }
	}
	/* 大螢幕V3C */
	else
	{
                if (pobTran->srTRec.lnTotalTopUpAmount > 0L)
                {        
                        if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_)
                        {
                                /* 交易完成 自動加值 票卡餘額 請按確認或清除 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_PutGraphic(_CHECK_TICKET_RESULT_02_, 0, _COORDINATE_Y_LINE_8_4_);
                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount);
                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                if (pobTran->srTRec.lnCardRemainAmount < 0)
                                {
                                        sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount - pobTran->srTRec.lnCardRemainAmount);
                                }
                                else
                                {
                                        sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount + pobTran->srTRec.lnCardRemainAmount);
                                }

                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
                        }
                        else
                        {
                                /* 交易完成 自動加值 票卡餘額 請按確認或清除 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_PutGraphic(_CHECK_TICKET_RESULT_02_, 0, _COORDINATE_Y_LINE_8_4_);
                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTotalTopUpAmount);
                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                if (pobTran->srTRec.lnFinalAfterAmt > 100000)
                                {
                                        sprintf(szDispMsg, "-$%ld ", pobTran->srTRec.lnFinalAfterAmt - 100000);
                                }
                                else
                                {
                                        sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnFinalAfterAmt);
                                }

                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
                        }
                }
                else
                {
                        if (pobTran->srTRec.inCode == _TICKET_IPASS_TOP_UP_ ||
                            pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_ ||
                            pobTran->srTRec.inCode == _TICKET_ICASH_TOP_UP_)
                        {
                                /* 加值完成 加值金額 票卡餘額 請按確認或清除 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_PutGraphic(_CHECK_TICKET_RESULT_03_, 0, _COORDINATE_Y_LINE_8_4_);

                                /* 加值金額 */
                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnTxnAmount);
                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

                                /* 票卡餘額 */
                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                if (pobTran->srTRec.lnFinalAfterAmt > 100000)
                                {
                                        sprintf(szDispMsg, "-$%ld ", pobTran->srTRec.lnFinalAfterAmt - 100000);
                                }
                                else
                                {
                                        sprintf(szDispMsg, "$%ld ", pobTran->srTRec.lnFinalAfterAmt);
                                }

                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
                        }
                        else if (pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_ ||
                                 pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_ ||
                                 pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_)
                        {
                                /* 取消加值完成 取消金額 票卡餘額 請按確認或清除 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_PutGraphic(_CHECK_TICKET_RESULT_04_, 0, _COORDINATE_Y_LINE_8_4_);

                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnTxnAmount);
                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                if (pobTran->srTRec.lnFinalAfterAmt > 100000)
                                {
                                        sprintf(szDispMsg, "-$%ld", pobTran->srTRec.lnFinalAfterAmt - 100000);
                                }
                                else
                                {
                                        sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnFinalAfterAmt);
                                }

                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
                        }        
                        else if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ ||
                                 pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
                                 pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
                        {       
                                /* 交易完成 票卡餘額 請按確認或清除 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_PutGraphic(_CHECK_TICKET_RESULT_01_, 0, _COORDINATE_Y_LINE_8_4_);

                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                if (pobTran->srTRec.lnCardRemainAmount < 0)
                                {
                                        sprintf(szDispMsg, "-$%ld", 0 - pobTran->srTRec.lnCardRemainAmount);
                                }
                                else
                                {
                                        sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnCardRemainAmount);
                                }

                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
                                inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);
                        }
                        else
                        {
                                /* 交易完成 票卡餘額 請按確認或清除 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_PutGraphic(_CHECK_TICKET_RESULT_01_, 0, _COORDINATE_Y_LINE_8_4_);

                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                if (pobTran->srTRec.lnFinalAfterAmt > 100000)
                                {
                                        sprintf(szDispMsg, "-$%ld", pobTran->srTRec.lnFinalAfterAmt - 100000);
                                }
                                else
                                {
                                        sprintf(szDispMsg, "$%ld", pobTran->srTRec.lnFinalAfterAmt);
                                }

                                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
                        }
                }
	}
        	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_GenMAC
Date&Time       :2020/8/19 下午 5:54
Describe        :
*/
int inNCCC_Ticket_GenMAC(TRANSACTION_OBJECT *pobTran, TICKET_NCCC_MAC_STRUCT* srMAC)
{
	int 	inMACLen = 0;
	int 	inRetVal;
	char	szDebugMsg[200 + 1] = {0}, szDataBuf[200 + 1] = {0};
	char	szAscii[16 + 1];
	

	/* 處理中‧‧‧‧‧ */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_); 

	if (pobTran->inISOTxnCode == _SETTLE_ || pobTran->inISOTxnCode == _CLS_SETTLE_ || pobTran->inISOTxnCode == _CLS_BATCH_)
	{
		/*
		 * 
			2 . 電票結帳：
			Message Type 為 0500 之 結帳 交易 ，使用下面四 項資料進行 MAC 演算 。
			1. ISO8583 F_11 ( 後 2 碼 (BCD to ASCII)。
			2. ISO8583 F_41 (TID)8 碼 。
			3. ISO8583 F_03( Process ing Cod e 中 間 兩碼 2 碼 (BCD to 。
			4. ISO8583 F _63 Data
			例：
			悠遊卡購貨筆數 悠遊卡購貨總額
			一卡通購貨筆數 一卡通購貨總額
			愛金卡購貨筆數 愛金卡購貨總額
			遠鑫卡購貨筆數 遠鑫卡購貨總額 共計 60 Bytes)
		*/
		memset(szDataBuf, 0x00, sizeof(szDataBuf));
		/* ISO8583 F_11(STAN)後2碼(BCD to ASCII) */
		strcat(szDataBuf, srMAC->szF_11);
		/* Terminal ID */
		strcat(szDataBuf, srMAC->szF_41);
		/* Process Code前兩碼 2 碼 (BCD to ASCII) */
		strcat(szDataBuf, srMAC->szF_03);
		/* ISO8583 F_63 Data */
		strcat(szDataBuf, srMAC->szF_63);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "F11: %s", srMAC->szF_11);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "F41: %s", srMAC->szF_41);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "F03: %s", srMAC->szF_03);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "F63: %s", srMAC->szF_63);
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	else
	{
		/*
		B1. 電票交易：
		包含Message Type 為 0100 、 0102 、 0200 及 0220 之相關交易 ，使用下面四項資料進行 MAC 演算 。
		1. ISO8583 F_11 (STAN) 後2碼 (BCD to ASCII)。
		2. ISO8583 F_41 (TID)8 碼 。
		3. ISO8583 F_03( Processing Code) 中間兩碼 2 碼 (BCD to ASCII)。
		4. ISO8583 F_35 (電票卡號) 前22碼 (ASCII)，不足22碼左靠右補0。。
		5. ISO8583 F_59 之Table ID “ET” (電票交易訊)18碼。 (若該筆交易電文沒有Table ID ,“ET”,則此欄位補滿0))
		6. ISO8583 F_04(Amount)。 例：NT$ 123 => 則DATA為”000000012300”” 12碼。
		*/
		memset(szDataBuf, 0x00, sizeof(szDataBuf));
		/* ISO8583 F_11(STAN)後2碼(BCD to ASCII) */
		strcat(szDataBuf, srMAC->szF_11);
		/* Terminal ID */
		strcat(szDataBuf, srMAC->szF_41);
		/* Process Code前兩碼 2 碼 (BCD to ASCII) */
		strcat(szDataBuf, srMAC->szF_03);
		/* ISO8583 F_35 Data */
		strcat(szDataBuf, srMAC->szF_35);
		/* ISO8583 F_59 Data */
		strcat(szDataBuf, srMAC->szF_59);
		/* ISO8583 F_04 Data */
		strcat(szDataBuf, srMAC->szF_04);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "F11: %s", srMAC->szF_11);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "F41: %s", srMAC->szF_41);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "F03: %s", srMAC->szF_03);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "F35: %s", srMAC->szF_35);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "F59: %s", srMAC->szF_59);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "F04: %s", srMAC->szF_04);
			inLogPrintf(AT, szDebugMsg);
		}
		
	}

	inMACLen = strlen(szDataBuf);

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "==========================================");
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "GenMAC Length [%d]", inMACLen);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "GenMAC Data [%s]", szDataBuf);
		inDISP_LogPrintf_Format(szDebugMsg, "  ", 34);
		
		inLogPrintf(AT, "==========================================");
	}
	
	if (ginISODebug == VS_TRUE)
	{
		inPRINT_ChineseFont("==========================================", _PRT_ISO_);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "GenMAC Length [%d]", inMACLen);
		inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "GenMAC Data [%s]", szDataBuf);
		inPRINT_ChineseFont_Format(szDebugMsg, "  ", 34, _PRT_ISO_);
		
		inPRINT_ChineseFont("==========================================", _PRT_ISO_);
	}

	inRetVal = inNCCC_TMK_CalculateMac(szDataBuf, pobTran->szMAC_HEX);
		
	if (ginDebug == VS_TRUE)
	{
                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char*)pobTran->szMAC_HEX, 8);
        	sprintf(szDebugMsg, "MAC RESULT [%d][%s]", strlen(pobTran->szMAC_HEX), szAscii);
                inDISP_LogPrintf_Format(szDebugMsg, "  ", 34);
        }
	
	if (ginISODebug == VS_TRUE)
	{
                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char*)pobTran->szMAC_HEX, 8);
        	sprintf(szDebugMsg, "MAC RESULT [%d][%s]", strlen(pobTran->szMAC_HEX), szAscii);
                inPRINT_ChineseFont_Format(szDebugMsg, "  ", 34, _PRT_ISO_);
        }

        if (inRetVal != VS_SUCCESS)
	        return (VS_ERROR);

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_Gen_F_56_File
Date&Time       :2020/8/26 下午 5:19
Describe        :
*/
int inNCCC_Ticket_Gen_F_56_File(char* szData, int inLen)
{
	unsigned long	ulFileHandle = 0;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_Ticket_Gen_F_56_File(Len:%d) START !", inLen);
	}
	
	/* 先刪除檔案 */
	inFile_Unlink_File(_ESVC_FILE_F_56_, _FS_DATA_PATH_);
	
	/* 創建檔案 */
	inFILE_Create(&ulFileHandle, (unsigned char*)_ESVC_FILE_F_56_);
	
	inFILE_Seek(ulFileHandle, 0, _SEEK_BEGIN_);
	
	inFILE_Write(&ulFileHandle, (unsigned char*)szData, (unsigned long)inLen);
	
	inFILE_Close(&ulFileHandle);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Ticket_Gen_F_56_File() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_Data_Compress_Encryption
Date&Time       :2020/9/1 下午 2:01
Describe        :加密及壓縮欄位56 
*/
int inNCCC_Ticket_Data_Compress_Encryption(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	int		inRetVal_All = VS_SUCCESS;
	int		inRetVal_SelectAID = VS_SUCCESS;
	int		inLen = 0;
	char		szTemplate[40 + 1] = {0};
	char		szRawData32[32 + 1] = {0};
	char		szRawData17_32[16 + 1] = {0};
	char		szEncryData[16 + 1] = {0};
	char		szField37[8 + 1], szField41[4 + 1] = {0};
	char		szAscii[84 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	unsigned long	ulHandle = 0;
	unsigned char	uszNCCCSlot = 0;
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Data_Compress_Encryption() START!");
	
	/* 抓tSAM Slot */
	inRetVal = inNCCC_tSAM_Decide_tSAM_Slot(&uszNCCCSlot);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inNCCC_tSAM_Decide_tSAM_Slot Failed");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 先清檔案 */
	inFILE_Delete((unsigned char*)_ESVC_FILE_F_56_GZ_ENCRYPTED_);	
		
	/* 加密並置換資料內容 */
	/* 開檔 */
	if (inFILE_Open(&ulHandle, (unsigned char*)_ESVC_FILE_F_56_GZ_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	inRetVal_All = VS_SUCCESS;
	do
	{
		if (inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
		{
			/* 跳出迴圈，關閉檔案 */
			inRetVal_All = VS_ERROR;
			break;
		}

		/* 取得前17到32 Bytes (szData為前32Byte) */
		memset(szRawData32, 0x00, sizeof(szRawData32));
		if (inFILE_Read(&ulHandle, (unsigned char*)szRawData32, 32) != VS_SUCCESS)
		{
			/* 跳出迴圈，關閉檔案 */
			inRetVal_All = VS_ERROR;
			break;
		}
		memset(szRawData17_32, 0x00, sizeof(szRawData17_32));
		memcpy(szRawData17_32, &szRawData32[16], 16);
		inLen = 16;	/* 32 nibble , 16Bytes */

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, (unsigned char*)szRawData17_32, inLen);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			inLogPrintf(AT, "[szEncrypt ]");
			sprintf(szDebugMsg, "%s L:%d", szAscii, inLen);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 加密 */
		/* tSAM加密用SAM加密 */
		/* 開始加密 */
		/* 組 Field_37 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, pobTran->srTRec.szRefNo, 11);
		inFunc_PAD_ASCII(szTemplate, szTemplate, 'F', 16, _PADDING_RIGHT_); /* 補【F】 */
		memset(szField37, 0x00, sizeof(szField37));
		inFunc_ASCII_to_BCD((unsigned char*)szField37, szTemplate, 8);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));	
			inFunc_BCD_to_ASCII(szAscii, (unsigned char*)szField37, 8);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "[field37: %s] %d", szAscii, 8);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 組 Field_41 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTerminalID(szTemplate);
		memset(szField41, 0x00, sizeof(szField41));
		inFunc_ASCII_to_BCD((unsigned char*)szField41, szTemplate, 4);

		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, (unsigned char*)szField41, 4);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "[field41: %s] %d", szAscii, 4);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 切回NCCC */
		inNCCC_tSAM_SelectAID_NCCC(uszNCCCSlot);
		
		inRetVal = inNCCC_tSAM_Encrypt_ESVC(uszNCCCSlot, 
						    inLen,
						    szRawData17_32,
						    (unsigned char*)&szField37,
						    (unsigned char*)&szField41,
						    &gusztSAMKeyIndex,
						    gusztSAMCheckSum_56);

		/* 切回票證AID */
		if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
		{
			inRetVal_SelectAID = inNCCC_tSAM_SelectAID_IPASS_Flow();
		}
		else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
		{
			inRetVal_SelectAID = inNCCC_tSAM_SelectAID_ECC_Flow();
		}
		else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
		{
			inRetVal_SelectAID = inNCCC_tSAM_SelectAID_ICASH_Flow();
		}
		else
		{
			inRetVal_SelectAID = VS_ERROR;
		}
		
		if (inRetVal_SelectAID != VS_SUCCESS)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_SAM_ENCRYPTED_FAIL_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, "無法切回票證AID");
			srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
			inRetVal_All = VS_ERROR;
			break;
		}
		
		if (inRetVal == VS_ERROR)
		{
			pobTran->inErrorMsg = _ERROR_CODE_V3_TSAM_ENCRYPT_FAIL_;
			/* 跳出迴圈，關閉檔案 */
			inRetVal_All = VS_ERROR;
			break;
		}
		else
		{
			memset(szEncryData, 0x00, sizeof(szEncryData));
			memcpy(szEncryData, szRawData17_32, inRetVal);
		}
		
		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, (unsigned char*)szEncryData, inLen);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			inLogPrintf(AT, "[szEncrypted ]");
			sprintf(szDebugMsg, "%s L:%d", szAscii, inLen);
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 取得加密後16 Bytes並塞回去 */
		memcpy(&szRawData32[16], szEncryData, 16);

		if (inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
		{
			/* 跳出迴圈，關閉檔案 */
			inRetVal_All = VS_ERROR;
			break;
		}
		if (inFILE_Write(&ulHandle, (unsigned char*)szRawData32, 32) != VS_SUCCESS)
		{
			/* 跳出迴圈，關閉檔案 */
			inRetVal_All = VS_ERROR;
			break;
		}
	}while (0);

	inFILE_Close(&ulHandle);

	/* 加密過程中有誤，先關再離開 */
	if (inRetVal_All != VS_SUCCESS)
	{
		return (inRetVal_All);
	}

	if (inFILE_Rename((unsigned char*)_ESVC_FILE_F_56_GZ_, (unsigned char*)_ESVC_FILE_F_56_GZ_ENCRYPTED_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
		
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_Ticket_Data_Compress_Encryption() END!");
	
		
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Ticket_InquiryCardNo_Flow
Date&Time       :2023/10/30 下午 6:10
Describe        :查詢票證卡號
*/
int inNCCC_Ticket_InquiryCardNo_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	int	inOrgCode = _TRANS_TYPE_NULL_;
	int	inOrgTxnCode = _TRANS_TYPE_NULL_;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_Ticket_InquiryCardNo_Flow() START !");
	}
	
	inOrgCode = pobTran->srTRec.inCode;
	inOrgTxnCode = pobTran->inTransactionCode;
	
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
	{
		pobTran->srTRec.inCode = _TICKET_IPASS_INQUIRY_;
		pobTran->inTransactionCode = _TICKET_IPASS_INQUIRY_;
		pobTran->srTRec.srIPASSRec.uszQueryBit = VS_TRUE;

		inRetVal = inFLOW_RunFunction(pobTran, _TICKET_INITIAL_LIGHT_STATE_);
		if (inRetVal != VS_SUCCESS)
		{
			pobTran->srTRec.inCode = inOrgCode;
			pobTran->inTransactionCode = inOrgTxnCode;
			pobTran->srTRec.srIPASSRec.uszQueryBit = VS_FALSE;
			return (inRetVal);
		}
		
		inRetVal = inFLOW_RunFunction(pobTran, _TICKET_CHECK_TRANS_ENABLE_);
		if (inRetVal != VS_SUCCESS)
		{
			pobTran->srTRec.inCode = inOrgCode;
			pobTran->inTransactionCode = inOrgTxnCode;
			pobTran->srTRec.srIPASSRec.uszQueryBit = VS_FALSE;
			return (inRetVal);
		}
		
		inRetVal = inFLOW_RunFunction(pobTran, _TICKET_FUNCTION_MUST_SETTLE_CHECK_);
		if (inRetVal != VS_SUCCESS)
		{
			pobTran->srTRec.inCode = inOrgCode;
			pobTran->inTransactionCode = inOrgTxnCode;
			pobTran->srTRec.srIPASSRec.uszQueryBit = VS_FALSE;
			return (inRetVal);
		}
		
		inRetVal = inFLOW_RunFunction(pobTran, _TICKET_GET_PARM_);
		if (inRetVal != VS_SUCCESS)
		{
			pobTran->srTRec.inCode = inOrgCode;
			pobTran->inTransactionCode = inOrgTxnCode;
			pobTran->srTRec.srIPASSRec.uszQueryBit = VS_FALSE;
			return (inRetVal);
		}
		
		inRetVal = inFLOW_RunFunction(pobTran, _TICKET_IPASS_TAPCARD_FIRST_);
		if (inRetVal != VS_SUCCESS)
		{
			pobTran->srTRec.inCode = inOrgCode;
			pobTran->inTransactionCode = inOrgTxnCode;
			pobTran->srTRec.srIPASSRec.uszQueryBit = VS_FALSE;
			return (inRetVal);
		}
		
		inRetVal = inFLOW_RunFunction(pobTran, _TICKET_CHECK_ACK_HOST_);
		if (inRetVal != VS_SUCCESS)
		{
			pobTran->srTRec.inCode = inOrgCode;
			pobTran->inTransactionCode = inOrgTxnCode;
			pobTran->srTRec.srIPASSRec.uszQueryBit = VS_FALSE;
			return (inRetVal);
		}
		
		inRetVal = inFLOW_RunFunction(pobTran, _TICKET_INQUIRY_BEEP_);
		if (inRetVal != VS_SUCCESS)
		{
			pobTran->srTRec.inCode = inOrgCode;
			pobTran->inTransactionCode = inOrgTxnCode;
			pobTran->srTRec.srIPASSRec.uszQueryBit = VS_FALSE;
			return (inRetVal);
		}
		
		pobTran->srTRec.srIPASSRec.uszQueryBit = VS_FALSE;
	}
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
		pobTran->srTRec.inCode = _TICKET_EASYCARD_INQUIRY_;
		pobTran->inTransactionCode = _TICKET_EASYCARD_INQUIRY_;
		
		inRetVal = inFLOW_RunFunction(pobTran, _TICKET_CHECK_TRANS_ENABLE_);
		if (inRetVal != VS_SUCCESS)
		{
			pobTran->srTRec.inCode = inOrgCode;
			pobTran->inTransactionCode = inOrgTxnCode;
			return (inRetVal);
		}
		
		inRetVal = inFLOW_RunFunction(pobTran, _TICKET_FUNCTION_MUST_SETTLE_CHECK_);
		if (inRetVal != VS_SUCCESS)
		{
			pobTran->srTRec.inCode = inOrgCode;
			pobTran->inTransactionCode = inOrgTxnCode;
			return (inRetVal);
		}
		
		inRetVal = inFLOW_RunFunction(pobTran, _TICKET_GET_PARM_);
		if (inRetVal != VS_SUCCESS)
		{
			pobTran->srTRec.inCode = inOrgCode;
			pobTran->inTransactionCode = inOrgTxnCode;
			return (inRetVal);
		}
		
		inRetVal = inFLOW_RunFunction(pobTran, _TICKET_ECC_API_FLOW_);
		if (inRetVal != VS_SUCCESS)
		{
			pobTran->srTRec.inCode = inOrgCode;
			pobTran->inTransactionCode = inOrgTxnCode;
			return (inRetVal);
		}
		
		inRetVal = inFLOW_RunFunction(pobTran, _NCCC_FUNCTION_BEEP_AFTER_AUTH_);
		if (inRetVal != VS_SUCCESS)
		{
			pobTran->srTRec.inCode = inOrgCode;
			pobTran->inTransactionCode = inOrgTxnCode;
			return (inRetVal);
		}
		
		inRetVal = inFLOW_RunFunction(pobTran, _TICKET_INQUIRY_BEEP_);
		if (inRetVal != VS_SUCCESS)
		{
			pobTran->srTRec.inCode = inOrgCode;
			pobTran->inTransactionCode = inOrgTxnCode;
			return (inRetVal);
		}
	}
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
	{
		pobTran->srTRec.inCode = _TICKET_ICASH_INQUIRY_;
		pobTran->inTransactionCode = _TICKET_ICASH_INQUIRY_;
		
		inRetVal = inFLOW_RunFunction(pobTran, _TICKET_CHECK_TRANS_ENABLE_);
		if (inRetVal != VS_SUCCESS)
		{
			pobTran->srTRec.inCode = inOrgCode;
			pobTran->inTransactionCode = inOrgTxnCode;
			return (inRetVal);
		}
		
		inRetVal = inFLOW_RunFunction(pobTran, _TICKET_FUNCTION_MUST_SETTLE_CHECK_);
		if (inRetVal != VS_SUCCESS)
		{
			pobTran->srTRec.inCode = inOrgCode;
			pobTran->inTransactionCode = inOrgTxnCode;
			return (inRetVal);
		}
		
		inRetVal = inFLOW_RunFunction(pobTran, _TICKET_GET_PARM_);
		if (inRetVal != VS_SUCCESS)
		{
			pobTran->srTRec.inCode = inOrgCode;
			pobTran->inTransactionCode = inOrgTxnCode;
			return (inRetVal);
		}
		
		inRetVal = inFLOW_RunFunction(pobTran, _TICKET_ICASH_GET_CARD_AMT_);
		if (inRetVal != VS_SUCCESS)
		{
			pobTran->srTRec.inCode = inOrgCode;
			pobTran->inTransactionCode = inOrgTxnCode;
			return (inRetVal);
		}
		
		inRetVal = inFLOW_RunFunction(pobTran, _TICKET_ICASH_DEMO_FLOW_);
		if (inRetVal != VS_SUCCESS)
		{
			pobTran->srTRec.inCode = inOrgCode;
			pobTran->inTransactionCode = inOrgTxnCode;
			return (inRetVal);
		}
		
		inRetVal = inFLOW_RunFunction(pobTran, _TICKET_INQUIRY_BEEP_);
		if (inRetVal != VS_SUCCESS)
		{
			pobTran->srTRec.inCode = inOrgCode;
			pobTran->inTransactionCode = inOrgTxnCode;
			return (inRetVal);
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_Ticket_InquiryCardNo_Flow() No Match Card Type !");
		}
	}
	
	/* 提示訊息 */
	inNCCC_Ticket_Display_Transaction_Result(pobTran);
	
	pobTran->srTRec.inCode = inOrgCode;
	pobTran->inTransactionCode = inOrgTxnCode;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Ticket_InquiryCardNo_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inNCCC_Ticket_Decide_ticket_Slot
Date&Time       :2024/12/4 下午 6:44
Describe        :
*/
int inNCCC_Ticket_Decide_Ticket_Slot(unsigned char *uszSlot)
{
	int	inRetVal = VS_ERROR;
	char	szSAMSlot[2 + 1] = {0};
	
	memset(szSAMSlot, 0x00, sizeof(szSAMSlot));
	inGetTicket_SAM_Slot(szSAMSlot);
	if (memcmp(szSAMSlot, "01", strlen("01")) == 0)
	{
		*uszSlot = _SAM_SLOT_1_;
		inRetVal = VS_SUCCESS;
	}
	else if (memcmp(szSAMSlot, "02", strlen("02")) == 0)
	{
		*uszSlot = _SAM_SLOT_2_;
		inRetVal = VS_SUCCESS;
	}
	else if (memcmp(szSAMSlot, "03", strlen("03")) == 0)
	{
		*uszSlot = _SAM_SLOT_3_;
		inRetVal = VS_SUCCESS;
	}
	else if (memcmp(szSAMSlot, "04", strlen("04")) == 0)
	{
		*uszSlot = _SAM_SLOT_4_;
		inRetVal = VS_SUCCESS;
	}
	/* 預設第一個SAM Slot，預防意外 */
	else
	{
		*uszSlot = _SAM_SLOT_1_;
		inRetVal = VS_ERROR;
	}
	
	return (inRetVal);
}