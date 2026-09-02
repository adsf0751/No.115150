
#ifdef EZ_AIRLINKEDC
#include "_stdAfx.h"
#else
#include "stdAfx.h"
#endif
//#include "Global.h"

extern BYTE gsBlackListVersion[30];
extern Store_Data stStore_DataInfo_t;
extern char gBlcFileFolder[100];

int inBuildSIS2Header(BYTE *bOut_Data,BYTE ucPurseVersionNumber,
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
									BYTE *ucEV_Before)

{
SIS2_HEADER *bOutData = (SIS2_HEADER *)bOut_Data;

	bOutData->ucPurseVersionNumber = ucPurseVersionNumber;
	bOutData->ucMsgType = ucMsgType;
	bOutData->ucSubType = ucSubType;
	bOutData->ucDeviceID_3[0] = ucDeviceID_3[2];
	bOutData->ucDeviceID_3[1] = ucDeviceID_3[0];
	bOutData->ucDeviceID_3[2] = ucDeviceID_3[1];
	//memcpy(bOutData->ucDeviceID_3,ucDeviceID_3,3);
	bOutData->ucSPID = ucSPID;
	memcpy(bOutData->ucTxnDateTime,ucTxnDateTime,sizeof(bOutData->ucTxnDateTime));
	memcpy(bOutData->ucCardID,ucCardID,sizeof(bOutData->ucCardID));
	bOutData->ucIssuerCode = ucIssuerCode;
	bOutData->ucTxnSN_1 = ucTxnSN_1;
	memcpy(bOutData->ucTxnAmt,ucTxnAmt,sizeof(bOutData->ucTxnAmt));
	memcpy(bOutData->ucEV,ucEV,sizeof(bOutData->ucEV));
	bOutData->ucLocationID = ucLocationID;
	bOutData->ucCardIDLen = ucCardIDLen;
	memcpy(bOutData->ucTxnSN,ucTxnSN,sizeof(bOutData->ucTxnSN));
	memcpy(bOutData->ucCPUDeviceID,ucCPUDeviceID,sizeof(bOutData->ucCPUDeviceID));
	memcpy(bOutData->ucCPUSPID,ucCPUSPID,sizeof(bOutData->ucCPUSPID));
	memcpy(bOutData->ucCPULocationID,ucCPULocationID,sizeof(bOutData->ucCPULocationID));
	bOutData->ucNewSubType = ucNewSubType;
	bOutData->ucPersonalProfile = ucPersonalProfile;
	bOutData->ucNewPersonalProfile = ucNewPersonalProfile;
	bOutData->ucACQID = ucACQID;
	memcpy(bOutData->ucPID,ucPID,sizeof(bOutData->ucPID));
	memcpy(bOutData->ucCTC,ucCTC,sizeof(bOutData->ucCTC));
	bOutData->ucAreaCode = ucAreaCode;
	memcpy(bOutData->ucSubAreaCode,ucSubAreaCode,sizeof(bOutData->ucSubAreaCode));
	memcpy(bOutData->ucTxnSN_Before,ucTxnSN_Before,sizeof(bOutData->ucTxnSN_Before));
	memcpy(bOutData->ucEV_Before,ucEV_Before,sizeof(bOutData->ucEV_Before));

	return sizeof(SIS2_HEADER);
}

int inBuildSIS2End(BYTE *bOut_Data,BYTE ucPurseVersionNumber,
								 BYTE *ucMAC,
								 BYTE *ucCPUSAMID,
								 BYTE ucHashType,
								 BYTE ucHostAdminKVN,
								 BYTE *ucHCrypto,
								 BYTE *ucDeviceID)
{
SIS2_END *bOutData = (SIS2_END *)bOut_Data;

	memcpy(bOutData->ucMAC,ucMAC,sizeof(bOutData->ucMAC));
	bOutData->ucMAC[1] = 0x01;
	if(ucPurseVersionNumber == 0x00 && srTxnData.srParameter.chGroupFlag == '1')
		memcpy(&bOutData->ucMAC[6],ucDeviceID,4);
	memcpy(bOutData->ucCPUSAMID,ucCPUSAMID,sizeof(bOutData->ucCPUSAMID));
	bOutData->ucHashType = ucHashType;
	bOutData->ucHostAdminKVN = ucHostAdminKVN;
	memcpy(bOutData->ucHCrypto,ucHCrypto,sizeof(bOutData->ucHCrypto));

	return sizeof(SIS2_END);
}

