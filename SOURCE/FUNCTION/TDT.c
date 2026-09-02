#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sqlite3.h>
#include <ctosapi.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/AllStruct.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "File.h"
#include "Function.h"
#include "Sqlite.h"
#include "EDC_Para_Table_Func.h"
#include "Utility.h"
#include "TDT.h"

static  TDT_REC	srTDTRec;	/* construct TDT record */
extern  int	ginDebug;       /* Debug使用 extern */
extern	int	ginFindRunTime;
extern	char	gszParamDBPath[];

SQLITE_TAG_TABLE TABLE_TDT_TAG[] = 
{
	{"inTableID"			,"INTEGER"	,"PRIMARY KEY"	,""},	/* Table ID Primary key, sqlite table專用避免PRIMARY KEY重複 */
	{"szTicket_HostIndex"		,"BLOB"		,""		,""},
	{"szTicket_HostName"		,"BLOB"		,""		,""},
	{"szTicket_HostTransFunc"	,"BLOB"		,""		,""},
	{"szTicket_HostEnable"		,"BLOB"		,""		,""},
	{"szTicket_LogOnOK"		,"BLOB"		,""		,""},
	{"szTicket_SAM_Slot"		,"BLOB"		,""		,""},
	{"szTicket_ReaderID"		,"BLOB"		,""		,""},/* For IPASS */
	{"szTicket_STAN"		,"BLOB"		,""		,""},/* FOR ECC */
	{"szTicket_LastTransDate"	,"BLOB"		,""		,""},/* FOR ECC 上次交易日期 YYYYMMDD */
	{"szTicket_LastRRN"		,"BLOB"		,""		,""},/* FOR ECC 上次交易RRN */
	{"szTicket_Device1"		,"BLOB"		,""		,""},/* FOR ECC 一代設備編號 */
	{"szTicket_Device2"		,"BLOB"		,""		,""},/* FOR ECC 二代設備編號 */
	{"szTicket_Batch"		,"BLOB"		,""		,""},/* FOR ECC 批次號碼之規則為yymmddxx(年月日(6) + 流水號(2)) */
	{"szTicket_NeedNewBatch"	,"BLOB"		,""		,""},/* FOR ECC 是否要更新批號(Y/N) */
	{"szTicket_Device3"		,"BLOB"		,""		,""},/* FOR ECC 悠遊卡Dongle Device ID */
	{"szTicket_ReversalBit"		,"BLOB"		,""		,""},/* 各票證是否要送Reversal */
	{"szTicket_LastAutoSignOnDate"	,"BLOB"		,""		,""},/* 最後一次自動SignOn日期，用來分辨本日是否已自動SignOn過 */
	{"szTicket_PIN"			,"BLOB"		,""		,""},/* For ICASH */
	{"szTicket_API_Version"		,"BLOB"		,""		,""},/* API Version */
	{""},
};

/*
Function        :inLoadTDTRec
Date&Time       :2019/6/10 下午 3:29
Describe        :優化存參數部份
*/
int inLoadTDTRec(int inTDTRec)
{
	int	inRetVal = VS_ERROR;
	
	if (_PARAMETER_SAVE_WAY_DAT_ == _PARAMETER_SAVE_WAY_DAT_SQLITE_)
	{
		inRetVal = inLoadTDTRec_SQLite(inTDTRec);
	}
	else
	{
		inRetVal = inLoadTDTRec_CTOS(inTDTRec);
	}

        return (inRetVal);
}

