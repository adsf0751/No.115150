
#ifdef EZ_AIRLINKEDC
#include "_stdAfx.h"
#else
#include "stdAfx.h"
#endif
#if READER_MANUFACTURERS==ANDROID_API
extern STRUCT_XML_DOC srXMLData;
extern STRUCT_TXN_DATA srTxnData;
#endif

int inBuildCMSSignOnAPDU(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
SignOn_APDU_In *bAPDUReq = (SignOn_APDU_In *)bAPDU_Req;
int i,inTagName;

	for(i=1;i<srXML->inXMLElementCnt;i++)
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
			case 4823:
				bAPDUReq->stSAMParameterInfo_t.bOneDayQuotaWrite3 = srXML->srXMLElement[i].chValue[0] - 0x30;
				bAPDUReq->stSAMParameterInfo_t.bOneDayQuotaWrite2 = srXML->srXMLElement[i].chValue[1] - 0x30;
				break;
			case 4824:
				bAPDUReq->stSAMParameterInfo_t.bCPDReadFlag1 = srXML->srXMLElement[i].chValue[0] - 0x30;
				bAPDUReq->stSAMParameterInfo_t.bCPDReadFlag0 = srXML->srXMLElement[i].chValue[1] - 0x30;
				break;
			case 5369:
				bAPDUReq->stSAMParameterInfo_t.bSAMSignOnControlFlag5 = srXML->srXMLElement[i].chValue[0] - 0x30;
				bAPDUReq->stSAMParameterInfo_t.bSAMSignOnControlFlag4 = srXML->srXMLElement[i].chValue[1] - 0x30;
				break;
			case 5303:
				fnPack((char *)srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,&bAPDUReq->ucHashType);
				break;
			case 5306:
				fnPack((char *)srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,bAPDUReq->ucCPUEDC);
				break;
			case 5367:
				fnPack((char *)srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,(BYTE *)&bAPDUReq->stSAMParaSettingInfo_t);
				break;
			case 6002:
				bAPDUReq->stSAMParameterInfo_t.bOneDayQuotaFlag1 = srXML->srXMLElement[i].chValue[0] - 0x30;
				bAPDUReq->stSAMParameterInfo_t.bOneDayQuotaFlag0 = srXML->srXMLElement[i].chValue[1] - 0x30;
				fnPack((char *)&srXML->srXMLElement[i].chValue[2],4,bAPDUReq->stSAMParameterInfo_t.ucOneDayQuota);
				bAPDUReq->stSAMParameterInfo_t.bOnceQuotaFlag2 = srXML->srXMLElement[i].chValue[7] - 0x30;
				fnPack((char *)&srXML->srXMLElement[i].chValue[8],4,bAPDUReq->stSAMParameterInfo_t.ucOnceQuota);
				bAPDUReq->stSAMParameterInfo_t.bCheckEVFlag6 = srXML->srXMLElement[i].chValue[13] - 0x30;
				bAPDUReq->stSAMParameterInfo_t.ucAddQuotaFlag = srXML->srXMLElement[i].chValue[15] - 0x30;
				fnPack((char *)&srXML->srXMLElement[i].chValue[16],6,bAPDUReq->stSAMParameterInfo_t.ucAddQuota);
				bAPDUReq->stSAMParameterInfo_t.bCheckDeductFlag3 = srXML->srXMLElement[i].chValue[23] - 0x30;
				fnPack((char *)&srXML->srXMLElement[i].chValue[24],4,bAPDUReq->stSAMParameterInfo_t.ucCheckDeductValue);
				bAPDUReq->stSAMParameterInfo_t.bDeductLimitFlag7 = srXML->srXMLElement[i].chValue[29] - 0x30;
				break;
			case 6401:
				fnPack((char *)srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,bAPDUReq->ucHTAC);
				break;
			case 6409:
				fnPack((char *)srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,bAPDUReq->ucHATOKEN);
				break;
		}
	}

	return(sizeof(SignOn_APDU_In));
}

