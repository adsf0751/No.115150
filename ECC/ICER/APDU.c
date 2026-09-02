
#ifdef EZ_AIRLINKEDC
#include "_StdAfx.h"
#else
#include "stdAfx.h"
#endif
//#include "Global.h"

int inBuildResetCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
Reset_APDU_In *bAPDUReq = (Reset_APDU_In *)bAPDU_Req;
Reset_TM_In *bTMIn = (Reset_TM_In *)bTM_In;

	memcpy(bAPDUReq->ucTMLocationID,bTMIn->ucTMLocationID,sizeof(bAPDUReq->ucTMLocationID));//終端機(TM)店號
	memcpy(bAPDUReq->ucTMID,bTMIn->ucTMID,sizeof(bAPDUReq->ucTMID));//終端機(TM)機號
	memcpy(bAPDUReq->ucTMTxnDateTime,bTMIn->ucTMTxnDateTime,sizeof(bAPDUReq->ucTMTxnDateTime));//終端機(TM)交易日期時間
	memcpy(bAPDUReq->ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(bAPDUReq->ucTMSerialNumber));//終端機(TM)交易序號
	memcpy(bAPDUReq->ucTMAgentNumber,bTMIn->ucTMAgentNumber,sizeof(bAPDUReq->ucTMAgentNumber));//終端機(TM)收銀員代號
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)
	//bAPDUReq->ucLocationID = LOCATION_ID;//舊場站代碼
	fnASCTOBIN(&bAPDUReq->ucLocationID,bTMIn->ucLocationID,3,1,DECIMAL);//舊場站代碼
	fnASCTOBIN(bAPDUReq->ucCPULocationID,bTMIn->ucCPULocationID,5,2,DECIMAL);//新場站代碼
	fnASCTOBIN(&bAPDUReq->ucSPID,bTMIn->ucSPID,3,1,DECIMAL);//舊服務業者代碼
	fnASCTOBIN(bAPDUReq->ucCPUSPID,bTMIn->ucCPUSPID,8,3,DECIMAL);//新服務業者代碼
/*#ifdef READER_MODE
	if(srTxnData.srParameter.bDeductType == '1')
		bAPDUReq->ucRFU1[0] = 0x08;
	else //if(srTxnData.srParameter.bDeductType == '2')
		bAPDUReq->ucRFU1[0] = 0x73;
	memcpy(&bAPDUReq->ucRFU1[1],"\xB8\x0B\xE8\x03",4);
#endif*/
	bAPDUReq->ucSAMSlot[0] = bTMIn->ucSAMSlot[0];//SAM卡位置
	if(srTxnData.srParameter.chAccFreeRidesMode == '1')
		bAPDUReq->AccumulatedFreeRidesMode = 1;
	else
		bAPDUReq->AccumulatedFreeRidesMode = 0;

	if(srTxnData.srParameter.chReaderUartDebug == '1')
		bAPDUReq->ecc_debug_flag = 1;
	else
		bAPDUReq->ecc_debug_flag = 0;

	if(srTxnData.srParameter.chCMASMode == '2')
		bAPDUReq->MAC_SPID_Mode = 1;
	else
		bAPDUReq->MAC_SPID_Mode = 0;

	return(sizeof(Reset_APDU_In));
}

int inBuildSignOnCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
SignOn_APDU_In *bAPDUReq = (SignOn_APDU_In *)bAPDU_Req;
SignOn_TM_In *bTMIn = (SignOn_TM_In *)bTM_In;

	memcpy(bAPDUReq->ucHTAC,bTMIn->ucHTAC,sizeof(bAPDUReq->ucHTAC));//H-TAC
	memcpy(bAPDUReq->ucHATOKEN,bTMIn->ucHATOKEN,sizeof(bAPDUReq->ucHATOKEN));//HA-Token
	memcpy(&bAPDUReq->stSAMParaSettingInfo_t,&bTMIn->stSAMParaSettingInfo_t,sizeof(bAPDUReq->stSAMParaSettingInfo_t));//CPU SAM Parameter Setting Data
	bAPDUReq->stSAMParameterInfo_t.bCPDReadFlag0 = bTMIn->ucCPDReadFlag;
	bAPDUReq->stSAMParameterInfo_t.bCPDReadFlag1 = bTMIn->ucCPDReadFlag>>4;
	bAPDUReq->stSAMParameterInfo_t.bOneDayQuotaWrite2 = bTMIn->ucOneDayQuotaWrite;
	bAPDUReq->stSAMParameterInfo_t.bOneDayQuotaWrite3 = bTMIn->ucOneDayQuotaWrite>>4;
	bAPDUReq->stSAMParameterInfo_t.bSAMSignOnControlFlag4 = bTMIn->ucSAMSignOnControlFlag;
	bAPDUReq->stSAMParameterInfo_t.bSAMSignOnControlFlag5 = bTMIn->ucSAMSignOnControlFlag>>4;
	bAPDUReq->stSAMParameterInfo_t.bCheckEVFlag6 = bTMIn->stTermHostInfo_t.ucCheckEVFlag;
	bAPDUReq->stSAMParameterInfo_t.bDeductLimitFlag7 = bTMIn->stTermHostInfo_t.ucDeductLimitFlag;
	bAPDUReq->stSAMParameterInfo_t.bOneDayQuotaFlag0 = bTMIn->stTermHostInfo_t.ucOneDayQuotaFlag;
	bAPDUReq->stSAMParameterInfo_t.bOneDayQuotaFlag1 = bTMIn->stTermHostInfo_t.ucOneDayQuotaFlag >> 4;
	bAPDUReq->stSAMParameterInfo_t.bOnceQuotaFlag2 = bTMIn->stTermHostInfo_t.ucOnceQuotaFlag;
	bAPDUReq->stSAMParameterInfo_t.bCheckDeductFlag3 = bTMIn->stTermHostInfo_t.ucCheckDeductFlag;
	memcpy(bAPDUReq->stSAMParameterInfo_t.ucOneDayQuota,bTMIn->stTermHostInfo_t.ucOneDayQuota,sizeof(bAPDUReq->stSAMParameterInfo_t.ucOneDayQuota));//One Day Quota
	memcpy(bAPDUReq->stSAMParameterInfo_t.ucOnceQuota,bTMIn->stTermHostInfo_t.ucOnceQuota,sizeof(bAPDUReq->stSAMParameterInfo_t.ucOnceQuota));//Once Quota
	memcpy(bAPDUReq->stSAMParameterInfo_t.ucCheckDeductValue,bTMIn->stTermHostInfo_t.ucCheckDeductValue,sizeof(bAPDUReq->stSAMParameterInfo_t.ucCheckDeductValue));//Check Deduct Value
	bAPDUReq->stSAMParameterInfo_t.ucAddQuotaFlag = bTMIn->stTermHostInfo_t.ucAddQuotaFlag;//Add Quota Flag
	memcpy(bAPDUReq->stSAMParameterInfo_t.ucAddQuota,bTMIn->stTermHostInfo_t.ucAddQuota,sizeof(bAPDUReq->stSAMParameterInfo_t.ucAddQuota));//Add Quota
//	inCnt += 31;//保留
	bAPDUReq->ucHashType = bTMIn->ucHashType;//EDC,Hash Type
	memcpy(bAPDUReq->ucCPUEDC,bTMIn->ucCPUEDC,sizeof(bAPDUReq->ucCPUEDC));//EDC,Hash Value
	return(sizeof(SignOn_APDU_In));
}

int inBuildReadCardBasicDataCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
ReadCardBasicData_APDU_In *bAPDUReq = (ReadCardBasicData_APDU_In *)bAPDU_Req;
ReadCardBasicData_TM_In *bTMIn = (ReadCardBasicData_TM_In *)bTM_In;

	bAPDUReq->ucLCDControlFlag = bTMIn->ucLCDControlFlag;
	if(srTxnData.srParameter.chBasicCheckQuotaFlag == '1')
	{
		memcpy(bAPDUReq->ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(bAPDUReq->ucTMSerialNumber));//終端機(TM)交易序號
		fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)
		bAPDUReq->OnceQuotaCheckFlag = 0x01;
		bAPDUReq->OneDayQuotaCheckFlag = 0x01;
		fnASCTOBIN(bAPDUReq->ucTxnAmt,bTMIn->anTxnAmt,8,3,DECIMAL);//交易金額
	}

	return(sizeof(ReadCardBasicData_APDU_In));
}

int inBuildTxnReqOnlineCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
TxnReqOnline_APDU_In *bAPDUReq = (TxnReqOnline_APDU_In *)bAPDU_Req;
TxnReqOnline_TM_In *bTMIn = (TxnReqOnline_TM_In *)bTM_In;

	fnASCTOBIN(&bAPDUReq->ucMsgType,bTMIn->anMsgType,2,1,HEX);
	fnASCTOBIN(&bAPDUReq->ucSubType,bTMIn->anSubType,2,1,HEX);
	if(srTxnData.srParameter.chCMASMode == '2')
		memcpy(bAPDUReq->ucTMLocationID,srTxnData.srREQData.bT5503,sizeof(bAPDUReq->ucTMLocationID));//終端機(TM)店號
	else
		memcpy(bAPDUReq->ucTMLocationID,bTMIn->ucTMLocationID,sizeof(bAPDUReq->ucTMLocationID));//終端機(TM)店號
	memcpy(bAPDUReq->ucTMID,bTMIn->ucTMID,sizeof(bAPDUReq->ucTMID));//終端機(TM)機號
	memcpy(bAPDUReq->ucTMTxnDateTime,bTMIn->ucTMTxnDateTime,sizeof(bAPDUReq->ucTMTxnDateTime));//終端機(TM)交易日期時間
	memcpy(bAPDUReq->ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(bAPDUReq->ucTMSerialNumber));//終端機(TM)交易序號
	memcpy(bAPDUReq->ucTMAgentNumber,bTMIn->ucTMAgentNumber,sizeof(bAPDUReq->ucTMAgentNumber));//終端機(TM)收銀員代號
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)
	fnASCTOBIN(bAPDUReq->ucTxnAmt,bTMIn->anTxnAmt,8,3,DECIMAL);//交易金額
	bAPDUReq->ucReadPurseFlag = bTMIn->ucReadPurseFlag;
	//bAPDUReq->ucReadPurseFlag = 0x00;//需重新Read Basic Data
	if(srTxnData.srIngData.inTransType == TXN_ECC_REFUND_CARD)
	{
		fnASCTOBIN(bAPDUReq->ucRefundFee,bTMIn->anRefundFee,5,2,DECIMAL);
		fnASCTOBIN(bAPDUReq->ucBrokenFee,bTMIn->anBrokenFee,5,2,DECIMAL);
		fnASCTOBIN(bAPDUReq->ucCustomerFee,bTMIn->anCustomerFee,5,2,DECIMAL);
	}
	else
	{
		memset(bAPDUReq->ucRefundFee,0x00,sizeof(bAPDUReq->ucRefundFee));//Refund Fee
		memset(bAPDUReq->ucBrokenFee,0x00,sizeof(bAPDUReq->ucBrokenFee));//Broken Fee
		memset(bAPDUReq->ucCustomerFee,0x00,sizeof(bAPDUReq->ucCustomerFee));//Customer Fee
	}
	bAPDUReq->ucSPID = 0x00;
	if(srTxnData.srParameter.chCMASMode == '2')
		fnASCTOBIN(bAPDUReq->ucCPUSPID,(BYTE *)srTxnData.srREQData.bT5503,strlen((char *)srTxnData.srREQData.bT5503),sizeof(bAPDUReq->ucCPUSPID),DECIMAL);//新服務業者代碼
	if(srTxnData.srIngData.fTMRetryFlag == TRUE)
		bAPDUReq->RetryFlag = TRUE;
	memset(bAPDUReq->ucRFU,0x00,sizeof(bAPDUReq->ucRFU));//保留

	memcpy(stStore_DataInfo_t.ucTMLocationID,bTMIn->ucTMLocationID,sizeof(stStore_DataInfo_t.ucTMLocationID));
	memcpy(stStore_DataInfo_t.ucTMID,bTMIn->ucTMID,sizeof(stStore_DataInfo_t.ucTMID));
	memcpy(stStore_DataInfo_t.ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(stStore_DataInfo_t.ucTMSerialNumber));
	memcpy(stStore_DataInfo_t.ucTMAgentNumber,bTMIn->ucTMAgentNumber,sizeof(stStore_DataInfo_t.ucTMAgentNumber));
	memcpy(stStore_DataInfo_t.ucRefundFee,bAPDUReq->ucRefundFee,sizeof(stStore_DataInfo_t.ucRefundFee));
	memcpy(stStore_DataInfo_t.ucBrokenFee,bAPDUReq->ucBrokenFee,sizeof(stStore_DataInfo_t.ucBrokenFee));
	memcpy(stStore_DataInfo_t.ucCustomerFee,bAPDUReq->ucCustomerFee,sizeof(stStore_DataInfo_t.ucCustomerFee));

	return(sizeof(TxnReqOnline_APDU_In));
}

int inBuildAuthTxnOnlineCommand1(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
BYTE bDate[20];
AuthTxnOnline_APDU_In *bAPDUReq = (AuthTxnOnline_APDU_In *)bAPDU_Req;
AuthTxnOnline_TM_In *bTMIn = (AuthTxnOnline_TM_In *)bTM_In;

	memcpy(bAPDUReq->ucExpiryDate,bTMIn->ucExpiryDate,sizeof(bAPDUReq->ucExpiryDate));//Expiry Date
	memcpy(bAPDUReq->ucProfileExpiryDate,bTMIn->ucProfileExpiryDate,sizeof(bAPDUReq->ucProfileExpiryDate));//Profile Expiry Date
	UnixToDateTime((unsigned char*)bAPDUReq->ucProfileExpiryDate,(unsigned char*)bDate,14);
	fngetDosDateCnt(bDate,bAPDUReq->ucProfileExpiryDateDOS);//Profile Expiry Date(DOS)
	if(bTMIn->ucPurseVersionNumber == MIFARE)
		memset(bAPDUReq->ucTxnToken,0x00,sizeof(bAPDUReq->ucTxnToken));//Txn Token
	else
		memcpy(bAPDUReq->ucTxnToken,bTMIn->ucHATOKEN,sizeof(bAPDUReq->ucTxnToken));//Txn Token
	if(bTMIn->ucPurseVersionNumber == LEVEL2)
		memset(bAPDUReq->ucHTAC,0x00,sizeof(bAPDUReq->ucHTAC));//HTAC
	else
		memcpy(bAPDUReq->ucHTAC,bTMIn->ucHTAC,sizeof(bAPDUReq->ucHTAC));//HTAC
	bAPDUReq->ucLCDControlFlag = bGetLCDControlFlag();
	//bAPDUReq->ucLCDControlFlag = bTMIn->ucLCDControlFlag;
	memset(bAPDUReq->ucRFU,0x00,sizeof(bAPDUReq->ucRFU));//保留

	return(sizeof(AuthTxnOnline_APDU_In));
}

int inBuildAuthTxnOnlineCommand2(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
AuthTxnOnline_APDU_In *bAPDUReq = (AuthTxnOnline_APDU_In *)bAPDU_Req;
AuthCardSale_TM_In *bTMIn = (AuthCardSale_TM_In *)bTM_In;

	memset(bAPDUReq->ucExpiryDate,0x00,sizeof(bAPDUReq->ucExpiryDate));//Expiry Date
	memset(bAPDUReq->ucProfileExpiryDate,0x00,sizeof(bAPDUReq->ucProfileExpiryDate));//Profile Expiry Date
	memset(bAPDUReq->ucProfileExpiryDateDOS,0x00,sizeof(bAPDUReq->ucProfileExpiryDateDOS));//Profile Expiry Date(DOS)
	if(bTMIn->ucPurseVersionNumber == MIFARE)
		memset(bAPDUReq->ucTxnToken,0x00,sizeof(bAPDUReq->ucTxnToken));//Txn Token
	else
		memcpy(bAPDUReq->ucTxnToken,bTMIn->ucHATOKEN,sizeof(bAPDUReq->ucTxnToken));//Txn Token
	if(bTMIn->ucPurseVersionNumber == LEVEL2)
		memset(bAPDUReq->ucHTAC,0x00,sizeof(bAPDUReq->ucHTAC));//HTAC
	else
		memcpy(bAPDUReq->ucHTAC,bTMIn->ucHTAC,sizeof(bAPDUReq->ucHTAC));//HTAC
	bAPDUReq->ucLCDControlFlag = bTMIn->ucLCDControlFlag;
	memset(bAPDUReq->ucRFU,0x00,sizeof(bAPDUReq->ucRFU));//保留

	return(sizeof(AuthTxnOnline_APDU_In));
}

int inBuildTxnReqOfflineCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
TxnReqOffline_APDU_In *bAPDUReq = (TxnReqOffline_APDU_In *)bAPDU_Req;
TxnReqOffline_TM_In *bTMIn = (TxnReqOffline_TM_In *)bTM_In;
	
	memset(bAPDU_Req,0x00,sizeof(TxnReqOffline_APDU_In));
	fnASCTOBIN(&bAPDUReq->ucMsgType,bTMIn->anMsgType,2,1,HEX);
	fnASCTOBIN(&bAPDUReq->ucSubType,bTMIn->anSubType,2,1,HEX);
	if(srTxnData.srParameter.chCMASMode == '2')
		memcpy(bAPDUReq->ucTMLocationID,srTxnData.srREQData.bT5503,sizeof(bAPDUReq->ucTMLocationID));//終端機(TM)店號
	else
		memcpy(bAPDUReq->ucTMLocationID,bTMIn->ucTMLocationID,sizeof(bAPDUReq->ucTMLocationID));//終端機(TM)店號
	memcpy(bAPDUReq->ucTMID,bTMIn->ucTMID,sizeof(bAPDUReq->ucTMID));//終端機(TM)機號
	memcpy(bAPDUReq->ucTMTxnDateTime,bTMIn->ucTMTxnDateTime,sizeof(bAPDUReq->ucTMTxnDateTime));//終端機(TM)交易日期時間
	memcpy(bAPDUReq->ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(bAPDUReq->ucTMSerialNumber));//終端機(TM)交易序號
	memcpy(bAPDUReq->ucTMAgentNumber,bTMIn->ucTMAgentNumber,sizeof(bAPDUReq->ucTMAgentNumber));//終端機(TM)收銀員代號
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)
	fnASCTOBIN(bAPDUReq->ucTxnAmt,bTMIn->anTxnAmt,8,3,DECIMAL);//交易金額
	bAPDUReq->ucReadPurseFlag = bTMIn->ucReadPurseFlag;
	//bAPDUReq->ucReadPurseFlag = 0x00;//需重新Read Basic Data
	if(srTxnData.srIngData.lnECCAmt1 + srTxnData.srIngData.lnECCAmt2 + srTxnData.srIngData.lnECCAmt3 + srTxnData.srIngData.lnECCAmt4 + srTxnData.srIngData.lnECCAmt5 > 0L)
	{
		memcpy(bAPDUReq->ucRefundFee,&srTxnData.srIngData.lnECCAmt1,sizeof(bAPDUReq->ucRefundFee));
		memcpy(bAPDUReq->ucBrokenFee,&srTxnData.srIngData.lnECCAmt2,sizeof(bAPDUReq->ucBrokenFee));
		memcpy(bAPDUReq->ucCustomerFee,&srTxnData.srIngData.lnECCAmt3,sizeof(bAPDUReq->ucCustomerFee));
	}
	else
	{
		memset(bAPDUReq->ucRefundFee,0xFF,sizeof(bAPDUReq->ucRefundFee));//Refund Fee
		memset(bAPDUReq->ucBrokenFee,0xFF,sizeof(bAPDUReq->ucBrokenFee));//Broken Fee
		memset(bAPDUReq->ucCustomerFee,0xFF,sizeof(bAPDUReq->ucCustomerFee));//Customer Fee
	}
	if(bAPDUReq->ucMsgType == 0x01)
		bAPDUReq->ucLCDControlFlag = 0x01;//LCD Control Flag
	else
		bAPDUReq->ucLCDControlFlag = 0x00;//LCD Control Flag

	if(srTxnData.srREQData.chOneDayQuotaFlag == '1')
		bAPDUReq->OneDayQuotaFlagForTxnOffline = TRUE;
	if(srTxnData.srREQData.chOnceQuotaFlag == '1')
		bAPDUReq->OnceQuotaFlagForTxnOffline = TRUE;

	bAPDUReq->ucSPID = 0x00;

	if(srTxnData.srParameter.chCMASMode == '2')
		fnASCTOBIN(bAPDUReq->ucCPUSPID,(BYTE *)srTxnData.srREQData.bT5503,strlen((char *)srTxnData.srREQData.bT5503),sizeof(bAPDUReq->ucCPUSPID),DECIMAL);//新服務業者代碼
	if(srTxnData.srIngData.fTMRetryFlag == TRUE)
		bAPDUReq->RetryFlag = TRUE;

	memset(bAPDUReq->ucRFU,0x00,sizeof(bAPDUReq->ucRFU));//保留

	memcpy(stStore_DataInfo_t.ucTMLocationID,bTMIn->ucTMLocationID,sizeof(stStore_DataInfo_t.ucTMLocationID));
	memcpy(stStore_DataInfo_t.ucTMID,bTMIn->ucTMID,sizeof(stStore_DataInfo_t.ucTMID));
	memcpy(stStore_DataInfo_t.ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(stStore_DataInfo_t.ucTMSerialNumber));
	memcpy(stStore_DataInfo_t.ucTMAgentNumber,bTMIn->ucTMAgentNumber,sizeof(stStore_DataInfo_t.ucTMAgentNumber));

	return(sizeof(TxnReqOffline_APDU_In));
}