/*
Function        :inLoadTDTRec_CTOS
Date&Time       :2017/12/18 上午 9:54
Describe        :讀TDT檔案，inTDTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadTDTRec_CTOS(int inTDTRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆TDT檔案 */
        char            szTDTRec[_SIZE_TDT_REC_ + 1];		/* 暫存, 放各個欄位檔案 */
        char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnTDTLength = 0;			/* TDT總長度 */
        long            lnReadLength;                           /* 記錄每次要從TDT.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;	                /* inRec記錄讀到第幾筆, i為目前從TDT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */

        /* inLoadTDTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadTDTRec_CTOS(%d) START!!", inTDTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 判斷傳進來的inTDTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inTDTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inTDTRec < 0:(index = %d) ERROR!!", inTDTRec);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open TDT.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_TDT_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnTDTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_TDT_FILE_NAME_);

        if (lnTDTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnTDTLength + 1);
        uszTemp = malloc(lnTDTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnTDTLength + 1);
        memset(uszTemp, 0x00, lnTDTLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnTDTLength;

                for (i = 0;; ++i)
                {
                        /* 剩餘長度大於或等於1024 */
                        if (lnReadLength >= 1024)
                        {
                                if (inFILE_Read(&ulFile_Handle, &uszTemp[1024 * i], 1024) == VS_SUCCESS)
                                {
                                        /* 一次讀1024 */
                                        lnReadLength -= 1024;

                                        /* 當剩餘長度剛好為1024，會剛好讀完 */
                                        if (lnReadLength == 0)
                                                break;
                                }
                                /* 讀失敗時 */
                                else
                                {
                                        /* Close檔案 */
                                        inFILE_Close(&ulFile_Handle);

                                        /* Free pointer */
                                        free(uszReadData);
                                        free(uszTemp);

                                        return (VS_ERROR);
                                }
                        }
                        /* 剩餘長度小於1024 */
                        else if (lnReadLength < 1024)
                        {
                                /* 就只讀剩餘長度 */
                                if (inFILE_Read(&ulFile_Handle, &uszTemp[1024 * i], lnReadLength) == VS_SUCCESS)
                                {
                                        break;
                                }
                                /* 讀失敗時 */
                                else
                                {
                                        /* Close檔案 */
                                        inFILE_Close(&ulFile_Handle);

                                        /* Free pointer */
                                        free(uszReadData);
                                        free(uszTemp);

                                        return (VS_ERROR);
                                }
                        }
                }/* end for loop */
        }
        /* seek不成功時 */
        else
        {
                /* 關檔並回傳 */
                inFILE_Close(&ulFile_Handle);
                /* Free pointer */
                free(uszReadData);
                free(uszTemp);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        /*
         *抓取所需要的那筆record
         *i為目前從TDT讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnTDTLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到TDT的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnTDTLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        /* 因為inTDT_Rec的index從0開始，所以inTDT_Rec要+1 */
                        if (inRec == (inTDTRec + 1))
                        {
                                break;
                        }

                        /* 為了跳過 0x0D 0x0A */
                        i = i + 2;
                        /* 每讀完一筆record，j就歸0 */
                        j = 0;
                }

                j ++;
        }

        /*
         * 如果沒有inTDTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inTDTRec + 1) || inSearchResult == -1)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "No data or Index ERROR");
                }

		/* 關檔 */
                inFILE_Close(&ulFile_Handle);

		/* Free pointer */
                free(uszReadData);
                free(uszTemp);

                return (VS_ERROR);
        }

        /* uszReadData沒抓到資料，關檔、釋放記憶體並return (VS_ERROR) */
        if (*uszReadData == 0x00)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "No specific data.");
                }

                /* 關檔 */
                inFILE_Close(&ulFile_Handle);

                /* Free pointer */
                free(uszReadData);
                free(uszTemp);

                return (VS_ERROR);
        }

        /* 結構初始化 */
        memset(&srTDTRec, 0x00, sizeof(srTDTRec));
        /*
         * 以下pattern為存入TDT_Rec
         * i為TDT的第幾個字元
         * 存入TDT_Rec
         */
        i = 0;


        /* 01_szTicket_HostIndex */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TDT unpack ERROR.");
                        }

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_HostIndex[0], &szTDTRec[0], k - 1);
        }

        /* 02_szTicket_HostName */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TDT unpack ERROR");
                        }

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_HostName[0], &szTDTRec[0], k - 1);
        }

        /* 03_szTicket_HostTransFunc */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TDT unpack ERROR");
                        }

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_HostTransFunc[0], &szTDTRec[0], k - 1);
        }

        /* 04_szTicket_HostEnable */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TDT unpack ERROR");
                        }

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_HostEnable[0], &szTDTRec[0], k - 1);
        }
	
	/* 05_szTicket_LogOnOK */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TDT unpack ERROR");
                        }

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_LogOnOK[0], &szTDTRec[0], k - 1);
        }
	
	/* 06_szTicket_SAM_Slot */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TDT unpack ERROR");
                        }

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_SAM_Slot[0], &szTDTRec[0], k - 1);
        }
	
	/* 07_szTicket_ReaderID */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TDT unpack ERROR");
                        }

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_ReaderID[0], &szTDTRec[0], k - 1);
        }
	
	/* 08_szTicket_STAN */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TDT unpack ERROR");
                        }

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_STAN[0], &szTDTRec[0], k - 1);
        }
	
	/* 09_szTicket_LastTransDate */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TDT unpack ERROR");
                        }

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_LastTransDate[0], &szTDTRec[0], k - 1);
        }
	
	/* 10.szTicket_LastRRN */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TDT unpack ERROR");
                        }

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_LastRRN[0], &szTDTRec[0], k - 1);
        }
	
	/* 11_szTicket_Device1 */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TDT unpack ERROR");
                        }

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_Device1[0], &szTDTRec[0], k - 1);
        }
	
	/* 12_szTicket_Device2 */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TDT unpack ERROR");
                        }

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_Device2[0], &szTDTRec[0], k - 1);
        }
	
	/* 13_szTicket_Batch */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TDT unpack ERROR");
                        }

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_Batch[0], &szTDTRec[0], k - 1);
        }
	
	/* 14_szTicket_NeedNewBatch */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TDT unpack ERROR");
                        }

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_NeedNewBatch[0], &szTDTRec[0], k - 1);
        }
	
	/* 15_悠遊卡Dongle Device ID */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TDT unpack ERROR");
                        }

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_Device3[0], &szTDTRec[0], k - 1);
        }
	
	/* 16_szTicket_ReversalBit */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TDT unpack ERROR");
                        }

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_ReversalBit[0], &szTDTRec[0], k - 1);
        }
	
	/* 17_Ticket_LastAutoSignOnDate */
        /* 初始化 */
        memset(szTDTRec, 0x00, sizeof(szTDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTDTRec[k ++] = uszReadData[i ++];
                if (szTDTRec[k - 1] == 0x2C	||
		    szTDTRec[k - 1] == 0x0D	||
		    szTDTRec[k - 1] == 0x0A	||
		    szTDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TDT unpack ERROR");
                        }

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szTDTRec[0] != 0x2C	&&
	    szTDTRec[0] != 0x0D	&&
	    szTDTRec[0] != 0x0A	&&
	    szTDTRec[0] != 0x00)
        {
                memcpy(&srTDTRec.szTicket_LastAutoSignOnDate[0], &szTDTRec[0], k - 1);
        }
	
        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadTDTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadTDTRec_CTOS(%d) END!!", inTDTRec);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inLoadTDTRec_SQLite
Date&Time       :2019/6/10 下午 3:42
Describe        :讀TDT檔案，inTDT_Rec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadTDTRec_SQLite(int inTDTRec)
{
	int			inRetVal = VS_ERROR;
	char			szDebugMsg[100 + 1] = {0};
	SQLITE_ALL_TABLE	srAll = {0};

        /* inLoadTDTRec_SQLite()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                sprintf(szDebugMsg, "inLoadTDTRec_SQLite(%d) START!!", inTDTRec);
                inLogPrintf(AT, szDebugMsg);
        }

        /* 判斷傳進來的inTDTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inTDTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                        sprintf(szDebugMsg, "inTDTRec < 0:(index = %d) ERROR!!", inTDTRec);
                        inLogPrintf(AT, szDebugMsg);
                }

                return (VS_ERROR);
        }
	
	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inTDT_Table_Link_TDTRec(&srAll, _LS_READ_);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Link 失敗");
			inLogPrintf(AT, szDebugMsg);
		}
		inUtility_StoreTraceLog_OneStep("Table Link 失敗");
		
		return (VS_ERROR);
	}
	
	inRetVal = inSqlite_Get_Table_ByRecordID_All(gszParamDBPath, _TDT_TABLE_NAME_, inTDTRec, &srAll);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "db:%s,table:%s,Rec:%d get data failed", gszParamDBPath, _TDT_TABLE_NAME_, inTDTRec);
			inLogPrintf(AT, szDebugMsg);
		}
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "db:%s,table:%s,Rec:%d get data failed", gszParamDBPath, _TDT_TABLE_NAME_, inTDTRec);
		inUtility_StoreTraceLog_OneStep(szDebugMsg);
		
		return (VS_ERROR);
	}

        /* inLoadTDTRec_SQLite() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                sprintf(szDebugMsg, "inLoadTDTRec_SQLite(%d) END!!", inTDTRec);
                inLogPrintf(AT, szDebugMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveTDTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSaveTDTRec(int inTDTRec)
{
	int	inRetVal = VS_ERROR;
	
	if (_PARAMETER_SAVE_WAY_DAT_ == _PARAMETER_SAVE_WAY_DAT_SQLITE_)
	{
		inRetVal = inSaveTDTRec_SQLite(inTDTRec);
	}
	else
	{
		inRetVal = inSaveTDTRec_CTOS(inTDTRec);
	}
	
	if (inRetVal != VS_SUCCESS)
	{
		inFunc_EDCLock(AT);
	}

        return (inRetVal);
}

/*
Function        :inSaveTDTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSaveTDTRec_CTOS(int inTDTRec)
{
        unsigned long   uldat_Handle;                           /* FILE Handle */
        unsigned long   ulbak_Handle;                           /* FILE Handle */
        int	        inPackCount = 0;                        /* uszWriteBuff_Record的index */
        int	        inRecIndex = 0;                         /* uszRead_Total_Buff的index */
        int             inTDT_Total_Rec = 0;			/* TDT.dat的總筆數 */
        int             inRetVal;
        int             i = 0, j = 0;
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long	        lnReadLength = 0;                       /* 每次要從TDT.dat讀多少byte出來 */
        long            lnTDTLength = 0;			/* TDT.dat檔案總長度 */
        unsigned char	*uszRead_Total_Buff;
        unsigned char   *uszWriteBuff_Record, *uszWriteBuff_Org;


        /* inSaveTDTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveTDTRec_CTOS(%d)_START!!", inTDTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 防呆先刪除TDT.bak */
        inFILE_Delete((unsigned char *)_TDT_FILE_NAME_BAK_);

        /* 新建TDT.bak檔案 */
        inRetVal = inFILE_Create(&ulbak_Handle, (unsigned char *)_TDT_FILE_NAME_BAK_);

        if (inRetVal != VS_SUCCESS)
        {
                inFILE_Close(&ulbak_Handle);

                return (VS_ERROR);
        }

        /* 開啟原檔案TDT.dat */
        inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_TDT_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                /* TDT.dat開檔失敗 ，不用關檔TDT.dat */
                /* TDT.bak仍要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_TDT_FILE_NAME_BAK_);

                return(VS_ERROR);
        }

        /* 取得TDT.dat檔案大小 */
        lnTDTLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *)_TDT_FILE_NAME_);

        /* 取得檔案大小失敗 */
        if (lnTDTLength == VS_ERROR)
        {
                /* TDT.bak和TDT.dat要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Close(&uldat_Handle);
                inFILE_Delete((unsigned char *)_TDT_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        uszWriteBuff_Record = malloc(_SIZE_TDT_REC_ + _SIZE_TDT_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_TDT_REC_ + _SIZE_TDT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原TDT.dat的每筆Record 初始化 */
        uszWriteBuff_Org = malloc(_SIZE_TDT_REC_ + _SIZE_TDT_COMMA_0D0A_);
        memset(uszWriteBuff_Org, 0x00, _SIZE_TDT_REC_ + _SIZE_TDT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存TDT.dat全部資料 */
        uszRead_Total_Buff = malloc(lnTDTLength + 1);
        memset(uszRead_Total_Buff, 0x00, lnTDTLength + 1);

        inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

        /* szTicket_HostIndex */
        memcpy(&uszWriteBuff_Record[0], &srTDTRec.szTicket_HostIndex[0], strlen(srTDTRec.szTicket_HostIndex));
        inPackCount += strlen(srTDTRec.szTicket_HostIndex);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* szTicket_HostName */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTDTRec.szTicket_HostName[0], strlen(srTDTRec.szTicket_HostName));
        inPackCount += strlen(srTDTRec.szTicket_HostName);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* szTicket_HostTransFunc */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTDTRec.szTicket_HostTransFunc[0], strlen(srTDTRec.szTicket_HostTransFunc));
        inPackCount += strlen(srTDTRec.szTicket_HostTransFunc);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* szTicket_HostEnable */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTDTRec.szTicket_HostEnable[0], strlen(srTDTRec.szTicket_HostEnable));
        inPackCount += strlen(srTDTRec.szTicket_HostEnable);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szTicket_LogOnOK */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTDTRec.szTicket_LogOnOK[0], strlen(srTDTRec.szTicket_LogOnOK));
        inPackCount += strlen(srTDTRec.szTicket_LogOnOK);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szTicket_SAM_Slot */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTDTRec.szTicket_SAM_Slot[0], strlen(srTDTRec.szTicket_SAM_Slot));
        inPackCount += strlen(srTDTRec.szTicket_SAM_Slot);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szTicket_ReaderID */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTDTRec.szTicket_ReaderID[0], strlen(srTDTRec.szTicket_ReaderID));
        inPackCount += strlen(srTDTRec.szTicket_ReaderID);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szTicket_STAN */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTDTRec.szTicket_STAN[0], strlen(srTDTRec.szTicket_STAN));
        inPackCount += strlen(srTDTRec.szTicket_STAN);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szTicket_LastTransDate */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTDTRec.szTicket_LastTransDate[0], strlen(srTDTRec.szTicket_LastTransDate));
        inPackCount += strlen(srTDTRec.szTicket_LastTransDate);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szTicket_LastRRN */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTDTRec.szTicket_LastRRN[0], strlen(srTDTRec.szTicket_LastRRN));
        inPackCount += strlen(srTDTRec.szTicket_LastRRN);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szTicket_Device1 */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTDTRec.szTicket_Device1[0], strlen(srTDTRec.szTicket_Device1));
        inPackCount += strlen(srTDTRec.szTicket_Device1);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szTicket_Device2 */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTDTRec.szTicket_Device2[0], strlen(srTDTRec.szTicket_Device2));
        inPackCount += strlen(srTDTRec.szTicket_Device2);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szTicket_Batch */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTDTRec.szTicket_Batch[0], strlen(srTDTRec.szTicket_Batch));
        inPackCount += strlen(srTDTRec.szTicket_Batch);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szTicket_NeedNewBatch */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTDTRec.szTicket_NeedNewBatch[0], strlen(srTDTRec.szTicket_NeedNewBatch));
        inPackCount += strlen(srTDTRec.szTicket_NeedNewBatch);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* 悠遊卡Dongle Device ID */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTDTRec.szTicket_Device3[0], strlen(srTDTRec.szTicket_Device3));
        inPackCount += strlen(srTDTRec.szTicket_Device3);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szTicket_ReversalBit */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTDTRec.szTicket_ReversalBit[0], strlen(srTDTRec.szTicket_ReversalBit));
        inPackCount += strlen(srTDTRec.szTicket_ReversalBit);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
        /* szTicket_LastAutoSignOnDate */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTDTRec.szTicket_LastAutoSignOnDate[0], strlen(srTDTRec.szTicket_LastAutoSignOnDate));
        inPackCount += strlen(srTDTRec.szTicket_LastAutoSignOnDate);

        /* 最後的data不用逗號 */
        /* 補上換行符號 */

        /* 0D是移至行首 */
        uszWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        uszWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;

        /* 讀TDT.dat檔案存到uszRead_Total_Buff */
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnTDTLength;

                for (i = 0;; ++i)
                {
                        /* 剩餘長度大於或等於1024 */
                        if (lnReadLength >= 1024)
                        {
                                if (inFILE_Read(&uldat_Handle, &uszRead_Total_Buff[1024 * i], 1024) == VS_SUCCESS)
                                {
                                        /* 一次讀1024 */
                                        lnReadLength -= 1024;

                                        /* 當剩餘長度剛好為1024，會剛好讀完 */
                                        if (lnReadLength == 0)
                                                break;
                                }
                                /* 讀失敗時 */
                                else
                                {
                                        /* Close檔案 */
                                        inFILE_Close(&uldat_Handle);
                                        inFILE_Close(&ulbak_Handle);
                                        inFILE_Delete((unsigned char *)_TDT_FILE_NAME_BAK_);

                                        /* Free pointer */
                                        free(uszRead_Total_Buff);
                                        free(uszWriteBuff_Record);
                                        free(uszWriteBuff_Org);

                                        return (VS_ERROR);
                                }
                        }
                        /* 剩餘長度小於1024 */
                        else if (lnReadLength < 1024)
                        {
                                /* 就只讀剩餘長度 */
                                if (inFILE_Read(&uldat_Handle, &uszRead_Total_Buff[1024 * i], lnReadLength) == VS_SUCCESS)
                                {
                                        break;
                                }
                                /* 讀失敗時 */
                                else
                                {
                                        /* Close檔案 */
                                        inFILE_Close(&uldat_Handle);
                                        inFILE_Close(&ulbak_Handle);
                                        inFILE_Delete((unsigned char *)_TDT_FILE_NAME_BAK_);

                                        /* Free pointer */
                                        free(uszRead_Total_Buff);
                                        free(uszWriteBuff_Record);
                                        free(uszWriteBuff_Org);

                                        return (VS_ERROR);
                                }
                        }
                } /* end for loop */
        }
        else
        {
                /* Close檔案 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_TDT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        /* 防呆 總record數量小於要存取inTDTRec Return ERROR */
        /* 算總Record數 */
        for (i = 0; i < (lnTDTLength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (uszRead_Total_Buff[i] == 0x0A)
                        inTDT_Total_Rec++;
        }

        /* 防呆 總record數量小於要存取inTDTRec Return ERROR */
        if ((inTDTRec + 1) > inTDT_Total_Rec)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "No data or Index ERROR");
                }

                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_TDT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                return (VS_ERROR);
        }

        /* 記錄ReadTotalBuffer的陣列位置 */
        inRecIndex = 0;
        /* 用inTDTRec決定要先存幾筆Record到TDT.bak，ex:inTDTRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inTDTRec; i ++)
        {
                j = 0;

                while (1)
                {
                        /* 存Record */
                        uszWriteBuff_Org[j] = uszRead_Total_Buff[inRecIndex];
                        j++;

                        /* 每讀完一筆record，就把baffer中record的資料寫入bak */
                        if (uszRead_Total_Buff[inRecIndex] == 0x0A)
                        {
                                /* 為防止寫入位置錯誤，先移動到bak的檔案結尾 */
                                inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

                                if (inRetVal != VS_SUCCESS)
                                {
                                        /* 關檔 */
                                        inFILE_Close(&uldat_Handle);
                                        inFILE_Close(&ulbak_Handle);
                                        inFILE_Delete((unsigned char *)_TDT_FILE_NAME_BAK_);

                                        /* Free pointer */
                                        free(uszWriteBuff_Record);
                                        free(uszWriteBuff_Org);
                                        free(uszRead_Total_Buff);

                                        return (VS_ERROR);
                                }

                                /* 寫入bak檔案 */
                                inRetVal = inFILE_Write(&ulbak_Handle, uszWriteBuff_Org, j);

                                if (inRetVal != VS_SUCCESS)
                                {
                                        /* 關檔 */
                                        inFILE_Close(&uldat_Handle);
                                        inFILE_Close(&ulbak_Handle);
                                        inFILE_Delete((unsigned char *)_TDT_FILE_NAME_BAK_);

                                        /* Free pointer */
                                        free(uszWriteBuff_Record);
                                        free(uszWriteBuff_Org);
                                        free(uszRead_Total_Buff);

                                        return (VS_ERROR);
                                }

                                /* 清空buffer為下一筆寫入做準備 */
                                memset(uszWriteBuff_Org, 0x00, sizeof(uszWriteBuff_Org));
                                inRecIndex++;

                                break;
                        }

                        /* 讀下一個字元 */
                        inRecIndex++;
                }
        }

        /* 存組好的該TDTRecord 到 TDT.bak */
        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_TDT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 將組好的record寫入bak檔 */
        inRetVal = inFILE_Write(&ulbak_Handle, uszWriteBuff_Record, inPackCount);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_TDT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原TDT.dat Record 到 TDT.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
        /* inRecIndex要指到剩下還沒存進bak檔的第一個記憶體位置 */
        inTDTRec = inTDTRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < lnTDTLength; i++)
        {
                if (uszRead_Total_Buff[i] == 0x0A)
                {
                        j++;

                        if (j == inTDTRec)
                        {
                                inRecIndex = i + 1;
                                break;
                        }
                }
        }

        i = 0;
        memset(uszWriteBuff_Org, 0x00, sizeof(uszWriteBuff_Org));

        while (1)
        {
                /* 接續存原TDT.dat的Record */
                uszWriteBuff_Org[i] = uszRead_Total_Buff[inRecIndex];
                i++;

                if (uszRead_Total_Buff[inRecIndex] == 0x0A)
                {
                        /* 移動到bak的檔案結尾 */
                        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

                        if (inRetVal != VS_SUCCESS)
                        {
                                /* 關檔 */
                                inFILE_Close(&uldat_Handle);
                                inFILE_Close(&ulbak_Handle);
                                inFILE_Delete((unsigned char *)_TDT_FILE_NAME_BAK_);

                                /* Free pointer */
                                free(uszWriteBuff_Record);
                                free(uszWriteBuff_Org);
                                free(uszRead_Total_Buff);

                                return (VS_ERROR);
                        }

                        inRetVal = inFILE_Write(&ulbak_Handle, uszWriteBuff_Org, i);

                        if (inRetVal != VS_SUCCESS)
                        {
                                /* 關檔 */
                                inFILE_Close(&uldat_Handle);
                                inFILE_Close(&ulbak_Handle);
                                inFILE_Delete((unsigned char *)_TDT_FILE_NAME_BAK_);

                                /* Free pointer */
                                free(uszWriteBuff_Record);
                                free(uszWriteBuff_Org);
                                free(uszRead_Total_Buff);

                                return (VS_ERROR);
                        }

                        memset(uszWriteBuff_Org, 0x00, sizeof(uszWriteBuff_Org));
                        i = 0;
                }
                else if (uszRead_Total_Buff[inRecIndex] == 0x00)
                {
                        /* 讀到0x00結束 */
                        break;
                }

                inRecIndex++;
        }

        /* 關檔 */
        inFILE_Close(&uldat_Handle);
        inFILE_Close(&ulbak_Handle);

        /* Free pointer */
        free(uszWriteBuff_Record);
        free(uszWriteBuff_Org);
        free(uszRead_Total_Buff);

        /* 刪除原TDT.dat */
        if (inFILE_Delete((unsigned char *)_TDT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* 將TDT.bak改名字為TDT.dat取代原檔案 */
        if (inFILE_Rename((unsigned char *)_TDT_FILE_NAME_BAK_, (unsigned char *)_TDT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* inSaveTDTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveTDTRec_CTOS(%d) END!!", inTDTRec - 1);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return(VS_SUCCESS);
}

/*
Function        :inSaveTDTRec_SQLite
Date&Time       :2019/6/10 下午 2:59
Describe        :寫入TDT.dat，inTDTRec是要讀哪一筆的紀錄，第一筆為0
 *		 改用SQLite
*/
int inSaveTDTRec_SQLite(int inTDTRec)
{
	int inRetVal = VS_ERROR;
	char szDebugMsg[100 + 1] = {0};
	SQLITE_ALL_TABLE srAll = {0};

	/* inSaveTDTRec_Linux()_START */
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inSaveTDTRec_SQLite(%d) START!!", inTDTRec);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inTDT_Table_Link_TDTRec(&srAll, _LS_UPDATE_);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Link 失敗");
			inLogPrintf(AT, szDebugMsg);
		}
		inUtility_StoreTraceLog_OneStep("Table Link 失敗");
		
		return (VS_ERROR);
	}
	
	inRetVal = inSqlite_Insert_Or_Replace_ByRecordID_All(gszParamDBPath, _TDT_TABLE_NAME_, inTDTRec, &srAll);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "db:%s,table:%s,Rec:%d update failed", gszParamDBPath, _TDT_TABLE_NAME_, inTDTRec);
			inLogPrintf(AT, szDebugMsg);
		}
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "db:%s,table:%s,Rec:%d update failed", gszParamDBPath, _TDT_TABLE_NAME_, inTDTRec);
		inUtility_StoreTraceLog_OneStep(szDebugMsg);
		
		return (VS_ERROR);
	}

	/*[260318_BUG] 因使用原廠API不需再進行同步,移除檔案同步函式 */
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	/* inSaveTDTRec_Linux() END */
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inSaveTDTRec_SQLite(%d) END!!", inTDTRec);
		inLogPrintf(AT, szDebugMsg);
		inLogPrintf(AT, "----------------------------------------");
	}

	return(VS_SUCCESS);
}

