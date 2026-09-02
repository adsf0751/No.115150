
#ifdef EZ_AIRLINKEDC
#include "_StdAfx.h"
#else
#include "stdAfx.h"
#endif
//#include "Global.h"

/*
1:送失敗也要+1
2:送成功,收失敗也要+1
3:送收成功,但RESPONSE CODE<>00也要+1
4:送收成功,且RESPONSE CODE==00才要+1
5:不須+1
6:加/扣成功才+1
7:其他(請詳述)"
*/

STRUCT_ISO8583_FLAG st_ISO8583Flag[MAX_ISO8583_FLAG_CNT] = {
//										NO_ADD_DATA 			NCCC_ADD_DATA			CTCB_ADD_DATA			FISC_ADD_DATA
//	bPCode		inMTI					inF11Add	inF62Add	inF11Add	inF62Add	inF11Add	inF62Add	inF11Add	inF62Add
	{"123456",	MTI_NORMAL_TXN	   ,	{{0,			0},			{0,			0},			{0,			0},			{0,			0}}},
	{"881999",	MTI_NORMAL_TXN	   ,	{{0,			0},			{1,			1},			{2,			5},			{1,			2}}},
	{"881999",	MTI_ADVICE_TXN	   ,	{{0,			0},			{1,			5},			{7,			5},			{1,			2}}},
	{"882999",	MTI_NORMAL_TXN	   ,	{{0,			0},			{1,			1},			{2,			5},			{1,			2}}},
	{"882999",	MTI_REVERSAL_TXN   ,	{{0,			0},			{5,			5},			{5,			5},			{1,			2}}},
	{"606100",	MTI_NORMAL_TXN	   ,	{{0,			0},			{1,			1},			{2,			5},			{1,			2}}},
	{"606100",	MTI_ADVICE_TXN	   ,	{{0,			0},			{1,			5},			{7,			6},			{1,			2}}},
	{"606100",	MTI_UPLOAD_TXN	   ,	{{0,			0},			{0,			0},			{2,			5},			{1,			2}}},
	{"606100",	MTI_REVERSAL_TXN   ,	{{0,			0},			{5,			5},			{5,			5},			{1,			2}}},
	{"825799",	MTI_NORMAL_TXN	   ,	{{0,			0},			{1,			1},			{2,			5},			{1,			2}}},
	{"825799",	MTI_ADVICE_TXN	   ,	{{0,			0},			{1,			5},			{7,			6},			{1,			2}}},
	{"825799",	MTI_UPLOAD_TXN	   ,	{{0,			0},			{0,			0},			{2,			5},			{1,			2}}},
	{"825799",	MTI_REVERSAL_TXN   ,	{{0,			0},			{5,			5},			{5,			5},			{1,			2}}},
	{"620061",	MTI_NORMAL_TXN	   ,	{{0,			0},			{1,			1},			{2,			5},			{1,			2}}},
	{"620061",	MTI_ADVICE_TXN	   ,	{{0,			0},			{1,			5},			{7,			6},			{1,			2}}},
	{"620061",	MTI_UPLOAD_TXN	   ,	{{0,			0},			{0,			0},			{2,			5},			{1,			2}}},
	{"620061",	MTI_REVERSAL_TXN   ,	{{0,			0},			{5,			5},			{5,			5},			{1,			2}}},
	{"801061",	MTI_NORMAL_TXN	   ,	{{0,			0},			{1,			1},			{2,			5},			{1,			2}}},
	{"801061",	MTI_ADVICE_TXN	   ,	{{0,			0},			{1,			5},			{7,			6},			{1,			2}}},
	{"801061",	MTI_UPLOAD_TXN	   ,	{{0,			0},			{0,			0},			{2,			5},			{1,			2}}},
	{"801061",	MTI_REVERSAL_TXN   ,	{{0,			0},			{5,			5},			{5,			5},			{1,			2}}},
	{"816100",	MTI_NORMAL_TXN	   ,	{{0,			0},			{1,			1},			{2,			5},			{1,			2}}},
	{"816100",	MTI_ADVICE_TXN	   ,	{{0,			0},			{1,			5},			{7,			6},			{1,			2}}},
	{"816100",	MTI_UPLOAD_TXN	   ,	{{0,			0},			{0,			0},			{2,			5},			{1,			2}}},
	{"816100",	MTI_REVERSAL_TXN   ,	{{0,			0},			{5,			5},			{5,			5},			{1,			2}}},
	{"816101",	MTI_NORMAL_TXN	   ,	{{0,			0},			{1,			1},			{2,			5},			{1,			2}}},
	{"816101",	MTI_ADVICE_TXN	   ,	{{0,			0},			{1,			5},			{7,			6},			{1,			2}}},
	{"816101",	MTI_UPLOAD_TXN	   ,	{{0,			0},			{0,			0},			{2,			5},			{1,			2}}},
	{"816101",	MTI_REVERSAL_TXN   ,	{{0,			0},			{5,			5},			{5,			5},			{1,			2}}},
	{"596100",	MTI_ADVICE_TXN	   ,	{{0,			0},			{1,			5},			{7,			5},			{1,			2}}},
	{"801066",	MTI_NORMAL_TXN	   ,	{{0,			0},			{1,			1},			{2,			5},			{1,			2}}},
	{"801066",	MTI_ADVICE_TXN	   ,	{{0,			0},			{1,			5},			{7,			6},			{1,			2}}},
	{"801066",	MTI_UPLOAD_TXN	   ,	{{0,			0},			{0,			0},			{2,			5},			{1,			2}}},
	{"801066",	MTI_REVERSAL_TXN   ,	{{0,			0},			{5,			5},			{5,			5},			{1,			2}}},
	{"900099",	MTI_NORMAL_TXN	   ,	{{0,			0},			{1,			5},			{2,			5},			{1,			2}}},
	{"900098",	MTI_NORMAL_TXN	   ,	{{0,			0},			{1,			5},			{2,			5},			{1,			2}}}
};