int inBuildCMSAuthTxnOnlineAPDU(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
AuthTxnOnline_APDU_In *bAPDUReq = (AuthTxnOnline_APDU_In *)bAPDU_Req;
int i,inTagName;

	for(i=1;i<srXML->inXMLElementCnt;i++)
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

					//@ 若為不展期設備，不把展期日期帶入 -- CMAS改以後台回覆為主，該Flag改用來展期愛陪卡 Mark By Eric Chan V4003D
					//log_msg(LOG_LEVEL_FLOW,"SetCardValidDate = :%d", srTxnData.srParameter.inSetCardValidDate);
					//if (srTxnData.srParameter.inSetCardValidDate == 0)
					//	ulDateTime.Value = 0;
					//else

					ulDateTime.Value = lnDateTimeToUnix((BYTE *)srXML->srXMLElement[i].chValue,(BYTE *)"000000");
					memcpy(bAPDUReq->ucExpiryDate,ulDateTime.Buf,sizeof(bAPDUReq->ucExpiryDate));//Expiry Date
				}
				break;
			case 4807:
				{
					BYTE bDate[20];

					fnPack((char *)srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,bAPDUReq->ucProfileExpiryDate);//Profile Expiry Date
					UnixToDateTime((BYTE*)bAPDUReq->ucProfileExpiryDate,(BYTE*)bDate,14);
					fngetDosDateCnt(bDate,bAPDUReq->ucProfileExpiryDateDOS);//Profile Expiry Date(DOS)
				}
				/*{
					UnionUlong ulDateTime;

					ulDateTime.Value = lnDateTimeToUnix((BYTE *)srXML->srXMLElement[i].chValue,(BYTE *)"000000");
					memcpy(bAPDUReq->ucProfileExpiryDate,ulDateTime.Buf,sizeof(bAPDUReq->ucProfileExpiryDate));//Profile Expiry Date
					fngetDosDateCnt(bAPDUReq->ucProfileExpiryDateDOS,(BYTE *)srXML->srXMLElement[i].chValue);//Profile Expiry Date(DOS)
				}*/
				break;
			case 6401:
				fnPack((char *)srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,bAPDUReq->ucHTAC);
				break;
			case 6409:
				fnPack((char *)srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,bAPDUReq->ucTxnToken);
				break;
		}
	}

	bAPDUReq->ucLCDControlFlag = bGetLCDControlFlag();

	return(sizeof(AuthTxnOnline_APDU_In));
}

int inBuildCMSAuthTxnOfflineAPDU(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
AuthTxnOffline_APDU_In *bAPDUReq = (AuthTxnOffline_APDU_In *)bAPDU_Req;
int i,inTagName;

	for(i=1;i<srXML->inXMLElementCnt;i++)
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
			case 6401:
			case 6407:
				fnPack((char *)srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,bAPDUReq->ucHVCrypto_HTAC);
				break;
		}
	}

	bAPDUReq->ucLCDControlFlag = bGetLCDControlFlag();

	return(sizeof(AuthTxnOnline_APDU_In));
}

int inBuildCMSTaxiDeductCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
TaxiDeduct_APDU_In *bAPDUReq = (TaxiDeduct_APDU_In *)bAPDU_Req;
TxnReqOnline_TM_In *bTMIn = (TxnReqOnline_TM_In *)bTM_In;//所以隨便使用一結構!!
int i,inTagName;
BYTE bBuf[20];

	fnASCTOBIN(&bAPDUReq->ucMsgType,bTMIn->anMsgType,2,1,HEX);
	fnASCTOBIN(&bAPDUReq->ucSubType,bTMIn->anSubType,2,1,HEX);
	memcpy(bAPDUReq->ucTMLocationID,bTMIn->ucTMLocationID,sizeof(bTMIn->ucTMLocationID));
	memcpy(bAPDUReq->ucTMID,bTMIn->ucTMID,sizeof(bTMIn->ucTMID));
	memcpy(bAPDUReq->ucTMTxnDateTime,bTMIn->ucTMTxnDateTime,sizeof(bTMIn->ucTMTxnDateTime));
	memcpy(bAPDUReq->ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(bAPDUReq->ucTMSerialNumber));//終端機(TM)交易序號
	memcpy(bAPDUReq->ucTMAgentNumber,bTMIn->ucTMAgentNumber,sizeof(bAPDUReq->ucTMAgentNumber));//終端機(TM)收銀員代號
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)
	fnASCTOBIN(bAPDUReq->ucTxnAmt,bTMIn->anTxnAmt,8,3,DECIMAL);//交易金額
	bAPDUReq->ucAutoLoad = 0x00;
	bAPDUReq->ucAccumulatedFreeRidesWriteFlag = 0x00;
	bAPDUReq->ucLCDControlFlag = bGetLCDControlFlag();
	memset(bAPDUReq->ucRFU,0x00,sizeof(bAPDUReq->ucRFU));

	for(i=1;i<srXML->inXMLElementCnt;i++)
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
			case 404:
				memset(bBuf,0x00,sizeof(bBuf));
				if(srXML->srXMLElement[i].shValueLen > 2)
					memcpy(bBuf,srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen - 2);
				else
					memcpy(bBuf,srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen);
				fnASCTOBIN(bAPDUReq->ucPersonalDiscount,bBuf,strlen((char *)bBuf),2,DECIMAL);
				if(memcmp(bAPDUReq->ucPersonalDiscount,"\x00\x00",2))
					bAPDUReq->ucAccumulatedFreeRidesWriteFlag = 0x01;
				break;
			case 4835:
				memset(bBuf,0x00,sizeof(bBuf));
				if(srXML->srXMLElement[i].shValueLen > 2)
					memcpy(bBuf,srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen - 2);
				else
					memcpy(bBuf,srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen);
				fnASCTOBIN(bAPDUReq->ucTransferDiscount,bBuf,strlen((char *)bBuf),2,DECIMAL);
				break;
			case 4836:
				memset(bBuf,0x00,sizeof(bBuf));
				vdPad((char *)srXML->srXMLElement[i].chValue,(char *)bBuf,2,FALSE,'0');//右靠左補0
				fnPack((char *)bBuf,2,&bAPDUReq->ucTXNType);
				break;
			case 4837:
				memset(bBuf,0x00,sizeof(bBuf));
				vdPad((char *)srXML->srXMLElement[i].chValue,(char *)bBuf,2,FALSE,'0');//右靠左補0
				fnPack((char *)bBuf,2,&bAPDUReq->ucTransferGroupCode);
				break;
			case 553309:
				fnASCTOBIN(bAPDUReq->ucAccumulatedFreeRides,(BYTE *)srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,2,DECIMAL);
				if(memcmp(bAPDUReq->ucAccumulatedFreeRides,"\x00\x00",2))
					bAPDUReq->ucAccumulatedFreeRidesWriteFlag = 0x01;
				break;
		}
	}


	return(sizeof(TaxiDeduct_APDU_In));
}

