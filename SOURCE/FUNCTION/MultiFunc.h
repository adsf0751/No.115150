/* 
 * File:   MultiFunc.h
 * Author: user
 *
 * Created on 2017年6月29日, 下午 6:21
 */
/* Verifone的命令前綴(17 Bytes) START */
/* Status Codes 【11 BYTE】
	00h OK
	01h Incorrect Header Tag
	02h Unknown Command
	03h Unknown Sub-Command
	04h CRC Error in Packet
	05h Incorrect Parameter
	06h Parameter Not Supported
	07h Mal-formatted Data
	08h Timeout
	0Ah Failed / Nak
	0Bh Command Not Allowed
	0Ch Sub-Command Not Allowed
	0Dh Buffer Overflow (data length too large for Reader buffer)
	0Eh User Interface Event
	23h Request Online Authorization
 */
/* Error Codes 【14 BYTE】
	00h No Error None.
	02h Go to Contact Interface
	03h Transaction Amount is Zero
	20h Card Returned Error Status
	21h Collision Error
	22h Amount Over Maximum Limit
	23h Request Online Authorization
	25h Card Blocked
	26h Card Expired
	27h Unsupported Card
	30h Card Did Not Respond
	40h Unknown Data Element
	41h Required Data Element(s) Missing
	42h Card Generated AAC
	43h Card Generated ARQC
	44h SDA/DDA Failed (Not Supported by Card)
	50h SDA/DDA/CDDA Failed (CA Public Key)
	51h SDA/DDA/CDDA Failed (Issuer Public Key)
	52h SDA Failed (SSAD)
	53h DDA/CDDA Failed (ICC Public Key)
	54h DDA/CDDA Failed (Dynamic Signature Verification)
	55h Processing Restrictions Failed
	56h Terminal Risk Management (TRM) Failed
	57h Cardholder Verification Failed
	58h Terminal Action Analysis (TAA) Failed
	61h SD Memory Error
*/
/* RF State Codes 【17 BYTE】
	00h None		RF State Code not available
	01h PPSE		Error occurred during PPSE command
	02h SELECT		Error occurred during SELECT command
	03h GPO			Error occurred during GET PROCESSING OPTIONS command
	04h READ RECORD		Error occurred during READ RECORD command
	05h GEN AC 		Error occurred during GEN AC command
	06h CCC 		Error occurred during CCC command
	07h IA			Error occurred during IA command
	08h SDA			Error occurred during SDA command
	09h DDA			Error occurred during DDA command
	0ah CDA			Error occurred during CDA command
	0bh TAA			Error occurred during TAA processing
	0ch UPDATE RECORD	Error occurred during UPDATE RECORD command
	10h GET DATA (Ticket)	Error occurred during GET DATA command to retrieve the Ticket
	11h GET DATA (Ticketing Prof)	Error occurred during GET DATA command to retrieve the Ticketing Profile
	12h GET DATA (Balance)	Error occurred during GET DATA command to retrieve the Balance
	13h GET DATA (All)	Error occurred during GET DATA command to retrieve all data
	20h PUT DATA (Ticket)	Error occurred during PUT DATA command to retrieve the Ticket
*/
/* 以下不一定檢核的我都填0x00 參考BSP520820中的inMultiFunc_CALL_Slave_CTLS_Result */
/* Header Tag and Protocol Version + Command */
//#define _VIVO_HEADER_				"\x56\x69\x56\x4F\x74\x65\x63\x68\x32\x00\x02"
#define _VIVO_HEADER_				"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
#define _VIVO_HEADER_LEN_			11
#define _VIVO_STATUS_CODES_FAIL_		"\x0A"
#define _VIVO_STATUS_CODES_ONLINE_		"\x23"
#define _VIVO_STATUS_CODES_SMARTPAY_		"\x25"		/* 這個判別是因為Smartpay備註欄一定會有0x25 */
#define _VIVO_STATUS_CODES_LEN_			1
/* 下面的要當STATUS_CODES回0x0A才會有 */
#define _VIVO_DATA_LENGTH_1_			"\x00\x00"
#define _VIVO_DATA_LENGTH_1_LEN_		2
#define _VIVO_ERROR_CODES_CTLS_INTERFACE_	"\x00"
#define _VIVO_ERROR_CODES_LEN_			1
#define _VIVO_SW1SW2_				"\x00\x00"
#define _VIVO_SW1SW2_LEN_			2
#define _VIVO_RF_STATE_CODES_NONE_		"\x00"
#define _VIVO_RF_STATE_CODES_LEN_		1
/* Verifone的命令前綴 END */

#define _STX_					0x02
#define _ETX_					0x03
#define _ACK_					0x06
#define _NAK_					0x15
#define _FS_					0x1C

#define _SLAVE_ECR_30_Standard_Data_Size_         30
/* Multi START */
#define _MULTI_RECV_SIZE_ 	1023    /* 接收的size */
#define _MULTI_MAX_SIZES_	1024    /* 封包最大數 */
#define _MULTI_SUB_SIZE_MAX_	990
#define _MULTI_SUB_SIZE_SMALL_	100
#define _MULTI_SUB_SIZE_MIDDLE_	200
#define _MULTI_SUB_SIZE_NONE_	0

