
#ifdef EZ_AIRLINKEDC
#include "_stdAfx.h"
#else
#include "stdAfx.h"
#endif
//#include "Global.h"

//#ifdef ICERAPI_ONLY
#include "../../SOURCE/FUNCTION/File.h"

unsigned long ulHeapSize = 0L;
extern	int	ginTrans_ClientFd_inLib;

#ifdef	FLASH_SYSTEM

STRUCT_FLASH_PAR st_FlashPar[MAX_FILE_CNT] = {
//	bFileName					ulStartAddress				ulFlashMaxSize				ulEndAddress
	{"ICERAPI_CMAS.rev" 		,FLASH_START_CMAS_REV		,FLASH_CMAS_REV_SIZE		,FLASH_START_CMAS_REV + FLASH_CMAS_REV_SIZE},
	{"ICERAPI_CMAS.rev.bak"		,FLASH_START_CMAS_REV_BAK	,FLASH_CMAS_REV_BAK_SIZE	,FLASH_START_CMAS_REV_BAK + FLASH_CMAS_REV_BAK_SIZE},
	{"ICERAPI_CMAS.adv" 		,FLASH_START_CMAS_ADV		,FLASH_CMAS_ADV_SIZE		,FLASH_START_CMAS_ADV + FLASH_CMAS_ADV_SIZE},
	{"ICERAPI.tmp"	 			,FLASH_START_TMP			,FLASH_TMP_SIZE				,FLASH_START_TMP + FLASH_TMP_SIZE},
	{"ICERAPI.Log"				,FLASH_START_LOG			,FLASH_LOG_SIZE 			,FLASH_START_LOG + FLASH_LOG_SIZE}
};

STRUCT_FLASH_DATA srFlashData;
int inStorageType;
BOOL fStorageMode;
#endif

#if READER_MANUFACTURERS==WINDOWS_CE_API
wchar_t wBuf1[100],wBuf2[100];
#endif

int ECC_FileOpen(BYTE* bFileName,BYTE* bFileMode)
{
int inRetVal = 0;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
#else

	#ifdef	FLASH_SYSTEM

		if(fCheckStorageMode(bFileName) == STORAGE_FLASH)
		{
			log_msg(LOG_LEVEL_FLOW,"ECC_FileOpen 0:%s",bFileName);
			return SUCCESS;
		}

	#endif

	#ifdef FILE_NAME_LIMIT
		if(strlen((char *)bFileName) > FILE_NAME_LIMIT)
			inRetVal = ECC_FileOpen_Lib(&bFileName[strlen((char *)bFileName) - FILE_NAME_LIMIT],bFileMode);
		else
	#endif
			inRetVal = ECC_FileOpen_Lib(bFileName,bFileMode);

#endif

	return inRetVal;
}

int ECC_FileRead(int FileHandle,BYTE* bFileData,unsigned short usRLen)
{
int inRetVal = 0;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
#else

	#ifdef	FLASH_SYSTEM

		if(fStorageMode == STORAGE_FLASH)
		{
			inRetVal = ICER_ReadDataFlash(srFlashData.ulFlashEndDataAddr[inStorageType],bFileData,usRLen);
			log_msg(LOG_LEVEL_FLOW,"ECC_FileRead 1,%d,%d,%d",inRetVal,usRLen,srFlashData.ulFlashEndDataAddr[inStorageType]);
			if(inRetVal == SUCCESS)
			{
				srFlashData.ulFlashEndDataAddr[inStorageType] += usRLen;
				return usRLen;
			}
			return ICER_ERROR;
		}

	#endif

	inRetVal = ECC_FileRead_Lib(FileHandle,bFileData,usRLen);

#endif

	return inRetVal;
}

int ECC_FileWrite(int FileHandle,BYTE* bFileData,unsigned short usWLen)
{
int inRetVal = 0;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
#else

	#ifdef	FLASH_SYSTEM

		if(fStorageMode == STORAGE_FLASH)
		{
			if(srFlashData.ulFlashEndDataAddr[inStorageType] + usWLen <= st_FlashPar[inStorageType].ulEndAddress)//欲寫入資料長度尚在範圍內
			{
				inRetVal = ICER_WriteDataFlash(srFlashData.ulFlashEndDataAddr[inStorageType],bFileData,usWLen);
				log_msg(LOG_LEVEL_FLOW,"ECC_FileWrite 1,%d,%ld,%ld",inRetVal,usWLen,srFlashData.ulFlashEndDataAddr[inStorageType]);
				if(inRetVal == SUCCESS)
				{
					srFlashData.ulFlashEndDataAddr[inStorageType] += usWLen;
					srFlashData.ulFlashDataSize[inStorageType] = srFlashData.ulFlashEndDataAddr[inStorageType] - st_FlashPar[inStorageType].ulStartAddress;
					//log_msg(LOG_LEVEL_ERROR,"ECC_FileWrite (%ld)(%ld)(%ld)[%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x]",srFlashData.ulFlashEndDataAddr[inStorageType],srFlashData.ulFlashDataSize[inStorageType],st_FlashPar[inStorageType].ulStartAddress,bFileData[0],bFileData[1],bFileData[2],bFileData[3],bFileData[4],bFileData[5],bFileData[6],bFileData[7],bFileData[8],bFileData[9]);

					//inWriteFlasfData();//移到inFinalizer()最後面執行!!
					return usWLen;
				}
			}
			else//欲寫入資料長度已超出範圍,不寫入,回覆失敗!!
			/*{//欲寫入資料長度已超出範圍,需覆蓋舊資料!!
				unsigned long ulFirstStoreCnt = 0L;

				ulFirstStoreCnt = st_FlashPar[inStorageType].ulEndAddress - srFlashData.ulFlashEndDataAddr[inStorageType];
				inRetVal = ICER_WriteDataFlash(srFlashData.ulFlashEndDataAddr[inStorageType],bFileData,ulFirstStoreCnt);
				if(inRetVal == SUCCESS)
				{
					srFlashData.ulFlashEndDataAddr[inStorageType] = st_FlashPar[inStorageType].ulEndAddress;
					srFlashData.ulFlashDataSize[inStorageType] = st_FlashPar[inStorageType].ulFlashMaxSize;

					//inWriteFlasfData();//移到inFinalizer()最後面執行!!
					//return usWLen;
				}
				else
					return ICER_ERROR;

				inRetVal = ICER_WriteDataFlash(st_FlashPar[inStorageType].ulStartAddress,&bFileData[ulFirstStoreCnt],usWLen - ulFirstStoreCnt);
				if(inRetVal == SUCCESS)
				{
					srFlashData.ulFlashEndDataAddr[inStorageType] = st_FlashPar[inStorageType].ulStartAddress + usWLen - ulFirstStoreCnt;
					srFlashData.ulFlashDataSize[inStorageType] = st_FlashPar[inStorageType].ulFlashMaxSize;

					//inWriteFlasfData();//移到inFinalizer()最後面執行!!
					return usWLen;
				}
				else
					return ulFirstStoreCnt;
			}*/
			//log_msg(LOG_LEVEL_ERROR,"ECC_FileWrite Fail 1(%d),(%ld),(%d),(%ld)",inStorageType,srFlashData.ulFlashEndDataAddr[inStorageType],usWLen,st_FlashPar[inStorageType].ulEndAddress);
			return ICER_ERROR;
		}

	#endif

	inRetVal = ECC_FileWrite_Lib(FileHandle,bFileData,usWLen);

#endif

	return inRetVal;
}

