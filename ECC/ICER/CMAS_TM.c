
#ifdef EZ_AIRLINKEDC
#include "_stdAfx.h"
#else
#include "stdAfx.h"
#endif
#if READER_MANUFACTURERS==ANDROID_API
extern STRUCT_TXN_DATA srTxnData;
extern STRUCT_XML_DOC srXMLData;
extern STRUCT_TMP_DATA gTmpData;
#endif

void vdGetLocalIP(BYTE *bBuf)
{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
char Name[255];
int inRetVal;
char *IP;

	PHOSTENT hostinfo;
	WSADATA wsaData;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	inRetVal = gethostname(Name, sizeof(Name));
	if(inRetVal == 0)
	{
		if((hostinfo = gethostbyname(Name)) != NULL)
		{
			IP = inet_ntoa (*(struct in_addr *)*hostinfo->h_addr_list);
			if(strlen(IP) > 0)
				memcpy((char *)bBuf,IP,strlen(IP));
		}
		else
			log_msg(LOG_LEVEL_ERROR,"gethostbyname failed with error: %ld",WSAGetLastError());
	}
	else
		log_msg(LOG_LEVEL_ERROR,"gethostname failed with error: %ld",WSAGetLastError());

	WSACleanup();
#else
//sprintf((char *)bBuf,"0.0.0.0");
/*char hname[128];
struct hostent *hostinfo;
struct in_addr in;
struct sockaddr_in local_addr;
char *IP;

    gethostname(hname, sizeof(hname));

    hostinfo = gethostbyname(hname);
    memcpy(&local_addr.sin_addr.s_addr,hostinfo->h_addr,4);
    in.s_addr = local_addr.sin_addr.s_addr;
	IP = inet_ntoa(in);
	if(strlen(IP) > 0)
		memcpy((char *)bBuf,IP,strlen(IP));*/
#endif
}

void vdBuildCMASCardAVR(BYTE *bCardAVRData,int inCardAVRDataLen,STRUCT_XML_DOC *srXML,char *chTagName,int inTagLen,int inNodeType,BOOL fXMLSendFlag)
{
BYTE bInData[45 + 1];

	memset(bInData,0x00,sizeof(bInData));
	memcpy(bInData,bCardAVRData,inCardAVRDataLen);
	vdUnPackToXMLData(bInData,45,srXML,chTagName,inTagLen,inNodeType,fXMLSendFlag);
}

int inBuildCMSResetTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
	BYTE bBuf[200],bTmp[100],T558801[5],T558802[5],T558803[20];
	Reset_APDU_In *DongleIn = (Reset_APDU_In *)Dongle_In;
	Reset_APDU_Out *DongleOut = (Reset_APDU_Out *)Dongle_Out;
	BOOL fXMLSendFlag = TRUE;

	if(inTxnType == DLL_RESET)
	{
		//T0100
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0100,(BYTE *)"0800",fXMLSendFlag);

		//T0300
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"881999",fXMLSendFlag);
	}
	else //if(inTxnType == DLL_MATCH)
	{
		//T0100
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0100,(BYTE *)"0900",fXMLSendFlag);

		//T0300
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"882999",fXMLSendFlag);
	}

	//T1100
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulCMASHostSerialNumber);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1100,bBuf,fXMLSendFlag);

	//T1101
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulTMSerialNumber);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1101,bBuf,fXMLSendFlag);

	//T1300,因為有人日期帶錯,主機回覆正確日期且6308後,沒做此步驟,導致給讀卡機跟給主機的日期不同,造成97.
	memset(bBuf,0x00,sizeof(bBuf));
	memcpy((char *)bBuf,srTxnData.srIngData.chTxDate,sizeof(srTxnData.srIngData.chTxDate) - 1);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1300,bBuf,fXMLSendFlag);

	//T3700
	memset(bBuf,0x00,sizeof(bBuf));
	vdCMASBuildRRNProcess(srXML,srTxnData.srIngData.ulCMASHostSerialNumber,bBuf);

	//T4100
	vdUnPackToXMLData(DongleOut->ucCPUDeviceID,sizeof(DongleOut->ucCPUDeviceID),srXML,(char *)TAG_NAME_4100,strlen(TAG_NAME_4100),VALUE_NAME,fXMLSendFlag);

	//T4101
	vdUnPackToXMLData(DongleOut->ucDeviceID,sizeof(DongleOut->ucDeviceID),srXML,(char *)"T4101",strlen("T4101"),VALUE_NAME,fXMLSendFlag);

	//T4102
	memset(bBuf,0x00,sizeof(bBuf));
	vdGetLocalIP(bBuf);
	inXMLAppendData(srXML,(char *)"T4102",strlen("T4102"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T4104
	vdUnPackToXMLData(DongleOut->ucReaderID,sizeof(DongleOut->ucReaderID),srXML,(char *)TAG_NAME_4104,strlen(TAG_NAME_4104),VALUE_NAME,fXMLSendFlag);

	//T4107
	if(strlen(srTxnData.srParameter.chPOS_ID) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4107,(BYTE *)srTxnData.srParameter.chPOS_ID,fXMLSendFlag);

	//T4108 off
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4108);

	//T5594 off
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_5594);

	//T4117
	if(strlen(srTxnData.srParameter.chPOS_ID2) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4117,(BYTE *)srTxnData.srParameter.chPOS_ID2,fXMLSendFlag);

	//T4200//銀行版-->TM Location ID,一般版-->New SP ID,商店模式-->New SP ID
	if(srTxnData.srParameter.chCMASMode == '1')//銀行併機
	{
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy(bBuf,&DongleIn->ucTMLocationID[2],8);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,bBuf,fXMLSendFlag);
	}
	else//一般版以及商店模式
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(&DongleOut->ucCPUDeviceID[3],3,bBuf,8,FALSE,'0',10);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,bBuf,fXMLSendFlag);
	}

	//T4210
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleIn->ucCPULocationID,2,bBuf,5,TRUE,0x00,10);
	inXMLAppendData(srXML,(char *)"T4210",strlen("T4210"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T4211//銀行版 or 商店版-->New SP ID,一般版-->不須此欄位
	if(srTxnData.srParameter.chCMASMode == '1' || srTxnData.srParameter.chCMASMode == '2')//銀行併機 or 商店模式
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(&DongleOut->ucCPUDeviceID[3],3,bBuf,8,FALSE,'0',10);
		inXMLAppendData(srXML,(char *)"T4211",strlen("T4211"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}

	//T4820
	vdUnPackToXMLData(&DongleOut->ucHostSpecVersionNo,sizeof(DongleOut->ucHostSpecVersionNo),srXML,(char *)"T4820",strlen("T4820"),VALUE_NAME,fXMLSendFlag);

	//T4823
	memset(bBuf,0x00,sizeof(bBuf));
	bBuf[0] = DongleOut->stSAMParameterInfo_t.bOneDayQuotaWrite3 + 0x30;
	bBuf[1] = DongleOut->stSAMParameterInfo_t.bOneDayQuotaWrite2 + 0x30;
	inXMLAppendData(srXML,(char *)"T4823",strlen("T4823"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T4824
	memset(bBuf,0x00,sizeof(bBuf));
	bBuf[0] = DongleOut->stSAMParameterInfo_t.bCPDReadFlag1 + 0x30;
	bBuf[1] = DongleOut->stSAMParameterInfo_t.bCPDReadFlag0 + 0x30;
	inXMLAppendData(srXML,(char *)"T4824",strlen("T4824"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T5301
	vdUnPackToXMLData(&DongleOut->ucSAMKeyVersion,sizeof(DongleOut->ucSAMKeyVersion),srXML,(char *)"T5301",strlen("T5301"),VALUE_NAME,fXMLSendFlag);

	//T5307
	vdUnPackToXMLData(DongleOut->ucRSAM,sizeof(DongleOut->ucRSAM),srXML,(char *)"T5307",strlen("T5307"),VALUE_NAME,fXMLSendFlag);

	//T5308
	vdUnPackToXMLData(DongleOut->ucRHOST,sizeof(DongleOut->ucRHOST),srXML,(char *)"T5308",strlen("T5308"),VALUE_NAME,fXMLSendFlag);

	//T5361
	vdUnPackToXMLData(DongleOut->ucSAMID,sizeof(DongleOut->ucSAMID),srXML,(char *)"T5361",strlen("T5361"),VALUE_NAME,fXMLSendFlag);

	//T5362
	vdUnPackToXMLData(DongleOut->ucSAMSN,sizeof(DongleOut->ucSAMSN),srXML,(char *)"T5362",strlen("T5362"),VALUE_NAME,fXMLSendFlag);

	//T5363
	vdUnPackToXMLData(DongleOut->ucSAMCRN,sizeof(DongleOut->ucSAMCRN),srXML,(char *)"T5363",strlen("T5363"),VALUE_NAME,fXMLSendFlag);

	//T5364
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	memcpy(&bTmp[0],&DongleOut->ucSAMVersion,sizeof(DongleOut->ucSAMVersion));
	memcpy(&bTmp[1],DongleOut->ucSAMUsageControl,sizeof(DongleOut->ucSAMUsageControl));
	memcpy(&bTmp[4],&DongleOut->ucSAMAdminKVN,sizeof(DongleOut->ucSAMAdminKVN));
	memcpy(&bTmp[5],&DongleOut->ucSAMIssuerKVN,sizeof(DongleOut->ucSAMIssuerKVN));
	memcpy(&bTmp[6],DongleOut->ucTagListTable,sizeof(DongleOut->ucTagListTable));
	memcpy(&bTmp[46],DongleOut->ucSAMIssuerSpecData,sizeof(DongleOut->ucSAMIssuerSpecData));
	vdUnPackToXMLData(bTmp,78,srXML,(char *)"T5364",strlen("T5364"),VALUE_NAME,fXMLSendFlag);

	//T5365
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	memcpy(&bTmp[0],DongleOut->ucAuthCreditLimit,sizeof(DongleOut->ucAuthCreditLimit));
	memcpy(&bTmp[3],DongleOut->ucAuthCreditBalance,sizeof(DongleOut->ucAuthCreditBalance));
	memcpy(&bTmp[6],DongleOut->ucAuthCreditCumulative,sizeof(DongleOut->ucAuthCreditCumulative));
	memcpy(&bTmp[9],DongleOut->ucAuthCancelCreditCumulative,sizeof(DongleOut->ucAuthCancelCreditCumulative));
	vdUnPackToXMLData(bTmp,12,srXML,(char *)"T5365",strlen("T5365"),VALUE_NAME,fXMLSendFlag);

	//T5366
	vdUnPackToXMLData(DongleOut->ucSingleCreditTxnAmtLimit,sizeof(DongleOut->ucSingleCreditTxnAmtLimit),srXML,(char *)"T5366",strlen("T5366"),VALUE_NAME,fXMLSendFlag);

	//T5368
	vdUnPackToXMLData(DongleOut->ucSTC,sizeof(DongleOut->ucSTC),srXML,(char *)"T5368",strlen("T5368"),VALUE_NAME,fXMLSendFlag);

	//T5369
	memset(bBuf,0x00,sizeof(bBuf));
	bBuf[0] = DongleOut->stSAMParameterInfo_t.bSAMSignOnControlFlag5 + 0x30;
	bBuf[1] = DongleOut->stSAMParameterInfo_t.bSAMSignOnControlFlag4 + 0x30;
	inXMLAppendData(srXML,(char *)"T5369",strlen("T5369"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T5370
	vdUnPackToXMLData((BYTE *)&DongleOut->stLastSignOnInfo_t,sizeof(DongleOut->stLastSignOnInfo_t),srXML,(char *)"T5370",strlen("T5370"),VALUE_NAME,fXMLSendFlag);

	//T5371
	vdUnPackToXMLData(DongleOut->ucCPUSAMID,sizeof(DongleOut->ucCPUSAMID),srXML,(char *)"T5371",strlen("T5371"),VALUE_NAME,fXMLSendFlag);

	//T5503//銀行版-->TM Location ID,一般版-->TM Location ID,商店模式-->New SP ID
	if(srTxnData.srParameter.chCMASMode == '2')//商店模式
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(&DongleOut->ucCPUDeviceID[3],3,bBuf,10,FALSE,'0',10);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_5503,bBuf,fXMLSendFlag);
	}
	else
	{
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy(bBuf,DongleIn->ucTMLocationID,sizeof(DongleIn->ucTMLocationID));
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_5503,bBuf,fXMLSendFlag);
		//inXMLAppendData(srXML,(char *)TAG_NAME_5503,strlen(TAG_NAME_5503),DongleIn->ucTMLocationID,10,VALUE_NAME,fXMLSendFlag);
	}

	//T5504
	memset(bBuf,0x00,sizeof(bBuf));
	memcpy(bBuf,DongleIn->ucTMID,sizeof(DongleIn->ucTMID));
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5504",bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T5504",strlen("T5504"),DongleIn->ucTMID,2,VALUE_NAME,fXMLSendFlag);

	//T5510
	memset(bBuf,0x00,sizeof(bBuf));
	memcpy(bBuf,DongleIn->ucTMAgentNumber,sizeof(DongleIn->ucTMAgentNumber));
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5510",bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T5510",strlen("T5510"),DongleIn->ucTMAgentNumber,4,VALUE_NAME,fXMLSendFlag);

	//T5588-02 黑名單
	memset(T558801,0x00,sizeof(T558801));
	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5588,(char *)T558801,TRUE,NODE_NO_SAME,TAG_NAME_START,TRUE);
	memcpy(T558801,"02",2);
	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_558801,(char *)T558801,FALSE,NODE_NO_ADD,VALUE_NAME,TRUE);

	memset(bTmp,0x00,sizeof(bTmp));
	memset(T558802,0x00,sizeof(T558802));
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
	memcpy(T558802,"BIG",3);//BIG
#else
	memcpy(T558802,&srTxnData.srParameter.bBLCName[strlen((char *)srTxnData.srParameter.bBLCName) - 3],3);//BIG
#endif
	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_558802,(char *)T558802,FALSE,NODE_NO_ADD,VALUE_NAME,TRUE);

	memset(T558803,0x00,sizeof(T558803));
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
	inGetBLCName(bTmp);
	memcpy(T558803,bTmp,5);
#else
	memcpy(T558803,&srTxnData.srParameter.bBLCName[3],5);
#endif
	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_558803,(char *)T558803,FALSE,NODE_NO_ADD,VALUE_NAME,TRUE);
	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5588,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,TRUE);
	inXMLInsertData(srXML,&srXMLActionData,(char *)"",FALSE);

	//T5588-03 ICERAPI
	memset(T558801,0x00,sizeof(T558801));
	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5588,(char *)T558801,TRUE,NODE_NO_SAME,TAG_NAME_START,TRUE);
	memcpy(T558801,"03",2);
	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_558801,(char *)T558801,FALSE,NODE_NO_ADD,VALUE_NAME,TRUE);

	memset(T558803,0x00,sizeof(T558803));
	memcpy(T558803,API_VERSION_CMAS,strlen(API_VERSION_CMAS));
	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_558803,(char *)T558803,FALSE,NODE_NO_ADD,VALUE_NAME,TRUE);
	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5588,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,TRUE);
	inXMLInsertData(srXML,&srXMLActionData,(char *)"",FALSE);

	//T5588-03 Reader FW Version
	memset(T558801,0x00,sizeof(T558801));
	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5588,(char *)T558801,TRUE,NODE_NO_SAME,TAG_NAME_START,TRUE);
	memcpy(T558801,"05",2);
	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_558801,(char *)T558801,FALSE,NODE_NO_ADD,VALUE_NAME,TRUE);

	memset(T558803,0x00,sizeof(T558803));
	fnUnPack(DongleOut->ucReaderFWVersion,6,T558803);
	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_558803,(char *)T558803,FALSE,NODE_NO_ADD,VALUE_NAME,TRUE);
	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5588,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,TRUE);
	inXMLInsertData(srXML,&srXMLActionData,(char *)"",FALSE);

	//T6000
	vdUnPackToXMLData(DongleOut->ucReaderFWVersion,sizeof(DongleOut->ucReaderFWVersion),srXML,(char *)"T6000",strlen("T6000"),VALUE_NAME,fXMLSendFlag);

	//T6002
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	bTmp[0] = (DongleOut->stSAMParameterInfo_t.bOneDayQuotaFlag1 >> 4) + DongleOut->stSAMParameterInfo_t.bOneDayQuotaFlag0;
	memcpy(&bTmp[1],DongleOut->stSAMParameterInfo_t.ucOneDayQuota,sizeof(DongleOut->stSAMParameterInfo_t.ucOneDayQuota));
	bTmp[3] = DongleOut->stSAMParameterInfo_t.bOnceQuotaFlag2;
	memcpy(&bTmp[4],DongleOut->stSAMParameterInfo_t.ucOnceQuota,sizeof(DongleOut->stSAMParameterInfo_t.ucOnceQuota));
	bTmp[6] = DongleOut->stSAMParameterInfo_t.bCheckEVFlag6;
	bTmp[7] = DongleOut->stSAMParameterInfo_t.ucAddQuotaFlag;
	memcpy(&bTmp[8],DongleOut->stSAMParameterInfo_t.ucAddQuota,sizeof(DongleOut->stSAMParameterInfo_t.ucAddQuota));
	bTmp[11] = DongleOut->stSAMParameterInfo_t.bCheckDeductFlag3;
	memcpy(&bTmp[12],DongleOut->stSAMParameterInfo_t.ucCheckDeductValue,sizeof(DongleOut->stSAMParameterInfo_t.ucCheckDeductValue));
	bTmp[14] = DongleOut->stSAMParameterInfo_t.bDeductLimitFlag7;
	memcpy(&bTmp[15],API_VERSION_R6,4);
	vdUnPackToXMLData(bTmp,24,srXML,(char *)"T6002",strlen("T6002"),VALUE_NAME,fXMLSendFlag);

	//T6003
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	memcpy(&bTmp[0],DongleOut->ucRemainderAddQuota,sizeof(DongleOut->ucRemainderAddQuota));
	memcpy(&bTmp[3],DongleOut->ucDeMAC,sizeof(DongleOut->ucDeMAC));
	memcpy(&bTmp[11],DongleOut->ucCancelCreditQuota,sizeof(DongleOut->ucCancelCreditQuota));
	vdUnPackToXMLData(bTmp,32,srXML,(char *)"T6003",strlen("T6003"),VALUE_NAME,fXMLSendFlag);

	//T6004
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	inGetBLCName(bTmp);
	memcpy(bBuf,bTmp,5);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T6004",bBuf,fXMLSendFlag);

	//T6400
	vdUnPackToXMLData(DongleOut->ucSTAC,sizeof(DongleOut->ucSTAC),srXML,(char *)"T6400",strlen("T6400"),VALUE_NAME,fXMLSendFlag);

	//T6408
	vdUnPackToXMLData(DongleOut->ucSATOKEN,sizeof(DongleOut->ucSATOKEN),srXML,(char *)"T6408",strlen("T6408"),VALUE_NAME,fXMLSendFlag);

	return SUCCESS;
}

int inBuildCMSSignOnTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
//SignOn_APDU_In *DongleIn = (SignOn_APDU_In *)Dongle_In;
SignOn_APDU_Out *DongleOut = (SignOn_APDU_Out *)Dongle_Out;
BOOL fXMLSendFlag = TRUE;
int inCntStart,inCntEnd;
BYTE bBuf[20];

	//TAG_TRANS_XML_HEADER
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_TRANS_XML_HEADER);

	//log_msg(LOG_LEVEL_ERROR,"inBuildCMSSignOnTM 1");
	//TAG_TRANS_HEADER
	if(inXMLSearchValueByTag(srXML,(char *)TAG_TRANS_HEADER,&inCntStart,&inCntEnd,0) >= SUCCESS)
	{
		//log_msg(LOG_LEVEL_ERROR,"inBuildCMSSignOnTM 2");
		srXML->srXMLElement[inCntStart].fXMLSendFlag = TRUE;
		//vdSetConfigFlag(srXML,FALSE,TAG_TRANS_HEADER);
		memcpy(srXML->srXMLElement[inCntStart].chTag,TAG_TRANS_HEADER,strlen(TAG_TRANS_HEADER));
	}
	//log_msg(LOG_LEVEL_ERROR,"inBuildCMSSignOnTM 3");

	//T0100
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0100);

	//T0300
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0300);

	//T1100
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulCMASHostSerialNumber);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1100,bBuf,fXMLSendFlag);

	//T1200
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1200,(BYTE *)srTxnData.srIngData.chTxTime,TRUE);

	//T1300
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_1300);

	//T3700
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_3700);

	//T4100
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4100);

	//T4104
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4104);

	//T4108 off
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4108);

	//T4200
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4200);

	//T4210
	vdSetConfigFlag(srXML,FALSE,(char *)"T4210");

	//T4211

	//T4211//銀行版 or 商店版-->New SP ID,一般版-->不須此欄位
	if(srTxnData.srParameter.chCMASMode == '1' || srTxnData.srParameter.chCMASMode == '2')//銀行併機 or 商店模式
		vdSetConfigFlag(srXML,FALSE,(char *)"T4211");

	//T4825
	vdUnPackToXMLData(&DongleOut->ucCreditBalanceChangeFlag,sizeof(DongleOut->ucCreditBalanceChangeFlag),srXML,(char *)"T4825",strlen("T4825"),VALUE_NAME,fXMLSendFlag);

	//T5501
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_5501);

	//T5503
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_5503);

	//T5504
	vdSetConfigFlag(srXML,FALSE,(char *)"T5504");

	//T5510
	vdSetConfigFlag(srXML,FALSE,(char *)"T5510");

	//T6406
	vdUnPackToXMLData(DongleOut->ucCACrypto,sizeof(DongleOut->ucCACrypto),srXML,(char *)"T6406",strlen("T6406"),VALUE_NAME,fXMLSendFlag);

	return SUCCESS;
}

int inBuildCMSGetCMASModeTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
	
	return SUCCESS;
}

int inBuildCMSTxnReqOnlineTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
BYTE bBuf[200],bTmp[50];
TxnReqOnline_APDU_In *DongleIn = (TxnReqOnline_APDU_In *)Dongle_In;
TxnReqOnline_APDU_Out *DongleOut = (TxnReqOnline_APDU_Out *)Dongle_Out;
BOOL fXMLSendFlag = TRUE;
int inCntStart,inCntEnd;

	//if(DongleOut->ucStatusCode[0] != 0x64 || DongleOut->ucStatusCode[1] != 0x15)
	//	return SUCCESS;

	//TAG_TRANS_XML_HEADER
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_TRANS_XML_HEADER);

	//TAG_TRANS_HEADER
	if(inXMLSearchValueByTag(srXML,(char *)TAG_TRANS_HEADER,&inCntStart,&inCntEnd,0) >= SUCCESS)
	{
		srXML->srXMLElement[inCntStart].fXMLSendFlag = TRUE;
		//vdSetConfigFlag(srXML,FALSE,TAG_TRANS_HEADER);
		memcpy(srXML->srXMLElement[inCntStart].chTag,TAG_TRANS_HEADER,strlen(TAG_TRANS_HEADER));
	}

	//T0100
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0100,(BYTE *)"0100",fXMLSendFlag);

	//T0200
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdUIntToAsc(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),bTmp,17,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,17);
	inXMLAppendData(srXML,(char *)TAG_NAME_0200,strlen(TAG_NAME_0200),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T0211
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucPID,sizeof(DongleOut->ucPID),srXML,(char *)TAG_NAME_0211,strlen(TAG_NAME_0211),VALUE_NAME,fXMLSendFlag);

	//T0212
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%02x",DongleOut->bAutoLoad);
	inXMLAppendData(srXML,(char *)TAG_NAME_0212,strlen(TAG_NAME_0212),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T0213
	vdUnPackToXMLData(&DongleOut->ucCardType,sizeof(DongleOut->ucCardType),srXML,(char *)TAG_NAME_0213,strlen(TAG_NAME_0213),VALUE_NAME,fXMLSendFlag);

	//T0214
	vdUnPackToXMLData(&DongleOut->ucPersonalProfile,sizeof(DongleOut->ucPersonalProfile),srXML,(char *)TAG_NAME_0214,strlen(TAG_NAME_0214),VALUE_NAME,fXMLSendFlag);

	//T0300
	if(inTxnType == DLL_AUTOLOAD || srTxnData.srIngData.inTransType == TXN_ECC_ADD)
	{
		if(srTxnData.srIngData.inTransType == TXN_ECC_ADD)
		{
			if(DongleOut->ucPersonalProfile >= 0x01 && DongleOut->ucPersonalProfile <= 0x03)
				inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"841799",TRUE);
			else
				inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"811799",TRUE);
		}
		else
		{
			//if(DongleOut->ucPersonalProfile >= 0x01 && DongleOut->ucPersonalProfile <= 0x03)
			//	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"845799",TRUE);
			//else
				inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"825799",TRUE);
		}
	}
	else if(inTxnType == DLL_AUTOLOAD_ENABLE)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"814799",TRUE);
	else if(srTxnData.srIngData.inTransType == TXN_ECC_REFUND)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"851999",TRUE);
	else if(srTxnData.srIngData.inTransType == TXN_ECC_VOID)
	{
		if(DongleOut->ucMsgType == 0x0B)//加值
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"811899",TRUE);
		else if(DongleOut->ucMsgType == 0x02 && DongleOut->ucSubType == 0x0A)//購貨
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"823899",TRUE);
	}
	else if (srTxnData.srIngData.inTransType == TXN_ECC_VOID1)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"871899",TRUE);
	else if(srTxnData.srIngData.inTransType == TXN_ECC_ADD4)
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"871799",TRUE);
	else if(srTxnData.srIngData.inTransType == TXN_ECC_PT_ADD) //點數兌換加值
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"705699",TRUE);


	//T0400
	memset(bBuf,0x00,sizeof(bBuf));
	if(inTxnType == DLL_AUTOLOAD && !memcmp(srTxnData.srIngData.chTMProcessCode,"825799",6))
	//if(inTxnType == DLL_AUTOLOAD && memcmp(srTxnData.srIngData.chTMProcessCode,"825799",6))
		memcpy(bBuf,srTxnData.srIngData.anAutoloadAmount,strlen((char *)srTxnData.srIngData.anAutoloadAmount) - 2);
	else
		sprintf((char *)bBuf,"%ld",srTxnData.srIngData.lnECCAmt);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0400,strlen(TAG_NAME_0400),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0400,bBuf,fXMLSendFlag);

	//T0403 因應自動加值改上限由後台決定，傳送交易金額讓後台辨識是否超過日限額、次限額-V4005F
	if (inTxnType == DLL_AUTOLOAD)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		if (srTxnData.srIngData.inTransType == TXN_ECC_AUTOLOAD)    //純自動加值交易T0403要帶0 -V4006C
			memset(bBuf, '0', 3);
		else
			sprintf((char *)bBuf,"%ld",srTxnData.srIngData.lnECCAmt);
		inXMLUpdateData(srXML, (char *)TAG_NAME_4200, (char *)TAG_NAME_0403, bBuf, TRUE);
	}

	//T0409
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdUIntToAsc(DongleOut->ucAutoLoadAmt,sizeof(DongleOut->ucAutoLoadAmt),bTmp,5,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,5);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0409,bBuf,fXMLSendFlag);

	//T0410
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),bTmp,5,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,5);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0410,bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0410,strlen(TAG_NAME_0410),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1100
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulCMASHostSerialNumber);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1100,bBuf,fXMLSendFlag);

	//T1101
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulTMSerialNumber);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1101,bBuf,fXMLSendFlag);

	//T1200	When Retry T1200 Must Changed, by Eric Chan 20190326//Marked by Eric Chan
	//if (srTxnData.srIngData.fTMRetryFlag == TRUE)
	//{
	//	memset(bBuf,0x00,sizeof(bBuf));
	//	memcpy((char *)bBuf,srTxnData.srIngData.chTxTime,sizeof(srTxnData.srIngData.chTxTime) - 1);
	//	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1200,bBuf,fXMLSendFlag);
	//}


	//T1201
	memset(bBuf,0x00,sizeof(bBuf));
	memcpy((char *)bBuf,srTxnData.srIngData.chTxTime,sizeof(srTxnData.srIngData.chTxTime) - 1);
	inXMLAppendData(srXML,(char *)"T1201",strlen("T1201"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1301
	memset(bBuf,0x00,sizeof(bBuf));
	memcpy((char *)bBuf,srTxnData.srIngData.chTxDate,sizeof(srTxnData.srIngData.chTxDate) - 1);
	inXMLAppendData(srXML,(char *)"T1301",strlen("T1301"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1401 二十年展期開關
	int iReaderVer = ((int)(DongleOut->ucReaderFWVersion[0] & 0x0F) * 100) + (int)DongleOut->ucReaderFWVersion[1];
	log_msg(LOG_LEVEL_ERROR,"ReaderFWVersion: %d", iReaderVer);
	if ((srTxnData.srParameter.inSetCardValidDate == 1) && (srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE) && (iReaderVer >= 810))
	{
		inXMLAppendData(srXML,(char *)"T1401",strlen("T1401"), (BYTE *)"1", 1,VALUE_NAME,fXMLSendFlag);
	}

	//T1402
	memset(bBuf,0x00,sizeof(bBuf));
	UnixToDateTime((BYTE*)DongleOut->ucExpiryDate,(BYTE*)bBuf,8);//Expiry Date
	inXMLAppendData(srXML,(char *)TAG_NAME_1402,strlen(TAG_NAME_1402),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T3700
	vdCMASBuildRRNProcess(srXML,srTxnData.srIngData.ulCMASHostSerialNumber,DongleOut->ucTxnSN);

	//T4100
	vdUnPackToXMLData(DongleOut->ucCPUDeviceID,sizeof(DongleOut->ucCPUDeviceID),srXML,(char *)TAG_NAME_4100,strlen(TAG_NAME_4100),VALUE_NAME,fXMLSendFlag);

	//T4101
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(DongleOut->ucDeviceID,sizeof(DongleOut->ucDeviceID),srXML,(char *)"T4101",strlen("T4101"),VALUE_NAME,fXMLSendFlag);

	//T4102
	memset(bBuf,0x00,sizeof(bBuf));
	vdGetLocalIP(bBuf);
	inXMLAppendData(srXML,(char *)"T4102",strlen("T4102"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T4104
	vdUnPackToXMLData(gTmpData.ucReaderID,sizeof(gTmpData.ucReaderID),srXML,(char *)TAG_NAME_4104,strlen(TAG_NAME_4104),VALUE_NAME,fXMLSendFlag);

	//T4107
	if(strlen(srTxnData.srParameter.chPOS_ID) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4107,(BYTE *)srTxnData.srParameter.chPOS_ID,fXMLSendFlag);

	//T4108 off
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4108);

	//T4117
	if(strlen(srTxnData.srParameter.chPOS_ID2) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4117,(BYTE *)srTxnData.srParameter.chPOS_ID2,fXMLSendFlag);

	//T4200//銀行版-->TM Location ID,一般版-->New SP ID,商店模式-->自訂的TM Location ID
	if(srTxnData.srParameter.chCMASMode == '1' || srTxnData.srParameter.chCMASMode == '2')//銀行併機 or 商店模式
	{
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy(bBuf,&DongleIn->ucTMLocationID[2],8);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,bBuf,fXMLSendFlag);
	}
	else//一般版
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(DongleOut->ucCPUSPID,sizeof(DongleOut->ucCPUSPID),bBuf,8,FALSE,'0',10);
		//vdUIntToAsc(&DongleOut->ucCPUDeviceID[3],3,bBuf,8,FALSE,'0',10);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,bBuf,fXMLSendFlag);
		//inXMLAppendData(srXML,(char *)TAG_NAME_4200,strlen(TAG_NAME_4200),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}

	//T4210
	memset(bBuf,0x00,sizeof(bBuf));
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUIntToAsc(&DongleOut->ucLocationID,1,bBuf,3,TRUE,0x00,10);
	else
		vdUIntToAsc(DongleOut->ucCPULocationID,2,bBuf,5,TRUE,0x00,10);
	inXMLAppendData(srXML,(char *)"T4210",strlen("T4210"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T4800
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucPurseVersionNumber,sizeof(DongleOut->ucPurseVersionNumber),srXML,(char *)TAG_NAME_4800,strlen(TAG_NAME_4800),VALUE_NAME,fXMLSendFlag);

	//T4801,33 byte unpack to 66 byte,後補0至90 byte
	vdBuildCMASCardAVR((BYTE *)&DongleOut->stLastCreditTxnLogInfo_t,sizeof(DongleOut->stLastCreditTxnLogInfo_t),srXML,(char *)"T4801",strlen("T4801"),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData((BYTE *)&DongleOut->stLastCreditTxnLogInfo_t,sizeof(DongleOut->stLastCreditTxnLogInfo_t),srXML,(char *)"T4801",strlen("T4801"),VALUE_NAME,fXMLSendFlag);

	//T4802
	vdUnPackToXMLData(&DongleOut->ucIssuerCode,sizeof(DongleOut->ucIssuerCode),srXML,(char *)"T4802",strlen("T4802"),VALUE_NAME,fXMLSendFlag);

	//T4803
	vdUnPackToXMLData(&DongleOut->ucBankCode,sizeof(DongleOut->ucBankCode),srXML,(char *)TAG_NAME_4803,strlen(TAG_NAME_4803),VALUE_NAME,fXMLSendFlag);

	//T4804
	vdUnPackToXMLData(&DongleOut->ucAreaCode,sizeof(DongleOut->ucAreaCode),srXML,(char *)TAG_NAME_4804,strlen(TAG_NAME_4804),VALUE_NAME,fXMLSendFlag);

	//T4805
	vdUnPackToXMLData(DongleOut->ucSubAreaCode,sizeof(DongleOut->ucSubAreaCode),srXML,(char *)TAG_NAME_4805,strlen(TAG_NAME_4805),VALUE_NAME,fXMLSendFlag);

	//T4806
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucProfileExpiryDate,sizeof(DongleOut->ucProfileExpiryDate),srXML,(char *)"T4806",strlen("T4806"),VALUE_NAME,fXMLSendFlag);

	//T4807
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucProfileExpiryDate,sizeof(DongleOut->ucProfileExpiryDate),srXML,(char *)"T4807",strlen("T4807"),VALUE_NAME,fXMLSendFlag);

	//T4808
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
	inXMLAppendData(srXML,(char *)"T4808",strlen("T4808"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),srXML,(char *)"T4808",strlen("T4808"),VALUE_NAME,fXMLSendFlag);

	//T4809
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucTxnMode,sizeof(DongleOut->ucTxnMode),srXML,(char *)"T4809",strlen("T4809"),VALUE_NAME,fXMLSendFlag);

	//T4810
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucTxnQuqlifier,sizeof(DongleOut->ucTxnQuqlifier),srXML,(char *)"T4810",strlen("T4810"),VALUE_NAME,fXMLSendFlag);

	//T4811
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
	inXMLAppendData(srXML,(char *)"T4811",strlen("T4811"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),srXML,(char *)"T4811",strlen("T4811"),VALUE_NAME,fXMLSendFlag);

	//T4812
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucCTC,sizeof(DongleOut->ucCTC),srXML,(char *)"T4812",strlen("T4812"),VALUE_NAME,fXMLSendFlag);

	//T4813
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucLoyaltyCounter,2,bBuf,6,TRUE,0x00,10);
	vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
	inXMLAppendData(srXML,(char *)"T4813",strlen("T4813"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucLoyaltyCounter,sizeof(DongleOut->ucLoyaltyCounter),srXML,(char *)"T4813",strlen("T4813"),VALUE_NAME,fXMLSendFlag);

	//T4814
	if(DongleOut->ucPurseVersionNumber != MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(DongleOut->ucDeposit,3,bBuf,6,TRUE,0x00,10);
		vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
		inXMLAppendData(srXML,(char *)"T4814",strlen("T4814"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
		//vdUnPackToXMLData(DongleOut->ucDeposit,sizeof(DongleOut->ucDeposit),srXML,(char *)TAG_NAME_4814,strlen(TAG_NAME_4814),VALUE_NAME,fXMLSendFlag);
	}

	//T4826
	vdUnPackToXMLData(&DongleOut->ucCardIDLen,sizeof(DongleOut->ucCardIDLen),srXML,(char *)"T4826",strlen("T4826"),VALUE_NAME,fXMLSendFlag);

	//T5301
	vdUnPackToXMLData(&DongleOut->ucSAMKVN,sizeof(DongleOut->ucSAMKVN),srXML,(char *)"T5301",strlen("T5301"),VALUE_NAME,fXMLSendFlag);

	//T5302
	if(DongleOut->ucPurseVersionNumber != MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		sprintf((char *)bBuf,"%02x%02x%02x",DongleOut->ucCPUAdminKeyKVN,DongleOut->ucCreditKeyKVN,DongleOut->ucCPUIssuerKeyKVN);
		inXMLAppendData(srXML,(char *)"T5302",strlen("T5302"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}

	//T5304
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucHostAdminKVN,sizeof(DongleOut->ucHostAdminKVN),srXML,(char *)"T5304",strlen("T5304"),VALUE_NAME,fXMLSendFlag);

	//T5305
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucSignatureKeyKVN,sizeof(DongleOut->ucSignatureKeyKVN),srXML,(char *)"T5305",strlen("T5305"),VALUE_NAME,fXMLSendFlag);

	//T5361
	vdUnPackToXMLData(DongleOut->ucSAMID,sizeof(DongleOut->ucSAMID),srXML,(char *)"T5361",strlen("T5361"),VALUE_NAME,fXMLSendFlag);

	//T5362
	vdUnPackToXMLData(DongleOut->ucSAMSN,sizeof(DongleOut->ucSAMSN),srXML,(char *)"T5362",strlen("T5362"),VALUE_NAME,fXMLSendFlag);

	//T5363
	vdUnPackToXMLData(DongleOut->ucSAMCRN,sizeof(DongleOut->ucSAMCRN),srXML,(char *)"T5363",strlen("T5363"),VALUE_NAME,fXMLSendFlag);

	//T5371
	vdUnPackToXMLData(DongleOut->ucCPUSAMID,sizeof(DongleOut->ucCPUSAMID),srXML,(char *)"T5371",strlen("T5371"),VALUE_NAME,fXMLSendFlag);

	//T5503
	memset(bBuf,0x00,sizeof(bBuf));
	if (srTxnData.srParameter.chCMASMode == '2')
	{
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5503",(BYTE *)srTxnData.srREQData.bT5503,TRUE);
	}
	else
	{
		memcpy(bBuf,DongleIn->ucTMLocationID,sizeof(DongleIn->ucTMLocationID));
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5503",bBuf,fXMLSendFlag);
		//inXMLAppendData(srXML,(char *)"T5503",strlen("T5503"),DongleIn->ucTMLocationID,10,VALUE_NAME,fXMLSendFlag);
	}

	//T5504
	inXMLAppendData(srXML,(char *)"T5504",strlen("T5504"),DongleIn->ucTMID,2,VALUE_NAME,fXMLSendFlag);

	//T5510
	inXMLAppendData(srXML,(char *)"T5510",strlen("T5510"),DongleIn->ucTMAgentNumber,4,VALUE_NAME,fXMLSendFlag);

	//T6000
	vdUnPackToXMLData(DongleOut->ucReaderFWVersion,sizeof(DongleOut->ucReaderFWVersion),srXML,(char *)"T6000",strlen("T6000"),VALUE_NAME,fXMLSendFlag);

	//T6001
	vdUnPackToXMLData((BYTE *)&DongleOut->stReaderAVRInfo_t,83,srXML,(char *)"T6001",strlen("T6001"),VALUE_NAME,fXMLSendFlag);

	//T6004
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	inGetBLCName(bTmp);
	memcpy(bBuf,bTmp,5);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T6004",bBuf,fXMLSendFlag);

	//T6400
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(DongleOut->ucSTAC,sizeof(DongleOut->ucSTAC),srXML,(char *)"T6400",strlen("T6400"),VALUE_NAME,fXMLSendFlag);

	//T6406
	if(DongleOut->ucPurseVersionNumber != MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy((char *)bBuf,DongleOut->ucTxnCrypto,sizeof(DongleOut->ucTxnCrypto));
		vdUnPackToXMLData(bBuf,sizeof(DongleOut->ucTxnCrypto) + 8,srXML,(char *)"T6406",strlen("T6406"),VALUE_NAME,fXMLSendFlag);
	}

	if(DongleOut->ucMsgType == 0x02 && DongleOut->ucSubType == 0x0A)//取消購貨
		memcpy(srTxnData.srIngData.chTMProcessCode,"816100",6);
	else if(DongleOut->ucMsgType == 0x0B/* && DongleOut->ucSubType == 0x0A*/)//取消加值
		memcpy(srTxnData.srIngData.chTMProcessCode,"816101",6);

	return SUCCESS;
}

int inBuildCMSAuthTxnOnlineTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
BYTE bBuf[50];
//AuthTxnOnline_APDU_In *DongleIn = (AuthTxnOnline_APDU_In *)Dongle_In;
AuthTxnOnline_APDU_Out *DongleOut = (AuthTxnOnline_APDU_Out *)Dongle_Out;
TxnReqOnline_APDU_Out *DongleReqOut = (TxnReqOnline_APDU_Out *)Dongle_ReqOut;
BOOL fXMLSendFlag = TRUE;
int inCntStart,inCntEnd;
BYTE bDate[20];

	//TAG_TRANS_XML_HEADER
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_TRANS_XML_HEADER);

	//TAG_TRANS_HEADER
	if(inXMLSearchValueByTag(srXML,(char *)TAG_TRANS_HEADER,&inCntStart,&inCntEnd,0) >= SUCCESS)
	{
		srXML->srXMLElement[inCntStart].fXMLSendFlag = TRUE;
		//vdSetConfigFlag(srXML,FALSE,TAG_TRANS_HEADER);
		memcpy(srXMLData.srXMLElement[inCntStart].chTag,TAG_TRANS_HEADER,strlen(TAG_TRANS_HEADER));
	}
	//T0100
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0100,(BYTE *)"0220",TRUE);

	//T0200
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0200);

	//T0211
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0211);

	//T0213
	vdUnPackToXMLData(&DongleReqOut->ucCardType,sizeof(DongleReqOut->ucCardType),srXML,(char *)TAG_NAME_0213,strlen(TAG_NAME_0213),VALUE_NAME,fXMLSendFlag);

	//T0214
	vdUnPackToXMLData(&DongleReqOut->ucPersonalProfile,sizeof(DongleReqOut->ucPersonalProfile),srXML,(char *)TAG_NAME_0214,strlen(TAG_NAME_0214),VALUE_NAME,fXMLSendFlag);

	//T0300
	if(inTxnType == DLL_AUTOLOAD_AUTH || srTxnData.srIngData.inTransType == TXN_ECC_ADD)
	{
		if(srTxnData.srIngData.inTransType == TXN_ECC_ADD)
		{
			if(DongleReqOut->ucPersonalProfile >= 0x01 && DongleReqOut->ucPersonalProfile <= 0x03)
				inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"841799",TRUE);
			else
				inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"811799",TRUE);
		}
		else
		{
			//if(DongleReqOut->ucPersonalProfile >= 0x01 && DongleReqOut->ucPersonalProfile <= 0x03)
			//	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"845799",TRUE);
			//else
				inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"825799",TRUE);
		}
	}
	else if(inTxnType == DLL_AUTOLOAD_ENABLE_AUTH)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"814799",TRUE);
  	else if(srTxnData.srIngData.inTransType == TXN_ECC_REFUND)
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"851999",TRUE);
	else if(srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"811599",TRUE);
	else if((srTxnData.srIngData.inTransType == TXN_ECC_VOID) || (srTxnData.srIngData.inTransType == TXN_ECC_VOID1))
	{
		if(DongleReqOut->ucMsgType == 0x0B)//加值
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"811899",TRUE);
		else if(DongleReqOut->ucMsgType == 0x02 && DongleReqOut->ucSubType == 0x0A)//購貨
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"823899",TRUE);
	}
	else if(srTxnData.srIngData.inTransType == TXN_ECC_ADD4)
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"871799",TRUE);
	else if(srTxnData.srIngData.inTransType == TXN_ECC_PT_ADD)
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"705699",TRUE);

	//T0400
	memset(bBuf,0x00,sizeof(bBuf));
	if(strlen((char *)srTxnData.srIngData.anAutoloadAmount) > 2)
		memcpy(bBuf,srTxnData.srIngData.anAutoloadAmount,strlen((char *)srTxnData.srIngData.anAutoloadAmount) - 2);
	else
		sprintf((char *)bBuf,"%ld",srTxnData.srIngData.lnECCAmt);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0400,bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0400,strlen(TAG_NAME_0400),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T0404
	if((srTxnData.srIngData.inTransType == TXN_ECC_VOID) || (srTxnData.srIngData.inTransType == TXN_ECC_VOID1))
	{
		if(srTxnData.srREQData.lnT0404 > 0)
		{
			memset(bBuf,0x00,sizeof(bBuf));
			sprintf((char *)bBuf,"%lu",srTxnData.srREQData.lnT0404);
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0404,bBuf,TRUE);
		}
	}

	//T0406 有折扣時，必需帶折扣金額
	if ((srTxnData.srREQData.chDiscountType == 'D') && (srTxnData.srIngData.lnDiscountAmt != 0))
	{
		memset(bBuf, 0x00, sizeof(bBuf));
		sprintf((char *)bBuf, "%lu", srTxnData.srIngData.lnDiscountAmt);
		inXMLUpdateData(srXML, (char *)TAG_NAME_4200, (char *)TAG_NAME_0406, bBuf, TRUE);
	}
	
	//T0408
	memset(bBuf,0x00,sizeof(bBuf));
	vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),bBuf,5,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0408,bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0408,strlen(TAG_NAME_0408),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	srTxnData.srIngData.lnECCEV = lnAmt3ByteToLong(DongleOut->ucEV);

	//T0409
	if(strlen((char *)srTxnData.srIngData.anAutoloadAmount) > 2)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy(bBuf,srTxnData.srIngData.anAutoloadAmount,strlen((char *)srTxnData.srIngData.anAutoloadAmount) - 2);
		//vdUIntToAsc(DongleReqOut->ucAutoLoadAmt,sizeof(DongleReqOut->ucAutoLoadAmt),bBuf,5,TRUE,0x00,10);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0409,bBuf,fXMLSendFlag);
		//inXMLAppendData(srXML,(char *)TAG_NAME_0409,strlen(TAG_NAME_0409),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}

	//T0410
	memset(bBuf,0x00,sizeof(bBuf));
	vdIntToAsc(DongleReqOut->ucEV,sizeof(DongleReqOut->ucEV),bBuf,5,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0410,bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0410,strlen(TAG_NAME_0410),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T0421
	//vdSetConfigFlag(srXML,FALSE,(char *)"T0421");
	if(srTxnData.srIngData.fForeignTxnFlag == TRUE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		sprintf((char *)bBuf,"%ld", srTxnData.srIngData.lnForeignAmt);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T0421",bBuf,fXMLSendFlag);

		//T0422
		vdSetConfigFlag(srXML,FALSE,(char *)"T0422");

		//T0423
		vdSetConfigFlag(srXML,FALSE,(char *)"T0423");

		//T1202
		vdSetConfigFlag(srXML,FALSE,(char *)"T1202");

		//T1302
		vdSetConfigFlag(srXML,FALSE,(char *)"T1302");
	}

	//T1100
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_1100);

	//T1101
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_1101);

	memset(bDate,0x00,sizeof(bDate));
	UnixToDateTime((BYTE*)DongleOut->ucTxnDateTime,(BYTE*)bDate,14);
	//T1200
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1200,&bDate[8],TRUE);

	//T1201
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T1201",&bDate[8],TRUE);
	bDate[8] = 0x00;
	//vdSetConfigFlag(srXML,FALSE,(char *)"T1201");

	//T1300
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1300,bDate,TRUE);
	//vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_1300);

	//T1301
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T1301",bDate,TRUE);
	//vdSetConfigFlag(srXML,FALSE,(char *)"T1301");

	//T3700
	vdCMASBuildRRNProcess(srXML,srTxnData.srIngData.ulCMASHostSerialNumber,DongleReqOut->ucTxnSN);

	//T4100
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4100);

	//T4101
	//if(DongleReqOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(DongleReqOut->ucDeviceID,sizeof(DongleReqOut->ucDeviceID),srXML,(char *)"T4101",strlen("T4101"),VALUE_NAME,fXMLSendFlag);

	//T4104
	memset(bBuf,0x00,sizeof(bBuf));
	if(DongleReqOut->ucPurseVersionNumber == MIFARE)
	{
		fnUnPack(&DongleOut->ucMAC_HCrypto[6],4,bBuf);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4104",bBuf,fXMLSendFlag);
	}
	else
	{
		vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4104);
		//sprintf((char *)bBuf,"00000000");
	}

	//T4107
	if(strlen(srTxnData.srParameter.chPOS_ID) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4107,(BYTE *)srTxnData.srParameter.chPOS_ID,fXMLSendFlag);

	//T4108 off
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4108);

	//T4109 off
	memset(bBuf,0x00,sizeof(bBuf));
	fnBINTODEVASC(DongleReqOut->ucDeviceID,bBuf,9,MIFARE);//n_TXN Device ID
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4109",bBuf,FALSE);

	//T4110 off
	memset(bBuf,0x00,sizeof(bBuf));
	fnBINTODEVASC(DongleReqOut->ucCPUDeviceID,bBuf,16,LEVEL2);//n_TXN Device ID
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4110",bBuf,FALSE);

	//T4117
	//if(strlen(srTxnData.srParameter.chPOS_ID2) > 0)
	//	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4117,(BYTE *)srTxnData.srParameter.chPOS_ID2,fXMLSendFlag);
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4117);

	//T4200
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4200);

	//T4210
	vdSetConfigFlag(srXML,FALSE,(char *)"T4210");

	//T4800
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4800);

	//T4801,33 byte unpack to 66 byte,後補0至90 byte
	vdBuildCMASCardAVR((BYTE *)&DongleReqOut->stLastCreditTxnLogInfo_t,sizeof(DongleReqOut->stLastCreditTxnLogInfo_t),srXML,(char *)"T4801",strlen("T4801"),VALUE_NAME,fXMLSendFlag);

	//T4802
	vdUnPackToXMLData(&DongleReqOut->ucIssuerCode,sizeof(DongleReqOut->ucIssuerCode),srXML,(char *)"T4802",strlen("T4802"),VALUE_NAME,fXMLSendFlag);

	//T4803
	vdUnPackToXMLData(&DongleReqOut->ucBankCode,sizeof(DongleReqOut->ucBankCode),srXML,(char *)TAG_NAME_4803,strlen(TAG_NAME_4803),VALUE_NAME,fXMLSendFlag);

	//T4804
	vdUnPackToXMLData(&DongleReqOut->ucAreaCode,sizeof(DongleReqOut->ucAreaCode),srXML,(char *)TAG_NAME_4804,strlen(TAG_NAME_4804),VALUE_NAME,fXMLSendFlag);

	//T4805
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleReqOut->ucSubAreaCode,sizeof(DongleReqOut->ucSubAreaCode),srXML,(char *)TAG_NAME_4805,strlen(TAG_NAME_4805),VALUE_NAME,fXMLSendFlag);

	//T4808
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4808",bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T4808",strlen("T4808"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),srXML,(char *)"T4808",strlen("T4808"),VALUE_NAME,fXMLSendFlag);

	//T4809
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleReqOut->ucTxnMode,sizeof(DongleReqOut->ucTxnMode),srXML,(char *)"T4809",strlen("T4809"),VALUE_NAME,fXMLSendFlag);

	//T4810
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleReqOut->ucTxnQuqlifier,sizeof(DongleReqOut->ucTxnQuqlifier),srXML,(char *)"T4810",strlen("T4810"),VALUE_NAME,fXMLSendFlag);

	//T4811
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleReqOut->ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4811",bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T4811",strlen("T4811"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleReqOut->ucTxnSN,sizeof(DongleReqOut->ucTxnSN),srXML,(char *)"T4811",strlen("T4811"),VALUE_NAME,fXMLSendFlag);

	//T4812
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleReqOut->ucCTC,sizeof(DongleReqOut->ucCTC),srXML,(char *)"T4812",strlen("T4812"),VALUE_NAME,fXMLSendFlag);

	//T4813
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleReqOut->ucLoyaltyCounter,2,bBuf,6,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4813",bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T4813",strlen("T4813"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleReqOut->ucLoyaltyCounter,sizeof(DongleReqOut->ucLoyaltyCounter),srXML,(char *)"T4813",strlen("T4813"),VALUE_NAME,fXMLSendFlag);

	//T4826
	vdSetConfigFlag(srXML,FALSE,(char *)"T4826");

	//T5301
	vdUnPackToXMLData(&DongleReqOut->ucSAMKVN,sizeof(DongleReqOut->ucSAMKVN),srXML,(char *)"T5301",strlen("T5301"),VALUE_NAME,fXMLSendFlag);

	//T5303
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucMAC_HCrypto[0],1,srXML,(char *)"T5303",strlen("T5303"),VALUE_NAME,fXMLSendFlag);

	//T5304
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucMAC_HCrypto[1],1,srXML,(char *)"T5304",strlen("T5304"),VALUE_NAME,fXMLSendFlag);

	//T5305
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleReqOut->ucSignatureKeyKVN,sizeof(DongleReqOut->ucSignatureKeyKVN),srXML,(char *)"T5305",strlen("T5305"),VALUE_NAME,fXMLSendFlag);

	//T5361
	vdUnPackToXMLData(DongleReqOut->ucSAMID,sizeof(DongleReqOut->ucSAMID),srXML,(char *)"T5361",strlen("T5361"),VALUE_NAME,fXMLSendFlag);

	//T5362
	vdUnPackToXMLData(DongleReqOut->ucSAMSN,sizeof(DongleReqOut->ucSAMSN),srXML,(char *)"T5362",strlen("T5362"),VALUE_NAME,fXMLSendFlag);

	//T5363
	vdUnPackToXMLData(DongleReqOut->ucSAMCRN,sizeof(DongleReqOut->ucSAMCRN),srXML,(char *)"T5363",strlen("T5363"),VALUE_NAME,fXMLSendFlag);

	//T5371
	vdUnPackToXMLData(DongleOut->ucCPUSAMID,sizeof(DongleOut->ucCPUSAMID),srXML,(char *)"T5371",strlen("T5371"),VALUE_NAME,fXMLSendFlag);

	//T5501
	//inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)srTxnData.srIngData.chTxDate,TRUE);
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_5501);

	//T5503
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_5503);

	//T5504
	vdSetConfigFlag(srXML,FALSE,(char *)"T5504");

	//T5510
	vdSetConfigFlag(srXML,FALSE,(char *)"T5510");

#if 1
	//T5533
	if((srTxnData.srIngData.inTransType == TXN_ECC_VOID) || (srTxnData.srIngData.inTransType == TXN_ECC_VOID1))//取消
	{
		if(DongleReqOut->ucMsgType == 0x02 && DongleReqOut->ucSubType == 0x0A)//購貨
		{
			BYTE fAccUsed = FALSE,bT553301[10 + 1],bT553304[8 + 1],bT553307[8 + 1],bT553309[5 + 1],bT553311[8 + 1],bTmp[10],Acc_Free_Rids_Date_Bef[5];
			UnionUShort usAccPointBef,usAccPointAft;

			memset(bT553301,0x00,sizeof(bT553301));
			memset(bT553304,0x00,sizeof(bT553304));
			memset(bT553307,0x00,sizeof(bT553307));
			memset(bT553309,0x00,sizeof(bT553309));
			memset(bT553311,0x00,sizeof(bT553311));
			memset(Acc_Free_Rids_Date_Bef,0x00,sizeof(Acc_Free_Rids_Date_Bef));
			usAccPointBef.Value = 0;
			usAccPointAft.Value = 0;

			if(DongleReqOut->ucPurseVersionNumber == MIFARE)//Mifare
			{
				if(memcmp(DongleReqOut->ucTxnCrypto,"\x00\x00\x00\x00",4))//Mifare的ucTxnCrypto非00表示原交易有使用社福點!!
				{
					memcpy(usAccPointBef.Buf,DongleReqOut->ucTxnCrypto,2);
					memcpy(Acc_Free_Rids_Date_Bef,&DongleReqOut->ucTxnCrypto[2],2);
					fAccUsed = TRUE;
				}
			}
			else if(memcmp(DongleReqOut->ucSAMSN,"\x00\x00\x00\x00",4))//L1 or L2的ucSAMSN非00表示原交易有使用社福點!!
			{
				memcpy(usAccPointBef.Buf,DongleReqOut->ucSAMSN,2);
				memcpy(Acc_Free_Rids_Date_Bef,&DongleReqOut->ucSAMSN[2],2);
				fAccUsed = TRUE;
			}

			if(fAccUsed == TRUE)
			{
				memcpy(usAccPointAft.Buf,DongleOut->Acc_Free_Rids,2);
				sprintf((char *)bT553301,"WELFARE%02X",DongleReqOut->ucAreaCode);
				if(!memcmp(Acc_Free_Rids_Date_Bef,DongleOut->Acc_Free_Rids_Date,2))
				{
					if(usAccPointBef.Value >= usAccPointAft.Value)
						sprintf((char *)bT553304,"%d",usAccPointBef.Value - usAccPointAft.Value);
					else
						sprintf((char *)bT553304,"0");
				}
				else
					sprintf((char *)bT553304,"%d",usAccPointAft.Value);
				sprintf((char *)bT553307,"WELFARE");
				sprintf((char *)bT553309,"%d",usAccPointAft.Value);
				fngetDosDate(DongleOut->Acc_Free_Rids_Date,bT553311);

				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5533,(char *)bTmp,TRUE,NODE_NO_SAME,TAG_NAME_START,TRUE);
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_553301,(char *)bT553301,FALSE,NODE_NO_ADD,VALUE_NAME,TRUE);
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_553304,(char *)bT553304,FALSE,NODE_NO_ADD,VALUE_NAME,TRUE);
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_553307,(char *)bT553307,FALSE,NODE_NO_ADD,VALUE_NAME,TRUE);
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_553309,(char *)bT553309,FALSE,NODE_NO_ADD,VALUE_NAME,TRUE);
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_553311,(char *)bT553311,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);//CMAS不收T553311
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5533,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,TRUE);
				inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"",TRUE);
			}
		}
	}
#endif

	//T6000
	vdUnPackToXMLData(DongleReqOut->ucReaderFWVersion,sizeof(DongleReqOut->ucReaderFWVersion),srXML,(char *)"T6000",strlen("T6000"),VALUE_NAME,fXMLSendFlag);

	//T6001
	vdUnPackToXMLData((BYTE *)&DongleReqOut->stReaderAVRInfo_t,83,srXML,(char *)"T6001",strlen("T6001"),VALUE_NAME,fXMLSendFlag);

	//T6402
	if(DongleReqOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(&DongleOut->ucMAC_HCrypto[10],8,srXML,(char *)"T6402",strlen("T6402"),VALUE_NAME,fXMLSendFlag);

	//T6403
	if(DongleReqOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(&DongleOut->ucMAC_HCrypto[2],4,srXML,(char *)"T6403",strlen("T6403"),VALUE_NAME,fXMLSendFlag);

	//T6404
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucMAC_HCrypto[2],16,srXML,(char *)"T6404",strlen("T6404"),VALUE_NAME,fXMLSendFlag);

	//T6405
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucSignature,16,srXML,(char *)"T6405",strlen("T6405"),VALUE_NAME,fXMLSendFlag);

	return SUCCESS;
}

int inBuildCMSTxnReqOfflineTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
BYTE bBuf[200],bTmp[50];
TxnReqOffline_APDU_In *DongleIn = (TxnReqOffline_APDU_In *)Dongle_In;
TxnReqOffline_APDU_Out *DongleOut = (TxnReqOffline_APDU_Out *)Dongle_Out;
BOOL fXMLSendFlag = TRUE;
int inCntStart,inCntEnd;

	//if(DongleOut->ucStatusCode[0] != 0x64 || DongleOut->ucStatusCode[1] != 0x15)
	//	return SUCCESS;

	//TAG_TRANS_XML_HEADER
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_TRANS_XML_HEADER);

	//TAG_TRANS_HEADER
	if(inXMLSearchValueByTag(srXML,(char *)TAG_TRANS_HEADER,&inCntStart,&inCntEnd,0) >= SUCCESS)
	{
		srXML->srXMLElement[inCntStart].fXMLSendFlag = TRUE;
		//vdSetConfigFlag(srXML,FALSE,TAG_TRANS_HEADER);
		memcpy(srXML->srXMLElement[inCntStart].chTag,TAG_TRANS_HEADER,strlen(TAG_TRANS_HEADER));
	}

	//T0100
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0100,(BYTE *)"0100",fXMLSendFlag);

	//T0200
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdUIntToAsc(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),bTmp,17,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,17);
	inXMLAppendData(srXML,(char *)TAG_NAME_0200,strlen(TAG_NAME_0200),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T0211
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucPID,sizeof(DongleOut->ucPID),srXML,(char *)TAG_NAME_0211,strlen(TAG_NAME_0211),VALUE_NAME,fXMLSendFlag);

	//T0212
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%02x",DongleOut->bAutoLoad);
	inXMLAppendData(srXML,(char *)TAG_NAME_0212,strlen(TAG_NAME_0212),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T0213
	vdUnPackToXMLData(&DongleOut->ucCardType,sizeof(DongleOut->ucCardType),srXML,(char *)TAG_NAME_0213,strlen(TAG_NAME_0213),VALUE_NAME,fXMLSendFlag);

	//T0214
	vdUnPackToXMLData(&DongleOut->ucPersonalProfile,sizeof(DongleOut->ucPersonalProfile),srXML,(char *)TAG_NAME_0214,strlen(TAG_NAME_0214),VALUE_NAME,fXMLSendFlag);

	//T0300
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"816399",TRUE);

	//T0400
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%ld",srTxnData.srIngData.lnECCAmt);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0400,bBuf,fXMLSendFlag);

	//T0409
	/*memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdUIntToAsc(DongleOut->ucAutoLoadAmt,sizeof(DongleOut->ucAutoLoadAmt),bTmp,5,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,5);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0409,bBuf,fXMLSendFlag);*/

	//T0410
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),bTmp,5,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,5);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0410,bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0410,strlen(TAG_NAME_0410),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1100
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulCMASHostSerialNumber);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1100,bBuf,fXMLSendFlag);

	//T1101
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulTMSerialNumber);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1101,bBuf,fXMLSendFlag);

	//T1200	When Retry T1200 Must Changed, by Eric Chan 20190326//Marked by Eric Chan
	//if (srTxnData.srIngData.fTMRetryFlag == TRUE)
	//{
	//	memset(bBuf,0x00,sizeof(bBuf));
	//	memcpy((char *)bBuf,srTxnData.srIngData.chTxTime,sizeof(srTxnData.srIngData.chTxTime) - 1);
	//	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1200,bBuf,fXMLSendFlag);
	//}

	//T1201
	memset(bBuf,0x00,sizeof(bBuf));
	memcpy((char *)bBuf,srTxnData.srIngData.chTxTime,sizeof(srTxnData.srIngData.chTxTime) - 1);
	inXMLAppendData(srXML,(char *)"T1201",strlen("T1201"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	/*{
		BYTE bT1300[10];

		memset(bT1300,0x00,sizeof(bT1300));
		inXMLGetData(srXML,(char *)TAG_NAME_1300,(char *)bT1300,sizeof(srTxnData.srIngData.chTxDate),0);
		log_msg(LOG_LEVEL_ERROR,"T1300 1:%s,%s",srTxnData.srIngData.chTxDate,bT1300);
	}*/

	//T1301
	memset(bBuf,0x00,sizeof(bBuf));
	memcpy((char *)bBuf,srTxnData.srIngData.chTxDate,sizeof(srTxnData.srIngData.chTxDate) - 1);
	inXMLAppendData(srXML,(char *)"T1301",strlen("T1301"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1402
	memset(bBuf,0x00,sizeof(bBuf));
	UnixToDateTime((BYTE*)DongleOut->ucExpiryDate,(BYTE*)bBuf,8);//Expiry Date
	inXMLAppendData(srXML,(char *)TAG_NAME_1402,strlen(TAG_NAME_1402),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T3700
	vdCMASBuildRRNProcess(srXML,srTxnData.srIngData.ulCMASHostSerialNumber,DongleOut->ucTxnSN);

	//T4100
	vdUnPackToXMLData(DongleOut->ucCPUDeviceID,sizeof(DongleOut->ucCPUDeviceID),srXML,(char *)TAG_NAME_4100,strlen(TAG_NAME_4100),VALUE_NAME,fXMLSendFlag);

	//T4101
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(DongleOut->ucDeviceID,sizeof(DongleOut->ucDeviceID),srXML,(char *)"T4101",strlen("T4101"),VALUE_NAME,fXMLSendFlag);

	//T4102
	memset(bBuf,0x00,sizeof(bBuf));
	vdGetLocalIP(bBuf);
	inXMLAppendData(srXML,(char *)"T4102",strlen("T4102"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T4104
	vdUnPackToXMLData(gTmpData.ucReaderID,sizeof(gTmpData.ucReaderID),srXML,(char *)TAG_NAME_4104,strlen(TAG_NAME_4104),VALUE_NAME,fXMLSendFlag);

	//T4107
	if(strlen(srTxnData.srParameter.chPOS_ID) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4107,(BYTE *)srTxnData.srParameter.chPOS_ID,fXMLSendFlag);

	//T4108 off
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4108);

	//T4117
	if(strlen(srTxnData.srParameter.chPOS_ID2) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4117,(BYTE *)srTxnData.srParameter.chPOS_ID2,fXMLSendFlag);

	//T4200//銀行版-->TM Location ID,一般版-->New SP ID,商店模式-->自訂的TM Location ID
	if(srTxnData.srParameter.chCMASMode == '1' || srTxnData.srParameter.chCMASMode == '2')//銀行併機 or 商店模式
	{
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy(bBuf,&DongleIn->ucTMLocationID[2],8);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,bBuf,fXMLSendFlag);
	}
	else//一般版
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(DongleOut->ucCPUSPID,sizeof(DongleOut->ucCPUSPID),bBuf,8,FALSE,'0',10);
		//vdUIntToAsc(&DongleOut->ucCPUDeviceID[3],3,bBuf,8,FALSE,'0',10);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,bBuf,fXMLSendFlag);
		//inXMLAppendData(srXML,(char *)TAG_NAME_4200,strlen(TAG_NAME_4200),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}

	//T4210
	memset(bBuf,0x00,sizeof(bBuf));
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUIntToAsc(&DongleOut->ucLocationID,1,bBuf,3,TRUE,0x00,10);
	else
		vdUIntToAsc(DongleOut->ucCPULocationID,2,bBuf,5,TRUE,0x00,10);
	inXMLAppendData(srXML,(char *)"T4210",strlen("T4210"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T4800
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucPurseVersionNumber,sizeof(DongleOut->ucPurseVersionNumber),srXML,(char *)TAG_NAME_4800,strlen(TAG_NAME_4800),VALUE_NAME,fXMLSendFlag);

	//T4801,33 byte unpack to 66 byte,後補0至90 byte
	vdBuildCMASCardAVR((BYTE *)&DongleOut->stLastCreditTxnLogInfo_t,sizeof(DongleOut->stLastCreditTxnLogInfo_t),srXML,(char *)"T4801",strlen("T4801"),VALUE_NAME,fXMLSendFlag);

	//T4802
	vdUnPackToXMLData(&DongleOut->ucIssuerCode,sizeof(DongleOut->ucIssuerCode),srXML,(char *)"T4802",strlen("T4802"),VALUE_NAME,fXMLSendFlag);

	//T4803
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(DongleOut->BankCode,sizeof(DongleOut->BankCode),srXML,(char *)TAG_NAME_4803,strlen(TAG_NAME_4803),VALUE_NAME,fXMLSendFlag);
	//else
	//	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4803,(BYTE *)"00",fXMLSendFlag);

	//T4804
	vdUnPackToXMLData(&DongleOut->ucAreaCode,sizeof(DongleOut->ucAreaCode),srXML,(char *)TAG_NAME_4804,strlen(TAG_NAME_4804),VALUE_NAME,fXMLSendFlag);

	//T4805
	vdUnPackToXMLData(DongleOut->ucSubAreaCode,sizeof(DongleOut->ucSubAreaCode),srXML,(char *)TAG_NAME_4805,strlen(TAG_NAME_4805),VALUE_NAME,fXMLSendFlag);

	//T4806
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucProfileExpiryDate,sizeof(DongleOut->ucProfileExpiryDate),srXML,(char *)"T4806",strlen("T4806"),VALUE_NAME,fXMLSendFlag);

	//T4808
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
	inXMLAppendData(srXML,(char *)"T4808",strlen("T4808"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),srXML,(char *)"T4808",strlen("T4808"),VALUE_NAME,fXMLSendFlag);

	//T4809
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucTxnMode,sizeof(DongleOut->ucTxnMode),srXML,(char *)"T4809",strlen("T4809"),VALUE_NAME,fXMLSendFlag);

	//T4810
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucTxnQuqlifier,sizeof(DongleOut->ucTxnQuqlifier),srXML,(char *)"T4810",strlen("T4810"),VALUE_NAME,fXMLSendFlag);

	//T4811
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
	inXMLAppendData(srXML,(char *)"T4811",strlen("T4811"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),srXML,(char *)"T4811",strlen("T4811"),VALUE_NAME,fXMLSendFlag);

	//T4812
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucCTC,sizeof(DongleOut->ucCTC),srXML,(char *)"T4812",strlen("T4812"),VALUE_NAME,fXMLSendFlag);

	//T4813
	if(DongleOut->ucPurseVersionNumber == MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(&DongleOut->ucCPD_SAMID[0],2,bBuf,6,TRUE,0x00,10);
		vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
		inXMLAppendData(srXML,(char *)"T4813",strlen("T4813"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
		//vdUnPackToXMLData(DongleOut->ucLoyaltyCounter,sizeof(DongleOut->ucLoyaltyCounter),srXML,(char *)"T4813",strlen("T4813"),VALUE_NAME,fXMLSendFlag);
	}
	//else
	//	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4813",(BYTE *)"0000",fXMLSendFlag);

	//T4814
	if(DongleOut->ucPurseVersionNumber != MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(DongleOut->ucDeposit,3,bBuf,6,TRUE,0x00,10);
		vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
		inXMLAppendData(srXML,(char *)"T4814",strlen("T4814"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
		//vdUnPackToXMLData(DongleOut->ucDeposit,sizeof(DongleOut->ucDeposit),srXML,(char *)TAG_NAME_4814,strlen(TAG_NAME_4814),VALUE_NAME,fXMLSendFlag);
	}

	//T4826
	vdUnPackToXMLData(&DongleOut->ucCardIDLen,sizeof(DongleOut->ucCardIDLen),srXML,(char *)"T4826",strlen("T4826"),VALUE_NAME,fXMLSendFlag);

	//T5301
	vdUnPackToXMLData(&DongleOut->ucCPDKVN_SAMKVN,sizeof(DongleOut->ucCPDKVN_SAMKVN),srXML,(char *)"T5301",strlen("T5301"),VALUE_NAME,fXMLSendFlag);

	//T5302
	if(DongleOut->ucPurseVersionNumber != MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		sprintf((char *)bBuf,"%02x%02x%02x",DongleOut->ucCPUAdminKeyKVN,DongleOut->ucCreditKeyKVN,DongleOut->ucCPUIssuerKeyKVN);
		inXMLAppendData(srXML,(char *)"T5302",strlen("T5302"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}

	//T5304
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucBankCode_HostAdminKVN,sizeof(DongleOut->ucBankCode_HostAdminKVN),srXML,(char *)"T5304",strlen("T5304"),VALUE_NAME,fXMLSendFlag);

	//T5305
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucSignatureKeyKVN,sizeof(DongleOut->ucSignatureKeyKVN),srXML,(char *)"T5305",strlen("T5305"),VALUE_NAME,fXMLSendFlag);

	//T5361
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(&DongleOut->ucCPD_SAMID[2],8,srXML,(char *)"T5361",strlen("T5361"),VALUE_NAME,fXMLSendFlag);

	//T5362
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(&DongleOut->ucCPD_SAMID[10],4,srXML,(char *)"T5362",strlen("T5362"),VALUE_NAME,fXMLSendFlag);

	//T5363
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(DongleOut->ucCPDRAN_SAMCRN,sizeof(DongleOut->ucCPDRAN_SAMCRN),srXML,(char *)"T5363",strlen("T5363"),VALUE_NAME,fXMLSendFlag);

	//T5371
	vdUnPackToXMLData(DongleOut->ucSID_STAC,sizeof(DongleOut->ucSID_STAC),srXML,(char *)"T5371",strlen("T5371"),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucCPUSAMID,sizeof(DongleOut->ucCPUSAMID),srXML,(char *)"T5371",strlen("T5371"),VALUE_NAME,fXMLSendFlag);

	//T5503
	memset(bBuf,0x00,sizeof(bBuf));
	if (srTxnData.srParameter.chCMASMode == '2')
	{
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5503",(BYTE *)srTxnData.srREQData.bT5503,TRUE);
	}
	else
	{
		memcpy(bBuf,DongleIn->ucTMLocationID,sizeof(DongleIn->ucTMLocationID));
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5503",bBuf,fXMLSendFlag);
		//inXMLAppendData(srXML,(char *)"T5503",strlen("T5503"),DongleIn->ucTMLocationID,10,VALUE_NAME,fXMLSendFlag);
	}

	//T5504
	inXMLAppendData(srXML,(char *)"T5504",strlen("T5504"),DongleIn->ucTMID,2,VALUE_NAME,fXMLSendFlag);

	//T5510
	inXMLAppendData(srXML,(char *)"T5510",strlen("T5510"),DongleIn->ucTMAgentNumber,4,VALUE_NAME,fXMLSendFlag);

	//T6000
	vdUnPackToXMLData(DongleOut->ucReaderFWVersion,sizeof(DongleOut->ucReaderFWVersion),srXML,(char *)"T6000",strlen("T6000"),VALUE_NAME,fXMLSendFlag);

	//T6004
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	inGetBLCName(bTmp);
	memcpy(bBuf,bTmp,5);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T6004",bBuf,fXMLSendFlag);

	//T6400
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(DongleOut->ucSID_STAC,sizeof(DongleOut->ucSID_STAC),srXML,(char *)"T6400",strlen("T6400"),VALUE_NAME,fXMLSendFlag);

	//T6406
	if(DongleOut->ucPurseVersionNumber != MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy((char *)bBuf,DongleOut->ucSVCrypto,sizeof(DongleOut->ucSVCrypto));
		vdUnPackToXMLData(bBuf,sizeof(DongleOut->ucSVCrypto),srXML,(char *)"T6406",strlen("T6406"),VALUE_NAME,fXMLSendFlag);
	}

  //#ifdef READER_MODE
	memset(bBuf,0x00,sizeof(bBuf));
	vdStoreDeviceID(DongleOut->ucDeviceID,DongleOut->ucDeviceID[2],DongleOut->ucLocationID,DongleOut->ucCPUDeviceID,DongleOut->ucCPUSPID,DongleOut->ucCPULocationID,gTmpData.ucReaderID,DongleOut->ucReaderFWVersion,DongleOut->ucCPUSAMID);
	//vdStoreDeviceID(DongleOut->ucDeviceID,DongleOut->ucDeviceID[2],DongleOut->ucLocationID,DongleOut->ucCPUDeviceID,&DongleOut->ucCPUDeviceID[3],DongleOut->ucCPULocationID,gTmpData.ucReaderID,DongleOut->ucReaderFWVersion,DongleOut->ucCPUSAMID);
  //#endif

	return SUCCESS;
}

int inBuildCMSTaxiReadTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
BYTE bBuf[200],bTmp[50];
//TaxiRead_APDU_In *DongleIn = (TaxiRead_APDU_In *)Dongle_In;
TaxiRead_APDU_Out *DongleOut = (TaxiRead_APDU_Out *)Dongle_Out;
BOOL fXMLSendFlag = TRUE;
int inCntStart,inCntEnd;

	//if(DongleOut->ucStatusCode[0] != 0x64 || DongleOut->ucStatusCode[1] != 0x15)
	//	return SUCCESS;

	//TAG_TRANS_XML_HEADER
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_TRANS_XML_HEADER);

	//TAG_TRANS_HEADER
	if(inXMLSearchValueByTag(srXML,(char *)TAG_TRANS_HEADER,&inCntStart,&inCntEnd,0) >= SUCCESS)
	{
		srXML->srXMLElement[inCntStart].fXMLSendFlag = TRUE;
		//vdSetConfigFlag(srXML,FALSE,TAG_TRANS_HEADER);
		memcpy(srXML->srXMLElement[inCntStart].chTag,TAG_TRANS_HEADER,strlen(TAG_TRANS_HEADER));
	}

	//T0100
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0100,(BYTE *)"0100",fXMLSendFlag);

	//T0200
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdUIntToAsc(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),bTmp,17,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,17);
	inXMLAppendData(srXML,(char *)TAG_NAME_0200,strlen(TAG_NAME_0200),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T0211
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucPID,sizeof(DongleOut->ucPID),srXML,(char *)TAG_NAME_0211,strlen(TAG_NAME_0211),VALUE_NAME,fXMLSendFlag);

	//T0212
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%02x",DongleOut->bAutoLoad);
	inXMLAppendData(srXML,(char *)TAG_NAME_0212,strlen(TAG_NAME_0212),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T0213
	vdUnPackToXMLData(&DongleOut->ucCardType,sizeof(DongleOut->ucCardType),srXML,(char *)TAG_NAME_0213,strlen(TAG_NAME_0213),VALUE_NAME,fXMLSendFlag);

	//T0214
	vdUnPackToXMLData(&DongleOut->ucPersonalProfile,sizeof(DongleOut->ucPersonalProfile),srXML,(char *)TAG_NAME_0214,strlen(TAG_NAME_0214),VALUE_NAME,fXMLSendFlag);

	//T0300
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"816399",TRUE);

	//T0400
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%ld",srTxnData.srIngData.lnECCAmt);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0400,bBuf,fXMLSendFlag);

	//T0409
	/*memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdUIntToAsc(DongleOut->ucAutoLoadAmt,sizeof(DongleOut->ucAutoLoadAmt),bTmp,5,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,5);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0409,bBuf,fXMLSendFlag);*/

	//T0410
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),bTmp,5,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,5);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0410,bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0410,strlen(TAG_NAME_0410),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1100
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulCMASHostSerialNumber);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1100,bBuf,fXMLSendFlag);

	//T1101
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulTMSerialNumber);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1101,bBuf,fXMLSendFlag);

	//T1201
	memset(bBuf,0x00,sizeof(bBuf));
	memcpy((char *)bBuf,srTxnData.srIngData.chTxTime,sizeof(srTxnData.srIngData.chTxTime) - 1);
	inXMLAppendData(srXML,(char *)"T1201",strlen("T1201"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1301
	memset(bBuf,0x00,sizeof(bBuf));
	memcpy((char *)bBuf,srTxnData.srIngData.chTxDate,sizeof(srTxnData.srIngData.chTxDate) - 1);
	inXMLAppendData(srXML,(char *)"T1301",strlen("T1301"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1402
	memset(bBuf,0x00,sizeof(bBuf));
	UnixToDateTime((BYTE*)DongleOut->ucExpiryDate,(BYTE*)bBuf,8);//Expiry Date
	inXMLAppendData(srXML,(char *)TAG_NAME_1402,strlen(TAG_NAME_1402),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T3700
	vdCMASBuildRRNProcess(srXML,srTxnData.srIngData.ulCMASHostSerialNumber,DongleOut->ucTxnSN);

	//T4100
	vdUnPackToXMLData(DongleOut->ucCPUDeviceID,sizeof(DongleOut->ucCPUDeviceID),srXML,(char *)TAG_NAME_4100,strlen(TAG_NAME_4100),VALUE_NAME,fXMLSendFlag);

	//T4101
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(DongleOut->ucDeviceID,sizeof(DongleOut->ucDeviceID),srXML,(char *)"T4101",strlen("T4101"),VALUE_NAME,fXMLSendFlag);

	//T4102
	memset(bBuf,0x00,sizeof(bBuf));
	vdGetLocalIP(bBuf);
	inXMLAppendData(srXML,(char *)"T4102",strlen("T4102"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T4104
	vdUnPackToXMLData(gTmpData.ucReaderID,sizeof(gTmpData.ucReaderID),srXML,(char *)TAG_NAME_4104,strlen(TAG_NAME_4104),VALUE_NAME,fXMLSendFlag);

	//T4107
	if(strlen(srTxnData.srParameter.chPOS_ID) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4107,(BYTE *)srTxnData.srParameter.chPOS_ID,fXMLSendFlag);

	//T4108 off
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4108);

	//T4117
	if(strlen(srTxnData.srParameter.chPOS_ID2) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4117,(BYTE *)srTxnData.srParameter.chPOS_ID2,fXMLSendFlag);

	//T4200//銀行版-->TM Location ID,一般版-->New SP ID,商店模式-->TM Location ID
	if(srTxnData.srParameter.chCMASMode == '1' || srTxnData.srParameter.chCMASMode == '2')//銀行併機 or 商店模式
	{
		//inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID,fXMLSendFlag);
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy(bBuf,&srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID[2],8);
		//memcpy(bBuf,&DongleIn->ucTMLocationID[2],8);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,bBuf,fXMLSendFlag);
	}
	else//一般版
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(DongleOut->ucCPUSPID,sizeof(DongleOut->ucCPUSPID),bBuf,8,FALSE,'0',10);
		//vdUIntToAsc(&DongleOut->ucCPUDeviceID[3],3,bBuf,8,FALSE,'0',10);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,bBuf,fXMLSendFlag);
		//inXMLAppendData(srXML,(char *)TAG_NAME_4200,strlen(TAG_NAME_4200),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}

	//T4210
	memset(bBuf,0x00,sizeof(bBuf));
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUIntToAsc(&DongleOut->ucLocationID,1,bBuf,3,TRUE,0x00,10);
	else
		vdUIntToAsc(DongleOut->ucCPULocationID,2,bBuf,5,TRUE,0x00,10);
	inXMLAppendData(srXML,(char *)"T4210",strlen("T4210"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T4800
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucPurseVersionNumber,sizeof(DongleOut->ucPurseVersionNumber),srXML,(char *)TAG_NAME_4800,strlen(TAG_NAME_4800),VALUE_NAME,fXMLSendFlag);

	//T4801,33 byte unpack to 66 byte,後補0至90 byte
	vdBuildCMASCardAVR((BYTE *)&DongleOut->stLastCreditTxnLogInfo_t,sizeof(DongleOut->stLastCreditTxnLogInfo_t),srXML,(char *)"T4801",strlen("T4801"),VALUE_NAME,fXMLSendFlag);

	//T4802
	vdUnPackToXMLData(&DongleOut->ucIssuerCode,sizeof(DongleOut->ucIssuerCode),srXML,(char *)"T4802",strlen("T4802"),VALUE_NAME,fXMLSendFlag);

	//T4803
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(&DongleOut->ucBankCode,sizeof(DongleOut->ucBankCode),srXML,(char *)TAG_NAME_4803,strlen(TAG_NAME_4803),VALUE_NAME,fXMLSendFlag);
	//else
	//	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4803,(BYTE *)"00",fXMLSendFlag);

	//T4804
	vdUnPackToXMLData(&DongleOut->ucAreaCode,sizeof(DongleOut->ucAreaCode),srXML,(char *)TAG_NAME_4804,strlen(TAG_NAME_4804),VALUE_NAME,fXMLSendFlag);

	//T4805
	vdUnPackToXMLData(DongleOut->ucSubAreaCode,sizeof(DongleOut->ucSubAreaCode),srXML,(char *)TAG_NAME_4805,strlen(TAG_NAME_4805),VALUE_NAME,fXMLSendFlag);

	//T4806
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucProfileExpiryDate,sizeof(DongleOut->ucProfileExpiryDate),srXML,(char *)"T4806",strlen("T4806"),VALUE_NAME,fXMLSendFlag);

	//T4808
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
	inXMLAppendData(srXML,(char *)"T4808",strlen("T4808"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),srXML,(char *)"T4808",strlen("T4808"),VALUE_NAME,fXMLSendFlag);

	//T4809
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
	//	vdUnPackToXMLData(&DongleOut->ucTxnMode,sizeof(DongleOut->ucTxnMode),srXML,(char *)"T4809",strlen("T4809"),VALUE_NAME,fXMLSendFlag);

	//T4810
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
	//	vdUnPackToXMLData(&DongleOut->ucTxnQuqlifier,sizeof(DongleOut->ucTxnQuqlifier),srXML,(char *)"T4810",strlen("T4810"),VALUE_NAME,fXMLSendFlag);

	//T4811
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
	inXMLAppendData(srXML,(char *)"T4811",strlen("T4811"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),srXML,(char *)"T4811",strlen("T4811"),VALUE_NAME,fXMLSendFlag);

	//T4812
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
	//	vdUnPackToXMLData(DongleOut->ucCTC,sizeof(DongleOut->ucCTC),srXML,(char *)"T4812",strlen("T4812"),VALUE_NAME,fXMLSendFlag);

	//T4813
	/*if(DongleOut->ucPurseVersionNumber == MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(&DongleOut->ucCPD_SAMID[0],2,bBuf,6,TRUE,0x00,10);
		vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
		inXMLAppendData(srXML,(char *)"T4813",strlen("T4813"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
		//vdUnPackToXMLData(DongleOut->ucLoyaltyCounter,sizeof(DongleOut->ucLoyaltyCounter),srXML,(char *)"T4813",strlen("T4813"),VALUE_NAME,fXMLSendFlag);
	}*/
	//else
	//	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4813",(BYTE *)"0000",fXMLSendFlag);

	//T4814
	/*if(DongleOut->ucPurseVersionNumber != MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(DongleOut->ucDeposit,3,bBuf,6,TRUE,0x00,10);
		vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
		inXMLAppendData(srXML,(char *)"T4814",strlen("T4814"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
		//vdUnPackToXMLData(DongleOut->ucDeposit,sizeof(DongleOut->ucDeposit),srXML,(char *)TAG_NAME_4814,strlen(TAG_NAME_4814),VALUE_NAME,fXMLSendFlag);
	}*/

	//T4826
	vdUnPackToXMLData(&DongleOut->ucCardIDLen,sizeof(DongleOut->ucCardIDLen),srXML,(char *)"T4826",strlen("T4826"),VALUE_NAME,fXMLSendFlag);

	//T5301
	//vdUnPackToXMLData(&DongleOut->ucCPDKVN_SAMKVN,sizeof(DongleOut->ucCPDKVN_SAMKVN),srXML,(char *)"T5301",strlen("T5301"),VALUE_NAME,fXMLSendFlag);

	//T5302
	/*if(DongleOut->ucPurseVersionNumber != MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		sprintf((char *)bBuf,"%02x%02x%02x",DongleOut->ucCPUAdminKeyKVN,DongleOut->ucCreditKeyKVN,DongleOut->ucCPUIssuerKeyKVN);
		inXMLAppendData(srXML,(char *)"T5302",strlen("T5302"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}*/

	//T5304
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
		//vdUnPackToXMLData(&DongleOut->ucBankCode_HostAdminKVN,sizeof(DongleOut->ucBankCode_HostAdminKVN),srXML,(char *)"T5304",strlen("T5304"),VALUE_NAME,fXMLSendFlag);

	//T5305
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
		//vdUnPackToXMLData(&DongleOut->ucSignatureKeyKVN,sizeof(DongleOut->ucSignatureKeyKVN),srXML,(char *)"T5305",strlen("T5305"),VALUE_NAME,fXMLSendFlag);

	//T5361
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
		//vdUnPackToXMLData(&DongleOut->ucCPD_SAMID[2],8,srXML,(char *)"T5361",strlen("T5361"),VALUE_NAME,fXMLSendFlag);

	//T5362
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
		//vdUnPackToXMLData(&DongleOut->ucCPD_SAMID[10],4,srXML,(char *)"T5362",strlen("T5362"),VALUE_NAME,fXMLSendFlag);

	//T5363
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
		//vdUnPackToXMLData(DongleOut->ucCPDRAN_SAMCRN,sizeof(DongleOut->ucCPDRAN_SAMCRN),srXML,(char *)"T5363",strlen("T5363"),VALUE_NAME,fXMLSendFlag);

	//T5371
	//vdUnPackToXMLData(DongleOut->ucSID_STAC,sizeof(DongleOut->ucSID_STAC),srXML,(char *)"T5371",strlen("T5371"),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucCPUSAMID,sizeof(DongleOut->ucCPUSAMID),srXML,(char *)"T5371",strlen("T5371"),VALUE_NAME,fXMLSendFlag);

	//T5503
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5503",(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T5503",strlen("T5503"),(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID,10,VALUE_NAME,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T5503",strlen("T5503"),DongleIn->ucTMLocationID,10,VALUE_NAME,fXMLSendFlag);

	//T5504
	inXMLAppendData(srXML,(char *)"T5504",strlen("T5504"),(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMID,2,VALUE_NAME,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T5504",strlen("T5504"),DongleIn->ucTMID,2,VALUE_NAME,fXMLSendFlag);

	//T5510
	inXMLAppendData(srXML,(char *)"T5510",strlen("T5510"),(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber,4,VALUE_NAME,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T5510",strlen("T5510"),DongleIn->ucTMAgentNumber,4,VALUE_NAME,fXMLSendFlag);

	//T5533
	//vdSetPointDetail(inTxnType,srXML);

	//T6000
	vdUnPackToXMLData(gTmpData.ucReaderFWVersion,sizeof(gTmpData.ucReaderFWVersion),srXML,(char *)"T6000",strlen("T6000"),VALUE_NAME,fXMLSendFlag);

	//T6000
	//vdUnPackToXMLData(DongleOut->ucReaderFWVersion,sizeof(DongleOut->ucReaderFWVersion),srXML,(char *)"T6000",strlen("T6000"),VALUE_NAME,fXMLSendFlag);

	//T6004
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	inGetBLCName(bTmp);
	memcpy(bBuf,bTmp,5);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T6004",bBuf,fXMLSendFlag);

	//T6400
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
	//	vdUnPackToXMLData(DongleOut->ucSID_STAC,sizeof(DongleOut->ucSID_STAC),srXML,(char *)"T6400",strlen("T6400"),VALUE_NAME,fXMLSendFlag);

	//T6406
	/*if(DongleOut->ucPurseVersionNumber != MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy((char *)bBuf,DongleOut->ucSVCrypto,sizeof(DongleOut->ucSVCrypto));
		vdUnPackToXMLData(bBuf,sizeof(DongleOut->ucSVCrypto),srXML,(char *)"T6406",strlen("T6406"),VALUE_NAME,fXMLSendFlag);
	}*/

	return SUCCESS;
}

int inBuildCMSTaxiDeductTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
BYTE bBuf[50];
TaxiDeduct_APDU_In *DongleIn = (TaxiDeduct_APDU_In *)Dongle_In;
TaxiDeduct_APDU_Out *DongleOut = (TaxiDeduct_APDU_Out *)Dongle_Out;
TaxiRead_APDU_Out *DongleReqOut = (TaxiRead_APDU_Out *)Dongle_ReqOut;
BOOL fXMLSendFlag = TRUE;
int inCntStart,inCntEnd;
BYTE bDate[20];

	//TAG_TRANS_XML_HEADER
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_TRANS_XML_HEADER);

	//TAG_TRANS_HEADER
	if(inXMLSearchValueByTag(srXML,(char *)TAG_TRANS_HEADER,&inCntStart,&inCntEnd,0) >= SUCCESS)
	{
		srXML->srXMLElement[inCntStart].fXMLSendFlag = TRUE;
		//vdSetConfigFlag(srXML,FALSE,TAG_TRANS_HEADER);
		memcpy(srXMLData.srXMLElement[inCntStart].chTag,TAG_TRANS_HEADER,strlen(TAG_TRANS_HEADER));
	}

	//T0100
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0100,(BYTE *)"0220",TRUE);

	//T0200
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0200);

	//T0211
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0211);

	//T0213
	vdUnPackToXMLData(&DongleReqOut->ucCardType,sizeof(DongleReqOut->ucCardType),srXML,(char *)TAG_NAME_0213,strlen(TAG_NAME_0213),VALUE_NAME,fXMLSendFlag);

	//T0214
	vdUnPackToXMLData(&DongleReqOut->ucPersonalProfile,sizeof(DongleReqOut->ucPersonalProfile),srXML,(char *)TAG_NAME_0214,strlen(TAG_NAME_0214),VALUE_NAME,fXMLSendFlag);

	//T0300
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"811599",TRUE);

	//T0400
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%ld",srTxnData.srIngData.lnECCAmt);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0400,bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0400,strlen(TAG_NAME_0400),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T0404,社福點數
	//vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0404);
	memset(bBuf,0x00,sizeof(bBuf));
	//sprintf((char *)bBuf,"%d",1);
	//inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0404,bBuf,fXMLSendFlag);
	if(inXMLGetData(srXML,(char *)TAG_NAME_0404,(char *)bBuf,sizeof(bBuf),0) < SUCCESS)
		log_msg(LOG_LEVEL_ERROR,"inBuildCMSTaxiDeductTM Fail 1: NO T0404");
	if(strlen((char *)bBuf) <= 2)//肯定沒有2碼小數
	{
		vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0404);
		log_msg(LOG_LEVEL_ERROR,"inBuildCMSTaxiDeductTM  2!!");
	}
	else if(memcmp((char *)&bBuf[strlen((char *)bBuf) - 2],"00",2))//最後2碼不是00
	{
		vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0404);
		log_msg(LOG_LEVEL_ERROR,"inBuildCMSTaxiDeductTM Fail 3!!");
	}
	else//最後2碼肯定是00,去掉後2碼
	{
		bBuf[strlen((char *)bBuf) - 2] = 0x00;
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0404,bBuf,fXMLSendFlag);
		log_msg(LOG_LEVEL_ERROR,"inBuildCMSTaxiDeductTM Fail 4!!");
	}

	//T0408
	memset(bBuf,0x00,sizeof(bBuf));
	vdIntToAsc(DongleOut->stDeductOut_t.ucEV,sizeof(DongleOut->stDeductOut_t.ucEV),bBuf,5,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0408,bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0408,strlen(TAG_NAME_0408),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	srTxnData.srIngData.lnECCEV = lnAmt3ByteToLong(DongleOut->stDeductOut_t.ucEV);

	//T0409
	if(strlen((char *)srTxnData.srIngData.anAutoloadAmount) > 2)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy(bBuf,srTxnData.srIngData.anAutoloadAmount,strlen((char *)srTxnData.srIngData.anAutoloadAmount) - 2);
		//vdUIntToAsc(DongleReqOut->ucAutoLoadAmt,sizeof(DongleReqOut->ucAutoLoadAmt),bBuf,5,TRUE,0x00,10);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0409,bBuf,fXMLSendFlag);
		//inXMLAppendData(srXML,(char *)TAG_NAME_0409,strlen(TAG_NAME_0409),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}

	//T0410
	memset(bBuf,0x00,sizeof(bBuf));
	vdIntToAsc(DongleReqOut->ucEV,sizeof(DongleReqOut->ucEV),bBuf,5,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0410,bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0410,strlen(TAG_NAME_0410),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1100
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_1100);

	//T1101
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_1101);

	memset(bDate,0x00,sizeof(bDate));
	UnixToDateTime((BYTE*)DongleOut->stDeductOut_t.ucTxnDateTime,(BYTE*)bDate,14);
	//T1200
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1200,&bDate[8],TRUE);

	//T1201
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T1201",&bDate[8],TRUE);
	bDate[8] = 0x00;
	//vdSetConfigFlag(srXML,FALSE,(char *)"T1201");

	//T1300
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1300,bDate,TRUE);
	//vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_1300);

	//T1301
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T1301",bDate,TRUE);
	//vdSetConfigFlag(srXML,FALSE,(char *)"T1301");

	//T3700
	vdCMASBuildRRNProcess(srXML,srTxnData.srIngData.ulCMASHostSerialNumber,DongleReqOut->ucTxnSN);

	//T4100
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4100);

	//T4101
	//if(DongleReqOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(DongleReqOut->ucDeviceID,sizeof(DongleReqOut->ucDeviceID),srXML,(char *)"T4101",strlen("T4101"),VALUE_NAME,fXMLSendFlag);

	//T4104
	memset(bBuf,0x00,sizeof(bBuf));
	if(DongleReqOut->ucPurseVersionNumber == MIFARE)
	{
		fnUnPack(&DongleOut->stDeductOut_t.ucMAC_HCrypto[6],4,bBuf);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4104",bBuf,fXMLSendFlag);
	}
	else
	{
		vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4104);
		//sprintf((char *)bBuf,"00000000");
	}

	//T4107
	if(strlen(srTxnData.srParameter.chPOS_ID) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4107,(BYTE *)srTxnData.srParameter.chPOS_ID,fXMLSendFlag);

	//T4108 off
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4108);

	//T4117
	if(strlen(srTxnData.srParameter.chPOS_ID2) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4117,(BYTE *)srTxnData.srParameter.chPOS_ID2,fXMLSendFlag);

	//T4200
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4200);

	//T4210
	vdSetConfigFlag(srXML,FALSE,(char *)"T4210");

	//T4800
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4800);

	//T4801,33 byte unpack to 66 byte,後補0至90 byte
	vdBuildCMASCardAVR((BYTE *)&DongleReqOut->stLastCreditTxnLogInfo_t,sizeof(DongleReqOut->stLastCreditTxnLogInfo_t),srXML,(char *)"T4801",strlen("T4801"),VALUE_NAME,fXMLSendFlag);

	//T4802
	vdUnPackToXMLData(&DongleReqOut->ucIssuerCode,sizeof(DongleReqOut->ucIssuerCode),srXML,(char *)"T4802",strlen("T4802"),VALUE_NAME,fXMLSendFlag);

	//T4803
	vdUnPackToXMLData(&DongleReqOut->ucBankCode,sizeof(DongleReqOut->ucBankCode),srXML,(char *)TAG_NAME_4803,strlen(TAG_NAME_4803),VALUE_NAME,fXMLSendFlag);

	//T4804
	vdUnPackToXMLData(&DongleReqOut->ucAreaCode,sizeof(DongleReqOut->ucAreaCode),srXML,(char *)TAG_NAME_4804,strlen(TAG_NAME_4804),VALUE_NAME,fXMLSendFlag);

	//T4805
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleReqOut->ucSubAreaCode,sizeof(DongleReqOut->ucSubAreaCode),srXML,(char *)TAG_NAME_4805,strlen(TAG_NAME_4805),VALUE_NAME,fXMLSendFlag);

	//T4808
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->stDeductOut_t.ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4808",bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T4808",strlen("T4808"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),srXML,(char *)"T4808",strlen("T4808"),VALUE_NAME,fXMLSendFlag);

	//T4809
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->stDeductOut_t.ucTxnMode,sizeof(DongleOut->stDeductOut_t.ucTxnMode),srXML,(char *)"T4809",strlen("T4809"),VALUE_NAME,fXMLSendFlag);

	//T4810
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->stDeductOut_t.ucTxnQuqlifier,sizeof(DongleOut->stDeductOut_t.ucTxnQuqlifier),srXML,(char *)"T4810",strlen("T4810"),VALUE_NAME,fXMLSendFlag);

	//T4811
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleReqOut->ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4811",bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T4811",strlen("T4811"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleReqOut->ucTxnSN,sizeof(DongleReqOut->ucTxnSN),srXML,(char *)"T4811",strlen("T4811"),VALUE_NAME,fXMLSendFlag);

	//T4812
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->stDeductOut_t.ucCTC,sizeof(DongleOut->stDeductOut_t.ucCTC),srXML,(char *)"T4812",strlen("T4812"),VALUE_NAME,fXMLSendFlag);

	//T4813
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleReqOut->ucLoyaltyCounter,2,bBuf,6,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4813",bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T4813",strlen("T4813"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleReqOut->ucLoyaltyCounter,sizeof(DongleReqOut->ucLoyaltyCounter),srXML,(char *)"T4813",strlen("T4813"),VALUE_NAME,fXMLSendFlag);

	//T4826
	vdSetConfigFlag(srXML,FALSE,(char *)"T4826");

	//T4833	//轉乘群組代碼
	vdUnPackToXMLData(&DongleOut->ucTransferGuoupCode,sizeof(DongleOut->ucTransferGuoupCode),srXML,(char *)TAG_NAME_4833,strlen(TAG_NAME_4833),VALUE_NAME,fXMLSendFlag);

	//T4834	//新轉乘組代碼
	vdUnPackToXMLData(DongleOut->ucTransferGuoupCode2,sizeof(DongleOut->ucTransferGuoupCode2),srXML,(char *)TAG_NAME_4834,strlen(TAG_NAME_4834),VALUE_NAME,fXMLSendFlag);
	
	//T4835	//轉乘折扣金額
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleIn->ucTransferDiscount,sizeof(DongleIn->ucTransferDiscount),bBuf,8,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4835,bBuf,fXMLSendFlag);
	//vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4835);

	//T4836
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4836);

	//T4837
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4837);

	//T4838
	//vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4838);

	//T5301
	//vdUnPackToXMLData(&DongleReqOut->ucSAMKVN,sizeof(DongleReqOut->ucSAMKVN),srXML,(char *)"T5301",strlen("T5301"),VALUE_NAME,fXMLSendFlag);

	//T5303
	//if(DongleReqOut->ucPurseVersionNumber != MIFARE)
	vdUnPackToXMLData(&DongleOut->ucHashType,1,srXML,(char *)"T5303",strlen("T5303"),VALUE_NAME,fXMLSendFlag);

	//T5304
	//if(DongleReqOut->ucPurseVersionNumber != MIFARE)
	vdUnPackToXMLData(&DongleOut->ucHostAdminKVN,1,srXML,(char *)"T5304",strlen("T5304"),VALUE_NAME,fXMLSendFlag);

	//T5305
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucSignatureKeyKVN,sizeof(DongleOut->ucSignatureKeyKVN),srXML,(char *)"T5305",strlen("T5305"),VALUE_NAME,fXMLSendFlag);

	//T5361
	//vdUnPackToXMLData(DongleReqOut->ucSAMID,sizeof(DongleReqOut->ucSAMID),srXML,(char *)"T5361",strlen("T5361"),VALUE_NAME,fXMLSendFlag);

	//T5362
	//vdUnPackToXMLData(DongleReqOut->ucSAMSN,sizeof(DongleReqOut->ucSAMSN),srXML,(char *)"T5362",strlen("T5362"),VALUE_NAME,fXMLSendFlag);

	//T5363
	//vdUnPackToXMLData(DongleReqOut->ucSAMCRN,sizeof(DongleReqOut->ucSAMCRN),srXML,(char *)"T5363",strlen("T5363"),VALUE_NAME,fXMLSendFlag);

	//T5371
	vdUnPackToXMLData(DongleOut->ucCPUSAMID,sizeof(DongleOut->ucCPUSAMID),srXML,(char *)"T5371",strlen("T5371"),VALUE_NAME,fXMLSendFlag);

	//T5501
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_5501);

	//T5503
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_5503);

	//T5504
	vdSetConfigFlag(srXML,FALSE,(char *)"T5504");

	//T5510
	vdSetConfigFlag(srXML,FALSE,(char *)"T5510");

	//T5533
	//vdSetPointDetail(inTxnType,srXML);

	//T6000
	vdUnPackToXMLData(gTmpData.ucReaderFWVersion,sizeof(gTmpData.ucReaderFWVersion),srXML,(char *)"T6000",strlen("T6000"),VALUE_NAME,fXMLSendFlag);

	//T6402
	//if(DongleReqOut->ucPurseVersionNumber == MIFARE)
	//	vdUnPackToXMLData(&DongleOut->stDeductOut_t.ucMAC_HCrypto[10],8,srXML,(char *)"T6402",strlen("T6402"),VALUE_NAME,fXMLSendFlag);

	//T6403
	if(DongleReqOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(&DongleOut->stDeductOut_t.ucMAC_HCrypto[2],4,srXML,(char *)"T6403",strlen("T6403"),VALUE_NAME,fXMLSendFlag);

	//T6404
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
	{
		vdUnPackToXMLData(&DongleOut->stDeductOut_t.ucMAC_HCrypto[0],16,srXML,(char *)"T6404",strlen("T6404"),VALUE_NAME,fXMLSendFlag);
		vdUnPackToXMLData(DongleOut->stDeductOut_t.ucSignature,16,srXML,(char *)"T6405",strlen("T6405"),VALUE_NAME,fXMLSendFlag);
	}

	return SUCCESS;
}

void vdSetPointDetail(int inTxnType,STRUCT_XML_DOC *srXML)
{
	BYTE bBuf[50];

	memset(bBuf,0x00,sizeof(bBuf));
	inXMLAppendData(srXML,(char *)"T5533",strlen("T5533"),bBuf,strlen((char *)bBuf),TAG_NAME_START,TRUE);
	inXMLAppendData(srXML,(char *)"T553301",strlen("T553301"),(BYTE *)"001-00001",9,VALUE_NAME,TRUE);
	inXMLAppendData(srXML,(char *)"T553302",strlen("T553302"),(BYTE *)"20130101",8,VALUE_NAME,TRUE);
	inXMLAppendData(srXML,(char *)"T553303",strlen("T553303"),(BYTE *)"20991231",8,VALUE_NAME,TRUE);
	inXMLAppendData(srXML,(char *)"T553304",strlen("T553304"),(BYTE *)"0",1,VALUE_NAME,TRUE);
	inXMLAppendData(srXML,(char *)"T553305",strlen("T553305"),(BYTE *)"2",1,VALUE_NAME,TRUE);
	inXMLAppendData(srXML,(char *)"T553306",strlen("T553306"),(BYTE *)"EV Before",9,VALUE_NAME,TRUE);
	inXMLAppendData(srXML,(char *)"T553307",strlen("T553307"),(BYTE *)"001",3,VALUE_NAME,TRUE);
	inXMLAppendData(srXML,(char *)"T553308",strlen("T553308"),(BYTE *)"0",1,VALUE_NAME,TRUE);
	inXMLAppendData(srXML,(char *)"T553309",strlen("T553309"),(BYTE *)"Point",5,VALUE_NAME,TRUE);
	inXMLAppendData(srXML,(char *)"T553310",strlen("T553310"),(BYTE *)"120",3,VALUE_NAME,TRUE);
	inXMLAppendData(srXML,(char *)"/T5533",strlen("/T5533"),bBuf,strlen((char *)bBuf),TAG_NAME_END,TRUE);
}

int inBuildCMASSettleTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
//TaxiDeduct_APDU_In *DongleIn = (TaxiDeduct_APDU_In *)Dongle_In;
//TaxiDeduct_APDU_Out *DongleOut = (TaxiDeduct_APDU_Out *)Dongle_Out;
//TaxiRead_APDU_Out *DongleReqOut = (TaxiRead_APDU_Out *)Dongle_ReqOut;
BOOL fXMLSendFlag = TRUE;
int inCntStart,inCntEnd;
//STRUCT_TMP_DATA srTmpData;
BYTE bBuf[20],bTmp[50];
//long lnTotalAmt;

	inGetDeviceID(srTxnData.srIngData.anDeviceID/*,&gTmpData*/);
	if(inXMLSearchValueByTag(srXML,(char *)TAG_TRANS_XML_END,&inCntStart,&inCntEnd,0) >= SUCCESS)//有 /TransXML
		inXMLDeleteData(srXML,inCntStart,1);

	if(inXMLSearchValueByTag(srXML,(char *)TAG_TRANS_END,&inCntStart,&inCntEnd,0) >= SUCCESS)//有 /TRANS
		inXMLDeleteData(srXML,inCntStart,1);

	//TAG_TRANS_XML_HEADER
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_TRANS_XML_HEADER);

	//TAG_TRANS_HEADER
	if(inXMLSearchValueByTag(srXML,(char *)TAG_TRANS_HEADER,&inCntStart,&inCntEnd,0) >= SUCCESS)
	{
		srXML->srXMLElement[inCntStart].fXMLSendFlag = TRUE;
		//vdSetConfigFlag(srXML,FALSE,TAG_TRANS_HEADER);
		memcpy(srXMLData.srXMLElement[inCntStart].chTag,TAG_TRANS_HEADER,strlen(TAG_TRANS_HEADER));
	}

	//T0100
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0100,(BYTE *)"0500",TRUE);

	//T0300
	if(inTxnType == TXN_ECC_SETTLE)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"900000",TRUE);
	else //if(inTxnType == TXN_ECC_SETTLE2)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"910000",TRUE);

	//T1100
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulCMASHostSerialNumber);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1100,bBuf,fXMLSendFlag);

	//T1101
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulTMSerialNumber);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1101,bBuf,fXMLSendFlag);

	//T3700
	memset(bBuf,0x00,sizeof(bBuf));
	vdCMASBuildRRNProcess(srXML,srTxnData.srIngData.ulCMASHostSerialNumber,bBuf);

	//T4100
	vdUnPackToXMLData(gTmpData.ucCPUDeviceID,sizeof(gTmpData.ucCPUDeviceID),srXML,(char *)TAG_NAME_4100,strlen(TAG_NAME_4100),VALUE_NAME,fXMLSendFlag);

	//T4101
	vdUnPackToXMLData(gTmpData.ucDeviceID,sizeof(gTmpData.ucDeviceID),srXML,(char *)"T4101",strlen("T4101"),VALUE_NAME,fXMLSendFlag);

	//T4104
	vdUnPackToXMLData(gTmpData.ucReaderID,sizeof(gTmpData.ucReaderID),srXML,(char *)"T4104",strlen("T4104"),VALUE_NAME,fXMLSendFlag);

	//T4200//銀行版-->TM Location ID,一般版-->New SP ID,商店模式-->TM Location ID
	//if(srTxnData.srParameter.chCMASMode == '1' || srTxnData.srParameter.chCMASMode == '2')//銀行併機 or 商店模式
	if(srTxnData.srParameter.chCMASMode == '1')//銀行併機
	{
		if(srTxnData.srParameter.gDLLVersion == 1)
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMLocationID,TRUE);
		else
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,(BYTE *)&srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID[2],TRUE);
	}
	else if (srTxnData.srParameter.chCMASMode == '2')//商店模式
	{
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,(BYTE *)&srTxnData.srREQData.bT5503[2],TRUE);
	}
	else//一般版
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(gTmpData.ucCPUSPID,3,bBuf,8,FALSE,'0',10);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,bBuf,fXMLSendFlag);
		//inXMLAppendData(srXML,(char *)TAG_NAME_4200,strlen(TAG_NAME_4200),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}

	//T4210
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(gTmpData.ucCPULocationID,2,bBuf,5,TRUE,0x00,10);
	inXMLAppendData(srXML,(char *)"T4210",strlen("T4210"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(srTmpData.ucCPULocationID,sizeof(srTmpData.ucCPULocationID),srXML,(char *)"T4210",strlen("T4210"),VALUE_NAME,fXMLSendFlag);

	//T5501
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_5501);

	//T5503
	if (srTxnData.srParameter.chCMASMode == '2')	//@ 商店卡模式以使用者輸入的T5503帶入
	{
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5503",(BYTE *)srTxnData.srREQData.bT5503,TRUE);
	}
	else
	{
	if(srTxnData.srParameter.gDLLVersion == 1)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5503",(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMLocationID,TRUE);
	else
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5503",(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID,TRUE);
	}

	//T5504
	if(srTxnData.srParameter.gDLLVersion == 1)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5504",(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMID,TRUE);
	else
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5504",(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMID,TRUE);

	//T5510
	if(srTxnData.srParameter.gDLLVersion == 1)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5510",(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMAgentNumber,TRUE);
	else
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5510",(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber,TRUE);

	//T5591
	vdSetConfigFlag(srXML,FALSE,(char *)"T5591");

	//T5592
	inXMLGetAmt(&srXMLData,(char *)"T559201",&srTxnData.srIngData.lnTotalAmt,0);
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%ld",srTxnData.srIngData.lnTotalAmt);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T559201",bBuf,fXMLSendFlag);

	//T6000
	vdUnPackToXMLData(gTmpData.ucReaderFWVersion,sizeof(gTmpData.ucReaderFWVersion),srXML,(char *)"T6000",strlen("T6000"),VALUE_NAME,fXMLSendFlag);

	//T6004
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	inGetBLCName(bTmp);
	memcpy(bBuf,bTmp,5);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T6004",bBuf,fXMLSendFlag);

	// /TRANS
	inXMLAppendData(srXML,(char *)TAG_TRANS_END,strlen(TAG_TRANS_END),bBuf,0,TAG_NAME_END,TRUE);

	// /TransXML
	inXMLAppendData(srXML,(char *)TAG_TRANS_XML_END,strlen(TAG_TRANS_XML_END),bBuf,0,TAG_NAME_END,TRUE);

	return SUCCESS;
}

