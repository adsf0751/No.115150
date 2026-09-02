#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <unistd.h>
#include <sqlite3.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/AllStruct.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "File.h"
#include "Function.h"
#include "Sqlite.h"
#include "EDC_Para_Table_Func.h"
#include "Utility.h"
#include "HDPT.h"
#include "HDT.h"

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))

static  HDPT_REC srHDPTRec;	/* construct HDPT record */
extern  int     ginDebug;  	/* Debug使用 extern */
extern	int	ginFindRunTime;
extern	char	gszParamDBPath[];

SQLITE_TAG_TABLE TABLE_HDPT_TAG[] = 
{
	{"inTableID"			,"INTEGER"	,"PRIMARY KEY"	,""},	/* Table ID Primary key, sqlite table專用避免PRIMARY KEY重複 */
	{"szInvoiceNum"			,"BLOB"		,""		,""},	/* Invoice Number */
	{"szBatchNum"			,"BLOB"		,""		,""},	/* Batch Number */
	{"szSTANNum"			,"BLOB"		,""		,""},	/* STAN Number */
	{"szReversalCnt"		,"BLOB"		,""		,""},	/* Reversal Count */
	{"szTRTFileName"		,"BLOB"		,""		,""},	/* szTRTFileName */
	{"szMustSettleBit"		,"BLOB"		,""		,""},	/* 是否結帳的flag，預設值為N */
	{"szSendReversalBit"		,"BLOB"		,""		,""},	/* 預設值為N */
	{"szCLS_SettleBit"		,"BLOB"		,""		,""},	/* 看是否要續傳批次 */
	{"szTicket_InvNum"		,"BLOB"		,""		,""},	/* 紀錄電票的invoiceNum，簽單的調閱編號用szInvoiceNum的 */
	{"szBatchNumLimit"		,"BLOB"		,""		,""},	/* 單筆Batch的最大數量 */
	{""},
};

TABLE_GET_SET_TABLE HDPT_FUNC_TABLE[] =
{
	{"szInvoiceNum"			,inGetInvoiceNum		,inSetInvoiceNum		},
	{"szBatchNum"			,inGetBatchNum			,inSetBatchNum			},
	{"szSTANNum"			,inGetSTANNum			,inSetSTANNum			},
	{"szReversalCnt"		,inGetReversalCnt		,inSetReversalCnt		},
	{"szTRTFileName"		,inGetTRTFileName		,inSetTRTFileName		},
	{"szMustSettleBit"		,inGetMustSettleBit		,inSetMustSettleBit		},
	{"szSendReversalBit"		,inGetSendReversalBit		,inSetSendReversalBit		},
	{"szCLS_SettleBit"		,inGetCLS_SettleBit		,inSetCLS_SettleBit		},
	{"szTicket_InvNum"		,inGetTicket_InvNum		,inSetTicket_InvNum		},
	{"szBatchNumLimit"		,inGetBatchNumLimit		,inSetBatchNumLimit		},
	{""},
};

/*
Function        :inLoadHDPTRec
Date&Time       :2019/4/24 下午 2:06
Describe        :優化存參數部份
*/
int inLoadHDPTRec(int inHDPTRec)
{
	int	inRetVal = VS_ERROR;
	
	/* 加速用 */
	if (_PARAMETER_SAVE_WAY_DAT_ == _PARAMETER_SAVE_WAY_DAT_SQLITE_)
	{
		inRetVal = inLoadHDPTRec_SQLite(inHDPTRec);
	}
	else
	{
		inRetVal = inLoadHDPTRec_CTOS(inHDPTRec);
	}

        return (inRetVal);
}