int ECC_FileClose(int FileHandle)
{
int inRetVal = 0;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
#else

	#ifdef	FLASH_SYSTEM

		if(fStorageMode == STORAGE_FLASH)
		{
			fStorageMode = STORAGE_FILE;
			return SUCCESS;
		}

	#endif

	inRetVal = ECC_FileClose_Lib(FileHandle);

#endif

	return inRetVal;
}

int ECC_FileSeek(int FileHandle,long lnOffset,unsigned char bFormWhere)
{
int inRetVal = 0;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
#else

	#ifdef	FLASH_SYSTEM

		if(fStorageMode == STORAGE_FLASH)
		{
			if(bFormWhere == SEEK_SET)
				srFlashData.ulFlashEndDataAddr[inStorageType] = st_FlashPar[inStorageType].ulStartAddress + lnOffset;
			else if(bFormWhere == SEEK_CUR)
				srFlashData.ulFlashEndDataAddr[inStorageType] += lnOffset;
			else if(bFormWhere == SEEK_END)
				srFlashData.ulFlashEndDataAddr[inStorageType] = st_FlashPar[inStorageType].ulStartAddress + srFlashData.ulFlashDataSize[inStorageType] + lnOffset;
			log_msg(LOG_LEVEL_FLOW,"ECC_FileSeek (%ld),(%ld),(%ld),(%ld)",srFlashData.ulFlashEndDataAddr[inStorageType],st_FlashPar[inStorageType].ulStartAddress,st_FlashPar[inStorageType].ulEndAddress,lnOffset);
			//inWriteFlasfData();//移到inFinalizer()最後面執行!!
			return SUCCESS;
		}

	#endif

	inRetVal = ECC_FileSeek_Lib(FileHandle,lnOffset,bFormWhere);

#endif

	return inRetVal;

}

int ECC_FileSize(BYTE* bFileName)
{
int inRetVal = 0;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
#else

	#ifdef	FLASH_SYSTEM

		if(fCheckStorageMode(bFileName) == STORAGE_FLASH)
		{
			log_msg(LOG_LEVEL_FLOW,"ECC_FileSize 0:%s,%d",bFileName,srFlashData.ulFlashDataSize[inStorageType]);
			return srFlashData.ulFlashDataSize[inStorageType];
		}

	#endif

	#ifdef FILE_NAME_LIMIT

		if(strlen((char *)bFileName) > FILE_NAME_LIMIT)
			inRetVal = ECC_FileSize_Lib(&bFileName[strlen((char *)bFileName) - FILE_NAME_LIMIT]);
		else

	#endif
			inRetVal = ECC_FileSize_Lib(bFileName);

#endif

	return inRetVal;
}

int ECC_FileRemove(BYTE* bFileName)
{
int inRetVal = 0;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API

	inRetVal = remove((char *)bFileName);

#elif READER_MANUFACTURERS==WINDOWS_CE_API

	memset(wBuf1,0x00,sizeof(wBuf1));
	MultiByteToWideChar(CP_UTF8, 0, (char *)bFileName, 100, wBuf1, 100);
	inRetVal = DeleteFile(wBuf1);

#else

	#ifdef	FLASH_SYSTEM

		if(fCheckStorageMode(bFileName) == STORAGE_FLASH)
		{
			log_msg(LOG_LEVEL_FLOW,"ECC_FileRemove 0:%s,%d,%d",bFileName,srFlashData.ulFlashDataSize[inStorageType],srFlashData.ulFlashEndDataAddr[inStorageType]);
			srFlashData.ulFlashDataSize[inStorageType] = 0;
			srFlashData.ulFlashEndDataAddr[inStorageType] = st_FlashPar[inStorageType].ulStartAddress;
			//inWriteFlasfData();//移到inFinalizer()最後面執行!!
			return SUCCESS;
		}
	#endif

	#ifdef FILE_NAME_LIMIT
		if(strlen((char *)bFileName) > FILE_NAME_LIMIT)
			inRetVal = ECC_FileRemove_Lib(&bFileName[strlen((char *)bFileName) - FILE_NAME_LIMIT]);
		else
	#endif
			inRetVal = ECC_FileRemove_Lib(bFileName);

#endif

	return inRetVal;
}