int inAddTcpipDataSend(STRUCT_XML_DOC *srBankXML,STRUCT_ADD_DATA *srAddData,BYTE *bSendData,BYTE *bSendBuf,int inSendLen)
{
int i,inCnt = 0,inRetVal,inCntStart1 = 0,inCntEnd1 = 0,inCntStart2 = 0,inCntEnd2 = 0;
BYTE bBuf[100],bTmp[50],bBefData[500],bAftData[500],bFieldType,bFieldAdr[10],bFieldTransType = 0x00,bTxnTagName[20];
int inBefDataLen,inAftDataLen,inFieldID,inFieldAde,inFieldDataLen;
long lnEVBef = 0L,lnEVAft = 0L;

	vdGetISO8583FlagIndex(srAddData->bPCode,srAddData->inMTI);
	strncpy((char *)srAddData->bTxnTagName,(char *)srAddData->bPCode,6);
	if(srAddData->inMTI == MTI_ADVICE_TXN)
		strcat((char *)srAddData->bTxnTagName,"_Adv");
	else if(srAddData->inMTI == MTI_REVERSAL_TXN)
		strcat((char *)srAddData->bTxnTagName,"_Rev");
	else if(srAddData->inMTI == MTI_UPLOAD_TXN)
		strcat((char *)srAddData->bTxnTagName,"_Upl");

	memset(bTxnTagName,0x00,sizeof(bTxnTagName));
	if(srTxnData.srParameter.inAdditionalTcpipData == FISC_ADD_DATA)
		memcpy(bTxnTagName,"000000",6);
	else
		memcpy((char *)bTxnTagName,srAddData->bTxnTagName,strlen((char *)srAddData->bTxnTagName));

	//Search第一層節點
	inRetVal = inXMLSearchValueByTag(srBankXML,(char *)bTxnTagName,&inCntStart1,&inCntEnd1,0);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_FLOW,"inAddTcpipDataSend Fail 2:%d(%s)(%s)",inRetVal,srAddData->bTxnTagName,bTxnTagName);
		return ICER_ERROR;
	}

	//Search BefData
	inRetVal = inXMLSearchValueByTag(srBankXML,(char *)"BefData",&inCntStart2,&inCntEnd2,inCntStart1);
	if(inRetVal < SUCCESS || inCntStart2 > inCntEnd1)
	{
		log_msg(LOG_LEVEL_FLOW,"inAddTcpipDataSend Fail 3:%d(%d)(%d)(%d)",inRetVal,inCntStart2,inCntEnd2,inCntEnd1);
		return ICER_ERROR;
	}

	memset((char *)bBefData,0x00,sizeof(bBefData));
	if(srBankXML->srXMLElement[inCntStart2].shValueLen % 2)
	{
		log_msg(LOG_LEVEL_FLOW,"inAddTcpipDataSend Fail 4:%s",srBankXML->srXMLElement[inCntStart2].chValue);
		return ICER_ERROR;
	}

	fnPack(srBankXML->srXMLElement[inCntStart2].chValue,srBankXML->srXMLElement[inCntStart2].shValueLen,bBefData);
	srAddData->inBefLen = inBefDataLen = srBankXML->srXMLElement[inCntStart2].shValueLen / 2;

	//Search AftData
	inRetVal = inXMLSearchValueByTag(srBankXML,(char *)"AftData",&inCntStart2,&inCntEnd2,inCntStart1);
	if(inRetVal < SUCCESS || inCntStart2 > inCntEnd1)
	{
		log_msg(LOG_LEVEL_FLOW,"inAddTcpipDataSend Fail 5:%d(%d)(%d)(%d)",inRetVal,inCntStart2,inCntEnd2,inCntEnd1);
		return ICER_ERROR;
	}

	memset((char *)bAftData,0x00,sizeof(bAftData));
	if(srBankXML->srXMLElement[inCntStart2].shValueLen % 2)
	{
		log_msg(LOG_LEVEL_FLOW,"inAddTcpipDataSend Fail 6:%d",strlen((char *)bAftData));
		return ICER_ERROR;
	}

	fnPack(srBankXML->srXMLElement[inCntStart2].chValue,srBankXML->srXMLElement[inCntStart2].shValueLen,bAftData);
	srAddData->inAftLen = inAftDataLen = srBankXML->srXMLElement[inCntStart2].shValueLen / 2;
	//輪尋,從inCntStart1到inCntEnd1
	for(i=inCntStart1;i<=inCntEnd1;i++)
	{
		if(!memcmp(&srBankXML->srXMLElement[i].chTag[0],"F39Adr",6))
		{
			inRetVal = inXMLFormatCheck((char *)srBankXML->srXMLElement[i].chValue,srBankXML->srXMLElement[i].shValueLen,XML_CHECK_NUMERIC);
			if(inRetVal != SUCCESS)
			{
				log_msg(LOG_LEVEL_FLOW,"inAddTcpipDataSend Fail 7(%s) :%d",srBankXML->srXMLElement[i].chValue,srBankXML->srXMLElement[i].shValueLen);
				return ICER_ERROR;
			}
			srAddData->inF39Adr = atoi((char *)srBankXML->srXMLElement[i].chValue);
			continue;
		}
		else if(!memcmp(&srBankXML->srXMLElement[i].chTag[0],"ECCAdr",6))
		{
			inRetVal = inXMLFormatCheck((char *)srBankXML->srXMLElement[i].chValue,srBankXML->srXMLElement[i].shValueLen,XML_CHECK_NUMERIC);
			if(inRetVal != SUCCESS)
			{
				log_msg(LOG_LEVEL_FLOW,"inAddTcpipDataSend Fail 8(%s) :%d",srBankXML->srXMLElement[i].chValue,srBankXML->srXMLElement[i].shValueLen);
				return ICER_ERROR;
			}
			srAddData->inECCAdr = atoi((char *)srBankXML->srXMLElement[i].chValue);
			continue;
		}
		//只要FXXAdr的Tag,其餘不處理!!
		else if(memcmp(&srBankXML->srXMLElement[i].chTag[3],"Adr",3) || srBankXML->srXMLElement[i].chTag[0] != 'F')
			continue;

		//處理inFieldID,inFieldAde,bFieldType
		inFieldID = ((srBankXML->srXMLElement[i].chTag[1] - '0') * 10) + srBankXML->srXMLElement[i].chTag[2] - '0';
		bFieldType = srBankXML->srXMLElement[i].chValue[0];
		memset((char *)bFieldAdr,0x00,sizeof(bFieldAdr));
		memcpy((char *)bFieldAdr,&srBankXML->srXMLElement[i].chValue[1],srBankXML->srXMLElement[i].shValueLen - 1);

		inRetVal = inXMLFormatCheck((char *)bFieldAdr,srBankXML->srXMLElement[i].shValueLen - 1,XML_CHECK_NUMERIC);
		if(inRetVal != SUCCESS)
		{
			log_msg(LOG_LEVEL_FLOW,"inAddTcpipDataSend Fail 9(%s) :%d",bFieldAdr,srBankXML->srXMLElement[i].shValueLen - 1);
			return ICER_ERROR;
		}
		inFieldAde = atoi((char *)bFieldAdr);

		inFieldDataLen = 3;
		bFieldTransType = 0;//Binary Pack

		memset((char *)bBuf,0x00,sizeof(bBuf));
		switch(inFieldID)
		{
			case 2:
				memset((char *)bTmp,0x00,sizeof(bTmp));
				inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_0200,(char *)bTmp,sizeof(bTmp),0);
				if(inRetVal < SUCCESS)
				{
					if(srTxnData.srParameter.inAdditionalTcpipData == FISC_ADD_DATA)
					{
						sprintf((char *)bBuf,"0000000000");
						break;
					}
					else
					{
						log_msg(LOG_LEVEL_FLOW,"inAddTcpipDataSend Fail 10:%d",inRetVal);
						continue;
					}
				}
				if(srTxnData.srParameter.inAdditionalTcpipData == CTCB_ADD_DATA || srTxnData.srParameter.inAdditionalTcpipData == FISC_ADD_DATA)
					vdPad((char *)bTmp,(char *)bBuf,10,FALSE,'0');//右靠左補0
				else
				{
					log_msg(LOG_LEVEL_FLOW,"inAddTcpipDataSend Fail 10-1:%d",srTxnData.srParameter.inAdditionalTcpipData);
					continue;
				}
				break;
			case 3:
					if(srTxnData.srParameter.inAdditionalTcpipData == NCCC_ADD_DATA)
						srAddData->inF03Adr = inFieldAde;
				continue;
			case 4:
				if(!memcmp(srAddData->bPCode,"825799",6))
					vdPad((char *)srTxnData.srIngData.anAutoloadAmount,(char *)bBuf,12,FALSE,'0');
				else
					sprintf((char *)bBuf,"%10ld00",srTxnData.srIngData.lnECCAmt);

				if(srTxnData.srParameter.inAdditionalTcpipData == NCCC_ADD_DATA)
						srAddData->inF04Adr = inFieldAde;
				break;
			case 11:
				sprintf((char *)bBuf,"%06ld",srTxnData.srIngData.ulBankSTAN);
				srAddData->inF11Adr = inFieldAde;

				if(srTxnData.srParameter.inAdditionalTcpipData == NCCC_ADD_DATA)
					srAddData->inF11Adr = inFieldAde;
				break;
			case 12:
				inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_1200,(char *)bBuf,sizeof(bBuf),0);
				if(inRetVal < SUCCESS)
				{
					log_msg(LOG_LEVEL_FLOW,"inAddTcpipDataSend Fail 11:%d",inRetVal);
					continue;
				}
				break;
			case 13:
				memset((char *)bTmp,0x00,sizeof(bTmp));
				inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_1300,(char *)bTmp,sizeof(bTmp),0);
				if(inRetVal < SUCCESS)
				{
					log_msg(LOG_LEVEL_FLOW,"inAddTcpipDataSend Fail 12:%d",inRetVal);
					continue;
				}
				if(strlen((char *)bTmp) == 8)
					memcpy((char *)bBuf,(char *)&bTmp[4],4);
				else if(strlen((char *)bTmp) == 6)
					memcpy((char *)bBuf,(char *)&bTmp[2],4);
				break;
			case 35:
				memset((char *)bTmp,0x00,sizeof(bTmp));
				inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_0211,(char *)bTmp,sizeof(bTmp),0);
				if(inRetVal < SUCCESS)
				{
					sprintf((char *)bBuf,"0000000000000000");
					//break;
				}
				vdPad((char *)bTmp,(char *)bBuf,16,FALSE,'0');//右靠左補0

				if(srTxnData.srParameter.inAdditionalTcpipData == NCCC_ADD_DATA)
				{
					strcat((char *)bBuf,"=");

					memset((char *)bTmp,0x00,sizeof(bTmp));
					inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_0200,(char *)bTmp,sizeof(bTmp),0);
					if(inRetVal < SUCCESS)
						memset((char *)&bBuf[17],0x20,17);
					else
						vdPad((char *)bTmp,(char *)&bBuf[17],17,TRUE,' ');//左靠右補空白

					bFieldTransType = 1;//ASCII
					srAddData->inF35Adr = inFieldAde;
				}
				break;
			case 37:
				if(srAddData->inMTI == MTI_ADVICE_TXN && srTxnData.srParameter.inAdditionalTcpipData == NCCC_ADD_DATA)
					sprintf((char *)bBuf,"%03ld",(srTxnData.srIngData.ulBankInvoiceNumber - 1) % 1000);
				else
					sprintf((char *)bBuf,"%03ld",srTxnData.srIngData.ulBankInvoiceNumber % 1000);
				bFieldTransType = 1;//ASCII

				if(srTxnData.srParameter.inAdditionalTcpipData == NCCC_ADD_DATA)
					srAddData->inF37Adr = inFieldAde;
				break;
			case 57:
				if(srTxnData.srParameter.inAdditionalTcpipData == NCCC_ADD_DATA)
				{
					srAddData->inF57Adr = inFieldAde;
					srAddData->inNCCCEncryptFlag = 1;
				}
				continue;
			case 58:
				inCnt = 0;
				memset((char *)bTmp,0x00,sizeof(bTmp));
				inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_3700,(char *)bTmp,sizeof(bTmp),0);
				if(inRetVal < SUCCESS)
				{
					log_msg(LOG_LEVEL_FLOW,"inAddTcpipDataSend Fail 13:%d",inRetVal);
					continue;
				}
				strcat((char *)&bBuf[inCnt],"4026");
				inCnt += 4;
				memcpy(&bBuf[inCnt],bTmp,14);//3700
				inCnt += 14;
				memset((char *)bTmp,0x00,strlen((char *)bTmp));
				inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_4100,(char *)bTmp,sizeof(bTmp),0);
				if(inRetVal < SUCCESS)
				{
					log_msg(LOG_LEVEL_FLOW,"inAddTcpipDataSend Fail 13-1:%d",inRetVal);
					continue;
				}
				memcpy(&bBuf[inCnt],bTmp,12);//4100
				inCnt += 12;
				memcpy(&bBuf[inCnt],"4116",4);
				inCnt += 4;
				memset((char *)bTmp,0x00,sizeof(bTmp));
				inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_0211,(char *)bTmp,sizeof(bTmp),0);
				if(inRetVal < SUCCESS)
					memcpy(&bBuf[inCnt],"0000000000000000",16);
				else
					memcpy(&bBuf[inCnt],bTmp,16);
				inCnt += 16;

				bFieldTransType = 1;//ASCII
				break;
			case 59:
				memset((char *)bTmp,0x00,sizeof(bTmp));
				inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_0410,(char *)bTmp,sizeof(bTmp),0);
				log_msg(LOG_LEVEL_FLOW,"F59 1(%s)",bTmp);
				if(inRetVal < SUCCESS)
					sprintf((char *)bTmp,"0");
				else
					lnEVBef = atol((char *)bTmp);

				if(bTmp[0] == '-')
				{
					bBuf[0] = '-';
					bTmp[0] = '0';
				}
				else
					bBuf[0] = ' ';
				vdPad((char *)bTmp,(char *)&bBuf[1],8,FALSE,'0');//右靠左補0
				log_msg(LOG_LEVEL_FLOW,"F59 2(%s)",bBuf);

				memset((char *)bTmp,0x00,sizeof(bTmp));
				inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_0408,(char *)bTmp,sizeof(bTmp),0);
				log_msg(LOG_LEVEL_FLOW,"F59 3(%s)",bTmp);
				if(inRetVal < SUCCESS)
				{
					lnEVAft = lnGetEVByPCode(&srXMLData,lnEVBef);
					sprintf((char *)bTmp,"%ld",lnEVAft);
				}

				if(bTmp[0] == '-')
				{
					bBuf[9] = '-';
					bTmp[0] = '0';
				}
				else
					bBuf[9] = ' ';
				vdPad((char *)bTmp,(char *)&bBuf[10],8,FALSE,'0');//右靠左補0
				log_msg(LOG_LEVEL_FLOW,"F59 4(%s)",bBuf);
				bFieldTransType = 1;//ASCII
				if(srTxnData.srParameter.inAdditionalTcpipData == NCCC_ADD_DATA)
					srAddData->inF59Adr = inFieldAde;

				break;
			case 60:
				sprintf((char *)bBuf,"%06ld",srTxnData.srIngData.ulBankSTAN);
				bFieldTransType = 1;//ASCII
				//srAddData->inF11Adr = inFieldAde;
				break;
			case 62:
				if(srAddData->inMTI == MTI_ADVICE_TXN && srTxnData.srParameter.inAdditionalTcpipData == NCCC_ADD_DATA)
					sprintf((char *)bBuf,"%06ld",srTxnData.srIngData.ulBankInvoiceNumber - 1);
				else
					sprintf((char *)bBuf,"%06ld",srTxnData.srIngData.ulBankInvoiceNumber);
				bFieldTransType = 1;//ASCII
				srAddData->inF62Adr = inFieldAde;
				break;
			case 63:
				if(srTxnData.srParameter.inAdditionalTcpipData == NCCC_ADD_DATA)
					srAddData->inF63Adr = inFieldAde;
				continue;
			default:
				log_msg(LOG_LEVEL_FLOW,"inAddTcpipDataSend Fail 14:%d",inFieldID);
				continue;
		}

		inFieldDataLen = strlen((char *)bBuf);


		if((bFieldType == '+' && inFieldAde > inBefDataLen - (inFieldDataLen / 2)) || (bFieldType == '-' && inFieldAde > inAftDataLen - (inFieldDataLen / 2)))
		{
			log_msg(LOG_LEVEL_FLOW,"inAddTcpipDataSend warning 15(%c) :%d > %d or %d",bFieldType,inFieldAde,inBefDataLen,inAftDataLen);
			continue;
		}
		else
			log_msg(LOG_LEVEL_FLOW,"inAddTcpipDataSend Debug 1(%c)(%c) :%d,%d,%d,%d",bFieldTransType,bFieldType,inFieldAde,inBefDataLen,inAftDataLen,inFieldDataLen);

		if(bFieldTransType == 0)
		{
			if(bFieldType == '+')
				fnPack((char *)bBuf,inFieldDataLen,&bBefData[inFieldAde]);
			else if(bFieldType == '-')
				fnPack((char *)bBuf,inFieldDataLen,&bAftData[inFieldAde]);
		}
		else //if(bFieldTransType == 1)
		{
			if(bFieldType == '+')
				memcpy((char *)&bBefData[inFieldAde],bBuf,inFieldDataLen);
			else
				memcpy((char *)&bAftData[inFieldAde],bBuf,inFieldDataLen);
		}
	}

	if(srAddData->inF39Adr == 0 || srAddData->inECCAdr == 0)
	{
		log_msg(LOG_LEVEL_FLOW,"inAddTcpipDataSend Fail 16(%s) :%d",bFieldAdr,srBankXML->srXMLElement[i].shValueLen - 1);
		return ICER_ERROR;
	}
	else
		log_msg(LOG_LEVEL_FLOW,"inAddTcpipDataSend 1(%d)(%d)",srAddData->inF39Adr,srAddData->inECCAdr);

	inCnt = 0;
	if(inBefDataLen > 0)
	{
		memcpy(&bSendData[0],(char *)bBefData,inBefDataLen);
		inCnt += inBefDataLen;
		{
			BYTE bBuffer[1024];
			int j;

			memset(bBuffer,0x00,sizeof(bBuffer));
			strcat((char *)bBuffer,"Bef:[");
			for(j=0;j<inBefDataLen;j++)
			{
				sprintf((char *)&bBuffer[strlen((char *)bBuffer)],"%02X",bBefData[j]);
			}
			strcat((char *)bBuffer,"]");
			log_msg(LOG_LEVEL_FLOW,(char *)bBuffer);
		}
	}

	if(srTxnData.srParameter.inAdditionalTcpipData == NCCC_ADD_DATA)
	{
		bSendData[inCnt++] = ((((inSendLen+6) / 100) / 10 ) * 16) + ((inSendLen+6) / 100) % 10;
		bSendData[inCnt++] = ((((inSendLen+6) % 100) / 10 ) * 16) + (inSendLen+6) % 10;
		memcpy(&bSendData[inCnt],"EZ01",4);
		inCnt += 4;
	}
	bSendData[inCnt++] = (((inSendLen / 100) / 10 ) * 16) + (inSendLen / 100) % 10;
	bSendData[inCnt++] = (((inSendLen % 100) / 10 ) * 16) + inSendLen % 10;
	memcpy((char *)&bSendData[inCnt],bSendBuf,inSendLen);
	inCnt += inSendLen;
	if(inAftDataLen > 0)
	{
		memcpy(&bSendData[inCnt],(char *)bAftData,inAftDataLen);
		inCnt += inAftDataLen;
		{
			BYTE bBuffer[1024];
			int j;

			memset(bBuffer,0x00,sizeof(bBuffer));
			strcat((char *)bBuffer,"Aft:[");
			for(j=0;j<inAftDataLen;j++)
			{
				sprintf((char *)&bBuffer[strlen((char *)bBuffer)],"%02X",bAftData[j]);
			}
			strcat((char *)bBuffer,"]");
			log_msg(LOG_LEVEL_FLOW,(char *)bBuffer);
		}
	}

	if(srTxnData.srParameter.inAdditionalTcpipData == FISC_ADD_DATA)
	{
		bSendData[0] = ((((inCnt - 2) / 100) / 10 ) * 16) + ((inCnt - 2) / 100) % 10;
		bSendData[1] = ((((inCnt - 2) % 100) / 10 ) * 16) + (inCnt - 2) % 10;
	}
	else
	{
		bSendData[0] = (inCnt - 2) / 256;
		bSendData[1] = (inCnt - 2) % 256;
	}

	return inCnt - inSendLen;
}

