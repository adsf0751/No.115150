
#ifdef EZ_AIRLINKEDC
#include "_stdAfx.h"
#else
#include "stdAfx.h"
#endif
#include "Global.h"

STRUCT_XML_DOC srXMLData;
//STRUCT_XML_ACTION_DATA srXMLActionData;
STRUCT_XML_DOC srBankXMLData;

STRUCT_XML_CHECK_DATA srXMLTMInputCheck[TXN_TOTAL_CNT] = {
/*TXN_QUERY_POINT*/	{{TAG_NAME_0100,TAG_NAME_0300,TAG_NAME_4200,TAG_NAME_5501,
					 TAG_NAME_5503,TAG_NAME_5509,"","",
					 "","","","",
					 "","","","",
					 "","","",""}},
/*TXN_SALE*/		{{TAG_NAME_0100,TAG_NAME_0300,TAG_NAME_0400,TAG_NAME_4200,
					 TAG_NAME_5501,TAG_NAME_5503,TAG_NAME_5509,TAG_NAME_5535,
					 TAG_NAME_5597,"","","",
					 "","","","",
					 "","","",""}},
/*TXN_REFUND*/		{{TAG_NAME_0100,TAG_NAME_0300,TAG_NAME_0400,TAG_NAME_4200,
					 TAG_NAME_5501,TAG_NAME_5503,TAG_NAME_5581,TAG_NAME_5582,
					 TAG_NAME_5583,TAG_NAME_5597,"","",
					 "","","","",
					 "","","",""}},
/*TXN_REDEEM*/		{{TAG_NAME_0100,TAG_NAME_0300,TAG_NAME_4200,TAG_NAME_5501,
					 TAG_NAME_5503,TAG_NAME_5531,TAG_NAME_5535,TAG_NAME_5597,
					 "","","","",
					 "","","","",
					 "","","",""}},
/*TXN_REDEEM_REFUND*/	{{TAG_NAME_0100,TAG_NAME_0300,TAG_NAME_4200,TAG_NAME_5501,
					 TAG_NAME_5503,TAG_NAME_5581,TAG_NAME_5582,TAG_NAME_5583,
					 TAG_NAME_5597,"","","",
					 "","","","",
					 "","","",""}},
/*TXN_ADJECT_CARD*/	{{TAG_NAME_0100,TAG_NAME_0300,TAG_NAME_4200,TAG_NAME_5501,
					 TAG_NAME_5503,"","","",
					 "","","","",
					 "","","","",
					 "","","",""}},
/*TXN_GIFT*/		{{TAG_NAME_0100,TAG_NAME_0300,TAG_NAME_4200,TAG_NAME_5501,
					 TAG_NAME_5503,TAG_NAME_5597,"","",
					 "","","","",
					 "","","","",
					 "","","",""}},
/*TXN_ADD_POINT*/	{{TAG_NAME_0100,TAG_NAME_0300,TAG_NAME_4200,TAG_NAME_5501,
					 TAG_NAME_5503,TAG_NAME_5533,TAG_NAME_5597,"",
					 "","","","",
					 "","","","",
					 "","","",""}},
/*TXN_SETTLE*/		{{TAG_NAME_0100,TAG_NAME_0300,TAG_NAME_4200,TAG_NAME_5501,
					 "","","","",
					 "","","","",
					 "","","","",
					 "","","",""}},
	};


STRUCT_XML_CHECK_DATA srXMLTMOutput[TXN_TOTAL_CNT] = {
/*TXN_QUERY_POINT*/	{{TAG_NAME_0200,TAG_NAME_0404,TAG_NAME_0406,TAG_NAME_0410,
					 TAG_NAME_1100,TAG_NAME_1402,TAG_NAME_3900,TAG_NAME_5532,
					 TAG_NAME_5535,TAG_NAME_5536,TAG_NAME_5572,"",
					 "","","","",
					 "","","",""}},
/*TXN_SALE*/		{{TAG_NAME_0200,TAG_NAME_1100,TAG_NAME_1200,TAG_NAME_1300,
					 TAG_NAME_1402,TAG_NAME_0400,TAG_NAME_0401,TAG_NAME_0403,
					 TAG_NAME_0404,TAG_NAME_0406,TAG_NAME_0410,TAG_NAME_3700,
					 TAG_NAME_3900,TAG_NAME_4100,TAG_NAME_5532,TAG_NAME_5533,
					 TAG_NAME_5535,TAG_NAME_5548,TAG_NAME_5572,TAG_NAME_5574}},
/*TXN_REFUND*/		{{TAG_NAME_0200,TAG_NAME_1100,TAG_NAME_1200,TAG_NAME_1300,
					 TAG_NAME_1402,TAG_NAME_0400,TAG_NAME_0404,TAG_NAME_0407,
					 TAG_NAME_0410,TAG_NAME_3700,TAG_NAME_3900,TAG_NAME_4100,
					 TAG_NAME_5532,TAG_NAME_5534,TAG_NAME_5535,TAG_NAME_5548,
					 "","","","",}},
/*TXN_REDEEM*/		{{TAG_NAME_0200,TAG_NAME_1100,TAG_NAME_1200,TAG_NAME_1300,
					 TAG_NAME_1402,TAG_NAME_0410,TAG_NAME_3700,TAG_NAME_3900,
					 TAG_NAME_4100,TAG_NAME_5532,TAG_NAME_5535,TAG_NAME_5572,
					 TAG_NAME_5574,"","","",
					 "","","","",}},
/*TXN_SETTLE*/		{{TAG_NAME_3900,"","","",
					 "","","","",
					 "","","","",
					 "","","","",
					 "","","","",}},
	};


STRUCT_XML_FORMAT_CHECK srXMLFormatCheck[] = {
	{100,XML_CHECK_NUMERIC},
	{200,XML_CHECK_NUMERIC},
	{205,XML_CHECK_NUMERIC},
	{206,XML_CHECK_NUMERIC_ALPHA},
	{300,XML_CHECK_NUMERIC},
	{400,XML_CHECK_NUMERIC_AMOUNT},
	{401,XML_CHECK_NUMERIC_AMOUNT},
	{402,XML_CHECK_NUMERIC_AMOUNT},
	{403,XML_CHECK_NUMERIC_AMOUNT},
	{404,XML_CHECK_NUMERIC_AMOUNT},
	{405,XML_CHECK_NUMERIC_AMOUNT},
	{407,XML_CHECK_NUMERIC_AMOUNT},
	{409,XML_CHECK_NUMERIC_AMOUNT},
	{410,XML_CHECK_NUMERIC_AMOUNT},
	{411,XML_CHECK_NUMERIC_AMOUNT},
	{412,XML_CHECK_NUMERIC_AMOUNT},
	{414,XML_CHECK_NUMERIC_AMOUNT},
	{416,XML_CHECK_NUMERIC_AMOUNT},
	{417,XML_CHECK_NUMERIC_AMOUNT},
	{418,XML_CHECK_NUMERIC_AMOUNT},
	{421,XML_CHECK_NUMERIC_AMOUNT},
	{422,XML_CHECK_NUMERIC_ALPHA},
	{423,XML_CHECK_NUMERIC_ALPHA},
	{442,XML_CHECK_NUMERIC_AMOUNT},
	{443,XML_CHECK_NUMERIC_AMOUNT},
	{444,XML_CHECK_NUMERIC_AMOUNT},
	{445,XML_CHECK_NUMERIC_AMOUNT},
	{446,XML_CHECK_NUMERIC_AMOUNT},
	{1100,XML_CHECK_NUMERIC},
	{1101,XML_CHECK_NUMERIC},
	{1102,XML_CHECK_NUMERIC},
	{1103,XML_CHECK_NUMERIC},
	{1104,XML_CHECK_NUMERIC},
	{1200,XML_CHECK_NUMERIC_TIME},
	{1202,XML_CHECK_NUMERIC_TIME},
	{1300,XML_CHECK_NUMERIC_DATE},
	{1302,XML_CHECK_NUMERIC_DATE},
	{1400,XML_CHECK_NUMERIC},
	{1402,XML_CHECK_NUMERIC},
	{1403,XML_CHECK_NUMERIC},
	{3700,XML_CHECK_NUMERIC},
	{3701,XML_CHECK_NUMERIC_ALPHA},
	{3800,XML_CHECK_NUMERIC_ALPHA},
	{3900,XML_CHECK_NUMERIC},
	{4100,XML_CHECK_NUMERIC},
	{4103,XML_CHECK_NUMERIC_ALPHA},
	{4105,XML_CHECK_NUMERIC_ALPHA},
	{4107,XML_CHECK_NUMERIC_ALPHA},
	{4108,XML_CHECK_NUMERIC},
	{4117,XML_CHECK_NUMERIC_ALPHA},
	{4200,XML_CHECK_NUMERIC_ALPHA},
	{4214,XML_CHECK_NUMERIC_ALPHA},
	{4830,XML_CHECK_NUMERIC},
	{4835,XML_CHECK_NUMERIC_AMOUNT},
	{4836,XML_CHECK_NUMERIC_ALPHA},
	{4837,XML_CHECK_NUMERIC_ALPHA},
	//{4838,XML_CHECK_NUMERIC_AMOUNT},
	//{4839,XML_CHECK_NUMERIC},
	{483100,XML_CHECK_NUMERIC},
	{483101,XML_CHECK_NUMERIC},
	{483102,XML_CHECK_NUMERIC},
	{483103,XML_CHECK_NUMERIC},
	{483104,XML_CHECK_NUMERIC},
	{483105,XML_CHECK_NUMERIC},
	{483106,XML_CHECK_NUMERIC},
	{483107,XML_CHECK_NUMERIC},
	{483108,XML_CHECK_NUMERIC},
	{483109,XML_CHECK_NUMERIC},
	{483110,XML_CHECK_NUMERIC},
	{483111,XML_CHECK_NUMERIC},
	{484200,XML_CHECK_NUMERIC},
	{484201,XML_CHECK_NUMERIC_DATE_TIME},
	{484202,XML_CHECK_NUMERIC},
	{484203,XML_CHECK_NUMERIC_ALPHA},
	{4832,XML_CHECK_NUMERIC},
	{4843,XML_CHECK_NUMERIC_DATE},
	{484600,XML_CHECK_NUMERIC_ALPHA},
	{484601,XML_CHECK_NUMERIC_AMOUNT},
	{484602,XML_CHECK_NUMERIC},
	{484603,XML_CHECK_NUMERIC_AMOUNT},
	{484604,XML_CHECK_NUMERIC_ALPHA},
	{4847,XML_CHECK_NUMERIC},
	{4848,XML_CHECK_NUMERIC},
	{4849,XML_CHECK_NUMERIC},
	{4900,XML_CHECK_NUMERIC},
	{4901,XML_CHECK_NUMERIC},
	{4902,XML_CHECK_NUMERIC},
	{4903,XML_CHECK_NUMERIC},
	{4904,XML_CHECK_NUMERIC},
	{4905,XML_CHECK_NUMERIC_DATE},
	{4906,XML_CHECK_NUMERIC_DATE},
	{5000,XML_CHECK_NUMERIC_ALPHA},
	{5001,XML_CHECK_NUMERIC_ALPHA},
	{5002,XML_CHECK_NUMERIC_ALPHA},
	{5003,XML_CHECK_NUMERIC_ALPHA},
	{5501,XML_CHECK_NUMERIC},
	{5503,XML_CHECK_NUMERIC_ALPHA},
	{5509,XML_CHECK_NUMERIC_ALPHA},
	{5531,XML_CHECK_NUMERIC},
	{553301,XML_CHECK_NUMERIC_ALPHA},
	{553302,XML_CHECK_NUMERIC_DATE},
	{553303,XML_CHECK_NUMERIC_DATE},
	{553304,XML_CHECK_NUMERIC},
	{553305,XML_CHECK_NUMERIC},
	{553306,XML_CHECK_NUMERIC},
	{553307,XML_CHECK_NUMERIC_ALPHA},
	{553308,XML_CHECK_NUMERIC},
	{553309,XML_CHECK_NUMERIC},
	{553310,XML_CHECK_NUMERIC},
	{553311,XML_CHECK_NUMERIC_DATE},
	{5535,XML_CHECK_NUMERIC},
	{554809,XML_CHECK_NUMERIC_ALPHA},
	{554810,XML_CHECK_NUMERIC_ALPHA},  
	{557901,XML_CHECK_NUMERIC_ALPHA},
	{557902,XML_CHECK_NUMERIC_ALPHA},
	{557903,XML_CHECK_NUMERIC_AMOUNT},
	{557904,XML_CHECK_NUMERIC_ALPHA},
	{557905,XML_CHECK_NUMERIC_ALPHA},
	{557906,XML_CHECK_NUMERIC_ALPHA},
	{557907,XML_CHECK_NUMERIC_ALPHA},
	{557908,XML_CHECK_NUMERIC_ALPHA},
	{557909,XML_CHECK_NUMERIC_DATE_TIME},
	{557910,XML_CHECK_NUMERIC_ALPHA},
	{557911,XML_CHECK_NUMERIC_AMOUNT},
	{5581,XML_CHECK_NUMERIC_ALPHA},
	{5582,XML_CHECK_NUMERIC},
	{5583,XML_CHECK_NUMERIC_DATE},
	{5591,XML_CHECK_NUMERIC},
	{5592,XML_CHECK_NUMERIC},
	{559201,XML_CHECK_NUMERIC},
	{5593,XML_CHECK_NUMERIC},
	{5594,XML_CHECK_NUMERIC},
	{5595,XML_CHECK_NUMERIC_ALPHA},
	{5596,XML_CHECK_NUMERIC_ALPHA},
	{5597,XML_CHECK_NUMERIC_ALPHA},
	{5598,XML_CHECK_NUMERIC},
	{5599,XML_CHECK_NUMERIC_ALPHA},
	{6401,XML_CHECK_NUMERIC_ALPHA},
	{6409,XML_CHECK_NUMERIC_ALPHA},

	};

