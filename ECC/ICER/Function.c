
#ifdef EZ_AIRLINKEDC
#include "_stdAfx.h"
#else
#include "stdAfx.h"
#endif
#include "Global.h"

BYTE gCurrentFolder[MAX_PATH];
#if READER_MANUFACTURERS==ANDROID_API
	#include <android/log.h>
	#include <jni.h>
	extern JNIEnv *env1;
	extern int fd_;
    extern int fd2_;

#elif READER_MANUFACTURERS==LINUX_API

	extern int fd_;
	extern int fd2_;

#elif READER_MANUFACTURERS==NE_SYMLINK_API

	extern UCHAR *ptrECCTempLog;

#endif

int compare(const void *arg1, const void *arg2)
{
	 return(strcmp(*(char**)arg2,*(char**)arg1));
}

int compare3(const void *arg1, const void *arg2)
{
int inRetVal;

	inRetVal = strcmp((char*)arg2,(char*)arg1);
	//inRetVal = strcmp(*(char**)arg2,*(char**)arg1);
	//log_msg(LOG_LEVEL_ERROR,"compare 2(%d)",inRetVal);

	return(inRetVal);
}

int fnLogFileProcedure()
{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
BYTE gLogFileFolder[100];
char filter[200];//1.0.5.5
#if READER_MANUFACTURERS==WINDOWS_API
struct _finddata_t c_file;
char *logfile[100];//1.0.5.5
#endif
long hFile= 0x00;
int i= 0x00,j= 0x00,nRtn= 0x00;

	memset(gLogFileFolder, 0x00, sizeof(gLogFileFolder));
	memset(filter,0,sizeof(filter));

  	strcpy((char *)gLogFileFolder,(const char *)gCurrentFolder);
	if(srTxnData.srParameter.chFolderCreatFlag == '1' || srTxnData.srParameter.chFolderCreatFlag == 0x00)
	{
		strcat((char *)gLogFileFolder,"\\ICERLog"); //dir folder

		/*#if READER_MANUFACTURERS==WINDOWS_CE_API
			CreateDirectory((const unsigned short *)gLogFileFolder, NULL);
		#else
			mkdir((const char *)gLogFileFolder);
		#endif*/

		ECC_CreateDirectory_Lib((char *)gLogFileFolder);

		strcpy(filter,(const char *)gLogFileFolder);
		strcat(filter,"\\ICER*.log"); //file type
	}
	else //if(srTxnData.srParameter.chFolderCreatFlag == '0')
		strcat(filter,"ICER*.log"); //file type

  #if READER_MANUFACTURERS==WINDOWS_API
	hFile = _findfirst(filter,&c_file); //search file
	if( hFile!=-1 )
	{
		do
		{
			logfile[i]=(char*)ECC_calloc(30,sizeof(char));
			if (logfile[i]==NULL)
			{
				//fnWriteLog("fnLogFileProcedure  malloc FALSE",NULL,0);
				return ICER_ERROR;
				//return MALLOC_ERROR;
			}
			sprintf( logfile[i], "\\%s", c_file.name );
			i+=1;
		}while( _findnext( hFile, &c_file )==0 );
		_findclose(hFile);//1.0.5.2
		j=i;
		qsort((void *)logfile,i,sizeof(logfile[0]),compare); //sort logfile
		while (i > srTxnData.srParameter.gLogCnt)  // delete log file while is not the 30 latest//1.0.5.5
		{
			memset(filter,0,sizeof(filter));
			strcpy( filter, (const char *)gLogFileFolder );
			strcat( filter, logfile[i-1] ); //file type
			#if READER_MANUFACTURERS==WINDOWS_CE_API
				DeleteFile((unsigned short *)filter);
			#else
				remove(filter);
			#endif
			i-=1;
		}
		for (i=0;i<j;i++)
			ECC_free((BYTE *)logfile[i]);
	}
#endif
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
//const char* msglog_ = "//ICERLog//ICER";//1.1.5
BYTE msglog_[50];

	memset(msglog_,0x00,sizeof(msglog_));
	if(srTxnData.srParameter.chFolderCreatFlag == '1' || srTxnData.srParameter.chFolderCreatFlag == 0x00)
		sprintf((char *)msglog_,"//ICERLog//ICER");
	else
		sprintf((char *)msglog_,"ICER");
	//if (msglog_)
	{
		set_log((char *)msglog_);
	}

#endif

	return SUCCESS;
}

void log_msg(int inLogLevel,const char* fmt, ...)
{
#if READER_MANUFACTURERS==WINDOWS_API
	va_list args;
	FILE *fp=NULL;
	char cLogFile[20],CurTime[20],sdir[200],mlog[MAX_XML_FILE_SIZE];
	int i= 0x00,nRtn= 0x00;
	struct tm * today=NULL;
	time_t now;

	if(srTxnData.srParameter.gLogFlag != 1 || (inLogLevel == LOG_LEVEL_FLOW && srTxnData.srParameter.ICERFlowDebug != '1'))
		return;

	memset(cLogFile,0,sizeof(cLogFile));
	memset(CurTime,0,sizeof(CurTime));
	memset(sdir,0,sizeof(sdir));
 	memset(mlog,0,sizeof(mlog));
	if(srTxnData.srParameter.chFolderCreatFlag == '1' || srTxnData.srParameter.chFolderCreatFlag == 0x00)
	{
	 	strcpy(sdir,(const char *)gCurrentFolder);
		strcat(sdir,"\\ICERLog"); //dir folder
	}

  	time(&now);
	today=localtime(&now);
	if(srTxnData.srParameter.chFolderCreatFlag == '1' || srTxnData.srParameter.chFolderCreatFlag == 0x00)
		strftime(cLogFile,20,"/ICER%Y%m%d.log",today);
	else
		strftime(cLogFile,20,"ICER%Y%m%d.log",today);
	strcat(sdir,cLogFile);
	strftime(CurTime,20,"%Y/%m/%d %H:%M:%S",today);

	fp = fopen(sdir,"a+");
	if(fp==NULL)
	{
		return;
	}
	va_start(args,fmt); //將取出的指標(va_list) 指到第一個不定參數
	vsprintf(mlog,fmt,args);//把參數args按照fmt指定的格式，寫到mlog中
	va_end(args);
	fprintf(fp,"%s\n",CurTime);
	fprintf(fp,"%s\n",mlog);
	fclose(fp);
#elif READER_MANUFACTURERS==WINDOWS_CE_API
va_list args;
FILE *fp=NULL;
char cLogFile[20],CurTime[20],sdir[200],mlog[MAX_XML_FILE_SIZE];
int i= 0x00,nRtn= 0x00;
struct tm * today=NULL;
//time_t now;
SYSTEMTIME now;
//add By Tim
	if(srTxnData.srParameter.gLogFlag != 1 || (inLogLevel == LOG_LEVEL_FLOW && srTxnData.srParameter.ICERFlowDebug == '0'))
		return;

	memset(cLogFile,0,sizeof(cLogFile));
	memset(CurTime,0,sizeof(CurTime));
	memset(sdir,0,sizeof(sdir));
 	memset(mlog,0,sizeof(mlog));
	if(srTxnData.srParameter.chFolderCreatFlag == '1' || srTxnData.srParameter.chFolderCreatFlag == 0x00)
	{
	 	strcpy(sdir,(const char *)gCurrentFolder);
		strcat(sdir,"\\ICERLog"); //dir folder
	}

//  	time(&now);
	GetLocalTime(&now);
	if(srTxnData.srParameter.chFolderCreatFlag == '1' || srTxnData.srParameter.chFolderCreatFlag == 0x00)
		//strftime(cLogFile,20,"/ICER%Y%m%d.log",today);
		sprintf(cLogFile, "/ICER%04d%02d%02d.log", now.wYear, now.wMonth, now.wDay);
	else
		//strftime(cLogFile,20,"ICER%Y%m%d.log",today);
		sprintf(cLogFile, "ICER%04d%02d%02d.log", now.wYear, now.wMonth, now.wDay);

	strcat(sdir,cLogFile);
	//strftime(CurTime,20,"%Y/%m/%d %H:%M:%S",today);
	sprintf(CurTime, "%04d%02d%02d %02d:%02d:%02d", now.wYear, now.wMonth, now.wDay, now.wHour,now.wMinute,now.wSecond);

	/*{
		va_start(args,fmt); //將取出的指標(va_list) 指到第一個不定參數
		vsprintf(mlog,fmt,args);//把參數args按照fmt指定的格式，寫到mlog中
		va_end(args);
	printf("%s\n",mlog);
	}*/
	fp = fopen(sdir,"a+");
	if(fp==NULL)
	{
		return;
	}
	va_start(args,fmt); //將取出的指標(va_list) 指到第一個不定參數
	vsprintf(mlog,fmt,args);//把參數args按照fmt指定的格式，寫到mlog中
	va_end(args);
	fprintf(fp,"%s\n",CurTime);
	fprintf(fp,"%s\n",mlog);
	fclose(fp);
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
time_t utTime;
struct tm * gTime;
char CurTime[20];
char msg[MAX_XML_FILE_SIZE], mlog[MAX_XML_FILE_SIZE];
va_list args;
struct timeval tv;

	if(srTxnData.srParameter.gLogFlag != 1 || (inLogLevel == LOG_LEVEL_FLOW && srTxnData.srParameter.ICERFlowDebug == '0'))
		return;

	assert(fmt != 0);

	va_start(args, fmt); //撠??箇???(va_list) ?蝚砌???摰???
	vsprintf(mlog, fmt, args);//???筠rgs?fmt???撘?撖怠mlog銝?
	va_end(args);

	#if READER_MANUFACTURERS==ANDROID_API
	__android_log_print(ANDROID_LOG_INFO,"*************", "%s",mlog);
	#endif

	gettimeofday(&tv,NULL);
	time(&utTime);
	gTime=localtime(&utTime);
	memset(CurTime,0,sizeof(CurTime));
	strftime(CurTime,20,"%Y/%m/%d %H:%M:%S",gTime);
	sprintf(msg, "%s.%03ld(Start)\n%s\n", CurTime,tv.tv_usec / 1000, mlog);//Ctime撠???ASCII摮葡
	#if READER_MANUFACTURERS==ANDROID_API
    write(fd_, msg, strlen(msg));
	#else
	if(write(fd_, msg, strlen(msg)) < 0) {
		syslog(LOG_MAKEPRI(LOG_USER, LOG_INFO), "%s", mlog);
	}
	#endif

#elif READER_MANUFACTURERS==NE_SYMLINK_API

char mlog[MAX_XML_FILE_SIZE],sdir[50];
va_list args;
unsigned long ulMSec = 0L;
BYTE bDateTime[20];
//int inRetVal;

	//assert(fmt != 0);

	if(srTxnData.srParameter.gLogFlag != 1 || (inLogLevel == LOG_LEVEL_FLOW && srTxnData.srParameter.ICERFlowDebug == '0'))
		return;

	memset(sdir,0,sizeof(sdir));
	memset(bDateTime,0,sizeof(bDateTime));

	if(srTxnData.srParameter.chFolderCreatFlag == '1' || srTxnData.srParameter.chFolderCreatFlag == 0x00)
		sprintf(sdir,"ICERLog");

	sprintf(sdir,"ICER");
	ECC_GetCurrentDateTime_Lib(bDateTime,&ulMSec);
	memcpy(&sdir[4],bDateTime,8);
	sprintf(&sdir[12],".log");

	//memset(mlog,0,sizeof(mlog));
	//inRetVal = inFileGetSize(sdir,FALSE);
	//if(inRetVal > 0)
	//	inFileRead(sdir,(BYTE *)mlog,inRetVal,inRetVal);

	memset(mlog,0,sizeof(mlog));
	memcpy(&mlog[0],&bDateTime[0],4);
	mlog[4] = '/';
	memcpy(&mlog[5],&bDateTime[4],2);
	mlog[7] = '/';
	memcpy(&mlog[8],&bDateTime[6],2);
	mlog[10] = ' ';
	memcpy(&mlog[11],&bDateTime[8],2);
	mlog[13] = ':';
	memcpy(&mlog[14],&bDateTime[10],2);
	mlog[16] = ':';
	memcpy(&mlog[17],&bDateTime[12],2);
	mlog[19] = 0x0D;
	mlog[20] = 0x0A;
	//sprintf(&mlog[strlen(mlog)],"\n");
	//inFileAppend(sdir,(BYTE *)mlog,strlen(mlog));
	memcpy(ptrECCTempLog, (BYTE *)mlog, strlen(mlog));
	ptrECCTempLog += strlen(mlog);
	//ptrECCTempLog += strlen(mlog) + 4;
//	DBG_Put_String(9, (UCHAR *)&"*********");
//	DBG_Put_String(strlen(mlog), (UCHAR *)mlog);
	ECC_ICERAPI_PutMsg(0x8A,mlog,strlen(mlog));


	memset(mlog,0,sizeof(mlog));
	va_start(args, fmt);
	vsprintf(mlog, (char *)fmt, args);
	va_end(args);
	mlog[strlen(mlog)] = 0x0D;
	mlog[strlen(mlog)] = 0x0A;
	//sprintf(&mlog[strlen(mlog)],"\n");
	//inFileAppend(sdir,(BYTE *)mlog,strlen(mlog));
	memcpy(ptrECCTempLog, (BYTE *)mlog, strlen(mlog));
	ptrECCTempLog += strlen(mlog);
	//ptrECCTempLog += 25 * (strlen(mlog) / 25) + 50;
//	DBG_Put_String(strlen(mlog), (UCHAR *)mlog);
//	DBG_Put_String(9, (UCHAR *)&"#########");
	ECC_ICERAPI_PutMsg(0x8A,mlog,strlen(mlog));


#else
char mlog[MAX_XML_FILE_SIZE],sdir[50];
va_list args;
unsigned long ulMSec = 0L;
BYTE bDateTime[20];
BYTE bMSec[10];
//int inRetVal;

	//assert(fmt != 0);

	if(srTxnData.srParameter.gLogFlag != 1 || (inLogLevel == LOG_LEVEL_FLOW && srTxnData.srParameter.ICERFlowDebug == '0'))
		return;

	memset(sdir,0,sizeof(sdir));
	memset(bDateTime,0,sizeof(bDateTime));

	if(srTxnData.srParameter.chFolderCreatFlag == '1' || srTxnData.srParameter.chFolderCreatFlag == 0x00)
		sprintf(sdir,"ICERLog");

	sprintf(sdir,"ICER");
	ECC_GetCurrentDateTime_Lib(bDateTime,&ulMSec);
	memcpy(&sdir[4],bDateTime,8);
	sprintf(&sdir[12],".log");

	//memset(mlog,0,sizeof(mlog));
	//inRetVal = inFileGetSize(sdir,FALSE);
	//if(inRetVal > 0)
	//	inFileRead(sdir,(BYTE *)mlog,inRetVal,inRetVal);

	memset(mlog,0,sizeof(mlog));
	memcpy(&mlog[0],&bDateTime[0],4);
	mlog[4] = '/';
	memcpy(&mlog[5],&bDateTime[4],2);
	mlog[7] = '/';
	memcpy(&mlog[8],&bDateTime[6],2);
	mlog[10] = ' ';
	memcpy(&mlog[11],&bDateTime[8],2);
	mlog[13] = ':';
	memcpy(&mlog[14],&bDateTime[10],2);
	mlog[16] = ':';
	memcpy(&mlog[17],&bDateTime[12],2);
	sprintf((char *)bMSec, ".%03ld", ulMSec);
	memcpy(&mlog[19], (char *)bMSec, 4);
	mlog[23] = 0x0D;
	mlog[24] = 0x0A;
	//sprintf(&mlog[strlen(mlog)],"\n");
  #ifdef FLASH_SYSTEM
	inFileAppend(FILE_LOG,(BYTE *)mlog,strlen(mlog));
  #else
	inFileAppend(sdir,(BYTE *)mlog,strlen(mlog));
  #endif

	memset(mlog,0,sizeof(mlog));
	va_start(args, fmt);
	vsprintf(mlog, (char *)fmt, args);
	va_end(args);
	mlog[strlen(mlog)] = 0x0D;
	mlog[strlen(mlog)] = 0x0A;
	//sprintf(&mlog[strlen(mlog)],"\n");
  #ifdef FLASH_SYSTEM
	inFileAppend(FILE_LOG,(BYTE *)mlog,strlen(mlog));
  #else
	inFileAppend(sdir,(BYTE *)mlog,strlen(mlog));
  #endif

  //printf("Ray Log:%s",mlog);

#endif
}

void fnUnPack(BYTE *BINData,int BINSize,BYTE *ASCDATA)
{
	int i;

	for(i = 0; i < BINSize;i ++)
	{
		sprintf((char *)&ASCDATA[i*2], "%02X", BINData[i]);
	}
}

void fnPack(char *ASCDATA,int ASCSize,BYTE *BINData)
{
	int i,inCnt = 0;

	for(i = 0; i < ASCSize; i++)
	{
		if(ASCDATA[i] >= '0' && ASCDATA[i] <= '9')
			BINData[inCnt] = (ASCDATA[i] - '0') << 4;
		else if(ASCDATA[i] >= 'A' && ASCDATA[i] <= 'F')
			BINData[inCnt] = (ASCDATA[i] - 'A' + 10) << 4;
		else if(ASCDATA[i] >= 'a' && ASCDATA[i] <= 'f')
			BINData[inCnt] = (ASCDATA[i] - 'a' + 10) << 4;
		i++;
		if(ASCDATA[i] >= '0' && ASCDATA[i] <= '9')
			BINData[inCnt] += ASCDATA[i] - '0';
		else if(ASCDATA[i] >= 'A' && ASCDATA[i] <= 'F')
			BINData[inCnt] += ASCDATA[i] - 'A' + 10;
		else if(ASCDATA[i] >= 'a' && ASCDATA[i] <= 'f')
			BINData[inCnt] += ASCDATA[i] - 'a' + 10;
		inCnt++;
	}
}

void vdWriteLog(BYTE *bLogMsg,int inLogMsgLen,int inLogType,int inTxnType)
{
	char chASCLog[MAX_XML_FILE_SIZE];
	int inRetVal;
	
	memset(chASCLog,0x00,sizeof(chASCLog));
	if(inLogType == WRITE_TM_REQ_LOG || inLogType == WRITE_TM_RES_LOG)
	{
		char chFileName[20];

		memset(chFileName,0x00,sizeof(chFileName));
		strcat(&chASCLog[strlen(chASCLog)],"TM ");
		switch(inLogType)
		{
			case WRITE_TM_REQ_LOG:
				strcat(&chASCLog[strlen(chASCLog)],"REQ :");
				strcat(chFileName,FILE_REQ);
				break;
			case WRITE_TM_RES_LOG:
				strcat(&chASCLog[strlen(chASCLog)],"RES :");
				strcat(chFileName,FILE_RES);
				break;
		}
		inRetVal = inGetFileDataByPassEnterKey(chFileName,bLogMsg,inLogMsgLen,(BYTE *)&chASCLog[strlen(chASCLog)]);
		if(inRetVal < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"%s Write Log Error!!",chFileName);
			return;
		}
	}
	else if(inLogType == WRITE_DLL_REQ_LOG || inLogType == WRITE_DLL_RES_LOG)
	{
		if(srTxnData.srParameter.gReaderMode == DLL_READER)
			strcat(&chASCLog[strlen(chASCLog)],"DLL ");
		else //if(srTxnData.srParameter.gReaderMode == 1 || srTxnData.srParameter.gReaderMode == 2)
			strcat(&chASCLog[strlen(chASCLog)],"RC ");
		switch(inLogType)
		{
			case WRITE_DLL_REQ_LOG:
				strcat(&chASCLog[strlen(chASCLog)],"IN ");
				break;
			case WRITE_DLL_RES_LOG:
				strcat(&chASCLog[strlen(chASCLog)],"OUT ");
				break;
		}
		strcat(&chASCLog[strlen(chASCLog)],chGetDLLName(inTxnType));
		if(inLogType == WRITE_DLL_RES_LOG)
			sprintf(&chASCLog[strlen(chASCLog)],"(%04x)",srTxnData.srIngData.inReaderSW);
		strcat(&chASCLog[strlen(chASCLog)],":");
		if(inLogMsgLen == 0)
			strcat(&chASCLog[strlen(chASCLog)],"No Data!!");
		else
			fnUnPack(bLogMsg,inLogMsgLen,(BYTE *)&chASCLog[strlen(chASCLog)]);
	}
	else if(inLogType == WRITE_XML_SEND_LOG || inLogType == WRITE_XML_RECV_LOG)
	{
		UnionUShort usTmp;

		memset(usTmp.Buf,0x00,sizeof(usTmp.Value));

		strcat(&chASCLog[strlen(chASCLog)],"ICER ");
		switch(inLogType)
		{
			case WRITE_XML_SEND_LOG:
				strcat(&chASCLog[strlen(chASCLog)],"SEND ");
				break;
			case WRITE_XML_RECV_LOG:
				strcat(&chASCLog[strlen(chASCLog)],"RECV ");
				break;
		}

		if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
			strcat(&chASCLog[strlen(chASCLog)],"CMAS ");

		switch(inTxnType)
		{
			case TXN_NORMAL:
				strcat(&chASCLog[strlen(chASCLog)],"NORMAL:");
				break;
			case TXN_REVERSAL:
				strcat(&chASCLog[strlen(chASCLog)],"REVERSAL:");
				break;
			case TXN_ADVICE:
				strcat(&chASCLog[strlen(chASCLog)],"ADVICE:");
				break;
			case TXN_UPLOAD:
				strcat(&chASCLog[strlen(chASCLog)],"UPLOAD:");
				break;
		}

		/*if(srTxnData.srParameter.chTCPIP_SSL == SSL_PROTOCOL && inLogType == WRITE_XML_RECV_LOG)
		{
			strcat(&chASCLog[strlen(chASCLog)],"(SSL)");
			memcpy(&chASCLog[strlen(chASCLog)],bLogMsg,inLogMsgLen);
		}
		else //if(srTxnData.srParameter.chTCPIP_SSL == TCPIP_PROTOCOL)*/
		{
			if(srTxnData.srParameter.chTCPIP_SSL == SSL_PROTOCOL)
				strcat(&chASCLog[strlen(chASCLog)],"(SSL)");
			if(srTxnData.srParameter.inAdditionalTcpipData != NO_ADD_DATA && (inTxnType == TXN_ADVICE || inTxnType == TXN_REVERSAL))
			{
				usTmp.Value = ((bLogMsg[0] / 16) * 1000) +
							  ((bLogMsg[0] % 16) * 100) +
							  ((bLogMsg[1] / 16) * 10) +
							   (bLogMsg[1] % 16);
			}
			else
			{
				usTmp.Buf[0] = bLogMsg[1];
				usTmp.Buf[1] = bLogMsg[0];
			}
			sprintf((char *)&chASCLog[strlen(chASCLog)],"%04d",usTmp.Value);
			memcpy(&chASCLog[strlen(chASCLog)],&bLogMsg[2],inLogMsgLen - 2);
		}
	}
	else if(inLogType == WRITE_R6_SEND_LOG || inLogType == WRITE_R6_RECV_LOG)
	{
		strcat(&chASCLog[strlen(chASCLog)],"R6 ");
		switch(inLogType)
		{
			case WRITE_R6_SEND_LOG:
				strcat(&chASCLog[strlen(chASCLog)],"SEND ");
				break;
			case WRITE_R6_RECV_LOG:
				strcat(&chASCLog[strlen(chASCLog)],"RECV ");
				break;
		}
		strcat(&chASCLog[strlen(chASCLog)],chGetDLLName(inTxnType));
		strcat(&chASCLog[strlen(chASCLog)]," :");
		fnUnPack(bLogMsg,inLogMsgLen,(BYTE *)&chASCLog[strlen(chASCLog)]);
	}

	log_msg(LOG_LEVEL_ERROR,(const char *)chASCLog);
}