int inBuildSIS2Body_Add(BYTE *bOut_Data,BYTE *ucLoyaltyCounter,
									  BYTE *ucAgentNumber,
									  BYTE ucBandCode,
									  BYTE *ucCPUSPID,
									  BYTE *ucRFU1,
									  BYTE ucTxnMode,
									  BYTE ucTxnQuqlifier,
									  BYTE ucSignatureKeyKVN,
									  BYTE *ucSignature)
{
SIS2_BODY_ADD *bOutData = (SIS2_BODY_ADD *)bOut_Data;
BYTE bBuf[50];
int inRetVal;

	memcpy(bOutData->ucLoyaltyCounter,ucLoyaltyCounter,sizeof(bOutData->ucLoyaltyCounter));
	memcpy(bOutData->ucAgentNumber,ucAgentNumber,sizeof(bOutData->ucAgentNumber));
	bOutData->ucBandCode = ucBandCode;
	memcpy(bOutData->ucCPUSPID,ucCPUSPID,sizeof(bOutData->ucCPUSPID));
	memset(bBuf,0x00,sizeof(bBuf));
	inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_4103,(char *)bBuf,20,0);
	if(inRetVal < SUCCESS)//沒有4103
		memset(bOutData->ucMachineID,0x20,sizeof(bOutData->ucMachineID));
	else
		vdPad((char *)bBuf,(char *)bOutData->ucMachineID,sizeof(bOutData->ucMachineID),TRUE,0x20);
	memset(bBuf,0x00,sizeof(bBuf));
	inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_4105,(char *)bBuf,20,0);
	if(inRetVal < SUCCESS)//沒有4105
		memset(bOutData->ucCMID,0x20,sizeof(bOutData->ucCMID));
	else
		vdPad((char *)bBuf,(char *)bOutData->ucCMID,sizeof(bOutData->ucCMID),TRUE,0x20);
	memset(bOutData->ucRFU1,0x00,sizeof(bOutData->ucRFU1));
	bOutData->ucTxnMode = ucTxnMode;
	bOutData->ucTxnQuqlifier = ucTxnQuqlifier;
	bOutData->ucSignatureKeyKVN = ucSignatureKeyKVN;
	memcpy(bOutData->ucSignature,ucSignature,sizeof(bOutData->ucSignature));

	return sizeof(SIS2_BODY_ADD);
}

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
										 BYTE *ucSignature)
{
SIS2_BODY_DEDUCT *bOutData = (SIS2_BODY_DEDUCT *)bOut_Data;

	memcpy(bOutData->ucEntryExit,ucEntryExit,sizeof(bOutData->ucEntryExit));
	memcpy(bOutData->ucRFU1_0,ucRFU1_0,sizeof(bOutData->ucRFU1_0));
	bOutData->ucTransferGuoupCode = ucTransferGuoupCode;
	memcpy(bOutData->ucTransferGuoupCode2,ucTransferGuoupCode2,sizeof(bOutData->ucTransferGuoupCode2));
	memcpy(bOutData->ucRFU1_1,ucRFU1_1,sizeof(bOutData->ucRFU1_1));
	memcpy(bOutData->ucTransferDiscount,ucTransferDiscount,sizeof(bOutData->ucTransferDiscount));
	memcpy(bOutData->ucPersonalDiscount,ucPersonalDiscount,sizeof(bOutData->ucPersonalDiscount));
	memcpy(bOutData->ucRFU1_2,ucRFU1_2,sizeof(bOutData->ucRFU1_2));
	//memcpy(bOutData->ucRFU1,ucRFU1,sizeof(bOutData->ucRFU1));
	memcpy(bOutData->ucLoyaltyCounter,ucLoyaltyCounter,sizeof(bOutData->ucLoyaltyCounter));
	memcpy(bOutData->ucAgentNumber,ucAgentNumber,sizeof(bOutData->ucAgentNumber));
	memcpy(bOutData->ucRFU2,ucRFU2,sizeof(bOutData->ucRFU2));
	memcpy(bOutData->ucRFU3,ucRFU3,sizeof(bOutData->ucRFU3));
	memcpy(bOutData->ucRFU4,ucRFU4,sizeof(bOutData->ucRFU4));
	memcpy(bOutData->ucRFU5,ucRFU5,sizeof(bOutData->ucRFU5));
	bOutData->ucTxnMode = ucTxnMode;
	bOutData->ucTxnQuqlifier = ucTxnQuqlifier;
	bOutData->ucSignatureKeyKVN = ucSignatureKeyKVN;
	memcpy(bOutData->ucSignature,ucSignature,sizeof(bOutData->ucSignature));

	return sizeof(SIS2_BODY_DEDUCT);
}

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
											BYTE *ucReCPULocationID)
{
SIS2_ADD_VALUE_RESEND *bOutData = (SIS2_ADD_VALUE_RESEND *)bOut_Data;

	bOutData->ucPurseVersionNumber = ucPurseVersionNumber;
	bOutData->ucMsgType = ucMsgType;
	bOutData->ucSubType = ucSubType;
	if(ucPurseVersionNumber != 0x02)
	{
		bOutData->ucCPUDeviceID[0] = ucCPUDeviceID[2];
		bOutData->ucCPUDeviceID[1] = ucCPUDeviceID[0];
		bOutData->ucCPUDeviceID[2] = ucCPUDeviceID[1];
	}
	else
		memcpy(bOutData->ucCPUDeviceID,ucCPUDeviceID,sizeof(bOutData->ucCPUDeviceID));
	memcpy(bOutData->ucCPUSPID,ucCPUSPID,sizeof(bOutData->ucCPUSPID));
	memcpy(bOutData->ucTxnDateTime,ucTxnDateTime,sizeof(bOutData->ucTxnDateTime));
	memcpy(bOutData->ucCardID,ucCardID,sizeof(bOutData->ucCardID));
	bOutData->ucIssuerCode = ucIssuerCode;
	memcpy(bOutData->ucTxnSN,ucTxnSN,sizeof(bOutData->ucTxnSN));
	memcpy(bOutData->ucTxnAmt,ucTxnAmt,sizeof(bOutData->ucTxnAmt));
	memcpy(bOutData->ucEV,ucEV,sizeof(bOutData->ucEV));
	memcpy(bOutData->ucCPULocationID,ucCPULocationID,sizeof(bOutData->ucCPULocationID));
	bOutData->ucCardIDLen = ucCardIDLen;
	bOutData->ucPersonalProfile = ucPersonalProfile;
	memcpy(bOutData->ucPID,ucPID,sizeof(bOutData->ucPID));
	bOutData->ucBankCode = ucBankCode;
	memcpy(bOutData->ucLoyaltyCounter,ucLoyaltyCounter,sizeof(bOutData->ucLoyaltyCounter));
	bOutData->ucReDeviceID[0] = ucReDeviceID[2];
	bOutData->ucReDeviceID[1] = ucReDeviceID[0];
	bOutData->ucReDeviceID[2] = ucReDeviceID[1];
	//memcpy(bOutData->ucReDeviceID,ucReDeviceID,sizeof(bOutData->ucReDeviceID));
	bOutData->ucReSPID = ucReSPID;
	bOutData->ucReLocationID = ucReLocationID;
	memcpy(bOutData->ucReCPUDeviceID,ucReCPUDeviceID,sizeof(bOutData->ucReCPUDeviceID));
	memcpy(bOutData->ucReCPUSPID,ucReCPUSPID,sizeof(bOutData->ucReCPUSPID));
	memcpy(bOutData->ucReCPULocationID,ucReCPULocationID,sizeof(bOutData->ucReCPULocationID));

	return SIS2_ADD_VALUE_RESEND_LEN;
}

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
								  BYTE *ucRFU)

{
SIS2_LOCK *bOutData = (SIS2_LOCK *)bOut_Data;

	bOutData->ucPurseVersionNumber = ucPurseVersionNumber;
	bOutData->ucMsgType = ucMsgType;
	bOutData->ucPersonalProfile = ucPersonalProfile;
	bOutData->ucDeviceID[0] = ucDeviceID[2];
	bOutData->ucDeviceID[1] = ucDeviceID[0];
	bOutData->ucDeviceID[2] = ucDeviceID[1];
	//memcpy(bOutData->ucDeviceID,ucDeviceID,sizeof(bOutData->ucDeviceID));
	bOutData->ucSPID = ucSPID;
	memcpy(bOutData->ucTxnDateTime,ucTxnDateTime,sizeof(bOutData->ucTxnDateTime));
	memcpy(bOutData->ucCardID,ucCardID,sizeof(bOutData->ucCardID));
	bOutData->ucIssuerCode = ucIssuerCode;
	bOutData->ucLocationID = ucLocationID;
	bOutData->ucCardIDLen = ucCardIDLen;
	memcpy(bOutData->ucCPUDeviceID,ucCPUDeviceID,sizeof(bOutData->ucCPUDeviceID));
	memcpy(bOutData->ucCPUSPID,ucCPUSPID,sizeof(bOutData->ucCPUSPID));
	memcpy(bOutData->ucCPULocationID,ucCPULocationID,sizeof(bOutData->ucCPULocationID));
	bOutData->ucNewSubType = ucNewSubType;
	bOutData->ucNewPersonalProfile = ucNewPersonalProfile;
	memcpy(bOutData->ucPID,ucPID,sizeof(bOutData->ucPID));
	memcpy(bOutData->ucCTC,ucCTC,sizeof(bOutData->ucCTC));
	bOutData->ucLockReason = ucLockReason;
	memcpy(bOutData->ucBLCName,ucBLCName,sizeof(bOutData->ucBLCName));
	bOutData->ucLockIDFlag = ucLockIDFlag;
	memset(ucDevInfo,0x20,sizeof(bOutData->ucDevInfo));
	memcpy(bOutData->ucDevInfo,ucDevInfo,strlen((char *)ucDevInfo));
	memcpy(bOutData->ucEV,ucEV,sizeof(bOutData->ucEV));
	memcpy(bOutData->ucTxnSN,ucTxnSN,sizeof(bOutData->ucTxnSN));
	memcpy(bOutData->ucLastTxnSN,ucLastTxnSN,sizeof(bOutData->ucLastTxnSN));
	memcpy(bOutData->ucLastTxnDateTime,ucLastTxnDateTime,sizeof(bOutData->ucLastTxnDateTime));
	bOutData->ucLastSubType = ucLastSubType;
	memcpy(bOutData->ucLastTxnAmt,ucLastTxnAmt,sizeof(bOutData->ucLastTxnAmt));
	memcpy(bOutData->ucLastEV,ucLastEV,sizeof(bOutData->ucLastEV));
	if(ucPurseVersionNumber != 0x02)
	{
		bOutData->ucLastCPUDeviceID[0] = ucLastCPUDeviceID[2];
		bOutData->ucLastCPUDeviceID[1] = ucLastCPUDeviceID[0];
		bOutData->ucLastCPUDeviceID[2] = ucLastCPUDeviceID[1];
	}
	else
		memcpy(bOutData->ucLastCPUDeviceID,ucLastCPUDeviceID,sizeof(bOutData->ucLastCPUDeviceID));
	memcpy(bOutData->ucEV2,ucEV2,sizeof(bOutData->ucEV2));
	bOutData->ucMifareSetPar = ucMifareSetPar;
	bOutData->ucCPUSetPar = ucCPUSetPar;
	memcpy(bOutData->ucRFU,ucRFU,sizeof(bOutData->ucRFU));

	return SIS2_LOCK_LEN;
}