int ECC_FileRename(BYTE* bFileName1,BYTE* bFileName2)
{
int inRetVal = 0;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API

	inRetVal = rename((char *)bFileName1,(char *)bFileName2);

	if (inRetVal != 0)
		log_msg(LOG_LEVEL_ERROR,"ECC_FileRename 0,%s:%s (%d)",bFileName1,bFileName2, inRetVal);

		
#elif READER_MANUFACTURERS==WINDOWS_CE_API

	memset(wBuf1,0x00,sizeof(wBuf1));
	MultiByteToWideChar(CP_UTF8, 0, (char *)bFileName1, 100, wBuf1, 100);
	memset(wBuf2,0x00,sizeof(wBuf2));
	MultiByteToWideChar(CP_UTF8, 0, (char *)bFileName2, 100, wBuf2, 100);
	inRetVal = MoveFile(wBuf1,wBuf2);
#else
BYTE *bFileData;
int inFileLen = 0;

	log_msg(LOG_LEVEL_FLOW,"ECC_FileRename 0,%s:%s",bFileName1,bFileName2);

	inRetVal = inFileGetSize((char *)bFileName1,FALSE);
	log_msg(LOG_LEVEL_FLOW,"ECC_FileRename 1,%d",inRetVal);
	if(inRetVal <= 0)
	{
		log_msg(LOG_LEVEL_FLOW,"ECC_FileRename Fail 1:%s(%d)",bFileName1,inRetVal);
		return SUCCESS;
	}

	inFileLen = inRetVal;

	inRetVal = inFileGetSize((char *)bFileName2,FALSE);
	log_msg(LOG_LEVEL_FLOW,"ECC_FileRename 2,%d",inRetVal);
	if(inRetVal > 0)
		inFileDelete((char *)bFileName2,FALSE);

	bFileData = (BYTE *)ECC_calloc(1,inFileLen);

	inRetVal = inFileRead((char *)bFileName1,bFileData,inFileLen,inFileLen);
	log_msg(LOG_LEVEL_FLOW,"ECC_FileRename 3,%d",inRetVal);
	if(inRetVal < 0)
	//if(inRetVal <= 0)
	{
		ECC_free(bFileData);
		log_msg(LOG_LEVEL_FLOW,"ECC_FileRename Fail 2:%s(%d)",bFileName1,inRetVal);
		return SUCCESS;
	}

	inFileWrite((char *)bFileName2,bFileData,inFileLen);
	inFileDelete((char *)bFileName1,FALSE);
	ECC_free(bFileData);
	log_msg(LOG_LEVEL_FLOW,"ECC_FileRename 4,%d",inFileLen);

#endif

	return inRetVal;
}

int ECC_FileCopy(BYTE* bFileName1,BYTE* bFileName2)
{
int inRetVal = 0,inFileLen = 0;

#if READER_MANUFACTURERS==WINDOWS_API// || READER_MANUFACTURERS==ANDROID_API

	inRetVal = CopyFile((char *)bFileName1,(char *)bFileName2,FALSE);

#elif READER_MANUFACTURERS==WINDOWS_CE_API

	memset(wBuf1,0x00,sizeof(wBuf1));
	MultiByteToWideChar(CP_UTF8, 0, (char *)bFileName1, 100, wBuf1, 100);
	memset(wBuf2,0x00,sizeof(wBuf2));
	MultiByteToWideChar(CP_UTF8, 0, (char *)bFileName2, 100, wBuf2, 100);
	inRetVal = CopyFile(wBuf1,wBuf2);

#else
BYTE *bFileData;

	log_msg(LOG_LEVEL_FLOW,"ECC_FileCopy 0,%s:%s",bFileName1,bFileName2);

//#if READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
	inRetVal = inFileGetSize((char *)bFileName1,FALSE);
//#else
	//inRetVal = inFileGetSize((char *)bFileName1,TRUE);
//#endif
	log_msg(LOG_LEVEL_FLOW,"ECC_FileCopy 1,%d",inRetVal);
	if(inRetVal <= 0)
	{
		log_msg(LOG_LEVEL_FLOW,"ECC_FileCopy Fail 1:%s(%d)",bFileName1,inRetVal);
		return SUCCESS;
	}

	inFileLen = inRetVal;

//#if READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
		inRetVal = inFileGetSize((char *)bFileName2,FALSE);
//#else
//		inRetVal = inFileGetSize((char *)bFileName2,TRUE);
//#endif

	log_msg(LOG_LEVEL_FLOW,"ECC_FileCopy 2,%d",inRetVal);
	if(inRetVal > 0)
		inFileDelete((char *)bFileName2,FALSE);

	bFileData = (BYTE *)ECC_calloc(1,inFileLen);

	inRetVal = inFileRead((char *)bFileName1,bFileData,inFileLen,inFileLen);
	log_msg(LOG_LEVEL_FLOW,"ECC_FileCopy 3,%d",inRetVal);
	if(inRetVal < 0)
	//if(inRetVal <= 0)
	{
		ECC_free(bFileData);
		log_msg(LOG_LEVEL_FLOW,"ECC_FileCopy Fail 2:%s(%d)",bFileName1,inRetVal);
		return SUCCESS;
	}

	inFileWrite((char *)bFileName2,bFileData,inFileLen);
	//inFileDelete((char *)bFileName1,FALSE);
	ECC_free(bFileData);
	log_msg(LOG_LEVEL_FLOW,"ECC_FileCopy 4,%d",inFileLen);

#endif

	return inRetVal;
}

int ECC_CreateDirectory_Lib(char *bFileName)
{
int inRetVal = 0;

#if READER_MANUFACTURERS==WINDOWS_API

	mkdir(bFileName);

#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API

	mkdir(bFileName,0777);

#elif READER_MANUFACTURERS==WINDOWS_CE_API

	memset(wBuf1,0x00,sizeof(wBuf1));
	MultiByteToWideChar(CP_UTF8, 0, (char *)bFileName, 100, wBuf1, 100);
	CreateDirectory(wBuf1, NULL);

#else

#endif

	return inRetVal;
}

int inFileDelete(char *bFileName,char chPathFlag)
{
char chAllFileName[FULL_FILE_NAME_MAX];
int inRetVal = 0;

	memset(chAllFileName,0x00,sizeof(chAllFileName));
	if(chPathFlag == FALSE)//相對路徑
	{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
	sprintf(chAllFileName,"%s\\%s",gCurrentFolder,bFileName);
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
	sprintf(chAllFileName,"%s/%s",gCurrentFolder,bFileName);
#else
	sprintf(chAllFileName,"%s",bFileName);
#endif
	}
	else//絕對路徑
		memcpy(chAllFileName,bFileName,strlen(bFileName));

	inRetVal = ECC_FileRemove((BYTE *)chAllFileName);
	//inRetVal = remove(chAllFileName);

	return inRetVal;
}

