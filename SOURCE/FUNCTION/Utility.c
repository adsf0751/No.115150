#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>   //2019.02.15 added by Hachi
#include <unistd.h>
#include <ctosapi.h>
#include <linux/stddef.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <limits.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Transaction.h"
#include "../DISPLAY/Display.h"
#include "File.h"
#include "Function.h"
#include "Utility.h"

#define max(x, y) (((x) > (y)) ? (x) : (y))

extern int ginDebug;
extern int ginMachineType;
extern unsigned long gulTotalROMSize;
int ginLogHandle = -1; //2019/3/11 下午 5:53

char gszCurrentLogFilename[256] = {0};
int gpLog_fd = -1; // 檔案描述符
unsigned char guszLogDirCheckBit = VS_FALSE;

static int ginLogCounter = 0;
static int ginSizeCheckCounter = 0;
/* 因為寫入的log msg可能很大，需要透過while慢慢傳過去? */
static int inUtility_WriteFile(int infd, const char *szbuf, size_t len)
{
	size_t total = 0;

	while (total < len)
	{
		ssize_t ret = write(infd, szbuf + total, len - total);

		if (ret < 0)
		{
			if (errno == EINTR)
				continue;

			return -1;
		}

		total += ret;
	}

	return 0;
}

static int safe_mul_size(size_t a, size_t b, size_t *out) {
    if (a == 0 || b == 0) {
        *out = 0;
        return 0;
    }
    if (a > SIZE_MAX / b) {
        return -1;
    }
    *out = a * b;
    return 0;
}

static int calc_stride24(int width) {
    return width * 3 + ((4 - ((width * 3) % 4)) % 4);
}

static int calc_stride1(int width) {
    return ((width + 31) / 32) * 4;
}

static int validate_bmp_header(const BMPFILEHEADER *fh, const BMPINFOHEADER *ih) {
    if (!fh || !ih) return -1;
    if (fh->bfType != 0x4D42) return -1;
    if (ih->biPlanes != 1) return -1;
    if (ih->biCompression != 0) return -1;
    if (ih->biWidth <= 0 || ih->biWidth > MAX_BMP_WIDTH) return -1;
    if (ih->biHeight == INT_MIN || abs(ih->biHeight) > MAX_BMP_HEIGHT) return -1;
    if (ih->biBitCount != 1 && ih->biBitCount != 24) return -1;
    return 0;
}

static unsigned char* safe_malloc_zero(size_t size) {
    unsigned char *p;
    if (size == 0) return NULL;
    p = (unsigned char*)malloc(size);
    if (!p) return NULL;
    memset(p, 0, size);
    return p;
}


/*  Added by Hachi(Nick)
Function        :inBATCH_CloseTraceLog
Date&Time       :2019/3/11 下午 3:51
Describe        :將log檔案開啟
 */
int inUtility_OpenTraceLogFile(void)
{
	char szTempBuf[100 + 1] = {0};

	strcpy(szTempBuf, _FS_DATA_PATH_);
	strcat(szTempBuf, _EDC_TRACE_LOG_FILE_NAME_);
	ginLogHandle = open(szTempBuf, O_RDWR);

	if (ginLogHandle <= 0)
	{
		return (VS_ERROR);
	} else
	{
		inFile_Open_File_Cnt_Increase();

		return (VS_SUCCESS);
	}

	/*Debug Message*/
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "ginLogHandle is %d ,func = %s, Line = %d", ginLogHandle, __FUNCTION__, __LINE__);
	}
}

/*  Added by Hachi(Nick)
Function        :inBATCH_CloseTraceLog
Date&Time       :2019/3/11 下午 3:51
Describe        :將log檔案關閉
 */
int inUtility_CloseTraceLogFile(void)
{
	int inHandle = 0;

	if (ginLogHandle != -1)
	{
		inHandle = close(ginLogHandle);

		if (inHandle == -1)
		{
			ginLogHandle = -1;
			return (VS_ERROR);
		} else
		{
			ginLogHandle = -1;
			inFile_Open_File_Cnt_Decrease();
			return (VS_SUCCESS);
		}

		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inHandle is %d ,func = %s, Line = %d", inHandle, __FUNCTION__, __LINE__);
		}
	}
	return (VS_SUCCESS);
}

/*  Added by Hachi(Nick)
Function        :inBATCH_StoreTraceLog
Date&Time       :2019/2/15 上午 11:36
Describe        :將錯誤log資訊記錄到檔案裡
 */

