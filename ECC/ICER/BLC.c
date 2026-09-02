
#ifdef EZ_AIRLINKEDC
#include "_StdAfx.h"
#else
#include "stdAfx.h"
#endif
//#include "Global.h"

char gBlcFileFolder[100];
//extern BYTE gsBlackListVersion[30];
#if READER_MANUFACTURERS==ANDROID_API
extern BYTE gCurrentFolder[MAX_PATH];
//extern STRUCT_TXN_DATA srTxnData;
extern const char *JNI_filePath;
extern const char *JNI_LogPath;
extern JNIEnv *Jenv;
extern jobject Jni_usbManager,Jni_context;
extern jobject ReaderObj;
extern jmethodID AndroidReaderInit;
  #ifdef LIB_DEFINE
	#ifdef LIB_DEFINE2
		jmethodID AndroidUart0infoProcess2;
		jmethodID AndroidEccRunIdle;
	#endif
  #endif

#endif

#ifdef LIB_DEFINE

	#ifdef LIB_DEFINE2

		extern UART_RECIEVE_INFO g_sUart0RecieveInfo2;
		extern UART_SEND_INFO	  g_sUart0AnswerInfo2;
		extern unsigned char chAnswerLen;
		extern volatile unsigned int iUart0RecieveOK;
		extern unsigned char chSerialPortMode;
		extern unsigned char chProtectFlag;

	#else

		extern UART_RECIEVE_INFO g_sUart0RecieveInfo;
		extern UART_SEND_INFO	  g_sUart0AnswerInfo;
		extern char g_cProtectFlag;

	#endif

#endif

//compare card nb of black list content
int compare2(const void *a, const void *b)
{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
INT64 llnA = 0L,llnB = 0L;

	memcpy((char *)&llnA,*(char**)a,7);
	memcpy((char *)&llnB,(char**)b,7);
	if(llnA > llnB)
		return(1);
	else if(llnA < llnB)
		return(-1);
	else
		return(0);
#else
	return 0;
#endif
}

