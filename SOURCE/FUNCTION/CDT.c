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
#include "CDT.h"

static  CDT_REC srCDTRec;	/* construct CDT record */
extern  int     ginDebug;       /* Debug使用 extern */

/*
Function        :inLoadCDTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀CDT檔案，inCDTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadCDTRec(int inCDTRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆CDT檔案 */
        char            szCDTRec[_SIZE_CDT_REC_ + 1];           /* 暫存, 放各個欄位檔案 */
        char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnCDTLength = 0;                        /* CDT總長度 */
        long            lnReadLength;                           /* 記錄每次要從CDT.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;	                /* inRec記錄讀到第幾筆, i為目前從CDT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */

        /* inLoadCDTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadCDTRec(%d) START!!", inCDTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 判斷傳進來的inCDTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inCDTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inCDTRec < 0:(index = %d) ERROR!!", inCDTRec);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open CDT.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_CDT_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnCDTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_CDT_FILE_NAME_);

        if (lnCDTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnCDTLength + 1);
        uszTemp = malloc(lnCDTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnCDTLength + 1);
        memset(uszTemp, 0x00, lnCDTLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnCDTLength;

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
         *i為目前從CDT讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnCDTLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到CDT的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnCDTLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        /* 因為inCDT_Rec的index從0開始，所以inCDT_Rec要+1 */
                        if (inRec == (inCDTRec + 1))
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
         * 如果沒有inCDTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inCDTRec + 1) || inSearchResult == -1)
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
        memset(&srCDTRec, 0x00, sizeof(srCDTRec));
        /*
         * 以下pattern為存入CDT_Rec
         * i為CDT的第幾個字元
         * 存入CDT_Rec
         */
        i = 0;


        /* 01_卡別索引 */
        /* 初始化 */
        memset(szCDTRec, 0x00, sizeof(szCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCDTRec[k ++] = uszReadData[i ++];
                if (szCDTRec[k - 1] == 0x2C	||
		    szCDTRec[k - 1] == 0x0D	||
		    szCDTRec[k - 1] == 0x0A	||
		    szCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "CDT unpack ERROR.");
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
        if (szCDTRec[0] != 0x2C	&&
	    szCDTRec[0] != 0x0D	&&
	    szCDTRec[0] != 0x0A	&&
	    szCDTRec[0] != 0x00)
        {
                memcpy(&srCDTRec.szCardIndex[0], &szCDTRec[0], k - 1);
        }

        /* 02_低卡號範圍 */
        /* 初始化 */
        memset(szCDTRec, 0x00, sizeof(szCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCDTRec[k ++] = uszReadData[i ++];
                if (szCDTRec[k - 1] == 0x2C	||
		    szCDTRec[k - 1] == 0x0D	||
		    szCDTRec[k - 1] == 0x0A	||
		    szCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "CDT unpack ERROR");
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
        if (szCDTRec[0] != 0x2C	&&
	    szCDTRec[0] != 0x0D	&&
	    szCDTRec[0] != 0x0A	&&
	    szCDTRec[0] != 0x00)
        {
                memcpy(&srCDTRec.szLowBinRange[0], &szCDTRec[0], k - 1);
        }

        /* 03_高卡號範圍 */
        /* 初始化 */
        memset(szCDTRec, 0x00, sizeof(szCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCDTRec[k ++] = uszReadData[i ++];
                if (szCDTRec[k - 1] == 0x2C	||
		    szCDTRec[k - 1] == 0x0D	||
		    szCDTRec[k - 1] == 0x0A	||
		    szCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "CDT unpack ERROR");
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
        if (szCDTRec[0] != 0x2C	&&
	    szCDTRec[0] != 0x0D	&&
	    szCDTRec[0] != 0x0A	&&
	    szCDTRec[0] != 0x00)
        {
                memcpy(&srCDTRec.szHighBinRange[0], &szCDTRec[0], k - 1);
        }

        /* 04_對應交易主機索引 */
        /* 初始化 */
        memset(szCDTRec, 0x00, sizeof(szCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCDTRec[k ++] = uszReadData[i ++];
                if (szCDTRec[k - 1] == 0x2C	||
		    szCDTRec[k - 1] == 0x0D	||
		    szCDTRec[k - 1] == 0x0A	||
		    szCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "CDT unpack ERROR");
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
        if (szCDTRec[0] != 0x2C	&&
	    szCDTRec[0] != 0x0D	&&
	    szCDTRec[0] != 0x0A	&&
	    szCDTRec[0] != 0x00)
        {
                memcpy(&srCDTRec.szHostCDTIndex[0], &szCDTRec[0], k - 1);
        }

        /* 05_檢查最短卡號長度 */
        /* 初始化 */
        memset(szCDTRec, 0x00, sizeof(szCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCDTRec[k ++] = uszReadData[i ++];
                if (szCDTRec[k - 1] == 0x2C	||
		    szCDTRec[k - 1] == 0x0D	||
		    szCDTRec[k - 1] == 0x0A	||
		    szCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "CDT unpack ERROR");
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
        if (szCDTRec[0] != 0x2C	&&
	    szCDTRec[0] != 0x0D	&&
	    szCDTRec[0] != 0x0A	&&
	    szCDTRec[0] != 0x00)
        {
                memcpy(&srCDTRec.szMinPANLength[0], &szCDTRec[0], k - 1);
        }

        /* 06_檢查最長卡號長度 */
        /* 初始化 */
        memset(szCDTRec, 0x00, sizeof(szCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCDTRec[k ++] = uszReadData[i ++];
                if (szCDTRec[k - 1] == 0x2C	||
		    szCDTRec[k - 1] == 0x0D	||
		    szCDTRec[k - 1] == 0x0A	||
		    szCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "CDT unpack ERROR");
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
        if (szCDTRec[0] != 0x2C	&&
	    szCDTRec[0] != 0x0D	&&
	    szCDTRec[0] != 0x0A	&&
	    szCDTRec[0] != 0x00)
        {
                memcpy(&srCDTRec.szMaxPANLength[0], &szCDTRec[0], k - 1);
        }

        /* 07_檢查碼查核(U CARD以11碼卡號，依U CARD檢查碼邏輯進行查核) */
        /* 初始化 */
        memset(szCDTRec, 0x00, sizeof(szCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCDTRec[k ++] = uszReadData[i ++];
                if (szCDTRec[k - 1] == 0x2C	||
		    szCDTRec[k - 1] == 0x0D	||
		    szCDTRec[k - 1] == 0x0A	||
		    szCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "CDT unpack ERROR");
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
        if (szCDTRec[0] != 0x2C	&&
	    szCDTRec[0] != 0x0D	&&
	    szCDTRec[0] != 0x0A	&&
	    szCDTRec[0] != 0x00)
        {
                memcpy(&srCDTRec.szModule10Check[0], &szCDTRec[0], k - 1);
        }

        /* 08_有效期查核 */
        /* 初始化 */
        memset(szCDTRec, 0x00, sizeof(szCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCDTRec[k ++] = uszReadData[i ++];
                if (szCDTRec[k - 1] == 0x2C	||
		    szCDTRec[k - 1] == 0x0D	||
		    szCDTRec[k - 1] == 0x0A	||
		    szCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "CDT unpack ERROR");
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
        if (szCDTRec[0] != 0x2C	&&
	    szCDTRec[0] != 0x0D	&&
	    szCDTRec[0] != 0x0A	&&
	    szCDTRec[0] != 0x00)
        {
                memcpy(&srCDTRec.szExpiredDateCheck[0], &szCDTRec[0], k - 1);
        }

        /* 09_輸入AMEX 4DBC或MASTER/VISA CVV2。 */
        /* 初始化 */
        memset(szCDTRec, 0x00, sizeof(szCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCDTRec[k ++] = uszReadData[i ++];
                if (szCDTRec[k - 1] == 0x2C	||
		    szCDTRec[k - 1] == 0x0D	||
		    szCDTRec[k - 1] == 0x0A	||
		    szCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "CDT unpack ERROR");
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
        if (szCDTRec[0] != 0x2C	&&
	    szCDTRec[0] != 0x0D	&&
	    szCDTRec[0] != 0x0A	&&
	    szCDTRec[0] != 0x00)
        {
                memcpy(&srCDTRec.sz4DBCEnable[0], &szCDTRec[0], k - 1);
        }

	/* 10_卡別名稱(VISA, MASTERCARD, JCB, U CARD, AMEX, DINERS) */
        /* 初始化 */
        memset(szCDTRec, 0x00, sizeof(szCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCDTRec[k ++] = uszReadData[i ++];
                if (szCDTRec[k - 1] == 0x2C	||
		    szCDTRec[k - 1] == 0x0D	||
		    szCDTRec[k - 1] == 0x0A	||
		    szCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "CDT unpack ERROR");
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
        if (szCDTRec[0] != 0x2C	&&
	    szCDTRec[0] != 0x0D	&&
	    szCDTRec[0] != 0x0A	&&
	    szCDTRec[0] != 0x00)
        {
                memcpy(&srCDTRec.szCardLabel[0], &szCDTRec[0], k - 1);
        }
	
        /* 11_列印交易編號及遮掩商店存根聯之卡號 */
        /* 初始化 */
        memset(szCDTRec, 0x00, sizeof(szCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCDTRec[k ++] = uszReadData[i ++];
                if (szCDTRec[k - 1] == 0x2C	||
		    szCDTRec[k - 1] == 0x0D	||
		    szCDTRec[k - 1] == 0x0A	||
		    szCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "CDT unpack ERROR");
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
        if (szCDTRec[0] != 0x2C	&&
	    szCDTRec[0] != 0x0D	&&
	    szCDTRec[0] != 0x0A	&&
	    szCDTRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srCDTRec.szPrint_Tx_No_Check_No[0], &szCDTRec[0], k - 1);
        }

        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadCDTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadCDTRec(%d) END!!", inCDTRec);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveCDTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSaveCDTRec(int inCDTRec)
{
        unsigned long   uldat_Handle;                           /* FILE Handle */
        unsigned long   ulbak_Handle;                           /* FILE Handle */
        int	        inPackCount = 0;                        /* uszWriteBuff_Record的index */
        int	        inRecIndex = 0;                         /* uszRead_Total_Buff的index */
        int             inCDT_Total_Rec = 0;                    /* CDT.dat的總筆數 */
        int             inRetVal;
        int             i = 0, j = 0;
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long	        lnReadLength = 0;                       /* 每次要從CDT.dat讀多少byte出來 */
        long            lnCDTLength = 0;                        /* CDT.dat檔案總長度 */
        unsigned char	*uszRead_Total_Buff;
        unsigned char   *uszWriteBuff_Record, *uszWriteBuff_Org;


        /* inSaveCDTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveCDTRec(%d)_START!!", inCDTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 防呆先刪除CDT.bak */
        inFILE_Delete((unsigned char *)_CDT_FILE_NAME_BAK_);

        /* 新建CDT.bak檔案 */
        inRetVal = inFILE_Create(&ulbak_Handle, (unsigned char *)_CDT_FILE_NAME_BAK_);

        if (inRetVal != VS_SUCCESS)
        {
                inFILE_Close(&ulbak_Handle);

                return (VS_ERROR);
        }

        /* 開啟原檔案CDT.dat */
        inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_CDT_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                /* CDT.dat開檔失敗 ，不用關檔CDT.dat */
                /* CDT.bak仍要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_CDT_FILE_NAME_BAK_);

                return(VS_ERROR);
        }

        /* 取得CDT.dat檔案大小 */
        lnCDTLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *)_CDT_FILE_NAME_);

        /* 取得檔案大小失敗 */
        if (lnCDTLength == VS_ERROR)
        {
                /* CDT.bak和CDT.dat要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Close(&uldat_Handle);
                inFILE_Delete((unsigned char *)_CDT_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        uszWriteBuff_Record = malloc(_SIZE_CDT_REC_ + _SIZE_CDT_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_CDT_REC_ + _SIZE_CDT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原CDT.dat的每筆Record 初始化 */
        uszWriteBuff_Org = malloc(_SIZE_CDT_REC_ + _SIZE_CDT_COMMA_0D0A_);
        memset(uszWriteBuff_Org, 0x00, _SIZE_CDT_REC_ + _SIZE_CDT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存CDT.dat全部資料 */
        uszRead_Total_Buff = malloc(lnCDTLength + 1);
        memset(uszRead_Total_Buff, 0x00, lnCDTLength + 1);

        inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

        /* CardIndex */
        memcpy(&uszWriteBuff_Record[0], &srCDTRec.szCardIndex[0], strlen(srCDTRec.szCardIndex));
        inPackCount += strlen(srCDTRec.szCardIndex);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* LowBinRange */
        memcpy(&uszWriteBuff_Record[inPackCount], &srCDTRec.szLowBinRange[0], strlen(srCDTRec.szLowBinRange));
        inPackCount += strlen(srCDTRec.szLowBinRange);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* HighBinRange */
        memcpy(&uszWriteBuff_Record[inPackCount], &srCDTRec.szHighBinRange[0], strlen(srCDTRec.szHighBinRange));
        inPackCount += strlen(srCDTRec.szHighBinRange);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* HostCDTIndex */
        memcpy(&uszWriteBuff_Record[inPackCount], &srCDTRec.szHostCDTIndex[0], strlen(srCDTRec.szHostCDTIndex));
        inPackCount += strlen(srCDTRec.szHostCDTIndex);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* MinPANLength */
        memcpy(&uszWriteBuff_Record[inPackCount], &srCDTRec.szMinPANLength[0], strlen(srCDTRec.szMinPANLength));
        inPackCount += strlen(srCDTRec.szMinPANLength);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* MaxPANLength */
        memcpy(&uszWriteBuff_Record[inPackCount], &srCDTRec.szMaxPANLength[0], strlen(srCDTRec.szMaxPANLength));
        inPackCount += strlen(srCDTRec.szMaxPANLength);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* Module10Check */
        memcpy(&uszWriteBuff_Record[inPackCount], &srCDTRec.szModule10Check[0], strlen(srCDTRec.szModule10Check));
        inPackCount += strlen(srCDTRec.szModule10Check);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* ExpiredDateCheck */
        memcpy(&uszWriteBuff_Record[inPackCount], &srCDTRec.szExpiredDateCheck[0], strlen(srCDTRec.szExpiredDateCheck));
        inPackCount += strlen(srCDTRec.szExpiredDateCheck);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* 4DBCEnable */
        memcpy(&uszWriteBuff_Record[inPackCount], &srCDTRec.sz4DBCEnable[0], strlen(srCDTRec.sz4DBCEnable));
        inPackCount += strlen(srCDTRec.sz4DBCEnable);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* CardLabel */
        memcpy(&uszWriteBuff_Record[inPackCount], &srCDTRec.szCardLabel[0], strlen(srCDTRec.szCardLabel));
        inPackCount += strlen(srCDTRec.szCardLabel);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
        /* Print_Tx_No_Check_No */
        memcpy(&uszWriteBuff_Record[inPackCount], &srCDTRec.szPrint_Tx_No_Check_No[0], strlen(srCDTRec.szPrint_Tx_No_Check_No));
        inPackCount += strlen(srCDTRec.szPrint_Tx_No_Check_No);

        /* 最後的data不用逗號 */
        /* 補上換行符號 */

        /* 0D是移至行首 */
        uszWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        uszWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;

        /* 讀CDT.dat檔案存到uszRead_Total_Buff */
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnCDTLength;

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
                                        inFILE_Delete((unsigned char *)_CDT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_CDT_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_CDT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        /* 防呆 總record數量小於要存取inCDTRec Return ERROR */
        /* 算總Record數 */
        for (i = 0; i < (lnCDTLength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (uszRead_Total_Buff[i] == 0x0A)
                        inCDT_Total_Rec++;
        }

//        /* 防呆 總record數量小於要存取inCDTRec Return ERROR */
//        if ((inCDTRec + 1) > inCDT_Total_Rec)
//        {
//                if (ginDebug == VS_TRUE)
//                {
//                        inLogPrintf(AT, "No data or Index ERROR");
//                }
//
//                /* 關檔 */
//                inFILE_Close(&uldat_Handle);
//                inFILE_Close(&ulbak_Handle);
//                inFILE_Delete((unsigned char *)_CDT_FILE_NAME_BAK_);
//
//                /* Free pointer */
//                free(uszRead_Total_Buff);
//                free(uszWriteBuff_Record);
//                free(uszWriteBuff_Org);
//
//                return (VS_ERROR);
//        }

        /* 記錄ReadTotalBuffer的陣列位置 */
        inRecIndex = 0;
        /* 用inCDTRec決定要先存幾筆Record到CDT.bak，ex:inCDTRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inCDTRec; i ++)
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
                                        inFILE_Delete((unsigned char *)_CDT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_CDT_FILE_NAME_BAK_);

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

        /* 存組好的該CDTRecord 到 CDT.bak */
        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_CDT_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_CDT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原CDT.dat Record 到 CDT.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
        /* inRecIndex要指到剩下還沒存進bak檔的第一個記憶體位置 */
        inCDTRec = inCDTRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < lnCDTLength; i++)
        {
                if (uszRead_Total_Buff[i] == 0x0A)
                {
                        j++;

                        if (j == inCDTRec)
                        {
                                break;
                        }
                }
        }
	inRecIndex = i + 1;

        i = 0;
        memset(uszWriteBuff_Org, 0x00, sizeof(uszWriteBuff_Org));

        while (1)
        {
                /* 接續存原CDT.dat的Record */
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
                                inFILE_Delete((unsigned char *)_CDT_FILE_NAME_BAK_);

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
                                inFILE_Delete((unsigned char *)_CDT_FILE_NAME_BAK_);

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

        /* 刪除原CDT.dat */
        if (inFILE_Delete((unsigned char *)_CDT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* 將CDT.bak改名字為CDT.dat取代原檔案 */
        if (inFILE_Rename((unsigned char *)_CDT_FILE_NAME_BAK_, (unsigned char *)_CDT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* inSaveCDTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveCDTRec(%d) END!!", inCDTRec - 1);
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
Function        :inGetCardIndex
Date&Time       :
Describe        :
*/
int inGetCardIndex(char* szCardIndex)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szCardIndex == NULL || strlen(srCDTRec.szCardIndex) <= 0 || strlen(srCDTRec.szCardIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetCardIndex() ERROR !!");

			if (szCardIndex == NULL)
                        {
                                inLogPrintf(AT, "szCardIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCardIndex length = (%d)", (int)strlen(srCDTRec.szCardIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCardIndex[0], &srCDTRec.szCardIndex[0], strlen(srCDTRec.szCardIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetCardIndex
Date&Time       :
Describe        :
*/
int inSetCardIndex(char* szCardIndex)
{
        memset(srCDTRec.szCardIndex, 0x00, sizeof(srCDTRec.szCardIndex));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szCardIndex == NULL || strlen(szCardIndex) <= 0 || strlen(szCardIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCardIndex() ERROR !!");
                        if (szCardIndex == NULL)
                        {
                                inLogPrintf(AT, "szCardIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCardIndex length = (%d)", (int)strlen(szCardIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCDTRec.szCardIndex[0], &szCardIndex[0], strlen(szCardIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetLowBinRange
Date&Time       :
Describe        :
*/
int inGetLowBinRange(char* szLowBinRange)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szLowBinRange == NULL || strlen(srCDTRec.szLowBinRange) <= 0 || strlen(srCDTRec.szLowBinRange) > 11)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetLowBinRange() ERROR !!");

                        if (szLowBinRange == NULL)
                        {
                                inLogPrintf(AT, "szLowBinRange == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szLowBinRange length = (%d)", (int)strlen(srCDTRec.szLowBinRange));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szLowBinRange[0], &srCDTRec.szLowBinRange[0], strlen(srCDTRec.szLowBinRange));

        return (VS_SUCCESS);
}

/*
Function        :inSetLowBinRange
Date&Time       :
Describe        :
*/
int inSetLowBinRange(char* szLowBinRange)
{
        memset(srCDTRec.szLowBinRange, 0x00, sizeof(srCDTRec.szLowBinRange));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szLowBinRange == NULL || strlen(szLowBinRange) <= 0 || strlen(szLowBinRange) > 11)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetLowBinRange() ERROR !!");

                        if (szLowBinRange == NULL)
                        {
                                inLogPrintf(AT, "szLowBinRange == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szLowBinRange length = (%d)", (int)strlen(szLowBinRange));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCDTRec.szLowBinRange[0], &szLowBinRange[0], strlen(szLowBinRange));

        return (VS_SUCCESS);
}

/*
Function        :inGetHighBinRange
Date&Time       :
Describe        :
*/
int inGetHighBinRange(char* szHighBinRange)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szHighBinRange == NULL || strlen(srCDTRec.szHighBinRange) <= 0 || strlen(srCDTRec.szHighBinRange) > 11)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetHighBinRange() ERROR !!");

                        if (szHighBinRange == NULL)
                        {
                                inLogPrintf(AT, "szHighBinRange == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szHighBinRange length = (%d)", (int)strlen(srCDTRec.szHighBinRange));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szHighBinRange[0], &srCDTRec.szHighBinRange[0], strlen(srCDTRec.szHighBinRange));

        return (VS_SUCCESS);
}

/*
Function        :inSetHighBinRange
Date&Time       :
Describe        :
*/
int inSetHighBinRange(char* szHighBinRange)
{
        memset(srCDTRec.szHighBinRange, 0x00, sizeof(srCDTRec.szHighBinRange));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szHighBinRange == NULL || strlen(szHighBinRange) <= 0 || strlen(szHighBinRange) > 11)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetHighBinRange() ERROR !!");

                        if (szHighBinRange == NULL)
                        {
                                inLogPrintf(AT, "szHighBinRange == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szHighBinRange length = (%d)", (int)strlen(szHighBinRange));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCDTRec.szHighBinRange[0], &szHighBinRange[0], strlen(szHighBinRange));

        return (VS_SUCCESS);
}

/*
Function        :inGetHostCDTIndex
Date&Time       :
Describe        :
*/
int inGetHostCDTIndex(char* szHostCDTIndex)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szHostCDTIndex == NULL || strlen(srCDTRec.szHostCDTIndex) <= 0 || strlen(srCDTRec.szHostCDTIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetHostCDTIndex() ERROR !!");

                        if (szHostCDTIndex == NULL)
                        {
                                inLogPrintf(AT, "szHostCDTIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szHostCDTIndex length = (%d)", (int)strlen(srCDTRec.szHostCDTIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szHostCDTIndex[0], &srCDTRec.szHostCDTIndex[0], strlen(srCDTRec.szHostCDTIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetHostCDTIndex
Date&Time       :
Describe        :
*/
int inSetHostCDTIndex(char* szHostCDTIndex)
{
        memset(srCDTRec.szHostCDTIndex, 0x00, sizeof(srCDTRec.szHostCDTIndex));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szHostCDTIndex == NULL || strlen(szHostCDTIndex) <= 0 || strlen(szHostCDTIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetHostCDTIndex() ERROR !!");

                        if (szHostCDTIndex == NULL)
                        {
                                inLogPrintf(AT, "szHostCDTIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szHostCDTIndex length = (%d)", (int)strlen(szHostCDTIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCDTRec.szHostCDTIndex[0], &szHostCDTIndex[0], strlen(szHostCDTIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetMinPANLength
Date&Time       :
Describe        :
*/
int inGetMinPANLength(char* szMinPANLength)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szMinPANLength == NULL || strlen(srCDTRec.szMinPANLength) <= 0 || strlen(srCDTRec.szMinPANLength) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetMinPANLength() ERROR !!");

                        if (szMinPANLength == NULL)
                        {
                                inLogPrintf(AT, "szMinPANLength == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMinPANLength length = (%d)", (int)strlen(srCDTRec.szMinPANLength));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMinPANLength[0], &srCDTRec.szMinPANLength[0], strlen(srCDTRec.szMinPANLength));

        return (VS_SUCCESS);
}

/*
Function        :inSetMinPANLength
Date&Time       :
Describe        :
*/
int inSetMinPANLength(char* szMinPANLength)
{
        memset(srCDTRec.szMinPANLength, 0x00, sizeof(srCDTRec.szMinPANLength));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szMinPANLength == NULL || strlen(szMinPANLength) <= 0 || strlen(szMinPANLength) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetMinPANLength() ERROR !!");

                        if (szMinPANLength == NULL)
                        {
                                inLogPrintf(AT, "szMinPANLength == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMinPANLength length = (%d)", (int)strlen(szMinPANLength));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCDTRec.szMinPANLength[0], &szMinPANLength[0], strlen(szMinPANLength));

        return (VS_SUCCESS);
}

/*
Function        :inGetMaxPANLength
Date&Time       :
Describe        :
*/
int inGetMaxPANLength(char* szMaxPANLength)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szMaxPANLength == NULL || strlen(srCDTRec.szMaxPANLength) <= 0 || strlen(srCDTRec.szMaxPANLength) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetMaxPANLength() ERROR !!");

                        if (szMaxPANLength == NULL)
                        {
                                inLogPrintf(AT, "szMaxPANLength == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMaxPANLength length = (%d)", (int)strlen(srCDTRec.szMaxPANLength));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMaxPANLength[0], &srCDTRec.szMaxPANLength[0], strlen(srCDTRec.szMaxPANLength));

        return (VS_SUCCESS);
}

/*
Function        :inSetMaxPANLength
Date&Time       :
Describe        :
*/
int inSetMaxPANLength(char* szMaxPANLength)
{
        memset(srCDTRec.szMaxPANLength, 0x00, sizeof(srCDTRec.szMaxPANLength));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szMaxPANLength == NULL || strlen(szMaxPANLength) <= 0 || strlen(szMaxPANLength) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetMaxPANLength() ERROR !!");

                        if (szMaxPANLength == NULL)
                        {
                                inLogPrintf(AT, "szMaxPANLength == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMaxPANLength length = (%d)", (int)strlen(szMaxPANLength));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCDTRec.szMaxPANLength[0], &szMaxPANLength[0], strlen(szMaxPANLength));

        return (VS_SUCCESS);
}

/*
Function        :inGetModule10Check
Date&Time       :
Describe        :
*/
int inGetModule10Check(char* szModule10Check)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szModule10Check == NULL || strlen(srCDTRec.szModule10Check) <= 0 || strlen(srCDTRec.szModule10Check) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetModule10Check() ERROR !!");

                        if (szModule10Check == NULL)
                        {
                                inLogPrintf(AT, "szModule10Check == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szModule10Check length = (%d)", (int)strlen(srCDTRec.szModule10Check));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szModule10Check[0], &srCDTRec.szModule10Check[0], strlen(srCDTRec.szModule10Check));

        return (VS_SUCCESS);
}

/*
Function        :inSetModule10Check
Date&Time       :
Describe        :
*/
int inSetModule10Check(char* szModule10Check)
{
        memset(srCDTRec.szModule10Check, 0x00, sizeof(srCDTRec.szModule10Check));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szModule10Check == NULL || strlen(szModule10Check) <= 0 || strlen(szModule10Check) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetModule10Check() ERROR !!");

                        if (szModule10Check == NULL)
                        {
                                inLogPrintf(AT, "szModule10Check == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szModule10Check length = (%d)", (int)strlen(szModule10Check));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCDTRec.szModule10Check[0], &szModule10Check[0], strlen(szModule10Check));

        return (VS_SUCCESS);
}

/*
Function        :inGetExpiredDateCheck
Date&Time       :
Describe        :
*/
int inGetExpiredDateCheck(char* szExpiredDateCheck)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szExpiredDateCheck == NULL || strlen(srCDTRec.szExpiredDateCheck) <= 0 || strlen(srCDTRec.szExpiredDateCheck) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetExpiredDateCheck() ERROR !!");

                        if (szExpiredDateCheck == NULL)
                        {
                                inLogPrintf(AT, "szExpiredDateCheck == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szExpiredDateCheck length = (%d)", (int)strlen(srCDTRec.szExpiredDateCheck));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szExpiredDateCheck[0], &srCDTRec.szExpiredDateCheck[0], strlen(srCDTRec.szExpiredDateCheck));

        return (VS_SUCCESS);
}

/*
Function        :inSetExpiredDateCheck
Date&Time       :
Describe        :
*/
int inSetExpiredDateCheck(char* szExpiredDateCheck)
{
        memset(srCDTRec.szExpiredDateCheck, 0x00, sizeof(srCDTRec.szExpiredDateCheck));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szExpiredDateCheck == NULL || strlen(szExpiredDateCheck) <= 0 || strlen(szExpiredDateCheck) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetExpiredDateCheck() ERROR !!");

                        if (szExpiredDateCheck == NULL)
                        {
                                inLogPrintf(AT, "szExpiredDateCheck == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szExpiredDateCheck length = (%d)", (int)strlen(szExpiredDateCheck));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCDTRec.szExpiredDateCheck[0], &szExpiredDateCheck[0], strlen(szExpiredDateCheck));

        return (VS_SUCCESS);
}

/*
Function        :inGet4DBCEnable
Date&Time       :
Describe        :
*/
int inGet4DBCEnable(char* sz4DBCEnable)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (sz4DBCEnable == NULL || strlen(srCDTRec.sz4DBCEnable) <= 0 || strlen(srCDTRec.sz4DBCEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGet4DBCEnable() ERROR !!");

                        if (sz4DBCEnable == NULL)
                        {
                                inLogPrintf(AT, "sz4DBCEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "sz4DBCEnable length = (%d)", (int)strlen(srCDTRec.sz4DBCEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&sz4DBCEnable[0], &srCDTRec.sz4DBCEnable[0], strlen(srCDTRec.sz4DBCEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetCVV4DBCEnable
Date&Time       :
Describe        :
*/
int inSet4DBCEnable(char* sz4DBCEnable)
{
        memset(srCDTRec.sz4DBCEnable, 0x00, sizeof(srCDTRec.sz4DBCEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (sz4DBCEnable == NULL || strlen(sz4DBCEnable) <= 0 || strlen(sz4DBCEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSet4DBCEnable() ERROR !!");

                        if (sz4DBCEnable == NULL)
                        {
                                inLogPrintf(AT, "sz4DBCEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "sz4DBCEnable length = (%d)", (int)strlen(sz4DBCEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCDTRec.sz4DBCEnable[0], &sz4DBCEnable[0], strlen(sz4DBCEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetCardLabel
Date&Time       :2016/11/25 下午 1:21
Describe        :卡別
*/
int inGetCardLabel(char* szCardLabel)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szCardLabel == NULL || strlen(srCDTRec.szCardLabel) <= 0 || strlen(srCDTRec.szCardLabel) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetCardLabel() ERROR !!");

                        if (szCardLabel == NULL)
                        {
                                inLogPrintf(AT, "szCardLabel == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCardLabel length = (%d)", (int)strlen(srCDTRec.szCardLabel));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCardLabel[0], &srCDTRec.szCardLabel[0], strlen(srCDTRec.szCardLabel));

        return (VS_SUCCESS);
}

/*
Function        :inSetCardLabel
Date&Time       :2016/11/25 下午 1:22
Describe        :卡別
*/
int inSetCardLabel(char* szCardLabel)
{
        memset(srCDTRec.szCardLabel, 0x00, sizeof(srCDTRec.szCardLabel));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCardLabel == NULL || strlen(szCardLabel) <= 0 || strlen(szCardLabel) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCardLabel() ERROR !!");

                        if (szCardLabel == NULL)
                        {
                                inLogPrintf(AT, "szCardLabel == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCardLabel length = (%d)", (int)strlen(szCardLabel));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCDTRec.szCardLabel[0], &szCardLabel[0], strlen(szCardLabel));

        return (VS_SUCCESS);
}

/*
Function        :inGetPrint_Tx_No_Check_No
Date&Time       :2017/5/15 下午 4:02
Describe        :卡別
*/
int inGetPrint_Tx_No_Check_No(char* szPrint_Tx_No_Check_No)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szPrint_Tx_No_Check_No == NULL || strlen(srCDTRec.szPrint_Tx_No_Check_No) <= 0 || strlen(srCDTRec.szPrint_Tx_No_Check_No) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPrint_Tx_No_Check_No() ERROR !!");

                        if (szPrint_Tx_No_Check_No == NULL)
                        {
                                inLogPrintf(AT, "szPrint_Tx_No_Check_No == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrint_Tx_No_Check_No length = (%d)", (int)strlen(srCDTRec.szPrint_Tx_No_Check_No));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPrint_Tx_No_Check_No[0], &srCDTRec.szPrint_Tx_No_Check_No[0], strlen(srCDTRec.szPrint_Tx_No_Check_No));

        return (VS_SUCCESS);
}

/*
Function        :inSetPrint_Tx_No_Check_No
Date&Time       :2017/5/15 下午 4:02
Describe        :卡別
*/
int inSetPrint_Tx_No_Check_No(char* szPrint_Tx_No_Check_No)
{
        memset(srCDTRec.szPrint_Tx_No_Check_No, 0x00, sizeof(srCDTRec.szPrint_Tx_No_Check_No));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPrint_Tx_No_Check_No == NULL || strlen(szPrint_Tx_No_Check_No) <= 0 || strlen(szPrint_Tx_No_Check_No) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPrint_Tx_No_Check_No() ERROR !!");

                        if (szPrint_Tx_No_Check_No == NULL)
                        {
                                inLogPrintf(AT, "szPrint_Tx_No_Check_No == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrint_Tx_No_Check_No length = (%d)", (int)strlen(szPrint_Tx_No_Check_No));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCDTRec.szPrint_Tx_No_Check_No[0], &szPrint_Tx_No_Check_No[0], strlen(szPrint_Tx_No_Check_No));

        return (VS_SUCCESS);
}

/*
Function        :inCDT_Edit_CDT_Table
Date&Time       :2017/5/15 下午 4:08
Describe        :
*/
int inCDT_Edit_CDT_Table(void)
{
	TABLE_GET_SET_TABLE CDT_FUNC_TABLE[] =
	{
		{"szCardIndex"			,inGetCardIndex			,inSetCardIndex			},
		{"szLowBinRange"		,inGetLowBinRange		,inSetLowBinRange		},
		{"szHighBinRange"		,inGetHighBinRange		,inSetHighBinRange		},
		{"szHostCDTIndex"		,inGetHostCDTIndex		,inSetHostCDTIndex		},
		{"szMinPANLength"		,inGetMinPANLength		,inSetMinPANLength		},
		{"szMaxPANLength"		,inGetMaxPANLength		,inSetMaxPANLength		},
		{"szModule10Check"		,inGetModule10Check		,inSetModule10Check		},
		{"szExpiredDateCheck"		,inGetExpiredDateCheck		,inSetExpiredDateCheck		},
		{"sz4DBCEnable"			,inGet4DBCEnable		,inSet4DBCEnable		},
		{"szCardLabel"			,inGetCardLabel			,inSetCardLabel			},
		{"szPrint_Tx_No_Check_No"	,inGetPrint_Tx_No_Check_No	,inSetPrint_Tx_No_Check_No	},
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
	inDISP_ChineseFont_Color("是否更改CDT", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
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
	inLoadCDTRec(inRecordCnt);
	
	inFunc_Edit_Table_Tag(CDT_FUNC_TABLE);
	inSaveCDTRec(inRecordCnt);
	
	return	(VS_SUCCESS);
}

int inCDT_Test (void)
{
	char    CDT_Data[1024];
	//int     i = 0;
	memset(CDT_Data, 0x00, sizeof(CDT_Data));

	inLoadCDTRec(0);
	inSetCardIndex("01");
	inSetLowBinRange("4000000000");
	inSetHighBinRange("4000999999");
	inSetHostCDTIndex("01");
	inSetMinPANLength("11");
	inSetMaxPANLength("16");
	inSetModule10Check("Y");
	inSetExpiredDateCheck("Y");
	inSet4DBCEnable("N");
	inSetCardLabel("U CARD");
	inSaveCDTRec(0);


	inLoadCDTRec(1);
	inSetCardIndex("02");
	inSetLowBinRange("4000000000");
	inSetHighBinRange("4999999999");
	inSetHostCDTIndex("01");
	inSetMinPANLength("01");
	inSetMaxPANLength("19");
	inSetModule10Check("Y");
	inSetExpiredDateCheck("Y");
	inSet4DBCEnable("N");
	inSetCardLabel("VISA");
	inSaveCDTRec(1);

	inLoadCDTRec(2);
	inSetCardIndex("03");
	inSetLowBinRange("5100000000");
	inSetHighBinRange("5599999999");
	inSetHostCDTIndex("01");
	inSetMinPANLength("13");
	inSetMaxPANLength("19");
	inSetModule10Check("Y");
	inSetExpiredDateCheck("Y");
	inSet4DBCEnable("N");
	inSetCardLabel("VISA");
	inSaveCDTRec(2);

	inLoadCDTRec(3);
	inSetCardIndex("04");
	inSetLowBinRange("3528000000");
	inSetHighBinRange("3589999999");
	inSetHostCDTIndex("01");
	inSetMinPANLength("13");
	inSetMaxPANLength("19");
	inSetModule10Check("Y");
	inSetExpiredDateCheck("Y");
	inSet4DBCEnable("N");
	inSetCardLabel("JCB");
	inSaveCDTRec(3);

	inLoadCDTRec(4);
	inSetCardIndex("05");
	inSetLowBinRange("1040000000");
	inSetHighBinRange("1040999999");
	inSetHostCDTIndex("02");
	inSetMinPANLength("04");
	inSetMaxPANLength("14");
	inSetModule10Check("Y");
	inSetExpiredDateCheck("Y");
	inSet4DBCEnable("N");
	inSetCardLabel("DINERS");
	inSaveCDTRec(4);

	inLoadCDTRec(5);
	inSetCardIndex("06");
	inSetLowBinRange("3400000000");
	inSetHighBinRange("3499999999");
	inSetHostCDTIndex("01");
	inSetMinPANLength("15");
	inSetMaxPANLength("15");
	inSetModule10Check("Y");
	inSetExpiredDateCheck("Y");
	inSet4DBCEnable("Y");
	inSetCardLabel("AMEX");
	inSaveCDTRec(5);

	inLoadCDTRec(6);
	inSetCardIndex("07");
	inSetLowBinRange("3700000000");
	inSetHighBinRange("3799999999");
	inSetHostCDTIndex("01");
	inSetMinPANLength("15");
	inSetMaxPANLength("15");
	inSetModule10Check("Y");
	inSetExpiredDateCheck("Y");
	inSet4DBCEnable("Y");
	inSetCardLabel("AMEX");
	inSaveCDTRec(6);

	inLoadCDTRec(7);
	inSetCardIndex("08");
	inSetLowBinRange("9999999999");
	inSetHighBinRange("9999999999");
	inSetHostCDTIndex("01");
	inSetMinPANLength("01");
	inSetMaxPANLength("19");
	inSetModule10Check("Y");
	inSetExpiredDateCheck("Y");
	inSet4DBCEnable("N");
	inSetCardLabel("VISA");
	inSaveCDTRec(6);


	return 0;
}
