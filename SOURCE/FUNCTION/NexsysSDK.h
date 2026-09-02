/* 
 * File:   NexsysSDK.h
 * Author: RussellBai
 *
 * Created on 2018年9月12日, 上午 10:26
 */

extern	int	ginPOSPrinterDebug;
extern	int	ginDebug;
extern	int	ginFindRunTime;
extern	int	ginExamBit;
extern	long	glnExamAmt;

#define _CEE_UPT1000_TERMINAL_VERSION_		"MD731UAGAS001"
#define _CEE_UPT1000_TERMINAL_VERSION_LEN_	13
#define _SDK_COM_PORT_NONE_			0
#define _SDK_COM_PORT_USB_			1
#define _SDK_COM_PORT_COM1_			2
#define _SDK_COM_PORT_COM2_			3
#define _SDK_COM_PORT_COM3_			4

#define _SDK_ERROR_CODE_SUCCESS_		0x0000
#define _SDK_ERROR_CODE_ERROR_			0x0001
#define _SDK_ERROR_CODE_TIMEOUT_		0x0002	/* 超時 */
#define _SDK_ERROR_CODE_GET_TIME_FAIL_		0x0003	/* 取得時間失敗 */
#define _SDK_ERROR_CODE_TRT_ERROR_		0x0004	/* TRT錯誤 */
#define _SDK_ERROR_CODE_SET_LENGTH_ERROR_	0x0005	/* 長度錯誤 */
#define _SDK_ERROR_CODE_POINTER_NULL_ERROR_	0x0006	/* pointer 無效 */
#define _SDK_ERROR_CODE_TMS_NOT_OK_ERROR_	0x0007	/* TMS未下載，不能做交易 */
#define _SDK_ERROR_CODE_FUNCTION_CLOSE_ERROR_	0x0008	/* 此功能已關閉 */
#define _SDK_ERROR_CODE_NO_TXN_RECORD_ERROR_	0x0009	/* 無交易紀錄 */
#define _SDK_ERROR_CODE_USER_CANCEL_		0x000A	/* 使用者取消 */
#define _SDK_ERROR_CODE_READ_BATCH_ERROR_	0x000B	/* 讀取批次有誤 */
#define _SDK_ERROR_CODE_OPT_INITIAL_NOT_YET_	0x000C	/* 尚未執行OPT_TerminalInitial，無法執行SDK內容 */
#define _SDK_ERROR_CODE_PASSWORD_ERROR_		0x000D	/* 管理密碼錯誤 */

#define _SDK_ERROR_CODE_LOAD_TMSCPT_ERROR_			0x1001	/* Load Table錯誤 */
#define _SDK_ERROR_CODE_SAVE_TMSCPT_ERROR_			0x1002	/* Save Table錯誤 */
#define _SDK_ERROR_CODE_LOAD_HDT_ERROR_				0x1003	/* Load Table錯誤 */
#define _SDK_ERROR_CODE_SAVE_HDT_ERROR_				0x1004	/* Save Table錯誤 */
#define _SDK_ERROR_CODE_LOAD_EDC_ERROR_				0x1005	/* Load Table錯誤 */
#define _SDK_ERROR_CODE_SAVE_EDC_ERROR_				0x1006	/* Save Table錯誤 */
#define _SDK_ERROR_CODE_LOAD_CPT_ERROR_				0x1007	/* Load Table錯誤 */
#define _SDK_ERROR_CODE_SAVE_CPT_ERROR_				0x1008	/* Save Table錯誤 */
#define _SDK_ERROR_CODE_LOAD_HDPT_ERROR_			0x1009	/* Load Table錯誤 */
#define _SDK_ERROR_CODE_SAVE_HDPT_ERROR_			0x100A	/* Save Table錯誤 */
#define _SDK_ERROR_CODE_LOAD_TMSFTP_ERROR_			0x100B	/* Load Table錯誤 */
#define _SDK_ERROR_CODE_SAVE_TMSFTP_ERROR_			0x100C	/* Save Table錯誤 */
#define _SDK_ERROR_CODE_LOAD_CFGT_ERROR_			0x100D	/* Load Table錯誤 */
#define _SDK_ERROR_CODE_SAVE_CFGT_ERROR_			0x100E	/* Save Table錯誤 */
#define _SDK_ERROR_CODE_SET_TMSCPT_TAG_ERROR_			0x1101	/* SET TAG錯誤 */
#define _SDK_ERROR_CODE_GET_TMSCPT_TAG_ERROR_			0x1102	/* GET TAG錯誤 */
#define _SDK_ERROR_CODE_SET_HDT_TAG_ERROR_			0x1103	/* SET TAG錯誤 */
#define _SDK_ERROR_CODE_GET_HDT_TAG_ERROR_			0x1104	/* GET TAG錯誤 */
#define _SDK_ERROR_CODE_SET_EDC_TAG_ERROR_			0x1105	/* SET TAG錯誤 */
#define _SDK_ERROR_CODE_GET_EDC_TAG_ERROR_			0x1106	/* GET TAG錯誤 */
#define _SDK_ERROR_CODE_SET_CPT_TAG_ERROR_			0x1107	/* SET TAG錯誤 */
#define _SDK_ERROR_CODE_GET_CPT_TAG_ERROR_			0x1108	/* GET TAG錯誤 */
#define _SDK_ERROR_CODE_GET_HDPT_TAG_ERROR_			0x1109	/* SET TAG錯誤 */
#define _SDK_ERROR_CODE_SET_HDPT_TAG_ERROR_			0x110A	/* GET TAG錯誤 */
#define _SDK_ERROR_CODE_GET_TMSFTP_TAG_ERROR_			0x110B	/* SET TAG錯誤 */
#define _SDK_ERROR_CODE_SET_TMSFTP_TAG_ERROR_			0x110C	/* GET TAG錯誤 */
#define _SDK_ERROR_CODE_GET_CFGT_TAG_ERROR_			0x110C	/* SET TAG錯誤 */
#define _SDK_ERROR_CODE_SET_CFGT_TAG_ERROR_			0x110D	/* GET TAG錯誤 */

