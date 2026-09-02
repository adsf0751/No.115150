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
#include "EST.h"

static  EST_REC srESTRec;	/* construct EST record */
extern  int     ginDebug;       /* Debug使用 extern */

/*
Function        :inLoadESTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀EST檔案，inESTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadESTRec(int inESTRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆EST檔案 */
        char            szESTRec[_SIZE_EST_REC_ + 1];           /* 暫存, 放各個欄位檔案 */
        char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnESTLength = 0;                        /* EST總長度 */
        long            lnReadLength;                           /* 記錄每次要從EST.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;	                /* inRec記錄讀到第幾筆, i為目前從EST讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */

        /* inLoadESTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadESTRec(%d) START!!", inESTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 判斷傳進來的inESTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inESTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inESTRec < 0:(index = %d) ERROR!!", inESTRec);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open EST.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_EST_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnESTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_EST_FILE_NAME_);

        if (lnESTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnESTLength + 1);
        uszTemp = malloc(lnESTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnESTLength + 1);
        memset(uszTemp, 0x00, lnESTLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnESTLength;

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
         *i為目前從EST讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnESTLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到EST的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnESTLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        /* 因為inEST_Rec的index從0開始，所以inEST_Rec要+1 */
                        if (inRec == (inESTRec + 1))
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
         * 如果沒有inESTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inESTRec + 1) || inSearchResult == -1)
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
        memset(&srESTRec, 0x00, sizeof(srESTRec));
        /*
         * 以下pattern為存入EST_Rec
         * i為EST的第幾個字元
         * 存入EST_Rec
         */
        i = 0;


        /* 01_EMVCAPK索引 */
        /* 初始化 */
        memset(szESTRec, 0x00, sizeof(szESTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szESTRec[k ++] = uszReadData[i ++];
                if (szESTRec[k - 1] == 0x2C	||
		    szESTRec[k - 1] == 0x0D	||
		    szESTRec[k - 1] == 0x0A	||
		    szESTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnESTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EST unpack ERROR.");
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
        if (szESTRec[0] != 0x2C	&&
	    szESTRec[0] != 0x0D	&&
	    szESTRec[0] != 0x0A	&&
	    szESTRec[0] != 0x00)
        {
                memcpy(&srESTRec.szEMVCAPKIndex[0], &szESTRec[0], k - 1);
        }

        /* 02_應用程式 ID */
        /* 初始化 */
        memset(szESTRec, 0x00, sizeof(szESTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szESTRec[k ++] = uszReadData[i ++];
                if (szESTRec[k - 1] == 0x2C	||
		    szESTRec[k - 1] == 0x0D	||
		    szESTRec[k - 1] == 0x0A	||
		    szESTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnESTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EST unpack ERROR");
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
        if (szESTRec[0] != 0x2C	&&
	    szESTRec[0] != 0x0D	&&
	    szESTRec[0] != 0x0A	&&
	    szESTRec[0] != 0x00)
        {
                memcpy(&srESTRec.szCAPKApplicationId[0], &szESTRec[0], k - 1);
        }

        /* 03_CAPK索引值 */
        /* 初始化 */
        memset(szESTRec, 0x00, sizeof(szESTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szESTRec[k ++] = uszReadData[i ++];
                if (szESTRec[k - 1] == 0x2C	||
		    szESTRec[k - 1] == 0x0D	||
		    szESTRec[k - 1] == 0x0A	||
		    szESTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnESTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EST unpack ERROR");
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
        if (szESTRec[0] != 0x2C	&&
	    szESTRec[0] != 0x0D	&&
	    szESTRec[0] != 0x0A	&&
	    szESTRec[0] != 0x00)
        {
                memcpy(&srESTRec.szCAPKIndex[0], &szESTRec[0], k - 1);
        }

        /* 04_CAPK Key值有效期 */
        /* 初始化 */
        memset(szESTRec, 0x00, sizeof(szESTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szESTRec[k ++] = uszReadData[i ++];
                if (szESTRec[k - 1] == 0x2C	||
		    szESTRec[k - 1] == 0x0D	||
		    szESTRec[k - 1] == 0x0A	||
		    szESTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnESTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EST unpack ERROR");
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
        if (szESTRec[0] != 0x2C	&&
	    szESTRec[0] != 0x0D	&&
	    szESTRec[0] != 0x0A	&&
	    szESTRec[0] != 0x00)
        {
                memcpy(&srESTRec.szCAPKKeyExpireDate[0], &szESTRec[0], k - 1);
        }

        /* 05_CAPK Key值實際長度 */
        /* 初始化 */
        memset(szESTRec, 0x00, sizeof(szESTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szESTRec[k ++] = uszReadData[i ++];
                if (szESTRec[k - 1] == 0x2C	||
		    szESTRec[k - 1] == 0x0D	||
		    szESTRec[k - 1] == 0x0A	||
		    szESTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnESTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EST unpack ERROR");
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
        if (szESTRec[0] != 0x2C	&&
	    szESTRec[0] != 0x0D	&&
	    szESTRec[0] != 0x0A	&&
	    szESTRec[0] != 0x00)
        {
                memcpy(&srESTRec.szCAPKKeyLength[0], &szESTRec[0], k - 1);
        }

        /* 06_CAPK Key值 */
        /* 初始化 */
        memset(szESTRec, 0x00, sizeof(szESTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szESTRec[k ++] = uszReadData[i ++];
                if (szESTRec[k - 1] == 0x2C	||
		    szESTRec[k - 1] == 0x0D	||
		    szESTRec[k - 1] == 0x0A	||
		    szESTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnESTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EST unpack ERROR");
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
        if (szESTRec[0] != 0x2C	&&
	    szESTRec[0] != 0x0D	&&
	    szESTRec[0] != 0x0A	&&
	    szESTRec[0] != 0x00)
        {
                memcpy(&srESTRec.szCAPKKeyModulus[0], &szESTRec[0], k - 1);
        }

        /* 07_CAPK Key值 Exponent */
        /* 初始化 */
        memset(szESTRec, 0x00, sizeof(szESTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szESTRec[k ++] = uszReadData[i ++];
                if (szESTRec[k - 1] == 0x2C	||
		    szESTRec[k - 1] == 0x0D	||
		    szESTRec[k - 1] == 0x0A	||
		    szESTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnESTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "EST unpack ERROR");
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
        if (szESTRec[0] != 0x2C	&&
	    szESTRec[0] != 0x0D	&&
	    szESTRec[0] != 0x0A	&&
	    szESTRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srESTRec.szCAPKExponent[0], &szESTRec[0], k - 1);
        }

        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadESTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadESTRec(%d) END!!", inESTRec);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveESTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSaveESTRec(int inESTRec)
{
        unsigned long   uldat_Handle;                           /* FILE Handle */
        unsigned long   ulbak_Handle;                           /* FILE Handle */
        int	        inPackCount = 0;                        /* uszWriteBuff_Record的index */
        int	        inRecIndex = 0;                         /* uszRead_Total_Buff的index */
        int             inEST_Total_Rec = 0;                    /* EST.dat的總筆數 */
        int             inRetVal;
        int             i = 0, j = 0;
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long	        lnReadLength = 0;                       /* 每次要從EST.dat讀多少byte出來 */
        long            lnESTLength = 0;                        /* EST.dat檔案總長度 */
        unsigned char	*uszRead_Total_Buff;
        unsigned char   *uszWriteBuff_Record, *uszWriteBuff_Org;


        /* inSaveESTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveESTRec(%d)_START!!", inESTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 防呆先刪除EST.bak */
        inFILE_Delete((unsigned char *)_EST_FILE_NAME_BAK_);

        /* 新建EST.bak檔案 */
        inRetVal = inFILE_Create(&ulbak_Handle, (unsigned char *)_EST_FILE_NAME_BAK_);

        if (inRetVal != VS_SUCCESS)
        {
                inFILE_Close(&ulbak_Handle);

                return (VS_ERROR);
        }

        /* 開啟原檔案EST.dat */
        inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_EST_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                /* EST.dat開檔失敗 ，不用關檔EST.dat */
                /* EST.bak仍要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_EST_FILE_NAME_BAK_);

                return(VS_ERROR);
        }

        /* 取得EST.dat檔案大小 */
        lnESTLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *)_EST_FILE_NAME_);

        /* 取得檔案大小失敗 */
        if (lnESTLength == VS_ERROR)
        {
                /* EST.bak和EST.dat要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Close(&uldat_Handle);
                inFILE_Delete((unsigned char *)_EST_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        uszWriteBuff_Record = malloc(_SIZE_EST_REC_ + _SIZE_EST_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_EST_REC_ + _SIZE_EST_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原EST.dat的每筆Record 初始化 */
        uszWriteBuff_Org = malloc(_SIZE_EST_REC_ + _SIZE_EST_COMMA_0D0A_);
        memset(uszWriteBuff_Org, 0x00, _SIZE_EST_REC_ + _SIZE_EST_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存EST.dat全部資料 */
        uszRead_Total_Buff = malloc(lnESTLength + 1);
        memset(uszRead_Total_Buff, 0x00, lnESTLength + 1);

        inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

        /* EMVCAPKIndex */
        memcpy(&uszWriteBuff_Record[0], &srESTRec.szEMVCAPKIndex[0], strlen(srESTRec.szEMVCAPKIndex));
        inPackCount += strlen(srESTRec.szEMVCAPKIndex);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* CAPKApplicationId */
        memcpy(&uszWriteBuff_Record[inPackCount], &srESTRec.szCAPKApplicationId[0], strlen(srESTRec.szCAPKApplicationId));
        inPackCount += strlen(srESTRec.szCAPKApplicationId);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* CAPKIndex */
        memcpy(&uszWriteBuff_Record[inPackCount], &srESTRec.szCAPKIndex[0], strlen(srESTRec.szCAPKIndex));
        inPackCount += strlen(srESTRec.szCAPKIndex);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* CAPKKeyExpireDate */
        memcpy(&uszWriteBuff_Record[inPackCount], &srESTRec.szCAPKKeyExpireDate[0], strlen(srESTRec.szCAPKKeyExpireDate));
        inPackCount += strlen(srESTRec.szCAPKKeyExpireDate);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* CAPKKeyLength */
        memcpy(&uszWriteBuff_Record[inPackCount], &srESTRec.szCAPKKeyLength[0], strlen(srESTRec.szCAPKKeyLength));
        inPackCount += strlen(srESTRec.szCAPKKeyLength);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* CAPKKeyModulus */
        memcpy(&uszWriteBuff_Record[inPackCount], &srESTRec.szCAPKKeyModulus[0], strlen(srESTRec.szCAPKKeyModulus));
        inPackCount += strlen(srESTRec.szCAPKKeyModulus);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* CAPKExponent */
        memcpy(&uszWriteBuff_Record[inPackCount], &srESTRec.szCAPKExponent[0], strlen(srESTRec.szCAPKExponent));
        inPackCount += strlen(srESTRec.szCAPKExponent);

        /* 最後的data不用逗號 */
        /* 補上換行符號 */

        /* 0D是移至行首 */
        uszWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        uszWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;

        /* 讀EST.dat檔案存到uszRead_Total_Buff */
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnESTLength;

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
                                        inFILE_Delete((unsigned char *)_EST_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_EST_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_EST_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        /* 防呆 總record數量小於要存取inESTRec Return ERROR */
        /* 算總Record數 */
        for (i = 0; i < (lnESTLength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (uszRead_Total_Buff[i] == 0x0A)
                        inEST_Total_Rec++;
        }

        /* 防呆 總record數量小於要存取inESTRec Return ERROR */
        if ((inESTRec + 1) > inEST_Total_Rec)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "No data or Index ERROR");
                }

                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_EST_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                return (VS_ERROR);
        }

        /* 記錄ReadTotalBuffer的陣列位置 */
        inRecIndex = 0;
        /* 用inESTRec決定要先存幾筆Record到EST.bak，ex:inESTRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inESTRec; i ++)
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
                                        inFILE_Delete((unsigned char *)_EST_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_EST_FILE_NAME_BAK_);

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

        /* 存組好的該ESTRecord 到 EST.bak */
        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_EST_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_EST_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原EST.dat Record 到 EST.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
        /* inRecIndex要指到剩下還沒存進bak檔的第一個記憶體位置 */
        inESTRec = inESTRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < lnESTLength; i++)
        {
                if (uszRead_Total_Buff[i] == 0x0A)
                {
                        j++;

                        if (j == inESTRec)
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
                /* 接續存原EST.dat的Record */
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
                                inFILE_Delete((unsigned char *)_EST_FILE_NAME_BAK_);

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
                                inFILE_Delete((unsigned char *)_EST_FILE_NAME_BAK_);

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

        /* 刪除原EST.dat */
        if (inFILE_Delete((unsigned char *)_EST_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* 將EST.bak改名字為EST.dat取代原檔案 */
        if (inFILE_Rename((unsigned char *)_EST_FILE_NAME_BAK_, (unsigned char *)_EST_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* inSaveESTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveESTRec(%d) END!!", inESTRec - 1);
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
Function        :inGetEMVCAPKIndex
Date&Time       :
Describe        :
*/
int inGetEMVCAPKIndex(char* szEMVCAPKIndex)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szEMVCAPKIndex == NULL || strlen(srESTRec.szEMVCAPKIndex) <= 0 || strlen(srESTRec.szEMVCAPKIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetEMVCAPKIndex() ERROR !!");

			if (szEMVCAPKIndex == NULL)
                        {
                                inLogPrintf(AT, "szEMVCAPKIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEMVCAPKIndex length = (%d)", (int)strlen(srESTRec.szEMVCAPKIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szEMVCAPKIndex[0], &srESTRec.szEMVCAPKIndex[0], strlen(srESTRec.szEMVCAPKIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetEMVCAPKIndex
Date&Time       :
Describe        :
*/
int inSetEMVCAPKIndex(char* szEMVCAPKIndex)
{
        memset(srESTRec.szEMVCAPKIndex, 0x00, sizeof(srESTRec.szEMVCAPKIndex));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szEMVCAPKIndex == NULL || strlen(szEMVCAPKIndex) <= 0 || strlen(szEMVCAPKIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetEMVCAPKIndex() ERROR !!");
                        if (szEMVCAPKIndex == NULL)
                        {
                                inLogPrintf(AT, "szEMVCAPKIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEMVCAPKIndex length = (%d)", (int)strlen(szEMVCAPKIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srESTRec.szEMVCAPKIndex[0], &szEMVCAPKIndex[0], strlen(szEMVCAPKIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetCAPKApplicationId
Date&Time       :
Describe        :
*/
int inGetCAPKApplicationId(char* szCAPKApplicationId)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szCAPKApplicationId == NULL || strlen(srESTRec.szCAPKApplicationId) <= 0 || strlen(srESTRec.szCAPKApplicationId) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetCAPKApplicationId() ERROR !!");

                        if (szCAPKApplicationId == NULL)
                        {
                                inLogPrintf(AT, "szCAPKApplicationId == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCAPKApplicationId length = (%d)", (int)strlen(srESTRec.szCAPKApplicationId));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCAPKApplicationId[0], &srESTRec.szCAPKApplicationId[0], strlen(srESTRec.szCAPKApplicationId));

        return (VS_SUCCESS);
}

/*
Function        :inSetCAPKApplicationId
Date&Time       :
Describe        :
*/
int inSetCAPKApplicationId(char* szCAPKApplicationId)
{
        memset(srESTRec.szCAPKApplicationId, 0x00, sizeof(srESTRec.szCAPKApplicationId));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCAPKApplicationId == NULL || strlen(szCAPKApplicationId) <= 0 || strlen(szCAPKApplicationId) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCAPKApplicationId() ERROR !!");

                        if (szCAPKApplicationId == NULL)
                        {
                                inLogPrintf(AT, "szCAPKApplicationId == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCAPKApplicationId length = (%d)", (int)strlen(szCAPKApplicationId));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srESTRec.szCAPKApplicationId[0], &szCAPKApplicationId[0], strlen(szCAPKApplicationId));

        return (VS_SUCCESS);
}

/*
Function        :inGetCAPKIndex
Date&Time       :
Describe        :
*/
int inGetCAPKIndex(char* szCAPKIndex)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szCAPKIndex == NULL || strlen(srESTRec.szCAPKIndex) <= 0 || strlen(srESTRec.szCAPKIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetCAPKIndex() ERROR !!");

                        if (szCAPKIndex == NULL)
                        {
                                inLogPrintf(AT, "szCAPKIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCAPKIndex length = (%d)", (int)strlen(srESTRec.szCAPKIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCAPKIndex[0], &srESTRec.szCAPKIndex[0], strlen(srESTRec.szCAPKIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetCAPKIndex
Date&Time       :
Describe        :
*/
int inSetCAPKIndex(char* szCAPKIndex)
{
        memset(srESTRec.szCAPKIndex, 0x00, sizeof(srESTRec.szCAPKIndex));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCAPKIndex == NULL || strlen(szCAPKIndex) <= 0 || strlen(szCAPKIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCAPKIndex() ERROR !!");

                        if (szCAPKIndex == NULL)
                        {
                                inLogPrintf(AT, "szCAPKIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCAPKIndex length = (%d)", (int)strlen(szCAPKIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srESTRec.szCAPKIndex[0], &szCAPKIndex[0], strlen(szCAPKIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetHostCAPKKeyExpireDate
Date&Time       :
Describe        :
*/
int inGetHostCAPKKeyExpireDate(char* szCAPKKeyExpireDate)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szCAPKKeyExpireDate == NULL || strlen(srESTRec.szCAPKKeyExpireDate) <= 0 || strlen(srESTRec.szCAPKKeyExpireDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetHostCAPKKeyExpireDate() ERROR !!");

                        if (szCAPKKeyExpireDate == NULL)
                        {
                                inLogPrintf(AT, "szCAPKKeyExpireDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCAPKKeyExpireDate length = (%d)", (int)strlen(srESTRec.szCAPKKeyExpireDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCAPKKeyExpireDate[0], &srESTRec.szCAPKKeyExpireDate[0], strlen(srESTRec.szCAPKKeyExpireDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetHostCAPKKeyExpireDate
Date&Time       :
Describe        :
*/
int inSetHostCAPKKeyExpireDate(char* szCAPKKeyExpireDate)
{
        memset(srESTRec.szCAPKKeyExpireDate, 0x00, sizeof(srESTRec.szCAPKKeyExpireDate));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCAPKKeyExpireDate == NULL || strlen(szCAPKKeyExpireDate) <= 0 || strlen(szCAPKKeyExpireDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetHostCAPKKeyExpireDate() ERROR !!");

                        if (szCAPKKeyExpireDate == NULL)
                        {
                                inLogPrintf(AT, "szCAPKKeyExpireDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCAPKKeyExpireDate length = (%d)", (int)strlen(szCAPKKeyExpireDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srESTRec.szCAPKKeyExpireDate[0], &szCAPKKeyExpireDate[0], strlen(szCAPKKeyExpireDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetCAPKKeyLength
Date&Time       :
Describe        :
*/
int inGetCAPKKeyLength(char* szCAPKKeyLength)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szCAPKKeyLength == NULL || strlen(srESTRec.szCAPKKeyLength) <= 0 || strlen(srESTRec.szCAPKKeyLength) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetCAPKKeyLength() ERROR !!");

                        if (szCAPKKeyLength == NULL)
                        {
                                inLogPrintf(AT, "szCAPKKeyLength == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCAPKKeyLength length = (%d)", (int)strlen(srESTRec.szCAPKKeyLength));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCAPKKeyLength[0], &srESTRec.szCAPKKeyLength[0], strlen(srESTRec.szCAPKKeyLength));

        return (VS_SUCCESS);
}

/*
Function        :inSetCAPKKeyLength
Date&Time       :
Describe        :
*/
int inSetCAPKKeyLength(char* szCAPKKeyLength)
{
        memset(srESTRec.szCAPKKeyLength, 0x00, sizeof(srESTRec.szCAPKKeyLength));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCAPKKeyLength == NULL || strlen(szCAPKKeyLength) <= 0 || strlen(szCAPKKeyLength) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCAPKKeyLength() ERROR !!");

                        if (szCAPKKeyLength == NULL)
                        {
                                inLogPrintf(AT, "szCAPKKeyLength == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCAPKKeyLength length = (%d)", (int)strlen(szCAPKKeyLength));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srESTRec.szCAPKKeyLength[0], &szCAPKKeyLength[0], strlen(szCAPKKeyLength));

        return (VS_SUCCESS);
}

/*
Function        :inGetCAPKKeyModulus
Date&Time       :
Describe        :
*/
int inGetCAPKKeyModulus(char* szCAPKKeyModulus)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szCAPKKeyModulus == NULL || strlen(srESTRec.szCAPKKeyModulus) <= 0 || strlen(srESTRec.szCAPKKeyModulus) > 496)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetCAPKKeyModulus() ERROR !!");

                        if (szCAPKKeyModulus == NULL)
                        {
                                inLogPrintf(AT, "szCAPKKeyModulus == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCAPKKeyModulus length = (%d)", (int)strlen(srESTRec.szCAPKKeyModulus));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCAPKKeyModulus[0], &srESTRec.szCAPKKeyModulus[0], strlen(srESTRec.szCAPKKeyModulus));

        return (VS_SUCCESS);
}

/*
Function        :inSetCAPKKeyModulus
Date&Time       :
Describe        :
*/
int inSetCAPKKeyModulus(char* szCAPKKeyModulus)
{
        memset(srESTRec.szCAPKKeyModulus, 0x00, sizeof(srESTRec.szCAPKKeyModulus));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCAPKKeyModulus == NULL || strlen(szCAPKKeyModulus) <= 0 || strlen(szCAPKKeyModulus) > 496)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCAPKKeyModulus() ERROR !!");

                        if (szCAPKKeyModulus == NULL)
                        {
                                inLogPrintf(AT, "szCAPKKeyModulus == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCAPKKeyModulus length = (%d)", (int)strlen(szCAPKKeyModulus));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srESTRec.szCAPKKeyModulus[0], &szCAPKKeyModulus[0], strlen(szCAPKKeyModulus));

        return (VS_SUCCESS);
}

/*
Function        :inGetCAPKExponent
Date&Time       :
Describe        :
*/
int inGetCAPKExponent(char* szCAPKExponent)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szCAPKExponent == NULL || strlen(srESTRec.szCAPKExponent) <= 0 || strlen(srESTRec.szCAPKExponent) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetCAPKExponent() ERROR !!");

                        if (szCAPKExponent == NULL)
                        {
                                inLogPrintf(AT, "szCAPKExponent == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCAPKExponent length = (%d)", (int)strlen(srESTRec.szCAPKExponent));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCAPKExponent[0], &srESTRec.szCAPKExponent[0], strlen(srESTRec.szCAPKExponent));

        return (VS_SUCCESS);
}

/*
Function        :inSetCAPKExponent
Date&Time       :
Describe        :
*/
int inSetCAPKExponent(char* szCAPKExponent)
{
        memset(srESTRec.szCAPKExponent, 0x00, sizeof(srESTRec.szCAPKExponent));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCAPKExponent == NULL || strlen(szCAPKExponent) <= 0 || strlen(szCAPKExponent) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCAPKExponent() ERROR !!");

                        if (szCAPKExponent == NULL)
                        {
                                inLogPrintf(AT, "szCAPKExponent == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCAPKExponent length = (%d)", (int)strlen(szCAPKExponent));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srESTRec.szCAPKExponent[0], &szCAPKExponent[0], strlen(szCAPKExponent));

        return (VS_SUCCESS);
}