void vdSetXMLActionData(STRUCT_XML_ACTION_DATA *srXMLAction,char *chXMLName,char *chValue,BOOL fInitial,int inNodeNoType,int inNodeType,BOOL fXMLSendFlag)
{

	if(fInitial == TRUE)
		memset(srXMLAction,0x00,sizeof(STRUCT_XML_ACTION_DATA));

	srXMLAction->srXMLDetail[srXMLAction->inParseCnt].inNodeNoType = inNodeNoType;
	srXMLAction->srXMLDetail[srXMLAction->inParseCnt].fXMLSendFlag = fXMLSendFlag;
	switch(inNodeType)
	{
		case TAG_NAME_START:
			memcpy(srXMLAction->srXMLDetail[srXMLAction->inParseCnt].chTagName,chXMLName,strlen(chXMLName));
			srXMLAction->srXMLDetail[srXMLAction->inParseCnt].fNodeFlag = TRUE;
			break;
		case VALUE_NAME:
			memcpy(srXMLAction->srXMLDetail[srXMLAction->inParseCnt].chTagName,chXMLName,strlen(chXMLName));
			srXMLAction->srXMLDetail[srXMLAction->inParseCnt].chValue = chValue;
			break;
		case TAG_NAME_END:
			srXMLAction->srXMLDetail[srXMLAction->inParseCnt].chTagName[0] = '/';
			memcpy(&srXMLAction->srXMLDetail[srXMLAction->inParseCnt].chTagName[1],chXMLName,strlen(chXMLName));
			break;
	}
	srXMLAction->inParseCnt++;
}

int inParseXML(char *bFileName,BYTE *bXMLInData,BOOL fParseByFile,int inXMLInLen,STRUCT_XML_DOC *srXML,int inFileOffset)
{
int inRetVal,inCnt = 0,inNodeNumber = 0;
BYTE bXMLFileData[MAX_XML_FILE_SIZE];
int inXMLFileSize;

	if(fParseByFile == TRUE)
	{
		inXMLFileSize = inFileGetSize(bFileName,FALSE);
		if(inXMLFileSize > MAX_XML_FILE_SIZE || inXMLFileSize < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inInitialSystemXML (%s) Fail 1:%d",bFileName,inXMLFileSize);
			return ICER_ERROR;
			//return XML_INITIAL_FAIL;
		}

		inRetVal = inFileRead(bFileName,bXMLFileData,sizeof(bXMLFileData),inXMLFileSize);
		if(inRetVal < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inInitialSystemXML (%s) Fail 2:%d",bFileName,inRetVal);
			return ICER_ERROR;
			//return XML_READ_FAIL;
		}
	}
	else
	{
		memset(bXMLFileData,0x00,sizeof(bXMLFileData));
		inXMLFileSize = inXMLInLen;
		memcpy(bXMLFileData,bXMLInData,inXMLFileSize);
	}

	memset(srXML,0x00,sizeof(STRUCT_XML_DOC));
	do
	{
		inRetVal = inGetTag(bXMLFileData,inCnt,inXMLFileSize,srXML,inNodeNumber);
		if(inRetVal < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inParseXML Fail 1");
			return ICER_ERROR;
			//return XML_PARSE_FAIL;
		}
		inCnt += inRetVal;

		inRetVal = inSkipLine(bXMLFileData,inCnt,inXMLFileSize);
		if(inRetVal < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inParseXML Fail 2");
			return ICER_ERROR;
			//return XML_PARSE_FAIL;
		}
		inCnt += inRetVal;

		if(srXML->srXMLElement[srXML->inXMLElementCnt - 1].chTag[0] == '?')
		{
		}
		else if(srXML->srXMLElement[srXML->inXMLElementCnt - 1].chTag[0] == '/')
		{
			inNodeNumber--;
			srXML->srXMLElement[srXML->inXMLElementCnt - 1].inNodeNumber--;
		}
		else if(bXMLFileData[inCnt] == '<')
		{
			inNodeNumber++;
			srXML->srXMLElement[srXML->inXMLElementCnt - 1].fNodeFlag = TRUE;//有子節點
		}
		else
		{
			inRetVal = inGetValue(bXMLFileData,inCnt,inXMLFileSize,srXML);
			if(inRetVal < SUCCESS)
			{
				log_msg(LOG_LEVEL_ERROR,"inParseXML Fail 3");
				return ICER_ERROR;
				//return XML_PARSE_FAIL;
			}
			inCnt += inRetVal;

			inRetVal = inGetEndTag(bXMLFileData,inCnt,inXMLFileSize,srXML);
			if(inRetVal < SUCCESS)
			{
				log_msg(LOG_LEVEL_ERROR,"inParseXML Fail 4");
				return ICER_ERROR;
				//return XML_PARSE_FAIL;
			}
			inCnt += inRetVal;

			inRetVal = inSkipLine(bXMLFileData,inCnt,inXMLFileSize);
			if(inRetVal < SUCCESS)
			{
				log_msg(LOG_LEVEL_ERROR,"inParseXML Fail 2");
				return ICER_ERROR;
				//return XML_PARSE_FAIL;
			}
			inCnt += inRetVal;
		}
	}
	while(inCnt < inXMLFileSize);

	return SUCCESS;

}

