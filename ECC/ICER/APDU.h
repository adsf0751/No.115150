#ifndef __APDU_H__
#define __APDU_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include "SubStruct.h"
#include "XMLFunc.h"

#if 0
#define API_START						0//0~3
#define API_PPR_RESET					1//4~7
#define API_PPR_SIGNON					2//8~11
#define API_PPR_SIGNONQUERY 			3//12~15
#define API_PPR_READCARDBASICDATA		4//16~19
#define API_PPR_TXNREQONLINE			5//20~23
#define API_PPR_AUTHTXNONLINE			6//24~27
#define API_PPR_TXNREQOFFLINE			7//28~31
#define API_PPR_AUTHTXNOFFLINE			8//32~35
#define API_PPR_LOCKCARD2				9//36~39
#define API_PPR_READDONGLEDEDUCT		10//40~43
#define API_PPR_CARDSALE				11//44~47
#define API_PPR_AUTHCARDSALE			12//48~51
#define API_PPR_CANCELTXN				13//52~55
#define API_PR_SETVALUE 				14//56~59
#define API_PR_AUTHSETVALUE 			15//60~63
#define API_PR_TXREFUND 				16//64~67
#define API_PR_AUTHTXREFUND 			17//68~71
#define API_PPR_TXADDVALUE				18//72~75
#define API_PPR_AUTHTXADDVALUE			19//76~79
#define API_PR_AUTOLOADENABLE			20//80~83
#define API_PR_AUTHAUTOLOADENABLE		21//84~87
#define API_PPR_ADDVALUE				22//88~91
#define API_PPR_AUTHADDVALUE			23//92~95
#define API_PPR_READCARDDEDUCT			24//96~99
#define API_PPR_READCODEVERSION 		25//100~103
#define API_PPR_REBOOT					26//104~107
#define API_PPR_LOCKCARD1				27//108~111
#define API_PPR_SETVALUE				28//112~115
#define API_PPR_AUTHSETVALUE			29//116~119
#define API_PPR_TXREFUND				30//120~123
#define API_PPR_AUTHTXREFUND			31//124~127
#define API_PPR_AUTOLOADENABLE			32//128~131
#define API_PPR_AUTHAUTOLOADENABLE		33//132~135
#define API_PPR_AUTHAUTOLOAD			34//136~139
#define API_PPR_READCARDNUMBER			35//140~143
#define API_END 						36
#endif

/********************************Reset********************************/
typedef struct __packed_back {
	BYTE ucTMLocationID[10];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE ucTxnDateTime[4];
	BYTE ucLocationID;
	BYTE ucCPULocationID[2];
	BYTE ucSPID;
	BYTE ucCPUSPID[3];
	BYTE ucRFU1[5];
	BYTE ucSAMSlot[1];
	BYTE ucRFU2[1];
#ifdef ECC_BIG_ENDIAN
	unsigned char RFU_Flag:4;
	unsigned char antenna_LED_flag:1;
	unsigned char MAC_SPID_Mode:1;
	unsigned char ecc_debug_flag:1;
	unsigned char AccumulatedFreeRidesMode:1;
#else
	unsigned char AccumulatedFreeRidesMode:1;
	unsigned char ecc_debug_flag:1;
	unsigned char MAC_SPID_Mode:1;
	unsigned char antenna_LED_flag:1;
	unsigned char RFU_Flag:4;
#endif
	//BYTE ucAccFreeRidesMode[1];
	BYTE ucRFU3[9];
}Reset_APDU_In;

typedef struct __packed_back{
	BYTE ucHostSpecVersionNo;
	BYTE ucReaderID[4];
	BYTE ucReaderFWVersion[6];
	BYTE ucSAMID[8];
	BYTE ucSAMSN[4];
	BYTE ucSAMCRN[8];
	BYTE ucDeviceID[4];
	BYTE ucSAMKeyVersion;
	BYTE ucSTAC[8];
	BYTE ucSAMVersion;
	BYTE ucCPUSAMID[8];
	BYTE ucSAMUsageControl[3];
	BYTE ucSAMAdminKVN;
	BYTE ucSAMIssuerKVN;
	BYTE ucAuthCreditLimit[3];
	BYTE ucSingleCreditTxnAmtLimit[3];
	BYTE ucAuthCreditBalance[3];
	BYTE ucAuthCreditCumulative[3];
	BYTE ucAuthCancelCreditCumulative[3];
	BYTE ucCPUDeviceID[6];
	BYTE ucTagListTable[40];
	BYTE ucSAMIssuerSpecData[32];
	BYTE ucSTC[4];
	BYTE ucRSAM[8];
	BYTE ucRHOST[8];
	BYTE ucSATOKEN[16];
	SAMParameter_t stSAMParameterInfo_t;//12
	BYTE ucRemainderAddQuota[3];
	BYTE ucCancelCreditQuota[3];
	BYTE ucDeMAC[8];
	BYTE ucLastTxnDateTime[4];
	LastSingOn_t stLastSignOnInfo_t;//33
	BYTE ucStatusCode[2];
}Reset_APDU_Out;
/****************************************************************/

/*****************************SignOn***********************************/
typedef struct __packed_back{
	BYTE ucHTAC[8];
	BYTE ucHATOKEN[16];
	SAMParaSetting_t stSAMParaSettingInfo_t;//57
	SAMParameter_t stSAMParameterInfo_t;//12
	BYTE ucRFU[31];
	BYTE ucHashType;
	BYTE ucCPUEDC[3];
}SignOn_APDU_In;

typedef struct __packed_back{
	BYTE ucCreditBalanceChangeFlag;
	BYTE ucAuthCreditLimit[3];
	BYTE ucAuthCreditBalance[3];
	BYTE ucAuthCreditCumulative[3];
	BYTE ucAuthCancelCreditCumulative[3];
	BYTE ucCACrypto[16];
	BYTE ucStatusCode[2];
}SignOn_APDU_Out;
/****************************************************************/

/*****************************SignOnQuery******************************/

typedef struct __packed_back{
	BYTE ucAuthCreditLimit[3];
	BYTE ucAuthCreditBalance[3];
	BYTE ucAuthCreditCumulative[3];
	BYTE ucAuthCancelCreditCumulative[3];
	SAMParameter_t stSAMParameterInfo_t;//12
	BYTE ucLastTxnDateTime[4];
	BYTE ucSPID;
	BYTE ucCPUSPID[3];
	BYTE ucRFU[2];
	BYTE ucRemainderAddQuota[3];
	BYTE ucCancelCreditQuota[3];
	BYTE ucStatusCode[2];
}SignOnQuery_APDU_Out;
/****************************************************************/

/**************************ReadCardBasicData***************************/
typedef struct __packed_back{
	BYTE ucLCDControlFlag;
	BYTE ucTMSerialNumber[ 6 ];								// TM 終端機(TM)交易序號(取消交易時補0x00)
	BYTE ucTxnDateTime[ 4 ]; 								// 交易日期時間(Unsigned and LSB First) (UnixDateTime)
#ifdef ECC_BIG_ENDIAN
	BYTE RFU_Flag:6;
	BYTE OnceQuotaCheckFlag:1; 						// Bit 0：次限額檢查旗標	 0:不檢查次限額 1:檢查次限額
	BYTE OneDayQuotaCheckFlag:1;						// Bit 0：日限額檢查旗標	 0:不檢查日限額 1:檢查日限額
#else
	BYTE OneDayQuotaCheckFlag:1;						// Bit 0：日限額檢查旗標	 0:不檢查日限額 1:檢查日限額
	BYTE OnceQuotaCheckFlag:1; 						// Bit 0：次限額檢查旗標	 0:不檢查次限額 1:檢查次限額
	BYTE RFU_Flag:6;
#endif
	BYTE ucTxnAmt[ 3 ];										// TM 交易金額(Signed and LSB First)(取消交易、退卡、開卡時補0x00)
	BYTE RFU[ 1 ]; 									// TM 保留，補0x00，8bytes
}ReadCardBasicData_APDU_In;

typedef struct __packed_back{
	BYTE ucPurseVersionNumber;
#ifdef ECC_BIG_ENDIAN
	BYTE bRFU7			:1;
	BYTE bRFU6			:1;
	BYTE bRFU5			:1;
	BYTE bCredit			:1;
	BYTE bAutoLoad			:1;
	BYTE bRefunded			:1;
	BYTE bBlocked			:1;
	BYTE bActivated 		:1;
#else
	BYTE bActivated 		:1;
	BYTE bBlocked			:1;
	BYTE bRefunded			:1;
	BYTE bAutoLoad			:1;
	BYTE bCredit			:1;
	BYTE bRFU5			:1;
	BYTE bRFU6			:1;
	BYTE bRFU7			:1;
#endif
	BYTE ucAutoLoadAmt[3];
	BYTE ucPID[8];
	BYTE ucCPUAdminKeyKVN;
	BYTE ucCreditKeyKVN;
	BYTE ucSignatureKeyKVN;
	BYTE ucCPUIssuerKeyKVN;
	BYTE ucDebitTxnRecords;
	BYTE ucMinBalanceAllowed[3];
	BYTE ucSubAreaCode[2];
	BYTE ucIssuedDate[4];
	BYTE ucExpiryDate[4];
	BYTE ucEV[3];
	BYTE ucTxnSN[3];
	BYTE ucCardType;
	BYTE ucPersonalProfile;
	BYTE ucProfileExpiryDate[4];
	BYTE ucAreaCode;
	BYTE ucCardID[7];
	BYTE ucCardIDLen;
	BYTE ucDeposit[3];
	BYTE ucIssuerCode;
	BYTE ucBankCode;
	BYTE ucLoyaltyCounter[2];
	LastTxnLog_t stLastCreditTxnLogInfo_t;//33
	LastTxnLog_t stLastDebitTxnLogInfo_t;//33
	BYTE ucCardOneDayQuota[3];
	BYTE ucCardOneDayQuotaDate[2];
	BYTE ucMerchantUseFlag;
	//BYTE ucRFU[5];
	BYTE ucIssuerStatus;
	BYTE ucBlockStatus;
	BYTE ucAutoloadCounter;
	BYTE ucAutoloadDate[2];
	BYTE ucAuthenFlag[1];					   //
	BYTE ucCPUAuthenFlag[1];					   //
	BYTE ucMifareSocialCode[6];                            	// Mifare 身份證號碼
	BYTE ucCPUSocialCodeReadFlag;                           // 0x00:原值讀出;0x01:資料解密
	BYTE ucCPUSocialCode[16];						// CPU 身份證號碼
	BYTE ucDDPowerFlag[1];						      // 點鑽應用旗標
	BYTE ucMaxEV[3];                                                // 加值交易金額上限
	BYTE RetailCardId[1];							//企業識別碼
	BYTE TLRTACCTXNValue[1];                               // 淡水輕軌常客回饋
	BYTE ucRFU[ 83 ];								// Card/Reader 保留，補0x00，6bytes
	BYTE ucStatusCode[2];
}ReadCardBasicData_APDU_Out;
/**************************ReadCardBasicData***************************/

