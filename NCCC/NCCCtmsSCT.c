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
#include "../SOURCE/FUNCTION/Utility.h"
#include "NCCCtmsSCT.h"

static  TMSSCT_REC srTMSSCTRec;	/* construct TMSSCT record */
extern  int     ginDebug;       /* Debug使用 extern */

/*
Function        :inLoadTMSSCTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀TMSSCT檔案，inTMSSCTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadTMSSCTRec(int inTMSSCTRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆TMSSCT檔案 */
        char            szTMSSCTRec[_SIZE_TMSSCT_REC_ + 1];         /* 暫存, 放各個欄位檔案 */
        char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnTMSSCTLength = 0;                       /* TMSSCT總長度 */
        long            lnReadLength;                           /* 記錄每次要從TMSSCT.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;	                /* inRec記錄讀到第幾筆, i為目前從TMSSCT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */
       
        /* inLoadTMSSCTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadTMSSCTRec(%d) START!!", inTMSSCTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 判斷傳進來的inTMSSCTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inTMSSCTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inTMSSCTRec < 0:(index = %d) ERROR!!", inTMSSCTRec);
                        inLogPrintf(AT, szErrorMsg);
                }
                vdUtility_SYSFIN_LogMessage(AT, "inTMSSCTRec < 0:(index = %d) ERROR!!", inTMSSCTRec);

                return (VS_ERROR);
        }

        /*
         * open TMSSCT.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_TMSSCT_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnTMSSCTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_TMSSCT_FILE_NAME_);
        
        if (lnTMSSCTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnTMSSCTLength + 1);
        uszTemp = malloc(lnTMSSCTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnTMSSCTLength + 1);
        memset(uszTemp, 0x00, lnTMSSCTLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnTMSSCTLength;

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
         *i為目前從TMSSCT讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnTMSSCTLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到TMSSCT的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnTMSSCTLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        /* 因為inTMSSCT_Rec的index從0開始，所以inTMSSCT_Rec要+1 */
                        if (inRec == (inTMSSCTRec + 1))
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
         * 如果沒有inTMSSCTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inTMSSCTRec + 1) || inSearchResult == -1)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "No data or Index ERROR");
                }

                vdUtility_SYSFIN_LogMessage(AT, "No data or Index ERROR, inRec:(%d), inSearchResult:(%d), inTMSSCTRec(%d)", inRec, inSearchResult, inTMSSCTRec);
                
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
                vdUtility_SYSFIN_LogMessage(AT, "No data or Index ERROR, inRec:(%d), inSearchResult:(%d), inTMSSCTRec(%d)", inRec, inSearchResult, inTMSSCTRec);
                
                /* 關檔 */
                inFILE_Close(&ulFile_Handle);
				
                /* Free pointer */
                free(uszReadData);
                free(uszTemp);

                return (VS_ERROR);
        }

        /* 結構初始化 */
        memset(&srTMSSCTRec, 0x00, sizeof(srTMSSCTRec));
        /*
         * 以下pattern為存入TMSSCT_Rec
         * i為TMSSCT的第幾個字元
         * 存入TMSSCT_Rec
         */
        i = 0;


        /* 01_TMS下載詢問機制 */
        /* 初始化 */
        memset(szTMSSCTRec, 0x00, sizeof(szTMSSCTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSSCTRec[k ++] = uszReadData[i ++];
                if (szTMSSCTRec[k - 1] == 0x2C	||
		    szTMSSCTRec[k - 1] == 0x0D	||
		    szTMSSCTRec[k - 1] == 0x0A	||
		    szTMSSCTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSSCTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSSCT unpack ERROR.");
                        }
                        vdUtility_SYSFIN_LogMessage(AT, "TMSSCT unpack ERROR.");

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);
						
                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szTMSSCTRec[0] != 0x2C	&&
	    szTMSSCTRec[0] != 0x0D	&&
	    szTMSSCTRec[0] != 0x0A	&&
	    szTMSSCTRec[0] != 0x00)
        {
                memcpy(&srTMSSCTRec.szTMSInquireMode[0], &szTMSSCTRec[0], k - 1);
        }

        /* 02_TMS 安排時間自動訊問 排程起始日 */
        /* 初始化 */
        memset(szTMSSCTRec, 0x00, sizeof(szTMSSCTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSSCTRec[k ++] = uszReadData[i ++];
                if (szTMSSCTRec[k - 1] == 0x2C	||
		    szTMSSCTRec[k - 1] == 0x0D	||
		    szTMSSCTRec[k - 1] == 0x0A	||
		    szTMSSCTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSSCTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSSCT unpack ERROR");
                        }
                        vdUtility_SYSFIN_LogMessage(AT, "TMSSCT unpack ERROR.");

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szTMSSCTRec[0] != 0x2C	&&
	    szTMSSCTRec[0] != 0x0D	&&
	    szTMSSCTRec[0] != 0x0A	&&
	    szTMSSCTRec[0] != 0x00)
        {
                memcpy(&srTMSSCTRec.szTMSInquireStartDate[0], &szTMSSCTRec[0], k - 1);
        }

        /* 03_TMS 安排時間自動訊問 指定詢問時間 */
        /* 初始化 */
        memset(szTMSSCTRec, 0x00, sizeof(szTMSSCTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSSCTRec[k ++] = uszReadData[i ++];
                if (szTMSSCTRec[k - 1] == 0x2C	||
		    szTMSSCTRec[k - 1] == 0x0D	||
		    szTMSSCTRec[k - 1] == 0x0A	||
		    szTMSSCTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSSCTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSSCT unpack ERROR");
                        }
                        vdUtility_SYSFIN_LogMessage(AT, "TMSSCT unpack ERROR.");

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szTMSSCTRec[0] != 0x2C	&&
	    szTMSSCTRec[0] != 0x0D	&&
	    szTMSSCTRec[0] != 0x0A	&&
	    szTMSSCTRec[0] != 0x00)
        {
                memcpy(&srTMSSCTRec.szTMSInquireTime[0], &szTMSSCTRec[0], k - 1);
        }

        /* 04_TMS 安排時間自動訊問 間隔幾天詢問 */
        /* 初始化 */
        memset(szTMSSCTRec, 0x00, sizeof(szTMSSCTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSSCTRec[k ++] = uszReadData[i ++];
                if (szTMSSCTRec[k - 1] == 0x2C	||
		    szTMSSCTRec[k - 1] == 0x0D	||
		    szTMSSCTRec[k - 1] == 0x0A	||
		    szTMSSCTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSSCTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSSCT unpack ERROR");
                        }
                        vdUtility_SYSFIN_LogMessage(AT, "TMSSCT unpack ERROR.");

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szTMSSCTRec[0] != 0x2C	&&
	    szTMSSCTRec[0] != 0x0D	&&
	    szTMSSCTRec[0] != 0x0A	&&
	    szTMSSCTRec[0] != 0x00)
        {
                memcpy(&srTMSSCTRec.szTMSInquireGap[0], &szTMSSCTRec[0], k - 1);
        }

        /* 05_Trace Log 上傳模式 */
        /* 初始化 */
        memset(szTMSSCTRec, 0x00, sizeof(szTMSSCTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSSCTRec[k ++] = uszReadData[i ++];
                if (szTMSSCTRec[k - 1] == 0x2C	||
		    szTMSSCTRec[k - 1] == 0x0D	||
		    szTMSSCTRec[k - 1] == 0x0A	||
		    szTMSSCTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSSCTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSSCT unpack ERROR");
                        }
                        vdUtility_SYSFIN_LogMessage(AT, "TMSSCT unpack ERROR.");

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szTMSSCTRec[0] != 0x2C	&&
	    szTMSSCTRec[0] != 0x0D	&&
	    szTMSSCTRec[0] != 0x0A	&&
	    szTMSSCTRec[0] != 0x00)
        {
                memcpy(&srTMSSCTRec.szTraceLogUploadMode[0], &szTMSSCTRec[0], k - 1);
        }

        /* 06_參數為上傳啟始日 */
        /* 初始化 */
        memset(szTMSSCTRec, 0x00, sizeof(szTMSSCTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSSCTRec[k ++] = uszReadData[i ++];
                if (szTMSSCTRec[k - 1] == 0x2C	||
		    szTMSSCTRec[k - 1] == 0x0D	||
		    szTMSSCTRec[k - 1] == 0x0A	||
		    szTMSSCTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSSCTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSSCT unpack ERROR");
                        }
                        vdUtility_SYSFIN_LogMessage(AT, "TMSSCT unpack ERROR.");

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }

        /*  該筆有資料 */
        if (szTMSSCTRec[0] != 0x2C	&&
	    szTMSSCTRec[0] != 0x0D	&&
	    szTMSSCTRec[0] != 0x0A	&&
	    szTMSSCTRec[0] != 0x00)
        {
                memcpy(&srTMSSCTRec.szTraceLogUploadStartDate[0], &szTMSSCTRec[0], k - 1);
        }

        /* 07_參數為上傳結束日 */
        /* 初始化 */
        memset(szTMSSCTRec, 0x00, sizeof(szTMSSCTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSSCTRec[k ++] = uszReadData[i ++];
                if (szTMSSCTRec[k - 1] == 0x2C	||
		    szTMSSCTRec[k - 1] == 0x0D	||
		    szTMSSCTRec[k - 1] == 0x0A	||
		    szTMSSCTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSSCTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSSCT unpack ERROR");
                        }
                        vdUtility_SYSFIN_LogMessage(AT, "TMSSCT unpack ERROR.");

                        /* 關檔 */
                        inFILE_Close(&ulFile_Handle);

                        /* Free pointer */
                        free(uszReadData);
                        free(uszTemp);

                        return (VS_ERROR);
                }
        }
   
        /*  該筆有資料 */
        if (szTMSSCTRec[0] != 0x2C	&&
	    szTMSSCTRec[0] != 0x0D	&&
	    szTMSSCTRec[0] != 0x0A	&&
	    szTMSSCTRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srTMSSCTRec.szTraceLogUploadEndDate[0], &szTMSSCTRec[0], k - 1);
        }

        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadTMSSCTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadTMSSCTRec(%d) END!!", inTMSSCTRec);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveTMSSCTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSaveTMSSCTRec(int inTMSSCTRec)
{
        unsigned long   uldat_Handle;                           /* FILE Handle */
        unsigned long   ulbak_Handle;                           /* FILE Handle */
        int	        inPackCount = 0;                        /* uszWriteBuff_Record的index */
        int	        inRecIndex = 0;                         /* uszRead_Total_Buff的index */
        int             inTMSSCT_Total_Rec = 0;                    /* TMSSCT.dat的總筆數 */
        int             inRetVal;
        int             i = 0, j = 0;
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long	        lnReadLength = 0;                       /* 每次要從TMSSCT.dat讀多少byte出來 */
        long            lnTMSSCTLength = 0;                        /* TMSSCT.dat檔案總長度 */
        unsigned char	*uszRead_Total_Buff;
        unsigned char   *uszWriteBuff_Record, *uszWriteBuff_Org;


        /* inSaveTMSSCTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveTMSSCTRec(%d)_START!!", inTMSSCTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 防呆先刪除TMSSCT.bak */
        inFILE_Delete((unsigned char *)_TMSSCT_FILE_NAME_BAK_);

        /* 新建TMSSCT.bak檔案 */
        inRetVal = inFILE_Create(&ulbak_Handle, (unsigned char *)_TMSSCT_FILE_NAME_BAK_);

        if (inRetVal != VS_SUCCESS)
        {
                inFILE_Close(&ulbak_Handle);

                return (VS_ERROR);
        }

        /* 開啟原檔案TMSSCT.dat */
        inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_TMSSCT_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                /* TMSSCT.dat開檔失敗 ，不用關檔TMSSCT.dat */
                /* TMSSCT.bak仍要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_TMSSCT_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 取得TMSSCT.dat檔案大小 */
        lnTMSSCTLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *)_TMSSCT_FILE_NAME_);

        /* 取得檔案大小失敗 */
        if (lnTMSSCTLength == VS_ERROR)
        {
                /* TMSSCT.bak和TMSSCT.dat要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Close(&uldat_Handle);
                inFILE_Delete((unsigned char *)_TMSSCT_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        uszWriteBuff_Record = malloc(_SIZE_TMSSCT_REC_ + _SIZE_TMSSCT_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_TMSSCT_REC_ + _SIZE_TMSSCT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原TMSSCT.dat的每筆Record 初始化 */
        uszWriteBuff_Org = malloc(_SIZE_TMSSCT_REC_ + _SIZE_TMSSCT_COMMA_0D0A_);
        memset(uszWriteBuff_Org, 0x00, _SIZE_TMSSCT_REC_ + _SIZE_TMSSCT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存TMSSCT.dat全部資料 */
        uszRead_Total_Buff = malloc(lnTMSSCTLength + 1);
        memset(uszRead_Total_Buff, 0x00, lnTMSSCTLength + 1);

        inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

        /* TMSInquireMode */
        memcpy(&uszWriteBuff_Record[0], &srTMSSCTRec.szTMSInquireMode[0], strlen(srTMSSCTRec.szTMSInquireMode));
        inPackCount += strlen(srTMSSCTRec.szTMSInquireMode);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TMSInquireStartDate */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTMSSCTRec.szTMSInquireStartDate[0], strlen(srTMSSCTRec.szTMSInquireStartDate));
        inPackCount += strlen(srTMSSCTRec.szTMSInquireStartDate);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TMSInquireTime */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTMSSCTRec.szTMSInquireTime[0], strlen(srTMSSCTRec.szTMSInquireTime));
        inPackCount += strlen(srTMSSCTRec.szTMSInquireTime);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TMSInquireGap */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTMSSCTRec.szTMSInquireGap[0], strlen(srTMSSCTRec.szTMSInquireGap));
        inPackCount += strlen(srTMSSCTRec.szTMSInquireGap);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TraceLogUploadMode */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTMSSCTRec.szTraceLogUploadMode[0], strlen(srTMSSCTRec.szTraceLogUploadMode));
        inPackCount += strlen(srTMSSCTRec.szTraceLogUploadMode);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TraceLogUploadStartDate */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTMSSCTRec.szTraceLogUploadStartDate[0], strlen(srTMSSCTRec.szTraceLogUploadStartDate));
        inPackCount += strlen(srTMSSCTRec.szTraceLogUploadStartDate);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TraceLogUploadEndDate */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTMSSCTRec.szTraceLogUploadEndDate[0], strlen(srTMSSCTRec.szTraceLogUploadEndDate));
        inPackCount += strlen(srTMSSCTRec.szTraceLogUploadEndDate);

        /* 最後的data不用逗號 */
        /* 補上換行符號 */

        /* 0D是移至行首 */
        uszWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        uszWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;

        /* 讀TMSSCT.dat檔案存到uszRead_Total_Buff */
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnTMSSCTLength;

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
                                        inFILE_Delete((unsigned char *)_TMSSCT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_TMSSCT_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_TMSSCT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        /* 防呆 總record數量小於要存取inTMSSCTRec Return ERROR */
        /* 算總Record數 */
        for (i = 0; i < (lnTMSSCTLength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (uszRead_Total_Buff[i] == 0x0A)
                        inTMSSCT_Total_Rec++;
        }

        /* 防呆 總record數量小於要存取inTMSSCTRec Return ERROR */
        if ((inTMSSCTRec + 1) > inTMSSCT_Total_Rec)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "No data or Index ERROR");
                }

                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_TMSSCT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                return (VS_ERROR);
        }

        /* 記錄ReadTotalBuffer的陣列位置 */
        inRecIndex = 0;
        /* 用inTMSSCTRec決定要先存幾筆Record到TMSSCT.bak，ex:inTMSSCTRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inTMSSCTRec; i ++)
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
                                        inFILE_Delete((unsigned char *)_TMSSCT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_TMSSCT_FILE_NAME_BAK_);

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

        /* 存組好的該TMSSCTRecord 到 TMSSCT.bak */
        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_TMSSCT_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_TMSSCT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原TMSSCT.dat Record 到 TMSSCT.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
        /* inRecIndex要指到剩下還沒存進bak檔的第一個記憶體位置 */
        inTMSSCTRec = inTMSSCTRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < lnTMSSCTLength; i++)
        {
                if (uszRead_Total_Buff[i] == 0x0A)
                {
                        j++;

                        if (j == inTMSSCTRec)
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
                /* 接續存原TMSSCT.dat的Record */
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
                                inFILE_Delete((unsigned char *)_TMSSCT_FILE_NAME_BAK_);

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
                                inFILE_Delete((unsigned char *)_TMSSCT_FILE_NAME_BAK_);

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

        /* 刪除原TMSSCT.dat */
        if (inFILE_Delete((unsigned char *)_TMSSCT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* 將TMSSCT.bak改名字為TMSSCT.dat取代原檔案 */
        if (inFILE_Rename((unsigned char *)_TMSSCT_FILE_NAME_BAK_, (unsigned char *)_TMSSCT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* inSaveTMSSCTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveTMSSCTRec(%d) END!!", inTMSSCTRec - 1);
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
Function        :inGetTMSInquireMode
Date&Time       :
Describe        :
*/
int inGetTMSInquireMode(char* szTMSInquireMode)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSInquireMode == NULL || strlen(srTMSSCTRec.szTMSInquireMode) <= 0 || strlen(srTMSSCTRec.szTMSInquireMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTMSInquireMode() ERROR !!");
                        
			if (szTMSInquireMode == NULL)
                        {
                                inLogPrintf(AT, "szTMSInquireMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSInquireMode length = (%d)", (int)strlen(srTMSSCTRec.szTMSInquireMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSInquireMode[0], &srTMSSCTRec.szTMSInquireMode[0], strlen(srTMSSCTRec.szTMSInquireMode));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSInquireMode
Date&Time       :
Describe        :
*/
int inSetTMSInquireMode(char* szTMSInquireMode)
{
        memset(srTMSSCTRec.szTMSInquireMode, 0x00, sizeof(srTMSSCTRec.szTMSInquireMode));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTMSInquireMode == NULL || strlen(szTMSInquireMode) <= 0 || strlen(szTMSInquireMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTMSInquireMode() ERROR !!");
                        if (szTMSInquireMode == NULL)
                        {
                                inLogPrintf(AT, "szTMSInquireMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSInquireMode length = (%d)", (int)strlen(szTMSInquireMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTMSSCTRec.szTMSInquireMode[0], &szTMSInquireMode[0], strlen(szTMSInquireMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMSInquireStartDate
Date&Time       :
Describe        :
*/
int inGetTMSInquireStartDate(char* szTMSInquireStartDate)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSInquireStartDate == NULL || strlen(srTMSSCTRec.szTMSInquireStartDate) <= 0 || strlen(srTMSSCTRec.szTMSInquireStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTMSInquireStartDate() ERROR !!");

                        if (szTMSInquireStartDate == NULL)
                        {
                                inLogPrintf(AT, "szTMSInquireStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSInquireStartDate length = (%d)", (int)strlen(srTMSSCTRec.szTMSInquireStartDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSInquireStartDate[0], &srTMSSCTRec.szTMSInquireStartDate[0], strlen(srTMSSCTRec.szTMSInquireStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSInquireStartDate
Date&Time       :
Describe        :
*/
int inSetTMSInquireStartDate(char* szTMSInquireStartDate)
{
        memset(srTMSSCTRec.szTMSInquireStartDate, 0x00, sizeof(srTMSSCTRec.szTMSInquireStartDate));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTMSInquireStartDate == NULL || strlen(szTMSInquireStartDate) <= 0 || strlen(szTMSInquireStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTMSInquireStartDate() ERROR !!");

                        if (szTMSInquireStartDate == NULL)
                        {
                                inLogPrintf(AT, "szTMSInquireStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSInquireStartDate length = (%d)", (int)strlen(szTMSInquireStartDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTMSSCTRec.szTMSInquireStartDate[0], &szTMSInquireStartDate[0], strlen(szTMSInquireStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMSInquireTime
Date&Time       :
Describe        :
*/
int inGetTMSInquireTime(char* szTMSInquireTime)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSInquireTime == NULL || strlen(srTMSSCTRec.szTMSInquireTime) <= 0 || strlen(srTMSSCTRec.szTMSInquireTime) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTMSInquireTime() ERROR !!");

                        if (szTMSInquireTime == NULL)
                        {
                                inLogPrintf(AT, "szTMSInquireTime == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSInquireTime length = (%d)", (int)strlen(srTMSSCTRec.szTMSInquireTime));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSInquireTime[0], &srTMSSCTRec.szTMSInquireTime[0], strlen(srTMSSCTRec.szTMSInquireTime));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSInquireTime
Date&Time       :
Describe        :
*/
int inSetTMSInquireTime(char* szTMSInquireTime)
{
        memset(srTMSSCTRec.szTMSInquireTime, 0x00, sizeof(srTMSSCTRec.szTMSInquireTime));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTMSInquireTime == NULL || strlen(szTMSInquireTime) <= 0 || strlen(szTMSInquireTime) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTMSInquireTime() ERROR !!");

                        if (szTMSInquireTime == NULL)
                        {
                                inLogPrintf(AT, "szTMSInquireTime == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSInquireTime length = (%d)", (int)strlen(szTMSInquireTime));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTMSSCTRec.szTMSInquireTime[0], &szTMSInquireTime[0], strlen(szTMSInquireTime));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMSInquireGap
Date&Time       :
Describe        :
*/
int inGetTMSInquireGap(char* szTMSInquireGap)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSInquireGap == NULL || strlen(srTMSSCTRec.szTMSInquireGap) <= 0 || strlen(srTMSSCTRec.szTMSInquireGap) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTMSInquireGap() ERROR !!");

                        if (szTMSInquireGap == NULL)
                        {
                                inLogPrintf(AT, "szTMSInquireGap == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSInquireGap length = (%d)", (int)strlen(srTMSSCTRec.szTMSInquireGap));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSInquireGap[0], &srTMSSCTRec.szTMSInquireGap[0], strlen(srTMSSCTRec.szTMSInquireGap));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSInquireGap
Date&Time       :
Describe        :
*/
int inSetTMSInquireGap(char* szTMSInquireGap)
{
        memset(srTMSSCTRec.szTMSInquireGap, 0x00, sizeof(srTMSSCTRec.szTMSInquireGap));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTMSInquireGap == NULL || strlen(szTMSInquireGap) <= 0 || strlen(szTMSInquireGap) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTMSInquireGap() ERROR !!");

                        if (szTMSInquireGap == NULL)
                        {
                                inLogPrintf(AT, "szTMSInquireGap == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSInquireGap length = (%d)", (int)strlen(szTMSInquireGap));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTMSSCTRec.szTMSInquireGap[0], &szTMSInquireGap[0], strlen(szTMSInquireGap));

        return (VS_SUCCESS);
}

/*
Function        :inGetTraceLogUploadMode
Date&Time       :
Describe        :
*/
int inGetTraceLogUploadMode(char* szTraceLogUploadMode)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTraceLogUploadMode == NULL || strlen(srTMSSCTRec.szTraceLogUploadMode) <= 0 || strlen(srTMSSCTRec.szTraceLogUploadMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTraceLogUploadMode() ERROR !!");

                        if (szTraceLogUploadMode == NULL)
                        {
                                inLogPrintf(AT, "szTraceLogUploadMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTraceLogUploadMode length = (%d)", (int)strlen(srTMSSCTRec.szTraceLogUploadMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTraceLogUploadMode[0], &srTMSSCTRec.szTraceLogUploadMode[0], strlen(srTMSSCTRec.szTraceLogUploadMode));

        return (VS_SUCCESS);
}

/*
Function        :inSetTraceLogUploadMode
Date&Time       :
Describe        :
*/
int inSetTraceLogUploadMode(char* szTraceLogUploadMode)
{
        memset(srTMSSCTRec.szTraceLogUploadMode, 0x00, sizeof(srTMSSCTRec.szTraceLogUploadMode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTraceLogUploadMode == NULL || strlen(szTraceLogUploadMode) <= 0 || strlen(szTraceLogUploadMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTraceLogUploadMode() ERROR !!");

                        if (szTraceLogUploadMode == NULL)
                        {
                                inLogPrintf(AT, "szTraceLogUploadMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTraceLogUploadMode length = (%d)", (int)strlen(szTraceLogUploadMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTMSSCTRec.szTraceLogUploadMode[0], &szTraceLogUploadMode[0], strlen(szTraceLogUploadMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetTraceLogUploadStartDate
Date&Time       :
Describe        :
*/
int inGetTraceLogUploadStartDate(char* szTraceLogUploadStartDate)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTraceLogUploadStartDate == NULL || strlen(srTMSSCTRec.szTraceLogUploadStartDate) <= 0 || strlen(srTMSSCTRec.szTraceLogUploadStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTraceLogUploadStartDate() ERROR !!");

                        if (szTraceLogUploadStartDate == NULL)
                        {
                                inLogPrintf(AT, "szTraceLogUploadStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTraceLogUploadStartDate length = (%d)", (int)strlen(srTMSSCTRec.szTraceLogUploadStartDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTraceLogUploadStartDate[0], &srTMSSCTRec.szTraceLogUploadStartDate[0], strlen(srTMSSCTRec.szTraceLogUploadStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetTraceLogUploadStartDate
Date&Time       :
Describe        :
*/
int inSetTraceLogUploadStartDate(char* szTraceLogUploadStartDate)
{
        memset(srTMSSCTRec.szTraceLogUploadStartDate, 0x00, sizeof(srTMSSCTRec.szTraceLogUploadStartDate));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTraceLogUploadStartDate == NULL || strlen(szTraceLogUploadStartDate) <= 0 || strlen(szTraceLogUploadStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTraceLogUploadStartDate() ERROR !!");

                        if (szTraceLogUploadStartDate == NULL)
                        {
                                inLogPrintf(AT, "szTraceLogUploadStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTraceLogUploadStartDate length = (%d)", (int)strlen(szTraceLogUploadStartDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTMSSCTRec.szTraceLogUploadStartDate[0], &szTraceLogUploadStartDate[0], strlen(szTraceLogUploadStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetTraceLogUploadEndDate
Date&Time       :
Describe        :
*/
int inGetTraceLogUploadEndDate(char* szTraceLogUploadEndDate)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTraceLogUploadEndDate == NULL || strlen(srTMSSCTRec.szTraceLogUploadEndDate) <= 0 || strlen(srTMSSCTRec.szTraceLogUploadEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTraceLogUploadEndDate() ERROR !!");

                        if (szTraceLogUploadEndDate == NULL)
                        {
                                inLogPrintf(AT, "szTraceLogUploadEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTraceLogUploadEndDate length = (%d)", (int)strlen(srTMSSCTRec.szTraceLogUploadEndDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTraceLogUploadEndDate[0], &srTMSSCTRec.szTraceLogUploadEndDate[0], strlen(srTMSSCTRec.szTraceLogUploadEndDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetTraceLogUploadEndDate
Date&Time       :
Describe        :
*/
int inSetTraceLogUploadEndDate(char* szTraceLogUploadEndDate)
{
        memset(srTMSSCTRec.szTraceLogUploadEndDate, 0x00, sizeof(srTMSSCTRec.szTraceLogUploadEndDate));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTraceLogUploadEndDate == NULL || strlen(szTraceLogUploadEndDate) <= 0 || strlen(szTraceLogUploadEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTraceLogUploadEndDate() ERROR !!");

                        if (szTraceLogUploadEndDate == NULL)
                        {
                                inLogPrintf(AT, "szTraceLogUploadEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTraceLogUploadEndDate length = (%d)", (int)strlen(szTraceLogUploadEndDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTMSSCTRec.szTraceLogUploadEndDate[0], &szTraceLogUploadEndDate[0], strlen(szTraceLogUploadEndDate));

        return (VS_SUCCESS);
}

/*
Function        :inTMSSCT_Edit_TMSSCT_Table
Date&Time       :2017/7/21 上午 11:03
Describe        :
*/
int inTMSSCT_Edit_TMSSCT_Table(void)
{
	TABLE_GET_SET_TABLE TMSSCT_FUNC_TABLE[] =
	{
		{"szTMSInquireMode"		,inGetTMSInquireMode		,inSetTMSInquireMode		},
		{"szTMSInquireStartDate"	,inGetTMSInquireStartDate	,inSetTMSInquireStartDate	},
		{"szTMSInquireTime"		,inGetTMSInquireTime		,inSetTMSInquireTime		},
		{"szTMSInquireGap"		,inGetTMSInquireGap		,inSetTMSInquireGap		},
		{"szTraceLogUploadMode"		,inGetTraceLogUploadMode	,inSetTraceLogUploadMode	},
		{"szTraceLogUploadStartDate"	,inGetTraceLogUploadStartDate	,inSetTraceLogUploadStartDate	},
		{"szTraceLogUploadEndDate"	,inGetTraceLogUploadEndDate	,inSetTraceLogUploadEndDate	},
		{""},
	};
	int	inRetVal;
	int	inFinalTimeout = 0;
	char	szKey;
	
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
	inDISP_ChineseFont_Color("是否更改TMSSCT", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
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
	
	inSaveTMSSCTRec(0);
	inFunc_Edit_Table_Tag(TMSSCT_FUNC_TABLE);
	inSaveTMSSCTRec(0);
	
	return	(VS_SUCCESS);
}