//---------------------------------------------------------------------//
/* ---------------------------------------------------------------------
  fnMaintainBLC .
	Returned status:
		 . return the latest edition of black list version
	Special Notes:
		1.This function will delete blacklist while is not last version

--------------------------------------------------------------------- */
int fnMaintainBLC(void)//1.0.5.5
{
BYTE gsBlackListVersion[30];
#if READER_MANUFACTURERS==WINDOWS_API//暫時不加 by Tim || READER_MANUFACTURERS==WINDOWS_CE_API
	int nRtn = SUCCESS;
	char   filter[MAX_PATH+50], /**blcfile[30]*/blcfile[30][25];
	int    i= 0x00,j= 0x00;
	long   hFile= 0x00;
	struct _finddata_t c_file;
	char chFileName[30];
	memset(filter,0,sizeof(filter));
  	strcpy( gBlcFileFolder, (char *)gCurrentFolder );
	if(srTxnData.srParameter.chFolderCreatFlag == '1' || srTxnData.srParameter.chFolderCreatFlag == 0x00)
	{
		strcat( gBlcFileFolder, "\\BlcFile" ); //dir folder
		strcpy( filter, gBlcFileFolder );
		strcat( filter, "\\BLC?????A_??????.BIG" ); //file type
	}
	else
		strcat( filter, "BLC?????A_??????.BIG" ); //file type

	/*#if READER_MANUFACTURERS==WINDOWS_CE_API
		CreateDirectory((const unsigned short *)gBlcFileFolder, NULL);
	#else
		mkdir(gBlcFileFolder);
	#endif*/

	ECC_CreateDirectory_Lib(gBlcFileFolder);

	memset(blcfile,0x00,sizeof(blcfile));
	/*for (i=0;i<30;i++)
	{
		blcfile[i]=(char*)ECC_calloc(21,sizeof(char));
		if (blcfile[i]==NULL)
		{
			for (;i>0;i--)
				ECC_free(blcfile[i-1]);
			//fnWriteLog((BYTE *)"fnBLCProcedure  malloc FALSE",(BYTE *)&i,4);
			return DONGLE_MALLOC_ERROR;
		}
	}*/
 	hFile = _findfirst(filter, &c_file); //search file
	if(hFile!=-1)
	{
		i=0;
		do
		{
			blcfile[i][0] = '\\';
			memcpy(&blcfile[i][1],&c_file.name[0],3);
			memcpy(&blcfile[i][4],&c_file.name[10],6);
			blcfile[i][10] = '_';
			memcpy(&blcfile[i][11],&c_file.name[3],6);
			i++;
			if(i >= 30)
			{
				qsort((void *)blcfile,i,sizeof(blcfile[0]),compare3); //sort black list
				while ( i > 15)
				{
					memset(chFileName,0x00,sizeof(chFileName));
					chFileName[0] = blcfile[i-1][0];
					memcpy(&chFileName[1],&blcfile[i-1][1],3);
					memcpy(&chFileName[4],&blcfile[i-1][11],6);
					chFileName[10] = blcfile[i-1][10];
					memcpy(&chFileName[11],&blcfile[i-1][4],6);
					strcpy( filter, gBlcFileFolder );
					strcat( filter, chFileName ); //file type
					strcat( filter, ".BIG" );
					#if READER_MANUFACTURERS==WINDOWS_CE_API
						DeleteFile((unsigned short *)filter);
					#else
						remove(filter); // delete blc file while is not the last version
					#endif
					i--;
				}
			}
		}
		while( _findnext( hFile, &c_file )==0 );
		_findclose(hFile);//1.0.5.2
		qsort((void *)blcfile,i,sizeof(blcfile[0]),compare3); //sort black list
		j=i;
		while ( i > 15)
		{
			memset(chFileName,0x00,sizeof(chFileName));
			chFileName[0] = blcfile[i-1][0];
			memcpy(&chFileName[1],&blcfile[i-1][1],3);
			memcpy(&chFileName[4],&blcfile[i-1][11],6);
			chFileName[10] = blcfile[i-1][10];
			memcpy(&chFileName[11],&blcfile[i-1][4],6);
			strcpy( filter, gBlcFileFolder );
			strcat( filter, chFileName ); //file type
			strcat( filter, ".BIG" );
			#if READER_MANUFACTURERS==WINDOWS_CE_API
				DeleteFile((unsigned short *)filter);
			#else
				remove(filter); // delete blc file while is not the last version
			#endif
			i-=1;
		}
	}
	else
	{
		//for (i=0;i<30;i++)
		//	ECC_free(blcfile[i]);
		return DONGLE_NO_BLACKLISTR;
	}

	memset(gsBlackListVersion,0x00,sizeof(gsBlackListVersion));
	memcpy(&gsBlackListVersion[0],&blcfile[0][11],6);
	gsBlackListVersion[6] = '_';
	memcpy(&gsBlackListVersion[7],&blcfile[0][4],6);
	inSetBLCName(gsBlackListVersion);
	//for (i=0;i<30;i++)
	//	ECC_free(blcfile[i]);

	return SUCCESS;
#elif READER_MANUFACTURERS==LINUX_API//this is mean define LINUX_API
//	int nRtn = SUCCESS;
	char   filter[MAX_PATH+50], /**blcfile[30]*/blcfile[30][25],szTmp[30];
	int    i= 0x00,j= 0x00;
	DIR *dir;
	struct dirent *ptr;
//	char szLog[100];
	char chFileName[30];
	regex_t re;
	memset(filter,0,sizeof(filter));
//	memset(gCurrentFolder,0,sizeof(gCurrentFolder));
	if(srTxnData.srParameter.chFolderCreatFlag == '1' || srTxnData.srParameter.chFolderCreatFlag == 0x00)
		sprintf(gBlcFileFolder,"%s/BlcFile",gCurrentFolder);
	else
		sprintf(gBlcFileFolder,"BlcFile");
	mkdir(gBlcFileFolder,0777);
	strcpy( filter, gBlcFileFolder );

	memset(blcfile,0x00,sizeof(blcfile));
	/*for (i=0;i<30;i++)
	{
		blcfile[i]=(char*)ECC_calloc(21,sizeof(char));
		if (blcfile[i]==NULL)
		{
			for (;i>0;i--)
				ECC_free(blcfile[i-1]);
			//fnWriteLog((BYTE *)"fnBLCProcedure  malloc FALSE",(BYTE *)&i,4);
			return DONGLE_MALLOC_ERROR;
		}
	}*/
	dir = opendir(filter);
	if(dir != NULL)
	{
		if(regcomp(&re,"^BLC[[:digit:]]{5}A_[[:digit:]]{6}\\.BIG$",REG_EXTENDED) == 0)
		{
			i=0;
			while((ptr = readdir(dir)) != NULL)
			{
				memset(szTmp,0x00,sizeof(szTmp));
				vdToUpper(szTmp,ptr->d_name);
				if(regexec(&re,szTmp,0,0,0) != 0)
					continue;
				blcfile[i][0] = '/';
				memcpy(&blcfile[i][1],&szTmp[0],3);
				memcpy(&blcfile[i][4],&szTmp[10],6);
				blcfile[i][10] = '_';
				memcpy(&blcfile[i][11],&szTmp[3],6);
				i++;
				if(i >= 30)
				{
					qsort((void *)blcfile,i,sizeof(blcfile[0]),compare3); //sort black list
					while ( i > 15)
					{
						memset(chFileName,0x00,sizeof(chFileName));
						chFileName[0] = blcfile[i-1][0];
						memcpy(&chFileName[1],&blcfile[i-1][1],3);
						memcpy(&chFileName[4],&blcfile[i-1][11],6);
						chFileName[10] = blcfile[i-1][10];
						memcpy(&chFileName[11],&blcfile[i-1][4],6);
						strcpy( filter, gBlcFileFolder );
						strcat( filter, chFileName ); //file type
						strcat( filter, ".BIG" );
						remove(filter); // delete blc file while is not the last version
						i--;
					}
				}
			}
			regfree(&re);
		}
		closedir(dir);
		qsort((void *)blcfile,i,sizeof(blcfile[0]),compare3); //sort black list
		j=i;
		while ( i > 15)
		{
			memset(chFileName,0x00,sizeof(chFileName));
			chFileName[0] = blcfile[i-1][0];
			memcpy(&chFileName[1],&blcfile[i-1][1],3);
			memcpy(&chFileName[4],&blcfile[i-1][11],6);
			chFileName[10] = blcfile[i-1][10];
			memcpy(&chFileName[11],&blcfile[i-1][4],6);
			strcpy( filter, gBlcFileFolder );
			strcat( filter, chFileName ); //file type
			strcat( filter, ".BIG" );
			remove(filter); // delete blc file while is not the last version
			i-=1;
		}
	}
	else
	{
		//for (i=0;i<30;i++)
		//	ECC_free(blcfile[i]);
		return DONGLE_NO_BLACKLISTR;
	}

	memset(gsBlackListVersion,0x00,sizeof(gsBlackListVersion));
	memcpy(&gsBlackListVersion[0],&blcfile[0][11],6);
	gsBlackListVersion[6] = '_';
	memcpy(&gsBlackListVersion[7],&blcfile[0][4],6);
	inSetBLCName(gsBlackListVersion);
//	sprintf(szLog,"%s BLC File Name = %s.BIG(%d)",chGetAPITypeName(inAPIStatus),gsBlackListVersion,strlen((char *)gsBlackListVersion));
//	fnWriteLog((BYTE *)szLog,gsBlackListVersion,strlen((char *)gsBlackListVersion));
	//for (i=0;i<30;i++)
	//	ECC_free(blcfile[i]);

	return SUCCESS;
//#elif READER_MANUFACTURERS==ANDROID_API
#else

	memset(gsBlackListVersion,0x00,sizeof(gsBlackListVersion));
	memcpy(gsBlackListVersion,&srTxnData.srParameter.bBLCName[3],strlen((char *)srTxnData.srParameter.bBLCName) - 4 - 3);//"BLC" & ".BIG"不copy
	inSetBLCName(gsBlackListVersion);

	return SUCCESS;
#endif
}

