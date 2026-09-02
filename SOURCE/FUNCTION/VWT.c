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
#include "VWT.h"

static  VWT_REC srVWTRec;	/* construct VWT record */
extern  int	ginDebug;	/* Debug使用 extern */

/*
Function        :inLoadVWTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀VWT檔案，inVWTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadVWTRec(int inVWTRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆VWT檔案 */
        char            szVWTRec[_SIZE_VWT_REC_ + 1];           /* 暫存, 放各個欄位檔案 */
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnVWTLength = 0;                        /* VWT總長度 */
        long            lnReadLength;                           /* 記錄每次要從VWT.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;                 /* inRec記錄讀到第幾筆, i為目前從VWT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */

        /* inLoadVWTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadVWTRec(%d) START!!", inVWTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 判斷傳進來的inVWTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inVWTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inVWTRec < 0:(index = %d) ERROR!!", inVWTRec);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open VWT.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_VWT_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnVWTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_VWT_FILE_NAME_);

	if (lnVWTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnVWTLength + 1);
        uszTemp = malloc(lnVWTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnVWTLength + 1);
        memset(uszTemp, 0x00, lnVWTLength + 1);

         /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnVWTLength;

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
         *i為目前從VWT讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnVWTLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到VWT的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                      	/* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
			inSearchResult = 1;

			/* 清空uszReadData */
                        memset(uszReadData, 0x00, lnVWTLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
			memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
			/* 因為inVWT_Rec的index從0開始，所以inVWT_Rec要+1 */
                        if (inRec == (inVWTRec + 1))
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
         * 如果沒有inVWTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inVWTRec + 1) || inSearchResult == -1)
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
        memset(&srVWTRec, 0x00, sizeof(srVWTRec));
	/*
         * 以下pattern為存入VWT_Rec
         * i為VWT的第幾個字元
         * 存入VWT_Rec
         */
        i = 0;


        /* 01_各非接觸式卡片索引 */
	/* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
	k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szContactlessSchemeIndex[0], &szVWTRec[0], k - 1);
        }

        /* 02_Terminal Type */
	/* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szVWTTerminalType[0], &szVWTRec[0], k - 1);
        }

        /* 03_CVM Requirement */
	/* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szCVMRequirement[0], &szVWTRec[0], k - 1);
        }

        /* 04_VLP Support Indicator */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szVLPSupportIndicator[0], &szVWTRec[0], k - 1);
        }

        /* 05_Terminal Country Code */
	/* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szTerminalCountryCode[0], &szVWTRec[0], k - 1);
        }

        /* 06_Transaction Currency Code */
	/* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szVWTTransactionCurrencyCode[0], &szVWTRec[0], k - 1);
        }

        /* 07_Transaction Type */
	/* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szTransactionType[0], &szVWTRec[0], k - 1);
        }

        /* 08_Contactless Transaction Limit */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szContactlessTransactionLimit[0], &szVWTRec[0], k - 1);
        }

        /* 09_CVM Required Limit */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szCVMRequiredLimit[0], &szVWTRec[0], k - 1);
        }

        /* 10_Contactless Floor Limit */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szContactlessFloorLimit[0], &szVWTRec[0], k - 1);
        }

        /* 11_Enhanced DDA Version Num */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szEnhancedDDAVersionNum[0], &szVWTRec[0], k - 1);
        }

        /* 12_Display Offline Funds */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szDisplayOfflineFunds[0], &szVWTRec[0], k - 1);
        }

        /* 13_Terminal Transaction Qualifier */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szTerminalTransactionQualifier[0], &szVWTRec[0], k - 1);
        }

        /* 14_Paypass Terminal Type */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPaypassTerminalType[0], &szVWTRec[0], k - 1);
        }

        /* 15_Paypass Terminal Capabilities */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPaypassTerminalCapabilities[0], &szVWTRec[0], k - 1);
        }

        /* 16_Paypass Terminal Country Code */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPaypassTerminalCountryCode[0], &szVWTRec[0], k - 1);
        }

        /* 17_Paypass Transaction Currency Code */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPaypassTransactionCurrencyCode[0], &szVWTRec[0], k - 1);
        }

        /* 18_Paypass Default TAC */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPaypassDefaultTAC[0], &szVWTRec[0], k - 1);
        }

        /* 19_Paypass Denial TAC */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPaypassDenialTAC[0], &szVWTRec[0], k - 1);
        }

        /* 20_Paypass Online TAC */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPaypassOnlineTAC[0], &szVWTRec[0], k - 1);
        }

        /* 21_Paypass Default TDOL */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPaypassDefaultTDOL[0], &szVWTRec[0], k - 1);
        }

        /* 22_Paypass EMV Floor Limit */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPaypassEMVFloorLimit[0], &szVWTRec[0], k - 1);
        }

        /* 23_Paypass Random Selection Threshold */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPaypassRandomSelectionThreshold[0], &szVWTRec[0], k - 1);
        }

        /* 24_Paypass Target Percent for Random Selection */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPaypassTargetPercentforRandomSelection[0], &szVWTRec[0], k - 1);
        }

        /* 25_Paypass Max Target Percent for Random Selection */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPaypassMaxTargetPercentforRandomSelection[0], &szVWTRec[0], k - 1);
        }

        /* 26_Paypass CVM Required Limit */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPaypassCVMRequiredLimit[0], &szVWTRec[0], k - 1);
        }

        /* 27_Merchant Category Code */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szVWTMerchantCategoryCode[0], &szVWTRec[0], k - 1);
        }

        /* 28_Transaction Category Code */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szVWTTransactionCategoryCode[0], &szVWTRec[0], k - 1);
        }

        /* 29_Combination Option */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szCombinationOption[0], &szVWTRec[0], k - 1);
        }

        /* 30_Terminal Interchange Profile  */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szTerminalInterchangeProfile[0], &szVWTRec[0], k - 1);
        }

        /* 31_PayWave AP ID */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPayWaveAPID[0], &szVWTRec[0], k - 1);
        }

        /* 32_PayWave AP ID Contactless Transaction Limit */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPayWaveAPIDContactlessTransactionLimit[0], &szVWTRec[0], k - 1);
        }

        /* 33_PayWave AP ID CVM Required Limit */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)
        {
                memcpy(&srVWTRec.szPayWaveAPIDCVMRequiredLimit[0], &szVWTRec[0], k - 1);
        }

        /* 34_PayWave AP ID Contactless Floor Limit */
        /* 初始化 */
        memset(szVWTRec, 0x00, sizeof(szVWTRec));
        k = 0;

	/* 從Record中讀欄位資料出來 */
        while (1)
        {
                szVWTRec[k ++] = uszReadData[i ++];
                if (szVWTRec[k - 1] == 0x2C	||
		    szVWTRec[k - 1] == 0x0D	||
		    szVWTRec[k - 1] == 0x0A	||
		    szVWTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnVWTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "VWT unpack ERROR");
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
        if (szVWTRec[0] != 0x2C	&&
	    szVWTRec[0] != 0x0D	&&
	    szVWTRec[0] != 0x0A	&&
	    szVWTRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srVWTRec.szPayWaveAPIDContactlessFloorLimit[0], &szVWTRec[0], k - 1);
        }

        /* release */
	/* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadVWTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadVWTRec(%d) END!!", inVWTRec);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveVWTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSaveVWTRec(int inVWTRec)
{
        unsigned long   uldat_Handle;   		        /* FILE Handle */
        unsigned long   ulbak_Handle;   		        /* FILE Handle */
        int             inPackCount = 0;        		/* uszWriteBuff_Record的index */
        int             inRecIndex = 0;         		/* uszRead_Total_Buff的index */
        int             inVWT_Total_Rec = 0;    		/* VWT.dat的總筆數 */
        int             inRetVal;
        int             i = 0, j = 0;
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];	/* debug message */
	long	        lnReadLength = 0;       		/* 每次要從VWT.dat讀多少byte出來 */
        long            lnVWTLength = 0;         		/* VWT.dat檔案總長度 */
        unsigned char   *uszRead_Total_Buff;
        unsigned char   *uszWriteBuff_Record, *uszWriteBuff_Org;


        /* inSaveVWTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveVWTRec(%d) START!!", inVWTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 防呆先刪除VWT.bak  */
        inFILE_Delete((unsigned char *)_VWT_FILE_NAME_BAK_);

        /* 新建VWT.bak檔案 */
        inRetVal = inFILE_Create(&ulbak_Handle, (unsigned char *)_VWT_FILE_NAME_BAK_);

        if (inRetVal != VS_SUCCESS)
        {
                inFILE_Close(&ulbak_Handle);

                return (VS_ERROR);
        }

        /* 開啟原檔案VWT.dat */
        inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_VWT_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                /* VWT.dat開檔失敗 ，不用關檔VWT.dat */
                /* VWT.bak仍要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_VWT_FILE_NAME_BAK_);

                return(VS_ERROR);
        }

        /* 取得VWT.dat檔案大小 */
        lnVWTLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *)_VWT_FILE_NAME_);

	/* 取得檔案大小失敗 */
        if (lnVWTLength == VS_ERROR)
        {
                /* VWT.bak和VWT.dat要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Close(&uldat_Handle);
                inFILE_Delete((unsigned char *)_VWT_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        uszWriteBuff_Record = malloc(_SIZE_VWT_REC_ + _SIZE_VWT_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_VWT_REC_ + _SIZE_VWT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原VWT.dat的每筆Record 初始化 */
        uszWriteBuff_Org = malloc(_SIZE_VWT_REC_ + _SIZE_VWT_COMMA_0D0A_);
        memset(uszWriteBuff_Org, 0x00, _SIZE_VWT_REC_ + _SIZE_VWT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存VWT.dat全部資料 */
        uszRead_Total_Buff = malloc(lnVWTLength + 1);
        memset(uszRead_Total_Buff, 0x00, lnVWTLength + 1);

        inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

        /* ContactlessSchemeIndex */
        memcpy(&uszWriteBuff_Record[0], &srVWTRec.szContactlessSchemeIndex[0], strlen(srVWTRec.szContactlessSchemeIndex));
        inPackCount += strlen(srVWTRec.szContactlessSchemeIndex);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TerminalType */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szVWTTerminalType[0], strlen(srVWTRec.szVWTTerminalType));
        inPackCount += strlen(srVWTRec.szVWTTerminalType);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* CVMRequirement */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szCVMRequirement[0], strlen(srVWTRec.szCVMRequirement));
        inPackCount += strlen(srVWTRec.szCVMRequirement);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* VLPSupportIndicator */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szVLPSupportIndicator[0], strlen(srVWTRec.szVLPSupportIndicator));
        inPackCount += strlen(srVWTRec.szVLPSupportIndicator);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TerminalCountryCode */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szTerminalCountryCode[0], strlen(srVWTRec.szTerminalCountryCode));
        inPackCount += strlen(srVWTRec.szTerminalCountryCode);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TransactionCurrencyCode */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szVWTTransactionCurrencyCode[0], strlen(srVWTRec.szVWTTransactionCurrencyCode));
        inPackCount += strlen(srVWTRec.szVWTTransactionCurrencyCode);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TransactionType */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szTransactionType[0], strlen(srVWTRec.szTransactionType));
        inPackCount += strlen(srVWTRec.szTransactionType);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* ContactlessTransactionLimit */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szContactlessTransactionLimit[0], strlen(srVWTRec.szContactlessTransactionLimit));
        inPackCount += strlen(srVWTRec.szContactlessTransactionLimit);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* CVMRequiredLimit */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szCVMRequiredLimit[0], strlen(srVWTRec.szCVMRequiredLimit));
        inPackCount += strlen(srVWTRec.szCVMRequiredLimit);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* ContactlessFloorLimit */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szContactlessFloorLimit[0], strlen(srVWTRec.szContactlessFloorLimit));
        inPackCount += strlen(srVWTRec.szContactlessFloorLimit);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* EnhancedDDAVersionNum */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szEnhancedDDAVersionNum[0], strlen(srVWTRec.szEnhancedDDAVersionNum));
        inPackCount += strlen(srVWTRec.szEnhancedDDAVersionNum);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* DisplayOfflineFunds */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szDisplayOfflineFunds[0], strlen(srVWTRec.szDisplayOfflineFunds));
        inPackCount += strlen(srVWTRec.szDisplayOfflineFunds);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TerminalTransactionQualifier */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szTerminalTransactionQualifier[0], strlen(srVWTRec.szTerminalTransactionQualifier));
        inPackCount += strlen(srVWTRec.szTerminalTransactionQualifier);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* PaypassTerminalType */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szPaypassTerminalType[0], strlen(srVWTRec.szPaypassTerminalType));
        inPackCount += strlen(srVWTRec.szPaypassTerminalType);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* PaypassTerminalCapabilities */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szPaypassTerminalCapabilities[0], strlen(srVWTRec.szPaypassTerminalCapabilities));
        inPackCount += strlen(srVWTRec.szPaypassTerminalCapabilities);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* PaypassTerminalCountryCode */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szPaypassTerminalCountryCode[0], strlen(srVWTRec.szPaypassTerminalCountryCode));
        inPackCount += strlen(srVWTRec.szPaypassTerminalCountryCode);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* PaypassTransactionCurrencyCode */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szPaypassTransactionCurrencyCode[0], strlen(srVWTRec.szPaypassTransactionCurrencyCode));
        inPackCount += strlen(srVWTRec.szPaypassTransactionCurrencyCode);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* PaypassDefaultTAC */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szPaypassDefaultTAC[0], strlen(srVWTRec.szPaypassDefaultTAC));
        inPackCount += strlen(srVWTRec.szPaypassDefaultTAC);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* PaypassDenialTAC */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szPaypassDenialTAC[0], strlen(srVWTRec.szPaypassDenialTAC));
        inPackCount += strlen(srVWTRec.szPaypassDenialTAC);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* PaypassOnlineTAC */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szPaypassOnlineTAC[0], strlen(srVWTRec.szPaypassOnlineTAC));
        inPackCount += strlen(srVWTRec.szPaypassOnlineTAC);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* PaypassDefaultTDOL */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szPaypassDefaultTDOL[0], strlen(srVWTRec.szPaypassDefaultTDOL));
        inPackCount += strlen(srVWTRec.szPaypassDefaultTDOL);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* PaypassEMVFloorLimit */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szPaypassEMVFloorLimit[0], strlen(srVWTRec.szPaypassEMVFloorLimit));
        inPackCount += strlen(srVWTRec.szPaypassEMVFloorLimit);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* PaypassRandomSelectionThreshold */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szPaypassRandomSelectionThreshold[0], strlen(srVWTRec.szPaypassRandomSelectionThreshold));
        inPackCount += strlen(srVWTRec.szPaypassRandomSelectionThreshold);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* PaypassTargetPercentforRandomSelection */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szPaypassTargetPercentforRandomSelection[0], strlen(srVWTRec.szPaypassTargetPercentforRandomSelection));
        inPackCount += strlen(srVWTRec.szPaypassTargetPercentforRandomSelection);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* PaypassMaxTargetPercentforRandomSelection */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szPaypassMaxTargetPercentforRandomSelection[0], strlen(srVWTRec.szPaypassMaxTargetPercentforRandomSelection));
        inPackCount += strlen(srVWTRec.szPaypassMaxTargetPercentforRandomSelection);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* PaypassCVMRequiredLimit */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szPaypassCVMRequiredLimit[0], strlen(srVWTRec.szPaypassCVMRequiredLimit));
        inPackCount += strlen(srVWTRec.szPaypassCVMRequiredLimit);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* MerchantCategoryCode */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szVWTMerchantCategoryCode[0], strlen(srVWTRec.szVWTMerchantCategoryCode));
        inPackCount += strlen(srVWTRec.szVWTMerchantCategoryCode);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TransactionCategoryCode */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szVWTTransactionCategoryCode[0], strlen(srVWTRec.szVWTTransactionCategoryCode));
        inPackCount += strlen(srVWTRec.szVWTTransactionCategoryCode);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* CombinationOption */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szCombinationOption[0], strlen(srVWTRec.szCombinationOption));
        inPackCount += strlen(srVWTRec.szCombinationOption);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TerminalInterchangeProfile */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szTerminalInterchangeProfile[0], strlen(srVWTRec.szTerminalInterchangeProfile));
        inPackCount += strlen(srVWTRec.szTerminalInterchangeProfile);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* PayWaveAPID */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szPayWaveAPID[0], strlen(srVWTRec.szPayWaveAPID));
        inPackCount += strlen(srVWTRec.szPayWaveAPID);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* PayWaveAPIDContactlessTransactionLimit */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szPayWaveAPIDContactlessTransactionLimit[0], strlen(srVWTRec.szPayWaveAPIDContactlessTransactionLimit));
        inPackCount += strlen(srVWTRec.szPayWaveAPIDContactlessTransactionLimit);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* PayWaveAPIDCVMRequiredLimit */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szPayWaveAPIDCVMRequiredLimit[0], strlen(srVWTRec.szPayWaveAPIDCVMRequiredLimit));
        inPackCount += strlen(srVWTRec.szPayWaveAPIDCVMRequiredLimit);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* PayWaveAPIDContactlessFloorLimit */
        memcpy(&uszWriteBuff_Record[inPackCount], &srVWTRec.szPayWaveAPIDContactlessFloorLimit[0], strlen(srVWTRec.szPayWaveAPIDContactlessFloorLimit));
        inPackCount += strlen(srVWTRec.szPayWaveAPIDContactlessFloorLimit);

        /* 最後的data不用逗號 */
        /* 補上換行符號 */

        /* 0D是移至行首 */
        uszWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        uszWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;

        /* 讀VWT.dat檔案存到uszRead_Total_Buff */
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnVWTLength;

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
                                        inFILE_Delete((unsigned char *)_VWT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_VWT_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_VWT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        /* 防呆 總record數量小於要存取inVWTRec Return ERROR */
	/* 算總Record數 */
        for (i = 0; i < (lnVWTLength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (uszRead_Total_Buff[i] == 0x0A)
                        inVWT_Total_Rec++;
        }

	/* 防呆 總record數量小於要存取inVWTRec Return ERROR */
        if ((inVWTRec + 1) > inVWT_Total_Rec)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "No data or Index ERROR");
                }

                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_VWT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                return (VS_ERROR);
        }

        /* 記錄ReadTotalBuffer的陣列位置 */
        inRecIndex = 0;
        /* 用inVWTRec決定要先存幾筆Record到VWT.bak，ex:inVWTRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inVWTRec; i ++)
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
                                        inFILE_Delete((unsigned char *)_VWT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_VWT_FILE_NAME_BAK_);

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

        /* 存組好的該VWTRecord 到 VWT.bak */
        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_VWT_FILE_NAME_BAK_);


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
                inFILE_Delete((unsigned char *)_VWT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原VWT.dat Record 到 VWT.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
	/* inRecIndex要指到剩下還沒存進bak檔的第一個記憶體位置 */
        inVWTRec = inVWTRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < lnVWTLength; i++)
        {
                if (uszRead_Total_Buff[i] == 0x0A)
                {
                        j++;

                        if (j == inVWTRec)
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
                /* 接續存原VWT.dat的Record */
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
                                inFILE_Delete((unsigned char *)_VWT_FILE_NAME_BAK_);

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
                                inFILE_Delete((unsigned char *)_VWT_FILE_NAME_BAK_);

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

        /* 刪除原VWT.dat */
        if (inFILE_Delete((unsigned char *)_VWT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* 將VWT.bak改名字為VWT.dat取代原檔案 */
        if (inFILE_Rename((unsigned char *)_VWT_FILE_NAME_BAK_, (unsigned char *)_VWT_FILE_NAME_) != VS_SUCCESS)
	{
                return (VS_ERROR);
        }

        /* inSaveCFTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveVWTRec(%d) END!!", (inVWTRec - 1));
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
Function        :inGetContactlessSchemeIndex
Date&Time       :
Describe        :
*/
int inGetContactlessSchemeIndex(char* szContactlessSchemeIndex)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szContactlessSchemeIndex == NULL || strlen(srVWTRec.szContactlessSchemeIndex) <= 0 || strlen(srVWTRec.szContactlessSchemeIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetContactlessSchemeIndex() ERROR !!");

                        if (szContactlessSchemeIndex == NULL)
                        {
                                inLogPrintf(AT, "szContactlessSchemeIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szContactlessSchemeIndex length = (%d)", (int)strlen(srVWTRec.szContactlessSchemeIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }
                return (VS_ERROR);
        }
        memcpy(&szContactlessSchemeIndex[0], &srVWTRec.szContactlessSchemeIndex[0], strlen(srVWTRec.szContactlessSchemeIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetContactlessSchemeIndex
Date&Time       :
Describe        :
*/
int inSetContactlessSchemeIndex(char* szContactlessSchemeIndex)
{
        memset(srVWTRec.szContactlessSchemeIndex, 0x00, sizeof(srVWTRec.szContactlessSchemeIndex));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szContactlessSchemeIndex == NULL || strlen(szContactlessSchemeIndex) <= 0 || strlen(szContactlessSchemeIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetContactlessSchemeIndex() ERROR !!");

                        if (szContactlessSchemeIndex == NULL)
                        {
                                inLogPrintf(AT, "szContactlessSchemeIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szContactlessSchemeIndex length = (%d)", (int)strlen(szContactlessSchemeIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szContactlessSchemeIndex[0], &szContactlessSchemeIndex[0], strlen(szContactlessSchemeIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetVWTTerminalType
Date&Time       :
Describe        :
*/
int inGetVWTTerminalType(char* szTerminalType)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTerminalType == NULL || strlen(srVWTRec.szVWTTerminalType) <= 0 || strlen(srVWTRec.szVWTTerminalType) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTerminalType() ERROR !!");

                        if (szTerminalType == NULL)
                        {
                                inLogPrintf(AT, "szTerminalType == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTerminalType length = (%d)", (int)strlen(srVWTRec.szVWTTerminalType));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTerminalType[0], &srVWTRec.szVWTTerminalType[0], strlen(srVWTRec.szVWTTerminalType));

        return (VS_SUCCESS);

}

/*
Function        :inSetVWTTerminalType
Date&Time       :
Describe        :
*/
int inSetVWTTerminalType(char* szTerminalType)
{
        memset(srVWTRec.szVWTTerminalType, 0x00, sizeof(srVWTRec.szVWTTerminalType));
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
        memcpy(&srVWTRec.szVWTTerminalType[0], &szTerminalType[0], strlen(szTerminalType));

        return (VS_SUCCESS);
}

/*
Function        :inGetCVMRequirement
Date&Time       :
Describe        :
*/
int inGetCVMRequirement(char* szCVMRequirement)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szCVMRequirement == NULL || strlen(srVWTRec.szCVMRequirement) <= 0 || strlen(srVWTRec.szCVMRequirement) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetCVMRequirement() ERROR !!");

                        if (szCVMRequirement == NULL)
                        {
                                inLogPrintf(AT, "szCVMRequirement == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCVMRequirement length = (%d)", (int)strlen(srVWTRec.szCVMRequirement));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCVMRequirement[0], &srVWTRec.szCVMRequirement[0], strlen(srVWTRec.szCVMRequirement));

        return (VS_SUCCESS);
}

/*
Function        :inSetCVMRequirement
Date&Time       :
Describe        :
*/
int inSetCVMRequirement(char* szCVMRequirement)
{
        memset(srVWTRec.szCVMRequirement, 0x00, sizeof(srVWTRec.szCVMRequirement));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCVMRequirement == NULL || strlen(szCVMRequirement) <= 0 || strlen(szCVMRequirement) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCVMRequirement() ERROR !!");

                        if (szCVMRequirement == NULL)
                        {
                                inLogPrintf(AT, "szCVMRequirement == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCVMRequirement length = (%d)", (int)strlen(szCVMRequirement));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szCVMRequirement[0], &szCVMRequirement[0], strlen(szCVMRequirement));

        return (VS_SUCCESS);
}

/*
Function        :inGetVLPSupportIndicator
Date&Time       :
Describe        :
*/
int inGetVLPSupportIndicator(char* szVLPSupportIndicator)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szVLPSupportIndicator == NULL || strlen(srVWTRec.szVLPSupportIndicator) <= 0 || strlen(srVWTRec.szVLPSupportIndicator) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetVLPSupportIndicator() ERROR !!");

                        if (szVLPSupportIndicator == NULL)
                        {
                                inLogPrintf(AT, "szVLPSupportIndicator == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szVLPSupportIndicator length = (%d)", (int)strlen(srVWTRec.szVLPSupportIndicator));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szVLPSupportIndicator[0], &srVWTRec.szVLPSupportIndicator[0], strlen(srVWTRec.szVLPSupportIndicator));

        return (VS_SUCCESS);
}

/*
Function        :inSetVLPSupportIndicator
Date&Time       :
Describe        :
*/
int inSetVLPSupportIndicator(char* szVLPSupportIndicator)
{
        memset(srVWTRec.szVLPSupportIndicator, 0x00, sizeof(srVWTRec.szVLPSupportIndicator));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szVLPSupportIndicator == NULL || strlen(szVLPSupportIndicator) <= 0 || strlen(szVLPSupportIndicator) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetVLPSupportIndicator() ERROR !!");

                        if (szVLPSupportIndicator == NULL)
                        {
                                inLogPrintf(AT, "szVLPSupportIndicator == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szVLPSupportIndicator length = (%d)", (int)strlen(szVLPSupportIndicator));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szVLPSupportIndicator[0], &szVLPSupportIndicator[0], strlen(szVLPSupportIndicator));

        return (VS_SUCCESS);
}

/*
Function        :inGetTerminalCountryCode
Date&Time       :
Describe        :
*/
int inGetTerminalCountryCode(char* szTerminalCountryCode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTerminalCountryCode == NULL || strlen(srVWTRec.szTerminalCountryCode) <= 0 || strlen(srVWTRec.szTerminalCountryCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTerminalCountryCode() ERROR !!");

                        if (szTerminalCountryCode == NULL)
                        {
                                inLogPrintf(AT, "szTerminalCountryCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTerminalCountryCode length = (%d)", (int)strlen(srVWTRec.szTerminalCountryCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTerminalCountryCode[0], &srVWTRec.szTerminalCountryCode[0], strlen(srVWTRec.szTerminalCountryCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetTerminalCountryCode
Date&Time       :
Describe        :
*/
int inSetTerminalCountryCode(char* szTerminalCountryCode)
{
        memset(srVWTRec.szTerminalCountryCode, 0x00, sizeof(srVWTRec.szTerminalCountryCode));
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
        memcpy(&srVWTRec.szTerminalCountryCode[0], &szTerminalCountryCode[0], strlen(szTerminalCountryCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetVWTTransactionCurrencyCode
Date&Time       :
Describe        :
*/
int inGetVWTTransactionCurrencyCode(char* szTransactionCurrencyCode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTransactionCurrencyCode == NULL || strlen(srVWTRec.szVWTTransactionCurrencyCode) <= 0 || strlen(srVWTRec.szVWTTransactionCurrencyCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTransactionCurrencyCode() ERROR !!");

                        if (szTransactionCurrencyCode == NULL)
                        {
                                inLogPrintf(AT, "szTransactionCurrencyCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionCurrencyCode length = (%d)", (int)strlen(srVWTRec.szVWTTransactionCurrencyCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTransactionCurrencyCode[0], &srVWTRec.szVWTTransactionCurrencyCode[0], strlen(srVWTRec.szVWTTransactionCurrencyCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetVWTTransactionCurrencyCode
Date&Time       :
Describe        :
*/
int inSetVWTTransactionCurrencyCode(char* szTransactionCurrencyCode)
{
        memset(srVWTRec.szVWTTransactionCurrencyCode, 0x00, sizeof(srVWTRec.szVWTTransactionCurrencyCode));
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
        memcpy(&srVWTRec.szVWTTransactionCurrencyCode[0], &szTransactionCurrencyCode[0], strlen(szTransactionCurrencyCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetTransactionType
Date&Time       :
Describe        :
*/
int inGetTransactionType(char* szTransactionType)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTransactionType == NULL || strlen(srVWTRec.szTransactionType) <= 0 || strlen(srVWTRec.szTransactionType) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTransactionType() ERROR !!");

                        if (szTransactionType == NULL)
                        {
                                inLogPrintf(AT, "szTransactionType == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionType length = (%d)", (int)strlen(srVWTRec.szTransactionType));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTransactionType[0], &srVWTRec.szTransactionType[0], strlen(srVWTRec.szTransactionType));

        return (VS_SUCCESS);
}

/*
Function        :inSetTransactionType
Date&Time       :
Describe        :
*/
int inSetTransactionType(char* szTransactionType)
{
        memset(srVWTRec.szTransactionType, 0x00, sizeof(srVWTRec.szTransactionType));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTransactionType == NULL || strlen(szTransactionType) <= 0 || strlen(szTransactionType) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTransactionType() ERROR !!");

                        if (szTransactionType == NULL)
                        {
                                inLogPrintf(AT, "szTransactionType == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionType length = (%d)", (int)strlen(szTransactionType));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szTransactionType[0], &szTransactionType[0], strlen(szTransactionType));

        return (VS_SUCCESS);
}

/*
Function        :inGetContactlessTransactionLimit
Date&Time       :
Describe        :
*/
int inGetContactlessTransactionLimit(char* szContactlessTransactionLimit)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szContactlessTransactionLimit == NULL || strlen(srVWTRec.szContactlessTransactionLimit) <= 0 || strlen(srVWTRec.szContactlessTransactionLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetContactlessTransactionLimit() ERROR !!");

                        if (szContactlessTransactionLimit == NULL)
                        {
                                inLogPrintf(AT, "szContactlessTransactionLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szContactlessTransactionLimit length = (%d)", (int)strlen(srVWTRec.szContactlessTransactionLimit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szContactlessTransactionLimit[0], &srVWTRec.szContactlessTransactionLimit[0], strlen(srVWTRec.szContactlessTransactionLimit));

        return (VS_SUCCESS);
}

/*
Function        :inSetContactlessTransactionLimit
Date&Time       :
Describe        :
*/
int inSetContactlessTransactionLimit(char* szContactlessTransactionLimit)
{
        memset(srVWTRec.szContactlessTransactionLimit, 0x00, sizeof(srVWTRec.szContactlessTransactionLimit));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szContactlessTransactionLimit == NULL || strlen(szContactlessTransactionLimit) <= 0 || strlen(szContactlessTransactionLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetContactlessTransactionLimit() ERROR !!");

                        if (szContactlessTransactionLimit == NULL)
                        {
                                inLogPrintf(AT, "szContactlessTransactionLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szContactlessTransactionLimit length = (%d)", (int)strlen(szContactlessTransactionLimit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szContactlessTransactionLimit[0], &szContactlessTransactionLimit[0], strlen(szContactlessTransactionLimit));

        return (VS_SUCCESS);
}

/*
Function        :inGetCVMRequiredLimit
Date&Time       :
Describe        :
*/
int inGetCVMRequiredLimit(char* szCVMRequiredLimit)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szCVMRequiredLimit == NULL || strlen(srVWTRec.szCVMRequiredLimit) <= 0 || strlen(srVWTRec.szCVMRequiredLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetCVMRequiredLimit() ERROR !!");

                        if (szCVMRequiredLimit == NULL)
                        {
                                inLogPrintf(AT, "szCVMRequiredLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCVMRequiredLimit length = (%d)", (int)strlen(srVWTRec.szCVMRequiredLimit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCVMRequiredLimit[0], &srVWTRec.szCVMRequiredLimit[0], strlen(srVWTRec.szCVMRequiredLimit));

        return (VS_SUCCESS);
}

/*
Function        :inSetCVMRequiredLimit
Date&Time       :
Describe        :
*/
int inSetCVMRequiredLimit(char* szCVMRequiredLimit)
{
        memset(srVWTRec.szCVMRequiredLimit, 0x00, sizeof(srVWTRec.szCVMRequiredLimit));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCVMRequiredLimit == NULL || strlen(szCVMRequiredLimit) <= 0 || strlen(szCVMRequiredLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCVMRequiredLimit() ERROR !!");

                        if (szCVMRequiredLimit == NULL)
                        {
                                inLogPrintf(AT, "szCVMRequiredLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCVMRequiredLimit length = (%d)", (int)strlen(szCVMRequiredLimit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szCVMRequiredLimit[0], &szCVMRequiredLimit[0], strlen(szCVMRequiredLimit));

        return (VS_SUCCESS);
}

/*
Function        :inGetContactlessFloorLimit
Date&Time       :
Describe        :
*/
int inGetContactlessFloorLimit(char* szContactlessFloorLimit)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szContactlessFloorLimit == NULL || strlen(srVWTRec.szContactlessFloorLimit) <= 0 || strlen(srVWTRec.szContactlessFloorLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetContactlessFloorLimit() ERROR !!");

                        if (szContactlessFloorLimit == NULL)
                        {
                                inLogPrintf(AT, "szContactlessFloorLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szContactlessFloorLimit length = (%d)", (int)strlen(srVWTRec.szContactlessFloorLimit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szContactlessFloorLimit[0], &srVWTRec.szContactlessFloorLimit[0], strlen(srVWTRec.szContactlessFloorLimit));

        return (VS_SUCCESS);
}

/*
Function        :inSetContactlessFloorLimit
Date&Time       :
Describe        :
*/
int inSetContactlessFloorLimit(char* szContactlessFloorLimit)
{
        memset(srVWTRec.szContactlessFloorLimit, 0x00, sizeof(srVWTRec.szContactlessFloorLimit));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szContactlessFloorLimit == NULL || strlen(szContactlessFloorLimit) <= 0 || strlen(szContactlessFloorLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetContactlessFloorLimit() ERROR !!");

                        if (szContactlessFloorLimit == NULL)
                        {
                                inLogPrintf(AT, "szContactlessFloorLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szContactlessFloorLimit length = (%d)", (int)strlen(szContactlessFloorLimit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szContactlessFloorLimit[0], &szContactlessFloorLimit[0], strlen(szContactlessFloorLimit));

        return (VS_SUCCESS);
}

/*
Function        :inGetEnhancedDDAVersionNum
Date&Time       :
Describe        :
*/
int inGetEnhancedDDAVersionNum(char* szEnhancedDDAVersionNum)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szEnhancedDDAVersionNum == NULL || strlen(srVWTRec.szEnhancedDDAVersionNum) <= 0 || strlen(srVWTRec.szEnhancedDDAVersionNum) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetEnhancedDDAVersionNum() ERROR !!");

                        if (szEnhancedDDAVersionNum == NULL)
                        {
                                inLogPrintf(AT, "szEnhancedDDAVersionNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEnhancedDDAVersionNum length = (%d)", (int)strlen(srVWTRec.szEnhancedDDAVersionNum));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szEnhancedDDAVersionNum[0], &srVWTRec.szEnhancedDDAVersionNum[0], strlen(srVWTRec.szEnhancedDDAVersionNum));

        return (VS_SUCCESS);

}

/*
Function        :inSetEnhancedDDAVersionNum
Date&Time       :
Describe        :
*/
int inSetEnhancedDDAVersionNum(char* szEnhancedDDAVersionNum)
{
        memset(srVWTRec.szEnhancedDDAVersionNum, 0x00, sizeof(srVWTRec.szEnhancedDDAVersionNum));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szEnhancedDDAVersionNum == NULL || strlen(szEnhancedDDAVersionNum) <= 0 || strlen(szEnhancedDDAVersionNum) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetEnhancedDDAVersionNum() ERROR !!");

                        if (szEnhancedDDAVersionNum == NULL)
                        {
                                inLogPrintf(AT, "szEnhancedDDAVersionNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEnhancedDDAVersionNum length = (%d)", (int)strlen(szEnhancedDDAVersionNum));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szEnhancedDDAVersionNum[0], &szEnhancedDDAVersionNum[0], strlen(szEnhancedDDAVersionNum));

        return (VS_SUCCESS);
}

/*
Function        :inGetDisplayOfflineFunds
Date&Time       :
Describe        :
*/
int inGetDisplayOfflineFunds(char* szDisplayOfflineFunds)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szDisplayOfflineFunds == NULL || strlen(srVWTRec.szDisplayOfflineFunds) <= 0 || strlen(srVWTRec.szDisplayOfflineFunds) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetDisplayOfflineFunds() ERROR !!");

                        if (szDisplayOfflineFunds == NULL)
                        {
                                inLogPrintf(AT, "szDisplayOfflineFunds == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDisplayOfflineFunds length = (%d)", (int)strlen(srVWTRec.szDisplayOfflineFunds));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szDisplayOfflineFunds[0], &srVWTRec.szDisplayOfflineFunds[0], strlen(srVWTRec.szDisplayOfflineFunds));

        return (VS_SUCCESS);
}

/*
Function        :inSetDisplayOfflineFunds
Date&Time       :
Describe        :
*/
int inSetDisplayOfflineFunds(char* szDisplayOfflineFunds)
{
        memset(srVWTRec.szDisplayOfflineFunds, 0x00, sizeof(srVWTRec.szDisplayOfflineFunds));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szDisplayOfflineFunds == NULL || strlen(szDisplayOfflineFunds) <= 0 || strlen(szDisplayOfflineFunds) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetDisplayOfflineFunds() ERROR !!");

                        if (szDisplayOfflineFunds == NULL)
                        {
                                inLogPrintf(AT, "szDisplayOfflineFunds == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDisplayOfflineFunds length = (%d)", (int)strlen(szDisplayOfflineFunds));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szDisplayOfflineFunds[0], &szDisplayOfflineFunds[0], strlen(szDisplayOfflineFunds));

        return (VS_SUCCESS);
}

/*
Function        :inGetTerminalTransactionQualifier
Date&Time       :
Describe        :
*/
int inGetTerminalTransactionQualifier(char* szTerminalTransactionQualifier)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTerminalTransactionQualifier == NULL || strlen(srVWTRec.szTerminalTransactionQualifier) <= 0 || strlen(srVWTRec.szTerminalTransactionQualifier) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTerminalTransactionQualifier() ERROR !!");

                        if (szTerminalTransactionQualifier == NULL)
                        {
                                inLogPrintf(AT, "szTerminalTransactionQualifier == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTerminalTransactionQualifier length = (%d)", (int)strlen(srVWTRec.szTerminalTransactionQualifier));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTerminalTransactionQualifier[0], &srVWTRec.szTerminalTransactionQualifier[0], strlen(srVWTRec.szTerminalTransactionQualifier));

        return (VS_SUCCESS);
}

/*
Function        :inSetTerminalTransactionQualifier
Date&Time       :
Describe        :
*/
int inSetTerminalTransactionQualifier(char* szTerminalTransactionQualifier)
{
        memset(srVWTRec.szTerminalTransactionQualifier, 0x00, sizeof(srVWTRec.szTerminalTransactionQualifier));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTerminalTransactionQualifier == NULL || strlen(szTerminalTransactionQualifier) <= 0 || strlen(szTerminalTransactionQualifier) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTerminalTransactionQualifier() ERROR !!");

                        if (szTerminalTransactionQualifier == NULL)
                        {
                                inLogPrintf(AT, "szTerminalTransactionQualifier == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTerminalTransactionQualifier length = (%d)", (int)strlen(szTerminalTransactionQualifier));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szTerminalTransactionQualifier[0], &szTerminalTransactionQualifier[0], strlen(szTerminalTransactionQualifier));

        return (VS_SUCCESS);
}

/*
Function        :inGetPaypassTerminalType
Date&Time       :
Describe        :
*/
int inGetPaypassTerminalType(char* szPaypassTerminalType)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPaypassTerminalType == NULL || strlen(srVWTRec.szPaypassTerminalType) <= 0 || strlen(srVWTRec.szPaypassTerminalType) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPaypassTerminalType() ERROR !!");

                        if (szPaypassTerminalType == NULL)
                        {
                                inLogPrintf(AT, "szPaypassTerminalType == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassTerminalType length = (%d)", (int)strlen(srVWTRec.szPaypassTerminalType));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPaypassTerminalType[0], &srVWTRec.szPaypassTerminalType[0], strlen(srVWTRec.szPaypassTerminalType));

        return (VS_SUCCESS);
}

/*
Function        :inSetPaypassTerminalType
Date&Time       :
Describe        :
*/
int inSetPaypassTerminalType(char* szPaypassTerminalType)
{
        memset(srVWTRec.szPaypassTerminalType, 0x00, sizeof(srVWTRec.szPaypassTerminalType));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassTerminalType == NULL || strlen(szPaypassTerminalType) <= 0 || strlen(szPaypassTerminalType) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPaypassTerminalType() ERROR !!");

                        if (szPaypassTerminalType == NULL)
                        {
                                inLogPrintf(AT, "szPaypassTerminalType == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassTerminalType length = (%d)", (int)strlen(szPaypassTerminalType));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPaypassTerminalType[0], &szPaypassTerminalType[0], strlen(szPaypassTerminalType));

        return (VS_SUCCESS);
}

/*
Function        :inGetPaypassTerminalCapabilities
Date&Time       :
Describe        :
*/
int inGetPaypassTerminalCapabilities(char* szPaypassTerminalCapabilities)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPaypassTerminalCapabilities == NULL || strlen(srVWTRec.szPaypassTerminalCapabilities) <= 0 || strlen(srVWTRec.szPaypassTerminalCapabilities) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPaypassTerminalCapabilities() ERROR !!");

                        if (szPaypassTerminalCapabilities == NULL)
                        {
                                inLogPrintf(AT, "szPaypassTerminalCapabilities == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassTerminalCapabilities length = (%d)", (int)strlen(srVWTRec.szPaypassTerminalCapabilities));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPaypassTerminalCapabilities[0], &srVWTRec.szPaypassTerminalCapabilities[0], strlen(srVWTRec.szPaypassTerminalCapabilities));

        return (VS_SUCCESS);
}

/*
Function        :inSetPaypassTerminalCapabilities
Date&Time       :
Describe        :
*/
int inSetPaypassTerminalCapabilities(char* szPaypassTerminalCapabilities)
{
        memset(srVWTRec.szPaypassTerminalCapabilities, 0x00, sizeof(srVWTRec.szPaypassTerminalCapabilities));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassTerminalCapabilities == NULL || strlen(szPaypassTerminalCapabilities) <= 0 || strlen(szPaypassTerminalCapabilities) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPaypassTerminalCapabilities() ERROR !!");

                        if (szPaypassTerminalCapabilities == NULL)
                        {
                                inLogPrintf(AT, "szPaypassTerminalCapabilities == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassTerminalCapabilities length = (%d)", (int)strlen(szPaypassTerminalCapabilities));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPaypassTerminalCapabilities[0], &szPaypassTerminalCapabilities[0], strlen(szPaypassTerminalCapabilities));

        return (VS_SUCCESS);
}

/*
Function        :inGetPaypassTerminalCountryCode
Date&Time       :
Describe        :
*/
int inGetPaypassTerminalCountryCode(char* szPaypassTerminalCountryCode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPaypassTerminalCountryCode == NULL || strlen(srVWTRec.szPaypassTerminalCountryCode) <= 0 || strlen(srVWTRec.szPaypassTerminalCountryCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPaypassTerminalCountryCode() ERROR !!");

                        if (szPaypassTerminalCountryCode == NULL)
                        {
                                inLogPrintf(AT, "szPaypassTerminalCountryCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassTerminalCountryCode length = (%d)", (int)strlen(srVWTRec.szPaypassTerminalCountryCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPaypassTerminalCountryCode[0], &srVWTRec.szPaypassTerminalCountryCode[0], strlen(srVWTRec.szPaypassTerminalCountryCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetPaypassTerminalCountryCode
Date&Time       :
Describe        :
*/
int inSetPaypassTerminalCountryCode(char* szPaypassTerminalCountryCode)
{
        memset(srVWTRec.szPaypassTerminalCountryCode, 0x00, sizeof(srVWTRec.szPaypassTerminalCountryCode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassTerminalCountryCode == NULL || strlen(szPaypassTerminalCountryCode) <= 0 || strlen(szPaypassTerminalCountryCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPaypassTerminalCountryCode() ERROR !!");

                        if (szPaypassTerminalCountryCode == NULL)
                        {
                                inLogPrintf(AT, "szPaypassTerminalCountryCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassTerminalCountryCode length = (%d)", (int)strlen(szPaypassTerminalCountryCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPaypassTerminalCountryCode[0], &szPaypassTerminalCountryCode[0], strlen(szPaypassTerminalCountryCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetPaypassTransactionCurrencyCode
Date&Time       :
Describe        :
*/
int inGetPaypassTransactionCurrencyCode(char* szPaypassTransactionCurrencyCode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPaypassTransactionCurrencyCode == NULL || strlen(srVWTRec.szPaypassTransactionCurrencyCode) <= 0 || strlen(srVWTRec.szPaypassTransactionCurrencyCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPaypassTransactionCurrencyCode() ERROR !!");

                        if (szPaypassTransactionCurrencyCode == NULL)
                        {
                                inLogPrintf(AT, "szPaypassTransactionCurrencyCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassTransactionCurrencyCode length = (%d)", (int)strlen(srVWTRec.szPaypassTransactionCurrencyCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPaypassTransactionCurrencyCode[0], &srVWTRec.szPaypassTransactionCurrencyCode[0], strlen(srVWTRec.szPaypassTransactionCurrencyCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetPaypassTransactionCurrencyCode
Date&Time       :
Describe        :
*/
int inSetPaypassTransactionCurrencyCode(char* szPaypassTransactionCurrencyCode)
{
        memset(srVWTRec.szPaypassTransactionCurrencyCode, 0x00, sizeof(srVWTRec.szPaypassTransactionCurrencyCode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassTransactionCurrencyCode == NULL || strlen(szPaypassTransactionCurrencyCode) <= 0 || strlen(szPaypassTransactionCurrencyCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPaypassTransactionCurrencyCode() ERROR !!");

                        if (szPaypassTransactionCurrencyCode == NULL)
                        {
                                inLogPrintf(AT, "szPaypassTransactionCurrencyCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassTransactionCurrencyCode length = (%d)", (int)strlen(szPaypassTransactionCurrencyCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPaypassTransactionCurrencyCode[0], &szPaypassTransactionCurrencyCode[0], strlen(szPaypassTransactionCurrencyCode));

        return (VS_SUCCESS);
}

int inGetPaypassDefaultTAC(char* szPaypassDefaultTAC)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassDefaultTAC == NULL || strlen(srVWTRec.szPaypassDefaultTAC) <= 0 || strlen(srVWTRec.szPaypassDefaultTAC) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPaypassDefaultTAC() ERROR !!");

                        if (szPaypassDefaultTAC == NULL)
                        {
                                inLogPrintf(AT, "szPaypassDefaultTAC == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassDefaultTAC length = (%d)", (int)strlen(srVWTRec.szPaypassDefaultTAC));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPaypassDefaultTAC[0], &srVWTRec.szPaypassDefaultTAC[0], strlen(srVWTRec.szPaypassDefaultTAC));

        return (VS_SUCCESS);
}

int inSetPaypassDefaultTAC(char* szPaypassDefaultTAC)
{
        memset(srVWTRec.szPaypassDefaultTAC, 0x00, sizeof(srVWTRec.szPaypassDefaultTAC));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassDefaultTAC == NULL || strlen(szPaypassDefaultTAC) <= 0 || strlen(szPaypassDefaultTAC) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        if (szPaypassDefaultTAC == NULL)
                        {
                                inLogPrintf(AT, "inSetSloganPrtPositio() ERROR !! szPaypassDefaultTAC == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "inSetSloganPrtPositio() ERROR !! szPaypassDefaultTAC length = (%d)", (int)strlen(szPaypassDefaultTAC));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPaypassDefaultTAC[0], &szPaypassDefaultTAC[0], strlen(szPaypassDefaultTAC));

        return (VS_SUCCESS);
}

/*
Function        :inGetPaypassDenialTAC
Date&Time       :
Describe        :
*/
int inGetPaypassDenialTAC(char* szPaypassDenialTAC)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPaypassDenialTAC == NULL || strlen(srVWTRec.szPaypassDenialTAC) <= 0 || strlen(srVWTRec.szPaypassDenialTAC) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPaypassDenialTAC() ERROR !!");

                        if (szPaypassDenialTAC == NULL)
                        {
                                inLogPrintf(AT, "szPaypassDenialTAC == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassDenialTAC length = (%d)", (int)strlen(srVWTRec.szPaypassDenialTAC));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPaypassDenialTAC[0], &srVWTRec.szPaypassDenialTAC[0], strlen(srVWTRec.szPaypassDenialTAC));

        return (VS_SUCCESS);
}

/*
Function        :inSetPaypassDenialTAC
Date&Time       :
Describe        :
*/
int inSetPaypassDenialTAC(char* szPaypassDenialTAC)
{
        memset(srVWTRec.szPaypassDenialTAC, 0x00, sizeof(srVWTRec.szPaypassDenialTAC));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassDenialTAC == NULL || strlen(szPaypassDenialTAC) <= 0 || strlen(szPaypassDenialTAC) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPaypassDenialTAC() ERROR !!");

                        if (szPaypassDenialTAC == NULL)
                        {
                                inLogPrintf(AT, "szPaypassDenialTAC == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassDenialTAC length = (%d)", (int)strlen(szPaypassDenialTAC));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPaypassDenialTAC[0], &szPaypassDenialTAC[0], strlen(szPaypassDenialTAC));

        return (VS_SUCCESS);
}

/*
Function        :inGetPaypassOnlineTAC
Date&Time       :
Describe        :
*/
int inGetPaypassOnlineTAC(char* szPaypassOnlineTAC)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPaypassOnlineTAC == NULL || strlen(srVWTRec.szPaypassOnlineTAC) <= 0 || strlen(srVWTRec.szPaypassOnlineTAC) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPaypassOnlineTAC() ERROR !!");

                        if (szPaypassOnlineTAC == NULL)
                        {
                                inLogPrintf(AT, "szPaypassOnlineTAC == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassOnlineTAC length = (%d)", (int)strlen(srVWTRec.szPaypassOnlineTAC));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPaypassOnlineTAC[0], &srVWTRec.szPaypassOnlineTAC[0], strlen(srVWTRec.szPaypassOnlineTAC));

        return (VS_SUCCESS);
}

/*
Function        :inSetPaypassOnlineTAC
Date&Time       :
Describe        :
*/
int inSetPaypassOnlineTAC(char* szPaypassOnlineTAC)
{
        memset(srVWTRec.szPaypassOnlineTAC, 0x00, sizeof(srVWTRec.szPaypassOnlineTAC));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassOnlineTAC == NULL || strlen(szPaypassOnlineTAC) <= 0 || strlen(szPaypassOnlineTAC) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPaypassOnlineTAC() ERROR !!");

                        if (szPaypassOnlineTAC == NULL)
                        {
                                inLogPrintf(AT, "szPaypassOnlineTAC == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassOnlineTAC length = (%d)", (int)strlen(szPaypassOnlineTAC));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPaypassOnlineTAC[0], &szPaypassOnlineTAC[0], strlen(szPaypassOnlineTAC));

        return (VS_SUCCESS);
}

/*
Function        :inGetPaypassDefaultTDOL
Date&Time       :
Describe        :
*/
int inGetPaypassDefaultTDOL(char* szPaypassDefaultTDOL)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPaypassDefaultTDOL == NULL || strlen(srVWTRec.szPaypassDefaultTDOL) <= 0 || strlen(srVWTRec.szPaypassDefaultTDOL) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPaypassDefaultTDOL() ERROR !!");

                        if (szPaypassDefaultTDOL == NULL)
                        {
                                inLogPrintf(AT, "szPaypassDefaultTDOL == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassDefaultTDOL length = (%d)", (int)strlen(srVWTRec.szPaypassDefaultTDOL));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPaypassDefaultTDOL[0], &srVWTRec.szPaypassDefaultTDOL[0], strlen(srVWTRec.szPaypassDefaultTDOL));

        return (VS_SUCCESS);
}

/*
Function        :inSetPaypassDefaultTDOL
Date&Time       :
Describe        :
*/
int inSetPaypassDefaultTDOL(char* szPaypassDefaultTDOL)
{
        memset(srVWTRec.szPaypassDefaultTDOL, 0x00, sizeof(srVWTRec.szPaypassDefaultTDOL));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassDefaultTDOL == NULL || strlen(szPaypassDefaultTDOL) <= 0 || strlen(szPaypassDefaultTDOL) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPaypassDefaultTDOL() ERROR !!");

                        if (szPaypassDefaultTDOL == NULL)
                        {
                                inLogPrintf(AT, "szPaypassDefaultTDOL == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassDefaultTDOL length = (%d)", (int)strlen(szPaypassDefaultTDOL));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPaypassDefaultTDOL[0], &szPaypassDefaultTDOL[0], strlen(szPaypassDefaultTDOL));

        return (VS_SUCCESS);
}

/*
Function        :inGetPaypassEMVFloorLimit
Date&Time       :
Describe        :
*/
int inGetPaypassEMVFloorLimit(char* szPaypassEMVFloorLimit)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPaypassEMVFloorLimit == NULL || strlen(srVWTRec.szPaypassEMVFloorLimit) <= 0 || strlen(srVWTRec.szPaypassEMVFloorLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPaypassEMVFloorLimit() ERROR !!");

                        if (szPaypassEMVFloorLimit == NULL)
                        {
                                inLogPrintf(AT, "szPaypassEMVFloorLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassEMVFloorLimit length = (%d)", (int)strlen(srVWTRec.szPaypassEMVFloorLimit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPaypassEMVFloorLimit[0], &srVWTRec.szPaypassEMVFloorLimit[0], strlen(srVWTRec.szPaypassEMVFloorLimit));

        return (VS_SUCCESS);

}

/*
Function        :inSetPaypassEMVFloorLimit
Date&Time       :
Describe        :
*/
int inSetPaypassEMVFloorLimit(char* szPaypassEMVFloorLimit)
{
        memset(srVWTRec.szPaypassEMVFloorLimit, 0x00, sizeof(srVWTRec.szPaypassEMVFloorLimit));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassEMVFloorLimit == NULL || strlen(szPaypassEMVFloorLimit) <= 0 || strlen(szPaypassEMVFloorLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPaypassEMVFloorLimit() ERROR !!");

                        if (szPaypassEMVFloorLimit == NULL)
                        {
                                inLogPrintf(AT, "szPaypassEMVFloorLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassEMVFloorLimit length = (%d)", (int)strlen(szPaypassEMVFloorLimit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPaypassEMVFloorLimit[0], &szPaypassEMVFloorLimit[0], strlen(szPaypassEMVFloorLimit));

        return (VS_SUCCESS);
}

/*
Function        :inGetPaypassRandomSelectionThreshold
Date&Time       :
Describe        :
*/
int inGetPaypassRandomSelectionThreshold(char* szPaypassRandomSelectionThreshold)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPaypassRandomSelectionThreshold == NULL || strlen(srVWTRec.szPaypassRandomSelectionThreshold) <= 0 || strlen(srVWTRec.szPaypassRandomSelectionThreshold) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPaypassRandomSelectionThreshold() ERROR !!");

                        if (szPaypassRandomSelectionThreshold == NULL)
                        {
                                inLogPrintf(AT, "szPaypassRandomSelectionThreshold == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassRandomSelectionThreshold length = (%d)", (int)strlen(srVWTRec.szPaypassRandomSelectionThreshold));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPaypassRandomSelectionThreshold[0], &srVWTRec.szPaypassRandomSelectionThreshold[0], strlen(srVWTRec.szPaypassRandomSelectionThreshold));

        return (VS_SUCCESS);
}

/*
Function        :inSetPaypassRandomSelectionThreshold
Date&Time       :
Describe        :
*/
int inSetPaypassRandomSelectionThreshold(char* szPaypassRandomSelectionThreshold)
{
        memset(srVWTRec.szPaypassRandomSelectionThreshold, 0x00, sizeof(srVWTRec.szPaypassRandomSelectionThreshold));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassRandomSelectionThreshold == NULL || strlen(szPaypassRandomSelectionThreshold) <= 0 || strlen(szPaypassRandomSelectionThreshold) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPaypassRandomSelectionThreshold() ERROR !!");

                        if (szPaypassRandomSelectionThreshold == NULL)
                        {
                                inLogPrintf(AT, "szPaypassRandomSelectionThreshold == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassRandomSelectionThreshold length = (%d)", (int)strlen(szPaypassRandomSelectionThreshold));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPaypassRandomSelectionThreshold[0], &szPaypassRandomSelectionThreshold[0], strlen(szPaypassRandomSelectionThreshold));

        return (VS_SUCCESS);
}

/*
Function        :inGetPaypassTargetPercentforRandomSelection
Date&Time       :
Describe        :
*/
int inGetPaypassTargetPercentforRandomSelection(char* szPaypassTargetPercentforRandomSelection)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPaypassTargetPercentforRandomSelection == NULL || strlen(srVWTRec.szPaypassTargetPercentforRandomSelection) <= 0 || strlen(srVWTRec.szPaypassTargetPercentforRandomSelection) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPaypassTargetPercentforRandomSelection() ERROR !!");

                        if (szPaypassTargetPercentforRandomSelection == NULL)
                        {
                                inLogPrintf(AT, "szPaypassTargetPercentforRandomSelection == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassTargetPercentforRandomSelection length = (%d)", (int)strlen(srVWTRec.szPaypassTargetPercentforRandomSelection));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPaypassTargetPercentforRandomSelection[0], &srVWTRec.szPaypassTargetPercentforRandomSelection[0], strlen(srVWTRec.szPaypassTargetPercentforRandomSelection));

        return (VS_SUCCESS);
}

/*
Function        :inSetPaypassTargetPercentforRandomSelection
Date&Time       :
Describe        :
*/
int inSetPaypassTargetPercentforRandomSelection(char* szPaypassTargetPercentforRandomSelection)
{
        memset(srVWTRec.szPaypassTargetPercentforRandomSelection, 0x00, sizeof(srVWTRec.szPaypassTargetPercentforRandomSelection));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassTargetPercentforRandomSelection == NULL || strlen(szPaypassTargetPercentforRandomSelection) <= 0 || strlen(szPaypassTargetPercentforRandomSelection) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPaypassTargetPercentforRandomSelection() ERROR !!");

                        if (szPaypassTargetPercentforRandomSelection == NULL)
                        {
                                inLogPrintf(AT, "szPaypassTargetPercentforRandomSelection == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassTargetPercentforRandomSelection length = (%d)", (int)strlen(szPaypassTargetPercentforRandomSelection));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPaypassTargetPercentforRandomSelection[0], &szPaypassTargetPercentforRandomSelection[0], strlen(szPaypassTargetPercentforRandomSelection));

        return (VS_SUCCESS);
}

/*
Function        :inGetPaypassMaxTargetPercentforRandomSelection
Date&Time       :
Describe        :
*/
int inGetPaypassMaxTargetPercentforRandomSelection(char* szPaypassMaxTargetPercentforRandomSelection)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPaypassMaxTargetPercentforRandomSelection == NULL || strlen(srVWTRec.szPaypassMaxTargetPercentforRandomSelection) <= 0 || strlen(srVWTRec.szPaypassMaxTargetPercentforRandomSelection) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPaypassMaxTargetPercentforRandomSelection() ERROR !!");

                        if (szPaypassMaxTargetPercentforRandomSelection == NULL)
                        {
                                inLogPrintf(AT, "szPaypassMaxTargetPercentforRandomSelection == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassMaxTargetPercentforRandomSelection length = (%d)", (int)strlen(srVWTRec.szPaypassMaxTargetPercentforRandomSelection));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPaypassMaxTargetPercentforRandomSelection[0], &srVWTRec.szPaypassMaxTargetPercentforRandomSelection[0], strlen(srVWTRec.szPaypassMaxTargetPercentforRandomSelection));

        return (VS_SUCCESS);
}

/*
Function        :inSetPaypassMaxTargetPercentforRandomSelection
Date&Time       :
Describe        :
*/
int inSetPaypassMaxTargetPercentforRandomSelection(char* szPaypassMaxTargetPercentforRandomSelection)
{
        memset(srVWTRec.szPaypassMaxTargetPercentforRandomSelection, 0x00, sizeof(srVWTRec.szPaypassMaxTargetPercentforRandomSelection));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassMaxTargetPercentforRandomSelection == NULL || strlen(szPaypassMaxTargetPercentforRandomSelection) <= 0 || strlen(szPaypassMaxTargetPercentforRandomSelection) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPaypassMaxTargetPercentforRandomSelection() ERROR !!");

                        if (szPaypassMaxTargetPercentforRandomSelection == NULL)
                        {
                                inLogPrintf(AT, "szPaypassMaxTargetPercentforRandomSelection == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassMaxTargetPercentforRandomSelection length = (%d)", (int)strlen(szPaypassMaxTargetPercentforRandomSelection));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPaypassMaxTargetPercentforRandomSelection[0], &szPaypassMaxTargetPercentforRandomSelection[0], strlen(szPaypassMaxTargetPercentforRandomSelection));

        return (VS_SUCCESS);
}

/*
Function        :inGetPaypassCVMRequiredLimit
Date&Time       :
Describe        :
*/
int inGetPaypassCVMRequiredLimit(char* szPaypassCVMRequiredLimit)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPaypassCVMRequiredLimit == NULL || strlen(srVWTRec.szPaypassCVMRequiredLimit) <= 0 || strlen(srVWTRec.szPaypassCVMRequiredLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPaypassCVMRequiredLimit() ERROR !!");

                        if (szPaypassCVMRequiredLimit == NULL)
                        {
                                inLogPrintf(AT, "szPaypassCVMRequiredLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassCVMRequiredLimit length = (%d)", (int)strlen(srVWTRec.szPaypassCVMRequiredLimit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPaypassCVMRequiredLimit[0], &srVWTRec.szPaypassCVMRequiredLimit[0], strlen(srVWTRec.szPaypassCVMRequiredLimit));

        return (VS_SUCCESS);
}

/*
Function        :inSetPaypassCVMRequiredLimit
Date&Time       :
Describe        :
*/
int inSetPaypassCVMRequiredLimit(char* szPaypassCVMRequiredLimit)
{
        memset(srVWTRec.szPaypassCVMRequiredLimit, 0x00, sizeof(srVWTRec.szPaypassCVMRequiredLimit));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPaypassCVMRequiredLimit == NULL || strlen(szPaypassCVMRequiredLimit) <= 0 || strlen(szPaypassCVMRequiredLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPaypassCVMRequiredLimit() ERROR !!");

                        if (szPaypassCVMRequiredLimit == NULL)
                        {
                                inLogPrintf(AT, "szPaypassCVMRequiredLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPaypassCVMRequiredLimit length = (%d)", (int)strlen(szPaypassCVMRequiredLimit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPaypassCVMRequiredLimit[0], &szPaypassCVMRequiredLimit[0], strlen(szPaypassCVMRequiredLimit));

        return (VS_SUCCESS);
}

/*
Function        :inGetVWTMerchantCategoryCode
Date&Time       :
Describe        :
*/
int inGetVWTMerchantCategoryCode(char* szMerchantCategoryCode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szMerchantCategoryCode == NULL || strlen(srVWTRec.szVWTMerchantCategoryCode) <= 0 || strlen(srVWTRec.szVWTMerchantCategoryCode) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetMerchantCategoryCode() ERROR !!");

                        if (szMerchantCategoryCode == NULL)
                        {
                                inLogPrintf(AT, "szMerchantCategoryCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMerchantCategoryCode length = (%d)", (int)strlen(srVWTRec.szVWTMerchantCategoryCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMerchantCategoryCode[0], &srVWTRec.szVWTMerchantCategoryCode[0], strlen(srVWTRec.szVWTMerchantCategoryCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetVWTMerchantCategoryCode
Date&Time       :
Describe        :
*/
int inSetVWTMerchantCategoryCode(char* szMerchantCategoryCode)
{
        memset(srVWTRec.szVWTMerchantCategoryCode, 0x00, sizeof(srVWTRec.szVWTMerchantCategoryCode));
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
        memcpy(&srVWTRec.szVWTMerchantCategoryCode[0], &szMerchantCategoryCode[0], strlen(szMerchantCategoryCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetVWTTransactionCategoryCode
Date&Time       :
Describe        :
*/
int inGetVWTTransactionCategoryCode(char* szTransactionCategoryCode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTransactionCategoryCode == NULL || strlen(srVWTRec.szVWTTransactionCategoryCode) <= 0 || strlen(srVWTRec.szVWTTransactionCategoryCode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTransactionCategoryCode() ERROR !!");

                        if (szTransactionCategoryCode == NULL)
                        {
                                inLogPrintf(AT, "szTransactionCategoryCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionCategoryCode length = (%d)", (int)strlen(srVWTRec.szVWTTransactionCategoryCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTransactionCategoryCode[0], &srVWTRec.szVWTTransactionCategoryCode[0], strlen(srVWTRec.szVWTTransactionCategoryCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetVWTTransactionCategoryCode
Date&Time       :
Describe        :
*/
int inSetVWTTransactionCategoryCode(char* szTransactionCategoryCode)
{
        memset(srVWTRec.szVWTTransactionCategoryCode, 0x00, sizeof(srVWTRec.szVWTTransactionCategoryCode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTransactionCategoryCode == NULL || strlen(szTransactionCategoryCode) <= 0 || strlen(szTransactionCategoryCode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTransactionCategoryCode() ERROR !!");

                        if (szTransactionCategoryCode == NULL)
                        {
                                inLogPrintf(AT, "szTransactionCategoryCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTransactionCategoryCode length = (%d)", (int)strlen(szTransactionCategoryCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szVWTTransactionCategoryCode[0], &szTransactionCategoryCode[0], strlen(szTransactionCategoryCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetCombinationOption
Date&Time       :
Describe        :
*/
int inGetCombinationOption(char* szCombinationOption)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szCombinationOption == NULL || strlen(srVWTRec.szCombinationOption) <= 0 || strlen(srVWTRec.szCombinationOption) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetszCombinationOption() ERROR !!");

                        if (szCombinationOption == NULL)
                        {
                                inLogPrintf(AT, "szCombinationOption == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCombinationOption length = (%d)", (int)strlen(srVWTRec.szCombinationOption));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCombinationOption[0], &srVWTRec.szCombinationOption[0], strlen(srVWTRec.szCombinationOption));

        return (VS_SUCCESS);
}

/*
Function        :inSetCombinationOption
Date&Time       :
Describe        :
*/
int inSetCombinationOption(char* szCombinationOption)
{
        memset(srVWTRec.szCombinationOption, 0x00, sizeof(srVWTRec.szCombinationOption));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCombinationOption == NULL || strlen(szCombinationOption) <= 0 || strlen(szCombinationOption) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCombinationOption() ERROR !!");

                        if (szCombinationOption == NULL)
                        {
                                inLogPrintf(AT, "szCombinationOption == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCombinationOption length = (%d)", (int)strlen(szCombinationOption));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szCombinationOption[0], &szCombinationOption[0], strlen(szCombinationOption));

        return (VS_SUCCESS);
}

/*
Function        :inGetTerminalInterchangeProfile
Date&Time       :
Describe        :
*/
int inGetTerminalInterchangeProfile(char* szTerminalInterchangeProfile)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTerminalInterchangeProfile == NULL || strlen(srVWTRec.szTerminalInterchangeProfile) <= 0 || strlen(srVWTRec.szTerminalInterchangeProfile) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTerminalInterchangeProfile() ERROR !!");

                        if (szTerminalInterchangeProfile == NULL)
                        {
                                inLogPrintf(AT, "szTerminalInterchangeProfile == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTerminalInterchangeProfile length = (%d)", (int)strlen(srVWTRec.szTerminalInterchangeProfile));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTerminalInterchangeProfile[0], &srVWTRec.szTerminalInterchangeProfile[0], strlen(srVWTRec.szTerminalInterchangeProfile));

        return (VS_SUCCESS);
}

/*
Function        :inSetTerminalInterchangeProfile
Date&Time       :
Describe        :
*/
int inSetTerminalInterchangeProfile(char* szTerminalInterchangeProfile)
{
        memset(srVWTRec.szTerminalInterchangeProfile, 0x00, sizeof(srVWTRec.szTerminalInterchangeProfile));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTerminalInterchangeProfile == NULL || strlen(szTerminalInterchangeProfile) <= 0 || strlen(szTerminalInterchangeProfile) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTerminalInterchangeProfile() ERROR !!");

                        if (szTerminalInterchangeProfile == NULL)
                        {
                                inLogPrintf(AT, "szTerminalInterchangeProfile == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTerminalInterchangeProfile length = (%d)", (int)strlen(szTerminalInterchangeProfile));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szTerminalInterchangeProfile[0], &szTerminalInterchangeProfile[0], strlen(szTerminalInterchangeProfile));

        return (VS_SUCCESS);
}

/*
Function        :inGetPayWaveAPID
Date&Time       :
Describe        :
*/
int inGetPayWaveAPID(char* szPayWaveAPID)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPayWaveAPID == NULL || strlen(srVWTRec.szPayWaveAPID) <= 0 || strlen(srVWTRec.szPayWaveAPID) > 32)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPayWaveAPID() ERROR !!");

                        if (szPayWaveAPID == NULL)
                        {
                                inLogPrintf(AT, "szPayWaveAPID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPayWaveAPID length = (%d)", (int)strlen(srVWTRec.szPayWaveAPID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPayWaveAPID[0], &srVWTRec.szPayWaveAPID[0], strlen(srVWTRec.szPayWaveAPID));

        return (VS_SUCCESS);
}

/*
Function        :inSetPayWaveAPID
Date&Time       :
Describe        :
*/
int inSetPayWaveAPID(char* szPayWaveAPID)
{
        memset(srVWTRec.szPayWaveAPID, 0x00, sizeof(srVWTRec.szPayWaveAPID));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPayWaveAPID == NULL || strlen(szPayWaveAPID) <= 0 || strlen(szPayWaveAPID) > 32)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPayWaveAPID() ERROR !!");

                        if (szPayWaveAPID == NULL)
                        {
                                inLogPrintf(AT, "szPayWaveAPID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPayWaveAPID length = (%d)", (int)strlen(szPayWaveAPID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPayWaveAPID[0], &szPayWaveAPID[0], strlen(szPayWaveAPID));

        return (VS_SUCCESS);
}

/*
Function        :inGetPayWaveAPIDContactlessTransactionLimit
Date&Time       :
Describe        :
*/
int inGetPayWaveAPIDContactlessTransactionLimit(char* szPayWaveAPIDContactlessTransactionLimit)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPayWaveAPIDContactlessTransactionLimit == NULL || strlen(srVWTRec.szPayWaveAPIDContactlessTransactionLimit) <= 0 || strlen(srVWTRec.szPayWaveAPIDContactlessTransactionLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPayWaveAPIDContactlessTransactionLimit() ERROR !!");

                        if (szPayWaveAPIDContactlessTransactionLimit == NULL)
                        {
                                inLogPrintf(AT, "szPayWaveAPIDContactlessTransactionLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPayWaveAPIDContactlessTransactionLimit length = (%d)", (int)strlen(srVWTRec.szPayWaveAPIDContactlessTransactionLimit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPayWaveAPIDContactlessTransactionLimit[0], &srVWTRec.szPayWaveAPIDContactlessTransactionLimit[0], strlen(srVWTRec.szPayWaveAPIDContactlessTransactionLimit));

        return (VS_SUCCESS);

}

/*
Function        :inSetPayWaveAPIDContactlessTransactionLimit
Date&Time       :
Describe        :
*/
int inSetPayWaveAPIDContactlessTransactionLimit(char* szPayWaveAPIDContactlessTransactionLimit)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPayWaveAPIDContactlessTransactionLimit == NULL || strlen(szPayWaveAPIDContactlessTransactionLimit) <= 0 || strlen(szPayWaveAPIDContactlessTransactionLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPayWaveAPIDContactlessTransactionLimit() ERROR !!");

                        if (szPayWaveAPIDContactlessTransactionLimit == NULL)
                        {
                                inLogPrintf(AT, "szPayWaveAPIDContactlessTransactionLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPayWaveAPIDContactlessTransactionLimit length = (%d)", (int)strlen(szPayWaveAPIDContactlessTransactionLimit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPayWaveAPIDContactlessTransactionLimit[0], &szPayWaveAPIDContactlessTransactionLimit[0], strlen(szPayWaveAPIDContactlessTransactionLimit));

        return (VS_SUCCESS);
}

/*
Function        :inGetPayWaveAPIDCVMRequiredLimit
Date&Time       :
Describe        :
*/
int inGetPayWaveAPIDCVMRequiredLimit(char* szPayWaveAPIDCVMRequiredLimit)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPayWaveAPIDCVMRequiredLimit == NULL || strlen(srVWTRec.szPayWaveAPIDCVMRequiredLimit) <= 0 || strlen(srVWTRec.szPayWaveAPIDCVMRequiredLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPayWaveAPIDCVMRequiredLimit() ERROR !!");

                        if (szPayWaveAPIDCVMRequiredLimit == NULL)
                        {
                                inLogPrintf(AT, "szPayWaveAPIDCVMRequiredLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPayWaveAPIDCVMRequiredLimit length = (%d)", (int)strlen(srVWTRec.szPayWaveAPIDCVMRequiredLimit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPayWaveAPIDCVMRequiredLimit[0], &srVWTRec.szPayWaveAPIDCVMRequiredLimit[0], strlen(srVWTRec.szPayWaveAPIDCVMRequiredLimit));

        return (VS_SUCCESS);
}

/*
Function        :inSetPayWaveAPIDCVMRequiredLimit
Date&Time       :
Describe        :
*/
int inSetPayWaveAPIDCVMRequiredLimit(char* szPayWaveAPIDCVMRequiredLimit)
{
        memset(srVWTRec.szPayWaveAPIDCVMRequiredLimit, 0x00, sizeof(srVWTRec.szPayWaveAPIDCVMRequiredLimit));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPayWaveAPIDCVMRequiredLimit == NULL || strlen(szPayWaveAPIDCVMRequiredLimit) <= 0 || strlen(szPayWaveAPIDCVMRequiredLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPayWaveAPIDCVMRequiredLimit() ERROR !!");

                        if (szPayWaveAPIDCVMRequiredLimit == NULL)
                        {
                                inLogPrintf(AT, "szPayWaveAPIDCVMRequiredLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPayWaveAPIDCVMRequiredLimit length = (%d)", (int)strlen(szPayWaveAPIDCVMRequiredLimit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPayWaveAPIDCVMRequiredLimit[0], &szPayWaveAPIDCVMRequiredLimit[0], strlen(szPayWaveAPIDCVMRequiredLimit));

        return (VS_SUCCESS);
}

/*
Function        :inGetPayWaveAPIDContactlessFloorLimit
Date&Time       :
Describe        :
*/
int inGetPayWaveAPIDContactlessFloorLimit(char* szPayWaveAPIDContactlessFloorLimit)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPayWaveAPIDContactlessFloorLimit == NULL || strlen(srVWTRec.szPayWaveAPIDContactlessFloorLimit) <= 0 || strlen(srVWTRec.szPayWaveAPIDContactlessFloorLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPayWaveAPIDContactlessFloorLimit() ERROR !!");

                        if (szPayWaveAPIDContactlessFloorLimit == NULL)
                        {
                                inLogPrintf(AT, "szPayWaveAPIDContactlessFloorLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPayWaveAPIDContactlessFloorLimit length = (%d)", (int)strlen(srVWTRec.szPayWaveAPIDContactlessFloorLimit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPayWaveAPIDContactlessFloorLimit[0], &srVWTRec.szPayWaveAPIDContactlessFloorLimit[0], strlen(srVWTRec.szPayWaveAPIDContactlessFloorLimit));

        return (VS_SUCCESS);
}

/*
Function        :inSetPayWaveAPIDContactlessFloorLimit
Date&Time       :
Describe        :
*/
int inSetPayWaveAPIDContactlessFloorLimit(char* szPayWaveAPIDContactlessFloorLimit)
{
        memset(srVWTRec.szPayWaveAPIDContactlessFloorLimit, 0x00, sizeof(srVWTRec.szPayWaveAPIDContactlessFloorLimit));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPayWaveAPIDContactlessFloorLimit == NULL || strlen(szPayWaveAPIDContactlessFloorLimit) <= 0 || strlen(szPayWaveAPIDContactlessFloorLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPayWaveAPIDContactlessFloorLimit() ERROR !!");

                        if (szPayWaveAPIDContactlessFloorLimit == NULL)
                        {
                                inLogPrintf(AT, "szPayWaveAPIDContactlessFloorLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPayWaveAPIDContactlessFloorLimit length = (%d)", (int)strlen(szPayWaveAPIDContactlessFloorLimit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srVWTRec.szPayWaveAPIDContactlessFloorLimit[0], &szPayWaveAPIDContactlessFloorLimit[0], strlen(szPayWaveAPIDContactlessFloorLimit));

        return (VS_SUCCESS);
}

/*
Function        :inVWT_Edit_VWT_Table
Date&Time       :2020/1/6 下午 5:31
Describe        :
*/
int inVWT_Edit_VWT_Table(void)
{
	TABLE_GET_SET_TABLE VWT_FUNC_TABLE[] =
	{
		{"szContactlessSchemeIndex"			,inGetContactlessSchemeIndex			,inSetContactlessSchemeIndex			},
		{"szVWTTerminalType"				,inGetVWTTerminalType				,inSetVWTTerminalType				},
		{"szCVMRequirement"				,inGetCVMRequirement				,inSetCVMRequirement				},
		{"szVLPSupportIndicator"			,inGetVLPSupportIndicator			,inSetVLPSupportIndicator			},
		{"szTerminalCountryCode"			,inGetTerminalCountryCode			,inSetTerminalCountryCode			},
		{"szVWTTransactionCurrencyCode"			,inGetVWTTransactionCurrencyCode		,inSetVWTTransactionCurrencyCode		},
		{"szTransactionType"				,inGetTransactionType				,inSetTransactionType				},
		{"szContactlessTransactionLimit"		,inGetContactlessTransactionLimit		,inSetContactlessTransactionLimit		},
		{"szCVMRequiredLimit"				,inGetCVMRequiredLimit				,inSetCVMRequiredLimit				},
		{"szContactlessFloorLimit"			,inGetContactlessFloorLimit			,inSetContactlessFloorLimit			},
		{"szEnhancedDDAVersionNum"			,inGetEnhancedDDAVersionNum			,inSetEnhancedDDAVersionNum			},
		{"szDisplayOfflineFunds"			,inGetDisplayOfflineFunds			,inSetDisplayOfflineFunds			},
		{"szTerminalTransactionQualifier"		,inGetTerminalTransactionQualifier		,inSetTerminalTransactionQualifier		},
		{"szPaypassTerminalType"			,inGetPaypassTerminalType			,inSetPaypassTerminalType			},
		{"szPaypassTerminalCapabilities"		,inGetPaypassTerminalCapabilities		,inSetPaypassTerminalCapabilities		},
		{"szPaypassTerminalCountryCode"			,inGetPaypassTerminalCountryCode		,inSetPaypassTerminalCountryCode		},
		{"szPaypassTransactionCurrencyCode"		,inGetPaypassTransactionCurrencyCode		,inSetPaypassTransactionCurrencyCode		},
		{"szPaypassDefaultTAC"				,inGetPaypassDefaultTAC				,inSetPaypassDefaultTAC				},
		{"szPaypassDenialTAC"				,inGetPaypassDenialTAC				,inSetPaypassDenialTAC				},
		{"szPaypassOnlineTAC"				,inGetPaypassOnlineTAC				,inSetPaypassOnlineTAC				},
		{"szPaypassDefaultTDOL"				,inGetPaypassDefaultTDOL			,inSetPaypassDefaultTDOL			},
		{"szPaypassEMVFloorLimit"			,inGetPaypassEMVFloorLimit			,inSetPaypassEMVFloorLimit			},
		{"szPaypassRandomSelectionThreshold"		,inGetPaypassRandomSelectionThreshold		,inSetPaypassRandomSelectionThreshold		},
		{"szPaypassTargetPercentforRandomSelection"	,inGetPaypassTargetPercentforRandomSelection	,inSetPaypassTargetPercentforRandomSelection	},
		{"szPaypassMaxTargetPercentforRandomSelection"	,inGetPaypassMaxTargetPercentforRandomSelection	,inSetPaypassMaxTargetPercentforRandomSelection	},
		{"szPaypassCVMRequiredLimit"			,inGetPaypassCVMRequiredLimit			,inSetPaypassCVMRequiredLimit			},
		{"szVWTMerchantCategoryCode"			,inGetVWTMerchantCategoryCode			,inSetVWTMerchantCategoryCode			},
		{"szVWTTransactionCategoryCode"			,inGetVWTTransactionCategoryCode		,inSetVWTTransactionCategoryCode		},
		{"szCombinationOption"				,inGetCombinationOption				,inSetCombinationOption				},
		{"szTerminalInterchangeProfile"			,inGetTerminalInterchangeProfile		,inSetTerminalInterchangeProfile		},
		{"szPayWaveAPID"				,inGetPayWaveAPID				,inSetPayWaveAPID				},
		{"szPayWaveAPIDContactlessTransactionLimit"	,inGetPayWaveAPIDContactlessTransactionLimit	,inSetPayWaveAPIDContactlessTransactionLimit	},
		{"szPayWaveAPIDCVMRequiredLimit"		,inGetPayWaveAPIDCVMRequiredLimit		,inSetPayWaveAPIDCVMRequiredLimit		},
		{"szPayWaveAPIDContactlessFloorLimit"		,inGetPayWaveAPIDContactlessFloorLimit		,inSetPayWaveAPIDContactlessFloorLimit		},
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
	inDISP_ChineseFont_Color("是否更改VWT", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
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
	inLoadVWTRec(inRecordCnt);
	
	inFunc_Edit_Table_Tag(VWT_FUNC_TABLE);
	inSaveVWTRec(inRecordCnt);
	
	return	(VS_SUCCESS);
}