int inBuildCMASLockTM1(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
BYTE bBuf[50],bTmp[50];
//AuthTxnOnline_APDU_In *DongleIn = (AuthTxnOnline_APDU_In *)Dongle_In;
LockCard_APDU_Out_3 *DongleOut = (LockCard_APDU_Out_3 *)Dongle_Out;
//TxnReqOnline_APDU_Out *DongleReqOut = (TxnReqOnline_APDU_Out *)Dongle_ReqOut;
BOOL fXMLSendFlag = TRUE;
int inCntStart,inCntEnd;

	//TAG_TRANS_XML_HEADER
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_TRANS_XML_HEADER);

	//TAG_TRANS_HEADER
	if(inXMLSearchValueByTag(srXML,(char *)TAG_TRANS_HEADER,&inCntStart,&inCntEnd,0) >= SUCCESS)
	{
		srXML->srXMLElement[inCntStart].fXMLSendFlag = TRUE;
		//vdSetConfigFlag(srXML,FALSE,TAG_TRANS_HEADER);
		memcpy(srXMLData.srXMLElement[inCntStart].chTag,TAG_TRANS_HEADER,strlen(TAG_TRANS_HEADER));
	}

	//T0100
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0100,(BYTE *)"0320",TRUE);

	//T0200
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdUIntToAsc(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),bTmp,17,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,17);
	inXMLAppendData(srXML,(char *)TAG_NAME_0200,strlen(TAG_NAME_0200),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0200);

	//T0211
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucPID,sizeof(DongleOut->ucPID),srXML,(char *)TAG_NAME_0211,strlen(TAG_NAME_0211),VALUE_NAME,fXMLSendFlag);
		//vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0211);

	//T0213
	vdUnPackToXMLData(&DongleOut->ucCardType,sizeof(DongleOut->ucCardType),srXML,(char *)TAG_NAME_0213,strlen(TAG_NAME_0213),VALUE_NAME,fXMLSendFlag);

	//T0214
	vdUnPackToXMLData(&DongleOut->ucPersonalProfile,sizeof(DongleOut->ucPersonalProfile),srXML,(char *)TAG_NAME_0214,strlen(TAG_NAME_0214),VALUE_NAME,fXMLSendFlag);

	//T0300
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"596100",TRUE);

	//T0410
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),bBuf,5,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0410,bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0410,strlen(TAG_NAME_0410),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1100
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulCMASHostSerialNumber);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1100,bBuf,fXMLSendFlag);

	//T1101
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulTMSerialNumber);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1101,bBuf,fXMLSendFlag);

	//T1200//T1201
	memset(bBuf,0x00,sizeof(bBuf));
	memcpy((char *)bBuf,srTxnData.srIngData.chTxTime,sizeof(srTxnData.srIngData.chTxTime) - 1);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1200,bBuf,fXMLSendFlag);
	inXMLAppendData(srXML,(char *)"T1201",strlen("T1201"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1300//T1301
	memset(bBuf,0x00,sizeof(bBuf));
	memcpy((char *)bBuf,srTxnData.srIngData.chTxDate,sizeof(srTxnData.srIngData.chTxDate) - 1);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1300,bBuf,fXMLSendFlag);
	inXMLAppendData(srXML,(char *)"T1301",strlen("T1301"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T3700
	memset(bBuf,0x00,sizeof(bBuf));
	vdCMASBuildRRNProcess(srXML,srTxnData.srIngData.ulCMASHostSerialNumber,bBuf);

	//T4100
	vdUnPackToXMLData(DongleOut->ucCPUDeviceID,sizeof(DongleOut->ucCPUDeviceID),srXML,(char *)TAG_NAME_4100,strlen(TAG_NAME_4100),VALUE_NAME,fXMLSendFlag);
	//vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4100);

	//T4101
	//if(DongleReqOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(DongleOut->ucDeviceID,sizeof(DongleOut->ucDeviceID),srXML,(char *)"T4101",strlen("T4101"),VALUE_NAME,fXMLSendFlag);

	//T4104
	vdUnPackToXMLData(gTmpData.ucReaderID,sizeof(gTmpData.ucReaderID),srXML,(char *)TAG_NAME_4104,strlen(TAG_NAME_4104),VALUE_NAME,fXMLSendFlag);

	//T4107
	if(strlen(srTxnData.srParameter.chPOS_ID) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4107,(BYTE *)srTxnData.srParameter.chPOS_ID,fXMLSendFlag);

	//T4108 off
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4108);

	//T4109 off
	memset(bBuf,0x00,sizeof(bBuf));
	fnBINTODEVASC(DongleOut->ucDeviceID,bBuf,9,MIFARE);//n_TXN Device ID
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4109",bBuf,FALSE);

	//T4110 off
	memset(bBuf,0x00,sizeof(bBuf));
	fnBINTODEVASC(DongleOut->ucCPUDeviceID,bBuf,16,LEVEL2);//n_TXN Device ID
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4110",bBuf,FALSE);

	//T4117
	if(strlen(srTxnData.srParameter.chPOS_ID2) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4117,(BYTE *)srTxnData.srParameter.chPOS_ID2,fXMLSendFlag);

	//T4200//銀行版-->TM Location ID,一般版-->New SP ID,商店模式-->TM Location ID
	if(srTxnData.srParameter.chCMASMode == '1' || srTxnData.srParameter.chCMASMode == '2')//銀行併機 or 商店模式
	{
		if(srTxnData.srParameter.gDLLVersion == 1)
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMLocationID,TRUE);
		else
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,(BYTE *)&srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID[2],TRUE);
	}
	else//一般版
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(DongleOut->ucCPUSPID,sizeof(DongleOut->ucCPUSPID),bBuf,8,FALSE,'0',10);
		//vdUIntToAsc(&DongleOut->ucCPUDeviceID[3],3,bBuf,8,FALSE,'0',10);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,bBuf,fXMLSendFlag);
	}

	//T4210
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucCPULocationID,2,bBuf,5,TRUE,0x00,10);
	inXMLAppendData(srXML,(char *)"T4210",strlen("T4210"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdSetConfigFlag(srXML,FALSE,(char *)"T4210");

	//T4800
	vdUnPackToXMLData(&DongleOut->ucPurseVersionNumber,sizeof(DongleOut->ucPurseVersionNumber),srXML,(char *)TAG_NAME_4800,strlen(TAG_NAME_4800),VALUE_NAME,fXMLSendFlag);
	//vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4800);

	//T4801,33 byte unpack to 66 byte,後補0至90 byte
	vdBuildCMASCardAVR((BYTE *)&DongleOut->stLastCreditTxnLogInfo_t,sizeof(DongleOut->stLastCreditTxnLogInfo_t),srXML,(char *)"T4801",strlen("T4801"),VALUE_NAME,fXMLSendFlag);

	//T4802
	vdUnPackToXMLData(&DongleOut->ucIssuerCode,sizeof(DongleOut->ucIssuerCode),srXML,(char *)"T4802",strlen("T4802"),VALUE_NAME,fXMLSendFlag);

	//T4803
	vdUnPackToXMLData(&DongleOut->ucBankCode,sizeof(DongleOut->ucBankCode),srXML,(char *)TAG_NAME_4803,strlen(TAG_NAME_4803),VALUE_NAME,fXMLSendFlag);

	//T4804
	vdUnPackToXMLData(&DongleOut->ucAreaCode,sizeof(DongleOut->ucAreaCode),srXML,(char *)TAG_NAME_4804,strlen(TAG_NAME_4804),VALUE_NAME,fXMLSendFlag);

	//T4805
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucSubAreaCode,sizeof(DongleOut->ucSubAreaCode),srXML,(char *)TAG_NAME_4805,strlen(TAG_NAME_4805),VALUE_NAME,fXMLSendFlag);

	//T4806
	vdUnPackToXMLData(DongleOut->ucProfileExpiryDate,sizeof(DongleOut->ucProfileExpiryDate),srXML,(char *)TAG_NAME_4806,strlen(TAG_NAME_4806),VALUE_NAME,fXMLSendFlag);

	//T4812
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4812",(BYTE *)"000000",TRUE);

	//T4814
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucDeposit,sizeof(DongleOut->ucDeposit),bBuf,5,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4814,bBuf,fXMLSendFlag);

	//T4818
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucEV2,sizeof(DongleOut->ucEV2),bBuf,5,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4818,bBuf,fXMLSendFlag);

	//T4819
	vdUnPackToXMLData(&DongleOut->ucStatusCode[1],1,srXML,(char *)"T4819",strlen("T4819"),VALUE_NAME,fXMLSendFlag);

	//T4826
	vdUnPackToXMLData(&DongleOut->ucCardIDLen,sizeof(DongleOut->ucCardIDLen),srXML,(char *)"T4826",strlen("T4826"),VALUE_NAME,fXMLSendFlag);
	//vdSetConfigFlag(srXML,FALSE,(char *)"T4826");

	//T4828
	memset(bBuf,0x00,sizeof(bBuf));
	bBuf[0] = DongleOut->ucMifareSettingParameter;
	vdUnPackToXMLData(bBuf,10,srXML,(char *)"T4828",strlen("T4828"),VALUE_NAME,fXMLSendFlag);

	//T4829
	memset(bBuf,0x00,sizeof(bBuf));
	bBuf[0] = DongleOut->ucCPUSettingParameter;
	vdUnPackToXMLData(bBuf,16,srXML,(char *)"T4829",strlen("T4829"),VALUE_NAME,fXMLSendFlag);

	//T5364

	//T5501
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_5501);

	//T5503
	if(srTxnData.srParameter.gDLLVersion == 1)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5503",(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMLocationID,TRUE);
	else
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5503",(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID,TRUE);

	//T5504
	if(srTxnData.srParameter.gDLLVersion == 1)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5504",(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMID,TRUE);
	else
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5504",(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMID,TRUE);

	//T5510
	if(srTxnData.srParameter.gDLLVersion == 1)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5510",(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMAgentNumber,TRUE);
	else
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5510",(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber,TRUE);

	return SUCCESS;
}

int inBuildCMASLockTM2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
BYTE bBuf[50],bTmp[50];
//AuthTxnOnline_APDU_In *DongleIn = (AuthTxnOnline_APDU_In *)Dongle_In;
LockCard_APDU_Out_2 *DongleOut = (LockCard_APDU_Out_2 *)Dongle_Out;
//TxnReqOnline_APDU_Out *DongleReqOut = (TxnReqOnline_APDU_Out *)Dongle_ReqOut;
BOOL fXMLSendFlag = TRUE;
int inCntStart,inCntEnd;

	//TAG_TRANS_XML_HEADER
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_TRANS_XML_HEADER);

	//TAG_TRANS_HEADER
	if(inXMLSearchValueByTag(srXML,(char *)TAG_TRANS_HEADER,&inCntStart,&inCntEnd,0) >= SUCCESS)
	{
		srXML->srXMLElement[inCntStart].fXMLSendFlag = TRUE;
		//vdSetConfigFlag(srXML,FALSE,TAG_TRANS_HEADER);
		memcpy(srXMLData.srXMLElement[inCntStart].chTag,TAG_TRANS_HEADER,strlen(TAG_TRANS_HEADER));
	}

	//T0100
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0100,(BYTE *)"0320",TRUE);

	//T0200
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdUIntToAsc(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),bTmp,17,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,17);
	inXMLAppendData(srXML,(char *)TAG_NAME_0200,strlen(TAG_NAME_0200),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0200);

	//T0211
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucPID,sizeof(DongleOut->ucPID),srXML,(char *)TAG_NAME_0211,strlen(TAG_NAME_0211),VALUE_NAME,fXMLSendFlag);
		//vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0211);

	//T0213
	vdUnPackToXMLData(&DongleOut->ucCardType,sizeof(DongleOut->ucCardType),srXML,(char *)TAG_NAME_0213,strlen(TAG_NAME_0213),VALUE_NAME,fXMLSendFlag);

	//T0214
	vdUnPackToXMLData(&DongleOut->ucPersonalProfile,sizeof(DongleOut->ucPersonalProfile),srXML,(char *)TAG_NAME_0214,strlen(TAG_NAME_0214),VALUE_NAME,fXMLSendFlag);

	//T0300
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"596100",TRUE);

	//T0410
/*	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),bBuf,5,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0410,bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0410,strlen(TAG_NAME_0410),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
*/

	//T1100
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulCMASHostSerialNumber);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1100,bBuf,fXMLSendFlag);

	//T1101
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulTMSerialNumber);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1101,bBuf,fXMLSendFlag);

	//T1200//T1201
	memset(bBuf,0x00,sizeof(bBuf));
	memcpy((char *)bBuf,srTxnData.srIngData.chTxTime,sizeof(srTxnData.srIngData.chTxTime) - 1);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1200,bBuf,fXMLSendFlag);
	inXMLAppendData(srXML,(char *)"T1201",strlen("T1201"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1300//T1301
	memset(bBuf,0x00,sizeof(bBuf));
	memcpy((char *)bBuf,srTxnData.srIngData.chTxDate,sizeof(srTxnData.srIngData.chTxDate) - 1);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1300,bBuf,TRUE);
	inXMLAppendData(srXML,(char *)"T1301",strlen("T1301"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T3700
	memset(bBuf,0x00,sizeof(bBuf));
	vdCMASBuildRRNProcess(srXML,srTxnData.srIngData.ulCMASHostSerialNumber,bBuf);

	//T4100
	vdUnPackToXMLData(DongleOut->ucCPUDeviceID,sizeof(DongleOut->ucCPUDeviceID),srXML,(char *)TAG_NAME_4100,strlen(TAG_NAME_4100),VALUE_NAME,fXMLSendFlag);
	//vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4100);

	//T4101
	//if(DongleReqOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(DongleOut->ucDeviceID,sizeof(DongleOut->ucDeviceID),srXML,(char *)"T4101",strlen("T4101"),VALUE_NAME,fXMLSendFlag);

	//T4104
	vdUnPackToXMLData(gTmpData.ucReaderID,sizeof(gTmpData.ucReaderID),srXML,(char *)TAG_NAME_4104,strlen(TAG_NAME_4104),VALUE_NAME,fXMLSendFlag);

	//T4107
	if(strlen(srTxnData.srParameter.chPOS_ID) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4107,(BYTE *)srTxnData.srParameter.chPOS_ID,fXMLSendFlag);

	//T4108 off
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4108);

	//T4109 off
	memset(bBuf,0x00,sizeof(bBuf));
	fnBINTODEVASC(DongleOut->ucDeviceID,bBuf,9,MIFARE);//n_TXN Device ID
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4109",bBuf,FALSE);

	//T4110 off
	memset(bBuf,0x00,sizeof(bBuf));
	fnBINTODEVASC(DongleOut->ucCPUDeviceID,bBuf,16,LEVEL2);//n_TXN Device ID
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4110",bBuf,FALSE);

	//T4117
	if(strlen(srTxnData.srParameter.chPOS_ID2) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4117,(BYTE *)srTxnData.srParameter.chPOS_ID2,fXMLSendFlag);

	//T4200//銀行版-->TM Location ID,一般版-->New SP ID,商店模式-->TM Location ID
	if(srTxnData.srParameter.chCMASMode == '1' || srTxnData.srParameter.chCMASMode == '2')//銀行併機 or 商店模式
	{
		if(srTxnData.srParameter.gDLLVersion == 1)
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMLocationID,TRUE);
		else
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,(BYTE *)&srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID[2],TRUE);
	}
	else//一般版
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(DongleOut->ucCPUSPID,sizeof(DongleOut->ucCPUSPID),bBuf,8,FALSE,'0',10);
		//vdUIntToAsc(&DongleOut->ucCPUDeviceID[3],3,bBuf,8,FALSE,'0',10);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,bBuf,fXMLSendFlag);
	}

	//T4210
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucCPULocationID,2,bBuf,5,TRUE,0x00,10);
	inXMLAppendData(srXML,(char *)"T4210",strlen("T4210"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdSetConfigFlag(srXML,FALSE,(char *)"T4210");

	//T4800
	vdUnPackToXMLData(&DongleOut->ucPurseVersionNumber,sizeof(DongleOut->ucPurseVersionNumber),srXML,(char *)TAG_NAME_4800,strlen(TAG_NAME_4800),VALUE_NAME,fXMLSendFlag);
	//vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4800);

	//T4801,33 byte unpack to 66 byte,後補0至90 byte
	//vdBuildCMASCardAVR((BYTE *)&DongleOut->stLastCreditTxnLogInfo_t,sizeof(DongleOut->stLastCreditTxnLogInfo_t),srXML,(char *)"T4801",strlen("T4801"),VALUE_NAME,fXMLSendFlag);

	//T4802
	vdUnPackToXMLData(&DongleOut->ucIssuerCode,sizeof(DongleOut->ucIssuerCode),srXML,(char *)"T4802",strlen("T4802"),VALUE_NAME,fXMLSendFlag);

	//T4803 off
	//vdUnPackToXMLData(&DongleOut->ucBankCode,sizeof(DongleOut->ucBankCode),srXML,(char *)TAG_NAME_4803,strlen(TAG_NAME_4803),VALUE_NAME,fXMLSendFlag);
	if(srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT_TAXI || srTxnData.srIngData.inTransType == TXN_ECC_READ_TAXI ||
	   srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT_CBIKE || srTxnData.srIngData.inTransType == TXN_ECC_READ_CBIKE ||
	   srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT_EDCA || srTxnData.srIngData.inTransType == TXN_ECC_READ_EDCA)
		vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4803);

	//T4804 off
	//vdUnPackToXMLData(&DongleOut->ucAreaCode,sizeof(DongleOut->ucAreaCode),srXML,(char *)TAG_NAME_4804,strlen(TAG_NAME_4804),VALUE_NAME,fXMLSendFlag);
	if(srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT_TAXI || srTxnData.srIngData.inTransType == TXN_ECC_READ_TAXI ||
	   srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT_CBIKE || srTxnData.srIngData.inTransType == TXN_ECC_READ_CBIKE ||
	   srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT_EDCA || srTxnData.srIngData.inTransType == TXN_ECC_READ_EDCA)
		vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4804);

	//T4805
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
	//	vdUnPackToXMLData(DongleOut->ucSubAreaCode,sizeof(DongleOut->ucSubAreaCode),srXML,(char *)TAG_NAME_4805,strlen(TAG_NAME_4805),VALUE_NAME,fXMLSendFlag);

	//T4806
	//vdUnPackToXMLData(DongleOut->ucProfileExpiryDate,sizeof(DongleOut->ucProfileExpiryDate),srXML,(char *)TAG_NAME_4806,strlen(TAG_NAME_4806),VALUE_NAME,fXMLSendFlag);

	//T4812
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4812",(BYTE *)"000000",TRUE);

	//T4814
/*	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucDeposit,sizeof(DongleOut->ucDeposit),bBuf,5,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4814,bBuf,fXMLSendFlag);
*/
	//T4818
/*	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucEV2,sizeof(DongleOut->ucEV2),bBuf,5,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4818,bBuf,fXMLSendFlag);
*/
	log_msg(LOG_LEVEL_ERROR,"inBuildCMASLockTM2 1 (%02x%02x),%d",DongleOut->ucStatusCode[0],DongleOut->ucStatusCode[1],srTxnData.srIngData.inTransType);
	//T4819
	if(!memcmp(DongleOut->ucStatusCode,"\x61\x03",2))
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4819",(BYTE *)"02",TRUE);
	else if(!memcmp(DongleOut->ucStatusCode,"\x64\x06",2) ||
			srTxnData.srIngData.inTransType == TXN_ECC_READ_BASIC ||
			//srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT_TAXI ||
			srTxnData.srIngData.inTransType == TXN_ECC_READ_TAXI ||
			//srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT_CBIKE ||
			srTxnData.srIngData.inTransType == TXN_ECC_READ_CBIKE ||
			srTxnData.srIngData.inTransType == TXN_ECC_READ_EDCA)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4819",(BYTE *)"01",TRUE);
	else
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4819",(BYTE *)"03",TRUE);

	//T4826
	vdUnPackToXMLData(&DongleOut->ucCardIDLen,sizeof(DongleOut->ucCardIDLen),srXML,(char *)"T4826",strlen("T4826"),VALUE_NAME,fXMLSendFlag);
	//vdSetConfigFlag(srXML,FALSE,(char *)"T4826");

	//T4828
/*	memset(bBuf,0x00,sizeof(bBuf));
	bBuf[0] = DongleOut->ucMifareSettingParameter;
	vdUnPackToXMLData(bBuf,10,srXML,(char *)"T4828",strlen("T4828"),VALUE_NAME,fXMLSendFlag);
*/
	//T4829
/*	memset(bBuf,0x00,sizeof(bBuf));
	bBuf[0] = DongleOut->ucCPUSettingParameter;
	vdUnPackToXMLData(bBuf,16,srXML,(char *)"T4829",strlen("T4829"),VALUE_NAME,fXMLSendFlag);
*/
	//T5364

	//T5501
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_5501);

	//T5503
	if(srTxnData.srParameter.gDLLVersion == 1)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5503",(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMLocationID,TRUE);
	else
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5503",(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID,TRUE);

	//T5504
	if(srTxnData.srParameter.gDLLVersion == 1)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5504",(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMID,TRUE);
	else
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5504",(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMID,TRUE);

	//T5510
	if(srTxnData.srParameter.gDLLVersion == 1)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5510",(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMAgentNumber,TRUE);
	else
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5510",(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber,TRUE);
	return SUCCESS;
}

int inBuildCMSReadDongleDeductTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
BYTE bBuf[50];
//AuthTxnOnline_APDU_In *DongleIn = (AuthTxnOnline_APDU_In *)Dongle_In;
ReadDongleDeduct_APDU_Out *DongleOut = (ReadDongleDeduct_APDU_Out *)Dongle_Out;
//TxnReqOnline_APDU_Out *DongleReqOut = (TxnReqOnline_APDU_Out *)Dongle_ReqOut;
BOOL fXMLSendFlag = TRUE;
int inCntStart,inCntEnd;
BYTE bDate[20];

	//TAG_TRANS_XML_HEADER
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_TRANS_XML_HEADER);

	//TAG_TRANS_HEADER
	if(inXMLSearchValueByTag(srXML,(char *)TAG_TRANS_HEADER,&inCntStart,&inCntEnd,0) >= SUCCESS)
	{
		srXML->srXMLElement[inCntStart].fXMLSendFlag = TRUE;
		//vdSetConfigFlag(srXML,FALSE,TAG_TRANS_HEADER);
		memcpy(srXMLData.srXMLElement[inCntStart].chTag,TAG_TRANS_HEADER,strlen(TAG_TRANS_HEADER));
	}
	//T0100
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0100,(BYTE *)"0220",TRUE);

	//T0200
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0200);

	//T0211
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0211);

	//T0213
	vdUnPackToXMLData(&DongleOut->ucCardType,sizeof(DongleOut->ucCardType),srXML,(char *)TAG_NAME_0213,strlen(TAG_NAME_0213),VALUE_NAME,fXMLSendFlag);

	//T0214
	vdUnPackToXMLData(&DongleOut->ucPersonalProfile,sizeof(DongleOut->ucPersonalProfile),srXML,(char *)TAG_NAME_0214,strlen(TAG_NAME_0214),VALUE_NAME,fXMLSendFlag);

	//T0300
	if(inTxnType == DLL_AUTOLOAD_AUTH || srTxnData.srIngData.inTransType == TXN_ECC_ADD)
	{
		if(srTxnData.srIngData.inTransType == TXN_ECC_ADD)
		{
			if(DongleOut->ucPersonalProfile >= 0x01 && DongleOut->ucPersonalProfile <= 0x03)
				inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"841799",TRUE);
			else
				inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"811799",TRUE);
		}
		else
		{
			//if(DongleOut->ucPersonalProfile >= 0x01 && DongleOut->ucPersonalProfile <= 0x03)
			//	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"845799",TRUE);
			//else
				inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"825799",TRUE);
		}
	}
	else if(inTxnType == DLL_AUTOLOAD_ENABLE_AUTH)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"814799",TRUE);
  	else if(srTxnData.srIngData.inTransType == TXN_ECC_REFUND)
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"851999",TRUE);
	else if(srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"811599",TRUE);
	else if((srTxnData.srIngData.inTransType == TXN_ECC_VOID) || (srTxnData.srIngData.inTransType == TXN_ECC_VOID1))
	{
		if(DongleOut->ucMsgType == 0x0B)//加值
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"811899",TRUE);
		else if(DongleOut->ucMsgType == 0x02 && DongleOut->ucSubType == 0x0A)//購貨
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"823899",TRUE);
	}
	else if(srTxnData.srIngData.inTransType == TXN_ECC_ADD4)
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"871799",TRUE);
	else if(srTxnData.srIngData.inTransType == TXN_ECC_PT_ADD)
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"705699",TRUE);

	//T0400
	memset(bBuf,0x00,sizeof(bBuf));
	if(strlen((char *)srTxnData.srIngData.anAutoloadAmount) > 2)
		memcpy(bBuf,srTxnData.srIngData.anAutoloadAmount,strlen((char *)srTxnData.srIngData.anAutoloadAmount) - 2);
	else
		sprintf((char *)bBuf,"%ld",srTxnData.srIngData.lnECCAmt);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0400,bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0400,strlen(TAG_NAME_0400),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T0408
	memset(bBuf,0x00,sizeof(bBuf));
	vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),bBuf,5,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0408,bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0408,strlen(TAG_NAME_0408),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	srTxnData.srIngData.lnECCEV = lnAmt3ByteToLong(DongleOut->ucEV);

	//T0409
	if(strlen((char *)srTxnData.srIngData.anAutoloadAmount) > 2)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy(bBuf,srTxnData.srIngData.anAutoloadAmount,strlen((char *)srTxnData.srIngData.anAutoloadAmount) - 2);
		//vdUIntToAsc(DongleReqOut->ucAutoLoadAmt,sizeof(DongleReqOut->ucAutoLoadAmt),bBuf,5,TRUE,0x00,10);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0409,bBuf,fXMLSendFlag);
		//inXMLAppendData(srXML,(char *)TAG_NAME_0409,strlen(TAG_NAME_0409),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}

	//T0410
	memset(bBuf,0x00,sizeof(bBuf));
	vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),bBuf,5,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0410,bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0410,strlen(TAG_NAME_0410),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1100
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_1100);

	//T1101
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_1101);

	memset(bDate,0x00,sizeof(bDate));
	UnixToDateTime((BYTE*)DongleOut->ucTxnDateTime,(BYTE*)bDate,14);
	//T1200
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1200,&bDate[8],TRUE);

	//T1201
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T1201",&bDate[8],TRUE);
	bDate[8] = 0x00;
	//vdSetConfigFlag(srXML,FALSE,(char *)"T1201");

	//T1300
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1300,bDate,TRUE);
	//vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_1300);

	//T1301
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T1301",bDate,TRUE);
	//vdSetConfigFlag(srXML,FALSE,(char *)"T1301");

	//T3700//NO
	//vdCMASBuildRRNProcess(srXML,srTxnData.srIngData.ulCMASHostSerialNumber,DongleReqOut->ucTxnSN);

	//T4100
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4100);

	//T4101
	//if(DongleReqOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(DongleOut->ucDeviceID,sizeof(DongleOut->ucDeviceID),srXML,(char *)"T4101",strlen("T4101"),VALUE_NAME,fXMLSendFlag);

	//T4104
	memset(bBuf,0x00,sizeof(bBuf));
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		fnUnPack(&DongleOut->ucMAC_HCrypto[6],4,bBuf);
	else
		sprintf((char *)bBuf,"00000000");
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4104",bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T4104",strlen("T4104"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T4107
	if(strlen(srTxnData.srParameter.chPOS_ID) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4107,(BYTE *)srTxnData.srParameter.chPOS_ID,fXMLSendFlag);

	//T4108 off
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4108);

	//T4109 off
	memset(bBuf,0x00,sizeof(bBuf));
	fnBINTODEVASC(DongleOut->ucDeviceID,bBuf,9,MIFARE);//n_TXN Device ID
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4109",bBuf,FALSE);

	//T4110 off
	memset(bBuf,0x00,sizeof(bBuf));
	fnBINTODEVASC(DongleOut->ucCPUDeviceID,bBuf,16,LEVEL2);//n_TXN Device ID
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4110",bBuf,FALSE);

	//T4117
	if(strlen(srTxnData.srParameter.chPOS_ID2) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4117,(BYTE *)srTxnData.srParameter.chPOS_ID2,fXMLSendFlag);

	//T4200
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4200);

	//T4210
	vdSetConfigFlag(srXML,FALSE,(char *)"T4210");

	//T4800
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4800);

	//T4801,33 byte unpack to 66 byte,後補0至90 byte
	vdBuildCMASCardAVR((BYTE *)&DongleOut->stLastCreditTxnLogInfo_t,sizeof(DongleOut->stLastCreditTxnLogInfo_t),srXML,(char *)"T4801",strlen("T4801"),VALUE_NAME,fXMLSendFlag);

	//T4802
	vdUnPackToXMLData(&DongleOut->ucIssuerCode,sizeof(DongleOut->ucIssuerCode),srXML,(char *)"T4802",strlen("T4802"),VALUE_NAME,fXMLSendFlag);

	//T4803
	vdUnPackToXMLData(&DongleOut->ucBankCode,sizeof(DongleOut->ucBankCode),srXML,(char *)TAG_NAME_4803,strlen(TAG_NAME_4803),VALUE_NAME,fXMLSendFlag);

	//T4804
	vdUnPackToXMLData(&DongleOut->ucAreaCode,sizeof(DongleOut->ucAreaCode),srXML,(char *)TAG_NAME_4804,strlen(TAG_NAME_4804),VALUE_NAME,fXMLSendFlag);

	//T4805
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucSubAreaCode,sizeof(DongleOut->ucSubAreaCode),srXML,(char *)TAG_NAME_4805,strlen(TAG_NAME_4805),VALUE_NAME,fXMLSendFlag);

	//T4808
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4808",bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T4808",strlen("T4808"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),srXML,(char *)"T4808",strlen("T4808"),VALUE_NAME,fXMLSendFlag);

	//T4809
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucTxnMode,sizeof(DongleOut->ucTxnMode),srXML,(char *)"T4809",strlen("T4809"),VALUE_NAME,fXMLSendFlag);

	//T4810
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucTxnQuqlifier,sizeof(DongleOut->ucTxnQuqlifier),srXML,(char *)"T4810",strlen("T4810"),VALUE_NAME,fXMLSendFlag);

	//T4811
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4811",bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T4811",strlen("T4811"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleReqOut->ucTxnSN,sizeof(DongleReqOut->ucTxnSN),srXML,(char *)"T4811",strlen("T4811"),VALUE_NAME,fXMLSendFlag);

	//T4812
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucCTC,sizeof(DongleOut->ucCTC),srXML,(char *)"T4812",strlen("T4812"),VALUE_NAME,fXMLSendFlag);

	//T4813
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucLoyaltyCounter,2,bBuf,6,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4813",bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T4813",strlen("T4813"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleReqOut->ucLoyaltyCounter,sizeof(DongleReqOut->ucLoyaltyCounter),srXML,(char *)"T4813",strlen("T4813"),VALUE_NAME,fXMLSendFlag);

	//T4826
	vdSetConfigFlag(srXML,FALSE,(char *)"T4826");

	//T5301//NO
	//vdUnPackToXMLData(&DongleOut->ucSAMKVN,sizeof(DongleOut->ucSAMKVN),srXML,(char *)"T5301",strlen("T5301"),VALUE_NAME,fXMLSendFlag);

	//T5303
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucMAC_HCrypto[0],1,srXML,(char *)"T5303",strlen("T5303"),VALUE_NAME,fXMLSendFlag);

	//T5304
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucMAC_HCrypto[1],1,srXML,(char *)"T5304",strlen("T5304"),VALUE_NAME,fXMLSendFlag);

	//T5305
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucSignatureKeyKVN,sizeof(DongleOut->ucSignatureKeyKVN),srXML,(char *)"T5305",strlen("T5305"),VALUE_NAME,fXMLSendFlag);

	//T5361//NO
	//vdUnPackToXMLData(DongleOut->ucSAMID,sizeof(DongleOut->ucSAMID),srXML,(char *)"T5361",strlen("T5361"),VALUE_NAME,fXMLSendFlag);

	//T5362//NO
	//vdUnPackToXMLData(DongleOut->ucSAMSN,sizeof(DongleOut->ucSAMSN),srXML,(char *)"T5362",strlen("T5362"),VALUE_NAME,fXMLSendFlag);

	//T5363//NO
	//vdUnPackToXMLData(DongleOut->ucSAMCRN,sizeof(DongleOut->ucSAMCRN),srXML,(char *)"T5363",strlen("T5363"),VALUE_NAME,fXMLSendFlag);

	//T5371
	vdUnPackToXMLData(DongleOut->ucCPUSAMID,sizeof(DongleOut->ucCPUSAMID),srXML,(char *)"T5371",strlen("T5371"),VALUE_NAME,fXMLSendFlag);

	//T5501
	//inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)srTxnData.srIngData.chTxDate,TRUE);
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_5501);

	//T5503
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_5503);

	//T5504
	vdSetConfigFlag(srXML,FALSE,(char *)"T5504");

	//T5510
	vdSetConfigFlag(srXML,FALSE,(char *)"T5510");

	//T6000//NO
	//vdUnPackToXMLData(DongleOut->ucReaderFWVersion,sizeof(DongleOut->ucReaderFWVersion),srXML,(char *)"T6000",strlen("T6000"),VALUE_NAME,fXMLSendFlag);

	//T6001//NO
	//vdUnPackToXMLData((BYTE *)&DongleOut->stReaderAVRInfo_t,83,srXML,(char *)"T6001",strlen("T6001"),VALUE_NAME,fXMLSendFlag);

	//T6402
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(&DongleOut->ucMAC_HCrypto[10],8,srXML,(char *)"T6402",strlen("T6402"),VALUE_NAME,fXMLSendFlag);

	//T6403
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(&DongleOut->ucMAC_HCrypto[2],4,srXML,(char *)"T6403",strlen("T6403"),VALUE_NAME,fXMLSendFlag);

	//T6404
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucMAC_HCrypto[2],16,srXML,(char *)"T6404",strlen("T6404"),VALUE_NAME,fXMLSendFlag);

	//T6405
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucSignature,16,srXML,(char *)"T6405",strlen("T6405"),VALUE_NAME,fXMLSendFlag);

	return SUCCESS;
}

int inBuildCMSAuthTxnOfflineTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
BYTE bBuf[50];
//TxnReqOffline_APDU_In *DongleIn = (TxnReqOffline_APDU_In *)Dongle_In;
AuthTxnOffline_APDU_Out *DongleOut = (AuthTxnOffline_APDU_Out *)Dongle_Out;
TxnReqOffline_APDU_Out *DongleReqOut = (TxnReqOffline_APDU_Out *)Dongle_ReqOut;
BOOL fXMLSendFlag = TRUE;
int inCntStart,inCntEnd;
BYTE bDate[20];

	//TAG_TRANS_XML_HEADER
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_TRANS_XML_HEADER);

	//TAG_TRANS_HEADER
	if(inXMLSearchValueByTag(srXML,(char *)TAG_TRANS_HEADER,&inCntStart,&inCntEnd,0) >= SUCCESS)
	{
		srXML->srXMLElement[inCntStart].fXMLSendFlag = TRUE;
		//vdSetConfigFlag(srXML,FALSE,TAG_TRANS_HEADER);
		memcpy(srXMLData.srXMLElement[inCntStart].chTag,TAG_TRANS_HEADER,strlen(TAG_TRANS_HEADER));
	}

	//T0100
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0100,(BYTE *)"0220",TRUE);

	//T0200
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0200);

	//T0211
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0211);

	//T0213
	vdUnPackToXMLData(&DongleReqOut->ucCardType,sizeof(DongleReqOut->ucCardType),srXML,(char *)TAG_NAME_0213,strlen(TAG_NAME_0213),VALUE_NAME,fXMLSendFlag);

	//T0214
	vdUnPackToXMLData(&DongleReqOut->ucPersonalProfile,sizeof(DongleReqOut->ucPersonalProfile),srXML,(char *)TAG_NAME_0214,strlen(TAG_NAME_0214),VALUE_NAME,fXMLSendFlag);

	//T0300
	if(inTxnType == DLL_AUTOLOAD_AUTH || srTxnData.srIngData.inTransType == TXN_ECC_ADD)
	{
		if(srTxnData.srIngData.inTransType == TXN_ECC_ADD)
		{
			if(DongleReqOut->ucPersonalProfile >= 0x01 && DongleReqOut->ucPersonalProfile <= 0x03)
				inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"841799",TRUE);
			else
				inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"811799",TRUE);
		}
		else
		{
			//if(DongleReqOut->ucPersonalProfile >= 0x01 && DongleReqOut->ucPersonalProfile <= 0x03)
			//	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"845799",TRUE);
			//else
				inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"825799",TRUE);
		}
	}
	else if(inTxnType == DLL_AUTOLOAD_ENABLE_AUTH)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"814799",TRUE);
	else if(srTxnData.srIngData.inTransType == TXN_ECC_REFUND)
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"851999",TRUE);
	else if(srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"811599",TRUE);
	else if((srTxnData.srIngData.inTransType == TXN_ECC_VOID) || (srTxnData.srIngData.inTransType == TXN_ECC_VOID1))
	{
		if(DongleReqOut->ucMsgType == 0x0B)//加值
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"811899",TRUE);
		else if(DongleReqOut->ucMsgType == 0x02 && DongleReqOut->ucSubType == 0x0A)//購貨
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"823899",TRUE);
	}
	else if(srTxnData.srIngData.inTransType == TXN_ECC_ADD4)
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"871799",TRUE);
	else if(srTxnData.srIngData.inTransType == TXN_ECC_PT_ADD)
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"705699",TRUE);

	//T0400
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%ld",srTxnData.srIngData.lnECCAmt);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0400,bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0400,strlen(TAG_NAME_0400),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T0406 有折扣時，必需帶折扣金額
	if ((srTxnData.srREQData.chDiscountType == 'D') && (srTxnData.srIngData.lnDiscountAmt != 0))
	{
		memset(bBuf, 0x00, sizeof(bBuf));
		sprintf((char *)bBuf, "%lu", srTxnData.srIngData.lnDiscountAmt);
		inXMLUpdateData(srXML, (char *)TAG_NAME_4200, (char *)TAG_NAME_0406, bBuf, TRUE);
	}

	//T0408
	memset(bBuf,0x00,sizeof(bBuf));
	vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),bBuf,5,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0408,bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0408,strlen(TAG_NAME_0408),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	srTxnData.srIngData.lnECCEV = lnAmt3ByteToLong(DongleOut->ucEV);

	//T0409
	if(strlen((char *)srTxnData.srIngData.anAutoloadAmount) > 2)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy(bBuf,srTxnData.srIngData.anAutoloadAmount,strlen((char *)srTxnData.srIngData.anAutoloadAmount) - 2);
		//vdUIntToAsc(DongleReqOut->ucAutoLoadAmt,sizeof(DongleReqOut->ucAutoLoadAmt),bBuf,5,TRUE,0x00,10);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0409,bBuf,fXMLSendFlag);
		//inXMLAppendData(srXML,(char *)TAG_NAME_0409,strlen(TAG_NAME_0409),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}

	//T0410
	memset(bBuf,0x00,sizeof(bBuf));
	vdIntToAsc(DongleReqOut->ucEV,sizeof(DongleReqOut->ucEV),bBuf,5,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0410,bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0410,strlen(TAG_NAME_0410),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T0421
	//vdSetConfigFlag(srXML,FALSE,(char *)"T0421");
	if(srTxnData.srIngData.fForeignTxnFlag == TRUE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		sprintf((char *)bBuf,"%ld", srTxnData.srIngData.lnForeignAmt);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T0421",bBuf,fXMLSendFlag);

		//T0422
		vdSetConfigFlag(srXML,FALSE,(char *)"T0422");

		//T0423
		vdSetConfigFlag(srXML,FALSE,(char *)"T0423");

		//T1202
		vdSetConfigFlag(srXML,FALSE,(char *)"T1202");

		//T1302
		vdSetConfigFlag(srXML,FALSE,(char *)"T1302");
	}

	//T1100
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_1100);

	//T1101
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_1101);

	memset(bDate,0x00,sizeof(bDate));
	UnixToDateTime((BYTE*)DongleOut->ucTxnDateTime,(BYTE*)bDate,14);
	//T1200
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1200,&bDate[8],TRUE);

	//T1201
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T1201",&bDate[8],TRUE);
	bDate[8] = 0x00;
	//vdSetConfigFlag(srXML,FALSE,(char *)"T1201");

	{
		BYTE bT1300[10];

		memset(bT1300,0x00,sizeof(bT1300));
		inXMLGetData(srXML,(char *)TAG_NAME_1300,(char *)bT1300,sizeof(srTxnData.srIngData.chTxDate),0);
		log_msg(LOG_LEVEL_FLOW,"T1300 2:%s,%s",srTxnData.srIngData.chTxDate,bT1300);
	}

	//T1300
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1300,bDate,TRUE);
	//vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_1300);

	//T1301
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T1301",bDate,TRUE);
	//vdSetConfigFlag(srXML,FALSE,(char *)"T1301");

	//T3700
	vdCMASBuildRRNProcess(srXML,srTxnData.srIngData.ulCMASHostSerialNumber,DongleReqOut->ucTxnSN);

	//T4100
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4100);

	//T4101
	//if(DongleReqOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(DongleReqOut->ucDeviceID,sizeof(DongleReqOut->ucDeviceID),srXML,(char *)"T4101",strlen("T4101"),VALUE_NAME,fXMLSendFlag);

	//T4104
	memset(bBuf,0x00,sizeof(bBuf));
	if(DongleReqOut->ucPurseVersionNumber == MIFARE)
	{
		fnUnPack(&DongleOut->ucMAC_HCrypto[6],4,bBuf);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4104",bBuf,fXMLSendFlag);
	}
	else
	{
		vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4104);
		//sprintf((char *)bBuf,"00000000");
	}

	//T4107
	if(strlen(srTxnData.srParameter.chPOS_ID) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4107,(BYTE *)srTxnData.srParameter.chPOS_ID,fXMLSendFlag);

	//T4108 off
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4108);

	//T4109 off
	memset(bBuf,0x00,sizeof(bBuf));
	fnBINTODEVASC(DongleReqOut->ucDeviceID,bBuf,9,MIFARE);//n_TXN Device ID
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4109",bBuf,FALSE);

	//T4110 off
	memset(bBuf,0x00,sizeof(bBuf));
	fnBINTODEVASC(DongleReqOut->ucCPUDeviceID,bBuf,16,LEVEL2);//n_TXN Device ID
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4110",bBuf,FALSE);

	//T4117
	//if(strlen(srTxnData.srParameter.chPOS_ID2) > 0)
	//	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4117,(BYTE *)srTxnData.srParameter.chPOS_ID2,fXMLSendFlag);
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4117);

	//T4200
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4200);

	//T4210
	vdSetConfigFlag(srXML,FALSE,(char *)"T4210");

	//T4800
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4800);

	//T4801,33 byte unpack to 66 byte,後補0至90 byte
	vdBuildCMASCardAVR((BYTE *)&DongleReqOut->stLastCreditTxnLogInfo_t,sizeof(DongleReqOut->stLastCreditTxnLogInfo_t),srXML,(char *)"T4801",strlen("T4801"),VALUE_NAME,fXMLSendFlag);

	//T4802
	vdUnPackToXMLData(&DongleReqOut->ucIssuerCode,sizeof(DongleReqOut->ucIssuerCode),srXML,(char *)"T4802",strlen("T4802"),VALUE_NAME,fXMLSendFlag);

	//T4803
	vdUnPackToXMLData(DongleReqOut->BankCode,sizeof(DongleReqOut->BankCode),srXML,(char *)TAG_NAME_4803,strlen(TAG_NAME_4803),VALUE_NAME,fXMLSendFlag);

	//T4804
	vdUnPackToXMLData(&DongleReqOut->ucAreaCode,sizeof(DongleReqOut->ucAreaCode),srXML,(char *)TAG_NAME_4804,strlen(TAG_NAME_4804),VALUE_NAME,fXMLSendFlag);

	//T4805
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleReqOut->ucSubAreaCode,sizeof(DongleReqOut->ucSubAreaCode),srXML,(char *)TAG_NAME_4805,strlen(TAG_NAME_4805),VALUE_NAME,fXMLSendFlag);

	//T4808
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4808",bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T4808",strlen("T4808"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),srXML,(char *)"T4808",strlen("T4808"),VALUE_NAME,fXMLSendFlag);

	//T4809
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleReqOut->ucTxnMode,sizeof(DongleReqOut->ucTxnMode),srXML,(char *)"T4809",strlen("T4809"),VALUE_NAME,fXMLSendFlag);

	//T4810
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleReqOut->ucTxnQuqlifier,sizeof(DongleReqOut->ucTxnQuqlifier),srXML,(char *)"T4810",strlen("T4810"),VALUE_NAME,fXMLSendFlag);

	//T4811
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleReqOut->ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4811",bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T4811",strlen("T4811"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleReqOut->ucTxnSN,sizeof(DongleReqOut->ucTxnSN),srXML,(char *)"T4811",strlen("T4811"),VALUE_NAME,fXMLSendFlag);

	//T4812
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleReqOut->ucCTC,sizeof(DongleReqOut->ucCTC),srXML,(char *)"T4812",strlen("T4812"),VALUE_NAME,fXMLSendFlag);

	//T4813
	if(DongleReqOut->ucPurseVersionNumber == MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(DongleReqOut->ucCPD_SAMID,2,bBuf,6,TRUE,0x00,10);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4813",bBuf,fXMLSendFlag);
		//inXMLAppendData(srXML,(char *)"T4813",strlen("T4813"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
		//vdUnPackToXMLData(DongleReqOut->ucLoyaltyCounter,sizeof(DongleReqOut->ucLoyaltyCounter),srXML,(char *)"T4813",strlen("T4813"),VALUE_NAME,fXMLSendFlag);
	}

	//T4826
	vdSetConfigFlag(srXML,FALSE,(char *)"T4826");

	//T5301
	vdUnPackToXMLData(&DongleReqOut->ucCPDKVN_SAMKVN,sizeof(DongleReqOut->ucCPDKVN_SAMKVN),srXML,(char *)"T5301",strlen("T5301"),VALUE_NAME,fXMLSendFlag);

	//T5303//T5304//T5305
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
	{
		vdUnPackToXMLData(&DongleOut->ucMAC_HCrypto[0],1,srXML,(char *)"T5303",strlen("T5303"),VALUE_NAME,fXMLSendFlag);
		vdUnPackToXMLData(&DongleOut->ucMAC_HCrypto[1],1,srXML,(char *)"T5304",strlen("T5304"),VALUE_NAME,fXMLSendFlag);
		vdUnPackToXMLData(&DongleReqOut->ucSignatureKeyKVN,sizeof(DongleReqOut->ucSignatureKeyKVN),srXML,(char *)"T5305",strlen("T5305"),VALUE_NAME,fXMLSendFlag);
	}

	//T5361
	vdUnPackToXMLData(&DongleReqOut->ucCPD_SAMID[2],8,srXML,(char *)"T5361",strlen("T5361"),VALUE_NAME,fXMLSendFlag);

	//T5362
	vdUnPackToXMLData(&DongleReqOut->ucCPD_SAMID[10],4,srXML,(char *)"T5362",strlen("T5362"),VALUE_NAME,fXMLSendFlag);

	//T5363
	vdUnPackToXMLData(DongleReqOut->ucCPDRAN_SAMCRN,sizeof(DongleReqOut->ucCPDRAN_SAMCRN),srXML,(char *)"T5363",strlen("T5363"),VALUE_NAME,fXMLSendFlag);

	//T5371
	vdUnPackToXMLData(DongleOut->ucCPUSAMID,sizeof(DongleOut->ucCPUSAMID),srXML,(char *)"T5371",strlen("T5371"),VALUE_NAME,fXMLSendFlag);

	//T5501
	//inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)srTxnData.srIngData.chTxDate,TRUE);
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_5501);

	//T5503
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_5503);

	//T5504
	vdSetConfigFlag(srXML,FALSE,(char *)"T5504");

	//T5510
	vdSetConfigFlag(srXML,FALSE,(char *)"T5510");

	//T6000
	vdUnPackToXMLData(DongleReqOut->ucReaderFWVersion,sizeof(DongleReqOut->ucReaderFWVersion),srXML,(char *)"T6000",strlen("T6000"),VALUE_NAME,fXMLSendFlag);

	//T6001
	//vdUnPackToXMLData((BYTE *)&DongleOut->stReaderAVRInfo_t,83,srXML,(char *)"T6001",strlen("T6001"),VALUE_NAME,fXMLSendFlag);

	//T6402//T6403
	if(DongleReqOut->ucPurseVersionNumber == MIFARE)
	{
		vdUnPackToXMLData(&DongleOut->ucMAC_HCrypto[10],8,srXML,(char *)"T6402",strlen("T6402"),VALUE_NAME,fXMLSendFlag);
		vdUnPackToXMLData(&DongleOut->ucMAC_HCrypto[2],4,srXML,(char *)"T6403",strlen("T6403"),VALUE_NAME,fXMLSendFlag);
	}

	//T6404//T6405
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
	{
		vdUnPackToXMLData(&DongleOut->ucMAC_HCrypto[2],16,srXML,(char *)"T6404",strlen("T6404"),VALUE_NAME,fXMLSendFlag);
		vdUnPackToXMLData(DongleOut->ucSignature,16,srXML,(char *)"T6405",strlen("T6405"),VALUE_NAME,fXMLSendFlag);
	}

	return SUCCESS;
}

int inBuildCMSSetStudentValueTM(int inTxnType, STRUCT_XML_DOC * srXML, BYTE * API_Out, BYTE * Dongle_In, BYTE * Dongle_Out, BYTE * Dongle_ReqOut)
{
	BYTE bBuf[200],bTmp[50];
	StudentSetValue_APDU_In *DongleIn = (StudentSetValue_APDU_In *)Dongle_In;
	StudentSetValue_APDU_Out*DongleOut = (StudentSetValue_APDU_Out*)Dongle_Out;
	BOOL fXMLSendFlag = TRUE;
	int inCntStart,inCntEnd;

	//TAG_TRANS_XML_HEADER
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_TRANS_XML_HEADER);

	//TAG_TRANS_HEADER
	if(inXMLSearchValueByTag(srXML,(char *)TAG_TRANS_HEADER,&inCntStart,&inCntEnd,0) >= SUCCESS)
	{
		srXML->srXMLElement[inCntStart].fXMLSendFlag = TRUE;
		//vdSetConfigFlag(srXML,FALSE,TAG_TRANS_HEADER);
		memcpy(srXML->srXMLElement[inCntStart].chTag,TAG_TRANS_HEADER,strlen(TAG_TRANS_HEADER));
	}

	//T0100
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0100,(BYTE *)"0100",fXMLSendFlag);

	//T0200
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdUIntToAsc(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),bTmp,17,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,17);
	inXMLAppendData(srXML,(char *)TAG_NAME_0200,strlen(TAG_NAME_0200),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T0211
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucPID,sizeof(DongleOut->ucPID),srXML,(char *)TAG_NAME_0211,strlen(TAG_NAME_0211),VALUE_NAME,fXMLSendFlag);

	//T0212
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%02x",DongleOut->bAutoLoad);
	inXMLAppendData(srXML,(char *)TAG_NAME_0212,strlen(TAG_NAME_0212),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T0213
	vdUnPackToXMLData(&DongleOut->ucCardType,sizeof(DongleOut->ucCardType),srXML,(char *)TAG_NAME_0213,strlen(TAG_NAME_0213),VALUE_NAME,fXMLSendFlag);

	//T0214
	vdUnPackToXMLData(&DongleOut->ucPersonalProfile,sizeof(DongleOut->ucPersonalProfile),srXML,(char *)TAG_NAME_0214,strlen(TAG_NAME_0214),VALUE_NAME,fXMLSendFlag);

	//T0300
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"811499",TRUE);

	//T0400
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%ld",srTxnData.srIngData.lnECCAmt);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0400,bBuf,fXMLSendFlag);

	//T0410
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),bTmp,5,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,5);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0410,bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0410,strlen(TAG_NAME_0410),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1100
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulCMASHostSerialNumber);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1100,bBuf,fXMLSendFlag);

	//T1101
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulTMSerialNumber);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1101,bBuf,fXMLSendFlag);

	//T1201
	memset(bBuf,0x00,sizeof(bBuf));
	memcpy((char *)bBuf,srTxnData.srIngData.chTxTime,sizeof(srTxnData.srIngData.chTxTime) - 1);
	inXMLAppendData(srXML,(char *)"T1201",strlen("T1201"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1301
	memset(bBuf,0x00,sizeof(bBuf));
	memcpy((char *)bBuf,srTxnData.srIngData.chTxDate,sizeof(srTxnData.srIngData.chTxDate) - 1);
	inXMLAppendData(srXML,(char *)"T1301",strlen("T1301"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1401
	if ((srTxnData.srParameter.inSetCardValidDate == 1) && (srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE))
	{
		inXMLAppendData(srXML,(char *)"T1401",strlen("T1401"), (BYTE *)"1", 1,VALUE_NAME,fXMLSendFlag);
	}

	//T1402
	memset(bBuf,0x00,sizeof(bBuf));
	UnixToDateTime((BYTE*)DongleOut->ucExpiryDate,(BYTE*)bBuf,8);//Expiry Date
	inXMLAppendData(srXML,(char *)TAG_NAME_1402,strlen(TAG_NAME_1402),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T3700
	vdCMASBuildRRNProcess(srXML,srTxnData.srIngData.ulCMASHostSerialNumber,DongleOut->ucTxnSN);

	//T4100
	vdUnPackToXMLData(DongleOut->ucCPUDeviceID,sizeof(DongleOut->ucCPUDeviceID),srXML,(char *)TAG_NAME_4100,strlen(TAG_NAME_4100),VALUE_NAME,fXMLSendFlag);

	//T4101
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(DongleOut->ucDeviceID,sizeof(DongleOut->ucDeviceID),srXML,(char *)"T4101",strlen("T4101"),VALUE_NAME,fXMLSendFlag);

	//T4102
	memset(bBuf,0x00,sizeof(bBuf));
	vdGetLocalIP(bBuf);
	inXMLAppendData(srXML,(char *)"T4102",strlen("T4102"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T4104
	vdUnPackToXMLData(gTmpData.ucReaderID,sizeof(gTmpData.ucReaderID),srXML,(char *)TAG_NAME_4104,strlen(TAG_NAME_4104),VALUE_NAME,fXMLSendFlag);

	//T4104
	//memset(bBuf,0x00,sizeof(bBuf));
	//fnUnPack(DongleOut->ucReaderID,sizeof(DongleOut->ucReaderID),bBuf);
	//inXMLAppendData(srXML,(char *)"T4104",strlen("T4104"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T4104",strlen("T4104"),(BYTE *)"C9041F5A",8,VALUE_NAME,fXMLSendFlag);

	//T4107
	if(strlen(srTxnData.srParameter.chPOS_ID) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4107,(BYTE *)srTxnData.srParameter.chPOS_ID,fXMLSendFlag);

	//T4108 off
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4108);

	//T4117
	if(strlen(srTxnData.srParameter.chPOS_ID2) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4117,(BYTE *)srTxnData.srParameter.chPOS_ID2,fXMLSendFlag);

	//T4200//銀行版-->TM Location ID,一般版-->New SP ID,商店模式-->TM Location ID
	if(srTxnData.srParameter.chCMASMode == '1' || srTxnData.srParameter.chCMASMode == '2')//銀行併機 or 商店模式
	{
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy(bBuf,&DongleIn->ucTMLocationID[2],8);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,bBuf,fXMLSendFlag);
	}
	else//一般版
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(DongleOut->ucCPUSPID,sizeof(DongleOut->ucCPUSPID),bBuf,8,FALSE,'0',10);
		//vdUIntToAsc(&DongleOut->ucCPUDeviceID[3],3,bBuf,8,FALSE,'0',10);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,bBuf,fXMLSendFlag);
		//inXMLAppendData(srXML,(char *)TAG_NAME_4200,strlen(TAG_NAME_4200),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}

	//T4210
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucCPULocationID,2,bBuf,5,TRUE,0x00,10);
	inXMLAppendData(srXML,(char *)"T4210",strlen("T4210"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T4800
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucPurseVersionNumber,sizeof(DongleOut->ucPurseVersionNumber),srXML,(char *)TAG_NAME_4800,strlen(TAG_NAME_4800),VALUE_NAME,fXMLSendFlag);

	//T4801,33 byte unpack to 66 byte,後補0至90 byte
	vdBuildCMASCardAVR((BYTE *)&DongleOut->stLastCreditTxnLogInfo_t,sizeof(DongleOut->stLastCreditTxnLogInfo_t),srXML,(char *)"T4801",strlen("T4801"),VALUE_NAME,fXMLSendFlag);

	//T4802
	vdUnPackToXMLData(&DongleOut->ucIssuerCode,sizeof(DongleOut->ucIssuerCode),srXML,(char *)"T4802",strlen("T4802"),VALUE_NAME,fXMLSendFlag);

	//T4803
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
		//SetValue2_APDU_Out無此欄位
		//vdUnPackToXMLData(DongleOut->BankCode,sizeof(DongleOut->BankCode),srXML,(char *)TAG_NAME_4803,strlen(TAG_NAME_4803),VALUE_NAME,fXMLSendFlag);
	//else
	//	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4803,(BYTE *)"00",fXMLSendFlag);

	//T4804
	vdUnPackToXMLData(&DongleOut->ucAreaCode,sizeof(DongleOut->ucAreaCode),srXML,(char *)TAG_NAME_4804,strlen(TAG_NAME_4804),VALUE_NAME,fXMLSendFlag);

	//T4805
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucLoyaltyCounterOrSubAreaCode,sizeof(DongleOut->ucLoyaltyCounterOrSubAreaCode),srXML,(char *)TAG_NAME_4805,strlen(TAG_NAME_4805),VALUE_NAME,fXMLSendFlag);

	//T4806
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucProfileExpiryDate,sizeof(DongleOut->ucProfileExpiryDate),srXML,(char *)"T4806",strlen("T4806"),VALUE_NAME,fXMLSendFlag);

	//T4808
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
	inXMLAppendData(srXML,(char *)"T4808",strlen("T4808"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),srXML,(char *)"T4808",strlen("T4808"),VALUE_NAME,fXMLSendFlag);

	//T4809
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucTxnMode,sizeof(DongleOut->ucTxnMode),srXML,(char *)"T4809",strlen("T4809"),VALUE_NAME,fXMLSendFlag);

	//T4810
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucTxnQuqlifier,sizeof(DongleOut->ucTxnQuqlifier),srXML,(char *)"T4810",strlen("T4810"),VALUE_NAME,fXMLSendFlag);

	//T4811
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
	inXMLAppendData(srXML,(char *)"T4811",strlen("T4811"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),srXML,(char *)"T4811",strlen("T4811"),VALUE_NAME,fXMLSendFlag);

	//T4812
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucCTC,sizeof(DongleOut->ucCTC),srXML,(char *)"T4812",strlen("T4812"),VALUE_NAME,fXMLSendFlag);

	//T4813
	if(DongleOut->ucPurseVersionNumber == MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(&DongleOut->ucLoyaltyCounterOrSubAreaCode[0],2,bBuf,6,TRUE,0x00,10);
		vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
		inXMLAppendData(srXML,(char *)"T4813",strlen("T4813"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
		//vdUnPackToXMLData(DongleOut->ucLoyaltyCounter,sizeof(DongleOut->ucLoyaltyCounter),srXML,(char *)"T4813",strlen("T4813"),VALUE_NAME,fXMLSendFlag);
	}
	//else
	//	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4813",(BYTE *)"0000",fXMLSendFlag);

	//T4814
	if(DongleOut->ucPurseVersionNumber != MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(DongleOut->ucDeposit,3,bBuf,6,TRUE,0x00,10);
		vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
		inXMLAppendData(srXML,(char *)"T4814",strlen("T4814"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
		//vdUnPackToXMLData(DongleOut->ucDeposit,sizeof(DongleOut->ucDeposit),srXML,(char *)TAG_NAME_4814,strlen(TAG_NAME_4814),VALUE_NAME,fXMLSendFlag);
	}

	//T4826
	vdUnPackToXMLData(&DongleOut->ucCardIDLen,sizeof(DongleOut->ucCardIDLen),srXML,(char *)"T4826",strlen("T4826"),VALUE_NAME,fXMLSendFlag);

	//T5301
	vdUnPackToXMLData(&DongleOut->ucHost_SAMKVN,sizeof(DongleOut->ucHost_SAMKVN),srXML,(char *)"T5301",strlen("T5301"),VALUE_NAME,fXMLSendFlag);

	//T5302
	if(DongleOut->ucPurseVersionNumber != MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		sprintf((char *)bBuf,"%02x%02x%02x",DongleOut->ucCPUAdminKeyKVN,DongleOut->ucCreditKeyKVN,DongleOut->ucCPUIssuerKeyKVN);
		inXMLAppendData(srXML,(char *)"T5302",strlen("T5302"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}

	//T5304
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucCPUAdminKeyKVN,sizeof(DongleOut->ucCPUAdminKeyKVN),srXML,(char *)"T5304",strlen("T5304"),VALUE_NAME,fXMLSendFlag);

	//T5305
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
	//	vdUnPackToXMLData(&DongleOut->ucSignatureKeyKVN,sizeof(DongleOut->ucSignatureKeyKVN),srXML,(char *)"T5305",strlen("T5305"),VALUE_NAME,fXMLSendFlag);

	//T5361
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(&DongleOut->ucSVCrypto_SAMCRN_SAMID[8],8,srXML,(char *)"T5361",strlen("T5361"),VALUE_NAME,fXMLSendFlag);

	//T5362
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(&DongleOut->ucSAMSN[0],4,srXML,(char *)"T5362",strlen("T5362"),VALUE_NAME,fXMLSendFlag);

	//T5363
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(DongleOut->ucSVCrypto_SAMCRN_SAMID,8,srXML,(char *)"T5363",strlen("T5363"),VALUE_NAME,fXMLSendFlag);

	//T5371
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucSID_STAC,sizeof(DongleOut->ucSID_STAC),srXML,(char *)"T5371",strlen("T5371"),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucCPUSAMID,sizeof(DongleOut->ucCPUSAMID),srXML,(char *)"T5371",strlen("T5371"),VALUE_NAME,fXMLSendFlag);

	//T5503
	memset(bBuf,0x00,sizeof(bBuf));
	memcpy(bBuf,DongleIn->ucTMLocationID,sizeof(DongleIn->ucTMLocationID));
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5503",bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T5503",strlen("T5503"),DongleIn->ucTMLocationID,10,VALUE_NAME,fXMLSendFlag);

	//T5504
	inXMLAppendData(srXML,(char *)"T5504",strlen("T5504"),DongleIn->ucTMID,2,VALUE_NAME,fXMLSendFlag);

	//T5510
	inXMLAppendData(srXML,(char *)"T5510",strlen("T5510"),DongleIn->ucTMAgentNumber,4,VALUE_NAME,fXMLSendFlag);

	//T6000
	vdUnPackToXMLData(DongleOut->ucReaderFWVersion,sizeof(DongleOut->ucReaderFWVersion),srXML,(char *)"T6000",strlen("T6000"),VALUE_NAME,fXMLSendFlag);

	//T6004
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	inGetBLCName(bTmp);
	memcpy(bBuf,bTmp,5);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T6004",bBuf,fXMLSendFlag);

	//T6400
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(DongleOut->ucSID_STAC,sizeof(DongleOut->ucSID_STAC),srXML,(char *)"T6400",strlen("T6400"),VALUE_NAME,fXMLSendFlag);

	//T6406
	if(DongleOut->ucPurseVersionNumber != MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy((char *)bBuf,DongleOut->ucSVCrypto_SAMCRN_SAMID,sizeof(DongleOut->ucSVCrypto_SAMCRN_SAMID));
		vdUnPackToXMLData(bBuf,sizeof(DongleOut->ucSVCrypto_SAMCRN_SAMID),srXML,(char *)"T6406",strlen("T6406"),VALUE_NAME,fXMLSendFlag);
	}

	return SUCCESS;
}

int inBuildCMSSetValueTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
BYTE bBuf[200],bTmp[50];
SetValue2_APDU_In *DongleIn = (SetValue2_APDU_In *)Dongle_In;
SetValue2_APDU_Out *DongleOut = (SetValue2_APDU_Out *)Dongle_Out;
BOOL fXMLSendFlag = TRUE;
int inCntStart,inCntEnd;

	//if(DongleOut->ucStatusCode[0] != 0x64 || DongleOut->ucStatusCode[1] != 0x15)
	//	return SUCCESS;

	//TAG_TRANS_XML_HEADER
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_TRANS_XML_HEADER);

	//TAG_TRANS_HEADER
	if(inXMLSearchValueByTag(srXML,(char *)TAG_TRANS_HEADER,&inCntStart,&inCntEnd,0) >= SUCCESS)
	{
		srXML->srXMLElement[inCntStart].fXMLSendFlag = TRUE;
		//vdSetConfigFlag(srXML,FALSE,TAG_TRANS_HEADER);
		memcpy(srXML->srXMLElement[inCntStart].chTag,TAG_TRANS_HEADER,strlen(TAG_TRANS_HEADER));
	}

	//T0100
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0100,(BYTE *)"0100",fXMLSendFlag);

	//T0200
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdUIntToAsc(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),bTmp,17,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,17);
	inXMLAppendData(srXML,(char *)TAG_NAME_0200,strlen(TAG_NAME_0200),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T0211
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucPID,sizeof(DongleOut->ucPID),srXML,(char *)TAG_NAME_0211,strlen(TAG_NAME_0211),VALUE_NAME,fXMLSendFlag);

	//T0212
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%02x",DongleOut->bAutoLoad);
	inXMLAppendData(srXML,(char *)TAG_NAME_0212,strlen(TAG_NAME_0212),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T0213
	vdUnPackToXMLData(&DongleOut->ucCardType,sizeof(DongleOut->ucCardType),srXML,(char *)TAG_NAME_0213,strlen(TAG_NAME_0213),VALUE_NAME,fXMLSendFlag);

	//T0214
	vdUnPackToXMLData(&DongleOut->ucPersonalProfile,sizeof(DongleOut->ucPersonalProfile),srXML,(char *)TAG_NAME_0214,strlen(TAG_NAME_0214),VALUE_NAME,fXMLSendFlag);

	//T0300
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"811399",TRUE);

	//T0400
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%ld",srTxnData.srIngData.lnECCAmt);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0400,bBuf,fXMLSendFlag);

	//T0409
	/*memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdUIntToAsc(DongleOut->ucAutoLoadAmt,sizeof(DongleOut->ucAutoLoadAmt),bTmp,5,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,5);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0409,bBuf,fXMLSendFlag);*/

	//T0410
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),bTmp,5,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,5);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0410,bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0410,strlen(TAG_NAME_0410),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1100
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulCMASHostSerialNumber);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1100,bBuf,fXMLSendFlag);

	//T1101
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulTMSerialNumber);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1101,bBuf,fXMLSendFlag);

	//T1201
	memset(bBuf,0x00,sizeof(bBuf));
	memcpy((char *)bBuf,srTxnData.srIngData.chTxTime,sizeof(srTxnData.srIngData.chTxTime) - 1);
	inXMLAppendData(srXML,(char *)"T1201",strlen("T1201"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1301
	memset(bBuf,0x00,sizeof(bBuf));
	memcpy((char *)bBuf,srTxnData.srIngData.chTxDate,sizeof(srTxnData.srIngData.chTxDate) - 1);
	inXMLAppendData(srXML,(char *)"T1301",strlen("T1301"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1402
	memset(bBuf,0x00,sizeof(bBuf));
	UnixToDateTime((BYTE*)DongleOut->ucExpiryDate,(BYTE*)bBuf,8);//Expiry Date
	inXMLAppendData(srXML,(char *)TAG_NAME_1402,strlen(TAG_NAME_1402),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T3700
	vdCMASBuildRRNProcess(srXML,srTxnData.srIngData.ulCMASHostSerialNumber,DongleOut->ucTxnSN);

	//T4100
	vdUnPackToXMLData(DongleOut->ucCPUDeviceID,sizeof(DongleOut->ucCPUDeviceID),srXML,(char *)TAG_NAME_4100,strlen(TAG_NAME_4100),VALUE_NAME,fXMLSendFlag);

	//T4101
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(DongleOut->ucDeviceID,sizeof(DongleOut->ucDeviceID),srXML,(char *)"T4101",strlen("T4101"),VALUE_NAME,fXMLSendFlag);

	//T4102
	memset(bBuf,0x00,sizeof(bBuf));
	vdGetLocalIP(bBuf);
	inXMLAppendData(srXML,(char *)"T4102",strlen("T4102"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T4104
	vdUnPackToXMLData(gTmpData.ucReaderID,sizeof(gTmpData.ucReaderID),srXML,(char *)TAG_NAME_4104,strlen(TAG_NAME_4104),VALUE_NAME,fXMLSendFlag);

	//T4104
	//memset(bBuf,0x00,sizeof(bBuf));
	//fnUnPack(DongleOut->ucReaderID,sizeof(DongleOut->ucReaderID),bBuf);
	//inXMLAppendData(srXML,(char *)"T4104",strlen("T4104"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T4104",strlen("T4104"),(BYTE *)"C9041F5A",8,VALUE_NAME,fXMLSendFlag);

	//T4107
	if(strlen(srTxnData.srParameter.chPOS_ID) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4107,(BYTE *)srTxnData.srParameter.chPOS_ID,fXMLSendFlag);

	//T4108 off
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4108);

	//T4117
	if(strlen(srTxnData.srParameter.chPOS_ID2) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4117,(BYTE *)srTxnData.srParameter.chPOS_ID2,fXMLSendFlag);

	//T4200//銀行版-->TM Location ID,一般版-->New SP ID,商店模式-->TM Location ID
	if(srTxnData.srParameter.chCMASMode == '1' || srTxnData.srParameter.chCMASMode == '2')//銀行併機 or 商店模式
	{
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy(bBuf,&DongleIn->ucTMLocationID[2],8);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,bBuf,fXMLSendFlag);
	}
	else//一般版
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(DongleOut->ucCPUSPID,sizeof(DongleOut->ucCPUSPID),bBuf,8,FALSE,'0',10);
		//vdUIntToAsc(&DongleOut->ucCPUDeviceID[3],3,bBuf,8,FALSE,'0',10);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,bBuf,fXMLSendFlag);
		//inXMLAppendData(srXML,(char *)TAG_NAME_4200,strlen(TAG_NAME_4200),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}

	//T4210
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucCPULocationID,2,bBuf,5,TRUE,0x00,10);
	inXMLAppendData(srXML,(char *)"T4210",strlen("T4210"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T4800
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucPurseVersionNumber,sizeof(DongleOut->ucPurseVersionNumber),srXML,(char *)TAG_NAME_4800,strlen(TAG_NAME_4800),VALUE_NAME,fXMLSendFlag);

	//T4801,33 byte unpack to 66 byte,後補0至90 byte
	vdBuildCMASCardAVR((BYTE *)&DongleOut->stLastCreditTxnLogInfo_t,sizeof(DongleOut->stLastCreditTxnLogInfo_t),srXML,(char *)"T4801",strlen("T4801"),VALUE_NAME,fXMLSendFlag);

	//T4802
	vdUnPackToXMLData(&DongleOut->ucIssuerCode,sizeof(DongleOut->ucIssuerCode),srXML,(char *)"T4802",strlen("T4802"),VALUE_NAME,fXMLSendFlag);

	//T4803
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
		//SetValue2_APDU_Out無此欄位
		//vdUnPackToXMLData(DongleOut->BankCode,sizeof(DongleOut->BankCode),srXML,(char *)TAG_NAME_4803,strlen(TAG_NAME_4803),VALUE_NAME,fXMLSendFlag);
	//else
	//	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4803,(BYTE *)"00",fXMLSendFlag);

	//T4804
	vdUnPackToXMLData(&DongleOut->ucAreaCode,sizeof(DongleOut->ucAreaCode),srXML,(char *)TAG_NAME_4804,strlen(TAG_NAME_4804),VALUE_NAME,fXMLSendFlag);

	//T4805
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->SubAreaCode_LoyaltyCounter,sizeof(DongleOut->SubAreaCode_LoyaltyCounter),srXML,(char *)TAG_NAME_4805,strlen(TAG_NAME_4805),VALUE_NAME,fXMLSendFlag);

	//T4806
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucProfileExpiryDate,sizeof(DongleOut->ucProfileExpiryDate),srXML,(char *)"T4806",strlen("T4806"),VALUE_NAME,fXMLSendFlag);

	//T4808
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
	inXMLAppendData(srXML,(char *)"T4808",strlen("T4808"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),srXML,(char *)"T4808",strlen("T4808"),VALUE_NAME,fXMLSendFlag);

	//T4809
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucTxnMode,sizeof(DongleOut->ucTxnMode),srXML,(char *)"T4809",strlen("T4809"),VALUE_NAME,fXMLSendFlag);

	//T4810
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucTxnQuqlifier,sizeof(DongleOut->ucTxnQuqlifier),srXML,(char *)"T4810",strlen("T4810"),VALUE_NAME,fXMLSendFlag);

	//T4811
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
	inXMLAppendData(srXML,(char *)"T4811",strlen("T4811"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),srXML,(char *)"T4811",strlen("T4811"),VALUE_NAME,fXMLSendFlag);

	//T4812
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucCTC,sizeof(DongleOut->ucCTC),srXML,(char *)"T4812",strlen("T4812"),VALUE_NAME,fXMLSendFlag);

	//T4813
	if(DongleOut->ucPurseVersionNumber == MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(&DongleOut->SubAreaCode_LoyaltyCounter[0],2,bBuf,6,TRUE,0x00,10);
		vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
		inXMLAppendData(srXML,(char *)"T4813",strlen("T4813"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
		//vdUnPackToXMLData(DongleOut->ucLoyaltyCounter,sizeof(DongleOut->ucLoyaltyCounter),srXML,(char *)"T4813",strlen("T4813"),VALUE_NAME,fXMLSendFlag);
	}
	//else
	//	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4813",(BYTE *)"0000",fXMLSendFlag);

	//T4814
	if(DongleOut->ucPurseVersionNumber != MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(DongleOut->ucDeposit,3,bBuf,6,TRUE,0x00,10);
		vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
		inXMLAppendData(srXML,(char *)"T4814",strlen("T4814"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
		//vdUnPackToXMLData(DongleOut->ucDeposit,sizeof(DongleOut->ucDeposit),srXML,(char *)TAG_NAME_4814,strlen(TAG_NAME_4814),VALUE_NAME,fXMLSendFlag);
	}

	//T4826
	vdUnPackToXMLData(&DongleOut->ucCardIDLen,sizeof(DongleOut->ucCardIDLen),srXML,(char *)"T4826",strlen("T4826"),VALUE_NAME,fXMLSendFlag);

	//T5301
	vdUnPackToXMLData(DongleOut->HostAdminKeyKVN_SAMKVN,sizeof(DongleOut->HostAdminKeyKVN_SAMKVN),srXML,(char *)"T5301",strlen("T5301"),VALUE_NAME,fXMLSendFlag);

	//T5302
	if(DongleOut->ucPurseVersionNumber != MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		sprintf((char *)bBuf,"%02x%02x%02x",DongleOut->ucCPUAdminKeyKVN,DongleOut->ucCreditKeyKVN,DongleOut->ucCPUIssuerKeyKVN);
		inXMLAppendData(srXML,(char *)"T5302",strlen("T5302"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}

	//T5304
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->HostAdminKeyKVN_SAMKVN,sizeof(DongleOut->HostAdminKeyKVN_SAMKVN),srXML,(char *)"T5304",strlen("T5304"),VALUE_NAME,fXMLSendFlag);

	//T5305
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
	//	vdUnPackToXMLData(&DongleOut->ucSignatureKeyKVN,sizeof(DongleOut->ucSignatureKeyKVN),srXML,(char *)"T5305",strlen("T5305"),VALUE_NAME,fXMLSendFlag);

	//T5361
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(&DongleOut->SVCrypto_SAMCRN_SAMID[8],8,srXML,(char *)"T5361",strlen("T5361"),VALUE_NAME,fXMLSendFlag);

	//T5362
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(&DongleOut->SAMSN[0],4,srXML,(char *)"T5362",strlen("T5362"),VALUE_NAME,fXMLSendFlag);

	//T5363
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(DongleOut->SVCrypto_SAMCRN_SAMID,8,srXML,(char *)"T5363",strlen("T5363"),VALUE_NAME,fXMLSendFlag);

	//T5371
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->SAMID_STAC,sizeof(DongleOut->SAMID_STAC),srXML,(char *)"T5371",strlen("T5371"),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucCPUSAMID,sizeof(DongleOut->ucCPUSAMID),srXML,(char *)"T5371",strlen("T5371"),VALUE_NAME,fXMLSendFlag);

	//T5503
	memset(bBuf,0x00,sizeof(bBuf));
	memcpy(bBuf,DongleIn->ucTMLocationID,sizeof(DongleIn->ucTMLocationID));
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5503",bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T5503",strlen("T5503"),DongleIn->ucTMLocationID,10,VALUE_NAME,fXMLSendFlag);

	//T5504
	inXMLAppendData(srXML,(char *)"T5504",strlen("T5504"),DongleIn->ucTMID,2,VALUE_NAME,fXMLSendFlag);

	//T5510
	inXMLAppendData(srXML,(char *)"T5510",strlen("T5510"),DongleIn->ucTMAgentNumber,4,VALUE_NAME,fXMLSendFlag);

	//T6000
	vdUnPackToXMLData(DongleOut->ucReaderFWVersion,sizeof(DongleOut->ucReaderFWVersion),srXML,(char *)"T6000",strlen("T6000"),VALUE_NAME,fXMLSendFlag);

	//T6004
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	inGetBLCName(bTmp);
	memcpy(bBuf,bTmp,5);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T6004",bBuf,fXMLSendFlag);

	//T6400
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(DongleOut->SAMID_STAC,sizeof(DongleOut->SAMID_STAC),srXML,(char *)"T6400",strlen("T6400"),VALUE_NAME,fXMLSendFlag);

	//T6406
	if(DongleOut->ucPurseVersionNumber != MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy((char *)bBuf,DongleOut->SVCrypto_SAMCRN_SAMID,sizeof(DongleOut->SVCrypto_SAMCRN_SAMID));
		vdUnPackToXMLData(bBuf,sizeof(DongleOut->SVCrypto_SAMCRN_SAMID),srXML,(char *)"T6406",strlen("T6406"),VALUE_NAME,fXMLSendFlag);
	}

	return SUCCESS;
}

int inBuildICERTxnReqOnlineTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
BYTE bBuf[200],bTmp[200],bT6410Data[200];
TxnReqOnline_APDU_In *DongleIn = (TxnReqOnline_APDU_In *)Dongle_In;
TxnReqOnline_APDU_Out *DongleOut = (TxnReqOnline_APDU_Out *)Dongle_Out;
BOOL fXMLSendFlag = TRUE;

	//if(DongleOut->ucStatusCode[0] != 0x64 || DongleOut->ucStatusCode[1] != 0x15)
	//	return SUCCESS;

	//TAG_TRANS_XML_HEADER
	vdSetConfigFlag(srXML,2,(char *)TAG_TRANS_XML_HEADER);

	//TAG_TRANS_HEADER
/*	if(inXMLSearchValueByTag(srXML,(char *)TAG_TRANS_HEADER,&inCntStart,&inCntEnd,0) >= SUCCESS)
	{
		srXML->srXMLElement[inCntStart].fXMLSendFlag = TRUE;
		//vdSetConfigFlag(srXML,FALSE,TAG_TRANS_HEADER);
		memcpy(srXML->srXMLElement[inCntStart].chTag,TAG_TRANS_HEADER,strlen(TAG_TRANS_HEADER));
	}
*/

	memset(bT6410Data,0x00,sizeof(bT6410Data));
	//T0100
	if ((srTxnData.srIngData.inTransType == TXN_ECC_READ_BASIC) || (srTxnData.srIngData.inTransType == TXN_QUERY_POINT))
	//if(srTxnData.srIngData.inTransType == TXN_QUERY_POINT)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0100,(BYTE *)"0100",fXMLSendFlag);
	else if(srTxnData.srIngData.inTransType >= TXN_E_ADD && srTxnData.srIngData.inTransType <= TXN_E_QR_VOID_ADD)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0100,(BYTE *)"0200",fXMLSendFlag);

	//T0200
	if(srTxnData.srIngData.inTransType == TXN_QUERY_POINT || srTxnData.srIngData.inTransType == TXN_GIFT_DEDUCT || srTxnData.srIngData.inTransType == TXN_ADJECT_CARD)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		memset(bTmp,0x00,sizeof(bTmp));
		vdUIntToAsc(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),bTmp,17,TRUE,0x00,10);
		vdTrimDataRight((char *)bBuf,(char *)bTmp,17);
		inXMLAppendData(srXML,(char *)TAG_NAME_0200,strlen(TAG_NAME_0200),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}

	//T0211
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucPID,sizeof(DongleOut->ucPID),srXML,(char *)TAG_NAME_0211,strlen(TAG_NAME_0211),VALUE_NAME,fXMLSendFlag);

	//T0212
	memset(bBuf,0x00,sizeof(bBuf));
	bBuf[0] = '0' + DongleOut->bAutoLoad;
	inXMLAppendData(srXML,(char *)TAG_NAME_0212,strlen(TAG_NAME_0212),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	/*if(srTxnData.srIngData.inTransType == TXN_QUERY_POINT && srTxnData.srParameter.chICERQRTxn == '1')
	{
		//T0231
		memset(bBuf,0x00,sizeof(bBuf));
		strcat((char *)bBuf,"1299123456@easycard.com.tw");
		inXMLAppendData(srXML,(char *)TAG_NAME_0231,strlen(TAG_NAME_0231),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],bBuf,strlen((char *)bBuf));

		//T0232
		memset(bBuf,0x00,sizeof(bBuf));
		strcat((char *)bBuf,"7110EDA4D09E062AA5E4A390B0A572AC0D2C0220");
		inXMLAppendData(srXML,(char *)TAG_NAME_0232,strlen(TAG_NAME_0232),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}*/

	//T0300
	if(srTxnData.srIngData.inTransType == TXN_ECC_READ_BASIC)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"200163",TRUE);
	/*if(srTxnData.srParameter.chICERQRTxn == '1')
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],srTxnData.srIngData.chTMProcessCode,strlen((char *)srTxnData.srIngData.chTMProcessCode));*/
	else if(srTxnData.srIngData.inTransType == TXN_QUERY_POINT && srTxnData.srREQData.chDiscountType == 'D')
	{
		//T0300
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"200000",TRUE);

		memset(bBuf,0x00,sizeof(bBuf));
		if(inXMLGetData(&srXMLData,(char *)TAG_NAME_0403,(char *)bBuf,sizeof(bBuf),0) < SUCCESS)//沒有T0403
		{
			memset(bBuf,0x00,sizeof(bBuf));
			sprintf((char *)bBuf,"%ld00",srTxnData.srIngData.lnECCAmt);
			inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0403,bBuf,TRUE);
		}
	}

	//T0409
	if(srTxnData.srIngData.inTransType == TXN_ADJECT_CARD)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		memset(bTmp,0x00,sizeof(bTmp));
		vdUIntToAsc(DongleOut->ucAutoLoadAmt,sizeof(DongleOut->ucAutoLoadAmt),bTmp,5,TRUE,0x00,10);
		vdTrimDataRight((char *)bBuf,(char *)bTmp,5);
		strcat((char *)bBuf,"00");
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0409,bBuf,fXMLSendFlag);
	}

	//T0410
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),bTmp,5,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,5);
	strcat((char *)bBuf,"00");
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0410,bBuf,fXMLSendFlag);

	if(srTxnData.srIngData.inTransType == TXN_E_DEDUCT || srTxnData.srIngData.inTransType == TXN_E_QR_DEDUCT)//扣款
	{	//T0433
		memset(bBuf,0x00,sizeof(bBuf));
		sprintf((char *)bBuf,"%ld00",srTxnData.srIngData.lnECCAmt);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T0433",bBuf,fXMLSendFlag);
	}
	else if(srTxnData.srIngData.inTransType == TXN_E_ADD || srTxnData.srIngData.inTransType == TXN_E_QR_ADD || srTxnData.srIngData.inTransType == TXN_E_REFUND)//加值
	{	//T0437
		memset(bBuf,0x00,sizeof(bBuf));
		sprintf((char *)bBuf,"%ld00",srTxnData.srIngData.lnECCAmt);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T0437",bBuf,fXMLSendFlag);
	}

	//T1100
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulTMSerialNumber);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1100,bBuf,fXMLSendFlag);
	/*if(srTxnData.srParameter.chICERQRTxn == '1')
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],bBuf,strlen((char *)bBuf));*/

	//T1200
	/*if(srTxnData.srParameter.chICERQRTxn == '1')
	{
		memcpy((char *)bBuf,srTxnData.srIngData.chTxTime,sizeof(srTxnData.srIngData.chTxTime) - 1);
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],bBuf,strlen((char *)bBuf));
	}*/

	//T1300
	/*if(srTxnData.srParameter.chICERQRTxn == '1')
	{
		memcpy((char *)bBuf,srTxnData.srIngData.chTxDate,sizeof(srTxnData.srIngData.chTxDate) - 1);
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],bBuf,strlen((char *)bBuf));
	}*/

	//T1400
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1400,(BYTE *)"9912",fXMLSendFlag);

	//T1402
	if(srTxnData.srIngData.inTransType == TXN_ADJECT_CARD)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		UnixToDateTime((BYTE*)DongleOut->ucExpiryDate,(BYTE*)bBuf,8);//Expiry Date
		inXMLAppendData(srXML,(char *)TAG_NAME_1402,strlen(TAG_NAME_1402),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}

	//T3700
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_3700,srTxnData.srIngData.anRRN,fXMLSendFlag);
	/*if(srTxnData.srParameter.chICERQRTxn == '1')
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],srTxnData.srIngData.anRRN,strlen((char *)srTxnData.srIngData.anRRN));*/

	//T4100
	vdStoreDeviceID(DongleOut->ucDeviceID,DongleOut->ucSPID,DongleOut->ucLocationID,DongleOut->ucCPUDeviceID,DongleOut->ucCPUSPID,DongleOut->ucCPULocationID,gTmpData.ucReaderID,DongleOut->ucReaderFWVersion,DongleOut->ucCPUSAMID);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4100,srTxnData.srIngData.anDeviceID,fXMLSendFlag);
	/*if(srTxnData.srParameter.chICERQRTxn == '1')
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],srTxnData.srIngData.anDeviceID,strlen((char *)srTxnData.srIngData.anDeviceID));*/

	//T4108 off
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4108);

	//T4110
	memset(bBuf,0x00,sizeof(bBuf));
	fnBINTODEVASC(DongleOut->ucCPUDeviceID,bBuf,16,LEVEL2);//n_TXN Device ID
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4110,bBuf,fXMLSendFlag);

	if(memcmp(srTxnData.srIngData.chTMProcessCode,"270100",6) &&//指定加值查詢
	   memcmp(srTxnData.srIngData.chTMProcessCode,"500060",6))//指定加值
	{
		//T4200
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy((char *)bBuf,srTxnData.srParameter.bMerchantID,strlen((char *)srTxnData.srParameter.bMerchantID));
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,bBuf,fXMLSendFlag);
		/*if(srTxnData.srParameter.chICERQRTxn == '1')
			memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],bBuf,strlen((char *)bBuf));*/
	}

	//T5371
	vdUnPackToXMLData(DongleOut->ucCPUSAMID,sizeof(DongleOut->ucCPUSAMID),srXML,(char *)"T5371",strlen("T5371"),VALUE_NAME,fXMLSendFlag);

	//T5503
	memset(bBuf,0x00,sizeof(bBuf));
	memcpy((char *)bBuf,DongleIn->ucTMLocationID,sizeof(DongleIn->ucTMLocationID));
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_5503,bBuf,fXMLSendFlag);
	/*if(srTxnData.srParameter.chICERQRTxn == '1')
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],bBuf,strlen((char *)bBuf));*/

	//T5509//上線一定是E,目前I僅供測試!!
	/*if(srTxnData.srParameter.chICERQRTxn == '1')
		inXMLAppendData(srXML,(char *)"T5509",strlen("T5509"),(BYTE *)"X",1,VALUE_NAME,fXMLSendFlag);
	else*/
		inXMLAppendData(srXML,(char *)"T5509",strlen("T5509"),(BYTE *)"E",1,VALUE_NAME,fXMLSendFlag);

	//T5510
	inXMLAppendData(srXML,(char *)"T5510",strlen("T5510"),DongleIn->ucTMAgentNumber,4,VALUE_NAME,fXMLSendFlag);

	//T5515//暫時寫死,到時拿掉!!
	/*if(srTxnData.srParameter.chICERQRTxn == '1')
		inXMLAppendData(srXML,(char *)"T5515",strlen("T5515"),(BYTE *)"Y",1,VALUE_NAME,fXMLSendFlag);*/

	//T6000
	vdUnPackToXMLData(DongleOut->ucReaderFWVersion,sizeof(DongleOut->ucReaderFWVersion),srXML,(char *)"T6000",strlen("T6000"),VALUE_NAME,fXMLSendFlag);

	//T554801 + T554808 + T6400 + T6406
	//if(srTxnData.srParameter.chICERQRTxn != '1')
		vdBuildICERAData(inTxnType,srXML,Dongle_In,Dongle_Out);

	//T6410
	/*if(srTxnData.srParameter.chICERQRTxn == '1')
	{
		memset(bBuf,0x00,sizeof(bBuf));
		memset(bTmp,0x00,sizeof(bTmp));
		sprintf((char *)bBuf,"0000000000000000");

		PKCS5Padding(bT6410Data,strlen((char *)bT6410Data),16);
		log_msg_debug(LOG_LEVEL_FLOW,FALSE,(char *)"bICERKey :",10,srTxnData.srParameter.bICERKey,strlen((char *)srTxnData.srParameter.bICERKey));
		log_msg_debug(LOG_LEVEL_FLOW,FALSE,(char *)"bT6410Data :",12,bT6410Data,strlen((char *)bT6410Data));
		ICER_AES_CBC_Func(0, srTxnData.srParameter.bICERKey, 2, bBuf, bT6410Data, strlen((char *)bT6410Data), bTmp);
		memset(bBuf,0x00,sizeof(bBuf));
		fnUnPack(&bTmp[strlen((char *)bTmp) - 8],8,bBuf);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T6410",bBuf,fXMLSendFlag);


		//TC101
		memset(bBuf,0x00,sizeof(bBuf));
		strcat((char *)bBuf,"EASYWALLET");
		inXMLAppendData(srXML,(char *)"TC101",strlen("TC101"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}*/

	//T6412
	/*if(srTxnData.srParameter.chICERQRTxn == '1')
	{
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy((char *)bBuf,"39DFA55283318D31AFE5A3FF4A0E3253E2045E43",40);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T6412",bBuf,fXMLSendFlag);
	}*/

	return SUCCESS;
}

