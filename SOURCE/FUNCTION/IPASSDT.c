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
#include "IPASSDT.h"

static  IPASSDT_REC	srIPASSDTRec;	/* construct IPASSDT record */
extern  int		ginDebug;       /* Debug使用 extern */

/*
Function        :inLoadIPASSDTRec
Date&Time       :2017/12/18 上午 9:54
Describe        :讀IPASSDT檔案，inIPASSDTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadIPASSDTRec(int inIPASSDTRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆IPASSDT檔案 */
        char            szIPASSDTRec[_SIZE_IPASSDT_REC_ + 1];	/* 暫存, 放各個欄位檔案 */
        char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnIPASSDTLength = 0;			/* IPASSDT總長度 */
        long            lnReadLength;                           /* 記錄每次要從IPASSDT.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;	                /* inRec記錄讀到第幾筆, i為目前從IPASSDT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */

        /* inLoadIPASSDTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadIPASSDTRec(%d) START!!", inIPASSDTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 判斷傳進來的inIPASSDTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inIPASSDTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inIPASSDTRec < 0:(index = %d) ERROR!!", inIPASSDTRec);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open IPASSDT.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_IPASSDT_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnIPASSDTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_IPASSDT_FILE_NAME_);

        if (lnIPASSDTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnIPASSDTLength + 1);
        uszTemp = malloc(lnIPASSDTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnIPASSDTLength + 1);
        memset(uszTemp, 0x00, lnIPASSDTLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnIPASSDTLength;

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
         *i為目前從IPASSDT讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnIPASSDTLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到IPASSDT的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnIPASSDTLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        /* 因為inIPASSDT_Rec的index從0開始，所以inIPASSDT_Rec要+1 */
                        if (inRec == (inIPASSDTRec + 1))
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
         * 如果沒有inIPASSDTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inIPASSDTRec + 1) || inSearchResult == -1)
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
        memset(&srIPASSDTRec, 0x00, sizeof(srIPASSDTRec));
        /*
         * 以下pattern為存入IPASSDT_Rec
         * i為IPASSDT的第幾個字元
         * 存入IPASSDT_Rec
         */
        i = 0;


        /* 01_一卡通SAM卡裝設卡槽 */
        /* 初始化 */
        memset(szIPASSDTRec, 0x00, sizeof(szIPASSDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szIPASSDTRec[k ++] = uszReadData[i ++];
                if (szIPASSDTRec[k - 1] == 0x2C	||
		    szIPASSDTRec[k - 1] == 0x0D	||
		    szIPASSDTRec[k - 1] == 0x0A	||
		    szIPASSDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnIPASSDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "IPASSDT unpack ERROR.");
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
        if (szIPASSDTRec[0] != 0x2C	&&
	    szIPASSDTRec[0] != 0x0D	&&
	    szIPASSDTRec[0] != 0x0A	&&
	    szIPASSDTRec[0] != 0x00)
        {
                memcpy(&srIPASSDTRec.szIPASS_SAM_Slot[0], &szIPASSDTRec[0], k - 1);
        }

        /* 02_交易功能參數 */
        /* 初始化 */
        memset(szIPASSDTRec, 0x00, sizeof(szIPASSDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szIPASSDTRec[k ++] = uszReadData[i ++];
                if (szIPASSDTRec[k - 1] == 0x2C	||
		    szIPASSDTRec[k - 1] == 0x0D	||
		    szIPASSDTRec[k - 1] == 0x0A	||
		    szIPASSDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnIPASSDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "IPASSDT unpack ERROR");
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
        if (szIPASSDTRec[0] != 0x2C	&&
	    szIPASSDTRec[0] != 0x0D	&&
	    szIPASSDTRec[0] != 0x0A	&&
	    szIPASSDTRec[0] != 0x00)
        {
                memcpy(&srIPASSDTRec.szIPASS_Transaction_Function[0], &szIPASSDTRec[0], k - 1);
        }

        /* 03_AMS/TMS IP Address(電文加密用) */
        /* 初始化 */
        memset(szIPASSDTRec, 0x00, sizeof(szIPASSDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szIPASSDTRec[k ++] = uszReadData[i ++];
                if (szIPASSDTRec[k - 1] == 0x2C	||
		    szIPASSDTRec[k - 1] == 0x0D	||
		    szIPASSDTRec[k - 1] == 0x0A	||
		    szIPASSDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnIPASSDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "IPASSDT unpack ERROR");
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
        if (szIPASSDTRec[0] != 0x2C	&&
	    szIPASSDTRec[0] != 0x0D	&&
	    szIPASSDTRec[0] != 0x0A	&&
	    szIPASSDTRec[0] != 0x00)
        {
                memcpy(&srIPASSDTRec.szIPASS_AMS_TMS_IP_Address[0], &szIPASSDTRec[0], k - 1);
        }

        /* 04_BMS IP Address (電文加密用) */
        /* 初始化 */
        memset(szIPASSDTRec, 0x00, sizeof(szIPASSDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szIPASSDTRec[k ++] = uszReadData[i ++];
                if (szIPASSDTRec[k - 1] == 0x2C	||
		    szIPASSDTRec[k - 1] == 0x0D	||
		    szIPASSDTRec[k - 1] == 0x0A	||
		    szIPASSDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnIPASSDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "IPASSDT unpack ERROR");
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
        if (szIPASSDTRec[0] != 0x2C	&&
	    szIPASSDTRec[0] != 0x0D	&&
	    szIPASSDTRec[0] != 0x0A	&&
	    szIPASSDTRec[0] != 0x00)
        {
                memcpy(&srIPASSDTRec.szIPASS_BMS_IP_Address[0], &szIPASSDTRec[0], k - 1);
        }

        /* 05_系統編號 */
        /* 初始化 */
        memset(szIPASSDTRec, 0x00, sizeof(szIPASSDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szIPASSDTRec[k ++] = uszReadData[i ++];
                if (szIPASSDTRec[k - 1] == 0x2C	||
		    szIPASSDTRec[k - 1] == 0x0D	||
		    szIPASSDTRec[k - 1] == 0x0A	||
		    szIPASSDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnIPASSDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "IPASSDT unpack ERROR");
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
        if (szIPASSDTRec[0] != 0x2C	&&
	    szIPASSDTRec[0] != 0x0D	&&
	    szIPASSDTRec[0] != 0x0A	&&
	    szIPASSDTRec[0] != 0x00)
        {
                memcpy(&srIPASSDTRec.szIPASS_System_ID[0], &szIPASSDTRec[0], k - 1);
        }

        /* 06_業者代碼(Service Provider) */
        /* 初始化 */
        memset(szIPASSDTRec, 0x00, sizeof(szIPASSDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szIPASSDTRec[k ++] = uszReadData[i ++];
                if (szIPASSDTRec[k - 1] == 0x2C	||
		    szIPASSDTRec[k - 1] == 0x0D	||
		    szIPASSDTRec[k - 1] == 0x0A	||
		    szIPASSDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnIPASSDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "IPASSDT unpack ERROR");
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
        if (szIPASSDTRec[0] != 0x2C	&&
	    szIPASSDTRec[0] != 0x0D	&&
	    szIPASSDTRec[0] != 0x0A	&&
	    szIPASSDTRec[0] != 0x00)
        {
                memcpy(&srIPASSDTRec.szIPASS_SP_ID[0], &szIPASSDTRec[0], k - 1);
        }

        /* 07_子公司編號 */
        /* 初始化 */
        memset(szIPASSDTRec, 0x00, sizeof(szIPASSDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szIPASSDTRec[k ++] = uszReadData[i ++];
                if (szIPASSDTRec[k - 1] == 0x2C	||
		    szIPASSDTRec[k - 1] == 0x0D	||
		    szIPASSDTRec[k - 1] == 0x0A	||
		    szIPASSDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnIPASSDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "IPASSDT unpack ERROR");
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
        if (szIPASSDTRec[0] != 0x2C	&&
	    szIPASSDTRec[0] != 0x0D	&&
	    szIPASSDTRec[0] != 0x0A	&&
	    szIPASSDTRec[0] != 0x00)
        {
                memcpy(&srIPASSDTRec.szIPASS_Sub_Company_ID[0], &szIPASSDTRec[0], k - 1);
        }

        /* 08_店鋪編號 */
        /* 初始化 */
        memset(szIPASSDTRec, 0x00, sizeof(szIPASSDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szIPASSDTRec[k ++] = uszReadData[i ++];
                if (szIPASSDTRec[k - 1] == 0x2C	||
		    szIPASSDTRec[k - 1] == 0x0D	||
		    szIPASSDTRec[k - 1] == 0x0A	||
		    szIPASSDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnIPASSDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "IPASSDT unpack ERROR");
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
        if (szIPASSDTRec[0] != 0x2C	&&
	    szIPASSDTRec[0] != 0x0D	&&
	    szIPASSDTRec[0] != 0x0A	&&
	    szIPASSDTRec[0] != 0x00)
        {
                memcpy(&srIPASSDTRec.szIPASS_Shop_ID[0], &szIPASSDTRec[0], k - 1);
        }

        /* 09_收銀機編號，預設值"00" */
        /* 初始化 */
        memset(szIPASSDTRec, 0x00, sizeof(szIPASSDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szIPASSDTRec[k ++] = uszReadData[i ++];
                if (szIPASSDTRec[k - 1] == 0x2C	||
		    szIPASSDTRec[k - 1] == 0x0D	||
		    szIPASSDTRec[k - 1] == 0x0A	||
		    szIPASSDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnIPASSDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "IPASSDT unpack ERROR");
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
        if (szIPASSDTRec[0] != 0x2C	&&
	    szIPASSDTRec[0] != 0x0D	&&
	    szIPASSDTRec[0] != 0x0A	&&
	    szIPASSDTRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srIPASSDTRec.szIPASS_POS_ID[0], &szIPASSDTRec[0], k - 1);
        }

        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadIPASSDTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadIPASSDTRec(%d) END!!", inIPASSDTRec);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveIPASSDTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSaveIPASSDTRec(int inIPASSDTRec)
{
        unsigned long   uldat_Handle;                           /* FILE Handle */
        unsigned long   ulbak_Handle;                           /* FILE Handle */
        int	        inPackCount = 0;                        /* uszWriteBuff_Record的index */
        int	        inRecIndex = 0;                         /* uszRead_Total_Buff的index */
        int             inIPASSDT_Total_Rec = 0;		/* IPASSDT.dat的總筆數 */
        int             inRetVal;
        int             i = 0, j = 0;
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long	        lnReadLength = 0;                       /* 每次要從IPASSDT.dat讀多少byte出來 */
        long            lnIPASSDTLength = 0;			/* IPASSDT.dat檔案總長度 */
        unsigned char	*uszRead_Total_Buff;
        unsigned char   *uszWriteBuff_Record, *uszWriteBuff_Org;


        /* inSaveIPASSDTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveIPASSDTRec(%d)_START!!", inIPASSDTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 防呆先刪除IPASSDT.bak */
        inFILE_Delete((unsigned char *)_IPASSDT_FILE_NAME_BAK_);

        /* 新建IPASSDT.bak檔案 */
        inRetVal = inFILE_Create(&ulbak_Handle, (unsigned char *)_IPASSDT_FILE_NAME_BAK_);

        if (inRetVal != VS_SUCCESS)
        {
                inFILE_Close(&ulbak_Handle);

                return (VS_ERROR);
        }

        /* 開啟原檔案IPASSDT.dat */
        inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_IPASSDT_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                /* IPASSDT.dat開檔失敗 ，不用關檔IPASSDT.dat */
                /* IPASSDT.bak仍要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_IPASSDT_FILE_NAME_BAK_);

                return(VS_ERROR);
        }

        /* 取得IPASSDT.dat檔案大小 */
        lnIPASSDTLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *)_IPASSDT_FILE_NAME_);

        /* 取得檔案大小失敗 */
        if (lnIPASSDTLength == VS_ERROR)
        {
                /* IPASSDT.bak和IPASSDT.dat要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Close(&uldat_Handle);
                inFILE_Delete((unsigned char *)_IPASSDT_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        uszWriteBuff_Record = malloc(_SIZE_IPASSDT_REC_ + _SIZE_IPASSDT_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_IPASSDT_REC_ + _SIZE_IPASSDT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原IPASSDT.dat的每筆Record 初始化 */
        uszWriteBuff_Org = malloc(_SIZE_IPASSDT_REC_ + _SIZE_IPASSDT_COMMA_0D0A_);
        memset(uszWriteBuff_Org, 0x00, _SIZE_IPASSDT_REC_ + _SIZE_IPASSDT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存IPASSDT.dat全部資料 */
        uszRead_Total_Buff = malloc(lnIPASSDTLength + 1);
        memset(uszRead_Total_Buff, 0x00, lnIPASSDTLength + 1);

        inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

        /* szIPASS_SAM_Slot */
        memcpy(&uszWriteBuff_Record[0], &srIPASSDTRec.szIPASS_SAM_Slot[0], strlen(srIPASSDTRec.szIPASS_SAM_Slot));
        inPackCount += strlen(srIPASSDTRec.szIPASS_SAM_Slot);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* szIPASS_Transaction_Function */
        memcpy(&uszWriteBuff_Record[inPackCount], &srIPASSDTRec.szIPASS_Transaction_Function[0], strlen(srIPASSDTRec.szIPASS_Transaction_Function));
        inPackCount += strlen(srIPASSDTRec.szIPASS_Transaction_Function);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* szIPASS_AMS_TMS_IP_Address */
        memcpy(&uszWriteBuff_Record[inPackCount], &srIPASSDTRec.szIPASS_AMS_TMS_IP_Address[0], strlen(srIPASSDTRec.szIPASS_AMS_TMS_IP_Address));
        inPackCount += strlen(srIPASSDTRec.szIPASS_AMS_TMS_IP_Address);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* szIPASS_BMS_IP_Address */
        memcpy(&uszWriteBuff_Record[inPackCount], &srIPASSDTRec.szIPASS_BMS_IP_Address[0], strlen(srIPASSDTRec.szIPASS_BMS_IP_Address));
        inPackCount += strlen(srIPASSDTRec.szIPASS_BMS_IP_Address);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* szIPASS_System_ID */
        memcpy(&uszWriteBuff_Record[inPackCount], &srIPASSDTRec.szIPASS_System_ID[0], strlen(srIPASSDTRec.szIPASS_System_ID));
        inPackCount += strlen(srIPASSDTRec.szIPASS_System_ID);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* szIPASS_SP_ID */
        memcpy(&uszWriteBuff_Record[inPackCount], &srIPASSDTRec.szIPASS_SP_ID[0], strlen(srIPASSDTRec.szIPASS_SP_ID));
        inPackCount += strlen(srIPASSDTRec.szIPASS_SP_ID);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* szIPASS_Sub_Company_ID */
        memcpy(&uszWriteBuff_Record[inPackCount], &srIPASSDTRec.szIPASS_Sub_Company_ID[0], strlen(srIPASSDTRec.szIPASS_Sub_Company_ID));
        inPackCount += strlen(srIPASSDTRec.szIPASS_Sub_Company_ID);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* szIPASS_Shop_ID */
        memcpy(&uszWriteBuff_Record[inPackCount], &srIPASSDTRec.szIPASS_Shop_ID[0], strlen(srIPASSDTRec.szIPASS_Shop_ID));
        inPackCount += strlen(srIPASSDTRec.szIPASS_Shop_ID);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* szIPASS_POS_ID */
        memcpy(&uszWriteBuff_Record[inPackCount], &srIPASSDTRec.szIPASS_POS_ID[0], strlen(srIPASSDTRec.szIPASS_POS_ID));
        inPackCount += strlen(srIPASSDTRec.szIPASS_POS_ID);

        /* 最後的data不用逗號 */
        /* 補上換行符號 */

        /* 0D是移至行首 */
        uszWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        uszWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;

        /* 讀IPASSDT.dat檔案存到uszRead_Total_Buff */
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnIPASSDTLength;

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
                                        inFILE_Delete((unsigned char *)_IPASSDT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_IPASSDT_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_IPASSDT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        /* 防呆 總record數量小於要存取inIPASSDTRec Return ERROR */
        /* 算總Record數 */
        for (i = 0; i < (lnIPASSDTLength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (uszRead_Total_Buff[i] == 0x0A)
                        inIPASSDT_Total_Rec++;
        }

        /* 防呆 總record數量小於要存取inIPASSDTRec Return ERROR */
        if ((inIPASSDTRec + 1) > inIPASSDT_Total_Rec)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "No data or Index ERROR");
                }

                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_IPASSDT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                return (VS_ERROR);
        }

        /* 記錄ReadTotalBuffer的陣列位置 */
        inRecIndex = 0;
        /* 用inIPASSDTRec決定要先存幾筆Record到IPASSDT.bak，ex:inIPASSDTRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inIPASSDTRec; i ++)
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
                                        inFILE_Delete((unsigned char *)_IPASSDT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_IPASSDT_FILE_NAME_BAK_);

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

        /* 存組好的該IPASSDTRecord 到 IPASSDT.bak */
        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_IPASSDT_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_IPASSDT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原IPASSDT.dat Record 到 IPASSDT.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
        /* inRecIndex要指到剩下還沒存進bak檔的第一個記憶體位置 */
        inIPASSDTRec = inIPASSDTRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < lnIPASSDTLength; i++)
        {
                if (uszRead_Total_Buff[i] == 0x0A)
                {
                        j++;

                        if (j == inIPASSDTRec)
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
                /* 接續存原IPASSDT.dat的Record */
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
                                inFILE_Delete((unsigned char *)_IPASSDT_FILE_NAME_BAK_);

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
                                inFILE_Delete((unsigned char *)_IPASSDT_FILE_NAME_BAK_);

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

        /* 刪除原IPASSDT.dat */
        if (inFILE_Delete((unsigned char *)_IPASSDT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* 將IPASSDT.bak改名字為IPASSDT.dat取代原檔案 */
        if (inFILE_Rename((unsigned char *)_IPASSDT_FILE_NAME_BAK_, (unsigned char *)_IPASSDT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* inSaveIPASSDTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveIPASSDTRec(%d) END!!", inIPASSDTRec - 1);
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
Function        :inGetIPASS_SAM_Slot
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inGetIPASS_SAM_Slot(char* szIPASS_SAM_Slot)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szIPASS_SAM_Slot == NULL || strlen(srIPASSDTRec.szIPASS_SAM_Slot) <= 0 || strlen(srIPASSDTRec.szIPASS_SAM_Slot) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetIPASS_SAM_Slot() ERROR !!");

			if (szIPASS_SAM_Slot == NULL)
                        {
                                inLogPrintf(AT, "szIPASS_SAM_Slot == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIPASS_SAM_Slot length = (%d)", (int)strlen(srIPASSDTRec.szIPASS_SAM_Slot));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szIPASS_SAM_Slot[0], &srIPASSDTRec.szIPASS_SAM_Slot[0], strlen(srIPASSDTRec.szIPASS_SAM_Slot));

        return (VS_SUCCESS);
}

/*
Function        :inSetIPASS_SAM_Slot
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inSetIPASS_SAM_Slot(char* szIPASS_SAM_Slot)
{
        memset(srIPASSDTRec.szIPASS_SAM_Slot, 0x00, sizeof(srIPASSDTRec.szIPASS_SAM_Slot));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szIPASS_SAM_Slot == NULL || strlen(szIPASS_SAM_Slot) <= 0 || strlen(szIPASS_SAM_Slot) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetIPASS_SAM_Slot() ERROR !!");
                        if (szIPASS_SAM_Slot == NULL)
                        {
                                inLogPrintf(AT, "szIPASS_SAM_Slot == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIPASS_SAM_Slot length = (%d)", (int)strlen(szIPASS_SAM_Slot));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srIPASSDTRec.szIPASS_SAM_Slot[0], &szIPASS_SAM_Slot[0], strlen(szIPASS_SAM_Slot));

        return (VS_SUCCESS);
}

/*
Function        :inGetIPASS_Transaction_Function
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inGetIPASS_Transaction_Function(char* szIPASS_Transaction_Function)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szIPASS_Transaction_Function == NULL || strlen(srIPASSDTRec.szIPASS_Transaction_Function) <= 0 || strlen(srIPASSDTRec.szIPASS_Transaction_Function) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTransaction_Function() ERROR !!");

			if (szIPASS_Transaction_Function == NULL)
                        {
                                inLogPrintf(AT, "szIPASS_Transaction_Function == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIPASS_Transaction_Function length = (%d)", (int)strlen(srIPASSDTRec.szIPASS_Transaction_Function));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szIPASS_Transaction_Function[0], &srIPASSDTRec.szIPASS_Transaction_Function[0], strlen(srIPASSDTRec.szIPASS_Transaction_Function));

        return (VS_SUCCESS);
}

/*
Function        :inSetIPASS_Transaction_Function
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inSetIPASS_Transaction_Function(char* szIPASS_Transaction_Function)
{
        memset(srIPASSDTRec.szIPASS_Transaction_Function, 0x00, sizeof(srIPASSDTRec.szIPASS_Transaction_Function));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szIPASS_Transaction_Function == NULL || strlen(szIPASS_Transaction_Function) <= 0 || strlen(szIPASS_Transaction_Function) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTransaction_Function() ERROR !!");
                        if (szIPASS_Transaction_Function == NULL)
                        {
                                inLogPrintf(AT, "szIPASS_Transaction_Function == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIPASS_Transaction_Function length = (%d)", (int)strlen(szIPASS_Transaction_Function));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srIPASSDTRec.szIPASS_Transaction_Function[0], &szIPASS_Transaction_Function[0], strlen(szIPASS_Transaction_Function));

        return (VS_SUCCESS);
}

/*
Function        :inGetIPASS_AMS_TMS_IP_Address
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inGetIPASS_AMS_TMS_IP_Address(char* szIPASS_AMS_TMS_IP_Address)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szIPASS_AMS_TMS_IP_Address == NULL || strlen(srIPASSDTRec.szIPASS_AMS_TMS_IP_Address) <= 0 || strlen(srIPASSDTRec.szIPASS_AMS_TMS_IP_Address) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetAMS_TMS_IP_Address() ERROR !!");

			if (szIPASS_AMS_TMS_IP_Address == NULL)
                        {
                                inLogPrintf(AT, "szIPASS_AMS_TMS_IP_Address == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIPASS_AMS_TMS_IP_Address length = (%d)", (int)strlen(srIPASSDTRec.szIPASS_AMS_TMS_IP_Address));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szIPASS_AMS_TMS_IP_Address[0], &srIPASSDTRec.szIPASS_AMS_TMS_IP_Address[0], strlen(srIPASSDTRec.szIPASS_AMS_TMS_IP_Address));

        return (VS_SUCCESS);
}

/*
Function        :inSetIPASS_AMS_TMS_IP_Address
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inSetIPASS_AMS_TMS_IP_Address(char* szIPASS_AMS_TMS_IP_Address)
{
        memset(srIPASSDTRec.szIPASS_AMS_TMS_IP_Address, 0x00, sizeof(srIPASSDTRec.szIPASS_AMS_TMS_IP_Address));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szIPASS_AMS_TMS_IP_Address == NULL || strlen(szIPASS_AMS_TMS_IP_Address) <= 0 || strlen(szIPASS_AMS_TMS_IP_Address) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetAMS_TMS_IP_Address() ERROR !!");
                        if (szIPASS_AMS_TMS_IP_Address == NULL)
                        {
                                inLogPrintf(AT, "szIPASS_AMS_TMS_IP_Address == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIPASS_AMS_TMS_IP_Address length = (%d)", (int)strlen(szIPASS_AMS_TMS_IP_Address));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srIPASSDTRec.szIPASS_AMS_TMS_IP_Address[0], &szIPASS_AMS_TMS_IP_Address[0], strlen(szIPASS_AMS_TMS_IP_Address));

        return (VS_SUCCESS);
}

/*
Function        :inGetIPASS_BMS_IP_Address
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inGetIPASS_BMS_IP_Address(char* szIPASS_BMS_IP_Address)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szIPASS_BMS_IP_Address == NULL || strlen(srIPASSDTRec.szIPASS_BMS_IP_Address) <= 0 || strlen(srIPASSDTRec.szIPASS_BMS_IP_Address) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetBMS_IP_Address() ERROR !!");

			if (szIPASS_BMS_IP_Address == NULL)
                        {
                                inLogPrintf(AT, "szIPASS_BMS_IP_Address == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIPASS_BMS_IP_Address length = (%d)", (int)strlen(srIPASSDTRec.szIPASS_BMS_IP_Address));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szIPASS_BMS_IP_Address[0], &srIPASSDTRec.szIPASS_BMS_IP_Address[0], strlen(srIPASSDTRec.szIPASS_BMS_IP_Address));

        return (VS_SUCCESS);
}

/*
Function        :inSetIPASS_BMS_IP_Address
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inSetIPASS_BMS_IP_Address(char* szIPASS_BMS_IP_Address)
{
        memset(srIPASSDTRec.szIPASS_BMS_IP_Address, 0x00, sizeof(srIPASSDTRec.szIPASS_BMS_IP_Address));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szIPASS_BMS_IP_Address == NULL || strlen(szIPASS_BMS_IP_Address) <= 0 || strlen(szIPASS_BMS_IP_Address) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetBMS_IP_Address() ERROR !!");
                        if (szIPASS_BMS_IP_Address == NULL)
                        {
                                inLogPrintf(AT, "szIPASS_BMS_IP_Address == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIPASS_BMS_IP_Address length = (%d)", (int)strlen(szIPASS_BMS_IP_Address));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srIPASSDTRec.szIPASS_BMS_IP_Address[0], &szIPASS_BMS_IP_Address[0], strlen(szIPASS_BMS_IP_Address));

        return (VS_SUCCESS);
}

/*
Function        :inGetIPASS_System_ID
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inGetIPASS_System_ID(char* szIPASS_System_ID)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szIPASS_System_ID == NULL || strlen(srIPASSDTRec.szIPASS_System_ID) <= 0 || strlen(srIPASSDTRec.szIPASS_System_ID) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetSystem_ID() ERROR !!");

			if (szIPASS_System_ID == NULL)
                        {
                                inLogPrintf(AT, "szIPASS_System_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIPASS_System_ID length = (%d)", (int)strlen(srIPASSDTRec.szIPASS_System_ID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szIPASS_System_ID[0], &srIPASSDTRec.szIPASS_System_ID[0], strlen(srIPASSDTRec.szIPASS_System_ID));

        return (VS_SUCCESS);
}

/*
Function        :inSetIPASS_System_ID
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inSetIPASS_System_ID(char* szIPASS_System_ID)
{
        memset(srIPASSDTRec.szIPASS_System_ID, 0x00, sizeof(srIPASSDTRec.szIPASS_System_ID));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szIPASS_System_ID == NULL || strlen(szIPASS_System_ID) <= 0 || strlen(szIPASS_System_ID) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetSystem_ID() ERROR !!");
                        if (szIPASS_System_ID == NULL)
                        {
                                inLogPrintf(AT, "szIPASS_System_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIPASS_System_ID length = (%d)", (int)strlen(szIPASS_System_ID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srIPASSDTRec.szIPASS_System_ID[0], &szIPASS_System_ID[0], strlen(szIPASS_System_ID));

        return (VS_SUCCESS);
}

/*
Function        :inGetIPASS_SP_ID
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inGetIPASS_SP_ID(char* szIPASS_SP_ID)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szIPASS_SP_ID == NULL || strlen(srIPASSDTRec.szIPASS_SP_ID) <= 0 || strlen(srIPASSDTRec.szIPASS_SP_ID) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetSP_ID() ERROR !!");

			if (szIPASS_SP_ID == NULL)
                        {
                                inLogPrintf(AT, "szIPASS_SP_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIPASS_SP_ID length = (%d)", (int)strlen(srIPASSDTRec.szIPASS_SP_ID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szIPASS_SP_ID[0], &srIPASSDTRec.szIPASS_SP_ID[0], strlen(srIPASSDTRec.szIPASS_SP_ID));

        return (VS_SUCCESS);
}

/*
Function        :inSetIPASS_SP_ID
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inSetIPASS_SP_ID(char* szIPASS_SP_ID)
{
        memset(srIPASSDTRec.szIPASS_SP_ID, 0x00, sizeof(srIPASSDTRec.szIPASS_SP_ID));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szIPASS_SP_ID == NULL || strlen(szIPASS_SP_ID) <= 0 || strlen(szIPASS_SP_ID) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetSP_ID() ERROR !!");
                        if (szIPASS_SP_ID == NULL)
                        {
                                inLogPrintf(AT, "szIPASS_SP_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIPASS_SP_ID length = (%d)", (int)strlen(szIPASS_SP_ID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srIPASSDTRec.szIPASS_SP_ID[0], &szIPASS_SP_ID[0], strlen(szIPASS_SP_ID));

        return (VS_SUCCESS);
}

/*
Function        :inGetIPASS_Sub_Company_ID
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inGetIPASS_Sub_Company_ID(char* szIPASS_Sub_Company_ID)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szIPASS_Sub_Company_ID == NULL || strlen(srIPASSDTRec.szIPASS_Sub_Company_ID) <= 0 || strlen(srIPASSDTRec.szIPASS_Sub_Company_ID) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetSub_Company_ID() ERROR !!");

			if (szIPASS_Sub_Company_ID == NULL)
                        {
                                inLogPrintf(AT, "szIPASS_Sub_Company_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIPASS_Sub_Company_ID length = (%d)", (int)strlen(srIPASSDTRec.szIPASS_Sub_Company_ID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szIPASS_Sub_Company_ID[0], &srIPASSDTRec.szIPASS_Sub_Company_ID[0], strlen(srIPASSDTRec.szIPASS_Sub_Company_ID));

        return (VS_SUCCESS);
}

/*
Function        :inSetIPASS_Sub_Company_ID
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inSetIPASS_Sub_Company_ID(char* szIPASS_Sub_Company_ID)
{
        memset(srIPASSDTRec.szIPASS_Sub_Company_ID, 0x00, sizeof(srIPASSDTRec.szIPASS_Sub_Company_ID));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szIPASS_Sub_Company_ID == NULL || strlen(szIPASS_Sub_Company_ID) <= 0 || strlen(szIPASS_Sub_Company_ID) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetSub_Company_ID() ERROR !!");
                        if (szIPASS_Sub_Company_ID == NULL)
                        {
                                inLogPrintf(AT, "szIPASS_Sub_Company_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIPASS_Sub_Company_ID length = (%d)", (int)strlen(szIPASS_Sub_Company_ID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srIPASSDTRec.szIPASS_Sub_Company_ID[0], &szIPASS_Sub_Company_ID[0], strlen(szIPASS_Sub_Company_ID));

        return (VS_SUCCESS);
}


/*
Function        :inGetIPASS_Shop_ID
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inGetIPASS_Shop_ID(char* szIPASS_Shop_ID)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szIPASS_Shop_ID == NULL || strlen(srIPASSDTRec.szIPASS_Shop_ID) <= 0 || strlen(srIPASSDTRec.szIPASS_Shop_ID) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetShop_ID() ERROR !!");

			if (szIPASS_Shop_ID == NULL)
                        {
                                inLogPrintf(AT, "szIPASS_Shop_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIPASS_Shop_ID length = (%d)", (int)strlen(srIPASSDTRec.szIPASS_Shop_ID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szIPASS_Shop_ID[0], &srIPASSDTRec.szIPASS_Shop_ID[0], strlen(srIPASSDTRec.szIPASS_Shop_ID));

        return (VS_SUCCESS);
}

/*
Function        :inSetIPASS_Shop_ID
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inSetIPASS_Shop_ID(char* szIPASS_Shop_ID)
{
        memset(srIPASSDTRec.szIPASS_Shop_ID, 0x00, sizeof(srIPASSDTRec.szIPASS_Shop_ID));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szIPASS_Shop_ID == NULL || strlen(szIPASS_Shop_ID) <= 0 || strlen(szIPASS_Shop_ID) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetShop_ID() ERROR !!");
                        if (szIPASS_Shop_ID == NULL)
                        {
                                inLogPrintf(AT, "szIPASS_Shop_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIPASS_Shop_ID length = (%d)", (int)strlen(szIPASS_Shop_ID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srIPASSDTRec.szIPASS_Shop_ID[0], &szIPASS_Shop_ID[0], strlen(szIPASS_Shop_ID));

        return (VS_SUCCESS);
}

/*
Function        :inGetIPASS_Shop_ID
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inGetIPASS_POS_ID(char* szIPASS_POS_ID)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szIPASS_POS_ID == NULL || strlen(srIPASSDTRec.szIPASS_POS_ID) <= 0 || strlen(srIPASSDTRec.szIPASS_POS_ID) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPOS_ID() ERROR !!");

			if (szIPASS_POS_ID == NULL)
                        {
                                inLogPrintf(AT, "szIPASS_POS_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIPASS_POS_ID length = (%d)", (int)strlen(srIPASSDTRec.szIPASS_POS_ID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szIPASS_POS_ID[0], &srIPASSDTRec.szIPASS_POS_ID[0], strlen(srIPASSDTRec.szIPASS_POS_ID));

        return (VS_SUCCESS);
}

/*
Function        :inSetIPASS_POS_ID
Date&Time       :2017/12/18 上午 11:33
Describe        :
*/
int inSetIPASS_POS_ID(char* szIPASS_POS_ID)
{
        memset(srIPASSDTRec.szIPASS_POS_ID, 0x00, sizeof(srIPASSDTRec.szIPASS_POS_ID));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szIPASS_POS_ID == NULL || strlen(szIPASS_POS_ID) <= 0 || strlen(szIPASS_POS_ID) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPOS_ID() ERROR !!");
                        if (szIPASS_POS_ID == NULL)
                        {
                                inLogPrintf(AT, "szIPASS_POS_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIPASS_POS_ID length = (%d)", (int)strlen(szIPASS_POS_ID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srIPASSDTRec.szIPASS_POS_ID[0], &szIPASS_POS_ID[0], strlen(szIPASS_POS_ID));

        return (VS_SUCCESS);
}

/*
Function        :inIPASSDT_Edit_IPASSDT_Table
Date&Time       :2017/5/15 下午 4:08
Describe        :
*/
int inIPASSDT_Edit_IPASSDT_Table(void)
{
	TABLE_GET_SET_TABLE IPASSDT_FUNC_TABLE[] =
	{
		{"szIPASS_SAM_Slot"		,inGetIPASS_SAM_Slot			,inSetIPASS_SAM_Slot			},
		{"szIPASS_Transaction_Function"	,inGetIPASS_Transaction_Function	,inSetIPASS_Transaction_Function	},
		{"szIPASS_AMS_TMS_IP_Address"	,inGetIPASS_AMS_TMS_IP_Address		,inSetIPASS_AMS_TMS_IP_Address		},
		{"szIPASS_BMS_IP_Address"	,inGetIPASS_BMS_IP_Address		,inSetIPASS_BMS_IP_Address		},
		{"szIPASS_System_ID"		,inGetIPASS_System_ID			,inSetIPASS_System_ID			},
		{"szIPASS_SP_ID"		,inGetIPASS_SP_ID			,inSetIPASS_SP_ID			},
		{"szIPASS_Sub_Company_ID"	,inGetIPASS_Sub_Company_ID		,inSetIPASS_Sub_Company_ID		},
		{"szIPASS_Shop_ID"		,inGetIPASS_Shop_ID			,inSetIPASS_Shop_ID			},
		{"szIPASS_POS_ID"		,inGetIPASS_POS_ID			,inSetIPASS_POS_ID			},
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
	inDISP_ChineseFont_Color("是否更改IPASSDT", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
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
	inLoadIPASSDTRec(inRecordCnt);
	
	inFunc_Edit_Table_Tag(IPASSDT_FUNC_TABLE);
	inSaveIPASSDTRec(inRecordCnt);
	
	return	(VS_SUCCESS);
}
