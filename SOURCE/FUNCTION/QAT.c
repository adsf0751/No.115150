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
#include "QAT.h"

static  QAT_REC srQATRec;	/* construct QAT record */
extern  int     ginDebug;       /* Debug使用 extern */

/*
Function        :inLoadQATRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀QAT檔案，inQATRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadQATRec(int inQATRec, unsigned char *uszFileName)
{
        unsigned long   ulFile_Handle;                          /* File Handle */
        unsigned char   *uszReadData;                           /* 放抓到的record */
        unsigned char   *uszTemp;                               /* 暫存，放整筆QAT檔案 */
        char            szQATRec[_SIZE_QAT_REC_ + 1];           /* 暫存, 放各個欄位檔案 */
        char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        long            lnQATLength = 0;                        /* QAT總長度 */
        long            lnReadLength;                           /* 記錄每次要從QAT.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0;	                /* inRec記錄讀到第幾筆, i為目前從QAT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;                    /* 判斷有沒有讀到0x0D 0x0A的Flag */
       
        /* inLoadQATRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadQATRec(%d) START!!", inQATRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 判斷傳進來的inQATRec是否小於零 大於等於零才是正確值(防呆) */
        if (inQATRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inQATRec < 0:(index = %d) ERROR!!", inQATRec);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open QAT.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, uszFileName) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnQATLength = lnFILE_GetSize(&ulFile_Handle, uszFileName);
        
        if (lnQATLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnQATLength + 1);
        uszTemp = malloc(lnQATLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnQATLength + 1);
        memset(uszTemp, 0x00, lnQATLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnQATLength;

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
         *i為目前從QAT讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnQATLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record或讀到QAT的結尾時  */
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
                        /* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
                        inSearchResult = 1;

                        /* 清空uszReadData */
                        memset(uszReadData, 0x00, lnQATLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[0], &uszTemp[i-j], j);
                        inRec++;
                        /* 因為inQAT_Rec的index從0開始，所以inQAT_Rec要+1 */
                        if (inRec == (inQATRec + 1))
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
         * 如果沒有inQATRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inQATRec + 1) || inSearchResult == -1)
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
        memset(&srQATRec, 0x00, sizeof(srQATRec));
        /*
         * 以下pattern為存入QAT_Rec
         * i為QAT的第幾個字元
         * 存入QAT_Rec
         */
        i = 0;


        /* 01_SmartPay應用程式ID */
        /* 初始化 */
        memset(szQATRec, 0x00, sizeof(szQATRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szQATRec[k ++] = uszReadData[i ++];
                if (szQATRec[k - 1] == 0x2C	||
		    szQATRec[k - 1] == 0x0D	||
		    szQATRec[k - 1] == 0x0A	||
		    szQATRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnQATLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "QAT unpack ERROR.");
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
        if (szQATRec[0] != 0x2C	&&
	    szQATRec[0] != 0x0D	&&
	    szQATRec[0] != 0x0A	&&
	    szQATRec[0] != 0x00)
        {
                memcpy(&srQATRec.szQuestionIndex[0], &szQATRec[0], k - 1);
        }

        /* 02_SmartPay感應卡驗證卡人身分限額(不含) */
        /* 初始化 */
        memset(szQATRec, 0x00, sizeof(szQATRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szQATRec[k ++] = uszReadData[i ++];
                if (szQATRec[k - 1] == 0x2C	||
		    szQATRec[k - 1] == 0x0D	||
		    szQATRec[k - 1] == 0x0A	||
		    szQATRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnQATLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "QAT unpack ERROR");
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
        if (szQATRec[0] != 0x2C	&&
	    szQATRec[0] != 0x0D	&&
	    szQATRec[0] != 0x0A	&&
	    szQATRec[0] != 0x00)
        {
                memcpy(&srQATRec.szQADBIndex[0], &szQATRec[0], k - 1);
        }

        /* 03_SmartPay感應卡離線授權限額(不含) */
        /* 初始化 */
        memset(szQATRec, 0x00, sizeof(szQATRec));
        k = 0;

        /* 從Record中讀欄位資料出來 */
        while (1)
        {
                szQATRec[k ++] = uszReadData[i ++];
                if (szQATRec[k - 1] == 0x2C	||
		    szQATRec[k - 1] == 0x0D	||
		    szQATRec[k - 1] == 0x0A	||
		    szQATRec[k - 1] == 0x00)
                {
                        break;
                }

                if (i > lnQATLength)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "QAT unpack ERROR");
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
        if (szQATRec[0] != 0x2C	&&
	    szQATRec[0] != 0x0D	&&
	    szQATRec[0] != 0x0A	&&
	    szQATRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srQATRec.szQuestion[0], &szQATRec[0], k - 1);
        }

        /* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadQATRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadQATRec(%d) END!!", inQATRec);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
set和get等價於相反的操作
各欄位的set和get function
*/

/*
Function        :inGetQuestionIndex
Date&Time       :
Describe        :
*/
int inGetQuestionIndex(char* szQuestionIndex)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szQuestionIndex == NULL || strlen(srQATRec.szQuestionIndex) <= 0 || strlen(srQATRec.szQuestionIndex) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetQuestionIndex() ERROR !!");
                        
			if (szQuestionIndex == NULL)
                        {
                                inLogPrintf(AT, "szQuestionIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szQuestionIndex length = (%d)", (int)strlen(srQATRec.szQuestionIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szQuestionIndex[0], &srQATRec.szQuestionIndex[0], strlen(srQATRec.szQuestionIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetQADBIndex
Date&Time       :
Describe        :
*/
int inGetQADBIndex(char* szQADBIndex)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szQADBIndex == NULL || strlen(srQATRec.szQADBIndex) <= 0 || strlen(srQATRec.szQADBIndex) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetQADBIndex() ERROR !!");

                        if (szQADBIndex == NULL)
                        {
                                inLogPrintf(AT, "szQADBIndex == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szQADBIndex length = (%d)", (int)strlen(srQATRec.szQADBIndex));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szQADBIndex[0], &srQATRec.szQADBIndex[0], strlen(srQATRec.szQADBIndex));

        return (VS_SUCCESS);
}

/*
Function        :inGetQuestion
Date&Time       :
Describe        :
*/
int inGetQuestion(char* szQuestion)
{
        /* 傳進的指標 不得為空  Rec中的值長度須為正數 小於規定最大值 */
        if (szQuestion == NULL || strlen(srQATRec.szQuestion) <= 0 || strlen(srQATRec.szQuestion) > 80)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetQuestion() ERROR !!");

                        if (szQuestion == NULL)
                        {
                                inLogPrintf(AT, "szQuestion == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szQuestion length = (%d)", (int)strlen(srQATRec.szQuestion));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szQuestion[0], &srQATRec.szQuestion[0], strlen(srQATRec.szQuestion));

        return (VS_SUCCESS);
}