int inBuildTxnReqOfflineCommand2(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
TxnReqOffline2_APDU_In *bAPDUReq = (TxnReqOffline2_APDU_In *)bAPDU_Req;
TxnReqOffline_TM_In *bTMIn = (TxnReqOffline_TM_In *)bTM_In;
	
	memset(bAPDU_Req,0x00,sizeof(TxnReqOffline_APDU_In));
	fnASCTOBIN(&bAPDUReq->ucMsgType,bTMIn->anMsgType,2,1,HEX);
	fnASCTOBIN(&bAPDUReq->ucSubType,bTMIn->anSubType,2,1,HEX);
	if(srTxnData.srParameter.chCMASMode == '2')
		memcpy(bAPDUReq->ucTMLocationID,srTxnData.srREQData.bT5503,sizeof(bAPDUReq->ucTMLocationID));//終端機(TM)店號
	else
		memcpy(bAPDUReq->ucTMLocationID,bTMIn->ucTMLocationID,sizeof(bAPDUReq->ucTMLocationID));//終端機(TM)店號
	memcpy(bAPDUReq->ucTMID,bTMIn->ucTMID,sizeof(bAPDUReq->ucTMID));//終端機(TM)機號
	memcpy(bAPDUReq->ucTMTxnDateTime,bTMIn->ucTMTxnDateTime,sizeof(bAPDUReq->ucTMTxnDateTime));//終端機(TM)交易日期時間
	memcpy(bAPDUReq->ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(bAPDUReq->ucTMSerialNumber));//終端機(TM)交易序號
	memcpy(bAPDUReq->ucTMAgentNumber,bTMIn->ucTMAgentNumber,sizeof(bAPDUReq->ucTMAgentNumber));//終端機(TM)收銀員代號
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)
	fnASCTOBIN(bAPDUReq->ucTxnAmt,bTMIn->anTxnAmt,8,3,DECIMAL);//交易金額
	bAPDUReq->ucReadPurseFlag = bTMIn->ucReadPurseFlag;
	//bAPDUReq->ucReadPurseFlag = 0x00;//需重新Read Basic Data
	if(srTxnData.srIngData.lnECCAmt1 + srTxnData.srIngData.lnECCAmt2 + srTxnData.srIngData.lnECCAmt3 + srTxnData.srIngData.lnECCAmt4 + srTxnData.srIngData.lnECCAmt5 > 0L)
	{
		memcpy(bAPDUReq->ucRefundFee, &srTxnData.srIngData.lnECCAmt1, sizeof(bAPDUReq->ucRefundFee));
		memcpy(bAPDUReq->ucBrokenFee, &srTxnData.srIngData.lnECCAmt2, sizeof(bAPDUReq->ucBrokenFee));
		memcpy(bAPDUReq->ucCustomerFee, &srTxnData.srIngData.lnECCAmt3, sizeof(bAPDUReq->ucCustomerFee));
		memcpy(bAPDUReq->ucLiquorTxnAmt, &srTxnData.srIngData.lnECCAmt4, sizeof(bAPDUReq->ucLiquorTxnAmt));
		memcpy(bAPDUReq->ucTobaccoTxnAmt, &srTxnData.srIngData.lnECCAmt5, sizeof(bAPDUReq->ucTobaccoTxnAmt));
	}
	else
	{
		memset(bAPDUReq->ucRefundFee,0xFF,sizeof(bAPDUReq->ucRefundFee));//Refund Fee
		memset(bAPDUReq->ucBrokenFee,0xFF,sizeof(bAPDUReq->ucBrokenFee));//Broken Fee
		memset(bAPDUReq->ucCustomerFee,0xFF,sizeof(bAPDUReq->ucCustomerFee));//Customer Fee
		memset(bAPDUReq->ucLiquorTxnAmt, 0xFF, sizeof(bAPDUReq->ucLiquorTxnAmt));
		memset(bAPDUReq->ucTobaccoTxnAmt, 0xFF, sizeof(bAPDUReq->ucTobaccoTxnAmt));
	}
	if(bAPDUReq->ucMsgType == 0x01)
		bAPDUReq->ucLCDControlFlag = 0x01;//LCD Control Flag
	else
		bAPDUReq->ucLCDControlFlag = 0x00;//LCD Control Flag

	if(srTxnData.srREQData.chOneDayQuotaFlag == '1')
		bAPDUReq->OneDayQuotaFlagForTxnOffline = TRUE;
	if(srTxnData.srREQData.chOnceQuotaFlag == '1')
		bAPDUReq->OnceQuotaFlagForTxnOffline = TRUE;

	bAPDUReq->ucSPID = 0x00;

	if(srTxnData.srParameter.chCMASMode == '2')
		fnASCTOBIN(bAPDUReq->ucCPUSPID,(BYTE *)srTxnData.srREQData.bT5503,strlen((char *)srTxnData.srREQData.bT5503),sizeof(bAPDUReq->ucCPUSPID),DECIMAL);//新服務業者代碼
	if(srTxnData.srIngData.fTMRetryFlag == TRUE)
		bAPDUReq->RetryFlag = TRUE;

	memset(bAPDUReq->ucNewExpiryDate, 0x00, sizeof(bAPDUReq->ucNewExpiryDate));
	memset(bAPDUReq->ucNewProfileExpiryDate1, 0x00, sizeof(bAPDUReq->ucNewProfileExpiryDate1));
	memset(bAPDUReq->ucNewProfileExpiryDate2, 0x00, sizeof(bAPDUReq->ucNewProfileExpiryDate2));
	memset(bAPDUReq->ucRFU, 0x00,sizeof(bAPDUReq->ucRFU));//保留
	memset(bAPDUReq->ucRFU2, 0x00, sizeof(bAPDUReq->ucRFU2));

	memcpy(stStore_DataInfo_t.ucTMLocationID,bTMIn->ucTMLocationID,sizeof(stStore_DataInfo_t.ucTMLocationID));
	memcpy(stStore_DataInfo_t.ucTMID,bTMIn->ucTMID,sizeof(stStore_DataInfo_t.ucTMID));
	memcpy(stStore_DataInfo_t.ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(stStore_DataInfo_t.ucTMSerialNumber));
	memcpy(stStore_DataInfo_t.ucTMAgentNumber,bTMIn->ucTMAgentNumber,sizeof(stStore_DataInfo_t.ucTMAgentNumber));

	return(sizeof(TxnReqOffline2_APDU_In));
}


int inBuildAuthTxnOfflineCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
AuthTxnOffline_APDU_In *bAPDUReq = (AuthTxnOffline_APDU_In *)bAPDU_Req;
AuthTxnOffline_TM_In *bTMIn = (AuthTxnOffline_TM_In *)bTM_In;

	if(bTMIn->ucPurseVersionNumber == MIFARE)
		memcpy(bAPDUReq->ucHVCrypto_HTAC,bTMIn->ucHTAC,sizeof(bTMIn->ucHTAC));
	else
		memcpy(bAPDUReq->ucHVCrypto_HTAC,bTMIn->ucHVCrypto,sizeof(bTMIn->ucHVCrypto));
	bAPDUReq->ucLCDControlFlag = bGetLCDControlFlag();
	//bAPDUReq->ucLCDControlFlag = bTMIn->ucLCDControlFlag;
	memset(bAPDUReq->ucRFU,0x00,sizeof(bAPDUReq->ucRFU));

	return(sizeof(AuthTxnOffline_APDU_In));
}

int inBuildLockCardCommand1(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
LockCard_APDU_In_1 *bAPDUReq = (LockCard_APDU_In_1 *)bAPDU_Req;
LockCard_TM_In_1 *bTMIn = (LockCard_TM_In_1 *)bTM_In;

	memcpy(bAPDUReq->ucCardID_4,bTMIn->ucCardID_4,sizeof(bAPDUReq->ucCardID_4));//Card ID
	bAPDUReq->ucLockReason = 0x01;//Lock Reason

	return(sizeof(LockCard_APDU_In_1));
}

int inBuildLockCardCommand2(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
LockCard_APDU_In_2 *bAPDUReq = (LockCard_APDU_In_2 *)bAPDU_Req;
LockCard_TM_In_2 *bTMIn = (LockCard_TM_In_2 *)bTM_In;

	bAPDUReq->ucMsgType = 0x22;//Msg Type
	bAPDUReq->ucSubType = 0x00;//Sub Type
	memcpy(bAPDUReq->ucCardID,bTMIn->ucCardID,sizeof(bAPDUReq->ucCardID));//Card ID
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)
	bAPDUReq->ucLockReason = 0x01;//Lock Reason

	return(sizeof(LockCard_APDU_In_2));
}

int inBuildReadDongleDeductCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
	ReadDongleDeduct_APDU_In *bAPDUReq = (ReadDongleDeduct_APDU_In *)bAPDU_Req;
	ReadDongleDeduct_TM_In *bTMIn = (ReadDongleDeduct_TM_In *)bTM_In;

	fnASCTOBIN(bAPDUReq->ucLogSeqNo,bTMIn->anLogSeqNo,5,2,DECIMAL);//讀取第幾筆Log

	return(sizeof(ReadDongleDeduct_APDU_In));
}

int inBuildSetValueCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
	SetValue_APDU_In *bAPDUReq = (SetValue_APDU_In *)bAPDU_Req;
	SetValue_TM_In *bTMIn = (SetValue_TM_In *)bTM_In;

	fnASCTOBIN(&bAPDUReq->ucMsgType,bTMIn->anMsgType,2,1,DECIMAL);
	memcpy(bAPDUReq->ucTMLocationID_6,bTMIn->ucTMLocationID_6,sizeof(bAPDUReq->ucTMLocationID_6));//終端機(TM)店號
	memcpy(bAPDUReq->ucTMID,bTMIn->ucTMID,sizeof(bAPDUReq->ucTMID));//終端機(TM)機號
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)

	memcpy(stStore_DataInfo_t.ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(stStore_DataInfo_t.ucTMSerialNumber));
	memcpy(stStore_DataInfo_t.ucTMAgentNumber,bTMIn->ucTMAgentNumber,sizeof(stStore_DataInfo_t.ucTMAgentNumber));
	memcpy(stStore_DataInfo_t.ucTMTxnDateTime,bTMIn->ucTMTxnDateTime,sizeof(stStore_DataInfo_t.ucTMTxnDateTime));
	memcpy(stStore_DataInfo_t.ucTxnDateTime,bAPDUReq->ucTxnDateTime,sizeof(stStore_DataInfo_t.ucTxnDateTime));

	return(sizeof(SetValue_APDU_In));
}

int inBuildAuthSetValueCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
	BYTE bDate[20];
	AuthSetValue_APDU_In *bAPDUReq = (AuthSetValue_APDU_In *)bAPDU_Req;
	AuthSetValue_TM_In *bTMIn = (AuthSetValue_TM_In *)bTM_In;

	memcpy(bAPDUReq->ucExpiryDate,bTMIn->ucExpiryDate,sizeof(bAPDUReq->ucExpiryDate));
	memcpy(bAPDUReq->ucHTAC,bTMIn->ucHTAC,sizeof(bAPDUReq->ucHTAC));
	//UnixToDateTime(bAPDUReq->ucExpiryDate,bDate,14);
	UnixToDateTime((BYTE*)bAPDUReq->ucExpiryDate,(BYTE*)bDate,14);
	fngetDosDateCnt(bDate,bAPDUReq->ucExpiryDateDOS);//Profile Expiry Date(DOS)

	return(sizeof(AuthSetValue_APDU_In));
}

int inBuildTxRefundCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
	TxRefund_APDU_In *bAPDUReq = (TxRefund_APDU_In *)bAPDU_Req;
	TxRefund_TM_In *bTMIn = (TxRefund_TM_In *)bTM_In;

	fnASCTOBIN(&bAPDUReq->ucMsgType,bTMIn->anMsgType,2,1,HEX);
	fnASCTOBIN(&bAPDUReq->ucSubType,bTMIn->anSubType,2,1,HEX);
	memcpy(bAPDUReq->ucTMLocationID_6,bTMIn->ucTMLocationID_6,sizeof(bAPDUReq->ucTMLocationID_6));//終端機(TM)店號
	memcpy(bAPDUReq->ucTMID,bTMIn->ucTMID,sizeof(bAPDUReq->ucTMID));//終端機(TM)機號
	memcpy(bAPDUReq->ucTMTxnDateTime,bTMIn->ucTMTxnDateTime,sizeof(bAPDUReq->ucTMTxnDateTime));//終端機(TM)交易日期時間
	memcpy(bAPDUReq->ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(bAPDUReq->ucTMSerialNumber));//終端機(TM)交易序號
	memcpy(bAPDUReq->ucTMAgentNumber,bTMIn->ucTMAgentNumber,sizeof(bAPDUReq->ucTMAgentNumber));//終端機(TM)收銀員代號
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)
	fnASCTOBIN(bAPDUReq->ucTxnAmt_2,bTMIn->anTxnAmt_5,5,2,DECIMAL);//交易金額
	memcpy(bAPDUReq->ucTMTxnDateTime2,bTMIn->ucTMTxnDateTime2,sizeof(bAPDUReq->ucTMTxnDateTime2));//終端機(TM)交易日期時間
	memcpy(bAPDUReq->ucTMSerialNumber2,bTMIn->ucTMSerialNumber2,sizeof(bAPDUReq->ucTMSerialNumber2));//終端機(TM)交易序號
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime2,bTMIn->ucTMTxnDateTime2); //交易日期時間(UnixDateTime)

	memcpy(stStore_DataInfo_t.ucTxnDateTime,bAPDUReq->ucTxnDateTime,sizeof(stStore_DataInfo_t.ucTxnDateTime));//Txn Date Time
	memcpy(stStore_DataInfo_t.ucTxnDateTime2,bAPDUReq->ucTxnDateTime2,sizeof(stStore_DataInfo_t.ucTxnDateTime2));//Txn Date Time

	return(sizeof(TxRefund_APDU_In));
}

int inBuildAuthTxRefundCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
	AuthTxRefund_APDU_In *bAPDUReq = (AuthTxRefund_APDU_In *)bAPDU_Req;
	AuthTxRefund_TM_In *bTMIn = (AuthTxRefund_TM_In *)bTM_In;

	memset(bAPDUReq->ucRFU4,0x00,sizeof(bAPDUReq->ucRFU4));//RFU4
	memcpy(bAPDUReq->ucHTAC,bTMIn->ucHTAC,sizeof(bAPDUReq->ucHTAC));//HTAC
	memset(bAPDUReq->ucRFU2,0x00,sizeof(bAPDUReq->ucRFU2));//RFU2
	bAPDUReq->ucLCDControlFlag = bTMIn->ucLCDControlFlag;//LCD Control Flag

	return(sizeof(AuthTxRefund_APDU_In));
}

int inBuildAddValueCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
	AddValue_APDU_In *bAPDUReq = (AddValue_APDU_In *)bAPDU_Req;
	AddValue_TM_In *bTMIn = (AddValue_TM_In *)bTM_In;

	fnASCTOBIN(&bAPDUReq->ucMsgType,bTMIn->anMsgType,2,1,HEX);
	fnASCTOBIN(&bAPDUReq->ucSubType,bTMIn->anSubType,2,1,HEX);
	memcpy(bAPDUReq->ucTMLocationID_6,bTMIn->ucTMLocationID_6,sizeof(bAPDUReq->ucTMLocationID_6));//終端機(TM)店號
	memcpy(bAPDUReq->ucTMID,bTMIn->ucTMID,sizeof(bAPDUReq->ucTMID));//終端機(TM)機號
	memcpy(bAPDUReq->ucTMTxnDateTime,bTMIn->ucTMTxnDateTime,sizeof(bAPDUReq->ucTMTxnDateTime));//終端機(TM)交易日期時間
	memcpy(bAPDUReq->ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(bAPDUReq->ucTMSerialNumber));//終端機(TM)交易序號
	memcpy(bAPDUReq->ucTMAgentNumber,bTMIn->ucTMAgentNumber,sizeof(bAPDUReq->ucTMAgentNumber));//終端機(TM)收銀員代號
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)
	fnASCTOBIN(bAPDUReq->ucTxnAmt_2,bTMIn->anTxnAmt_5,5,2,DECIMAL);//交易金額

	memcpy(stStore_DataInfo_t.ucTxnDateTime,bAPDUReq->ucTxnDateTime,sizeof(stStore_DataInfo_t.ucTxnDateTime));//Txn Date Time

	return(sizeof(AddValue_APDU_In));
}

int inBuildAuthAddValueCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
AuthAddValue_APDU_In *bAPDUReq = (AuthAddValue_APDU_In *)bAPDU_Req;
AuthAddValue_TM_In *bTMIn = (AuthAddValue_TM_In *)bTM_In;

	memset(bAPDUReq->ucExpiryDate,0x00,sizeof(bAPDUReq->ucExpiryDate));//Expiry Date
	memcpy(bAPDUReq->ucHTAC,bTMIn->ucHTAC,sizeof(bAPDUReq->ucHTAC));//HTAC
	memset(bAPDUReq->ucProfileExpiryDateDOS,0x00,sizeof(bAPDUReq->ucProfileExpiryDateDOS));//Profile Expiry Date DOS
	bAPDUReq->ucLCDControlFlag = 0x01;//LCD Control Flag

	return(sizeof(AuthAddValue_APDU_In));
}

int inBuildTxAuthAddValueCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
AuthAddValue_APDU_In *bAPDUReq = (AuthAddValue_APDU_In *)bAPDU_Req;
TxAuthAddValue_TM_In *bTMIn = (TxAuthAddValue_TM_In *)bTM_In;

	memset(bAPDUReq->ucExpiryDate,0x00,sizeof(bAPDUReq->ucExpiryDate));//Expiry Date
	memcpy(bAPDUReq->ucHTAC,bTMIn->ucHTAC,sizeof(bAPDUReq->ucHTAC));//HTAC
	memset(bAPDUReq->ucProfileExpiryDateDOS,0x00,sizeof(bAPDUReq->ucProfileExpiryDateDOS));//Profile Expiry Date DOS
	bAPDUReq->ucLCDControlFlag = bTMIn->ucLCDControlFlag;//LCD Control Flag

	return(sizeof(AuthAddValue_APDU_In));
}

int inBuildAutoloadEnableCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
AutoloadEnable_APDU_In *bAPDUReq = (AutoloadEnable_APDU_In *)bAPDU_Req;
AutoloadEnable_TM_In *bTMIn = (AutoloadEnable_TM_In *)bTM_In;

	fnASCTOBIN(&bAPDUReq->ucMsgType,bTMIn->anMsgType,2,1,DECIMAL);
	fnASCTOBIN(&bAPDUReq->ucSubType,bTMIn->anSubType,2,1,HEX);
	memcpy(bAPDUReq->ucTMLocationID_6,bTMIn->ucTMLocationID_6,sizeof(bAPDUReq->ucTMLocationID_6));//終端機(TM)店號
	memcpy(bAPDUReq->ucTMID,bTMIn->ucTMID,sizeof(bAPDUReq->ucTMID));//終端機(TM)機號
	memcpy(bAPDUReq->ucTMTxnDateTime,bTMIn->ucTMTxnDateTime,sizeof(bAPDUReq->ucTMTxnDateTime));//終端機(TM)交易日期時間
	memcpy(bAPDUReq->ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(bAPDUReq->ucTMSerialNumber));//終端機(TM)交易序號
	memcpy(bAPDUReq->ucTMAgentNumber,bTMIn->ucTMAgentNumber,sizeof(bAPDUReq->ucTMAgentNumber));//終端機(TM)收銀員代號
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)

	memcpy(stStore_DataInfo_t.ucTxnDateTime,bAPDUReq->ucTxnDateTime,sizeof(stStore_DataInfo_t.ucTxnDateTime));//Txn Date Time

	return(sizeof(AutoloadEnable_APDU_In));
}

int inBuildAuthAutoloadEnableCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
AuthAutoloadEnable_APDU_In *bAPDUReq = (AuthAutoloadEnable_APDU_In *)bAPDU_Req;
AuthAutoloadEnable_TM_In *bTMIn = (AuthAutoloadEnable_TM_In *)bTM_In;

	memcpy(bAPDUReq->ucHTAC,bTMIn->ucHTAC,sizeof(bAPDUReq->ucHTAC));//HTAC
	bAPDUReq->ucLCDControlFlag = bTMIn->ucLCDControlFlag;//LCD Control Flag

	return(sizeof(AuthAutoloadEnable_APDU_In));
}

int inBuildReadCardDeductCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
ReadCardDeduct_APDU_In *bAPDUReq = (ReadCardDeduct_APDU_In *)bAPDU_Req;
ReadCardDeduct_TM_In *bTMIn = (ReadCardDeduct_TM_In *)bTM_In;

	bAPDUReq->ucLogIndex = bTMIn->ucLogIndex;//讀取第幾組Log

	return(sizeof(ReadCardDeduct_APDU_In));
}

int inBuildSetValueCommand2(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
SetValue2_APDU_In *bAPDUReq = (SetValue2_APDU_In *)bAPDU_Req;
SetValue2_TM_In *bTMIn = (SetValue2_TM_In *)bTM_In;

	fnASCTOBIN(&bAPDUReq->ucMsgType,bTMIn->anMsgType,2,1,DECIMAL);
	fnASCTOBIN(&bAPDUReq->ucSubType,bTMIn->anSubType,2,1,DECIMAL);
	memcpy(bAPDUReq->ucTMLocationID,bTMIn->ucTMLocationID,sizeof(bAPDUReq->ucTMLocationID));//終端機(TM)店號
	memcpy(bAPDUReq->ucTMID,bTMIn->ucTMID,sizeof(bAPDUReq->ucTMID));//終端機(TM)機號
	memcpy(bAPDUReq->ucTMTxnDateTime,bTMIn->ucTMTxnDateTime,sizeof(bAPDUReq->ucTMTxnDateTime));//終端機(TM)交易日期時間
	memcpy(bAPDUReq->ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(bAPDUReq->ucTMSerialNumber));//終端機(TM)交易序號
	memcpy(bAPDUReq->ucTMAgentNumber,bTMIn->ucTMAgentNumber,sizeof(bAPDUReq->ucTMAgentNumber));//終端機(TM)收銀員代號
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)
	memset(bAPDUReq->ucTxnAmt,0x00,sizeof(bAPDUReq->ucRFU));//交易金額
	bAPDUReq->ucReadPurseFlag = bTMIn->ucReadPurseFlag;
	memset(bAPDUReq->ucRefundFee,0x00,sizeof(bAPDUReq->ucRefundFee));//Refund Fee
	memset(bAPDUReq->ucBrokenFee,0x00,sizeof(bAPDUReq->ucBrokenFee));//Broken Fee
	bAPDUReq->ucLCDControlFlag = 0x01;//LCD Control Flag
	memset(bAPDUReq->ucRFU,0x00,sizeof(bAPDUReq->ucRFU));//保留

	memcpy(stStore_DataInfo_t.ucTMLocationID,bTMIn->ucTMLocationID,sizeof(stStore_DataInfo_t.ucTMLocationID));
	memcpy(stStore_DataInfo_t.ucTMID,bTMIn->ucTMID,sizeof(stStore_DataInfo_t.ucTMID));
	memcpy(stStore_DataInfo_t.ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(stStore_DataInfo_t.ucTMSerialNumber));
	memcpy(stStore_DataInfo_t.ucTMAgentNumber,bTMIn->ucTMAgentNumber,sizeof(stStore_DataInfo_t.ucTMAgentNumber));

	return(sizeof(SetValue2_APDU_In));
}

int inBuildAuthSetValueCommand2(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
BYTE bDate[20];
AuthSetValue2_APDU_In *bAPDUReq = (AuthSetValue2_APDU_In *)bAPDU_Req;
AuthSetValue2_TM_In *bTMIn = (AuthSetValue2_TM_In *)bTM_In;

	memcpy(bAPDUReq->ucExpiryDate,bTMIn->ucExpiryDate,sizeof(bTMIn->ucExpiryDate));
	memcpy(bAPDUReq->ucProfileExpiryDate,bTMIn->ucProfileExpiryDate,sizeof(bTMIn->ucProfileExpiryDate));
	UnixToDateTime((BYTE*)bAPDUReq->ucProfileExpiryDate,(BYTE*)bDate,14);
	fngetDosDateCnt(bDate,bAPDUReq->ucProfileExpiryDateDOS);//Profile Expiry Date(DOS)
	memcpy(bAPDUReq->ucTxnToken,bTMIn->ucHVCrypto,sizeof(bTMIn->ucHVCrypto));
	memcpy(bAPDUReq->ucHTAC,bTMIn->ucHTAC,sizeof(bTMIn->ucHTAC));
	bAPDUReq->ucLCDControlFlag = bTMIn->ucLCDControlFlag;
	memset(bAPDUReq->ucRFU,0x00,sizeof(bAPDUReq->ucRFU));

	return(sizeof(AuthSetValue2_APDU_In));
}

int inBuildTxRefundCommand2(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
TxRefund_APDU_In *bAPDUReq = (TxRefund_APDU_In *)bAPDU_Req;
TxRefund2_TM_In *bTMIn = (TxRefund2_TM_In *)bTM_In;

	fnASCTOBIN(&bAPDUReq->ucMsgType,bTMIn->anMsgType,2,1,HEX);
	fnASCTOBIN(&bAPDUReq->ucSubType,bTMIn->anSubType,2,1,HEX);
	memcpy(bAPDUReq->ucTMLocationID_6,&bTMIn->ucTMLocationID[4],sizeof(bAPDUReq->ucTMLocationID_6));//終端機(TM)店號
	memcpy(bAPDUReq->ucTMID,bTMIn->ucTMID,sizeof(bAPDUReq->ucTMID));//終端機(TM)機號
	memcpy(bAPDUReq->ucTMTxnDateTime,bTMIn->ucTMTxnDateTime,sizeof(bAPDUReq->ucTMTxnDateTime));//終端機(TM)交易日期時間
	memcpy(bAPDUReq->ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(bAPDUReq->ucTMSerialNumber));//終端機(TM)交易序號
	memcpy(bAPDUReq->ucTMAgentNumber,bTMIn->ucTMAgentNumber,sizeof(bAPDUReq->ucTMAgentNumber));//終端機(TM)收銀員代號
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)
	fnASCTOBIN(bAPDUReq->ucTxnAmt_2,&bTMIn->anTxnAmt[3],5,2,DECIMAL);//交易金額
	memcpy(bAPDUReq->ucTMTxnDateTime2,bTMIn->ucTMTxnDateTime2,sizeof(bAPDUReq->ucTMTxnDateTime2));//終端機(TM)交易日期時間
	memcpy(bAPDUReq->ucTMSerialNumber2,bTMIn->ucTMSerialNumber2,sizeof(bAPDUReq->ucTMSerialNumber2));//終端機(TM)交易序號
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime2,bTMIn->ucTMTxnDateTime2); //交易日期時間(UnixDateTime)

	memcpy(stStore_DataInfo_t.ucTxnDateTime,bAPDUReq->ucTxnDateTime,sizeof(stStore_DataInfo_t.ucTxnDateTime));//Txn Date Time
	memcpy(stStore_DataInfo_t.ucTxnDateTime2,bAPDUReq->ucTxnDateTime2,sizeof(stStore_DataInfo_t.ucTxnDateTime2));//Txn Date Time

	return(sizeof(TxRefund_APDU_In));
}

int inBuildAuthTxRefundCommand2(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
AuthTxRefund_APDU_In *bAPDUReq = (AuthTxRefund_APDU_In *)bAPDU_Req;
AuthTxnOnline_TM_In *bTMIn = (AuthTxnOnline_TM_In *)bTM_In;

	memset(bAPDUReq->ucRFU4,0x00,sizeof(bAPDUReq->ucRFU4));//RFU4
	memcpy(bAPDUReq->ucHTAC,bTMIn->ucHTAC,sizeof(bAPDUReq->ucHTAC));//HTAC
	memset(bAPDUReq->ucRFU2,0x00,sizeof(bAPDUReq->ucRFU2));//RFU2
	bAPDUReq->ucLCDControlFlag = bTMIn->ucLCDControlFlag;//LCD Control Flag

	return(sizeof(AuthTxRefund_APDU_In));
}

int inBuildAutoLoadEnableCommand2(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
TxnReqOnline_APDU_In *bAPDUReq = (TxnReqOnline_APDU_In *)bAPDU_Req;
AutoloadEnable2_TM_In *bTMIn = (AutoloadEnable2_TM_In *)bTM_In;

	fnASCTOBIN(&bAPDUReq->ucMsgType,bTMIn->anMsgType,2,1,DECIMAL);
	fnASCTOBIN(&bAPDUReq->ucSubType,bTMIn->anSubType,2,1,HEX);
	memcpy(bAPDUReq->ucTMLocationID,bTMIn->ucTMLocationID,sizeof(bAPDUReq->ucTMLocationID));//終端機(TM)店號
	memcpy(bAPDUReq->ucTMID,bTMIn->ucTMID,sizeof(bAPDUReq->ucTMID));//終端機(TM)機號
	memcpy(bAPDUReq->ucTMTxnDateTime,bTMIn->ucTMTxnDateTime,sizeof(bAPDUReq->ucTMTxnDateTime));//終端機(TM)交易日期時間
	memcpy(bAPDUReq->ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(bAPDUReq->ucTMSerialNumber));//終端機(TM)交易序號
	memcpy(bAPDUReq->ucTMAgentNumber,bTMIn->ucTMAgentNumber,sizeof(bAPDUReq->ucTMAgentNumber));//終端機(TM)收銀員代號
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)
	fnASCTOBIN(bAPDUReq->ucTxnAmt,bTMIn->anTxnAmt,8,3,DECIMAL);//交易金額
	bAPDUReq->ucReadPurseFlag = bTMIn->ucReadPurseFlag;
	//bAPDUReq->ucReadPurseFlag = 0x00;//需重新Read Basic Data
	memset(bAPDUReq->ucRFU,0x00,sizeof(bAPDUReq->ucRFU));//保留

	memcpy(stStore_DataInfo_t.ucTMLocationID,bTMIn->ucTMLocationID,sizeof(stStore_DataInfo_t.ucTMLocationID));
	memcpy(stStore_DataInfo_t.ucTMID,bTMIn->ucTMID,sizeof(stStore_DataInfo_t.ucTMID));
	memcpy(stStore_DataInfo_t.ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(stStore_DataInfo_t.ucTMSerialNumber));
	memcpy(stStore_DataInfo_t.ucTMAgentNumber,bTMIn->ucTMAgentNumber,sizeof(stStore_DataInfo_t.ucTMAgentNumber));

	return(sizeof(TxnReqOnline_APDU_In));
}

