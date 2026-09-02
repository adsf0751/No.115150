/* Define Card Status */
#define _NO_CARD_                                       0
#define _CARD_PRESENT_                                  1
#define _CARD_REMOVE_                                   8

#define _FISC_ISSURE_ID_SIZE_                           8
#define _FISC_ACCOUNT_SIZE_                             16
#define _FISC_CARD_COMMET_SIZE_                         30

#define _SELECT_EF_SIZE_                                2
#define _WRITE_RECORD_SIZE_                             100
#define _SELECT_TIMEOUT_                                120
#define _FISC_AID_                                      "\xA0\x00\x00\x01\x72\x95\x00\x01"
#define _FISC_PIN_TIMEOUT_                              120

/* FISC - 定義 Smart Pay Parameter Sizes (START) */
#define _FISC_ISSUER_ID_SIZE_                           8
#define _FISC_CARD_COMMET_SIZE_                         30
#define _FISC_ACCOUNT_SIZE_                             16
#define _FISC_STAN_SIZE_                        	8
#define _FISC_TAC_SIZE_                                 32
#define _FISC_PIN_SIZE_                                 16
#define _FISC_PIN_BCD_SIZE_                             8
#define _FISC_DATE_AND_TIME_SIZE_               	14
#define _FISC_RRN_SIZE_                                 12
#define _FISC_REFUND_DATE_                              8
#define _FISC_TD_SIZE_                                  4
#define _FISC_MCC_SIZE_                                 15
#define _FISC_TCC_SIZE_                                 8
/* FISC - 定義 Smart Pay Parameter Sizes (END) */

/* 金融卡載具分類 */
#define _FISC_PAY_DEVICE_SMARTPAY_CARD_			"\x01"	/* 實體卡 */
#define _FISC_PAY_DEVICE_MOBILE_			"\x02"	/* 手機 */

/* 行動金融卡分類 */
#define _FISC_MOBILE_NFTYPE_WITHOUT_ENTER_PIN_		"\x00"	/* 行動金融卡且不須輸入密碼 */
#define _FISC_MOBILE_NFTYPE_CONDITION_ENTER_PIN_	"\x01"	/* 行動金融卡且視情況輸入密碼 */
#define _FISC_MOBILE_NFTYPE_MANDATORY_ENTER_PIN_	"\x02"	/* 行動金融卡且一定要輸入密碼 */

/* 交易裝置判別 只分為雲端卡片和非雲端卡片 */
#define _FISC_MOBILE_DEVICE_CLOUD_TXN_			"\x05"

/* 判別支援近端的交易類型設定 */
#define _FISC_MOBILE_NFSETTING_SINGLE_ISSUER_WALLET_	"\x00"
#define _FISC_MOBILE_NFSETTING_THIRD_PARTY_WALLET_	"\x01"
#define _FISC_MOBILE_NFSETTING_RESERVED_		"\x02"
#define _FISC_MOBILE_NFSETTING_OTHERS_			"\x0A"

typedef struct
{
	int	inSelectAccountLen;
	char	szSelectAccount[16 + 1];
} SELECT_ACCOUNT_OBJECT;

int inFISC_SelectAID(TRANSACTION_OBJECT *pobTran);
int inFISC_PowerON(TRANSACTION_OBJECT *pobTran);
int inFISC_PowerOFF(TRANSACTION_OBJECT *pobTran);
int inFISC_Select_Menu(TRANSACTION_OBJECT *pobTran);
int inFISC_CardProcess(TRANSACTION_OBJECT *pobTran);
int inFISC_CARD_SelectEF(TRANSACTION_OBJECT *pobTran);
int inFISC_ReadRecords(TRANSACTION_OBJECT *pobTran);
int inFISC_ReadRecords_1004(TRANSACTION_OBJECT *pobTran);
int inFISC_ReadTxnAuthReqres(TRANSACTION_OBJECT *pobTran);
int inFISC_SelectAccountMenu(TRANSACTION_OBJECT *pobTran, BYTE *uszReceBuf);

int inFISC_VerifyPin(TRANSACTION_OBJECT *pobTran);
int inFISC_GetUserPin(TRANSACTION_OBJECT *pobTran);
int inFISC_WriteRecords(TRANSACTION_OBJECT *pobTran);
int inFISC_CheckAccountData(char *szAccountData, int inCheckLen);
int inFISC_EnterPin(char *szPinData);
int inFISC_GenMACAndTCC(TRANSACTION_OBJECT *pobTran);

int inFISC_CheckSmartPayEnable(void);
int inFISC_Check_Mobile(TRANSACTION_OBJECT *pobTran);

int inFISC_CTLSProcess(TRANSACTION_OBJECT *pobTran);
int inFISC_SelectAID_CTLS(TRANSACTION_OBJECT * pobTran);
int inFISC_CTLS_LED_TONE(int inResult);
int inFISC_TwoTap_Wait_Second_Approach(TRANSACTION_OBJECT *pobTran);
int inFISC_Read_Card_Data_Flow(TRANSACTION_OBJECT *pobTran);
int inFISC_Power_On_And_Select_AID(TRANSACTION_OBJECT *pobTran);

/* 流程 */
int inFISC_Flow_SmartPayPayment(TRANSACTION_OBJECT *pobTran);
int inFISC_Flow_MobileMicroPayment(TRANSACTION_OBJECT *pobTran);
int inFISC_Flow_MobileHighValuePayment(TRANSACTION_OBJECT *pobTran);
