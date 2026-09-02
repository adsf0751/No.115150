/* 
 * File:   NCCCLoyalty.h
 * Author: user
 *
 * Created on 2017年2月14日, 下午 1:56
 */

#define _REWARD_FILE_NAME_		"Reward.dat"
#define _REWARD_ESVC_FILE_NAME_		"Reward_ESVC.dat"
#define _REWARD_BACKUP_FILE_NAME_	"Reward.bak"

#define _L5_ASMDATA_FILE_NAME_		"ASMDATA.dat"
#define _L5_ASMDATA_GZIP_FILE_NAME_	"ASMDATA.dat.gz"

#define _AWARD_ACTIVITY_MAX_NUM_	5	/* 活動最大數量*/
#define _AWARD_CANCELLATION_MAX_NUM_	2	/* 核銷最大數量*/

/* 這裡因為unicode的關係，內容部份都改1.5倍長度 */

typedef struct 
{
	char	szTitleFlag[2];			/* 是否列印優惠活動之標題 */
	char	szTitleLen[2 + 1];		/* 優惠活動之標題長度。(最大長度限制50 Bytes，右靠左補零) */
	char	szTitleContent[75 + 1];		/* 優惠活動之標題內容。(端末機列印時需自動置中) */
	char	sz39Code1Flag[2];		/* 是否列印一維條碼(一) */
	char	sz39Code1Len[2 + 1];		/* 一維條碼(一)長度。(最大長度限制20 Bytes，右靠左補空白) */
	char	sz39Code1Content[20 + 1];	/* 一維條碼(一) 補足空白至20 Bytes。 */
	char	sz39Code2Flag[2];		/* 是否列印一維條碼(二) */
	char	sz39Code2Len[2 + 1];		/* 一維條碼(二)長度。(最大長度限制20 Bytes，右靠左補空白) */
	char	sz39Code2Content[20 + 1];	/* 一維條碼(二) 補足空白至20 Bytes。 */
	char	szContentFlag[2];		/* 是否列印優惠活動之內容 1 byte */
	char	szContentLen[4];		/* 優惠活動之內容長度。(最大長度限制200 Bytes，左靠右補空白) */
	char	szContent[300 + 1];		/* 優惠活動之內容 */
	char	szChecksum[2];			/* 檢查碼(Checksum) */
} LOYALTY_AWARD_DATA;				/* 優惠結構 */

				/* L1結構 */

typedef struct 
{
	char	szADTitleFlag[2];		/* 優惠平台 L2 是否列印廣告標題資訊 */
	char	szADTitleLen[2 + 1];		/* 優惠平台 L2 廣告資訊標題長度 */
	char	szADTitleContent[75 + 1];	/* 優惠平台 L2 廣告資訊標題內容 */
	char	szADFlag[2];			/* 優惠平台 L2 是否列印廣告資訊 */
	char	szADLen[3 + 1];			/* 優惠平台 L2 廣告資訊長度 */
	char	szADContent[225 + 1];		/* 優惠平台 L2 廣告資訊內容 */
	char	szQRCodeFlag[2];		/* 優惠平台 L2 是否列印QR code */
	char	szQRCodeLen[3 + 1];		/* 優惠平台 L2 QR Code 長度 */
	char	szQRCodeContent[150 + 1];	/* 優惠平台 L2 QR Code 內容 */
	char	szChecksum[2];			/* 優惠平台 L2 checksum */
} LOYALTY_AD_DATA;				/* 廣告結構 */

typedef struct 
{
	char	szCancelFlag[2];		/* 優惠平台 L3 是否列印兌換核銷資訊(一)。’1’=列印，’0’=不列印 */
	char	szCancelLen[3 + 1];		/* 優惠平台 L3 兌換核銷資訊(一)長度。(右靠左補零) */
	char	szCancelContent[150 + 1];	/* 優惠平台 L3 兌換核銷資訊(一)之內容。(左靠右補空白) */
} LOYALTY_CANCELLATION_DATA;			/* 核銷結構 */

typedef struct 
{
	char			szPrintRewardNum[1 + 1];		/* 優惠平台 L1 & L2 優惠活動個數 */
	char			szAwardSN[22 + 1];			/* 優惠平台 TID (8Bytes) + YYYYMMDDhhmmss (14Bytes) */
	char			szSupInfFlag[1 + 1];			/* 優惠平台  是否列印補充資訊。‘0’=不列印，’1’=列印 */
	char			szSupInfLen[2 + 1];			/* 優惠平台  補充資訊實際長度。(最大長度限制20 Bytes，右靠左補零) */
	char			szSupInfLocation[2 + 1];		/* 優惠平台  補充資訊內容列印位置 */
	char			szSupInfContent[90 + 1];		/* 優惠平台  補充資訊內容 */
	LOYALTY_AWARD_DATA	srL1_AWARD_DATA[_AWARD_ACTIVITY_MAX_NUM_];
} LOYALTY_L1_DATA;

