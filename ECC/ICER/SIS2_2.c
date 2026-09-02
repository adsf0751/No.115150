
#ifdef EZ_AIRLINKEDC
#include "_stdAfx.h"
#else
#include "stdAfx.h"
#endif
//#include "Global.h"

extern BYTE gsBlackListVersion[30];
extern Store_Data stStore_DataInfo_t;
extern char gBlcFileFolder[100];

int inBuildSIS2Body_FareSale(BYTE *bOut_Data,BYTE *ucLoyaltyCounter,
										  BYTE *ucAgentNumber,
										  BYTE ucFareProductType,
										  BYTE ucGroupsize,
										  BYTE *ucEntryStation,
										  BYTE *ucExitStation,
										  BYTE *ucJourneyNumber,
										  BYTE *ucFirstDate,
										  BYTE *ucLastDate,
										  BYTE *ucCarNo,
										  BYTE *ucDuration,
										  BYTE *ucAuthoriseGroup,
										  BYTE *ucRFU,
										  BYTE ucTxnMode,
										  BYTE ucTxnQuqlifier,
										  BYTE ucSignatureKeyKVN,
										  BYTE *ucSignature)
{
SIS2_BODY_FARE_SALE *bOutData = (SIS2_BODY_FARE_SALE *)bOut_Data;

	memcpy(bOutData->ucLoyaltyCounter,ucLoyaltyCounter,sizeof(bOutData->ucLoyaltyCounter));
	memcpy(bOutData->ucAgentNumber,ucAgentNumber,sizeof(bOutData->ucAgentNumber));
	bOutData->ucFareProductType = ucFareProductType;
	bOutData->ucGroupsize = ucGroupsize;
	memcpy(bOutData->ucEntryStation,ucEntryStation,sizeof(bOutData->ucEntryStation));
	memcpy(bOutData->ucExitStation,ucExitStation,sizeof(bOutData->ucExitStation));
	memcpy(bOutData->ucJourneyNumber,ucJourneyNumber,sizeof(bOutData->ucJourneyNumber));
	memcpy(bOutData->ucFirstDate,ucFirstDate,sizeof(bOutData->ucFirstDate));
	memcpy(bOutData->ucLastDate,ucLastDate,sizeof(bOutData->ucLastDate));
	memcpy(bOutData->ucCarNo,ucCarNo,sizeof(bOutData->ucCarNo));
	memcpy(bOutData->ucDuration,ucDuration,sizeof(bOutData->ucDuration));
	memcpy(bOutData->ucAuthoriseGroup,ucAuthoriseGroup,sizeof(bOutData->ucAuthoriseGroup));
	memcpy(bOutData->ucRFU,ucRFU,sizeof(bOutData->ucRFU));
	bOutData->ucTxnMode = ucTxnMode;
	bOutData->ucTxnQuqlifier = ucTxnQuqlifier;
	bOutData->ucSignatureKeyKVN = ucSignatureKeyKVN;
	memcpy(bOutData->ucSignature,ucSignature,sizeof(bOutData->ucSignature));

	return sizeof(SIS2_BODY_FARE_SALE);
}

int inBuildSIS2Body_VoidFare(BYTE *bOut_Data,BYTE *ucLoyaltyCounter,
										  BYTE *ucAgentNumber,
										  BYTE ucFareProductType,
										  BYTE *ucRFU,
										  BYTE ucTxnMode,
										  BYTE ucTxnQuqlifier,
										  BYTE ucSignatureKeyKVN,
										  BYTE *ucSignature)
{
SIS2_BODY_VOID_FARE *bOutData = (SIS2_BODY_VOID_FARE *)bOut_Data;

	memcpy(bOutData->ucLoyaltyCounter,ucLoyaltyCounter,sizeof(bOutData->ucLoyaltyCounter));
	memcpy(bOutData->ucAgentNumber,ucAgentNumber,sizeof(bOutData->ucAgentNumber));
	bOutData->ucFareProductType = ucFareProductType;
	memcpy(bOutData->ucRFU,ucRFU,sizeof(bOutData->ucRFU));
	bOutData->ucTxnMode = ucTxnMode;
	bOutData->ucTxnQuqlifier = ucTxnQuqlifier;
	bOutData->ucSignatureKeyKVN = ucSignatureKeyKVN;
	memcpy(bOutData->ucSignature,ucSignature,sizeof(bOutData->ucSignature));

	return sizeof(SIS2_BODY_VOID_FARE);
}

int inBuildSIS2Body_RefundFare(BYTE *bOut_Data,BYTE *ucLoyaltyCounter,
										  BYTE ucFareProductType,
										  BYTE *ucRFU,
										  BYTE ucTxnMode,
										  BYTE ucTxnQuqlifier,
										  BYTE ucSignatureKeyKVN,
										  BYTE *ucSignature)
{
SIS2_BODY_REFUND_FARE *bOutData = (SIS2_BODY_REFUND_FARE *)bOut_Data;

	memcpy(bOutData->ucLoyaltyCounter,ucLoyaltyCounter,sizeof(bOutData->ucLoyaltyCounter));
	bOutData->ucFareProductType = ucFareProductType;
	memcpy(bOutData->ucRFU,ucRFU,sizeof(bOutData->ucRFU));
	bOutData->ucTxnMode = ucTxnMode;
	bOutData->ucTxnQuqlifier = ucTxnQuqlifier;
	bOutData->ucSignatureKeyKVN = ucSignatureKeyKVN;
	memcpy(bOutData->ucSignature,ucSignature,sizeof(bOutData->ucSignature));

	return sizeof(SIS2_BODY_VOID_FARE);
}

int inBuildFareSaleSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inCnt = 0;
#if 0
BYTE bTmp[50],ucAgentNumber[5];
FareSale_APDU_In *DongleIn = (FareSale_APDU_In *)Dongle_In;
FareSale_APDU_Out *DongleOut = (FareSale_APDU_Out *)Dongle_Out;
ReadCardBasicData_APDU_Out *TxnReqOut = (ReadCardBasicData_APDU_Out *)Dongle_ReqOut;
STRUCT_TMP_DATA srTmpData;
BYTE ucCardNumber[10],ucTXN_Value[5],ucTXN_RemainValue[5],ucFirstDate[5],ucLastDate[5],ucEntryStation[5],ucEntryExitFlag[5],ucEntryZoneNumber[5],ucMAC[15];

	memset(bTmp,0x00,sizeof(bTmp));
	memset(ucCardNumber,0x00,sizeof(ucCardNumber));
	memset(ucTXN_Value,0x00,sizeof(ucTXN_Value));
	memset(ucTXN_RemainValue,0x00,sizeof(ucTXN_RemainValue));
	memset(ucAgentNumber,0x00,sizeof(ucAgentNumber));
	memset(ucFirstDate,0x00,sizeof(ucFirstDate));
	memset(ucLastDate,0x00,sizeof(ucLastDate));
	memset(ucEntryStation,0x00,sizeof(ucEntryStation));
	memset(ucEntryExitFlag,0x00,sizeof(ucEntryExitFlag));
	memset(ucEntryZoneNumber,0x00,sizeof(ucEntryZoneNumber));
	memset(ucMAC,0x00,sizeof(ucMAC));
	inGetDeviceID(srTxnData.srIngData.anDeviceID,&srTmpData);
	memcpy(ucCardNumber,DongleOut->ucCardNumber,sizeof(DongleOut->ucCardNumber));
	vdAmt2ByteTo3Byte(DongleOut->ucTXN_Value,ucTXN_Value);
	vdAmt2ByteTo3Byte(DongleOut->ucTXN_RemainValue,ucTXN_RemainValue);
	//表頭
	inCnt += inBuildSIS2Header(&API_Out[inCnt],
								TxnReqOut->ucPurseVersionNumber,
								DongleOut->ucMsgType,
								0x00,
								srTmpData.ucDeviceID,
								srTmpData.ucDeviceID[2],
								DongleOut->ucTXNDateTime,
								ucCardNumber,
								DongleOut->ucIssueCompany,
								DongleOut->ucTXN_SN_LSB,
								ucTXN_Value,
								ucTXN_RemainValue,
								srTmpData.ucLocationID,
								0x04,
								TxnReqOut->ucTxnSN,
								srTmpData.ucCPUDeviceID,
								srTmpData.ucCPUSPID,
								srTmpData.ucCPULocationID,
								0x00,
								TxnReqOut->ucPersonalProfile,
								TxnReqOut->ucPersonalProfile,
								0x02,
								TxnReqOut->ucPID,
								bTmp,
								DongleOut->ucAreaCode,
								TxnReqOut->ucSubAreaCode,
								TxnReqOut->ucTxnSN,
								TxnReqOut->ucEV
								);

	fnASCTOBIN(ucAgentNumber,(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber,4,2,HEX);
	//表身
	ucFirstDate[0] = DongleOut->ucAreaCode;
	ucFirstDate[1] = DongleOut->ucTransferDiscount[0];
	ucLastDate[0] = DongleOut->ucTransferDiscount[1];
	ucLastDate[1] = DongleOut->ucPersonalDiscount[0];
	ucEntryStation[0] = DongleOut->ucGroupCode;
	ucEntryExitFlag[0] = DongleOut->ucEntryExitFlag;
	ucEntryZoneNumber[0] = DongleOut->ucEntryZoneNumber;
	inCnt += inBuildSIS2Body_FareSale(&API_Out[inCnt],DongleOut->ucLoyaltyPoint,
													  DongleOut->ucLineNumber,//ucAgentNumber
													  DongleOut->ucPersonalProfile,//ucFareProductType
													  DongleOut->ucCurrentZoneNumber,//ucGroupsize
													  ucEntryStation,//ucEntryStation
													  ucEntryExitFlag,//ucExitStation
													  ucEntryZoneNumber,//ucJourneyNumber
													  ucFirstDate,//ucFirstDate
													  ucLastDate,//ucLastDate
													  bTmp,//ucCarNo
													  bTmp,//ucDuration
													  bTmp,//ucAuthoriseGroup
													  bTmp,//ucRFU
													  0x00,//ucTxnMode
													  0x00,//ucTxnQuqlifier
													  0x00,//ucSignatureKeyKVN
													  bTmp);//ucSignature

	//表尾
	ucMAC[0] = 0x00;
	ucMAC[1] = 0x01;
	memcpy(&ucMAC[2],DongleOut->ucMAC,sizeof(DongleOut->ucMAC));
	inCnt += inBuildSIS2End(&API_Out[inCnt],ucMAC,
											bTmp,
											bTmp);

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
					 ucCardNumber,
					 bTmp,
					 (BYTE )TxnReqOut->bAutoLoad,
					 TxnReqOut->ucCardType,
					 TxnReqOut->ucPersonalProfile,
					 TxnReqOut->ucAutoLoadAmt,
					 ucTXN_RemainValue,
					 TxnReqOut->ucExpiryDate,
					 TxnReqOut->ucPurseVersionNumber,
					 TxnReqOut->ucBankCode,
					 TxnReqOut->ucAreaCode,
					 TxnReqOut->ucSubAreaCode,
					 TxnReqOut->ucDeposit,
					 TxnReqOut->ucTxnSN);
#endif

	return inCnt;
}

int inBuildVoidFareSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inCnt = 0;
#if 0
BYTE bTmp[50];
VoidFare_APDU_In *DongleIn = (VoidFare_APDU_In *)Dongle_In;
VoidFare_APDU_Out *DongleOut = (VoidFare_APDU_Out *)Dongle_Out;
ReadCardBasicData_APDU_Out *TxnReqOut = (ReadCardBasicData_APDU_Out *)Dongle_ReqOut;
STRUCT_TMP_DATA srTmpData;
BYTE ucCardNumber[10],ucTXN_Value[5],ucTXN_RemainValue[5],ucFirstDate[5],ucLastDate[5],ucEntryStation[5],ucEntryExitFlag[5],ucEntryZoneNumber[5],ucMAC[15];

	memset(bTmp,0x00,sizeof(bTmp));
	memset(ucCardNumber,0x00,sizeof(ucCardNumber));
	memset(ucTXN_Value,0x00,sizeof(ucTXN_Value));
	memset(ucTXN_RemainValue,0x00,sizeof(ucTXN_RemainValue));
	memset(ucFirstDate,0x00,sizeof(ucFirstDate));
	memset(ucLastDate,0x00,sizeof(ucLastDate));
	memset(ucEntryStation,0x00,sizeof(ucEntryStation));
	memset(ucEntryExitFlag,0x00,sizeof(ucEntryExitFlag));
	memset(ucEntryZoneNumber,0x00,sizeof(ucEntryZoneNumber));
	memset(ucMAC,0x00,sizeof(ucMAC));
	inGetDeviceID(srTxnData.srIngData.anDeviceID,&srTmpData);
	memcpy(ucCardNumber,DongleOut->ucCardNumber,sizeof(DongleOut->ucCardNumber));
	vdAmt2ByteTo3Byte(DongleOut->ucTXN_Value,ucTXN_Value);
	vdAmt2ByteTo3Byte(DongleOut->ucTXN_RemainValue,ucTXN_RemainValue);
	//表頭
	inCnt += inBuildSIS2Header(&API_Out[inCnt],
								TxnReqOut->ucPurseVersionNumber,
								DongleOut->ucMsgType,
								0x00,
								srTmpData.ucDeviceID,
								srTmpData.ucDeviceID[2],
								DongleOut->ucTXNDateTime,
								ucCardNumber,
								DongleOut->ucIssueCompany,
								DongleOut->ucTXN_SN_LSB,
								ucTXN_Value,
								ucTXN_RemainValue,
								srTmpData.ucLocationID,
								0x04,
								TxnReqOut->ucTxnSN,
								srTmpData.ucCPUDeviceID,
								srTmpData.ucCPUSPID,
								srTmpData.ucCPULocationID,
								0x00,
								TxnReqOut->ucPersonalProfile,
								TxnReqOut->ucPersonalProfile,
								0x02,
								TxnReqOut->ucPID,
								bTmp,
								TxnReqOut->ucAreaCode,
								TxnReqOut->ucSubAreaCode,
								TxnReqOut->ucTxnSN,
								TxnReqOut->ucEV
								);

	//表身
	inCnt += inBuildSIS2Body_VoidFare(&API_Out[inCnt],DongleOut->ucDeposit,
													  DongleOut->ucAgentNumber,//ucAgentNumber
													  DongleOut->ucSubType,//ucFareProductType
													  bTmp,//ucRFU
													  0x00,//ucTxnMode
													  0x00,//ucTxnQuqlifier
													  0x00,//ucSignatureKeyKVN
													  bTmp);//ucSignature

	//表尾
	ucMAC[0] = 0x00;
	ucMAC[1] = 0x01;
	memcpy(&ucMAC[2],DongleOut->ucMAC,sizeof(DongleOut->ucMAC));
	inCnt += inBuildSIS2End(&API_Out[inCnt],ucMAC,
											bTmp,
											bTmp);

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
					 ucCardNumber,
					 bTmp,
					 (BYTE )TxnReqOut->bAutoLoad,
					 TxnReqOut->ucCardType,
					 TxnReqOut->ucPersonalProfile,
					 TxnReqOut->ucAutoLoadAmt,
					 ucTXN_RemainValue,
					 TxnReqOut->ucExpiryDate,
					 TxnReqOut->ucPurseVersionNumber,
					 TxnReqOut->ucBankCode,
					 TxnReqOut->ucAreaCode,
					 TxnReqOut->ucSubAreaCode,
					 TxnReqOut->ucDeposit,
					 TxnReqOut->ucTxnSN);