int inFileRename(char *bOldName,char *NewName)
{
char chAllFileName1[FULL_FILE_NAME_MAX],chAllFileName2[FULL_FILE_NAME_MAX];
int inRetVal = 0;

	log_msg(LOG_LEVEL_FLOW,"inFileRename 0,%s:%s",bOldName,NewName);
	memset(chAllFileName1,0x00,sizeof(chAllFileName1));
	memset(chAllFileName2,0x00,sizeof(chAllFileName2));
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
	sprintf(chAllFileName1,"%s\\%s",gCurrentFolder,bOldName);
	sprintf(chAllFileName2,"%s\\%s",gCurrentFolder,NewName);
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
	sprintf(chAllFileName1,"%s/%s",gCurrentFolder,bOldName);
	sprintf(chAllFileName2,"%s/%s",gCurrentFolder,NewName);
#else
	sprintf(chAllFileName1,"%s",bOldName);
	sprintf(chAllFileName2,"%s",NewName);
#endif

	inRetVal = ECC_FileRename((BYTE *)chAllFileName1,(BYTE *)chAllFileName2);
	log_msg(LOG_LEVEL_FLOW,"inFileRename 1,%d,%s:%s",inRetVal,chAllFileName1,chAllFileName2);

	return inRetVal;
}

int inFileGetSize(char *bFileName,char chPathFlag)
{
int inRetVal;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
FILE *fp=NULL;
char chAllFileName[FULL_FILE_NAME_MAX];
DWORD err = 0;//錯誤函數回傳值

	memset(chAllFileName,0x00,sizeof(chAllFileName));
	if(chPathFlag == FALSE)//相對路徑
	{
	  #if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
		sprintf(chAllFileName,"%s\\%s",gCurrentFolder,bFileName);
	  #elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
		sprintf(chAllFileName,"%s/%s",gCurrentFolder,bFileName);
	  #endif
	}
	else//絕對路徑
		memcpy(chAllFileName,bFileName,strlen(bFileName));
	fp = fopen(chAllFileName,"rb");
	if(fp == NULL)
	{
	#if READER_MANUFACTURERS!=LINUX_API && READER_MANUFACTURERS!=ANDROID_API
		err = GetLastError();
	#endif

	#if READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
		if(memcmp(bFileName,"outputdata",10))
	#endif
			if(chPathFlag == FALSE)//相對路徑
				log_msg(LOG_LEVEL_ERROR,"inFileGetSize (%s) FILE_OPEN_FAIL:%d(%x),%d",chAllFileName,fp,chPathFlag,err);
		return ICER_ERROR;
		//return FILE_OPEN_FAIL;
	}

	inRetVal = fseek(fp,0,SEEK_END);
	if(inRetVal < SUCCESS)
	{
		fclose(fp);
		log_msg(LOG_LEVEL_ERROR,"inFileGetSize (%s) FILE_SEEK_FAIL:%d(%x)",chAllFileName,inRetVal,chPathFlag);
		return ICER_ERROR;
		//return FILE_SEEK_FAIL;
	}

	inRetVal = ftell(fp);
	if(inRetVal < SUCCESS)
	{
		fclose(fp);
		log_msg(LOG_LEVEL_ERROR,"inFileGetSize (%s) FILE_TELL_FAIL:%d(%x)",chAllFileName,inRetVal,chPathFlag);
		return ICER_ERROR;
		//return FILE_TELL_FAIL;
	}

	fclose(fp);

	return inRetVal;
#else

	inRetVal = ECC_FileSize((BYTE *)bFileName);

	return inRetVal;

#endif
}

int inFileRead(char *bFileName,BYTE *bFileData,int inFileDataLen,int inFileReadCnt)
{
int inRetVal;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
FILE *fp=NULL;
char chAllFileName[FULL_FILE_NAME_MAX];

	memset(chAllFileName,0x00,sizeof(chAllFileName));
	#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
	sprintf(chAllFileName,"%s\\%s",gCurrentFolder,bFileName);
	#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
	sprintf(chAllFileName,"%s/%s",gCurrentFolder,bFileName);
	#endif

	memset(bFileData,0x00,inFileDataLen);

	fp = fopen(chAllFileName,"r+b");
	if(fp == NULL)
	{
		log_msg(LOG_LEVEL_ERROR,"inFileRead (%s) FILE_OPEN_FAIL:%d",chAllFileName,fp);
		return ICER_ERROR;
	}

	inRetVal = fread(bFileData,inFileReadCnt,1,fp);

	if(!memcmp(bFileName,FILE_TMP,strlen(FILE_TMP)))
	{
		BYTE bBuf[sizeof(STRUCT_TMP_DATA) * 2 + 1];

		memset(bBuf,0x00,sizeof(bBuf));
		fnUnPack((BYTE *)bFileData, inFileReadCnt, bBuf);
		log_msg(LOG_LEVEL_FLOW,"inFileRead FILE_TMP: %s",bBuf);
	}

	if(inRetVal < 1)
		log_msg(LOG_LEVEL_ERROR,"inFileRead (%s) FILE_READ_FAIL:%d",chAllFileName,inRetVal);

	fclose(fp);
	inRetVal = SUCCESS;
#else
int fp;

	fp = ECC_FileOpen((BYTE *)bFileName,(BYTE *)"r+b");
	if(fp < 0)
	{
		log_msg(LOG_LEVEL_ERROR,"inFileRead (%s) FILE_OPEN_FAIL:%d",bFileName,fp);
		return ICER_ERROR;
	}

	inRetVal = ECC_FileRead(fp,bFileData,inFileReadCnt);
	if(inRetVal < 1)
		log_msg(LOG_LEVEL_ERROR,"inFileRead (%s) FILE_READ_FAIL:%d",bFileName,inRetVal);

	ECC_FileClose(fp);
	inRetVal = SUCCESS;

#endif

	return inRetVal;
}

#if READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
int inFileRead2(char *bFileName,BYTE *bFileData,int inFileDataLen,int inFileReadCnt,int inOffset)
{
FILE *fp=NULL;
int inRetVal;
char chAllFileName[FULL_FILE_NAME_MAX];

	memset(chAllFileName,0x00,sizeof(chAllFileName));
	//sprintf(chAllFileName,"%s",bFileName);
	sprintf(chAllFileName,"%s/%s",gCurrentFolder,bFileName);

	memset(bFileData,0x00,inFileDataLen);

	fp = fopen(chAllFileName,"r+b");
	if(fp == NULL)
	{
		log_msg(LOG_LEVEL_ERROR,"inFileRead2 (%s) FILE_OPEN_FAIL:%d",chAllFileName,fp);
		return ICER_ERROR;
		//return FILE_OPEN_FAIL;
	}

#if 0
	fseek(fp,inOffset,SEEK_SET);
	inRetVal = fread(bFileData,inFileReadCnt,1,fp);
#else
	fseek(fp,inOffset,SEEK_SET);
	inRetVal = fread(bFileData,inFileReadCnt,1,fp);
	if(inRetVal < 1)
	//if(inRetVal < inFileReadCnt)
	{
		//fclose(fp);
		log_msg(LOG_LEVEL_ERROR,"inFileRead2 (%s) FILE_READ_FAIL:%d",chAllFileName,inRetVal);
		//return FILE_READ_FAIL;
	}
#endif

	fclose(fp);

	return SUCCESS;
}
#endif