////////////////////////解析XML文件////////////////////////////////////
int inInitialSystemXML(char *bFileName)
{
int i,inRetVal;

	inRetVal = inParseXML(bFileName,(BYTE *)NULL,TRUE,0,&srXMLData,0);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inInitialSystemXML (%s) Fail 3:%d",bFileName,inRetVal);
		return ICER_PAR_INI_FAIL;
	}

	srTxnData.srParameter.bSlot[0] = srTxnData.srParameter.bSlot[1] = '0';
	srTxnData.srParameter.chOnlineFlag = ICER_ONLINE;

	for(i=0;i<srXMLData.inXMLElementCnt;i++)
	{
		if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"LogFlag",strlen("LogFlag")))
			srTxnData.srParameter.gLogFlag = atoi(srXMLData.srXMLElement[i].chValue);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"DLLVersion",strlen("DLLVersion")))
			srTxnData.srParameter.gDLLVersion = atoi(srXMLData.srXMLElement[i].chValue);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"TCPIPTimeOut",strlen("TCPIPTimeOut")))
			srTxnData.srParameter.gTCPIPTimeOut = atoi(srXMLData.srXMLElement[i].chValue);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"LogCnt",strlen("LogCnt")))	//原來這兩行是Mark的，但如此Log只能固定30天 v4006C版取消Mark
			srTxnData.srParameter.gLogCnt = atoi(srXMLData.srXMLElement[i].chValue);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"ComPort2",strlen("ComPort2")))
			memcpy(srTxnData.srParameter.bComPort,srXMLData.srXMLElement[i].chValue,srXMLData.srXMLElement[i].shValueLen);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"ComPort",strlen("ComPort")))
			srTxnData.srParameter.gComPort = atoi(srXMLData.srXMLElement[i].chValue);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"BaudRate",strlen("BaudRate")))
			srTxnData.srParameter.ulBaudRate = atoi(srXMLData.srXMLElement[i].chValue);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"ECC_IP",strlen("ECC_IP")))
			memcpy(srTxnData.srParameter.bECCIP,srXMLData.srXMLElement[i].chValue,srXMLData.srXMLElement[i].shValueLen);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"ECC_Port",strlen("ECC_Port")))
			srTxnData.srParameter.usECCPort = atoi(srXMLData.srXMLElement[i].chValue);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"ICER_IP",strlen("ICER_IP")))
			memcpy(srTxnData.srParameter.bICERIP,srXMLData.srXMLElement[i].chValue,srXMLData.srXMLElement[i].shValueLen);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"ICER_Port",strlen("ICER_Port")))
			srTxnData.srParameter.usICERPort = atoi(srXMLData.srXMLElement[i].chValue);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"CMAS_IP",strlen("CMAS_IP")))
			memcpy(srTxnData.srParameter.bCMASIP,srXMLData.srXMLElement[i].chValue,srXMLData.srXMLElement[i].shValueLen);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"CMAS_Port",strlen("CMAS_Port")))
			srTxnData.srParameter.usCMASPort = atoi(srXMLData.srXMLElement[i].chValue);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"TMLocationID",strlen("TMLocationID")))
		{
			if(srTxnData.srParameter.gDLLVersion == 1)
				vdPad(srXMLData.srXMLElement[i].chValue,srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMLocationID,sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMLocationID) - 1,FALSE,0x30);
			else //if(srTxnData.gDLLVersion == 2)
				vdPad(srXMLData.srXMLElement[i].chValue,srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID,sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID) - 1,FALSE,0x30);
		}
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"TMID",strlen("TMID")))
		{
			if(srTxnData.srParameter.gDLLVersion == 1)
				//vdPad(srXMLData.srXMLElement[i].chValue,srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMID,sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMID) - 1,FALSE,0x20);
				vdPad(srXMLData.srXMLElement[i].chValue,srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMID,sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMID) - 1,FALSE,0x30);
			else //if(srTxnData.gDLLVersion == 2)
				//vdPad(srXMLData.srXMLElement[i].chValue,srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMID,sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMID) - 1,FALSE,0x20);
				vdPad(srXMLData.srXMLElement[i].chValue,srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMID,sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMID) - 1,FALSE,0x30);
		}
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"TMAgentNumber",strlen("TMAgentNumber")))
		{
			if(srTxnData.srParameter.gDLLVersion == 1)
				//vdPad(srXMLData.srXMLElement[i].chValue,srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMAgentNumber,sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMAgentNumber) - 1,FALSE,0x20);
				vdPad(srXMLData.srXMLElement[i].chValue,srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMAgentNumber,sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMAgentNumber) - 1,FALSE,0x30);
			else //if(srTxnData.gDLLVersion == 2)
				//vdPad(srXMLData.srXMLElement[i].chValue,srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber,sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber) - 1,FALSE,0x20);
				vdPad(srXMLData.srXMLElement[i].chValue,srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber,sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber) - 1,FALSE,0x30);
		}
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"LocationID",strlen("LocationID")))
		{
			if(srTxnData.srParameter.gDLLVersion == 2)
				vdPad(srXMLData.srXMLElement[i].chValue,srTxnData.srParameter.APIDefaultData.srAPIInData2.chLocationID,sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chLocationID) - 1,FALSE,0x20);
		}
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"NewLocationID",strlen("NewLocationID")))
		{
			if(srTxnData.srParameter.gDLLVersion == 2)
				vdPad(srXMLData.srXMLElement[i].chValue,srTxnData.srParameter.APIDefaultData.srAPIInData2.chNewLocationID,sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chNewLocationID) - 1,FALSE,0x20);
		}
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"SPID",strlen("SPID")))
		{
			if(srTxnData.srParameter.gDLLVersion == 2)
				vdPad(srXMLData.srXMLElement[i].chValue,srTxnData.srParameter.APIDefaultData.srAPIInData2.chSPID,sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chSPID) - 1,FALSE,0x20);
		}
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"NewSPID",strlen("NewSPID")))
		{
			if(srTxnData.srParameter.gDLLVersion == 2)
				vdPad(srXMLData.srXMLElement[i].chValue,srTxnData.srParameter.APIDefaultData.srAPIInData2.chNewSPID,sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chNewSPID) - 1,FALSE,0x20);
		}
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"Debug_Mode",strlen("Debug_Mode")))
			srTxnData.srParameter.chDebugMode = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"Slot",strlen("Slot")))
			vdPad(srXMLData.srXMLElement[i].chValue,(char *)srTxnData.srParameter.bSlot,sizeof(srTxnData.srParameter.bSlot) - 1,FALSE,0x20);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"OpenCom",strlen("OpenCom")))
			srTxnData.srParameter.chOpenComMode[0] = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"MustSettleDate",strlen("MustSettleDate")))
			srTxnData.srParameter.inMustSettleDate = atoi(srXMLData.srXMLElement[i].chValue);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"ReaderMode",strlen("ReaderMode")))
			srTxnData.srParameter.gReaderMode = atoi(srXMLData.srXMLElement[i].chValue);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"OnlineFlag",strlen("OnlineFlag")))
			srTxnData.srParameter.chOnlineFlag = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"BatchFlag",strlen("BatchFlag")))
			srTxnData.srParameter.chBatchFlag = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"ICERDataFlag",strlen("ICERDataFlag")))
			srTxnData.srParameter.chICERDataFlag = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"MessageHeader",strlen("MessageHeader")))
			vdPad(srXMLData.srXMLElement[i].chValue,(char *)srTxnData.srParameter.bMsgHeader,sizeof(srTxnData.srParameter.bMsgHeader) - 1,FALSE,0x20);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"DLLMode",strlen("DLLMode")))
			srTxnData.srParameter.gDLLMode = atoi(srXMLData.srXMLElement[i].chValue);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"MaxALAmt",strlen("MaxALAmt")))
			srTxnData.srParameter.ulMaxALAmt = atoi(srXMLData.srXMLElement[i].chValue);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"AutoLoadMode",strlen("AutoLoadMode")))
			srTxnData.srParameter.chAutoLoadMode = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"Dev_Info",strlen("Dev_Info")))
			memcpy(srTxnData.srParameter.bDeviceInfo,srXMLData.srXMLElement[i].chValue,srXMLData.srXMLElement[i].shValueLen);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"GroupFlag",strlen("GroupFlag")))
			srTxnData.srParameter.chGroupFlag = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"TCPIP_SSL",strlen("TCPIP_SSL")))
			srTxnData.srParameter.chTCPIP_SSL = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"CMASAdviceVerify",strlen("CMASAdviceVerify")))
			srTxnData.srParameter.chCMASAdviceVerify = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"BLCName",strlen("BLCName")))
			vdPad(srXMLData.srXMLElement[i].chValue,(char *)&srTxnData.srParameter.bBLCName,sizeof(srTxnData.srParameter.bBLCName) - 1,TRUE,0x00);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"EchoTest",strlen("EchoTest")))
			srTxnData.srParameter.chEchoTest = srXMLData.srXMLElement[i].chValue[0];
		//else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"AutoLoadFunction",strlen("AutoLoadFunction")))
		//	srTxnData.srParameter.chAutoLoadFunction = srXMLData.srXMLElement[i].chValue[0];
		//else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"ReSendReaderAVR",strlen("ReSendReaderAVR")))
		//	srTxnData.srParameter.chReSendReaderAVR = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"XMLHeaderFlag",strlen("XMLHeaderFlag")))
			srTxnData.srParameter.chXMLHeaderFlag = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"FolderCreatFlag",strlen("FolderCreatFlag")))
			srTxnData.srParameter.chFolderCreatFlag = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"CMASMode",strlen("CMASMode")))
			srTxnData.srParameter.chCMASMode = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"POS_ID",strlen("POS_ID")))
			if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"POS_ID2",strlen("POS_ID2")))
				memcpy(srTxnData.srParameter.chPOS_ID2,srXMLData.srXMLElement[i].chValue,srXMLData.srXMLElement[i].shValueLen);
			else
				memcpy(srTxnData.srParameter.chPOS_ID,srXMLData.srXMLElement[i].chValue,srXMLData.srXMLElement[i].shValueLen);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"AdditionalTcpipData",strlen("AdditionalTcpipData")))
			srTxnData.srParameter.inAdditionalTcpipData = atoi(srXMLData.srXMLElement[i].chValue);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"PacketLenFlag",strlen("PacketLenFlag")))
			srTxnData.srParameter.PacketLenFlag = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"CRT_FileName",strlen("CRT_FileName")))
			memcpy(srTxnData.srParameter.bCRT_FileName,srXMLData.srXMLElement[i].chValue,srXMLData.srXMLElement[i].shValueLen);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"Key_FileName",strlen("Key_FileName")))
			memcpy(srTxnData.srParameter.bKey_FileName,srXMLData.srXMLElement[i].chValue,srXMLData.srXMLElement[i].shValueLen);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"ICERFlowDebug",strlen("ICERFlowDebug")))
			srTxnData.srParameter.ICERFlowDebug = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"BLCFlag",strlen("BLCFlag")))
			srTxnData.srParameter.chBLCFlag = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"AdviceFlag",strlen("AdviceFlag")))
			srTxnData.srParameter.chAdviceFlag = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"ReaderPortocol",strlen("ReaderPortocol")))
			srTxnData.srParameter.chReaderPortocol = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"AccFreeRidesMode",strlen("AccFreeRidesMode")))
			srTxnData.srParameter.chAccFreeRidesMode = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"ETxnSignOnMode",strlen("ETxnSignOnMode")))
			srTxnData.srParameter.chETxnSignOnMode = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"CloseAntenna",strlen("CloseAntenna")))
			srTxnData.srParameter.chCloseAntenna = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"ReaderUartDebug",strlen("ReaderUartDebug")))
			srTxnData.srParameter.chReaderUartDebug = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"MerchantID",strlen("MerchantID")))
			memcpy(srTxnData.srParameter.bMerchantID,srXMLData.srXMLElement[i].chValue,srXMLData.srXMLElement[i].shValueLen);
		//else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"ICERQRTxn",strlen("ICERQRTxn")))
		//	srTxnData.srParameter.chICERQRTxn = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"ICERKey",strlen("ICERKey")))
			memcpy(srTxnData.srParameter.bICERKey,srXMLData.srXMLElement[i].chValue,srXMLData.srXMLElement[i].shValueLen);
		/*else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"OneDayQuotaFlag",strlen("OneDayQuotaFlag")))
			srTxnData.srParameter.chOneDayQuotaFlag = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"OnceQuotaFlag",strlen("OnceQuotaFlag")))
			srTxnData.srParameter.chOnceQuotaFlag = srXMLData.srXMLElement[i].chValue[0];*/
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"RS232Parameter",strlen("RS232Parameter")))
			memcpy(srTxnData.srParameter.bRS232Parameter,srXMLData.srXMLElement[i].chValue,srXMLData.srXMLElement[i].shValueLen);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"SignOnMode",strlen("SignOnMode")))
			srTxnData.srParameter.chSignOnMode = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"ReadAccPointsFlag",strlen("ReadAccPointsFlag")))
			srTxnData.srParameter.chReadAccPointsFlag = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"ReadAccPointsMode",strlen("ReadAccPointsMode")))
			srTxnData.srParameter.chReadAccPointsMode = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"ReadDateOfFirstTransFlag",strlen("ReadDateOfFirstTransFlag")))
			srTxnData.srParameter.chReadDateOfFirstTransFlag = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"CommandMode",strlen("CommandMode")))
			srTxnData.srParameter.chCommandMode = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"ReadBasicVersion",strlen("ReadBasicVersion")))
			srTxnData.srParameter.chReadBasicVersion = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"MultiDeductFlag",strlen("MultiDeductFlag")))
			srTxnData.srParameter.chMultiDeductFlag = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"BatchDataFlag",strlen("BatchDataFlag")))
			srTxnData.srParameter.chBatchDataFlag = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"NegativeZeroDeduct",strlen("NegativeZeroDeduct")))
			srTxnData.srParameter.chNegativeZeroDeduct = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"ETxnBatchFlag",strlen("ETxnBatchFlag")))
			srTxnData.srParameter.chETxnBatchFlag = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"BasicCheckQuotaFlag",strlen("BasicCheckQuotaFlag")))
			srTxnData.srParameter.chBasicCheckQuotaFlag = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"CRPDataReq",strlen("CRPDataReq")))
			memcpy(srTxnData.srParameter.bCRPDataReq,srXMLData.srXMLElement[i].chValue,srXMLData.srXMLElement[i].shValueLen);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"CRPDataRes",strlen("CRPDataRes")))
			memcpy(srTxnData.srParameter.bCRPDataRes,srXMLData.srXMLElement[i].chValue,srXMLData.srXMLElement[i].shValueLen);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"AVM",strlen("AVM")))
			srTxnData.srParameter.chAVM = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"CRPDelay",strlen("CRPDelay")))
			srTxnData.srParameter.gCRPDelay = atoi(srXMLData.srXMLElement[i].chValue);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag, (char *)"ICERConnectMode", strlen("ICERConnectMode")))
			srTxnData.srParameter.gICERConnMode = atoi(srXMLData.srXMLElement[i].chValue);
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"SaveReSendDataForReadBasic",strlen("SaveReSendDataForReadBasic")))
			srTxnData.srParameter.chSaveReSendDataForReadBasic = srXMLData.srXMLElement[i].chValue[0];
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"SetCardValidDate",strlen("SetCardValidDate")))
			srTxnData.srParameter.inSetCardValidDate= atoi(srXMLData.srXMLElement[i].chValue);	
		else if(!ECC_strncasecmp(srXMLData.srXMLElement[i].chTag,(char *)"UsbToRs232Chip",strlen("UsbToRs232Chip")))
			srTxnData.srParameter.gUsbToRs232Chip = atoi(srXMLData.srXMLElement[i].chValue);
	}

	//源裕需求!!
	if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
	{
		if(srTxnData.srParameter.chCMASMode == '1')
			memcpy(srTxnData.srParameter.APIDefaultData.srAPIInData2.chNewSPID,&srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID[2],sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chNewSPID) - 1);
		else if(srTxnData.srParameter.chCMASMode == '2')
			memset(srTxnData.srParameter.APIDefaultData.srAPIInData2.chNewSPID,0x00,sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chNewSPID) - 1);
	}

	vdFreeXMLDOC(&srXMLData);

	return SUCCESS;
}