char fIsCheckBLC(int inTxnType,BYTE *bAPDUOut)
{

	//if(srTxnData.srParameter.gReaderMode != RS232_READER)
	//	return(FALSE);//不檢查黑名單
	log_msg(LOG_LEVEL_FLOW,"fIsCheckBLC 0:%d",inTxnType);

	if(inTxnType != DLL_DEDUCT_VALUE &&//不是扣款
		inTxnType != DDL_DEDUCT_VALUE2 && // 也不是三合一扣款
	 (srTxnData.srIngData.inTransType != TXN_ECC_DEDUCT_TAXI || inTxnType != DLL_TAXI_READ) &&//也不是計程車扣款第一動
	 (srTxnData.srIngData.inTransType != TXN_ECC_DEDUCT_CBIKE || inTxnType != DLL_CBIKE_READ) &&//也不是計程車扣款第一動
	 (srTxnData.srIngData.inTransType != TXN_ECC_DEDUCT_EDCA || inTxnType != DLL_EDCA_READ))//也不是EDCA扣款第一動
	//if(inTxnType != DLL_DEDUCT_VALUE && inTxnType != DLL_EDCA_DEDUCT && inTxnType != DLL_CBIKE_DEDUCT)
	//if(inTxnType != API_PPR_TXNREQOFFLINE)
		return(FALSE);//不檢查黑名單

	/*log_msg(LOG_LEVEL_FLOW,"fIsCheckBLC 1:%02x",bAPDUOut[2]);
	if(bAPDUOut[2] != sizeof(TxnReqOffline_APDU_Out))
		return(FALSE);*///不檢查黑名單

	log_msg(LOG_LEVEL_FLOW,"fIsCheckBLC 2:%02x%02x",bAPDUOut[OUT_DATA_OFFSET + bAPDUOut[2] - 2],bAPDUOut[OUT_DATA_OFFSET + bAPDUOut[2] - 1]);
	if(!memcmp(&bAPDUOut[OUT_DATA_OFFSET + bAPDUOut[2] - 2],"\x90\x00",2))
		return(TRUE);//需檢查黑名單

	log_msg(LOG_LEVEL_FLOW,"fIsCheckBLC 3:%d",inTxnType);
	if(!memcmp(&bAPDUOut[OUT_DATA_OFFSET + bAPDUOut[2] - 2],"\x64\x03",2))
		return(TRUE);//需檢查黑名單

	log_msg(LOG_LEVEL_FLOW,"fIsCheckBLC 4:%d",inTxnType);
	return(FALSE);//不檢查黑名單
}