/*****************************TxnReqOnline*****************************/
typedef struct __packed_back{
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucTMLocationID[10];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE ucTxnDateTime[4];
	BYTE ucTxnAmt[3];
	BYTE ucReadPurseFlag;
	BYTE ucRefundFee[2];
	BYTE ucBrokenFee[2];
	BYTE ucCustomerFee[2];
	BYTE ucSPID;
	BYTE ucCPUSPID[3];
#ifdef ECC_BIG_ENDIAN
	unsigned char RFU_Flag:7;
	unsigned char RetryFlag:1;						// Bit 0：是否為Retry交易	 0:非Retry交易 1:Retry交易
#else
	unsigned char RetryFlag:1;						// Bit 0：是否為Retry交易	 0:非Retry交易 1:Retry交易
	unsigned char RFU_Flag:7;
#endif
	BYTE ucRFU[7];
}TxnReqOnline_APDU_In;

typedef struct __packed_back{
	BYTE ucPurseVersionNumber;
#ifdef ECC_BIG_ENDIAN
	BYTE bRFU7			:1;
	BYTE bRFU6			:1;
	BYTE bRFU5			:1;
	BYTE bCredit			:1;
	BYTE bAutoLoad			:1;
	BYTE bRefunded			:1;
	BYTE bBlocked			:1;
	BYTE bActivated 		:1;
#else
	BYTE bActivated 		:1;
	BYTE bBlocked			:1;
	BYTE bRefunded			:1;
	BYTE bAutoLoad			:1;
	BYTE bCredit			:1;
	BYTE bRFU5			:1;
	BYTE bRFU6			:1;
	BYTE bRFU7			:1;
#endif
	BYTE ucAutoLoadAmt[3];
	BYTE ucPID[8];
	BYTE ucCPUAdminKeyKVN;
	BYTE ucCreditKeyKVN;
	BYTE ucSignatureKeyKVN;
	BYTE ucCPUIssuerKeyKVN;
	BYTE ucCTC[3];
	BYTE ucTxnMode;
	BYTE ucTxnQuqlifier;
	BYTE ucSubAreaCode[2];
	BYTE ucExpiryDate[4];
	BYTE ucEV[3];
	BYTE ucTxnSN[3];
	BYTE ucCardType;
	BYTE ucPersonalProfile;
	BYTE ucProfileExpiryDate[4];
	BYTE ucAreaCode;
	BYTE ucCardID[7];
	BYTE ucCardIDLen;
	BYTE ucTxnAmt[3];
	BYTE ucHostSpecVersionNo;
	BYTE ucReaderFWVersion[6];
	BYTE ucDeviceID[4];
	BYTE ucCPUDeviceID[6];
	BYTE ucSPID;
	BYTE ucCPUSPID[3];
	BYTE ucLocationID;
	BYTE ucCPULocationID[2];
	BYTE ucDeposit[3];
	BYTE ucIssuerCode;
	BYTE ucBankCode;
	BYTE ucLoyaltyCounter[2];
	BYTE ucSAMID[8];
	BYTE ucSAMSN[4];
	BYTE ucSAMCRN[8];
	BYTE ucSAMKVN;
	BYTE ucSTAC[8];
	BYTE ucTMSerialNumber[6];
	LastTxnLog_t stLastCreditTxnLogInfo_t;//33
	ReaderAVRAPDU_t stReaderAVRInfo_t;//83
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucCPUSAMID[8];
	BYTE ucHostAdminKVN;
	BYTE ucTxnCrypto[8];
	BYTE ucStatusCode[2];
}TxnReqOnline_APDU_Out;
/*****************************TxnReqOnline*****************************/

/*****************************AuthTxnOnline****************************/
typedef struct __packed_back{
	BYTE ucExpiryDate[4];
	BYTE ucProfileExpiryDate[4];
	BYTE ucProfileExpiryDateDOS[2];
	BYTE ucTxnToken[16];
	BYTE ucHTAC[8];
	BYTE ucLCDControlFlag;
	BYTE ucRFU[5];
}AuthTxnOnline_APDU_In;

typedef struct __packed_back{
	BYTE ucTxnSN[3];
	BYTE ucEV[3];
	BYTE ucConfirmCode[2];
	BYTE ucSignature[16];
	BYTE ucCPUSAMID[8];
	BYTE ucMAC_HCrypto[18];
	BYTE ucTxnDateTime[4];
	BYTE BankCode[ 1 ];
	BYTE SingleAutoLoadTransactionAmount[ 2 ];
	BYTE AutoloadFlag[ 1 ];
	BYTE Acc_Free_Rids[2]; 					   //社福卡免費搭乘累積優惠點數
	BYTE Acc_Free_Rids_Date[2];				   //社福卡免費搭乘交易日期,跨越時,點數歸零重計
	BYTE ucRFU[ 2 ];											// 保留，補0x00，14bytes
	//BYTE ucRFU[10];
	BYTE ucStatusCode[2];
}AuthTxnOnline_APDU_Out;
/*****************************AuthTxnOnline****************************/

/*****************************TxnReqOffline****************************/
typedef struct __packed_back{
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucTMLocationID[10];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE ucTxnDateTime[4];
	BYTE ucTxnAmt[3];
	BYTE ucReadPurseFlag;
	BYTE ucRefundFee[2];
	BYTE ucBrokenFee[2];
	BYTE ucCustomerFee[2];
	BYTE ucLCDControlFlag;
	BYTE CMASTxnSN[3]; 						//CMAS交易序號
#ifdef ECC_BIG_ENDIAN
	BYTE RFU_Flag:4;
	BYTE RetryFlag:1;						// Bit 3：是否為Retry交易    0:非Retry交易 1:Retry交易
	BYTE OnceQuotaFlagForTxnOffline:1; 	// Bit 2：小額消費次限額旗標 0:次限額參考PPR_ResetOffline/PPR_SignOn之參數. 1:無次限額
	BYTE OneDayQuotaFlagForTxnOffline:1;	// Bit 1：小額消費日限額旗標 0:日限額參考PPR_ResetOffline/PPR_SignOn之參數. 1:無日限額
	BYTE WriteCMASTxnSNFlag:1;
#else
	BYTE WriteCMASTxnSNFlag:1;
	BYTE OneDayQuotaFlagForTxnOffline:1;	// Bit 1：小額消費日限額旗標 0:日限額參考PPR_ResetOffline/PPR_SignOn之參數. 1:無日限額
	BYTE OnceQuotaFlagForTxnOffline:1; 	// Bit 2：小額消費次限額旗標 0:次限額參考PPR_ResetOffline/PPR_SignOn之參數. 1:無次限額
	BYTE RetryFlag:1;						// Bit 3：是否為Retry交易    0:非Retry交易 1:Retry交易
	BYTE RFU_Flag:4;
#endif
	BYTE ucSPID;
	BYTE ucCPUSPID[3];
	BYTE ucRFU[3];
}TxnReqOffline_APDU_In;

typedef struct __packed_back{
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucTMLocationID[10];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE ucTxnDateTime[4];
	BYTE ucTxnAmt[3];
	BYTE ucReadPurseFlag;
	BYTE ucRefundFee[2];
	BYTE ucBrokenFee[2];
	BYTE ucCustomerFee[2];
	BYTE ucLCDControlFlag;
	BYTE CMASTxnSN[3]; 						//CMAS交易序號
#ifdef ECC_BIG_ENDIAN
	BYTE RFU_Flag:4;
	BYTE RetryFlag:1;						// Bit 3：是否為Retry交易    0:非Retry交易 1:Retry交易
	BYTE OnceQuotaFlagForTxnOffline:1; 	// Bit 2：小額消費次限額旗標 0:次限額參考PPR_ResetOffline/PPR_SignOn之參數. 1:無次限額
	BYTE OneDayQuotaFlagForTxnOffline:1;	// Bit 1：小額消費日限額旗標 0:日限額參考PPR_ResetOffline/PPR_SignOn之參數. 1:無日限額
	BYTE WriteCMASTxnSNFlag:1;
#else
	BYTE WriteCMASTxnSNFlag:1;
	BYTE OneDayQuotaFlagForTxnOffline:1;	// Bit 1：小額消費日限額旗標 0:日限額參考PPR_ResetOffline/PPR_SignOn之參數. 1:無日限額
	BYTE OnceQuotaFlagForTxnOffline:1; 	// Bit 2：小額消費次限額旗標 0:次限額參考PPR_ResetOffline/PPR_SignOn之參數. 1:無次限額
	BYTE RetryFlag:1;						// Bit 3：是否為Retry交易    0:非Retry交易 1:Retry交易
	BYTE RFU_Flag:4;
#endif
	BYTE ucSPID;
	BYTE ucCPUSPID[3];
	BYTE ucRFU[3];
	BYTE ucNewExpiryDate[4];
	BYTE ucNewProfileExpiryDate1[4];
	BYTE ucNewProfileExpiryDate2[2];
	BYTE ucLiquorTxnAmt[2];
	BYTE ucTobaccoTxnAmt[2];
	BYTE ucRFU2[18];
}TxnReqOffline2_APDU_In;