int inGetTag(BYTE *bInData,int inCnt,int inFileSize,STRUCT_XML_DOC *srXML,int inNodeNumber)
{
int i,j = 0;
int inTagStart = 0,inTagLen = 0;
BOOL fTagStart = FALSE;

	for(i = inCnt;i < inFileSize;i++)
	{
		switch(bInData[i])
		{
			case '<':
				inTagStart = i + 1;
				fTagStart = TRUE;
				break;
			case '>':
				srXML->srXMLElement[srXML->inXMLElementCnt].shTagLen = inTagLen;
				srXML->srXMLElement[srXML->inXMLElementCnt].chTag = (char *)ECC_calloc(1,inTagLen + 1);
				memset(srXML->srXMLElement[srXML->inXMLElementCnt].chTag,0x00,inTagLen + 1);
				memcpy(srXML->srXMLElement[srXML->inXMLElementCnt].chTag,&bInData[inTagStart],inTagLen);
				srXML->srXMLElement[srXML->inXMLElementCnt].chTag[inTagLen] = 0x00;
				srXML->srXMLElement[srXML->inXMLElementCnt].inNodeNumber = inNodeNumber;
				srXML->inXMLElementCnt++;
				return inTagLen + 2 + j;//需加上 "<" & ">"
			default:
				if(fTagStart == TRUE)
					inTagLen++;
				else
					j++;
				break;
		}
	}

	log_msg(LOG_LEVEL_ERROR,"inGetTag Fail '<' in (%d),No '>'",inTagStart - 1);
	return ICER_ERROR;
	//return XML_PARSE_FAIL;
}

int inGetValue(BYTE *bInData,int inCnt,int inFileSize,STRUCT_XML_DOC *srXML)
{
int i;
int inValueLen = 0,j = 0;
BYTE bBuf[500];

	memset(bBuf,0x00,sizeof(bBuf));
	for(i = inCnt;i < inFileSize;i++)
	{
		if(bInData[i] >= 0x80)
		{
			memcpy(&bBuf[j],(char *)&bInData[i],3);
			j += 3;
			inValueLen += 3;
			i += 2;//Continue後會再加1,所以這裡加2就好
			continue;
		}

		switch(bInData[i])
		{
			case '<':
				if(inValueLen > 0)
				{
					srXML->srXMLElement[srXML->inXMLElementCnt - 1].shValueLen = inValueLen;
					srXML->srXMLElement[srXML->inXMLElementCnt - 1].chValue = (char *)ECC_calloc(1,inValueLen + 1);
					memset(srXML->srXMLElement[srXML->inXMLElementCnt - 1].chValue,0x00,inValueLen + 1);
					memcpy(srXML->srXMLElement[srXML->inXMLElementCnt - 1].chValue,&bInData[inCnt],inValueLen);
					srXML->srXMLElement[srXML->inXMLElementCnt - 1].chValue[inValueLen] = 0x00;
				}
				return inValueLen;
			default:
				inValueLen++;
				break;
		}
	}

	log_msg(LOG_LEVEL_ERROR,"inGetValue Fail !!");
	return ICER_ERROR;
	//return XML_PARSE_FAIL;
}

int inSkipLine(BYTE *bInData,int inCnt,int inFileSize)
{
int i,inCRLen = 0;

	for(i = inCnt;i < inFileSize;i++)
	{
		if((bInData[i] == 0x0D && bInData[i + 1] == 0x0A) || (bInData[i] == 0x0A && bInData[i + 1] == 0x0D))
		//if(bInData[i] == 0x0D && bInData[i + 1] == 0x0A)
		{
			inCRLen += 2;
			i++;
		}
		else if(bInData[i] <= 0x20/* || bInData[i] >= 0x7E*/)
			inCRLen++;
		else
			return inCRLen;
	}

	//log_msg(LOG_LEVEL_ERROR,"inSkipLine Fail !!");
	return inCRLen;
}

int inGetEndTag(BYTE *bInData,int inCnt,int inFileSize,STRUCT_XML_DOC *srXML)
{
int i;
int inTagStart = 0,inTagLen = 0;
BOOL fTagStart = FALSE;

	for(i = inCnt;i < inFileSize;i++)
	{
		switch(bInData[i])
		{
			case '<':
				inTagStart = i + 1;
				fTagStart = TRUE;
				break;
			case '>':
				return inTagLen + 2;//需加上 "<" & ">"
			default:
				if(fTagStart == TRUE)
					inTagLen++;
				break;
		}
	}

	log_msg(LOG_LEVEL_ERROR,"inGetEndTag Fail '<' in (%d),No '>'",inTagStart - 1);
	return ICER_ERROR;
	//return XML_PARSE_FAIL;
}

void vdFreeXMLDOC(STRUCT_XML_DOC *srXML)
{
int i;

	//log_msg(LOG_LEVEL_ERROR,"vdFreeXMLDOC 1");
	for(i=0;i<srXML->inXMLElementCnt;i++)
	{
		if(srXML->srXMLElement[i].chTag)
		{
			ECC_free((BYTE *)srXML->srXMLElement[i].chTag);
			srXML->srXMLElement[i].chTag = 0;
		}
		if(srXML->srXMLElement[i].chValue)
		{
			ECC_free((BYTE *)srXML->srXMLElement[i].chValue);
			srXML->srXMLElement[i].chValue = 0;
		}
	}
	//log_msg(LOG_LEVEL_ERROR,"vdFreeXMLDOC 2");
	srXML->inXMLElementCnt = 0;
}

int inXMLGetParseData(STRUCT_XML_ACTION_DATA *srXMLAction,STRUCT_XML_DOC *srXML)
{
	int i,j,inOK = 0;

	for(i=0;i<srXML->inXMLElementCnt;i++)
	{
		for(j=0;j<srXMLAction->inParseCnt;j++)
		{
			if(!ECC_strncasecmp(srXMLAction->srXMLDetail[j].chTagName,srXML->srXMLElement[i].chTag,strlen(srXMLAction->srXMLDetail[j].chTagName)))
			{
				if(strlen(srXMLAction->srXMLDetail[j].chTagName) == strlen(srXML->srXMLElement[i].chTag))
				{
					srXMLAction->srXMLDetail[j].chValue = srXML->srXMLElement[i].chValue;
					srXMLAction->srXMLDetail[j].fOK = TRUE;
					inOK++;
					if(inOK == srXMLAction->inParseCnt)
						return inOK;
					break;
				}
			}
		}
	}

	return inOK;
}

int inXMLGetData(STRUCT_XML_DOC *srXML,char *chGetName,char *chValue,int inValueLen,int inStartIndex)
{
int i;

	for(i=inStartIndex;i<srXML->inXMLElementCnt;i++)
	{
		if(!ECC_strncasecmp(chGetName,srXML->srXMLElement[i].chTag,strlen(chGetName)))
		{
			if(strlen(chGetName) == strlen(srXML->srXMLElement[i].chTag))
			{
				if(inValueLen < srXML->srXMLElement[i].shValueLen)
					memcpy(chValue,srXML->srXMLElement[i].chValue,inValueLen);
				else
					memcpy(chValue,srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen);
				return (inValueLen < srXML->srXMLElement[i].shValueLen)?inValueLen:srXML->srXMLElement[i].shValueLen;
			}
		}
	}

	return ICER_ERROR;
}

int inXMLGetAmt(STRUCT_XML_DOC *srXML,char *chGetName,long *lnValue,int inStartIndex)
{
int i;
BYTE bBuf[50];

	*lnValue = 0L;
	for(i=inStartIndex;i<srXML->inXMLElementCnt;i++)
	{
		if(!ECC_strncasecmp(chGetName,srXML->srXMLElement[i].chTag,strlen(chGetName)))
		{
			if(strlen(chGetName) == strlen(srXML->srXMLElement[i].chTag))
			{
				memset(bBuf,0x00,sizeof(bBuf));
				if(srXML->srXMLElement[i].shValueLen > 2)
					memcpy(bBuf,srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen - 2);
				else
					memcpy(bBuf,srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen);
				*lnValue = atol((char *)bBuf);
				return i;
			}
		}
	}

	return ICER_ERROR;
}

int inXMLInsertData(STRUCT_XML_DOC *srXML,STRUCT_XML_ACTION_DATA *srXMLAction,char *chInsertTagName,BOOL fXMLSendFlag)
{
int i;
//BOOL fInsertNodeFinded = FALSE;
//char *chTag;

#if 1
	if(strlen(chInsertTagName) == 0)
	{
		inXMLMoveMultipleData(srXML,srXML->inXMLElementCnt,srXMLAction->inParseCnt);
		inXMLCopyMultipleData(srXML,srXML->inXMLElementCnt,srXMLAction);
		srXML->inXMLElementCnt += srXMLAction->inParseCnt;
		return srXML->inXMLElementCnt;
	}

	for(i=0;i<srXML->inXMLElementCnt;i++)
	{
		if(!ECC_strncasecmp(srXML->srXMLElement[i].chTag,chInsertTagName,strlen(chInsertTagName)) &&
			srXML->srXMLElement[i].shTagLen == (short)strlen(chInsertTagName))
		{
			inXMLMoveMultipleData(srXML,i,srXMLAction->inParseCnt);
			inXMLCopyMultipleData(srXML,i,srXMLAction);
			//srXML->srXMLElement[i].fXMLSendFlag = fXMLSendFlag;
			srXML->inXMLElementCnt += srXMLAction->inParseCnt;
			return i;
		}
	}
#else
	for(i=0;i<srXML->inXMLElementCnt;i++)
	{
		if(fInsertNodeFinded == TRUE)
		{
			if(!memcmp(chTag,srXML->srXMLElement[i].chTag,srXML->srXMLElement[i].shTagLen))
			{
				inXMLMoveMultipleData(srXML,i,srXMLAction->inParseCnt);
				return i;
			}
		}
		else if(!memcmp(srXML->srXMLElement[i].chTag,chInsertTagName,strlen(chInsertTagName)) &&
				srXML->srXMLElement[i].shTagLen == (short)strlen(chInsertTagName))
		//else if(/*fInsertNodeFinded == FALSE && */srXML->srXMLElement[i].inNodeNumber == inInsertNodeNumber)
		{
			fInsertNodeFinded = TRUE;
			chTag = (char *)ECC_calloc(1,srXML->srXMLElement[i].shTagLen + 2);
			chTag[0] = '/';
			memcpy(&chTag[1],srXML->srXMLElement[i].chTag,srXML->srXMLElement[i].shTagLen);
		}
	}
#endif

	return ICER_ERROR;
}