void GetCurrentDateTime(unsigned char *bDate)
{
#if READER_MANUFACTURERS==CS_PAXSXX
	Get_Time(bDate);
#elif  READER_MANUFACTURERS==SYSTEX_CASTLES || READER_MANUFACTURERS==NE_SYMLINK_API
  ulong ulMSec;

  //
  ECC_GetCurrentDateTime_Lib(bDate, &ulMSec);
#elif READER_MANUFACTURERS==WINDOWS_CE_API
	SYSTEMTIME now;
	GetLocalTime(&now);

	sprintf((char *)bDate,"%04d%02d%02d%02d%02d%02d",now.wYear, now.wMonth, now.wDay, now.wHour,now.wMinute,now.wSecond);
#else
time_t utTime;
struct tm * gTime;

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		unsigned long ulUnixTime;
	#else
		long ulUnixTime;
	#endif

#if READER_MANUFACTURERS==NE_VX520
	time((time_t *)&ulUnixTime);
#else
	time(&ulUnixTime);
#endif

	utTime= ulUnixTime;
	gTime=localtime(&utTime);
	sprintf((char *)bDate,"%04d%02d%02d%02d%02d%02d",gTime->tm_year+1900,gTime->tm_mon+1,gTime->tm_mday,gTime->tm_hour,gTime->tm_min,gTime->tm_sec);
#endif
}

void UnixToDateTime(unsigned char *bUnixDate,unsigned char *bDate,int inSize)
{
char   tmpUnix[50];
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
/*struct tm *lpUTCTime=NULL;
//char bBuf[10];

    time_t t;
    t=time(NULL);
    //log_msg(LOG_LEVEL_ERROR,"-----time1:%s------",ctime(&t));
	memset(tmpUnix,0,sizeof(tmpUnix));
	//memset(bBuf,0x00,sizeof(bBuf));
	//memcpy(bBuf,(char *)&t,sizeof(time_t));
	//log_msg(LOG_LEVEL_ERROR,"time_t t(%d)=%02x %02x %02x %02x %02x %02x %02x %02x ",sizeof(time_t),bBuf[0],bBuf[1],bBuf[2],bBuf[3],bBuf[4],bBuf[5],bBuf[6],bBuf[7]);
	memcpy((char *)&t,bUnixDate,4);
	//log_msg(LOG_LEVEL_ERROR,"time_2 t(%d)=%02x %02x %02x %02x ",sizeof(time_t),bUnixDate[0],bUnixDate[1],bUnixDate[2],bUnixDate[3]);
    lpUTCTime=gmtime(&t);
	if(lpUTCTime == NULL)
	    log_msg(LOG_LEVEL_ERROR,"lpUTCTime is null");
	//lpUTCTime=gmtime((time_t *)bUnixDate);
	strftime(tmpUnix,20,"%Y%m%d%H%M%S",lpUTCTime);
	//log_msg(LOG_LEVEL_ERROR,"time_3 %s",tmpUnix);
#elif  READER_MANUFACTURERS==WINDOWS_CE_API*/
int inDay, inTotalDay, inRTC_Year, inRTC_Yday, inRTC_Mon, inRTC_Mday, inRTC_Hour, inRTC_Min, inRTC_Sec;
#if  READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
uint32_t ulUnixTime,ulTolSecond;
#else
unsigned long ulUnixTime,ulTolSecond;
#endif

int inDayOfMon[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

	ulUnixTime = (bUnixDate[3] * 256 * 256 * 256) +
		     (bUnixDate[2] * 256 * 256) +
		     (bUnixDate[1] * 256) +
		     (bUnixDate[0]);

	if(ulUnixTime == 0L)
	{
		memcpy(bDate,"19700101000000",inSize);
		return;
	}

	if(ulUnixTime < (10956 * 86400))//10956 * 86400表示從1970年開始到1999年為止的總秒數
		return;
	ulUnixTime -= (10956 * 86400);//先把1970年開始到1999年為止的總秒數扣掉,表示從2000年開始計算
	inTotalDay = ulUnixTime / 86400;//86400表示1天24小時的總秒數
	ulTolSecond = ulUnixTime % 86400;//不足一天的總秒數

	//計算inYear
	for(inRTC_Year=2000;;inRTC_Year++)
	{
		inDay = 365;
		if(inICERChkSpecialYear(inRTC_Year) == 0)//檢查 inYear 是否是閏年
			inDay = 366;

		if(inTotalDay <= inDay)//inTotalDay 總天數少於第 inYear 年的 inDay 天數就離開
			break;

		inTotalDay -= inDay;
	}
	inRTC_Yday = inTotalDay;

	//計算inMon
	for(inRTC_Mon=1;;inRTC_Mon++)
	{
		inDayOfMon[1] = 28;//先設定2月為28天;
		if(inRTC_Mon == 2)
			if(inICERChkSpecialYear(inRTC_Year) == 0)//inYear表示該年,檢查該年是否是閏年
				inDayOfMon[1] = 29;//若為閏年,2月為29天

		if(inTotalDay <= inDayOfMon[inRTC_Mon - 1])//inTotalDay 總天數少於該年的第inDayOfMon[inMon - 1]月天數就離開
			break;

		inTotalDay -= inDayOfMon[inRTC_Mon - 1];
	}

	//計算inDay
	inRTC_Mday = inTotalDay;

	//計算inHour
	inRTC_Hour = (int )(ulTolSecond / 3600);//一小時有3600秒
	ulTolSecond %= 3600;//不足一小時的總秒數

	//計算inMin
	inRTC_Min = (int )(ulTolSecond / 60);//一分鐘有60秒

	//計算inSec
	inRTC_Sec = (int )(ulTolSecond % 60);//不足一分鐘的總秒數

	sprintf(tmpUnix,"%04d%02d%02d%02d%02d%02d", inRTC_Year, inRTC_Mon, inRTC_Mday, inRTC_Hour, inRTC_Min, inRTC_Sec);
#else
	ECC_UnixToDateTime(bUnixDate,(BYTE *)tmpUnix);
#endif
	memcpy(bDate,tmpUnix,inSize);

}

//#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
//#if  READER_MANUFACTURERS==WINDOWS_CE_API
int inICERChkSpecialYear(int inYear)
{

	if((inYear % 4) != 0)//不是閏年就傳回 1
		return(1);

	//每100年不閏,但每400年要閏
	if((inYear % 100) == 0)//100的倍數
		if((inYear % 400) != 0)//且不是400的倍數,就不閏,傳回 1
			return(1);

	return(0);
}
//#endif

long lnDateTimeToUnix(unsigned char *bDate,unsigned char *bTime)
{
unsigned long tcnt= 0x00;//1.0.5.2
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
const char init_mdays[]={31,28,31,30,31,30,31,31,30,31,30,31};
int  year= 0x00,mon= 0x00,day= 0x00,hour= 0x00,min= 0x00,sec= 0x00;
char temp[10];
int  i= 0x00;
char mdays[12];

	memset(temp,0,sizeof(temp));
	memset(mdays,0,sizeof(mdays));
	memcpy(mdays,init_mdays,12);
	memcpy(temp,bTime,6);
	sec = atoi(&temp[4]);
	temp[4] = 0;
	min = atoi(&temp[2]);
	temp[2] = 0;
	hour = atoi(&temp[0]);
	memcpy(temp,bDate,8);
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

	//OutDate[0] = (BYTE )tcnt;
	//OutDate[1] = (BYTE )(tcnt >> 8);
	//OutDate[2] = (BYTE )(tcnt >> 16);
	//OutDate[3] = (BYTE )(tcnt >> 24);
#else
BYTE temp1[20],temp2[5];

	memset(temp1,0x00,sizeof(temp1));
	memset(temp2,0x00,sizeof(temp2));
	memcpy(&temp1[0],bDate,8);
	memcpy(&temp1[8],bTime,8);
	ECC_DateTimeToUnix(temp1,temp2);
#ifdef ECC_BIG_ENDIAN
	memcpy((char *)&tcnt,temp2,4);
#else
	tcnt = temp2[0] + (temp2[1] * 256) + (temp2[2] * 256 * 256) + (temp2[3] * 256 * 256 * 256);
#endif
	//log_msg(LOG_LEVEL_ERROR,"lnDateTimeToUnix 1 = %s",temp1);
	//log_msg(LOG_LEVEL_ERROR,"lnDateTimeToUnix 2(%02x%02x%02x%02x) = %ld",temp2[0],temp2[1],temp2[2],temp2[3],tcnt);
#endif

  	return tcnt;
}

int inInitialTxn()
{
BYTE bInData[10],bOutData[10];
int inRetVal,i;

	srTxnData.srParameter.gLogCnt = 30;//此參數fnLogFileProcedure()要用!!
	inRetVal = inInitialSystemXML((char *)FILE_INI);
	if(inRetVal < SUCCESS)
		return inRetVal;

	vdECCFolderProcess();
	fnLogFileProcedure();

	inRetVal = inCheckProcess();//要有參數才能檢查!!
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inInitialTxn Fail 0:%d",inRetVal);
		//return inRetVal;
	}

	memset(bInData,0x00,sizeof(bInData));
	memset(bOutData,0x00,sizeof(bOutData));
	inRetVal = inLoadLibrary();
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inInitialTxn Fail 1:%d",inRetVal);
		return inRetVal;
	}

	for(i=0;i<5;i++)
	{
		inRetVal = inReaderCommand(DLL_OPENCOM,&srXMLData,bInData,bOutData,FALSE,0,FALSE);
		if(inRetVal >= SUCCESS)
			break;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
		Sleep(10);
#endif
	}

	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inInitialTxn Fail 2:%d",inRetVal);
		//vdWriteLog(NULL,0,WRITE_TM_REQ_LOG,0);
		return inRetVal;
	}

	if(srTxnData.srParameter.chTCPIP_SSL == SSL_PROTOCOL)
	{
		inRetVal = inSSLInitial();
		if(inRetVal != SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inInitialTxn Fail 3:%d",inRetVal);
			return inRetVal;
		}
	}

	if(inFileGetSize((char *)FILE_REQ_OK,FALSE) < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inInitialTxn Fail 4:%d",ICER_REQ_FORMAT_FAIL);
		return ICER_REQ_FORMAT_FAIL;
	}

	//if(inFileGetSize((char *)FILE_REQ_CMAS_BAT,FALSE) > SUCCESS)
	//	inFileDelete((char *)FILE_REQ_CMAS_BAT,FALSE);

	//if(inFileGetSize((char *)FILE_RES_CMAS_BAT,FALSE) > SUCCESS)
	//	inFileDelete((char *)FILE_RES_CMAS_BAT,FALSE);

	//if(inFileGetSize((char *)FILE_RES_CMAS2_BAT,FALSE) > SUCCESS)
	//	inFileDelete((char *)FILE_RES_CMAS2_BAT,FALSE);

	if(srTxnData.srParameter.chReaderPortocol == READER_SYMLINK)
	{
		inRetVal = inCustomizrdReaderPorotocol(FALSE);
		if(inRetVal != SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inInitialTxn Fail 5:%d",inRetVal);
			return inRetVal;
		}
	}

	log_msg(LOG_LEVEL_ERROR,"inInitialTxn = %d",inRetVal);

	return inRetVal;
}

//---------------------------------------------------------------------//
/* ---------------------------------------------------------------------
  vdPad .
	Input parameters:
		. bInData -- 欲轉換的字串資料
		. inOutSize -- 轉換後的字串總長度
		. fFlag---- TRUE=左靠/FALSE=右靠
		. chFiller---- 用此資料補滿轉換後不足的部份
	Returned status:
		. bOutData -- 轉換後的字串資料
--------------------------------------------------------------------- */
void vdPad(char *bInData,char *bOutData,int inOutSize,char fFlag,char chFiller)
{
int inLen,inCnt = 0;
BYTE bBuf[50];

	memset(bBuf,0x00,sizeof(bBuf));
	inLen = strlen((char *)bInData);
	if(inOutSize < inLen)
	{
		memcpy(bOutData,bInData,inOutSize);
		return;
	}

	if(fFlag == FALSE)//右靠
	{
		memset(bBuf,chFiller,inOutSize - inLen);//左補chFiller
		inCnt = inOutSize - inLen;
	}

	memcpy(&bBuf[inCnt],bInData,inLen);
	inCnt += inLen;

	if(fFlag == TRUE)//左靠
		memset(&bBuf[inCnt],chFiller,inOutSize - inLen);//右補chFiller

	memcpy(bOutData,bBuf,inOutSize);
}

void vdReplace(char *bInData,int inInLen,char *bOutData,char chOldFiller,char chNewFiller)
{
int i;

	for(i=0;i<inInLen;i++)
	{
		bOutData[i] = (bInData[i] == chOldFiller)?chNewFiller:bInData[i];
	}
}

int inQueryPoint(int inTxnType)
{
int inRetVal = SUCCESS;
BYTE bOutData[300];

	memset(bOutData,0x00,sizeof(bOutData));

	/*inRetVal = inXMLCheckTMInputData(inTxnType,&srXMLData);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inQueryPoint Fail 1:%d",inRetVal);
		return ICER_REQ_FORMAT_FAIL;
	}*/

	inRetVal = inXMLCheckTMCanNotInputData(inTxnType,&srXMLData);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inQueryPoint Fail 2:%d",inRetVal);
		return ICER_REQ_FORMAT_FAIL;
	}

	if(inTxnType == TXN_SETTLE)
		inRetVal = inGetSettleData();
	else
	{
		/*if(!srTxnData.srIngData.fTMRetryFlag)
		{
			inRetVal = inQueryCardData(inTxnType);//RC_READ_CARD_BASIC_DATA
			if(inRetVal < SUCCESS)
			{
				log_msg(LOG_LEVEL_ERROR,"inQueryPoint Fail 3:%d",inRetVal);
				return inRetVal;
			}
		}*/

		inRetVal = inGetCardData(inTxnType,FALSE);//RC_ADD_VALUE
	}
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inQueryPoint Fail 4:%d",inRetVal);
		return inRetVal;
	}

	inRetVal = inCheckECCEVEnough(inTxnType);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inQueryPoint Fail 5:%d",inRetVal);
		return inRetVal;
	}

	if(srTxnData.srIngData.fRetryFlag)
		return SUCCESS;

	if(inTxnType != TXN_SETTLE)
	{
		char fInitial = FALSE;
		if(!srTxnData.srIngData.fTMRetryFlag)
		{
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_0200,(char *)srTxnData.srIngData.anCardID,TRUE,NODE_NO_SAME,VALUE_NAME,TRUE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_0212,(char *)srTxnData.srIngData.anAutoloadFlag,FALSE,NODE_NO_SAME,VALUE_NAME,TRUE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_0409,(char *)srTxnData.srIngData.anAutoloadAmount,FALSE,NODE_NO_SAME,VALUE_NAME,TRUE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_0410,(char *)srTxnData.srIngData.anCardEVBeforeTxn,FALSE,NODE_NO_SAME,VALUE_NAME,TRUE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_1402,(char *)srTxnData.srIngData.anExpiryDate,FALSE,NODE_NO_SAME,VALUE_NAME,TRUE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_1400,(char *)"9912",FALSE,NODE_NO_SAME,VALUE_NAME,TRUE);
		}
		else
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_1400,(char *)"9912",TRUE,NODE_NO_SAME,VALUE_NAME,TRUE);
		//vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_1400,"9912",FALSE,NODE_NO_SAME,VALUE_NAME,FALSE);
		vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_3700,(char *)srTxnData.srIngData.anRRN,FALSE,NODE_NO_SAME,VALUE_NAME,TRUE);
		vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_4100,(char *)srTxnData.srIngData.anDeviceID,FALSE,NODE_NO_SAME,VALUE_NAME,TRUE);
		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_5548,(char *)bOutData,0,0);
		if(inRetVal < SUCCESS)//沒有5548
		{
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_SAME,TAG_NAME_START,TRUE);
			if(srTxnData.srIngData.ucPurseVersionNumber == MIFARE)
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554801,(char *)srTxnData.srIngData.anAData,FALSE,NODE_NO_ADD,VALUE_NAME,TRUE);
			else //if(srTxnData.srIngData.ucPurseVersionNumber == LEVEL2)
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554808,(char *)srTxnData.srIngData.anAData2,FALSE,NODE_NO_ADD,VALUE_NAME,TRUE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,TRUE);
		}
		else
		{
			inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/TRANS",FALSE);
			if(srTxnData.srIngData.ucPurseVersionNumber == MIFARE)
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554801,(char *)srTxnData.srIngData.anAData,TRUE,NODE_NO_ADD,VALUE_NAME,TRUE);
			else //if(srTxnData.srIngData.ucPurseVersionNumber == LEVEL2)
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554808,(char *)srTxnData.srIngData.anAData2,TRUE,NODE_NO_ADD,VALUE_NAME,TRUE);
			inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/T5548",FALSE);
			fInitial = TRUE;
		}
		if(srTxnData.srIngData.ucPurseVersionNumber == MIFARE)
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_6400,(char *)srTxnData.srIngData.anMAC,fInitial,NODE_NO_SAME,VALUE_NAME,TRUE);
		else //if(srTxnData.srIngData.ucPurseVersionNumber == LEVEL2)
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_6406,(char *)srTxnData.srIngData.anMAC2,fInitial,NODE_NO_SAME,VALUE_NAME,TRUE);
		inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/TRANS",FALSE);
	}
	else
	{
		vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_3700,(char *)srTxnData.srIngData.anRRN,TRUE,NODE_NO_SAME,VALUE_NAME,TRUE);
		vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_4100,(char *)srTxnData.srIngData.anDeviceID,FALSE,NODE_NO_SAME,VALUE_NAME,TRUE);
		inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/TRANS",FALSE);
		srTxnData.srIngData.chAdviceFlag = 'N';
	}

	if(fIsOnlineTxn() == FALSE)
	{
		BYTE bSendBuf[MAX_XML_FILE_SIZE];

		memset(bSendBuf,0x00,sizeof(bSendBuf));
		inRetVal = inBuildSendPackage(HOST_ICER,&srXMLData,bSendBuf,REVERSAL_OFF,ADVICE_ON,STORE_BATCH_OFF);
		if(inRetVal < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inQueryPoint Fail 6:%d",inRetVal);
			return inRetVal;
		}
	}
	else
	//if(fIsOnlineTxn() == TRUE)
	{
		inRetVal = inSendRecvXML(HOST_ICER,&srXMLData,inTxnType);
		if(inRetVal < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inQueryPoint Fail 7:%d",inRetVal);
			return inRetVal;
		}
	}
	//else
	//	srTxnData.srIngData.fICERAdviceFlag = TRUE;

	return inRetVal;
}

int inDeductOrRefundProcess(int inTxnType)
{
int inRetVal = SUCCESS;
long lnDeductAmt,lnAddAmt,lnDiscountAmt;
//BOOL fDeductFlag = FALSE;

	lnDeductAmt = lnAddAmt = lnDiscountAmt = 0L;
	inRetVal = inXMLGetAmt(&srXMLData,(char *)TAG_NAME_0403,&lnDeductAmt,0);
	/*if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inDeductOrRefundProcess Fail 1:%d",inRetVal);
		return SUCCESS;
	}*/

	inRetVal = inXMLGetAmt(&srXMLData,(char *)TAG_NAME_0407,&lnAddAmt,0);
	/*if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inDeductOrRefundProcess Fail 2:%d",inRetVal);
		return SUCCESS;
	}*/

	inRetVal = inXMLGetAmt(&srXMLData,(char *)TAG_NAME_0406,&srTxnData.srIngData.lnDiscountAmt,0);
	if(inRetVal >= SUCCESS)//有回折扣金額
	{
		//inRetVal = inXMLGetAmt(&srXMLData,(char *)TAG_NAME_0400,&lnECCAmt,0);
		if(srTxnData.srIngData.lnECCAmt > srTxnData.srIngData.lnDiscountAmt)//交易金額大於折扣金額
			lnDiscountAmt = srTxnData.srIngData.lnECCAmt - srTxnData.srIngData.lnDiscountAmt;
		else
		{
			log_msg(LOG_LEVEL_ERROR,"inDeductOrRefundProcess Fail 0:%ld,%ld",srTxnData.srIngData.lnECCAmt,lnDiscountAmt);
			lnDiscountAmt = 0;
		}
	}

#if 0
	if(lnDeductAmt == lnAddAmt)
	{
		log_msg(LOG_LEVEL_ERROR,"inDeductOrRefundProcess Fail 1:%ld = %ld",lnDeductAmt,lnAddAmt);
		return SUCCESS;
	}

	if(lnDeductAmt > lnAddAmt)
	{
		fDeductFlag = TRUE;
		srTxnData.srIngData.lnECCAmt = lnDeductAmt - lnAddAmt;
	}
	else
		srTxnData.srIngData.lnECCAmt = lnAddAmt - lnDeductAmt;
#endif

	/*if(srTxnData.srIngData.lnECCAmt <= 0L)
	{
		log_msg(LOG_LEVEL_ERROR,"inDeductOrRefundProcess Fail 3:%ld",srTxnData.srIngData.lnECCAmt);
		return SUCCESS;
	}*/

	inRetVal = SUCCESS;
	if(lnAddAmt > 0)
		inRetVal = inECCDeductOrRefund(CARD_ADD,inTxnType,lnAddAmt);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inDeductOrRefundProcess Fail 1:%d",inRetVal);
		return inRetVal;
	}

	inRetVal = SUCCESS;
	if(lnDeductAmt > 0)
		inRetVal = inECCDeductOrRefund(CARD_DEDUCT,inTxnType,lnDeductAmt);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inDeductOrRefundProcess Fail 2:%d",inRetVal);
		return inRetVal;
	}

	inRetVal = SUCCESS;
	if(lnDiscountAmt > 0)
	{
//		int inCntStart = 0,inCntEnd = 0;

		srTxnData.srIngData.inTransType = TXN_ECC_DEDUCT;
		srTxnData.srIngData.lnECCAmt = lnDiscountAmt;

		/*if(srTxnData.srParameter.chBatchFlag == CMAS_BATCH)
		{
			inRetVal = inXMLSearchValueByTag(&srXMLData,(char *)TAG_TRANS_XML_HEADER,&inCntStart,&inCntEnd,0);
			if(inRetVal < SUCCESS)//沒有 <TransXML>,要補上!!
			{
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_TRANS_XML_HEADER,(char *)srTxnData.srIngData.bTmp,TRUE,NODE_NO_SAME,TAG_NAME_START,FALSE);
				inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"TRANS",FALSE);
				inXMLAppendData(&srXMLData,(char *)TAG_TRANS_XML_END,strlen(TAG_TRANS_XML_END),srTxnData.srIngData.bTmp,0,TAG_NAME_END,TRUE);
			}
		}*/

		inRetVal = inDoECCAmtTxn(TXN_ECC_DEDUCT);
	}
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inDeductOrRefundProcess Fail 4:%d",inRetVal);
		return inRetVal;
	}

	return inRetVal;
}