typedef struct __packed_back{
	BYTE ucPurseVersionNumber;
#ifdef ECC_BIG_ENDIAN
	BYTE bRFU7			:1;
	BYTE bRFU6			:1;
	BYTE bRFU5			:1;
	BYTE bCredit			:1;
	BYTE bAutoLoad			:1;
	BYTE bRefunded			:1;
	BYTE bBlocked			:1;
	BYTE bActivated 		:1;
#else
	BYTE bActivated 		:1;
	BYTE bBlocked			:1;
	BYTE bRefunded			:1;
	BYTE bAutoLoad			:1;
	BYTE bCredit			:1;
	BYTE bRFU5			:1;
	BYTE bRFU6			:1;
	BYTE bRFU7			:1;
#endif
	BYTE ucAutoLoadAmt[3];
	BYTE ucPID[8];
	BYTE ucCPUAdminKeyKVN;
	BYTE ucCreditKeyKVN;
	BYTE ucSignatureKeyKVN;
	BYTE ucCPUIssuerKeyKVN;
	BYTE ucCTC[3];
	BYTE ucTxnMode;
	BYTE ucTxnQuqlifier;
	BYTE ucSubAreaCode[2];
	BYTE ucExpiryDate[4];
	BYTE ucEV[3];
	BYTE ucTxnSN[3];
	BYTE ucCardType;
	BYTE ucPersonalProfile;
	BYTE ucProfileExpiryDate[4];
	BYTE ucAreaCode;
	BYTE ucCardID[7];
	BYTE ucCardIDLen;
	BYTE ucTxnAmt[3];
	BYTE ucHostSpecVersionNo;
	BYTE ucReaderFWVersion[6];
	BYTE ucDeviceID[4];
	BYTE ucCPUDeviceID[6];
	BYTE ucSPID;
	BYTE ucCPUSPID[3];
	BYTE ucLocationID;
	BYTE ucCPULocationID[2];
	BYTE ucDeposit[3];
	BYTE ucIssuerCode;
	BYTE ucBankCode_HostAdminKVN;
	BYTE ucCPDReadFlag;
	BYTE ucCPD_SAMID[16];
	BYTE ucCPDRAN_SAMCRN[8];
	BYTE ucCPDKVN_SAMKVN;
	BYTE ucSID_STAC[8];
	BYTE ucTMSerialNumber[6];
	LastTxnLog_t stLastCreditTxnLogInfo_t;//33
	BYTE ucSVCrypto[16];
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucCardOneDayQuota[3];
	BYTE ucCardOneDayQuotaDate[2];
	BYTE ucTxnSN2[3];
	BYTE ucEV2[3];
	BYTE ucConfirmCode[2];
	BYTE ucSignature[16];
	BYTE ucCPUSAMID[8];
	BYTE ucMAC_HCrypto[18];
	BYTE ucTxnDateTime[4];
	BYTE DDPowerFlag[ 1 ];
	BYTE BankCode[ 1 ];
	BYTE ucRFU[ 17 ];										// Card/Reader 保留，補0x00，19bytes
	BYTE ucStatusCode[2];
}TxnReqOffline_APDU_Out;
/*****************************TxnReqOffline****************************/

/*****************************AuthTxnOffline***************************/
typedef struct __packed_back{
	BYTE ucHVCrypto_HTAC[16];
	BYTE ucLCDControlFlag;
	BYTE ucRFU[5];
}AuthTxnOffline_APDU_In;

typedef struct __packed_back{
	BYTE ucTxnSN[3];
	BYTE ucEV[3];
	BYTE ucConfirmCode[2];
	BYTE ucSignature[16];
	BYTE ucCPUSAMID[8];
	BYTE ucMAC_HCrypto[18];
	BYTE ucTxnDateTime[4];
	BYTE ucCardOneDayQuota[3];
	BYTE ucCardOneDayQuotaDate[2];
	BYTE ucRFU[5];
	BYTE ucStatusCode[2];
}AuthTxnOffline_APDU_Out;
/*****************************AuthTxnOffline***************************/

/******************************LockCard 1******************************/
typedef struct __packed_back{
	BYTE ucCardID_4[4];
	BYTE ucLockReason;
}LockCard_APDU_In_1;

typedef struct __packed_back{
	BYTE ucDeviceID[4];
	BYTE ucCardType;
	BYTE ucPersonalProfile;
	BYTE ucCardID_4[4];
	BYTE ucIssuerCode;
	BYTE ucStatusCode[2];
}LockCard_APDU_Out_1;
/******************************LockCard 1******************************/

/******************************LockCard 2******************************/
typedef struct __packed_back{
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucCardID[7];
	BYTE ucTxnDateTime[4];
	BYTE ucLockReason;
}LockCard_APDU_In_2;

typedef struct __packed_back{
	BYTE ucPurseVersionNumber;
	BYTE ucPID[8];
	BYTE ucCTC[3];
	BYTE ucCardType;
	BYTE ucPersonalProfile;
	BYTE ucCardID[7];
	BYTE ucCardIDLen;
	BYTE ucDeviceID[4];
	BYTE ucCPUDeviceID[6];
	BYTE ucSPID;
	BYTE ucCPUSPID[3];
	BYTE ucLocationID;
	BYTE ucCPULocationID[2];
	BYTE ucIssuerCode;
	BYTE ucStatusCode[2];
}LockCard_APDU_Out_2;

typedef struct __packed_back{
	BYTE ucPurseVersionNumber;
#ifdef ECC_BIG_ENDIAN
	BYTE bRFU7			:1;
	BYTE bRFU6			:1;
	BYTE bRFU5			:1;
	BYTE bCredit			:1;
	BYTE bAutoLoad			:1;
	BYTE bRefunded			:1;
	BYTE bBlocked			:1;
	BYTE bActivated 		:1;
#else
	BYTE bActivated 		:1;
	BYTE bBlocked			:1;
	BYTE bRefunded			:1;
	BYTE bAutoLoad			:1;
	BYTE bCredit			:1;
	BYTE bRFU5			:1;
	BYTE bRFU6			:1;
	BYTE bRFU7			:1;
#endif
	BYTE ucAutoLoadAmt[3];
	BYTE ucPID[8];
	BYTE ucCPUIssuerKeyKVN;
	BYTE ucSubAreaCode[2];
	BYTE ucExpiryDate[4];
	BYTE ucEV[3];
	BYTE ucTxnSN[3];
	BYTE ucCardType;
	BYTE ucPersonalProfile;
	BYTE ucProfileExpiryDate[4];
	BYTE ucAreaCode;
	BYTE ucCardID[7];
	BYTE ucCardIDLen;
	BYTE ucDeviceID[4];
	BYTE ucCPUDeviceID[6];
	BYTE ucSPID;
	BYTE ucCPUSPID[3];
	BYTE ucLocationID;
	BYTE ucCPULocationID[2];
	BYTE ucDeposit[3];
	BYTE ucIssuerCode;
	BYTE ucBankCode;
	BYTE ucLoyaltyCounter[2];
	LastTxnLog_t stLastCreditTxnLogInfo_t;//33
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucEV2[3];
	BYTE ucMifareSettingParameter;
	BYTE ucCPUSettingParameter;
	BYTE ucRFU[15];
	BYTE ucStatusCode[2];
}LockCard_APDU_Out_3;
/******************************LockCard 2******************************/

/***************************ReadDongleDeduct***************************/
typedef struct __packed_back{
	BYTE ucLogSeqNo[2];
}ReadDongleDeduct_APDU_In;

typedef struct __packed_back{
	BYTE ucLogSeqNo[2];
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucPurseVersionNumber;
	BYTE ucPID[8];
	BYTE ucSignatureKeyKVN;
	BYTE ucCTC[3];
	BYTE ucTxnMode;
	BYTE ucTxnQuqlifier;
	BYTE ucSubAreaCode[2];
	BYTE ucTxnDateTime[4];
	BYTE ucEVBeforeTxn[3];
	BYTE ucTxnSNBeforeTxn[3];
	BYTE ucCardType;
	BYTE ucPersonalProfile;
	BYTE ucAreaCode;
	BYTE ucCardID[7];
	BYTE ucCardIDLen;
	BYTE ucTxnAmt[3];
	BYTE ucDeviceID[4];
	BYTE ucCPUDeviceID[6];
	BYTE ucSPID;
	BYTE ucCPUSPID[3];
	BYTE ucLocationID;
	BYTE ucCPULocationID[2];
	BYTE ucDeposit[3];
	BYTE ucIssuerCode;
	BYTE ucBankCode;
	BYTE ucLoyaltyCounter[2];
	BYTE ucTxnSN[3];
	BYTE ucEV[3];
	BYTE ucSignature[16];
	BYTE ucCPUSAMID[8];
	BYTE ucMAC_HCrypto[18];
	LastTxnLog_t stLastCreditTxnLogInfo_t;//33
	BYTE ucTMLocationID[10];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
    BYTE ucTxnAMT1[2];
    BYTE ucTxnAMT2[2];
    BYTE ucTxnAMT3[2];
	BYTE ucCMASTxnSN[3];
	BYTE ucTxnAMT4[2];
	BYTE ucTxnAMT5[2];
	BYTE ucRFU[3];
	BYTE ucStatusCode[2];
}ReadDongleDeduct_APDU_Out;
/***************************ReadDongleDeduct***************************/

/********************************SetValue******************************/
typedef struct __packed_back{
	BYTE ucMsgType;
	BYTE ucTMLocationID_6[6];
	BYTE ucTMID[2];
	BYTE ucTxnDateTime[4];
}SetValue_APDU_In;

typedef struct __packed_back{
	BYTE ucCardType;
	BYTE ucPersonalProfile;
	BYTE ucCardID_4[4];
	BYTE ucIssuerCode;
	BYTE ucEV_2[2];
	BYTE ucExpiryDate[4];
	BYTE ucAreaCode;
	BYTE ucReaderFWVersion[6];
	BYTE ucSAMID[8];
	BYTE ucSAMSN[4];
	BYTE ucSAMCRN[8];
	BYTE ucDeviceID[4];
	BYTE ucSAMKeyVersion;
	BYTE ucSTAC[8];
	BYTE ucStatusCode[2];
}SetValue_APDU_Out;
/********************************SetValue******************************/

/******************************AuthSetValue****************************/
typedef struct __packed_back{
	BYTE ucExpiryDate[4];
	BYTE ucHTAC[8];
	BYTE ucExpiryDateDOS[2];
}AuthSetValue_APDU_In;

typedef struct __packed_back{
	BYTE ucStatusCode[2];
}AuthSetValue_APDU_Out;
/******************************AuthSetValue****************************/