int inXMLMoveMultipleData(STRUCT_XML_DOC *srXML,int inMoveStart,int inMoveCnt)
{
int i;

	for(i=srXML->inXMLElementCnt - 1;i>inMoveStart - 1;i--)
	{
		inXMLMoveData(&srXML->srXMLElement[i],&srXML->srXMLElement[i+inMoveCnt]);
	}

	return SUCCESS;
}

int inXMLMoveMultipleData2(STRUCT_XML_DOC *srXML,int inMoveSour,int inMoveDest,int inMoveCnt)
{
int i;

	for(i=0;i<inMoveCnt;i++)
	{
		inXMLMoveData(&srXML->srXMLElement[inMoveSour + i],&srXML->srXMLElement[inMoveDest + i]);
	}

	return SUCCESS;
}

int inXMLMoveData(STRUCT_XML_ELEMENT *srSourElement,STRUCT_XML_ELEMENT *srDestElement)
{

	srDestElement->fNodeFlag = srSourElement->fNodeFlag;
	srDestElement->inNodeNumber = srSourElement->inNodeNumber;
	srDestElement->shTagLen = srSourElement->shTagLen;
	srDestElement->shValueLen = srSourElement->shValueLen;
	srDestElement->chTag = srSourElement->chTag;
	srDestElement->chValue = srSourElement->chValue;
	srDestElement->fXMLSendFlag = srSourElement->fXMLSendFlag;
	srDestElement->fNoResponseTM = srSourElement->fNoResponseTM;
	memset(srSourElement,0x00,sizeof(STRUCT_XML_ELEMENT));

	return SUCCESS;
}

int inXMLCopyMultipleData(STRUCT_XML_DOC *srXML,int inCopytart,STRUCT_XML_ACTION_DATA *srXMLAction)
{
int i,inNodeNo;

	//inNodeNo = srXML->srXMLElement[srXML->inXMLElementCnt].inNodeNumber + 1;
	for(i=0;i<srXMLAction->inParseCnt;i++)
	{
		inNodeNo = 0;
		switch(srXMLAction->srXMLDetail[i].inNodeNoType)
		{
			case NODE_NO_SAME:
				if((inCopytart + i) > 0)
					inNodeNo = srXML->srXMLElement[inCopytart + i - 1].inNodeNumber;
				break;
			case NODE_NO_ADD:
				if((inCopytart + i) > 0)
					inNodeNo = srXML->srXMLElement[inCopytart + i - 1].inNodeNumber + 1;
				break;
			case NODE_NO_DEC:
				if((inCopytart + i) > 0)
					inNodeNo = srXML->srXMLElement[inCopytart + i - 1].inNodeNumber - 1;
				break;
		}
		inXMLCopyData(inNodeNo,&srXMLAction->srXMLDetail[i],&srXML->srXMLElement[inCopytart + i]);
	}

	return SUCCESS;
}

int inXMLCopyData(int inNodeNo,STRUCT_XML_DETAIL *srSourElement,STRUCT_XML_ELEMENT *srDestElement)
{

	srDestElement->fNodeFlag = srSourElement->fNodeFlag;
	srDestElement->fXMLSendFlag = srSourElement->fXMLSendFlag;
	srDestElement->inNodeNumber = inNodeNo;
	srDestElement->shTagLen = strlen(srSourElement->chTagName);
	srDestElement->chTag = (char *)ECC_calloc(1,srDestElement->shTagLen + 1);
	memset(srDestElement->chTag,0x00,srDestElement->shTagLen + 1);
	memcpy(srDestElement->chTag,srSourElement->chTagName,srDestElement->shTagLen);
	if(!srDestElement->fNodeFlag && srSourElement->chTagName[0] != '/')
	{
		srDestElement->shValueLen = strlen(srSourElement->chValue);
		srDestElement->chValue = (char *)ECC_calloc(1,srDestElement->shValueLen + 1);
		memset(srDestElement->chValue,0x00,srDestElement->shValueLen + 1);
		memcpy(srDestElement->chValue,srSourElement->chValue,srDestElement->shValueLen);
	}

	return SUCCESS;
}

int inXMLCheckTMInputData(int inTxnType,STRUCT_XML_DOC *srXML)
{
int i,inCntStart,inCntEnd;

	for(i=0;i<MAX_XML_CHECK_CNT;i++)
	{
		if(strlen(srXMLTMInputCheck[inTxnType].chTagName[i]) == 0)
			return SUCCESS;

		if(inXMLSearchValueByTag(srXML,srXMLTMInputCheck[inTxnType].chTagName[i],&inCntStart,&inCntEnd,0) < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inXMLCheckTMInputData Fail 1:%s",srXMLTMInputCheck[inTxnType].chTagName[i]);
			return ICER_ERROR;
		}
	}

	return SUCCESS;
}

int inXMLCheckTagDuplicated(STRUCT_XML_DOC *srXML,char *chTag)
{
	int startIndex = 0, endIndex = 0, lastIndex = 0;
	
	if (inXMLSearchValueByTag(srXML, chTag, &startIndex, &endIndex, 0) >= SUCCESS)
	{
		
		lastIndex = startIndex  + 1;
		if (inXMLSearchValueByTag(srXML, chTag, &startIndex, &endIndex, lastIndex) >= SUCCESS)
				return 1;
		
		return 0;
	}

	return - 1;
}

int inXMLSearchValueByTag(STRUCT_XML_DOC *srXML,char *chTag,int *inStart,int *inEnd,int inInStartCnt)
{
int i,j;
BOOL fSearchOK = FALSE;

	for(i=inInStartCnt;i<srXML->inXMLElementCnt;i++)
	{
		if(!ECC_strncasecmp(chTag,srXML->srXMLElement[i].chTag,srXML->srXMLElement[i].shTagLen))
		{
			if((short)strlen(chTag) == srXML->srXMLElement[i].shTagLen)
			{
				if(fSearchOK == FALSE)
					*inStart = *inEnd = i;

				if(srXML->srXMLElement[i].shValueLen > 0 || chTag[0] == '/' || !ECC_strncasecmp(chTag,(char *)TAG_TRANS_HEADER,strlen(chTag)))
					return SUCCESS;

				for(j=i+1;j<srXML->inXMLElementCnt;j++)
				{
					if(srXML->srXMLElement[j].inNodeNumber == srXML->srXMLElement[i].inNodeNumber)
					{
						fSearchOK = TRUE;
						i = *inEnd = j;
						break;
					}
				}
			}
		}
	}

	if(fSearchOK == TRUE)
		return SUCCESS;

	return ICER_ERROR;
}

void vdBuildXMLHeaderMsg(BYTE *bOutData,STRUCT_XML_DOC *srXMLRes)
{
//STRUCT_XML_DOC *srXMLRes;

//	srXMLRes = (STRUCT_XML_DOC *)bInData;

	if(srXMLRes->srXMLElement[0].chTag[0] == '?' && srXMLRes->srXMLElement[0].chTag[srXMLRes->srXMLElement[0].shTagLen - 1] == '?')
	{
		bOutData[0] = '<';
		memcpy(&bOutData[1],srXMLRes->srXMLElement[0].chTag,srXMLRes->srXMLElement[0].shTagLen);
		bOutData[srXMLRes->srXMLElement[0].shTagLen + 1] = '>';
		memcpy(&bOutData[srXMLRes->srXMLElement[0].shTagLen + 2],"\x0D\x0A",2);
	}
}

void vdBuildXMLResponse(int inTxnType,BYTE *bOutData,int inOutDataStart,STRUCT_XML_DOC *srXMLRes,BOOL bResponseAll)
{
//STRUCT_XML_DOC *srXMLRes = (STRUCT_XML_DOC *)bInData;
int /*i,j,*/k,inCnt = inOutDataStart,inSize,inCntStart,inCntEnd;
//BOOL fFirstTag = TRUE;

	//for(i=0;i<MAX_XML_CHECK_CNT;i++)
	{
		/*if(strlen(srXMLTMOutput[inTxnType].chTagName[i]) == 0)
			return;

		j = inXMLSearchValueByTag(srXMLRes,srXMLTMOutput[inTxnType].chTagName[i],&inCntStart,&inCntEnd,0);
		if(j != SUCCESS)
			continue;*/
		/*if(j != SUCCESS)
		{
			if(fFirstTag == FALSE)
			{
				memcpy(&bOutData[inCnt],TAG_TRANS_END,strlen(TAG_TRANS_END));
				inCnt += strlen(TAG_TRANS_END);
				memcpy(&bOutData[inCnt++],"\n",1);
			}

			return;
		}*/

		inCntStart = 1;
		inCntEnd = srXMLRes->inXMLElementCnt - 1;
		k = inCntStart;
		do
		{
			if((srXMLRes->srXMLElement[k].chTag[0] == '?' && srXMLRes->srXMLElement[k].chTag[srXMLRes->srXMLElement[k].shTagLen - 1] == '?') ||
				!ECC_strncasecmp(srXMLRes->srXMLElement[k].chTag,(char *)TAG_TRANS_XML_HEADER,strlen(TAG_TRANS_XML_HEADER)) ||
				!ECC_strncasecmp(srXMLRes->srXMLElement[k].chTag,(char *)TAG_TRANS_HEADER,strlen(TAG_TRANS_HEADER)) ||
				!ECC_strncasecmp(srXMLRes->srXMLElement[k].chTag,(char *)TAG_TRANS_END,strlen(TAG_TRANS_END)) ||
				!ECC_strncasecmp(srXMLRes->srXMLElement[k].chTag,(char *)TAG_TRANS_XML_END,strlen(TAG_TRANS_XML_END)))
			{
				k++;
				continue;
			}

			if(bResponseAll == FALSE && ECC_strncasecmp(srXMLRes->srXMLElement[k].chTag,(char *)"T5548",5) && ECC_strncasecmp(srXMLRes->srXMLElement[k].chTag,(char *)"/T5548",6))
			{
				k++;
				continue;
			}

			if(srXMLRes->srXMLElement[k].fNoResponseTM)
			{
				k++;
				continue;
			}

			//if(srXMLRes->srXMLElement[k].shValueLen > 0/* && !srXMLRes->srXMLElement[k].fNodeFlag*/)
			{
				bOutData[inCnt++] = '<';
				inSize = srXMLRes->srXMLElement[k].shTagLen;
				memcpy(&bOutData[inCnt],srXMLRes->srXMLElement[k].chTag,inSize);
				inCnt += inSize;
				bOutData[inCnt++] = '>';
			}

			if(srXMLRes->srXMLElement[k].shValueLen > 0)
			{
				inSize = srXMLRes->srXMLElement[k].shValueLen;
				memcpy(&bOutData[inCnt],srXMLRes->srXMLElement[k].chValue,inSize);
				inCnt += inSize;

				bOutData[inCnt++] = '<';
				bOutData[inCnt++] = '/';
				inSize = srXMLRes->srXMLElement[k].shTagLen;
				memcpy(&bOutData[inCnt],srXMLRes->srXMLElement[k].chTag,inSize);
				inCnt += inSize;
				bOutData[inCnt++] = '>';
			}

			memcpy(&bOutData[inCnt++],"\n",1);
			k++;
		}
		while(k <= inCntEnd);
	}

}

