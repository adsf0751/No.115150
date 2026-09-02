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
#include "SCDT.h"

static  SCDT_REC srSCDTRec;	/* construct SCDT record */
extern  int     ginDebug;       /* Debug使用 extern */

TABLE_GET_SET_TABLE SCDT_FUNC_TABLE[] =
{
	{"szSCDTCardIndex"		,inGetSCDTCardIndex		,inSetSCDTCardIndex		},
	{"szSCDTLowBinRange"		,inGetSCDTLowBinRange		,inSetSCDTLowBinRange		},
	{"szSCDTHighBinRange"		,inGetSCDTHighBinRange		,inSetSCDTHighBinRange		},
	{"szCampaignNumber"		,inGetCampaignNumber		,inSetCampaignNumber		},
	{"szCampaignStartDate"		,inGetCampaignStartDate		,inSetCampaignStartDate		},
	{"szCampaignEndDate"		,inGetCampaignEndDate		,inSetCampaignEndDate		},
	{"szCampaignAmount"		,inGetCampaignAmount		,inSetCampaignAmount		},
	{""},
};

/*
Function        :inLoadSCDTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀SCDT檔案，inSCDTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadSCDTRec(int inSCDTRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆SCDT檔案 */
        char            szSCDTRec[_SIZE_SCDT_REC_ + 1];           /* 暫存, 放各個欄位檔案 */
        char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnSCDTLength = 0;                        /* SCDT總長度 */
        long            lnReadLength;                           /* 記錄每次要從SCDT.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;	                /* inRec記錄讀到第幾筆, i為目前從SCDT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */
       
        /* inLoadSCDTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadSCDTRec(%d) START!!", inSCDTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 判斷傳進來的inSCDTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inSCDTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inSCDTRec < 0:(index = %d) ERROR!!", inSCDTRec);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open SCDT.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_SCDT_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnSCDTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_SCDT_FILE_NAME_);
        
        if (lnSCDTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnSCDTLength + 1);
        uszTemp = malloc(lnSCDTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnSCDTLength + 1);
        memset(uszTemp, 0x00, lnSCDTLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnSCDTLength;

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
         *i為目前從SCDT讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnSCDTLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到SCDT的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnSCDTLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        /* 因為inSCDT_Rec的index從0開始，所以inSCDT_Rec要+1 */
                        if (inRec == (inSCDTRec + 1))
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
         * 如果沒有inSCDTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inSCDTRec + 1) || inSearchResult == -1)
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
        memset(&srSCDTRec, 0x00, sizeof(srSCDTRec));
        /*
         * 以下pattern為存入SCDT_Rec
         * i為SCDT的第幾個字元
         * 存入SCDT_Rec
         */
        i = 0;


        /* 01_卡別索引 */
        /* 初始化 */
        memset(szSCDTRec, 0x00, sizeof(szSCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szSCDTRec[k ++] = uszReadData[i ++];
                if (szSCDTRec[k - 1] == 0x2C	||
		    szSCDTRec[k - 1] == 0x0D	||
		    szSCDTRec[k - 1] == 0x0A	||
		    szSCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnSCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "SCDT unpack ERROR.");
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
        if (szSCDTRec[0] != 0x2C	&&
	    szSCDTRec[0] != 0x0D	&&
	    szSCDTRec[0] != 0x0A	&&
	    szSCDTRec[0] != 0x00)
        {
                memcpy(&srSCDTRec.szSCDTCardIndex[0], &szSCDTRec[0], k - 1);
        }

        /* 02_低卡號範圍 */
        /* 初始化 */
        memset(szSCDTRec, 0x00, sizeof(szSCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szSCDTRec[k ++] = uszReadData[i ++];
                if (szSCDTRec[k - 1] == 0x2C	||
		    szSCDTRec[k - 1] == 0x0D	||
		    szSCDTRec[k - 1] == 0x0A	||
		    szSCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnSCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "SCDT unpack ERROR");
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
        if (szSCDTRec[0] != 0x2C	&&
	    szSCDTRec[0] != 0x0D	&&
	    szSCDTRec[0] != 0x0A	&&
	    szSCDTRec[0] != 0x00)
        {
                memcpy(&srSCDTRec.szSCDTLowBinRange[0], &szSCDTRec[0], k - 1);
        }

        /* 03_高卡號範圍 */
        /* 初始化 */
        memset(szSCDTRec, 0x00, sizeof(szSCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szSCDTRec[k ++] = uszReadData[i ++];
                if (szSCDTRec[k - 1] == 0x2C	||
		    szSCDTRec[k - 1] == 0x0D	||
		    szSCDTRec[k - 1] == 0x0A	||
		    szSCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnSCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "SCDT unpack ERROR");
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
        if (szSCDTRec[0] != 0x2C	&&
	    szSCDTRec[0] != 0x0D	&&
	    szSCDTRec[0] != 0x0A	&&
	    szSCDTRec[0] != 0x00)
        {
                memcpy(&srSCDTRec.szSCDTHighBinRange[0], &szSCDTRec[0], k - 1);
        }

        /* 04_活動代碼 */
        /* 初始化 */
        memset(szSCDTRec, 0x00, sizeof(szSCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szSCDTRec[k ++] = uszReadData[i ++];
                if (szSCDTRec[k - 1] == 0x2C	||
		    szSCDTRec[k - 1] == 0x0D	||
		    szSCDTRec[k - 1] == 0x0A	||
		    szSCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnSCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "SCDT unpack ERROR");
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
        if (szSCDTRec[0] != 0x2C	&&
	    szSCDTRec[0] != 0x0D	&&
	    szSCDTRec[0] != 0x0A	&&
	    szSCDTRec[0] != 0x00)
        {
                memcpy(&srSCDTRec.szCampaignNumber[0], &szSCDTRec[0], k - 1);
        }

        /* 05_活動起始日 */
        /* 初始化 */
        memset(szSCDTRec, 0x00, sizeof(szSCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szSCDTRec[k ++] = uszReadData[i ++];
                if (szSCDTRec[k - 1] == 0x2C	||
		    szSCDTRec[k - 1] == 0x0D	||
		    szSCDTRec[k - 1] == 0x0A	||
		    szSCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnSCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "SCDT unpack ERROR");
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
        if (szSCDTRec[0] != 0x2C	&&
	    szSCDTRec[0] != 0x0D	&&
	    szSCDTRec[0] != 0x0A	&&
	    szSCDTRec[0] != 0x00)
        {
                memcpy(&srSCDTRec.szCampaignStartDate[0], &szSCDTRec[0], k - 1);
        }

        /* 06_活動結束日 */
        /* 初始化 */
        memset(szSCDTRec, 0x00, sizeof(szSCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szSCDTRec[k ++] = uszReadData[i ++];
                if (szSCDTRec[k - 1] == 0x2C	||
		    szSCDTRec[k - 1] == 0x0D	||
		    szSCDTRec[k - 1] == 0x0A	||
		    szSCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnSCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "SCDT unpack ERROR");
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
        if (szSCDTRec[0] != 0x2C	&&
	    szSCDTRec[0] != 0x0D	&&
	    szSCDTRec[0] != 0x0A	&&
	    szSCDTRec[0] != 0x00)
        {
                memcpy(&srSCDTRec.szCampaignEndDate[0], &szSCDTRec[0], k - 1);
        }

        /* 07_活動限額 */
        /* 初始化 */
        memset(szSCDTRec, 0x00, sizeof(szSCDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szSCDTRec[k ++] = uszReadData[i ++];
                if (szSCDTRec[k - 1] == 0x2C	||
		    szSCDTRec[k - 1] == 0x0D	||
		    szSCDTRec[k - 1] == 0x0A	||
		    szSCDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnSCDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "SCDT unpack ERROR");
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
        if (szSCDTRec[0] != 0x2C	&&
	    szSCDTRec[0] != 0x0D	&&
	    szSCDTRec[0] != 0x0A	&&
	    szSCDTRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srSCDTRec.szCampaignAmount[0], &szSCDTRec[0], k - 1);
        }

        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadSCDTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadSCDTRec(%d) END!!", inSCDTRec);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveSCDTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSaveSCDTRec(int inSCDTRec)
{
        unsigned long   uldat_Handle;                           /* FILE Handle */
        unsigned long   ulbak_Handle;                           /* FILE Handle */
        int	        inPackCount = 0;                        /* uszWriteBuff_Record的index */
        int	        inRecIndex = 0;                         /* uszRead_Total_Buff的index */
        int             inSCDT_Total_Rec = 0;                    /* SCDT.dat的總筆數 */
        int             inRetVal;
        int             i = 0, j = 0;
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long	        lnReadLength = 0;                       /* 每次要從SCDT.dat讀多少byte出來 */
        long            lnSCDTLength = 0;                        /* SCDT.dat檔案總長度 */
        unsigned char	*uszRead_Total_Buff;
        unsigned char   *uszWriteBuff_Record, *uszWriteBuff_Org;


        /* inSaveSCDTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveSCDTRec(%d)_START!!", inSCDTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 防呆先刪除SCDT.bak */
        inFILE_Delete((unsigned char *)_SCDT_FILE_NAME_BAK_);

        /* 新建SCDT.bak檔案 */
        inRetVal = inFILE_Create(&ulbak_Handle, (unsigned char *)_SCDT_FILE_NAME_BAK_);

        if (inRetVal != VS_SUCCESS)
        {
                inFILE_Close(&ulbak_Handle);

                return (VS_ERROR);
        }

        /* 開啟原檔案SCDT.dat */
        inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_SCDT_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                /* SCDT.dat開檔失敗 ，不用關檔SCDT.dat */
                /* SCDT.bak仍要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_SCDT_FILE_NAME_BAK_);

                return(VS_ERROR);
        }

        /* 取得SCDT.dat檔案大小 */
        lnSCDTLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *)_SCDT_FILE_NAME_);

        /* 取得檔案大小失敗 */
        if (lnSCDTLength == VS_ERROR)
        {
                /* SCDT.bak和SCDT.dat要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Close(&uldat_Handle);
                inFILE_Delete((unsigned char *)_SCDT_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        uszWriteBuff_Record = malloc(_SIZE_SCDT_REC_ + _SIZE_SCDT_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_SCDT_REC_ + _SIZE_SCDT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原SCDT.dat的每筆Record 初始化 */
        uszWriteBuff_Org = malloc(_SIZE_SCDT_REC_ + _SIZE_SCDT_COMMA_0D0A_);
        memset(uszWriteBuff_Org, 0x00, _SIZE_SCDT_REC_ + _SIZE_SCDT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存SCDT.dat全部資料 */
        uszRead_Total_Buff = malloc(lnSCDTLength + 1);
        memset(uszRead_Total_Buff, 0x00, lnSCDTLength + 1);

        inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

        /* SCDTCardIndex */
        memcpy(&uszWriteBuff_Record[0], &srSCDTRec.szSCDTCardIndex[0], strlen(srSCDTRec.szSCDTCardIndex));
        inPackCount += strlen(srSCDTRec.szSCDTCardIndex);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* SCDTLowBinRange */
        memcpy(&uszWriteBuff_Record[inPackCount], &srSCDTRec.szSCDTLowBinRange[0], strlen(srSCDTRec.szSCDTLowBinRange));
        inPackCount += strlen(srSCDTRec.szSCDTLowBinRange);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* SCDTHighBinRange */
        memcpy(&uszWriteBuff_Record[inPackCount], &srSCDTRec.szSCDTHighBinRange[0], strlen(srSCDTRec.szSCDTHighBinRange));
        inPackCount += strlen(srSCDTRec.szSCDTHighBinRange);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* CampaignNumber */
        memcpy(&uszWriteBuff_Record[inPackCount], &srSCDTRec.szCampaignNumber[0], strlen(srSCDTRec.szCampaignNumber));
        inPackCount += strlen(srSCDTRec.szCampaignNumber);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* CampaignStartDate */
        memcpy(&uszWriteBuff_Record[inPackCount], &srSCDTRec.szCampaignStartDate[0], strlen(srSCDTRec.szCampaignStartDate));
        inPackCount += strlen(srSCDTRec.szCampaignStartDate);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* CampaignEndDate */
        memcpy(&uszWriteBuff_Record[inPackCount], &srSCDTRec.szCampaignEndDate[0], strlen(srSCDTRec.szCampaignEndDate));
        inPackCount += strlen(srSCDTRec.szCampaignEndDate);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* CampaignAmount */
        memcpy(&uszWriteBuff_Record[inPackCount], &srSCDTRec.szCampaignAmount[0], strlen(srSCDTRec.szCampaignAmount));
        inPackCount += strlen(srSCDTRec.szCampaignAmount);

        /* 最後的data不用逗號 */
        /* 補上換行符號 */

        /* 0D是移至行首 */
        uszWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        uszWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;

        /* 讀SCDT.dat檔案存到uszRead_Total_Buff */
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnSCDTLength;

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
                                        inFILE_Delete((unsigned char *)_SCDT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_SCDT_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_SCDT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        /* 防呆 總record數量小於要存取inSCDTRec Return ERROR */
        /* 算總Record數 */
        for (i = 0; i < (lnSCDTLength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (uszRead_Total_Buff[i] == 0x0A)
                        inSCDT_Total_Rec++;
        }

//        /* 防呆 總record數量小於要存取inSCDTRec Return ERROR */
//        if ((inSCDTRec + 1) > inSCDT_Total_Rec)
//        {
//                if (ginDebug == VS_TRUE)
//                {
//                        inLogPrintf(AT, "No data or Index ERROR");
//                }
//
//                /* 關檔 */
//                inFILE_Close(&uldat_Handle);
//                inFILE_Close(&ulbak_Handle);
//                inFILE_Delete((unsigned char *)_SCDT_FILE_NAME_BAK_);
//
//                /* Free pointer */
//                free(uszRead_Total_Buff);
//                free(uszWriteBuff_Record);
//                free(uszWriteBuff_Org);
//
//                return (VS_ERROR);
//        }

        /* 記錄ReadTotalBuffer的陣列位置 */
        inRecIndex = 0;
        /* 用inSCDTRec決定要先存幾筆Record到SCDT.bak，ex:inSCDTRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inSCDTRec; i ++)
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
                                        inFILE_Delete((unsigned char *)_SCDT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_SCDT_FILE_NAME_BAK_);

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

        /* 存組好的該SCDTRecord 到 SCDT.bak */
        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_SCDT_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_SCDT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原SCDT.dat Record 到 SCDT.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
        /* inRecIndex要指到剩下還沒存進bak檔的第一個記憶體位置 */
        inSCDTRec = inSCDTRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < lnSCDTLength; i++)
        {
                if (uszRead_Total_Buff[i] == 0x0A)
                {
                        j++;

                        if (j == inSCDTRec)
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
                /* 接續存原SCDT.dat的Record */
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
                                inFILE_Delete((unsigned char *)_SCDT_FILE_NAME_BAK_);

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
                                inFILE_Delete((unsigned char *)_SCDT_FILE_NAME_BAK_);

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

        /* 刪除原SCDT.dat */
        if (inFILE_Delete((unsigned char *)_SCDT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* 將SCDT.bak改名字為SCDT.dat取代原檔案 */
        if (inFILE_Rename((unsigned char *)_SCDT_FILE_NAME_BAK_, (unsigned char *)_SCDT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* inSaveSCDTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveSCDTRec(%d) END!!", inSCDTRec - 1);
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
Function        :inGetSCDTCardIndex
Date&Time       :
Describe        :
*/
int inGetSCDTCardIndex(char* szSCDTCardIndex)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szSCDTCardIndex == NULL || strlen(srSCDTRec.szSCDTCardIndex) <= 0 || strlen(srSCDTRec.szSCDTCardIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetSCDTCardIndex() ERROR !!");
                        
			if (szSCDTCardIndex == NULL)
                        {
                                inLogPrintf(AT, "szSCDTCardIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSCDTCardIndex length = (%d)", (int)strlen(srSCDTRec.szSCDTCardIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSCDTCardIndex[0], &srSCDTRec.szSCDTCardIndex[0], strlen(srSCDTRec.szSCDTCardIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetSCDTCardIndex
Date&Time       :
Describe        :
*/
int inSetSCDTCardIndex(char* szSCDTCardIndex)
{
        memset(srSCDTRec.szSCDTCardIndex, 0x00, sizeof(srSCDTRec.szSCDTCardIndex));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szSCDTCardIndex == NULL || strlen(szSCDTCardIndex) <= 0 || strlen(szSCDTCardIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetSCDTCardIndex() ERROR !!");
                        if (szSCDTCardIndex == NULL)
                        {
                                inLogPrintf(AT, "szSCDTCardIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSCDTCardIndex length = (%d)", (int)strlen(szSCDTCardIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srSCDTRec.szSCDTCardIndex[0], &szSCDTCardIndex[0], strlen(szSCDTCardIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetSCDTLowBinRange
Date&Time       :
Describe        :
*/
int inGetSCDTLowBinRange(char* szSCDTLowBinRange)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szSCDTLowBinRange == NULL || strlen(srSCDTRec.szSCDTLowBinRange) <= 0 || strlen(srSCDTRec.szSCDTLowBinRange) > 11)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetSCDTLowBinRange() ERROR !!");

                        if (szSCDTLowBinRange == NULL)
                        {
                                inLogPrintf(AT, "szSCDTLowBinRange == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSCDTLowBinRange length = (%d)", (int)strlen(srSCDTRec.szSCDTLowBinRange));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSCDTLowBinRange[0], &srSCDTRec.szSCDTLowBinRange[0], strlen(srSCDTRec.szSCDTLowBinRange));

        return (VS_SUCCESS);
}

/*
Function        :inSetSCDTLowBinRange
Date&Time       :
Describe        :
*/
int inSetSCDTLowBinRange(char* szSCDTLowBinRange)
{
        memset(srSCDTRec.szSCDTLowBinRange, 0x00, sizeof(srSCDTRec.szSCDTLowBinRange));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szSCDTLowBinRange == NULL || strlen(szSCDTLowBinRange) <= 0 || strlen(szSCDTLowBinRange) > 11)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetSCDTLowBinRange() ERROR !!");

                        if (szSCDTLowBinRange == NULL)
                        {
                                inLogPrintf(AT, "szSCDTLowBinRange == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSCDTLowBinRange length = (%d)", (int)strlen(szSCDTLowBinRange));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srSCDTRec.szSCDTLowBinRange[0], &szSCDTLowBinRange[0], strlen(szSCDTLowBinRange));

        return (VS_SUCCESS);
}

/*
Function        :inGetSCDTHighBinRange
Date&Time       :
Describe        :
*/
int inGetSCDTHighBinRange(char* szSCDTHighBinRange)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szSCDTHighBinRange == NULL || strlen(srSCDTRec.szSCDTHighBinRange) <= 0 || strlen(srSCDTRec.szSCDTHighBinRange) > 11)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetSCDTHighBinRange() ERROR !!");

                        if (szSCDTHighBinRange == NULL)
                        {
                                inLogPrintf(AT, "szSCDTHighBinRange == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSCDTHighBinRange length = (%d)", (int)strlen(srSCDTRec.szSCDTHighBinRange));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSCDTHighBinRange[0], &srSCDTRec.szSCDTHighBinRange[0], strlen(srSCDTRec.szSCDTHighBinRange));

        return (VS_SUCCESS);
}

/*
Function        :inSetSCDTHighBinRange
Date&Time       :
Describe        :
*/
int inSetSCDTHighBinRange(char* szSCDTHighBinRange)
{
        memset(srSCDTRec.szSCDTHighBinRange, 0x00, sizeof(srSCDTRec.szSCDTHighBinRange));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szSCDTHighBinRange == NULL || strlen(szSCDTHighBinRange) <= 0 || strlen(szSCDTHighBinRange) > 11)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetSCDTHighBinRange() ERROR !!");

                        if (szSCDTHighBinRange == NULL)
                        {
                                inLogPrintf(AT, "szSCDTHighBinRange == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSCDTHighBinRange length = (%d)", (int)strlen(szSCDTHighBinRange));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srSCDTRec.szSCDTHighBinRange[0], &szSCDTHighBinRange[0], strlen(szSCDTHighBinRange));

        return (VS_SUCCESS);
}

/*
Function        :inGetCampaignNumber
Date&Time       :
Describe        :
*/
int inGetCampaignNumber(char* szCampaignNumber)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szCampaignNumber == NULL || strlen(srSCDTRec.szCampaignNumber) <= 0 || strlen(srSCDTRec.szCampaignNumber) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetHostCampaignNumber() ERROR !!");

                        if (szCampaignNumber == NULL)
                        {
                                inLogPrintf(AT, "szCampaignNumber == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCampaignNumber length = (%d)", (int)strlen(srSCDTRec.szCampaignNumber));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCampaignNumber[0], &srSCDTRec.szCampaignNumber[0], strlen(srSCDTRec.szCampaignNumber));

        return (VS_SUCCESS);
}

/*
Function        :inSetCampaignNumber
Date&Time       :
Describe        :
*/
int inSetCampaignNumber(char* szCampaignNumber)
{
        memset(srSCDTRec.szCampaignNumber, 0x00, sizeof(srSCDTRec.szCampaignNumber));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCampaignNumber == NULL || strlen(szCampaignNumber) <= 0 || strlen(szCampaignNumber) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetHostCampaignNumber() ERROR !!");

                        if (szCampaignNumber == NULL)
                        {
                                inLogPrintf(AT, "szCampaignNumber == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCampaignNumber length = (%d)", (int)strlen(szCampaignNumber));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srSCDTRec.szCampaignNumber[0], &szCampaignNumber[0], strlen(szCampaignNumber));

        return (VS_SUCCESS);
}

/*
Function        :inGetCampaignStartDate
Date&Time       :
Describe        :
*/
int inGetCampaignStartDate(char* szCampaignStartDate)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szCampaignStartDate == NULL || strlen(srSCDTRec.szCampaignStartDate) <= 0 || strlen(srSCDTRec.szCampaignStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetCampaignStartDate() ERROR !!");

                        if (szCampaignStartDate == NULL)
                        {
                                inLogPrintf(AT, "szCampaignStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCampaignStartDate length = (%d)", (int)strlen(srSCDTRec.szCampaignStartDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCampaignStartDate[0], &srSCDTRec.szCampaignStartDate[0], strlen(srSCDTRec.szCampaignStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetCampaignStartDate
Date&Time       :
Describe        :
*/
int inSetCampaignStartDate(char* szCampaignStartDate)
{
        memset(srSCDTRec.szCampaignStartDate, 0x00, sizeof(srSCDTRec.szCampaignStartDate));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCampaignStartDate == NULL || strlen(szCampaignStartDate) <= 0 || strlen(szCampaignStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCampaignStartDate() ERROR !!");

                        if (szCampaignStartDate == NULL)
                        {
                                inLogPrintf(AT, "szCampaignStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCampaignStartDate length = (%d)", (int)strlen(szCampaignStartDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srSCDTRec.szCampaignStartDate[0], &szCampaignStartDate[0], strlen(szCampaignStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetCampaignEndDate
Date&Time       :
Describe        :
*/
int inGetCampaignEndDate(char* szCampaignEndDate)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szCampaignEndDate == NULL || strlen(srSCDTRec.szCampaignEndDate) <= 0 || strlen(srSCDTRec.szCampaignEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetCampaignEndDate() ERROR !!");

                        if (szCampaignEndDate == NULL)
                        {
                                inLogPrintf(AT, "szCampaignEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCampaignEndDate length = (%d)", (int)strlen(srSCDTRec.szCampaignEndDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCampaignEndDate[0], &srSCDTRec.szCampaignEndDate[0], strlen(srSCDTRec.szCampaignEndDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetCampaignEndDate
Date&Time       :
Describe        :
*/
int inSetCampaignEndDate(char* szCampaignEndDate)
{
        memset(srSCDTRec.szCampaignEndDate, 0x00, sizeof(srSCDTRec.szCampaignEndDate));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCampaignEndDate == NULL || strlen(szCampaignEndDate) <= 0 || strlen(szCampaignEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCampaignEndDate() ERROR !!");

                        if (szCampaignEndDate == NULL)
                        {
                                inLogPrintf(AT, "szCampaignEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCampaignEndDate length = (%d)", (int)strlen(szCampaignEndDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srSCDTRec.szCampaignEndDate[0], &szCampaignEndDate[0], strlen(szCampaignEndDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetCampaignAmount
Date&Time       :
Describe        :
*/
int inGetCampaignAmount(char* szCampaignAmount)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szCampaignAmount == NULL || strlen(srSCDTRec.szCampaignAmount) <= 0 || strlen(srSCDTRec.szCampaignAmount) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetCampaignAmount() ERROR !!");

                        if (szCampaignAmount == NULL)
                        {
                                inLogPrintf(AT, "szCampaignAmount == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCampaignAmount length = (%d)", (int)strlen(srSCDTRec.szCampaignAmount));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCampaignAmount[0], &srSCDTRec.szCampaignAmount[0], strlen(srSCDTRec.szCampaignAmount));

        return (VS_SUCCESS);
}

/*
Function        :inSetCampaignAmount
Date&Time       :
Describe        :
*/
int inSetCampaignAmount(char* szCampaignAmount)
{
        memset(srSCDTRec.szCampaignAmount, 0x00, sizeof(srSCDTRec.szCampaignAmount));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCampaignAmount == NULL || strlen(szCampaignAmount) <= 0 || strlen(szCampaignAmount) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCampaignAmount() ERROR !!");

                        if (szCampaignAmount == NULL)
                        {
                                inLogPrintf(AT, "szCampaignAmount == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCampaignAmount length = (%d)", (int)strlen(szCampaignAmount));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srSCDTRec.szCampaignAmount[0], &szCampaignAmount[0], strlen(szCampaignAmount));

        return (VS_SUCCESS);
}

/*
Function        :inSCDT_Edit_SCDT_Table
Date&Time       :2022/6/2 上午 11:11
Describe        :
*/
int inSCDT_Edit_SCDT_Table(void)
{
	int		inRetVal = VS_ERROR;
	int		inRecordCnt = 0;
	int		inFinalTimeout = 0;
	char		szKey;
	DISPLAY_OBJECT  srDispObj;
	
	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	srDispObj.inY = _LINE_8_8_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 2;
	srDispObj.inColor = _COLOR_BLACK_;
	
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
	inDISP_ChineseFont_Color("是否更改SCDT", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
	inDISP_ChineseFont_Color("列印請按0 更改請按Enter", _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);
	inDISP_ChineseFont_Color("跳過請按取消鍵", _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_BLACK_, _DISP_LEFT_);
	while (1)
	{
		szKey = uszKBD_GetKey(_EDC_TIMEOUT_);
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
	inLoadSCDTRec(inRecordCnt);
	
	inFunc_Edit_Table_Tag(SCDT_FUNC_TABLE);
	inSaveSCDTRec(inRecordCnt);
	
	return	(VS_SUCCESS);
}