int inBuildCMSAuthSetValueAPDU(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
AuthTxnOnline_APDU_In *bAPDUReq = (AuthTxnOnline_APDU_In *)bAPDU_Req;
int i,inTagName;

	for(i=1;i<srXML->inXMLElementCnt;i++)
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
					memcpy(bAPDUReq->ucExpiryDate,ulDateTime.Buf,sizeof(bAPDUReq->ucExpiryDate));//Expiry Date
				}
				break;
			case 4807:
				{
					BYTE bDate[20];

					fnPack((char *)srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,bAPDUReq->ucProfileExpiryDate);//Profile Expiry Date
					UnixToDateTime((BYTE*)bAPDUReq->ucProfileExpiryDate,(BYTE*)bDate,14);
					fngetDosDateCnt(bDate,bAPDUReq->ucProfileExpiryDateDOS);//Profile Expiry Date(DOS)
				}
				/*{
					UnionUlong ulDateTime;

					ulDateTime.Value = lnDateTimeToUnix((BYTE *)srXML->srXMLElement[i].chValue,(BYTE *)"000000");
					memcpy(bAPDUReq->ucProfileExpiryDate,ulDateTime.Buf,sizeof(bAPDUReq->ucProfileExpiryDate));//Profile Expiry Date
					fngetDosDateCnt(bAPDUReq->ucProfileExpiryDateDOS,(BYTE *)srXML->srXMLElement[i].chValue);//Profile Expiry Date(DOS)
				}*/
				break;
			case 6401:
				fnPack((char *)srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,bAPDUReq->ucHTAC);
				break;
			case 6407:
				fnPack((char *)srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,bAPDUReq->ucTxnToken);
				break;
		}
	}

	bAPDUReq->ucLCDControlFlag = bGetLCDControlFlag();

	return(sizeof(AuthTxnOnline_APDU_In));
}

int inCMSVerifyHCryptCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
VerifyHCrypt_APDU_In *bAPDUReq = (VerifyHCrypt_APDU_In *)bAPDU_Req;
int	i,inTagName,inTagNameTmp = 0;
//int	in6407Start = 0, inCnt = 0;
BYTE bBuf[50];
UnionLong lnAmt;
UnionUlong ulDateTime;

	for(i=1;i<srXML->inXMLElementCnt;i++)
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
			case 211:
				fnPack(srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,bAPDUReq->ucPID);
				break;
			case 300:
				fnPack(srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,bAPDUReq->ucPCode);
				break;
			case 1100:
				fnPack(srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,bAPDUReq->ucTMSerialNumber_3);
				break;
			case 4200:
				fnPack(srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,bAPDUReq->ucMerchantID);
				break;
			case 1107:
			case 400:
			case 438:
				memset(bBuf,0x00,sizeof(bBuf));
				lnAmt.Value = atol(srXML->srXMLElement[i].chValue);
				if(inTagName != 1107)
					lnAmt.Value /= 100;
			  #ifndef ECC_BIG_ENDIAN
				lnAmt.Buf[2] = lnAmt.Buf[2] | (lnAmt.Buf[3] & 0x80);
			  #else
				lnAmt.Buf[1] = lnAmt.Buf[1] | (lnAmt.Buf[0] & 0x80);
			  #endif

				if(inTagName == 400)
					memcpy((char *)bAPDUReq->ucTxnAmt,lnAmt.Buf,3);
				else if(inTagName == 438)
					memcpy((char *)bAPDUReq->ucEV,lnAmt.Buf,3);
				else if(inTagName == 1107)
					memcpy((char *)bAPDUReq->ucCardSN,lnAmt.Buf,3);
				break;
			/*case 438:
				memset(bBuf,0x00,sizeof(bBuf));
				lnAmt.Value = atol(srXML->srXMLElement[i].chValue);
				lnAmt.Value /= 100;
			  #ifndef ECC_BIG_ENDIAN
				lnAmt.Buf[2] = lnAmt.Buf[2] | (lnAmt.Buf[3] & 0x80);
			  #else
				lnAmt.Buf[1] = lnAmt.Buf[1] | (lnAmt.Buf[0] & 0x80);
			  #endif
				memcpy((char *)bAPDUReq->ucEV,lnAmt.Buf,3);
				break;*/
			case 440:
				memset(bBuf,0x00,sizeof(bBuf));
				lnAmt.Value = atol(srXML->srXMLElement[i].chValue);
				lnAmt.Value /= 100;
			  #ifndef ECC_BIG_ENDIAN
				lnAmt.Buf[2] = lnAmt.Buf[2] | (lnAmt.Buf[3] & 0x80);
			  #else
				lnAmt.Buf[1] = lnAmt.Buf[1] | (lnAmt.Buf[0] & 0x80);
			  #endif
				memcpy((char *)bAPDUReq->unEVBeforeTxn,lnAmt.Buf,3);
				break;
			case 1200:
				memset(bBuf,0x00,sizeof(bBuf));
				memcpy(bBuf,srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen);
				break;
			case 1300:
				ulDateTime.Value = lnDateTimeToUnix((BYTE *)srXML->srXMLElement[i].chValue,bBuf);
				memcpy((char *)bAPDUReq->ucTxnDateTime,ulDateTime.Buf,4);
				break;
			case 3800:
				memset((char *)bAPDUReq->ucAuthCode,0x20,sizeof(bAPDUReq->ucAuthCode));
				memcpy((char *)bAPDUReq->ucAuthCode,srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen);
				break;
			case 4110://現在ICER沒給,自己組!!
				memcpy((char *)bAPDUReq->ucCPUDeviceID,gTmpData.ucCPUDeviceID,sizeof(gTmpData.ucCPUDeviceID));
				break;
			case 5503:
				memcpy((char *)bAPDUReq->ucTMLocationID,srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen);
				break;
			case 6407:
				fnPack(srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,bAPDUReq->ucHCrypto);
				break;
			default:
				continue;
		}

		//memcpy((char *)bAPDUReq->ucCPUDeviceID,gTmpData.ucCPUDeviceID,sizeof(gTmpData.ucCPUDeviceID));

		if(inTagNameTmp > inTagName)
		{
			log_msg(LOG_LEVEL_ERROR,"inCMSVerifyHCryptCommand Fail 1,%d,%d",inTagNameTmp,inTagName);
			return ICER_ERROR;
		}

		inTagNameTmp = inTagName;
	}