int inXMLUpdateData(STRUCT_XML_DOC *srXML,char *chInsterTagName,char *chUpdateTagName,BYTE *bUpdateValue,BOOL fXMLSendFlag)
{
	int inRetVal,inCntStart,inCntEnd;
	char chBuf[10];

	if(strlen((char *)bUpdateValue) == 0)
	{
		log_msg(LOG_LEVEL_FLOW,"inXMLUpdateData Update(%s) Value = NULL !!",chUpdateTagName);
		return SUCCESS;
	}

	inCntStart = inCntEnd = 0;
	memset(chBuf,0x00,sizeof(chBuf));
	inRetVal = inXMLSearchValueByTag(srXML,chUpdateTagName,&inCntStart,&inCntEnd,0);
	if(inRetVal < SUCCESS)
	{
		vdSetXMLActionData(&srXMLActionData,chUpdateTagName,(char *)bUpdateValue,TRUE,NODE_NO_SAME,VALUE_NAME,fXMLSendFlag);
		if(inXMLSearchValueByTag(srXML,(char *)"/TRANS",&inCntStart,&inCntEnd,0) >= SUCCESS)
			inXMLInsertData(srXML,&srXMLActionData,(char *)"/TRANS",FALSE);//chInsterTagName此參數暫不使用,用預設的"/TRANS"
		else if(inXMLSearchValueByTag(srXML,(char *)"/TransXML",&inCntStart,&inCntEnd,0) >= SUCCESS)
			inXMLInsertData(srXML,&srXMLActionData,(char *)"/TransXML",FALSE);
		else
			inXMLInsertData(srXML,&srXMLActionData,(char *)"",FALSE);
		//inXMLInsertData(srXML,&srXMLActionData,chInsterTagName,FALSE);
		if(fXMLSendFlag == TRUE)
		{
			inRetVal = inXMLSearchValueByTag(srXML,chUpdateTagName,&inCntStart,&inCntEnd,0);
			if(inRetVal >= SUCCESS)
				srXML->srXMLElement[inCntStart].fXMLSendFlag = fXMLSendFlag;
		}
	}
	else
	{
		if(srXML->srXMLElement[inCntStart].shValueLen != (short)strlen((char *)bUpdateValue) ||//長度不同
			memcmp(srXML->srXMLElement[inCntStart].chValue,bUpdateValue,strlen((char *)bUpdateValue)) ||//資料不同
			srXML->srXMLElement[inCntStart].fXMLSendFlag != fXMLSendFlag)
		{
			srXML->srXMLElement[inCntStart].shValueLen = strlen((char *)bUpdateValue);
			ECC_free((BYTE *)srXML->srXMLElement[inCntStart].chValue);
			srXML->srXMLElement[inCntStart].chValue = (char *)ECC_calloc(1,strlen((char *)bUpdateValue) + 1);
			memset(srXML->srXMLElement[inCntStart].chValue,0x00,strlen((char *)bUpdateValue) + 1);
			memcpy(srXML->srXMLElement[inCntStart].chValue,bUpdateValue,strlen((char *)bUpdateValue));
			srXML->srXMLElement[inCntStart].fXMLSendFlag = fXMLSendFlag;
		}
	}

	return SUCCESS;
}

int inXMLCheckTMCanNotInputData(int inTxnType,STRUCT_XML_DOC *srXML)
{
long lnAmt = 0L;
char chRedeemFlag = 0x00;

	if(inTxnType != TXN_SALE)
		return SUCCESS;

	inXMLGetData(&srXMLData,(char *)TAG_NAME_5599,&chRedeemFlag,sizeof(chRedeemFlag),0);
	if(fIsOnlineTxn() == FALSE || chRedeemFlag == 'N')
	{
		inXMLGetAmt(&srXMLData,(char *)TAG_NAME_0404,&lnAmt,0);
		if(lnAmt > 0)
		{
			log_msg(LOG_LEVEL_ERROR,"inXMLCheckTMCanNotInputData(%s) Fail 1:%ld",(char *)TAG_NAME_0404,lnAmt);
			return TM_INPUT_DATA_ERROR;
		}

		inXMLGetAmt(&srXMLData,(char *)TAG_NAME_5535,&lnAmt,0);
		if(lnAmt > 0)
		{
			log_msg(LOG_LEVEL_ERROR,"inXMLCheckTMCanNotInputData(%s) Fail 2:%ld",(char *)TAG_NAME_5535,lnAmt);
			return TM_INPUT_DATA_ERROR;
		}
	}

	return SUCCESS;
}

/*int inXMLModifyData(STRUCT_XML_DOC *srXML,char *chModiftName,char *chModifyValue)
{
int i,inOK = 0;

	for(i=0;i<srXML->inXMLElementCnt;i++)
	{
		if(!ECC_strncasecmp(chModiftName,srXML->srXMLElement[i].chTag,strlen(chModiftName)))
		{
			if(srXML->srXMLElement[i].shValueLen != strlen(chModifyValue))
			{
				srXML->srXMLElement[i].shValueLen = strlen(chModifyValue);
				ECC_free(srXML->srXMLElement[i].chValue);
				srXML->srXMLElement[i].chValue = (char *)ECC_calloc(1,srXML->srXMLElement[i].shValueLen + 1);
			}

			memcpy(srXML->srXMLElement[i].chValue,chModifyValue,srXML->srXMLElement[i].shValueLen);
			return SUCCESS;
		}
	}

	return ICER_ERROR;
}*/

int inCheckXMLDataFormat(STRUCT_XML_DOC *srXML)
{
int i,inCheckType,inRetVal;
long lnTag;

	for(i=0;i<srXML->inXMLElementCnt;i++)
	{
		if(srXML->srXMLElement[i].fNodeFlag)
			continue;
		if(fIsXMLTAGFormat(&srXML->srXMLElement[i]) != TRUE)
			continue;
		lnTag = inASCToLONG(&srXML->srXMLElement[i].chTag[1],srXML->srXMLElement[i].shTagLen - 1);
		inCheckType = inGetXMLCheckType(lnTag);
		if(inCheckType == ICER_ERROR)
		{
			log_msg(LOG_LEVEL_ERROR,"inCheckXMLDataFormat[%s] (%s)(%d) Fail 1!!",srXML->srXMLElement[i].chTag,srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen);
			return ICER_REQ_DATA_FAIL;
		}
		inRetVal = inXMLFormatCheck(srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,inCheckType);
		if(inRetVal != SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inCheckXMLDataFormat[%s] (%s)(%d) Fail 2!!%d",srXML->srXMLElement[i].chTag,srXML->srXMLElement[i].chValue,srXML->srXMLElement[i].shValueLen,inCheckType);
			return ICER_REQ_DATA_FAIL;
		}
	}

	return SUCCESS;
}

BOOL fIsXMLTAGFormat(STRUCT_XML_ELEMENT *srElement)
{
int inRetVal;

	if(srElement->chTag[0] != 'T')
		return FALSE;

	if(srElement->shTagLen < 5 || srElement->shTagLen > 7)
		return FALSE;

	inRetVal = inXMLFormatCheck(&srElement->chTag[1],srElement->shTagLen - 1,XML_CHECK_NUMERIC);
	if(inRetVal != SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"fIsXMLTAGFormat(%s) Fail:%d",srElement->chTag,srElement->shTagLen);
		return FALSE;
	}

	return TRUE;
}

int inXMLFormatCheck(char *chData,short shDataLen,int inFormatType)
{
int i;
long j;

	//文數字不檢查
	if(inFormatType == XML_CHECK_NUMERIC_ALPHA)
		return SUCCESS;

	//長度檢查
	switch(inFormatType)
	{
		case XML_CHECK_NUMERIC_AMOUNT:
			if(shDataLen < 3)
				return ICER_REQ_DATA_FAIL;
			break;
		case XML_CHECK_NUMERIC_TIME:
			if(shDataLen != 6)
				return ICER_REQ_DATA_FAIL;
			break;
		case XML_CHECK_NUMERIC_DATE:
			if(shDataLen != 8)
				return ICER_REQ_DATA_FAIL;
			break;
		case XML_CHECK_NUMERIC_DATE_TIME:
			if(shDataLen != 14)
				return ICER_REQ_DATA_FAIL;
			break;
	}

	//純數字檢查
	for(i=0;i<shDataLen;i++)
	{
		if(chData[i] < '0' || chData[i] > '9')
			return ICER_REQ_DATA_FAIL;
	}

	if(inFormatType == XML_CHECK_NUMERIC)
		return SUCCESS;
	else if(inFormatType == XML_CHECK_NUMERIC_AMOUNT)
	{
		j = inASCToLONG(&chData[shDataLen - 2],2);
		if(j != 0)
			return ICER_REQ_DATA_FAIL;
	}
	else if(inFormatType == XML_CHECK_NUMERIC_TIME)
	{
		j = inASCToLONG(&chData[0],2);//mm
		if(j > 24)
			return ICER_REQ_DATA_FAIL;
		j = inASCToLONG(&chData[2],2);//hh
		if(j > 59)
			return ICER_REQ_DATA_FAIL;
		j = inASCToLONG(&chData[4],2);//ss
		if(j > 59)
			return ICER_REQ_DATA_FAIL;
	}
	else if(inFormatType == XML_CHECK_NUMERIC_DATE)
	{
		j = inASCToLONG(&chData[4],2);//MM
		if(j > 12)
			return ICER_REQ_DATA_FAIL;
		j = inASCToLONG(&chData[6],2);//DD
		if(j > 31)
			return ICER_REQ_DATA_FAIL;
	}
	else if(inFormatType == XML_CHECK_NUMERIC_DATE_TIME)
	{
		j = inASCToLONG(&chData[4],2);//MM
		if(j > 12)
			return ICER_REQ_DATA_FAIL;
		j = inASCToLONG(&chData[6],2);//DD
		if(j > 31)
			return ICER_REQ_DATA_FAIL;
		j = inASCToLONG(&chData[8],2);//mm
		if(j > 24)
			return ICER_REQ_DATA_FAIL;
		j = inASCToLONG(&chData[10],2);//hh
		if(j > 59)
			return ICER_REQ_DATA_FAIL;
		j = inASCToLONG(&chData[12],2);//ss
		if(j > 59)
			return ICER_REQ_DATA_FAIL;
	}

	return SUCCESS;
}

int inGetXMLCheckType(long lnTag)
{
int i,inCnt;

	inCnt = sizeof(srXMLFormatCheck) / sizeof(STRUCT_XML_FORMAT_CHECK);
	for(i=0;i<inCnt;i++)
	{
		if(lnTag == srXMLFormatCheck[i].lnTag)
			return srXMLFormatCheck[i].inXMLCheckType;
	}

	return ICER_ERROR;
}

int inXMLCompareData(STRUCT_XML_DOC *srXML,char *chTagName,char *chCompareValue,int inStartIndex)
{
int inRetVal,inCntStart = 0,inCntEnd = 0;

	inRetVal = inXMLSearchValueByTag(srXML,chTagName,&inCntStart,&inCntEnd,0);
	if(inRetVal < SUCCESS)
		return ICER_ERROR;

	if(srXML->srXMLElement[inCntStart].shValueLen == (short)strlen(chCompareValue))
		if(!memcmp(srXML->srXMLElement[inCntStart].chValue,chCompareValue,srXML->srXMLElement[inCntStart].shValueLen))
			return SUCCESS;

	return ICER_ERROR;
}