void vdBuildICERAData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *Dongle_In,BYTE *Dongle_Out)
{
int	inRetVal,inCnt = 0,inSize = 0;
BYTE ucBuf[10],anAData[300],ucAData[200],ucMACData[50];

	memset(ucAData,0x00,sizeof(ucAData));
	memset(ucMACData,0x00,sizeof(ucMACData));
	memset(anAData,0x00,sizeof(anAData));
	//if(inTxnType == DLL_ADD_VALUE)
	{
		TxnReqOnline_APDU_In *DongleIn = (TxnReqOnline_APDU_In *)Dongle_In;
		TxnReqOnline_APDU_Out *DongleOut = (TxnReqOnline_APDU_Out *)Dongle_Out;

		srTxnData.srIngData.ucPurseVersionNumber = DongleOut->ucPurseVersionNumber;
		if(DongleOut->ucPurseVersionNumber == MIFARE)
		{
			ucAData[inCnt++] = DongleOut->ucMsgType;

			ucAData[inCnt++] = DongleOut->ucSubType;

			inSize = sizeof(DongleOut->ucDeviceID);
			memcpy(&ucAData[inCnt],DongleOut->ucDeviceID,inSize);
			inCnt += inSize;

			inSize = sizeof(DongleIn->ucTxnDateTime);
			memcpy(&ucAData[inCnt],DongleIn->ucTxnDateTime,inSize);
			inCnt += inSize;

			inSize = 4;
			memcpy(&ucAData[inCnt],DongleOut->ucCardID,inSize);
			inCnt += inSize;

			inSize = 2;
			memcpy(&ucAData[inCnt],DongleOut->ucTxnAmt,inSize);
			inCnt += inSize;

			inSize = 6;
			memcpy(&ucAData[inCnt],&DongleIn->ucTMLocationID[4],inSize);
			inCnt += inSize;

			inSize = sizeof(DongleIn->ucTMID);
			memcpy(&ucAData[inCnt],DongleIn->ucTMID,inSize);
			inCnt += inSize;

			inSize = 2;
			memcpy(&ucAData[inCnt],DongleOut->ucEV,inSize);
			inCnt += inSize;

			ucAData[inCnt++] = DongleOut->ucSAMKVN;

			inSize = sizeof(DongleOut->ucSAMID);
			memcpy(&ucAData[inCnt],DongleOut->ucSAMID,inSize);
			inCnt += inSize;

			inSize = sizeof(DongleOut->ucSAMSN);
			memcpy(&ucAData[inCnt],DongleOut->ucSAMSN,inSize);
			inCnt += inSize;

			inSize = sizeof(DongleOut->ucSAMCRN);
			memcpy(&ucAData[inCnt],DongleOut->ucSAMCRN,inSize);
			inCnt += inSize;

			fnUnPack(ucAData,inCnt,anAData);
			fnUnPack(DongleOut->ucSTAC,sizeof(DongleOut->ucSTAC),ucMACData);
		}
		else
		{
			ucAData[inCnt++] = DongleOut->ucCreditKeyKVN;

			inSize = sizeof(DongleOut->ucPID);
			memcpy(&ucAData[inCnt],DongleOut->ucPID,inSize);
			inCnt += inSize;

			inSize = sizeof(DongleOut->ucCTC);
			memcpy(&ucAData[inCnt],DongleOut->ucCTC,inSize);
			inCnt += inSize;

			ucAData[inCnt++] = 0x36;

			inSize = sizeof(DongleOut->ucTxnAmt);
			memcpy(&ucAData[inCnt],DongleOut->ucTxnAmt,inSize);
			inCnt += inSize;

			inSize = sizeof(DongleIn->ucTxnDateTime);
			memcpy(&ucAData[inCnt],DongleIn->ucTxnDateTime,inSize);
			inCnt += inSize;

			ucAData[inCnt++] = DongleOut->ucTxnQuqlifier;

			inSize = sizeof(DongleOut->ucCPUDeviceID);
			memcpy(&ucAData[inCnt],DongleOut->ucCPUDeviceID,inSize);
			inCnt += inSize;

			ucAData[inCnt++] = DongleOut->ucTxnMode;

			inSize = sizeof(DongleIn->ucTMLocationID);
			memcpy(&ucAData[inCnt],DongleIn->ucTMLocationID,inSize);
			inCnt += inSize;

			inSize = sizeof(DongleOut->ucPID);
			memcpy(&ucAData[inCnt],DongleOut->ucPID,inSize);
			inCnt += inSize;

			inSize = sizeof(DongleOut->ucCardID);
			memcpy(&ucAData[inCnt],DongleOut->ucCardID,inSize);
			inCnt += inSize;

			inSize = sizeof(DongleOut->ucEV);
			memcpy(&ucAData[inCnt],DongleOut->ucEV,inSize);
			inCnt += inSize;

			inSize = sizeof(DongleOut->ucTxnSN);
			memcpy(&ucAData[inCnt],DongleOut->ucTxnSN,inSize);
			inCnt += inSize;

			ucAData[inCnt++] = DongleOut->ucHostAdminKVN;

			inSize = sizeof(DongleOut->ucCPUSAMID);
			memcpy(&ucAData[inCnt],DongleOut->ucCPUSAMID,inSize);
			inCnt += inSize;

			fnUnPack(ucAData,inCnt,anAData);
			fnUnPack(DongleOut->ucTxnCrypto,sizeof(DongleOut->ucTxnCrypto),ucMACData);
		}
	}

	memset(ucBuf,0x00,sizeof(ucBuf));
	inRetVal = inXMLGetData(srXML,(char *)TAG_NAME_5548,(char *)ucBuf,0,0);
	if(inRetVal < SUCCESS)//沒有5548
	{
		inXMLAppendData(srXML,(char *)TAG_NAME_5548,strlen(TAG_NAME_5548),ucBuf,0,TAG_NAME_START,TRUE);
		if(srTxnData.srIngData.ucPurseVersionNumber == MIFARE)
			inXMLAppendData(srXML,(char *)TAG_NAME_554801,strlen(TAG_NAME_554801),anAData,strlen((char *)anAData),VALUE_NAME,TRUE);
		else //if(srTxnData.srIngData.ucPurseVersionNumber == LEVEL2)
			inXMLAppendData(srXML,(char *)TAG_NAME_554808,strlen(TAG_NAME_554808),anAData,strlen((char *)anAData),VALUE_NAME,TRUE);
		inXMLAppendData(srXML,(char *)TAG_NAME_5548_END,strlen(TAG_NAME_5548_END),ucBuf,0,TAG_NAME_END,TRUE);
	}
	else
	{
		//inXMLInsertData(srXML,&srXMLActionData,(char *)"/TRANS",FALSE);
		if(srTxnData.srIngData.ucPurseVersionNumber == MIFARE)
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554801,(char *)anAData,TRUE,NODE_NO_ADD,VALUE_NAME,TRUE);
		else //if(srTxnData.srIngData.ucPurseVersionNumber == LEVEL2)
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554808,(char *)anAData,TRUE,NODE_NO_ADD,VALUE_NAME,TRUE);
		inXMLInsertData(srXML,&srXMLActionData,(char *)"/T5548",TRUE);
	}

	if(srTxnData.srIngData.ucPurseVersionNumber == MIFARE)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_6400,ucMACData,TRUE);
	else //if(srTxnData.srIngData.ucPurseVersionNumber == LEVEL2)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_6406,ucMACData,TRUE);
}