/********************************TxRefund******************************/
typedef struct __packed_back{
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucTMLocationID_6[6];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE ucTxnDateTime[4];
	BYTE ucTxnAmt_2[2];
	BYTE ucTMTxnDateTime2[14];
	BYTE ucTMSerialNumber2[6];
	BYTE ucTxnDateTime2[4];
}TxRefund_APDU_In;

typedef struct __packed_back{
	BYTE ucCardType;
	BYTE ucPersonalProfile;
	BYTE ucCardID_4[4];
	BYTE ucIssuerCode;
	BYTE ucBankCode;
	BYTE ucLoyaltyCounter[2];
	BYTE ucDeposit[2];
	BYTE ucEV_2[2];
	BYTE ucTxnAmt_2[2];
	BYTE ucExpiryDate[4];
	BYTE ucAreaCode;
	BYTE ucReaderFWVersion[6];
	BYTE ucSAMID[8];
	BYTE ucSAMSN[4];
	BYTE ucSAMCRN[8];
	BYTE ucDeviceID[4];
	BYTE ucSAMKeyVersion;
	BYTE ucSTAC[8];
	BYTE ucTMLocationID_6[6];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE ucCAVRTxnSNLSB;
	BYTE ucCAVRTxnDateTime[4];
	BYTE ucCAVRSubType;
	BYTE ucCAVRTxnAmt[2];
	BYTE ucCAVREV[2];
	BYTE ucCAVRSPID;
	BYTE ucCAVRLocationID;
	BYTE ucCAVRDeviceID[4];//108
	ReaderAVR_t stReaderAVR_t;//38
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucEV2_2[2];
	BYTE ucTxnAmt2_2[2];
	BYTE ucTMTxnDateTime2[14];
	BYTE ucTMSerialNumber2[6];
	BYTE ucStatusCode[2];
}TxRefund_APDU_Out;
/********************************TxRefund******************************/

/******************************AuthTxRefund****************************/
typedef struct __packed_back{
	BYTE ucRFU4[4];
	BYTE ucHTAC[8];
	BYTE ucRFU2[2];
	BYTE ucLCDControlFlag;
}AuthTxRefund_APDU_In;

typedef struct __packed_back{
	BYTE ucTxnSN_2[2];
	BYTE ucEV_2[2];
	BYTE ucMAC[10];
	BYTE ucConfirmCode[2];
	BYTE ucTxnSN2_2[2];
	BYTE ucEV2_2[2];
	BYTE ucMAC2[10];
	BYTE ucConfirmCode2[2];
	BYTE ucCTAC[8];
	BYTE ucStatusCode[2];
}AuthTxRefund_APDU_Out;
/******************************AuthTxRefund****************************/

/*****************************AutoloadEnable***************************/
typedef struct __packed_back{
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucTMLocationID_6[6];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE ucTxnDateTime[4];
}AutoloadEnable_APDU_In;

typedef struct __packed_back{
	BYTE ucCardType;
	BYTE ucPersonalProfile;
	BYTE ucCardID_4[4];
	BYTE ucIssuerCode;
	BYTE ucBankCode;
	BYTE ucLoyaltyCounter[2];
	BYTE ucDeposit[2];
	BYTE ucEV_2[2];
	BYTE ucTxnAmt_2[2];
	BYTE ucExpiryDate[4];
	BYTE ucAreaCode;
	BYTE ucReaderFWVersion[6];
	BYTE ucSAMID[8];
	BYTE ucSAMSN[4];
	BYTE ucSAMCRN[8];
	BYTE ucDeviceID[4];
	BYTE ucSAMKeyVersion;
	BYTE ucSTAC[8];
	BYTE ucTMLocationID_6[6];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE ucCAVRTxnSNLSB;
	BYTE ucCAVRTxnDateTime[4];
	BYTE ucCAVRSubType;
	BYTE ucCAVRTxnAmt[2];
	BYTE ucCAVREV[2];
	BYTE ucCAVRSPID;
	BYTE ucCAVRLocationID;
	BYTE ucCAVRDeviceID[4];//108
	ReaderAVR_t stReaderAVR_t;//38
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucAutoLoadFlag;
	BYTE ucAutoLoadAmt_2[2];
	BYTE ucStatusCode[2];
}AutoloadEnable_APDU_Out;
/*****************************AutoloadEnable***************************/

/***************************AuthAutoloadEnable*************************/
typedef struct __packed_back{
	BYTE ucHTAC[8];
	BYTE ucLCDControlFlag;
}AuthAutoloadEnable_APDU_In;

typedef struct __packed_back{
	BYTE ucTxnSN_2[2];
	BYTE ucEV_2[2];
	BYTE ucMAC[10];
	BYTE ucConfirmCode[2];
	BYTE ucCTAC[8];
	BYTE ucStatusCode[2];
}AuthAutoloadEnable_APDU_Out;
/***************************AuthAutoloadEnable*************************/

/********************************AddValue******************************/
typedef struct __packed_back{
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucTMLocationID_6[6];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE ucTxnDateTime[4];
	BYTE ucTxnAmt_2[2];
}AddValue_APDU_In;

typedef struct __packed_back{
	BYTE ucCardType;
	BYTE ucPersonalProfile;
	BYTE ucCardID_4[4];
	BYTE ucIssuerCode;
	BYTE ucBankCode;
	BYTE ucLoyaltyCounter[2];
	BYTE ucDeposit[2];
	BYTE ucEV_2[2];
	BYTE ucTxnAmt_2[2];
	BYTE ucExpiryDate[4];
	BYTE ucAreaCode;
	BYTE ucReaderFWVersion[6];
	BYTE ucSAMID[8];
	BYTE ucSAMSN[4];
	BYTE ucSAMCRN[8];
	BYTE ucDeviceID[4];
	BYTE ucSAMKeyVersion;
	BYTE ucSTAC[8];
	BYTE ucTMLocationID_6[6];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE ucCAVRTxnSNLSB;
	BYTE ucCAVRTxnDateTime[4];
	BYTE ucCAVRSubType;
	BYTE ucCAVRTxnAmt[2];
	BYTE ucCAVREV[2];
	BYTE ucCAVRSPID;
	BYTE ucCAVRLocationID;
	BYTE ucCAVRDeviceID[4];//108
	ReaderAVR_t stReaderAVR_t;//38
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucStatusCode[2];
}AddValue_APDU_Out;
/********************************AddValue******************************/

/******************************AuthAddValue****************************/
typedef struct __packed_back{
	BYTE ucExpiryDate[4];
	BYTE ucHTAC[8];
	BYTE ucProfileExpiryDateDOS[2];
	BYTE ucLCDControlFlag;
}AuthAddValue_APDU_In;

typedef struct __packed_back{
	BYTE ucTxnSN_2[2];
	BYTE ucEV_2[2];
	BYTE ucMAC[10];
	BYTE ucConfirmCode[2];
	BYTE ucCTAC[8];
	BYTE ucStatusCode[2];
}AuthAddValue_APDU_Out;
/******************************AuthAddValue****************************/

/****************************ReadCardDeduct****************************/
typedef struct __packed_back{
	BYTE ucLogIndex;
}ReadCardDeduct_APDU_In;

typedef struct __packed_back{
	LastTxnLog_t stCardDeductLog1_t;//33
	LastTxnLog_t stCardDeductLog2_t;//33
	LastTxnLog_t stCardDeductLog3_t;//33
	LastTxnLog_t stCardDeductLog4_t;//33
	LastTxnLog_t stCardDeductLog5_t;//33
	LastTxnLog_t stCardDeductLog6_t;//33
	BYTE ucStatusCode[2];
}ReadCardDeduct_APDU_Out;
/****************************ReadCardDeduct****************************/

/****************************ReadCodeVersion***************************/
typedef struct __packed_back{
	BYTE ucSAMAppletVersion;
	BYTE ucSAMType;
	BYTE ucSAMVersion;
	BYTE ucReaderFWVersion[6];
	BYTE ucHostSpecVersionNo;
	BYTE ucStatusCode[2];
}ReadCodeVersion_APDU_Out;
/****************************ReadCodeVersion***************************/

/********************************Reboot********************************/
typedef struct __packed_back{
	BYTE ucStatusCode[2];
}Reboot_APDU_Out;
/********************************Reboot********************************/

/*******************************SetValue2******************************/
typedef struct __packed_back{
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucTMLocationID[10];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE ucTxnDateTime[4];
	BYTE ucTxnAmt[3];
	BYTE ucReadPurseFlag;
	BYTE ucRefundFee[3];
	BYTE ucBrokenFee[3];
	BYTE ucLCDControlFlag;
	BYTE ucRFU[11];
}SetValue2_APDU_In;

typedef struct __packed_back{
	BYTE ucPurseVersionNumber;
#ifdef ECC_BIG_ENDIAN
	BYTE bRFU7			:1;
	BYTE bRFU6			:1;
	BYTE bRFU5			:1;
	BYTE bCredit			:1;
	BYTE bAutoLoad			:1;
	BYTE bRefunded			:1;
	BYTE bBlocked			:1;
	BYTE bActivated 		:1;
#else
	BYTE bActivated 		:1;
	BYTE bBlocked			:1;
	BYTE bRefunded			:1;
	BYTE bAutoLoad			:1;
	BYTE bCredit			:1;
	BYTE bRFU5			:1;
	BYTE bRFU6			:1;
	BYTE bRFU7			:1;
#endif
	BYTE ucAutoLoadAmt[3];
	BYTE ucPID[8];
	BYTE ucCPUAdminKeyKVN;
	BYTE ucCreditKeyKVN;
	BYTE ucCPUIssuerKeyKVN;
	BYTE ucCTC[3];
	BYTE ucTxnMode;
	BYTE ucTxnQuqlifier;
	BYTE ucExpiryDate[4];
	BYTE ucEV[3];
	BYTE ucTxnSN[3];
	BYTE ucCardType;
	BYTE ucPersonalProfile;
	BYTE ucProfileExpiryDate[4];
	BYTE ucAreaCode;
	BYTE ucCardID[7];
	BYTE ucCardIDLen;
	BYTE ucTxnAmt[3];
	BYTE ucHostSpecVersionNo;
	BYTE ucReaderFWVersion[6];
	BYTE ucDeviceID[4];
	BYTE ucCPUDeviceID[6];
	BYTE ucSPID;
	BYTE ucCPUSPID[3];
	BYTE ucLocationID;
	BYTE ucCPULocationID[2];
	BYTE ucDeposit[3];
	BYTE ucIssuerCode;
	BYTE HostAdminKeyKVN_SAMKVN[1];
	BYTE SAMID_STAC[8];
	BYTE SVCrypto_SAMCRN_SAMID[16];
	BYTE SAMSN[4];
	BYTE SubAreaCode_LoyaltyCounter[2];
	BYTE ucTMSerialNumber[6];
	LastTxnLog_t stLastCreditTxnLogInfo_t;//33
	//ReaderAVRAPDU_t stReaderAVRInfo_t;//83
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucRFU[31];
	BYTE ucStatusCode[2];
}SetValue2_APDU_Out;
/*******************************SetValue2******************************/