int inXMLDeleteData(STRUCT_XML_DOC *srXML,int inDeleteStart,int inDeleteCnt)
{
int i;

	for(i=inDeleteStart;i<inDeleteStart + inDeleteCnt;i++)
	{
		srXML->srXMLElement[i].fNodeFlag = 0x00;
		srXML->srXMLElement[i].inNodeNumber = 0;
		srXML->srXMLElement[i].shTagLen = 0;
		srXML->srXMLElement[i].shValueLen = 0;
		if(srXML->srXMLElement[i].chTag)
			ECC_free((BYTE *)srXML->srXMLElement[i].chTag);
		if(srXML->srXMLElement[i].chValue)
			ECC_free((BYTE *)srXML->srXMLElement[i].chValue);
		srXML->inXMLElementCnt--;
	}

	return SUCCESS;
}

int inXMLAppendData(STRUCT_XML_DOC *srXML,char *chTagName,int inTagLen,BYTE *chValueName,int inValueLen,int inNodeType,BOOL fXMLSendFlag)
{

	if(inNodeType == VALUE_NAME && inValueLen <= 0)
		return SUCCESS;

	srXML->srXMLElement[srXML->inXMLElementCnt].fXMLSendFlag = fXMLSendFlag;
	srXML->srXMLElement[srXML->inXMLElementCnt].fNodeFlag = (inNodeType == TAG_NAME_START)?TRUE:FALSE;
	if(inNodeType == TAG_NAME_START)
		srXML->srXMLElement[srXML->inXMLElementCnt].inNodeNumber = srXML->srXMLElement[srXML->inXMLElementCnt - 1].inNodeNumber + 1;
	else if(inNodeType == TAG_NAME_END)
		srXML->srXMLElement[srXML->inXMLElementCnt].inNodeNumber = srXML->srXMLElement[srXML->inXMLElementCnt - 1].inNodeNumber - 1;
	else
		srXML->srXMLElement[srXML->inXMLElementCnt].inNodeNumber = srXML->srXMLElement[srXML->inXMLElementCnt - 1].inNodeNumber;
	srXML->srXMLElement[srXML->inXMLElementCnt].shTagLen = inTagLen;
	srXML->srXMLElement[srXML->inXMLElementCnt].shValueLen = inValueLen;
	if(inTagLen > 0)
	{
		srXML->srXMLElement[srXML->inXMLElementCnt].chTag = (char *)ECC_calloc(1,srXML->srXMLElement[srXML->inXMLElementCnt].shTagLen + 1);
		memset(srXML->srXMLElement[srXML->inXMLElementCnt].chTag,0x00,srXML->srXMLElement[srXML->inXMLElementCnt].shTagLen + 1);
		memcpy(srXML->srXMLElement[srXML->inXMLElementCnt].chTag,chTagName,srXML->srXMLElement[srXML->inXMLElementCnt].shTagLen);
	}
	if(inValueLen > 0)
	{
		srXML->srXMLElement[srXML->inXMLElementCnt].chValue = (char *)ECC_calloc(1,srXML->srXMLElement[srXML->inXMLElementCnt].shValueLen + 1);
		memset(srXML->srXMLElement[srXML->inXMLElementCnt].chValue,0x00,srXML->srXMLElement[srXML->inXMLElementCnt].shValueLen + 1);
		memcpy(srXML->srXMLElement[srXML->inXMLElementCnt].chValue,chValueName,srXML->srXMLElement[srXML->inXMLElementCnt].shValueLen);
	}
	srXML->inXMLElementCnt++;

	return SUCCESS;
}

void vdSetConfigFlag(STRUCT_XML_DOC *srXML,int inSetType,char *chTagName)
{
int i;

	for(i=0;i<srXML->inXMLElementCnt;i++)
	{
		//if(fSetAllTag == TRUE)//這樣才對!!
		if(inSetType == TRUE)//全部設定
		{
			if((short)strlen(chTagName) != srXML->srXMLElement[i].shTagLen || ECC_strncasecmp(chTagName,srXML->srXMLElement[i].chTag,srXML->srXMLElement[i].shTagLen))
				srXML->srXMLElement[i].fXMLSendFlag = TRUE;
		}
		else if(inSetType == FALSE)//設定一個Tag成TRUE
		{
			if((short)strlen(chTagName) == srXML->srXMLElement[i].shTagLen && !ECC_strncasecmp(chTagName,srXML->srXMLElement[i].chTag,srXML->srXMLElement[i].shTagLen))
			{
				srXML->srXMLElement[i].fXMLSendFlag = TRUE;
				return;
			}
		}
		else if(inSetType == 2)//設定一個Tag成FALSE
		{
			if((short)strlen(chTagName) == srXML->srXMLElement[i].shTagLen && !ECC_strncasecmp(chTagName,srXML->srXMLElement[i].chTag,srXML->srXMLElement[i].shTagLen))
			{
				srXML->srXMLElement[i].fXMLSendFlag = FALSE;
				return;
			}
		}
		else if(inSetType == 3)//設定多個Tag成FALSE
		{
			if( !ECC_strncasecmp(chTagName,srXML->srXMLElement[i].chTag,strlen(chTagName)) ||
				!ECC_strncasecmp(chTagName,&srXML->srXMLElement[i].chTag[1],strlen(chTagName)))
			{
				srXML->srXMLElement[i].fXMLSendFlag = FALSE;
				//return;
			}
		}
	}
}

void vdUnPackToXMLData(BYTE *BINData,int BINSize,STRUCT_XML_DOC *srXML,char *chTagName,int inTagLen,int inNodeType,BOOL fXMLSendFlag)
{
BYTE bBuf[200];

	memset(bBuf,0x00,sizeof(bBuf));
	fnUnPack(BINData,BINSize,bBuf);
	//inXMLAppendData(srXML,chTagName,inTagLen,bBuf,strlen((char *)bBuf),inNodeType,fXMLSendFlag);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,chTagName,bBuf,fXMLSendFlag);
}

void vdXMLBackUpData(STRUCT_XML_DOC *srXMLIn,STRUCT_XML_DOC *srXMLOut,BOOL fFreeData)
{
	int i;

	if(fFreeData == TRUE)
		vdFreeXMLDOC(srXMLOut);
	else
		memset(srXMLOut,0x00,sizeof(STRUCT_XML_DOC));

	memset(srXMLOut,0x00,sizeof(STRUCT_XML_DOC));
	srXMLOut->inXMLElementCnt = srXMLIn->inXMLElementCnt;

	for(i=0;i<srXMLIn->inXMLElementCnt;i++)
	{
		srXMLOut->srXMLElement[i].fNodeFlag = srXMLIn->srXMLElement[i].fNodeFlag;
		srXMLOut->srXMLElement[i].fXMLSendFlag = srXMLIn->srXMLElement[i].fXMLSendFlag;
		srXMLOut->srXMLElement[i].fRFU = srXMLIn->srXMLElement[i].fRFU;
		srXMLOut->srXMLElement[i].inNodeNumber = srXMLIn->srXMLElement[i].inNodeNumber;
		srXMLOut->srXMLElement[i].shTagLen = srXMLIn->srXMLElement[i].shTagLen;
		if(srXMLOut->srXMLElement[i].shTagLen > 0)
		{
			srXMLOut->srXMLElement[i].chTag = (char *)ECC_calloc(1,srXMLIn->srXMLElement[i].shTagLen + 1);
			memset(srXMLOut->srXMLElement[i].chTag,0x00,srXMLIn->srXMLElement[i].shTagLen + 1);
			memcpy(srXMLOut->srXMLElement[i].chTag,srXMLIn->srXMLElement[i].chTag,srXMLIn->srXMLElement[i].shTagLen);
			srXMLOut->srXMLElement[i].chTag[srXMLIn->srXMLElement[i].shTagLen] = 0x00;
		}
		srXMLOut->srXMLElement[i].shValueLen = srXMLIn->srXMLElement[i].shValueLen;
		if(srXMLOut->srXMLElement[i].shValueLen > 0)
		{
			srXMLOut->srXMLElement[i].chValue = (char *)ECC_calloc(1,srXMLIn->srXMLElement[i].shValueLen + 1);
			memset(srXMLOut->srXMLElement[i].chValue,0x00,srXMLIn->srXMLElement[i].shValueLen + 1);
			memcpy(srXMLOut->srXMLElement[i].chValue,srXMLIn->srXMLElement[i].chValue,srXMLIn->srXMLElement[i].shValueLen);
			srXMLOut->srXMLElement[i].chValue[srXMLIn->srXMLElement[i].shValueLen] = 0x00;
		}
	}
}

void vdModifySAmtTag(STRUCT_XML_DOC *srXML,int inMainRetVal)
{
BYTE bBuf[50];
int inRetVal = 0,inCntStart = 0,inCntEnd = 0/*,in1100Start = 0,in1100End = 0,in1101Start = 0,in1101End = 0*/;

	if(srTxnData.srIngData.inTransType == TXN_ECC_SIGN_ON || srTxnData.srIngData.inTransType == TXN_ECC_MATCH)
	{
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1200,(BYTE *)srTxnData.srIngData.chCMASTxTime,TRUE);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1300,(BYTE *)srTxnData.srIngData.chCMASTxDate,TRUE);
	}

	//inXMLSearchValueByTag(srXML,(char *)TAG_NAME_1100,&in1100Start,&in1100End,0);
	//inXMLSearchValueByTag(srXML,(char *)TAG_NAME_1101,&in1101Start,&in1101End,0);

	//T1100
	/*if(in1101Start > SUCCESS)
		memcpy(srXML->srXMLElement[in1101Start].chTag,(char *)TAG_NAME_1100,strlen(TAG_NAME_1100));*/
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulTMSerialNumber);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1100,bBuf,TRUE);

//#if 1//移到上層,不然,會有該回沒有的問題!!
	//T1101
	/*if(in1100Start > SUCCESS)
		memcpy(srXML->srXMLElement[in1100Start].chTag,(char *)TAG_NAME_1101,strlen(TAG_NAME_1101));*/
	//if(srTxnData.srIngData.ulCMASHostSerialNumber > 0)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		log_msg(LOG_LEVEL_FLOW,"ulCMASHostSerialNumber 3:%ld:%ld",srTxnData.srIngData.ulCMASHostSerialNumber,srTxnData.srREQData.ulT1101);
		//if(srTxnData.srIngData.ulCMASHostSerialNumber > srTxnData.srREQData.ulT1101)
		//	srTxnData.srIngData.ulCMASHostSerialNumber--;
		sprintf((char *)bBuf,"%06lu",srTxnData.srIngData.ulCMASHostSerialNumber);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1101,bBuf,TRUE);
		memcpy(gTmpData.ucCMAS_REQ_T1101,bBuf,sizeof(gTmpData.ucCMAS_REQ_T1101)-1);
	}
