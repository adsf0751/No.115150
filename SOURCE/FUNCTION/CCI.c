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
#include "CCI.h"

static	CCI_REC srCCIRec;
extern  int     ginDebug;       /* Debug使用 extern */

/*
Function        :inLoadCCIRec
Date&Time       :2016/8/22 下午 2:17
Describe        :讀CCI檔案，inCCIRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadCCIRec(int inCCIRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆CCI檔案 */
        char            szCCIRec[_SIZE_CCI_REC_ + 1];           /* 暫存, 放各個欄位檔案 */
        char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnCCILength = 0;                        /* CCI總長度 */
        long            lnReadLength;                           /* 記錄每次要從CCI.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;	                /* inRec記錄讀到第幾筆, i為目前從CCI讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */

        /* inLoadCCIRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadCCIRec(%d) START!!", inCCIRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 判斷傳進來的inCCIRec是否小於零 大於等於零才是正確值(防呆) */
        if (inCCIRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inCCIRec < 0:(index = %d) ERROR!!", inCCIRec);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open CCI.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_CCI_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnCCILength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_CCI_FILE_NAME_);

        if (lnCCILength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnCCILength + 1);
        uszTemp = malloc(lnCCILength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnCCILength + 1);
        memset(uszTemp, 0x00, lnCCILength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnCCILength;

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
         *i為目前從CCI讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnCCILength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到CCI的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnCCILength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        /* 因為inCCI_Rec的index從0開始，所以inCCI_Rec要+1 */
                        if (inRec == (inCCIRec + 1))
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
         * 如果沒有inCCIRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inCCIRec + 1) || inSearchResult == -1)
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
        memset(&srCCIRec, 0x00, sizeof(srCCIRec));
        /*
         * 以下pattern為存入CCI_Rec
         * i為CCI的第幾個字元
         * 存入CCI_Rec
         */
        i = 0;


        /* 01_貨幣索引 */
        /* 初始化 */
        memset(szCCIRec, 0x00, sizeof(szCCIRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCCIRec[k ++] = uszReadData[i ++];
                if (szCCIRec[k - 1] == 0x2C	||
		    szCCIRec[k - 1] == 0x0D	||
		    szCCIRec[k - 1] == 0x0A	||
		    szCCIRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCCILength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "CCI unpack ERROR.");
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
        if (szCCIRec[0] != 0x2C	&&
	    szCCIRec[0] != 0x0D	&&
	    szCCIRec[0] != 0x0A	&&
	    szCCIRec[0] != 0x00)
        {
                memcpy(&srCCIRec.szDCCCurrencyIndex[0], &szCCIRec[0], k - 1);
        }

        /* 02_貨幣名稱 */
        /* 初始化 */
        memset(szCCIRec, 0x00, sizeof(szCCIRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCCIRec[k ++] = uszReadData[i ++];
                if (szCCIRec[k - 1] == 0x2C	||
		    szCCIRec[k - 1] == 0x0D	||
		    szCCIRec[k - 1] == 0x0A	||
		    szCCIRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCCILength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "CCI unpack ERROR");
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
        if (szCCIRec[0] != 0x2C	&&
	    szCCIRec[0] != 0x0D	&&
	    szCCIRec[0] != 0x0A	&&
	    szCCIRec[0] != 0x00)
        {
                memcpy(&srCCIRec.szDCCCurrencyName[0], &szCCIRec[0], k - 1);
        }

        /* 03_幣別碼 */
        /* 初始化 */
        memset(szCCIRec, 0x00, sizeof(szCCIRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCCIRec[k ++] = uszReadData[i ++];
                if (szCCIRec[k - 1] == 0x2C	||
		    szCCIRec[k - 1] == 0x0D	||
		    szCCIRec[k - 1] == 0x0A	||
		    szCCIRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCCILength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "CCI unpack ERROR");
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
        if (szCCIRec[0] != 0x2C	&&
	    szCCIRec[0] != 0x0D	&&
	    szCCIRec[0] != 0x0A	&&
	    szCCIRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srCCIRec.szDCCCurrencyCode[0], &szCCIRec[0], k - 1);
        }

        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadCCIRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadCCIRec(%d) END!!", inCCIRec);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveCCIRec
Date&Time       :2016/8/22 下午 2:16
Describe        :
*/
int inSaveCCIRec(int inCCIRec)
{
        unsigned long   uldat_Handle;                           /* FILE Handle */
        unsigned long   ulbak_Handle;                           /* FILE Handle */
        int	        inPackCount = 0;                        /* uszWriteBuff_Record的index */
        int	        inRecIndex = 0;                         /* uszRead_Total_Buff的index */
        int             inCCI_Total_Rec = 0;                    /* CCI.dat的總筆數 */
        int             inRetVal;
        int             i = 0, j = 0;
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long	        lnReadLength = 0;                       /* 每次要從CCI.dat讀多少byte出來 */
        long            lnCCILength = 0;                        /* CCI.dat檔案總長度 */
        unsigned char	*uszRead_Total_Buff;
        unsigned char   *uszWriteBuff_Record, *uszWriteBuff_Org;


        /* inSaveCCIRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveCCIRec(%d)_START!!", inCCIRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 防呆先刪除CCI.bak */
        inFILE_Delete((unsigned char *)_CCI_FILE_NAME_BAK_);

        /* 新建CCI.bak檔案 */
        inRetVal = inFILE_Create(&ulbak_Handle, (unsigned char *)_CCI_FILE_NAME_BAK_);

        if (inRetVal != VS_SUCCESS)
        {
                inFILE_Close(&ulbak_Handle);

                return (VS_ERROR);
        }

        /* 開啟原檔案CCI.dat */
        inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_CCI_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                /* CCI.dat開檔失敗 ，不用關檔CCI.dat */
                /* CCI.bak仍要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_CCI_FILE_NAME_BAK_);

                return(VS_ERROR);
        }

        /* 取得CCI.dat檔案大小 */
        lnCCILength = lnFILE_GetSize(&uldat_Handle, (unsigned char *)_CCI_FILE_NAME_);

        /* 取得檔案大小失敗 */
        if (lnCCILength == VS_ERROR)
        {
                /* CCI.bak和CCI.dat要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Close(&uldat_Handle);
                inFILE_Delete((unsigned char *)_CCI_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        uszWriteBuff_Record = malloc(_SIZE_CCI_REC_ + _SIZE_CCI_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_CCI_REC_ + _SIZE_CCI_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原CCI.dat的每筆Record 初始化 */
        uszWriteBuff_Org = malloc(_SIZE_CCI_REC_ + _SIZE_CCI_COMMA_0D0A_);
        memset(uszWriteBuff_Org, 0x00, _SIZE_CCI_REC_ + _SIZE_CCI_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存CCI.dat全部資料 */
        uszRead_Total_Buff = malloc(lnCCILength + 1);
        memset(uszRead_Total_Buff, 0x00, lnCCILength + 1);

        inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

        /* 貨幣索引 */
        memcpy(&uszWriteBuff_Record[0], &srCCIRec.szDCCCurrencyIndex[0], strlen(srCCIRec.szDCCCurrencyIndex));
        inPackCount += strlen(srCCIRec.szDCCCurrencyIndex);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* 貨幣名稱 */
        memcpy(&uszWriteBuff_Record[inPackCount], &srCCIRec.szDCCCurrencyName[0], strlen(srCCIRec.szDCCCurrencyName));
        inPackCount += strlen(srCCIRec.szDCCCurrencyName);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* 幣別碼 */
        memcpy(&uszWriteBuff_Record[inPackCount], &srCCIRec.szDCCCurrencyCode[0], strlen(srCCIRec.szDCCCurrencyCode));
        inPackCount += strlen(srCCIRec.szDCCCurrencyCode);

        /* 最後的data不用逗號 */
        /* 補上換行符號 */

        /* 0D是移至行首 */
        uszWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        uszWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;

        /* 讀CCI.dat檔案存到uszRead_Total_Buff */
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnCCILength;

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
                                        inFILE_Delete((unsigned char *)_CCI_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_CCI_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_CCI_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        /* 防呆 總record數量小於要存取inCCIRec Return ERROR */
        /* 算總Record數 */
        for (i = 0; i < (lnCCILength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (uszRead_Total_Buff[i] == 0x0A)
                        inCCI_Total_Rec++;
        }

        /* 防呆 總record數量小於要存取inCCIRec Return ERROR */
        if ((inCCIRec + 1) > inCCI_Total_Rec)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "No data or Index ERROR");
                }

                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_CCI_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                return (VS_ERROR);
        }

        /* 記錄ReadTotalBuffer的陣列位置 */
        inRecIndex = 0;
        /* 用inCCIRec決定要先存幾筆Record到CCI.bak，ex:inCCIRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inCCIRec; i ++)
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
                                        inFILE_Delete((unsigned char *)_CCI_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_CCI_FILE_NAME_BAK_);

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

        /* 存組好的該CCIRecord 到 CCI.bak */
        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_CCI_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_CCI_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原CCI.dat Record 到 CCI.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
        /* inRecIndex要指到剩下還沒存進bak檔的第一個記憶體位置 */
        inCCIRec = inCCIRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < lnCCILength; i++)
        {
                if (uszRead_Total_Buff[i] == 0x0A)
                {
                        j++;

                        if (j == inCCIRec)
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
                /* 接續存原CCI.dat的Record */
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
                                inFILE_Delete((unsigned char *)_CCI_FILE_NAME_BAK_);

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
                                inFILE_Delete((unsigned char *)_CCI_FILE_NAME_BAK_);

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

        /* 刪除原CCI.dat */
        if (inFILE_Delete((unsigned char *)_CCI_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* 將CCI.bak改名字為CCI.dat取代原檔案 */
        if (inFILE_Rename((unsigned char *)_CCI_FILE_NAME_BAK_, (unsigned char *)_CCI_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* inSaveCCIRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveCCIRec(%d) END!!", inCCIRec - 1);
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
Function        :inGetDCCCurrencyIndex
Date&Time       :
Describe        :
*/
int inGetDCCCurrencyIndex(char* szDCCCurrencyIndex)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szDCCCurrencyIndex == NULL || strlen(srCCIRec.szDCCCurrencyIndex) <= 0 || strlen(srCCIRec.szDCCCurrencyIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetDCCCurrencyIndex() ERROR !!");

			if (szDCCCurrencyIndex == NULL)
                        {
                                inLogPrintf(AT, "szDCCCurrencyIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDCCCurrencyIndex length = (%d)", strlen(srCCIRec.szDCCCurrencyIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szDCCCurrencyIndex[0], &srCCIRec.szDCCCurrencyIndex[0], strlen(srCCIRec.szDCCCurrencyIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetDCCCurrencyIndex
Date&Time       :
Describe        :
*/
int inSetDCCCurrencyIndex(char* szDCCCurrencyIndex)
{
        memset(srCCIRec.szDCCCurrencyIndex, 0x00, sizeof(srCCIRec.szDCCCurrencyIndex));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szDCCCurrencyIndex == NULL || strlen(szDCCCurrencyIndex) <= 0 || strlen(szDCCCurrencyIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetDCCCurrencyIndex() ERROR !!");
                        if (szDCCCurrencyIndex == NULL)
                        {
                                inLogPrintf(AT, "szDCCCurrencyIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDCCCurrencyIndex length = (%d)", strlen(szDCCCurrencyIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCCIRec.szDCCCurrencyIndex[0], &szDCCCurrencyIndex[0], strlen(szDCCCurrencyIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetDCCCurrencyName
Date&Time       :
Describe        :
*/
int inGetDCCCurrencyName(char* szDCCCurrencyName)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szDCCCurrencyName == NULL || strlen(srCCIRec.szDCCCurrencyName) <= 0 || strlen(srCCIRec.szDCCCurrencyName) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetDCCCurrencyName() ERROR !!");

                        if (szDCCCurrencyName == NULL)
                        {
                                inLogPrintf(AT, "szDCCCurrencyName == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDCCCurrencyName length = (%d)", strlen(srCCIRec.szDCCCurrencyName));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szDCCCurrencyName[0], &srCCIRec.szDCCCurrencyName[0], strlen(srCCIRec.szDCCCurrencyName));

        return (VS_SUCCESS);
}

/*
Function        :inSetDCCCurrencyName
Date&Time       :
Describe        :
*/
int inSetDCCCurrencyName(char* szDCCCurrencyName)
{
        memset(srCCIRec.szDCCCurrencyName, 0x00, sizeof(srCCIRec.szDCCCurrencyName));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szDCCCurrencyName == NULL || strlen(szDCCCurrencyName) <= 0 || strlen(szDCCCurrencyName) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetDCCCurrencyName() ERROR !!");

                        if (szDCCCurrencyName == NULL)
                        {
                                inLogPrintf(AT, "szDCCCurrencyName == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDCCCurrencyName length = (%d)", strlen(szDCCCurrencyName));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCCIRec.szDCCCurrencyName[0], &szDCCCurrencyName[0], strlen(szDCCCurrencyName));

        return (VS_SUCCESS);
}

/*
Function        :inGetDCCCurrencyCode
Date&Time       :
Describe        :
*/
int inGetDCCCurrencyCode(char* szDCCCurrencyCode)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szDCCCurrencyCode == NULL || strlen(srCCIRec.szDCCCurrencyCode) <= 0 || strlen(srCCIRec.szDCCCurrencyCode) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetDCCCurrencyCode() ERROR !!");

                        if (szDCCCurrencyCode == NULL)
                        {
                                inLogPrintf(AT, "szDCCCurrencyCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDCCCurrencyCode length = (%d)", strlen(srCCIRec.szDCCCurrencyCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szDCCCurrencyCode[0], &srCCIRec.szDCCCurrencyCode[0], strlen(srCCIRec.szDCCCurrencyCode));

        return (VS_SUCCESS);
}

/*
Function        :inSetDCCCurrencyCode
Date&Time       :
Describe        :
*/
int inSetDCCCurrencyCode(char* szDCCCurrencyCode)
{
        memset(srCCIRec.szDCCCurrencyCode, 0x00, sizeof(srCCIRec.szDCCCurrencyCode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szDCCCurrencyCode == NULL || strlen(szDCCCurrencyCode) <= 0 || strlen(szDCCCurrencyCode) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetDCCCurrencyCode() ERROR !!");

                        if (szDCCCurrencyCode == NULL)
                        {
                                inLogPrintf(AT, "szDCCCurrencyCode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDCCCurrencyCode length = (%d)", strlen(szDCCCurrencyCode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCCIRec.szDCCCurrencyCode[0], &szDCCCurrencyCode[0], strlen(szDCCCurrencyCode));

        return (VS_SUCCESS);
}