int inCheckBLC(BYTE *ucCardID)
{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
int inRetVal = 0;
long lnFileLength,lnBLCCnt;
BYTE bBLCFileName[MAX_PATH+50];    //@ 原設為200，可能不夠，加大長度
FILE *fp = (FILE *)NULL;
BLC_DATA *lpBLC = NULL,*lpsBLC = NULL,srBLC;
BLC_HEADER srBLCHeader;
BLC_END srBLCEnd;
BYTE gsBlackListVersion[30];

	log_msg(LOG_LEVEL_FLOW,"inCheckBLC 0:%02x%02x%02x%02x",ucCardID[0],ucCardID[1],ucCardID[2],ucCardID[3]);
	if(fnMaintainBLC() != SUCCESS)
		return(DONGLE_BLOCKCARD_NOT_FIND);
	log_msg(LOG_LEVEL_FLOW,"inCheckBLC 1:%d",inRetVal);

	memset(bBLCFileName,0x00,sizeof(bBLCFileName));
	memset(gsBlackListVersion,0x00,sizeof(gsBlackListVersion));
	inGetBLCName(gsBlackListVersion);
  #if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API

	if(srTxnData.srParameter.chFolderCreatFlag == '1' || srTxnData.srParameter.chFolderCreatFlag == 0x00)
		sprintf((char *)bBLCFileName,"%s\\BlcFile\\BLC%s.BIG",gCurrentFolder,gsBlackListVersion);
	else
		sprintf((char *)bBLCFileName,"BLC%s.BIG",gsBlackListVersion);

  #elif READER_MANUFACTURERS==LINUX_API//this is mean define LINUX_API

	if(srTxnData.srParameter.chFolderCreatFlag == '1' || srTxnData.srParameter.chFolderCreatFlag == 0x00)
		sprintf((char *)bBLCFileName,"%s/BlcFile/BLC%s.BIG",gCurrentFolder,gsBlackListVersion);
	else
		sprintf((char *)bBLCFileName,"BLC%s.BIG",gsBlackListVersion);

  #elif READER_MANUFACTURERS==ANDROID_API

	if(srTxnData.srParameter.chFolderCreatFlag == '1' || srTxnData.srParameter.chFolderCreatFlag == 0x00)
		sprintf((char *)bBLCFileName,"%s/BlcFile/BLC%s.BIG",gCurrentFolder,gsBlackListVersion);
	else
		sprintf((char *)bBLCFileName,"BLC%s.BIG",gsBlackListVersion);

  #else

	if(srTxnData.srParameter.chFolderCreatFlag == '1' || srTxnData.srParameter.chFolderCreatFlag == 0x00)
		sprintf((char *)bBLCFileName,"%s\\BlcFile\\BLC%s.BIG",gCurrentFolder,gsBlackListVersion);
	else
		sprintf((char *)bBLCFileName,"BLC%s.BIG",gsBlackListVersion);

  #endif
	log_msg(LOG_LEVEL_FLOW,"inCheckBLC 2:%s",bBLCFileName);
	fp = fopen((char *)bBLCFileName,"rb");
	if(fp == NULL)
	{
		log_msg(LOG_LEVEL_ERROR,"inCheckBLC  fopen FALSE,%d",fp);
		return DONGLE_OPENFILE_ERROR;
	}
	inRetVal = fseek(fp,0,SEEK_END);
	log_msg(LOG_LEVEL_FLOW,"inCheckBLC 3:%d",inRetVal);
	if(inRetVal != 0)
	{
		fclose(fp);
		//fnWriteLog((BYTE *)"inCheckBLC  fseek FALSE",NULL,0);
		return DONGLE_FSEEK_ERROR;
	}
 	lnFileLength = ftell(fp);//取得檔案大小 單位byte
	log_msg(LOG_LEVEL_FLOW,"inCheckBLC 4:%d",lnFileLength);
 	if((lnFileLength - (sizeof(BLC_HEADER) + sizeof(BLC_END))) % sizeof(BLC_DATA))
 	{
 		fclose(fp);
		//fnWriteLog((BYTE *)"inCheckBLC  lnFileLength FALSE",(BYTE *)&lnFileLength,4);
		return DONGLE_FSEEK_ERROR;
 	}
 	lnBLCCnt = (lnFileLength - (sizeof(BLC_HEADER) + sizeof(BLC_END))) / sizeof(BLC_DATA);
	#if READER_MANUFACTURERS==WINDOWS_CE_API
		fseek(fp,0L,SEEK_SET); //Win CE 不支持該函數，可用fseek函數替換
	#else
   		rewind(fp);//移動指標回檔案起始
	#endif
	lpBLC = (BLC_DATA*)ECC_calloc(lnBLCCnt,sizeof(BLC_DATA));
	log_msg(LOG_LEVEL_FLOW,"inCheckBLC 5:%d",inRetVal);
	if(lpBLC == NULL)
	{
		fclose(fp);
		//fnWriteLog((BYTE *)"inCheckBLC  malloc FALSE",NULL,0);
		return DONGLE_MALLOC_ERROR;
	}
  #if 1
	memset(&srBLCHeader,0x00,sizeof(BLC_HEADER));
	inRetVal = fread(&srBLCHeader,sizeof(BLC_HEADER),1,fp);
	gsBlackListVersion[17] = srBLCHeader.ucBlockingIDFlag;
	inSetBLCName(gsBlackListVersion);
  #else
	fseek(fp,sizeof(BLC_HEADER),SEEK_SET);
  #endif
	inRetVal = fread(lpBLC,sizeof(BLC_DATA),lnBLCCnt,fp);
	log_msg(LOG_LEVEL_FLOW,"inCheckBLC 6:%d",inRetVal);
	inRetVal = ferror(fp);
	if(inRetVal != 0)
	{
		fclose(fp);
		ECC_free((BYTE *)lpBLC);
		//fnWriteLog((BYTE *)"inCheckBLC  ferror FALSE",NULL,0);
		return DONGLE_FERROR_ERROR;
	}
  #if 1
	memset(&srBLCEnd,0x00,sizeof(BLC_END));
	inRetVal = fread(&srBLCEnd,sizeof(BLC_END),1,fp);
  #endif
	log_msg(LOG_LEVEL_FLOW,"inCheckBLC 7:%d",inRetVal);
	fclose(fp);

  #if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
	lpsBLC = (BLC_DATA*)bsearch(&ucCardID,lpBLC,lnBLCCnt,sizeof(BLC_DATA),compare2);
  #else
	#warning "inCheckBLC 1 Not Coding !!"
  #endif
	log_msg(LOG_LEVEL_FLOW,"inCheckBLC 8:%d",lpsBLC);

	memset(&srBLC,0x00,sizeof(BLC_DATA));
	if(lpsBLC == NULL)
	{
		ECC_free((BYTE *)lpBLC);
		return DONGLE_BLOCKCARD_NOT_FIND;
	}
	memcpy(&srBLC,lpsBLC,sizeof(BLC_DATA));
	ECC_free((BYTE *)lpBLC);
	log_msg(LOG_LEVEL_FLOW,"inCheckBLC 9:%02x%02x%02x%02x",srBLC.ucCardID[0],srBLC.ucCardID[1],srBLC.ucCardID[2],srBLC.ucCardID[3]);

	if(!memcmp(ucCardID,srBLC.ucCardID,7))
		return(SUCCESS);
#else //READER_MANUFACTURERS==NE_PAXSXX

	//char bBLCFileName[100];
	//BYTE gsBlackListVersion[30];

	if (fnMaintainBLC() != SUCCESS)
		return(DONGLE_BLOCKCARD_NOT_FIND);

	if (IsInlist((char*)srTxnData.srParameter.bBLCName, ucCardID) == 1)
	{
		return SUCCESS;
	}

/*#else

#warning "inCheckBLC 2 Not Coding !!"*/

#endif

	return DONGLE_BLOCKCARD_NOT_FIND;
}