typedef struct 
{
	char			szPrintRewardNum[1 + 1];		/* 優惠平台 L1 & L2 優惠活動個數 */
	char			szAwardSN[22 + 1];			/* 優惠平台 TID (8Bytes) + YYYYMMDDhhmmss (14Bytes) */
	char			szSupInfFlag[1 + 1];			/* 優惠平台  是否列印補充資訊。‘0’=不列印，’1’=列印 */
	char			szSupInfLen[2 + 1];			/* 優惠平台  補充資訊實際長度。(最大長度限制20 Bytes，右靠左補零) */
	char			szSupInfLocation[2 + 1];		/* 優惠平台  補充資訊內容列印位置 */
	char			szSupInfContent[90 + 1];		/* 優惠平台  補充資訊內容 */
	LOYALTY_AWARD_DATA	srL2_AWARD_DATA;
	LOYALTY_AD_DATA		srL2_AD_DATA;
} LOYALTY_L2_DATA;

typedef struct 
{
	char				szRewardWay[1 + 1];				/* 優惠平台 L3兌換方式 */
	char				szAwardSN[22 + 1];				/* 優惠平台 TID (8Bytes) + YYYYMMDDhhmmss (14Bytes) */
	char				szSupInfFlag[1 + 1];				/* 優惠平台  是否列印補充資訊。‘0’=不列印，’1’=列印 */
	char				szSupInfLen[2 + 1];				/* 優惠平台  補充資訊實際長度。(最大長度限制20 Bytes，右靠左補零) */
	char				szSupInfContent[30 + 1];			/* 優惠平台  補充資訊內容(L3補充內容 20Byte) */
	LOYALTY_AWARD_DATA		srL3_AWARD_DATA;
	LOYALTY_CANCELLATION_DATA	srL3_CANCEL_DATA[_AWARD_CANCELLATION_MAX_NUM_];
	char				szChecksum[2];					/* 優惠平台 L3 checksum */
} LOYALTY_L3_DATA;

/* L5長的和L1一模一樣，但為了避免混淆，多放一個L5結構 */
typedef struct 
{
	char			szPrintRewardNum[1 + 1];		/* 優惠平台 L5 優惠活動個數 */
	char			szAwardSN[22 + 1];			/* 優惠平台 TID (8Bytes) + YYYYMMDDhhmmss (14Bytes) */
	char			szSupInfFlag[1 + 1];			/* 優惠平台  是否列印補充資訊。‘0’=不列印，’1’=列印 */
	char			szSupInfLen[2 + 1];			/* 優惠平台  補充資訊實際長度。(最大長度限制20 Bytes，右靠左補零) */
	char			szSupInfLocation[2 + 1];		/* 優惠平台  補充資訊內容列印位置 */
	char			szSupInfContent[90 + 1];		/* 優惠平台  補充資訊內容 */
	LOYALTY_AWARD_DATA	srL5_AWARD_DATA[_AWARD_ACTIVITY_MAX_NUM_];
} LOYALTY_L5_DATA;

typedef struct 
{
	char			szAwardName[2 + 1];			/* 優惠平台  判斷活動為何 'L1', 'L2', 'L3' */
	char			szSubTotalLen[2 + 1];			/* Table內的長度 (不知道會不會用到，先存)*/
	LOYALTY_L1_DATA		srL1DATA;
	LOYALTY_L2_DATA		srL2DATA;
	LOYALTY_L3_DATA		srL3DATA;
	LOYALTY_L5_DATA		srL5DATA;
} LOYALTY_L1L2L3_OBJECT;

#define _SIZE_ASM_DATA_		sizeof(LOYALTY_AWARD_DATA)

int inNCCC_Loyalty_ASM_Flag(void);
int inNCCC_Loyalty_CreditCardFlag(int inCode);
int inNCCC_Loyalty_BarCodeFlag(int inCode);
int inNCCC_Loyalty_VoidRedeemFlag(int inCode);
int inNCCC_Loyalty_RefundFlag(int inCode);
int inNCCC_Loyalty_Save_Reward_Data(unsigned long ulDataLen, char *szFileData, char* szSaveFileName);
int inNCCC_Loyalty_Save_Reward_Data_L5(unsigned long ulDataLen, char *szFileData, char* szSaveFileName);
int inNCCC_Loyalty_Process_GzipData(unsigned long ulDataLen, char *szFileData);
int inNCCC_Loyalty_Get_ASMDATA(unsigned char *uszReadData);
int inNCCC_Loyalty_Read_Reward_Data(unsigned long *ulBufferSize, char *szFileData, char* szFileName);
int inNCCC_Loyalty_Data_Format(LOYALTY_L1L2L3_OBJECT *srLoyaltyObj, int inDataLen, char *szData);

