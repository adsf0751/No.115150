#ifndef __SIS2_H__
#define __SIS2_H__

#ifdef __cplusplus
	extern "C" {
#endif


/****************************表頭/表尾*****************************/
typedef struct{
	BYTE ucPurseVersionNumber;
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucDeviceID_3[3];
	BYTE ucSPID;
	BYTE ucTxnDateTime[4];
	BYTE ucCardID[7];
	BYTE ucIssuerCode;
	BYTE ucTxnSN_1;
	BYTE ucTxnAmt[3];
	BYTE ucEV[3];
	BYTE ucLocationID;
	BYTE ucCardIDLen;
	BYTE ucTxnSN[3];
	BYTE ucCPUDeviceID[6];
	BYTE ucCPUSPID[3];
	BYTE ucCPULocationID[2];
	BYTE ucNewSubType;
	BYTE ucPersonalProfile;
	BYTE ucNewPersonalProfile;
	BYTE ucACQID;
	BYTE ucPID[8];
	BYTE ucCTC[3];
	BYTE ucAreaCode;
	BYTE ucSubAreaCode[2];
	BYTE ucTxnSN_Before[3];
	BYTE ucEV_Before[3];
}SIS2_HEADER;

typedef struct{
	BYTE ucMAC[10];
	BYTE ucCPUSAMID[8];
	BYTE ucHashType;
	BYTE ucHostAdminKVN;
	BYTE ucHCrypto[16];
}SIS2_END;
/****************************表頭/表尾*****************************/

/*******************************取消加值表身*******************************/
typedef struct{
	BYTE ucLoyaltyCounter[2];
	BYTE ucAgentNumber[2];
	BYTE ucBandCode;
	BYTE ucCPUSPID[3];
	BYTE ucMachineID[20];
	BYTE ucCMID[20];
	BYTE ucRFU1[34];
	//BYTE ucRFU1[74];
	BYTE ucTxnMode;
	BYTE ucTxnQuqlifier;
	BYTE ucSignatureKeyKVN;
	BYTE ucSignature[16];
}SIS2_BODY_VOID;

#define SIS2_TXN_BLOB_9_LEN		sizeof(SIS2_HEADER) + sizeof(SIS2_BODY_VOID) + sizeof(SIS2_END)


/*******************************退卡表身*******************************/
typedef struct{
	BYTE ucAgentNumber[2];
	BYTE ucDepost[3];
	BYTE ucCardRefundFee[2];
	BYTE ucCPUBrokenFee[2];
	BYTE ucCardCustomerFee[2];
	BYTE ucRefFee[2];
	//BYTE ucRFU[54];
	BYTE ucMachineID[20];
	BYTE ucRFU[34];
	BYTE ucTxnMode;
	BYTE ucTxnQuqlifier;
	BYTE ucSignatureKeyKVN;
	BYTE ucSignature[16];
}SIS2_BODY_REFUND_CARD;

#define SIS2_TXN_BLOB_8_LEN 	sizeof(SIS2_HEADER) + sizeof(SIS2_BODY_REFUND_CARD) + sizeof(SIS2_END)
/*******************************退卡表身*******************************/

/*******************************退票表身*******************************/
typedef struct{
	BYTE ucLoyaltyCounter[2];
	BYTE ucFareProductType;
	BYTE ucRFU[34];
	BYTE ucTxnMode;
	BYTE ucTxnQuqlifier;
	BYTE ucSignatureKeyKVN;
	BYTE ucSignature[16];
}SIS2_BODY_REFUND_FARE;

#define SIS2_TXN_BLOB_7_LEN		sizeof(SIS2_HEADER) + sizeof(SIS2_BODY_REFUND_FARE) + sizeof(SIS2_END)
/*******************************退票表身*******************************/

/*******************************取消售票表身*******************************/
typedef struct{
	BYTE ucLoyaltyCounter[2];
	BYTE ucAgentNumber[2];
	BYTE ucFareProductType;
	BYTE ucRFU[34];
	BYTE ucTxnMode;
	BYTE ucTxnQuqlifier;
	BYTE ucSignatureKeyKVN;
	BYTE ucSignature[16];
}SIS2_BODY_VOID_FARE;

#define SIS2_TXN_BLOB_6_LEN		sizeof(SIS2_HEADER) + sizeof(SIS2_BODY_VOID_FARE) + sizeof(SIS2_END)
/*******************************取消售票表身*******************************/

/*******************************售票表身*******************************/
typedef struct{
	BYTE ucLoyaltyCounter[2];
	BYTE ucAgentNumber[2];
	BYTE ucFareProductType;
	BYTE ucGroupsize;
	BYTE ucEntryStation[2];
	BYTE ucExitStation[2];
	BYTE ucJourneyNumber[2];
	BYTE ucFirstDate[2];
	BYTE ucLastDate[2];
	BYTE ucCarNo[10];
	BYTE ucDuration[2];
	BYTE ucAuthoriseGroup[2];
	BYTE ucRFU[34];
	BYTE ucTxnMode;
	BYTE ucTxnQuqlifier;
	BYTE ucSignatureKeyKVN;
	BYTE ucSignature[16];
}SIS2_BODY_FARE_SALE;

#define SIS2_TXN_BLOB_5_LEN		sizeof(SIS2_HEADER) + sizeof(SIS2_BODY_FARE_SALE) + sizeof(SIS2_END)
/*******************************售票表身*******************************/

/*******************************罰款表身*******************************/
typedef struct{
	BYTE ucAgentNumber[2];
	BYTE ucRFU1[34];
	BYTE ucTxnMode;
	BYTE ucTxnQuqlifier;
	BYTE ucSignatureKeyKVN;
	BYTE ucSignature[16];
}SIS2_BODY_PENALTY;

#define SIS2_TXN_BLOB_4_LEN		sizeof(SIS2_HEADER) + sizeof(SIS2_BODY_PENALTY) + sizeof(SIS2_END)
/*******************************罰款表身*******************************/

/*******************************售卡表身*******************************/
typedef struct{
	BYTE ucAgentNumber[2];
	BYTE ucCPUSPID[3];
	BYTE ucDeposit[3];
	BYTE ucRFU1[34];
}SIS2_BODY_SALE_CARD;

#define SIS2_TXN_BLOB_3_LEN		sizeof(SIS2_HEADER) + sizeof(SIS2_BODY_SALE_CARD) + sizeof(SIS2_END)
/*******************************售卡表身*******************************/

/*******************************加值表身*******************************/
typedef struct{
	BYTE ucLoyaltyCounter[2];
	BYTE ucAgentNumber[2];
	BYTE ucBandCode;
	BYTE ucCPUSPID[3];
	BYTE ucMachineID[20];
	BYTE ucCMID[20];
	BYTE ucRFU1[34];
	//BYTE ucRFU1[74];
	BYTE ucTxnMode;
	BYTE ucTxnQuqlifier;
	BYTE ucSignatureKeyKVN;
	BYTE ucSignature[16];
}SIS2_BODY_ADD;

#define SIS2_TXN_BLOB_2_LEN		sizeof(SIS2_HEADER) + sizeof(SIS2_BODY_ADD) + sizeof(SIS2_END)

/*******************************加值表身*******************************/

/*******************************扣款表身*******************************/
typedef struct{
	BYTE ucEntryExit[1];
	BYTE ucRFU1_0[6];
	BYTE ucTransferGuoupCode;
	BYTE ucTransferGuoupCode2[2];
	BYTE ucRFU1_1[7];
	BYTE ucTransferDiscount[3];
	BYTE ucPersonalDiscount[3];
	BYTE ucRFU1_2[16];
	//BYTE ucRFU1[38];
	BYTE ucLoyaltyCounter[2];
	BYTE ucAgentNumber[2];
	BYTE ucRFU2[1];
	BYTE ucRFU3[30];
	BYTE ucRFU4[16];
	BYTE ucRFU5[1];
	BYTE ucTxnMode;
	BYTE ucTxnQuqlifier;
	BYTE ucSignatureKeyKVN;
	BYTE ucSignature[16];
}SIS2_BODY_DEDUCT;

#define SIS2_TXN_BLOB_1_LEN		sizeof(SIS2_HEADER) + sizeof(SIS2_BODY_DEDUCT) + sizeof(SIS2_END)
/*******************************扣款表身*******************************/

/*******************************加值重送表身*******************************/
typedef struct{
	BYTE ucPurseVersionNumber;
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucCPUDeviceID[6];
	BYTE ucCPUSPID[3];
	BYTE ucTxnDateTime[4];
	BYTE ucCardID[7];
	BYTE ucIssuerCode;
	BYTE ucTxnSN[3];
	BYTE ucTxnAmt[3];
	BYTE ucEV[3];
	BYTE ucCPULocationID[2];
	BYTE ucCardIDLen;
	BYTE ucPersonalProfile;
	BYTE ucPID[8];
	BYTE ucBankCode;
	BYTE ucLoyaltyCounter[2];
	BYTE ucReDeviceID[3];
	BYTE ucReSPID;
	BYTE ucReLocationID;
	BYTE ucReCPUDeviceID[6];
	BYTE ucReCPUSPID[3];
	BYTE ucReCPULocationID[2];
}SIS2_ADD_VALUE_RESEND;

#define SIS2_ADD_VALUE_RESEND_LEN		sizeof(SIS2_ADD_VALUE_RESEND)
/*******************************加值重送表身*******************************/

/*******************************鎖卡回饋表身*******************************/
typedef struct{
	BYTE ucPurseVersionNumber;
	BYTE ucMsgType;
	BYTE ucPersonalProfile;
	BYTE ucDeviceID[3];
	BYTE ucSPID;
	BYTE ucTxnDateTime[4];
	BYTE ucCardID[7];
	BYTE ucIssuerCode;
	BYTE ucLocationID;
	BYTE ucCardIDLen;
	BYTE ucCPUDeviceID[6];
	BYTE ucCPUSPID[3];
	BYTE ucCPULocationID[2];
	BYTE ucNewSubType;
	BYTE ucNewPersonalProfile;
	BYTE ucPID[8];
	BYTE ucCTC[3];
	BYTE ucLockReason;
	BYTE ucBLCName[20];
	BYTE ucLockIDFlag;
	BYTE ucDevInfo[20];
	BYTE ucEV[3];
	BYTE ucTxnSN[3];
	BYTE ucLastTxnSN[3];
	BYTE ucLastTxnDateTime[4];
	BYTE ucLastSubType;
	BYTE ucLastTxnAmt[3];
	BYTE ucLastEV[3];
	BYTE ucLastCPUDeviceID[6];
	BYTE ucEV2[3];
	BYTE ucMifareSetPar;
	BYTE ucCPUSetPar;
	BYTE ucRFU[16];
}SIS2_LOCK;

#define SIS2_LOCK_LEN		sizeof(SIS2_LOCK)
/*******************************鎖卡回饋表身*******************************/

int inBuildSIS2Header(BYTE *bOutData, BYTE ucPurseVersionNumber,
									BYTE ucMsgType,
									BYTE ucSubType,
									BYTE *ucDeviceID_3,
									BYTE ucSPID,
									BYTE *ucTxnDateTime,
									BYTE *ucCardID,
									BYTE ucIssuerCode,
									BYTE ucTxnSN_1,
									BYTE *ucTxnAmt,
									BYTE *ucEV,
									BYTE ucLocationID,
									BYTE ucCardIDLen,
									BYTE *ucTxnSN,
									BYTE *ucCPUDeviceID,
									BYTE *ucCPUSPID,
									BYTE *ucCPULocationID,
									BYTE ucNewSubType,
									BYTE ucPersonalProfile,
									BYTE ucNewPersonalProfile,
									BYTE ucACQID,
									BYTE *ucPID,
									BYTE *ucCTC,
									BYTE ucAreaCode,
									BYTE *ucSubAreaCode,
									BYTE *ucTxnSN_Before,
									BYTE *ucEV_Before);
int inBuildSIS2End(BYTE *bOut_Data,BYTE ucPurseVersionNumber,
								 BYTE *ucMAC,
								 BYTE *ucCPUSAMID,
								 BYTE ucHashType,
								 BYTE ucHostAdminKVN,
								 BYTE *ucHCrypto,
								 BYTE *ucDeviceID);
int inBuildSIS2Body_Void(BYTE *bOut_Data,BYTE *ucLoyaltyCounter,
									  BYTE *ucAgentNumber,
									  BYTE ucBandCode,
									  BYTE *ucCPUSPID,
									  BYTE *ucRFU1,
									  BYTE ucTxnMode,
									  BYTE ucTxnQuqlifier,
									  BYTE ucSignatureKeyKVN,
									  BYTE *ucSignature);

int inBuildSIS2Body_Add(BYTE *bOut_Data,BYTE *ucLoyaltyCounter,
									  BYTE *ucAgentNumber,
									  BYTE ucBandCode,
									  BYTE *ucCPUSPID,
									  BYTE *ucRFU1,
									  BYTE ucTxnMode,
									  BYTE ucTxnQuqlifier,
									  BYTE ucSignatureKeyKVN,
									  BYTE *ucSignature);
int inBuildSIS2Body_Deduct(BYTE *bOut_Data,BYTE *ucEntryExit,
										 BYTE *ucRFU1_0,
										 BYTE ucTransferGuoupCode,
										 BYTE *ucTransferGuoupCode2,
										 BYTE *ucRFU1_1,
										 BYTE *ucTransferDiscount,
										 BYTE *ucPersonalDiscount,
										 BYTE *ucRFU1_2,
										 //BYTE *ucRFU1,
										 BYTE *ucLoyaltyCounter,
										 BYTE *ucAgentNumber,
										 BYTE *ucRFU2,
										 BYTE *ucRFU3,
										 BYTE *ucRFU4,
										 BYTE *ucRFU5,
										 BYTE ucTxnMode,
										 BYTE ucTxnQuqlifier,
										 BYTE ucSignatureKeyKVN,
										 BYTE *ucSignature);
int inBuildSIS2AddValueResend(BYTE *bOut_Data,BYTE ucPurseVersionNumber,
											BYTE ucMsgType,
											BYTE ucSubType,
											BYTE *ucCPUDeviceID,
											BYTE *ucCPUSPID,
											BYTE *ucTxnDateTime,
											BYTE *ucCardID,
											BYTE ucIssuerCode,
											BYTE *ucTxnSN,
											BYTE *ucTxnAmt,
											BYTE *ucEV,
											BYTE *ucCPULocationID,
											BYTE ucCardIDLen,
											BYTE ucPersonalProfile,
											BYTE *ucPID,
											BYTE ucBankCode,
											BYTE *ucLoyaltyCounter,
											BYTE *ucReDeviceID,
											BYTE ucReSPID,
											BYTE ucReLocationID,
											BYTE *ucReCPUDeviceID,
											BYTE *ucReCPUSPID,
											BYTE *ucReCPULocationID);
int inBuildSIS2Lock(BYTE *bOut_Data,BYTE ucPurseVersionNumber,
								  BYTE ucMsgType,
								  BYTE ucPersonalProfile,
								  BYTE *ucDeviceID,
								  BYTE ucSPID,
								  BYTE *ucTxnDateTime,
								  BYTE *ucCardID,
								  BYTE ucIssuerCode,
								  BYTE ucLocationID,
								  BYTE ucCardIDLen,
								  BYTE *ucCPUDeviceID,
								  BYTE *ucCPUSPID,
								  BYTE *ucCPULocationID,
								  BYTE ucNewSubType,
								  BYTE ucNewPersonalProfile,
								  BYTE *ucPID,
								  BYTE *ucCTC,
								  BYTE ucLockReason,
								  BYTE *ucBLCName,
								  BYTE ucLockIDFlag,
								  BYTE *ucDevInfo,
								  BYTE *ucEV,
								  BYTE *ucTxnSN,
								  BYTE *ucLastTxnSN,
								  BYTE *ucLastTxnDateTime,
								  BYTE ucLastSubType,
								  BYTE *ucLastTxnAmt,
								  BYTE *ucLastEV,
								  BYTE *ucLastCPUDeviceID,
								  BYTE *ucEV2,
								  BYTE ucMifareSetPar,
								  BYTE ucCPUSetPar,
								  BYTE *ucRFU);

int inSIS2SaveBlob(void);
int inEZHostSaveData(void);
int inMakeSIS2Blob(void);
int inMakeEZHostBatchData(void);

int inBuildAuthTxnOfflineSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildLock1SIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildLock2SIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildTxnOfflineSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *NotUse);
int inBuildFareSaleSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);

#ifdef __cplusplus
	}
#endif // __cplusplus

#endif

