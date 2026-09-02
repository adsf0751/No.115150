#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "Function.h"
#include "Utility.h"
#include "File.h"

/* Debug使用 extern */
int		ginFileOpenCnt = 0;
extern int	ginDebug;

/*
Function	:inFILE_Open
Date&Time	:2015/8/5 下午 5:12
Describe	:輸入檔案名稱及Handle，開啟檔案，判斷檔案不存在直接Return不建立新檔，檔名(含附檔名)長度最多15個字
*/
int inFILE_Open(unsigned long *ulFileHandle, unsigned char *uszFileName)
{
	char		szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];
        unsigned long   ulFileSize = 0;
	unsigned short  usRetVal;
        
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFILE_Open() Start!");
        }

        if (ulFileHandle == NULL || strlen((char *)uszFileName) == 0)
        {
                return (VS_ERROR);
        }

	/* 移除檔名長度錯誤限制 by Russell 2018/12/24 下午 6:12 */

        /* 取得檔案大小，得到回傳值來判斷檔案是否存在 */
        usRetVal = CTOS_FileGetSize(uszFileName, &ulFileSize);

        /* 判斷檔案不存在，直接Return Error不建立新檔 */
        if (usRetVal == d_FS_FILE_NOT_FOUND)
        {
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "%s不存在 Handle:%u", uszFileName, (int)ulFileHandle);
                        inLogPrintf(AT, szErrorMsg);
                }
                vdUtility_SYSFIN_LogMessage(AT, "File open Failed F:%s", uszFileName);
                return (VS_ERROR);
        }

        /* 用輸入的檔名開啟檔案 */
	usRetVal = CTOS_FileOpen(uszFileName, d_STORAGE_FLASH, ulFileHandle);

	if (usRetVal != d_OK)
	{
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "開檔失敗%s Handle:%u", uszFileName, (int)ulFileHandle);
                        inLogPrintf(AT, szErrorMsg);
			
			memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "usRetVal : 0x%x", usRetVal);
                        inLogPrintf(AT, szErrorMsg);
                }
                vdUtility_SYSFIN_LogMessage(AT, "File open Failed F:%s Handle:%u usRetVal : 0x%x", uszFileName, (int)ulFileHandle, usRetVal);
		return (VS_ERROR);
	}
        else
        {
		if (ginDebug == VS_TRUE)
		{
			memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
			sprintf(szErrorMsg, "開檔成功%s  Handle:%u", uszFileName, (int)ulFileHandle);
			inLogPrintf(AT, szErrorMsg);
		}
		inFile_Open_File_Cnt_Increase();
        }

        if (ginDebug == VS_TRUE)
        {
		inLogPrintf(AT, "inFILE_Open() END!");
        }

	return (VS_SUCCESS);
}

/*
Function	:inFILE_OpenReadOnly
Date&Time	:2015/9/16 上午 11:34
Describe	:輸入檔案名稱及Handle，開啟檔案，判斷檔案不存在直接Return不建立新檔
*/
int inFILE_OpenReadOnly(unsigned long *ulFileHandle, unsigned char *uszFileName)
{
	char		szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];
        unsigned long   ulFileSize = 0;
	unsigned short  usRetVal;

        if (ulFileHandle == NULL || strlen((char *)uszFileName) == 0)
        {
                return (VS_ERROR);
        }

//        if (ginDebug == VS_TRUE)
//        {
//                inLogPrintf(AT, "inFILE_OpenReadOnly() Start!");
//        }

        /* 移除檔名長度錯誤限制 by Russell 2018/12/24 下午 6:12 */

        /* 取得檔案大小，得到回傳值來判斷檔案是否存在 */
        usRetVal = CTOS_FileGetSize(uszFileName, &ulFileSize);

        /* 判斷檔案不存在，直接Return Error不建立新檔 */
        if (usRetVal == d_FS_FILE_NOT_FOUND)
        {
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "%s不存在 Handle:%u", uszFileName, (int)ulFileHandle);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

        /* 用輸入的檔名開啟檔案 */
	usRetVal = CTOS_FileOpen(uszFileName, d_STORAGE_FLASH, ulFileHandle);

	if (usRetVal != d_OK)
	{
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "開檔失敗%s  Handle:%u", uszFileName, (int)ulFileHandle);
                        inLogPrintf(AT, szErrorMsg);
                }
		/* 開檔案失敗，還需要再關檔案嗎? */
		/* 不寫成inFILE_Close(ulFileHandle);是因為不直觀? */		
		inFILE_Close(&(*ulFileHandle));
		return (VS_ERROR);
	}
        else
        {
//                if (ginDebug == VS_TRUE)
//                {
//                    memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
//                    sprintf(szErrorMsg, "開檔成功%s  Handle:%u", uszFileName, (int)ulFileHandle);
//                    inLogPrintf(AT, szErrorMsg);
//                }
		inFile_Open_File_Cnt_Increase();
        }

//        if (ginDebug == VS_TRUE)
//        {
//                inLogPrintf(AT, "inFILE_Open() END!");
//        }

	return (VS_SUCCESS);
}

