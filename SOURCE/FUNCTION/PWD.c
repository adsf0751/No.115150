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
#include "PWD.h"

static 	PWD_REC srPWDRec;	/* construct PWD record */
extern  int     ginDebug;  	/* Debug使用 extern */

/*
Function        :inLoadPWDRec
Date&Time       :2015/9/2 下午 3:00
Describe        :讀PWD檔案，inPWDRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadPWDRec(int inPWDRec)
{
        unsigned long   ulFile_Handle;          		/* File Handle */
        unsigned char   *uszReadData;           		/* 放抓到的record */
        unsigned char   *uszTemp;				/* 暫存，放整筆PWD檔案 */
        char            szPWDRec[_SIZE_PWD_REC_ + 1];		/* 暫存, 放各個欄位檔案 */
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1]; 	/* debug message */
        long            lnPWDLength = 0;        		/* PWD總長度 */
        long            lnReadLength;           		/* 記錄每次要從PWD.dat讀多長的資料 */
        int             i, k, j = 0, inRec = 0; 		/* inRec記錄讀到第幾筆, i為目前從PWD讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
        int             inSearchResult = -1;    		/* 判斷有沒有讀到0x0D 0x0A的Flag */

        /* inLoadPWDRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadPWDRec(%d) START!!", inPWDRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 判斷傳進來的inPWDRec是否小於零 大於等於零才是正確值(防呆) */
        if (inPWDRec < 0)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "inPWDRec < 0:(index = %d) ERROR!!", inPWDRec);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_ERROR);
        }

        /*
         * open PWD.dat file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)_PWD_FILE_NAME_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnPWDLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_PWD_FILE_NAME_);
        
	if (lnPWDLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnPWDLength + 1);
        uszTemp = malloc(lnPWDLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnPWDLength + 1);
        memset(uszTemp, 0x00, lnPWDLength + 1);

        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnPWDLength;

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
         *i為目前從PWD讀到的第幾個字元
         *j為該record的長度
         */
        j = 0;
        for (i = 0; i <= lnPWDLength; ++i)      /* "<=" 是為了抓到最後一個0x00 */
        {
                if ((uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A) || uszTemp[i] == 0x00)
                {
			/* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
			inSearchResult = 1;

                        /* 清空uszReadData */
			memset(uszReadData, 0x00, lnPWDLength + 1);
                        /* 把record從temp指定的位置截取出來放到uszReadData */
			memcpy(&uszReadData[0], &uszTemp[i-j], j);
			inRec++;
                        /* 因為inPWD_Rec的index從0開始，所以inPWD_Rec要+1 */
			if (inRec == (inPWDRec + 1))
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
         * 如果沒有inPWDRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
         * 關檔、釋放記憶體並return VS_ERROR
         * 如果總record數量小於要存取Record的Index
         * 特例：有可能會遇到全文都沒有0x0D 0x0A
         */
        if (inRec < (inPWDRec + 1) || inSearchResult == -1)
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
	memset(&srPWDRec, 0x00, sizeof(srPWDRec));
	/*
	 * 以下pattern為存入PWD_Rec
	 * i為PWD的第幾個字元
	 * 存入PWD_Rec
	 */
	i = 0;


	/* 01_開啟管理者號碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szInitialManagerEnable[0], &szPWDRec[0], k - 1);
	}

	/* 02_管理者號碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szInitialManagerPwd[0], &szPWDRec[0], k - 1);
	}

	/* 03_管理開機號碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szRebootPwdEnale[0], &szPWDRec[0], k - 1);
	}

	/* 04_開機號碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szRebootPwd[0], &szPWDRec[0], k - 1);
	}

	/* 05_開啟銷售密碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szSalePwdEnable[0], &szPWDRec[0], k - 1);
	}

	/* 06_銷售密碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szSalePwd[0], &szPWDRec[0], k - 1);
	}

	/* 07_開啟預先授權密碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szPreauthPwdEnable[0], &szPWDRec[0], k - 1);
	}

	/* 08_預先授權密碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szPreauthPwd[0], &szPWDRec[0], k - 1);
	}

	/* 09_開啟分期付款密碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szInstallmentPwdEnable[0], &szPWDRec[0], k - 1);
	}

	/* 10_分期付款密碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szInstallmentPwd[0], &szPWDRec[0], k - 1);
	}

	/* 11_開啟紅利折扣密碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szRedeemPwdEnable[0], &szPWDRec[0], k - 1);
	}

	/* 12_紅利折扣密碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szRedeemPwd[0], &szPWDRec[0], k - 1);
	}

	/* 13_開啟郵購密碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szMailOrderPwdEnable[0], &szPWDRec[0], k - 1);
	}

	/* 14_郵購密碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szMailOrderPwd[0], &szPWDRec[0], k - 1);
	}

	/* 15_開啟交易補登密碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szOfflinePwdEnable[0], &szPWDRec[0], k - 1);
	}

	/* 16_交易補登密碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szOfflinePwd[0], &szPWDRec[0], k - 1);
	}

	/* 17_開啟分期付款調帳密碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szInstallmentAdjustPwdEnable[0], &szPWDRec[0], k - 1);
	}

	/* 18_分期付款調帳密碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szInstallmentAdjustPwd[0], &szPWDRec[0], k - 1);
	}

	/* 19_開啟紅利扣抵調帳密碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szRedeemAdjustPwdEnable[0], &szPWDRec[0], k - 1);
	}

	/* 20_紅利扣抵調帳密碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szRedeemAdjustPwd[0], &szPWDRec[0], k - 1);
	}

	/* 21_開啟取消密碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szVoidPwdEnable[0], &szPWDRec[0], k - 1);
	}

	/* 22_取消密碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szVoidPwd[0], &szPWDRec[0], k - 1);
	}

	/* 23_開啟結帳密碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szSettlementPwdEnable[0], &szPWDRec[0], k - 1);
	}

	/* 24_結帳密碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szSettlementPwd[0], &szPWDRec[0], k - 1);
	}

	/* 25_開啟退貨密碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szRefundPwdEnable[0], &szPWDRec[0], k - 1);
	}

	/* 26_退貨密碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szRefundPwd[0], &szPWDRec[0], k - 1);
	}

	/* 27_開啟HappyGo退貨密碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szHGRefundPwdEnable[0], &szPWDRec[0], k - 1);
	}

	/* 28_HappyGo退貨密碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szHGRefundPwd[0], &szPWDRec[0], k - 1);
	}
	
	/* 29_開啟授權碼補登管理號碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
	if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)
	{
		memcpy(&srPWDRec.szCallBankForcePwdEnable[0], &szPWDRec[0], k - 1);
	}

	/* 30.授權碼補登管理號碼 */
	/* 初始化 */
	memset(szPWDRec, 0x00, sizeof(szPWDRec));
	k = 0;
	while (1)
	{
		szPWDRec[k++] = uszReadData[i++];
		if (szPWDRec[k - 1] == 0x2C	||
		    szPWDRec[k - 1] == 0x0D	||
		    szPWDRec[k - 1] == 0x0A	||
		    szPWDRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnPWDLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "PWD unpack ERROR");
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
        if (szPWDRec[0] != 0x2C	&&
	    szPWDRec[0] != 0x0D	&&
	    szPWDRec[0] != 0x0A	&&
	    szPWDRec[0] != 0x00)  /* 因為是最後的欄位還要多判斷0x00 */
        {
                memcpy(&srPWDRec.szCallBankForcePwd[0], &szPWDRec[0], k - 1);
        }

        /* release */
	/* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszReadData);
        free(uszTemp);

        /* inLoadPWDRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inLoadPWDRec(%d) END!!", inPWDRec);
                inLogPrintf(AT, szErrorMsg);
                inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inSavePWDRec
Date&Time       :2015/9/2 下午 3:00
Describe        :
*/
int inSavePWDRec (int inPWDRec)
{
        unsigned long   uldat_Handle;   			/* FILE Handle */
        unsigned long   ulbak_Handle;   			/* FILE Handle */
        int     	inPackCount = 0;        		/* uszWriteBuff_Record的index */
        int     	inRecIndex = 0;         		/* uszRead_Total_Buff的index */
        int     	inPWD_Total_Rec = 0;   			/* PWD.dat的總筆數 */
        int     	inRetVal;
        int     	i = 0, j = 0;
        char    	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];	/* debug message */
        long    	lnReadLength = 0;       		/* 每次要從PWD.dat讀多少byte出來 */
        long    	lnPWDLength = 0;         		/* PWD.dat檔案總長度 */
        unsigned char   *uszRead_Total_Buff;
        unsigned char   *uszWriteBuff_Record, *uszWriteBuff_Org;


        /* inSavePWDRec()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSavePWDRec(%d) START!!", inPWDRec);
                inLogPrintf(AT, szErrorMsg);
        }

        /* 防呆先刪除PWD.bak  */
        inFILE_Delete((unsigned char *)_PWD_FILE_NAME_BAK_);

        /* 新建PWD.bak檔案 */
        inRetVal = inFILE_Create(&ulbak_Handle, (unsigned char *)_PWD_FILE_NAME_BAK_);

        if (inRetVal != VS_SUCCESS)
        {
                inFILE_Close(&ulbak_Handle);

                return (VS_ERROR);
        }

        /* 開啟原檔案PWD.dat */
        inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_PWD_FILE_NAME_);

        if (inRetVal != VS_SUCCESS)
        {
                /* PWD.dat開檔失敗 ，不用關檔PWD.dat */
                /* PWD.bak仍要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_PWD_FILE_NAME_BAK_);

                return(VS_ERROR);
        }

        /* 取得PWD.dat檔案大小 */
        lnPWDLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *)_PWD_FILE_NAME_);

        /* 取得檔案大小失敗 */
        if (lnPWDLength == VS_ERROR)
        {
                /* PWD.bak和PWD.dat要關並刪除 */
                inFILE_Close(&ulbak_Handle);
                inFILE_Close(&uldat_Handle);
                inFILE_Delete((unsigned char *)_PWD_FILE_NAME_BAK_);

                return (VS_ERROR);
        }

        /* 組Write Record封包 */
        /* 給WriteBuff記憶體大小 */
        uszWriteBuff_Record = malloc(_SIZE_PWD_REC_ + _SIZE_PWD_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_PWD_REC_ + _SIZE_PWD_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* 存原PWD.dat的每筆Record 初始化 */
        uszWriteBuff_Org = malloc(_SIZE_PWD_REC_ + _SIZE_PWD_COMMA_0D0A_);
        memset(uszWriteBuff_Org, 0x00, _SIZE_PWD_REC_ + _SIZE_PWD_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

        /* uszRead_Total_Buff儲存PWD.dat全部資料 */
        uszRead_Total_Buff = malloc(lnPWDLength + 1);
        memset(uszRead_Total_Buff, 0x00, lnPWDLength + 1);

        inPackCount = 0; /* uszWriteBuff_Record的index位置 */
        /* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

        /* InitialManagerEnable */
        memcpy(&uszWriteBuff_Record[0], &srPWDRec.szInitialManagerEnable[0], strlen(srPWDRec.szInitialManagerEnable));
        inPackCount += strlen(srPWDRec.szInitialManagerEnable);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* InitialManagerPwd */
        memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szInitialManagerPwd[0], strlen(srPWDRec.szInitialManagerPwd));
        inPackCount += strlen(srPWDRec.szInitialManagerPwd);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* RebootPwdEnale */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szRebootPwdEnale[0], strlen(srPWDRec.szRebootPwdEnale));
        inPackCount += strlen(srPWDRec.szRebootPwdEnale);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* RebootPwd */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szRebootPwd[0], strlen(srPWDRec.szRebootPwd));
        inPackCount += strlen(srPWDRec.szRebootPwd);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* SalePwdEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szSalePwdEnable[0], strlen(srPWDRec.szSalePwdEnable));
        inPackCount += strlen(srPWDRec.szSalePwdEnable);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* SalePwd */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szSalePwd[0], strlen(srPWDRec.szSalePwd));
        inPackCount += strlen(srPWDRec.szSalePwd);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* PreauthPwdEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szPreauthPwdEnable[0], strlen(srPWDRec.szPreauthPwdEnable));
        inPackCount += strlen(srPWDRec.szPreauthPwdEnable);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* PreauthPwd */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szPreauthPwd[0], strlen(srPWDRec.szPreauthPwd));
        inPackCount += strlen(srPWDRec.szPreauthPwd);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* InstallmentPwdEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szInstallmentPwdEnable[0], strlen(srPWDRec.szInstallmentPwdEnable));
        inPackCount += strlen(srPWDRec.szInstallmentPwdEnable);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* InstallmentPwd */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szInstallmentPwd[0], strlen(srPWDRec.szInstallmentPwd));
        inPackCount += strlen(srPWDRec.szInstallmentPwd);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* RedeemPwdEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szRedeemPwdEnable[0], strlen(srPWDRec.szRedeemPwdEnable));
        inPackCount += strlen(srPWDRec.szRedeemPwdEnable);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* RedeemPwd */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szRedeemPwd[0], strlen(srPWDRec.szRedeemPwd));
        inPackCount += strlen(srPWDRec.szRedeemPwd);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* MailOrderPwdEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szMailOrderPwdEnable[0], strlen(srPWDRec.szMailOrderPwdEnable));
        inPackCount += strlen(srPWDRec.szMailOrderPwdEnable);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* MailOrderPwd */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szMailOrderPwd[0], strlen(srPWDRec.szMailOrderPwd));
        inPackCount += strlen(srPWDRec.szMailOrderPwd);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* OfflinePwdEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szOfflinePwdEnable[0], strlen(srPWDRec.szOfflinePwdEnable));
        inPackCount += strlen(srPWDRec.szOfflinePwdEnable);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* OfflinePwd */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szOfflinePwd[0], strlen(srPWDRec.szOfflinePwd));
        inPackCount += strlen(srPWDRec.szOfflinePwd);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* InstallmentAdjustPwdEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szInstallmentAdjustPwdEnable[0], strlen(srPWDRec.szInstallmentAdjustPwdEnable));
        inPackCount += strlen(srPWDRec.szInstallmentAdjustPwdEnable);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* InstallmentAdjustPwd */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szInstallmentAdjustPwd[0], strlen(srPWDRec.szInstallmentAdjustPwd));
        inPackCount += strlen(srPWDRec.szInstallmentAdjustPwd);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* RedeemAdjustPwdEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szRedeemAdjustPwdEnable[0], strlen(srPWDRec.szRedeemAdjustPwdEnable));
        inPackCount += strlen(srPWDRec.szRedeemAdjustPwdEnable);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* RedeemAdjustPwd */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szRedeemAdjustPwd[0], strlen(srPWDRec.szRedeemAdjustPwd));
        inPackCount += strlen(srPWDRec.szRedeemAdjustPwd);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* VoidPwdEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szVoidPwdEnable[0], strlen(srPWDRec.szVoidPwdEnable));
        inPackCount += strlen(srPWDRec.szVoidPwdEnable);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* VoidPwd */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szVoidPwd[0], strlen(srPWDRec.szVoidPwd));
        inPackCount += strlen(srPWDRec.szVoidPwd);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* SettlementPwdEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szSettlementPwdEnable[0], strlen(srPWDRec.szSettlementPwdEnable));
        inPackCount += strlen(srPWDRec.szSettlementPwdEnable);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* SettlementPwd */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szSettlementPwd[0], strlen(srPWDRec.szSettlementPwd));
        inPackCount += strlen(srPWDRec.szSettlementPwd);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* RefundPwdEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szRefundPwdEnable[0], strlen(srPWDRec.szRefundPwdEnable));
        inPackCount += strlen(srPWDRec.szRefundPwdEnable);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* RefundPwd */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szRefundPwd[0], strlen(srPWDRec.szRefundPwd));
        inPackCount += strlen(srPWDRec.szRefundPwd);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* HGRefundPwdEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szHGRefundPwdEnable[0], strlen(srPWDRec.szHGRefundPwdEnable));
        inPackCount += strlen(srPWDRec.szHGRefundPwdEnable);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* HGRefundPwd */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szHGRefundPwd[0], strlen(srPWDRec.szHGRefundPwd));
        inPackCount += strlen(srPWDRec.szHGRefundPwd);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
        /* CallBankForcePwdEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szCallBankForcePwdEnable[0], strlen(srPWDRec.szCallBankForcePwdEnable));
        inPackCount += strlen(srPWDRec.szCallBankForcePwdEnable);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

        /* CallBankForcePwd */
	memcpy(&uszWriteBuff_Record[inPackCount], &srPWDRec.szCallBankForcePwd[0], strlen(srPWDRec.szCallBankForcePwd));
        inPackCount += strlen(srPWDRec.szCallBankForcePwd);

        /* 最後的data不用逗號 */
        /* 補上換行符號 */

        /* 0D是移至行首 */
        uszWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        uszWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;

        /* 讀PWD.dat檔案存到uszRead_Total_Buff */
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnPWDLength;

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
			                 inFILE_Delete((unsigned char *)_PWD_FILE_NAME_BAK_);

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
			                 inFILE_Delete((unsigned char *)_PWD_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_PWD_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

        /* 防呆 總record數量小於要存取inPWDRec Return ERROR */
        /* 算總Record數 */
        for (i = 0; i < (lnPWDLength + 1); i++)
        {
                /* 一個換行，record數就+1 */
                if (uszRead_Total_Buff[i] == 0x0A)
			 inPWD_Total_Rec++;
        }

        /* 防呆 總record數量小於要存取inPWDRec Return ERROR */
        if ((inPWDRec + 1) > inPWD_Total_Rec)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "No data or Index ERROR");
                }

                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_PWD_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszRead_Total_Buff);
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);

                return (VS_ERROR);
        }

        /* 記錄ReadTotalBuffer的陣列位置 */
        inRecIndex = 0;
        /* 用inPWDRec決定要先存幾筆Record到PWD.bak，ex:inPWDRec = 4，就會先存0~3的record進去bak檔 */
        for (i = 0; i < inPWDRec; i ++)
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
			                 inFILE_Delete((unsigned char *)_PWD_FILE_NAME_BAK_);

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
			                 inFILE_Delete((unsigned char *)_PWD_FILE_NAME_BAK_);

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

        /* 存組好的該PWDRecord 到 PWD.bak */
        inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

        if (inRetVal != VS_SUCCESS)
        {
                /* 關檔 */
                inFILE_Close(&uldat_Handle);
                inFILE_Close(&ulbak_Handle);
                inFILE_Delete((unsigned char *)_PWD_FILE_NAME_BAK_);

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
                inFILE_Delete((unsigned char *)_PWD_FILE_NAME_BAK_);

                /* Free pointer */
                free(uszWriteBuff_Record);
                free(uszWriteBuff_Org);
                free(uszRead_Total_Buff);

                return (VS_ERROR);
        }

        /* 存剩下的原PWD.dat Record 到 PWD.bak */
        /* 計算已存幾筆Record 移動到正確陣列位置 */
        inPWDRec = inPWDRec + 1;
        j = 0;
        inRecIndex = 0;

        for (i = 0; i < lnPWDLength; i++)
        {
                if (uszRead_Total_Buff[i] == 0x0A)
                {
			 j++;

			 if (j == inPWDRec)
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
                /* 接續存原PWD.dat的Record */
                uszWriteBuff_Org[i] = uszRead_Total_Buff[inRecIndex];
                i++;

                if (uszRead_Total_Buff[inRecIndex] == 0X0A)
                {
                        /* 移動到bak的檔案結尾 */
			 inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

			 if (inRetVal != VS_SUCCESS)
			 {
                                /* 關檔 */
			         inFILE_Close(&uldat_Handle);
			         inFILE_Close(&ulbak_Handle);
			         inFILE_Delete((unsigned char *)_PWD_FILE_NAME_BAK_);

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
			         inFILE_Delete((unsigned char *)_PWD_FILE_NAME_BAK_);

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

        /* 刪除原PWD.dat */
	if (inFILE_Delete((unsigned char *)_PWD_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* 將PWD.bak改名字為PWD.dat取代原檔案 */
        if (inFILE_Rename((unsigned char *)_PWD_FILE_NAME_BAK_, (unsigned char *)_PWD_FILE_NAME_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        /* inSavePWDRec() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inSavePWDRec(%d) END!!", inPWDRec - 1);
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
Function        :inGetInitialManagerEnable
Date&Time       :
Describe        :
*/
int inGetInitialManagerEnable(char* szInitialManagerEnable)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szInitialManagerEnable == NULL || strlen(srPWDRec.szInitialManagerEnable) <= 0 || strlen(srPWDRec.szInitialManagerEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetInitialManagerEnable() ERROR !!");

                	if (szInitialManagerEnable == NULL)
                	{
                	        inLogPrintf(AT, "szInitialManagerEnable == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szInitialManagerEnable length = (%d)", (int)strlen(srPWDRec.szInitialManagerEnable));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szInitialManagerEnable[0], &srPWDRec.szInitialManagerEnable[0], strlen(srPWDRec.szInitialManagerEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetInitialManagerEnable
Date&Time       :
Describe        :
*/
int inSetInitialManagerEnable(char* szInitialManagerEnable)
{
        memset(srPWDRec.szInitialManagerEnable, 0x00, sizeof(srPWDRec.szInitialManagerEnable));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szInitialManagerEnable == NULL || strlen(szInitialManagerEnable) <= 0 || strlen(szInitialManagerEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inSetInitialManagerEnable() ERROR !!");

                	if (szInitialManagerEnable == NULL)
                	{
                	        inLogPrintf(AT, "szInitialManagerEnable == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szInitialManagerEnable length = (%d)", (int)strlen(srPWDRec.szInitialManagerEnable));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szInitialManagerEnable[0], &szInitialManagerEnable[0], strlen(szInitialManagerEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetInitialManagerPwd
Date&Time       :
Describe        :
*/
int inGetInitialManagerPwd(char* szInitialManagerPwd)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szInitialManagerPwd == NULL || strlen(srPWDRec.szInitialManagerPwd) <= 0 || strlen(srPWDRec.szInitialManagerPwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetInitialManagerPwd() ERROR !!");

                	if (szInitialManagerPwd == NULL)
                	{
                	        inLogPrintf(AT, "szInitialManagerPwd == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szInitialManagerPwd length = (%d)", (int)strlen(srPWDRec.szInitialManagerPwd));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szInitialManagerPwd[0], &srPWDRec.szInitialManagerPwd[0], strlen(srPWDRec.szInitialManagerPwd));

        return (VS_SUCCESS);
}

/*
Function        :inSetInitialManagerPwd
Date&Time       :
Describe        :
*/
int inSetInitialManagerPwd(char* szInitialManagerPwd)
{
        memset(srPWDRec.szInitialManagerPwd, 0x00, sizeof(srPWDRec.szInitialManagerPwd));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szInitialManagerPwd == NULL || strlen(szInitialManagerPwd) <= 0 || strlen(szInitialManagerPwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

		        inLogPrintf(AT, "inSetInitialManagerPwd() ERROR !!");

			if (szInitialManagerPwd == NULL)
			{
			        inLogPrintf(AT, "szInitialManagerPwd == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szInitialManagerPwd length = (%d)", (int)strlen(szInitialManagerPwd));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szInitialManagerPwd[0], &szInitialManagerPwd[0], strlen(szInitialManagerPwd));

        return (VS_SUCCESS);
}

/*
Function        :inGetRebootPwdEnale
Date&Time       :
Describe        :
*/
int inGetRebootPwdEnale(char* szRebootPwdEnale)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szRebootPwdEnale == NULL || strlen(srPWDRec.szRebootPwdEnale) <= 0 || strlen(srPWDRec.szRebootPwdEnale) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetRebootPwdEnale() ERROR !!");

                	if (szRebootPwdEnale == NULL)
                	{
                	        inLogPrintf(AT, "szRebootPwdEnale == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szRebootPwdEnale length = (%d)", (int)strlen(srPWDRec.szRebootPwdEnale));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szRebootPwdEnale[0], &srPWDRec.szRebootPwdEnale[0], strlen(srPWDRec.szRebootPwdEnale));

        return (VS_SUCCESS);
}

/*
Function        :inSetRebootPwdEnale
Date&Time       :
Describe        :
*/
int inSetRebootPwdEnale(char* szRebootPwdEnale)
{
        memset(srPWDRec.szRebootPwdEnale, 0x00, sizeof(srPWDRec.szRebootPwdEnale));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szRebootPwdEnale == NULL || strlen(szRebootPwdEnale) <= 0 || strlen(szRebootPwdEnale) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetRebootPwdEnale() ERROR !!");

			if (szRebootPwdEnale == NULL)
			{
			        inLogPrintf(AT, "szRebootPwdEnale == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szRebootPwdEnale length = (%d)", (int)strlen(szRebootPwdEnale));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szRebootPwdEnale[0], &szRebootPwdEnale[0], strlen(szRebootPwdEnale));

        return (VS_SUCCESS);
}

/*
Function        :inGetRebootPwd
Date&Time       :
Describe        :
*/
int inGetRebootPwd(char* szRebootPwd)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szRebootPwd == NULL || strlen(srPWDRec.szRebootPwd) <= 0 || strlen(srPWDRec.szRebootPwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetRebootPwd() ERROR !!");

                	if (szRebootPwd == NULL)
                	{
                	        inLogPrintf(AT, "szRebootPwd == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szRebootPwd length = (%d)", (int)strlen(srPWDRec.szRebootPwd));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szRebootPwd[0], &srPWDRec.szRebootPwd[0], strlen(srPWDRec.szRebootPwd));

        return (VS_SUCCESS);
}

/*
Function        :inSetRebootPwd
Date&Time       :
Describe        :
*/
int inSetRebootPwd(char* szRebootPwd)
{
        memset(srPWDRec.szRebootPwd, 0x00, sizeof(srPWDRec.szRebootPwd));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szRebootPwd == NULL || strlen(szRebootPwd) <= 0 || strlen(szRebootPwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetRebootPwd() ERROR !!");

			if (szRebootPwd == NULL)
			{
			        inLogPrintf(AT, "szRebootPwd == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szRebootPwd length = (%d)", (int)strlen(szRebootPwd));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szRebootPwd[0], &szRebootPwd[0], strlen(szRebootPwd));

        return (VS_SUCCESS);
}

/*
Function        :inGetSalePwdEnable
Date&Time       :
Describe        :
*/
int inGetSalePwdEnable(char* szSalePwdEnable)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSalePwdEnable == NULL || strlen(srPWDRec.szSalePwdEnable) <= 0 || strlen(srPWDRec.szSalePwdEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetSalePwdEnable() ERROR !!");

                	if (szSalePwdEnable == NULL)
                	{
                	        inLogPrintf(AT, "szSalePwdEnable == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szSalePwdEnable length = (%d)", (int)strlen(srPWDRec.szSalePwdEnable));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szSalePwdEnable[0], &srPWDRec.szSalePwdEnable[0], strlen(srPWDRec.szSalePwdEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetSalePwdEnable
Date&Time       :
Describe        :
*/
int inSetSalePwdEnable(char* szSalePwdEnable)
{
        memset(srPWDRec.szSalePwdEnable, 0x00, sizeof(srPWDRec.szSalePwdEnable));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSalePwdEnable == NULL || strlen(szSalePwdEnable) <= 0 || strlen(szSalePwdEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetSalePwdEnable() ERROR !!");

			if (szSalePwdEnable == NULL)
			{
			        inLogPrintf(AT, "szSalePwdEnable == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szSalePwdEnable length = (%d)", (int)strlen(szSalePwdEnable));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szSalePwdEnable[0], &szSalePwdEnable[0], strlen(szSalePwdEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetSalePwd
Date&Time       :
Describe        :
*/
int inGetSalePwd(char* szSalePwd)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSalePwd == NULL || strlen(srPWDRec.szSalePwd) <= 0 || strlen(srPWDRec.szSalePwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetSalePwd() ERROR !!");

                	if (szSalePwd == NULL)
                	{
                	        inLogPrintf(AT, "szSalePwd == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szSalePwd length = (%d)", (int)strlen(srPWDRec.szSalePwd));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szSalePwd[0], &srPWDRec.szSalePwd[0], strlen(srPWDRec.szSalePwd));

        return (VS_SUCCESS);
}

/*
Function        :inSetSalePwd
Date&Time       :
Describe        :
*/
int inSetSalePwd(char* szSalePwd)
{
        memset(srPWDRec.szSalePwd, 0x00, sizeof(srPWDRec.szSalePwd));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSalePwd == NULL || strlen(szSalePwd) <= 0 || strlen(szSalePwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetSalePwd() ERROR !!");

			if (szSalePwd == NULL)
			{
			        inLogPrintf(AT, "szSalePwd == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szSalePwd length = (%d)", (int)strlen(szSalePwd));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szSalePwd[0], &szSalePwd[0], strlen(szSalePwd));

        return (VS_SUCCESS);
}

/*
Function        :inGetPreauthPwdEnable
Date&Time       :
Describe        :
*/
int inGetPreauthPwdEnable(char* szPreauthPwdEnable)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPreauthPwdEnable == NULL || strlen(srPWDRec.szPreauthPwdEnable) <= 0 || strlen(srPWDRec.szPreauthPwdEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetPreauthPwdEnable() ERROR !!");

                	if (szPreauthPwdEnable == NULL)
                	{
                	        inLogPrintf(AT, "szPreauthPwdEnable == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szPreauthPwdEnable length = (%d)", (int)strlen(srPWDRec.szPreauthPwdEnable));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szPreauthPwdEnable[0], &srPWDRec.szPreauthPwdEnable[0], strlen(srPWDRec.szPreauthPwdEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetPreauthPwdEnable
Date&Time       :
Describe        :
*/
int inSetPreauthPwdEnable(char* szPreauthPwdEnable)
{
        memset(srPWDRec.szPreauthPwdEnable, 0x00, sizeof(srPWDRec.szPreauthPwdEnable));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPreauthPwdEnable == NULL || strlen(szPreauthPwdEnable) <= 0 || strlen(szPreauthPwdEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetPreauthPwdEnable() ERROR !!");

			if (szPreauthPwdEnable == NULL)
			{
			        inLogPrintf(AT, "szPreauthPwdEnable == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szPreauthPwdEnable length = (%d)", (int)strlen(szPreauthPwdEnable));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szPreauthPwdEnable[0], &szPreauthPwdEnable[0], strlen(szPreauthPwdEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetPreauthPwd
Date&Time       :
Describe        :
*/
int inGetPreauthPwd(char* szPreauthPwd)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPreauthPwd == NULL || strlen(srPWDRec.szPreauthPwd) <= 0 || strlen(srPWDRec.szPreauthPwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetPreauthPwd() ERROR !!");

                	if (szPreauthPwd == NULL)
                	{
                	        inLogPrintf(AT, "szPreauthPwd == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szPreauthPwd length = (%d)", (int)strlen(srPWDRec.szPreauthPwd));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szPreauthPwd[0], &srPWDRec.szPreauthPwd[0], strlen(srPWDRec.szPreauthPwd));

        return (VS_SUCCESS);
}

/*
Function        :inSetPreauthPwd
Date&Time       :
Describe        :
*/
int inSetPreauthPwd(char* szPreauthPwd)
{
        memset(srPWDRec.szPreauthPwd, 0x00, sizeof(srPWDRec.szPreauthPwd));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szPreauthPwd == NULL || strlen(szPreauthPwd) <= 0 || strlen(szPreauthPwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetPreauthPwd() ERROR !!");

			if (szPreauthPwd == NULL)
			{
			        inLogPrintf(AT, "szPreauthPwd == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szPreauthPwd length = (%d)", (int)strlen(szPreauthPwd));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szPreauthPwd[0], &szPreauthPwd[0], strlen(szPreauthPwd));

        return (VS_SUCCESS);
}

/*
Function        :inGetInstallmentPwdEnable
Date&Time       :
Describe        :
*/
int inGetInstallmentPwdEnable(char* szInstallmentPwdEnable)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szInstallmentPwdEnable == NULL || strlen(srPWDRec.szInstallmentPwdEnable) <= 0 || strlen(srPWDRec.szInstallmentPwdEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetInstallmentPwdEnable() ERROR !!");

                	if (szInstallmentPwdEnable == NULL)
                	{
                	        inLogPrintf(AT, "szInstallmentPwdEnable == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szInstallmentPwdEnable length = (%d)", (int)strlen(srPWDRec.szInstallmentPwdEnable));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szInstallmentPwdEnable[0], &srPWDRec.szInstallmentPwdEnable[0], strlen(srPWDRec.szInstallmentPwdEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetInstallmentPwdEnable
Date&Time       :
Describe        :
*/
int inSetInstallmentPwdEnable(char* szInstallmentPwdEnable)
{
        memset(srPWDRec.szInstallmentPwdEnable, 0x00, sizeof(srPWDRec.szInstallmentPwdEnable));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szInstallmentPwdEnable == NULL || strlen(szInstallmentPwdEnable) <= 0 || strlen(szInstallmentPwdEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetInstallmentPwdEnable() ERROR !!");

			if (szInstallmentPwdEnable == NULL)
			{
			        inLogPrintf(AT, "szInstallmentPwdEnable == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szInstallmentPwdEnable length = (%d)", (int)strlen(szInstallmentPwdEnable));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szInstallmentPwdEnable[0], &szInstallmentPwdEnable[0], strlen(szInstallmentPwdEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetInstallmentPwd
Date&Time       :
Describe        :
*/
int inGetInstallmentPwd(char* szInstallmentPwd)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szInstallmentPwd == NULL || strlen(srPWDRec.szInstallmentPwd) <= 0 || strlen(srPWDRec.szInstallmentPwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetInstallmentPwd() ERROR !!");

                	if (szInstallmentPwd == NULL)
                	{
                	        inLogPrintf(AT, "szInstallmentPwd == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szInstallmentPwd length = (%d)", (int)strlen(srPWDRec.szInstallmentPwd));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szInstallmentPwd[0], &srPWDRec.szInstallmentPwd[0], strlen(srPWDRec.szInstallmentPwd));

        return (VS_SUCCESS);
}

/*
Function        :inSetInstallmentPwd
Date&Time       :
Describe        :
*/
int inSetInstallmentPwd(char* szInstallmentPwd)
{
        memset(srPWDRec.szInstallmentPwd, 0x00, sizeof(srPWDRec.szInstallmentPwd));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szInstallmentPwd == NULL || strlen(szInstallmentPwd) <= 0 || strlen(szInstallmentPwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetInstallmentPwd() ERROR !!");

			if (szInstallmentPwd == NULL)
			{
			        inLogPrintf(AT, "szInstallmentPwd == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szInstallmentPwd length = (%d)", (int)strlen(szInstallmentPwd));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szInstallmentPwd[0], &szInstallmentPwd[0], strlen(szInstallmentPwd));

        return (VS_SUCCESS);
}

/*
Function        :inGetRedeemPwdEnable
Date&Time       :
Describe        :
*/
int inGetRedeemPwdEnable(char* szRedeemPwdEnable)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szRedeemPwdEnable == NULL || strlen(srPWDRec.szRedeemPwdEnable) <= 0 || strlen(srPWDRec.szRedeemPwdEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetRedeemPwdEnable() ERROR !!");

                	if (szRedeemPwdEnable == NULL)
                	{
                	        inLogPrintf(AT, "szRedeemPwdEnable == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szRedeemPwdEnable length = (%d)", (int)strlen(srPWDRec.szRedeemPwdEnable));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szRedeemPwdEnable[0], &srPWDRec.szRedeemPwdEnable[0], strlen(srPWDRec.szRedeemPwdEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetRedeemPwdEnable
Date&Time       :
Describe        :
*/
int inSetRedeemPwdEnable(char* szRedeemPwdEnable)
{
        memset(srPWDRec.szRedeemPwdEnable, 0x00, sizeof(srPWDRec.szRedeemPwdEnable));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szRedeemPwdEnable == NULL || strlen(szRedeemPwdEnable) <= 0 || strlen(szRedeemPwdEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetRedeemPwdEnable() ERROR !!");

			if (szRedeemPwdEnable == NULL)
			{
			        inLogPrintf(AT, "szRedeemPwdEnable == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szRedeemPwdEnable length = (%d)", (int)strlen(szRedeemPwdEnable));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szRedeemPwdEnable[0], &szRedeemPwdEnable[0], strlen(szRedeemPwdEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetRedeemPwd
Date&Time       :
Describe        :
*/
int inGetRedeemPwd(char* szRedeemPwd)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szRedeemPwd == NULL || strlen(srPWDRec.szRedeemPwd) <= 0 || strlen(srPWDRec.szRedeemPwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetRedeemPwd() ERROR !!");

                	if (szRedeemPwd == NULL)
                	{
                	        inLogPrintf(AT, "szRedeemPwd == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szRedeemPwd length = (%d)", (int)strlen(srPWDRec.szRedeemPwd));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szRedeemPwd[0], &srPWDRec.szRedeemPwd[0], strlen(srPWDRec.szRedeemPwd));

        return (VS_SUCCESS);
}

/*
Function        :inSetRedeemPwd
Date&Time       :
Describe        :
*/
int inSetRedeemPwd(char* szRedeemPwd)
{
        memset(srPWDRec.szRedeemPwd, 0x00, sizeof(srPWDRec.szRedeemPwd));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szRedeemPwd == NULL || strlen(szRedeemPwd) <= 0 || strlen(szRedeemPwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetRedeemPwd() ERROR !!");

			if (szRedeemPwd == NULL)
			{
			        inLogPrintf(AT, "szRedeemPwd == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szRedeemPwd length = (%d)", (int)strlen(szRedeemPwd));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szRedeemPwd[0], &szRedeemPwd[0], strlen(szRedeemPwd));

        return (VS_SUCCESS);
}

/*
Function        :inGetMailOrderPwdEnable
Date&Time       :
Describe        :
*/
int inGetMailOrderPwdEnable(char* szMailOrderPwdEnable)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMailOrderPwdEnable == NULL || strlen(srPWDRec.szMailOrderPwdEnable) <= 0 || strlen(srPWDRec.szMailOrderPwdEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetMailOrderPwdEnable() ERROR !!");

                	if (szMailOrderPwdEnable == NULL)
                	{
                	        inLogPrintf(AT, "szMailOrderPwdEnable == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szMailOrderPwdEnable length = (%d)", (int)strlen(srPWDRec.szMailOrderPwdEnable));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szMailOrderPwdEnable[0], &srPWDRec.szMailOrderPwdEnable[0], strlen(srPWDRec.szMailOrderPwdEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetMailOrderPwdEnable
Date&Time       :
Describe        :
*/
int inSetMailOrderPwdEnable(char* szMailOrderPwdEnable)
{
        memset(srPWDRec.szMailOrderPwdEnable, 0x00, sizeof(srPWDRec.szMailOrderPwdEnable));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMailOrderPwdEnable == NULL || strlen(szMailOrderPwdEnable) <= 0 || strlen(szMailOrderPwdEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetMailOrderPwdEnable() ERROR !!");

			if (szMailOrderPwdEnable == NULL)
			{
			        inLogPrintf(AT, "szMailOrderPwdEnable == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szMailOrderPwdEnable length = (%d)", (int)strlen(szMailOrderPwdEnable));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szMailOrderPwdEnable[0], &szMailOrderPwdEnable[0], strlen(szMailOrderPwdEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetMailOrderPwd
Date&Time       :
Describe        :
*/
int inGetMailOrderPwd(char* szMailOrderPwd)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMailOrderPwd == NULL || strlen(srPWDRec.szMailOrderPwd) <= 0 || strlen(srPWDRec.szMailOrderPwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetMailOrderPwd() ERROR !!");

                	if (szMailOrderPwd == NULL)
                	{
                	        inLogPrintf(AT, "szMailOrderPwd == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szMailOrderPwd length = (%d)", (int)strlen(srPWDRec.szMailOrderPwd));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szMailOrderPwd[0], &srPWDRec.szMailOrderPwd[0], strlen(srPWDRec.szMailOrderPwd));

        return (VS_SUCCESS);
}

/*
Function        :inSetMailOrderPwd
Date&Time       :
Describe        :
*/
int inSetMailOrderPwd(char* szMailOrderPwd)
{
        memset(srPWDRec.szMailOrderPwd, 0x00, sizeof(srPWDRec.szMailOrderPwd));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szMailOrderPwd == NULL || strlen(szMailOrderPwd) <= 0 || strlen(szMailOrderPwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetMailOrderPwd() ERROR !!");

			if (szMailOrderPwd == NULL)
			{
			        inLogPrintf(AT, "szMailOrderPwd == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szMailOrderPwd length = (%d)", (int)strlen(szMailOrderPwd));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szMailOrderPwd[0], &szMailOrderPwd[0], strlen(szMailOrderPwd));

        return (VS_SUCCESS);
}

/*
Function        :inGetOfflinePwdEnable
Date&Time       :
Describe        :
*/
int inGetOfflinePwdEnable(char* szOfflinePwdEnable)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szOfflinePwdEnable == NULL || strlen(srPWDRec.szOfflinePwdEnable) <= 0 || strlen(srPWDRec.szOfflinePwdEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetOfflinePwdEnable() ERROR !!");

                	if (szOfflinePwdEnable == NULL)
                	{
                	        inLogPrintf(AT, "szOfflinePwdEnable == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szOfflinePwdEnable length = (%d)", (int)strlen(srPWDRec.szOfflinePwdEnable));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szOfflinePwdEnable[0], &srPWDRec.szOfflinePwdEnable[0], strlen(srPWDRec.szOfflinePwdEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetOfflinePwdEnable
Date&Time       :
Describe        :
*/
int inSetOfflinePwdEnable(char* szOfflinePwdEnable)
{
        memset(srPWDRec.szOfflinePwdEnable, 0x00, sizeof(srPWDRec.szOfflinePwdEnable));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szOfflinePwdEnable == NULL || strlen(szOfflinePwdEnable) <= 0 || strlen(szOfflinePwdEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetOfflinePwdEnable() ERROR !!");

			if (szOfflinePwdEnable == NULL)
			{
			        inLogPrintf(AT, "szOfflinePwdEnable == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szOfflinePwdEnable length = (%d)", (int)strlen(szOfflinePwdEnable));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szOfflinePwdEnable[0], &szOfflinePwdEnable[0], strlen(szOfflinePwdEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetOfflinePwd
Date&Time       :
Describe        :
*/
int inGetOfflinePwd(char* szOfflinePwd)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szOfflinePwd == NULL || strlen(srPWDRec.szOfflinePwd) <= 0 || strlen(srPWDRec.szOfflinePwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetOfflinePwd() ERROR !!");

                	if (szOfflinePwd == NULL)
                	{
                	        inLogPrintf(AT, "szOfflinePwd == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szOfflinePwd length = (%d)", (int)strlen(srPWDRec.szOfflinePwd));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szOfflinePwd[0], &srPWDRec.szOfflinePwd[0], strlen(srPWDRec.szOfflinePwd));

        return (VS_SUCCESS);
}

/*
Function        :inSetOfflinePwd
Date&Time       :
Describe        :
*/
int inSetOfflinePwd(char* szOfflinePwd)
{
        memset(srPWDRec.szOfflinePwd, 0x00, sizeof(srPWDRec.szOfflinePwd));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szOfflinePwd == NULL || strlen(szOfflinePwd) <= 0 || strlen(szOfflinePwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetOfflinePwd() ERROR !!");

			if (szOfflinePwd == NULL)
			{
			        inLogPrintf(AT, "szOfflinePwd == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szOfflinePwd length = (%d)", (int)strlen(szOfflinePwd));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szOfflinePwd[0], &szOfflinePwd[0], strlen(szOfflinePwd));

        return (VS_SUCCESS);
}

/*
Function        :inGetInstallmentAdjustPwdEnable
Date&Time       :
Describe        :
*/
int inGetInstallmentAdjustPwdEnable(char* szInstallmentAdjustPwdEnable)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szInstallmentAdjustPwdEnable == NULL || strlen(srPWDRec.szInstallmentAdjustPwdEnable) <= 0 || strlen(srPWDRec.szInstallmentAdjustPwdEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetInstallmentAdjustPwdEnable() ERROR !!");

                	if (szInstallmentAdjustPwdEnable == NULL)
                	{
                	        inLogPrintf(AT, "szInstallmentAdjustPwdEnable == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szInstallmentAdjustPwdEnable length = (%d)", (int)strlen(srPWDRec.szInstallmentAdjustPwdEnable));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szInstallmentAdjustPwdEnable[0], &srPWDRec.szInstallmentAdjustPwdEnable[0], strlen(srPWDRec.szInstallmentAdjustPwdEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetInstallmentAdjustPwdEnable
Date&Time       :
Describe        :
*/
int inSetInstallmentAdjustPwdEnable(char* szInstallmentAdjustPwdEnable)
{
        memset(srPWDRec.szInstallmentAdjustPwdEnable, 0x00, sizeof(srPWDRec.szInstallmentAdjustPwdEnable));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szInstallmentAdjustPwdEnable == NULL || strlen(szInstallmentAdjustPwdEnable) <= 0 || strlen(szInstallmentAdjustPwdEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetInstallmentAdjustPwdEnable() ERROR !!");

			if (szInstallmentAdjustPwdEnable == NULL)
			{
			        inLogPrintf(AT, "szInstallmentAdjustPwdEnable == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szInstallmentAdjustPwdEnable length = (%d)", (int)strlen(szInstallmentAdjustPwdEnable));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szInstallmentAdjustPwdEnable[0], &szInstallmentAdjustPwdEnable[0], strlen(szInstallmentAdjustPwdEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetInstallmentAdjustPwd
Date&Time       :
Describe        :
*/
int inGetInstallmentAdjustPwd(char* szInstallmentAdjustPwd)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szInstallmentAdjustPwd == NULL || strlen(srPWDRec.szInstallmentAdjustPwd) <= 0 || strlen(srPWDRec.szInstallmentAdjustPwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetInstallmentAdjustPwd() ERROR !!");

                	if (szInstallmentAdjustPwd == NULL)
                	{
                	        inLogPrintf(AT, "szInstallmentAdjustPwd == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szInstallmentAdjustPwd length = (%d)", (int)strlen(srPWDRec.szInstallmentAdjustPwd));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szInstallmentAdjustPwd[0], &srPWDRec.szInstallmentAdjustPwd[0], strlen(srPWDRec.szInstallmentAdjustPwd));

        return (VS_SUCCESS);
}

/*
Function        :inSetInstallmentAdjustPwd
Date&Time       :
Describe        :
*/
int inSetInstallmentAdjustPwd(char* szInstallmentAdjustPwd)
{
        memset(srPWDRec.szInstallmentAdjustPwd, 0x00, sizeof(srPWDRec.szInstallmentAdjustPwd));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szInstallmentAdjustPwd == NULL || strlen(szInstallmentAdjustPwd) <= 0 || strlen(szInstallmentAdjustPwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetInstallmentAdjustPwd() ERROR !!");

			if (szInstallmentAdjustPwd == NULL)
			{
			        inLogPrintf(AT, "szInstallmentAdjustPwd == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szInstallmentAdjustPwd length = (%d)", (int)strlen(szInstallmentAdjustPwd));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szInstallmentAdjustPwd[0], &szInstallmentAdjustPwd[0], strlen(szInstallmentAdjustPwd));

        return (VS_SUCCESS);
}

/*
Function        :inGetRedeemAdjustPwdEnable
Date&Time       :
Describe        :
*/
int inGetRedeemAdjustPwdEnable(char* szRedeemAdjustPwdEnable)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szRedeemAdjustPwdEnable == NULL || strlen(srPWDRec.szRedeemAdjustPwdEnable) <= 0 || strlen(srPWDRec.szRedeemAdjustPwdEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetRedeemAdjustPwdEnable() ERROR !!");

                	if (szRedeemAdjustPwdEnable == NULL)
                	{
                	        inLogPrintf(AT, "szRedeemAdjustPwdEnable == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szRedeemAdjustPwdEnable length = (%d)", (int)strlen(srPWDRec.szRedeemAdjustPwdEnable));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szRedeemAdjustPwdEnable[0], &srPWDRec.szRedeemAdjustPwdEnable[0], strlen(srPWDRec.szRedeemAdjustPwdEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetRedeemAdjustPwdEnable
Date&Time       :
Describe        :
*/
int inSetRedeemAdjustPwdEnable(char* szRedeemAdjustPwdEnable)
{
        memset(srPWDRec.szRedeemAdjustPwdEnable, 0x00, sizeof(srPWDRec.szRedeemAdjustPwdEnable));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szRedeemAdjustPwdEnable == NULL || strlen(szRedeemAdjustPwdEnable) <= 0 || strlen(szRedeemAdjustPwdEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetRedeemAdjustPwdEnable() ERROR !!");

			if (szRedeemAdjustPwdEnable == NULL)
			{
			        inLogPrintf(AT, "szRedeemAdjustPwdEnable == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szRedeemAdjustPwdEnable length = (%d)", (int)strlen(szRedeemAdjustPwdEnable));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szRedeemAdjustPwdEnable[0], &szRedeemAdjustPwdEnable[0], strlen(szRedeemAdjustPwdEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetRedeemAdjustPwd
Date&Time       :
Describe        :
*/
int inGetRedeemAdjustPwd(char* szRedeemAdjustPwd)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szRedeemAdjustPwd == NULL || strlen(srPWDRec.szRedeemAdjustPwd) <= 0 || strlen(srPWDRec.szRedeemAdjustPwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetRedeemAdjustPwd() ERROR !!");

                	if (szRedeemAdjustPwd == NULL)
                	{
                	        inLogPrintf(AT, "szRedeemAdjustPwd == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szRedeemAdjustPwd length = (%d)", (int)strlen(srPWDRec.szRedeemAdjustPwd));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szRedeemAdjustPwd[0], &srPWDRec.szRedeemAdjustPwd[0], strlen(srPWDRec.szRedeemAdjustPwd));

        return (VS_SUCCESS);
}

/*
Function        :inSetRedeemAdjustPwd
Date&Time       :
Describe        :
*/
int inSetRedeemAdjustPwd(char* szRedeemAdjustPwd)
{
        memset(srPWDRec.szRedeemAdjustPwd, 0x00, sizeof(srPWDRec.szRedeemAdjustPwd));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szRedeemAdjustPwd == NULL || strlen(szRedeemAdjustPwd) <= 0 || strlen(szRedeemAdjustPwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetRedeemAdjustPwd() ERROR !!");

			if (szRedeemAdjustPwd == NULL)
			{
			        inLogPrintf(AT, "szRedeemAdjustPwd == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szRedeemAdjustPwd length = (%d)", (int)strlen(szRedeemAdjustPwd));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szRedeemAdjustPwd[0], &szRedeemAdjustPwd[0], strlen(szRedeemAdjustPwd));

        return (VS_SUCCESS);
}

/*
Function        :inGetVoidPwdEnable
Date&Time       :
Describe        :
*/
int inGetVoidPwdEnable(char* szVoidPwdEnable)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szVoidPwdEnable == NULL || strlen(srPWDRec.szVoidPwdEnable) <= 0 || strlen(srPWDRec.szVoidPwdEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetVoidPwdEnable() ERROR !!");

                	if (szVoidPwdEnable == NULL)
                	{
                	        inLogPrintf(AT, "szVoidPwdEnable == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szVoidPwdEnable length = (%d)", (int)strlen(srPWDRec.szVoidPwdEnable));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szVoidPwdEnable[0], &srPWDRec.szVoidPwdEnable[0], strlen(srPWDRec.szVoidPwdEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetVoidPwdEnable
Date&Time       :
Describe        :
*/
int inSetVoidPwdEnable(char* szVoidPwdEnable)
{
        memset(srPWDRec.szVoidPwdEnable, 0x00, sizeof(srPWDRec.szVoidPwdEnable));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szVoidPwdEnable == NULL || strlen(szVoidPwdEnable) <= 0 || strlen(szVoidPwdEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetVoidPwdEnable() ERROR !!");

			if (szVoidPwdEnable == NULL)
			{
			        inLogPrintf(AT, "szVoidPwdEnable == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szVoidPwdEnable length = (%d)", (int)strlen(szVoidPwdEnable));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szVoidPwdEnable[0], &szVoidPwdEnable[0], strlen(szVoidPwdEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetVoidPwd
Date&Time       :
Describe        :
*/
int inGetVoidPwd(char* szVoidPwd)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szVoidPwd == NULL || strlen(srPWDRec.szVoidPwd) <= 0 || strlen(srPWDRec.szVoidPwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetVoidPwd() ERROR !!");

                	if (szVoidPwd == NULL)
                	{
                	        inLogPrintf(AT, "szVoidPwd == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szVoidPwd length = (%d)", (int)strlen(srPWDRec.szVoidPwd));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szVoidPwd[0], &srPWDRec.szVoidPwd[0], strlen(srPWDRec.szVoidPwd));

        return (VS_SUCCESS);
}

/*
Function        :inSetVoidPwd
Date&Time       :
Describe        :
*/
int inSetVoidPwd(char* szVoidPwd)
{
        memset(srPWDRec.szVoidPwd, 0x00, sizeof(srPWDRec.szVoidPwd));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szVoidPwd == NULL || strlen(szVoidPwd) <= 0 || strlen(szVoidPwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetVoidPwd() ERROR !!");

			if (szVoidPwd == NULL)
			{
			        inLogPrintf(AT, "szVoidPwd == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szVoidPwd length = (%d)", (int)strlen(szVoidPwd));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szVoidPwd[0], &szVoidPwd[0], strlen(szVoidPwd));

        return (VS_SUCCESS);
}

/*
Function        :inGetSettlementPwdEnable
Date&Time       :
Describe        :
*/
int inGetSettlementPwdEnable(char* szSettlementPwdEnable)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSettlementPwdEnable == NULL || strlen(srPWDRec.szSettlementPwdEnable) <= 0 || strlen(srPWDRec.szSettlementPwdEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetSettlementPwdEnable() ERROR !!");

                	if (szSettlementPwdEnable == NULL)
                	{
                	        inLogPrintf(AT, "szSettlementPwdEnable == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szSettlementPwdEnable length = (%d)", (int)strlen(srPWDRec.szSettlementPwdEnable));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szSettlementPwdEnable[0], &srPWDRec.szSettlementPwdEnable[0], strlen(srPWDRec.szSettlementPwdEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetSettlementPwdEnable
Date&Time       :
Describe        :
*/
int inSetSettlementPwdEnable(char* szSettlementPwdEnable)
{
        memset(srPWDRec.szSettlementPwdEnable, 0x00, sizeof(srPWDRec.szSettlementPwdEnable));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSettlementPwdEnable == NULL || strlen(szSettlementPwdEnable) <= 0 || strlen(szSettlementPwdEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetSettlementPwdEnable() ERROR !!");

			if (szSettlementPwdEnable == NULL)
			{
			        inLogPrintf(AT, "szSettlementPwdEnable == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szSettlementPwdEnable length = (%d)", (int)strlen(szSettlementPwdEnable));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szSettlementPwdEnable[0], &szSettlementPwdEnable[0], strlen(szSettlementPwdEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetSettlementPwd
Date&Time       :
Describe        :
*/
int inGetSettlementPwd(char* szSettlementPwd)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSettlementPwd == NULL || strlen(srPWDRec.szSettlementPwd) <= 0 || strlen(srPWDRec.szSettlementPwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetSettlementPwd() ERROR !!");

                	if (szSettlementPwd == NULL)
                	{
                	        inLogPrintf(AT, "szSettlementPwd == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szSettlementPwd length = (%d)", (int)strlen(srPWDRec.szSettlementPwd));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szSettlementPwd[0], &srPWDRec.szSettlementPwd[0], strlen(srPWDRec.szSettlementPwd));

        return (VS_SUCCESS);
}

/*
Function        :inSetSettlementPwd
Date&Time       :
Describe        :
*/
int inSetSettlementPwd(char* szSettlementPwd)
{
        memset(srPWDRec.szSettlementPwd, 0x00, sizeof(srPWDRec.szSettlementPwd));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szSettlementPwd == NULL || strlen(szSettlementPwd) <= 0 || strlen(szSettlementPwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetSettlementPwd() ERROR !!");

			if (szSettlementPwd == NULL)
			{
			        inLogPrintf(AT, "szSettlementPwd == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szSettlementPwd length = (%d)", (int)strlen(szSettlementPwd));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szSettlementPwd[0], &szSettlementPwd[0], strlen(szSettlementPwd));

        return (VS_SUCCESS);
}


/*
Function        :inGetRefundPwdEnable
Date&Time       :
Describe        :
*/
int inGetRefundPwdEnable(char* szRefundPwdEnable)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szRefundPwdEnable == NULL || strlen(srPWDRec.szRefundPwdEnable) <= 0 || strlen(srPWDRec.szRefundPwdEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetRefundPwdEnable() ERROR !!");

                	if (szRefundPwdEnable == NULL)
                	{
                	        inLogPrintf(AT, "szRefundPwdEnable == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szRefundPwdEnable length = (%d)", (int)strlen(srPWDRec.szRefundPwdEnable));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szRefundPwdEnable[0], &srPWDRec.szRefundPwdEnable[0], strlen(srPWDRec.szRefundPwdEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetRefundPwdEnable
Date&Time       :
Describe        :
*/
int inSetRefundPwdEnable(char* szRefundPwdEnable)
{
        memset(srPWDRec.szRefundPwdEnable, 0x00, sizeof(srPWDRec.szRefundPwdEnable));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szRefundPwdEnable == NULL || strlen(szRefundPwdEnable) <= 0 || strlen(szRefundPwdEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetRefundPwdEnable() ERROR !!");

			if (szRefundPwdEnable == NULL)
			{
			        inLogPrintf(AT, "szRefundPwdEnable == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szRefundPwdEnable length = (%d)", (int)strlen(szRefundPwdEnable));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szRefundPwdEnable[0], &szRefundPwdEnable[0], strlen(szRefundPwdEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetRefundPwd
Date&Time       :
Describe        :
*/
int inGetRefundPwd(char* szRefundPwd)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szRefundPwd == NULL || strlen(srPWDRec.szRefundPwd) <= 0 || strlen(srPWDRec.szRefundPwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetRefundPwd() ERROR !!");

                	if (szRefundPwd == NULL)
                	{
                	        inLogPrintf(AT, "szRefundPwd == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szRefundPwd length = (%d)", (int)strlen(srPWDRec.szRefundPwd));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szRefundPwd[0], &srPWDRec.szRefundPwd[0], strlen(srPWDRec.szRefundPwd));

        return (VS_SUCCESS);
}

/*
Function        :inSetRefundPwd
Date&Time       :
Describe        :
*/
int inSetRefundPwd(char* szRefundPwd)
{
        memset(srPWDRec.szRefundPwd, 0x00, sizeof(srPWDRec.szRefundPwd));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szRefundPwd == NULL || strlen(szRefundPwd) <= 0 || strlen(szRefundPwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetRefundPwd() ERROR !!");

			if (szRefundPwd == NULL)
			{
			        inLogPrintf(AT, "szRefundPwd == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szRefundPwd length = (%d)", (int)strlen(szRefundPwd));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szRefundPwd[0], &szRefundPwd[0], strlen(szRefundPwd));

        return (VS_SUCCESS);
}

/*
Function        :inGetHGRefundPwdEnable
Date&Time       :
Describe        :
*/
int inGetHGRefundPwdEnable(char* szHGRefundPwdEnable)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHGRefundPwdEnable == NULL || strlen(srPWDRec.szHGRefundPwdEnable) <= 0 || strlen(srPWDRec.szHGRefundPwdEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetHGRefundPwdEnable() ERROR !!");

                	if (szHGRefundPwdEnable == NULL)
                	{
                	        inLogPrintf(AT, "szHGRefundPwdEnable == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szHGRefundPwdEnable length = (%d)", (int)strlen(srPWDRec.szHGRefundPwdEnable));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szHGRefundPwdEnable[0], &srPWDRec.szHGRefundPwdEnable[0], strlen(srPWDRec.szHGRefundPwdEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetHGRefundPwdEnable
Date&Time       :
Describe        :
*/
int inSetHGRefundPwdEnable(char* szHGRefundPwdEnable)
{
        memset(srPWDRec.szHGRefundPwdEnable, 0x00, sizeof(srPWDRec.szHGRefundPwdEnable));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHGRefundPwdEnable == NULL || strlen(szHGRefundPwdEnable) <= 0 || strlen(szHGRefundPwdEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetHGRefundPwdEnable() ERROR !!");

			if (szHGRefundPwdEnable == NULL)
			{
			        inLogPrintf(AT, "szHGRefundPwdEnable == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szHGRefundPwdEnable length = (%d)", (int)strlen(szHGRefundPwdEnable));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szHGRefundPwdEnable[0], &szHGRefundPwdEnable[0], strlen(szHGRefundPwdEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetHGRefundPwd
Date&Time       :
Describe        :
*/
int inGetHGRefundPwd(char* szHGRefundPwd)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHGRefundPwd == NULL || strlen(srPWDRec.szHGRefundPwd) <= 0 || strlen(srPWDRec.szHGRefundPwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetHGRefundPwd() ERROR !!");

                	if (szHGRefundPwd == NULL)
                	{
                	        inLogPrintf(AT, "szHGRefundPwd == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szHGRefundPwd length = (%d)", (int)strlen(srPWDRec.szHGRefundPwd));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szHGRefundPwd[0], &srPWDRec.szHGRefundPwd[0], strlen(srPWDRec.szHGRefundPwd));

        return (VS_SUCCESS);
}

/*
Function        :inSetHGRefundPwd
Date&Time       :
Describe        :
*/
int inSetHGRefundPwd(char* szHGRefundPwd)
{
        memset(srPWDRec.szHGRefundPwd, 0x00, sizeof(srPWDRec.szHGRefundPwd));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szHGRefundPwd == NULL || strlen(szHGRefundPwd) <= 0 || strlen(szHGRefundPwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetHGRefundPwd() ERROR !!");

			if (szHGRefundPwd == NULL)
			{
			        inLogPrintf(AT, "szHGRefundPwd == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szHGRefundPwd length = (%d)", (int)strlen(szHGRefundPwd));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szHGRefundPwd[0], &szHGRefundPwd[0], strlen(szHGRefundPwd));

        return (VS_SUCCESS);
}

/*
Function        :inGetCallBankForcePwdEnable
Date&Time       :
Describe        :
*/
int inGetCallBankForcePwdEnable(char* szCallBankForcePwdEnable)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCallBankForcePwdEnable == NULL || strlen(srPWDRec.szCallBankForcePwdEnable) <= 0 || strlen(srPWDRec.szCallBankForcePwdEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetCallBankForcePwdEnable() ERROR !!");

                	if (szCallBankForcePwdEnable == NULL)
                	{
                	        inLogPrintf(AT, "szCallBankForcePwdEnable == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szCallBankForcePwdEnable length = (%d)", (int)strlen(srPWDRec.szCallBankForcePwdEnable));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szCallBankForcePwdEnable[0], &srPWDRec.szCallBankForcePwdEnable[0], strlen(srPWDRec.szCallBankForcePwdEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetCallBankForcePwdEnable
Date&Time       :
Describe        :
*/
int inSetCallBankForcePwdEnable(char* szCallBankForcePwdEnable)
{
        memset(srPWDRec.szCallBankForcePwdEnable, 0x00, sizeof(srPWDRec.szCallBankForcePwdEnable));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCallBankForcePwdEnable == NULL || strlen(szCallBankForcePwdEnable) <= 0 || strlen(szCallBankForcePwdEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetCallBankForcePwdEnable() ERROR !!");

			if (szCallBankForcePwdEnable == NULL)
			{
			        inLogPrintf(AT, "szCallBankForcePwdEnable == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szCallBankForcePwdEnable length = (%d)", (int)strlen(szCallBankForcePwdEnable));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szCallBankForcePwdEnable[0], &szCallBankForcePwdEnable[0], strlen(szCallBankForcePwdEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetCallBankForcePwd
Date&Time       :
Describe        :
*/
int inGetCallBankForcePwd(char* szCallBankForcePwd)
{
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCallBankForcePwd == NULL || strlen(srPWDRec.szCallBankForcePwd) <= 0 || strlen(srPWDRec.szCallBankForcePwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                	char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        	        inLogPrintf(AT, "inGetCallBankForcePwd() ERROR !!");

                	if (szCallBankForcePwd == NULL)
                	{
                	        inLogPrintf(AT, "szCallBankForcePwd == NULL");
                	}
                	else
                	{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szCallBankForcePwd length = (%d)", (int)strlen(srPWDRec.szCallBankForcePwd));
                	        inLogPrintf(AT, szErrorMsg);
                	}
                }

                return (VS_ERROR);
        }
        memcpy(&szCallBankForcePwd[0], &srPWDRec.szCallBankForcePwd[0], strlen(srPWDRec.szCallBankForcePwd));

        return (VS_SUCCESS);
}

/*
Function        :inSetCallBankForcePwd
Date&Time       :
Describe        :
*/
int inSetCallBankForcePwd(char* szCallBankForcePwd)
{
        memset(srPWDRec.szCallBankForcePwd, 0x00, sizeof(srPWDRec.szCallBankForcePwd));
        /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
        if (szCallBankForcePwd == NULL || strlen(szCallBankForcePwd) <= 0 || strlen(szCallBankForcePwd) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
			char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

			inLogPrintf(AT, "inSetCallBankForcePwd() ERROR !!");

			if (szCallBankForcePwd == NULL)
			{
			        inLogPrintf(AT, "szCallBankForcePwd == NULL");
			}
			else
			{
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                	        sprintf(szErrorMsg, "szCallBankForcePwd length = (%d)", (int)strlen(szCallBankForcePwd));
			        inLogPrintf(AT, szErrorMsg);
			}
                }

                return (VS_ERROR);
        }
        memcpy(&srPWDRec.szCallBankForcePwd[0], &szCallBankForcePwd[0], strlen(szCallBankForcePwd));

        return (VS_SUCCESS);
}

/*
Function        :inFunc_Edit_PWD_Table
Date&Time       :2017/3/28 下午 3:09
Describe        :
*/
int inPWD_Edit_PWD_Table(void)
{
	TABLE_GET_SET_TABLE PWD_FUNC_TABLE[] =
	{
		{"szInitialManagerEnable"			,inGetInitialManagerEnable		,inSetInitialManagerEnable		},
		{"szInitialManagerPwd"				,inGetInitialManagerPwd			,inSetInitialManagerPwd			},
		{"szRebootPwdEnale"				,inGetRebootPwdEnale			,inSetRebootPwdEnale			},
		{"szRebootPwd"					,inGetRebootPwd				,inSetRebootPwd				},
		{"szSalePwdEnable"				,inGetSalePwdEnable			,inSetSalePwdEnable			},
		{"szSalePwd"					,inGetSalePwd				,inSetSalePwd				},
		{"szPreauthPwdEnable"				,inGetPreauthPwdEnable			,inSetPreauthPwdEnable			},
		{"szPreauthPwd"					,inGetPreauthPwd			,inSetPreauthPwd			},
		{"szInstallmentPwdEnable"			,inGetInstallmentPwdEnable		,inSetInstallmentPwdEnable		},
		{"szInstallmentPwd"				,inGetInstallmentPwd			,inSetInstallmentPwd			},
		{"szRedeemPwdEnable"				,inGetRedeemPwdEnable			,inSetRedeemPwdEnable			},
		{"szRedeemPwd"					,inGetRedeemPwd				,inSetRedeemPwd				},
		{"szMailOrderPwdEnable"				,inGetMailOrderPwdEnable		,inSetMailOrderPwdEnable		},
		{"szMailOrderPwd"				,inGetMailOrderPwd			,inSetMailOrderPwd			},
		{"szOfflinePwdEnable"				,inGetOfflinePwdEnable			,inSetOfflinePwdEnable			},
		{"szOfflinePwd"					,inGetOfflinePwd			,inSetOfflinePwd			},
		{"szInstallmentAdjustPwdEnable"			,inGetInstallmentAdjustPwdEnable	,inSetInstallmentAdjustPwdEnable	},
		{"szInstallmentAdjustPwd"			,inGetInstallmentAdjustPwd		,inSetInstallmentAdjustPwd		},
		{"szRedeemAdjustPwdEnable"			,inGetRedeemAdjustPwdEnable		,inSetRedeemAdjustPwdEnable		},
		{"szRedeemAdjustPwd"				,inGetRedeemAdjustPwd			,inSetRedeemAdjustPwd			},
		{"szVoidPwdEnable"				,inGetVoidPwdEnable			,inSetVoidPwdEnable			},
		{"szVoidPwd"					,inGetVoidPwd				,inSetVoidPwd				},
		{"szSettlementPwdEnable"			,inGetSettlementPwdEnable		,inSetSettlementPwdEnable		},
		{"szSettlementPwd"				,inGetSettlementPwd			,inSetSettlementPwd			},
		{"szRefundPwdEnable"				,inGetRefundPwdEnable			,inSetRefundPwdEnable			},
		{"szRefundPwd"					,inGetRefundPwd				,inSetRefundPwd				},
		{"szHGRefundPwdEnable"				,inGetHGRefundPwdEnable			,inSetHGRefundPwdEnable			},
		{"szHGRefundPwd"				,inGetHGRefundPwd			,inSetHGRefundPwd			},
		{"szCallBankForcePwdEnable"			,inGetCallBankForcePwdEnable		,inSetCallBankForcePwdEnable		},
		{"szCallBankForcePwd"				,inGetCallBankForcePwd			,inSetCallBankForcePwd			},
		{""},
	};
	int	inRetVal = VS_ERROR;
	int	inFinalTimeout = 0;
	char	szKey = 0x00;
	
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
	inDISP_ChineseFont_Color("是否更改PWD", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
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
			break;
		}
		
	}
	
	inFunc_Edit_Table_Tag(PWD_FUNC_TABLE);
	inSavePWDRec(0);
	
	return	(VS_SUCCESS);
}

int inPWD_Test (void)
{
        char    PWD_Data[1024];
        int     i = 0;

        memset(PWD_Data, 0x00, sizeof(PWD_Data));

        inSetInitialManagerEnable("A");
        inSetInitialManagerPwd("0000");
        inSetRebootPwdEnale("B");
        inSetRebootPwd("0001");
        inSetSalePwdEnable("C");
        inSetSalePwd("0002");
        inSetPreauthPwdEnable("D");
        inSetPreauthPwd("0003");
        inSetInstallmentPwdEnable("E");
        inSetInstallmentPwd("0004");
        inSetRedeemPwdEnable("F");
        inSetRedeemPwd("0005");
        inSetMailOrderPwdEnable("G");
        inSetMailOrderPwd("0006");
        inSetOfflinePwdEnable("H");
        inSetOfflinePwd("0007");
        inSetInstallmentAdjustPwdEnable("K");
        inSetInstallmentAdjustPwd("0009");
        inSetRedeemAdjustPwdEnable("L");
        inSetRedeemAdjustPwd("0010");
        inSetVoidPwdEnable("M");
        inSetVoidPwd("0011");
        inSetSettlementPwdEnable("N");
        inSetSettlementPwd("0012");
        inSetRefundPwdEnable("O");
        inSetRefundPwd("0013");
        inSetHGRefundPwdEnable("P");
        inSetHGRefundPwd("0014");

        inSavePWDRec(0);


        inLoadPWDRec(0);


        inGetInitialManagerEnable(&PWD_Data[i]);
        i++;
        PWD_Data[i] = 0x2C;
        i++;
        inGetInitialManagerPwd(&PWD_Data[i]);
        i+=4;
        PWD_Data[i] = 0x2C;
        i++;
        inGetRebootPwdEnale(&PWD_Data[i]);
        i++;
        PWD_Data[i] = 0x2C;
        i++;
        inGetRebootPwd(&PWD_Data[i]);
        i+=4;
        PWD_Data[i] = 0x2C;
        i++;
        inGetSalePwdEnable(&PWD_Data[i]);
        i++;
        PWD_Data[i] = 0x2C;
        i++;
        inGetSalePwd(&PWD_Data[i]);
        i+=4;
        PWD_Data[i] = 0x2C;
        i++;
        inGetPreauthPwdEnable(&PWD_Data[i]);
        i++;
        PWD_Data[i] = 0x2C;
        i++;
        inGetPreauthPwd(&PWD_Data[i]);
        i+=4;
        PWD_Data[i] = 0x2C;
        i++;
        inGetInstallmentPwdEnable(&PWD_Data[i]);
        i++;
        PWD_Data[i] = 0x2C;
        i++;
        inGetInstallmentPwd(&PWD_Data[i]);
        i+=4;
        PWD_Data[i] = 0x2C;
        i++;
        inGetRedeemPwdEnable(&PWD_Data[i]);
        i++;
        PWD_Data[i] = 0x2C;
        i++;
        inGetRedeemPwd(&PWD_Data[i]);
        i+=4;
        PWD_Data[i] = 0x2C;
        i++;
        inGetMailOrderPwdEnable(&PWD_Data[i]);
        i++;
        PWD_Data[i] = 0x2C;
        i++;
        inGetMailOrderPwd(&PWD_Data[i]);
        i+=4;
        PWD_Data[i] = 0x2C;
        i++;
        inGetOfflinePwdEnable(&PWD_Data[i]);
        i++;
        PWD_Data[i] = 0x2C;
        i++;
        inGetOfflinePwd(&PWD_Data[i]);
        i+=4;
        PWD_Data[i] = 0x2C;
        i++;
        PWD_Data[i] = 0x2C;
        i++;
        inGetInstallmentAdjustPwdEnable(&PWD_Data[i]);
        i++;
        PWD_Data[i] = 0x2C;
        i++;
        inGetInstallmentAdjustPwd(&PWD_Data[i]);
        i+=4;
        PWD_Data[i] = 0x2C;
        i++;
        inGetRedeemAdjustPwdEnable(&PWD_Data[i]);
        i++;
        PWD_Data[i] = 0x2C;
        i++;
        inGetRedeemAdjustPwd(&PWD_Data[i]);
        i+=4;
        PWD_Data[i] = 0x2C;
        i++;
        inGetVoidPwdEnable(&PWD_Data[i]);
        i++;
        PWD_Data[i] = 0x2C;
        i++;
        inGetVoidPwd(&PWD_Data[i]);
        i+=4;
        PWD_Data[i] = 0x2C;
        i++;
        inGetSettlementPwdEnable(&PWD_Data[i]);
        i++;
        PWD_Data[i] = 0x2C;
        i++;
        inGetSettlementPwd(&PWD_Data[i]);
        i+=4;
        PWD_Data[i] = 0x2C;
        i++;
        inGetRefundPwdEnable(&PWD_Data[i]);
        i++;
        PWD_Data[i] = 0x2C;
        i++;
        inGetRefundPwd(&PWD_Data[i]);
        i+=4;
        PWD_Data[i] = 0x2C;
        i++;
        inGetHGRefundPwdEnable(&PWD_Data[i]);
        i++;
        PWD_Data[i] = 0x2C;
        i++;
        inGetHGRefundPwd(&PWD_Data[i]);

        inLogPrintf(AT, PWD_Data);

        return (VS_SUCCESS);
}
