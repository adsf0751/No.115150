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
#include "HDT.h"

static  HDT_REC srHDTRec;	/* construct HDT record */
extern  int     ginDebug;	/* Debug使用 extern */

/*
Function        :inLoadHDTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀HDT檔案，inHDTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadHDTRec(int inHDTRec)
{
        unsigned long   ulFile_Handle;          		/* File Handle */
        unsigned char   *uszReadData;           		/* 放抓到的record */
        unsigned char   *uszTemp;               		/* 暫存，放整筆HDT檔案 */
        char            szHDTRec[_SIZE_HDT_REC_ + 1];      	/* 暫存, 放各個欄位檔案 */
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1]; 	/* debug message */
        long            lnHDTLength = 0;        		/* HDT總長度 */
        long            lnReadLength;           		/* 記錄每次要從HDT.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0; 		/* inRec記錄讀到第幾筆, i為目前從HDT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;    		/* 判斷有沒有讀到0x0D 0x0A的Flag */

        /* inLoadHDTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadHDTRec(%d) START!!", inHDTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 判斷傳進來的inHDTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inHDTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inHDTRec < 0:(index = %d) ERROR!!", inHDTRec);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open HDT.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_HDT_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnHDTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_HDT_FILE_NAME_);

	if (lnHDTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnHDTLength + 1);
        uszTemp = malloc(lnHDTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnHDTLength + 1);
        memset(uszTemp, 0x00, lnHDTLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnHDTLength;

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
         *i為目前從HDT讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnHDTLength; ++i)      /* "<="是為了抓到最後一個0x00 */
        {
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
			inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnHDTLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        /* 因為inHDT_Rec的index從0開始，所以inHDT_Rec要+1 */
                        if (inRec == (inHDTRec + 1))
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
         * 如果沒有inHDTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inHDTRec + 1) || inSearchResult == -1)
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
        memset(&srHDTRec, 0x00, sizeof(srHDTRec));
        /*
         * 以下pattern為存入HDT_Rec
         * i為HDT的第幾個字元
         * 存入HDT_Rec
         */
        i = 0;


        /* 01_主機索引 */
        /* 初始化 */
        memset(szHDTRec, 0x00, sizeof(szHDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDTRec[k ++] = uszReadData[i ++];
                if (szHDTRec[k - 1] == 0x2C	||
		    szHDTRec[k - 1] == 0x0D	||
		    szHDTRec[k - 1] == 0x0A	||
		    szHDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "HDT unpack ERROR");
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
        if (szHDTRec[0] != 0x2C	&&
	    szHDTRec[0] != 0x0D	&&
	    szHDTRec[0] != 0x0A	&&
	    szHDTRec[0] != 0x00)
        {
                memcpy(&srHDTRec.szHostIndex[0], &szHDTRec[0], k - 1);
        }

        /* 02_主機功能是否開啟 */
	/* 初始化 */
        memset(szHDTRec, 0x00, sizeof(szHDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDTRec[k ++] = uszReadData[i ++];
                if (szHDTRec[k - 1] == 0x2C	||
		    szHDTRec[k - 1] == 0x0D	||
		    szHDTRec[k - 1] == 0x0A	||
		    szHDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "HDT unpack ERROR");
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
        if (szHDTRec[0] != 0x2C	&&
	    szHDTRec[0] != 0x0D	&&
	    szHDTRec[0] != 0x0A	&&
	    szHDTRec[0] != 0x00)
        {
                memcpy(&srHDTRec.szHostEnable[0], &szHDTRec[0], k - 1);
        }

        /* 03_主機名稱 */
	/* 初始化 */
        memset(szHDTRec, 0x00, sizeof(szHDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDTRec[k ++] = uszReadData[i ++];
                if (szHDTRec[k - 1] == 0x2C	||
		    szHDTRec[k - 1] == 0x0D	||
		    szHDTRec[k - 1] == 0x0A	||
		    szHDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "HDT unpack ERROR");
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
        if (szHDTRec[0] != 0x2C	&&
	    szHDTRec[0] != 0x0D	&&
	    szHDTRec[0] != 0x0A	&&
	    szHDTRec[0] != 0x00)
        {
                memcpy(&srHDTRec.szHostLabel[0], &szHDTRec[0], k - 1);
        }

        /* 04_商店代號 */
        /* 初始化 */
        memset(szHDTRec, 0x00, sizeof(szHDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDTRec[k ++] = uszReadData[i ++];
                if (szHDTRec[k - 1] == 0x2C	||
		    szHDTRec[k - 1] == 0x0D	||
		    szHDTRec[k - 1] == 0x0A	||
		    szHDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "HDT unpack ERROR");
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
        if (szHDTRec[0] != 0x2C	&&
	    szHDTRec[0] != 0x0D	&&
	    szHDTRec[0] != 0x0A	&&
	    szHDTRec[0] != 0x00)
        {
                memcpy(&srHDTRec.szMerchantID[0], &szHDTRec[0], k - 1);
        }

        /* 05_端末機代號 */
        /* 初始化 */
        memset(szHDTRec, 0x00, sizeof(szHDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDTRec[k ++] = uszReadData[i ++];
                if (szHDTRec[k - 1] == 0x2C	||
		    szHDTRec[k - 1] == 0x0D	||
		    szHDTRec[k - 1] == 0x0A	||
		    szHDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "HDT unpack ERROR");
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
        if (szHDTRec[0] != 0x2C	&&
	    szHDTRec[0] != 0x0D	&&
	    szHDTRec[0] != 0x0A	&&
	    szHDTRec[0] != 0x00)
        {
                memcpy(&srHDTRec.szTerminalID[0], &szHDTRec[0], k - 1);
        }

        /* 06_對應通訊參數索引 */
        /* 初始化 */
        memset(szHDTRec, 0x00, sizeof(szHDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDTRec[k ++] = uszReadData[i ++];
                if (szHDTRec[k - 1] == 0x2C	||
		    szHDTRec[k - 1] == 0x0D	||
		    szHDTRec[k - 1] == 0x0A	||
		    szHDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "HDT unpack ERROR");
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
        if (szHDTRec[0] != 0x2C	&&
	    szHDTRec[0] != 0x0D	&&
	    szHDTRec[0] != 0x0A	&&
	    szHDTRec[0] != 0x00)
        {
                memcpy(&srHDTRec.szCommunicationIndex[0], &szHDTRec[0], k - 1);
        }

        /* 07_交易功能參數 */
        /* 初始化 */
        memset(szHDTRec, 0x00, sizeof(szHDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDTRec[k ++] = uszReadData[i ++];
                if (szHDTRec[k - 1] == 0x2C	||
		    szHDTRec[k - 1] == 0x0D	||
		    szHDTRec[k - 1] == 0x0A	||
		    szHDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "HDT unpack ERROR");
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
        if (szHDTRec[0] != 0x2C	&&
	    szHDTRec[0] != 0x0D	&&
	    szHDTRec[0] != 0x0A	&&
	    szHDTRec[0] != 0x00)
        {
                memcpy(&srHDTRec.szTransFunc[0], &szHDTRec[0], k - 1);
        }

        /* 08_人工輸入卡號功能 */
        /* 初始化 */
        memset(szHDTRec, 0x00, sizeof(szHDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDTRec[k ++] = uszReadData[i ++];
                if (szHDTRec[k - 1] == 0x2C	||
		    szHDTRec[k - 1] == 0x0D	||
		    szHDTRec[k - 1] == 0x0A	||
		    szHDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "HDT unpack ERROR");
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
        if (szHDTRec[0] != 0x2C	&&
	    szHDTRec[0] != 0x0D	&&
	    szHDTRec[0] != 0x0A	&&
	    szHDTRec[0] != 0x00)
        {
                memcpy(&srHDTRec.szManualKeyin[0], &szHDTRec[0], k - 1);
        }

        /* 09_Call Bank功能 */
        /* 初始化 */
        memset(szHDTRec, 0x00, sizeof(szHDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDTRec[k ++] = uszReadData[i ++];
                if (szHDTRec[k - 1] == 0x2C	||
		    szHDTRec[k - 1] == 0x0D	||
		    szHDTRec[k - 1] == 0x0A	||
		    szHDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "HDT unpack ERROR");
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
        if (szHDTRec[0] != 0x2C	&&
	    szHDTRec[0] != 0x0D	&&
	    szHDTRec[0] != 0x0A	&&
	    szHDTRec[0] != 0x00)
        {
                memcpy(&srHDTRec.szCallBankEnable[0], &szHDTRec[0], k - 1);
        }

        /* 10_小費檢合百分比 */
        /* 初始化 */
        memset(szHDTRec, 0x00, sizeof(szHDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDTRec[k ++] = uszReadData[i ++];
                if (szHDTRec[k - 1] == 0x2C	||
		    szHDTRec[k - 1] == 0x0D	||
		    szHDTRec[k - 1] == 0x0A	||
		    szHDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "HDT unpack ERROR");
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
        if (szHDTRec[0] != 0x2C	&&
	    szHDTRec[0] != 0x0D	&&
	    szHDTRec[0] != 0x0A	&&
	    szHDTRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srHDTRec.szTipPercent[0], &szHDTRec[0], k - 1);
        }

        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadHDTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadHDTRec(%d) END!!", inHDTRec);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveHDTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :寫入HDT.dat，inHDTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inSaveHDTRec(int inHDTRec)
{
        unsigned long   uldat_Handle;   			/* FILE Handle */
        unsigned long   ulbak_Handle;   			/* FILE Handle */
        int		inPackCount = 0;        		/* uszWriteBuff_Record的index */
        int		inRecIndex = 0;         		/* uszRead_Total_Buff的index */
        int     	inHDT_Total_Rec = 0;    		/* HDT.dat的總筆數 */
        int     	inRetVal;
        int     	i = 0, j = 0;
        char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];	/* debug message */
        long    	lnReadLength = 0;       		/* 每次要從HDT.dat讀多少byte出來 */
        long    	lnHDTLength = 0;         		/* HDT.dat檔案總長度 */
        unsigned char	*uszRead_Total_Buff;
        unsigned char   *uszWriteBuff_Record, *uszWriteBuff_Org;


        /* inSaveHDTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveHDTRec(%d) START!!", inHDTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 防呆先刪除HDT.bak  */
	
        inFILE_Delete((unsigned char *)_HDT_FILE_NAME_BAK_);

        /* 新建HDT.bak檔案 */
        inRetVal = inFILE_Create(&ulbak_Handle, (unsigned char *)_HDT_FILE_NAME_BAK_);

        if (inRetVal != VS_SUCCESS)
        {
                inFILE_Close(&ulbak_Handle);

                return (VS_ERROR);
        }

        /* 開啟原檔案HDT.dat */
        inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_HDT_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                /* HDT.dat開檔失敗 ，不用關檔HDT.dat */
                /* HDT.bak仍要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_HDT_FILE_NAME_BAK_);

                return(VS_ERROR);
        }

        /* 取得HDT.dat檔案大小 */
        lnHDTLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *)_HDT_FILE_NAME_);

        /* 取得檔案大小失敗 */
        if (lnHDTLength == VS_ERROR)
        {
                /* HDT.bak和HDT.dat要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Close(&uldat_Handle);
                inFILE_Delete((unsigned char *)_HDT_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        uszWriteBuff_Record = malloc(_SIZE_HDT_REC_ + _SIZE_HDT_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_HDT_REC_ + _SIZE_HDT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原HDT.dat的每筆Record 初始化 */
        uszWriteBuff_Org = malloc(_SIZE_HDT_REC_ + _SIZE_HDT_COMMA_0D0A_);
        memset(uszWriteBuff_Org, 0x00, _SIZE_HDT_REC_ + _SIZE_HDT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存HDT.dat全部資料 */
        uszRead_Total_Buff = malloc(lnHDTLength + 1);
        memset(uszRead_Total_Buff, 0x00, lnHDTLength + 1);

        inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

	/* HostIndex */
        memcpy(&uszWriteBuff_Record[0], &srHDTRec.szHostIndex[0], strlen(srHDTRec.szHostIndex));
        inPackCount += strlen(srHDTRec.szHostIndex);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* HostEnable */
        memcpy(&uszWriteBuff_Record[inPackCount], &srHDTRec.szHostEnable[0], strlen(srHDTRec.szHostEnable));
        inPackCount += strlen(srHDTRec.szHostEnable);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* HostLabel */
        memcpy(&uszWriteBuff_Record[inPackCount], &srHDTRec.szHostLabel[0], strlen(srHDTRec.szHostLabel));
        inPackCount += strlen(srHDTRec.szHostLabel);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* MerchantID */
        memcpy(&uszWriteBuff_Record[inPackCount], &srHDTRec.szMerchantID[0], strlen(srHDTRec.szMerchantID));
        inPackCount += strlen(srHDTRec.szMerchantID);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* TerminalID */
        memcpy(&uszWriteBuff_Record[inPackCount], &srHDTRec.szTerminalID[0], strlen(srHDTRec.szTerminalID));
        inPackCount += strlen(srHDTRec.szTerminalID);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* CommunicationIndex */
        memcpy(&uszWriteBuff_Record[inPackCount], &srHDTRec.szCommunicationIndex[0], strlen(srHDTRec.szCommunicationIndex));
        inPackCount += strlen(srHDTRec.szCommunicationIndex);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* TransFunc */
        memcpy(&uszWriteBuff_Record[inPackCount], &srHDTRec.szTransFunc[0], strlen(srHDTRec.szTransFunc));
        inPackCount += strlen(srHDTRec.szTransFunc);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* ManualKeyin */
        memcpy(&uszWriteBuff_Record[inPackCount], &srHDTRec.szManualKeyin[0], strlen(srHDTRec.szManualKeyin));
        inPackCount += strlen(srHDTRec.szManualKeyin);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* CallBankEnable */
        memcpy(&uszWriteBuff_Record[inPackCount], &srHDTRec.szCallBankEnable[0], strlen(srHDTRec.szCallBankEnable));
        inPackCount += strlen(srHDTRec.szCallBankEnable);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* TipPercent */
        memcpy(&uszWriteBuff_Record[inPackCount], &srHDTRec.szTipPercent[0], strlen(srHDTRec.szTipPercent));
        inPackCount += strlen(srHDTRec.szTipPercent);

        /* 最後的data不用逗號 */
        /* 補上換行符號 */

        /* 0D是移至行首 */
        uszWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        uszWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;

        /* 讀HDT.dat檔案存到uszRead_Total_Buff */
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnHDTLength;

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
                                        inFILE_Delete((unsigned char *)_HDT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_HDT_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_HDT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        /* 防呆 總record數量小於要存取inHDTRec Return ERROR */
        /* 算總Record數 */
        for (i = 0; i < (lnHDTLength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (uszRead_Total_Buff[i] == 0x0A)
                        inHDT_Total_Rec++;
        }

//        /* 防呆 總record數量小於要存取inHDTRec Return ERROR */
//        if ((inHDTRec + 1) > inHDT_Total_Rec)
//        {
//                if (ginDebug == VS_TRUE)
//                {
//                        inLogPrintf(AT, "No data or Index ERROR");
//                }
//
//                /* 關檔 */
//                inFILE_Close(&uldat_Handle);
//                inFILE_Close(&ulbak_Handle);
//                inFILE_Delete((unsigned char *)_HDT_FILE_NAME_BAK_);
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
        /* 用inHDTRec決定要先存幾筆Record到HDT.bak，ex:inHDTRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inHDTRec; i ++)
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
                                        inFILE_Delete((unsigned char *)_HDT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_HDT_FILE_NAME_BAK_);

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

        /* 存組好的該HDTRecord 到 HDT.bak */
        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_HDT_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_HDT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原HDT.dat Record 到 HDT.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
        /* inRecIndex要指到剩下還沒存進bak檔的第一個記憶體位置 */
        inHDTRec = inHDTRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < lnHDTLength; i++)
        {
                if (uszRead_Total_Buff[i] == 0x0A)
                {
                        j++;

                        if (j == inHDTRec)
                        {
                                break;
                        }
                }
        }
	/* 若inRecIndex = lnHDTLength + 1，代表後面不需再寫東西 */
	inRecIndex = i + 1;

        i = 0;
        memset(uszWriteBuff_Org, 0x00, sizeof(uszWriteBuff_Org));

        while (1)
        {
                /* 接續存原HDT.dat的Record */
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
                                inFILE_Delete((unsigned char *)_HDT_FILE_NAME_BAK_);

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
                                inFILE_Delete((unsigned char *)_HDT_FILE_NAME_BAK_);

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

        /* 刪除原HDT.dat */
        if (inFILE_Delete((unsigned char *)_HDT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }
        /* 將HDT.bak改名字為HDT.dat取代原檔案 */
        if (inFILE_Rename((unsigned char *)_HDT_FILE_NAME_BAK_, (unsigned char *)_HDT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* inSaveHDTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveHDTRec(%d) END!!", inHDTRec - 1);
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
Function        :inGetHostIndex
Date&Time       :
Describe        :
*/
int inGetHostIndex(char* szHostIndex)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostIndex == NULL || strlen(srHDTRec.szHostIndex) <= 0 || strlen(srHDTRec.szHostIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                	inLogPrintf(AT, "inGetHostIndex() ERROR !!");

			if (szHostIndex == NULL)
			{
				inLogPrintf(AT, "szHostIndex == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szHostIndex length = (%d)", (int)strlen(srHDTRec.szHostIndex));
				inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&szHostIndex[0], &srHDTRec.szHostIndex[0], strlen(srHDTRec.szHostIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetHostIndex
Date&Time       :
Describe        :
*/
int inSetHostIndex(char* szHostIndex)
{
        memset(srHDTRec.szHostIndex, 0x00, sizeof(srHDTRec.szHostIndex));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostIndex == NULL || strlen(szHostIndex) <= 0 || strlen(szHostIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetHostIndex() ERROR !!");

			if (szHostIndex == NULL)
			{
				inLogPrintf(AT, "szHostIndex == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szHostIndex length = (%d)", (int)strlen(szHostIndex));
				inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srHDTRec.szHostIndex[0], &szHostIndex[0], strlen(szHostIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetHostEnable
Date&Time       :
Describe        :
*/
int inGetHostEnable(char* szHostEnable)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostEnable == NULL || strlen(srHDTRec.szHostEnable) <= 0 || strlen(srHDTRec.szHostEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                	inLogPrintf(AT, "inGetHostEnable() ERROR !!");

                	if (szHostEnable == NULL)
                	{
                	        inLogPrintf(AT, "szHostEnable == NULL");
                	}
                	else
                	{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szHostEnable length = (%d)", (int)strlen(srHDTRec.szHostEnable));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szHostEnable[0], &srHDTRec.szHostEnable[0], strlen(srHDTRec.szHostEnable));

        return (VS_SUCCESS);

}

/*
Function        :inSetHostEnable
Date&Time       :
Describe        :
*/
int inSetHostEnable(char* szHostEnable)
{
        memset(srHDTRec.szHostEnable, 0x00, sizeof(srHDTRec.szHostEnable));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostEnable == NULL || strlen(szHostEnable) <= 0 || strlen(szHostEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetHostEnable() ERROR !! ");

			if (szHostEnable == NULL)
			{
				inLogPrintf(AT, "szHostEnable == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szHostEnable length = (%d)", (int)strlen(szHostEnable));
				inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srHDTRec.szHostEnable[0], &szHostEnable[0], strlen(szHostEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetHostLabel
Date&Time       :
Describe        :
*/
int inGetHostLabel(char* szHostLabel)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostLabel == NULL || strlen(srHDTRec.szHostLabel) < 0 || strlen(srHDTRec.szHostLabel) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inGetHostLabel() ERROR !!");

			if (szHostLabel == NULL)
			{
				inLogPrintf(AT, "szHostLabel == NULL");
			}
			else if (strlen(srHDTRec.szHostLabel) < 0 || strlen(srHDTRec.szHostLabel) > 8)
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szHostLabel length = (%d) ", (int)strlen(srHDTRec.szHostLabel));
				inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&szHostLabel[0], &srHDTRec.szHostLabel[0], strlen(srHDTRec.szHostLabel));

        return (VS_SUCCESS);
}

/*
Function        :inSetHostLabel
Date&Time       :
Describe        :
*/
int inSetHostLabel(char* szHostLabel)
{
        memset(srHDTRec.szHostLabel, 0x00, sizeof(srHDTRec.szHostLabel));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHostLabel == NULL || strlen(szHostLabel) <= 0 || strlen(szHostLabel) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetHostLabel() ERROR !!");

			if (szHostLabel == NULL)
			{
				inLogPrintf(AT, "szHostLabel == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szHostLabel length = (%d)", (int)strlen(szHostLabel));
				inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srHDTRec.szHostLabel[0], &szHostLabel[0], strlen(szHostLabel));

        return (VS_SUCCESS);
}

/*
Function        :inGetMerchantID
Date&Time       :
Describe        :
*/
int inGetMerchantID(char* szMerchantID)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMerchantID == NULL || strlen(srHDTRec.szMerchantID) <= 0 || strlen(srHDTRec.szMerchantID) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inGetMerchantID() ERROR !!");

			if (szMerchantID == NULL)
			{
				inLogPrintf(AT, "szMerchantID == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szMerchantID length = (%d)", (int)strlen(srHDTRec.szMerchantID));
				inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&szMerchantID[0], &srHDTRec.szMerchantID[0], strlen(srHDTRec.szMerchantID));

        return (VS_SUCCESS);
}

/*
Function        :inSetMerchantID
Date&Time       :
Describe        :
*/
int inSetMerchantID(char* szMerchantID)
{
        memset(srHDTRec.szMerchantID, 0x00, sizeof(srHDTRec.szMerchantID));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMerchantID == NULL || strlen(szMerchantID) <= 0 || strlen(szMerchantID) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetMerchantID() ERROR !!");

			if (szMerchantID == NULL)
			{
				inLogPrintf(AT, "szMerchantID == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szMerchantID length = (%d)", (int)strlen(szMerchantID));
				inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srHDTRec.szMerchantID[0], &szMerchantID[0], strlen(szMerchantID));

        return (VS_SUCCESS);
}

/*
Function        :inGetTerminalID
Date&Time       :
Describe        :
*/
int inGetTerminalID(char* szTerminalID)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTerminalID == NULL || strlen(srHDTRec.szTerminalID) <= 0 || strlen(srHDTRec.szTerminalID) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                	inLogPrintf(AT, "inGetTerminalID() ERROR !!");

                	if (szTerminalID == NULL)
                	{
                	        inLogPrintf(AT, "szTerminalID == NULL");
                	}
                	else
                	{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szTerminalID length = (%d)", (int)strlen(srHDTRec.szTerminalID));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szTerminalID[0], &srHDTRec.szTerminalID[0], strlen(srHDTRec.szTerminalID));

        return (VS_SUCCESS);
}

/*
Function        :inSetTerminalID
Date&Time       :
Describe        :
*/
int inSetTerminalID(char* szTerminalID)
{
        memset(srHDTRec.szTerminalID, 0x00, sizeof(srHDTRec.szTerminalID));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTerminalID == NULL || strlen(szTerminalID) <= 0 || strlen(szTerminalID) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetTerminalID() ERROR !! ");

			if (szTerminalID == NULL)
			{
				inLogPrintf(AT, "szTerminalID == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szTerminalID length = (%d)", (int)strlen(szTerminalID));
				inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srHDTRec.szTerminalID[0], &szTerminalID[0], strlen(szTerminalID));

        return (VS_SUCCESS);
}

/*
Function        :inGetCommunicationIndex
Date&Time       :
Describe        :
*/
int inGetCommunicationIndex(char* szCommunicationIndex)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCommunicationIndex == NULL || strlen(srHDTRec.szCommunicationIndex) < 0 || strlen(srHDTRec.szCommunicationIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                	inLogPrintf(AT, "inGetCommunicationIndex() ERROR !!");

                        if (szCommunicationIndex == NULL)
                        {
				inLogPrintf(AT, "szCommunicationIndex == NULL");
                        }
                        else if (strlen(srHDTRec.szCommunicationIndex) < 0 || strlen(srHDTRec.szCommunicationIndex) > 2)
                        {
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szCommunicationIndex length = (%d) ", (int)strlen(srHDTRec.szCommunicationIndex));
				inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCommunicationIndex[0], &srHDTRec.szCommunicationIndex[0], strlen(srHDTRec.szCommunicationIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetCommunicationIndex
Date&Time       :
Describe        :
*/
int inSetCommunicationIndex(char* szCommunicationIndex)
{
        memset(srHDTRec.szCommunicationIndex, 0x00, sizeof(srHDTRec.szCommunicationIndex));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCommunicationIndex == NULL || strlen(szCommunicationIndex) <= 0 || strlen(szCommunicationIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetCommunicationIndex() ERROR !!");

			if (szCommunicationIndex == NULL)
			{
				inLogPrintf(AT, "szCommunicationIndex == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szCommunicationIndex length = (%d)", (int)strlen(szCommunicationIndex));
				inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srHDTRec.szCommunicationIndex[0], &szCommunicationIndex[0], strlen(szCommunicationIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetTransFunc
Date&Time       :
Describe        :
*/
int inGetTransFunc(char* szTransFunc)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTransFunc == NULL || strlen(srHDTRec.szTransFunc) <= 0 || strlen(srHDTRec.szTransFunc) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                	inLogPrintf(AT, "inGetTransFunc() ERROR !!");

                	if (szTransFunc == NULL)
                	{
                	        inLogPrintf(AT, "szTransFunc == NULL");
                	}
                	else
                	{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szTransFunc length = (%d)", (int)strlen(srHDTRec.szTransFunc));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szTransFunc[0], &srHDTRec.szTransFunc[0], strlen(srHDTRec.szTransFunc));

        return (VS_SUCCESS);
}

/*
Function        :inSetTransFunc
Date&Time       :
Describe        :
*/
int inSetTransFunc(char* szTransFunc)
{
        memset(srHDTRec.szTransFunc, 0x00, sizeof(srHDTRec.szTransFunc));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTransFunc == NULL || strlen(szTransFunc) <= 0 || strlen(szTransFunc) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetTransFunc() ERROR !!");

			if (szTransFunc == NULL)
			{
				inLogPrintf(AT, "szTransFunc == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szTransFunc length = (%d)", (int)strlen(szTransFunc));
				inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srHDTRec.szTransFunc[0], &szTransFunc[0], strlen(szTransFunc));

        return (VS_SUCCESS);
}

/*
Function        :inGetManualKeyin
Date&Time       :
Describe        :
*/
int inGetManualKeyin(char* szManualKeyin)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szManualKeyin == NULL || strlen(srHDTRec.szManualKeyin) <= 0 || strlen(srHDTRec.szManualKeyin) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                	inLogPrintf(AT, "inGetManualKeyin() ERROR !!");

                	if (szManualKeyin == NULL)
                	{
                	        inLogPrintf(AT, "szManualKeyin == NULL");
                	}
                	else
                	{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szManualKeyin length = (%d)", (int)strlen(srHDTRec.szManualKeyin));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szManualKeyin[0], &srHDTRec.szManualKeyin[0], strlen(srHDTRec.szManualKeyin));

        return (VS_SUCCESS);
}

/*
Function        :inSetManualKeyin
Date&Time       :
Describe        :
*/
int inSetManualKeyin(char* szManualKeyin)
{
        memset(srHDTRec.szManualKeyin, 0x00, sizeof(srHDTRec.szManualKeyin));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (strlen(szManualKeyin) <= 0 || strlen(szManualKeyin) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetManualKeyin() ERROR !!");

			if (szManualKeyin == NULL)
			{
				inLogPrintf(AT, "szTransFunc == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szManualKeyin length = (%d)", (int)strlen(szManualKeyin));
				inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srHDTRec.szManualKeyin[0], &szManualKeyin[0], strlen(szManualKeyin));

        return (VS_SUCCESS);
}

/*
Function        :inGetCallBankEnable
Date&Time       :
Describe        :
*/
int inGetCallBankEnable(char* szCallBankEnable)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCallBankEnable == NULL || strlen(srHDTRec.szCallBankEnable) <= 0 || strlen(srHDTRec.szCallBankEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                	inLogPrintf(AT, "inGetCallBankEnable() ERROR !!");

                	if (szCallBankEnable == NULL)
                	{
                	        inLogPrintf(AT, "szCallBankEnable == NULL");
                	}
                	else
                	{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szCallBankEnable length = (%d)", (int)strlen(srHDTRec.szCallBankEnable));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szCallBankEnable[0], &srHDTRec.szCallBankEnable[0], strlen(srHDTRec.szCallBankEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetCallBankEnable
Date&Time       :
Describe        :
*/
int inSetCallBankEnable(char* szCallBankEnable)
{
        memset(srHDTRec.szCallBankEnable, 0x00, sizeof(srHDTRec.szCallBankEnable));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCallBankEnable == NULL || strlen(szCallBankEnable) <= 0 || strlen(szCallBankEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetCallBankEnable() ERROR !!");

			if (szCallBankEnable == NULL)
			{
				inLogPrintf(AT, "szCallBankEnable == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szCallBankEnable length = (%d)", (int)strlen(szCallBankEnable));
				inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srHDTRec.szCallBankEnable[0], &szCallBankEnable[0], strlen(szCallBankEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetTipPercent
Date&Time       :
Describe        :
*/
int inGetTipPercent(char* szTipPercent)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTipPercent == NULL || strlen(srHDTRec.szTipPercent) <= 0 || strlen(srHDTRec.szTipPercent) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                	inLogPrintf(AT, "inGetTipPercent() ERROR !!");

                	if (szTipPercent == NULL)
                	{
                		inLogPrintf(AT, "szTipPercent == NULL");
                	}
                	else
                	{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szTipPercent length = (%d)", (int)strlen(srHDTRec.szTipPercent));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szTipPercent[0], &srHDTRec.szTipPercent[0], strlen(srHDTRec.szTipPercent));

        return (VS_SUCCESS);
}

/*
Function        :inSetTipPercent
Date&Time       :
Describe        :
*/
int inSetTipPercent(char* szTipPercent)
{
        memset(srHDTRec.szTipPercent, 0x00, sizeof(srHDTRec.szTipPercent));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTipPercent == NULL || strlen(szTipPercent) <= 0 || strlen(szTipPercent) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetTipPercent() ERROR !!");

			if (szTipPercent == NULL)
			{
				inLogPrintf(AT, "szTipPercent == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szTipPercent length = (%d)", (int)strlen(szTipPercent));
				inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srHDTRec.szTipPercent[0], &szTipPercent[0], strlen(szTipPercent));

        return (VS_SUCCESS);
}

/*
Function        :inHDT_Edit_HDT_Table
Date&Time       :2017/5/15 下午 4:08
Describe        :
*/
int inHDT_Edit_HDT_Table(void)
{
	TABLE_GET_SET_TABLE HDT_FUNC_TABLE[] =
	{
		{"szHostIndex"			,inGetHostIndex			,inSetHostIndex			},
		{"szHostEnable"			,inGetHostEnable		,inSetHostEnable		},
		{"szHostLabel"			,inGetHostLabel			,inSetHostLabel			},
		{"szMerchantID"			,inGetMerchantID		,inSetMerchantID		},
		{"szTerminalID"			,inGetTerminalID		,inSetTerminalID		},
		{"szCommunicationIndex"		,inGetCommunicationIndex	,inSetCommunicationIndex	},
		{"szTransFunc"			,inGetTransFunc			,inSetTransFunc			},
		{"szManualKeyin"		,inGetManualKeyin		,inSetManualKeyin		},
		{"szCallBankEnable"		,inGetCallBankEnable		,inSetCallBankEnable		},
		{"szTipPercent"			,inGetTipPercent		,inSetTipPercent		},
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
	inDISP_ChineseFont_Color("是否更改HDT", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
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
	inLoadHDTRec(inRecordCnt);
	
	inFunc_Edit_Table_Tag(HDT_FUNC_TABLE);
	inSaveHDTRec(inRecordCnt);
	
	return	(VS_SUCCESS);
}

int inHDT_Test1(void)
{
//        inLoadHDTRec(0);
//        inSetHostIndex("QQ");
//        inSetHostEnable("Y");
//        inSetHostLabel("TT");
//        inSetMerchantID("Q23456789012345");
//        inSetTerminalID("P2109876");
//        inSetCommunicationIndex("01");
//        inSetPasswordIndex("05");
//        inSetTransFunc("11111111111111111111");
//        inSetManualKeyin("2");
//        inSetCallBankEnable("4");
//        inSetTipPercent("999");
//        inSaveHDTRec(0);
//
//        inLoadHDTRec(5);
//        inSetHostIndex("QQ");
//        inSetHostEnable("Y");
//        inSetHostLabel("TT");
//        inSetMerchantID("Q23456789012345");
//        inSetTerminalID("P2109876");
//        inSetCommunicationIndex("01");
//        inSetPasswordIndex("05");
//        inSetTransFunc("11111111111111111111");
//        inSetManualKeyin("2");
//        inSetCallBankEnable("4");
//        inSetTipPercent("999");
//        inSaveHDTRec(5);


        inLoadHDTRec(17);
        inSetHostIndex("QQ");
        inSetHostEnable("Y");
        inSetHostLabel("TT");
        inSetMerchantID("Q23456789012345");
        inSetTerminalID("P2109876");
        inSetCommunicationIndex("01");
        inSetTransFunc("11111111111111111111");
        inSetManualKeyin("2");
        inSetCallBankEnable("4");
        inSetTipPercent("999");
        inSaveHDTRec(17);

        return (VS_SUCCESS);
}


int inHDT_Test2(void)
{

        if (inLoadHDTRec(0) == VS_SUCCESS)
        {

                inSetHostIndex("02");
                inSaveHDTRec(0);
        }
        if (inLoadHDTRec(1)== VS_SUCCESS)
        {
                inSetHostEnable("Y");
                inSaveHDTRec(1);
        }

        if (inLoadHDTRec(2))
        {
                inSetHostLabel("CTCB");
                inSaveHDTRec(2);
        }
        if (inLoadHDTRec(3))
        {
                inSetMerchantID("123456789012345");
                inSaveHDTRec(3);
        }
        if (inLoadHDTRec(4))
        {
                inSetTerminalID("12345678");
                inSaveHDTRec(4);
        }
        if (inLoadHDTRec(5))
        {
                inSetCommunicationIndex("01");
                inSaveHDTRec(5);
        }
        if (inLoadHDTRec(6))
        {
                inSetTransFunc("ABCDEFGHIJKLMNOPQRST");
                inSaveHDTRec(7);
        }
        if (inLoadHDTRec(7))
        {
                inSetManualKeyin("2");
                inSetCallBankEnable("4");
                inSetTipPercent("200");
                inSaveHDTRec(8);
        }
        return (VS_SUCCESS);
}