int inBLCLockCard(int inTxnType,BYTE *cOutputData,BYTE *CardID,BYTE *TxnDateTime)
{
int inRetVal,inCnt = 3;
BYTE ucCardID[7],ucTxnDateTime[4],cAPDU[256];
#if READER_MANUFACTURERS==LINUX_API
//int inOutLen,inStatus;
#endif

	memcpy(ucCardID,CardID,sizeof(ucCardID));
	memcpy(ucTxnDateTime,TxnDateTime,sizeof(ucTxnDateTime));
	memset(cAPDU,0,sizeof(cAPDU));
	memcpy(&cAPDU[inCnt],"\x80\x41\x01\x00\x0E",5);
	inCnt += 5;
	cAPDU[inCnt++] = 0x22;//Msg Type
	cAPDU[inCnt++] = 0x00;//Sub Type
	memcpy(&cAPDU[inCnt],ucCardID,sizeof(ucCardID));
	inCnt += 7;//Card ID
	memcpy(&cAPDU[inCnt],ucTxnDateTime,sizeof(ucTxnDateTime));
	inCnt += 4;//Txn Date Time
	cAPDU[inCnt++] = 0x01;//Reason
	cAPDU[inCnt++] = 0x28;//Le
	cAPDU[2] = inCnt - 3;//LEN
	cAPDU[inCnt] = checksum(inCnt,cAPDU);//EDC
	inCnt++;

	vdWriteLog(cAPDU,inCnt,WRITE_DLL_REQ_LOG,inTxnType);

#ifdef LIB_DEFINE

  #ifdef LIB_DEFINE2

	if(srTxnData.srParameter.gReaderMode == LIBARY_READER2)
	{
		memset((char *)&g_sUart0RecieveInfo2,0x00,sizeof(g_sUart0RecieveInfo2));
		memset((char *)&g_sUart0AnswerInfo2,0x00,sizeof(g_sUart0AnswerInfo2));

		memcpy(g_sUart0RecieveInfo2.cProLog,cAPDU,sizeof(g_sUart0RecieveInfo2.cProLog));
		memcpy(g_sUart0RecieveInfo2.cInfoHeader,&cAPDU[3],sizeof(g_sUart0RecieveInfo2.cInfoHeader));
		memcpy(g_sUart0RecieveInfo2.cInfoBody,&cAPDU[3 + 4],inCnt - 4);
		g_sUart0RecieveInfo2.cEDC = cAPDU[inCnt - 1];
		iUart0RecieveOK = chSerialPortMode = chProtectFlag = 0;

		//log_msg(LOG_LEVEL_ERROR,"Ts_Uart0InfoProcess Start!!");
	#if READER_MANUFACTURERS==ANDROID_API
		jbyteArray jSendData=(*Jenv)->NewByteArray(Jenv, sizeof(g_sUart0RecieveInfo2));
		jbyteArray jRecvData=(*Jenv)->NewByteArray(Jenv, sizeof(g_sUart0AnswerInfo2));
		jbyteArray jAnswerLen=(*Jenv)->NewByteArray(Jenv, 1);
		jintArray jUart0RecieveOK=(*Jenv)->NewIntArray(Jenv, 1 * sizeof(jint));
		jbyteArray jSerialPortMode=(*Jenv)->NewByteArray(Jenv, 1);
		jbyteArray jProtectFlag=(*Jenv)->NewByteArray(Jenv, 1);

		(*Jenv)->SetByteArrayRegion(Jenv, jSendData, 0, sizeof(g_sUart0RecieveInfo2), (jbyte*)&g_sUart0RecieveInfo2);
		//jUart0RecieveOK[0] = 0;
		jstring str1 = (*Jenv)->NewStringUTF(Jenv,JNI_filePath);
		jstring str2 = (*Jenv)->NewStringUTF(Jenv,JNI_LogPath);
		(*Jenv) -> CallVoidMethod(Jenv,ReaderObj,AndroidUart0infoProcess2,str1,str2,Jni_usbManager,Jni_context,jSendData,jRecvData,jAnswerLen,jUart0RecieveOK,jSerialPortMode,jProtectFlag);
		if(jRecvData != NULL)
		{
			jsize len = (*Jenv) -> GetArrayLength(Jenv,jRecvData);
			jbyte* jbarray = (jbyte*) ECC_calloc(len,sizeof(jbyte));
			(*Jenv) -> GetByteArrayRegion(Jenv,jRecvData,0,len,jbarray);
			memcpy(&g_sUart0AnswerInfo2,jbarray,len);
			ECC_free(jbarray);
		}
	#else
		Ts_Uart0InfoProcess2(&g_sUart0RecieveInfo2,&g_sUart0AnswerInfo2,&chAnswerLen,&iUart0RecieveOK,&chSerialPortMode,&chProtectFlag);
	#endif
		//log_msg(LOG_LEVEL_ERROR,"Ts_Uart0InfoProcess End!!");

		/*{
		int i;
		BYTE bTmp[300];

		memset(bTmp,0x00,sizeof(bTmp));
		for(i=0;i<100;i++)
		{
		sprintf((char *)&bTmp[strlen((char *)bTmp)],"%02x",g_sUart0AnswerInfo.cProLog[i]);
		}
		log_msg((char *)bTmp);
		}*/
		memcpy(cOutputData,g_sUart0AnswerInfo2.cProLog,sizeof(g_sUart0AnswerInfo2.cProLog));
		//if(Tscc_cAnswerLen > 0)
		if(g_sUart0AnswerInfo2.cProLog[2] >= 2)
		{
			memcpy(&cOutputData[3],g_sUart0AnswerInfo2.cInfoBody,g_sUart0AnswerInfo2.cProLog[2] - 2);
			memcpy(&cOutputData[3 + g_sUart0AnswerInfo2.cProLog[2] - 2],g_sUart0AnswerInfo2.cSW,2);
			cOutputData[3 + g_sUart0AnswerInfo2.cProLog[2]] = g_sUart0AnswerInfo2.cEDC;
			inRetVal = (cOutputData[cOutputData[2] + 1] << 8) | (cOutputData[cOutputData[2] + 2]);
			vdWriteLog(cOutputData,0x28 + 2 + 4,WRITE_DLL_RES_LOG,inTxnType);
			if(inRetVal == CARD_SUCCESS)
			{
				inRetVal = 0x6406;
				memcpy(((LockCard_APDU_Out_2 *)&cOutputData[OUT_DATA_OFFSET])->ucStatusCode,"\x64\x06",2);
			}
		}
		else
			inRetVal = ICER_ERROR;
	}
	else

  #else

	if(srTxnData.srParameter.gReaderMode == LIBARY_READER)
	{
		memset((char *)&g_sUart0RecieveInfo,0x00,sizeof(g_sUart0RecieveInfo));
		memset((char *)&g_sUart0AnswerInfo,0x00,sizeof(g_sUart0AnswerInfo));

		memcpy(g_sUart0RecieveInfo.cProLog,cAPDU,sizeof(g_sUart0RecieveInfo.cProLog));
		memcpy(g_sUart0RecieveInfo.cInfoHeader,&cAPDU[3],sizeof(g_sUart0RecieveInfo.cInfoHeader));
		memcpy(g_sUart0RecieveInfo.cInfoBody,&cAPDU[3 + 4],inCnt - 4);
		g_sUart0RecieveInfo.cEDC = cAPDU[inCnt - 1];
		g_cProtectFlag = 0;

		//log_msg(LOG_LEVEL_ERROR,"Ts_Uart0InfoProcess Start!!");
		Ts_Uart0InfoProcess(/*(BYTE *)&g_sUart0RecieveInfo,(BYTE *)&g_sUart0AnswerInfo*/);
		//log_msg(LOG_LEVEL_ERROR,"Ts_Uart0InfoProcess End!!");

		/*{
			int i;
			BYTE bTmp[300];

			memset(bTmp,0x00,sizeof(bTmp));
			for(i=0;i<100;i++)
			{
				sprintf((char *)&bTmp[strlen((char *)bTmp)],"%02x",g_sUart0AnswerInfo1.cProLog[i]);
			}
			log_msg((char *)bTmp);
		}*/
		memcpy(cOutputData,g_sUart0AnswerInfo.cProLog,sizeof(g_sUart0AnswerInfo.cProLog));
		//if(Tscc_cAnswerLen > 0)
		if(g_sUart0AnswerInfo.cProLog[2] >= 2)
		{
			memcpy(&cOutputData[3],g_sUart0AnswerInfo.cInfoBody,g_sUart0AnswerInfo.cProLog[2] - 2);
			memcpy(&cOutputData[3 + g_sUart0AnswerInfo.cProLog[2] - 2],g_sUart0AnswerInfo.cSW,2);
			cOutputData[3 + g_sUart0AnswerInfo.cProLog[2]] = g_sUart0AnswerInfo.cEDC;
			inRetVal = (cOutputData[cOutputData[2] + 1] << 8) | (cOutputData[cOutputData[2] + 2]);
			vdWriteLog(cOutputData,0x28 + 2 + 4,WRITE_DLL_RES_LOG,inTxnType);
			if(inRetVal == CARD_SUCCESS)
			{
				inRetVal = 0x6406;
				memcpy(((LockCard_APDU_Out_2 *)&cOutputData[OUT_DATA_OFFSET])->ucStatusCode,"\x64\x06",2);
			}
		}
		else
			inRetVal = ICER_ERROR;
	}
	else

  #endif

#endif
	{
		if(srTxnData.srParameter.chReaderPortocol == READER_INFOCHAMP)
		{
			BYTE bTmp[500];

			memset(bTmp,0x00,sizeof(bTmp));

			bTmp[0] = 0xFA;
			bTmp[1] = (inCnt + 4) / 256;
			bTmp[2] = (inCnt + 4) % 256;
			memcpy(&bTmp[3],"\x02\x01\x00\x00",4);
			memcpy(&bTmp[7],cAPDU,inCnt);
			inCnt += 7;
			bTmp[inCnt] = checksum(inCnt,&bTmp[0]);//EDC
			inCnt++;

			memset(cAPDU,0x00,sizeof(cAPDU));
			memcpy(cAPDU,bTmp,inCnt);
		}
		else if(srTxnData.srParameter.chReaderPortocol == READER_CASTLES)
		{
			BYTE bTmp[500];
			int inCnt1 = 0,inCnt2 = 0,i;

			memset(bTmp,0x00,sizeof(bTmp));

			memcpy(&bTmp[inCnt1],"\x10\x02\x08",3);
			inCnt1 += 9;
			bTmp[inCnt1++] = inCnt / 256;
			bTmp[inCnt1++] = inCnt % 256;
			//memcpy(&bTmp[inCnt1],bSendData,inCnt);
			//inCnt1 += inCnt;
			//DLE:遇到0x10時,要多補一個0x10
			for(i=0;i<inCnt;i++)
			{
				bTmp[inCnt1++] = cAPDU[i];
				if(cAPDU[i] == 0x10)
				{
					bTmp[inCnt1++] = 0x10;
					inCnt2++;
					//log_msg(LOG_LEVEL_FLOW,"inReaderCommand2 DLE 0x10:(%d)",i);
				}
			}
			bTmp[inCnt1++] = checksum(inCnt + 9 + inCnt2,&bTmp[2]);//EDC
			if(inCnt2 % 2)
				bTmp[inCnt1 - 1] = bTmp[inCnt1 - 1] ^ 0x10;
			if(bTmp[inCnt1 - 1] == 0x10)
			{
				bTmp[inCnt1++] = 0x10;
				inCnt2++;
			}
			memcpy(&bTmp[inCnt1],"\x10\x03",2);
			inCnt1 += 2;

			memset(cAPDU,0x00,sizeof(cAPDU));
			memcpy(cAPDU,bTmp,inCnt1);
			inCnt = inCnt1;
		}

		inRetVal = inComSend(cAPDU,inCnt);

		if(inRetVal != SUCCESS)
		{
			//vdWriteDongleLog(API_PPR_LOCKCARD2,inRetVal,"Request",cAPDU,inCnt);
			return(inRetVal);
		}
		inRetVal = inComRecv(cOutputData,sizeof(LockCard_APDU_Out_2) + 4,4000);

		if(srTxnData.srParameter.chReaderPortocol == READER_INFOCHAMP)
		{
			BYTE bTmp[500];
			int inLen = 0;

			memset(bTmp,0x00,sizeof(bTmp));

			inLen = (cOutputData[1] * 256) + cOutputData[2];
			bTmp[2] = (inLen - 4);
			memcpy(&bTmp[3],&cOutputData[7],bTmp[2] + 1);

			//memset(cOutputData,0x00,sizeof(cOutputData));
			memcpy(cOutputData,bTmp,sizeof(bTmp));
		}
		else if(srTxnData.srParameter.chReaderPortocol == READER_CASTLES)
		{
			BYTE bTmp[READER_BUFFER];
			int inLen = 0;

			inLen = cOutputData[9] * 256 + cOutputData[10];;
			log_msg_debug(LOG_LEVEL_ERROR,FALSE,(char *)"DLE Header",10,cOutputData,inLen + 14);

			memset(bTmp,0x00,sizeof(bTmp));

			inLen = cOutputData[13];
			memcpy(bTmp,&cOutputData[11],inLen + 4);

			//memset(cOutputData,0x00,sizeof(cOutputData))
			memcpy(cOutputData,bTmp,sizeof(bTmp));

			inLen = cAPDU[9] * 256 + cAPDU[10];
			memcpy(cAPDU,&cAPDU[11],inLen);
		}

		if(inRetVal == SUCCESS)
		{
			memcpy(((LockCard_APDU_Out_2 *)&cOutputData[OUT_DATA_OFFSET])->ucStatusCode,"\x64\x06",2);
			cOutputData[cOutputData[2]+3] = checksum(cOutputData[2] + 3,cOutputData);//EDC
			inRetVal = (cOutputData[cOutputData[2] + 1] << 8) | (cOutputData[cOutputData[2] + 2]);
			vdWriteLog(cOutputData,0x28 + 2 + 4,WRITE_DLL_RES_LOG,inTxnType);
		}
	}

	return(inRetVal);
}