#define _MULTI_PIN_             "01"
#define _MULTI_CTLS_            "02"
#define _MULTI_SIGNPAD_         "03"
#define _MULTI_SIGNPAD_API_     "05"
#define _MULTI_SIGN_CONFIRM_    "06"
#define _MULTI_TMS_CAPK_        "11"
#define _MULTI_TMS_MASTER_      "12"
#define _MULTI_TMS_VISA_        "13"
#define _MULTI_TMS_JCB_         "14"
#define _MULTI_TMS_CUP_         "15"/*20151014浩瑋新增*/
#define _MULTI_SLAVE_REBOOT_    "19"
#define _MULTI_POLL_            "20"
#define _MULTI_EXCHANGE_        "21"
#define _MULTI_NOSIGN_          "22"

#define _MULTI_PIN_NO_          1
#define _MULTI_CTLS_NO_         2
#define _MULTI_SIGNPAD_NO_      3
#define _MULTI_SIGNPAD_API_NO_  5
#define _MULTI_SIGN_CONFIRM_NO_ 6
#define _MULTI_TMS_CAPK_NO_     11
#define _MULTI_TMS_MASTER_NO_   12
#define _MULTI_TMS_VISA_NO_     13
#define _MULTI_TMS_JCB_NO_      14
#define _MULTI_TMS_CUP_NO_      15/*20151014浩瑋新增*/
#define _MULTI_SLAVE_REBOOT_NO_ 19
#define _MULTI_POLL_NO_         20
#define _MULTI_EXCHANGE_NO_     21
#define _MULTI_NOSIGN_NO_       22

/* Multi END */

#define _MULTIFUNC_PINKEY_VALUE_	"30303030303030313030303030303031"
#define _MULTIFUNC_PINKEY_VALUE_LEN_	32

/* 合庫520 transaction code */
#define _520_MULTIFUNC_SALE_			1
#define _520_MULTIFUNC_REFUND_			16
#define _520_MULTIFUNC_FISC2_PURCHASE_		694
#define _520_MULTIFUNC_FISC2_BALANCE_INQ_	695
#define	_520_MULTIFUNC_NP_INQ_BLANCE_TRNAS_	215
#define	_520_MULTIFUNC_FISC2_FUND_TRANSFER_	693
#define	_520_MULTIFUNC_FISC2_REFUND_NOTIFY_	683
#define	_520_MULTIFUNC_FISC2_REFUND_TRANSFER_	681
#define	_520_MULTIFUNC_NP_NATIONAL_PAY_		201
#define _520_MULTIFUNC_NP_NORMAL_PAY_		206


typedef struct
{
	int     inTotalPacketNum;
	int     inSubPacketNum;
	int     inSubPacketSize;
	int     inErrorType;
	int     inCTLS_Timeout;
	long    lnTotalPacketSize;

	char 	szTransType[2 + 1];
	char    szTranHost[4 + 1];
	char 	szSendData[_MULTI_MAX_SIZES_ + 1];
	char 	szReceData[_MULTI_MAX_SIZES_ + 1];
	char    szErrorCode[4 + 1];
	char    szKeyIndex[2 + 1];
	char    szKeyValue[32 + 1];
	char	szTermSN[6 + 1];
}MULTIFUC_SLAVE_TRANSACTION_DATA;

typedef struct
{
	int					inACKTimeOut; 
	int					inMaxRetries;
	unsigned char				uszPadStxEtx; 
	unsigned char				uszWaitForAck;
}MULTIFUC_SLAVE_OPTIONAL_SETTING;

/* Multi START */
typedef struct
{
	ECR_SETTING				srSetting;			/* initial完不會動的 */
	MULTIFUC_SLAVE_TRANSACTION_DATA		stMulti_TransData;
	MULTIFUC_SLAVE_OPTIONAL_SETTING		stMulti_Optional_Setting;
} MULTI_TABLE;
/* Multi END */

/* Multi START */
typedef struct
{
	int (*inMultiInitial)(MULTI_TABLE *);
	int (*inMultiRece)(TRANSACTION_OBJECT *, MULTI_TABLE *);
	int (*inMultiSend)(TRANSACTION_OBJECT *, MULTI_TABLE *);
	int (*inMultiSendError)(TRANSACTION_OBJECT *, MULTI_TABLE *);
	int (*inMultiEnd)(MULTI_TABLE *);
} MULTI_TRANS_TABLE;
/* Multi END */

/* Multi Test */
typedef struct
{
	char	szBank[4 + 1];
	char	szEDCSN[6 + 1];
	char	szResponseCode[4 + 1];
	char	szTransType[2 + 1];
	char	szTotalPacketIndex[2 + 1];
	char	szSubPacketIndex[2 + 1];
	char	szTotalPacketSize[6 + 1];
	char	szSubPacketSize[4 + 1];
}MULTI_TEST_STRUCTURE;
/* Multi Test End*/

