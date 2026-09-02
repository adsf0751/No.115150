#ifndef __TM_H__
#define __TM_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include "SubStruct.h"
#include "APDU.h"
#include "XMLFunc.h"

typedef struct __packed_back{//將資料儲存在此,好讓下一個交易使用!!
	BYTE ucDeviceID[4];
	BYTE ucTxnDateTime[4];
	BYTE ucTxnDateTime2[4];
	BYTE ucCPUDeviceID[6];
	BYTE ucSTC[4];
	BYTE ucTMLocationID[10];
	BYTE ucTMID[2];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE ucTMTxnDateTime[14];
	BYTE ucRefundFee[2];
	BYTE ucBrokenFee[2];
	BYTE ucCustomerFee[2];
}Store_Data;

/********************************Reset********************************/
typedef struct __packed_back{
	BYTE ucTMLocationID[10];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE ucLocationID[3];
	BYTE ucCPULocationID[5];
	BYTE ucSPID[3];
	BYTE ucCPUSPID[8];
	BYTE ucSAMSlot[1];
	//BYTE ucAccFreeRidesMode[1];
}Reset_TM_In;

typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];						//Field 1    3
	BYTE ucProcessignCode[6];				//Field 2    6
	BYTE ucBLVersion[2];					//Field 6    2
	BYTE ucMsgType;							//Field 7    1
	BYTE ucSubType;							//Field 8    1
	BYTE ucDeviceID[4];						//Field 9    4
	BYTE ucSPID;							//Field 10   1
	BYTE ucTxnDateTime[4];					//Field 11   4
	BYTE ucTMID[2];							//Field 28   2
	BYTE ucTMTxnDateTime[14];				//Field 29   14
	BYTE ucTMSerialNumber[6];				//Field 30   6
	BYTE ucTMAgentNumber[4];				//Field 31   4
	BYTE ucSTAC[8];							//Field 33   8
	BYTE ucKeyVersion;						//Field 35   1
	BYTE ucSAMID[8];						//Field 36   8
	BYTE ucSAMSN[4];						//Field 37   4
	BYTE ucSAMCRN[8];						//Field 38   8
	BYTE ucReaderFWVersion[6];				//Field 39   6
	BYTE ucNetManagementWorkCode[3];		//Field 46   3
	TermHost_t stTermHostInfo_t;			//Field 50   24
	TermPara_t stTermParaInfo_t;			//Field 51   32
	BYTE ucCardID[7];						//Field 58   7
	BYTE ucTxnAmt[3];						//Field 59   3
	BYTE ucCPUDeviceID[6];					//Field 62   6
	BYTE ucCPUSPID[3];						//Field 63   3
	BYTE ucEVBeforeTxn[3];					//Field 71   3
	BYTE ucCPUSAMID[8];						//Field 79   8
	BYTE ucSTC[4];							//Field 80   4
	CPUSAMInfo_t stCPUSAMInfo_t;			//Field 81   78
	BYTE ucRSAM[8];							//Field 82   8
	BYTE ucRHOST[8];						//Field 83   8
	BYTE ucAuthCreditLimit[3];				//Field 84-1 3
	BYTE ucAuthCreditBalance[3];			//Field 84-2 3
	BYTE ucAuthCreditCumulative[3];			//Field 84-3 3
	BYTE ucAuthCancelCreditCumulative[3];	//Field 84-4 3
	BYTE ucSingleCreditTxnAmtLimit[3];		//Field 85   3
	BYTE ucTMLocationID[10];				//Field 90   10
	BYTE ucSATOKEN[16];						//Field 91   16
	LastSingOn_t stLastSignOnInfo_t;		//Field 93   33
	BYTE ucSAMSignOnControlFlag;			//Field 95   1
	BYTE ucHostSpecVersionNo;				//Field 96   1
	BYTE ucOneDayQuotaWrite;				//Field 97   1
	BYTE ucCPDReadFlag;						//Field 98   1
}Reset_TM_Out;
/********************************Reset********************************/

/*****************************SignOn***********************************/
typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];			//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucMsgType;				//Field 7	//1
	BYTE ucSubType;				//Field 8	//1
	BYTE ucDeviceID[4];			//Field 9	//4
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucTMID[2];				//Field 28	//2
	BYTE ucTMTxnDateTime[14];		//Field 29	//14
	BYTE ucTMSerialNumber[6];		//Field 30	//6
	BYTE ucTMAgentNumber[4];		//Field 31	//4
	BYTE ucHTAC[8];				//Field 34	//8
	BYTE ucResponseCode[2];			//Field 43	//2
	BYTE ucNetManagementWorkCode[3];	//Field 46	//3
	TermHost_t stTermHostInfo_t;		//Field 50	//24
	BYTE ucCardID[7];			//Field 58	//7
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucSTC[4];				//Field 80	//4
	BYTE ucHashType;			//Field 86	//1
	BYTE ucTMLocationID[10];		//Field 90	//10
	BYTE ucHATOKEN[16];			//Field 92	//16
	SAMParaSetting_t stSAMParaSettingInfo_t;//Field 94	//57
	BYTE ucSAMSignOnControlFlag;		//Field 95	//1
	BYTE ucOneDayQuotaWrite;		//Field 97	//1
	BYTE ucCPDReadFlag;			//Field 98	//1
	BYTE ucCPUEDC[3];			//Field 99	//3
}SignOn_TM_In;

typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];			//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 3	//6
	BYTE ucMsgType;				//Field 7	//1
	BYTE ucSubType;				//Field 8	//1
	BYTE ucDeviceID[4];			//Field 9	//4
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucConfirmCode[2];			//Field 47	//2
	BYTE ucCardID[7];			//Field 58	//7
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucSTC[4];				//Field 80	//4
	BYTE ucCreditBalanceChangeFlag;		//Field 100	//1
	BYTE ucCACrypto[16];			//Field 101	//16
}SignOn_TM_Out;
/*****************************SignOn***********************************/

/*****************************SignOnQuery******************************/
typedef struct __packed_back{
	BYTE anAuthCreditLimit[8];
	BYTE anAuthCreditBalance[8];
	BYTE anAuthCreditCumulative[8];
	BYTE anAuthCancelCreditCumulative[8];
	BYTE anCPDReadFlag;
	BYTE anOneDayQuotaWrite;
	BYTE anSAMSignOnControlFlag;
	BYTE anCheckEVFlag;
	BYTE anDeductLimitFlag;
	BYTE anOneDayQuotaFlag;
	BYTE anOneDayQuota[5];
	BYTE anOnceQuotaFlag;
	BYTE anOnceQuota[5];
	BYTE anCheckDeductFlag;
	BYTE anCheckDeductValue[5];
	BYTE anAddQuotaFlag;
	BYTE anAddQuota[8];
	BYTE anRemainderAddQuota[8];
	BYTE anCancelCreditQuota[8];
	BYTE anSPID[3];
	BYTE anCPUSPID[8];
	BYTE anLastTxnDateTime[14];
}SignOnQuery_TM_Out;
/*****************************SignOnQuery******************************/

/**************************ReadCardBasicData***************************/
typedef struct __packed_back{
	BYTE ucLCDControlFlag;
	BYTE ucTMLocationID[10];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE anTxnAmt[8];
}ReadCardBasicData_TM_In;

typedef struct __packed_back{
	BYTE anCardID[10];
	BYTE anEV[6];
	BYTE anIssuerCode[2];
	BYTE anExpiryDate[14];
	BYTE anAreaCode[2];
	BYTE anPersonalProfile[2];
	BYTE anCardType[2];
	BYTE anAutoLoad[3];
	BYTE anAutoLoadAmt[5];
	BYTE anBankCode[2];
	BYTE anLoyaltyCounter[5];
	BYTE anTxnSN[5];
	LastTxnRecoredL1_t stLastCreditTxnRecoredInfo_t;//74
	LastTxnRecoredL1_t stLastDebitTxnRecoredInfo_t;//74
}ReadCardBasicDataL1_TM_Out;

typedef struct __packed_back{
	BYTE ucActivated;
	BYTE ucBlocked;
	BYTE ucRefunded;
	BYTE ucAutoLoad;
	BYTE ucCredit;
	BYTE ucMerchantUseFlag;
	BYTE anAutoLoadAmt[8];
	BYTE anPID[16];
	BYTE anCardID[17];
	BYTE anAreaCode[2];
	BYTE anSubAreaCode[4];
	BYTE anExpiryDate[14];
	BYTE anEV[8];
	BYTE anTxnSN[8];
	BYTE anCardType[2];
	BYTE anPersonalProfile[2];
	BYTE anProfileExpiryDate[14];
	BYTE anDeposit[8];
	BYTE anIssuerCode[2];
	BYTE anBankCode[2];
	BYTE anLoyaltyCounter[5];
	BYTE anCardOneDayQuota[8];
	BYTE anCardOneDayQuotaDate[8];
	BYTE anDebitTxnRecords[2];
	LastTxnRecoredL2_t stLastCreditTxnRecoredInfo_t;//74
	LastTxnRecoredL2_t stLastDebitTxnRecoredInfo_t;//74
	BYTE anPurseVersionNumber[2];
}ReadCardBasicDataL2_TM_Out;
/**************************ReadCardBasicData***************************/

/*****************************DeductValue****************************/
typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];				//Field 1		//3
	BYTE ucProcessignCode[6];		//Field 2		//6
	BYTE ucCommandCode[2];
	BYTE ucBLVersion[2];			//Field 6		//2
	BYTE ucMsgType;					//Field 7		//1
	BYTE ucSubType;					//Field 8		//1
	BYTE ucDeviceID[4];				//Field 9		//4
	BYTE ucSPID;					//Field 10		//1
	BYTE ucTxnDateTime[4];			//Field 11		//4
	BYTE ucCardType;				//Field 12		//1
	BYTE ucPersonalProfile;			//Field 13		//1
	BYTE ucCardID[4];
	BYTE ucIssuerCode;				//Field 15		//1
	BYTE ucTxnAmt[2];
	BYTE ucExpiryDate[4];			//Field 24		//4
	BYTE ucAreaCode;				//Field 26		//1
	BYTE ucTMLocationID[6];
	BYTE ucTMID[2];					//Field 28		//2
	BYTE ucTMTxnDateTime[14];		//Field 29		//14
	BYTE ucTMSerialNumber[6];		//Field 30		//6
	BYTE ucTMAgentNumber[4];		//Field 31		//4
	BYTE ucEV[2];
	BYTE ucSTAC[8];					//Field 33		//8
	BYTE ucKeyVersion;				//Field 35		//1
	BYTE ucSAMID[8];				//Field 36		//8
	BYTE ucSAMSN[4];				//Field 37		//4
	BYTE ucSAMCRN[8];				//Field 38		//8
	BYTE ucReaderFWVersion[6];		//Field 39		//6
	BYTE ucCardAVR[25];
	BYTE ucCardOneDayQuota[2];
	BYTE ucCardOneDayQuotaDate[2];
}DeductValueL1_TM_Out_6415;

typedef struct __packed_back{
	BYTE  unCardID[10];		  		//卡號，換算為ASCII Code
	BYTE  unDeviceID[10];			//設備編號，換算為ASCII Code
	BYTE  unEVBeforeTxn[5];			//交易前餘額，換算為ASCII Code
	BYTE  unTxnSN[5];			  	//交易序號，換算為ASCII Code
	BYTE  unTxnAmt[5];				//交易金額，換算為ASCII Code
	BYTE  unAutoPayFlag[3];			//自動加值授權認證，換算為ASCII Code
	BYTE  unAutoPayAmt[5];			//自動加值金額，換算為ASCII Code
}DeductValueL1_TM_Out_9000;

typedef struct __packed_back{
	BYTE anCardID[17];
	BYTE anDeviceID[10];
	BYTE anCPUDeviceID[16];
	BYTE anEVBeforeTxn[8];
	BYTE anTxnSN[8];
	BYTE anTxnAmt[8];
	BYTE ucAutoLoad;
	BYTE anAutoLoadAmt[8];
}DeductValueL2_TM_Out_9000;

typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];				//Field 1		//3
	BYTE ucProcessignCode[6];		//Field 2		//6
	BYTE ucBLVersion[2];			//Field 6		//2
	BYTE ucMsgType;					//Field 7		//1
	BYTE ucSubType;					//Field 8		//1
	BYTE ucDeviceID[4];				//Field 9		//4
	BYTE ucSPID;					//Field 10		//1
	BYTE ucTxnDateTime[4];			//Field 11		//4
	BYTE ucCardType;				//Field 12		//1
	BYTE ucPersonalProfile;			//Field 13		//1
	BYTE ucIssuerCode;				//Field 15		//1
	BYTE ucExpiryDate[4];			//Field 24		//4
	BYTE ucAreaCode;				//Field 26		//1
	BYTE ucTMID[2];					//Field 28		//2
	BYTE ucTMTxnDateTime[14];		//Field 29		//14
	BYTE ucTMSerialNumber[6];		//Field 30		//6
	BYTE ucTMAgentNumber[4];		//Field 31		//4
	BYTE ucSTAC[8];					//Field 33		//8
	BYTE ucKeyVersion;				//Field 35		//1
	BYTE ucSAMID[8];				//Field 36		//8
	BYTE ucSAMSN[4];				//Field 37		//4
	BYTE ucSAMCRN[8];				//Field 38		//8
	BYTE ucReaderFWVersion[6];		//Field 39		//6
	BYTE ucCardID[7];				//Field 58		//7
	BYTE ucTxnAmt[3];				//Field 59		//3
	BYTE ucCPUDeviceID[6];			//Field 62		//6
	BYTE ucCPUSPID[3];				//Field 63		//3
	BYTE ucCPULocationID[2];		//Field 64		//2
	BYTE ucPID[8];					//Field 65		//8
	BYTE ucCTC[3];					//Field 66		//3
	BYTE ucProfileExpiryDate[4];	//Field 67		//4
	BYTE ucSubAreaCode[2];			//Field 69		//2
	BYTE ucTxnSN[3];				//Field 70		//3
	BYTE ucEV[3];					//Field 71		//3
	BYTE ucCPUAdminKeyKVN;			//Field 74-1	//1
	BYTE ucCreditKeyKVN;			//Field 74-2	//1
	BYTE ucCPUIssuerKeyKVN;			//Field 74-3	//1
	BYTE ucTxnMode;					//Field 75		//1
	BYTE ucTxnQuqlifier;			//Field 76		//1
	BYTE ucCPUSAMID[8];				//Field 79		//8
	BYTE ucHostAdminKVN;			//Field 87		//1
	BYTE ucPurseVersionNumber;		//Field 89		//1
	BYTE ucTMLocationID[10];		//Field 90		//10
	BYTE ucCACrypto[16];			//Field 101		//16
	BYTE ucCardOneDayQuota[3];		//Field 103-1	//3
	BYTE ucCardOneDayQuotaDate[2];	//Field 103-1	//2
	CardAVR_t stCardAVRInfo_t;		//Field 104		//45
	BYTE ucCardIDLen;				//Field 106		//1
}DeductValueL2_TM_Out_6415;
/*****************************DeductValue****************************/

