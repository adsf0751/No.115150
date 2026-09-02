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
#include "NCCCtmsFTPFLT.h"

static  FTPFLT_REC srFTPFLTRec;	/* construct FTPFLT record */
extern  int     ginDebug;       /* Debug使用 extern */

/*
Function        :inLoadFTPFLTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀FTPFLT檔案，inFTPFLTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadFTPFLTRec(int inFTPFLTRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆FTPFLT檔案 */
        char            szFTPFLTRec[_SIZE_FTPFLT_REC_ + 1];         /* 暫存, 放各個欄位檔案 */
        char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnFTPFLTLength = 0;                       /* FTPFLT總長度 */
        long            lnReadLength;                           /* 記錄每次要從FTPFLT.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;	                /* inRec記錄讀到第幾筆, i為目前從FTPFLT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */
       
        /* inLoadFTPFLTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadFTPFLTRec(%d) START!!", inFTPFLTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 判斷傳進來的inFTPFLTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inFTPFLTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inFTPFLTRec < 0:(index = %d) ERROR!!", inFTPFLTRec);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open FTPFLT.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_FTPFLT_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnFTPFLTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_FTPFLT_FILE_NAME_);
        
        if (lnFTPFLTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnFTPFLTLength + 1);
        uszTemp = malloc(lnFTPFLTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnFTPFLTLength + 1);
        memset(uszTemp, 0x00, lnFTPFLTLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnFTPFLTLength;

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
         *i為目前從FTPFLT讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnFTPFLTLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到FTPFLT的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnFTPFLTLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        /* 因為inFTPFLT_Rec的index從0開始，所以inFTPFLT_Rec要+1 */
                        if (inRec == (inFTPFLTRec + 1))
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
         * 如果沒有inFTPFLTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inFTPFLTRec + 1) || inSearchResult == -1)
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
        memset(&srFTPFLTRec, 0x00, sizeof(srFTPFLTRec));
        /*
         * 以下pattern為存入FTPFLT_Rec
         * i為FTPFLT的第幾個字元
         * 存入FTPFLT_Rec
         */
        i = 0;


        /* 01_檔案索引 */
        /* 初始化 */
        memset(szFTPFLTRec, 0x00, sizeof(szFTPFLTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szFTPFLTRec[k ++] = uszReadData[i ++];
                if (szFTPFLTRec[k - 1] == 0x2C	||
		    szFTPFLTRec[k - 1] == 0x0D	||
		    szFTPFLTRec[k - 1] == 0x0A	||
		    szFTPFLTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnFTPFLTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "FTPFLT unpack ERROR.");
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
        if (szFTPFLTRec[0] != 0x2C	&&
	    szFTPFLTRec[0] != 0x0D	&&
	    szFTPFLTRec[0] != 0x0A	&&
	    szFTPFLTRec[0] != 0x00)
        {
                memcpy(&srFTPFLTRec.szFTPFileIndex[0], &szFTPFLTRec[0], k - 1);
        }

        /* 02_檔案屬性 */
        /* 初始化 */
        memset(szFTPFLTRec, 0x00, sizeof(szFTPFLTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szFTPFLTRec[k ++] = uszReadData[i ++];
                if (szFTPFLTRec[k - 1] == 0x2C	||
		    szFTPFLTRec[k - 1] == 0x0D	||
		    szFTPFLTRec[k - 1] == 0x0A	||
		    szFTPFLTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnFTPFLTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "FTPFLT unpack ERROR");
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
        if (szFTPFLTRec[0] != 0x2C	&&
	    szFTPFLTRec[0] != 0x0D	&&
	    szFTPFLTRec[0] != 0x0A	&&
	    szFTPFLTRec[0] != 0x00)
        {
                memcpy(&srFTPFLTRec.szFTPFileAttribute[0], &szFTPFLTRec[0], k - 1);
        }

        /* 03_檔案路徑 */
        /* 初始化 */
        memset(szFTPFLTRec, 0x00, sizeof(szFTPFLTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szFTPFLTRec[k ++] = uszReadData[i ++];
                if (szFTPFLTRec[k - 1] == 0x2C	||
		    szFTPFLTRec[k - 1] == 0x0D	||
		    szFTPFLTRec[k - 1] == 0x0A	||
		    szFTPFLTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnFTPFLTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "FTPFLT unpack ERROR");
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
        if (szFTPFLTRec[0] != 0x2C	&&
	    szFTPFLTRec[0] != 0x0D	&&
	    szFTPFLTRec[0] != 0x0A	&&
	    szFTPFLTRec[0] != 0x00)
        {
                memcpy(&srFTPFLTRec.szFTPFilePath[0], &szFTPFLTRec[0], k - 1);
        }

        /* 04_檔案名稱 */
        /* 初始化 */
        memset(szFTPFLTRec, 0x00, sizeof(szFTPFLTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szFTPFLTRec[k ++] = uszReadData[i ++];
                if (szFTPFLTRec[k - 1] == 0x2C	||
		    szFTPFLTRec[k - 1] == 0x0D	||
		    szFTPFLTRec[k - 1] == 0x0A	||
		    szFTPFLTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnFTPFLTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "FTPFLT unpack ERROR");
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
        if (szFTPFLTRec[0] != 0x2C	&&
	    szFTPFLTRec[0] != 0x0D	&&
	    szFTPFLTRec[0] != 0x0A	&&
	    szFTPFLTRec[0] != 0x00)
        {
                memcpy(&srFTPFLTRec.szFTPFileName[0], &szFTPFLTRec[0], k - 1);
        }
        
        /* 05_檔案大小 */
        /* 初始化 */
        memset(szFTPFLTRec, 0x00, sizeof(szFTPFLTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szFTPFLTRec[k ++] = uszReadData[i ++];
                if (szFTPFLTRec[k - 1] == 0x2C	||
		    szFTPFLTRec[k - 1] == 0x0D	||
		    szFTPFLTRec[k - 1] == 0x0A	||
		    szFTPFLTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnFTPFLTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "FTPFLT unpack ERROR");
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
        if (szFTPFLTRec[0] != 0x2C	&&
	    szFTPFLTRec[0] != 0x0D	&&
	    szFTPFLTRec[0] != 0x0A	&&
	    szFTPFLTRec[0] != 0x00 && szFTPFLTRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srFTPFLTRec.szFTPFileSize[0], &szFTPFLTRec[0], k - 1);
        }

        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadFTPFLTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadFTPFLTRec(%d) END!!", inFTPFLTRec);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveFTPFLTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSaveFTPFLTRec(int inFTPFLTRec)
{
        unsigned long   uldat_Handle;                           /* FILE Handle */
        unsigned long   ulbak_Handle;                           /* FILE Handle */
        int	        inPackCount = 0;                        /* uszWriteBuff_Record的index */
        int	        inRecIndex = 0;                         /* uszRead_Total_Buff的index */
        int             inFTPFLT_Total_Rec = 0;                    /* FTPFLT.dat的總筆數 */
        int             inRetVal;
        int             i = 0, j = 0;
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long	        lnReadLength = 0;                       /* 每次要從FTPFLT.dat讀多少byte出來 */
        long            lnFTPFLTLength = 0;                        /* FTPFLT.dat檔案總長度 */
        unsigned char	*uszRead_Total_Buff;
        unsigned char   *uszWriteBuff_Record, *uszWriteBuff_Org;


        /* inSaveFTPFLTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveFTPFLTRec(%d)_START!!", inFTPFLTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 防呆先刪除FTPFLT.bak */
        inFILE_Delete((unsigned char *)_FTPFLT_FILE_NAME_BAK_);

        /* 新建FTPFLT.bak檔案 */
        inRetVal = inFILE_Create(&ulbak_Handle, (unsigned char *)_FTPFLT_FILE_NAME_BAK_);

        if (inRetVal != VS_SUCCESS)
        {
                inFILE_Close(&ulbak_Handle);

                return (VS_ERROR);
        }

        /* 開啟原檔案FTPFLT.dat */
        inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_FTPFLT_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                /* FTPFLT.dat開檔失敗 ，不用關檔FTPFLT.dat */
                /* FTPFLT.bak仍要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_FTPFLT_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 取得FTPFLT.dat檔案大小 */
        lnFTPFLTLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *)_FTPFLT_FILE_NAME_);

        /* 取得檔案大小失敗 */
        if (lnFTPFLTLength == VS_ERROR)
        {
                /* FTPFLT.bak和FTPFLT.dat要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Close(&uldat_Handle);
                inFILE_Delete((unsigned char *)_FTPFLT_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        uszWriteBuff_Record = malloc(_SIZE_FTPFLT_REC_ + _SIZE_FTPFLT_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_FTPFLT_REC_ + _SIZE_FTPFLT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原FTPFLT.dat的每筆Record 初始化 */
        uszWriteBuff_Org = malloc(_SIZE_FTPFLT_REC_ + _SIZE_FTPFLT_COMMA_0D0A_);
        memset(uszWriteBuff_Org, 0x00, _SIZE_FTPFLT_REC_ + _SIZE_FTPFLT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存FTPFLT.dat全部資料 */
        uszRead_Total_Buff = malloc(lnFTPFLTLength + 1);
        memset(uszRead_Total_Buff, 0x00, lnFTPFLTLength + 1);

        inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

        /* FTPFileIndex */
        memcpy(&uszWriteBuff_Record[0], &srFTPFLTRec.szFTPFileIndex[0], strlen(srFTPFLTRec.szFTPFileIndex));
        inPackCount += strlen(srFTPFLTRec.szFTPFileIndex);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* FTPFileAttribute */
        memcpy(&uszWriteBuff_Record[inPackCount], &srFTPFLTRec.szFTPFileAttribute[0], strlen(srFTPFLTRec.szFTPFileAttribute));
        inPackCount += strlen(srFTPFLTRec.szFTPFileAttribute);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* FTPFilePath */
        memcpy(&uszWriteBuff_Record[inPackCount], &srFTPFLTRec.szFTPFilePath[0], strlen(srFTPFLTRec.szFTPFilePath));
        inPackCount += strlen(srFTPFLTRec.szFTPFilePath);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* FTPFilePath */
        memcpy(&uszWriteBuff_Record[inPackCount], &srFTPFLTRec.szFTPFileName[0], strlen(srFTPFLTRec.szFTPFileName));
        inPackCount += strlen(srFTPFLTRec.szFTPFileName);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
        
        /* FTPFileSize */
        memcpy(&uszWriteBuff_Record[inPackCount], &srFTPFLTRec.szFTPFileSize[0], strlen(srFTPFLTRec.szFTPFileSize));
        inPackCount += strlen(srFTPFLTRec.szFTPFileSize);

        /* 最後的data不用逗號 */
        /* 補上換行符號 */

        /* 0D是移至行首 */
        uszWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        uszWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;

        /* 讀FTPFLT.dat檔案存到uszRead_Total_Buff */
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnFTPFLTLength;

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
                                        inFILE_Delete((unsigned char *)_FTPFLT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_FTPFLT_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_FTPFLT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        /* 防呆 總record數量小於要存取inFTPFLTRec Return ERROR */
        /* 算總Record數 */
        for (i = 0; i < (lnFTPFLTLength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (uszRead_Total_Buff[i] == 0x0A)
                        inFTPFLT_Total_Rec++;
        }

        /* 防呆 總record數量小於要存取inFTPFLTRec Return ERROR */
        if ((inFTPFLTRec + 1) > inFTPFLT_Total_Rec)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "No data or Index ERROR");
                }

                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_FTPFLT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                return (VS_ERROR);
        }

        /* 記錄ReadTotalBuffer的陣列位置 */
        inRecIndex = 0;
        /* 用inFTPFLTRec決定要先存幾筆Record到FTPFLT.bak，ex:inFTPFLTRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inFTPFLTRec; i ++)
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
                                        inFILE_Delete((unsigned char *)_FTPFLT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_FTPFLT_FILE_NAME_BAK_);

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

        /* 存組好的該FTPFLTRecord 到 FTPFLT.bak */
        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_FTPFLT_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_FTPFLT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原FTPFLT.dat Record 到 FTPFLT.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
        /* inRecIndex要指到剩下還沒存進bak檔的第一個記憶體位置 */
        inFTPFLTRec = inFTPFLTRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < lnFTPFLTLength; i++)
        {
                if (uszRead_Total_Buff[i] == 0x0A)
                {
                        j++;

                        if (j == inFTPFLTRec)
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
                /* 接續存原FTPFLT.dat的Record */
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
                                inFILE_Delete((unsigned char *)_FTPFLT_FILE_NAME_BAK_);

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
                                inFILE_Delete((unsigned char *)_FTPFLT_FILE_NAME_BAK_);

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

        /* 刪除原FTPFLT.dat */
        if (inFILE_Delete((unsigned char *)_FTPFLT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* 將FTPFLT.bak改名字為FTPFLT.dat取代原檔案 */
        if (inFILE_Rename((unsigned char *)_FTPFLT_FILE_NAME_BAK_, (unsigned char *)_FTPFLT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* inSaveFTPFLTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveFTPFLTRec(%d) END!!", inFTPFLTRec - 1);
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
Function        :inGetFTPFileIndex
Date&Time       :
Describe        :
*/
int inGetFTPFileIndex(char* szFTPFileIndex)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPFileIndex == NULL || strlen(srFTPFLTRec.szFTPFileIndex) <= 0 || strlen(srFTPFLTRec.szFTPFileIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetFTPFileIndex() ERROR !!");
                        
			if (szFTPFileIndex == NULL)
                        {
                                inLogPrintf(AT, "szFTPFileIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPFileIndex length = (%d)", (int)strlen(srFTPFLTRec.szFTPFileIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPFileIndex[0], &srFTPFLTRec.szFTPFileIndex[0], strlen(srFTPFLTRec.szFTPFileIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPFileIndex
Date&Time       :
Describe        :
*/
int inSetFTPFileIndex(char* szFTPFileIndex)
{
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szFTPFileIndex == NULL || strlen(szFTPFileIndex) <= 0 || strlen(szFTPFileIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetFTPFileIndex() ERROR !!");
                        if (szFTPFileIndex == NULL)
                        {
                                inLogPrintf(AT, "szFTPFileIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPFileIndex length = (%d)", (int)strlen(szFTPFileIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srFTPFLTRec.szFTPFileIndex, 0x00, sizeof(srFTPFLTRec.szFTPFileIndex));
        memcpy(&srFTPFLTRec.szFTPFileIndex[0], &szFTPFileIndex[0], strlen(szFTPFileIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPFileAttribute
Date&Time       :
Describe        :
*/
int inGetFTPFileAttribute(char* szFTPFileAttribute)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPFileAttribute == NULL || strlen(srFTPFLTRec.szFTPFileAttribute) <= 0 || strlen(srFTPFLTRec.szFTPFileAttribute) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetFTPFileAttribute() ERROR !!");

                        if (szFTPFileAttribute == NULL)
                        {
                                inLogPrintf(AT, "szFTPFileAttribute == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPFileAttribute length = (%d)", (int)strlen(srFTPFLTRec.szFTPFileAttribute));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPFileAttribute[0], &srFTPFLTRec.szFTPFileAttribute[0], strlen(srFTPFLTRec.szFTPFileAttribute));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPFileAttribute
Date&Time       :
Describe        :
*/
int inSetFTPFileAttribute(char* szFTPFileAttribute)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPFileAttribute == NULL || strlen(szFTPFileAttribute) <= 0 || strlen(szFTPFileAttribute) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetFTPFileAttribute() ERROR !!");

                        if (szFTPFileAttribute == NULL)
                        {
                                inLogPrintf(AT, "szFTPFileAttribute == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPFileAttribute length = (%d)", (int)strlen(szFTPFileAttribute));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srFTPFLTRec.szFTPFileAttribute, 0x00, sizeof(srFTPFLTRec.szFTPFileAttribute));
        memcpy(&srFTPFLTRec.szFTPFileAttribute[0], &szFTPFileAttribute[0], strlen(szFTPFileAttribute));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPFilePath
Date&Time       :
Describe        :
*/
int inGetFTPFilePath(char* szFTPFilePath)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPFilePath == NULL || strlen(srFTPFLTRec.szFTPFilePath) <= 0 || strlen(srFTPFLTRec.szFTPFilePath) > 60)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetFTPFilePath() ERROR !!");

                        if (szFTPFilePath == NULL)
                        {
                                inLogPrintf(AT, "szFTPFilePath == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPFilePath length = (%d)", (int)strlen(srFTPFLTRec.szFTPFilePath));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPFilePath[0], &srFTPFLTRec.szFTPFilePath[0], strlen(srFTPFLTRec.szFTPFilePath));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPFilePath
Date&Time       :
Describe        :
*/
int inSetFTPFilePath(char* szFTPFilePath)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPFilePath == NULL || strlen(szFTPFilePath) <= 0 || strlen(szFTPFilePath) > 60)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetFTPFilePath() ERROR !!");

                        if (szFTPFilePath == NULL)
                        {
                                inLogPrintf(AT, "szFTPFilePath == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPFilePath length = (%d)", (int)strlen(szFTPFilePath));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srFTPFLTRec.szFTPFilePath, 0x00, sizeof(srFTPFLTRec.szFTPFilePath));
        memcpy(&srFTPFLTRec.szFTPFilePath[0], &szFTPFilePath[0], strlen(szFTPFilePath));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPFileName
Date&Time       :
Describe        :
*/
int inGetFTPFileName(char* szFTPFileName)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPFileName == NULL || strlen(srFTPFLTRec.szFTPFileName) <= 0 || strlen(srFTPFLTRec.szFTPFileName) > 26)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetFTPFileName() ERROR !!");

                        if (szFTPFileName == NULL)
                        {
                                inLogPrintf(AT, "szFTPFileName == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPFileName length = (%d)", (int)strlen(srFTPFLTRec.szFTPFileName));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPFileName[0], &srFTPFLTRec.szFTPFileName[0], strlen(srFTPFLTRec.szFTPFileName));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPFileName
Date&Time       :
Describe        :
*/
int inSetFTPFileName(char* szFTPFileName)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPFileName == NULL || strlen(szFTPFileName) <= 0 || strlen(szFTPFileName) > 26)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetFTPFileName() ERROR !!");

                        if (szFTPFileName == NULL)
                        {
                                inLogPrintf(AT, "szFTPFileName == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPFileName length = (%d)", (int)strlen(szFTPFileName));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srFTPFLTRec.szFTPFileName, 0x00, sizeof(srFTPFLTRec.szFTPFileName));
        memcpy(&srFTPFLTRec.szFTPFileName[0], &szFTPFileName[0], strlen(szFTPFileName));

        return (VS_SUCCESS);
}

/*
Function        :inGetFTPFileSize
Date&Time       :
Describe        :
*/
int inGetFTPFileSize(char* szFTPFileSize)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szFTPFileSize == NULL || strlen(srFTPFLTRec.szFTPFileSize) <= 0 || strlen(srFTPFLTRec.szFTPFileSize) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetFTPFileSize() ERROR !!");

                        if (szFTPFileSize == NULL)
                        {
                                inLogPrintf(AT, "szFTPFileSize == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPFileSize length = (%d)", (int)strlen(srFTPFLTRec.szFTPFileSize));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFTPFileSize[0], &srFTPFLTRec.szFTPFileSize[0], strlen(srFTPFLTRec.szFTPFileSize));

        return (VS_SUCCESS);
}

/*
Function        :inSetFTPFileSize
Date&Time       :
Describe        :
*/
int inSetFTPFileSize(char* szFTPFileSize)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFTPFileSize == NULL || strlen(szFTPFileSize) <= 0 || strlen(szFTPFileSize) > 10)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetFTPFileSize() ERROR !!");

                        if (szFTPFileSize == NULL)
                        {
                                inLogPrintf(AT, "szFTPFileSize == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFTPFileSize length = (%d)", (int)strlen(szFTPFileSize));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        
        memset(srFTPFLTRec.szFTPFileSize, 0x00, sizeof(srFTPFLTRec.szFTPFileSize));
        memcpy(&srFTPFLTRec.szFTPFileSize[0], &szFTPFileSize[0], strlen(szFTPFileSize));

        return (VS_SUCCESS);
}