/*
Function	:inFILE_Create
Date&Time	:2015/8/5 下午 5:12
Describe	:輸入檔案名稱及Handle，開啟檔案，判斷檔案不存在建立一個新檔案
*/
int inFILE_Create(unsigned long *ulFileHandle, unsigned char *uszFileName)
{
	char		szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];
        unsigned long   ulFileSize = 0;
	unsigned short  usRetVal;
        
        if (ulFileHandle == NULL || strlen((char *)uszFileName) == 0)
        {
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "null pointer or no FileName");
		}
		
		return (VS_ERROR);
        }

//        if (ginDebug == VS_TRUE)
//        {
//                inLogPrintf(AT, "inFILE_Create() Start!");
//        }

        /* 移除檔名長度錯誤限制 by Russell 2018/12/24 下午 6:12 */

        usRetVal = CTOS_FileGetSize(uszFileName, &ulFileSize);

        /* 判斷檔案不存在，建立新檔案 */
        if (usRetVal == d_FS_FILE_NOT_FOUND)
        {
//                if (ginDebug == VS_TRUE)
//                {
//                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
//                        sprintf(szErrorMsg, "%s檔案不存在 usRetVal : 0x%x", uszFileName, usRetVal);
//                        inLogPrintf(AT, szErrorMsg);
//                }

                usRetVal = CTOS_FileOpen(uszFileName, d_STORAGE_FLASH, ulFileHandle);

                if (usRetVal != d_OK)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "%s開檔失敗 usRetVal : 0x%x", uszFileName, usRetVal);
                                inLogPrintf(AT, szErrorMsg);
                        }

                        inFILE_Close(ulFileHandle);
                        return (VS_ERROR);
                }
                else
                {
			inFile_Open_File_Cnt_Increase();
			
                        if (ginDebug == VS_TRUE)
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "建立新檔案 : %s", uszFileName);
                                inLogPrintf(AT, szErrorMsg);
                        }
                }
        }
        else if (usRetVal == d_OK)
        {
                /* 判斷檔案已存在，return Error */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "檔案已存在 %s   SIZE = %ld", uszFileName, ulFileSize);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }
        else
        {
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "檔案錯誤 usRetVal : 0x%x", usRetVal);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

//        if (ginDebug == VS_TRUE)
//        {
//                inLogPrintf(AT, "inFILE_Create() END!");
//        }

	return (VS_SUCCESS);
}

/*
Function	:inFILE_Read
Date&Time	:2015/8/5 下午 5:12
Describe	:讀檔案，傳入Handle接收的Buffer及讀取的size
*/
int inFILE_Read(unsigned long *ulFileHandle, unsigned char *uszReadData, unsigned long ulReadSize)
{
	char		szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];
        unsigned short  usRetVal;	/* 實際讀到的長度 */
        

//	if (ginDebug == VS_TRUE)
//	{
//		inLogPrintf(AT, "inFILE_Read() START!");
//	}
//	if (ginDebug == VS_TRUE)
//	{
//		memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
//		sprintf(szErrorMsg, "Handle:%u %lu",(int)ulFileHandle ,ulReadSize);
//		inLogPrintf(AT, szErrorMsg);
//	}
	
         /* 讀取檔案存在szReadBuff */
        usRetVal = CTOS_FileRead(*ulFileHandle, uszReadData, &ulReadSize);

        if (usRetVal != d_OK)
        {
		if (ginDebug == VS_TRUE)
                {
			memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "讀取檔案失敗Handle:%u", (int)ulFileHandle);
                        inLogPrintf(AT, szErrorMsg);
			
			memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "usRetVal:0x%x", usRetVal);
                        inLogPrintf(AT, szErrorMsg);
                }

                vdUtility_SYSFIN_LogMessage(AT, "讀取檔案失敗Handle:%u usRetVal:0x%x", (int)ulFileHandle, usRetVal);
		return (VS_ERROR);
        }
	
//	if (ginDebug == VS_TRUE)
//	{
//		memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
//		sprintf(szErrorMsg, "讀取檔案成功Handle:%u",(int)ulFileHandle);
//		inLogPrintf(AT, szErrorMsg);
//	}

//        if (ginDebug == VS_TRUE)
//        {
//                inLogPrintf(AT, "inFILE_Read() END!");
//        }

        return (VS_SUCCESS);
}

/*
Function	:inFILE_Seek
Date&Time	:2015/8/5 下午 5:12
Describe	:尋找檔案，傳入Handle尋找位置及尋找模式_SEEK_BEGIN_或_SEEK_CURRENT_或_SEEK_END_
*/
int inFILE_Seek(unsigned long ulFileHandle, unsigned long ulOffset, int inSeekMode)
{
	char		szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1] = {0};
        unsigned short  usRetVal = VS_ERROR;
        