#if 1
	/*if(in6407Start <= 0 || srXML->srXMLElement[in6407Start].shValueLen != 32)
	{
		log_msg(LOG_LEVEL_ERROR,"inCMSVerifyHCryptCommand Fail 2,%d,%d",in6407Start,srXML->srXMLElement[in6407Start].shValueLen);
		return ICER_ERROR;
	}*/

	return(sizeof(VerifyHCrypt_APDU_In));

#else

	{
		int inRetVal,inCntEnd = 0,inCnt = 0;

		i = 0;
		//T6410
		inRetVal = inXMLSearchValueByTag(srXML,(char *)"TCRYPADATA",&i,&inCntEnd,inCnt);
		if(inRetVal < 0)
		{
			log_msg(LOG_LEVEL_ERROR,"inCMSVerifyHCryptCommand No TCRYPADATA");
			return SUCCESS;
		}
		//memcpy(bAPDU_Req,srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen);
		fnPack(srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,bAPDU_Req);
	}

	fnPack(srXML->srXMLElement[in6407Start].chValue,srXML->srXMLElement[in6407Start].shValueLen,&bAPDU_Req[srXML->srXMLElement[i].shValueLen / 2]);
	log_msg_debug(LOG_LEVEL_ERROR,FALSE,"inCMSVerifyHCryptCommand",29,bAPDU_Req,srXML->srXMLElement[i].shValueLen / 2 + 16);

	return(i * 16 + 16);
#endif

}

int inBuildCMASCBikeDeductCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
CBikeDeduct_APDU_In *bAPDUReq = (CBikeDeduct_APDU_In *)bAPDU_Req;
TxnReqOnline_TM_In *bTMIn = (TxnReqOnline_TM_In *)bTM_In;//所以隨便使用一結構!!
int i,inTagName;
BYTE bBuf[20];
UnionUlong ulDateTime;

	fnASCTOBIN(&bAPDUReq->ucMsgType,bTMIn->anMsgType,2,1,HEX);
	fnASCTOBIN(&bAPDUReq->ucSubType,bTMIn->anSubType,2,1,HEX);
	memcpy(bAPDUReq->ucTMLocationID,bTMIn->ucTMLocationID,sizeof(bTMIn->ucTMLocationID));
	memcpy(bAPDUReq->ucTMID,bTMIn->ucTMID,sizeof(bTMIn->ucTMID));
	memcpy(bAPDUReq->ucTMTxnDateTime,bTMIn->ucTMTxnDateTime,sizeof(bTMIn->ucTMTxnDateTime));
	memcpy(bAPDUReq->ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(bAPDUReq->ucTMSerialNumber));//終端機(TM)交易序號
	memcpy(bAPDUReq->ucTMAgentNumber,bTMIn->ucTMAgentNumber,sizeof(bAPDUReq->ucTMAgentNumber));//終端機(TM)收銀員代號
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)
	fnASCTOBIN(bAPDUReq->ucTxnAmt,bTMIn->anTxnAmt,8,3,DECIMAL);//交易金額
	bAPDUReq->ucAutoLoad = 0x00;
	bAPDUReq->ucAccumulatedFreeRidesWriteFlag = 0x00;
	memset(bAPDUReq->ucRFU,0x00,sizeof(bAPDUReq->ucRFU));
	bAPDUReq->ucLCDControlFlag = bGetLCDControlFlag();
	memset(bAPDUReq->ucAccumulatedFreeRides,0x00,sizeof(bAPDUReq->ucAccumulatedFreeRides));

	for(i=1;i<srXML->inXMLElementCnt;i++)
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
			case 404:
				memset(bBuf,0x00,sizeof(bBuf));
				if(srXML->srXMLElement[i].shValueLen > 2)
					memcpy(bBuf,srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen - 2);
				else
					memcpy(bBuf,srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen);
				fnASCTOBIN(bAPDUReq->ucPersonalDiscount,bBuf,strlen((char *)bBuf),2,DECIMAL);
				if(memcmp(bAPDUReq->ucPersonalDiscount,"\x00\x00",2))
					bAPDUReq->ucAccumulatedFreeRidesWriteFlag = 0x01;
				break;
			case 4835:
				memset(bBuf,0x00,sizeof(bBuf));
				if(srXML->srXMLElement[i].shValueLen > 2)
					memcpy(bBuf,srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen - 2);
				else
					memcpy(bBuf,srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen);
				fnASCTOBIN(bAPDUReq->ucTransferDiscount,bBuf,strlen((char *)bBuf),2,DECIMAL);
				break;
			case 4836:
				memset(bBuf,0x00,sizeof(bBuf));
				vdPad((char *)srXML->srXMLElement[i].chValue,(char *)bBuf,2,FALSE,'0');//右靠左補0
				fnPack((char *)bBuf,2,&bAPDUReq->ucTXNType);
				break;
			case 4837:
				memset(bBuf,0x00,sizeof(bBuf));
				vdPad((char *)srXML->srXMLElement[i].chValue,(char *)bBuf,2,FALSE,'0');//右靠左補0
				fnPack((char *)bBuf,2,&bAPDUReq->ucTransferGroupCode);
				break;
			case 484200:
				srTxnData.srIngData.fCBikeRentalFlag = FALSE;//還車
				if(srXML->srXMLElement[i].chValue[0] == '1')//租車
				{
					bAPDUReq->ucRentalFlag[0] = 0x80;//租車
					srTxnData.srIngData.fCBikeRentalFlag = TRUE;//租車
				}
				else//還車
					bAPDUReq->ucRentalFlag[0] = 0x00;//還車
				break;
			case 484201:
				ulDateTime.Value = lnDateTimeToUnix((BYTE *)&srXML->srXMLElement[i].chValue[0],(BYTE *)&srXML->srXMLElement[i].chValue[8]);
				memcpy(bAPDUReq->ucTransactionDateAndTime,ulDateTime.Buf,sizeof(bAPDUReq->ucTransactionDateAndTime));
				memcpy(srTxnData.srIngData.bCBikeTime,ulDateTime.Buf,sizeof(bAPDUReq->ucTransactionDateAndTime));
				break;
			case 484202:
				memset(bBuf,0x00,sizeof(bBuf));
				ECC_ASCIIToUINT64((BYTE *)srXML->srXMLElement[i].chValue,(unsigned short)srXML->srXMLElement[i].shValueLen,(BYTE *)bBuf);
				memcpy(bAPDUReq->ucCBikePlateNumber,bBuf,sizeof(bAPDUReq->ucCBikePlateNumber));
				break;
			case 484203:
				memset(bBuf,0x00,sizeof(bBuf));
				if(srXML->srXMLElement[i].shValueLen < 2)
					vdPad(srXML->srXMLElement[i].chValue,(char *)bBuf,2,FALSE,'0');
				else
					memcpy(bBuf,srXML->srXMLElement[i].chValue,2);
				fnPack((char *)bBuf,2,bAPDUReq->ucAreaCodeOfMembership);
				break;
		}
	}


	return(sizeof(CBikeDeduct_APDU_In));
}

int inBuildCMASEDCADeductCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
EDCADeduct_APDU_In *bAPDUReq = (EDCADeduct_APDU_In *)bAPDU_Req;
TxnReqOnline_TM_In *bTMIn = (TxnReqOnline_TM_In *)bTM_In;//所以隨便使用一結構!!
int i,inTagName;
BYTE bBuf[20];

	memset(bAPDU_Req,0x00,sizeof(EDCADeduct_APDU_In));
	fnASCTOBIN(&bAPDUReq->ucMsgType,bTMIn->anMsgType,2,1,HEX);
	fnASCTOBIN(&bAPDUReq->ucSubType,bTMIn->anSubType,2,1,HEX);
	if(srTxnData.srParameter.chCMASMode == '2')	//@ 商店模式下，T4200要使用代入的T5503
		memcpy(bAPDUReq->ucTMLocationID,srTxnData.srREQData.bT5503,sizeof(bAPDUReq->ucTMLocationID));//終端機(TM)店號
	else
		memcpy(bAPDUReq->ucTMLocationID,bTMIn->ucTMLocationID,sizeof(bTMIn->ucTMLocationID));
	memcpy(bAPDUReq->ucTMID,bTMIn->ucTMID,sizeof(bTMIn->ucTMID));
	memcpy(bAPDUReq->ucTMTxnDateTime,bTMIn->ucTMTxnDateTime,sizeof(bTMIn->ucTMTxnDateTime));
	memcpy(bAPDUReq->ucTMSerialNumber,bTMIn->ucTMSerialNumber,sizeof(bAPDUReq->ucTMSerialNumber));//終端機(TM)交易序號
	memcpy(bAPDUReq->ucTMAgentNumber,bTMIn->ucTMAgentNumber,sizeof(bAPDUReq->ucTMAgentNumber));//終端機(TM)收銀員代號
	fngetUnixTimeCnt(bAPDUReq->ucTxnDateTime,bTMIn->ucTMTxnDateTime); //交易日期時間(UnixDateTime)
	fnASCTOBIN(bAPDUReq->ucTxnAmt,bTMIn->anTxnAmt,8,3,DECIMAL);//交易金額
	bAPDUReq->ucAutoLoad = 0x00;
	bAPDUReq->WriteAccPointsFlag = 0x00;
	bAPDUReq->WriteDateOfFirstTransactionFlag = 0x00;
	bAPDUReq->ucLCDControlFlag = bGetLCDControlFlag();
	//memset(bAPDUReq->ucRFU,0x00,sizeof(bAPDUReq->ucRFU));

	for(i=1;i<srXML->inXMLElementCnt;i++)
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
			case 404:
				memset(bBuf,0x00,sizeof(bBuf));
				if(srXML->srXMLElement[i].shValueLen > 2)
					memcpy(bBuf,srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen - 2);
				else
					memcpy(bBuf,srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen);
				fnASCTOBIN(bAPDUReq->ucPersonalDiscount,bBuf,strlen((char *)bBuf),2,DECIMAL);
				if(memcmp(bAPDUReq->ucPersonalDiscount,"\x00\x00",2))
					bAPDUReq->WriteAccPointsFlag = 0x01;
				break;
			case 4835:
				memset(bBuf,0x00,sizeof(bBuf));
				if(srXML->srXMLElement[i].shValueLen > 2)
					memcpy(bBuf,srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen - 2);
				else
					memcpy(bBuf,srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen);
				fnASCTOBIN(bAPDUReq->ucTransferDiscount,bBuf,strlen((char *)bBuf),2,DECIMAL);
				break;
			case 4836:
				memset(bBuf,0x00,sizeof(bBuf));
				vdPad((char *)srXML->srXMLElement[i].chValue,(char *)bBuf,2,FALSE,'0');//右靠左補0
				fnPack((char *)bBuf,2,&bAPDUReq->ucTXNType);
				break;
			case 4837:
				memset(bBuf,0x00,sizeof(bBuf));
				vdPad((char *)srXML->srXMLElement[i].chValue,(char *)bBuf,2,FALSE,'0');//右靠左補0
				fnPack((char *)bBuf,2,&bAPDUReq->ucTransferGroupCode);
				break;
			case 4843:
				if(srXML->srXMLElement[i].shValueLen >= 8)
				{
					fngetDosDateCnt((BYTE *)srXML->srXMLElement[i].chValue,bAPDUReq->ucDateOfFirstTransaction);
					bAPDUReq->WriteDateOfFirstTransactionFlag = 0x01;
				}
				break;
			case 553309:
				fnASCTOBIN(bAPDUReq->ucAccumulatedFreeRides,(BYTE *)srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,2,DECIMAL);
				if(memcmp(bAPDUReq->ucAccumulatedFreeRides,"\x00\x00",2))
					bAPDUReq->WriteAccPointsFlag = 0x01;
				break;
		}
	}


	return(sizeof(EDCADeduct_APDU_In));

}