#define _SDK_ERROR_CODE_CARD_UNPACK_FAIL_			0x2001	/* 分析卡片資料失敗 */
#define _SDK_ERROR_CODE_CARD_GET_BIN_				0x2002	/* */
#define _SDK_ERROR_CODE_CARD_MODULE_10_ERROR_			0x2003	/* Module 10檢核錯誤 */
#define _SDK_ERROR_CODE_CARD_EXP_ERROR_				0x2004	/* 有效期錯誤 */
#define _SDK_ERROR_CODE_CARD_PAN_LEN_ERROR_			0X2005	/* 卡號長度錯誤 */
#define _SDK_ERROR_CODE_EXPDATE_TRANSFORM_FAIL_			0X2006	/* 有效期轉換失敗 */
#define _SDK_ERROR_CODE_SWIPE_FAIL_				0x2101	/* 刷卡失敗 */
#define _SDK_ERROR_CODE_SWIPE_EMV_CARD_				0x2102	/* 有晶片界面的卡不能直接刷卡 */
#define _SDK_ERROR_CODE_EMV_SELECT_AID_FAIL_			0x2201	/* 晶片卡SELECT AID失敗 */
#define _SDK_ERROR_CODE_EMV_GET_NO_FAIL_			0x2202	/* 晶片卡取得卡號失敗 */
#define _SDK_ERROR_CODE_CTLS_ERROR_				0x2301	/* 發送感應訊息失敗 */
#define _SDK_ERROR_CODE_CTLS_SEND_FAIL_				0x2302	/* 發送感應訊息失敗 */
#define _SDK_ERROR_CODE_CTLS_MORE_CARDS_			0x2303	/* 同時感應超過一張卡 */
#define _SDK_ERROR_CODE_CTLS_FISC_FAIL_				0x2304	/* 金融卡感應流程錯誤 */

#define _SDK_ERROR_CODE_TXN_ERROR_				0x3001
#define _SDK_ERROR_CODE_TXN_AMOUNT_0_				0x3002	/* 交易金額為0 */
#define _SDK_ERROR_CODE_TXN_COMM_ERROR_				0x3003	/* 通訊失敗 */
#define _SDK_ERROR_CODE_TXN_DECLINE_				0x3004	/* 交易拒絕 */
#define _SDK_ERROR_CODE_TXN_ORGDATE_ERROR_			0x3005	/* 原交易日期有誤 */
#define _SDK_ERROR_CODE_TXN_AUTHCODE_ERROR_			0x3006	/* 授權碼有誤 */
#define _SDK_ERROR_CODE_TXN_PLS_SETTLE_FIRST_			0x3007	/* 請先結帳 */
#define _SDK_ERROR_CODE_TXN_INVOICE_NUMBER_ERROR_		0x3008	/* 調閱編號有誤 */
#define _SDK_ERROR_CODE_TXN_CONNECT_FAIL_			0x3009	/* 連線失敗 */

