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
#include "Big5NameT.h"

static	Big5NameT_REC	srBig5NameTRec;
extern  int		ginDebug;       /* Debug使用 extern */

/*
Function        :inLoadBig5NameTRec
Date&Time       :2025/5/21 下午 4:23
Describe        :讀Big5NameT檔案，inBig5NameTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadBig5NameTRec(int inBig5NameTRec)
{
        unsigned long   ulFile_Handle;					/* File Handle */
        unsigned char   *uszReadData;					/* 放抓到的record */
        unsigned char   *uszTemp;					/* 暫存，放整筆Big5NameT檔案 */
        char            szBig5NameTRec[_SIZE_Big5NameT_REC_ + 1];	/* 暫存, 放各個欄位檔案 */
        char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];		/* debug message */
        long            lnBig5NameTLength = 0;				/* Big5NameT總長度 */
        long            lnReadLength;					/* 記錄每次要從Big5NameT.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;				/* inRec記錄讀到第幾筆, i為目前從Big5NameT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;				/* 判斷有沒有讀到0x0D 0x0A的Flag */

        /* inLoadBig5NameTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadBig5NameTRec(%d) START!!", inBig5NameTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 判斷傳進來的inBig5NameTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inBig5NameTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inBig5NameTRec < 0:(index = %d) ERROR!!", inBig5NameTRec);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open Big5NameT.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_Big5NameT_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnBig5NameTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_Big5NameT_FILE_NAME_);

        if (lnBig5NameTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnBig5NameTLength + 1);
        uszTemp = malloc(lnBig5NameTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnBig5NameTLength + 1);
        memset(uszTemp, 0x00, lnBig5NameTLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnBig5NameTLength;

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
         *i為目前從Big5NameT讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnBig5NameTLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到Big5NameT的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnBig5NameTLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        /* 因為inBig5NameT_Rec的index從0開始，所以inBig5NameT_Rec要+1 */
                        if (inRec == (inBig5NameTRec + 1))
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
         * 如果沒有inBig5NameTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inBig5NameTRec + 1) || inSearchResult == -1)
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
        memset(&srBig5NameTRec, 0x00, sizeof(srBig5NameTRec));
        /*
         * 以下pattern為存入Big5NameT_Rec
         * i為Big5NameT的第幾個字元
         * 存入Big5NameT_Rec
         */
        i = 0;


        /* 01_表頭索引(最多兩行) */
        /* 初始化 */
        memset(szBig5NameTRec, 0x00, sizeof(szBig5NameTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szBig5NameTRec[k ++] = uszReadData[i ++];
                if (szBig5NameTRec[k - 1] == 0x2C	||
		    szBig5NameTRec[k - 1] == 0x0D	||
		    szBig5NameTRec[k - 1] == 0x0A	||
		    szBig5NameTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnBig5NameTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "Big5NameT unpack ERROR.");
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
        if (szBig5NameTRec[0] != 0x2C	&&
	    szBig5NameTRec[0] != 0x0D	&&
	    szBig5NameTRec[0] != 0x0A	&&
	    szBig5NameTRec[0] != 0x00)
        {
                memcpy(&srBig5NameTRec.szMerchant_Name_Index[0], &szBig5NameTRec[0], k - 1);
        }

        /* 02_表頭(32個半形字，16個全形字) */
        /* 初始化 */
        memset(szBig5NameTRec, 0x00, sizeof(szBig5NameTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szBig5NameTRec[k ++] = uszReadData[i ++];
                if (szBig5NameTRec[k - 1] == 0x2C	||
		    szBig5NameTRec[k - 1] == 0x0D	||
		    szBig5NameTRec[k - 1] == 0x0A	||
		    szBig5NameTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnBig5NameTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "Big5NameT unpack ERROR");
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
        if (szBig5NameTRec[0] != 0x2C	&&
	    szBig5NameTRec[0] != 0x0D	&&
	    szBig5NameTRec[0] != 0x0A	&&
	    szBig5NameTRec[0] != 0x00)
        {
                memcpy(&srBig5NameTRec.szMerchant_Name[0], &szBig5NameTRec[0], k - 1);
        }

        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadBig5NameTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadBig5NameTRec(%d) END!!", inBig5NameTRec);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveBig5NameTRec
Date&Time       :2025/5/21 下午 4:46
Describe        :
*/
int inSaveBig5NameTRec(int inBig5NameTRec)
{
        unsigned long   uldat_Handle;                           /* FILE Handle */
        unsigned long   ulbak_Handle;                           /* FILE Handle */
        int	        inPackCount = 0;                        /* uszWriteBuff_Record的index */
        int	        inRecIndex = 0;                         /* uszRead_Total_Buff的index */
        int             inBig5NameT_Total_Rec = 0;                    /* Big5NameT.dat的總筆數 */
        int             inRetVal;
        int             i = 0, j = 0;
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long	        lnReadLength = 0;                       /* 每次要從Big5NameT.dat讀多少byte出來 */
        long            lnBig5NameTLength = 0;                        /* Big5NameT.dat檔案總長度 */
        unsigned char	*uszRead_Total_Buff;
        unsigned char   *uszWriteBuff_Record, *uszWriteBuff_Org;


        /* inSaveBig5NameTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveBig5NameTRec(%d)_START!!", inBig5NameTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 防呆先刪除Big5NameT.bak */
        inFILE_Delete((unsigned char *)_Big5NameT_FILE_NAME_BAK_);

        /* 新建Big5NameT.bak檔案 */
        inRetVal = inFILE_Create(&ulbak_Handle, (unsigned char *)_Big5NameT_FILE_NAME_BAK_);

        if (inRetVal != VS_SUCCESS)
        {
                inFILE_Close(&ulbak_Handle);

                return (VS_ERROR);
        }

        /* 開啟原檔案Big5NameT.dat */
        inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_Big5NameT_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                /* Big5NameT.dat開檔失敗 ，不用關檔Big5NameT.dat */
                /* Big5NameT.bak仍要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_Big5NameT_FILE_NAME_BAK_);

                return(VS_ERROR);
        }

        /* 取得Big5NameT.dat檔案大小 */
        lnBig5NameTLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *)_Big5NameT_FILE_NAME_);

        /* 取得檔案大小失敗 */
        if (lnBig5NameTLength == VS_ERROR)
        {
                /* Big5NameT.bak和Big5NameT.dat要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Close(&uldat_Handle);
                inFILE_Delete((unsigned char *)_Big5NameT_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        uszWriteBuff_Record = malloc(_SIZE_Big5NameT_REC_ + _SIZE_Big5NameT_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_Big5NameT_REC_ + _SIZE_Big5NameT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原Big5NameT.dat的每筆Record 初始化 */
        uszWriteBuff_Org = malloc(_SIZE_Big5NameT_REC_ + _SIZE_Big5NameT_COMMA_0D0A_);
        memset(uszWriteBuff_Org, 0x00, _SIZE_Big5NameT_REC_ + _SIZE_Big5NameT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存Big5NameT.dat全部資料 */
        uszRead_Total_Buff = malloc(lnBig5NameTLength + 1);
        memset(uszRead_Total_Buff, 0x00, lnBig5NameTLength + 1);

        inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

        /* 表頭索引 */
        memcpy(&uszWriteBuff_Record[0], &srBig5NameTRec.szMerchant_Name_Index[0], strlen(srBig5NameTRec.szMerchant_Name_Index));
        inPackCount += strlen(srBig5NameTRec.szMerchant_Name_Index);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* 表頭(32個半形字，16個全形字) */
        memcpy(&uszWriteBuff_Record[inPackCount], &srBig5NameTRec.szMerchant_Name[0], strlen(srBig5NameTRec.szMerchant_Name));
        inPackCount += strlen(srBig5NameTRec.szMerchant_Name);

        /* 最後的data不用逗號 */
        /* 補上換行符號 */

        /* 0D是移至行首 */
        uszWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        uszWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;

        /* 讀Big5NameT.dat檔案存到uszRead_Total_Buff */
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnBig5NameTLength;

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
                                        inFILE_Delete((unsigned char *)_Big5NameT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_Big5NameT_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_Big5NameT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        /* 防呆 總record數量小於要存取inBig5NameTRec Return ERROR */
        /* 算總Record數 */
        for (i = 0; i < (lnBig5NameTLength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (uszRead_Total_Buff[i] == 0x0A)
                        inBig5NameT_Total_Rec++;
        }

        /* 防呆 總record數量小於要存取inBig5NameTRec Return ERROR */
        if ((inBig5NameTRec + 1) > inBig5NameT_Total_Rec)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "No data or Index ERROR");
                }

                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_Big5NameT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                return (VS_ERROR);
        }

        /* 記錄ReadTotalBuffer的陣列位置 */
        inRecIndex = 0;
        /* 用inBig5NameTRec決定要先存幾筆Record到Big5NameT.bak，ex:inBig5NameTRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inBig5NameTRec; i ++)
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
                                        inFILE_Delete((unsigned char *)_Big5NameT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_Big5NameT_FILE_NAME_BAK_);

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

        /* 存組好的該Big5NameTRecord 到 Big5NameT.bak */
        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_Big5NameT_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_Big5NameT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原Big5NameT.dat Record 到 Big5NameT.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
        /* inRecIndex要指到剩下還沒存進bak檔的第一個記憶體位置 */
        inBig5NameTRec = inBig5NameTRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < lnBig5NameTLength; i++)
        {
                if (uszRead_Total_Buff[i] == 0x0A)
                {
                        j++;

                        if (j == inBig5NameTRec)
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
                /* 接續存原Big5NameT.dat的Record */
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
                                inFILE_Delete((unsigned char *)_Big5NameT_FILE_NAME_BAK_);

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
                                inFILE_Delete((unsigned char *)_Big5NameT_FILE_NAME_BAK_);

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

        /* 刪除原Big5NameT.dat */
        if (inFILE_Delete((unsigned char *)_Big5NameT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* 將Big5NameT.bak改名字為Big5NameT.dat取代原檔案 */
        if (inFILE_Rename((unsigned char *)_Big5NameT_FILE_NAME_BAK_, (unsigned char *)_Big5NameT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* inSaveBig5NameTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveBig5NameTRec(%d) END!!", inBig5NameTRec - 1);
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
Function        :inGetMerchant_Name_Index
Date&Time       :2025/5/21 下午 4:50
Describe        :
*/
int inGetMerchant_Name_Index(char* szMerchant_Name_Index)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szMerchant_Name_Index == NULL || strlen(srBig5NameTRec.szMerchant_Name_Index) <= 0 || strlen(srBig5NameTRec.szMerchant_Name_Index) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetMerchant_Name_Index() ERROR !!");

			if (szMerchant_Name_Index == NULL)
                        {
                                inLogPrintf(AT, "szMerchant_Name_Index == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMerchant_Name_Index length = (%d)", strlen(srBig5NameTRec.szMerchant_Name_Index));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMerchant_Name_Index[0], &srBig5NameTRec.szMerchant_Name_Index[0], strlen(srBig5NameTRec.szMerchant_Name_Index));

        return (VS_SUCCESS);
}

/*
Function        :inSetMerchant_Name_Index
Date&Time       :2025/5/21 下午 4:50
Describe        :
*/
int inSetMerchant_Name_Index(char* szMerchant_Name_Index)
{
        memset(srBig5NameTRec.szMerchant_Name_Index, 0x00, sizeof(srBig5NameTRec.szMerchant_Name_Index));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szMerchant_Name_Index == NULL || strlen(szMerchant_Name_Index) <= 0 || strlen(szMerchant_Name_Index) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetMerchant_Name_Index() ERROR !!");
                        if (szMerchant_Name_Index == NULL)
                        {
                                inLogPrintf(AT, "szMerchant_Name_Index == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMerchant_Name_Index length = (%d)", strlen(szMerchant_Name_Index));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srBig5NameTRec.szMerchant_Name_Index[0], &szMerchant_Name_Index[0], strlen(szMerchant_Name_Index));

        return (VS_SUCCESS);
}

/*
Function        :inGetMerchant_Name
Date&Time       :2025/5/21 下午 4:49
Describe        :
*/
int inGetMerchant_Name(char* szMerchant_Name)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szMerchant_Name == NULL || strlen(srBig5NameTRec.szMerchant_Name) <= 0 || strlen(srBig5NameTRec.szMerchant_Name) > 48)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetMerchant_Name() ERROR !!");

                        if (szMerchant_Name == NULL)
                        {
                                inLogPrintf(AT, "szMerchant_Name == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMerchant_Name length = (%d)", strlen(srBig5NameTRec.szMerchant_Name));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMerchant_Name[0], &srBig5NameTRec.szMerchant_Name[0], strlen(srBig5NameTRec.szMerchant_Name));

        return (VS_SUCCESS);
}

/*
Function        :inSetMerchant_Name
Date&Time       :2025/5/21 下午 4:49
Describe        :
*/
int inSetMerchant_Name(char* szMerchant_Name)
{
        memset(srBig5NameTRec.szMerchant_Name, 0x00, sizeof(srBig5NameTRec.szMerchant_Name));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szMerchant_Name == NULL || strlen(szMerchant_Name) <= 0 || strlen(szMerchant_Name) > 48)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetMerchant_Name() ERROR !!");

                        if (szMerchant_Name == NULL)
                        {
                                inLogPrintf(AT, "szMerchant_Name == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMerchant_Name length = (%d)", strlen(szMerchant_Name));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srBig5NameTRec.szMerchant_Name[0], &szMerchant_Name[0], strlen(szMerchant_Name));

        return (VS_SUCCESS);
}