int inFileWrite(char *bFileName,BYTE *bFileData,int inFileWriteCnt)
{
int inRetVal;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
FILE *fp=NULL;
char chAllFileName[FULL_FILE_NAME_MAX];

	memset(chAllFileName,0x00,sizeof(chAllFileName));
	#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
	sprintf(chAllFileName,"%s\\%s",gCurrentFolder,bFileName);
	#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
	sprintf(chAllFileName,"%s/%s",gCurrentFolder,bFileName);
	#endif

	if(!memcmp(bFileName,FILE_TMP,strlen(FILE_TMP)))
	{
		BYTE bBuf[sizeof(STRUCT_TMP_DATA) * 2 + 1];

		memset(bBuf,0x00,sizeof(bBuf));
		fnUnPack((BYTE *)bFileData, inFileWriteCnt, bBuf);
		log_msg(LOG_LEVEL_FLOW,"inFileWrite FILE_TMP: %s",bBuf);
	}

	fp = fopen(chAllFileName,"w+b");
	if(fp == NULL)
	{
		log_msg(LOG_LEVEL_ERROR,"inFileWrite (%s) FILE_OPEN_FAIL:%d",chAllFileName,fp);
		return ICER_ERROR;
		//return FILE_OPEN_FAIL;
	}

	inRetVal = fwrite(bFileData,inFileWriteCnt,1,fp);
	if(inRetVal < 1)
	{
		fclose(fp);
		log_msg(LOG_LEVEL_ERROR,"inFileWrite (%s) FILE_WRITE_FAIL:%d",chAllFileName,inRetVal);
		return ICER_ERROR;
	}

	fclose(fp);
#else
int fp;

	/*#if READER_MANUFACTURERS==NE_PAXSXX
		fp = ECC_FileOpen((BYTE *)bFileName,(BYTE *)"ab+");
	#elif READER_MANUFACTURERS==NE_SYMLINK_API*/
	#if READER_MANUFACTURERS==NE_SYMLINK_API
		fp = ECC_FileOpen((BYTE *)bFileName, (BYTE *)"ab+");
		ECC_FileSeek_Lib(fp, 0, SEEK_SET);
	#else
		fp = ECC_FileOpen((BYTE *)bFileName,(BYTE *)"w+b");
	#endif

	if(fp < 0)
	{
		log_msg(LOG_LEVEL_ERROR,"inFileWrite (%s) FILE_OPEN_FAIL:%d",bFileName,fp);
		return ICER_ERROR;
	}

	inRetVal = ECC_FileWrite(fp,bFileData,inFileWriteCnt);
	if(inRetVal < 1)
	{
		ECC_FileClose(fp);
		log_msg(LOG_LEVEL_ERROR,"inFileWrite (%s) FILE_WRITE_FAIL:%d",bFileName,inRetVal);
		return ICER_ERROR;
	}

	ECC_FileClose(fp);

#endif

	return inRetVal;
}

int inFileAppend(char *bFileName,BYTE *bFileData,int inFileWriteCnt)
{
int inRetVal;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
FILE *fp=NULL;
char chAllFileName[FULL_FILE_NAME_MAX];

	memset(chAllFileName,0x00,sizeof(chAllFileName));
	#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
	sprintf(chAllFileName,"%s\\%s",gCurrentFolder,bFileName);
	#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
	sprintf(chAllFileName,"%s/%s",gCurrentFolder,bFileName);
	#endif

	fp = fopen(chAllFileName,"a+b");
	if(fp == NULL)
	{
		log_msg(LOG_LEVEL_ERROR,"inFileAppend (%s) FILE_OPEN_FAIL:%d",chAllFileName,fp);
		return ICER_ERROR;
	}

	inRetVal = fwrite(bFileData,inFileWriteCnt,1,fp);
		if(inRetVal < 1)		
		{
			fclose(fp);
			log_msg(LOG_LEVEL_ERROR,"inFileAppend (%s) FILE_WRITE_FAIL:%d",chAllFileName,inRetVal);
			return ICER_ERROR;
		}
	

	fclose(fp);

#else
int fp;

	fp = ECC_FileOpen((BYTE *)bFileName,(BYTE *)"a+b");
#if READER_MANUFACTURERS==CS_PAXSXX || READER_MANUFACTURERS==NE_SYMLINK_API || defined(FLASH_SYSTEM)
	if(fp < 0)
#else
	if(fp == (int)NULL)
#endif
	{
		log_msg(LOG_LEVEL_ERROR,"inFileAppend (%s) FILE_OPEN_FAIL:%d",bFileName,fp);
		return ICER_ERROR;
	}

	ECC_FileSeek(fp,0,SEEK_END);

	inRetVal = ECC_FileWrite(fp,bFileData,inFileWriteCnt);
	if(inRetVal < 1)
	{
		ECC_FileClose(fp);
		log_msg(LOG_LEVEL_ERROR,"inFileAppend (%s) FILE_WRITE_FAIL:%d",bFileName,inRetVal);
		return ICER_ERROR;
	}

	ECC_FileClose(fp);

#endif

	return inRetVal;
}

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
int inFileModify(FILE *FileHandle,BOOL bOpened,char *bFileName,BYTE *bFileData,int inFileWriteCnt,int inFileSeek)
#else
int inFileModify(int FileHandle,BOOL bOpened,char *bFileName,BYTE *bFileData,int inFileWriteCnt,int inFileSeek)
#endif
{
int inRetVal;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
FILE *fp=NULL;
char chAllFileName[FULL_FILE_NAME_MAX];

	memset(chAllFileName,0x00,sizeof(chAllFileName));
	#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
	sprintf(chAllFileName,"%s\\%s",gCurrentFolder,bFileName);
	#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
	sprintf(chAllFileName,"%s/%s",gCurrentFolder,bFileName);
	#endif

	if(bOpened == TRUE)//檔案在外面已開就不要再開了!!
		fp = FileHandle;
	else
		fp = fopen(chAllFileName,"r+b");
	if(fp == NULL)
	{
		log_msg(LOG_LEVEL_ERROR,"inFileModify (%s) FILE_OPEN_FAIL:%d",chAllFileName,fp);
		return ICER_ERROR;
	}

	fseek(fp,inFileSeek,SEEK_SET);

	inRetVal = fwrite(bFileData,inFileWriteCnt,1,fp);
	if(inRetVal < 1)
	{
		if(bOpened == FALSE)//檔案在外面已開,不要關!!
			fclose(fp);
		log_msg(LOG_LEVEL_ERROR,"inFileModify (%s) FILE_WRITE_FAIL:%d",chAllFileName,inRetVal);
		return ICER_ERROR;
	}

	if(bOpened == FALSE)//檔案在外面已開,不要關!!
		fclose(fp);

#else
int fp;

	if(bOpened == TRUE)//檔案在外面已開就不要再開了!!
		fp = FileHandle;
	else
		fp = ECC_FileOpen((BYTE *)bFileName,(BYTE *)"r+b");

#if READER_MANUFACTURERS==CS_PAXSXX || READER_MANUFACTURERS==NE_SYMLINK_API || defined(FLASH_SYSTEM)
	if(fp < 0)
#else
	if(fp == (int)NULL)
#endif
	{
		log_msg(LOG_LEVEL_ERROR,"inFileModify (%s) FILE_OPEN_FAIL:%d",bFileName,fp);
		return ICER_ERROR;
	}

	ECC_FileSeek(fp,inFileSeek,SEEK_SET);

	inRetVal = ECC_FileWrite(fp,bFileData,inFileWriteCnt);
	if(inRetVal < 1)
	{
		if(bOpened == FALSE)//檔案在外面已開,不要關!!
			ECC_FileClose(fp);
		log_msg(LOG_LEVEL_ERROR,"inFileModify (%s) FILE_WRITE_FAIL:%d",bFileName,inRetVal);
		return ICER_ERROR;
	}

	if(bOpened == FALSE)//檔案在外面已開,不要關!!
		ECC_FileClose(fp);

#endif

	return SUCCESS;
}