int inUtility_StoreTraceLog(char *szMsg)
{
	int inWriteSizes = 0, i = 0;
	long lnOldSize = 0;
	char szTempBuf[100 + 1] = {0};
	char szTempWrite[3000 + 1] = "";
	char szTempOut[3000 + 1] = "";
	unsigned long ulHandle;
	RTC_NEXSYS get_time;

	strcpy(szTempBuf, _FS_DATA_PATH_);
	strcat(szTempBuf, _EDC_TRACE_LOG_FILE_NAME_);

	lnOldSize = lnFILE_GetSize(&ulHandle, (unsigned char *) _EDC_TRACE_LOG_FILE_NAME_);

	/*檢查寫檔前的檔案大小*/
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "lnOldsize is %ld,line is %d", lnOldSize, __LINE__);
	}

	inFunc_GetSystemDateAndTime(&get_time);
	inWriteSizes = snprintf(szTempWrite, sizeof (szTempWrite), "%02u%02u%02u%02u%02u %s\n"
		, get_time.uszMonth
		, get_time.uszDay
		, get_time.uszHour
		, get_time.uszMinute
		, get_time.uszSecond
		, szMsg);

	if (!memcmp(&szTempWrite[10], " : EDC_BOOT_Time", 16)) /* 開機時間永遠寫再第一格 */
	{
		lseek(ginLogHandle, 0L, SEEK_SET);
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "fun=%s ,line =%d", __FUNCTION__, __LINE__);
		}
	} else if (lnOldSize + inWriteSizes <= 3000)
	{
		lseek(ginLogHandle, 0L, SEEK_END);

		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "lnOldSize +inWriteSizes<=3000");
			inLogPrintf(AT, "lnOldSize is : %ld,inWriteSizes is: %d", lnOldSize, inWriteSizes);
		}
	} else
	{
		lseek(ginLogHandle, 0L, SEEK_SET);
		read(ginLogHandle, szTempOut, (int) lnOldSize);

		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "lnOldSize +inWriteSizes>3000");
			inLogPrintf(AT, "lnOldSize is : %ld,inWriteSizes is: %d", lnOldSize, inWriteSizes);
		}

		for (i = 27; i < (int) lnOldSize; i++)
		{
			if (szTempOut[i] == 0x0A)
			{
				if ((i - 27 + 1) >= inWriteSizes)
				{
					lseek(ginLogHandle, 0L, SEEK_SET);
					memset(szTempOut, 0x00, sizeof (szTempOut));
					read(ginLogHandle, szTempOut, 27);
					lseek(ginLogHandle, (long) (i + 1), SEEK_SET);
					read(ginLogHandle, szTempOut + 27, (int) lnOldSize - (i + 1));

					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "function = %s ,line = %d,index i = %d", __FUNCTION__, __LINE__, i);
						//                                            inLogPrintf(AT, "szTempOut buf is: \n%s\n", szTempOut);
					}
					break;
				}
			}
		}

		truncate(szTempBuf, 0); /*LOG檔案清空*/

		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "lnOldsize is %ld,line is %d", lnOldSize, __LINE__);
		}

		lseek(ginLogHandle, 0L, SEEK_SET); //2019/3/12 上午 10:33
		write(ginLogHandle, szTempOut, ((int) lnOldSize - (i + 1)) + 27);

		if (ginDebug == VS_TRUE)
		{
			//                    inLogPrintf(AT, "szTempOut buf is:\n%s\n",szTempOut);
			inLogPrintf(AT, "function = %s ,line = %d,index i = %d", __FUNCTION__, __LINE__, i);
		}
	}
	write(ginLogHandle, szTempWrite, inWriteSizes);

	/*檢查寫完的檔案內容*/
	/*
	lnOldSize = lnFILE_GetSize(&ulHandle,( unsigned char *)_EDC_TRACE_LOG_FILE_NAME_);
	printf("lnOldsize is %ld,line is %d\n",lnOldSize,__LINE__);
	lseek(ginLogHandle, 0L,SEEK_SET); 
	memset(szTempOut,0x00,sizeof(szTempOut));
	read(ginLogHandle,szTempOut,(int)lnOldSize);
	printf("String BUFF is :\n%s\n",szTempOut);
	 */

	return (VS_SUCCESS);
}

/*  Added by Hachi(Nick)
Function        :inBATCH_StoreTraceLog
Date&Time       :2019/2/15 上午 11:36
Describe        :將檔案內容清除，並保留開機時間
 */