int inBuildAuthAutoLoadEnableCommand2(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
BYTE bDate[20];
AuthTxnOnline_APDU_In *bAPDUReq = (AuthTxnOnline_APDU_In *)bAPDU_Req;
AuthTxnOnline_TM_In *bTMIn = (AuthTxnOnline_TM_In *)bTM_In;

	memcpy(bAPDUReq->ucExpiryDate,bTMIn->ucExpiryDate,sizeof(bAPDUReq->ucExpiryDate));//Expiry Date
	memcpy(bAPDUReq->ucProfileExpiryDate,bTMIn->ucProfileExpiryDate,sizeof(bAPDUReq->ucProfileExpiryDate));//Profile Expiry Date
	UnixToDateTime((BYTE*)bAPDUReq->ucProfileExpiryDate,(BYTE*)bDate,14);
	fngetDosDateCnt(bDate,bAPDUReq->ucProfileExpiryDateDOS);//Profile Expiry Date(DOS)
	if(bTMIn->ucPurseVersionNumber == MIFARE)
		memset(bAPDUReq->ucTxnToken,0x00,sizeof(bAPDUReq->ucTxnToken));//Txn Token
	else
		memcpy(bAPDUReq->ucTxnToken,bTMIn->ucHATOKEN,sizeof(bAPDUReq->ucTxnToken));//Txn Token
	if(bTMIn->ucPurseVersionNumber == LEVEL2)
		memset(bAPDUReq->ucHTAC,0x00,sizeof(bAPDUReq->ucHTAC));//HTAC
	else
		memcpy(bAPDUReq->ucHTAC,bTMIn->ucHTAC,sizeof(bAPDUReq->ucHTAC));//HTAC
	bAPDUReq->ucLCDControlFlag = bGetLCDControlFlag();
	//bAPDUReq->ucLCDControlFlag = bTMIn->ucLCDControlFlag;
	memset(bAPDUReq->ucRFU,0x00,sizeof(bAPDUReq->ucRFU));//保留

	return(sizeof(AuthTxnOnline_APDU_In));
}

int inBuildAuthAutoLoadCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
BYTE bDate[20];
AuthTxnOnline_APDU_In *bAPDUReq = (AuthTxnOnline_APDU_In *)bAPDU_Req;
AuthAutoload_TM_In *bTMIn = (AuthAutoload_TM_In *)bTM_In;

	memcpy(bAPDUReq->ucExpiryDate,bTMIn->ucExpiryDate,sizeof(bAPDUReq->ucExpiryDate));//Expiry Date
	memcpy(bAPDUReq->ucProfileExpiryDate,bTMIn->ucProfileExpiryDate,sizeof(bAPDUReq->ucProfileExpiryDate));//Profile Expiry Date
	UnixToDateTime((BYTE*)bAPDUReq->ucProfileExpiryDate,(BYTE*)bDate,14);
	fngetDosDateCnt(bDate,bAPDUReq->ucProfileExpiryDateDOS);//Profile Expiry Date(DOS)
	if(bTMIn->ucPurseVersionNumber == MIFARE)
		memset(bAPDUReq->ucTxnToken,0x00,sizeof(bAPDUReq->ucTxnToken));//Txn Token
	else
		memcpy(bAPDUReq->ucTxnToken,bTMIn->ucHATOKEN,sizeof(bAPDUReq->ucTxnToken));//Txn Token
	if(bTMIn->ucPurseVersionNumber == LEVEL2)
		memset(bAPDUReq->ucHTAC,0x00,sizeof(bAPDUReq->ucHTAC));//HTAC
	else
		memcpy(bAPDUReq->ucHTAC,bTMIn->ucHTAC,sizeof(bAPDUReq->ucHTAC));//HTAC
	bAPDUReq->ucLCDControlFlag = bGetLCDControlFlag();
	//bAPDUReq->ucLCDControlFlag = bTMIn->ucLCDControlFlag;
	memset(bAPDUReq->ucRFU,0x00,sizeof(bAPDUReq->ucRFU));//保留

	return(sizeof(AuthTxnOnline_APDU_In));
}

int inBuildReadCardNumberCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
ReadCardNumber_APDU_In *bAPDUReq = (ReadCardNumber_APDU_In *)bAPDU_Req;
ReadCardNumber_TM_In *bTMIn = (ReadCardNumber_TM_In *)bTM_In;

	fnASCTOBIN(bAPDUReq->ucTxnAmt,bTMIn->anTxnAmt,8,3,DECIMAL);//交易金額
	bAPDUReq->ucLCDControlFlag = bTMIn->ucLCDControlFlag;//讀取第幾組Log

	return(sizeof(ReadCardNumber_APDU_In));
}

int inBuildFareSaleCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
FareSale_APDU_In *bAPDUReq = (FareSale_APDU_In *)bAPDU_Req;
FareSale_TM_In *bTMIn = (FareSale_TM_In *)bTM_In;

	fnASCTOBIN(&bAPDUReq->ucFareProductType,bTMIn->anFareProductType,3,1,DECIMAL);
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)
	fnASCTOBIN(bAPDUReq->ucCardNumber,bTMIn->anCardNumber,10,4,DECIMAL);
	fnASCTOBIN(bAPDUReq->ucAgentNumber,bTMIn->anAgentNumber,5,2,DECIMAL);
	fnASCTOBIN(&bAPDUReq->ucGroupsize,bTMIn->anGroupsize,3,1,DECIMAL);
	fnASCTOBIN(&bAPDUReq->ucEntryStation,bTMIn->anEntryStation,3,1,DECIMAL);
	fnASCTOBIN(&bAPDUReq->ucExitStation,bTMIn->anExitStation,3,1,DECIMAL);
	fnASCTOBIN(&bAPDUReq->ucJourneyNumber,bTMIn->anJourneyNumber,3,1,DECIMAL);
	fngetDosDateCnt(bTMIn->anFirstDate,bAPDUReq->ucFirstDate);
	fngetDosDateCnt(bTMIn->anLastDate,bAPDUReq->ucLastDate);

	return(sizeof(FareSale_APDU_In));
}

int inBuildVoidFareCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
VoidFare_APDU_In *bAPDUReq = (VoidFare_APDU_In *)bAPDU_Req;
VoidFare_TM_In *bTMIn = (VoidFare_TM_In *)bTM_In;

	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)
	fnASCTOBIN(bAPDUReq->ucAgentNumber,bTMIn->anAgentNumber,5,2,DECIMAL);

	return(sizeof(VoidFare_APDU_In));
}

int inBuildRefundFareCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
RefundFare_APDU_In *bAPDUReq = (RefundFare_APDU_In *)bAPDU_Req;
RefundFare_TM_In *bTMIn = (RefundFare_TM_In *)bTM_In;

	fnASCTOBIN(&bAPDUReq->ucFareProductType,bTMIn->anFareProductType,3,1,DECIMAL);
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)
	fnASCTOBIN(bAPDUReq->ucCardNumber,bTMIn->anCardNumber,10,4,DECIMAL);

	return(sizeof(RefundFare_APDU_In));
}

int inBuildReadTRTCCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
TRTCRead_APDU_In *bAPDUReq = (TRTCRead_APDU_In *)bAPDU_Req;
TRTCRead_TM_In *bTMIn = (TRTCRead_TM_In *)bTM_In;

	bAPDUReq->ucLCDControlFlag = bTMIn->ucLCDControlFlag;//LCD Control Flag
	memcpy(bAPDUReq->ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(bAPDUReq->ucTMSerialNumber));//終端機(TM)交易序號
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)
	memcpy(bAPDUReq->ucRFU,bTMIn->ucTMSerialNumber,sizeof(bAPDUReq->ucRFU));//RFU

	return(sizeof(TRTCRead_APDU_In));
}

int inBuildTRTCSetValueCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
TRTCSetValue_APDU_In *bAPDUReq = (TRTCSetValue_APDU_In *)bAPDU_Req;
TRTCSetValue_TM_In *bTMIn = (TRTCSetValue_TM_In *)bTM_In;

	bAPDUReq->ucFuncOption = bTMIn->ucFuncOption;//LCD Control Flag
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTxnDateTime); //交易日期時間(UnixDateTime)
	fnPack((char *)bTMIn->ucURTData,sizeof(bTMIn->ucURTData),bAPDUReq->ucURTData);
	fnPack((char *)bTMIn->ucUTRData,sizeof(bTMIn->ucURTData),bAPDUReq->ucUTRData);
	fnPack((char *)bTMIn->ucTRTCData,sizeof(bTMIn->ucURTData),bAPDUReq->ucTRTCData);
	bAPDUReq->ucLCDControlFlag = bTMIn->ucLCDControlFlag;//LCD Control Flag

	return(sizeof(TRTCSetValue_APDU_In));
}

int inBuildSetValueOfflineCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
SetValueOffline_APDU_In *bAPDUReq = (SetValueOffline_APDU_In *)bAPDU_Req;
SetValueOffline_TM_In *bTMIn = (SetValueOffline_TM_In *)bTM_In;

	bAPDUReq->ucMsgType = 0x00;
	bAPDUReq->ucSubType = 0x00;
	memcpy(bAPDUReq->ucTMLocationID,bTMIn->ucTMLocationID,sizeof(bTMIn->ucTMLocationID));
	memcpy(bAPDUReq->ucTMID,bTMIn->ucTMID,sizeof(bTMIn->ucTMID));
	memcpy(bAPDUReq->ucTMTxnDateTime,bTMIn->ucTMTxnDateTime,sizeof(bTMIn->ucTMTxnDateTime));
	memcpy(bAPDUReq->ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(bAPDUReq->ucTMSerialNumber));//終端機(TM)交易序號
	memcpy(bAPDUReq->ucTMAgentNumber,bTMIn->ucTMAgentNumber,sizeof(bAPDUReq->ucTMAgentNumber));//終端機(TM)收銀員代號
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)
	fnASCTOBIN(bAPDUReq->ucTxnAmt,bTMIn->anTxnAmt,8,3,DECIMAL);//交易金額
	bAPDUReq->ucReadPurseFlag = bTMIn->ucReadPurseFlag;
	bAPDUReq->fNormalUpdateFlag = bTMIn->fNormalUpdateFlag;
	fngetUnixTimeCnt(bAPDUReq->ucNewExpiryDate,bTMIn->anNewExpiryDate); //交易日期時間(UnixDateTime)
	fngetUnixTimeCnt(bAPDUReq->ucNewPersonalExpiryDate1,bTMIn->anNewPersonalExpiryDate); //交易日期時間(UnixDateTime)
	fngetDosDateCnt(bTMIn->anNewPersonalExpiryDate,bAPDUReq->ucNewPersonalExpiryDate2);

	bAPDUReq->ucLCDControlFlag = bTMIn->ucLCDControlFlag;//LCD Control Flag

	return(sizeof(SetValueOffline_APDU_In));
}

int inBuildStudentSetValueCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
StudentSetValue_APDU_In *bAPDUReq = (StudentSetValue_APDU_In *)bAPDU_Req;
TxnReqOnline_TM_In *bTMIn = (TxnReqOnline_TM_In *)bTM_In;

	fnASCTOBIN(&bAPDUReq->ucMsgType,bTMIn->anMsgType,2,1,HEX);
	fnASCTOBIN(&bAPDUReq->ucSubType,bTMIn->anSubType,2,1,HEX);
	memcpy(bAPDUReq->ucTMLocationID,bTMIn->ucTMLocationID,sizeof(bTMIn->ucTMLocationID));
	memcpy(bAPDUReq->ucTMID,bTMIn->ucTMID,sizeof(bTMIn->ucTMID));
	memcpy(bAPDUReq->ucTMTxnDateTime,bTMIn->ucTMTxnDateTime,sizeof(bTMIn->ucTMTxnDateTime));
	memcpy(bAPDUReq->ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(bAPDUReq->ucTMSerialNumber));//終端機(TM)交易序號
	memcpy(bAPDUReq->ucTMAgentNumber,bTMIn->ucTMAgentNumber,sizeof(bAPDUReq->ucTMAgentNumber));//終端機(TM)收銀員代號
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)
	fnASCTOBIN(bAPDUReq->ucTxnAmt,bTMIn->anTxnAmt,8,3,DECIMAL);//交易金額
	bAPDUReq->ucReadPurseFlag = bTMIn->ucReadPurseFlag;

	memcpy(stStore_DataInfo_t.ucTMLocationID,bTMIn->ucTMLocationID,sizeof(stStore_DataInfo_t.ucTMLocationID));
	memcpy(stStore_DataInfo_t.ucTMID,bTMIn->ucTMID,sizeof(stStore_DataInfo_t.ucTMID));
	memcpy(stStore_DataInfo_t.ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(stStore_DataInfo_t.ucTMSerialNumber));
	memcpy(stStore_DataInfo_t.ucTMAgentNumber,bTMIn->ucTMAgentNumber,sizeof(stStore_DataInfo_t.ucTMAgentNumber));

	return(sizeof(SetValueOffline_APDU_In));
}

int inBuildAuthStudentSetValueCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
BYTE bDate[20], bHTAC[8], bHCrypto[16], bHToken[16];
AuthStudentSetValue_APDU_In *bAPDUReq = (AuthStudentSetValue_APDU_In *)bAPDU_Req;
AuthStudentSetValue_TM_In *bTMIn = (AuthStudentSetValue_TM_In *)bTM_In;

	if ((srTxnData.srIngData.inTransType == TXN_ECC_EXTEND_VALID) && (srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE))	//@ 20年展期
	{
		int inTagName;
		int i = 0;
		for (i = 1; i < srXML->inXMLElementCnt; i++)
		{
			if(srXML->srXMLElement[i].fNodeFlag == TRUE)
				continue;

			if(srXML->srXMLElement[i].shTagLen <= 0 || srXML->srXMLElement[i].shValueLen <= 0)
				continue;

			if(srXML->srXMLElement[i].chTag[0] == '/' || srXML->srXMLElement[i].chTag[0] != 'T')
				continue;

			inTagName = atoi(&srXML->srXMLElement[i].chTag[1]);

			switch(inTagName)
			{
				case 1403:
					{
						UnionUlong ulDateTime;

						ulDateTime.Value = lnDateTimeToUnix((BYTE *)srXML->srXMLElement[i].chValue,(BYTE *)"000000");
						memcpy(bAPDUReq->ucNewExpiryDate,ulDateTime.Buf,sizeof(bAPDUReq->ucNewExpiryDate));//Expiry Date
					}
					break;
				case 4807:
					{
						BYTE bDate[20];

						fnPack((char *)srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,bAPDUReq->ucNewPersonalExpiryDate);//Profile Expiry Date
						if (memcmp(bAPDUReq->ucNewPersonalExpiryDate, "\x00\x00\x00\x00", 4) == 0)
						{
							bAPDUReq->ucPersonalExpiryDateUpdateFlag = 0x00;
							memset(bAPDUReq->ucNewPersonalExpiryDateDOS, 0x00, sizeof(bAPDUReq->ucNewPersonalExpiryDateDOS));
						}
						else
						{
							bAPDUReq->ucPersonalExpiryDateUpdateFlag = 0x01;
							UnixToDateTime((BYTE*)bAPDUReq->ucNewPersonalExpiryDate,(BYTE*)bDate,14);
							fngetDosDateCnt(bDate,bAPDUReq->ucNewPersonalExpiryDateDOS);//Profile Expiry Date(DOS)
						}
					}
					break;
				case 6401:
					fnPack((char *)srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,bHTAC);
					break;
				case 6407:
					fnPack((char *)srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,bHCrypto);
					break;
				case 6409:
					fnPack((char *)srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,bHToken);
					break;
			}
		}

		if (srTxnData.srIngData.ucPurseVersionNumber == MIFARE)
		{
			memset(bAPDUReq->ucHVCrypto, 0x00, sizeof(bAPDUReq->ucHVCrypto));
			memset(bAPDUReq->ucPutDataMAC, 0x00, sizeof(bAPDUReq->ucPutDataMAC));
		}
		else
		{
			memcpy((unsigned char *)bAPDUReq->ucHVCrypto, bHCrypto, sizeof(bAPDUReq->ucHVCrypto));
			memcpy((unsigned char *)bAPDUReq->ucPutDataMAC, bHToken, sizeof(bAPDUReq->ucPutDataMAC));
		}

		if (srTxnData.srIngData.ucPurseVersionNumber == LEVEL2)
		{
			memset(bAPDUReq->ucHTAC, 0x00, sizeof(bAPDUReq->ucHTAC));
		}
		else
		{
			memcpy((unsigned char *)bAPDUReq->ucHTAC, bHTAC, sizeof(bAPDUReq->ucHTAC));
		}
		
						
		bAPDUReq->ucLCDControlFlag = bGetLCDControlFlag();
		bAPDUReq->ucEPDUpdateFlag = 0x01;	//@ EPDUpdateFlag = 1 才展期
		return(sizeof(AuthStudentSetValue_APDU_In));
	}

	if (bTMIn->ucCPUCardSettingParameter[1] == 0x31)
		bAPDUReq->ucCardTypeUpdateFlag = 0x01;
	else
		bAPDUReq->ucCardTypeUpdateFlag = 0x00;
	bAPDUReq->ucNewCardType = bTMIn->ucCardType;
	if (bTMIn->ucCPUCardSettingParameter[2] == 0x31)
		bAPDUReq->ucPersonalExpiryDateUpdateFlag = 0x01;
	else
		bAPDUReq->ucPersonalExpiryDateUpdateFlag = 0x00;
	// 更新票卡到期日
	if (bTMIn->ucCPUCardSettingParameter[3] == 0x31)
		bAPDUReq->ucEPDUpdateFlag = 0x01;
	else
		bAPDUReq->ucEPDUpdateFlag = 0x00;

	memcpy(bAPDUReq->ucNewExpiryDate, bTMIn->ucExpiryDate, sizeof(bAPDUReq->ucNewExpiryDate));
	
	memcpy(bAPDUReq->ucNewPersonalExpiryDate,bTMIn->ucProfileExpiryDate,sizeof(bAPDUReq->ucNewPersonalExpiryDate));//Expiry Date
	UnixToDateTime((BYTE*)bAPDUReq->ucNewPersonalExpiryDate,(BYTE*)bDate,14);	
	fngetDosDateCnt(bDate,bAPDUReq->ucNewPersonalExpiryDateDOS);//Profile Expiry Date(DOS)
	
	//memset(bAPDUReq->ucRFU1,0x00,sizeof(bAPDUReq->ucRFU1));//保留
	if(bTMIn->ucPurseVersionNumber == MIFARE)
	{
		memset(bAPDUReq->ucHVCrypto,0x00,sizeof(bAPDUReq->ucHVCrypto));
		memset(bAPDUReq->ucPutDataMAC,0x00,sizeof(bAPDUReq->ucPutDataMAC));
		memcpy(bAPDUReq->ucHTAC,bTMIn->ucHTAC,sizeof(bAPDUReq->ucHTAC));
	}
	else
	{
		memcpy(bAPDUReq->ucHVCrypto,bTMIn->ucCPUHostCryptogram,sizeof(bAPDUReq->ucHVCrypto));
		memcpy(bAPDUReq->ucPutDataMAC,bTMIn->ucHATOKEN,sizeof(bAPDUReq->ucPutDataMAC));
		memset(bAPDUReq->ucHTAC,0x00,sizeof(bAPDUReq->ucHTAC));
	}

	bAPDUReq->ucLCDControlFlag = bTMIn->ucLCDControlFlag;
	memcpy(bAPDUReq->ucExtAddValueTxnAmt,bTMIn->ucTxnAmt,sizeof(bAPDUReq->ucExtAddValueTxnAmt));
	memset(bAPDUReq->ucRFU2,0x00,sizeof(bAPDUReq->ucRFU2));//保留
	return(sizeof(AuthStudentSetValue_APDU_In));
}

//add by bean 20141027 start
int inBuildMMSelectCard2Command(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
MMSelectCard2_APDU_In *bAPDUReq = (MMSelectCard2_APDU_In *)bAPDU_Req;
MMSelectCard2_TM_In *bTMIn = (MMSelectCard2_TM_In *)bTM_In;

	if (bTMIn->anCardCheckFlag1 == 0x31)
		bAPDUReq->ucCardCheckFlag[0] |= 0x01;

	if (bTMIn->anCardCheckFlag2 == 0x31)
		bAPDUReq->ucCardCheckFlag[0] |= 0x02;

	if (bTMIn->anCardCheckFlag3 == 0x31)
		bAPDUReq->ucCardCheckFlag[0] |= 0x04;

	if (bTMIn->anCardCheckFlag4 == 0x31)
		bAPDUReq->ucCardCheckFlag[0] |= 0x08;

	fnASCTOBIN(&bAPDUReq->ucEZCard_SAMSlot_Address,bTMIn->anEZCard_SAMSlot_Address,2,1,HEX);
	fnASCTOBIN(&bAPDUReq->ucIPASS_SAMSlot_Address,bTMIn->anIPASS_SAMSlot_Address,2,1,HEX);
	fnASCTOBIN(&bAPDUReq->ucSAMSlot_Address_RFU1,bTMIn->anSAMSlot_Address_RFU1,2,1,HEX);
	fnASCTOBIN(&bAPDUReq->ucSAMSlot_Address_RFU2,bTMIn->anSAMSlot_Address_RFU2,2,1,HEX);
	fnASCTOBIN(&bAPDUReq->ucSAMSlot_Address_RFU3,bTMIn->anSAMSlot_Address_RFU3,2,1,HEX);

	if (bTMIn->anCardPollingFlag == 0x31)
		bAPDUReq->ucCardPollingMode |= 0x80;

	if (bTMIn->anReaderDisplayMode >= 0x30 && bTMIn->anReaderDisplayMode <= 0x32)
		  bAPDUReq->ucCardPollingMode |= (bTMIn->anReaderDisplayMode - 0x30);

	fnASCTOBIN(&bAPDUReq->ucPollingTime,bTMIn->anPollingTime,2,1,HEX);

	memset(bAPDUReq->ucRFU,0x00,sizeof(bAPDUReq->ucRFU));

	return(sizeof(MMSelectCard2_APDU_In));
}
//add by bean 20141027 end

int inBuildResetOffCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
Reset_APDU_In *bAPDUReq = (Reset_APDU_In *)bAPDU_Req;
Reset_TM_In *bTMIn = (Reset_TM_In *)bTM_In;

	memset(bAPDU_Req,0x00,sizeof(Reset_APDU_In));
	memcpy(bAPDUReq->ucTMLocationID,bTMIn->ucTMLocationID,sizeof(bAPDUReq->ucTMLocationID));//終端機(TM)店號
	memcpy(bAPDUReq->ucTMID,bTMIn->ucTMID,sizeof(bAPDUReq->ucTMID));//終端機(TM)機號
	memcpy(bAPDUReq->ucTMTxnDateTime,bTMIn->ucTMTxnDateTime,sizeof(bAPDUReq->ucTMTxnDateTime));//終端機(TM)交易日期時間
	memcpy(bAPDUReq->ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(bAPDUReq->ucTMSerialNumber));//終端機(TM)交易序號
	memcpy(bAPDUReq->ucTMAgentNumber,bTMIn->ucTMAgentNumber,sizeof(bAPDUReq->ucTMAgentNumber));//終端機(TM)收銀員代號
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)
	//bAPDUReq->ucLocationID = LOCATION_ID;//舊場站代碼
	fnASCTOBIN(&bAPDUReq->ucLocationID,bTMIn->ucLocationID,3,1,DECIMAL);//舊場站代碼
	fnASCTOBIN(bAPDUReq->ucCPULocationID,bTMIn->ucCPULocationID,5,2,DECIMAL);//新場站代碼
	fnASCTOBIN(&bAPDUReq->ucSPID,bTMIn->ucSPID,3,1,DECIMAL);//舊服務業者代碼
	fnASCTOBIN(bAPDUReq->ucCPUSPID,bTMIn->ucCPUSPID,8,3,DECIMAL);//新服務業者代碼
	//if(srTxnData.srParameter.bDeductType == '1')
	bAPDUReq->ucRFU1[0] = 0x08;
	//else //if(srTxnData.srParameter.bDeductType == '2')
	//	bAPDUReq->ucRFU1[0] = 0x73;
	memcpy(&bAPDUReq->ucRFU1[1],"\xB8\x0B\xE8\x03",4);
	bAPDUReq->ucSAMSlot[0] = bTMIn->ucSAMSlot[0];//SAM卡位置
	if(srTxnData.srParameter.chAccFreeRidesMode == '1')
		bAPDUReq->AccumulatedFreeRidesMode = 1;
	else
		bAPDUReq->AccumulatedFreeRidesMode = 0;

	if(srTxnData.srParameter.chReaderUartDebug == '1')
		bAPDUReq->ecc_debug_flag = 1;
	else
		bAPDUReq->ecc_debug_flag = 0;

	return(sizeof(Reset_APDU_In));
}

int inBuildAuthAccuntLinkCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
	BYTE bDate[20];
	AuthTxnOnline_APDU_In *bAPDUReq = (AuthTxnOnline_APDU_In *)bAPDU_Req;
	AuthAccuntLink_TM_In *bTMIn = (AuthAccuntLink_TM_In *)bTM_In;

	memcpy(bAPDUReq->ucExpiryDate,bTMIn->ucExpiryDate,sizeof(bAPDUReq->ucExpiryDate));//Expiry Date
	memcpy(bAPDUReq->ucProfileExpiryDate,bTMIn->ucProfileExpiryDate,sizeof(bAPDUReq->ucProfileExpiryDate));//Profile Expiry Date
	UnixToDateTime((BYTE*)bAPDUReq->ucProfileExpiryDate,(BYTE*)bDate,14);
	fngetDosDateCnt(bDate,bAPDUReq->ucProfileExpiryDateDOS);//Profile Expiry Date(DOS)
	//if(bTMIn->ucPurseVersionNumber == MIFARE)
	//	memset(bAPDUReq->ucTxnToken,0x00,sizeof(bAPDUReq->ucTxnToken));//Txn Token
	//else
		memcpy(bAPDUReq->ucTxnToken,bTMIn->ucHVCrypto,sizeof(bAPDUReq->ucTxnToken));//Txn Token
	if(bTMIn->ucPurseVersionNumber == LEVEL2)
		memset(bAPDUReq->ucHTAC,0x00,sizeof(bAPDUReq->ucHTAC));//HTAC
	else
		memcpy(bAPDUReq->ucHTAC,bTMIn->ucHTAC,sizeof(bAPDUReq->ucHTAC));//HTAC
	bAPDUReq->ucLCDControlFlag = bGetLCDControlFlag();
	//bAPDUReq->ucLCDControlFlag = bTMIn->ucLCDControlFlag;
	bAPDUReq->ucRFU[0] = bTMIn->ucBankCode;//Bank Code
	memcpy(&bAPDUReq->ucRFU[1],bTMIn->ucAutoLoadAmt_2,sizeof(bTMIn->ucAutoLoadAmt_2));//AutoLoad Amt
	bAPDUReq->ucRFU[3] = bTMIn->ucAutoLoadFlag;//AutoLoad Flag
	memset(&bAPDUReq->ucRFU[4],0x00,1);//保留

	return(sizeof(AuthTxnOnline_APDU_In));
}

int inBuildTaxiReadCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
	TaxiRead_APDU_In *bAPDUReq = (TaxiRead_APDU_In *)bAPDU_Req;
	//Reset_TM_In *bTMIn = (Reset_TM_In *)bTM_In;//僅使用ucTMSerialNumber,ucTMTxnDateTime,所以隨便使用一結構!!
	TaxiRead_TM_In *bTMIn = (TaxiRead_TM_In *)bTM_In;

	memset(bAPDU_Req,0x00,sizeof(TaxiRead_APDU_In));
	bAPDUReq->ucLCDControlFlag = bTMIn->ucLCDControlFlag;
	memcpy(bAPDUReq->ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(bAPDUReq->ucTMSerialNumber));//終端機(TM)交易序號
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)
	memset(bAPDUReq->ucRFU,0x00,sizeof(bAPDUReq->ucRFU));

	return(sizeof(TaxiRead_APDU_In));
}

int inBuildTHSRCReadCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
	THSRCRead_APDU_In *bAPDUReq = (THSRCRead_APDU_In *)bAPDU_Req;
	//Reset_TM_In *bTMIn = (Reset_TM_In *)bTM_In;//僅使用ucTMSerialNumber,ucTMTxnDateTime,所以隨便使用一結構!!
	TaxiRead_TM_In *bTMIn = (TaxiRead_TM_In *)bTM_In;

	memset(bAPDU_Req,0x00,sizeof(THSRCRead_APDU_In));
	bAPDUReq->ucLCDControlFlag = bTMIn->ucLCDControlFlag;
	memcpy(bAPDUReq->ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(bAPDUReq->ucTMSerialNumber));//終端機(TM)交易序號
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)
	memset(bAPDUReq->ucRFU,0x00,sizeof(bAPDUReq->ucRFU));

	return(sizeof(THSRCRead_APDU_In));
}

int inBuildCBikeReadCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
CBikeRead_APDU_In *bAPDUReq = (CBikeRead_APDU_In *)bAPDU_Req;
//Reset_TM_In *bTMIn = (Reset_TM_In *)bTM_In;//僅使用ucTMSerialNumber,ucTMTxnDateTime,所以隨便使用一結構!!
TaxiRead_TM_In *bTMIn = (TaxiRead_TM_In *)bTM_In;

	memset(bAPDU_Req,0x00,sizeof(CBikeRead_APDU_In));
	bAPDUReq->ucLCDControlFlag = bTMIn->ucLCDControlFlag;
	memcpy(bAPDUReq->ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(bAPDUReq->ucTMSerialNumber));//終端機(TM)交易序號
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)
	memset(bAPDUReq->ucRFU,0x00,sizeof(bAPDUReq->ucRFU));

	return(sizeof(CBikeRead_APDU_In));
}

int inBuildEDCAReadCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
EDCARead_APDU_In *bAPDUReq = (EDCARead_APDU_In *)bAPDU_Req;
//Reset_TM_In *bTMIn = (Reset_TM_In *)bTM_In;//僅使用ucTMSerialNumber,ucTMTxnDateTime,所以隨便使用一結構!!
TaxiRead_TM_In *bTMIn = (TaxiRead_TM_In *)bTM_In;

	memset(bAPDU_Req,0x00,sizeof(EDCARead_APDU_In));
	bAPDUReq->ucLCDControlFlag = bTMIn->ucLCDControlFlag;
	memcpy(bAPDUReq->ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(bAPDUReq->ucTMSerialNumber));//終端機(TM)交易序號
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)
	if(srTxnData.srParameter.chReadAccPointsFlag == '1')
		bAPDUReq->ReadAccPointsFlag = 0x01;
	if(srTxnData.srParameter.chReadAccPointsMode == '1')
		bAPDUReq->ReadAccPointsMode = 0x01;
	if(srTxnData.srParameter.chReadDateOfFirstTransFlag == '1')
		bAPDUReq->ReadDateOfFirstTransactionFlag = 0x01;
	memset(bAPDUReq->ucRFU,0x00,sizeof(bAPDUReq->ucRFU));

	return(sizeof(EDCARead_APDU_In));
}

int inBuildFastReadCardAPDU(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
FastReadCard_APDU_In *bAPDUReq = (FastReadCard_APDU_In *)bAPDU_Req;

	memset(bAPDU_Req,0x00,sizeof(FastReadCard_APDU_In));
	bAPDUReq->ucAntennaFlag[0] = 0x00;
	if(srTxnData.srREQData.chReadPVNFlag == '1')
		bAPDUReq->ucPVNReadFlag[0] = 0x01;
	else
		bAPDUReq->ucPVNReadFlag[0] = 0x00;

	return(sizeof(EDCARead_APDU_In));
}