int inSIS2SaveBlob()
{
int i,inRetVal;
long inRecLen = 0L;
char chSaveName[100];
unsigned long ulSPID = 0L,ulCPUSPID = 0L,ulCPULocID = 0L,ulCPUDevNo = 0L,ulCardTypeID = 0L;
int inSaveMode,inSaveLen;
BYTE bSaveData[300],bHeadData[15],bSendType;

	memset(chSaveName,0x00,sizeof(chSaveName));

	for(i=0;i<4;i++)
	{
		inSaveMode = 0;
		bSendType = 'U';
		memset(bSaveData,0x00,sizeof(bSaveData));
		switch(i)
		{
			case 0://扣款
				if(strlen((char *)srTxnData.srIngData.anSETM_STMC_1) == 0)
					break;

				if(srTxnData.srIngData.inTransType == TXN_ECC_ADD_OFFLINE ||
				   srTxnData.srIngData.inTransType == TXN_ECC_ADD_OFFLINE1||
				   srTxnData.srIngData.inTransType == TXN_ECC_ADD ||
				   srTxnData.srIngData.inTransType == TXN_ECC_ADD3 ||
				   srTxnData.srIngData.inTransType == TXN_ECC_AUTOLOAD ||
				   srTxnData.srIngData.inTransType == TXN_ECC_ADD4 ||
				   srTxnData.srIngData.inTransType == TXN_ECC_PT_ADD ||
				   srTxnData.srIngData.inTransType == TXN_E_ADD ||
				   srTxnData.srIngData.inTransType == TXN_E_REFUND ||
				   srTxnData.srIngData.inTransType == TXN_E_QR_ADD ||
				   srTxnData.srIngData.inTransType == TXN_E_QR_REFUND
				   )
				{
					ulCardTypeID = 2;
					inRecLen = SIS2_TXN_BLOB_2_LEN;
					if(srTxnData.srIngData.inTransType == TXN_E_ADD ||
					   srTxnData.srIngData.inTransType == TXN_E_REFUND ||
					   srTxnData.srIngData.inTransType == TXN_E_QR_ADD ||
					   srTxnData.srIngData.inTransType == TXN_E_QR_REFUND
					   )
						bSendType = 'D';
				}
				else if(srTxnData.srIngData.inTransType == TXN_ECC_FARE_SALE || srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT2)
				{
					ulCardTypeID = 3;
					inRecLen = SIS2_TXN_BLOB_5_LEN;
				}
				else if(srTxnData.srIngData.inTransType == TXN_ECC_VOID_FARE)
				{
					ulCardTypeID = 10;
					inRecLen = SIS2_TXN_BLOB_6_LEN;
				}
				else if(srTxnData.srIngData.inTransType == TXN_ECC_REFUND_FARE)
				{
					ulCardTypeID = 6;
					inRecLen = SIS2_TXN_BLOB_7_LEN;
				}
				else if(srTxnData.srIngData.inTransType == TXN_ECC_VOID_OFFLINE ||
					    srTxnData.srIngData.inTransType == TXN_ECC_VOID_OFFLINE1 ||
					    srTxnData.srIngData.inTransType == TXN_ECC_VOID1 ||
						srTxnData.srIngData.inTransType == TXN_E_VOID_ADD ||
						srTxnData.srIngData.inTransType == TXN_E_QR_VOID_ADD)
				{
					ulCardTypeID = 11;
					inRecLen = SIS2_TXN_BLOB_2_LEN;
					if( srTxnData.srIngData.inTransType == TXN_E_VOID_ADD ||
						srTxnData.srIngData.inTransType == TXN_E_QR_VOID_ADD)
						bSendType = 'D';
				}
				else if(srTxnData.srIngData.inTransType == TXN_ECC_SALE_CARD_OFFLINE)
				{
					ulCardTypeID = 7;
					inRecLen = SIS2_TXN_BLOB_3_LEN;
				}
				else if(srTxnData.srIngData.inTransType == TXN_ECC_PENALTY)
				{
					ulCardTypeID = 8;
					inRecLen = SIS2_TXN_BLOB_4_LEN;
				}
				else if(srTxnData.srIngData.inTransType == TXN_ECC_REFUND_CARD)
				{
					ulCardTypeID = 5;
					inRecLen = SIS2_TXN_BLOB_8_LEN;
				}
				else
				{
					ulCardTypeID = 1;
					inRecLen = SIS2_TXN_BLOB_1_LEN;
					if(srTxnData.srIngData.inTransType == TXN_E_DEDUCT ||
					   srTxnData.srIngData.inTransType == TXN_E_QR_DEDUCT
					   )
						bSendType = 'D';
				}
				inSaveLen = inRecLen;
				inSaveMode = 1;
				fnPack((char *)srTxnData.srIngData.anSETM_STMC_1,strlen((char *)srTxnData.srIngData.anSETM_STMC_1),bSaveData);
				break;
			case 1://自動加值
				if(strlen((char *)srTxnData.srIngData.anSETM_STMC_2) == 0)
					break;

				inSaveMode = 1;
				
				fnPack((char *)srTxnData.srIngData.anSETM_STMC_2,strlen((char *)srTxnData.srIngData.anSETM_STMC_2),bSaveData);
				if (bSaveData[1] == 0x02)
				{
					ulCardTypeID = 2;
					inRecLen = SIS2_TXN_BLOB_2_LEN;
				}
				else
					inRecLen = SIS2_TXN_BLOB_1_LEN;

				inSaveLen = inRecLen; //SIS2_TXN_BLOB_1_LEN;
				break;
			case 2://加值重送
				if(strlen((char *)srTxnData.srIngData.anTAVR_TARC) == 0)
					break;

				inSaveMode = 2;
				
				fnPack((char *)srTxnData.srIngData.anTAVR_TARC,strlen((char *)srTxnData.srIngData.anTAVR_TARC),bSaveData);
				inRecLen = SIS2_ADD_VALUE_RESEND_LEN;
				inSaveLen = SIS2_ADD_VALUE_RESEND_LEN;
				break;
			case 3://鎖卡回饋
				if(strlen((char *)srTxnData.srIngData.anTLKR_TLRC) == 0)
					break;

				inSaveMode = 3;
				
				fnPack((char *)srTxnData.srIngData.anTLKR_TLRC,strlen((char *)srTxnData.srIngData.anTLKR_TLRC),bSaveData);
				inRecLen = SIS2_LOCK_LEN;
				inSaveLen = SIS2_LOCK_LEN;
				break;
			/*case 4://加值
				if(strlen((char *)srTxnData.srIngData.anSETM_STMC_1) == 0)
					break;

				inSaveMode = 4;
				inRecLen = SIS2_TXN_BLOB_2_LEN;
				fnPack((char *)srTxnData.srIngData.anSETM_STMC_1,strlen((char *)srTxnData.srIngData.anSETM_STMC_1),bSaveData);
				inSaveLen = SIS2_TXN_BLOB_2_LEN;
				break;*/
		}

		if(inSaveMode == 1)
		{
			SIS2_HEADER *tHeader = (SIS2_HEADER *)&bSaveData[0];
			//SIS2_BODY_DEDUCT *tBody = (SIS2_BODY_DEDUCT *)&bSaveData[sizeof(SIS2_HEADER)];
			//SIS2_END *tEnd = (SIS2_END *)&bSaveData[sizeof(SIS2_HEADER) + sizeof(SIS2_BODY_DEDUCT)];

			ulSPID = tHeader->ucSPID;
			memcpy((char *)&ulCPUSPID,tHeader->ucCPUSPID,sizeof(tHeader->ucCPUSPID));
			memcpy((char *)&ulCPULocID,tHeader->ucCPULocationID,sizeof(tHeader->ucCPULocationID));
			ulCPUDevNo = tHeader->ucCPUDeviceID[1] * 256 + tHeader->ucCPUDeviceID[0] ;
			//ulCPUDevNo = ((tHeader->ucDeviceID_3[2] & 0x0F) << 8) +tHeader->ucDeviceID_3[1] ;
		}
		else if(inSaveMode == 2)
		{
			SIS2_ADD_VALUE_RESEND *tBody = (SIS2_ADD_VALUE_RESEND *)bSaveData;

			ulCardTypeID = 12;
			ulSPID = tBody->ucReSPID;
			memcpy((char *)&ulCPUSPID,tBody->ucReCPUSPID,sizeof(tBody->ucReCPUSPID));
			memcpy((char *)&ulCPULocID,tBody->ucReCPULocationID,sizeof(tBody->ucReCPULocationID));
			ulCPUDevNo = tBody->ucReCPUDeviceID[1] * 256 + tBody->ucReCPUDeviceID[0] ;
			//ulCPUDevNo = ((tBody->ucReDeviceID[2] & 0x0F) << 8) +tBody->ucReDeviceID[1] ;
		}
		else if(inSaveMode == 3)
		{
			SIS2_LOCK *tBody = (SIS2_LOCK *)bSaveData;

			ulCardTypeID = 34;
			ulSPID = tBody->ucSPID;
			memcpy((char *)&ulCPUSPID,tBody->ucCPUSPID,sizeof(tBody->ucCPUSPID));
			memcpy((char *)&ulCPULocID,tBody->ucCPULocationID,sizeof(tBody->ucCPULocationID));
			ulCPUDevNo = tBody->ucCPUDeviceID[1] * 256 + tBody->ucCPUDeviceID[0] ;
			//ulCPUDevNo = ((tBody->ucDeviceID[2] & 0x0F) << 8) +tBody->ucDeviceID[1] ;
		}
		else
			continue;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
		sprintf(chSaveName,"ECCSIS2_BAK\\00220.%03ld_%08ld_%05ld_%05ld.YYYYMMDDhhmmss_%03ld%C.dat",ulSPID,ulCPUSPID,ulCPULocID,ulCPUDevNo,ulCardTypeID,bSendType);
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
		sprintf(chSaveName,"ECCSIS2_BAK/00220.%03ld_%08ld_%05ld_%05ld.YYYYMMDDhhmmss_%03ld%C.dat",ulSPID,ulCPUSPID,ulCPULocID,ulCPUDevNo,ulCardTypeID,bSendType);
#else
		sprintf(chSaveName,"00220.%03ld_%08ld_%05ld_%05ld.YYYYMMDDhhmmss_%03ld%C.dat",ulSPID,ulCPUSPID,ulCPULocID,ulCPUDevNo,ulCardTypeID,bSendType);
#endif

		inRetVal = inFileGetSize(chSaveName,FALSE);
		if(inRetVal <= SUCCESS)//無Advice資料,回傳成功!!
		{
			memset(bHeadData,0x00,sizeof(bHeadData));
			sprintf((char *)bHeadData,"HEAD%04ldDATA",(long)inRecLen);
			inFileAppend(chSaveName,bHeadData,12);
		}

		inRetVal = inFileAppend(chSaveName,bSaveData,inSaveLen);
		if(inRetVal < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inSIS2SaveBlob Fail 1:%d (%s)",inRetVal,chSaveName);
			return inRetVal;
		}
	}

	return inRetVal;
}