/*******************************AddValue*****************************/
typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];				//Field 1		//3
	BYTE ucProcessignCode[6];		//Field 2		//6
	BYTE ucCommandCode[2];
	BYTE ucBLVersion[2];			//Field 6		//2
	BYTE ucMsgType;					//Field 7		//1
	BYTE ucSubType;					//Field 8		//1
	BYTE ucDeviceID[4];				//Field 9		//4
	BYTE ucSPID;					//Field 10		//1
	BYTE ucTxnDateTime[4];			//Field 11		//4
	BYTE ucCardType;				//Field 12		//1
	BYTE ucPersonalProfile;			//Field 13		//1
	BYTE ucCardID[4];
	BYTE ucIssuerCode;				//Field 15		//1
	BYTE ucTxnAmt[2];
	BYTE ucLocationID;				//0xED
	BYTE ucBankCode;				//銀行代碼
	BYTE ucLoyaltyCounter[2];		//忠誠點
	BYTE ucExpiryDate[4];			//Field 24		//4
	BYTE ucAreaCode;				//Field 26		//1
	BYTE ucTMLocationID[6];
	BYTE ucTMID[2];					//Field 28		//2
	BYTE ucTMTxnDateTime[14];		//Field 29		//14
	BYTE ucTMSerialNumber[6];		//Field 30		//6
	BYTE ucTMAgentNumber[4];		//Field 31		//4
	BYTE ucEV[2];
	BYTE ucSTAC[8];					//Field 33		//8
	BYTE ucKeyVersion;				//Field 35		//1
	BYTE ucSAMID[8];				//Field 36		//8
	BYTE ucSAMSN[4];				//Field 37		//4
	BYTE ucSAMCRN[8];				//Field 38		//8
	BYTE ucReaderFWVersion[6];		//Field 39		//6
	BYTE ucCardAVR[25];
	BYTE ucReaderAVR[38];
}AddValueL1_TM_Out_9000;

/*typedef struct __packed_back{
	BYTE anCardID[17];
	BYTE anDeviceID[10];
	BYTE anCPUDeviceID[16];
	BYTE anEVBeforeTxn[8];
	BYTE anTxnSN[8];
	BYTE anTxnAmt[8];
	BYTE ucAutoLoad;
	BYTE anAutoLoadAmt[8];
}AddValueL2_TM_Out_9000;*/

typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];			//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucBLVersion[2];			//Field 6	//2
	BYTE ucMsgType;				//Field 7	//1
	BYTE ucSubType;				//Field 8	//1
	BYTE ucDeviceID[4];			//Field 9	//4
	BYTE ucSPID;				//Field 10	//1
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucCardType;			//Field 12	//1
	BYTE ucPersonalProfile;			//Field 13	//1
	BYTE ucIssuerCode;			//Field 15	//1
	BYTE ucLocationID;			//Field 19	//1
	BYTE ucBankCode;			//Field 20	//1
	BYTE ucLoyaltyCounter[2];		//Field 21	//2
	BYTE ucExpiryDate[4];			//Field 24	//4
	BYTE ucAreaCode;			//Field 26	//1
	BYTE ucTMID[2];				//Field 28	//2
	BYTE ucTMTxnDateTime[14];		//Field 29	//14
	BYTE ucTMSerialNumber[6];		//Field 30	//6
	BYTE ucTMAgentNumber[4];		//Field 31	//4
	BYTE ucSTAC[8];				//Field 33	//8
	BYTE ucKeyVersion;			//Field 35	//1
	BYTE ucSAMID[8];			//Field 36	//8
	BYTE ucSAMSN[4];			//Field 37	//4
	BYTE ucSAMCRN[8];			//Field 38	//8
	BYTE ucReaderFWVersion[6];		//Field 39	//6
	BYTE ucCardID[7];			//Field 58	//7
	BYTE ucTxnAmt[3];			//Field 59	//3
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucCPUSPID[3];			//Field 63	//3
	BYTE ucCPULocationID[2];		//Field 64	//2
	BYTE ucPID[8];				//Field 65	//8
	BYTE ucCTC[3];				//Field 66	//3
	BYTE ucProfileExpiryDate[4];		//Field 67	//4
	BYTE ucSubAreaCode[2];			//Field 69	//2
	BYTE ucTxnSN[3];			//Field 70	//3
	BYTE ucEV[3];				//Field 71	//3
	BYTE ucDeposit[3];			//Field 72	//3
	BYTE ucCPUAdminKeyKVN;			//Field 74-1	//1
	BYTE ucCreditKeyKVN;			//Field 74-2	//1
	BYTE ucCPUIssuerKeyKVN;			//Field 74-3	//1
	BYTE ucTxnMode;				//Field 75	//1
	BYTE ucTxnQuqlifier;			//Field 76	//1
	BYTE ucSignatureKeyKVN;			//Field 77	//1
	BYTE ucCPUSAMID[8];			//Field 79	//8
	BYTE ucHostAdminKVN;			//Field 87	//1
	BYTE ucPurseVersionNumber;		//Field 89	//1
	BYTE ucTMLocationID[10];		//Field 90	//10
	BYTE ucCACrypto[16];			//Field 101	//16
	CardAVR_t stCardAVRInfo_t;		//Field 104	//45
	ReaderAVRTM_t stReaderAVRInfo_t;	//Field 105	//83
	BYTE ucCardIDLen;			//Field 106	//1
}AddValueL2_TM_Out_6415;

/*******************************AddValue*****************************/

/******************************AuthAddValue****************************/
typedef struct __packed_back{
	//指定加值交易
	SETM_t stAddValueSETM_t;
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];				//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucCommandCode[2];			//Field 3	//2
	BYTE ucMsgType;					//Field 7	//1
	BYTE ucSubType;					//Field 8	//1
	BYTE ucDeviceID[4];				//Field 9	//4
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucCardID_4[4];				//Field 14	//4
	BYTE ucTxnSN_2[2];				//Field 16	//2
	BYTE ucEV_2[2];					//Field 18	//2
	BYTE ucMAC[10];					//Field 23	//10
	BYTE ucConfirmCode[2];			//Field 47	//2
	BYTE ucCTAC[8];					//Field 48	//8
}AuthAddValueL1_TM_Out;

typedef struct __packed_back{
	STMC_t stSTMC_t;
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];			//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucMsgType;				//Field 7	//1
	BYTE ucSubType;				//Field 8	//1
	BYTE ucDeviceID[4];			//Field 9	//4
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucMAC[10];				//Field 23	//10
	BYTE ucConfirmCode[2];			//Field 47	//2
	BYTE ucCTAC[8];				//Field 48	//8
	BYTE ucCardID[7];			//Field 58	//7
	BYTE ucEV[3];				//Field 60	//3
	BYTE ucTxnSN[3];			//Field 61	//3
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucPID[8];				//Field 65	//8
	BYTE ucSignature[16];			//Field 78	//16
	BYTE ucHashType;			//Field 86	//1
	BYTE ucHostAdminKVN;			//Field 87	//1
	BYTE ucCPUMAC[16];			//Field 88	//16
}AuthAddValueL2_TM_Out;

/******************************AuthAddValue****************************/

/*****************************TxnReqOnline*****************************/
typedef struct __packed_back{
	BYTE anMsgType[2];
	BYTE anSubType[2];
	BYTE ucTMLocationID[10];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE anTxnAmt[8];
	BYTE ucReadPurseFlag;
	BYTE anRefundFee[5];
	BYTE anBrokenFee[5];
	BYTE anCustomerFee[5];
}TxnReqOnline_TM_In;

typedef struct __packed_back{
	BYTE anCardID[17];
	BYTE anDeviceID[10];
	BYTE anCPUDeviceID[16];
	BYTE anEVBeforeTxn[8];
	BYTE anTxnSN[8];
	BYTE anTxnAmt[8];
	BYTE ucAutoLoad;
	BYTE anAutoLoadAmt[8];
}TxnReqOnline_TM_Out_1;

typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];			//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucBLVersion[2];			//Field 6	//2
	BYTE ucMsgType;				//Field 7	//1
	BYTE ucSubType;				//Field 8	//1
	BYTE ucDeviceID[4];			//Field 9	//4
	BYTE ucSPID;				//Field 10	//1
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucCardType;			//Field 12	//1
	BYTE ucPersonalProfile;			//Field 13	//1
	BYTE ucIssuerCode;			//Field 15	//1
	BYTE ucLocationID;			//Field 19	//1
	BYTE ucBankCode;			//Field 20	//1
	BYTE ucLoyaltyCounter[2];		//Field 21	//2
	BYTE ucExpiryDate[4];			//Field 24	//4
	BYTE ucAreaCode;			//Field 26	//1
	BYTE ucTMID[2];				//Field 28	//2
	BYTE ucTMTxnDateTime[14];		//Field 29	//14
	BYTE ucTMSerialNumber[6];		//Field 30	//6
	BYTE ucTMAgentNumber[4];		//Field 31	//4
	BYTE ucSTAC[8];				//Field 33	//8
	BYTE ucKeyVersion;			//Field 35	//1
	BYTE ucSAMID[8];			//Field 36	//8
	BYTE ucSAMSN[4];			//Field 37	//4
	BYTE ucSAMCRN[8];			//Field 38	//8
	BYTE ucReaderFWVersion[6];		//Field 39	//6
	BYTE ucCardID[7];			//Field 58	//7
	BYTE ucTxnAmt[3];			//Field 59	//3
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucCPUSPID[3];			//Field 63	//3
	BYTE ucCPULocationID[2];		//Field 64	//2
	BYTE ucPID[8];				//Field 65	//8
	BYTE ucCTC[3];				//Field 66	//3
	BYTE ucProfileExpiryDate[4];		//Field 67	//4
	BYTE ucSubAreaCode[2];			//Field 69	//2
	BYTE ucTxnSN[3];			//Field 70	//3
	BYTE ucEV[3];				//Field 71	//3
	BYTE ucDeposit[3];			//Field 72	//3
	BYTE ucCPUAdminKeyKVN;			//Field 74-1	//1
	BYTE ucCreditKeyKVN;			//Field 74-2	//1
	BYTE ucCPUIssuerKeyKVN;			//Field 74-3	//1
	BYTE ucTxnMode;				//Field 75	//1
	BYTE ucTxnQuqlifier;			//Field 76	//1
	BYTE ucSignatureKeyKVN;			//Field 77	//1
	BYTE ucCPUSAMID[8];			//Field 79	//8
	BYTE ucHostAdminKVN;			//Field 87	//1
	BYTE ucPurseVersionNumber;		//Field 89	//1
	BYTE ucTMLocationID[10];		//Field 90	//10
	BYTE ucCACrypto[16];			//Field 101	//16
	CardAVR_t stCardAVRInfo_t;		//Field 104	//45
	ReaderAVRTM_t stReaderAVRInfo_t;	//Field 105	//83
	BYTE ucCardIDLen;			//Field 106	//1
}TxnReqOnline_TM_Out_2;//For 加值

typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];				//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucBLVersion[2];			//Field 6	//2
	BYTE ucMsgType; 				//Field 7	//1
	BYTE ucSubType; 				//Field 8	//1
	BYTE ucDeviceID[4]; 			//Field 9	//4
	BYTE ucSPID;					//Field 10	//1
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucCardType;				//Field 12	//1
	BYTE ucPersonalProfile; 		//Field 13	//1
	BYTE ucIssuerCode;				//Field 15	//1
	BYTE ucLocationID;				//Field 19	//1
	BYTE ucBankCode;				//Field 20	//1
	BYTE ucLoyaltyCounter[2];		//Field 21	//2
	BYTE ucExpiryDate[4];			//Field 24	//4
	BYTE ucAreaCode;				//Field 26	//1
	BYTE ucTMID[2]; 				//Field 28	//2
	BYTE ucTMTxnDateTime[14];		//Field 29	//14
	BYTE ucTMSerialNumber[6];		//Field 30	//6
	BYTE ucTMAgentNumber[4];		//Field 31	//4
	BYTE ucSTAC[8]; 				//Field 33	//8
	BYTE ucKeyVersion;				//Field 35	//1
	BYTE ucSAMID[8];				//Field 36	//8
	BYTE ucSAMSN[4];				//Field 37	//4
	BYTE ucSAMCRN[8];				//Field 38	//8
	BYTE ucReaderFWVersion[6];		//Field 39	//6
	BYTE ucCardRefundFee[2];		//Field ??	//2
	BYTE ucCardID[7];				//Field 58	//7
	BYTE ucTxnAmt[3];				//Field 59	//3
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucCPUSPID[3];				//Field 63	//3
	BYTE ucCPULocationID[2];		//Field 64	//2
	BYTE ucPID[8];					//Field 65	//8
	BYTE ucCTC[3];					//Field 66	//3
	BYTE ucProfileExpiryDate[4];	//Field 67	//4
	BYTE ucSubAreaCode[2];			//Field 69	//2
	BYTE ucTxnSN[3];				//Field 70	//3
	BYTE ucEV[3];					//Field 71	//3
	BYTE ucDeposit[3];				//Field 72	//3
	BYTE ucCPUBrokenFee[2];			//Field ??	//2
	BYTE ucCPUAdminKeyKVN;			//Field 74-1//1
	BYTE ucCreditKeyKVN;			//Field 74-2//1
	BYTE ucCPUIssuerKeyKVN; 		//Field 74-3//1
	BYTE ucTxnMode; 				//Field 75	//1
	BYTE ucTxnQuqlifier;			//Field 76	//1
	BYTE ucSignatureKeyKVN; 		//Field 77	//1
	BYTE ucCPUSAMID[8]; 			//Field 79	//8
	BYTE ucHostAdminKVN;			//Field 87	//1
	BYTE ucPurseVersionNumber;		//Field 89	//1
	BYTE ucTMLocationID[10];		//Field 90	//10
	BYTE ucCACrypto[16];			//Field 101 //16
	CardAVR_t stCardAVRInfo_t;		//Field 104 //45
	ReaderAVRTM_t stReaderAVRInfo_t;//Field 105 //83
	BYTE ucCardIDLen;				//Field 106 //1
	BYTE ucCardCustomerFee[2]; 		//Field ??	//2
}TxnReqOnline_TM_Out_3;//For 退卡

