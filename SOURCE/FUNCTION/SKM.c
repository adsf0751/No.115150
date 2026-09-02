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
#include "SKM.h"

static  SKM_REC srSKMRec;	/* construct SKM record */
extern  int     ginDebug;  	/* Debug使用 extern */

/*
Function        :inLoadSKMRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀SKM檔案，inSKM_Rec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadSKMRec(int inSKMRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆SKM檔案 */
        char            szSKMRec[_SIZE_SKM_REC_ + 1];           /* 暫存, 放各個欄位檔案 */
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnSKMLength = 0;                        /* SKM總長度 */
        long            lnReadLength;                           /* 記錄每次要從SKM.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;                 /* inRec記錄讀到第幾筆, i為目前從SKM讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */

        /* inLoadSKMRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadSKMRec(%d) START!!", inSKMRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 判斷傳進來的inSKMRec是否小於零 大於等於零才是正確值(防呆) */
        if (inSKMRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inSKMRec < 0:(index = %d) ERROR!!", inSKMRec);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open SKM.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_SKM_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnSKMLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_SKM_FILE_NAME_);
        
	if (lnSKMLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnSKMLength + 1);
        uszTemp = malloc(lnSKMLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnSKMLength + 1);
        memset(uszTemp, 0x00, lnSKMLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnSKMLength;

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
                } /* end for loop */
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
         *i為目前從SKM讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnSKMLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到SKM的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

			/* 清空uszReadData */
                        memset(uszReadData, 0x00, lnSKMLength + 1);
			/* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
			/* 因為inSKM_Rec的index從0開始，所以inSKM_Rec要+1 */
                        if (inRec == (inSKMRec + 1))
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
         * 如果沒有inSKMRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inSKMRec + 1) || inSearchResult == -1)
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
        memset(&srSKMRec, 0x00, sizeof(srSKMRec));
        /*
         * 以下pattern為存入SKM_Rec
         * i為SKM的第幾個字元
         * 存入SKM_Rec
         */
        i = 0;


        /* 01_密鑰群組索引 */
	/* 初始化 */
        memset(szSKMRec, 0x00, sizeof(szSKMRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szSKMRec[k ++] = uszReadData[i ++];
                if (szSKMRec[k - 1] == 0x2C	||
		    szSKMRec[k - 1] == 0x0D	||
		    szSKMRec[k - 1] == 0x0A	||
		    szSKMRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnSKMLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "SKM unpack ERROR");
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
        if (szSKMRec[0] != 0x2C	&&
	    szSKMRec[0] != 0x0D	&&
	    szSKMRec[0] != 0x0A	&&
	    szSKMRec[0] != 0x00)
        {
                memcpy(&srSKMRec.szKeyGroupIndex[0], &szSKMRec[0], k - 1);
        }

        /* 02_密鑰群組 */
	/* 初始化 */
        memset(szSKMRec, 0x00, sizeof(szSKMRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szSKMRec[k ++] = uszReadData[i ++];
                if (szSKMRec[k - 1] == 0x2C	||
		    szSKMRec[k - 1] == 0x0D	||
		    szSKMRec[k - 1] == 0x0A	||
		    szSKMRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnSKMLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "SKM unpack ERROR");
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
        if (szSKMRec[0] != 0x2C	&&
	    szSKMRec[0] != 0x0D	&&
	    szSKMRec[0] != 0x0A	&&
	    szSKMRec[0] != 0x00)
        {
                memcpy(&srSKMRec.szKeyGroupID[0], &szSKMRec[0], k - 1);
        }

        /* 03_最後一次密鑰群組修改時間 */
	/* 初始化 */
        memset(szSKMRec, 0x00, sizeof(szSKMRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szSKMRec[k ++] = uszReadData[i ++];
                if (szSKMRec[k - 1] == 0x2C	||
		    szSKMRec[k - 1] == 0x0D	||
		    szSKMRec[k - 1] == 0x0A	||
		    szSKMRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnSKMLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "SKM unpack ERROR");
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
        if (szSKMRec[0] != 0x2C	&&
	    szSKMRec[0] != 0x0D	&&
	    szSKMRec[0] != 0x0A	&&
	    szSKMRec[0] != 0x00)
        {
                memcpy(&srSKMRec.szLastUpdateTime[0], &szSKMRec[0], k - 1);
        }

        /* 04_Session_Data */
	/* 初始化 */
        memset(szSKMRec, 0x00, sizeof(szSKMRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szSKMRec[k ++] = uszReadData[i ++];
                if (szSKMRec[k - 1] == 0x2C	||
		    szSKMRec[k - 1] == 0x0D	||
		    szSKMRec[k - 1] == 0x0A	||
		    szSKMRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnSKMLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "SKM unpack ERROR");
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
        if (szSKMRec[0] != 0x2C	&&
	    szSKMRec[0] != 0x0D	&&
	    szSKMRec[0] != 0x0A	&&
	    szSKMRec[0] != 0x00)
        {
                memcpy(&srSKMRec.szSessionData[0], &szSKMRec[0], k - 1);
        }

        /* 05_Tsam_BIN */
	/* 初始化 */
        memset(szSKMRec, 0x00, sizeof(szSKMRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szSKMRec[k ++] = uszReadData[i ++];
                if (szSKMRec[k - 1] == 0x2C	||
		    szSKMRec[k - 1] == 0x0D	||
		    szSKMRec[k - 1] == 0x0A	||
		    szSKMRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnSKMLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "SKM unpack ERROR");
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
        if (szSKMRec[0] != 0x2C	&&
	    szSKMRec[0] != 0x0D	&&
	    szSKMRec[0] != 0x0A	&&
	    szSKMRec[0] != 0x00)
        {
                memcpy(&srSKMRec.szTsamBIN[0], &szSKMRec[0], k - 1);
        }

        /* 06_Key_Set */
	/* 初始化 */
        memset(szSKMRec, 0x00, sizeof(szSKMRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szSKMRec[k ++] = uszReadData[i ++];
                if (szSKMRec[k - 1] == 0x2C	||
		    szSKMRec[k - 1] == 0x0D	||
		    szSKMRec[k - 1] == 0x0A	||
		    szSKMRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnSKMLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "SKM unpack ERROR");
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
        if (szSKMRec[0] != 0x2C	&&
	    szSKMRec[0] != 0x0D	&&
	    szSKMRec[0] != 0x0A	&&
	    szSKMRec[0] != 0x00)
        {
                memcpy(&srSKMRec.szKeySet[0], &szSKMRec[0], k - 1);
        }

        /* 07_Key_Index */
	/* 初始化 */
        memset(szSKMRec, 0x00, sizeof(szSKMRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szSKMRec[k ++] = uszReadData[i ++];
                if (szSKMRec[k - 1] == 0x2C	||
		    szSKMRec[k - 1] == 0x0D	||
		    szSKMRec[k - 1] == 0x0A	||
		    szSKMRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnSKMLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "SKM unpack ERROR");
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
        if (szSKMRec[0] != 0x2C	&&
	    szSKMRec[0] != 0x0D	&&
	    szSKMRec[0] != 0x0A	&&
	    szSKMRec[0] != 0x00)
        {
                memcpy(&srSKMRec.szKeyIndex[0], &szSKMRec[0], k - 1);
        }

        /* 08_Key_Value */
	/* 初始化 */
        memset(szSKMRec, 0x00, sizeof(szSKMRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szSKMRec[k ++] = uszReadData[i ++];
                if (szSKMRec[k - 1] == 0x2C	||
		    szSKMRec[k - 1] == 0x0D	||
		    szSKMRec[k - 1] == 0x0A	||
		    szSKMRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnSKMLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "SKM unpack ERROR");
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
        if (szSKMRec[0] != 0x2C	&&
	    szSKMRec[0] != 0x0D	&&
	    szSKMRec[0] != 0x0A	&&
	    szSKMRec[0] != 0x00)
        {
                memcpy(&srSKMRec.szKeyValue[0], &szSKMRec[0], k - 1);
        }

        /* 09_Key_Check_Value */
	/* 初始化 */
        memset(szSKMRec, 0x00, sizeof(szSKMRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szSKMRec[k ++] = uszReadData[i ++];
                if (szSKMRec[k - 1] == 0x2C	||
		    szSKMRec[k - 1] == 0x0D	||
		    szSKMRec[k - 1] == 0x0A	||
		    szSKMRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnSKMLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "SKM unpack ERROR");
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
        if (szSKMRec[0] != 0x2C	&&
	    szSKMRec[0] != 0x0D	&&
	    szSKMRec[0] != 0x0A	&&
	    szSKMRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srSKMRec.szKeyCheckValue[0], &szSKMRec[0], k - 1);
        }

        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

	/* inLoadSKMRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadSKMRec(%d) END!!", inSKMRec);
                inLogPrintf(AT, szErrorMsg);
		inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveSKMRec
Date&Time       :
Describe        :
*/
int inSaveSKMRec(int inSKMRec)
{
        unsigned long   uldat_Handle;   			/* FILE Handle */
        unsigned long   ulbak_Handle;   			/* FILE Handle */
        int     	inPackCount = 0;        		/* uszWriteBuff_Record的index */
        int     	inRecIndex = 0;         		/* uszRead_Total_Buff的index */
        int     	inSKM_Total_Rec = 0;    		/* SKM.dat的總筆數 */
        int     	inRetVal;
        int     	i = 0, j = 0;
	char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];	/* debug message */
        long    	lnReadLength = 0;       		/* 每次要從SKM.dat讀多少byte出來 */
        long    	lnSKMLength = 0;         		/* SKM.dat檔案總長度 */
        unsigned char   *uszRead_Total_Buff;
        unsigned char   *uszWriteBuff_Record, *uszWriteBuff_Org;


        /* inSaveSKMRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveSKMRec(%d) START!!", inSKMRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 防呆先刪除SKM.bak  */
        inFILE_Delete((unsigned char *)_SKM_FILE_NAME_BAK_);

        /* 新建SKM.bak檔案 */
        inRetVal = inFILE_Create(&ulbak_Handle, (unsigned char *)_SKM_FILE_NAME_BAK_);

        if (inRetVal != VS_SUCCESS)
        {
                inFILE_Close(&ulbak_Handle);

                return (VS_ERROR);
        }

        /* 開啟原檔案SKM.dat */
        inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_SKM_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                /* SKM.dat開檔失敗 ，不用關檔SKM.dat */
                /* SKM.bak仍要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_SKM_FILE_NAME_BAK_);

                return(VS_ERROR);
        }

        /* 取得SKM.dat檔案大小 */
        lnSKMLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *)_SKM_FILE_NAME_);

	/* 取得檔案大小失敗 */
        if (lnSKMLength == VS_ERROR)
        {
		/* SKM.bak和SKM.dat要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Close(&uldat_Handle);
                inFILE_Delete((unsigned char *)_SKM_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        uszWriteBuff_Record = malloc(_SIZE_SKM_REC_ + _SIZE_SKM_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_SKM_REC_ + _SIZE_SKM_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原SKM.dat的每筆Record 初始化 */
        uszWriteBuff_Org = malloc(_SIZE_SKM_REC_ + _SIZE_SKM_COMMA_0D0A_);
        memset(uszWriteBuff_Org, 0x00, _SIZE_SKM_REC_ + _SIZE_SKM_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存SKM.dat全部資料 */
        uszRead_Total_Buff = malloc(lnSKMLength + 1);
        memset(uszRead_Total_Buff, 0x00, lnSKMLength + 1);

	inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

	/* KeyGroupIndex */
        memcpy(&uszWriteBuff_Record[0], &srSKMRec.szKeyGroupIndex[0], strlen(srSKMRec.szKeyGroupIndex));
        inPackCount += strlen(srSKMRec.szKeyGroupIndex);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* KeyGroupID */
        memcpy(&uszWriteBuff_Record[inPackCount], &srSKMRec.szKeyGroupID[0], strlen(srSKMRec.szKeyGroupID));
        inPackCount += strlen(srSKMRec.szKeyGroupID);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* LastUpdateTime */
        memcpy(&uszWriteBuff_Record[inPackCount], &srSKMRec.szLastUpdateTime[0], strlen(srSKMRec.szLastUpdateTime));
        inPackCount += strlen(srSKMRec.szLastUpdateTime);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* SessionData */
        memcpy(&uszWriteBuff_Record[inPackCount], &srSKMRec.szSessionData[0], strlen(srSKMRec.szSessionData));
        inPackCount += strlen(srSKMRec.szSessionData);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* TsamBIN */
        memcpy(&uszWriteBuff_Record[inPackCount], &srSKMRec.szTsamBIN[0], strlen(srSKMRec.szTsamBIN));
        inPackCount += strlen(srSKMRec.szTsamBIN);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* KeySet */
        memcpy(&uszWriteBuff_Record[inPackCount], &srSKMRec.szKeySet[0], strlen(srSKMRec.szKeySet));
        inPackCount += strlen(srSKMRec.szKeySet);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* KeyIndex */
        memcpy(&uszWriteBuff_Record[inPackCount], &srSKMRec.szKeyIndex[0], strlen(srSKMRec.szKeyIndex));
        inPackCount += strlen(srSKMRec.szKeyIndex);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* KeyValue */
        memcpy(&uszWriteBuff_Record[inPackCount], &srSKMRec.szKeyValue[0], strlen(srSKMRec.szKeyValue));
        inPackCount += strlen(srSKMRec.szKeyValue);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* KeyCheckValue */
        memcpy(&uszWriteBuff_Record[inPackCount], &srSKMRec.szKeyCheckValue[0], strlen(srSKMRec.szKeyCheckValue));
        inPackCount += strlen(srSKMRec.szKeyCheckValue);

        /* 最後的data不用逗號 */
        /* 補上換行符號 */

        /* 0D是移至行首 */
        uszWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        uszWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;

        /* 讀SKM.dat檔案存到uszRead_Total_Buff */
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnSKMLength;

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
                                        inFILE_Delete((unsigned char *)_SKM_FILE_NAME_BAK_);
                                        
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
                                        inFILE_Delete((unsigned char *)_SKM_FILE_NAME_BAK_);
                                        
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
                inFILE_Delete((unsigned char *)_SKM_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        /* 防呆 總record數量小於要存取inSKMRec Return ERROR */
        /* 算總Record數 */
	for (i = 0; i < (lnSKMLength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (uszRead_Total_Buff[i] == 0x0A)
                        inSKM_Total_Rec++;
        }

	/* 防呆 總record數量小於要存取inSKMRec Return ERROR */
        if ((inSKMRec + 1) > inSKM_Total_Rec)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "No data or Index ERROR");
                }

                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_SKM_FILE_NAME_BAK_);
                
                /* Free pointer */
		free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                return (VS_ERROR);
        }

        /* 記錄ReadTotalBuffer的陣列位置 */
        inRecIndex = 0;
        /* 用inSKMRec決定要先存幾筆Record到SKM.bak，ex:inSKMRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inSKMRec; i ++)
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
                                        inFILE_Delete((unsigned char *)_SKM_FILE_NAME_BAK_);
                                        
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
                                        inFILE_Delete((unsigned char *)_SKM_FILE_NAME_BAK_);
                                        
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

        /* 存組好的該SKMRecord 到 SKM.bak */
        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_SKM_FILE_NAME_BAK_);
                
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
                inFILE_Delete((unsigned char *)_SKM_FILE_NAME_BAK_);
                
                /* Free pointer */
		free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原SKM.dat Record 到 SKM.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
	/* inRecIndex要指到剩下還沒存進bak檔的第一個記憶體位置 */
	inSKMRec = inSKMRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < lnSKMLength; i++)
        {
                if (uszRead_Total_Buff[i] == 0x0A)
                {
                        j++;

                        if (j == inSKMRec)
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
                /* 接續存原SKM.dat的Record */
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
                                inFILE_Delete((unsigned char *)_SKM_FILE_NAME_BAK_);
                                
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
                                inFILE_Delete((unsigned char *)_SKM_FILE_NAME_BAK_);
                                
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

        /* 刪除原SKM.dat */
        if (inFILE_Delete((unsigned char *)_SKM_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* 將SKM.bak改名字為SKM.dat取代原檔案 */
        if (inFILE_Rename((unsigned char *)_SKM_FILE_NAME_BAK_, (unsigned char *)_SKM_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* inSaveSKMRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveSKMRec(%d) END!!", inSKMRec - 1);
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
Function        :inGetKeyGroupIndex
Date&Time       :
Describe        :
*/
int inGetKeyGroupIndex(char* szKeyGroupIndex)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szKeyGroupIndex == NULL || strlen(srSKMRec.szKeyGroupIndex) <= 0 || strlen(srSKMRec.szKeyGroupIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetKeyGroupIndex() ERROR !!");

                        if (szKeyGroupIndex == NULL)
                        {
                                inLogPrintf(AT, "szKeyGroupIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szKeyGroupIndex length = (%d)", (int)strlen(srSKMRec.szKeyGroupIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szKeyGroupIndex[0], &srSKMRec.szKeyGroupIndex[0], strlen(srSKMRec.szKeyGroupIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetKeyGroupIndex
Date&Time       :
Describe        :
*/
int inSetKeyGroupIndex(char* szKeyGroupIndex)
{
        memset(srSKMRec.szKeyGroupIndex, 0x00, sizeof(srSKMRec.szKeyGroupIndex));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szKeyGroupIndex == NULL || strlen(szKeyGroupIndex) <= 0 || strlen(szKeyGroupIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetKeyGroupIndex() ERROR !!");

                        if (szKeyGroupIndex == NULL)
                        {
                                inLogPrintf(AT, "szKeyGroupIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szKeyGroupIndex length = (%d)", (int)strlen(szKeyGroupIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srSKMRec.szKeyGroupIndex[0], &szKeyGroupIndex[0], strlen(szKeyGroupIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetKeyGroupID
Date&Time       :
Describe        :
*/
int inGetKeyGroupID(char* szKeyGroupID)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szKeyGroupID == NULL || strlen(srSKMRec.szKeyGroupID) <= 0 || strlen(srSKMRec.szKeyGroupID) > 13)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetKeyGroupID() ERROR !!");

                        if (szKeyGroupID == NULL)
                        {
                                inLogPrintf(AT, "szKeyGroupID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szKeyGroupID length = (%d)", (int)strlen(srSKMRec.szKeyGroupID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szKeyGroupID[0], &srSKMRec.szKeyGroupID[0], strlen(srSKMRec.szKeyGroupID));

        return (VS_SUCCESS);

}

/*
Function        :inSetKeyGroupID
Date&Time       :
Describe        :
*/
int inSetKeyGroupID(char* szKeyGroupID)
{
        memset(srSKMRec.szKeyGroupID, 0x00, sizeof(srSKMRec.szKeyGroupID));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szKeyGroupID == NULL || strlen(szKeyGroupID) <= 0 || strlen(szKeyGroupID) > 13)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetKeyGroupID() ERROR !!");

                        if (szKeyGroupID == NULL)
                        {
                                inLogPrintf(AT, "szKeyGroupID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szKeyGroupID length = (%d)", (int)strlen(szKeyGroupID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srSKMRec.szKeyGroupID[0], &szKeyGroupID[0], strlen(szKeyGroupID));

        return (VS_SUCCESS);
}

/*
Function        :inGetLastUpdateTime
Date&Time       :
Describe        :
*/
int inGetLastUpdateTime(char* szLastUpdateTime)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szLastUpdateTime == NULL || strlen(srSKMRec.szLastUpdateTime) <= 0 || strlen(srSKMRec.szLastUpdateTime) > 14)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetLastUpdateTime() ERROR !!");

                        if (szLastUpdateTime == NULL)
                        {
                                inLogPrintf(AT, "szLastUpdateTime == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szLastUpdateTime length = (%d)", (int)strlen(szLastUpdateTime));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szLastUpdateTime[0], &srSKMRec.szLastUpdateTime[0], strlen(srSKMRec.szLastUpdateTime));

        return (VS_SUCCESS);
}

/*
Function        :inSetLastUpdateTime
Date&Time       :
Describe        :
*/
int inSetLastUpdateTime(char* szLastUpdateTime)
{
        memset(srSKMRec.szLastUpdateTime, 0x00, sizeof(srSKMRec.szLastUpdateTime));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szLastUpdateTime == NULL || strlen(szLastUpdateTime) <= 0 || strlen(szLastUpdateTime) > 14)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetLastUpdateTime() ERROR !!");

                        if (szLastUpdateTime == NULL)
                        {
                                inLogPrintf(AT, "szLastUpdateTime == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szLastUpdateTime length = (%d)", (int)strlen(szLastUpdateTime));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srSKMRec.szLastUpdateTime[0], &szLastUpdateTime[0], strlen(szLastUpdateTime));

        return (VS_SUCCESS);
}

/*
Function        :inGetSessionData
Date&Time       :
Describe        :
*/
int inGetSessionData(char* szSessionData)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSessionData == NULL || strlen(srSKMRec.szSessionData) <= 0 || strlen(srSKMRec.szSessionData) > 32)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetSessionData() ERROR !!");

                        if (szSessionData == NULL)
                        {
                                inLogPrintf(AT, "szSessionData == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSessionData length = (%d)", (int)strlen(szSessionData));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSessionData[0], &srSKMRec.szSessionData[0], strlen(srSKMRec.szSessionData));

        return (VS_SUCCESS);
}

/*
Function        :inSetSessionData
Date&Time       :
Describe        :
*/
int inSetSessionData(char* szSessionData)
{
        memset(srSKMRec.szSessionData, 0x00, sizeof(srSKMRec.szSessionData));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSessionData == NULL || strlen(szSessionData) <= 0 || strlen(szSessionData) > 32)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetSessionData() ERROR !!");

                        if (szSessionData == NULL)
                        {
                                inLogPrintf(AT, "szSessionData == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSessionData length = (%d)", (int)strlen(szSessionData));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srSKMRec.szSessionData[0], &szSessionData[0], strlen(szSessionData));

        return (VS_SUCCESS);
}

/*
Function        :inGetTsamBIN
Date&Time       :
Describe        :
*/
int inGetTsamBIN(char* szTsamBIN)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTsamBIN == NULL || strlen(srSKMRec.szTsamBIN) <= 0 || strlen(srSKMRec.szTsamBIN) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTsamBIN() ERROR !!");

                        if (szTsamBIN == NULL)
                        {
                                inLogPrintf(AT, "szTsamBIN == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTsamBIN length = (%d)", (int)strlen(szTsamBIN));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTsamBIN[0], &srSKMRec.szTsamBIN[0], strlen(srSKMRec.szTsamBIN));

        return (VS_SUCCESS);
}

/*
Function        :inSetTsamBIN
Date&Time       :
Describe        :
*/
int inSetTsamBIN(char* szTsamBIN)
{
        memset(srSKMRec.szTsamBIN, 0x00, sizeof(srSKMRec.szTsamBIN));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTsamBIN == NULL || strlen(szTsamBIN) <= 0 || strlen(szTsamBIN) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTsamBIN() ERROR !!");

                        if (szTsamBIN == NULL)
                        {
                                inLogPrintf(AT, "szTsamBIN == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTsamBIN length = (%d)", (int)strlen(szTsamBIN));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srSKMRec.szTsamBIN[0], &szTsamBIN[0], strlen(szTsamBIN));

        return (VS_SUCCESS);
}

/*
Function        :inGetKeySet
Date&Time       :
Describe        :
*/
int inGetKeySet(char* szKeySet)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szKeySet == NULL || strlen(srSKMRec.szKeySet) <= 0 || strlen(srSKMRec.szKeySet) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetKeySet() ERROR !!");

                        if (szKeySet == NULL)
                        {
                                inLogPrintf(AT, "szKeySet == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szKeySet length = (%d)", (int)strlen(szKeySet));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szKeySet[0], &srSKMRec.szKeySet[0], strlen(srSKMRec.szKeySet));

        return (VS_SUCCESS);
}

/*
Function        :inSetKeySet
Date&Time       :
Describe        :
*/
int inSetKeySet(char* szKeySet)
{
        memset(srSKMRec.szKeySet, 0x00, sizeof(srSKMRec.szKeySet));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szKeySet == NULL || strlen(szKeySet) <= 0 || strlen(szKeySet) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetKeySet() ERROR !!");

                        if (szKeySet == NULL)
                        {
                                inLogPrintf(AT, "szKeySet == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szKeySet length = (%d)", (int)strlen(szKeySet));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srSKMRec.szKeySet[0], &szKeySet[0], strlen(szKeySet));

        return (VS_SUCCESS);
}

/*
Function        :inGetKeyIndex
Date&Time       :
Describe        :
*/
int inGetKeyIndex(char* szKeyIndex)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szKeyIndex == NULL || strlen(srSKMRec.szKeyIndex) <= 0 || strlen(srSKMRec.szKeyIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetKeyIndex() ERROR !!");

                        if (szKeyIndex == NULL)
                        {
                                inLogPrintf(AT, "szKeyIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szKeyIndex length = (%d)", (int)strlen(szKeyIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szKeyIndex[0], &srSKMRec.szKeyIndex[0], strlen(srSKMRec.szKeyIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetKeyIndex
Date&Time       :
Describe        :
*/
int inSetKeyIndex(char* szKeyIndex)
{
        memset(srSKMRec.szKeyIndex, 0x00, sizeof(srSKMRec.szKeyIndex));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szKeyIndex == NULL || strlen(szKeyIndex) <= 0 || strlen(szKeyIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetKeyIndex() ERROR !!");

                        if (szKeyIndex == NULL)
                        {
                                inLogPrintf(AT, "szKeyIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szKeyIndex length = (%d)", (int)strlen(szKeyIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srSKMRec.szKeyIndex[0], &szKeyIndex[0], strlen(szKeyIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetKeyValue
Date&Time       :
Describe        :
*/
int inGetKeyValue(char* szKeyValue)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szKeyValue == NULL || strlen(srSKMRec.szKeyValue) <= 0 || strlen(srSKMRec.szKeyValue) > 32)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetKeyValue() ERROR !!");

                        if (szKeyValue == NULL)
                        {
                                inLogPrintf(AT, "szKeyValue == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szKeyValue length = (%d)", (int)strlen(szKeyValue));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szKeyValue[0], &srSKMRec.szKeyValue[0], strlen(srSKMRec.szKeyValue));

        return (VS_SUCCESS);
}

/*
Function        :inSetKeyValue
Date&Time       :
Describe        :
*/
int inSetKeyValue(char* szKeyValue)
{
        memset(srSKMRec.szKeyValue, 0x00, sizeof(srSKMRec.szKeyValue));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szKeyValue == NULL || strlen(szKeyValue) <= 0 || strlen(szKeyValue) > 32)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetKeyValue() ERROR !!");

                        if (szKeyValue == NULL)
                        {
                                inLogPrintf(AT, "szKeyValue == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szKeyValue length = (%d)", (int)strlen(szKeyValue));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srSKMRec.szKeyValue[0], &szKeyValue[0], strlen(szKeyValue));

        return (VS_SUCCESS);
}

/*
Function        :inGetKeyCheckValue
Date&Time       :
Describe        :
*/
int inGetKeyCheckValue(char* szKeyCheckValue)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szKeyCheckValue == NULL || strlen(srSKMRec.szKeyCheckValue) <= 0 || strlen(srSKMRec.szKeyCheckValue) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetKeyCheckValue() ERROR !!");

                        if (szKeyCheckValue == NULL)
                        {
                                inLogPrintf(AT, "szKeyCheckValue == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szKeyCheckValue length = (%d)", (int)strlen(szKeyCheckValue));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szKeyCheckValue[0], &srSKMRec.szKeyCheckValue[0], strlen(srSKMRec.szKeyCheckValue));

        return (VS_SUCCESS);
}

/*
Function        :inSetKeyCheckValue
Date&Time       :
Describe        :
*/
int inSetKeyCheckValue(char* szKeyCheckValue)
{
        memset(srSKMRec.szKeyCheckValue, 0x00, sizeof(srSKMRec.szKeyCheckValue));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szKeyCheckValue == NULL || strlen(szKeyCheckValue) <= 0 || strlen(szKeyCheckValue) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetKeyCheckValue() ERROR !!");

                        if (szKeyCheckValue == NULL)
                        {
                                inLogPrintf(AT, "szKeyCheckValue == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szKeyCheckValue length = (%d)", (int)strlen(szKeyCheckValue));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srSKMRec.szKeyCheckValue[0], &szKeyCheckValue[0], strlen(szKeyCheckValue));

        return (VS_SUCCESS);
}