int inEZHostSaveData()
{
int inRetVal = 0;
char chSaveName[100];
//unsigned long ulSPID = 0L,ulCPUSPID = 0L,ulCPULocID = 0L,ulCPUDevNo = 0L,ulCardTypeID = 0L;
BYTE bSaveData[250],bHeadData[250];

	memset(chSaveName,0x00,sizeof(chSaveName));
	memset(bSaveData,0x00,sizeof(bSaveData));
	memset(bHeadData,0x00,sizeof(bHeadData));

	if(strlen((char *)srTxnData.srIngData.anSETM_STMC_1) == 0)
		return inRetVal;

	vdPad((char *)srTxnData.srIngData.anSETM_STMC_1,(char *)bSaveData,232,TRUE,0x20);
	bSaveData[232] = 0x0D;
	bSaveData[233] = 0x0A;
	bSaveData[234] = 0x00;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
	sprintf(chSaveName,"ECCSIS2_BAK\\TXET.99600000.YYYYMMDD00002");
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
	sprintf(chSaveName,"ECCSIS2_BAK/TXET.99600000.YYYYMMDD00002");
#else
	sprintf(chSaveName,"TXET.99600000.YYYYMMDD00002");
#endif
	vdReplace(srTxnData.srParameter.APIDefaultData.srAPIInData2.chLocationID,3,&chSaveName[strlen((char *)chSaveName)],' ','0');

	inRetVal = inFileGetSize(chSaveName,FALSE);
	if(inRetVal <= SUCCESS)//無Advice資料,回傳成功!!
	{
		memset(bHeadData,0x00,sizeof(bHeadData));
		sprintf((char *)bHeadData,"HTXET99600000");
		memcpy(&bHeadData[strlen((char *)bHeadData)],srTxnData.srIngData.chTxDate,8);
		memcpy(&bHeadData[strlen((char *)bHeadData)],srTxnData.srIngData.chTxTime,6);
		memset(&bHeadData[strlen((char *)bHeadData)],0x20,189);
		memset(&bHeadData[strlen((char *)bHeadData)],0x30,16);
		bHeadData[232] = 0x0D;
		bHeadData[233] = 0x0A;
		bHeadData[234] = 0x00;
		inFileAppend(chSaveName,bHeadData,234);
	}

	inRetVal = inFileAppend(chSaveName,bSaveData,234);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inEZHostSaveData Fail 1:%d (%s)",inRetVal,chSaveName);
		return inRetVal;
	}

	return inRetVal;
}