int OPT_TerminalInitial(void);
int OPT_ReadCard(TRANSACTION_OBJECT *pobTran);
int OPT_PreAuth(TRANSACTION_OBJECT *pobTran);
int OPT_PreAuthComp(TRANSACTION_OBJECT *pobTran);
int OPT_Refund(TRANSACTION_OBJECT *pobTran);
int OPT_Settle(TRANSACTION_OBJECT *pobTran);
int OPT_TMS_DOWNLOAD(TRANSACTION_OBJECT *pobTran);
int OPT_TMS_DOWNLOAD_FTPS(TRANSACTION_OBJECT *pobTran);
int OPT_WRITE_NCCC_TEST_KEY(void);					/* 連NCCC測試主機用，正式機請不要使用 */
int OPT_WRITE_NCCC_TEST_KEY_SELF_TRANS(void);
int OPT_WRITE_NCCC_TMK_BY_KEY_CARD(void);				
int OPT_GET_NEXSYS_SDK_VERSION(char* szVersion);			/* (20)*/
int OPT_GET_TMS_IP(char* szTMS_IP);					/* (15)*/
int OPT_SET_TMS_IP(char* szTMS_IP);					/* (15)*/
int OPT_GET_TMS_PORT(char* szTMS_PORT);					/* (5)*/
int OPT_SET_TMS_PORT(char* szTMS_PORT);					/* (5)*/
int OPT_GET_FTP_IP(char* szFTP_IP);					/* (15)*/
int OPT_SET_FTP_IP(char* szFTP_IP);					/* (15)*/
int OPT_GET_FTP_PORT(char* szFTP_PORT);					/* (5)*/
int OPT_SET_FTP_PORT(char* szFTP_PORT);					/* (5)*/
int OPT_GET_HOST_IP(char* szHOST_IP);					/* (15)*/
int OPT_SET_HOST_IP(char* szHOST_IP);					/* (15)，只用於測試時使用*/
int OPT_GET_HOST_PORT(char* szHOST_PORT);				/* (5)*/
int OPT_SET_HOST_PORT(char* szHOST_PORT);				/* (5)，只用於測試時使用*/
int OPT_GET_EDC_IP(char* szEDC_IP);					/* (15)*/
int OPT_SET_EDC_IP(char* szEDC_IP);					/* (15)*/
int OPT_GET_TID(char* szTID);						/* (8)*/
int OPT_SET_TID(char* szTID);						/* (8)*/
int OPT_GET_MID(char* szMID);						/* (15)*/
int OPT_SET_MID(char* szMID);						/* (15)*/
int OPT_GET_BATCH_NUMBER(char *szBatchNum);				/* (6) */
int OPT_GET_TMS_OK(char *szTMSOK);					/* (2) */
int OPT_SET_COMM_MODE_ETHERNET(void);
int OPT_SET_COMM_MODE_ETHERNET_IFES(void);				/* IP設為IFES使用 */
int OPT_DEBUG_ON(void);							/* 開啟Debug Log */
int OPT_DEBUG_OFF(void);						/* 關閉Debug Log */
int OPT_DEMO_MODE_ON(void);						/* 開啟DEMO Mode */
int OPT_DEMO_MODE_OFF(void);						/* 關閉DEMO Mode */
int OPT_READ_DETAIL_RECORD_START(TRANSACTION_OBJECT *pobTran);
int OPT_READ_DETAIL_RECORD_READ(TRANSACTION_OBJECT *pobTran, int inCnt);
int OPT_READ_DETAIL_RECORD_END(TRANSACTION_OBJECT *pobTran);
int OPT_GET_BATCH_RECORD_COUNT(TRANSACTION_OBJECT *pobTran, int *inCnt);
int OPT_GET_ACCUM_RECORD(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec);
int OPT_EXPIRE_DATE_TRANSFORM_TO_CHECK_NO(TRANSACTION_OBJECT *pobTran, char *szOutput);		/* 需要卡號和有效期 */
int OPT_PAN_TRANSFORM_TO_HASH_NO(TRANSACTION_OBJECT *pobTran, char *szOutput);			/* (50) 電子票加密卡號，buffer須50位 */
int OPT_CARD_EXIST(char* szExist);								/* (2) 回傳'Y'or'N'*/
int OPT_SET_ENCRYPTMODE_NONE_FOR_TEST(void);							/* 關閉加密模式 只用於測試時使用 */
int OPT_DEL_ALL_TRANS(void);									/* 清除批次，會砍掉帳務，請謹慎使用 */
int OPT_CHECK_TMS_FILE(int inCnt, char *szFileName, char *szResult);				/* inCnt:從0開始 szFileName:讀到的檔案名稱(61) szResult:結果(Y/N)(2)*/
int OPT_GET_MIFARE_CARD_NO(char *szCardNO);							/* (32)填入的陣列請預留32 bytes */
int OPT_POS_PRINTER_DEBUG_ON(void);
int OPT_POS_PRINTER_DEBUG_OFF(void);
int OPT_LOAD_FILE(void);
int OPT_SET_CASTLE_EMVCL_DEBUG(unsigned char uszSwitch, unsigned char uszPort);
int OPT_EXAM_FLOW_ON(void);
int OPT_EXAM_FLOW_OFF(void);
int inNexsysSDK_GetCardNO_CTLS(TRANSACTION_OBJECT *pobTran);
int inNexsysSDK_InitCommDevice(TRANSACTION_OBJECT *pobTran);
int inNexsysSDK_ErrorMsg_Transform(TRANSACTION_OBJECT *pobTran, int *inSDKErrorMsg);
int inNexsysSDK_SelfTest(void);
int inNexsysSDK_CardDisplayTest(TRANSACTION_OBJECT *pobTran);
int inNexsysSDK_IP_Before_Test(void);
int inNexsysSDK_IP_After_Test(void);
int inNexsysSDK_TMS_Download(TRANSACTION_OBJECT *pobTran);
int inNexsysSDK_Set_TMSOK_Flow(void);