int inUtility_ClearFile(void)
{
	int inRetVal;
	long lnOldSize;
	unsigned long ulHandle;
	unsigned char uzPathBuf[100 + 1] = "";
	unsigned char uzTempBuf[27 + 1] = "";

	strcpy((char *) uzPathBuf, _FS_DATA_PATH_);
	strcat((char *) uzPathBuf, _EDC_TRACE_LOG_FILE_NAME_);

	inRetVal = open((char *) uzPathBuf, O_RDONLY);
	if (inRetVal > 0)
	{
		inFile_Open_File_Cnt_Increase();
	}
	read(inRetVal, uzTempBuf, 27);
	inRetVal = close(inRetVal);
	if (inRetVal == 0)
	{
		inFile_Open_File_Cnt_Decrease();
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "the Readstring is: %s", uzTempBuf);
	}

	inRetVal = open((char *) uzPathBuf, O_TRUNC | O_RDWR);
	if (inRetVal > 0)
	{
		inFile_Open_File_Cnt_Increase();
	}
	write(inRetVal, uzTempBuf, 27);
	inRetVal = close(inRetVal);
	if (inRetVal == 0)
	{
		inFile_Open_File_Cnt_Decrease();
	}

	if (ginDebug == VS_TRUE)
	{
		lnOldSize = lnFILE_GetSize(&ulHandle, (unsigned char *) _EDC_TRACE_LOG_FILE_NAME_);
		inLogPrintf(AT, "the Writestring is: %s", uzTempBuf);
		inLogPrintf(AT, "Oldsize is %ld,line is %d", lnOldSize, __LINE__);
	}

	/* 刪除檔案用*/
	//ret=remove((char *)uzPathBuf);
	//printf("ret is %d\n",ret);

	return (VS_SUCCESS);
}

/*
Function        :inBATCH_StoreTraceLog_OneStep
Date&Time       :2019/4/8 下午 5:18
Describe        :
 */
int inUtility_StoreTraceLog_OneStep(char *szMsg, ...)
{
	va_list list;
	char szDebugMsg[5000 + 1] = {0};

	inUtility_OpenTraceLogFile();

	va_start(list, szMsg);
	/*[260318_BUG] 修改擷取資料使用方式 */
	vsnprintf(szDebugMsg, sizeof (szDebugMsg), szMsg, list);
	va_end(list);
	inUtility_StoreTraceLog(szDebugMsg);

	inUtility_CloseTraceLogFile();

	return (VS_SUCCESS);
}

/*
 Function        :vdUtility_SYSFIN_OpenLogFile
 Date&Time       :2024/10/9 上午 11:15
 Describe        :開啟 log 檔案
 */
void vdUtility_SYSFIN_OpenLogFile()
{
	char szLogFilename[256] = {0};

	// 取得 log 檔名
	// 取得機器當前日期時間戳的log檔名
	vdUtility_SYSFIN_get_log_filename(szLogFilename, sizeof (szLogFilename));

	// 開啟 log 檔案，若不存在則創建
	gpLog_fd = open(szLogFilename, O_CREAT | O_WRONLY | O_APPEND, 0644);

	if (gpLog_fd == -1)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Error opening log file %s", gszCurrentLogFilename);
			inLogPrintf(AT, " [FS] open failed: %s", strerror(errno));
		}
	}

	memset(gszCurrentLogFilename, 0x00, sizeof (gszCurrentLogFilename));
	sprintf(gszCurrentLogFilename, "%s", szLogFilename);
}

/*
 Function        :vdUtility_SYSFIN_CloseLogFile
 Date&Time       :2024/10/9 上午 11:16
 Describe        :關閉 log 檔案
 */
void vdUtility_SYSFIN_CloseLogFile()
{
	if (gpLog_fd != -1)
	{
		close(gpLog_fd);
		gpLog_fd = -1;
		memset(gszCurrentLogFilename, 0x00, sizeof (gszCurrentLogFilename));
	}
}

/*
 Function        :vdUtility_SYSFIN_LogMessage
 Date&Time       :2024/10/9 上午 11:16
 Describe        :寫入 log 並立即使用 fsync 同步到磁碟
 */