int inMakeSIS2Blob(void)
{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API//暫時不加 by Tim || READER_MANUFACTURERS==WINDOWS_CE_API
//int inRetVal = SUCCESS;
char   filter[200], *blcfile[30],bTxnType[5];
int	i= 0x00;
//int	j= 0x00;
//long   hFile= 0x00;
BYTE bFileEnd[40];
unsigned long ulFileLen = 0L;
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
	struct _finddata_t c_file;
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
	DIR *dir;
	struct dirent *ptr;
	regex_t re;
	char szTmp[100];
#endif

	memset(filter,0,sizeof(filter));
	strcpy( gBlcFileFolder, (char *)gCurrentFolder );
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
	strcat( gBlcFileFolder, "\\ECCSIS2_BAK" ); //dir folder
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
	strcat( gBlcFileFolder, "/ECCSIS2_BAK" ); //dir folder
#endif

	strcpy( filter, gBlcFileFolder );

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
	strcat( filter, "\\*YYYYMMDDhhmmss*.dat" ); //file type
	/*#if READER_MANUFACTURERS==WINDOWS_CE_API
		CreateDirectory((const unsigned short *)gBlcFileFolder, NULL);
	#else
		mkdir(gBlcFileFolder);
	#endif*/
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
	//strcat( filter, "/*YYYYMMDDhhmmss*.dat" ); //file type
	/*mkdir(gBlcFileFolder,0777);*/
#endif
	ECC_CreateDirectory_Lib(gBlcFileFolder);

	for (i=0;i<30;i++)
	{
		blcfile[i]=(char*)ECC_calloc(55,sizeof(char));
		if (blcfile[i]==NULL)
		{
			for (;i>0;i--)
				ECC_free((BYTE *)blcfile[i-1]);
			//fnWriteLog((BYTE *)"fnBLCProcedure  malloc FALSE",(BYTE *)&i,4);
			return DONGLE_MALLOC_ERROR;
		}
	}
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
	hFile = _findfirst(filter, &c_file); //search file
	if(hFile!=-1)
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API

	if(regcomp(&re,"[[:digit:]]{5}\\.[[:digit:]]{3}_[[:digit:]]{8}_[[:digit:]]{5}_[[:digit:]]{5}\\.YYYYMMDDHHMMSS_[[:digit:]]{3}U\\.DAT",REG_EXTENDED) != 0)
	{
		log_msg(LOG_LEVEL_FLOW,"inMakeSIS2Blob Fail 1");
		return ICER_ERROR;
	}

	dir = opendir(filter);
	if(dir != NULL)
#endif
	{
		i=0;
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
		do
		{
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
		while((ptr = readdir(dir)) != NULL)
		{
			memset(szTmp,0x00,sizeof(szTmp));
			vdToUpper(szTmp,ptr->d_name);
			if(regexec(&re,szTmp,0,0,0) != 0)
			{
				log_msg(LOG_LEVEL_FLOW,"inMakeSIS2Blob 1(%s)",szTmp);
				continue;
			}
			log_msg(LOG_LEVEL_FLOW,"inMakeSIS2Blob 2(%s)",szTmp);
#else
		{
#endif

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
			char chOldName[200],chNewName[200];

			memset(chOldName,0x00,sizeof(chOldName));
			memset(chNewName,0x00,sizeof(chNewName));
			memset(bTxnType,0x00,sizeof(bTxnType));

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
			memcpy(bTxnType,&c_file.name[46],3);
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
			memcpy(bTxnType,&ptr->d_name[46],3);
#endif

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
			memcpy(&blcfile[i][0],&c_file.name[0],31);
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
			memcpy(&blcfile[i][0],&ptr->d_name[0],31);
#endif

			memcpy(&blcfile[i][31],srTxnData.srIngData.chTxDate,8);
			memcpy(&blcfile[i][39],srTxnData.srIngData.chTxTime,6);

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
			memcpy(&blcfile[i][45],&c_file.name[45],9);
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
			memcpy(&blcfile[i][45],&ptr->d_name[45],9);
#endif
			blcfile[i][54] = 0x00;
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
			strcat(chOldName,"\\ECCSIS2_BAK\\");
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
			strcat(chOldName,"/ECCSIS2_BAK/");
#endif

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
			strcat(chOldName,c_file.name);
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
			strcat(chOldName,ptr->d_name);
#endif

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
			strcat(chNewName,"\\ECCSIS2\\");
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
			strcat(chNewName,"/ECCSIS2/");
#endif
			strcat(chNewName,blcfile[i]);
			inFileRename(chOldName,chNewName);

			memset(bFileEnd,0x00,sizeof(bFileEnd));
			ulFileLen = inFileGetSize(chNewName,FALSE);
			if(ulFileLen < 12)
				continue;

			ulFileLen -= 12;
			if(!memcmp(bTxnType,"001",3))
				ulFileLen /= SIS2_TXN_BLOB_1_LEN;
			else if(!memcmp(bTxnType,"002",3) || !memcmp(bTxnType,"011",3))
				ulFileLen /= SIS2_TXN_BLOB_2_LEN;
			else if(!memcmp(bTxnType,"003",3))
				ulFileLen /= SIS2_TXN_BLOB_5_LEN;
			else if(!memcmp(bTxnType,"005",3))
				ulFileLen /= SIS2_TXN_BLOB_8_LEN;
			else if(!memcmp(bTxnType,"006",3))
				ulFileLen /= SIS2_TXN_BLOB_7_LEN;
			else if(!memcmp(bTxnType,"007",3))
				ulFileLen /= SIS2_TXN_BLOB_3_LEN;
			else if(!memcmp(bTxnType,"008",3))
				ulFileLen /= SIS2_TXN_BLOB_4_LEN;
			else if(!memcmp(bTxnType,"010",3))
				ulFileLen /= SIS2_TXN_BLOB_6_LEN;
			else if(!memcmp(bTxnType,"012",3))
				ulFileLen /= SIS2_ADD_VALUE_RESEND_LEN;
			else if(!memcmp(bTxnType,"034",3))
				ulFileLen /= SIS2_LOCK_LEN;
			strcat((char *)bFileEnd,"TAIL");
			sprintf((char *)&bFileEnd[20],"%08ldEND",ulFileLen);
			inFileAppend(chNewName,bFileEnd,31);
#else

#warning "inMakeSIS2Blob Not Coding !!"

#endif

		}

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
		while( _findnext( hFile, &c_file )==0 );
		_findclose(hFile);//1.0.5.2
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
		closedir(dir);
#endif
	}

#if READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
	regfree(&re);
#endif

	for (i=0;i<30;i++)
		ECC_free((BYTE *)blcfile[i]);
#endif

	return SUCCESS;
}

int inMakeEZHostBatchData(void)
{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API//暫時不加 by Tim || READER_MANUFACTURERS==WINDOWS_CE_API
//int inRetVal = SUCCESS;
char   filter[200], *blcfile[30];
int	i= 0x00;
//int	j= 0x00;
long   hFile= 0x00;
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
struct _finddata_t c_file;
#endif
//BYTE bFileEnd[40];
//unsigned long ulFileLen = 0L;

	memset(filter,0,sizeof(filter));
	strcpy( gBlcFileFolder, (char *)gCurrentFolder );

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
	strcat( gBlcFileFolder, "\\ECCSIS2_BAK" ); //dir folder
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
	strcat( gBlcFileFolder, "/ECCSIS2_BAK" ); //dir folder
#endif

	strcpy( filter, gBlcFileFolder );
	strcat( filter, "\\TXET.99600000.YYYYMMDD01002*" ); //file type

#if 0
#if READER_MANUFACTURERS==WINDOWS_API
	mkdir(gBlcFileFolder);
#elif READER_MANUFACTURERS==WINDOWS_CE_API
	CreateDirectory((const unsigned short *)gBlcFileFolder, NULL);
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
	mkdir(gBlcFileFolder,0777);
#endif
#endif

	ECC_CreateDirectory_Lib(gBlcFileFolder);

	for (i=0;i<30;i++)
	{
		blcfile[i]=(char*)ECC_calloc(40,sizeof(char));
		if (blcfile[i]==NULL)
		{
			for (;i>0;i--)
				ECC_free((BYTE *)blcfile[i-1]);
			//fnWriteLog((BYTE *)"fnBLCProcedure  malloc FALSE",(BYTE *)&i,4);
			return DONGLE_MALLOC_ERROR;
		}
	}
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
	hFile = _findfirst(filter, &c_file); //search file
#else
#endif
	if(hFile!=-1)
	{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
		i=0;
		do
		{
			char chOldName[200],chNewName[200];

			memset(chOldName,0x00,sizeof(chOldName));
			memset(chNewName,0x00,sizeof(chNewName));
			memcpy(&blcfile[i][0],&c_file.name[0],strlen(c_file.name));
			memcpy(&blcfile[i][14],srTxnData.srIngData.chTxDate,8);
			blcfile[i][30] = 0x00;
			//memcpy(&blcfile[i][22],srTxnData.srIngData.chTxTime,6);
			strcat(chOldName,"\\ECCSIS2_BAK\\");
			strcat(chOldName,c_file.name);
			strcat(chNewName,"\\ECCSIS2\\");
			strcat(chNewName,blcfile[i]);
			inFileRename(chOldName,chNewName);

			memset(bFileEnd,0x00,sizeof(bFileEnd));
			ulFileLen = inFileGetSize(chNewName,FALSE);
			//if(ulFileLen % 234)
			//	continue;
			ulFileLen /= 234;

			sprintf((char *)&bFileEnd[0],"T%08ld",ulFileLen - 1);
			memset(&bFileEnd[9],0x20,207);
			memset(&bFileEnd[216],0x30,16);
			bFileEnd[232] = 0x0D;
			bFileEnd[233] = 0x0A;
			bFileEnd[234] = 0x00;
			inFileAppend((char *)chNewName,bFileEnd,234);
		}
		while( _findnext( hFile, &c_file )==0 );
		_findclose(hFile);//1.0.5.2
#else

#warning "inMakeSIS2Blob Not Coding !!"

#endif
	}

	for (i=0;i<30;i++)
		ECC_free((BYTE *)blcfile[i]);
#endif

	return SUCCESS;
}

int inBuildAuthTxnOfflineSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inCnt = 0;
BYTE bTmp[50],ucAgentNumber[5];
//AuthTxnOffline_APDU_In *AuthIn = (AuthTxnOffline_APDU_In *)Dongle_In;
AuthTxnOffline_APDU_Out *AuthOut = (AuthTxnOffline_APDU_Out *)Dongle_Out;
TxnReqOffline_APDU_Out *TxnReqOut = (TxnReqOffline_APDU_Out *)Dongle_ReqOut;
//TxnReqOffline_APDU_In *TxnReqIn = (TxnReqOffline_APDU_In)Dongle_ReqIn;

	memset(bTmp,0x00,sizeof(bTmp));
	memset(ucAgentNumber,0x00,sizeof(ucAgentNumber));
	//表頭
	inCnt += inBuildSIS2Header(&API_Out[inCnt],
								TxnReqOut->ucPurseVersionNumber,
								TxnReqOut->ucMsgType,
								(srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT)?TxnReqOut->ucSubType:0x00,
								TxnReqOut->ucDeviceID,
								TxnReqOut->ucSPID,
								AuthOut->ucTxnDateTime,
								TxnReqOut->ucCardID,
								TxnReqOut->ucIssuerCode,
								AuthOut->ucTxnSN[0],
								TxnReqOut->ucTxnAmt,
								AuthOut->ucEV,
								TxnReqOut->ucLocationID,
								TxnReqOut->ucCardIDLen,
								AuthOut->ucTxnSN,
								TxnReqOut->ucCPUDeviceID,
								TxnReqOut->ucCPUSPID,
								TxnReqOut->ucCPULocationID,
								0x00,
								TxnReqOut->ucPersonalProfile,
								TxnReqOut->ucPersonalProfile,
								0x02,
								TxnReqOut->ucPID,
								TxnReqOut->ucCTC,
								TxnReqOut->ucAreaCode,
								TxnReqOut->ucSubAreaCode,
								TxnReqOut->ucTxnSN,
								TxnReqOut->ucEV
								);

	fnASCTOBIN(ucAgentNumber,(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber,4,2,HEX);
	if(srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT)
	{	//一般扣款表身
		inCnt += inBuildSIS2Body_Deduct(&API_Out[inCnt],bTmp,
													    bTmp,
														bTmp[0],
														bTmp,
														bTmp,
														bTmp,
														bTmp,
														bTmp,
													   (TxnReqOut->ucPurseVersionNumber == 0x00)?TxnReqOut->ucCPD_SAMID:bTmp,
														ucAgentNumber,
														bTmp,
														bTmp,
														bTmp,
														bTmp,
														TxnReqOut->ucTxnMode,
														TxnReqOut->ucTxnQuqlifier,
														TxnReqOut->ucSignatureKeyKVN,
														AuthOut->ucSignature);
	}
	else
	{	//代收扣款表身
		inCnt += inBuildSIS2Body_FareSale(&API_Out[inCnt],(TxnReqOut->ucPurseVersionNumber == 0x00)?TxnReqOut->ucCPD_SAMID:bTmp,
														  ucAgentNumber,//ucAgentNumber
														  TxnReqOut->ucSubType,//ucFareProductType
														  bTmp[0],//ucGroupsize
														  bTmp,//ucEntryStation
														  bTmp,//ucExitStation
														  bTmp,//ucJourneyNumber
														  bTmp,//ucFirstDate
														  bTmp,//ucLastDate
														  bTmp,//ucCarNo
														  bTmp,//ucDuration
														  bTmp,//ucAuthoriseGroup
														  bTmp,//ucRFU
														  TxnReqOut->ucTxnMode,
														  TxnReqOut->ucTxnQuqlifier,
														  TxnReqOut->ucSignatureKeyKVN,
														  AuthOut->ucSignature);//ucSignature
	}

	//表尾
	inCnt += inBuildSIS2End(&API_Out[inCnt],TxnReqOut->ucPurseVersionNumber,
											(TxnReqOut->ucPurseVersionNumber == 0x00)?AuthOut->ucMAC_HCrypto:bTmp,
											 AuthOut->ucCPUSAMID,
											(TxnReqOut->ucPurseVersionNumber != 0x00)?AuthOut->ucMAC_HCrypto[0]:bTmp[0],
											(TxnReqOut->ucPurseVersionNumber != 0x00)?AuthOut->ucMAC_HCrypto[1]:bTmp[0],
											(TxnReqOut->ucPurseVersionNumber != 0x00)?&AuthOut->ucMAC_HCrypto[2]:bTmp,
											 TxnReqOut->ucDeviceID);

	//if(srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT2)
	{//加值重送
		BYTE bRecvData[SIS2_ADD_VALUE_RESEND_LEN + 1];

		memset(bRecvData,0x00,sizeof(bRecvData));
		memset(srTxnData.srIngData.anTAVR_TARC,0x00,sizeof(srTxnData.srIngData.anTAVR_TARC));
		inCnt += inBuildSIS2AddValueResend(bRecvData,TxnReqOut->stLastCreditTxnLogInfo_t.ucPurseVersionNumber_Index,
													 0x0C,
													 TxnReqOut->stLastCreditTxnLogInfo_t.ucSubType,
													 TxnReqOut->stLastCreditTxnLogInfo_t.ucCPUDeviceID,
													 TxnReqOut->stLastCreditTxnLogInfo_t.ucCPUSPID,
													 TxnReqOut->stLastCreditTxnLogInfo_t.ucTxnDateTime,
													 TxnReqOut->ucCardID,
													 TxnReqOut->ucIssuerCode,
													 TxnReqOut->stLastCreditTxnLogInfo_t.ucTxnSN,
													 TxnReqOut->stLastCreditTxnLogInfo_t.ucTxnAmt,
													 TxnReqOut->stLastCreditTxnLogInfo_t.ucEV,
													 TxnReqOut->stLastCreditTxnLogInfo_t.ucCPULocationID,
													 TxnReqOut->ucCardIDLen,
													 TxnReqOut->ucPersonalProfile,
													 TxnReqOut->ucPID,
													 TxnReqOut->ucBankCode_HostAdminKVN,
													 (TxnReqOut->ucPurseVersionNumber == 0x00)?TxnReqOut->ucCPD_SAMID:bTmp,
													 TxnReqOut->ucDeviceID,
													 TxnReqOut->ucSPID,
													 TxnReqOut->ucLocationID,
													 TxnReqOut->ucCPUDeviceID,
													 TxnReqOut->ucCPUSPID,
													 TxnReqOut->ucCPULocationID);
		fnUnPack(bRecvData,SIS2_ADD_VALUE_RESEND_LEN,srTxnData.srIngData.anTAVR_TARC);
	}

	vdUpdateCardInfo2(inTxnType,
					 TxnReqOut->ucCardID,
					 TxnReqOut->ucPID,
					 (BYTE )TxnReqOut->bAutoLoad,
					 TxnReqOut->ucCardType,
					 TxnReqOut->ucPersonalProfile,
					 TxnReqOut->ucAutoLoadAmt,
					 AuthOut->ucEV,
					 TxnReqOut->ucExpiryDate,
					 TxnReqOut->ucPurseVersionNumber,
					 (TxnReqOut->ucPurseVersionNumber == 0x00)?TxnReqOut->ucBankCode_HostAdminKVN:0x00,
					 TxnReqOut->ucAreaCode,
					 TxnReqOut->ucSubAreaCode,
					 TxnReqOut->ucDeposit,
					 AuthOut->ucTxnSN);

	return inCnt;
}

int inBuildLock1SIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inCnt = 0;
BYTE bTmp[20],bTmp2[20],ucTxnDateTime[4];
LockCard_APDU_Out_3 *DongleOut = (LockCard_APDU_Out_3 *)Dongle_Out;

	memset(bTmp,0x00,sizeof(bTmp));
	memset(ucTxnDateTime,0x00,sizeof(ucTxnDateTime));
	strcat((char *)bTmp,srTxnData.srIngData.chTxDate);
	strcat((char *)bTmp,srTxnData.srIngData.chTxTime);
	fngetUnixTimeCnt(ucTxnDateTime,bTmp);
	memset(bTmp,0x00,sizeof(bTmp));
	memset(bTmp2,0x20,sizeof(bTmp2));

	inCnt += inBuildSIS2Lock(&API_Out[inCnt],DongleOut->ucPurseVersionNumber,
											 0x22,
											 DongleOut->ucPersonalProfile,
											 DongleOut->ucDeviceID,
											 DongleOut->ucSPID,
											 ucTxnDateTime,
											 DongleOut->ucCardID,
											 DongleOut->ucIssuerCode,
											 DongleOut->ucLocationID,
											 DongleOut->ucCardIDLen,
											 DongleOut->ucCPUDeviceID,
											 DongleOut->ucCPUSPID,
											 DongleOut->ucCPULocationID,
											 0x00,
											 DongleOut->ucPersonalProfile,
											 DongleOut->ucPID,
											 bTmp,
											 DongleOut->ucStatusCode[1],
											 bTmp2,
											 0x20,
											 srTxnData.srParameter.bDeviceInfo,
											 DongleOut->ucEV,
											 DongleOut->ucTxnSN,
											 DongleOut->stLastCreditTxnLogInfo_t.ucTxnSN,
											 DongleOut->stLastCreditTxnLogInfo_t.ucTxnDateTime,
											 DongleOut->stLastCreditTxnLogInfo_t.ucSubType,
											 DongleOut->stLastCreditTxnLogInfo_t.ucTxnAmt,
											 DongleOut->stLastCreditTxnLogInfo_t.ucEV,
											 DongleOut->stLastCreditTxnLogInfo_t.ucCPUDeviceID,
											 DongleOut->ucEV2,
											 DongleOut->ucMifareSettingParameter,
											 DongleOut->ucCPUSettingParameter,
											 bTmp);

	return inCnt;
}

int inBuildLock2SIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inCnt = 0;
BYTE bTmp[20],bTmp2[20],ucTxnDateTime[4],ucLockReason = 0;
LockCard_APDU_Out_2 *DongleOut = (LockCard_APDU_Out_2 *)Dongle_Out;
BYTE gsBlackListVersion[30];

	memset(bTmp,0x00,sizeof(bTmp));
	memset(ucTxnDateTime,0x00,sizeof(ucTxnDateTime));
	strcat((char *)bTmp,srTxnData.srIngData.chTxDate);
	strcat((char *)bTmp,srTxnData.srIngData.chTxTime);
	fngetUnixTimeCnt(ucTxnDateTime,bTmp);
	memset(bTmp,0x00,sizeof(bTmp));
	memset(bTmp2,0x20,sizeof(bTmp2));
	memset(gsBlackListVersion,0x20,sizeof(gsBlackListVersion));

	if(DongleOut->ucStatusCode[1] == 0x06)//黑名單鎖卡
	{
		ucLockReason = 0x01;
		inGetBLCName(gsBlackListVersion);
	}
	else if (DongleOut->ucStatusCode[1]== 0x03)//6103
		ucLockReason = 0x02;
	else if(DongleOut->ucStatusCode[1] == 0x00)//連線鎖卡
		ucLockReason = 0x03;
	inCnt += inBuildSIS2Lock(&API_Out[inCnt],DongleOut->ucPurseVersionNumber,
											 0x22,
											 DongleOut->ucPersonalProfile,
											 DongleOut->ucDeviceID,
											 DongleOut->ucSPID,
											 ucTxnDateTime,
											 DongleOut->ucCardID,
											 DongleOut->ucIssuerCode,
											 DongleOut->ucLocationID,
											 DongleOut->ucCardIDLen,
											 DongleOut->ucCPUDeviceID,
											 DongleOut->ucCPUSPID,
											 DongleOut->ucCPULocationID,
											 0x00,
											 DongleOut->ucPersonalProfile,
											 DongleOut->ucPID,
											 bTmp,
											 ucLockReason,
											 gsBlackListVersion,
											 (DongleOut->ucStatusCode[1] == 0x06)?'M':' ',
											 srTxnData.srParameter.bDeviceInfo,
											 bTmp,
											 bTmp,
											 bTmp,
											 bTmp,
											 0x00,
											 bTmp,
											 bTmp,
											 bTmp,
											 bTmp,
											 0x00,
											 0x00,
											 bTmp);

	return inCnt;
}

int inBuildTxnOfflineSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *NotUse)
{
int inCnt = 0;
BYTE bTmp[50],ucAgentNumber[5],bSPID[5];
//TxnReqOffline_APDU_In *DongleIn = (TxnReqOffline_APDU_In *)Dongle_In;
TxnReqOffline_APDU_Out *DongleOut = (TxnReqOffline_APDU_Out *)Dongle_Out;
//TxnReqOffline_APDU_Out *TxnReqOut = (TxnReqOffline_APDU_Out *)Dongle_ReqOut;

	memset(bTmp,0x00,sizeof(bTmp));
	memset(ucAgentNumber,0x00,sizeof(ucAgentNumber));
	//表頭
	inCnt += inBuildSIS2Header(&API_Out[inCnt],
								DongleOut->ucPurseVersionNumber,
								DongleOut->ucMsgType,
								DongleOut->ucSubType,
								DongleOut->ucDeviceID,
								DongleOut->ucSPID,
								DongleOut->ucTxnDateTime,
								DongleOut->ucCardID,
								DongleOut->ucIssuerCode,
								DongleOut->ucTxnSN2[0],
								DongleOut->ucTxnAmt,
								DongleOut->ucEV2,
								DongleOut->ucLocationID,
								DongleOut->ucCardIDLen,
								DongleOut->ucTxnSN2,
								DongleOut->ucCPUDeviceID,
								DongleOut->ucCPUSPID,
								DongleOut->ucCPULocationID,
								DongleOut->ucSubType,
								//0x00,
								DongleOut->ucPersonalProfile,
								DongleOut->ucPersonalProfile,
								0x02,
								DongleOut->ucPID,
								DongleOut->ucCTC,
								DongleOut->ucAreaCode,
								DongleOut->ucSubAreaCode,
								DongleOut->ucTxnSN,
								DongleOut->ucEV
								);

	fnASCTOBIN(ucAgentNumber,(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber,4,2,HEX);
	memset(bSPID,0x00,sizeof(bSPID));
	if(DongleOut->ucPurseVersionNumber == 0x00)
		memcpy(bSPID,&DongleOut->ucSPID,1);
	else
		memcpy(bSPID,DongleOut->ucCPUSPID,sizeof(DongleOut->ucCPUSPID));
	//表身
	inCnt += inBuildSIS2Body_Add(&API_Out[inCnt],  (DongleOut->ucPurseVersionNumber == 0x00)?DongleOut->ucCPD_SAMID:bTmp,
													ucAgentNumber,
													DongleOut->ucBankCode_HostAdminKVN,
													bSPID,
													bTmp,
													DongleOut->ucTxnMode,
													DongleOut->ucTxnQuqlifier,
													DongleOut->ucSignatureKeyKVN,
													DongleOut->ucSignature);

	//表尾
	inCnt += inBuildSIS2End(&API_Out[inCnt],DongleOut->ucPurseVersionNumber,
											(DongleOut->ucPurseVersionNumber == 0x00)?DongleOut->ucMAC_HCrypto:bTmp,
											 DongleOut->ucCPUSAMID,
											(DongleOut->ucPurseVersionNumber != 0x00)?DongleOut->ucMAC_HCrypto[0]:bTmp[0],
											(DongleOut->ucPurseVersionNumber != 0x00)?DongleOut->ucMAC_HCrypto[1]:bTmp[0],
											(DongleOut->ucPurseVersionNumber != 0x00)?&DongleOut->ucMAC_HCrypto[2]:bTmp,
											 DongleOut->ucDeviceID);

	//加值重送
	/*inCnt += inBuildSIS2AddValueResend(&API_Out[inCnt], TxnReqOut->stLastCreditTxnLogInfo_t.ucPurseVersionNumber_Index,
														0x0C,
														TxnReqOut->stLastCreditTxnLogInfo_t.ucSubType,
														TxnReqOut->stLastCreditTxnLogInfo_t.ucCPUDeviceID,
														TxnReqOut->stLastCreditTxnLogInfo_t.ucCPUSPID,
														TxnReqOut->stLastCreditTxnLogInfo_t.ucTxnDateTime,
														TxnReqOut->ucCardID,
														TxnReqOut->ucIssuerCode,
														TxnReqOut->stLastCreditTxnLogInfo_t.ucTxnSN,
														TxnReqOut->stLastCreditTxnLogInfo_t.ucTxnAmt,
														TxnReqOut->stLastCreditTxnLogInfo_t.ucEV,
														TxnReqOut->stLastCreditTxnLogInfo_t.ucCPULocationID,
														TxnReqOut->ucCardIDLen,
														TxnReqOut->ucPersonalProfile,
														TxnReqOut->ucPID,
														TxnReqOut->ucBankCode_HostAdminKVN,
													   (TxnReqOut->ucPurseVersionNumber == 0x00)?TxnReqOut->ucCPD_SAMID:bTmp,
														TxnReqOut->ucDeviceID,
														TxnReqOut->ucSPID,
														TxnReqOut->ucLocationID,
														TxnReqOut->ucCPUDeviceID,
														TxnReqOut->ucCPUSPID,
														TxnReqOut->ucCPULocationID);*/

	vdUpdateCardInfo2(inTxnType,
					 DongleOut->ucCardID,
					 DongleOut->ucPID,
					 (BYTE )DongleOut->bAutoLoad,
					 DongleOut->ucCardType,
					 DongleOut->ucPersonalProfile,
					 DongleOut->ucAutoLoadAmt,
					 DongleOut->ucEV2,
					 DongleOut->ucExpiryDate,
					 DongleOut->ucPurseVersionNumber,
					 (DongleOut->ucPurseVersionNumber == 0x00)?DongleOut->ucBankCode_HostAdminKVN:0x00,
					 DongleOut->ucAreaCode,
					 DongleOut->ucSubAreaCode,
					 DongleOut->ucDeposit,
					 DongleOut->ucTxnSN);

	return inCnt;
}

