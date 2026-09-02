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
#include "CFGT.h"
#include "../../CTLS/CTLS.h"	/* For inCFGT_Edit_CFGT_Table 用 */

static  CFGT_REC srCFGTRec;	/* construct CFGT record */
extern  int	ginDebug;	/* Debug使用 extern */

/*
Function        :inLoadCFGTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :讀CFGT檔案，inCFGTRec是要讀哪一筆的紀錄，第一筆為0
*/
int inLoadCFGTRec(int inCFGTRec)
{
	unsigned long ulFile_Handle; /* File Handle */
	unsigned char *uszReadData; /* 放抓到的record */
	unsigned char *uszTemp; /* 暫存，放整筆CFGT檔案 */
	char szCFGTRec[_SIZE_CFGT_REC_ + 1]; /* 暫存, 放各個欄位檔案 */
	char szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1]; /* debug message */
	long lnCFGTLength = 0; /* CFGT總長度 */
	long lnReadLength; /* 記錄每次要從CFGT.dat讀多長的資料 */
	int i, k, j = 0, inRec = 0; /* inRec記錄讀到第幾筆, i為目前從CFGT讀到的第幾個字元, j為該record的長度, k為該欄位的第幾個字元 */
	int inSearchResult = -1; /* 判斷有沒有讀到0x0D 0x0A的Flag */

	/* inLoadCFGTRec()_START */
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		memset(szErrorMsg, 0x00, sizeof (szErrorMsg));
		sprintf(szErrorMsg, "inLoadCFGTRec(%d) START!!", inCFGTRec);
		inLogPrintf(AT, szErrorMsg);
	}

	/* 判斷傳進來的inCFGTRec是否小於零 大於等於零才是正確值(防呆) */
	if (inCFGTRec < 0)
	{
		/* debug */
		if (ginDebug == VS_TRUE)
		{
			memset(szErrorMsg, 0x00, sizeof (szErrorMsg));
			sprintf(szErrorMsg, "inCFGTRec < 0:(index = %d) ERROR!!", inCFGTRec);
			inLogPrintf(AT, szErrorMsg);
		}

		return (VS_ERROR);
	}

	/*
	 * open CFGT.dat file
	 * open失敗時，if條件成立，關檔並回傳VS_ERROR
	 */
	if (inFILE_Open(&ulFile_Handle, (unsigned char *) _CFGT_FILE_NAME_) == VS_ERROR)
	{
		/* 開檔失敗 ，不用關檔 */
		/* 開檔失敗，所以回傳error */
		return (VS_ERROR);
	}

	/*
	 * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
	 * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
	 */
	lnCFGTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *) _CFGT_FILE_NAME_);

	if (lnCFGTLength == VS_ERROR)
	{
		/* GetSize失敗 ，關檔 */
		inFILE_Close(&ulFile_Handle);

		return (VS_ERROR);
	}

	/*
	 * allocate 記憶體
	 * allocate時多分配一個byte以防萬一（ex:換行符號）
	 */
	uszReadData = malloc(lnCFGTLength + 1);
	uszTemp = malloc(lnCFGTLength + 1);
	/* 初始化 uszTemp uszReadData */
	memset(uszReadData, 0x00, lnCFGTLength + 1);
	memset(uszTemp, 0x00, lnCFGTLength + 1);

	/* seek 到檔案開頭 & 從檔案開頭開始read */
	if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
	{
		lnReadLength = lnCFGTLength;

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
				}					/* 讀失敗時 */
				else
				{
					/* Close檔案 */
					inFILE_Close(&ulFile_Handle);

					/* Free pointer */
					free(uszReadData);
					free(uszTemp);

					return (VS_ERROR);
				}
			}				/* 剩餘長度小於1024 */
			else if (lnReadLength < 1024)
			{
				/* 就只讀剩餘長度 */
				if (inFILE_Read(&ulFile_Handle, &uszTemp[1024 * i], lnReadLength) == VS_SUCCESS)
				{
					break;
				}					/* 讀失敗時 */
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
	}		/* seek不成功時 */
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
	 *i為目前從CFGT讀到的第幾個字元
	 *j為該record的長度
	 */
	j = 0;
	for (i = 0; i <= lnCFGTLength; ++i) /* "<=" 是為了抓到最後一個0x00 */
	{
		/* 讀完一筆record或讀到CFGT的結尾時  */
		if ((uszTemp[i] == 0x0D && uszTemp[i + 1] == 0x0A) || uszTemp[i] == 0x00)
		{
			/* 只要讀到0x0D 0x0A，Flag改為1(表示讀到record的結尾) */
			inSearchResult = 1;

			/* 清空uszReadData */
			memset(uszReadData, 0x00, lnCFGTLength + 1);
			/* 把record從temp指定的位置截取出來放到uszReadData */
			memcpy(&uszReadData[0], &uszTemp[i - j], j);
			inRec++;
			/* 因為inCFGT_Rec的index從0開始，所以inCFGT_Rec要+1 */
			if (inRec == (inCFGTRec + 1))
			{
				break;
			}

			/* 為了跳過 0x0D 0x0A */
			i = i + 2;
			/* 每讀完一筆record，j就歸0 */
			j = 0;
		}

		j++;
	}

	/*
	 * 如果沒有inCFGTRec的那筆資料或者整筆記錄都沒有0x0D 0x0A就回傳Error
	 * 關檔、釋放記憶體並return VS_ERROR
	 * 如果總record數量小於要存取Record的Index
	 * 特例：有可能會遇到全文都沒有0x0D 0x0A
	 */
	if (inRec < (inCFGTRec + 1) || inSearchResult == -1)
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
	memset(&srCFGTRec, 0x00, sizeof (srCFGTRec));
	/*
	 * 以下pattern為存入CFGT_Rec
	 * i為CFGT的第幾個字元
	 * 存入CFGT_Rec
	 */
	i = 0;


	/* 01_客製化參數 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szCustomIndicator[0], &szCFGTRec[0], k - 1);
	}

	/* 02_NCCC FES 模式 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szNCCCFESMode[0], &szCFGTRec[0], k - 1);
	}

	/* 03_通訊模式 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szCommMode[0], &szCFGTRec[0], k - 1);
	}

	/* 04_撥接備援 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szDialBackupEnable[0], &szCFGTRec[0], k - 1);
	}

	/* 05_加密模式 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szEncryptMode[0], &szCFGTRec[0], k - 1);
	}

	/* 06_不可連續刷卡檢核 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szSplitTransCheckEnable[0], &szCFGTRec[0], k - 1);
	}

	/* 07_城市別 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szCityName[0], &szCFGTRec[0], k - 1);
	}

	/* 08_櫃號功能 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szStoreIDEnable[0], &szCFGTRec[0], k - 1);
	}

	/* 09_櫃號輸入最短位數 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szMinStoreIDLen[0], &szCFGTRec[0], k - 1);
	}

	/* 10_櫃號輸入最長位數 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szMaxStoreIDLen[0], &szCFGTRec[0], k - 1);
	}

	/* 11_ECR連線是否啟動 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szECREnable[0], &szCFGTRec[0], k - 1);
	}

	/* 12_ECR卡號遮掩 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szECRCardNoTruncateEnable[0], &szCFGTRec[0], k - 1);
	}

	/* 13_ECR卡片有效期回傳 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szECRExpDateReturnEnable[0], &szCFGTRec[0], k - 1);
	}

	/* 14_列印產品代碼 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szProductCodeEnable[0], &szCFGTRec[0], k - 1);
	}

	/* 15_列印商店 Slogan */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szPrtSlogan[0], &szCFGTRec[0], k - 1);
	}

	/* 16_Slogan起始日 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szSloganStartDate[0], &szCFGTRec[0], k - 1);
	}

	/* 17_Slogan停用日 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szSloganEndDate[0], &szCFGTRec[0], k - 1);
	}

	/* 18_Slogan列印位置 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szSloganPrtPosition[0], &szCFGTRec[0], k - 1);
	}

	/* 19_帳單列印模式 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szPrtMode[0], &szCFGTRec[0], k - 1);
	}

	/* 20_啟動非接觸式讀卡功能 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szContactlessEnable[0], &szCFGTRec[0], k - 1);
	}

	/* 21_接受 Visa Paywave非接觸式卡片 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szVISAPaywaveEnable[0], &szCFGTRec[0], k - 1);
	}

	/* 22_接受 JCB Jspeedy非接觸式卡片  */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szJCBJspeedyEnable[0], &szCFGTRec[0], k - 1);
	}

	/* 23_接受 MC Paypass非接觸式卡片 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szMCPaypassEnable[0], &szCFGTRec[0], k - 1);
	}

	/* 24_CUP退貨限額 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szCUPRefundLimit[0], &szCFGTRec[0], k - 1);
	}

	/* 25_CUP自動安全認證次數 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szCUPKeyExchangeTimes[0], &szCFGTRec[0], k - 1);
	}

	/* 26_交易電文是否上傳MAC驗證 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szMACEnable[0], &szCFGTRec[0], k - 1);
	}

	/* 27_密碼機模式 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szPinpadMode[0], &szCFGTRec[0], k - 1);
	}

	/* 28_CVV2功能 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szFORCECVV2[0], &szCFGTRec[0], k - 1);
	}

	/* 29_啟動特殊卡別參數檔功能 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szSpecialCardRangeEnable[0], &szCFGTRec[0], k - 1);
	}

	/* 30_列印商店 Logo  */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szPrtMerchantLogo[0], &szCFGTRec[0], k - 1);
	}

	/* 31_列印商店表頭  */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szPrtMerchantName[0], &szCFGTRec[0], k - 1);
	}

	/* 32_列印商店提示語 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szPrtNotice[0], &szCFGTRec[0], k - 1);
	}

	/* 33_郵購及定期性行業Indicator */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szElecCommerceFlag[0], &szCFGTRec[0], k - 1);
	}

	/* 34_DCC詢價版本 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szDccFlowVersion[0], &szCFGTRec[0], k - 1);
	}

	/* 35_DCC是否接受VISA卡 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szSupDccVisa[0], &szCFGTRec[0], k - 1);
	}

	/* 36_DCC是否接受MasterCard卡 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szSupDccMasterCard[0], &szCFGTRec[0], k - 1);
	}

	/* 37_DHCP Retry 次數 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szDHCPRetryTimes[0], &szCFGTRec[0], k - 1);
	}

	/* 38_是否開啟BarCode Reader */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szBarCodeReaderEnable[0], &szCFGTRec[0], k - 1);
	}

	/* 39_是否開啟EMV PIN Bypass功能 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szEMVPINBypassEnable[0], &szCFGTRec[0], k - 1);
	}

	/* 40_等候CUP Online PIN輸入的Time out時間 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szCUPOnlinePINEntryTimeout[0], &szCFGTRec[0], k - 1);
	}

	/* 41_簽名板模式 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szSignPadMode[0], &szCFGTRec[0], k - 1);
	}

	/* 42_有開啟簽名板功能(SignPad_Mode='1' || '2')，是否列印商店商店存根聯 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szESCPrintMerchantCopy[0], &szCFGTRec[0], k - 1);
	}

	/* 43_開始日會列印商店存根聯 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szESCPrintMerchantCopyStartDate[0], &szCFGTRec[0], k - 1);
	}

	/* 44_結束日不列印商店存根聯 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szESCPrintMerchantCopyEndDate[0], &szCFGTRec[0], k - 1);
	}

	/* 45_電子簽帳單之上水位參數 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szESCReciptUploadUpLimit[0], &szCFGTRec[0], k - 1);
	}

	/* 46_Contactless_Reader_Mode模式 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szContactlessReaderMode[0], &szCFGTRec[0], k - 1);
	}

	/* 47_端末機依此參數判斷對TMS或Download Server(FTPS)下載 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szTMSDownloadMode[0], &szCFGTRec[0], k - 1);
	}

	/* 48_接受 AMEX 非接觸式卡片(保留未來使用) */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szAMEXContactlessEnable[0], &szCFGTRec[0], k - 1);
	}

	/* 49_接受 CUP非接觸式卡片(QuickPass) */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szCUPContactlessEnable[0], &szCFGTRec[0], k - 1);
	}

	/* 50_接受 SmartPay非接觸式卡片 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szSmartPayContactlessEnable[0], &szCFGTRec[0], k - 1);
	}

	/* 51_開啟繳費項目功能 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szPay_Item_Enable[0], &szCFGTRec[0], k - 1);
	}

	/* 52_商店自存聯卡號遮掩 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szStore_Stub_CardNo_Truncate_Enable[0], &szCFGTRec[0], k - 1);
	}

	/* 53_是否為整合型週邊設備 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szIntegrate_Device[0], &szCFGTRec[0], k - 1);
	}

	/* 54_FES請款代碼 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szFES_ID[0], &szCFGTRec[0], k - 1);
	}

	/* 55_整合型周邊設備之AP ID */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szIntegrate_Device_AP_ID, &szCFGTRec[0], k - 1);
	}

	/* 56_短式簽單模式 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szShort_Receipt_Mode[0], &szCFGTRec[0], k - 1);
	}

	/* 57_I-FES(Internet Gateway)模式 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szI_FES_Mode[0], &szCFGTRec[0], k - 1);
	}

	/* 58_是否開啟DHCP模式 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00) /* 因為是最後的欄位還要多判斷0x00 */
	{
		memcpy(&srCFGTRec.szDHCP_Mode[0], &szCFGTRec[0], k - 1);
	}

	/* 59_電票優先權 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szESVC_Priority[0], &szCFGTRec[0], k - 1);
	}

	/* 60_接受DFS非接觸式卡片 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00)
	{
		memcpy(&srCFGTRec.szDFS_Contactless_Enable[0], &szCFGTRec[0], k - 1);
	}

	/* 61_是否為雲端MFES */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00) /* 因為是最後的欄位還要多判斷0x00 */
	{
		memcpy(&srCFGTRec.szCloud_MFES[0], &szCFGTRec[0], k - 1);
	}

	/* 62_BIN_CHECK */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00) /* 因為是最後的欄位還要多判斷0x00 */
	{
		memcpy(&srCFGTRec.szBIN_CHECK[0], &szCFGTRec[0], k - 1);
	}

	/* 63_是否接受NCCC品牌卡非接觸式卡片 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00) /* 因為是最後的欄位還要多判斷0x00 */
	{
		memcpy(&srCFGTRec.szNCCC_Contactless_Enable[0], &szCFGTRec[0], k - 1);
	}

	/* 64_是否強迫要輸入CID值 */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00) /* 因為是最後的欄位還要多判斷0x00 */
	{
		memcpy(&srCFGTRec.szFORCE_CID[0], &szCFGTRec[0], k - 1);
	}

	/* 65_信託交易是否要查核病患/入住者ID */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00) /* 因為是最後的欄位還要多判斷0x00 */
	{
		memcpy(&srCFGTRec.szCHECK_ID[0], &szCFGTRec[0], k - 1);
	}
	
	/* 66_[114197] 中心收單作業ESG永續發展優化 (Y=持卡人存根聯數位化) */
	/* 初始化 */
	memset(szCFGTRec, 0x00, sizeof (szCFGTRec));
	k = 0;

	/* 從Record中讀欄位資料出來 */
	while (1)
	{
		szCFGTRec[k++] = uszReadData[i++];
		if (szCFGTRec[k - 1] == 0x2C ||
			szCFGTRec[k - 1] == 0x0D ||
			szCFGTRec[k - 1] == 0x0A ||
			szCFGTRec[k - 1] == 0x00)
		{
			break;
		}

		if (i > lnCFGTLength)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "CFGT unpack szCHESG_FLAG ERROR");
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
	if (szCFGTRec[0] != 0x2C &&
		szCFGTRec[0] != 0x0D &&
		szCFGTRec[0] != 0x0A &&
		szCFGTRec[0] != 0x00) /* 因為是最後的欄位還要多判斷0x00 */
	{
		memcpy(&srCFGTRec.szCHESG_FLAG[0], &szCFGTRec[0], k - 1);
	}

	/* release */
	/* 關檔 */
	inFILE_Close(&ulFile_Handle);

	/* Free pointer */
	free(uszReadData);
	free(uszTemp);

	/* inLoadCFGTRec() END */
	if (ginDebug == VS_TRUE)
	{
		memset(szErrorMsg, 0x00, sizeof (szErrorMsg));
		sprintf(szErrorMsg, "inLoadCFGTRec(%d) END!!", inCFGTRec);
		inLogPrintf(AT, szErrorMsg);
		inLogPrintf(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inSaveCFGTRec
Date&Time       :2015/8/31 下午 2:00
Describe        :
*/
int inSaveCFGTRec(int inCFGTRec)
{
	unsigned long uldat_Handle; /* FILE Handle */
	unsigned long ulbak_Handle; /* FILE Handle */
	int inPackCount = 0; /* uszWriteBuff_Record的index */
	int inRecIndex = 0; /* uszRead_Total_Buff的index */
	int inCFGT_Total_Rec = 0; /* CFGT.dat的總筆數 */
	int inRetVal;
	int i = 0, j = 0;
	char szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1]; /* debug message */
	long lnReadLength = 0; /* 每次要從CFGT.dat讀多少byte出來 */
	long lnCFGTLength = 0; /* CFGT.dat檔案總長度 */
	unsigned char *uszRead_Total_Buff;
	unsigned char *uszWriteBuff_Record, *uszWriteBuff_Org;


	/* inSaveCFGTRec()_START */
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		memset(szErrorMsg, 0x00, sizeof (szErrorMsg));
		sprintf(szErrorMsg, "inSaveCFGTRec(%d) START!!", inCFGTRec);
		inLogPrintf(AT, szErrorMsg);
	}

	/* 防呆先刪除CFGT.bak  */
	inFILE_Delete((unsigned char *) _CFGT_FILE_NAME_BAK_);

	/* 新建CFGT.bak檔案 */
	inRetVal = inFILE_Create(&ulbak_Handle, (unsigned char *) _CFGT_FILE_NAME_BAK_);

	if (inRetVal != VS_SUCCESS)
	{
		inFILE_Close(&ulbak_Handle);

		return (VS_ERROR);
	}

	/* 開啟原檔案CFGT.dat */
	inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *) _CFGT_FILE_NAME_);

	if (inRetVal != VS_SUCCESS)
	{
		/* CFGT.dat開檔失敗 ，不用關檔CFGT.dat */
		/* CFGT.bak仍要關並刪除 */
		inFILE_Close(&ulbak_Handle);
		inFILE_Delete((unsigned char *) _CFGT_FILE_NAME_BAK_);

		return (VS_ERROR);
	}

	/* 取得CFGT.dat檔案大小 */
	lnCFGTLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *) _CFGT_FILE_NAME_);

	/* 取得檔案大小失敗 */
	if (lnCFGTLength == VS_ERROR)
	{
		/* CFGT.bak和CFGT.dat要關並刪除 */
		inFILE_Close(&ulbak_Handle);
		inFILE_Close(&uldat_Handle);
		inFILE_Delete((unsigned char *) _CFGT_FILE_NAME_BAK_);

		return (VS_ERROR);
	}

	/* 組Write Record封包 */
	/* 給WriteBuff記憶體大小 */
	uszWriteBuff_Record = malloc(_SIZE_CFGT_REC_ + _SIZE_CFGT_COMMA_0D0A_);
	memset(uszWriteBuff_Record, 0x00, _SIZE_CFGT_REC_ + _SIZE_CFGT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

	/* 存原CFGT.dat的每筆Record 初始化 */
	uszWriteBuff_Org = malloc(_SIZE_CFGT_REC_ + _SIZE_CFGT_COMMA_0D0A_);
	memset(uszWriteBuff_Org, 0x00, _SIZE_CFGT_REC_ + _SIZE_CFGT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */

	/* uszRead_Total_Buff儲存CFGT.dat全部資料 */
	uszRead_Total_Buff = malloc(lnCFGTLength + 1);
	memset(uszRead_Total_Buff, 0x00, lnCFGTLength + 1);

	inPackCount = 0; /* uszWriteBuff_Record的index位置 */
	/* 以下依照Structure宣告的順序依序將參數值塞入Buffer中並補上逗號 */

	/* CustomIndicator */
	memcpy(&uszWriteBuff_Record[0], &srCFGTRec.szCustomIndicator[0], strlen(srCFGTRec.szCustomIndicator));
	inPackCount += strlen(srCFGTRec.szCustomIndicator);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* NCCCFESMode */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szNCCCFESMode[0], strlen(srCFGTRec.szNCCCFESMode));
	inPackCount += strlen(srCFGTRec.szNCCCFESMode);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* CommMode */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szCommMode[0], strlen(srCFGTRec.szCommMode));
	inPackCount += strlen(srCFGTRec.szCommMode);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* DialBackupEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szDialBackupEnable[0], strlen(srCFGTRec.szDialBackupEnable));
	inPackCount += strlen(srCFGTRec.szDialBackupEnable);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* EncryptMode */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szEncryptMode[0], strlen(srCFGTRec.szEncryptMode));
	inPackCount += strlen(srCFGTRec.szEncryptMode);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* SplitTransCheckEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szSplitTransCheckEnable[0], strlen(srCFGTRec.szSplitTransCheckEnable));
	inPackCount += strlen(srCFGTRec.szSplitTransCheckEnable);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* CityName */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szCityName[0], strlen(srCFGTRec.szCityName));
	inPackCount += strlen(srCFGTRec.szCityName);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* StoreIDEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szStoreIDEnable[0], strlen(srCFGTRec.szStoreIDEnable));
	inPackCount += strlen(srCFGTRec.szStoreIDEnable);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* MinStoreIDLen */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szMinStoreIDLen[0], strlen(srCFGTRec.szMinStoreIDLen));
	inPackCount += strlen(srCFGTRec.szMinStoreIDLen);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* MaxStoreIDLen */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szMaxStoreIDLen[0], strlen(srCFGTRec.szMaxStoreIDLen));
	inPackCount += strlen(srCFGTRec.szMaxStoreIDLen);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* ECREnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szECREnable[0], strlen(srCFGTRec.szECREnable));
	inPackCount += strlen(srCFGTRec.szECREnable);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* ECRCardNoTruncateEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szECRCardNoTruncateEnable[0], strlen(srCFGTRec.szECRCardNoTruncateEnable));
	inPackCount += strlen(srCFGTRec.szECRCardNoTruncateEnable);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* ECRExpDateReturnEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szECRExpDateReturnEnable[0], strlen(srCFGTRec.szECRExpDateReturnEnable));
	inPackCount += strlen(srCFGTRec.szECRExpDateReturnEnable);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* ProductCodeEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szProductCodeEnable[0], strlen(srCFGTRec.szProductCodeEnable));
	inPackCount += strlen(srCFGTRec.szProductCodeEnable);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* PrtSlogan */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szPrtSlogan[0], strlen(srCFGTRec.szPrtSlogan));
	inPackCount += strlen(srCFGTRec.szPrtSlogan);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* SloganStartDate */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szSloganStartDate[0], strlen(srCFGTRec.szSloganStartDate));
	inPackCount += strlen(srCFGTRec.szSloganStartDate);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* SloganEndDate */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szSloganEndDate[0], strlen(srCFGTRec.szSloganEndDate));
	inPackCount += strlen(srCFGTRec.szSloganEndDate);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* SloganPrtPosition */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szSloganPrtPosition[0], strlen(srCFGTRec.szSloganPrtPosition));
	inPackCount += strlen(srCFGTRec.szSloganPrtPosition);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* PrtMode */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szPrtMode[0], strlen(srCFGTRec.szPrtMode));
	inPackCount += strlen(srCFGTRec.szPrtMode);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* ContactlessEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szContactlessEnable[0], strlen(srCFGTRec.szContactlessEnable));
	inPackCount += strlen(srCFGTRec.szContactlessEnable);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* VISAPaywaveEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szVISAPaywaveEnable[0], strlen(srCFGTRec.szVISAPaywaveEnable));
	inPackCount += strlen(srCFGTRec.szVISAPaywaveEnable);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* JCBJspeedyEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szJCBJspeedyEnable[0], strlen(srCFGTRec.szJCBJspeedyEnable));
	inPackCount += strlen(srCFGTRec.szJCBJspeedyEnable);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* MCPaypassEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szMCPaypassEnable[0], strlen(srCFGTRec.szMCPaypassEnable));
	inPackCount += strlen(srCFGTRec.szMCPaypassEnable);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* CUPRefundLimit */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szCUPRefundLimit[0], strlen(srCFGTRec.szCUPRefundLimit));
	inPackCount += strlen(srCFGTRec.szCUPRefundLimit);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* CUPKeyExchangeTimes */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szCUPKeyExchangeTimes[0], strlen(srCFGTRec.szCUPKeyExchangeTimes));
	inPackCount += strlen(srCFGTRec.szCUPKeyExchangeTimes);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* MACEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szMACEnable[0], strlen(srCFGTRec.szMACEnable));
	inPackCount += strlen(srCFGTRec.szMACEnable);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* PinpadMode */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szPinpadMode[0], strlen(srCFGTRec.szPinpadMode));
	inPackCount += strlen(srCFGTRec.szPinpadMode);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* FORCECVV2 */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szFORCECVV2[0], strlen(srCFGTRec.szFORCECVV2));
	inPackCount += strlen(srCFGTRec.szFORCECVV2);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* SpecialCardRangeEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szSpecialCardRangeEnable[0], strlen(srCFGTRec.szSpecialCardRangeEnable));
	inPackCount += strlen(srCFGTRec.szSpecialCardRangeEnable);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* PrtMerchantLogo */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szPrtMerchantLogo[0], strlen(srCFGTRec.szPrtMerchantLogo));
	inPackCount += strlen(srCFGTRec.szPrtMerchantLogo);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* PrtMerchantName */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szPrtMerchantName[0], strlen(srCFGTRec.szPrtMerchantName));
	inPackCount += strlen(srCFGTRec.szPrtMerchantName);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* PrtNotice */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szPrtNotice[0], strlen(srCFGTRec.szPrtNotice));
	inPackCount += strlen(srCFGTRec.szPrtNotice);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* ElecCommerceFlag */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szElecCommerceFlag[0], strlen(srCFGTRec.szElecCommerceFlag));
	inPackCount += strlen(srCFGTRec.szElecCommerceFlag);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* DccFlowVersion */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szDccFlowVersion[0], strlen(srCFGTRec.szDccFlowVersion));
	inPackCount += strlen(srCFGTRec.szDccFlowVersion);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* SupDccVisa */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szSupDccVisa[0], strlen(srCFGTRec.szSupDccVisa));
	inPackCount += strlen(srCFGTRec.szSupDccVisa);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* SupDccMasterCard */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szSupDccMasterCard[0], strlen(srCFGTRec.szSupDccMasterCard));
	inPackCount += strlen(srCFGTRec.szSupDccMasterCard);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* DHCPRetryTimes */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szDHCPRetryTimes[0], strlen(srCFGTRec.szDHCPRetryTimes));
	inPackCount += strlen(srCFGTRec.szDHCPRetryTimes);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* BarCodeReaderEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szBarCodeReaderEnable[0], strlen(srCFGTRec.szBarCodeReaderEnable));
	inPackCount += strlen(srCFGTRec.szBarCodeReaderEnable);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* EMVPINBypassEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szEMVPINBypassEnable[0], strlen(srCFGTRec.szEMVPINBypassEnable));
	inPackCount += strlen(srCFGTRec.szEMVPINBypassEnable);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* CUPOnlinePINEntryTimeout */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szCUPOnlinePINEntryTimeout[0], strlen(srCFGTRec.szCUPOnlinePINEntryTimeout));
	inPackCount += strlen(srCFGTRec.szCUPOnlinePINEntryTimeout);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* SignPadMode */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szSignPadMode[0], strlen(srCFGTRec.szSignPadMode));
	inPackCount += strlen(srCFGTRec.szSignPadMode);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* ESCPrintMerchantCopy */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szESCPrintMerchantCopy[0], strlen(srCFGTRec.szESCPrintMerchantCopy));
	inPackCount += strlen(srCFGTRec.szESCPrintMerchantCopy);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* ESCPrintMerchantCopyStartDate */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szESCPrintMerchantCopyStartDate[0], strlen(srCFGTRec.szESCPrintMerchantCopyStartDate));
	inPackCount += strlen(srCFGTRec.szESCPrintMerchantCopyStartDate);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* ESCPrintMerchantCopyEndDate */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szESCPrintMerchantCopyEndDate[0], strlen(srCFGTRec.szESCPrintMerchantCopyEndDate));
	inPackCount += strlen(srCFGTRec.szESCPrintMerchantCopyEndDate);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* ESCReciptUploadUpLimit */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szESCReciptUploadUpLimit[0], strlen(srCFGTRec.szESCReciptUploadUpLimit));
	inPackCount += strlen(srCFGTRec.szESCReciptUploadUpLimit);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* ContactlessReaderMode */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szContactlessReaderMode[0], strlen(srCFGTRec.szContactlessReaderMode));
	inPackCount += strlen(srCFGTRec.szContactlessReaderMode);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* TMSDownloadMode */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szTMSDownloadMode[0], strlen(srCFGTRec.szTMSDownloadMode));
	inPackCount += strlen(srCFGTRec.szTMSDownloadMode);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* AMEXContactlessEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szAMEXContactlessEnable[0], strlen(srCFGTRec.szAMEXContactlessEnable));
	inPackCount += strlen(srCFGTRec.szAMEXContactlessEnable);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* CUPContactlessEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szCUPContactlessEnable[0], strlen(srCFGTRec.szCUPContactlessEnable));
	inPackCount += strlen(srCFGTRec.szCUPContactlessEnable);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* SmartPayContactlessEnable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szSmartPayContactlessEnable[0], strlen(srCFGTRec.szSmartPayContactlessEnable));
	inPackCount += strlen(srCFGTRec.szSmartPayContactlessEnable);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* Pay_Item_Enable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szPay_Item_Enable[0], strlen(srCFGTRec.szPay_Item_Enable));
	inPackCount += strlen(srCFGTRec.szPay_Item_Enable);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* Store_Stub_CardNo_Truncate_Enable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szStore_Stub_CardNo_Truncate_Enable[0], strlen(srCFGTRec.szStore_Stub_CardNo_Truncate_Enable));
	inPackCount += strlen(srCFGTRec.szStore_Stub_CardNo_Truncate_Enable);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* Integrate_Device */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szIntegrate_Device[0], strlen(srCFGTRec.szIntegrate_Device));
	inPackCount += strlen(srCFGTRec.szIntegrate_Device);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* FES_ID */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szFES_ID[0], strlen(srCFGTRec.szFES_ID));
	inPackCount += strlen(srCFGTRec.szFES_ID);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* Integrate_Device_AP_ID */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szIntegrate_Device_AP_ID[0], strlen(srCFGTRec.szIntegrate_Device_AP_ID));
	inPackCount += strlen(srCFGTRec.szIntegrate_Device_AP_ID);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* Short_Receipt_Mode */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szShort_Receipt_Mode[0], strlen(srCFGTRec.szShort_Receipt_Mode));
	inPackCount += strlen(srCFGTRec.szShort_Receipt_Mode);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* I_FES_Mode */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szI_FES_Mode[0], strlen(srCFGTRec.szI_FES_Mode));
	inPackCount += strlen(srCFGTRec.szI_FES_Mode);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* DHCP_Mode */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szDHCP_Mode[0], strlen(srCFGTRec.szDHCP_Mode));
	inPackCount += strlen(srCFGTRec.szDHCP_Mode);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* ESVC_Priority */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szESVC_Priority[0], strlen(srCFGTRec.szESVC_Priority));
	inPackCount += strlen(srCFGTRec.szESVC_Priority);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* DFS_Contactless_Enable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szDFS_Contactless_Enable[0], strlen(srCFGTRec.szDFS_Contactless_Enable));
	inPackCount += strlen(srCFGTRec.szDFS_Contactless_Enable);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* Cloud_MFES */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szCloud_MFES[0], strlen(srCFGTRec.szCloud_MFES));
	inPackCount += strlen(srCFGTRec.szCloud_MFES);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* BIN_CHECK */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szBIN_CHECK[0], strlen(srCFGTRec.szBIN_CHECK));
	inPackCount += strlen(srCFGTRec.szBIN_CHECK);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* NCCC_Contactless_Enable */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szNCCC_Contactless_Enable[0], strlen(srCFGTRec.szNCCC_Contactless_Enable));
	inPackCount += strlen(srCFGTRec.szNCCC_Contactless_Enable);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* FORCE_CID */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szFORCE_CID[0], strlen(srCFGTRec.szFORCE_CID));
	inPackCount += strlen(srCFGTRec.szFORCE_CID);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;

	/* CHECK_ID */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szCHECK_ID[0], strlen(srCFGTRec.szCHECK_ID));
	inPackCount += strlen(srCFGTRec.szCHECK_ID);
	uszWriteBuff_Record[inPackCount] = 0x2C;
	inPackCount++;
	
	/* CHESG_FLAG [114197] 中心收單作業ESG永續發展優化 (Y=持卡人存根聯數位化) */
	memcpy(&uszWriteBuff_Record[inPackCount], &srCFGTRec.szCHESG_FLAG[0], strlen(srCFGTRec.szCHESG_FLAG));
	inPackCount += strlen(srCFGTRec.szCHESG_FLAG);

	/* 最後的data不用逗號 */
	/* 補上換行符號 */

	/* 0D是移至行首 */
	uszWriteBuff_Record[inPackCount] = 0x0D;
	inPackCount++;
	/* 0A是移至下一行 */
	uszWriteBuff_Record[inPackCount] = 0x0A;
	inPackCount++;

	/* 讀CFGT.dat檔案存到uszRead_Total_Buff */
	if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
	{
		lnReadLength = lnCFGTLength;

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
				}					/* 讀失敗時 */
				else
				{
					/* Close檔案 */
					inFILE_Close(&uldat_Handle);
					inFILE_Close(&ulbak_Handle);
					inFILE_Delete((unsigned char *) _CFGT_FILE_NAME_BAK_);

					/* Free pointer */
					free(uszRead_Total_Buff);
					free(uszWriteBuff_Record);
					free(uszWriteBuff_Org);

					return (VS_ERROR);
				}
			}				/* 剩餘長度小於1024 */
			else if (lnReadLength < 1024)
			{
				/* 就只讀剩餘長度 */
				if (inFILE_Read(&uldat_Handle, &uszRead_Total_Buff[1024 * i], lnReadLength) == VS_SUCCESS)
				{
					break;
				}					/* 讀失敗時 */
				else
				{
					/* Close檔案 */
					inFILE_Close(&uldat_Handle);
					inFILE_Close(&ulbak_Handle);
					inFILE_Delete((unsigned char *) _CFGT_FILE_NAME_BAK_);

					/* Free pointer */
					free(uszRead_Total_Buff);
					free(uszWriteBuff_Record);
					free(uszWriteBuff_Org);

					return (VS_ERROR);
				}
			}
		} /* end for loop */
	} else
	{
		/* Close檔案 */
		inFILE_Close(&uldat_Handle);
		inFILE_Close(&ulbak_Handle);
		inFILE_Delete((unsigned char *) _CFGT_FILE_NAME_BAK_);

		/* Free pointer */
		free(uszRead_Total_Buff);
		free(uszWriteBuff_Record);
		free(uszWriteBuff_Org);

		/* Seek失敗，所以回傳Error */
		return (VS_ERROR);
	}

	/* 防呆 總record數量小於要存取inCFGTRec Return ERROR */
	/* 算總Record數 */
	for (i = 0; i < (lnCFGTLength + 1); i++)
	{
		/* 一個換行，record數就+1 */
		if (uszRead_Total_Buff[i] == 0x0A)
			inCFGT_Total_Rec++;
	}

	/* 防呆 總record數量小於要存取inCFGTRec Return ERROR */
	if ((inCFGTRec + 1) > inCFGT_Total_Rec)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "No data or Index ERROR");
		}

		/* 關檔 */
		inFILE_Close(&uldat_Handle);
		inFILE_Close(&ulbak_Handle);
		inFILE_Delete((unsigned char *) _CFGT_FILE_NAME_BAK_);

		/* Free pointer */
		free(uszRead_Total_Buff);
		free(uszWriteBuff_Record);
		free(uszWriteBuff_Org);

		return (VS_ERROR);
	}

	/* 記錄ReadTotalBuffer的陣列位置 */
	inRecIndex = 0;
	/* 用inCFGTRec決定要先存幾筆Record到CFGT.bak，ex:inCFGTRec = 4，就會先存0~3的record進去bak檔 */
	for (i = 0; i < inCFGTRec; i++)
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
					inFILE_Delete((unsigned char *) _CFGT_FILE_NAME_BAK_);

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
					inFILE_Delete((unsigned char *) _CFGT_FILE_NAME_BAK_);

					/* Free pointer */
					free(uszWriteBuff_Record);
					free(uszWriteBuff_Org);
					free(uszRead_Total_Buff);

					return (VS_ERROR);
				}

				/* 清空buffer為下一筆寫入做準備 */
				memset(uszWriteBuff_Org, 0x00, sizeof (uszWriteBuff_Org));
				inRecIndex++;

				break;
			}

			/* 讀下一個字元 */
			inRecIndex++;
		}
	}

	/* 存組好的該CFGTRecord 到 CFGT.bak */
	inRetVal = inFILE_Seek(ulbak_Handle, 0, _SEEK_END_);

	if (inRetVal != VS_SUCCESS)
	{
		/* 關檔 */
		inFILE_Close(&uldat_Handle);
		inFILE_Close(&ulbak_Handle);
		inFILE_Delete((unsigned char *) _CFGT_FILE_NAME_BAK_);


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
		inFILE_Delete((unsigned char *) _CFGT_FILE_NAME_BAK_);

		/* Free pointer */
		free(uszWriteBuff_Record);
		free(uszWriteBuff_Org);
		free(uszRead_Total_Buff);

		return (VS_ERROR);
	}

	/* 存剩下的原CFGT.dat Record 到 CFGT.bak */
	/* 計算已存幾筆Record 移動到正確陣列位置 */
	/* inRecIndex要指到剩下還沒存進bak檔的第一個記憶體位置 */
	inCFGTRec = inCFGTRec + 1;
	j = 0;
	inRecIndex = 0;

	for (i = 0; i < lnCFGTLength; i++)
	{
		if (uszRead_Total_Buff[i] == 0x0A)
		{
			j++;

			if (j == inCFGTRec)
			{
				inRecIndex = i + 1;
				break;
			}
		}
	}

	i = 0;
	memset(uszWriteBuff_Org, 0x00, sizeof (uszWriteBuff_Org));

	while (1)
	{
		/* 接續存原CFGT.dat的Record */
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
				inFILE_Delete((unsigned char *) _CFGT_FILE_NAME_BAK_);

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
				inFILE_Delete((unsigned char *) _CFGT_FILE_NAME_BAK_);

				/* Free pointer */
				free(uszWriteBuff_Record);
				free(uszWriteBuff_Org);
				free(uszRead_Total_Buff);

				return (VS_ERROR);
			}

			memset(uszWriteBuff_Org, 0x00, sizeof (uszWriteBuff_Org));
			i = 0;
		} else if (uszRead_Total_Buff[inRecIndex] == 0x00)
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

	/* 刪除原CFGT.dat */
	if (inFILE_Delete((unsigned char *) _CFGT_FILE_NAME_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* 將CFGT.bak改名字為CFGT.dat取代原檔案 */
	if (inFILE_Rename((unsigned char *) _CFGT_FILE_NAME_BAK_, (unsigned char *) _CFGT_FILE_NAME_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* inSaveCFTRec() END */
	if (ginDebug == VS_TRUE)
	{
		memset(szErrorMsg, 0x00, sizeof (szErrorMsg));
		sprintf(szErrorMsg, "inSaveCFGTRec(%d) END!!", (inCFGTRec - 1));
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
Function        :inGetCustomIndicator
Date&Time       :
Describe        :
*/
int inGetCustomIndicator(char* szCustomIndicator)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szCustomIndicator == NULL || strlen(srCFGTRec.szCustomIndicator) <= 0 || strlen(srCFGTRec.szCustomIndicator) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetCustomIndicator() ERROR !!");

                        if (szCustomIndicator == NULL)
                        {
                                inLogPrintf(AT, "szCustomIndicator == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCustomIndicator length = (%d)", (int)strlen(srCFGTRec.szCustomIndicator));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }
                return (VS_ERROR);
        }
        memcpy(&szCustomIndicator[0], &srCFGTRec.szCustomIndicator[0], strlen(srCFGTRec.szCustomIndicator));

        return (VS_SUCCESS);
}

/*
Function        :inSetCustomIndicator
Date&Time       :
Describe        :
*/
int inSetCustomIndicator(char* szCustomIndicator)
{
        memset(srCFGTRec.szCustomIndicator, 0x00, sizeof(srCFGTRec.szCustomIndicator));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCustomIndicator == NULL || strlen(szCustomIndicator) <= 0 || strlen(szCustomIndicator) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCustomIndicator() ERROR !!");

                        if (szCustomIndicator == NULL)
                        {
                                inLogPrintf(AT, "szCustomIndicator == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCustomIndicator length = (%d)", (int)strlen(szCustomIndicator));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szCustomIndicator[0], &szCustomIndicator[0], strlen(szCustomIndicator));

        return (VS_SUCCESS);
}

/*
Function        :inGetNCCCFESMode
Date&Time       :
Describe        :
*/
int inGetNCCCFESMode(char* szNCCCFESMode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szNCCCFESMode == NULL || strlen(srCFGTRec.szNCCCFESMode) <= 0 || strlen(srCFGTRec.szNCCCFESMode) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetNCCCFESMode() ERROR !!");

                        if (szNCCCFESMode == NULL)
                        {
                                inLogPrintf(AT, "szNCCCFESMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szNCCCFESMode length = (%d)", (int)strlen(srCFGTRec.szNCCCFESMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szNCCCFESMode[0], &srCFGTRec.szNCCCFESMode[0], strlen(srCFGTRec.szNCCCFESMode));

        return (VS_SUCCESS);

}

/*
Function        :inSetNCCCFESMode
Date&Time       :
Describe        :
*/
int inSetNCCCFESMode(char* szNCCCFESMode)
{
        memset(srCFGTRec.szNCCCFESMode, 0x00, sizeof(srCFGTRec.szNCCCFESMode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szNCCCFESMode == NULL || strlen(szNCCCFESMode) <= 0 || strlen(szNCCCFESMode) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetNCCCFESMode() ERROR !!");

                        if (szNCCCFESMode == NULL)
                        {
                                inLogPrintf(AT, "szNCCCFESMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szNCCCFESMode length = (%d)", (int)strlen(szNCCCFESMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szNCCCFESMode[0], &szNCCCFESMode[0], strlen(szNCCCFESMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetCommMode
Date&Time       :
Describe        :
*/
int inGetCommMode(char* szCommMode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szCommMode == NULL || strlen(srCFGTRec.szCommMode) <= 0 || strlen(srCFGTRec.szCommMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetCommMode() ERROR !!");

                        if (szCommMode == NULL)
                        {
                                inLogPrintf(AT, "szCommMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCommMode length = (%d)", (int)strlen(srCFGTRec.szCommMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCommMode[0], &srCFGTRec.szCommMode[0], strlen(srCFGTRec.szCommMode));

        return (VS_SUCCESS);
}

/*
Function        :inSetCommMode
Date&Time       :
Describe        :
*/
int inSetCommMode(char* szCommMode)
{
        memset(srCFGTRec.szCommMode, 0x00, sizeof(srCFGTRec.szCommMode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCommMode == NULL || strlen(szCommMode) <= 0 || strlen(szCommMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCommMode() ERROR !!");

                        if (szCommMode == NULL)
                        {
                                inLogPrintf(AT, "szCommMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCommMode length = (%d)", (int)strlen(szCommMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szCommMode[0], &szCommMode[0], strlen(szCommMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetDialBackupEnable
Date&Time       :
Describe        :
*/
int inGetDialBackupEnable(char* szDialBackupEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szDialBackupEnable == NULL || strlen(srCFGTRec.szDialBackupEnable) <= 0 || strlen(srCFGTRec.szDialBackupEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetDialBackupEnable() ERROR !!");

                        if (szDialBackupEnable == NULL)
                        {
                                inLogPrintf(AT, "szDialBackupEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDialBackupEnable length = (%d)", (int)strlen(srCFGTRec.szDialBackupEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szDialBackupEnable[0], &srCFGTRec.szDialBackupEnable[0], strlen(srCFGTRec.szDialBackupEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetDialBackupEnable
Date&Time       :
Describe        :
*/
int inSetDialBackupEnable(char* szDialBackupEnable)
{
        memset(srCFGTRec.szDialBackupEnable, 0x00, sizeof(srCFGTRec.szDialBackupEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szDialBackupEnable == NULL || strlen(szDialBackupEnable) <= 0 || strlen(szDialBackupEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetDialBackupEnable() ERROR !!");

                        if (szDialBackupEnable == NULL)
                        {
                                inLogPrintf(AT, "szDialBackupEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDialBackupEnable length = (%d)", (int)strlen(szDialBackupEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szDialBackupEnable[0], &szDialBackupEnable[0], strlen(szDialBackupEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetEncryptMode
Date&Time       :
Describe        :
*/
int inGetEncryptMode(char* szEncryptMode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szEncryptMode == NULL || strlen(srCFGTRec.szEncryptMode) <= 0 || strlen(srCFGTRec.szEncryptMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetEncryptMode() ERROR !!");

                        if (szEncryptMode == NULL)
                        {
                                inLogPrintf(AT, "szEncryptMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEncryptMode length = (%d)", (int)strlen(srCFGTRec.szEncryptMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szEncryptMode[0], &srCFGTRec.szEncryptMode[0], strlen(srCFGTRec.szEncryptMode));

        return (VS_SUCCESS);
}

/*
Function        :inSetEncryptMode
Date&Time       :
Describe        :
*/
int inSetEncryptMode(char* szEncryptMode)
{
        memset(srCFGTRec.szEncryptMode, 0x00, sizeof(srCFGTRec.szEncryptMode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szEncryptMode == NULL || strlen(szEncryptMode) <= 0 || strlen(szEncryptMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetEncryptMode() ERROR !!");

                        if (szEncryptMode == NULL)
                        {
                                inLogPrintf(AT, "szEncryptMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEncryptMode length = (%d)", (int)strlen(szEncryptMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szEncryptMode[0], &szEncryptMode[0], strlen(szEncryptMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetSplitTransCheckEnable
Date&Time       :
Describe        :
*/
int inGetSplitTransCheckEnable(char* szSplitTransCheckEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szSplitTransCheckEnable == NULL || strlen(srCFGTRec.szSplitTransCheckEnable) <= 0 || strlen(srCFGTRec.szSplitTransCheckEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetSplitTransCheckEnable() ERROR !!");

                        if (szSplitTransCheckEnable == NULL)
                        {
                                inLogPrintf(AT, "szSplitTransCheckEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSplitTransCheckEnable length = (%d)", (int)strlen(srCFGTRec.szSplitTransCheckEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSplitTransCheckEnable[0], &srCFGTRec.szSplitTransCheckEnable[0], strlen(srCFGTRec.szSplitTransCheckEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetSplitTransCheckEnable
Date&Time       :
Describe        :
*/
int inSetSplitTransCheckEnable(char* szSplitTransCheckEnable)
{
        memset(srCFGTRec.szSplitTransCheckEnable, 0x00, sizeof(srCFGTRec.szSplitTransCheckEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szSplitTransCheckEnable == NULL || strlen(szSplitTransCheckEnable) <= 0 || strlen(szSplitTransCheckEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetSplitTransCheckEnable() ERROR !!");

                        if (szSplitTransCheckEnable == NULL)
                        {
                                inLogPrintf(AT, "szSplitTransCheckEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSplitTransCheckEnable length = (%d)", (int)strlen(szSplitTransCheckEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szSplitTransCheckEnable[0], &szSplitTransCheckEnable[0], strlen(szSplitTransCheckEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetCityName
Date&Time       :
Describe        :
*/
int inGetCityName(char* szCityName)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szCityName == NULL || strlen(srCFGTRec.szCityName) <= 0 || strlen(srCFGTRec.szCityName) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetCityName() ERROR !!");

                        if (szCityName == NULL)
                        {
                                inLogPrintf(AT, "szCityName == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCityName length = (%d)", (int)strlen(srCFGTRec.szCityName));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCityName[0], &srCFGTRec.szCityName[0], strlen(srCFGTRec.szCityName));

        return (VS_SUCCESS);
}

/*
Function        :inSetCityName
Date&Time       :
Describe        :
*/
int inSetCityName(char* szCityName)
{
        memset(srCFGTRec.szCityName, 0x00, sizeof(srCFGTRec.szCityName));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCityName == NULL || strlen(szCityName) <= 0 || strlen(szCityName) > 20)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCityName() ERROR !!");

                        if (szCityName == NULL)
                        {
                                inLogPrintf(AT, "szCityName == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCityName length = (%d)", (int)strlen(szCityName));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szCityName[0], &szCityName[0], strlen(szCityName));

        return (VS_SUCCESS);
}

/*
Function        :inGetStoreIDEnable
Date&Time       :
Describe        :
*/
int inGetStoreIDEnable(char* szStoreIDEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szStoreIDEnable == NULL || strlen(srCFGTRec.szStoreIDEnable) <= 0 || strlen(srCFGTRec.szStoreIDEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetStoreIDEnable() ERROR !!");

                        if (szStoreIDEnable == NULL)
                        {
                                inLogPrintf(AT, "szStoreIDEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szStoreIDEnable length = (%d)", (int)strlen(srCFGTRec.szStoreIDEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szStoreIDEnable[0], &srCFGTRec.szStoreIDEnable[0], strlen(srCFGTRec.szStoreIDEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetStoreIDEnable
Date&Time       :
Describe        :
*/
int inSetStoreIDEnable(char* szStoreIDEnable)
{
        memset(srCFGTRec.szStoreIDEnable, 0x00, sizeof(srCFGTRec.szStoreIDEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szStoreIDEnable == NULL || strlen(szStoreIDEnable) <= 0 || strlen(szStoreIDEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetStoreIDEnable() ERROR !!");

                        if (szStoreIDEnable == NULL)
                        {
                                inLogPrintf(AT, "szStoreIDEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szStoreIDEnable length = (%d)", (int)strlen(szStoreIDEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szStoreIDEnable[0], &szStoreIDEnable[0], strlen(szStoreIDEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetMinStoreIDLen
Date&Time       :
Describe        :
*/
int inGetMinStoreIDLen(char* szMinStoreIDLen)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szMinStoreIDLen == NULL || strlen(srCFGTRec.szMinStoreIDLen) <= 0 || strlen(srCFGTRec.szMinStoreIDLen) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetMinStoreIDLen() ERROR !!");

                        if (szMinStoreIDLen == NULL)
                        {
                                inLogPrintf(AT, "szMinStoreIDLen == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMinStoreIDLen length = (%d)", (int)strlen(srCFGTRec.szMinStoreIDLen));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMinStoreIDLen[0], &srCFGTRec.szMinStoreIDLen[0], strlen(srCFGTRec.szMinStoreIDLen));

        return (VS_SUCCESS);
}

/*
Function        :inSetMinStoreIDLen
Date&Time       :
Describe        :
*/
int inSetMinStoreIDLen(char* szMinStoreIDLen)
{
        memset(srCFGTRec.szMinStoreIDLen, 0x00, sizeof(srCFGTRec.szMinStoreIDLen));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szMinStoreIDLen == NULL || strlen(szMinStoreIDLen) <= 0 || strlen(szMinStoreIDLen) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetMinStoreIDLen() ERROR !!");

                        if (szMinStoreIDLen == NULL)
                        {
                                inLogPrintf(AT, "szMinStoreIDLen == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMinStoreIDLen length = (%d)", (int)strlen(szMinStoreIDLen));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szMinStoreIDLen[0], &szMinStoreIDLen[0], strlen(szMinStoreIDLen));

        return (VS_SUCCESS);
}

/*
Function        :inGetMaxStoreIDLen
Date&Time       :
Describe        :
*/
int inGetMaxStoreIDLen(char* szMaxStoreIDLen)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szMaxStoreIDLen == NULL || strlen(srCFGTRec.szMaxStoreIDLen) <= 0 || strlen(srCFGTRec.szMaxStoreIDLen) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetMaxStoreIDLen() ERROR !!");

                        if (szMaxStoreIDLen == NULL)
                        {
                                inLogPrintf(AT, "szMaxStoreIDLen == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMaxStoreIDLen length = (%d)", (int)strlen(srCFGTRec.szMaxStoreIDLen));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMaxStoreIDLen[0], &srCFGTRec.szMaxStoreIDLen[0], strlen(srCFGTRec.szMaxStoreIDLen));

        return (VS_SUCCESS);
}

/*
Function        :inSetMaxStoreIDLen
Date&Time       :
Describe        :
*/
int inSetMaxStoreIDLen(char* szMaxStoreIDLen)
{
        memset(srCFGTRec.szMaxStoreIDLen, 0x00, sizeof(srCFGTRec.szMaxStoreIDLen));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szMaxStoreIDLen == NULL || strlen(szMaxStoreIDLen) <= 0 || strlen(szMaxStoreIDLen) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetMaxStoreIDLen() ERROR !!");

                        if (szMaxStoreIDLen == NULL)
                        {
                                inLogPrintf(AT, "szMaxStoreIDLen == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMaxStoreIDLen length = (%d)", (int)strlen(szMaxStoreIDLen));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szMaxStoreIDLen[0], &szMaxStoreIDLen[0], strlen(szMaxStoreIDLen));

        return (VS_SUCCESS);
}

/*
Function        :inGetECREnable
Date&Time       :
Describe        :
*/
int inGetECREnable(char* szECREnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szECREnable == NULL || strlen(srCFGTRec.szECREnable) <= 0 || strlen(srCFGTRec.szECREnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetECREnable() ERROR !!");

                        if (szECREnable == NULL)
                        {
                                inLogPrintf(AT, "szECREnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szECREnable length = (%d)", (int)strlen(srCFGTRec.szECREnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szECREnable[0], &srCFGTRec.szECREnable[0], strlen(srCFGTRec.szECREnable));

        return (VS_SUCCESS);

}

/*
Function        :inSetECREnable
Date&Time       :
Describe        :
*/
int inSetECREnable(char* szECREnable)
{
        memset(srCFGTRec.szECREnable, 0x00, sizeof(srCFGTRec.szECREnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szECREnable == NULL || strlen(szECREnable) <= 0 || strlen(szECREnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetECREnable() ERROR !!");

                        if (szECREnable == NULL)
                        {
                                inLogPrintf(AT, "szECREnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szECREnable length = (%d)", (int)strlen(szECREnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szECREnable[0], &szECREnable[0], strlen(szECREnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetECRCardNoTruncateEnable
Date&Time       :
Describe        :
*/
int inGetECRCardNoTruncateEnable(char* szECRCardNoTruncateEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szECRCardNoTruncateEnable == NULL || strlen(srCFGTRec.szECRCardNoTruncateEnable) <= 0 || strlen(srCFGTRec.szECRCardNoTruncateEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetECRCardNoTruncateEnable() ERROR !!");

                        if (szECRCardNoTruncateEnable == NULL)
                        {
                                inLogPrintf(AT, "szECRCardNoTruncateEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szECRCardNoTruncateEnable length = (%d)", (int)strlen(srCFGTRec.szECRCardNoTruncateEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szECRCardNoTruncateEnable[0], &srCFGTRec.szECRCardNoTruncateEnable[0], strlen(srCFGTRec.szECRCardNoTruncateEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetECRCardNoTruncateEnable
Date&Time       :
Describe        :
*/
int inSetECRCardNoTruncateEnable(char* szECRCardNoTruncateEnable)
{
        memset(srCFGTRec.szECRCardNoTruncateEnable, 0x00, sizeof(srCFGTRec.szECRCardNoTruncateEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szECRCardNoTruncateEnable == NULL || strlen(szECRCardNoTruncateEnable) <= 0 || strlen(szECRCardNoTruncateEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetECRCardNoTruncateEnable() ERROR !!");

                        if (szECRCardNoTruncateEnable == NULL)
                        {
                                inLogPrintf(AT, "szECRCardNoTruncateEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szECRCardNoTruncateEnable length = (%d)", (int)strlen(szECRCardNoTruncateEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szECRCardNoTruncateEnable[0], &szECRCardNoTruncateEnable[0], strlen(szECRCardNoTruncateEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetECRExpDateReturnEnable
Date&Time       :
Describe        :
*/
int inGetECRExpDateReturnEnable(char* szECRExpDateReturnEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szECRExpDateReturnEnable == NULL || strlen(srCFGTRec.szECRExpDateReturnEnable) <= 0 || strlen(srCFGTRec.szECRExpDateReturnEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetECRExpDateReturnEnable() ERROR !!");

                        if (szECRExpDateReturnEnable == NULL)
                        {
                                inLogPrintf(AT, "szECRExpDateReturnEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szECRExpDateReturnEnable length = (%d)", (int)strlen(srCFGTRec.szECRExpDateReturnEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szECRExpDateReturnEnable[0], &srCFGTRec.szECRExpDateReturnEnable[0], strlen(srCFGTRec.szECRExpDateReturnEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetECRExpDateReturnEnable
Date&Time       :
Describe        :
*/
int inSetECRExpDateReturnEnable(char* szECRExpDateReturnEnable)
{
        memset(srCFGTRec.szECRExpDateReturnEnable, 0x00, sizeof(srCFGTRec.szECRExpDateReturnEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szECRExpDateReturnEnable == NULL || strlen(szECRExpDateReturnEnable) <= 0 || strlen(szECRExpDateReturnEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetECRExpDateReturnEnable() ERROR !!");

                        if (szECRExpDateReturnEnable == NULL)
                        {
                                inLogPrintf(AT, "szECRExpDateReturnEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szECRExpDateReturnEnable length = (%d)", (int)strlen(szECRExpDateReturnEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szECRExpDateReturnEnable[0], &szECRExpDateReturnEnable[0], strlen(szECRExpDateReturnEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetProductCodeEnable
Date&Time       :
Describe        :
*/
int inGetProductCodeEnable(char* szProductCodeEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szProductCodeEnable == NULL || strlen(srCFGTRec.szProductCodeEnable) <= 0 || strlen(srCFGTRec.szProductCodeEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetProductCodeEnable() ERROR !!");

                        if (szProductCodeEnable == NULL)
                        {
                                inLogPrintf(AT, "szProductCodeEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szProductCodeEnable length = (%d)", (int)strlen(srCFGTRec.szProductCodeEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szProductCodeEnable[0], &srCFGTRec.szProductCodeEnable[0], strlen(srCFGTRec.szProductCodeEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetProductCodeEnable
Date&Time       :
Describe        :
*/
int inSetProductCodeEnable(char* szProductCodeEnable)
{
        memset(srCFGTRec.szProductCodeEnable, 0x00, sizeof(srCFGTRec.szProductCodeEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szProductCodeEnable == NULL || strlen(szProductCodeEnable) <= 0 || strlen(szProductCodeEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetProductCodeEnable() ERROR !!");

                        if (szProductCodeEnable == NULL)
                        {
                                inLogPrintf(AT, "szProductCodeEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szProductCodeEnable length = (%d)", (int)strlen(szProductCodeEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szProductCodeEnable[0], &szProductCodeEnable[0], strlen(szProductCodeEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetPrtSlogan
Date&Time       :
Describe        :
*/
int inGetPrtSlogan(char* szPrtSlogan)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPrtSlogan == NULL || strlen(srCFGTRec.szPrtSlogan) <= 0 || strlen(srCFGTRec.szPrtSlogan) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPrtSlogan() ERROR !!");

                        if (szPrtSlogan == NULL)
                        {
                                inLogPrintf(AT, "szPrtSlogan == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrtSlogan length = (%d)", (int)strlen(srCFGTRec.szPrtSlogan));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPrtSlogan[0], &srCFGTRec.szPrtSlogan[0], strlen(srCFGTRec.szPrtSlogan));

        return (VS_SUCCESS);
}

/*
Function        :inSetPrtSlogan
Date&Time       :
Describe        :
*/
int inSetPrtSlogan(char* szPrtSlogan)
{
        memset(srCFGTRec.szPrtSlogan, 0x00, sizeof(srCFGTRec.szPrtSlogan));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPrtSlogan == NULL || strlen(szPrtSlogan) <= 0 || strlen(szPrtSlogan) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPrtSlogan() ERROR !!");

                        if (szPrtSlogan == NULL)
                        {
                                inLogPrintf(AT, "szPrtSlogan == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrtSlogan length = (%d)", (int)strlen(szPrtSlogan));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szPrtSlogan[0], &szPrtSlogan[0], strlen(szPrtSlogan));

        return (VS_SUCCESS);
}

/*
Function        :inGetSloganStartDate
Date&Time       :
Describe        :
*/
int inGetSloganStartDate(char* szSloganStartDate)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szSloganStartDate == NULL || strlen(srCFGTRec.szSloganStartDate) <= 0 || strlen(srCFGTRec.szSloganStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetSloganStartDate() ERROR !!");

                        if (szSloganStartDate == NULL)
                        {
                                inLogPrintf(AT, "szSloganStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSloganStartDate length = (%d)", (int)strlen(srCFGTRec.szSloganStartDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSloganStartDate[0], &srCFGTRec.szSloganStartDate[0], strlen(srCFGTRec.szSloganStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetSloganStartDate
Date&Time       :
Describe        :
*/
int inSetSloganStartDate(char* szSloganStartDate)
{
        memset(srCFGTRec.szSloganStartDate, 0x00, sizeof(srCFGTRec.szSloganStartDate));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szSloganStartDate == NULL || strlen(szSloganStartDate) <= 0 || strlen(szSloganStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetSloganStartDate() ERROR !!");

                        if (szSloganStartDate == NULL)
                        {
                                inLogPrintf(AT, "szSloganStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSloganStartDate length = (%d)", (int)strlen(szSloganStartDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szSloganStartDate[0], &szSloganStartDate[0], strlen(szSloganStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetSloganEndDate
Date&Time       :
Describe        :
*/
int inGetSloganEndDate(char* szSloganEndDate)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szSloganEndDate == NULL || strlen(srCFGTRec.szSloganEndDate) <= 0 || strlen(srCFGTRec.szSloganEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetSloganEndDate() ERROR !!");

                        if (szSloganEndDate == NULL)
                        {
                                inLogPrintf(AT, "szSloganEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSloganEndDate length = (%d)", (int)strlen(srCFGTRec.szSloganEndDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSloganEndDate[0], &srCFGTRec.szSloganEndDate[0], strlen(srCFGTRec.szSloganEndDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetSloganEndDate
Date&Time       :
Describe        :
*/
int inSetSloganEndDate(char* szSloganEndDate)
{
        memset(srCFGTRec.szSloganEndDate, 0x00, sizeof(srCFGTRec.szSloganEndDate));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szSloganEndDate == NULL || strlen(szSloganEndDate) <= 0 || strlen(szSloganEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetSloganEndDate() ERROR !!");

                        if (szSloganEndDate == NULL)
                        {
                                inLogPrintf(AT, "szSloganEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSloganEndDate length = (%d)", (int)strlen(szSloganEndDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szSloganEndDate[0], &szSloganEndDate[0], strlen(szSloganEndDate));

        return (VS_SUCCESS);
}

int inGetSloganPrtPosition(char* szSloganPrtPosition)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szSloganPrtPosition == NULL || strlen(srCFGTRec.szSloganPrtPosition) <= 0 || strlen(srCFGTRec.szSloganPrtPosition) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetSloganPrtPosition() ERROR !!");

                        if (szSloganPrtPosition == NULL)
                        {
                                inLogPrintf(AT, "szSloganPrtPosition == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSloganPrtPosition length = (%d)", (int)strlen(srCFGTRec.szSloganPrtPosition));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSloganPrtPosition[0], &srCFGTRec.szSloganPrtPosition[0], strlen(srCFGTRec.szSloganPrtPosition));

        return (VS_SUCCESS);
}

int inSetSloganPrtPosition(char* szSloganPrtPosition)
{
        memset(srCFGTRec.szSloganPrtPosition, 0x00, sizeof(srCFGTRec.szSloganPrtPosition));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szSloganPrtPosition == NULL || strlen(szSloganPrtPosition) <= 0 || strlen(szSloganPrtPosition) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        if (szSloganPrtPosition == NULL)
                        {
                                inLogPrintf(AT, "inSetSloganPrtPositio() ERROR !! szSloganPrtPosition == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "inSetSloganPrtPositio() ERROR !! szSloganPrtPosition length = (%d)", (int)strlen(szSloganPrtPosition));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szSloganPrtPosition[0], &szSloganPrtPosition[0], strlen(szSloganPrtPosition));

        return (VS_SUCCESS);
}

/*
Function        :inGetPrtMode
Date&Time       :
Describe        :
*/
int inGetPrtMode(char* szPrtMode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPrtMode == NULL || strlen(srCFGTRec.szPrtMode) <= 0 || strlen(srCFGTRec.szPrtMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPrtMode() ERROR !!");

                        if (szPrtMode == NULL)
                        {
                                inLogPrintf(AT, "szPrtMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrtMode length = (%d)", (int)strlen(srCFGTRec.szPrtMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPrtMode[0], &srCFGTRec.szPrtMode[0], strlen(srCFGTRec.szPrtMode));

        return (VS_SUCCESS);
}

/*
Function        :inSetPrtMode
Date&Time       :
Describe        :
*/
int inSetPrtMode(char* szPrtMode)
{
        memset(srCFGTRec.szPrtMode, 0x00, sizeof(srCFGTRec.szPrtMode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPrtMode == NULL || strlen(szPrtMode) <= 0 || strlen(szPrtMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPrtMode() ERROR !!");

                        if (szPrtMode == NULL)
                        {
                                inLogPrintf(AT, "szPrtMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrtMode length = (%d)", (int)strlen(szPrtMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szPrtMode[0], &szPrtMode[0], strlen(szPrtMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetContactlessEnable
Date&Time       :
Describe        :
*/
int inGetContactlessEnable(char* szContactlessEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szContactlessEnable == NULL || strlen(srCFGTRec.szContactlessEnable) <= 0 || strlen(srCFGTRec.szContactlessEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetContactlessEnable() ERROR !!");

                        if (szContactlessEnable == NULL)
                        {
                                inLogPrintf(AT, "szContactlessEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szContactlessEnable length = (%d)", (int)strlen(srCFGTRec.szContactlessEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szContactlessEnable[0], &srCFGTRec.szContactlessEnable[0], strlen(srCFGTRec.szContactlessEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetContactlessEnable
Date&Time       :
Describe        :
*/
int inSetContactlessEnable(char* szContactlessEnable)
{
        memset(srCFGTRec.szContactlessEnable, 0x00, sizeof(srCFGTRec.szContactlessEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szContactlessEnable == NULL || strlen(szContactlessEnable) <= 0 || strlen(szContactlessEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetContactlessEnable() ERROR !!");

                        if (szContactlessEnable == NULL)
                        {
                                inLogPrintf(AT, "szContactlessEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szContactlessEnable length = (%d)", (int)strlen(szContactlessEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szContactlessEnable[0], &szContactlessEnable[0], strlen(szContactlessEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetVISAPaywaveEnable
Date&Time       :
Describe        :
*/
int inGetVISAPaywaveEnable(char* szVISAPaywaveEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szVISAPaywaveEnable == NULL || strlen(srCFGTRec.szVISAPaywaveEnable) <= 0 || strlen(srCFGTRec.szVISAPaywaveEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetVISAPaywaveEnable() ERROR !!");

                        if (szVISAPaywaveEnable == NULL)
                        {
                                inLogPrintf(AT, "szVISAPaywaveEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szVISAPaywaveEnable length = (%d)", (int)strlen(srCFGTRec.szVISAPaywaveEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szVISAPaywaveEnable[0], &srCFGTRec.szVISAPaywaveEnable[0], strlen(srCFGTRec.szVISAPaywaveEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetVISAPaywaveEnable
Date&Time       :
Describe        :
*/
int inSetVISAPaywaveEnable(char* szVISAPaywaveEnable)
{
        memset(srCFGTRec.szVISAPaywaveEnable, 0x00, sizeof(srCFGTRec.szVISAPaywaveEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szVISAPaywaveEnable == NULL || strlen(szVISAPaywaveEnable) <= 0 || strlen(szVISAPaywaveEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetVISAPaywaveEnable() ERROR !!");

                        if (szVISAPaywaveEnable == NULL)
                        {
                                inLogPrintf(AT, "szVISAPaywaveEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szVISAPaywaveEnable length = (%d)", (int)strlen(szVISAPaywaveEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szVISAPaywaveEnable[0], &szVISAPaywaveEnable[0], strlen(szVISAPaywaveEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetJCBJspeedyEnable
Date&Time       :
Describe        :
*/
int inGetJCBJspeedyEnable(char* szJCBJspeedyEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szJCBJspeedyEnable == NULL || strlen(srCFGTRec.szJCBJspeedyEnable) <= 0 || strlen(srCFGTRec.szJCBJspeedyEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetJCBJspeedyEnable() ERROR !!");

                        if (szJCBJspeedyEnable == NULL)
                        {
                                inLogPrintf(AT, "szJCBJspeedyEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szJCBJspeedyEnable length = (%d)", (int)strlen(srCFGTRec.szJCBJspeedyEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szJCBJspeedyEnable[0], &srCFGTRec.szJCBJspeedyEnable[0], strlen(srCFGTRec.szJCBJspeedyEnable));

        return (VS_SUCCESS);

}

/*
Function        :inSetJCBJspeedyEnable
Date&Time       :
Describe        :
*/
int inSetJCBJspeedyEnable(char* szJCBJspeedyEnable)
{
        memset(srCFGTRec.szJCBJspeedyEnable, 0x00, sizeof(srCFGTRec.szJCBJspeedyEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szJCBJspeedyEnable == NULL || strlen(szJCBJspeedyEnable) <= 0 || strlen(szJCBJspeedyEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetJCBJspeedyEnable() ERROR !!");

                        if (szJCBJspeedyEnable == NULL)
                        {
                                inLogPrintf(AT, "szJCBJspeedyEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szJCBJspeedyEnable length = (%d)", (int)strlen(szJCBJspeedyEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szJCBJspeedyEnable[0], &szJCBJspeedyEnable[0], strlen(szJCBJspeedyEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetMCPaypassEnable
Date&Time       :
Describe        :
*/
int inGetMCPaypassEnable(char* szMCPaypassEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szMCPaypassEnable == NULL || strlen(srCFGTRec.szMCPaypassEnable) <= 0 || strlen(srCFGTRec.szMCPaypassEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetMCPaypassEnable() ERROR !!");

                        if (szMCPaypassEnable == NULL)
                        {
                                inLogPrintf(AT, "szMCPaypassEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMCPaypassEnable length = (%d)", (int)strlen(srCFGTRec.szMCPaypassEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMCPaypassEnable[0], &srCFGTRec.szMCPaypassEnable[0], strlen(srCFGTRec.szMCPaypassEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetMCPaypassEnable
Date&Time       :
Describe        :
*/
int inSetMCPaypassEnable(char* szMCPaypassEnable)
{
        memset(srCFGTRec.szMCPaypassEnable, 0x00, sizeof(srCFGTRec.szMCPaypassEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szMCPaypassEnable == NULL || strlen(szMCPaypassEnable) <= 0 || strlen(szMCPaypassEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetMCPaypassEnable() ERROR !!");

                        if (szMCPaypassEnable == NULL)
                        {
                                inLogPrintf(AT, "szMCPaypassEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMCPaypassEnable length = (%d)", (int)strlen(szMCPaypassEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szMCPaypassEnable[0], &szMCPaypassEnable[0], strlen(szMCPaypassEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetCUPRefundLimit
Date&Time       :
Describe        :
*/
int inGetCUPRefundLimit(char* szCUPRefundLimit)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szCUPRefundLimit == NULL || strlen(srCFGTRec.szCUPRefundLimit) <= 0 || strlen(srCFGTRec.szCUPRefundLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetCUPRefundLimit() ERROR !!");

                        if (szCUPRefundLimit == NULL)
                        {
                                inLogPrintf(AT, "szCUPRefundLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCUPRefundLimit length = (%d)", (int)strlen(srCFGTRec.szCUPRefundLimit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCUPRefundLimit[0], &srCFGTRec.szCUPRefundLimit[0], strlen(srCFGTRec.szCUPRefundLimit));

        return (VS_SUCCESS);
}

/*
Function        :inSetCUPRefundLimit
Date&Time       :
Describe        :
*/
int inSetCUPRefundLimit(char* szCUPRefundLimit)
{
        memset(srCFGTRec.szCUPRefundLimit, 0x00, sizeof(srCFGTRec.szCUPRefundLimit));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCUPRefundLimit == NULL || strlen(szCUPRefundLimit) <= 0 || strlen(szCUPRefundLimit) > 12)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCUPRefundLimit() ERROR !!");

                        if (szCUPRefundLimit == NULL)
                        {
                                inLogPrintf(AT, "szCUPRefundLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCUPRefundLimit length = (%d)", (int)strlen(szCUPRefundLimit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szCUPRefundLimit[0], &szCUPRefundLimit[0], strlen(szCUPRefundLimit));

        return (VS_SUCCESS);
}

/*
Function        :inGetCUPKeyExchangeTimes
Date&Time       :
Describe        :
*/
int inGetCUPKeyExchangeTimes(char* szCUPKeyExchangeTimes)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szCUPKeyExchangeTimes == NULL || strlen(srCFGTRec.szCUPKeyExchangeTimes) <= 0 || strlen(srCFGTRec.szCUPKeyExchangeTimes) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetCUPKeyExchangeTimes() ERROR !!");

                        if (szCUPKeyExchangeTimes == NULL)
                        {
                                inLogPrintf(AT, "szCUPKeyExchangeTimes == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCUPKeyExchangeTimes length = (%d)", (int)strlen(srCFGTRec.szCUPKeyExchangeTimes));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCUPKeyExchangeTimes[0], &srCFGTRec.szCUPKeyExchangeTimes[0], strlen(srCFGTRec.szCUPKeyExchangeTimes));

        return (VS_SUCCESS);
}

/*
Function        :inSetCUPKeyExchangeTimes
Date&Time       :
Describe        :
*/
int inSetCUPKeyExchangeTimes(char* szCUPKeyExchangeTimes)
{
        memset(srCFGTRec.szCUPKeyExchangeTimes, 0x00, sizeof(srCFGTRec.szCUPKeyExchangeTimes));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCUPKeyExchangeTimes == NULL || strlen(szCUPKeyExchangeTimes) <= 0 || strlen(szCUPKeyExchangeTimes) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCUPKeyExchangeTimes() ERROR !!");

                        if (szCUPKeyExchangeTimes == NULL)
                        {
                                inLogPrintf(AT, "szCUPKeyExchangeTimes == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCUPKeyExchangeTimes length = (%d)", (int)strlen(szCUPKeyExchangeTimes));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szCUPKeyExchangeTimes[0], &szCUPKeyExchangeTimes[0], strlen(szCUPKeyExchangeTimes));

        return (VS_SUCCESS);
}

/*
Function        :inGetMACEnable
Date&Time       :
Describe        :
*/
int inGetMACEnable(char* szMACEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szMACEnable == NULL || strlen(srCFGTRec.szMACEnable) <= 0 || strlen(srCFGTRec.szMACEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetMACEnable() ERROR !!");

                        if (szMACEnable == NULL)
                        {
                                inLogPrintf(AT, "szMACEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMACEnable length = (%d)", (int)strlen(srCFGTRec.szMACEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szMACEnable[0], &srCFGTRec.szMACEnable[0], strlen(srCFGTRec.szMACEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetMACEnable
Date&Time       :
Describe        :
*/
int inSetMACEnable(char* szMACEnable)
{
        memset(srCFGTRec.szMACEnable, 0x00, sizeof(srCFGTRec.szMACEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szMACEnable == NULL || strlen(szMACEnable) <= 0 || strlen(szMACEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetMACEnable() ERROR !!");

                        if (szMACEnable == NULL)
                        {
                                inLogPrintf(AT, "szMACEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szMACEnable length = (%d)", (int)strlen(szMACEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szMACEnable[0], &szMACEnable[0], strlen(szMACEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetPinpadMode
Date&Time       :
Describe        :
*/
int inGetPinpadMode(char* szPinpadMode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPinpadMode == NULL || strlen(srCFGTRec.szPinpadMode) <= 0 || strlen(srCFGTRec.szPinpadMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPinpadMode() ERROR !!");

                        if (szPinpadMode == NULL)
                        {
                                inLogPrintf(AT, "szPinpadMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPinpadMode length = (%d)", (int)strlen(srCFGTRec.szPinpadMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPinpadMode[0], &srCFGTRec.szPinpadMode[0], strlen(srCFGTRec.szPinpadMode));

        return (VS_SUCCESS);
}

/*
Function        :inSetPinpadMode
Date&Time       :
Describe        :
*/
int inSetPinpadMode(char* szPinpadMode)
{
        memset(srCFGTRec.szPinpadMode, 0x00, sizeof(srCFGTRec.szPinpadMode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPinpadMode == NULL || strlen(szPinpadMode) <= 0 || strlen(szPinpadMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPinpadMode() ERROR !!");

                        if (szPinpadMode == NULL)
                        {
                                inLogPrintf(AT, "szPinpadMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPinpadMode length = (%d)", (int)strlen(szPinpadMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szPinpadMode[0], &szPinpadMode[0], strlen(szPinpadMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetFORCECVV2
Date&Time       :
Describe        :
*/
int inGetFORCECVV2(char* szFORCECVV2)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szFORCECVV2 == NULL || strlen(srCFGTRec.szFORCECVV2) <= 0 || strlen(srCFGTRec.szFORCECVV2) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetFORCECVV2() ERROR !!");

                        if (szFORCECVV2 == NULL)
                        {
                                inLogPrintf(AT, "szFORCECVV2 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFORCECVV2 length = (%d)", (int)strlen(srCFGTRec.szFORCECVV2));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFORCECVV2[0], &srCFGTRec.szFORCECVV2[0], strlen(srCFGTRec.szFORCECVV2));

        return (VS_SUCCESS);
}

/*
Function        :inSetFORCECVV2
Date&Time       :
Describe        :
*/
int inSetFORCECVV2(char* szFORCECVV2)
{
        memset(srCFGTRec.szFORCECVV2, 0x00, sizeof(srCFGTRec.szFORCECVV2));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFORCECVV2 == NULL || strlen(szFORCECVV2) <= 0 || strlen(szFORCECVV2) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetFORCECVV2() ERROR !!");

                        if (szFORCECVV2 == NULL)
                        {
                                inLogPrintf(AT, "szFORCECVV2 == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFORCECVV2 length = (%d)", (int)strlen(szFORCECVV2));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szFORCECVV2[0], &szFORCECVV2[0], strlen(szFORCECVV2));

        return (VS_SUCCESS);
}

/*
Function        :inGetSpecialCardRangeEnable
Date&Time       :
Describe        :
*/
int inGetSpecialCardRangeEnable(char* szSpecialCardRangeEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szSpecialCardRangeEnable == NULL || strlen(srCFGTRec.szSpecialCardRangeEnable) <= 0 || strlen(srCFGTRec.szSpecialCardRangeEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetszSpecialCardRangeEnable() ERROR !!");

                        if (szSpecialCardRangeEnable == NULL)
                        {
                                inLogPrintf(AT, "szSpecialCardRangeEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSpecialCardRangeEnable length = (%d)", (int)strlen(srCFGTRec.szSpecialCardRangeEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSpecialCardRangeEnable[0], &srCFGTRec.szSpecialCardRangeEnable[0], strlen(srCFGTRec.szSpecialCardRangeEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetSpecialCardRangeEnable
Date&Time       :
Describe        :
*/
int inSetSpecialCardRangeEnable(char* szSpecialCardRangeEnable)
{
        memset(srCFGTRec.szSpecialCardRangeEnable, 0x00, sizeof(srCFGTRec.szSpecialCardRangeEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szSpecialCardRangeEnable == NULL || strlen(szSpecialCardRangeEnable) <= 0 || strlen(szSpecialCardRangeEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetSpecialCardRangeEnable() ERROR !!");

                        if (szSpecialCardRangeEnable == NULL)
                        {
                                inLogPrintf(AT, "szSpecialCardRangeEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSpecialCardRangeEnable length = (%d)", (int)strlen(szSpecialCardRangeEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szSpecialCardRangeEnable[0], &szSpecialCardRangeEnable[0], strlen(szSpecialCardRangeEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetPrtMerchantLogo
Date&Time       :
Describe        :
*/
int inGetPrtMerchantLogo(char* szPrtMerchantLogo)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPrtMerchantLogo == NULL || strlen(srCFGTRec.szPrtMerchantLogo) <= 0 || strlen(srCFGTRec.szPrtMerchantLogo) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPrtMerchantLogo() ERROR !!");

                        if (szPrtMerchantLogo == NULL)
                        {
                                inLogPrintf(AT, "szPrtMerchantLogo == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrtMerchantLogo length = (%d)", (int)strlen(srCFGTRec.szPrtMerchantLogo));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPrtMerchantLogo[0], &srCFGTRec.szPrtMerchantLogo[0], strlen(srCFGTRec.szPrtMerchantLogo));

        return (VS_SUCCESS);
}

/*
Function        :inSetPrtMerchantLogo
Date&Time       :
Describe        :
*/
int inSetPrtMerchantLogo(char* szPrtMerchantLogo)
{
        memset(srCFGTRec.szPrtMerchantLogo, 0x00, sizeof(srCFGTRec.szPrtMerchantLogo));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPrtMerchantLogo == NULL || strlen(szPrtMerchantLogo) <= 0 || strlen(szPrtMerchantLogo) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPrtMerchantLogo() ERROR !!");

                        if (szPrtMerchantLogo == NULL)
                        {
                                inLogPrintf(AT, "szPrtMerchantLogo == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrtMerchantLogo length = (%d)", (int)strlen(szPrtMerchantLogo));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szPrtMerchantLogo[0], &szPrtMerchantLogo[0], strlen(szPrtMerchantLogo));

        return (VS_SUCCESS);
}

/*
Function        :inGetPrtMerchantName
Date&Time       :
Describe        :
*/
int inGetPrtMerchantName(char* szPrtMerchantName)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPrtMerchantName == NULL || strlen(srCFGTRec.szPrtMerchantName) <= 0 || strlen(srCFGTRec.szPrtMerchantName) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPrtMerchantName() ERROR !!");

                        if (szPrtMerchantName == NULL)
                        {
                                inLogPrintf(AT, "szPrtMerchantName == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrtMerchantName length = (%d)", (int)strlen(srCFGTRec.szPrtMerchantName));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPrtMerchantName[0], &srCFGTRec.szPrtMerchantName[0], strlen(srCFGTRec.szPrtMerchantName));

        return (VS_SUCCESS);
}

/*
Function        :inSetPrtMerchantName
Date&Time       :
Describe        :
*/
int inSetPrtMerchantName(char* szPrtMerchantName)
{
        memset(srCFGTRec.szPrtMerchantName, 0x00, sizeof(srCFGTRec.szPrtMerchantName));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPrtMerchantName == NULL || strlen(szPrtMerchantName) <= 0 || strlen(szPrtMerchantName) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPrtMerchantName() ERROR !!");

                        if (szPrtMerchantName == NULL)
                        {
                                inLogPrintf(AT, "szPrtMerchantName == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrtMerchantName length = (%d)", (int)strlen(szPrtMerchantName));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szPrtMerchantName[0], &szPrtMerchantName[0], strlen(szPrtMerchantName));

        return (VS_SUCCESS);
}

/*
Function        :inGetPrtNotice
Date&Time       :
Describe        :
*/
int inGetPrtNotice(char* szPrtNotice)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPrtNotice == NULL || strlen(srCFGTRec.szPrtNotice) <= 0 || strlen(srCFGTRec.szPrtNotice) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPrtNotice() ERROR !!");

                        if (szPrtNotice == NULL)
                        {
                                inLogPrintf(AT, "szPrtNotice == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrtNotice length = (%d)", (int)strlen(srCFGTRec.szPrtNotice));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPrtNotice[0], &srCFGTRec.szPrtNotice[0], strlen(srCFGTRec.szPrtNotice));

        return (VS_SUCCESS);

}

/*
Function        :inSetPrtNotice
Date&Time       :
Describe        :
*/
int inSetPrtNotice(char* szPrtNotice)
{
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPrtNotice == NULL || strlen(szPrtNotice) <= 0 || strlen(szPrtNotice) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPrtNotice() ERROR !!");

                        if (szPrtNotice == NULL)
                        {
                                inLogPrintf(AT, "szPrtNotice == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPrtNotice length = (%d)", (int)strlen(szPrtNotice));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szPrtNotice[0], &szPrtNotice[0], strlen(szPrtNotice));

        return (VS_SUCCESS);
}

/*
Function        :inGetElecCommerceFlag
Date&Time       :
Describe        :
*/
int inGetElecCommerceFlag(char* szElecCommerceFlag)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szElecCommerceFlag == NULL || strlen(srCFGTRec.szElecCommerceFlag) <= 0 || strlen(srCFGTRec.szElecCommerceFlag) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetElecCommerceFlag() ERROR !!");

                        if (szElecCommerceFlag == NULL)
                        {
                                inLogPrintf(AT, "szElecCommerceFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szElecCommerceFlag length = (%d)", (int)strlen(srCFGTRec.szElecCommerceFlag));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szElecCommerceFlag[0], &srCFGTRec.szElecCommerceFlag[0], strlen(srCFGTRec.szElecCommerceFlag));

        return (VS_SUCCESS);
}

/*
Function        :inSetElecCommerceFlag
Date&Time       :
Describe        :
*/
int inSetElecCommerceFlag(char* szElecCommerceFlag)
{
        memset(srCFGTRec.szElecCommerceFlag, 0x00, sizeof(srCFGTRec.szElecCommerceFlag));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szElecCommerceFlag == NULL || strlen(szElecCommerceFlag) <= 0 || strlen(szElecCommerceFlag) > 2)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetElecCommerceFlag() ERROR !!");

                        if (szElecCommerceFlag == NULL)
                        {
                                inLogPrintf(AT, "szElecCommerceFlag == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szElecCommerceFlag length = (%d)", (int)strlen(szElecCommerceFlag));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szElecCommerceFlag[0], &szElecCommerceFlag[0], strlen(szElecCommerceFlag));

        return (VS_SUCCESS);
}

/*
Function        :inGetDccFlowVersion
Date&Time       :
Describe        :
*/
int inGetDccFlowVersion(char* szDccFlowVersion)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szDccFlowVersion == NULL || strlen(srCFGTRec.szDccFlowVersion) <= 0 || strlen(srCFGTRec.szDccFlowVersion) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetDccFlowVersion() ERROR !!");

                        if (szDccFlowVersion == NULL)
                        {
                                inLogPrintf(AT, "szDccFlowVersion == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDccFlowVersion length = (%d)", (int)strlen(srCFGTRec.szDccFlowVersion));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szDccFlowVersion[0], &srCFGTRec.szDccFlowVersion[0], strlen(srCFGTRec.szDccFlowVersion));

        return (VS_SUCCESS);
}

/*
Function        :inSetDccFlowVersion
Date&Time       :
Describe        :
*/
int inSetDccFlowVersion(char* szDccFlowVersion)
{
        memset(srCFGTRec.szDccFlowVersion, 0x00, sizeof(srCFGTRec.szDccFlowVersion));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szDccFlowVersion == NULL || strlen(szDccFlowVersion) <= 0 || strlen(szDccFlowVersion) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetDccFlowVersion() ERROR !!");

                        if (szDccFlowVersion == NULL)
                        {
                                inLogPrintf(AT, "szDccFlowVersion == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDccFlowVersion length = (%d)", (int)strlen(szDccFlowVersion));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szDccFlowVersion[0], &szDccFlowVersion[0], strlen(szDccFlowVersion));

        return (VS_SUCCESS);
}

/*
Function        :inGetSupDccVisa
Date&Time       :
Describe        :
*/
int inGetSupDccVisa(char* szSupDccVisa)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szSupDccVisa == NULL || strlen(srCFGTRec.szSupDccVisa) <= 0 || strlen(srCFGTRec.szSupDccVisa) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetSupDccVisa() ERROR !!");

                        if (szSupDccVisa == NULL)
                        {
                                inLogPrintf(AT, "szSupDccVisa == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSupDccVisa length = (%d)", (int)strlen(srCFGTRec.szSupDccVisa));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSupDccVisa[0], &srCFGTRec.szSupDccVisa[0], strlen(srCFGTRec.szSupDccVisa));

        return (VS_SUCCESS);
}

/*
Function        :inSetSupDccVisa
Date&Time       :
Describe        :
*/
int inSetSupDccVisa(char* szSupDccVisa)
{
        memset(srCFGTRec.szSupDccVisa, 0x00, sizeof(srCFGTRec.szSupDccVisa));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szSupDccVisa == NULL || strlen(szSupDccVisa) <= 0 || strlen(szSupDccVisa) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetSupDccVisa() ERROR !!");

                        if (szSupDccVisa == NULL)
                        {
                                inLogPrintf(AT, "szSupDccVisa == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSupDccVisa length = (%d)", (int)strlen(szSupDccVisa));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szSupDccVisa[0], &szSupDccVisa[0], strlen(szSupDccVisa));

        return (VS_SUCCESS);
}

/*
Function        :inGetSupDccMasterCard
Date&Time       :
Describe        :
*/
int inGetSupDccMasterCard(char* szSupDccMasterCard)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szSupDccMasterCard == NULL || strlen(srCFGTRec.szSupDccMasterCard) <= 0 || strlen(srCFGTRec.szSupDccMasterCard) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetSupDccMasterCard() ERROR !!");

                        if (szSupDccMasterCard == NULL)
                        {
                                inLogPrintf(AT, "szSupDccMasterCard == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSupDccMasterCard length = (%d)", (int)strlen(srCFGTRec.szSupDccMasterCard));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSupDccMasterCard[0], &srCFGTRec.szSupDccMasterCard[0], strlen(srCFGTRec.szSupDccMasterCard));

        return (VS_SUCCESS);
}

/*
Function        :inSetSupDccMasterCard
Date&Time       :
Describe        :
*/
int inSetSupDccMasterCard(char* szSupDccMasterCard)
{
        memset(srCFGTRec.szSupDccMasterCard, 0x00, sizeof(srCFGTRec.szSupDccMasterCard));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szSupDccMasterCard == NULL || strlen(szSupDccMasterCard) <= 0 || strlen(szSupDccMasterCard) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetSupDccMasterCard() ERROR !!");

                        if (szSupDccMasterCard == NULL)
                        {
                                inLogPrintf(AT, "szSupDccMasterCard == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSupDccMasterCard length = (%d)", (int)strlen(szSupDccMasterCard));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szSupDccMasterCard[0], &szSupDccMasterCard[0], strlen(szSupDccMasterCard));

        return (VS_SUCCESS);
}

/*
Function        :inGetDHCPRetryTimes
Date&Time       :
Describe        :
*/
int inGetDHCPRetryTimes(char* szDHCPRetryTimes)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szDHCPRetryTimes == NULL || strlen(srCFGTRec.szDHCPRetryTimes) <= 0 || strlen(srCFGTRec.szDHCPRetryTimes) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetDHCPRetryTimes() ERROR !!");

                        if (szDHCPRetryTimes == NULL)
                        {
                                inLogPrintf(AT, "szDHCPRetryTimes == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDHCPRetryTimes length = (%d)", (int)strlen(srCFGTRec.szDHCPRetryTimes));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szDHCPRetryTimes[0], &srCFGTRec.szDHCPRetryTimes[0], strlen(srCFGTRec.szDHCPRetryTimes));

        return (VS_SUCCESS);
}

/*
Function        :inSetDHCPRetryTimes
Date&Time       :
Describe        :
*/
int inSetDHCPRetryTimes(char* szDHCPRetryTimes)
{
        memset(srCFGTRec.szDHCPRetryTimes, 0x00, sizeof(srCFGTRec.szDHCPRetryTimes));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szDHCPRetryTimes == NULL || strlen(szDHCPRetryTimes) <= 0 || strlen(szDHCPRetryTimes) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetDHCPRetryTimes() ERROR !!");

                        if (szDHCPRetryTimes == NULL)
                        {
                                inLogPrintf(AT, "szDHCPRetryTimes == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDHCPRetryTimes length = (%d)", (int)strlen(szDHCPRetryTimes));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szDHCPRetryTimes[0], &szDHCPRetryTimes[0], strlen(szDHCPRetryTimes));

        return (VS_SUCCESS);
}

/*
Function        :inGetBarCodeReaderEnable
Date&Time       :
Describe        :
*/
int inGetBarCodeReaderEnable(char* szBarCodeReaderEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szBarCodeReaderEnable == NULL || strlen(srCFGTRec.szBarCodeReaderEnable) <= 0 || strlen(srCFGTRec.szBarCodeReaderEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetBarCodeReaderEnable() ERROR !!");

                        if (szBarCodeReaderEnable == NULL)
                        {
                                inLogPrintf(AT, "szBarCodeReaderEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBarCodeReaderEnable length = (%d)", (int)strlen(srCFGTRec.szBarCodeReaderEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szBarCodeReaderEnable[0], &srCFGTRec.szBarCodeReaderEnable[0], strlen(srCFGTRec.szBarCodeReaderEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetBarCodeReaderEnable
Date&Time       :
Describe        :
*/
int inSetBarCodeReaderEnable(char* szBarCodeReaderEnable)
{
        memset(srCFGTRec.szBarCodeReaderEnable, 0x00, sizeof(srCFGTRec.szBarCodeReaderEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szBarCodeReaderEnable == NULL || strlen(szBarCodeReaderEnable) <= 0 || strlen(szBarCodeReaderEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetBarCodeReaderEnable() ERROR !!");

                        if (szBarCodeReaderEnable == NULL)
                        {
                                inLogPrintf(AT, "szBarCodeReaderEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBarCodeReaderEnable length = (%d)", (int)strlen(szBarCodeReaderEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szBarCodeReaderEnable[0], &szBarCodeReaderEnable[0], strlen(szBarCodeReaderEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetEMVPINBypassEnable
Date&Time       :
Describe        :
*/
int inGetEMVPINBypassEnable(char* szEMVPINBypassEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szEMVPINBypassEnable == NULL || strlen(srCFGTRec.szEMVPINBypassEnable) <= 0 || strlen(srCFGTRec.szEMVPINBypassEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetEMVPINBypassEnable() ERROR !!");

                        if (szEMVPINBypassEnable == NULL)
                        {
                                inLogPrintf(AT, "szEMVPINBypassEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEMVPINBypassEnable length = (%d)", (int)strlen(srCFGTRec.szEMVPINBypassEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szEMVPINBypassEnable[0], &srCFGTRec.szEMVPINBypassEnable[0], strlen(srCFGTRec.szEMVPINBypassEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetEMVPINBypassEnable
Date&Time       :
Describe        :
*/
int inSetEMVPINBypassEnable(char* szEMVPINBypassEnable)
{
        memset(srCFGTRec.szEMVPINBypassEnable, 0x00, sizeof(srCFGTRec.szEMVPINBypassEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szEMVPINBypassEnable == NULL || strlen(szEMVPINBypassEnable) <= 0 || strlen(szEMVPINBypassEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetEMVPINBypassEnable() ERROR !!");

                        if (szEMVPINBypassEnable == NULL)
                        {
                                inLogPrintf(AT, "szEMVPINBypassEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szEMVPINBypassEnable length = (%d)", (int)strlen(szEMVPINBypassEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szEMVPINBypassEnable[0], &szEMVPINBypassEnable[0], strlen(szEMVPINBypassEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetCUPOnlinePINEntryTimeout
Date&Time       :
Describe        :
*/
int inGetCUPOnlinePINEntryTimeout(char* szCUPOnlinePINEntryTimeout)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szCUPOnlinePINEntryTimeout == NULL || strlen(srCFGTRec.szCUPOnlinePINEntryTimeout) <= 0 || strlen(srCFGTRec.szCUPOnlinePINEntryTimeout) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetCUPOnlinePINEntryTimeout() ERROR !!");

                        if (szCUPOnlinePINEntryTimeout == NULL)
                        {
                                inLogPrintf(AT, "szCUPOnlinePINEntryTimeout == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCUPOnlinePINEntryTimeout length = (%d)", (int)strlen(srCFGTRec.szCUPOnlinePINEntryTimeout));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCUPOnlinePINEntryTimeout[0], &srCFGTRec.szCUPOnlinePINEntryTimeout[0], strlen(srCFGTRec.szCUPOnlinePINEntryTimeout));

        return (VS_SUCCESS);
}

/*
Function        :inSetCUPOnlinePINEntryTimeout
Date&Time       :
Describe        :
*/
int inSetCUPOnlinePINEntryTimeout(char* szCUPOnlinePINEntryTimeout)
{
        memset(srCFGTRec.szCUPOnlinePINEntryTimeout, 0x00, sizeof(srCFGTRec.szCUPOnlinePINEntryTimeout));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCUPOnlinePINEntryTimeout == NULL || strlen(szCUPOnlinePINEntryTimeout) <= 0 || strlen(szCUPOnlinePINEntryTimeout) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCUPOnlinePINEntryTimeout() ERROR !!");

                        if (szCUPOnlinePINEntryTimeout == NULL)
                        {
                                inLogPrintf(AT, "szCUPOnlinePINEntryTimeout == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCUPOnlinePINEntryTimeout length = (%d)", (int)strlen(szCUPOnlinePINEntryTimeout));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szCUPOnlinePINEntryTimeout[0], &szCUPOnlinePINEntryTimeout[0], strlen(szCUPOnlinePINEntryTimeout));

        return (VS_SUCCESS);
}

/*
Function        :inGetSignPadMode
Date&Time       :
Describe        :
*/
int inGetSignPadMode(char* szSignPadMode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szSignPadMode == NULL || strlen(srCFGTRec.szSignPadMode) <= 0 || strlen(srCFGTRec.szSignPadMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetSignPadMode() ERROR !!");

                        if (szSignPadMode == NULL)
                        {
                                inLogPrintf(AT, "szSignPadMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSignPadMode length = (%d)", (int)strlen(srCFGTRec.szSignPadMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSignPadMode[0], &srCFGTRec.szSignPadMode[0], strlen(srCFGTRec.szSignPadMode));

        return (VS_SUCCESS);
}

/*
Function        :inSetSignPadMode
Date&Time       :
Describe        :
*/
int inSetSignPadMode(char* szSignPadMode)
{
        memset(srCFGTRec.szSignPadMode, 0x00, sizeof(srCFGTRec.szSignPadMode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szSignPadMode == NULL || strlen(szSignPadMode) <= 0 || strlen(szSignPadMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetSignPadMode() ERROR !!");

                        if (szSignPadMode == NULL)
                        {
                                inLogPrintf(AT, "szSignPadMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSignPadMode length = (%d)", (int)strlen(szSignPadMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szSignPadMode[0], &szSignPadMode[0], strlen(szSignPadMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetESCPrintMerchantCopy
Date&Time       :
Describe        :
*/
int inGetESCPrintMerchantCopy(char* szESCPrintMerchantCopy)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szESCPrintMerchantCopy == NULL || strlen(srCFGTRec.szESCPrintMerchantCopy) <= 0 || strlen(srCFGTRec.szESCPrintMerchantCopy) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetESCPrintMerchantCopy() ERROR !!");

                        if (szESCPrintMerchantCopy == NULL)
                        {
                                inLogPrintf(AT, "szESCPrintMerchantCopy == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szESCPrintMerchantCopy length = (%d)", (int)strlen(srCFGTRec.szESCPrintMerchantCopy));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szESCPrintMerchantCopy[0], &srCFGTRec.szESCPrintMerchantCopy[0], strlen(srCFGTRec.szESCPrintMerchantCopy));

        return (VS_SUCCESS);
}

/*
Function        :inSetESCPrintMerchantCopy
Date&Time       :
Describe        :
*/
int inSetESCPrintMerchantCopy(char* szESCPrintMerchantCopy)
{
        memset(srCFGTRec.szESCPrintMerchantCopy, 0x00, sizeof(srCFGTRec.szESCPrintMerchantCopy));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szESCPrintMerchantCopy == NULL || strlen(szESCPrintMerchantCopy) <= 0 || strlen(szESCPrintMerchantCopy) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetESCPrintMerchantCopy() ERROR !!");

                        if (szESCPrintMerchantCopy == NULL)
                        {
                                inLogPrintf(AT, "szESCPrintMerchantCopy == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szESCPrintMerchantCopy length = (%d)", (int)strlen(szESCPrintMerchantCopy));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szESCPrintMerchantCopy[0], &szESCPrintMerchantCopy[0], strlen(szESCPrintMerchantCopy));

        return (VS_SUCCESS);
}

/*
Function        :inGetESCPrintMerchantCopyStartDate
Date&Time       :
Describe        :
*/
int inGetESCPrintMerchantCopyStartDate(char* szESCPrintMerchantCopyStartDate)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szESCPrintMerchantCopyStartDate == NULL || strlen(srCFGTRec.szESCPrintMerchantCopyStartDate) <= 0 || strlen(srCFGTRec.szESCPrintMerchantCopyStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetESCPrintMerchantCopyStartDate() ERROR !!");

                        if (szESCPrintMerchantCopyStartDate == NULL)
                        {
                                inLogPrintf(AT, "szESCPrintMerchantCopyStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szESCPrintMerchantCopyStartDate length = (%d)", (int)strlen(srCFGTRec.szESCPrintMerchantCopyStartDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szESCPrintMerchantCopyStartDate[0], &srCFGTRec.szESCPrintMerchantCopyStartDate[0], strlen(srCFGTRec.szESCPrintMerchantCopyStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetESCPrintMerchantCopyStartDate
Date&Time       :
Describe        :
*/
int inSetESCPrintMerchantCopyStartDate(char* szESCPrintMerchantCopyStartDate)
{
        memset(srCFGTRec.szESCPrintMerchantCopyStartDate, 0x00, sizeof(srCFGTRec.szESCPrintMerchantCopyStartDate));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szESCPrintMerchantCopyStartDate == NULL || strlen(szESCPrintMerchantCopyStartDate) <= 0 || strlen(szESCPrintMerchantCopyStartDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetESCPrintMerchantCopyStartDate() ERROR !!");

                        if (szESCPrintMerchantCopyStartDate == NULL)
                        {
                                inLogPrintf(AT, "szESCPrintMerchantCopyStartDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szESCPrintMerchantCopyStartDate length = (%d)", (int)strlen(szESCPrintMerchantCopyStartDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szESCPrintMerchantCopyStartDate[0], &szESCPrintMerchantCopyStartDate[0], strlen(szESCPrintMerchantCopyStartDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetESCPrintMerchantCopyStartDate
Date&Time       :
Describe        :
*/
int inGetESCPrintMerchantCopyEndDate(char* szESCPrintMerchantCopyEndDate)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szESCPrintMerchantCopyEndDate == NULL || strlen(srCFGTRec.szESCPrintMerchantCopyEndDate) <= 0 || strlen(srCFGTRec.szESCPrintMerchantCopyEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetESCPrintMerchantCopyEndDate() ERROR !!");

                        if (szESCPrintMerchantCopyEndDate == NULL)
                        {
                                inLogPrintf(AT, "szESCPrintMerchantCopyEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szESCPrintMerchantCopyEndDate length = (%d)", (int)strlen(srCFGTRec.szESCPrintMerchantCopyEndDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szESCPrintMerchantCopyEndDate[0], &srCFGTRec.szESCPrintMerchantCopyEndDate[0], strlen(srCFGTRec.szESCPrintMerchantCopyEndDate));

        return (VS_SUCCESS);
}

/*
Function        :inSetESCPrintMerchantCopyStartDate
Date&Time       :
Describe        :
*/
int inSetESCPrintMerchantCopyEndDate(char* szESCPrintMerchantCopyEndDate)
{
        memset(srCFGTRec.szESCPrintMerchantCopyEndDate, 0x00, sizeof(srCFGTRec.szESCPrintMerchantCopyEndDate));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szESCPrintMerchantCopyEndDate == NULL || strlen(szESCPrintMerchantCopyEndDate) <= 0 || strlen(szESCPrintMerchantCopyEndDate) > 8)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetESCPrintMerchantCopyEndDate() ERROR !!");

                        if (szESCPrintMerchantCopyEndDate == NULL)
                        {
                                inLogPrintf(AT, "szESCPrintMerchantCopyEndDate == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szESCPrintMerchantCopyEndDate length = (%d)", (int)strlen(szESCPrintMerchantCopyEndDate));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szESCPrintMerchantCopyEndDate[0], &szESCPrintMerchantCopyEndDate[0], strlen(szESCPrintMerchantCopyEndDate));

        return (VS_SUCCESS);
}

/*
Function        :inGetESCReciptUploadUpLimit
Date&Time       :
Describe        :
*/
int inGetESCReciptUploadUpLimit(char* szESCReciptUploadUpLimit)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szESCReciptUploadUpLimit == NULL || strlen(srCFGTRec.szESCReciptUploadUpLimit) <= 0 || strlen(srCFGTRec.szESCReciptUploadUpLimit) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetESCReciptUploadUpLimit() ERROR !!");

                        if (szESCReciptUploadUpLimit == NULL)
                        {
                                inLogPrintf(AT, "szESCReciptUploadUpLimit == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szESCReciptUploadUpLimit length = (%d)", (int)strlen(srCFGTRec.szESCReciptUploadUpLimit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szESCReciptUploadUpLimit[0], &srCFGTRec.szESCReciptUploadUpLimit[0], strlen(srCFGTRec.szESCReciptUploadUpLimit));

        return (VS_SUCCESS);
}

/*
Function        :inSetESCReciptUploadUpLimit
Date&Time       :
Describe        :
*/
int inSetESCReciptUploadUpLimit(char* szESCReciptUploadUpLimit)
{
        memset(srCFGTRec.szESCReciptUploadUpLimit, 0x00, sizeof(srCFGTRec.szESCReciptUploadUpLimit));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szESCReciptUploadUpLimit == NULL || strlen(szESCReciptUploadUpLimit) <= 0 || strlen(szESCReciptUploadUpLimit) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetESCReciptUploadUpLimit() ERROR !!");

                        if (szESCReciptUploadUpLimit == NULL)
                        {
                                inLogPrintf(AT, "szESCReciptUploadUpLimite == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szESCReciptUploadUpLimit length = (%d)", (int)strlen(szESCReciptUploadUpLimit));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szESCReciptUploadUpLimit[0], &szESCReciptUploadUpLimit[0], strlen(szESCReciptUploadUpLimit));

        return (VS_SUCCESS);
}

/*
Function        :inGetContactlessReaderMode
Date&Time       :
Describe        :
*/
int inGetContactlessReaderMode(char* szContactlessReaderMode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szContactlessReaderMode == NULL || strlen(srCFGTRec.szContactlessReaderMode) <= 0 || strlen(srCFGTRec.szContactlessReaderMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetContactlessReaderMode() ERROR !!");

                        if (szContactlessReaderMode == NULL)
                        {
                                inLogPrintf(AT, "szContactlessReaderMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szContactlessReaderMode length = (%d)", (int)strlen(srCFGTRec.szContactlessReaderMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szContactlessReaderMode[0], &srCFGTRec.szContactlessReaderMode[0], strlen(srCFGTRec.szContactlessReaderMode));

        return (VS_SUCCESS);
}

/*
Function        :inSetContactlessReaderMode
Date&Time       :
Describe        :
*/
int inSetContactlessReaderMode(char* szContactlessReaderMode)
{
        memset(srCFGTRec.szContactlessReaderMode, 0x00, sizeof(srCFGTRec.szContactlessReaderMode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szContactlessReaderMode == NULL || strlen(szContactlessReaderMode) <= 0 || strlen(szContactlessReaderMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetContactlessReaderMode() ERROR !!");

                        if (szContactlessReaderMode == NULL)
                        {
                                inLogPrintf(AT, "szContactlessReaderMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szContactlessReaderMode length = (%d)", (int)strlen(szContactlessReaderMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szContactlessReaderMode[0], &szContactlessReaderMode[0], strlen(szContactlessReaderMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetTMSDownloadMode
Date&Time       :
Describe        :
*/
int inGetTMSDownloadMode(char* szTMSDownloadMode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szTMSDownloadMode == NULL || strlen(srCFGTRec.szTMSDownloadMode) <= 0 || strlen(srCFGTRec.szTMSDownloadMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetTMSDownloadMode() ERROR !!");

                        if (szTMSDownloadMode == NULL)
                        {
                                inLogPrintf(AT, "szTMSDownloadMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSDownloadMode length = (%d)", (int)strlen(srCFGTRec.szTMSDownloadMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szTMSDownloadMode[0], &srCFGTRec.szTMSDownloadMode[0], strlen(srCFGTRec.szTMSDownloadMode));

        return (VS_SUCCESS);
}

/*
Function        :inSetTMSDownloadMode
Date&Time       :
Describe        :
*/
int inSetTMSDownloadMode(char* szTMSDownloadMode)
{
        memset(srCFGTRec.szTMSDownloadMode, 0x00, sizeof(srCFGTRec.szTMSDownloadMode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szTMSDownloadMode == NULL || strlen(szTMSDownloadMode) <= 0 || strlen(szTMSDownloadMode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetTMSDownloadMode() ERROR !!");

                        if (szTMSDownloadMode == NULL)
                        {
                                inLogPrintf(AT, "szTMSDownloadMode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szTMSDownloadMode length = (%d)", (int)strlen(szTMSDownloadMode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szTMSDownloadMode[0], &szTMSDownloadMode[0], strlen(szTMSDownloadMode));

        return (VS_SUCCESS);
}

/*
Function        :inGetAMEXContactlessEnable
Date&Time       :
Describe        :
*/
int inGetAMEXContactlessEnable(char* szAMEXContactlessEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szAMEXContactlessEnable == NULL || strlen(srCFGTRec.szAMEXContactlessEnable) <= 0 || strlen(srCFGTRec.szAMEXContactlessEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetAMEXContactlessEnable() ERROR !!");

                        if (szAMEXContactlessEnable == NULL)
                        {
                                inLogPrintf(AT, "szAMEXContactlessEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szAMEXContactlessEnable length = (%d)", (int)strlen(srCFGTRec.szAMEXContactlessEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szAMEXContactlessEnable[0], &srCFGTRec.szAMEXContactlessEnable[0], strlen(srCFGTRec.szAMEXContactlessEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetAMEXContactlessEnable
Date&Time       :
Describe        :
*/
int inSetAMEXContactlessEnable(char* szAMEXContactlessEnable)
{
        memset(srCFGTRec.szAMEXContactlessEnable, 0x00, sizeof(srCFGTRec.szAMEXContactlessEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szAMEXContactlessEnable == NULL || strlen(szAMEXContactlessEnable) <= 0 || strlen(szAMEXContactlessEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetAMEXContactlessEnable() ERROR !!");

                        if (szAMEXContactlessEnable == NULL)
                        {
                                inLogPrintf(AT, "szAMEXContactlessEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szAMEXContactlessEnable length = (%d)", (int)strlen(szAMEXContactlessEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szAMEXContactlessEnable[0], &szAMEXContactlessEnable[0], strlen(szAMEXContactlessEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetCUPContactlessEnable
Date&Time       :
Describe        :
*/
int inGetCUPContactlessEnable(char* szCUPContactlessEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szCUPContactlessEnable == NULL || strlen(srCFGTRec.szCUPContactlessEnable) <= 0 || strlen(srCFGTRec.szCUPContactlessEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetCUPContactlessEnable() ERROR !!");

                        if (szCUPContactlessEnable == NULL)
                        {
                                inLogPrintf(AT, "szCUPContactlessEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCUPContactlessEnable length = (%d)", (int)strlen(srCFGTRec.szCUPContactlessEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCUPContactlessEnable[0], &srCFGTRec.szCUPContactlessEnable[0], strlen(srCFGTRec.szCUPContactlessEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetCUPContactlessEnable
Date&Time       :
Describe        :
*/
int inSetCUPContactlessEnable(char* szCUPContactlessEnable)
{
        memset(srCFGTRec.szCUPContactlessEnable, 0x00, sizeof(srCFGTRec.szCUPContactlessEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCUPContactlessEnable == NULL || strlen(szCUPContactlessEnable) <= 0 || strlen(szCUPContactlessEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCUPContactlessEnable() ERROR !!");

                        if (szCUPContactlessEnable == NULL)
                        {
                                inLogPrintf(AT, "szCUPContactlessEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCUPContactlessEnable length = (%d)", (int)strlen(szCUPContactlessEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szCUPContactlessEnable[0], &szCUPContactlessEnable[0], strlen(szCUPContactlessEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetSmartPayContactlessEnable
Date&Time       :
Describe        :
*/
int inGetSmartPayContactlessEnable(char* szSmartPayContactlessEnable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szSmartPayContactlessEnable == NULL || strlen(srCFGTRec.szSmartPayContactlessEnable) <= 0 || strlen(srCFGTRec.szSmartPayContactlessEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetSmartPayContactlessEnable() ERROR !!");

                        if (szSmartPayContactlessEnable == NULL)
                        {
                                inLogPrintf(AT, "szSmartPayContactlessEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSmartPayContactlessEnable length = (%d)", (int)strlen(srCFGTRec.szSmartPayContactlessEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szSmartPayContactlessEnable[0], &srCFGTRec.szSmartPayContactlessEnable[0], strlen(srCFGTRec.szSmartPayContactlessEnable));

        return (VS_SUCCESS);
}

/*
Function        :inSetSmartPayContactlessEnable
Date&Time       :
Describe        :
*/
int inSetSmartPayContactlessEnable(char* szSmartPayContactlessEnable)
{
        memset(srCFGTRec.szSmartPayContactlessEnable, 0x00, sizeof(srCFGTRec.szSmartPayContactlessEnable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szSmartPayContactlessEnable == NULL || strlen(szSmartPayContactlessEnable) <= 0 || strlen(szSmartPayContactlessEnable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetSmartPayContactlessEnable() ERROR !!");

                        if (szSmartPayContactlessEnable == NULL)
                        {
                                inLogPrintf(AT, "szSmartPayContactlessEnable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szSmartPayContactlessEnable length = (%d)", (int)strlen(szSmartPayContactlessEnable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szSmartPayContactlessEnable[0], &szSmartPayContactlessEnable[0], strlen(szSmartPayContactlessEnable));

        return (VS_SUCCESS);
}

/*
Function        :inGetPayItemEnable
Date&Time       :2017/4/12 下午 1:31
Describe        :
*/
int inGetPayItemEnable(char* szPay_Item_Enable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szPay_Item_Enable == NULL || strlen(srCFGTRec.szPay_Item_Enable) <= 0 || strlen(srCFGTRec.szPay_Item_Enable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetPay_Item_Enable() ERROR !!");

                        if (szPay_Item_Enable == NULL)
                        {
                                inLogPrintf(AT, "szPay_Item_Enable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPay_Item_Enable length = (%d)", (int)strlen(srCFGTRec.szPay_Item_Enable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szPay_Item_Enable[0], &srCFGTRec.szPay_Item_Enable[0], strlen(srCFGTRec.szPay_Item_Enable));

        return (VS_SUCCESS);
}

/*
Function        :inSetPayItemEnable
Date&Time       :2017/4/12 下午 1:32
Describe        :
*/
int inSetPayItemEnable(char* szPay_Item_Enable)
{
        memset(srCFGTRec.szPay_Item_Enable, 0x00, sizeof(srCFGTRec.szPay_Item_Enable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szPay_Item_Enable == NULL || strlen(szPay_Item_Enable) <= 0 || strlen(szPay_Item_Enable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetPay_Item_Enable() ERROR !!");

                        if (szPay_Item_Enable == NULL)
                        {
                                inLogPrintf(AT, "szPay_Item_Enable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szPay_Item_Enable length = (%d)", (int)strlen(szPay_Item_Enable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szPay_Item_Enable[0], &szPay_Item_Enable[0], strlen(szPay_Item_Enable));

        return (VS_SUCCESS);
}

/*
Function        :inGetStore_Stub_CardNo_Truncate_Enable
Date&Time       :2017/4/12 下午 1:32
Describe        :
*/
int inGetStore_Stub_CardNo_Truncate_Enable(char* szStore_Stub_CardNo_Truncate_Enable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szStore_Stub_CardNo_Truncate_Enable == NULL || strlen(srCFGTRec.szStore_Stub_CardNo_Truncate_Enable) <= 0 || strlen(srCFGTRec.szStore_Stub_CardNo_Truncate_Enable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetStore_Stub_CardNo_Truncate_Enable() ERROR !!");

                        if (szStore_Stub_CardNo_Truncate_Enable == NULL)
                        {
                                inLogPrintf(AT, "szStore_Stub_CardNo_Truncate_Enable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szStore_Stub_CardNo_Truncate_Enable length = (%d)", (int)strlen(srCFGTRec.szStore_Stub_CardNo_Truncate_Enable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szStore_Stub_CardNo_Truncate_Enable[0], &srCFGTRec.szStore_Stub_CardNo_Truncate_Enable[0], strlen(srCFGTRec.szStore_Stub_CardNo_Truncate_Enable));

        return (VS_SUCCESS);
}

/*
Function        :inSetStore_Stub_CardNo_Truncate_Enable
Date&Time       :2017/4/12 下午 1:32
Describe        :
*/
int inSetStore_Stub_CardNo_Truncate_Enable(char* szStore_Stub_CardNo_Truncate_Enable)
{
        memset(srCFGTRec.szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(srCFGTRec.szStore_Stub_CardNo_Truncate_Enable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szStore_Stub_CardNo_Truncate_Enable == NULL || strlen(szStore_Stub_CardNo_Truncate_Enable) <= 0 || strlen(szStore_Stub_CardNo_Truncate_Enable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetStore_Stub_CardNo_Truncate_Enable() ERROR !!");

                        if (szStore_Stub_CardNo_Truncate_Enable == NULL)
                        {
                                inLogPrintf(AT, "szStore_Stub_CardNo_Truncate_Enable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szStore_Stub_CardNo_Truncate_Enable length = (%d)", (int)strlen(szStore_Stub_CardNo_Truncate_Enable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szStore_Stub_CardNo_Truncate_Enable[0], &szStore_Stub_CardNo_Truncate_Enable[0], strlen(szStore_Stub_CardNo_Truncate_Enable));

        return (VS_SUCCESS);
}

/*
Function        :inGetIntegrate_Device
Date&Time       :2017/4/12 下午 1:34
Describe        :
*/
int inGetIntegrate_Device(char* szIntegrate_Device)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szIntegrate_Device == NULL || strlen(srCFGTRec.szIntegrate_Device) <= 0 || strlen(srCFGTRec.szIntegrate_Device) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetIntegrate_Device() ERROR !!");

                        if (szIntegrate_Device == NULL)
                        {
                                inLogPrintf(AT, "szIntegrate_Device == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIntegrate_Device length = (%d)", (int)strlen(srCFGTRec.szIntegrate_Device));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szIntegrate_Device[0], &srCFGTRec.szIntegrate_Device[0], strlen(srCFGTRec.szIntegrate_Device));

        return (VS_SUCCESS);
}

/*
Function        :inSetIntegrate_Device
Date&Time       :2017/4/12 下午 1:34
Describe        :
*/
int inSetIntegrate_Device(char* szIntegrate_Device)
{
        memset(srCFGTRec.szIntegrate_Device, 0x00, sizeof(srCFGTRec.szIntegrate_Device));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szIntegrate_Device == NULL || strlen(szIntegrate_Device) <= 0 || strlen(szIntegrate_Device) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetIntegrate_Device() ERROR !!");

                        if (szIntegrate_Device == NULL)
                        {
                                inLogPrintf(AT, "szIntegrate_Device == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIntegrate_Device length = (%d)", (int)strlen(szIntegrate_Device));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szIntegrate_Device[0], &szIntegrate_Device[0], strlen(szIntegrate_Device));

        return (VS_SUCCESS);
}

/*
Function        :inGetFES_ID
Date&Time       :2017/4/12 下午 1:35
Describe        :
*/
int inGetFES_ID(char* szFES_ID)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szFES_ID == NULL || strlen(srCFGTRec.szFES_ID) <= 0 || strlen(srCFGTRec.szFES_ID) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetFES_ID() ERROR !!");

                        if (szFES_ID == NULL)
                        {
                                inLogPrintf(AT, "szFES_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFES_ID length = (%d)", (int)strlen(srCFGTRec.szFES_ID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFES_ID[0], &srCFGTRec.szFES_ID[0], strlen(srCFGTRec.szFES_ID));

        return (VS_SUCCESS);
}

/*
Function        :inSetFES_ID
Date&Time       :2017/4/12 下午 1:35
Describe        :
*/
int inSetFES_ID(char* szFES_ID)
{
        memset(srCFGTRec.szFES_ID, 0x00, sizeof(srCFGTRec.szFES_ID));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFES_ID == NULL || strlen(szFES_ID) <= 0 || strlen(szFES_ID) > 3)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetFES_ID() ERROR !!");

                        if (szFES_ID == NULL)
                        {
                                inLogPrintf(AT, "szFES_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFES_ID length = (%d)", (int)strlen(szFES_ID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szFES_ID[0], &szFES_ID[0], strlen(szFES_ID));

        return (VS_SUCCESS);
}

/*
Function        :inGetIntegrate_Device_AP_ID
Date&Time       :2017/4/12 下午 1:35
Describe        :
*/
int inGetIntegrate_Device_AP_ID(char* szIntegrate_Device_AP_ID)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szIntegrate_Device_AP_ID == NULL || strlen(srCFGTRec.szIntegrate_Device_AP_ID) <= 0 || strlen(srCFGTRec.szIntegrate_Device_AP_ID) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetIntegrate_Device_AP_ID() ERROR !!");

                        if (szIntegrate_Device_AP_ID == NULL)
                        {
                                inLogPrintf(AT, "szIntegrate_Device_AP_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIntegrate_Device_AP_ID length = (%d)", (int)strlen(srCFGTRec.szIntegrate_Device_AP_ID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szIntegrate_Device_AP_ID[0], &srCFGTRec.szIntegrate_Device_AP_ID[0], strlen(srCFGTRec.szIntegrate_Device_AP_ID));

        return (VS_SUCCESS);
}

/*
Function        :inSetIntegrate_Device_AP_ID
Date&Time       :2017/4/12 下午 1:35
Describe        :
*/
int inSetIntegrate_Device_AP_ID(char* szIntegrate_Device_AP_ID)
{
        memset(srCFGTRec.szIntegrate_Device_AP_ID, 0x00, sizeof(srCFGTRec.szIntegrate_Device_AP_ID));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szIntegrate_Device_AP_ID == NULL || strlen(szIntegrate_Device_AP_ID) <= 0 || strlen(szIntegrate_Device_AP_ID) > 15)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetIntegrate_Device_AP_ID() ERROR !!");

                        if (szIntegrate_Device_AP_ID == NULL)
                        {
                                inLogPrintf(AT, "szIntegrate_Device_AP_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szIntegrate_Device_AP_ID length = (%d)", (int)strlen(szIntegrate_Device_AP_ID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szIntegrate_Device_AP_ID[0], &szIntegrate_Device_AP_ID[0], strlen(szIntegrate_Device_AP_ID));

        return (VS_SUCCESS);
}

/*
Function        :inGetShort_Receipt_Mode
Date&Time       :2017/4/12 下午 1:36
Describe        :
*/
int inGetShort_Receipt_Mode(char* szShort_Receipt_Mode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szShort_Receipt_Mode == NULL || strlen(srCFGTRec.szShort_Receipt_Mode) <= 0 || strlen(srCFGTRec.szShort_Receipt_Mode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetShort_Receipt_Mode() ERROR !!");

                        if (szShort_Receipt_Mode == NULL)
                        {
                                inLogPrintf(AT, "szShort_Receipt_Mode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szShort_Receipt_Mode length = (%d)", (int)strlen(srCFGTRec.szShort_Receipt_Mode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szShort_Receipt_Mode[0], &srCFGTRec.szShort_Receipt_Mode[0], strlen(srCFGTRec.szShort_Receipt_Mode));

        return (VS_SUCCESS);
}

/*
Function        :inSetShort_Receipt_Mode
Date&Time       :2017/4/12 下午 1:36
Describe        :
*/
int inSetShort_Receipt_Mode(char* szShort_Receipt_Mode)
{
        memset(srCFGTRec.szShort_Receipt_Mode, 0x00, sizeof(srCFGTRec.szShort_Receipt_Mode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szShort_Receipt_Mode == NULL || strlen(szShort_Receipt_Mode) <= 0 || strlen(szShort_Receipt_Mode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetShort_Receipt_Mode() ERROR !!");

                        if (szShort_Receipt_Mode == NULL)
                        {
                                inLogPrintf(AT, "szShort_Receipt_Mode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szShort_Receipt_Mode length = (%d)", (int)strlen(szShort_Receipt_Mode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szShort_Receipt_Mode[0], &szShort_Receipt_Mode[0], strlen(szShort_Receipt_Mode));

        return (VS_SUCCESS);
}

/*
Function        :inGetI_FES_Mode
Date&Time       :2017/4/12 下午 1:36
Describe        :
*/
int inGetI_FES_Mode(char* szI_FES_Mode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szI_FES_Mode == NULL || strlen(srCFGTRec.szI_FES_Mode) <= 0 || strlen(srCFGTRec.szI_FES_Mode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetI_FES_Mode() ERROR !!");

                        if (szI_FES_Mode == NULL)
                        {
                                inLogPrintf(AT, "szI_FES_Mode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szI_FES_Mode length = (%d)", (int)strlen(srCFGTRec.szI_FES_Mode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szI_FES_Mode[0], &srCFGTRec.szI_FES_Mode[0], strlen(srCFGTRec.szI_FES_Mode));

        return (VS_SUCCESS);
}

/*
Function        :inSetI_FES_Mode
Date&Time       :2017/4/12 下午 1:36
Describe        :
*/
int inSetI_FES_Mode(char* szI_FES_Mode)
{
        memset(srCFGTRec.szI_FES_Mode, 0x00, sizeof(srCFGTRec.szI_FES_Mode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szI_FES_Mode == NULL || strlen(szI_FES_Mode) <= 0 || strlen(szI_FES_Mode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetI_FES_Mode() ERROR !!");

                        if (szI_FES_Mode == NULL)
                        {
                                inLogPrintf(AT, "szI_FES_Mode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szI_FES_Mode length = (%d)", (int)strlen(szI_FES_Mode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szI_FES_Mode[0], &szI_FES_Mode[0], strlen(szI_FES_Mode));

        return (VS_SUCCESS);
}

/*
Function        :inGetDHCP_Mode
Date&Time       :2017/4/12 下午 1:36
Describe        :
*/
int inGetDHCP_Mode(char* szDHCP_Mode)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szDHCP_Mode == NULL || strlen(srCFGTRec.szDHCP_Mode) <= 0 || strlen(srCFGTRec.szDHCP_Mode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetDHCP_Mode() ERROR !!");

                        if (szDHCP_Mode == NULL)
                        {
                                inLogPrintf(AT, "szDHCP_Mode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDHCP_Mode length = (%d)", (int)strlen(srCFGTRec.szDHCP_Mode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szDHCP_Mode[0], &srCFGTRec.szDHCP_Mode[0], strlen(srCFGTRec.szDHCP_Mode));

        return (VS_SUCCESS);
}

/*
Function        :inSetDHCP_Mode
Date&Time       :2017/4/12 下午 1:36
Describe        :
*/
int inSetDHCP_Mode(char* szDHCP_Mode)
{
        memset(srCFGTRec.szDHCP_Mode, 0x00, sizeof(srCFGTRec.szDHCP_Mode));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szDHCP_Mode == NULL || strlen(szDHCP_Mode) <= 0 || strlen(szDHCP_Mode) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetDHCP_Mode() ERROR !!");

                        if (szDHCP_Mode == NULL)
                        {
                                inLogPrintf(AT, "szDHCP_Mode == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDHCP_Mode length = (%d)", (int)strlen(szDHCP_Mode));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szDHCP_Mode[0], &szDHCP_Mode[0], strlen(szDHCP_Mode));

        return (VS_SUCCESS);
}

/*
Function        :inGetESVC_Priority
Date&Time       :2018/1/17 下午 6:28
Describe        :
*/
int inGetESVC_Priority(char* szESVC_Priority)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szESVC_Priority == NULL || strlen(srCFGTRec.szESVC_Priority) <= 0 || strlen(srCFGTRec.szESVC_Priority) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetESVC_Priority() ERROR !!");

                        if (szESVC_Priority == NULL)
                        {
                                inLogPrintf(AT, "szESVC_Priority == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szESVC_Priority length = (%d)", (int)strlen(srCFGTRec.szESVC_Priority));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szESVC_Priority[0], &srCFGTRec.szESVC_Priority[0], strlen(srCFGTRec.szESVC_Priority));

        return (VS_SUCCESS);
}

/*
Function        :inSetESVC_Priority
Date&Time       :2018/1/17 下午 6:28
Describe        :
*/
int inSetESVC_Priority(char* szESVC_Priority)
{
        memset(srCFGTRec.szESVC_Priority, 0x00, sizeof(srCFGTRec.szESVC_Priority));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szESVC_Priority == NULL || strlen(szESVC_Priority) <= 0 || strlen(szESVC_Priority) > 4)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetESVC_Priority() ERROR !!");

                        if (szESVC_Priority == NULL)
                        {
                                inLogPrintf(AT, "szESVC_Priority == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szESVC_Priority length = (%d)", (int)strlen(szESVC_Priority));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szESVC_Priority[0], &szESVC_Priority[0], strlen(szESVC_Priority));

        return (VS_SUCCESS);
}

/*
Function        :inGetDFS_Contactless_Enable
Date&Time       :2017/4/12 下午 1:36
Describe        :
*/
int inGetDFS_Contactless_Enable(char* szDFS_Contactless_Enable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szDFS_Contactless_Enable == NULL || strlen(srCFGTRec.szDFS_Contactless_Enable) <= 0 || strlen(srCFGTRec.szDFS_Contactless_Enable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetDFS_Contactless_Enable() ERROR !!");

                        if (szDFS_Contactless_Enable == NULL)
                        {
                                inLogPrintf(AT, "szDFS_Contactless_Enable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDFS_Contactless_Enable length = (%d)", (int)strlen(srCFGTRec.szDFS_Contactless_Enable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szDFS_Contactless_Enable[0], &srCFGTRec.szDFS_Contactless_Enable[0], strlen(srCFGTRec.szDFS_Contactless_Enable));

        return (VS_SUCCESS);
}

/*
Function        :inSetDFS_Contactless_Enable
Date&Time       :2017/4/12 下午 1:36
Describe        :
*/
int inSetDFS_Contactless_Enable(char* szDFS_Contactless_Enable)
{
        memset(srCFGTRec.szDFS_Contactless_Enable, 0x00, sizeof(srCFGTRec.szDFS_Contactless_Enable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szDFS_Contactless_Enable == NULL || strlen(szDFS_Contactless_Enable) <= 0 || strlen(szDFS_Contactless_Enable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetDFS_Contactless_Enable() ERROR !!");

                        if (szDFS_Contactless_Enable == NULL)
                        {
                                inLogPrintf(AT, "szDFS_Contactless_Enable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szDFS_Contactless_Enable length = (%d)", (int)strlen(szDFS_Contactless_Enable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szDFS_Contactless_Enable[0], &szDFS_Contactless_Enable[0], strlen(szDFS_Contactless_Enable));

        return (VS_SUCCESS);
}

/*
Function        :inGetCloud_MFES
Date&Time       :2018/5/8 下午 3:04
Describe        :
*/
int inGetCloud_MFES(char* szCloud_MFES)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szCloud_MFES == NULL || strlen(srCFGTRec.szCloud_MFES) <= 0 || strlen(srCFGTRec.szCloud_MFES) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetCloud_MFES() ERROR !!");

                        if (szCloud_MFES == NULL)
                        {
                                inLogPrintf(AT, "szCloud_MFES == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCloud_MFES length = (%d)", (int)strlen(srCFGTRec.szCloud_MFES));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCloud_MFES[0], &srCFGTRec.szCloud_MFES[0], strlen(srCFGTRec.szCloud_MFES));

        return (VS_SUCCESS);
}

/*
Function        :inSetCloud_MFES
Date&Time       :2018/5/8 下午 3:05
Describe        :
*/
int inSetCloud_MFES(char* szCloud_MFES)
{
        memset(srCFGTRec.szCloud_MFES, 0x00, sizeof(srCFGTRec.szCloud_MFES));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCloud_MFES == NULL || strlen(szCloud_MFES) <= 0 || strlen(szCloud_MFES) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCloud_MFES() ERROR !!");

                        if (szCloud_MFES == NULL)
                        {
                                inLogPrintf(AT, "szCloud_MFES == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCloud_MFES length = (%d)", (int)strlen(szCloud_MFES));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szCloud_MFES[0], &szCloud_MFES[0], strlen(szCloud_MFES));

        return (VS_SUCCESS);
}

/*
Function        :inGetBIN_CHECK
Date&Time       :2019/9/16 下午 6:02
Describe        :
*/
int inGetBIN_CHECK(char* szBIN_CHECK)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szBIN_CHECK == NULL || strlen(srCFGTRec.szBIN_CHECK) <= 0 || strlen(srCFGTRec.szBIN_CHECK) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetBIN_CHECK() ERROR !!");

                        if (szBIN_CHECK == NULL)
                        {
                                inLogPrintf(AT, "szBIN_CHECK == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBIN_CHECK length = (%d)", (int)strlen(srCFGTRec.szBIN_CHECK));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szBIN_CHECK[0], &srCFGTRec.szBIN_CHECK[0], strlen(srCFGTRec.szBIN_CHECK));

        return (VS_SUCCESS);
}

/*
Function        :inSetBIN_CHECK
Date&Time       :2019/9/16 下午 6:02
Describe        :
*/
int inSetBIN_CHECK(char* szBIN_CHECK)
{
        memset(srCFGTRec.szBIN_CHECK, 0x00, sizeof(srCFGTRec.szBIN_CHECK));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szBIN_CHECK == NULL || strlen(szBIN_CHECK) <= 0 || strlen(szBIN_CHECK) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetBIN_CHECK() ERROR !!");

                        if (szBIN_CHECK == NULL)
                        {
                                inLogPrintf(AT, "szBIN_CHECK == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szBIN_CHECK length = (%d)", (int)strlen(szBIN_CHECK));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szBIN_CHECK[0], &szBIN_CHECK[0], strlen(szBIN_CHECK));

        return (VS_SUCCESS);
}

/*
Function        :inGetNCCC_Contactless_Enable
Date&Time       :2019/9/16 下午 6:03
Describe        :
*/
int inGetNCCC_Contactless_Enable(char* szNCCC_Contactless_Enable)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szNCCC_Contactless_Enable == NULL || strlen(srCFGTRec.szNCCC_Contactless_Enable) <= 0 || strlen(srCFGTRec.szNCCC_Contactless_Enable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetNCCC_Contactless_Enable() ERROR !!");

                        if (szNCCC_Contactless_Enable == NULL)
                        {
                                inLogPrintf(AT, "szNCCC_Contactless_Enable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szNCCC_Contactless_Enable length = (%d)", (int)strlen(srCFGTRec.szNCCC_Contactless_Enable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szNCCC_Contactless_Enable[0], &srCFGTRec.szNCCC_Contactless_Enable[0], strlen(srCFGTRec.szNCCC_Contactless_Enable));

        return (VS_SUCCESS);
}

/*
Function        :inSetNCCC_Contactless_Enable
Date&Time       :2019/9/16 下午 6:11
Describe        :
*/
int inSetNCCC_Contactless_Enable(char* szNCCC_Contactless_Enable)
{
        memset(srCFGTRec.szNCCC_Contactless_Enable, 0x00, sizeof(srCFGTRec.szNCCC_Contactless_Enable));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szNCCC_Contactless_Enable == NULL || strlen(szNCCC_Contactless_Enable) <= 0 || strlen(szNCCC_Contactless_Enable) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetNCCC_Contactless_Enable() ERROR !!");

                        if (szNCCC_Contactless_Enable == NULL)
                        {
                                inLogPrintf(AT, "szNCCC_Contactless_Enable == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szNCCC_Contactless_Enable length = (%d)", (int)strlen(szNCCC_Contactless_Enable));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szNCCC_Contactless_Enable[0], &szNCCC_Contactless_Enable[0], strlen(szNCCC_Contactless_Enable));

        return (VS_SUCCESS);
}

/*
Function        :inGetFORCE_CID
Date&Time       :2019/9/16 下午 6:19
Describe        :
*/
int inGetFORCE_CID(char* szFORCE_CID)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szFORCE_CID == NULL || strlen(srCFGTRec.szFORCE_CID) <= 0 || strlen(srCFGTRec.szFORCE_CID) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetFORCE_CID() ERROR !!");

                        if (szFORCE_CID == NULL)
                        {
                                inLogPrintf(AT, "szFORCE_CID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFORCE_CID length = (%d)", (int)strlen(srCFGTRec.szFORCE_CID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szFORCE_CID[0], &srCFGTRec.szFORCE_CID[0], strlen(srCFGTRec.szFORCE_CID));

        return (VS_SUCCESS);
}

/*
Function        :inSetFORCE_CID
Date&Time       :2019/9/16 下午 6:19
Describe        :
*/
int inSetFORCE_CID(char* szFORCE_CID)
{
        memset(srCFGTRec.szFORCE_CID, 0x00, sizeof(srCFGTRec.szFORCE_CID));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szFORCE_CID == NULL || strlen(szFORCE_CID) <= 0 || strlen(szFORCE_CID) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetFORCE_CID() ERROR !!");

                        if (szFORCE_CID == NULL)
                        {
                                inLogPrintf(AT, "szFORCE_CID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szFORCE_CID length = (%d)", (int)strlen(szFORCE_CID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szFORCE_CID[0], &szFORCE_CID[0], strlen(szFORCE_CID));

        return (VS_SUCCESS);
}

/*
Function        :inGetCHECK_ID
Date&Time       :2025/9/15 上午 11:23
Describe        :
*/
int inGetCHECK_ID(char* szCHECK_ID)
{
        /* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
        if (szCHECK_ID == NULL || strlen(srCFGTRec.szCHECK_ID) <= 0 || strlen(srCFGTRec.szCHECK_ID) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inGetCHECK_ID() ERROR !!");

                        if (szCHECK_ID == NULL)
                        {
                                inLogPrintf(AT, "szCHECK_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCHECK_ID length = (%d)", (int)strlen(srCFGTRec.szCHECK_ID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&szCHECK_ID[0], &srCFGTRec.szCHECK_ID[0], strlen(srCFGTRec.szCHECK_ID));

        return (VS_SUCCESS);
}

/*
Function        :inSetCHECK_ID
Date&Time       :2025/9/15 上午 11:24
Describe        :
*/
int inSetCHECK_ID(char* szCHECK_ID)
{
        memset(srCFGTRec.szCHECK_ID, 0x00, sizeof(srCFGTRec.szCHECK_ID));
        /* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
        if (szCHECK_ID == NULL || strlen(szCHECK_ID) <= 0 || strlen(szCHECK_ID) > 1)
        {
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        char    szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

                        inLogPrintf(AT, "inSetCHECK_ID() ERROR !!");

                        if (szCHECK_ID == NULL)
                        {
                                inLogPrintf(AT, "szCHECK_ID == NULL");
                        }
                        else
                        {
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "szCHECK_ID length = (%d)", (int)strlen(szCHECK_ID));
                                inLogPrintf(AT, szErrorMsg);
                        }
                }

                return (VS_ERROR);
        }
        memcpy(&srCFGTRec.szCHECK_ID[0], &szCHECK_ID[0], strlen(szCHECK_ID));

        return (VS_SUCCESS);
}

/*
Function        :inGetCHESG_FLAG
Date&Time       :2026/5/18
Describe        :
 * [114197] 中心收單作業ESG永續發展優化
 * [20260518] 新增功能
 */
int inGetCHESG_FLAG(char* szCHESG_FLAG)
{
	/* 傳進的指標 不得為空   Rec中的值長度須為正數 小於規定最大值 */
	if (szCHESG_FLAG == NULL || strlen(srCFGTRec.szCHESG_FLAG) <= 0 || strlen(srCFGTRec.szCHESG_FLAG) > 1)
	{
		/* debug */
		if (ginDebug == VS_TRUE)
		{
			char szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1]; /* debug message */

			inLogPrintf(AT, "inGetCHESG_FLAG() ERROR !!");

			if (szCHESG_FLAG == NULL)
			{
				inLogPrintf(AT, "szCHESG_FLAG == NULL");
			} else
			{
				memset(szErrorMsg, 0x00, sizeof (szErrorMsg));
				sprintf(szErrorMsg, "szCHESG_FLAG length = (%d)", (int) strlen(srCFGTRec.szCHESG_FLAG));
				inLogPrintf(AT, szErrorMsg);
			}
		}

		return (VS_ERROR);
	}
	memcpy(&szCHESG_FLAG[0], &srCFGTRec.szCHESG_FLAG[0], strlen(srCFGTRec.szCHESG_FLAG));

	return (VS_SUCCESS);
}

/*
Function        :inSetCHESG_FLAG
Date&Time       :2026/5/18 
Describe        :
 * [114197] 中心收單作業ESG永續發展優化
 * [20260518] 新增功能
 */
int inSetCHESG_FLAG(char* szCHESG_FLAG)
{
	memset(srCFGTRec.szCHESG_FLAG, 0x00, sizeof (srCFGTRec.szCHESG_FLAG));
	/* 傳進的指標 不得為空  傳進的值長度須為正數 小於規定最大值 */
	if (szCHESG_FLAG == NULL || strlen(szCHESG_FLAG) <= 0 || strlen(szCHESG_FLAG) > 1)
	{
		/* debug */
		if (ginDebug == VS_TRUE)
		{
			char szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1]; /* debug message */

			inLogPrintf(AT, "inSetCHESG_FLAG() ERROR !!");

			if (szCHESG_FLAG == NULL)
			{
				inLogPrintf(AT, "szCHESG_FLAG == NULL");
			} else
			{
				memset(szErrorMsg, 0x00, sizeof (szErrorMsg));
				sprintf(szErrorMsg, "szCHESG_FLAG length = (%d)", (int) strlen(szCHESG_FLAG));
				inLogPrintf(AT, szErrorMsg);
			}
		}

		return (VS_ERROR);
	}
	memcpy(&srCFGTRec.szCHESG_FLAG[0], &szCHESG_FLAG[0], strlen(szCHESG_FLAG));

	return (VS_SUCCESS);
}


/*
Function        :inFunc_Edit_CFGT_Table
Date&Time       :2017/3/28 下午 3:09
Describe        :
*/
int inCFGT_Edit_CFGT_Table(void)
{
	TABLE_GET_SET_TABLE CFGT_FUNC_TABLE[] =
	{
		{"szCustomIndicator", inGetCustomIndicator, inSetCustomIndicator},
		{"szNCCCFESMode", inGetNCCCFESMode, inSetNCCCFESMode},
		{"szCommMode", inGetCommMode, inSetCommMode},
		{"szDialBackupEnable", inGetDialBackupEnable, inSetDialBackupEnable},
		{"szEncryptMode", inGetEncryptMode, inSetEncryptMode},
		{"szSplitTransCheckEnable", inGetSplitTransCheckEnable, inSetSplitTransCheckEnable},
		{"szCityName", inGetCityName, inSetCityName},
		{"szStoreIDEnable", inGetStoreIDEnable, inSetStoreIDEnable},
		{"szMinStoreIDLen", inGetMinStoreIDLen, inSetMinStoreIDLen},
		{"szMaxStoreIDLen", inGetMaxStoreIDLen, inSetMaxStoreIDLen},
		{"szECREnable", inGetECREnable, inSetECREnable},
		{"szECRCardNoTruncateEnable", inGetECRCardNoTruncateEnable, inSetECRCardNoTruncateEnable},
		{"szECRExpDateReturnEnable", inGetECRExpDateReturnEnable, inSetECRExpDateReturnEnable},
		{"szProductCodeEnable", inGetProductCodeEnable, inSetProductCodeEnable},
		{"szPrtSlogan", inGetPrtSlogan, inSetPrtSlogan},
		{"szSloganStartDate", inGetSloganStartDate, inSetSloganStartDate},
		{"szSloganEndDate", inGetSloganEndDate, inSetSloganEndDate},
		{"szSloganPrtPosition", inGetSloganPrtPosition, inSetSloganPrtPosition},
		{"szPrtMode", inGetPrtMode, inSetPrtMode},
		{"szContactlessEnable", inGetContactlessEnable, inSetContactlessEnable},
		{"szVISAPaywaveEnable", inGetVISAPaywaveEnable, inSetVISAPaywaveEnable},
		{"szJCBJspeedyEnable", inGetJCBJspeedyEnable, inSetJCBJspeedyEnable},
		{"szMCPaypassEnable", inGetMCPaypassEnable, inSetMCPaypassEnable},
		{"szCUPRefundLimit", inGetCUPRefundLimit, inSetCUPRefundLimit},
		{"szCUPKeyExchangeTimes", inGetCUPKeyExchangeTimes, inSetCUPKeyExchangeTimes},
		{"szMACEnable", inGetMACEnable, inSetMACEnable},
		{"szPinpadMode", inGetPinpadMode, inSetPinpadMode},
		{"szFORCECVV2", inGetFORCECVV2, inSetFORCECVV2},
		{"szSpecialCardRangeEnable", inGetSpecialCardRangeEnable, inSetSpecialCardRangeEnable},
		{"szPrtMerchantLogo", inGetPrtMerchantLogo, inSetPrtMerchantLogo},
		{"szPrtMerchantName", inGetPrtMerchantName, inSetPrtMerchantName},
		{"szPrtNotice", inGetPrtNotice, inSetPrtNotice},
		{"szElecCommerceFlag", inGetElecCommerceFlag, inSetElecCommerceFlag},
		{"szDccFlowVersion", inGetDccFlowVersion, inSetDccFlowVersion},
		{"szSupDccVisa", inGetSupDccVisa, inSetSupDccVisa},
		{"szSupDccMasterCard", inGetSupDccMasterCard, inSetSupDccMasterCard},
		{"szDHCPRetryTimes", inGetDHCPRetryTimes, inSetDHCPRetryTimes},
		{"szBarCodeReaderEnable", inGetBarCodeReaderEnable, inSetBarCodeReaderEnable},
		{"szEMVPINBypassEnable", inGetEMVPINBypassEnable, inSetEMVPINBypassEnable},
		{"szCUPOnlinePINEntryTimeout", inGetCUPOnlinePINEntryTimeout, inSetCUPOnlinePINEntryTimeout},
		{"szSignPadMode", inGetSignPadMode, inSetSignPadMode},
		{"szESCPrintMerchantCopy", inGetESCPrintMerchantCopy, inSetESCPrintMerchantCopy},
		{"szESCPrintMerchantCopyStartDate", inGetESCPrintMerchantCopyStartDate, inSetESCPrintMerchantCopyStartDate},
		{"szESCPrintMerchantCopyEndDate", inGetESCPrintMerchantCopyEndDate, inSetESCPrintMerchantCopyEndDate},
		{"szESCReciptUploadUpLimit", inGetESCReciptUploadUpLimit, inSetESCReciptUploadUpLimit},
		{"szContactlessReaderMode", inGetContactlessReaderMode, inSetContactlessReaderMode},
		{"szTMSDownloadMode", inGetTMSDownloadMode, inSetTMSDownloadMode},
		{"szAMEXContactlessEnable", inGetAMEXContactlessEnable, inSetAMEXContactlessEnable},
		{"szCUPContactlessEnable", inGetCUPContactlessEnable, inSetCUPContactlessEnable},
		{"szSmartPayContactlessEnable", inGetSmartPayContactlessEnable, inSetSmartPayContactlessEnable},
		{"szPayItemEnable", inGetPayItemEnable, inSetPayItemEnable},
		{"szStore_Stub_CardNo_Truncate_Enable", inGetStore_Stub_CardNo_Truncate_Enable, inSetStore_Stub_CardNo_Truncate_Enable},
		{"szIntegrate_Device", inGetIntegrate_Device, inSetIntegrate_Device},
		{"szFES_ID", inGetFES_ID, inSetFES_ID},
		{"szIntegrate_Device_AP_ID", inGetIntegrate_Device_AP_ID, inSetIntegrate_Device_AP_ID},
		{"szShort_Receipt_Mode", inGetShort_Receipt_Mode, inSetShort_Receipt_Mode},
		{"szI_FES_Mode", inGetI_FES_Mode, inSetI_FES_Mode},
		{"szDHCP_Mode", inGetDHCP_Mode, inSetDHCP_Mode},
		{"szESVC_Priority", inGetESVC_Priority, inSetESVC_Priority},
		{"szDFS_Contactless_Enable", inGetDFS_Contactless_Enable, inSetDFS_Contactless_Enable},
		{"szCloud_MFES", inGetCloud_MFES, inSetCloud_MFES},
		{"szBIN_CHECK", inGetBIN_CHECK, inSetBIN_CHECK},
		{"szNCCC_Contactless_Enable", inGetNCCC_Contactless_Enable, inSetNCCC_Contactless_Enable},
		{"szFORCE_CID", inGetFORCE_CID, inSetFORCE_CID},
		{"szCHECK_ID", inGetCHECK_ID, inSetCHECK_ID},
		{"szCHESG_FLAG", inGetCHESG_FLAG, inSetCHESG_FLAG},
		{""},
	};
	int	inRetVal = VS_ERROR;
	char	szKey = 0x00;
	int	inFinalTimeout = 0;
	
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
	inDISP_ChineseFont_Color("是否更改CFGT", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
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
	
	/* 如果改到感應開關，而原先開機時沒initial感應器，遇到關LED燈時會crash START */
	char		szContactlessEnable[2 + 1];
	char		szContactlessReaderMode[2 + 1];
	unsigned char	usCTLSInitFlag = VS_FALSE;
	
	memset(szContactlessEnable, 0x00, sizeof(szContactlessEnable));
	memset(szContactlessReaderMode, 0x00, sizeof(szContactlessReaderMode));
	
	inGetContactlessEnable(szContactlessEnable);
	inGetContactlessReaderMode(szContactlessReaderMode);
	
	if (memcmp(szContactlessEnable, "Y", strlen("Y")) != 0 || memcmp(szContactlessReaderMode, "0", strlen("0")) == 0)
	{
		usCTLSInitFlag = VS_TRUE;
	}
	/* 如果改到感應開關，而原先開機時沒initial感應器，遇到關LED燈時會crash END */
	
	inFunc_Edit_Table_Tag(CFGT_FUNC_TABLE);
	inSaveCFGTRec(0);
	
	/* 如果改到感應開關，而原先開機時沒initial感應器，遇到關LED燈時會crash START */
	memset(szContactlessEnable, 0x00, sizeof(szContactlessEnable));
	memset(szContactlessReaderMode, 0x00, sizeof(szContactlessReaderMode));
	
	inGetContactlessEnable(szContactlessEnable);
	inGetContactlessReaderMode(szContactlessReaderMode);
	
	if (memcmp(szContactlessEnable, "Y", strlen("Y")) == 0 && memcmp(szContactlessReaderMode, "0", strlen("0")) != 0 && usCTLSInitFlag == VS_TRUE)
	{
		inCTLS_InitReader_Flow();
	}
	/* 如果改到感應開關，而原先開機時沒initial感應器，遇到關LED燈時會crash END */
	
	return	(VS_SUCCESS);
}