int ECC_strncasecmp(char *chString1,char *chString2,int inLen)
{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
char *str1,*str2;
int inRetVal;

	str1 = (char *)ECC_calloc(1,strlen(chString1) + 1);
	memset(str1,0x00,strlen(chString1) + 1);
	str2 = (char *)ECC_calloc(1,strlen(chString2) + 1);
	memset(str2,0x00,strlen(chString2) + 1);
	ECC_LCase(chString1,str1);
	ECC_LCase(chString2,str2);
	str1[strlen(chString1)] = 0x00;
	str2[strlen(chString2)] = 0x00;
	inRetVal = strncmp(str1,str2,inLen);
	ECC_free((BYTE *)str1);
	ECC_free((BYTE *)str2);

#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
int inRetVal;

	inRetVal = strncasecmp(chString1,chString2,inLen);

#else
char *str1,*str2;
int inRetVal;

	str1 = (char *)ECC_calloc(1,strlen(chString1) + 1);
	memset(str1,0x00,strlen(chString1) + 1);
	str2 = (char *)ECC_calloc(1,strlen(chString2) + 1);
	memset(str2,0x00,strlen(chString2) + 1);
	ECC_LCase(chString1,str1);
	ECC_LCase(chString2,str2);
	str1[strlen(chString1)] = 0x00;
	str2[strlen(chString2)] = 0x00;
	inRetVal = strncmp(str1,str2,inLen);
	ECC_free((BYTE *)str1);
	ECC_free((BYTE *)str2);

#endif

	return(inRetVal);
}

void ECC_LCase(char *InStr,char *OutStr)
{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
int i;

	for(i=0;i<(int )strlen(InStr);i++)
	{
		OutStr[i] = InStr[i];
		if(InStr[i] >= 'A' && InStr[i] <= 'Z')
			OutStr[i] += 0x20;
	}
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API

	//LCase(InStr,OutStr);

#else
	int i;

	for(i=0;i<(int )strlen(InStr);i++)
	{
		OutStr[i] = InStr[i];
		if(InStr[i] >= 'A' && InStr[i] <= 'Z')
			OutStr[i] += 0x20;
	}

#endif
}

BOOL fIsOnlineTxn(void)
{

	if(srTxnData.srIngData.chAdviceFlag == 'Y')
		return FALSE;
	return TRUE;
}

int inGetREQData()
{
int inRetVal = 0;
char /*chTMSerialNumber[6 + 1],*/T1100[6 + 1],T1101[6 + 1],bBuf[100],chTMProcessCode[6 + 1],chTxDate[8 + 1],chTxTime[6 + 1],T4214[5 + 1];
//BYTE bDateTime[20];
char fRetryFlag = FALSE;
BOOL fCMASNoReqT1101Flag = FALSE;

	inRetVal = inCheckXMLDataFormat(&srXMLData);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inGetREQData Fail 0:%d",inRetVal);
		return inRetVal;
	}

	// 判斷Process code 是否重複
	inRetVal = inXMLCheckTagDuplicated(&srXMLData, TAG_NAME_0300);
	if (inRetVal > SUCCESS)
	{	
		log_msg(LOG_LEVEL_ERROR,"inGetREQData Fail 0-1:%d",inRetVal);
		return ICER_REQ_DATA_FAIL;
	}
	
	inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_0100,srTxnData.srIngData.chTMMTI,sizeof(srTxnData.srIngData.chTMMTI),0);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inGetREQData Fail 1-1:%d",inRetVal);
		return inRetVal;
	}
	log_msg(LOG_LEVEL_FLOW,"ucAutoload_fail_flag 3= %d", gTmpData.ucAutoload_fail_flag);

	// 新Retry
	if(!memcmp(srTxnData.srIngData.chTMMTI,"0000",4))
	{
		memset(chTMProcessCode,0x00,sizeof(chTMProcessCode));
		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_0300,chTMProcessCode,sizeof(chTMProcessCode) - 1,0);
		if(inRetVal < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inGetREQData Fail 1-2:%d",inRetVal);
			return inRetVal;
		}

		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_1200,chTxTime,sizeof(chTxTime) - 1,0);
		if(inRetVal < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inGetREQData Fail 1-3:%d",inRetVal);
			return inRetVal;
		}

		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_1300,chTxDate,sizeof(chTxDate) - 1,0);
		if(inRetVal < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inGetREQData Fail 1-4:%d",inRetVal);
			return inRetVal;
		}

		vdFreeXMLDOC(&srXMLData);
		inRetVal = inParseXML((char *)FILE_REQ_BAK,(BYTE *)NULL,TRUE,0,&srXMLData,0);
		if(inRetVal < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inGetREQData Fail 1-5:%d",inRetVal);
			return ICER_ERROR;
		}

		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_0100,srTxnData.srIngData.chTMMTI,sizeof(srTxnData.srIngData.chTMMTI),0);
		if(inRetVal < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inGetREQData Fail 1-6:%d",inRetVal);
			return inRetVal;
		}

		//@ 在Retry的情況下，T1200和T1300要以傳入的值為主
		chTxTime[sizeof(chTxTime)-1] = 0x00;
		chTxDate[sizeof(chTxDate)-1] = 0x00;
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_1200,(BYTE *)chTxTime,TRUE);
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_1300,(BYTE *)chTxDate,TRUE);

		srTxnData.srIngData.fTMRetryFlag = TRUE;
		fRetryFlag = TRUE;//CMAS用
	}
	else
	{
		gTmpData.ucAutoload_fail_flag = FALSE;	// 非重試交易一律把此Flag改為FALSE
		if(inFileGetSize((char *)FILE_REQ_BAK,FALSE) >= SUCCESS)
			inFileDelete((char *)FILE_REQ_BAK,FALSE);
		inFileCopy((char *)FILE_REQ,(char *)FILE_REQ_BAK);
	}

	inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_0300,srTxnData.srIngData.chTMProcessCode,sizeof(srTxnData.srIngData.chTMProcessCode),0);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inGetREQData Fail 1:%d",inRetVal);
		return inRetVal;
	}

	if(fRetryFlag == TRUE && memcmp(chTMProcessCode,srTxnData.srIngData.chTMProcessCode,sizeof(srTxnData.srIngData.chTMProcessCode) - 1))
	{
		log_msg(LOG_LEVEL_ERROR,"inGetREQData Fail 1-7:[%s]:[%s]",chTMProcessCode,srTxnData.srIngData.chTMProcessCode);
		return ICER_ERROR;
	}


	//memcpy(srTxnData.srREQData.bTMProcessCode,srTxnData.srIngData.chTMProcessCode,6);

	if( !memcmp(srTxnData.srIngData.chTMProcessCode,"270100",6) ||
		!memcmp(srTxnData.srIngData.chTMProcessCode,"500060",6) ||
		!memcmp(srTxnData.srIngData.chTMProcessCode,"900000",6))
		srTxnData.srParameter.chOnlineFlag = ICER_ONLINE;

	inXMLGetData(&srXMLData,(char *)TAG_NAME_5599,&srTxnData.srREQData.chDiscountType,1,0);

	memset(T1100,0x00,sizeof(T1100));
	inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_1100,T1100,sizeof(T1100) - 1,0);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inGetREQData Fail 2-1:%d",inRetVal);
		return inRetVal;
	}
	/*srTxnData.srREQData.ulT1100 = */srTxnData.srREQData.ulT1101 = srTxnData.srIngData.ulTMSerialNumber = atol(T1100);

	memset(T1101,0x00,sizeof(T1101));
	if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
	{	//不論REQ有無T1101,都不管,直接去tmp裡面抓!!
		//inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_1101,T1101,sizeof(T1101) - 1,0);
		//if(inRetVal < SUCCESS)
		{
			log_msg(LOG_LEVEL_FLOW,"inGetREQData Fail 2-2(No T1101 For CMAS Host):%d",inRetVal);
			fCMASNoReqT1101Flag = TRUE;//TM沒帶T1101,去ICERAPI.tmp抓!!
			//memcpy(T1101,gTmpData.ucCMAS_REQ_T1101,sizeof(gTmpData.ucCMAS_REQ_T1101));//移到inCheckProcess()裡面!!
			//memcpy(T1101,T1100,strlen(T1100));
		}

		//悠遊卡扣款
		if(!memcmp(srTxnData.srIngData.chTMProcessCode,"606100",6))
		{
			//維揚需求!!
			inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_5501,bBuf,sizeof(bBuf),0);
			log_msg(LOG_LEVEL_ERROR,"inGetREQData Fail 2-?:%d",inRetVal);
			if(inRetVal < SUCCESS)//沒有T5501
			{
				log_msg(LOG_LEVEL_ERROR,"inGetREQData Fail 2-3(No T5501 For CMAS Host):%d",inRetVal);
				return inRetVal;
			}
			else if (inRetVal > 8)		//@ 新增需求，若5501非8位長度，視為格式錯誤
			{
				log_msg(LOG_LEVEL_ERROR,"inGetREQData Fail 2-4(T5501 Format Error):%s",bBuf);
				return -108;
				
			}
			/*	商店卡模式不只有扣款才要帶入T5503，移到下面

			if(srTxnData.srParameter.chCMASMode == '2')
			{
				memset(bBuf,0x00,sizeof(bBuf));
				inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_5503,bBuf,sizeof(bBuf),0);
				if(inRetVal < SUCCESS)
				{
					log_msg(LOG_LEVEL_ERROR,"inGetREQData Fail 2-4(No T5503 For CMAS Host):%d",inRetVal);
					return inRetVal;
				}

				vdPad(bBuf,(char *)srTxnData.srREQData.bT5503,sizeof(srTxnData.srREQData.bT5503) - 1,FALSE,0x30);
			}*/
		}
		//針對商店卡交易會另外由T5503帶入TM Location ID，但登入或是配對5503等同4200，就不需要帶入了
		if ((srTxnData.srParameter.chCMASMode == '2') && 
			((memcmp(srTxnData.srIngData.chTMProcessCode,"881999",6) != 0) && (memcmp(srTxnData.srIngData.chTMProcessCode,"882999",6) != 0)))
		{
			memset(bBuf,0x00,sizeof(bBuf));
			inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_5503,bBuf,sizeof(bBuf),0);
			if(inRetVal < SUCCESS)
			{
				log_msg(LOG_LEVEL_ERROR,"inGetREQData Fail 2-5(No T5503 For CMAS Host):%d",inRetVal);
				return inRetVal;
			}
			vdPad(bBuf,(char *)srTxnData.srREQData.bT5503,sizeof(srTxnData.srREQData.bT5503) - 1,FALSE,0x30);
		}
	}
	else
		memcpy(T1101,T1100,strlen(T1100));
	//srTxnData.srREQData.ulT1101 = srTxnData.srIngData.ulCMASHostSerialNumber = atol(T1101);//移到inCheckProcess()裡面!!
	if(fCMASNoReqT1101Flag == TRUE)
	{
		srTxnData.srREQData.ulT1101++;
		srTxnData.srIngData.ulCMASHostSerialNumber++;
	}
	log_msg(LOG_LEVEL_FLOW,"ulCMASHostSerialNumber 2(%d):%ld,%ld",fCMASNoReqT1101Flag,srTxnData.srIngData.ulCMASHostSerialNumber,srTxnData.srREQData.ulT1101);

	if(fRetryFlag == TRUE)
	{
		memcpy(srTxnData.srIngData.chTxTime,chTxTime,sizeof(srTxnData.srIngData.chTxTime) - 1);
		inRetVal = SUCCESS;
	}
	else
		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_1200,srTxnData.srIngData.chTxTime,sizeof(srTxnData.srIngData.chTxTime),0);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inGetREQData Fail 3:%d",inRetVal);
		return inRetVal;
	}
	memcpy(srTxnData.srIngData.chCMASTxTime,srTxnData.srIngData.chTxTime,sizeof(srTxnData.srIngData.chCMASTxTime) - 1);

	if(fRetryFlag == TRUE)
	{
		memcpy(srTxnData.srIngData.chTxDate,chTxDate,sizeof(srTxnData.srIngData.chTxDate) - 1);
		inRetVal = SUCCESS;
	}
	else
		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_1300,srTxnData.srIngData.chTxDate,sizeof(srTxnData.srIngData.chTxDate),0);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inGetREQData Fail 4:%d",inRetVal);
		return inRetVal;
	}
	memcpy(srTxnData.srIngData.chCMASTxTime,srTxnData.srIngData.chTxDate,sizeof(srTxnData.srIngData.chCMASTxTime) - 1);

	inXMLGetData(&srXMLData,(char *)TAG_NAME_5597,&srTxnData.srIngData.chAdviceFlag,sizeof(srTxnData.srIngData.chAdviceFlag),0);
	if((inXMLGetData(&srXMLData,(char *)TAG_NAME_554809,(char *)srTxnData.srIngData.bTMData,sizeof(srTxnData.srIngData.bTMData),0)) >= SUCCESS)//沒有6402
		srTxnData.srIngData.fTMRetryFlag = TRUE;//ICER用

	if((inXMLGetData(&srXMLData,(char *)TAG_NAME_554810,(char *)srTxnData.srIngData.bTMData,sizeof(srTxnData.srIngData.bTMData),0)) >= SUCCESS)//沒有6402
		srTxnData.srIngData.fTMRetryFlag = TRUE;//ICER用

	inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_4840,bBuf,sizeof(bBuf),0);
	if(inRetVal >= SUCCESS && bBuf[0] == '1')
		srTxnData.srIngData.fTMRetryFlag = TRUE;//CMAS用

	//if(srTxnData.srParameter.chOnlineFlag != R6_ONLINE)//ICER數位付交易時,需RRN
	//if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
	memcpy(&srTxnData.srIngData.anRRN[0],&srTxnData.srIngData.chTxDate[2],6);
	if(fIsETxn(srTxnData.srIngData.inTransType) == TRUE)//數位悠遊卡交易
		sprintf((char *)&srTxnData.srIngData.anRRN[6],"%06ld",srTxnData.srIngData.ulTMSerialNumber);
	else
		sprintf((char *)&srTxnData.srIngData.anRRN[6],"%06ld",srTxnData.srIngData.ulCMASHostSerialNumber);

	if(srTxnData.srParameter.gDLLVersion == 1)
	{
		//memcpy(srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMID,&chTMSerialNumber[0],2);
		//memcpy(srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMAgentNumber,&chTMSerialNumber[2],4);
		//memcpy(srTxnData.srIngData.chICERTMAgentNumber,&chTMSerialNumber[2],4);
		memcpy(srTxnData.srIngData.chICERTMAgentNumber,&T1100[2],4);
	}
	else
	{
		//memcpy(srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMID,&chTMSerialNumber[0],2);
		//memcpy(srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber,&chTMSerialNumber[2],4);
		//memcpy(srTxnData.srIngData.chICERTMAgentNumber,&chTMSerialNumber[2],4);
		memcpy(srTxnData.srIngData.chICERTMAgentNumber,&T1100[2],4);
	}
	if(inXMLGetAmt(&srXMLData,(char *)TAG_NAME_0400,&srTxnData.srIngData.lnECCAmt,0) == ICER_ERROR)
	{
		if(srTxnData.srParameter.chBasicCheckQuotaFlag == '1' && !memcmp(srTxnData.srIngData.chTMProcessCode,"296000",6))
		{
			log_msg(LOG_LEVEL_ERROR,"inGetREQData Fail 4-0(296000 & BasicCheckQuotaFlag,But No T0400)!!");
			return inRetVal;
		}
	}
	log_msg(LOG_LEVEL_ERROR,"srTxnData.srIngData.lnECCAmt 1 = %d",srTxnData.srIngData.lnECCAmt);

	if(!memcmp(srTxnData.srIngData.chTMProcessCode,"825799",6))
		inXMLGetAmt(&srXMLData,(char *)TAG_NAME_0409,&srTxnData.srIngData.lnECCAmt,0);
	else if(!memcmp(srTxnData.srIngData.chTMProcessCode,"606100",6))
	{
		inXMLGetAmt(&srXMLData,(char *)TAG_NAME_0442,&srTxnData.srIngData.lnECCAmt1,0);
		inXMLGetAmt(&srXMLData,(char *)TAG_NAME_0443,&srTxnData.srIngData.lnECCAmt2,0);
		inXMLGetAmt(&srXMLData,(char *)TAG_NAME_0444,&srTxnData.srIngData.lnECCAmt3,0);
		//菸酒
		inXMLGetAmt(&srXMLData,(char *)TAG_NAME_0445,&srTxnData.srIngData.lnECCAmt4,0);
		inXMLGetAmt(&srXMLData,(char *)TAG_NAME_0446,&srTxnData.srIngData.lnECCAmt5,0);

		memset(bBuf,0x00,sizeof(bBuf));
		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_4847,bBuf,sizeof(bBuf),0);
		if(inRetVal >= SUCCESS)
			srTxnData.srREQData.chOneDayQuotaFlag = bBuf[0];

		memset(bBuf,0x00,sizeof(bBuf));
		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_4848,bBuf,sizeof(bBuf),0);
		if(inRetVal >= SUCCESS)
			srTxnData.srREQData.chOnceQuotaFlag = bBuf[0];
	}
	else if(!memcmp(srTxnData.srIngData.chTMProcessCode,"620061",6))
	{
		inXMLGetAmt(&srXMLData,(char *)TAG_NAME_0442,&srTxnData.srIngData.lnECCAmt1,0);
		inXMLGetAmt(&srXMLData,(char *)TAG_NAME_0443,&srTxnData.srIngData.lnECCAmt2,0);
		inXMLGetAmt(&srXMLData,(char *)TAG_NAME_0444,&srTxnData.srIngData.lnECCAmt3,0);
		//菸酒
		inXMLGetAmt(&srXMLData,(char *)TAG_NAME_0445,&srTxnData.srIngData.lnECCAmt4,0);
		inXMLGetAmt(&srXMLData,(char *)TAG_NAME_0446,&srTxnData.srIngData.lnECCAmt5,0);
	}
	else if(!memcmp(srTxnData.srIngData.chTMProcessCode,"816100",6))
	{
		inXMLGetAmt(&srXMLData,(char *)TAG_NAME_0404,&srTxnData.srREQData.lnT0404,0);
		if(srTxnData.srREQData.chDiscountType == 'D')
		{
			if(inFileGetSize((char *)FILE_ICER_REV,FALSE) >= SUCCESS)
			{
				log_msg(LOG_LEVEL_ERROR,"inGetREQData Fail 4-1:%d",inRetVal);
				return ICER_ERROR;
			}
			if(inFileGetSize((char *)FILE_ICER_REV_BAK,FALSE) < SUCCESS)
			{
				log_msg(LOG_LEVEL_ERROR,"inGetREQData Fail 4-2:%d",inRetVal);
				return ICER_ERROR;
			}
		}
	}

	/*memset(bDateTime,0x00,sizeof(bDateTime));
	memcpy((char *)&bDateTime[0],"00",2);
	strcat((char *)&bDateTime[2],(char *)srTxnData.srIngData.anRRN,sizeof(srTxnData.srIngData.anRRN) - 1);
	fngetUnixTimeCnt((BYTE *)&&srTxnData.srIngData.ulTxnTime,bDateTime);*/

	if(srTxnData.srParameter.inAdditionalTcpipData != NO_ADD_DATA)
	{
		memset(T1100,0x00,sizeof(T1100));
		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_1102,T1100,sizeof(T1100) - 1,0);
		if(inRetVal < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inGetREQData Fail 5:%d",inRetVal);
			return inRetVal;
		}
		srTxnData.srIngData.ulBankSTAN = atol(T1100);

		memset(T1100,0x00,sizeof(T1100));
		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_1103,T1100,sizeof(T1100) - 1,0);
		if(inRetVal < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inGetREQData Fail 6:%d",inRetVal);
			return inRetVal;
		}
		srTxnData.srIngData.ulBankInvoiceNumber = atol(T1100);
	}

	memset(bBuf,0x00,sizeof(bBuf));
	inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_4107,(char *)bBuf,sizeof(srTxnData.srParameter.chPOS_ID),0);
	if(inRetVal >= SUCCESS)
	{
		memset(srTxnData.srParameter.chPOS_ID,0x00,sizeof(srTxnData.srParameter.chPOS_ID));
		memcpy(srTxnData.srParameter.chPOS_ID,bBuf,strlen(bBuf));
	}

	memset(bBuf,0x00,sizeof(bBuf));
	inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_4117,(char *)bBuf,sizeof(srTxnData.srParameter.chPOS_ID2),0);
	if(inRetVal >= SUCCESS)
	{
		memset(srTxnData.srParameter.chPOS_ID2,0x00,sizeof(srTxnData.srParameter.chPOS_ID2));
		memcpy(srTxnData.srParameter.chPOS_ID2,bBuf,strlen(bBuf));
	}

	inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_4108,(char *)bBuf,1,0);
	if(inRetVal >= SUCCESS && bBuf[0] == '1')
		srTxnData.srREQData.ucT4108 = '1';
	else
		srTxnData.srREQData.ucT4108 = '0';

	memset(srTxnData.srREQData.bT0200,0x00,sizeof(srTxnData.srREQData.bT0200));
	inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_0200,(char *)srTxnData.srREQData.bT0200,sizeof(srTxnData.srREQData.bT0200) - 1,0);
	/*if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inGetREQData Fail 3:%d",inRetVal);
		return inRetVal;
	}*/

	memset(T4214,0x00,sizeof(T4214));
	inXMLGetData(&srXMLData,(char *)TAG_NAME_4214,T4214,sizeof(T4214) - 1,0);
	memset(&srTxnData.srIngData.ulSubLocId,0x00,sizeof(srTxnData.srIngData.ulSubLocId));
	srTxnData.srIngData.ulSubLocId = atol(T4214);

	if(inXMLGetAmt(&srXMLData,(char *)"T0421", &srTxnData.srIngData.lnForeignAmt,0) != ICER_ERROR)
		srTxnData.srIngData.fForeignTxnFlag = TRUE;

	return SUCCESS;
}
int inECCstrchr(BYTE *bInData,int inInLen,char chWord)
{
int i;

	for(i=0;i<inInLen;i++)
	{
		if(bInData[i] == chWord)
			return i;
	}

	return SUCCESS;
}