typedef struct{
    BYTE ucMessageType[4];
    BYTE ucDataLen[3];                //Field 1        //3
    BYTE ucProcessignCode[6];        //Field 2        //6
    BYTE ucBLVersion[2];            //Field 6        //2
    BYTE ucMsgType;                 //Field 7        //1
    BYTE ucSubType;                 //Field 8        //1
    BYTE ucDeviceID[4];             //Field 9        //4
    BYTE ucSPID;                    //Field 10        //1
    BYTE ucTxnDateTime[4];            //Field 11        //4
    BYTE ucCardType;                //Field 12        //1
    BYTE ucPersonalProfile;         //Field 13        //1
    BYTE ucIssuerCode;                //Field 15        //1
    BYTE ucLocationID;                //Field 19        //1
    BYTE ucBankCode;                //Field 20        //1
    BYTE ucLoyaltyCounter[2];        //Field 21        //2
    BYTE ucExpiryDate[4];            //Field 24        //4
    BYTE ucAreaCode;                //Field 26        //1
    BYTE ucTMID[2];                 //Field 28        //2
    BYTE ucTMTxnDateTime[14];        //Field 29        //14
    BYTE ucTMSerialNumber[6];        //Field 30        //6
    BYTE ucTMAgentNumber[4];        //Field 31        //4
    BYTE ucSTAC[8];                 //Field 33        //8
    BYTE ucKeyVersion;                //Field 35        //1
    BYTE ucSAMID[8];                //Field 36        //8
    BYTE ucSAMSN[4];                //Field 37        //4
    BYTE ucSAMCRN[8];                //Field 38        //8
    BYTE ucReaderFWVersion[6];        //Field 39        //6
    BYTE ucCardID[7];                //Field 58        //7
    BYTE ucTxnAmt[3];                //Field 59        //3
    BYTE ucCPUDeviceID[6];            //Field 62        //6
    BYTE ucCPUSPID[3];                //Field 63        //3
    BYTE ucCPULocationID[2];        //Field 64        //2
    BYTE ucPID[8];                    //Field 65        //8
    BYTE ucCTC[3];                    //Field 66        //3
    BYTE ucProfileExpiryDate[4];    //Field 67        //4
    BYTE ucSubAreaCode[2];            //Field 69        //2
    BYTE ucTxnSN[3];                //Field 70        //3
    BYTE ucEV[3];                    //Field 71        //3
    BYTE ucDeposit[3];                //Field 72        //3
    BYTE ucCPUAdminKeyKVN;            //Field 74-1    //1
    BYTE ucCreditKeyKVN;            //Field 74-2    //1
    BYTE ucCPUIssuerKeyKVN;         //Field 74-3    //1
    BYTE ucTxnMode;                 //Field 75        //1
    BYTE ucTxnQuqlifier;            //Field 76        //1
    BYTE ucSignatureKeyKVN;         //Field 77        //1
    BYTE ucCPUSAMID[8];             //Field 79        //8
    BYTE ucHostAdminKVN;            //Field 87        //1
    BYTE ucPurseVersionNumber;        //Field 89        //1
    BYTE ucTMLocationID[10];        //Field 90        //10
    BYTE ucCACrypto[16];            //Field 101     //16
    CardAVR_t stCardAVRInfo_t;        //Field 104     //45
    ReaderAVRTM_t stReaderAVRInfo_t;//Field 105     //83
    BYTE ucCardIDLen;                //Field 106     //1
    BYTE ucTxnAmt1[2];                //Field 1XX-1     //24
    BYTE ucTxnAmt2[2];                //Field 1XX-2     //24
    BYTE ucTxnAmt3[2];                //Field 1XX-3     //24
    BYTE ucTxnAmt4[2];
    BYTE ucTxnAmt5[2];
    BYTE ucRFU[14];                    //Field 1XX     //24
}TxnReqOnline_TM_Out_4;//For 3合1退貨

typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];				//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucBLVersion[2];			//Field 6	//2
	BYTE ucMsgType;					//Field 7	//1
	BYTE ucSubType;					//Field 8	//1
	BYTE ucDeviceID[4];				//Field 9	//4
	BYTE ucSPID;					//Field 10	//1
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucCardType;				//Field 12	//1
	BYTE ucPersonalProfile;			//Field 13	//1
	BYTE ucIssuerCode;				//Field 15	//1
	BYTE ucLocationID;				//Field 19	//1
	BYTE ucBankCode;				//Field 20	//1
	BYTE ucLoyaltyCounter[2];		//Field 21	//2
	BYTE ucExpiryDate[4];			//Field 24	//4
	BYTE ucAreaCode;				//Field 26	//1
	BYTE ucTMID[2];					//Field 28	//2
	BYTE ucTMTxnDateTime[14];		//Field 29	//14
	BYTE ucTMSerialNumber[6];		//Field 30	//6
	BYTE ucTMAgentNumber[4];		//Field 31	//4
	BYTE ucSTAC[8];					//Field 33	//8
	BYTE ucKeyVersion;				//Field 35	//1
	BYTE ucSAMID[8];				//Field 36	//8
	BYTE ucSAMSN[4];				//Field 37	//4
	BYTE ucSAMCRN[8];				//Field 38	//8
	BYTE ucReaderFWVersion[6];	//Field 39	//6
	BYTE ucECCAmt[3];				//Field 53  //3
	BYTE ucCardID[7];				//Field 58	//7
	BYTE ucTxnAmt[3];				//Field 59	//3
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucCPUSPID[3];				//Field 63	//3
	BYTE ucCPULocationID[2];		//Field 64	//2
	BYTE ucPID[8];					//Field 65	//8
	BYTE ucCTC[3];					//Field 66	//3
	BYTE ucProfileExpiryDate[4];	//Field 67	//4
	BYTE ucSubAreaCode[2];			//Field 69	//2
	BYTE ucTxnSN[3];				//Field 70	//3
	BYTE ucEV[3];					//Field 71	//3
	BYTE ucDeposit[3];				//Field 72	//3
	BYTE ucCPUAdminKeyKVN;			//Field 74-1//1
	BYTE ucCreditKeyKVN;			//Field 74-2//1
	BYTE ucCPUIssuerKeyKVN;			//Field 74-3//1
	BYTE ucTxnMode;					//Field 75	//1
	BYTE ucTxnQuqlifier;			//Field 76	//1
	BYTE ucSignatureKeyKVN;			//Field 77	//1
	BYTE ucCPUSAMID[8];				//Field 79	//8
	BYTE ucHostAdminKVN;			//Field 87	//1
	BYTE ucPurseVersionNumber;		//Field 89	//1
	BYTE ucTMLocationID[10];		//Field 90	//10
	BYTE ucCACrypto[16];			//Field 101	//16
	CardAVR_t stCardAVRInfo_t;		//Field 104	//45
	ReaderAVRTM_t stReaderAVRInfo_t;//Field 105	//83
	BYTE ucCardIDLen;				//Field 106	//1

}TxnReqOnline_TM_Out_5;//For 新版自動加值


typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];			//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucBLVersion[2];			//Field 6	//2
	BYTE ucMsgType;				//Field 7	//1
	BYTE ucSubType;				//Field 8	//1
	BYTE ucDeviceID[4];			//Field 9	//4
	BYTE ucSPID;				//Field 10	//1
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucCardType;			//Field 12	//1
	BYTE ucPersonalProfile;			//Field 13	//1
	BYTE ucIssuerCode;			//Field 15	//1
	BYTE ucLocationID;			//Field 19	//1
	BYTE ucTMID[2];				//Field 28	//2
	BYTE ucTMTxnDateTime[14];		//Field 29	//14
	BYTE ucTMSerialNumber[6];		//Field 30	//6
	BYTE ucTMAgentNumber[4];		//Field 31	//4
	BYTE ucSTAC[8];				//Field 33	//8
	BYTE ucKeyVersion;			//Field 35	//1
	BYTE ucSAMID[8];			//Field 36	//8
	BYTE ucSAMSN[4];			//Field 37	//4
	BYTE ucSAMCRN[8];			//Field 38	//8
	BYTE ucReaderFWVersion[6];		//Field 39	//6
	BYTE ucCardID[7];			//Field 58	//7
	BYTE ucTxnAmt[3];			//Field 59	//3
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucCPUSPID[3];			//Field 63	//3
	BYTE ucCPULocationID[2];		//Field 64	//2
	BYTE ucPID[8];				//Field 65	//8
	BYTE ucCTC[3];				//Field 66	//3
	BYTE ucSubAreaCode[2];			//Field 69	//2
	BYTE ucTxnSN[3];			//Field 70	//3
	BYTE ucEV[3];				//Field 71	//3
	BYTE ucDeposit[3];			//Field 72	//3
	BYTE ucCPUAdminKeyKVN;			//Field 74-1	//1
	BYTE ucCreditKeyKVN;			//Field 74-2	//1
	BYTE ucCPUIssuerKeyKVN;			//Field 74-3	//1
	BYTE ucTxnMode;				//Field 75	//1
	BYTE ucTxnQuqlifier;			//Field 76	//1
	BYTE ucCPUSAMID[8];			//Field 79	//8
	BYTE ucHostAdminKVN;			//Field 87	//1
	BYTE ucPurseVersionNumber;		//Field 89	//1
	BYTE ucTMLocationID[10];		//Field 90	//10
	BYTE ucCACrypto[16];			//Field 101	//16
	ReaderAVRTM_t stReaderAVRInfo_t;	//Field 105	//83
	BYTE ucCardIDLen;			//Field 106	//1
}CardSale_TM_Out;//For 售卡

typedef struct __packed_back{
	BYTE ucDataLen[3];
	BYTE anCardID[17];
	BYTE anDeviceID[10];
	BYTE anCPUDeviceID[16];
	BYTE anEVBeforeTxn[8];
	BYTE anTxnSN[8];
	BYTE anTxnAmt[8];
	BYTE ucAutoLoad;
	BYTE anAutoLoadAmt[8];
}CancelTxn_TM_Out;//For 取消

/*****************************TxnReqOnline*****************************/

/*****************************AuthTxnOnline****************************/
typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];				//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucMsgType;					//Field 7	//1
	BYTE ucSubType;					//Field 8	//1
	BYTE ucDeviceID[4];				//Field 9	//4
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucExpiryDate[4];			//Field 25	//4
	BYTE ucTMID[2];					//Field 28	//2
	BYTE ucTMTxnDateTime[14];		//Field 29	//14
	BYTE ucTMSerialNumber[6];		//Field 30	//6
	BYTE ucTMAgentNumber[4];		//Field 31	//4
	BYTE ucHTAC[8];					//Field 34	//8
	BYTE ucResponseCode[2];			//Field 43	//2
	BYTE ucCardID[7];				//Field 58	//7
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucPID[8];					//Field 65	//8
	BYTE ucProfileExpiryDate[4];	//Field 68	//4
	BYTE ucPurseVersionNumber;		//Field 89	//1
	BYTE ucTMLocationID[10];		//Field 90	//10
	BYTE ucHATOKEN[16];				//Field 92	//16
	BYTE ucCardIDLen;				//Field 106	//1
	BYTE ucLCDControlFlag;
}AuthTxnOnline_TM_In;//For 加值

typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];			//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucMsgType;				//Field 7	//1
	BYTE ucSubType;				//Field 8	//1
	BYTE ucDeviceID[4];			//Field 9	//4
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucTMID[2];				//Field 28	//2
	BYTE ucTMTxnDateTime[14];		//Field 29	//14
	BYTE ucTMSerialNumber[6];		//Field 30	//6
	BYTE ucTMAgentNumber[4];		//Field 31	//4
	BYTE ucHTAC[8];				//Field 34	//8
	BYTE ucResponseCode[2];			//Field 43	//2
	BYTE ucCardID[7];			//Field 58	//7
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucPID[8];				//Field 65	//8
	BYTE ucPurseVersionNumber;		//Field 89	//1
	BYTE ucTMLocationID[10];		//Field 90	//10
	BYTE ucHATOKEN[16];			//Field 92	//16
	BYTE ucCardIDLen;			//Field 106	//1
	BYTE ucLCDControlFlag;
}AuthCardSale_TM_In;//For 售卡

typedef struct __packed_back{
	STMC_t stSTMC_t;
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];			//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucMsgType;				//Field 7	//1
	BYTE ucSubType;				//Field 8	//1
	BYTE ucDeviceID[4];			//Field 9	//4
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucMAC[10];				//Field 23	//10
	BYTE ucConfirmCode[2];			//Field 47	//2
	BYTE ucCTAC[8];				//Field 48	//8
	BYTE ucCardID[7];			//Field 58	//7
	BYTE ucEV[3];				//Field 60	//3
	BYTE ucTxnSN[3];			//Field 61	//3
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucPID[8];				//Field 65	//8
	BYTE ucSignature[16];			//Field 78	//16
	BYTE ucHashType;			//Field 86	//1
	BYTE ucHostAdminKVN;			//Field 87	//1
	BYTE ucCPUMAC[16];			//Field 88	//16
}AuthTxnOnline_TM_Out;