int inAddTcpipDataRecv(STRUCT_XML_DOC *srBankXML,STRUCT_ADD_DATA *srAddData,BYTE *bRecvData,BYTE *bRecvBuf,int inRecvLen)
{
int inECCDataLen = 0,inBefDataLen,inAftDataLen;
BYTE bFileData[MAX_XML_FILE_SIZE];

	log_msg(LOG_LEVEL_FLOW,"inAddTcpipDataRecv 0:%d",srAddData->inECCAdr);
	log_msg(LOG_LEVEL_FLOW,"inAddTcpipDataRecv 1:%02x%02x",bRecvData[srAddData->inECCAdr],bRecvData[srAddData->inECCAdr + 1]);
	inECCDataLen = (bRecvData[srAddData->inECCAdr] / 16 * 1000) + (bRecvData[srAddData->inECCAdr] % 16 * 100) +
				   (bRecvData[srAddData->inECCAdr + 1] / 16 * 10) + (bRecvData[srAddData->inECCAdr + 1] % 16);
	log_msg(LOG_LEVEL_FLOW,"inAddTcpipDataRecv 2:%d",inECCDataLen);
	//inECCDataLen = bRecvData[srAddData->inECCAdr] * 256 + bRecvData[srAddData->inECCAdr + 1];
	inBefDataLen = srAddData->inECCAdr;
	inAftDataLen = inRecvLen - srAddData->inECCAdr - inECCDataLen - 2;
	log_msg(LOG_LEVEL_FLOW,"inAddTcpipDataRecv 3:%d,%d",inBefDataLen,inAftDataLen);
	memcpy(bRecvBuf,&bRecvData[srAddData->inECCAdr + 2],inECCDataLen);
	if(inBefDataLen < 0 || inAftDataLen < 0 || inRecvLen < srAddData->inECCAdr + inECCDataLen)
	{
		log_msg(LOG_LEVEL_ERROR,"inAddTcpipDataRecv Fail 1:%d,%d,%d",inRecvLen,inBefDataLen,inECCDataLen);
		return ICER_ERROR;
	}

	if(inBefDataLen + inAftDataLen > 0)
	{
		memset(bFileData,0x00,sizeof(bFileData));
		if(inFileGetSize((char *)FILE2_RES,FALSE) <= SUCCESS)
			sprintf((char *)&bFileData[strlen((char *)bFileData)],"<%s>\n",TAG_TRANS_XML_HEADER);

		sprintf((char *)&bFileData[strlen((char *)bFileData)],"<%s>\n",srAddData->bTxnTagName);
		if(inBefDataLen > 0)
		{
			strcat((char *)&bFileData[strlen((char *)bFileData)],"<BefData>");
			fnUnPack(bRecvData,inBefDataLen,&bFileData[strlen((char *)bFileData)]);
			strcat((char *)&bFileData[strlen((char *)bFileData)],"</BefData>\n");
		}
		if(inAftDataLen > 0)
		{
			strcat((char *)&bFileData[strlen((char *)bFileData)],"<AftData>");
			fnUnPack(&bRecvData[2 + inECCDataLen + srAddData->inECCAdr],inAftDataLen,&bFileData[strlen((char *)bFileData)]);
			sprintf((char *)&bFileData[strlen((char *)bFileData)],"</AftData>\n");
		}
		sprintf((char *)&bFileData[strlen((char *)bFileData)],"</%s>\n",srAddData->bTxnTagName);

		inFileAppend((char *)FILE2_RES,bFileData,strlen((char *)bFileData));
	}

	if(srTxnData.srParameter.inAdditionalTcpipData == NCCC_ADD_DATA)
	{
		if(inECCDataLen <= 2)
		{
			memcpy(srTxnData.srIngData.chICERResponseCode,&bRecvData[srAddData->inF39Adr],2);
			//inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4100,(char *)TAG_NAME_3908,(BYTE *)srTxnData.srIngData.chICERResponseCode,TRUE);
			log_msg(LOG_LEVEL_ERROR,"inAddTcpipDataRecv Fail 2(%d)(%02x%02x)",srAddData->inF39Adr,bRecvData[srAddData->inF39Adr],bRecvData[srAddData->inF39Adr + 1]);
			return BANK_DECLINE_FAIL;
		}
	}
	else if(memcmp((char *)&bRecvData[srAddData->inF39Adr],"00",2))//非NCCC_ADD_DATA
	{
		memcpy(srTxnData.srIngData.chICERResponseCode,&bRecvData[srAddData->inF39Adr],2);
		//inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4100,(char *)TAG_NAME_3908,(BYTE *)srTxnData.srIngData.chICERResponseCode,TRUE);
		log_msg(LOG_LEVEL_ERROR,"inAddTcpipDataRecv Fail 3(%d)(%02x%02x)",srAddData->inF39Adr,bRecvData[srAddData->inF39Adr],bRecvData[srAddData->inF39Adr + 1]);
		return BANK_DECLINE_FAIL;
	}

	log_msg(LOG_LEVEL_FLOW,"inAddTcpipDataRecv 4:%d,%d",inBefDataLen,inAftDataLen);
	return inECCDataLen;
}