int inDefaultDataCheck(long lnProcCode)
{
int inRetVal;
char chSettleDate[8 + 1],chTxTime[7];
long lnNowDate,lnSettleDate;

	if(lnProcCode == 900000 || srTxnData.srParameter.inMustSettleDate <= 0)
		return SUCCESS;

	inRetVal = inFileGetSize((char *)FILE_ICER_ADV,FALSE);
	if(inRetVal <= SUCCESS)//無Advice資料,回傳成功!!
		return SUCCESS;

	memset(chSettleDate,0x00,sizeof(chSettleDate));
	inRetVal = inGetSettleDate((BYTE *)chSettleDate);
	if(inRetVal != SUCCESS)
		return SUCCESS;
	memset(chTxTime,0x30,sizeof(chTxTime));
	chTxTime[6] = 0x00;
	lnNowDate = lnDateTimeToUnix((BYTE *)srTxnData.srIngData.chTxDate,(BYTE *)chTxTime);
	lnSettleDate = lnDateTimeToUnix((BYTE *)chSettleDate,(BYTE *)chTxTime);
	if((lnSettleDate + (srTxnData.srParameter.inMustSettleDate*24*60*60)) <= lnNowDate)
	//if((lnSettleDate + (2*24*60*60)) <= lnNowDate)
		return MUST_SETTLE;

	return SUCCESS;
}

void vdTxnEndProcess(int inTxnType,char *chSettleDate)
{
//int inRetVal;
//STRUCT_TMP_DATA srTmpData;
	if(inTxnType == TXN_QUERY_POINT)
		return;

	//memset(&srTmpData,0x00,sizeof(srTmpData));
	//inRetVal = inFileGetSize((char *)FILE_TMP,FALSE);
	//if(inRetVal == sizeof(srTmpData))
	{
		/*inRetVal = inFileRead((char *)FILE_TMP,(BYTE *)&srTmpData,sizeof(srTmpData),sizeof(srTmpData));
		if(inRetVal < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"vdTxnEndProcess failed 1:%d",inRetVal);
			return;
		}*/
		if(gTmpData.fHasTxn == TRUE && inTxnType != TXN_SETTLE)
			return;
	}
	if(inTxnType == TXN_SETTLE)
	{
		memcpy(gTmpData.chSettleDate,chSettleDate,8);
		gTmpData.fHasTxn = FALSE;
	}
	else
		gTmpData.fHasTxn = TRUE;
	inFileWrite((char *)FILE_TMP,(BYTE *)&gTmpData,sizeof(gTmpData));
}

/*int inSettleProcess()
{

	inRetVal = inReversalProcess();
	if(inRetVal != SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inSettleProcess Fail 1:%d",inRetVal);
        return inRetVal;
	}

	inRetVal = inTCPIPAdviceProcess(TRUE);
	if(inRetVal != SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inSettleProcess Fail 2:%d",inRetVal);
        return inRetVal;
	}

	vdTxnEndProcess(inTxnType);

	return SUCCESS;
}*/

void vdStoreDeviceID(BYTE *bDeviceID,BYTE bSPID,BYTE bLocID,BYTE *bCPUDeviceID,BYTE *bCPUSPID,BYTE *bCPULocID,BYTE *bReaderID,BYTE *bReaderFWVersion,BYTE *bCPUSAMID)
{
//STRUCT_TMP_DATA srTmpData;

	sprintf((char *)srTxnData.srIngData.anDeviceID,"%03d%02d%04d",bDeviceID[2],bDeviceID[1] >> 4,((bDeviceID[1] & 0x0F) * 256) + bDeviceID[0]);

	//vdGetTmpData(&gTmpData); //這Func是空的，先Mark

	memcpy((char *)gTmpData.anDeviceID,(char *)srTxnData.srIngData.anDeviceID,strlen((char *)srTxnData.srIngData.anDeviceID));
	memcpy((char *)gTmpData.ucDeviceID,bDeviceID,sizeof(gTmpData.ucDeviceID));
	gTmpData.ucSPID = bSPID;
	gTmpData.ucLocationID = bLocID;
	memcpy((char *)gTmpData.ucCPUDeviceID,bCPUDeviceID,sizeof(gTmpData.ucCPUDeviceID));
	memcpy((char *)gTmpData.ucCPUSPID,bCPUSPID,sizeof(gTmpData.ucCPUSPID));
	memcpy((char *)gTmpData.ucCPULocationID,bCPULocID,sizeof(gTmpData.ucCPULocationID));
	memcpy((char *)gTmpData.ucReaderID,bReaderID,sizeof(gTmpData.ucReaderID));
	memcpy((char *)gTmpData.ucReaderFWVersion,bReaderFWVersion,sizeof(gTmpData.ucReaderFWVersion));
	memcpy((char *)gTmpData.ucCPUSAMID,bCPUSAMID,sizeof(gTmpData.ucCPUSAMID));
	inFileWrite((char *)FILE_TMP,(BYTE *)&gTmpData,sizeof(gTmpData));

	{
		BYTE bBuf[sizeof(STRUCT_TMP_DATA) * 2 + 1];

		memset(bBuf,0x00,sizeof(bBuf));
		fnUnPack((BYTE *)&gTmpData, sizeof(STRUCT_TMP_DATA), bBuf);
		log_msg(LOG_LEVEL_FLOW,"vdStoreDeviceID : %s",bBuf);
	}
}

void vdStoreTMLocationID(BYTE *bTMLocationID)
{
	memset((char *)gTmpData.ucTMLocationID, ' ', sizeof(gTmpData.ucTMLocationID));
	memcpy((char *)gTmpData.ucTMLocationID, bTMLocationID, sizeof(gTmpData.ucTMLocationID));
	inFileWrite((char *)FILE_TMP,(BYTE *)&gTmpData,sizeof(gTmpData));
	log_msg(LOG_LEVEL_FLOW,"vdStoreTMLocationID : %s",bTMLocationID);
}

int inGetTMLocationID(BYTE *bTMLocationID)
{
	BYTE bBuf[10+1];

	memset(bBuf, 0x00, sizeof(bBuf));
	memcpy(bBuf, gTmpData.ucTMLocationID, sizeof(gTmpData.ucTMLocationID));
	if (strlen((char *)bBuf) <= 0)
		return ICER_ERROR;
	memcpy(bTMLocationID, &gTmpData.ucTMLocationID, sizeof(gTmpData.ucTMLocationID));
	log_msg(LOG_LEVEL_FLOW, "inGetTMLocationID : %s", bBuf);
	return SUCCESS;
}

// 把自動加值內容存到tmp檔中   V4005F改存unPack內容
void vdStoreAutoloadData(BYTE *bAutoloadData, int iDataLen)
{
	char chASCLog[MAX_XML_FILE_SIZE];
	memset((char *)gTmpData.ucAutoloadData, 0x00, sizeof(gTmpData.ucAutoloadData));
	memset(chASCLog, 0x00, sizeof(chASCLog));

	fnUnPack(bAutoloadData, iDataLen, (BYTE *)chASCLog);
	memcpy((char *)gTmpData.ucAutoloadData, chASCLog, iDataLen*2);


	inFileWrite((char *)FILE_TMP,(BYTE *)&gTmpData,sizeof(gTmpData));
	log_msg(LOG_LEVEL_FLOW,"vdStoreAutoloadData : %s", chASCLog);
}


// 從tmp讀回autoload交易資料  V4005F改為unpack內容
int inGetAutoloadData(BYTE *bAutoloadData)
{
	int iSize = strlen((char *)gTmpData.ucAutoloadData);
	if (iSize <= 0)
		return ICER_ERROR;

	if (iSize > READER_BUFFER)
		iSize = READER_BUFFER;
	
	log_msg(LOG_LEVEL_FLOW, "inGetAutoloadData : %s", gTmpData.ucAutoloadData);
	
	fnPack((char *)gTmpData.ucAutoloadData, iSize, bAutoloadData);
	//memcpy(bAutoloadData, gTmpData.ucAutoloadData, iSize);
	
	return SUCCESS;
}
int inGetDeviceID(BYTE *bDeviceID/*,STRUCT_TMP_DATA *srTmpData*/)
{
//STRUCT_TMP_DATA srTmpData;

	vdGetTmpData(&gTmpData);

	//inFileRead((char *)FILE_TMP,(BYTE *)srTmpData,sizeof(STRUCT_TMP_DATA),sizeof(STRUCT_TMP_DATA));

	{
		BYTE bBuf[sizeof(STRUCT_TMP_DATA) * 2 + 1];

		memset(bBuf,0x00,sizeof(bBuf));
		fnUnPack((BYTE *)&gTmpData, sizeof(STRUCT_TMP_DATA), bBuf);
		log_msg(LOG_LEVEL_ERROR,"inGetDeviceID : %s",bBuf);
	}

	if(strlen((char *)gTmpData.anDeviceID) <= 0)
		return ICER_ERROR;
	memcpy((char *)bDeviceID,(char *)gTmpData.anDeviceID,strlen((char *)gTmpData.anDeviceID));

	return SUCCESS;
}

int inGetSettleDate(BYTE *bSettleDate)
{
//STRUCT_TMP_DATA srTmpData;

	vdGetTmpData(&gTmpData);

	//inFileRead((char *)FILE_TMP,(BYTE *)&srTmpData,sizeof(srTmpData),sizeof(srTmpData));
	if(strlen((char *)gTmpData.chSettleDate) <= 0)
		return ICER_ERROR;
	
	memcpy((char *)bSettleDate,(char *)gTmpData.chSettleDate,strlen((char *)gTmpData.chSettleDate));
	return SUCCESS;
}

int inGetSettleData()
{
BYTE bInData[READER_BUFFER],bOutData[READER_BUFFER];
int inRetVal;
//STRUCT_TMP_DATA srTmpData;

	memset(srTxnData.srIngData.anDeviceID,0x00,sizeof(srTxnData.srIngData.anDeviceID));
	memset(bInData,0x00,sizeof(bInData));
	memset(bOutData,0x00,sizeof(bOutData));
	inRetVal = inGetDeviceID(srTxnData.srIngData.anDeviceID/*,&srTmpData*/);
	if(inRetVal != SUCCESS)
	{
		if( srTxnData.srParameter.chSignOnMode == SIGN_ON_ONLY ||
			srTxnData.srParameter.chSignOnMode == RESET_OFF_ONLY ||
			srTxnData.srParameter.chSignOnMode == SIGN_ON_AND_RESET_OFF ||
			srTxnData.srParameter.chSignOnMode == NO_SIGN_ON_AND_RESET)
			return(inSignOnQueryProcess2(inRetVal,bInData,bOutData));
		else
			return inSignOnProcess(TXN_ECC_SIGN_ON,bOutData,TRUE);
	}

	return SUCCESS;
}

int inGetFileDataByPassEnterKey(char *chFileName,BYTE *bMsg,int inMsgLen,BYTE *bOutData)
{
int inRetVal,inFileSize,i,inCnt = 0;
BYTE bBuf[MAX_XML_FILE_SIZE];

	memset(bBuf,0x00,sizeof(bBuf));
	if(!memcmp(chFileName,FILE_REQ,strlen(FILE_REQ)))
	{
		inFileSize = inFileGetSize(chFileName,FALSE);
		if(inFileSize > MAX_XML_FILE_SIZE || inFileSize < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inGetFileDataByPassEnterKey failed 1:%d",inFileSize);
			return ICER_ERROR;
		}

		inRetVal = inFileRead(chFileName,bBuf,sizeof(bBuf),inFileSize);
		if(inRetVal < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inGetFileDataByPassEnterKey failed 2:%d",inRetVal);
			return ICER_ERROR;
		}
	}
	else
	{
		inFileSize = inMsgLen;
		memcpy(bBuf,bMsg,inMsgLen);
	}

	for(i=0;i<inFileSize;i++)
	{
		if((bBuf[i] == 0x0D && bBuf[i + 1] == 0x0A) || (bBuf[i] == 0x0A && bBuf[i + 1] == 0x0D))
		//if(bBuf[i] == 0x0D && bBuf[i + 1] == 0x0A)
		{
			i++;
			inFileSize++;
		}
		else if(bBuf[i] != 0x0A)
			bOutData[inCnt++] = bBuf[i];
	}

	return SUCCESS;
}

void vdTrimData(char *chOutData,char *chInData,int inInLen)
{
int i;

	for(i=0;i<inInLen;i++)
	{
		if(chInData[i] != ' ')
		{
			memcpy(chOutData,&chInData[i],inInLen - i);
			return;
		}
	}
}

void vdTrimDataRight(char *chOutData,char *chInData,int inInLen)
{
int i,j = 0;

	for(i=0;i<inInLen;i++)
	{
		if(chInData[i] != ' ')
			chOutData[j++] = chInData[i];
	}
}

void vdBatchDataProcess(BYTE *bOutData,BYTE *bInData1,int inInDataLen1,BYTE *bInData2,int inInDataLen2)
{
SETM_t *srSETM_Info;
STMC_t *srSTMC_Info;

	if(srTxnData.srParameter.gDLLVersion == 1)//1代API
	{
		memcpy(bOutData,bInData1,inInDataLen1);
		if(inInDataLen1 == sizeof(SETM_t))
			memcpy(&bOutData[inInDataLen1],bInData2,inInDataLen2);

		srSETM_Info = (SETM_t *)bOutData;
		vdBatchDataTransfer(srSETM_Info->anEVBeforeTxn_5,sizeof(srSETM_Info->anEVBeforeTxn_5));
		vdBatchDataTransfer(srSETM_Info->anTxnSN_5,sizeof(srSETM_Info->anTxnSN_5));
		vdBatchDataTransfer(srSETM_Info->anTxnAmt_5,sizeof(srSETM_Info->anTxnAmt_5));
		vdBatchDataTransfer(srSETM_Info->anEV_5,sizeof(srSETM_Info->anEV_5));
	}
	else if(srTxnData.srParameter.gDLLVersion == 2)//2代API
	{
		memcpy(bOutData,bInData1,inInDataLen1);
		//if(inInDataLen1 < sizeof(STMC_t))
		//	memcpy(&bOutData[inInDataLen1],bInData2,inInDataLen2);

		srSTMC_Info = (STMC_t *)bOutData;
		vdBatchDataTransfer(srSTMC_Info->anEVBeforeTxn,sizeof(srSTMC_Info->anEVBeforeTxn));
		vdBatchDataTransfer(srSTMC_Info->anTxnSN,sizeof(srSTMC_Info->anTxnSN));
		vdBatchDataTransfer(srSTMC_Info->anTxnAmt,sizeof(srSTMC_Info->anTxnAmt));
		vdBatchDataTransfer(srSTMC_Info->anEV,sizeof(srSTMC_Info->anEV));
	}
}

void vdBatchDataTransfer(BYTE *bTransferData,int inDataLen)
{
int i;

	for(i=0;i<inDataLen;i++)
	{
		if(bTransferData[i] == ' ')
			bTransferData[i] = 0x30;
		else if(bTransferData[i] == '-')
		{
			bTransferData[0] = '-';
			if(i != 0)
				bTransferData[i] = 0x30;
		}
	}

}

int inCheckECCEVEnough(int inTxnType)
{
long lnDeductAmt;

	if(inTxnType != TXN_SALE)
		return SUCCESS;

	lnDeductAmt = 0L;
	inXMLGetAmt(&srXMLData,(char *)TAG_NAME_0403,&lnDeductAmt,0);
	if(lnDeductAmt > 0 && lnDeductAmt > srTxnData.srIngData.lnECCEV)
		if(inCheckCanAutoload(lnDeductAmt) != SUCCESS)
			return AMOUNT_FAIL;

	return SUCCESS;
}

int inCheckCanAutoload(long lnTxnAmt)
{
long lnAutoloadAmt = 0L;
char chBuf[10];

	if(srTxnData.srIngData.anAutoloadFlag[0] != '1')
	{
		log_msg(LOG_LEVEL_ERROR,"inCheckCanAutoload failed 1:%c",srTxnData.srIngData.anAutoloadFlag[0]);
		return ICER_ERROR;
	}

	memset(chBuf,0x00,sizeof(chBuf));
	if(strlen((char *)srTxnData.srIngData.anAutoloadAmount) > 2)
		memcpy(chBuf,srTxnData.srIngData.anAutoloadAmount,strlen((char *)srTxnData.srIngData.anAutoloadAmount) - 2);
	else
		chBuf[0] = '0';
	lnAutoloadAmt = atol(chBuf);
	if(lnAutoloadAmt + srTxnData.srIngData.lnECCEV < lnTxnAmt)
	{
		log_msg(LOG_LEVEL_ERROR,"inCheckCanAutoload failed 2:%d + %d < %d",lnAutoloadAmt,srTxnData.srIngData.lnECCEV,lnTxnAmt);
		return ICER_ERROR;
	}

	return SUCCESS;
}

BOOL fIsMustReversal(void)//自動加值成功,但後續加/扣款失敗,則需Reversal該交易!!
{
int inRetVal;
BYTE bBuf[10];

	inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_554803,(char *)bBuf,0,0);
	if(inRetVal < SUCCESS)//沒有自動加值就離開
		return FALSE;

	inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_554802,(char *)bBuf,0,0);
	if(inRetVal >= SUCCESS)//有成功的加/扣過,就離開
		return FALSE;

	inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_554806,(char *)bBuf,0,0);
	if(inRetVal >= SUCCESS)//有成功的行銷平台帳務,就離開
		return FALSE;

	return TRUE;
}

void vdUpdateCardEV(BYTE *bCardEV,BYTE *bOrgCardEV,int inEVLen,int inOrgEVLen,char *chTagName)
{

	memset(bCardEV,0x00,inEVLen);
	vdTrimData((char *)bCardEV,(char *)bOrgCardEV,inOrgEVLen);
	strcat((char *)bCardEV,"00");
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,chTagName,bCardEV,FALSE);
}

int inASCToLONG(char *chASC,int inASCLen)
{
char chBuf[20];

	memset(chBuf,0x00,sizeof(chBuf));
	memcpy(chBuf,chASC,inASCLen);
	return atol(chBuf);
}

BOOL fIsRetry(int inRetVal)
{

	if(inRetVal == CARD_SUCCESS && srTxnData.srParameter.gDLLVersion == 2 && srTxnData.srIngData.fTMRetryFlag)
	{
		srTxnData.srIngData.fRetryFlag = TRUE;
		return TRUE;
	}

	return FALSE;
}

BOOL fIsECCTxn(int inTransType)
{

	if((inTransType >= TXN_ECC_DEDUCT && inTransType <= TXN_ECC_SETTLE) || inTransType == TXN_ADJECT_CARD)
		return TRUE;

	return FALSE;
}

int inECCTxnProcess(int inTxnType)
{
int inRetVal;

	if(inTxnType == TXN_ECC_SETTLE || inTxnType == TXN_ECC_SETTLE2)
		inRetVal = inECCSettle(inTxnType);
	else if(inTxnType == TXN_ECC_BATCH_UPLOAD)
		inRetVal = inECCBatchUpload();
	else if(inTxnType == TXN_ECC_ADVICE)
		inRetVal = inECCAdvice();
	else if((inTxnType >= TXN_ECC_DEDUCT && inTxnType < TXN_ECC_READ_BASIC) || inTxnType == TXN_ADJECT_CARD)
		inRetVal = inDoECCAmtTxn(inTxnType);
	else if(inTxnType != TXN_ECC_READ_DEDUCT || srTxnData.srParameter.gDLLVersion != 1)
		inRetVal = inDoECCQueryTxn(inTxnType);
	else
		inRetVal = LOAD_DLL_FUNCTION_ERROR;

	if(inRetVal == SUCCESS)
	{
		if(srTxnData.srIngData.inTransType == TXN_ECC_VOID && srTxnData.srREQData.chDiscountType == 'D')
		{
			char chOnlineFlag;//Online Flag,0=R6 / 1=CMAS
			char chBatchFlag;//Batch Flag,0=CMAS Batch / 1=SIS2 Batch / 2=R6
			char chTCPIP_SSL;
			char PacketLenFlag;//PacketLenFlag 0=電文沒有2Byte長度 / 1=電文有2Byte長度

			vdSetICERTxnParameter(&chOnlineFlag,&chBatchFlag,&chTCPIP_SSL,&PacketLenFlag);
			vdReversalOn(HOST_ICER);
			inReversalProcess(HOST_ICER);
			vdGetICERTxnParameter(chOnlineFlag,chBatchFlag,chTCPIP_SSL,PacketLenFlag);
		}
	}

	return inRetVal;
}