typedef struct __packed_back{
	STMC_t stSTMC_t;
	BYTE unSTMCAmt1[4];
	BYTE unSTMCAmt2[4];
	BYTE unSTMCAmt3[4];
	BYTE unSTMCAmt4[4];
	BYTE unSTMCAmt5[4];
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];			//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucMsgType;				//Field 7	//1
	BYTE ucSubType;				//Field 8	//1
	BYTE ucDeviceID[4];			//Field 9	//4
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucMAC[10];				//Field 23	//10
	BYTE ucConfirmCode[2];			//Field 47	//2
	BYTE ucCTAC[8];				//Field 48	//8
	BYTE ucCardID[7];			//Field 58	//7
	BYTE ucEV[3];				//Field 60	//3
	BYTE ucTxnSN[3];			//Field 61	//3
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucPID[8];				//Field 65	//8
	BYTE ucSignature[16];			//Field 78	//16
	BYTE ucHashType;			//Field 86	//1
	BYTE ucHostAdminKVN;			//Field 87	//1
	BYTE ucCPUMAC[16];			//Field 88	//16
}AuthTxnOnline1_TM_Out;

typedef struct __packed_back{
	STMC_t stSTMC_t;
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];			//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucMsgType;				//Field 7	//1
	BYTE ucSubType;				//Field 8	//1
	BYTE ucDeviceID[4];			//Field 9	//4
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucMAC[10];				//Field 23	//10
	BYTE ucConfirmCode[2];			//Field 47	//2
	BYTE ucCTAC[8];				//Field 48	//8
	BYTE ucCardID[7];			//Field 58	//7
	BYTE ucEV[3];				//Field 60	//3
	BYTE ucTxnSN[3];			//Field 61	//3
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucPID[8];				//Field 65	//8
	BYTE ucHashType;			//Field 86	//1
	BYTE ucHostAdminKVN;			//Field 87	//1
	BYTE ucCPUMAC[16];			//Field 88	//16
}AuthCardSale_TM_Out;
/*****************************AuthTxnOnline****************************/

/*****************************TxnReqOffline****************************/
typedef struct __packed_back{
	BYTE anMsgType[2];
	BYTE anSubType[2];
	BYTE ucTMLocationID[10];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE anTxnAmt[8];
	BYTE ucReadPurseFlag;
}TxnReqOffline_TM_In;

typedef struct __packed_back{
	BYTE anCardID[17];
	BYTE anDeviceID[10];
	BYTE anCPUDeviceID[16];
	BYTE anEVBeforeTxn[8];
	BYTE anTxnSN[8];
	BYTE anTxnAmt[8];
	BYTE ucAutoLoad;
	BYTE anAutoLoadAmt[8];
}TxnReqOffline_TM_Out_1;

typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];			//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucBLVersion[2];			//Field 6	//2
	BYTE ucMsgType;				//Field 7	//1
	BYTE ucSubType;				//Field 8	//1
	BYTE ucDeviceID[4];			//Field 9	//4
	BYTE ucSPID;				//Field 10	//1
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucCardType;			//Field 12	//1
	BYTE ucPersonalProfile;			//Field 13	//1
	BYTE ucIssuerCode;			//Field 15	//1
	BYTE ucExpiryDate[4];			//Field 24	//4
	BYTE ucAreaCode;			//Field 26	//1
	BYTE ucTMID[2];				//Field 28	//2
	BYTE ucTMTxnDateTime[14];		//Field 29	//14
	BYTE ucTMSerialNumber[6];		//Field 30	//6
	BYTE ucTMAgentNumber[4];		//Field 31	//4
	BYTE ucSTAC[8];				//Field 33	//8
	BYTE ucKeyVersion;			//Field 35	//1
	BYTE ucSAMID[8];			//Field 36	//8
	BYTE ucSAMSN[4];			//Field 37	//4
	BYTE ucSAMCRN[8];			//Field 38	//8
	BYTE ucReaderFWVersion[6];		//Field 39	//6
	BYTE ucCardID[7];			//Field 58	//7
	BYTE ucTxnAmt[3];			//Field 59	//3
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucCPUSPID[3];			//Field 63	//3
	BYTE ucCPULocationID[2];		//Field 64	//2
	BYTE ucPID[8];				//Field 65	//8
	BYTE ucCTC[3];				//Field 66	//3
	BYTE ucProfileExpiryDate[4];		//Field 67	//4
	BYTE ucSubAreaCode[2];			//Field 69	//2
	BYTE ucTxnSN[3];			//Field 70	//3
	BYTE ucEV[3];				//Field 71	//3
	BYTE ucCPUAdminKeyKVN;			//Field 74-1	//1
	BYTE ucCreditKeyKVN;			//Field 74-2	//1
	BYTE ucCPUIssuerKeyKVN;			//Field 74-3	//1
	BYTE ucTxnMode;				//Field 75	//1
	BYTE ucTxnQuqlifier;			//Field 76	//1
	BYTE ucCPUSAMID[8];			//Field 79	//8
	BYTE ucHostAdminKVN;			//Field 87	//1
	BYTE ucPurseVersionNumber;		//Field 89	//1
	BYTE ucTMLocationID[10];		//Field 90	//10
	BYTE ucCACrypto[16];			//Field 101	//16
	BYTE ucCardOneDayQuota[3];		//Field 103-1	//3
	BYTE ucCardOneDayQuotaDate[2];		//Field 103-1	//2
	CardAVR_t stCardAVRInfo_t;		//Field 104	//45
	BYTE ucCardIDLen;			//Field 106	//1
}TxnReqOffline_TM_Out_2;

typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];			//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucBLVersion[2];			//Field 6	//2
	BYTE ucMsgType;				//Field 7	//1
	BYTE ucSubType;				//Field 8	//1
	BYTE ucDeviceID[4];			//Field 9	//4
	BYTE ucSPID;				//Field 10	//1
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucCardType;			//Field 12	//1
	BYTE ucPersonalProfile;			//Field 13	//1
	BYTE ucIssuerCode;			//Field 15	//1
	BYTE ucExpiryDate[4];			//Field 24	//4
	BYTE ucAreaCode;			//Field 26	//1
	BYTE ucTMID[2];				//Field 28	//2
	BYTE ucTMTxnDateTime[14];		//Field 29	//14
	BYTE ucTMSerialNumber[6];		//Field 30	//6
	BYTE ucTMAgentNumber[4];		//Field 31	//4
	BYTE ucSTAC[8];				//Field 33	//8
	BYTE ucKeyVersion;			//Field 35	//1
	BYTE ucSAMID[8];			//Field 36	//8
	BYTE ucSAMSN[4];			//Field 37	//4
	BYTE ucSAMCRN[8];			//Field 38	//8
	BYTE ucReaderFWVersion[6];		//Field 39	//6
	BYTE ucCardID[7];			//Field 58	//7
	BYTE ucTxnAmt[3];			//Field 59	//3
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucCPUSPID[3];			//Field 63	//3
	BYTE ucCPULocationID[2];		//Field 64	//2
	BYTE ucPID[8];				//Field 65	//8
	BYTE ucCTC[3];				//Field 66	//3
	BYTE ucProfileExpiryDate[4];		//Field 67	//4
	BYTE ucSubAreaCode[2];			//Field 69	//2
	BYTE ucTxnSN[3];			//Field 70	//3
	BYTE ucEV[3];				//Field 71	//3
	BYTE ucCPUAdminKeyKVN;			//Field 74-1	//1
	BYTE ucCreditKeyKVN;			//Field 74-2	//1
	BYTE ucCPUIssuerKeyKVN;			//Field 74-3	//1
	BYTE ucTxnMode;				//Field 75	//1
	BYTE ucTxnQuqlifier;			//Field 76	//1
	BYTE ucCPUSAMID[8];			//Field 79	//8
	BYTE ucHostAdminKVN;			//Field 87	//1
	BYTE ucPurseVersionNumber;		//Field 89	//1
	BYTE ucTMLocationID[10];		//Field 90	//10
	BYTE ucCACrypto[16];			//Field 101	//16
	CardAVR_t stCardAVRInfo_t;		//Field 104	//45
	BYTE ucCardIDLen;			//Field 106	//1
}TxnReqOffline_TM_Out_3;

/*****************************TxnReqOffline****************************/

/*****************************AuthTxnOffline***************************/
typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];			//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucMsgType;				//Field 7	//1
	BYTE ucSubType;				//Field 8	//1
	BYTE ucDeviceID[4];			//Field 9	//4
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucTMID[2];				//Field 28	//2
	BYTE ucTMTxnDateTime[14];		//Field 29	//14
	BYTE ucTMSerialNumber[6];		//Field 30	//6
	BYTE ucTMAgentNumber[4];		//Field 31	//4
	BYTE ucHTAC[8];				//Field 34	//8
	BYTE ucResponseCode[2];			//Field 43	//2
	BYTE ucCardID[7];			//Field 58	//7
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucPID[8];				//Field 65	//8
	BYTE ucPurseVersionNumber;		//Field 89	//1
	BYTE ucTMLocationID[10];		//Field 90	//10
	BYTE ucHVCrypto[16];			//Field 102	//16
	BYTE ucCardIDLen;			//Field 106	//1
	BYTE ucLCDControlFlag;
}AuthTxnOffline_TM_In;

typedef struct __packed_back{
	STMC_t stSTMC_t;
	BYTE unSTMCAmt1[4];
	BYTE unSTMCAmt2[4];
	BYTE unSTMCAmt3[4];
	BYTE unSTMCAmt4[4];
	BYTE unSTMCAmt5[4];
	//加值重送資料開始
	BYTE unVARMsgType[2];
	BYTE unVARPurseVersionNumber[2];
	BYTE unVARTxnSN[6];
	BYTE unVARTxnDateTime[8];
	BYTE unVARSubType[2];
	BYTE unVARTxnAmt[6];
	BYTE unVAREV[6];
	BYTE unVARCardID[14];
	BYTE unVARIssuerCode[2];
	BYTE unVARCPUSPID[6];
	BYTE unVARCPULocationID[4];
	BYTE unVARPID[16];
	BYTE unVARCPUDeviceID[12];
	BYTE unVARLoyaltyCounter[4];
	BYTE unVARPersonalProfile[2];
	BYTE unVARBankCode[2];
	BYTE unVARCardIDLen[2];
	BYTE unVAR2DeviceID[8];
	BYTE unVAR2SPID[2];
	BYTE unVAR2LocationID[2];
	BYTE unVAR2CPUDeviceID[12];
	BYTE unVAR2CPUSPID[6];
	BYTE unVAR2CPULocationID[4];
}AuthTxnOffline_TM_Out;

typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];			//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucMsgType;				//Field 7	//1
	BYTE ucSubType;				//Field 8	//1
	BYTE ucDeviceID[4];			//Field 9	//4
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucTMID[2];				//Field 28	//2
	BYTE ucTMTxnDateTime[14];		//Field 29	//14
	BYTE ucTMSerialNumber[6];		//Field 30	//6
	BYTE ucTMAgentNumber[4];		//Field 31	//4
	BYTE ucHTAC[8];				//Field 34	//8
	BYTE ucResponseCode[2];			//Field 43	//2
	BYTE ucCardID[7];			//Field 58	//7
	BYTE ucTXNAMT[3];                 //Fied 59//3
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucPID[8];				//Field 65	//8
	BYTE ucPurseVersionNumber;		//Field 89	//1
	BYTE ucTMLocationID[10];		//Field 90	//10
	BYTE ucHVCrypto[16];			//Field 102	//16
	BYTE ucCardIDLen;			//Field 106	//1
}AuthTxnOffline_TM_In2;
/*****************************AuthTxnOffline***************************/

/******************************LockCard 1******************************/
typedef struct __packed_back{
	BYTE ucTMTxnDateTime[14];
	BYTE ucCardID_4[4];
}LockCard_TM_In_1;

typedef struct __packed_back{
	BYTE unMsgType[2];
	BYTE unSubType[2];
	BYTE unDeviceID[8];
	BYTE unSPID[2];
	BYTE unTxnDateTime[8];
	BYTE unCardType[2];
	BYTE unPersonalProfile[2];
	BYTE unCardID_8[8];
	BYTE unIssuerCode[2];
	BYTE unLocationID[2];
	BYTE unLockReason[2];
}LockCard_TM_Out_1;
/******************************LockCard 1******************************/

/******************************LockCard 2******************************/
typedef struct __packed_back{
	BYTE ucCardID[7];
	BYTE ucTMLocationID[10];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
}LockCard_TM_In_2;

typedef struct __packed_back{
	BYTE unPurseVersionNumber[2];//0	//0		//1
	BYTE unMsgType[2];           //1    //1 	//3
	BYTE unSubType[2];           //2    //2 	//5
	BYTE unDeviceID[8];          //3    //3 	//7
	BYTE unSPID[2];              //7    //7 	//15
	BYTE unTxnDateTime[8];       //8    //8 	//17
	BYTE unCardID[14];           //12   //12	//25
	BYTE unIssuerCode[2];        //19   //19	//39
	BYTE unLocationID[2];        //20   //20	//41
	BYTE unCardIDLen[2];         //21   //21	//43
	BYTE unCPUDeviceID[12];      //22   //22	//45
	BYTE unCPUSPID[6];           //28   //28	//57
	BYTE unCPULocationID[4];     //31   //31	//63
	BYTE unCardType[2];          //33   //33	//67
	BYTE unPersonalProfile[2];   //34   //34	//69
	BYTE unPID[16];              //35   //35	//71
	BYTE unCTC[6];               //43   //43	//87
	BYTE unLockReason[2];        //46   //46	//93
	BYTE unBLCFileName[20];      //47   //47	//95
	BYTE unBLCIDFlag;            //57   //67	//115
	BYTE unTMLocationID[10];     //58   //68	//116
	BYTE unTMID[2];              //63   //78	//126
	BYTE unRFU[8];               //64   //80	//128
	BYTE unEV[6];                //68   //88	//136
	BYTE unCPUEV[6];             //71   //91	//142
	BYTE unMSettingPar[2];       //74   //94	//148
	BYTE unCSettingPar[2];       //75   //95	//150
	BYTE unTxnSN[6];             //76   //96	//152
	BYTE unVARTxnSN[6];          //79   //99	//158
	BYTE unVARTxnDateTime[8];    //82   //102	//164
	BYTE unVARSubType[2];        //86   //106	//172
	BYTE unVARTxnAmt[6];         //87   //107	//174
	BYTE unVAREV[6];             //90   //110	//180
	BYTE unVARDeviceID[12];      //93   //113	//186
}LockCard_TM_Out_2;				 //99   //119	//198
/******************************LockCard 2******************************/

/***************************ReadDongleDeduct***************************/
typedef struct __packed_back{
	BYTE anLogSeqNo[5];
}ReadDongleDeduct_TM_In;