int inAddTcpipDataModify(STRUCT_ADD_DATA *srAddData,BYTE *bSendBuf,int inSendLen)
{
	BYTE bBuf[10];

	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06ld",srTxnData.srIngData.ulBankSTAN);
	fnPack((char *)bBuf,6,&bSendBuf[srAddData->inF11Adr]);

	return SUCCESS;
}

void vdGetISO8583FlagIndex(BYTE *bPCode,int inMTI)
{

	for(srTxnData.srIngData.inISO8583FlagIndex=0;srTxnData.srIngData.inISO8583FlagIndex<MAX_ISO8583_FLAG_CNT;srTxnData.srIngData.inISO8583FlagIndex++)
	{
		if(!memcmp(bPCode,st_ISO8583Flag[srTxnData.srIngData.inISO8583FlagIndex].bPCode,6) && inMTI == st_ISO8583Flag[srTxnData.srIngData.inISO8583FlagIndex].inMTI)
			return;
	}

	log_msg(LOG_LEVEL_FLOW,"inGetISO8583FlagIndex Fail :%s,%d",bPCode,inMTI);
	srTxnData.srIngData.inISO8583FlagIndex = 0;//表示失敗
}

void vdIcnBankData(int inIncType)
{
	if(srTxnData.srParameter.inAdditionalTcpipData == NO_ADD_DATA)
		return;

	if(inIncType == st_ISO8583Flag[srTxnData.srIngData.inISO8583FlagIndex].st_ISO8583BankFlag[srTxnData.srParameter.inAdditionalTcpipData].inF11AddFlag)
		srTxnData.srIngData.ulBankSTAN++;

	if(inIncType == st_ISO8583Flag[srTxnData.srIngData.inISO8583FlagIndex].st_ISO8583BankFlag[srTxnData.srParameter.inAdditionalTcpipData].inF62AddFlag)
		srTxnData.srIngData.ulBankInvoiceNumber++;
}