int vdUpdateECCDataAfterTxn(int inTxnType,BYTE *bOutData)
{
//	int inExpDateAddr;
	int inCardIDAddr,inCardIDLen,inEVAddr,inEVLen,inDeviceIDAddr,inDeviceIDLen = 0;

	inCardIDAddr = inCardIDLen = inEVAddr = inEVLen = inDeviceIDAddr = inDeviceIDLen = -1;
	if(srTxnData.srParameter.gDLLVersion == 1)
	{
		if(inTxnType == TXN_ECC_READ_CARD)
		{
			unsigned long ulCardID = 0L;
			BYTE anCardID[17 + 1];

			memset(anCardID,0x00,sizeof(anCardID));
			memcpy((char *)&ulCardID,bOutData,4);
			sprintf((char *)anCardID,"%ld",ulCardID);
			vdTrimData((char *)srTxnData.srIngData.anCardID,(char *)anCardID,sizeof(anCardID));
			inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0200,srTxnData.srIngData.anCardID,FALSE);
			return SUCCESS;
		}
		else if(inTxnType == TXN_ECC_READ_BASIC)
		{
#if 1
			BYTE bTmp[30 + 1];
			ReadCardBasicDataL1_TM_Out *srSETM_Info = (ReadCardBasicDataL1_TM_Out *)bOutData;

			memset(bTmp,0x30,sizeof(bTmp));
			bTmp[30] = 0x00;

			/*for(i=0;i<6;i++)//把非ASCII資料轉換成ASCII值,否則,寫RES檔案時,會寫不進去!!
			{
				if(bOutData[i] < 0x20)
					bOutData[i] += 0x30;
			}*/
			srTxnData.srIngData.anAutoloadFlag[0] = srSETM_Info->anAutoLoad[2];
			vdTrimData((char *)srTxnData.srIngData.anAutoloadAmount,(char *)srSETM_Info->anAutoLoadAmt,sizeof(srSETM_Info->anAutoLoadAmt));
			strcat((char *)srTxnData.srIngData.anAutoloadAmount,"00");
			vdUpdateCardInfo(inTxnType,srSETM_Info->anCardID,
									   bTmp,
									   srSETM_Info->anCardType,
									   srSETM_Info->anPersonalProfile,
									   &srSETM_Info->anEV[1],
									   srSETM_Info->anExpiryDate,
									   srSETM_Info->anExpiryDate,
									   bTmp,
									   srSETM_Info->anBankCode,
									   srSETM_Info->anAreaCode,
									   bTmp,
									   bTmp,
									   bTmp,
									   bTmp,
									   srSETM_Info->anTxnSN);
			return SUCCESS;
#else
			inCardIDAddr = 0;
			inCardIDLen = 10;
			inEVAddr = 10;
			inEVLen = 6;
#endif
		}
		else
		{
#if 1
			SETM_t *srSETM_Info = (SETM_t *)bOutData;
			BYTE bTmp[30 + 1];
			int inRetVal;

			inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_554803,(char *)bTmp,0,0);
			if(inRetVal < SUCCESS)//沒有554803
				vdGetECCEVBefore(srSETM_Info->anEVBeforeTxn_5,sizeof(srSETM_Info->anEVBeforeTxn_5));
			memset(bTmp,0x30,sizeof(bTmp));
			vdUpdateCardInfo(inTxnType,srSETM_Info->anCardID_10,
									   bTmp,
									   srSETM_Info->unCardType,
									   srSETM_Info->unPersonalProfile,
									   srSETM_Info->anEV_5,
									   srSETM_Info->anNewExpiryDate,
									   srSETM_Info->anOrgExpiryDate,
									   bTmp,
									   srSETM_Info->unBankCode,
									   srSETM_Info->unAreaCode,
									   bTmp,
									   srSETM_Info->unDeposit_4,
									   srSETM_Info->anDeviceID,
									   bTmp,
									   srSETM_Info->anTxnSN_5);
			return SUCCESS;
#else
			inCardIDAddr = 0;
			inCardIDLen = 10;
			if(inTxnType == TXN_ECC_SET_VALUE)
			{
				inEVAddr = 20;
				inExpDateAddr = 79;
			}
			else
				inEVAddr = 35;
			inEVLen = 5;
			inDeviceIDAddr = 10;
			inDeviceIDLen = 10;
#endif
		}
	}
	else //if(srTxnData.srParameter.gDLLVersion == 2)
	{
		if(inTxnType == TXN_ECC_READ_CARD)
		{
			//unsigned long ulCardID = 0L;
			BYTE anCardID[17 + 1];

			memset(anCardID,0x00,sizeof(anCardID));
			memcpy(anCardID,bOutData,17);
			vdTrimData((char *)srTxnData.srIngData.anCardID,(char *)anCardID,sizeof(anCardID));
			inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0200,srTxnData.srIngData.anCardID,FALSE);
			return SUCCESS;
		}
		else if(inTxnType == TXN_ECC_READ_BASIC)
		{
#if 1
			BYTE bTmp[30 + 1],unPurseVersionNumber[2];
			int i;
			ReadCardBasicDataL2_TM_Out *srSTMC_Info = (ReadCardBasicDataL2_TM_Out *)bOutData;

			memset(bTmp,0x30,sizeof(bTmp));

			for(i=0;i<6;i++)//把非ASCII資料轉換成ASCII值,否則,寫RES檔案時,會寫不進去!!
			{
				if(bOutData[i] < 0x20)
					bOutData[i] += 0x30;
			}
			bTmp[30] = 0x00;
			srTxnData.srIngData.anAutoloadFlag[0] = srSTMC_Info->ucAutoLoad;
			if(srTxnData.srParameter.chAutoLoadMode == '0')
				vdTrimData((char *)srTxnData.srIngData.anAutoloadAmount,(char *)srSTMC_Info->anAutoLoadAmt,sizeof(srSTMC_Info->anAutoLoadAmt));
			else if(srTxnData.srParameter.chAutoLoadMode == '1')
				lnConvertAutoloadAmt(srTxnData.srIngData.lnECCAmt,srSTMC_Info->anAutoLoadAmt,sizeof(srSTMC_Info->anAutoLoadAmt),srSTMC_Info->anEV,sizeof(srSTMC_Info->anEV),srTxnData.srIngData.anAutoloadAmount);
			else if(srTxnData.srParameter.chAutoLoadMode == '2')
				sprintf((char *)srTxnData.srIngData.anAutoloadAmount,"%ld",srTxnData.srParameter.ulMaxALAmt);
			else
			{
				srTxnData.srIngData.anAutoloadAmount[0] = '0';
				log_msg(LOG_LEVEL_ERROR,"srTxnData.srParameter.chAutoLoadMode Error = %02x",srTxnData.srParameter.chAutoLoadMode);
			}
			strcat((char *)srTxnData.srIngData.anAutoloadAmount,"00");
			memcpy(unPurseVersionNumber, srSTMC_Info->anPurseVersionNumber, 2);
			//if(memcmp(srSTMC_Info->anPID,"0000000000000000",16))
			//	memcpy(unPurseVersionNumber,"01",2);
			//else
			//	memcpy(unPurseVersionNumber,"00",2);
			vdUpdateCardInfo(inTxnType,srSTMC_Info->anCardID,
									   srSTMC_Info->anPID,
									   srSTMC_Info->anCardType,
									   srSTMC_Info->anPersonalProfile,
									   srSTMC_Info->anEV,
									   srSTMC_Info->anExpiryDate,
									   srSTMC_Info->anExpiryDate,
									   unPurseVersionNumber,
									   srSTMC_Info->anBankCode,
									   srSTMC_Info->anAreaCode,
									   srSTMC_Info->anSubAreaCode,
									   srSTMC_Info->anDeposit,
									   bTmp,
									   bTmp,
									   srSTMC_Info->anTxnSN);
			return SUCCESS;
#else
			for(i=0;i<6;i++)//把非ASCII資料轉換成ASCII值,否則,寫RES檔案時,會寫不進去!!
			{
				if(bOutData[i] < 0x20)
					bOutData[i] += 0x30;
			}

			if(memcmp(&bOutData[14],"0000000000000000",16))
			{
				inCardIDAddr = 14;
				inCardIDLen = 16;
			}
			else
			{
				inCardIDAddr = 30;
				inCardIDLen = 17;
			}
			inEVAddr = 67;
			inEVLen = 8;
#endif
		}
		else if(inTxnType == TXN_ECC_READ_DEDUCT)
			return SUCCESS;
		else if(inTxnType == TXN_ECC_MULTI_SELECT)
		{
			BYTE bBuf[20 + 1];

			memset(bBuf,0x00,sizeof(bBuf));
			memcpy(bBuf,&bOutData[0],2);
			inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0216,bBuf,FALSE);

			memset(bBuf,0x00,sizeof(bBuf));
			memcpy(bBuf,&bOutData[2],17);
			inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0217,bBuf,FALSE);

			memset(bBuf,0x00,sizeof(bBuf));
			memcpy(bBuf,&bOutData[19],2);
			inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0218,bBuf,FALSE);

			return SUCCESS;
		}
		else if(inTxnType == TXN_ECC_SET_VALUE)
		{
			SET_VALUE_STMC_t *srSTMC_Info = (SET_VALUE_STMC_t *)bOutData;
			BYTE bBuf[5];
			int inRetVal;

			inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_554803,(char *)bBuf,0,0);
			if(inRetVal < SUCCESS)//沒有554803
				vdGetECCEVBefore(srSTMC_Info->anEVBeforeTxn,sizeof(srSTMC_Info->anEVBeforeTxn));
			vdUpdateCardInfo3(inTxnType,srSTMC_Info->anCardID,
										srSTMC_Info->anEVBeforeTxn,
										srSTMC_Info->unNewExpiryDate,
										srSTMC_Info->unOrgExpiryDate);


			if(srTxnData.srParameter.chOnlineFlag == R6_ONLINE)
			{
				BYTE tmp[10];
				
				// Update Process code T0300
				memset(tmp, 0x00, sizeof(tmp));
				memcpy(tmp, srSTMC_Info->anProcessignCode, 6);
				inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,tmp,TRUE);
			}

			return SUCCESS;
		}
		else if(inTxnType == TXN_ECC_STUDENT_SET_VALUE ||
			    inTxnType == TXN_ECC_EXTEND_VALID)
		{
			STMC_t *srSTMC_Info = &((AuthStudentSetValue_TM_Out *)bOutData)->stSTMC_t;
			BYTE bBuf[15];
			BYTE bBuf1[4];
			int inRetVal;

			inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_554803,(char *)bBuf,0,0);
			if(inRetVal < SUCCESS)//沒有554803
				vdGetECCEVBefore(srSTMC_Info->anEVBeforeTxn,sizeof(srSTMC_Info->anEVBeforeTxn));
			vdUpdateCardInfo(inTxnType,srSTMC_Info->anCardID,
									   srSTMC_Info->unPID,
									   srSTMC_Info->unCardType,
									   srSTMC_Info->unPersonalProfile,
									   srSTMC_Info->anEV,
									   srSTMC_Info->unNewExpiryDate,
									   srSTMC_Info->unOrgExpiryDate,
									   srSTMC_Info->unPurseVersionNumber,
									   srSTMC_Info->unBankCode,
									   srSTMC_Info->unAreaCode,
									   srSTMC_Info->unSubAreaCode,
									   srSTMC_Info->unDeposit,
									   srSTMC_Info->anDeviceID,
									   srSTMC_Info->anCPUDeviceID,
									   srSTMC_Info->anTxnSN);

			memset(bBuf,0x00,sizeof(bBuf));
			vdTrimData((char *)bBuf,(char *)((AuthStudentSetValue_TM_Out *)bOutData)->anTxnAmt,8);
			if(inXMLFormatCheck((char *)bBuf,strlen((char *)bBuf),XML_CHECK_NUMERIC) == SUCCESS)
			{
				long lnAmt = 0L;

				lnAmt = atol((char *)bBuf);
				if(lnAmt != 0)
				{
					strcat((char *)bBuf,"00");
					inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0400,bBuf,FALSE);
				}
			}

			BYTE personalProfile;
			fnPack((char *)((AuthStudentSetValue_TM_Out *)bOutData)->stSTMC_t.unPersonalProfile, 2, &personalProfile);
			memset(bBuf,0x00,sizeof(bBuf));
			if (personalProfile == 0x05)
				memcpy(bBuf,((AuthStudentSetValue_TM_Out *)bOutData)->anProfileExpiryDate,8);				
			else
			{
				fnPack((char *)((AuthStudentSetValue_TM_Out *)bOutData)->stSTMC_t.unNewExpiryDate, 8, bBuf1);
				UnixToDateTime(bBuf1, bBuf, 8);
			}
		
			inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4807,bBuf,FALSE);

			if(srTxnData.srParameter.chOnlineFlag == R6_ONLINE)
			{
				BYTE tmp[10];
				
				// Update Process code T0300
				memset(tmp, 0x00, sizeof(tmp));
				memcpy(tmp, srSTMC_Info->anProcessignCode, 6);
				inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,tmp,TRUE);
			}

			return SUCCESS;
		}
		else if(inTxnType == TXN_ECC_FAST_READ_CARD2)
		{
			BYTE bBuf[50],bTmp[50];
			FastReadCard_APDU_Out *DongleOut = (FastReadCard_APDU_Out *)bOutData;

			//T0200
			memset(bBuf,0x00,sizeof(bBuf));
			memset(bTmp,0x00,sizeof(bTmp));
			vdUIntToAsc(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),bTmp,17,TRUE,0x00,10);
			vdTrimDataRight((char *)bBuf,(char *)bTmp,17);
			inXMLAppendData(&srXMLData,(char *)TAG_NAME_0200,strlen(TAG_NAME_0200),bBuf,strlen((char *)bBuf),VALUE_NAME,FALSE);

			//T0216
			vdUnPackToXMLData(&DongleOut->ucCardIDLen,sizeof(DongleOut->ucCardIDLen),&srXMLData,(char *)TAG_NAME_0216,strlen(TAG_NAME_0216),VALUE_NAME,FALSE);

			//T0223
			vdUnPackToXMLData(DongleOut->ucSAK,sizeof(DongleOut->ucSAK),&srXMLData,(char *)TAG_NAME_0223,strlen(TAG_NAME_0223),VALUE_NAME,FALSE);

			//T4800
			vdUnPackToXMLData(&DongleOut->ucPurseVersionNumber,sizeof(DongleOut->ucPurseVersionNumber),&srXMLData,(char *)TAG_NAME_4800,strlen(TAG_NAME_4800),VALUE_NAME,FALSE);

			return SUCCESS;
		}
		else if (inTxnType == TXN_ECC_ADD_INQ)
			return SUCCESS;
		else
		{
			STMC_t *srSTMC_Info = (STMC_t *)bOutData;
			BYTE bBuf[5];
			int inRetVal;

			inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_554803,(char *)bBuf,0,0);
			if(inRetVal < SUCCESS)//沒有554803
				vdGetECCEVBefore(srSTMC_Info->anEVBeforeTxn,sizeof(srSTMC_Info->anEVBeforeTxn));
			vdUpdateCardInfo(inTxnType,srSTMC_Info->anCardID,
									   srSTMC_Info->unPID,
									   srSTMC_Info->unCardType,
									   srSTMC_Info->unPersonalProfile,
									   srSTMC_Info->anEV,
									   srSTMC_Info->unNewExpiryDate,
									   srSTMC_Info->unOrgExpiryDate,
									   srSTMC_Info->unPurseVersionNumber,
									   srSTMC_Info->unBankCode,
									   srSTMC_Info->unAreaCode,
									   srSTMC_Info->unSubAreaCode,
									   srSTMC_Info->unDeposit,
									   srSTMC_Info->anDeviceID,
									   srSTMC_Info->anCPUDeviceID,
									   srSTMC_Info->anTxnSN);

			if(inTxnType == TXN_ECC_ACCUNT_LINK)
			{
				BYTE bBuf[20],bTmp[20];
				AuthAccuntLink_TM_Out *APIOut = (AuthAccuntLink_TM_Out *)bOutData;

				memset(bBuf,0x00,sizeof(bBuf));
				sprintf((char *)bBuf,"%02x",APIOut->ucAutoLoadFlag);
				inXMLAppendData(&srXMLData,(char *)TAG_NAME_0212,strlen(TAG_NAME_0212),bBuf,strlen((char *)bBuf),VALUE_NAME,TRUE);

				//T0409
				memset(bBuf,0x00,sizeof(bBuf));
				memset(bTmp,0x00,sizeof(bTmp));
				vdUIntToAsc(APIOut->ucAutoLoadAmt_2,sizeof(APIOut->ucAutoLoadAmt_2),bTmp,5,TRUE,0x00,10);
				vdTrimDataRight((char *)bBuf,(char *)bTmp,5);
				strcat((char *)bBuf,"00");
				inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0409,bBuf,TRUE);
			}

			if(srTxnData.srParameter.chOnlineFlag == R6_ONLINE)
			{
				BYTE tmp[10];
				
				// Update Process code T0300
				memset(tmp, 0x00, sizeof(tmp));
				memcpy(tmp, srSTMC_Info->anProcessignCode, 6);
				inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,tmp,TRUE);
			}

			return SUCCESS;
		}
	}

#if 0
	memcpy(srTxnData.srIngData.anCardID,&bOutData[inCardIDAddr],inCardIDLen);
	memset(srTxnData.srIngData.anCardEVBeforeTxn,0x00,sizeof(srTxnData.srIngData.anCardEVBeforeTxn));
	vdTrimData((char *)srTxnData.srIngData.anCardEVBeforeTxn,(char *)&bOutData[inEVAddr],inEVLen);
	strcat((char *)srTxnData.srIngData.anCardEVBeforeTxn,"00");
	if(inDeviceIDAddr > -1)
		memcpy(srTxnData.srIngData.anDeviceID,&bOutData[inDeviceIDAddr],inDeviceIDLen);
	if(inTxnType == TXN_ECC_SET_VALUE)
		memcpy(srTxnData.srIngData.anExpiryDate,&bOutData[inExpDateAddr],8);

	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0200,srTxnData.srIngData.anCardID,FALSE);
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0410,srTxnData.srIngData.anCardEVBeforeTxn,FALSE);
	if(inTxnType == TXN_ECC_SET_VALUE)
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_1402,srTxnData.srIngData.anExpiryDate,FALSE);
	if(inDeviceIDAddr > -1)
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4100,srTxnData.srIngData.anDeviceID,FALSE);
	/*vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_0200,(char *)srTxnData.srIngData.anCardID,TRUE,NODE_NO_SAME,VALUE_NAME,FALSE);
	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_0410,(char *)srTxnData.srIngData.anCardEVBeforeTxn,FALSE,NODE_NO_SAME,VALUE_NAME,FALSE);
	if(inTxnType == TXN_ECC_SET_VALUE)
		vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_1402,(char *)srTxnData.srIngData.anExpiryDate,FALSE,NODE_NO_SAME,VALUE_NAME,FALSE);
	if(inDeviceIDAddr > -1)
		vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_4100,(char *)srTxnData.srIngData.anDeviceID,FALSE,NODE_NO_SAME,VALUE_NAME,FALSE);
	inXMLInsertData(&srXMLData,&srXMLActionData,"/TRANS",FALSE);*/

	return SUCCESS;
#endif
}

BOOL fIsRetryTxn(int inTxnType,int inRetVal)
{
BYTE bBuf[5];

	//指定加值 & TimeOut & 有554809 即需Retry處理,不可Reversal
	if(inTxnType == TXN_ADJECT_CARD && inRetVal == CALL_DLL_TIMEOUT_ERROR && (inXMLGetData(&srXMLData,(char *)TAG_NAME_554809,(char *)bBuf,0,0) >= SUCCESS))
		return TRUE;

	return FALSE;
}

void vdIntToAsc(BYTE *bInData,int inInSize,BYTE *bOutData,int inOutSize,char fFlag,char chFiller,int inRadix)
{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
INT64 llnTmp = 0L;
int size = 0;
#endif
BYTE bBuf[30],bTmp[10];

	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	if(bInData[inInSize - 1] >= 0x80)
		memset(bTmp,0xFF,sizeof(bTmp));
	memcpy(bTmp,bInData,inInSize);

/*#ifdef ECC_BIG_ENDIAN
	{
		BYTE bTmp1[10];

		memset(bTmp1,0x00,sizeof(bTmp1));
		vdChangeEndian(bTmp,bTmp1,sizeof(bTmp));
		memset(bTmp,0x00,sizeof(bTmp));
		memcpy(bTmp,bTmp1,sizeof(bTmp));
	}
#endif*/

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API

	memcpy((BYTE *)&llnTmp,bTmp,sizeof(INT64));

	switch(inRadix)
	{
		case 8:
	#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
			size = sprintf((char *)bBuf, "%I64o",llnTmp);
	#endif
	#if READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
			size = sprintf((char *)bBuf, "%llo",llnTmp);
	#endif
			break;
		case 10:
	#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
			size = sprintf((char *)bBuf, "%I64d",llnTmp);
	#endif
	#if READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
			size = sprintf((char *)bBuf, "%lld",llnTmp);
	#endif
			break;
		case 16:
	#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
			size = sprintf((char *)bBuf, "%I64X",llnTmp);
	#endif
	#if READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
			size = sprintf((char *)bBuf, "%llX",llnTmp);
	#endif
			break;
		default:
			break;
	}
#else

	ECC_INT64ToASCII(bTmp,bBuf);

#endif

	vdPad((char *)bBuf,(char *)bOutData,inOutSize,fFlag,chFiller);
}

void vdUIntToAsc(BYTE *bInData,int inInSize,BYTE *bOutData,int inOutSize,char fFlag,char chFiller,int inRadix)
{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
UINT64 llnTmp = 0L;
int size = 0;
#else
BYTE llnTmp[10];
#endif
BYTE bBuf[30];
BYTE bTmp[10];

	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));

//#ifdef ECC_BIG_ENDIAN
//	vdChangeEndian(bInData,bTmp,inInSize);
//#else
	memcpy(bTmp,bInData,inInSize);
//#endif

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
	memcpy((BYTE *)&llnTmp,bTmp,inInSize);
	switch(inRadix)
	{
		case 8:
	#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
			size = sprintf((char *)bBuf, "%I64o",llnTmp);
	#endif
	#if READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
			size = sprintf((char *)bBuf, "%llo",llnTmp);
	#endif
			break;
		case 10:
	#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
			size = sprintf((char *)bBuf, "%I64u",llnTmp);
	#endif
	#if READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
			size = sprintf((char *)bBuf, "%llu",llnTmp);
	#endif
			break;
		case 16:
	#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
			size = sprintf((char *)bBuf, "%I64X",llnTmp);
	#endif
	#if READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
			size = sprintf((char *)bBuf, "%llX",llnTmp);
	#endif
			break;
		default:
			break;
	}
#else

	memset(llnTmp,0x00,sizeof(llnTmp));
	memcpy(llnTmp,bTmp,inInSize);

	ECC_UINT64ToASCII(llnTmp,bBuf);

#endif

	vdPad((char *)bBuf,(char *)bOutData,inOutSize,fFlag,chFiller);
}

