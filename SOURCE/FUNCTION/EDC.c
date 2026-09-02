#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/AllStruct.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "File.h"
#include "Function.h"
#include "Accum.h"
#include "NexsysSDK.h"
#include "EDC.h"

static  EDC_REC srEDCRec;					/* construct EDC record */
extern  int	ginDebug;					/* Debug使用 extern */
/* 2018/5/23 下午 3:43
   VersionID和VersionDate改為寫死在程式中
 */
#if _MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_
	#if _NEXSYS_APP_MODE_ == _NEXSYS_APP_MODE_SDK_
		char		gszTermVersionID[16 + 1] = _CEE_UPT1000_TERMINAL_VERSION_;	/* 暫存的TerminalVersionID，重開機要還原(測試導向功能) */
	#else
		char		gszTermVersionID[16 + 1] = "CUPT1AH-D-019";	/* 暫存的TerminalVersionID，重開機要還原(測試導向功能) */;
	#endif
#else
	#ifdef _NCCC_AP_ID_MFES_
		char		gszTermVersionID[16 + 1] = "CASV3AH047019";	/* 暫存的TerminalVersionID，重開機要還原(測試導向功能) */;
	#else
		char		gszTermVersionID[16 + 1] = "CASV3AH-D-019";	/* 暫存的TerminalVersionID，重開機要還原(測試導向功能) */;
	#endif
#endif

char		gszTermVersionDate[16 + 1] = "20260625";		/* 暫存的TerminalVersionDate，重開機要還原(測試導向功能) */;
/*
Function        :inLoadEDCRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀EDC檔案，inEDCRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadEDCRec(int inEDCRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆EDC檔案 */
        char            szEDCRec[_SIZE_EDC_REC_ + 1];           /* 暫存, 放各個欄位檔案 */
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnEDCLength = 0;                        /* EDC總長度 */
        long            lnReadLength;                           /* 記錄每次要從EDC.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;                 /* inRec記錄讀到第幾筆, i為目前從EDC讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */

        /* inLoadEDCRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadEDCRec(%d) START!!", inEDCRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 判斷傳進來的inEDCRec是否小於零 大於等於零才是正確值(防呆) */
        if (inEDCRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inEDCRec < 0:(index = %d) ERROR!!", inEDCRec);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open EDC.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_EDC_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnEDCLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_EDC_FILE_NAME_);

	if (lnEDCLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnEDCLength + 1);
        uszTemp = malloc(lnEDCLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnEDCLength + 1);
        memset(uszTemp, 0x00, lnEDCLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnEDCLength;

                for (i = 0;; ++i)
                {
                        /* 剩餘長度大於或等於1024 */
                        if (lnReadLength >= 1024)
                        {
                                if (inFILE_Read(&ulFile_Handle, &uszTemp[1024*i], 1024) == VS_SUCCESS)
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
                                if (inFILE_Read(&ulFile_Handle, &uszTemp[1024*i], lnReadLength) == VS_SUCCESS)
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
                } /* end for loop */
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
         *i為目前從EDC讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnEDCLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到EDC的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnEDCLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        /* 因為inEDC_Rec的index從0開始，所以inEDC_Rec要+1 */
                        if (inRec == (inEDCRec + 1))
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
         * 如果沒有inEDCRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inEDCRec + 1) || inSearchResult == -1)
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
        memset(&srEDCRec, 0x00, sizeof(srEDCRec));
	/*
         * 以下pattern為存入EDC_Rec
         * i為EDC的第幾個字元
         * 存入EDC_Rec
         */
        i = 0;


	/* 01_是否開啟銀聯 */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

		/* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szCUPFuncEnable empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szCUPFuncEnable[0], &szEDCRec[0], k - 1);
        }
	
	/* 02_是否開啟SmartPay */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szFiscFuncEnable empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szFiscFuncEnable[0], &szEDCRec[0], k - 1);
        }
	
	/* 03_ECRComPort */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szECRComPort empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szECRComPort[0], &szEDCRec[0], k - 1);
        }
	
	/* 04_ECRVersion */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szECRVersion empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szECRVersion[0], &szEDCRec[0], k - 1);
        }
	
	/* 05_PABX Code */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szPABXCode empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szPABXCode[0], &szEDCRec[0], k - 1);
        }
	
	/* 06_TSAM RegisterEnable */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szTSAMRegisterEnable empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szTSAMRegisterEnable[0], &szEDCRec[0], k - 1);
        }

        /* 07_TMS下載是否成功 */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szTMSOK empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szTMSOK[0], &szEDCRec[0], k - 1);
        }
	
	/* 08_是否為新裝機 */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szDCCInit empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szDCCInit[0], &szEDCRec[0], k - 1);
        }

	/* 09_DCC下載模式 */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szDCCDownloadMode empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szDCCDownloadMode[0], &szEDCRec[0], k - 1);
        }
	
	/* 10_上次啟動DCC下載日期 */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szDCCLastUpdateDate empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szDCCLastUpdateDate[0], &szEDCRec[0], k - 1);
        }
	
	/* 11_結帳完是否要再做DCC下載 */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szDCCSettleDownload empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szDCCSettleDownload[0], &szEDCRec[0], k - 1);
        }
	
	/* 12_DCCBinTableVersion */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szDCCBinVer empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C )  
        {
                memcpy(&srEDCRec.szDCCBinVer[0], &szEDCRec[0], k - 1);
        }
	
        /* 13_EDC是否鎖機 */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szEDCLOCK empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szEDCLOCK[0], &szEDCRec[0], k - 1);
        }
        
        /* 14_Debug列印模式 */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szISODebug empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szISODebug[0], &szEDCRec[0], k - 1);
        }

        /* 15_工程師管理密碼 */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szManagerPassword empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szManagerPassword[0], &szEDCRec[0], k - 1);
        }
	
	/* 16_裝機工程師管理密碼 */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szFunctionPassword empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szFunctionPassword[0], &szEDCRec[0], k - 1);
        }
	
	/* 17_商店管理密碼 */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szMerchantPassword empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szMerchantPassword[0], &szEDCRec[0], k - 1);
        }
	
	/* 18_??管理密碼 */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szSuperPassword empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szSuperPassword[0], &szEDCRec[0], k - 1);
        }

        /* 19_MCCCode */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szMCCCode empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szMCCCode[0], &szEDCRec[0], k - 1);
        }

        /* 20_IssuerID */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szIssuerID empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szIssuerID[0], &szEDCRec[0], k - 1);
        }

        /* 21_輸入逾時時間 */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szEnterTimeout empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szEnterTimeout[0], &szEDCRec[0], k - 1);
        }

        /* 22_IP Send TimeOut時間 */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szIPSendTimeout empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szIPSendTimeout[0], &szEDCRec[0], k - 1);
        }

        /* 23_TermVersionID */
	/* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szTermVersionID empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szTermVersionID[0], &szEDCRec[0], k - 1);
        }

        /* 24_TermVersionDate */
	/* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szTermVersionDate empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szTermVersionDate[0], &szEDCRec[0], k - 1);
        }

        /* 25_EDC IP */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szTermIPAddress empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szTermIPAddress[0], &szEDCRec[0], k - 1);
        }

        /* 26_TermGetewayAddress */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szTermGetewayAddress empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szTermGetewayAddress[0], &szEDCRec[0], k - 1);
        }

        /* 27_TermMASKAddress */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szTermMASKAddress empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szTermMASKAddress[0], &szEDCRec[0], k - 1);
        }
	
	/* 28_TermECRPort */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szTermECRPort empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szTermECRPort[0], &szEDCRec[0], k - 1);
        }
	
	/* 29 szESCMode */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szESCMode empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szESCMode[0], &szEDCRec[0], k - 1);
        }
	
	/* 30_MultiComPort1 */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szMultiComPort1 empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szMultiComPort1[0], &szEDCRec[0], k - 1);
        }
	
	/* 31 szMultiComPort1Version */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szMultiComPort1Version empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szMultiComPort1Version[0], &szEDCRec[0], k - 1);
        }
	
	/* 32 MultiComPort2 */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szMultiComPort2 empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szMultiComPort2[0], &szEDCRec[0], k - 1);
        }
	
	/* 33 szMultiComPort2Version */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szMultiComPort2Version empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szMultiComPort2Version[0], &szEDCRec[0], k - 1);
        }
	
	/* 34 szMultiComPort3 */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szMultiComPort3 empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szMultiComPort3[0], &szEDCRec[0], k - 1);
        }
	
	/* 35 szMultiComPort3Version */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szMultiComPort3Version empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szMultiComPort3Version[0], &szEDCRec[0], k - 1);
        }
	
	/* 36 szEMVForceOnline */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szEMVForceOnline empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szEMVForceOnline[0], &szEDCRec[0], k - 1);
        }
	
	/* 37 szAutoConnect */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szAutoConnect empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szAutoConnect[0], &szEDCRec[0], k - 1);
        }
	
	/* 38 szLOGONum */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szLOGONum empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szLOGONum[0], &szEDCRec[0], k - 1);
        }
 
	/* 39 szHostSAMSlot */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szHostSAMSlot empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szHostSAMSlot[0], &szEDCRec[0], k - 1);
        }
	
	/* 40 szSAMSlotSN1 */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szSAMSlotSN1 empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szSAMSlotSN1[0], &szEDCRec[0], k - 1);
        }
	
	/* 41 szSAMSlotSN2 */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szSAMSlotSN2 empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szSAMSlotSN2[0], &szEDCRec[0], k - 1);
        }
	
	/* 42 szSAMSlotSN3 */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szSAMSlotSN3 empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szSAMSlotSN3[0], &szEDCRec[0], k - 1);
        }
	
	/* 43 szSAMSlotSN4 */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szSAMSlotSN4 empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szSAMSlotSN4[0], &szEDCRec[0], k - 1);
        }
	
	/* 44 szPWMEnable */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szPWMEnable empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szPWMEnable[0], &szEDCRec[0], k - 1);
        }
	
	/* 45 szPWMMode */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szPWMMode empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szPWMMode[0], &szEDCRec[0], k - 1);
        }
	
	/* 46 szPWMIdleTimeout */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szPWMIdleTimeout empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
	/* 因為是最後的欄位還要多判斷0x00 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szPWMIdleTimeout[0], &szEDCRec[0], k - 1);
        }
	
	/* 47 szDemoMode */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szDemoMode empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
	/* 因為是最後的欄位還要多判斷0x00 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szDemoMode[0], &szEDCRec[0], k - 1);
        }
	
	/* 48 szKBDLock */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szKBDLock empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
	/* 因為是最後的欄位還要多判斷0x00 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szKBDLock[0], &szEDCRec[0], k - 1);
        }
	
	/* 49 szLastDownloadMode */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szLastDownloadMode empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
	/* 因為是最後的欄位還要多判斷0x00 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szLastDownloadMode[0], &szEDCRec[0], k - 1);
        }
	
	/* 50 szPMPassword */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szPMPassword empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
	/* 因為是最後的欄位還要多判斷0x00 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szPMPassword[0], &szEDCRec[0], k - 1);
        }
	
	/* 51 szFunKeyPassword */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                /* 修改為僅跳過，不跳錯誤 2019/10/22 上午 11:54 by Russell */
                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szFunKeyPassword empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
	/* 因為是最後的欄位還要多判斷0x00 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szFunKeyPassword[0], &szEDCRec[0], k - 1);
        }
	
	/* 52 szTMSDownloadTimes */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szTMSDownloadTimes empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
	/* 因為是最後的欄位還要多判斷0x00 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szTMSDownloadTimes[0], &szEDCRec[0], k - 1);
        }
	
	/* 53 szExamBit */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szExamBit empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
	/* 因為是最後的欄位還要多判斷0x00 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szExamBit[0], &szEDCRec[0], k - 1);
        }
	
	/* 54 szTMSPacketSize */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szTMSPacketSize empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
	/* 因為是最後的欄位還要多判斷0x00 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szTMSPacketSize[0], &szEDCRec[0], k - 1);
        }
	
	/* 55 szSupECR_UDP */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szTMSPacketSize empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
	/* 因為是最後的欄位還要多判斷0x00 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szSupECR_UDP[0], &szEDCRec[0], k - 1);
        }
	
	/* 56 szUDP_Port */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szTMSPacketSize empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
	/* 因為是最後的欄位還要多判斷0x00 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szUDP_Port[0], &szEDCRec[0], k - 1);
        }
	
	/* 57 szPOS_IP */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szTMSPacketSize empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
	/* 因為是最後的欄位還要多判斷0x00 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szPOS_IP[0], &szEDCRec[0], k - 1);
        }
	
	/* 58 szPOSTxUniqueNo */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szTMSPacketSize empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
	/* 因為是最後的欄位還要多判斷0x00 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szPOSTxUniqueNo[0], &szEDCRec[0], k - 1);
        }
	
	/* 59 szUDP_ECRVersion */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szUDP_ECRVersion empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
	/* 因為是最後的欄位還要多判斷0x00 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szUDP_ECRVersion[0], &szEDCRec[0], k - 1);
        }
	
	/* 60 szPOS_ID */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szPOS_ID empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
	/* 因為是最後的欄位還要多判斷0x00 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szPOS_ID[0], &szEDCRec[0], k - 1);
        }
	
	/* 60 szGPRS_APN */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szGPRS_APN empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
	/* 因為是最後的欄位還要多判斷0x00 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szGPRS_APN[0], &szEDCRec[0], k - 1);
        }
	
	/* 61 szScreenBrightness */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szScreenBrightness empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
	/* 因為是最後的欄位還要多判斷0x00 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szScreenBrightness[0], &szEDCRec[0], k - 1);
        }
	
	/* 62 szSignpadBeepInterval */
        /* 初始化 */
        memset(szEDCRec, 0x00, sizeof(szEDCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szEDCRec[k ++] = uszReadData[i ++];
                if (szEDCRec[k - 1] == 0x2C	||
		    szEDCRec[k - 1] == 0x0D	||
		    szEDCRec[k - 1] == 0x0A	||
		    szEDCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnEDCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EDC szSignpadBeepInterval empty");
                        }

                        break;
                }
        }

        /*  該筆有資料 */
	/* 因為是最後的欄位還要多判斷0x00 */
        if (szEDCRec[0] != 0x2C	&&
	    szEDCRec[0] != 0x0D	&&
	    szEDCRec[0] != 0x0A	&&
	    szEDCRec[0] != 0x00)
        {
                memcpy(&srEDCRec.szSignpadBeepInterval[0], &szEDCRec[0], k - 1);
        }
	
        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadEDCRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadEDCRec(%d) END!!", inEDCRec);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveEDCRec
Date&Time       :2015/8/31 下午 2:00
Describe        :寫入EDC.dat，inEDCRec是要讀哪一筆的紀錄，第一筆為0
*/
int inSaveEDCRec(int inEDCRec)
{
        unsigned long   uldat_Handle;   			/* FILE Handle */
        unsigned long   ulbak_Handle;   			/* FILE Handle */
        int     	inPackCount = 0;        		/* uszWriteBuff_Record的index */
        int     	inRecIndex = 0;         		/* uszRead_Total_Buff的index */
        int     	inEDC_Total_Rec = 0;    		/* EDC.dat的總筆數 */
        int     	inRetVal;
        int     	i = 0, j = 0;
        char    	szDebugMsg[_DEBUG_MESSAGE_SIZE_ + 1];	/* debug message */
        long    	lnReadLength = 0;       		/* 每次要從EDC.dat讀多少byte出來 */
        long    	lnEDCLength = 0;         		/* EDC.dat檔案總長度 */
        unsigned char   *uszRead_Total_Buff;
        unsigned char   *uszWriteBuff_Record, *uszWriteBuff_Org;


        /* inSaveEDCRec()_START */
        if (ginDebug == VS_TRUE)
        {
		inLogPrintf(AT, "----------------------------------------");
                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                sprintf(szDebugMsg, "inSaveEDCRec(%d) START!!", inEDCRec);
                inLogPrintf(AT, szDebugMsg);
        }

        /* 防呆先刪除EDC.bak  */
        inFILE_Delete((unsigned char *)_EDC_FILE_NAME_BAK_);

        /* 新建EDC.bak檔案 */
        inRetVal = inFILE_Create(&ulbak_Handle, (unsigned char *)_EDC_FILE_NAME_BAK_);

        if (inRetVal != VS_SUCCESS)
        {
                inFILE_Close(&ulbak_Handle);

                return (VS_ERROR);
        }

        /* 開啟原檔案EDC.dat */
        inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_EDC_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                /* EDC.dat開檔失敗 ，不用關檔EDC.dat */
                /* EDC.bak仍要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_EDC_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 取得EDC.dat檔案大小 */
        lnEDCLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *)_EDC_FILE_NAME_);

        /* 取得檔案大小失敗 */
        if (lnEDCLength == VS_ERROR)
        {
                /* EDC.bak和EDC.dat要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Close(&uldat_Handle);
                inFILE_Delete((unsigned char *)_EDC_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        uszWriteBuff_Record = malloc(_SIZE_EDC_REC_ + _SIZE_EDC_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_EDC_REC_ + _SIZE_EDC_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原EDC.dat的每筆Record 初始化 */
        uszWriteBuff_Org = malloc(_SIZE_EDC_REC_ + _SIZE_EDC_COMMA_0D0A_);
        memset(uszWriteBuff_Org, 0x00, _SIZE_EDC_REC_ + _SIZE_EDC_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存EDC.dat全部資料 */
        uszRead_Total_Buff = malloc(lnEDCLength + 1);
        memset(uszRead_Total_Buff, 0x00, lnEDCLength + 1);

        inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

	
	/* CUPFuncEnable */
	memcpy(&uszWriteBuff_Record[0], &srEDCRec.szCUPFuncEnable[0], strlen(srEDCRec.szCUPFuncEnable));
        inPackCount += strlen(srEDCRec.szCUPFuncEnable);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* FiscFuncEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szFiscFuncEnable[0], strlen(srEDCRec.szFiscFuncEnable));
        inPackCount += strlen(srEDCRec.szFiscFuncEnable);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szECRComPort */
	memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szECRComPort[0], strlen(srEDCRec.szECRComPort));
        inPackCount += strlen(srEDCRec.szECRComPort);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* ECRVersion */
	memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szECRVersion[0], strlen(srEDCRec.szECRVersion));
        inPackCount += strlen(srEDCRec.szECRVersion);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* PABXCODE */
	memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szPABXCode[0], strlen(srEDCRec.szPABXCode));
        inPackCount += strlen(srEDCRec.szPABXCode);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* TSAMRegisterEnable*/
	memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szTSAMRegisterEnable[0], strlen(srEDCRec.szTSAMRegisterEnable));
        inPackCount += strlen(srEDCRec.szTSAMRegisterEnable);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TMSOK */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szTMSOK[0], strlen(srEDCRec.szTMSOK));
        inPackCount += strlen(srEDCRec.szTMSOK);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* DCCInit */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szDCCInit[0], strlen(srEDCRec.szDCCInit));
        inPackCount += strlen(srEDCRec.szDCCInit);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* DCCDownloadMode */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szDCCDownloadMode[0], strlen(srEDCRec.szDCCDownloadMode));
        inPackCount += strlen(srEDCRec.szDCCDownloadMode);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* DCCLastUpdateDate */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szDCCLastUpdateDate[0], strlen(srEDCRec.szDCCLastUpdateDate));
        inPackCount += strlen(srEDCRec.szDCCLastUpdateDate);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* DCCSettleDownload */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szDCCSettleDownload[0], strlen(srEDCRec.szDCCSettleDownload));
        inPackCount += strlen(srEDCRec.szDCCSettleDownload);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* DCCBinTableVersion */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szDCCBinVer[0], strlen(srEDCRec.szDCCBinVer));
        inPackCount += strlen(srEDCRec.szDCCBinVer);
	uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
        /* EDCLOCK */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szEDCLOCK[0], strlen(srEDCRec.szEDCLOCK));
        inPackCount += strlen(srEDCRec.szEDCLOCK);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
        
        /* ISODebug */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szISODebug[0], strlen(srEDCRec.szISODebug));
        inPackCount += strlen(srEDCRec.szISODebug);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* szManagerPassword */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szManagerPassword[0], strlen(srEDCRec.szManagerPassword));
        inPackCount += strlen(srEDCRec.szManagerPassword);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szFunctionPassword */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szFunctionPassword[0], strlen(srEDCRec.szFunctionPassword));
        inPackCount += strlen(srEDCRec.szFunctionPassword);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szMerchantPassword */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szMerchantPassword[0], strlen(srEDCRec.szMerchantPassword));
        inPackCount += strlen(srEDCRec.szMerchantPassword);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szSuperPassword */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szSuperPassword[0], strlen(srEDCRec.szSuperPassword));
        inPackCount += strlen(srEDCRec.szSuperPassword);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* MCCCode */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szMCCCode[0], strlen(srEDCRec.szMCCCode));
        inPackCount += strlen(srEDCRec.szMCCCode);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* IssuerID */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szIssuerID[0], strlen(srEDCRec.szIssuerID));
        inPackCount += strlen(srEDCRec.szIssuerID);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* EnterTimeout */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szEnterTimeout[0], strlen(srEDCRec.szEnterTimeout));
        inPackCount += strlen(srEDCRec.szEnterTimeout);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* IPSendTimeout */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szIPSendTimeout[0], strlen(srEDCRec.szIPSendTimeout));
        inPackCount += strlen(srEDCRec.szIPSendTimeout);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TermVersionID */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szTermVersionID[0], strlen(srEDCRec.szTermVersionID));
        inPackCount += strlen(srEDCRec.szTermVersionID);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TermVersionDate */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szTermVersionDate[0], strlen(srEDCRec.szTermVersionDate));
        inPackCount += strlen(srEDCRec.szTermVersionDate);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TermIPAddress */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szTermIPAddress[0], strlen(srEDCRec.szTermIPAddress));
        inPackCount += strlen(srEDCRec.szTermIPAddress);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TermGetewayAddress */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szTermGetewayAddress[0], strlen(srEDCRec.szTermGetewayAddress));
        inPackCount += strlen(srEDCRec.szTermGetewayAddress);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* TermMASKAddress */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szTermMASKAddress[0], strlen(srEDCRec.szTermMASKAddress));
        inPackCount += strlen(srEDCRec.szTermMASKAddress);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* TermECRPort */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szTermECRPort[0], strlen(srEDCRec.szTermECRPort));
        inPackCount += strlen(srEDCRec.szTermECRPort);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* ESCMode */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szESCMode[0], strlen(srEDCRec.szESCMode));
        inPackCount += strlen(srEDCRec.szESCMode);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szMultiComPort1 */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szMultiComPort1[0], strlen(srEDCRec.szMultiComPort1));
        inPackCount += strlen(srEDCRec.szMultiComPort1);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szMultiComPort1Version */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szMultiComPort1Version[0], strlen(srEDCRec.szMultiComPort1Version));
        inPackCount += strlen(srEDCRec.szMultiComPort1Version);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szMultiComPort2 */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szMultiComPort2[0], strlen(srEDCRec.szMultiComPort2));
        inPackCount += strlen(srEDCRec.szMultiComPort2);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szMultiComPort2Version */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szMultiComPort2Version[0], strlen(srEDCRec.szMultiComPort2Version));
        inPackCount += strlen(srEDCRec.szMultiComPort2Version);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szMultiComPort3 */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szMultiComPort3[0], strlen(srEDCRec.szMultiComPort3));
        inPackCount += strlen(srEDCRec.szMultiComPort3);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szMultiComPort3Version */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szMultiComPort3Version[0], strlen(srEDCRec.szMultiComPort3Version));
        inPackCount += strlen(srEDCRec.szMultiComPort3Version);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szEMVForceOnline */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szEMVForceOnline[0], strlen(srEDCRec.szEMVForceOnline));
        inPackCount += strlen(srEDCRec.szEMVForceOnline);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szAutoConnect */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szAutoConnect[0], strlen(srEDCRec.szAutoConnect));
        inPackCount += strlen(srEDCRec.szAutoConnect);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szLOGONum */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szLOGONum[0], strlen(srEDCRec.szLOGONum));
        inPackCount += strlen(srEDCRec.szLOGONum);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szHostSAMSlot */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szHostSAMSlot[0], strlen(srEDCRec.szHostSAMSlot));
        inPackCount += strlen(srEDCRec.szHostSAMSlot);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szSAMSlotSN1 */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szSAMSlotSN1[0], strlen(srEDCRec.szSAMSlotSN1));
        inPackCount += strlen(srEDCRec.szSAMSlotSN1);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szSAMSlotSN2 */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szSAMSlotSN2[0], strlen(srEDCRec.szSAMSlotSN2));
        inPackCount += strlen(srEDCRec.szSAMSlotSN2);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szSAMSlotSN3 */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szSAMSlotSN3[0], strlen(srEDCRec.szSAMSlotSN3));
        inPackCount += strlen(srEDCRec.szSAMSlotSN3);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szSAMSlotSN4 */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szSAMSlotSN4[0], strlen(srEDCRec.szSAMSlotSN4));
        inPackCount += strlen(srEDCRec.szSAMSlotSN4);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szPWMEnable */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szPWMEnable[0], strlen(srEDCRec.szPWMEnable));
        inPackCount += strlen(srEDCRec.szPWMEnable);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szPWMMode */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szPWMMode[0], strlen(srEDCRec.szPWMMode));
        inPackCount += strlen(srEDCRec.szPWMMode);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szPWMIdleTimeout */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szPWMIdleTimeout[0], strlen(srEDCRec.szPWMIdleTimeout));
        inPackCount += strlen(srEDCRec.szPWMIdleTimeout);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szDemoMode */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szDemoMode[0], strlen(srEDCRec.szDemoMode));
        inPackCount += strlen(srEDCRec.szDemoMode);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szKBDLock */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szKBDLock[0], strlen(srEDCRec.szKBDLock));
        inPackCount += strlen(srEDCRec.szKBDLock);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szLastDownloadMode */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szLastDownloadMode[0], strlen(srEDCRec.szLastDownloadMode));
        inPackCount += strlen(srEDCRec.szLastDownloadMode);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szPMPassword */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szPMPassword[0], strlen(srEDCRec.szPMPassword));
        inPackCount += strlen(srEDCRec.szPMPassword);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szFunKeyPassword */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szFunKeyPassword[0], strlen(srEDCRec.szFunKeyPassword));
        inPackCount += strlen(srEDCRec.szFunKeyPassword);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szTMSDownloadTimes */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szTMSDownloadTimes[0], strlen(srEDCRec.szTMSDownloadTimes));
        inPackCount += strlen(srEDCRec.szTMSDownloadTimes);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szExamBit */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szExamBit[0], strlen(srEDCRec.szExamBit));
        inPackCount += strlen(srEDCRec.szExamBit);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szTMSPacketSize */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szTMSPacketSize[0], strlen(srEDCRec.szTMSPacketSize));
        inPackCount += strlen(srEDCRec.szTMSPacketSize);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szSupECR_UDP */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szSupECR_UDP[0], strlen(srEDCRec.szSupECR_UDP));
        inPackCount += strlen(srEDCRec.szSupECR_UDP);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szUDP_Port */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szUDP_Port[0], strlen(srEDCRec.szUDP_Port));
        inPackCount += strlen(srEDCRec.szUDP_Port);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szPOS_IP */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szPOS_IP[0], strlen(srEDCRec.szPOS_IP));
        inPackCount += strlen(srEDCRec.szPOS_IP);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szPOSTxUniqueNo */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szPOSTxUniqueNo[0], strlen(srEDCRec.szPOSTxUniqueNo));
        inPackCount += strlen(srEDCRec.szPOSTxUniqueNo);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szUDP_ECRVersion */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szUDP_ECRVersion[0], strlen(srEDCRec.szUDP_ECRVersion));
        inPackCount += strlen(srEDCRec.szUDP_ECRVersion);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szPOS_ID */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szPOS_ID[0], strlen(srEDCRec.szPOS_ID));
        inPackCount += strlen(srEDCRec.szPOS_ID);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szGPRS_APN */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szGPRS_APN[0], strlen(srEDCRec.szGPRS_APN));
        inPackCount += strlen(srEDCRec.szGPRS_APN);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szScreenBrightness */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szScreenBrightness[0], strlen(srEDCRec.szScreenBrightness));
        inPackCount += strlen(srEDCRec.szScreenBrightness);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szSignpadBeepInterval */
        memcpy(&uszWriteBuff_Record[inPackCount], &srEDCRec.szSignpadBeepInterval[0], strlen(srEDCRec.szSignpadBeepInterval));
        inPackCount += strlen(srEDCRec.szSignpadBeepInterval);
        
        /* 最後的data不用逗號 */
	/* 補上換行符號 */

        /* 0D是移至行首 */
        uszWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        uszWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;

        /* 讀EDC.dat檔案存到uszRead_Total_Buff */
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnEDCLength;

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
                                        inFILE_Delete((unsigned char *)_EDC_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_EDC_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_EDC_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        /* 防呆 總record數量小於要存取inEDCRec Return ERROR */
	/* 算總Record數 */
        for (i = 0; i < (lnEDCLength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (uszRead_Total_Buff[i] == 0x0A)
                        inEDC_Total_Rec++;
        }

	/* 防呆 總record數量小於要存取inEDCRec Return ERROR */
        if ((inEDCRec + 1) > inEDC_Total_Rec)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "No data or Index ERROR");
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inEDCRec: %d", inEDCRec);
			inLogPrintf(AT, szDebugMsg);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inEDC_Total_Rec: %d", inEDC_Total_Rec);
			inLogPrintf(AT, szDebugMsg);
                }

                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_EDC_FILE_NAME_BAK_);

                /* Free pointer */
		free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                return (VS_ERROR);
        }

        /* 記錄ReadTotalBuffer的陣列位置 */
        inRecIndex = 0;
        /* 用inEDCRec決定要先存幾筆Record到EDC.bak，ex:inEDCRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inEDCRec; i ++)
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
                                        inFILE_Delete((unsigned char *)_EDC_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_EDC_FILE_NAME_BAK_);

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

        /* 存組好的該EDCRecord 到 EDC.bak */
        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_EDC_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_EDC_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原EDC.dat Record 到 EDC.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
        inEDCRec = inEDCRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < lnEDCLength; i++)
        {
                if (uszRead_Total_Buff[i] == 0x0A)
                {
                        j++;

                        if (j == inEDCRec)
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
                /* 接續存原EDC.dat的Record */
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
                                inFILE_Delete((unsigned char *)_EDC_FILE_NAME_BAK_);

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
                                inFILE_Delete((unsigned char *)_EDC_FILE_NAME_BAK_);

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

        /* 刪除原EDC.dat */
        if (inFILE_Delete((unsigned char *)_EDC_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* 將EDC.bak改名字為EDC.dat取代原檔案 */
        if (inFILE_Rename((unsigned char *)_EDC_FILE_NAME_BAK_, (unsigned char *)_EDC_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* inSaveEDCRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                sprintf(szDebugMsg, "inSaveEDCRec(%d) END!!", inEDCRec - 1);
                inLogPrintf(AT, szDebugMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return(VS_SUCCESS);
}

/*
set和get等價於相反的操作
各欄位的set和get function
*/

/*
Function        :inGetCUPFuncEnable
Date&Time       :
Describe        :
*/
int inGetCUPFuncEnable(char* szCUPFuncEnable)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCUPFuncEnable == NULL || strlen(srEDCRec.szCUPFuncEnable) < 0 || strlen(srEDCRec.szCUPFuncEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        if (szCUPFuncEnable == NULL)
                        {
                                inLogPrintf(AT, "inGetCUPFuncEnable() ERROR !! szCUPFuncEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "inGetCUPFuncEnable() ERROR !! szCUPFuncEnable length = (%d)", (int)strlen(srEDCRec.szCUPFuncEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCUPFuncEnable[0], &srEDCRec.szCUPFuncEnable[0], strlen(srEDCRec.szCUPFuncEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetCUPFuncEnable
Date&Time       :
Describe        :
*/
int inSetCUPFuncEnable(char* szCUPFuncEnable)
{
        memset(srEDCRec.szCUPFuncEnable, 0x00, sizeof(srEDCRec.szCUPFuncEnable));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCUPFuncEnable == NULL || strlen(szCUPFuncEnable) < 0 || strlen(szCUPFuncEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCUPFuncEnable() ERROR !!");

                        if (szCUPFuncEnable == NULL)
                        {
                                inLogPrintf(AT, "szCUPFuncEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCUPFuncEnable length = (%d)", (int)strlen(szCUPFuncEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szCUPFuncEnable[0], &szCUPFuncEnable[0], strlen(szCUPFuncEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetFiscFuncEnable
Date&Time       :2016/11/25 下午 12:00
Describe        :是否開啟SmartPay開關
*/
int inGetFiscFuncEnable(char* szFiscFuncEnable)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szFiscFuncEnable == NULL || strlen(srEDCRec.szFiscFuncEnable) < 0 || strlen(srEDCRec.szFiscFuncEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        if (szFiscFuncEnable == NULL)
                        {
                                inLogPrintf(AT, "inGetFiscFuncEnable() ERROR !! szFiscFuncEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "inGetFiscFuncEnable() ERROR !! szFiscFuncEnable length = (%d)", (int)strlen(srEDCRec.szFiscFuncEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFiscFuncEnable[0], &srEDCRec.szFiscFuncEnable[0], strlen(srEDCRec.szFiscFuncEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetFiscFuncEnable
Date&Time       :2016/11/25 下午 12:00
Describe        :是否開啟SmartPay開關
*/
int inSetFiscFuncEnable(char* szFiscFuncEnable)
{
        memset(srEDCRec.szFiscFuncEnable, 0x00, sizeof(srEDCRec.szFiscFuncEnable));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szFiscFuncEnable == NULL || strlen(szFiscFuncEnable) < 0 || strlen(szFiscFuncEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetFiscFuncEnable() ERROR !!");

                        if (szFiscFuncEnable == NULL)
                        {
                                inLogPrintf(AT, "szFiscFuncEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFiscFuncEnable length = (%d)", (int)strlen(szFiscFuncEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szFiscFuncEnable[0], &szFiscFuncEnable[0], strlen(szFiscFuncEnable));

        return (VS_SUCCESS);
}


/*
Function        :inGetECRComPort
Date&Time       :
Describe        :
*/
int inGetECRComPort(char* szECRComPort)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szECRComPort == NULL || strlen(srEDCRec.szECRComPort) < 0 || strlen(srEDCRec.szECRComPort) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        if (szECRComPort == NULL)
                        {
                                inLogPrintf(AT, "inGetECRComPort() ERROR !! szECRComPort == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "inGetECRComPort() ERROR !! szECRComPort length = (%d)", (int)strlen(srEDCRec.szECRComPort));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szECRComPort[0], &srEDCRec.szECRComPort[0], strlen(srEDCRec.szECRComPort));

        return (VS_SUCCESS);
}

/*
Function        :inSetECRComPort
Date&Time       :
Describe        :
*/
int inSetECRComPort(char* szECRComPort)
{
        memset(srEDCRec.szECRComPort, 0x00, sizeof(srEDCRec.szECRComPort));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szECRComPort == NULL || strlen(szECRComPort) < 0 || strlen(szECRComPort) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetECRComPort() ERROR !!");

                        if (szECRComPort == NULL)
                        {
                                inLogPrintf(AT, "szECRComPort == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szECRComPort length = (%d)", (int)strlen(szECRComPort));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szECRComPort[0], &szECRComPort[0], strlen(szECRComPort));

        return (VS_SUCCESS);
}

/*
Function        :inGetECRVersion
Date&Time       :
Describe        :
*/
int inGetECRVersion(char* szECRVersion)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szECRVersion == NULL || strlen(srEDCRec.szECRVersion) < 0 || strlen(srEDCRec.szECRVersion) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        if (szECRVersion == NULL)
                        {
                                inLogPrintf(AT, "inGetECRVersion() ERROR !! szECRVersion == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "inGetECRVersion() ERROR !! szECRVersion length = (%d)", (int)strlen(srEDCRec.szECRVersion));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szECRVersion[0], &srEDCRec.szECRVersion[0], strlen(srEDCRec.szECRVersion));

        return (VS_SUCCESS);
}

/*
Function        :inSetECRVersion
Date&Time       :
Describe        :
*/
int inSetECRVersion(char* szECRVersion)
{
        memset(srEDCRec.szECRVersion, 0x00, sizeof(srEDCRec.szECRVersion));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szECRVersion == NULL || strlen(szECRVersion) < 0 || strlen(szECRVersion) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetECRVersion() ERROR !!");

                        if (szECRVersion == NULL)
                        {
                                inLogPrintf(AT, "szECRVersion == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szECRVersion length = (%d)", (int)strlen(szECRVersion));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szECRVersion[0], &szECRVersion[0], strlen(szECRVersion));

        return (VS_SUCCESS);
}

/*
Function        :inGetPABXCode
Date&Time       :
Describe        :
*/
int inGetPABXCode(char* szPABXCode)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPABXCode == NULL || strlen(srEDCRec.szPABXCode) < 0 || strlen(srEDCRec.szPABXCode) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        if (szPABXCode == NULL)
                        {
                                inLogPrintf(AT, "inGetPABXCode() ERROR !! szPABXCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "inGetPABXCode() ERROR !! szPABXCode length = (%d)", (int)strlen(srEDCRec.szPABXCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPABXCode[0], &srEDCRec.szPABXCode[0], strlen(srEDCRec.szPABXCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetPABXCode
Date&Time       :
Describe        :
*/
int inSetPABXCode(char* szPABXCode)
{
        memset(srEDCRec.szPABXCode, 0x00, sizeof(srEDCRec.szPABXCode));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPABXCode == NULL || strlen(szPABXCode) < 0 || strlen(szPABXCode) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPABXCode() ERROR !!");

                        if (szPABXCode == NULL)
                        {
                                inLogPrintf(AT, "szPABXCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPABXCode length = (%d)", (int)strlen(szPABXCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szPABXCode[0], &szPABXCode[0], strlen(szPABXCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetTSAMRegisterEnable
Date&Time       :
Describe        :
*/
int inGetTSAMRegisterEnable(char* szTSAMRegisterEnable)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTSAMRegisterEnable == NULL || strlen(srEDCRec.szTSAMRegisterEnable) < 0 || strlen(srEDCRec.szTSAMRegisterEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        if (szTSAMRegisterEnable == NULL)
                        {
                                inLogPrintf(AT, "inGetTSAMRegisterEnable() ERROR !! szTSAMRegisterEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "inGetTSAMRegisterEnable() ERROR !! szTSAMRegisterEnable length = (%d)", (int)strlen(srEDCRec.szTSAMRegisterEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTSAMRegisterEnable[0], &srEDCRec.szTSAMRegisterEnable[0], strlen(srEDCRec.szTSAMRegisterEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetTSAMRegisterEnable
Date&Time       :
Describe        :
*/
int inSetTSAMRegisterEnable(char* szTSAMRegisterEnable)
{
        memset(srEDCRec.szTSAMRegisterEnable, 0x00, sizeof(srEDCRec.szTSAMRegisterEnable));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTSAMRegisterEnable == NULL || strlen(szTSAMRegisterEnable) < 0 || strlen(szTSAMRegisterEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTSAMRegisterEnable() ERROR !!");

                        if (szTSAMRegisterEnable == NULL)
                        {
                                inLogPrintf(AT, "szTSAMRegisterEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTSAMRegisterEnable length = (%d)", (int)strlen(szTSAMRegisterEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szTSAMRegisterEnable[0], &szTSAMRegisterEnable[0], strlen(szTSAMRegisterEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMSOK
Date&Time       :
Describe        :
*/
int inGetTMSOK(char* szTMSOK)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTMSOK == NULL || strlen(srEDCRec.szTMSOK) < 0 || strlen(srEDCRec.szTMSOK) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTMSOK() ERROR !!");

                        if (szTMSOK == NULL)
                        {
                                inLogPrintf(AT, "szTMSOK == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSOK length = (%d)", (int)strlen(srEDCRec.szTMSOK));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }


                return (VS_ERROR);
        }
        memcpy(&szTMSOK[0], &srEDCRec.szTMSOK[0], strlen(srEDCRec.szTMSOK));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSOK
Date&Time       :
Describe        :
*/
int inSetTMSOK(char* szTMSOK)
{
        memset(srEDCRec.szTMSOK, 0x00, sizeof(srEDCRec.szTMSOK));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTMSOK == NULL || strlen(szTMSOK) < 0 || strlen(szTMSOK) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTMSOK() ERROR !!");

                        if (szTMSOK == NULL)
                        {
                                inLogPrintf(AT, "szTMSOK == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSOK length = (%d)", (int)strlen(szTMSOK));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }


                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szTMSOK[0], &szTMSOK[0], strlen(szTMSOK));

        return (VS_SUCCESS);
}

/*
Function        :inGetDCCInit
Date&Time       :2016/10/4 下午 1:53
Describe        :
*/
int inGetDCCInit(char* szDCCInit)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szDCCInit == NULL || strlen(srEDCRec.szDCCInit) < 0 || strlen(srEDCRec.szDCCInit) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetDCCFullDownload() ERROR !!");

                        if (szDCCInit == NULL)
                        {
                                inLogPrintf(AT, "szDCCFullDownload == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDCCFullDownload length = (%d)", (int)strlen(srEDCRec.szDCCInit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }


                return (VS_ERROR);
        }
        memcpy(&szDCCInit[0], &srEDCRec.szDCCInit[0], strlen(srEDCRec.szDCCInit));

        return (VS_SUCCESS);
}

/*
Function        :inSetDCCInit
Date&Time       :2016/10/4 下午 1:53
Describe        :
*/
int inSetDCCInit(char* szDCCInit)
{
        memset(srEDCRec.szDCCInit, 0x00, sizeof(srEDCRec.szDCCInit));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szDCCInit == NULL || strlen(szDCCInit) < 0 || strlen(szDCCInit) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetDCCFullDownload() ERROR !!");

                        if (szDCCInit == NULL)
                        {
                                inLogPrintf(AT, "szDCCFullDownload == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDCCFullDownload length = (%d)", (int)strlen(szDCCInit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }


                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szDCCInit[0], &szDCCInit[0], strlen(szDCCInit));

        return (VS_SUCCESS);
}

/*
Function        :inGetDCCDownloadMode
Date&Time       :2016/10/20 下午 12:02
Describe        :
*/
int inGetDCCDownloadMode(char* szDCCDownloadMode)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szDCCDownloadMode == NULL || strlen(srEDCRec.szDCCDownloadMode) < 0 || strlen(srEDCRec.szDCCDownloadMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetDCCFullDownload() ERROR !!");

                        if (szDCCDownloadMode == NULL)
                        {
                                inLogPrintf(AT, "szDCCFullDownload == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDCCFullDownload length = (%d)", (int)strlen(srEDCRec.szDCCDownloadMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }


                return (VS_ERROR);
        }
        memcpy(&szDCCDownloadMode[0], &srEDCRec.szDCCDownloadMode[0], strlen(srEDCRec.szDCCDownloadMode));

        return (VS_SUCCESS);
}

/*
Function        :inSetDCCDownloadMode
Date&Time       :2016/10/20 下午 12:02
Describe        :
*/
int inSetDCCDownloadMode(char* szDCCDownloadMode)
{
        memset(srEDCRec.szDCCDownloadMode, 0x00, sizeof(srEDCRec.szDCCDownloadMode));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szDCCDownloadMode == NULL || strlen(szDCCDownloadMode) < 0 || strlen(szDCCDownloadMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetDCCFullDownload() ERROR !!");

                        if (szDCCDownloadMode == NULL)
                        {
                                inLogPrintf(AT, "szDCCFullDownload == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDCCFullDownload length = (%d)", (int)strlen(szDCCDownloadMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }


                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szDCCDownloadMode[0], &szDCCDownloadMode[0], strlen(szDCCDownloadMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetDCCLastUpdateDate
Date&Time       :2016/10/4 下午 1:53
Describe        :
*/
int inGetDCCLastUpdateDate(char* szDCCLastUpdateDate)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szDCCLastUpdateDate == NULL || strlen(srEDCRec.szDCCLastUpdateDate) < 0 || strlen(srEDCRec.szDCCLastUpdateDate) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetDCCFullDownload() ERROR !!");

                        if (szDCCLastUpdateDate == NULL)
                        {
                                inLogPrintf(AT, "szDCCFullDownload == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDCCFullDownload length = (%d)", (int)strlen(srEDCRec.szDCCLastUpdateDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }


                return (VS_ERROR);
        }
        memcpy(&szDCCLastUpdateDate[0], &srEDCRec.szDCCLastUpdateDate[0], strlen(srEDCRec.szDCCLastUpdateDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetDCCLastUpdateDate
Date&Time       :2016/10/4 下午 1:53
Describe        :
*/
int inSetDCCLastUpdateDate(char* szDCCLastUpdateDate)
{
        memset(srEDCRec.szDCCLastUpdateDate, 0x00, sizeof(srEDCRec.szDCCLastUpdateDate));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szDCCLastUpdateDate == NULL || strlen(szDCCLastUpdateDate) < 0 || strlen(szDCCLastUpdateDate) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetDCCFullDownload() ERROR !!");

                        if (szDCCLastUpdateDate == NULL)
                        {
                                inLogPrintf(AT, "szDCCFullDownload == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDCCFullDownload length = (%d)", (int)strlen(szDCCLastUpdateDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }


                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szDCCLastUpdateDate[0], &szDCCLastUpdateDate[0], strlen(szDCCLastUpdateDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetDCCSettleDownload
Date&Time       :2016/10/20 下午 4:47
Describe        :
*/
int inGetDCCSettleDownload(char* szDCCSettleDownload)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szDCCSettleDownload == NULL || strlen(srEDCRec.szDCCSettleDownload) < 0 || strlen(srEDCRec.szDCCSettleDownload) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetDCCFullDownload() ERROR !!");

                        if (szDCCSettleDownload == NULL)
                        {
                                inLogPrintf(AT, "szDCCFullDownload == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDCCFullDownload length = (%d)", (int)strlen(srEDCRec.szDCCSettleDownload));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }


                return (VS_ERROR);
        }
        memcpy(&szDCCSettleDownload[0], &srEDCRec.szDCCSettleDownload[0], strlen(srEDCRec.szDCCSettleDownload));

        return (VS_SUCCESS);
}

/*
Function        :inSetDCCSettleDownload
Date&Time       :2016/10/20 下午 4:47
Describe        :
*/
int inSetDCCSettleDownload(char* szDCCSettleDownload)
{
        memset(srEDCRec.szDCCSettleDownload, 0x00, sizeof(srEDCRec.szDCCSettleDownload));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szDCCSettleDownload == NULL || strlen(szDCCSettleDownload) < 0 || strlen(szDCCSettleDownload) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetDCCFullDownload() ERROR !!");

                        if (szDCCSettleDownload == NULL)
                        {
                                inLogPrintf(AT, "szDCCFullDownload == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDCCFullDownload length = (%d)", (int)strlen(szDCCSettleDownload));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }


                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szDCCSettleDownload[0], &szDCCSettleDownload[0], strlen(szDCCSettleDownload));

        return (VS_SUCCESS);
}

/*
Function        :inGetDCCBinVer
Date&Time       :2016/10/20 上午 11:57
Describe        :
*/
int inGetDCCBinVer(char* szDCCBinVer)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szDCCBinVer == NULL || strlen(srEDCRec.szDCCBinVer) < 0 || strlen(srEDCRec.szDCCBinVer) > 4)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetDCCBinVer() ERROR !!");

                        if (szDCCBinVer == NULL)
                        {
                                inLogPrintf(AT, "szDCCBinVer == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDCCBinVer length = (%d)", (int)strlen(srEDCRec.szDCCBinVer));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szDCCBinVer[0], &srEDCRec.szDCCBinVer[0], strlen(srEDCRec.szDCCBinVer));

        return (VS_SUCCESS);
}

/*
Function        :inSetDCCBinVer
Date&Time       :2016/10/20 上午 11:57
Describe        :
*/
int inSetDCCBinVer(char* szDCCBinVer)
{
        memset(srEDCRec.szDCCBinVer, 0x00, sizeof(srEDCRec.szDCCBinVer));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szDCCBinVer == NULL || strlen(szDCCBinVer) < 0 || strlen(szDCCBinVer) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetDCCBinVer() ERROR !!");

                        if (szDCCBinVer == NULL)
                        {
                                inLogPrintf(AT, "szDCCBinVer == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDCCBinVer length = (%d)", (int)strlen(szDCCBinVer));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szDCCBinVer[0], &szDCCBinVer[0], strlen(szDCCBinVer));

        return (VS_SUCCESS);
}

/*
Function        :inGetEDCLOCK
Date&Time       :
Describe        :
*/
int inGetEDCLOCK(char* szEDCLOCK)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szEDCLOCK == NULL || strlen(srEDCRec.szEDCLOCK) < 0 || strlen(srEDCRec.szEDCLOCK) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetEDCLOCK() ERROR !!");

                        if (szEDCLOCK == NULL)
                        {
                                inLogPrintf(AT, "szEDCLOCK == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEDCLOCK length = (%d)", (int)strlen(srEDCRec.szEDCLOCK));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }


                return (VS_ERROR);
        }
        memcpy(&szEDCLOCK[0], &srEDCRec.szEDCLOCK[0], strlen(srEDCRec.szEDCLOCK));

        return (VS_SUCCESS);
}

/*
Function        :inSetEDCLOCK
Date&Time       :
Describe        :
*/
int inSetEDCLOCK(char* szEDCLOCK)
{
        memset(srEDCRec.szEDCLOCK, 0x00, sizeof(srEDCRec.szEDCLOCK));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szEDCLOCK == NULL || strlen(szEDCLOCK) < 0 || strlen(szEDCLOCK) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetEDCLOCK() ERROR !!");

                        if (szEDCLOCK == NULL)
                        {
                                inLogPrintf(AT, "szEDCLOCK == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEDCLOCK length = (%d)", (int)strlen(szEDCLOCK));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }


                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szEDCLOCK[0], &szEDCLOCK[0], strlen(szEDCLOCK));

        return (VS_SUCCESS);
}

/*
Function        :inGetISODebug
Date&Time       :
Describe        :
*/
int inGetISODebug(char* szISODebug)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szISODebug == NULL || strlen(srEDCRec.szISODebug) < 0 || strlen(srEDCRec.szISODebug) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetISODebug() ERROR !!");

                        if (szISODebug == NULL)
                        {
                                inLogPrintf(AT, "szISODebug == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szISODebug length = (%d)", (int)strlen(srEDCRec.szISODebug));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szISODebug[0], &srEDCRec.szISODebug[0], strlen(srEDCRec.szISODebug));

        return (VS_SUCCESS);
}

/*
Function        :inSetISODebug
Date&Time       :
Describe        :
*/
int inSetISODebug(char* szISODebug)
{
        memset(srEDCRec.szISODebug, 0x00, sizeof(srEDCRec.szISODebug));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szISODebug == NULL || strlen(szISODebug) < 0 || strlen(szISODebug) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetISODebug() ERROR !!");

                        if (szISODebug == NULL)
                        {
                                inLogPrintf(AT, "szISODebug == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szISODebug length = (%d)", (int)strlen(szISODebug));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szISODebug[0], &szISODebug[0], strlen(szISODebug));

        return (VS_SUCCESS);
}

/*
Function        :inGetManagerPassword
Date&Time       :2017/11/17 下午 2:13
Describe        :
*/
int inGetManagerPassword(char* szManagerPassword)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szManagerPassword == NULL || strlen(srEDCRec.szManagerPassword) < 0 || strlen(srEDCRec.szManagerPassword) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPassWord() ERROR !!");

                        if (szManagerPassword == NULL)
                        {
                                inLogPrintf(AT, "szPassWord == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPassWord length = (%d)", (int)strlen(srEDCRec.szManagerPassword));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szManagerPassword[0], &srEDCRec.szManagerPassword[0], strlen(srEDCRec.szManagerPassword));

        return (VS_SUCCESS);
}

/*
Function        :inSetManagerPassword
Date&Time       :2017/11/17 下午 2:12
Describe        :
*/
int inSetManagerPassword(char* szManagerPassword)
{
        memset(srEDCRec.szManagerPassword, 0x00, sizeof(srEDCRec.szManagerPassword));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szManagerPassword == NULL || strlen(szManagerPassword) < 0 || strlen(szManagerPassword) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPassWord() ERROR !!");

                        if (szManagerPassword == NULL)
                        {
                                inLogPrintf(AT, "szPassWord == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPassWord length = (%d)", (int)strlen(szManagerPassword));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szManagerPassword[0], &szManagerPassword[0], strlen(szManagerPassword));

        return (VS_SUCCESS);
}

/*
Function        :inGetFunctionPassword
Date&Time       :2017/11/17 下午 2:13
Describe        :
*/
int inGetFunctionPassword(char* szFunctionPassword)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szFunctionPassword == NULL || strlen(srEDCRec.szFunctionPassword) < 0 || strlen(srEDCRec.szFunctionPassword) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPassWord() ERROR !!");

                        if (szFunctionPassword == NULL)
                        {
                                inLogPrintf(AT, "szPassWord == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPassWord length = (%d)", (int)strlen(srEDCRec.szFunctionPassword));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFunctionPassword[0], &srEDCRec.szFunctionPassword[0], strlen(srEDCRec.szFunctionPassword));

        return (VS_SUCCESS);
}

/*
Function        :inSetFunctionPassword
Date&Time       :2017/11/17 下午 2:12
Describe        :
*/
int inSetFunctionPassword(char* szFunctionPassword)
{
        memset(srEDCRec.szFunctionPassword, 0x00, sizeof(srEDCRec.szFunctionPassword));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szFunctionPassword == NULL || strlen(szFunctionPassword) < 0 || strlen(szFunctionPassword) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPassWord() ERROR !!");

                        if (szFunctionPassword == NULL)
                        {
                                inLogPrintf(AT, "szPassWord == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPassWord length = (%d)", (int)strlen(szFunctionPassword));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szFunctionPassword[0], &szFunctionPassword[0], strlen(szFunctionPassword));

        return (VS_SUCCESS);
}

/*
Function        :inGetMerchantPassword
Date&Time       :2017/11/17 下午 2:13
Describe        :
*/
int inGetMerchantPassword(char* szMerchantPassword)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMerchantPassword == NULL || strlen(srEDCRec.szMerchantPassword) < 0 || strlen(srEDCRec.szMerchantPassword) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPassWord() ERROR !!");

                        if (szMerchantPassword == NULL)
                        {
                                inLogPrintf(AT, "szPassWord == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPassWord length = (%d)", (int)strlen(srEDCRec.szMerchantPassword));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMerchantPassword[0], &srEDCRec.szMerchantPassword[0], strlen(srEDCRec.szMerchantPassword));

        return (VS_SUCCESS);
}

/*
Function        :inSetMerchantPassword
Date&Time       :2017/11/17 下午 2:12
Describe        :
*/
int inSetMerchantPassword(char* szMerchantPassword)
{
        memset(srEDCRec.szMerchantPassword, 0x00, sizeof(srEDCRec.szMerchantPassword));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMerchantPassword == NULL || strlen(szMerchantPassword) < 0 || strlen(szMerchantPassword) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPassWord() ERROR !!");

                        if (szMerchantPassword == NULL)
                        {
                                inLogPrintf(AT, "szPassWord == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPassWord length = (%d)", (int)strlen(szMerchantPassword));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szMerchantPassword[0], &szMerchantPassword[0], strlen(szMerchantPassword));

        return (VS_SUCCESS);
}

/*
Function        :inGetSuperPassword
Date&Time       :2017/11/17 下午 2:13
Describe        :
*/
int inGetSuperPassword(char* szSuperPassword)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSuperPassword == NULL || strlen(srEDCRec.szSuperPassword) < 0 || strlen(srEDCRec.szSuperPassword) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPassWord() ERROR !!");

                        if (szSuperPassword == NULL)
                        {
                                inLogPrintf(AT, "szPassWord == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPassWord length = (%d)", (int)strlen(srEDCRec.szSuperPassword));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSuperPassword[0], &srEDCRec.szSuperPassword[0], strlen(srEDCRec.szSuperPassword));

        return (VS_SUCCESS);
}

/*
Function        :inSetSuperPassword
Date&Time       :2017/11/17 下午 2:12
Describe        :
*/
int inSetSuperPassword(char* szSuperPassword)
{
        memset(srEDCRec.szSuperPassword, 0x00, sizeof(srEDCRec.szSuperPassword));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSuperPassword == NULL || strlen(szSuperPassword) < 0 || strlen(szSuperPassword) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPassWord() ERROR !!");

                        if (szSuperPassword == NULL)
                        {
                                inLogPrintf(AT, "szPassWord == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPassWord length = (%d)", (int)strlen(szSuperPassword));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szSuperPassword[0], &szSuperPassword[0], strlen(szSuperPassword));

        return (VS_SUCCESS);
}

/*
Function        :inGetMCCCode
Date&Time       :
Describe        :
*/
int inGetMCCCode(char* szMCCCode)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMCCCode == NULL || strlen(srEDCRec.szMCCCode) < 0 || strlen(srEDCRec.szMCCCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetMCCCode() ERROR !!");

                        if (szMCCCode == NULL)
                        {
                                inLogPrintf(AT, "szMCCCode == NULL");
                        }
                        else
                        {
                                sprintf(szErrorMsg, "szMCCCode length = (%d)", (int)strlen(srEDCRec.szMCCCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMCCCode[0], &srEDCRec.szMCCCode[0], strlen(srEDCRec.szMCCCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetMCCCode
Date&Time       :
Describe        :
*/
int inSetMCCCode(char* szMCCCode)
{
        memset(srEDCRec.szMCCCode, 0x00, sizeof(srEDCRec.szMCCCode));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMCCCode == NULL || strlen(szMCCCode) < 0 || strlen(szMCCCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetMCCCode() ERROR !!");

                        if (szMCCCode == NULL)
                        {
                                inLogPrintf(AT, "szMCCCode == NULL");
                        }
                        else
                        {
                                sprintf(szErrorMsg, "szMCCCode length = (%d)", (int)strlen(szMCCCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szMCCCode[0], &szMCCCode[0], strlen(szMCCCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetIssuerID
Date&Time       :
Describe        :
*/
int inGetIssuerID(char* szIssuerID)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szIssuerID == NULL || strlen(srEDCRec.szIssuerID) < 0 || strlen(srEDCRec.szIssuerID) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetIssuerID() ERROR !!");

                        if (szIssuerID == NULL)
                        {
                                inLogPrintf(AT, "szIssuerID == NULL");
                        }
                        else
                        {
                                sprintf(szErrorMsg, "szIssuerID length = (%d)", (int)strlen(srEDCRec.szIssuerID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szIssuerID[0], &srEDCRec.szIssuerID[0], strlen(srEDCRec.szIssuerID));

        return (VS_SUCCESS);
}

/*
Function        :inSetIssuerID
Date&Time       :
Describe        :
*/
int inSetIssuerID(char* szIssuerID)
{
        memset(srEDCRec.szIssuerID, 0x00, sizeof(srEDCRec.szIssuerID));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szIssuerID == NULL || strlen(szIssuerID) < 0 || strlen(szIssuerID) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetIssuerID() ERROR !!");

                        if (szIssuerID == NULL)
                        {
                                inLogPrintf(AT, "szIssuerID == NULL");
                        }
                        else
                        {
                                sprintf(szErrorMsg, "szIssuerID length = (%d)", (int)strlen(szIssuerID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szIssuerID[0], &szIssuerID[0], strlen(szIssuerID));

        return (VS_SUCCESS);
}

/*
Function        :inGetEnterTimeout
Date&Time       :
Describe        :
*/
int inGetEnterTimeout(char* szEnterTimeout)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szEnterTimeout == NULL || strlen(srEDCRec.szEnterTimeout) < 0 || strlen(srEDCRec.szEnterTimeout) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetEnterTimeout() ERROR !!");

                        if (szEnterTimeout == NULL)
                        {
                                inLogPrintf(AT, "szEnterTimeout == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEnterTimeout length = (%d)", (int)strlen(srEDCRec.szEnterTimeout));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szEnterTimeout[0], &srEDCRec.szEnterTimeout[0], strlen(srEDCRec.szEnterTimeout));

        return (VS_SUCCESS);
}

/*
Function        :inSetEnterTimeout
Date&Time       :
Describe        :
*/
int inSetEnterTimeout(char* szEnterTimeout)
{
        memset(srEDCRec.szEnterTimeout, 0x00, sizeof(srEDCRec.szEnterTimeout));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szEnterTimeout == NULL || strlen(szEnterTimeout) < 0 || strlen(szEnterTimeout) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetEnterTimeout() ERROR !!");

                        if (szEnterTimeout == NULL)
                        {
                                inLogPrintf(AT, "szEnterTimeout == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEnterTimeout length = (%d)", (int)strlen(szEnterTimeout));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szEnterTimeout[0], &szEnterTimeout[0], strlen(szEnterTimeout));

        return (VS_SUCCESS);
}

/*
Function        :inGetIPSendTimeout
Date&Time       :
Describe        :
*/
int inGetIPSendTimeout(char* szIPSendTimeout)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szIPSendTimeout == NULL || strlen(srEDCRec.szIPSendTimeout) < 0 || strlen(srEDCRec.szIPSendTimeout) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetIPSendTimeout() ERROR !!");

                        if (szIPSendTimeout == NULL)
                        {
                                inLogPrintf(AT, "szIPSendTimeout == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIPSendTimeout length = (%d)", (int)strlen(srEDCRec.szIPSendTimeout));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szIPSendTimeout[0], &srEDCRec.szIPSendTimeout[0], strlen(srEDCRec.szIPSendTimeout));

        return (VS_SUCCESS);
}

/*
Function        :inSetIPSendTimeout
Date&Time       :
Describe        :
*/
int inSetIPSendTimeout(char* szIPSendTimeout)
{
        memset(srEDCRec.szIPSendTimeout, 0x00, sizeof(srEDCRec.szIPSendTimeout));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szIPSendTimeout == NULL || strlen(szIPSendTimeout) < 0 || strlen(szIPSendTimeout) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetIPSendTimeout() ERROR !!");

                        if (szIPSendTimeout == NULL)
                        {
                                inLogPrintf(AT, "szIPSendTimeout == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIPSendTimeout length = (%d)", (int)strlen(szIPSendTimeout));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szIPSendTimeout[0], &szIPSendTimeout[0], strlen(szIPSendTimeout));

        return (VS_SUCCESS);
}

/*
Function        :inGetTermVersionID
Date&Time       :
Describe        :
*/
int inGetTermVersionID(char* szTermVersionID)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTermVersionID == NULL || strlen(srEDCRec.szTermVersionID) < 0 || strlen(srEDCRec.szTermVersionID) > 15)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTermVersionID() ERROR !!");

                        if (szTermVersionID == NULL)
                        {
                                inLogPrintf(AT, "szTermVersionID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTermVersionID length = (%d)", (int)strlen(srEDCRec.szTermVersionID));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szTermVersionID[0], &srEDCRec.szTermVersionID[0], strlen(srEDCRec.szTermVersionID));

        return (VS_SUCCESS);
}

/*
Function        :inSetTermVersionID
Date&Time       :
Describe        :
*/
int inSetTermVersionID(char* szTermVersionID)
{
        memset(srEDCRec.szTermVersionID, 0x00, sizeof(srEDCRec.szTermVersionID));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTermVersionID == NULL || strlen(szTermVersionID) < 0 || strlen(szTermVersionID) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTermVersionID() ERROR !!");

                        if (szTermVersionID == NULL)
                        {
                                inLogPrintf(AT, "szTermVersionID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTermVersionID length = (%d)", (int)strlen(szTermVersionID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szTermVersionID[0], &szTermVersionID[0], strlen(szTermVersionID));

        return (VS_SUCCESS);
}

/*
Function        :inGetTermVersionDate
Date&Time       :
Describe        :
*/
int inGetTermVersionDate(char* szTermVersionDate)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTermVersionDate == NULL || strlen(srEDCRec.szTermVersionDate) < 0 || strlen(srEDCRec.szTermVersionDate) > 15)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTermVersionDate() ERROR !!");

                        if (szTermVersionDate == NULL)
                        {
                                inLogPrintf(AT, "szTermVersionDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTermVersionDate length = (%d)", (int)strlen(srEDCRec.szTermVersionDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szTermVersionDate[0], &srEDCRec.szTermVersionDate[0], strlen(srEDCRec.szTermVersionDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetTermVersionDate
Date&Time       :
Describe        :
*/
int inSetTermVersionDate(char* szTermVersionDate)
{
        memset(srEDCRec.szTermVersionDate, 0x00, sizeof(srEDCRec.szTermVersionDate));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTermVersionDate == NULL || strlen(szTermVersionDate) < 0 || strlen(szTermVersionDate) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTermVersionDate() ERROR !!");

                        if (szTermVersionDate == NULL)
                        {
                                inLogPrintf(AT, "szTermVersionDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTermVersionDate length = (%d)", (int)strlen(szTermVersionDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szTermVersionDate[0], &szTermVersionDate[0], strlen(szTermVersionDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetTermIPAddress
Date&Time       :
Describe        :
*/
int inGetTermIPAddress(char* szTermIPAddress)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTermIPAddress == NULL || strlen(srEDCRec.szTermIPAddress) < 0 || strlen(srEDCRec.szTermIPAddress) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        if (szTermIPAddress == NULL)
                        {
                                inLogPrintf(AT, "inGetTermIPAddress() ERROR !! szTermIPAddress == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "inGetTermIPAddress() ERROR !! szTermIPAddress length = (%d)", (int)strlen(srEDCRec.szTermIPAddress));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTermIPAddress[0], &srEDCRec.szTermIPAddress[0], strlen(srEDCRec.szTermIPAddress));

        return (VS_SUCCESS);
}

/*
Function        :inSetTermIPAddress
Date&Time       :
Describe        :
*/
int inSetTermIPAddress(char* szTermIPAddress)
{
        memset(srEDCRec.szTermIPAddress, 0x00, sizeof(srEDCRec.szTermIPAddress));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTermIPAddress == NULL || strlen(szTermIPAddress) < 0 || strlen(szTermIPAddress) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTermIPAddress() ERROR !!");

                        if (szTermIPAddress == NULL)
                        {
                                inLogPrintf(AT, "szTermIPAddress == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTermIPAddress length = (%d)", (int)strlen(szTermIPAddress));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szTermIPAddress[0], &szTermIPAddress[0], strlen(szTermIPAddress));

        return (VS_SUCCESS);
}

/*
Function        :inGetTermGetewayAddress
Date&Time       :
Describe        :
*/
int inGetTermGetewayAddress(char* szTermGetewayAddress)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTermGetewayAddress == NULL || strlen(srEDCRec.szTermGetewayAddress) < 0 || strlen(srEDCRec.szTermGetewayAddress) > 15)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTermGetewayAddress() ERROR !!");

                        if (szTermGetewayAddress == NULL)
                        {
                                inLogPrintf(AT, "szTermGetewayAddress == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTermGetewayAddress length = (%d)", (int)strlen(srEDCRec.szTermGetewayAddress));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }

                return (VS_ERROR);
        }
        memcpy(&szTermGetewayAddress[0], &srEDCRec.szTermGetewayAddress[0], strlen(srEDCRec.szTermGetewayAddress));

        return (VS_SUCCESS);
}

/*
Function        :inSetTermGetewayAddress
Date&Time       :
Describe        :
*/
int inSetTermGetewayAddress(char* szTermGetewayAddress)
{
        memset(srEDCRec.szTermGetewayAddress, 0x00, sizeof(srEDCRec.szTermGetewayAddress));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTermGetewayAddress == NULL || strlen(szTermGetewayAddress) < 0 || strlen(szTermGetewayAddress) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTermGetewayAddress() ERROR !!");

                        if (szTermGetewayAddress == NULL)
                        {
                                inLogPrintf(AT, "szTermGetewayAddress == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTermGetewayAddress length = (%d)", (int)strlen(szTermGetewayAddress));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szTermGetewayAddress[0], &szTermGetewayAddress[0], strlen(szTermGetewayAddress));

        return (VS_SUCCESS);
}

/*
Function        :inGetTermMASKAddress
Date&Time       :
Describe        :
*/
int inGetTermMASKAddress(char* szTermMASKAddress)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTermMASKAddress == NULL || strlen(srEDCRec.szTermMASKAddress) < 0 || strlen(srEDCRec.szTermMASKAddress) > 15)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTermMASKAddress() ERROR !!");

                        if (szTermMASKAddress == NULL)
                        {
                                inLogPrintf(AT, "szTermMASKAddress == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTermMASKAddress length = (%d)", (int)strlen(srEDCRec.szTermMASKAddress));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szTermMASKAddress[0], &srEDCRec.szTermMASKAddress[0], strlen(srEDCRec.szTermMASKAddress));

        return (VS_SUCCESS);
}

/*
Function        :inSetTermMASKAddress
Date&Time       :
Describe        :
*/
int inSetTermMASKAddress(char* szTermMASKAddress)
{
        memset(srEDCRec.szTermMASKAddress, 0x00, sizeof(srEDCRec.szTermMASKAddress));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTermMASKAddress == NULL || strlen(szTermMASKAddress) < 0 || strlen(szTermMASKAddress) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTermMASKAddress() ERROR !!");

                        if (szTermMASKAddress == NULL)
                        {
                                inLogPrintf(AT, "szTermMASKAddress == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTermMASKAddress length = (%d)", (int)strlen(szTermMASKAddress));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szTermMASKAddress[0], &szTermMASKAddress[0], strlen(szTermMASKAddress));

        return (VS_SUCCESS);
}

/*
Function        :inGetTermECRPort
Date&Time       :2017/6/5 下午 2:24
Describe        :
*/
int inGetTermECRPort(char* szTermECRPort)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTermECRPort == NULL || strlen(srEDCRec.szTermECRPort) < 0 || strlen(srEDCRec.szTermECRPort) > 6)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTermECRPort() ERROR !!");

                        if (szTermECRPort == NULL)
                        {
                                inLogPrintf(AT, "szTermECRPort == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTermECRPort length = (%d)", (int)strlen(srEDCRec.szTermECRPort));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szTermECRPort[0], &srEDCRec.szTermECRPort[0], strlen(srEDCRec.szTermECRPort));

        return (VS_SUCCESS);
}

/*
Function        :inSetTermECRPort
Date&Time       :2017/6/5 下午 2:24
Describe        :
*/
int inSetTermECRPort(char* szTermECRPort)
{
        memset(srEDCRec.szTermECRPort, 0x00, sizeof(srEDCRec.szTermECRPort));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTermECRPort == NULL || strlen(szTermECRPort) < 0 || strlen(szTermECRPort) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTermECRPort() ERROR !!");

                        if (szTermECRPort == NULL)
                        {
                                inLogPrintf(AT, "szTermECRPort == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTermECRPort length = (%d)", (int)strlen(szTermECRPort));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szTermECRPort[0], &szTermECRPort[0], strlen(szTermECRPort));

        return (VS_SUCCESS);
}

/*
Function        :inGetESCMode
Date&Time       :2017/4/11 下午 1:25
Describe        :
*/
int inGetESCMode(char* szESCMode)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szESCMode == NULL || strlen(srEDCRec.szESCMode) < 0 || strlen(srEDCRec.szESCMode) > 2)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetESCMode() ERROR !!");

                        if (szESCMode == NULL)
                        {
                                inLogPrintf(AT, "szESCMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szESCMode length = (%d)", (int)strlen(srEDCRec.szESCMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szESCMode[0], &srEDCRec.szESCMode[0], strlen(srEDCRec.szESCMode));

        return (VS_SUCCESS);
}

/*
Function        :inSetESCMode
Date&Time       :2017/4/11 下午 1:25
Describe        :
*/
int inSetESCMode(char* szESCMode)
{
        memset(srEDCRec.szESCMode, 0x00, sizeof(srEDCRec.szESCMode));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szESCMode == NULL || strlen(szESCMode) < 0 || strlen(szESCMode) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetESCMode() ERROR !!");

                        if (szESCMode == NULL)
                        {
                                inLogPrintf(AT, "szESCMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szESCMode length = (%d)", (int)strlen(szESCMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szESCMode[0], &szESCMode[0], strlen(szESCMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetMultiComPort1
Date&Time       :2017/6/30 下午 3:53
Describe        :
*/
int inGetMultiComPort1(char* szMultiComPort1)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMultiComPort1 == NULL || strlen(srEDCRec.szMultiComPort1) < 0 || strlen(srEDCRec.szMultiComPort1) > 4)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetMultiComPort1() ERROR !!");

                        if (szMultiComPort1 == NULL)
                        {
                                inLogPrintf(AT, "szMultiComPort1 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMultiComPort1 length = (%d)", (int)strlen(srEDCRec.szMultiComPort1));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szMultiComPort1[0], &srEDCRec.szMultiComPort1[0], strlen(srEDCRec.szMultiComPort1));

        return (VS_SUCCESS);
}

/*
Function        :inSetMultiComPort1
Date&Time       :2017/6/30 下午 3:53
Describe        :
*/
int inSetMultiComPort1(char* szMultiComPort1)
{
        memset(srEDCRec.szMultiComPort1, 0x00, sizeof(srEDCRec.szMultiComPort1));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMultiComPort1 == NULL || strlen(szMultiComPort1) < 0 || strlen(szMultiComPort1) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetMultiComPort1() ERROR !!");

                        if (szMultiComPort1 == NULL)
                        {
                                inLogPrintf(AT, "szMultiComPort1 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMultiComPort1 length = (%d)", (int)strlen(szMultiComPort1));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szMultiComPort1[0], &szMultiComPort1[0], strlen(szMultiComPort1));

        return (VS_SUCCESS);
}

/*
Function        :inGetMultiComPort1Version
Date&Time       :2017/7/3 上午 10:50
Describe        :
*/
int inGetMultiComPort1Version(char* szMultiComPort1Version)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMultiComPort1Version == NULL || strlen(srEDCRec.szMultiComPort1Version) < 0 || strlen(srEDCRec.szMultiComPort1Version) > 2)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetMultiComPort1Version() ERROR !!");

                        if (szMultiComPort1Version == NULL)
                        {
                                inLogPrintf(AT, "szMultiComPort1Version == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMultiComPort1Version length = (%d)", (int)strlen(srEDCRec.szMultiComPort1Version));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szMultiComPort1Version[0], &srEDCRec.szMultiComPort1Version[0], strlen(srEDCRec.szMultiComPort1Version));

        return (VS_SUCCESS);
}

/*
Function        :inSetMultiComPort1Version
Date&Time       :2017/7/3 上午 10:50
Describe        :
*/
int inSetMultiComPort1Version(char* szMultiComPort1Version)
{
        memset(srEDCRec.szMultiComPort1Version, 0x00, sizeof(srEDCRec.szMultiComPort1Version));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMultiComPort1Version == NULL || strlen(szMultiComPort1Version) < 0 || strlen(szMultiComPort1Version) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetMultiComPort1Version() ERROR !!");

                        if (szMultiComPort1Version == NULL)
                        {
                                inLogPrintf(AT, "szMultiComPort1Version == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMultiComPort1Version length = (%d)", (int)strlen(szMultiComPort1Version));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szMultiComPort1Version[0], &szMultiComPort1Version[0], strlen(szMultiComPort1Version));

        return (VS_SUCCESS);
}

/*
Function        :inGetMultiComPort2
Date&Time       :2017/6/30 下午 3:53
Describe        :
*/
int inGetMultiComPort2(char* szMultiComPort2)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMultiComPort2 == NULL || strlen(srEDCRec.szMultiComPort2) < 0 || strlen(srEDCRec.szMultiComPort2) > 4)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetMultiComPort2() ERROR !!");

                        if (szMultiComPort2 == NULL)
                        {
                                inLogPrintf(AT, "szMultiComPort2 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMultiComPort2 length = (%d)", (int)strlen(srEDCRec.szMultiComPort2));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szMultiComPort2[0], &srEDCRec.szMultiComPort2[0], strlen(srEDCRec.szMultiComPort2));

        return (VS_SUCCESS);
}

/*
Function        :inSetMultiComPort2
Date&Time       :2017/6/30 下午 3:53
Describe        :
*/
int inSetMultiComPort2(char* szMultiComPort2)
{
        memset(srEDCRec.szMultiComPort2, 0x00, sizeof(srEDCRec.szMultiComPort2));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMultiComPort2 == NULL || strlen(szMultiComPort2) < 0 || strlen(szMultiComPort2) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetMultiComPort2() ERROR !!");

                        if (szMultiComPort2 == NULL)
                        {
                                inLogPrintf(AT, "szMultiComPort2 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMultiComPort2 length = (%d)", (int)strlen(szMultiComPort2));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szMultiComPort2[0], &szMultiComPort2[0], strlen(szMultiComPort2));

        return (VS_SUCCESS);
}

/*
Function        :inGetMultiComPort2Version
Date&Time       :2017/7/3 上午 10:50
Describe        :
*/
int inGetMultiComPort2Version(char* szMultiComPort2Version)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMultiComPort2Version == NULL || strlen(srEDCRec.szMultiComPort2Version) < 0 || strlen(srEDCRec.szMultiComPort2Version) > 2)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetMultiComPort2Version() ERROR !!");

                        if (szMultiComPort2Version == NULL)
                        {
                                inLogPrintf(AT, "szMultiComPort2Version == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMultiComPort2Version length = (%d)", (int)strlen(srEDCRec.szMultiComPort2Version));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szMultiComPort2Version[0], &srEDCRec.szMultiComPort2Version[0], strlen(srEDCRec.szMultiComPort2Version));

        return (VS_SUCCESS);
}

/*
Function        :inSetMultiComPort2Version
Date&Time       :2017/7/3 上午 10:50
Describe        :
*/
int inSetMultiComPort2Version(char* szMultiComPort2Version)
{
        memset(srEDCRec.szMultiComPort2Version, 0x00, sizeof(srEDCRec.szMultiComPort2Version));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMultiComPort2Version == NULL || strlen(szMultiComPort2Version) < 0 || strlen(szMultiComPort2Version) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetMultiComPort2Version() ERROR !!");

                        if (szMultiComPort2Version == NULL)
                        {
                                inLogPrintf(AT, "szMultiComPort2Version == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMultiComPort2Version length = (%d)", (int)strlen(szMultiComPort2Version));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szMultiComPort2Version[0], &szMultiComPort2Version[0], strlen(szMultiComPort2Version));

        return (VS_SUCCESS);
}

/*
Function        :inGetMultiComPort3
Date&Time       :2017/6/30 下午 3:53
Describe        :
*/
int inGetMultiComPort3(char* szMultiComPort3)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMultiComPort3 == NULL || strlen(srEDCRec.szMultiComPort3) < 0 || strlen(srEDCRec.szMultiComPort3) > 4)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetMultiComPort3() ERROR !!");

                        if (szMultiComPort3 == NULL)
                        {
                                inLogPrintf(AT, "szMultiComPort3 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMultiComPort3 length = (%d)", (int)strlen(srEDCRec.szMultiComPort3));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szMultiComPort3[0], &srEDCRec.szMultiComPort3[0], strlen(srEDCRec.szMultiComPort3));

        return (VS_SUCCESS);
}

/*
Function        :inSetMultiComPort3
Date&Time       :2017/6/30 下午 3:53
Describe        :
*/
int inSetMultiComPort3(char* szMultiComPort3)
{
        memset(srEDCRec.szMultiComPort3, 0x00, sizeof(srEDCRec.szMultiComPort3));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMultiComPort3 == NULL || strlen(szMultiComPort3) < 0 || strlen(szMultiComPort3) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetMultiComPort3() ERROR !!");

                        if (szMultiComPort3 == NULL)
                        {
                                inLogPrintf(AT, "szMultiComPort3 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMultiComPort3 length = (%d)", (int)strlen(szMultiComPort3));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szMultiComPort3[0], &szMultiComPort3[0], strlen(szMultiComPort3));

        return (VS_SUCCESS);
}

/*
Function        :inGetMultiComPort3Version
Date&Time       :2017/7/3 上午 10:50
Describe        :
*/
int inGetMultiComPort3Version(char* szMultiComPort3Version)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMultiComPort3Version == NULL || strlen(srEDCRec.szMultiComPort3Version) < 0 || strlen(srEDCRec.szMultiComPort3Version) > 2)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetMultiComPort3Version() ERROR !!");

                        if (szMultiComPort3Version == NULL)
                        {
                                inLogPrintf(AT, "szMultiComPort3Version == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMultiComPort3Version length = (%d)", (int)strlen(srEDCRec.szMultiComPort3Version));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szMultiComPort3Version[0], &srEDCRec.szMultiComPort3Version[0], strlen(srEDCRec.szMultiComPort3Version));

        return (VS_SUCCESS);
}

/*
Function        :inSetMultiComPort3Version
Date&Time       :2017/7/3 上午 10:50
Describe        :
*/
int inSetMultiComPort3Version(char* szMultiComPort3Version)
{
        memset(srEDCRec.szMultiComPort3Version, 0x00, sizeof(srEDCRec.szMultiComPort3Version));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMultiComPort3Version == NULL || strlen(szMultiComPort3Version) < 0 || strlen(szMultiComPort3Version) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetMultiComPort3Version() ERROR !!");

                        if (szMultiComPort3Version == NULL)
                        {
                                inLogPrintf(AT, "szMultiComPort3Version == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMultiComPort3Version length = (%d)", (int)strlen(szMultiComPort3Version));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szMultiComPort3Version[0], &szMultiComPort3Version[0], strlen(szMultiComPort3Version));

        return (VS_SUCCESS);
}

/*
Function        :inGetEMVForceOnline
Date&Time       :2017/9/4 上午 10:42
Describe        :
*/
int inGetEMVForceOnline(char* szEMVForceOnline)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szEMVForceOnline == NULL || strlen(srEDCRec.szEMVForceOnline) < 0 || strlen(srEDCRec.szEMVForceOnline) > 2)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetEMVForceOnline() ERROR !!");

                        if (szEMVForceOnline == NULL)
                        {
                                inLogPrintf(AT, "szEMVForceOnline == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEMVForceOnline length = (%d)", (int)strlen(srEDCRec.szEMVForceOnline));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szEMVForceOnline[0], &srEDCRec.szEMVForceOnline[0], strlen(srEDCRec.szEMVForceOnline));

        return (VS_SUCCESS);
}

/*
Function        :inSetEMVForceOnline
Date&Time       :2017/7/3 上午 10:50
Describe        :
*/
int inSetEMVForceOnline(char* szEMVForceOnline)
{
        memset(srEDCRec.szEMVForceOnline, 0x00, sizeof(srEDCRec.szEMVForceOnline));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szEMVForceOnline == NULL || strlen(szEMVForceOnline) < 0 || strlen(szEMVForceOnline) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetEMVForceOnline() ERROR !!");

                        if (szEMVForceOnline == NULL)
                        {
                                inLogPrintf(AT, "szEMVForceOnline == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEMVForceOnline length = (%d)", (int)strlen(szEMVForceOnline));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szEMVForceOnline[0], &szEMVForceOnline[0], strlen(szEMVForceOnline));

        return (VS_SUCCESS);
}

/*
Function        :inGetAutoConnect
Date&Time       :2017/10/2 上午 11:47
Describe        :
*/
int inGetAutoConnect(char* szAutoConnect)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szAutoConnect == NULL || strlen(srEDCRec.szAutoConnect) < 0 || strlen(srEDCRec.szAutoConnect) > 10)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetAutoConnect() ERROR !!");

                        if (szAutoConnect == NULL)
                        {
                                inLogPrintf(AT, "szAutoConnect == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szAutoConnect length = (%d)", (int)strlen(srEDCRec.szAutoConnect));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szAutoConnect[0], &srEDCRec.szAutoConnect[0], strlen(srEDCRec.szAutoConnect));

        return (VS_SUCCESS);
}

/*
Function        :inSetAutoConnect
Date&Time       :2017/10/2 上午 11:47
Describe        :
*/
int inSetAutoConnect(char* szAutoConnect)
{
        memset(srEDCRec.szAutoConnect, 0x00, sizeof(srEDCRec.szAutoConnect));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szAutoConnect == NULL || strlen(szAutoConnect) < 0 || strlen(szAutoConnect) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetAutoConnect() ERROR !!");

                        if (szAutoConnect == NULL)
                        {
                                inLogPrintf(AT, "szAutoConnect == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szAutoConnect length = (%d)", (int)strlen(szAutoConnect));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szAutoConnect[0], &szAutoConnect[0], strlen(szAutoConnect));

        return (VS_SUCCESS);
}

/*
Function        :inGetLOGONum
Date&Time       :2017/10/2 上午 11:47
Describe        :
*/
int inGetLOGONum(char* szLOGONum)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szLOGONum == NULL || strlen(srEDCRec.szLOGONum) < 0 || strlen(srEDCRec.szLOGONum) > 4)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetLOGONum() ERROR !!");

                        if (szLOGONum == NULL)
                        {
                                inLogPrintf(AT, "szLOGONum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szLOGONum length = (%d)", (int)strlen(srEDCRec.szLOGONum));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szLOGONum[0], &srEDCRec.szLOGONum[0], strlen(srEDCRec.szLOGONum));

        return (VS_SUCCESS);
}

/*
Function        :inSetLOGONum
Date&Time       :2017/10/2 上午 11:47
Describe        :
*/
int inSetLOGONum(char* szLOGONum)
{
        memset(srEDCRec.szLOGONum, 0x00, sizeof(srEDCRec.szLOGONum));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szLOGONum == NULL || strlen(szLOGONum) < 0 || strlen(szLOGONum) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetLOGONum() ERROR !!");

                        if (szLOGONum == NULL)
                        {
                                inLogPrintf(AT, "szLOGONum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szLOGONum length = (%d)", (int)strlen(szLOGONum));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szLOGONum[0], &szLOGONum[0], strlen(szLOGONum));

        return (VS_SUCCESS);
}

/*
Function        :inGetHostSAMSlot
Date&Time       :2018/1/11 下午 4:30
Describe        :
*/
int inGetHostSAMSlot(char* szHostSAMSlot)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostSAMSlot == NULL || strlen(srEDCRec.szHostSAMSlot) < 0 || strlen(srEDCRec.szHostSAMSlot) > 2)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetHostSAMSlot() ERROR !!");

                        if (szHostSAMSlot == NULL)
                        {
                                inLogPrintf(AT, "szHostSAMSlot == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szHostSAMSlot length = (%d)", (int)strlen(srEDCRec.szHostSAMSlot));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szHostSAMSlot[0], &srEDCRec.szHostSAMSlot[0], strlen(srEDCRec.szHostSAMSlot));

        return (VS_SUCCESS);
}

/*
Function        :inSetHostSAMSlot
Date&Time       :2018/1/11 下午 4:30
Describe        :
*/
int inSetHostSAMSlot(char* szHostSAMSlot)
{
        memset(srEDCRec.szHostSAMSlot, 0x00, sizeof(srEDCRec.szHostSAMSlot));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostSAMSlot == NULL || strlen(szHostSAMSlot) < 0 || strlen(szHostSAMSlot) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetHostSAMSlot() ERROR !!");

                        if (szHostSAMSlot == NULL)
                        {
                                inLogPrintf(AT, "szHostSAMSlot == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szHostSAMSlot length = (%d)", (int)strlen(szHostSAMSlot));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szHostSAMSlot[0], &szHostSAMSlot[0], strlen(szHostSAMSlot));

        return (VS_SUCCESS);
}

/*
Function        :inGetSAMSlotSN1
Date&Time       :2018/1/11 下午 4:30
Describe        :
*/
int inGetSAMSlotSN1(char* szSAMSlotSN1)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSAMSlotSN1 == NULL || strlen(srEDCRec.szSAMSlotSN1) < 0 || strlen(srEDCRec.szSAMSlotSN1) > 16)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetSAMSlotSN1() ERROR !!");

                        if (szSAMSlotSN1 == NULL)
                        {
                                inLogPrintf(AT, "szSAMSlotSN1 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSAMSlotSN1 length = (%d)", (int)strlen(srEDCRec.szSAMSlotSN1));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szSAMSlotSN1[0], &srEDCRec.szSAMSlotSN1[0], strlen(srEDCRec.szSAMSlotSN1));

        return (VS_SUCCESS);
}

/*
Function        :inSetSAMSlotSN1
Date&Time       :2018/1/11 下午 4:30
Describe        :
*/
int inSetSAMSlotSN1(char* szSAMSlotSN1)
{
        memset(srEDCRec.szSAMSlotSN1, 0x00, sizeof(srEDCRec.szSAMSlotSN1));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSAMSlotSN1 == NULL || strlen(szSAMSlotSN1) < 0 || strlen(szSAMSlotSN1) > 16)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetSAMSlotSN1() ERROR !!");

                        if (szSAMSlotSN1 == NULL)
                        {
                                inLogPrintf(AT, "szSAMSlotSN1 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSAMSlotSN1 length = (%d)", (int)strlen(szSAMSlotSN1));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szSAMSlotSN1[0], &szSAMSlotSN1[0], strlen(szSAMSlotSN1));

        return (VS_SUCCESS);
}

/*
Function        :inGetSAMSlotSN2
Date&Time       :2018/1/11 下午 4:30
Describe        :
*/
int inGetSAMSlotSN2(char* szSAMSlotSN2)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSAMSlotSN2 == NULL || strlen(srEDCRec.szSAMSlotSN2) < 0 || strlen(srEDCRec.szSAMSlotSN2) > 16)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetSAMSlotSN2() ERROR !!");

                        if (szSAMSlotSN2 == NULL)
                        {
                                inLogPrintf(AT, "szSAMSlotSN2 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSAMSlotSN2 length = (%d)", (int)strlen(srEDCRec.szSAMSlotSN2));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szSAMSlotSN2[0], &srEDCRec.szSAMSlotSN2[0], strlen(srEDCRec.szSAMSlotSN2));

        return (VS_SUCCESS);
}

/*
Function        :inSetSAMSlotSN2
Date&Time       :2018/1/11 下午 4:30
Describe        :
*/
int inSetSAMSlotSN2(char* szSAMSlotSN2)
{
        memset(srEDCRec.szSAMSlotSN2, 0x00, sizeof(srEDCRec.szSAMSlotSN2));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSAMSlotSN2 == NULL || strlen(szSAMSlotSN2) < 0 || strlen(szSAMSlotSN2) > 16)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetSAMSlotSN2() ERROR !!");

                        if (szSAMSlotSN2 == NULL)
                        {
                                inLogPrintf(AT, "szSAMSlotSN2 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSAMSlotSN2 length = (%d)", (int)strlen(szSAMSlotSN2));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szSAMSlotSN2[0], &szSAMSlotSN2[0], strlen(szSAMSlotSN2));

        return (VS_SUCCESS);
}

/*
Function        :inGetSAMSlotSN3
Date&Time       :2018/1/11 下午 4:30
Describe        :
*/
int inGetSAMSlotSN3(char* szSAMSlotSN3)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSAMSlotSN3 == NULL || strlen(srEDCRec.szSAMSlotSN3) < 0 || strlen(srEDCRec.szSAMSlotSN3) > 16)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetSAMSlotSN3() ERROR !!");

                        if (szSAMSlotSN3 == NULL)
                        {
                                inLogPrintf(AT, "szSAMSlotSN3 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSAMSlotSN3 length = (%d)", (int)strlen(srEDCRec.szSAMSlotSN3));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szSAMSlotSN3[0], &srEDCRec.szSAMSlotSN3[0], strlen(srEDCRec.szSAMSlotSN3));

        return (VS_SUCCESS);
}

/*
Function        :inSetSAMSlotSN3
Date&Time       :2018/1/11 下午 4:30
Describe        :
*/
int inSetSAMSlotSN3(char* szSAMSlotSN3)
{
        memset(srEDCRec.szSAMSlotSN3, 0x00, sizeof(srEDCRec.szSAMSlotSN3));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSAMSlotSN3 == NULL || strlen(szSAMSlotSN3) < 0 || strlen(szSAMSlotSN3) > 16)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetSAMSlotSN3() ERROR !!");

                        if (szSAMSlotSN3 == NULL)
                        {
                                inLogPrintf(AT, "szSAMSlotSN3 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSAMSlotSN3 length = (%d)", (int)strlen(szSAMSlotSN3));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szSAMSlotSN3[0], &szSAMSlotSN3[0], strlen(szSAMSlotSN3));

        return (VS_SUCCESS);
}

/*
Function        :inGetSAMSlotSN4
Date&Time       :2018/1/11 下午 4:30
Describe        :
*/
int inGetSAMSlotSN4(char* szSAMSlotSN4)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSAMSlotSN4 == NULL || strlen(srEDCRec.szSAMSlotSN4) < 0 || strlen(srEDCRec.szSAMSlotSN4) > 16)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetSAMSlotSN4() ERROR !!");

                        if (szSAMSlotSN4 == NULL)
                        {
                                inLogPrintf(AT, "szSAMSlotSN4 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSAMSlotSN4 length = (%d)", (int)strlen(srEDCRec.szSAMSlotSN4));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szSAMSlotSN4[0], &srEDCRec.szSAMSlotSN4[0], strlen(srEDCRec.szSAMSlotSN4));

        return (VS_SUCCESS);
}

/*
Function        :inSetSAMSlotSN4
Date&Time       :2018/1/11 下午 4:30
Describe        :
*/
int inSetSAMSlotSN4(char* szSAMSlotSN4)
{
        memset(srEDCRec.szSAMSlotSN4, 0x00, sizeof(srEDCRec.szSAMSlotSN4));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSAMSlotSN4 == NULL || strlen(szSAMSlotSN4) < 0 || strlen(szSAMSlotSN4) > 16)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetSAMSlotSN4() ERROR !!");

                        if (szSAMSlotSN4 == NULL)
                        {
                                inLogPrintf(AT, "szSAMSlotSN4 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSAMSlotSN4 length = (%d)", (int)strlen(szSAMSlotSN4));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szSAMSlotSN4[0], &szSAMSlotSN4[0], strlen(szSAMSlotSN4));

        return (VS_SUCCESS);
}

/*
Function        :inGetPWMEnable
Date&Time       :2018/4/11 下午 3:00
Describe        :
*/
int inGetPWMEnable(char* szPWMEnable)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPWMEnable == NULL || strlen(srEDCRec.szPWMEnable) < 0 || strlen(srEDCRec.szPWMEnable) > 2)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPWMEnable() ERROR !!");

                        if (szPWMEnable == NULL)
                        {
                                inLogPrintf(AT, "szPWMEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPWMEnable length = (%d)", (int)strlen(srEDCRec.szPWMEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szPWMEnable[0], &srEDCRec.szPWMEnable[0], strlen(srEDCRec.szPWMEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetPWMEnable
Date&Time       :2018/4/11 下午 3:00
Describe        :
*/
int inSetPWMEnable(char* szPWMEnable)
{
        memset(srEDCRec.szPWMEnable, 0x00, sizeof(srEDCRec.szPWMEnable));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPWMEnable == NULL || strlen(szPWMEnable) < 0 || strlen(szPWMEnable) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPWMEnable() ERROR !!");

                        if (szPWMEnable == NULL)
                        {
                                inLogPrintf(AT, "szPWMEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPWMEnable length = (%d)", (int)strlen(szPWMEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szPWMEnable[0], &szPWMEnable[0], strlen(szPWMEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetPWMMode
Date&Time       :2018/4/11 下午 3:04
Describe        :
*/
int inGetPWMMode(char* szPWMMode)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPWMMode == NULL || strlen(srEDCRec.szPWMMode) < 0 || strlen(srEDCRec.szPWMMode) > 2)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPWMMode() ERROR !!");

                        if (szPWMMode == NULL)
                        {
                                inLogPrintf(AT, "szPWMMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPWMMode length = (%d)", (int)strlen(srEDCRec.szPWMMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szPWMMode[0], &srEDCRec.szPWMMode[0], strlen(srEDCRec.szPWMMode));

        return (VS_SUCCESS);
}

/*
Function        :inSetPWMMode
Date&Time       :2018/4/11 下午 3:05
Describe        :
*/
int inSetPWMMode(char* szPWMMode)
{
        memset(srEDCRec.szPWMMode, 0x00, sizeof(srEDCRec.szPWMMode));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPWMMode == NULL || strlen(szPWMMode) < 0 || strlen(szPWMMode) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPWMMode() ERROR !!");

                        if (szPWMMode == NULL)
                        {
                                inLogPrintf(AT, "szPWMMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPWMMode length = (%d)", (int)strlen(szPWMMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szPWMMode[0], &szPWMMode[0], strlen(szPWMMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetPWMIdleTimeout
Date&Time       :2018/4/11 下午 3:06
Describe        :
*/
int inGetPWMIdleTimeout(char* szPWMIdleTimeout)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPWMIdleTimeout == NULL || strlen(srEDCRec.szPWMIdleTimeout) < 0 || strlen(srEDCRec.szPWMIdleTimeout) > 4)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPWMIdleTimeout() ERROR !!");

                        if (szPWMIdleTimeout == NULL)
                        {
                                inLogPrintf(AT, "szPWMIdleTimeout == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPWMIdleTimeout length = (%d)", (int)strlen(srEDCRec.szPWMIdleTimeout));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szPWMIdleTimeout[0], &srEDCRec.szPWMIdleTimeout[0], strlen(srEDCRec.szPWMIdleTimeout));

        return (VS_SUCCESS);
}

/*
Function        :inSetPWMIdleTimeout
Date&Time       :2018/4/11 下午 3:06
Describe        :
*/
int inSetPWMIdleTimeout(char* szPWMIdleTimeout)
{
        memset(srEDCRec.szPWMIdleTimeout, 0x00, sizeof(srEDCRec.szPWMIdleTimeout));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPWMIdleTimeout == NULL || strlen(szPWMIdleTimeout) < 0 || strlen(szPWMIdleTimeout) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPWMIdleTimeout() ERROR !!");

                        if (szPWMIdleTimeout == NULL)
                        {
                                inLogPrintf(AT, "szPWMIdleTimeout == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPWMIdleTimeout length = (%d)", (int)strlen(szPWMIdleTimeout));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szPWMIdleTimeout[0], &szPWMIdleTimeout[0], strlen(szPWMIdleTimeout));

        return (VS_SUCCESS);
}

/*
Function        :inGetDemoMode
Date&Time       :2018/8/22 上午 11:16
Describe        :
*/
int inGetDemoMode(char* szDemoMode)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szDemoMode == NULL || strlen(srEDCRec.szDemoMode) < 0 || strlen(srEDCRec.szDemoMode) > 2)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetDemoMode() ERROR !!");

                        if (szDemoMode == NULL)
                        {
                                inLogPrintf(AT, "szDemoMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDemoMode length = (%d)", (int)strlen(srEDCRec.szDemoMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szDemoMode[0], &srEDCRec.szDemoMode[0], strlen(srEDCRec.szDemoMode));

        return (VS_SUCCESS);
}

/*
Function        :inSetDemoMode
Date&Time       :2018/4/11 下午 3:06
Describe        :
*/
int inSetDemoMode(char* szDemoMode)
{
        memset(srEDCRec.szDemoMode, 0x00, sizeof(srEDCRec.szDemoMode));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szDemoMode == NULL || strlen(szDemoMode) < 0 || strlen(szDemoMode) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetDemoMode() ERROR !!");

                        if (szDemoMode == NULL)
                        {
                                inLogPrintf(AT, "szDemoMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDemoMode length = (%d)", (int)strlen(szDemoMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szDemoMode[0], &szDemoMode[0], strlen(szDemoMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetKBDLock
Date&Time       :2018/12/8 下午 9:40
Describe        :
*/
int inGetKBDLock(char* szKBDLock)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szKBDLock == NULL || strlen(srEDCRec.szKBDLock) < 0 || strlen(srEDCRec.szKBDLock) > 2)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetKBDLock() ERROR !!");

                        if (szKBDLock == NULL)
                        {
                                inLogPrintf(AT, "szKBDLock == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szKBDLock length = (%d)", (int)strlen(srEDCRec.szKBDLock));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szKBDLock[0], &srEDCRec.szKBDLock[0], strlen(srEDCRec.szKBDLock));

        return (VS_SUCCESS);
}

/*
Function        :inSetKBDLock
Date&Time       :2018/12/8 下午 9:40
Describe        :
*/
int inSetKBDLock(char* szKBDLock)
{
        memset(srEDCRec.szKBDLock, 0x00, sizeof(srEDCRec.szKBDLock));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szKBDLock == NULL || strlen(szKBDLock) < 0 || strlen(szKBDLock) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetKBDLock() ERROR !!");

                        if (szKBDLock == NULL)
                        {
                                inLogPrintf(AT, "szKBDLock == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szKBDLock length = (%d)", (int)strlen(szKBDLock));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szKBDLock[0], &szKBDLock[0], strlen(szKBDLock));

        return (VS_SUCCESS);
}

/*
Function        :inGetLastDownloadMode
Date&Time       :2019/5/30 下午 5:30
Describe        :
*/
int inGetLastDownloadMode(char* szLastDownloadMode)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szLastDownloadMode == NULL || strlen(srEDCRec.szLastDownloadMode) < 0 || strlen(srEDCRec.szLastDownloadMode) > 2)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetLastDownloadMode() ERROR !!");

                        if (szLastDownloadMode == NULL)
                        {
                                inLogPrintf(AT, "szLastDownloadMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szLastDownloadMode length = (%d)", (int)strlen(srEDCRec.szLastDownloadMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szLastDownloadMode[0], &srEDCRec.szLastDownloadMode[0], strlen(srEDCRec.szLastDownloadMode));

        return (VS_SUCCESS);
}

/*
Function        :inSetLastDownloadMode
Date&Time       :2019/5/30 下午 5:30
Describe        :
*/
int inSetLastDownloadMode(char* szLastDownloadMode)
{
        memset(srEDCRec.szLastDownloadMode, 0x00, sizeof(srEDCRec.szLastDownloadMode));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szLastDownloadMode == NULL || strlen(szLastDownloadMode) < 0 || strlen(szLastDownloadMode) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetLastDownloadMode() ERROR !!");

                        if (szLastDownloadMode == NULL)
                        {
                                inLogPrintf(AT, "szLastDownloadMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szLastDownloadMode length = (%d)", (int)strlen(szLastDownloadMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szLastDownloadMode[0], &szLastDownloadMode[0], strlen(szLastDownloadMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetPMPassword
Date&Time       :2019/9/16 下午 2:55
Describe        :
*/
int inGetPMPassword(char* szPMPassword)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPMPassword == NULL || strlen(srEDCRec.szPMPassword) < 0 || strlen(srEDCRec.szPMPassword) > 8)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPMPassword() ERROR !!");

                        if (szPMPassword == NULL)
                        {
                                inLogPrintf(AT, "szPMPassword == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPMPassword length = (%d)", (int)strlen(srEDCRec.szPMPassword));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szPMPassword[0], &srEDCRec.szPMPassword[0], strlen(srEDCRec.szPMPassword));

        return (VS_SUCCESS);
}

/*
Function        :inSetPMPassword
Date&Time       :2019/9/16 下午 2:56
Describe        :
*/
int inSetPMPassword(char* szPMPassword)
{
        memset(srEDCRec.szPMPassword, 0x00, sizeof(srEDCRec.szPMPassword));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPMPassword == NULL || strlen(szPMPassword) < 0 || strlen(szPMPassword) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPMPassword() ERROR !!");

                        if (szPMPassword == NULL)
                        {
                                inLogPrintf(AT, "szPMPassword == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPMPassword length = (%d)", (int)strlen(szPMPassword));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szPMPassword[0], &szPMPassword[0], strlen(szPMPassword));

        return (VS_SUCCESS);
}

/*
Function        :inGetFunKeyPassword
Date&Time       :2019/9/16 下午 2:55
Describe        :
*/
int inGetFunKeyPassword(char* szFunKeyPassword)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szFunKeyPassword == NULL || strlen(srEDCRec.szFunKeyPassword) < 0 || strlen(srEDCRec.szFunKeyPassword) > 8)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetFunKeyPassword() ERROR !!");

                        if (szFunKeyPassword == NULL)
                        {
                                inLogPrintf(AT, "szFunKeyPassword == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFunKeyPassword length = (%d)", (int)strlen(srEDCRec.szFunKeyPassword));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szFunKeyPassword[0], &srEDCRec.szFunKeyPassword[0], strlen(srEDCRec.szFunKeyPassword));

        return (VS_SUCCESS);
}

/*
Function        :inSetFunKeyPassword
Date&Time       :2019/9/16 下午 2:56
Describe        :
*/
int inSetFunKeyPassword(char* szFunKeyPassword)
{
        memset(srEDCRec.szFunKeyPassword, 0x00, sizeof(srEDCRec.szFunKeyPassword));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szFunKeyPassword == NULL || strlen(szFunKeyPassword) < 0 || strlen(szFunKeyPassword) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetFunKeyPassword() ERROR !!");

                        if (szFunKeyPassword == NULL)
                        {
                                inLogPrintf(AT, "szFunKeyPassword == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFunKeyPassword length = (%d)", (int)strlen(szFunKeyPassword));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szFunKeyPassword[0], &szFunKeyPassword[0], strlen(szFunKeyPassword));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMSDownloadTimes
Date&Time       :2019/12/19 上午 10:54
Describe        :
*/
int inGetTMSDownloadTimes(char* szTMSDownloadTimes)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTMSDownloadTimes == NULL || strlen(srEDCRec.szTMSDownloadTimes) < 0 || strlen(srEDCRec.szTMSDownloadTimes) > 6)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTMSDownloadTimes() ERROR !!");

                        if (szTMSDownloadTimes == NULL)
                        {
                                inLogPrintf(AT, "szTMSDownloadTimes == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSDownloadTimes length = (%d)", (int)strlen(srEDCRec.szTMSDownloadTimes));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szTMSDownloadTimes[0], &srEDCRec.szTMSDownloadTimes[0], strlen(srEDCRec.szTMSDownloadTimes));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSDownloadTimes
Date&Time       :2019/12/19 上午 10:55
Describe        :
*/
int inSetTMSDownloadTimes(char* szTMSDownloadTimes)
{
        memset(srEDCRec.szTMSDownloadTimes, 0x00, sizeof(srEDCRec.szTMSDownloadTimes));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTMSDownloadTimes == NULL || strlen(szTMSDownloadTimes) < 0 || strlen(szTMSDownloadTimes) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTMSDownloadTimes() ERROR !!");

                        if (szTMSDownloadTimes == NULL)
                        {
                                inLogPrintf(AT, "szTMSDownloadTimes == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSDownloadTimes length = (%d)", (int)strlen(szTMSDownloadTimes));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szTMSDownloadTimes[0], &szTMSDownloadTimes[0], strlen(szTMSDownloadTimes));

        return (VS_SUCCESS);
}

/*
Function        :inGetExamBit
Date&Time       :2020/1/14 下午 6:22
Describe        :
*/
int inGetExamBit(char* szExamBit)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szExamBit == NULL || strlen(srEDCRec.szExamBit) < 0 || strlen(srEDCRec.szExamBit) > 10)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetExamBit() ERROR !!");

                        if (szExamBit == NULL)
                        {
                                inLogPrintf(AT, "szExamBit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szExamBit length = (%d)", (int)strlen(srEDCRec.szExamBit));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szExamBit[0], &srEDCRec.szExamBit[0], strlen(srEDCRec.szExamBit));

        return (VS_SUCCESS);
}

/*
Function        :inSetExamBit
Date&Time       :2020/1/14 下午 6:23
Describe        :
*/
int inSetExamBit(char* szExamBit)
{
        memset(srEDCRec.szExamBit, 0x00, sizeof(srEDCRec.szExamBit));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szExamBit == NULL || strlen(szExamBit) < 0 || strlen(szExamBit) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetExamBit() ERROR !!");

                        if (szExamBit == NULL)
                        {
                                inLogPrintf(AT, "szExamBit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szExamBit length = (%d)", (int)strlen(szExamBit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szExamBit[0], &szExamBit[0], strlen(szExamBit));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMSPacketSize
Date&Time       :2021/2/24 下午 3:01
Describe        :
*/
int inGetTMSPacketSize(char* szTMSPacketSize)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTMSPacketSize == NULL || strlen(srEDCRec.szTMSPacketSize) < 0 || strlen(srEDCRec.szTMSPacketSize) > 4)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTMSPacketSize() ERROR !!");

                        if (szTMSPacketSize == NULL)
                        {
                                inLogPrintf(AT, "szTMSPacketSize == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSPacketSize length = (%d)", (int)strlen(srEDCRec.szTMSPacketSize));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szTMSPacketSize[0], &srEDCRec.szTMSPacketSize[0], strlen(srEDCRec.szTMSPacketSize));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSPacketSize
Date&Time       :2021/2/24 下午 3:01
Describe        :
*/
int inSetTMSPacketSize(char* szTMSPacketSize)
{
        memset(srEDCRec.szTMSPacketSize, 0x00, sizeof(srEDCRec.szTMSPacketSize));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTMSPacketSize == NULL || strlen(szTMSPacketSize) < 0 || strlen(szTMSPacketSize) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTMSPacketSize() ERROR !!");

                        if (szTMSPacketSize == NULL)
                        {
                                inLogPrintf(AT, "szTMSPacketSize == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSPacketSize length = (%d)", (int)strlen(szTMSPacketSize));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szTMSPacketSize[0], &szTMSPacketSize[0], strlen(szTMSPacketSize));

        return (VS_SUCCESS);
}

/*
Function        :inGetSupECR_UDP
Date&Time       :2021/11/30 下午 5:47
Describe        :
*/
int inGetSupECR_UDP(char* szSupECR_UDP)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSupECR_UDP == NULL || strlen(srEDCRec.szSupECR_UDP) < 0 || strlen(srEDCRec.szSupECR_UDP) > 2)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetSupECR_UDP() ERROR !!");

                        if (szSupECR_UDP == NULL)
                        {
                                inLogPrintf(AT, "szSupECR_UDP == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSupECR_UDP length = (%d)", (int)strlen(srEDCRec.szSupECR_UDP));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szSupECR_UDP[0], &srEDCRec.szSupECR_UDP[0], strlen(srEDCRec.szSupECR_UDP));

        return (VS_SUCCESS);
}

/*
Function        :inSetSupECR_UDP
Date&Time       :2021/11/30 下午 5:47
Describe        :
*/
int inSetSupECR_UDP(char* szSupECR_UDP)
{
        memset(srEDCRec.szSupECR_UDP, 0x00, sizeof(srEDCRec.szSupECR_UDP));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSupECR_UDP == NULL || strlen(szSupECR_UDP) < 0 || strlen(szSupECR_UDP) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetSupECR_UDP() ERROR !!");

                        if (szSupECR_UDP == NULL)
                        {
                                inLogPrintf(AT, "szSupECR_UDP == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSupECR_UDP length = (%d)", (int)strlen(szSupECR_UDP));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szSupECR_UDP[0], &szSupECR_UDP[0], strlen(szSupECR_UDP));

        return (VS_SUCCESS);
}

/*
Function        :inGetUDP_Port
Date&Time       :2021/11/30 下午 5:48
Describe        :
*/
int inGetUDP_Port(char* szUDP_Port)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szUDP_Port == NULL || strlen(srEDCRec.szUDP_Port) < 0 || strlen(srEDCRec.szUDP_Port) > 5)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetUDP_Port() ERROR !!");

                        if (szUDP_Port == NULL)
                        {
                                inLogPrintf(AT, "szUDP_Port == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szUDP_Port length = (%d)", (int)strlen(srEDCRec.szUDP_Port));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szUDP_Port[0], &srEDCRec.szUDP_Port[0], strlen(srEDCRec.szUDP_Port));

        return (VS_SUCCESS);
}

/*
Function        :inSetUDP_Port
Date&Time       :2021/11/30 下午 5:48
Describe        :
*/
int inSetUDP_Port(char* szUDP_Port)
{
        memset(srEDCRec.szUDP_Port, 0x00, sizeof(srEDCRec.szUDP_Port));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szUDP_Port == NULL || strlen(szUDP_Port) < 0 || strlen(szUDP_Port) > 5)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetUDP_Port() ERROR !!");

                        if (szUDP_Port == NULL)
                        {
                                inLogPrintf(AT, "szUDP_Port == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szUDP_Port length = (%d)", (int)strlen(szUDP_Port));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szUDP_Port[0], &szUDP_Port[0], strlen(szUDP_Port));

        return (VS_SUCCESS);
}

/*
Function        :inGetPOS_IP
Date&Time       :2021/11/30 下午 5:49
Describe        :
*/
int inGetPOS_IP(char* szPOS_IP)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPOS_IP == NULL || strlen(srEDCRec.szPOS_IP) < 0 || strlen(srEDCRec.szPOS_IP) > 16)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPOS_IP() ERROR !!");

                        if (szPOS_IP == NULL)
                        {
                                inLogPrintf(AT, "szPOS_IP == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPOS_IP length = (%d)", (int)strlen(srEDCRec.szPOS_IP));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szPOS_IP[0], &srEDCRec.szPOS_IP[0], strlen(srEDCRec.szPOS_IP));

        return (VS_SUCCESS);
}

/*
Function        :inSetPOS_IP
Date&Time       :2021/11/30 下午 5:49
Describe        :
*/
int inSetPOS_IP(char* szPOS_IP)
{
        memset(srEDCRec.szPOS_IP, 0x00, sizeof(srEDCRec.szPOS_IP));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPOS_IP == NULL || strlen(szPOS_IP) < 0 || strlen(szPOS_IP) > 16)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPOS_IP() ERROR !!");

                        if (szPOS_IP == NULL)
                        {
                                inLogPrintf(AT, "szPOS_IP == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPOS_IP length = (%d)", (int)strlen(szPOS_IP));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szPOS_IP[0], &szPOS_IP[0], strlen(szPOS_IP));

        return (VS_SUCCESS);
}

/*
Function        :inGetPOSTxUniqueNo
Date&Time       :2021/11/30 下午 5:49
Describe        :
*/
int inGetPOSTxUniqueNo(char* szPOSTxUniqueNo)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPOSTxUniqueNo == NULL || strlen(srEDCRec.szPOSTxUniqueNo) < 0 || strlen(srEDCRec.szPOSTxUniqueNo) > 16)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPOSTxUniqueNo() ERROR !!");

                        if (szPOSTxUniqueNo == NULL)
                        {
                                inLogPrintf(AT, "szPOSTxUniqueNo == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPOSTxUniqueNo length = (%d)", (int)strlen(srEDCRec.szPOSTxUniqueNo));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szPOSTxUniqueNo[0], &srEDCRec.szPOSTxUniqueNo[0], strlen(srEDCRec.szPOSTxUniqueNo));

        return (VS_SUCCESS);
}

/*
Function        :inSetPOSTxUniqueNo
Date&Time       :2021/11/30 下午 5:49
Describe        :
*/
int inSetPOSTxUniqueNo(char* szPOSTxUniqueNo)
{
        memset(srEDCRec.szPOSTxUniqueNo, 0x00, sizeof(srEDCRec.szPOSTxUniqueNo));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPOSTxUniqueNo == NULL || strlen(szPOSTxUniqueNo) < 0 || strlen(szPOSTxUniqueNo) > 16)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPOSTxUniqueNo() ERROR !!");

                        if (szPOSTxUniqueNo == NULL)
                        {
                                inLogPrintf(AT, "szPOSTxUniqueNo == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPOSTxUniqueNo length = (%d)", (int)strlen(szPOSTxUniqueNo));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szPOSTxUniqueNo[0], &szPOSTxUniqueNo[0], strlen(szPOSTxUniqueNo));

        return (VS_SUCCESS);
}

/*
Function        :inGetUDP_ECRVersion
Date&Time       :2022/2/18 下午 6:09
Describe        :
*/
int inGetUDP_ECRVersion(char* szUDP_ECRVersion)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szUDP_ECRVersion == NULL || strlen(srEDCRec.szUDP_ECRVersion) < 0 || strlen(srEDCRec.szUDP_ECRVersion) > 2)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetUDP_ECRVersion() ERROR !!");

                        if (szUDP_ECRVersion == NULL)
                        {
                                inLogPrintf(AT, "szUDP_ECRVersion == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szUDP_ECRVersion length = (%d)", (int)strlen(srEDCRec.szUDP_ECRVersion));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szUDP_ECRVersion[0], &srEDCRec.szUDP_ECRVersion[0], strlen(srEDCRec.szUDP_ECRVersion));

        return (VS_SUCCESS);
}

/*
Function        :inSetUDP_ECRVersion
Date&Time       :2022/2/18 下午 6:09
Describe        :
*/
int inSetUDP_ECRVersion(char* szUDP_ECRVersion)
{
        memset(srEDCRec.szUDP_ECRVersion, 0x00, sizeof(srEDCRec.szUDP_ECRVersion));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szUDP_ECRVersion == NULL || strlen(szUDP_ECRVersion) < 0 || strlen(szUDP_ECRVersion) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetUDP_ECRVersion() ERROR !!");

                        if (szUDP_ECRVersion == NULL)
                        {
                                inLogPrintf(AT, "szUDP_ECRVersion == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szUDP_ECRVersion length = (%d)", (int)strlen(szUDP_ECRVersion));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szUDP_ECRVersion[0], &szUDP_ECRVersion[0], strlen(szUDP_ECRVersion));

        return (VS_SUCCESS);
}

/*
Function        :inGetPOS_ID
Date&Time       :2022/10/17 下午 5:05
Describe        :
*/
int inGetPOS_ID(char* szPOS_ID)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPOS_ID == NULL || strlen(srEDCRec.szPOS_ID) < 0 || strlen(srEDCRec.szPOS_ID) > 6)
        {
               /* debug */
               if (ginDebug == VS_TRUE)
               {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPOS_ID() ERROR !!");

                        if (szPOS_ID == NULL)
                        {
                                inLogPrintf(AT, "szPOS_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPOS_ID length = (%d)", (int)strlen(srEDCRec.szPOS_ID));
                                inLogPrintf(AT, szErrorMsg);
                        }
               }
               return (VS_ERROR);
        }
        memcpy(&szPOS_ID[0], &srEDCRec.szPOS_ID[0], strlen(srEDCRec.szPOS_ID));

        return (VS_SUCCESS);
}

/*
Function        :inSetPOS_ID
Date&Time       :2022/10/17 下午 5:11
Describe        :
*/
int inSetPOS_ID(char* szPOS_ID)
{
        memset(srEDCRec.szPOS_ID, 0x00, sizeof(srEDCRec.szPOS_ID));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPOS_ID == NULL || strlen(szPOS_ID) < 0 || strlen(szPOS_ID) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPOS_ID() ERROR !!");

                        if (szPOS_ID == NULL)
                        {
                                inLogPrintf(AT, "szPOS_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPOS_ID length = (%d)", (int)strlen(szPOS_ID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szPOS_ID[0], &szPOS_ID[0], strlen(szPOS_ID));

        return (VS_SUCCESS);
}

/*
Function        :inGetGPRS_APN
Date&Time       :2023/4/21 下午 1:30
Describe        :
*/
int inGetGPRS_APN(char* szGPRS_APN)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szGPRS_APN == NULL || strlen(srEDCRec.szGPRS_APN) < 0 || strlen(srEDCRec.szGPRS_APN) > 100)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        if (szGPRS_APN == NULL)
                        {
                                inLogPrintf(AT, "inGetGPRS_APN() ERROR !! szGPRS_APN == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "inGetGPRS_APN() ERROR !! szGPRS_APN length = (%d)", (int)strlen(srEDCRec.szGPRS_APN));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szGPRS_APN[0], &srEDCRec.szGPRS_APN[0], strlen(srEDCRec.szGPRS_APN));

        return (VS_SUCCESS);
}

/*
Function        :inSetGPRS_APN
Date&Time       :2023/4/21 下午 1:30
Describe        :
*/
int inSetGPRS_APN(char* szGPRS_APN)
{
        memset(srEDCRec.szGPRS_APN, 0x00, sizeof(srEDCRec.szGPRS_APN));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szGPRS_APN == NULL || strlen(szGPRS_APN) < 0 || strlen(szGPRS_APN) > 100)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetGPRS_APN() ERROR !!");

                        if (szGPRS_APN == NULL)
                        {
                                inLogPrintf(AT, "szGPRS_APN == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szGPRS_APN length = (%d)", (int)strlen(szGPRS_APN));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szGPRS_APN[0], &szGPRS_APN[0], strlen(szGPRS_APN));

        return (VS_SUCCESS);
}

/*
Function        :inGetScreenBrightness
Date&Time       :2023/8/30 上午 11:58
Describe        :
*/
int inGetScreenBrightness(char* szScreenBrightness)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szScreenBrightness == NULL || strlen(srEDCRec.szScreenBrightness) < 0 || strlen(srEDCRec.szScreenBrightness) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        if (szScreenBrightness == NULL)
                        {
                                inLogPrintf(AT, "inGetScreenBrightness() ERROR !! szScreenBrightness == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "inGetScreenBrightness() ERROR !! szScreenBrightness length = (%d)", (int)strlen(srEDCRec.szScreenBrightness));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szScreenBrightness[0], &srEDCRec.szScreenBrightness[0], strlen(srEDCRec.szScreenBrightness));

        return (VS_SUCCESS);
}

/*
Function        :inSetScreenBrightness
Date&Time       :2023/8/30 上午 11:59
Describe        :
*/
int inSetScreenBrightness(char* szScreenBrightness)
{
        memset(srEDCRec.szScreenBrightness, 0x00, sizeof(srEDCRec.szScreenBrightness));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szScreenBrightness == NULL || strlen(szScreenBrightness) < 0 || strlen(szScreenBrightness) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetScreenBrightness() ERROR !!");

                        if (szScreenBrightness == NULL)
                        {
                                inLogPrintf(AT, "szScreenBrightness == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szScreenBrightness length = (%d)", (int)strlen(szScreenBrightness));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szScreenBrightness[0], &szScreenBrightness[0], strlen(szScreenBrightness));

        return (VS_SUCCESS);
}

/*
Function        :inGetSignpadBeepInterval
Date&Time       :2023/9/1 下午 2:58
Describe        :
*/
int inGetSignpadBeepInterval(char* szSignpadBeepInterval)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSignpadBeepInterval == NULL || strlen(srEDCRec.szSignpadBeepInterval) < 0 || strlen(srEDCRec.szSignpadBeepInterval) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        if (szSignpadBeepInterval == NULL)
                        {
                                inLogPrintf(AT, "inGetSignpadBeepInterval() ERROR !! szSignpadBeepInterval == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "inGetSignpadBeepInterval() ERROR !! szSignpadBeepInterval length = (%d)", (int)strlen(srEDCRec.szSignpadBeepInterval));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSignpadBeepInterval[0], &srEDCRec.szSignpadBeepInterval[0], strlen(srEDCRec.szSignpadBeepInterval));

        return (VS_SUCCESS);
}

/*
Function        :inSetSignpadBeepInterval
Date&Time       :2023/9/1 下午 2:58
Describe        :
*/
int inSetSignpadBeepInterval(char* szSignpadBeepInterval)
{
        memset(srEDCRec.szSignpadBeepInterval, 0x00, sizeof(srEDCRec.szSignpadBeepInterval));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSignpadBeepInterval == NULL || strlen(szSignpadBeepInterval) < 0 || strlen(szSignpadBeepInterval) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetSignpadBeepInterval() ERROR !!");

                        if (szSignpadBeepInterval == NULL)
                        {
                                inLogPrintf(AT, "szSignpadBeepInterval == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSignpadBeepInterval length = (%d)", (int)strlen(szSignpadBeepInterval));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srEDCRec.szSignpadBeepInterval[0], &szSignpadBeepInterval[0], strlen(szSignpadBeepInterval));

        return (VS_SUCCESS);
}

/*
Function        :inEDC_Edit_EDC_Table
Date&Time       :2017/5/9 下午 4:21
Describe        :
*/
int inEDC_Edit_EDC_Table(void)
{
	TABLE_GET_SET_TABLE EDC_FUNC_TABLE[] =
	{
		{"szCUPFuncEnable"		,inGetCUPFuncEnable			,inSetCUPFuncEnable			},
		{"szFiscFuncEnable"		,inGetFiscFuncEnable			,inSetFiscFuncEnable			},
		{"szECRComPort"			,inGetECRComPort			,inSetECRComPort			},
		{"szECRVersion"			,inGetECRVersion			,inSetECRVersion			},
		{"szPABXCode"			,inGetPABXCode				,inSetPABXCode				},
		{"szTSAMRegisterEnable"		,inGetTSAMRegisterEnable		,inSetTSAMRegisterEnable		},
		{"szTMSOK"			,inGetTMSOK				,inSetTMSOK				},
		{"szDCCInit"			,inGetDCCInit				,inSetDCCInit				},
		{"szDCCDownloadMode"		,inGetDCCDownloadMode			,inSetDCCDownloadMode			},
		{"szDCCLastUpdateDate"		,inGetDCCLastUpdateDate			,inSetDCCLastUpdateDate			},
		{"szDCCSettleDownload"		,inGetDCCSettleDownload			,inSetDCCSettleDownload			},
		{"szDCCBinVer"			,inGetDCCBinVer				,inSetDCCBinVer				},
		{"szEDCLOCK"			,inGetEDCLOCK				,inSetEDCLOCK				},
		{"szISODebug"			,inGetISODebug				,inSetISODebug				},
		{"szManagerPassword"		,inGetManagerPassword			,inSetManagerPassword			},
		{"szFunctionPassword"		,inGetFunctionPassword			,inSetFunctionPassword			},
		{"szMerchantPassword"		,inGetMerchantPassword			,inSetMerchantPassword			},
		{"szSuperPassword"		,inGetSuperPassword			,inSetSuperPassword			},
		{"szMCCCode"			,inGetMCCCode				,inSetMCCCode				},
		{"szIssuerID"			,inGetIssuerID				,inSetIssuerID				},
		{"szEnterTimeout"		,inGetEnterTimeout			,inSetEnterTimeout			},
		{"szIPSendTimeout"		,inGetIPSendTimeout			,inSetIPSendTimeout			},
		{"szTermVersionID"		,inGetTermVersionID			,inSetTermVersionID			},
		{"szTermVersionDate"		,inGetTermVersionDate			,inSetTermVersionDate			},
		{"szTermIPAddress"		,inGetTermIPAddress			,inSetTermIPAddress			},
		{"szTermGetewayAddress"		,inGetTermGetewayAddress		,inSetTermGetewayAddress		},
		{"szTermMASKAddress"		,inGetTermMASKAddress			,inSetTermMASKAddress			},
		{"szTermECRPort"		,inGetTermECRPort			,inSetTermECRPort			},
		{"szESCMode"			,inGetESCMode				,inSetESCMode				},
		{"szMultiComPort1"		,inGetMultiComPort1			,inSetMultiComPort1			},
		{"szMultiComPort1Version"	,inGetMultiComPort1Version		,inSetMultiComPort1Version		},
		{"szMultiComPort2"		,inGetMultiComPort2			,inSetMultiComPort2			},
		{"szMultiComPort2Version"	,inGetMultiComPort2Version		,inSetMultiComPort2Version		},
		{"szMultiComPort3"		,inGetMultiComPort3			,inSetMultiComPort3			},
		{"szMultiComPort3Version"	,inGetMultiComPort3Version		,inSetMultiComPort3Version		},
		{"szEMVForceOnline"		,inGetEMVForceOnline			,inSetEMVForceOnline			},
		{"szAutoConnect"		,inGetAutoConnect			,inSetAutoConnect			},
		{"szLOGONum"			,inGetLOGONum				,inSetLOGONum				},
		{"szHostSAMSlot"		,inGetHostSAMSlot			,inSetHostSAMSlot			},
		{"szSAMSlotSN1"			,inGetSAMSlotSN1			,inSetSAMSlotSN1			},
		{"szSAMSlotSN2"			,inGetSAMSlotSN2			,inSetSAMSlotSN2			},
		{"szSAMSlotSN3"			,inGetSAMSlotSN3			,inSetSAMSlotSN3			},
		{"szSAMSlotSN4"			,inGetSAMSlotSN4			,inSetSAMSlotSN4			},
		{"szPWMEnable"			,inGetPWMEnable				,inSetPWMEnable				},
		{"szPWMMode"			,inGetPWMMode				,inSetPWMMode				},
		{"szPWMIdleTimeout"		,inGetPWMIdleTimeout			,inSetPWMIdleTimeout			},
		{"szDemoMode"			,inGetDemoMode				,inSetDemoMode				},
		{"szKBDLock"			,inGetKBDLock				,inSetKBDLock				},
		{"szLastDownloadMode"		,inGetLastDownloadMode			,inSetLastDownloadMode			},
		{"szPMPassword"			,inGetPMPassword			,inSetPMPassword			},
		{"szFunKeyPassword"		,inGetFunKeyPassword			,inSetFunKeyPassword			},
		{"szTMSDownloadTimes"		,inGetTMSDownloadTimes			,inSetTMSDownloadTimes			},
		{"szExamBit"			,inGetExamBit				,inSetExamBit				},
		{"szTMSPacketSize"		,inGetTMSPacketSize			,inSetTMSPacketSize			},
		{"szSupECR_UDP"			,inGetSupECR_UDP			,inSetSupECR_UDP			},
		{"szUDP_Port"			,inGetUDP_Port				,inSetUDP_Port				},
		{"szPOS_IP"			,inGetPOS_IP				,inSetPOS_IP				},
		{"szPOSTxUniqueNo"		,inGetPOSTxUniqueNo			,inSetPOSTxUniqueNo			},
		{"szUDP_ECRVersion"		,inGetUDP_ECRVersion			,inSetUDP_ECRVersion			},
		{"szPOS_ID"			,inGetPOS_ID				,inSetPOS_ID				},
		{"szGPRS_APN"			,inGetGPRS_APN				,inSetGPRS_APN				},
		{"szScreenBrightness"		,inGetScreenBrightness			,inSetScreenBrightness			},
		{"szSignpadBeepInterval"	,inGetSignpadBeepInterval		,inSetSignpadBeepInterval		},
		{""},
	};
	int	inRetVal = 0x00;
	char	szKey = 0x00;
	int	inFinalTimeout = 0;
	
	inFinalTimeout = 30;
	if (inFinalTimeout > 0)
	{
		inFinalTimeout = inFinalTimeout;
	}
	else
	{
		inFinalTimeout = _EDC_TIMEOUT_;
	}
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont_Color("是否更改EDC", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
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
			break;
		}
		
	}
	
	inFunc_Edit_Table_Tag(EDC_FUNC_TABLE);
	inSaveEDCRec(0);
	
	return	(VS_SUCCESS);
}

/*
Function        :inEDC_TMSDownloadTimes_Increase
Date&Time       :2019/12/19 上午 11:02
Describe        :
*/
int inEDC_TMSDownloadTimes_Increase(void)
{
	int	inRetVal = VS_ERROR;
	int	inTMSDownloadTimes = 0;
	char	szTMSDownloadTimes[6 + 1] = {0};
	
	memset(szTMSDownloadTimes, 0x00, sizeof(szTMSDownloadTimes));
	
	inLoadEDCRec(0);
	inRetVal = inGetTMSDownloadTimes(szTMSDownloadTimes);
	
	if (inRetVal != VS_SUCCESS)
	{
		/* 已裝機但還沒有這個欄位，透過APPL更新的狀況 */
		/* 至少下過一次 */
		inTMSDownloadTimes = 1;
		inTMSDownloadTimes++;
	}
	else
	{
		inTMSDownloadTimes = atoi(szTMSDownloadTimes);
		inTMSDownloadTimes++;
		
		/* 如果真的TMS下超過999999次，要從2開始，不然會被當成新裝機 */
		if (inTMSDownloadTimes > 999999)
		{
			inTMSDownloadTimes = 2;
		}
	}
	
	memset(szTMSDownloadTimes, 0x00, sizeof(szTMSDownloadTimes));
	sprintf(szTMSDownloadTimes, "%06d", inTMSDownloadTimes);
	inSetTMSDownloadTimes(szTMSDownloadTimes);
	
	inSaveEDCRec(0);
	
	return (VS_SUCCESS);
}