int inFileCopy(char *bOldName,char *NewName)
{
char chAllFileName1[FULL_FILE_NAME_MAX],chAllFileName2[FULL_FILE_NAME_MAX];
int inRetVal = 0;

	log_msg(LOG_LEVEL_FLOW,"inFileCopy 0,%s:%s",bOldName,NewName);
	memset(chAllFileName1,0x00,sizeof(chAllFileName1));
	memset(chAllFileName2,0x00,sizeof(chAllFileName2));
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
	sprintf(chAllFileName1,"%s\\%s",gCurrentFolder,bOldName);
	sprintf(chAllFileName2,"%s\\%s",gCurrentFolder,NewName);
/*#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
	sprintf(chAllFileName1,"%s/%s",gCurrentFolder,bOldName);
	sprintf(chAllFileName2,"%s/%s",gCurrentFolder,NewName);*/
#else
	sprintf(chAllFileName1,"%s",bOldName);
	sprintf(chAllFileName2,"%s",NewName);
#endif

	inRetVal = ECC_FileCopy((BYTE *)chAllFileName1,(BYTE *)chAllFileName2);
	log_msg(LOG_LEVEL_FLOW,"inFileCopy 1,%d,%s:%s",inRetVal,chAllFileName1,chAllFileName2);

	return inRetVal;
}

short ECC_TCPIPConnect(BYTE* bIP,unsigned short usPort,unsigned short usTimeOut)
{
int inRetVal = 0;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
#else

	int inOrgHandle = ginTrans_ClientFd_inLib;
	
	inRetVal = ECC_TCPIPConnect_Lib(bIP,usPort,usTimeOut);
	
	if (inOrgHandle == -1 && ginTrans_ClientFd_inLib >= 0)
	{
		inFile_Open_File_Cnt_Increase();
	}

#endif

	return inRetVal;
}

short ECC_TCPIPSend(BYTE* bSendData,unsigned short usSendLen,unsigned short usTimeOut)
{
int inRetVal = 0;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
#else

	inRetVal = ECC_TCPIPSend_Lib(bSendData,usSendLen,usTimeOut);

#endif

	return inRetVal;
}

short ECC_TCPIPRecv(BYTE* bRecvData,unsigned short* usRecvLen,unsigned short usTimeOut)
{
int inRetVal = 0;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
#else

	inRetVal = ECC_TCPIPRecv_Lib(bRecvData,usRecvLen,usTimeOut);
	log_msg(LOG_LEVEL_FLOW,"ECC_TCPIPRecv_Lib 1 inRetVal = %d,usRecvLen = %d[%02x%02x%02x%02x%02x%02x]",inRetVal,*usRecvLen,bRecvData[0],bRecvData[1],bRecvData[2],bRecvData[3],bRecvData[4],bRecvData[5]);

#endif

	return inRetVal;
}

short ECC_TCPIPClose(void)
{
int inRetVal = 0;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
#else

	int inOrgHandle = ginTrans_ClientFd_inLib;

	inRetVal = ECC_TCPIPClose_Lib();
	
	if (inOrgHandle >= 0 && ginTrans_ClientFd_inLib == -1)
	{
		inFile_Open_File_Cnt_Decrease();
	}

#endif

	return inRetVal;
}

short ECC_UnixToDateTime(unsigned char *bUnixTime,unsigned char *bDate)
{
int inRetVal = 0;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
#else

	inRetVal = ECC_UnixToDateTime_Lib(bUnixTime,bDate);
	//log_msg(LOG_LEVEL_ERROR,"ECC_UnixToDateTime in[%02X%02X%02X%02X],out[%s]",bUnixTime[0],bUnixTime[1],bUnixTime[2],bUnixTime[3],bDate);

#endif

	return inRetVal;
}

short ECC_DateTimeToUnix(unsigned char *bDate,unsigned char *bUnixTime)
{
int inRetVal = 0;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
#else

	inRetVal = ECC_DateTimeToUnix_Lib(bDate,bUnixTime);

#endif

	return inRetVal;
}

short ECC_INT64ToASCII(unsigned char *bData,unsigned char *bAsciiData)
{
int inRetVal = 0;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
#else

	inRetVal = ECC_INT64ToASCII_Lib(bData,bAsciiData);
	//log_msg(LOG_LEVEL_ERROR,"ECC_INT64ToASCII in[%02X%02X%02X%02X],out[%s]",bData[0],bData[1],bData[2],bData[3],bAsciiData);

#endif

	return inRetVal;
}