long lnGetEVByPCode(STRUCT_XML_DOC *srXML,long lnEVBef)
{
	long lnEVAft = 0L,lnTxnAmt = 0L;
	int inRetVal = 0;
	BYTE bPCode[10],bTxnAmt[10];

	memset((char *)bPCode,0x00,sizeof(bPCode));
	memset((char *)bTxnAmt,0x00,sizeof(bTxnAmt));
	inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_0300,(char *)bPCode,sizeof(bPCode),0);
	if(inRetVal > SUCCESS)
	{
		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_0400,(char *)bTxnAmt,sizeof(bTxnAmt),0);
		if(inRetVal > SUCCESS)
			lnTxnAmt = atol((char *)bTxnAmt);
		if(!memcmp(bPCode,"811899",6) || !memcmp(bPCode,"816399",6))//加值取消 & 購貨
			lnEVAft = lnEVBef - lnTxnAmt;
		else
			lnEVAft = lnEVBef + lnTxnAmt;
	}
	else
		log_msg(LOG_LEVEL_ERROR,"lnGetEVByPCode Fail 1:%d",inRetVal);

	return lnEVAft;
}

int inNCCCEncryptionData(STRUCT_ADD_DATA *srAddData, BYTE *bData)
{
	unsigned int fieldLen, totalDataLen, orgECCDataLen, zipECCDataLen;
	int inRetVal, MACFlag = 0;
	BYTE tmpBuf[10 * 1024];
	BYTE field37Data[12];
	BYTE field57Data[4];
	BYTE MACData[8];
	unsigned int msgType=0;
	BYTE keyIndex = 0;
	int  field59Pos = srAddData->inF59Adr;
	int  field63Pos = srAddData->inF63Adr;

	totalDataLen = 0;
	log_msg(LOG_LEVEL_FLOW,"inNCCCEncryptionData 1,%s", srAddData->bPCode);
	log_msg(LOG_LEVEL_FLOW,"inNCCCEncryptionData 2,%d, %d, %d, %d, %d, %d", srAddData->inF03Adr, srAddData->inF04Adr, srAddData->inF11Adr, srAddData->inF35Adr,  srAddData->inF59Adr, srAddData->inF63Adr);
	// 結帳交易
	//if (!memcmp(srAddData->bPCode, "900098", 6) || !memcmp(srAddData->bPCode, "900099", 6))
	//	transType = 1;

	orgECCDataLen = (((bData[srAddData->inBefLen] & 0xF0) >> 4) * 10 + (bData[srAddData->inBefLen] & 0x0F)) * 100 + 
			         ((bData[srAddData->inBefLen+1] & 0xF0) >> 4) * 10 + (bData[srAddData->inBefLen+1] & 0x0F);

	msgType = bData[7] * 256 + bData[8]; 
	log_msg(LOG_LEVEL_ERROR,"inNCCCEncryptionData 3,%d, %d, %d, %d, %d, %d, %x",srAddData->inF35Adr, srAddData->inF37Adr, srAddData->inF57Adr, srAddData->inBefLen, srAddData->inAftLen, orgECCDataLen, msgType);
	
	field59Pos += srAddData->inBefLen + 2 + orgECCDataLen;
	field63Pos += srAddData->inBefLen + 2 + orgECCDataLen;	

	MACFlag = ECC_NCCC_MAC(msgType, bData, srAddData->inF03Adr, srAddData->inF04Adr, srAddData->inF11Adr, srAddData->inF35Adr, field59Pos, field63Pos, MACData);

	if (MACFlag)
	{
		bData[16] |= 0x01;
		totalDataLen = bData[0] * 256 + bData[1];
		memcpy(bData+totalDataLen+2, MACData, 8);
		bData[0] = (totalDataLen +8) / 256;
		bData[1] = (totalDataLen +8) % 256;
		totalDataLen += 8 + 2;
		log_msg(LOG_LEVEL_FLOW,"inNCCCEncryptionData 4,%d", totalDataLen);
	}
	
	if (srAddData->inNCCCEncryptFlag)
	{
		log_msg(LOG_LEVEL_FLOW,"inNCCCEncryptionData 5,%d, %d, %d, %d, %d",srAddData->inF37Adr, srAddData->inF35Adr, srAddData->inF57Adr, srAddData->inBefLen, srAddData->inAftLen);
		//orgECCDataLen = (((bData[srAddData->inBefLen] & 0xF0) >> 4) * 10 + (bData[srAddData->inBefLen] & 0x0F)) * 100 + 
		//	              	((bData[srAddData->inBefLen+1] & 0xF0) >> 4) * 10 + (bData[srAddData->inBefLen+1] & 0x0F);
		//log_msg(LOG_LEVEL_ERROR,"inNCCCEncryptionData 5,%d",orgECCDataLen);
		zipECCDataLen = orgECCDataLen;
		fieldLen = 34;
		// ICERAPI.REQ2 Field 37 index offset 8 bytes
		memcpy(field37Data, bData+srAddData->inF37Adr - 8, 12);
		memcpy(tmpBuf, bData+srAddData->inF35Adr, fieldLen);
		log_msg_debug(LOG_LEVEL_FLOW,FALSE,(char *)"inNCCCEncryptionData 6", 22, field37Data, 12);
		log_msg_debug(LOG_LEVEL_FLOW,FALSE,(char *)"inNCCCEncryptionData 7", 22, tmpBuf, 34);
		// Field 35 Encrypt
		inRetVal = ECC_NCCC_Encryption_TSAM(tmpBuf, &fieldLen, 35, field37Data, field57Data, &keyIndex);
		if (inRetVal != SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inNCCCEncryptionData 8,%d", inRetVal);
			return totalDataLen;
		}
		log_msg_debug(LOG_LEVEL_FLOW,FALSE,(char *)"inNCCCEncryptionData 9", 22, tmpBuf, fieldLen);
		log_msg_debug(LOG_LEVEL_FLOW,FALSE,(char *)"inNCCCEncryptionData 10", 23, field57Data, 4);
		memcpy(bData+srAddData->inF35Adr, tmpBuf, 34);
		memcpy(bData+srAddData->inBefLen+(orgECCDataLen+2)+srAddData->inF57Adr, field57Data, 4);
		// Field 56 Encrypt
		memset(tmpBuf, 0x00, sizeof(tmpBuf));
		memset(field57Data, 0x00, sizeof(field57Data));
		memcpy(tmpBuf, bData+srAddData->inBefLen+2, orgECCDataLen);
		inRetVal = ECC_NCCC_Encryption_TSAM(tmpBuf, &zipECCDataLen, 56, field37Data, field57Data, &keyIndex);
		
		if (inRetVal != SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inNCCCEncryptionData 11,%d", inRetVal);
			return totalDataLen;
		}
		if (MACFlag)
			srAddData->inAftLen += 8;
		bData[6] = keyIndex;
		memcpy(bData+srAddData->inBefLen+(orgECCDataLen+2)+srAddData->inF57Adr+4, field57Data, 4);
		bData[srAddData->inBefLen] = (((zipECCDataLen / 100) / 10 ) * 16) + (zipECCDataLen / 100) % 10;
		bData[srAddData->inBefLen+1] = (((zipECCDataLen % 100) / 10 ) * 16) + zipECCDataLen % 10;
		memcpy(tmpBuf+ zipECCDataLen, bData+srAddData->inBefLen+orgECCDataLen+2, srAddData->inAftLen);
		memcpy(bData+srAddData->inBefLen+2, tmpBuf, zipECCDataLen+srAddData->inAftLen);
		totalDataLen = srAddData->inBefLen + zipECCDataLen+2 +srAddData->inAftLen;
		log_msg(LOG_LEVEL_FLOW,(char *)"inNCCCEncryptionData 12, %d, %d", totalDataLen, zipECCDataLen);
		log_msg_debug(LOG_LEVEL_FLOW,FALSE,(char *)"inNCCCEncryptionData 13", 23, tmpBuf, 10);
		bData[0] = (totalDataLen - 2) / 256;
		bData[1] = (totalDataLen - 2) % 256;
	}

	return totalDataLen;

}

