/*
 * File:   Card.h
 * Author: user
 *
 * Created on 2015年8月20日, 下午 2:46
 */


#define _MAX_MONTHS_IN_YEAR_ 			12
#define _MIN_MONTHS_IN_YEAR_ 			1

/* 有效期轉檢查碼用 */
#define _EXP_ENCRYPT_				0
#define _EXP_DECRYPT_				1

/* 定義Track結構 */
typedef struct
{
	short           shTrack1Len;            /* track 1 len */
	short           shTrack2Len;            /* track 2 len */
	short           shTrack3Len;            /* track 3 len */
	char            szTrack1[128];          /* track 1 buffer */
	char            szTrack2[128];          /* track 2 buffer */
	char      	szTrack3[128];          /* track 3 buffer */
}TRACK_DATA;

#define TRACK_DATA_SIZE                         sizeof(TRACK_DATA)

typedef struct
{
	char	szCardHolderName[40 + 1];
	char	szTrack2[128+1];
}SWIPE_TRACK_DATA;

int inCARD_MSREvent(void);
int inCARD_Clean_MSR_Buffer(void);
int inCARD_GetTrack123(TRANSACTION_OBJECT *pobTran);
int inCARD_unPackCard(TRANSACTION_OBJECT *pobTran);
int inCARD_ValidTrack2_ExpDate(TRANSACTION_OBJECT* pobTran);
int inCARD_ValidTrack2_PAN(TRANSACTION_OBJECT * pobTran);
int inCARD_ValidTrack2_HGPAN(TRANSACTION_OBJECT * pobTran);
int inCARD_ValidTrack2_UCard_PAN(char *szCardNo);
int inCARD_ValidTrack2_UCard_PAN_MenuKeyIn(char *szCardNo);
int inCARD_GetBin(TRANSACTION_OBJECT *pobTran);
int inCARD_LoadGetCDTIndex(TRANSACTION_OBJECT *pobTran);
int inCARD_GetBin_HG(TRANSACTION_OBJECT *pobTran);
int inCARD_LoadGetCDTIndex_HG(TRANSACTION_OBJECT *pobTran);
int inCARD_ExpDateEncryptAndDecrypt(TRANSACTION_OBJECT *pobTran, char *szInBuf, char *szOutBuf, int inMethod);
int inCARD_Generate_Special_Card(char* szPAN);
int inCARD_UCardModGen(char *szAcct);