/******************************AuthSetValue2***************************/
typedef struct __packed_back{
	BYTE ucExpiryDate[4];
	BYTE ucProfileExpiryDate[4];
	BYTE ucProfileExpiryDateDOS[2];
	BYTE ucTxnToken[16];
	BYTE ucHTAC[8];
	BYTE ucLCDControlFlag;
	BYTE ucRFU[5];
}AuthSetValue2_APDU_In;

typedef struct __packed_back{
	BYTE ucTxnSN[3];
	BYTE ucEV[3];
	BYTE ucTxnDateTime[4];
	BYTE ucRFU[22];
	BYTE ucStatusCode[2];
}AuthSetValue2_APDU_Out;
/******************************AuthSetValue2***************************/

/********************************TxRefund2*****************************/
typedef struct __packed_back{
	BYTE ucCardType;
	BYTE ucPersonalProfile;
	BYTE ucCardID_4[4];
	BYTE ucIssuerCode;
	BYTE ucBankCode;
	BYTE ucLoyaltyCounter[2];
	BYTE ucDeposit[2];
	BYTE ucEV_2[2];
	BYTE ucTxnAmt_2[2];
	BYTE ucTxmSn_2[2];
	BYTE ucRFU[2];
	BYTE ucAreaCode;
	BYTE ucReaderFWVersion[6];
	BYTE ucSAMID[8];
	BYTE ucSAMSN[4];
	BYTE ucSAMCRN[8];
	BYTE ucDeviceID[4];
	BYTE ucCPUDeviceID[6];
	BYTE ucSAMKeyVersion;
	BYTE ucSTAC[8];
	BYTE ucTMLocationID_6[6];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE ucCAVRTxnSNLSB;
	BYTE ucCAVRTxnDateTime[4];
	BYTE ucCAVRSubType;
	BYTE ucCAVRTxnAmt[2];
	BYTE ucCAVREV[2];
	BYTE ucCAVRSPID;
	BYTE ucCAVRLocationID;
	BYTE ucCAVRDeviceID[4];//108
	ReaderAVRAPDU_t stReaderAVRInfo_t;//83
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucEV2_2[2];
	BYTE ucTxnAmt2_2[2];
	BYTE ucTxmSn2_2[2];
	BYTE ucTMTxnDateTime2[14];
	BYTE ucTMSerialNumber2[6];
	BYTE ucStatusCode[2];
}TxRefund2_APDU_Out;
/********************************TxRefund2*****************************/

/****************************ReadCardNumber****************************/
typedef struct __packed_back{
	BYTE ucTxnAmt[3];
	BYTE ucLCDControlFlag;
}ReadCardNumber_APDU_In;

typedef struct __packed_back{
	BYTE ucCardID[7];
	BYTE ucCardIDLen;
	BYTE ucCardClass;
	BYTE ucStatusCode[2];
}ReadCardNumber_APDU_Out;
/****************************ReadCardNumber****************************/

/*******************************FareSale********************************/
typedef struct __packed_back{
	BYTE ucFareProductType; 	 //特種票種類
	BYTE ucTxnDateTime[4];		//交易時間
	BYTE ucCardNumber[4];		//卡號
	BYTE ucAgentNumber[2];		 //操作人員代碼
	BYTE ucGroupsize;			 //the numpber of persons in a group
	BYTE ucEntryStation;
	BYTE ucExitStation;
	BYTE ucJourneyNumber;		 //maximum number of uses per period
	BYTE ucFirstDate[2];
	BYTE ucLastDate[2];
}FareSale_APDU_In;

typedef struct __packed_back{
	BYTE ucMsgType; 				//訊息代碼
	BYTE ucPersonalProfile; 		//票卡身份別
	BYTE ucTXNDateTime[4];			//交易時間
	BYTE ucCardNumber[4];			//卡號
	BYTE ucIssueCompany;				//發卡公司
	BYTE ucTXN_SN_LSB;				//交易序號末碼>
	BYTE ucTXN_Value[2];				//交易金額
	BYTE ucTXN_RemainValue[2];		//交易後票卡餘額>
	BYTE ucLineNumber[2];			//路線代碼0x00,0x00
	BYTE ucCurrentZoneNumber;		//出站代碼0x01
	BYTE ucGroupCode;				//轉乘群組
	BYTE ucEntryExitFlag;			//進出站代碼0x01
	BYTE ucEntryZoneNumber; 		//進站代碼0x01
	BYTE ucAreaCode;					//區碼
	BYTE ucTransferDiscount[2]; 	//轉乘優惠金額0x00,0x00
	BYTE ucPersonalDiscount[2]; 	//個人(社福)優惠金額0x00,0x00
	BYTE ucLoyaltyPoint[2]; 		//忠誠累積點數
	BYTE ucMAC[8];					//MAC
}FareSale_APDU_Out;
/*******************************FareSale********************************/

/*******************************VoidFare********************************/
typedef struct __packed_back{
	BYTE ucTxnDateTime[4];		//交易時間
	BYTE ucAgentNumber[2];		 //操作人員代碼
}VoidFare_APDU_In;

typedef struct __packed_back{
	BYTE ucMsgType; 				//訊息代碼
	BYTE ucSubType; 				//交易方式
	BYTE ucTXNDateTime[4];			//交易時間
	BYTE ucCardNumber[4];			//卡號
	BYTE ucIssueCompany;			//發卡公司
	BYTE ucTXN_SN_LSB;				//交易序號末碼
	BYTE ucTXN_Value[2];			//交易金額
	BYTE ucTXN_RemainValue[2];		//交易後票卡餘額
	BYTE ucAgentNumber[2];
	BYTE ucDeposit[2];				//押金
	BYTE ucMAC[8];					//MAC
}VoidFare_APDU_Out;
/*******************************VoidFare********************************/

/*******************************VoidFare********************************/
typedef struct __packed_back{
	BYTE ucFareProductType; 	 //特種票種類
	BYTE ucTxnDateTime[4];		//交易時間
	BYTE ucCardNumber[4];		//卡號
}RefundFare_APDU_In;

typedef struct __packed_back{
	BYTE ucMsgType; 				 //訊息代碼
	BYTE ucFareProductType; 		 //月票種類
	BYTE ucTXNDateTime[4];			 //交易時間
	BYTE ucCardNumber[4];			 //卡號
	BYTE ucIssueCompany;			 //發卡公司
	BYTE ucTXN_SN_LSB;				 //交易序號末碼
	BYTE ucTXN_Value[2];			 //交易金額
	BYTE ucTXN_RemainValue[2];		 //交易後票卡餘額
	BYTE ucLoyaltyPoint[2]; 		 //忠誠累積點數
	BYTE ucMAC[8];					//MAC
}RefundFare_APDU_Out;
/*******************************VoidFare********************************/

/*******************************TRTCRead********************************/
typedef struct __packed_back{
	BYTE ucLCDControlFlag;
	BYTE ucTMSerialNumber[6];
	BYTE ucTxnDateTime[4];
	BYTE ucRFU[5];
}TRTCRead_APDU_In;

typedef struct __packed_back{
	BYTE ucPurseVersionNumber;
#ifdef ECC_BIG_ENDIAN
	BYTE bRFU7			:1;
	BYTE bRFU6			:1;
	BYTE bRFU5			:1;
	BYTE bCredit			:1;
	BYTE bAutoLoad			:1;
	BYTE bRefunded			:1;
	BYTE bBlocked			:1;
	BYTE bActivated 		:1;
#else
	BYTE bActivated 		:1;
	BYTE bBlocked			:1;
	BYTE bRefunded			:1;
	BYTE bAutoLoad			:1;
	BYTE bCredit			:1;
	BYTE bRFU5			:1;
	BYTE bRFU6			:1;
	BYTE bRFU7			:1;
#endif
	BYTE ucAutoLoadAmt[3];
	BYTE ucPID[8];
	BYTE ucSubAreaCode[2];
	BYTE ucExpiryDate[4];
	BYTE ucEV[3];
	BYTE ucTxnSN[3];
	BYTE ucCardType;
	BYTE ucPersonalProfile;
	BYTE ucProfileExpiryDate[4];
	BYTE ucAreaCode;
	BYTE ucCardID[7];
	BYTE ucCardIDLen;
	BYTE ucDeviceID[4];
	BYTE ucCPUDeviceID[6];
	BYTE ucSPID;
	BYTE ucCPUSPID[3];
	BYTE ucLocationID;
	BYTE ucCPULocationID[2];
	BYTE ucIssuerCode;
	BYTE ucBankCode;
	BYTE ucLoyaltyCounter[2];
	LastTxnLog_t stLastCreditTxnLogInfo_t;//33
	BYTE ucAutoloadCounter;
	BYTE ucAutoloadDate[2];
	BYTE ucURT[26];
	BYTE ucSocial_Code[16]; 								//身份證號碼
	BYTE ucTRTCFixedRead[27];
	BYTE ucTRTCVariable[26];
	BYTE ucGONDOLAFixed[26];
	BYTE ucGONDOLAVariable[26];
	BYTE ucTheRemainderOfAddQuota[3];					//剩餘加值額度
	BYTE ucRFU[3];										//保留
	BYTE ucStatusCode[2];
}TRTCRead_APDU_Out;
/*******************************TRTCRead********************************/