//        if (ginDebug == VS_TRUE)
//        {
//                inLogPrintf(AT, "inFILE_Seek() START!");
//        }

        /* _SEEK_BEGIN_ = 0  _SEEK_CURRENT_ = 1  _SEEK_END_ = 2 */
        if (inSeekMode == _SEEK_BEGIN_ || _SEEK_CURRENT_ || _SEEK_END_)
        {
                usRetVal = CTOS_FileSeek(ulFileHandle ,ulOffset ,inSeekMode);
        }
        else
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "Seek Mode Error!!");
                }

                vdUtility_SYSFIN_LogMessage(AT, "Seek Mode Error (%d)", inSeekMode);
                return (VS_ERROR);
        }

        if (usRetVal != d_OK)
        {
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        snprintf(szErrorMsg, sizeof(szErrorMsg), "尋找檔案失敗 usRetVal : 0x%x", usRetVal);
                        inLogPrintf(AT, szErrorMsg);
                }

                vdUtility_SYSFIN_LogMessage(AT, "尋找檔案失敗 usRetVal : 0x%x", usRetVal);
		return (VS_ERROR);
        }

//        if (ginDebug == VS_TRUE)
//        {
//                inLogPrintf(AT, "inFILE_Seek() END!");
//        }

        return (VS_SUCCESS);
}

/*
Function	:inFILE_Write
Date&Time	:2015/8/10 下午 8:48
Describe	:寫入檔案
*/
int inFILE_Write(unsigned long *ulFileHandle, unsigned char *uszWriteData, unsigned long ulWriteSize)
{
	char		szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];
        unsigned short  usRetVal;

//        if (ginDebug == VS_TRUE)
//        {
//                inLogPrintf(AT, "inFILE_Write() START!");
//        }

        /* 寫入檔案 */
        usRetVal = CTOS_FileWrite(*ulFileHandle, uszWriteData, ulWriteSize);

        if (usRetVal != d_OK)
        {
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szErrorMsg, "寫入檔案失敗 usRetVal : 0x%x", usRetVal);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

//        if (ginDebug == VS_TRUE)
//        {
//                inLogPrintf(AT, "inFILE_Write() END!");
//        }

        return (VS_SUCCESS);
}

/*
Function	:inFile_Delete
Date&Time	:2015/8/10 下午 8:47
Describe	:刪除檔案，要先關閉Handle才可刪除
*/
int inFILE_Delete(unsigned char *uszFileName)
{
	unsigned short usRetVal;
	char szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFile_Delete() START!");
	}

	usRetVal = CTOS_FileDelete(uszFileName);

	if (usRetVal == d_FS_FILE_NOT_FOUND)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
			snprintf(szErrorMsg, sizeof(szErrorMsg), "找不到檔案 FName:%s", uszFileName);
			inLogPrintf(AT, szErrorMsg);
		}
	}
	else if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
			snprintf(szErrorMsg, sizeof(szErrorMsg), "刪除失敗 FName:%s RetVal : 0x%x", uszFileName, usRetVal);
			inLogPrintf(AT, szErrorMsg);
		}
		inUtility_StoreTraceLog_OneStep("deleteFail FName:%s RetVal : 0x%x", uszFileName, usRetVal);

		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inFile_Delete() END!");

	return (VS_SUCCESS);
}

/*
Function	:inFILE_Close
Date&Time	:2015/8/5 下午 5:12
Describe	:關檔案
*/
int inFILE_Close(unsigned long *ulFileHandle)
{
	char		szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];
        unsigned short  usRetVal;
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inFILE_Close() START!");

        /* 關閉檔案Handle */
        usRetVal = CTOS_FileClose(*ulFileHandle);

        if (usRetVal != d_OK)
        {
                if (ginDebug == VS_TRUE)
                {
			memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "關檔失敗Handle:%u", (int)ulFileHandle);
                        inLogPrintf(AT, szErrorMsg);
			
			memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "usRetVal:0x%x", usRetVal);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }
	else
	{
		if (ginDebug == VS_TRUE)
		{
			sprintf(szErrorMsg, "關檔成功Handle:%u", (int)ulFileHandle);
			inLogPrintf(AT, szErrorMsg);
		}
		inFile_Open_File_Cnt_Decrease();
	}
	/* 這邊改*ulFileHandle = 0會不會比較好，因為*ulFileHandle 是unsigned long型別，
	 * 未來可以新增判斷 *ulFileHandle >0 去做防呆? */
	*ulFileHandle = VS_HANDLE_NULL;
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFILE_Close() END!");
        }
        
        return (VS_SUCCESS);
}

/*
Function	:inFILE_Rename
Date&Time	:2015/8/5 下午 5:12
Describe	:檔名置換
*/
int inFILE_Rename(unsigned char *uszOldFileName, unsigned char *uszNewFileName)
{
        char		szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];
	unsigned short  usRetVal;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFILE_Rename(%s, %s) START!",uszOldFileName, uszNewFileName);
        }

        usRetVal = CTOS_FileRename(uszOldFileName, uszNewFileName);

        if (usRetVal != d_OK)
        {
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szErrorMsg, "檔名修改失敗 usRetVal : 0x%x", usRetVal);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFILE_Rename(%s, %s) END!",uszOldFileName, uszNewFileName);
        }

        return (VS_SUCCESS);
}