typedef struct __packed_back{
	STMC_t stSTMC_t;
    BYTE unSTMCAmt1[4];
    BYTE unSTMCAmt2[4];
    BYTE unSTMCAmt3[4];
	BYTE unSTMCAmt4[4];
	BYTE unSTMCAmt5[4];
	//加值重送資料開始
	BYTE unVARMsgType[2];
	BYTE unVARPurseVersionNumber[2];
	BYTE unVARTxnSN[6];
	BYTE unVARTxnDateTime[8];
	BYTE unVARSubType[2];
	BYTE unVARTxnAmt[6];
	BYTE unVAREV[6];
	BYTE unVARCardID[14];
	BYTE unVARIssuerCode[2];
	BYTE unVARCPUSPID[6];
	BYTE unVARCPULocationID[4];
	BYTE unVARPID[16];
	BYTE unVARCPUDeviceID[12];
	BYTE unVARLoyaltyCounter[4];
	BYTE unVARPersonalProfile[2];
	BYTE unVARBankCode[2];
	BYTE unVARCardIDLen[2];
	BYTE unVAR2DeviceID[8];
	BYTE unVAR2SPID[2];
	BYTE unVAR2LocationID[2];
	BYTE unVAR2CPUDeviceID[12];
	BYTE unVAR2CPUSPID[6];
	BYTE unVAR2CPULocationID[4];
}ReadDongleDeduct_TM_Out;
/***************************ReadDongleDeduct***************************/

/********************************SetValue******************************/
typedef struct __packed_back{
	BYTE anMsgType[2];
	BYTE ucTMLocationID_6[6];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
}SetValue_TM_In;

typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];			//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucCommandCode[2];			//Field 3	//2
	BYTE ucBLVersion[2];			//Field 6	//2
	BYTE ucMsgType;				//Field 7	//1
	BYTE ucSubType;				//Field 8	//1
	BYTE ucDeviceID[4];			//Field 9	//4
	BYTE ucSPID;				//Field 10	//1
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucCardType;			//Field 12	//1
	BYTE ucPersonalProfile;			//Field 13	//1
	BYTE ucCardID_4[4];			//Field 14	//4
	BYTE ucIssuerCode;			//Field 15	//1
	BYTE ucTxnAmt_2[2];			//Field 17	//2
	BYTE ucExpiryDate[4];			//Field 24	//4
	BYTE ucAreaCode;			//Field 26	//1
	BYTE ucTMLocationID_6[6];		//Field 27	//6
	BYTE ucTMID[2];				//Field 28	//2
	BYTE ucTMTxnDateTime[14];		//Field 29	//14
	BYTE ucTMSerialNumber[6];		//Field 30	//6
	BYTE ucTMAgentNumber[4];		//Field 31	//4
	BYTE ucEV_2[2];				//Field 32	//2
	BYTE ucSTAC[8];				//Field 33	//8
	BYTE ucKeyVersion;			//Field 35	//1
	BYTE ucSAMID[8];			//Field 36	//8
	BYTE ucSAMSN[4];			//Field 37	//4
	BYTE ucSAMCRN[8];			//Field 38	//8
	BYTE ucReaderFWVersion[6];		//Field 39	//6
}SetValue_TM_Out;
/********************************SetValue******************************/

/******************************AuthSetValue****************************/
typedef struct __packed_back{
	BYTE ucExpiryDate[4];			//Field 25	//4
	BYTE ucHTAC[8];				//Field 34	//8
}AuthSetValue_TM_In;

typedef struct __packed_back{
	BYTE anCardID_10[10];
	BYTE anDeviceID[10];
	BYTE anEVBeforeTxn_5[5];
	BYTE anOrgExpiryDate[8];
	BYTE anNewExpiryDate[8];
	BYTE anProcessignCode[6];
	BYTE unMsgType[2];
	BYTE unSubType[2];
	BYTE unDeviceID[8];
	BYTE unTxnDateTime[8];
	BYTE unCardType[2];
	BYTE unPersonalProfile[2];
	BYTE unCardID_8[8];
	BYTE unNewExpiryDate[8];
}AuthSetValue_TM_Out;
/******************************AuthSetValue****************************/

/********************************TxRefund******************************/
typedef struct __packed_back{
	BYTE anMsgType[2];
	BYTE anSubType[2];
	BYTE ucTMLocationID_6[6];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE anTxnAmt_5[5];
	BYTE ucTMTxnDateTime2[14];
	BYTE ucTMSerialNumber2[6];
}TxRefund_TM_In;

typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];			//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucCommandCode[2];			//Field 3	//2
	BYTE ucBLVersion[2];			//Field 6	//2
	BYTE ucMsgType;				//Field 7	//1
	BYTE ucSubType;				//Field 8	//1
	BYTE ucDeviceID[4];			//Field 9	//4
	BYTE ucSPID;				//Field 10	//1
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucCardType;			//Field 12	//1
	BYTE ucPersonalProfile;			//Field 13	//1
	BYTE ucCardID_4[4];			//Field 14	//4
	BYTE ucIssuerCode;			//Field 15	//1
	BYTE ucTxnAmt_2[2];			//Field 17	//2
	BYTE ucLocationID;			//Field 19	//1
	BYTE ucBankCode;			//Field 20	//1
	BYTE ucLoyaltyCounter[2];		//Field 21	//2
	BYTE ucDeposit[2];			//Field 22	//2
	BYTE ucExpiryDate[4];			//Field 24	//4
	BYTE ucAreaCode;			//Field 26	//1
	BYTE ucTMLocationID_6[6];		//Field 27	//6
	BYTE ucTMID[2];				//Field 28	//2
	BYTE ucTMTxnDateTime[14];		//Field 29	//14
	BYTE ucTMSerialNumber[6];		//Field 30	//6
	BYTE ucTMAgentNumber[4];		//Field 31	//4
	BYTE ucEV_2[2];				//Field 32	//2
	BYTE ucSTAC[8];				//Field 33	//8
	BYTE ucKeyVersion;			//Field 35	//1
	BYTE ucSAMID[8];			//Field 36	//8
	BYTE ucSAMSN[4];			//Field 37	//4
	BYTE ucSAMCRN[8];			//Field 38	//8
	BYTE ucReaderFWVersion[6];		//Field 39	//6
	BYTE ucCAVRMsgType;			//Field 40-1	//1
	BYTE ucCAVRTxnSNLSB;                    //Field 40-2    //1
	BYTE ucCAVRTxnDateTime[4];              //Field 40-3    //4
	BYTE ucCAVRSubType;                     //Field 40-4    //1
	BYTE ucCAVRTxnAmt[2];                   //Field 40-5    //2
	BYTE ucCAVREV[2];                       //Field 40-6    //2
	BYTE ucCAVRCardID[4];                   //Field 40-7    //4
	BYTE ucCAVRIssuerCode;                  //Field 40-8    //1
	BYTE ucCAVRSPID;                        //Field 40-9    //1
	BYTE ucCAVRLocationID;                  //Field 40-10   //1
	BYTE ucCAVRDeviceID[4];                 //Field 40-11   //4
	BYTE ucCAVRBankCode;                    //Field 40-12   //1
	BYTE ucCAVRLoyaltyCounter[2];           //Field 40-13   //2
	ReaderAVR_t stReaderAVR_t;		//Field 41	//38
}TxRefund_TM_Out;
/********************************TxRefund******************************/

/******************************AuthTxRefund****************************/
typedef struct __packed_back{
	BYTE ucRFU4[4];
	BYTE ucHTAC[8];
	BYTE ucRFU2[2];
	BYTE ucLCDControlFlag;
}AuthTxRefund_TM_In;

typedef struct __packed_back{
	//餘轉退卡交易
	SETM_t stTxRefundSETM_t;
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];			//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucCommandCode[2];			//Field 3	//2
	BYTE ucMsgType;				//Field 7	//1
	BYTE ucSubType;				//Field 8	//1
	BYTE ucDeviceID[4];			//Field 9	//4
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucCardID_4[4];			//Field 14	//4
	BYTE ucTxnSN_2[2];			//Field 16	//2
	BYTE ucEV_2[2];				//Field 18	//2
	BYTE ucMAC[10];				//Field 23	//10
	BYTE ucConfirmCode[2];			//Field 47	//2
	BYTE ucCTAC[8];				//Field 48	//8
	//餘轉扣款交易
	SETM_t stTxDeductSETM_t;
	BYTE ucTMLocationID_6[6];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	//加值重送資料開始
	BYTE unVARSPID[2];
	BYTE unVARLocationID[2];
	BYTE unVARMsgType[2];
	BYTE unVARTxnSNLSB[2];
	BYTE unVARTxnDateTime[8];
	BYTE unVARSubType[2];
	BYTE unVARTxnAmt_4[4];
	BYTE unVAREV_4[4];
	BYTE unVARCardID_8[8];
	BYTE unVARIssuerCode[2];
	BYTE unVAR2SPID[2];
	BYTE unVAR2LocationID[2];
	BYTE unVAR2DeviceID[8];
	BYTE unVARBankCode[2];
	BYTE unVARLoyaltyCounter[4];
}AuthTxRefund_TM_Out;
/******************************AuthTxRefund****************************/

/********************************AddValue******************************/
typedef struct __packed_back{
	BYTE anMsgType[2];
	BYTE anSubType[2];
	BYTE ucTMLocationID_6[6];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE anTxnAmt_5[5];
}AddValue_TM_In;

typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];			//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucCommandCode[2];			//Field 3	//2
	BYTE ucBLVersion[2];			//Field 6	//2
	BYTE ucMsgType;				//Field 7	//1
	BYTE ucSubType;				//Field 8	//1
	BYTE ucDeviceID[4];			//Field 9	//4
	BYTE ucSPID;				//Field 10	//1
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucCardType;			//Field 12	//1
	BYTE ucPersonalProfile;			//Field 13	//1
	BYTE ucCardID_4[4];			//Field 14	//4
	BYTE ucIssuerCode;			//Field 15	//1
	BYTE ucTxnAmt_2[2];			//Field 17	//2
	BYTE ucLocationID;			//Field 19	//1
	BYTE ucBankCode;			//Field 20	//1
	BYTE ucLoyaltyCounter[2];		//Field 21	//2
	BYTE ucExpiryDate[4];			//Field 24	//4
	BYTE ucAreaCode;			//Field 26	//1
	BYTE ucTMLocationID_6[6];		//Field 27	//6
	BYTE ucTMID[2];				//Field 28	//2
	BYTE ucTMTxnDateTime[14];		//Field 29	//14
	BYTE ucTMSerialNumber[6];		//Field 30	//6
	BYTE ucTMAgentNumber[4];		//Field 31	//4
	BYTE ucEV_2[2];				//Field 32	//2
	BYTE ucSTAC[8];				//Field 33	//8
	BYTE ucKeyVersion;			//Field 35	//1
	BYTE ucSAMID[8];			//Field 36	//8
	BYTE ucSAMSN[4];			//Field 37	//4
	BYTE ucSAMCRN[8];			//Field 38	//8
	BYTE ucReaderFWVersion[6];		//Field 39	//6
	BYTE ucCAVRMsgType;			//Field 40-1	//1
	BYTE ucCAVRTxnSNLSB;                    //Field 40-2    //1
	BYTE ucCAVRTxnDateTime[4];              //Field 40-3    //4
	BYTE ucCAVRSubType;                     //Field 40-4    //1
	BYTE ucCAVRTxnAmt[2];                   //Field 40-5    //2
	BYTE ucCAVREV[2];                       //Field 40-6    //2
	BYTE ucCAVRCardID[4];                   //Field 40-7    //4
	BYTE ucCAVRIssuerCode;                  //Field 40-8    //1
	BYTE ucCAVRSPID;                        //Field 40-9    //1
	BYTE ucCAVRLocationID;                  //Field 40-10   //1
	BYTE ucCAVRDeviceID[4];                 //Field 40-11   //4
	BYTE ucCAVRBankCode;                    //Field 40-12   //1
	BYTE ucCAVRLoyaltyCounter[2];           //Field 40-13   //2
	ReaderAVR_t stReaderAVR_t;		//Field 41	//38
}AddValue_TM_Out;
/********************************AddValue******************************/

/******************************AuthAddValue****************************/
typedef struct __packed_back{
	BYTE ucExpiryDate[4];
	BYTE ucHTAC[8];
}AuthAddValue_TM_In;

typedef struct __packed_back{
	BYTE ucExpiryDate[4];
	BYTE ucHTAC[8];
	BYTE ucProfileExpiryDateDOS[2];
	BYTE ucLCDControlFlag;
}TxAuthAddValue_TM_In;

typedef struct __packed_back{
	//餘轉加值交易
	SETM_t stAddValueSETM_t;
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];			//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucCommandCode[2];			//Field 3	//2
	BYTE ucMsgType;				//Field 7	//1
	BYTE ucSubType;				//Field 8	//1
	BYTE ucDeviceID[4];			//Field 9	//4
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucCardID_4[4];			//Field 14	//4
	BYTE ucTxnSN_2[2];			//Field 16	//2
	BYTE ucEV_2[2];				//Field 18	//2
	BYTE ucMAC[10];				//Field 23	//10
	BYTE ucConfirmCode[2];			//Field 47	//2
	BYTE ucCTAC[8];				//Field 48	//8
}AuthAddValue_TM_Out;
/******************************AuthAddValue****************************/

/*****************************AutoloadEnable***************************/
typedef struct __packed_back{
	BYTE anMsgType[2];
	BYTE anSubType[2];
	BYTE ucTMLocationID_6[6];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
}AutoloadEnable_TM_In;

typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];			//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucCommandCode[2];			//Field 3	//2
	BYTE ucBLVersion[2];			//Field 6	//2
	BYTE ucMsgType;				//Field 7	//1
	BYTE ucSubType;				//Field 8	//1
	BYTE ucDeviceID[4];			//Field 9	//4
	BYTE ucSPID;				//Field 10	//1
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucCardType;			//Field 12	//1
	BYTE ucPersonalProfile;			//Field 13	//1
	BYTE ucCardID_4[4];			//Field 14	//4
	BYTE ucIssuerCode;			//Field 15	//1
	BYTE ucTxnAmt_2[2];			//Field 17	//2
	BYTE ucLocationID;			//Field 19	//1
	BYTE ucBankCode;			//Field 20	//1
	BYTE ucLoyaltyCounter[2];		//Field 21	//2
	BYTE ucExpiryDate[4];			//Field 24	//4
	BYTE ucAreaCode;			//Field 26	//1
	BYTE ucTMLocationID_6[6];		//Field 27	//6
	BYTE ucTMID[2];				//Field 28	//2
	BYTE ucTMTxnDateTime[14];		//Field 29	//14
	BYTE ucTMSerialNumber[6];		//Field 30	//6
	BYTE ucTMAgentNumber[4];		//Field 31	//4
	BYTE ucEV_2[2];				//Field 32	//2
	BYTE ucSTAC[8];				//Field 33	//8
	BYTE ucKeyVersion;			//Field 35	//1
	BYTE ucSAMID[8];			//Field 36	//8
	BYTE ucSAMSN[4];			//Field 37	//4
	BYTE ucSAMCRN[8];			//Field 38	//8
	BYTE ucReaderFWVersion[6];		//Field 39	//6
	BYTE ucCAVRMsgType;			//Field 40-1	//1
	BYTE ucCAVRTxnSNLSB;                    //Field 40-2    //1
	BYTE ucCAVRTxnDateTime[4];              //Field 40-3    //4
	BYTE ucCAVRSubType;                     //Field 40-4    //1
	BYTE ucCAVRTxnAmt[2];                   //Field 40-5    //2
	BYTE ucCAVREV[2];                       //Field 40-6    //2
	BYTE ucCAVRCardID[4];                   //Field 40-7    //4
	BYTE ucCAVRIssuerCode;                  //Field 40-8    //1
	BYTE ucCAVRSPID;                        //Field 40-9    //1
	BYTE ucCAVRLocationID;                  //Field 40-10   //1
	BYTE ucCAVRDeviceID[4];                 //Field 40-11   //4
	BYTE ucCAVRBankCode;                    //Field 40-12   //1
	BYTE ucCAVRLoyaltyCounter[2];           //Field 40-13   //2
	ReaderAVR_t stReaderAVR_t;		//Field 41	//38
	BYTE ucAutoLoadFlag;			//Field 53-1	//1
	BYTE ucAutoLoadAmt_2[2];		//Field 53-2	//2
}AutoloadEnable_TM_Out;
/*****************************AutoloadEnable***************************/

/***************************AuthAutoloadEnable*************************/
typedef struct __packed_back{
	BYTE ucHTAC[8];
	BYTE ucLCDControlFlag;
}AuthAutoloadEnable_TM_In;

typedef struct __packed_back{
	//餘轉加值交易
	SETM_t stAddValueSETM_t;
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];			//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucCommandCode[2];			//Field 3	//2
	BYTE ucMsgType;				//Field 7	//1
	BYTE ucSubType;				//Field 8	//1
	BYTE ucDeviceID[4];			//Field 9	//4
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucCardID_4[4];			//Field 14	//4
	BYTE ucTxnSN_2[2];			//Field 16	//2
	BYTE ucEV_2[2];				//Field 18	//2
	BYTE ucMAC[10];				//Field 23	//10
	BYTE ucConfirmCode[2];			//Field 47	//2
	BYTE ucCTAC[8];				//Field 48	//8
	BYTE ucAutoLoadFlag;			//Field 53-1	//1
	BYTE ucAutoLoadAmt_2[2];		//Field 53-2	//2
}AuthAutoloadEnable_TM_Out;
/***************************AuthAutoloadEnable*************************/

/****************************ReadCardDeduct****************************/
typedef struct __packed_back{
	BYTE ucLogIndex;
}ReadCardDeduct_TM_In;

typedef struct __packed_back{
	LastTxnRecoredL2_t stCardDeductRecored1_t;//74
	LastTxnRecoredL2_t stCardDeductRecored2_t;//74
	LastTxnRecoredL2_t stCardDeductRecored3_t;//74
	LastTxnRecoredL2_t stCardDeductRecored4_t;//74
	LastTxnRecoredL2_t stCardDeductRecored5_t;//74
	LastTxnRecoredL2_t stCardDeductRecored6_t;//74
}ReadCardDeduct_TM_Out;
/****************************ReadCardDeduct****************************/

/****************************ReadCodeVersion***************************/
typedef struct __packed_back{
	BYTE ucSAMAppletVersion;
	BYTE ucSAMType;
	BYTE ucSAMVersion;
	BYTE ucReaderFWVersion[6];
	BYTE ucHostSpecVersionNo;
}ReadCodeVersion_TM_Out;
/****************************ReadCodeVersion***************************/

/*******************************SetValue2******************************/
typedef struct __packed_back{
	BYTE anMsgType[2];
	BYTE anSubType[2];
	BYTE ucTMLocationID[10];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE ucReadPurseFlag;
}SetValue2_TM_In;

typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];				//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucBLVersion[2];			//Field 6	//2
	BYTE ucMsgType; 				//Field 7	//1
	BYTE ucSubType; 				//Field 8	//1
	BYTE ucDeviceID[4]; 			//Field 9	//4
	BYTE ucSPID;					//Field 10	//1
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucCardType;				//Field 12	//1
	BYTE ucPersonalProfile; 		//Field 13	//1
	BYTE ucIssuerCode;				//Field 15	//1
	BYTE ucExpiryDate[4];			//Field 24	//4
	BYTE ucAreaCode;				//Field 26	//1
	BYTE ucTMID[2]; 				//Field 28	//2
	BYTE ucTMTxnDateTime[14];		//Field 29	//14
	BYTE ucTMSerialNumber[6];		//Field 30	//6
	BYTE ucTMAgentNumber[4];		//Field 31	//4
	BYTE ucSTAC[8]; 				//Field 33	//8
	BYTE ucKeyVersion;				//Field 35	//1
	BYTE ucSAMID[8];				//Field 36	//8
	BYTE ucSAMSN[4];				//Field 37	//4
	BYTE ucSAMCRN[8];				//Field 38	//8
	BYTE ucReaderFWVersion[6];		//Field 39	//6
	BYTE ucCardID[7];				//Field 58	//7
	BYTE ucTxnAmt[3];				//Field 59	//3
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucCPUSPID[3];				//Field 63	//3
	BYTE ucCPULocationID[2];		//Field 64	//2
	BYTE ucPID[8];					//Field 65	//8
	BYTE ucCTC[3];					//Field 66	//3
	BYTE ucProfileExpiryDate[4];	//Field 67	//4
	BYTE ucSubAreaCode[2];			//Field 69	//2
	BYTE ucTxnSN[3];				//Field 70	//3
	BYTE ucEV[3];					//Field 71	//3
	BYTE ucCPUAdminKeyKVN;			//Field 74-1	//1
	BYTE ucCreditKeyKVN;			//Field 74-2	//1
	BYTE ucCPUIssuerKeyKVN; 		//Field 74-3	//1
	BYTE ucTxnMode; 				//Field 75	//1
	BYTE ucTxnQuqlifier;			//Field 76	//1
	BYTE ucCPUSAMID[8]; 			//Field 79	//8
	BYTE ucHostAdminKVN;			//Field 87	//1
	BYTE ucPurseVersionNumber;		//Field 89	//1
	BYTE ucTMLocationID[10];		//Field 90	//10
	BYTE ucCACrypto[16];			//Field 101 //16
	CardAVR_t stCardAVRInfo_t;		//Field 104 //45
	BYTE ucCardIDLen;				//Field 106 //1
}SetValue2_TM_Out;
/*******************************SetValue2******************************/

/*****************************AuthSetValue2***************************/
typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];				//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucMsgType; 				//Field 7	//1
	BYTE ucSubType; 				//Field 8	//1
	BYTE ucDeviceID[4]; 			//Field 9	//4
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucExpiryDate[4];			//Field 25	//4
	BYTE ucTMID[2]; 				//Field 28	//2
	BYTE ucTMTxnDateTime[14];		//Field 29	//14
	BYTE ucTMSerialNumber[6];		//Field 30	//6
	BYTE ucTMAgentNumber[4];		//Field 31	//4
	BYTE ucHTAC[8]; 				//Field 34	//8
	BYTE ucResponseCode[2]; 		//Field 43	//2
	BYTE ucCardID[7];				//Field 58	//7
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucPID[8];					//Field 65	//8
	BYTE ucProfileExpiryDate[4];	//Field 68	//4
	BYTE ucPurseVersionNumber;		//Field 89	//1
	BYTE ucTMLocationID[10];		//Field 90	//10
	BYTE ucHVCrypto[16];			//Field 102 //16
	BYTE ucCardIDLen;				//Field 106 //1
	BYTE ucLCDControlFlag;
}AuthSetValue2_TM_In;

typedef struct __packed_back{
	BYTE anCardID[17];
	BYTE anDeviceID[10];
	BYTE anCPUDeviceID[16];
	BYTE anEVBeforeTxn[8];
	BYTE anOrgExpiryDate[8];
	BYTE anNewExpiryDate[8];
	BYTE anProcessignCode[6];
}AuthSetValue2_TM_Out;
/*****************************AuthSetValue2***************************/

/********************************TxRefund2******************************/
typedef struct __packed_back{
	BYTE anMsgType[2];
	BYTE anSubType[2];
	BYTE ucTMLocationID[10];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE anTxnAmt[8];
	BYTE ucTMTxnDateTime2[14];
	BYTE ucTMSerialNumber2[6];
}TxRefund2_TM_In;

/*typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];				//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucBLVersion[2];			//Field 6	//2
	BYTE ucMsgType; 				//Field 7	//1
	BYTE ucSubType; 				//Field 8	//1
	BYTE ucDeviceID[4]; 			//Field 9	//4
	BYTE ucSPID;					//Field 10	//1
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucCardType;				//Field 12	//1
	BYTE ucPersonalProfile; 		//Field 13	//1
	BYTE ucIssuerCode;				//Field 15	//1
	BYTE ucLocationID;				//Field 19	//1
	BYTE ucBankCode;				//Field 20	//1
	BYTE ucLoyaltyCounter[2];		//Field 21	//2
	BYTE ucExpiryDate[4];			//Field 24	//4
	BYTE ucAreaCode;				//Field 26	//1
	BYTE ucTMID[2]; 				//Field 28	//2
	BYTE ucTMTxnDateTime[14];		//Field 29	//14
	BYTE ucTMSerialNumber[6];		//Field 30	//6
	BYTE ucTMAgentNumber[4];		//Field 31	//4
	BYTE ucSTAC[8]; 				//Field 33	//8
	BYTE ucKeyVersion;				//Field 35	//1
	BYTE ucSAMID[8];				//Field 36	//8
	BYTE ucSAMSN[4];				//Field 37	//4
	BYTE ucSAMCRN[8];				//Field 38	//8
	BYTE ucReaderFWVersion[6];		//Field 39	//6
	BYTE ucCardID[7];				//Field 58	//7
	BYTE ucTxnAmt[3];				//Field 59	//3
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucCPUSPID[3];				//Field 63	//3
	BYTE ucCPULocationID[2];		//Field 64	//2
	BYTE ucPID[8];					//Field 65	//8
	BYTE ucCTC[3];					//Field 66	//3
	BYTE ucProfileExpiryDate[4];	//Field 67	//4
	BYTE ucSubAreaCode[2];			//Field 69	//2
	BYTE ucTxnSN[3];				//Field 70	//3
	BYTE ucEV[3];					//Field 71	//3
	BYTE ucDeposit[3];				//Field 72	//3
	BYTE ucCPUAdminKeyKVN;			//Field 74-1//1
	BYTE ucCreditKeyKVN;			//Field 74-2//1
	BYTE ucCPUIssuerKeyKVN; 		//Field 74-3//1
	BYTE ucTxnMode; 				//Field 75	//1
	BYTE ucTxnQuqlifier;			//Field 76	//1
	BYTE ucSignatureKeyKVN;			//Field 77	//1
	BYTE ucCPUSAMID[8]; 			//Field 79	//8
	BYTE ucHostAdminKVN;			//Field 87	//1
	BYTE ucPurseVersionNumber;		//Field 89	//1
	BYTE ucTMLocationID[10];		//Field 90	//10
	BYTE ucCACrypto[16];			//Field 101 //16
	CardAVR_t stCardAVRInfo_t;		//Field 104 //45
	ReaderAVRTM_t stReaderAVRInfo_t;//Field 105	//83
	BYTE ucCardIDLen;				//Field 106 //1
}TxRefund2_TM_Out;*/
/********************************TxRefund2******************************/

/******************************AuthTxRefund2****************************/
/*typedef struct __packed_back{
	BYTE ucRFU4[4];
	BYTE ucHTAC[8];
	BYTE ucRFU2[2];
	BYTE ucLCDControlFlag;
}AuthTxRefund2_TM_In;*/

typedef struct __packed_back{
	//餘轉退卡交易
	STMC_t stTxRefundSTMC_t;
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];				//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucMsgType; 				//Field 7	//1
	BYTE ucSubType; 				//Field 8	//1
	BYTE ucDeviceID[4]; 			//Field 9	//4
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucMAC[10]; 				//Field 23	//10
	BYTE ucConfirmCode[2];			//Field 47	//2
	BYTE ucCTAC[8]; 				//Field 48	//8
	BYTE ucCardID[7];				//Field 58	//7
	BYTE ucEV[3];					//Field 60	//3
	BYTE ucTxnSN[3];				//Field 61	//3
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucPID[8];					//Field 65	//8
	BYTE ucSignature[16];			//Field 78	//16
	BYTE ucHashType;				//Field 86	//1
	BYTE ucHostAdminKVN;			//Field 87	//1
	BYTE ucCPUMAC[16];				//Field 88	//16
	//餘轉扣款交易
	STMC_t stTxDeductSTMC_t;
	//加值重送資料開始
	BYTE unVARMsgType[2];
	BYTE unVARPurseVersionNumber[2];
	BYTE unVARTxnSN[6];
	BYTE unVARTxnDateTime[8];
	BYTE unVARSubType[2];
	BYTE unVARTxnAmt[6];
	BYTE unVAREV[6];
	BYTE unVARCardID[14];
	BYTE unVARIssuerCode[2];
	BYTE unVARCPUSPID[6];
	BYTE unVARCPULocationID[4];
	BYTE unVARPID[16];
	BYTE unVARCPUDeviceID[12];
	BYTE unVARLoyaltyCounter[4];
	BYTE unVARPersonalProfile[2];
	BYTE unVARBankCode[2];
	BYTE unVARCardIDLen[2];
	BYTE unVAR2DeviceID[8];
	BYTE unVAR2SPID[2];
	BYTE unVAR2LocationID[2];
	BYTE unVAR2CPUDeviceID[12];
	BYTE unVAR2CPUSPID[6];
	BYTE unVAR2CPULocationID[4];
}AuthTxRefund2_TM_Out;
/******************************AuthTxRefund2****************************/