void vdUtility_SYSFIN_LogMessage(char* szLocation, const char *format, ...)
{
	/* gulTotalROMSize 為CTOS_SystemMemoryStatus()取得的 disk memory */
	/* 總空間小於200M的機器不存log */
	if (gulTotalROMSize < 200)
	{
		return;
	}

	if (gpLog_fd == -1)
	{
		vdUtility_SYSFIN_OpenLogFile();
		if (gpLog_fd == -1)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "vdUtility_SYSFIN_LogMessage Open File Error");
			}
			return;
		}
		lseek(gpLog_fd, 0L, SEEK_END);
	}

	char szLogMessage[8 * 1024] = {0};
	char szFinalLogMessage[10 * 1024] = {0};
	// 取得目前時間，格式為 24-10-08 17:22:10.210
	RTC_NEXSYS srRTC;
	unsigned long ulMileSecond = 0;
	char szTimeStamp[24] = {0};
	struct timeval tTimeVal;

	va_list args;
	va_start(args, format);
	vsnprintf(szLogMessage, sizeof (szLogMessage), format, args);
	va_end(args);

	inFunc_GetSystemDateAndTime(&srRTC);
	gettimeofday(&tTimeVal, NULL);
	ulMileSecond = (tTimeVal.tv_usec / 1000);
	snprintf(szTimeStamp, sizeof (szTimeStamp), "%02u-%02u-%02u %02u:%02u:%02u.%03lu",
		srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay,
		srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond, ulMileSecond); // 固定毫秒

	// 寫入 log 訊息，包含時間戳
	snprintf(szFinalLogMessage, sizeof (szFinalLogMessage), "[%s][%s] %s\n", szTimeStamp, szLocation, szLogMessage);
	if (inUtility_WriteFile(gpLog_fd, szFinalLogMessage, strlen(szFinalLogMessage)) < 0)
	{
		if (ginDebug)
			printf("LOG write error\n");

		vdUtility_SYSFIN_CloseLogFile();
		return;
	}

	ginLogCounter++;
	ginSizeCheckCounter++;

	if (ginDebug == VS_TRUE)
	{
		printf("NEX:");
		printf(szFinalLogMessage);
	}

	/* periodic fsync */
	if (ginLogCounter >= LOG_FSYNC_INTERVAL)
	{
		fsync(gpLog_fd);
		ginLogCounter = 0;
	}

	/* check log size occasionally */
	if (ginSizeCheckCounter >= SIZE_CHECK_INTERVAL)
	{
		/* 清除最舊log */
		vdUtility_SYSFIN_ManageLogFileSize();
		ginSizeCheckCounter = 0;
	}

}

/*
 Function        :vdUtility_SYSFIN_ManageLogFileSize
 Date&Time       :2024/10/9 上午 10:56
 Describe        :檢查並管理 log 檔案大小
 */
void vdUtility_SYSFIN_ManageLogFileSize()
{	/* 計算 sysfinlog目錄底下的 所有一般檔案的大小總和 */
	long total_size = inUtility_SYSFIN_GetTotalLogSize();
	
	while (total_size > MAX_TOTAL_LOG_SIZE)
	{
		vdUtility_SYSFIN_DeleteOldestLogFile();
		total_size = inUtility_SYSFIN_GetTotalLogSize();
	}
}

/*
 Function        :inUtility_SYSFIN_GetTotalLogSize
 Date&Time       :2024/10/8 下午 6:18
 Describe        :計算所有 log 檔案的總大小
 */
int inUtility_SYSFIN_GetTotalLogSize()
{
	int inFileSize = 0;
	long total_size = 0;
	struct dirent *entry;
	DIR *dir = opendir(_AP_PUB_LOG_PATH_);

	if (dir == NULL)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Unable to open log directory");
		}

		return -1;
	}

	// 遍歷目錄並累加檔案大小
	while ((entry = readdir(dir)) != NULL)
	{
		if (entry->d_type == DT_REG) // 僅計算檔案
		{
			char filepath[512] = {0};
			snprintf(filepath, sizeof (filepath), "%s%s", _AP_PUB_LOG_PATH_, entry->d_name);
			inFileSize = 0;
			inFile_Linux_Get_FileSize_By_Stat((char *) filepath, &inFileSize);
			total_size += inFileSize;
		}
	}

	closedir(dir);

	return total_size;
}

/*
 Function        :vdUtility_SYSFIN_DeleteOldestLogFile
 Date&Time       :2024/10/8 下午 6:06
 Describe        :刪除最舊的 log 檔案
 */