/*
Function	:lnFILE_GetSize
Date&Time	:2015/8/5 下午 5:12
Describe	:取得檔案大小，傳入檔名回傳大小，Handle無使用
*/
long lnFILE_GetSize(unsigned long *ulFileHandle, unsigned char *uszFileName)
{
        unsigned short  usRetVal;
        unsigned long   ulFileSize = 0;
        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "lnFILE_GetSize() START");
        }
        usRetVal = CTOS_FileGetSize(uszFileName, &ulFileSize);

        if (usRetVal != d_OK)
        {
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szErrorMsg, "取得檔案大小失敗 usRetVal : 0x%x", usRetVal);
                        inLogPrintf(AT, szErrorMsg);
                }
                vdUtility_SYSFIN_LogMessage(AT, "取得檔案大小失敗 usRetVal : 0x%x F:%s", usRetVal, uszFileName);

                return (VS_ERROR);
        }
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Get File Len:%lu", ulFileSize);
		}
	}
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "lnFILE_GetSize() END");
        }
        return (ulFileSize);
}

/*
Function        :inFILE_Check_Exist
Date&Time       :2015/9/1 下午 2:00
Describe        :檢查檔案是否存在，不在回傳VS_ERROR
*/
int inFILE_Check_Exist(unsigned char *uszFileName)
{
        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];
        unsigned long   ulFileSize = 0;
        unsigned short  usRetVal;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFILE_Check_Exist() start!");
        }

        usRetVal = CTOS_FileGetSize(uszFileName, &ulFileSize);

        if (usRetVal == d_FS_FILE_NOT_FOUND)
        {
                /* 檔案不存在 */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "%s不存在,error:0x%x", uszFileName,usRetVal);
                        inLogPrintf(AT, szErrorMsg);
                }
                return (VS_ERROR);
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFILE_Check_Exist() end!");
        }

        return (VS_SUCCESS);
}

