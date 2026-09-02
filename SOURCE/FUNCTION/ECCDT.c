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
#include "ECCDT.h"

static  ECCDT_REC	srECCDTRec;	/* construct ECCDT record */
extern  int		ginDebug;       /* Debug使用 extern */

/*
Function        :inLoadECCDTRec
Date&Time       :2018/3/20 下午 5:45
Describe        :讀ECCDT檔案，inECCDTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadECCDTRec(int inECCDTRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆ECCDT檔案 */
        char            szECCDTRec[_SIZE_ECCDT_REC_ + 1];	/* 暫存, 放各個欄位檔案 */
        char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnECCDTLength = 0;			/* ECCDT總長度 */
        long            lnReadLength;                           /* 記錄每次要從ECCDT.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;	                /* inRec記錄讀到第幾筆, i為目前從ECCDT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */

        /* inLoadECCDTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadECCDTRec(%d) START!!", inECCDTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 判斷傳進來的inECCDTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inECCDTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inECCDTRec < 0:(index = %d) ERROR!!", inECCDTRec);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open ECCDT.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_ECCDT_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnECCDTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_ECCDT_FILE_NAME_);

        if (lnECCDTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnECCDTLength + 1);
        uszTemp = malloc(lnECCDTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnECCDTLength + 1);
        memset(uszTemp, 0x00, lnECCDTLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnECCDTLength;

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
         *i為目前從ECCDT讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnECCDTLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到ECCDT的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnECCDTLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        /* 因為inECCDT_Rec的index從0開始，所以inECCDT_Rec要+1 */
                        if (inRec == (inECCDTRec + 1))
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
         * 如果沒有inECCDTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inECCDTRec + 1) || inSearchResult == -1)
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
        memset(&srECCDTRec, 0x00, sizeof(srECCDTRec));
        /*
         * 以下pattern為存入ECCDT_Rec
         * i為ECCDT的第幾個字元
         * 存入ECCDT_Rec
         */
        i = 0;


        /* 01_一卡通SAM卡裝設卡槽 */
        /* 初始化 */
        memset(szECCDTRec, 0x00, sizeof(szECCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szECCDTRec[k ++] = uszReadData[i ++];
                if (szECCDTRec[k - 1] == 0x2C	||
		    szECCDTRec[k - 1] == 0x0D	||
		    szECCDTRec[k - 1] == 0x0A	||
		    szECCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnECCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "ECCDT unpack ERROR.");
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
        if (szECCDTRec[0] != 0x2C	&&
	    szECCDTRec[0] != 0x0D	&&
	    szECCDTRec[0] != 0x0A	&&
	    szECCDTRec[0] != 0x00)
        {
                memcpy(&srECCDTRec.szECC_SAM_Slot[0], &szECCDTRec[0], k - 1);
        }

        /* 02_交易功能參數 */
        /* 初始化 */
        memset(szECCDTRec, 0x00, sizeof(szECCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szECCDTRec[k ++] = uszReadData[i ++];
                if (szECCDTRec[k - 1] == 0x2C	||
		    szECCDTRec[k - 1] == 0x0D	||
		    szECCDTRec[k - 1] == 0x0A	||
		    szECCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnECCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "ECCDT unpack ERROR");
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
        if (szECCDTRec[0] != 0x2C	&&
	    szECCDTRec[0] != 0x0D	&&
	    szECCDTRec[0] != 0x0A	&&
	    szECCDTRec[0] != 0x00)
        {
                memcpy(&srECCDTRec.szECC_Transaction_Function[0], &szECCDTRec[0], k - 1);
        }

        /* 03_業者代碼(Service Provider) */
        /* 初始化 */
        memset(szECCDTRec, 0x00, sizeof(szECCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szECCDTRec[k ++] = uszReadData[i ++];
                if (szECCDTRec[k - 1] == 0x2C	||
		    szECCDTRec[k - 1] == 0x0D	||
		    szECCDTRec[k - 1] == 0x0A	||
		    szECCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnECCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "ECCDT unpack ERROR");
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
        if (szECCDTRec[0] != 0x2C	&&
	    szECCDTRec[0] != 0x0D	&&
	    szECCDTRec[0] != 0x0A	&&
	    szECCDTRec[0] != 0x00)
        {
                memcpy(&srECCDTRec.szECC_New_SP_ID[0], &szECCDTRec[0], k - 1);
        }

        
        /* 04_收銀機編號 */
        /* 初始化 */
        memset(szECCDTRec, 0x00, sizeof(szECCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szECCDTRec[k ++] = uszReadData[i ++];
                if (szECCDTRec[k - 1] == 0x2C	||
		    szECCDTRec[k - 1] == 0x0D	||
		    szECCDTRec[k - 1] == 0x0A	||
		    szECCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnECCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "ECCDT unpack ERROR");
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
        if (szECCDTRec[0] != 0x2C	&&
	    szECCDTRec[0] != 0x0D	&&
	    szECCDTRec[0] != 0x0A	&&
	    szECCDTRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srECCDTRec.szECC_POS_ID[0], &szECCDTRec[0], k - 1);
        }

        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadECCDTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadECCDTRec(%d) END!!", inECCDTRec);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveECCDTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSaveECCDTRec(int inECCDTRec)
{
        unsigned long   uldat_Handle;                           /* FILE Handle */
        unsigned long   ulbak_Handle;                           /* FILE Handle */
        int	        inPackCount = 0;                        /* uszWriteBuff_Record的index */
        int	        inRecIndex = 0;                         /* uszRead_Total_Buff的index */
        int             inECCDT_Total_Rec = 0;		/* ECCDT.dat的總筆數 */
        int             inRetVal;
        int             i = 0, j = 0;
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long	        lnReadLength = 0;                       /* 每次要從ECCDT.dat讀多少byte出來 */
        long            lnECCDTLength = 0;			/* ECCDT.dat檔案總長度 */
        unsigned char	*uszRead_Total_Buff;
        unsigned char   *uszWriteBuff_Record, *uszWriteBuff_Org;


        /* inSaveECCDTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveECCDTRec(%d)_START!!", inECCDTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 防呆先刪除ECCDT.bak */
        inFILE_Delete((unsigned char *)_ECCDT_FILE_NAME_BAK_);

        /* 新建ECCDT.bak檔案 */
        inRetVal = inFILE_Create(&ulbak_Handle, (unsigned char *)_ECCDT_FILE_NAME_BAK_);

        if (inRetVal != VS_SUCCESS)
        {
                inFILE_Close(&ulbak_Handle);

                return (VS_ERROR);
        }

        /* 開啟原檔案ECCDT.dat */
        inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_ECCDT_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                /* ECCDT.dat開檔失敗 ，不用關檔ECCDT.dat */
                /* ECCDT.bak仍要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_ECCDT_FILE_NAME_BAK_);

                return(VS_ERROR);
        }

        /* 取得ECCDT.dat檔案大小 */
        lnECCDTLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *)_ECCDT_FILE_NAME_);

        /* 取得檔案大小失敗 */
        if (lnECCDTLength == VS_ERROR)
        {
                /* ECCDT.bak和ECCDT.dat要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Close(&uldat_Handle);
                inFILE_Delete((unsigned char *)_ECCDT_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        uszWriteBuff_Record = malloc(_SIZE_ECCDT_REC_ + _SIZE_ECCDT_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_ECCDT_REC_ + _SIZE_ECCDT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原ECCDT.dat的每筆Record 初始化 */
        uszWriteBuff_Org = malloc(_SIZE_ECCDT_REC_ + _SIZE_ECCDT_COMMA_0D0A_);
        memset(uszWriteBuff_Org, 0x00, _SIZE_ECCDT_REC_ + _SIZE_ECCDT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存ECCDT.dat全部資料 */
        uszRead_Total_Buff = malloc(lnECCDTLength + 1);
        memset(uszRead_Total_Buff, 0x00, lnECCDTLength + 1);

        inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

        /* szECC_SAM_Slot */
        memcpy(&uszWriteBuff_Record[0], &srECCDTRec.szECC_SAM_Slot[0], strlen(srECCDTRec.szECC_SAM_Slot));
        inPackCount += strlen(srECCDTRec.szECC_SAM_Slot);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* szECC_Transaction_Function */
        memcpy(&uszWriteBuff_Record[inPackCount], &srECCDTRec.szECC_Transaction_Function[0], strlen(srECCDTRec.szECC_Transaction_Function));
        inPackCount += strlen(srECCDTRec.szECC_Transaction_Function);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* szECC_New_SP_ID */
        memcpy(&uszWriteBuff_Record[inPackCount], &srECCDTRec.szECC_New_SP_ID[0], strlen(srECCDTRec.szECC_New_SP_ID));
        inPackCount += strlen(srECCDTRec.szECC_New_SP_ID);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* szECC_POS_ID */
        memcpy(&uszWriteBuff_Record[inPackCount], &srECCDTRec.szECC_POS_ID[0], strlen(srECCDTRec.szECC_POS_ID));
        inPackCount += strlen(srECCDTRec.szECC_POS_ID);

        /* 最後的data不用逗號 */
        /* 補上換行符號 */

        /* 0D是移至行首 */
        uszWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        uszWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;

        /* 讀ECCDT.dat檔案存到uszRead_Total_Buff */
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnECCDTLength;

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
                                        inFILE_Delete((unsigned char *)_ECCDT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_ECCDT_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_ECCDT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        /* 防呆 總record數量小於要存取inECCDTRec Return ERROR */
        /* 算總Record數 */
        for (i = 0; i < (lnECCDTLength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (uszRead_Total_Buff[i] == 0x0A)
                        inECCDT_Total_Rec++;
        }

        /* 防呆 總record數量小於要存取inECCDTRec Return ERROR */
        if ((inECCDTRec + 1) > inECCDT_Total_Rec)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "No data or Index ERROR");
                }

                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_ECCDT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                return (VS_ERROR);
        }

        /* 記錄ReadTotalBuffer的陣列位置 */
        inRecIndex = 0;
        /* 用inECCDTRec決定要先存幾筆Record到ECCDT.bak，ex:inECCDTRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inECCDTRec; i ++)
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
                                        inFILE_Delete((unsigned char *)_ECCDT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_ECCDT_FILE_NAME_BAK_);

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

        /* 存組好的該ECCDTRecord 到 ECCDT.bak */
        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_ECCDT_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_ECCDT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原ECCDT.dat Record 到 ECCDT.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
        /* inRecIndex要指到剩下還沒存進bak檔的第一個記憶體位置 */
        inECCDTRec = inECCDTRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < lnECCDTLength; i++)
        {
                if (uszRead_Total_Buff[i] == 0x0A)
                {
                        j++;

                        if (j == inECCDTRec)
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
                /* 接續存原ECCDT.dat的Record */
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
                                inFILE_Delete((unsigned char *)_ECCDT_FILE_NAME_BAK_);

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
                                inFILE_Delete((unsigned char *)_ECCDT_FILE_NAME_BAK_);

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

        /* 刪除原ECCDT.dat */
        if (inFILE_Delete((unsigned char *)_ECCDT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* 將ECCDT.bak改名字為ECCDT.dat取代原檔案 */
        if (inFILE_Rename((unsigned char *)_ECCDT_FILE_NAME_BAK_, (unsigned char *)_ECCDT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* inSaveECCDTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveECCDTRec(%d) END!!", inECCDTRec - 1);
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
Function        :inGetECC_SAM_Slot
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inGetECC_SAM_Slot(char* szECC_SAM_Slot)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szECC_SAM_Slot == NULL || strlen(srECCDTRec.szECC_SAM_Slot) <= 0 || strlen(srECCDTRec.szECC_SAM_Slot) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetECC_SAM_Slot() ERROR !!");

			if (szECC_SAM_Slot == NULL)
                        {
                                inLogPrintf(AT, "szECC_SAM_Slot == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szECC_SAM_Slot length = (%d)", (int)strlen(srECCDTRec.szECC_SAM_Slot));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szECC_SAM_Slot[0], &srECCDTRec.szECC_SAM_Slot[0], strlen(srECCDTRec.szECC_SAM_Slot));

        return (VS_SUCCESS);
}

/*
Function        :inSetECC_SAM_Slot
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inSetECC_SAM_Slot(char* szECC_SAM_Slot)
{
        memset(srECCDTRec.szECC_SAM_Slot, 0x00, sizeof(srECCDTRec.szECC_SAM_Slot));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szECC_SAM_Slot == NULL || strlen(szECC_SAM_Slot) <= 0 || strlen(szECC_SAM_Slot) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetECC_SAM_Slot() ERROR !!");
                        if (szECC_SAM_Slot == NULL)
                        {
                                inLogPrintf(AT, "szECC_SAM_Slot == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szECC_SAM_Slot length = (%d)", (int)strlen(szECC_SAM_Slot));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srECCDTRec.szECC_SAM_Slot[0], &szECC_SAM_Slot[0], strlen(szECC_SAM_Slot));

        return (VS_SUCCESS);
}

/*
Function        :inGetECC_Transaction_Function
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inGetECC_Transaction_Function(char* szECC_Transaction_Function)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szECC_Transaction_Function == NULL || strlen(srECCDTRec.szECC_Transaction_Function) <= 0 || strlen(srECCDTRec.szECC_Transaction_Function) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTransaction_Function() ERROR !!");

			if (szECC_Transaction_Function == NULL)
                        {
                                inLogPrintf(AT, "szECC_Transaction_Function == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szECC_Transaction_Function length = (%d)", (int)strlen(srECCDTRec.szECC_Transaction_Function));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szECC_Transaction_Function[0], &srECCDTRec.szECC_Transaction_Function[0], strlen(srECCDTRec.szECC_Transaction_Function));

        return (VS_SUCCESS);
}

/*
Function        :inSetECC_Transaction_Function
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inSetECC_Transaction_Function(char* szECC_Transaction_Function)
{
        memset(srECCDTRec.szECC_Transaction_Function, 0x00, sizeof(srECCDTRec.szECC_Transaction_Function));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szECC_Transaction_Function == NULL || strlen(szECC_Transaction_Function) <= 0 || strlen(szECC_Transaction_Function) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTransaction_Function() ERROR !!");
                        if (szECC_Transaction_Function == NULL)
                        {
                                inLogPrintf(AT, "szECC_Transaction_Function == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szECC_Transaction_Function length = (%d)", (int)strlen(szECC_Transaction_Function));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srECCDTRec.szECC_Transaction_Function[0], &szECC_Transaction_Function[0], strlen(szECC_Transaction_Function));

        return (VS_SUCCESS);
}

/*
Function        :inGetECC_New_SP_ID
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inGetECC_New_SP_ID(char* szECC_New_SP_ID)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szECC_New_SP_ID == NULL || strlen(srECCDTRec.szECC_New_SP_ID) <= 0 || strlen(srECCDTRec.szECC_New_SP_ID) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetSP_ID() ERROR !!");

			if (szECC_New_SP_ID == NULL)
                        {
                                inLogPrintf(AT, "szECC_New_SP_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szECC_New_SP_ID length = (%d)", (int)strlen(srECCDTRec.szECC_New_SP_ID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szECC_New_SP_ID[0], &srECCDTRec.szECC_New_SP_ID[0], strlen(srECCDTRec.szECC_New_SP_ID));

        return (VS_SUCCESS);
}

/*
Function        :inSetECC_New_SP_ID
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inSetECC_New_SP_ID(char* szECC_New_SP_ID)
{
        memset(srECCDTRec.szECC_New_SP_ID, 0x00, sizeof(srECCDTRec.szECC_New_SP_ID));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szECC_New_SP_ID == NULL || strlen(szECC_New_SP_ID) <= 0 || strlen(szECC_New_SP_ID) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetSP_ID() ERROR !!");
                        if (szECC_New_SP_ID == NULL)
                        {
                                inLogPrintf(AT, "szECC_New_SP_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szECC_New_SP_ID length = (%d)", (int)strlen(szECC_New_SP_ID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srECCDTRec.szECC_New_SP_ID[0], &szECC_New_SP_ID[0], strlen(szECC_New_SP_ID));

        return (VS_SUCCESS);
}

/*
Function        :inGetECC_Shop_ID
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inGetECC_POS_ID(char* szECC_POS_ID)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szECC_POS_ID == NULL || strlen(srECCDTRec.szECC_POS_ID) <= 0 || strlen(srECCDTRec.szECC_POS_ID) > 30)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPOS_ID() ERROR !!");

			if (szECC_POS_ID == NULL)
                        {
                                inLogPrintf(AT, "szECC_POS_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szECC_POS_ID length = (%d)", (int)strlen(srECCDTRec.szECC_POS_ID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szECC_POS_ID[0], &srECCDTRec.szECC_POS_ID[0], strlen(srECCDTRec.szECC_POS_ID));

        return (VS_SUCCESS);
}

/*
Function        :inSetECC_POS_ID
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inSetECC_POS_ID(char* szECC_POS_ID)
{
        memset(srECCDTRec.szECC_POS_ID, 0x00, sizeof(srECCDTRec.szECC_POS_ID));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szECC_POS_ID == NULL || strlen(szECC_POS_ID) <= 0 || strlen(szECC_POS_ID) > 30)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPOS_ID() ERROR !!");
                        if (szECC_POS_ID == NULL)
                        {
                                inLogPrintf(AT, "szECC_POS_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szECC_POS_ID length = (%d)", (int)strlen(szECC_POS_ID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srECCDTRec.szECC_POS_ID[0], &szECC_POS_ID[0], strlen(szECC_POS_ID));

        return (VS_SUCCESS);
}

/*
Function        :inECCDT_Edit_ECCDT_Table
Date&Time       :2017/5/15 下午 4:08
Describe        :
*/
int inECCDT_Edit_ECCDT_Table(void)
{
	TABLE_GET_SET_TABLE ECCDT_FUNC_TABLE[] =
	{
		{"szECC_SAM_Slot"		,inGetECC_SAM_Slot			,inSetECC_SAM_Slot		},
		{"szECC_Transaction_Function"	,inGetECC_Transaction_Function		,inSetECC_Transaction_Function	},
		{"szECC_New_SP_ID"			,inGetECC_New_SP_ID			,inSetECC_New_SP_ID		},
		{"szECC_POS_ID"			,inGetECC_POS_ID			,inSetECC_POS_ID		},
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
	inDISP_ChineseFont_Color("是否更改ECCDT", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
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
	inLoadECCDTRec(inRecordCnt);
	
	inFunc_Edit_Table_Tag(ECCDT_FUNC_TABLE);
	inSaveECCDTRec(inRecordCnt);
	
	return	(VS_SUCCESS);
}
