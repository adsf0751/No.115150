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
#include "NCCCtmsFLT.h"

static  TMSFLT_REC srTMSFLTRec;	/* construct TMSFLT record */
extern  int     ginDebug;       /* Debug使用 extern */

/*
Function        :inLoadTMSFLTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀TMSFLT檔案，inTMSFLTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadTMSFLTRec(int inTMSFLTRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆TMSFLT檔案 */
        char            szTMSFLTRec[_SIZE_TMSFLT_REC_ + 1];         /* 暫存, 放各個欄位檔案 */
        char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnTMSFLTLength = 0;                       /* TMSFLT總長度 */
        long            lnReadLength;                           /* 記錄每次要從TMSFLT.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;	                /* inRec記錄讀到第幾筆, i為目前從TMSFLT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */
       
        /* inLoadTMSFLTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadTMSFLTRec(%d) START!!", inTMSFLTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 判斷傳進來的inTMSFLTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inTMSFLTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inTMSFLTRec < 0:(index = %d) ERROR!!", inTMSFLTRec);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open TMSFLT.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_TMSFLT_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnTMSFLTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_TMSFLT_FILE_NAME_);
        
        if (lnTMSFLTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnTMSFLTLength + 1);
        uszTemp = malloc(lnTMSFLTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnTMSFLTLength + 1);
        memset(uszTemp, 0x00, lnTMSFLTLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnTMSFLTLength;

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
         *i為目前從TMSFLT讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnTMSFLTLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到TMSFLT的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnTMSFLTLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        /* 因為inTMSFLT_Rec的index從0開始，所以inTMSFLT_Rec要+1 */
                        if (inRec == (inTMSFLTRec + 1))
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
         * 如果沒有inTMSFLTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inTMSFLTRec + 1) || inSearchResult == -1)
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
        memset(&srTMSFLTRec, 0x00, sizeof(srTMSFLTRec));
        /*
         * 以下pattern為存入TMSFLT_Rec
         * i為TMSFLT的第幾個字元
         * 存入TMSFLT_Rec
         */
        i = 0;


        /* 01_檔案索引 */
        /* 初始化 */
        memset(szTMSFLTRec, 0x00, sizeof(szTMSFLTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFLTRec[k ++] = uszReadData[i ++];
                if (szTMSFLTRec[k - 1] == 0x2C	||
		    szTMSFLTRec[k - 1] == 0x0D	||
		    szTMSFLTRec[k - 1] == 0x0A	||
		    szTMSFLTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFLTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSFLT unpack ERROR.");
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
        if (szTMSFLTRec[0] != 0x2C	&&
	    szTMSFLTRec[0] != 0x0D	&&
	    szTMSFLTRec[0] != 0x0A	&&
	    szTMSFLTRec[0] != 0x00)
        {
                memcpy(&srTMSFLTRec.szTMSFileIndex[0], &szTMSFLTRec[0], k - 1);
        }

        /* 02_檔案屬性 */
        /* 初始化 */
        memset(szTMSFLTRec, 0x00, sizeof(szTMSFLTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFLTRec[k ++] = uszReadData[i ++];
                if (szTMSFLTRec[k - 1] == 0x2C	||
		    szTMSFLTRec[k - 1] == 0x0D	||
		    szTMSFLTRec[k - 1] == 0x0A	||
		    szTMSFLTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFLTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSFLT unpack ERROR");
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
        if (szTMSFLTRec[0] != 0x2C	&&
	    szTMSFLTRec[0] != 0x0D	&&
	    szTMSFLTRec[0] != 0x0A	&&
	    szTMSFLTRec[0] != 0x00)
        {
                memcpy(&srTMSFLTRec.szTMSFileAttribute[0], &szTMSFLTRec[0], k - 1);
        }

        /* 03_檔案路徑及檔案名稱 */
        /* 初始化 */
        memset(szTMSFLTRec, 0x00, sizeof(szTMSFLTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFLTRec[k ++] = uszReadData[i ++];
                if (szTMSFLTRec[k - 1] == 0x2C	||
		    szTMSFLTRec[k - 1] == 0x0D	||
		    szTMSFLTRec[k - 1] == 0x0A	||
		    szTMSFLTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFLTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSFLT unpack ERROR");
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
        if (szTMSFLTRec[0] != 0x2C	&&
	    szTMSFLTRec[0] != 0x0D	&&
	    szTMSFLTRec[0] != 0x0A	&&
	    szTMSFLTRec[0] != 0x00)
        {
                memcpy(&srTMSFLTRec.szTMSFilePathName[0], &szTMSFLTRec[0], k - 1);
        }

        /* 04_檔案大小 */
        /* 初始化 */
        memset(szTMSFLTRec, 0x00, sizeof(szTMSFLTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSFLTRec[k ++] = uszReadData[i ++];
                if (szTMSFLTRec[k - 1] == 0x2C	||
		    szTMSFLTRec[k - 1] == 0x0D	||
		    szTMSFLTRec[k - 1] == 0x0A	||
		    szTMSFLTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSFLTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSFLT unpack ERROR");
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
        if (szTMSFLTRec[0] != 0x2C	&&
	    szTMSFLTRec[0] != 0x0D	&&
	    szTMSFLTRec[0] != 0x0A	&&
	    szTMSFLTRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srTMSFLTRec.szTMSFileSize[0], &szTMSFLTRec[0], k - 1);
        }

        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadTMSFLTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadTMSFLTRec(%d) END!!", inTMSFLTRec);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveTMSFLTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSaveTMSFLTRec(int inTMSFLTRec)
{
        unsigned long   uldat_Handle;                           /* FILE Handle */
        unsigned long   ulbak_Handle;                           /* FILE Handle */
        int	        inPackCount = 0;                        /* uszWriteBuff_Record的index */
        int	        inRecIndex = 0;                         /* uszRead_Total_Buff的index */
        int             inTMSFLT_Total_Rec = 0;                    /* TMSFLT.dat的總筆數 */
        int             inRetVal;
        int             i = 0, j = 0;
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long	        lnReadLength = 0;                       /* 每次要從TMSFLT.dat讀多少byte出來 */
        long            lnTMSFLTLength = 0;                        /* TMSFLT.dat檔案總長度 */
        unsigned char	*uszRead_Total_Buff;
        unsigned char   *uszWriteBuff_Record, *uszWriteBuff_Org;


        /* inSaveTMSFLTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveTMSFLTRec(%d)_START!!", inTMSFLTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 防呆先刪除TMSFLT.bak */
        inFILE_Delete((unsigned char *)_TMSFLT_FILE_NAME_BAK_);

        /* 新建TMSFLT.bak檔案 */
        inRetVal = inFILE_Create(&ulbak_Handle, (unsigned char *)_TMSFLT_FILE_NAME_BAK_);

        if (inRetVal != VS_SUCCESS)
        {
                inFILE_Close(&ulbak_Handle);

                return (VS_ERROR);
        }

        /* 開啟原檔案TMSFLT.dat */
        inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_TMSFLT_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                /* TMSFLT.dat開檔失敗 ，不用關檔TMSFLT.dat */
                /* TMSFLT.bak仍要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_TMSFLT_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 取得TMSFLT.dat檔案大小 */
        lnTMSFLTLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *)_TMSFLT_FILE_NAME_);

        /* 取得檔案大小失敗 */
        if (lnTMSFLTLength == VS_ERROR)
        {
                /* TMSFLT.bak和TMSFLT.dat要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Close(&uldat_Handle);
                inFILE_Delete((unsigned char *)_TMSFLT_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        uszWriteBuff_Record = malloc(_SIZE_TMSFLT_REC_ + _SIZE_TMSFLT_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_TMSFLT_REC_ + _SIZE_TMSFLT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原TMSFLT.dat的每筆Record 初始化 */
        uszWriteBuff_Org = malloc(_SIZE_TMSFLT_REC_ + _SIZE_TMSFLT_COMMA_0D0A_);
        memset(uszWriteBuff_Org, 0x00, _SIZE_TMSFLT_REC_ + _SIZE_TMSFLT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存TMSFLT.dat全部資料 */
        uszRead_Total_Buff = malloc(lnTMSFLTLength + 1);
        memset(uszRead_Total_Buff, 0x00, lnTMSFLTLength + 1);

        inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

        /* TMSFileIndex */
        memcpy(&uszWriteBuff_Record[0], &srTMSFLTRec.szTMSFileIndex[0], strlen(srTMSFLTRec.szTMSFileIndex));
        inPackCount += strlen(srTMSFLTRec.szTMSFileIndex);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TMSFileAttribute */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTMSFLTRec.szTMSFileAttribute[0], strlen(srTMSFLTRec.szTMSFileAttribute));
        inPackCount += strlen(srTMSFLTRec.szTMSFileAttribute);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TMSFilePathName */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTMSFLTRec.szTMSFilePathName[0], strlen(srTMSFLTRec.szTMSFilePathName));
        inPackCount += strlen(srTMSFLTRec.szTMSFilePathName);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* TMSFileSize */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTMSFLTRec.szTMSFileSize[0], strlen(srTMSFLTRec.szTMSFileSize));
        inPackCount += strlen(srTMSFLTRec.szTMSFileSize);

        /* 最後的data不用逗號 */
        /* 補上換行符號 */

        /* 0D是移至行首 */
        uszWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        uszWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;

        /* 讀TMSFLT.dat檔案存到uszRead_Total_Buff */
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnTMSFLTLength;

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
                                        inFILE_Delete((unsigned char *)_TMSFLT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_TMSFLT_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_TMSFLT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        /* 防呆 總record數量小於要存取inTMSFLTRec Return ERROR */
        /* 算總Record數 */
        for (i = 0; i < (lnTMSFLTLength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (uszRead_Total_Buff[i] == 0x0A)
                        inTMSFLT_Total_Rec++;
        }

        /* 防呆 總record數量小於要存取inTMSFLTRec Return ERROR */
        if ((inTMSFLTRec + 1) > inTMSFLT_Total_Rec)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "No data or Index ERROR");
                }

                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_TMSFLT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                return (VS_ERROR);
        }

        /* 記錄ReadTotalBuffer的陣列位置 */
        inRecIndex = 0;
        /* 用inTMSFLTRec決定要先存幾筆Record到TMSFLT.bak，ex:inTMSFLTRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inTMSFLTRec; i ++)
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
                                        inFILE_Delete((unsigned char *)_TMSFLT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_TMSFLT_FILE_NAME_BAK_);

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

        /* 存組好的該TMSFLTRecord 到 TMSFLT.bak */
        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_TMSFLT_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_TMSFLT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原TMSFLT.dat Record 到 TMSFLT.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
        /* inRecIndex要指到剩下還沒存進bak檔的第一個記憶體位置 */
        inTMSFLTRec = inTMSFLTRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < lnTMSFLTLength; i++)
        {
                if (uszRead_Total_Buff[i] == 0x0A)
                {
                        j++;

                        if (j == inTMSFLTRec)
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
                /* 接續存原TMSFLT.dat的Record */
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
                                inFILE_Delete((unsigned char *)_TMSFLT_FILE_NAME_BAK_);

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
                                inFILE_Delete((unsigned char *)_TMSFLT_FILE_NAME_BAK_);

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

        /* 刪除原TMSFLT.dat */
        if (inFILE_Delete((unsigned char *)_TMSFLT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* 將TMSFLT.bak改名字為TMSFLT.dat取代原檔案 */
        if (inFILE_Rename((unsigned char *)_TMSFLT_FILE_NAME_BAK_, (unsigned char *)_TMSFLT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* inSaveTMSFLTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveTMSFLTRec(%d) END!!", inTMSFLTRec - 1);
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
Function        :inGetTMSFileIndex
Date&Time       :
Describe        :
*/
int inGetTMSFileIndex(char* szTMSFileIndex)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSFileIndex == NULL || strlen(srTMSFLTRec.szTMSFileIndex) <= 0 || strlen(srTMSFLTRec.szTMSFileIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTMSFileIndex() ERROR !!");
                        
			if (szTMSFileIndex == NULL)
                        {
                                inLogPrintf(AT, "szTMSFileIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSFileIndex length = (%d)", (int)strlen(srTMSFLTRec.szTMSFileIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSFileIndex[0], &srTMSFLTRec.szTMSFileIndex[0], strlen(srTMSFLTRec.szTMSFileIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSFileIndex
Date&Time       :
Describe        :
*/
int inSetTMSFileIndex(char* szTMSFileIndex)
{
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTMSFileIndex == NULL || strlen(szTMSFileIndex) <= 0 || strlen(szTMSFileIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTMSFileIndex() ERROR !!");
                        if (szTMSFileIndex == NULL)
                        {
                                inLogPrintf(AT, "szTMSFileIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSFileIndex length = (%d)", (int)strlen(szTMSFileIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSFLTRec.szTMSFileIndex, 0x00, sizeof(srTMSFLTRec.szTMSFileIndex));
        memcpy(&srTMSFLTRec.szTMSFileIndex[0], &szTMSFileIndex[0], strlen(szTMSFileIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMSFileAttribute
Date&Time       :
Describe        :
*/
int inGetTMSFileAttribute(char* szTMSFileAttribute)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSFileAttribute == NULL || strlen(srTMSFLTRec.szTMSFileAttribute) <= 0 || strlen(srTMSFLTRec.szTMSFileAttribute) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTMSFileAttribute() ERROR !!");

                        if (szTMSFileAttribute == NULL)
                        {
                                inLogPrintf(AT, "szTMSFileAttribute == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSFileAttribute length = (%d)", (int)strlen(srTMSFLTRec.szTMSFileAttribute));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSFileAttribute[0], &srTMSFLTRec.szTMSFileAttribute[0], strlen(srTMSFLTRec.szTMSFileAttribute));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSFileAttribute
Date&Time       :
Describe        :
*/
int inSetTMSFileAttribute(char* szTMSFileAttribute)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTMSFileAttribute == NULL || strlen(szTMSFileAttribute) <= 0 || strlen(szTMSFileAttribute) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTMSFileAttribute() ERROR !!");

                        if (szTMSFileAttribute == NULL)
                        {
                                inLogPrintf(AT, "szTMSFileAttribute == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSFileAttribute length = (%d)", (int)strlen(szTMSFileAttribute));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSFLTRec.szTMSFileAttribute, 0x00, sizeof(srTMSFLTRec.szTMSFileAttribute));
        memcpy(&srTMSFLTRec.szTMSFileAttribute[0], &szTMSFileAttribute[0], strlen(szTMSFileAttribute));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMSFilePathName
Date&Time       :
Describe        :
*/
int inGetTMSFilePathName(char* szTMSFilePathName)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSFilePathName == NULL || strlen(srTMSFLTRec.szTMSFilePathName) <= 0 || strlen(srTMSFLTRec.szTMSFilePathName) > 60)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTMSFilePathName() ERROR !!");

                        if (szTMSFilePathName == NULL)
                        {
                                inLogPrintf(AT, "szTMSFilePathName == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSFilePathName length = (%d)", (int)strlen(srTMSFLTRec.szTMSFilePathName));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSFilePathName[0], &srTMSFLTRec.szTMSFilePathName[0], strlen(srTMSFLTRec.szTMSFilePathName));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSFilePathName
Date&Time       :
Describe        :
*/
int inSetTMSFilePathName(char* szTMSFilePathName)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTMSFilePathName == NULL || strlen(szTMSFilePathName) <= 0 || strlen(szTMSFilePathName) > 60)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTMSFilePathName() ERROR !!");

                        if (szTMSFilePathName == NULL)
                        {
                                inLogPrintf(AT, "szTMSFilePathName == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSFilePathName length = (%d)", (int)strlen(szTMSFilePathName));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSFLTRec.szTMSFilePathName, 0x00, sizeof(srTMSFLTRec.szTMSFilePathName));
        memcpy(&srTMSFLTRec.szTMSFilePathName[0], &szTMSFilePathName[0], strlen(szTMSFilePathName));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMSFileSize
Date&Time       :
Describe        :
*/
int inGetTMSFileSize(char* szTMSFileSize)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSFileSize == NULL || strlen(srTMSFLTRec.szTMSFileSize) <= 0 || strlen(srTMSFLTRec.szTMSFileSize) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTMSFileSize() ERROR !!");

                        if (szTMSFileSize == NULL)
                        {
                                inLogPrintf(AT, "szTMSFileSize == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSFileSize length = (%d)", (int)strlen(srTMSFLTRec.szTMSFileSize));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSFileSize[0], &srTMSFLTRec.szTMSFileSize[0], strlen(srTMSFLTRec.szTMSFileSize));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSFileSize
Date&Time       :
Describe        :
*/
int inSetTMSFileSize(char* szTMSFileSize)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTMSFileSize == NULL || strlen(szTMSFileSize) <= 0 || strlen(szTMSFileSize) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTMSFileSize() ERROR !!");

                        if (szTMSFileSize == NULL)
                        {
                                inLogPrintf(AT, "szTMSFileSize == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSFileSize length = (%d)", (int)strlen(szTMSFileSize));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srTMSFLTRec.szTMSFileSize, 0x00, sizeof(srTMSFLTRec.szTMSFileSize));
        memcpy(&srTMSFLTRec.szTMSFileSize[0], &szTMSFileSize[0], strlen(szTMSFileSize));

        return (VS_SUCCESS);
}