//#endif

	memset(bBuf,0x00,sizeof(bBuf));
	vdMaskCMASCardID(srXML);

	if(srTxnData.srIngData.inTransType == TXN_ECC_READ_BASIC ||
	   srTxnData.srIngData.inTransType == TXN_ECC_READ_TAXI ||
	   srTxnData.srIngData.inTransType == TXN_ECC_READ_CBIKE ||
	   srTxnData.srIngData.inTransType == TXN_ECC_READ_EDCA ||
	   srTxnData.srIngData.inTransType == TXN_QUERY_POINT)
		return;

	if(srTxnData.srIngData.inTransType == TXN_ECC_SETTLE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		if(inXMLGetData(srXML,(char *)"T559201",(char *)bBuf,sizeof(bBuf),0) >= SUCCESS)
		{
			strcat((char *)bBuf,"00");
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T559201",bBuf,TRUE);
		}

		memset(bBuf,0x00,sizeof(bBuf));
		if(inXMLGetData(srXML,(char *)"T559202",(char *)bBuf,sizeof(bBuf),0) >= SUCCESS)
		{
			strcat((char *)bBuf,"00");
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T559202",bBuf,TRUE);
		}

		memset(bBuf,0x00,sizeof(bBuf));
		if(inXMLGetData(srXML,(char *)"T559203",(char *)bBuf,sizeof(bBuf),0) >= SUCCESS)
		{
			strcat((char *)bBuf,"00");
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T559203",bBuf,TRUE);
		}
	}
	else if((srTxnData.srIngData.inTransType == TXN_ECC_VOID) || (srTxnData.srIngData.inTransType == TXN_ECC_VOID1))
	{
		if(srTxnData.srREQData.lnT0404 > 0)
		{
			memset(bBuf,0x00,sizeof(bBuf));
			sprintf((char *)bBuf,"%lu00",srTxnData.srREQData.lnT0404);
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0404,bBuf,TRUE);
		}
	}

	memset(bBuf,0x00,sizeof(bBuf));
	if(inXMLGetData(srXML,(char *)TAG_NAME_0400,(char *)bBuf,sizeof(bBuf),0) >= SUCCESS)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		sprintf((char *)bBuf,"%lu00",srTxnData.srIngData.lnECCAmt);
		/*if(srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT && srTxnData.srREQData.chDiscountType == 'D')

		{
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0403,bBuf,TRUE);
			memset(bBuf,0x00,sizeof(bBuf));
			sprintf((char *)bBuf,"%lu00",srTxnData.srIngData.lnECCAmt + srTxnData.srIngData.lnDiscountAmt);
		}*/

		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0400,bBuf,TRUE);
	}

	memset(bBuf,0x00,sizeof(bBuf));
	inRetVal = inXMLSearchValueByTag(srXML,(char *)TAG_NAME_0409,&inCntStart,&inCntEnd,0);
	if(inRetVal >= SUCCESS)
	{
		//memset(bBuf,0x00,sizeof(bBuf));
		//strcat((char *)bBuf,(char *)srTxnData.srIngData.anAutoloadAmount);
		//sprintf((char *)bBuf,"%lu00",srTxnData.srIngData.lnECCEV);
		//strcat((char *)bBuf,"00");
		if(fIsDeductTxn(srTxnData.srIngData.inTransType) || srTxnData.srIngData.inTransType == TXN_ECC_AUTOLOAD)
		{
			if(memcmp(bBuf,srTxnData.srIngData.anAutoloadAmount,strlen((char *)bBuf)) || strlen((char *)bBuf) != strlen((char *)srTxnData.srIngData.anAutoloadAmount))
				inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0409,srTxnData.srIngData.anAutoloadAmount,TRUE);
		}
		else
			srXML->srXMLElement[inCntStart].fNoResponseTM = TRUE;
	}

	if(fIsDeductTxn(srTxnData.srIngData.inTransType) && (strlen((char *)srTxnData.srIngData.anCardEVBeforeTxn) >= 1))
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0410,srTxnData.srIngData.anCardEVBeforeTxn,TRUE);

	if (srTxnData.srParameter.chBatchFlag == CMAS_BATCH)
	{
		//T0410改成T0415
		//if(inXMLGetData(&srXMLData,(char *)TAG_NAME_0410,(char *)bBuf,0,0) >= SUCCESS)//有0410
		vdXMLChangeTagName(srXML,(char *)TAG_NAME_0410,(char *)TAG_NAME_0415,(char *)"00");

		//T0408改成T0410
		//if(inXMLGetData(&srXMLData,(char *)TAG_NAME_0408,(char *)bBuf,0,0) >= SUCCESS)//有0408
		vdXMLChangeTagName(srXML,(char *)TAG_NAME_0408,(char *)TAG_NAME_0410,(char *)"00");
	}
	//else
	//	vdXMLChangeTagName(srXML,(char *)TAG_NAME_0410,(char *)TAG_NAME_0410,(char *)"00");

	vdBuildT4213Tag(srXML);

	if (srTxnData.srIngData.fForeignTxnFlag == TRUE)
	{
		memset(bBuf,0x00,sizeof(bBuf));
		sprintf((char *)bBuf,"%lu00",srTxnData.srIngData.lnForeignAmt);
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)"T0421",bBuf,TRUE);
	}
}

void vdXMLChangeTagName(STRUCT_XML_DOC *srXML,char *bOrgTagName,char *bNewTagName,char *bAppendData)
{
int inRetVal;
BYTE bBuf[50];
int inCntStart,inCntEnd;

	memset(bBuf,0x00,sizeof(bBuf));
	inCntStart = inCntEnd = 0;
	inRetVal = inXMLSearchValueByTag(srXML,bOrgTagName,&inCntStart,&inCntEnd,0);
	if(inRetVal == SUCCESS)
	{
		strcat((char *)bBuf,srXML->srXMLElement[inCntStart].chValue);
		if(strlen(bAppendData) > 0)
			strcat((char *)bBuf,bAppendData);
	#if 1
		if(srXML->srXMLElement[inCntStart].shValueLen != (short)strlen((char *)bBuf) ||//長度不同
			memcmp(srXML->srXMLElement[inCntStart].chValue,bBuf,strlen((char *)bBuf)))//資料不同
	#endif
			inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,bOrgTagName,bBuf,TRUE);
		memcpy(srXML->srXMLElement[inCntStart].chTag,bNewTagName,strlen(bNewTagName));
	}
}

void vdMaskCMASCardID(STRUCT_XML_DOC *srXML)
{
	int	inRetVal, inCntStart = 0,inCntEnd = 0;
//	int	inMaskAdr = 0;
	BYTE	unPurseVersionNumber[5],anCardID[20],unPID[20],bOutData[20];

	memset(bOutData,0x00,sizeof(bOutData));
	memset(unPurseVersionNumber,0x00,sizeof(unPurseVersionNumber));
	inRetVal = inXMLSearchValueByTag(srXML,(char *)TAG_NAME_4800,&inCntStart,&inCntEnd,0);
	if(inRetVal < SUCCESS)
	{
		//log_msg(LOG_LEVEL_ERROR,"vdMaskCMASCardID No Tag[%s]!!",(char *)TAG_NAME_4800);
		return;
	}
	memcpy(unPurseVersionNumber,srXML->srXMLElement[inCntStart].chValue,2);

	if(unPurseVersionNumber[1] == '0')
	{
		memset(anCardID,0x00,sizeof(anCardID));
		inRetVal = inXMLSearchValueByTag(srXML,(char *)TAG_NAME_0200,&inCntStart,&inCntEnd,0);
		if(inRetVal < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"vdMaskCMASCardID No Tag[%s]!!",(char *)TAG_NAME_0200);
			return;
		}
		memcpy(anCardID,srXML->srXMLElement[inCntStart].chValue,srXML->srXMLElement[inCntStart].shValueLen);

		vdTrimData((char *)bOutData,(char *)anCardID,GET_CARD_LEN);
		//卡號不隱藏,By Leo !!
		/*inMaskAdr = strlen((char *)bOutData);
		if(inMaskAdr >= 7)
		{
			inMaskAdr -= 7;
			memset((char *)&bOutData[inMaskAdr],'*',3);
		}*/
	}
	else
	{
		memset(unPID,0x00,sizeof(unPID));
		inRetVal = inXMLSearchValueByTag(srXML,(char *)TAG_NAME_0211,&inCntStart,&inCntEnd,0);
		if(inRetVal < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"vdMaskCMASCardID No Tag[%s]!!",(char *)TAG_NAME_0211);
			return;
		}
		memcpy(unPID,srXML->srXMLElement[inCntStart].chValue,srXML->srXMLElement[inCntStart].shValueLen);

		memcpy((char *)bOutData,(char *)unPID,16);
		//卡號不隱藏,By Leo !!
		//inMaskAdr = 5;
		//memset((char *)&bOutData[inMaskAdr],'*',6);
	}

	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0215,bOutData,FALSE);
}

void vdBuildAutolaodData(STRUCT_XML_DOC *srXML)
{
BYTE bBuf[200],bTmp[50];
int inRetVal = 0,inCntStart = 0,inCntEnd = 0;

	//T0200
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdUIntToAsc(srTxnData.srAutoloadData.ucCardID,sizeof(srTxnData.srAutoloadData.ucCardID),bTmp,17,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,17);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0200,bBuf,TRUE);

	//T0211
	if(srTxnData.srAutoloadData.ucPurseVersionNumber != MIFARE)
		vdUnPackToXMLData(srTxnData.srAutoloadData.ucPID,sizeof(srTxnData.srAutoloadData.ucPID),srXML,(char *)TAG_NAME_0211,strlen(TAG_NAME_0211),VALUE_NAME,TRUE);

	//T0213
	vdUnPackToXMLData(&srTxnData.srAutoloadData.ucCardType,sizeof(srTxnData.srAutoloadData.ucCardType),srXML,(char *)TAG_NAME_0213,strlen(TAG_NAME_0213),VALUE_NAME,TRUE);

	//T0214
	vdUnPackToXMLData(&srTxnData.srAutoloadData.ucPersonalProfile,sizeof(srTxnData.srAutoloadData.ucPersonalProfile),srXML,(char *)TAG_NAME_0214,strlen(TAG_NAME_0214),VALUE_NAME,TRUE);

	//T0408
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	vdIntToAsc(srTxnData.srAutoloadData.ucEV,sizeof(srTxnData.srAutoloadData.ucEV),bTmp,5,TRUE,0x00,10);
	vdTrimDataRight((char *)bBuf,(char *)bTmp,5);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0408,bBuf,TRUE);

	//T0409
	inRetVal = inXMLSearchValueByTag(srXML,(char *)TAG_NAME_0409,&inCntStart,&inCntEnd,0);
	if(inRetVal < SUCCESS)//沒有T0409
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_0409,srTxnData.srIngData.anAutoloadAmount,TRUE);

	//T1402
	memset(bBuf,0x00,sizeof(bBuf));
	UnixToDateTime((BYTE*)srTxnData.srAutoloadData.ucExpiryDate,(BYTE*)bBuf,8);//Expiry Date
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_1402,bBuf,TRUE);

	//T3700
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_3700,srTxnData.srAutoloadData.anRRN,TRUE);

	//T4100
	vdUnPackToXMLData(srTxnData.srAutoloadData.ucCPUDeviceID,sizeof(srTxnData.srAutoloadData.ucCPUDeviceID),srXML,(char *)TAG_NAME_4100,strlen(TAG_NAME_4100),VALUE_NAME,TRUE);

	//T4101
	vdUnPackToXMLData(srTxnData.srAutoloadData.ucDeviceID,sizeof(srTxnData.srAutoloadData.ucDeviceID),srXML,(char *)"T4101",strlen("T4101"),VALUE_NAME,TRUE);

	//T4800
	vdUnPackToXMLData(&srTxnData.srAutoloadData.ucPurseVersionNumber,sizeof(srTxnData.srAutoloadData.ucPurseVersionNumber),srXML,(char *)TAG_NAME_4800,strlen(TAG_NAME_4800),VALUE_NAME,TRUE);

}

