	#ifndef __ICERAPI_H__
#define __ICERAPI_H__

#ifdef __cplusplus
	extern "C" {
#endif

typedef union
{
	unsigned long Value;
	unsigned char Buf[4];
}UnionUlong;

typedef union
{
	long Value;
	unsigned char Buf[8];
} Union8Long;

typedef union
{
	unsigned short Value;
	unsigned char Buf[2];
}UnionUShort;

#define MAX_XML_FILE_SIZE		1024 * 10
#define MAX_IP_SIZE				128

typedef struct
{
	char chTMLocationID[6 + 1];
	char chTMID[2 + 1];
	char chTMAgentNumber[4 + 1];
	char chRFU[23];
}STRUCT_API_1_INPUT_DATA;

typedef struct
{
	char chTMLocationID[10 + 1];
	char chTMID[2 + 1];
	char chTMAgentNumber[4 + 1];
	char chLocationID[3 + 1];
	char chNewLocationID[5 + 1];
	char chSPID[3 + 1];
	char chNewSPID[8 + 1];
}STRUCT_API_2_INPUT_DATA;

typedef struct
{
	BYTE ucPurseVersionNumber;
	BYTE ucCardID[7];
	BYTE ucPID[8];
	BYTE ucCardType;
	BYTE ucPersonalProfile;
	BYTE ucEV[3];
	BYTE ucExpiryDate[4];
	BYTE anRRN[12 + 3 + 1];
	BYTE ucDeviceID[4];
	BYTE ucCPUDeviceID[6];
}STRUCT_AUTOLOAD_DATA;

typedef struct
{
	//long lnT0400;
	//unsigned long ulT1100;
	long lnT0404;
	unsigned long ulT1101;
	unsigned char ucT4108;
	BYTE bT0200[10 + 1];
	//BYTE bTMProcessCode[6 + 1];
	BYTE bT5503[10 + 1];
	char chOneDayQuotaFlag;
	char chOnceQuotaFlag;
	char chReadPVNFlag;
	char chDiscountType;
}STRUCT_REQ_DATA;

typedef struct
{
	BOOL fAPILoadFlag;
	BOOL fAPIOpenFlag;
	BOOL fCardActionOKFlag;
	BOOL fICERAdviceFlag;
	BOOL fRetryFlag;
	BOOL fTMRetryFlag;
	BOOL fOnlineHostSuccFlag;
	BOOL fCBikeRentalFlag;
	BOOL fTmpFileReaded;
	BOOL fForeignTxnFlag;
	//BOOL fCMASNoReqT1101Flag;
	//UnionUShort unionEV;
	int inTransType;
	int inReaderSW;
	int inISO8583FlagIndex;
	long lnECCEV;
	long lnECCAmt;
	long lnTotalAmt;
	long lnECCAmt1;//一般
	long lnECCAmt2;//代售
	long lnECCAmt3;//代收
	long lnECCAmt4;//酒類
	long lnECCAmt5;//菸類
	long lnDiscountAmt;//折扣金額
	long lnForeignAmt; //外幣交易金額
	unsigned long ulTMSerialNumber;
	unsigned long ulCMASHostSerialNumber;
	unsigned long ulBankSTAN;
	unsigned long ulBankInvoiceNumber;
	char chTMMTI[4 + 1];
	char chTMProcessCode[6 + 1];
	char chAdviceFlag;
	//char chTMSerialNumber[6 + 1];
	char chTxDate[8 + 1];
	char chTxTime[6 + 1];
	char chICERResponseCode[2 + 1];
	char chR6ResponseCode[2 + 1];
	char chICERTMAgentNumber[4 + 1];
	BYTE ucPurseVersionNumber;
	BYTE anCardID[17 + 1];
	BYTE anExpiryDate[14 + 1];
	BYTE anCardEVBeforeTxn[10 + 1];
	BYTE anCardEV[10 + 1];
	BYTE anRRN[12 + 3 + 1];//37,加卡片交易序號3碼
	BYTE anDeviceID[16 + 1];//41
	BYTE anAData[(47 + 30) * 2 + 1];//554801
	BYTE anAData2[(68 + 22) * 2 + 1];//554808
//#ifdef READER_MODE
	//BYTE anQueryData[600 + 1];//554801
//#endif
	BYTE anSETM_STMC_1[600 + 1];//554802
	BYTE anSETM_STMC_2[600 + 1];//554803
	BYTE anSETM_STMC_3[600 + 1];//554806
	BYTE anTLKR_TLRC[300 + 1];//554804
	BYTE anTAVR_TARC[130 + 1];//554805
	BYTE anMAC[16 + 1];
	BYTE anMAC2[16 + 1];
	//BYTE anICER_BATCH[180 + 1];
	BYTE anC_TAC[8 * 2 + 1];
	BYTE anC_TAC2[16 * 2 + 1];
	BYTE anSAMID[8 + 1];
	BYTE anAutoloadFlag[1 + 1];
	BYTE anAutoloadAmount[10 + 1];
	BYTE bTMData[32 + 4 + 8 + 1];//1代32Byte,2代36Byte,加金額 8 Byte
	BYTE bTmp[5 + 1];//保留
	BYTE bECCEVBefore[8 + 1];
	BYTE bCBikeTime[4 + 1];
	char chCMASTxDate[8 + 1];
	char chCMASTxTime[6 + 1];
	unsigned long ulSubLocId;
}STRUCT_TXN_ING_DATA;

typedef struct
{
	int gLogFlag;//1=Log On / 0=Log Off
	int gDLLVersion;//1=一代API / 2=二代API
	int gReaderMode;// 0=DLL_READER / 1=RS232_READER
	int gDLLMode;// 0=Message / 1=File
	int gTCPIPTimeOut;//主機連線TimeOut秒數
	int gLogCnt;//Log保留個數(一天一個)
	int gComPort;//Log保留個數(一天一個)
	BYTE bComPort[20];//For Linux Reader版專用
	int inMustSettleDate;
	unsigned long ulBaudRate;
	unsigned long ulMaxALAmt;//預設自動加值最大值
	char chICERDataFlag;//ICERData Folder Flag,0=No Folder / 1=Have Folder
	char chOnlineFlag;//Online Flag,0=R6 / 1=CMAS
	char chBatchFlag;//Batch Flag,0=CMAS Batch / 1=SIS2 Batch / 2=R6
	char chAutoLoadMode;//AutoLoad Mode,0=CMAS Batch / 1=SIS2 Batch / 2=R6
	char chGroupFlag;//Group Flag,0=Not Group SAM / 1=Group SAM
	char chTCPIP_SSL;//TCPIP_SSL,0=TCPIP / 1=SSL
	char chEchoTest;//TCPIP_SSL,0=TCPIP / 1=SSL
	BYTE bECCIP[MAX_IP_SIZE];
	unsigned short usECCPort;
	BYTE bICERIP[MAX_IP_SIZE];
	unsigned short usICERPort;
	BYTE bCMASIP[MAX_IP_SIZE];
	unsigned short usCMASPort;
	char chDebugMode;
	BYTE bSlot[2 + 1];//SAM Slot
	char chOpenComMode[1 + 1];
	BYTE bMsgHeader[8 + 1];//Message Header
	BYTE bDeviceInfo[20 + 1];//Device Info(鎖卡回饋檔使用)
	BYTE bBLCName[30 + 1];//黑名單檔名
	char chCMASAdviceVerify;
	char chAutoLoadFunction;//0x30=扣款無自動加值功能 / 0x31=扣款餘額不足時,有自動加值功能
	char chReSendReaderAVR;
	char chXMLHeaderFlag;
	char chFolderCreatFlag;
	char chCMASMode;
	char chPOS_ID[30 + 1];
	char chPOS_ID2[30 + 1];
	union
	{
		STRUCT_API_1_INPUT_DATA srAPIInData1;
		STRUCT_API_2_INPUT_DATA srAPIInData2;
	}APIDefaultData;
	int inAdditionalTcpipData;//附加額外的TCPIP資料
	char PacketLenFlag;//PacketLenFlag 0=電文沒有2Byte長度 / 1=電文有2Byte長度
	BYTE bCRT_FileName[30 + 1];//SSL憑證檔名
	BYTE bKey_FileName[30 + 1];//SSL金鑰檔名
	char ICERFlowDebug;//ICERFlowFlag 0=流程訊息不記錄 / 1=流程訊息要記錄
	char chBLCFlag;
	char chAdviceFlag;
	char chReaderPortocol;
	char chAccFreeRidesMode;
	char chETxnSignOnMode;
	char chCloseAntenna;
	char chReaderUartDebug;
	BYTE bMerchantID[20 + 1];
	//char chICERQRTxn;
	BYTE bICERKey[100 + 1];
	BYTE bRS232Parameter[10 + 1];
	char chSignOnMode;
	char chReadAccPointsFlag;
	char chReadAccPointsMode;
	char chReadDateOfFirstTransFlag;
	char chCommandMode;
	char chReadBasicVersion;
	char chMultiDeductFlag;
	char chBatchDataFlag;
	char chNegativeZeroDeduct;
	char chETxnBatchFlag;//Batch Flag,0=CMAS Batch / 1=SIS2 Batch / 2=R6
	char chBasicCheckQuotaFlag;
	BYTE bCRPDataReq[100 + 1];//CustomizrdReaderPorotocolData Request
	BYTE bCRPDataRes[100 + 1];//CustomizrdReaderPorotocolData Response
	char chAVM;
	int gCRPDelay;
	int gICERConnMode;	//@ Connect to ICER via -> 0=VPN, 1=CMAS(internet)
	char chSaveReSendDataForReadBasic;
	int inSetCardValidDate;	//@ 0=不進行展期，1=連線交易時依回覆進行展期
	int gUsbToRs232Chip;    // 0=PL2303HXD / 1=PL2303GC
}STRUCT_TXN_PARA;

typedef struct
{
	STRUCT_REQ_DATA srREQData;
	STRUCT_TXN_PARA srParameter;
	STRUCT_TXN_ING_DATA srIngData;
	STRUCT_AUTOLOAD_DATA srAutoloadData;
}STRUCT_TXN_DATA;

typedef struct
{
	BOOL fProcessing;
	BOOL fHasTxn;
	char chSettleDate[8 + 1];
	BYTE anDeviceID[16 + 1];//41
	BYTE ucDeviceID[4];
	BYTE ucCPUDeviceID[6];
	BYTE ucSPID;
	BYTE ucCPUSPID[3];
	BYTE ucLocationID;
	BYTE ucCPULocationID[2];
	BYTE ucBLCName[20];
	BYTE ucReaderID[4];
	BYTE ucReaderFWVersion[6];
	BYTE ucCPUSAMID[8];
	BYTE ucCMAS_REQ_T1101[6+1];
	BYTE ucAutoload_fail_flag;	//@ 新增Autoload_fail_flag
	//STRUCT_REQ_DATA stREQTmp;
	BYTE ucTMLocationID[10];
	BYTE ucAutoloadData[1024];

}STRUCT_TMP_DATA;

/*
#define TXN_QUERY_POINT					0//查詢交易
#define TXN_SALE						1//一般交易
#define TXN_REFUND						2//一般交易退貨
#define TXN_REDEEM						3//扣點
#define TXN_REDEEM_REFUND				4//扣點退貨
#define TXN_ADJECT_CARD					5//指定加值
#define TXN_GIFT						6//來店禮
#define TXN_ADD_POINT					7//加點
#define TXN_SETTLE						8//結帳
#define TXN_ECC_DEDUCT					9//悠遊扣款卡交易
#define TXN_ECC_REFUND					10//悠遊退貨卡交易
#define TXN_ECC_ADD						11//悠遊卡加值交易
#define TXN_ECC_VOID					12//悠遊卡加值取消交易
#define TXN_ECC_TX_REFUND				13//悠遊卡餘退交易
#define TXN_ECC_TX_ADD					14//悠遊卡餘加交易
#define TXN_ECC_SET_VALUE				15//悠遊卡展期交易
#define TXN_ECC_AUTOLOAD_ENABLE			16//悠遊卡自動加值開啟
#define TXN_ECC_READ_BASIC				17//悠遊卡查詢交易
#define TXN_ECC_READ_DONGLE				18//悠遊卡查詢1000筆交易
#define TXN_ECC_READ_DEDUCT				19//悠遊卡查詢6筆交易
#define TXN_ECC_READ_CARD				20//悠遊卡讀卡號
#define TXN_END							21
*/
enum
{
	TXN_QUERY_POINT = 0,			//查詢交易
	TXN_SALE,						//一般交易
	TXN_REFUND,						//一般交易退貨
	TXN_REDEEM,						//扣點
	TXN_REDEEM_REFUND,				//扣點退貨
	TXN_ADJECT_CARD, 				//指定加值
	TXN_GIFT,						//來店禮
	TXN_ADD_POINT,					//加點
	TXN_E_READ_BASIC,				//數位付查詢票卡基本資料
	//TXN_SALE2,						//現折交易
	TXN_E_ADD,						//數位付加值交易
	TXN_E_DEDUCT,					//數位付扣款交易
	TXN_E_REFUND,					//數位付退貨交易
	TXN_E_VOID_ADD,					//數位付加值取消交易
	TXN_GIFT_DEDUCT,				//贈品兌換
	TXN_E_RETRY_QUERY,				//數位付Retry查詢
	TXN_E_READ_QR_CODE, 			//數位付查詢QR Code
	TXN_E_QR_ADD,					//數位付QR加值交易
	TXN_E_QR_DEDUCT,				//數位付QR扣款交易
	TXN_E_QR_REFUND,				//數位付QR退貨交易
	TXN_E_QR_VOID_ADD, 				//數位付QR加值取消交易
	TXN_SETTLE,						//結帳
	TXN_ECC_DEDUCT,					//悠遊扣款卡交易
	TXN_ECC_REFUND,					//悠遊退貨卡交易
	TXN_ECC_ADD, 					//悠遊卡加值交易
	TXN_ECC_PT_ADD,                       //點數兌換加值交易(HAMI POINT)
	TXN_ECC_ADD4,					//悠遊卡機具加值交易
	TXN_ECC_VOID,					//悠遊卡加值取消交易
	TXN_ECC_VOID1,					//悠遊卡機具加值取消交易
	TXN_ECC_PENALTY,				//悠遊罰款交易
	TXN_ECC_ADD_OFFLINE,			//悠遊卡加值交易(離線)
	TXN_ECC_VOID_OFFLINE,			//悠遊卡加值取消交易(離線)
	TXN_ECC_ADD_OFFLINE1,			//悠遊卡機具加值交易(離線)
	TXN_ECC_VOID_OFFLINE1,			//悠遊卡機具值取消交易(離線)
	TXN_ECC_SALE_CARD_OFFLINE,		//悠遊卡加值取消交易(離線)
	TXN_ECC_TX_REFUND,				//悠遊卡餘退交易
	TXN_ECC_TX_ADD,					//悠遊卡餘加交易
	TXN_ECC_SET_VALUE,				//悠遊卡展期交易
	TXN_ECC_FARE_SALE,				//悠遊卡售特種票
	TXN_ECC_VOID_FARE,				//悠遊卡取消特種票
	TXN_ECC_REFUND_FARE,			//悠遊卡退特種票
	TXN_ECC_TRTC_SET_VALUE,			//悠遊卡捷運資料修改
	TXN_ECC_REFUND_CARD,        	//悠遊退卡交易
	TXN_ECC_ADD2,					//溢扣返還
	TXN_ECC_DEDUCT_TAXI,			//計程車扣款
	TXN_ECC_DEDUCT_CBIKE,			//CBike扣款
	TXN_ECC_DEDUCT_EDCA,			//EDCA扣款
	TXN_ECC_AUTOLOAD, 				//悠遊卡自動加值
	TXN_ECC_AUTOLOAD_ENABLE, 		//悠遊卡自動加值開啟
	TXN_ECC_SET_VALUE_OFFLINE,  	//悠遊卡展期(離線)
	TXN_ECC_STUDENT_SET_VALUE,		//悠遊卡學生展期
	TXN_ECC_EXTEND_VALID,           //悠遊卡展期交易2
	TXN_ECC_ADD3,					//悠遊卡展期加值交易 -> 二代後台加值
	TXN_ECC_ADD_INQ,                      //悠遊卡後台加值查詢
	TXN_ECC_DEDUCT2,				//捷運代收扣款(代收停車費)
	TXN_ECC_ACCUNT_LINK,			//Accunt Link帳戶連結交易
	TXN_ECC_TEST1,					//離線加值
	TXN_ECC_READ_BASIC,				//悠遊卡查詢交易
	TXN_ECC_READ_DONGLE, 			//悠遊卡查詢1000筆交易
	TXN_ECC_READ_DEDUCT, 			//悠遊卡查詢6筆交易
	TXN_ECC_SIGN_ON,				//悠遊卡SignOn
	TXN_ECC_MATCH,					//悠遊卡連線配對
	TXN_ECC_MATCH_OFF,				//悠遊卡離線配對
	TXN_ECC_RESET_OFF,				//悠遊卡Reset Offline
	TXN_ECC_SIGN_ON_QUERY,			//悠遊卡SignOnQuery
	TXN_ECC_READ_TRTC_DATA,			//悠遊卡讀捷運資料
	TXN_ECC_READ_CARD,				//悠遊卡讀卡號
	TXN_ECC_MULTI_SELECT,			//多家票證尋卡
	TXN_ECC_ANTENN_ACONTROL,		//天線開關
	TXN_ECC_FAST_READ_CARD,			//悠遊卡快速查詢
	TXN_ECC_FAST_READ_CARD2, 		//悠遊卡快速查詢2
	TXN_ECC_READ_DEVICE_INFO,		//悠遊卡設備資訊查詢
	TXN_ECC_READ_TAXI,				//計程車讀取
	TXN_ECC_READ_THSRC,				//高鐵讀取
	TXN_ECC_READ_CBIKE, 			//CBike讀取
	TXN_ECC_READ_EDCA, 				//EDCA讀取
	TXN_ECC_TLRT_AUTH,				//淡海輕軌員工卡片認證
	TXN_ECC_ADVICE, 				//悠遊卡Advice上傳
	TXN_ECC_BATCH_UPLOAD,			//悠遊卡批次上傳
	TXN_ECC_SETTLE2, 				//悠遊卡結帳2
	TXN_ECC_SETTLE, 				//悠遊卡結帳
	TXN_END,
};

enum HOST_TYPE
{
	HOST_R6 = 0,
	HOST_ICER,
	HOST_CMAS,
};

#define	TXN_TOTAL_CNT					TXN_END

#define TM1_DATA_LEN						32
#define TM2_DATA_LEN						36

#define Plus		0x2b
#define Minus		0x2d
#define DECIMAL 	10
#define HEX 		16

#define TLKR_TLRC_LEN					(srTxnData.srParameter.gDLLVersion == 1)?40:197
#define TAVR_TARC_LEN					(srTxnData.srParameter.gDLLVersion == 1)?54:130

#define NETWORK_MANAGEMENT_CODE 	"079"
#define API_VERSION_R6 			"\x40\x06\x02\x00"
#define API_VERSION_CMAS 			"V4006C"

#define LOCATION_ID 		0x01
#define LOCATION_ID_BATCH		"01"

#define OUT_DATA_OFFSET 	3
#define IN_DATA_OFFSET		8

#define DLL_READER			0
#define RS232_READER		1
#define LIBARY_READER		2
#define USB_READER			3
#define USB_TO_RS232_READER	4
#define LIBARY_READER2		5
#define TCPIP_READER		6

#define R6_ONLINE		'0'
#define CMAS_ONLINE		'1'
#define ICER_ONLINE 	'2'

#define R6_BATCH		'0'
#define CMAS_BATCH		'1'
#define SIS2_BATCH		'2'
#define ICER_BATCH		'3'

#define TCPIP_PROTOCOL	'0'
#define SSL_PROTOCOL	'1'

#define SIGN_ON_ONLY			'0'
#define RESET_OFF_ONLY			'1'
#define SIGN_ON_AND_RESET_OFF	'2'
#define NO_SIGN_ON_AND_RESET	'3'

#define DLL_MESSAGE_MODE	0
#define DLL_FILE_MODE		1

#define NO_ADD_DATA		0//No Additional Data
#define NCCC_ADD_DATA	1//NCCC
#define CTCB_ADD_DATA	2//CTCB
#define FISC_ADD_DATA	3//FISC

#define GET_CARD_LEN	(srTxnData.srParameter.gDLLVersion == 1)?10:17
//#define GET_EV_LEN		(srTxnData.srParameter.gDLLVersion == 1)?8:6

#if READER_MANUFACTURERS!=WINDOWS_API && (READER_MANUFACTURERS!=LINUX_API || RDEVN!=NE_X86) && READER_MANUFACTURERS!=ANDROID_API && READER_MANUFACTURERS!=WINDOWS_CE_API
	int ICERApi_exe(void);
#endif

void inFinalizer(void);
int inProcessTxn(long lnProcCode);
BOOL fDLLResponseLock(int inReaderSW);
BOOL fDLLResponseCheck(int inRet);

#ifdef LIB_DEFINE
	int inICERAPI_InitialReader(void);
	int inICERAPI_InitialReader2(char chSlot);
	void vdICERAPI_ReaderMode(int inReaderMode,int inComPort,unsigned long ulBaudrate,BYTE bParity,BYTE bDataBits,BYTE bStopBits,BYTE bContinueFlag,BYTE bOpenFlaged, int inTimeout);
	int inMF_CAMS_Fast_Tap(int inReaderMode);
#endif

#if READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
	#define LOG_FILE_PATTERN "^ICER[[:digit:]]{8}\\.log$"
	#define MAX_LOG_FILE     (30)
	BOOL set_log(const char* pathname);
	void close_log(void);
	void call_stack(const char* fmt, ...);
	int log_filter(const struct dirent* entry);
	void TRACE_(const char* fmt, ...);
	void TRACE0_(const char* msg);
	BOOL ere_matches(const char* str, const char* pattern, BOOL case_sensitive);
	BOOL compile_ere(regex_t* reg,const char* pattern,BOOL case_sensitive,BOOL no_detail);
	const char* const regerror_msg(const regex_t* reg, int err);
#endif

#ifdef __cplusplus
	}
#endif // __cplusplus

#endif