#if READER_MANUFACTURERS!=WINDOWS_API && READER_MANUFACTURERS!=LINUX_API && READER_MANUFACTURERS!=ANDROID_API && READER_MANUFACTURERS!=WINDOWS_CE_API
short ECC_file_read(char* i_pc_filename, unsigned char* o_puc_buffer, int i_l_offset, int *i_l_length)
{
	int fp,ret;

	fp = ECC_FileOpen_Lib((BYTE *)i_pc_filename,(BYTE *)"rb");
	if(fp == 0x00)
	{
		return ICER_ERROR;
	}

	if(i_l_offset >= 0)
		ECC_FileSeek_Lib(fp, i_l_offset * *i_l_length, SEEK_SET);
	else
		ECC_FileSeek_Lib(fp, (0 - *i_l_length), SEEK_END);

	ret = ECC_FileRead_Lib(fp, o_puc_buffer, *i_l_length);
	*i_l_length = ret;
	ECC_FileClose_Lib(fp);

	return SUCCESS;
}

short file_read2(char* i_pc_filename, unsigned char* o_puc_buffer, int i_l_offset, int *i_l_length)
{
	int fp,ret;
	unsigned char tmpbuf[1024];
	memset(tmpbuf,0x00,1024);
	fp = ECC_FileOpen_Lib((BYTE *)i_pc_filename,(BYTE *)"rb");
	if(fp == 0)
	{
		return ICER_ERROR;
	}

	//log_msg(LOG_LEVEL_ERROR,"file_read2 1,%02x%02x%02x%02x,%d,%d",o_puc_buffer[0],o_puc_buffer[1],o_puc_buffer[2],o_puc_buffer[3],*i_l_length,ret);
	if(i_l_offset >= 0)
		ECC_FileSeek_Lib(fp,i_l_offset,SEEK_SET);
	else
		ECC_FileSeek_Lib(fp,(i_l_offset),SEEK_END);

	//long offset=ftell(fp);

	ret = ECC_FileRead_Lib(fp,o_puc_buffer,*i_l_length);
	//log_msg(LOG_LEVEL_ERROR,"file_read2 2,%02x%02x%02x%02x,%d,%d",o_puc_buffer[0],o_puc_buffer[1],o_puc_buffer[2],o_puc_buffer[3],*i_l_length,ret);

	*i_l_length = ret;
	ECC_FileClose_Lib(fp);
	return SUCCESS;
}