//For EZHost
void vdUpdateCardInfo(int inTxnType,BYTE *anCardID,BYTE *unPID,BYTE *unCardType,BYTE *unPersonalProfile,BYTE *anEV,BYTE *unNewExpiryDate,BYTE *unOrgExpiryDate,BYTE *unPurseVersionNumber,BYTE *unBankCode,BYTE *unAreaCode,BYTE *unSubAreaCode,BYTE *unDeposit,BYTE *bDeviceID,BYTE *bCPUDeviceID,BYTE *bTxnSN)
{
BYTE bData[30],bData1[30];

	memset(bData,0x00,sizeof(bData));
	memset(bData1,0x00,sizeof(bData1));
	vdTrimData((char *)bData,(char *)anCardID,GET_CARD_LEN);
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0200,bData,FALSE);

	if(srTxnData.srParameter.gDLLVersion != 1)
	{
		memset(bData,0x00,sizeof(bData));
		memcpy(bData,unPID,16);
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0211,bData,FALSE);
	}

	if(strlen((char *)srTxnData.srIngData.anAutoloadFlag) > 0)
	{
		memset(bData,0x00,sizeof(bData));
		memcpy(bData,srTxnData.srIngData.anAutoloadFlag,1);
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0212,bData,FALSE);
	}

	memset(bData,0x00,sizeof(bData));
	memcpy(bData,unCardType,2);
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0213,bData,FALSE);

	memset(bData,0x00,sizeof(bData));
	memcpy(bData,unPersonalProfile,2);
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0214,bData,FALSE);

	memset(bData,0x00,sizeof(bData));
	vdMaskCardID(anCardID,unPID,unPurseVersionNumber,bData);
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0215,bData,FALSE);

	memset(bData,0x00,sizeof(bData));
	vdTrimData((char *)bData,(char *)anCardID,GET_CARD_LEN);
	if(strlen((char *)bData) >= 7)
		memset((char *)&bData[strlen((char *)bData) - 7],'*',3);
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0221,bData,FALSE);//For 頂好

	if(unPurseVersionNumber[1] != '0')
	{
		memset(bData,0x00,sizeof(bData));
		memcpy((char *)bData,(char *)unPID,16);
		memset((char *)&bData[5],'*',6);
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0220,bData,FALSE);
	}

	if(strlen((char *)srTxnData.srIngData.anAutoloadAmount) > 0)
	{
		memset(bData,0x00,sizeof(bData));
		memcpy(bData,srTxnData.srIngData.anAutoloadAmount,strlen((char *)srTxnData.srIngData.anAutoloadAmount));
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0409,bData,FALSE);
	}

	memset(bData,0x00,sizeof(bData));
	vdTrimData((char *)bData,(char *)anEV,(srTxnData.srParameter.gDLLVersion == 1)?5:8);
	strcat((char *)bData,"00");
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0410,bData,FALSE);

	if(inTxnType != TXN_ECC_READ_BASIC)
	{
		memset(bData,0x00,sizeof(bData));
		vdTrimData((char *)bData,(char *)srTxnData.srIngData.bECCEVBefore,8);
		strcat((char *)bData,"00");
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0415,bData,FALSE);
	}

	if(srTxnData.srParameter.chAutoLoadMode == '2' && inTxnType == TXN_ECC_READ_BASIC)
	{
		memset(bData,0x00,sizeof(bData));
		sprintf((char *)bData,"%ld00",srTxnData.srParameter.ulMaxALAmt);
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0416,bData,FALSE);
	}

	memset(bData,0x00,sizeof(bData));
	if(inTxnType != TXN_ECC_READ_BASIC && inTxnType != TXN_ECC_READ_DONGLE && inTxnType != TXN_ECC_READ_DEDUCT)
	{
		if(memcmp(unNewExpiryDate,"00000000",8))
			fnPack((char *)unNewExpiryDate,8,bData1);
		else
			fnPack((char *)unOrgExpiryDate,8,bData1);
		UnixToDateTime(bData1,bData,8);//Expiry Date
	}
	else
		memcpy(bData,unNewExpiryDate,8);

	//if(strlen((char *)bData) == 8)//For 頂好
	//	strcat((char *)bData,"000000");

	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_1402,bData,FALSE);

	if(memcmp(bDeviceID,"000000000",9))
	{
		memset(bData,0x00,sizeof(bData));
		memcpy(bData,bDeviceID,10);
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4109,bData,FALSE);//For 頂好
	}

	if(memcmp(bCPUDeviceID,"0000000000000000",16))
	{
		memset(bData,0x00,sizeof(bData));
		memcpy(bData,bCPUDeviceID,16);
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4110,bData,FALSE);//For 頂好
	}

	vdBuildT4213Tag(&srXMLData);

	memset(bData,0x00,sizeof(bData));
	memcpy(bData,unPurseVersionNumber,2);
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4800,bData,FALSE);

	memset(bData,0x00,sizeof(bData));
	memcpy(bData,unBankCode,2);
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4803,bData,FALSE);

	memset(bData,0x00,sizeof(bData));
	memcpy(bData,unAreaCode,2);
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4804,bData,FALSE);

	if(srTxnData.srParameter.gDLLVersion != 1)
	{
		memset(bData,0x00,sizeof(bData));
		memcpy(bData,unSubAreaCode,4);
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4805,bData,FALSE);
	}

	memset(bData,0x00,sizeof(bData));
	vdTrimData((char *)bData,(char *)bTxnSN,(srTxnData.srParameter.gDLLVersion == 1)?5:8);
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4808,bData,FALSE);//For 頂好

	if(srTxnData.srParameter.gDLLVersion != 1 || inTxnType != TXN_ECC_READ_BASIC)
	{
		memset(bData,0x00,sizeof(bData));
		memset(bData1,0x00,sizeof(bData1));
		fnPack((char *)unDeposit,(srTxnData.srParameter.gDLLVersion == 1)?4:6,bData);
		vdIntToAsc(bData,2,bData1,5,FALSE,' ',10);
		memset(bData,0x00,sizeof(bData));
		vdTrimData((char *)bData,(char *)bData1,17);
		strcat((char *)bData,"00");
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4814,bData,FALSE);
	}
}

//For SIS2
void vdUpdateCardInfo2(int inTxnType,BYTE *ucCardID,BYTE *ucPID,BYTE bAutoLoad,BYTE ucCardType,BYTE ucPersonalProfile,BYTE *ucAutoLoadAmt,BYTE *ucEV,BYTE *ucExpiryDate,BYTE ucPurseVersionNumber,BYTE ucBankCode,BYTE ucAreaCode,BYTE *ucSubAreaCode,BYTE *ucDeposit,BYTE *ucTxnSN)
{
BYTE bData[30],bData1[30],bAutoLoadAmt[5];

	memset(bData,0x00,sizeof(bData));
	memset(bData1,0x00,sizeof(bData1));
	vdUIntToAsc(ucCardID,7,bData1,17,FALSE,' ',10);//n_Card Physical ID
	vdTrimData((char *)bData,(char *)bData1,17);
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0200,bData,FALSE);

	memset(bData,0x00,sizeof(bData));
	fnUnPack(ucPID,8,bData);
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0211,bData,FALSE);

	memset(bData,0x00,sizeof(bData));
	fnUnPack(&bAutoLoad,1,bData);
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0212,&bData[1],FALSE);

	memset(bData,0x00,sizeof(bData));
	fnUnPack(&ucCardType,1,bData);
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0213,bData,FALSE);

	memset(bData,0x00,sizeof(bData));
	fnUnPack(&ucPersonalProfile,1,bData);
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0214,bData,FALSE);

	if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE && srTxnData.srParameter.chBatchFlag == SIS2_BATCH)
	{
		memset(bData,0x00,sizeof(bData));
		sprintf((char *)bData,"%ld00",srTxnData.srIngData.lnECCAmt);
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0400,bData,TRUE);
	}

	if( inTxnType == DLL_READ_CARD_BASIC_DATA ||
		inTxnType == DLL_TAXI_READ ||
		inTxnType == DLL_CBIKE_READ ||
		inTxnType == DLL_EDCA_READ ||
		inTxnType == DLL_E_ADD_VALUE)
	{
		BYTE anAutoLoadAmt[20],anEV[20],anAutoloadAmount[20];
		int/*long*/ lnAutoloadAmt = 0L,lnEV = 0L,lnAutoloadAmount = 0L;

		log_msg(LOG_LEVEL_FLOW,"ucAutoLoadAmt 5:%02x%02x%02x,%d",ucAutoLoadAmt[0],ucAutoLoadAmt[1],ucAutoLoadAmt[2],srTxnData.srIngData.lnECCAmt);
		memset(bAutoLoadAmt,0x00,sizeof(bAutoLoadAmt));
		memset(anAutoLoadAmt,0x00,sizeof(anAutoLoadAmt));
		memset(anEV,0x00,sizeof(anEV));
		memset(anAutoloadAmount,0x00,sizeof(anAutoloadAmount));

		memcpy((char *)&lnAutoloadAmt,ucAutoLoadAmt,3);
		log_msg(LOG_LEVEL_FLOW,"ucAutoLoadAmt 5-1:%d,%ld",lnAutoloadAmt,lnAutoloadAmt);
		sprintf((char *)anAutoLoadAmt,"%d",lnAutoloadAmt);
		log_msg(LOG_LEVEL_FLOW,"ucAutoLoadAmt 5-2:%s",anAutoLoadAmt);

		lnEV = lnAmt3ByteToLong(ucEV);
		//memcpy((char *)&lnEV,ucEV,3);
		sprintf((char *)anEV,"%d",lnEV);
		log_msg(LOG_LEVEL_FLOW,"ucAutoLoadAmt 5-3:%s",anEV);

		if(srTxnData.srParameter.chAutoLoadMode == '0')
			memcpy(bAutoLoadAmt,ucAutoLoadAmt,3);
		else if(srTxnData.srParameter.chAutoLoadMode == '1')
		{
			log_msg(LOG_LEVEL_FLOW,"ucAutoLoadAmt 6:%s,%d",anAutoLoadAmt,srTxnData.srIngData.lnECCAmt);
			lnConvertAutoloadAmt(srTxnData.srIngData.lnECCAmt,anAutoLoadAmt,sizeof(anAutoLoadAmt),anEV,sizeof(anEV),anAutoloadAmount);
			lnAutoloadAmount = atoi((char *)anAutoloadAmount);
			memcpy(bAutoLoadAmt,(char *)&lnAutoloadAmount,4);
		}
		else if(srTxnData.srParameter.chAutoLoadMode == '2')
		{
			//if(inTxnType == DLL_READ_CARD_BASIC_DATA)
			memcpy(bAutoLoadAmt,(char *)&srTxnData.srParameter.ulMaxALAmt,4);
		#ifdef ECC_BIG_ENDIAN
			vdChangeEndian(bAutoLoadAmt,bAutoLoadAmt,4);
		#endif
			/*else
			{
				lnConvertAutoloadAmt(srTxnData.srIngData.lnECCAmt,anAutoLoadAmt,sizeof(anAutoLoadAmt),anEV,sizeof(anEV),anAutoloadAmount);
				lnAutoloadAmount = atoi((char *)anAutoloadAmount);
				memcpy(bAutoLoadAmt,(char *)&lnAutoloadAmount,4);
			}*/
		}

		memset(bData,0x00,sizeof(bData));
		memset(bData1,0x00,sizeof(bData1));
		vdIntToAsc(bAutoLoadAmt,3,bData1,8,FALSE,' ',10);
		vdTrimData((char *)bData,(char *)bData1,17);
		strcat((char *)bData,"00");
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0409,bData,FALSE);
	}
	else if(strlen((char *)srTxnData.srIngData.anAutoloadAmount) > 2)
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0409,srTxnData.srIngData.anAutoloadAmount,FALSE);

	/*if(inTxnType == DLL_E_ADD_VALUE)
	{
		long lnALAmt = 0L;

		memset(bData,0x00,sizeof(bData));

		//T0438改成T0410
		vdXMLChangeTagName(&srXMLData,(char *)TAG_NAME_0438,(char *)TAG_NAME_0410,(char *)bData);

		//T0439改成T0409
		if(inXMLGetAmt(&srXMLData,(char *)TAG_NAME_0439,&lnALAmt,0) >= SUCCESS)
			if(lnALAmt > 0)
				vdXMLChangeTagName(&srXMLData,(char *)TAG_NAME_0439,(char *)TAG_NAME_0409,(char *)bData);

		//T0440改成T0415
		vdXMLChangeTagName(&srXMLData,(char *)TAG_NAME_0440,(char *)TAG_NAME_0415,(char *)bData);
	}
	else*/
	{
		memset(bData,0x00,sizeof(bData));
		memset(bData1,0x00,sizeof(bData1));
		vdIntToAsc(ucEV,3,bData1,8,FALSE,' ',10);
		vdTrimData((char *)bData,(char *)bData1,17);
		strcat((char *)bData,"00");
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0410,bData,FALSE);
	}

	log_msg(LOG_LEVEL_FLOW, "vdUpdateCardInfo2-1:%d", srTxnData.srIngData.inTransType);
	if ((srTxnData.srIngData.inTransType == TXN_ECC_ADD_OFFLINE1) || (srTxnData.srIngData.inTransType == TXN_ECC_ADD4))
	{
		int lnEVBeforeTxn = lnAmt3ByteToLong(ucEV) - srTxnData.srIngData.lnECCAmt;
		memset(bData,0x00,sizeof(bData));
		sprintf((char *)bData,"%d00", lnEVBeforeTxn);
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0415,bData,FALSE);
	}
	else if (srTxnData.srIngData.inTransType == TXN_ECC_AUTOLOAD)
	{
		int lnEVBeforeTxn = lnAmt3ByteToLong(ucEV);
		lnEVBeforeTxn -= (inASCToLONG((char *)srTxnData.srIngData.anAutoloadAmount, sizeof(srTxnData.srIngData.anAutoloadAmount)) / 100);		
		memset(bData,0x00,sizeof(bData));
		sprintf((char *)bData,"%d00", lnEVBeforeTxn);
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0415,bData,FALSE);
	}
	else if ((srTxnData.srIngData.inTransType != TXN_ECC_READ_BASIC) && (srTxnData.srIngData.inTransType != TXN_ECC_READ_TAXI))
	{
		int lnEVBeforeTxn = lnAmt3ByteToLong(ucEV) + srTxnData.srIngData.lnECCAmt;
		if(strlen((char *)srTxnData.srIngData.anAutoloadAmount) > 2)
			lnEVBeforeTxn -= (inASCToLONG((char *)srTxnData.srIngData.anAutoloadAmount, sizeof(srTxnData.srIngData.anAutoloadAmount)) / 100);
		memset(bData,0x00,sizeof(bData));
		//vdTrimData((char *)bData,(char *)srTxnData.srIngData.bECCEVBefore,8);
		//strcat((char *)bData,"00");
		sprintf((char *)bData,"%d00", lnEVBeforeTxn);
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0415,bData,FALSE);
	}

	memset(bData,0x00,sizeof(bData));
	memset(bData1,0x00,sizeof(bData1));
	UnixToDateTime(ucExpiryDate,bData,14);//Expiry Date

	memcpy(bData1, bData, 8);//有效期只需8 Bytes
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_1402,bData1,FALSE);

	vdBuildT4213Tag(&srXMLData);

	memset(bData,0x00,sizeof(bData));
	fnUnPack(&ucPurseVersionNumber,1,bData);
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4800,bData,FALSE);

	memset(bData,0x00,sizeof(bData));
	fnUnPack(&ucBankCode,1,bData);
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4803,bData,FALSE);

	memset(bData,0x00,sizeof(bData));
	fnUnPack(&ucAreaCode,1,bData);
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4804,bData,FALSE);

	memset(bData,0x00,sizeof(bData));
	fnUnPack(ucSubAreaCode,2,bData);
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4805,bData,FALSE);

	memset(bData,0x00,sizeof(bData));
	vdIntToAsc(ucTxnSN,3,bData,8,TRUE,0x00,10);
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4808,bData,FALSE);

	if(inTxnType != DLL_TAXI_READ && inTxnType != DLL_CBIKE_READ && inTxnType != DLL_EDCA_READ)
	{
		memset(bData,0x00,sizeof(bData));
		memset(bData1,0x00,sizeof(bData1));
		vdIntToAsc(ucDeposit,2,bData1,5,FALSE,' ',10);
		vdTrimData((char *)bData,(char *)bData1,17);
		strcat((char *)bData,"00");
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4814,bData,FALSE);
	}
}

void vdMaskCardID(BYTE *anCardID,BYTE *unPID,BYTE *unPurseVersionNumber,BYTE *bOutData)
{
int inMaskAdr = 0;

	if(unPurseVersionNumber[1] == '0')
	{
		vdTrimData((char *)bOutData,(char *)anCardID,GET_CARD_LEN);
		/*inMaskAdr = strlen((char *)bOutData);
		if(inMaskAdr >= 7)
		{
			inMaskAdr -= 7;
			memset((char *)&bOutData[inMaskAdr],'*',3);
		}*/
	}
	else
	{
		inMaskAdr = 5;
		memcpy((char *)bOutData,(char *)unPID,16);
		//memset((char *)&bOutData[inMaskAdr],'*',6);
	}
}

void vdGetECCEVBefore(BYTE *anEVBeforeTxn,int inEVLen)
{
BYTE bBuf[10];

	memset(bBuf,0x00,sizeof(bBuf));
	vdTrimData((char *)srTxnData.srIngData.bECCEVBefore,(char *)anEVBeforeTxn,inEVLen);
}

int inCheckProcess()
{
int inRetVal;
//STRUCT_TMP_DATA srTmpData;
char T1101[6 + 1];

	memset(&gTmpData,0x00,sizeof(gTmpData));
	memset(T1101,0x00,sizeof(T1101));
	inRetVal = inFileGetSize((char *)FILE_TMP,FALSE);
	log_msg(LOG_LEVEL_ERROR,"inCheckProcess = %d,%d",inRetVal,sizeof(gTmpData));
	if(inRetVal == sizeof(gTmpData))
	{
		inFileRead((char *)FILE_TMP,(BYTE *)&gTmpData,sizeof(gTmpData),sizeof(gTmpData));
		srTxnData.srIngData.fTmpFileReaded = TRUE;
		memcpy(T1101,gTmpData.ucCMAS_REQ_T1101,sizeof(gTmpData.ucCMAS_REQ_T1101)-1);
		srTxnData.srREQData.ulT1101 = srTxnData.srIngData.ulCMASHostSerialNumber = atol(T1101);
		log_msg_debug(LOG_LEVEL_FLOW,TRUE,(char *)"FILE_TMP 1 :",12,(BYTE *)&gTmpData,sizeof(gTmpData));
		log_msg(LOG_LEVEL_FLOW,"inCheckProcess ulCMASHostSerialNumber 1:%ld,%ld",srTxnData.srIngData.ulCMASHostSerialNumber,srTxnData.srREQData.ulT1101);
		if(gTmpData.fProcessing == TRUE)
		{
			return API_IS_PROCESSING;
		}
	}
	vdProcessingFlag(TRUE);

	return SUCCESS;
}

void vdProcessingFlag(BYTE bFlag)
{
//STRUCT_TMP_DATA srTmpData;

	vdGetTmpData(&gTmpData);

	gTmpData.fProcessing = bFlag;
	inFileWrite((char *)FILE_TMP,(BYTE *)&gTmpData,sizeof(gTmpData));
	log_msg_debug(LOG_LEVEL_FLOW,TRUE,(char *)"FILE_TMP 2 :",12,(BYTE *)&gTmpData,sizeof(gTmpData));

}

int inECCSettle(int inTxnType)
{
int inRetVal = 0;

	if(srTxnData.srParameter.chBatchFlag == SIS2_BATCH || srTxnData.srParameter.chETxnBatchFlag == SIS2_BATCH)
	{
		inMakeEZHostBatchData();
		inRetVal = inMakeSIS2Blob();
		if(inRetVal < SUCCESS)//有成功的行銷平台帳務,就離開
			return inRetVal;
	}
	else if(srTxnData.srParameter.chBatchFlag == CMAS_BATCH)
	{
		if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
			inRetVal = inCMASSettle(inTxnType);
		else
		{
			log_msg(LOG_LEVEL_ERROR,"inECCSettle Fail 1:%02x",srTxnData.srParameter.chOnlineFlag);
			return ICER_PAR_INI_FAIL;
		}
	}

	return inRetVal;
}

void vdECCFolderProcess()
{
BYTE gLogFileFolder[300];

	if(srTxnData.srParameter.chICERDataFlag == '1')
	{
#if READER_MANUFACTURERS==WINDOWS_API
		strcat((char *)gCurrentFolder,"\\ICERData");
		//mkdir((const char *)gCurrentFolder);
#elif READER_MANUFACTURERS==WINDOWS_CE_API
		strcat((char *)gCurrentFolder,"\\ICERData");
		/*{
			WCHAR		szPort[100];

			memset(szPort,0x00,sizeof(szPort));
			MultiByteToWideChar(CP_UTF8, 0, (char *)gCurrentFolder, 100, szPort, 100);
			CreateDirectory(szPort, NULL);
		}*/
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
		strcat((char *)gCurrentFolder,"/ICERData");
		//mkdir((const char *)gCurrentFolder,0777);
#endif

		ECC_CreateDirectory_Lib((char *)gCurrentFolder);
	}

	if(srTxnData.srParameter.chFolderCreatFlag != '0')//'0' = 不產生
	{
		memset(gLogFileFolder,0x00,sizeof(gLogFileFolder));
		strcpy((char *)gLogFileFolder,(const char *)gCurrentFolder);
#if READER_MANUFACTURERS==WINDOWS_API
		strcat((char *)gLogFileFolder,"\\ECCSIS2"); //dir folder
		//mkdir((const char *)gLogFileFolder);
#elif READER_MANUFACTURERS==WINDOWS_CE_API
		strcat((char *)gLogFileFolder,"\\ECCSIS2"); //dir folder
		//CreateDirectory((WCHAR *)gLogFileFolder, NULL);
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
		strcat((char *)gLogFileFolder,"/ECCSIS2"); //dir folder
		//mkdir((const char *)gLogFileFolder,0777);
#endif
		ECC_CreateDirectory_Lib((char *)gLogFileFolder);
		memset(gLogFileFolder,0x00,sizeof(gLogFileFolder));

		strcpy((char *)gLogFileFolder,(const char *)gCurrentFolder);
#if READER_MANUFACTURERS==WINDOWS_API
		strcat((char *)gLogFileFolder,"\\ECCSIS2_BAK"); //dir folder
		//mkdir((const char *)gLogFileFolder);
#elif READER_MANUFACTURERS==WINDOWS_CE_API
		strcat((char *)gLogFileFolder,"\\ECCSIS2_BAK"); //dir folder
		//CreateDirectory((WCHAR *)gLogFileFolder, NULL);
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
		strcat((char *)gLogFileFolder,"/ECCSIS2_BAK"); //dir folder
		//mkdir((const char *)gLogFileFolder,0777);
#endif
		ECC_CreateDirectory_Lib((char *)gLogFileFolder);
	}
	memset(gLogFileFolder,0x00,sizeof(gLogFileFolder));
}