void vdUtility_SYSFIN_DeleteOldestLogFile()
{
	struct dirent *entry;
	DIR *dir = opendir(_AP_PUB_LOG_PATH_);
	char szOldest_file[512] = {0};
	time_t oldest_time = time(NULL);

	if (dir == NULL)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Unable to open log directory");
		}
	}

	// 找出最舊的檔案
	while ((entry = readdir(dir)) != NULL)
	{
		if (entry->d_type == DT_REG) // 僅檔案
		{
			char filepath[512] = {0};
			snprintf(filepath, sizeof (filepath), "%s%s", _AP_PUB_LOG_PATH_, entry->d_name);

			struct stat st;
			if (stat(filepath, &st) == 0)
			{	
				if (st.st_mtime < oldest_time)
				{
					oldest_time = st.st_mtime;
					strncpy(szOldest_file, filepath, sizeof (szOldest_file) - 1);
				}
			}
		}
	}

	closedir(dir);

	// 刪除最舊的檔案
	if (strlen(szOldest_file) > 0)
	{
		if (inUtility_TryDelete(szOldest_file) == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Deleted oldest log file: %s ", szOldest_file);
			}
		} else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Error deleting log file");
			}
		}
	}
}

/*
 Function        :vdUtility_SYSFIN_get_log_filename
 Date&Time       :2024/10/9 上午 11:17
 Describe        :取得 log 檔名
 */
void vdUtility_SYSFIN_get_log_filename(char *szFilename, size_t size)
{
	RTC_NEXSYS srRTC = {0};

	if (guszLogDirCheckBit != VS_TRUE)
	{	/* 確認是否有這個路徑和是否為目錄，如果不存在要建立。 */
		struct stat st;
		if (stat(_AP_PUB_PATH_ _AP_PUB_LOG_DIR_NAME_, &st) == 0 && (st.st_mode & S_IFDIR))
		{

		} else
		{
			inFunc_Dir_Make(_AP_PUB_LOG_DIR_NAME_, _AP_PUB_PATH_);
		}

		guszLogDirCheckBit = VS_TRUE;
	}

	inFunc_GetSystemDateAndTime(&srRTC);
	snprintf(szFilename, size, "%s%02u%02u%02u_nexsys.log", _AP_PUB_LOG_PATH_, srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
}

/*
 Function        :vdUtility_SYSFIN_check_log_date
 Date&Time       :2025/1/15 下午 1:59
 Describe        :若vdUtility_SYSFIN_get_log_filename取得的檔名
 */
void vdUtility_SYSFIN_check_log_date()
{
	char szLogFilename[256] = {0};

	// 取得 log 檔名
	vdUtility_SYSFIN_get_log_filename(szLogFilename, sizeof (szLogFilename));

	if (memcmp(gszCurrentLogFilename, szLogFilename, max(strlen(gszCurrentLogFilename), strlen(szLogFilename))))
	{
		vdUtility_SYSFIN_CloseLogFile();
	}

}

int inUtility_TryDelete(const char *path)
{
	if (unlink(path) == 0) return 0;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "File Unlink Error[%d] ", errno);
	}

	return -1;
}



/* 24-bit 轉 1-bit 二值化 (含防護) */
unsigned char* pszUtility_SYSFIN_Convert24BitTo1Bit(const unsigned char* in24Bit, int width, int height) {
    int inStride, outStride;
    size_t outDataSize;
    unsigned char* out1Bit;
    int y, x, gray, val, byteIdx, bitIdx;
    const unsigned char* src;

    inStride = calc_stride24(width);
    outStride = calc_stride1(width);
    
    if (safe_mul_size((size_t)outStride, (size_t)height, &outDataSize) != 0) return NULL;
    
    out1Bit = safe_malloc_zero(outDataSize);
    if (!out1Bit) return NULL;
    memset(out1Bit, 0xFF, outDataSize); 

    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            src = in24Bit + y * inStride + x * 3;
            /* RGB 轉灰階，使用 BT.601 公式 */
            gray = (src[2] * 299 + src[1] * 587 + src[0] * 114) / 1000;
            val = (gray > BIN_THRESHOLD) ? 1 : 0; 
            
            byteIdx = x / 8;
            bitIdx = 7 - (x % 8);
            if (val == 1) {
                out1Bit[y * outStride + byteIdx] |= (1 << bitIdx);
            } else {
                out1Bit[y * outStride + byteIdx] &= ~(1 << bitIdx);
            }
        }
    }
    return out1Bit;
}