/*
Function        :inLoadHDPTRec_CTOS
Date&Time       :2015/8/31 下午 2:00
Describe        :讀HDPT檔案，inHDPT_Rec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadHDPTRec_CTOS(int inHDPTRec)
{
        unsigned long   ulFile_Handle;				/* File Handle */
        unsigned char   *uszReadData;				/* 放抓到的record */
        unsigned char   *uszTemp;				/* 暫存，放整筆HDPT檔案 */
        char            szHDPTRec[_SIZE_HDPT_REC_ + 1];		/* 暫存, 放各個欄位檔案 */
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];	/* debug message */
        long            lnHDPTLength = 0;       		/* HDPT總長度 */
        long            lnReadLength;           		/* 記錄每次要從HDPT.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0; 		/* inRec記錄讀到第幾筆, i為目前從HDPT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;    		/* 判斷有沒有讀到0x0D 0x0A的Flag */

        /* inLoadHDPTRec_CTOS()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadHDPTRec_CTOS(%d) START!!", inHDPTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 判斷傳進來的inHDPTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inHDPTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inHDPTRec < 0:(index = %d) ERROR!!", inHDPTRec);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

	/* rwxr--r--*/
	inFunc_Data_Chmod("744", _HDPT_FILE_NAME_, _FS_DATA_PATH_);
	
        /*
         * open HDPT.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_HDPT_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnHDPTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_HDPT_FILE_NAME_);

	if (lnHDPTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnHDPTLength + 1);
        uszTemp = malloc(lnHDPTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnHDPTLength + 1);
        memset(uszTemp, 0x00, lnHDPTLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnHDPTLength;

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
         *i為目前從HDPT讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnHDPTLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到HDPT的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
			inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnHDPTLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        /* 因為inHDPT_Rec的index從0開始，所以inHDPT_Rec要+1 */
                        if (inRec == (inHDPTRec + 1))
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
         * 如果沒有inHDPTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inHDPTRec + 1) || inSearchResult == -1)
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
        memset(&srHDPTRec, 0x00, sizeof(srHDPTRec));
        /*
         * 以下pattern為存入HDPT_Rec
         * i為HDPT的第幾個字元
         * 存入HDPT_Rec
         */
        i = 0;


        /* 01_Invoice Number */
        /* 初始化 */
        memset(szHDPTRec, 0x00, sizeof(szHDPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDPTRec[k++] = uszReadData[i++];
                if (szHDPTRec[k - 1] == 0x2C	||
		    szHDPTRec[k - 1] == 0x0D	||
		    szHDPTRec[k - 1] == 0x0A	||
		    szHDPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "HDPT unpack ERROR");
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
        if (szHDPTRec[0] != 0x2C	&&
	    szHDPTRec[0] != 0x0D	&&
	    szHDPTRec[0] != 0x0A	&&
	    szHDPTRec[0] != 0x00)
        {
                memcpy(&srHDPTRec.szInvoiceNum[0], &szHDPTRec[0], k - 1);
        }

        /* 02_Batch Number */
        /* 初始化 */
        memset(szHDPTRec, 0x00, sizeof(szHDPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDPTRec[k++] = uszReadData[i++];
                if (szHDPTRec[k - 1] == 0x2C	||
		    szHDPTRec[k - 1] == 0x0D	||
		    szHDPTRec[k - 1] == 0x0A	||
		    szHDPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "HDPT unpack ERROR");
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
        if (szHDPTRec[0] != 0x2C	&&
	    szHDPTRec[0] != 0x0D	&&
	    szHDPTRec[0] != 0x0A	&&
	    szHDPTRec[0] != 0x00)
        {
                memcpy(&srHDPTRec.szBatchNum[0], &szHDPTRec[0], k - 1);
        }

        /* 03_STAN Number */
        /* 初始化 */
        memset(szHDPTRec, 0x00, sizeof(szHDPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDPTRec[k++] = uszReadData[i++];
                if (szHDPTRec[k - 1] == 0x2C	||
		    szHDPTRec[k - 1] == 0x0D	||
		    szHDPTRec[k - 1] == 0x0A	||
		    szHDPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "HDPT unpack ERROR");
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
        if (szHDPTRec[0] != 0x2C	&&
	    szHDPTRec[0] != 0x0D	&&
	    szHDPTRec[0] != 0x0A	&&
	    szHDPTRec[0] != 0x00)
        {
                memcpy(&srHDPTRec.szSTANNum[0], &szHDPTRec[0], k - 1);
        }

        /* 04_Reversal Count */
        /* 初始化 */
        memset(szHDPTRec, 0x00, sizeof(szHDPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDPTRec[k++] = uszReadData[i++];
                if (szHDPTRec[k - 1] == 0x2C	||
		    szHDPTRec[k - 1] == 0x0D	||
		    szHDPTRec[k - 1] == 0x0A	||
		    szHDPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "HDPT unpack ERROR");
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
        if (szHDPTRec[0] != 0x2C	&&
	    szHDPTRec[0] != 0x0D	&&
	    szHDPTRec[0] != 0x0A	&&
	    szHDPTRec[0] != 0x00)
        {
                memcpy(&srHDPTRec.szReversalCnt[0], &szHDPTRec[0], k - 1);
        }

        /* 05_TRTFILENAME */
        /* 初始化 */
        memset(szHDPTRec, 0x00, sizeof(szHDPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDPTRec[k++] = uszReadData[i++];
                if (szHDPTRec[k - 1] == 0x2C	||
		    szHDPTRec[k - 1] == 0x0D	||
		    szHDPTRec[k - 1] == 0x0A	||
		    szHDPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "HDPT unpack ERROR");
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
        if (szHDPTRec[0] != 0x2C	&&
	    szHDPTRec[0] != 0x0D	&&
	    szHDPTRec[0] != 0x0A	&&
	    szHDPTRec[0] != 0x00)
        {
                memcpy(&srHDPTRec.szTRTFileName[0], &szHDPTRec[0], k - 1);
        }

        /* 06_szMustSettleBit */
        /* 初始化 */
        memset(szHDPTRec, 0x00, sizeof(szHDPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDPTRec[k++] = uszReadData[i++];
                if (szHDPTRec[k - 1] == 0x2C	||
		    szHDPTRec[k - 1] == 0x0D	||
		    szHDPTRec[k - 1] == 0x0A	||
		    szHDPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "HDPT unpack error");
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
        if (szHDPTRec[0] != 0x2C	&&
	    szHDPTRec[0] != 0x0D	&&
	    szHDPTRec[0] != 0x0A	&&
	    szHDPTRec[0] != 0x00)
        {
                memcpy(&srHDPTRec.szMustSettleBit[0], &szHDPTRec[0], k - 1);
        }

        /* 07_szSendReversalBit */
        /* 初始化 */
        memset(szHDPTRec, 0x00, sizeof(szHDPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDPTRec[k++] = uszReadData[i++];
                if (szHDPTRec[k - 1] == 0x2C	||
		    szHDPTRec[k - 1] == 0x0D	||
		    szHDPTRec[k - 1] == 0x0A	||
		    szHDPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "HDPT unpack ERROR");
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
        if (szHDPTRec[0] != 0x2C	&&
	    szHDPTRec[0] != 0x0D	&&
	    szHDPTRec[0] != 0x0A	&&
	    szHDPTRec[0] != 0x00)
        {
                memcpy(&srHDPTRec.szSendReversalBit[0], &szHDPTRec[0], k - 1);
        }
	
	/* 08_szCLS_SettleBit */
        /* 初始化 */
        memset(szHDPTRec, 0x00, sizeof(szHDPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDPTRec[k++] = uszReadData[i++];
                if (szHDPTRec[k - 1] == 0x2C	||
		    szHDPTRec[k - 1] == 0x0D	||
		    szHDPTRec[k - 1] == 0x0A	||
		    szHDPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "HDPT unpack ERROR");
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
        if (szHDPTRec[0] != 0x2C	&&
	    szHDPTRec[0] != 0x0D	&&
	    szHDPTRec[0] != 0x0A	&&
	    szHDPTRec[0] != 0x00)
        {
                memcpy(&srHDPTRec.szCLS_SettleBit[0], &szHDPTRec[0], k - 1);
        }
	
	/* 09_szTicket_InvNum */
        /* 初始化 */
        memset(szHDPTRec, 0x00, sizeof(szHDPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDPTRec[k++] = uszReadData[i++];
                if (szHDPTRec[k - 1] == 0x2C	||
		    szHDPTRec[k - 1] == 0x0D	||
		    szHDPTRec[k - 1] == 0x0A	||
		    szHDPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "HDPT unpack ERROR");
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
        if (szHDPTRec[0] != 0x2C	&&
	    szHDPTRec[0] != 0x0D	&&
	    szHDPTRec[0] != 0x0A	&&
	    szHDPTRec[0] != 0x00)
        {
                memcpy(&srHDPTRec.szTicket_InvNum[0], &szHDPTRec[0], k - 1);
        }
	
	/* 10_szBatchNumLimit */
        /* 初始化 */
        memset(szHDPTRec, 0x00, sizeof(szHDPTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szHDPTRec[k++] = uszReadData[i++];
                if (szHDPTRec[k - 1] == 0x2C	||
		    szHDPTRec[k - 1] == 0x0D	||
		    szHDPTRec[k - 1] == 0x0A	||
		    szHDPTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnHDPTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "HDPT unpack ERROR");
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
        if (szHDPTRec[0] != 0x2C	&&
	    szHDPTRec[0] != 0x0D	&&
	    szHDPTRec[0] != 0x0A	&&
	    szHDPTRec[0] != 0x00) /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srHDPTRec.szBatchNumLimit[0], &szHDPTRec[0], k - 1);
        }

        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadHDPTRec_CTOS() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadHDPTRec_CTOS(%d) END!!", inHDPTRec);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inLoadHDPTRec_SQLite
Date&Time       :2019/4/24 下午 2:11
Describe        :讀HDPT檔案，inHDPT_Rec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadHDPTRec_SQLite(int inHDPTRec)
{
	int			inRetVal = VS_ERROR;
	char			szDebugMsg[100 + 1] = {0};
	SQLITE_ALL_TABLE	srAll = {0};

        /* inLoadHDPTRec_SQLite()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                sprintf(szDebugMsg, "inLoadHDPTRec_SQLite(%d) START!!", inHDPTRec);
                inLogPrintf(AT, szDebugMsg);
        }

        /* 判斷傳進來的inHDPTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inHDPTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                        sprintf(szDebugMsg, "inHDPTRec < 0:(index = %d) ERROR!!", inHDPTRec);
                        inLogPrintf(AT, szDebugMsg);
                }

                return (VS_ERROR);
        }
	
	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inHDPT_Table_Link_HDPTRec(&srAll, _LS_READ_);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Link 失敗");
			inLogPrintf(AT, szDebugMsg);
		}
		inUtility_StoreTraceLog_OneStep("Table Link 失敗");
		
		return (VS_ERROR);
	}
	
	inRetVal = inSqlite_Get_Table_ByRecordID_All(gszParamDBPath, _HDPT_TABLE_NAME_, inHDPTRec, &srAll);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "db:%s,table:%s,Rec:%d get data failed", gszParamDBPath, _HDPT_TABLE_NAME_, inHDPTRec);
			inLogPrintf(AT, szDebugMsg);
		}
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "db:%s,table:%s,Rec:%d get data failed", gszParamDBPath, _HDPT_TABLE_NAME_, inHDPTRec);
		inUtility_StoreTraceLog_OneStep(szDebugMsg);
		
		return (VS_ERROR);
	}

        /* inLoadHDPTRec_SQLite() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                sprintf(szDebugMsg, "inLoadHDPTRec_SQLite(%d) END!!", inHDPTRec);
                inLogPrintf(AT, szDebugMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveHDPTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :寫入HDPT.dat，inHDPTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inSaveHDPTRec(int inHDPTRec)
{
	int	inRetVal = VS_ERROR;
	
	/* 加速用 */
	if (_PARAMETER_SAVE_WAY_DAT_ == _PARAMETER_SAVE_WAY_DAT_SQLITE_)
	{
		inRetVal = inSaveHDPTRec_SQLite(inHDPTRec);
	}
	else if (_PARAMETER_SAVE_WAY_DAT_ == _PARAMETER_SAVE_WAY_DAT_LINUX_)
	{
		inRetVal = inSaveHDPTRec_Linux(inHDPTRec);
	}
	else
	{
		inRetVal = inSaveHDPTRec_CTOS(inHDPTRec);
	}
	
	if (inRetVal != VS_SUCCESS)
	{
		inFunc_EDCLock(AT);
	}

        return (inRetVal);
}

/*
Function        :inSaveHDPTRec_Linux
Date&Time       :2019/1/17 上午 11:10
Describe        :寫入HDPT.dat，inHDPTRec是要讀哪一筆的紀錄，第一筆為0
 *		 改用原生Linux Function
*/
int inSaveHDPTRec_Linux(int inHDPTRec)
{
        int     	inPackCount = 0;        		/* uszWriteBuff_Record的index */
        int     	inRecIndex = 0;         		/* uszRead_Total_Buff的index */
        int     	inHDPT_Total_Rec = 0;    		/* HDPT.dat的總筆數 */
        int     	inRetVal;
        int     	i = 0, j = 0;
	int		inDatFd = 0;
	int		inBakFd = 0;
	int		inHDPTLength = 0;         		/* HDPT.dat檔案總長度 */
	int		inReadCnt = 0;
	int		inNowReadOffset = 0;
	char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];	/* debug message */
	char		*szRead_Total_Buff;
	char		*szWriteBuff_Record, *szWriteBuff_Org;
        long    	lnReadLength = 0;       		/* 每次要從HDPT.dat讀多少byte出來 */

        /* inSaveHDPTRec_Linux()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveHDPTRec_Linux(%d) START!!", inHDPTRec);
                inLogPrintf(AT, szErrorMsg);
        }
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
        /* 防呆先刪除HDPT.bak  */
	inFile_Linux_Delete_In_Fs_Data(_HDPT_FILE_NAME_BAK_);

        /* 新建HDPT.bak檔案 */
        inRetVal = inFile_Linux_Create_In_Fs_Data(&inBakFd, _HDPT_FILE_NAME_BAK_);

        if (inRetVal != VS_SUCCESS)
        {
                inFile_Linux_Close(inBakFd);

                return (VS_ERROR);
        }

        /* 開啟原檔案HDPT.dat */
        inRetVal = inFile_Linux_Open_In_Fs_Data(&inDatFd, _HDPT_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                /* HDPT.dat開檔失敗 ，不用關檔HDPT.dat */
                /* HDPT.bak仍要關並刪除 */
                inFile_Linux_Close(inBakFd);
                inFile_Linux_Delete_In_Fs_Data(_HDPT_FILE_NAME_BAK_);

                return(VS_ERROR);
        }

        /* 取得HDPT.dat檔案大小 */
	inRetVal = inFile_Linux_Get_FileSize_By_LSeek(inDatFd, &inHDPTLength);

        /* 取得檔案大小失敗 */
        if (inRetVal == VS_ERROR)
        {
                /* HDPT.bak和HDPT.dat要關並刪除 */
                inFile_Linux_Close(inBakFd);
                inFile_Linux_Close(inDatFd);
                inFile_Linux_Delete_In_Fs_Data(_HDPT_FILE_NAME_BAK_);

                return (VS_ERROR);
        }
	
        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        szWriteBuff_Record = malloc(_SIZE_HDPT_REC_ + _SIZE_HDPT_COMMA_0D0A_);
        memset(szWriteBuff_Record, 0x00, _SIZE_HDPT_REC_ + _SIZE_HDPT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原HDPT.dat的每筆Record 初始化 */
        szWriteBuff_Org = malloc(_SIZE_HDPT_REC_ + _SIZE_HDPT_COMMA_0D0A_);
        memset(szWriteBuff_Org, 0x00, _SIZE_HDPT_REC_ + _SIZE_HDPT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存HDPT.dat全部資料 */
        szRead_Total_Buff = malloc(inHDPTLength + 1);
        memset(szRead_Total_Buff, 0x00, inHDPTLength + 1);
	
        inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

	/* InvoiceNum */
        memcpy(&szWriteBuff_Record[0], &srHDPTRec.szInvoiceNum[0], strlen(srHDPTRec.szInvoiceNum));
        inPackCount += strlen(srHDPTRec.szInvoiceNum);
        szWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* BatchNum */
        memcpy(&szWriteBuff_Record[inPackCount], &srHDPTRec.szBatchNum[0], strlen(srHDPTRec.szBatchNum));
        inPackCount += strlen(srHDPTRec.szBatchNum);
        szWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* STANNum */
        memcpy(&szWriteBuff_Record[inPackCount], &srHDPTRec.szSTANNum[0], strlen(srHDPTRec.szSTANNum));
        inPackCount += strlen(srHDPTRec.szSTANNum);
	szWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* ReversalCnt */
        memcpy(&szWriteBuff_Record[inPackCount], &srHDPTRec.szReversalCnt[0], strlen(srHDPTRec.szReversalCnt));
        inPackCount += strlen(srHDPTRec.szReversalCnt);
        szWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* TRTFileName */
        memcpy(&szWriteBuff_Record[inPackCount], &srHDPTRec.szTRTFileName[0], strlen(srHDPTRec.szTRTFileName));
	inPackCount += strlen(srHDPTRec.szTRTFileName);
        szWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* MustSettleBit */
	memcpy(&szWriteBuff_Record[inPackCount], &srHDPTRec.szMustSettleBit[0], strlen(srHDPTRec.szMustSettleBit));
        inPackCount += strlen(srHDPTRec.szMustSettleBit);
        szWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* SendReversalBit */
	memcpy(&szWriteBuff_Record[inPackCount], &srHDPTRec.szSendReversalBit[0], strlen(srHDPTRec.szSendReversalBit));
        inPackCount += strlen(srHDPTRec.szSendReversalBit);
        szWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* CLS_SettleBit */
	memcpy(&szWriteBuff_Record[inPackCount], &srHDPTRec.szCLS_SettleBit[0], strlen(srHDPTRec.szCLS_SettleBit));
        inPackCount += strlen(srHDPTRec.szCLS_SettleBit);
        szWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szTicket_InvNum */
	memcpy(&szWriteBuff_Record[inPackCount], &srHDPTRec.szTicket_InvNum[0], strlen(srHDPTRec.szTicket_InvNum));
        inPackCount += strlen(srHDPTRec.szTicket_InvNum);
        szWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* szBatchNumLimit */
        memcpy(&szWriteBuff_Record[inPackCount], &srHDPTRec.szBatchNumLimit[0], strlen(srHDPTRec.szBatchNumLimit));
        inPackCount += strlen(srHDPTRec.szBatchNumLimit);

        /* 最後的data不用逗號 */
	/* 補上換行符號 */

        /* 0D是移至行首 */
        szWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        szWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;
	
        /* 讀HDPT.dat檔案存到uszRead_Total_Buff */
        if (inFile_Linux_Seek(inDatFd, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = inHDPTLength;
		inNowReadOffset = 0;
		
                while(1)
                {
			inReadCnt = lnReadLength;
			
			/* 就只讀剩餘長度 */
			if (inFile_Linux_Read(inDatFd, (char*)&szRead_Total_Buff[inNowReadOffset], &inReadCnt) == VS_SUCCESS)
			{
				lnReadLength -= inReadCnt;
				inNowReadOffset += inReadCnt;
			}
			/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFile_Linux_Close(inDatFd);
				inFile_Linux_Close(inBakFd);
				inFile_Linux_Delete_In_Fs_Data(_HDPT_FILE_NAME_BAK_);

				/* Free pointer */
				free(szRead_Total_Buff);
				free(szWriteBuff_Record);
				free(szWriteBuff_Org);

				return (VS_ERROR);
			}
                        
			if (lnReadLength == 0)
				break;
                } /* end for loop */
        }
        else
        {
                /* Close檔案 */
                inFile_Linux_Close(inDatFd);
                inFile_Linux_Close(inBakFd);
                inFile_Linux_Delete_In_Fs_Data(_HDPT_FILE_NAME_BAK_);

                /* Free pointer */
                free(szRead_Total_Buff);
                free(szWriteBuff_Record);
                free(szWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }
	
        /* 防呆 總record數量小於要存取inHDPTRec Return ERROR */
        /* 算總Record數 */
        for (i = 0; i < (inHDPTLength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (szRead_Total_Buff[i] == 0x0A)
                        inHDPT_Total_Rec++;
        }

        /* 防呆 總record數量小於要存取inHDPTRec Return ERROR */
        if ((inHDPTRec + 1) > inHDPT_Total_Rec)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "No data or Index ERROR");
                }

                /* 關檔 */
                inFile_Linux_Close(inDatFd);
                inFile_Linux_Close(inBakFd);
                inFile_Linux_Delete_In_Fs_Data(_HDPT_FILE_NAME_BAK_);

                /* Free pointer */
                free(szRead_Total_Buff);
                free(szWriteBuff_Record);
                free(szWriteBuff_Org);

                return (VS_ERROR);
        }

        /* 記錄ReadTotalBuffer的陣列位置 */
        inRecIndex = 0;
        /* 用inHDPTRec決定要先存幾筆Record到HDPT.bak，ex:inHDPTRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inHDPTRec; i ++)
        {
                j = 0;

                while (1)
                {
                        /* 存Record */
                        szWriteBuff_Org[j] = szRead_Total_Buff[inRecIndex];
                        j++;

                        /* 每讀完一筆record，就把baffer中record的資料寫入bak */
                        if (szRead_Total_Buff[inRecIndex] == 0x0A)
                        {
                                /* 為防止寫入位置錯誤，先移動到bak的檔案結尾 */
                                inRetVal = inFile_Linux_Seek(inBakFd, 0, _SEEK_END_);

                                if (inRetVal != VS_SUCCESS)
                                {
                                        /* 關檔 */
                                        inFile_Linux_Close(inDatFd);
                                        inFile_Linux_Close(inBakFd);
                                        inFile_Linux_Delete_In_Fs_Data(_HDPT_FILE_NAME_BAK_);

                                        /* Free pointer */
                                        free(szWriteBuff_Record);
                                        free(szWriteBuff_Org);
                                        free(szRead_Total_Buff);

                                        return (VS_ERROR);
                                }

                                /* 寫入bak檔案 */
                                inRetVal = inFile_Linux_Write(inBakFd, szWriteBuff_Org, &j);

                                if (inRetVal != VS_SUCCESS)
                                {
                                        /* 關檔 */
                                        inFile_Linux_Close(inDatFd);
                                        inFile_Linux_Close(inBakFd);
                                        inFile_Linux_Delete_In_Fs_Data(_HDPT_FILE_NAME_BAK_);

                                        /* Free pointer */
                                        free(szWriteBuff_Record);
                                        free(szWriteBuff_Org);
                                        free(szRead_Total_Buff);

                                        return (VS_ERROR);
                                }

                                /* 清空buffer為下一筆寫入做準備 */
                                memset(szWriteBuff_Org, 0x00, sizeof(szWriteBuff_Org));
                                inRecIndex++;

                                break;
                        }

                        /* 讀下一個字元 */
                        inRecIndex++;
                }
        }

        /* 存組好的該HDPTRecord 到 HDPT.bak */
        inRetVal = inFile_Linux_Seek(inBakFd, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFile_Linux_Close(inDatFd);
                inFile_Linux_Close(inBakFd);
                inFile_Linux_Delete_In_Fs_Data(_HDPT_FILE_NAME_BAK_);

                /* Free pointer */
                free(szWriteBuff_Record);
                free(szWriteBuff_Org);
                free(szRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 將組好的record寫入bak檔 */
        inRetVal = inFile_Linux_Write(inBakFd, szWriteBuff_Record, &inPackCount);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFile_Linux_Close(inDatFd);
                inFile_Linux_Close(inBakFd);
                inFile_Linux_Delete_In_Fs_Data(_HDPT_FILE_NAME_BAK_);

                /* Free pointer */
                free(szWriteBuff_Record);
                free(szWriteBuff_Org);
                free(szRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原HDPT.dat Record 到 HDPT.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
        /* inRecIndex要指到剩下還沒存進bak檔的第一個記憶體位置 */
        inHDPTRec = inHDPTRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < inHDPTLength; i++)
        {
                if (szRead_Total_Buff[i] == 0x0A)
                {
                        j++;

                        if (j == inHDPTRec)
                        {
                                inRecIndex = i + 1;
                                break;
                        }
                }
        }
	
        i = 0;
        memset(szWriteBuff_Org, 0x00, sizeof(szWriteBuff_Org));

        while (1)
        {
                /* 接續存原HDPT.dat的Record */
                szWriteBuff_Org[i] = szRead_Total_Buff[inRecIndex];
                i++;

                if (szRead_Total_Buff[inRecIndex] == 0x0A)
                {
                        /* 移動到bak的檔案結尾 */
                        inRetVal = inFile_Linux_Seek(inBakFd, 0, _SEEK_END_);
			
                        if (inRetVal != VS_SUCCESS)
                        {
                                /* 關檔 */
                                inFile_Linux_Close(inDatFd);
                                inFile_Linux_Close(inBakFd);
                                inFile_Linux_Delete_In_Fs_Data(_HDPT_FILE_NAME_BAK_);

                                /* Free pointer */
                                free(szWriteBuff_Record);
                                free(szWriteBuff_Org);
                                free(szRead_Total_Buff);

                                return (VS_ERROR);
                        }
		
                        inRetVal = inFile_Linux_Write(inBakFd, szWriteBuff_Org, &i);
			
                        if (inRetVal != VS_SUCCESS)
                        {
                                /* 關檔 */
                                inFile_Linux_Close(inDatFd);
                                inFile_Linux_Close(inBakFd);
                                inFile_Linux_Delete_In_Fs_Data(_HDPT_FILE_NAME_BAK_);

                                /* Free pointer */
                                free(szWriteBuff_Record);
                                free(szWriteBuff_Org);
                                free(szRead_Total_Buff);

                                return (VS_ERROR);
                        }

                        memset(szWriteBuff_Org, 0x00, sizeof(szWriteBuff_Org));
                        i = 0;
                }
                else if (szRead_Total_Buff[inRecIndex] == 0x00)
                {
                        /* 讀到0x00結束 */
                        break;
                }

                inRecIndex++;
        }
	
        /* 關檔 */
	fdatasync(inBakFd);
        inFile_Linux_Close(inDatFd);
        inFile_Linux_Close(inBakFd);

        /* Free pointer */
        free(szWriteBuff_Record);
        free(szWriteBuff_Org);
        free(szRead_Total_Buff);
	
        /* 刪除原HDPT.dat */
        inFile_Linux_Delete_In_Fs_Data(_HDPT_FILE_NAME_);

        /* 將HDPT.bak改名字為HDPT.dat取代原檔案 */
        if (inFile_Linux_Rename_In_Fs_Data(_HDPT_FILE_NAME_BAK_, _HDPT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }
	
	/* rwxr--r--*/
	inFunc_Data_Chmod("744", _HDPT_FILE_NAME_, _FS_DATA_PATH_);
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
        /* inSaveHDPTRec_Linux() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveHDPTRec_Linux(%d) END!!", inHDPTRec - 1);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return(VS_SUCCESS);
}

/*
Function        :inSaveHDPTRec_CTOS
Date&Time       :2019/1/24 下午 10:23
Describe        :寫入HDPT.dat，inHDPTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inSaveHDPTRec_CTOS(int inHDPTRec)
{
        unsigned long   uldat_Handle;   			/* FILE Handle */
        unsigned long   ulbak_Handle;   			/* FILE Handle */
        int     	inPackCount = 0;        		/* uszWriteBuff_Record的index */
        int     	inRecIndex = 0;         		/* uszRead_Total_Buff的index */
        int     	inHDPT_Total_Rec = 0;    		/* HDPT.dat的總筆數 */
        int     	inRetVal;
        int     	i = 0, j = 0;
	char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];	/* debug message */
        long    	lnReadLength = 0;       		/* 每次要從HDPT.dat讀多少byte出來 */
        long    	lnHDPTLength = 0;         		/* HDPT.dat檔案總長度 */
        unsigned char   *uszRead_Total_Buff;
        unsigned char   *uszWriteBuff_Record, *uszWriteBuff_Org;

        /* inSaveHDPTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveHDPTRec(%d) START!!", inHDPTRec);
                inLogPrintf(AT, szErrorMsg);
        }
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s start", __LINE__, __FUNCTION__);
	}
	
        /* 防呆先刪除HDPT.bak  */
        inFILE_Delete((unsigned char *)_HDPT_FILE_NAME_BAK_);

        /* 新建HDPT.bak檔案 */
        inRetVal = inFILE_Create(&ulbak_Handle, (unsigned char *)_HDPT_FILE_NAME_BAK_);

        if (inRetVal != VS_SUCCESS)
        {
                inFILE_Close(&ulbak_Handle);

                return (VS_ERROR);
        }

        /* 開啟原檔案HDPT.dat */
        inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_HDPT_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                /* HDPT.dat開檔失敗 ，不用關檔HDPT.dat */
                /* HDPT.bak仍要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_HDPT_FILE_NAME_BAK_);

                return(VS_ERROR);
        }

        /* 取得HDPT.dat檔案大小 */
        lnHDPTLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *)_HDPT_FILE_NAME_);

        /* 取得檔案大小失敗 */
        if (lnHDPTLength == VS_ERROR)
        {
                /* HDPT.bak和HDPT.dat要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Close(&uldat_Handle);
                inFILE_Delete((unsigned char *)_HDPT_FILE_NAME_BAK_);

                return (VS_ERROR);
        }
	
        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        uszWriteBuff_Record = malloc(_SIZE_HDPT_REC_ + _SIZE_HDPT_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_HDPT_REC_ + _SIZE_HDPT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原HDPT.dat的每筆Record 初始化 */
        uszWriteBuff_Org = malloc(_SIZE_HDPT_REC_ + _SIZE_HDPT_COMMA_0D0A_);
        memset(uszWriteBuff_Org, 0x00, _SIZE_HDPT_REC_ + _SIZE_HDPT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存HDPT.dat全部資料 */
        uszRead_Total_Buff = malloc(lnHDPTLength + 1);
        memset(uszRead_Total_Buff, 0x00, lnHDPTLength + 1);
	
        inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

	/* InvoiceNum */
        memcpy(&uszWriteBuff_Record[0], &srHDPTRec.szInvoiceNum[0], strlen(srHDPTRec.szInvoiceNum));
        inPackCount += strlen(srHDPTRec.szInvoiceNum);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* BatchNum */
        memcpy(&uszWriteBuff_Record[inPackCount], &srHDPTRec.szBatchNum[0], strlen(srHDPTRec.szBatchNum));
        inPackCount += strlen(srHDPTRec.szBatchNum);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* STANNum */
        memcpy(&uszWriteBuff_Record[inPackCount], &srHDPTRec.szSTANNum[0], strlen(srHDPTRec.szSTANNum));
        inPackCount += strlen(srHDPTRec.szSTANNum);
	uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* ReversalCnt */
        memcpy(&uszWriteBuff_Record[inPackCount], &srHDPTRec.szReversalCnt[0], strlen(srHDPTRec.szReversalCnt));
        inPackCount += strlen(srHDPTRec.szReversalCnt);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* TRTFileName */
        memcpy(&uszWriteBuff_Record[inPackCount], &srHDPTRec.szTRTFileName[0], strlen(srHDPTRec.szTRTFileName));
	inPackCount += strlen(srHDPTRec.szTRTFileName);
        uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* MustSettleBit */
	memcpy(&uszWriteBuff_Record[inPackCount], &srHDPTRec.szMustSettleBit[0], strlen(srHDPTRec.szMustSettleBit));
        inPackCount += strlen(srHDPTRec.szMustSettleBit);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* SendReversalBit */
	memcpy(&uszWriteBuff_Record[inPackCount], &srHDPTRec.szSendReversalBit[0], strlen(srHDPTRec.szSendReversalBit));
        inPackCount += strlen(srHDPTRec.szSendReversalBit);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* CLS_SettleBit */
	memcpy(&uszWriteBuff_Record[inPackCount], &srHDPTRec.szCLS_SettleBit[0], strlen(srHDPTRec.szCLS_SettleBit));
        inPackCount += strlen(srHDPTRec.szCLS_SettleBit);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* szTicket_InvNum */
	memcpy(&uszWriteBuff_Record[inPackCount], &srHDPTRec.szTicket_InvNum[0], strlen(srHDPTRec.szTicket_InvNum));
        inPackCount += strlen(srHDPTRec.szTicket_InvNum);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* szBatchNumLimit */
        memcpy(&uszWriteBuff_Record[inPackCount], &srHDPTRec.szBatchNumLimit[0], strlen(srHDPTRec.szBatchNumLimit));
        inPackCount += strlen(srHDPTRec.szBatchNumLimit);

        /* 最後的data不用逗號 */
	/* 補上換行符號 */

        /* 0D是移至行首 */
        uszWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        uszWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;
	
        /* 讀HDPT.dat檔案存到uszRead_Total_Buff */
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnHDPTLength;

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
                                        inFILE_Delete((unsigned char *)_HDPT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_HDPT_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_HDPT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }
	
        /* 防呆 總record數量小於要存取inHDPTRec Return ERROR */
        /* 算總Record數 */
        for (i = 0; i < (lnHDPTLength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (uszRead_Total_Buff[i] == 0x0A)
                        inHDPT_Total_Rec++;
        }

        /* 防呆 總record數量小於要存取inHDPTRec Return ERROR */
        if ((inHDPTRec + 1) > inHDPT_Total_Rec)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "No data or Index ERROR");
                }

                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_HDPT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                return (VS_ERROR);
        }

        /* 記錄ReadTotalBuffer的陣列位置 */
        inRecIndex = 0;
        /* 用inHDPTRec決定要先存幾筆Record到HDPT.bak，ex:inHDPTRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inHDPTRec; i ++)
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
                                        inFILE_Delete((unsigned char *)_HDPT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_HDPT_FILE_NAME_BAK_);

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

        /* 存組好的該HDPTRecord 到 HDPT.bak */
        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_HDPT_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_HDPT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原HDPT.dat Record 到 HDPT.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
        /* inRecIndex要指到剩下還沒存進bak檔的第一個記憶體位置 */
        inHDPTRec = inHDPTRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < lnHDPTLength; i++)
        {
                if (uszRead_Total_Buff[i] == 0x0A)
                {
                        j++;

                        if (j == inHDPTRec)
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
                /* 接續存原HDPT.dat的Record */
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
                                inFILE_Delete((unsigned char *)_HDPT_FILE_NAME_BAK_);

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
                                inFILE_Delete((unsigned char *)_HDPT_FILE_NAME_BAK_);

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
	
        /* 刪除原HDPT.dat */
        if (inFILE_Delete((unsigned char *)_HDPT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* 將HDPT.bak改名字為HDPT.dat取代原檔案 */
        if (inFILE_Rename((unsigned char *)_HDPT_FILE_NAME_BAK_, (unsigned char *)_HDPT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s end", __LINE__, __FUNCTION__);
	}
	
        /* inSaveHDPTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveHDPTRec(%d) END!!", inHDPTRec - 1);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return(VS_SUCCESS);
}

/*
Function        :inSaveHDPTRec_SQLite
Date&Time       :2019/4/23 下午 6:00
Describe        :寫入HDPT.dat，inHDPTRec是要讀哪一筆的紀錄，第一筆為0
 *		 改用SQLite
*/
int inSaveHDPTRec_SQLite(int inHDPTRec)
{
	int inRetVal = VS_ERROR;
	char szDebugMsg[100 + 1] = {0};
	SQLITE_ALL_TABLE srAll = {0};

	/* inSaveHDPTRec_Linux()_START */
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inSaveHDPTRec_SQLite(%d) START!!", inHDPTRec);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	/* 將pobTran變數pointer位置放到Table中 */
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inHDPT_Table_Link_HDPTRec(&srAll, _LS_UPDATE_);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Table Link 失敗");
			inLogPrintf(AT, szDebugMsg);
		}
		inUtility_StoreTraceLog_OneStep("Table Link 失敗");
		
		return (VS_ERROR);
	}
	
	inRetVal = inSqlite_Insert_Or_Replace_ByRecordID_All(gszParamDBPath, _HDPT_TABLE_NAME_, inHDPTRec, &srAll);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "db:%s,table:%s,Rec:%d update failed", gszParamDBPath, _HDPT_TABLE_NAME_, inHDPTRec);
			inLogPrintf(AT, szDebugMsg);
		}
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "db:%s,table:%s,Rec:%d update failed", gszParamDBPath, _HDPT_TABLE_NAME_, inHDPTRec);
		inUtility_StoreTraceLog_OneStep(szDebugMsg);
		
		return (VS_ERROR);
	}

	/*[260318_BUG] 因使用原廠API不需再進行同步,移除檔案同步函式 */
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	/* inSaveHDPTRec_Linux() END */
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inSaveHDPTRec_SQLite(%d) END!!", inHDPTRec);
		inLogPrintf(AT, szDebugMsg);
		inLogPrintf(AT, "----------------------------------------");
	}

	return(VS_SUCCESS);
}

/*
Function        :inHDPT_Table_Link_HDPTRec
Date&Time       :2019/4/24 上午 11:00
Describe        :將FunctionPointer放至對應位置
*/
int inHDPT_Table_Link_HDPTRec(SQLITE_ALL_TABLE *srAll, int inLinkState)
{
	SQLITE_INT32T_TABLE TABLE_BATCH_INT[] = 
	{
		{0	,"inTableID"			,&srHDPTRec.inRecordRowID							},	/* inTableID */
		{0	,""				,NULL										}	/* 這行用Null用來知道尾端在哪 */	
	};
	
	SQLITE_INT32T_TABLE TABLE_BATCH_INT64T[] = 
	{
		{0	,""				,NULL										}	/* 這行用Null用來知道尾端在哪 */
	};
	
	SQLITE_CHAR_TABLE TABLE_BATCH_CHAR[] =
	{
		{0	,""				,NULL				,0						}	/* 這行用Null用來知道尾端在哪 */
	};
	
	SQLITE_CHAR_TABLE TABLE_BATCH_TEXT[] =
	{
		
		{0	,"szInvoiceNum"			,&srHDPTRec.szInvoiceNum	,strlen(srHDPTRec.szInvoiceNum)			},	/* Invoice Number */
		{0	,"szBatchNum"			,&srHDPTRec.szBatchNum		,strlen(srHDPTRec.szBatchNum)			},	/* Batch Number */
		{0	,"szSTANNum"			,&srHDPTRec.szSTANNum		,strlen(srHDPTRec.szSTANNum)			},	/* STAN Number */
		{0	,"szReversalCnt"		,&srHDPTRec.szReversalCnt	,strlen(srHDPTRec.szReversalCnt)		},	/* Reversal Count */
		{0	,"szTRTFileName"		,&srHDPTRec.szTRTFileName	,strlen(srHDPTRec.szTRTFileName)		},	/* szTRTFileName */
		{0	,"szMustSettleBit"		,&srHDPTRec.szMustSettleBit	,strlen(srHDPTRec.szMustSettleBit)		},	/* 是否結帳的flag，預設值為N */
		{0	,"szSendReversalBit"		,&srHDPTRec.szSendReversalBit	,strlen(srHDPTRec.szSendReversalBit)		},	/* 預設值為N */
		{0	,"szCLS_SettleBit"		,&srHDPTRec.szCLS_SettleBit	,strlen(srHDPTRec.szCLS_SettleBit)		},	/* 看是否要續傳批次 */
		{0	,"szTicket_InvNum"		,&srHDPTRec.szTicket_InvNum	,strlen(srHDPTRec.szTicket_InvNum)		},	/* 紀錄電票的invoiceNum，簽單的調閱編號用szInvoiceNum的 */
		{0	,"szBatchNumLimit"		,&srHDPTRec.szBatchNumLimit	,strlen(srHDPTRec.szBatchNumLimit)		},	/* 單筆Batch的最大數量 */
		{0	,""				,NULL				,0						}	/* 這行用Null用來知道尾端在哪 */
	};
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inHDPT_Table_Link_HDPTRec()_START");
        }
	
	SQLITE_LINK_TABLE	srLink;
	memset(&srLink, 0x00, sizeof(SQLITE_LINK_TABLE));
	srLink.psrInt = TABLE_BATCH_INT;
	srLink.psrInt64t = TABLE_BATCH_INT64T;
	srLink.psrChar = TABLE_BATCH_CHAR;
	srLink.psrText = TABLE_BATCH_TEXT;
	
	inSqlite_Table_Link(srAll, inLinkState, &srLink);
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inHDPT_Table_Link_HDPTRec()_END");
                inLogPrintf(AT, "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inHDPT_Initial_AllRercord
Date&Time       :2019/4/26 下午 4:35
Describe        :若參數生效，初始化HDPT
*/
int inHDPT_Initial_AllRercord(char* szDBName, char* szTableName)
{
	int	i = 0;
	char	szHostName[8 + 1] = {0};
	
	/* 資料庫先清Table再重建*/
	if (_PARAMETER_SAVE_WAY_DAT_ == _PARAMETER_SAVE_WAY_DAT_SQLITE_)
	{
		inSqlite_Drop_Table(szDBName, szTableName);
		inEDCPara_Create_Table_Flow(szDBName, szTableName, TABLE_HDPT_TAG);
	}
	
	for (i = 0; ; i++)
	{
		if (inLoadHDTRec(i) != VS_SUCCESS)
		{
			break;
		}
		
		inSetInvoiceNum("000001");
		inSetBatchNum("000001");
		inSetSTANNum("000001");
		inSetReversalCnt("000001");
		inSetMustSettleBit("N");
		inSetSendReversalBit("N");
		inSetCLS_SettleBit("N");
		inSetTicket_InvNum("000001");
		inSetBatchNumLimit("0600");
		
		memset(szHostName, 0x00, sizeof(szHostName));
		inGetHostLabel(szHostName);
		
		if (!memcmp(szHostName, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_)))
		{
			inSetTRTFileName(_TRT_FILE_NAME_CREDIT_);
		}
		else if (!memcmp(szHostName, _HOST_NAME_DINERS_, strlen(_HOST_NAME_DINERS_)))
		{
			inSetTRTFileName(_TRT_FILE_NAME_DINERS_);
		}
		else if (!memcmp(szHostName, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_)))
		{
			inSetTRTFileName(_TRT_FILE_NAME_HG_);
		}
		else if (!memcmp(szHostName, _HOST_NAME_VAS_, strlen(_HOST_NAME_VAS_)))
		{
			inSetTRTFileName(_TRT_FILE_NAME_VAS_);
		}
		else if (!memcmp(szHostName, _HOST_NAME_CLM_, strlen(_HOST_NAME_CLM_)))
		{
			inSetTRTFileName(_TRT_FILE_NAME_CLM_);
		}
		else if (!memcmp(szHostName, _HOST_NAME_AMEX_, strlen(_HOST_NAME_AMEX_)))
		{
			inSetTRTFileName(_TRT_FILE_NAME_AMEX_);
		}
		else if (!memcmp(szHostName, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_)))
		{
			inSetTRTFileName(_TRT_FILE_NAME_DCC_);
		}
		else if (!memcmp(szHostName, _HOST_NAME_ESC_, strlen(_HOST_NAME_ESC_)))
		{
			inSetTRTFileName(_TRT_FILE_NAME_ESC_);
		}
		else if (!memcmp(szHostName, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_)))
		{
			inSetTRTFileName(_TRT_FILE_NAME_ESVC_);
		}
		else if (!memcmp(szHostName, _HOST_NAME_EW_, strlen(_HOST_NAME_EW_)))
		{
			inSetTRTFileName(_TRT_FILE_NAME_EW_);
		}
		else if (!memcmp(szHostName, _HOST_NAME_TAKA_, strlen(_HOST_NAME_TAKA_)))
		{
			inSetTRTFileName(_TRT_FILE_NAME_TAKA_);
		}
		else if (!memcmp(szHostName, _HOST_NAME_REDEEM_, strlen(_HOST_NAME_REDEEM_)))
		{
			inSetTRTFileName(_TRT_FILE_NAME_REDEMPTION_);
		}
		else if (!memcmp(szHostName, _HOST_NAME_INST_, strlen(_HOST_NAME_INST_)))
		{
			inSetTRTFileName(_TRT_FILE_NAME_INSTALLMENT_);
		}
		else if (!memcmp(szHostName, _HOST_NAME_UNION_PAY_, strlen(_HOST_NAME_UNION_PAY_)))
		{
			inSetTRTFileName(_TRT_FILE_NAME_UNION_PAY_);
		}
		else if (!memcmp(szHostName, _HOST_NAME_FISC_, strlen(_HOST_NAME_FISC_)))
		{
			inSetTRTFileName(_TRT_FILE_NAME_FISC_);
		}
		else if (!memcmp(szHostName, _HOST_NAME_MAIL_ORDER_, strlen(_HOST_NAME_MAIL_ORDER_)))
		{
			inSetTRTFileName(_TRT_FILE_NAME_MAIL_ORDER_);
		}
                else if (!memcmp(szHostName, _HOST_NAME_TRUST_, strlen(_HOST_NAME_TRUST_)))
		{
			inSetTRTFileName(_TRT_FILE_NAME_TRUST_);
		}
                else if (!memcmp(szHostName, _HOST_NAME_TRUST_WRONG_, strlen(_HOST_NAME_TRUST_WRONG_)))
		{
			inSetTRTFileName(_TRT_FILE_NAME_TRUST_);
		}
		else
		{
			inSetTRTFileName("NULLTRT");
		}
		
		inSaveHDPTRec(i);
	}
	
	return (VS_SUCCESS);
}

/*
set和get等價於相反的操作
各欄位的set和get function
*/

/*
Function        :inGetInvoiceNum
Date&Time       :
Describe        :
*/
int inGetInvoiceNum(char* szInvoiceNum)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szInvoiceNum == NULL || strlen(srHDPTRec.szInvoiceNum) <= 0 || strlen(srHDPTRec.szInvoiceNum) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetInvoiceNum() ERROR !!");

                        if (szInvoiceNum == NULL)
                        {
                                inLogPrintf(AT, "szInvoiceNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szInvoiceNum length = (%d)", (int)strlen(srHDPTRec.szInvoiceNum));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szInvoiceNum[0], &srHDPTRec.szInvoiceNum[0], strlen(srHDPTRec.szInvoiceNum));

        return (VS_SUCCESS);
}

/*
Function        :inSetInvoiceNum
Date&Time       :
Describe        :
*/
int inSetInvoiceNum(char* szInvoiceNum)
{
        memset(srHDPTRec.szInvoiceNum, 0x00, sizeof(srHDPTRec.szInvoiceNum));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szInvoiceNum == NULL || strlen(szInvoiceNum) <= 0 || strlen(szInvoiceNum) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetInvoiceNum() ERROR !!");

                        if (szInvoiceNum == NULL)
                        {
                                inLogPrintf(AT, "szInvoiceNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szInvoiceNum length = (%d)", (int)strlen(szInvoiceNum));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srHDPTRec.szInvoiceNum[0], &szInvoiceNum[0], strlen(szInvoiceNum));

        return (VS_SUCCESS);
}

/*
Function        :inGetBatchNum
Date&Time       :
Describe        :
*/
int inGetBatchNum(char* szBatchNum)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szBatchNum == NULL || strlen(srHDPTRec.szBatchNum) <= 0 || strlen(srHDPTRec.szBatchNum) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetBatchNum() ERROR !!");

                        if (szBatchNum == NULL)
                        {
                                inLogPrintf(AT, "szBatchNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBatchNum length = (%d) ", (int)strlen(srHDPTRec.szBatchNum));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }

        memcpy(&szBatchNum[0], &srHDPTRec.szBatchNum[0], strlen(srHDPTRec.szBatchNum));

        return (VS_SUCCESS);
}

/*
Function        :inSetBatchNum
Date&Time       :
Describe        :
*/
int inSetBatchNum(char* szBatchNum)
{
        memset(srHDPTRec.szBatchNum, 0x00, sizeof(srHDPTRec.szBatchNum));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szBatchNum == NULL || strlen(szBatchNum) <= 0 || strlen(szBatchNum) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetBatchNum() ERROR !!");

                        if (szBatchNum == NULL)
                        {
                                inLogPrintf(AT, "szBatchNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBatchNum length = (%d)", (int)strlen(szBatchNum));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srHDPTRec.szBatchNum[0], &szBatchNum[0], strlen(szBatchNum));

        return (VS_SUCCESS);
}

/*
Function        :inGetSTANNum
Date&Time       :
Describe        :
*/
int inGetSTANNum(char* szSTANNum)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSTANNum == NULL || strlen(srHDPTRec.szSTANNum) <= 0 || strlen(srHDPTRec.szSTANNum) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetSTANNum() ERROR !!");

                        if (szSTANNum == NULL)
                        {
                                inLogPrintf(AT, "szSTANNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSTANNum length = (%d) ", (int)strlen(srHDPTRec.szSTANNum));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSTANNum[0], &srHDPTRec.szSTANNum[0], strlen(srHDPTRec.szSTANNum));

        return (VS_SUCCESS);
}

/*
Function        :inSetSTANNum
Date&Time       :
Describe        :
*/
int inSetSTANNum(char* szSTANNum)
{
        memset(srHDPTRec.szSTANNum, 0x00, sizeof(srHDPTRec.szSTANNum));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSTANNum == NULL || strlen(szSTANNum) <= 0 || strlen(szSTANNum) > 6)
        {
		/* debug */
		if (ginDebug == VS_TRUE)
		{
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetSTANNum() ERROR !!");

			if (szSTANNum == NULL)
			{
				inLogPrintf(AT, "szSTANNum == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szSTANNum length = (%d)", (int)strlen(szSTANNum));
				inLogPrintf(AT, szErrorMsg);
			}
		}

		return (VS_ERROR);
	}
        memcpy(&srHDPTRec.szSTANNum[0], &szSTANNum[0], strlen(szSTANNum));

        return (VS_SUCCESS);
}

/*
Function        :inGetReversalCnt
Date&Time       :
Describe        :
*/
int inGetReversalCnt(char* szReversalCnt)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szReversalCnt == NULL || strlen(srHDPTRec.szReversalCnt) <= 0 || strlen(srHDPTRec.szReversalCnt) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetReversalCnt() ERROR !!");

                        if (szReversalCnt == NULL)
                        {
                                inLogPrintf(AT, "szReversalCnt == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szReversalCnt length = (%d) ", (int)strlen(srHDPTRec.szReversalCnt));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szReversalCnt[0], &srHDPTRec.szReversalCnt[0], strlen(srHDPTRec.szReversalCnt));

        return (VS_SUCCESS);
}

/*
Function        :inSetReversalCnt
Date&Time       :
Describe        :
*/
int inSetReversalCnt(char* szReversalCnt)
{
        memset(srHDPTRec.szReversalCnt, 0x00, sizeof(srHDPTRec.szReversalCnt));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szReversalCnt == NULL || strlen(szReversalCnt) <= 0 || strlen(szReversalCnt) > 6)
        {
		/* debug */
		if (ginDebug == VS_TRUE)
		{
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetReversalCnt() ERROR !!");

			if (szReversalCnt == NULL)
			{
				inLogPrintf(AT, "szReversalCnt == NULL");
			}
			else
			{
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "szReversalCnt length = (%d)", (int)strlen(szReversalCnt));
				inLogPrintf(AT, szErrorMsg);
			}
		}

		return (VS_ERROR);
	}
        memcpy(&srHDPTRec.szReversalCnt[0], &szReversalCnt[0], strlen(szReversalCnt));

        return (VS_SUCCESS);
}

/*
Function        :inGetTRTFileName
Date&Time       :
Describe        :
*/
int inGetTRTFileName(char* szTRTFileName)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTRTFileName == NULL || strlen(srHDPTRec.szTRTFileName) <= 0 || strlen(srHDPTRec.szTRTFileName) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inGetTRTFileName() ERROR !!");

                        if (szTRTFileName == NULL)
                        {
                                inLogPrintf(AT, "szTRTFileName == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "(szTRTFileName length = %d) ", (int)strlen(srHDPTRec.szTRTFileName));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTRTFileName[0], &srHDPTRec.szTRTFileName[0], strlen(srHDPTRec.szTRTFileName));

        return (VS_SUCCESS);
}

/*
Function        :inSetTRTFileName
Date&Time       :
Describe        :
*/
int inSetTRTFileName(char* szTRTFileName)
{
        memset(srHDPTRec.szTRTFileName, 0x00, sizeof(srHDPTRec.szTRTFileName));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szTRTFileName == NULL || strlen(szTRTFileName) <= 0 || strlen(szTRTFileName) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTRTFileName() ERROR !!");

                        if (szTRTFileName == NULL)
                        {
                                inLogPrintf(AT, "szTRTFileName == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTRTFileName length = (%d) ", (int)strlen(srHDPTRec.szTRTFileName));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srHDPTRec.szTRTFileName[0], &szTRTFileName[0], strlen(szTRTFileName));

        return (VS_SUCCESS);
}

/*
Function        :inGetMustSettleBit
Date&Time       :
Describe        :
*/
int inGetMustSettleBit(char* szMustSettleBit)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMustSettleBit == NULL || strlen(srHDPTRec.szMustSettleBit) <= 0 || strlen(srHDPTRec.szMustSettleBit) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inGetMustSettleBit() ERROR !!");

                        if (szMustSettleBit == NULL)
                        {
                                inLogPrintf(AT, "szMustSettleBit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "(szMustSettleBit length = %d) ", (int)strlen(srHDPTRec.szMustSettleBit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMustSettleBit[0], &srHDPTRec.szMustSettleBit[0], strlen(srHDPTRec.szMustSettleBit));

        return (VS_SUCCESS);
}

/*
Function        :inSetMustSettleBit
Date&Time       :
Describe        :
*/
int inSetMustSettleBit(char* szMustSettleBit)
{
        memset(srHDPTRec.szMustSettleBit, 0x00, sizeof(srHDPTRec.szMustSettleBit));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMustSettleBit == NULL || strlen(szMustSettleBit) <= 0 || strlen(szMustSettleBit) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetMustSettleBit() ERROR !!");

                        if (szMustSettleBit == NULL)
                        {
                                inLogPrintf(AT, "szMustSettleBit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMustSettleBit length = (%d) ", (int)strlen(srHDPTRec.szMustSettleBit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srHDPTRec.szMustSettleBit[0], &szMustSettleBit[0], strlen(szMustSettleBit));

        return (VS_SUCCESS);
}

/*
Function        :inGetSendReversalBit
Date&Time       :
Describe        :
*/
int inGetSendReversalBit(char* szSendReversalBit)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSendReversalBit == NULL || strlen(srHDPTRec.szSendReversalBit) <= 0 || strlen(srHDPTRec.szSendReversalBit) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inGetSendReversalBit() ERROR !!");

                        if (szSendReversalBit == NULL)
                        {
                                inLogPrintf(AT, "szSendReversalBit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "(szSendReversalBit length = %d) ", (int)strlen(srHDPTRec.szSendReversalBit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSendReversalBit[0], &srHDPTRec.szSendReversalBit[0], strlen(srHDPTRec.szSendReversalBit));

        return (VS_SUCCESS);
}

/*
Function        :inSetSendReversalBit
Date&Time       :
Describe        :
*/
int inSetSendReversalBit(char* szSendReversalBit)
{
        memset(srHDPTRec.szSendReversalBit, 0x00, sizeof(srHDPTRec.szSendReversalBit));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSendReversalBit == NULL || strlen(szSendReversalBit) <= 0 || strlen(szSendReversalBit) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetSendReversalBit() ERROR !!");

                        if (szSendReversalBit == NULL)
                        {
                                inLogPrintf(AT, "szSendReversalBit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSendReversalBit length = (%d) ", (int)strlen(srHDPTRec.szSendReversalBit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srHDPTRec.szSendReversalBit[0], &szSendReversalBit[0], strlen(szSendReversalBit));

        return (VS_SUCCESS);
}

/*
Function        :inGetCLS_SettleBit
Date&Time       :2017/4/10 下午 3:29
Describe        :
*/
int inGetCLS_SettleBit(char* szCLS_SettleBit)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCLS_SettleBit == NULL || strlen(srHDPTRec.szCLS_SettleBit) <= 0 || strlen(srHDPTRec.szCLS_SettleBit) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inGetCLS_SettleBit() ERROR !!");

                        if (szCLS_SettleBit == NULL)
                        {
                                inLogPrintf(AT, "szCLS_SettleBit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "(szCLS_SettleBit length = %d) ", (int)strlen(srHDPTRec.szCLS_SettleBit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCLS_SettleBit[0], &srHDPTRec.szCLS_SettleBit[0], strlen(srHDPTRec.szCLS_SettleBit));

        return (VS_SUCCESS);
}

/*
Function        :inSetCLS_SettleBit
Date&Time       :2017/4/10 下午 3:29
Describe        :
*/
int inSetCLS_SettleBit(char* szCLS_SettleBit)
{
        memset(srHDPTRec.szCLS_SettleBit, 0x00, sizeof(srHDPTRec.szCLS_SettleBit));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCLS_SettleBit == NULL || strlen(szCLS_SettleBit) <= 0 || strlen(szCLS_SettleBit) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCLS_SettleBit() ERROR !!");

                        if (szCLS_SettleBit == NULL)
                        {
                                inLogPrintf(AT, "szCLS_SettleBit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCLS_SettleBit length = (%d) ", (int)strlen(srHDPTRec.szCLS_SettleBit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srHDPTRec.szCLS_SettleBit[0], &szCLS_SettleBit[0], strlen(szCLS_SettleBit));

        return (VS_SUCCESS);
}

/*
Function        :inGetTicket_InvNum
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inGetTicket_InvNum(char* szTicket_InvNum)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTicket_InvNum == NULL || strlen(srHDPTRec.szTicket_InvNum) <= 0 || strlen(srHDPTRec.szTicket_InvNum) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTicket_InvNum() ERROR !!");

			if (szTicket_InvNum == NULL)
                        {
                                inLogPrintf(AT, "szTicket_InvNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_InvNum length = (%d)", (int)strlen(srHDPTRec.szTicket_InvNum));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTicket_InvNum[0], &srHDPTRec.szTicket_InvNum[0], strlen(srHDPTRec.szTicket_InvNum));

        return (VS_SUCCESS);
}

/*
Function        :inSetTicket_InvNum
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inSetTicket_InvNum(char* szTicket_InvNum)
{
        memset(srHDPTRec.szTicket_InvNum, 0x00, sizeof(srHDPTRec.szTicket_InvNum));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTicket_InvNum == NULL || strlen(szTicket_InvNum) <= 0 || strlen(szTicket_InvNum) > 6)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTicket_InvNum() ERROR !!");
                        if (szTicket_InvNum == NULL)
                        {
                                inLogPrintf(AT, "szTicket_InvNum == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTicket_InvNum length = (%d)", (int)strlen(szTicket_InvNum));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srHDPTRec.szTicket_InvNum[0], &szTicket_InvNum[0], strlen(szTicket_InvNum));

        return (VS_SUCCESS);
}

/*
Function        :inGetBatchNumLimit
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inGetBatchNumLimit(char* szBatchNumLimit)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szBatchNumLimit == NULL || strlen(srHDPTRec.szBatchNumLimit) <= 0 || strlen(srHDPTRec.szBatchNumLimit) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetBatchNumLimit() ERROR !!");

			if (szBatchNumLimit == NULL)
                        {
                                inLogPrintf(AT, "szBatchNumLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBatchNumLimit length = (%d)", (int)strlen(srHDPTRec.szBatchNumLimit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szBatchNumLimit[0], &srHDPTRec.szBatchNumLimit[0], strlen(srHDPTRec.szBatchNumLimit));

        return (VS_SUCCESS);
}

/*
Function        :inSetBatchNumLimit
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inSetBatchNumLimit(char* szBatchNumLimit)
{
        memset(srHDPTRec.szBatchNumLimit, 0x00, sizeof(srHDPTRec.szBatchNumLimit));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szBatchNumLimit == NULL || strlen(szBatchNumLimit) <= 0 || strlen(szBatchNumLimit) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetBatchNumLimit() ERROR !!");
                        if (szBatchNumLimit == NULL)
                        {
                                inLogPrintf(AT, "szBatchNumLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBatchNumLimit length = (%d)", (int)strlen(szBatchNumLimit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srHDPTRec.szBatchNumLimit[0], &szBatchNumLimit[0], strlen(szBatchNumLimit));

        return (VS_SUCCESS);
}

/*
Function        :inHDPT_Edit_HDPT_Table
Date&Time       :2017/4/28 下午 5:18
Describe        :
*/
int inHDPT_Edit_HDPT_Table(void)
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
	inDISP_ChineseFont_Color("是否更改HDPT", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
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
	inLoadHDPTRec(inRecordCnt);
	
	inFunc_Edit_Table_Tag(HDPT_FUNC_TABLE);
	inSaveHDPTRec(inRecordCnt);
	
	return	(VS_SUCCESS);
}

/*
Function        :inHDPT_CheckCopyHDPT
Date&Time       :2017/8/18 下午 1:26
Describe        :因有些流程錯誤會讓DCC覆蓋NCCC的TRT，所以用這個檢查
*/
int inHDPT_CheckCopyHDPT()
{
	int	inIndex = 0;
	char	szTRTFileName[12 + 1];
	char	szDebugMsg[100 + 1];
	
	inIndex = 0;
	inLoadHDPTRec(inIndex);
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	inDISP_ChineseFont(szTRTFileName, _FONTSIZE_8X16_, _LINE_8_1_, _DISP_RIGHT_);
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "---------------------------");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "HDPT %d:%s", inIndex, szTRTFileName);
		inLogPrintf(AT, szDebugMsg);
		inLogPrintf(AT, "---------------------------");
	}
	
	inIndex = 1;
	inLoadHDPTRec(inIndex);
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	inDISP_ChineseFont(szTRTFileName, _FONTSIZE_8X16_, _LINE_8_2_, _DISP_RIGHT_);
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "---------------------------");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "HDPT %d:%s", inIndex, szTRTFileName);
		inLogPrintf(AT, szDebugMsg);
		inLogPrintf(AT, "---------------------------");
	}
	
	inIndex = 2;
	inLoadHDPTRec(inIndex);
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	inDISP_ChineseFont(szTRTFileName, _FONTSIZE_8X16_, _LINE_8_3_, _DISP_RIGHT_);
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "---------------------------");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "HDPT %d:%s", inIndex, szTRTFileName);
		inLogPrintf(AT, szDebugMsg);
		inLogPrintf(AT, "---------------------------");
	}
	
	inIndex = 3;
	inLoadHDPTRec(inIndex);
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	inDISP_ChineseFont(szTRTFileName, _FONTSIZE_8X16_, _LINE_8_4_, _DISP_RIGHT_);
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "---------------------------");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "HDPT %d:%s", inIndex, szTRTFileName);
		inLogPrintf(AT, szDebugMsg);
		inLogPrintf(AT, "---------------------------");
	}
	
	inIndex = 7;
	inLoadHDPTRec(inIndex);
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	inDISP_ChineseFont(szTRTFileName, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_RIGHT_);
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "---------------------------");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "HDPT %d:%s", inIndex, szTRTFileName);
		inLogPrintf(AT, szDebugMsg);
		inLogPrintf(AT, "---------------------------");
	}
	
	inIndex = 8;
	inLoadHDPTRec(inIndex);
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	inDISP_ChineseFont(szTRTFileName, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_);
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "---------------------------");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "HDPT %d:%s", inIndex, szTRTFileName);
		inLogPrintf(AT, szDebugMsg);
		inLogPrintf(AT, "---------------------------");
	}
	
	inIndex = 9;
	inLoadHDPTRec(inIndex);
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	inDISP_ChineseFont(szTRTFileName, _FONTSIZE_8X16_, _LINE_8_7_, _DISP_RIGHT_);
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "---------------------------");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "HDPT %d:%s", inIndex, szTRTFileName);
		inLogPrintf(AT, szDebugMsg);
		inLogPrintf(AT, "---------------------------");
	}
	
	return (VS_SUCCESS);
}