int inMultiFunc_First_Initial(void);
int inMultiFunc_First_Receive_Check(void);
int inMultiFunc_First_Receive_Cancel(void);
/* MultiFunc 介接RS232 START */
int inMultiFunc_RS232_Initial(void);
int inMultiFunc_RS232_FlushRxBuffer(void);
int inMultiFunc_RS232_Send(TRANSACTION_OBJECT *pobTran, MULTI_TABLE * stMultiOb, char *szDataBuffer, int inDataSize);
int inMultiFunc_RS232_Send_ACKorNAK (MULTI_TABLE *stMultiOb, int inAckorNak);
int inMultiFunc_RS232_Receive_ACKandNAK(MULTI_TABLE *stMultiOb);
int inMultiFunc_RS232_Slave_GetCancel(unsigned char inHandle, MULTI_TABLE* stMultiFuncOb);
/* MultiFunc 介接 END */

int inMultiFunc_Receive_Command(TRANSACTION_OBJECT *pobTran);
int inMultiFunc_SendResult(TRANSACTION_OBJECT *pobTran);
int inMultiFunc_SendError(TRANSACTION_OBJECT * pobTran, int inErrorType);
int inMultiFunc_SlaveInitial(MULTI_TABLE *stMultiOb);
int inMultiFunc_SlaveRecePacket(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *stMultiOb);
int inMultiFunc_SlaveSendPacket(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *stMultiOb);
int inMultiFunc_SlaveSendError(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *stMultiOb);
int inMultiFunc_SlaveEnd(MULTI_TABLE* stMultiob);
int inMultiFunc_SlaveData_Recv(unsigned char inHandle, int inRespTimeOut, char *szGetMultiData, unsigned char uszSendAck, MULTI_TABLE* stMultiFuncOb);
int inMultiFunc_SlaveData_Unpack_Header(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, MULTI_TABLE *stMultiOb);

int inMultiFunc_SlaveRecePacket_Signpad(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *stMultiOb);
int inMultiFunc_SlaveReceComPortData(TRANSACTION_OBJECT *pobTran);
int inMultiFunc_SendError_Slave(TRANSACTION_OBJECT * pobTran, int inErrorType);
int inMultiFunc_SlavePackResult(TRANSACTION_OBJECT *pobTran, char *szPackData, MULTI_TABLE * stMultiFuncOb);
int inMultiFunc_SlaveData_Send(unsigned char inHandle, char *szMsgBuffer, int inMsgSize, unsigned char uszPadStxEtx, int inACKTimeOut, unsigned char uszWaitForAck, int inMaxRetries);

/* 下面是工具用function，或MultiFun用流程(和V3 CounterTop完全獨立) */
int inMultiFunc_TimeCheck(char* szDate, char* szTime);
int inMultiFunc_Smartpay_data(TRANSACTION_OBJECT *pobTran);
int inMultiFunc_ValidDate(char *szDate);
int inMultiFunc_ValidTime(char *szTime);
int inMultiFunc_Response_Host(TRANSACTION_OBJECT *pobTran, MULTI_TABLE* srMultiTable);
int inMultiFunc_GetCardFields_CTLS(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *stMultiOb);
int inMultiFunc_GetCardFields_Refund_CTLS(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *stMultiOb);
unsigned long ulMultiFunc_CheckResponseCode_SALE(TRANSACTION_OBJECT *pobTran);
unsigned long ulMultiFunc_CheckResponseCode_REFUND(TRANSACTION_OBJECT *pobTran);
int inMultiFunc_Process_VIVO_Header(char* szData);
int inMultiFunc_Process_CTLS_DATA(void);
int inMultiFunc_Process_CTLS_ERROR_DATA(void);
int inMultiFunc_Process_SmartPay_Data(TRANSACTION_OBJECT *pobTran);
int inMultiFunc_FISC_CTLSProcess(TRANSACTION_OBJECT *pobTran);
int inMultiFunc_FISC_Flow_SmartPayPayment(TRANSACTION_OBJECT *pobTran);
int inMultiFunc_FISC_Flow_MobileMicroPayment(TRANSACTION_OBJECT *pobTran);
int inMultiFunc_FISC_Flow_MobileConditionPayment(TRANSACTION_OBJECT *pobTran);
int inMultiFunc_FISC_Flow_MobileHighValuePayment(TRANSACTION_OBJECT *pobTran);
int inMultiFunc_FISC_CARD_SelectEF(TRANSACTION_OBJECT *pobTran);
int inMultiFunc_FISC_ReadRecords(TRANSACTION_OBJECT *pobTran);
int inMultiFunc_ReadRecords_1004(TRANSACTION_OBJECT *pobTran);
int inMultiFunc_FISC_WriteRecords(TRANSACTION_OBJECT *pobTran);
int inMultiFunc_FISC_WriteTxnAuthReqres(TRANSACTION_OBJECT *pobTran);
int inMultiFunc_EnterPin(TRANSACTION_OBJECT *pobTran, char *szPinData);
int inMultiFunc_Write_PINKey(void);
int inMultiFunc_3DES_Encrypt(char* szInPlaindata, int inInPlaindataLen, char *szResult);
int inMultiFunc_GetCardFields_CTLS_Test(void);