/* 雙線性插值縮放 (Bilinear Interpolation) */
unsigned char* pszUtility_SYSFIN_ResizeBMP24Data(const unsigned char* inData, int inWidth, int inHeight, int outWidth, int outHeight) {
    int inStride, outStride;
    size_t outDataSize;
    unsigned char* outData;
    int y, x, srcX, srcY, dx, dy;
    int w00, w10, w01, w11;
    const unsigned char *p00, *p10, *p01, *p11;
    int b, g, r;

    if (!inData || inWidth <= 0 || inHeight <= 0 || outWidth <= 0 || outHeight <= 0) return NULL;
    if (outWidth > MAX_BMP_WIDTH || outHeight > MAX_BMP_HEIGHT) return NULL;

    inStride = calc_stride24(inWidth);
    outStride = calc_stride24(outWidth);
    if (safe_mul_size((size_t)outStride, (size_t)outHeight, &outDataSize) != 0) return NULL;

    outData = safe_malloc_zero(outDataSize);
    if (!outData) return NULL;
    memset(outData, 0xFF, outDataSize);

    for (y = 0; y < outHeight; ++y) {
        srcY = (y * (inHeight - 1)) / outHeight;
        dy = ((y * (inHeight - 1) * 256) / outHeight) - (srcY * 256);

        for (x = 0; x < outWidth; ++x) {
            srcX = (x * (inWidth - 1)) / outWidth;
            dx = ((x * (inWidth - 1) * 256) / outWidth) - (srcX * 256);

            w00 = (256 - dx) * (256 - dy);
            w10 = dx * (256 - dy);
            w01 = (256 - dx) * dy;
            w11 = dx * dy;

            p00 = inData + srcY * inStride + srcX * 3;
            p10 = inData + srcY * inStride + (srcX + 1 < inWidth ? srcX + 1 : srcX) * 3;
            p01 = inData + (srcY + 1 < inHeight ? srcY + 1 : srcY) * inStride + srcX * 3;
            p11 = inData + (srcY + 1 < inHeight ? srcY + 1 : srcY) * inStride + (srcX + 1 < inWidth ? srcX + 1 : srcX) * 3;

            b = (p00[0] * w00 + p10[0] * w10 + p01[0] * w01 + p11[0] * w11) >> 16;
            g = (p00[1] * w00 + p10[1] * w10 + p01[1] * w01 + p11[1] * w11) >> 16;
            r = (p00[2] * w00 + p10[2] * w10 + p01[2] * w01 + p11[2] * w11) >> 16;

            outData[y * outStride + x * 3 + 0] = (unsigned char)b;
            outData[y * outStride + x * 3 + 1] = (unsigned char)g;
            outData[y * outStride + x * 3 + 2] = (unsigned char)r;
        }
    }
    return outData;
}

/* 智慧讀取：自動判別 1/24 bit，並統一轉成 24-bit (安全防護版) */
unsigned char* pszUtility_SYSFIN_ReadBMP_AutoNormalized24(const char *filename, BMPFILEHEADER *fh, BMPINFOHEADER *ih) {
    FILE *fp = NULL;
    unsigned char *data = NULL;
    size_t totalSize = 0;
    int stride = 0;
    int height = 0;

    fp = fopen(filename, "rb");
    if (!fp) return NULL;

    if (fread(fh, sizeof(BMPFILEHEADER), 1, fp) != 1) goto cleanup;
    if (fread(ih, sizeof(BMPINFOHEADER), 1, fp) != 1) goto cleanup;
    if (validate_bmp_header(fh, ih) != 0) goto cleanup;

    height = abs(ih->biHeight);

    if (ih->biBitCount == 24) {
        stride = calc_stride24(ih->biWidth);
        if (safe_mul_size((size_t)stride, (size_t)height, &totalSize) != 0) goto cleanup;
        
        data = safe_malloc_zero(totalSize);
        if (!data) goto cleanup;
        
        if (fseek(fp, fh->bfOffBits, SEEK_SET) != 0) {
            free(data); data = NULL; goto cleanup;
        }
        if (fread(data, 1, totalSize, fp) != totalSize) {
            free(data); data = NULL; goto cleanup;
        }
    } else {
        /* 1-bit 解析處理，正確解析調色盤並防護記憶體洩漏 */
        unsigned char palette[8];
        unsigned char *raw1 = NULL;
        unsigned char *out24 = NULL;
        int inStride;
        unsigned char c0_B, c0_G, c0_R, c1_B, c1_G, c1_R;
        int x, y, byteIdx, bitIdx, val;

        if (fread(palette, 1, 8, fp) != 8) goto cleanup;
        if (fseek(fp, fh->bfOffBits, SEEK_SET) != 0) goto cleanup;

        inStride = calc_stride1(ih->biWidth);
        if (safe_mul_size((size_t)inStride, (size_t)height, &totalSize) != 0) goto cleanup;

        raw1 = safe_malloc_zero(totalSize);
        if (!raw1) goto cleanup;
        if (fread(raw1, 1, totalSize, fp) != totalSize) { 
            free(raw1); goto cleanup; 
        }

        stride = calc_stride24(ih->biWidth);
        if (safe_mul_size((size_t)stride, (size_t)height, &totalSize) != 0) { 
            free(raw1); goto cleanup; 
        }

        out24 = safe_malloc_zero(totalSize);
        if (!out24) { 
            free(raw1); goto cleanup; 
        }

        /* 安全套用調色盤，防止反相圖變黑塊 */
        c0_B = palette[0]; c0_G = palette[1]; c0_R = palette[2];
        c1_B = palette[4]; c1_G = palette[5]; c1_R = palette[6];

        for (y = 0; y < height; y++) {
            for (x = 0; x < ih->biWidth; x++) {
                byteIdx = x / 8;
                bitIdx = 7 - (x % 8);
                val = (raw1[y * inStride + byteIdx] >> bitIdx) & 1;

                if (val) {
                    out24[y * stride + x * 3 + 0] = c1_B;
                    out24[y * stride + x * 3 + 1] = c1_G;
                    out24[y * stride + x * 3 + 2] = c1_R;
                } else {
                    out24[y * stride + x * 3 + 0] = c0_B;
                    out24[y * stride + x * 3 + 1] = c0_G;
                    out24[y * stride + x * 3 + 2] = c0_R;
                }
            }
        }
        free(raw1);
        data = out24;
        
        /* 徹底更新 Header 欺騙後續系統這是一張完美的 24-bit 圖 */
        ih->biBitCount = 24;
        ih->biSizeImage = stride * height;
        fh->bfOffBits = sizeof(BMPFILEHEADER) + sizeof(BMPINFOHEADER);
        fh->bfSize = fh->bfOffBits + ih->biSizeImage;
    }

cleanup:
    if (fp) fclose(fp);
    return data;
}

