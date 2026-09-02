typedef struct
{
        char    szKeyGroupIndex[2 + 1];         /* 密鑰群組索引 */
        char    szKeyGroupID[13 + 1];           /* 密鑰群組 */
        char    szLastUpdateTime[14 + 1];       /* 最後一次密鑰群組修改時間 */
        char    szSessionData[32 + 1];          /* Session_Data */
        char    szTsamBIN[6 + 1];               /* Tsam_BIN */
        char    szKeySet[2 + 1];                /* Key_Set */
        char    szKeyIndex[2 + 1];              /* Key_Index */
        char    szKeyValue[32 + 1];             /* Key_Value */
        char    szKeyCheckValue[6 + 1];         /* Key_Check_Value */
} SKM_REC;


/* 以欄位數決定 ex:欄位數是9個，理論上會有8個comma和兩個byte的0x0D 0X0A */
#define _SIZE_SKM_COMMA_0D0A_           10
#define _SIZE_SKM_REC_                  (sizeof(SKM_REC))
#define _SKM_FILE_NAME_                 "SKM.dat"
#define _SKM_FILE_NAME_BAK_             "SKM.bak"


/* Load & Save function */
int inLoadSKMRec(int inSKMRec);
int inSaveSKMRec(int inSKMRec);

/* Set function */
int inSetKeyGroupIndex(char* szKeyGroupIndex);
int inSetKeyGroupID(char* szKeyGroupID);
int inSetLastUpdateTime(char* szLastUpdateTime);
int inSetSessionData(char* szSessionData);
int inSetTsamBIN(char* szTsamBIN);
int inSetKeySet(char* szKeySet);
int inSetKeyIndex (char* szKeyIndex);
int inSetKeyValue(char* szKeyValue);
int inSetKeyCheckValue(char* szKeyCheckValue);

/* Get function */
int inGetKeyGroupIndex(char* szKeyGroupIndex);
int inGetKeyGroupID(char* szKeyGroupID);
int inGetLastUpdateTime(char* szLastUpdateTime);
int inGetSessionData(char* szSessionData);
int inGetTsamBIN(char* szTsamBIN);
int inGetKeySet(char* szKeySet);
int inGetKeyIndex (char* szKeyIndex);
int inGetKeyValue(char* szKeyValue);
int inGetKeyCheckValue(char* szKeyCheckValue);

