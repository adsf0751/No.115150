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
#include "CDTX.h"

static  CDTX_REC srCDTXRec;	/* construct CDTX record */
extern  int     ginDebug;       /* Debug使用 extern */

/*
Function        :inLoadCDTXRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀CDTX檔案，inCDTXRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadCDTXRec(int inCDTXRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆CDTX檔案 */
        char            szCDTXRec[_SIZE_CDTX_REC_ + 1];           /* 暫存, 放各個欄位檔案 */
        char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnCDTXLength = 0;                        /* CDTX總長度 */
        long            lnReadLength;                           /* 記錄每次要從CDTX.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;	                /* inRec記錄讀到第幾筆, i為目前從CDTX讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */

        /* inLoadCDTXRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadCDTXRec(%d) START!!", inCDTXRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 判斷傳進來的inCDTXRec是否小於零 大於等於零才是正確值(防呆) */
        if (inCDTXRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inCDTXRec < 0:(index = %d) ERROR!!", inCDTXRec);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open CDTX.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_CDTX_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnCDTXLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_CDTX_FILE_NAME_);

        if (lnCDTXLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnCDTXLength + 1);
        uszTemp = malloc(lnCDTXLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnCDTXLength + 1);
        memset(uszTemp, 0x00, lnCDTXLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnCDTXLength;

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
         *i為目前從CDTX讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnCDTXLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到CDTX的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnCDTXLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        /* 因為inCDTX_Rec的index從0開始，所以inCDTX_Rec要+1 */
                        if (inRec == (inCDTXRec + 1))
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
         * 如果沒有inCDTXRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inCDTXRec + 1) || inSearchResult == -1)
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
        memset(&srCDTXRec, 0x00, sizeof(srCDTXRec));
        /*
         * 以下pattern為存入CDTX_Rec
         * i為CDTX的第幾個字元
         * 存入CDTX_Rec
         */
        i = 0;


        /* 01_卡別索引 */
        /* 初始化 */
        memset(szCDTXRec, 0x00, sizeof(szCDTXRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCDTXRec[k ++] = uszReadData[i ++];
                if (szCDTXRec[k - 1] == 0x2C	||
		    szCDTXRec[k - 1] == 0x0D	||
		    szCDTXRec[k - 1] == 0x0A	||
		    szCDTXRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCDTXLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "CDTX unpack ERROR.");
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
        if (szCDTXRec[0] != 0x2C	&&
	    szCDTXRec[0] != 0x0D	&&
	    szCDTXRec[0] != 0x0A	&&
	    szCDTXRec[0] != 0x00)
        {
                memcpy(&srCDTXRec.szCardIndex[0], &szCDTXRec[0], k - 1);
        }

	/* 2_卡別名稱(VISA, MASTERCARD, JCB, U CARD, AMEX, DINERS) */
        /* 初始化 */
        memset(szCDTXRec, 0x00, sizeof(szCDTXRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szCDTXRec[k ++] = uszReadData[i ++];
                if (szCDTXRec[k - 1] == 0x2C	||
		    szCDTXRec[k - 1] == 0x0D	||
		    szCDTXRec[k - 1] == 0x0A	||
		    szCDTXRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnCDTXLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "CDTX unpack ERROR");
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
        if (szCDTXRec[0] != 0x2C	&&
	    szCDTXRec[0] != 0x0D	&&
	    szCDTXRec[0] != 0x0A	&&
	    szCDTXRec[0] != 0x00)	/* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srCDTXRec.szCardLabel[0], &szCDTXRec[0], k - 1);
        }

        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadCDTXRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadCDTXRec(%d) END!!", inCDTXRec);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveCDTXRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSaveCDTXRec(int inCDTXRec)
{
        unsigned long   uldat_Handle;                           /* FILE Handle */
        unsigned long   ulbak_Handle;                           /* FILE Handle */
        int	        inPackCount = 0;                        /* uszWriteBuff_Record的index */
        int	        inRecIndex = 0;                         /* uszRead_Total_Buff的index */
        int             inCDTX_Total_Rec = 0;                    /* CDTX.dat的總筆數 */
        int             inRetVal;
        int             i = 0, j = 0;
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long	        lnReadLength = 0;                       /* 每次要從CDTX.dat讀多少byte出來 */
        long            lnCDTXLength = 0;                        /* CDTX.dat檔案總長度 */
        unsigned char	*uszRead_Total_Buff;
        unsigned char   *uszWriteBuff_Record, *uszWriteBuff_Org;


        /* inSaveCDTXRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveCDTXRec(%d)_START!!", inCDTXRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 防呆先刪除CDTX.bak */
        inFILE_Delete((unsigned char *)_CDTX_FILE_NAME_BAK_);

        /* 新建CDTX.bak檔案 */
        inRetVal = inFILE_Create(&ulbak_Handle, (unsigned char *)_CDTX_FILE_NAME_BAK_);

        if (inRetVal != VS_SUCCESS)
        {
                inFILE_Close(&ulbak_Handle);

                return (VS_ERROR);
        }

        /* 開啟原檔案CDTX.dat */
        inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_CDTX_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                /* CDTX.dat開檔失敗 ，不用關檔CDTX.dat */
                /* CDTX.bak仍要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_CDTX_FILE_NAME_BAK_);

                return(VS_ERROR);
        }

        /* 取得CDTX.dat檔案大小 */
        lnCDTXLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *)_CDTX_FILE_NAME_);

        /* 取得檔案大小失敗 */
        if (lnCDTXLength == VS_ERROR)
        {
                /* CDTX.bak和CDTX.dat要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Close(&uldat_Handle);
                inFILE_Delete((unsigned char *)_CDTX_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        uszWriteBuff_Record = malloc(_SIZE_CDTX_REC_ + _SIZE_CDTX_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_CDTX_REC_ + _SIZE_CDTX_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原CDTX.dat的每筆Record 初始化 */
        uszWriteBuff_Org = malloc(_SIZE_CDTX_REC_ + _SIZE_CDTX_COMMA_0D0A_);
        memset(uszWriteBuff_Org, 0x00, _SIZE_CDTX_REC_ + _SIZE_CDTX_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存CDTX.dat全部資料 */
        uszRead_Total_Buff = malloc(lnCDTXLength + 1);
        memset(uszRead_Total_Buff, 0x00, lnCDTXLength + 1);

        inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

        /* CardIndex */
        memcpy(&uszWriteBuff_Record[0], &srCDTXRec.szCardIndex[0], strlen(srCDTXRec.szCardIndex));
        inPackCount += strlen(srCDTXRec.szCardIndex);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* CardLabel */
        memcpy(&uszWriteBuff_Record[inPackCount], &srCDTXRec.szCardLabel[0], strlen(srCDTXRec.szCardLabel));
        inPackCount += strlen(srCDTXRec.szCardLabel);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* 最後的data不用逗號 */
        /* 補上換行符號 */

        /* 0D是移至行首 */
        uszWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        uszWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;

        /* 讀CDTX.dat檔案存到uszRead_Total_Buff */
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnCDTXLength;

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
                                        inFILE_Delete((unsigned char *)_CDTX_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_CDTX_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_CDTX_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        /* 防呆 總record數量小於要存取inCDTXRec Return ERROR */
        /* 算總Record數 */
        for (i = 0; i < (lnCDTXLength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (uszRead_Total_Buff[i] == 0x0A)
                        inCDTX_Total_Rec++;
        }

        /* 防呆 總record數量小於要存取inCDTXRec Return ERROR */
        if ((inCDTXRec + 1) > inCDTX_Total_Rec)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "No data or Index ERROR");
                }

                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_CDTX_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                return (VS_ERROR);
        }

        /* 記錄ReadTotalBuffer的陣列位置 */
        inRecIndex = 0;
        /* 用inCDTXRec決定要先存幾筆Record到CDTX.bak，ex:inCDTXRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inCDTXRec; i ++)
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
                                        inFILE_Delete((unsigned char *)_CDTX_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_CDTX_FILE_NAME_BAK_);

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

        /* 存組好的該CDTXRecord 到 CDTX.bak */
        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_CDTX_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_CDTX_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原CDTX.dat Record 到 CDTX.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
        /* inRecIndex要指到剩下還沒存進bak檔的第一個記憶體位置 */
        inCDTXRec = inCDTXRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < lnCDTXLength; i++)
        {
                if (uszRead_Total_Buff[i] == 0x0A)
                {
                        j++;

                        if (j == inCDTXRec)
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
                /* 接續存原CDTX.dat的Record */
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
                                inFILE_Delete((unsigned char *)_CDTX_FILE_NAME_BAK_);

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
                                inFILE_Delete((unsigned char *)_CDTX_FILE_NAME_BAK_);

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

        /* 刪除原CDTX.dat */
        if (inFILE_Delete((unsigned char *)_CDTX_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* 將CDTX.bak改名字為CDTX.dat取代原檔案 */
        if (inFILE_Rename((unsigned char *)_CDTX_FILE_NAME_BAK_, (unsigned char *)_CDTX_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* inSaveCDTXRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveCDTXRec(%d) END!!", inCDTXRec - 1);
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
Function        :inGetUnknownCardIndex
Date&Time       :
Describe        :
*/
int inGetUnknownCardIndex(char* szCardIndex)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szCardIndex == NULL || strlen(srCDTXRec.szCardIndex) <= 0 || strlen(srCDTXRec.szCardIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetUnknownCardIndex() ERROR !!");

			if (szCardIndex == NULL)
                        {
                                inLogPrintf(AT, "szCardIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCardIndex length = (%d)", (int)strlen(srCDTXRec.szCardIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCardIndex[0], &srCDTXRec.szCardIndex[0], strlen(srCDTXRec.szCardIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetUnknownCardIndex
Date&Time       :
Describe        :
*/
int inSetUnknownCardIndex(char* szCardIndex)
{
        memset(srCDTXRec.szCardIndex, 0x00, sizeof(srCDTXRec.szCardIndex));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szCardIndex == NULL || strlen(szCardIndex) <= 0 || strlen(szCardIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetUnknownCardIndex() ERROR !!");
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
        memcpy(&srCDTXRec.szCardIndex[0], &szCardIndex[0], strlen(szCardIndex));

        return (VS_SUCCESS);
}


/*
Function        :inGetUnknownCardLabel
Date&Time       :2016/11/25 下午 1:21
Describe        :卡別
*/
int inGetUnknownCardLabel(char* szCardLabel)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szCardLabel == NULL || strlen(srCDTXRec.szCardLabel) <= 0 || strlen(srCDTXRec.szCardLabel) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetUnknownCardLabel() ERROR !!");

                        if (szCardLabel == NULL)
                        {
                                inLogPrintf(AT, "szCardLabel == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCardLabel length = (%d)", (int)strlen(srCDTXRec.szCardLabel));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCardLabel[0], &srCDTXRec.szCardLabel[0], strlen(srCDTXRec.szCardLabel));

        return (VS_SUCCESS);
}

/*
Function        :inSetUnknownCardLabel
Date&Time       :2016/11/25 下午 1:22
Describe        :卡別
*/
int inSetUnknownCardLabel(char* szCardLabel)
{
        memset(srCDTXRec.szCardLabel, 0x00, sizeof(srCDTXRec.szCardLabel));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCardLabel == NULL || strlen(szCardLabel) <= 0 || strlen(szCardLabel) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetUnknownCardLabel() ERROR !!");

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
        memcpy(&srCDTXRec.szCardLabel[0], &szCardLabel[0], strlen(szCardLabel));

        return (VS_SUCCESS);
}

/*
Function        :inCDTX_Edit_CDTX_Table
Date&Time       :2017/5/15 下午 4:08
Describe        :
*/
int inCDTX_Edit_CDTX_Table(void)
{
	TABLE_GET_SET_TABLE CDTX_FUNC_TABLE[] =
	{
		{"szCardIndex"			,inGetUnknownCardIndex			,inSetUnknownCardIndex			},
		{"szCardLabel"			,inGetUnknownCardLabel			,inSetUnknownCardLabel			},
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
	inDISP_ChineseFont_Color("是否更改CDTX", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
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
	inLoadCDTXRec(inRecordCnt);
	
	inFunc_Edit_Table_Tag(CDTX_FUNC_TABLE);
	inSaveCDTXRec(inRecordCnt);
	
	return	(VS_SUCCESS);
}

int inCDTX_Test (void)
{
	char    CDTX_Data[1024];
	//int     i = 0;
	memset(CDTX_Data, 0x00, sizeof(CDTX_Data));

	inLoadCDTXRec(0);
	inSetUnknownCardIndex("01");
	inSetUnknownCardLabel("U CARD");
	inSaveCDTXRec(0);


	inLoadCDTXRec(1);
	inSetUnknownCardIndex("02");
	inSetUnknownCardLabel("VISA");
	inSaveCDTXRec(1);

	inLoadCDTXRec(2);
	inSetUnknownCardIndex("03");
	inSetUnknownCardLabel("VISA");
	inSaveCDTXRec(2);

	inLoadCDTXRec(3);
	inSetUnknownCardIndex("04");
	inSetUnknownCardLabel("JCB");
	inSaveCDTXRec(3);

	inLoadCDTXRec(4);
	inSetUnknownCardIndex("05");
	inSetUnknownCardLabel("DINERS");
	inSaveCDTXRec(4);

	inLoadCDTXRec(5);
	inSetUnknownCardIndex("06");
	inSetUnknownCardLabel("AMEX");
	inSaveCDTXRec(5);

	inLoadCDTXRec(6);
	inSetUnknownCardIndex("07");
	inSetUnknownCardLabel("AMEX");
	inSaveCDTXRec(6);

	inLoadCDTXRec(7);
	inSetUnknownCardIndex("08");
	inSetUnknownCardLabel("VISA");
	inSaveCDTXRec(6);


	return 0;
}