/*
Function        :inFILE_Cut
Date&Time       :2017/1/6 上午 10:11
Describe        :
*/
int inFILE_Cut(unsigned char* uszFileName, unsigned long ulFileSize)
{
	char		szErrorMsg[100 + 1];
	unsigned short	usRetVal;
	
	if (strlen((char*)uszFileName) <= 0)
		return (VS_ERROR);
	
	usRetVal = CTOS_FileCut(uszFileName, ulFileSize);

        if (usRetVal != d_OK)
        {
                /* 檔案不存在 */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "%s切檔失敗,error:0x%x", uszFileName,usRetVal);
                        inLogPrintf(AT, szErrorMsg);
                }
                return (VS_ERROR);
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inFILE_Data_Delete
Date&Time       :2017/1/4 下午 4:11
Describe        :仿照Verifone中的Function行為，會將指定位置指定長度的資料清空，並接上剩餘的資料
*/
int inFILE_Data_Delete(unsigned char *uszFileName, int inOffset, int inFileLen)
{
	int		i = 0;
	long		lnLastFileLen = 0;	/* 需要接回去的長度 */
	long		lnFileSize = 0;
	long		lnLeftSize;		/* 剩餘長度 */
	unsigned char	*uszLastFile;
	unsigned long	ulHandle;

	if (inFILE_Open(&ulHandle, uszFileName) != VS_SUCCESS)
		return (VS_ERROR);
	
	/* V3此function中，無使用handle */
	lnFileSize = lnFILE_GetSize(&ulHandle, uszFileName);
	
	lnLastFileLen = lnFileSize - (inOffset - 1) - inFileLen ;
	/* 理論上不會小於0 */
	if (lnLastFileLen < 0)
	{
		inFILE_Close(&ulHandle);
		return (VS_ERROR);
	}
	
	/* 先把後面讀出來 */
	if (inFILE_Seek(ulHandle, inOffset + inFileLen, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		inFILE_Close(&ulHandle);
		return (VS_ERROR);
	}
	
	uszLastFile = malloc((lnLastFileLen + 1) * sizeof(char));
	memset(uszLastFile, 0x00, (lnLastFileLen + 1) * sizeof(char));
	lnLeftSize = lnLastFileLen;
	for (i = 0;; ++i)
	{
		/* 剩餘長度大於或等於1024 */
		if (lnLeftSize >= 1024)
		{
			if (inFILE_Read(&ulHandle, &uszLastFile[1024 * i], 1024) == VS_SUCCESS)
			{
				/* 一次讀1024 */
				lnLeftSize -= 1024;

				/* 當剩餘長度剛好為1024，會剛好讀完 */
				if (lnLeftSize == 0)
					break;
			}
			/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulHandle);

				/* Free pointer */
				free(uszLastFile);

				return (VS_ERROR);
			}
		}
		/* 剩餘長度小於1024 */
		else if (lnLeftSize < 1024)
		{
			/* 就只讀剩餘長度 */
			if (inFILE_Read(&ulHandle, &uszLastFile[1024 * i], lnLeftSize) == VS_SUCCESS)
			{
				break;
			}
			/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulHandle);

				/* Free pointer */
				free(uszLastFile);

				return (VS_ERROR);
			}
		}
	}
	
	if (inFILE_Seek(ulHandle, inOffset, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		free(uszLastFile);
		inFILE_Close(&ulHandle);
		return (VS_ERROR);
	}
	
	if (inFILE_Write(&ulHandle, uszLastFile, (unsigned long)lnLastFileLen) != VS_SUCCESS)
	{
		free(uszLastFile);
		inFILE_Close(&ulHandle);
		return (VS_ERROR);
	}
	
	inFILE_Close(&ulHandle);
	free(uszLastFile);
	
	/* 將多餘的結尾Cut掉 */
	if (inFILE_Cut(uszFileName, lnFileSize - inFileLen) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFILE_Data_Insert
Date&Time       :2017/1/6 上午 10:51
Describe        :仿照Verifone中的Function行為，會在指定位置塞入特定資料，並接上之後的資料
*/
int inFILE_Data_Insert(unsigned char *uszFileName, int inOffset, int inFileLen, unsigned char *uszInsertData)
{
	int		i = 0;
	long		lnLastFileLen = 0;	/* 需要接回去的長度 */
	long		lnFileSize = 0;
	long		lnLeftSize;		/* 剩餘長度 */
	unsigned char	*uszLastFile;
	unsigned long	ulHandle;

	if (inFILE_Open(&ulHandle, uszFileName) != VS_SUCCESS)
		return (VS_ERROR);
	
	/* V3此function中，無使用handle */
	lnFileSize = lnFILE_GetSize(&ulHandle, uszFileName);
	
	lnLastFileLen = lnFileSize - inOffset;
	/* 理論上不會小於0 */
	if (lnLastFileLen < 0)
	{
		inFILE_Close(&ulHandle);
		return (VS_ERROR);
	}
	
	/* 先把後面讀出來 */
	if (inFILE_Seek(ulHandle, inOffset, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		inFILE_Close(&ulHandle);
		return (VS_ERROR);
	}
	
	uszLastFile = malloc((lnLastFileLen + 1) * sizeof(char));
	memset(uszLastFile, 0x00, (lnLastFileLen + 1) * sizeof(char));
	lnLeftSize = lnLastFileLen;	
	for (i = 0;; ++i)
	{
		/* 剩餘長度大於或等於1024 */
		if (lnLeftSize >= 1024)
		{
			if (inFILE_Read(&ulHandle, &uszLastFile[1024 * i], 1024) == VS_SUCCESS)
			{
				/* 一次讀1024 */
				lnLeftSize -= 1024;

				/* 當剩餘長度剛好為1024，會剛好讀完 */
				if (lnLeftSize == 0)
					break;
			}
			/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulHandle);

				/* Free pointer */
				free(uszLastFile);

				return (VS_ERROR);
			}
		}
		/* 剩餘長度小於1024 */
		else if (lnLeftSize < 1024)
		{
			/* 就只讀剩餘長度 */
			if (inFILE_Read(&ulHandle, &uszLastFile[1024 * i], lnLeftSize) == VS_SUCCESS)
			{
				break;
			}
			/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulHandle);

				/* Free pointer */
				free(uszLastFile);

				return (VS_ERROR);
			}
		}
	}
	
	if (inFILE_Seek(ulHandle, inOffset + inFileLen, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		free(uszLastFile);
		inFILE_Close(&ulHandle);
		return (VS_ERROR);
	}
	
	if (inFILE_Write(&ulHandle, uszLastFile, (unsigned long)lnLastFileLen) != VS_SUCCESS)
	{
		free(uszLastFile);
		inFILE_Close(&ulHandle);
		return (VS_ERROR);
	}
	
	/* Free 後面那一段 */
	free(uszLastFile);
	
	if (inFILE_Seek(ulHandle, inOffset, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		inFILE_Close(&ulHandle);
		return (VS_ERROR);
	}
	
	if (inFILE_Write(&ulHandle, uszInsertData, (unsigned long)inFileLen) != VS_SUCCESS)
	{
		inFILE_Close(&ulHandle);
		return (VS_ERROR);
	}
	
	inFILE_Close(&ulHandle);
	
	return (VS_SUCCESS);
}

/*
Function        :inFILE_Copy_File
Date&Time       :2017/1/9 上午 11:21
Describe        :
*/
int inFILE_Copy_File(unsigned char *uszOldFileName, unsigned char *uszNewFileName)
{
	int		i;
	long		lnFileSize;
	long		lnLeftSize;		/* 剩餘長度 */
	unsigned char*	uszFileData;
	unsigned long	ulHandle;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFILE_Copy_File() START !");
	}
	
	lnFileSize = lnFILE_GetSize(&ulHandle, uszOldFileName);
	if (inFILE_OpenReadOnly(&ulHandle, uszOldFileName) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	uszFileData = malloc((lnFileSize + 1) * sizeof(unsigned char));
	memset(uszFileData, 0x00, (lnFileSize + 1) * sizeof(unsigned char));
	lnLeftSize = lnFileSize;
	
	for (i = 0;; ++i)
	{
		/* 剩餘長度大於或等於1024 */
		if (lnLeftSize >= 1024)
		{
			if (inFILE_Read(&ulHandle, &uszFileData[1024 * i], 1024) == VS_SUCCESS)
			{
				/* 一次讀1024 */
				lnLeftSize -= 1024;

				/* 當剩餘長度剛好為1024，會剛好讀完 */
				if (lnLeftSize == 0)
					break;
			}
			/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulHandle);

				/* Free pointer */
				free(uszFileData);

				return (VS_ERROR);
			}
		}
		/* 剩餘長度小於1024 */
		else if (lnLeftSize < 1024)
		{
			/* 就只讀剩餘長度 */
			if (inFILE_Read(&ulHandle, &uszFileData[1024 * i], lnLeftSize) == VS_SUCCESS)
			{
				break;
			}
			/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulHandle);

				/* Free pointer */
				free(uszFileData);

				return (VS_ERROR);
			}
		}
	}
	
	
	if (inFILE_Close(&ulHandle) != VS_SUCCESS)
	{
		free(uszFileData);
		
		return (VS_ERROR);
	}
	
	inFILE_Delete(uszNewFileName);
	
	if (inFILE_Create(&ulHandle, uszNewFileName) != VS_SUCCESS)
	{
		free(uszFileData);
		
		return (VS_ERROR);
	}
	
	if (inFILE_Write(&ulHandle, uszFileData, lnFileSize) != VS_SUCCESS)
	{
		free(uszFileData);
		inFILE_Close(&ulHandle);
		
		return (VS_ERROR);
	}
	
	if (inFILE_Close(&ulHandle) != VS_SUCCESS)
	{
		free(uszFileData);
		
		return (VS_ERROR);
	}
	
	free(uszFileData);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFILE_Copy_File() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