/*
Function        :inTDT_Table_Link_TDTRec
Date&Time       :2019/4/24 上午 11:00
Describe        :將FunctionPointer放至對應位置
*/
int inTDT_Table_Link_TDTRec(SQLITE_ALL_TABLE *srAll, int inLinkState)
{
	SQLITE_INT32T_TABLE TABLE_BATCH_INT[] = 
	{
		{0	,"inTableID"			,&srTDTRec.inRecordRowID							},	/* inTableID */
		{0	,""				,NULL										}	/* 這行用Null用來知道尾端在哪 */	
	};
	
	SQLITE_INT32T_TABLE TABLE_BATCH_INT64T[] = 
	{
		{0	,""				,NULL										}	/* 這行用Null用來知道尾端在哪 */
	};
	
	SQLITE_CHAR_TABLE TABLE_BATCH_CHAR[] =
	{
		{0	,""				,NULL				,0						}	/* 這行用Null用來知道尾端在哪 */
	};
	
	SQLITE_CHAR_TABLE TABLE_BATCH_TEXT[] =
	{
		
		{0	,"szTicket_HostIndex"		,&srTDTRec.szTicket_HostIndex		,strlen(srTDTRec.szTicket_HostIndex)		},
		{0	,"szTicket_HostName"		,&srTDTRec.szTicket_HostName		,strlen(srTDTRec.szTicket_HostName)		},
		{0	,"szTicket_HostTransFunc"	,&srTDTRec.szTicket_HostTransFunc	,strlen(srTDTRec.szTicket_HostTransFunc)	},
		{0	,"szTicket_HostEnable"		,&srTDTRec.szTicket_HostEnable		,strlen(srTDTRec.szTicket_HostEnable)		},
		{0	,"szTicket_LogOnOK"		,&srTDTRec.szTicket_LogOnOK		,strlen(srTDTRec.szTicket_LogOnOK)		},
		{0	,"szTicket_SAM_Slot"		,&srTDTRec.szTicket_SAM_Slot		,strlen(srTDTRec.szTicket_SAM_Slot)		},
		{0	,"szTicket_ReaderID"		,&srTDTRec.szTicket_ReaderID		,strlen(srTDTRec.szTicket_ReaderID)		},/* For IPASS */
		{0	,"szTicket_STAN"		,&srTDTRec.szTicket_STAN		,strlen(srTDTRec.szTicket_STAN)			},/* FOR ECC */
		{0	,"szTicket_LastTransDate"	,&srTDTRec.szTicket_LastTransDate	,strlen(srTDTRec.szTicket_LastTransDate)	},/* FOR ECC 上次交易日期 YYYYMMDD */
		{0	,"szTicket_LastRRN"		,&srTDTRec.szTicket_LastRRN		,strlen(srTDTRec.szTicket_LastRRN)		},/* FOR ECC 上次交易RRN */
		{0	,"szTicket_Device1"		,&srTDTRec.szTicket_Device1		,strlen(srTDTRec.szTicket_Device1)		},/* FOR ECC 一代設備編號 */
		{0	,"szTicket_Device2"		,&srTDTRec.szTicket_Device2		,strlen(srTDTRec.szTicket_Device2)		},/* FOR ECC 二代設備編號 */
		{0	,"szTicket_Batch"		,&srTDTRec.szTicket_Batch		,strlen(srTDTRec.szTicket_Batch)		},/* FOR ECC 批次號碼之規則為yymmddxx(年月日(6) + 流水號(2)) */
		{0	,"szTicket_NeedNewBatch"	,&srTDTRec.szTicket_NeedNewBatch	,strlen(srTDTRec.szTicket_NeedNewBatch)		},/* FOR ECC 是否要更新批號(Y/N) */
		{0	,"szTicket_Device3"		,&srTDTRec.szTicket_Device3		,strlen(srTDTRec.szTicket_Device3)		},/* FOR ECC 悠遊卡Dongle Device ID */
		{0	,"szTicket_ReversalBit"		,&srTDTRec.szTicket_ReversalBit		,strlen(srTDTRec.szTicket_ReversalBit)		},/* 各票證是否要送Reversal */
		{0	,"szTicket_LastAutoSignOnDate"	,&srTDTRec.szTicket_LastAutoSignOnDate	,strlen(srTDTRec.szTicket_LastAutoSignOnDate)	},/* 最後一次自動SignOn日期，用來分辨本日是否已自動SignOn過 */
		{0	,"szTicket_PIN"			,&srTDTRec.szTicket_PIN			,strlen(srTDTRec.szTicket_PIN)			},/* For ICASH */
		{0	,"szTicket_API_Version"		,&srTDTRec.szTicket_API_Version		,strlen(srTDTRec.szTicket_API_Version)		},/* 各票證的API版本 */
		{0	,""				,NULL					,0						}	/* 這行用Null用來知道尾端在哪 */
	};
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inTDT_Table_Link_TDTRec()_START");
        }
	
	SQLITE_LINK_TABLE	srLink;
	memset(&srLink, 0x00, sizeof(SQLITE_LINK_TABLE));
	srLink.psrInt = TABLE_BATCH_INT;
	srLink.psrInt64t = TABLE_BATCH_INT64T;
	srLink.psrChar = TABLE_BATCH_CHAR;
	srLink.psrText = TABLE_BATCH_TEXT;
	
	inSqlite_Table_Link(srAll, inLinkState, &srLink);
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inTDT_Table_Link_TDTRec()_END");
                inLogPrintf(AT, "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inTDT_Initial_AllRercord
Date&Time       :2019/4/26 下午 4:35
Describe        :若參數生效，初始化TDT
*/
int inTDT_Initial_AllRercord(char* szDBName, char* szTableName)
{
	int	i = 0;
	char	szTemplate[20 + 1] = {0};
	
	/* 資料庫先清Table再重建*/
	if (_PARAMETER_SAVE_WAY_DAT_ == _PARAMETER_SAVE_WAY_DAT_SQLITE_)
	{
		inSqlite_Drop_Table(szDBName, szTableName);
		inEDCPara_Create_Table_Flow(szDBName, szTableName, TABLE_TDT_TAG);
	}
	
	for (i = 0; ; i++)
	{
		/* 目前只有4個default record */
		if (i >= _TDT_INDEX_MAX_)
		{
			break;
		}
		
		inSetTicket_HostIndex("00");
		memset(szTemplate, 0x00, sizeof(szTemplate));
		switch (i)
		{
			case 0:
				strcpy(szTemplate, _HOST_NAME_IPASS_);
				break;
			case 1:
				strcpy(szTemplate, _HOST_NAME_ECC_);
				break;
			case 2:
				strcpy(szTemplate, _HOST_NAME_ICASH_);
				break;
			default:
				break;
		}
		inSetTicket_HostName(szTemplate);
		inSetTicket_HostTransFunc(" ");
		inSetTicket_HostEnable("N");
		inSetTicket_LogOnOK("N");
		inSetTicket_SAM_Slot("01");
		inSetTicket_ReaderID(" ");
		inSetTicket_STAN("000001");
		inSetTicket_LastTransDate("00000000");
		inSetTicket_LastRRN(" ");
		inSetTicket_Device1(" ");
		inSetTicket_Device2(" ");
		inSetTicket_Batch("00000000");
		inSetTicket_NeedNewBatch("Y");
		inSetTicket_Device3(" ");
		inSetTicket_ReversalBit("N");
		inSetTicket_LastAutoSignOnDate(" ");
		inSetTicket_PIN("0000");
		inSetTicket_API_Version("                    ");
		
		inSaveTDTRec(i);
	}
	
	return (VS_SUCCESS);
}

/*
set和get等價於相反的操作
各欄位的set和get function
*/

/*
Function        :inGetTicket_HostIndex
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inGetTicket_HostIndex(char* szTicket_HostIndex)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_HostIndex == NULL || strlen(srTDTRec.szTicket_HostIndex) <= 0 || strlen(srTDTRec.szTicket_HostIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTicket_HostIndex() ERROR !!");

			if (szTicket_HostIndex == NULL)
                        {
                                inLogPrintf(AT, "szTicket_HostIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_HostIndex length = (%d)", (int)strlen(srTDTRec.szTicket_HostIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_HostIndex[0], &srTDTRec.szTicket_HostIndex[0], strlen(srTDTRec.szTicket_HostIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_HostIndex
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inSetTicket_HostIndex(char* szTicket_HostIndex)
{
        memset(srTDTRec.szTicket_HostIndex, 0x00, sizeof(srTDTRec.szTicket_HostIndex));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_HostIndex == NULL || strlen(szTicket_HostIndex) <= 0 || strlen(szTicket_HostIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTicket_HostIndex() ERROR !!");
                        if (szTicket_HostIndex == NULL)
                        {
                                inLogPrintf(AT, "szTicket_HostIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_HostIndex length = (%d)", (int)strlen(szTicket_HostIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_HostIndex[0], &szTicket_HostIndex[0], strlen(szTicket_HostIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_HostName
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inGetTicket_HostName(char* szTicket_HostName)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_HostName == NULL || strlen(srTDTRec.szTicket_HostName) <= 0 || strlen(srTDTRec.szTicket_HostName) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTicket_HostName() ERROR !!");

			if (szTicket_HostName == NULL)
                        {
                                inLogPrintf(AT, "szTicket_HostName == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_HostName length = (%d)", (int)strlen(srTDTRec.szTicket_HostName));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_HostName[0], &srTDTRec.szTicket_HostName[0], strlen(srTDTRec.szTicket_HostName));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_HostName
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inSetTicket_HostName(char* szTicket_HostName)
{
        memset(srTDTRec.szTicket_HostName, 0x00, sizeof(srTDTRec.szTicket_HostName));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_HostName == NULL || strlen(szTicket_HostName) <= 0 || strlen(szTicket_HostName) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTicket_HostName() ERROR !!");
                        if (szTicket_HostName == NULL)
                        {
                                inLogPrintf(AT, "szTicket_HostName == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_HostName length = (%d)", (int)strlen(szTicket_HostName));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_HostName[0], &szTicket_HostName[0], strlen(szTicket_HostName));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_HostTransFunc
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inGetTicket_HostTransFunc(char* szTicket_HostTransFunc)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_HostTransFunc == NULL || strlen(srTDTRec.szTicket_HostTransFunc) <= 0 || strlen(srTDTRec.szTicket_HostTransFunc) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTicket_HostTransFunc() ERROR !!");

			if (szTicket_HostTransFunc == NULL)
                        {
                                inLogPrintf(AT, "szTicket_HostTransFunc == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_HostTransFunc length = (%d)", (int)strlen(srTDTRec.szTicket_HostTransFunc));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_HostTransFunc[0], &srTDTRec.szTicket_HostTransFunc[0], strlen(srTDTRec.szTicket_HostTransFunc));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_HostTransFunc
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inSetTicket_HostTransFunc(char* szTicket_HostTransFunc)
{
        memset(srTDTRec.szTicket_HostTransFunc, 0x00, sizeof(srTDTRec.szTicket_HostTransFunc));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_HostTransFunc == NULL || strlen(szTicket_HostTransFunc) <= 0 || strlen(szTicket_HostTransFunc) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTicket_HostTransFunc() ERROR !!");
                        if (szTicket_HostTransFunc == NULL)
                        {
                                inLogPrintf(AT, "szTicket_HostTransFunc == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_HostTransFunc length = (%d)", (int)strlen(szTicket_HostTransFunc));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_HostTransFunc[0], &szTicket_HostTransFunc[0], strlen(szTicket_HostTransFunc));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_HostEnable
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inGetTicket_HostEnable(char* szTicket_HostEnable)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_HostEnable == NULL || strlen(srTDTRec.szTicket_HostEnable) <= 0 || strlen(srTDTRec.szTicket_HostEnable) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTicket_HostEnable() ERROR !!");

			if (szTicket_HostEnable == NULL)
                        {
                                inLogPrintf(AT, "szTicket_HostEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_HostEnable length = (%d)", (int)strlen(srTDTRec.szTicket_HostEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_HostEnable[0], &srTDTRec.szTicket_HostEnable[0], strlen(srTDTRec.szTicket_HostEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_HostEnable
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inSetTicket_HostEnable(char* szTicket_HostEnable)
{
        memset(srTDTRec.szTicket_HostEnable, 0x00, sizeof(srTDTRec.szTicket_HostEnable));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_HostEnable == NULL || strlen(szTicket_HostEnable) <= 0 || strlen(szTicket_HostEnable) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTicket_HostEnable() ERROR !!");
                        if (szTicket_HostEnable == NULL)
                        {
                                inLogPrintf(AT, "szTicket_HostEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_HostEnable length = (%d)", (int)strlen(szTicket_HostEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_HostEnable[0], &szTicket_HostEnable[0], strlen(szTicket_HostEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_LogOnOK
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inGetTicket_LogOnOK(char* szTicket_LogOnOK)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_LogOnOK == NULL || strlen(srTDTRec.szTicket_LogOnOK) <= 0 || strlen(srTDTRec.szTicket_LogOnOK) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTicket_LogOnOK() ERROR !!");

			if (szTicket_LogOnOK == NULL)
                        {
                                inLogPrintf(AT, "szTicket_LogOnOK == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_LogOnOK length = (%d)", (int)strlen(srTDTRec.szTicket_LogOnOK));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_LogOnOK[0], &srTDTRec.szTicket_LogOnOK[0], strlen(srTDTRec.szTicket_LogOnOK));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_LogOnOK
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inSetTicket_LogOnOK(char* szTicket_LogOnOK)
{
        memset(srTDTRec.szTicket_LogOnOK, 0x00, sizeof(srTDTRec.szTicket_LogOnOK));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_LogOnOK == NULL || strlen(szTicket_LogOnOK) <= 0 || strlen(szTicket_LogOnOK) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTicket_LogOnOK() ERROR !!");
                        if (szTicket_LogOnOK == NULL)
                        {
                                inLogPrintf(AT, "szTicket_LogOnOK == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_LogOnOK length = (%d)", (int)strlen(szTicket_LogOnOK));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_LogOnOK[0], &szTicket_LogOnOK[0], strlen(szTicket_LogOnOK));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_SAM_Slot
Date&Time       :2018/1/4 下午 3:18
Describe        :
*/
int inGetTicket_SAM_Slot(char* szTicket_SAM_Slot)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_SAM_Slot == NULL || strlen(srTDTRec.szTicket_SAM_Slot) <= 0 || strlen(srTDTRec.szTicket_SAM_Slot) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTicket_SAM_Slot() ERROR !!");

			if (szTicket_SAM_Slot == NULL)
                        {
                                inLogPrintf(AT, "szTicket_SAM_Slot == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_SAM_Slot length = (%d)", (int)strlen(srTDTRec.szTicket_SAM_Slot));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_SAM_Slot[0], &srTDTRec.szTicket_SAM_Slot[0], strlen(srTDTRec.szTicket_SAM_Slot));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_SAM_Slot
Date&Time       :2018/1/4 下午 3:18
Describe        :
*/
int inSetTicket_SAM_Slot(char* szTicket_SAM_Slot)
{
        memset(srTDTRec.szTicket_SAM_Slot, 0x00, sizeof(srTDTRec.szTicket_SAM_Slot));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_SAM_Slot == NULL || strlen(szTicket_SAM_Slot) <= 0 || strlen(szTicket_SAM_Slot) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTicket_SAM_Slot() ERROR !!");
                        if (szTicket_SAM_Slot == NULL)
                        {
                                inLogPrintf(AT, "szTicket_SAM_Slot == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_SAM_Slot length = (%d)", (int)strlen(szTicket_SAM_Slot));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_SAM_Slot[0], &szTicket_SAM_Slot[0], strlen(szTicket_SAM_Slot));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_ReaderID
Date&Time       :2018/1/4 下午 3:19
Describe        :
*/
int inGetTicket_ReaderID(char* szTicket_ReaderID)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_ReaderID == NULL || strlen(srTDTRec.szTicket_ReaderID) <= 0 || strlen(srTDTRec.szTicket_ReaderID) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTicket_ReaderID() ERROR !!");

			if (szTicket_ReaderID == NULL)
                        {
                                inLogPrintf(AT, "szTicket_ReaderID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_ReaderID length = (%d)", (int)strlen(srTDTRec.szTicket_ReaderID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_ReaderID[0], &srTDTRec.szTicket_ReaderID[0], strlen(srTDTRec.szTicket_ReaderID));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_ReaderID
Date&Time       :2018/1/4 下午 3:20
Describe        :
*/
int inSetTicket_ReaderID(char* szTicket_ReaderID)
{
        memset(srTDTRec.szTicket_ReaderID, 0x00, sizeof(srTDTRec.szTicket_ReaderID));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_ReaderID == NULL || strlen(szTicket_ReaderID) <= 0 || strlen(szTicket_ReaderID) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTicket_ReaderID() ERROR !!");
                        if (szTicket_ReaderID == NULL)
                        {
                                inLogPrintf(AT, "szTicket_ReaderID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_ReaderID length = (%d)", (int)strlen(szTicket_ReaderID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_ReaderID[0], &szTicket_ReaderID[0], strlen(szTicket_ReaderID));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_STAN
Date&Time       :2018/3/22 下午 1:28
Describe        :
*/
int inGetTicket_STAN(char* szTicket_STAN)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_STAN == NULL || strlen(srTDTRec.szTicket_STAN) <= 0 || strlen(srTDTRec.szTicket_STAN) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTicket_STAN() ERROR !!");

			if (szTicket_STAN == NULL)
                        {
                                inLogPrintf(AT, "szTicket_STAN == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_STAN length = (%d)", (int)strlen(srTDTRec.szTicket_STAN));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_STAN[0], &srTDTRec.szTicket_STAN[0], strlen(srTDTRec.szTicket_STAN));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_STAN
Date&Time       :2018/3/22 下午 1:28
Describe        :
*/
int inSetTicket_STAN(char* szTicket_STAN)
{
        memset(srTDTRec.szTicket_STAN, 0x00, sizeof(srTDTRec.szTicket_STAN));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_STAN == NULL || strlen(szTicket_STAN) <= 0 || strlen(szTicket_STAN) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTicket_STAN() ERROR !!");
                        if (szTicket_STAN == NULL)
                        {
                                inLogPrintf(AT, "szTicket_STAN == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_STAN length = (%d)", (int)strlen(szTicket_STAN));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_STAN[0], &szTicket_STAN[0], strlen(szTicket_STAN));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_LastTransDate
Date&Time       :2018/3/22 下午 1:28
Describe        :
*/
int inGetTicket_LastTransDate(char* szTicket_LastTransDate)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_LastTransDate == NULL || strlen(srTDTRec.szTicket_LastTransDate) <= 0 || strlen(srTDTRec.szTicket_LastTransDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTicket_LastTransDate() ERROR !!");

			if (szTicket_LastTransDate == NULL)
                        {
                                inLogPrintf(AT, "szTicket_LastTransDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_LastTransDate length = (%d)", (int)strlen(srTDTRec.szTicket_LastTransDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_LastTransDate[0], &srTDTRec.szTicket_LastTransDate[0], strlen(srTDTRec.szTicket_LastTransDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_LastTransDate
Date&Time       :2018/3/22 下午 1:28
Describe        :
*/
int inSetTicket_LastTransDate(char* szTicket_LastTransDate)
{
        memset(srTDTRec.szTicket_LastTransDate, 0x00, sizeof(srTDTRec.szTicket_LastTransDate));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_LastTransDate == NULL || strlen(szTicket_LastTransDate) <= 0 || strlen(szTicket_LastTransDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTicket_LastTransDate() ERROR !!");
                        if (szTicket_LastTransDate == NULL)
                        {
                                inLogPrintf(AT, "szTicket_LastTransDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_LastTransDate length = (%d)", (int)strlen(szTicket_LastTransDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_LastTransDate[0], &szTicket_LastTransDate[0], strlen(szTicket_LastTransDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_LastRRN
Date&Time       :2018/3/28 下午 1:16
Describe        :
*/
int inGetTicket_LastRRN(char* szTicket_LastRRN)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_LastRRN == NULL || strlen(srTDTRec.szTicket_LastRRN) <= 0 || strlen(srTDTRec.szTicket_LastRRN) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTicket_LastRRN() ERROR !!");

			if (szTicket_LastRRN == NULL)
                        {
                                inLogPrintf(AT, "szTicket_LastRRN == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_LastRRN length = (%d)", (int)strlen(srTDTRec.szTicket_LastRRN));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_LastRRN[0], &srTDTRec.szTicket_LastRRN[0], strlen(srTDTRec.szTicket_LastRRN));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_LastRRN
Date&Time       :2018/3/28 下午 1:16
Describe        :
*/
int inSetTicket_LastRRN(char* szTicket_LastRRN)
{
        memset(srTDTRec.szTicket_LastRRN, 0x00, sizeof(srTDTRec.szTicket_LastRRN));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_LastRRN == NULL || strlen(szTicket_LastRRN) <= 0 || strlen(szTicket_LastRRN) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTicket_LastRRN() ERROR !!");
                        if (szTicket_LastRRN == NULL)
                        {
                                inLogPrintf(AT, "szTicket_LastRRN == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_LastRRN length = (%d)", (int)strlen(szTicket_LastRRN));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_LastRRN[0], &szTicket_LastRRN[0], strlen(szTicket_LastRRN));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_Device1
Date&Time       :2018/3/28 下午 1:16
Describe        :
*/
int inGetTicket_Device1(char* szTicket_Device1)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_Device1 == NULL || strlen(srTDTRec.szTicket_Device1) <= 0 || strlen(srTDTRec.szTicket_Device1) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTicket_Device1() ERROR !!");

			if (szTicket_Device1 == NULL)
                        {
                                inLogPrintf(AT, "szTicket_Device1 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_Device1 length = (%d)", (int)strlen(srTDTRec.szTicket_Device1));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_Device1[0], &srTDTRec.szTicket_Device1[0], strlen(srTDTRec.szTicket_Device1));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_Device1
Date&Time       :2018/3/28 下午 1:16
Describe        :
*/
int inSetTicket_Device1(char* szTicket_Device1)
{
        memset(srTDTRec.szTicket_Device1, 0x00, sizeof(srTDTRec.szTicket_Device1));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_Device1 == NULL || strlen(szTicket_Device1) <= 0 || strlen(szTicket_Device1) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTicket_Device1() ERROR !!");
                        if (szTicket_Device1 == NULL)
                        {
                                inLogPrintf(AT, "szTicket_Device1 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_Device1 length = (%d)", (int)strlen(szTicket_Device1));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_Device1[0], &szTicket_Device1[0], strlen(szTicket_Device1));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_Device2
Date&Time       :2018/3/28 下午 1:16
Describe        :
*/
int inGetTicket_Device2(char* szTicket_Device2)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_Device2 == NULL || strlen(srTDTRec.szTicket_Device2) <= 0 || strlen(srTDTRec.szTicket_Device2) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTicket_Device2() ERROR !!");

			if (szTicket_Device2 == NULL)
                        {
                                inLogPrintf(AT, "szTicket_Device2 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_Device2 length = (%d)", (int)strlen(srTDTRec.szTicket_Device2));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_Device2[0], &srTDTRec.szTicket_Device2[0], strlen(srTDTRec.szTicket_Device2));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_Device2
Date&Time       :2018/3/28 下午 1:16
Describe        :
*/
int inSetTicket_Device2(char* szTicket_Device2)
{
        memset(srTDTRec.szTicket_Device2, 0x00, sizeof(srTDTRec.szTicket_Device2));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_Device2 == NULL || strlen(szTicket_Device2) <= 0 || strlen(szTicket_Device2) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTicket_Device2() ERROR !!");
                        if (szTicket_Device2 == NULL)
                        {
                                inLogPrintf(AT, "szTicket_Device2 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_Device2 length = (%d)", (int)strlen(szTicket_Device2));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_Device2[0], &szTicket_Device2[0], strlen(szTicket_Device2));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_Batch
Date&Time       :2018/3/28 下午 1:16
Describe        :
*/
int inGetTicket_Batch(char* szTicket_Batch)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_Batch == NULL || strlen(srTDTRec.szTicket_Batch) <= 0 || strlen(srTDTRec.szTicket_Batch) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTicket_Batch() ERROR !!");

			if (szTicket_Batch == NULL)
                        {
                                inLogPrintf(AT, "szTicket_Batch == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_Batch length = (%d)", (int)strlen(srTDTRec.szTicket_Batch));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_Batch[0], &srTDTRec.szTicket_Batch[0], strlen(srTDTRec.szTicket_Batch));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_Batch
Date&Time       :2018/3/28 下午 1:16
Describe        :
*/
int inSetTicket_Batch(char* szTicket_Batch)
{
        memset(srTDTRec.szTicket_Batch, 0x00, sizeof(srTDTRec.szTicket_Batch));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_Batch == NULL || strlen(szTicket_Batch) <= 0 || strlen(szTicket_Batch) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTicket_Batch() ERROR !!");
                        if (szTicket_Batch == NULL)
                        {
                                inLogPrintf(AT, "szTicket_Batch == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_Batch length = (%d)", (int)strlen(szTicket_Batch));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_Batch[0], &szTicket_Batch[0], strlen(szTicket_Batch));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_FirstTxnDate
Date&Time       :2018/3/28 下午 1:16
Describe        :
*/
int inGetTicket_NeedNewBatch(char* szTicket_NeedNewBatch)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_NeedNewBatch == NULL || strlen(srTDTRec.szTicket_NeedNewBatch) <= 0 || strlen(srTDTRec.szTicket_NeedNewBatch) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTicket_FirstTxnDate() ERROR !!");

			if (szTicket_NeedNewBatch == NULL)
                        {
                                inLogPrintf(AT, "szTicket_FirstTxnDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_FirstTxnDate length = (%d)", (int)strlen(srTDTRec.szTicket_NeedNewBatch));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_NeedNewBatch[0], &srTDTRec.szTicket_NeedNewBatch[0], strlen(srTDTRec.szTicket_NeedNewBatch));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_FirstTxnDate
Date&Time       :2018/3/28 下午 1:16
Describe        :
*/
int inSetTicket_NeedNewBatch(char* szTicket_NeedNewBatch)
{
        memset(srTDTRec.szTicket_NeedNewBatch, 0x00, sizeof(srTDTRec.szTicket_NeedNewBatch));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_NeedNewBatch == NULL || strlen(szTicket_NeedNewBatch) <= 0 || strlen(szTicket_NeedNewBatch) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTicket_FirstTxnDate() ERROR !!");
                        if (szTicket_NeedNewBatch == NULL)
                        {
                                inLogPrintf(AT, "szTicket_FirstTxnDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_FirstTxnDate length = (%d)", (int)strlen(szTicket_NeedNewBatch));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_NeedNewBatch[0], &szTicket_NeedNewBatch[0], strlen(szTicket_NeedNewBatch));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_Device3
Date&Time       :2018/7/10 下午 3:44
Describe        :
*/
int inGetTicket_Device3(char* szTicket_Device3)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_Device3 == NULL || strlen(srTDTRec.szTicket_Device3) <= 0 || strlen(srTDTRec.szTicket_Device3) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTicket_Device3() ERROR !!");

			if (szTicket_Device3 == NULL)
                        {
                                inLogPrintf(AT, "szTicket_Device3 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_Device3 length = (%d)", (int)strlen(srTDTRec.szTicket_Device3));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_Device3[0], &srTDTRec.szTicket_Device3[0], strlen(srTDTRec.szTicket_Device3));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_Device3
Date&Time       :2018/7/10 下午 3:44
Describe        :
*/
int inSetTicket_Device3(char* szTicket_Device3)
{
        memset(srTDTRec.szTicket_Device3, 0x00, sizeof(srTDTRec.szTicket_Device3));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_Device3 == NULL || strlen(szTicket_Device3) <= 0 || strlen(szTicket_Device3) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTicket_Device3() ERROR !!");
                        if (szTicket_Device3 == NULL)
                        {
                                inLogPrintf(AT, "szTicket_Device3 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_Device3 length = (%d)", (int)strlen(szTicket_Device3));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_Device3[0], &szTicket_Device3[0], strlen(szTicket_Device3));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_ReversalBit
Date&Time       :2018/11/23 上午 10:58
Describe        :
*/
int inGetTicket_ReversalBit(char* szTicket_ReversalBit)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_ReversalBit == NULL || strlen(srTDTRec.szTicket_ReversalBit) <= 0 || strlen(srTDTRec.szTicket_ReversalBit) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTicket_ReversalBit() ERROR !!");

			if (szTicket_ReversalBit == NULL)
                        {
                                inLogPrintf(AT, "szTicket_ReversalBit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_ReversalBit length = (%d)", (int)strlen(srTDTRec.szTicket_ReversalBit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_ReversalBit[0], &srTDTRec.szTicket_ReversalBit[0], strlen(srTDTRec.szTicket_ReversalBit));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_ReversalBit
Date&Time       :2019/1/23 下午 12:00
Describe        :
*/
int inSetTicket_ReversalBit(char* szTicket_ReversalBit)
{
        memset(srTDTRec.szTicket_ReversalBit, 0x00, sizeof(srTDTRec.szTicket_ReversalBit));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_ReversalBit == NULL || strlen(szTicket_ReversalBit) <= 0 || strlen(szTicket_ReversalBit) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTicket_ReversalBit() ERROR !!");
                        if (szTicket_ReversalBit == NULL)
                        {
                                inLogPrintf(AT, "szTicket_ReversalBit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_ReversalBit length = (%d)", (int)strlen(szTicket_ReversalBit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_ReversalBit[0], &szTicket_ReversalBit[0], strlen(szTicket_ReversalBit));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_LastAutoSignOnDate
Date&Time       :2019/1/23 下午 12:02
Describe        :
*/
int inGetTicket_LastAutoSignOnDate(char* szTicket_LastAutoSignOnDate)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_LastAutoSignOnDate == NULL || strlen(srTDTRec.szTicket_LastAutoSignOnDate) <= 0 || strlen(srTDTRec.szTicket_LastAutoSignOnDate) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTicket_LastAutoSignOnDate() ERROR !!");

			if (szTicket_LastAutoSignOnDate == NULL)
                        {
                                inLogPrintf(AT, "szTicket_LastAutoSignOnDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_LastAutoSignOnDate length = (%d)", (int)strlen(srTDTRec.szTicket_LastAutoSignOnDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_LastAutoSignOnDate[0], &srTDTRec.szTicket_LastAutoSignOnDate[0], strlen(srTDTRec.szTicket_LastAutoSignOnDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_LastAutoSignOnDate
Date&Time       :2019/1/23 下午 12:02
Describe        :
*/
int inSetTicket_LastAutoSignOnDate(char* szTicket_LastAutoSignOnDate)
{
        memset(srTDTRec.szTicket_LastAutoSignOnDate, 0x00, sizeof(srTDTRec.szTicket_LastAutoSignOnDate));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_LastAutoSignOnDate == NULL || strlen(szTicket_LastAutoSignOnDate) <= 0 || strlen(szTicket_LastAutoSignOnDate) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTicket_LastAutoSignOnDate() ERROR !!");
                        if (szTicket_LastAutoSignOnDate == NULL)
                        {
                                inLogPrintf(AT, "szTicket_LastAutoSignOnDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_LastAutoSignOnDate length = (%d)", (int)strlen(szTicket_LastAutoSignOnDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_LastAutoSignOnDate[0], &szTicket_LastAutoSignOnDate[0], strlen(szTicket_LastAutoSignOnDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_PIN
Date&Time       :2019/11/14 下午 1:38
Describe        :
*/
int inGetTicket_PIN(char* szTicket_PIN)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_PIN == NULL || strlen(srTDTRec.szTicket_PIN) <= 0 || strlen(srTDTRec.szTicket_PIN) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTicket_PIN() ERROR !!");

			if (szTicket_PIN == NULL)
                        {
                                inLogPrintf(AT, "szTicket_PIN == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_PIN length = (%d)", (int)strlen(srTDTRec.szTicket_PIN));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_PIN[0], &srTDTRec.szTicket_PIN[0], strlen(srTDTRec.szTicket_PIN));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_PIN
Date&Time       :2019/11/14 下午 1:38
Describe        :
*/
int inSetTicket_PIN(char* szTicket_PIN)
{
        memset(srTDTRec.szTicket_PIN, 0x00, sizeof(srTDTRec.szTicket_PIN));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_PIN == NULL || strlen(szTicket_PIN) <= 0 || strlen(szTicket_PIN) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTicket_PIN() ERROR !!");
                        if (szTicket_PIN == NULL)
                        {
                                inLogPrintf(AT, "szTicket_PIN == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_PIN length = (%d)", (int)strlen(szTicket_PIN));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_PIN[0], &szTicket_PIN[0], strlen(szTicket_PIN));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_API_Version
Date&Time       :2020/3/4 下午 5:12
Describe        :
*/
int inGetTicket_API_Version(char* szTicket_API_Version)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_API_Version == NULL || strlen(srTDTRec.szTicket_API_Version) <= 0 || strlen(srTDTRec.szTicket_API_Version) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTicket_API_Version() ERROR !!");

			if (szTicket_API_Version == NULL)
                        {
                                inLogPrintf(AT, "szTicket_API_Version == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_API_Version length = (%d)", (int)strlen(srTDTRec.szTicket_API_Version));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_API_Version[0], &srTDTRec.szTicket_API_Version[0], strlen(srTDTRec.szTicket_API_Version));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_API_Version
Date&Time       :2020/3/4 下午 5:12
Describe        :
*/
int inSetTicket_API_Version(char* szTicket_API_Version)
{
        memset(srTDTRec.szTicket_API_Version, 0x00, sizeof(srTDTRec.szTicket_API_Version));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_API_Version == NULL || strlen(szTicket_API_Version) <= 0 || strlen(szTicket_API_Version) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTicket_API_Version() ERROR !!");
                        if (szTicket_API_Version == NULL)
                        {
                                inLogPrintf(AT, "szTicket_API_Version == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_API_Version length = (%d)", (int)strlen(szTicket_API_Version));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTDTRec.szTicket_API_Version[0], &szTicket_API_Version[0], strlen(szTicket_API_Version));

        return (VS_SUCCESS);
}

/*
Function        :inTDT_Edit_TDT_Table
Date&Time       :2017/5/15 下午 4:08
Describe        :
*/
int inTDT_Edit_TDT_Table(void)
{
	TABLE_GET_SET_TABLE TDT_FUNC_TABLE[] =
	{
		{"szTicket_HostIndex"			,inGetTicket_HostIndex			,inSetTicket_HostIndex			},
		{"szTicket_HostName"			,inGetTicket_HostName			,inSetTicket_HostName			},
		{"szTicket_HostTransFunc"		,inGetTicket_HostTransFunc		,inSetTicket_HostTransFunc		},
		{"szTicket_HostEnable"			,inGetTicket_HostEnable			,inSetTicket_HostEnable			},
		{"szTicket_LogOnOK"			,inGetTicket_LogOnOK			,inSetTicket_LogOnOK			},
		{"szTicket_SAM_Slot"			,inGetTicket_SAM_Slot			,inSetTicket_SAM_Slot			},
		{"szTicket_ReaderID"			,inGetTicket_ReaderID			,inSetTicket_ReaderID			},
		{"szTicket_STAN"			,inGetTicket_STAN			,inSetTicket_STAN			},
		{"szTicket_LastTransDate"		,inGetTicket_LastTransDate		,inSetTicket_LastTransDate		},
		{"szTicket_LastRRN"			,inGetTicket_LastRRN			,inSetTicket_LastRRN			},
		{"szTicket_Device1"			,inGetTicket_Device1			,inSetTicket_Device1			},
		{"szTicket_Device2"			,inGetTicket_Device2			,inSetTicket_Device2			},
		{"szTicket_Batch"			,inGetTicket_Batch			,inSetTicket_Batch			},
		{"szTicket_NeedNewBatch"		,inGetTicket_NeedNewBatch		,inSetTicket_NeedNewBatch		},
		{"szTicket_Device3"			,inGetTicket_Device3			,inSetTicket_Device3			},
		{"szTicket_ReversalBit"			,inGetTicket_ReversalBit		,inSetTicket_ReversalBit		},
		{"szTicket_LastAutoSignOnDate"		,inGetTicket_LastAutoSignOnDate		,inSetTicket_LastAutoSignOnDate		},
		{"szTicket_PIN"				,inGetTicket_PIN			,inSetTicket_PIN			},
		{"szTicket_API_Version"			,inGetTicket_API_Version		,inSetTicket_API_Version		},
		{""},
	};
	int		inRetVal;
	int		inRecordCnt = 0;
	int		inFinalTimeout = 0;
	char		szKey;
	DISPLAY_OBJECT  srDispObj;
	
	inFinalTimeout = 30;
	if (inFinalTimeout > 0)
	{
		inFinalTimeout = inFinalTimeout;
	}
	else
	{
		inFinalTimeout = _EDC_TIMEOUT_;
	}
	
	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	srDispObj.inY = _LINE_8_8_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 2;
	srDispObj.inColor = _COLOR_BLACK_;
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont_Color("是否更改TDT", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
	inDISP_ChineseFont_Color("列印請按0 更改請按Enter", _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);
	inDISP_ChineseFont_Color("跳過請按取消鍵", _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_BLACK_, _DISP_LEFT_);
	while (1)
	{
		szKey = uszKBD_GetKey(inFinalTimeout);
		if (szKey == _KEY_0_)
		{
			inRetVal = VS_SUCCESS;
			/* 這裡放列印的Function */
			return	(inRetVal);
		}
		else if (szKey == _KEY_CANCEL_ )
		{
			inRetVal = VS_USER_CANCEL;
			
			return	(inRetVal);
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			
			return	(inRetVal);
		}
		else if (szKey == _KEY_ENTER_)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont_Color("請輸入Record Index?", _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_BLACK_, _DISP_LEFT_);
			while (1)
			{
				memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
				srDispObj.inOutputLen = 0;
	
				inRetVal = inDISP_Enter8x16(&srDispObj);
				if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				{
					return (inRetVal);
				}
				else if (srDispObj.inOutputLen > 0)
				{
					inRecordCnt = atoi(srDispObj.szOutput);
					break;
				}
			}

			break;
		}
		
	}
	inLoadTDTRec(inRecordCnt);
	
	inFunc_Edit_Table_Tag(TDT_FUNC_TABLE);
	inSaveTDTRec(inRecordCnt);
	
	return	(VS_SUCCESS);
}