short IsInlist(char* i_pc_filename, BYTE *ucCardID)
{
	int filesize,inRetVal;
	int recsize = 9, ret, inCardLens = 8, inMAXRec;
	int currec,toprec,bottomrec;
	//unsigned short beginoffset;
	int inHeaderCnt = 25, inFooterCnt = 32;
	long lnTotalCnt = 0;
	BYTE	szHeader[25], szFooter[32], szTotal[8], szBlackCardNo[8], filename[40],bBuf1[10], bBuf2[10];
	unsigned long long ulBlackCardNo, ulTxnCardNo;

	sprintf((char *)filename,"%s", (char *)i_pc_filename);
	filesize=ECC_FileSize_Lib((BYTE *)filename);
	log_msg(LOG_LEVEL_FLOW,"IsInlist 1 %d", filesize);
	toprec=(filesize-inHeaderCnt - inFooterCnt)/recsize;
	if(toprec <= 0)
	{
		log_msg(LOG_LEVEL_ERROR,"IsInlist BLC File Error(%s),%d - %d - %d / %d = %d",filename,filesize,inHeaderCnt,inFooterCnt,recsize,toprec);
		return 0;
	}

	inMAXRec = toprec;
	memset(szHeader, 0x00, sizeof(szHeader));
	memset(szFooter, 0x00, sizeof(szFooter));
	ECC_file_read((char *)filename, (BYTE *)szHeader, 0, &inHeaderCnt);
	ECC_file_read((char *)filename, (BYTE *)szFooter, -1, &inFooterCnt);
	memcpy(szTotal, &szFooter[21], 8);
	lnTotalCnt = atol((char *)szTotal);
	bottomrec=1;
	currec=(toprec+bottomrec)/2;

	//memcpy(&ulTxnCardNo, ucCardID, 7);
	vdChangeEndian(ucCardID, bBuf1, 8);
	log_msg(LOG_LEVEL_FLOW,"IsInlist 3 %d", 0);

	while(1)
	{
		if(toprec > inMAXRec || bottomrec < 0 || currec > inMAXRec)
		{
			log_msg(LOG_LEVEL_FLOW,"IsInlist 4 %d,%d,%d,%d", toprec ,inMAXRec ,bottomrec ,currec);
			break;
		}
		memset(szBlackCardNo, 0x00, sizeof(szBlackCardNo));
		log_msg(LOG_LEVEL_FLOW,"IsInlist 5 %d,%d,%d,%d", currec ,recsize,inHeaderCnt, inCardLens);
		ret=file_read2((char *)filename,(unsigned char*)szBlackCardNo,(currec-1)*recsize + inHeaderCnt,&inCardLens);
		log_msg(LOG_LEVEL_FLOW,"IsInlist 6 %d", ret);
		if((ret != 0 ) ||( inCardLens !=  8))
		{
			log_msg(LOG_LEVEL_FLOW,"IsInlist 7 %d,%d", ret, inCardLens);
			break;
		}
		memcpy(&ulBlackCardNo, szBlackCardNo , inCardLens);
		log_msg(LOG_LEVEL_FLOW,"IsInlist 8 [%02x%02x%02x%02x%02x%02x%02x],[%02x%02x%02x%02x%02x%02x%02x]", ucCardID[0], ucCardID[1], ucCardID[2], ucCardID[3], ucCardID[4], ucCardID[5], ucCardID[6], szBlackCardNo[0], szBlackCardNo[1], szBlackCardNo[2], szBlackCardNo[3], szBlackCardNo[4], szBlackCardNo[5], szBlackCardNo[6]);
		log_msg(LOG_LEVEL_FLOW,"IsInlist 9 %llu,%llu", ulTxnCardNo, ulBlackCardNo);

		vdChangeEndian(szBlackCardNo, bBuf2, 8);
		inRetVal = memcmp(bBuf1, bBuf2,8);
		log_msg(LOG_LEVEL_FLOW,"IsInlist 10 %02x%02x%02x%02x:%02x%02x%02x%02x",bBuf1[0],bBuf1[1],bBuf1[2],bBuf1[3],bBuf2[0],bBuf2[1],bBuf2[2],bBuf2[3]);
		if(inRetVal == 0)
		//if(ulTxnCardNo==ulBlackCardNo)
		{
			log_msg(LOG_LEVEL_FLOW,"IsInlist 11 %d,%d", ulTxnCardNo , ulBlackCardNo);
			return  1;
		}
		else
		if(inRetVal > 0)
		//if(ulTxnCardNo > ulBlackCardNo)
		{
			if (bottomrec == currec)
			{
				log_msg(LOG_LEVEL_FLOW,"IsInlist 12 %d,%d", bottomrec, currec);
				break;
			}
			else
			{
				bottomrec = currec;
				currec=(toprec+bottomrec)/2;
				if (currec ==bottomrec )
					currec ++;
			}
		}
		else
		if (inRetVal < 0)
		//if(ulTxnCardNo < ulBlackCardNo)
		{
			if(toprec == currec)
			{
				log_msg(LOG_LEVEL_FLOW,"IsInlist 13 %d,%d", toprec, currec);
				break;
			}
			else
			{
				toprec = currec;
				currec=(toprec+bottomrec)/2;
			}
		}
	}
	log_msg(LOG_LEVEL_FLOW,"IsInlist 14");
	return 0;
}