short ECC_UINT64ToASCII(unsigned char *bData,unsigned char *bAsciiData)
{
int inRetVal = 0;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
#else

	inRetVal = ECC_UINT64ToASCII_Lib(bData,bAsciiData);
	log_msg(LOG_LEVEL_FLOW,"ECC_UINT64ToASCII in[%02X%02X%02X%02X],out[%s]",bData[0],bData[1],bData[2],bData[3],bAsciiData);

#endif

	return inRetVal;
}

short ECC_ASCIIToINT64(unsigned char *bAsciiData,unsigned short usLen,unsigned char *bData)
{
int inRetVal = 0;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
#else

	inRetVal = ECC_ASCIIToINT64_Lib(bAsciiData,usLen,bData);

#endif

	return inRetVal;
}

short ECC_ASCIIToUINT64(unsigned char *bAsciiData,unsigned short usLen,unsigned char *bData)
{
int inRetVal = 0;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API

UINT64 ui64Data = 0L;

	ui64Data = _atoi64((char *)bAsciiData);
	memcpy(bData,&ui64Data,sizeof(ui64Data));

#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API

UINT64 ui64Data = 0L;

	ui64Data = strtoull((char *)bAsciiData,NULL,10);
	memcpy(bData,&ui64Data,sizeof(ui64Data));

#else

	inRetVal = ECC_ASCIIToUINT64_Lib(bAsciiData,usLen,bData);

#endif

	return inRetVal;
}

short ECC_SSLConnect(BYTE* bIP,unsigned short usPort,unsigned short usTimeOut)
{
int inRetVal = 0;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
#else

	int inOrgHandle = ginTrans_ClientFd_inLib;
	
	log_msg(LOG_LEVEL_FLOW,"ECC_SSLConnect 0 bIP = %s,usPort = %d",bIP,usPort);
	inRetVal = ECC_SSLConnect_Lib(bIP,usPort,usTimeOut);
	log_msg(LOG_LEVEL_FLOW,"ECC_SSLConnect 1 inRetVal = %d,bIP = %s,usPort = %d",inRetVal,bIP,usPort);
	
	if (inOrgHandle == -1 && ginTrans_ClientFd_inLib >= 0)
	{
		inFile_Open_File_Cnt_Increase();
	}

#endif

	return inRetVal;
}

short ECC_SSLSend(BYTE* bSendData,unsigned short usSendLen,unsigned short usTimeOut)
{
int inRetVal = 0;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
#else

	inRetVal = ECC_SSLSend_Lib(bSendData,usSendLen,usTimeOut);

#endif

	return inRetVal;
}

short ECC_SSLRecv(BYTE* bRecvData,unsigned short* usRecvLen,unsigned short usTimeOut)
{
int inRetVal = 0;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
#else
unsigned short usLen = *usRecvLen;

	inRetVal = ECC_SSLRecv_Lib(bRecvData,usRecvLen,usTimeOut);
	log_msg(LOG_LEVEL_FLOW,"ECC_SSLRecv 1 inRetVal = %d,usRecvLen = %d,usLen = %d",inRetVal,*usRecvLen,usLen);

#endif

	return inRetVal;
}

short ECC_SSLClose(void)
{
int inRetVal = 0;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
#else
	int inOrgHandle = ginTrans_ClientFd_inLib;
	
	inRetVal = ECC_SSLClose_Lib();
	
	if (inOrgHandle >= 0 && ginTrans_ClientFd_inLib == -1)
	{
		inFile_Open_File_Cnt_Decrease();
	}
	
#endif

	return inRetVal;
}

short ECC_NCCC_Encryption_TSAM(BYTE *bData, unsigned int *dataLen, BYTE fieldNo, BYTE *keyField37, BYTE *checksumField57, BYTE *keyIndex)
{
	int inRetVal = 0;
#ifdef NCCC_ENCRYPTION

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
#else
	inRetVal = ECC_NCCC_Encryption_TSAM_Lib(bData, dataLen, fieldNo, keyField37, checksumField57, keyIndex);
#endif

#endif

	return inRetVal;
}

short ECC_NCCC_MAC(unsigned int msgType, BYTE *bData, unsigned int idxField3, unsigned idxField4, unsigned int idxField11, unsigned int idxField35, unsigned int idxField59, unsigned int idxField63, BYTE *MACData)
{
	int inRetVal = 0;
#ifdef NCCC_ENCRYPTION

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
#else
	inRetVal = ECC_NCCC_MAC_Lib(msgType, bData, idxField3, idxField4, idxField11, idxField35, idxField59, idxField63, MACData);
#endif

#endif

	return inRetVal;
}

//#endif

#ifdef	FLASH_SYSTEM
short ICER_WriteDataFlash(unsigned long ulAddress, unsigned char *bpData, unsigned short usDataLen)
{
int inRetVal = 0;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
#else

	inRetVal = ECC_WriteDataFlash_Lib(ulAddress,bpData,usDataLen);

#endif

	return inRetVal;
}

short ICER_ReadDataFlash(unsigned long ulAddress, unsigned char *bpData, unsigned short usDataLen)
{
int inRetVal = 0;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
#else

	inRetVal = ECC_ReadDataFlash_Lib(ulAddress,bpData,usDataLen);

#endif

	return inRetVal;
}


