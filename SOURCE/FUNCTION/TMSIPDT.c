#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctosapi.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../DISPLAY/Display.h"
#include "File.h"
#include "Function.h"
#include "Utility.h"
#include "TMSIPDT.h"

static  TMSIPDT_REC	srTMSIPDTRec;	/* construct TMSIPDT record */
extern  int	ginDebug;		/* Debug使用 extern */

/*
Function        :inLoadTMSIPDTRec
Date&Time       :2024/4/1 下午 1:16
Describe        :優化存參數部份
*/
int inLoadTMSIPDTRec(int inTMSIPDTRec)
{
	int	inRetVal = VS_ERROR;
	
	/* 僅一行資料，不考慮轉存為資料庫 */
	inRetVal = inLoadTMSIPDTRec_CTOS(inTMSIPDTRec);

        return (inRetVal);
}

/*
Function        :inLoadTMSIPDTRec_CTOS
Date&Time       :2024/4/1 下午 1:20
Describe        :讀TMSIPDT檔案，inTMSIPDTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadTMSIPDTRec_CTOS(int inTMSIPDTRec)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆TMSIPDT檔案 */
        char            szTMSIPDTRec[_SIZE_TMSIPDT_REC_ + 1];	/* 暫存, 放各個欄位檔案 */
        char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnTMSIPDTLength = 0;			/* TMSIPDT總長度 */
        long            lnReadLength;                           /* 記錄每次要從TMSIPDT.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;	                /* inRec記錄讀到第幾筆, i為目前從TMSIPDT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */

        /* inLoadTMSIPDTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadTMSIPDTRec_CTOS(%d) START!!", inTMSIPDTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 判斷傳進來的inTMSIPDTRec是否小於零 大於等於零才是正確值(防呆) */
        if (inTMSIPDTRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inTMSIPDTRec < 0:(index = %d) ERROR!!", inTMSIPDTRec);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open TMSIPDT.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_TMSIPDT_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnTMSIPDTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_TMSIPDT_FILE_NAME_);

        if (lnTMSIPDTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnTMSIPDTLength + 1);
        uszTemp = malloc(lnTMSIPDTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnTMSIPDTLength + 1);
        memset(uszTemp, 0x00, lnTMSIPDTLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnTMSIPDTLength;

                for (i = 0;; ++i)
                {
                        /* 剩餘長度大於或等於1024 */
                        if (lnReadLength >= 1024)
                        {
                                if (inFILE_Read(&ulFile_Handle, &uszTemp[1024 * i], 1024) == VS_SUCCESS)
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
                                if (inFILE_Read(&ulFile_Handle, &uszTemp[1024 * i], lnReadLength) == VS_SUCCESS)
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
         *i為目前從TMSIPDT讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnTMSIPDTLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到TMSIPDT的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnTMSIPDTLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        /* 因為inTMSIPDT_Rec的index從0開始，所以inTMSIPDT_Rec要+1 */
                        if (inRec == (inTMSIPDTRec + 1))
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
         * 如果沒有inTMSIPDTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inTMSIPDTRec + 1) || inSearchResult == -1)
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
        memset(&srTMSIPDTRec, 0x00, sizeof(srTMSIPDTRec));
        /*
         * 以下pattern為存入TMSIPDT_Rec
         * i為TMSIPDT的第幾個字元
         * 存入TMSIPDT_Rec
         */
        i = 0;


        /* 01_szTMS_IP_Primary */
        /* 初始化 */
        memset(szTMSIPDTRec, 0x00, sizeof(szTMSIPDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSIPDTRec[k ++] = uszReadData[i ++];
                if (szTMSIPDTRec[k - 1] == 0x2C	||
		    szTMSIPDTRec[k - 1] == 0x0D	||
		    szTMSIPDTRec[k - 1] == 0x0A	||
		    szTMSIPDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSIPDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSIPDT unpack ERROR.");
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
        if (szTMSIPDTRec[0] != 0x2C	&&
	    szTMSIPDTRec[0] != 0x0D	&&
	    szTMSIPDTRec[0] != 0x0A	&&
	    szTMSIPDTRec[0] != 0x00)
        {
                memcpy(&srTMSIPDTRec.szTMS_IP_Primary[0], &szTMSIPDTRec[0], k - 1);
        }

        /* 02_szTMS_PortNo_Primary */
        /* 初始化 */
        memset(szTMSIPDTRec, 0x00, sizeof(szTMSIPDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSIPDTRec[k ++] = uszReadData[i ++];
                if (szTMSIPDTRec[k - 1] == 0x2C	||
		    szTMSIPDTRec[k - 1] == 0x0D	||
		    szTMSIPDTRec[k - 1] == 0x0A	||
		    szTMSIPDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSIPDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSIPDT unpack ERROR");
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
        if (szTMSIPDTRec[0] != 0x2C	&&
	    szTMSIPDTRec[0] != 0x0D	&&
	    szTMSIPDTRec[0] != 0x0A	&&
	    szTMSIPDTRec[0] != 0x00)
        {
                memcpy(&srTMSIPDTRec.szTMS_PortNo_Primary[0], &szTMSIPDTRec[0], k - 1);
        }

        /* 03_szTMS_IP_Second */
        /* 初始化 */
        memset(szTMSIPDTRec, 0x00, sizeof(szTMSIPDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSIPDTRec[k ++] = uszReadData[i ++];
                if (szTMSIPDTRec[k - 1] == 0x2C	||
		    szTMSIPDTRec[k - 1] == 0x0D	||
		    szTMSIPDTRec[k - 1] == 0x0A	||
		    szTMSIPDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSIPDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSIPDT unpack ERROR");
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
        if (szTMSIPDTRec[0] != 0x2C	&&
	    szTMSIPDTRec[0] != 0x0D	&&
	    szTMSIPDTRec[0] != 0x0A	&&
	    szTMSIPDTRec[0] != 0x00)
        {
                memcpy(&srTMSIPDTRec.szTMS_IP_Second[0], &szTMSIPDTRec[0], k - 1);
        }

        /* 04_szTMS_PortNo_Second */
        /* 初始化 */
        memset(szTMSIPDTRec, 0x00, sizeof(szTMSIPDTRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szTMSIPDTRec[k ++] = uszReadData[i ++];
                if (szTMSIPDTRec[k - 1] == 0x2C	||
		    szTMSIPDTRec[k - 1] == 0x0D	||
		    szTMSIPDTRec[k - 1] == 0x0A	||
		    szTMSIPDTRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnTMSIPDTLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "TMSIPDT unpack ERROR");
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
        if (szTMSIPDTRec[0] != 0x2C	&&
	    szTMSIPDTRec[0] != 0x0D	&&
	    szTMSIPDTRec[0] != 0x0A	&&
	    szTMSIPDTRec[0] != 0x00)
        {
                memcpy(&srTMSIPDTRec.szTMS_PortNo_Second[0], &szTMSIPDTRec[0], k - 1);
        }
	
        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadTMSIPDTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadTMSIPDTRec_CTOS(%d) END!!", inTMSIPDTRec);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSaveTMSIPDTRec
Date&Time       :2024/4/2 下午 1:41
Describe        :
*/
int inSaveTMSIPDTRec(int inTMSIPDTRec)
{
	int	inRetVal = VS_ERROR;
	
	/* 僅一行資料，不考慮轉存為資料庫 */
	inRetVal = inSaveTMSIPDTRec_CTOS(inTMSIPDTRec);
	
	if (inRetVal != VS_SUCCESS)
	{
		inFunc_EDCLock(AT);
	}

        return (inRetVal);
}

/*
Function        :inSaveTMSIPDTRec
Date&Time       :2024/4/1 下午 2:14
Describe        :
*/
int inSaveTMSIPDTRec_CTOS(int inTMSIPDTRec)
{
        unsigned long   uldat_Handle;                           /* FILE Handle */
        unsigned long   ulbak_Handle;                           /* FILE Handle */
        int	        inPackCount = 0;                        /* uszWriteBuff_Record的index */
        int	        inRecIndex = 0;                         /* uszRead_Total_Buff的index */
        int             inTMSIPDT_Total_Rec = 0;			/* TMSIPDT.dat的總筆數 */
        int             inRetVal;
        int             i = 0, j = 0;
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long	        lnReadLength = 0;                       /* 每次要從TMSIPDT.dat讀多少byte出來 */
        long            lnTMSIPDTLength = 0;			/* TMSIPDT.dat檔案總長度 */
        unsigned char	*uszRead_Total_Buff;
        unsigned char   *uszWriteBuff_Record, *uszWriteBuff_Org;


        /* inSaveTMSIPDTRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveTMSIPDTRec_CTOS(%d)_START!!", inTMSIPDTRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 防呆先刪除TMSIPDT.bak */
        inFILE_Delete((unsigned char *)_TMSIPDT_FILE_NAME_BAK_);

        /* 新建TMSIPDT.bak檔案 */
        inRetVal = inFILE_Create(&ulbak_Handle, (unsigned char *)_TMSIPDT_FILE_NAME_BAK_);

        if (inRetVal != VS_SUCCESS)
        {
                inFILE_Close(&ulbak_Handle);

                return (VS_ERROR);
        }

        /* 開啟原檔案TMSIPDT.dat */
        inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_TMSIPDT_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                inRetVal = inFILE_Create(&uldat_Handle, (unsigned char *)_TMSIPDT_FILE_NAME_);
		if (inRetVal != VS_SUCCESS)
		{
			/* TMSIPDT.dat開檔失敗 ，不用關檔TMSIPDT.dat */
			/* TMSIPDT.bak仍要關並刪除 */
			inFILE_Close(&ulbak_Handle);
			inFILE_Delete((unsigned char *)_TMSIPDT_FILE_NAME_BAK_);

			return(VS_ERROR);
		}
        }

        /* 取得TMSIPDT.dat檔案大小 */
        lnTMSIPDTLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *)_TMSIPDT_FILE_NAME_);

        /* 取得檔案大小失敗 */
        if (lnTMSIPDTLength == VS_ERROR)
        {
                /* TMSIPDT.bak和TMSIPDT.dat要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Close(&uldat_Handle);
                inFILE_Delete((unsigned char *)_TMSIPDT_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        uszWriteBuff_Record = malloc(_SIZE_TMSIPDT_REC_ + _SIZE_TMSIPDT_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_TMSIPDT_REC_ + _SIZE_TMSIPDT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原TMSIPDT.dat的每筆Record 初始化 */
        uszWriteBuff_Org = malloc(_SIZE_TMSIPDT_REC_ + _SIZE_TMSIPDT_COMMA_0D0A_);
        memset(uszWriteBuff_Org, 0x00, _SIZE_TMSIPDT_REC_ + _SIZE_TMSIPDT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存TMSIPDT.dat全部資料 */
        uszRead_Total_Buff = malloc(lnTMSIPDTLength + 1);
        memset(uszRead_Total_Buff, 0x00, lnTMSIPDTLength + 1);

        inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

        /* szTMS_IP_Primary */
        memcpy(&uszWriteBuff_Record[0], &srTMSIPDTRec.szTMS_IP_Primary[0], strlen(srTMSIPDTRec.szTMS_IP_Primary));
        inPackCount += strlen(srTMSIPDTRec.szTMS_IP_Primary);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* szTMS_PortNo_Primary */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTMSIPDTRec.szTMS_PortNo_Primary[0], strlen(srTMSIPDTRec.szTMS_PortNo_Primary));
        inPackCount += strlen(srTMSIPDTRec.szTMS_PortNo_Primary);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* szTMS_IP_Second */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTMSIPDTRec.szTMS_IP_Second[0], strlen(srTMSIPDTRec.szTMS_IP_Second));
        inPackCount += strlen(srTMSIPDTRec.szTMS_IP_Second);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* szTMS_PortNo_Second */
        memcpy(&uszWriteBuff_Record[inPackCount], &srTMSIPDTRec.szTMS_PortNo_Second[0], strlen(srTMSIPDTRec.szTMS_PortNo_Second));
        inPackCount += strlen(srTMSIPDTRec.szTMS_PortNo_Second);

        /* 最後的data不用逗號 */
        /* 補上換行符號 */

        /* 0D是移至行首 */
        uszWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        uszWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;

        /* 讀TMSIPDT.dat檔案存到uszRead_Total_Buff */
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnTMSIPDTLength;

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
                                        inFILE_Delete((unsigned char *)_TMSIPDT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_TMSIPDT_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_TMSIPDT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        /* 防呆 總record數量小於要存取inTMSIPDTRec Return ERROR */
        /* 算總Record數 */
        for (i = 0; i < (lnTMSIPDTLength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (uszRead_Total_Buff[i] == 0x0A)
                        inTMSIPDT_Total_Rec++;
        }

        /* 防呆 總record數量小於要存取inTMSIPDTRec Return ERROR */
        if ((inTMSIPDTRec + 1) > inTMSIPDT_Total_Rec)
        {
		if (lnTMSIPDTLength == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Org Dat No data");
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "No data or Index ERROR");
			}

			/* 關檔 */
			inFILE_Close(&uldat_Handle);
			inFILE_Close(&ulbak_Handle);
			inFILE_Delete((unsigned char *)_TMSIPDT_FILE_NAME_BAK_);

			/* Free pointer */
			free(uszRead_Total_Buff);
			free(uszWriteBuff_Record);
			free(uszWriteBuff_Org);

			return (VS_ERROR);
		}
        }

        /* 記錄ReadTotalBuffer的陣列位置 */
        inRecIndex = 0;
        /* 用inTMSIPDTRec決定要先存幾筆Record到TMSIPDT.bak，ex:inTMSIPDTRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inTMSIPDTRec; i ++)
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
                                        inFILE_Delete((unsigned char *)_TMSIPDT_FILE_NAME_BAK_);

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
                                        inFILE_Delete((unsigned char *)_TMSIPDT_FILE_NAME_BAK_);

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

        /* 存組好的該TMSIPDTRecord 到 TMSIPDT.bak */
        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_TMSIPDT_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_TMSIPDT_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原TMSIPDT.dat Record 到 TMSIPDT.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
        /* inRecIndex要指到剩下還沒存進bak檔的第一個記憶體位置 */
        inTMSIPDTRec = inTMSIPDTRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < lnTMSIPDTLength; i++)
        {
                if (uszRead_Total_Buff[i] == 0x0A)
                {
                        j++;

                        if (j == inTMSIPDTRec)
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
                /* 接續存原TMSIPDT.dat的Record */
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
                                inFILE_Delete((unsigned char *)_TMSIPDT_FILE_NAME_BAK_);

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
                                inFILE_Delete((unsigned char *)_TMSIPDT_FILE_NAME_BAK_);

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

        /* 刪除原TMSIPDT.dat */
        if (inFILE_Delete((unsigned char *)_TMSIPDT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* 將TMSIPDT.bak改名字為TMSIPDT.dat取代原檔案 */
        if (inFILE_Rename((unsigned char *)_TMSIPDT_FILE_NAME_BAK_, (unsigned char *)_TMSIPDT_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* inSaveTMSIPDTRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSaveTMSIPDTRec_CTOS(%d) END!!", inTMSIPDTRec - 1);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return(VS_SUCCESS);
}

/*
Function        :inGetTMS_IP_Primary
Date&Time       :2024/4/2 下午 1:55
Describe        :
*/
int inGetTMS_IP_Primary(char* szTMS_IP_Primary)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMS_IP_Primary == NULL || strlen(srTMSIPDTRec.szTMS_IP_Primary) <= 0 || strlen(srTMSIPDTRec.szTMS_IP_Primary) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTMS_IP_Primary() ERROR !!");

			if (szTMS_IP_Primary == NULL)
                        {
                                inLogPrintf(AT, "szTMS_IP_Primary == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMS_IP_Primary length = (%d)", (int)strlen(srTMSIPDTRec.szTMS_IP_Primary));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMS_IP_Primary[0], &srTMSIPDTRec.szTMS_IP_Primary[0], strlen(srTMSIPDTRec.szTMS_IP_Primary));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMS_IP_Primary
Date&Time       :2017/12/19 下午 4:33
Describe        :
*/
int inSetTMS_IP_Primary(char* szTMS_IP_Primary)
{
        memset(srTMSIPDTRec.szTMS_IP_Primary, 0x00, sizeof(srTMSIPDTRec.szTMS_IP_Primary));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTMS_IP_Primary == NULL || strlen(szTMS_IP_Primary) <= 0 || strlen(szTMS_IP_Primary) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTMS_IP_Primary() ERROR !!");
                        if (szTMS_IP_Primary == NULL)
                        {
                                inLogPrintf(AT, "szTMS_IP_Primary == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMS_IP_Primary length = (%d)", (int)strlen(szTMS_IP_Primary));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTMSIPDTRec.szTMS_IP_Primary[0], &szTMS_IP_Primary[0], strlen(szTMS_IP_Primary));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMS_PortNo_Primary
Date&Time       :2024/4/2 下午 2:12
Describe        :
*/
int inGetTMS_PortNo_Primary(char* szTMS_PortNo_Primary)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMS_PortNo_Primary == NULL || strlen(srTMSIPDTRec.szTMS_PortNo_Primary) <= 0 || strlen(srTMSIPDTRec.szTMS_PortNo_Primary) > 5)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTMS_PortNo_Primary() ERROR !!");

			if (szTMS_PortNo_Primary == NULL)
                        {
                                inLogPrintf(AT, "szTMS_PortNo_Primary == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMS_PortNo_Primary length = (%d)", (int)strlen(srTMSIPDTRec.szTMS_PortNo_Primary));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMS_PortNo_Primary[0], &srTMSIPDTRec.szTMS_PortNo_Primary[0], strlen(srTMSIPDTRec.szTMS_PortNo_Primary));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMS_PortNo_Primary
Date&Time       :2024/4/2 下午 2:14
Describe        :
*/
int inSetTMS_PortNo_Primary(char* szTMS_PortNo_Primary)
{
        memset(srTMSIPDTRec.szTMS_PortNo_Primary, 0x00, sizeof(srTMSIPDTRec.szTMS_PortNo_Primary));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTMS_PortNo_Primary == NULL || strlen(szTMS_PortNo_Primary) <= 0 || strlen(szTMS_PortNo_Primary) > 5)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTMS_PortNo_Primary() ERROR !!");
                        if (szTMS_PortNo_Primary == NULL)
                        {
                                inLogPrintf(AT, "szTMS_PortNo_Primary == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMS_PortNo_Primary length = (%d)", (int)strlen(szTMS_PortNo_Primary));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTMSIPDTRec.szTMS_PortNo_Primary[0], &szTMS_PortNo_Primary[0], strlen(szTMS_PortNo_Primary));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMS_IP_Second
Date&Time       :2024/4/2 下午 2:14
Describe        :
*/
int inGetTMS_IP_Second(char* szTMS_IP_Second)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMS_IP_Second == NULL || strlen(srTMSIPDTRec.szTMS_IP_Second) <= 0 || strlen(srTMSIPDTRec.szTMS_IP_Second) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTMS_IP_Second() ERROR !!");

			if (szTMS_IP_Second == NULL)
                        {
                                inLogPrintf(AT, "szTMS_IP_Second == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMS_IP_Second length = (%d)", (int)strlen(srTMSIPDTRec.szTMS_IP_Second));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMS_IP_Second[0], &srTMSIPDTRec.szTMS_IP_Second[0], strlen(srTMSIPDTRec.szTMS_IP_Second));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMS_IP_Second
Date&Time       :2024/4/2 下午 2:15
Describe        :
*/
int inSetTMS_IP_Second(char* szTMS_IP_Second)
{
        memset(srTMSIPDTRec.szTMS_IP_Second, 0x00, sizeof(srTMSIPDTRec.szTMS_IP_Second));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTMS_IP_Second == NULL || strlen(szTMS_IP_Second) <= 0 || strlen(szTMS_IP_Second) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTMS_IP_Second() ERROR !!");
                        if (szTMS_IP_Second == NULL)
                        {
                                inLogPrintf(AT, "szTMS_IP_Second == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMS_IP_Second length = (%d)", (int)strlen(szTMS_IP_Second));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTMSIPDTRec.szTMS_IP_Second[0], &szTMS_IP_Second[0], strlen(szTMS_IP_Second));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMS_PortNo_Second
Date&Time       :2024/4/2 下午 2:15
Describe        :
*/
int inGetTMS_PortNo_Second(char* szTMS_PortNo_Second)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szTMS_PortNo_Second == NULL || strlen(srTMSIPDTRec.szTMS_PortNo_Second) <= 0 || strlen(srTMSIPDTRec.szTMS_PortNo_Second) > 5)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTMS_PortNo_Second() ERROR !!");

			if (szTMS_PortNo_Second == NULL)
                        {
                                inLogPrintf(AT, "szTMS_PortNo_Second == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMS_PortNo_Second length = (%d)", (int)strlen(srTMSIPDTRec.szTMS_PortNo_Second));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMS_PortNo_Second[0], &srTMSIPDTRec.szTMS_PortNo_Second[0], strlen(srTMSIPDTRec.szTMS_PortNo_Second));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMS_PortNo_Second
Date&Time       :2024/4/2 下午 2:16
Describe        :
*/
int inSetTMS_PortNo_Second(char* szTMS_PortNo_Second)
{
        memset(srTMSIPDTRec.szTMS_PortNo_Second, 0x00, sizeof(srTMSIPDTRec.szTMS_PortNo_Second));
        /* 傳進的指標 不得為空  傳進的值長度需大於0 小於規定最大值 */
        if (szTMS_PortNo_Second == NULL || strlen(szTMS_PortNo_Second) <= 0 || strlen(szTMS_PortNo_Second) > 5)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTMS_PortNo_Second() ERROR !!");
                        if (szTMS_PortNo_Second == NULL)
                        {
                                inLogPrintf(AT, "szTMS_PortNo_Second == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMS_PortNo_Second length = (%d)", (int)strlen(szTMS_PortNo_Second));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srTMSIPDTRec.szTMS_PortNo_Second[0], &szTMS_PortNo_Second[0], strlen(szTMS_PortNo_Second));

        return (VS_SUCCESS);
}

/*
Function        :inTMSIPDT_Edit_TMSIPDT_Table
Date&Time       :2024/4/2 下午 2:19
Describe        :
*/
int inTMSIPDT_Edit_TMSIPDT_Table(void)
{
	TABLE_GET_SET_TABLE TMSIPDT_FUNC_TABLE[] =
	{
		{"szTMS_IP_Primary"			,inGetTMS_IP_Primary			,inSetTMS_IP_Primary			},
		{"szTMS_PortNo_Primary"			,inGetTMS_PortNo_Primary		,inSetTMS_PortNo_Primary		},
		{"szTMS_IP_Second"			,inGetTMS_IP_Second			,inSetTMS_IP_Second			},
		{"szTMS_PortNo_Second"			,inGetTMS_PortNo_Second			,inSetTMS_PortNo_Second			},
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
	inDISP_ChineseFont_Color("是否更改TMSIPDT", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
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
	inLoadTMSIPDTRec(inRecordCnt);
	
	inFunc_Edit_Table_Tag(TMSIPDT_FUNC_TABLE);
	inSaveTMSIPDTRec(inRecordCnt);
	
	return	(VS_SUCCESS);
}