#endif

	return inCnt;
}

int inBuildRefundFareSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inCnt = 0;
#if 0
BYTE bTmp[50];
RefundFare_APDU_In *DongleIn = (RefundFare_APDU_In *)Dongle_In;
RefundFare_APDU_Out *DongleOut = (RefundFare_APDU_Out *)Dongle_Out;
ReadCardBasicData_APDU_Out *TxnReqOut = (ReadCardBasicData_APDU_Out *)Dongle_ReqOut;
STRUCT_TMP_DATA srTmpData;
BYTE ucCardNumber[10],ucTXN_Value[5],ucTXN_RemainValue[5],ucFirstDate[5],ucLastDate[5],ucEntryStation[5],ucEntryExitFlag[5],ucEntryZoneNumber[5],ucMAC[15];

	memset(bTmp,0x00,sizeof(bTmp));
	memset(ucCardNumber,0x00,sizeof(ucCardNumber));
	memset(ucTXN_Value,0x00,sizeof(ucTXN_Value));
	memset(ucTXN_RemainValue,0x00,sizeof(ucTXN_RemainValue));
	memset(ucFirstDate,0x00,sizeof(ucFirstDate));
	memset(ucLastDate,0x00,sizeof(ucLastDate));
	memset(ucEntryStation,0x00,sizeof(ucEntryStation));
	memset(ucEntryExitFlag,0x00,sizeof(ucEntryExitFlag));
	memset(ucEntryZoneNumber,0x00,sizeof(ucEntryZoneNumber));
	memset(ucMAC,0x00,sizeof(ucMAC));
	inGetDeviceID(srTxnData.srIngData.anDeviceID,&srTmpData);
	memcpy(ucCardNumber,DongleOut->ucCardNumber,sizeof(DongleOut->ucCardNumber));
	vdAmt2ByteTo3Byte(DongleOut->ucTXN_Value,ucTXN_Value);
	vdAmt2ByteTo3Byte(DongleOut->ucTXN_RemainValue,ucTXN_RemainValue);
	//表頭
	inCnt += inBuildSIS2Header(&API_Out[inCnt],
								TxnReqOut->ucPurseVersionNumber,
								DongleOut->ucMsgType,
								0x00,
								srTmpData.ucDeviceID,
								srTmpData.ucDeviceID[2],
								DongleOut->ucTXNDateTime,
								ucCardNumber,
								DongleOut->ucIssueCompany,
								DongleOut->ucTXN_SN_LSB,
								ucTXN_Value,
								ucTXN_RemainValue,
								srTmpData.ucLocationID,
								0x04,
								TxnReqOut->ucTxnSN,
								srTmpData.ucCPUDeviceID,
								srTmpData.ucCPUSPID,
								srTmpData.ucCPULocationID,
								0x00,
								TxnReqOut->ucPersonalProfile,
								TxnReqOut->ucPersonalProfile,
								0x02,
								TxnReqOut->ucPID,
								bTmp,
								TxnReqOut->ucAreaCode,
								TxnReqOut->ucSubAreaCode,
								TxnReqOut->ucTxnSN,
								TxnReqOut->ucEV
								);

	//表身
	inCnt += inBuildSIS2Body_RefundFare(&API_Out[inCnt],DongleOut->ucLoyaltyPoint,
														DongleOut->ucFareProductType,//ucFareProductType
														bTmp,//ucRFU
														0x00,//ucTxnMode
														0x00,//ucTxnQuqlifier
														0x00,//ucSignatureKeyKVN
														bTmp);//ucSignature

	//表尾
	ucMAC[0] = 0x00;
	ucMAC[1] = 0x01;
	memcpy(&ucMAC[2],DongleOut->ucMAC,sizeof(DongleOut->ucMAC));
	inCnt += inBuildSIS2End(&API_Out[inCnt],ucMAC,
											bTmp,
											bTmp);

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
					 ucCardNumber,
					 bTmp,
					 (BYTE )TxnReqOut->bAutoLoad,
					 TxnReqOut->ucCardType,
					 TxnReqOut->ucPersonalProfile,
					 TxnReqOut->ucAutoLoadAmt,
					 ucTXN_RemainValue,
					 TxnReqOut->ucExpiryDate,
					 TxnReqOut->ucPurseVersionNumber,
					 TxnReqOut->ucBankCode,
					 TxnReqOut->ucAreaCode,
					 TxnReqOut->ucSubAreaCode,
					 TxnReqOut->ucDeposit,
					 TxnReqOut->ucTxnSN);
#endif

	return inCnt;
}

int inBuildTRTCSetValueSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inCnt = 0;
//TRTCSetValue_APDU_In *DongleIn = (TRTCSetValue_APDU_In *)Dongle_In;
TRTCRead_APDU_Out *TxnReqOut = (TRTCRead_APDU_Out *)Dongle_ReqOut;

	vdUpdateCardInfo2(inTxnType,
					 TxnReqOut->ucCardID,
					 TxnReqOut->ucPID,
					 (BYTE )TxnReqOut->bAutoLoad,
					 TxnReqOut->ucCardType,
					 TxnReqOut->ucPersonalProfile,
					 TxnReqOut->ucAutoLoadAmt,
					 TxnReqOut->ucEV,
					 TxnReqOut->ucExpiryDate,
					 TxnReqOut->ucPurseVersionNumber,
					 TxnReqOut->ucBankCode,
					 TxnReqOut->ucAreaCode,
					 TxnReqOut->ucSubAreaCode,
					 (BYTE *)"\x00\x00\x00",
					 TxnReqOut->ucTxnSN);

	return inCnt;
}

int inBuildSetValueOfflineSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inCnt = 0;
//SetValueOffline_APDU_In *DongleIn = (SetValueOffline_APDU_In *)Dongle_In;
SetValueOffline_APDU_Out *DongleOut = (SetValueOffline_APDU_Out *)Dongle_Out;

	vdUpdateCardInfo2(inTxnType,
					 DongleOut->ucCardID,
					 DongleOut->ucPID,
					 (BYTE )DongleOut->bAutoLoad,
					 DongleOut->ucCardType,
					 DongleOut->ucPersonalProfile,
					 DongleOut->ucAutoLoadAmt,
					 DongleOut->ucEV,
					 DongleOut->ucExpiryDate,
					 DongleOut->ucPurseVersionNumber,
					 0x00,
					 DongleOut->ucAreaCode,
					 (DongleOut->ucPurseVersionNumber != 0)?DongleOut->ucSubAreaCode_LoyaltyCounter:(BYTE *)"\x00\x00",
					 DongleOut->ucDeposit,
					 DongleOut->ucTxnSN);

	return inCnt;
}

int inBuildAuthTxnOnlineSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inCnt = 0;
BYTE bTmp[60],ucAgentNumber[5],bSPID[5];
//AuthTxnOnline_APDU_In *AuthIn = (AuthTxnOnline_APDU_In *)Dongle_In;
AuthTxnOnline_APDU_Out *AuthOut = (AuthTxnOnline_APDU_Out *)Dongle_Out;
TxnReqOnline_APDU_Out *TxnReqOut = (TxnReqOnline_APDU_Out *)Dongle_ReqOut;

	memset(bTmp,0x00,sizeof(bTmp));
	memset(ucAgentNumber,0x00,sizeof(ucAgentNumber));
	//表頭
	inCnt += inBuildSIS2Header(&API_Out[inCnt],
								TxnReqOut->ucPurseVersionNumber,
								TxnReqOut->ucMsgType,
								TxnReqOut->ucSubType,
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
								TxnReqOut->ucSubType,
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

	//退卡表身
	fnASCTOBIN(ucAgentNumber,(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber,4,2,HEX);
	memset(bSPID,0x00,sizeof(bSPID));
	if(TxnReqOut->ucPurseVersionNumber == 0x00)
		memcpy(bSPID,&TxnReqOut->ucSPID,1);
	else
		memcpy(bSPID,TxnReqOut->ucCPUSPID,sizeof(TxnReqOut->ucCPUSPID));
	inCnt += inBuildSIS2Body_Add(&API_Out[inCnt],TxnReqOut->ucLoyaltyCounter,
												 ucAgentNumber,
												 TxnReqOut->ucBankCode,
												 bSPID,
												 bTmp,
												 TxnReqOut->ucTxnMode,
												 TxnReqOut->ucTxnQuqlifier,
												 TxnReqOut->ucSignatureKeyKVN,
												 AuthOut->ucSignature);

	//表尾
	inCnt += inBuildSIS2End(&API_Out[inCnt],TxnReqOut->ucPurseVersionNumber,
											(TxnReqOut->ucPurseVersionNumber == 0x00)?AuthOut->ucMAC_HCrypto:bTmp,
											 AuthOut->ucCPUSAMID,
											(TxnReqOut->ucPurseVersionNumber != 0x00)?AuthOut->ucMAC_HCrypto[0]:bTmp[0],
											(TxnReqOut->ucPurseVersionNumber != 0x00)?AuthOut->ucMAC_HCrypto[1]:bTmp[0],
											(TxnReqOut->ucPurseVersionNumber != 0x00)?&AuthOut->ucMAC_HCrypto[2]:bTmp,
											 TxnReqOut->ucDeviceID);

	/*if(srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT2)*/
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
													 TxnReqOut->ucBankCode,
													 TxnReqOut->ucLoyaltyCounter,
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
					 TxnReqOut->ucBankCode,
					 TxnReqOut->ucAreaCode,
					 TxnReqOut->ucSubAreaCode,
					 TxnReqOut->ucDeposit,
					 AuthOut->ucTxnSN);

	return inCnt;
}

int inBuildAuthRefundCardSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inCnt = 0;
BYTE bTmp[60],ucAgentNumber[5];
//AuthTxnOnline_APDU_In *AuthIn = (AuthTxnOnline_APDU_In *)Dongle_In;
AuthTxnOnline_APDU_Out *AuthOut = (AuthTxnOnline_APDU_Out *)Dongle_Out;
TxnReqOnline_APDU_Out *TxnReqOut = (TxnReqOnline_APDU_Out *)Dongle_ReqOut;

	memset(bTmp,0x00,sizeof(bTmp));
	memset(ucAgentNumber,0x00,sizeof(ucAgentNumber));

	//follow reader退卡時押mac的方式
	if ((TxnReqOut->ucDeviceID[1] & 0xF0) == 0x70)
		fnASCTOBIN(ucAgentNumber,(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber,4,2, DECIMAL);
	else
	{
	fnASCTOBIN(ucAgentNumber,(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber,4,2,HEX);
		ucAgentNumber[2] = ucAgentNumber[0];
		ucAgentNumber[0] = ucAgentNumber[1];
		ucAgentNumber[1] = ucAgentNumber[2];
	}

	//表頭
	inCnt += inBuildSIS2Header(&API_Out[inCnt],
								TxnReqOut->ucPurseVersionNumber,
								TxnReqOut->ucMsgType,
								TxnReqOut->ucPersonalProfile,
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
								0x00,//0:無/1:故障卡退卡/20:無記名退費/21:記名卡贖回
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

	//退卡表身
	inCnt += inBuildSIS2Body_RefundCard(&API_Out[inCnt],ucAgentNumber,
														TxnReqOut->ucDeposit,
														stStore_DataInfo_t.ucRefundFee,
														stStore_DataInfo_t.ucBrokenFee,
														stStore_DataInfo_t.ucCustomerFee,
														bTmp,
														bTmp,
														TxnReqOut->ucTxnMode,
														TxnReqOut->ucTxnQuqlifier,
														TxnReqOut->ucSignatureKeyKVN,
														AuthOut->ucSignature);

	//表尾
	inCnt += inBuildSIS2End(&API_Out[inCnt],TxnReqOut->ucPurseVersionNumber,
											(TxnReqOut->ucPurseVersionNumber == 0x00)?AuthOut->ucMAC_HCrypto:bTmp,
											 AuthOut->ucCPUSAMID,
											(TxnReqOut->ucPurseVersionNumber != 0x00)?AuthOut->ucMAC_HCrypto[0]:bTmp[0],
											(TxnReqOut->ucPurseVersionNumber != 0x00)?AuthOut->ucMAC_HCrypto[1]:bTmp[0],
											(TxnReqOut->ucPurseVersionNumber != 0x00)?&AuthOut->ucMAC_HCrypto[2]:bTmp,
											 TxnReqOut->ucDeviceID);

	/*if(srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT2)
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
	}*/

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
					 TxnReqOut->ucBankCode,
					 TxnReqOut->ucAreaCode,
					 TxnReqOut->ucSubAreaCode,
					 TxnReqOut->ucDeposit,
					 AuthOut->ucTxnSN);

	return inCnt;
}

int inBuildSIS2Body_RefundCard(BYTE *bOut_Data,BYTE *ucAgentNumber,
											BYTE *ucDepost,
											BYTE *ucCardRefundFee,
											BYTE *ucCPUBrokenFee,
											BYTE *ucCardCustomerFee,
											BYTE *ucRefFee,
											BYTE *ucRFU,
											BYTE ucTxnMode,
											BYTE ucTxnQuqlifier,
											BYTE ucSignatureKeyKVN,
											BYTE *ucSignature)

{
SIS2_BODY_REFUND_CARD *bOutData = (SIS2_BODY_REFUND_CARD *)bOut_Data;

BYTE bBuf[50];
int inRetVal;

	memcpy(bOutData->ucAgentNumber,ucAgentNumber,sizeof(bOutData->ucAgentNumber));
	memcpy(bOutData->ucDepost,ucDepost,sizeof(bOutData->ucDepost));
	memcpy(bOutData->ucCardRefundFee,ucCardRefundFee,sizeof(bOutData->ucCardRefundFee));
	memcpy(bOutData->ucCPUBrokenFee,ucCPUBrokenFee,sizeof(bOutData->ucCPUBrokenFee));
	memcpy(bOutData->ucCardCustomerFee,ucCardCustomerFee,sizeof(bOutData->ucCardCustomerFee));
	memcpy(bOutData->ucRefFee,ucRefFee,sizeof(bOutData->ucRefFee));

	memset(bBuf,0x00,sizeof(bBuf));
	inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_4103,(char *)bBuf,20,0);
	if(inRetVal < SUCCESS)//沒有4103
		memset(bOutData->ucMachineID,0x20,sizeof(bOutData->ucMachineID));
	else
		vdPad((char *)bBuf,(char *)bOutData->ucMachineID,sizeof(bOutData->ucMachineID),TRUE,0x20);
	
	memcpy(bOutData->ucRFU,ucRFU,sizeof(bOutData->ucRFU));
	bOutData->ucTxnMode = ucTxnMode;
	bOutData->ucTxnQuqlifier = ucTxnQuqlifier;
	bOutData->ucSignatureKeyKVN = ucSignatureKeyKVN;
	memcpy(bOutData->ucSignature,ucSignature,sizeof(bOutData->ucSignature));

	return sizeof(SIS2_BODY_REFUND_CARD);
}

int inBuildETxnReqOnlineSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inCnt = 0;
BYTE bTmp[60],ucAgentNumber[5],bSPID[5];
//AuthTxnOnline_APDU_In *AuthIn = (AuthTxnOnline_APDU_In *)Dongle_In;
//AuthTxnOnline_APDU_Out *AuthOut = (AuthTxnOnline_APDU_Out *)Dongle_Out;
TxnReqOnline_APDU_In *TxnReqIn = (TxnReqOnline_APDU_In *)Dongle_In;
TxnReqOnline_APDU_Out *TxnReqOut = (TxnReqOnline_APDU_Out *)Dongle_Out;

	memset(bTmp,0x00,sizeof(bTmp));
	memset(ucAgentNumber,0x00,sizeof(ucAgentNumber));
	//表頭
	inCnt += inBuildSIS2Header(&API_Out[inCnt],
								TxnReqOut->ucPurseVersionNumber,
								TxnReqOut->ucMsgType,
								TxnReqOut->ucSubType,
								TxnReqOut->ucDeviceID,
								TxnReqOut->ucSPID,
								TxnReqIn->ucTxnDateTime,
								TxnReqOut->ucCardID,
								TxnReqOut->ucIssuerCode,
								TxnReqOut->ucTxnSN[0],
								TxnReqOut->ucTxnAmt,
								TxnReqOut->ucEV,
								TxnReqOut->ucLocationID,
								TxnReqOut->ucCardIDLen,
								TxnReqOut->ucTxnSN,
								TxnReqOut->ucCPUDeviceID,
								TxnReqOut->ucCPUSPID,
								TxnReqOut->ucCPULocationID,
								TxnReqOut->ucSubType,
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

	//退卡表身
	fnASCTOBIN(ucAgentNumber,(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber,4,2,HEX);
	memset(bSPID,0x00,sizeof(bSPID));
	if(TxnReqOut->ucPurseVersionNumber == 0x00)
		memcpy(bSPID,&TxnReqOut->ucSPID,1);
	else
		memcpy(bSPID,TxnReqOut->ucCPUSPID,sizeof(TxnReqOut->ucCPUSPID));
	inCnt += inBuildSIS2Body_Add(&API_Out[inCnt],TxnReqOut->ucLoyaltyCounter,
												 ucAgentNumber,
												 TxnReqOut->ucBankCode,
												 bSPID,
												 bTmp,
												 TxnReqOut->ucTxnMode,
												 TxnReqOut->ucTxnQuqlifier,
												 TxnReqOut->ucSignatureKeyKVN,
												 bTmp);

	//表尾
	inCnt += inBuildSIS2End(&API_Out[inCnt],TxnReqOut->ucPurseVersionNumber,
											bTmp,
											bTmp,
											bTmp[0],
											bTmp[0],
											bTmp,
											TxnReqOut->ucDeviceID);

	/*if(srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT2)*/
	/*{//加值重送
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
													 TxnReqOut->ucBankCode,
													 TxnReqOut->ucLoyaltyCounter,
													 TxnReqOut->ucDeviceID,
													 TxnReqOut->ucSPID,
													 TxnReqOut->ucLocationID,
													 TxnReqOut->ucCPUDeviceID,
													 TxnReqOut->ucCPUSPID,
													 TxnReqOut->ucCPULocationID);
		fnUnPack(bRecvData,SIS2_ADD_VALUE_RESEND_LEN,srTxnData.srIngData.anTAVR_TARC);
	}*/

	vdUpdateCardInfo2(inTxnType,
					 TxnReqOut->ucCardID,
					 TxnReqOut->ucPID,
					 (BYTE )TxnReqOut->bAutoLoad,
					 TxnReqOut->ucCardType,
					 TxnReqOut->ucPersonalProfile,
					 TxnReqOut->ucAutoLoadAmt,
					 TxnReqOut->ucEV,
					 TxnReqOut->ucExpiryDate,
					 TxnReqOut->ucPurseVersionNumber,
					 TxnReqOut->ucBankCode,
					 TxnReqOut->ucAreaCode,
					 TxnReqOut->ucSubAreaCode,
					 TxnReqOut->ucDeposit,
					 TxnReqOut->ucTxnSN);

	return inCnt;
}

int inBuildCBikeDeductSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inCnt = 0;
BYTE bTmp[60],bBuf[50],T484600[2];
CBikeDeduct_APDU_In *AuthIn = (CBikeDeduct_APDU_In *)Dongle_In;
CBikeDeduct_APDU_Out *AuthOut = (CBikeDeduct_APDU_Out *)Dongle_Out;
CBikeRead_APDU_Out *TxnReqOut = (CBikeRead_APDU_Out *)Dongle_ReqOut;
SIS2_BODY_DEDUCT srSIS2Body;
long lnAmount = 0L;

	memset(&srSIS2Body,0x00,sizeof(srSIS2Body));
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	memset(T484600,0x00,sizeof(T484600));
	if(inXMLGetData(srXML,(char *)"T484600",(char *)bBuf,sizeof(bBuf),0) >= SUCCESS)
	{
		if(strlen((char *)bBuf) == 2)
			fnPack((char *)bBuf,strlen((char *)bBuf),T484600);
		else
			log_msg(LOG_LEVEL_ERROR,"inBuildCBikeDeductSIS2 Fail 1 = %s",bBuf);
	}
	//表頭
	inCnt += inBuildSIS2Header(&API_Out[inCnt],
								TxnReqOut->ucPurseVersionNumber,
								AuthOut->ucDeductMsgType,
								(TxnReqOut->ucPurseVersionNumber == LEVEL2)?TxnReqOut->ucPersonalProfile:AuthOut->ucDeductSubType,
								TxnReqOut->ucDeviceID,
								TxnReqOut->ucSPID,
								AuthOut->stDeductOut_t.ucTxnDateTime,
								TxnReqOut->ucCardID,
								TxnReqOut->ucIssuerCode,
								AuthOut->stDeductOut_t.ucTxnSN[0],
								AuthOut->stDeductOut_t.ucTxnAmt,
								AuthOut->stDeductOut_t.ucEV,
								TxnReqOut->ucLocationID,
								TxnReqOut->ucCardIDLen,
								AuthOut->stDeductOut_t.ucTxnSN,
								TxnReqOut->ucCPUDeviceID,
								TxnReqOut->ucCPUSPID,
								TxnReqOut->ucCPULocationID,
								T484600[0],
								TxnReqOut->ucPersonalProfile,
								TxnReqOut->ucPersonalProfile,
								0x02,
								TxnReqOut->ucPID,
								AuthOut->stDeductOut_t.ucCTC,
								TxnReqOut->ucAreaCode,
								TxnReqOut->ucSubAreaCode,
								TxnReqOut->ucTxnSN,
								TxnReqOut->ucEV
								);

	//退卡表身
	fnASCTOBIN(srSIS2Body.ucAgentNumber,(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber,4,2,HEX);
	if(srTxnData.srIngData.fCBikeRentalFlag == TRUE)
		srSIS2Body.ucEntryExit[0] = 0x01;
	else
		srSIS2Body.ucEntryExit[0] = 0x00;
	memcpy(srSIS2Body.ucTransferDiscount,AuthIn->ucTransferDiscount,sizeof(AuthIn->ucTransferDiscount));
	memcpy(srSIS2Body.ucPersonalDiscount,AuthIn->ucPersonalDiscount,sizeof(AuthIn->ucPersonalDiscount));
	memset(bBuf,0x00,sizeof(bBuf));
	lnAmount = 0L;
	if(inXMLGetAmt(srXML,(char *)"T484601",&lnAmount,0) >= SUCCESS)//折扣前
		memcpy(&srSIS2Body.ucRFU1_1[0],(char *)&lnAmount,3);
	if(inXMLGetData(srXML,(char *)"T484602",(char *)bBuf,sizeof(bBuf),0) >= SUCCESS)//費率
		fnASCTOBIN(&srSIS2Body.ucRFU1_1[3],bBuf,strlen((char *)bBuf),1,DECIMAL);
	lnAmount = 0L;
	if(inXMLGetAmt(srXML,(char *)"T484603",&lnAmount,0) >= SUCCESS)//折扣後
		memcpy(&srSIS2Body.ucRFU1_1[4],(char *)&lnAmount,3);
	if(inXMLGetData(srXML,(char *)"T484604",(char *)bBuf,sizeof(bBuf),0) >= SUCCESS)//車牌號碼
		memcpy(&srSIS2Body.ucRFU3[0],bBuf,(strlen((char *)bBuf) > 10)?10:strlen((char *)bBuf));
	//memcpy(&srSIS2Body.ucRFU3[13],&TxnReqOut->ucURT[0],3);//轉乘-上筆交易序號
	srSIS2Body.ucRFU3[13] = TxnReqOut->ucURT[2];
	srSIS2Body.ucRFU3[14] = TxnReqOut->ucURT[1];
	srSIS2Body.ucRFU3[15] = TxnReqOut->ucURT[0];
	srSIS2Body.ucRFU3[16] = TxnReqOut->ucCardType;
	//memcpy(&srSIS2Body.ucRFU3[17],&TxnReqOut->ucURT[3],4);//轉乘-上筆交易時間
	srSIS2Body.ucRFU3[17] = TxnReqOut->ucURT[6];
	srSIS2Body.ucRFU3[18] = TxnReqOut->ucURT[5];
	srSIS2Body.ucRFU3[19] = TxnReqOut->ucURT[4];
	srSIS2Body.ucRFU3[20] = TxnReqOut->ucURT[3];
	srSIS2Body.ucRFU3[29] = 0x01;
	memcpy(&srSIS2Body.ucRFU4[0],TxnReqOut->ucCPULocationID,2);
	memcpy(&srSIS2Body.ucRFU4[2],srTxnData.srIngData.bCBikeTime,4);
	srSIS2Body.ucRFU5[0] = 0x01;

	inCnt += inBuildSIS2Body_Deduct(&API_Out[inCnt],srSIS2Body.ucEntryExit,
												    bTmp,
												    AuthOut->ucTransferGuoupCode,
												    AuthOut->ucTransferGuoupCode2,
												    srSIS2Body.ucRFU1_1,
												    srSIS2Body.ucTransferDiscount,
												    srSIS2Body.ucPersonalDiscount,
												    bTmp,
												    //bTmp,
													TxnReqOut->ucLoyaltyCounter,
													srSIS2Body.ucAgentNumber,
													bTmp,
													srSIS2Body.ucRFU3,
													srSIS2Body.ucRFU4,
													srSIS2Body.ucRFU5,
													AuthOut->stDeductOut_t.ucTxnMode,
													AuthOut->stDeductOut_t.ucTxnQuqlifier,
													AuthOut->ucSignatureKeyKVN,
													AuthOut->stDeductOut_t.ucSignature);

	//表尾
	inCnt += inBuildSIS2End(&API_Out[inCnt],TxnReqOut->ucPurseVersionNumber,
											(TxnReqOut->ucPurseVersionNumber == 0x00)?AuthOut->stDeductOut_t.ucMAC_HCrypto:bTmp,
											 AuthOut->ucCPUSAMID,
											(TxnReqOut->ucPurseVersionNumber != 0x00)?AuthOut->ucHashType:bTmp[0],
											(TxnReqOut->ucPurseVersionNumber != 0x00)?AuthOut->ucHostAdminKVN:bTmp[0],
											(TxnReqOut->ucPurseVersionNumber != 0x00)?AuthOut->stDeductOut_t.ucMAC_HCrypto:bTmp,
											 TxnReqOut->ucDeviceID);

	/*if(srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT2)*/
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
													 TxnReqOut->ucBankCode,
													 TxnReqOut->ucLoyaltyCounter,
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
					 AuthOut->stDeductOut_t.ucEV,
					 TxnReqOut->ucExpiryDate,
					 TxnReqOut->ucPurseVersionNumber,
					 TxnReqOut->ucBankCode,
					 TxnReqOut->ucAreaCode,
					 TxnReqOut->ucSubAreaCode,
					 bTmp,
					 AuthOut->stDeductOut_t.ucTxnSN);

	return inCnt;
}

int inBuildEDCADeductSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inCnt = 0;
BYTE bTmp[60],bBuf[50],T484600[2];
EDCADeduct_APDU_In *AuthIn = (EDCADeduct_APDU_In *)Dongle_In;
EDCADeduct_APDU_Out *AuthOut = (EDCADeduct_APDU_Out *)Dongle_Out;
EDCARead_APDU_Out *TxnReqOut = (EDCARead_APDU_Out *)Dongle_ReqOut;
SIS2_BODY_DEDUCT srSIS2Body;
long lnAmount = 0L;
long lnAmount1 = 0L;
long lnAmount2 = 0L;

	memset(&srSIS2Body,0x00,sizeof(srSIS2Body));
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	memset(T484600,0x00,sizeof(T484600));
	if(inXMLGetData(srXML,(char *)"T484600",(char *)bBuf,sizeof(bBuf),0) >= SUCCESS)
	{
		if(strlen((char *)bBuf) == 2)
			fnPack((char *)bBuf,strlen((char *)bBuf),T484600);
		else
			log_msg(LOG_LEVEL_ERROR,"inBuildEDCADeductSIS2 Fail 1 = %s",bBuf);
	}
	//表頭
	inCnt += inBuildSIS2Header(&API_Out[inCnt],
								TxnReqOut->ucPurseVersionNumber,
								AuthOut->ucDeductMsgType,
								(TxnReqOut->ucPurseVersionNumber == LEVEL2)?TxnReqOut->ucPersonalProfile:AuthOut->ucDeductSubType,
								TxnReqOut->ucDeviceID,
								TxnReqOut->ucSPID,
								AuthOut->stDeductOut_t.ucTxnDateTime,
								TxnReqOut->ucCardID,
								TxnReqOut->ucIssuerCode,
								AuthOut->stDeductOut_t.ucTxnSN[0],
								AuthOut->stDeductOut_t.ucTxnAmt,
								AuthOut->stDeductOut_t.ucEV,
								TxnReqOut->ucLocationID,
								TxnReqOut->ucCardIDLen,
								AuthOut->stDeductOut_t.ucTxnSN,
								TxnReqOut->ucCPUDeviceID,
								TxnReqOut->ucCPUSPID,
								TxnReqOut->ucCPULocationID,
								T484600[0],
								TxnReqOut->ucPersonalProfile,
								TxnReqOut->ucPersonalProfile,
								0x02,
								TxnReqOut->ucPID,
								AuthOut->stDeductOut_t.ucCTC,
								TxnReqOut->ucAreaCode,
								TxnReqOut->ucSubAreaCode,
								TxnReqOut->ucTxnSN,
								TxnReqOut->ucEV
								);


	fnASCTOBIN(srSIS2Body.ucAgentNumber,(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber,4,2,HEX);

	memcpy((char *)&lnAmount, AuthOut->stDeductOut_t.ucTxnAmt, sizeof(AuthOut->stDeductOut_t.ucTxnAmt));
	memcpy((char *)&lnAmount1, AuthIn->ucTransferDiscount,sizeof(AuthIn->ucTransferDiscount));
	memcpy((char *)&lnAmount2, AuthIn->ucPersonalDiscount,sizeof(AuthIn->ucPersonalDiscount));
	lnAmount += lnAmount1 + lnAmount2;
	memcpy(&srSIS2Body.ucRFU1_1[0], (char *)&lnAmount, 3);
	srSIS2Body.ucRFU1_1[3] = 0x64;
	memcpy(&srSIS2Body.ucRFU1_1[4], (char *)&lnAmount, 3);

	memcpy(srSIS2Body.ucTransferDiscount,AuthIn->ucTransferDiscount,sizeof(AuthIn->ucTransferDiscount));
	memcpy(srSIS2Body.ucPersonalDiscount,AuthIn->ucPersonalDiscount,sizeof(AuthIn->ucPersonalDiscount));
/*	memset(bBuf,0x00,sizeof(bBuf));
	lnAmount = 0L;
	if(inXMLGetAmt(srXML,(char *)"T484601",&lnAmount,0) >= SUCCESS)//折扣前
		memcpy(&srSIS2Body.ucRFU1_1[0],(char *)&lnAmount,3);
	if(inXMLGetData(srXML,(char *)"T484602",(char *)bBuf,sizeof(bBuf),0) >= SUCCESS)//費率
		fnASCTOBIN(&srSIS2Body.ucRFU1_1[3],bBuf,strlen((char *)bBuf),1,DECIMAL);
	lnAmount = 0L;
	if(inXMLGetAmt(srXML,(char *)"T484603",&lnAmount,0) >= SUCCESS)//折扣後
		memcpy(&srSIS2Body.ucRFU1_1[4],(char *)&lnAmount,3);
	if(inXMLGetData(srXML,(char *)"T484604",(char *)bBuf,sizeof(bBuf),0) >= SUCCESS)//車牌號碼
		memcpy(&srSIS2Body.ucRFU3[0],bBuf,(strlen((char *)bBuf) > 10)?10:strlen((char *)bBuf));

	memcpy(&srSIS2Body.ucRFU4[0],TxnReqOut->ucCPULocationID,2);
	memcpy(&srSIS2Body.ucRFU4[2],srTxnData.srIngData.bCBikeTime,4);
	srSIS2Body.ucRFU5[0] = 0x01;
*/
	//memcpy(&srSIS2Body.ucRFU3[13],&TxnReqOut->ucURT[0],3);//轉乘-上筆交易序號
	srSIS2Body.ucRFU3[13] = TxnReqOut->ucURT[2];
	srSIS2Body.ucRFU3[14] = TxnReqOut->ucURT[1];
	srSIS2Body.ucRFU3[15] = TxnReqOut->ucURT[0];
	srSIS2Body.ucRFU3[16] = TxnReqOut->ucCardType;
	//memcpy(&srSIS2Body.ucRFU3[17],&TxnReqOut->ucURT[3],4);//轉乘-上筆交易時間
	srSIS2Body.ucRFU3[17] = TxnReqOut->ucURT[6];
	srSIS2Body.ucRFU3[18] = TxnReqOut->ucURT[5];
	srSIS2Body.ucRFU3[19] = TxnReqOut->ucURT[4];
	srSIS2Body.ucRFU3[20] = TxnReqOut->ucURT[3];
	srSIS2Body.ucRFU3[29] = 0x01;

	srSIS2Body.ucRFU4[0] = (unsigned char)(srTxnData.srIngData.ulSubLocId / 256);
	srSIS2Body.ucRFU4[1] = (unsigned char)(srTxnData.srIngData.ulSubLocId % 256);
	srSIS2Body.ucRFU5[0] = 0x01;
	inCnt += inBuildSIS2Body_Deduct(&API_Out[inCnt],srSIS2Body.ucEntryExit,
												    bTmp,
												    AuthOut->ucTransferGuoupCode,
												    AuthOut->ucTransferGuoupCode2,
												    srSIS2Body.ucRFU1_1,
												    srSIS2Body.ucTransferDiscount,
												    srSIS2Body.ucPersonalDiscount,
												    bTmp,
												    //bTmp,
													TxnReqOut->ucLoyaltyCounter,
													srSIS2Body.ucAgentNumber,
													bTmp,
													srSIS2Body.ucRFU3,
													srSIS2Body.ucRFU4,
													srSIS2Body.ucRFU5,
													AuthOut->stDeductOut_t.ucTxnMode,
													AuthOut->stDeductOut_t.ucTxnQuqlifier,
													AuthOut->ucSignatureKeyKVN,
													AuthOut->stDeductOut_t.ucSignature);

	//表尾
	inCnt += inBuildSIS2End(&API_Out[inCnt],TxnReqOut->ucPurseVersionNumber,
											(TxnReqOut->ucPurseVersionNumber == 0x00)?AuthOut->stDeductOut_t.ucMAC_HCrypto:bTmp,
											 AuthOut->ucCPUSAMID,
											(TxnReqOut->ucPurseVersionNumber != 0x00)?AuthOut->ucHashType:bTmp[0],
											(TxnReqOut->ucPurseVersionNumber != 0x00)?AuthOut->ucHostAdminKVN:bTmp[0],
											(TxnReqOut->ucPurseVersionNumber != 0x00)?AuthOut->stDeductOut_t.ucMAC_HCrypto:bTmp,
											 TxnReqOut->ucDeviceID);

	/*if(srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT2)*/
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
													 TxnReqOut->ucBankCode,
													 TxnReqOut->ucLoyaltyCounter,
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
					 AuthOut->stDeductOut_t.ucEV,
					 TxnReqOut->ucExpiryDate,
					 TxnReqOut->ucPurseVersionNumber,
					 TxnReqOut->ucBankCode,
					 TxnReqOut->ucAreaCode,
					 TxnReqOut->ucSubAreaCode,
					 bTmp,
					 AuthOut->stDeductOut_t.ucTxnSN);

	return inCnt;
}

int inBuildICERETxnSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inCnt = 0;
BYTE bTmp[60],ucMsgType,ucSubType,ucSubType2,ucAgentNumber[5],bSPID[5];
//AuthTxnOnline_APDU_In *AuthIn = (AuthTxnOnline_APDU_In *)Dongle_In;
VerifyHCrypt_APDU_In *AuthOut = (VerifyHCrypt_APDU_In *)Dongle_In;
TxnReqOnline_APDU_Out *TxnReqOut = (TxnReqOnline_APDU_Out *)Dongle_ReqOut;

	memset(bTmp,0x00,sizeof(bTmp));
	memset(ucAgentNumber,0x00,sizeof(ucAgentNumber));
	vdICERBuildETxnPCode(&ucMsgType,&ucSubType2);
	ucSubType = 0xC8;
	//表頭
	inCnt += inBuildSIS2Header(&API_Out[inCnt],
								TxnReqOut->ucPurseVersionNumber,
								ucMsgType,
								ucSubType,
								TxnReqOut->ucDeviceID,
								TxnReqOut->ucSPID,
								AuthOut->ucTxnDateTime,
								TxnReqOut->ucCardID,
								TxnReqOut->ucIssuerCode,
								AuthOut->ucCardSN[0],
								AuthOut->ucTxnAmt,
								AuthOut->ucEV,
								TxnReqOut->ucLocationID,
								TxnReqOut->ucCardIDLen,
								AuthOut->ucCardSN,
								TxnReqOut->ucCPUDeviceID,
								TxnReqOut->ucCPUSPID,//要改srTxnData.srParameter.bMerchantID
								TxnReqOut->ucCPULocationID,
								ucSubType2,
								TxnReqOut->ucPersonalProfile,
								TxnReqOut->ucPersonalProfile,
								0x02,
								TxnReqOut->ucPID,
								TxnReqOut->ucCTC,
								TxnReqOut->ucAreaCode,
								TxnReqOut->ucSubAreaCode,
								TxnReqOut->ucTxnSN,
								AuthOut->unEVBeforeTxn
								);

	if (srTxnData.srIngData.inTransType == TXN_E_ADD || srTxnData.srIngData.inTransType == TXN_E_REFUND || srTxnData.srIngData.inTransType == TXN_E_VOID_ADD ||
		srTxnData.srIngData.inTransType == TXN_E_QR_ADD || srTxnData.srIngData.inTransType == TXN_E_QR_REFUND || srTxnData.srIngData.inTransType == TXN_E_QR_VOID_ADD)
	{
		fnASCTOBIN(ucAgentNumber,(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber,4,2,HEX);
		memset(bSPID,0x00,sizeof(bSPID));
		if(TxnReqOut->ucPurseVersionNumber == 0x00)
			memcpy(bSPID,&TxnReqOut->ucSPID,1);
		else
			memcpy(bSPID,TxnReqOut->ucCPUSPID,sizeof(TxnReqOut->ucCPUSPID));
		inCnt += inBuildSIS2Body_Add(&API_Out[inCnt],TxnReqOut->ucLoyaltyCounter,
													 ucAgentNumber,
													 TxnReqOut->ucBankCode,
													 bSPID,
													 bTmp,
													 AuthOut->ucTMSerialNumber_3[0],
													 AuthOut->ucTMSerialNumber_3[1],
													 AuthOut->ucTMSerialNumber_3[2],
													 AuthOut->ucAuthCode);
	}
	else if (srTxnData.srIngData.inTransType == TXN_E_DEDUCT || srTxnData.srIngData.inTransType == TXN_E_QR_DEDUCT)
	{
		SIS2_BODY_DEDUCT srSIS2Body;

		memset(&srSIS2Body,0x00,sizeof(srSIS2Body));
		fnASCTOBIN(srSIS2Body.ucAgentNumber,(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber,4,2,HEX);
		srSIS2Body.ucRFU3[29] = 0x01;
		memcpy(&srSIS2Body.ucRFU4[0],TxnReqOut->ucCPULocationID,2);
		srSIS2Body.ucRFU5[0] = 0x01;

		inCnt += inBuildSIS2Body_Deduct(&API_Out[inCnt],bTmp,
													    bTmp,
													    bTmp[0],
													    bTmp,
													    srSIS2Body.ucRFU1_1,
													    srSIS2Body.ucTransferDiscount,
													    srSIS2Body.ucPersonalDiscount,
													    bTmp,
													    //bTmp,
														TxnReqOut->ucLoyaltyCounter,
														srSIS2Body.ucAgentNumber,
														bTmp,
														srSIS2Body.ucRFU3,
														srSIS2Body.ucRFU4,
														srSIS2Body.ucRFU5,
														AuthOut->ucTMSerialNumber_3[0],
														AuthOut->ucTMSerialNumber_3[1],
														AuthOut->ucTMSerialNumber_3[2],
														AuthOut->ucAuthCode);
	}

	//表尾
	inCnt += inBuildSIS2End(&API_Out[inCnt],TxnReqOut->ucPurseVersionNumber,
											bTmp,
											TxnReqOut->ucCPUSAMID,
											bTmp[0],
											bTmp[0],
											AuthOut->ucHCrypto,
											TxnReqOut->ucDeviceID);

#if 0
	/*if(srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT2)*/
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
													 TxnReqOut->ucBankCode,
													 TxnReqOut->ucLoyaltyCounter,
													 TxnReqOut->ucDeviceID,
													 TxnReqOut->ucSPID,
													 TxnReqOut->ucLocationID,
													 TxnReqOut->ucCPUDeviceID,
													 TxnReqOut->ucCPUSPID,
													 TxnReqOut->ucCPULocationID);
		fnUnPack(bRecvData,SIS2_ADD_VALUE_RESEND_LEN,srTxnData.srIngData.anTAVR_TARC);
	}
#endif

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
					 TxnReqOut->ucBankCode,
					 TxnReqOut->ucAreaCode,
					 TxnReqOut->ucSubAreaCode,
					 TxnReqOut->ucDeposit,
					 AuthOut->ucCardSN);

	return inCnt;
}