BOOL fCheckStorageMode(BYTE* bFileName)
{
int i,j = 0,k = 0;

	inStorageType = 0;
	fStorageMode = STORAGE_FILE;

	/*{
		BYTE bBuf[1000],bTmp[1000];

		memset(bBuf,0x00,sizeof(bBuf));
		memset(bTmp,0x00,sizeof(bTmp));
		memcpy(bTmp,st_FlashPar[0].bFileName,sizeof(st_FlashPar));
		sprintf((char *)bBuf,"fCheckStorageMode %d,",sizeof(st_FlashPar));
		for(i=0;i<sizeof(st_FlashPar);i++)
		{
			if(bTmp[i] >= 0x20 && bTmp[i] <= 0x7F)
				bBuf[strlen((char *)bBuf)] = bTmp[i];
			else
				sprintf((char *)&bBuf[strlen((char *)bBuf)],"[%02x]",bTmp[i]);
		}
		vdICERFlowDebug((char *)bBuf);
	}*/

	for(i=0;i<MAX_FILE_CNT;i++)
	{
		j = strlen((char *)(st_FlashPar[i].bFileName));
		k = strlen((char *)bFileName);
		//log_msg(LOG_LEVEL_FLOW,"fCheckStorageMode i=%d,%s:%s,%d:%d",i,bFileName,st_FlashPar[i].bFileName,j,k);
		if(!memcmp(bFileName,st_FlashPar[i].bFileName,j) && (j == k))
		{
			inStorageType = i;
			fStorageMode = STORAGE_FLASH;
			if(srFlashData.ulFlashEndDataAddr[i] != st_FlashPar[i].ulStartAddress)//open後,指標預設到起始位置
			{
				log_msg(LOG_LEVEL_FLOW,"fCheckStorageMode i=%d,%ld,%ld",i,srFlashData.ulFlashEndDataAddr[i],st_FlashPar[i].ulStartAddress);
				srFlashData.ulFlashEndDataAddr[i] = st_FlashPar[i].ulStartAddress;
				//inWriteFlasfData();//移到inFinalizer()最後面執行!!
			}

			return fStorageMode;
		}
	}

	return fStorageMode;
}

#endif

int inReadFlasfData(void)
{
//	int inRetVal = 0;
//	BOOL fFlag = FALSE;

#ifdef	FLASH_SYSTEM
	memset((char *)&srFlashData,0x00,sizeof(srFlashData));
	//ICER_WriteDataFlash(FLASH_PAR,(BYTE *)&srFlashData,sizeof(STRUCT_FLASH_DATA));

	inRetVal = ICER_ReadDataFlash(FLASH_PAR,(BYTE *)&srFlashData,sizeof(STRUCT_FLASH_DATA));
	if(inRetVal == SUCCESS)
	{
		int i;

		for(i=0;i<MAX_FILE_CNT;i++)
		{
			//log_msg(LOG_LEVEL_ERROR,"inReadFlasfData 0,%ld,%ld,%ld",srFlashData.ulFlashEndDataAddr[i],st_FlashPar[i].ulStartAddress,st_FlashPar[i].ulEndAddress);
			if(srFlashData.ulFlashEndDataAddr[i] < st_FlashPar[i].ulStartAddress || srFlashData.ulFlashEndDataAddr[i] > st_FlashPar[i].ulEndAddress)
			{
				log_msg(LOG_LEVEL_FLOW,"inReadFlasfData 1,%ld,%ld,%ld",srFlashData.ulFlashEndDataAddr[i],st_FlashPar[i].ulStartAddress,st_FlashPar[i].ulEndAddress);
				srFlashData.ulFlashEndDataAddr[i] = st_FlashPar[i].ulStartAddress;
				fFlag = TRUE;
			}

		}

		//if(fFlag == TRUE)
			//inWriteFlasfData();//移到inFinalizer()最後面執行!!
	}
#endif

	return SUCCESS;
}

int inWriteFlasfData(void)
{
//	int inRetVal = 0;

#ifdef	FLASH_SYSTEM
	inRetVal = ICER_WriteDataFlash(FLASH_PAR,(BYTE *)&srFlashData,sizeof(STRUCT_FLASH_DATA));
#endif

	return SUCCESS;
}

#ifdef	FLASH_SYSTEM

int inGetICERLogSize(unsigned long *ulOutSize)
{
	*ulOutSize = srFlashData.ulFlashDataSize[STORAGE_LOG];
	return SUCCESS;
}

unsigned long inGetICERLog(unsigned long ulStartAdress,BYTE *bOutLogData,unsigned long ulOutSize,BOOL fClearFlag)
{
unsigned long ulLogSize = 0L;

	ulLogSize = ICER_ReadDataFlash(FLASH_START_LOG + ulStartAdress,bOutLogData,ulOutSize);
	if(fClearFlag)
	{
		srFlashData.ulFlashDataSize[STORAGE_LOG] = 0L;
		srFlashData.ulFlashEndDataAddr[STORAGE_LOG] = st_FlashPar[STORAGE_LOG].ulStartAddress;
	}

	return ulLogSize;
}

#endif

/*short ECC_AES(unsigned char bMode,unsigned char* bpKey,unsigned char bKeyType,unsigned char* bpInData,unsigned short usInLen, unsigned char* bpOutData)
{
int inRetVal = 0;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
#else


	inRetVal = ECC_AES_Lib(bMode,bpKey,bKeyType,bpInData,usInLen,bpOutData);
	//log_msg(LOG_LEVEL_ERROR,"ECC_AES in[%02X%02X%02X%02X],out[%s]",bData[0],bData[1],bData[2],bData[3],bAsciiData);

#endif

	return inRetVal;
}*/

BYTE *ECC_calloc(unsigned long ulCnt,unsigned long ulSize)
{
BYTE *bAdr = NULL;
//	unsigned long ulHeapBef = ulHeapSize;

	bAdr = (BYTE *)calloc(ulCnt,ulSize);

#if READER_MANUFACTURERS==NE_SYMLINK_API
	ulHeapSize = BSP_heap_allocated();
#endif
	//log_msg(LOG_LEVEL_ERROR,"ECC_calloc Test ,%ld,%x,%ld,%ld,%ld,%ld,%ld",ulHeapSize - ulHeapBef,bAdr,ulHeapBef,ulHeapSize,ulCnt,ulSize,ulCnt*ulSize);

	return bAdr;
}

void ECC_free(BYTE *bFreeAdr)
{
//	unsigned long ulHeapBef = ulHeapSize;
//	BYTE *bAdr = bFreeAdr,bBuf[100];
//	int i;

	/*memset(bBuf,0x00,sizeof(bBuf));
	for(i=0;i<10;i++)
	{
		if(bFreeAdr[i] >= 0x20 && bFreeAdr[i] <= 0x7F)
			memcpy(&bBuf[strlen((char *)bBuf)],&bFreeAdr[i],1);
		else
			sprintf((char *)&bBuf[strlen((char *)bBuf)],"[%02x]",bFreeAdr[i]);
	}*/

	free(bFreeAdr);

#if READER_MANUFACTURERS==NE_SYMLINK_API
	ulHeapSize = BSP_heap_allocated();
#endif
	//log_msg(LOG_LEVEL_ERROR,"ECC_free Test ,%ld,%x,%ld,%ld,%s",ulHeapBef - ulHeapSize,bAdr,ulHeapBef,ulHeapSize,bBuf);

	return ;
}