/*******************************TRTCSetValue********************************/
typedef struct __packed_back{
	BYTE ucFuncOption;			   //TM0x01修改進出站碼0x02愛心票註記
	BYTE ucTxnDateTime[4];		   //TM交易日期時間UnsignedandLSBFirst(UnixDateTime)
	BYTE ucURTData[26]; 			   //TM欲修改之轉乘資料,PVN=0時,用前16Byte/PVN<>0時,用26Byte
	BYTE ucUTRData[26]; 			   //TM欲修改之六筆資料,PVN=0時,用前16Byte/PVN<>0時,用26Byte
	BYTE ucTRTCData[26];				   //TM欲修改之捷運資料,PVN=0時,用前16Byte/PVN<>0時,用26Byte
	BYTE ucLCDControlFlag;		   //TM用於控制交易完成後之LCD顯示0x00：顯示【交易完成　請取卡】(default)0x01：顯示【（請勿移動票卡）】
}TRTCSetValue_APDU_In;
/*******************************TRTCSetValue********************************/

/*****************************SetValueOffline*******************************/
typedef struct __packed_back{
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucTMLocationID[10];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE ucTxnDateTime[4];
	BYTE ucTxnAmt[3];
	BYTE ucReadPurseFlag;
	BYTE fNormalUpdateFlag:1;								// Bit 0：一般展期
	BYTE fPersonalUpdateFlag:1; 							// Bit 1：身份變更
	BYTE fExpireUpdateFlag:1;								// Bit 2：逾期展期
	BYTE fAreaUpdateFlag:1; 								// Bit 3：Area Code變更
	BYTE fAuthenUpdateFlag:1;								// Bit 4：地區認證旗標變更
	BYTE fRFUFlag:3;										// Bit 5~7：RFU
	BYTE ucRFUFlag[ 1 ];									// 保留
	BYTE ucNewExpiryDate[ 4 ];								// 新票卡到期日期，若不需展期則填0x00(Unsigned and LSB First (UnixDateTime))
	BYTE ucNewPersonalExpiryDate1[ 4 ]; 					// 新身分到期日期，若不需展期則填0x00(Unsigned and LSB First (UnixDateTime))
	BYTE ucNewPersonalExpiryDate2[ 2 ]; 					// API 轉換 新身分到期日期，若不需展期則填0x00(Unsigned and LSB First (DOS Date))
	BYTE ucNewPersonalProfile[ 1 ]; 						// 新身分別
	BYTE ucNewAreaCode[ 1 ];								// 新區碼
	BYTE ucNewAuthen[ 1 ];									// 新地區認證旗標
	BYTE ucNewCPUAuthen[ 1 ];								// 新CPU地區認證旗標
	BYTE ucNewSubAreaCode[ 2 ]; 							// 新子區碼
	BYTE ucRFU[ 31 ];										// TM 保留，補0x00，40bytes
	BYTE ucLCDControlFlag;							/* TM
																		用於控制交易完成後之LCD 顯示
																		0x00：顯示【交易完成 請取卡】(default)
																		0x01：顯示【（請勿移動票卡）】
																	*/
}SetValueOffline_APDU_In;

typedef struct __packed_back{
	BYTE ucPurseVersionNumber;
#ifdef ECC_BIG_ENDIAN
	BYTE bRFU7			:1;
	BYTE bRFU6			:1;
	BYTE bRFU5			:1;
	BYTE bCredit			:1;
	BYTE bAutoLoad			:1;
	BYTE bRefunded			:1;
	BYTE bBlocked			:1;
	BYTE bActivated 		:1;
#else
	BYTE bActivated 		:1;
	BYTE bBlocked			:1;
	BYTE bRefunded			:1;
	BYTE bAutoLoad			:1;
	BYTE bCredit			:1;
	BYTE bRFU5			:1;
	BYTE bRFU6			:1;
	BYTE bRFU7			:1;
#endif
	BYTE ucAutoLoadAmt[3];
	BYTE ucPID[8];
	BYTE ucCPUAdminKeyKVN;
	BYTE ucCreditKeyKVN;
	BYTE ucCPUIssuerKeyKVN;
	BYTE ucCTC[3];
	BYTE ucTxnMode;
	BYTE ucTxnQuqlifier;
	BYTE ucExpiryDate[4];
	BYTE ucEV[3];
	BYTE ucTxnSN[3];
	BYTE ucCardType;
	BYTE ucPersonalProfile;
	BYTE ucProfileExpiryDate[4];
	BYTE ucAreaCode;
	BYTE ucCardID[7];
	BYTE ucCardIDLen;
	BYTE ucTxnAmt[3];
	BYTE ucHostSpecVersionNo;
	BYTE ucReaderFWVersion[6];
	BYTE ucDeviceID[4];
	BYTE ucCPUDeviceID[6];
	BYTE ucSPID;
	BYTE ucCPUSPID[3];
	BYTE ucLocationID;
	BYTE ucCPULocationID[2];
	BYTE ucDeposit[3];
	BYTE ucIssuerCode;
	BYTE ucSubAreaCode_LoyaltyCounter[2];
	BYTE ucTMSerialNumber[6];
	LastTxnLog_t stLastCreditTxnLogInfo_t;//33
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucTxnSN2[3];
	BYTE ucEV2[3];
	BYTE ucucTxnDateTime2[4];
	BYTE RFU[50];
	BYTE ucStatusCode[2];
}SetValueOffline_APDU_Out;

/*****************************SetValueOffline*******************************/

/*****************************StudentSetValue*******************************/
typedef struct __packed_back{
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucTMLocationID[10];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE ucTxnDateTime[4];
	BYTE ucTxnAmt[3];
	BYTE ucReadPurseFlag;
	BYTE ucRFU[50];
}StudentSetValue_APDU_In;

typedef struct __packed_back{
	BYTE ucPurseVersionNumber;
#ifdef ECC_BIG_ENDIAN
	BYTE bRFU7			:1;
	BYTE bRFU6			:1;
	BYTE bRFU5			:1;
	BYTE bCredit			:1;
	BYTE bAutoLoad			:1;
	BYTE bRefunded			:1;
	BYTE bBlocked			:1;
	BYTE bActivated 		:1;
#else
	BYTE bActivated 		:1;
	BYTE bBlocked			:1;
	BYTE bRefunded			:1;
	BYTE bAutoLoad			:1;
	BYTE bCredit			:1;
	BYTE bRFU5			:1;
	BYTE bRFU6			:1;
	BYTE bRFU7			:1;
#endif
	BYTE ucAutoLoadAmt[3];
	BYTE ucPID[8];
	BYTE ucCPUAdminKeyKVN;
	BYTE ucCreditKeyKVN;
	BYTE ucSignatureKeyKVN;
	BYTE ucCPUDebitKeyKVN;
	BYTE ucCPUIssuerKeyKVN;
	BYTE ucCTC[3];
	BYTE ucTxnMode;
	BYTE ucTxnQuqlifier;
	BYTE ucExpiryDate[4];
	BYTE ucEV[3];
	BYTE ucTxnSN[3];
	BYTE ucCardType;
	BYTE ucPersonalProfile;
	BYTE ucProfileExpiryDate[4];
	BYTE ucAreaCode;
	BYTE ucCardID[7];
	BYTE ucCardIDLen;
	BYTE ucTxnAmt[3];
	BYTE ucHostSpecVersionNo;
	BYTE ucReaderFWVersion[6];
	BYTE ucDeviceID[4];
	BYTE ucCPUDeviceID[6];
	BYTE ucSPID;
	BYTE ucCPUSPID[3];
	BYTE ucLocationID;
	BYTE ucCPULocationID[2];
	BYTE ucDeposit[3];
	BYTE ucIssuerCode;
	BYTE ucLoyaltyCounterOrSubAreaCode[2];
	BYTE ucBankCode;
	BYTE ucSVCrypto_SAMCRN_SAMID[16];
	BYTE ucHost_SAMKVN;
	BYTE ucSAMSN[4];
	BYTE ucSID_STAC[8];
	BYTE ucTMSerialNumber[6];
	LastTxnLog_t stLastCreditTxnLogInfo_t;//33
	ReaderAVRAPDU_t stReaderAVRInfo_t;//83
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucRFU[18];
	BYTE ucStatusCode[2];
}StudentSetValue_APDU_Out;
/*****************************StudentSetValue*******************************/

/***************************AuthStudentSetValue*****************************/
typedef struct __packed_back{
	BYTE ucCardTypeUpdateFlag;
	BYTE ucNewCardType;
	BYTE ucPersonalExpiryDateUpdateFlag;
	BYTE ucNewPersonalExpiryDate[4];
	BYTE ucNewPersonalExpiryDateDOS[2];
	BYTE ucNewExpiryDate[4];
	BYTE ucHVCrypto[16];
	BYTE ucPutDataMAC[16];
	BYTE ucHTAC[8];
	BYTE ucLCDControlFlag;
	BYTE ucExtAddValueTxnAmt[3];
	BYTE ucEPDUpdateFlag;
	BYTE ucRFU2[38];
}AuthStudentSetValue_APDU_In;

typedef struct __packed_back{
	BYTE ucTxnSN[3];
	BYTE ucEV[3];
	BYTE ucConfirmCode[2];
	BYTE ucSignature[16];
	BYTE ucCPUSAMID[8];
	BYTE ucMAC_HCrypto[18];
	BYTE ucTxnDateTime[4];
	BYTE ucNewPersonalExpiryDate[4];
	BYTE ucNewCardType;
	BYTE ucExtAddValueTxnAmt[3];
	BYTE ucRFU2[34];
	BYTE ucStatusCode[2];
}AuthStudentSetValue_APDU_Out;
/***************************AuthStudentSetValue*****************************/

//add by bean 20141023 start
typedef struct __packed_back{
	BYTE ucCardCheckFlag[2];
	BYTE ucEZCard_SAMSlot_Address;
	BYTE ucIPASS_SAMSlot_Address;
	BYTE ucSAMSlot_Address_RFU1;
	BYTE ucSAMSlot_Address_RFU2;
	BYTE ucSAMSlot_Address_RFU3;
	BYTE ucCardPollingMode;
	BYTE ucPollingTime;
	BYTE ucRFU[11];
}MMSelectCard2_APDU_In;

typedef struct __packed_back{
	BYTE ucCardIDLen;
	BYTE ucCardID[7];
	BYTE ucATQA[2];
	BYTE ucSAK;
	BYTE ucCardType;
	BYTE ucRFU[20];
	BYTE ucComfirmCode[2];
}MMSelectCard2_APDU_Out;
//add by bean 20141023 end