int inFILE_Test(void)
{
        unsigned long   ulHandle;
        unsigned char   szFile_Buffer[_FILE_DATA_LENGTH_MAX_ + 1];
//        int     inRetVal = 0;
        char    szTemplate[_FILE_DATA_LENGTH_MAX_ + 1];

        memset(szFile_Buffer, 0x00, sizeof(szFile_Buffer));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        /* 開啟已存在檔案 */
        //inRetVal = inFILE_Open(&ulHandle, "HDT.dat");
        /* 檔案不存在，建立新檔案 */
        //inRetVal = inFILE_Create(&ulHandle, "HDT.dat");
        /* 已唯獨方式開啟檔案 */
        //inFILE_OpenReadOnly(&ulHandle, "HDT.dat");
        /* 尋找檔案測試 */
        //inFILE_Seek(ulHandle, 0, _SEEK_END_);
        /* 寫入檔案測試 不用seek END的話會把原資料改寫掉 */
        //inFILE_Write(&ulHandle, "12341234", sizeof("12341234"));
        /* 寫入檔案位置會到最尾巴，需要用seek回到開頭 */
        //inFILE_Seek(ulHandle, 0, _SEEK_BEGIN_);
        /* 讀取檔案測試 */
        //inFILE_Read(&ulHandle , szFile_Buffer , _FILE_DATA_LENGTH_MAX_);

        //strcpy(szTemplate, szFile_Buffer);
        //inLogPrintf(AT, szTemplate);
        //inFILE_Print(szFile_Buffer , _FILE_DATA_LENGTH_MAX_);
        inFILE_Close(&ulHandle);

        return (VS_SUCCESS);
}

/*
Function        :inFile_Linux_Open
Date&Time       :2018/7/13 上午 11:30
Describe        :
*/
int inFile_Linux_Open(int* inFd, char* szFileName)
{
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFile_Linux_Open(%s) START !", szFileName);
	}
	
	*inFd = open(szFileName, O_RDWR);
	
	if (*inFd == -1)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Open fail (%d)", *inFd);
			inLogPrintf(AT, "errno: %d", errno);
		}
		return (VS_ERROR);
	}
	else
	{
//		if (ginDebug == VS_TRUE)
//		{
//			inLogPrintf(AT, "Open Success (%d)", *inFd);
//		}
		inFile_Open_File_Cnt_Increase();
		
		return (VS_SUCCESS);
	}
	
}

/*
Function        :inFile_Linux_Close
Date&Time       :2018/7/13 上午 11:39
Describe        :
*/
int inFile_Linux_Close(int inFd)
{
	int	inRetVal = -1;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFile_Linux_Close(%d) START !", inFd);
	}
	
	inRetVal = close(inFd);
	if (inRetVal == -1)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Close Fail");
		}
		return (VS_ERROR);
	}
	else
	{
//		if (ginDebug == VS_TRUE)
//		{
//			inLogPrintf(AT, "Close Success");
//		}
		inFile_Open_File_Cnt_Decrease();
		
		return (VS_SUCCESS);
	}
}

/*
Function        :inFile_Linux_Create
Date&Time       :2018/7/13 上午 11:39
Describe        :用這隻function create出來的檔案要自己調權限
*/
int inFile_Linux_Create(int* inFd, char* szFileName)
{
	*inFd = open(szFileName, O_CREAT | O_RDWR);
	
	if (*inFd == -1)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Create fail");
		}
		return (VS_ERROR);
	}
	else
	{
//		if (ginDebug == VS_TRUE)
//		{
//			inLogPrintf(AT, "Create Success");
//		}
		inFile_Open_File_Cnt_Increase();
		
		return (VS_SUCCESS);
	}
}

