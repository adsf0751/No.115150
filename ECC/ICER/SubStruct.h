#ifndef __SUBSTRUCT_H__
#define __SUBSTRUCT_H__

#ifdef __cplusplus
	extern "C" {
#endif

//TM.h與APDU.h會用到的結構都放在這裡

typedef struct __packed_back{
	BYTE ucOneDayQuotaFlag;
	BYTE ucOneDayQuota[2];
	BYTE ucOnceQuotaFlag;
	BYTE ucOnceQuota[2];
	BYTE ucCheckEVFlag;
	BYTE ucAddQuotaFlag;
	BYTE ucAddQuota[3];
	BYTE ucCheckDeductFlag;
	BYTE ucCheckDeductValue[2];
	BYTE ucDeductLimitFlag;
	BYTE ucAPIVersion[4];
	BYTE ucRFU[5];
}TermHost_t;

typedef struct __packed_back{
	BYTE ucRemainderAddQuota[3];
	BYTE ucDeMAC[8];
	BYTE ucRFU[21];
}TermPara_t;

typedef struct __packed_back{
	BYTE ucPreCPUDeviceID[6];
	BYTE ucPreSTC[4];
	BYTE ucPreTxnDateTime[4];
	BYTE ucPreCreditBalanceChangeFlag;
	BYTE ucPreConfirmCode[2];
	BYTE ucPreCACrypto[16];
}LastSingOn_t;

typedef struct __packed_back{
	BYTE ucSAMUpdateOption;
	BYTE ucNewSAMValue[40];
	BYTE ucUpdateSAMValueMAC[16];
}SAMParaSetting_t;

typedef struct __packed_back{
#ifdef ECC_BIG_ENDIAN
	BYTE bDeductLimitFlag7		:1;
	BYTE bCheckEVFlag6		:1;
	BYTE bSAMSignOnControlFlag5 :1;
	BYTE bSAMSignOnControlFlag4 :1;
	BYTE bOneDayQuotaWrite3 	:1;
	BYTE bOneDayQuotaWrite2 	:1;
	BYTE bCPDReadFlag1		:1;
	BYTE bCPDReadFlag0		:1;

	BYTE bRFU27 		:1;
	BYTE bRFU26 		:1;
	BYTE bRFU25 		:1;
	BYTE bRFU24 		:1;
	BYTE bCheckDeductFlag3		:1;
	BYTE bOnceQuotaFlag2		:1;
	BYTE bOneDayQuotaFlag1		:1;
	BYTE bOneDayQuotaFlag0		:1;
#else
	BYTE bCPDReadFlag0		:1;
	BYTE bCPDReadFlag1		:1;
	BYTE bOneDayQuotaWrite2		:1;
	BYTE bOneDayQuotaWrite3		:1;
	BYTE bSAMSignOnControlFlag4	:1;
	BYTE bSAMSignOnControlFlag5	:1;
	BYTE bCheckEVFlag6		:1;
	BYTE bDeductLimitFlag7		:1;

	BYTE bOneDayQuotaFlag0		:1;
	BYTE bOneDayQuotaFlag1		:1;
	BYTE bOnceQuotaFlag2		:1;
	BYTE bCheckDeductFlag3		:1;
	BYTE bRFU24			:1;
	BYTE bRFU25			:1;
	BYTE bRFU26			:1;
	BYTE bRFU27			:1;
#endif
	BYTE ucOneDayQuota[2];
	BYTE ucOnceQuota[2];
	BYTE ucCheckDeductValue[2];
	BYTE ucAddQuotaFlag;
	BYTE ucAddQuota[3];
}SAMParameter_t;

typedef struct __packed_back{
	BYTE ucSAMVersion;
	BYTE ucSAMUsageControl[3];
	BYTE ucSAMAdminKVN;
	BYTE ucSAMIssuerKVN;
	BYTE ucTagListTable[40];
	BYTE ucSAMIssuerSpecData[32];
}CPUSAMInfo_t;

typedef struct __packed_back{
	BYTE ucPurseVersionNumber_Index;
	BYTE ucTxnSN[3];
	BYTE ucTxnDateTime[4];
	BYTE ucSubType;
	BYTE ucTxnAmt[3];
	BYTE ucEV[3];
	BYTE ucCPUSPID[3];
	BYTE ucCPULocationID[2];
	BYTE ucCPUDeviceID[6];
	BYTE ucRFU;
	BYTE ucTSQN2[3];
	BYTE ucEV2[3];
}LastTxnLog_t;

typedef struct __packed_back{
	BYTE anTxnSN[8];
	BYTE anTxnDateTime[14];
	BYTE anSubType[2];
	BYTE anEVBeforeTxn[8];
	BYTE anTxnAmt[8];
	BYTE anEV[8];
	BYTE anCPUSPID[8];
	BYTE anLocationID[5];
	BYTE anCPUDeviceID[16];
}LastTxnRecored_t;

typedef struct __packed_back{
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucDeviceID[4];
	BYTE ucCPUDeviceID[6];
	BYTE ucTxnDateTime[4];
	BYTE ucPurseVersionNumber;
	BYTE ucCardID[7];
	BYTE ucCardIDLen;
	BYTE ucTxnSN[3];
	BYTE ucEV[3];
	BYTE ucStatusCode[2];
	BYTE ucPID[8];
	BYTE ucSignature[16];
	BYTE ucSAMID[8];
	BYTE ucMAC_HCrypto[18];
}ReaderAVRAPDU_t;

typedef struct __packed_back{
	BYTE ucMsgType;
	BYTE ucSubType;
	BYTE ucDeviceID[4];
	BYTE ucCPUDeviceID[6];
	BYTE ucTxnDateTime[4];
	BYTE ucPurseVersionNumber;
	BYTE ucCardID[7];
	BYTE ucTxnSN[3];
	BYTE ucEV[3];
	BYTE ucStatusCode[2];
	BYTE ucPID[8];
	BYTE ucSignature[16];
	BYTE ucSAMID[8];
	BYTE ucMAC_HCrypto[18];
	BYTE ucCardIDLen;
}ReaderAVRTM_t;

typedef struct __packed_back{
	BYTE ucMsgType;
	BYTE ucPurseVersionNumber;
	BYTE ucTxnSN[3];
	BYTE ucTxnDateTime[4];
	BYTE ucSubType;
	BYTE ucTxnAmt[3];
	BYTE ucEV[3];
	BYTE ucCardID[7];
	BYTE ucIssuerCode;
	BYTE ucCPUSPID[3];
	BYTE ucCPULocationID[2];
	BYTE ucPID[8];
	BYTE ucCPUDeviceID[6];
	BYTE ucLoyaltyCounter[2];
}CardAVR_t;//45

typedef struct __packed_back{
	BYTE ucRAVRMsgType;
	BYTE ucRAVRSubType;
	BYTE ucRAVRDeviceID[4];
	BYTE ucRAVRTxnDateTime[4];
	BYTE ucRAVRCardID[4];
	BYTE ucRAVRTxnSN[2];
	BYTE ucRAVREV[2];
	BYTE ucRAVRMAC[10];
	BYTE ucRAVRConfirmCode[2];
	BYTE ucRAVRCTAC[8];
}ReaderAVR_t;

typedef struct __packed_back{
	BYTE anCardID_10[10];
	BYTE anDeviceID[10];
	BYTE anEVBeforeTxn_5[5];
	BYTE anTxnSN_5[5];
	BYTE anTxnAmt_5[5];
	BYTE anEV_5[5];
	BYTE anProcessignCode[6];
	BYTE unMsgType[2];
	BYTE unSubType[2];
	BYTE unDeviceID[8];
	BYTE unSPID[2];
	BYTE unTxnDateTime[8];
	BYTE unCardType[2];
	BYTE unPersonalProfile[2];
	BYTE unCardID_8[8];
	BYTE unIssuerCode[2];
	BYTE unTxnSN_4[4];
	BYTE unTxnAmt_4[4];
	BYTE unEV_4[4];
	BYTE unLocationID[2];
	BYTE unBankCode[2];
	BYTE unLoyaltyCounter[4];
	BYTE unDeposit_4[4];
	BYTE unMAC[20];
	BYTE anOrgExpiryDate[8];
	BYTE anNewExpiryDate[8];
	BYTE unAreaCode[2];
	BYTE unEVBeforeTxn_4[4];
}SETM_t;

#define SIZE_SETM			sizeof(SETM_t) + TM1_DATA_LEN

typedef struct __packed_back{
	BYTE anCardID[17];
	BYTE anDeviceID[10];
	BYTE anCPUDeviceID[16];
	BYTE anEVBeforeTxn[8];
	BYTE anTxnSN[8];
	BYTE anTxnAmt[8];
	BYTE anEV[8];
	BYTE anProcessignCode[6];
	BYTE unMsgType[2];
	BYTE unSubType[2];
	BYTE unDeviceID[8];
	BYTE unSPID[2];
	BYTE unTxnDateTime[8];
	BYTE unCardType[2];
	BYTE unPersonalProfile[2];
	BYTE unLocationID[2];
	BYTE unCardID[14];
	BYTE unIssuerCode[2];
	BYTE unBankCode[2];
	BYTE unLoyaltyCounter[4];
	BYTE unMAC[20];
	BYTE unOrgExpiryDate[8];
	BYTE unNewExpiryDate[8];
	BYTE unAreaCode[2];
	BYTE unTxnAmt[6];
	BYTE unEV[6];
	BYTE unTxnSN[6];
	BYTE unCPUDeviceID[12];
	BYTE unCPUSPID[6];
	BYTE unCPULocationID[4];
	BYTE unPID[16];
	BYTE unCTC[6];
	BYTE unOrgProfileExpiryDate[8];
	BYTE unNewProfileExpiryDate[8];
	BYTE unSubAreaCode[4];
	BYTE unTxnSNBeforeTxn[6];
	BYTE unEVBeforeTxn[6];
	BYTE unDeposit[6];
	BYTE unTxnMode[2];
	BYTE unTxnQuqlifier[2];
	BYTE unSignatureKeyKVN[2];
	BYTE unSignature[32];
	BYTE unCPUSAMID[16];
	BYTE unHashType[2];
	BYTE unHostAdminKVN[2];
	BYTE unCPUMAC[32];
	BYTE unPurseVersionNumber[2];
	BYTE unCardIDLen[2];
	BYTE ucTMLocationID[10];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
}STMC_t;

#define SIZE_STMC			sizeof(STMC_t)

typedef struct __packed_back{
	BYTE anTxnSN[3];
	BYTE anTxnDateTime[14];
	BYTE anSubType[2];
	BYTE anEVBeforeTxn[5];
	BYTE anTxnAmt[5];
	BYTE anEV[5];
	BYTE anSPID[2];
	BYTE anLocationID[2];
	BYTE anDeviceID[10];
}LastTxnRecoredL1_t;

typedef struct __packed_back{
	BYTE anTxnSN[8];
	BYTE anTxnDateTime[14];
	BYTE anSubType[2];
	BYTE anEVBeforeTxn[8];
	BYTE anTxnAmt[8];
	BYTE anEV[8];
	BYTE anCPUSPID[8];
	BYTE anLocationID[5];
	BYTE anCPUDeviceID[16];
}LastTxnRecoredL2_t;

typedef struct __packed_back{
	BYTE anHeader[3];
	BYTE anProcessignCode[6];
	BYTE unCardID[14];
	BYTE unTxnDateTime[8];
	BYTE unCPUDeviceID[12];
	BYTE anOrgExpiryDate[8];
	BYTE anNewExpiryDate[8];
	BYTE anOrgProfileExpiryDate[8];
	BYTE anNewProfileExpiryDate[8];
	BYTE anTxnSN[8];
	BYTE unMsgType[2];
	BYTE unSubType[2];
	BYTE unCardType[2];
	BYTE unPersonalProfile[2];
	BYTE unAreaCode[2];
	BYTE unIssuerCode[2];
	BYTE unBankCode[2];
	BYTE anOnlineFlag[1];
	BYTE anIssuerAuthCode[12];
	BYTE anACQMerchantID[8];
	BYTE anACQMerchaneDesc[20];
	BYTE anACQLocationID[10];
	BYTE anACQLocationDesc[20];
	BYTE anACQTxnDesc[20];
	BYTE unPID[16];
	BYTE unPurseVersionNumber[2];
	BYTE unRFU[10];
	BYTE unHash[16];
	BYTE anEnd[2];
}TXET_t;//For 學生票展期

#define SIZE_TXET			sizeof(TXET_t)

typedef struct __packed_back{
	BYTE anCardID[17];
	BYTE anDeviceID[10];
	BYTE anCPUDeviceID[16];
	BYTE anEVBeforeTxn[8];
	BYTE unOrgExpiryDate[8];
	BYTE unNewExpiryDate[8];
	BYTE anProcessignCode[6];
}SET_VALUE_STMC_t;

typedef struct __packed_back{
	BYTE ucCTC[3];
	BYTE ucTxnMode;
	BYTE ucTxnQuqlifier;
	BYTE ucSignature[16];
	BYTE ucTxnSN[3];
	BYTE ucTxnAmt[3];
	BYTE ucEV[3];
	BYTE ucMAC_HCrypto[16];
	BYTE ucTxnDateTime[4];
	BYTE ucConfirmCode[2];
}DeductOut_t;

typedef struct __packed_back
{
	BYTE ucTransactionSequenceNumber[ 3 ];//(轉乘交易序號)
	BYTE ucTransactionDate[ 4 ]; //(轉乘交易時間)
	BYTE ucTransactionType[ 1 ]; //(轉乘交易方式)
	BYTE ucValueofTransaction[ 3 ]; //(轉乘交易金額)
	BYTE ucValueaftertransaction[ 3 ]; //(轉乘交易後金額)
	BYTE ucTransfergroupcode[ 2 ];	//(轉乘群組)
	BYTE ucTransactionLocationID[ 2 ];//(轉乘場站代碼)
	BYTE ucTransactionDeviceID[ 6 ];//(轉乘交易設備編號)
	BYTE ucRFU[ 2 ];
}TRANSPORT_DATA;

#ifdef __cplusplus
	}
#endif // __cplusplus

#endif