/*****************************AutoloadEnable2***************************/
typedef struct __packed_back{
	BYTE anMsgType[2];
	BYTE anSubType[2];
	BYTE ucTMLocationID[10];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE anTxnAmt[8];
	BYTE ucReadPurseFlag;
}AutoloadEnable2_TM_In;

typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];				//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucBLVersion[2];			//Field 6	//2
	BYTE ucMsgType;					//Field 7	//1
	BYTE ucSubType;					//Field 8	//1
	BYTE ucDeviceID[4];				//Field 9	//4
	BYTE ucSPID;					//Field 10	//1
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucCardType;				//Field 12	//1
	BYTE ucPersonalProfile;			//Field 13	//1
	BYTE ucIssuerCode;				//Field 15	//1
	BYTE ucLocationID;				//Field 19	//1
	BYTE ucBankCode;				//Field 20	//1
	BYTE ucLoyaltyCounter[2];		//Field 21	//2
	BYTE ucExpiryDate[4];			//Field 24	//4
	BYTE ucAreaCode;				//Field 26	//1
	BYTE ucTMID[2];					//Field 28	//2
	BYTE ucTMTxnDateTime[14];		//Field 29	//14
	BYTE ucTMSerialNumber[6];		//Field 30	//6
	BYTE ucTMAgentNumber[4];		//Field 31	//4
	BYTE ucSTAC[8];					//Field 33	//8
	BYTE ucKeyVersion;				//Field 35	//1
	BYTE ucSAMID[8];				//Field 36	//8
	BYTE ucSAMSN[4];				//Field 37	//4
	BYTE ucSAMCRN[8];				//Field 38	//8
	BYTE ucReaderFWVersion[6];		//Field 39	//6
	BYTE ucAutoLoadFlag;			//Field 53-1//1
	BYTE ucAutoLoadAmt_2[2];		//Field 53-2//2
	BYTE ucCardID[7];				//Field 58	//7
	BYTE ucTxnAmt[3];				//Field 59	//3
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucCPUSPID[3];				//Field 63	//3
	BYTE ucCPULocationID[2];		//Field 64	//2
	BYTE ucPID[8];					//Field 65	//8
	BYTE ucCTC[3];					//Field 66	//3
	BYTE ucProfileExpiryDate[4];	//Field 67	//4
	BYTE ucSubAreaCode[2];			//Field 69	//2
	BYTE ucTxnSN[3];				//Field 70	//3
	BYTE ucEV[3];					//Field 71	//3
	BYTE ucDeposit[3];				//Field 72	//3
	BYTE ucCPUAdminKeyKVN;			//Field 74-1//1
	BYTE ucCreditKeyKVN;			//Field 74-2//1
	BYTE ucCPUIssuerKeyKVN;			//Field 74-3//1
	BYTE ucTxnMode;					//Field 75	//1
	BYTE ucTxnQuqlifier;			//Field 76	//1
	BYTE ucSignatureKeyKVN;			//Field 77	//1
	BYTE ucCPUSAMID[8];				//Field 79	//8
	BYTE ucHostAdminKVN;			//Field 87	//1
	BYTE ucPurseVersionNumber;		//Field 89	//1
	BYTE ucTMLocationID[10];		//Field 90	//10
	BYTE ucCACrypto[16];			//Field 101	//16
	CardAVR_t stCardAVRInfo_t;		//Field 104	//45
	ReaderAVRTM_t stReaderAVRInfo_t;//Field 105	//83
	BYTE ucCardIDLen;				//Field 106	//1
}AutoloadEnable2_TM_Out;
/*****************************AutoloadEnable2***************************/

/***************************AuthAutoloadEnable2*************************/
typedef struct __packed_back{
	STMC_t stSTMC_t;
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];				//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucMsgType; 				//Field 7	//1
	BYTE ucSubType; 				//Field 8	//1
	BYTE ucDeviceID[4]; 			//Field 9	//4
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucMAC[10]; 				//Field 23	//10
	BYTE ucConfirmCode[2];			//Field 47	//2
	BYTE ucCTAC[8]; 				//Field 48	//8
	BYTE ucAutoLoadFlag;			//Field 53-1//1
	BYTE ucAutoLoadAmt_2[2];		//Field 53-2//2
	BYTE ucCardID[7];				//Field 58	//7
	BYTE ucEV[3];					//Field 60	//3
	BYTE ucTxnSN[3];				//Field 61	//3
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucPID[8];					//Field 65	//8
	BYTE ucSignature[16];			//Field 78	//16
	BYTE ucHashType;				//Field 86	//1
	BYTE ucHostAdminKVN;			//Field 87	//1
	BYTE ucCPUMAC[16];				//Field 88	//16
}AuthAutoloadEnable2_TM_Out;
/***************************AuthAutoloadEnable2*************************/

/******************************AuthAutoload*****************************/
typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];				//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucMsgType; 				//Field 7	//1
	BYTE ucSubType; 				//Field 8	//1
	BYTE ucDeviceID[4]; 			//Field 9	//4
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucExpiryDate[4];			//Field 25	//4
	BYTE ucTMID[2]; 				//Field 28	//2
	BYTE ucTMTxnDateTime[14];		//Field 29	//14
	BYTE ucTMSerialNumber[6];		//Field 30	//6
	BYTE ucTMAgentNumber[4];		//Field 31	//4
	BYTE ucHTAC[8]; 				//Field 34	//8
	BYTE ucResponseCode[2]; 		//Field 43	//2
	BYTE ucCardID[7];				//Field 58	//7
	BYTE ucTxnAmt[3];				//Field 59	//3
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucPID[8];					//Field 65	//8
	BYTE ucProfileExpiryDate[4];	//Field 68	//4
	BYTE ucPurseVersionNumber;		//Field 89	//1
	BYTE ucTMLocationID[10];		//Field 90	//10
	BYTE ucHATOKEN[16]; 			//Field 92	//16
	BYTE ucCardIDLen;				//Field 106 //1
	BYTE ucLCDControlFlag;
}AuthAutoload_TM_In;//For 自動加值

typedef struct __packed_back{
	STMC_t stSTMC_t;
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];				//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucMsgType; 				//Field 7	//1
	BYTE ucSubType; 				//Field 8	//1
	BYTE ucDeviceID[4]; 			//Field 9	//4
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucMAC[10]; 				//Field 23	//10
	BYTE ucConfirmCode[2];			//Field 47	//2
	BYTE ucCTAC[8]; 				//Field 48	//8
	BYTE ucCardID[7];				//Field 58	//7
	BYTE ucTxnAmt[3];				//Field 59	//3
	BYTE ucEV[3];					//Field 60	//3
	BYTE ucTxnSN[3];				//Field 61	//3
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucPID[8];					//Field 65	//8
	BYTE ucSignature[16];			//Field 78	//16
	BYTE ucHashType;				//Field 86	//1
	BYTE ucHostAdminKVN;			//Field 87	//1
	BYTE ucCPUMAC[16];				//Field 88	//16
}AuthAutoload_TM_Out;
/******************************AuthAutoload*****************************/

/****************************ReadCardNumber*****************************/
typedef struct __packed_back{
	BYTE anTxnAmt[8];
	BYTE ucLCDControlFlag;
}ReadCardNumber_TM_In;

typedef struct __packed_back{
	BYTE anCardID[17];
	BYTE ucCardClass;
}ReadCardNumber_TM_Out;
/****************************ReadCardNumber*****************************/

/*******************************FareSale********************************/
typedef struct __packed_back{
    BYTE anFareProductType[3];		//特種票種類
    BYTE ucTMTxnDateTime[14];		//交易時間
    BYTE anCardNumber[10];			//卡號
    BYTE anAgentNumber[5];			//操作人員代碼
    BYTE anGroupsize[3];            //the numpber of persons in a group
    BYTE anEntryStation[3];
    BYTE anExitStation[3];
    BYTE anJourneyNumber[3];        //maximum number of uses per period
    BYTE anFirstDate[8];
    BYTE anLastDate[8];
}FareSale_TM_In;
/*******************************FareSale********************************/

/*******************************VoidFare********************************/
typedef struct __packed_back{
    BYTE ucTMTxnDateTime[14];		//交易時間
    BYTE anAgentNumber[5];			//操作人員代碼
}VoidFare_TM_In;
/*******************************VoidFare********************************/

/*******************************RefundFare********************************/
typedef struct __packed_back{
    BYTE anFareProductType[3];		//特種票種類
    BYTE ucTMTxnDateTime[14];		//交易時間
    BYTE anCardNumber[10];			//卡號
}RefundFare_TM_In;
/*******************************RefundFare********************************/

/*******************************TRTCRead********************************/
typedef struct __packed_back{
	BYTE ucLCDControlFlag;
	BYTE ucTMSerialNumber[6];
	BYTE ucTMTxnDateTime[14];		//交易時間
	BYTE ucRFU[5];
}TRTCRead_TM_In;
/*******************************TRTCRead********************************/

/*******************************TRTCSetValue********************************/
typedef struct __packed_back{
	BYTE ucFuncOption;			   //TM0x01修改進出站碼0x02愛心票註記
	BYTE ucTxnDateTime[14]; 		   //TM交易日期時間UnsignedandLSBFirst(UnixDateTime)
	BYTE ucURTData[52];				   //TM欲修改之轉乘資料,PVN=0時,用前16Byte/PVN<>0時,用26Byte
	BYTE ucUTRData[52];				   //TM欲修改之六筆資料,PVN=0時,用前16Byte/PVN<>0時,用26Byte
	BYTE ucTRTCData[52];				   //TM欲修改之捷運資料,PVN=0時,用前16Byte/PVN<>0時,用26Byte
	BYTE ucLCDControlFlag;		   //TM用於控制交易完成後之LCD顯示0x00：顯示【交易完成　請取卡】(default)0x01：顯示【（請勿移動票卡）】
}TRTCSetValue_TM_In;
/*******************************TRTCSetValue********************************/

/*******************************TRTCSetValue********************************/
typedef struct __packed_back{
	//BYTE anMsgType[2];
	//BYTE anSubType[2];
	BYTE ucTMLocationID[10];
	BYTE ucTMID[2];
	BYTE ucTMTxnDateTime[14];
	BYTE ucTMSerialNumber[6];
	BYTE ucTMAgentNumber[4];
	BYTE anTxnAmt[8];
	BYTE ucReadPurseFlag;
	BYTE fNormalUpdateFlag:1;								// Bit 0：一般展期
	BYTE fPersonalUpdateFlag:1; 							// Bit 1：身份變更
	BYTE fExpireUpdateFlag:1;								// Bit 2：逾期展期
	BYTE fAreaUpdateFlag:1; 								// Bit 3：Area Code變更
	BYTE fAuthenUpdateFlag:1;								// Bit 4：地區認證旗標變更
	BYTE fRFUFlag:3;										// Bit 5~7：RFU
	BYTE ucRFUFlag[ 1 ];									// 保留
	BYTE anNewExpiryDate[14];
	BYTE anNewPersonalExpiryDate[14];
	BYTE ucNewPersonalProfile[ 1 ]; 						// 新身分別
	BYTE ucNewAreaCode[ 1 ];								// 新區碼
	BYTE ucNewAuthen[ 1 ];									// 新地區認證旗標
	BYTE ucNewCPUAuthen[ 1 ];								// 新CPU地區認證旗標
	BYTE ucNewSubAreaCode[ 2 ]; 							// 新子區碼
	BYTE ucLCDControlFlag;
}SetValueOffline_TM_In;
/*******************************TRTCSetValue********************************/

/*****************************StudentSetValue*******************************/
typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];				//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucBLVersion[2];			//Field 6	//2
	BYTE ucMsgType; 				//Field 7	//1
	BYTE ucSubType; 				//Field 8	//1
	BYTE ucDeviceID[4]; 			//Field 9	//4
	BYTE ucSPID;					//Field 10	//1
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucCardType;				//Field 12	//1
	BYTE ucPersonalProfile; 		//Field 13	//1
	BYTE ucIssuerCode;				//Field 15	//1
	BYTE ucLocationID;				//Field 19	//1
	BYTE ucBankCode;				//Field 20	//1
	BYTE ucLoyaltyCounter[2];		//Field 21	//2
	BYTE ucExpiryDate[4];			//Field 24	//4
	BYTE ucAreaCode;				//Field 26	//1
	BYTE ucTMID[2]; 				//Field 28	//2
	BYTE ucTMTxnDateTime[14];		//Field 29	//14
	BYTE ucTMSerialNumber[6];		//Field 30	//6
	BYTE ucTMAgentNumber[4];		//Field 31	//4
	BYTE ucSTAC[8]; 				//Field 33	//8
	BYTE ucKeyVersion;				//Field 35	//1
	BYTE ucSAMID[8];				//Field 36	//8
	BYTE ucSAMSN[4];				//Field 37	//4
	BYTE ucSAMCRN[8];				//Field 38	//8
	BYTE ucReaderFWVersion[6];		//Field 39	//6
	BYTE ucCardID[7];				//Field 58	//7
	BYTE ucTxnAmt[3];				//Field 59	//3
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucCPUSPID[3];				//Field 63	//3
	BYTE ucCPULocationID[2];		//Field 64	//2
	BYTE ucPID[8];					//Field 65	//8
	BYTE ucCTC[3];					//Field 66	//3
	BYTE ucProfileExpiryDate[4];	//Field 67	//4
	BYTE ucSubAreaCode[2];			//Field 69	//2
	BYTE ucTxnSN[3];				//Field 70	//3
	BYTE ucEV[3];					//Field 71	//3
	BYTE ucDeposit[3];				//Field 72	//3
	BYTE ucCPUAdminKeyKVN;			//Field 74-1	//1
	BYTE ucCreditKeyKVN;			//Field 74-2	//1
	BYTE ucCPUIssuerKeyKVN; 		//Field 74-3	//1
	BYTE ucTxnMode; 				//Field 75	//1
	BYTE ucTxnQuqlifier;			//Field 76	//1
	BYTE ucSignatureKeyKVN; 		//Field 77	//1
	BYTE ucCPUSAMID[8]; 			//Field 79	//8
	BYTE ucHostAdminKVN;			//Field 87	//1
	BYTE ucPurseVersionNumber;		//Field 89	//1
	BYTE ucTMLocationID[10];		//Field 90	//10
	BYTE ucCACrypto[16];			//Field 101 //16
	CardAVR_t stCardAVRInfo_t;		//Field 104 //45
	ReaderAVRTM_t stReaderAVRInfo_t;//Field 105 //83
	BYTE ucCardIDLen;				//Field 106 //1
	BYTE ucCPUIssuerKey;				//Field 113 //1
}StudentSetValue_TM_Out;//For 學生票展期
/*****************************StudentSetValue*******************************/

