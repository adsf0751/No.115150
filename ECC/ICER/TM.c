
#ifdef EZ_AIRLINKEDC
#include "_stdAfx.h"
#else
#include "stdAfx.h"
#endif
//#include "Global.h"

BYTE gsBlackListVersion[30];

void vdBuildMessageType(int inTxnType,BYTE *cOut)
{

	switch(inTxnType)
	{
		//case DLL_RESET_OFF:
		case DLL_RESET:
			memcpy(cOut,"0800",4);
			break;
		case DLL_SIGNON:
			memcpy(cOut,"0801",4);
			break;
		case DLL_ADD_VALUE:
		case DLL_ADD_VALUE1:
		case DLL_ADD_VALUE2:
		//case API_PPR_CARDSALE:
		//case API_PPR_CANCELTXN:
		case DLL_TX_REFUND:
		//case API_PPR_ADDVALUE:
		case DLL_TX_ADD_VALUE:
		case DLL_AUTOLOAD_ENABLE:
		//case API_PPR_TXREFUND:
		//case API_PPR_AUTOLOADENABLE:
		case DLL_AUTOLOAD:
		//case DLL_STUDENT_SET_VALUE:
		//case DLL_REFUND_CARD:
		case DLL_VOID_TXN:
		case DLL_ACCUNT_LINK:
		case DLL_S_SET_VALUE:
			memcpy(cOut,"0100",4);
			break;
		case DLL_ADD_VALUE_AUTH:
		case DLL_REFUND_CARD_AUTH:
		case DLL_ADD_VALUE_AUTH1:
		//case API_PPR_AUTHCARDSALE:
		case DLL_TX_REFUND_AUTH:
		//case API_PPR_AUTHADDVALUE:
		case DLL_TX_ADD_VALUE_AUTH:
		case DLL_AUTOLOAD_ENABLE_AUTH:
		//case API_PPR_AUTHTXREFUND:
		//case API_PPR_AUTHAUTOLOADENABLE:
		case DLL_AUTOLOAD_AUTH:
		//case DLL_STUDENT_SET_VALUE_AUTH:
		//case DLL_REFUND_CARD_AUTH:
		case DLL_ACCUNT_LINK_AUTH:
		case DLL_S_SET_VALUE_AUTH:
			memcpy(cOut,"0101",4);
			break;
		case DLL_DEDUCT_VALUE:
		case DLL_DEDUCT_VALUE1:
		case DDL_DEDUCT_VALUE2:
			memcpy(cOut,"0302",4);
			break;
		case DLL_SET_VALUE:
		case DLL_SET_VALUE_AUTH:
			memcpy(cOut,"0302",4);
			break;
		case DLL_MATCH:
			memcpy(cOut,"0800",4);
			break;
		case DLL_MATCH_AUTH:
			memcpy(cOut,"0801",4);
			break;
	}
}

void vdBuildProcessingCode(int inTxnType,BYTE ucMsgType,BYTE ucSubType,BYTE ucPersonalProfile,BYTE *cOut)
{

	switch(inTxnType)
	{
		//case DLL_RESET_OFF:
		case DLL_RESET:
		case DLL_SIGNON:
			memcpy(cOut,"881999",6);
			break;
		case DLL_MATCH:
		case DLL_MATCH_AUTH:
			memcpy(cOut,"882999",6);
			break;
		case DLL_ADD_VALUE:
		case DLL_ADD_VALUE_AUTH:
		case DLL_REFUND_CARD_AUTH:
		//case API_PPR_CARDSALE:
		//case API_PPR_CANCELTXN:
		//case API_PPR_AUTHCARDSALE:
		case DLL_AUTOLOAD_AUTH:
		case DLL_VOID_TXN:
		case DLL_AUTOLOAD:
			if(ucMsgType == 0x02)//加值
			{
				if(ucSubType == 0x0A)
					memcpy(cOut,"823899",6);//購貨取消
				else if(ucSubType == 0x0B)
					memcpy(cOut,"851999",6);//退貨
				else if(ucSubType == 0x0C)
					memcpy(cOut,"839799",6);//餘加
				else if(ucSubType == 0x30)
				{
					if(ucPersonalProfile >= 0x01 && ucPersonalProfile <= 0x03)
						memcpy(cOut,"841799",6);//社福卡現金加值
					else
						memcpy(cOut,"811799",6);//現金加值
				}
				else if(ucSubType == 0x40)
				{
					// 社福卡自動加值
					if(ucPersonalProfile >= 0x01 && ucPersonalProfile <= 0x03)
						memcpy(cOut,"845799",6);//社福卡自動加值
					else
						memcpy(cOut,"825799",6);//自動加值
				}
				else if(ucSubType == 0x34)//展期加值 > 二代後台加值
					memcpy(cOut,"816799",6);
			}
			else if(ucMsgType == 0x07)//售卡
				memcpy(cOut,"851799",6);
			else if(ucMsgType == 0x09)//售卡取消
				memcpy(cOut,"851899",6);
			else if(ucMsgType == 0x0B)//加值取消
				memcpy(cOut,"811899",6);
			else if(ucMsgType == 0x05)//退卡
				memcpy(cOut,"861799",6);
			break;
		case DLL_DEDUCT_VALUE:
		case DDL_DEDUCT_VALUE2:
			memcpy(cOut,"816399",6);
			break;
		case DLL_DEDUCT_VALUE1:
			// 後台加值查詢
			memcpy(cOut,"810199",6);
			break;
		case DLL_DEDUCT_VALUE_AUTH://購貨(批次)
			if(srTxnData.srIngData.lnECCAmt1 + srTxnData.srIngData.lnECCAmt2 + srTxnData.srIngData.lnECCAmt3  + srTxnData.srIngData.lnECCAmt4 + srTxnData.srIngData.lnECCAmt5 > 0L)
				memcpy(cOut,"819599",6);//代收售購貨
			else
				memcpy(cOut,"811599",6);
			break;
		/*case API_PR_SETVALUE:
		case API_PR_AUTHSETVALUE:
			memcpy(cOut,"810399",6);//展期
			break;
		case API_PR_TXREFUND:
		case API_PR_AUTHTXREFUND:
			memcpy(cOut,"862799",6);//餘退
			break;
		case API_PPR_ADDVALUE:
		case API_PPR_AUTHADDVALUE:
			memcpy(cOut,"824799",6);//自動加值
			break;*/
		case DLL_TX_ADD_VALUE:
		case DLL_TX_ADD_VALUE_AUTH:
			memcpy(cOut,"838799",6);//餘加
			break;
		/*case API_PR_AUTOLOADENABLE:
		case API_PR_AUTHAUTOLOADENABLE:
			memcpy(cOut,"813799",6);//自動加值開啟
			break;*/
		case DLL_SET_VALUE:
		case DLL_SET_VALUE_AUTH:
			memcpy(cOut,"811399",6);//展期
			break;
		case DLL_TX_REFUND:
		case DLL_TX_REFUND_AUTH:
			memcpy(cOut,"863799",6);//餘退
			break;
		case DLL_AUTOLOAD_ENABLE:
		case DLL_AUTOLOAD_ENABLE_AUTH:
		case DLL_ACCUNT_LINK:
		case DLL_ACCUNT_LINK_AUTH:
			if(ucSubType == 0x01)
				memcpy(cOut,"817799",6);//Accunt Link
			else
				memcpy(cOut,"814799",6);//自動加值開啟
			break;
		case DLL_S_SET_VALUE:
			//log_msg(LOG_LEVEL_FLOW, "vdBuildProcessingCode in TransType = %d", srTxnData.srIngData.inTransType);
			//if (srTxnData.srIngData.inTransType == TXN_ECC_EXTEND_VALID)	//20年展期
			//	memcpy(cOut, "811499", 6);
			//else
			memcpy(cOut,"813399",6);//學生票展期 or 20年展期
			break;
		/*case DLL_REFUND_CARD:
			memcpy(cOut,"864799",6);//學生票展期
			break;*/
		case DLL_VERIFY_HOST_CRYPT:
		case DLL_QR_VERIFY_HOST_CRYPT:
			memcpy(cOut,srTxnData.srIngData.chTMProcessCode,6);//數位付交易
			break;
		case DLL_ADD_VALUE1:
		case DLL_ADD_VALUE_AUTH1:
			memcpy(cOut,"857999",6);//代收售退貨
			break;
		default:
			memcpy(cOut,"999999",6);//未定義
			break;
	}
}

void vdCountEVBefore(BYTE *bAmt,int inAmtSize,BYTE *bEV,int inEVSize,BYTE *bEVBefore,int inEVBeforeSize,BYTE ucMsgType)
{
int inSize;
long lnAmt = 0L,lnEV = 0L,lnEVBefore = 0L;
BYTE bBuf[10],bTmp[10];

	memset(bTmp,0x00,sizeof(bTmp));
	if(bAmt[inAmtSize - 1] >= 0x80)
		memset(bTmp,0xFF,sizeof(long));
	memcpy(bTmp,bAmt,inAmtSize);
  #ifdef ECC_BIG_ENDIAN
	vdChangeEndian(bTmp,bTmp,sizeof(long));
  #else
	memcpy((BYTE *)&lnAmt,bTmp,sizeof(long));
  #endif

	memset(bTmp,0x00,sizeof(bTmp));
	if(bEV[inEVSize - 1] >= 0x80)
		memset(bTmp,0xFF,sizeof(long));
	memcpy(bTmp,bEV,inEVSize);
  #ifdef ECC_BIG_ENDIAN
	vdChangeEndian(bTmp,bTmp,sizeof(long));
  #else
	memcpy((BYTE *)&lnEV,bTmp,sizeof(long));
  #endif
	if(ucMsgType == 0x02)//加值類
		lnEVBefore = lnEV - lnAmt;
	else
		lnEVBefore = lnEV + lnAmt;
//	if(fCreditFlag == TRUE)
//		lnEVBefore = lnEV - lnAmt;
//	else //if(fCreditFlag == FALSE)
//		lnEVBefore = lnEV + lnAmt;

	memset(bBuf,0x00,sizeof(bBuf));
	inSize = sprintf((char *)bBuf,"%ld",lnEVBefore);
	memset(bEVBefore,' ',inEVBeforeSize - inSize);
	memcpy(&bEVBefore[inEVBeforeSize - inSize],bBuf,inSize);
}

BOOL fnASCTOBIN(BYTE *OutData,BYTE ASCData[],int sizex,int OffsetBit,int mode)
{
	//tmp = (unsigned char *)ECC_calloc((size_t)sizex,sizeof(unsigned char));
	unsigned char tmp[20] ;
	unsigned long ulTmp;
	int i;

	memset(tmp,0,sizeof(tmp));
	memcpy(tmp,ASCData,sizex);
	ulTmp = strtoul((char *)tmp,(char **)NULL,mode);

	for(i=0;i<OffsetBit;i++)
	{
		OutData[i] = (BYTE )(ulTmp >> (i * 8));
	}

#if READER_MANUFACTURERS==WINDOWS_CE_API
	return (char)ulTmp;
#else
	return ulTmp;
#endif
}

void vdUnPackAmt2ByteTo3Byte(BYTE *bIn2Amt,BYTE *bOutAmt)
{
BYTE bBuf[5];

	memset(bBuf,0x00,sizeof(bBuf));
	memcpy(bBuf,bIn2Amt,2);
	if(bIn2Amt[1] >= 0x80)
		bBuf[2] = 0xFF;
	fnUnPack(bBuf,3,bOutAmt);
}

void vdAmt2ByteTo3Byte(BYTE *bIn2Amt,BYTE *bOut3Amt)
{

	memcpy(bOut3Amt,bIn2Amt,2);
	if(bIn2Amt[1] >= 0x80)
		bOut3Amt[2] = 0xFF;
}

void fngetDosDate(BYTE *bDosDate,BYTE *bOutDate)
{
	int  year= 0x00,mon= 0x00,day= 0x00;

	day = (bDosDate[0] & 0x1F);
	mon = ((bDosDate[0] >> 5) & 0x07) + ((bDosDate[1] & 0x01) << 3);
	year = bDosDate[1] >> 1;
	sprintf((char *)bOutDate,"%04d%02d%02d",year + 1980,mon,day);

}

void fnBINTODEVASC(BYTE *BINData,BYTE *ASCDATA,int ASCSize,char fPurseVersion)
{
char buffer[20];
int length= 0x00 ;
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
UINT64  lTmp= 0L,lTmp2 = 0L;
//int i= 0x00;

	memset(buffer,0,sizeof(buffer));
	if(fPurseVersion != LEVEL2)
		lTmp=(BINData[0])+((BINData[1]&0x0F)*256)+((BINData[1]>>4)*10000)+(BINData[2]*1000000);
	else
	{
//		lTmp=(BINData[0]+(BINData[1]*256))+(BINData[2]*100000)+((BINData[3]+(BINData[4]*256)+(BINData[5]*256*256))*100000000);
		lTmp=(BINData[0]+(BINData[1]*256));
		lTmp += (BINData[2]*100000);
		lTmp2 = (BINData[3]+(BINData[4]*256)+(BINData[5]*256*256));
		lTmp2 *= 100000000;
		lTmp += lTmp2;
	}
	#if READER_MANUFACTURERS==WINDOWS_API
	_i64toa( lTmp, buffer, 10 );
	#endif
	#if READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
	sprintf((char *)buffer, "%llu",lTmp);
	#endif
#else

	memset(buffer,0,sizeof(buffer));
	if(fPurseVersion != LEVEL2)
		sprintf(buffer,"%03d%02d%04d",BINData[2],(BINData[1]>>4),(BINData[0])+((BINData[1]&0x0F)*256));
	else
		sprintf(buffer,"%08ld%03d%05ld",(long)(BINData[3]+(BINData[4]*256)+(BINData[5]*256*256)),BINData[2],(long)(BINData[0]+(BINData[1]*256)));

#endif
	length=strlen(buffer);
	if(fPurseVersion != LEVEL2)
	{
		if(ASCSize > 10)
		{
			vdPad(buffer,(char *)ASCDATA,10,FALSE,'0');//ASCDATA的左邊10碼先右靠左補0
			memset(&ASCDATA[10],' ',ASCSize - 10);//ASCDATA的後10碼補空白
		}
		else
			vdPad(buffer,(char *)ASCDATA,ASCSize,FALSE,'0');//ASCDATA的左邊10碼先右靠左補0
		//if(ASCSize >= 10)
		//	memset(&ASCDATA[10],' ',ASCSize - 10);//ASCDATA的後10碼補空白
	}
	else
		vdPad(buffer,(char *)ASCDATA,16,FALSE,'0');//ASCDATA右靠左補0
}

unsigned long fngetUnixTimeCnt(BYTE *OutDate,BYTE *date)
{
	const char init_mdays[]={31,28,31,30,31,30,31,31,30,31,30,31};
	int  year= 0x00,mon= 0x00,day= 0x00,hour= 0x00,min= 0x00,sec= 0x00;
	char temp[14 + 1];//不加一會有問題!!
	unsigned long tcnt= 0x00;//1.0.5.2
	int  i= 0x00;
	char mdays[12];

	memset(temp,0,sizeof(temp));
	memset(mdays,0,sizeof(mdays));
	memcpy(mdays,init_mdays,12);
	memcpy(temp,date,14);
	sec = atoi(&temp[12]);
	temp[12] = 0;
	min = atoi(&temp[10]);
	temp[10] = 0;
	hour = atoi(&temp[8]);
	temp[8] = 0;
	day = atoi(&temp[6]);
	temp[6] = 0;
	mon = atoi(&temp[4]);
	temp[4] = 0;
	year = atoi(temp);

	tcnt = 0;
	for(i=1970;i<year;i++)
	{

		tcnt += (86400 * 365);//YEAR
		if(i/4*4 == i)
		{
			tcnt += 86400;//閏年
		}
	}
	if(year/4*4 == year){
		mdays[1] = 29;
	}
	for(i=1;i<mon;i++)
		tcnt += (mdays[i-1]*86400);
	tcnt += ((day-1)*86400);
	tcnt += (hour * 3600);
	tcnt += (min * 60);
	tcnt += sec;

/*#ifdef ECC_BIG_ENDIAN
	OutDate[3] = (BYTE )tcnt;
	OutDate[2] = (BYTE )(tcnt >> 8);
	OutDate[1] = (BYTE )(tcnt >> 16);
	OutDate[0] = (BYTE )(tcnt >> 24);
#else*/
	OutDate[0] = (BYTE )tcnt;
	OutDate[1] = (BYTE )(tcnt >> 8);
	OutDate[2] = (BYTE )(tcnt >> 16);
	OutDate[3] = (BYTE )(tcnt >> 24);
//#endif

  	return tcnt;
}

void fngetDosDateCnt(BYTE *date,BYTE *DosDate)
{
	int  year= 0x00,mon= 0x00,day= 0x00;
	char temp[10];

	if(!memcmp(date,"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",14) ||
	   !memcmp(date,"19700101",8))
	{
		memset(DosDate,0x00,2);
		return;
	}

	memset(temp,0,sizeof(temp));
	//memset(DosDate,0,sizeof(DosDate));
	memset(DosDate,0, 2);
	memcpy(temp,date,8);
	day = atoi(&temp[6]);
	temp[6] = 0;
	mon = atoi(&temp[4]);
	temp[4] = 0;
	year = atoi(temp);
	if(year >= 1980)
		year -= 1980;
	else
		year = 0;

	DosDate[1] = ((year & 0x7F) << 1) | ((mon & 0x08) >> 3);
	DosDate[0] = ((mon & 0x07) << 5) | (day & 0x1F);
	//return (year * (2 << 8)) + ((0x00 | mon) * 32) + (0x00 | day);

}

void vdBuildF37_SAM_SN(BYTE ucPurseVersionNumber,BYTE *ucOutData,BYTE *ucInSAM_SN,BYTE *ucCardID)
{
	if(ucPurseVersionNumber == MIFARE)
        memcpy(ucOutData,ucInSAM_SN,4);
    else
        memcpy(ucOutData,ucCardID,4);
}

void vdBuildF66_CTC(BYTE ucPurseVersionNumber,BYTE *ucOutData,BYTE *ucCTC,BYTE *ucDeviceID)
{
	if(ucPurseVersionNumber == MIFARE)
        memcpy(ucOutData,ucDeviceID,3);
    else
        memcpy(ucOutData,ucCTC,3);
}

int inBuildResetData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
Reset_TM_Out *APIOut = (Reset_TM_Out *)API_Out;
Reset_APDU_In *DongleIn = (Reset_APDU_In *)Dongle_In;
Reset_APDU_Out *DongleOut = (Reset_APDU_Out *)Dongle_Out;


	inTMOutLen = sizeof(Reset_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",inTMOutLen - 7);//Data Len
	vdBuildProcessingCode(inTxnType,0x00,0x00,0x00,APIOut->ucProcessignCode);//Processing Code
	vdBuildBLCVersion(APIOut->ucBLVersion);
	//fnASCTOBIN(APIOut->ucBLVersion,gsBlackListVersion,5,sizeof(APIOut->ucBLVersion),DECIMAL);//Black List Version
//	inCnt++;//Msg Type
//	inCnt++;//Sub Type
	memcpy(APIOut->ucDeviceID,DongleOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	APIOut->ucSPID = DongleOut->ucDeviceID[2];//Service Provider
	memcpy(APIOut->ucTxnDateTime,DongleIn->ucTxnDateTime,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	memcpy(APIOut->ucTMID,DongleIn->ucTMID,sizeof(APIOut->ucTMID));//TM ID
	memcpy(APIOut->ucTMTxnDateTime,DongleIn->ucTMTxnDateTime,sizeof(APIOut->ucTMTxnDateTime));//TM Date Time
	memcpy(APIOut->ucTMSerialNumber,DongleIn->ucTMSerialNumber,sizeof(APIOut->ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->ucTMAgentNumber,DongleIn->ucTMAgentNumber,sizeof(APIOut->ucTMAgentNumber));//TM Agent Number
	memcpy(APIOut->ucSTAC,DongleOut->ucSTAC,sizeof(APIOut->ucSTAC));//S-TAC
	APIOut->ucKeyVersion = DongleOut->ucSAMKeyVersion;//Key Version
	memcpy(APIOut->ucSAMID,DongleOut->ucSAMID,sizeof(APIOut->ucSAMID));//SAM ID
	memcpy(APIOut->ucSAMSN,DongleOut->ucSAMSN,sizeof(APIOut->ucSAMSN));//SAM SN
	memcpy(APIOut->ucSAMCRN,DongleOut->ucSAMCRN,sizeof(APIOut->ucSAMCRN));//SAM CRN
	memcpy(APIOut->ucReaderFWVersion,DongleOut->ucReaderFWVersion,sizeof(APIOut->ucReaderFWVersion));//Reader Firmward Version
	memcpy(APIOut->ucNetManagementWorkCode,NETWORK_MANAGEMENT_CODE,sizeof(APIOut->ucNetManagementWorkCode));//Network Management Code
	APIOut->stTermHostInfo_t.ucOneDayQuotaFlag = DongleOut->stSAMParameterInfo_t.bOneDayQuotaFlag0 |
						    (DongleOut->stSAMParameterInfo_t.bOneDayQuotaFlag1 << 4);//One Day Quota Flag
	memcpy(APIOut->stTermHostInfo_t.ucOneDayQuota,DongleOut->stSAMParameterInfo_t.ucOneDayQuota,sizeof(APIOut->stTermHostInfo_t.ucOneDayQuota));//One Day Quota
	APIOut->stTermHostInfo_t.ucOnceQuotaFlag = DongleOut->stSAMParameterInfo_t.bOnceQuotaFlag2;//Once Quota Flag
	memcpy(APIOut->stTermHostInfo_t.ucOnceQuota,DongleOut->stSAMParameterInfo_t.ucOnceQuota,sizeof(APIOut->stTermHostInfo_t.ucOnceQuota));//Once Quota
	APIOut->stTermHostInfo_t.ucCheckEVFlag = DongleOut->stSAMParameterInfo_t.bCheckEVFlag6;//Check EV Flag
	APIOut->stTermHostInfo_t.ucAddQuotaFlag = DongleOut->stSAMParameterInfo_t.ucAddQuotaFlag;//Add Quota Flag
	memcpy(APIOut->stTermHostInfo_t.ucAddQuota,DongleOut->stSAMParameterInfo_t.ucAddQuota,sizeof(APIOut->stTermHostInfo_t.ucAddQuota));//Add Quota
	APIOut->stTermHostInfo_t.ucCheckDeductFlag = DongleOut->stSAMParameterInfo_t.bCheckDeductFlag3;//Check Debit Flag
	memcpy(APIOut->stTermHostInfo_t.ucCheckDeductValue,DongleOut->stSAMParameterInfo_t.ucCheckDeductValue,sizeof(APIOut->stTermHostInfo_t.ucCheckDeductValue));//Check Debit
	APIOut->stTermHostInfo_t.ucDeductLimitFlag = DongleOut->stSAMParameterInfo_t.bDeductLimitFlag7;//Deduct Limit Flag
	//vdGetAPIVersionInfo(APIOut->stTermHostInfo_t.ucAPIVersion);
	memcpy(APIOut->stTermHostInfo_t.ucAPIVersion,API_VERSION_R6,3);//API Version
	APIOut->stTermHostInfo_t.ucAPIVersion[3] = 0x00;
#ifdef MARKET_7_11
	APIOut->stTermHostInfo_t.ucAPIVersion[3] = 0x01;
#endif
//	inLen = sizeof(APIOut->ucTermHostInfo_t.ucRFU);//RFU
	memcpy(APIOut->stTermParaInfo_t.ucRemainderAddQuota,DongleOut->ucRemainderAddQuota,sizeof(APIOut->stTermParaInfo_t.ucRemainderAddQuota));//Remainder Add Quota
	memcpy(APIOut->stTermParaInfo_t.ucDeMAC,DongleOut->ucDeMAC,sizeof(APIOut->stTermParaInfo_t.ucDeMAC));//De MAC
//	inLen = sizeof(APIOut->stTermParaInfo_t.ucRFU);//RFU
//	inLen = sizeof(APIOut->);//CPU Card ID
//	inLen = sizeof(APIOut->);//CPU Txn Amt
	memcpy(APIOut->ucCPUDeviceID,DongleOut->ucCPUDeviceID,sizeof(APIOut->ucCPUDeviceID));//CPU Device ID
	memcpy(APIOut->ucCPUSPID,&DongleOut->ucCPUDeviceID[3],sizeof(APIOut->ucCPUSPID));//CPU SP ID
//	inLen = sizeof(APIOut->);//CPU EV Before Txn
	memcpy(APIOut->ucCPUSAMID,DongleOut->ucCPUSAMID,sizeof(APIOut->ucCPUSAMID));//CPU SAM ID
	memcpy(APIOut->ucSTC,DongleOut->ucSTC,sizeof(APIOut->ucSTC));//CPU SAM Txn Cnt
	APIOut->stCPUSAMInfo_t.ucSAMVersion = DongleOut->ucSAMVersion;//SAM Version Number
	memcpy(APIOut->stCPUSAMInfo_t.ucSAMUsageControl,DongleOut->ucSAMUsageControl,sizeof(APIOut->stCPUSAMInfo_t.ucSAMUsageControl));//SAM Usage Control
	APIOut->stCPUSAMInfo_t.ucSAMAdminKVN = DongleOut->ucSAMAdminKVN;//SAM Admin KVN
	APIOut->stCPUSAMInfo_t.ucSAMIssuerKVN = DongleOut->ucSAMIssuerKVN;//SAM Issuer KVN
	memcpy(APIOut->stCPUSAMInfo_t.ucTagListTable,DongleOut->ucTagListTable,sizeof(APIOut->stCPUSAMInfo_t.ucTagListTable));//SAM Tag List Table
	memcpy(APIOut->stCPUSAMInfo_t.ucSAMIssuerSpecData,DongleOut->ucSAMIssuerSpecData,sizeof(APIOut->stCPUSAMInfo_t.ucSAMIssuerSpecData));//SAM Issuer Spec Data
	memcpy(APIOut->ucRSAM,DongleOut->ucRSAM,sizeof(APIOut->ucRSAM));//CPU RSAM
	memcpy(APIOut->ucRHOST,DongleOut->ucRHOST,sizeof(APIOut->ucRHOST));//CPU RHOST
	memcpy(APIOut->ucAuthCreditLimit,DongleOut->ucAuthCreditLimit,sizeof(APIOut->ucAuthCreditLimit));//CPU Auth Credit Limit
	memcpy(APIOut->ucAuthCreditBalance,DongleOut->ucAuthCreditBalance,sizeof(APIOut->ucAuthCreditBalance));//CPU Auth Credit Balance
	memcpy(APIOut->ucAuthCreditCumulative,DongleOut->ucAuthCreditCumulative,sizeof(APIOut->ucAuthCreditCumulative));//CPU Auth Credit Cumulative
	memcpy(APIOut->ucAuthCancelCreditCumulative,DongleOut->ucAuthCancelCreditCumulative,sizeof(APIOut->ucAuthCancelCreditCumulative));//CPU Auth Cancel Credit Cumulative
	memcpy(APIOut->ucSingleCreditTxnAmtLimit,DongleOut->ucSingleCreditTxnAmtLimit,sizeof(APIOut->ucSingleCreditTxnAmtLimit));//CPU Single Credit Txn Amt Limit
	memcpy(APIOut->ucTMLocationID,DongleIn->ucTMLocationID,sizeof(APIOut->ucTMLocationID));//CPU TM Location ID
	memcpy(APIOut->ucSATOKEN,DongleOut->ucSATOKEN,sizeof(APIOut->ucSATOKEN));//CPU SA TOKEN
	memcpy(APIOut->stLastSignOnInfo_t.ucPreCPUDeviceID,DongleOut->stLastSignOnInfo_t.ucPreCPUDeviceID,sizeof(APIOut->stLastSignOnInfo_t));//CPU Last Sign On Info
	APIOut->ucSAMSignOnControlFlag = DongleOut->stSAMParameterInfo_t.bSAMSignOnControlFlag4 |
					(DongleOut->stSAMParameterInfo_t.bSAMSignOnControlFlag5<<4);//CPU SAM Sign On Control Flag
	APIOut->ucHostSpecVersionNo = DongleOut->ucHostSpecVersionNo;//CPU Spec. Version Number
	APIOut->ucOneDayQuotaWrite = DongleOut->stSAMParameterInfo_t.bOneDayQuotaWrite2 |
				     (DongleOut->stSAMParameterInfo_t.bOneDayQuotaWrite3<<1);//CPU One Day Quota Write Flag
	APIOut->ucCPDReadFlag = DongleOut->stSAMParameterInfo_t.bCPDReadFlag0 |
			       (DongleOut->stSAMParameterInfo_t.bCPDReadFlag1<<3);//CPU CPD Read
//			       (DongleOut->stSAMParameterInfo_t.bCPDReadFlag1<<1);//CPU CPD Read

	memcpy(stStore_DataInfo_t.ucDeviceID,DongleOut->ucDeviceID,sizeof(stStore_DataInfo_t.ucDeviceID));//Device ID
	memcpy(stStore_DataInfo_t.ucTxnDateTime,DongleIn->ucTxnDateTime,sizeof(stStore_DataInfo_t.ucTxnDateTime));//Txn Date Time
	memcpy(stStore_DataInfo_t.ucCPUDeviceID,DongleOut->ucCPUDeviceID,sizeof(stStore_DataInfo_t.ucCPUDeviceID));//CPU SP ID
	memcpy(stStore_DataInfo_t.ucSTC,DongleOut->ucSTC,sizeof(stStore_DataInfo_t.ucSTC));//CPU SAM Txn Cnt

	return(inTMOutLen);
}

int inBuildSignOnData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
SignOn_TM_Out *APIOut = (SignOn_TM_Out *)API_Out;
//SignOn_APDU_In *DongleIn = (SignOn_APDU_In *)Dongle_In;
SignOn_APDU_Out *DongleOut = (SignOn_APDU_Out *)Dongle_Out;

	inTMOutLen = sizeof(SignOn_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",inTMOutLen - 7);//Data Len
	vdBuildProcessingCode(inTxnType,0x00,0x00,0x00,APIOut->ucProcessignCode);//Processing Code
//	inCnt++;//Msg Type
//	inCnt++;//Sub Type
	memcpy(APIOut->ucDeviceID,stStore_DataInfo_t.ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	memcpy(APIOut->ucTxnDateTime,stStore_DataInfo_t.ucTxnDateTime,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	memcpy(APIOut->ucConfirmCode,DongleOut->ucStatusCode,sizeof(APIOut->ucConfirmCode));//Confirm Code
//	inLen = sizeof(APIOut->);//CPU Card ID
	memcpy(APIOut->ucCPUDeviceID,stStore_DataInfo_t.ucCPUDeviceID,sizeof(APIOut->ucCPUDeviceID));//CPU Device ID
	memcpy(APIOut->ucSTC,stStore_DataInfo_t.ucSTC,sizeof(APIOut->ucSTC));//CPU SAM Txn Cnt
	APIOut->ucCreditBalanceChangeFlag = DongleOut->ucCreditBalanceChangeFlag;//Credit Balance Change Flag
	memcpy(APIOut->ucCACrypto,DongleOut->ucCACrypto,sizeof(APIOut->ucCACrypto));//CACrypto

	//memset(srTxnData.srIngData.anQueryData,0x00,sizeof(srTxnData.srIngData.anQueryData));
	//fnUnPack(Dongle_Out,sizeof(SignOn_APDU_Out),srTxnData.srIngData.anQueryData);

	return(inTMOutLen);
}

int inBuildSignOnQueryData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
SignOnQuery_TM_Out *APIOut = (SignOnQuery_TM_Out *)API_Out;
SignOnQuery_APDU_Out *DongleOut = (SignOnQuery_APDU_Out *)Dongle_Out;

	inTMOutLen = sizeof(SignOnQuery_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	vdUIntToAsc(DongleOut->ucAuthCreditLimit,sizeof(DongleOut->ucAuthCreditLimit),APIOut->anAuthCreditLimit,sizeof(APIOut->anAuthCreditLimit),FALSE,' ',10);
	vdUIntToAsc(DongleOut->ucAuthCreditBalance,sizeof(DongleOut->ucAuthCreditBalance),APIOut->anAuthCreditBalance,sizeof(APIOut->anAuthCreditBalance),FALSE,' ',10);
	vdUIntToAsc(DongleOut->ucAuthCreditCumulative,sizeof(DongleOut->ucAuthCreditCumulative),APIOut->anAuthCreditCumulative,sizeof(APIOut->anAuthCreditCumulative),FALSE,' ',10);
	vdUIntToAsc(DongleOut->ucAuthCancelCreditCumulative,sizeof(DongleOut->ucAuthCancelCreditCumulative),APIOut->anAuthCancelCreditCumulative,sizeof(APIOut->anAuthCancelCreditCumulative),FALSE,' ',10);
	APIOut->anCPDReadFlag = DongleOut->stSAMParameterInfo_t.bCPDReadFlag0 |
			       (DongleOut->stSAMParameterInfo_t.bCPDReadFlag1<<3);//CPU CPD Read
//			       (DongleOut->stSAMParameterInfo_t.bCPDReadFlag1<<1);//CPU CPD Read
	APIOut->anOneDayQuotaWrite = DongleOut->stSAMParameterInfo_t.bOneDayQuotaWrite2 |
				     (DongleOut->stSAMParameterInfo_t.bOneDayQuotaWrite3<<1);//CPU One Day Quota Write Flag
	APIOut->anSAMSignOnControlFlag = DongleOut->stSAMParameterInfo_t.bSAMSignOnControlFlag4 |
					(DongleOut->stSAMParameterInfo_t.bSAMSignOnControlFlag5<<4);//CPU SAM Sign On Control Flag
	APIOut->anCheckEVFlag = DongleOut->stSAMParameterInfo_t.bCheckEVFlag6;//Check EV Flag
	APIOut->anDeductLimitFlag = DongleOut->stSAMParameterInfo_t.bDeductLimitFlag7;//Deduct Limit Flag
	APIOut->anOneDayQuotaFlag = DongleOut->stSAMParameterInfo_t.bOneDayQuotaFlag0 |
				   (DongleOut->stSAMParameterInfo_t.bOneDayQuotaFlag1 << 4);//One Day Quota Flag
	vdUIntToAsc(DongleOut->stSAMParameterInfo_t.ucOneDayQuota,sizeof(DongleOut->stSAMParameterInfo_t.ucOneDayQuota),APIOut->anOneDayQuota,sizeof(APIOut->anOneDayQuota),FALSE,' ',10);//One Day Quota
	APIOut->anOnceQuotaFlag = DongleOut->stSAMParameterInfo_t.bOnceQuotaFlag2;//Once Quota Flag
	vdUIntToAsc(DongleOut->stSAMParameterInfo_t.ucOnceQuota,sizeof(DongleOut->stSAMParameterInfo_t.ucOnceQuota),APIOut->anOnceQuota,sizeof(APIOut->anOnceQuota),FALSE,' ',10);//Once Quota
	APIOut->anCheckDeductFlag = DongleOut->stSAMParameterInfo_t.bCheckDeductFlag3;//Check Debit Flag
	vdUIntToAsc(DongleOut->stSAMParameterInfo_t.ucCheckDeductValue,sizeof(DongleOut->stSAMParameterInfo_t.ucCheckDeductValue),APIOut->anCheckDeductValue,sizeof(APIOut->anCheckDeductValue),FALSE,' ',10);//Check Debit
	APIOut->anAddQuotaFlag = DongleOut->stSAMParameterInfo_t.ucAddQuotaFlag;//Add Quota Flag
	vdUIntToAsc(DongleOut->stSAMParameterInfo_t.ucAddQuota,sizeof(DongleOut->stSAMParameterInfo_t.ucAddQuota),APIOut->anAddQuota,sizeof(APIOut->anAddQuota),FALSE,' ',10);//Add Quota
	vdUIntToAsc(DongleOut->ucRemainderAddQuota,sizeof(DongleOut->ucRemainderAddQuota),APIOut->anRemainderAddQuota,sizeof(APIOut->anRemainderAddQuota),FALSE,' ',10);//Remainder Add Quota
	vdUIntToAsc(DongleOut->ucCancelCreditQuota,sizeof(DongleOut->ucCancelCreditQuota),APIOut->anCancelCreditQuota,sizeof(APIOut->anCancelCreditQuota),FALSE,' ',10);//Cancel Credit Quota
	vdUIntToAsc(&DongleOut->ucSPID,sizeof(DongleOut->ucSPID),APIOut->anSPID,sizeof(APIOut->anSPID),FALSE,' ',10);//SP ID
	vdUIntToAsc(DongleOut->ucCPUSPID,sizeof(DongleOut->ucCPUSPID),APIOut->anCPUSPID,sizeof(APIOut->anCPUSPID),FALSE,' ',10);//CPU SP ID
	UnixToDateTime((BYTE*)DongleOut->ucLastTxnDateTime,(BYTE*)APIOut->anLastTxnDateTime,14);//最後一次SignOn成功時間

	return(inTMOutLen);
}

int inBuildReadCardBasicDataData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
ReadCardBasicDataL2_TM_Out *APIOut = (ReadCardBasicDataL2_TM_Out *)API_Out;
ReadCardBasicData_APDU_Out *DongleOut = (ReadCardBasicData_APDU_Out *)Dongle_Out;
BYTE bData[30];
	
	inTMOutLen = sizeof(ReadCardBasicDataL2_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	APIOut->ucActivated = DongleOut->bActivated + 0x30;//Activated Flag
	APIOut->ucBlocked = DongleOut->bBlocked + 0x30;//Blocked Flag
	APIOut->ucRefunded = DongleOut->bRefunded + 0x30;//Refunded Flag
	APIOut->ucAutoLoad = DongleOut->bAutoLoad + 0x30;//AutoLoad Flag
	APIOut->ucCredit = DongleOut->bCredit + 0x30;//Credit Flag
	APIOut->ucMerchantUseFlag = DongleOut->ucMerchantUseFlag + 0x30;//Merchant Use Flag
	vdUIntToAsc(DongleOut->ucAutoLoadAmt,sizeof(DongleOut->ucAutoLoadAmt),APIOut->anAutoLoadAmt,sizeof(APIOut->anAutoLoadAmt),FALSE,' ',10);//n_AutoLoad Amount
	fnUnPack(DongleOut->ucPID,sizeof(DongleOut->ucPID),APIOut->anPID);//Purse ID
	vdUIntToAsc(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),APIOut->anCardID,sizeof(APIOut->anCardID),FALSE,' ',10);//n_Card Physical ID
	fnUnPack(&DongleOut->ucAreaCode,sizeof(DongleOut->ucAreaCode),APIOut->anAreaCode);//an_Area Code
	fnUnPack(DongleOut->ucSubAreaCode,sizeof(DongleOut->ucSubAreaCode),APIOut->anSubAreaCode);//an_Sub Area Code
	UnixToDateTime((BYTE*)DongleOut->ucExpiryDate,(BYTE*)APIOut->anExpiryDate,14);//Expiry Date
	vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->anEV,sizeof(APIOut->anEV),FALSE,' ',10);//n_EV
	vdUIntToAsc(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),APIOut->anTxnSN,sizeof(APIOut->anTxnSN),FALSE,' ',10);//n_TXN SNum
	fnUnPack(&DongleOut->ucCardType,sizeof(DongleOut->ucCardType),APIOut->anCardType);//an_Card Type
	fnUnPack(&DongleOut->ucPersonalProfile,sizeof(DongleOut->ucPersonalProfile),APIOut->anPersonalProfile);//an_Personal Profile
	UnixToDateTime((BYTE*)DongleOut->ucProfileExpiryDate,(BYTE*)APIOut->anProfileExpiryDate,14);//Personal Profile Expiry Date
	vdIntToAsc(DongleOut->ucDeposit,sizeof(DongleOut->ucDeposit),APIOut->anDeposit,sizeof(APIOut->anDeposit),FALSE,' ',10);//n_Deposit
	fnUnPack(&DongleOut->ucIssuerCode,sizeof(DongleOut->ucIssuerCode),APIOut->anIssuerCode);//an_Issuer Code
	fnUnPack(&DongleOut->ucBankCode,sizeof(DongleOut->ucBankCode),APIOut->anBankCode);//an_Bank Code
	vdUIntToAsc(DongleOut->ucLoyaltyCounter,sizeof(DongleOut->ucLoyaltyCounter),APIOut->anLoyaltyCounter,sizeof(APIOut->anLoyaltyCounter),FALSE,' ',10);//n_Loyalty Counter
	vdUIntToAsc(DongleOut->ucCardOneDayQuota,sizeof(DongleOut->ucCardOneDayQuota),APIOut->anCardOneDayQuota,sizeof(APIOut->anCardOneDayQuota),FALSE,' ',10);//n_Card One Day Quota
	fngetDosDate(DongleOut->ucCardOneDayQuotaDate,APIOut->anCardOneDayQuotaDate);//Card One Day Quota Date
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		memcpy(APIOut->anDebitTxnRecords,"06",sizeof(APIOut->anDebitTxnRecords));
	else
		vdUIntToAsc(&DongleOut->ucDebitTxnRecords,sizeof(DongleOut->ucDebitTxnRecords),APIOut->anDebitTxnRecords,sizeof(APIOut->anDebitTxnRecords),FALSE,' ',10);//Debit Txn Records
	vdBuildReadCardBasicData2(0x02,&DongleOut->stLastCreditTxnLogInfo_t,&APIOut->stLastCreditTxnRecoredInfo_t);
	vdBuildReadCardBasicData2(0x01,&DongleOut->stLastDebitTxnLogInfo_t,&APIOut->stLastDebitTxnRecoredInfo_t);

	if(srTxnData.srParameter.chOnlineFlag != ICER_ONLINE)
	{
		memset(bData,0x00,sizeof(bData));
		fnUnPack(DongleOut->RetailCardId,sizeof(DongleOut->RetailCardId),bData);//RetailCardId
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4841,bData,FALSE);
	}

	if ((srTxnData.srParameter.chBatchFlag == SIS2_BATCH) && (srTxnData.srParameter.chSaveReSendDataForReadBasic == '1'))
	{
		ReadCardBasicData_APDU_Out *tReadBasicOut = (ReadCardBasicData_APDU_Out *)Dongle_Out;
		BYTE bRecvData[SIS2_ADD_VALUE_RESEND_LEN + 1];
//		int inCnt = 0;

		inBuildSIS2AddValueResend(bRecvData,tReadBasicOut->stLastCreditTxnLogInfo_t.ucPurseVersionNumber_Index,
								0x0C,
								tReadBasicOut->stLastCreditTxnLogInfo_t.ucSubType,
								tReadBasicOut->stLastCreditTxnLogInfo_t.ucCPUDeviceID,
								tReadBasicOut->stLastCreditTxnLogInfo_t.ucCPUSPID,
								tReadBasicOut->stLastCreditTxnLogInfo_t.ucTxnDateTime,
								tReadBasicOut->ucCardID,
								tReadBasicOut->ucIssuerCode,
								tReadBasicOut->stLastCreditTxnLogInfo_t.ucTxnSN,
								tReadBasicOut->stLastCreditTxnLogInfo_t.ucTxnAmt,
								tReadBasicOut->stLastCreditTxnLogInfo_t.ucEV,
								tReadBasicOut->stLastCreditTxnLogInfo_t.ucCPULocationID,
								tReadBasicOut->ucCardIDLen,
								tReadBasicOut->ucPersonalProfile,
								tReadBasicOut->ucPID,
								tReadBasicOut->ucBankCode,
								tReadBasicOut->ucLoyaltyCounter,
								gTmpData.ucDeviceID,
								gTmpData.ucSPID,
								gTmpData.ucLocationID,
								gTmpData.ucCPUDeviceID,
								gTmpData.ucCPUSPID,
								gTmpData.ucCPULocationID);

		fnUnPack(bRecvData,SIS2_ADD_VALUE_RESEND_LEN,srTxnData.srIngData.anTAVR_TARC);
		//log_msg_debug(LOG_LEVEL_ERROR,TRUE,(char *)"Tommy AVR",9,srTxnData.srIngData.anTAVR_TARC,100);

	}
	fnUnPack(&DongleOut->ucPurseVersionNumber,sizeof(DongleOut->ucPurseVersionNumber),APIOut->anPurseVersionNumber);//anPurseVersionNumber
/*
	vdUpdateCardInfo2(inTxnType,
					 DongleOut->ucCardID,
					 DongleOut->ucPID,
					 (BYTE )DongleOut->bAutoLoad,
					 DongleOut->ucCardType,
					 DongleOut->ucPersonalProfile,
					 DongleOut->ucEV,
					 DongleOut->ucExpiryDate,
					 DongleOut->ucPurseVersionNumber,
					 DongleOut->ucBankCode,
					 DongleOut->ucAreaCode,
					 DongleOut->ucSubAreaCode,
					 DongleOut->ucDeposit,
					 DongleOut->ucTxnSN);
*/
	return(inTMOutLen);
}

void vdBuildReadCardBasicData2(BYTE ucMsgType,LastTxnLog_t *DongleOutLog,LastTxnRecoredL2_t *APIOutRecored)
{
	vdUIntToAsc(DongleOutLog->ucTxnSN,sizeof(DongleOutLog->ucTxnSN),APIOutRecored->anTxnSN,sizeof(APIOutRecored->anTxnSN),FALSE,' ',10);//n_TXN SNum
	UnixToDateTime((BYTE*)DongleOutLog->ucTxnDateTime,(BYTE*)APIOutRecored->anTxnDateTime,14);//TXN Date Time
	fnUnPack(&DongleOutLog->ucSubType,sizeof(DongleOutLog->ucSubType),APIOutRecored->anSubType);//an_Sub Type
	vdCountEVBefore(DongleOutLog->ucTxnAmt,sizeof(DongleOutLog->ucTxnAmt),DongleOutLog->ucEV,sizeof(DongleOutLog->ucEV),APIOutRecored->anEVBeforeTxn,sizeof(APIOutRecored->anEVBeforeTxn),ucMsgType);//n_EV Before TXN
	vdUIntToAsc(DongleOutLog->ucTxnAmt,sizeof(DongleOutLog->ucTxnAmt),APIOutRecored->anTxnAmt,sizeof(APIOutRecored->anTxnAmt),FALSE,' ',10);//n_TXN AMT
	vdIntToAsc(DongleOutLog->ucEV,sizeof(DongleOutLog->ucEV),APIOutRecored->anEV,sizeof(APIOutRecored->anEV),FALSE,' ',10);//n_EV After TXN
	vdUIntToAsc(DongleOutLog->ucCPUSPID,sizeof(DongleOutLog->ucCPUSPID),APIOutRecored->anCPUSPID,sizeof(APIOutRecored->anCPUSPID),FALSE,' ',10);//CPU SP ID
	vdUIntToAsc(DongleOutLog->ucCPULocationID,sizeof(DongleOutLog->ucCPULocationID),APIOutRecored->anLocationID,sizeof(APIOutRecored->anLocationID),FALSE,' ',10);//an_TXN Location ID
	fnBINTODEVASC(DongleOutLog->ucCPUDeviceID,APIOutRecored->anCPUDeviceID,sizeof(APIOutRecored->anCPUDeviceID),DongleOutLog->ucPurseVersionNumber_Index);//n_TXN Device ID
}

void vdBuildReadCardBasicData2_2(BYTE ucMsgType,LastTxnLog_t *DongleOutLog,LastTxnRecoredL2_t *APIOutRecored)
{
    vdUIntToAsc(DongleOutLog->ucTxnSN,sizeof(DongleOutLog->ucTxnSN),APIOutRecored->anTxnSN,sizeof(APIOutRecored->anTxnSN),FALSE,' ',10);//n_TXN SNum
    UnixToDateTime((BYTE*)DongleOutLog->ucTxnDateTime,(BYTE*)APIOutRecored->anTxnDateTime,sizeof(APIOutRecored->anTxnDateTime));//Personal Profile Expiry Date
	//fnLongToUnix(APIOutRecored->anTxnDateTime,(unsigned long *)DongleOutLog->ucTxnDateTime,14);//TXN Date Time
    fnUnPack(&DongleOutLog->ucSubType,sizeof(DongleOutLog->ucSubType),APIOutRecored->anSubType);//an_Sub Type
    vdCountEVBefore(DongleOutLog->ucTxnAmt,sizeof(DongleOutLog->ucTxnAmt),DongleOutLog->ucEV,sizeof(DongleOutLog->ucEV),APIOutRecored->anEVBeforeTxn,sizeof(APIOutRecored->anEVBeforeTxn),ucMsgType);//n_EV Before TXN
    //vdUIntToAsc(DongleOutLog->ucTxnAmt,sizeof(DongleOutLog->ucTxnAmt),APIOutRecored->anTxnAmt,sizeof(APIOutRecored->anTxnAmt),' ',10);//n_TXN AMT
    vdIntToAsc(DongleOutLog->ucTxnAmt,sizeof(DongleOutLog->ucTxnAmt),APIOutRecored->anTxnAmt,sizeof(APIOutRecored->anTxnAmt),FALSE,' ',10);//n_TXN AMT
    vdIntToAsc(DongleOutLog->ucEV,sizeof(DongleOutLog->ucEV),APIOutRecored->anEV,sizeof(APIOutRecored->anEV),FALSE,' ',10);//n_EV After TXN
    vdUIntToAsc(DongleOutLog->ucCPUSPID,sizeof(DongleOutLog->ucCPUSPID),APIOutRecored->anCPUSPID,sizeof(APIOutRecored->anCPUSPID),FALSE,' ',10);//CPU SP ID
    vdUIntToAsc(DongleOutLog->ucCPULocationID,sizeof(DongleOutLog->ucCPULocationID),APIOutRecored->anLocationID,sizeof(APIOutRecored->anLocationID),FALSE,' ',10);//an_TXN Location ID
    //fnBINTODEVASC(DongleOutLog->ucCPUDeviceID,APIOutRecored->anCPUDeviceID,sizeof(APIOutRecored->anCPUDeviceID),DongleOutLog->ucPurseVersionNumber_Index);//n_TXN Device ID
    fnBINTODEVASC(DongleOutLog->ucCPUDeviceID,APIOutRecored->anCPUDeviceID,sizeof(APIOutRecored->anCPUDeviceID),DongleOutLog->ucRFU);//n_TXN Device ID
}

int inBuildTxnReqOnlineData1(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
TxnReqOnline_TM_Out_1 *APIOut = (TxnReqOnline_TM_Out_1 *)API_Out;
TxnReqOnline_APDU_Out *DongleOut = (TxnReqOnline_APDU_Out *)Dongle_Out;

	inTMOutLen = sizeof(TxnReqOnline_TM_Out_1);
	memset(APIOut,0x00,inTMOutLen);
	vdUIntToAsc(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),APIOut->anCardID,sizeof(APIOut->anCardID),FALSE,' ',10);//n_Card Physical ID
	fnBINTODEVASC(DongleOut->ucDeviceID,APIOut->anDeviceID,sizeof(APIOut->anDeviceID),MIFARE);//n_TXN Device ID
	fnBINTODEVASC(DongleOut->ucCPUDeviceID,APIOut->anCPUDeviceID,sizeof(APIOut->anCPUDeviceID),LEVEL2);//n_TXN Device ID
	vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->anEVBeforeTxn,sizeof(APIOut->anEVBeforeTxn),FALSE,' ',10);//n_EV Before TXN
	vdUIntToAsc(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),APIOut->anTxnSN,sizeof(APIOut->anTxnSN),FALSE,' ',10);//n_TXN SNum
	vdUIntToAsc(DongleOut->ucTxnAmt,sizeof(DongleOut->ucTxnAmt),APIOut->anTxnAmt,sizeof(APIOut->anTxnAmt),FALSE,' ',10);//n_TXN AMT
	APIOut->ucAutoLoad = DongleOut->bAutoLoad;//AutoLoad Flag
	vdUIntToAsc(DongleOut->ucAutoLoadAmt,sizeof(DongleOut->ucAutoLoadAmt),APIOut->anAutoLoadAmt,sizeof(APIOut->anAutoLoadAmt),FALSE,' ',10);//n_AutoLoad Amount

	return(inTMOutLen);
}

int inBuildTxnReqOnlineData2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
TxnReqOnline_TM_Out_2 *APIOut = (TxnReqOnline_TM_Out_2 *)API_Out;
TxnReqOnline_APDU_In *DongleIn = (TxnReqOnline_APDU_In *)Dongle_In;
TxnReqOnline_APDU_Out *DongleOut = (TxnReqOnline_APDU_Out *)Dongle_Out;

	inTMOutLen = sizeof(TxnReqOnline_TM_Out_2);
	memset(APIOut,0x00,inTMOutLen);
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",inTMOutLen - 7);//Data Len
	vdBuildProcessingCode(inTxnType,DongleOut->ucMsgType,DongleOut->ucSubType,DongleOut->ucPersonalProfile,APIOut->ucProcessignCode);//Processing Code
	vdBuildBLCVersion(APIOut->ucBLVersion);
	//fnASCTOBIN(APIOut->ucBLVersion,gsBlackListVersion,5,sizeof(APIOut->ucBLVersion),DECIMAL);//Black List
	APIOut->ucMsgType = DongleOut->ucMsgType;//Msg Type
	APIOut->ucSubType = DongleOut->ucSubType;//Sub Type
	memcpy(APIOut->ucDeviceID,DongleOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	APIOut->ucSPID = DongleOut->ucSPID;//SP ID
	memcpy(APIOut->ucTxnDateTime,DongleIn->ucTxnDateTime,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	APIOut->ucCardType = DongleOut->ucCardType;//Card Type
	APIOut->ucPersonalProfile = DongleOut->ucPersonalProfile;//Personal Profile
	APIOut->ucIssuerCode = DongleOut->ucIssuerCode;//Issuer Code
	APIOut->ucLocationID = DongleOut->ucLocationID;//Location ID
	APIOut->ucBankCode = DongleOut->ucBankCode;//Bank Code
	memcpy(APIOut->ucLoyaltyCounter,DongleOut->ucLoyaltyCounter,sizeof(APIOut->ucLoyaltyCounter));//Loyalty Counter
	memcpy(APIOut->ucExpiryDate,DongleOut->ucExpiryDate,sizeof(APIOut->ucExpiryDate));//Expiry Date
	APIOut->ucAreaCode = DongleOut->ucAreaCode;//Area Code
	memcpy(APIOut->ucTMID,DongleIn->ucTMID,sizeof(APIOut->ucTMID));//TM ID
	memcpy(APIOut->ucTMTxnDateTime,DongleIn->ucTMTxnDateTime,sizeof(APIOut->ucTMTxnDateTime));//TM Txn Date Time
	memcpy(APIOut->ucTMSerialNumber,DongleOut->ucTMSerialNumber,sizeof(APIOut->ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->ucTMAgentNumber,DongleIn->ucTMAgentNumber,sizeof(APIOut->ucTMAgentNumber));//TM Agent Number
	memcpy(APIOut->ucSTAC,DongleOut->ucSTAC,sizeof(APIOut->ucSTAC));//STAC
	APIOut->ucKeyVersion = DongleOut->ucSAMKVN;//SAM KVN
	memcpy(APIOut->ucSAMID,DongleOut->ucSAMID,sizeof(APIOut->ucSAMID));//SAM ID
	memcpy(APIOut->ucSAMSN,DongleOut->ucSAMSN,sizeof(APIOut->ucSAMSN));//SAM SN
	memcpy(APIOut->ucSAMCRN,DongleOut->ucSAMCRN,sizeof(APIOut->ucSAMCRN));//SAM CRN
	memcpy(APIOut->ucReaderFWVersion,DongleOut->ucReaderFWVersion,sizeof(APIOut->ucReaderFWVersion));//Reader FW Version
	memcpy(APIOut->ucCardID,DongleOut->ucCardID,sizeof(APIOut->ucCardID));//Card ID
	memcpy(APIOut->ucTxnAmt,DongleOut->ucTxnAmt,sizeof(APIOut->ucTxnAmt));//Txn Amt
	memcpy(APIOut->ucCPUDeviceID,DongleOut->ucCPUDeviceID,sizeof(APIOut->ucCPUDeviceID));//CPU Device ID
	memcpy(APIOut->ucCPUSPID,DongleOut->ucCPUSPID,sizeof(APIOut->ucCPUSPID));//CPU SP ID
	memcpy(APIOut->ucCPULocationID,DongleOut->ucCPULocationID,sizeof(APIOut->ucCPULocationID));//CPU Location ID
	memcpy(APIOut->ucPID,DongleOut->ucPID,sizeof(APIOut->ucPID));//CPU Purse ID
	memcpy(APIOut->ucCTC,DongleOut->ucCTC,sizeof(APIOut->ucCTC));//CPU CTC
	memcpy(APIOut->ucProfileExpiryDate,DongleOut->ucProfileExpiryDate,sizeof(APIOut->ucProfileExpiryDate));//Profile Expiry Date
	memcpy(APIOut->ucSubAreaCode,DongleOut->ucSubAreaCode,sizeof(APIOut->ucSubAreaCode));//Sub Area Code
	memcpy(APIOut->ucTxnSN,DongleOut->ucTxnSN,sizeof(APIOut->ucTxnSN));//Txn SN
	memcpy(APIOut->ucEV,DongleOut->ucEV,sizeof(APIOut->ucEV));//EV
	memcpy(APIOut->ucDeposit,DongleOut->ucDeposit,sizeof(APIOut->ucDeposit));//Deposit
	APIOut->ucCPUAdminKeyKVN = DongleOut->ucCPUAdminKeyKVN;//CPU Admin Key KVN
	APIOut->ucCreditKeyKVN = DongleOut->ucCreditKeyKVN;//Credit Key KVN
	APIOut->ucCPUIssuerKeyKVN = DongleOut->ucCPUIssuerKeyKVN;//CPU Issuer Key KVN
	APIOut->ucTxnMode = DongleOut->ucTxnMode;//Txn Mode
	APIOut->ucTxnQuqlifier = DongleOut->ucTxnQuqlifier;//Txn Quqlifier
	APIOut->ucSignatureKeyKVN = DongleOut->ucSignatureKeyKVN;//Signature Key KVN
	memcpy(APIOut->ucCPUSAMID,DongleOut->ucCPUSAMID,sizeof(APIOut->ucCPUSAMID));//CPU SAM ID
	APIOut->ucHostAdminKVN = DongleOut->ucHostAdminKVN;//Host Admin KVN
	APIOut->ucPurseVersionNumber = DongleOut->ucPurseVersionNumber;//Purse Version Number
	memcpy(APIOut->ucTMLocationID,DongleIn->ucTMLocationID,sizeof(APIOut->ucTMLocationID));//TM Location ID
	memcpy(APIOut->ucCACrypto,DongleOut->ucTxnCrypto,8);//CACrypto前8碼
	memset(&APIOut->ucCACrypto[8],0x00,8);//CACrypto後8碼
	vdBuildCardAVRData(&APIOut->stCardAVRInfo_t,&DongleOut->stLastCreditTxnLogInfo_t,DongleOut->ucCardID,DongleOut->ucIssuerCode,DongleOut->ucCPUSPID,DongleOut->ucCPULocationID,DongleOut->ucPID,APIOut->ucLoyaltyCounter);//Card AVR Info
	vdBuildReaderAVRData(&APIOut->stReaderAVRInfo_t,&DongleOut->stReaderAVRInfo_t);//Reader AVR Info
	//memcpy(&APIOut->stReaderAVRInfo_t,&DongleOut->stReaderAVRInfo_t,sizeof(APIOut->stReaderAVRInfo_t));//Reader AVR Info
	APIOut->ucCardIDLen = DongleOut->ucCardIDLen;//Card ID Len

	// 二十年展期開關
	int iReaderVer = ((int)(DongleOut->ucReaderFWVersion[0] & 0x0F) * 100) + (int)DongleOut->ucReaderFWVersion[1];
	log_msg(LOG_LEVEL_ERROR,"ReaderFWVersion: %d", iReaderVer);
	if ((srTxnData.srParameter.inSetCardValidDate == 1) && (srTxnData.srParameter.chOnlineFlag == R6_ONLINE) && (iReaderVer >= 810))
	{
		APIOut->ucCardIDLen |= 0x80;
	}

	memcpy(stStore_DataInfo_t.ucTMSerialNumber,DongleOut->ucTMSerialNumber,sizeof(stStore_DataInfo_t.ucTMSerialNumber));

	return(inTMOutLen);
}

int inBuildTxnReqOnlineData3(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
CardSale_TM_Out *APIOut = (CardSale_TM_Out *)API_Out;
TxnReqOnline_APDU_In *DongleIn = (TxnReqOnline_APDU_In *)Dongle_In;
TxnReqOnline_APDU_Out *DongleOut = (TxnReqOnline_APDU_Out *)Dongle_Out;

	inTMOutLen = sizeof(CardSale_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",inTMOutLen - 7);//Data Len
	vdBuildProcessingCode(inTxnType,DongleOut->ucMsgType,DongleOut->ucSubType,DongleOut->ucPersonalProfile,APIOut->ucProcessignCode);//Processing Code
	vdBuildBLCVersion(APIOut->ucBLVersion);
	//fnASCTOBIN(APIOut->ucBLVersion,gsBlackListVersion,5,sizeof(APIOut->ucBLVersion),DECIMAL);//Black List
	APIOut->ucMsgType = DongleOut->ucMsgType;//Msg Type
	APIOut->ucSubType = DongleOut->ucSubType;//Sub Type
	memcpy(APIOut->ucDeviceID,DongleOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	APIOut->ucSPID = DongleOut->ucSPID;//SP ID
	memcpy(APIOut->ucTxnDateTime,DongleIn->ucTxnDateTime,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	APIOut->ucCardType = DongleOut->ucCardType;//Card Type
	APIOut->ucPersonalProfile = DongleOut->ucPersonalProfile;//Personal Profile
	APIOut->ucIssuerCode = DongleOut->ucIssuerCode;//Issuer Code
	APIOut->ucLocationID = DongleOut->ucLocationID;//Location ID
	memcpy(APIOut->ucTMID,DongleIn->ucTMID,sizeof(APIOut->ucTMID));//TM ID
	memcpy(APIOut->ucTMTxnDateTime,DongleIn->ucTMTxnDateTime,sizeof(APIOut->ucTMTxnDateTime));//TM Txn Date Time
	memcpy(APIOut->ucTMSerialNumber,DongleOut->ucTMSerialNumber,sizeof(APIOut->ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->ucTMAgentNumber,DongleIn->ucTMAgentNumber,sizeof(APIOut->ucTMAgentNumber));//TM Agent Number
	memcpy(APIOut->ucSTAC,DongleOut->ucSTAC,sizeof(APIOut->ucSTAC));//STAC
	APIOut->ucKeyVersion = DongleOut->ucSAMKVN;//SAM KVN
	memcpy(APIOut->ucSAMID,DongleOut->ucSAMID,sizeof(APIOut->ucSAMID));//SAM ID
	memcpy(APIOut->ucSAMSN,DongleOut->ucSAMSN,sizeof(APIOut->ucSAMSN));//SAM SN
	memcpy(APIOut->ucSAMCRN,DongleOut->ucSAMCRN,sizeof(APIOut->ucSAMCRN));//SAM CRN
	memcpy(APIOut->ucReaderFWVersion,DongleOut->ucReaderFWVersion,sizeof(APIOut->ucReaderFWVersion));//Reader FW Version
	memcpy(APIOut->ucCardID,DongleOut->ucCardID,sizeof(APIOut->ucCardID));//Card ID
	memcpy(APIOut->ucTxnAmt,DongleOut->ucTxnAmt,sizeof(APIOut->ucTxnAmt));//Txn Amt
	memcpy(APIOut->ucCPUDeviceID,DongleOut->ucCPUDeviceID,sizeof(APIOut->ucCPUDeviceID));//CPU Device ID
	memcpy(APIOut->ucCPUSPID,DongleOut->ucCPUSPID,sizeof(APIOut->ucCPUSPID));//CPU SP ID
	memcpy(APIOut->ucCPULocationID,DongleOut->ucCPULocationID,sizeof(APIOut->ucCPULocationID));//CPU Location ID
	memcpy(APIOut->ucPID,DongleOut->ucPID,sizeof(APIOut->ucPID));//CPU Purse ID
	memcpy(APIOut->ucCTC,DongleOut->ucCTC,sizeof(APIOut->ucCTC));//CPU CTC
	memcpy(APIOut->ucSubAreaCode,DongleOut->ucSubAreaCode,sizeof(APIOut->ucSubAreaCode));//Sub Area Code
	memcpy(APIOut->ucTxnSN,DongleOut->ucTxnSN,sizeof(APIOut->ucTxnSN));//Txn SN
	memcpy(APIOut->ucEV,DongleOut->ucEV,sizeof(APIOut->ucEV));//EV
	memcpy(APIOut->ucDeposit,DongleOut->ucDeposit,sizeof(APIOut->ucDeposit));//Deposit
	APIOut->ucCPUAdminKeyKVN = DongleOut->ucCPUAdminKeyKVN;//CPU Admin Key KVN
	APIOut->ucCreditKeyKVN = DongleOut->ucCreditKeyKVN;//Credit Key KVN
	APIOut->ucCPUIssuerKeyKVN = DongleOut->ucCPUIssuerKeyKVN;//CPU Issuer Key KVN
	APIOut->ucTxnMode = DongleOut->ucTxnMode;//Txn Mode
	APIOut->ucTxnQuqlifier = DongleOut->ucTxnQuqlifier;//Txn Quqlifier
	memcpy(APIOut->ucCPUSAMID,DongleOut->ucCPUSAMID,sizeof(APIOut->ucCPUSAMID));//CPU SAM ID
	APIOut->ucHostAdminKVN = DongleOut->ucHostAdminKVN;//Host Admin KVN
	APIOut->ucPurseVersionNumber = DongleOut->ucPurseVersionNumber;//Purse Version Number
	memcpy(APIOut->ucTMLocationID,DongleIn->ucTMLocationID,sizeof(APIOut->ucTMLocationID));//TM Location ID
	memcpy(APIOut->ucCACrypto,DongleOut->ucTxnCrypto,8);//CACrypto前8碼
	memset(&APIOut->ucCACrypto[8],0x00,8);//CACrypto後8碼
	vdBuildReaderAVRData(&APIOut->stReaderAVRInfo_t,&DongleOut->stReaderAVRInfo_t);//Reader AVR Info
	//memcpy(&APIOut->stReaderAVRInfo_t,&DongleOut->stReaderAVRInfo_t,sizeof(APIOut->stReaderAVRInfo_t));//Reader AVR Info
	APIOut->ucCardIDLen = DongleOut->ucCardIDLen;//Card ID Len

	memcpy(stStore_DataInfo_t.ucTMSerialNumber,DongleOut->ucTMSerialNumber,sizeof(stStore_DataInfo_t.ucTMSerialNumber));

	return(inTMOutLen);
}

int inBuildTxnReqOnlineData4(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
TxnReqOnline_TM_Out_4 *APIOut = (TxnReqOnline_TM_Out_4 *)API_Out;
TxnReqOnline_APDU_In *DongleIn = (TxnReqOnline_APDU_In *)Dongle_In;
TxnReqOnline_APDU_Out *DongleOut = (TxnReqOnline_APDU_Out *)Dongle_Out;

	inTMOutLen = sizeof(TxnReqOnline_TM_Out_4);
	memset(APIOut,0x00,inTMOutLen);
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",inTMOutLen - 7);//Data Len
	vdBuildProcessingCode(inTxnType,DongleOut->ucMsgType,DongleOut->ucSubType,DongleOut->ucPersonalProfile,APIOut->ucProcessignCode);//Processing Code
	vdBuildBLCVersion(APIOut->ucBLVersion);
	//fnASCTOBIN(APIOut->ucBLVersion,gsBlackListVersion,5,sizeof(APIOut->ucBLVersion),DECIMAL);//Black List
	APIOut->ucMsgType = DongleOut->ucMsgType;//Msg Type
	APIOut->ucSubType = DongleOut->ucSubType;//Sub Type
	memcpy(APIOut->ucDeviceID,DongleOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	APIOut->ucSPID = DongleOut->ucSPID;//SP ID
	memcpy(APIOut->ucTxnDateTime,DongleIn->ucTxnDateTime,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	APIOut->ucCardType = DongleOut->ucCardType;//Card Type
	APIOut->ucPersonalProfile = DongleOut->ucPersonalProfile;//Personal Profile
	APIOut->ucIssuerCode = DongleOut->ucIssuerCode;//Issuer Code
	APIOut->ucLocationID = DongleOut->ucLocationID;//Location ID
	APIOut->ucBankCode = DongleOut->ucBankCode;//Bank Code
	memcpy(APIOut->ucLoyaltyCounter,DongleOut->ucLoyaltyCounter,sizeof(APIOut->ucLoyaltyCounter));//Loyalty Counter
	memcpy(APIOut->ucExpiryDate,DongleOut->ucExpiryDate,sizeof(APIOut->ucExpiryDate));//Expiry Date
	APIOut->ucAreaCode = DongleOut->ucAreaCode;//Area Code
	memcpy(APIOut->ucTMID,DongleIn->ucTMID,sizeof(APIOut->ucTMID));//TM ID
	memcpy(APIOut->ucTMTxnDateTime,DongleIn->ucTMTxnDateTime,sizeof(APIOut->ucTMTxnDateTime));//TM Txn Date Time
	memcpy(APIOut->ucTMSerialNumber,DongleOut->ucTMSerialNumber,sizeof(APIOut->ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->ucTMAgentNumber,DongleIn->ucTMAgentNumber,sizeof(APIOut->ucTMAgentNumber));//TM Agent Number
	memcpy(APIOut->ucSTAC,DongleOut->ucSTAC,sizeof(APIOut->ucSTAC));//STAC
	APIOut->ucKeyVersion = DongleOut->ucSAMKVN;//SAM KVN
	memcpy(APIOut->ucSAMID,DongleOut->ucSAMID,sizeof(APIOut->ucSAMID));//SAM ID
	memcpy(APIOut->ucSAMSN,DongleOut->ucSAMSN,sizeof(APIOut->ucSAMSN));//SAM SN
	memcpy(APIOut->ucSAMCRN,DongleOut->ucSAMCRN,sizeof(APIOut->ucSAMCRN));//SAM CRN
	memcpy(APIOut->ucReaderFWVersion,DongleOut->ucReaderFWVersion,sizeof(APIOut->ucReaderFWVersion));//Reader FW Version
	memcpy(APIOut->ucCardID,DongleOut->ucCardID,sizeof(APIOut->ucCardID));//Card ID
	memcpy(APIOut->ucTxnAmt,DongleOut->ucTxnAmt,sizeof(APIOut->ucTxnAmt));//Txn Amt
	memcpy(APIOut->ucCPUDeviceID,DongleOut->ucCPUDeviceID,sizeof(APIOut->ucCPUDeviceID));//CPU Device ID
	memcpy(APIOut->ucCPUSPID,DongleOut->ucCPUSPID,sizeof(APIOut->ucCPUSPID));//CPU SP ID
	memcpy(APIOut->ucCPULocationID,DongleOut->ucCPULocationID,sizeof(APIOut->ucCPULocationID));//CPU Location ID
	memcpy(APIOut->ucPID,DongleOut->ucPID,sizeof(APIOut->ucPID));//CPU Purse ID
	memcpy(APIOut->ucCTC,DongleOut->ucCTC,sizeof(APIOut->ucCTC));//CPU CTC
	memcpy(APIOut->ucProfileExpiryDate,DongleOut->ucProfileExpiryDate,sizeof(APIOut->ucProfileExpiryDate));//Profile Expiry Date
	memcpy(APIOut->ucSubAreaCode,DongleOut->ucSubAreaCode,sizeof(APIOut->ucSubAreaCode));//Sub Area Code
	memcpy(APIOut->ucTxnSN,DongleOut->ucTxnSN,sizeof(APIOut->ucTxnSN));//Txn SN
	memcpy(APIOut->ucEV,DongleOut->ucEV,sizeof(APIOut->ucEV));//EV
	memcpy(APIOut->ucDeposit,DongleOut->ucDeposit,sizeof(APIOut->ucDeposit));//Deposit
	APIOut->ucCPUAdminKeyKVN = DongleOut->ucCPUAdminKeyKVN;//CPU Admin Key KVN
	APIOut->ucCreditKeyKVN = DongleOut->ucCreditKeyKVN;//Credit Key KVN
	APIOut->ucCPUIssuerKeyKVN = DongleOut->ucCPUIssuerKeyKVN;//CPU Issuer Key KVN
	APIOut->ucTxnMode = DongleOut->ucTxnMode;//Txn Mode
	APIOut->ucTxnQuqlifier = DongleOut->ucTxnQuqlifier;//Txn Quqlifier
	APIOut->ucSignatureKeyKVN = DongleOut->ucSignatureKeyKVN;//Signature Key KVN
	memcpy(APIOut->ucCPUSAMID,DongleOut->ucCPUSAMID,sizeof(APIOut->ucCPUSAMID));//CPU SAM ID
	APIOut->ucHostAdminKVN = DongleOut->ucHostAdminKVN;//Host Admin KVN
	APIOut->ucPurseVersionNumber = DongleOut->ucPurseVersionNumber;//Purse Version Number
	memcpy(APIOut->ucTMLocationID,DongleIn->ucTMLocationID,sizeof(APIOut->ucTMLocationID));//TM Location ID
	memcpy(APIOut->ucCACrypto,DongleOut->ucTxnCrypto,8);//CACrypto前8碼
	memset(&APIOut->ucCACrypto[8],0x00,8);//CACrypto後8碼
	vdBuildCardAVRData(&APIOut->stCardAVRInfo_t,&DongleOut->stLastCreditTxnLogInfo_t,DongleOut->ucCardID,DongleOut->ucIssuerCode,DongleOut->ucCPUSPID,DongleOut->ucCPULocationID,DongleOut->ucPID,APIOut->ucLoyaltyCounter);//Card AVR Info
	vdBuildReaderAVRData(&APIOut->stReaderAVRInfo_t,&DongleOut->stReaderAVRInfo_t);//Reader AVR Info
	//memcpy(&APIOut->stReaderAVRInfo_t,&DongleOut->stReaderAVRInfo_t,sizeof(APIOut->stReaderAVRInfo_t));//Reader AVR Info
	APIOut->ucCardIDLen = DongleOut->ucCardIDLen;//Card ID Len

	// 二十年展期開關
	int iReaderVer = ((int)(DongleOut->ucReaderFWVersion[0] & 0x0F) * 100) + (int)DongleOut->ucReaderFWVersion[1];
	log_msg(LOG_LEVEL_ERROR,"ReaderFWVersion: %d", iReaderVer);
	if ((srTxnData.srParameter.inSetCardValidDate == 1) && (srTxnData.srParameter.chOnlineFlag == R6_ONLINE) && (iReaderVer >= 810))
	{
		APIOut->ucCardIDLen |= 0x80;
	}
    memcpy(APIOut->ucTxnAmt1,(BYTE *)&srTxnData.srIngData.lnECCAmt1,2);
    memcpy(APIOut->ucTxnAmt2,(BYTE *)&srTxnData.srIngData.lnECCAmt2,2);
    memcpy(APIOut->ucTxnAmt3,(BYTE *)&srTxnData.srIngData.lnECCAmt3,2);
	memcpy(APIOut->ucTxnAmt4,(BYTE *)&srTxnData.srIngData.lnECCAmt4,2);
	memcpy(APIOut->ucTxnAmt5,(BYTE *)&srTxnData.srIngData.lnECCAmt5,2);

	memcpy(stStore_DataInfo_t.ucTMSerialNumber,DongleOut->ucTMSerialNumber,sizeof(stStore_DataInfo_t.ucTMSerialNumber));

	return(inTMOutLen);
}

void vdBuildCardAVRData(CardAVR_t *APIOutData,LastTxnLog_t *DongleOutLog,BYTE *ucCardID,BYTE ucIssuerCode,BYTE *ucCPUSPID,BYTE *ucCPULocationID,BYTE *ucPID,BYTE *ucLoyaltyCounter)
{

	APIOutData->ucMsgType = 0x0C;//加值重送
	APIOutData->ucPurseVersionNumber = DongleOutLog->ucPurseVersionNumber_Index;//Purse Version Number
	memcpy(APIOutData->ucTxnSN,DongleOutLog->ucTxnSN,sizeof(APIOutData->ucTxnSN));//Txn SN
	memcpy(APIOutData->ucTxnDateTime,DongleOutLog->ucTxnDateTime,sizeof(APIOutData->ucTxnDateTime));//Txn Date Time
	APIOutData->ucSubType = DongleOutLog->ucSubType;//Sub Type
	memcpy(APIOutData->ucTxnAmt,DongleOutLog->ucTxnAmt,sizeof(APIOutData->ucTxnAmt));//Txn Amt
	memcpy(APIOutData->ucEV,DongleOutLog->ucEV,sizeof(APIOutData->ucEV));//EV
	memcpy(APIOutData->ucCardID,ucCardID,sizeof(APIOutData->ucCardID));//Card ID
	APIOutData->ucIssuerCode = ucIssuerCode;//Issuer Code
	memcpy(APIOutData->ucCPUSPID,DongleOutLog->ucCPUSPID,sizeof(APIOutData->ucCPUSPID));//CPU SP ID
	//memcpy(APIOutData->ucCPUSPID,ucCPUSPID,sizeof(APIOutData->ucCPUSPID));//CPU SP ID
	memcpy(APIOutData->ucCPULocationID,DongleOutLog->ucCPULocationID,sizeof(APIOutData->ucCPULocationID));//CPU Location ID
	//memcpy(APIOutData->ucCPULocationID,ucCPULocationID,sizeof(APIOutData->ucCPULocationID));//CPU Location ID
	memcpy(APIOutData->ucPID,ucPID,sizeof(APIOutData->ucPID));//Loyalty Counter
	memcpy(APIOutData->ucCPUDeviceID,DongleOutLog->ucCPUDeviceID,sizeof(APIOutData->ucCPUDeviceID));//CPU Device ID
	memcpy(APIOutData->ucLoyaltyCounter,ucLoyaltyCounter,sizeof(APIOutData->ucLoyaltyCounter));//Loyalty Counter

}

void vdBuildReaderAVRData(ReaderAVRTM_t *APIOutData,ReaderAVRAPDU_t *DongleOutLog)
{
	APIOutData->ucMsgType = DongleOutLog->ucMsgType;
	APIOutData->ucSubType = DongleOutLog->ucSubType;
	memcpy(APIOutData->ucDeviceID,DongleOutLog->ucDeviceID,sizeof(APIOutData->ucDeviceID));
	memcpy(APIOutData->ucCPUDeviceID,DongleOutLog->ucCPUDeviceID,sizeof(APIOutData->ucCPUDeviceID));
	memcpy(APIOutData->ucTxnDateTime,DongleOutLog->ucTxnDateTime,sizeof(APIOutData->ucTxnDateTime));
	APIOutData->ucPurseVersionNumber = DongleOutLog->ucPurseVersionNumber;
	memcpy(APIOutData->ucCardID,DongleOutLog->ucCardID,sizeof(APIOutData->ucCardID));
	memcpy(APIOutData->ucTxnSN,DongleOutLog->ucTxnSN,sizeof(APIOutData->ucTxnSN));
	memcpy(APIOutData->ucEV,DongleOutLog->ucEV,sizeof(APIOutData->ucEV));
	memcpy(APIOutData->ucStatusCode,DongleOutLog->ucStatusCode,sizeof(APIOutData->ucStatusCode));
	memcpy(APIOutData->ucPID,DongleOutLog->ucPID,sizeof(APIOutData->ucPID));
	memcpy(APIOutData->ucSignature,DongleOutLog->ucSignature,sizeof(APIOutData->ucSignature));
	memcpy(APIOutData->ucSAMID,DongleOutLog->ucSAMID,sizeof(APIOutData->ucSAMID));
	memcpy(APIOutData->ucMAC_HCrypto,DongleOutLog->ucMAC_HCrypto,sizeof(APIOutData->ucMAC_HCrypto));
	APIOutData->ucCardIDLen = DongleOutLog->ucCardIDLen;
}

int inBuildAuthTxnOnlineData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
AuthTxnOnline_TM_Out *APIOut = (AuthTxnOnline_TM_Out *)API_Out;
AuthTxnOnline_APDU_In *DongleIn = (AuthTxnOnline_APDU_In *)Dongle_In;
AuthTxnOnline_APDU_Out *DongleOut = (AuthTxnOnline_APDU_Out *)Dongle_Out;
TxnReqOnline_APDU_Out *TxnReqOut = (TxnReqOnline_APDU_Out *)Dongle_ReqOut;

	inTMOutLen = sizeof(AuthTxnOnline_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	vdUIntToAsc(TxnReqOut->ucCardID,sizeof(TxnReqOut->ucCardID),APIOut->stSTMC_t.anCardID,sizeof(APIOut->stSTMC_t.anCardID),FALSE,' ',10);//n_Card Physical ID
	fnBINTODEVASC(TxnReqOut->ucDeviceID,APIOut->stSTMC_t.anDeviceID,sizeof(APIOut->stSTMC_t.anDeviceID),MIFARE);//n_TXN Device ID
	fnBINTODEVASC(TxnReqOut->ucCPUDeviceID,APIOut->stSTMC_t.anCPUDeviceID,sizeof(APIOut->stSTMC_t.anCPUDeviceID),LEVEL2);//n_TXN Device ID
    vdIntToAsc(TxnReqOut->ucEV,sizeof(TxnReqOut->ucEV),APIOut->stSTMC_t.anEVBeforeTxn,sizeof(APIOut->stSTMC_t.anEVBeforeTxn),FALSE,' ',10);//n_EV Before TXN
	//vdCountEVBefore(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.anEVBeforeTxn,sizeof(APIOut->stSTMC_t.anEVBeforeTxn),TxnReqOut->ucMsgType);//n_EV Before TXN
	vdUIntToAsc(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),APIOut->stSTMC_t.anTxnSN,sizeof(APIOut->stSTMC_t.anTxnSN),FALSE,' ',10);//n_TXN SNum
	vdIntToAsc(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),APIOut->stSTMC_t.anTxnAmt,sizeof(APIOut->stSTMC_t.anTxnAmt),FALSE,' ',10);//n_TXN AMT
	vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.anEV,sizeof(APIOut->stSTMC_t.anEV),FALSE,' ',10);//n_EV
	vdBuildProcessingCode(inTxnType,TxnReqOut->ucMsgType,TxnReqOut->ucSubType,TxnReqOut->ucPersonalProfile,APIOut->stSTMC_t.anProcessignCode);//Processing Code
	fnUnPack(&TxnReqOut->ucMsgType,sizeof(TxnReqOut->ucMsgType),APIOut->stSTMC_t.unMsgType);//Msg Type
	fnUnPack(&TxnReqOut->ucSubType,sizeof(TxnReqOut->ucSubType),APIOut->stSTMC_t.unSubType);//Sub Type
	fnUnPack(TxnReqOut->ucDeviceID,sizeof(TxnReqOut->ucDeviceID),APIOut->stSTMC_t.unDeviceID);//Device ID
	fnUnPack(&TxnReqOut->ucSPID,sizeof(TxnReqOut->ucSPID),APIOut->stSTMC_t.unSPID);//SP ID
	fnUnPack(DongleOut->ucTxnDateTime,sizeof(DongleOut->ucTxnDateTime),APIOut->stSTMC_t.unTxnDateTime);//Txn Date Time
	fnUnPack(&TxnReqOut->ucCardType,sizeof(TxnReqOut->ucCardType),APIOut->stSTMC_t.unCardType);//Card Type
	fnUnPack(&TxnReqOut->ucPersonalProfile,sizeof(TxnReqOut->ucPersonalProfile),APIOut->stSTMC_t.unPersonalProfile);//Personal Profile
	fnUnPack(&TxnReqOut->ucLocationID,sizeof(TxnReqOut->ucLocationID),APIOut->stSTMC_t.unLocationID);//Location ID
	fnUnPack(TxnReqOut->ucCardID,sizeof(TxnReqOut->ucCardID),APIOut->stSTMC_t.unCardID);//CPU Card ID
	fnUnPack(&TxnReqOut->ucIssuerCode,sizeof(TxnReqOut->ucIssuerCode),APIOut->stSTMC_t.unIssuerCode);//Issuer Code
	fnUnPack(&TxnReqOut->ucBankCode,sizeof(TxnReqOut->ucBankCode),APIOut->stSTMC_t.unBankCode);//Bank Code
	fnUnPack(TxnReqOut->ucLoyaltyCounter,sizeof(TxnReqOut->ucLoyaltyCounter),APIOut->stSTMC_t.unLoyaltyCounter);//Loyalty Counter
	if(TxnReqOut->ucPurseVersionNumber == MIFARE)
	{
		if (TxnReqOut->ucReaderFWVersion[5] >= 0x80) //match sam的mfrc改用device id
			memcpy(&DongleOut->ucMAC_HCrypto[6], TxnReqOut->ucDeviceID, 4);
		fnUnPack(DongleOut->ucMAC_HCrypto,10,APIOut->stSTMC_t.unMAC);//MAC
		// 配對SAM用Device id
		//if (TxnReqOut->ucReaderFWVersion[5] >= 0x80)
		//	fnUnPack(TxnReqOut->ucDeviceID, sizeof(TxnReqOut->ucDeviceID), &APIOut->stSTMC_t.unMAC[12]);
	}
	else
		memset(APIOut->stSTMC_t.unMAC,0x30,sizeof(APIOut->stSTMC_t.unMAC));//MAC
	fnUnPack(TxnReqOut->ucExpiryDate,sizeof(TxnReqOut->ucExpiryDate),APIOut->stSTMC_t.unOrgExpiryDate);//Org Expiry Date
	fnUnPack(DongleIn->ucExpiryDate,sizeof(DongleIn->ucExpiryDate),APIOut->stSTMC_t.unNewExpiryDate);//New Expiry Date
	fnUnPack(&TxnReqOut->ucAreaCode,sizeof(TxnReqOut->ucAreaCode),APIOut->stSTMC_t.unAreaCode);//Area Code
	fnUnPack(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),APIOut->stSTMC_t.unTxnAmt);//Txn Amt
	fnUnPack(DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.unEV);//EV
	fnUnPack(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),APIOut->stSTMC_t.unTxnSN);//Txn SN
	fnUnPack(TxnReqOut->ucCPUDeviceID,sizeof(TxnReqOut->ucCPUDeviceID),APIOut->stSTMC_t.unCPUDeviceID);//CPU Device ID
	fnUnPack(TxnReqOut->ucCPUSPID,sizeof(TxnReqOut->ucCPUSPID),APIOut->stSTMC_t.unCPUSPID);//CPU SP ID
	fnUnPack(TxnReqOut->ucCPULocationID,sizeof(TxnReqOut->ucCPULocationID),APIOut->stSTMC_t.unCPULocationID);//CPU Location ID
	fnUnPack(TxnReqOut->ucPID,sizeof(TxnReqOut->ucPID),APIOut->stSTMC_t.unPID);//Purse ID
	fnUnPack(TxnReqOut->ucCTC,sizeof(TxnReqOut->ucCTC),APIOut->stSTMC_t.unCTC);//CTC
	fnUnPack(TxnReqOut->ucProfileExpiryDate,sizeof(TxnReqOut->ucProfileExpiryDate),APIOut->stSTMC_t.unOrgProfileExpiryDate);//Org Profile Expiry Date
	fnUnPack(DongleIn->ucProfileExpiryDate,sizeof(DongleIn->ucProfileExpiryDate),APIOut->stSTMC_t.unNewProfileExpiryDate);//New Profile Expiry Date
	fnUnPack(TxnReqOut->ucSubAreaCode,sizeof(TxnReqOut->ucSubAreaCode),APIOut->stSTMC_t.unSubAreaCode);//Sub Area Code
	fnUnPack(TxnReqOut->ucTxnSN,sizeof(TxnReqOut->ucTxnSN),APIOut->stSTMC_t.unTxnSNBeforeTxn);//Txn SN Before Txn
	fnUnPack(TxnReqOut->ucEV,sizeof(TxnReqOut->ucEV),APIOut->stSTMC_t.unEVBeforeTxn);//EV Before Txn
	fnUnPack(TxnReqOut->ucDeposit,sizeof(TxnReqOut->ucDeposit),APIOut->stSTMC_t.unDeposit);//Deposit
	fnUnPack(&TxnReqOut->ucTxnMode,sizeof(TxnReqOut->ucTxnMode),APIOut->stSTMC_t.unTxnMode);//Txn Mode
	fnUnPack(&TxnReqOut->ucTxnQuqlifier,sizeof(TxnReqOut->ucTxnQuqlifier),APIOut->stSTMC_t.unTxnQuqlifier);//Txn Quqlifier
	fnUnPack(&TxnReqOut->ucSignatureKeyKVN,sizeof(TxnReqOut->ucSignatureKeyKVN),APIOut->stSTMC_t.unSignatureKeyKVN);//Signature Key KVN
	fnUnPack(DongleOut->ucSignature,sizeof(DongleOut->ucSignature),APIOut->stSTMC_t.unSignature);//Signature
	fnUnPack(DongleOut->ucCPUSAMID,sizeof(DongleOut->ucCPUSAMID),APIOut->stSTMC_t.unCPUSAMID);//SAM ID
	if(TxnReqOut->ucPurseVersionNumber != MIFARE)
		fnUnPack(&DongleOut->ucMAC_HCrypto[0],1,APIOut->stSTMC_t.unHashType);//Hash Type
	else
		memset(APIOut->stSTMC_t.unHashType,0x30,sizeof(APIOut->stSTMC_t.unHashType));
	if(TxnReqOut->ucPurseVersionNumber != MIFARE)
	{
		fnUnPack(&DongleOut->ucMAC_HCrypto[1],1,APIOut->stSTMC_t.unHostAdminKVN);//Host Admin KVN
		fnUnPack(&DongleOut->ucMAC_HCrypto[2],16,APIOut->stSTMC_t.unCPUMAC);//MAC
	}
	else
	{
		memset(APIOut->stSTMC_t.unHostAdminKVN,0x30,sizeof(APIOut->stSTMC_t.unHostAdminKVN));
		memset(APIOut->stSTMC_t.unCPUMAC,0x30,sizeof(APIOut->stSTMC_t.unCPUMAC));//MAC
	}
	fnUnPack(&TxnReqOut->ucPurseVersionNumber,sizeof(TxnReqOut->ucPurseVersionNumber),APIOut->stSTMC_t.unPurseVersionNumber);//Purse Version Number
	fnUnPack(&TxnReqOut->ucCardIDLen,sizeof(TxnReqOut->ucCardIDLen),APIOut->stSTMC_t.unCardIDLen);//Card ID Len
	memcpy(APIOut->stSTMC_t.ucTMLocationID,stStore_DataInfo_t.ucTMLocationID,sizeof(stStore_DataInfo_t.ucTMLocationID));//TM Location ID
	memcpy(APIOut->stSTMC_t.ucTMID,stStore_DataInfo_t.ucTMID,sizeof(stStore_DataInfo_t.ucTMID));//TM ID
	UnixToDateTime((BYTE*)DongleOut->ucTxnDateTime,(BYTE*)APIOut->stSTMC_t.ucTMTxnDateTime,14);//TM Txn Date Time
	memcpy(APIOut->stSTMC_t.ucTMSerialNumber,stStore_DataInfo_t.ucTMSerialNumber,sizeof(stStore_DataInfo_t.ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->stSTMC_t.ucTMAgentNumber,stStore_DataInfo_t.ucTMAgentNumber,sizeof(stStore_DataInfo_t.ucTMAgentNumber));//TM Agent Number
	//帳務資料結束
	//Confirm資料開始
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",79);//Data Len
	vdBuildProcessingCode(inTxnType,TxnReqOut->ucMsgType,TxnReqOut->ucSubType,TxnReqOut->ucPersonalProfile,APIOut->ucProcessignCode);//Processing Code
	APIOut->ucMsgType = TxnReqOut->ucMsgType;//Msg Type
	APIOut->ucSubType = TxnReqOut->ucSubType;//Sub Type
	memcpy(APIOut->ucDeviceID,TxnReqOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	memcpy(APIOut->ucTxnDateTime,DongleOut->ucTxnDateTime,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	if(TxnReqOut->ucPurseVersionNumber == MIFARE)
		memcpy(APIOut->ucMAC,DongleOut->ucMAC_HCrypto,sizeof(APIOut->ucMAC));//MAC
	else
		memset(APIOut->ucMAC,0x00,sizeof(APIOut->ucMAC));//MAC
	memcpy(APIOut->ucConfirmCode,DongleOut->ucConfirmCode,sizeof(APIOut->ucConfirmCode));//Confirm Code
	if(TxnReqOut->ucPurseVersionNumber == MIFARE)
		memcpy(APIOut->ucCTAC,&DongleOut->ucMAC_HCrypto[10],sizeof(APIOut->ucMAC));//CTAC
	else
		memset(APIOut->ucCTAC,0x00,sizeof(APIOut->ucMAC));//CTAC
	memcpy(APIOut->ucCardID,TxnReqOut->ucCardID,sizeof(APIOut->ucCardID));//Card ID
	memcpy(APIOut->ucEV,DongleOut->ucEV,sizeof(APIOut->ucEV));//EV
	memcpy(APIOut->ucTxnSN,DongleOut->ucTxnSN,sizeof(APIOut->ucTxnSN));//Txn SN
	memcpy(APIOut->ucCPUDeviceID,TxnReqOut->ucCPUDeviceID,sizeof(APIOut->ucCPUDeviceID));//CPU Device ID
	memcpy(APIOut->ucPID,TxnReqOut->ucPID,sizeof(APIOut->ucPID));//Purse ID
	memcpy(APIOut->ucSignature,DongleOut->ucSignature,sizeof(APIOut->ucSignature));//Signature
	if(TxnReqOut->ucPurseVersionNumber != MIFARE)
	{
		APIOut->ucHashType = DongleOut->ucMAC_HCrypto[0];//Hash Type
		APIOut->ucHostAdminKVN = DongleOut->ucMAC_HCrypto[1];//Host Admin KVN
		memcpy(APIOut->ucCPUMAC,&DongleOut->ucMAC_HCrypto[2],sizeof(APIOut->ucCPUMAC));//CPU MAC
	}
	else
	{
		APIOut->ucHashType = 0x00;
		APIOut->ucHostAdminKVN = 0x00;
		memset(APIOut->ucCPUMAC,0x00,sizeof(APIOut->ucCPUMAC));//CPU MAC
	}

	return(inTMOutLen);
}

int inBuildAuthTxnOnlineData1(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
AuthTxnOnline1_TM_Out *APIOut = (AuthTxnOnline1_TM_Out *)API_Out;
AuthTxnOnline_APDU_In *DongleIn = (AuthTxnOnline_APDU_In *)Dongle_In;
AuthTxnOnline_APDU_Out *DongleOut = (AuthTxnOnline_APDU_Out *)Dongle_Out;
TxnReqOnline_APDU_Out *TxnReqOut = (TxnReqOnline_APDU_Out *)Dongle_ReqOut;

	inTMOutLen = sizeof(AuthTxnOnline1_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	vdUIntToAsc(TxnReqOut->ucCardID,sizeof(TxnReqOut->ucCardID),APIOut->stSTMC_t.anCardID,sizeof(APIOut->stSTMC_t.anCardID),FALSE,' ',10);//n_Card Physical ID
	fnBINTODEVASC(TxnReqOut->ucDeviceID,APIOut->stSTMC_t.anDeviceID,sizeof(APIOut->stSTMC_t.anDeviceID),MIFARE);//n_TXN Device ID
	fnBINTODEVASC(TxnReqOut->ucCPUDeviceID,APIOut->stSTMC_t.anCPUDeviceID,sizeof(APIOut->stSTMC_t.anCPUDeviceID),LEVEL2);//n_TXN Device ID
    vdIntToAsc(TxnReqOut->ucEV,sizeof(TxnReqOut->ucEV),APIOut->stSTMC_t.anEVBeforeTxn,sizeof(APIOut->stSTMC_t.anEVBeforeTxn),FALSE,' ',10);//n_EV Before TXN
	//vdCountEVBefore(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.anEVBeforeTxn,sizeof(APIOut->stSTMC_t.anEVBeforeTxn),TxnReqOut->ucMsgType);//n_EV Before TXN
	vdUIntToAsc(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),APIOut->stSTMC_t.anTxnSN,sizeof(APIOut->stSTMC_t.anTxnSN),FALSE,' ',10);//n_TXN SNum
	vdIntToAsc(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),APIOut->stSTMC_t.anTxnAmt,sizeof(APIOut->stSTMC_t.anTxnAmt),FALSE,' ',10);//n_TXN AMT
	vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.anEV,sizeof(APIOut->stSTMC_t.anEV),FALSE,' ',10);//n_EV
	vdBuildProcessingCode(inTxnType,TxnReqOut->ucMsgType,TxnReqOut->ucSubType,TxnReqOut->ucPersonalProfile,APIOut->stSTMC_t.anProcessignCode);//Processing Code
	fnUnPack(&TxnReqOut->ucMsgType,sizeof(TxnReqOut->ucMsgType),APIOut->stSTMC_t.unMsgType);//Msg Type
	fnUnPack(&TxnReqOut->ucSubType,sizeof(TxnReqOut->ucSubType),APIOut->stSTMC_t.unSubType);//Sub Type
	fnUnPack(TxnReqOut->ucDeviceID,sizeof(TxnReqOut->ucDeviceID),APIOut->stSTMC_t.unDeviceID);//Device ID
	fnUnPack(&TxnReqOut->ucSPID,sizeof(TxnReqOut->ucSPID),APIOut->stSTMC_t.unSPID);//SP ID
	fnUnPack(DongleOut->ucTxnDateTime,sizeof(DongleOut->ucTxnDateTime),APIOut->stSTMC_t.unTxnDateTime);//Txn Date Time
	fnUnPack(&TxnReqOut->ucCardType,sizeof(TxnReqOut->ucCardType),APIOut->stSTMC_t.unCardType);//Card Type
	fnUnPack(&TxnReqOut->ucPersonalProfile,sizeof(TxnReqOut->ucPersonalProfile),APIOut->stSTMC_t.unPersonalProfile);//Personal Profile
	fnUnPack(&TxnReqOut->ucLocationID,sizeof(TxnReqOut->ucLocationID),APIOut->stSTMC_t.unLocationID);//Location ID
	fnUnPack(TxnReqOut->ucCardID,sizeof(TxnReqOut->ucCardID),APIOut->stSTMC_t.unCardID);//CPU Card ID
	fnUnPack(&TxnReqOut->ucIssuerCode,sizeof(TxnReqOut->ucIssuerCode),APIOut->stSTMC_t.unIssuerCode);//Issuer Code
	fnUnPack(&TxnReqOut->ucBankCode,sizeof(TxnReqOut->ucBankCode),APIOut->stSTMC_t.unBankCode);//Bank Code
	fnUnPack(TxnReqOut->ucLoyaltyCounter,sizeof(TxnReqOut->ucLoyaltyCounter),APIOut->stSTMC_t.unLoyaltyCounter);//Loyalty Counter
	if(TxnReqOut->ucPurseVersionNumber == MIFARE)
	{
		if (TxnReqOut->ucReaderFWVersion[5] >= 0x80) //match sam的mfrc改用device id
			memcpy(&DongleOut->ucMAC_HCrypto[6], TxnReqOut->ucDeviceID, 4);
		fnUnPack(DongleOut->ucMAC_HCrypto,10,APIOut->stSTMC_t.unMAC);//MAC
			// 配對SAM用Device id
		//if (TxnReqOut->ucReaderFWVersion[5] >= 0x80)
		//	fnUnPack(TxnReqOut->ucDeviceID, sizeof(TxnReqOut->ucDeviceID), &APIOut->stSTMC_t.unMAC[12]);
	}
	else
		memset(APIOut->stSTMC_t.unMAC,0x30,sizeof(APIOut->stSTMC_t.unMAC));//MAC
	fnUnPack(TxnReqOut->ucExpiryDate,sizeof(TxnReqOut->ucExpiryDate),APIOut->stSTMC_t.unOrgExpiryDate);//Org Expiry Date
	fnUnPack(DongleIn->ucExpiryDate,sizeof(DongleIn->ucExpiryDate),APIOut->stSTMC_t.unNewExpiryDate);//New Expiry Date
	fnUnPack(&TxnReqOut->ucAreaCode,sizeof(TxnReqOut->ucAreaCode),APIOut->stSTMC_t.unAreaCode);//Area Code
	fnUnPack(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),APIOut->stSTMC_t.unTxnAmt);//Txn Amt
	fnUnPack(DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.unEV);//EV
	fnUnPack(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),APIOut->stSTMC_t.unTxnSN);//Txn SN
	fnUnPack(TxnReqOut->ucCPUDeviceID,sizeof(TxnReqOut->ucCPUDeviceID),APIOut->stSTMC_t.unCPUDeviceID);//CPU Device ID
	fnUnPack(TxnReqOut->ucCPUSPID,sizeof(TxnReqOut->ucCPUSPID),APIOut->stSTMC_t.unCPUSPID);//CPU SP ID
	fnUnPack(TxnReqOut->ucCPULocationID,sizeof(TxnReqOut->ucCPULocationID),APIOut->stSTMC_t.unCPULocationID);//CPU Location ID
	fnUnPack(TxnReqOut->ucPID,sizeof(TxnReqOut->ucPID),APIOut->stSTMC_t.unPID);//Purse ID
	fnUnPack(TxnReqOut->ucCTC,sizeof(TxnReqOut->ucCTC),APIOut->stSTMC_t.unCTC);//CTC
	fnUnPack(TxnReqOut->ucProfileExpiryDate,sizeof(TxnReqOut->ucProfileExpiryDate),APIOut->stSTMC_t.unOrgProfileExpiryDate);//Org Profile Expiry Date
	fnUnPack(DongleIn->ucProfileExpiryDate,sizeof(DongleIn->ucProfileExpiryDate),APIOut->stSTMC_t.unNewProfileExpiryDate);//New Profile Expiry Date
	fnUnPack(TxnReqOut->ucSubAreaCode,sizeof(TxnReqOut->ucSubAreaCode),APIOut->stSTMC_t.unSubAreaCode);//Sub Area Code
	fnUnPack(TxnReqOut->ucTxnSN,sizeof(TxnReqOut->ucTxnSN),APIOut->stSTMC_t.unTxnSNBeforeTxn);//Txn SN Before Txn
	fnUnPack(TxnReqOut->ucEV,sizeof(TxnReqOut->ucEV),APIOut->stSTMC_t.unEVBeforeTxn);//EV Before Txn
	fnUnPack(TxnReqOut->ucDeposit,sizeof(TxnReqOut->ucDeposit),APIOut->stSTMC_t.unDeposit);//Deposit
	fnUnPack(&TxnReqOut->ucTxnMode,sizeof(TxnReqOut->ucTxnMode),APIOut->stSTMC_t.unTxnMode);//Txn Mode
	fnUnPack(&TxnReqOut->ucTxnQuqlifier,sizeof(TxnReqOut->ucTxnQuqlifier),APIOut->stSTMC_t.unTxnQuqlifier);//Txn Quqlifier
	fnUnPack(&TxnReqOut->ucSignatureKeyKVN,sizeof(TxnReqOut->ucSignatureKeyKVN),APIOut->stSTMC_t.unSignatureKeyKVN);//Signature Key KVN
	fnUnPack(DongleOut->ucSignature,sizeof(DongleOut->ucSignature),APIOut->stSTMC_t.unSignature);//Signature
	fnUnPack(DongleOut->ucCPUSAMID,sizeof(DongleOut->ucCPUSAMID),APIOut->stSTMC_t.unCPUSAMID);//SAM ID
	if(TxnReqOut->ucPurseVersionNumber != MIFARE)
		fnUnPack(&DongleOut->ucMAC_HCrypto[0],1,APIOut->stSTMC_t.unHashType);//Hash Type
	else
		memset(APIOut->stSTMC_t.unHashType,0x30,sizeof(APIOut->stSTMC_t.unHashType));
	if(TxnReqOut->ucPurseVersionNumber != MIFARE)
	{
		fnUnPack(&DongleOut->ucMAC_HCrypto[1],1,APIOut->stSTMC_t.unHostAdminKVN);//Host Admin KVN
		fnUnPack(&DongleOut->ucMAC_HCrypto[2],16,APIOut->stSTMC_t.unCPUMAC);//MAC
	}
	else
	{
		memset(APIOut->stSTMC_t.unHostAdminKVN,0x30,sizeof(APIOut->stSTMC_t.unHostAdminKVN));
		memset(APIOut->stSTMC_t.unCPUMAC,0x30,sizeof(APIOut->stSTMC_t.unCPUMAC));//MAC
	}
	fnUnPack(&TxnReqOut->ucPurseVersionNumber,sizeof(TxnReqOut->ucPurseVersionNumber),APIOut->stSTMC_t.unPurseVersionNumber);//Purse Version Number
	fnUnPack(&TxnReqOut->ucCardIDLen,sizeof(TxnReqOut->ucCardIDLen),APIOut->stSTMC_t.unCardIDLen);//Card ID Len
	memcpy(APIOut->stSTMC_t.ucTMLocationID,stStore_DataInfo_t.ucTMLocationID,sizeof(stStore_DataInfo_t.ucTMLocationID));//TM Location ID
	memcpy(APIOut->stSTMC_t.ucTMID,stStore_DataInfo_t.ucTMID,sizeof(stStore_DataInfo_t.ucTMID));//TM ID
	UnixToDateTime((BYTE*)DongleOut->ucTxnDateTime,(BYTE*)APIOut->stSTMC_t.ucTMTxnDateTime,14);//TM Txn Date Time
	memcpy(APIOut->stSTMC_t.ucTMSerialNumber,stStore_DataInfo_t.ucTMSerialNumber,sizeof(stStore_DataInfo_t.ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->stSTMC_t.ucTMAgentNumber,stStore_DataInfo_t.ucTMAgentNumber,sizeof(stStore_DataInfo_t.ucTMAgentNumber));//TM Agent Number
	if(srTxnData.srIngData.lnECCAmt1 + srTxnData.srIngData.lnECCAmt2 + srTxnData.srIngData.lnECCAmt3 + srTxnData.srIngData.lnECCAmt4 + srTxnData.srIngData.lnECCAmt5 == 0L)
	{
		memset(APIOut->unSTMCAmt1,0x00,sizeof(APIOut->unSTMCAmt1));
		memset(APIOut->unSTMCAmt2,0x00,sizeof(APIOut->unSTMCAmt2));
		memset(APIOut->unSTMCAmt3,0x00,sizeof(APIOut->unSTMCAmt3));
		memset(APIOut->unSTMCAmt4,0x00,sizeof(APIOut->unSTMCAmt4));
		memset(APIOut->unSTMCAmt5,0x00,sizeof(APIOut->unSTMCAmt5));
	}
	else
	{
		fnUnPack((BYTE *)&srTxnData.srIngData.lnECCAmt1,2,APIOut->unSTMCAmt1);//TxnAmt1
		fnUnPack((BYTE *)&srTxnData.srIngData.lnECCAmt2,2,APIOut->unSTMCAmt2);//TxnAmt2
		fnUnPack((BYTE *)&srTxnData.srIngData.lnECCAmt3,2,APIOut->unSTMCAmt3);//TxnAmt3
		fnUnPack((BYTE *)&srTxnData.srIngData.lnECCAmt4,2,APIOut->unSTMCAmt4);//TxnAmt4
		fnUnPack((BYTE *)&srTxnData.srIngData.lnECCAmt5,2,APIOut->unSTMCAmt5);//TxnAmt5
	}
	//帳務資料結束
	//Confirm資料開始
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",79);//Data Len
	vdBuildProcessingCode(inTxnType,TxnReqOut->ucMsgType,TxnReqOut->ucSubType,TxnReqOut->ucPersonalProfile,APIOut->ucProcessignCode);//Processing Code
	APIOut->ucMsgType = TxnReqOut->ucMsgType;//Msg Type
	APIOut->ucSubType = TxnReqOut->ucSubType;//Sub Type
	memcpy(APIOut->ucDeviceID,TxnReqOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	memcpy(APIOut->ucTxnDateTime,DongleOut->ucTxnDateTime,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	if(TxnReqOut->ucPurseVersionNumber == MIFARE)
		memcpy(APIOut->ucMAC,DongleOut->ucMAC_HCrypto,sizeof(APIOut->ucMAC));//MAC
	else
		memset(APIOut->ucMAC,0x00,sizeof(APIOut->ucMAC));//MAC
	memcpy(APIOut->ucConfirmCode,DongleOut->ucConfirmCode,sizeof(APIOut->ucConfirmCode));//Confirm Code
	if(TxnReqOut->ucPurseVersionNumber == MIFARE)
		memcpy(APIOut->ucCTAC,&DongleOut->ucMAC_HCrypto[10],sizeof(APIOut->ucMAC));//CTAC
	else
		memset(APIOut->ucCTAC,0x00,sizeof(APIOut->ucMAC));//CTAC
	memcpy(APIOut->ucCardID,TxnReqOut->ucCardID,sizeof(APIOut->ucCardID));//Card ID
	memcpy(APIOut->ucEV,DongleOut->ucEV,sizeof(APIOut->ucEV));//EV
	memcpy(APIOut->ucTxnSN,DongleOut->ucTxnSN,sizeof(APIOut->ucTxnSN));//Txn SN
	memcpy(APIOut->ucCPUDeviceID,TxnReqOut->ucCPUDeviceID,sizeof(APIOut->ucCPUDeviceID));//CPU Device ID
	memcpy(APIOut->ucPID,TxnReqOut->ucPID,sizeof(APIOut->ucPID));//Purse ID
	memcpy(APIOut->ucSignature,DongleOut->ucSignature,sizeof(APIOut->ucSignature));//Signature
	if(TxnReqOut->ucPurseVersionNumber != MIFARE)
	{
		APIOut->ucHashType = DongleOut->ucMAC_HCrypto[0];//Hash Type
		APIOut->ucHostAdminKVN = DongleOut->ucMAC_HCrypto[1];//Host Admin KVN
		memcpy(APIOut->ucCPUMAC,&DongleOut->ucMAC_HCrypto[2],sizeof(APIOut->ucCPUMAC));//CPU MAC
	}
	else
	{
		APIOut->ucHashType = 0x00;
		APIOut->ucHostAdminKVN = 0x00;
		memset(APIOut->ucCPUMAC,0x00,sizeof(APIOut->ucCPUMAC));//CPU MAC
	}

	return(inTMOutLen);
}

int inBuildTxnReqOfflineData1(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
TxnReqOffline_TM_Out_1 *APIOut = (TxnReqOffline_TM_Out_1 *)API_Out;
//TxnReqOffline_APDU_In *DongleIn = (TxnReqOffline_APDU_In *)Dongle_In;
TxnReqOffline_APDU_Out *DongleOut = (TxnReqOffline_APDU_Out *)Dongle_Out;

	inTMOutLen = sizeof(TxnReqOffline_TM_Out_1);
	memset(APIOut,0x00,inTMOutLen);

	vdUIntToAsc(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),APIOut->anCardID,sizeof(APIOut->anCardID),FALSE,' ',10);//n_Card Physical ID
	fnBINTODEVASC(DongleOut->ucDeviceID,APIOut->anDeviceID,sizeof(APIOut->anDeviceID),MIFARE);//n_TXN Device ID
	fnBINTODEVASC(DongleOut->ucCPUDeviceID,APIOut->anCPUDeviceID,sizeof(APIOut->anCPUDeviceID),LEVEL2);//n_TXN Device ID
	vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->anEVBeforeTxn,sizeof(APIOut->anEVBeforeTxn),FALSE,' ',10);//n_EV Before TXN
	vdUIntToAsc(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),APIOut->anTxnSN,sizeof(APIOut->anTxnSN),FALSE,' ',10);//n_TXN SNum
	vdUIntToAsc(DongleOut->ucTxnAmt,sizeof(DongleOut->ucTxnAmt),APIOut->anTxnAmt,sizeof(APIOut->anTxnAmt),FALSE,' ',10);//n_TXN AMT
	APIOut->ucAutoLoad = DongleOut->bAutoLoad;//AutoLoad Flag
	vdUIntToAsc(DongleOut->ucAutoLoadAmt,sizeof(DongleOut->ucAutoLoadAmt),APIOut->anAutoLoadAmt,sizeof(APIOut->anAutoLoadAmt),FALSE,' ',10);//n_AutoLoad Amount

	return(inTMOutLen);
}

int inBuildTxnReqOfflineData2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
BYTE ucLoyaltyCounter[2];
//#ifdef READER_MODE
BYTE bBuf[10];
//#endif
TxnReqOffline_TM_Out_2 *APIOut = (TxnReqOffline_TM_Out_2 *)API_Out;
TxnReqOffline_APDU_In *DongleIn = (TxnReqOffline_APDU_In *)Dongle_In;
TxnReqOffline_APDU_Out *DongleOut = (TxnReqOffline_APDU_Out *)Dongle_Out;
	
	inTMOutLen = sizeof(TxnReqOffline_TM_Out_2);
	memset(APIOut,0x00,inTMOutLen);
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",inTMOutLen - 7);//Data Len
	vdBuildProcessingCode(inTxnType,DongleOut->ucMsgType,DongleOut->ucSubType,DongleOut->ucPersonalProfile,APIOut->ucProcessignCode);//Processing Code
	vdBuildBLCVersion(APIOut->ucBLVersion);
	//fnASCTOBIN(APIOut->ucBLVersion,gsBlackListVersion,5,sizeof(APIOut->ucBLVersion),DECIMAL);//Black List
	APIOut->ucMsgType = DongleOut->ucMsgType;//Msg Type
	APIOut->ucSubType = DongleOut->ucSubType;//Sub Type
	memcpy(APIOut->ucDeviceID,DongleOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	APIOut->ucSPID = DongleOut->ucSPID;//SP ID
	memcpy(APIOut->ucTxnDateTime,DongleIn->ucTxnDateTime,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	APIOut->ucCardType = DongleOut->ucCardType;//Card Type
	APIOut->ucPersonalProfile = DongleOut->ucPersonalProfile;//Personal Profile
	APIOut->ucIssuerCode = DongleOut->ucIssuerCode;//Issuer Code
	memcpy(APIOut->ucExpiryDate,DongleOut->ucExpiryDate,sizeof(APIOut->ucExpiryDate));//Expiry Date
	APIOut->ucAreaCode = DongleOut->ucAreaCode;//Area Code
	memcpy(APIOut->ucTMID,DongleIn->ucTMID,sizeof(APIOut->ucTMID));//TM ID
	memcpy(APIOut->ucTMTxnDateTime,DongleIn->ucTMTxnDateTime,sizeof(APIOut->ucTMTxnDateTime));//TM Txn Date Time
	memcpy(APIOut->ucTMSerialNumber,DongleOut->ucTMSerialNumber,sizeof(APIOut->ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->ucTMAgentNumber,DongleIn->ucTMAgentNumber,sizeof(APIOut->ucTMAgentNumber));//TM Agent Number
	if(DongleOut->ucPurseVersionNumber == MIFARE)
	{
		memcpy(APIOut->ucSTAC,DongleOut->ucSID_STAC,sizeof(APIOut->ucSTAC));//STAC
		APIOut->ucKeyVersion = DongleOut->ucCPDKVN_SAMKVN;//SAM KVN
		memcpy(APIOut->ucSAMID,&DongleOut->ucCPD_SAMID[2],sizeof(APIOut->ucSAMID));//SAM ID
		memcpy(APIOut->ucSAMSN,&DongleOut->ucCPD_SAMID[10],sizeof(APIOut->ucSAMSN));//SAM SN
		memcpy(APIOut->ucSAMCRN,DongleOut->ucCPDRAN_SAMCRN,sizeof(APIOut->ucSAMCRN));//SAM CRN
	}
	else
	{
		memset(APIOut->ucSTAC,0x00,sizeof(APIOut->ucSTAC));//STAC
		APIOut->ucKeyVersion = 0;//SAM KVN
		memset(APIOut->ucSAMID,0x00,sizeof(APIOut->ucSAMID));//SAM ID
		memset(APIOut->ucSAMSN,0x00,sizeof(APIOut->ucSAMSN));//SAM SN
		memset(APIOut->ucSAMCRN,0x00,sizeof(APIOut->ucSAMCRN));//SAM CRN
	}
	memcpy(APIOut->ucReaderFWVersion,DongleOut->ucReaderFWVersion,sizeof(APIOut->ucReaderFWVersion));//Reader FW Version
	memcpy(APIOut->ucCardID,DongleOut->ucCardID,sizeof(APIOut->ucCardID));//Card ID
	memcpy(APIOut->ucTxnAmt,DongleOut->ucTxnAmt,sizeof(APIOut->ucTxnAmt));//Txn Amt
	memcpy(APIOut->ucCPUDeviceID,DongleOut->ucCPUDeviceID,sizeof(APIOut->ucCPUDeviceID));//CPU Device ID
	memcpy(APIOut->ucCPUSPID,DongleOut->ucCPUSPID,sizeof(APIOut->ucCPUSPID));//CPU SP ID
	memcpy(APIOut->ucCPULocationID,DongleOut->ucCPULocationID,sizeof(APIOut->ucCPULocationID));//CPU Location ID
	memcpy(APIOut->ucPID,DongleOut->ucPID,sizeof(APIOut->ucPID));//CPU Purse ID
	memcpy(APIOut->ucCTC,DongleOut->ucCTC,sizeof(APIOut->ucCTC));//CPU CTC
	memcpy(APIOut->ucProfileExpiryDate,DongleOut->ucProfileExpiryDate,sizeof(APIOut->ucProfileExpiryDate));//Profile Expiry Date
	memcpy(APIOut->ucSubAreaCode,DongleOut->ucSubAreaCode,sizeof(APIOut->ucSubAreaCode));//Sub Area Code
	memcpy(APIOut->ucTxnSN,DongleOut->ucTxnSN,sizeof(APIOut->ucTxnSN));//Txn SN
	memcpy(APIOut->ucEV,DongleOut->ucEV,sizeof(APIOut->ucEV));//EV
	APIOut->ucCPUAdminKeyKVN = DongleOut->ucCPUAdminKeyKVN;//CPU Admin Key KVN
	APIOut->ucCreditKeyKVN = DongleOut->ucCreditKeyKVN;//Credit Key KVN
	APIOut->ucCPUIssuerKeyKVN = DongleOut->ucCPUIssuerKeyKVN;//CPU Issuer Key KVN
	APIOut->ucTxnMode = DongleOut->ucTxnMode;//Txn Mode
	APIOut->ucTxnQuqlifier = DongleOut->ucTxnQuqlifier;//Txn Quqlifier
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		memcpy(APIOut->ucCPUSAMID,&DongleOut->ucCPD_SAMID[2],sizeof(APIOut->ucCPUSAMID));//CPU SAM ID
	else
		memcpy(APIOut->ucCPUSAMID,DongleOut->ucSID_STAC,sizeof(APIOut->ucCPUSAMID));//CPU SAM ID
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		APIOut->ucHostAdminKVN = 0x00;//Host Admin KVN
	else
		APIOut->ucHostAdminKVN = DongleOut->ucBankCode_HostAdminKVN;//Host Admin KVN
	APIOut->ucPurseVersionNumber = DongleOut->ucPurseVersionNumber;//Purse Version Number
	memcpy(APIOut->ucTMLocationID,DongleIn->ucTMLocationID,sizeof(APIOut->ucTMLocationID));//TM Location ID
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		memset(APIOut->ucCACrypto,0x00,sizeof(APIOut->ucCACrypto));//CACrypto後8碼
	else
		memcpy(APIOut->ucCACrypto,DongleOut->ucSVCrypto,sizeof(APIOut->ucCACrypto));//CACrypto
	memcpy(APIOut->ucCardOneDayQuota,DongleOut->ucCardOneDayQuota,sizeof(APIOut->ucCardOneDayQuota));//Card One Day Quota
	memcpy(APIOut->ucCardOneDayQuotaDate,DongleOut->ucCardOneDayQuotaDate,sizeof(APIOut->ucCardOneDayQuotaDate));//Card One Day Quota Date
	memset(ucLoyaltyCounter,0x00,sizeof(ucLoyaltyCounter));
	vdBuildCardAVRData(&APIOut->stCardAVRInfo_t,&DongleOut->stLastCreditTxnLogInfo_t,DongleOut->ucCardID,DongleOut->ucIssuerCode,DongleOut->ucCPUSPID,DongleOut->ucCPULocationID,DongleOut->ucPID,ucLoyaltyCounter);//Card AVR Info
	APIOut->ucCardIDLen = DongleOut->ucCardIDLen;//Card ID Len

//#ifdef READER_MODE
	memset(bBuf,0x00,sizeof(bBuf));
	vdStoreDeviceID(DongleOut->ucDeviceID,DongleOut->ucDeviceID[2],DongleOut->ucLocationID,DongleOut->ucCPUDeviceID,&DongleOut->ucCPUDeviceID[3],DongleOut->ucCPULocationID,gTmpData.ucReaderID,DongleOut->ucReaderFWVersion,DongleOut->ucCPUSAMID);
//#endif

	return(inTMOutLen);
}

int inBuildTxnReqOfflineData3(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
BYTE ucLoyaltyCounter[2];
//#ifdef READER_MODE
BYTE bBuf[10];
//#endif
TxnReqOffline_TM_Out_3 *APIOut = (TxnReqOffline_TM_Out_3 *)API_Out;
TxnReqOffline_APDU_In *DongleIn = (TxnReqOffline_APDU_In *)Dongle_In;
TxnReqOffline_APDU_Out *DongleOut = (TxnReqOffline_APDU_Out *)Dongle_Out;
	
	inTMOutLen = sizeof(TxnReqOffline_TM_Out_3);
	memset(APIOut,0x00,inTMOutLen);
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",inTMOutLen - 7);//Data Len
	vdBuildProcessingCode(inTxnType,DongleOut->ucMsgType,DongleOut->ucSubType,DongleOut->ucPersonalProfile,APIOut->ucProcessignCode);//Processing Code
	vdBuildBLCVersion(APIOut->ucBLVersion);
	//fnASCTOBIN(APIOut->ucBLVersion,gsBlackListVersion,5,sizeof(APIOut->ucBLVersion),DECIMAL);//Black List
	APIOut->ucMsgType = DongleOut->ucMsgType;//Msg Type
	APIOut->ucSubType = DongleOut->ucSubType;//Sub Type
	memcpy(APIOut->ucDeviceID,DongleOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	APIOut->ucSPID = DongleOut->ucSPID;//SP ID
	memcpy(APIOut->ucTxnDateTime,DongleIn->ucTxnDateTime,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	APIOut->ucCardType = DongleOut->ucCardType;//Card Type
	APIOut->ucPersonalProfile = DongleOut->ucPersonalProfile;//Personal Profile
	APIOut->ucIssuerCode = DongleOut->ucIssuerCode;//Issuer Code
	memcpy(APIOut->ucExpiryDate,DongleOut->ucExpiryDate,sizeof(APIOut->ucExpiryDate));//Expiry Date
	APIOut->ucAreaCode = DongleOut->ucAreaCode;//Area Code
	memcpy(APIOut->ucTMID,DongleIn->ucTMID,sizeof(APIOut->ucTMID));//TM ID
	memcpy(APIOut->ucTMTxnDateTime,DongleIn->ucTMTxnDateTime,sizeof(APIOut->ucTMTxnDateTime));//TM Txn Date Time
	memcpy(APIOut->ucTMSerialNumber,DongleOut->ucTMSerialNumber,sizeof(APIOut->ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->ucTMAgentNumber,DongleIn->ucTMAgentNumber,sizeof(APIOut->ucTMAgentNumber));//TM Agent Number
	if(DongleOut->ucPurseVersionNumber == MIFARE)
	{
		memcpy(APIOut->ucSTAC,DongleOut->ucSID_STAC,sizeof(APIOut->ucSTAC));//STAC
		APIOut->ucKeyVersion = DongleOut->ucCPDKVN_SAMKVN;//SAM KVN
		memcpy(APIOut->ucSAMID,&DongleOut->ucCPD_SAMID[2],sizeof(APIOut->ucSAMID));//SAM ID
		memcpy(APIOut->ucSAMSN,&DongleOut->ucCPD_SAMID[10],sizeof(APIOut->ucSAMSN));//SAM SN
		memcpy(APIOut->ucSAMCRN,DongleOut->ucCPDRAN_SAMCRN,sizeof(APIOut->ucSAMCRN));//SAM CRN
	}
	else
	{
		memset(APIOut->ucSTAC,0x00,sizeof(APIOut->ucSTAC));//STAC
		APIOut->ucKeyVersion = 0;//SAM KVN
		memset(APIOut->ucSAMID,0x00,sizeof(APIOut->ucSAMID));//SAM ID
		memset(APIOut->ucSAMSN,0x00,sizeof(APIOut->ucSAMSN));//SAM SN
		memset(APIOut->ucSAMCRN,0x00,sizeof(APIOut->ucSAMCRN));//SAM CRN
	}
	memcpy(APIOut->ucReaderFWVersion,DongleOut->ucReaderFWVersion,sizeof(APIOut->ucReaderFWVersion));//Reader FW Version
	memcpy(APIOut->ucCardID,DongleOut->ucCardID,sizeof(APIOut->ucCardID));//Card ID
	memcpy(APIOut->ucTxnAmt,DongleOut->ucTxnAmt,sizeof(APIOut->ucTxnAmt));//Txn Amt
	memcpy(APIOut->ucCPUDeviceID,DongleOut->ucCPUDeviceID,sizeof(APIOut->ucCPUDeviceID));//CPU Device ID
	memcpy(APIOut->ucCPUSPID,DongleOut->ucCPUSPID,sizeof(APIOut->ucCPUSPID));//CPU SP ID
	memcpy(APIOut->ucCPULocationID,DongleOut->ucCPULocationID,sizeof(APIOut->ucCPULocationID));//CPU Location ID
	memcpy(APIOut->ucPID,DongleOut->ucPID,sizeof(APIOut->ucPID));//CPU Purse ID
	memcpy(APIOut->ucCTC,DongleOut->ucCTC,sizeof(APIOut->ucCTC));//CPU CTC
	memcpy(APIOut->ucProfileExpiryDate,DongleOut->ucProfileExpiryDate,sizeof(APIOut->ucProfileExpiryDate));//Profile Expiry Date
	memcpy(APIOut->ucSubAreaCode,DongleOut->ucSubAreaCode,sizeof(APIOut->ucSubAreaCode));//Sub Area Code
	memcpy(APIOut->ucTxnSN,DongleOut->ucTxnSN,sizeof(APIOut->ucTxnSN));//Txn SN
	memcpy(APIOut->ucEV,DongleOut->ucEV,sizeof(APIOut->ucEV));//EV
	APIOut->ucCPUAdminKeyKVN = DongleOut->ucCPUAdminKeyKVN;//CPU Admin Key KVN
	APIOut->ucCreditKeyKVN = DongleOut->ucCreditKeyKVN;//Credit Key KVN
	APIOut->ucCPUIssuerKeyKVN = DongleOut->ucCPUIssuerKeyKVN;//CPU Issuer Key KVN
	APIOut->ucTxnMode = DongleOut->ucTxnMode;//Txn Mode
	APIOut->ucTxnQuqlifier = DongleOut->ucTxnQuqlifier;//Txn Quqlifier
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		memcpy(APIOut->ucCPUSAMID,&DongleOut->ucCPD_SAMID[2],sizeof(APIOut->ucCPUSAMID));//CPU SAM ID
	else
		memcpy(APIOut->ucCPUSAMID,DongleOut->ucSID_STAC,sizeof(APIOut->ucCPUSAMID));//CPU SAM ID
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		APIOut->ucHostAdminKVN = 0x00;//Host Admin KVN
	else
		APIOut->ucHostAdminKVN = DongleOut->ucBankCode_HostAdminKVN;//Host Admin KVN
	APIOut->ucPurseVersionNumber = DongleOut->ucPurseVersionNumber;//Purse Version Number
	memcpy(APIOut->ucTMLocationID,DongleIn->ucTMLocationID,sizeof(APIOut->ucTMLocationID));//TM Location ID
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		memset(APIOut->ucCACrypto,0x00,sizeof(APIOut->ucCACrypto));//CACrypto後8碼
	else
		memcpy(APIOut->ucCACrypto,DongleOut->ucSVCrypto,sizeof(APIOut->ucCACrypto));//CACrypto
	//memcpy(APIOut->ucCardOneDayQuota,DongleOut->ucCardOneDayQuota,sizeof(APIOut->ucCardOneDayQuota));//Card One Day Quota
	//memcpy(APIOut->ucCardOneDayQuotaDate,DongleOut->ucCardOneDayQuotaDate,sizeof(APIOut->ucCardOneDayQuotaDate));//Card One Day Quota Date
	memset(ucLoyaltyCounter,0x00,sizeof(ucLoyaltyCounter));
	vdBuildCardAVRData(&APIOut->stCardAVRInfo_t,&DongleOut->stLastCreditTxnLogInfo_t,DongleOut->ucCardID,DongleOut->ucIssuerCode,DongleOut->ucCPUSPID,DongleOut->ucCPULocationID,DongleOut->ucPID,ucLoyaltyCounter);//Card AVR Info
	APIOut->ucCardIDLen = DongleOut->ucCardIDLen;//Card ID Len

//#ifdef READER_MODE
	memset(bBuf,0x00,sizeof(bBuf));
	vdStoreDeviceID(DongleOut->ucDeviceID,DongleOut->ucDeviceID[2],DongleOut->ucLocationID,DongleOut->ucCPUDeviceID,&DongleOut->ucCPUDeviceID[3],DongleOut->ucCPULocationID,gTmpData.ucReaderID,DongleOut->ucReaderFWVersion,DongleOut->ucCPUSAMID);
//#endif

	return(inTMOutLen);
}

int inBuildTxnReqOfflineData4(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
	int inTMOutLen;
	BYTE ucLoyaltyCounter[2];
	//#ifdef READER_MODE
	BYTE bBuf[10];
	//#endif
	TxnReqOffline_TM_Out_3 *APIOut = (TxnReqOffline_TM_Out_3 *)API_Out;
	TxnReqOffline_APDU_In *DongleIn = (TxnReqOffline_APDU_In *)Dongle_In;
	TxnReqOffline_APDU_Out *DongleOut = (TxnReqOffline_APDU_Out *)Dongle_Out;
	
	inTMOutLen = sizeof(TxnReqOffline_TM_Out_3);
	memset(APIOut,0x00,inTMOutLen);
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",inTMOutLen - 7);//Data Len
	vdBuildProcessingCode(inTxnType,DongleOut->ucMsgType,DongleOut->ucSubType,DongleOut->ucPersonalProfile,APIOut->ucProcessignCode);//Processing Code
	vdBuildBLCVersion(APIOut->ucBLVersion);
	//fnASCTOBIN(APIOut->ucBLVersion,gsBlackListVersion,5,sizeof(APIOut->ucBLVersion),DECIMAL);//Black List
	APIOut->ucMsgType = DongleOut->ucMsgType;//Msg Type
	APIOut->ucSubType = DongleOut->ucSubType;//Sub Type
	memcpy(APIOut->ucDeviceID,DongleOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	APIOut->ucSPID = DongleOut->ucSPID;//SP ID
	memcpy(APIOut->ucTxnDateTime,DongleIn->ucTxnDateTime,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	APIOut->ucCardType = DongleOut->ucCardType;//Card Type
	APIOut->ucPersonalProfile = DongleOut->ucPersonalProfile;//Personal Profile
	APIOut->ucIssuerCode = DongleOut->ucIssuerCode;//Issuer Code
	memcpy(APIOut->ucExpiryDate,DongleOut->ucExpiryDate,sizeof(APIOut->ucExpiryDate));//Expiry Date
	APIOut->ucAreaCode = DongleOut->ucAreaCode;//Area Code
	memcpy(APIOut->ucTMID,DongleIn->ucTMID,sizeof(APIOut->ucTMID));//TM ID
	memcpy(APIOut->ucTMTxnDateTime,DongleIn->ucTMTxnDateTime,sizeof(APIOut->ucTMTxnDateTime));//TM Txn Date Time
	memcpy(APIOut->ucTMSerialNumber,DongleOut->ucTMSerialNumber,sizeof(APIOut->ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->ucTMAgentNumber,DongleIn->ucTMAgentNumber,sizeof(APIOut->ucTMAgentNumber));//TM Agent Number
	if(DongleOut->ucPurseVersionNumber == MIFARE)
	{
		memcpy(APIOut->ucSTAC,DongleOut->ucSID_STAC,sizeof(APIOut->ucSTAC));//STAC
		APIOut->ucKeyVersion = DongleOut->ucCPDKVN_SAMKVN;//SAM KVN
		memcpy(APIOut->ucSAMID,&DongleOut->ucCPD_SAMID[2],sizeof(APIOut->ucSAMID));//SAM ID
		memcpy(APIOut->ucSAMSN,&DongleOut->ucCPD_SAMID[10],sizeof(APIOut->ucSAMSN));//SAM SN
		memcpy(APIOut->ucSAMCRN,DongleOut->ucCPDRAN_SAMCRN,sizeof(APIOut->ucSAMCRN));//SAM CRN
	}
	else
	{
		memset(APIOut->ucSTAC,0x00,sizeof(APIOut->ucSTAC));//STAC
		APIOut->ucKeyVersion = 0;//SAM KVN
		memset(APIOut->ucSAMID,0x00,sizeof(APIOut->ucSAMID));//SAM ID
		memset(APIOut->ucSAMSN,0x00,sizeof(APIOut->ucSAMSN));//SAM SN
		memset(APIOut->ucSAMCRN,0x00,sizeof(APIOut->ucSAMCRN));//SAM CRN
	}
	memcpy(APIOut->ucReaderFWVersion,DongleOut->ucReaderFWVersion,sizeof(APIOut->ucReaderFWVersion));//Reader FW Version
	memcpy(APIOut->ucCardID,DongleOut->ucCardID,sizeof(APIOut->ucCardID));//Card ID
	memcpy(APIOut->ucTxnAmt,DongleOut->ucTxnAmt,sizeof(APIOut->ucTxnAmt));//Txn Amt
	memcpy(APIOut->ucCPUDeviceID,DongleOut->ucCPUDeviceID,sizeof(APIOut->ucCPUDeviceID));//CPU Device ID
	memcpy(APIOut->ucCPUSPID,DongleOut->ucCPUSPID,sizeof(APIOut->ucCPUSPID));//CPU SP ID
	memcpy(APIOut->ucCPULocationID,DongleOut->ucCPULocationID,sizeof(APIOut->ucCPULocationID));//CPU Location ID
	memcpy(APIOut->ucPID,DongleOut->ucPID,sizeof(APIOut->ucPID));//CPU Purse ID
	memcpy(APIOut->ucCTC,DongleOut->ucCTC,sizeof(APIOut->ucCTC));//CPU CTC
	memcpy(APIOut->ucProfileExpiryDate,DongleOut->ucProfileExpiryDate,sizeof(APIOut->ucProfileExpiryDate));//Profile Expiry Date
	memcpy(APIOut->ucSubAreaCode,DongleOut->ucSubAreaCode,sizeof(APIOut->ucSubAreaCode));//Sub Area Code
	memcpy(APIOut->ucTxnSN,DongleOut->ucTxnSN,sizeof(APIOut->ucTxnSN));//Txn SN
	memcpy(APIOut->ucEV,DongleOut->ucEV,sizeof(APIOut->ucEV));//EV
	APIOut->ucCPUAdminKeyKVN = DongleOut->ucCPUAdminKeyKVN;//CPU Admin Key KVN
	APIOut->ucCreditKeyKVN = DongleOut->ucCreditKeyKVN;//Credit Key KVN
	APIOut->ucCPUIssuerKeyKVN = DongleOut->ucCPUIssuerKeyKVN;//CPU Issuer Key KVN
	APIOut->ucTxnMode = DongleOut->ucTxnMode;//Txn Mode
	APIOut->ucTxnQuqlifier = DongleOut->ucTxnQuqlifier;//Txn Quqlifier
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		memcpy(APIOut->ucCPUSAMID,&DongleOut->ucCPD_SAMID[2],sizeof(APIOut->ucCPUSAMID));//CPU SAM ID
	else
		memcpy(APIOut->ucCPUSAMID,DongleOut->ucSID_STAC,sizeof(APIOut->ucCPUSAMID));//CPU SAM ID
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		APIOut->ucHostAdminKVN = 0x00;//Host Admin KVN
	else
		APIOut->ucHostAdminKVN = DongleOut->ucBankCode_HostAdminKVN;//Host Admin KVN
	APIOut->ucPurseVersionNumber = DongleOut->ucPurseVersionNumber;//Purse Version Number
	memcpy(APIOut->ucTMLocationID,DongleIn->ucTMLocationID,sizeof(APIOut->ucTMLocationID));//TM Location ID
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		memset(APIOut->ucCACrypto,0x00,sizeof(APIOut->ucCACrypto));//CACrypto後8碼
	else
		memcpy(APIOut->ucCACrypto,DongleOut->ucSVCrypto,sizeof(APIOut->ucCACrypto));//CACrypto
	//memcpy(APIOut->ucCardOneDayQuota,DongleOut->ucCardOneDayQuota,sizeof(APIOut->ucCardOneDayQuota));//Card One Day Quota
	//memcpy(APIOut->ucCardOneDayQuotaDate,DongleOut->ucCardOneDayQuotaDate,sizeof(APIOut->ucCardOneDayQuotaDate));//Card One Day Quota Date
	memset(ucLoyaltyCounter,0x00,sizeof(ucLoyaltyCounter));
	vdBuildCardAVRData(&APIOut->stCardAVRInfo_t,&DongleOut->stLastCreditTxnLogInfo_t,DongleOut->ucCardID,DongleOut->ucIssuerCode,DongleOut->ucCPUSPID,DongleOut->ucCPULocationID,DongleOut->ucPID,ucLoyaltyCounter);//Card AVR Info
	APIOut->ucCardIDLen = DongleOut->ucCardIDLen;//Card ID Len

//#ifdef READER_MODE
	memset(bBuf,0x00,sizeof(bBuf));
	vdStoreDeviceID(DongleOut->ucDeviceID,DongleOut->ucDeviceID[2],DongleOut->ucLocationID,DongleOut->ucCPUDeviceID,&DongleOut->ucCPUDeviceID[3],DongleOut->ucCPULocationID,gTmpData.ucReaderID,DongleOut->ucReaderFWVersion,DongleOut->ucCPUSAMID);
//#endif

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
					 (DongleOut->ucPurseVersionNumber == 0x00)?DongleOut->ucBankCode_HostAdminKVN:0x00,
					 DongleOut->ucAreaCode,
					 DongleOut->ucSubAreaCode,
					 DongleOut->ucDeposit,
					 DongleOut->ucTxnSN);

	return(inTMOutLen);
}



int inBuildAuthTxnOfflineData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen = 0;
AuthTxnOffline_TM_Out *APIOut = (AuthTxnOffline_TM_Out *)API_Out;
//AuthTxnOffline_APDU_In *DongleIn = (AuthTxnOffline_APDU_In *)Dongle_In;
AuthTxnOffline_APDU_Out *DongleOut = (AuthTxnOffline_APDU_Out *)Dongle_Out;
TxnReqOffline_APDU_Out *TxnReqOut = (TxnReqOffline_APDU_Out *)Dongle_ReqOut;

	inTMOutLen = sizeof(AuthTxnOffline_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	vdUIntToAsc(TxnReqOut->ucCardID,sizeof(TxnReqOut->ucCardID),APIOut->stSTMC_t.anCardID,sizeof(APIOut->stSTMC_t.anCardID),FALSE,' ',10);//n_Card Physical ID
	fnBINTODEVASC(TxnReqOut->ucDeviceID,APIOut->stSTMC_t.anDeviceID,sizeof(APIOut->stSTMC_t.anDeviceID),MIFARE);//n_TXN Device ID
	fnBINTODEVASC(TxnReqOut->ucCPUDeviceID,APIOut->stSTMC_t.anCPUDeviceID,sizeof(APIOut->stSTMC_t.anCPUDeviceID),LEVEL2);//n_TXN CPU Device ID
	vdCountEVBefore(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.anEVBeforeTxn,sizeof(APIOut->stSTMC_t.anEVBeforeTxn),TxnReqOut->ucMsgType);//n_EV Before TXN
	vdUIntToAsc(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),APIOut->stSTMC_t.anTxnSN,sizeof(APIOut->stSTMC_t.anTxnSN),FALSE,' ',10);//n_TXN SNum
	vdUIntToAsc(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),APIOut->stSTMC_t.anTxnAmt,sizeof(APIOut->stSTMC_t.anTxnAmt),FALSE,' ',10);//n_TXN AMT
	vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.anEV,sizeof(APIOut->stSTMC_t.anEV),FALSE,' ',10);//n_EV
	vdBuildProcessingCode(inTxnType,TxnReqOut->ucMsgType,TxnReqOut->ucSubType,TxnReqOut->ucPersonalProfile,APIOut->stSTMC_t.anProcessignCode);//Processing Code
	fnUnPack(&TxnReqOut->ucMsgType,sizeof(TxnReqOut->ucMsgType),APIOut->stSTMC_t.unMsgType);//Msg Type
	fnUnPack(&TxnReqOut->ucSubType,sizeof(TxnReqOut->ucSubType),APIOut->stSTMC_t.unSubType);//Sub Type
	fnUnPack(TxnReqOut->ucDeviceID,sizeof(TxnReqOut->ucDeviceID),APIOut->stSTMC_t.unDeviceID);//Device ID
	fnUnPack(&TxnReqOut->ucSPID,sizeof(TxnReqOut->ucSPID),APIOut->stSTMC_t.unSPID);//SP ID
	fnUnPack(DongleOut->ucTxnDateTime,sizeof(DongleOut->ucTxnDateTime),APIOut->stSTMC_t.unTxnDateTime);//Txn Date Time
	fnUnPack(&TxnReqOut->ucCardType,sizeof(TxnReqOut->ucCardType),APIOut->stSTMC_t.unCardType);//Card Type
	fnUnPack(&TxnReqOut->ucPersonalProfile,sizeof(TxnReqOut->ucPersonalProfile),APIOut->stSTMC_t.unPersonalProfile);//Personal Profile
	fnUnPack(&TxnReqOut->ucLocationID,sizeof(TxnReqOut->ucLocationID),APIOut->stSTMC_t.unLocationID);//Location ID
	fnUnPack(TxnReqOut->ucCardID,sizeof(TxnReqOut->ucCardID),APIOut->stSTMC_t.unCardID);//CPU Card ID
	fnUnPack(&TxnReqOut->ucIssuerCode,sizeof(TxnReqOut->ucIssuerCode),APIOut->stSTMC_t.unIssuerCode);//Issuer Code
	if(TxnReqOut->ucPurseVersionNumber == MIFARE)
	{
		fnUnPack(&TxnReqOut->ucBankCode_HostAdminKVN,1,APIOut->stSTMC_t.unBankCode);//Bank Code
		fnUnPack(TxnReqOut->ucCPD_SAMID,2,APIOut->stSTMC_t.unLoyaltyCounter);//Loyalty Counter

		if (TxnReqOut->ucReaderFWVersion[5] >= 0x80) //match sam的mfrc改用device id
			memcpy(&DongleOut->ucMAC_HCrypto[6], TxnReqOut->ucDeviceID, 4);	
		fnUnPack(DongleOut->ucMAC_HCrypto,10,APIOut->stSTMC_t.unMAC);//MAC
		// 配對SAM用Device id
		//if (TxnReqOut->ucReaderFWVersion[5] >= 0x80)
		//	fnUnPack(TxnReqOut->ucDeviceID, sizeof(TxnReqOut->ucDeviceID), &APIOut->stSTMC_t.unMAC[12]);
	}
	else
	{
		memset(APIOut->stSTMC_t.unBankCode,0x30,sizeof(APIOut->stSTMC_t.unBankCode));//Bank Counter
		memset(APIOut->stSTMC_t.unLoyaltyCounter,0x30,sizeof(APIOut->stSTMC_t.unLoyaltyCounter));//Loyalty Counter
		memset(APIOut->stSTMC_t.unMAC,0x30,sizeof(APIOut->stSTMC_t.unMAC));//MAC
	}
	fnUnPack(TxnReqOut->ucExpiryDate,sizeof(TxnReqOut->ucExpiryDate),APIOut->stSTMC_t.unOrgExpiryDate);//Org Expiry Date
	//memset(APIOut->stSTMC_t.unOrgExpiryDate,0x30,sizeof(APIOut->stSTMC_t.unOrgExpiryDate));//Org Expiry Date
	memset(APIOut->stSTMC_t.unNewExpiryDate,0x30,sizeof(APIOut->stSTMC_t.unNewExpiryDate));//New Expiry Date
	fnUnPack(&TxnReqOut->ucAreaCode,sizeof(TxnReqOut->ucAreaCode),APIOut->stSTMC_t.unAreaCode);//Area Code
	fnUnPack(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),APIOut->stSTMC_t.unTxnAmt);//Txn Amt
	fnUnPack(DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.unEV);//EV
	fnUnPack(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),APIOut->stSTMC_t.unTxnSN);//Txn SN
	fnUnPack(TxnReqOut->ucCPUDeviceID,sizeof(TxnReqOut->ucCPUDeviceID),APIOut->stSTMC_t.unCPUDeviceID);//CPU Device ID
	fnUnPack(TxnReqOut->ucCPUSPID,sizeof(TxnReqOut->ucCPUSPID),APIOut->stSTMC_t.unCPUSPID);//CPU SP ID
	fnUnPack(TxnReqOut->ucCPULocationID,sizeof(TxnReqOut->ucCPULocationID),APIOut->stSTMC_t.unCPULocationID);//CPU Location ID
	fnUnPack(TxnReqOut->ucPID,sizeof(TxnReqOut->ucPID),APIOut->stSTMC_t.unPID);//Purse ID
	fnUnPack(TxnReqOut->ucCTC,sizeof(TxnReqOut->ucCTC),APIOut->stSTMC_t.unCTC);//CTC
	fnUnPack(TxnReqOut->ucProfileExpiryDate,sizeof(TxnReqOut->ucProfileExpiryDate),APIOut->stSTMC_t.unOrgProfileExpiryDate);//Org Profile Expiry Date
	//memset(APIOut->stSTMC_t.unOrgProfileExpiryDate,0x30,sizeof(APIOut->stSTMC_t.unOrgProfileExpiryDate));//Org Profile Expiry Date
	memset(APIOut->stSTMC_t.unNewProfileExpiryDate,0x30,sizeof(APIOut->stSTMC_t.unNewProfileExpiryDate));//New Profile Expiry Date
	fnUnPack(TxnReqOut->ucSubAreaCode,sizeof(TxnReqOut->ucSubAreaCode),APIOut->stSTMC_t.unSubAreaCode);//Sub Area Code
	fnUnPack(TxnReqOut->ucTxnSN,sizeof(TxnReqOut->ucTxnSN),APIOut->stSTMC_t.unTxnSNBeforeTxn);//Txn SN Before Txn
	fnUnPack(TxnReqOut->ucEV,sizeof(TxnReqOut->ucEV),APIOut->stSTMC_t.unEVBeforeTxn);//EV Before Txn
	fnUnPack(TxnReqOut->ucDeposit,sizeof(TxnReqOut->ucDeposit),APIOut->stSTMC_t.unDeposit);//Deposit
	fnUnPack(&TxnReqOut->ucTxnMode,sizeof(TxnReqOut->ucTxnMode),APIOut->stSTMC_t.unTxnMode);//Txn Mode
	fnUnPack(&TxnReqOut->ucTxnQuqlifier,sizeof(TxnReqOut->ucTxnQuqlifier),APIOut->stSTMC_t.unTxnQuqlifier);//Txn Quqlifier
	fnUnPack(&TxnReqOut->ucSignatureKeyKVN,sizeof(TxnReqOut->ucSignatureKeyKVN),APIOut->stSTMC_t.unSignatureKeyKVN);//Signature Key KVN
	fnUnPack(DongleOut->ucSignature,sizeof(DongleOut->ucSignature),APIOut->stSTMC_t.unSignature);//Signature
	fnUnPack(DongleOut->ucCPUSAMID,sizeof(DongleOut->ucCPUSAMID),APIOut->stSTMC_t.unCPUSAMID);//SAM ID
	if(TxnReqOut->ucPurseVersionNumber != MIFARE)
	{
		fnUnPack(&DongleOut->ucMAC_HCrypto[0],1,APIOut->stSTMC_t.unHashType);//Hash Type
		fnUnPack(&DongleOut->ucMAC_HCrypto[1],1,APIOut->stSTMC_t.unHostAdminKVN);//Host Admin KVN
		fnUnPack(&DongleOut->ucMAC_HCrypto[2],16,APIOut->stSTMC_t.unCPUMAC);//MAC
	}
	else
	{
		memset(APIOut->stSTMC_t.unHashType,0x30,sizeof(APIOut->stSTMC_t.unHashType));
		memset(APIOut->stSTMC_t.unHostAdminKVN,0x30,sizeof(APIOut->stSTMC_t.unHostAdminKVN));
		memset(APIOut->stSTMC_t.unCPUMAC,0x30,sizeof(APIOut->stSTMC_t.unCPUMAC));//MAC
	}
	fnUnPack(&TxnReqOut->ucPurseVersionNumber,sizeof(TxnReqOut->ucPurseVersionNumber),APIOut->stSTMC_t.unPurseVersionNumber);//Purse Version Number
	fnUnPack(&TxnReqOut->ucCardIDLen,sizeof(TxnReqOut->ucCardIDLen),APIOut->stSTMC_t.unCardIDLen);//Card ID Len
	memcpy(APIOut->stSTMC_t.ucTMLocationID,stStore_DataInfo_t.ucTMLocationID,sizeof(stStore_DataInfo_t.ucTMLocationID));//TM Location ID
	memcpy(APIOut->stSTMC_t.ucTMID,stStore_DataInfo_t.ucTMID,sizeof(stStore_DataInfo_t.ucTMID));//TM ID
	UnixToDateTime((BYTE*)DongleOut->ucTxnDateTime,(BYTE*)APIOut->stSTMC_t.ucTMTxnDateTime,14);//TM Txn Date Time
	memcpy(APIOut->stSTMC_t.ucTMSerialNumber,stStore_DataInfo_t.ucTMSerialNumber,sizeof(stStore_DataInfo_t.ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->stSTMC_t.ucTMAgentNumber,stStore_DataInfo_t.ucTMAgentNumber,sizeof(stStore_DataInfo_t.ucTMAgentNumber));//TM Agent Number
	if(srTxnData.srIngData.lnECCAmt1 + srTxnData.srIngData.lnECCAmt2 + srTxnData.srIngData.lnECCAmt3 + srTxnData.srIngData.lnECCAmt4 + srTxnData.srIngData.lnECCAmt5== 0L)
	{
		memset(APIOut->unSTMCAmt1,0x00,sizeof(APIOut->unSTMCAmt1));
		memset(APIOut->unSTMCAmt2,0x00,sizeof(APIOut->unSTMCAmt2));
		memset(APIOut->unSTMCAmt3,0x00,sizeof(APIOut->unSTMCAmt3));
		memset(APIOut->unSTMCAmt4,0x00,sizeof(APIOut->unSTMCAmt4));
		memset(APIOut->unSTMCAmt5,0x00,sizeof(APIOut->unSTMCAmt5));
	}
	else
	{
		fnUnPack((BYTE *)&srTxnData.srIngData.lnECCAmt1,2,APIOut->unSTMCAmt1);//TxnAmt1
		fnUnPack((BYTE *)&srTxnData.srIngData.lnECCAmt2,2,APIOut->unSTMCAmt2);//TxnAmt2
		fnUnPack((BYTE *)&srTxnData.srIngData.lnECCAmt3,2,APIOut->unSTMCAmt3);//TxnAmt3
		fnUnPack((BYTE *)&srTxnData.srIngData.lnECCAmt4,2,APIOut->unSTMCAmt4);//TxnAmt4
		fnUnPack((BYTE *)&srTxnData.srIngData.lnECCAmt5,2,APIOut->unSTMCAmt5);//TxnAmt5
	}
	//帳務資料結束,加值重送資料開始
	memcpy(APIOut->unVARMsgType,"0C",2);//VAR Msg Type
	fnUnPack(&TxnReqOut->stLastCreditTxnLogInfo_t.ucPurseVersionNumber_Index,sizeof(TxnReqOut->stLastCreditTxnLogInfo_t.ucPurseVersionNumber_Index),APIOut->unVARPurseVersionNumber);//VAR PurseVersion Number
	fnUnPack(TxnReqOut->stLastCreditTxnLogInfo_t.ucTxnSN,sizeof(TxnReqOut->stLastCreditTxnLogInfo_t.ucTxnSN),APIOut->unVARTxnSN);//VAR Txn SN
	fnUnPack(TxnReqOut->stLastCreditTxnLogInfo_t.ucTxnDateTime,sizeof(TxnReqOut->stLastCreditTxnLogInfo_t.ucTxnDateTime),APIOut->unVARTxnDateTime);//VAR Txn Date Time
	fnUnPack(&TxnReqOut->stLastCreditTxnLogInfo_t.ucSubType,sizeof(TxnReqOut->stLastCreditTxnLogInfo_t.ucSubType),APIOut->unVARSubType);//VAR Sub Type
	fnUnPack(TxnReqOut->stLastCreditTxnLogInfo_t.ucTxnAmt,sizeof(TxnReqOut->stLastCreditTxnLogInfo_t.ucTxnAmt),APIOut->unVARTxnAmt);//VAR Txn Amt
	fnUnPack(TxnReqOut->stLastCreditTxnLogInfo_t.ucEV,sizeof(TxnReqOut->stLastCreditTxnLogInfo_t.ucEV),APIOut->unVAREV);//VAR EV
	fnUnPack(TxnReqOut->ucCardID,sizeof(TxnReqOut->ucCardID),APIOut->unVARCardID);//VAR Card ID
	fnUnPack(&TxnReqOut->ucIssuerCode,sizeof(TxnReqOut->ucIssuerCode),APIOut->unVARIssuerCode);//VAR Issuer Code
	fnUnPack(TxnReqOut->stLastCreditTxnLogInfo_t.ucCPUSPID,sizeof(TxnReqOut->stLastCreditTxnLogInfo_t.ucCPUSPID),APIOut->unVARCPUSPID);//VAR CPU SP ID
	fnUnPack(TxnReqOut->stLastCreditTxnLogInfo_t.ucCPULocationID,sizeof(TxnReqOut->stLastCreditTxnLogInfo_t.ucCPULocationID),APIOut->unVARCPULocationID);//VAR CPU Location ID
	fnUnPack(TxnReqOut->ucPID,sizeof(TxnReqOut->ucPID),APIOut->unVARPID);//VAR Purse ID
	fnUnPack(TxnReqOut->stLastCreditTxnLogInfo_t.ucCPUDeviceID,sizeof(TxnReqOut->stLastCreditTxnLogInfo_t.ucCPUDeviceID),APIOut->unVARCPUDeviceID);//VAR CPU Device ID
	if(TxnReqOut->ucPurseVersionNumber == MIFARE)
		fnUnPack(TxnReqOut->ucCPD_SAMID,2,APIOut->unVARLoyaltyCounter);//VAR Loyalty Counter
	else
		memset(APIOut->unVARLoyaltyCounter,0x30,sizeof(APIOut->unVARLoyaltyCounter));//VAR Loyalty Counter
	fnUnPack(&TxnReqOut->ucPersonalProfile,sizeof(TxnReqOut->ucPersonalProfile),APIOut->unVARPersonalProfile);//VAR Personal Profile
	if(TxnReqOut->ucPurseVersionNumber == MIFARE)
		fnUnPack(&TxnReqOut->ucBankCode_HostAdminKVN,1,APIOut->unVARBankCode);//VAR Bank Code
	else
		memset(APIOut->unVARBankCode,0x30,sizeof(APIOut->unVARBankCode));//VAR Bank Counter
	fnUnPack(&TxnReqOut->ucCardIDLen,sizeof(TxnReqOut->ucCardIDLen),APIOut->unVARCardIDLen);//VAR Card ID Len
	//以上為加值當下資料
	//以下為重送當下資料
	fnUnPack(TxnReqOut->ucDeviceID,sizeof(TxnReqOut->ucDeviceID),APIOut->unVAR2DeviceID);//VAR Device ID
	fnUnPack(&TxnReqOut->ucSPID,sizeof(TxnReqOut->ucSPID),APIOut->unVAR2SPID);//VAR SP ID
	fnUnPack(&TxnReqOut->ucLocationID,sizeof(TxnReqOut->ucLocationID),APIOut->unVAR2LocationID);//VAR SP ID
	fnUnPack(TxnReqOut->ucCPUDeviceID,sizeof(TxnReqOut->ucCPUDeviceID),APIOut->unVAR2CPUDeviceID);//VAR CPU Device ID
	fnUnPack(TxnReqOut->ucCPUSPID,sizeof(TxnReqOut->ucCPUSPID),APIOut->unVAR2CPUSPID);//VAR CPU SP ID
	fnUnPack(TxnReqOut->ucCPULocationID,sizeof(TxnReqOut->ucCPULocationID),APIOut->unVAR2CPULocationID);//VAR CPU Location ID

	return(inTMOutLen);
}

int inBuildLockCardData1(int inTxnType,STRUCT_XML_DOC *srXML,LockCard_TM_Out_2 *APIOut,LockCard_APDU_Out_2 *DongleOut,BYTE *ucTxnDateTime,BYTE ucLockReason,BYTE *ucTMLocationID,BYTE *ucTMID)
{
int inTMOutLen;
BYTE bBLCName[50];

	inTMOutLen = sizeof(LockCard_TM_Out_2);
	memset(APIOut,0x00,inTMOutLen);
	fnUnPack(&DongleOut->ucPurseVersionNumber,sizeof(DongleOut->ucPurseVersionNumber),APIOut->unPurseVersionNumber);//Purse Version Number
	sprintf((char *)APIOut->unMsgType,"22");//Msg Type
	sprintf((char *)APIOut->unSubType,"00");//Sub Type
	fnUnPack(DongleOut->ucDeviceID,sizeof(DongleOut->ucDeviceID),APIOut->unDeviceID);//Device ID
	fnUnPack(&DongleOut->ucSPID,sizeof(DongleOut->ucSPID),APIOut->unSPID);//SP ID
	fnUnPack(ucTxnDateTime,sizeof(APIOut->unTxnDateTime) / 2,APIOut->unTxnDateTime);//Txn Date Time
	fnUnPack(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),APIOut->unCardID);//Card ID
	fnUnPack(&DongleOut->ucIssuerCode,sizeof(DongleOut->ucIssuerCode),APIOut->unIssuerCode);//Issuer Code
	fnUnPack(&DongleOut->ucLocationID,sizeof(DongleOut->ucLocationID),APIOut->unLocationID);//Location ID
	fnUnPack(&DongleOut->ucCardIDLen,sizeof(DongleOut->ucCardIDLen),APIOut->unCardIDLen);//Card ID Len
	fnUnPack(DongleOut->ucCPUDeviceID,sizeof(DongleOut->ucCPUDeviceID),APIOut->unCPUDeviceID);//CPU Device ID
	fnUnPack(DongleOut->ucCPUSPID,sizeof(DongleOut->ucCPUSPID),APIOut->unCPUSPID);//CPU SP ID
	fnUnPack(DongleOut->ucCPULocationID,sizeof(DongleOut->ucCPULocationID),APIOut->unCPULocationID);//CPU Location ID
	fnUnPack(&DongleOut->ucCardType,sizeof(DongleOut->ucCardType),APIOut->unCardType);//Card Type
	fnUnPack(&DongleOut->ucPersonalProfile,sizeof(DongleOut->ucPersonalProfile),APIOut->unPersonalProfile);//Personal Profile
	fnUnPack(DongleOut->ucPID,sizeof(DongleOut->ucPID),APIOut->unPID);//CPU Purse ID
	fnUnPack(DongleOut->ucCTC,sizeof(DongleOut->ucCTC),APIOut->unCTC);//CTC
	fnUnPack(&ucLockReason,sizeof(ucLockReason),APIOut->unLockReason);//Lock Reason
	if(ucLockReason == 0x01)
	{
		BYTE gsBlackListVersion[30];

		memset(gsBlackListVersion,0x00,sizeof(gsBlackListVersion));
		inGetBLCName(gsBlackListVersion);
		memset(bBLCName,0x00,sizeof(bBLCName));
		sprintf((char *)bBLCName,"BLC%s.BIG",gsBlackListVersion);
		vdPad((char *)bBLCName,(char *)APIOut->unBLCFileName,sizeof(APIOut->unBLCFileName),TRUE,0x20);//BLC File Name
		//vdPad(gsBlackListVersion,APIOut->unBLCFileName,sizeof(APIOut->unBLCFileName),TRUE,0x20);//BLC File Name
		APIOut->unBLCIDFlag = gsBlackListVersion[17];//BLC ID Flag
		if(APIOut->unBLCIDFlag == 0x00)
			APIOut->unBLCIDFlag = 0x20;
	}
	else
	{
		memset(APIOut->unBLCFileName,0x20,sizeof(APIOut->unBLCFileName));//BLC File Name
		APIOut->unBLCIDFlag = 0x20;//BLC ID Flag
	}
	memcpy(APIOut->unTMLocationID,ucTMLocationID,sizeof(APIOut->unTMLocationID));//TM Location ID
	memcpy(APIOut->unTMID,ucTMID,sizeof(APIOut->unTMID));//TM ID
	memset(APIOut->unRFU,0x20,sizeof(APIOut->unRFU));//RFU
	memset(APIOut->unEV,0x30,sizeof(APIOut->unEV));//EV
	memset(APIOut->unCPUEV,0x30,sizeof(APIOut->unCPUEV));//CPU EV
	memset(APIOut->unMSettingPar,0x30,sizeof(APIOut->unMSettingPar));//M SettingPar
	memset(APIOut->unCSettingPar,0x30,sizeof(APIOut->unCSettingPar));//C SettingPar
	memset(APIOut->unTxnSN,0x30,sizeof(APIOut->unTxnSN));//Txn SN
	memset(APIOut->unVARTxnSN,0x30,sizeof(APIOut->unVARTxnSN));//VAR Txn SN
	memset(APIOut->unVARTxnDateTime,0x30,sizeof(APIOut->unVARTxnDateTime));//VAR Txn Date Time
	memset(APIOut->unVARSubType,0x30,sizeof(APIOut->unVARSubType));//VAR Sub Type
	memset(APIOut->unVARTxnAmt,0x30,sizeof(APIOut->unVARTxnAmt));//VAR Txn Amt
	memset(APIOut->unVAREV,0x30,sizeof(APIOut->unVAREV));//VAR EV
	memset(APIOut->unVARDeviceID,0x30,sizeof(APIOut->unVARDeviceID));//VAR Device ID

	return(inTMOutLen);
}

int inBuildLockCardData2(int inTxnType,STRUCT_XML_DOC *srXML,LockCard_TM_Out_2 *APIOut,LockCard_APDU_Out_3 *DongleOut,BYTE *ucTxnDateTime,BYTE ucLockReason,BYTE *ucTMLocationID,BYTE *ucTMID)
{
int inTMOutLen;
BYTE bSpace[20];

	inTMOutLen = sizeof(LockCard_TM_Out_2);
	memset(APIOut,0x00,inTMOutLen);
	memset(bSpace,0x20,sizeof(bSpace));
	fnUnPack(&DongleOut->ucPurseVersionNumber,sizeof(DongleOut->ucPurseVersionNumber),APIOut->unPurseVersionNumber);//Purse Version Number
	sprintf((char *)APIOut->unMsgType,"22");//Msg Type
	sprintf((char *)APIOut->unSubType,"00");//Sub Type
	fnUnPack(DongleOut->ucDeviceID,sizeof(DongleOut->ucDeviceID),APIOut->unDeviceID);//Device ID
	fnUnPack(&DongleOut->ucSPID,sizeof(DongleOut->ucSPID),APIOut->unSPID);//SP ID
	fnUnPack(ucTxnDateTime,sizeof(APIOut->unTxnDateTime) / 2,APIOut->unTxnDateTime);//Txn Date Time
	fnUnPack(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),APIOut->unCardID);//Card ID
	fnUnPack(&DongleOut->ucIssuerCode,sizeof(DongleOut->ucIssuerCode),APIOut->unIssuerCode);//Issuer Code
	fnUnPack(&DongleOut->ucLocationID,sizeof(DongleOut->ucLocationID),APIOut->unLocationID);//Location ID
	fnUnPack(&DongleOut->ucCardIDLen,sizeof(DongleOut->ucCardIDLen),APIOut->unCardIDLen);//Card ID Len
	fnUnPack(DongleOut->ucCPUDeviceID,sizeof(DongleOut->ucCPUDeviceID),APIOut->unCPUDeviceID);//CPU Device ID
	fnUnPack(DongleOut->ucCPUSPID,sizeof(DongleOut->ucCPUSPID),APIOut->unCPUSPID);//CPU SP ID
	fnUnPack(DongleOut->ucCPULocationID,sizeof(DongleOut->ucCPULocationID),APIOut->unCPULocationID);//CPU Location ID
	fnUnPack(&DongleOut->ucCardType,sizeof(DongleOut->ucCardType),APIOut->unCardType);//Card Type
	fnUnPack(&DongleOut->ucPersonalProfile,sizeof(DongleOut->ucPersonalProfile),APIOut->unPersonalProfile);//Personal Profile
	fnUnPack(DongleOut->ucPID,sizeof(DongleOut->ucPID),APIOut->unPID);//CPU Purse ID
	memset(APIOut->unCTC,0x30,sizeof(APIOut->unCTC));//CTC
	//fnUnPack(DongleOut->ucCTC,sizeof(DongleOut->ucCTC),APIOut->unCTC);//CTC
	fnUnPack(&ucLockReason,sizeof(ucLockReason),APIOut->unLockReason);//Lock Reason
	memset(APIOut->unBLCFileName,0x20,sizeof(APIOut->unBLCFileName));//BLC File Name
	APIOut->unBLCIDFlag = 0x20;//BLC ID Flag
	memcpy(APIOut->unTMLocationID,ucTMLocationID,sizeof(APIOut->unTMLocationID));//TM Location ID
	memcpy(APIOut->unTMID,ucTMID,sizeof(APIOut->unTMID));//TM ID
	memset(APIOut->unRFU,0x20,sizeof(APIOut->unRFU));//RFU

	fnUnPack(DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->unEV);//EV
	fnUnPack(DongleOut->ucEV2,sizeof(DongleOut->ucEV2),APIOut->unCPUEV);//EV2
	fnUnPack(&DongleOut->ucMifareSettingParameter,sizeof(DongleOut->ucMifareSettingParameter),APIOut->unMSettingPar);//Mifare Setting Parameter
	fnUnPack(&DongleOut->ucCPUSettingParameter,sizeof(DongleOut->ucCPUSettingParameter),APIOut->unCSettingPar);//CPU Setting Parameter
	fnUnPack(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),APIOut->unTxnSN);//Txn SN
	fnUnPack(DongleOut->stLastCreditTxnLogInfo_t.ucTxnSN,sizeof(DongleOut->stLastCreditTxnLogInfo_t.ucTxnSN),APIOut->unVARTxnSN);//VAR Txn SN
	fnUnPack(DongleOut->stLastCreditTxnLogInfo_t.ucTxnDateTime,sizeof(DongleOut->stLastCreditTxnLogInfo_t.ucTxnDateTime),APIOut->unVARTxnDateTime);//VAR Txn Date Time
	fnUnPack(&DongleOut->stLastCreditTxnLogInfo_t.ucSubType,sizeof(DongleOut->stLastCreditTxnLogInfo_t.ucSubType),APIOut->unVARSubType);//VAR Sub Type
	fnUnPack(DongleOut->stLastCreditTxnLogInfo_t.ucTxnAmt,sizeof(DongleOut->stLastCreditTxnLogInfo_t.ucTxnAmt),APIOut->unVARTxnAmt);//VAR Txn Amt
	fnUnPack(DongleOut->stLastCreditTxnLogInfo_t.ucEV,sizeof(DongleOut->stLastCreditTxnLogInfo_t.ucEV),APIOut->unVAREV);//VAR EV
	fnUnPack(DongleOut->stLastCreditTxnLogInfo_t.ucCPUDeviceID,sizeof(DongleOut->stLastCreditTxnLogInfo_t.ucCPUDeviceID),APIOut->unVARDeviceID);//VAR Device ID

	return(inTMOutLen);
}

int inBuildLockCardData3(int inTxnType,STRUCT_XML_DOC *srXML,LockCard_TM_Out_2 *APIOut,TxnReqOffline_APDU_Out *DongleOut,BYTE *ucTxnDateTime,BYTE ucLockReason,BYTE *ucTMLocationID,BYTE *ucTMID)
{
#if 0
int inTMOutLen;
BYTE bSpace[20];

	inTMOutLen = sizeof(LockCard_TM_Out_2);
	memset(APIOut,0x00,inTMOutLen);
	memset(bSpace,0x20,sizeof(bSpace));
	fnUnPack(&DongleOut->ucPurseVersionNumber,sizeof(DongleOut->ucPurseVersionNumber),APIOut->unPurseVersionNumber);//Purse Version Number
	sprintf((char *)APIOut->unMsgType,"22");//Msg Type
	sprintf((char *)APIOut->unSubType,"00");//Sub Type
	fnUnPack(DongleOut->ucDeviceID,sizeof(DongleOut->ucDeviceID),APIOut->unDeviceID);//Device ID
	fnUnPack(&DongleOut->ucSPID,sizeof(DongleOut->ucSPID),APIOut->unSPID);//SP ID
	fnUnPack(ucTxnDateTime,sizeof(APIOut->unTxnDateTime) / 2,APIOut->unTxnDateTime);//Txn Date Time
	fnUnPack(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),APIOut->unCardID);//Card ID
	fnUnPack(&DongleOut->ucIssuerCode,sizeof(DongleOut->ucIssuerCode),APIOut->unIssuerCode);//Issuer Code
	fnUnPack(&DongleOut->ucLocationID,sizeof(DongleOut->ucLocationID),APIOut->unLocationID);//Location ID
	fnUnPack(&DongleOut->ucCardIDLen,sizeof(DongleOut->ucCardIDLen),APIOut->unCardIDLen);//Card ID Len
	fnUnPack(DongleOut->ucCPUDeviceID,sizeof(DongleOut->ucCPUDeviceID),APIOut->unCPUDeviceID);//CPU Device ID
	fnUnPack(DongleOut->ucCPUSPID,sizeof(DongleOut->ucCPUSPID),APIOut->unCPUSPID);//CPU SP ID
	fnUnPack(DongleOut->ucCPULocationID,sizeof(DongleOut->ucCPULocationID),APIOut->unCPULocationID);//CPU Location ID
	fnUnPack(&DongleOut->ucCardType,sizeof(DongleOut->ucCardType),APIOut->unCardType);//Card Type
	fnUnPack(&DongleOut->ucPersonalProfile,sizeof(DongleOut->ucPersonalProfile),APIOut->unPersonalProfile);//Personal Profile
	fnUnPack(DongleOut->ucPID,sizeof(DongleOut->ucPID),APIOut->unPID);//CPU Purse ID
	fnUnPack(DongleOut->ucCTC,sizeof(DongleOut->ucCTC),APIOut->unCTC);//CTC
	fnUnPack(&ucLockReason,sizeof(ucLockReason),APIOut->unLockReason);//Lock Reason
	memset(APIOut->unBLCFileName,0x20,sizeof(APIOut->unBLCFileName));//BLC File Name
	APIOut->unBLCIDFlag = 0x20;//BLC ID Flag
	memcpy(APIOut->unTMLocationID,ucTMLocationID,sizeof(APIOut->unTMLocationID));//TM Location ID
	memcpy(APIOut->unTMID,ucTMID,sizeof(APIOut->unTMID));//TM ID
	memset(APIOut->unRFU,0x20,sizeof(APIOut->unRFU));//RFU

	return(inTMOutLen);
#else
	return(0);
#endif
}

int inBuildLockCardData4(int inTxnType,STRUCT_XML_DOC *srXML,LockCard_TM_Out_1 *APIOut,TxRefund_APDU_Out *DongleOut,BYTE *ucTxnDateTime,BYTE ucLockReason)
{
int inTMOutLen;
BYTE bSpace[20];

	inTMOutLen = sizeof(LockCard_TM_Out_1);
	memset(APIOut,0x00,inTMOutLen);
	memset(bSpace,0x20,sizeof(bSpace));
	sprintf((char *)APIOut->unMsgType,"22");//Msg Type
	sprintf((char *)APIOut->unSubType,"00");//Sub Type
	fnUnPack(DongleOut->ucDeviceID,sizeof(DongleOut->ucDeviceID),APIOut->unDeviceID);//Device ID
	fnUnPack(&DongleOut->ucDeviceID[2],sizeof(APIOut->unSPID) / 2,APIOut->unSPID);//SP ID
	fnUnPack(ucTxnDateTime,sizeof(APIOut->unTxnDateTime) / 2,APIOut->unTxnDateTime);//Txn Date Time
	fnUnPack(&DongleOut->ucCardType,sizeof(DongleOut->ucCardType),APIOut->unCardType);//Card Type
	fnUnPack(&DongleOut->ucPersonalProfile,sizeof(DongleOut->ucPersonalProfile),APIOut->unPersonalProfile);//Personal Profile
	fnUnPack(DongleOut->ucCardID_4,sizeof(DongleOut->ucCardID_4),APIOut->unCardID_8);//Card ID
	fnUnPack(&DongleOut->ucIssuerCode,sizeof(DongleOut->ucIssuerCode),APIOut->unIssuerCode);//Issuer Code
	memcpy(APIOut->unLocationID,LOCATION_ID_BATCH,2);//Location ID
	fnUnPack(&ucLockReason,sizeof(ucLockReason),APIOut->unLockReason);//Lock Reason

	return(inTMOutLen);
}

int inBuildLockCardData5(int inTxnType,STRUCT_XML_DOC *srXML,LockCard_TM_Out_1 *APIOut,AddValue_APDU_Out *DongleOut,BYTE *ucTxnDateTime,BYTE ucLockReason)
{
int inTMOutLen;
BYTE bSpace[20];

	inTMOutLen = sizeof(LockCard_TM_Out_1);
	memset(APIOut,0x00,inTMOutLen);
	memset(bSpace,0x20,sizeof(bSpace));
	sprintf((char *)APIOut->unMsgType,"22");//Msg Type
	sprintf((char *)APIOut->unSubType,"00");//Sub Type
	fnUnPack(DongleOut->ucDeviceID,sizeof(DongleOut->ucDeviceID),APIOut->unDeviceID);//Device ID
	fnUnPack(&DongleOut->ucDeviceID[2],sizeof(APIOut->unSPID) / 2,APIOut->unSPID);//SP ID
	fnUnPack(ucTxnDateTime,sizeof(APIOut->unTxnDateTime) / 2,APIOut->unTxnDateTime);//Txn Date Time
	fnUnPack(&DongleOut->ucCardType,sizeof(DongleOut->ucCardType),APIOut->unCardType);//Card Type
	fnUnPack(&DongleOut->ucPersonalProfile,sizeof(DongleOut->ucPersonalProfile),APIOut->unPersonalProfile);//Personal Profile
	fnUnPack(DongleOut->ucCardID_4,sizeof(DongleOut->ucCardID_4),APIOut->unCardID_8);//Card ID
	fnUnPack(&DongleOut->ucIssuerCode,sizeof(DongleOut->ucIssuerCode),APIOut->unIssuerCode);//Issuer Code
	memcpy(APIOut->unLocationID,LOCATION_ID_BATCH,2);//Location ID
	fnUnPack(&ucLockReason,sizeof(ucLockReason),APIOut->unLockReason);//Lock Reason

	return(inTMOutLen);
}

int inBuildLockCardData6(int inTxnType,STRUCT_XML_DOC *srXML,LockCard_TM_Out_1 *APIOut,LockCard_APDU_Out_1 *DongleOut,BYTE *ucTxnDateTime,BYTE ucLockReason)
{
int inTMOutLen;
BYTE bSpace[20];

	inTMOutLen = sizeof(LockCard_TM_Out_1);
	memset(APIOut,0x00,inTMOutLen);
	memset(bSpace,0x20,sizeof(bSpace));
	sprintf((char *)APIOut->unMsgType,"22");//Msg Type
	sprintf((char *)APIOut->unSubType,"00");//Sub Type
	fnUnPack(DongleOut->ucDeviceID,sizeof(DongleOut->ucDeviceID),APIOut->unDeviceID);//Device ID
	fnUnPack(&DongleOut->ucDeviceID[2],sizeof(APIOut->unSPID) / 2,APIOut->unSPID);//SP ID
	fnUnPack(ucTxnDateTime,sizeof(APIOut->unTxnDateTime) / 2,APIOut->unTxnDateTime);//Txn Date Time
	fnUnPack(&DongleOut->ucCardType,sizeof(DongleOut->ucCardType),APIOut->unCardType);//Card Type
	fnUnPack(&DongleOut->ucPersonalProfile,sizeof(DongleOut->ucPersonalProfile),APIOut->unPersonalProfile);//Personal Profile
	fnUnPack(DongleOut->ucCardID_4,sizeof(DongleOut->ucCardID_4),APIOut->unCardID_8);//Card ID
	fnUnPack(&DongleOut->ucIssuerCode,sizeof(DongleOut->ucIssuerCode),APIOut->unIssuerCode);//Issuer Code
	memcpy(APIOut->unLocationID,LOCATION_ID_BATCH,2);//Location ID
	fnUnPack(&ucLockReason,sizeof(ucLockReason),APIOut->unLockReason);//Lock Reason

	return(inTMOutLen);
}

int inBuildLockCardData8(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
//int inBuildLockCardData9(int inTxnType,LockCard_TM_Out_2 *APIOut,LockCard_APDU_Out_3 *DongleOut,BYTE *ucTxnDateTime,BYTE ucLockReason,BYTE *ucTMLocationID,BYTE *ucTMID)
{
int inTMOutLen;
BYTE bSpace[20];
LockCard_TM_Out_2 *APIOut = (LockCard_TM_Out_2 *)API_Out;
LockCard_APDU_Out_3 *DongleOut = (LockCard_APDU_Out_3 *)Dongle_Out;
BYTE ucTxnDateTime[5],ucLockReason,*ucTMLocationID,*ucTMID,ucDateTime[20];

	ucTMLocationID = (BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID;
	ucTMID = (BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMID;
	ucLockReason = DongleOut->ucStatusCode[1];
	memset(ucTxnDateTime,0x00,sizeof(ucTxnDateTime));
	memset(ucDateTime,0x00,sizeof(ucDateTime));
	memcpy(&ucDateTime[0],srTxnData.srIngData.chTxDate,8);
	memcpy(&ucDateTime[8],srTxnData.srIngData.chTxTime,6);
	fngetUnixTimeCnt(ucTxnDateTime,ucDateTime); //交易日期時間(UnixDateTime)

	inTMOutLen = sizeof(LockCard_TM_Out_2);
	memset(APIOut,0x00,inTMOutLen);
	memset(bSpace,0x20,sizeof(bSpace));
	fnUnPack(&DongleOut->ucPurseVersionNumber,sizeof(DongleOut->ucPurseVersionNumber),APIOut->unPurseVersionNumber);//Purse Version Number
	sprintf((char *)APIOut->unMsgType,"22");//Msg Type
	sprintf((char *)APIOut->unSubType,"00");//Sub Type
	fnUnPack(DongleOut->ucDeviceID,sizeof(DongleOut->ucDeviceID),APIOut->unDeviceID);//Device ID
	fnUnPack(&DongleOut->ucSPID,sizeof(DongleOut->ucSPID),APIOut->unSPID);//SP ID
	fnUnPack(ucTxnDateTime,sizeof(APIOut->unTxnDateTime) / 2,APIOut->unTxnDateTime);//Txn Date Time
	fnUnPack(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),APIOut->unCardID);//Card ID
	fnUnPack(&DongleOut->ucIssuerCode,sizeof(DongleOut->ucIssuerCode),APIOut->unIssuerCode);//Issuer Code
	fnUnPack(&DongleOut->ucLocationID,sizeof(DongleOut->ucLocationID),APIOut->unLocationID);//Location ID
	fnUnPack(&DongleOut->ucCardIDLen,sizeof(DongleOut->ucCardIDLen),APIOut->unCardIDLen);//Card ID Len
	fnUnPack(DongleOut->ucCPUDeviceID,sizeof(DongleOut->ucCPUDeviceID),APIOut->unCPUDeviceID);//CPU Device ID
	fnUnPack(DongleOut->ucCPUSPID,sizeof(DongleOut->ucCPUSPID),APIOut->unCPUSPID);//CPU SP ID
	fnUnPack(DongleOut->ucCPULocationID,sizeof(DongleOut->ucCPULocationID),APIOut->unCPULocationID);//CPU Location ID
	fnUnPack(&DongleOut->ucCardType,sizeof(DongleOut->ucCardType),APIOut->unCardType);//Card Type
	fnUnPack(&DongleOut->ucPersonalProfile,sizeof(DongleOut->ucPersonalProfile),APIOut->unPersonalProfile);//Personal Profile
	fnUnPack(DongleOut->ucPID,sizeof(DongleOut->ucPID),APIOut->unPID);//CPU Purse ID
	memset(APIOut->unCTC,0x30,sizeof(APIOut->unCTC));//CTC
	//fnUnPack(DongleOut->ucCTC,sizeof(DongleOut->ucCTC),APIOut->unCTC);//CTC
	fnUnPack(&ucLockReason,sizeof(ucLockReason),APIOut->unLockReason);//Lock Reason
	memset(APIOut->unBLCFileName,0x20,sizeof(APIOut->unBLCFileName));//BLC File Name
	APIOut->unBLCIDFlag = 0x20;//BLC ID Flag
	memcpy(APIOut->unTMLocationID,ucTMLocationID,sizeof(APIOut->unTMLocationID));//TM Location ID
	memcpy(APIOut->unTMID,ucTMID,sizeof(APIOut->unTMID));//TM ID
	memset(APIOut->unRFU,0x20,sizeof(APIOut->unRFU));//RFU

	fnUnPack(DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->unEV);//EV
	fnUnPack(DongleOut->ucEV2,sizeof(DongleOut->ucEV2),APIOut->unCPUEV);//EV2
	fnUnPack(&DongleOut->ucMifareSettingParameter,sizeof(DongleOut->ucMifareSettingParameter),APIOut->unMSettingPar);//Mifare Setting Parameter
	fnUnPack(&DongleOut->ucCPUSettingParameter,sizeof(DongleOut->ucCPUSettingParameter),APIOut->unCSettingPar);//CPU Setting Parameter
	fnUnPack(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),APIOut->unTxnSN);//Txn SN
	fnUnPack(DongleOut->stLastCreditTxnLogInfo_t.ucTxnSN,sizeof(DongleOut->stLastCreditTxnLogInfo_t.ucTxnSN),APIOut->unVARTxnSN);//VAR Txn SN
	fnUnPack(DongleOut->stLastCreditTxnLogInfo_t.ucTxnDateTime,sizeof(DongleOut->stLastCreditTxnLogInfo_t.ucTxnDateTime),APIOut->unVARTxnDateTime);//VAR Txn Date Time
	fnUnPack(&DongleOut->stLastCreditTxnLogInfo_t.ucSubType,sizeof(DongleOut->stLastCreditTxnLogInfo_t.ucSubType),APIOut->unVARSubType);//VAR Sub Type
	fnUnPack(DongleOut->stLastCreditTxnLogInfo_t.ucTxnAmt,sizeof(DongleOut->stLastCreditTxnLogInfo_t.ucTxnAmt),APIOut->unVARTxnAmt);//VAR Txn Amt
	fnUnPack(DongleOut->stLastCreditTxnLogInfo_t.ucEV,sizeof(DongleOut->stLastCreditTxnLogInfo_t.ucEV),APIOut->unVAREV);//VAR EV
	fnUnPack(DongleOut->stLastCreditTxnLogInfo_t.ucCPUDeviceID,sizeof(DongleOut->stLastCreditTxnLogInfo_t.ucCPUDeviceID),APIOut->unVARDeviceID);//VAR Device ID

	return(inTMOutLen);
}

int inBuildLockCardData9(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
//int inBuildLockCardData8(int inTxnType,LockCard_TM_Out_2 *APIOut,LockCard_APDU_Out_2 *DongleOut,BYTE *ucTxnDateTime,BYTE ucLockReason,BYTE *ucTMLocationID,BYTE *ucTMID)
{
int inTMOutLen;
BYTE bBLCName[50];
LockCard_TM_Out_2 *APIOut = (LockCard_TM_Out_2 *)API_Out;
LockCard_APDU_Out_2 *DongleOut = (LockCard_APDU_Out_2 *)Dongle_Out;
BYTE ucTxnDateTime[5],ucLockReason,*ucTMLocationID,*ucTMID,ucDateTime[20];

	ucTMLocationID = (BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID;
	ucTMID = (BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMID;
	ucLockReason = (DongleOut->ucStatusCode[1] == 0x06)?0x01:(DongleOut->ucStatusCode[1] == 0x03)?0x02:0x03;
	memset(ucTxnDateTime,0x00,sizeof(ucTxnDateTime));
	memset(ucDateTime,0x00,sizeof(ucDateTime));
	memcpy(&ucDateTime[0],srTxnData.srIngData.chTxDate,8);
	memcpy(&ucDateTime[8],srTxnData.srIngData.chTxTime,6);
	fngetUnixTimeCnt(ucTxnDateTime,ucDateTime); //交易日期時間(UnixDateTime)

	inTMOutLen = sizeof(LockCard_TM_Out_2);
	memset(APIOut,0x00,inTMOutLen);
	fnUnPack(&DongleOut->ucPurseVersionNumber,sizeof(DongleOut->ucPurseVersionNumber),APIOut->unPurseVersionNumber);//Purse Version Number
	sprintf((char *)APIOut->unMsgType,"22");//Msg Type
	sprintf((char *)APIOut->unSubType,"00");//Sub Type
	fnUnPack(DongleOut->ucDeviceID,sizeof(DongleOut->ucDeviceID),APIOut->unDeviceID);//Device ID
	fnUnPack(&DongleOut->ucSPID,sizeof(DongleOut->ucSPID),APIOut->unSPID);//SP ID
	fnUnPack(ucTxnDateTime,sizeof(APIOut->unTxnDateTime) / 2,APIOut->unTxnDateTime);//Txn Date Time
	fnUnPack(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),APIOut->unCardID);//Card ID
	fnUnPack(&DongleOut->ucIssuerCode,sizeof(DongleOut->ucIssuerCode),APIOut->unIssuerCode);//Issuer Code
	fnUnPack(&DongleOut->ucLocationID,sizeof(DongleOut->ucLocationID),APIOut->unLocationID);//Location ID
	fnUnPack(&DongleOut->ucCardIDLen,sizeof(DongleOut->ucCardIDLen),APIOut->unCardIDLen);//Card ID Len
	fnUnPack(DongleOut->ucCPUDeviceID,sizeof(DongleOut->ucCPUDeviceID),APIOut->unCPUDeviceID);//CPU Device ID
	fnUnPack(DongleOut->ucCPUSPID,sizeof(DongleOut->ucCPUSPID),APIOut->unCPUSPID);//CPU SP ID
	fnUnPack(DongleOut->ucCPULocationID,sizeof(DongleOut->ucCPULocationID),APIOut->unCPULocationID);//CPU Location ID
	fnUnPack(&DongleOut->ucCardType,sizeof(DongleOut->ucCardType),APIOut->unCardType);//Card Type
	fnUnPack(&DongleOut->ucPersonalProfile,sizeof(DongleOut->ucPersonalProfile),APIOut->unPersonalProfile);//Personal Profile
	fnUnPack(DongleOut->ucPID,sizeof(DongleOut->ucPID),APIOut->unPID);//CPU Purse ID
	fnUnPack(DongleOut->ucCTC,sizeof(DongleOut->ucCTC),APIOut->unCTC);//CTC
	fnUnPack(&ucLockReason,sizeof(ucLockReason),APIOut->unLockReason);//Lock Reason
	if(ucLockReason == 0x01)
	{
		BYTE gsBlackListVersion[30];

		memset(gsBlackListVersion,0x00,sizeof(gsBlackListVersion));
		inGetBLCName(gsBlackListVersion);
		memset(bBLCName,0x00,sizeof(bBLCName));
		sprintf((char *)bBLCName,"BLC%s.BIG",gsBlackListVersion);
		vdPad((char *)bBLCName,(char *)APIOut->unBLCFileName,sizeof(APIOut->unBLCFileName),TRUE,0x20);//BLC File Name
		//vdPad(gsBlackListVersion,APIOut->unBLCFileName,sizeof(APIOut->unBLCFileName),TRUE,0x20);//BLC File Name
		APIOut->unBLCIDFlag = gsBlackListVersion[17];//BLC ID Flag
		if(APIOut->unBLCIDFlag == 0x00)
			APIOut->unBLCIDFlag = 0x20;
	}
	else
	{
		memset(APIOut->unBLCFileName,0x20,sizeof(APIOut->unBLCFileName));//BLC File Name
		APIOut->unBLCIDFlag = 0x20;//BLC ID Flag
	}
	memcpy(APIOut->unTMLocationID,ucTMLocationID,sizeof(APIOut->unTMLocationID));//TM Location ID
	memcpy(APIOut->unTMID,ucTMID,sizeof(APIOut->unTMID));//TM ID
	memset(APIOut->unRFU,0x20,sizeof(APIOut->unRFU));//RFU
	memset(APIOut->unEV,0x30,sizeof(APIOut->unEV));//EV
	memset(APIOut->unCPUEV,0x30,sizeof(APIOut->unCPUEV));//CPU EV
	memset(APIOut->unMSettingPar,0x30,sizeof(APIOut->unMSettingPar));//M SettingPar
	memset(APIOut->unCSettingPar,0x30,sizeof(APIOut->unCSettingPar));//C SettingPar
	memset(APIOut->unTxnSN,0x30,sizeof(APIOut->unTxnSN));//Txn SN
	memset(APIOut->unVARTxnSN,0x30,sizeof(APIOut->unVARTxnSN));//VAR Txn SN
	memset(APIOut->unVARTxnDateTime,0x30,sizeof(APIOut->unVARTxnDateTime));//VAR Txn Date Time
	memset(APIOut->unVARSubType,0x30,sizeof(APIOut->unVARSubType));//VAR Sub Type
	memset(APIOut->unVARTxnAmt,0x30,sizeof(APIOut->unVARTxnAmt));//VAR Txn Amt
	memset(APIOut->unVAREV,0x30,sizeof(APIOut->unVAREV));//VAR EV
	memset(APIOut->unVARDeviceID,0x30,sizeof(APIOut->unVARDeviceID));//VAR Device ID

	return(inTMOutLen);
}

int inBuildReadDongleDeductData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
ReadDongleDeduct_TM_Out *APIOut = (ReadDongleDeduct_TM_Out *)API_Out;
ReadDongleDeduct_APDU_Out *DongleOut = (ReadDongleDeduct_APDU_Out *)Dongle_Out;

	inTMOutLen = sizeof(ReadDongleDeduct_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	vdUIntToAsc(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),APIOut->stSTMC_t.anCardID,sizeof(APIOut->stSTMC_t.anCardID),FALSE,' ',10);//n_Card Physical ID
	fnBINTODEVASC(DongleOut->ucDeviceID,APIOut->stSTMC_t.anDeviceID,sizeof(APIOut->stSTMC_t.anDeviceID),MIFARE);//n_TXN Device ID
	fnBINTODEVASC(DongleOut->ucCPUDeviceID,APIOut->stSTMC_t.anCPUDeviceID,sizeof(APIOut->stSTMC_t.anCPUDeviceID),LEVEL2);//n_TXN CPU Device ID
	vdCountEVBefore(DongleOut->ucTxnAmt,sizeof(DongleOut->ucTxnAmt),DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.anEVBeforeTxn,sizeof(APIOut->stSTMC_t.anEVBeforeTxn),DongleOut->ucMsgType);//n_EV Before TXN
	vdUIntToAsc(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),APIOut->stSTMC_t.anTxnSN,sizeof(APIOut->stSTMC_t.anTxnSN),FALSE,' ',10);//n_TXN SNum
	vdUIntToAsc(DongleOut->ucTxnAmt,sizeof(DongleOut->ucTxnAmt),APIOut->stSTMC_t.anTxnAmt,sizeof(APIOut->stSTMC_t.anTxnAmt),FALSE,' ',10);//n_TXN AMT
	vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.anEV,sizeof(APIOut->stSTMC_t.anEV),FALSE,' ',10);//n_EV
    //Processing Code
    if (DongleOut->ucTxnAMT1[0] == 0xFF && DongleOut->ucTxnAMT1[1] == 0xFF &&
        DongleOut->ucTxnAMT2[0] == 0xFF && DongleOut->ucTxnAMT2[1] == 0xFF &&
        DongleOut->ucTxnAMT3[0] == 0xFF && DongleOut->ucTxnAMT3[1] == 0xFF && 
        DongleOut->ucTxnAMT4[0] == 0xFF && DongleOut->ucTxnAMT4[1] == 0xFF &&
        DongleOut->ucTxnAMT5[0] == 0xFF && DongleOut->ucTxnAMT5[1] == 0xFF)
		vdBuildProcessingCode(DLL_DEDUCT_VALUE_AUTH,DongleOut->ucMsgType,DongleOut->ucSubType,DongleOut->ucPersonalProfile,APIOut->stSTMC_t.anProcessignCode);//Processing Code
    else
        memcpy(APIOut->stSTMC_t.anProcessignCode, "819599", 6); //3合1購貨
	//vdBuildProcessingCode(API_PPR_AUTHTXNOFFLINE,DongleOut->ucMsgType,DongleOut->ucSubType,DongleOut->ucPersonalProfile,APIOut->stSTMC_t.anProcessignCode);//Processing Code
	fnUnPack(&DongleOut->ucMsgType,sizeof(DongleOut->ucMsgType),APIOut->stSTMC_t.unMsgType);//Msg Type
	fnUnPack(&DongleOut->ucSubType,sizeof(DongleOut->ucSubType),APIOut->stSTMC_t.unSubType);//Sub Type
	fnUnPack(DongleOut->ucDeviceID,sizeof(DongleOut->ucDeviceID),APIOut->stSTMC_t.unDeviceID);//Device ID
	fnUnPack(&DongleOut->ucSPID,sizeof(DongleOut->ucSPID),APIOut->stSTMC_t.unSPID);//SP ID
	fnUnPack(DongleOut->ucTxnDateTime,sizeof(DongleOut->ucTxnDateTime),APIOut->stSTMC_t.unTxnDateTime);//Txn Date Time
	fnUnPack(&DongleOut->ucCardType,sizeof(DongleOut->ucCardType),APIOut->stSTMC_t.unCardType);//Card Type
	fnUnPack(&DongleOut->ucPersonalProfile,sizeof(DongleOut->ucPersonalProfile),APIOut->stSTMC_t.unPersonalProfile);//Personal Profile
	fnUnPack(&DongleOut->ucLocationID,sizeof(DongleOut->ucLocationID),APIOut->stSTMC_t.unLocationID);//Location ID
	fnUnPack(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),APIOut->stSTMC_t.unCardID);//CPU Card ID
	fnUnPack(&DongleOut->ucIssuerCode,sizeof(DongleOut->ucIssuerCode),APIOut->stSTMC_t.unIssuerCode);//Issuer Code
	if(DongleOut->ucPurseVersionNumber == MIFARE)
	{
		fnUnPack(&DongleOut->ucBankCode,1,APIOut->stSTMC_t.unBankCode);//Bank Code
		fnUnPack(DongleOut->ucLoyaltyCounter,2,APIOut->stSTMC_t.unLoyaltyCounter);//Loyalty Counter
		fnUnPack(DongleOut->ucMAC_HCrypto,10,APIOut->stSTMC_t.unMAC);//MAC
	}
	else
	{
		memset(APIOut->stSTMC_t.unBankCode,0x30,sizeof(APIOut->stSTMC_t.unBankCode));//Bank Counter
		memset(APIOut->stSTMC_t.unLoyaltyCounter,0x30,sizeof(APIOut->stSTMC_t.unLoyaltyCounter));//Loyalty Counter
		memset(APIOut->stSTMC_t.unMAC,0x30,sizeof(APIOut->stSTMC_t.unMAC));//MAC
	}
	memset(APIOut->stSTMC_t.unOrgExpiryDate,0x30,sizeof(APIOut->stSTMC_t.unOrgExpiryDate));//Org Expiry Date
	memset(APIOut->stSTMC_t.unNewExpiryDate,0x30,sizeof(APIOut->stSTMC_t.unNewExpiryDate));//New Expiry Date
	fnUnPack(&DongleOut->ucAreaCode,sizeof(DongleOut->ucAreaCode),APIOut->stSTMC_t.unAreaCode);//Area Code
	fnUnPack(DongleOut->ucTxnAmt,sizeof(DongleOut->ucTxnAmt),APIOut->stSTMC_t.unTxnAmt);//Txn Amt
	fnUnPack(DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.unEV);//EV
	fnUnPack(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),APIOut->stSTMC_t.unTxnSN);//Txn SN
	fnUnPack(DongleOut->ucCPUDeviceID,sizeof(DongleOut->ucCPUDeviceID),APIOut->stSTMC_t.unCPUDeviceID);//CPU Device ID
	fnUnPack(DongleOut->ucCPUSPID,sizeof(DongleOut->ucCPUSPID),APIOut->stSTMC_t.unCPUSPID);//CPU SP ID
	fnUnPack(DongleOut->ucCPULocationID,sizeof(DongleOut->ucCPULocationID),APIOut->stSTMC_t.unCPULocationID);//CPU Location ID
	fnUnPack(DongleOut->ucPID,sizeof(DongleOut->ucPID),APIOut->stSTMC_t.unPID);//Purse ID
	fnUnPack(DongleOut->ucCTC,sizeof(DongleOut->ucCTC),APIOut->stSTMC_t.unCTC);//CTC
	memset(APIOut->stSTMC_t.unOrgProfileExpiryDate,0x30,sizeof(APIOut->stSTMC_t.unOrgProfileExpiryDate));//Org Profile Expiry Date
	memset(APIOut->stSTMC_t.unNewProfileExpiryDate,0x30,sizeof(APIOut->stSTMC_t.unNewProfileExpiryDate));//New Profile Expiry Date
	fnUnPack(DongleOut->ucSubAreaCode,sizeof(DongleOut->ucSubAreaCode),APIOut->stSTMC_t.unSubAreaCode);//Sub Area Code
	fnUnPack(DongleOut->ucTxnSNBeforeTxn,sizeof(DongleOut->ucTxnSNBeforeTxn),APIOut->stSTMC_t.unTxnSNBeforeTxn);//Txn SN Before Txn
	//fnUnPack(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),APIOut->stSTMC_t.unTxnSNBeforeTxn);//Txn SN Before Txn
	fnUnPack(DongleOut->ucEVBeforeTxn,sizeof(DongleOut->ucEVBeforeTxn),APIOut->stSTMC_t.unEVBeforeTxn);//EV Before Txn
	fnUnPack(DongleOut->ucDeposit,sizeof(DongleOut->ucDeposit),APIOut->stSTMC_t.unDeposit);//Deposit
	fnUnPack(&DongleOut->ucTxnMode,sizeof(DongleOut->ucTxnMode),APIOut->stSTMC_t.unTxnMode);//Txn Mode
	fnUnPack(&DongleOut->ucTxnQuqlifier,sizeof(DongleOut->ucTxnQuqlifier),APIOut->stSTMC_t.unTxnQuqlifier);//Txn Quqlifier
	fnUnPack(&DongleOut->ucSignatureKeyKVN,sizeof(DongleOut->ucSignatureKeyKVN),APIOut->stSTMC_t.unSignatureKeyKVN);//Signature Key KVN
	fnUnPack(DongleOut->ucSignature,sizeof(DongleOut->ucSignature),APIOut->stSTMC_t.unSignature);//Signature
	fnUnPack(DongleOut->ucCPUSAMID,sizeof(DongleOut->ucCPUSAMID),APIOut->stSTMC_t.unCPUSAMID);//SAM ID
	if(DongleOut->ucPurseVersionNumber != MIFARE)
	{
		fnUnPack(&DongleOut->ucMAC_HCrypto[0],1,APIOut->stSTMC_t.unHashType);//Hash Type
		fnUnPack(&DongleOut->ucMAC_HCrypto[1],1,APIOut->stSTMC_t.unHostAdminKVN);//Host Admin KVN
		fnUnPack(&DongleOut->ucMAC_HCrypto[2],16,APIOut->stSTMC_t.unCPUMAC);//MAC
	}
	else
	{
		memset(APIOut->stSTMC_t.unHashType,0x30,sizeof(APIOut->stSTMC_t.unHashType));
		memset(APIOut->stSTMC_t.unHostAdminKVN,0x30,sizeof(APIOut->stSTMC_t.unHostAdminKVN));
		memset(APIOut->stSTMC_t.unCPUMAC,0x30,sizeof(APIOut->stSTMC_t.unCPUMAC));//MAC
	}
	fnUnPack(&DongleOut->ucPurseVersionNumber,sizeof(DongleOut->ucPurseVersionNumber),APIOut->stSTMC_t.unPurseVersionNumber);//Purse Version Number
	fnUnPack(&DongleOut->ucCardIDLen,sizeof(DongleOut->ucCardIDLen),APIOut->stSTMC_t.unCardIDLen);//Card ID Len
	memcpy(APIOut->stSTMC_t.ucTMLocationID,DongleOut->ucTMLocationID,sizeof(DongleOut->ucTMLocationID));//TM Location ID
	memcpy(APIOut->stSTMC_t.ucTMID,DongleOut->ucTMID,sizeof(DongleOut->ucTMID));//TM ID
	UnixToDateTime((BYTE*)DongleOut->ucTxnDateTime,(BYTE*)APIOut->stSTMC_t.ucTMTxnDateTime,14);//TM Txn Date Time
	memcpy(APIOut->stSTMC_t.ucTMSerialNumber,DongleOut->ucTMSerialNumber,sizeof(DongleOut->ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->stSTMC_t.ucTMAgentNumber,DongleOut->ucTMAgentNumber,sizeof(DongleOut->ucTMAgentNumber));//TM Agent Number
	if (DongleOut->ucTxnAMT1[0] == 0xFF && DongleOut->ucTxnAMT1[1] == 0xFF &&
        DongleOut->ucTxnAMT2[0] == 0xFF && DongleOut->ucTxnAMT2[1] == 0xFF &&
        DongleOut->ucTxnAMT3[0] == 0xFF && DongleOut->ucTxnAMT3[1] == 0xFF &&
        DongleOut->ucTxnAMT4[0] == 0xFF && DongleOut->ucTxnAMT4[1] == 0xFF &&
        DongleOut->ucTxnAMT5[0] == 0xFF && DongleOut->ucTxnAMT5[1] == 0xFF)
	{
		memset(APIOut->unSTMCAmt1,0x20,sizeof(APIOut->unSTMCAmt1));
		memset(APIOut->unSTMCAmt2,0x20,sizeof(APIOut->unSTMCAmt2));
		memset(APIOut->unSTMCAmt3,0x20,sizeof(APIOut->unSTMCAmt3));
		memset(APIOut->unSTMCAmt4,0x20,sizeof(APIOut->unSTMCAmt4));
		memset(APIOut->unSTMCAmt5,0x20,sizeof(APIOut->unSTMCAmt5));
	}
	else
	{
	    fnUnPack(DongleOut->ucTxnAMT1,sizeof(DongleOut->ucTxnAMT1),APIOut->unSTMCAmt1);//Txn AMT1
	    fnUnPack(DongleOut->ucTxnAMT2,sizeof(DongleOut->ucTxnAMT2),APIOut->unSTMCAmt2);//Txn AMT2
	    fnUnPack(DongleOut->ucTxnAMT3,sizeof(DongleOut->ucTxnAMT3),APIOut->unSTMCAmt3);//Txn AMT3
	    if (DongleOut->ucTxnAMT4[0] == 0xFF && DongleOut->ucTxnAMT4[1] == 0xFF &&
        	DongleOut->ucTxnAMT5[0] == 0xFF && DongleOut->ucTxnAMT5[1] == 0xFF)
	    {
	    	memset(APIOut->unSTMCAmt4,0x30,sizeof(APIOut->unSTMCAmt4));
			memset(APIOut->unSTMCAmt5,0x30,sizeof(APIOut->unSTMCAmt5));
	    }
		else
		{
	    	fnUnPack(DongleOut->ucTxnAMT4,sizeof(DongleOut->ucTxnAMT2),APIOut->unSTMCAmt4);//Txn AMT4
	    	fnUnPack(DongleOut->ucTxnAMT5,sizeof(DongleOut->ucTxnAMT3),APIOut->unSTMCAmt5);//Txn AMT5
		}
	}
	//帳務資料結束,加值重送資料開始
	memcpy(APIOut->unVARMsgType,"0C",2);//VAR Msg Type
	fnUnPack(&DongleOut->stLastCreditTxnLogInfo_t.ucPurseVersionNumber_Index,sizeof(DongleOut->stLastCreditTxnLogInfo_t.ucPurseVersionNumber_Index),APIOut->unVARPurseVersionNumber);//VAR PurseVersion Number
	fnUnPack(DongleOut->stLastCreditTxnLogInfo_t.ucTxnSN,sizeof(DongleOut->stLastCreditTxnLogInfo_t.ucTxnSN),APIOut->unVARTxnSN);//VAR Txn SN
	fnUnPack(DongleOut->stLastCreditTxnLogInfo_t.ucTxnDateTime,sizeof(DongleOut->stLastCreditTxnLogInfo_t.ucTxnDateTime),APIOut->unVARTxnDateTime);//VAR Txn Date Time
	fnUnPack(&DongleOut->stLastCreditTxnLogInfo_t.ucSubType,sizeof(DongleOut->stLastCreditTxnLogInfo_t.ucSubType),APIOut->unVARSubType);//VAR Sub Type
	fnUnPack(DongleOut->stLastCreditTxnLogInfo_t.ucTxnAmt,sizeof(DongleOut->stLastCreditTxnLogInfo_t.ucTxnAmt),APIOut->unVARTxnAmt);//VAR Txn Amt
	fnUnPack(DongleOut->stLastCreditTxnLogInfo_t.ucEV,sizeof(DongleOut->stLastCreditTxnLogInfo_t.ucEV),APIOut->unVAREV);//VAR EV
	fnUnPack(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),APIOut->unVARCardID);//VAR Card ID
	fnUnPack(&DongleOut->ucIssuerCode,sizeof(DongleOut->ucIssuerCode),APIOut->unVARIssuerCode);//VAR Issuer Code
	fnUnPack(DongleOut->stLastCreditTxnLogInfo_t.ucCPUSPID,sizeof(DongleOut->stLastCreditTxnLogInfo_t.ucCPUSPID),APIOut->unVARCPUSPID);//VAR CPU SP ID
	fnUnPack(DongleOut->stLastCreditTxnLogInfo_t.ucCPULocationID,sizeof(DongleOut->stLastCreditTxnLogInfo_t.ucCPULocationID),APIOut->unVARCPULocationID);//VAR CPU Location ID
	fnUnPack(DongleOut->ucPID,sizeof(DongleOut->ucPID),APIOut->unVARPID);//VAR Purse ID
	fnUnPack(DongleOut->stLastCreditTxnLogInfo_t.ucCPUDeviceID,sizeof(DongleOut->stLastCreditTxnLogInfo_t.ucCPUDeviceID),APIOut->unVARCPUDeviceID);//VAR CPU Device ID
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		fnUnPack(DongleOut->ucLoyaltyCounter,2,APIOut->unVARLoyaltyCounter);//VAR Loyalty Counter
	else
		memset(APIOut->unVARLoyaltyCounter,0x30,sizeof(APIOut->unVARLoyaltyCounter));//VAR Loyalty Counter
	fnUnPack(&DongleOut->ucPersonalProfile,sizeof(DongleOut->ucPersonalProfile),APIOut->unVARPersonalProfile);//VAR Personal Profile
	if(DongleOut->ucPurseVersionNumber == MIFARE)
		fnUnPack(&DongleOut->ucBankCode,1,APIOut->unVARBankCode);//VAR Bank Code
	else
		memset(APIOut->unVARBankCode,0x30,sizeof(APIOut->unVARBankCode));//VAR Bank Counter
	fnUnPack(&DongleOut->ucCardIDLen,sizeof(DongleOut->ucCardIDLen),APIOut->unVARCardIDLen);//VAR Card ID Len

	fnUnPack(DongleOut->ucDeviceID,sizeof(DongleOut->ucDeviceID),APIOut->unVAR2DeviceID);//VAR Device ID
	fnUnPack(&DongleOut->ucSPID,sizeof(DongleOut->ucSPID),APIOut->unVAR2SPID);//VAR SP ID
	fnUnPack(&DongleOut->ucLocationID,sizeof(DongleOut->ucLocationID),APIOut->unVAR2LocationID);//VAR SP ID
	fnUnPack(DongleOut->ucCPUDeviceID,sizeof(DongleOut->ucCPUDeviceID),APIOut->unVAR2CPUDeviceID);//VAR CPU Device ID
	fnUnPack(DongleOut->ucCPUSPID,sizeof(DongleOut->ucCPUSPID),APIOut->unVAR2CPUSPID);//VAR CPU SP ID
	fnUnPack(DongleOut->ucCPULocationID,sizeof(DongleOut->ucCPULocationID),APIOut->unVAR2CPULocationID);//VAR CPU Location ID

	if(srTxnData.srParameter.chMultiDeductFlag == '1')
		return(inTMOutLen);
	else
	{
		memcpy(APIOut->unSTMCAmt1,APIOut->unVARMsgType,TAVR_TARC_LEN);
		memset(&APIOut->unVAR2CPUDeviceID[2],0x00,20);
		return(inTMOutLen - 20);
	}
}

int inBuildAuthCardSaleData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
AuthCardSale_TM_Out *APIOut = (AuthCardSale_TM_Out *)API_Out;
AuthTxnOnline_APDU_In *DongleIn = (AuthTxnOnline_APDU_In *)Dongle_In;
AuthTxnOnline_APDU_Out *DongleOut = (AuthTxnOnline_APDU_Out *)Dongle_Out;
TxnReqOnline_APDU_Out *TxnReqOut = (TxnReqOnline_APDU_Out *)Dongle_ReqOut;

	inTMOutLen = sizeof(AuthCardSale_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	vdUIntToAsc(TxnReqOut->ucCardID,sizeof(TxnReqOut->ucCardID),APIOut->stSTMC_t.anCardID,sizeof(APIOut->stSTMC_t.anCardID),FALSE,' ',10);//n_Card Physical ID
	fnBINTODEVASC(TxnReqOut->ucDeviceID,APIOut->stSTMC_t.anDeviceID,sizeof(APIOut->stSTMC_t.anDeviceID),MIFARE);//n_TXN Device ID
	fnBINTODEVASC(TxnReqOut->ucCPUDeviceID,APIOut->stSTMC_t.anCPUDeviceID,sizeof(APIOut->stSTMC_t.anCPUDeviceID),LEVEL2);//n_TXN Device ID
	vdIntToAsc(TxnReqOut->ucEV,sizeof(TxnReqOut->ucEV),APIOut->stSTMC_t.anEVBeforeTxn,sizeof(APIOut->stSTMC_t.anEVBeforeTxn),FALSE,' ',10);//n_TXN SNum
	//vdCountEVBefore(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.anEVBeforeTxn,sizeof(APIOut->stSTMC_t.anEVBeforeTxn),TxnReqOut->ucMsgType);//n_EV Before TXN
	vdUIntToAsc(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),APIOut->stSTMC_t.anTxnSN,sizeof(APIOut->stSTMC_t.anTxnSN),FALSE,' ',10);//n_TXN SNum
	vdUIntToAsc(TxnReqOut->ucDeposit,sizeof(TxnReqOut->ucDeposit),APIOut->stSTMC_t.anTxnAmt,sizeof(APIOut->stSTMC_t.anTxnAmt),FALSE,' ',10);//n_TXN AMT
	vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.anEV,sizeof(APIOut->stSTMC_t.anEV),FALSE,' ',10);//n_EV
	vdBuildProcessingCode(inTxnType,TxnReqOut->ucMsgType,TxnReqOut->ucSubType,TxnReqOut->ucPersonalProfile,APIOut->stSTMC_t.anProcessignCode);//Processing Code
	fnUnPack(&TxnReqOut->ucMsgType,sizeof(TxnReqOut->ucMsgType),APIOut->stSTMC_t.unMsgType);//Msg Type
	fnUnPack(&TxnReqOut->ucSubType,sizeof(TxnReqOut->ucSubType),APIOut->stSTMC_t.unSubType);//Sub Type
	fnUnPack(TxnReqOut->ucDeviceID,sizeof(TxnReqOut->ucDeviceID),APIOut->stSTMC_t.unDeviceID);//Device ID
	fnUnPack(&TxnReqOut->ucSPID,sizeof(TxnReqOut->ucSPID),APIOut->stSTMC_t.unSPID);//SP ID
	fnUnPack(DongleOut->ucTxnDateTime,sizeof(DongleOut->ucTxnDateTime),APIOut->stSTMC_t.unTxnDateTime);//Txn Date Time
	fnUnPack(&TxnReqOut->ucCardType,sizeof(TxnReqOut->ucCardType),APIOut->stSTMC_t.unCardType);//Card Type
	fnUnPack(&TxnReqOut->ucPersonalProfile,sizeof(TxnReqOut->ucPersonalProfile),APIOut->stSTMC_t.unPersonalProfile);//Personal Profile
	fnUnPack(&TxnReqOut->ucLocationID,sizeof(TxnReqOut->ucLocationID),APIOut->stSTMC_t.unLocationID);//Location ID
	fnUnPack(TxnReqOut->ucCardID,sizeof(TxnReqOut->ucCardID),APIOut->stSTMC_t.unCardID);//CPU Card ID
	fnUnPack(&TxnReqOut->ucIssuerCode,sizeof(TxnReqOut->ucIssuerCode),APIOut->stSTMC_t.unIssuerCode);//Issuer Code
	fnUnPack(&TxnReqOut->ucBankCode,sizeof(TxnReqOut->ucBankCode),APIOut->stSTMC_t.unBankCode);//Bank Code
	fnUnPack(TxnReqOut->ucLoyaltyCounter,sizeof(TxnReqOut->ucLoyaltyCounter),APIOut->stSTMC_t.unLoyaltyCounter);//Loyalty Counter
	if(TxnReqOut->ucPurseVersionNumber == MIFARE)
	{
		if (TxnReqOut->ucReaderFWVersion[5] >= 0x80) //match sam的mfrc改用device id
			memcpy(&DongleOut->ucMAC_HCrypto[6], TxnReqOut->ucDeviceID, 4);
		fnUnPack(DongleOut->ucMAC_HCrypto,10,APIOut->stSTMC_t.unMAC);//MAC
	}
	else
		memset(APIOut->stSTMC_t.unMAC,0x30,sizeof(APIOut->stSTMC_t.unMAC));//MAC
	fnUnPack(TxnReqOut->ucExpiryDate,sizeof(TxnReqOut->ucExpiryDate),APIOut->stSTMC_t.unOrgExpiryDate);//Org Expiry Date
	fnUnPack(DongleIn->ucExpiryDate,sizeof(DongleIn->ucExpiryDate),APIOut->stSTMC_t.unNewExpiryDate);//New Expiry Date
	fnUnPack(&TxnReqOut->ucAreaCode,sizeof(TxnReqOut->ucAreaCode),APIOut->stSTMC_t.unAreaCode);//Area Code
	fnUnPack(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),APIOut->stSTMC_t.unTxnAmt);//Txn Amt
	fnUnPack(DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.unEV);//EV
	fnUnPack(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),APIOut->stSTMC_t.unTxnSN);//Txn SN
	fnUnPack(TxnReqOut->ucCPUDeviceID,sizeof(TxnReqOut->ucCPUDeviceID),APIOut->stSTMC_t.unCPUDeviceID);//CPU Device ID
	fnUnPack(TxnReqOut->ucCPUSPID,sizeof(TxnReqOut->ucCPUSPID),APIOut->stSTMC_t.unCPUSPID);//CPU SP ID
	fnUnPack(TxnReqOut->ucCPULocationID,sizeof(TxnReqOut->ucCPULocationID),APIOut->stSTMC_t.unCPULocationID);//CPU Location ID
	fnUnPack(TxnReqOut->ucPID,sizeof(TxnReqOut->ucPID),APIOut->stSTMC_t.unPID);//Purse ID
	fnUnPack(TxnReqOut->ucCTC,sizeof(TxnReqOut->ucCTC),APIOut->stSTMC_t.unCTC);//CTC
	fnUnPack(TxnReqOut->ucProfileExpiryDate,sizeof(TxnReqOut->ucProfileExpiryDate),APIOut->stSTMC_t.unOrgProfileExpiryDate);//Org Profile Expiry Date
	fnUnPack(DongleIn->ucProfileExpiryDate,sizeof(DongleIn->ucProfileExpiryDate),APIOut->stSTMC_t.unNewProfileExpiryDate);//New Profile Expiry Date
	fnUnPack(TxnReqOut->ucSubAreaCode,sizeof(TxnReqOut->ucSubAreaCode),APIOut->stSTMC_t.unSubAreaCode);//Sub Area Code
	fnUnPack(TxnReqOut->ucTxnSN,sizeof(TxnReqOut->ucTxnSN),APIOut->stSTMC_t.unTxnSNBeforeTxn);//Txn SN Before Txn
	fnUnPack(TxnReqOut->ucEV,sizeof(TxnReqOut->ucEV),APIOut->stSTMC_t.unEVBeforeTxn);//EV Before Txn
	fnUnPack(TxnReqOut->ucDeposit,sizeof(TxnReqOut->ucDeposit),APIOut->stSTMC_t.unDeposit);//Deposit
	fnUnPack(&TxnReqOut->ucTxnMode,sizeof(TxnReqOut->ucTxnMode),APIOut->stSTMC_t.unTxnMode);//Txn Mode
	fnUnPack(&TxnReqOut->ucTxnQuqlifier,sizeof(TxnReqOut->ucTxnQuqlifier),APIOut->stSTMC_t.unTxnQuqlifier);//Txn Quqlifier
	fnUnPack(&TxnReqOut->ucSignatureKeyKVN,sizeof(TxnReqOut->ucSignatureKeyKVN),APIOut->stSTMC_t.unSignatureKeyKVN);//Signature Key KVN
	fnUnPack(DongleOut->ucSignature,sizeof(DongleOut->ucSignature),APIOut->stSTMC_t.unSignature);//Signature
	fnUnPack(DongleOut->ucCPUSAMID,sizeof(DongleOut->ucCPUSAMID),APIOut->stSTMC_t.unCPUSAMID);//SAM ID
	if(TxnReqOut->ucPurseVersionNumber != MIFARE)
		fnUnPack(&DongleOut->ucMAC_HCrypto[0],1,APIOut->stSTMC_t.unHashType);//Hash Type
	else
		memset(APIOut->stSTMC_t.unHashType,0x30,sizeof(APIOut->stSTMC_t.unHashType));
	if(TxnReqOut->ucPurseVersionNumber != MIFARE)
	{
		fnUnPack(&DongleOut->ucMAC_HCrypto[1],1,APIOut->stSTMC_t.unHostAdminKVN);//Host Admin KVN
		fnUnPack(&DongleOut->ucMAC_HCrypto[2],16,APIOut->stSTMC_t.unCPUMAC);//MAC
	}
	else
	{
		memset(APIOut->stSTMC_t.unHostAdminKVN,0x30,sizeof(APIOut->stSTMC_t.unHostAdminKVN));
		memset(APIOut->stSTMC_t.unCPUMAC,0x30,sizeof(APIOut->stSTMC_t.unCPUMAC));//MAC
	}
	fnUnPack(&TxnReqOut->ucPurseVersionNumber,sizeof(TxnReqOut->ucPurseVersionNumber),APIOut->stSTMC_t.unPurseVersionNumber);//Purse Version Number
	fnUnPack(&TxnReqOut->ucCardIDLen,sizeof(TxnReqOut->ucCardIDLen),APIOut->stSTMC_t.unCardIDLen);//Card ID Len
	memcpy(APIOut->stSTMC_t.ucTMLocationID,stStore_DataInfo_t.ucTMLocationID,sizeof(stStore_DataInfo_t.ucTMLocationID));//TM Location ID
	memcpy(APIOut->stSTMC_t.ucTMID,stStore_DataInfo_t.ucTMID,sizeof(stStore_DataInfo_t.ucTMID));//TM ID
	UnixToDateTime((BYTE*)DongleOut->ucTxnDateTime,(BYTE*)APIOut->stSTMC_t.ucTMTxnDateTime,14);//TM Txn Date Time
	memcpy(APIOut->stSTMC_t.ucTMSerialNumber,stStore_DataInfo_t.ucTMSerialNumber,sizeof(stStore_DataInfo_t.ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->stSTMC_t.ucTMAgentNumber,stStore_DataInfo_t.ucTMAgentNumber,sizeof(stStore_DataInfo_t.ucTMAgentNumber));//TM Agent Number
	//帳務資料結束
	//Confirm資料開始
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",81);//Data Len
	vdBuildProcessingCode(inTxnType,TxnReqOut->ucMsgType,TxnReqOut->ucSubType,TxnReqOut->ucPersonalProfile,APIOut->ucProcessignCode);//Processing Code
	APIOut->ucMsgType = TxnReqOut->ucMsgType;//Msg Type
	APIOut->ucSubType = TxnReqOut->ucSubType;//Sub Type
	memcpy(APIOut->ucDeviceID,TxnReqOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	memcpy(APIOut->ucTxnDateTime,DongleOut->ucTxnDateTime,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	if(TxnReqOut->ucPurseVersionNumber == MIFARE)
		memcpy(APIOut->ucMAC,DongleOut->ucMAC_HCrypto,sizeof(APIOut->ucMAC));//MAC
	else
		memset(APIOut->ucMAC,0x00,sizeof(APIOut->ucMAC));//MAC
	memcpy(APIOut->ucConfirmCode,DongleOut->ucConfirmCode,sizeof(APIOut->ucConfirmCode));//Confirm Code
	if(TxnReqOut->ucPurseVersionNumber == MIFARE)
		memcpy(APIOut->ucCTAC,&DongleOut->ucMAC_HCrypto[10],sizeof(APIOut->ucMAC));//CTAC
	else
		memset(APIOut->ucCTAC,0x00,sizeof(APIOut->ucMAC));//CTAC
	memcpy(APIOut->ucCardID,TxnReqOut->ucCardID,sizeof(APIOut->ucCardID));//Card ID
	memcpy(APIOut->ucEV,DongleOut->ucEV,sizeof(APIOut->ucEV));//EV
	memcpy(APIOut->ucTxnSN,DongleOut->ucTxnSN,sizeof(APIOut->ucTxnSN));//Txn SN
	memcpy(APIOut->ucCPUDeviceID,TxnReqOut->ucCPUDeviceID,sizeof(APIOut->ucCPUDeviceID));//CPU Device ID
	memcpy(APIOut->ucPID,TxnReqOut->ucPID,sizeof(APIOut->ucPID));//Purse ID
	if(TxnReqOut->ucPurseVersionNumber != MIFARE)
	{
		APIOut->ucHashType = DongleOut->ucMAC_HCrypto[0];//Hash Type
		APIOut->ucHostAdminKVN = DongleOut->ucMAC_HCrypto[1];//Host Admin KVN
		memcpy(APIOut->ucCPUMAC,&DongleOut->ucMAC_HCrypto[2],sizeof(APIOut->ucCPUMAC));//CPU MAC
	}
	else
	{
		APIOut->ucHashType = 0x00;
		APIOut->ucHostAdminKVN = 0x00;
		memset(APIOut->ucCPUMAC,0x00,sizeof(APIOut->ucCPUMAC));//CPU MAC
	}

	return(inTMOutLen);
}

int inBuildSetValueData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
SetValue_TM_Out *APIOut = (SetValue_TM_Out *)API_Out;
SetValue_APDU_In *DongleIn = (SetValue_APDU_In *)Dongle_In;
SetValue_APDU_Out *DongleOut = (SetValue_APDU_Out *)Dongle_Out;

	inTMOutLen = sizeof(SetValue_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",inTMOutLen - 7);//Data Len
	vdBuildProcessingCode(inTxnType,0x00,0x00,DongleOut->ucPersonalProfile,APIOut->ucProcessignCode);//Processing Code
	memcpy(APIOut->ucCommandCode,"05",sizeof(APIOut->ucCommandCode));//Command Code
	vdBuildBLCVersion(APIOut->ucBLVersion);
	//fnASCTOBIN(APIOut->ucBLVersion,gsBlackListVersion,5,sizeof(APIOut->ucBLVersion),DECIMAL);//Black List Version
	APIOut->ucMsgType = DongleIn->ucMsgType;//Msg Type
	APIOut->ucSubType = 0x00;//Sub Type
	memcpy(APIOut->ucDeviceID,DongleOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	APIOut->ucSPID = DongleOut->ucDeviceID[2];//SP ID
	memcpy(APIOut->ucTxnDateTime,DongleIn->ucTxnDateTime,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	APIOut->ucCardType = DongleOut->ucCardType;//Card Type
	APIOut->ucPersonalProfile = DongleOut->ucPersonalProfile;//Personal Profile
	memcpy(APIOut->ucCardID_4,DongleOut->ucCardID_4,sizeof(APIOut->ucCardID_4));//Card ID 4
	APIOut->ucIssuerCode = DongleOut->ucIssuerCode;//Issuer Code
	memset(APIOut->ucTxnAmt_2,0x00,sizeof(APIOut->ucTxnAmt_2));//Txn Amt 2
	memcpy(APIOut->ucExpiryDate,DongleOut->ucExpiryDate,sizeof(APIOut->ucExpiryDate));//Expiry Date
	APIOut->ucAreaCode = DongleOut->ucAreaCode;//Area Code
	memcpy(APIOut->ucTMLocationID_6,DongleIn->ucTMLocationID_6,sizeof(APIOut->ucTMLocationID_6));//TM Location ID 6
	memcpy(APIOut->ucTMID,DongleIn->ucTMID,sizeof(APIOut->ucTMID));//TM ID
	memcpy(APIOut->ucTMTxnDateTime,stStore_DataInfo_t.ucTMTxnDateTime,sizeof(stStore_DataInfo_t.ucTMTxnDateTime));//TM Date Time
	memcpy(APIOut->ucTMSerialNumber,stStore_DataInfo_t.ucTMSerialNumber,sizeof(stStore_DataInfo_t.ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->ucTMAgentNumber,stStore_DataInfo_t.ucTMAgentNumber,sizeof(stStore_DataInfo_t.ucTMAgentNumber));//TM Agent Number
	memcpy(APIOut->ucEV_2,DongleOut->ucEV_2,sizeof(APIOut->ucEV_2));//EV 2
	memcpy(APIOut->ucSTAC,DongleOut->ucSTAC,sizeof(APIOut->ucSTAC));//S-TAC
	APIOut->ucKeyVersion = DongleOut->ucSAMKeyVersion;//Key Version
	memcpy(APIOut->ucSAMID,DongleOut->ucSAMID,sizeof(APIOut->ucSAMID));//SAM ID
	memcpy(APIOut->ucSAMSN,DongleOut->ucSAMSN,sizeof(APIOut->ucSAMSN));//SAM SN
	memcpy(APIOut->ucSAMCRN,DongleOut->ucSAMCRN,sizeof(APIOut->ucSAMCRN));//SAM CRN
	memcpy(APIOut->ucReaderFWVersion,DongleOut->ucReaderFWVersion,sizeof(APIOut->ucReaderFWVersion));//Reader Firmward Version

	return(inTMOutLen);
}

int inBuildAuthSetValueData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
AuthSetValue_TM_Out *APIOut = (AuthSetValue_TM_Out *)API_Out;
AuthSetValue_APDU_In *DongleIn = (AuthSetValue_APDU_In *)Dongle_In;
//AuthSetValue_APDU_Out *DongleOut = (AuthSetValue_APDU_Out *)Dongle_Out;
SetValue_APDU_Out *TxnReqOut = (SetValue_APDU_Out *)Dongle_ReqOut;

	inTMOutLen = sizeof(AuthSetValue_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	vdUIntToAsc(TxnReqOut->ucCardID_4,sizeof(TxnReqOut->ucCardID_4),APIOut->anCardID_10,sizeof(APIOut->anCardID_10),FALSE,' ',10);//n_Card Physical ID
	fnBINTODEVASC(TxnReqOut->ucDeviceID,APIOut->anDeviceID,sizeof(APIOut->anDeviceID),MIFARE);//n_TXN Device ID
	vdIntToAsc(TxnReqOut->ucEV_2,sizeof(TxnReqOut->ucEV_2),APIOut->anEVBeforeTxn_5,sizeof(APIOut->anEVBeforeTxn_5),FALSE,' ',10);//n_EV Before TXN
	UnixToDateTime((BYTE*)TxnReqOut->ucExpiryDate,(BYTE*)APIOut->anOrgExpiryDate,8);//原票卡到期日
	UnixToDateTime((BYTE*)DongleIn->ucExpiryDate,(BYTE*)APIOut->anNewExpiryDate,8);//新票卡到期日
	vdBuildProcessingCode(inTxnType,0x00,0x00,TxnReqOut->ucPersonalProfile,APIOut->anProcessignCode);//Processing Code
	memcpy(APIOut->unMsgType,"50",sizeof(APIOut->unMsgType));//Msg Type
	memset(APIOut->unSubType,0x30,sizeof(APIOut->unSubType));//Sub Type
	fnUnPack(TxnReqOut->ucDeviceID,sizeof(TxnReqOut->ucDeviceID),APIOut->unDeviceID);//Device ID
	fnUnPack(stStore_DataInfo_t.ucTxnDateTime,sizeof(stStore_DataInfo_t.ucTxnDateTime),APIOut->unTxnDateTime);//Txn Date Time
	fnUnPack(&TxnReqOut->ucCardType,sizeof(TxnReqOut->ucCardType),APIOut->unCardType);//Card Type
	fnUnPack(&TxnReqOut->ucPersonalProfile,sizeof(TxnReqOut->ucPersonalProfile),APIOut->unPersonalProfile);//Personal Profile
	fnUnPack(TxnReqOut->ucCardID_4,sizeof(TxnReqOut->ucCardID_4),APIOut->unCardID_8);//CPU Card ID
	fnUnPack(DongleIn->ucExpiryDate,sizeof(DongleIn->ucExpiryDate),APIOut->unNewExpiryDate);//New Expiry Date

	return(inTMOutLen);
}

int inBuildTxRefundData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
TxRefund_TM_Out *APIOut = (TxRefund_TM_Out *)API_Out;
TxRefund_APDU_In *DongleIn = (TxRefund_APDU_In *)Dongle_In;
TxRefund_APDU_Out *DongleOut = (TxRefund_APDU_Out *)Dongle_Out;

	inTMOutLen = sizeof(TxRefund_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",inTMOutLen - 7);//Data Len
	vdBuildProcessingCode(inTxnType,0x00,0x00,DongleOut->ucPersonalProfile,APIOut->ucProcessignCode);//Processing Code
	memcpy(APIOut->ucCommandCode,"05",sizeof(APIOut->ucCommandCode));//Command Code
	vdBuildBLCVersion(APIOut->ucBLVersion);
	//fnASCTOBIN(APIOut->ucBLVersion,gsBlackListVersion,5,sizeof(APIOut->ucBLVersion),DECIMAL);//Black List Version
	APIOut->ucMsgType = DongleIn->ucMsgType;//Msg Type
	APIOut->ucSubType = DongleIn->ucSubType;//Sub Type
	memcpy(APIOut->ucDeviceID,DongleOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	APIOut->ucSPID = DongleOut->ucDeviceID[2];//SP ID
	memcpy(APIOut->ucTxnDateTime,DongleIn->ucTxnDateTime2,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	APIOut->ucCardType = DongleOut->ucCardType;//Card Type
	APIOut->ucPersonalProfile = DongleOut->ucPersonalProfile;//Personal Profile
	memcpy(APIOut->ucCardID_4,DongleOut->ucCardID_4,sizeof(APIOut->ucCardID_4));//Card ID 4
	APIOut->ucIssuerCode = DongleOut->ucIssuerCode;//Issuer Code
	memcpy(APIOut->ucTxnAmt_2,DongleOut->ucTxnAmt2_2,sizeof(APIOut->ucTxnAmt_2));//Txn Amt 2
	APIOut->ucLocationID = LOCATION_ID;//Location ID
	APIOut->ucBankCode = DongleOut->ucBankCode;//Bank Code
	memcpy(APIOut->ucLoyaltyCounter,DongleOut->ucLoyaltyCounter,sizeof(APIOut->ucLoyaltyCounter));//Loyalty Counter
	memcpy(APIOut->ucDeposit,DongleOut->ucDeposit,sizeof(APIOut->ucDeposit));//Deposit
	memcpy(APIOut->ucExpiryDate,DongleOut->ucExpiryDate,sizeof(APIOut->ucExpiryDate));//Expiry Date
	APIOut->ucAreaCode = DongleOut->ucAreaCode;//Area Code
	memcpy(APIOut->ucTMLocationID_6,DongleIn->ucTMLocationID_6,sizeof(APIOut->ucTMLocationID_6));//TM Location ID 6
	memcpy(APIOut->ucTMID,DongleIn->ucTMID,sizeof(APIOut->ucTMID));//TM ID
	memcpy(APIOut->ucTMTxnDateTime,DongleOut->ucTMTxnDateTime,sizeof(DongleOut->ucTMTxnDateTime));//TM Date Time
	memcpy(APIOut->ucTMSerialNumber,DongleOut->ucTMSerialNumber,sizeof(DongleOut->ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->ucTMAgentNumber,DongleOut->ucTMAgentNumber,sizeof(DongleOut->ucTMAgentNumber));//TM Agent Number
	memcpy(APIOut->ucEV_2,DongleOut->ucEV2_2,sizeof(APIOut->ucEV_2));//EV 2
	memcpy(APIOut->ucSTAC,DongleOut->ucSTAC,sizeof(APIOut->ucSTAC));//S-TAC
	APIOut->ucKeyVersion = DongleOut->ucSAMKeyVersion;//Key Version
	memcpy(APIOut->ucSAMID,DongleOut->ucSAMID,sizeof(APIOut->ucSAMID));//SAM ID
	memcpy(APIOut->ucSAMSN,DongleOut->ucSAMSN,sizeof(APIOut->ucSAMSN));//SAM SN
	memcpy(APIOut->ucSAMCRN,DongleOut->ucSAMCRN,sizeof(APIOut->ucSAMCRN));//SAM CRN
	memcpy(APIOut->ucReaderFWVersion,DongleOut->ucReaderFWVersion,sizeof(APIOut->ucReaderFWVersion));//Reader Firmward Version
	APIOut->ucCAVRMsgType = 0x0C;//CAVR Msg Type
	APIOut->ucCAVRTxnSNLSB = DongleOut->ucCAVRTxnSNLSB;//CAVR Txn SN LSB
	memcpy(APIOut->ucCAVRTxnDateTime,DongleOut->ucCAVRTxnDateTime,sizeof(APIOut->ucCAVRTxnDateTime));//CAVR Txn Date Time
	APIOut->ucCAVRSubType = DongleOut->ucCAVRSubType;//CAVR Sub Type
	memcpy(APIOut->ucCAVRTxnAmt,DongleOut->ucCAVRTxnAmt,sizeof(APIOut->ucCAVRTxnAmt));//CAVR Txn Amt
	memcpy(APIOut->ucCAVREV,DongleOut->ucCAVREV,sizeof(APIOut->ucCAVREV));//CAVR EV
	memcpy(APIOut->ucCAVRCardID,DongleOut->ucCardID_4,sizeof(APIOut->ucCAVRCardID));//CAVR Card ID
	APIOut->ucCAVRIssuerCode = DongleOut->ucIssuerCode;//CAVR Issuer Code
	APIOut->ucCAVRSPID = DongleOut->ucCAVRSPID;//CAVR SP Code
	APIOut->ucCAVRLocationID = DongleOut->ucCAVRLocationID;//CAVR Location ID
	memcpy(APIOut->ucCAVRDeviceID,DongleOut->ucCAVRDeviceID,sizeof(APIOut->ucCAVRDeviceID));//CAVR Device ID
	APIOut->ucCAVRBankCode = DongleOut->ucBankCode;//CAVR Bank Code
	memcpy(APIOut->ucCAVRLoyaltyCounter,DongleOut->ucLoyaltyCounter,sizeof(APIOut->ucCAVRLoyaltyCounter));//CAVR Loyalty Counter
	memcpy(&APIOut->stReaderAVR_t,&DongleOut->stReaderAVR_t,sizeof(APIOut->stReaderAVR_t));//Reader AVR

	return(inTMOutLen);
}

int inBuildAuthTxRefundData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
AuthTxRefund_TM_Out *APIOut = (AuthTxRefund_TM_Out *)API_Out;
AuthTxRefund_APDU_In *DongleIn = (AuthTxRefund_APDU_In *)Dongle_In;
AuthTxRefund_APDU_Out *DongleOut = (AuthTxRefund_APDU_Out *)Dongle_Out;
TxRefund_APDU_Out *TxnReqOut = (TxRefund_APDU_Out *)Dongle_ReqOut;

	inTMOutLen = sizeof(AuthTxRefund_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	//餘退SETM
	vdUIntToAsc(TxnReqOut->ucCardID_4,sizeof(TxnReqOut->ucCardID_4),APIOut->stTxRefundSETM_t.anCardID_10,sizeof(APIOut->stTxRefundSETM_t.anCardID_10),FALSE,' ',10);//n_Card Physical ID
	fnBINTODEVASC(TxnReqOut->ucDeviceID,APIOut->stTxRefundSETM_t.anDeviceID,sizeof(APIOut->stTxRefundSETM_t.anDeviceID),MIFARE);//n_TXN Device ID
	vdIntToAsc(TxnReqOut->ucEV2_2,sizeof(TxnReqOut->ucEV2_2),APIOut->stTxRefundSETM_t.anEVBeforeTxn_5,sizeof(APIOut->stTxRefundSETM_t.anEVBeforeTxn_5),FALSE,' ',10);//n_EV Before TXN
	vdUIntToAsc(DongleOut->ucTxnSN2_2,sizeof(DongleOut->ucTxnSN2_2),APIOut->stTxRefundSETM_t.anTxnSN_5,sizeof(APIOut->stTxRefundSETM_t.anTxnSN_5),FALSE,' ',10);//n_TXN SNum
	vdUIntToAsc(TxnReqOut->ucTxnAmt2_2,sizeof(TxnReqOut->ucTxnAmt2_2),APIOut->stTxRefundSETM_t.anTxnAmt_5,sizeof(APIOut->stTxRefundSETM_t.anTxnAmt_5),FALSE,' ',10);//n_TXN AMT
	vdIntToAsc(DongleOut->ucEV2_2,sizeof(DongleOut->ucEV2_2),APIOut->stTxRefundSETM_t.anEV_5,sizeof(APIOut->stTxRefundSETM_t.anEV_5),FALSE,' ',10);//n_EV
	vdBuildProcessingCode(inTxnType,TxnReqOut->ucMsgType,TxnReqOut->ucSubType,TxnReqOut->ucPersonalProfile,APIOut->stTxRefundSETM_t.anProcessignCode);//Processing Code
	fnUnPack(&TxnReqOut->ucMsgType,sizeof(TxnReqOut->ucMsgType),APIOut->stTxRefundSETM_t.unMsgType);//Msg Type
	fnUnPack(&TxnReqOut->ucSubType,sizeof(TxnReqOut->ucSubType),APIOut->stTxRefundSETM_t.unSubType);//Sub Type
	fnUnPack(TxnReqOut->ucDeviceID,sizeof(TxnReqOut->ucDeviceID),APIOut->stTxRefundSETM_t.unDeviceID);//Device ID
	fnUnPack(&TxnReqOut->ucDeviceID[2],sizeof(APIOut->stTxRefundSETM_t.unSPID) / 2,APIOut->stTxRefundSETM_t.unSPID);//SP ID
	fnUnPack(stStore_DataInfo_t.ucTxnDateTime2,sizeof(stStore_DataInfo_t.ucTxnDateTime2),APIOut->stTxRefundSETM_t.unTxnDateTime);//Txn Date Time
	fnUnPack(&TxnReqOut->ucCardType,sizeof(TxnReqOut->ucCardType),APIOut->stTxRefundSETM_t.unCardType);//Card Type
	fnUnPack(&TxnReqOut->ucPersonalProfile,sizeof(TxnReqOut->ucPersonalProfile),APIOut->stTxRefundSETM_t.unPersonalProfile);//Personal Profile
	fnUnPack(TxnReqOut->ucCardID_4,sizeof(TxnReqOut->ucCardID_4),APIOut->stTxRefundSETM_t.unCardID_8);//CPU Card ID
	fnUnPack(&TxnReqOut->ucIssuerCode,sizeof(TxnReqOut->ucIssuerCode),APIOut->stTxRefundSETM_t.unIssuerCode);//Issuer Code
	fnUnPack(DongleOut->ucTxnSN2_2,sizeof(DongleOut->ucTxnSN2_2),APIOut->stTxRefundSETM_t.unTxnSN_4);//Txn SN
	fnUnPack(TxnReqOut->ucTxnAmt2_2,sizeof(TxnReqOut->ucTxnAmt2_2),APIOut->stTxRefundSETM_t.unTxnAmt_4);//Txn Amt
	fnUnPack(DongleOut->ucEV2_2,sizeof(DongleOut->ucEV2_2),APIOut->stTxRefundSETM_t.unEV_4);//EV
	memcpy(APIOut->stTxRefundSETM_t.unLocationID,LOCATION_ID_BATCH,sizeof(APIOut->stTxRefundSETM_t.unLocationID));//Location ID
	fnUnPack(&TxnReqOut->ucBankCode,sizeof(TxnReqOut->ucBankCode),APIOut->stTxRefundSETM_t.unBankCode);//Bank Code
	fnUnPack(TxnReqOut->ucLoyaltyCounter,sizeof(TxnReqOut->ucLoyaltyCounter),APIOut->stTxRefundSETM_t.unLoyaltyCounter);//Loyalty Counter
	fnUnPack(TxnReqOut->ucDeposit,sizeof(TxnReqOut->ucDeposit),APIOut->stTxRefundSETM_t.unDeposit_4);//Deposit
	fnUnPack(DongleOut->ucMAC2,sizeof(DongleOut->ucMAC2),APIOut->stTxRefundSETM_t.unMAC);//MAC
	fnUnPack(TxnReqOut->ucExpiryDate,sizeof(TxnReqOut->ucExpiryDate),APIOut->stTxRefundSETM_t.anOrgExpiryDate);//Org Expiry Date
	fnUnPack(DongleIn->ucRFU4,sizeof(DongleIn->ucRFU4),APIOut->stTxRefundSETM_t.anNewExpiryDate);//New Expiry Date
	fnUnPack(&TxnReqOut->ucAreaCode,sizeof(TxnReqOut->ucAreaCode),APIOut->stTxRefundSETM_t.unAreaCode);//Area Code
	fnUnPack(TxnReqOut->ucEV2_2,sizeof(TxnReqOut->ucEV2_2),APIOut->stTxRefundSETM_t.unEVBeforeTxn_4);//EV Before Txn
	//餘退電文
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",53);//Data Len
	vdBuildProcessingCode(inTxnType,0x00,0x00,0x00,APIOut->ucProcessignCode);//Processing Code
	memcpy(APIOut->ucCommandCode,"05",sizeof(APIOut->ucCommandCode));//Command Code
	APIOut->ucMsgType = TxnReqOut->ucMsgType;//Msg Type
	APIOut->ucSubType = TxnReqOut->ucSubType;//Sub Type
	memcpy(APIOut->ucDeviceID,TxnReqOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	memcpy(APIOut->ucTxnDateTime,stStore_DataInfo_t.ucTxnDateTime2,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	memcpy(APIOut->ucCardID_4,TxnReqOut->ucCardID_4,sizeof(APIOut->ucCardID_4));//Card ID
	memcpy(APIOut->ucTxnSN_2,DongleOut->ucTxnSN2_2,sizeof(APIOut->ucTxnSN_2));//Txn SN
	memcpy(APIOut->ucEV_2,DongleOut->ucEV2_2,sizeof(APIOut->ucEV_2));//EV
	memcpy(APIOut->ucMAC,DongleOut->ucMAC2,sizeof(APIOut->ucMAC));//MAC
	memcpy(APIOut->ucConfirmCode,DongleOut->ucConfirmCode2,sizeof(APIOut->ucConfirmCode));//Confirm Code
	memcpy(APIOut->ucCTAC,DongleOut->ucCTAC,sizeof(APIOut->ucCTAC));//CTAC
	//餘扣SETM
	vdUIntToAsc(TxnReqOut->ucCardID_4,sizeof(TxnReqOut->ucCardID_4),APIOut->stTxDeductSETM_t.anCardID_10,sizeof(APIOut->stTxDeductSETM_t.anCardID_10),FALSE,' ',10);//n_Card Physical ID
	fnBINTODEVASC(TxnReqOut->ucDeviceID,APIOut->stTxDeductSETM_t.anDeviceID,sizeof(APIOut->stTxDeductSETM_t.anDeviceID),MIFARE);//n_TXN Device ID
	vdIntToAsc(TxnReqOut->ucEV_2,sizeof(TxnReqOut->ucEV_2),APIOut->stTxDeductSETM_t.anEVBeforeTxn_5,sizeof(APIOut->stTxDeductSETM_t.anEVBeforeTxn_5),FALSE,' ',10);//n_EV Before TXN
	vdUIntToAsc(DongleOut->ucTxnSN_2,sizeof(DongleOut->ucTxnSN_2),APIOut->stTxDeductSETM_t.anTxnSN_5,sizeof(APIOut->stTxDeductSETM_t.anTxnSN_5),FALSE,' ',10);//n_TXN SNum
	vdUIntToAsc(TxnReqOut->ucTxnAmt_2,sizeof(TxnReqOut->ucTxnAmt_2),APIOut->stTxDeductSETM_t.anTxnAmt_5,sizeof(APIOut->stTxDeductSETM_t.anTxnAmt_5),FALSE,' ',10);//n_TXN AMT
	vdIntToAsc(DongleOut->ucEV_2,sizeof(DongleOut->ucEV_2),APIOut->stTxDeductSETM_t.anEV_5,sizeof(APIOut->stTxDeductSETM_t.anEV_5),FALSE,' ',10);//n_EV
	memcpy(APIOut->stTxDeductSETM_t.anProcessignCode,"812599",sizeof(APIOut->stTxDeductSETM_t.anProcessignCode));//Processign Code
	memcpy(APIOut->stTxDeductSETM_t.unMsgType,"01",sizeof(APIOut->stTxDeductSETM_t.unMsgType));//Msg Type
	memcpy(APIOut->stTxDeductSETM_t.unSubType,"00",sizeof(APIOut->stTxDeductSETM_t.unSubType));//Sub Type
	fnUnPack(TxnReqOut->ucDeviceID,sizeof(TxnReqOut->ucDeviceID),APIOut->stTxDeductSETM_t.unDeviceID);//Device ID
	fnUnPack(&TxnReqOut->ucDeviceID[2],sizeof(APIOut->stTxDeductSETM_t.unSPID) / 2,APIOut->stTxDeductSETM_t.unSPID);//SP ID
	fnUnPack(stStore_DataInfo_t.ucTxnDateTime,sizeof(stStore_DataInfo_t.ucTxnDateTime),APIOut->stTxDeductSETM_t.unTxnDateTime);//Txn Date Time
	fnUnPack(&TxnReqOut->ucCardType,sizeof(TxnReqOut->ucCardType),APIOut->stTxDeductSETM_t.unCardType);//Card Type
	fnUnPack(&TxnReqOut->ucPersonalProfile,sizeof(TxnReqOut->ucPersonalProfile),APIOut->stTxDeductSETM_t.unPersonalProfile);//Personal Profile
	fnUnPack(TxnReqOut->ucCardID_4,sizeof(TxnReqOut->ucCardID_4),APIOut->stTxDeductSETM_t.unCardID_8);//CPU Card ID
	fnUnPack(&TxnReqOut->ucIssuerCode,sizeof(TxnReqOut->ucIssuerCode),APIOut->stTxDeductSETM_t.unIssuerCode);//Issuer Code
	fnUnPack(DongleOut->ucTxnSN_2,sizeof(DongleOut->ucTxnSN_2),APIOut->stTxDeductSETM_t.unTxnSN_4);//Txn SN
	fnUnPack(TxnReqOut->ucTxnAmt_2,sizeof(TxnReqOut->ucTxnAmt_2),APIOut->stTxDeductSETM_t.unTxnAmt_4);//Txn Amt
	fnUnPack(DongleOut->ucEV_2,sizeof(DongleOut->ucEV_2),APIOut->stTxDeductSETM_t.unEV_4);//EV
	memcpy(APIOut->stTxDeductSETM_t.unLocationID,LOCATION_ID_BATCH,sizeof(APIOut->stTxDeductSETM_t.unLocationID));//Location ID
	fnUnPack(&TxnReqOut->ucBankCode,sizeof(TxnReqOut->ucBankCode),APIOut->stTxDeductSETM_t.unBankCode);//Bank Code
	fnUnPack(TxnReqOut->ucLoyaltyCounter,sizeof(TxnReqOut->ucLoyaltyCounter),APIOut->stTxDeductSETM_t.unLoyaltyCounter);//Loyalty Counter
	fnUnPack(TxnReqOut->ucDeposit,sizeof(TxnReqOut->ucDeposit),APIOut->stTxDeductSETM_t.unDeposit_4);//Deposit
	fnUnPack(DongleOut->ucMAC,sizeof(DongleOut->ucMAC),APIOut->stTxDeductSETM_t.unMAC);//MAC
	memcpy(APIOut->stTxDeductSETM_t.anOrgExpiryDate,"00000000",sizeof(APIOut->stTxDeductSETM_t.anOrgExpiryDate));//Org Expiry Date
	memcpy(APIOut->stTxDeductSETM_t.anNewExpiryDate,"00000000",sizeof(APIOut->stTxDeductSETM_t.anNewExpiryDate));//New Expiry Date
	fnUnPack(&TxnReqOut->ucAreaCode,sizeof(TxnReqOut->ucAreaCode),APIOut->stTxDeductSETM_t.unAreaCode);//Area Code
	fnUnPack(TxnReqOut->ucEV_2,sizeof(TxnReqOut->ucEV_2),APIOut->stTxDeductSETM_t.unEVBeforeTxn_4);//EV Before Txn
	memcpy(APIOut->ucTMLocationID_6,TxnReqOut->ucTMLocationID_6,sizeof(APIOut->ucTMLocationID_6));//TM Location ID
	memcpy(APIOut->ucTMID,TxnReqOut->ucTMID,sizeof(APIOut->ucTMID));//TM ID
	memcpy(APIOut->ucTMTxnDateTime,TxnReqOut->ucTMTxnDateTime,sizeof(APIOut->ucTMTxnDateTime));//TM Txn Date Time
	memcpy(APIOut->ucTMSerialNumber,TxnReqOut->ucTMSerialNumber,sizeof(APIOut->ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->ucTMAgentNumber,TxnReqOut->ucTMAgentNumber,sizeof(APIOut->ucTMAgentNumber));//TM Agent Number
	//餘退加值重送
	fnUnPack(&TxnReqOut->ucDeviceID[2],sizeof(TxnReqOut->ucDeviceID[2]),APIOut->unVARSPID);//VAR SP ID
	memcpy(APIOut->unVARLocationID,LOCATION_ID_BATCH,2);//VAR Location ID
	memcpy(APIOut->unVARMsgType,"0C",2);//VAR Msg Type
	fnUnPack(&TxnReqOut->ucCAVRTxnSNLSB,sizeof(TxnReqOut->ucCAVRTxnSNLSB),APIOut->unVARTxnSNLSB);//VAR Txn SN LSB
	fnUnPack(TxnReqOut->ucCAVRTxnDateTime,sizeof(TxnReqOut->ucCAVRTxnDateTime),APIOut->unVARTxnDateTime);//VAR Txn Date Time
	fnUnPack(&TxnReqOut->ucCAVRSubType,sizeof(TxnReqOut->ucCAVRSubType),APIOut->unVARSubType);//VAR Sub Type
	fnUnPack(TxnReqOut->ucCAVRTxnAmt,sizeof(TxnReqOut->ucCAVRTxnAmt),APIOut->unVARTxnAmt_4);//VAR Txn Amt
	fnUnPack(TxnReqOut->ucCAVREV,sizeof(TxnReqOut->ucCAVREV),APIOut->unVAREV_4);//VAR EV
	fnUnPack(TxnReqOut->ucCardID_4,sizeof(TxnReqOut->ucCardID_4),APIOut->unVARCardID_8);//VAR Card ID
	fnUnPack(&TxnReqOut->ucIssuerCode,sizeof(TxnReqOut->ucIssuerCode),APIOut->unVARIssuerCode);//VAR Issuer Code
	fnUnPack(&TxnReqOut->ucCAVRSPID,sizeof(TxnReqOut->ucCAVRSPID),APIOut->unVAR2SPID);//VAR SP ID
	fnUnPack(&TxnReqOut->ucCAVRLocationID,sizeof(TxnReqOut->ucCAVRLocationID),APIOut->unVAR2LocationID);//VAR Location ID
	fnUnPack(TxnReqOut->ucCAVRDeviceID,sizeof(TxnReqOut->ucCAVRDeviceID),APIOut->unVAR2DeviceID);//VAR Device ID
	fnUnPack(&TxnReqOut->ucBankCode,sizeof(TxnReqOut->ucBankCode),APIOut->unVARBankCode);//VAR Bank Code
	fnUnPack(TxnReqOut->ucLoyaltyCounter,sizeof(TxnReqOut->ucLoyaltyCounter),APIOut->unVARLoyaltyCounter);//VAR Loyalty Counter

	return(inTMOutLen);
}

int inBuildAddValueData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
AddValue_TM_Out *APIOut = (AddValue_TM_Out *)API_Out;
AddValue_APDU_In *DongleIn = (AddValue_APDU_In *)Dongle_In;
AddValue_APDU_Out *DongleOut = (AddValue_APDU_Out *)Dongle_Out;

	inTMOutLen = sizeof(AddValue_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",inTMOutLen - 7);//Data Len
	vdBuildProcessingCode(inTxnType,0x00,0x00,DongleOut->ucPersonalProfile,APIOut->ucProcessignCode);//Processing Code
	memcpy(APIOut->ucCommandCode,"05",sizeof(APIOut->ucCommandCode));//Command Code
	vdBuildBLCVersion(APIOut->ucBLVersion);
	//fnASCTOBIN(APIOut->ucBLVersion,gsBlackListVersion,5,sizeof(APIOut->ucBLVersion),DECIMAL);//Black List Version
	APIOut->ucMsgType = DongleIn->ucMsgType;//Msg Type
	APIOut->ucSubType = DongleIn->ucSubType;//Sub Type
	memcpy(APIOut->ucDeviceID,DongleOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	APIOut->ucSPID = DongleOut->ucDeviceID[2];//SP ID
	memcpy(APIOut->ucTxnDateTime,DongleIn->ucTxnDateTime,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	APIOut->ucCardType = DongleOut->ucCardType;//Card Type
	APIOut->ucPersonalProfile = DongleOut->ucPersonalProfile;//Personal Profile
	memcpy(APIOut->ucCardID_4,DongleOut->ucCardID_4,sizeof(APIOut->ucCardID_4));//Card ID 4
	APIOut->ucIssuerCode = DongleOut->ucIssuerCode;//Issuer Code
	memcpy(APIOut->ucTxnAmt_2,DongleOut->ucTxnAmt_2,sizeof(APIOut->ucTxnAmt_2));//Txn Amt 2
	APIOut->ucLocationID = LOCATION_ID;//Location ID
	APIOut->ucBankCode = DongleOut->ucBankCode;//Bank Code
	memcpy(APIOut->ucLoyaltyCounter,DongleOut->ucLoyaltyCounter,sizeof(APIOut->ucLoyaltyCounter));//Loyalty Counter
	memcpy(APIOut->ucExpiryDate,DongleOut->ucExpiryDate,sizeof(APIOut->ucExpiryDate));//Expiry Date
	APIOut->ucAreaCode = DongleOut->ucAreaCode;//Area Code
	memcpy(APIOut->ucTMLocationID_6,DongleIn->ucTMLocationID_6,sizeof(APIOut->ucTMLocationID_6));//TM Location ID 6
	memcpy(APIOut->ucTMID,DongleIn->ucTMID,sizeof(APIOut->ucTMID));//TM ID
	memcpy(APIOut->ucTMTxnDateTime,DongleOut->ucTMTxnDateTime,sizeof(DongleOut->ucTMTxnDateTime));//TM Date Time
	memcpy(APIOut->ucTMSerialNumber,DongleOut->ucTMSerialNumber,sizeof(DongleOut->ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->ucTMAgentNumber,DongleOut->ucTMAgentNumber,sizeof(DongleOut->ucTMAgentNumber));//TM Agent Number
	memcpy(APIOut->ucEV_2,DongleOut->ucEV_2,sizeof(APIOut->ucEV_2));//EV 2
	memcpy(APIOut->ucSTAC,DongleOut->ucSTAC,sizeof(APIOut->ucSTAC));//S-TAC
	APIOut->ucKeyVersion = DongleOut->ucSAMKeyVersion;//Key Version
	memcpy(APIOut->ucSAMID,DongleOut->ucSAMID,sizeof(APIOut->ucSAMID));//SAM ID
	memcpy(APIOut->ucSAMSN,DongleOut->ucSAMSN,sizeof(APIOut->ucSAMSN));//SAM SN
	memcpy(APIOut->ucSAMCRN,DongleOut->ucSAMCRN,sizeof(APIOut->ucSAMCRN));//SAM CRN
	memcpy(APIOut->ucReaderFWVersion,DongleOut->ucReaderFWVersion,sizeof(APIOut->ucReaderFWVersion));//Reader Firmward Version
	APIOut->ucCAVRMsgType = 0x0C;//CAVR Msg Type
	APIOut->ucCAVRTxnSNLSB = DongleOut->ucCAVRTxnSNLSB;//CAVR Txn SN LSB
	memcpy(APIOut->ucCAVRTxnDateTime,DongleOut->ucCAVRTxnDateTime,sizeof(APIOut->ucCAVRTxnDateTime));//CAVR Txn Date Time
	APIOut->ucCAVRSubType = DongleOut->ucCAVRSubType;//CAVR Sub Type
	memcpy(APIOut->ucCAVRTxnAmt,DongleOut->ucCAVRTxnAmt,sizeof(APIOut->ucCAVRTxnAmt));//CAVR Txn Amt
	memcpy(APIOut->ucCAVREV,DongleOut->ucCAVREV,sizeof(APIOut->ucCAVREV));//CAVR EV
	memcpy(APIOut->ucCAVRCardID,DongleOut->ucCardID_4,sizeof(APIOut->ucCAVRCardID));//CAVR Card ID
	APIOut->ucCAVRIssuerCode = DongleOut->ucIssuerCode;//CAVR Issuer Code
	APIOut->ucCAVRSPID = DongleOut->ucCAVRSPID;//CAVR SP Code
	APIOut->ucCAVRLocationID = DongleOut->ucCAVRLocationID;//CAVR Location ID
	memcpy(APIOut->ucCAVRDeviceID,DongleOut->ucCAVRDeviceID,sizeof(APIOut->ucCAVRDeviceID));//CAVR Device ID
	APIOut->ucCAVRBankCode = DongleOut->ucBankCode;//CAVR Bank Code
	memcpy(APIOut->ucCAVRLoyaltyCounter,DongleOut->ucLoyaltyCounter,sizeof(APIOut->ucCAVRLoyaltyCounter));//CAVR Loyalty Counter
	memcpy(&APIOut->stReaderAVR_t,&DongleOut->stReaderAVR_t,sizeof(APIOut->stReaderAVR_t));//Reader AVR

	return(inTMOutLen);
}

int inBuildAuthAddValueData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
AuthAddValue_TM_Out *APIOut = (AuthAddValue_TM_Out *)API_Out;
AuthAddValue_APDU_In *DongleIn = (AuthAddValue_APDU_In *)Dongle_In;
AuthAddValue_APDU_Out *DongleOut = (AuthAddValue_APDU_Out *)Dongle_Out;
AddValue_APDU_Out *TxnReqOut = (AddValue_APDU_Out *)Dongle_ReqOut;

	inTMOutLen = sizeof(AuthAddValue_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	//餘加SETM
	vdUIntToAsc(TxnReqOut->ucCardID_4,sizeof(TxnReqOut->ucCardID_4),APIOut->stAddValueSETM_t.anCardID_10,sizeof(APIOut->stAddValueSETM_t.anCardID_10),FALSE,' ',10);//n_Card Physical ID
	fnBINTODEVASC(TxnReqOut->ucDeviceID,APIOut->stAddValueSETM_t.anDeviceID,sizeof(APIOut->stAddValueSETM_t.anDeviceID),MIFARE);//n_TXN Device ID
	vdIntToAsc(TxnReqOut->ucEV_2,sizeof(TxnReqOut->ucEV_2),APIOut->stAddValueSETM_t.anEVBeforeTxn_5,sizeof(APIOut->stAddValueSETM_t.anEVBeforeTxn_5),FALSE,' ',10);//n_EV Before TXN
	vdUIntToAsc(DongleOut->ucTxnSN_2,sizeof(DongleOut->ucTxnSN_2),APIOut->stAddValueSETM_t.anTxnSN_5,sizeof(APIOut->stAddValueSETM_t.anTxnSN_5),FALSE,' ',10);//n_TXN SNum
	vdUIntToAsc(TxnReqOut->ucTxnAmt_2,sizeof(TxnReqOut->ucTxnAmt_2),APIOut->stAddValueSETM_t.anTxnAmt_5,sizeof(APIOut->stAddValueSETM_t.anTxnAmt_5),FALSE,' ',10);//n_TXN AMT
	vdIntToAsc(DongleOut->ucEV_2,sizeof(DongleOut->ucEV_2),APIOut->stAddValueSETM_t.anEV_5,sizeof(APIOut->stAddValueSETM_t.anEV_5),FALSE,' ',10);//n_EV
	vdBuildProcessingCode(inTxnType,TxnReqOut->ucMsgType,TxnReqOut->ucSubType,TxnReqOut->ucPersonalProfile,APIOut->stAddValueSETM_t.anProcessignCode);//Processing Code
	fnUnPack(&TxnReqOut->ucMsgType,sizeof(TxnReqOut->ucMsgType),APIOut->stAddValueSETM_t.unMsgType);//Msg Type
	fnUnPack(&TxnReqOut->ucSubType,sizeof(TxnReqOut->ucSubType),APIOut->stAddValueSETM_t.unSubType);//Sub Type
	fnUnPack(TxnReqOut->ucDeviceID,sizeof(TxnReqOut->ucDeviceID),APIOut->stAddValueSETM_t.unDeviceID);//Device ID
	fnUnPack(&TxnReqOut->ucDeviceID[2],sizeof(APIOut->stAddValueSETM_t.unSPID) / 2,APIOut->stAddValueSETM_t.unSPID);//SP ID
	fnUnPack(stStore_DataInfo_t.ucTxnDateTime,sizeof(stStore_DataInfo_t.ucTxnDateTime),APIOut->stAddValueSETM_t.unTxnDateTime);//Txn Date Time
	fnUnPack(&TxnReqOut->ucCardType,sizeof(TxnReqOut->ucCardType),APIOut->stAddValueSETM_t.unCardType);//Card Type
	fnUnPack(&TxnReqOut->ucPersonalProfile,sizeof(TxnReqOut->ucPersonalProfile),APIOut->stAddValueSETM_t.unPersonalProfile);//Personal Profile
	fnUnPack(TxnReqOut->ucCardID_4,sizeof(TxnReqOut->ucCardID_4),APIOut->stAddValueSETM_t.unCardID_8);//CPU Card ID
	fnUnPack(&TxnReqOut->ucIssuerCode,sizeof(TxnReqOut->ucIssuerCode),APIOut->stAddValueSETM_t.unIssuerCode);//Issuer Code
	fnUnPack(DongleOut->ucTxnSN_2,sizeof(DongleOut->ucTxnSN_2),APIOut->stAddValueSETM_t.unTxnSN_4);//Txn SN
	fnUnPack(TxnReqOut->ucTxnAmt_2,sizeof(TxnReqOut->ucTxnAmt_2),APIOut->stAddValueSETM_t.unTxnAmt_4);//Txn Amt
	fnUnPack(DongleOut->ucEV_2,sizeof(DongleOut->ucEV_2),APIOut->stAddValueSETM_t.unEV_4);//EV
	memcpy(APIOut->stAddValueSETM_t.unLocationID,LOCATION_ID_BATCH,sizeof(APIOut->stAddValueSETM_t.unLocationID));//Location ID
	fnUnPack(&TxnReqOut->ucBankCode,sizeof(TxnReqOut->ucBankCode),APIOut->stAddValueSETM_t.unBankCode);//Bank Code
	fnUnPack(TxnReqOut->ucLoyaltyCounter,sizeof(TxnReqOut->ucLoyaltyCounter),APIOut->stAddValueSETM_t.unLoyaltyCounter);//Loyalty Counter
	fnUnPack(TxnReqOut->ucDeposit,sizeof(TxnReqOut->ucDeposit),APIOut->stAddValueSETM_t.unDeposit_4);//Deposit
	fnUnPack(DongleOut->ucMAC,sizeof(DongleOut->ucMAC),APIOut->stAddValueSETM_t.unMAC);//MAC
	fnUnPack(TxnReqOut->ucExpiryDate,sizeof(TxnReqOut->ucExpiryDate),APIOut->stAddValueSETM_t.anOrgExpiryDate);//Org Expiry Date
	fnUnPack(DongleIn->ucExpiryDate,sizeof(DongleIn->ucExpiryDate),APIOut->stAddValueSETM_t.anNewExpiryDate);//New Expiry Date
	fnUnPack(&TxnReqOut->ucAreaCode,sizeof(TxnReqOut->ucAreaCode),APIOut->stAddValueSETM_t.unAreaCode);//Area Code
	fnUnPack(TxnReqOut->ucEV_2,sizeof(TxnReqOut->ucEV_2),APIOut->stAddValueSETM_t.unEVBeforeTxn_4);//EV Before Txn
	//餘加電文
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",53);//Data Len
	vdBuildProcessingCode(inTxnType,0x00,0x00,0x00,APIOut->ucProcessignCode);//Processing Code
	memcpy(APIOut->ucCommandCode,"05",sizeof(APIOut->ucCommandCode));//Command Code
	APIOut->ucMsgType = TxnReqOut->ucMsgType;//Msg Type
	APIOut->ucSubType = TxnReqOut->ucSubType;//Sub Type
	memcpy(APIOut->ucDeviceID,TxnReqOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	memcpy(APIOut->ucTxnDateTime,stStore_DataInfo_t.ucTxnDateTime,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	memcpy(APIOut->ucCardID_4,TxnReqOut->ucCardID_4,sizeof(APIOut->ucCardID_4));//Card ID
	memcpy(APIOut->ucTxnSN_2,DongleOut->ucTxnSN_2,sizeof(APIOut->ucTxnSN_2));//Txn SN
	memcpy(APIOut->ucEV_2,DongleOut->ucEV_2,sizeof(APIOut->ucEV_2));//EV
	memcpy(APIOut->ucMAC,DongleOut->ucMAC,sizeof(APIOut->ucMAC));//MAC
	memcpy(APIOut->ucConfirmCode,DongleOut->ucConfirmCode,sizeof(APIOut->ucConfirmCode));//Confirm Code
	memcpy(APIOut->ucCTAC,DongleOut->ucCTAC,sizeof(APIOut->ucCTAC));//CTAC

	return(inTMOutLen);
}

int inBuildAutoloadEnableData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
AutoloadEnable_TM_Out *APIOut = (AutoloadEnable_TM_Out *)API_Out;
AutoloadEnable_APDU_In *DongleIn = (AutoloadEnable_APDU_In *)Dongle_In;
AutoloadEnable_APDU_Out *DongleOut = (AutoloadEnable_APDU_Out *)Dongle_Out;

	inTMOutLen = sizeof(AutoloadEnable_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",inTMOutLen - 7);//Data Len
	vdBuildProcessingCode(inTxnType,0x00,0x00,DongleOut->ucPersonalProfile,APIOut->ucProcessignCode);//Processing Code
	memcpy(APIOut->ucCommandCode,"05",sizeof(APIOut->ucCommandCode));//Command Code
	vdBuildBLCVersion(APIOut->ucBLVersion);
	//fnASCTOBIN(APIOut->ucBLVersion,gsBlackListVersion,5,sizeof(APIOut->ucBLVersion),DECIMAL);//Black List Version
	APIOut->ucMsgType = DongleIn->ucMsgType;//Msg Type
	APIOut->ucSubType = DongleIn->ucSubType;//Sub Type
	memcpy(APIOut->ucDeviceID,DongleOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	APIOut->ucSPID = DongleOut->ucDeviceID[2];//SP ID
	memcpy(APIOut->ucTxnDateTime,DongleIn->ucTxnDateTime,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	APIOut->ucCardType = DongleOut->ucCardType;//Card Type
	APIOut->ucPersonalProfile = DongleOut->ucPersonalProfile;//Personal Profile
	memcpy(APIOut->ucCardID_4,DongleOut->ucCardID_4,sizeof(APIOut->ucCardID_4));//Card ID 4
	APIOut->ucIssuerCode = DongleOut->ucIssuerCode;//Issuer Code
	memcpy(APIOut->ucTxnAmt_2,DongleOut->ucTxnAmt_2,sizeof(APIOut->ucTxnAmt_2));//Txn Amt 2
	APIOut->ucLocationID = LOCATION_ID;//Location ID
	APIOut->ucBankCode = DongleOut->ucBankCode;//Bank Code
	memcpy(APIOut->ucLoyaltyCounter,DongleOut->ucLoyaltyCounter,sizeof(APIOut->ucLoyaltyCounter));//Loyalty Counter
	memcpy(APIOut->ucExpiryDate,DongleOut->ucExpiryDate,sizeof(APIOut->ucExpiryDate));//Expiry Date
	APIOut->ucAreaCode = DongleOut->ucAreaCode;//Area Code
	memcpy(APIOut->ucTMLocationID_6,DongleIn->ucTMLocationID_6,sizeof(APIOut->ucTMLocationID_6));//TM Location ID 6
	memcpy(APIOut->ucTMID,DongleIn->ucTMID,sizeof(APIOut->ucTMID));//TM ID
	memcpy(APIOut->ucTMTxnDateTime,DongleOut->ucTMTxnDateTime,sizeof(DongleOut->ucTMTxnDateTime));//TM Date Time
	memcpy(APIOut->ucTMSerialNumber,DongleOut->ucTMSerialNumber,sizeof(DongleOut->ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->ucTMAgentNumber,DongleOut->ucTMAgentNumber,sizeof(DongleOut->ucTMAgentNumber));//TM Agent Number
	memcpy(APIOut->ucEV_2,DongleOut->ucEV_2,sizeof(APIOut->ucEV_2));//EV 2
	memcpy(APIOut->ucSTAC,DongleOut->ucSTAC,sizeof(APIOut->ucSTAC));//S-TAC
	APIOut->ucKeyVersion = DongleOut->ucSAMKeyVersion;//Key Version
	memcpy(APIOut->ucSAMID,DongleOut->ucSAMID,sizeof(APIOut->ucSAMID));//SAM ID
	memcpy(APIOut->ucSAMSN,DongleOut->ucSAMSN,sizeof(APIOut->ucSAMSN));//SAM SN
	memcpy(APIOut->ucSAMCRN,DongleOut->ucSAMCRN,sizeof(APIOut->ucSAMCRN));//SAM CRN
	memcpy(APIOut->ucReaderFWVersion,DongleOut->ucReaderFWVersion,sizeof(APIOut->ucReaderFWVersion));//Reader Firmward Version
	APIOut->ucCAVRMsgType = 0x0C;//CAVR Msg Type
	APIOut->ucCAVRTxnSNLSB = DongleOut->ucCAVRTxnSNLSB;//CAVR Txn SN LSB
	memcpy(APIOut->ucCAVRTxnDateTime,DongleOut->ucCAVRTxnDateTime,sizeof(APIOut->ucCAVRTxnDateTime));//CAVR Txn Date Time
	APIOut->ucCAVRSubType = DongleOut->ucCAVRSubType;//CAVR Sub Type
	memcpy(APIOut->ucCAVRTxnAmt,DongleOut->ucCAVRTxnAmt,sizeof(APIOut->ucCAVRTxnAmt));//CAVR Txn Amt
	memcpy(APIOut->ucCAVREV,DongleOut->ucCAVREV,sizeof(APIOut->ucCAVREV));//CAVR EV
	memcpy(APIOut->ucCAVRCardID,DongleOut->ucCardID_4,sizeof(APIOut->ucCAVRCardID));//CAVR Card ID
	APIOut->ucCAVRIssuerCode = DongleOut->ucIssuerCode;//CAVR Issuer Code
	APIOut->ucCAVRSPID = DongleOut->ucCAVRSPID;//CAVR SP Code
	APIOut->ucCAVRLocationID = DongleOut->ucCAVRLocationID;//CAVR Location ID
	memcpy(APIOut->ucCAVRDeviceID,DongleOut->ucCAVRDeviceID,sizeof(APIOut->ucCAVRDeviceID));//CAVR Device ID
	APIOut->ucCAVRBankCode = DongleOut->ucBankCode;//CAVR Bank Code
	memcpy(APIOut->ucCAVRLoyaltyCounter,DongleOut->ucLoyaltyCounter,sizeof(APIOut->ucCAVRLoyaltyCounter));//CAVR Loyalty Counter
	memcpy(&APIOut->stReaderAVR_t,&DongleOut->stReaderAVR_t,sizeof(APIOut->stReaderAVR_t));//Reader AVR
	APIOut->ucAutoLoadFlag = DongleOut->ucAutoLoadFlag;////AutoLoad Flag
	memcpy(APIOut->ucAutoLoadAmt_2,DongleOut->ucAutoLoadAmt_2,sizeof(APIOut->ucAutoLoadAmt_2));//AutoLoad Amt

	return(inTMOutLen);
}

int inBuildAuthAutoloadEnableData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
AuthAutoloadEnable_TM_Out *APIOut = (AuthAutoloadEnable_TM_Out *)API_Out;
//AuthAutoloadEnable_APDU_In *DongleIn = (AuthAutoloadEnable_APDU_In *)Dongle_In;
AuthAutoloadEnable_APDU_Out *DongleOut = (AuthAutoloadEnable_APDU_Out *)Dongle_Out;
AutoloadEnable_APDU_Out *TxnReqOut = (AutoloadEnable_APDU_Out *)Dongle_ReqOut;

	inTMOutLen = sizeof(AuthAutoloadEnable_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	//自動加值開啟SETM
	vdUIntToAsc(TxnReqOut->ucCardID_4,sizeof(TxnReqOut->ucCardID_4),APIOut->stAddValueSETM_t.anCardID_10,sizeof(APIOut->stAddValueSETM_t.anCardID_10),FALSE,' ',10);//n_Card Physical ID
	fnBINTODEVASC(TxnReqOut->ucDeviceID,APIOut->stAddValueSETM_t.anDeviceID,sizeof(APIOut->stAddValueSETM_t.anDeviceID),MIFARE);//n_TXN Device ID
	vdIntToAsc(TxnReqOut->ucEV_2,sizeof(TxnReqOut->ucEV_2),APIOut->stAddValueSETM_t.anEVBeforeTxn_5,sizeof(APIOut->stAddValueSETM_t.anEVBeforeTxn_5),FALSE,' ',10);//n_EV Before TXN
	vdUIntToAsc(DongleOut->ucTxnSN_2,sizeof(DongleOut->ucTxnSN_2),APIOut->stAddValueSETM_t.anTxnSN_5,sizeof(APIOut->stAddValueSETM_t.anTxnSN_5),FALSE,' ',10);//n_TXN SNum
	vdUIntToAsc(TxnReqOut->ucTxnAmt_2,sizeof(TxnReqOut->ucTxnAmt_2),APIOut->stAddValueSETM_t.anTxnAmt_5,sizeof(APIOut->stAddValueSETM_t.anTxnAmt_5),FALSE,' ',10);//n_TXN AMT
	vdIntToAsc(DongleOut->ucEV_2,sizeof(DongleOut->ucEV_2),APIOut->stAddValueSETM_t.anEV_5,sizeof(APIOut->stAddValueSETM_t.anEV_5),FALSE,' ',10);//n_EV
	vdBuildProcessingCode(inTxnType,TxnReqOut->ucMsgType,TxnReqOut->ucSubType,TxnReqOut->ucPersonalProfile,APIOut->stAddValueSETM_t.anProcessignCode);//Processing Code
	fnUnPack(&TxnReqOut->ucMsgType,sizeof(TxnReqOut->ucMsgType),APIOut->stAddValueSETM_t.unMsgType);//Msg Type
	fnUnPack(&TxnReqOut->ucSubType,sizeof(TxnReqOut->ucSubType),APIOut->stAddValueSETM_t.unSubType);//Sub Type
	fnUnPack(TxnReqOut->ucDeviceID,sizeof(TxnReqOut->ucDeviceID),APIOut->stAddValueSETM_t.unDeviceID);//Device ID
	fnUnPack(&TxnReqOut->ucDeviceID[2],sizeof(APIOut->stAddValueSETM_t.unSPID) / 2,APIOut->stAddValueSETM_t.unSPID);//SP ID
	fnUnPack(stStore_DataInfo_t.ucTxnDateTime,sizeof(stStore_DataInfo_t.ucTxnDateTime),APIOut->stAddValueSETM_t.unTxnDateTime);//Txn Date Time
	fnUnPack(&TxnReqOut->ucCardType,sizeof(TxnReqOut->ucCardType),APIOut->stAddValueSETM_t.unCardType);//Card Type
	fnUnPack(&TxnReqOut->ucPersonalProfile,sizeof(TxnReqOut->ucPersonalProfile),APIOut->stAddValueSETM_t.unPersonalProfile);//Personal Profile
	fnUnPack(TxnReqOut->ucCardID_4,sizeof(TxnReqOut->ucCardID_4),APIOut->stAddValueSETM_t.unCardID_8);//CPU Card ID
	fnUnPack(&TxnReqOut->ucIssuerCode,sizeof(TxnReqOut->ucIssuerCode),APIOut->stAddValueSETM_t.unIssuerCode);//Issuer Code
	fnUnPack(DongleOut->ucTxnSN_2,sizeof(DongleOut->ucTxnSN_2),APIOut->stAddValueSETM_t.unTxnSN_4);//Txn SN
	fnUnPack(TxnReqOut->ucTxnAmt_2,sizeof(TxnReqOut->ucTxnAmt_2),APIOut->stAddValueSETM_t.unTxnAmt_4);//Txn Amt
	fnUnPack(DongleOut->ucEV_2,sizeof(DongleOut->ucEV_2),APIOut->stAddValueSETM_t.unEV_4);//EV
	memcpy(APIOut->stAddValueSETM_t.unLocationID,LOCATION_ID_BATCH,sizeof(APIOut->stAddValueSETM_t.unLocationID));//Location ID
	fnUnPack(&TxnReqOut->ucBankCode,sizeof(TxnReqOut->ucBankCode),APIOut->stAddValueSETM_t.unBankCode);//Bank Code
	fnUnPack(TxnReqOut->ucLoyaltyCounter,sizeof(TxnReqOut->ucLoyaltyCounter),APIOut->stAddValueSETM_t.unLoyaltyCounter);//Loyalty Counter
	fnUnPack(TxnReqOut->ucDeposit,sizeof(TxnReqOut->ucDeposit),APIOut->stAddValueSETM_t.unDeposit_4);//Deposit
	fnUnPack(DongleOut->ucMAC,sizeof(DongleOut->ucMAC),APIOut->stAddValueSETM_t.unMAC);//MAC
	fnUnPack(TxnReqOut->ucExpiryDate,sizeof(TxnReqOut->ucExpiryDate),APIOut->stAddValueSETM_t.anOrgExpiryDate);//Org Expiry Date
	memset(APIOut->stAddValueSETM_t.anNewExpiryDate,0x30,sizeof(APIOut->stAddValueSETM_t.anNewExpiryDate));//New Expiry Date
	fnUnPack(&TxnReqOut->ucAreaCode,sizeof(TxnReqOut->ucAreaCode),APIOut->stAddValueSETM_t.unAreaCode);//Area Code
	fnUnPack(TxnReqOut->ucEV_2,sizeof(TxnReqOut->ucEV_2),APIOut->stAddValueSETM_t.unEVBeforeTxn_4);//EV Before Txn
	//自動加值開啟電文
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",53);//Data Len
	vdBuildProcessingCode(inTxnType,0x00,0x00,0x00,APIOut->ucProcessignCode);//Processing Code
	memcpy(APIOut->ucCommandCode,"05",sizeof(APIOut->ucCommandCode));//Command Code
	APIOut->ucMsgType = TxnReqOut->ucMsgType;//Msg Type
	APIOut->ucSubType = TxnReqOut->ucSubType;//Sub Type
	memcpy(APIOut->ucDeviceID,TxnReqOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	memcpy(APIOut->ucTxnDateTime,stStore_DataInfo_t.ucTxnDateTime,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	memcpy(APIOut->ucCardID_4,TxnReqOut->ucCardID_4,sizeof(APIOut->ucCardID_4));//Card ID
	memcpy(APIOut->ucTxnSN_2,DongleOut->ucTxnSN_2,sizeof(APIOut->ucTxnSN_2));//Txn SN
	memcpy(APIOut->ucEV_2,DongleOut->ucEV_2,sizeof(APIOut->ucEV_2));//EV
	memcpy(APIOut->ucMAC,DongleOut->ucMAC,sizeof(APIOut->ucMAC));//MAC
	memcpy(APIOut->ucConfirmCode,DongleOut->ucConfirmCode,sizeof(APIOut->ucConfirmCode));//Confirm Code
	memcpy(APIOut->ucCTAC,DongleOut->ucCTAC,sizeof(APIOut->ucCTAC));//CTAC
	APIOut->ucAutoLoadFlag = 0x01;////AutoLoad Flag
	//APIOut->ucAutoLoadFlag = TxnReqOut->ucAutoLoadFlag;////AutoLoad Flag
	memcpy(APIOut->ucAutoLoadAmt_2,TxnReqOut->ucAutoLoadAmt_2,sizeof(APIOut->ucAutoLoadAmt_2));//AutoLoad Amt

	return(inTMOutLen);
}

int inBuildReadCardDeductData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
ReadCardDeduct_TM_Out *APIOut = (ReadCardDeduct_TM_Out *)API_Out;
ReadCardDeduct_APDU_Out *DongleOut = (ReadCardDeduct_APDU_Out *)Dongle_Out;

	inTMOutLen = sizeof(ReadCardDeduct_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	//fnPack("01320000EE591358206400006F0800FE00000000B2D3FE00000000000000000000023000006D591358206400006F0800FE00000000B2D3FE00000000000000000000032B0000E356135820640000D30800FE00000000B2D3FE00000000000000000000042800004CDE105820640000370900FE00000000B2D3FE0000000000000000000005270000EC5F0758206400009B0900FE00000000B2D3FE0000000000000000000006260000085F075820640000FF0900FE00000000B2D3FE000000000000000000009000",200,Dongle_Out);
	vdBuildReadCardBasicData2_2(0x01,&DongleOut->stCardDeductLog1_t,&APIOut->stCardDeductRecored1_t);
	vdBuildReadCardBasicData2_2(0x01,&DongleOut->stCardDeductLog2_t,&APIOut->stCardDeductRecored2_t);
	vdBuildReadCardBasicData2_2(0x01,&DongleOut->stCardDeductLog3_t,&APIOut->stCardDeductRecored3_t);
	vdBuildReadCardBasicData2_2(0x01,&DongleOut->stCardDeductLog4_t,&APIOut->stCardDeductRecored4_t);
	vdBuildReadCardBasicData2_2(0x01,&DongleOut->stCardDeductLog5_t,&APIOut->stCardDeductRecored5_t);
	vdBuildReadCardBasicData2_2(0x01,&DongleOut->stCardDeductLog6_t,&APIOut->stCardDeductRecored6_t);

	return(inTMOutLen);
}

int inBuildReadCodeVersionData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
ReadCodeVersion_TM_Out *APIOut = (ReadCodeVersion_TM_Out *)API_Out;
ReadCodeVersion_APDU_Out *DongleOut = (ReadCodeVersion_APDU_Out *)Dongle_Out;

	inTMOutLen = sizeof(ReadCodeVersion_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	APIOut->ucSAMAppletVersion = DongleOut->ucSAMAppletVersion;//SAM Applet Version
	APIOut->ucSAMType = DongleOut->ucSAMType;//SAM Type
	APIOut->ucSAMVersion = DongleOut->ucSAMVersion;//SAM Version
	memcpy(APIOut->ucReaderFWVersion,DongleOut->ucReaderFWVersion,sizeof(APIOut->ucReaderFWVersion));//Reader FW Version
	APIOut->ucHostSpecVersionNo = DongleOut->ucHostSpecVersionNo;//Host Spec Version No

	return(inTMOutLen);
}

int inBuildSetValue2Data(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
BYTE ucLoyaltyCounter[2];
SetValue2_TM_Out *APIOut = (SetValue2_TM_Out *)API_Out;
SetValue2_APDU_In *DongleIn = (SetValue2_APDU_In *)Dongle_In;
SetValue2_APDU_Out *DongleOut = (SetValue2_APDU_Out *)Dongle_Out;

	inTMOutLen = sizeof(SetValue2_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",inTMOutLen - 7);//Data Len
	vdBuildProcessingCode(inTxnType,DongleOut->ucMsgType,DongleOut->ucSubType,DongleOut->ucPersonalProfile,APIOut->ucProcessignCode);//Processing Code
	vdBuildBLCVersion(APIOut->ucBLVersion);
	//fnASCTOBIN(APIOut->ucBLVersion,gsBlackListVersion,5,sizeof(APIOut->ucBLVersion),DECIMAL);//Black List
	APIOut->ucMsgType = DongleOut->ucMsgType;//Msg Type
	APIOut->ucSubType = DongleOut->ucSubType;//Sub Type
	memcpy(APIOut->ucDeviceID,DongleOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	APIOut->ucSPID = DongleOut->ucSPID;//SP ID
	memcpy(APIOut->ucTxnDateTime,DongleIn->ucTxnDateTime,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	APIOut->ucCardType = DongleOut->ucCardType;//Card Type
	APIOut->ucPersonalProfile = DongleOut->ucPersonalProfile;//Personal Profile
	APIOut->ucIssuerCode = DongleOut->ucIssuerCode;//Issuer Code
	memcpy(APIOut->ucExpiryDate,DongleOut->ucExpiryDate,sizeof(APIOut->ucExpiryDate));//Expiry Date
	APIOut->ucAreaCode = DongleOut->ucAreaCode;//Area Code
	memcpy(APIOut->ucTMID,DongleIn->ucTMID,sizeof(APIOut->ucTMID));//TM ID
	memcpy(APIOut->ucTMTxnDateTime,DongleIn->ucTMTxnDateTime,sizeof(APIOut->ucTMTxnDateTime));//TM Txn Date Time
	memcpy(APIOut->ucTMSerialNumber,DongleOut->ucTMSerialNumber,sizeof(APIOut->ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->ucTMAgentNumber,DongleIn->ucTMAgentNumber,sizeof(APIOut->ucTMAgentNumber));//TM Agent Number
	if(DongleOut->ucPurseVersionNumber != LEVEL2)
	{
		memcpy(APIOut->ucSTAC,DongleOut->SAMID_STAC,sizeof(APIOut->ucSTAC));//STAC
		APIOut->ucKeyVersion = DongleOut->HostAdminKeyKVN_SAMKVN[0];//SAM KVN
		memcpy(APIOut->ucSAMID,&DongleOut->SVCrypto_SAMCRN_SAMID[8],sizeof(APIOut->ucSAMID));//SAM ID
		memcpy(APIOut->ucSAMSN,DongleOut->SAMSN,sizeof(APIOut->ucSAMSN));//SAM SN
		memcpy(APIOut->ucSAMCRN,DongleOut->SVCrypto_SAMCRN_SAMID,sizeof(APIOut->ucSAMCRN));//SAM CRN
	}
	else
	{
		memset(APIOut->ucSTAC,0x00,sizeof(APIOut->ucSTAC));//STAC
		APIOut->ucKeyVersion = 0;//SAM KVN
		memset(APIOut->ucSAMID,0x00,sizeof(APIOut->ucSAMID));//SAM ID
		memset(APIOut->ucSAMSN,0x00,sizeof(APIOut->ucSAMSN));//SAM SN
		memset(APIOut->ucSAMCRN,0x00,sizeof(APIOut->ucSAMCRN));//SAM CRN
	}
	memcpy(APIOut->ucReaderFWVersion,DongleOut->ucReaderFWVersion,sizeof(APIOut->ucReaderFWVersion));//Reader FW Version
	memcpy(APIOut->ucCardID,DongleOut->ucCardID,sizeof(APIOut->ucCardID));//Card ID
	memcpy(APIOut->ucTxnAmt,DongleOut->ucTxnAmt,sizeof(APIOut->ucTxnAmt));//Txn Amt
	memcpy(APIOut->ucCPUDeviceID,DongleOut->ucCPUDeviceID,sizeof(APIOut->ucCPUDeviceID));//CPU Device ID
	memcpy(APIOut->ucCPUSPID,DongleOut->ucCPUSPID,sizeof(APIOut->ucCPUSPID));//CPU SP ID
	memcpy(APIOut->ucCPULocationID,DongleOut->ucCPULocationID,sizeof(APIOut->ucCPULocationID));//CPU Location ID
	memcpy(APIOut->ucPID,DongleOut->ucPID,sizeof(APIOut->ucPID));//CPU Purse ID
	memcpy(APIOut->ucCTC,DongleOut->ucCTC,sizeof(APIOut->ucCTC));//CPU CTC
	memcpy(APIOut->ucProfileExpiryDate,DongleOut->ucProfileExpiryDate,sizeof(APIOut->ucProfileExpiryDate));//Profile Expiry Date
	if(DongleOut->ucPurseVersionNumber == LEVEL2)
		memcpy(APIOut->ucSubAreaCode,DongleOut->SubAreaCode_LoyaltyCounter,sizeof(APIOut->ucSubAreaCode));//Sub Area Code
	else
		memset(APIOut->ucSubAreaCode,0x00,sizeof(APIOut->ucSubAreaCode));//Sub Area Code
	memcpy(APIOut->ucTxnSN,DongleOut->ucTxnSN,sizeof(APIOut->ucTxnSN));//Txn SN
	memcpy(APIOut->ucEV,DongleOut->ucEV,sizeof(APIOut->ucEV));//EV
	APIOut->ucCPUAdminKeyKVN = DongleOut->ucCPUAdminKeyKVN;//CPU Admin Key KVN
	APIOut->ucCreditKeyKVN = DongleOut->ucCreditKeyKVN;//Credit Key KVN
	APIOut->ucCPUIssuerKeyKVN = DongleOut->ucCPUIssuerKeyKVN;//CPU Issuer Key KVN
	APIOut->ucTxnMode = DongleOut->ucTxnMode;//Txn Mode
	APIOut->ucTxnQuqlifier = DongleOut->ucTxnQuqlifier;//Txn Quqlifier
	if(DongleOut->ucPurseVersionNumber == LEVEL2)
	{
		memcpy(APIOut->ucCPUSAMID,&DongleOut->SAMID_STAC,sizeof(APIOut->ucCPUSAMID));//CPU SAM ID
		APIOut->ucHostAdminKVN = DongleOut->HostAdminKeyKVN_SAMKVN[0];//Host Admin KVN
	}
	else
	{
		memset(APIOut->ucCPUSAMID,0x00,sizeof(APIOut->ucCPUSAMID));//CPU SAM ID
		APIOut->ucHostAdminKVN = 0x00;//Host Admin KVN
	}
	APIOut->ucPurseVersionNumber = DongleOut->ucPurseVersionNumber;//Purse Version Number
	memcpy(APIOut->ucTMLocationID,DongleIn->ucTMLocationID,sizeof(APIOut->ucTMLocationID));//TM Location ID
	if(DongleOut->ucPurseVersionNumber != LEVEL2)
		memset(APIOut->ucCACrypto,0x00,sizeof(APIOut->ucCACrypto));//CACrypto後8碼
	else
		memcpy(APIOut->ucCACrypto,DongleOut->SVCrypto_SAMCRN_SAMID,sizeof(APIOut->ucCACrypto));//CACrypto
	memset(ucLoyaltyCounter,0x00,sizeof(ucLoyaltyCounter));
	vdBuildCardAVRData(&APIOut->stCardAVRInfo_t,&DongleOut->stLastCreditTxnLogInfo_t,DongleOut->ucCardID,DongleOut->ucIssuerCode,DongleOut->ucCPUSPID,DongleOut->ucCPULocationID,DongleOut->ucPID,ucLoyaltyCounter);//Card AVR Info
	APIOut->ucCardIDLen = DongleOut->ucCardIDLen;//Card ID Len

	return(inTMOutLen);
}

int inBuildAuthSetValue2Data(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
AuthSetValue2_TM_Out *APIOut = (AuthSetValue2_TM_Out *)API_Out;
AuthSetValue2_APDU_In *DongleIn = (AuthSetValue2_APDU_In *)Dongle_In;
AuthSetValue2_APDU_Out *DongleOut = (AuthSetValue2_APDU_Out *)Dongle_Out;
SetValue2_APDU_Out *TxnReqOut = (SetValue2_APDU_Out *)Dongle_ReqOut;

	inTMOutLen = sizeof(AuthSetValue2_TM_Out);
	memset(APIOut,0x00,inTMOutLen);

	vdUIntToAsc(TxnReqOut->ucCardID,sizeof(TxnReqOut->ucCardID),APIOut->anCardID,sizeof(APIOut->anCardID),FALSE,' ',10);//n_Card Physical ID
	fnBINTODEVASC(TxnReqOut->ucDeviceID,APIOut->anDeviceID,sizeof(APIOut->anDeviceID),MIFARE);//n_TXN Device ID
	fnBINTODEVASC(TxnReqOut->ucCPUDeviceID,APIOut->anCPUDeviceID,sizeof(APIOut->anCPUDeviceID),LEVEL2);//n_TXN Device ID
	vdCountEVBefore(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->anEVBeforeTxn,sizeof(APIOut->anEVBeforeTxn),TxnReqOut->ucMsgType);//n_EV Before TXN
	UnixToDateTime((BYTE*)TxnReqOut->ucExpiryDate,(BYTE*)APIOut->anOrgExpiryDate,8);//原票卡到期日
	UnixToDateTime((BYTE*)DongleIn->ucExpiryDate,(BYTE*)APIOut->anNewExpiryDate,8);//新票卡到期日
	vdBuildProcessingCode(inTxnType,TxnReqOut->ucMsgType,TxnReqOut->ucSubType,TxnReqOut->ucPersonalProfile,APIOut->anProcessignCode);//Processing Code
	return(inTMOutLen);
}

int inBuildLockCardData7(int inTxnType,STRUCT_XML_DOC *srXML,LockCard_TM_Out_2 *APIOut,TxRefund_APDU_Out *DongleOut,BYTE *ucTxnDateTime,BYTE ucLockReason)
{
int inTMOutLen;
BYTE bSpace[20],ucCardID[7];

	inTMOutLen = sizeof(LockCard_TM_Out_2);
	memset(APIOut,0x00,inTMOutLen);
	memset(bSpace,0x20,sizeof(bSpace));
	memset(APIOut->unPurseVersionNumber,0x30,sizeof(APIOut->unPurseVersionNumber));//Purse Version Number
	sprintf((char *)APIOut->unMsgType,"22");//Msg Type
	sprintf((char *)APIOut->unSubType,"00");//Sub Type
	fnUnPack(DongleOut->ucDeviceID,sizeof(DongleOut->ucDeviceID),APIOut->unDeviceID);//Device ID
	fnUnPack(&DongleOut->ucDeviceID[2],1,APIOut->unSPID);//SP ID
	fnUnPack(ucTxnDateTime,sizeof(APIOut->unTxnDateTime) / 2,APIOut->unTxnDateTime);//Txn Date Time
	memset(ucCardID,0x00,sizeof(ucCardID));
	memcpy(ucCardID,DongleOut->ucCardID_4,sizeof(DongleOut->ucCardID_4));
	fnUnPack(ucCardID,sizeof(ucCardID),APIOut->unCardID);//Card ID
	fnUnPack(&DongleOut->ucIssuerCode,sizeof(DongleOut->ucIssuerCode),APIOut->unIssuerCode);//Issuer Code
	memcpy(APIOut->unLocationID,LOCATION_ID_BATCH,2);//Location ID
	memset(APIOut->unCardIDLen,0x30,sizeof(APIOut->unCardIDLen));//Card ID Len
	memset(APIOut->unCPUDeviceID,0x30,sizeof(APIOut->unCPUDeviceID));//CPU Device ID
	memset(APIOut->unCPUSPID,0x30,sizeof(APIOut->unCPUSPID));//CPU SP ID
	memset(APIOut->unCPULocationID,0x30,sizeof(APIOut->unCPULocationID));//CPU Location ID
	fnUnPack(&DongleOut->ucCardType,sizeof(DongleOut->ucCardType),APIOut->unCardType);//Card Type
	fnUnPack(&DongleOut->ucPersonalProfile,sizeof(DongleOut->ucPersonalProfile),APIOut->unPersonalProfile);//Personal Profile
	memset(APIOut->unPID,0x30,sizeof(APIOut->unPID));//CPU Purse ID
	memset(APIOut->unCTC,0x30,sizeof(APIOut->unCTC));//CTC
	fnUnPack(&ucLockReason,sizeof(ucLockReason),APIOut->unLockReason);//Lock Reason
	memset(APIOut->unBLCFileName,0x20,sizeof(APIOut->unBLCFileName));//BLC File Name
	APIOut->unBLCIDFlag = 0x20;//BLC ID Flag
	memcpy(APIOut->unTMLocationID,DongleOut->ucTMLocationID_6,sizeof(APIOut->unTMLocationID));//TM Location ID
	memcpy(APIOut->unTMID,DongleOut->ucTMID,sizeof(APIOut->unTMID));//TM ID
	memset(APIOut->unRFU,0x20,sizeof(APIOut->unRFU));//RFU

	return(inTMOutLen);
}

int inBuildTxRefundData2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
TxnReqOnline_TM_Out_2 *APIOut = (TxnReqOnline_TM_Out_2 *)API_Out;
TxRefund_APDU_In *DongleIn = (TxRefund_APDU_In *)Dongle_In;
TxRefund2_APDU_Out *DongleOut = (TxRefund2_APDU_Out *)Dongle_Out;

	inTMOutLen = sizeof(TxnReqOnline_TM_Out_2);
	memset(APIOut,0x00,inTMOutLen);
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",inTMOutLen - 7);//Data Len
	vdBuildProcessingCode(inTxnType,0x00,0x00,DongleOut->ucPersonalProfile,APIOut->ucProcessignCode);//Processing Code
	vdBuildBLCVersion(APIOut->ucBLVersion);
	//fnASCTOBIN(APIOut->ucBLVersion,gsBlackListVersion,5,sizeof(APIOut->ucBLVersion),DECIMAL);//Black List Version
	APIOut->ucMsgType = DongleIn->ucMsgType;//Msg Type
	APIOut->ucSubType = DongleIn->ucSubType;//Sub Type
	memcpy(APIOut->ucDeviceID,DongleOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	APIOut->ucSPID = DongleOut->ucDeviceID[2];//SP ID
	memcpy(APIOut->ucTxnDateTime,DongleIn->ucTxnDateTime2,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	APIOut->ucCardType = DongleOut->ucCardType;//Card Type
	APIOut->ucPersonalProfile = DongleOut->ucPersonalProfile;//Personal Profile
	APIOut->ucIssuerCode = DongleOut->ucIssuerCode;//Issuer Code
	APIOut->ucLocationID = LOCATION_ID;//Location ID
	APIOut->ucBankCode = DongleOut->ucBankCode;//Bank Code
	memcpy(APIOut->ucLoyaltyCounter,DongleOut->ucLoyaltyCounter,sizeof(APIOut->ucLoyaltyCounter));//Loyalty Counter
	memset(APIOut->ucExpiryDate,0x00,sizeof(APIOut->ucExpiryDate));//Expiry Date
	APIOut->ucAreaCode = DongleOut->ucAreaCode;//Area Code
	memcpy(APIOut->ucTMID,DongleIn->ucTMID,sizeof(APIOut->ucTMID));//TM ID
	memcpy(APIOut->ucTMTxnDateTime,DongleOut->ucTMTxnDateTime,sizeof(DongleOut->ucTMTxnDateTime));//TM Date Time
	memcpy(APIOut->ucTMSerialNumber,DongleOut->ucTMSerialNumber,sizeof(DongleOut->ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->ucTMAgentNumber,DongleOut->ucTMAgentNumber,sizeof(DongleOut->ucTMAgentNumber));//TM Agent Number
	memcpy(APIOut->ucSTAC,DongleOut->ucSTAC,sizeof(APIOut->ucSTAC));//S-TAC
	APIOut->ucKeyVersion = DongleOut->ucSAMKeyVersion;//Key Version
	memcpy(APIOut->ucSAMID,DongleOut->ucSAMID,sizeof(APIOut->ucSAMID));//SAM ID
	memcpy(APIOut->ucSAMSN,DongleOut->ucSAMSN,sizeof(APIOut->ucSAMSN));//SAM SN
	memcpy(APIOut->ucSAMCRN,DongleOut->ucSAMCRN,sizeof(APIOut->ucSAMCRN));//SAM CRN
	memcpy(APIOut->ucReaderFWVersion,DongleOut->ucReaderFWVersion,sizeof(APIOut->ucReaderFWVersion));//Reader Firmward Version
	memcpy(APIOut->ucCardID,DongleOut->ucCardID_4,sizeof(DongleOut->ucCardID_4));//Card ID 4
	memcpy(APIOut->ucTxnAmt,DongleOut->ucTxnAmt2_2,sizeof(DongleOut->ucTxnAmt_2));//Txn Amt 2
	if((APIOut->ucTxnAmt[1] & 0x80) == 0x80)
		APIOut->ucTxnAmt[2] = 0xFF;
	memcpy(APIOut->ucCPUDeviceID,DongleOut->ucCPUDeviceID,sizeof(DongleOut->ucCPUDeviceID));//CPU Device ID
	memcpy(APIOut->ucCPUSPID,&DongleOut->ucCPUDeviceID[3],sizeof(APIOut->ucCPUSPID));//CPU SP ID
	memset(APIOut->ucCPULocationID,0x00,sizeof(APIOut->ucCPULocationID));//CPU Location ID
	memset(APIOut->ucPID,0x00,sizeof(APIOut->ucPID));//PID
	memset(APIOut->ucCTC,0x00,sizeof(APIOut->ucCTC));//CTC
	memset(APIOut->ucProfileExpiryDate,0x00,sizeof(APIOut->ucProfileExpiryDate));//Profile Expiry Date
	memset(APIOut->ucSubAreaCode,0x00,sizeof(APIOut->ucSubAreaCode));//Sub Area Code
	memset(APIOut->ucTxnSN,0x00,sizeof(APIOut->ucTxnSN));//Txn SN
	vdAmt2ByteTo3Byte(DongleOut->ucEV2_2,APIOut->ucEV);//EV
	//memcpy(APIOut->ucEV,DongleOut->ucEV2_2,sizeof(DongleOut->ucEV_2));//EV 2
	memcpy(APIOut->ucDeposit,DongleOut->ucDeposit,sizeof(DongleOut->ucDeposit));//Deposit
	APIOut->ucCPUAdminKeyKVN = 0x00;
	APIOut->ucCreditKeyKVN = 0x00;
	APIOut->ucCPUIssuerKeyKVN = 0x00;
	APIOut->ucTxnMode = 0x00;
	APIOut->ucTxnQuqlifier = 0x00;
	APIOut->ucSignatureKeyKVN = DongleOut->ucSAMKeyVersion;
	memset(APIOut->ucCPUSAMID,0x00,sizeof(APIOut->ucCPUSAMID));//Txn SN
	APIOut->ucHostAdminKVN = 0x00;
	APIOut->ucPurseVersionNumber = 0x00;
	memset(APIOut->ucTMLocationID,0x20,sizeof(APIOut->ucTMLocationID));
	memcpy(&APIOut->ucTMLocationID[4],DongleIn->ucTMLocationID_6,sizeof(DongleIn->ucTMLocationID_6));//TM Location ID 6
	memset(APIOut->ucCACrypto,0x00,sizeof(APIOut->ucCACrypto));//CA Crypto

	APIOut->stCardAVRInfo_t.ucMsgType = 0x0C;
	APIOut->stCardAVRInfo_t.ucPurseVersionNumber = 0x00;
	APIOut->stCardAVRInfo_t.ucTxnSN[0] = DongleOut->ucCAVRTxnSNLSB;//CAVR Txn SN LSB
	memcpy(APIOut->stCardAVRInfo_t.ucTxnDateTime,DongleOut->ucCAVRTxnDateTime,sizeof(DongleOut->ucCAVRTxnDateTime));//CAVR Txn Date Time
	APIOut->stCardAVRInfo_t.ucSubType = DongleOut->ucCAVRSubType;
	memcpy(APIOut->stCardAVRInfo_t.ucTxnAmt,DongleOut->ucCAVRTxnAmt,sizeof(DongleOut->ucCAVRTxnAmt));//CAVR Txn Amt
	memcpy(APIOut->stCardAVRInfo_t.ucEV,DongleOut->ucCAVREV,sizeof(DongleOut->ucCAVREV));//CAVR Txn EV
	memcpy(APIOut->stCardAVRInfo_t.ucCardID,DongleOut->ucCardID_4,sizeof(DongleOut->ucCardID_4));//CAVR Card ID 4
	APIOut->stCardAVRInfo_t.ucIssuerCode = DongleOut->ucIssuerCode;//CAVR Issuer Code
	memset(APIOut->stCardAVRInfo_t.ucCPUSPID,0x00,sizeof(APIOut->stCardAVRInfo_t.ucCPUSPID));//CAVR CPU SP ID
	memset(APIOut->stCardAVRInfo_t.ucCPULocationID,0x00,sizeof(APIOut->stCardAVRInfo_t.ucCPULocationID));//CAVR CPU Location ID
	memset(APIOut->stCardAVRInfo_t.ucPID,0x00,sizeof(APIOut->stCardAVRInfo_t.ucPID));//CAVR CPU PID
	memset(APIOut->stCardAVRInfo_t.ucCPUDeviceID,0x00,sizeof(APIOut->stCardAVRInfo_t.ucCPUDeviceID));//CAVR CPU Device ID
	memcpy(APIOut->stCardAVRInfo_t.ucLoyaltyCounter,DongleOut->ucLoyaltyCounter,sizeof(APIOut->ucLoyaltyCounter));//CAVR Loyalty Counter
	vdBuildReaderAVRData(&APIOut->stReaderAVRInfo_t,&DongleOut->stReaderAVRInfo_t);//Reader AVR Info
	APIOut->ucCardIDLen = 4;

	return(inTMOutLen);
}

int inBuildAuthTxRefundData2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
BYTE ucTxnDateTime[4];
AuthTxRefund2_TM_Out *APIOut = (AuthTxRefund2_TM_Out *)API_Out;
AuthTxnOnline_TM_In *DongleIn = (AuthTxnOnline_TM_In *)Dongle_In;
AuthTxRefund_APDU_Out *DongleOut = (AuthTxRefund_APDU_Out *)Dongle_Out;
TxRefund2_APDU_Out *TxnReqOut = (TxRefund2_APDU_Out *)Dongle_ReqOut;

	inTMOutLen = sizeof(AuthTxRefund2_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	//餘退STMC開始
	vdUIntToAsc(TxnReqOut->ucCardID_4,sizeof(TxnReqOut->ucCardID_4),APIOut->stTxRefundSTMC_t.anCardID,sizeof(APIOut->stTxRefundSTMC_t.anCardID),FALSE,' ',10);//n_Card Physical ID
	fnBINTODEVASC(TxnReqOut->ucDeviceID,APIOut->stTxRefundSTMC_t.anDeviceID,sizeof(APIOut->stTxRefundSTMC_t.anDeviceID),MIFARE);//n_TXN Device ID
	fnBINTODEVASC(TxnReqOut->ucCPUDeviceID,APIOut->stTxRefundSTMC_t.anCPUDeviceID,sizeof(APIOut->stTxRefundSTMC_t.anCPUDeviceID),LEVEL2);//n_TXN Device ID
	vdIntToAsc(TxnReqOut->ucEV2_2,sizeof(TxnReqOut->ucEV2_2),APIOut->stTxRefundSTMC_t.anEVBeforeTxn,sizeof(APIOut->stTxRefundSTMC_t.anEVBeforeTxn),FALSE,' ',10);//n_EV Before TXN
	//vdCountEVBefore(TxnReqOut->ucTxnAmt2_2,sizeof(TxnReqOut->ucTxnAmt2_2),DongleOut->ucEV2_2,sizeof(DongleOut->ucEV2_2),APIOut->stTxRefundSTMC_t.anEVBeforeTxn,sizeof(APIOut->stTxRefundSTMC_t.anEVBeforeTxn),TxnReqOut->ucMsgType);//n_EV Before TXN
	vdUIntToAsc(DongleOut->ucTxnSN2_2,sizeof(DongleOut->ucTxnSN2_2),APIOut->stTxRefundSTMC_t.anTxnSN,sizeof(APIOut->stTxRefundSTMC_t.anTxnSN),FALSE,' ',10);//n_TXN SNum
	vdUIntToAsc(TxnReqOut->ucTxnAmt2_2,sizeof(TxnReqOut->ucTxnAmt2_2),APIOut->stTxRefundSTMC_t.anTxnAmt,sizeof(APIOut->stTxRefundSTMC_t.anTxnAmt),FALSE,' ',10);//n_TXN AMT
	vdIntToAsc(DongleOut->ucEV2_2,sizeof(DongleOut->ucEV2_2),APIOut->stTxRefundSTMC_t.anEV,sizeof(APIOut->stTxRefundSTMC_t.anEV),FALSE,' ',10);//n_EV
	vdBuildProcessingCode(inTxnType,TxnReqOut->ucMsgType,TxnReqOut->ucSubType,TxnReqOut->ucPersonalProfile,APIOut->stTxRefundSTMC_t.anProcessignCode);//Processing Code
	fnUnPack(&TxnReqOut->ucMsgType,sizeof(TxnReqOut->ucMsgType),APIOut->stTxRefundSTMC_t.unMsgType);//Msg Type
	fnUnPack(&TxnReqOut->ucSubType,sizeof(TxnReqOut->ucSubType),APIOut->stTxRefundSTMC_t.unSubType);//Sub Type
	fnUnPack(TxnReqOut->ucDeviceID,sizeof(TxnReqOut->ucDeviceID),APIOut->stTxRefundSTMC_t.unDeviceID);//Device ID
	fnUnPack(&TxnReqOut->ucDeviceID[2],1,APIOut->stTxRefundSTMC_t.unSPID);//SP ID
	fngetUnixTimeCnt(ucTxnDateTime,TxnReqOut->ucTMTxnDateTime2);
	fnUnPack(ucTxnDateTime,sizeof(ucTxnDateTime),APIOut->stTxRefundSTMC_t.unTxnDateTime);//Txn Date Time
	fnUnPack(&TxnReqOut->ucCardType,sizeof(TxnReqOut->ucCardType),APIOut->stTxRefundSTMC_t.unCardType);//Card Type
	fnUnPack(&TxnReqOut->ucPersonalProfile,sizeof(TxnReqOut->ucPersonalProfile),APIOut->stTxRefundSTMC_t.unPersonalProfile);//Personal Profile
	sprintf((char *)APIOut->stTxRefundSTMC_t.unLocationID,"01");//Location ID
	memset(APIOut->stTxRefundSTMC_t.unCardID,0x30,sizeof(APIOut->stTxRefundSTMC_t.unCardID));//Org Card ID
	fnUnPack(TxnReqOut->ucCardID_4,sizeof(TxnReqOut->ucCardID_4),APIOut->stTxRefundSTMC_t.unCardID);//CPU Card ID
	fnUnPack(&TxnReqOut->ucIssuerCode,sizeof(TxnReqOut->ucIssuerCode),APIOut->stTxRefundSTMC_t.unIssuerCode);//Issuer Code
	fnUnPack(&TxnReqOut->ucBankCode,sizeof(TxnReqOut->ucBankCode),APIOut->stTxRefundSTMC_t.unBankCode);//Bank Code
	fnUnPack(TxnReqOut->ucLoyaltyCounter,sizeof(TxnReqOut->ucLoyaltyCounter),APIOut->stTxRefundSTMC_t.unLoyaltyCounter);//Loyalty Counter
	fnUnPack(DongleOut->ucMAC2,sizeof(DongleOut->ucMAC2),APIOut->stTxRefundSTMC_t.unMAC);//MAC
	memset(APIOut->stTxRefundSTMC_t.unOrgExpiryDate,0x30,sizeof(APIOut->stTxRefundSTMC_t.unOrgExpiryDate));//Org Expiry Date
	fnUnPack(DongleIn->ucExpiryDate,sizeof(DongleIn->ucExpiryDate),APIOut->stTxRefundSTMC_t.unNewExpiryDate);//New Expiry Date
	fnUnPack(&TxnReqOut->ucAreaCode,sizeof(TxnReqOut->ucAreaCode),APIOut->stTxRefundSTMC_t.unAreaCode);//Area Code
	memset(APIOut->stTxRefundSTMC_t.unTxnAmt,0x30,sizeof(APIOut->stTxRefundSTMC_t.unTxnAmt));//Txn Amt
	fnUnPack(TxnReqOut->ucTxnAmt2_2,sizeof(TxnReqOut->ucTxnAmt2_2),APIOut->stTxRefundSTMC_t.unTxnAmt);//Txn Amt
	vdUnPackAmt2ByteTo3Byte(DongleOut->ucEV2_2,APIOut->stTxRefundSTMC_t.unEV);
	//memset(APIOut->stTxRefundSTMC_t.unEV,0x30,sizeof(APIOut->stTxRefundSTMC_t.unEV));//EV
	//fnUnPack(DongleOut->ucEV2_2,sizeof(DongleOut->ucEV2_2),APIOut->stTxRefundSTMC_t.unEV);//EV
	memset(APIOut->stTxRefundSTMC_t.unTxnSN,0x30,sizeof(APIOut->stTxRefundSTMC_t.unTxnSN));//Txn SN
	fnUnPack(DongleOut->ucTxnSN2_2,sizeof(DongleOut->ucTxnSN2_2),APIOut->stTxRefundSTMC_t.unTxnSN);//Txn SN
	fnUnPack(TxnReqOut->ucCPUDeviceID,sizeof(TxnReqOut->ucCPUDeviceID),APIOut->stTxRefundSTMC_t.unCPUDeviceID);//CPU Device ID
	fnUnPack(&TxnReqOut->ucCPUDeviceID[3],3,APIOut->stTxRefundSTMC_t.unCPUSPID);//CPU SP ID
	memset(APIOut->stTxRefundSTMC_t.unCPULocationID,0x30,sizeof(APIOut->stTxRefundSTMC_t.unCPULocationID));//CPU Location ID
	memset(APIOut->stTxRefundSTMC_t.unPID,0x30,sizeof(APIOut->stTxRefundSTMC_t.unPID));//Purse ID
	memset(APIOut->stTxRefundSTMC_t.unCTC,0x30,sizeof(APIOut->stTxRefundSTMC_t.unCTC));//CTC
	memset(APIOut->stTxRefundSTMC_t.unOrgProfileExpiryDate,0x30,sizeof(APIOut->stTxRefundSTMC_t.unOrgProfileExpiryDate));//Org Profile Expiry Date
	fnUnPack(DongleIn->ucProfileExpiryDate,sizeof(DongleIn->ucProfileExpiryDate),APIOut->stTxRefundSTMC_t.unNewProfileExpiryDate);//New Profile Expiry Date
	memset(APIOut->stTxRefundSTMC_t.unSubAreaCode,0x30,sizeof(APIOut->stTxRefundSTMC_t.unSubAreaCode));//Sub Area Code
	memset(APIOut->stTxRefundSTMC_t.unTxnSNBeforeTxn,0x30,sizeof(APIOut->stTxRefundSTMC_t.unTxnSNBeforeTxn));//Txn SN Before Txn
	fnUnPack(TxnReqOut->ucTxmSn2_2,sizeof(TxnReqOut->ucTxmSn2_2),APIOut->stTxRefundSTMC_t.unTxnSNBeforeTxn);//Txn SN Before Txn
	vdUnPackAmt2ByteTo3Byte(TxnReqOut->ucEV2_2,APIOut->stTxRefundSTMC_t.unEVBeforeTxn);
	//memset(APIOut->stTxRefundSTMC_t.unEVBeforeTxn,0x30,sizeof(APIOut->stTxRefundSTMC_t.unEVBeforeTxn));//EV Before Txn
	//fnUnPack(TxnReqOut->ucEV2_2,sizeof(TxnReqOut->ucEV2_2),APIOut->stTxRefundSTMC_t.unEVBeforeTxn);//EV Before Txn
	memset(APIOut->stTxRefundSTMC_t.unDeposit,0x30,sizeof(APIOut->stTxRefundSTMC_t.unDeposit));//Deposit
	fnUnPack(TxnReqOut->ucDeposit,sizeof(TxnReqOut->ucDeposit),APIOut->stTxRefundSTMC_t.unDeposit);//Deposit
	memset(APIOut->stTxRefundSTMC_t.unTxnMode,0x30,sizeof(APIOut->stTxRefundSTMC_t.unTxnMode));//Txn Mode
	memset(APIOut->stTxRefundSTMC_t.unTxnQuqlifier,0x30,sizeof(APIOut->stTxRefundSTMC_t.unTxnQuqlifier));//Txn Quqlifier
	memset(APIOut->stTxRefundSTMC_t.unSignatureKeyKVN,0x30,sizeof(APIOut->stTxRefundSTMC_t.unSignatureKeyKVN));//Signature Key KVN
	memset(APIOut->stTxRefundSTMC_t.unSignature,0x30,sizeof(APIOut->stTxRefundSTMC_t.unSignature));//Signature
	memset(APIOut->stTxRefundSTMC_t.unCPUSAMID,0x30,sizeof(APIOut->stTxRefundSTMC_t.unCPUSAMID));//SAM ID
	memset(APIOut->stTxRefundSTMC_t.unHashType,0x30,sizeof(APIOut->stTxRefundSTMC_t.unHashType));
	memset(APIOut->stTxRefundSTMC_t.unHostAdminKVN,0x30,sizeof(APIOut->stTxRefundSTMC_t.unHostAdminKVN));
	memset(APIOut->stTxRefundSTMC_t.unCPUMAC,0x30,sizeof(APIOut->stTxRefundSTMC_t.unCPUMAC));//MAC
	sprintf((char *)APIOut->stTxRefundSTMC_t.unPurseVersionNumber,"00");//Purse Version Number
	sprintf((char *)APIOut->stTxRefundSTMC_t.unCardIDLen,"04");//Card ID Len
	memset(APIOut->stTxRefundSTMC_t.ucTMLocationID,0x30,sizeof(APIOut->stTxRefundSTMC_t.ucTMLocationID));//MAC
	memcpy(APIOut->stTxRefundSTMC_t.ucTMLocationID,TxnReqOut->ucTMLocationID_6,sizeof(TxnReqOut->ucTMLocationID_6));//TM Location ID
	memcpy(APIOut->stTxRefundSTMC_t.ucTMID,TxnReqOut->ucTMID,sizeof(TxnReqOut->ucTMID));//TM ID
	memcpy(APIOut->stTxRefundSTMC_t.ucTMTxnDateTime,TxnReqOut->ucTMTxnDateTime2,sizeof(TxnReqOut->ucTMTxnDateTime2));//TM Txn Date Time
	memcpy(APIOut->stTxRefundSTMC_t.ucTMSerialNumber,TxnReqOut->ucTMSerialNumber2,sizeof(TxnReqOut->ucTMSerialNumber2));//TM Serial Number
	memcpy(APIOut->stTxRefundSTMC_t.ucTMAgentNumber,TxnReqOut->ucTMAgentNumber,sizeof(TxnReqOut->ucTMAgentNumber));//TM Agent Number
	//帳務資料結束
	//Congirm資料開始
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",79);//Data Len
	vdBuildProcessingCode(inTxnType,TxnReqOut->ucMsgType,TxnReqOut->ucSubType,TxnReqOut->ucPersonalProfile,APIOut->ucProcessignCode);//Processing Code
	APIOut->ucMsgType = TxnReqOut->ucMsgType;//Msg Type
	APIOut->ucSubType = TxnReqOut->ucSubType;//Sub Type
	memcpy(APIOut->ucDeviceID,TxnReqOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	memcpy(APIOut->ucTxnDateTime,ucTxnDateTime,sizeof(ucTxnDateTime));//Txn Date Time
	memcpy(APIOut->ucMAC,DongleOut->ucMAC2,sizeof(APIOut->ucMAC));//MAC
	memcpy(APIOut->ucConfirmCode,DongleOut->ucConfirmCode,sizeof(APIOut->ucConfirmCode));//Confirm Code
	memcpy(APIOut->ucCTAC,DongleOut->ucCTAC,sizeof(APIOut->ucMAC));//CTAC
	memcpy(APIOut->ucCardID,TxnReqOut->ucCardID_4,sizeof(TxnReqOut->ucCardID_4));//Card ID
	vdAmt2ByteTo3Byte(DongleOut->ucEV2_2,APIOut->ucEV);//EV
	memcpy(APIOut->ucTxnSN,DongleOut->ucTxnSN2_2,sizeof(DongleOut->ucTxnSN2_2));//Txn SN
	memcpy(APIOut->ucCPUDeviceID,TxnReqOut->ucCPUDeviceID,sizeof(APIOut->ucCPUDeviceID));//CPU Device ID
	memset(APIOut->ucPID,0x00,sizeof(APIOut->ucPID));//Purse ID
	memset(APIOut->ucSignature,0x00,sizeof(APIOut->ucSignature));//Signature
	APIOut->ucHashType = 0x00;
	APIOut->ucHostAdminKVN = 0x00;
	memset(APIOut->ucCPUMAC,0x00,sizeof(APIOut->ucCPUMAC));//CPU MAC

	//餘扣STMC開始
	memcpy(&APIOut->stTxDeductSTMC_t,&APIOut->stTxRefundSTMC_t,sizeof(STMC_t));
	vdCountEVBefore(TxnReqOut->ucTxnAmt_2,sizeof(TxnReqOut->ucTxnAmt_2),DongleOut->ucEV_2,sizeof(DongleOut->ucEV_2),APIOut->stTxDeductSTMC_t.anEVBeforeTxn,sizeof(APIOut->stTxDeductSTMC_t.anEVBeforeTxn),TxnReqOut->ucMsgType);//n_EV Before TXN
	vdUIntToAsc(DongleOut->ucTxnSN_2,sizeof(DongleOut->ucTxnSN_2),APIOut->stTxDeductSTMC_t.anTxnSN,sizeof(APIOut->stTxDeductSTMC_t.anTxnSN),FALSE,' ',10);//n_TXN SNum
	vdUIntToAsc(TxnReqOut->ucTxnAmt_2,sizeof(TxnReqOut->ucTxnAmt_2),APIOut->stTxDeductSTMC_t.anTxnAmt,sizeof(APIOut->stTxDeductSTMC_t.anTxnAmt),FALSE,' ',10);//n_TXN AMT
	vdIntToAsc(DongleOut->ucEV_2,sizeof(DongleOut->ucEV_2),APIOut->stTxDeductSTMC_t.anEV,sizeof(APIOut->stTxDeductSTMC_t.anEV),FALSE,' ',10);//n_EV
	fngetUnixTimeCnt(ucTxnDateTime,TxnReqOut->ucTMTxnDateTime);
	fnUnPack(ucTxnDateTime,sizeof(ucTxnDateTime),APIOut->stTxDeductSTMC_t.unTxnDateTime);//Txn Date Time
	fnUnPack(DongleOut->ucMAC,sizeof(DongleOut->ucMAC),APIOut->stTxDeductSTMC_t.unMAC);//MAC
	fnUnPack(TxnReqOut->ucTxnAmt_2,sizeof(TxnReqOut->ucTxnAmt_2),APIOut->stTxDeductSTMC_t.unTxnAmt);//Txn Amt
	vdUnPackAmt2ByteTo3Byte(DongleOut->ucEV_2,APIOut->stTxDeductSTMC_t.unEV);
	//vdUnPackAmt2ByteTo3Byte(DongleOut->ucEV2_2,APIOut->stTxDeductSTMC_t.unEV);
	//fnUnPack(DongleOut->ucEV_2,sizeof(DongleOut->ucEV_2),APIOut->stTxDeductSTMC_t.unEV);//EV
	fnUnPack(DongleOut->ucTxnSN_2,sizeof(DongleOut->ucTxnSN_2),APIOut->stTxDeductSTMC_t.unTxnSN);//Txn SN
	fnUnPack(TxnReqOut->ucTxmSn_2,sizeof(TxnReqOut->ucTxmSn_2),APIOut->stTxDeductSTMC_t.unTxnSNBeforeTxn);//Txn SN Before Txn
	vdUnPackAmt2ByteTo3Byte(TxnReqOut->ucEV_2,APIOut->stTxDeductSTMC_t.unEVBeforeTxn);
	//vdUnPackAmt2ByteTo3Byte(TxnReqOut->ucEV2_2,APIOut->stTxDeductSTMC_t.unEVBeforeTxn);
	//fnUnPack(TxnReqOut->ucEV_2,sizeof(TxnReqOut->ucEV_2),APIOut->stTxDeductSTMC_t.unEVBeforeTxn);//EV Before Txn
	memcpy(APIOut->stTxDeductSTMC_t.anProcessignCode,"813599",sizeof(APIOut->stTxDeductSTMC_t.anProcessignCode));//Processign Code
	memcpy(APIOut->stTxDeductSTMC_t.unMsgType,"01",sizeof(APIOut->stTxDeductSTMC_t.unMsgType));//Msg Type
	memcpy(APIOut->stTxDeductSTMC_t.unSubType,"00",sizeof(APIOut->stTxDeductSTMC_t.unSubType));//Sub Type
	memcpy(APIOut->stTxDeductSTMC_t.ucTMTxnDateTime,TxnReqOut->ucTMTxnDateTime,sizeof(TxnReqOut->ucTMTxnDateTime));//TM Txn Date Time
	memcpy(APIOut->stTxDeductSTMC_t.ucTMSerialNumber,TxnReqOut->ucTMSerialNumber,sizeof(TxnReqOut->ucTMSerialNumber));//TM Serial Number
	//帳務資料結束,加值重送資料開始
	memcpy(APIOut->unVARMsgType,"0C",2);//VAR Msg Type
	sprintf((char *)APIOut->unVARPurseVersionNumber,"00");//VAR PurseVersion Number
	memset(APIOut->unVARTxnSN,0x30,sizeof(APIOut->unVARTxnSN));//VAR Txn SN
	fnUnPack(&TxnReqOut->ucCAVRTxnSNLSB,sizeof(TxnReqOut->ucCAVRTxnSNLSB),APIOut->unVARTxnSN);//VAR Txn SN
	fnUnPack(TxnReqOut->ucCAVRTxnDateTime,sizeof(TxnReqOut->ucCAVRTxnDateTime),APIOut->unVARTxnDateTime);//VAR Txn Date Time
	fnUnPack(&TxnReqOut->ucCAVRSubType,sizeof(TxnReqOut->ucCAVRSubType),APIOut->unVARSubType);//VAR Sub Type
	memset(APIOut->unVARTxnAmt,0x30,sizeof(APIOut->unVARTxnAmt));//VAR Txn Amt
	fnUnPack(TxnReqOut->ucCAVRTxnAmt,sizeof(TxnReqOut->ucCAVRTxnAmt),APIOut->unVARTxnAmt);//VAR Txn Amt
	memset(APIOut->unVAREV,0x30,sizeof(APIOut->unVAREV));//VAR EV
	fnUnPack(TxnReqOut->ucCAVREV,sizeof(TxnReqOut->ucCAVREV),APIOut->unVAREV);//VAR EV
	memset(APIOut->unVARCardID,0x30,sizeof(APIOut->unVARCardID));//VAR Card ID
	fnUnPack(TxnReqOut->ucCardID_4,sizeof(TxnReqOut->ucCardID_4),APIOut->unVARCardID);//VAR Card ID
	fnUnPack(&TxnReqOut->ucIssuerCode,sizeof(TxnReqOut->ucIssuerCode),APIOut->unVARIssuerCode);//VAR Issuer Code
	memset(APIOut->unVARCPUSPID,0x30,sizeof(APIOut->unVARCPUSPID));//VAR CPU SP ID
	fnUnPack(&TxnReqOut->ucCAVRSPID,sizeof(TxnReqOut->ucCAVRSPID),APIOut->unVARCPUSPID);//VAR CPU SP ID
	memset(APIOut->unVARCPULocationID,0x30,sizeof(APIOut->unVARCPULocationID));//VAR CPU Location ID
	fnUnPack(&TxnReqOut->ucCAVRLocationID,sizeof(TxnReqOut->ucCAVRLocationID),APIOut->unVARCPULocationID);//VAR CPU Location ID
	memset(APIOut->unVARPID,0x30,sizeof(APIOut->unVARPID));//VAR Purse ID
	memset(APIOut->unVARCPUDeviceID,0x30,sizeof(APIOut->unVARCPUDeviceID));//VAR CPU Device ID
	fnUnPack(TxnReqOut->ucCAVRDeviceID,sizeof(TxnReqOut->ucCAVRDeviceID),APIOut->unVARCPUDeviceID);//VAR CPU Device ID
	fnUnPack(TxnReqOut->ucLoyaltyCounter,sizeof(TxnReqOut->ucLoyaltyCounter),APIOut->unVARLoyaltyCounter);//VAR Loyalty Counter
	fnUnPack(&TxnReqOut->ucPersonalProfile,sizeof(TxnReqOut->ucPersonalProfile),APIOut->unVARPersonalProfile);//VAR Personal Profile
	fnUnPack(&TxnReqOut->ucBankCode,sizeof(TxnReqOut->ucBankCode),APIOut->unVARBankCode);//VAR Bank Code
	sprintf((char *)APIOut->unVARCardIDLen,"04");//VAR Card ID Len

	fnUnPack(TxnReqOut->ucDeviceID,sizeof(TxnReqOut->ucDeviceID),APIOut->unVAR2DeviceID);//VAR Device ID
	fnUnPack(&TxnReqOut->ucDeviceID[2],1,APIOut->unVAR2SPID);//VAR SP ID
	sprintf((char *)APIOut->unVAR2LocationID,"01");//VAR SP ID
	fnUnPack(TxnReqOut->ucCPUDeviceID,sizeof(TxnReqOut->ucCPUDeviceID),APIOut->unVAR2CPUDeviceID);//VAR CPU Device ID
	fnUnPack(&TxnReqOut->ucCPUDeviceID[3],3,APIOut->unVAR2CPUSPID);//VAR CPU SP ID
	memset(APIOut->unVAR2CPULocationID,0x30,sizeof(APIOut->unVAR2CPULocationID));//VAR CPU Location ID

	return(inTMOutLen);
}

int inBuildTxnReqOnlineAutoload(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
TxnReqOnline_TM_Out_5 *APIOut = (TxnReqOnline_TM_Out_5 *)API_Out;
TxnReqOnline_APDU_In *DongleIn = (TxnReqOnline_APDU_In *)Dongle_In;
TxnReqOnline_APDU_Out *DongleOut = (TxnReqOnline_APDU_Out *)Dongle_Out;

	inTMOutLen = sizeof(TxnReqOnline_TM_Out_5);
	memset(APIOut,0x00,inTMOutLen);
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",inTMOutLen - 7);//Data Len
	vdBuildProcessingCode(inTxnType,DongleOut->ucMsgType,DongleOut->ucSubType,DongleOut->ucPersonalProfile,APIOut->ucProcessignCode);//Processing Code
	vdBuildBLCVersion(APIOut->ucBLVersion);
	//fnASCTOBIN(APIOut->ucBLVersion,gsBlackListVersion,5,sizeof(APIOut->ucBLVersion),DECIMAL);//Black List
	APIOut->ucMsgType = DongleOut->ucMsgType;//Msg Type
	APIOut->ucSubType = DongleOut->ucSubType;//Sub Type
	memcpy(APIOut->ucDeviceID,DongleOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	APIOut->ucSPID = DongleOut->ucSPID;//SP ID
	memcpy(APIOut->ucTxnDateTime,DongleIn->ucTxnDateTime,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	APIOut->ucCardType = DongleOut->ucCardType;//Card Type
	APIOut->ucPersonalProfile = DongleOut->ucPersonalProfile;//Personal Profile
	APIOut->ucIssuerCode = DongleOut->ucIssuerCode;//Issuer Code
	APIOut->ucLocationID = DongleOut->ucLocationID;//Location ID
	APIOut->ucBankCode = DongleOut->ucBankCode;//Bank Code
	memcpy(APIOut->ucLoyaltyCounter,DongleOut->ucLoyaltyCounter,sizeof(APIOut->ucLoyaltyCounter));//Loyalty Counter
	memcpy(APIOut->ucExpiryDate,DongleOut->ucExpiryDate,sizeof(APIOut->ucExpiryDate));//Expiry Date
	APIOut->ucAreaCode = DongleOut->ucAreaCode;//Area Code
	memcpy(APIOut->ucTMID,DongleIn->ucTMID,sizeof(APIOut->ucTMID));//TM ID
	memcpy(APIOut->ucTMTxnDateTime,DongleIn->ucTMTxnDateTime,sizeof(APIOut->ucTMTxnDateTime));//TM Txn Date Time
	memcpy(APIOut->ucTMSerialNumber,DongleOut->ucTMSerialNumber,sizeof(APIOut->ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->ucTMAgentNumber,DongleIn->ucTMAgentNumber,sizeof(APIOut->ucTMAgentNumber));//TM Agent Number
	memcpy(APIOut->ucSTAC,DongleOut->ucSTAC,sizeof(APIOut->ucSTAC));//STAC
	APIOut->ucKeyVersion = DongleOut->ucSAMKVN;//SAM KVN
	memcpy(APIOut->ucSAMID,DongleOut->ucSAMID,sizeof(APIOut->ucSAMID));//SAM ID
	memcpy(APIOut->ucSAMSN,DongleOut->ucSAMSN,sizeof(APIOut->ucSAMSN));//SAM SN
	memcpy(APIOut->ucSAMCRN,DongleOut->ucSAMCRN,sizeof(APIOut->ucSAMCRN));//SAM CRN
	memcpy(APIOut->ucReaderFWVersion,DongleOut->ucReaderFWVersion,sizeof(APIOut->ucReaderFWVersion));//Reader FW Version
	if (srTxnData.srIngData.inTransType == TXN_ECC_AUTOLOAD)
		memcpy(APIOut->ucECCAmt, (BYTE *)"\x00\x00\x00", sizeof(APIOut->ucECCAmt)); // ECC Amt
	else
                                memcpy(APIOut->ucECCAmt, (BYTE *)&srTxnData.srIngData.lnECCAmt, sizeof(APIOut->ucECCAmt)); // ECC Amt
	memcpy(APIOut->ucCardID,DongleOut->ucCardID,sizeof(APIOut->ucCardID));//Card ID
	memcpy(APIOut->ucTxnAmt,DongleOut->ucTxnAmt,sizeof(APIOut->ucTxnAmt));//Txn Amt
	memcpy(APIOut->ucCPUDeviceID,DongleOut->ucCPUDeviceID,sizeof(APIOut->ucCPUDeviceID));//CPU Device ID
	memcpy(APIOut->ucCPUSPID,DongleOut->ucCPUSPID,sizeof(APIOut->ucCPUSPID));//CPU SP ID
	memcpy(APIOut->ucCPULocationID,DongleOut->ucCPULocationID,sizeof(APIOut->ucCPULocationID));//CPU Location ID
	memcpy(APIOut->ucPID,DongleOut->ucPID,sizeof(APIOut->ucPID));//CPU Purse ID
	memcpy(APIOut->ucCTC,DongleOut->ucCTC,sizeof(APIOut->ucCTC));//CPU CTC
	memcpy(APIOut->ucProfileExpiryDate,DongleOut->ucProfileExpiryDate,sizeof(APIOut->ucProfileExpiryDate));//Profile Expiry Date
	memcpy(APIOut->ucSubAreaCode,DongleOut->ucSubAreaCode,sizeof(APIOut->ucSubAreaCode));//Sub Area Code
	memcpy(APIOut->ucTxnSN,DongleOut->ucTxnSN,sizeof(APIOut->ucTxnSN));//Txn SN
	memcpy(APIOut->ucEV,DongleOut->ucEV,sizeof(APIOut->ucEV));//EV
	memcpy(APIOut->ucDeposit,DongleOut->ucDeposit,sizeof(APIOut->ucDeposit));//Deposit
	APIOut->ucCPUAdminKeyKVN = DongleOut->ucCPUAdminKeyKVN;//CPU Admin Key KVN
	APIOut->ucCreditKeyKVN = DongleOut->ucCreditKeyKVN;//Credit Key KVN
	APIOut->ucCPUIssuerKeyKVN = DongleOut->ucCPUIssuerKeyKVN;//CPU Issuer Key KVN
	APIOut->ucTxnMode = DongleOut->ucTxnMode;//Txn Mode
	APIOut->ucTxnQuqlifier = DongleOut->ucTxnQuqlifier;//Txn Quqlifier
	APIOut->ucSignatureKeyKVN = DongleOut->ucSignatureKeyKVN;//Signature Key KVN
	memcpy(APIOut->ucCPUSAMID,DongleOut->ucCPUSAMID,sizeof(APIOut->ucCPUSAMID));//CPU SAM ID
	APIOut->ucHostAdminKVN = DongleOut->ucHostAdminKVN;//Host Admin KVN
	APIOut->ucPurseVersionNumber = DongleOut->ucPurseVersionNumber;//Purse Version Number
	memcpy(APIOut->ucTMLocationID,DongleIn->ucTMLocationID,sizeof(APIOut->ucTMLocationID));//TM Location ID
	memcpy(APIOut->ucCACrypto,DongleOut->ucTxnCrypto,8);//CACrypto前8碼
	memset(&APIOut->ucCACrypto[8],0x00,8);//CACrypto後8碼
	vdBuildCardAVRData(&APIOut->stCardAVRInfo_t,&DongleOut->stLastCreditTxnLogInfo_t,DongleOut->ucCardID,DongleOut->ucIssuerCode,DongleOut->ucCPUSPID,DongleOut->ucCPULocationID,DongleOut->ucPID,APIOut->ucLoyaltyCounter);//Card AVR Info
	vdBuildReaderAVRData(&APIOut->stReaderAVRInfo_t,&DongleOut->stReaderAVRInfo_t);//Reader AVR Info
	//memcpy(&APIOut->stReaderAVRInfo_t,&DongleOut->stReaderAVRInfo_t,sizeof(APIOut->stReaderAVRInfo_t));//Reader AVR Info
	APIOut->ucCardIDLen = DongleOut->ucCardIDLen;//Card ID Len

	// 二十年展期開關
	int iReaderVer = ((int)(DongleOut->ucReaderFWVersion[0] & 0x0F) * 100) + (int)DongleOut->ucReaderFWVersion[1];
	log_msg(LOG_LEVEL_ERROR,"ReaderFWVersion: %d", iReaderVer);
	if ((srTxnData.srParameter.inSetCardValidDate == 1) && (srTxnData.srParameter.chOnlineFlag == R6_ONLINE) && (iReaderVer >= 810))
	{
		APIOut->ucCardIDLen |= 0x80;
	}

	memcpy(stStore_DataInfo_t.ucTMSerialNumber,DongleOut->ucTMSerialNumber,sizeof(stStore_DataInfo_t.ucTMSerialNumber));

	return(inTMOutLen);
}


int inBuildAutoloadEnableData2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
AutoloadEnable2_TM_Out *APIOut = (AutoloadEnable2_TM_Out *)API_Out;
TxnReqOnline_APDU_In *DongleIn = (TxnReqOnline_APDU_In *)Dongle_In;
TxnReqOnline_APDU_Out *DongleOut = (TxnReqOnline_APDU_Out *)Dongle_Out;

	inTMOutLen = sizeof(AutoloadEnable2_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",inTMOutLen - 7);//Data Len
	vdBuildProcessingCode(inTxnType,DongleOut->ucMsgType,DongleOut->ucSubType,DongleOut->ucPersonalProfile,APIOut->ucProcessignCode);//Processing Code
	vdBuildBLCVersion(APIOut->ucBLVersion);
	//fnASCTOBIN(APIOut->ucBLVersion,gsBlackListVersion,5,sizeof(APIOut->ucBLVersion),DECIMAL);//Black List
	APIOut->ucMsgType = DongleOut->ucMsgType;//Msg Type
	APIOut->ucSubType = DongleOut->ucSubType;//Sub Type
	memcpy(APIOut->ucDeviceID,DongleOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	APIOut->ucSPID = DongleOut->ucSPID;//SP ID
	memcpy(APIOut->ucTxnDateTime,DongleIn->ucTxnDateTime,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	APIOut->ucCardType = DongleOut->ucCardType;//Card Type
	APIOut->ucPersonalProfile = DongleOut->ucPersonalProfile;//Personal Profile
	APIOut->ucIssuerCode = DongleOut->ucIssuerCode;//Issuer Code
	APIOut->ucLocationID = DongleOut->ucLocationID;//Location ID
	APIOut->ucBankCode = DongleOut->ucBankCode;//Bank Code
	memcpy(APIOut->ucLoyaltyCounter,DongleOut->ucLoyaltyCounter,sizeof(APIOut->ucLoyaltyCounter));//Loyalty Counter
	memcpy(APIOut->ucExpiryDate,DongleOut->ucExpiryDate,sizeof(APIOut->ucExpiryDate));//Expiry Date
	APIOut->ucAreaCode = DongleOut->ucAreaCode;//Area Code
	memcpy(APIOut->ucTMID,DongleIn->ucTMID,sizeof(APIOut->ucTMID));//TM ID
	memcpy(APIOut->ucTMTxnDateTime,DongleIn->ucTMTxnDateTime,sizeof(APIOut->ucTMTxnDateTime));//TM Txn Date Time
	memcpy(APIOut->ucTMSerialNumber,DongleOut->ucTMSerialNumber,sizeof(APIOut->ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->ucTMAgentNumber,DongleIn->ucTMAgentNumber,sizeof(APIOut->ucTMAgentNumber));//TM Agent Number
	memcpy(APIOut->ucSTAC,DongleOut->ucSTAC,sizeof(APIOut->ucSTAC));//STAC
	APIOut->ucKeyVersion = DongleOut->ucSAMKVN;//SAM KVN
	memcpy(APIOut->ucSAMID,DongleOut->ucSAMID,sizeof(APIOut->ucSAMID));//SAM ID
	memcpy(APIOut->ucSAMSN,DongleOut->ucSAMSN,sizeof(APIOut->ucSAMSN));//SAM SN
	memcpy(APIOut->ucSAMCRN,DongleOut->ucSAMCRN,sizeof(APIOut->ucSAMCRN));//SAM CRN
	memcpy(APIOut->ucReaderFWVersion,DongleOut->ucReaderFWVersion,sizeof(APIOut->ucReaderFWVersion));//Reader FW Version
	APIOut->ucAutoLoadFlag = DongleOut->bAutoLoad;//Auto Load Flag
	memcpy(APIOut->ucAutoLoadAmt_2,DongleOut->ucAutoLoadAmt,sizeof(APIOut->ucAutoLoadAmt_2));//Auto Load Amt
	memcpy(APIOut->ucCardID,DongleOut->ucCardID,sizeof(APIOut->ucCardID));//Card ID
	memcpy(APIOut->ucTxnAmt,DongleOut->ucTxnAmt,sizeof(APIOut->ucTxnAmt));//Txn Amt
	memcpy(APIOut->ucCPUDeviceID,DongleOut->ucCPUDeviceID,sizeof(APIOut->ucCPUDeviceID));//CPU Device ID
	memcpy(APIOut->ucCPUSPID,DongleOut->ucCPUSPID,sizeof(APIOut->ucCPUSPID));//CPU SP ID
	memcpy(APIOut->ucCPULocationID,DongleOut->ucCPULocationID,sizeof(APIOut->ucCPULocationID));//CPU Location ID
	memcpy(APIOut->ucPID,DongleOut->ucPID,sizeof(APIOut->ucPID));//CPU Purse ID
	memcpy(APIOut->ucCTC,DongleOut->ucCTC,sizeof(APIOut->ucCTC));//CPU CTC
	memcpy(APIOut->ucProfileExpiryDate,DongleOut->ucProfileExpiryDate,sizeof(APIOut->ucProfileExpiryDate));//Profile Expiry Date
	memcpy(APIOut->ucSubAreaCode,DongleOut->ucSubAreaCode,sizeof(APIOut->ucSubAreaCode));//Sub Area Code
	memcpy(APIOut->ucTxnSN,DongleOut->ucTxnSN,sizeof(APIOut->ucTxnSN));//Txn SN
	memcpy(APIOut->ucEV,DongleOut->ucEV,sizeof(APIOut->ucEV));//EV
	memcpy(APIOut->ucDeposit,DongleOut->ucDeposit,sizeof(APIOut->ucDeposit));//Deposit
	APIOut->ucCPUAdminKeyKVN = DongleOut->ucCPUAdminKeyKVN;//CPU Admin Key KVN
	APIOut->ucCreditKeyKVN = DongleOut->ucCreditKeyKVN;//Credit Key KVN
	APIOut->ucCPUIssuerKeyKVN = DongleOut->ucCPUIssuerKeyKVN;//CPU Issuer Key KVN
	APIOut->ucTxnMode = DongleOut->ucTxnMode;//Txn Mode
	APIOut->ucTxnQuqlifier = DongleOut->ucTxnQuqlifier;//Txn Quqlifier
	APIOut->ucSignatureKeyKVN = DongleOut->ucSignatureKeyKVN;//Signature Key KVN
	memcpy(APIOut->ucCPUSAMID,DongleOut->ucCPUSAMID,sizeof(APIOut->ucCPUSAMID));//CPU SAM ID
	APIOut->ucHostAdminKVN = DongleOut->ucHostAdminKVN;//Host Admin KVN
	APIOut->ucPurseVersionNumber = DongleOut->ucPurseVersionNumber;//Purse Version Number
	memcpy(APIOut->ucTMLocationID,DongleIn->ucTMLocationID,sizeof(APIOut->ucTMLocationID));//TM Location ID
	memcpy(APIOut->ucCACrypto,DongleOut->ucTxnCrypto,8);//CACrypto前8碼
	memset(&APIOut->ucCACrypto[8],0x00,8);//CACrypto後8碼
	vdBuildCardAVRData(&APIOut->stCardAVRInfo_t,&DongleOut->stLastCreditTxnLogInfo_t,DongleOut->ucCardID,DongleOut->ucIssuerCode,DongleOut->ucCPUSPID,DongleOut->ucCPULocationID,DongleOut->ucPID,APIOut->ucLoyaltyCounter);//Card AVR Info
	vdBuildReaderAVRData(&APIOut->stReaderAVRInfo_t,&DongleOut->stReaderAVRInfo_t);//Reader AVR Info
	//memcpy(&APIOut->stReaderAVRInfo_t,&DongleOut->stReaderAVRInfo_t,sizeof(APIOut->stReaderAVRInfo_t));//Reader AVR Info
	APIOut->ucCardIDLen = DongleOut->ucCardIDLen;//Card ID Len

	memcpy(stStore_DataInfo_t.ucTMSerialNumber,DongleOut->ucTMSerialNumber,sizeof(stStore_DataInfo_t.ucTMSerialNumber));

	return(inTMOutLen);
}

int inBuildAuthAutoloadEnableData2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
AuthAutoloadEnable2_TM_Out *APIOut = (AuthAutoloadEnable2_TM_Out *)API_Out;
AuthTxnOnline_APDU_In *DongleIn = (AuthTxnOnline_APDU_In *)Dongle_In;
AuthTxnOnline_APDU_Out *DongleOut = (AuthTxnOnline_APDU_Out *)Dongle_Out;
TxnReqOnline_APDU_Out *TxnReqOut = (TxnReqOnline_APDU_Out *)Dongle_ReqOut;

	inTMOutLen = sizeof(AuthAutoloadEnable2_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	vdUIntToAsc(TxnReqOut->ucCardID,sizeof(TxnReqOut->ucCardID),APIOut->stSTMC_t.anCardID,sizeof(APIOut->stSTMC_t.anCardID),FALSE,' ',10);//n_Card Physical ID
	fnBINTODEVASC(TxnReqOut->ucDeviceID,APIOut->stSTMC_t.anDeviceID,sizeof(APIOut->stSTMC_t.anDeviceID),MIFARE);//n_TXN Device ID
	fnBINTODEVASC(TxnReqOut->ucCPUDeviceID,APIOut->stSTMC_t.anCPUDeviceID,sizeof(APIOut->stSTMC_t.anCPUDeviceID),LEVEL2);//n_TXN Device ID
	vdCountEVBefore(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.anEVBeforeTxn,sizeof(APIOut->stSTMC_t.anEVBeforeTxn),TxnReqOut->ucMsgType);//n_EV Before TXN
	vdUIntToAsc(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),APIOut->stSTMC_t.anTxnSN,sizeof(APIOut->stSTMC_t.anTxnSN),FALSE,' ',10);//n_TXN SNum
	vdUIntToAsc(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),APIOut->stSTMC_t.anTxnAmt,sizeof(APIOut->stSTMC_t.anTxnAmt),FALSE,' ',10);//n_TXN AMT
	vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.anEV,sizeof(APIOut->stSTMC_t.anEV),FALSE,' ',10);//n_EV
	vdBuildProcessingCode(inTxnType,TxnReqOut->ucMsgType,TxnReqOut->ucSubType,TxnReqOut->ucPersonalProfile,APIOut->stSTMC_t.anProcessignCode);//Processing Code
	fnUnPack(&TxnReqOut->ucMsgType,sizeof(TxnReqOut->ucMsgType),APIOut->stSTMC_t.unMsgType);//Msg Type
	fnUnPack(&TxnReqOut->ucSubType,sizeof(TxnReqOut->ucSubType),APIOut->stSTMC_t.unSubType);//Sub Type
	fnUnPack(TxnReqOut->ucDeviceID,sizeof(TxnReqOut->ucDeviceID),APIOut->stSTMC_t.unDeviceID);//Device ID
	fnUnPack(&TxnReqOut->ucSPID,sizeof(TxnReqOut->ucSPID),APIOut->stSTMC_t.unSPID);//SP ID
	fnUnPack(DongleOut->ucTxnDateTime,sizeof(DongleOut->ucTxnDateTime),APIOut->stSTMC_t.unTxnDateTime);//Txn Date Time
	fnUnPack(&TxnReqOut->ucCardType,sizeof(TxnReqOut->ucCardType),APIOut->stSTMC_t.unCardType);//Card Type
	fnUnPack(&TxnReqOut->ucPersonalProfile,sizeof(TxnReqOut->ucPersonalProfile),APIOut->stSTMC_t.unPersonalProfile);//Personal Profile
	fnUnPack(&TxnReqOut->ucLocationID,sizeof(TxnReqOut->ucLocationID),APIOut->stSTMC_t.unLocationID);//Location ID
	fnUnPack(TxnReqOut->ucCardID,sizeof(TxnReqOut->ucCardID),APIOut->stSTMC_t.unCardID);//CPU Card ID
	fnUnPack(&TxnReqOut->ucIssuerCode,sizeof(TxnReqOut->ucIssuerCode),APIOut->stSTMC_t.unIssuerCode);//Issuer Code
	fnUnPack(&TxnReqOut->ucBankCode,sizeof(TxnReqOut->ucBankCode),APIOut->stSTMC_t.unBankCode);//Bank Code
	fnUnPack(TxnReqOut->ucLoyaltyCounter,sizeof(TxnReqOut->ucLoyaltyCounter),APIOut->stSTMC_t.unLoyaltyCounter);//Loyalty Counter
	if(TxnReqOut->ucPurseVersionNumber == MIFARE)
	{
		if (TxnReqOut->ucReaderFWVersion[5] >= 0x80) //match sam的mfrc改用device id
			memcpy(&DongleOut->ucMAC_HCrypto[6], TxnReqOut->ucDeviceID, 4);
		fnUnPack(DongleOut->ucMAC_HCrypto,10,APIOut->stSTMC_t.unMAC);//MAC
		// 配對SAM用Device id
		//if (TxnReqOut->ucReaderFWVersion[5] >= 0x80)
		//	fnUnPack(TxnReqOut->ucDeviceID, sizeof(TxnReqOut->ucDeviceID), &APIOut->stSTMC_t.unMAC[12]);
	}
	else
		memset(APIOut->stSTMC_t.unMAC,0x30,sizeof(APIOut->stSTMC_t.unMAC));//MAC
	fnUnPack(TxnReqOut->ucExpiryDate,sizeof(TxnReqOut->ucExpiryDate),APIOut->stSTMC_t.unOrgExpiryDate);//Org Expiry Date
	fnUnPack(DongleIn->ucExpiryDate,sizeof(DongleIn->ucExpiryDate),APIOut->stSTMC_t.unNewExpiryDate);//New Expiry Date
	fnUnPack(&TxnReqOut->ucAreaCode,sizeof(TxnReqOut->ucAreaCode),APIOut->stSTMC_t.unAreaCode);//Area Code
	fnUnPack(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),APIOut->stSTMC_t.unTxnAmt);//Txn Amt
	fnUnPack(DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.unEV);//EV
	fnUnPack(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),APIOut->stSTMC_t.unTxnSN);//Txn SN
	fnUnPack(TxnReqOut->ucCPUDeviceID,sizeof(TxnReqOut->ucCPUDeviceID),APIOut->stSTMC_t.unCPUDeviceID);//CPU Device ID
	fnUnPack(TxnReqOut->ucCPUSPID,sizeof(TxnReqOut->ucCPUSPID),APIOut->stSTMC_t.unCPUSPID);//CPU SP ID
	fnUnPack(TxnReqOut->ucCPULocationID,sizeof(TxnReqOut->ucCPULocationID),APIOut->stSTMC_t.unCPULocationID);//CPU Location ID
	fnUnPack(TxnReqOut->ucPID,sizeof(TxnReqOut->ucPID),APIOut->stSTMC_t.unPID);//Purse ID
	fnUnPack(TxnReqOut->ucCTC,sizeof(TxnReqOut->ucCTC),APIOut->stSTMC_t.unCTC);//CTC
	fnUnPack(TxnReqOut->ucProfileExpiryDate,sizeof(TxnReqOut->ucProfileExpiryDate),APIOut->stSTMC_t.unOrgProfileExpiryDate);//Org Profile Expiry Date
	fnUnPack(DongleIn->ucProfileExpiryDate,sizeof(DongleIn->ucProfileExpiryDate),APIOut->stSTMC_t.unNewProfileExpiryDate);//New Profile Expiry Date
	fnUnPack(TxnReqOut->ucSubAreaCode,sizeof(TxnReqOut->ucSubAreaCode),APIOut->stSTMC_t.unSubAreaCode);//Sub Area Code
	fnUnPack(TxnReqOut->ucTxnSN,sizeof(TxnReqOut->ucTxnSN),APIOut->stSTMC_t.unTxnSNBeforeTxn);//Txn SN Before Txn
	fnUnPack(TxnReqOut->ucEV,sizeof(TxnReqOut->ucEV),APIOut->stSTMC_t.unEVBeforeTxn);//EV Before Txn
	fnUnPack(TxnReqOut->ucDeposit,sizeof(TxnReqOut->ucDeposit),APIOut->stSTMC_t.unDeposit);//Deposit
	fnUnPack(&TxnReqOut->ucTxnMode,sizeof(TxnReqOut->ucTxnMode),APIOut->stSTMC_t.unTxnMode);//Txn Mode
	fnUnPack(&TxnReqOut->ucTxnQuqlifier,sizeof(TxnReqOut->ucTxnQuqlifier),APIOut->stSTMC_t.unTxnQuqlifier);//Txn Quqlifier
	fnUnPack(&TxnReqOut->ucSignatureKeyKVN,sizeof(TxnReqOut->ucSignatureKeyKVN),APIOut->stSTMC_t.unSignatureKeyKVN);//Signature Key KVN
	fnUnPack(DongleOut->ucSignature,sizeof(DongleOut->ucSignature),APIOut->stSTMC_t.unSignature);//Signature
	fnUnPack(DongleOut->ucCPUSAMID,sizeof(DongleOut->ucCPUSAMID),APIOut->stSTMC_t.unCPUSAMID);//SAM ID
	if(TxnReqOut->ucPurseVersionNumber != MIFARE)
		fnUnPack(&DongleOut->ucMAC_HCrypto[0],1,APIOut->stSTMC_t.unHashType);//Hash Type
	else
		memset(APIOut->stSTMC_t.unHashType,0x30,sizeof(APIOut->stSTMC_t.unHashType));
	if(TxnReqOut->ucPurseVersionNumber != MIFARE)
	{
		fnUnPack(&DongleOut->ucMAC_HCrypto[1],1,APIOut->stSTMC_t.unHostAdminKVN);//Host Admin KVN
		fnUnPack(&DongleOut->ucMAC_HCrypto[2],16,APIOut->stSTMC_t.unCPUMAC);//MAC
	}
	else
	{
		memset(APIOut->stSTMC_t.unHostAdminKVN,0x30,sizeof(APIOut->stSTMC_t.unHostAdminKVN));
		memset(APIOut->stSTMC_t.unCPUMAC,0x30,sizeof(APIOut->stSTMC_t.unCPUMAC));//MAC
	}
	fnUnPack(&TxnReqOut->ucPurseVersionNumber,sizeof(TxnReqOut->ucPurseVersionNumber),APIOut->stSTMC_t.unPurseVersionNumber);//Purse Version Number
	fnUnPack(&TxnReqOut->ucCardIDLen,sizeof(TxnReqOut->ucCardIDLen),APIOut->stSTMC_t.unCardIDLen);//Card ID Len
	memcpy(APIOut->stSTMC_t.ucTMLocationID,stStore_DataInfo_t.ucTMLocationID,sizeof(stStore_DataInfo_t.ucTMLocationID));//TM Location ID
	memcpy(APIOut->stSTMC_t.ucTMID,stStore_DataInfo_t.ucTMID,sizeof(stStore_DataInfo_t.ucTMID));//TM ID
	UnixToDateTime((BYTE*)DongleOut->ucTxnDateTime,(BYTE*)APIOut->stSTMC_t.ucTMTxnDateTime,14);//TM Txn Date Time
	memcpy(APIOut->stSTMC_t.ucTMSerialNumber,stStore_DataInfo_t.ucTMSerialNumber,sizeof(stStore_DataInfo_t.ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->stSTMC_t.ucTMAgentNumber,stStore_DataInfo_t.ucTMAgentNumber,sizeof(stStore_DataInfo_t.ucTMAgentNumber));//TM Agent Number
	//帳務資料結束
	//Confirm資料開始
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",79);//Data Len
	vdBuildProcessingCode(inTxnType,TxnReqOut->ucMsgType,TxnReqOut->ucSubType,TxnReqOut->ucPersonalProfile,APIOut->ucProcessignCode);//Processing Code
	APIOut->ucMsgType = TxnReqOut->ucMsgType;//Msg Type
	APIOut->ucSubType = TxnReqOut->ucSubType;//Sub Type
	memcpy(APIOut->ucDeviceID,TxnReqOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	memcpy(APIOut->ucTxnDateTime,DongleOut->ucTxnDateTime,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	if(TxnReqOut->ucPurseVersionNumber == MIFARE)
		memcpy(APIOut->ucMAC,DongleOut->ucMAC_HCrypto,sizeof(APIOut->ucMAC));//MAC
	else
		memset(APIOut->ucMAC,0x00,sizeof(APIOut->ucMAC));//MAC
	memcpy(APIOut->ucConfirmCode,DongleOut->ucConfirmCode,sizeof(APIOut->ucConfirmCode));//Confirm Code
	if(TxnReqOut->ucPurseVersionNumber == MIFARE)
		memcpy(APIOut->ucCTAC,&DongleOut->ucMAC_HCrypto[10],sizeof(APIOut->ucMAC));//CTAC
	else
		memset(APIOut->ucCTAC,0x00,sizeof(APIOut->ucMAC));//CTAC
	APIOut->ucAutoLoadFlag = 0x01;//AutoLoad Flag
	memcpy(APIOut->ucAutoLoadAmt_2,TxnReqOut->ucAutoLoadAmt,sizeof(APIOut->ucAutoLoadAmt_2));//AutoLoad Amt
	memcpy(APIOut->ucCardID,TxnReqOut->ucCardID,sizeof(APIOut->ucCardID));//Card ID
	memcpy(APIOut->ucEV,DongleOut->ucEV,sizeof(APIOut->ucEV));//EV
	memcpy(APIOut->ucTxnSN,DongleOut->ucTxnSN,sizeof(APIOut->ucTxnSN));//Txn SN
	memcpy(APIOut->ucCPUDeviceID,TxnReqOut->ucCPUDeviceID,sizeof(APIOut->ucCPUDeviceID));//CPU Device ID
	memcpy(APIOut->ucPID,TxnReqOut->ucPID,sizeof(APIOut->ucPID));//Purse ID
	memcpy(APIOut->ucSignature,DongleOut->ucSignature,sizeof(APIOut->ucSignature));//Signature
	if(TxnReqOut->ucPurseVersionNumber != MIFARE)
	{
		APIOut->ucHashType = DongleOut->ucMAC_HCrypto[0];//Hash Type
		APIOut->ucHostAdminKVN = DongleOut->ucMAC_HCrypto[1];//Host Admin KVN
		memcpy(APIOut->ucCPUMAC,&DongleOut->ucMAC_HCrypto[2],sizeof(APIOut->ucCPUMAC));//CPU MAC
	}
	else
	{
		APIOut->ucHashType = 0x00;
		APIOut->ucHostAdminKVN = 0x00;
		memset(APIOut->ucCPUMAC,0x00,sizeof(APIOut->ucCPUMAC));//CPU MAC
	}

	return(inTMOutLen);
}

int inBuildAuthAutoloadData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
AuthAutoload_TM_Out *APIOut = (AuthAutoload_TM_Out *)API_Out;
AuthTxnOnline_APDU_In *DongleIn = (AuthTxnOnline_APDU_In *)Dongle_In;
AuthTxnOnline_APDU_Out *DongleOut = (AuthTxnOnline_APDU_Out *)Dongle_Out;
TxnReqOnline_APDU_Out *TxnReqOut = (TxnReqOnline_APDU_Out *)Dongle_ReqOut;

	inTMOutLen = sizeof(AuthAutoload_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	vdUIntToAsc(TxnReqOut->ucCardID,sizeof(TxnReqOut->ucCardID),APIOut->stSTMC_t.anCardID,sizeof(APIOut->stSTMC_t.anCardID),FALSE,' ',10);//n_Card Physical ID
	fnBINTODEVASC(TxnReqOut->ucDeviceID,APIOut->stSTMC_t.anDeviceID,sizeof(APIOut->stSTMC_t.anDeviceID),MIFARE);//n_TXN Device ID
	fnBINTODEVASC(TxnReqOut->ucCPUDeviceID,APIOut->stSTMC_t.anCPUDeviceID,sizeof(APIOut->stSTMC_t.anCPUDeviceID),LEVEL2);//n_TXN Device ID
	vdCountEVBefore(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.anEVBeforeTxn,sizeof(APIOut->stSTMC_t.anEVBeforeTxn),TxnReqOut->ucMsgType);//n_EV Before TXN
	vdUIntToAsc(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),APIOut->stSTMC_t.anTxnSN,sizeof(APIOut->stSTMC_t.anTxnSN),FALSE,' ',10);//n_TXN SNum
	vdUIntToAsc(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),APIOut->stSTMC_t.anTxnAmt,sizeof(APIOut->stSTMC_t.anTxnAmt),FALSE,' ',10);//n_TXN AMT
	vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.anEV,sizeof(APIOut->stSTMC_t.anEV),FALSE,' ',10);//n_EV
	vdBuildProcessingCode(inTxnType,TxnReqOut->ucMsgType,TxnReqOut->ucSubType,TxnReqOut->ucPersonalProfile,APIOut->stSTMC_t.anProcessignCode);//Processing Code
	fnUnPack(&TxnReqOut->ucMsgType,sizeof(TxnReqOut->ucMsgType),APIOut->stSTMC_t.unMsgType);//Msg Type
	fnUnPack(&TxnReqOut->ucSubType,sizeof(TxnReqOut->ucSubType),APIOut->stSTMC_t.unSubType);//Sub Type
	fnUnPack(TxnReqOut->ucDeviceID,sizeof(TxnReqOut->ucDeviceID),APIOut->stSTMC_t.unDeviceID);//Device ID
	fnUnPack(&TxnReqOut->ucSPID,sizeof(TxnReqOut->ucSPID),APIOut->stSTMC_t.unSPID);//SP ID
	fnUnPack(DongleOut->ucTxnDateTime,sizeof(DongleOut->ucTxnDateTime),APIOut->stSTMC_t.unTxnDateTime);//Txn Date Time
	fnUnPack(&TxnReqOut->ucCardType,sizeof(TxnReqOut->ucCardType),APIOut->stSTMC_t.unCardType);//Card Type
	fnUnPack(&TxnReqOut->ucPersonalProfile,sizeof(TxnReqOut->ucPersonalProfile),APIOut->stSTMC_t.unPersonalProfile);//Personal Profile
	fnUnPack(&TxnReqOut->ucLocationID,sizeof(TxnReqOut->ucLocationID),APIOut->stSTMC_t.unLocationID);//Location ID
	fnUnPack(TxnReqOut->ucCardID,sizeof(TxnReqOut->ucCardID),APIOut->stSTMC_t.unCardID);//CPU Card ID
	fnUnPack(&TxnReqOut->ucIssuerCode,sizeof(TxnReqOut->ucIssuerCode),APIOut->stSTMC_t.unIssuerCode);//Issuer Code
	fnUnPack(&TxnReqOut->ucBankCode,sizeof(TxnReqOut->ucBankCode),APIOut->stSTMC_t.unBankCode);//Bank Code
	fnUnPack(TxnReqOut->ucLoyaltyCounter,sizeof(TxnReqOut->ucLoyaltyCounter),APIOut->stSTMC_t.unLoyaltyCounter);//Loyalty Counter
	if(TxnReqOut->ucPurseVersionNumber == MIFARE)
	{
		if (TxnReqOut->ucReaderFWVersion[5] >= 0x80) //match sam的mfrc改用device id
			memcpy(&DongleOut->ucMAC_HCrypto[6], TxnReqOut->ucDeviceID, 4);
		fnUnPack(DongleOut->ucMAC_HCrypto,10,APIOut->stSTMC_t.unMAC);//MAC
		// 配對SAM用Device id
		//if (TxnReqOut->ucReaderFWVersion[5] >= 0x80)
		//	fnUnPack(TxnReqOut->ucDeviceID, sizeof(TxnReqOut->ucDeviceID), &APIOut->stSTMC_t.unMAC[12]);
	}
	else
		memset(APIOut->stSTMC_t.unMAC,0x30,sizeof(APIOut->stSTMC_t.unMAC));//MAC
	fnUnPack(TxnReqOut->ucExpiryDate,sizeof(TxnReqOut->ucExpiryDate),APIOut->stSTMC_t.unOrgExpiryDate);//Org Expiry Date
	fnUnPack(DongleIn->ucExpiryDate,sizeof(DongleIn->ucExpiryDate),APIOut->stSTMC_t.unNewExpiryDate);//New Expiry Date
	fnUnPack(&TxnReqOut->ucAreaCode,sizeof(TxnReqOut->ucAreaCode),APIOut->stSTMC_t.unAreaCode);//Area Code
	fnUnPack(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),APIOut->stSTMC_t.unTxnAmt);//Txn Amt
	fnUnPack(DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.unEV);//EV
	fnUnPack(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),APIOut->stSTMC_t.unTxnSN);//Txn SN
	fnUnPack(TxnReqOut->ucCPUDeviceID,sizeof(TxnReqOut->ucCPUDeviceID),APIOut->stSTMC_t.unCPUDeviceID);//CPU Device ID
	fnUnPack(TxnReqOut->ucCPUSPID,sizeof(TxnReqOut->ucCPUSPID),APIOut->stSTMC_t.unCPUSPID);//CPU SP ID
	fnUnPack(TxnReqOut->ucCPULocationID,sizeof(TxnReqOut->ucCPULocationID),APIOut->stSTMC_t.unCPULocationID);//CPU Location ID
	fnUnPack(TxnReqOut->ucPID,sizeof(TxnReqOut->ucPID),APIOut->stSTMC_t.unPID);//Purse ID
	fnUnPack(TxnReqOut->ucCTC,sizeof(TxnReqOut->ucCTC),APIOut->stSTMC_t.unCTC);//CTC
	fnUnPack(TxnReqOut->ucProfileExpiryDate,sizeof(TxnReqOut->ucProfileExpiryDate),APIOut->stSTMC_t.unOrgProfileExpiryDate);//Org Profile Expiry Date
	fnUnPack(DongleIn->ucProfileExpiryDate,sizeof(DongleIn->ucProfileExpiryDate),APIOut->stSTMC_t.unNewProfileExpiryDate);//New Profile Expiry Date
	fnUnPack(TxnReqOut->ucSubAreaCode,sizeof(TxnReqOut->ucSubAreaCode),APIOut->stSTMC_t.unSubAreaCode);//Sub Area Code
	fnUnPack(TxnReqOut->ucTxnSN,sizeof(TxnReqOut->ucTxnSN),APIOut->stSTMC_t.unTxnSNBeforeTxn);//Txn SN Before Txn
	fnUnPack(TxnReqOut->ucEV,sizeof(TxnReqOut->ucEV),APIOut->stSTMC_t.unEVBeforeTxn);//EV Before Txn
	fnUnPack(TxnReqOut->ucDeposit,sizeof(TxnReqOut->ucDeposit),APIOut->stSTMC_t.unDeposit);//Deposit
	fnUnPack(&TxnReqOut->ucTxnMode,sizeof(TxnReqOut->ucTxnMode),APIOut->stSTMC_t.unTxnMode);//Txn Mode
	fnUnPack(&TxnReqOut->ucTxnQuqlifier,sizeof(TxnReqOut->ucTxnQuqlifier),APIOut->stSTMC_t.unTxnQuqlifier);//Txn Quqlifier
	fnUnPack(&TxnReqOut->ucSignatureKeyKVN,sizeof(TxnReqOut->ucSignatureKeyKVN),APIOut->stSTMC_t.unSignatureKeyKVN);//Signature Key KVN
	fnUnPack(DongleOut->ucSignature,sizeof(DongleOut->ucSignature),APIOut->stSTMC_t.unSignature);//Signature
	fnUnPack(DongleOut->ucCPUSAMID,sizeof(DongleOut->ucCPUSAMID),APIOut->stSTMC_t.unCPUSAMID);//SAM ID
	if(TxnReqOut->ucPurseVersionNumber != MIFARE)
		fnUnPack(&DongleOut->ucMAC_HCrypto[0],1,APIOut->stSTMC_t.unHashType);//Hash Type
	else
		memset(APIOut->stSTMC_t.unHashType,0x30,sizeof(APIOut->stSTMC_t.unHashType));
	if(TxnReqOut->ucPurseVersionNumber != MIFARE)
	{
		fnUnPack(&DongleOut->ucMAC_HCrypto[1],1,APIOut->stSTMC_t.unHostAdminKVN);//Host Admin KVN
		fnUnPack(&DongleOut->ucMAC_HCrypto[2],16,APIOut->stSTMC_t.unCPUMAC);//MAC
	}
	else
	{
		memset(APIOut->stSTMC_t.unHostAdminKVN,0x30,sizeof(APIOut->stSTMC_t.unHostAdminKVN));
		memset(APIOut->stSTMC_t.unCPUMAC,0x30,sizeof(APIOut->stSTMC_t.unCPUMAC));//MAC
	}
	fnUnPack(&TxnReqOut->ucPurseVersionNumber,sizeof(TxnReqOut->ucPurseVersionNumber),APIOut->stSTMC_t.unPurseVersionNumber);//Purse Version Number
	fnUnPack(&TxnReqOut->ucCardIDLen,sizeof(TxnReqOut->ucCardIDLen),APIOut->stSTMC_t.unCardIDLen);//Card ID Len
	memcpy(APIOut->stSTMC_t.ucTMLocationID,stStore_DataInfo_t.ucTMLocationID,sizeof(stStore_DataInfo_t.ucTMLocationID));//TM Location ID
	memcpy(APIOut->stSTMC_t.ucTMID,stStore_DataInfo_t.ucTMID,sizeof(stStore_DataInfo_t.ucTMID));//TM ID
	UnixToDateTime((BYTE*)DongleOut->ucTxnDateTime,(BYTE*)APIOut->stSTMC_t.ucTMTxnDateTime,14);//TM Txn Date Time
	memcpy(APIOut->stSTMC_t.ucTMSerialNumber,stStore_DataInfo_t.ucTMSerialNumber,sizeof(stStore_DataInfo_t.ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->stSTMC_t.ucTMAgentNumber,stStore_DataInfo_t.ucTMAgentNumber,sizeof(stStore_DataInfo_t.ucTMAgentNumber));//TM Agent Number
	//帳務資料結束
	//Confirm資料開始
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",79);//Data Len
	vdBuildProcessingCode(inTxnType,TxnReqOut->ucMsgType,TxnReqOut->ucSubType,TxnReqOut->ucPersonalProfile,APIOut->ucProcessignCode);//Processing Code
	APIOut->ucMsgType = TxnReqOut->ucMsgType;//Msg Type
	APIOut->ucSubType = TxnReqOut->ucSubType;//Sub Type
	memcpy(APIOut->ucDeviceID,TxnReqOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	memcpy(APIOut->ucTxnDateTime,DongleOut->ucTxnDateTime,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	if(TxnReqOut->ucPurseVersionNumber == MIFARE)
		memcpy(APIOut->ucMAC,DongleOut->ucMAC_HCrypto,sizeof(APIOut->ucMAC));//MAC
	else
		memset(APIOut->ucMAC,0x00,sizeof(APIOut->ucMAC));//MAC

	memcpy(APIOut->ucConfirmCode,DongleOut->ucConfirmCode,sizeof(APIOut->ucConfirmCode));//Confirm Code
	if(TxnReqOut->ucPurseVersionNumber == MIFARE)
		memcpy(APIOut->ucCTAC,&DongleOut->ucMAC_HCrypto[10],sizeof(APIOut->ucMAC));//CTAC
	else
		memset(APIOut->ucCTAC,0x00,sizeof(APIOut->ucMAC));//CTAC
	memcpy(APIOut->ucCardID,TxnReqOut->ucCardID,sizeof(APIOut->ucCardID));//Card ID
	memcpy(APIOut->ucTxnAmt,TxnReqOut->ucTxnAmt,sizeof(APIOut->ucTxnAmt));//Txn Amt
	memcpy(APIOut->ucEV,DongleOut->ucEV,sizeof(APIOut->ucEV));//EV
	memcpy(APIOut->ucTxnSN,DongleOut->ucTxnSN,sizeof(APIOut->ucTxnSN));//Txn SN
	memcpy(APIOut->ucCPUDeviceID,TxnReqOut->ucCPUDeviceID,sizeof(APIOut->ucCPUDeviceID));//CPU Device ID
	memcpy(APIOut->ucPID,TxnReqOut->ucPID,sizeof(APIOut->ucPID));//Purse ID
	memcpy(APIOut->ucSignature,DongleOut->ucSignature,sizeof(APIOut->ucSignature));//Signature
	if(TxnReqOut->ucPurseVersionNumber != MIFARE)
	{
		APIOut->ucHashType = DongleOut->ucMAC_HCrypto[0];//Hash Type
		APIOut->ucHostAdminKVN = DongleOut->ucMAC_HCrypto[1];//Host Admin KVN
		memcpy(APIOut->ucCPUMAC,&DongleOut->ucMAC_HCrypto[2],sizeof(APIOut->ucCPUMAC));//CPU MAC
	}
	else
	{
		APIOut->ucHashType = 0x00;
		APIOut->ucHostAdminKVN = 0x00;
		memset(APIOut->ucCPUMAC,0x00,sizeof(APIOut->ucCPUMAC));//CPU MAC
	}

	return(inTMOutLen);
}

int inBuildReadCardNumberData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
ReadCardNumber_TM_Out *APIOut = (ReadCardNumber_TM_Out *)API_Out;
ReadCardNumber_APDU_Out *DongleOut = (ReadCardNumber_APDU_Out *)Dongle_Out;

	inTMOutLen = sizeof(ReadCardNumber_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	vdUIntToAsc(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),APIOut->anCardID,sizeof(APIOut->anCardID),FALSE,' ',10);//n_Card Physical ID
	APIOut->ucCardClass = DongleOut->ucCardClass;

	return(inTMOutLen);
}

int inBuildStudentSetValueData1(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
TxnReqOnline_TM_Out_1 *APIOut = (TxnReqOnline_TM_Out_1 *)API_Out;
StudentSetValue_APDU_Out *DongleOut = (StudentSetValue_APDU_Out *)Dongle_Out;

    inTMOutLen = sizeof(TxnReqOnline_TM_Out_1);
    memset(APIOut,0x00,inTMOutLen);
    vdUIntToAsc(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),APIOut->anCardID,sizeof(APIOut->anCardID),FALSE,' ',10);//n_Card Physical ID
    fnBINTODEVASC(DongleOut->ucDeviceID,APIOut->anDeviceID,sizeof(APIOut->anDeviceID),MIFARE);//n_TXN Device ID
    fnBINTODEVASC(DongleOut->ucCPUDeviceID,APIOut->anCPUDeviceID,sizeof(APIOut->anCPUDeviceID),LEVEL2);//n_TXN Device ID
    vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->anEVBeforeTxn,sizeof(APIOut->anEVBeforeTxn),FALSE,' ',10);//n_EV Before TXN
    vdUIntToAsc(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),APIOut->anTxnSN,sizeof(APIOut->anTxnSN),TRUE,' ',10);//n_TXN SNum
    vdUIntToAsc(DongleOut->ucTxnAmt,sizeof(DongleOut->ucTxnAmt),APIOut->anTxnAmt,sizeof(APIOut->anTxnAmt),FALSE,' ',10);//n_TXN AMT
    APIOut->ucAutoLoad = DongleOut->bAutoLoad;//AutoLoad Flag
    vdUIntToAsc(DongleOut->ucAutoLoadAmt,sizeof(DongleOut->ucAutoLoadAmt),APIOut->anAutoLoadAmt,sizeof(APIOut->anAutoLoadAmt),FALSE,' ',10);//n_AutoLoad Amount

    return(inTMOutLen);
}

int inBuildStudentSetValueData2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
StudentSetValue_TM_Out *APIOut = (StudentSetValue_TM_Out *)API_Out;
StudentSetValue_APDU_In *DongleIn = (StudentSetValue_APDU_In *)Dongle_In;
StudentSetValue_APDU_Out *DongleOut = (StudentSetValue_APDU_Out *)Dongle_Out;

	inTMOutLen = sizeof(StudentSetValue_TM_Out);
    memset(APIOut,0x00,inTMOutLen);
    vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
    sprintf((char *)APIOut->ucDataLen,"%03d",inTMOutLen - 7);//Data Len

    vdBuildProcessingCode(inTxnType,DongleOut->ucMsgType,DongleOut->ucSubType,DongleOut->ucPersonalProfile,APIOut->ucProcessignCode);//Processing Code
    fnASCTOBIN(APIOut->ucBLVersion,&gsBlackListVersion[3],5,sizeof(APIOut->ucBLVersion),DECIMAL);//Black List
    APIOut->ucMsgType = DongleOut->ucMsgType;//Msg Type
    APIOut->ucSubType = DongleOut->ucSubType;//Sub Type
    memcpy(APIOut->ucDeviceID,DongleOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
    APIOut->ucSPID = DongleOut->ucSPID;//SP ID
    memcpy(APIOut->ucTxnDateTime,DongleIn->ucTxnDateTime,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
    APIOut->ucCardType = DongleOut->ucCardType;//Card Type
    APIOut->ucPersonalProfile = DongleOut->ucPersonalProfile;//Personal Profile
    APIOut->ucIssuerCode = DongleOut->ucIssuerCode;//Issuer Code
    APIOut->ucLocationID = DongleOut->ucLocationID;//Location ID
    if (DongleOut->ucPurseVersionNumber != MIFARE)
        APIOut->ucBankCode = DongleOut->ucBankCode;//Bank Code
    if (DongleOut->ucPurseVersionNumber == MIFARE)
        memcpy(APIOut->ucLoyaltyCounter,DongleOut->ucLoyaltyCounterOrSubAreaCode,sizeof(APIOut->ucLoyaltyCounter));//Loyalty Counter
    memcpy(APIOut->ucExpiryDate,DongleOut->ucExpiryDate,sizeof(APIOut->ucExpiryDate));//Expiry Date
    APIOut->ucAreaCode = DongleOut->ucAreaCode;//Area Code
    memcpy(APIOut->ucTMID,DongleIn->ucTMID,sizeof(APIOut->ucTMID));//TM ID
    memcpy(APIOut->ucTMTxnDateTime,DongleIn->ucTMTxnDateTime,sizeof(APIOut->ucTMTxnDateTime));//TM Txn Date Time
    memcpy(APIOut->ucTMSerialNumber,DongleOut->ucTMSerialNumber,sizeof(APIOut->ucTMSerialNumber));//TM Serial Number
    memcpy(APIOut->ucTMAgentNumber,DongleIn->ucTMAgentNumber,sizeof(APIOut->ucTMAgentNumber));//TM Agent Number
    if (DongleOut->ucPurseVersionNumber == MIFARE)
        memcpy(APIOut->ucSTAC,DongleOut->ucSID_STAC,sizeof(APIOut->ucSTAC));//STAC
    if (DongleOut->ucPurseVersionNumber == MIFARE)
        APIOut->ucKeyVersion = DongleOut->ucHost_SAMKVN;//SAM KVN
    if (DongleOut->ucPurseVersionNumber == MIFARE)
        memcpy(APIOut->ucSAMID,&DongleOut->ucSVCrypto_SAMCRN_SAMID[8],sizeof(APIOut->ucSAMID));//SAM ID
    else
        memcpy(APIOut->ucSAMID,DongleOut->ucSID_STAC,sizeof(APIOut->ucSAMID));//SAM ID
    //modify by bean 20141022 start 佳昌說要改的
    vdBuildF37_SAM_SN(DongleOut->ucPurseVersionNumber,APIOut->ucSAMSN,DongleOut->ucSAMSN,DongleOut->ucCardID);
    //if (DongleOut->ucPurseVersionNumber != LEVEL2)
    //    memcpy(APIOut->ucSAMSN,DongleOut->ucSAMSN,sizeof(APIOut->ucSAMSN));//SAM SN
    if (DongleOut->ucPurseVersionNumber == MIFARE)
        memcpy(APIOut->ucSAMCRN,DongleOut->ucSVCrypto_SAMCRN_SAMID,sizeof(APIOut->ucSAMCRN));//SAM CRN
    memcpy(APIOut->ucReaderFWVersion,DongleOut->ucReaderFWVersion,sizeof(APIOut->ucReaderFWVersion));//Reader FW Version
    memcpy(APIOut->ucCardID,DongleOut->ucCardID,sizeof(APIOut->ucCardID));//Card ID
    memcpy(APIOut->ucTxnAmt,DongleOut->ucTxnAmt,sizeof(APIOut->ucTxnAmt));//Txn Amt
    memcpy(APIOut->ucCPUDeviceID,DongleOut->ucCPUDeviceID,sizeof(APIOut->ucCPUDeviceID));//CPU Device ID
    memcpy(APIOut->ucCPUSPID,DongleOut->ucCPUSPID,sizeof(APIOut->ucCPUSPID));//CPU SP ID
    memcpy(APIOut->ucCPULocationID,DongleOut->ucCPULocationID,sizeof(APIOut->ucCPULocationID));//CPU Location ID
    memcpy(APIOut->ucPID,DongleOut->ucPID,sizeof(APIOut->ucPID));//CPU Purse ID
    //memcpy(APIOut->ucCTC,DongleOut->ucCTC,sizeof(APIOut->ucCTC));//CPU CTC
    //modify by bean 20141024 start 佳昌說要改的
    vdBuildF66_CTC(DongleOut->ucPurseVersionNumber, APIOut->ucCTC, DongleOut->ucCTC, DongleOut->ucDeviceID);
    //modify by bean 20141024 end
    memcpy(APIOut->ucProfileExpiryDate,DongleOut->ucProfileExpiryDate,sizeof(APIOut->ucProfileExpiryDate));//Profile Expiry Date
    if (DongleOut->ucPurseVersionNumber != MIFARE)
        memcpy(APIOut->ucSubAreaCode,DongleOut->ucLoyaltyCounterOrSubAreaCode,sizeof(APIOut->ucSubAreaCode));//Sub Area Code
    memcpy(APIOut->ucTxnSN,DongleOut->ucTxnSN,sizeof(APIOut->ucTxnSN));//Txn SN
    memcpy(APIOut->ucEV,DongleOut->ucEV,sizeof(APIOut->ucEV));//EV
    memcpy(APIOut->ucDeposit,DongleOut->ucDeposit,sizeof(APIOut->ucDeposit));//Deposit
    APIOut->ucCPUAdminKeyKVN = DongleOut->ucCPUAdminKeyKVN;//CPU Admin Key KVN
    APIOut->ucCreditKeyKVN = DongleOut->ucCreditKeyKVN;//Credit Key KVN
    APIOut->ucCPUIssuerKeyKVN = DongleOut->ucCPUIssuerKeyKVN;//CPU Issuer Key KVN
    APIOut->ucTxnMode = DongleOut->ucTxnMode;//Txn Mode
    APIOut->ucTxnQuqlifier = DongleOut->ucTxnQuqlifier;//Txn Quqlifier
    APIOut->ucSignatureKeyKVN = DongleOut->ucSignatureKeyKVN;//Signature Key KVN
    if (DongleOut->ucPurseVersionNumber != MIFARE)
        memcpy(APIOut->ucCPUSAMID,DongleOut->ucSID_STAC,sizeof(APIOut->ucCPUSAMID));//CPU SAM ID
    if (DongleOut->ucPurseVersionNumber != MIFARE)
        APIOut->ucHostAdminKVN = DongleOut->ucHost_SAMKVN;//Host Admin KVN
    APIOut->ucPurseVersionNumber = DongleOut->ucPurseVersionNumber;//Purse Version Number
    memcpy(APIOut->ucTMLocationID,DongleIn->ucTMLocationID,sizeof(APIOut->ucTMLocationID));//TM Location ID
    if (DongleOut->ucPurseVersionNumber != MIFARE)
        memcpy(APIOut->ucCACrypto,DongleOut->ucSVCrypto_SAMCRN_SAMID,16);//CACrypto
    vdBuildCardAVRData(&APIOut->stCardAVRInfo_t,&DongleOut->stLastCreditTxnLogInfo_t,DongleOut->ucCardID,DongleOut->ucIssuerCode,DongleOut->ucCPUSPID,DongleOut->ucCPULocationID,DongleOut->ucPID,APIOut->ucLoyaltyCounter);//Card AVR Info
    vdBuildReaderAVRData(&APIOut->stReaderAVRInfo_t,&DongleOut->stReaderAVRInfo_t);//Reader AVR Info
    // 20年單獨展期
    if ((srTxnData.srIngData.inTransType == TXN_ECC_EXTEND_VALID) || (srTxnData.srParameter.inSetCardValidDate == 1))
		APIOut->ucCardIDLen = DongleOut->ucCardIDLen | 0x80;//Card ID Len
	else
	    APIOut->ucCardIDLen = DongleOut->ucCardIDLen;//Card ID Len
    APIOut->ucCPUIssuerKey = DongleOut->ucCPUIssuerKeyKVN;//CPU Issuer Key

    memcpy(stStore_DataInfo_t.ucTMSerialNumber,DongleOut->ucTMSerialNumber,sizeof(stStore_DataInfo_t.ucTMSerialNumber));

    return(inTMOutLen);
}

int inBuildAuthStudentSetValueData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
AuthStudentSetValue_TM_Out *APIOut = (AuthStudentSetValue_TM_Out *)API_Out;
AuthStudentSetValue_APDU_In *DongleIn = (AuthStudentSetValue_APDU_In *)Dongle_In;
AuthStudentSetValue_APDU_Out *DongleOut = (AuthStudentSetValue_APDU_Out *)Dongle_Out;
StudentSetValue_APDU_Out *TxnReqOut = (StudentSetValue_APDU_Out *)Dongle_ReqOut;

	inTMOutLen = sizeof(AuthStudentSetValue_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	vdUIntToAsc(DongleOut->ucExtAddValueTxnAmt,sizeof(DongleOut->ucExtAddValueTxnAmt),APIOut->anTxnAmt,sizeof(APIOut->anTxnAmt),FALSE,' ',10);//Txn Amt
	fnUnPack(&DongleOut->ucNewCardType,sizeof(DongleOut->ucNewCardType),APIOut->anCardType);//Card Type
	UnixToDateTime((BYTE*)DongleOut->ucNewPersonalExpiryDate,(BYTE*)APIOut->anProfileExpiryDate,sizeof(APIOut->anProfileExpiryDate));//Personal Profile Expiry Date
	//fnLongToUnix(APIOut->anProfileExpiryDate,(unsigned long *)DongleOut->ucNewPersonalExpiryDate,sizeof(APIOut->anProfileExpiryDate));
	//fngetDosDate(DongleOut->ucNewPersonalExpiryDate,APIOut->anProfileExpiryDate);//New Profile Expiry Date
	vdUIntToAsc(TxnReqOut->ucCardID,sizeof(TxnReqOut->ucCardID),APIOut->stSTMC_t.anCardID,sizeof(APIOut->stSTMC_t.anCardID),FALSE,' ',10);//n_Card Physical ID
	fnBINTODEVASC(TxnReqOut->ucDeviceID,APIOut->stSTMC_t.anDeviceID,sizeof(APIOut->stSTMC_t.anDeviceID),MIFARE);//n_TXN Device ID
	fnBINTODEVASC(TxnReqOut->ucCPUDeviceID,APIOut->stSTMC_t.anCPUDeviceID,sizeof(APIOut->stSTMC_t.anCPUDeviceID),LEVEL2);//n_TXN Device ID
	vdCountEVBefore(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.anEVBeforeTxn,sizeof(APIOut->stSTMC_t.anEVBeforeTxn),TxnReqOut->ucMsgType);//n_EV Before TXN
	vdUIntToAsc(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),APIOut->stSTMC_t.anTxnSN,sizeof(APIOut->stSTMC_t.anTxnSN),FALSE,' ',10);//n_TXN SNum
	//vdUIntToAsc(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),APIOut->stSTMC_t.anTxnAmt,sizeof(APIOut->stSTMC_t.anTxnAmt),' ',10);//n_TXN AMT
	memcpy(APIOut->stSTMC_t.anTxnAmt, "       0", 8);
	vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.anEV,sizeof(APIOut->stSTMC_t.anEV),FALSE,' ',10);//n_EV
	memcpy(APIOut->stSTMC_t.anProcessignCode, "813399", 6);
	fnUnPack(&TxnReqOut->ucMsgType,sizeof(TxnReqOut->ucMsgType),APIOut->stSTMC_t.unMsgType);//Msg Type
	fnUnPack(&TxnReqOut->ucSubType,sizeof(TxnReqOut->ucSubType),APIOut->stSTMC_t.unSubType);//Sub Type
	fnUnPack(TxnReqOut->ucDeviceID,sizeof(TxnReqOut->ucDeviceID),APIOut->stSTMC_t.unDeviceID);//Device ID
	fnUnPack(&TxnReqOut->ucSPID,sizeof(TxnReqOut->ucSPID),APIOut->stSTMC_t.unSPID);//SP ID
	fnUnPack(DongleOut->ucTxnDateTime,sizeof(DongleOut->ucTxnDateTime),APIOut->stSTMC_t.unTxnDateTime);//Txn Date Time
	fnUnPack(&DongleOut->ucNewCardType,sizeof(DongleOut->ucNewCardType),APIOut->stSTMC_t.unCardType);//Card Type
	fnUnPack(&TxnReqOut->ucPersonalProfile,sizeof(TxnReqOut->ucPersonalProfile),APIOut->stSTMC_t.unPersonalProfile);//Personal Profile
	fnUnPack(&TxnReqOut->ucLocationID,sizeof(TxnReqOut->ucLocationID),APIOut->stSTMC_t.unLocationID);//Location ID
	fnUnPack(TxnReqOut->ucCardID,sizeof(TxnReqOut->ucCardID),APIOut->stSTMC_t.unCardID);//CPU Card ID
	fnUnPack(&TxnReqOut->ucIssuerCode,sizeof(TxnReqOut->ucIssuerCode),APIOut->stSTMC_t.unIssuerCode);//Issuer Code
	fnUnPack(&TxnReqOut->ucBankCode,sizeof(TxnReqOut->ucBankCode),APIOut->stSTMC_t.unBankCode);//Bank Code
	if(TxnReqOut->ucPurseVersionNumber == MIFARE)
		fnUnPack(TxnReqOut->ucLoyaltyCounterOrSubAreaCode,sizeof(TxnReqOut->ucLoyaltyCounterOrSubAreaCode),APIOut->stSTMC_t.unLoyaltyCounter);//Loyalty Counter
	else
		memset(APIOut->stSTMC_t.unLoyaltyCounter,0x30,sizeof(APIOut->stSTMC_t.unLoyaltyCounter));//Loyalty Counter
	if(TxnReqOut->ucPurseVersionNumber == MIFARE)
	{
		if (TxnReqOut->ucReaderFWVersion[5] >= 0x80) //match sam的mfrc改用device id
			memcpy(&DongleOut->ucMAC_HCrypto[6], TxnReqOut->ucDeviceID, 4);
		fnUnPack(DongleOut->ucMAC_HCrypto,10,APIOut->stSTMC_t.unMAC);//MAC
		// 配對SAM用Device id
		//if (TxnReqOut->ucReaderFWVersion[5] >= 0x80)
		//	fnUnPack(TxnReqOut->ucDeviceID, sizeof(TxnReqOut->ucDeviceID), &APIOut->stSTMC_t.unMAC[12]);
	}
	else
		memset(APIOut->stSTMC_t.unMAC,0x30,sizeof(APIOut->stSTMC_t.unMAC));//MAC
	fnUnPack(TxnReqOut->ucExpiryDate,sizeof(TxnReqOut->ucExpiryDate),APIOut->stSTMC_t.unOrgExpiryDate);//Org Expiry Date
	if (DongleIn->ucEPDUpdateFlag == 0x01)
		fnUnPack(DongleIn->ucNewExpiryDate,sizeof(DongleIn->ucNewExpiryDate),APIOut->stSTMC_t.unNewExpiryDate);//New Expiry Date
	else
		fnUnPack(TxnReqOut->ucExpiryDate,sizeof(TxnReqOut->ucExpiryDate),APIOut->stSTMC_t.unNewExpiryDate);//New Expiry Date
	//fnUnPack(DongleIn->ucNewPersonalExpiryDate,sizeof(DongleIn->ucNewPersonalExpiryDate),APIOut->stSTMC_t.unNewExpiryDate);//New Expiry Date
	fnUnPack(&TxnReqOut->ucAreaCode,sizeof(TxnReqOut->ucAreaCode),APIOut->stSTMC_t.unAreaCode);//Area Code
	//fnUnPack(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),APIOut->stSTMC_t.unTxnAmt);//Txn Amt
	memcpy(APIOut->stSTMC_t.unTxnAmt, "000000", 6);
	fnUnPack(DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.unEV);//EV
	fnUnPack(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),APIOut->stSTMC_t.unTxnSN);//Txn SN
	fnUnPack(TxnReqOut->ucCPUDeviceID,sizeof(TxnReqOut->ucCPUDeviceID),APIOut->stSTMC_t.unCPUDeviceID);//CPU Device ID
	fnUnPack(TxnReqOut->ucCPUSPID,sizeof(TxnReqOut->ucCPUSPID),APIOut->stSTMC_t.unCPUSPID);//CPU SP ID
	fnUnPack(TxnReqOut->ucCPULocationID,sizeof(TxnReqOut->ucCPULocationID),APIOut->stSTMC_t.unCPULocationID);//CPU Location ID
	fnUnPack(TxnReqOut->ucPID,sizeof(TxnReqOut->ucPID),APIOut->stSTMC_t.unPID);//Purse ID
	fnUnPack(TxnReqOut->ucCTC,sizeof(TxnReqOut->ucCTC),APIOut->stSTMC_t.unCTC);//CTC
	fnUnPack(TxnReqOut->ucProfileExpiryDate,sizeof(TxnReqOut->ucProfileExpiryDate),APIOut->stSTMC_t.unOrgProfileExpiryDate);//Org Profile Expiry Date
	fnUnPack(DongleOut->ucNewPersonalExpiryDate,sizeof(DongleOut->ucNewPersonalExpiryDate),APIOut->stSTMC_t.unNewProfileExpiryDate);//New Profile Expiry Date
	if(TxnReqOut->ucPurseVersionNumber != MIFARE)
		fnUnPack(TxnReqOut->ucLoyaltyCounterOrSubAreaCode,sizeof(TxnReqOut->ucLoyaltyCounterOrSubAreaCode),APIOut->stSTMC_t.unSubAreaCode);//Sub Area Code
	else
		memset(APIOut->stSTMC_t.unSubAreaCode,0x30,sizeof(APIOut->stSTMC_t.unSubAreaCode));//Sub Area Code
	fnUnPack(TxnReqOut->ucTxnSN,sizeof(TxnReqOut->ucTxnSN),APIOut->stSTMC_t.unTxnSNBeforeTxn);//Txn SN Before Txn
	fnUnPack(TxnReqOut->ucEV,sizeof(TxnReqOut->ucEV),APIOut->stSTMC_t.unEVBeforeTxn);//EV Before Txn
	fnUnPack(TxnReqOut->ucDeposit,sizeof(TxnReqOut->ucDeposit),APIOut->stSTMC_t.unDeposit);//Deposit
	fnUnPack(&TxnReqOut->ucTxnMode,sizeof(TxnReqOut->ucTxnMode),APIOut->stSTMC_t.unTxnMode);//Txn Mode
	fnUnPack(&TxnReqOut->ucTxnQuqlifier,sizeof(TxnReqOut->ucTxnQuqlifier),APIOut->stSTMC_t.unTxnQuqlifier);//Txn Quqlifier
	fnUnPack(&TxnReqOut->ucSignatureKeyKVN,sizeof(TxnReqOut->ucSignatureKeyKVN),APIOut->stSTMC_t.unSignatureKeyKVN);//Signature Key KVN
	fnUnPack(DongleOut->ucSignature,sizeof(DongleOut->ucSignature),APIOut->stSTMC_t.unSignature);//Signature
	fnUnPack(DongleOut->ucCPUSAMID,sizeof(DongleOut->ucCPUSAMID),APIOut->stSTMC_t.unCPUSAMID);//SAM ID
	if(TxnReqOut->ucPurseVersionNumber != MIFARE)
		fnUnPack(&DongleOut->ucMAC_HCrypto[0],1,APIOut->stSTMC_t.unHashType);//Hash Type
	else
		memset(APIOut->stSTMC_t.unHashType,0x30,sizeof(APIOut->stSTMC_t.unHashType));
	if(TxnReqOut->ucPurseVersionNumber != MIFARE)
	{
		fnUnPack(&DongleOut->ucMAC_HCrypto[1],1,APIOut->stSTMC_t.unHostAdminKVN);//Host Admin KVN
		fnUnPack(&DongleOut->ucMAC_HCrypto[2],16,APIOut->stSTMC_t.unCPUMAC);//MAC
	}
	else
	{
		memset(APIOut->stSTMC_t.unHostAdminKVN,0x30,sizeof(APIOut->stSTMC_t.unHostAdminKVN));
		memset(APIOut->stSTMC_t.unCPUMAC,0x30,sizeof(APIOut->stSTMC_t.unCPUMAC));//MAC
	}
	fnUnPack(&TxnReqOut->ucPurseVersionNumber,sizeof(TxnReqOut->ucPurseVersionNumber),APIOut->stSTMC_t.unPurseVersionNumber);//Purse Version Number
	fnUnPack(&TxnReqOut->ucCardIDLen,sizeof(TxnReqOut->ucCardIDLen),APIOut->stSTMC_t.unCardIDLen);//Card ID Len
	memcpy(APIOut->stSTMC_t.ucTMLocationID,stStore_DataInfo_t.ucTMLocationID,sizeof(stStore_DataInfo_t.ucTMLocationID));//TM Location ID
	memcpy(APIOut->stSTMC_t.ucTMID,stStore_DataInfo_t.ucTMID,sizeof(stStore_DataInfo_t.ucTMID));//TM ID
	UnixToDateTime(DongleOut->ucTxnDateTime,APIOut->stSTMC_t.ucTMTxnDateTime,sizeof(APIOut->stSTMC_t.ucTMTxnDateTime));
	//fnLongToUnix(APIOut->stSTMC_t.ucTMTxnDateTime,(unsigned long *)DongleOut->ucTxnDateTime,14);//TM Txn Date Time
	memcpy(APIOut->stSTMC_t.ucTMSerialNumber,stStore_DataInfo_t.ucTMSerialNumber,sizeof(stStore_DataInfo_t.ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->stSTMC_t.ucTMAgentNumber,stStore_DataInfo_t.ucTMAgentNumber,sizeof(stStore_DataInfo_t.ucTMAgentNumber));//TM Agent Number
	//帳務資料結束
	//Congirm資料開始
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",97);//Data Len
	memcpy(APIOut->ucProcessignCode, "813399", 6);
	APIOut->ucMsgType = TxnReqOut->ucMsgType;//Msg Type
	APIOut->ucSubType = TxnReqOut->ucSubType;//Sub Type
	memcpy(APIOut->ucDeviceID,TxnReqOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	memcpy(APIOut->ucTxnDateTime,DongleOut->ucTxnDateTime,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	if(TxnReqOut->ucPurseVersionNumber == MIFARE)
		memcpy(APIOut->ucMAC,DongleOut->ucMAC_HCrypto,sizeof(APIOut->ucMAC));//MAC
	else
		memset(APIOut->ucMAC,0x00,sizeof(APIOut->ucMAC));//MAC
	memcpy(APIOut->ucConfirmCode,DongleOut->ucConfirmCode,sizeof(APIOut->ucConfirmCode));//Confirm Code
	if(TxnReqOut->ucPurseVersionNumber == MIFARE)
		memcpy(APIOut->ucCTAC,&DongleOut->ucMAC_HCrypto[10],sizeof(APIOut->ucMAC));//CTAC
	else
		memset(APIOut->ucCTAC,0x00,sizeof(APIOut->ucMAC));//CTAC
	memcpy(APIOut->ucCardID,TxnReqOut->ucCardID,sizeof(APIOut->ucCardID));//Card ID
	//memcpy(APIOut->ucTxnAmt,DongleOut->ucExtAddValueTxnAmt,sizeof(APIOut->ucTxnAmt));//Card ID
	memcpy(APIOut->ucEV,DongleOut->ucEV,sizeof(APIOut->ucEV));//EV
	memcpy(APIOut->ucTxnSN,DongleOut->ucTxnSN,sizeof(APIOut->ucTxnSN));//Txn SN
	memcpy(APIOut->ucCPUDeviceID,TxnReqOut->ucCPUDeviceID,sizeof(APIOut->ucCPUDeviceID));//CPU Device ID
	memcpy(APIOut->ucPID,TxnReqOut->ucPID,sizeof(APIOut->ucPID));//Purse ID
	memcpy(APIOut->ucSignature,DongleOut->ucSignature,sizeof(APIOut->ucSignature));//Signature
	if(TxnReqOut->ucPurseVersionNumber != MIFARE)
	{
		APIOut->ucHashType = DongleOut->ucMAC_HCrypto[0];//Hash Type
		APIOut->ucHostAdminKVN = DongleOut->ucMAC_HCrypto[1];//Host Admin KVN
		memcpy(APIOut->ucCPUMAC,&DongleOut->ucMAC_HCrypto[2],sizeof(APIOut->ucCPUMAC));//CPU MAC
	}
	else
	{
		APIOut->ucHashType = 0x00;
		APIOut->ucHostAdminKVN = 0x00;
		memset(APIOut->ucCPUMAC,0x00,sizeof(APIOut->ucCPUMAC));//CPU MAC
	}

	return(inTMOutLen);
}

//沒再用,先移除再說!!
/*int inBuildTxnReqOnlineData4(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
TxnReqOnline_TM_Out_3 *APIOut = (TxnReqOnline_TM_Out_3 *)API_Out;
TxnReqOnline_APDU_In *DongleIn = (TxnReqOnline_APDU_In *)Dongle_In;
TxnReqOnline_APDU_Out *DongleOut = (TxnReqOnline_APDU_Out *)Dongle_Out;

	inTMOutLen = sizeof(TxnReqOnline_TM_Out_3);
	memset(APIOut,0x00,inTMOutLen);
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",inTMOutLen - 7);//Data Len
	vdBuildProcessingCode(inTxnType,DongleOut->ucMsgType,DongleOut->ucSubType,DongleOut->ucPersonalProfile,APIOut->ucProcessignCode);//Processing Code
	vdBuildBLCVersion(APIOut->ucBLVersion);
	//fnASCTOBIN(APIOut->ucBLVersion,gsBlackListVersion,5,sizeof(APIOut->ucBLVersion),DECIMAL);//Black List
	APIOut->ucMsgType = DongleOut->ucMsgType;//Msg Type
	APIOut->ucSubType = DongleOut->ucSubType;//Sub Type
	memcpy(APIOut->ucDeviceID,DongleOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	APIOut->ucSPID = DongleOut->ucSPID;//SP ID
	memcpy(APIOut->ucTxnDateTime,DongleIn->ucTxnDateTime,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	APIOut->ucCardType = DongleOut->ucCardType;//Card Type
	APIOut->ucPersonalProfile = DongleOut->ucPersonalProfile;//Personal Profile
	APIOut->ucIssuerCode = DongleOut->ucIssuerCode;//Issuer Code
	APIOut->ucLocationID = DongleOut->ucLocationID;//Location ID
	APIOut->ucBankCode = DongleOut->ucBankCode;//Bank Code
	memcpy(APIOut->ucLoyaltyCounter,DongleOut->ucLoyaltyCounter,sizeof(APIOut->ucLoyaltyCounter));//Loyalty Counter
	memcpy(APIOut->ucExpiryDate,DongleOut->ucExpiryDate,sizeof(APIOut->ucExpiryDate));//Expiry Date
	APIOut->ucAreaCode = DongleOut->ucAreaCode;//Area Code
	memcpy(APIOut->ucTMID,DongleIn->ucTMID,sizeof(APIOut->ucTMID));//TM ID
	memcpy(APIOut->ucTMTxnDateTime,DongleIn->ucTMTxnDateTime,sizeof(APIOut->ucTMTxnDateTime));//TM Txn Date Time
	memcpy(APIOut->ucTMSerialNumber,DongleOut->ucTMSerialNumber,sizeof(APIOut->ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->ucTMAgentNumber,DongleIn->ucTMAgentNumber,sizeof(APIOut->ucTMAgentNumber));//TM Agent Number
	memcpy(APIOut->ucSTAC,DongleOut->ucSTAC,sizeof(APIOut->ucSTAC));//STAC
	APIOut->ucKeyVersion = DongleOut->ucSAMKVN;//SAM KVN
	memcpy(APIOut->ucSAMID,DongleOut->ucSAMID,sizeof(APIOut->ucSAMID));//SAM ID
	memcpy(APIOut->ucSAMSN,DongleOut->ucSAMSN,sizeof(APIOut->ucSAMSN));//SAM SN
	memcpy(APIOut->ucSAMCRN,DongleOut->ucSAMCRN,sizeof(APIOut->ucSAMCRN));//SAM CRN
	memcpy(APIOut->ucReaderFWVersion,DongleOut->ucReaderFWVersion,sizeof(APIOut->ucReaderFWVersion));//Reader FW Version
	memcpy(APIOut->ucCardRefundFee,DongleIn->ucRefundFee,sizeof(APIOut->ucCardRefundFee));//Card Refund Fee
	memcpy(APIOut->ucCardID,DongleOut->ucCardID,sizeof(APIOut->ucCardID));//Card ID
	memcpy(APIOut->ucTxnAmt,DongleOut->ucTxnAmt,sizeof(APIOut->ucTxnAmt));//Txn Amt
	memcpy(APIOut->ucCPUDeviceID,DongleOut->ucCPUDeviceID,sizeof(APIOut->ucCPUDeviceID));//CPU Device ID
	memcpy(APIOut->ucCPUSPID,DongleOut->ucCPUSPID,sizeof(APIOut->ucCPUSPID));//CPU SP ID
	memcpy(APIOut->ucCPULocationID,DongleOut->ucCPULocationID,sizeof(APIOut->ucCPULocationID));//CPU Location ID
	memcpy(APIOut->ucPID,DongleOut->ucPID,sizeof(APIOut->ucPID));//CPU Purse ID
	memcpy(APIOut->ucCTC,DongleOut->ucCTC,sizeof(APIOut->ucCTC));//CPU CTC
	memcpy(APIOut->ucProfileExpiryDate,DongleOut->ucProfileExpiryDate,sizeof(APIOut->ucProfileExpiryDate));//Profile Expiry Date
	memcpy(APIOut->ucSubAreaCode,DongleOut->ucSubAreaCode,sizeof(APIOut->ucSubAreaCode));//Sub Area Code
	memcpy(APIOut->ucTxnSN,DongleOut->ucTxnSN,sizeof(APIOut->ucTxnSN));//Txn SN
	memcpy(APIOut->ucEV,DongleOut->ucEV,sizeof(APIOut->ucEV));//EV
	memcpy(APIOut->ucDeposit,DongleOut->ucDeposit,sizeof(APIOut->ucDeposit));//Deposit
	memcpy(APIOut->ucCPUBrokenFee,DongleIn->ucBrokenFee,sizeof(APIOut->ucCPUBrokenFee));//CPU Broken Fee
	APIOut->ucCPUAdminKeyKVN = DongleOut->ucCPUAdminKeyKVN;//CPU Admin Key KVN
	APIOut->ucCreditKeyKVN = DongleOut->ucCreditKeyKVN;//Credit Key KVN
	APIOut->ucCPUIssuerKeyKVN = DongleOut->ucCPUIssuerKeyKVN;//CPU Issuer Key KVN
	APIOut->ucTxnMode = DongleOut->ucTxnMode;//Txn Mode
	APIOut->ucTxnQuqlifier = DongleOut->ucTxnQuqlifier;//Txn Quqlifier
	APIOut->ucSignatureKeyKVN = DongleOut->ucSignatureKeyKVN;//Signature Key KVN
	memcpy(APIOut->ucCPUSAMID,DongleOut->ucCPUSAMID,sizeof(APIOut->ucCPUSAMID));//CPU SAM ID
	APIOut->ucHostAdminKVN = DongleOut->ucHostAdminKVN;//Host Admin KVN
	APIOut->ucPurseVersionNumber = DongleOut->ucPurseVersionNumber;//Purse Version Number
	memcpy(APIOut->ucTMLocationID,DongleIn->ucTMLocationID,sizeof(APIOut->ucTMLocationID));//TM Location ID
	memcpy(APIOut->ucCACrypto,DongleOut->ucTxnCrypto,8);//CACrypto前8碼
	memset(&APIOut->ucCACrypto[8],0x00,8);//CACrypto後8碼
	vdBuildCardAVRData(&APIOut->stCardAVRInfo_t,&DongleOut->stLastCreditTxnLogInfo_t,DongleOut->ucCardID,DongleOut->ucIssuerCode,DongleOut->ucCPUSPID,DongleOut->ucCPULocationID,DongleOut->ucPID,APIOut->ucLoyaltyCounter);//Card AVR Info
	vdBuildReaderAVRData(&APIOut->stReaderAVRInfo_t,&DongleOut->stReaderAVRInfo_t);//Reader AVR Info
	//memcpy(&APIOut->stReaderAVRInfo_t,&DongleOut->stReaderAVRInfo_t,sizeof(APIOut->stReaderAVRInfo_t));//Reader AVR Info
	APIOut->ucCardIDLen = DongleOut->ucCardIDLen;//Card ID Len
	memcpy(APIOut->ucCardCustomerFee,DongleIn->ucCustomerFee,sizeof(APIOut->ucCardCustomerFee));//Card Customer Fee

	memcpy(stStore_DataInfo_t.ucTMSerialNumber,DongleOut->ucTMSerialNumber,sizeof(stStore_DataInfo_t.ucTMSerialNumber));

	return(inTMOutLen);
}*/

int inBuildResetOffData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
//Reset_TM_Out *APIOut = (Reset_TM_Out *)API_Out;
Reset_APDU_In *DongleIn = (Reset_APDU_In *)Dongle_In;
Reset_APDU_Out *DongleOut = (Reset_APDU_Out *)Dongle_Out;
BYTE bBuf[10];

	inTMOutLen = sizeof(Reset_TM_Out);

	memset(bBuf,0x00,sizeof(bBuf));
	vdStoreDeviceID(DongleOut->ucDeviceID,DongleOut->ucDeviceID[2],DongleIn->ucLocationID,DongleOut->ucCPUDeviceID,&DongleOut->ucCPUDeviceID[3],DongleIn->ucCPULocationID,DongleOut->ucReaderID,DongleOut->ucReaderFWVersion,DongleOut->ucCPUSAMID);

	return(inTMOutLen);
}

void vdBuildBLCVersion(BYTE *bOutData)
{
BYTE gsBlackListVersion[30];

	memset(gsBlackListVersion,0x00,sizeof(gsBlackListVersion));
	inGetBLCName(gsBlackListVersion);
	fnASCTOBIN(bOutData,gsBlackListVersion,5,2,DECIMAL);//Black List Version
}

//add by bean 20141027 start
int inBuildMMSelectCard2Data(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
MMSelectCard2_TM_Out *APIOut = (MMSelectCard2_TM_Out *)API_Out;
//MMSelectCard2_APDU_In *DongleIn = (MMSelectCard2_APDU_In *)Dongle_In;
MMSelectCard2_APDU_Out *DongleOut = (MMSelectCard2_APDU_Out *)Dongle_Out;

	inTMOutLen = sizeof(MMSelectCard2_TM_Out);

	fnUnPack(&DongleOut->ucCardIDLen,sizeof(DongleOut->ucCardIDLen),APIOut->anCardIDLen);
	vdUIntToAsc(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),APIOut->anCardID,sizeof(APIOut->anCardID),TRUE,' ',10);//Txn Amt
	fnUnPack(&DongleOut->ucCardType,sizeof(DongleOut->ucCardType),APIOut->anCardType);

	return(inTMOutLen);
}

int inBuildAuthAccuntLinkData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
AuthAccuntLink_TM_Out *APIOut = (AuthAccuntLink_TM_Out *)API_Out;
AuthTxnOnline_APDU_In *DongleIn = (AuthTxnOnline_APDU_In *)Dongle_In;
AuthTxnOnline_APDU_Out *DongleOut = (AuthTxnOnline_APDU_Out *)Dongle_Out;
TxnReqOnline_APDU_Out *TxnReqOut = (TxnReqOnline_APDU_Out *)Dongle_ReqOut;

	inTMOutLen = sizeof(AuthAccuntLink_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	vdUIntToAsc(TxnReqOut->ucCardID,sizeof(TxnReqOut->ucCardID),APIOut->stSTMC_t.anCardID,sizeof(APIOut->stSTMC_t.anCardID),FALSE,' ',10);//n_Card Physical ID
	fnBINTODEVASC(TxnReqOut->ucDeviceID,APIOut->stSTMC_t.anDeviceID,sizeof(APIOut->stSTMC_t.anDeviceID),MIFARE);//n_TXN Device ID
	fnBINTODEVASC(TxnReqOut->ucCPUDeviceID,APIOut->stSTMC_t.anCPUDeviceID,sizeof(APIOut->stSTMC_t.anCPUDeviceID),LEVEL2);//n_TXN Device ID
	vdCountEVBefore(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.anEVBeforeTxn,sizeof(APIOut->stSTMC_t.anEVBeforeTxn),TxnReqOut->ucMsgType);//n_EV Before TXN
	vdUIntToAsc(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),APIOut->stSTMC_t.anTxnSN,sizeof(APIOut->stSTMC_t.anTxnSN),FALSE,' ',10);//n_TXN SNum
	vdUIntToAsc(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),APIOut->stSTMC_t.anTxnAmt,sizeof(APIOut->stSTMC_t.anTxnAmt),FALSE,' ',10);//n_TXN AMT
	vdIntToAsc(DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.anEV,sizeof(APIOut->stSTMC_t.anEV),FALSE,' ',10);//n_EV
	vdBuildProcessingCode(inTxnType,TxnReqOut->ucMsgType,TxnReqOut->ucSubType,TxnReqOut->ucPersonalProfile,APIOut->stSTMC_t.anProcessignCode);//Processing Code
	fnUnPack(&TxnReqOut->ucMsgType,sizeof(TxnReqOut->ucMsgType),APIOut->stSTMC_t.unMsgType);//Msg Type
	fnUnPack(&TxnReqOut->ucSubType,sizeof(TxnReqOut->ucSubType),APIOut->stSTMC_t.unSubType);//Sub Type
	fnUnPack(TxnReqOut->ucDeviceID,sizeof(TxnReqOut->ucDeviceID),APIOut->stSTMC_t.unDeviceID);//Device ID
	fnUnPack(&TxnReqOut->ucSPID,sizeof(TxnReqOut->ucSPID),APIOut->stSTMC_t.unSPID);//SP ID
	fnUnPack(DongleOut->ucTxnDateTime,sizeof(DongleOut->ucTxnDateTime),APIOut->stSTMC_t.unTxnDateTime);//Txn Date Time
	fnUnPack(&TxnReqOut->ucCardType,sizeof(TxnReqOut->ucCardType),APIOut->stSTMC_t.unCardType);//Card Type
	fnUnPack(&TxnReqOut->ucPersonalProfile,sizeof(TxnReqOut->ucPersonalProfile),APIOut->stSTMC_t.unPersonalProfile);//Personal Profile
	fnUnPack(&TxnReqOut->ucLocationID,sizeof(TxnReqOut->ucLocationID),APIOut->stSTMC_t.unLocationID);//Location ID
	fnUnPack(TxnReqOut->ucCardID,sizeof(TxnReqOut->ucCardID),APIOut->stSTMC_t.unCardID);//CPU Card ID
	fnUnPack(&TxnReqOut->ucIssuerCode,sizeof(TxnReqOut->ucIssuerCode),APIOut->stSTMC_t.unIssuerCode);//Issuer Code
	fnUnPack(&DongleOut->BankCode[0],1,APIOut->stSTMC_t.unBankCode);//Bank Code
	fnUnPack(TxnReqOut->ucLoyaltyCounter,sizeof(TxnReqOut->ucLoyaltyCounter),APIOut->stSTMC_t.unLoyaltyCounter);//Loyalty Counter
	if(TxnReqOut->ucPurseVersionNumber == MIFARE)
	{
		if (TxnReqOut->ucReaderFWVersion[5] >= 0x80) //match sam的mfrc改用device id
			memcpy(&DongleOut->ucMAC_HCrypto[6], TxnReqOut->ucDeviceID, 4);
		fnUnPack(DongleOut->ucMAC_HCrypto,10,APIOut->stSTMC_t.unMAC);//MAC
		// 配對SAM用Device id
		//if (TxnReqOut->ucReaderFWVersion[5] >= 0x80)
		//	fnUnPack(TxnReqOut->ucDeviceID, sizeof(TxnReqOut->ucDeviceID), &APIOut->stSTMC_t.unMAC[12]);
	}
	else
		memset(APIOut->stSTMC_t.unMAC,0x30,sizeof(APIOut->stSTMC_t.unMAC));//MAC
	fnUnPack(TxnReqOut->ucExpiryDate,sizeof(TxnReqOut->ucExpiryDate),APIOut->stSTMC_t.unOrgExpiryDate);//Org Expiry Date
	fnUnPack(DongleIn->ucExpiryDate,sizeof(DongleIn->ucExpiryDate),APIOut->stSTMC_t.unNewExpiryDate);//New Expiry Date
	fnUnPack(&TxnReqOut->ucAreaCode,sizeof(TxnReqOut->ucAreaCode),APIOut->stSTMC_t.unAreaCode);//Area Code
	fnUnPack(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),APIOut->stSTMC_t.unTxnAmt);//Txn Amt
	fnUnPack(DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.unEV);//EV
	fnUnPack(DongleOut->ucTxnSN,sizeof(DongleOut->ucTxnSN),APIOut->stSTMC_t.unTxnSN);//Txn SN
	fnUnPack(TxnReqOut->ucCPUDeviceID,sizeof(TxnReqOut->ucCPUDeviceID),APIOut->stSTMC_t.unCPUDeviceID);//CPU Device ID
	fnUnPack(TxnReqOut->ucCPUSPID,sizeof(TxnReqOut->ucCPUSPID),APIOut->stSTMC_t.unCPUSPID);//CPU SP ID
	fnUnPack(TxnReqOut->ucCPULocationID,sizeof(TxnReqOut->ucCPULocationID),APIOut->stSTMC_t.unCPULocationID);//CPU Location ID
	fnUnPack(TxnReqOut->ucPID,sizeof(TxnReqOut->ucPID),APIOut->stSTMC_t.unPID);//Purse ID
	fnUnPack(TxnReqOut->ucCTC,sizeof(TxnReqOut->ucCTC),APIOut->stSTMC_t.unCTC);//CTC
	fnUnPack(TxnReqOut->ucProfileExpiryDate,sizeof(TxnReqOut->ucProfileExpiryDate),APIOut->stSTMC_t.unOrgProfileExpiryDate);//Org Profile Expiry Date
	fnUnPack(DongleIn->ucProfileExpiryDate,sizeof(DongleIn->ucProfileExpiryDate),APIOut->stSTMC_t.unNewProfileExpiryDate);//New Profile Expiry Date
	fnUnPack(TxnReqOut->ucSubAreaCode,sizeof(TxnReqOut->ucSubAreaCode),APIOut->stSTMC_t.unSubAreaCode);//Sub Area Code
	fnUnPack(TxnReqOut->ucTxnSN,sizeof(TxnReqOut->ucTxnSN),APIOut->stSTMC_t.unTxnSNBeforeTxn);//Txn SN Before Txn
	fnUnPack(TxnReqOut->ucEV,sizeof(TxnReqOut->ucEV),APIOut->stSTMC_t.unEVBeforeTxn);//EV Before Txn
	fnUnPack(TxnReqOut->ucDeposit,sizeof(TxnReqOut->ucDeposit),APIOut->stSTMC_t.unDeposit);//Deposit
	fnUnPack(&TxnReqOut->ucTxnMode,sizeof(TxnReqOut->ucTxnMode),APIOut->stSTMC_t.unTxnMode);//Txn Mode
	fnUnPack(&TxnReqOut->ucTxnQuqlifier,sizeof(TxnReqOut->ucTxnQuqlifier),APIOut->stSTMC_t.unTxnQuqlifier);//Txn Quqlifier
	fnUnPack(&TxnReqOut->ucSignatureKeyKVN,sizeof(TxnReqOut->ucSignatureKeyKVN),APIOut->stSTMC_t.unSignatureKeyKVN);//Signature Key KVN
	fnUnPack(DongleOut->ucSignature,sizeof(DongleOut->ucSignature),APIOut->stSTMC_t.unSignature);//Signature
	fnUnPack(DongleOut->ucCPUSAMID,sizeof(DongleOut->ucCPUSAMID),APIOut->stSTMC_t.unCPUSAMID);//SAM ID
	if(TxnReqOut->ucPurseVersionNumber != MIFARE)
		fnUnPack(&DongleOut->ucMAC_HCrypto[0],1,APIOut->stSTMC_t.unHashType);//Hash Type
	else
		memset(APIOut->stSTMC_t.unHashType,0x30,sizeof(APIOut->stSTMC_t.unHashType));
	if(TxnReqOut->ucPurseVersionNumber != MIFARE)
	{
		fnUnPack(&DongleOut->ucMAC_HCrypto[1],1,APIOut->stSTMC_t.unHostAdminKVN);//Host Admin KVN
		fnUnPack(&DongleOut->ucMAC_HCrypto[2],16,APIOut->stSTMC_t.unCPUMAC);//MAC
	}
	else
	{
		memset(APIOut->stSTMC_t.unHostAdminKVN,0x30,sizeof(APIOut->stSTMC_t.unHostAdminKVN));
		memset(APIOut->stSTMC_t.unCPUMAC,0x30,sizeof(APIOut->stSTMC_t.unCPUMAC));//MAC
	}
	fnUnPack(&TxnReqOut->ucPurseVersionNumber,sizeof(TxnReqOut->ucPurseVersionNumber),APIOut->stSTMC_t.unPurseVersionNumber);//Purse Version Number
	fnUnPack(&TxnReqOut->ucCardIDLen,sizeof(TxnReqOut->ucCardIDLen),APIOut->stSTMC_t.unCardIDLen);//Card ID Len
	memcpy(APIOut->stSTMC_t.ucTMLocationID,stStore_DataInfo_t.ucTMLocationID,sizeof(stStore_DataInfo_t.ucTMLocationID));//TM Location ID
	memcpy(APIOut->stSTMC_t.ucTMID,stStore_DataInfo_t.ucTMID,sizeof(stStore_DataInfo_t.ucTMID));//TM ID
	UnixToDateTime((BYTE*)DongleOut->ucTxnDateTime,(BYTE*)APIOut->stSTMC_t.ucTMTxnDateTime,14);//TM Txn Date Time
	memcpy(APIOut->stSTMC_t.ucTMSerialNumber,stStore_DataInfo_t.ucTMSerialNumber,sizeof(stStore_DataInfo_t.ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->stSTMC_t.ucTMAgentNumber,stStore_DataInfo_t.ucTMAgentNumber,sizeof(stStore_DataInfo_t.ucTMAgentNumber));//TM Agent Number
	//帳務資料結束
	//Confirm資料開始
	vdBuildMessageType(inTxnType,APIOut->ucMessageType);//Message Type ID
	sprintf((char *)APIOut->ucDataLen,"%03d",76);//Data Len
	vdBuildProcessingCode(inTxnType,TxnReqOut->ucMsgType,TxnReqOut->ucSubType,TxnReqOut->ucPersonalProfile,APIOut->ucProcessignCode);//Processing Code
	APIOut->ucMsgType = TxnReqOut->ucMsgType;//Msg Type
	APIOut->ucSubType = TxnReqOut->ucSubType;//Sub Type
	memcpy(APIOut->ucDeviceID,TxnReqOut->ucDeviceID,sizeof(APIOut->ucDeviceID));//Device ID
	memcpy(APIOut->ucTxnDateTime,DongleOut->ucTxnDateTime,sizeof(APIOut->ucTxnDateTime));//Txn Date Time
	APIOut->ucBankCode = DongleOut->BankCode[0];//Bank Code
	if(TxnReqOut->ucPurseVersionNumber == MIFARE)
		memcpy(APIOut->ucMAC,DongleOut->ucMAC_HCrypto,sizeof(APIOut->ucMAC));//MAC
	else
		memset(APIOut->ucMAC,0x00,sizeof(APIOut->ucMAC));//MAC
	memcpy(APIOut->ucConfirmCode,DongleOut->ucConfirmCode,sizeof(APIOut->ucConfirmCode));//Confirm Code
	if(TxnReqOut->ucPurseVersionNumber == MIFARE)
		memcpy(APIOut->ucCTAC,&DongleOut->ucMAC_HCrypto[10],sizeof(APIOut->ucMAC));//CTAC
	else
		memset(APIOut->ucCTAC,0x00,sizeof(APIOut->ucMAC));//CTAC
	APIOut->ucAutoLoadFlag = DongleOut->AutoloadFlag[0];//AutoLoad Flag
	memcpy(APIOut->ucAutoLoadAmt_2,DongleOut->SingleAutoLoadTransactionAmount,sizeof(APIOut->ucAutoLoadAmt_2));//AutoLoad Amt
	memcpy(APIOut->ucCardID,TxnReqOut->ucCardID,sizeof(APIOut->ucCardID));//Card ID
	memcpy(APIOut->ucEV,DongleOut->ucEV,sizeof(APIOut->ucEV));//EV
	memcpy(APIOut->ucTxnSN,DongleOut->ucTxnSN,sizeof(APIOut->ucTxnSN));//Txn SN
	memcpy(APIOut->ucCPUDeviceID,TxnReqOut->ucCPUDeviceID,sizeof(APIOut->ucCPUDeviceID));//CPU Device ID
	memcpy(APIOut->ucPID,TxnReqOut->ucPID,sizeof(APIOut->ucPID));//Purse ID
	memcpy(APIOut->ucSignature,DongleOut->ucSignature,sizeof(APIOut->ucSignature));//Signature
	if(TxnReqOut->ucPurseVersionNumber != MIFARE)
	{
		APIOut->ucHashType = DongleOut->ucMAC_HCrypto[0];//Hash Type
		APIOut->ucHostAdminKVN = DongleOut->ucMAC_HCrypto[1];//Host Admin KVN
		memcpy(APIOut->ucCPUMAC,&DongleOut->ucMAC_HCrypto[2],sizeof(APIOut->ucCPUMAC));//CPU MAC
	}
	else
	{
		APIOut->ucHashType = 0x00;
		APIOut->ucHostAdminKVN = 0x00;
		memset(APIOut->ucCPUMAC,0x00,sizeof(APIOut->ucCPUMAC));//CPU MAC
	}

	return(inTMOutLen);
}

int inBuildICERETxnData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
AuthTxnOnline_TM_Out *APIOut = (AuthTxnOnline_TM_Out *)API_Out;
//AuthTxnOnline_APDU_In *DongleIn = (AuthTxnOnline_APDU_In *)Dongle_In;
VerifyHCrypt_APDU_In *DongleIn = (VerifyHCrypt_APDU_In *)Dongle_In;
//AuthTxnOnline_APDU_Out *DongleOut = (AuthTxnOnline_APDU_Out *)Dongle_Out;
TxnReqOnline_APDU_Out *TxnReqOut = (TxnReqOnline_APDU_Out *)Dongle_ReqOut;
BYTE anCardID[17],unCardID[8];

	inTMOutLen = sizeof(AuthTxnOnline_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	memset(anCardID,0x00,sizeof(anCardID));
	memset(unCardID,0x00,sizeof(unCardID));
	fnUnPack(DongleIn->ucPID,sizeof(DongleIn->ucPID),anCardID);//n_Card Physical ID
	APIOut->stSTMC_t.anCardID[0] = 0x20;
	memcpy(&APIOut->stSTMC_t.anCardID[1],anCardID,16);
	fnBINTODEVASC(TxnReqOut->ucDeviceID,APIOut->stSTMC_t.anDeviceID,sizeof(APIOut->stSTMC_t.anDeviceID),MIFARE);//n_TXN Device ID
	fnBINTODEVASC(TxnReqOut->ucCPUDeviceID,APIOut->stSTMC_t.anCPUDeviceID,sizeof(APIOut->stSTMC_t.anCPUDeviceID),LEVEL2);//n_TXN Device ID
    vdIntToAsc(DongleIn->unEVBeforeTxn,sizeof(DongleIn->unEVBeforeTxn),APIOut->stSTMC_t.anEVBeforeTxn,sizeof(APIOut->stSTMC_t.anEVBeforeTxn),FALSE,' ',10);//n_EV Before TXN
	//vdCountEVBefore(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.anEVBeforeTxn,sizeof(APIOut->stSTMC_t.anEVBeforeTxn),TxnReqOut->ucMsgType);//n_EV Before TXN
	vdUIntToAsc(DongleIn->ucCardSN,sizeof(DongleIn->ucCardSN),APIOut->stSTMC_t.anTxnSN,sizeof(APIOut->stSTMC_t.anTxnSN),FALSE,' ',10);//n_TXN SNum
	vdIntToAsc(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),APIOut->stSTMC_t.anTxnAmt,sizeof(APIOut->stSTMC_t.anTxnAmt),FALSE,' ',10);//n_TXN AMT
	vdIntToAsc(DongleIn->ucEV,sizeof(DongleIn->ucEV),APIOut->stSTMC_t.anEV,sizeof(APIOut->stSTMC_t.anEV),FALSE,' ',10);//n_EV
	vdBuildProcessingCode(inTxnType,TxnReqOut->ucMsgType,TxnReqOut->ucSubType,TxnReqOut->ucPersonalProfile,APIOut->stSTMC_t.anProcessignCode);//Processing Code
	fnUnPack(&TxnReqOut->ucMsgType,sizeof(TxnReqOut->ucMsgType),APIOut->stSTMC_t.unMsgType);//Msg Type
	fnUnPack(&TxnReqOut->ucSubType,sizeof(TxnReqOut->ucSubType),APIOut->stSTMC_t.unSubType);//Sub Type
	fnUnPack(TxnReqOut->ucDeviceID,sizeof(TxnReqOut->ucDeviceID),APIOut->stSTMC_t.unDeviceID);//Device ID
	fnUnPack(&TxnReqOut->ucSPID,sizeof(TxnReqOut->ucSPID),APIOut->stSTMC_t.unSPID);//SP ID
	fnUnPack(DongleIn->ucTxnDateTime,sizeof(DongleIn->ucTxnDateTime),APIOut->stSTMC_t.unTxnDateTime);//Txn Date Time
	fnUnPack(&TxnReqOut->ucCardType,sizeof(TxnReqOut->ucCardType),APIOut->stSTMC_t.unCardType);//Card Type
	fnUnPack(&TxnReqOut->ucPersonalProfile,sizeof(TxnReqOut->ucPersonalProfile),APIOut->stSTMC_t.unPersonalProfile);//Personal Profile
	fnUnPack(&TxnReqOut->ucLocationID,sizeof(TxnReqOut->ucLocationID),APIOut->stSTMC_t.unLocationID);//Location ID
	ECC_ASCIIToUINT64(anCardID,strlen((char *)anCardID),unCardID);//CPU Card ID
	fnUnPack(unCardID,7,APIOut->stSTMC_t.unCardID);//CPU Card ID
	fnUnPack(&TxnReqOut->ucIssuerCode,sizeof(TxnReqOut->ucIssuerCode),APIOut->stSTMC_t.unIssuerCode);//Issuer Code
	fnUnPack(&TxnReqOut->ucBankCode,sizeof(TxnReqOut->ucBankCode),APIOut->stSTMC_t.unBankCode);//Bank Code
	fnUnPack(TxnReqOut->ucLoyaltyCounter,sizeof(TxnReqOut->ucLoyaltyCounter),APIOut->stSTMC_t.unLoyaltyCounter);//Loyalty Counter
	memset(APIOut->stSTMC_t.unMAC,0x30,sizeof(APIOut->stSTMC_t.unMAC));//MAC
	fnUnPack(TxnReqOut->ucExpiryDate,sizeof(TxnReqOut->ucExpiryDate),APIOut->stSTMC_t.unOrgExpiryDate);//Org Expiry Date
	fnUnPack(TxnReqOut->ucExpiryDate,sizeof(TxnReqOut->ucExpiryDate),APIOut->stSTMC_t.unNewExpiryDate);//New Expiry Date
	fnUnPack(&TxnReqOut->ucAreaCode,sizeof(TxnReqOut->ucAreaCode),APIOut->stSTMC_t.unAreaCode);//Area Code
	fnUnPack(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),APIOut->stSTMC_t.unTxnAmt);//Txn Amt
	fnUnPack(DongleIn->ucEV,sizeof(DongleIn->ucEV),APIOut->stSTMC_t.unEV);//EV
	//memset(APIOut->stSTMC_t.unTxnSN,0x30,sizeof(APIOut->stSTMC_t.unTxnSN));//Txn SN
	fnUnPack(DongleIn->ucCardSN,sizeof(DongleIn->ucCardSN),APIOut->stSTMC_t.unTxnSN);//Txn SN
	fnUnPack(TxnReqOut->ucCPUDeviceID,sizeof(TxnReqOut->ucCPUDeviceID),APIOut->stSTMC_t.unCPUDeviceID);//CPU Device ID
	fnUnPack(TxnReqOut->ucCPUSPID,sizeof(TxnReqOut->ucCPUSPID),APIOut->stSTMC_t.unCPUSPID);//CPU SP ID
	fnUnPack(TxnReqOut->ucCPULocationID,sizeof(TxnReqOut->ucCPULocationID),APIOut->stSTMC_t.unCPULocationID);//CPU Location ID
	fnUnPack(TxnReqOut->ucPID,sizeof(TxnReqOut->ucPID),APIOut->stSTMC_t.unPID);//Purse ID
	fnUnPack(TxnReqOut->ucCTC,sizeof(TxnReqOut->ucCTC),APIOut->stSTMC_t.unCTC);//CTC
	fnUnPack(TxnReqOut->ucProfileExpiryDate,sizeof(TxnReqOut->ucProfileExpiryDate),APIOut->stSTMC_t.unOrgProfileExpiryDate);//Org Profile Expiry Date
	fnUnPack(TxnReqOut->ucProfileExpiryDate,sizeof(TxnReqOut->ucProfileExpiryDate),APIOut->stSTMC_t.unNewProfileExpiryDate);//New Profile Expiry Date
	fnUnPack(TxnReqOut->ucSubAreaCode,sizeof(TxnReqOut->ucSubAreaCode),APIOut->stSTMC_t.unSubAreaCode);//Sub Area Code
	memset(APIOut->stSTMC_t.unTxnSNBeforeTxn,0x30,sizeof(APIOut->stSTMC_t.unTxnSNBeforeTxn));//Txn SN Before Txn
	//fnUnPack(TxnReqOut->ucTxnSN,sizeof(TxnReqOut->ucTxnSN),APIOut->stSTMC_t.unTxnSNBeforeTxn);//Txn SN Before Txn
	fnUnPack(DongleIn->unEVBeforeTxn,sizeof(DongleIn->unEVBeforeTxn),APIOut->stSTMC_t.unEVBeforeTxn);//EV Before Txn
	fnUnPack(TxnReqOut->ucDeposit,sizeof(TxnReqOut->ucDeposit),APIOut->stSTMC_t.unDeposit);//Deposit
	fnUnPack(&TxnReqOut->ucTxnMode,sizeof(TxnReqOut->ucTxnMode),APIOut->stSTMC_t.unTxnMode);//Txn Mode
	fnUnPack(&TxnReqOut->ucTxnQuqlifier,sizeof(TxnReqOut->ucTxnQuqlifier),APIOut->stSTMC_t.unTxnQuqlifier);//Txn Quqlifier
	fnUnPack(&TxnReqOut->ucSignatureKeyKVN,sizeof(TxnReqOut->ucSignatureKeyKVN),APIOut->stSTMC_t.unSignatureKeyKVN);//Signature Key KVN
	fnUnPack(DongleIn->ucAuthCode,16,APIOut->stSTMC_t.unSignature);//Signature
	//memset(APIOut->stSTMC_t.unSignature,0x30,sizeof(APIOut->stSTMC_t.unSignature));
	fnUnPack(TxnReqOut->ucCPUSAMID,sizeof(TxnReqOut->ucCPUSAMID),APIOut->stSTMC_t.unCPUSAMID);//SAM ID
	memset(APIOut->stSTMC_t.unHashType,0x30,sizeof(APIOut->stSTMC_t.unHashType));
	memset(APIOut->stSTMC_t.unHostAdminKVN,0x30,sizeof(APIOut->stSTMC_t.unHostAdminKVN));
	fnUnPack(DongleIn->ucHCrypto,16,APIOut->stSTMC_t.unCPUMAC);//MAC
	fnUnPack(&TxnReqOut->ucPurseVersionNumber,sizeof(TxnReqOut->ucPurseVersionNumber),APIOut->stSTMC_t.unPurseVersionNumber);//Purse Version Number
	fnUnPack(&TxnReqOut->ucCardIDLen,sizeof(TxnReqOut->ucCardIDLen),APIOut->stSTMC_t.unCardIDLen);//Card ID Len
	memcpy(APIOut->stSTMC_t.ucTMLocationID,stStore_DataInfo_t.ucTMLocationID,sizeof(stStore_DataInfo_t.ucTMLocationID));//TM Location ID
	memcpy(APIOut->stSTMC_t.ucTMID,stStore_DataInfo_t.ucTMID,sizeof(stStore_DataInfo_t.ucTMID));//TM ID
	UnixToDateTime((BYTE*)DongleIn->ucTxnDateTime,(BYTE*)APIOut->stSTMC_t.ucTMTxnDateTime,14);//TM Txn Date Time
	memcpy(APIOut->stSTMC_t.ucTMSerialNumber,stStore_DataInfo_t.ucTMSerialNumber,sizeof(stStore_DataInfo_t.ucTMSerialNumber));//TM Serial Number
	memcpy(APIOut->stSTMC_t.ucTMAgentNumber,stStore_DataInfo_t.ucTMAgentNumber,sizeof(stStore_DataInfo_t.ucTMAgentNumber));//TM Agent Number

	return(inTMOutLen);
}

int inBuildICEREQRTxnData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
int inTMOutLen;
AuthTxnOnline_TM_Out *APIOut = (AuthTxnOnline_TM_Out *)API_Out;
//AuthTxnOnline_APDU_In *DongleIn = (AuthTxnOnline_APDU_In *)Dongle_In;
VerifyHCrypt_APDU_In *DongleIn = (VerifyHCrypt_APDU_In *)Dongle_In;
//AuthTxnOnline_APDU_Out *DongleOut = (AuthTxnOnline_APDU_Out *)Dongle_Out;
TxnReqOnline_APDU_Out *TxnReqOut = (TxnReqOnline_APDU_Out *)Dongle_ReqOut;
BYTE anCardID[17],unCardID[8];

	inTMOutLen = sizeof(AuthTxnOnline_TM_Out);
	memset(APIOut,0x00,inTMOutLen);
	memset(anCardID,0x00,sizeof(anCardID));
	memset(unCardID,0x00,sizeof(unCardID));
	fnUnPack(DongleIn->ucPID,sizeof(DongleIn->ucPID),anCardID);//n_Card Physical ID
	APIOut->stSTMC_t.anCardID[0] = 0x20;
	memcpy(&APIOut->stSTMC_t.anCardID[1],anCardID,16);
	fnBINTODEVASC(gTmpData.ucDeviceID,APIOut->stSTMC_t.anDeviceID,sizeof(APIOut->stSTMC_t.anDeviceID),MIFARE);//n_TXN Device ID
	fnBINTODEVASC(gTmpData.ucCPUDeviceID,APIOut->stSTMC_t.anCPUDeviceID,sizeof(APIOut->stSTMC_t.anCPUDeviceID),LEVEL2);//n_TXN Device ID
    vdIntToAsc(DongleIn->unEVBeforeTxn,sizeof(DongleIn->unEVBeforeTxn),APIOut->stSTMC_t.anEVBeforeTxn,sizeof(APIOut->stSTMC_t.anEVBeforeTxn),FALSE,' ',10);//n_EV Before TXN
	//vdCountEVBefore(TxnReqOut->ucTxnAmt,sizeof(TxnReqOut->ucTxnAmt),DongleOut->ucEV,sizeof(DongleOut->ucEV),APIOut->stSTMC_t.anEVBeforeTxn,sizeof(APIOut->stSTMC_t.anEVBeforeTxn),TxnReqOut->ucMsgType);//n_EV Before TXN
	vdUIntToAsc(DongleIn->ucCardSN,sizeof(DongleIn->ucCardSN),APIOut->stSTMC_t.anTxnSN,sizeof(APIOut->stSTMC_t.anTxnSN),FALSE,' ',10);//n_TXN SNum
	vdIntToAsc(DongleIn->ucTxnAmt,sizeof(DongleIn->ucTxnAmt),APIOut->stSTMC_t.anTxnAmt,sizeof(APIOut->stSTMC_t.anTxnAmt),FALSE,' ',10);//n_TXN AMT
	vdIntToAsc(DongleIn->ucEV,sizeof(DongleIn->ucEV),APIOut->stSTMC_t.anEV,sizeof(APIOut->stSTMC_t.anEV),FALSE,' ',10);//n_EV
	vdBuildProcessingCode(inTxnType,TxnReqOut->ucMsgType,TxnReqOut->ucSubType,TxnReqOut->ucPersonalProfile,APIOut->stSTMC_t.anProcessignCode);//Processing Code
	memset(APIOut->stSTMC_t.unMsgType,0x30,sizeof(APIOut->stSTMC_t.unMsgType));//Msg Type
	memset(APIOut->stSTMC_t.unSubType,0x30,sizeof(APIOut->stSTMC_t.unSubType));//Sub Type
	fnUnPack(gTmpData.ucDeviceID,sizeof(gTmpData.ucDeviceID),APIOut->stSTMC_t.unDeviceID);//Device ID
	fnUnPack(&gTmpData.ucSPID,sizeof(gTmpData.ucSPID),APIOut->stSTMC_t.unSPID);//SP ID
	fnUnPack(DongleIn->ucTxnDateTime,sizeof(DongleIn->ucTxnDateTime),APIOut->stSTMC_t.unTxnDateTime);//Txn Date Time
	memset(APIOut->stSTMC_t.unCardType,0x30,sizeof(APIOut->stSTMC_t.unCardType));//Card Type
	memset(APIOut->stSTMC_t.unPersonalProfile,0x30,sizeof(APIOut->stSTMC_t.unPersonalProfile));//Personal Profile
	fnUnPack(&gTmpData.ucLocationID,sizeof(gTmpData.ucLocationID),APIOut->stSTMC_t.unLocationID);//Location ID
	ECC_ASCIIToUINT64(anCardID,strlen((char *)anCardID),unCardID);//CPU Card ID
	fnUnPack(unCardID,7,APIOut->stSTMC_t.unCardID);//CPU Card ID
	memset(APIOut->stSTMC_t.unIssuerCode,0x30,sizeof(APIOut->stSTMC_t.unIssuerCode));//Issuer Code
	memset(APIOut->stSTMC_t.unBankCode,0x30,sizeof(APIOut->stSTMC_t.unBankCode));//Bank Code
	memset(APIOut->stSTMC_t.unLoyaltyCounter,0x30,sizeof(APIOut->stSTMC_t.unLoyaltyCounter));//Loyalty Counter
	memset(APIOut->stSTMC_t.unMAC,0x30,sizeof(APIOut->stSTMC_t.unMAC));//MAC
	memset(APIOut->stSTMC_t.unOrgExpiryDate,0x30,sizeof(APIOut->stSTMC_t.unOrgExpiryDate));//Org Expiry Date
	memset(APIOut->stSTMC_t.unNewExpiryDate,0x30,sizeof(APIOut->stSTMC_t.unNewExpiryDate));//New Expiry Date
	memset(APIOut->stSTMC_t.unAreaCode,0x30,sizeof(APIOut->stSTMC_t.unAreaCode));//Area Code
	fnUnPack(DongleIn->ucTxnAmt,sizeof(DongleIn->ucTxnAmt),APIOut->stSTMC_t.unTxnAmt);//Txn Amt
	fnUnPack(DongleIn->ucEV,sizeof(DongleIn->ucEV),APIOut->stSTMC_t.unEV);//EV
	//memset(APIOut->stSTMC_t.unTxnSN,0x30,sizeof(APIOut->stSTMC_t.unTxnSN));//Txn SN
	fnUnPack(DongleIn->ucCardSN,sizeof(DongleIn->ucCardSN),APIOut->stSTMC_t.unTxnSN);//Txn SN
	fnUnPack(gTmpData.ucCPUDeviceID,sizeof(gTmpData.ucCPUDeviceID),APIOut->stSTMC_t.unCPUDeviceID);//CPU Device ID
	fnUnPack(gTmpData.ucCPUSPID,sizeof(gTmpData.ucCPUSPID),APIOut->stSTMC_t.unCPUSPID);//CPU SP ID
	fnUnPack(gTmpData.ucCPULocationID,sizeof(gTmpData.ucCPULocationID),APIOut->stSTMC_t.unCPULocationID);//CPU Location ID
	fnUnPack(DongleIn->ucPID,sizeof(DongleIn->ucPID),APIOut->stSTMC_t.unPID);//Purse ID
	memset(APIOut->stSTMC_t.unCTC,0x30,sizeof(APIOut->stSTMC_t.unCTC));//CTC
	memset(APIOut->stSTMC_t.unOrgProfileExpiryDate,0x30,sizeof(APIOut->stSTMC_t.unOrgProfileExpiryDate));//Org Profile Expiry Date
	memset(APIOut->stSTMC_t.unNewProfileExpiryDate,0x30,sizeof(APIOut->stSTMC_t.unNewProfileExpiryDate));//New Profile Expiry Date
	memset(APIOut->stSTMC_t.unSubAreaCode,0x30,sizeof(APIOut->stSTMC_t.unSubAreaCode));//Sub Area Code
	memset(APIOut->stSTMC_t.unTxnSNBeforeTxn,0x30,sizeof(APIOut->stSTMC_t.unTxnSNBeforeTxn));//Txn SN Before Txn
	//fnUnPack(TxnReqOut->ucTxnSN,sizeof(TxnReqOut->ucTxnSN),APIOut->stSTMC_t.unTxnSNBeforeTxn);//Txn SN Before Txn
	fnUnPack(DongleIn->unEVBeforeTxn,sizeof(DongleIn->unEVBeforeTxn),APIOut->stSTMC_t.unEVBeforeTxn);//EV Before Txn
	memset(APIOut->stSTMC_t.unDeposit,0x30,sizeof(APIOut->stSTMC_t.unDeposit));//Deposit
	memset(APIOut->stSTMC_t.unTxnMode,0x30,sizeof(APIOut->stSTMC_t.unTxnMode));//Txn Mode
	memset(APIOut->stSTMC_t.unTxnQuqlifier,0x30,sizeof(APIOut->stSTMC_t.unTxnQuqlifier));//Txn Quqlifier
	memset(APIOut->stSTMC_t.unSignatureKeyKVN,0x30,sizeof(APIOut->stSTMC_t.unSignatureKeyKVN));//Signature Key KVN
	fnUnPack(DongleIn->ucAuthCode,16,APIOut->stSTMC_t.unSignature);//Signature
	//memset(APIOut->stSTMC_t.unSignature,0x30,sizeof(APIOut->stSTMC_t.unSignature));
	fnUnPack(gTmpData.ucCPUSAMID,sizeof(gTmpData.ucCPUSAMID),APIOut->stSTMC_t.unCPUSAMID);//SAM ID
	memset(APIOut->stSTMC_t.unHashType,0x30,sizeof(APIOut->stSTMC_t.unHashType));
	memset(APIOut->stSTMC_t.unHostAdminKVN,0x30,sizeof(APIOut->stSTMC_t.unHostAdminKVN));
	fnUnPack(DongleIn->ucHCrypto,16,APIOut->stSTMC_t.unCPUMAC);//MAC
	memcpy(APIOut->stSTMC_t.unPurseVersionNumber,"02",2);//Purse Version Number
	memset(APIOut->stSTMC_t.unCardIDLen,0x30,sizeof(APIOut->stSTMC_t.unCardIDLen));//Card ID Len
	memcpy(APIOut->stSTMC_t.ucTMLocationID,srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID,sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID) - 1);//TM Location ID
	memcpy(APIOut->stSTMC_t.ucTMID,srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMID,sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMID) - 1);//TM ID
//log_msg_debug(LOG_LEVEL_ERROR,FALSE,(char *)"inBuildICEREQRTxnData 3",23,APIOut->stSTMC_t.unCardIDLen,30);
	UnixToDateTime((BYTE*)DongleIn->ucTxnDateTime,(BYTE*)APIOut->stSTMC_t.ucTMTxnDateTime,14);//TM Txn Date Time
	sprintf((char *)APIOut->stSTMC_t.ucTMSerialNumber,"%06ld",srTxnData.srIngData.ulTMSerialNumber);//TM Serial Number
	memcpy(APIOut->stSTMC_t.ucTMAgentNumber,srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber,sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber) - 1);//TM Agent Number

	return(inTMOutLen);
}

