#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/FUNCTION/File.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "NCCCtmsFTP.h"

static  TMSFTP_REC	srTMSFTPRec;	/* construct TMSFTP record */
extern  int		ginDebug;       /* Debug使用 extern */

/*
Function        :inLoadTMSFTPRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀TMSFTP檔案，inTMSFTPRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadTMSFTPRec(int inTMSFTPRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆TMSFTP檔案 */
        char            szTMSFTPRec[_SIZE_TMSFTP_REC_ + 1];           /* 暫存, 放各個欄位檔案 */
        char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnTMSFTPLength = 0;                        /* TMSFTP總長度 */
        long            lnReadLength;                           /* 記錄每次要從TMSFTP.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;	                /* inRec記錄讀到第幾筆, i為目前從TMSFTP讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */
       
        /* inLoadTMSFTPRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadTMSFTPRec(%d) START!!", inTMSFTPRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 判斷傳進來的inTMSFTPRec是否小於零 大於等於零才是正確值(防呆) */
        if (inTMSFTPRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inTMSFTPRec < 0:(index = %d) ERROR!!", inTMSFTPRec);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open TMSFTP.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_TMSFTP_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnTMSFTPLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_TMSFTP_FILE_NAME_);
        
        if (lnTMSFTPLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnTMSFTPLength + 1);
        uszTemp = malloc(lnTMSFTPLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnTMSFTPLength + 1);
        memset(uszTemp, 0x00, lnTMSFTPLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnTMSFTPLength;

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
         *i為目前從TMSFTP讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnTMSFTPLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到TMSFTP的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnTMSFTPLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        /* 因為inTMSFTP_Rec的index從0開始，所以inTMSFTP_Rec要+1 */
                        if (inRec == (inTMSFTPRec + 1))
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
         * 如果沒有inTMSFTPRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inTMSFTPRec + 1) || inSearchResult == -1)
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
        memset(&srTMSFTPRec, 0x00, sizeof(srTMSFTPRec));
        /*
         * 以下pattern為存入TMSFTP_Rec
         * i為TMSFTP的第幾個字元
         * 存入TMSFTP_Rec
         */
        i = 0;


        /* 01_TMS IP Address */
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSFTP unpack ERROR.");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)
        {
                memcpy(&srTMSFTPRec.szFTPIPAddress[0], &szTMSFTPRec[0], k - 1);
        }

        /* 02_TMS Port Number */
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSFTP unpack ERROR");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)
        {
                memcpy(&srTMSFTPRec.szFTPPortNum[0], &szTMSFTPRec[0], k - 1);
        }

        /* 03_TMS Phone Number */
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSFTP unpack ERROR");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)
        {
                memcpy(&srTMSFTPRec.szFTPID[0], &szTMSFTPRec[0], k - 1);
        }

        /* 04_結帳後更新參數的開關 */
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSFTP unpack ERROR");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)
        {
                memcpy(&srTMSFTPRec.szFTPPW[0], &szTMSFTPRec[0], k - 1);
        }
        
        /* 05_是否允許自動下載 */
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSFTP unpack ERROR");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)
        {
                memcpy(&srTMSFTPRec.szFTPAutoDownloadFlag[0], &szTMSFTPRec[0], k - 1);
        }
        
        /* 06_允許端末機下載的日期(時間) */
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSFTP unpack ERROR");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srTMSFTPRec.szFTPStartDownloadDateTime[0], &szTMSFTPRec[0], k - 1);
        }

        /* 07_端末機參數異動日期時間 */
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSFTP unpack ERROR");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)
        {
                memcpy(&srTMSFTPRec.szFTPTermParemeterDateTime[0], &szTMSFTPRec[0], k - 1);
        }
        
        /* 08_端末機是否須檢查有無帳務才可更新 */
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSFTP unpack ERROR");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)
        {
                memcpy(&srTMSFTPRec.szFTPEffectiveCloseBatch[0], &szTMSFTPRec[0], k - 1);
        }
        
        /* 09_作業批號 */
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSFTP unpack ERROR");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)
        {
                memcpy(&srTMSFTPRec.szFTPBatchNum[0], &szTMSFTPRec[0], k - 1);
        }
	
	/* 10_szFTPInquiryResponseCode */
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSFTP unpack ERROR");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)
        {
                memcpy(&srTMSFTPRec.szFTPInquiryResponseCode[0], &szTMSFTPRec[0], k - 1);
        }
	
	/* 11_szFTPDownloadResponseCode */
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSFTP unpack ERROR");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)
        {
                memcpy(&srTMSFTPRec.szFTPDownloadResponseCode[0], &szTMSFTPRec[0], k - 1);
        }
	
	/* 12_szDownloadCategory */
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSFTP unpack ERROR");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)
        {
                memcpy(&srTMSFTPRec.szFTPDownloadCategory[0], &szTMSFTPRec[0], k - 1);
        }
	
	/* 12_FTPEffectiveReportBit */
        /* 初始化 */
        memset(szTMSFTPRec, 0x00, sizeof(szTMSFTPRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFTPRec[k ++] = uszReadData[i ++];
                if (szTMSFTPRec[k - 1] == 0x2C	||
		    szTMSFTPRec[k - 1] == 0x0D	||
		    szTMSFTPRec[k - 1] == 0x0A	||
		    szTMSFTPRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFTPLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSFTP unpack ERROR");
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
        if (szTMSFTPRec[0] != 0x2C	&&
	    szTMSFTPRec[0] != 0x0D	&&
	    szTMSFTPRec[0] != 0x0A	&&
	    szTMSFTPRec[0] != 0x00)
        {
                memcpy(&srTMSFTPRec.szFTPEffectiveReportBit[0], &szTMSFTPRec[0], k - 1);
        }
	
        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadTMSFTPRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadTMSFTPRec(%d) END!!", inTMSFTPRec);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveTMSFTPRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSaveTMSFTPRec(int inTMSFTPRec)
{
        unsigned long   uldat_Handle;                           /* FILE Handle */
        unsigned long   ulbak_Handle;                           /* FILE Handle */
        int	        inPackCount = 0;                        /* uszWriteBuff_Record的index */
        int	        inRecIndex = 0;                         /* uszRead_Total_Buff的index */
        int             inTMSFTP_Total_Rec = 0;                    /* TMSFTP.dat的總筆數 */
        int             inRetVal;
        int             i = 0, j = 0;
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long	        lnReadLength = 0;                       /* 每次要從TMSFTP.dat讀多少byte出來 */
        long            lnTMSFTPLength = 0;                        /* TMSFTP.dat檔案總長度 */
        unsigned char	*uszRead_Total_Buff;
        unsigned char   *uszWriteBuff_Record, *uszWriteBuff_Org;


        /* inSaveTMSFTPRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveTMSFTPRec(%d)_START!!", inTMSFTPRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 防呆先刪除TMSFTP.bak */
        inFILE_Delete((unsigned char *)_TMSFTP_FILE_NAME_BAK_);

        /* 新建TMSFTP.bak檔案 */
        inRetVal = inFILE_Create(&ulbak_Handle, (unsigned char *)_TMSFTP_FILE_NAME_BAK_);

        if (inRetVal != VS_SUCCESS)
        {
                inFILE_Close(&ulbak_Handle);

                return (VS_ERROR);
        }

        /* 開啟原檔案TMSFTP.dat */
        inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_TMSFTP_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                /* TMSFTP.dat開檔失敗 ，不用關檔TMSFTP.dat */
                /* TMSFTP.bak仍要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_TMSFTP_FILE_NAME_BAK_);

                return(VS_ERROR);
        }

        /* 取得TMSFTP.dat檔案大小 */
        lnTMSFTPLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *)_TMSFTP_FILE_NAME_);

        /* 取得檔案大小失敗 */
        if (lnTMSFTPLength == VS_ERROR)
        {
                /* TMSFTP.bak和TMSFTP.dat要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Close(&uldat_Handle);
                inFILE_Delete((unsigned char *)_TMSFTP_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        uszWriteBuff_Record = malloc(_SIZE_TMSFTP_REC_ + _SIZE_TMSFTP_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_TMSFTP_REC_ + _SIZE_TMSFTP_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原TMSFTP.dat的每筆Record 初始化 */
        uszWriteBuff_Org = malloc(_SIZE_TMSFTP_REC_ + _SIZE_TMSFTP_COMMA_0D0A_);
        memset(uszWriteBuff_Org, 0x00, _SIZE_TMSFTP_REC_ + _SIZE_TMSFTP_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存TMSFTP.dat全部資料 */
        uszRead_Total_Buff = malloc(lnTMSFTPLength + 1);
        memset(uszRead_Total_Buff, 0x00, lnTMSFTPLength + 1);

        inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

        /* FTPIPAddress */
        memcpy(&uszWriteBuff_Record[0], &srTMSFTPRec.szFTPIPAddress[0], strlen(srTMSFTPRec.szFTPIPAddress));
        inPackCount += strlen(srTMSFTPRec.szFTPIPAddress);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* FTPPortNum */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTMSFTPRec.szFTPPortNum[0], strlen(srTMSFTPRec.szFTPPortNum));
        inPackCount += strlen(srTMSFTPRec.szFTPPortNum);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* FTPID */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTMSFTPRec.szFTPID[0], strlen(srTMSFTPRec.szFTPID));
        inPackCount += strlen(srTMSFTPRec.szFTPID);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
        
        /* FTPPW */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTMSFTPRec.szFTPPW[0], strlen(srTMSFTPRec.szFTPPW));
        inPackCount += strlen(srTMSFTPRec.szFTPPW);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* szFTPAutoDownloadFlag */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTMSFTPRec.szFTPAutoDownloadFlag[0], strlen(srTMSFTPRec.szFTPAutoDownloadFlag));
        inPackCount += strlen(srTMSFTPRec.szFTPAutoDownloadFlag);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* szFTPStartDownloadDateTime */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTMSFTPRec.szFTPStartDownloadDateTime[0], strlen(srTMSFTPRec.szFTPStartDownloadDateTime));
        inPackCount += strlen(srTMSFTPRec.szFTPStartDownloadDateTime);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* szFTPTermParemeterDateTime */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTMSFTPRec.szFTPTermParemeterDateTime[0], strlen(srTMSFTPRec.szFTPTermParemeterDateTime));
        inPackCount += strlen(srTMSFTPRec.szFTPTermParemeterDateTime);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* szFTPEffectiveCloseBatch */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTMSFTPRec.szFTPEffectiveCloseBatch[0], strlen(srTMSFTPRec.szFTPEffectiveCloseBatch));
        inPackCount += strlen(srTMSFTPRec.szFTPEffectiveCloseBatch);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* szFTPBatchNum */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTMSFTPRec.szFTPBatchNum[0], strlen(srTMSFTPRec.szFTPBatchNum));
        inPackCount += strlen(srTMSFTPRec.szFTPBatchNum);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szFTPInquiryResponseCode */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTMSFTPRec.szFTPInquiryResponseCode[0], strlen(srTMSFTPRec.szFTPInquiryResponseCode));
        inPackCount += strlen(srTMSFTPRec.szFTPInquiryResponseCode);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szFTPDownloadResponseCode */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTMSFTPRec.szFTPDownloadResponseCode[0], strlen(srTMSFTPRec.szFTPDownloadResponseCode));
        inPackCount += strlen(srTMSFTPRec.szFTPDownloadResponseCode);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szFTPDownloadCategory */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTMSFTPRec.szFTPDownloadCategory[0], strlen(srTMSFTPRec.szFTPDownloadCategory));
        inPackCount += strlen(srTMSFTPRec.szFTPDownloadCategory);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
        /* szFTPEffectiveReportBit */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTMSFTPRec.szFTPEffectiveReportBit[0], strlen(srTMSFTPRec.szFTPEffectiveReportBit));
        inPackCount += strlen(srTMSFTPRec.szFTPEffectiveReportBit);
    
        /* 最後的data不用逗號 */
        /* 補上換行符號 */

        /* 0D是移至行首 */
        uszWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        uszWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;

        /* 讀TMSFTP.dat檔案存到uszRead_Total_Buff */
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnTMSFTPLength;

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
                                        inFILE_Delete((unsigned char *)_TMSFTP_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_TMSFTP_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_TMSFTP_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        /* 防呆 總record數量小於要存取inTMSFTPRec Return ERROR */
        /* 算總Record數 */
        for (i = 0; i < (lnTMSFTPLength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (uszRead_Total_Buff[i] == 0x0A)
                        inTMSFTP_Total_Rec++;
        }

        /* 防呆 總record數量小於要存取inTMSFTPRec Return ERROR */
        if ((inTMSFTPRec + 1) > inTMSFTP_Total_Rec)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "No data or Index ERROR");
                }

                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_TMSFTP_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                return (VS_ERROR);
        }

        /* 記錄ReadTotalBuffer的陣列位置 */
        inRecIndex = 0;
        /* 用inTMSFTPRec決定要先存幾筆Record到TMSFTP.bak，ex:inTMSFTPRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inTMSFTPRec; i ++)
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
                                        inFILE_Delete((unsigned char *)_TMSFTP_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_TMSFTP_FILE_NAME_BAK_);

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

        /* 存組好的該TMSFTPRecord 到 TMSFTP.bak */
        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_TMSFTP_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_TMSFTP_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原TMSFTP.dat Record 到 TMSFTP.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
        /* inRecIndex要指到剩下還沒存進bak檔的第一個記憶體位置 */
        inTMSFTPRec = inTMSFTPRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < lnTMSFTPLength; i++)
        {
                if (uszRead_Total_Buff[i] == 0x0A)
                {
                        j++;

                        if (j == inTMSFTPRec)
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
                /* 接續存原TMSFTP.dat的Record */
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
                                inFILE_Delete((unsigned char *)_TMSFTP_FILE_NAME_BAK_);

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
                                inFILE_Delete((unsigned char *)_TMSFTP_FILE_NAME_BAK_);

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

        /* 刪除原TMSFTP.dat */
        if (inFILE_Delete((unsigned char *)_TMSFTP_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* 將TMSFTP.bak改名字為TMSFTP.dat取代原檔案 */
        if (inFILE_Rename((unsigned char *)_TMSFTP_FILE_NAME_BAK_, (unsigned char *)_TMSFTP_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* inSaveTMSFTPRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveTMSFTPRec(%d) END!!", inTMSFTPRec - 1);
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
Function        :inGetFTPIPAddress
Date&Time       :
Describe        :
*/
int inGetFTPIPAddress(char* szFTPIPAddress)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPIPAddress == NULL || strlen(srTMSFTPRec.szFTPIPAddress) <= 0 || strlen(srTMSFTPRec.szFTPIPAddress) > 16)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetFTPIPAddress() ERROR !!");
                        
			if (szFTPIPAddress == NULL)
                        {
                                inLogPrintf(AT, "szFTPIPAddress == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPIPAddress length = (%d)", (int)strlen(srTMSFTPRec.szFTPIPAddress));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPIPAddress[0], &srTMSFTPRec.szFTPIPAddress[0], strlen(srTMSFTPRec.szFTPIPAddress));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPIPAddress
Date&Time       :
Describe        :
*/
int inSetFTPIPAddress(char* szFTPIPAddress)
{
        memset(srTMSFTPRec.szFTPIPAddress, 0x00, sizeof(srTMSFTPRec.szFTPIPAddress));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szFTPIPAddress == NULL || strlen(szFTPIPAddress) <= 0 || strlen(szFTPIPAddress) > 16)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetFTPIPAddress() ERROR !!");
                        if (szFTPIPAddress == NULL)
                        {
                                inLogPrintf(AT, "szFTPIPAddress == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPIPAddress length = (%d)", (int)strlen(szFTPIPAddress));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTMSFTPRec.szFTPIPAddress[0], &szFTPIPAddress[0], strlen(szFTPIPAddress));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPPortNum
Date&Time       :
Describe        :
*/
int inGetFTPPortNum(char* szFTPPortNum)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPPortNum == NULL || strlen(srTMSFTPRec.szFTPPortNum) <= 0 || strlen(srTMSFTPRec.szFTPPortNum) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetFTPPortNum() ERROR !!");

                        if (szFTPPortNum == NULL)
                        {
                                inLogPrintf(AT, "szFTPPortNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPPortNum length = (%d)", (int)strlen(srTMSFTPRec.szFTPPortNum));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPPortNum[0], &srTMSFTPRec.szFTPPortNum[0], strlen(srTMSFTPRec.szFTPPortNum));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPPortNum
Date&Time       :
Describe        :
*/
int inSetFTPPortNum(char* szFTPPortNum)
{
        memset(srTMSFTPRec.szFTPPortNum, 0x00, sizeof(srTMSFTPRec.szFTPPortNum));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPPortNum == NULL || strlen(szFTPPortNum) <= 0 || strlen(szFTPPortNum) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetFTPPortNum() ERROR !!");

                        if (szFTPPortNum == NULL)
                        {
                                inLogPrintf(AT, "szFTPPortNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPPortNum length = (%d)", (int)strlen(szFTPPortNum));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTMSFTPRec.szFTPPortNum[0], &szFTPPortNum[0], strlen(szFTPPortNum));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPID
Date&Time       :
Describe        :
*/
int inGetFTPID(char* szFTPID)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPID == NULL || strlen(srTMSFTPRec.szFTPID) <= 0 || strlen(srTMSFTPRec.szFTPID) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetFTPID() ERROR !!");

                        if (szFTPID == NULL)
                        {
                                inLogPrintf(AT, "szFTPID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPID length = (%d)", (int)strlen(srTMSFTPRec.szFTPID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPID[0], &srTMSFTPRec.szFTPID[0], strlen(srTMSFTPRec.szFTPID));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPID
Date&Time       :
Describe        :
*/
int inSetFTPID(char* szFTPID)
{
        memset(srTMSFTPRec.szFTPID, 0x00, sizeof(srTMSFTPRec.szFTPID));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPID == NULL || strlen(szFTPID) <= 0 || strlen(szFTPID) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetFTPID() ERROR !!");

                        if (szFTPID == NULL)
                        {
                                inLogPrintf(AT, "szFTPID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPID length = (%d)", (int)strlen(szFTPID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTMSFTPRec.szFTPID[0], &szFTPID[0], strlen(szFTPID));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPPW
Date&Time       :
Describe        :
*/
int inGetFTPPW(char* szFTPPW)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPPW == NULL || strlen(srTMSFTPRec.szFTPPW) <= 0 || strlen(srTMSFTPRec.szFTPPW) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetFTPPW() ERROR !!");

                        if (szFTPPW == NULL)
                        {
                                inLogPrintf(AT, "szFTPPW == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPPW length = (%d)", (int)strlen(srTMSFTPRec.szFTPPW));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPPW[0], &srTMSFTPRec.szFTPPW[0], strlen(srTMSFTPRec.szFTPPW));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPPW
Date&Time       :
Describe        :
*/
int inSetFTPPW(char* szFTPPW)
{
        memset(srTMSFTPRec.szFTPPW, 0x00, sizeof(srTMSFTPRec.szFTPPW));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPPW == NULL || strlen(szFTPPW) <= 0 || strlen(szFTPPW) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetFTPPW() ERROR !!");

                        if (szFTPPW == NULL)
                        {
                                inLogPrintf(AT, "szFTPPW == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPPW length = (%d)", (int)strlen(szFTPPW));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTMSFTPRec.szFTPPW[0], &szFTPPW[0], strlen(szFTPPW));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPAutoDownloadFlag
Date&Time       :2018/6/27 上午 11:22
Describe        :
*/
int inGetFTPAutoDownloadFlag(char* szFTPAutoDownloadFlag)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPAutoDownloadFlag == NULL || strlen(srTMSFTPRec.szFTPAutoDownloadFlag) <= 0 || strlen(srTMSFTPRec.szFTPAutoDownloadFlag) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetFTPAutoDownloadFlag() ERROR !!");

                        if (szFTPAutoDownloadFlag == NULL)
                        {
                                inLogPrintf(AT, "szFTPAutoDownloadFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPAutoDownloadFlag length = (%d)", (int)strlen(srTMSFTPRec.szFTPAutoDownloadFlag));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPAutoDownloadFlag[0], &srTMSFTPRec.szFTPAutoDownloadFlag[0], strlen(srTMSFTPRec.szFTPAutoDownloadFlag));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPAutoDownloadFlag
Date&Time       :2018/6/27 上午 11:22
Describe        :
*/
int inSetFTPAutoDownloadFlag(char* szFTPAutoDownloadFlag)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPAutoDownloadFlag == NULL || strlen(szFTPAutoDownloadFlag) <= 0 || strlen(szFTPAutoDownloadFlag) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetFTPAutoDownloadFlag() ERROR !!");

                        if (szFTPAutoDownloadFlag == NULL)
                        {
                                inLogPrintf(AT, "szFTPAutoDownloadFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPAutoDownloadFlag length = (%d)", (int)strlen(szFTPAutoDownloadFlag));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSFTPRec.szFTPAutoDownloadFlag, 0x00, sizeof(srTMSFTPRec.szFTPAutoDownloadFlag));
        memcpy(&srTMSFTPRec.szFTPAutoDownloadFlag[0], &szFTPAutoDownloadFlag[0], strlen(szFTPAutoDownloadFlag));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPStartDownloadDateTime
Date&Time       :2018/6/27 上午 11:23
Describe        :
*/
int inGetFTPStartDownloadDateTime(char* szFTPStartDownloadDateTime)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPStartDownloadDateTime == NULL || strlen(srTMSFTPRec.szFTPStartDownloadDateTime) <= 0 || strlen(srTMSFTPRec.szFTPStartDownloadDateTime) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetFTPStartDownloadDateTime() ERROR !!");

                        if (szFTPStartDownloadDateTime == NULL)
                        {
                                inLogPrintf(AT, "szFTPStartDownloadDateTime == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPStartDownloadDateTime length = (%d)", (int)strlen(srTMSFTPRec.szFTPStartDownloadDateTime));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPStartDownloadDateTime[0], &srTMSFTPRec.szFTPStartDownloadDateTime[0], strlen(srTMSFTPRec.szFTPStartDownloadDateTime));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPStartDownloadDateTime
Date&Time       :2018/6/27 上午 11:23
Describe        :
*/
int inSetFTPStartDownloadDateTime(char* szFTPStartDownloadDateTime)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPStartDownloadDateTime == NULL || strlen(szFTPStartDownloadDateTime) <= 0 || strlen(szFTPStartDownloadDateTime) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetFTPStartDownloadDateTime() ERROR !!");

                        if (szFTPStartDownloadDateTime == NULL)
                        {
                                inLogPrintf(AT, "szFTPStartDownloadDateTime == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPStartDownloadDateTime length = (%d)", (int)strlen(szFTPStartDownloadDateTime));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSFTPRec.szFTPStartDownloadDateTime, 0x00, sizeof(srTMSFTPRec.szFTPStartDownloadDateTime));
        memcpy(&srTMSFTPRec.szFTPStartDownloadDateTime[0], &szFTPStartDownloadDateTime[0], strlen(szFTPStartDownloadDateTime));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPTermParemeterDateTime
Date&Time       :2018/6/27 上午 11:25
Describe        :
*/
int inGetFTPTermParemeterDateTime(char* szFTPTermParemeterDateTime)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPTermParemeterDateTime == NULL || strlen(srTMSFTPRec.szFTPTermParemeterDateTime) <= 0 || strlen(srTMSFTPRec.szFTPTermParemeterDateTime) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetFTPTermParemeterDateTime() ERROR !!");

                        if (szFTPTermParemeterDateTime == NULL)
                        {
                                inLogPrintf(AT, "szFTPTermParemeterDateTime == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPTermParemeterDateTime length = (%d)", (int)strlen(srTMSFTPRec.szFTPTermParemeterDateTime));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPTermParemeterDateTime[0], &srTMSFTPRec.szFTPTermParemeterDateTime[0], strlen(srTMSFTPRec.szFTPTermParemeterDateTime));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPTermParemeterDateTime
Date&Time       :2018/6/27 上午 11:25
Describe        :
*/
int inSetFTPTermParemeterDateTime(char* szFTPTermParemeterDateTime)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPTermParemeterDateTime == NULL || strlen(szFTPTermParemeterDateTime) <= 0 || strlen(szFTPTermParemeterDateTime) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetFTPTermParemeterDateTime() ERROR !!");

                        if (szFTPTermParemeterDateTime == NULL)
                        {
                                inLogPrintf(AT, "szFTPTermParemeterDateTime == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPTermParemeterDateTime length = (%d)", (int)strlen(szFTPTermParemeterDateTime));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSFTPRec.szFTPTermParemeterDateTime, 0x00, sizeof(srTMSFTPRec.szFTPTermParemeterDateTime));
        memcpy(&srTMSFTPRec.szFTPTermParemeterDateTime[0], &szFTPTermParemeterDateTime[0], strlen(szFTPTermParemeterDateTime));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPEffectiveCloseBatch
Date&Time       :2018/6/27 上午 11:25
Describe        :
*/
int inGetFTPEffectiveCloseBatch(char* szFTPEffectiveCloseBatch)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPEffectiveCloseBatch == NULL || strlen(srTMSFTPRec.szFTPEffectiveCloseBatch) <= 0 || strlen(srTMSFTPRec.szFTPEffectiveCloseBatch) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetFTPEffectiveCloseBatch() ERROR !!");

                        if (szFTPEffectiveCloseBatch == NULL)
                        {
                                inLogPrintf(AT, "szFTPEffectiveCloseBatch == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPEffectiveCloseBatch length = (%d)", (int)strlen(srTMSFTPRec.szFTPEffectiveCloseBatch));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPEffectiveCloseBatch[0], &srTMSFTPRec.szFTPEffectiveCloseBatch[0], strlen(srTMSFTPRec.szFTPEffectiveCloseBatch));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPEffectiveCloseBatch
Date&Time       :2018/6/27 上午 11:25
Describe        :
*/
int inSetFTPEffectiveCloseBatch(char* szFTPEffectiveCloseBatch)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPEffectiveCloseBatch == NULL || strlen(szFTPEffectiveCloseBatch) <= 0 || strlen(szFTPEffectiveCloseBatch) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetFTPEffectiveCloseBatch() ERROR !!");

                        if (szFTPEffectiveCloseBatch == NULL)
                        {
                                inLogPrintf(AT, "szFTPEffectiveCloseBatch == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPEffectiveCloseBatch length = (%d)", (int)strlen(szFTPEffectiveCloseBatch));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSFTPRec.szFTPEffectiveCloseBatch, 0x00, sizeof(srTMSFTPRec.szFTPEffectiveCloseBatch));
        memcpy(&srTMSFTPRec.szFTPEffectiveCloseBatch[0], &szFTPEffectiveCloseBatch[0], strlen(szFTPEffectiveCloseBatch));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPBatchNum
Date&Time       :2018/6/27 上午 11:26
Describe        :
*/
int inGetFTPBatchNum(char* szFTPBatchNum)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPBatchNum == NULL || strlen(srTMSFTPRec.szFTPBatchNum) <= 0 || strlen(srTMSFTPRec.szFTPBatchNum) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetFTPBatchNum() ERROR !!");

                        if (szFTPBatchNum == NULL)
                        {
                                inLogPrintf(AT, "szFTPBatchNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPBatchNum length = (%d)", (int)strlen(srTMSFTPRec.szFTPBatchNum));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPBatchNum[0], &srTMSFTPRec.szFTPBatchNum[0], strlen(srTMSFTPRec.szFTPBatchNum));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPBatchNum
Date&Time       :
Describe        :
*/
int inSetFTPBatchNum(char* szFTPBatchNum)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPBatchNum == NULL || strlen(szFTPBatchNum) <= 0 || strlen(szFTPBatchNum) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetFTPBatchNum() ERROR !!");

                        if (szFTPBatchNum == NULL)
                        {
                                inLogPrintf(AT, "szFTPBatchNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPBatchNum length = (%d)", (int)strlen(szFTPBatchNum));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSFTPRec.szFTPBatchNum, 0x00, sizeof(srTMSFTPRec.szFTPBatchNum));        
        memcpy(&srTMSFTPRec.szFTPBatchNum[0], &szFTPBatchNum[0], strlen(szFTPBatchNum));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPInquiryResponseCode
Date&Time       :2018/6/27 上午 11:26
Describe        :
*/
int inGetFTPInquiryResponseCode(char* szFTPInquiryResponseCode)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPInquiryResponseCode == NULL || strlen(srTMSFTPRec.szFTPInquiryResponseCode) <= 0 || strlen(srTMSFTPRec.szFTPInquiryResponseCode) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetFTPInquiryResponseCode() ERROR !!");

                        if (szFTPInquiryResponseCode == NULL)
                        {
                                inLogPrintf(AT, "szFTPInquiryResponseCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPInquiryResponseCode length = (%d)", (int)strlen(srTMSFTPRec.szFTPInquiryResponseCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPInquiryResponseCode[0], &srTMSFTPRec.szFTPInquiryResponseCode[0], strlen(srTMSFTPRec.szFTPInquiryResponseCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPInquiryResponseCode
Date&Time       :
Describe        :
*/
int inSetFTPInquiryResponseCode(char* szFTPInquiryResponseCode)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPInquiryResponseCode == NULL || strlen(szFTPInquiryResponseCode) <= 0 || strlen(szFTPInquiryResponseCode) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetFTPInquiryResponseCode() ERROR !!");

                        if (szFTPInquiryResponseCode == NULL)
                        {
                                inLogPrintf(AT, "szFTPInquiryResponseCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPInquiryResponseCode length = (%d)", (int)strlen(szFTPInquiryResponseCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSFTPRec.szFTPInquiryResponseCode, 0x00, sizeof(srTMSFTPRec.szFTPInquiryResponseCode));        
        memcpy(&srTMSFTPRec.szFTPInquiryResponseCode[0], &szFTPInquiryResponseCode[0], strlen(szFTPInquiryResponseCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPDownloadResponseCode
Date&Time       :2018/6/27 上午 11:26
Describe        :
*/
int inGetFTPDownloadResponseCode(char* szFTPDownloadResponseCode)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPDownloadResponseCode == NULL || strlen(srTMSFTPRec.szFTPDownloadResponseCode) <= 0 || strlen(srTMSFTPRec.szFTPDownloadResponseCode) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetFTPDownloadResponseCode() ERROR !!");

                        if (szFTPDownloadResponseCode == NULL)
                        {
                                inLogPrintf(AT, "szFTPDownloadResponseCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPDownloadResponseCode length = (%d)", (int)strlen(srTMSFTPRec.szFTPDownloadResponseCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPDownloadResponseCode[0], &srTMSFTPRec.szFTPDownloadResponseCode[0], strlen(srTMSFTPRec.szFTPDownloadResponseCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPDownloadResponseCode
Date&Time       :
Describe        :
*/
int inSetFTPDownloadResponseCode(char* szFTPDownloadResponseCode)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPDownloadResponseCode == NULL || strlen(szFTPDownloadResponseCode) <= 0 || strlen(szFTPDownloadResponseCode) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetFTPDownloadResponseCode() ERROR !!");

                        if (szFTPDownloadResponseCode == NULL)
                        {
                                inLogPrintf(AT, "szFTPDownloadResponseCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPDownloadResponseCode length = (%d)", (int)strlen(szFTPDownloadResponseCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSFTPRec.szFTPDownloadResponseCode, 0x00, sizeof(srTMSFTPRec.szFTPDownloadResponseCode));        
        memcpy(&srTMSFTPRec.szFTPDownloadResponseCode[0], &szFTPDownloadResponseCode[0], strlen(szFTPDownloadResponseCode));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPDownloadCategory
Date&Time       :2018/6/27 上午 11:26
Describe        :
*/
int inGetFTPDownloadCategory(char* szFTPDownloadCategory)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPDownloadCategory == NULL || strlen(srTMSFTPRec.szFTPDownloadCategory) <= 0 || strlen(srTMSFTPRec.szFTPDownloadCategory) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetFTPDownloadCategory() ERROR !!");

                        if (szFTPDownloadCategory == NULL)
                        {
                                inLogPrintf(AT, "szFTPDownloadCategory == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPDownloadCategory length = (%d)", (int)strlen(srTMSFTPRec.szFTPDownloadCategory));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPDownloadCategory[0], &srTMSFTPRec.szFTPDownloadCategory[0], strlen(srTMSFTPRec.szFTPDownloadCategory));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPDownloadCategory
Date&Time       :
Describe        :
*/
int inSetFTPDownloadCategory(char* szFTPDownloadCategory)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPDownloadCategory == NULL || strlen(szFTPDownloadCategory) <= 0 || strlen(szFTPDownloadCategory) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetFTPDownloadCategory() ERROR !!");

                        if (szFTPDownloadCategory == NULL)
                        {
                                inLogPrintf(AT, "szFTPDownloadCategory == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPDownloadCategory length = (%d)", (int)strlen(szFTPDownloadCategory));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSFTPRec.szFTPDownloadCategory, 0x00, sizeof(srTMSFTPRec.szFTPDownloadCategory));        
        memcpy(&srTMSFTPRec.szFTPDownloadCategory[0], &szFTPDownloadCategory[0], strlen(szFTPDownloadCategory));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPEffectiveReportBit
Date&Time       :2018/6/27 上午 11:26
Describe        :
*/
int inGetFTPEffectiveReportBit(char* szFTPEffectiveReportBit)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPEffectiveReportBit == NULL || strlen(srTMSFTPRec.szFTPEffectiveReportBit) <= 0 || strlen(srTMSFTPRec.szFTPEffectiveReportBit) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetFTPEffectiveReportBit() ERROR !!");

                        if (szFTPEffectiveReportBit == NULL)
                        {
                                inLogPrintf(AT, "szFTPEffectiveReportBit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPEffectiveReportBit length = (%d)", (int)strlen(srTMSFTPRec.szFTPEffectiveReportBit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPEffectiveReportBit[0], &srTMSFTPRec.szFTPEffectiveReportBit[0], strlen(srTMSFTPRec.szFTPEffectiveReportBit));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPEffectiveReportBit
Date&Time       :
Describe        :
*/
int inSetFTPEffectiveReportBit(char* szFTPEffectiveReportBit)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPEffectiveReportBit == NULL || strlen(szFTPEffectiveReportBit) <= 0 || strlen(szFTPEffectiveReportBit) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetFTPEffectiveReportBit() ERROR !!");

                        if (szFTPEffectiveReportBit == NULL)
                        {
                                inLogPrintf(AT, "szFTPEffectiveReportBit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPEffectiveReportBit length = (%d)", (int)strlen(szFTPEffectiveReportBit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSFTPRec.szFTPEffectiveReportBit, 0x00, sizeof(srTMSFTPRec.szFTPEffectiveReportBit));        
        memcpy(&srTMSFTPRec.szFTPEffectiveReportBit[0], &szFTPEffectiveReportBit[0], strlen(szFTPEffectiveReportBit));

        return (VS_SUCCESS);
}

/*
Function        :inTMSFTP_Edit_TMSFTP_Table
Date&Time       :2017/7/21 上午 11:03
Describe        :
*/
int inTMSFTP_Edit_TMSFTP_Table(void)
{
	TABLE_GET_SET_TABLE TMSFTP_FUNC_TABLE[] =
	{
		{"szFTPIPAddress"		,inGetFTPIPAddress		,inSetFTPIPAddress		},
		{"szFTPPortNum"			,inGetFTPPortNum		,inSetFTPPortNum		},
		{"szFTPID"			,inGetFTPID			,inSetFTPID			},
		{"szFTPPW"			,inGetFTPPW			,inSetFTPPW			},
		{"szFTPAutoDownloadFlag"	,inGetFTPAutoDownloadFlag	,inSetFTPAutoDownloadFlag	},
		{"szFTPStartDownloadDateTime"	,inGetFTPStartDownloadDateTime	,inSetFTPStartDownloadDateTime	},
		{"szFTPTermParemeterDateTime"	,inGetFTPTermParemeterDateTime	,inSetFTPTermParemeterDateTime	},
		{"szFTPEffectiveCloseBatch"	,inGetFTPEffectiveCloseBatch	,inSetFTPEffectiveCloseBatch	},
		{"szFTPBatchNum"		,inGetFTPBatchNum		,inSetFTPBatchNum		},
		{"szFTPInquiryResponseCode"	,inGetFTPInquiryResponseCode	,inSetFTPInquiryResponseCode	},
		{"szFTPDownloadResponseCode"	,inGetFTPDownloadResponseCode	,inSetFTPDownloadResponseCode	},
		{"szFTPDownloadCategory"	,inGetFTPDownloadCategory	,inSetFTPDownloadCategory	},
		{"szFTPEffectiveReportBit"	,inGetFTPEffectiveReportBit	,inSetFTPEffectiveReportBit	},
		{""},
	};
	int	inRetVal = VS_ERROR;
	int	inFinalTimeout = 0;
	char	szKey = 0x00;
	
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
	inDISP_ChineseFont_Color("是否更改TMSFTP", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
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
	
	inSaveTMSFTPRec(0);
	inFunc_Edit_Table_Tag(TMSFTP_FUNC_TABLE);
	inSaveTMSFTPRec(0);
	
	return	(VS_SUCCESS);
}