/*
Function        :inFile_Linux_Read
Date&Time       :2018/7/13 上午 11:39
Describe        :
*/
int inFile_Linux_Read(int inFd, char* szBuf, int* inCnt)
{
	int	inRetVal = -1;
	
	inRetVal = read(inFd, szBuf, *inCnt);
	
	if (inRetVal == -1)
	{
		*inCnt = 0;
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Read fail");
		}
		return (VS_ERROR);
	}
	else
	{
		*inCnt = inRetVal;
		
//		if (ginDebug == VS_TRUE)
//		{
//			inLogPrintf(AT, "Read Success");
//		}
		return (VS_SUCCESS);
	}
}

/*
Function        :inFile_Linux_Write
Date&Time       :2018/7/13 上午 11:39
Describe        :
*/
int inFile_Linux_Write(int inFd, char* szBuf, int* inCnt)
{
	int	inRetVal = -1;
	
	inRetVal = write(inFd, szBuf, *inCnt);
	
	if (inRetVal == -1)
	{
		*inCnt = 0;
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "write fail");
		}
		return (VS_ERROR);
	}
	else
	{
		*inCnt = inRetVal;
		
//		if (ginDebug == VS_TRUE)
//		{
//			inLogPrintf(AT, "write Success");
//		}
		return (VS_SUCCESS);
	}
}

/*
Function        :inFile_Linux_Seek
Date&Time       :2018/7/13 上午 11:39
Describe        :
*/
int inFile_Linux_Seek(int inFd, int inOffset, int inMode)
{
	int	inRetVal = -1;
	int	inWhence = 0;
	
	if (inMode == _SEEK_BEGIN_)
	{
		inWhence = SEEK_SET;
	}
	else if (inMode == _SEEK_CURRENT_)
	{
		inWhence = SEEK_CUR;
	}
	else if (inMode == _SEEK_END_)
	{
		inWhence = SEEK_END;
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Mode error");
		}
		return (VS_ERROR);
	}
	
	inRetVal = lseek(inFd, inOffset, inWhence);
	if (inRetVal == -1)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "seek fail");
		}
		return (VS_ERROR);
	}
	else
	{
//		if (ginDebug == VS_TRUE)
//		{
//			inLogPrintf(AT, "seek Success");
//		}
		return (VS_SUCCESS);
	}
}

/*
Function        :inFile_Linux_Get_FileSize
Date&Time       :2018/7/13 下午 1:36
Describe        :如果須查詢的檔案很多會較慢，如果大量使用建議改用新方法inFile_Linux_Get_FileSize_By_Stat
*/
int inFile_Linux_Get_FileSize_By_LSeek(int inFd, int* inFileSize)
{
	int	inRetVal = -1;
	
	inRetVal = lseek(inFd, 0, SEEK_END);
	if (inRetVal == -1)
	{
		*inFileSize = 0;
		return (VS_ERROR);
	}
	else
	{
		*inFileSize = inRetVal;
		return (VS_SUCCESS);
	}
}

/*
Function        :inFile_Linux_Get_FileSize_By_Stat
Date&Time       :2020/7/1 下午 5:10
Describe        :
*/
int inFile_Linux_Get_FileSize_By_Stat(char* szPath, int* inFileSize)
{
	int		inRetVal = -1;
	struct stat	srStatBuf = {0};
	
	inRetVal = stat(szPath, &srStatBuf);
	
	if (inRetVal == -1)
	{
		*inFileSize = 0;
		return (VS_ERROR);
	}
	else
	{
		*inFileSize = srStatBuf.st_size;
		return (VS_SUCCESS);
	}
}
/*
Function        :inFile_Linux_Open_In_Fs_Data
Date&Time       :2019/1/18 上午 9:39
Describe        :
*/
int inFile_Linux_Open_In_Fs_Data(int* inFd, char* szFileName)
{
	int	inRetVal = VS_SUCCESS;
	char	szPathName[100 + 1] = {0};
	
	memset(szPathName, 0x00, sizeof(szPathName));
	sprintf(szPathName, "%s%s", _FS_DATA_PATH_, szFileName);
	
	inRetVal = inFile_Linux_Open(inFd, szPathName);
	
	return (inRetVal);
}

/*
Function        :inFile_Linux_Create_In_Fs_Data
Date&Time       :2019/1/18 上午 9:33
Describe        :
*/
int inFile_Linux_Create_In_Fs_Data(int* inFd, char* szFileName)
{
	int	inRetVal = VS_SUCCESS;
	char	szPathName[100 + 1] = {0};
	
	memset(szPathName, 0x00, sizeof(szPathName));
	sprintf(szPathName, "%s%s", _FS_DATA_PATH_, szFileName);
	
	inRetVal = inFile_Linux_Create(inFd, szPathName);
	
	return (inRetVal);
}

/*
Function        :inFile_Linux_Delete_In_Fs_Data
Date&Time       :2019/1/17 上午 11:49
Describe        :處理fs_data內的檔案刪除
*/
int inFile_Linux_Delete_In_Fs_Data(char* szFileName)
{
	int	inRetVal = VS_SUCCESS;
	
	inRetVal = inFile_Unlink_File(szFileName, _FS_DATA_PATH_);
	
	return (inRetVal);
}