/*****************************TaxiRead****************************/
typedef struct __packed_back{
	BYTE ucLCDControlFlag;
	BYTE ucTMSerialNumber[6];
	BYTE ucTxnDateTime[4];
	BYTE ucRFU[21];
}TaxiRead_APDU_In;

typedef struct __packed_back{
	BYTE ucPurseVersionNumber;
#ifdef ECC_BIG_ENDIAN
	BYTE bRFU7			:1;
	BYTE bRFU6			:1;
	BYTE bRFU5			:1;
	BYTE bCredit		:1;
	BYTE bAutoLoad		:1;
	BYTE bRefunded		:1;
	BYTE bBlocked		:1;
	BYTE bActivated 	:1;
#else
	BYTE bActivated 	:1;
	BYTE bBlocked		:1;
	BYTE bRefunded		:1;
	BYTE bAutoLoad		:1;
	BYTE bCredit		:1;
	BYTE bRFU5			:1;
	BYTE bRFU6			:1;
	BYTE bRFU7			:1;
#endif
	BYTE ucAutoLoadAmt[3];
	BYTE ucPID[8];
	BYTE ucSubAreaCode[2];
	BYTE ucExpiryDate[4];
	BYTE ucEV[3];
	BYTE ucTxnSN[3];
	BYTE ucCardType;
	BYTE ucPersonalProfile;
	BYTE ucProfileExpiryDate[4];
	BYTE ucAreaCode;
	BYTE ucCardID[7];
	BYTE ucCardIDLen;
	BYTE ucDeviceID[4];
	BYTE ucCPUDeviceID[6];
	BYTE ucSPID;
	BYTE ucCPUSPID[3];
	BYTE ucLocationID;
	BYTE ucCPULocationID[2];
	BYTE ucIssuerCode;
	BYTE ucBankCode;
	BYTE ucLoyaltyCounter[2];
	LastTxnLog_t stLastCreditTxnLogInfo_t;//33
	BYTE ucAutoloadCounter;
	BYTE ucAutoloadDate[2];
	BYTE ucCardOneDayQuota[3];
	BYTE ucCardOneDayQuotaDate[2];
	BYTE ucURT[24];
	BYTE ucSocial_Code[16]; 								//身份證號碼
	BYTE ucTheRemainderOfAddQuota[3];					//剩餘加值額度
	BYTE ucPersonalProfileAuth[1];
	BYTE ucAccFreeRides[2];								//社福卡免費搭乘累積優惠點數
	BYTE ucAccFreeRidesDate[2];							//社福卡免費搭乘交易日期
	BYTE ucRFU[10];										//保留
	BYTE ucStatusCode[2];
}TaxiRead_APDU_Out;
/*****************************TaxiRead****************************/

/****************************TaxiDeduct***************************/
typedef struct __packed_back{
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucTMLocationID[10];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE ucTxnDateTime[4];
	BYTE ucTxnAmt[3];
	BYTE ucAutoLoad; 			 //TM是否進行自動?值-0x00：否-0x01：是
	BYTE ucTXNType;				 //TM交易方式-0x20
	BYTE ucTransferGroupCode;	 //TM本運具之轉乘群組代碼-0x00無轉乘
	BYTE ucTransferDiscount[2];
	BYTE ucPersonalDiscount[2];
	BYTE ucAccumulatedFreeRidesWriteFlag; // 社福卡免費搭乘累積優惠點數寫入旗標  0x00 不寫入	0x01 寫入
	BYTE ucAccumulatedFreeRides[2];
	BYTE ucLCDControlFlag;		 //TM用於控制交易完成後之LCD顯示0x00：顯示【交易完成　請取卡】(default)0x01：顯示【（請勿移動票卡）】
	BYTE ucRFU[24]; //TM保留，補0x00，15bytes
}TaxiDeduct_APDU_In;

typedef struct __packed_back{
	BYTE ucSignatureKeyKVN;
	BYTE ucCPUSAMID[8];
	BYTE ucHashType;
	BYTE ucHostAdminKVN;
	BYTE ucAutoloadMsgType;
	BYTE ucAutoloadSubType;
	DeductOut_t stAutoloadOut_t;
	BYTE ucDeductMsgType;
	BYTE ucDeductSubType;
	BYTE ucTransferGuoupCode;
	BYTE ucTransferGuoupCode2[2];
	DeductOut_t stDeductOut_t;
	BYTE ucRFU[22];
	BYTE ucStatusCode[2];
}TaxiDeduct_APDU_Out;
/****************************TaxiDeduct***************************/

/****************************THSRCRead****************************/
typedef struct __packed_back{
	BYTE ucLCDControlFlag;
	BYTE ucTMSerialNumber[6];
	BYTE ucTxnDateTime[4];
	BYTE ucRFU[5];
}THSRCRead_APDU_In;

typedef struct __packed_back{
	BYTE ucPurseVersionNumber;
#ifdef ECC_BIG_ENDIAN
	BYTE bRFU7			:1;
	BYTE bRFU6			:1;
	BYTE bRFU5			:1;
	BYTE bCredit			:1;
	BYTE bAutoLoad			:1;
	BYTE bRefunded			:1;
	BYTE bBlocked			:1;
	BYTE bActivated 		:1;
#else
	BYTE bActivated 		:1;
	BYTE bBlocked			:1;
	BYTE bRefunded			:1;
	BYTE bAutoLoad			:1;
	BYTE bCredit			:1;
	BYTE bRFU5			:1;
	BYTE bRFU6			:1;
	BYTE bRFU7			:1;
#endif
	BYTE ucAutoLoadAmt[3];
	BYTE ucPID[8];
	BYTE ucSubAreaCode[2];
	BYTE ucExpiryDate[4];
	BYTE ucEV[3];
	BYTE ucTxnSN[3];
	BYTE ucCardType;
	BYTE ucPersonalProfile;
	BYTE ucProfileExpiryDate[4];
	BYTE ucAreaCode;
	BYTE ucCardID[7];
	BYTE ucCardIDLen;
	BYTE ucDeviceID[4];
	BYTE ucCPUDeviceID[6];
	BYTE ucSPID;
	BYTE ucCPUSPID[3];
	BYTE ucLocationID;
	BYTE ucCPULocationID[2];
	BYTE ucIssuerCode;
	BYTE ucBankCode;
	BYTE ucLoyaltyCounter[2];
	LastTxnLog_t stLastCreditTxnLogInfo_t;//33
	BYTE ucAutoloadCounter;
	BYTE ucAutoloadDate[2];
	BYTE ucURT[26];
	BYTE ucSocial_Code[16]; 								//身份證號碼
	BYTE ucTHSRCFixedData[26];
	BYTE ucTHSRCVariableData[26];
	BYTE ucTheRemainderOfAddQuota[3];					//剩餘加值額度
	BYTE ucRFU[56];										//保留
	BYTE ucStatusCode[2];
}THSRCRead_APDU_Out;
/****************************THSRCRead****************************/

/*****************************VerifyHCrypt****************************/
typedef struct __packed_back{
	BYTE ucPID[8];
	BYTE ucPCode[3];
	BYTE ucTxnAmt[3];
	BYTE ucEV[3];
	BYTE unEVBeforeTxn[3];
	BYTE ucTMSerialNumber_3[3];
	BYTE ucCardSN[3];
	BYTE ucTxnDateTime[4];
	BYTE ucAuthCode[16];
	BYTE ucCPUDeviceID[6];
	BYTE ucMerchantID[4];
	BYTE ucTMLocationID[10];
	BYTE ucRFU[14];
	BYTE ucHCrypto[16];
}VerifyHCrypt_APDU_In;
/*****************************VerifyHCrypt****************************/

/****************************CBikeRead****************************/
typedef struct __packed_back{
	BYTE ucLCDControlFlag;
	BYTE ucTMSerialNumber[6];
	BYTE ucTxnDateTime[4];
	BYTE ucRFU[5];
}CBikeRead_APDU_In;

typedef struct __packed_back{
	BYTE ucPurseVersionNumber;
#ifdef ECC_BIG_ENDIAN
	BYTE bRFU7			:1;
	BYTE bRFU6			:1;
	BYTE bRFU5			:1;
	BYTE bCredit			:1;
	BYTE bAutoLoad			:1;
	BYTE bRefunded			:1;
	BYTE bBlocked			:1;
	BYTE bActivated 		:1;
#else
	BYTE bActivated 		:1;
	BYTE bBlocked			:1;
	BYTE bRefunded			:1;
	BYTE bAutoLoad			:1;
	BYTE bCredit			:1;
	BYTE bRFU5			:1;
	BYTE bRFU6			:1;
	BYTE bRFU7			:1;
#endif
	BYTE ucAutoLoadAmt[3];
	BYTE ucPID[8];
	BYTE ucSubAreaCode[2];
	BYTE ucExpiryDate[4];
	BYTE ucEV[3];
	BYTE ucTxnSN[3];
	BYTE ucCardType;
	BYTE ucPersonalProfile;
	BYTE ucProfileExpiryDate[4];
	BYTE ucAreaCode;
	BYTE ucCardID[7];
	BYTE ucCardIDLen;
	BYTE ucDeviceID[4];
	BYTE ucCPUDeviceID[6];
	BYTE ucSPID;
	BYTE ucCPUSPID[3];
	BYTE ucLocationID;
	BYTE ucCPULocationID[2];
	BYTE ucIssuerCode;
	BYTE ucBankCode;
	BYTE ucLoyaltyCounter[2];
	LastTxnLog_t stLastCreditTxnLogInfo_t;//33
	BYTE ucAutoloadCounter;
	BYTE ucAutoloadDate[2];
	BYTE ucCardOneDayQuota[3];
	BYTE ucCardOneDayQuotaDate[2];
	BYTE ucURT[26];
	BYTE ucSocial_Code[16]; 								//身份證號碼
	BYTE ucTheRemainderOfAddQuota[3];					//剩餘加值額度
	BYTE ucAccFreeRides[2];
	BYTE ucAccFreeRidesDate[2];
	BYTE ucPersonalProfileAuthorization[1];
	BYTE ucRentalFlag[1];
	BYTE ucTransactionDateAndTime[4];
	BYTE ucCBikePlateNumber[3];
	BYTE ucAreaCodeOfMembership[1];
	BYTE ucRFU[63];
	BYTE ucStatusCode[2];
}CBikeRead_APDU_Out;
/****************************CBikeRead****************************/