/***************************AuthStudentSetValue*****************************/
typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];					//Field 1	//3
	BYTE ucProcessignCode[6];			//Field 2	//6
	BYTE ucMsgType; 					//Field 7	//1
	BYTE ucSubType; 					//Field 8	//1
	BYTE ucDeviceID[4]; 				//Field 9	//4
	BYTE ucTxnDateTime[4];				//Field 11	//4
	BYTE ucCardType;					//Field 12	//1
	BYTE ucExpiryDate[4];				//Field 25	//4
	BYTE ucTMID[2]; 					//Field 28	//2
	BYTE ucTMTxnDateTime[14];			//Field 29	//14
	BYTE ucTMSerialNumber[6];			//Field 30	//6
	BYTE ucTMAgentNumber[4];			//Field 31	//4
	BYTE ucHTAC[8]; 					//Field 34	//8
	BYTE ucResponseCode[2]; 			//Field 43	//2
	BYTE ucCardID[7];					//Field 58	//7
	BYTE ucTxnAmt[3];					//Field 59	//3
	BYTE ucCPUDeviceID[6];				//Field 62	//6
	BYTE ucPID[8];						//Field 65	//8
	BYTE ucProfileExpiryDate[4];		//Field 68	//4
	BYTE ucPurseVersionNumber;			//Field 89	//1
	BYTE ucTMLocationID[10];			//Field 90	//10
	BYTE ucHATOKEN[16]; 				//Field 92	//16
	BYTE ucCPUHostCryptogram[16];		//File 102 //16
	BYTE ucCardIDLen;					//Field 106 //1
	BYTE ucCPUCardSettingParameter[16]; //File 109 //16
	BYTE ucLCDControlFlag;
}AuthStudentSetValue_TM_In;//For 學生票展期


typedef struct __packed_back{
	BYTE anTxnAmt[8];
	BYTE anCardType[2];
	BYTE anProfileExpiryDate[8];
	STMC_t stSTMC_t;
	//TXET_t stTXET_t;
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];			//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucMsgType; 			//Field 7	//1
	BYTE ucSubType; 			//Field 8	//1
	BYTE ucDeviceID[4]; 		//Field 9	//4
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucMAC[10]; 			//Field 23	//10
	BYTE ucConfirmCode[2];			//Field 47	//2
	BYTE ucCTAC[8]; 			//Field 48	//8
	BYTE ucCardID[7];			//Field 58	//7
	BYTE ucEV[3];				//Field 60	//3
	BYTE ucTxnSN[3];			//Field 61	//3
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucPID[8];				//Field 65	//8
	BYTE ucSignature[16];			//Field 78	//16
	BYTE ucHashType;			//Field 86	//1
	BYTE ucHostAdminKVN;			//Field 87	//1
	BYTE ucCPUMAC[16];			//Field 88	//16
}AuthStudentSetValue_TM_Out;//For 學生票展期
/***************************AuthStudentSetValue*****************************/

/*********************************學生展期*********************************/
//typedef struct __packed_back{
//}SIS2_STUDENT_SET_VALUE;
/*********************************學生展期*********************************/

//add by bean 20141027 start
typedef struct __packed_back{
	BYTE anCardCheckFlag1;
	BYTE anCardCheckFlag2;
	BYTE anCardCheckFlag3;
	BYTE anCardCheckFlag4;
	BYTE anEZCard_SAMSlot_Address[2];
	BYTE anIPASS_SAMSlot_Address[2];
	BYTE anSAMSlot_Address_RFU1[2];
	BYTE anSAMSlot_Address_RFU2[2];
	BYTE anSAMSlot_Address_RFU3[2];
	BYTE anCardPollingFlag;
	BYTE anReaderDisplayMode;
	BYTE anPollingTime[2];
}MMSelectCard2_TM_In;

typedef struct __packed_back{
	BYTE anCardIDLen[2];
	BYTE anCardID[17];
	BYTE anCardType[2];
}MMSelectCard2_TM_Out;
//add by bean 20141027 end

/*****************************AuthAccuntLink***************************/
typedef struct __packed_back{
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];				//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucMsgType;					//Field 7	//1
	BYTE ucSubType;					//Field 8	//1
	BYTE ucDeviceID[4];				//Field 9	//4
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucBankCode;				//Field 20	//1
	BYTE ucExpiryDate[4];			//Field 25	//4
	BYTE ucTMID[2];					//Field 28	//2
	BYTE ucTMTxnDateTime[14];		//Field 29	//14
	BYTE ucTMSerialNumber[6];		//Field 30	//6
	BYTE ucTMAgentNumber[4];		//Field 31	//4
	BYTE ucHTAC[8];					//Field 34	//8
	BYTE ucResponseCode[2];			//Field 43	//2
	BYTE ucAutoLoadFlag;			//Field 53-1//1
	BYTE ucAutoLoadAmt_2[2];		//Field 53-2//2
	BYTE ucCardID[7];				//Field 58	//7
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucPID[8];					//Field 65	//8
	BYTE ucProfileExpiryDate[4];	//Field 68	//4
	BYTE ucPurseVersionNumber;		//Field 89	//1
	BYTE ucTMLocationID[10];		//Field 90	//10
	BYTE ucHATOKEN[16];				//Field 92	//16
	BYTE ucHVCrypto[16];			//Field 102	//16
	BYTE ucCardIDLen;				//Field 106	//1
	BYTE ucLCDControlFlag;
}AuthAccuntLink_TM_In;//For 加值

typedef struct __packed_back{
	STMC_t stSTMC_t;
	BYTE ucMessageType[4];
	BYTE ucDataLen[3];				//Field 1	//3
	BYTE ucProcessignCode[6];		//Field 2	//6
	BYTE ucMsgType;					//Field 7	//1
	BYTE ucSubType;					//Field 8	//1
	BYTE ucDeviceID[4];				//Field 9	//4
	BYTE ucTxnDateTime[4];			//Field 11	//4
	BYTE ucBankCode;				//Field 20	//1
	BYTE ucMAC[10];					//Field 23	//10
	BYTE ucConfirmCode[2];			//Field 47	//2
	BYTE ucCTAC[8];					//Field 48	//8
	BYTE ucAutoLoadFlag;			//Field 53-1//1
	BYTE ucAutoLoadAmt_2[2];		//Field 53-2//2
	BYTE ucCardID[7];				//Field 58	//7
	BYTE ucEV[3];					//Field 60	//3
	BYTE ucTxnSN[3];				//Field 61	//3
	BYTE ucCPUDeviceID[6];			//Field 62	//6
	BYTE ucPID[8];					//Field 65	//8
	BYTE ucSignature[16];			//Field 78	//16
	BYTE ucHashType;				//Field 86	//1
	BYTE ucHostAdminKVN;			//Field 87	//1
	BYTE ucCPUMAC[16];				//Field 88	//16
}AuthAccuntLink_TM_Out;
 /*****************************AuthAccuntLink***************************/

 /*******************************TaxiRead*******************************/
 typedef struct __packed_back{
	 BYTE ucLCDControlFlag;
	 BYTE ucTMLocationID[10];
	 BYTE ucTMID[2];
	 BYTE ucTMTxnDateTime[14];
	 BYTE ucTMSerialNumber[6];
	 BYTE ucTMAgentNumber[4];
 }TaxiRead_TM_In;
 /*******************************TaxiRead*******************************/

void vdBuildMessageType(int inTxnType,BYTE *cOut);
void vdBuildProcessingCode(int inTxnType,BYTE ucMsgType,BYTE ucSubType,BYTE ucPersonalProfile,BYTE *cOut);
void vdCountEVBefore(BYTE *bAmt,int inAmtSize,BYTE *bEV,int inEVSize,BYTE *bEVBefore,int inEVBeforeSize,BYTE ucMsgType);
BOOL fnASCTOBIN(BYTE *OutData,BYTE ASCData[],int sizex,int OffsetBit,int mode);
void vdUnPackAmt2ByteTo3Byte(BYTE *bIn2Amt,BYTE *bOutAmt);
void vdAmt2ByteTo3Byte(BYTE *bIn2Amt,BYTE *bOut3Amt);
void fngetDosDate(BYTE *bDosDate,BYTE *bOutDate);
void fnBINTODEVASC(BYTE *BINData,BYTE *ASCDATA,int ASCSize,char fPurseVersion);
unsigned long fngetUnixTimeCnt(BYTE *OutDate,BYTE *date);
void fngetDosDateCnt(BYTE *date,BYTE *DosDate);
void vdBuildF37_SAM_SN(BYTE ucPurseVersionNumber,BYTE *ucOutData,BYTE *ucInSAM_SN,BYTE *ucCardID);
void vdBuildF66_CTC(BYTE ucPurseVersionNumber,BYTE *ucOutData,BYTE *ucCTC,BYTE *ucDeviceID);

int inBuildResetData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildSignOnData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildSignOnQueryData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildReadCardBasicDataData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
void vdBuildReadCardBasicData2(BYTE ucMsgType,LastTxnLog_t *DongleOutLog,LastTxnRecoredL2_t *APIOutRecored);
void vdBuildReadCardBasicData2_2(BYTE ucMsgType,LastTxnLog_t *DongleOutLog,LastTxnRecoredL2_t *APIOutRecored);
int inBuildTxnReqOnlineData1(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildTxnReqOnlineData2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildTxnReqOnlineData3(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildTxnReqOnlineData4(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
//sint inBuildTxnReqOnlineData4(int inTxnType,STRUCT_XML_DOC *srXML,CancelTxn_TM_Out *APIOut,TxnReqOnline_APDU_Out *DongleOut,BYTE ucMsgType);
void vdBuildCardAVRData(CardAVR_t *APIOutData,LastTxnLog_t *DongleOutLog,BYTE *ucCardID,BYTE ucIssuerCode,BYTE *ucCPUSPID,BYTE *ucCPULocationID,BYTE *ucPID,BYTE *ucLoyaltyCounter);
void vdBuildReaderAVRData(ReaderAVRTM_t *APIOutData,ReaderAVRAPDU_t *DongleOutLog);
int inBuildAuthTxnOnlineData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildAuthTxnOnlineData1(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildTxnReqOfflineData1(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildTxnReqOfflineData2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildTxnReqOfflineData3(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildTxnReqOfflineData4(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildAuthTxnOfflineData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildLockCardData1(int inTxnType,STRUCT_XML_DOC *srXML,LockCard_TM_Out_2 *APIOut,LockCard_APDU_Out_2 *DongleOut,BYTE *ucTxnDateTime,BYTE ucLockReason,BYTE *ucTMLocationID,BYTE *ucTMID);
int inBuildLockCardData2(int inTxnType,STRUCT_XML_DOC *srXML,LockCard_TM_Out_2 *APIOut,LockCard_APDU_Out_3 *DongleOut,BYTE *ucTxnDateTime,BYTE ucLockReason,BYTE *ucTMLocationID,BYTE *ucTMID);
int inBuildLockCardData3(int inTxnType,STRUCT_XML_DOC *srXML,LockCard_TM_Out_2 *APIOut,TxnReqOffline_APDU_Out *DongleOut,BYTE *ucTxnDateTime,BYTE ucLockReason,BYTE *ucTMLocationID,BYTE *ucTMID);
int inBuildLockCardData4(int inTxnType,STRUCT_XML_DOC *srXML,LockCard_TM_Out_1 *APIOut,TxRefund_APDU_Out *DongleOut,BYTE *ucTxnDateTime,BYTE ucLockReason);
int inBuildLockCardData5(int inTxnType,STRUCT_XML_DOC *srXML,LockCard_TM_Out_1 *APIOut,AddValue_APDU_Out *DongleOut,BYTE *ucTxnDateTime,BYTE ucLockReason);
int inBuildLockCardData6(int inTxnType,STRUCT_XML_DOC *srXML,LockCard_TM_Out_1 *APIOut,LockCard_APDU_Out_1 *DongleOut,BYTE *ucTxnDateTime,BYTE ucLockReason);
int inBuildLockCardData7(int inTxnType,STRUCT_XML_DOC *srXML,LockCard_TM_Out_2 *APIOut,TxRefund_APDU_Out *DongleOut,BYTE *ucTxnDateTime,BYTE ucLockReason);
int inBuildLockCardData8(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildLockCardData9(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildReadDongleDeductData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildAuthCardSaleData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildSetValueData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildAuthSetValueData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildTxRefundData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildAuthTxRefundData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildAddValueData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildAuthAddValueData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildAutoloadEnableData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildAuthAutoloadEnableData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildReadCardDeductData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildReadCodeVersionData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildSetValue2Data(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildAuthSetValue2Data(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildTxRefundData2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildAuthTxRefundData2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildAutoloadEnableData2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildAuthAutoloadEnableData2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildAuthAutoloadData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildReadCardNumberData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildStudentSetValueData1(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildStudentSetValueData2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildAuthStudentSetValueData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
//int inBuildTxnReqOnlineData4(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildResetOffData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
void vdBuildBLCVersion(BYTE *bOutData);
int inBuildMMSelectCard2Data(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildAuthAccuntLinkData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);

int inBuildICERETxnData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildICEREQRTxnData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildTxnReqOnlineAutoload(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);

#ifdef __cplusplus
	}
#endif // __cplusplus

#endif