void vdChangeEndian(BYTE *bInData,BYTE *bOutData,int inInDataLen)
{
	int i;
	BYTE Tmp[50];

	memset((char *)Tmp,0x00, sizeof(Tmp));
	memcpy((char *)Tmp,(char *)bInData,inInDataLen);
	memset((char *)bOutData,0x00, inInDataLen);
	for (i = 0;i < inInDataLen;i++)
	{
		bOutData[i] = Tmp[inInDataLen - 1 - i];
	}
}

#endif

#if READER_MANUFACTURERS==WINDOWS_CE_API
void * __cdecl bsearch ( const void *key, const void *base, size_t num, size_t width, int (__cdecl *compare)(const void *, const void *) )
{
     char *lo = (char *)base;
     char *hi = (char *)base + (num - 1) * width;
     char *mid;
     unsigned int half;
     int result;

     while (lo <= hi)
     if (half = num / 2)
     {
     	mid = lo + (num & 1 ? half : (half - 1)) * width;
     	if (!(result = (*compare)(key,mid)))
     		return(mid);
     	else if (result < 0)
     	{
	     hi = mid - width;
	     num = num & 1 ? half : half-1;
	     }
	    else 
		{
	     lo = mid + width;
	     num = half;
	     }
     }
     else if (num)
     return((*compare)(key,lo) ? NULL : lo);
     else
     break;

     return(NULL);
}
#endif