/****************************CBikeDeduct***************************/
typedef struct __packed_back{
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucTMLocationID[10];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE ucTxnDateTime[4];
	BYTE ucTxnAmt[3];
	BYTE ucAutoLoad; 			 //TM是否進行自動?值-0x00：否-0x01：是
	BYTE ucTXNType;				 //TM交易方式-0x20
	BYTE ucTransferGroupCode;	 //TM本運具之轉乘群組代碼-0x00無轉乘
	BYTE ucTransferDiscount[2];
	BYTE ucPersonalDiscount[2];
	BYTE ucAccumulatedFreeRidesWriteFlag; // 社福卡免費搭乘累積優惠點數寫入旗標  0x00 不寫入	0x01 寫入
	BYTE ucAccumulatedFreeRides[2];
	BYTE ucRentalFlag[1];
	BYTE ucTransactionDateAndTime[4];
	BYTE ucCBikePlateNumber[3];
	BYTE ucAreaCodeOfMembership[1];
	BYTE ucRFU[31]; //TM保留，補0x00，15bytes
	BYTE ucLCDControlFlag;		 //TM用於控制交易完成後之LCD顯示0x00：顯示【交易完成　請取卡】(default)0x01：顯示【（請勿移動票卡）】
}CBikeDeduct_APDU_In;

typedef struct __packed_back{
	BYTE ucSignatureKeyKVN;
	BYTE ucCPUSAMID[8];
	BYTE ucHashType;
	BYTE ucHostAdminKVN;
	BYTE ucTransferGuoupCode;
	BYTE ucTransferGuoupCode2[2];
	BYTE ucAutoloadMsgType;
	BYTE ucAutoloadSubType;
	DeductOut_t stAutoloadOut_t;
	BYTE ucDeductMsgType;
	BYTE ucDeductSubType;
	DeductOut_t stDeductOut_t;
	BYTE ucRFU[22];
	BYTE ucStatusCode[2];
}CBikeDeduct_APDU_Out;
/****************************CBikeDeduct***************************/

/****************************EDCARead****************************/
typedef struct __packed_back{
	BYTE ucLCDControlFlag;
	BYTE ucTMSerialNumber[6];
	BYTE ucTxnDateTime[4];
#ifdef ECC_BIG_ENDIAN
	BYTE RFU_Flag:5;
	BYTE ReadDateOfFirstTransactionFlag:1;
	BYTE ReadAccPointsMode:1;
	BYTE ReadAccPointsFlag:1;
#else
	BYTE ReadAccPointsFlag:1;
	BYTE ReadAccPointsMode:1;
	BYTE ReadDateOfFirstTransactionFlag:1;
	BYTE RFU_Flag:5;
#endif
	BYTE ucRFU[4];
}EDCARead_APDU_In;

typedef struct __packed_back{
	BYTE ucPurseVersionNumber;
#ifdef ECC_BIG_ENDIAN
	BYTE bRFU7			:1;
	BYTE bRFU6			:1;
	BYTE bRFU5			:1;
	BYTE bCredit			:1;
	BYTE bAutoLoad			:1;
	BYTE bRefunded			:1;
	BYTE bBlocked			:1;
	BYTE bActivated 		:1;
#else
	BYTE bActivated 		:1;
	BYTE bBlocked			:1;
	BYTE bRefunded			:1;
	BYTE bAutoLoad			:1;
	BYTE bCredit			:1;
	BYTE bRFU5			:1;
	BYTE bRFU6			:1;
	BYTE bRFU7			:1;
#endif
	BYTE ucAutoLoadAmt[3];
	BYTE ucPID[8];
	BYTE ucSubAreaCode[2];
	BYTE ucExpiryDate[4];
	BYTE ucEV[3];
	BYTE ucTxnSN[3];
	BYTE ucCardType;
	BYTE ucPersonalProfile;
	BYTE ucProfileExpiryDate[4];
	BYTE ucAreaCode;
	BYTE ucCardID[7];
	BYTE ucCardIDLen;
	BYTE ucDeviceID[4];
	BYTE ucCPUDeviceID[6];
	BYTE ucSPID;
	BYTE ucCPUSPID[3];
	BYTE ucLocationID;
	BYTE ucCPULocationID[2];
	BYTE ucIssuerCode;
	BYTE ucBankCode;
	BYTE ucLoyaltyCounter[2];
	LastTxnLog_t stLastCreditTxnLogInfo_t;//33
	BYTE ucAutoloadCounter;
	BYTE ucAutoloadDate[2];
	BYTE ucCardOneDayQuota[3];
	BYTE ucCardOneDayQuotaDate[2];
	BYTE ucURT[26 - 2];
	BYTE ucSocial_Code[16]; 								//身份證號碼
	BYTE ucTheRemainderOfAddQuota[3];					//剩餘加值額度
	BYTE ucAccFreeRides[2];
	BYTE ucAccFreeRidesDate[2];
	BYTE ucFareProductExpireDate[2];					//北捷月票到期日
	BYTE ucPersonalProfileAuthorization[1];
	BYTE ucDateOfFirstTransaction[2];					//首次使用日期(台中花博)DOS date
	BYTE ucRFU[6];
	BYTE ucStatusCode[2];
}EDCARead_APDU_Out;
/****************************EDCARead****************************/

/****************************EDCADeduct***************************/
typedef struct __packed_back{
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucTMLocationID[10];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE ucTxnDateTime[4];
	BYTE ucTxnAmt[3];
	BYTE ucAutoLoad; 			 //TM是否進行自動?值-0x00：否-0x01：是
	BYTE ucTXNType;				 //TM交易方式-0x20
	BYTE ucTransferGroupCode;	 //TM本運具之轉乘群組代碼-0x00無轉乘
	BYTE ucTransferDiscount[2];
	BYTE ucPersonalDiscount[2];
	BYTE ucAccumulatedFreeRides[2];
#ifdef ECC_BIG_ENDIAN
	BYTE RFU_Flag:6;
	BYTE WriteDateOfFirstTransactionFlag:1;
	BYTE WriteAccPointsFlag:1;
#else
	BYTE WriteAccPointsFlag:1;
	BYTE WriteDateOfFirstTransactionFlag:1;
	BYTE RFU_Flag:6;
#endif
	BYTE ucEntryroute[2];//此欄位用在特種票扣款,花博暫無此交易,先不處理!!
	BYTE ucZonenumber[1];//此欄位用在特種票扣款,花博暫無此交易,先不處理!!
	BYTE ucFareProductType[1];//此欄位用在特種票扣款,花博暫無此交易,先不處理!!
	BYTE ucCurrentusednumberofjourneys[1];//此欄位用在特種票扣款,花博暫無此交易,先不處理!!
	BYTE ucDuration[1];//此欄位用在特種票扣款,花博暫無此交易,先不處理!!
	BYTE ucDateOfFirstTransaction[2];
	BYTE ucLCDControlFlag;		 //TM用於控制交易完成後之LCD顯示0x00：顯示【交易完成　請取卡】(default)0x01：顯示【（請勿移動票卡）】
}EDCADeduct_APDU_In;

typedef struct __packed_back{
	BYTE ucSignatureKeyKVN;
	BYTE ucCPUSAMID[8];
	BYTE ucHashType;
	BYTE ucHostAdminKVN;
	//BYTE ucTransferGuoupCode;
	//BYTE ucTransferGuoupCode2[2];
	BYTE ucAutoloadMsgType;
	BYTE ucAutoloadSubType;
	DeductOut_t stAutoloadOut_t;
	BYTE ucDeductMsgType;
	BYTE ucDeductSubType;
	BYTE ucTransferGuoupCode;
	BYTE ucTransferGuoupCode2[2];
	DeductOut_t stDeductOut_t;
	BYTE ucFareDeductMsgType;
	BYTE ucFareDeductSubType;
	BYTE ucFareProductType;
	DeductOut_t stFareDeductOut_t;
	BYTE ucStatusCode[2];
}EDCADeduct_APDU_Out;
/****************************EDCADeduct***************************/

/****************************FastReadCard****************************/
typedef struct __packed_back{
	BYTE ucAntennaFlag[1];
	BYTE ucPVNReadFlag[1];
}FastReadCard_APDU_In;

typedef struct __packed_back{
	BYTE ucSAK[1];
	BYTE ucPurseVersionNumber;
	BYTE ucCardID[7];
	BYTE ucCardIDLen;
	BYTE ucStatusCode[2];
}FastReadCard_APDU_Out;
/****************************FastReadCard****************************/

int inBuildNullAPDU(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildResetCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildSignOnCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildReadCardBasicDataCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildTxnReqOnlineCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildAuthTxnOnlineCommand1(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildAuthTxnOnlineCommand2(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildTxnReqOfflineCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildTxnReqOfflineCommand2(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildAuthTxnOfflineCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildLockCardCommand1(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildLockCardCommand2(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildReadDongleDeductCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildSetValueCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildAuthSetValueCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildTxRefundCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildAuthTxRefundCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildAddValueCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildAuthAddValueCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildTxAuthAddValueCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildAutoloadEnableCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildAuthAutoloadEnableCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildReadCardDeductCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildSetValueCommand2(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildAuthSetValueCommand2(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildTxRefundCommand2(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildAuthTxRefundCommand2(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildAutoLoadEnableCommand2(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildAuthAutoLoadEnableCommand2(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildAuthAutoLoadCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildReadCardNumberCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildFareSaleCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildVoidFareCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildRefundFareCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildReadTRTCCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildTRTCSetValueCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildSetValueOfflineCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildStudentSetValueCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildAuthStudentSetValueCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildMMSelectCard2Command(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildResetOffCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildAuthAccuntLinkCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildTaxiReadCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildTHSRCReadCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildCBikeReadCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildEDCAReadCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildFastReadCardAPDU(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);

#ifdef __cplusplus
	}
#endif // __cplusplus

#endif

