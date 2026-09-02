#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "File.h"
#include "Function.h"
#include "MVT.h"

static  MVT_REC srMVTRec;	/* construct MVT record */
extern  int	ginDebug;	/* Debug使用 extern */

/*
Function        :inLoadMVTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀MVT檔案，inMVTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadMVTRec(int inMVTRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆MVT檔案 */
        char            szMVTRec[_SIZE_MVT_REC_ + 1];           /* 暫存, 放各個欄位檔案 */
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnMVTLength = 0;                        /* MVT總長度 */
        long            lnReadLength;                           /* 記錄每次要從MVT.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;                 /* inRec記錄讀到第幾筆, i為目前從MVT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */

        /* inLoadMVTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadMVTRec(%d) START!!", inMVTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 判斷傳進來的inMVTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inMVTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inMVTRec < 0:(index = %d) ERROR!!", inMVTRec);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open MVT.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_MVT_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnMVTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_MVT_FILE_NAME_);

	if (lnMVTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnMVTLength + 1);
        uszTemp = malloc(lnMVTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnMVTLength + 1);
        memset(uszTemp, 0x00, lnMVTLength + 1);

         /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnMVTLength;

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
         *i為目前從MVT讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnMVTLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到MVT的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                      	/* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
			inSearchResult = 1;

			/* 清空uszReadData */
                        memset(uszReadData, 0x00, lnMVTLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
			memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
			/* 因為inMVT_Rec的index從0開始，所以inMVT_Rec要+1 */
                        if (inRec == (inMVTRec + 1))
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
         * 如果沒有inMVTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inMVTRec + 1) || inSearchResult == -1)
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
        memset(&srMVTRec, 0x00, sizeof(srMVTRec));
	/*
         * 以下pattern為存入MVT_Rec
         * i為MVT的第幾個字元
         * 存入MVT_Rec
         */
        i = 0;


        /* 01_EMV應用程式索引 */
	/* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
	k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "MVT unpack ERROR_1");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szApplicationIndex[0], &szMVTRec[0], k - 1);
        }

        /* 02_應用程式 ID */
	/* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "MVT unpack ERROR_2");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szMVTApplicationId[0], &szMVTRec[0], k - 1);
        }

        /* 03_Terminal_Type */
	/* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "MVT unpack ERROR_3");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szMVTTerminalType[0], &szMVTRec[0], k - 1);
        }

        /* 04_Terminal_Capabilities */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "MVT unpack ERROR_4");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szTerminalCapabilities[0], &szMVTRec[0], k - 1);
        }

        /* 05_Additional_Terminal_Capabilities */
	/* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "MVT unpack ERROR_5");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szAdditionalTerminalCapabilities[0], &szMVTRec[0], k - 1);
        }

        /* 06_Terminal_Country_Code */
	/* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "MVT unpack ERROR_6");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szTerminalCountryCode[0], &szMVTRec[0], k - 1);
        }

        /* 07_Transaction_Currency_Code */
	/* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "MVT unpack ERROR_7");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szMVTTransactionCurrencyCode[0], &szMVTRec[0], k - 1);
        }

        /* 08_Default_TAC */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "MVT unpack ERROR_8");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szDefaultTAC[0], &szMVTRec[0], k - 1);
        }

        /* 09_Online_TAC */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "MVT unpack ERROR_9");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szOnlineTAC[0], &szMVTRec[0], k - 1);
        }

        /* 10_Denial_TAC */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "MVT unpack ERROR_10");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szDenialTAC[0], &szMVTRec[0], k - 1);
        }

        /* 11_Default_TDOL */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "MVT unpack ERROR_11");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szDefaultTDOL[0], &szMVTRec[0], k - 1);
        }

        /* 12_Default_DDOL */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "MVT unpack ERROR_12");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szDefaultDDOL[0], &szMVTRec[0], k - 1);
        }

        /* 13_EMV_Floor_Limit */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "MVT unpack ERROR_13");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szEMVFloorLimit[0], &szMVTRec[0], k - 1);
        }

        /* 14_Random_Selection_Threshol */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "MVT unpack ERROR_14");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szRandomSelectionThreshold[0], &szMVTRec[0], k - 1);
        }

        /* 15_Target_Percent_for_Random_Selection */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "MVT unpack ERROR_15");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szTargetPercentforRandomSelection[0], &szMVTRec[0], k - 1);
        }

        /* 16_Max_Target_Percent_for_Random_Selection */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "MVT unpack ERROR_16");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szMaxTargetPercentforRandomSelection[0], &szMVTRec[0], k - 1);
        }

        /* 17_Merchant Category Code(MCC) */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "MVT unpack ERROR_17");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szMVTMerchantCategoryCode[0], &szMVTRec[0], k - 1);
        }

        /* 18_Transaction Category Code(TCC) */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "MVT unpack ERROR_18");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szMVTTransactionCategoryCode[0], &szMVTRec[0], k - 1);
        }

        /* 19_Transaction_Reference_Currency_Code */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "MVT unpack ERROR_19");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szTransactionReferenceCurrencyCode[0], &szMVTRec[0], k - 1);
        }

        /* 20_Transaction_Reference_Currency_Coversion */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "MVT unpack ERROR_20");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szTransactionReferenceCurrencyCoversion[0], &szMVTRec[0], k - 1);
        }

        /* 21_Transaction_Reference_Currency_Exponent */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "MVT unpack ERROR_21");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)
        {
                memcpy(&srMVTRec.szTransactionReferenceCurrencyExponent[0], &szMVTRec[0], k - 1);
        }

        /* 22_等候EMV Offline PIN輸入的Time out時間  */
        /* 初始化 */
        memset(szMVTRec, 0x00, sizeof(szMVTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szMVTRec[k ++] = uszReadData[i ++];
                if (szMVTRec[k - 1] == 0x2C	||
		    szMVTRec[k - 1] == 0x0D	||
		    szMVTRec[k - 1] == 0x0A	||
		    szMVTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnMVTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "MVT unpack ERROR_22");
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
        if (szMVTRec[0] != 0x2C	&&
	    szMVTRec[0] != 0x0D	&&
	    szMVTRec[0] != 0x0A	&&
	    szMVTRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srMVTRec.szEMVPINEntryTimeout[0], &szMVTRec[0], k - 1);
        }

        /* release */
	/* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadMVTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadMVTRec(%d) END!!", inMVTRec);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveMVTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSaveMVTRec(int inMVTRec)
{
        unsigned long   uldat_Handle;   		        /* FILE Handle */
        unsigned long   ulbak_Handle;   		        /* FILE Handle */
        int             inPackCount = 0;        		/* uszWriteBuff_Record的index */
        int             inRecIndex = 0;         		/* uszRead_Total_Buff的index */
        int             inMVT_Total_Rec = 0;    		/* MVT.dat的總筆數 */
        int             inRetVal;
        int             i = 0, j = 0;
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];	/* debug message */
	long	        lnReadLength = 0;       		/* 每次要從MVT.dat讀多少byte出來 */
        long            lnMVTLength = 0;         		/* MVT.dat檔案總長度 */
        unsigned char   *uszRead_Total_Buff;
        unsigned char   *uszWriteBuff_Record, *uszWriteBuff_Org;


        /* inSaveMVTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveMVTRec(%d) START!!", inMVTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 防呆先刪除MVT.bak  */
        inFILE_Delete((unsigned char *)_MVT_FILE_NAME_BAK_);

        /* 新建MVT.bak檔案 */
        inRetVal = inFILE_Create(&ulbak_Handle, (unsigned char *)_MVT_FILE_NAME_BAK_);

        if (inRetVal != VS_SUCCESS)
        {
                inFILE_Close(&ulbak_Handle);

                return (VS_ERROR);
        }

        /* 開啟原檔案MVT.dat */
        inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_MVT_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                /* MVT.dat開檔失敗 ，不用關檔MVT.dat */
                /* MVT.bak仍要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_MVT_FILE_NAME_BAK_);

                return(VS_ERROR);
        }

        /* 取得MVT.dat檔案大小 */
        lnMVTLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *)_MVT_FILE_NAME_);

	/* 取得檔案大小失敗 */
        if (lnMVTLength == VS_ERROR)
        {
                /* MVT.bak和MVT.dat要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Close(&uldat_Handle);
                inFILE_Delete((unsigned char *)_MVT_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        uszWriteBuff_Record = malloc(_SIZE_MVT_REC_ + _SIZE_MVT_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_MVT_REC_ + _SIZE_MVT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原MVT.dat的每筆Record 初始化 */
        uszWriteBuff_Org = malloc(_SIZE_MVT_REC_ + _SIZE_MVT_COMMA_0D0A_);
        memset(uszWriteBuff_Org, 0x00, _SIZE_MVT_REC_ + _SIZE_MVT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存MVT.dat全部資料 */
        uszRead_Total_Buff = malloc(lnMVTLength + 1);
        memset(uszRead_Total_Buff, 0x00, lnMVTLength + 1);

        inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

        /* ApplicationIndex */
        memcpy(&uszWriteBuff_Record[0], &srMVTRec.szApplicationIndex[0], strlen(srMVTRec.szApplicationIndex));
        inPackCount += strlen(srMVTRec.szApplicationIndex);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* ApplicationId */
        memcpy(&uszWriteBuff_Record[inPackCount], &srMVTRec.szMVTApplicationId[0], strlen(srMVTRec.szMVTApplicationId));
        inPackCount += strlen(srMVTRec.szMVTApplicationId);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TerminalType */
        memcpy(&uszWriteBuff_Record[inPackCount], &srMVTRec.szMVTTerminalType[0], strlen(srMVTRec.szMVTTerminalType));
        inPackCount += strlen(srMVTRec.szMVTTerminalType);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TerminalCapabilities */
        memcpy(&uszWriteBuff_Record[inPackCount], &srMVTRec.szTerminalCapabilities[0], strlen(srMVTRec.szTerminalCapabilities));
        inPackCount += strlen(srMVTRec.szTerminalCapabilities);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* AdditionalTerminalCapabilities */
        memcpy(&uszWriteBuff_Record[inPackCount], &srMVTRec.szAdditionalTerminalCapabilities[0], strlen(srMVTRec.szAdditionalTerminalCapabilities));
        inPackCount += strlen(srMVTRec.szAdditionalTerminalCapabilities);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TerminalCountryCode */
        memcpy(&uszWriteBuff_Record[inPackCount], &srMVTRec.szTerminalCountryCode[0], strlen(srMVTRec.szTerminalCountryCode));
        inPackCount += strlen(srMVTRec.szTerminalCountryCode);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TransactionCurrencyCode */
        memcpy(&uszWriteBuff_Record[inPackCount], &srMVTRec.szMVTTransactionCurrencyCode[0], strlen(srMVTRec.szMVTTransactionCurrencyCode));
        inPackCount += strlen(srMVTRec.szMVTTransactionCurrencyCode);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* DefaultTAC */
        memcpy(&uszWriteBuff_Record[inPackCount], &srMVTRec.szDefaultTAC[0], strlen(srMVTRec.szDefaultTAC));
        inPackCount += strlen(srMVTRec.szDefaultTAC);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* OnlineTAC */
        memcpy(&uszWriteBuff_Record[inPackCount], &srMVTRec.szOnlineTAC[0], strlen(srMVTRec.szOnlineTAC));
        inPackCount += strlen(srMVTRec.szOnlineTAC);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* DenialTAC */
        memcpy(&uszWriteBuff_Record[inPackCount], &srMVTRec.szDenialTAC[0], strlen(srMVTRec.szDenialTAC));
        inPackCount += strlen(srMVTRec.szDenialTAC);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* DefaultTDOL */
        memcpy(&uszWriteBuff_Record[inPackCount], &srMVTRec.szDefaultTDOL[0], strlen(srMVTRec.szDefaultTDOL));
        inPackCount += strlen(srMVTRec.szDefaultTDOL);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* DefaultDDOL */
        memcpy(&uszWriteBuff_Record[inPackCount], &srMVTRec.szDefaultDDOL[0], strlen(srMVTRec.szDefaultDDOL));
        inPackCount += strlen(srMVTRec.szDefaultDDOL);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* EMVFloorLimit */
        memcpy(&uszWriteBuff_Record[inPackCount], &srMVTRec.szEMVFloorLimit[0], strlen(srMVTRec.szEMVFloorLimit));
        inPackCount += strlen(srMVTRec.szEMVFloorLimit);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* RandomSelectionThreshold */
        memcpy(&uszWriteBuff_Record[inPackCount], &srMVTRec.szRandomSelectionThreshold[0], strlen(srMVTRec.szRandomSelectionThreshold));
        inPackCount += strlen(srMVTRec.szRandomSelectionThreshold);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TargetPercentforRandomSelection */
        memcpy(&uszWriteBuff_Record[inPackCount], &srMVTRec.szTargetPercentforRandomSelection[0], strlen(srMVTRec.szTargetPercentforRandomSelection));
        inPackCount += strlen(srMVTRec.szTargetPercentforRandomSelection);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* MaxTargetPercentforRandomSelection */
        memcpy(&uszWriteBuff_Record[inPackCount], &srMVTRec.szMaxTargetPercentforRandomSelection[0], strlen(srMVTRec.szMaxTargetPercentforRandomSelection));
        inPackCount += strlen(srMVTRec.szMaxTargetPercentforRandomSelection);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* MerchantCategoryCode */
        memcpy(&uszWriteBuff_Record[inPackCount], &srMVTRec.szMVTMerchantCategoryCode[0], strlen(srMVTRec.szMVTMerchantCategoryCode));
        inPackCount += strlen(srMVTRec.szMVTMerchantCategoryCode);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TransactionCategoryCode */
        memcpy(&uszWriteBuff_Record[inPackCount], &srMVTRec.szMVTTransactionCategoryCode[0], strlen(srMVTRec.szMVTTransactionCategoryCode));
        inPackCount += strlen(srMVTRec.szMVTTransactionCategoryCode);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TransactionReferenceCurrencyCode */
        memcpy(&uszWriteBuff_Record[inPackCount], &srMVTRec.szTransactionReferenceCurrencyCode[0], strlen(srMVTRec.szTransactionReferenceCurrencyCode));
        inPackCount += strlen(srMVTRec.szTransactionReferenceCurrencyCode);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TransactionReferenceCurrencyCoversion */
        memcpy(&uszWriteBuff_Record[inPackCount], &srMVTRec.szTransactionReferenceCurrencyCoversion[0], strlen(srMVTRec.szTransactionReferenceCurrencyCoversion));
        inPackCount += strlen(srMVTRec.szTransactionReferenceCurrencyCoversion);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TransactionReferenceCurrencyExponent */
        memcpy(&uszWriteBuff_Record[inPackCount], &srMVTRec.szTransactionReferenceCurrencyExponent[0], strlen(srMVTRec.szTransactionReferenceCurrencyExponent));
        inPackCount += strlen(srMVTRec.szTransactionReferenceCurrencyExponent);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* EMVPINEntryTimeout */
        memcpy(&uszWriteBuff_Record[inPackCount], &srMVTRec.szEMVPINEntryTimeout[0], strlen(srMVTRec.szEMVPINEntryTimeout));
        inPackCount += strlen(srMVTRec.szEMVPINEntryTimeout);

        /* 最後的data不用逗號 */
        /* 補上換行符號 */

        /* 0D是移至行首 */
        uszWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        uszWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;

        /* 讀MVT.dat檔案存到uszRead_Total_Buff */
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnMVTLength;

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
                                        inFILE_Delete((unsigned char *)_MVT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_MVT_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_MVT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        /* 防呆 總record數量小於要存取inMVTRec Return ERROR */
	/* 算總Record數 */
        for (i = 0; i < (lnMVTLength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (uszRead_Total_Buff[i] == 0x0A)
                        inMVT_Total_Rec++;
        }

	/* 防呆 總record數量小於要存取inMVTRec Return ERROR */
        if ((inMVTRec + 1) > inMVT_Total_Rec)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "No data or Index ERROR");
                }

                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_MVT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                return (VS_ERROR);
        }

        /* 記錄ReadTotalBuffer的陣列位置 */
        inRecIndex = 0;
        /* 用inMVTRec決定要先存幾筆Record到MVT.bak，ex:inMVTRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inMVTRec; i ++)
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
                                        inFILE_Delete((unsigned char *)_MVT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_MVT_FILE_NAME_BAK_);

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

        /* 存組好的該MVTRecord 到 MVT.bak */
        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_MVT_FILE_NAME_BAK_);


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
                inFILE_Delete((unsigned char *)_MVT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原MVT.dat Record 到 MVT.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
	/* inRecIndex要指到剩下還沒存進bak檔的第一個記憶體位置 */
        inMVTRec = inMVTRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < lnMVTLength; i++)
        {
                if (uszRead_Total_Buff[i] == 0x0A)
                {
                        j++;

                        if (j == inMVTRec)
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
                /* 接續存原MVT.dat的Record */
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
                                inFILE_Delete((unsigned char *)_MVT_FILE_NAME_BAK_);

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
                                inFILE_Delete((unsigned char *)_MVT_FILE_NAME_BAK_);

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

        /* 刪除原MVT.dat */
        if (inFILE_Delete((unsigned char *)_MVT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* 將MVT.bak改名字為MVT.dat取代原檔案 */
        if (inFILE_Rename((unsigned char *)_MVT_FILE_NAME_BAK_, (unsigned char *)_MVT_FILE_NAME_) != VS_SUCCESS)
	{
                return (VS_ERROR);
        }

        /* inSaveCFTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveMVTRec(%d) END!!", (inMVTRec - 1));
                inLogPrintf(AT, szErrorMsg);
		inLogPrintf(AT, "----------------------------------------");
        }

        return(VS_SUCCESS);
}

/*
set和get等價於相反的操作
各欄位的set和get function
*/

/*
Function        :inGetApplicationIndex
Date&Time       :
Describe        :
*/
int inGetApplicationIndex(char* szApplicationIndex)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szApplicationIndex == NULL || strlen(srMVTRec.szApplicationIndex) <= 0 || strlen(srMVTRec.szApplicationIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetApplicationIndex() ERROR !!");

                        if (szApplicationIndex == NULL)
                        {
                                inLogPrintf(AT, "szApplicationIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szApplicationIndex length = (%d)", (int)strlen(srMVTRec.szApplicationIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }
                return (VS_ERROR);
        }
        memcpy(&szApplicationIndex[0], &srMVTRec.szApplicationIndex[0], strlen(srMVTRec.szApplicationIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetApplicationIndex
Date&Time       :
Describe        :
*/
int inSetApplicationIndex(char* szApplicationIndex)
{
        memset(srMVTRec.szApplicationIndex, 0x00, sizeof(srMVTRec.szApplicationIndex));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szApplicationIndex == NULL || strlen(szApplicationIndex) <= 0 || strlen(szApplicationIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetApplicationIndex() ERROR !!");

                        if (szApplicationIndex == NULL)
                        {
                                inLogPrintf(AT, "szApplicationIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szApplicationIndex length = (%d)", (int)strlen(szApplicationIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szApplicationIndex[0], &szApplicationIndex[0], strlen(szApplicationIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetMVTApplicationId
Date&Time       :
Describe        :
*/
int inGetMVTApplicationId(char* szMVTApplicationId)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szMVTApplicationId == NULL || strlen(srMVTRec.szMVTApplicationId) <= 0 || strlen(srMVTRec.szMVTApplicationId) > 16)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetMVTApplicationId() ERROR !!");

                        if (szMVTApplicationId == NULL)
                        {
                                inLogPrintf(AT, "szApplicationId == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szApplicationId length = (%d)", (int)strlen(srMVTRec.szMVTApplicationId));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMVTApplicationId[0], &srMVTRec.szMVTApplicationId[0], strlen(srMVTRec.szMVTApplicationId));

        return (VS_SUCCESS);

}

/*
Function        :inSetMVTApplicationId
Date&Time       :
Describe        :
*/
int inSetMVTApplicationId(char* szMVTApplicationId)
{
        memset(srMVTRec.szMVTApplicationId, 0x00, sizeof(srMVTRec.szMVTApplicationId));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szMVTApplicationId == NULL || strlen(szMVTApplicationId) <= 0 || strlen(szMVTApplicationId) > 16)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetApplicationId() ERROR !!");

                        if (szMVTApplicationId == NULL)
                        {
                                inLogPrintf(AT, "szApplicationId == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szApplicationId length = (%d)", (int)strlen(szMVTApplicationId));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szMVTApplicationId[0], &szMVTApplicationId[0], strlen(szMVTApplicationId));

        return (VS_SUCCESS);
}

/*
Function        :inGetMVTTerminalType
Date&Time       :
Describe        :
*/
int inGetMVTTerminalType(char* szTerminalType)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTerminalType == NULL || strlen(srMVTRec.szMVTTerminalType) <= 0 || strlen(srMVTRec.szMVTTerminalType) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetMVTTerminalType() ERROR !!");

                        if (szTerminalType == NULL)
                        {
                                inLogPrintf(AT, "szTerminalType == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTerminalType length = (%d)", (int)strlen(srMVTRec.szMVTTerminalType));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTerminalType[0], &srMVTRec.szMVTTerminalType[0], strlen(srMVTRec.szMVTTerminalType));

        return (VS_SUCCESS);
}

/*
Function        :inSetTerminalType
Date&Time       :
Describe        :
*/
int inSetMVTTerminalType(char* szTerminalType)
{
        memset(srMVTRec.szMVTTerminalType, 0x00, sizeof(srMVTRec.szMVTTerminalType));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTerminalType == NULL || strlen(szTerminalType) <= 0 || strlen(szTerminalType) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTerminalType() ERROR !!");

                        if (szTerminalType == NULL)
                        {
                                inLogPrintf(AT, "szTerminalType == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTerminalType length = (%d)", (int)strlen(szTerminalType));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szMVTTerminalType[0], &szTerminalType[0], strlen(szTerminalType));

        return (VS_SUCCESS);
}

/*
Function        :inGetTerminalCapabilities
Date&Time       :
Describe        :
*/
int inGetTerminalCapabilities(char* szTerminalCapabilities)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTerminalCapabilities == NULL || strlen(srMVTRec.szTerminalCapabilities) <= 0 || strlen(srMVTRec.szTerminalCapabilities) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTerminalCapabilities() ERROR !!");

                        if (szTerminalCapabilities == NULL)
                        {
                                inLogPrintf(AT, "szTerminalCapabilities == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTerminalCapabilities length = (%d)", (int)strlen(srMVTRec.szTerminalCapabilities));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTerminalCapabilities[0], &srMVTRec.szTerminalCapabilities[0], strlen(srMVTRec.szTerminalCapabilities));

        return (VS_SUCCESS);
}

/*
Function        :inSetTerminalCapabilities
Date&Time       :
Describe        :
*/
int inSetTerminalCapabilities(char* szTerminalCapabilities)
{
        memset(srMVTRec.szTerminalCapabilities, 0x00, sizeof(srMVTRec.szTerminalCapabilities));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTerminalCapabilities == NULL || strlen(szTerminalCapabilities) <= 0 || strlen(szTerminalCapabilities) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTerminalCapabilities() ERROR !!");

                        if (szTerminalCapabilities == NULL)
                        {
                                inLogPrintf(AT, "szTerminalCapabilities == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTerminalCapabilities length = (%d)", (int)strlen(szTerminalCapabilities));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szTerminalCapabilities[0], &szTerminalCapabilities[0], strlen(szTerminalCapabilities));

        return (VS_SUCCESS);
}

/*
Function        :inGetAdditionalTerminalCapabilities
Date&Time       :
Describe        :
*/
int inGetAdditionalTerminalCapabilities(char* szAdditionalTerminalCapabilities)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szAdditionalTerminalCapabilities == NULL || strlen(srMVTRec.szAdditionalTerminalCapabilities) <= 0 || strlen(srMVTRec.szAdditionalTerminalCapabilities) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetAdditionalTerminalCapabilities() ERROR !!");

                        if (szAdditionalTerminalCapabilities == NULL)
                        {
                                inLogPrintf(AT, "szAdditionalTerminalCapabilities == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szAdditionalTerminalCapabilities length = (%d)", (int)strlen(srMVTRec.szAdditionalTerminalCapabilities));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szAdditionalTerminalCapabilities[0], &srMVTRec.szAdditionalTerminalCapabilities[0], strlen(srMVTRec.szAdditionalTerminalCapabilities));

        return (VS_SUCCESS);
}

/*
Function        :inSetAdditionalTerminalCapabilities
Date&Time       :
Describe        :
*/
int inSetAdditionalTerminalCapabilities(char* szAdditionalTerminalCapabilities)
{
        memset(srMVTRec.szAdditionalTerminalCapabilities, 0x00, sizeof(srMVTRec.szAdditionalTerminalCapabilities));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szAdditionalTerminalCapabilities == NULL || strlen(szAdditionalTerminalCapabilities) <= 0 || strlen(szAdditionalTerminalCapabilities) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetAdditionalTerminalCapabilities() ERROR !!");

                        if (szAdditionalTerminalCapabilities == NULL)
                        {
                                inLogPrintf(AT, "szAdditionalTerminalCapabilities == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szAdditionalTerminalCapabilities length = (%d)", (int)strlen(szAdditionalTerminalCapabilities));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szAdditionalTerminalCapabilities[0], &szAdditionalTerminalCapabilities[0], strlen(szAdditionalTerminalCapabilities));

        return (VS_SUCCESS);
}

/*
Function        :inGetMVTTerminalCountryCode
Date&Time       :
Describe        :
*/
int inGetMVTTerminalCountryCode(char* szTerminalCountryCode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTerminalCountryCode == NULL || strlen(srMVTRec.szTerminalCountryCode) <= 0 || strlen(srMVTRec.szTerminalCountryCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetMVTTerminalCountryCode() ERROR !!");

                        if (szTerminalCountryCode == NULL)
                        {
                                inLogPrintf(AT, "szTerminalCountryCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTerminalCountryCode length = (%d)", (int)strlen(srMVTRec.szTerminalCountryCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTerminalCountryCode[0], &srMVTRec.szTerminalCountryCode[0], strlen(srMVTRec.szTerminalCountryCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetMVTTerminalCountryCode
Date&Time       :
Describe        :
*/
int inSetMVTTerminalCountryCode(char* szTerminalCountryCode)
{
        memset(srMVTRec.szTerminalCountryCode, 0x00, sizeof(srMVTRec.szTerminalCountryCode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTerminalCountryCode == NULL || strlen(szTerminalCountryCode) <= 0 || strlen(szTerminalCountryCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTerminalCountryCode() ERROR !!");

                        if (szTerminalCountryCode == NULL)
                        {
                                inLogPrintf(AT, "szTerminalCountryCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTerminalCountryCode length = (%d)", (int)strlen(szTerminalCountryCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szTerminalCountryCode[0], &szTerminalCountryCode[0], strlen(szTerminalCountryCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetMVTTransactionCurrencyCode
Date&Time       :
Describe        :
*/
int inGetMVTTransactionCurrencyCode(char* szTransactionCurrencyCode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTransactionCurrencyCode == NULL || strlen(srMVTRec.szMVTTransactionCurrencyCode) <= 0 || strlen(srMVTRec.szMVTTransactionCurrencyCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetMVTTransactionCurrencyCode() ERROR !!");

                        if (szTransactionCurrencyCode == NULL)
                        {
                                inLogPrintf(AT, "szTransactionCurrencyCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionCurrencyCode length = (%d)", (int)strlen(srMVTRec.szMVTTransactionCurrencyCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTransactionCurrencyCode[0], &srMVTRec.szMVTTransactionCurrencyCode[0], strlen(srMVTRec.szMVTTransactionCurrencyCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetMVTTransactionCurrencyCode
Date&Time       :
Describe        :
*/
int inSetMVTTransactionCurrencyCode(char* szTransactionCurrencyCode)
{
        memset(srMVTRec.szMVTTransactionCurrencyCode, 0x00, sizeof(srMVTRec.szMVTTransactionCurrencyCode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTransactionCurrencyCode == NULL || strlen(szTransactionCurrencyCode) <= 0 || strlen(szTransactionCurrencyCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTransactionCurrencyCode() ERROR !!");

                        if (szTransactionCurrencyCode == NULL)
                        {
                                inLogPrintf(AT, "szTransactionCurrencyCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionCurrencyCode length = (%d)", (int)strlen(szTransactionCurrencyCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szMVTTransactionCurrencyCode[0], &szTransactionCurrencyCode[0], strlen(szTransactionCurrencyCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetDefaultTAC
Date&Time       :
Describe        :
*/
int inGetDefaultTAC(char* szDefaultTAC)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szDefaultTAC == NULL || strlen(srMVTRec.szDefaultTAC) <= 0 || strlen(srMVTRec.szDefaultTAC) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetDefaultTAC() ERROR !!");

                        if (szDefaultTAC == NULL)
                        {
                                inLogPrintf(AT, "szDefaultTAC == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDefaultTAC length = (%d)", (int)strlen(srMVTRec.szDefaultTAC));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szDefaultTAC[0], &srMVTRec.szDefaultTAC[0], strlen(srMVTRec.szDefaultTAC));

        return (VS_SUCCESS);
}

/*
Function        :inSetDefaultTAC
Date&Time       :
Describe        :
*/
int inSetDefaultTAC(char* szDefaultTAC)
{
        memset(srMVTRec.szDefaultTAC, 0x00, sizeof(srMVTRec.szDefaultTAC));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szDefaultTAC == NULL || strlen(szDefaultTAC) <= 0 || strlen(szDefaultTAC) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetDefaultTAC() ERROR !!");

                        if (szDefaultTAC == NULL)
                        {
                                inLogPrintf(AT, "szDefaultTAC == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDefaultTAC length = (%d)", (int)strlen(szDefaultTAC));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szDefaultTAC[0], &szDefaultTAC[0], strlen(szDefaultTAC));

        return (VS_SUCCESS);
}

/*
Function        :inGetOnlineTAC
Date&Time       :
Describe        :
*/
int inGetOnlineTAC(char* szOnlineTAC)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szOnlineTAC == NULL || strlen(srMVTRec.szOnlineTAC) <= 0 || strlen(srMVTRec.szOnlineTAC) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetOnlineTAC() ERROR !!");

                        if (szOnlineTAC == NULL)
                        {
                                inLogPrintf(AT, "szOnlineTAC == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szOnlineTAC length = (%d)", (int)strlen(srMVTRec.szOnlineTAC));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szOnlineTAC[0], &srMVTRec.szOnlineTAC[0], strlen(srMVTRec.szOnlineTAC));

        return (VS_SUCCESS);
}

/*
Function        :inSetOnlineTAC
Date&Time       :
Describe        :
*/
int inSetOnlineTAC(char* szOnlineTAC)
{
        memset(srMVTRec.szOnlineTAC, 0x00, sizeof(srMVTRec.szOnlineTAC));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szOnlineTAC == NULL || strlen(szOnlineTAC) <= 0 || strlen(szOnlineTAC) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetOnlineTAC() ERROR !!");

                        if (szOnlineTAC == NULL)
                        {
                                inLogPrintf(AT, "szOnlineTAC == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szOnlineTAC length = (%d)", (int)strlen(szOnlineTAC));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szOnlineTAC[0], &szOnlineTAC[0], strlen(szOnlineTAC));

        return (VS_SUCCESS);
}

/*
Function        :inGetDenialTAC
Date&Time       :
Describe        :
*/
int inGetDenialTAC(char* szDenialTAC)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szDenialTAC == NULL || strlen(srMVTRec.szDenialTAC) <= 0 || strlen(srMVTRec.szDenialTAC) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetDenialTAC() ERROR !!");

                        if (szDenialTAC == NULL)
                        {
                                inLogPrintf(AT, "szDenialTAC == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDenialTAC length = (%d)", (int)strlen(srMVTRec.szDenialTAC));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szDenialTAC[0], &srMVTRec.szDenialTAC[0], strlen(srMVTRec.szDenialTAC));

        return (VS_SUCCESS);
}

/*
Function        :inSetDenialTAC
Date&Time       :
Describe        :
*/
int inSetDenialTAC(char* szDenialTAC)
{
        memset(srMVTRec.szDenialTAC, 0x00, sizeof(srMVTRec.szDenialTAC));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szDenialTAC == NULL || strlen(szDenialTAC) <= 0 || strlen(szDenialTAC) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetDenialTAC() ERROR !!");

                        if (szDenialTAC == NULL)
                        {
                                inLogPrintf(AT, "szDenialTAC == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDenialTAC length = (%d)", (int)strlen(szDenialTAC));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szDenialTAC[0], &szDenialTAC[0], strlen(szDenialTAC));

        return (VS_SUCCESS);
}

/*
Function        :inGetDefaultTDOL
Date&Time       :
Describe        :
*/
int inGetDefaultTDOL(char* szDefaultTDOL)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szDefaultTDOL == NULL || strlen(srMVTRec.szDefaultTDOL) <= 0 || strlen(srMVTRec.szDefaultTDOL) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetDefaultTDOL() ERROR !!");

                        if (szDefaultTDOL == NULL)
                        {
                                inLogPrintf(AT, "szDefaultTDOL == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDefaultTDOL length = (%d)", (int)strlen(srMVTRec.szDefaultTDOL));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szDefaultTDOL[0], &srMVTRec.szDefaultTDOL[0], strlen(srMVTRec.szDefaultTDOL));

        return (VS_SUCCESS);

}

/*
Function        :inSetDefaultTDOL
Date&Time       :
Describe        :
*/
int inSetDefaultTDOL(char* szDefaultTDOL)
{
        memset(srMVTRec.szDefaultTDOL, 0x00, sizeof(srMVTRec.szDefaultTDOL));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szDefaultTDOL == NULL || strlen(szDefaultTDOL) <= 0 || strlen(szDefaultTDOL) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetDefaultTDOL() ERROR !!");

                        if (szDefaultTDOL == NULL)
                        {
                                inLogPrintf(AT, "szDefaultTDOL == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDefaultTDOL length = (%d)", (int)strlen(szDefaultTDOL));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szDefaultTDOL[0], &szDefaultTDOL[0], strlen(szDefaultTDOL));

        return (VS_SUCCESS);
}

/*
Function        :inGetDefaultDDOL
Date&Time       :
Describe        :
*/
int inGetDefaultDDOL(char* szDefaultDDOL)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szDefaultDDOL == NULL || strlen(srMVTRec.szDefaultDDOL) <= 0 || strlen(srMVTRec.szDefaultDDOL) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetDefaultDDOL() ERROR !!");

                        if (szDefaultDDOL == NULL)
                        {
                                inLogPrintf(AT, "szDefaultDDOL == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDefaultDDOL length = (%d)", (int)strlen(srMVTRec.szDefaultDDOL));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szDefaultDDOL[0], &srMVTRec.szDefaultDDOL[0], strlen(srMVTRec.szDefaultDDOL));

        return (VS_SUCCESS);
}

/*
Function        :inSetDefaultDDOL
Date&Time       :
Describe        :
*/
int inSetDefaultDDOL(char* szDefaultDDOL)
{
        memset(srMVTRec.szDefaultDDOL, 0x00, sizeof(srMVTRec.szDefaultDDOL));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szDefaultDDOL == NULL || strlen(szDefaultDDOL) <= 0 || strlen(szDefaultDDOL) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetDefaultDDOL() ERROR !!");

                        if (szDefaultDDOL == NULL)
                        {
                                inLogPrintf(AT, "szDefaultDDOL == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDefaultDDOL length = (%d)", (int)strlen(szDefaultDDOL));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szDefaultDDOL[0], &szDefaultDDOL[0], strlen(szDefaultDDOL));

        return (VS_SUCCESS);
}

/*
Function        :inGetEMVFloorLimit
Date&Time       :
Describe        :
*/
int inGetEMVFloorLimit(char* szEMVFloorLimit)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szEMVFloorLimit == NULL || strlen(srMVTRec.szEMVFloorLimit) <= 0 || strlen(srMVTRec.szEMVFloorLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetEMVFloorLimit() ERROR !!");

                        if (szEMVFloorLimit == NULL)
                        {
                                inLogPrintf(AT, "szEMVFloorLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEMVFloorLimit length = (%d)", (int)strlen(srMVTRec.szEMVFloorLimit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szEMVFloorLimit[0], &srMVTRec.szEMVFloorLimit[0], strlen(srMVTRec.szEMVFloorLimit));

        return (VS_SUCCESS);
}

/*
Function        :inSetEMVFloorLimit
Date&Time       :
Describe        :
*/
int inSetEMVFloorLimit(char* szEMVFloorLimit)
{
        memset(srMVTRec.szEMVFloorLimit, 0x00, sizeof(srMVTRec.szEMVFloorLimit));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szEMVFloorLimit == NULL || strlen(szEMVFloorLimit) <= 0 || strlen(szEMVFloorLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetEMVFloorLimit() ERROR !!");

                        if (szEMVFloorLimit == NULL)
                        {
                                inLogPrintf(AT, "szEMVFloorLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEMVFloorLimit length = (%d)", (int)strlen(szEMVFloorLimit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szEMVFloorLimit[0], &szEMVFloorLimit[0], strlen(szEMVFloorLimit));

        return (VS_SUCCESS);
}

/*
Function        :inGetRandomSelectionThreshold
Date&Time       :
Describe        :
*/
int inGetRandomSelectionThreshold(char* szRandomSelectionThreshold)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szRandomSelectionThreshold == NULL || strlen(srMVTRec.szRandomSelectionThreshold) <= 0 || strlen(srMVTRec.szRandomSelectionThreshold) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetRandomSelectionThreshold() ERROR !!");

                        if (szRandomSelectionThreshold == NULL)
                        {
                                inLogPrintf(AT, "szRandomSelectionThreshold == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szRandomSelectionThreshold length = (%d)", (int)strlen(srMVTRec.szRandomSelectionThreshold));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szRandomSelectionThreshold[0], &srMVTRec.szRandomSelectionThreshold[0], strlen(srMVTRec.szRandomSelectionThreshold));

        return (VS_SUCCESS);
}

/*
Function        :inSetRandomSelectionThreshold
Date&Time       :
Describe        :
*/
int inSetRandomSelectionThreshold(char* szRandomSelectionThreshold)
{
        memset(srMVTRec.szRandomSelectionThreshold, 0x00, sizeof(srMVTRec.szRandomSelectionThreshold));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szRandomSelectionThreshold == NULL || strlen(szRandomSelectionThreshold) <= 0 || strlen(szRandomSelectionThreshold) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetRandomSelectionThreshold() ERROR !!");

                        if (szRandomSelectionThreshold == NULL)
                        {
                                inLogPrintf(AT, "szRandomSelectionThreshold == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szRandomSelectionThreshold length = (%d)", (int)strlen(szRandomSelectionThreshold));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szRandomSelectionThreshold[0], &szRandomSelectionThreshold[0], strlen(szRandomSelectionThreshold));

        return (VS_SUCCESS);
}

/*
Function        :inGetTargetPercentforRandomSelection
Date&Time       :
Describe        :
*/
int inGetTargetPercentforRandomSelection(char* szTargetPercentforRandomSelection)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTargetPercentforRandomSelection == NULL || strlen(srMVTRec.szTargetPercentforRandomSelection) <= 0 || strlen(srMVTRec.szTargetPercentforRandomSelection) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTargetPercentforRandomSelection() ERROR !!");

                        if (szTargetPercentforRandomSelection == NULL)
                        {
                                inLogPrintf(AT, "szTargetPercentforRandomSelection == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTargetPercentforRandomSelection length = (%d)", (int)strlen(srMVTRec.szTargetPercentforRandomSelection));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTargetPercentforRandomSelection[0], &srMVTRec.szTargetPercentforRandomSelection[0], strlen(srMVTRec.szTargetPercentforRandomSelection));

        return (VS_SUCCESS);
}

/*
Function        :inSetTargetPercentforRandomSelection
Date&Time       :
Describe        :
*/
int inSetTargetPercentforRandomSelection(char* szTargetPercentforRandomSelection)
{
        memset(srMVTRec.szTargetPercentforRandomSelection, 0x00, sizeof(srMVTRec.szTargetPercentforRandomSelection));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTargetPercentforRandomSelection == NULL || strlen(szTargetPercentforRandomSelection) <= 0 || strlen(szTargetPercentforRandomSelection) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTargetPercentforRandomSelection() ERROR !!");

                        if (szTargetPercentforRandomSelection == NULL)
                        {
                                inLogPrintf(AT, "szTargetPercentforRandomSelection == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTargetPercentforRandomSelection length = (%d)", (int)strlen(szTargetPercentforRandomSelection));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szTargetPercentforRandomSelection[0], &szTargetPercentforRandomSelection[0], strlen(szTargetPercentforRandomSelection));

        return (VS_SUCCESS);
}

/*
Function        :inGetMaxTargetPercentforRandomSelection
Date&Time       :
Describe        :
*/
int inGetMaxTargetPercentforRandomSelection(char* szMaxTargetPercentforRandomSelection)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szMaxTargetPercentforRandomSelection == NULL || strlen(srMVTRec.szMaxTargetPercentforRandomSelection) <= 0 || strlen(srMVTRec.szMaxTargetPercentforRandomSelection) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetMaxTargetPercentforRandomSelection() ERROR !!");

                        if (szMaxTargetPercentforRandomSelection == NULL)
                        {
                                inLogPrintf(AT, "szMaxTargetPercentforRandomSelection == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMaxTargetPercentforRandomSelection length = (%d)", (int)strlen(srMVTRec.szMaxTargetPercentforRandomSelection));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMaxTargetPercentforRandomSelection[0], &srMVTRec.szMaxTargetPercentforRandomSelection[0], strlen(srMVTRec.szMaxTargetPercentforRandomSelection));

        return (VS_SUCCESS);
}

/*
Function        :inSetMaxTargetPercentforRandomSelection
Date&Time       :
Describe        :
*/
int inSetMaxTargetPercentforRandomSelection(char* szMaxTargetPercentforRandomSelection)
{
        memset(srMVTRec.szMaxTargetPercentforRandomSelection, 0x00, sizeof(srMVTRec.szMaxTargetPercentforRandomSelection));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szMaxTargetPercentforRandomSelection == NULL || strlen(szMaxTargetPercentforRandomSelection) <= 0 || strlen(szMaxTargetPercentforRandomSelection) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetMaxTargetPercentforRandomSelection() ERROR !!");

                        if (szMaxTargetPercentforRandomSelection == NULL)
                        {
                                inLogPrintf(AT, "szMaxTargetPercentforRandomSelection == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMaxTargetPercentforRandomSelection length = (%d)", (int)strlen(szMaxTargetPercentforRandomSelection));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szMaxTargetPercentforRandomSelection[0], &szMaxTargetPercentforRandomSelection[0], strlen(szMaxTargetPercentforRandomSelection));

        return (VS_SUCCESS);
}

/*
Function        :inGetMVTMerchantCategoryCode
Date&Time       :
Describe        :
*/
int inGetMVTMerchantCategoryCode(char* szMerchantCategoryCode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szMerchantCategoryCode == NULL || strlen(srMVTRec.szMVTMerchantCategoryCode) <= 0 || strlen(srMVTRec.szMVTMerchantCategoryCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetMVTMerchantCategoryCode() ERROR !!");

                        if (szMerchantCategoryCode == NULL)
                        {
                                inLogPrintf(AT, "szMerchantCategoryCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMerchantCategoryCode length = (%d)", (int)strlen(srMVTRec.szMVTMerchantCategoryCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMerchantCategoryCode[0], &srMVTRec.szMVTMerchantCategoryCode[0], strlen(srMVTRec.szMVTMerchantCategoryCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetMVTMerchantCategoryCode
Date&Time       :
Describe        :
*/
int inSetMVTMerchantCategoryCode(char* szMerchantCategoryCode)
{
        memset(srMVTRec.szMVTMerchantCategoryCode, 0x00, sizeof(srMVTRec.szMVTMerchantCategoryCode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szMerchantCategoryCode == NULL || strlen(szMerchantCategoryCode) <= 0 || strlen(szMerchantCategoryCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetMerchantCategoryCode() ERROR !!");

                        if (szMerchantCategoryCode == NULL)
                        {
                                inLogPrintf(AT, "szMerchantCategoryCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMerchantCategoryCode length = (%d)", (int)strlen(szMerchantCategoryCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szMVTMerchantCategoryCode[0], &szMerchantCategoryCode[0], strlen(szMerchantCategoryCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetMVTTransactionCategoryCode
Date&Time       :
Describe        :
*/
int inGetMVTTransactionCategoryCode(char* szTransactionCategoryCode)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTransactionCategoryCode == NULL || strlen(srMVTRec.szMVTTransactionCategoryCode) <= 0 || strlen(srMVTRec.szMVTTransactionCategoryCode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetMVTTransactionCategoryCode() ERROR !!");

                        if (szTransactionCategoryCode == NULL)
                        {
                                inLogPrintf(AT, "szTransactionCategoryCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionCategoryCode length = (%d)", (int)strlen(srMVTRec.szMVTTransactionCategoryCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTransactionCategoryCode[0], &srMVTRec.szMVTTransactionCategoryCode[0], strlen(srMVTRec.szMVTTransactionCategoryCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetMVTTransactionCategoryCode
Date&Time       :
Describe        :
*/
int inSetMVTTransactionCategoryCode(char* szTransactionCategoryCode)
{
        memset(srMVTRec.szMVTTransactionCategoryCode, 0x00, sizeof(srMVTRec.szMVTTransactionCategoryCode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTransactionCategoryCode == NULL || strlen(szTransactionCategoryCode) <= 0 || strlen(szTransactionCategoryCode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        if (szTransactionCategoryCode == NULL)
                        {
                                inLogPrintf(AT, "inSetSloganPrtPositio() ERROR !! szTransactionCategoryCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "inSetSloganPrtPositio() ERROR !! szTransactionCategoryCode length = (%d)", (int)strlen(szTransactionCategoryCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szMVTTransactionCategoryCode[0], &szTransactionCategoryCode[0], strlen(szTransactionCategoryCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetTransactionReferenceCurrencyCode
Date&Time       :
Describe        :
*/
int inGetTransactionReferenceCurrencyCode(char* szTransactionReferenceCurrencyCode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTransactionReferenceCurrencyCode == NULL || strlen(srMVTRec.szTransactionReferenceCurrencyCode) <= 0 || strlen(srMVTRec.szTransactionReferenceCurrencyCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTransactionReferenceCurrencyCode() ERROR !!");

                        if (szTransactionReferenceCurrencyCode == NULL)
                        {
                                inLogPrintf(AT, "szTransactionReferenceCurrencyCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionReferenceCurrencyCode length = (%d)", (int)strlen(srMVTRec.szTransactionReferenceCurrencyCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTransactionReferenceCurrencyCode[0], &srMVTRec.szTransactionReferenceCurrencyCode[0], strlen(srMVTRec.szTransactionReferenceCurrencyCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetTransactionReferenceCurrencyCode
Date&Time       :
Describe        :
*/
int inSetTransactionReferenceCurrencyCode(char* szTransactionReferenceCurrencyCode)
{
        memset(srMVTRec.szTransactionReferenceCurrencyCode, 0x00, sizeof(srMVTRec.szTransactionReferenceCurrencyCode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTransactionReferenceCurrencyCode == NULL || strlen(szTransactionReferenceCurrencyCode) <= 0 || strlen(szTransactionReferenceCurrencyCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTransactionReferenceCurrencyCode() ERROR !!");

                        if (szTransactionReferenceCurrencyCode == NULL)
                        {
                                inLogPrintf(AT, "szTransactionReferenceCurrencyCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionReferenceCurrencyCode length = (%d)", (int)strlen(szTransactionReferenceCurrencyCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szTransactionReferenceCurrencyCode[0], &szTransactionReferenceCurrencyCode[0], strlen(szTransactionReferenceCurrencyCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetTransactionReferenceCurrencyCoversion
Date&Time       :
Describe        :
*/
int inGetTransactionReferenceCurrencyCoversion(char* szTransactionReferenceCurrencyCoversion)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTransactionReferenceCurrencyCoversion == NULL || strlen(srMVTRec.szTransactionReferenceCurrencyCoversion) <= 0 || strlen(srMVTRec.szTransactionReferenceCurrencyCoversion) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTransactionReferenceCurrencyCoversion() ERROR !!");

                        if (szTransactionReferenceCurrencyCoversion == NULL)
                        {
                                inLogPrintf(AT, "szTransactionReferenceCurrencyCoversion == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionReferenceCurrencyCoversion length = (%d)", (int)strlen(srMVTRec.szTransactionReferenceCurrencyCoversion));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTransactionReferenceCurrencyCoversion[0], &srMVTRec.szTransactionReferenceCurrencyCoversion[0], strlen(srMVTRec.szTransactionReferenceCurrencyCoversion));

        return (VS_SUCCESS);
}

/*
Function        :inSetTransactionReferenceCurrencyCoversion
Date&Time       :
Describe        :
*/
int inSetTransactionReferenceCurrencyCoversion(char* szTransactionReferenceCurrencyCoversion)
{
        memset(srMVTRec.szTransactionReferenceCurrencyCoversion, 0x00, sizeof(srMVTRec.szTransactionReferenceCurrencyCoversion));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTransactionReferenceCurrencyCoversion == NULL || strlen(szTransactionReferenceCurrencyCoversion) <= 0 || strlen(szTransactionReferenceCurrencyCoversion) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTransactionReferenceCurrencyCoversion() ERROR !!");

                        if (szTransactionReferenceCurrencyCoversion == NULL)
                        {
                                inLogPrintf(AT, "szTransactionReferenceCurrencyCoversion == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionReferenceCurrencyCoversion length = (%d)", (int)strlen(szTransactionReferenceCurrencyCoversion));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szTransactionReferenceCurrencyCoversion[0], &szTransactionReferenceCurrencyCoversion[0], strlen(szTransactionReferenceCurrencyCoversion));

        return (VS_SUCCESS);
}

/*
Function        :inGetTransactionReferenceCurrencyExponent
Date&Time       :
Describe        :
*/
int inGetTransactionReferenceCurrencyExponent(char* szTransactionReferenceCurrencyExponent)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTransactionReferenceCurrencyExponent == NULL || strlen(srMVTRec.szTransactionReferenceCurrencyExponent) <= 0 || strlen(srMVTRec.szTransactionReferenceCurrencyExponent) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTransactionReferenceCurrencyExponent() ERROR !!");

                        if (szTransactionReferenceCurrencyExponent == NULL)
                        {
                                inLogPrintf(AT, "szTransactionReferenceCurrencyExponent == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionReferenceCurrencyExponent length = (%d)", (int)strlen(srMVTRec.szTransactionReferenceCurrencyExponent));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTransactionReferenceCurrencyExponent[0], &srMVTRec.szTransactionReferenceCurrencyExponent[0], strlen(srMVTRec.szTransactionReferenceCurrencyExponent));

        return (VS_SUCCESS);
}

/*
Function        :inSetTransactionReferenceCurrencyExponent
Date&Time       :
Describe        :
*/
int inSetTransactionReferenceCurrencyExponent(char* szTransactionReferenceCurrencyExponent)
{
        memset(srMVTRec.szTransactionReferenceCurrencyExponent, 0x00, sizeof(srMVTRec.szTransactionReferenceCurrencyExponent));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTransactionReferenceCurrencyExponent == NULL || strlen(szTransactionReferenceCurrencyExponent) <= 0 || strlen(szTransactionReferenceCurrencyExponent) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTransactionReferenceCurrencyExponent() ERROR !!");

                        if (szTransactionReferenceCurrencyExponent == NULL)
                        {
                                inLogPrintf(AT, "szTransactionReferenceCurrencyExponent == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionReferenceCurrencyExponent length = (%d)", (int)strlen(szTransactionReferenceCurrencyExponent));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szTransactionReferenceCurrencyExponent[0], &szTransactionReferenceCurrencyExponent[0], strlen(szTransactionReferenceCurrencyExponent));

        return (VS_SUCCESS);
}

/*
Function        :inGetEMVPINEntryTimeout
Date&Time       :
Describe        :
*/
int inGetEMVPINEntryTimeout(char* szEMVPINEntryTimeout)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szEMVPINEntryTimeout == NULL || strlen(srMVTRec.szEMVPINEntryTimeout) <= 0 || strlen(srMVTRec.szEMVPINEntryTimeout) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetEMVPINEntryTimeout() ERROR !!");

                        if (szEMVPINEntryTimeout == NULL)
                        {
                                inLogPrintf(AT, "szEMVPINEntryTimeout == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEMVPINEntryTimeout length = (%d)", (int)strlen(srMVTRec.szEMVPINEntryTimeout));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szEMVPINEntryTimeout[0], &srMVTRec.szEMVPINEntryTimeout[0], strlen(srMVTRec.szEMVPINEntryTimeout));

        return (VS_SUCCESS);

}

/*
Function        :inSetEMVPINEntryTimeout
Date&Time       :
Describe        :
*/
int inSetEMVPINEntryTimeout(char* szEMVPINEntryTimeout)
{
        memset(srMVTRec.szEMVPINEntryTimeout, 0x00, sizeof(srMVTRec.szEMVPINEntryTimeout));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szEMVPINEntryTimeout == NULL || strlen(szEMVPINEntryTimeout) <= 0 || strlen(szEMVPINEntryTimeout) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetEMVPINEntryTimeout() ERROR !!");

                        if (szEMVPINEntryTimeout == NULL)
                        {
                                inLogPrintf(AT, "szEMVPINEntryTimeout == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEMVPINEntryTimeout length = (%d)", (int)strlen(szEMVPINEntryTimeout));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srMVTRec.szEMVPINEntryTimeout[0], &szEMVPINEntryTimeout[0], strlen(szEMVPINEntryTimeout));

        return (VS_SUCCESS);
}

/*
Function        :inMVT_Edit_MVT_Table
Date&Time       :2018/10/12 下午 3:46
Describe        :
*/
int inMVT_Edit_MVT_Table(void)
{
	TABLE_GET_SET_TABLE MVT_FUNC_TABLE[] =
	{
		{"szApplicationIndex"				,inGetApplicationIndex				,inSetApplicationIndex				},
		{"szMVTApplicationId"				,inGetMVTApplicationId				,inSetMVTApplicationId				},
		{"szMVTTerminalType"				,inGetMVTTerminalType				,inSetMVTTerminalType				},
		{"szTerminalCapabilities"			,inGetTerminalCapabilities			,inSetTerminalCapabilities			},
		{"szAdditionalTerminalCapabilities"		,inGetAdditionalTerminalCapabilities		,inSetAdditionalTerminalCapabilities		},
		{"szMVTTerminalCountryCode"			,inGetMVTTerminalCountryCode			,inSetMVTTerminalCountryCode			},
		{"szMVTTransactionCurrencyCode"			,inGetMVTTransactionCurrencyCode		,inSetMVTTransactionCurrencyCode		},
		{"szDefaultTAC"					,inGetDefaultTAC				,inSetDefaultTAC				},
		{"szOnlineTAC"					,inGetOnlineTAC					,inSetOnlineTAC					},
		{"szDenialTAC"					,inGetDenialTAC					,inSetDenialTAC					},
		{"szDefaultTDOL"				,inGetDefaultTDOL				,inSetDefaultTDOL				},
		{"szDefaultDDOL"				,inGetDefaultDDOL				,inSetDefaultDDOL				},
		{"szEMVFloorLimit"				,inGetEMVFloorLimit				,inSetEMVFloorLimit				},
		{"szRandomSelectionThreshold"			,inGetRandomSelectionThreshold			,inSetRandomSelectionThreshold			},
		{"szTargetPercentforRandomSelection"		,inGetTargetPercentforRandomSelection		,inSetTargetPercentforRandomSelection		},
		{"szMaxTargetPercentforRandomSelection"		,inGetMaxTargetPercentforRandomSelection	,inSetMaxTargetPercentforRandomSelection	},
		{"szMVTMerchantCategoryCode"			,inGetMVTMerchantCategoryCode			,inSetMVTMerchantCategoryCode			},
		{"szMVTTransactionCategoryCode"			,inGetMVTTransactionCategoryCode		,inSetMVTTransactionCategoryCode		},
		{"szTransactionReferenceCurrencyCode"		,inGetTransactionReferenceCurrencyCode		,inSetTransactionReferenceCurrencyCode		},
		{"szTransactionReferenceCurrencyCoversion"	,inGetTransactionReferenceCurrencyCoversion	,inSetTransactionReferenceCurrencyCoversion	},
		{"szTransactionReferenceCurrencyExponent"	,inGetTransactionReferenceCurrencyExponent	,inSetTransactionReferenceCurrencyExponent	},
		{"szEMVPINEntryTimeout"				,inGetEMVPINEntryTimeout			,inSetEMVPINEntryTimeout			},
		{""},
	};
	int		inRetVal = VS_ERROR;
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
	inDISP_ChineseFont_Color("是否更改MVT", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
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
	inLoadMVTRec(inRecordCnt);
	
	inFunc_Edit_Table_Tag(MVT_FUNC_TABLE);
	inSaveMVTRec(inRecordCnt);
	
	return	(VS_SUCCESS);
}