long lnAmt3ByteToLong(BYTE *b3ByteAmt)
{
	long lret = 0;
	if (sizeof(long) == 4)
	{
UnionUlong ulAmt;

	ulAmt.Value = 0L;
	memcpy(ulAmt.Buf,b3ByteAmt,3);
	if(b3ByteAmt[2] >= 0x80)
		ulAmt.Buf[3] = 0xFF;

#ifdef ECC_BIG_ENDIAN
	vdChangeEndian(ulAmt.Buf,ulAmt.Buf,sizeof(ulAmt.Buf));
#endif

		lret = ulAmt.Value;
	}
	else
	{
		Union8Long ulAmt;
		ulAmt.Value = 0L;
		memcpy(ulAmt.Buf, b3ByteAmt, 3);
		if (b3ByteAmt[2] >= 0x80)
			memset(&ulAmt.Buf[3], 0xFF, 5);
#ifdef ECC_BIG_ENDIAN
	vdChangeEndian(ulAmt.Buf,ulAmt.Buf,sizeof(ulAmt.Buf));
#endif
		lret = ulAmt.Value;
		log_msg(LOG_LEVEL_FLOW,"long as 8 bytes : %ld", lret);
	}
	return lret;
}

//For SetValue
void vdUpdateCardInfo3(int inTxnType,BYTE *anCardID,BYTE *anEVBeforeTxn,BYTE *unNewExpiryDate,BYTE *unOrgExpiryDate)
{
BYTE bData[30],bData1[30],unPurseVersionNumber[2],unPID[20];

	memset(bData,0x00,sizeof(bData));
	memset(bData1,0x00,sizeof(bData1));
	memset(unPurseVersionNumber,0x00,sizeof(unPurseVersionNumber));
	memset(unPID,0x00,sizeof(unPID));
	vdTrimData((char *)bData,(char *)anCardID,GET_CARD_LEN);
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0200,bData,FALSE);

	memset(bData,0x00,sizeof(bData));
	vdTrimData((char *)bData,(char *)anEVBeforeTxn,(srTxnData.srParameter.gDLLVersion == 1)?5:8);
	strcat((char *)bData,"00");
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0410,bData,FALSE);

	if(inTxnType != TXN_ECC_READ_BASIC)
	{
		memset(bData,0x00,sizeof(bData));
		vdTrimData((char *)bData,(char *)srTxnData.srIngData.bECCEVBefore,8);
		strcat((char *)bData,"00");
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0415,bData,FALSE);
	}

	memset(bData,0x00,sizeof(bData));
	/*if(inTxnType != TXN_ECC_READ_BASIC)
	{
		if(memcmp(unNewExpiryDate,"00000000",8))
			fnPack((char *)unNewExpiryDate,8,bData1);
		else
			fnPack((char *)unOrgExpiryDate,8,bData1);
		UnixToDateTime(bData1,bData,14);//Expiry Date
	}
	else*/
		//memcpy(bData,unNewExpiryDate,14);//有效期應只要8byte，修正
		memcpy(bData,unNewExpiryDate,8);
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_1402,bData,FALSE);
}

long lnConvertAutoloadAmt(long lnECCAmt,BYTE *bInALAmt,int inInLen,BYTE *bEVAmt,int inEVLen,BYTE *bOutALAmt)
{
BYTE bALAmt[20],bEV[20];
long ulMaxALAmt = 0L,ulCardALAmt = 0L,ulEVAmt = 0L;

	memset(bALAmt,0x00,sizeof(bALAmt));
	vdTrimData((char *)bALAmt,(char *)bInALAmt,inInLen);
	ulMaxALAmt = ulCardALAmt = atol((char *)bALAmt);
	memset(bEV,0x00,sizeof(bEV));
	vdTrimData((char *)bEV,(char *)bEVAmt,inEVLen);
	ulEVAmt = atol((char *)bEV);

	log_msg(LOG_LEVEL_FLOW,"lnConvertAutoloadAmt 1 %ld,%ld,%ld,%ld,%ld",ulCardALAmt,ulMaxALAmt,srTxnData.srParameter.ulMaxALAmt,lnECCAmt,ulEVAmt);

	//if(ulCardALAmt <= 0L || ulMaxALAmt * 2 > (long)srTxnData.srParameter.ulMaxALAmt)
	if(ulCardALAmt <= 0L)
	{
		sprintf((char *)bOutALAmt,"%ld",ulMaxALAmt);
		return ulMaxALAmt;
	}

	while(lnECCAmt > (long)(ulEVAmt + ulMaxALAmt))
	{
		log_msg(LOG_LEVEL_FLOW,"lnConvertAutoloadAmt 2 %ld,%ld,%ld",ulCardALAmt,ulMaxALAmt,srTxnData.srParameter.ulMaxALAmt);
		//if(ulMaxALAmt + ulCardALAmt <= (long)srTxnData.srParameter.ulMaxALAmt)
		if(ulMaxALAmt + ulCardALAmt <= (long)0xFFFFFF)
			ulMaxALAmt += ulCardALAmt;
		else
			break;
	};

	sprintf((char *)bOutALAmt,"%ld",ulMaxALAmt);

	log_msg(LOG_LEVEL_FLOW,"lnConvertAutoloadAmt 3 %ld,%s(%d),%s(%d),%s,%ld",lnECCAmt,bInALAmt,inInLen,bEVAmt,inEVLen,bOutALAmt,ulMaxALAmt);

	return ulMaxALAmt;
}

void vdToUpper(char *szOutData,char *szInData)
{
int i;

	for(i=0;i<(int )strlen(szInData);i++)
	{
		szOutData[i] = toupper(szInData[i]);
	}
}

int inGetBLCName(BYTE *bBLCName)
{
//STRUCT_TMP_DATA srTmpData;

	vdGetTmpData(&gTmpData);
	if(strlen((char *)gTmpData.ucBLCName) == 0)
		memcpy((char *)bBLCName,"00000",5);
	else
		memcpy((char *)bBLCName,(char *)gTmpData.ucBLCName,sizeof(gTmpData.ucBLCName));

	return SUCCESS;
}

int inSetBLCName(BYTE *bBLCName)
{
//STRUCT_TMP_DATA srTmpData;

	vdGetTmpData(&gTmpData);
	memcpy((char *)gTmpData.ucBLCName,(char *)bBLCName,sizeof(gTmpData.ucBLCName));
	inFileWrite((char *)FILE_TMP,(BYTE *)&gTmpData,sizeof(gTmpData));
	return SUCCESS;
}

void vdGetTmpData(STRUCT_TMP_DATA *srTmpData)
{
/*int inRetVal;

	memset(srTmpData,0x00,sizeof(STRUCT_TMP_DATA));
	inRetVal = inFileGetSize((char *)FILE_TMP,FALSE);
	if(inRetVal == sizeof(STRUCT_TMP_DATA))
		inFileRead((char *)FILE_TMP,(BYTE *)srTmpData,sizeof(STRUCT_TMP_DATA),sizeof(STRUCT_TMP_DATA));
	else
		inFileDelete((char *)FILE_TMP,FALSE);*/
}

/*void vdTest(int inCnt,BYTE *srTmpData)
{
int i;

	printf("vdTest %d",inCnt);
	for(i=0;i<(int)sizeof(STRUCT_TMP_DATA);i++)
	{
		if(srTmpData[i] < 0x20)
			printf("[%02x]",srTmpData[i]);
		else
			printf("%c",srTmpData[i]);
	}
	printf("\n");
}*/

BYTE bGetLCDControlFlag()
{
#if 1

	if(srTxnData.srREQData.ucT4108 == '1')
		return 0x01;
	else
		return 0x00;

#else
BYTE bLCDControlFlag[2];

	int inRetVal;
	inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_4108,(char *)bLCDControlFlag,1,0);
	if(inRetVal >= SUCCESS)//有4830
	{
		if(bLCDControlFlag[0] == '1')
			bLCDControlFlag[0] = 0x01;
		else
			bLCDControlFlag[0] = 0x00;
	}
	else
		bLCDControlFlag[0] = 0x00;

	return bLCDControlFlag[0];
#endif
}

void log_msg2(const char* fmt, ...)
{
#if READER_MANUFACTURERS==WINDOWS_API
va_list args;
FILE *fp=NULL;
char cLogFile[20],CurTime[20],sdir[200],mlog[MAX_XML_FILE_SIZE];
int i= 0x00,nRtn= 0x00;
struct tm * today=NULL;
time_t now;

	if(srTxnData.srParameter.gLogFlag != 1)
		return;

	memset(cLogFile,0,sizeof(cLogFile));
	memset(CurTime,0,sizeof(CurTime));
	memset(sdir,0,sizeof(sdir));
	memset(mlog,0,sizeof(mlog));
	strcpy(sdir,(const char *)gCurrentFolder);
	if(srTxnData.srParameter.chFolderCreatFlag == '1' || srTxnData.srParameter.chFolderCreatFlag == 0x00)
		strcat(sdir,"\\ICERLog"); //dir folder
	time(&now);
	today=localtime(&now);
	if(srTxnData.srParameter.chFolderCreatFlag == '1' || srTxnData.srParameter.chFolderCreatFlag == 0x00)
		strftime(cLogFile,20,"/RDebug%y%m%d.log",today);
	else
		strftime(cLogFile,20,"RDebug%y%m%d.log",today);
	strcat(sdir,cLogFile);
	strftime(CurTime,20,"%Y/%m/%d %H:%M:%S",today);

	fp = fopen(sdir,"a+");
	if(fp==NULL)
	{
		return;
	}
	va_start(args,fmt); //將取出的指標(va_list) 指到第一個不定參數
	vsprintf(mlog,fmt,args);//把參數args按照fmt指定的格式，寫到mlog中
	va_end(args);
	fprintf(fp,"(%s):",CurTime);
	fprintf(fp,"%s\n",mlog);
	fclose(fp);
#elif READER_MANUFACTURERS==WINDOWS_CE_API
va_list args;
FILE *fp=NULL;
char cLogFile[20],CurTime[20],sdir[200],mlog[MAX_XML_FILE_SIZE];
int i= 0x00,nRtn= 0x00;
struct tm * today=NULL;
//time_t now;
SYSTEMTIME now;

	if(srTxnData.srParameter.gLogFlag != 1)
		return;

	memset(cLogFile,0,sizeof(cLogFile));
	memset(CurTime,0,sizeof(CurTime));
	memset(sdir,0,sizeof(sdir));
 	memset(mlog,0,sizeof(mlog));
	if(srTxnData.srParameter.chFolderCreatFlag == '1' || srTxnData.srParameter.chFolderCreatFlag == 0x00)
	{
	 	strcpy(sdir,(const char *)gCurrentFolder);
		strcat(sdir,"\\ICERLog"); //dir folder
	}

//  	time(&now);
	GetLocalTime(&now);
	if(srTxnData.srParameter.chFolderCreatFlag == '1' || srTxnData.srParameter.chFolderCreatFlag == 0x00)
		//strftime(cLogFile,20,"/ICER%Y%m%d.log",today);
		sprintf(cLogFile, "/RDebug%02d%02d%02d.log", now.wYear % 100, now.wMonth, now.wDay);
	else
		//strftime(cLogFile,20,"ICER%Y%m%d.log",today);
		sprintf(cLogFile, "RDebug%02d%02d%02d.log", now.wYear % 100, now.wMonth, now.wDay);

	strcat(sdir,cLogFile);
	//strftime(CurTime,20,"%Y/%m/%d %H:%M:%S",today);
	sprintf(CurTime, "%04d%02d%02d %02d:%02d:%02d", now.wYear, now.wMonth, now.wDay, now.wHour,now.wMinute,now.wSecond);

	fp = fopen(sdir,"a+");
	if(fp==NULL)
	{
		return;
	}
	va_start(args,fmt); //將取出的指標(va_list) 指到第一個不定參數
	vsprintf(mlog,fmt,args);//把參數args按照fmt指定的格式，寫到mlog中
	va_end(args);
	fprintf(fp,"(%s)",CurTime);
	fprintf(fp,"%s\n",mlog);
	fclose(fp);
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
time_t utTime;
struct tm * gTime;
char CurTime[20];
char msg[MAX_XML_FILE_SIZE], mlog[MAX_XML_FILE_SIZE];
va_list args;
struct timeval tv;


	assert(fmt != 0);

	gettimeofday(&tv,NULL);
	va_start(args, fmt); //撠??箇???(va_list) ?蝚砌???摰???
	vsprintf(mlog, fmt, args);//???筠rgs?fmt???撘?撖怠mlog銝?
	va_end(args);
	time(&utTime);
	gTime=localtime(&utTime);
	memset(CurTime,0,sizeof(CurTime));
	strftime(CurTime,20,"%Y/%m/%d %H:%M:%S",gTime);
	sprintf(msg, "(%s)(%ld:%ld):%s\n", CurTime,tv.tv_sec,tv.tv_usec,mlog);//Ctime撠???ASCII摮葡

#if READER_MANUFACTURERS==ANDROID_API
	write(fd2_, msg, strlen(msg));
#else
	if(write(fd2_, msg, strlen(msg)) < 0) {
		syslog(LOG_MAKEPRI(LOG_USER, LOG_INFO), "%s", mlog);
	}
#endif
#endif
}

void vdSaveReaderDebugLog(BYTE *bReaderDebugLog,int inLen)
{
BYTE bBuf[300];

	memset(bBuf,0x00,sizeof(bBuf));
	memcpy(bBuf,bReaderDebugLog,inLen);
	log_msg2((char *)bBuf);
}

int inCMASSettle(int inTxnType)
{
int inRetVal = CARD_SUCCESS;
BYTE bBuf[100];
BYTE bOutData[READER_BUFFER];

	memset(bBuf,0x00,sizeof(bBuf));
	memset(bOutData,0x00,sizeof(bOutData));
	if(!memcmp(gTmpData.ucCPUDeviceID,"\x00\x00\x00\x00\x00\x00",6) || !memcmp(gTmpData.ucDeviceID,"\x00\x00\x00\x00",4))
		inRetVal = inSignOnProcess(TXN_ECC_SIGN_ON,bOutData,TRUE);
	if(inRetVal != CARD_SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inCMASSettle Fail 1:%d",inRetVal);
		return inRetVal;
	}
	
	inBuildCMASSettleTM(inTxnType,&srXMLData,bBuf,bBuf,bBuf,bBuf);
	inRetVal = inSendRecvXML(HOST_CMAS,&srXMLData,inTxnType);
	if(inRetVal < SUCCESS)
		log_msg(LOG_LEVEL_ERROR,"inCMASSettle Fail 1:%d",inRetVal);

	return inRetVal;
}

void vd640EProcess(int inReaderSW,BYTE *bReaderOutData,BYTE *bReaderOutData2)
{
BYTE bBuf[10];
BYTE bData[30],bData1[30];
LockCard_APDU_Out_3 tReadBasicOut3;
LockCard_APDU_Out_2 tReadBasicOut2;

	memset(bBuf,0x00,sizeof(bBuf));

	if(srTxnData.srParameter.chBatchFlag == CMAS_BATCH)
	{
		char chTMProcessCode[6 + 1];
		BYTE bBuffer[MAX_XML_FILE_SIZE];

		memset(chTMProcessCode,0x00,sizeof(chTMProcessCode));
		memcpy(chTMProcessCode,srTxnData.srIngData.chTMProcessCode,sizeof(chTMProcessCode));
		memcpy(srTxnData.srIngData.chTMProcessCode,"596100",sizeof(chTMProcessCode));

		if(inBuildSendPackage(HOST_CMAS,&srXMLData,bBuffer,REVERSAL_OFF,ADVICE_ON,STORE_BATCH_OFF) < SUCCESS)
			log_msg(LOG_LEVEL_ERROR,"vd640EProcess Fail 1");
		else
			inTCPIPAdviceProcess(HOST_CMAS,FALSE);

		memcpy(srTxnData.srIngData.chTMProcessCode,chTMProcessCode,sizeof(chTMProcessCode));

		//return SUCCESS;
	}

	if(inReaderSW == 0x640E || inReaderSW == 0x610F || inReaderSW == 0x6418)
	{
		if(srTxnData.srParameter.gReaderMode == DLL_READER || srTxnData.srParameter.chBatchFlag == R6_BATCH || srTxnData.srParameter.chBatchFlag == SIS2_BATCH)
			memcpy(&tReadBasicOut3,&bReaderOutData2[OUT_DATA_OFFSET],sizeof(LockCard_APDU_Out_3));
		else
			memcpy(&tReadBasicOut3,bReaderOutData,sizeof(LockCard_APDU_Out_3));
		//LockCard_APDU_Out_3 *tReadBasicOut = (LockCard_APDU_Out_3 *)bReaderOutData;

		vdUpdateCardInfo2(DLL_LOCK_CARD,
		//vdUpdateCardInfo2(DLL_READ_CARD_BASIC_DATA,
					 tReadBasicOut3.ucCardID,
					 tReadBasicOut3.ucPID,
					 (BYTE )tReadBasicOut3.bAutoLoad,
					 tReadBasicOut3.ucCardType,
					 tReadBasicOut3.ucPersonalProfile,
					 tReadBasicOut3.ucAutoLoadAmt,
					 tReadBasicOut3.ucEV,
					 tReadBasicOut3.ucExpiryDate,
					 tReadBasicOut3.ucPurseVersionNumber,
					 tReadBasicOut3.ucBankCode,
					 tReadBasicOut3.ucAreaCode,
					 tReadBasicOut3.ucSubAreaCode,
					 tReadBasicOut3.ucDeposit,
					 tReadBasicOut3.ucTxnSN);
	}
	else if(inReaderSW == 0x6406 || inReaderSW == 0x6103 || inReaderSW == 0x9000)
	{
		if(srTxnData.srParameter.gReaderMode == DLL_READER || srTxnData.srParameter.chBatchFlag == R6_BATCH || srTxnData.srParameter.chBatchFlag == SIS2_BATCH)
			memcpy(&tReadBasicOut2,&bReaderOutData2[OUT_DATA_OFFSET],sizeof(LockCard_APDU_Out_3));
		else
			memcpy(&tReadBasicOut2,bReaderOutData,sizeof(LockCard_APDU_Out_2));
		//LockCard_APDU_Out_2 *tReadBasicOut = (LockCard_APDU_Out_2 *)bReaderOutData;

		memset(bData,0x00,sizeof(bData));
		memset(bData1,0x00,sizeof(bData1));
		vdUIntToAsc(tReadBasicOut2.ucCardID,7,bData1,17,FALSE,' ',10);//n_Card Physical ID
		vdTrimData((char *)bData,(char *)bData1,17);
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0200,bData,FALSE);

		memset(bData,0x00,sizeof(bData));
		fnUnPack(tReadBasicOut2.ucPID,8,bData);
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0211,bData,FALSE);

		memset(bData,0x00,sizeof(bData));
		fnUnPack(&tReadBasicOut2.ucCardType,1,bData);
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0213,bData,FALSE);

		memset(bData,0x00,sizeof(bData));
		fnUnPack(&tReadBasicOut2.ucPersonalProfile,1,bData);
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0214,bData,FALSE);

		if(strlen((char *)srTxnData.srIngData.anAutoloadAmount) > 2)
			inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0409,srTxnData.srIngData.anAutoloadAmount,FALSE);

		memset(bData,0x00,sizeof(bData));
		fnUnPack(&tReadBasicOut2.ucPurseVersionNumber,1,bData);
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4800,bData,FALSE);
	}

	if(srTxnData.srParameter.chBatchFlag == CMAS_BATCH)
		return;

	if(srTxnData.srParameter.gReaderMode == DLL_READER || srTxnData.srParameter.chBatchFlag == R6_BATCH)
		memcpy(srTxnData.srIngData.anTLKR_TLRC,bReaderOutData,TLKR_TLRC_LEN);
	else
		fnUnPack(bReaderOutData,SIS2_LOCK_LEN,srTxnData.srIngData.anTLKR_TLRC);
	if(inXMLGetData(&srXMLData,(char *)TAG_NAME_5548,(char *)bBuf,0,0) < SUCCESS)//尚未有5548
	{
		vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,TRUE,NODE_NO_SAME,TAG_NAME_START,FALSE);
		vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554804,(char *)srTxnData.srIngData.anTLKR_TLRC,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
		vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,FALSE);
		inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/TRANS",FALSE);
	}
	else//已有5548
	{
		vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554804,(char *)srTxnData.srIngData.anTLKR_TLRC,TRUE,NODE_NO_SAME,VALUE_NAME,FALSE);
		inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/T5548",FALSE);
	}
}

void vdBuildDeviceIDResponse(STRUCT_XML_DOC *srXML)
{
int inRetVal,inCntStart,inCntEnd;
BYTE ucDeviceID[4],ucCPUDeviceID[6],anDeviceID[10 + 1],anCPUDeviceID[16 + 1];

	memset(ucDeviceID,0x00,sizeof(ucDeviceID));
	memset(ucCPUDeviceID,0x00,sizeof(ucCPUDeviceID));
	memset(anDeviceID,0x00,sizeof(anDeviceID));
	memset(anCPUDeviceID,0x00,sizeof(anCPUDeviceID));

	inRetVal = inXMLSearchValueByTag(srXML,(char *)TAG_NAME_4100,&inCntStart,&inCntEnd,0);
	if(inRetVal >= SUCCESS)//有 TAG_NAME_4100
	{
		if(srTxnData.srParameter.chOnlineFlag == ICER_ONLINE)
		{
			memcpy(anCPUDeviceID,srXML->srXMLElement[inCntStart].chValue,srXML->srXMLElement[inCntStart].shValueLen);
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4109,anCPUDeviceID,TRUE);
		}
		else
		{
			fnPack(srXML->srXMLElement[inCntStart].chValue,srXML->srXMLElement[inCntStart].shValueLen,ucCPUDeviceID);
			fnBINTODEVASC(ucCPUDeviceID,anCPUDeviceID,sizeof(anCPUDeviceID) - 1,LEVEL2);//n_TXN Device ID
			inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4110,anCPUDeviceID,TRUE);
		}
		log_msg(LOG_LEVEL_FLOW,"vdBuildDeviceIDResponse T4100,%s,%s",srXML->srXMLElement[inCntStart].chValue,anCPUDeviceID);
	}
	else
		log_msg(LOG_LEVEL_FLOW,"vdBuildDeviceIDResponse Fail T4100");

	inRetVal = inXMLSearchValueByTag(srXML,(char *)TAG_NAME_4101,&inCntStart,&inCntEnd,0);
	if(inRetVal >= SUCCESS)//有 TAG_NAME_4101
	{
		fnPack(srXML->srXMLElement[inCntStart].chValue,srXML->srXMLElement[inCntStart].shValueLen,ucDeviceID);
		fnBINTODEVASC(ucDeviceID,anDeviceID,sizeof(anDeviceID) - 1,MIFARE);//n_TXN Device ID
		inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_4109,anDeviceID,TRUE);
		log_msg(LOG_LEVEL_FLOW,"vdBuildDeviceIDResponse T4101,%s,%s",srXML->srXMLElement[inCntStart].chValue,anDeviceID);
	}
	else
		log_msg(LOG_LEVEL_FLOW,"vdBuildDeviceIDResponse Fail T4101");

}