int inHDPT_Test1(void)
{
        unsigned long   ulHandle;
        unsigned char   szFile_Buffer[1024 + 1];
        int     inRetVal = 0;
        char    szTemplate[1024 + 1];


        memset(szFile_Buffer, 0x00, sizeof(szFile_Buffer));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        /* 開啟已存在檔案 */
        inRetVal = inFILE_Open(&ulHandle, (unsigned char*)"HDPT.dat");
        /* 檔案不存在，建立新檔案 */
        //inRetVal = inFILE_Create(&ulHandle, "HDPT.dat");
        /* 尋找檔案測試 */
        //inFILE_Seek(ulHandle, 0, _SEEK_END_);
        /* 寫入檔案測試 不用seek END的話會把原資料改寫掉 */
        //inFILE_Write(&ulHandle, "12341234", sizeof("12341234"));
        /* 寫入檔案位置會到最尾巴，需要用seek回到開頭 */
        inFILE_Seek(ulHandle, 0, _SEEK_BEGIN_);
        /* 讀取檔案測試 */
        inFILE_Read(&ulHandle , szFile_Buffer , 1024);

        strcpy(szTemplate, (char *)szFile_Buffer);
        inLogPrintf(AT, szTemplate);
        inFILE_Close(&ulHandle);

        return (VS_SUCCESS);
}

int inHDPT_Test2(void)
{
//        char    szTemplate[_FILE_DATA_LENGTH_MAX_ + 1];

        inLoadHDPTRec(0);
        inSetInvoiceNum("000002");
        inSetBatchNum("000003");
        inSetSTANNum("000004");
        inSaveHDPTRec(0);

        return (VS_SUCCESS);
}