void vdBuildICERADataAuth(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *Dongle_In,BYTE *DongleReq_Out,BYTE *Dongle_Out)
{
int	inRetVal,inCnt = 0,inLen = 0;
BYTE ucBuf[10],anAData[300],ucMACData[50],anC_TAC[8 * 2 + 1],anC_TAC2[16 * 2 + 1];
//TxnReqOnline_APDU_In *DongleIn = (TxnReqOnline_APDU_In *)Dongle_In;
TxnReqOnline_APDU_Out *DongleReqOut = (TxnReqOnline_APDU_Out *)DongleReq_Out;
AuthTxnOnline_APDU_Out *DongleOut = (AuthTxnOnline_APDU_Out *)Dongle_Out;

	memset(ucMACData,0x00,sizeof(ucMACData));
	memset(anAData,0x00,sizeof(anAData));
	if(DongleReqOut->ucPurseVersionNumber == MIFARE)
	{
		inLen = sizeof(DongleOut->ucTxnDateTime);
		fnUnPack(DongleOut->ucTxnDateTime,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = 4;
		fnUnPack(DongleReqOut->ucCardID,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = 2;
		fnUnPack(DongleOut->ucTxnSN,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = 2;
		fnUnPack(DongleOut->ucEV,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = 6;
		fnUnPack(DongleOut->ucMAC_HCrypto,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;
		inLen = 4;
		if(DongleReqOut->ucReaderFWVersion[5] >= 0x80 || srTxnData.srParameter.chGroupFlag == '1')
			fnUnPack(DongleReqOut->ucDeviceID,inLen,&anAData[inCnt]);
		else
			fnUnPack(&DongleOut->ucMAC_HCrypto[6],inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = 2;
		sprintf((char *)&anAData[inCnt],"01");
		inCnt += inLen;

		inLen = sizeof(DongleReqOut->ucSAMID);
		fnUnPack(DongleReqOut->ucSAMID,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(DongleOut->ucStatusCode);
		fnUnPack(DongleOut->ucStatusCode,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		//新增MAC欄位資料
		inLen = sizeof(DongleReqOut->ucMsgType);
		fnUnPack(&DongleReqOut->ucMsgType,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(DongleReqOut->ucSubType);
		fnUnPack(&DongleReqOut->ucSubType,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = 3;
		fnUnPack(DongleReqOut->ucDeviceID,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = 1;
		fnUnPack(&DongleReqOut->ucDeviceID[2],inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(DongleReqOut->ucIssuerCode);
		fnUnPack(&DongleReqOut->ucIssuerCode,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = 2;
		fnUnPack(DongleReqOut->ucTxnAmt,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(DongleReqOut->ucLocationID);
		fnUnPack(&DongleReqOut->ucLocationID,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = 1;
		fnUnPack(&DongleReqOut->ucDeviceID[2],inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(DongleReqOut->ucBankCode);
		fnUnPack(&DongleReqOut->ucBankCode,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(DongleReqOut->ucLoyaltyCounter);
		fnUnPack(DongleReqOut->ucLoyaltyCounter,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

	}
	else //if(chPurseVersion == LEVEL1 || chPurseVersion == LEVEL2)
	{
		inLen = 1;
		fnUnPack(&DongleOut->ucMAC_HCrypto[1],inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(DongleOut->ucCPUSAMID);
		fnUnPack(DongleOut->ucCPUSAMID,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = 1;
		fnUnPack(&DongleOut->ucMAC_HCrypto[0],inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(DongleReqOut->ucMsgType);
		fnUnPack(&DongleReqOut->ucMsgType,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(DongleReqOut->ucSubType);
		fnUnPack(&DongleReqOut->ucSubType,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(DongleOut->ucTxnDateTime);
		fnUnPack(DongleOut->ucTxnDateTime,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(DongleReqOut->ucCardID);
		fnUnPack(DongleReqOut->ucCardID,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(DongleReqOut->ucIssuerCode);
		fnUnPack(&DongleReqOut->ucIssuerCode,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(DongleOut->ucTxnSN);
		fnUnPack(DongleOut->ucTxnSN,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(DongleReqOut->ucTxnAmt);
		fnUnPack(DongleReqOut->ucTxnAmt,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(DongleOut->ucEV);
		fnUnPack(DongleOut->ucEV,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(DongleReqOut->ucAreaCode);
		fnUnPack(&DongleReqOut->ucAreaCode,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(DongleReqOut->ucCPUDeviceID);
		fnUnPack(DongleReqOut->ucCPUDeviceID,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(DongleReqOut->ucCPUSPID);
		fnUnPack(DongleReqOut->ucCPUSPID,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(DongleReqOut->ucCPULocationID);
		fnUnPack(DongleReqOut->ucCPULocationID,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(DongleReqOut->ucPersonalProfile);
		fnUnPack(&DongleReqOut->ucPersonalProfile,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(DongleReqOut->ucPID);
		fnUnPack(DongleReqOut->ucPID,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(DongleReqOut->ucPurseVersionNumber);
		fnUnPack(&DongleReqOut->ucPurseVersionNumber,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(DongleReqOut->ucSubAreaCode);
		fnUnPack(DongleReqOut->ucSubAreaCode,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		//新增MAC欄位資料
		inLen = sizeof(DongleReqOut->ucCTC);
		fnUnPack(DongleReqOut->ucCTC,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(DongleReqOut->ucTxnQuqlifier);
		fnUnPack(&DongleReqOut->ucTxnQuqlifier,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(DongleReqOut->ucSignatureKeyKVN);
		fnUnPack(&DongleReqOut->ucSignatureKeyKVN,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(DongleOut->ucSignature);
		fnUnPack(DongleOut->ucSignature,inLen,&anAData[inCnt]);
		inCnt += inLen * 2;
	}

	memset(ucBuf,0x00,sizeof(ucBuf));
	inRetVal = inXMLGetData(srXML,(char *)TAG_NAME_5548,(char *)ucBuf,0,0);
	if(inRetVal < SUCCESS)//沒有5548
	{
		inXMLAppendData(srXML,(char *)TAG_NAME_5548,strlen(TAG_NAME_5548),ucBuf,0,TAG_NAME_START,TRUE);
		if(srTxnData.srIngData.ucPurseVersionNumber == MIFARE)
		{
			inXMLAppendData(srXML,(char *)TAG_NAME_554801,strlen(TAG_NAME_554801),anAData,strlen((char *)anAData),VALUE_NAME,TRUE);
			memset(anC_TAC,0x00,sizeof(anC_TAC));
			fnUnPack(&DongleOut->ucMAC_HCrypto[10],8,anC_TAC);
			inXMLAppendData(srXML,(char *)TAG_NAME_554807,strlen(TAG_NAME_554807),anC_TAC,strlen((char *)anC_TAC),VALUE_NAME,TRUE);
		}
		else //if(srTxnData.srIngData.ucPurseVersionNumber == LEVEL2)
			inXMLAppendData(srXML,(char *)TAG_NAME_554808,strlen(TAG_NAME_554808),anAData,strlen((char *)anAData),VALUE_NAME,TRUE);
		inXMLAppendData(srXML,(char *)TAG_NAME_5548_END,strlen(TAG_NAME_5548_END),ucBuf,0,TAG_NAME_END,TRUE);
	}
	else
	{
		//inXMLInsertData(srXML,&srXMLActionData,(char *)"/TRANS",FALSE);
		if(srTxnData.srIngData.ucPurseVersionNumber == MIFARE)
		{
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554801,(char *)anAData,TRUE,NODE_NO_ADD,VALUE_NAME,TRUE);
			memset(anC_TAC,0x00,sizeof(anC_TAC));
			fnUnPack(&DongleOut->ucMAC_HCrypto[10],8,anC_TAC);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554807,(char *)anC_TAC,FALSE,NODE_NO_SAME,VALUE_NAME,TRUE);
		}
		else //if(srTxnData.srIngData.ucPurseVersionNumber == LEVEL2)
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554808,(char *)anAData,TRUE,NODE_NO_ADD,VALUE_NAME,TRUE);
		inXMLInsertData(srXML,&srXMLActionData,(char *)"/T5548",TRUE);
	}

	if(srTxnData.srIngData.ucPurseVersionNumber == MIFARE)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_6400,ucMACData,TRUE);
	else //if(srTxnData.srIngData.ucPurseVersionNumber == LEVEL2)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_6406,ucMACData,TRUE);
	if(srTxnData.srIngData.ucPurseVersionNumber != MIFARE)//指定加值
	{
		memset(anC_TAC2,0x00,sizeof(anC_TAC2));
		fnUnPack(&DongleOut->ucMAC_HCrypto[2],16,anC_TAC2);
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_6404,anC_TAC2,TRUE);
	}
}

int inBuildICERQRTxnTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
BYTE bBuf[200],bTmp[200],bT6410Data[200];
//TxnReqOnline_APDU_In *DongleIn = (TxnReqOnline_APDU_In *)Dongle_In;
//TxnReqOnline_APDU_Out *DongleOut = (TxnReqOnline_APDU_Out *)Dongle_Out;
BOOL fXMLSendFlag = TRUE;

	//if(DongleOut->ucStatusCode[0] != 0x64 || DongleOut->ucStatusCode[1] != 0x15)
	//	return SUCCESS;

/*	//TAG_TRANS_XML_HEADER
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_TRANS_XML_HEADER);

	//TAG_TRANS_HEADER
	if(inXMLSearchValueByTag(srXML,(char *)TAG_TRANS_HEADER,&inCntStart,&inCntEnd,0) >= SUCCESS)
	{
		srXML->srXMLElement[inCntStart].fXMLSendFlag = TRUE;
		//vdSetConfigFlag(srXML,FALSE,TAG_TRANS_HEADER);
		memcpy(srXML->srXMLElement[inCntStart].chTag,TAG_TRANS_HEADER,strlen(TAG_TRANS_HEADER));
	}
*/

	memset(bT6410Data,0x00,sizeof(bT6410Data));
	//T0100
	/*if(srTxnData.srIngData.inTransType == TXN_QUERY_POINT)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0100,(BYTE *)"0100",fXMLSendFlag);
	else
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0100,(BYTE *)"0200",fXMLSendFlag);*/

	if(srTxnData.srIngData.inTransType != TXN_E_READ_QR_CODE)
	{
		int inRetVal,inCntStart = 0,inCntEnd = 0;
		//T0206
		memset(bBuf,0x00,sizeof(bBuf));
		inRetVal = inXMLSearchValueByTag(srXML,(char *)TAG_NAME_0206,&inCntStart,&inCntEnd,0);
		if(inRetVal >= SUCCESS)
			memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],srXML->srXMLElement[inCntStart].chValue,strlen((char *)srXML->srXMLElement[inCntStart].chValue));
	}
	else //if(srTxnData.srIngData.inTransType == TXN_E_READ_QR_CODE)
	{
		//T0231
		memset(bBuf,0x00,sizeof(bBuf));
		strcat((char *)bBuf,"1299123456@easycard.com.tw");
		inXMLAppendData(srXML,(char *)TAG_NAME_0231,strlen(TAG_NAME_0231),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],bBuf,strlen((char *)bBuf));

		//T0232
		memset(bBuf,0x00,sizeof(bBuf));
		strcat((char *)bBuf,"7110EDA4D09E062AA5E4A390B0A572AC0D2C0220");
		inXMLAppendData(srXML,(char *)TAG_NAME_0232,strlen(TAG_NAME_0232),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}

	//T0300
	if(srTxnData.srIngData.inTransType == TXN_E_READ_QR_CODE)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"200163",TRUE);
	//if(srTxnData.srParameter.chICERQRTxn == '1')
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],srTxnData.srIngData.chTMProcessCode,strlen((char *)srTxnData.srIngData.chTMProcessCode));

	//T0410
	/*memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),bTmp,5,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,5);
	strcat((char *)bBuf,"00");
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0410,bBuf,fXMLSendFlag);*/

	if(srTxnData.srIngData.inTransType == TXN_E_QR_DEDUCT || srTxnData.srIngData.inTransType == TXN_E_QR_VOID_ADD)//扣款
	{	//T0433
		memset(bBuf,0x00,sizeof(bBuf));
		sprintf((char *)bBuf,"%ld00",srTxnData.srIngData.lnECCAmt);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T0433",bBuf,fXMLSendFlag);
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],bBuf,strlen((char *)bBuf));
		if(srTxnData.srIngData.inTransType == TXN_E_QR_DEDUCT)
			memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],bBuf,strlen((char *)bBuf));
	}
	else if(srTxnData.srIngData.inTransType == TXN_E_QR_ADD || srTxnData.srIngData.inTransType == TXN_E_QR_REFUND)//加值
	{	//T0437
		memset(bBuf,0x00,sizeof(bBuf));
		sprintf((char *)bBuf,"%ld00",srTxnData.srIngData.lnECCAmt);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T0437",bBuf,fXMLSendFlag);
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],bBuf,strlen((char *)bBuf));
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],bBuf,strlen((char *)bBuf));
	}

	//T1100
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulTMSerialNumber);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1100,bBuf,fXMLSendFlag);
	//if(srTxnData.srParameter.chICERQRTxn == '1')
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],bBuf,strlen((char *)bBuf));

	//T1200
	//if(srTxnData.srParameter.chICERQRTxn == '1')
	{
		memcpy((char *)bBuf,srTxnData.srIngData.chTxTime,sizeof(srTxnData.srIngData.chTxTime) - 1);
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],bBuf,strlen((char *)bBuf));
	}

	//T1300
	//if(srTxnData.srParameter.chICERQRTxn == '1')
	{
		memcpy((char *)bBuf,srTxnData.srIngData.chTxDate,sizeof(srTxnData.srIngData.chTxDate) - 1);
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],bBuf,strlen((char *)bBuf));
	}

	//T1400
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1400,(BYTE *)"9912",fXMLSendFlag);

	//T3700
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_3700,srTxnData.srIngData.anRRN,fXMLSendFlag);
	//if(srTxnData.srParameter.chICERQRTxn == '1')
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],srTxnData.srIngData.anRRN,strlen((char *)srTxnData.srIngData.anRRN));

	//T4100
	memset(bBuf,0x00,sizeof(bBuf));
	fnBINTODEVASC(gTmpData.ucDeviceID,bBuf,9,MIFARE);//n_TXN Device ID
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4100,bBuf,fXMLSendFlag);
	//if(srTxnData.srParameter.chICERQRTxn == '1')
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],bBuf,strlen((char *)bBuf));

	//T4108 off
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4108);

	//T4110
	memset(bBuf,0x00,sizeof(bBuf));
	fnBINTODEVASC(gTmpData.ucCPUDeviceID,bBuf,16,LEVEL2);//n_TXN Device ID
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4110,bBuf,fXMLSendFlag);

	//T4200
	memset(bBuf,0x00,sizeof(bBuf));
	memcpy((char *)bBuf,srTxnData.srParameter.bMerchantID,strlen((char *)srTxnData.srParameter.bMerchantID));
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,bBuf,fXMLSendFlag);
	//if(srTxnData.srParameter.chICERQRTxn == '1')
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],bBuf,strlen((char *)bBuf));

	//T5371
	vdUnPackToXMLData(gTmpData.ucCPUSAMID,sizeof(gTmpData.ucCPUSAMID),srXML,(char *)"T5371",strlen("T5371"),VALUE_NAME,fXMLSendFlag);

	//T5503
	memset(bBuf,0x00,sizeof(bBuf));
	memcpy((char *)bBuf,srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID,sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID) - 1);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_5503,bBuf,fXMLSendFlag);
	//if(srTxnData.srParameter.chICERQRTxn == '1')
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],bBuf,strlen((char *)bBuf));

	//T5509//上線一定是E,目前I僅供測試!!
	if(srTxnData.srIngData.inTransType == TXN_E_READ_QR_CODE)
	//if(srTxnData.srParameter.chICERQRTxn == '1')
		inXMLAppendData(srXML,(char *)"T5509",strlen("T5509"),(BYTE *)"X",1,VALUE_NAME,fXMLSendFlag);
	else
		inXMLAppendData(srXML,(char *)"T5509",strlen("T5509"),(BYTE *)"E",1,VALUE_NAME,fXMLSendFlag);

	//T5510
	inXMLAppendData(srXML,(char *)"T5510",strlen("T5510"),(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber,4,VALUE_NAME,fXMLSendFlag);

	//T5515//暫時寫死,到時拿掉!!
	//if(srTxnData.srParameter.chICERQRTxn == '1')
		inXMLAppendData(srXML,(char *)"T5515",strlen("T5515"),(BYTE *)"Y",1,VALUE_NAME,fXMLSendFlag);

	//T554801 + T554808 + T6400 + T6406
	/*if(srTxnData.srParameter.chICERQRTxn != '1')
		vdBuildICERAData(inTxnType,srXML,Dongle_In,Dongle_Out);*/

	//T6410
	//if(srTxnData.srParameter.chICERQRTxn == '1')
	{
		int inT6410Len;
		memset(bBuf,0x00,sizeof(bBuf));
		memset(bTmp,0x00,sizeof(bTmp));
		sprintf((char *)bBuf,"0000000000000000");

		PKCS5Padding(bT6410Data,strlen((char *)bT6410Data),16);
		inT6410Len = strlen((char *)bT6410Data);
		log_msg_debug(LOG_LEVEL_FLOW,FALSE,(char *)"bICERKey :",10,srTxnData.srParameter.bICERKey,strlen((char *)srTxnData.srParameter.bICERKey));
		log_msg_debug(LOG_LEVEL_FLOW,FALSE,(char *)"bT6410Data :",12,bT6410Data,inT6410Len);
		ICER_AES_CBC_Func(0, srTxnData.srParameter.bICERKey, 2, bBuf, bT6410Data, inT6410Len, bTmp);
		log_msg_debug(LOG_LEVEL_FLOW,TRUE,(char *)"bT6410Out :",11,bTmp,inT6410Len);
		memset(bBuf,0x00,sizeof(bBuf));
		fnUnPack(&bTmp[inT6410Len - 8],8,bBuf);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T6410",bBuf,fXMLSendFlag);


		//TC101
		memset(bBuf,0x00,sizeof(bBuf));
		strcat((char *)bBuf,"EASYWALLET");
		inXMLAppendData(srXML,(char *)"TC101",strlen("TC101"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}

	//T6412
	/*if(srTxnData.srParameter.chICERQRTxn == '1')
	{
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy((char *)bBuf,"39DFA55283318D31AFE5A3FF4A0E3253E2045E43",40);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T6412",bBuf,fXMLSendFlag);
	}*/

	return SUCCESS;
}

int inBuildCMSCBikeReadTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
BYTE bBuf[200],bTmp[50];
//CBikeRead_APDU_In *DongleIn = (CBikeRead_APDU_In *)Dongle_In;
CBikeRead_APDU_Out *DongleOut = (CBikeRead_APDU_Out *)Dongle_Out;
BOOL fXMLSendFlag = TRUE;
int inCntStart,inCntEnd;

	//if(DongleOut->ucStatusCode[0] != 0x64 || DongleOut->ucStatusCode[1] != 0x15)
	//	return SUCCESS;

	//TAG_TRANS_XML_HEADER
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_TRANS_XML_HEADER);

	//TAG_TRANS_HEADER
	if(inXMLSearchValueByTag(srXML,(char *)TAG_TRANS_HEADER,&inCntStart,&inCntEnd,0) >= SUCCESS)
	{
		srXML->srXMLElement[inCntStart].fXMLSendFlag = TRUE;
		//vdSetConfigFlag(srXML,FALSE,TAG_TRANS_HEADER);
		memcpy(srXML->srXMLElement[inCntStart].chTag,TAG_TRANS_HEADER,strlen(TAG_TRANS_HEADER));
	}

	//T0100
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0100,(BYTE *)"0100",fXMLSendFlag);

	//T0200
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdUIntToAsc(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),bTmp,17,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,17);
	inXMLAppendData(srXML,(char *)TAG_NAME_0200,strlen(TAG_NAME_0200),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T0211
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucPID,sizeof(DongleOut->ucPID),srXML,(char *)TAG_NAME_0211,strlen(TAG_NAME_0211),VALUE_NAME,fXMLSendFlag);

	//T0212
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%02x",DongleOut->bAutoLoad);
	inXMLAppendData(srXML,(char *)TAG_NAME_0212,strlen(TAG_NAME_0212),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T0213
	vdUnPackToXMLData(&DongleOut->ucCardType,sizeof(DongleOut->ucCardType),srXML,(char *)TAG_NAME_0213,strlen(TAG_NAME_0213),VALUE_NAME,fXMLSendFlag);

	//T0214
	vdUnPackToXMLData(&DongleOut->ucPersonalProfile,sizeof(DongleOut->ucPersonalProfile),srXML,(char *)TAG_NAME_0214,strlen(TAG_NAME_0214),VALUE_NAME,fXMLSendFlag);

	//T0300
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"816399",TRUE);

	//T0400
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%ld",srTxnData.srIngData.lnECCAmt);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0400,bBuf,fXMLSendFlag);

	//T0409
	/*memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdUIntToAsc(DongleOut->ucAutoLoadAmt,sizeof(DongleOut->ucAutoLoadAmt),bTmp,5,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,5);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0409,bBuf,fXMLSendFlag);*/

	//T0410
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),bTmp,5,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,5);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0410,bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0410,strlen(TAG_NAME_0410),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1100
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulCMASHostSerialNumber);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1100,bBuf,fXMLSendFlag);

	//T1101
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulTMSerialNumber);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1101,bBuf,fXMLSendFlag);

	//T1201
	memset(bBuf,0x00,sizeof(bBuf));
	memcpy((char *)bBuf,srTxnData.srIngData.chTxTime,sizeof(srTxnData.srIngData.chTxTime) - 1);
	inXMLAppendData(srXML,(char *)"T1201",strlen("T1201"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1301
	memset(bBuf,0x00,sizeof(bBuf));
	memcpy((char *)bBuf,srTxnData.srIngData.chTxDate,sizeof(srTxnData.srIngData.chTxDate) - 1);
	inXMLAppendData(srXML,(char *)"T1301",strlen("T1301"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1402
	memset(bBuf,0x00,sizeof(bBuf));
	UnixToDateTime((BYTE*)DongleOut->ucExpiryDate,(BYTE*)bBuf,8);//Expiry Date
	inXMLAppendData(srXML,(char *)TAG_NAME_1402,strlen(TAG_NAME_1402),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T3700
	vdCMASBuildRRNProcess(srXML,srTxnData.srIngData.ulCMASHostSerialNumber,DongleOut->ucTxnSN);

	//T4100
	vdUnPackToXMLData(DongleOut->ucCPUDeviceID,sizeof(DongleOut->ucCPUDeviceID),srXML,(char *)TAG_NAME_4100,strlen(TAG_NAME_4100),VALUE_NAME,fXMLSendFlag);

	//T4101
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(DongleOut->ucDeviceID,sizeof(DongleOut->ucDeviceID),srXML,(char *)"T4101",strlen("T4101"),VALUE_NAME,fXMLSendFlag);

	//T4102
	memset(bBuf,0x00,sizeof(bBuf));
	vdGetLocalIP(bBuf);
	inXMLAppendData(srXML,(char *)"T4102",strlen("T4102"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T4104
	vdUnPackToXMLData(gTmpData.ucReaderID,sizeof(gTmpData.ucReaderID),srXML,(char *)TAG_NAME_4104,strlen(TAG_NAME_4104),VALUE_NAME,fXMLSendFlag);

	//T4107
	if(strlen(srTxnData.srParameter.chPOS_ID) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4107,(BYTE *)srTxnData.srParameter.chPOS_ID,fXMLSendFlag);

	//T4108 off
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4108);

	//T4117
	if(strlen(srTxnData.srParameter.chPOS_ID2) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4117,(BYTE *)srTxnData.srParameter.chPOS_ID2,fXMLSendFlag);

	//T4200//銀行版-->TM Location ID,一般版-->New SP ID,商店模式-->TM Location ID
	if(srTxnData.srParameter.chCMASMode == '1' || srTxnData.srParameter.chCMASMode == '2')//銀行併機 or 商店模式
	{
		//inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID,fXMLSendFlag);
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy(bBuf,&srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID[2],8);
		//memcpy(bBuf,&DongleIn->ucTMLocationID[2],8);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,bBuf,fXMLSendFlag);
	}
	else//一般版
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(DongleOut->ucCPUSPID,sizeof(DongleOut->ucCPUSPID),bBuf,8,FALSE,'0',10);
		//vdUIntToAsc(&DongleOut->ucCPUDeviceID[3],3,bBuf,8,FALSE,'0',10);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,bBuf,fXMLSendFlag);
		//inXMLAppendData(srXML,(char *)TAG_NAME_4200,strlen(TAG_NAME_4200),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}

	//T4210
	memset(bBuf,0x00,sizeof(bBuf));
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUIntToAsc(&DongleOut->ucLocationID,1,bBuf,3,TRUE,0x00,10);
	else
		vdUIntToAsc(DongleOut->ucCPULocationID,2,bBuf,5,TRUE,0x00,10);
	inXMLAppendData(srXML,(char *)"T4210",strlen("T4210"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T4800
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucPurseVersionNumber,sizeof(DongleOut->ucPurseVersionNumber),srXML,(char *)TAG_NAME_4800,strlen(TAG_NAME_4800),VALUE_NAME,fXMLSendFlag);

	//T4801,33 byte unpack to 66 byte,後補0至90 byte
	vdBuildCMASCardAVR((BYTE *)&DongleOut->stLastCreditTxnLogInfo_t,sizeof(DongleOut->stLastCreditTxnLogInfo_t),srXML,(char *)"T4801",strlen("T4801"),VALUE_NAME,fXMLSendFlag);

	//T4802
	vdUnPackToXMLData(&DongleOut->ucIssuerCode,sizeof(DongleOut->ucIssuerCode),srXML,(char *)"T4802",strlen("T4802"),VALUE_NAME,fXMLSendFlag);

	//T4803
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(&DongleOut->ucBankCode,sizeof(DongleOut->ucBankCode),srXML,(char *)TAG_NAME_4803,strlen(TAG_NAME_4803),VALUE_NAME,fXMLSendFlag);
	//else
	//	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4803,(BYTE *)"00",fXMLSendFlag);

	//T4804
	vdUnPackToXMLData(&DongleOut->ucAreaCode,sizeof(DongleOut->ucAreaCode),srXML,(char *)TAG_NAME_4804,strlen(TAG_NAME_4804),VALUE_NAME,fXMLSendFlag);

	//T4805
	vdUnPackToXMLData(DongleOut->ucSubAreaCode,sizeof(DongleOut->ucSubAreaCode),srXML,(char *)TAG_NAME_4805,strlen(TAG_NAME_4805),VALUE_NAME,fXMLSendFlag);

	//T4806
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucProfileExpiryDate,sizeof(DongleOut->ucProfileExpiryDate),srXML,(char *)"T4806",strlen("T4806"),VALUE_NAME,fXMLSendFlag);

	//T4808
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
	inXMLAppendData(srXML,(char *)"T4808",strlen("T4808"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),srXML,(char *)"T4808",strlen("T4808"),VALUE_NAME,fXMLSendFlag);

	//T4809
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
	//	vdUnPackToXMLData(&DongleOut->ucTxnMode,sizeof(DongleOut->ucTxnMode),srXML,(char *)"T4809",strlen("T4809"),VALUE_NAME,fXMLSendFlag);

	//T4810
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
	//	vdUnPackToXMLData(&DongleOut->ucTxnQuqlifier,sizeof(DongleOut->ucTxnQuqlifier),srXML,(char *)"T4810",strlen("T4810"),VALUE_NAME,fXMLSendFlag);

	//T4811
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
	inXMLAppendData(srXML,(char *)"T4811",strlen("T4811"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),srXML,(char *)"T4811",strlen("T4811"),VALUE_NAME,fXMLSendFlag);

	//T4812
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
	//	vdUnPackToXMLData(DongleOut->ucCTC,sizeof(DongleOut->ucCTC),srXML,(char *)"T4812",strlen("T4812"),VALUE_NAME,fXMLSendFlag);

	//T4813
	/*if(DongleOut->ucPurseVersionNumber == MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(&DongleOut->ucCPD_SAMID[0],2,bBuf,6,TRUE,0x00,10);
		vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
		inXMLAppendData(srXML,(char *)"T4813",strlen("T4813"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
		//vdUnPackToXMLData(DongleOut->ucLoyaltyCounter,sizeof(DongleOut->ucLoyaltyCounter),srXML,(char *)"T4813",strlen("T4813"),VALUE_NAME,fXMLSendFlag);
	}*/
	//else
	//	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4813",(BYTE *)"0000",fXMLSendFlag);

	//T4814
	/*if(DongleOut->ucPurseVersionNumber != MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(DongleOut->ucDeposit,3,bBuf,6,TRUE,0x00,10);
		vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
		inXMLAppendData(srXML,(char *)"T4814",strlen("T4814"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
		//vdUnPackToXMLData(DongleOut->ucDeposit,sizeof(DongleOut->ucDeposit),srXML,(char *)TAG_NAME_4814,strlen(TAG_NAME_4814),VALUE_NAME,fXMLSendFlag);
	}*/

	//T4826
	vdUnPackToXMLData(&DongleOut->ucCardIDLen,sizeof(DongleOut->ucCardIDLen),srXML,(char *)"T4826",strlen("T4826"),VALUE_NAME,fXMLSendFlag);

	//T5301
	//vdUnPackToXMLData(&DongleOut->ucCPDKVN_SAMKVN,sizeof(DongleOut->ucCPDKVN_SAMKVN),srXML,(char *)"T5301",strlen("T5301"),VALUE_NAME,fXMLSendFlag);

	//T5302
	/*if(DongleOut->ucPurseVersionNumber != MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		sprintf((char *)bBuf,"%02x%02x%02x",DongleOut->ucCPUAdminKeyKVN,DongleOut->ucCreditKeyKVN,DongleOut->ucCPUIssuerKeyKVN);
		inXMLAppendData(srXML,(char *)"T5302",strlen("T5302"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}*/

	//T5304
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
		//vdUnPackToXMLData(&DongleOut->ucBankCode_HostAdminKVN,sizeof(DongleOut->ucBankCode_HostAdminKVN),srXML,(char *)"T5304",strlen("T5304"),VALUE_NAME,fXMLSendFlag);

	//T5305
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
		//vdUnPackToXMLData(&DongleOut->ucSignatureKeyKVN,sizeof(DongleOut->ucSignatureKeyKVN),srXML,(char *)"T5305",strlen("T5305"),VALUE_NAME,fXMLSendFlag);

	//T5361
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
		//vdUnPackToXMLData(&DongleOut->ucCPD_SAMID[2],8,srXML,(char *)"T5361",strlen("T5361"),VALUE_NAME,fXMLSendFlag);

	//T5362
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
		//vdUnPackToXMLData(&DongleOut->ucCPD_SAMID[10],4,srXML,(char *)"T5362",strlen("T5362"),VALUE_NAME,fXMLSendFlag);

	//T5363
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
		//vdUnPackToXMLData(DongleOut->ucCPDRAN_SAMCRN,sizeof(DongleOut->ucCPDRAN_SAMCRN),srXML,(char *)"T5363",strlen("T5363"),VALUE_NAME,fXMLSendFlag);

	//T5371
	//vdUnPackToXMLData(DongleOut->ucSID_STAC,sizeof(DongleOut->ucSID_STAC),srXML,(char *)"T5371",strlen("T5371"),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucCPUSAMID,sizeof(DongleOut->ucCPUSAMID),srXML,(char *)"T5371",strlen("T5371"),VALUE_NAME,fXMLSendFlag);

	//T5503
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5503",(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T5503",strlen("T5503"),(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID,10,VALUE_NAME,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T5503",strlen("T5503"),DongleIn->ucTMLocationID,10,VALUE_NAME,fXMLSendFlag);

	//T5504
	inXMLAppendData(srXML,(char *)"T5504",strlen("T5504"),(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMID,2,VALUE_NAME,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T5504",strlen("T5504"),DongleIn->ucTMID,2,VALUE_NAME,fXMLSendFlag);

	//T5510
	inXMLAppendData(srXML,(char *)"T5510",strlen("T5510"),(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber,4,VALUE_NAME,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T5510",strlen("T5510"),DongleIn->ucTMAgentNumber,4,VALUE_NAME,fXMLSendFlag);

	//T5533
	//vdSetPointDetail(inTxnType,srXML);

	//T6000
	vdUnPackToXMLData(gTmpData.ucReaderFWVersion,sizeof(gTmpData.ucReaderFWVersion),srXML,(char *)"T6000",strlen("T6000"),VALUE_NAME,fXMLSendFlag);

	//T6000
	//vdUnPackToXMLData(DongleOut->ucReaderFWVersion,sizeof(DongleOut->ucReaderFWVersion),srXML,(char *)"T6000",strlen("T6000"),VALUE_NAME,fXMLSendFlag);

	//T6004
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	inGetBLCName(bTmp);
	memcpy(bBuf,bTmp,5);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T6004",bBuf,fXMLSendFlag);

	//T6400
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
	//	vdUnPackToXMLData(DongleOut->ucSID_STAC,sizeof(DongleOut->ucSID_STAC),srXML,(char *)"T6400",strlen("T6400"),VALUE_NAME,fXMLSendFlag);

	//T6406
	/*if(DongleOut->ucPurseVersionNumber != MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy((char *)bBuf,DongleOut->ucSVCrypto,sizeof(DongleOut->ucSVCrypto));
		vdUnPackToXMLData(bBuf,sizeof(DongleOut->ucSVCrypto),srXML,(char *)"T6406",strlen("T6406"),VALUE_NAME,fXMLSendFlag);
	}*/

	return SUCCESS;
}

int inBuildCMSCBikeDeductTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
BYTE bBuf[50];
//TaxiDeduct_APDU_In *DongleIn = (TaxiDeduct_APDU_In *)Dongle_In;
CBikeDeduct_APDU_Out *DongleOut = (CBikeDeduct_APDU_Out *)Dongle_Out;
CBikeRead_APDU_Out *DongleReqOut = (CBikeRead_APDU_Out *)Dongle_ReqOut;
BOOL fXMLSendFlag = TRUE;
int inCntStart,inCntEnd;
BYTE bDate[20];

	//TAG_TRANS_XML_HEADER
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_TRANS_XML_HEADER);

	//TAG_TRANS_HEADER
	if(inXMLSearchValueByTag(srXML,(char *)TAG_TRANS_HEADER,&inCntStart,&inCntEnd,0) >= SUCCESS)
	{
		srXML->srXMLElement[inCntStart].fXMLSendFlag = TRUE;
		//vdSetConfigFlag(srXML,FALSE,TAG_TRANS_HEADER);
		memcpy(srXMLData.srXMLElement[inCntStart].chTag,TAG_TRANS_HEADER,strlen(TAG_TRANS_HEADER));
	}

	//T0100
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0100,(BYTE *)"0220",TRUE);

	//T0200
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0200);

	//T0211
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0211);

	//T0213
	vdUnPackToXMLData(&DongleReqOut->ucCardType,sizeof(DongleReqOut->ucCardType),srXML,(char *)TAG_NAME_0213,strlen(TAG_NAME_0213),VALUE_NAME,fXMLSendFlag);

	//T0214
	vdUnPackToXMLData(&DongleReqOut->ucPersonalProfile,sizeof(DongleReqOut->ucPersonalProfile),srXML,(char *)TAG_NAME_0214,strlen(TAG_NAME_0214),VALUE_NAME,fXMLSendFlag);

	//T0300
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"811599",TRUE);

	//T0400
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%ld",srTxnData.srIngData.lnECCAmt);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0400,bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0400,strlen(TAG_NAME_0400),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T0404,社福點數
	//vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0404);
	memset(bBuf,0x00,sizeof(bBuf));
	//sprintf((char *)bBuf,"%d",1);
	//inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0404,bBuf,fXMLSendFlag);
	if(inXMLGetData(srXML,(char *)TAG_NAME_0404,(char *)bBuf,sizeof(bBuf),0) < SUCCESS)
		log_msg(LOG_LEVEL_ERROR,"inBuildCMSCBikeDeductTM Fail 1: NO T0404");
	if(strlen((char *)bBuf) <= 2)//肯定沒有2碼小數
	{
		vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0404);
		log_msg(LOG_LEVEL_ERROR,"inBuildCMSCBikeDeductTM  2!!");
	}
	else if(memcmp((char *)&bBuf[strlen((char *)bBuf) - 2],"00",2))//最後2碼不是00
	{
		vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0404);
		log_msg(LOG_LEVEL_ERROR,"inBuildCMSCBikeDeductTM Fail 3!!");
	}
	else//最後2碼肯定是00,去掉後2碼
	{
		bBuf[strlen((char *)bBuf) - 2] = 0x00;
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0404,bBuf,fXMLSendFlag);
		log_msg(LOG_LEVEL_ERROR,"inBuildCMSCBikeDeductTM Fail 4!!");
	}

	//T0408
	memset(bBuf,0x00,sizeof(bBuf));
	vdIntToAsc(DongleOut->stDeductOut_t.ucEV,sizeof(DongleOut->stDeductOut_t.ucEV),bBuf,5,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0408,bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0408,strlen(TAG_NAME_0408),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	srTxnData.srIngData.lnECCEV = lnAmt3ByteToLong(DongleOut->stDeductOut_t.ucEV);

	//T0409
	if(strlen((char *)srTxnData.srIngData.anAutoloadAmount) > 2)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy(bBuf,srTxnData.srIngData.anAutoloadAmount,strlen((char *)srTxnData.srIngData.anAutoloadAmount) - 2);
		//vdUIntToAsc(DongleReqOut->ucAutoLoadAmt,sizeof(DongleReqOut->ucAutoLoadAmt),bBuf,5,TRUE,0x00,10);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0409,bBuf,fXMLSendFlag);
		//inXMLAppendData(srXML,(char *)TAG_NAME_0409,strlen(TAG_NAME_0409),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}

	//T0410
	memset(bBuf,0x00,sizeof(bBuf));
	vdIntToAsc(DongleReqOut->ucEV,sizeof(DongleReqOut->ucEV),bBuf,5,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0410,bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0410,strlen(TAG_NAME_0410),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1100
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_1100);

	//T1101
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_1101);

	memset(bDate,0x00,sizeof(bDate));
	UnixToDateTime((BYTE*)DongleOut->stDeductOut_t.ucTxnDateTime,(BYTE*)bDate,14);
	//T1200
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1200,&bDate[8],TRUE);

	//T1201
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T1201",&bDate[8],TRUE);
	bDate[8] = 0x00;
	//vdSetConfigFlag(srXML,FALSE,(char *)"T1201");

	//T1300
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1300,bDate,TRUE);
	//vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_1300);

	//T1301
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T1301",bDate,TRUE);
	//vdSetConfigFlag(srXML,FALSE,(char *)"T1301");

	//T3700
	vdCMASBuildRRNProcess(srXML,srTxnData.srIngData.ulCMASHostSerialNumber,DongleReqOut->ucTxnSN);

	//T4100
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4100);

	//T4101
	//if(DongleReqOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(DongleReqOut->ucDeviceID,sizeof(DongleReqOut->ucDeviceID),srXML,(char *)"T4101",strlen("T4101"),VALUE_NAME,fXMLSendFlag);

	//T4104
	memset(bBuf,0x00,sizeof(bBuf));
	if(DongleReqOut->ucPurseVersionNumber == MIFARE)
	{
		fnUnPack(&DongleOut->stDeductOut_t.ucMAC_HCrypto[6],4,bBuf);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4104",bBuf,fXMLSendFlag);
	}
	else
	{
		vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4104);
		//sprintf((char *)bBuf,"00000000");
	}

	//T4107
	if(strlen(srTxnData.srParameter.chPOS_ID) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4107,(BYTE *)srTxnData.srParameter.chPOS_ID,fXMLSendFlag);

	//T4108 off
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4108);

	//T4117
	if(strlen(srTxnData.srParameter.chPOS_ID2) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4117,(BYTE *)srTxnData.srParameter.chPOS_ID2,fXMLSendFlag);

	//T4200
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4200);

	//T4210
	vdSetConfigFlag(srXML,FALSE,(char *)"T4210");

	//T4800
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4800);

	//T4801,33 byte unpack to 66 byte,後補0至90 byte
	vdBuildCMASCardAVR((BYTE *)&DongleReqOut->stLastCreditTxnLogInfo_t,sizeof(DongleReqOut->stLastCreditTxnLogInfo_t),srXML,(char *)"T4801",strlen("T4801"),VALUE_NAME,fXMLSendFlag);

	//T4802
	vdUnPackToXMLData(&DongleReqOut->ucIssuerCode,sizeof(DongleReqOut->ucIssuerCode),srXML,(char *)"T4802",strlen("T4802"),VALUE_NAME,fXMLSendFlag);

	//T4803
	vdUnPackToXMLData(&DongleReqOut->ucBankCode,sizeof(DongleReqOut->ucBankCode),srXML,(char *)TAG_NAME_4803,strlen(TAG_NAME_4803),VALUE_NAME,fXMLSendFlag);

	//T4804
	vdUnPackToXMLData(&DongleReqOut->ucAreaCode,sizeof(DongleReqOut->ucAreaCode),srXML,(char *)TAG_NAME_4804,strlen(TAG_NAME_4804),VALUE_NAME,fXMLSendFlag);

	//T4805
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleReqOut->ucSubAreaCode,sizeof(DongleReqOut->ucSubAreaCode),srXML,(char *)TAG_NAME_4805,strlen(TAG_NAME_4805),VALUE_NAME,fXMLSendFlag);

	//T4808
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->stDeductOut_t.ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4808",bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T4808",strlen("T4808"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),srXML,(char *)"T4808",strlen("T4808"),VALUE_NAME,fXMLSendFlag);

	//T4809
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->stDeductOut_t.ucTxnMode,sizeof(DongleOut->stDeductOut_t.ucTxnMode),srXML,(char *)"T4809",strlen("T4809"),VALUE_NAME,fXMLSendFlag);

	//T4810
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->stDeductOut_t.ucTxnQuqlifier,sizeof(DongleOut->stDeductOut_t.ucTxnQuqlifier),srXML,(char *)"T4810",strlen("T4810"),VALUE_NAME,fXMLSendFlag);

	//T4811
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleReqOut->ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4811",bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T4811",strlen("T4811"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleReqOut->ucTxnSN,sizeof(DongleReqOut->ucTxnSN),srXML,(char *)"T4811",strlen("T4811"),VALUE_NAME,fXMLSendFlag);

	//T4812
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->stDeductOut_t.ucCTC,sizeof(DongleOut->stDeductOut_t.ucCTC),srXML,(char *)"T4812",strlen("T4812"),VALUE_NAME,fXMLSendFlag);

	//T4813
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleReqOut->ucLoyaltyCounter,2,bBuf,6,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4813",bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T4813",strlen("T4813"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleReqOut->ucLoyaltyCounter,sizeof(DongleReqOut->ucLoyaltyCounter),srXML,(char *)"T4813",strlen("T4813"),VALUE_NAME,fXMLSendFlag);

	//T4826
	vdSetConfigFlag(srXML,FALSE,(char *)"T4826");

	//T4833
	//if(DongleReqOut->ucPurseVersionNumber != MIFARE)
	vdUnPackToXMLData(&DongleOut->ucTransferGuoupCode,1,srXML,(char *)TAG_NAME_4833,strlen(TAG_NAME_4833),VALUE_NAME,fXMLSendFlag);

	//T4835
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4835);

	//T4836
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4836);

	//T4837
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4837);

	//T4838
	//vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4838);

	//T5301
	//vdUnPackToXMLData(&DongleReqOut->ucSAMKVN,sizeof(DongleReqOut->ucSAMKVN),srXML,(char *)"T5301",strlen("T5301"),VALUE_NAME,fXMLSendFlag);

	//T5303
	//if(DongleReqOut->ucPurseVersionNumber != MIFARE)
	vdUnPackToXMLData(&DongleOut->ucHashType,1,srXML,(char *)"T5303",strlen("T5303"),VALUE_NAME,fXMLSendFlag);

	//T5304
	//if(DongleReqOut->ucPurseVersionNumber != MIFARE)
	vdUnPackToXMLData(&DongleOut->ucHostAdminKVN,1,srXML,(char *)"T5304",strlen("T5304"),VALUE_NAME,fXMLSendFlag);

	//T5305
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucSignatureKeyKVN,sizeof(DongleOut->ucSignatureKeyKVN),srXML,(char *)"T5305",strlen("T5305"),VALUE_NAME,fXMLSendFlag);

	//T5361
	//vdUnPackToXMLData(DongleReqOut->ucSAMID,sizeof(DongleReqOut->ucSAMID),srXML,(char *)"T5361",strlen("T5361"),VALUE_NAME,fXMLSendFlag);

	//T5362
	//vdUnPackToXMLData(DongleReqOut->ucSAMSN,sizeof(DongleReqOut->ucSAMSN),srXML,(char *)"T5362",strlen("T5362"),VALUE_NAME,fXMLSendFlag);

	//T5363
	//vdUnPackToXMLData(DongleReqOut->ucSAMCRN,sizeof(DongleReqOut->ucSAMCRN),srXML,(char *)"T5363",strlen("T5363"),VALUE_NAME,fXMLSendFlag);

	//T5371
	vdUnPackToXMLData(DongleOut->ucCPUSAMID,sizeof(DongleOut->ucCPUSAMID),srXML,(char *)"T5371",strlen("T5371"),VALUE_NAME,fXMLSendFlag);

	//T5501
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_5501);

	//T5503
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_5503);

	//T5504
	vdSetConfigFlag(srXML,FALSE,(char *)"T5504");

	//T5510
	vdSetConfigFlag(srXML,FALSE,(char *)"T5510");

	//T5533
	//vdSetPointDetail(inTxnType,srXML);

	//T6000
	vdUnPackToXMLData(gTmpData.ucReaderFWVersion,sizeof(gTmpData.ucReaderFWVersion),srXML,(char *)"T6000",strlen("T6000"),VALUE_NAME,fXMLSendFlag);

	//T6402
	//if(DongleReqOut->ucPurseVersionNumber == MIFARE)
	//	vdUnPackToXMLData(&DongleOut->stDeductOut_t.ucMAC_HCrypto[10],8,srXML,(char *)"T6402",strlen("T6402"),VALUE_NAME,fXMLSendFlag);

	//T6403
	if(DongleReqOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(&DongleOut->stDeductOut_t.ucMAC_HCrypto[2],4,srXML,(char *)"T6403",strlen("T6403"),VALUE_NAME,fXMLSendFlag);

	//T6404
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
	{
		vdUnPackToXMLData(&DongleOut->stDeductOut_t.ucMAC_HCrypto[0],16,srXML,(char *)"T6404",strlen("T6404"),VALUE_NAME,fXMLSendFlag);
		vdUnPackToXMLData(DongleOut->stDeductOut_t.ucSignature,16,srXML,(char *)"T6405",strlen("T6405"),VALUE_NAME,fXMLSendFlag);
	}

	return SUCCESS;
}

//add by bean 20141027 start
int inBuildCMASMMSelectCard2Data(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
//int inTMOutLen;
//MMSelectCard2_TM_Out *APIOut = (MMSelectCard2_TM_Out *)API_Out;
//MMSelectCard2_APDU_In *DongleIn = (MMSelectCard2_APDU_In *)Dongle_In;
MMSelectCard2_APDU_Out *DongleOut = (MMSelectCard2_APDU_Out *)Dongle_Out;
BYTE bBuf[20 + 1];

	//inTMOutLen = sizeof(MMSelectCard2_TM_Out);

	memset(bBuf,0x00,sizeof(bBuf));
	fnUnPack(&DongleOut->ucCardIDLen,sizeof(DongleOut->ucCardIDLen),bBuf);
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0216,bBuf,FALSE);

	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),bBuf,10,TRUE,' ',10);//Txn Amt
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0217,bBuf,FALSE);

	memset(bBuf,0x00,sizeof(bBuf));
	fnUnPack(&DongleOut->ucCardType,sizeof(DongleOut->ucCardType),bBuf);
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0218,bBuf,FALSE);

	return SUCCESS;
}

int inBuildCMASEDCAReadTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
BYTE bBuf[200],bTmp[50];
//EDCARead_APDU_In *DongleIn = (EDCARead_APDU_In *)Dongle_In;
EDCARead_APDU_Out *DongleOut = (EDCARead_APDU_Out *)Dongle_Out;
BOOL fXMLSendFlag = TRUE;
int inCntStart,inCntEnd;

	//if(DongleOut->ucStatusCode[0] != 0x64 || DongleOut->ucStatusCode[1] != 0x15)
	//	return SUCCESS;

	//TAG_TRANS_XML_HEADER
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_TRANS_XML_HEADER);

	//TAG_TRANS_HEADER
	if(inXMLSearchValueByTag(srXML,(char *)TAG_TRANS_HEADER,&inCntStart,&inCntEnd,0) >= SUCCESS)
	{
		srXML->srXMLElement[inCntStart].fXMLSendFlag = TRUE;
		//vdSetConfigFlag(srXML,FALSE,TAG_TRANS_HEADER);
		memcpy(srXML->srXMLElement[inCntStart].chTag,TAG_TRANS_HEADER,strlen(TAG_TRANS_HEADER));
	}

	//T0100
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0100,(BYTE *)"0100",fXMLSendFlag);

	//T0200
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdUIntToAsc(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),bTmp,17,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,17);
	inXMLAppendData(srXML,(char *)TAG_NAME_0200,strlen(TAG_NAME_0200),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T0211
	if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucPID,sizeof(DongleOut->ucPID),srXML,(char *)TAG_NAME_0211,strlen(TAG_NAME_0211),VALUE_NAME,fXMLSendFlag);

	//T0212
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%02x",DongleOut->bAutoLoad);
	inXMLAppendData(srXML,(char *)TAG_NAME_0212,strlen(TAG_NAME_0212),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T0213
	vdUnPackToXMLData(&DongleOut->ucCardType,sizeof(DongleOut->ucCardType),srXML,(char *)TAG_NAME_0213,strlen(TAG_NAME_0213),VALUE_NAME,fXMLSendFlag);

	//T0214
	vdUnPackToXMLData(&DongleOut->ucPersonalProfile,sizeof(DongleOut->ucPersonalProfile),srXML,(char *)TAG_NAME_0214,strlen(TAG_NAME_0214),VALUE_NAME,fXMLSendFlag);

	//T0300
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"816399",TRUE);

	//T0400
	//memset(bBuf,0x00,sizeof(bBuf));
	//sprintf((char *)bBuf,"%ld",srTxnData.srIngData.lnECCAmt);
	//inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0400,bBuf,fXMLSendFlag);

	//T0409
	/*memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdUIntToAsc(DongleOut->ucAutoLoadAmt,sizeof(DongleOut->ucAutoLoadAmt),bTmp,5,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,5);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0409,bBuf,fXMLSendFlag);*/

	//T0410
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),bTmp,5,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,5);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0410,bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0410,strlen(TAG_NAME_0410),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1100
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulCMASHostSerialNumber);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1100,bBuf,fXMLSendFlag);

	//T1101
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulTMSerialNumber);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1101,bBuf,fXMLSendFlag);

	//T1201
	memset(bBuf,0x00,sizeof(bBuf));
	memcpy((char *)bBuf,srTxnData.srIngData.chTxTime,sizeof(srTxnData.srIngData.chTxTime) - 1);
	inXMLAppendData(srXML,(char *)"T1201",strlen("T1201"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1301
	memset(bBuf,0x00,sizeof(bBuf));
	memcpy((char *)bBuf,srTxnData.srIngData.chTxDate,sizeof(srTxnData.srIngData.chTxDate) - 1);
	inXMLAppendData(srXML,(char *)"T1301",strlen("T1301"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1402
	memset(bBuf,0x00,sizeof(bBuf));
	UnixToDateTime((BYTE*)DongleOut->ucExpiryDate,(BYTE*)bBuf,8);//Expiry Date
	inXMLAppendData(srXML,(char *)TAG_NAME_1402,strlen(TAG_NAME_1402),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T3700
	vdCMASBuildRRNProcess(srXML,srTxnData.srIngData.ulCMASHostSerialNumber,DongleOut->ucTxnSN);

	//T4100
	vdUnPackToXMLData(DongleOut->ucCPUDeviceID,sizeof(DongleOut->ucCPUDeviceID),srXML,(char *)TAG_NAME_4100,strlen(TAG_NAME_4100),VALUE_NAME,fXMLSendFlag);

	//T4101
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(DongleOut->ucDeviceID,sizeof(DongleOut->ucDeviceID),srXML,(char *)"T4101",strlen("T4101"),VALUE_NAME,fXMLSendFlag);

	//T4102
	memset(bBuf,0x00,sizeof(bBuf));
	vdGetLocalIP(bBuf);
	inXMLAppendData(srXML,(char *)"T4102",strlen("T4102"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T4104
	vdUnPackToXMLData(gTmpData.ucReaderID,sizeof(gTmpData.ucReaderID),srXML,(char *)TAG_NAME_4104,strlen(TAG_NAME_4104),VALUE_NAME,fXMLSendFlag);

	//T4107
	if(strlen(srTxnData.srParameter.chPOS_ID) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4107,(BYTE *)srTxnData.srParameter.chPOS_ID,fXMLSendFlag);

	//T4108 off
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4108);

	//T4117
	if(strlen(srTxnData.srParameter.chPOS_ID2) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4117,(BYTE *)srTxnData.srParameter.chPOS_ID2,fXMLSendFlag);

	//T4200//銀行版-->TM Location ID,一般版-->New SP ID,商店模式-->TM Location ID
	if(srTxnData.srParameter.chCMASMode == '1' || srTxnData.srParameter.chCMASMode == '2')//銀行併機 or 商店模式
	{
		//inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID,fXMLSendFlag);
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy(bBuf,&srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID[2],8);
		//memcpy(bBuf,&DongleIn->ucTMLocationID[2],8);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,bBuf,fXMLSendFlag);
	}
	else//一般版
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(DongleOut->ucCPUSPID,sizeof(DongleOut->ucCPUSPID),bBuf,8,FALSE,'0',10);
		//vdUIntToAsc(&DongleOut->ucCPUDeviceID[3],3,bBuf,8,FALSE,'0',10);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,bBuf,fXMLSendFlag);
		//inXMLAppendData(srXML,(char *)TAG_NAME_4200,strlen(TAG_NAME_4200),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}

	//T4210
	memset(bBuf,0x00,sizeof(bBuf));
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUIntToAsc(&DongleOut->ucLocationID,1,bBuf,3,TRUE,0x00,10);
	else
		vdUIntToAsc(DongleOut->ucCPULocationID,2,bBuf,5,TRUE,0x00,10);
	inXMLAppendData(srXML,(char *)"T4210",strlen("T4210"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T4800
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucPurseVersionNumber,sizeof(DongleOut->ucPurseVersionNumber),srXML,(char *)TAG_NAME_4800,strlen(TAG_NAME_4800),VALUE_NAME,fXMLSendFlag);

	//T4801,33 byte unpack to 66 byte,後補0至90 byte
	vdBuildCMASCardAVR((BYTE *)&DongleOut->stLastCreditTxnLogInfo_t,sizeof(DongleOut->stLastCreditTxnLogInfo_t),srXML,(char *)"T4801",strlen("T4801"),VALUE_NAME,fXMLSendFlag);

	//T4802
	vdUnPackToXMLData(&DongleOut->ucIssuerCode,sizeof(DongleOut->ucIssuerCode),srXML,(char *)"T4802",strlen("T4802"),VALUE_NAME,fXMLSendFlag);

	//T4803
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(&DongleOut->ucBankCode,sizeof(DongleOut->ucBankCode),srXML,(char *)TAG_NAME_4803,strlen(TAG_NAME_4803),VALUE_NAME,fXMLSendFlag);
	//else
	//	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4803,(BYTE *)"00",fXMLSendFlag);

	//T4804
	vdUnPackToXMLData(&DongleOut->ucAreaCode,sizeof(DongleOut->ucAreaCode),srXML,(char *)TAG_NAME_4804,strlen(TAG_NAME_4804),VALUE_NAME,fXMLSendFlag);

	//T4805
	vdUnPackToXMLData(DongleOut->ucSubAreaCode,sizeof(DongleOut->ucSubAreaCode),srXML,(char *)TAG_NAME_4805,strlen(TAG_NAME_4805),VALUE_NAME,fXMLSendFlag);

	//T4806
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->ucProfileExpiryDate,sizeof(DongleOut->ucProfileExpiryDate),srXML,(char *)"T4806",strlen("T4806"),VALUE_NAME,fXMLSendFlag);

	//T4808
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
	inXMLAppendData(srXML,(char *)"T4808",strlen("T4808"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),srXML,(char *)"T4808",strlen("T4808"),VALUE_NAME,fXMLSendFlag);

	//T4809
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
	//	vdUnPackToXMLData(&DongleOut->ucTxnMode,sizeof(DongleOut->ucTxnMode),srXML,(char *)"T4809",strlen("T4809"),VALUE_NAME,fXMLSendFlag);

	//T4810
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
	//	vdUnPackToXMLData(&DongleOut->ucTxnQuqlifier,sizeof(DongleOut->ucTxnQuqlifier),srXML,(char *)"T4810",strlen("T4810"),VALUE_NAME,fXMLSendFlag);

	//T4811
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
	inXMLAppendData(srXML,(char *)"T4811",strlen("T4811"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),srXML,(char *)"T4811",strlen("T4811"),VALUE_NAME,fXMLSendFlag);

	//T4812
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
	//	vdUnPackToXMLData(DongleOut->ucCTC,sizeof(DongleOut->ucCTC),srXML,(char *)"T4812",strlen("T4812"),VALUE_NAME,fXMLSendFlag);

	//T4813
	/*if(DongleOut->ucPurseVersionNumber == MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(&DongleOut->ucCPD_SAMID[0],2,bBuf,6,TRUE,0x00,10);
		vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
		inXMLAppendData(srXML,(char *)"T4813",strlen("T4813"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
		//vdUnPackToXMLData(DongleOut->ucLoyaltyCounter,sizeof(DongleOut->ucLoyaltyCounter),srXML,(char *)"T4813",strlen("T4813"),VALUE_NAME,fXMLSendFlag);
	}*/
	//else
	//	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4813",(BYTE *)"0000",fXMLSendFlag);

	//T4814
	/*if(DongleOut->ucPurseVersionNumber != MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		vdUIntToAsc(DongleOut->ucDeposit,3,bBuf,6,TRUE,0x00,10);
		vdPad((char *)bBuf,(char *)bBuf,6,FALSE,0x30);
		inXMLAppendData(srXML,(char *)"T4814",strlen("T4814"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
		//vdUnPackToXMLData(DongleOut->ucDeposit,sizeof(DongleOut->ucDeposit),srXML,(char *)TAG_NAME_4814,strlen(TAG_NAME_4814),VALUE_NAME,fXMLSendFlag);
	}*/

	//T4826
	vdUnPackToXMLData(&DongleOut->ucCardIDLen,sizeof(DongleOut->ucCardIDLen),srXML,(char *)"T4826",strlen("T4826"),VALUE_NAME,fXMLSendFlag);

	//T5301
	//vdUnPackToXMLData(&DongleOut->ucCPDKVN_SAMKVN,sizeof(DongleOut->ucCPDKVN_SAMKVN),srXML,(char *)"T5301",strlen("T5301"),VALUE_NAME,fXMLSendFlag);

	//T5302
	/*if(DongleOut->ucPurseVersionNumber != MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		sprintf((char *)bBuf,"%02x%02x%02x",DongleOut->ucCPUAdminKeyKVN,DongleOut->ucCreditKeyKVN,DongleOut->ucCPUIssuerKeyKVN);
		inXMLAppendData(srXML,(char *)"T5302",strlen("T5302"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	}*/

	//T5304
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
		//vdUnPackToXMLData(&DongleOut->ucBankCode_HostAdminKVN,sizeof(DongleOut->ucBankCode_HostAdminKVN),srXML,(char *)"T5304",strlen("T5304"),VALUE_NAME,fXMLSendFlag);

	//T5305
	//if(DongleOut->ucPurseVersionNumber != MIFARE)
		//vdUnPackToXMLData(&DongleOut->ucSignatureKeyKVN,sizeof(DongleOut->ucSignatureKeyKVN),srXML,(char *)"T5305",strlen("T5305"),VALUE_NAME,fXMLSendFlag);

	//T5361
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
		//vdUnPackToXMLData(&DongleOut->ucCPD_SAMID[2],8,srXML,(char *)"T5361",strlen("T5361"),VALUE_NAME,fXMLSendFlag);

	//T5362
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
		//vdUnPackToXMLData(&DongleOut->ucCPD_SAMID[10],4,srXML,(char *)"T5362",strlen("T5362"),VALUE_NAME,fXMLSendFlag);

	//T5363
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
		//vdUnPackToXMLData(DongleOut->ucCPDRAN_SAMCRN,sizeof(DongleOut->ucCPDRAN_SAMCRN),srXML,(char *)"T5363",strlen("T5363"),VALUE_NAME,fXMLSendFlag);

	//T5371
	//vdUnPackToXMLData(DongleOut->ucSID_STAC,sizeof(DongleOut->ucSID_STAC),srXML,(char *)"T5371",strlen("T5371"),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucCPUSAMID,sizeof(DongleOut->ucCPUSAMID),srXML,(char *)"T5371",strlen("T5371"),VALUE_NAME,fXMLSendFlag);

	//T5503
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5503",(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T5503",strlen("T5503"),(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID,10,VALUE_NAME,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T5503",strlen("T5503"),DongleIn->ucTMLocationID,10,VALUE_NAME,fXMLSendFlag);

	//T5504
	inXMLAppendData(srXML,(char *)"T5504",strlen("T5504"),(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMID,2,VALUE_NAME,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T5504",strlen("T5504"),DongleIn->ucTMID,2,VALUE_NAME,fXMLSendFlag);

	//T5510
	inXMLAppendData(srXML,(char *)"T5510",strlen("T5510"),(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber,4,VALUE_NAME,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T5510",strlen("T5510"),DongleIn->ucTMAgentNumber,4,VALUE_NAME,fXMLSendFlag);

	//T5533
	//vdSetPointDetail(inTxnType,srXML);

	//T6000
	vdUnPackToXMLData(gTmpData.ucReaderFWVersion,sizeof(gTmpData.ucReaderFWVersion),srXML,(char *)"T6000",strlen("T6000"),VALUE_NAME,fXMLSendFlag);

	//T6000
	//vdUnPackToXMLData(DongleOut->ucReaderFWVersion,sizeof(DongleOut->ucReaderFWVersion),srXML,(char *)"T6000",strlen("T6000"),VALUE_NAME,fXMLSendFlag);

	//T6004
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	inGetBLCName(bTmp);
	memcpy(bBuf,bTmp,5);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T6004",bBuf,fXMLSendFlag);

	//T6400
	//if(DongleOut->ucPurseVersionNumber == MIFARE)
	//	vdUnPackToXMLData(DongleOut->ucSID_STAC,sizeof(DongleOut->ucSID_STAC),srXML,(char *)"T6400",strlen("T6400"),VALUE_NAME,fXMLSendFlag);

	//T6406
	/*if(DongleOut->ucPurseVersionNumber != MIFARE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy((char *)bBuf,DongleOut->ucSVCrypto,sizeof(DongleOut->ucSVCrypto));
		vdUnPackToXMLData(bBuf,sizeof(DongleOut->ucSVCrypto),srXML,(char *)"T6406",strlen("T6406"),VALUE_NAME,fXMLSendFlag);
	}*/

	return SUCCESS;
}

int inBuildCMASEDCADeductTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
BYTE bBuf[50];
EDCADeduct_APDU_In *DongleIn = (EDCADeduct_APDU_In *)Dongle_In;
EDCADeduct_APDU_Out *DongleOut = (EDCADeduct_APDU_Out *)Dongle_Out;
EDCARead_APDU_Out *DongleReqOut = (EDCARead_APDU_Out *)Dongle_ReqOut;
BOOL fXMLSendFlag = TRUE;
int inCntStart,inCntEnd;
BYTE bDate[20];

	//TAG_TRANS_XML_HEADER
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_TRANS_XML_HEADER);

	//TAG_TRANS_HEADER
	if(inXMLSearchValueByTag(srXML,(char *)TAG_TRANS_HEADER,&inCntStart,&inCntEnd,0) >= SUCCESS)
	{
		srXML->srXMLElement[inCntStart].fXMLSendFlag = TRUE;
		//vdSetConfigFlag(srXML,FALSE,TAG_TRANS_HEADER);
		memcpy(srXMLData.srXMLElement[inCntStart].chTag,TAG_TRANS_HEADER,strlen(TAG_TRANS_HEADER));
	}

	//T0100
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0100,(BYTE *)"0220",TRUE);

	//T0200
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0200);

	//T0211
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0211);

	//T0213
	vdUnPackToXMLData(&DongleReqOut->ucCardType,sizeof(DongleReqOut->ucCardType),srXML,(char *)TAG_NAME_0213,strlen(TAG_NAME_0213),VALUE_NAME,fXMLSendFlag);

	//T0214
	vdUnPackToXMLData(&DongleReqOut->ucPersonalProfile,sizeof(DongleReqOut->ucPersonalProfile),srXML,(char *)TAG_NAME_0214,strlen(TAG_NAME_0214),VALUE_NAME,fXMLSendFlag);

	//T0300
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"811599",TRUE);

	//T0400
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%ld",srTxnData.srIngData.lnECCAmt);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0400,bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0400,strlen(TAG_NAME_0400),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T0404,社福點數
	//vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0404);
	memset(bBuf,0x00,sizeof(bBuf));
	//sprintf((char *)bBuf,"%d",1);
	//inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0404,bBuf,fXMLSendFlag);
	inXMLGetData(srXML,(char *)TAG_NAME_0404,(char *)bBuf,sizeof(bBuf),0);
	//if(inXMLGetData(srXML,(char *)TAG_NAME_0404,(char *)bBuf,sizeof(bBuf),0) < SUCCESS)
		//log_msg(LOG_LEVEL_FLOW,"inBuildCMSEDCADeductTM 1: NO T0404");
	if(strlen((char *)bBuf) <= 2)//肯定沒有2碼小數
	{
		vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0404);
		log_msg(LOG_LEVEL_FLOW,"inBuildCMSEDCADeductTM  2!!");
	}
	else if(memcmp((char *)&bBuf[strlen((char *)bBuf) - 2],"00",2))//最後2碼不是00
	{
		vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0404);
		log_msg(LOG_LEVEL_FLOW,"inBuildCMSEDCADeductTM 3!!");
	}
	else//最後2碼肯定是00,去掉後2碼
	{
		bBuf[strlen((char *)bBuf) - 2] = 0x00;
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0404,bBuf,fXMLSendFlag);
		log_msg(LOG_LEVEL_FLOW,"inBuildCMSEDCADeductTM 4!!");
	}

	//T0408
	memset(bBuf,0x00,sizeof(bBuf));
	vdIntToAsc(DongleOut->stDeductOut_t.ucEV,sizeof(DongleOut->stDeductOut_t.ucEV),bBuf,5,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0408,bBuf,fXMLSendFlag);
	srTxnData.srIngData.lnECCEV = lnAmt3ByteToLong(DongleOut->stDeductOut_t.ucEV);

	//T0409
	if(strlen((char *)srTxnData.srIngData.anAutoloadAmount) > 2)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy(bBuf,srTxnData.srIngData.anAutoloadAmount,strlen((char *)srTxnData.srIngData.anAutoloadAmount) - 2);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0409,bBuf,fXMLSendFlag);
	}

	//T0410
	memset(bBuf,0x00,sizeof(bBuf));
	vdIntToAsc(DongleReqOut->ucEV,sizeof(DongleReqOut->ucEV),bBuf,5,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0410,bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)TAG_NAME_0410,strlen(TAG_NAME_0410),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T1100
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_1100);

	//T1101
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_1101);

	memset(bDate,0x00,sizeof(bDate));
	UnixToDateTime((BYTE*)DongleOut->stDeductOut_t.ucTxnDateTime,(BYTE*)bDate,14);
	//T1200
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1200,&bDate[8],TRUE);

	//T1201
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T1201",&bDate[8],TRUE);
	bDate[8] = 0x00;
	//vdSetConfigFlag(srXML,FALSE,(char *)"T1201");

	//T1300
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1300,bDate,TRUE);
	//vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_1300);

	//T1301
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T1301",bDate,TRUE);
	//vdSetConfigFlag(srXML,FALSE,(char *)"T1301");

	//T3700
	vdCMASBuildRRNProcess(srXML,srTxnData.srIngData.ulCMASHostSerialNumber,DongleReqOut->ucTxnSN);

	//T4100
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4100);

	//T4101
	//if(DongleReqOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(DongleReqOut->ucDeviceID,sizeof(DongleReqOut->ucDeviceID),srXML,(char *)"T4101",strlen("T4101"),VALUE_NAME,fXMLSendFlag);

	//T4104
	memset(bBuf,0x00,sizeof(bBuf));
	if(DongleReqOut->ucPurseVersionNumber == MIFARE)
	{
		fnUnPack(&DongleOut->stDeductOut_t.ucMAC_HCrypto[6],4,bBuf);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4104",bBuf,fXMLSendFlag);
	}
	else
	{
		vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4104);
		//sprintf((char *)bBuf,"00000000");
	}

	//T4107
	if(strlen(srTxnData.srParameter.chPOS_ID) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4107,(BYTE *)srTxnData.srParameter.chPOS_ID,fXMLSendFlag);

	//T4108 off
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4108);

	//T4117
	if(strlen(srTxnData.srParameter.chPOS_ID2) > 0)
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4117,(BYTE *)srTxnData.srParameter.chPOS_ID2,fXMLSendFlag);

	//T4200
	if(srTxnData.srParameter.chCMASMode == '1' || srTxnData.srParameter.chCMASMode == '2')//銀行併機 or 商店模式
	{
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy(bBuf,&DongleIn->ucTMLocationID[2],8);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,bBuf,fXMLSendFlag);
	}
	else
		vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4200);

	//T4210
	vdSetConfigFlag(srXML,FALSE,(char *)"T4210");

	//T4800
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4800);

	//T4801,33 byte unpack to 66 byte,後補0至90 byte
	vdBuildCMASCardAVR((BYTE *)&DongleReqOut->stLastCreditTxnLogInfo_t,sizeof(DongleReqOut->stLastCreditTxnLogInfo_t),srXML,(char *)"T4801",strlen("T4801"),VALUE_NAME,fXMLSendFlag);

	//T4802
	vdUnPackToXMLData(&DongleReqOut->ucIssuerCode,sizeof(DongleReqOut->ucIssuerCode),srXML,(char *)"T4802",strlen("T4802"),VALUE_NAME,fXMLSendFlag);

	//T4803
	vdUnPackToXMLData(&DongleReqOut->ucBankCode,sizeof(DongleReqOut->ucBankCode),srXML,(char *)TAG_NAME_4803,strlen(TAG_NAME_4803),VALUE_NAME,fXMLSendFlag);

	//T4804
	vdUnPackToXMLData(&DongleReqOut->ucAreaCode,sizeof(DongleReqOut->ucAreaCode),srXML,(char *)TAG_NAME_4804,strlen(TAG_NAME_4804),VALUE_NAME,fXMLSendFlag);

	//T4805
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleReqOut->ucSubAreaCode,sizeof(DongleReqOut->ucSubAreaCode),srXML,(char *)TAG_NAME_4805,strlen(TAG_NAME_4805),VALUE_NAME,fXMLSendFlag);

	//T4808
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->stDeductOut_t.ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4808",bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T4808",strlen("T4808"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),srXML,(char *)"T4808",strlen("T4808"),VALUE_NAME,fXMLSendFlag);

	//T4809
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->stDeductOut_t.ucTxnMode,sizeof(DongleOut->stDeductOut_t.ucTxnMode),srXML,(char *)"T4809",strlen("T4809"),VALUE_NAME,fXMLSendFlag);

	//T4810
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->stDeductOut_t.ucTxnQuqlifier,sizeof(DongleOut->stDeductOut_t.ucTxnQuqlifier),srXML,(char *)"T4810",strlen("T4810"),VALUE_NAME,fXMLSendFlag);

	//T4811
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleReqOut->ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4811",bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T4811",strlen("T4811"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleReqOut->ucTxnSN,sizeof(DongleReqOut->ucTxnSN),srXML,(char *)"T4811",strlen("T4811"),VALUE_NAME,fXMLSendFlag);

	//T4812
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleOut->stDeductOut_t.ucCTC,sizeof(DongleOut->stDeductOut_t.ucCTC),srXML,(char *)"T4812",strlen("T4812"),VALUE_NAME,fXMLSendFlag);

	//T4813
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleReqOut->ucLoyaltyCounter,2,bBuf,6,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4813",bBuf,fXMLSendFlag);
	//inXMLAppendData(srXML,(char *)"T4813",strlen("T4813"),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);
	//vdUnPackToXMLData(DongleReqOut->ucLoyaltyCounter,sizeof(DongleReqOut->ucLoyaltyCounter),srXML,(char *)"T4813",strlen("T4813"),VALUE_NAME,fXMLSendFlag);

	//T4826
	vdSetConfigFlag(srXML,FALSE,(char *)"T4826");

	//T4833
	if((gTmpData.ucReaderFWVersion[0] & 0x0F) > 0x08 || (((gTmpData.ucReaderFWVersion[0] & 0x0F) == 0x08) && (gTmpData.ucReaderFWVersion[1] & 0x0F) >= 0x07))
		vdUnPackToXMLData(&DongleOut->ucTransferGuoupCode,sizeof(DongleOut->ucTransferGuoupCode),srXML,(char *)TAG_NAME_4833,strlen(TAG_NAME_4833),VALUE_NAME,fXMLSendFlag);
	else
		vdUnPackToXMLData(&DongleOut->ucAutoloadMsgType,sizeof(DongleOut->ucAutoloadMsgType),srXML,(char *)TAG_NAME_4833,strlen(TAG_NAME_4833),VALUE_NAME,fXMLSendFlag);

	//T4834
	if((gTmpData.ucReaderFWVersion[0] & 0x0F) > 0x08 || (((gTmpData.ucReaderFWVersion[0] & 0x0F) == 0x08) && (gTmpData.ucReaderFWVersion[1] & 0x0F) >= 0x07))
		vdUnPackToXMLData(DongleOut->ucTransferGuoupCode2,sizeof(DongleOut->ucTransferGuoupCode2),srXML,(char *)TAG_NAME_4834,strlen(TAG_NAME_4834),VALUE_NAME,fXMLSendFlag);
	else
		vdUnPackToXMLData(&DongleOut->ucAutoloadSubType,2,srXML,(char *)TAG_NAME_4834,strlen(TAG_NAME_4834),VALUE_NAME,fXMLSendFlag);

	//T4835
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleIn->ucTransferDiscount,sizeof(DongleIn->ucTransferDiscount),bBuf,8,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4835,bBuf,fXMLSendFlag);
	//vdUnPackToXMLData(DongleIn->ucTransferDiscount,sizeof(DongleIn->ucTransferDiscount),srXML,(char *)TAG_NAME_4835,strlen(TAG_NAME_4835),VALUE_NAME,fXMLSendFlag);

	//T4836
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4836);

	//T4837
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4837);

	//T4838
	//vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4838);

	//T4843
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4843);

	//T4844
	vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4844);

	//T5301
	//vdUnPackToXMLData(&DongleReqOut->ucSAMKVN,sizeof(DongleReqOut->ucSAMKVN),srXML,(char *)"T5301",strlen("T5301"),VALUE_NAME,fXMLSendFlag);

	//T5303
	//if(DongleReqOut->ucPurseVersionNumber != MIFARE)
	vdUnPackToXMLData(&DongleOut->ucHashType,1,srXML,(char *)"T5303",strlen("T5303"),VALUE_NAME,fXMLSendFlag);

	//T5304
	//if(DongleReqOut->ucPurseVersionNumber != MIFARE)
	vdUnPackToXMLData(&DongleOut->ucHostAdminKVN,1,srXML,(char *)"T5304",strlen("T5304"),VALUE_NAME,fXMLSendFlag);

	//T5305
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(&DongleOut->ucSignatureKeyKVN,sizeof(DongleOut->ucSignatureKeyKVN),srXML,(char *)"T5305",strlen("T5305"),VALUE_NAME,fXMLSendFlag);

	//T5361
	//vdUnPackToXMLData(DongleReqOut->ucSAMID,sizeof(DongleReqOut->ucSAMID),srXML,(char *)"T5361",strlen("T5361"),VALUE_NAME,fXMLSendFlag);

	//T5362
	//vdUnPackToXMLData(DongleReqOut->ucSAMSN,sizeof(DongleReqOut->ucSAMSN),srXML,(char *)"T5362",strlen("T5362"),VALUE_NAME,fXMLSendFlag);

	//T5363
	//vdUnPackToXMLData(DongleReqOut->ucSAMCRN,sizeof(DongleReqOut->ucSAMCRN),srXML,(char *)"T5363",strlen("T5363"),VALUE_NAME,fXMLSendFlag);

	//T5371
	vdUnPackToXMLData(DongleOut->ucCPUSAMID,sizeof(DongleOut->ucCPUSAMID),srXML,(char *)"T5371",strlen("T5371"),VALUE_NAME,fXMLSendFlag);

	//T5501
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_5501);

	//T5503
	memset(bBuf,0x00,sizeof(bBuf));
	if (srTxnData.srParameter.chCMASMode == '2')
	{
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T5503",(BYTE *)srTxnData.srREQData.bT5503,TRUE);
	}
	else
		vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_5503);

	//T5504
	vdSetConfigFlag(srXML,FALSE,(char *)"T5504");

	//T5510
	vdSetConfigFlag(srXML,FALSE,(char *)"T5510");

	//T5533
	//vdSetPointDetail(inTxnType,srXML);

	//T6000
	vdUnPackToXMLData(gTmpData.ucReaderFWVersion,sizeof(gTmpData.ucReaderFWVersion),srXML,(char *)"T6000",strlen("T6000"),VALUE_NAME,fXMLSendFlag);

	//T6402
	//if(DongleReqOut->ucPurseVersionNumber == MIFARE)
	//	vdUnPackToXMLData(&DongleOut->stDeductOut_t.ucMAC_HCrypto[10],8,srXML,(char *)"T6402",strlen("T6402"),VALUE_NAME,fXMLSendFlag);

	//T6403
	if(DongleReqOut->ucPurseVersionNumber == MIFARE)
		vdUnPackToXMLData(&DongleOut->stDeductOut_t.ucMAC_HCrypto[2],4,srXML,(char *)"T6403",strlen("T6403"),VALUE_NAME,fXMLSendFlag);

	//T6404
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
	{
		vdUnPackToXMLData(&DongleOut->stDeductOut_t.ucMAC_HCrypto[0],16,srXML,(char *)"T6404",strlen("T6404"),VALUE_NAME,fXMLSendFlag);
		vdUnPackToXMLData(DongleOut->stDeductOut_t.ucSignature,16,srXML,(char *)"T6405",strlen("T6405"),VALUE_NAME,fXMLSendFlag);
	}

	return SUCCESS;
}

int inBuildICERAuthTxnOnlineTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
BYTE bBuf[50],bTmp[50];
//AuthTxnOnline_APDU_In *DongleIn = (AuthTxnOnline_APDU_In *)Dongle_In;
AuthTxnOnline_APDU_Out *DongleOut = (AuthTxnOnline_APDU_Out *)Dongle_Out;
TxnReqOnline_APDU_Out *DongleReqOut = (TxnReqOnline_APDU_Out *)Dongle_ReqOut;
BOOL fXMLSendFlag = TRUE;
BYTE bDate[20];
int inCntStart,inCntEnd;

	//Delete T5548
	if(inXMLSearchValueByTag(srXML,(char *)TAG_NAME_5548,&inCntStart,&inCntEnd,0) >= SUCCESS)
	{
		int inXMLElementCnt;

		inXMLElementCnt = srXML->inXMLElementCnt;
		inXMLDeleteData(srXML,inCntStart,inCntEnd - inCntStart + 1);
		inXMLMoveMultipleData2(srXML,inCntEnd + 1,inCntStart,inXMLElementCnt - inCntEnd);
	}

	//TAG_TRANS_HEADER
	if(inXMLSearchValueByTag(srXML,(char *)"Trans",&inCntStart,&inCntEnd,0) >= SUCCESS)
		memcpy(srXML->srXMLElement[inCntStart].chTag,TAG_TRANS_HEADER,strlen(TAG_TRANS_HEADER));

	if(inXMLSearchValueByTag(srXML,(char *)TAG_TRANS_HEADER,&inCntStart,&inCntEnd,0) >= SUCCESS)
		srXML->srXMLElement[inCntStart].fXMLSendFlag = TRUE;

	vdSetConfigFlag(srXML,FALSE,(char *)"T0205");
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0300);
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0400);
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0407);
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_1100);
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_1400);
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_3700);
	vdSetConfigFlag(srXML,FALSE,(char *)"T3800");
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4100);
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4200);

	//T0100
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0100,(BYTE *)"0220",TRUE);

	//T0200
	vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0200);

	//T0211
	if(DongleReqOut->ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(DongleReqOut->ucPID,sizeof(DongleReqOut->ucPID),srXML,(char *)TAG_NAME_0211,strlen(TAG_NAME_0211),VALUE_NAME,fXMLSendFlag);

	//T0212
	memset(bBuf,0x00,sizeof(bBuf));
	bBuf[0] = '0' + DongleReqOut->bAutoLoad;
	inXMLAppendData(srXML,(char *)TAG_NAME_0212,strlen(TAG_NAME_0212),bBuf,strlen((char *)bBuf),VALUE_NAME,fXMLSendFlag);

	//T0213
	vdUnPackToXMLData(&DongleReqOut->ucCardType,sizeof(DongleReqOut->ucCardType),srXML,(char *)TAG_NAME_0213,strlen(TAG_NAME_0213),VALUE_NAME,fXMLSendFlag);

	//T0214
	vdUnPackToXMLData(&DongleReqOut->ucPersonalProfile,sizeof(DongleReqOut->ucPersonalProfile),srXML,(char *)TAG_NAME_0214,strlen(TAG_NAME_0214),VALUE_NAME,fXMLSendFlag);

	//T0300
	//vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0300);

	//T0400
	//vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_0400);

	//T0410
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),bTmp,5,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,5);
	strcat((char *)bBuf,"00");
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0410,bBuf,fXMLSendFlag);

	//T1100
	//vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_1100);

	memset(bDate,0x00,sizeof(bDate));
	UnixToDateTime((BYTE*)DongleOut->ucTxnDateTime,(BYTE*)bDate,14);
	//T1200
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1200,&bDate[8],TRUE);

	//T1300
	bDate[8] = 0x00;
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1300,bDate,TRUE);

	//T3700
	//vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_3700);

	//T4100
	//vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4100);

	//T4109
	memset(bBuf,0x00,sizeof(bBuf));
	fnBINTODEVASC(DongleReqOut->ucDeviceID,bBuf,9,MIFARE);//n_TXN Device ID
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4109",bBuf,FALSE);

	//T4110
	memset(bBuf,0x00,sizeof(bBuf));
	fnBINTODEVASC(DongleReqOut->ucCPUDeviceID,bBuf,16,LEVEL2);//n_TXN Device ID
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4110",bBuf,FALSE);

	//T4200
	//vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_4200);

	//T4803
	vdUnPackToXMLData(&DongleReqOut->ucBankCode,sizeof(DongleReqOut->ucBankCode),srXML,(char *)TAG_NAME_4803,strlen(TAG_NAME_4803),VALUE_NAME,fXMLSendFlag);

	//T4804
	vdUnPackToXMLData(&DongleReqOut->ucAreaCode,sizeof(DongleReqOut->ucAreaCode),srXML,(char *)TAG_NAME_4804,strlen(TAG_NAME_4804),VALUE_NAME,fXMLSendFlag);

	//T4808
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleOut->ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4808",bBuf,fXMLSendFlag);

	//T4811
	memset(bBuf,0x00,sizeof(bBuf));
	vdUIntToAsc(DongleReqOut->ucTxnSN,3,bBuf,6,TRUE,0x00,10);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T4811",bBuf,fXMLSendFlag);

	//T5362
	vdUnPackToXMLData(DongleReqOut->ucSAMSN,sizeof(DongleReqOut->ucSAMSN),srXML,(char *)"T5362",strlen("T5362"),VALUE_NAME,fXMLSendFlag);

	//T5501
	//inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)srTxnData.srIngData.chTxDate,TRUE);
	//vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_5501);

	//T5503
	//vdSetConfigFlag(srXML,FALSE,(char *)TAG_NAME_5503);

	//T5509
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_5509,(BYTE *)"E",fXMLSendFlag);

	//T5510
	//vdSetConfigFlag(srXML,FALSE,(char *)"T5510");

	//T5524
	//inXMLAppendData(srXML,(char *)"T5524",strlen("T5524"),DongleReqOut->ucTMAgentNumber,4,VALUE_NAME,fXMLSendFlag);

	//T6000
	vdUnPackToXMLData(DongleReqOut->ucReaderFWVersion,sizeof(DongleReqOut->ucReaderFWVersion),srXML,(char *)"T6000",strlen("T6000"),VALUE_NAME,fXMLSendFlag);

	//T554801 + T554808 + T6400 + T6406
	vdBuildICERADataAuth(inTxnType,srXML,Dongle_In,Dongle_ReqOut,Dongle_Out);

	return SUCCESS;
}

