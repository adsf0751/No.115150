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
#include "PCD.h"

static  PCD_REC srPCDRec;	/* construct PCD record */
extern  int     ginDebug;       /* Debug使用 extern */

/*
Function        :inLoadPCDRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀PCD檔案，inPCDRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadPCDRec(int inPCDRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆PCD檔案 */
        char            szPCDRec[_SIZE_PCD_REC_ + 1];           /* 暫存, 放各個欄位檔案 */
        char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnPCDLength = 0;                        /* PCD總長度 */
        long            lnReadLength;                           /* 記錄每次要從PCD.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;	                /* inRec記錄讀到第幾筆, i為目前從PCD讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */
       
        /* inLoadPCDRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadPCDRec(%d) START!!", inPCDRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 判斷傳進來的inPCDRec是否小於零 大於等於零才是正確值(防呆) */
        if (inPCDRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inPCDRec < 0:(index = %d) ERROR!!", inPCDRec);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open PCD.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_PCD_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnPCDLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_PCD_FILE_NAME_);
        
        if (lnPCDLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnPCDLength + 1);
        uszTemp = malloc(lnPCDLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnPCDLength + 1);
        memset(uszTemp, 0x00, lnPCDLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnPCDLength;

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
         *i為目前從PCD讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnPCDLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到PCD的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnPCDLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        /* 因為inPCD_Rec的index從0開始，所以inPCD_Rec要+1 */
                        if (inRec == (inPCDRec + 1))
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
         * 如果沒有inPCDRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inPCDRec + 1) || inSearchResult == -1)
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
        memset(&srPCDRec, 0x00, sizeof(srPCDRec));
        /*
         * 以下pattern為存入PCD_Rec
         * i為PCD的第幾個字元
         * 存入PCD_Rec
         */
        i = 0;


        /* 01_管理號碼索引 */
        /* 初始化 */
        memset(szPCDRec, 0x00, sizeof(szPCDRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szPCDRec[k ++] = uszReadData[i ++];
                if (szPCDRec[k - 1] == 0x2C	||
		    szPCDRec[k - 1] == 0x0D	||
		    szPCDRec[k - 1] == 0x0A	||
		    szPCDRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnPCDLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "PCD unpack ERROR.");
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
        if (szPCDRec[0] != 0x2C	&&
	    szPCDRec[0] != 0x0D	&&
	    szPCDRec[0] != 0x0A	&&
	    szPCDRec[0] != 0x00)
        {
                memcpy(&srPCDRec.szProductCodeIndex[0], &szPCDRec[0], k - 1);
        }

        /* 02_端末機對應之按鍵(Hot Key) */
        /* 初始化 */
        memset(szPCDRec, 0x00, sizeof(szPCDRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szPCDRec[k ++] = uszReadData[i ++];
                if (szPCDRec[k - 1] == 0x2C	||
		    szPCDRec[k - 1] == 0x0D	||
		    szPCDRec[k - 1] == 0x0A	||
		    szPCDRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnPCDLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "PCD unpack ERROR");
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
        if (szPCDRec[0] != 0x2C	&&
	    szPCDRec[0] != 0x0D	&&
	    szPCDRec[0] != 0x0A	&&
	    szPCDRec[0] != 0x00)
        {
                memcpy(&srPCDRec.szKeyMap[0], &szPCDRec[0], k - 1);
        }

        /* 03_產品代碼及產品說明 */
        /* 初始化 */
        memset(szPCDRec, 0x00, sizeof(szPCDRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szPCDRec[k ++] = uszReadData[i ++];
                if (szPCDRec[k - 1] == 0x2C	||
		    szPCDRec[k - 1] == 0x0D	||
		    szPCDRec[k - 1] == 0x0A	||
		    szPCDRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnPCDLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "PCD unpack ERROR");
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
        if (szPCDRec[0] != 0x2C	&&
	    szPCDRec[0] != 0x0D	&&
	    szPCDRec[0] != 0x0A	&&
	    szPCDRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srPCDRec.szProductScript[0], &szPCDRec[0], k - 1);
        }

        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadPCDRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadPCDRec(%d) END!!", inPCDRec);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSavePCDRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSavePCDRec(int inPCDRec)
{
        unsigned long   uldat_Handle;                           /* FILE Handle */
        unsigned long   ulbak_Handle;                           /* FILE Handle */
        int	        inPackCount = 0;                        /* uszWriteBuff_Record的index */
        int	        inRecIndex = 0;                         /* uszRead_Total_Buff的index */
        int             inPCD_Total_Rec = 0;                    /* PCD.dat的總筆數 */
        int             inRetVal;
        int             i = 0, j = 0;
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long	        lnReadLength = 0;                       /* 每次要從PCD.dat讀多少byte出來 */
        long            lnPCDLength = 0;                        /* PCD.dat檔案總長度 */
        unsigned char	*uszRead_Total_Buff;
        unsigned char   *uszWriteBuff_Record, *uszWriteBuff_Org;


        /* inSavePCDRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSavePCDRec(%d)_START!!", inPCDRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 防呆先刪除PCD.bak */
        inFILE_Delete((unsigned char *)_PCD_FILE_NAME_BAK_);

        /* 新建PCD.bak檔案 */
        inRetVal = inFILE_Create(&ulbak_Handle, (unsigned char *)_PCD_FILE_NAME_BAK_);

        if (inRetVal != VS_SUCCESS)
        {
                inFILE_Close(&ulbak_Handle);

                return (VS_ERROR);
        }

        /* 開啟原檔案PCD.dat */
        inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_PCD_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                /* PCD.dat開檔失敗 ，不用關檔PCD.dat */
                /* PCD.bak仍要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_PCD_FILE_NAME_BAK_);

                return(VS_ERROR);
        }

        /* 取得PCD.dat檔案大小 */
        lnPCDLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *)_PCD_FILE_NAME_);

        /* 取得檔案大小失敗 */
        if (lnPCDLength == VS_ERROR)
        {
                /* PCD.bak和PCD.dat要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Close(&uldat_Handle);
                inFILE_Delete((unsigned char *)_PCD_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        uszWriteBuff_Record = malloc(_SIZE_PCD_REC_ + _SIZE_PCD_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_PCD_REC_ + _SIZE_PCD_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原PCD.dat的每筆Record 初始化 */
        uszWriteBuff_Org = malloc(_SIZE_PCD_REC_ + _SIZE_PCD_COMMA_0D0A_);
        memset(uszWriteBuff_Org, 0x00, _SIZE_PCD_REC_ + _SIZE_PCD_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存PCD.dat全部資料 */
        uszRead_Total_Buff = malloc(lnPCDLength + 1);
        memset(uszRead_Total_Buff, 0x00, lnPCDLength + 1);

        inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

        /* ProductCodeIndex */
        memcpy(&uszWriteBuff_Record[0], &srPCDRec.szProductCodeIndex[0], strlen(srPCDRec.szProductCodeIndex));
        inPackCount += strlen(srPCDRec.szProductCodeIndex);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* KeyMap */
        memcpy(&uszWriteBuff_Record[inPackCount], &srPCDRec.szKeyMap[0], strlen(srPCDRec.szKeyMap));
        inPackCount += strlen(srPCDRec.szKeyMap);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* ProductScript */
        memcpy(&uszWriteBuff_Record[inPackCount], &srPCDRec.szProductScript[0], strlen(srPCDRec.szProductScript));
        inPackCount += strlen(srPCDRec.szProductScript);

        /* 最後的data不用逗號 */
        /* 補上換行符號 */

        /* 0D是移至行首 */
        uszWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        uszWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;

        /* 讀PCD.dat檔案存到uszRead_Total_Buff */
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnPCDLength;

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
                                        inFILE_Delete((unsigned char *)_PCD_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_PCD_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_PCD_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        /* 防呆 總record數量小於要存取inPCDRec Return ERROR */
        /* 算總Record數 */
        for (i = 0; i < (lnPCDLength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (uszRead_Total_Buff[i] == 0x0A)
                        inPCD_Total_Rec++;
        }

        /* 防呆 總record數量小於要存取inPCDRec Return ERROR */
        if ((inPCDRec + 1) > inPCD_Total_Rec)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "No data or Index ERROR");
                }

                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_PCD_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                return (VS_ERROR);
        }

        /* 記錄ReadTotalBuffer的陣列位置 */
        inRecIndex = 0;
        /* 用inPCDRec決定要先存幾筆Record到PCD.bak，ex:inPCDRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inPCDRec; i ++)
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
                                        inFILE_Delete((unsigned char *)_PCD_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_PCD_FILE_NAME_BAK_);

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

        /* 存組好的該PCDRecord 到 PCD.bak */
        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_PCD_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_PCD_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原PCD.dat Record 到 PCD.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
        /* inRecIndex要指到剩下還沒存進bak檔的第一個記憶體位置 */
        inPCDRec = inPCDRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < lnPCDLength; i++)
        {
                if (uszRead_Total_Buff[i] == 0x0A)
                {
                        j++;

                        if (j == inPCDRec)
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
                /* 接續存原PCD.dat的Record */
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
                                inFILE_Delete((unsigned char *)_PCD_FILE_NAME_BAK_);

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
                                inFILE_Delete((unsigned char *)_PCD_FILE_NAME_BAK_);

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

        /* 刪除原PCD.dat */
        if (inFILE_Delete((unsigned char *)_PCD_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* 將PCD.bak改名字為PCD.dat取代原檔案 */
        if (inFILE_Rename((unsigned char *)_PCD_FILE_NAME_BAK_, (unsigned char *)_PCD_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* inSavePCDRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSavePCDRec(%d) END!!", inPCDRec - 1);
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
Function        :inGetProductCodeIndex
Date&Time       :
Describe        :
*/
int inGetProductCodeIndex(char* szProductCodeIndex)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szProductCodeIndex == NULL || strlen(srPCDRec.szProductCodeIndex) <= 0 || strlen(srPCDRec.szProductCodeIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetProductCodeIndex() ERROR !!");
                        
			if (szProductCodeIndex == NULL)
                        {
                                inLogPrintf(AT, "szProductCodeIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szProductCodeIndex length = (%d)", (int)strlen(srPCDRec.szProductCodeIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szProductCodeIndex[0], &srPCDRec.szProductCodeIndex[0], strlen(srPCDRec.szProductCodeIndex));

        return (VS_SUCCESS);
}

/*
Function        :inSetProductCodeIndex
Date&Time       :
Describe        :
*/
int inSetProductCodeIndex(char* szProductCodeIndex)
{
        memset(srPCDRec.szProductCodeIndex, 0x00, sizeof(srPCDRec.szProductCodeIndex));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szProductCodeIndex == NULL || strlen(szProductCodeIndex) <= 0 || strlen(szProductCodeIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetProductCodeIndex() ERROR !!");
                        if (szProductCodeIndex == NULL)
                        {
                                inLogPrintf(AT, "szProductCodeIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szProductCodeIndex length = (%d)", (int)strlen(szProductCodeIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srPCDRec.szProductCodeIndex[0], &szProductCodeIndex[0], strlen(szProductCodeIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetKeyMap
Date&Time       :
Describe        :
*/
int inGetKeyMap(char* szKeyMap)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szKeyMap == NULL || strlen(srPCDRec.szKeyMap) <= 0 || strlen(srPCDRec.szKeyMap) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetKeyMap() ERROR !!");

                        if (szKeyMap == NULL)
                        {
                                inLogPrintf(AT, "szKeyMap == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szKeyMap length = (%d)", (int)strlen(srPCDRec.szKeyMap));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szKeyMap[0], &srPCDRec.szKeyMap[0], strlen(srPCDRec.szKeyMap));

        return (VS_SUCCESS);
}

/*
Function        :inSetKeyMap
Date&Time       :
Describe        :
*/
int inSetKeyMap(char* szKeyMap)
{
        memset(srPCDRec.szKeyMap, 0x00, sizeof(srPCDRec.szKeyMap));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szKeyMap == NULL || strlen(szKeyMap) <= 0 || strlen(szKeyMap) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetKeyMap() ERROR !!");

                        if (szKeyMap == NULL)
                        {
                                inLogPrintf(AT, "szKeyMap == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szKeyMap length = (%d)", (int)strlen(szKeyMap));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srPCDRec.szKeyMap[0], &szKeyMap[0], strlen(szKeyMap));

        return (VS_SUCCESS);
}

/*
Function        :inGetProductScript
Date&Time       :
Describe        :
*/
int inGetProductScript(char* szProductScript)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szProductScript == NULL || strlen(srPCDRec.szProductScript) <= 0 || strlen(srPCDRec.szProductScript) > 32)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetProductScript() ERROR !!");

                        if (szProductScript == NULL)
                        {
                                inLogPrintf(AT, "szProductScript == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szProductScript length = (%d)", (int)strlen(srPCDRec.szProductScript));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szProductScript[0], &srPCDRec.szProductScript[0], strlen(srPCDRec.szProductScript));

        return (VS_SUCCESS);
}

/*
Function        :inSetProductScript
Date&Time       :
Describe        :
*/
int inSetProductScript(char* szProductScript)
{
        memset(srPCDRec.szProductScript, 0x00, sizeof(srPCDRec.szProductScript));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szProductScript == NULL || strlen(szProductScript) <= 0 || strlen(szProductScript) > 32)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetProductScript() ERROR !!");

                        if (szProductScript == NULL)
                        {
                                inLogPrintf(AT, "szProductScript == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szProductScript length = (%d)", (int)strlen(szProductScript));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srPCDRec.szProductScript[0], &szProductScript[0], strlen(szProductScript));

        return (VS_SUCCESS);
}