/*
Function        :inFile_Linux_Rename_In_Fs_Data
Date&Time       :2019/1/17 下午 1:35
Describe        :
*/
int inFile_Linux_Rename_In_Fs_Data(char *szOldFileName, char *szNewFileName)
{
	int	inRetVal = VS_SUCCESS;
	
	inRetVal = inFunc_Data_Rename(szOldFileName, _FS_DATA_PATH_, szNewFileName, _FS_DATA_PATH_);
	
	return (VS_SUCCESS);
}

/*
Function        :inFile_Unlink_File
Date&Time       :2019/7/4 上午 11:41
Describe        :刪除檔案，用system call rm需要至少30毫秒，改用此執行五次約只耗10毫秒
 *		 linux專用，或可使用stdio.h的remove
*/
int inFile_Unlink_File(char *szFileName, char* szSource)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1] = {0};
	char	szPath[100 + 1] = {0};	/* Shell Command*/
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFile_Unlink_File START!");
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FNaeme: %s", szFileName);
		inLogPrintf(AT, szDebugMsg);
	}
		
	/* rm 資料夾路徑/檔名 */
	/* 組命令 */
	memset(szPath, 0x00, sizeof(szPath));
	if (szFileName != NULL)
	{	
		if (szSource != NULL)
		{
			strcat(szPath, szSource);
		}

		/* 目標檔名 */
		strcat(szPath, szFileName);
	}
	else
	{
		return (VS_ERROR);
	}
	
	/* 執行命令 */
	inRetVal = unlink(szPath);
	if (inRetVal < 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Unlink failed:%d", inRetVal);
		}
		
		vdUtility_SYSFIN_LogMessage(AT,  "Unlink failed [%s] Rvl[%d] Error[%d]", szPath, inRetVal, errno);
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FNaeme : %s", szFileName);
		inLogPrintf(AT, szDebugMsg);
		
		inLogPrintf(AT, "inFile_Unlink_File END!");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inFile_Move_File
Date&Time       :2019/7/4 下午 4:18
Describe        :
*/
int inFile_Move_File(char *szSrcFileName, char* szSource, char* szDesFileName, char* szDestination)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1] = {0};
	char	szSrcPath[100 + 1] = {0};
	char	szDesPath[100 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFile_Move_File START!");
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FName: %s", szSrcFileName);
		inLogPrintf(AT, szDebugMsg);
	}
	
	memset(szSrcPath, 0x00, sizeof(szSrcPath));
	if (strlen(szSrcFileName) != 0)
	{
		if (strlen(szSource) != 0)
		{
			strcat(szSrcPath, szSource);
		}
		
		strcat(szSrcPath, szSrcFileName);
	}
	/* 沒有來源檔 */
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "No Source File Name");
		}
		return (VS_ERROR);
	}
	
	memset(szDesPath, 0x00, sizeof(szDesPath));
	if (strlen(szDestination) != 0)
	{
		strcat(szDesPath, szDestination);
	}

	if (strlen(szDesFileName) != 0)
	{
		strcat(szDesPath, szDesFileName);
	}
	/* 沒給目標檔名就用原檔名 */
	else
	{
		strcat(szDesPath, szSrcFileName);
	}
	
	/* 執行命令 */
	inRetVal = rename(szSrcPath, szDesPath);
	if (inRetVal < 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "move failed:%d", inRetVal);
		}
		return (inRetVal);
	}
		
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FName : %s", szSrcFileName);
		inLogPrintf(AT, szDebugMsg);
		
		inLogPrintf(AT, "inFile_Move_File END!");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFile_Open_File_Cnt_Increase
Date&Time       :2019/8/14 上午 9:42
Describe        :
*/
int inFile_Open_File_Cnt_Increase(void)
{
	ginFileOpenCnt++;
	if (ginFileOpenCnt > 1)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "ginFileOpenCnt :%d", ginFileOpenCnt);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFile_Open_File_Cnt_Decrease
Date&Time       :2019/8/14 上午 9:42
Describe        :
*/
int inFile_Open_File_Cnt_Decrease(void)
{
	ginFileOpenCnt--;
	
	if (ginFileOpenCnt < 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "ginFileOpenCnt :%d", ginFileOpenCnt);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFile_Open_File_Cnt_Return
Date&Time       :2019/8/14 上午 9:42
Describe        :
*/
int inFile_Open_File_Cnt_Return(int* inFileOpenCnt)
{
	*inFileOpenCnt = ginFileOpenCnt;
	
	return (VS_SUCCESS);
}

/*
Function        :inFile_Linux_Check_File_Exist
Date&Time       :2024/8/7 下午 3:13
Describe        :
*/
int inFile_Linux_Check_File_Exist(char* szPath, char* szFileName)
{
        int     inFd = 0;
	int	inRetVal = VS_SUCCESS;
	char	szPathName[100 + 1] = {0};
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inFile_Linux_Check_File_Exist() START !");
        }
	
	memset(szPathName, 0x00, sizeof(szPathName));
	sprintf(szPathName, "%s%s", szPath, szFileName);
	
	inRetVal = inFile_Linux_Open(&inFd, szPathName);
	
        if (inRetVal == -1)
        {
                return (VS_ERROR);
        }
        else
        {
                inFile_Linux_Close(inFd);
                
                return (VS_SUCCESS);
        }
}