/* ==============================================================
 * 最終整合：萬用合併縮放模組 (支援等比例縮放)
 * ============================================================== */
int inUtility_SYSFIN_MergeAndScaleMultipleBMPUniversal(const MergeBMPItem* itemList, int itemCount, const char* outFile, int outBitCount) {
    BMPFILEHEADER* fhArray;
    BMPINFOHEADER* ihArray;
    unsigned char** dataArray;
    int outWidth = 0;
    int outHeight = 0;
    int successCount = 0;
    int currentOutRow = 0;
    int i, y, firstValid;
    int outRowPadding, outRowSize;
    size_t outDataSize;
    unsigned char* outData;
    BMPFILEHEADER outFh;
    BMPINFOHEADER outIh;
    FILE* outFp;

    if (itemCount <= 0 || !itemList || (outBitCount != 1 && outBitCount != 24)) return -1;

    fhArray = (BMPFILEHEADER*)safe_malloc_zero(sizeof(BMPFILEHEADER) * itemCount);
    ihArray = (BMPINFOHEADER*)safe_malloc_zero(sizeof(BMPINFOHEADER) * itemCount);
    dataArray = (unsigned char**)safe_malloc_zero(sizeof(unsigned char*) * itemCount);
    
    if (!fhArray || !ihArray || !dataArray) {
        if (fhArray) free(fhArray);
        if (ihArray) free(ihArray);
        if (dataArray) free(dataArray);
        return -1;
    }

    for (i = 0; i < itemCount; i++) {
        unsigned char* originalData = pszUtility_SYSFIN_ReadBMP_AutoNormalized24(itemList[i].filename, &fhArray[i], &ihArray[i]);
        if (originalData) {
            int originW = ihArray[i].biWidth;
            int originH = abs(ihArray[i].biHeight);
            
            /* 智慧等比例縮放邏輯 */
            int finalW = originW;
            int finalH = originH;

            if (itemList[i].targetWidth > 0 && itemList[i].targetHeight > 0) {
                finalW = itemList[i].targetWidth;
                finalH = itemList[i].targetHeight;
            } else if (itemList[i].targetWidth > 0 && itemList[i].targetHeight == 0) {
                finalW = itemList[i].targetWidth;
                finalH = (originH * finalW) / originW;
            } else if (itemList[i].targetHeight > 0 && itemList[i].targetWidth == 0) {
                finalH = itemList[i].targetHeight;
                finalW = (originW * finalH) / originH;
            }

            if (finalW != originW || finalH != originH) {
                unsigned char* resizedData = pszUtility_SYSFIN_ResizeBMP24Data(originalData, originW, originH, finalW, finalH);
                if (resizedData) {
                    free(originalData);        
                    originalData = resizedData; 
                    ihArray[i].biWidth = finalW;
                    ihArray[i].biHeight = finalH;
                }
            }
            
            dataArray[i] = originalData;
            outHeight += abs(ihArray[i].biHeight);
            if (ihArray[i].biWidth > outWidth) {
                outWidth = ihArray[i].biWidth;
            }
            successCount++;
        }
    }

    if (successCount == 0 || outHeight > MAX_BMP_HEIGHT || outWidth > MAX_BMP_WIDTH) {
        for (i = 0; i < itemCount; i++) if (dataArray[i]) free(dataArray[i]);
        free(fhArray); free(ihArray); free(dataArray);
        return -1;
    }

    outRowPadding = (4 - (outWidth * 3) % 4) % 4;
    outRowSize = outWidth * 3 + outRowPadding;
    
    if (safe_mul_size((size_t)outRowSize, (size_t)outHeight, &outDataSize) != 0) {
        for (i = 0; i < itemCount; i++) if (dataArray[i]) free(dataArray[i]);
        free(fhArray); free(ihArray); free(dataArray);
        return -1;
    }

    outData = safe_malloc_zero(outDataSize);
    if (!outData) {
        for (i = 0; i < itemCount; i++) if (dataArray[i]) free(dataArray[i]);
        free(fhArray); free(ihArray); free(dataArray);
        return -1;
    }
    memset(outData, 0xFF, outDataSize);

    /* 從最下面的一張開始往上畫圖 */
    for (i = itemCount - 1; i >= 0; i--) {
        int rowSize, pad, stride;
        if (!dataArray[i]) continue; 
        rowSize = ihArray[i].biWidth * 3;
        pad = (4 - rowSize % 4) % 4;
        stride = rowSize + pad;
        for (y = 0; y < abs(ihArray[i].biHeight); ++y) {
            memcpy(outData + currentOutRow * outRowSize, dataArray[i] + y * stride, rowSize);
            currentOutRow++;
        }
    }

    firstValid = -1;
    for (i = 0; i < itemCount; i++) {
        if (dataArray[i]) { firstValid = i; break; }
    }
    outFh = fhArray[firstValid];
    outIh = ihArray[firstValid];
    outIh.biWidth = outWidth;
    outIh.biHeight = outHeight; 

    outFp = fopen(outFile, "wb");
    if (outFp) {
        if (outBitCount == 1) {
            unsigned char defaultPalette[8] = { 0x00,0x00,0x00,0x00, 0xFF,0xFF,0xFF,0x00 };
            unsigned char* final1BitData = pszUtility_SYSFIN_Convert24BitTo1Bit(outData, outWidth, outHeight);
            
            if (final1BitData) {
                outIh.biBitCount = 1;
                outIh.biClrUsed = 2;
                outIh.biClrImportant = 2;
                outIh.biSizeImage = (((outWidth + 31) / 32) * 4) * outHeight;
                
                outFh.bfOffBits = sizeof(BMPFILEHEADER) + sizeof(BMPINFOHEADER) + 8;
                outFh.bfSize = outFh.bfOffBits + outIh.biSizeImage;

                fwrite(&outFh, sizeof(BMPFILEHEADER), 1, outFp);
                fwrite(&outIh, sizeof(BMPINFOHEADER), 1, outFp);
                fwrite(defaultPalette, 1, 8, outFp);
                fwrite(final1BitData, 1, outIh.biSizeImage, outFp);
                free(final1BitData);
            }
        } else {
            outIh.biBitCount = 24;
            outIh.biClrUsed = 0;
            outIh.biClrImportant = 0;
            outIh.biSizeImage = outDataSize;
            outFh.bfOffBits = sizeof(BMPFILEHEADER) + sizeof(BMPINFOHEADER);
            outFh.bfSize = outFh.bfOffBits + outDataSize;

            fwrite(&outFh, sizeof(BMPFILEHEADER), 1, outFp);
            fwrite(&outIh, sizeof(BMPINFOHEADER), 1, outFp);
            fwrite(outData, 1, outDataSize, outFp);
        }
        
        fflush(outFp);
        fsync(fileno(outFp));
        fclose(outFp);
    }

    for (i = 0; i < itemCount; i++) {
        if (dataArray[i]) free(dataArray[i]);
    }
    free(dataArray); free(fhArray); free(ihArray); free(outData);

    return (outFp != NULL) ? 0 : -1;
}

