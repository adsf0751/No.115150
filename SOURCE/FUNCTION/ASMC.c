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
#include "ASMC.h"

static  ASMC_REC srASMCRec;	/* construct ASMC record */
extern  int     ginDebug;  	/* Debug使用 extern */

/*
Function        :inLoadASMCRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀ASMC檔案，inASMC_Rec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadASMCRec(int inASMCRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆ASMC檔案 */
        char            szASMCRec[_SIZE_ASMC_REC_ + 1];         /* 暫存, 放各個欄位檔案 */
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnASMCLength = 0;                       /* ASMC總長度 */
        long            lnReadLength;                           /* 記錄每次要從ASMC.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;                 /* inRec記錄讀到第幾筆, i為目前從ASMC讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */

        /* inLoadASMCRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadASMCRec(%d) START!!", inASMCRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 判斷傳進來的inASMCRec是否小於零 大於等於零才是正確值(防呆) */
        if (inASMCRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inASMCRec < 0:(index = %d) ERROR!!", inASMCRec);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open ASMC.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_ASMC_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnASMCLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_ASMC_FILE_NAME_);
        
	if (lnASMCLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnASMCLength + 1);
        uszTemp = malloc(lnASMCLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnASMCLength + 1);
        memset(uszTemp, 0x00, lnASMCLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnASMCLength;

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
         *i為目前從ASMC讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnASMCLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到ASMC的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

			/* 清空uszReadData */
                        memset(uszReadData, 0x00, lnASMCLength + 1);
			/* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
			/* 因為inASMC_Rec的index從0開始，所以inASMC_Rec要+1 */
                        if (inRec == (inASMCRec + 1))
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
         * 如果沒有inASMCRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inASMCRec + 1) || inSearchResult == -1)
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
        memset(&srASMCRec, 0x00, sizeof(srASMCRec));
        /*
         * 以下pattern為存入ASMC_Rec
         * i為ASMC的第幾個字元
         * 存入ASMC_Rec
         */
        i = 0;


        /* 01_支援刷卡兌換 */
	/* 初始化 */
        memset(szASMCRec, 0x00, sizeof(szASMCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szASMCRec[k ++] = uszReadData[i ++];
                if (szASMCRec[k - 1] == 0x2C	||
		    szASMCRec[k - 1] == 0x0D	||
		    szASMCRec[k - 1] == 0x0A	||
		    szASMCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnASMCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "ASMC unpack ERROR");
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
        if (szASMCRec[0] != 0x2C	&&
	    szASMCRec[0] != 0x0D	&&
	    szASMCRec[0] != 0x0A	&&
	    szASMCRec[0] != 0x00)
        {
                memcpy(&srASMCRec.szCreditCardFlag[0], &szASMCRec[0], k - 1);
        }

        /* 02_刷卡兌換起日參數 */
	/* 初始化 */
        memset(szASMCRec, 0x00, sizeof(szASMCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szASMCRec[k ++] = uszReadData[i ++];
                if (szASMCRec[k - 1] == 0x2C	||
		    szASMCRec[k - 1] == 0x0D	||
		    szASMCRec[k - 1] == 0x0A	||
		    szASMCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnASMCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "ASMC unpack ERROR");
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
        if (szASMCRec[0] != 0x2C	&&
	    szASMCRec[0] != 0x0D	&&
	    szASMCRec[0] != 0x0A	&&
	    szASMCRec[0] != 0x00)
        {
                memcpy(&srASMCRec.szCreditCardStartDate[0], &szASMCRec[0], k - 1);
        }

        /* 03_刷卡兌換迄日參數 */
	/* 初始化 */
        memset(szASMCRec, 0x00, sizeof(szASMCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szASMCRec[k ++] = uszReadData[i ++];
                if (szASMCRec[k - 1] == 0x2C	||
		    szASMCRec[k - 1] == 0x0D	||
		    szASMCRec[k - 1] == 0x0A	||
		    szASMCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnASMCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "ASMC unpack ERROR");
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
        if (szASMCRec[0] != 0x2C	&&
	    szASMCRec[0] != 0x0D	&&
	    szASMCRec[0] != 0x0A	&&
	    szASMCRec[0] != 0x00)
        {
                memcpy(&srASMCRec.szCreditCardEndDate[0], &szASMCRec[0], k - 1);
        }

        /* 04_支援條碼兑換 */
	/* 初始化 */
        memset(szASMCRec, 0x00, sizeof(szASMCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szASMCRec[k ++] = uszReadData[i ++];
                if (szASMCRec[k - 1] == 0x2C	||
		    szASMCRec[k - 1] == 0x0D	||
		    szASMCRec[k - 1] == 0x0A	||
		    szASMCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnASMCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "ASMC unpack ERROR");
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
        if (szASMCRec[0] != 0x2C	&&
	    szASMCRec[0] != 0x0D	&&
	    szASMCRec[0] != 0x0A	&&
	    szASMCRec[0] != 0x00)
        {
                memcpy(&srASMCRec.szBarCodeFlag[0], &szASMCRec[0], k - 1);
        }

        /* 05_條碼兑換起日參數 */
	/* 初始化 */
        memset(szASMCRec, 0x00, sizeof(szASMCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szASMCRec[k ++] = uszReadData[i ++];
                if (szASMCRec[k - 1] == 0x2C	||
		    szASMCRec[k - 1] == 0x0D	||
		    szASMCRec[k - 1] == 0x0A	||
		    szASMCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnASMCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "ASMC unpack ERROR");
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
        if (szASMCRec[0] != 0x2C	&&
	    szASMCRec[0] != 0x0D	&&
	    szASMCRec[0] != 0x0A	&&
	    szASMCRec[0] != 0x00)
        {
                memcpy(&srASMCRec.szBarCodeStartDate[0], &szASMCRec[0], k - 1);
        }

        /* 06_條碼兑換迄日參數 */
	/* 初始化 */
        memset(szASMCRec, 0x00, sizeof(szASMCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szASMCRec[k ++] = uszReadData[i ++];
                if (szASMCRec[k - 1] == 0x2C	||
		    szASMCRec[k - 1] == 0x0D	||
		    szASMCRec[k - 1] == 0x0A	||
		    szASMCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnASMCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "ASMC unpack ERROR");
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
        if (szASMCRec[0] != 0x2C	&&
	    szASMCRec[0] != 0x0D	&&
	    szASMCRec[0] != 0x0A	&&
	    szASMCRec[0] != 0x00)
        {
                memcpy(&srASMCRec.szBarCodeEndDate[0], &szASMCRec[0], k - 1);
        }

        /* 07_支援條碼兌換取消 */
	/* 初始化 */
        memset(szASMCRec, 0x00, sizeof(szASMCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szASMCRec[k ++] = uszReadData[i ++];
                if (szASMCRec[k - 1] == 0x2C	||
		    szASMCRec[k - 1] == 0x0D	||
		    szASMCRec[k - 1] == 0x0A	||
		    szASMCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnASMCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "ASMC unpack ERROR");
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
        if (szASMCRec[0] != 0x2C	&&
	    szASMCRec[0] != 0x0D	&&
	    szASMCRec[0] != 0x0A	&&
	    szASMCRec[0] != 0x00)
        {
                memcpy(&srASMCRec.szVoidRedeemFlag[0], &szASMCRec[0], k - 1);
        }

        /* 08_條碼兌換取消起日參數 */
	/* 初始化 */
        memset(szASMCRec, 0x00, sizeof(szASMCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szASMCRec[k ++] = uszReadData[i ++];
                if (szASMCRec[k - 1] == 0x2C	||
		    szASMCRec[k - 1] == 0x0D	||
		    szASMCRec[k - 1] == 0x0A	||
		    szASMCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnASMCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "ASMC unpack ERROR");
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
        if (szASMCRec[0] != 0x2C	&&
	    szASMCRec[0] != 0x0D	&&
	    szASMCRec[0] != 0x0A	&&
	    szASMCRec[0] != 0x00)
        {
                memcpy(&srASMCRec.szVoidRedeemStartDate[0], &szASMCRec[0], k - 1);
        }

        /* 09_條碼兌換取消迄日參數 */
	/* 初始化 */
        memset(szASMCRec, 0x00, sizeof(szASMCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szASMCRec[k ++] = uszReadData[i ++];
                if (szASMCRec[k - 1] == 0x2C	||
		    szASMCRec[k - 1] == 0x0D	||
		    szASMCRec[k - 1] == 0x0A	||
		    szASMCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnASMCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "ASMC unpack ERROR");
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
        if (szASMCRec[0] != 0x2C	&&
	    szASMCRec[0] != 0x0D	&&
	    szASMCRec[0] != 0x0A	&&
	    szASMCRec[0] != 0x00)
        {
                memcpy(&srASMCRec.szVoidRedeemEndDate[0], &szASMCRec[0], k - 1);
        }

        /* 10_支援優惠(含請求電文)功能 */
	/* 初始化 */
        memset(szASMCRec, 0x00, sizeof(szASMCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szASMCRec[k ++] = uszReadData[i ++];
                if (szASMCRec[k - 1] == 0x2C	||
		    szASMCRec[k - 1] == 0x0D	||
		    szASMCRec[k - 1] == 0x0A	||
		    szASMCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnASMCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "ASMC unpack ERROR");
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
        if (szASMCRec[0] != 0x2C	&&
	    szASMCRec[0] != 0x0D	&&
	    szASMCRec[0] != 0x0A	&&
	    szASMCRec[0] != 0x00)
        {
                memcpy(&srASMCRec.szASMFlag[0], &szASMCRec[0], k - 1);
        }

        /* 11_優惠起日參數 */
	/* 初始化 */
        memset(szASMCRec, 0x00, sizeof(szASMCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szASMCRec[k ++] = uszReadData[i ++];
                if (szASMCRec[k - 1] == 0x2C	||
		    szASMCRec[k - 1] == 0x0D	||
		    szASMCRec[k - 1] == 0x0A	||
		    szASMCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnASMCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "ASMC unpack ERROR");
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
        if (szASMCRec[0] != 0x2C	&&
	    szASMCRec[0] != 0x0D	&&
	    szASMCRec[0] != 0x0A	&&
	    szASMCRec[0] != 0x00)
        {
                memcpy(&srASMCRec.szASMStartDate[0], &szASMCRec[0], k - 1);
        }

        /* 12_優惠迄日參數 */
	/* 初始化 */
        memset(szASMCRec, 0x00, sizeof(szASMCRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szASMCRec[k ++] = uszReadData[i ++];
                if (szASMCRec[k - 1] == 0x2C	||
		    szASMCRec[k - 1] == 0x0D	||
		    szASMCRec[k - 1] == 0x0A	||
		    szASMCRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnASMCLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "ASMC unpack ERROR");
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
        if (szASMCRec[0] != 0x2C	&&
	    szASMCRec[0] != 0x0D	&&
	    szASMCRec[0] != 0x0A	&&
	    szASMCRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srASMCRec.szASMEndDate[0], &szASMCRec[0], k - 1);
        }

        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

	/* inLoadASMCRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadASMCRec(%d) END!!", inASMCRec);
                inLogPrintf(AT, szErrorMsg);
		inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveASMCRec
Date&Time       :
Describe        :
*/
int inSaveASMCRec(int inASMCRec)
{
        unsigned long   uldat_Handle;   			/* FILE Handle */
        unsigned long   ulbak_Handle;   			/* FILE Handle */
        int     	inPackCount = 0;        		/* uszWriteBuff_Record的index */
        int     	inRecIndex = 0;         		/* uszRead_Total_Buff的index */
        int     	inASMC_Total_Rec = 0;    		/* ASMC.dat的總筆數 */
        int     	inRetVal;
        int     	i = 0, j = 0;
	char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];	/* debug message */
        long    	lnReadLength = 0;       		/* 每次要從ASMC.dat讀多少byte出來 */
        long    	lnASMCLength = 0;         		/* ASMC.dat檔案總長度 */
        unsigned char   *uszRead_Total_Buff;
        unsigned char   *uszWriteBuff_Record, *uszWriteBuff_Org;


        /* inSaveASMCRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveASMCRec(%d) START!!", inASMCRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 防呆先刪除ASMC.bak  */
        inFILE_Delete((unsigned char *)_ASMC_FILE_NAME_BAK_);

        /* 新建ASMC.bak檔案 */
        inRetVal = inFILE_Create(&ulbak_Handle, (unsigned char *)_ASMC_FILE_NAME_BAK_);

        if (inRetVal != VS_SUCCESS)
        {
                inFILE_Close(&ulbak_Handle);

                return (VS_ERROR);
        }

        /* 開啟原檔案ASMC.dat */
        inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_ASMC_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                /* ASMC.dat開檔失敗 ，不用關檔ASMC.dat */
                /* ASMC.bak仍要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_ASMC_FILE_NAME_BAK_);

                return(VS_ERROR);
        }

        /* 取得ASMC.dat檔案大小 */
        lnASMCLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *)_ASMC_FILE_NAME_);

	/* 取得檔案大小失敗 */
        if (lnASMCLength == VS_ERROR)
        {
		/* ASMC.bak和ASMC.dat要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Close(&uldat_Handle);
                inFILE_Delete((unsigned char *)_ASMC_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        uszWriteBuff_Record = malloc(_SIZE_ASMC_REC_ + _SIZE_ASMC_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_ASMC_REC_ + _SIZE_ASMC_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原ASMC.dat的每筆Record 初始化 */
        uszWriteBuff_Org = malloc(_SIZE_ASMC_REC_ + _SIZE_ASMC_COMMA_0D0A_);
        memset(uszWriteBuff_Org, 0x00, _SIZE_ASMC_REC_ + _SIZE_ASMC_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存ASMC.dat全部資料 */
        uszRead_Total_Buff = malloc(lnASMCLength + 1);
        memset(uszRead_Total_Buff, 0x00, lnASMCLength + 1);

	inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

	/* CreditCardFlag */
        memcpy(&uszWriteBuff_Record[0], &srASMCRec.szCreditCardFlag[0], strlen(srASMCRec.szCreditCardFlag));
        inPackCount += strlen(srASMCRec.szCreditCardFlag);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* CreditCardStartDate */
        memcpy(&uszWriteBuff_Record[inPackCount], &srASMCRec.szCreditCardStartDate[0], strlen(srASMCRec.szCreditCardStartDate));
        inPackCount += strlen(srASMCRec.szCreditCardStartDate);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* CreditCardEndDate */
        memcpy(&uszWriteBuff_Record[inPackCount], &srASMCRec.szCreditCardEndDate[0], strlen(srASMCRec.szCreditCardEndDate));
        inPackCount += strlen(srASMCRec.szCreditCardEndDate);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* BarCodeFlag */
        memcpy(&uszWriteBuff_Record[inPackCount], &srASMCRec.szBarCodeFlag[0], strlen(srASMCRec.szBarCodeFlag));
        inPackCount += strlen(srASMCRec.szBarCodeFlag);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* BarCodeStartDate */
        memcpy(&uszWriteBuff_Record[inPackCount], &srASMCRec.szBarCodeStartDate[0], strlen(srASMCRec.szBarCodeStartDate));
        inPackCount += strlen(srASMCRec.szBarCodeStartDate);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* BarCodeEndDate */
        memcpy(&uszWriteBuff_Record[inPackCount], &srASMCRec.szBarCodeEndDate[0], strlen(srASMCRec.szBarCodeEndDate));
        inPackCount += strlen(srASMCRec.szBarCodeEndDate);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* VoidRedeemFlag */
        memcpy(&uszWriteBuff_Record[inPackCount], &srASMCRec.szVoidRedeemFlag[0], strlen(srASMCRec.szVoidRedeemFlag));
        inPackCount += strlen(srASMCRec.szVoidRedeemFlag);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* VoidRedeemStartDate */
        memcpy(&uszWriteBuff_Record[inPackCount], &srASMCRec.szVoidRedeemStartDate[0], strlen(srASMCRec.szVoidRedeemStartDate));
        inPackCount += strlen(srASMCRec.szVoidRedeemStartDate);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* VoidRedeemEndDate */
        memcpy(&uszWriteBuff_Record[inPackCount], &srASMCRec.szVoidRedeemEndDate[0], strlen(srASMCRec.szVoidRedeemEndDate));
        inPackCount += strlen(srASMCRec.szVoidRedeemEndDate);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* ASMFlag */
        memcpy(&uszWriteBuff_Record[inPackCount], &srASMCRec.szASMFlag[0], strlen(srASMCRec.szASMFlag));
        inPackCount += strlen(srASMCRec.szASMFlag);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* ASMStartDate */
        memcpy(&uszWriteBuff_Record[inPackCount], &srASMCRec.szASMStartDate[0], strlen(srASMCRec.szASMStartDate));
        inPackCount += strlen(srASMCRec.szASMStartDate);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* ASMEndDate */
        memcpy(&uszWriteBuff_Record[inPackCount], &srASMCRec.szASMEndDate[0], strlen(srASMCRec.szASMEndDate));
        inPackCount += strlen(srASMCRec.szASMEndDate);

        /* 最後的data不用逗號 */
        /* 補上換行符號 */

        /* 0D是移至行首 */
        uszWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        uszWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;

        /* 讀ASMC.dat檔案存到uszRead_Total_Buff */
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnASMCLength;

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
                                        inFILE_Delete((unsigned char *)_ASMC_FILE_NAME_BAK_);
                                        
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
                                        inFILE_Delete((unsigned char *)_ASMC_FILE_NAME_BAK_);
                                        
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
                inFILE_Delete((unsigned char *)_ASMC_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        /* 防呆 總record數量小於要存取inASMCRec Return ERROR */
        /* 算總Record數 */
	for (i = 0; i < (lnASMCLength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (uszRead_Total_Buff[i] == 0x0A)
                        inASMC_Total_Rec++;
        }

	/* 防呆 總record數量小於要存取inASMCRec Return ERROR */
        if ((inASMCRec + 1) > inASMC_Total_Rec)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "No data or Index ERROR");
                }

                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_ASMC_FILE_NAME_BAK_);
                
                /* Free pointer */
		free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                return (VS_ERROR);
        }

        /* 記錄ReadTotalBuffer的陣列位置 */
        inRecIndex = 0;
        /* 用inASMCRec決定要先存幾筆Record到ASMC.bak，ex:inASMCRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inASMCRec; i ++)
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
                                        inFILE_Delete((unsigned char *)_ASMC_FILE_NAME_BAK_);
                                        
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
                                        inFILE_Delete((unsigned char *)_ASMC_FILE_NAME_BAK_);
                                        
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

        /* 存組好的該ASMCRecord 到 ASMC.bak */
        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_ASMC_FILE_NAME_BAK_);
                
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
                inFILE_Delete((unsigned char *)_ASMC_FILE_NAME_BAK_);
                
                /* Free pointer */
		free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原ASMC.dat Record 到 ASMC.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
	/* inRecIndex要指到剩下還沒存進bak檔的第一個記憶體位置 */
	inASMCRec = inASMCRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < lnASMCLength; i++)
        {
                if (uszRead_Total_Buff[i] == 0x0A)
                {
                        j++;

                        if (j == inASMCRec)
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
                /* 接續存原ASMC.dat的Record */
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
                                inFILE_Delete((unsigned char *)_ASMC_FILE_NAME_BAK_);
                                
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
                                inFILE_Delete((unsigned char *)_ASMC_FILE_NAME_BAK_);
                                
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

        /* 刪除原ASMC.dat */
        if (inFILE_Delete((unsigned char *)_ASMC_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* 將ASMC.bak改名字為ASMC.dat取代原檔案 */
        if (inFILE_Rename((unsigned char *)_ASMC_FILE_NAME_BAK_, (unsigned char *)_ASMC_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* inSaveASMCRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveASMCRec(%d) END!!", inASMCRec - 1);
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
Function        :inGetCreditCardFlag
Date&Time       :
Describe        :
*/
int inGetCreditCardFlag(char* szCreditCardFlag)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCreditCardFlag == NULL || strlen(srASMCRec.szCreditCardFlag) <= 0 || strlen(srASMCRec.szCreditCardFlag) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetCreditCardFlag() ERROR !!");

                        if (szCreditCardFlag == NULL)
                        {
                                inLogPrintf(AT, "szCreditCardFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCreditCardFlag length = (%d)", (int)strlen(srASMCRec.szCreditCardFlag));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCreditCardFlag[0], &srASMCRec.szCreditCardFlag[0], strlen(srASMCRec.szCreditCardFlag));

        return (VS_SUCCESS);
}

/*
Function        :inSetCreditCardFlag
Date&Time       :
Describe        :
*/
int inSetCreditCardFlag(char* szCreditCardFlag)
{
        memset(srASMCRec.szCreditCardFlag, 0x00, sizeof(srASMCRec.szCreditCardFlag));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCreditCardFlag == NULL || strlen(szCreditCardFlag) <= 0 || strlen(szCreditCardFlag) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCreditCardFlag() ERROR !!");

                        if (szCreditCardFlag == NULL)
                        {
                                inLogPrintf(AT, "szCreditCardFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCreditCardFlag length = (%d)", (int)strlen(szCreditCardFlag));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srASMCRec.szCreditCardFlag[0], &szCreditCardFlag[0], strlen(szCreditCardFlag));

        return (VS_SUCCESS);
}

/*
Function        :inGetCreditCardStartDate
Date&Time       :
Describe        :
*/
int inGetCreditCardStartDate(char* szCreditCardStartDate)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCreditCardStartDate == NULL || strlen(srASMCRec.szCreditCardStartDate) <= 0 || strlen(srASMCRec.szCreditCardStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetCreditCardStartDate() ERROR !!");

                        if (szCreditCardStartDate == NULL)
                        {
                                inLogPrintf(AT, "szCreditCardStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCreditCardStartDate length = (%d)", (int)strlen(srASMCRec.szCreditCardStartDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCreditCardStartDate[0], &srASMCRec.szCreditCardStartDate[0], strlen(srASMCRec.szCreditCardStartDate));

        return (VS_SUCCESS);

}

/*
Function        :inSetCreditCardStartDate
Date&Time       :
Describe        :
*/
int inSetCreditCardStartDate(char* szCreditCardStartDate)
{
        memset(srASMCRec.szCreditCardStartDate, 0x00, sizeof(srASMCRec.szCreditCardStartDate));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCreditCardStartDate == NULL || strlen(szCreditCardStartDate) <= 0 || strlen(szCreditCardStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCreditCardStartDate() ERROR !!");

                        if (szCreditCardStartDate == NULL)
                        {
                                inLogPrintf(AT, "szCreditCardStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCreditCardStartDate length = (%d)", (int)strlen(szCreditCardStartDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srASMCRec.szCreditCardStartDate[0], &szCreditCardStartDate[0], strlen(szCreditCardStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetCreditCardEndDate
Date&Time       :
Describe        :
*/
int inGetCreditCardEndDate(char* szCreditCardEndDate)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCreditCardEndDate == NULL || strlen(srASMCRec.szCreditCardEndDate) <= 0 || strlen(srASMCRec.szCreditCardEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetCreditCardEndDate() ERROR !!");

                        if (szCreditCardEndDate == NULL)
                        {
                                inLogPrintf(AT, "szCreditCardEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCreditCardEndDate length = (%d)", (int)strlen(szCreditCardEndDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCreditCardEndDate[0], &srASMCRec.szCreditCardEndDate[0], strlen(srASMCRec.szCreditCardEndDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetCreditCardEndDate
Date&Time       :
Describe        :
*/
int inSetCreditCardEndDate(char* szCreditCardEndDate)
{
        memset(srASMCRec.szCreditCardEndDate, 0x00, sizeof(srASMCRec.szCreditCardEndDate));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCreditCardEndDate == NULL || strlen(szCreditCardEndDate) <= 0 || strlen(szCreditCardEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCreditCardEndDate() ERROR !!");

                        if (szCreditCardEndDate == NULL)
                        {
                                inLogPrintf(AT, "szCreditCardEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCreditCardEndDate length = (%d)", (int)strlen(szCreditCardEndDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srASMCRec.szCreditCardEndDate[0], &szCreditCardEndDate[0], strlen(szCreditCardEndDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetBarCodeFlag
Date&Time       :
Describe        :
*/
int inGetBarCodeFlag(char* szBarCodeFlag)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szBarCodeFlag == NULL || strlen(srASMCRec.szBarCodeFlag) <= 0 || strlen(srASMCRec.szBarCodeFlag) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetBarCodeFlag() ERROR !!");

                        if (szBarCodeFlag == NULL)
                        {
                                inLogPrintf(AT, "szBarCodeFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBarCodeFlag length = (%d)", (int)strlen(szBarCodeFlag));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szBarCodeFlag[0], &srASMCRec.szBarCodeFlag[0], strlen(srASMCRec.szBarCodeFlag));

        return (VS_SUCCESS);
}

/*
Function        :inSetBarCodeFlag
Date&Time       :
Describe        :
*/
int inSetBarCodeFlag(char* szBarCodeFlag)
{
        memset(srASMCRec.szBarCodeFlag, 0x00, sizeof(srASMCRec.szBarCodeFlag));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szBarCodeFlag == NULL || strlen(szBarCodeFlag) <= 0 || strlen(szBarCodeFlag) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetBarCodeFlag() ERROR !!");

                        if (szBarCodeFlag == NULL)
                        {
                                inLogPrintf(AT, "szBarCodeFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBarCodeFlag length = (%d)", (int)strlen(szBarCodeFlag));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srASMCRec.szBarCodeFlag[0], &szBarCodeFlag[0], strlen(szBarCodeFlag));

        return (VS_SUCCESS);
}

/*
Function        :inGetBarCodeStartDate
Date&Time       :
Describe        :
*/
int inGetBarCodeStartDate(char* szBarCodeStartDate)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szBarCodeStartDate == NULL || strlen(srASMCRec.szBarCodeStartDate) <= 0 || strlen(srASMCRec.szBarCodeStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetBarCodeStartDate() ERROR !!");

                        if (szBarCodeStartDate == NULL)
                        {
                                inLogPrintf(AT, "szBarCodeStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBarCodeStartDate length = (%d)", (int)strlen(szBarCodeStartDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szBarCodeStartDate[0], &srASMCRec.szBarCodeStartDate[0], strlen(srASMCRec.szBarCodeStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetBarCodeStartDate
Date&Time       :
Describe        :
*/
int inSetBarCodeStartDate(char* szBarCodeStartDate)
{
        memset(srASMCRec.szBarCodeStartDate, 0x00, sizeof(srASMCRec.szBarCodeStartDate));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szBarCodeStartDate == NULL || strlen(szBarCodeStartDate) <= 0 || strlen(szBarCodeStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetBarCodeStartDate() ERROR !!");

                        if (szBarCodeStartDate == NULL)
                        {
                                inLogPrintf(AT, "szBarCodeStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBarCodeStartDate length = (%d)", (int)strlen(szBarCodeStartDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srASMCRec.szBarCodeStartDate[0], &szBarCodeStartDate[0], strlen(szBarCodeStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetBarCodeEndDate
Date&Time       :
Describe        :
*/
int inGetBarCodeEndDate(char* szBarCodeEndDate)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szBarCodeEndDate == NULL || strlen(srASMCRec.szBarCodeEndDate) <= 0 || strlen(srASMCRec.szBarCodeEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetBarCodeEndDate() ERROR !!");

                        if (szBarCodeEndDate == NULL)
                        {
                                inLogPrintf(AT, "szBarCodeEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBarCodeEndDate length = (%d)", (int)strlen(szBarCodeEndDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szBarCodeEndDate[0], &srASMCRec.szBarCodeEndDate[0], strlen(srASMCRec.szBarCodeEndDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetBarCodeEndDate
Date&Time       :
Describe        :
*/
int inSetBarCodeEndDate(char* szBarCodeEndDate)
{
        memset(srASMCRec.szBarCodeEndDate, 0x00, sizeof(srASMCRec.szBarCodeEndDate));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szBarCodeEndDate == NULL || strlen(szBarCodeEndDate) <= 0 || strlen(szBarCodeEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetBarCodeEndDate() ERROR !!");

                        if (szBarCodeEndDate == NULL)
                        {
                                inLogPrintf(AT, "szBarCodeEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBarCodeEndDate length = (%d)", (int)strlen(szBarCodeEndDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srASMCRec.szBarCodeEndDate[0], &szBarCodeEndDate[0], strlen(szBarCodeEndDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetVoidRedeemFlag
Date&Time       :
Describe        :
*/
int inGetVoidRedeemFlag(char* szVoidRedeemFlag)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szVoidRedeemFlag == NULL || strlen(srASMCRec.szVoidRedeemFlag) <= 0 || strlen(srASMCRec.szVoidRedeemFlag) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetVoidRedeemFlag() ERROR !!");

                        if (szVoidRedeemFlag == NULL)
                        {
                                inLogPrintf(AT, "szVoidRedeemFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szVoidRedeemFlag length = (%d)", (int)strlen(szVoidRedeemFlag));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szVoidRedeemFlag[0], &srASMCRec.szVoidRedeemFlag[0], strlen(srASMCRec.szVoidRedeemFlag));

        return (VS_SUCCESS);
}

/*
Function        :inSetVoidRedeemFlag
Date&Time       :
Describe        :
*/
int inSetVoidRedeemFlag(char* szVoidRedeemFlag)
{
        memset(srASMCRec.szVoidRedeemFlag, 0x00, sizeof(srASMCRec.szVoidRedeemFlag));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szVoidRedeemFlag == NULL || strlen(szVoidRedeemFlag) <= 0 || strlen(szVoidRedeemFlag) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetVoidRedeemFlag() ERROR !!");

                        if (szVoidRedeemFlag == NULL)
                        {
                                inLogPrintf(AT, "szVoidRedeemFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szVoidRedeemFlag length = (%d)", (int)strlen(szVoidRedeemFlag));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srASMCRec.szVoidRedeemFlag[0], &szVoidRedeemFlag[0], strlen(szVoidRedeemFlag));

        return (VS_SUCCESS);
}

/*
Function        :inGetVoidRedeemStartDate
Date&Time       :
Describe        :
*/
int inGetVoidRedeemStartDate(char* szVoidRedeemStartDate)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szVoidRedeemStartDate == NULL || strlen(srASMCRec.szVoidRedeemStartDate) <= 0 || strlen(srASMCRec.szVoidRedeemStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetVoidRedeemStartDate() ERROR !!");

                        if (szVoidRedeemStartDate == NULL)
                        {
                                inLogPrintf(AT, "szVoidRedeemStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szVoidRedeemStartDate length = (%d)", (int)strlen(szVoidRedeemStartDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szVoidRedeemStartDate[0], &srASMCRec.szVoidRedeemStartDate[0], strlen(srASMCRec.szVoidRedeemStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetVoidRedeemStartDate
Date&Time       :
Describe        :
*/
int inSetVoidRedeemStartDate(char* szVoidRedeemStartDate)
{
        memset(srASMCRec.szVoidRedeemStartDate, 0x00, sizeof(srASMCRec.szVoidRedeemStartDate));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szVoidRedeemStartDate == NULL || strlen(szVoidRedeemStartDate) <= 0 || strlen(szVoidRedeemStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetVoidRedeemStartDate() ERROR !!");

                        if (szVoidRedeemStartDate == NULL)
                        {
                                inLogPrintf(AT, "szVoidRedeemStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szVoidRedeemStartDate length = (%d)", (int)strlen(szVoidRedeemStartDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srASMCRec.szVoidRedeemStartDate[0], &szVoidRedeemStartDate[0], strlen(szVoidRedeemStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetVoidRedeemEndDate
Date&Time       :
Describe        :
*/
int inGetVoidRedeemEndDate(char* szVoidRedeemEndDate)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szVoidRedeemEndDate == NULL || strlen(srASMCRec.szVoidRedeemEndDate) <= 0 || strlen(srASMCRec.szVoidRedeemEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetVoidRedeemEndDate() ERROR !!");

                        if (szVoidRedeemEndDate == NULL)
                        {
                                inLogPrintf(AT, "szVoidRedeemEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szVoidRedeemEndDate length = (%d)", (int)strlen(szVoidRedeemEndDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szVoidRedeemEndDate[0], &srASMCRec.szVoidRedeemEndDate[0], strlen(srASMCRec.szVoidRedeemEndDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetVoidRedeemEndDate
Date&Time       :
Describe        :
*/
int inSetVoidRedeemEndDate(char* szVoidRedeemEndDate)
{
        memset(srASMCRec.szVoidRedeemEndDate, 0x00, sizeof(srASMCRec.szVoidRedeemEndDate));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szVoidRedeemEndDate == NULL || strlen(szVoidRedeemEndDate) <= 0 || strlen(szVoidRedeemEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetVoidRedeemEndDate() ERROR !!");

                        if (szVoidRedeemEndDate == NULL)
                        {
                                inLogPrintf(AT, "szVoidRedeemEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szVoidRedeemEndDate length = (%d)", (int)strlen(szVoidRedeemEndDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srASMCRec.szVoidRedeemEndDate[0], &szVoidRedeemEndDate[0], strlen(szVoidRedeemEndDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetASMFlag
Date&Time       :
Describe        :
*/
int inGetASMFlag(char* szASMFlag)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szASMFlag == NULL || strlen(srASMCRec.szASMFlag) <= 0 || strlen(srASMCRec.szASMFlag) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetASMFlag() ERROR !!");

                        if (szASMFlag == NULL)
                        {
                                inLogPrintf(AT, "szASMFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szASMFlag length = (%d)", (int)strlen(szASMFlag));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szASMFlag[0], &srASMCRec.szASMFlag[0], strlen(srASMCRec.szASMFlag));

        return (VS_SUCCESS);
}

/*
Function        :inSetASMFlag
Date&Time       :
Describe        :
*/
int inSetASMFlag(char* szASMFlag)
{
        memset(srASMCRec.szASMFlag, 0x00, sizeof(srASMCRec.szASMFlag));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szASMFlag == NULL || strlen(szASMFlag) <= 0 || strlen(szASMFlag) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetASMFlag() ERROR !!");

                        if (szASMFlag == NULL)
                        {
                                inLogPrintf(AT, "szASMFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szASMFlag length = (%d)", (int)strlen(szASMFlag));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srASMCRec.szASMFlag[0], &szASMFlag[0], strlen(szASMFlag));

        return (VS_SUCCESS);
}

/*
Function        :inGetASMStartDate
Date&Time       :
Describe        :
*/
int inGetASMStartDate(char* szASMStartDate)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szASMStartDate == NULL || strlen(srASMCRec.szASMStartDate) <= 0 || strlen(srASMCRec.szASMStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetASMStartDate() ERROR !!");

                        if (szASMStartDate == NULL)
                        {
                                inLogPrintf(AT, "szASMStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szASMStartDate length = (%d)", (int)strlen(szASMStartDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szASMStartDate[0], &srASMCRec.szASMStartDate[0], strlen(srASMCRec.szASMStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetASMStartDate
Date&Time       :
Describe        :
*/
int inSetASMStartDate(char* szASMStartDate)
{
        memset(srASMCRec.szASMStartDate, 0x00, sizeof(srASMCRec.szASMStartDate));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szASMStartDate == NULL || strlen(szASMStartDate) <= 0 || strlen(szASMStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetASMStartDate() ERROR !!");

                        if (szASMStartDate == NULL)
                        {
                                inLogPrintf(AT, "szASMStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szASMStartDate length = (%d)", (int)strlen(szASMStartDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srASMCRec.szASMStartDate[0], &szASMStartDate[0], strlen(szASMStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetASMEndDate
Date&Time       :
Describe        :
*/
int inGetASMEndDate(char* szASMEndDate)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szASMEndDate == NULL || strlen(srASMCRec.szASMEndDate) <= 0 || strlen(srASMCRec.szASMEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetASMEndDate() ERROR !!");

                        if (szASMEndDate == NULL)
                        {
                                inLogPrintf(AT, "szASMEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szASMEndDate length = (%d)", (int)strlen(szASMEndDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szASMEndDate[0], &srASMCRec.szASMEndDate[0], strlen(srASMCRec.szASMEndDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetASMEndDate
Date&Time       :
Describe        :
*/
int inSetASMEndDate(char* szASMEndDate)
{
        memset(srASMCRec.szASMEndDate, 0x00, sizeof(srASMCRec.szASMEndDate));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szASMEndDate == NULL || strlen(szASMEndDate) <= 0 || strlen(szASMEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetASMEndDate() ERROR !!");

                        if (szASMEndDate == NULL)
                        {
                                inLogPrintf(AT, "szASMEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szASMEndDate length = (%d)", (int)strlen(szASMEndDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srASMCRec.szASMEndDate[0], &szASMEndDate[0], strlen(szASMEndDate));

        return (VS_SUCCESS);
}
