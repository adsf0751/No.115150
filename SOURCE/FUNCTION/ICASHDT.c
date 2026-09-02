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
#include "ICASHDT.h"

static  ICASHDT_REC	srICASHDTRec;	/* construct ICASHDT record */
extern  int		ginDebug;       /* Debug使用 extern */

/*
Function        :inLoadICASHDTRec
Date&Time       :2017/12/18 上午 9:54
Describe        :讀ICASHDT檔案，inICASHDTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadICASHDTRec(int inICASHDTRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆ICASHDT檔案 */
        char            szICASHDTRec[_SIZE_ICASHDT_REC_ + 1];	/* 暫存, 放各個欄位檔案 */
        char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnICASHDTLength = 0;			/* ICASHDT總長度 */
        long            lnReadLength;                           /* 記錄每次要從ICASHDT.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;	                /* inRec記錄讀到第幾筆, i為目前從ICASHDT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */

        /* inLoadICASHDTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadICASHDTRec(%d) START!!", inICASHDTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 判斷傳進來的inICASHDTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inICASHDTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inICASHDTRec < 0:(index = %d) ERROR!!", inICASHDTRec);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open ICASHDT.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_ICASHDT_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnICASHDTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_ICASHDT_FILE_NAME_);

        if (lnICASHDTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnICASHDTLength + 1);
        uszTemp = malloc(lnICASHDTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnICASHDTLength + 1);
        memset(uszTemp, 0x00, lnICASHDTLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnICASHDTLength;

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
         *i為目前從ICASHDT讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnICASHDTLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到ICASHDT的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnICASHDTLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        /* 因為inICASHDT_Rec的index從0開始，所以inICASHDT_Rec要+1 */
                        if (inRec == (inICASHDTRec + 1))
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
         * 如果沒有inICASHDTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inICASHDTRec + 1) || inSearchResult == -1)
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
        memset(&srICASHDTRec, 0x00, sizeof(srICASHDTRec));
        /*
         * 以下pattern為存入ICASHDT_Rec
         * i為ICASHDT的第幾個字元
         * 存入ICASHDT_Rec
         */
        i = 0;


        /* 01_愛金卡SAM卡裝設卡槽 */
        /* 初始化 */
        memset(szICASHDTRec, 0x00, sizeof(szICASHDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szICASHDTRec[k ++] = uszReadData[i ++];
                if (szICASHDTRec[k - 1] == 0x2C	||
		    szICASHDTRec[k - 1] == 0x0D	||
		    szICASHDTRec[k - 1] == 0x0A	||
		    szICASHDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnICASHDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "ICASHDT unpack ERROR.");
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
        if (szICASHDTRec[0] != 0x2C	&&
	    szICASHDTRec[0] != 0x0D	&&
	    szICASHDTRec[0] != 0x0A	&&
	    szICASHDTRec[0] != 0x00)
        {
                memcpy(&srICASHDTRec.szICASH_SAM_Slot[0], &szICASHDTRec[0], k - 1);
        }

        /* 02_交易功能參數 */
        /* 初始化 */
        memset(szICASHDTRec, 0x00, sizeof(szICASHDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szICASHDTRec[k ++] = uszReadData[i ++];
                if (szICASHDTRec[k - 1] == 0x2C	||
		    szICASHDTRec[k - 1] == 0x0D	||
		    szICASHDTRec[k - 1] == 0x0A	||
		    szICASHDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnICASHDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "ICASHDT unpack ERROR");
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
        if (szICASHDTRec[0] != 0x2C	&&
	    szICASHDTRec[0] != 0x0D	&&
	    szICASHDTRec[0] != 0x0A	&&
	    szICASHDTRec[0] != 0x00)
        {
                memcpy(&srICASHDTRec.szICASH_Transaction_Function[0], &szICASHDTRec[0], k - 1);
        }

        /* 03_特店簡碼 NCCC收單的特約機構代碼 */
        /* 初始化 */
        memset(szICASHDTRec, 0x00, sizeof(szICASHDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szICASHDTRec[k ++] = uszReadData[i ++];
                if (szICASHDTRec[k - 1] == 0x2C	||
		    szICASHDTRec[k - 1] == 0x0D	||
		    szICASHDTRec[k - 1] == 0x0A	||
		    szICASHDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnICASHDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "ICASHDT unpack ERROR");
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
        if (szICASHDTRec[0] != 0x2C	&&
	    szICASHDTRec[0] != 0x0D	&&
	    szICASHDTRec[0] != 0x0A	&&
	    szICASHDTRec[0] != 0x00)
        {
                memcpy(&srICASHDTRec.szICASH_NCCC_Agency_Code[0], &szICASHDTRec[0], k - 1);
        }

        /* 04_門市代碼 門市對照碼 供愛金卡系統對應使用 */
        /* 初始化 */
        memset(szICASHDTRec, 0x00, sizeof(szICASHDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szICASHDTRec[k ++] = uszReadData[i ++];
                if (szICASHDTRec[k - 1] == 0x2C	||
		    szICASHDTRec[k - 1] == 0x0D	||
		    szICASHDTRec[k - 1] == 0x0A	||
		    szICASHDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnICASHDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "ICASHDT unpack ERROR");
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
        if (szICASHDTRec[0] != 0x2C	&&
	    szICASHDTRec[0] != 0x0D	&&
	    szICASHDTRec[0] != 0x0A	&&
	    szICASHDTRec[0] != 0x00)
        {
                memcpy(&srICASHDTRec.szICASH_Shop_ID[0], &szICASHDTRec[0], k - 1);
        }

        /* 05_愛金卡端末機編號 */
        /* 初始化 */
        memset(szICASHDTRec, 0x00, sizeof(szICASHDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szICASHDTRec[k ++] = uszReadData[i ++];
                if (szICASHDTRec[k - 1] == 0x2C	||
		    szICASHDTRec[k - 1] == 0x0D	||
		    szICASHDTRec[k - 1] == 0x0A	||
		    szICASHDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnICASHDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "ICASHDT unpack ERROR");
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
        if (szICASHDTRec[0] != 0x2C	&&
	    szICASHDTRec[0] != 0x0D	&&
	    szICASHDTRec[0] != 0x0A	&&
	    szICASHDTRec[0] != 0x00)
        {
                memcpy(&srICASHDTRec.szICASH_RegID[0], &szICASHDTRec[0], k - 1);
        }

        /* 06_特約機構編號 供愛金卡系統對應使用 */
        /* 初始化 */
        memset(szICASHDTRec, 0x00, sizeof(szICASHDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szICASHDTRec[k ++] = uszReadData[i ++];
                if (szICASHDTRec[k - 1] == 0x2C	||
		    szICASHDTRec[k - 1] == 0x0D	||
		    szICASHDTRec[k - 1] == 0x0A	||
		    szICASHDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnICASHDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "ICASHDT unpack ERROR");
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
        if (szICASHDTRec[0] != 0x2C	&&
	    szICASHDTRec[0] != 0x0D	&&
	    szICASHDTRec[0] != 0x0A	&&
	    szICASHDTRec[0] != 0x00)
        {
                memcpy(&srICASHDTRec.szICASH_Special_Agency_Code[0], &szICASHDTRec[0], k - 1);
        }

        /* 07_收單行代碼 NCCC:0956 */
        /* 初始化 */
        memset(szICASHDTRec, 0x00, sizeof(szICASHDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szICASHDTRec[k ++] = uszReadData[i ++];
                if (szICASHDTRec[k - 1] == 0x2C	||
		    szICASHDTRec[k - 1] == 0x0D	||
		    szICASHDTRec[k - 1] == 0x0A	||
		    szICASHDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnICASHDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "ICASHDT unpack ERROR");
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
        if (szICASHDTRec[0] != 0x2C	&&
	    szICASHDTRec[0] != 0x0D	&&
	    szICASHDTRec[0] != 0x0A	&&
	    szICASHDTRec[0] != 0x00)
        {
                memcpy(&srICASHDTRec.szICASH_Bank_ID[0], &szICASHDTRec[0], k - 1);
        }

        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadICASHDTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadICASHDTRec(%d) END!!", inICASHDTRec);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveICASHDTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSaveICASHDTRec(int inICASHDTRec)
{
        unsigned long   uldat_Handle;                           /* FILE Handle */
        unsigned long   ulbak_Handle;                           /* FILE Handle */
        int	        inPackCount = 0;                        /* uszWriteBuff_Record的index */
        int	        inRecIndex = 0;                         /* uszRead_Total_Buff的index */
        int             inICASHDT_Total_Rec = 0;		/* ICASHDT.dat的總筆數 */
        int             inRetVal;
        int             i = 0, j = 0;
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long	        lnReadLength = 0;                       /* 每次要從ICASHDT.dat讀多少byte出來 */
        long            lnICASHDTLength = 0;			/* ICASHDT.dat檔案總長度 */
        unsigned char	*uszRead_Total_Buff;
        unsigned char   *uszWriteBuff_Record, *uszWriteBuff_Org;


        /* inSaveICASHDTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveICASHDTRec(%d)_START!!", inICASHDTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 防呆先刪除ICASHDT.bak */
        inFILE_Delete((unsigned char *)_ICASHDT_FILE_NAME_BAK_);

        /* 新建ICASHDT.bak檔案 */
        inRetVal = inFILE_Create(&ulbak_Handle, (unsigned char *)_ICASHDT_FILE_NAME_BAK_);

        if (inRetVal != VS_SUCCESS)
        {
                inFILE_Close(&ulbak_Handle);

                return (VS_ERROR);
        }

        /* 開啟原檔案ICASHDT.dat */
        inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_ICASHDT_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                /* ICASHDT.dat開檔失敗 ，不用關檔ICASHDT.dat */
                /* ICASHDT.bak仍要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_ICASHDT_FILE_NAME_BAK_);

                return(VS_ERROR);
        }

        /* 取得ICASHDT.dat檔案大小 */
        lnICASHDTLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *)_ICASHDT_FILE_NAME_);

        /* 取得檔案大小失敗 */
        if (lnICASHDTLength == VS_ERROR)
        {
                /* ICASHDT.bak和ICASHDT.dat要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Close(&uldat_Handle);
                inFILE_Delete((unsigned char *)_ICASHDT_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        uszWriteBuff_Record = malloc(_SIZE_ICASHDT_REC_ + _SIZE_ICASHDT_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_ICASHDT_REC_ + _SIZE_ICASHDT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原ICASHDT.dat的每筆Record 初始化 */
        uszWriteBuff_Org = malloc(_SIZE_ICASHDT_REC_ + _SIZE_ICASHDT_COMMA_0D0A_);
        memset(uszWriteBuff_Org, 0x00, _SIZE_ICASHDT_REC_ + _SIZE_ICASHDT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存ICASHDT.dat全部資料 */
        uszRead_Total_Buff = malloc(lnICASHDTLength + 1);
        memset(uszRead_Total_Buff, 0x00, lnICASHDTLength + 1);

        inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

        /* szICASH_SAM_Slot */
        memcpy(&uszWriteBuff_Record[0], &srICASHDTRec.szICASH_SAM_Slot[0], strlen(srICASHDTRec.szICASH_SAM_Slot));
        inPackCount += strlen(srICASHDTRec.szICASH_SAM_Slot);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* szICASH_Transaction_Function */
        memcpy(&uszWriteBuff_Record[inPackCount], &srICASHDTRec.szICASH_Transaction_Function[0], strlen(srICASHDTRec.szICASH_Transaction_Function));
        inPackCount += strlen(srICASHDTRec.szICASH_Transaction_Function);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* szICASH_NCCC_Agency_Code */
        memcpy(&uszWriteBuff_Record[inPackCount], &srICASHDTRec.szICASH_NCCC_Agency_Code[0], strlen(srICASHDTRec.szICASH_NCCC_Agency_Code));
        inPackCount += strlen(srICASHDTRec.szICASH_NCCC_Agency_Code);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* szICASH_Shop_ID */
        memcpy(&uszWriteBuff_Record[inPackCount], &srICASHDTRec.szICASH_Shop_ID[0], strlen(srICASHDTRec.szICASH_Shop_ID));
        inPackCount += strlen(srICASHDTRec.szICASH_Shop_ID);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* szICASH_RegID */
        memcpy(&uszWriteBuff_Record[inPackCount], &srICASHDTRec.szICASH_RegID[0], strlen(srICASHDTRec.szICASH_RegID));
        inPackCount += strlen(srICASHDTRec.szICASH_RegID);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* szICASH_Special_Agency_Code */
        memcpy(&uszWriteBuff_Record[inPackCount], &srICASHDTRec.szICASH_Special_Agency_Code[0], strlen(srICASHDTRec.szICASH_Special_Agency_Code));
        inPackCount += strlen(srICASHDTRec.szICASH_Special_Agency_Code);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* szICASH_Bank_ID */
        memcpy(&uszWriteBuff_Record[inPackCount], &srICASHDTRec.szICASH_Bank_ID[0], strlen(srICASHDTRec.szICASH_Bank_ID));
        inPackCount += strlen(srICASHDTRec.szICASH_Bank_ID);

        /* 最後的data不用逗號 */
        /* 補上換行符號 */

        /* 0D是移至行首 */
        uszWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        uszWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;

        /* 讀ICASHDT.dat檔案存到uszRead_Total_Buff */
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnICASHDTLength;

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
                                        inFILE_Delete((unsigned char *)_ICASHDT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_ICASHDT_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_ICASHDT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        /* 防呆 總record數量小於要存取inICASHDTRec Return ERROR */
        /* 算總Record數 */
        for (i = 0; i < (lnICASHDTLength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (uszRead_Total_Buff[i] == 0x0A)
                        inICASHDT_Total_Rec++;
        }

        /* 防呆 總record數量小於要存取inICASHDTRec Return ERROR */
        if ((inICASHDTRec + 1) > inICASHDT_Total_Rec)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "No data or Index ERROR");
                }

                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_ICASHDT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                return (VS_ERROR);
        }

        /* 記錄ReadTotalBuffer的陣列位置 */
        inRecIndex = 0;
        /* 用inICASHDTRec決定要先存幾筆Record到ICASHDT.bak，ex:inICASHDTRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inICASHDTRec; i ++)
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
                                        inFILE_Delete((unsigned char *)_ICASHDT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_ICASHDT_FILE_NAME_BAK_);

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

        /* 存組好的該ICASHDTRecord 到 ICASHDT.bak */
        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_ICASHDT_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_ICASHDT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原ICASHDT.dat Record 到 ICASHDT.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
        /* inRecIndex要指到剩下還沒存進bak檔的第一個記憶體位置 */
        inICASHDTRec = inICASHDTRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < lnICASHDTLength; i++)
        {
                if (uszRead_Total_Buff[i] == 0x0A)
                {
                        j++;

                        if (j == inICASHDTRec)
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
                /* 接續存原ICASHDT.dat的Record */
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
                                inFILE_Delete((unsigned char *)_ICASHDT_FILE_NAME_BAK_);

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
                                inFILE_Delete((unsigned char *)_ICASHDT_FILE_NAME_BAK_);

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

        /* 刪除原ICASHDT.dat */
        if (inFILE_Delete((unsigned char *)_ICASHDT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* 將ICASHDT.bak改名字為ICASHDT.dat取代原檔案 */
        if (inFILE_Rename((unsigned char *)_ICASHDT_FILE_NAME_BAK_, (unsigned char *)_ICASHDT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* inSaveICASHDTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveICASHDTRec(%d) END!!", inICASHDTRec - 1);
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
Function        :inGetICASH_SAM_Slot
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inGetICASH_SAM_Slot(char* szICASH_SAM_Slot)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szICASH_SAM_Slot == NULL || strlen(srICASHDTRec.szICASH_SAM_Slot) <= 0 || strlen(srICASHDTRec.szICASH_SAM_Slot) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetICASH_SAM_Slot() ERROR !!");

			if (szICASH_SAM_Slot == NULL)
                        {
                                inLogPrintf(AT, "szICASH_SAM_Slot == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szICASH_SAM_Slot length = (%d)", (int)strlen(srICASHDTRec.szICASH_SAM_Slot));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szICASH_SAM_Slot[0], &srICASHDTRec.szICASH_SAM_Slot[0], strlen(srICASHDTRec.szICASH_SAM_Slot));

        return (VS_SUCCESS);
}

/*
Function        :inSetICASH_SAM_Slot
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inSetICASH_SAM_Slot(char* szICASH_SAM_Slot)
{
        memset(srICASHDTRec.szICASH_SAM_Slot, 0x00, sizeof(srICASHDTRec.szICASH_SAM_Slot));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szICASH_SAM_Slot == NULL || strlen(szICASH_SAM_Slot) <= 0 || strlen(szICASH_SAM_Slot) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetICASH_SAM_Slot() ERROR !!");
                        if (szICASH_SAM_Slot == NULL)
                        {
                                inLogPrintf(AT, "szICASH_SAM_Slot == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szICASH_SAM_Slot length = (%d)", (int)strlen(szICASH_SAM_Slot));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srICASHDTRec.szICASH_SAM_Slot[0], &szICASH_SAM_Slot[0], strlen(szICASH_SAM_Slot));

        return (VS_SUCCESS);
}

/*
Function        :inGetICASH_Transaction_Function
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inGetICASH_Transaction_Function(char* szICASH_Transaction_Function)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szICASH_Transaction_Function == NULL || strlen(srICASHDTRec.szICASH_Transaction_Function) <= 0 || strlen(srICASHDTRec.szICASH_Transaction_Function) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTransaction_Function() ERROR !!");

			if (szICASH_Transaction_Function == NULL)
                        {
                                inLogPrintf(AT, "szICASH_Transaction_Function == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szICASH_Transaction_Function length = (%d)", (int)strlen(srICASHDTRec.szICASH_Transaction_Function));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szICASH_Transaction_Function[0], &srICASHDTRec.szICASH_Transaction_Function[0], strlen(srICASHDTRec.szICASH_Transaction_Function));

        return (VS_SUCCESS);
}

/*
Function        :inSetICASH_Transaction_Function
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inSetICASH_Transaction_Function(char* szICASH_Transaction_Function)
{
        memset(srICASHDTRec.szICASH_Transaction_Function, 0x00, sizeof(srICASHDTRec.szICASH_Transaction_Function));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szICASH_Transaction_Function == NULL || strlen(szICASH_Transaction_Function) <= 0 || strlen(szICASH_Transaction_Function) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTransaction_Function() ERROR !!");
                        if (szICASH_Transaction_Function == NULL)
                        {
                                inLogPrintf(AT, "szICASH_Transaction_Function == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szICASH_Transaction_Function length = (%d)", (int)strlen(szICASH_Transaction_Function));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srICASHDTRec.szICASH_Transaction_Function[0], &szICASH_Transaction_Function[0], strlen(szICASH_Transaction_Function));

        return (VS_SUCCESS);
}

/*
Function        :inGetICASH_NCCC_Agency_Code
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inGetICASH_NCCC_Agency_Code(char* szICASH_NCCC_Agency_Code)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szICASH_NCCC_Agency_Code == NULL || strlen(srICASHDTRec.szICASH_NCCC_Agency_Code) <= 0 || strlen(srICASHDTRec.szICASH_NCCC_Agency_Code) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetNCCC_Agency_Code() ERROR !!");

			if (szICASH_NCCC_Agency_Code == NULL)
                        {
                                inLogPrintf(AT, "szICASH_NCCC_Agency_Code == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szICASH_NCCC_Agency_Code length = (%d)", (int)strlen(srICASHDTRec.szICASH_NCCC_Agency_Code));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szICASH_NCCC_Agency_Code[0], &srICASHDTRec.szICASH_NCCC_Agency_Code[0], strlen(srICASHDTRec.szICASH_NCCC_Agency_Code));

        return (VS_SUCCESS);
}

/*
Function        :inSetICASH_NCCC_Agency_Code
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inSetICASH_NCCC_Agency_Code(char* szICASH_NCCC_Agency_Code)
{
        memset(srICASHDTRec.szICASH_NCCC_Agency_Code, 0x00, sizeof(srICASHDTRec.szICASH_NCCC_Agency_Code));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szICASH_NCCC_Agency_Code == NULL || strlen(szICASH_NCCC_Agency_Code) <= 0 || strlen(szICASH_NCCC_Agency_Code) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetNCCC_Agency_Code() ERROR !!");
                        if (szICASH_NCCC_Agency_Code == NULL)
                        {
                                inLogPrintf(AT, "szICASH_NCCC_Agency_Code == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szICASH_NCCC_Agency_Code length = (%d)", (int)strlen(szICASH_NCCC_Agency_Code));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srICASHDTRec.szICASH_NCCC_Agency_Code[0], &szICASH_NCCC_Agency_Code[0], strlen(szICASH_NCCC_Agency_Code));

        return (VS_SUCCESS);
}

/*
Function        :inGetICASH_Shop_ID
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inGetICASH_Shop_ID(char* szICASH_Shop_ID)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szICASH_Shop_ID == NULL || strlen(srICASHDTRec.szICASH_Shop_ID) <= 0 || strlen(srICASHDTRec.szICASH_Shop_ID) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetShop_ID() ERROR !!");

			if (szICASH_Shop_ID == NULL)
                        {
                                inLogPrintf(AT, "szICASH_Shop_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szICASH_Shop_ID length = (%d)", (int)strlen(srICASHDTRec.szICASH_Shop_ID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szICASH_Shop_ID[0], &srICASHDTRec.szICASH_Shop_ID[0], strlen(srICASHDTRec.szICASH_Shop_ID));

        return (VS_SUCCESS);
}

/*
Function        :inSetICASH_Shop_ID
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inSetICASH_Shop_ID(char* szICASH_Shop_ID)
{
        memset(srICASHDTRec.szICASH_Shop_ID, 0x00, sizeof(srICASHDTRec.szICASH_Shop_ID));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szICASH_Shop_ID == NULL || strlen(szICASH_Shop_ID) <= 0 || strlen(szICASH_Shop_ID) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetShop_ID() ERROR !!");
                        if (szICASH_Shop_ID == NULL)
                        {
                                inLogPrintf(AT, "szICASH_Shop_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szICASH_Shop_ID length = (%d)", (int)strlen(szICASH_Shop_ID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srICASHDTRec.szICASH_Shop_ID[0], &szICASH_Shop_ID[0], strlen(szICASH_Shop_ID));

        return (VS_SUCCESS);
}

/*
Function        :inGetICASH_RegID
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inGetICASH_RegID(char* szICASH_RegID)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szICASH_RegID == NULL || strlen(srICASHDTRec.szICASH_RegID) <= 0 || strlen(srICASHDTRec.szICASH_RegID) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetRegID() ERROR !!");

			if (szICASH_RegID == NULL)
                        {
                                inLogPrintf(AT, "szICASH_RegID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szICASH_RegID length = (%d)", (int)strlen(srICASHDTRec.szICASH_RegID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szICASH_RegID[0], &srICASHDTRec.szICASH_RegID[0], strlen(srICASHDTRec.szICASH_RegID));

        return (VS_SUCCESS);
}

/*
Function        :inSetICASH_RegID
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inSetICASH_RegID(char* szICASH_RegID)
{
        memset(srICASHDTRec.szICASH_RegID, 0x00, sizeof(srICASHDTRec.szICASH_RegID));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szICASH_RegID == NULL || strlen(szICASH_RegID) <= 0 || strlen(szICASH_RegID) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetRegID() ERROR !!");
                        if (szICASH_RegID == NULL)
                        {
                                inLogPrintf(AT, "szICASH_RegID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szICASH_RegID length = (%d)", (int)strlen(szICASH_RegID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srICASHDTRec.szICASH_RegID[0], &szICASH_RegID[0], strlen(szICASH_RegID));

        return (VS_SUCCESS);
}

/*
Function        :inGetICASH_Special_Agency_Code
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inGetICASH_Special_Agency_Code(char* szICASH_Special_Agency_Code)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szICASH_Special_Agency_Code == NULL || strlen(srICASHDTRec.szICASH_Special_Agency_Code) <= 0 || strlen(srICASHDTRec.szICASH_Special_Agency_Code) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetSpecial_Agency_Code() ERROR !!");

			if (szICASH_Special_Agency_Code == NULL)
                        {
                                inLogPrintf(AT, "szICASH_Special_Agency_Code == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szICASH_Special_Agency_Code length = (%d)", (int)strlen(srICASHDTRec.szICASH_Special_Agency_Code));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szICASH_Special_Agency_Code[0], &srICASHDTRec.szICASH_Special_Agency_Code[0], strlen(srICASHDTRec.szICASH_Special_Agency_Code));

        return (VS_SUCCESS);
}

/*
Function        :inSetICASH_Special_Agency_Code
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inSetICASH_Special_Agency_Code(char* szICASH_Special_Agency_Code)
{
        memset(srICASHDTRec.szICASH_Special_Agency_Code, 0x00, sizeof(srICASHDTRec.szICASH_Special_Agency_Code));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szICASH_Special_Agency_Code == NULL || strlen(szICASH_Special_Agency_Code) <= 0 || strlen(szICASH_Special_Agency_Code) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetSpecial_Agency_Code() ERROR !!");
                        if (szICASH_Special_Agency_Code == NULL)
                        {
                                inLogPrintf(AT, "szICASH_Special_Agency_Code == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szICASH_Special_Agency_Code length = (%d)", (int)strlen(szICASH_Special_Agency_Code));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srICASHDTRec.szICASH_Special_Agency_Code[0], &szICASH_Special_Agency_Code[0], strlen(szICASH_Special_Agency_Code));

        return (VS_SUCCESS);
}

/*
Function        :inGetICASH_Bank_ID
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inGetICASH_Bank_ID(char* szICASH_Bank_ID)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szICASH_Bank_ID == NULL || strlen(srICASHDTRec.szICASH_Bank_ID) <= 0 || strlen(srICASHDTRec.szICASH_Bank_ID) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetBank_ID() ERROR !!");

			if (szICASH_Bank_ID == NULL)
                        {
                                inLogPrintf(AT, "szICASH_Bank_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szICASH_Bank_ID length = (%d)", (int)strlen(srICASHDTRec.szICASH_Bank_ID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szICASH_Bank_ID[0], &srICASHDTRec.szICASH_Bank_ID[0], strlen(srICASHDTRec.szICASH_Bank_ID));

        return (VS_SUCCESS);
}

/*
Function        :inSetICASH_Bank_ID
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inSetICASH_Bank_ID(char* szICASH_Bank_ID)
{
        memset(srICASHDTRec.szICASH_Bank_ID, 0x00, sizeof(srICASHDTRec.szICASH_Bank_ID));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szICASH_Bank_ID == NULL || strlen(szICASH_Bank_ID) <= 0 || strlen(szICASH_Bank_ID) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetBank_ID() ERROR !!");
                        if (szICASH_Bank_ID == NULL)
                        {
                                inLogPrintf(AT, "szICASH_Bank_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szICASH_Bank_ID length = (%d)", (int)strlen(szICASH_Bank_ID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srICASHDTRec.szICASH_Bank_ID[0], &szICASH_Bank_ID[0], strlen(szICASH_Bank_ID));

        return (VS_SUCCESS);
}

/*
Function        :inICASHDT_Edit_ICASHDT_Table
Date&Time       :2017/5/15 下午 4:08
Describe        :
*/
int inICASHDT_Edit_ICASHDT_Table(void)
{
	TABLE_GET_SET_TABLE ICASHDT_FUNC_TABLE[] =
	{
		{"szICASH_SAM_Slot"		,inGetICASH_SAM_Slot			,inSetICASH_SAM_Slot			},
		{"szICASH_Transaction_Function"	,inGetICASH_Transaction_Function	,inSetICASH_Transaction_Function	},
		{"szICASH_NCCC_Agency_Code"	,inGetICASH_NCCC_Agency_Code		,inSetICASH_NCCC_Agency_Code		},
		{"szICASH_Shop_ID"		,inGetICASH_Shop_ID			,inSetICASH_Shop_ID			},
		{"szICASH_RegID"		,inGetICASH_RegID			,inSetICASH_RegID			},
		{"szICASH_Special_Agency_Code"	,inGetICASH_Special_Agency_Code		,inSetICASH_Special_Agency_Code		},
		{"szICASH_Bank_ID"		,inGetICASH_Bank_ID			,inSetICASH_Bank_ID			},
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
	inDISP_ChineseFont_Color("是否更改ICASHDT", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
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
	inLoadICASHDTRec(inRecordCnt);
	
	inFunc_Edit_Table_Tag(ICASHDT_FUNC_TABLE);
	inSaveICASHDTRec(inRecordCnt);
	
	return	(VS_SUCCESS);
}