void vdCMASBuildRRNProcess(STRUCT_XML_DOC *srXML,unsigned long ulSerialNumber,BYTE *ucTxnSN)
{
BYTE bBuf[20];
int inRetVal;
unsigned long ulTxnSN = 0L;

	memcpy((char *)&ulTxnSN,ucTxnSN,3);
	memset(bBuf,0x00,sizeof(bBuf));
	inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_3700,(char *)bBuf,sizeof(bBuf),0);
	if(inRetVal >=SUCCESS)
	{
		memset(srTxnData.srIngData.anRRN,0x00,sizeof(srTxnData.srIngData.anRRN));
		memcpy(srTxnData.srIngData.anRRN,bBuf,strlen((char *)bBuf));
	}
	else if(strlen((char *)srTxnData.srIngData.anRRN) == 12 || strlen((char *)srTxnData.srIngData.anRRN) == 14)
		memcpy(bBuf,srTxnData.srIngData.anRRN,strlen((char *)srTxnData.srIngData.anRRN));
	else
	{
		log_msg(LOG_LEVEL_ERROR,"vdCMASSerialNumberProcess Fail = %s",srTxnData.srIngData.anRRN);
		return;
	}

	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%010ld",ulTxnSN);
	sprintf((char *)&srTxnData.srIngData.anRRN[6],"%06ld",ulSerialNumber);
	memcpy((char *)&srTxnData.srIngData.anRRN[12],&bBuf[8],2);
	inXMLUpdateData(srXML,(char *)TAG_NAME_4200,(char *)TAG_NAME_3700,srTxnData.srIngData.anRRN,TRUE);
}

void vdReSendR6ReaderAVR(BYTE *bReaderOutData)
{
AddValueL2_TM_Out_6415 *TMOut = (AddValueL2_TM_Out_6415 *)bReaderOutData;
AuthTxnOnline_TM_Out APIOut;
BYTE bRecvData[50];

	memset(&APIOut,0x00,sizeof(APIOut));
	memset(bRecvData,0x00,sizeof(bRecvData));

	//因EZHost說02 35的交易不用送!!
	if(TMOut->stReaderAVRInfo_t.ucMsgType == 0x02 && TMOut->stReaderAVRInfo_t.ucSubType == 0x35)
		return;

	//Congirm資料開始
	vdBuildMessageType(DLL_ADD_VALUE_AUTH,APIOut.ucMessageType);//Message Type ID
	sprintf((char *)APIOut.ucDataLen,"%03d",79);//Data Len
	vdBuildProcessingCode(DLL_ADD_VALUE_AUTH,TMOut->stReaderAVRInfo_t.ucMsgType,TMOut->stReaderAVRInfo_t.ucSubType,TMOut->ucPersonalProfile,APIOut.ucProcessignCode);//Processing Code
	APIOut.ucMsgType = TMOut->stReaderAVRInfo_t.ucMsgType;//Msg Type
	APIOut.ucSubType = TMOut->stReaderAVRInfo_t.ucSubType;//Sub Type
	memcpy(APIOut.ucDeviceID,TMOut->stReaderAVRInfo_t.ucDeviceID,sizeof(APIOut.ucDeviceID));//Device ID
	memcpy(APIOut.ucTxnDateTime,TMOut->stReaderAVRInfo_t.ucTxnDateTime,sizeof(APIOut.ucTxnDateTime));//Txn Date Time
	if(TMOut->stReaderAVRInfo_t.ucPurseVersionNumber == MIFARE)
		memcpy(APIOut.ucMAC,TMOut->stReaderAVRInfo_t.ucMAC_HCrypto,sizeof(APIOut.ucMAC));//MAC
	else
		memset(APIOut.ucMAC,0x00,sizeof(APIOut.ucMAC));//MAC
	memcpy(APIOut.ucConfirmCode,TMOut->stReaderAVRInfo_t.ucStatusCode,sizeof(APIOut.ucConfirmCode));//Confirm Code
	if(TMOut->stReaderAVRInfo_t.ucPurseVersionNumber == MIFARE)
		memcpy(APIOut.ucCTAC,&TMOut->stReaderAVRInfo_t.ucMAC_HCrypto[10],sizeof(APIOut.ucMAC));//CTAC
	else
		memset(APIOut.ucCTAC,0x00,sizeof(APIOut.ucMAC));//CTAC
	memcpy(APIOut.ucCardID,TMOut->stReaderAVRInfo_t.ucCardID,sizeof(APIOut.ucCardID));//Card ID
	memcpy(APIOut.ucEV,TMOut->stReaderAVRInfo_t.ucEV,sizeof(APIOut.ucEV));//EV
	memcpy(APIOut.ucTxnSN,TMOut->stReaderAVRInfo_t.ucTxnSN,sizeof(APIOut.ucTxnSN));//Txn SN
	memcpy(APIOut.ucCPUDeviceID,TMOut->stReaderAVRInfo_t.ucCPUDeviceID,sizeof(APIOut.ucCPUDeviceID));//CPU Device ID
	memcpy(APIOut.ucPID,TMOut->stReaderAVRInfo_t.ucPID,sizeof(APIOut.ucPID));//Purse ID
	memcpy(APIOut.ucSignature,TMOut->stReaderAVRInfo_t.ucSignature,sizeof(APIOut.ucSignature));//Signature
	if(TMOut->stReaderAVRInfo_t.ucPurseVersionNumber != MIFARE)
	{
		APIOut.ucHashType = TMOut->stReaderAVRInfo_t.ucMAC_HCrypto[0];//Hash Type
		APIOut.ucHostAdminKVN = TMOut->stReaderAVRInfo_t.ucMAC_HCrypto[1];//Host Admin KVN
		memcpy(APIOut.ucCPUMAC,&TMOut->stReaderAVRInfo_t.ucMAC_HCrypto[2],sizeof(APIOut.ucCPUMAC));//CPU MAC
	}
	else
	{
		APIOut.ucHashType = 0x00;
		APIOut.ucHostAdminKVN = 0x00;
		memset(APIOut.ucCPUMAC,0x00,sizeof(APIOut.ucCPUMAC));//CPU MAC
	}

	inTCPSendAndReceive(DLL_ADD_VALUE_AUTH,APIOut.ucMessageType,bRecvData,FALSE);
}

/*#if READER_MANUFACTURERS!=WINDOWS_API && READER_MANUFACTURERS!=LINUX_API && READER_MANUFACTURERS!=ANDROID_API
char toupper(char chInData)
{

	if(chInData >= 'a' && chInData <= 'z')
		return chInData - 0x20;
	else
		return chInData;
}
#endif*/

int inECCBatchUpload(void)
{
int inRetVal = 0;

	inRetVal = inCMASBatchUpload();

	return inRetVal;
}

int inCMASBatchUpload()
{
int inRetVal;
BYTE *bUploadData;
long ulFileSize = 0L,inOffset = 0;

	ulFileSize = inFileGetSize((char *)FILE_REQ_CMAS_BAT,FALSE);
	if(ulFileSize <= SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inCMASBatchUpload failed 1:%ld",ulFileSize);
		return ICER_ERROR;
	}

	if(srTxnData.srParameter.inAdditionalTcpipData == NO_ADD_DATA)
		inOffset = 3;

	bUploadData = (BYTE *)ECC_calloc(1,ulFileSize + inOffset);

	inRetVal = inFileRead((char *)FILE_REQ_CMAS_BAT,&bUploadData[inOffset],ulFileSize,ulFileSize);
	if(inRetVal < SUCCESS)
	{
		ECC_free(bUploadData);
		log_msg(LOG_LEVEL_ERROR,"inCMASBatchUpload failed 2:%d",inRetVal);
		return ICER_ERROR;
	}

	inRetVal = inCMASBatchUploadProcess(bUploadData,ulFileSize + inOffset);
	ECC_free(bUploadData);

	return inRetVal;
}

void vdSetCMASTagDefault(STRUCT_XML_DOC *srXML,int inTransType)
{
	vdSetConfigFlag(srXML,TRUE,(char *)TAG_NAME_4830);
	vdSetConfigFlag(srXML,2,(char *)TAG_NAME_1102);
	vdSetConfigFlag(srXML,2,(char *)TAG_NAME_1103);
	vdSetConfigFlag(srXML,2,(char *)TAG_NAME_4105);
	vdSetConfigFlag(srXML,2,(char *)TAG_NAME_4836);
	vdSetConfigFlag(srXML,2,(char *)TAG_NAME_4837);
	vdSetConfigFlag(srXML,2,(char *)TAG_NAME_4843);
	vdSetConfigFlag(srXML,2,(char *)TAG_NAME_554810);
	if(inTransType == TXN_ECC_DEDUCT_TAXI)
	{
		vdSetConfigFlag(srXML,2,(char *)TAG_NAME_4835);
		//vdSetConfigFlag(srXML,2,(char *)TAG_NAME_4836);
		//vdSetConfigFlag(srXML,2,(char *)TAG_NAME_4837);
	}
	else if(inTransType == TXN_ECC_MULTI_SELECT)
		vdSetConfigFlag(srXML,3,(char *)TAG_NAME_4831);

	if(inTransType == TXN_ECC_DEDUCT_TAXI || inTransType == TXN_ECC_DEDUCT_CBIKE || inTransType == TXN_ECC_DEDUCT_EDCA)
		vdSetConfigFlag(srXML,3,(char *)TAG_NAME_4846);

	//if(srTxnData.srParameter.chCMASMode == '2')
	//	vdSetConfigFlag(srXML,2,(char *)TAG_NAME_4214);

	if(inTransType == TXN_ECC_DEDUCT)
	{
		vdSetConfigFlag(srXML,2,(char *)TAG_NAME_4847);
		vdSetConfigFlag(srXML,2,(char *)TAG_NAME_4848);
	}
}

BOOL fIsDeductTxn(int inTransType)
{

	if(inTransType == TXN_ECC_DEDUCT || inTransType == TXN_ECC_DEDUCT_TAXI || inTransType == TXN_ECC_DEDUCT_EDCA || inTransType == TXN_ECC_DEDUCT_CBIKE)
		return TRUE;

	return FALSE;
}

int inECCAdvice(void)
{
int inRetVal = 0;

	inRetVal = inTCPIPAdviceProcess(HOST_CMAS,FALSE);

	return inRetVal;
}

BOOL fIsETxn(int inTransType)
{

	if((inTransType >= TXN_E_READ_BASIC && inTransType < TXN_SETTLE) || inTransType == TXN_QUERY_POINT)
		return TRUE;

	return FALSE;
}

int inETxnProcess(int inTxnType)
{
int inRetVal;

	//if(inTxnType >= TXN_E_READ_QR_CODE && inTxnType < TXN_E_QR_VOID_ADD)
	//	inRetVal = inDoICERQRTxn(inTxnType);
	//else //if((inTxnType >= TXN_E_ADD && inTxnType < TXN_SETTLE) || inTxnType == TXN_QUERY_POINT)
		inRetVal = inDoICERTxn(inTxnType);

	if(inRetVal == SUCCESS)
	{
	}

	return inRetVal;
}

void log_msg_debug(int inLogLevel,char fDebugFlag,char *chMsg,int inMsgLen,BYTE *chData,int inDataLen)
{
BYTE *bBuf;
int i;

	bBuf = (BYTE *)ECC_calloc(1,inDataLen * 4 + inMsgLen + 10);
	memset(bBuf,0x00,inDataLen * 4 + inMsgLen + 10);
	memcpy(bBuf,chMsg,inMsgLen);
	sprintf((char *)&bBuf[strlen((char *)bBuf)]," (%d):",inDataLen);
	for(i=0;i<inDataLen;i++)
	{
		if(fDebugFlag ==FALSE)
		{
			if(chData[i] >= 0x20 && chData[i] <= 0x7F)
				bBuf[strlen((char *)bBuf)] = chData[i];
			else
				sprintf((char *)&bBuf[strlen((char *)bBuf)],"[%02X]",chData[i]);
		}
		else
			sprintf((char *)&bBuf[strlen((char *)bBuf)],"%02X",chData[i]);
	}
	log_msg(inLogLevel,(char *)bBuf);
	ECC_free(bBuf);
}

void vdCloseAntenn(void)
{
BYTE bInData[10],bOutData[10];
int inRetVal;

	memset(bInData,0x00,sizeof(bInData));
	memset(bOutData,0x00,sizeof(bOutData));

	bInData[0] = 0x01;
	inRetVal = inReaderCommand(DLL_ANTENN_CONTROL,&srXMLData,bInData,bOutData,FALSE,0,FALSE);
	log_msg(LOG_LEVEL_ERROR,"vdCloseAntenn 1(%d)",inRetVal);
}

void PKCS5Padding(BYTE *bInData,int inInLen,int inPadBase)
{
int i,inCnt = 0;

	inCnt = inInLen % inPadBase;
	//if(inCnt > 0)
		inCnt = inPadBase - inCnt;
	for(i=0;i<inCnt;i++)
	{
		bInData[inInLen + i] = inCnt % 256;
	}
}

void vdICERTest(int inCnt)
{
int i;
BYTE bBuf[50];

	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"Test-%d ",inCnt);
	for(i=0;i<5;i++)
	{
		if(srXMLData.srXMLElement[i].chTag[0] == '?')
			sprintf((char *)&bBuf[strlen((char *)bBuf)],"[%i]=1(%c)/",i,(srXMLData.srXMLElement[i].fXMLSendFlag == TRUE)?'1':'0');
		else if(!memcmp((char *)srXMLData.srXMLElement[i].chTag,TAG_TRANS_XML_HEADER,strlen(TAG_TRANS_XML_HEADER)))
			sprintf((char *)&bBuf[strlen((char *)bBuf)],"[%i]=2(%c)/",i,(srXMLData.srXMLElement[i].fXMLSendFlag == TRUE)?'1':'0');
		else if(!memcmp((char *)srXMLData.srXMLElement[i].chTag,TAG_TRANS_HEADER,strlen(TAG_TRANS_HEADER)))
			sprintf((char *)&bBuf[strlen((char *)bBuf)],"[%i]=3(%c)/",i,(srXMLData.srXMLElement[i].fXMLSendFlag == TRUE)?'1':'0');
	}

	log_msg(LOG_LEVEL_ERROR,(char *)bBuf);
}

void vdBuildT4213Tag(STRUCT_XML_DOC *srXML)
{
BYTE bBuf[30],bTmp[10];
int inCnt = 0;

	memset(bBuf,0x00,sizeof(bBuf));
	if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE && srTxnData.srParameter.chCMASMode == '1')//CMAS && 銀行併機
		memcpy(&bBuf[inCnt],&srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID[2],8);
	else
	{
		memset(bTmp,0x00,sizeof(bTmp));
		vdUIntToAsc(gTmpData.ucCPUSPID,sizeof(gTmpData.ucCPUSPID),bTmp,8,FALSE,'0',10);
		memcpy(&bBuf[inCnt],bTmp,8);
	}
	inCnt += 8;

	bBuf[inCnt++] = '-';

	memset(bTmp,0x00,sizeof(bTmp));
	vdUIntToAsc(gTmpData.ucCPULocationID,sizeof(gTmpData.ucCPULocationID),bTmp,5,FALSE,'0',10);
	memcpy(&bBuf[inCnt],bTmp,5);
	inCnt += 5;

	bBuf[inCnt++] = '-';

	memcpy(&bBuf[inCnt],srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID,10);
	inCnt += 10;

	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4213,bBuf,FALSE);

}

void vdURTDataProcess(STRUCT_XML_DOC *srXML,BYTE ucPurseVersionNumber,TRANSPORT_DATA *URT)
{
BYTE bTmp[20],bT484500[10 + 1],bT484501[14 + 1],bT484502[2 + 1],bT484503[20 + 1],bT484504[20 + 1],bT484505[4 + 1],bT484506[5 + 1],bT484507[16 + 1];

	memset(bTmp,0x00,sizeof(bTmp));
	memset(bT484500,0x00,sizeof(bT484500));
	memset(bT484501,0x00,sizeof(bT484501));
	memset(bT484502,0x00,sizeof(bT484502));
	memset(bT484503,0x00,sizeof(bT484503));
	memset(bT484504,0x00,sizeof(bT484504));
	memset(bT484505,0x00,sizeof(bT484505));
	memset(bT484506,0x00,sizeof(bT484506));
	memset(bT484507,0x00,sizeof(bT484507));

	vdUIntToAsc(URT->ucTransactionSequenceNumber,sizeof(URT->ucTransactionSequenceNumber),bT484500,sizeof(bT484500),TRUE,0x00,10);
	UnixToDateTime(URT->ucTransactionDate,bT484501,14);
	fnUnPack(URT->ucTransactionType,sizeof(URT->ucTransactionType),bT484502);
	vdIntToAsc(URT->ucValueofTransaction,sizeof(URT->ucValueofTransaction),bT484503,sizeof(bT484503),TRUE,0x00,10);
	strcat((char *)bT484503,"00");
	vdIntToAsc(URT->ucValueaftertransaction,sizeof(URT->ucValueaftertransaction),bT484504,sizeof(bT484504),TRUE,0x00,10);
	strcat((char *)bT484504,"00");
	fnUnPack(URT->ucTransfergroupcode,sizeof(URT->ucTransfergroupcode),bT484505);
	fnUnPack(URT->ucTransactionLocationID,sizeof(URT->ucTransactionLocationID),bT484506);
	fnBINTODEVASC(URT->ucTransactionDeviceID,bT484507,(ucPurseVersionNumber == MIFARE)?9:16,ucPurseVersionNumber);
	if(URT->ucTransfergroupcode[0] > 0x0F)
		bTmp[0] = 'F';
	else
		bTmp[0] = bT484505[1];
	if(URT->ucTransfergroupcode[1] > 0x0F)
		bTmp[1] = 'F';
	else
		bTmp[1] = bT484505[3];
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4833,bTmp,FALSE);
	inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4834,bT484505,FALSE);

	memset(bTmp,0x00,sizeof(bTmp));
	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_4845,(char *)bTmp,TRUE,NODE_NO_SAME,TAG_NAME_START,FALSE);
	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_484500,(char *)bT484500,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_484501,(char *)bT484501,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_484502,(char *)bT484502,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_484503,(char *)bT484503,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_484504,(char *)bT484504,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_484505,(char *)bT484505,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_484506,(char *)bT484506,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_484507,(char *)bT484507,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_4845,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,FALSE);
	inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/TRANS",FALSE);
}

int inCheckDiscountCanVoid(BYTE *bOutData)
{
int inRetVal;
long lnEV_Bef = 0L,lnEV_Aft = 0L;
STRUCT_XML_DOC srRevXMLData;

	inRetVal = inParseXML((char *)FILE_ICER_REV_BAK,(BYTE *)NULL,TRUE,0,&srRevXMLData,3);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"fCheckDiscountCanVoid failed 1:%d",inRetVal);
		return ICER_ERROR;
	}

	lnEV_Aft = lnAmt3ByteToLong(((TxnReqOnline_APDU_Out *)bOutData)->ucEV);
	inXMLGetAmt(&srRevXMLData,(char *)TAG_NAME_0410,&lnEV_Bef,0);
	vdFreeXMLDOC(&srRevXMLData);

	if(lnEV_Bef - srTxnData.srIngData.lnECCAmt != lnEV_Aft)
	{
		log_msg(LOG_LEVEL_ERROR,"fCheckDiscountCanVoid failed 2:%ld - %ld != %ld",lnEV_Bef,srTxnData.srIngData.lnECCAmt,lnEV_Aft);
		return ICER_ERROR;
	}

	return SUCCESS;
}

void vdSetICERTxnParameter(char *chOnlineFlag,char *chBatchFlag,char *chTCPIP_SSL,char *PacketLenFlag)
{
	*chOnlineFlag = srTxnData.srParameter.chOnlineFlag;
	*chBatchFlag = srTxnData.srParameter.chBatchFlag;
	*chTCPIP_SSL = srTxnData.srParameter.chTCPIP_SSL;
	*PacketLenFlag = srTxnData.srParameter.PacketLenFlag;
	srTxnData.srParameter.chOnlineFlag = ICER_ONLINE;
	if (srTxnData.srParameter.gICERConnMode == 1)
	{
		srTxnData.srParameter.PacketLenFlag = '0';
		srTxnData.srParameter.chTCPIP_SSL = '1';
	}
	else
	{
		srTxnData.srParameter.PacketLenFlag = '1';
		srTxnData.srParameter.chTCPIP_SSL = '0';
       }
}

void vdGetICERTxnParameter(char chOnlineFlag,char chBatchFlag,char chTCPIP_SSL,char PacketLenFlag)
{
	srTxnData.srParameter.chOnlineFlag = chOnlineFlag;
	srTxnData.srParameter.chBatchFlag = chBatchFlag;
	srTxnData.srParameter.PacketLenFlag = PacketLenFlag;
	srTxnData.srParameter.chTCPIP_SSL = chTCPIP_SSL;
}

void vdICERBuildETxnPCode(BYTE *ucMsgType,BYTE *ucSubType)
{

	switch(srTxnData.srIngData.inTransType)
	{
		case TXN_E_ADD:
			ucMsgType[0] = 0x02;
			ucSubType[0] = 0xCB;
			break;
		case TXN_E_DEDUCT:
			ucMsgType[0] = 0x01;
			ucSubType[0] = 0xC9;
			break;
		case TXN_E_REFUND:
			ucMsgType[0] = 0x02;
			ucSubType[0] = 0xCD;
			break;
		case TXN_E_VOID_ADD:
			ucMsgType[0] = 0x0B;
			ucSubType[0] = 0xCF;
			break;
		case TXN_E_QR_ADD:
			ucMsgType[0] = 0x02;
			ucSubType[0] = 0xCC;
			break;
		case TXN_E_QR_DEDUCT:
			ucMsgType[0] = 0x01;
			ucSubType[0] = 0xCA;
			break;
		case TXN_E_QR_REFUND:
			ucMsgType[0] = 0x02;
			ucSubType[0] = 0xCE;
			break;
		case TXN_E_QR_VOID_ADD:
			ucMsgType[0] = 0x0B;
			ucSubType[0] = 0xD0;
			break;
	}
}