int inBuildICERAuthTxnOnlineAPDU(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{
AuthTxnOnline_APDU_In *bAPDUReq = (AuthTxnOnline_APDU_In *)bAPDU_Req;
int i,inTagName;

	for(i=1;i<srXML->inXMLElementCnt;i++)
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
			/*case 1403:
				{
					UnionUlong ulDateTime;

					ulDateTime.Value = lnDateTimeToUnix((BYTE *)srXML->srXMLElement[i].chValue,(BYTE *)"000000");
					memcpy(bAPDUReq->ucExpiryDate,ulDateTime.Buf,sizeof(bAPDUReq->ucExpiryDate));//Expiry Date
				}
				break;
			case 4807:
				{
					BYTE bDate[20];

					fnPack((char *)srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,bAPDUReq->ucProfileExpiryDate);//Profile Expiry Date
					UnixToDateTime((BYTE*)bAPDUReq->ucProfileExpiryDate,(BYTE*)bDate,14);
					fngetDosDateCnt(bDate,bAPDUReq->ucProfileExpiryDateDOS);//Profile Expiry Date(DOS)
				}
				break;*/
			case 6401:
				fnPack((char *)srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,bAPDUReq->ucHTAC);
				break;
			case 6409:
				fnPack((char *)srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,bAPDUReq->ucTxnToken);
				break;
		}
	}

	bAPDUReq->ucLCDControlFlag = bGetLCDControlFlag();

	return(sizeof(AuthTxnOnline_APDU_In));
}

