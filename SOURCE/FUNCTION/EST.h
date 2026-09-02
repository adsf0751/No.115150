typedef struct
{
        char    szEMVCAPKIndex[2 + 1];          /* EMVCAPK索引 */
        char    szCAPKApplicationId[10 + 1];    /* 應用程式 ID */
        char    szCAPKIndex[2 + 1];             /* CAPK索引值(16進位) */
        char    szCAPKKeyExpireDate[8 + 1];     /* CAPK Key值有效期(端末機暫不參考CAPK有效期限) */
        char    szCAPKKeyLength[3 + 1];         /* CAPK Key值實際長度( 10進位，Byte計算) */
        char    szCAPKKeyModulus[496 + 1];      /* CAPK Key值 (原資料為 Binary 248 Bytes，故轉成 ASC 496 bytes) */
        char    szCAPKExponent[6 + 1];          /* CAPK Key值 Exponent (原資料為 Binary 3 Bytes，故轉成 ASC 6 bytes) */
} EST_REC;


/* 以欄位數決定 ex:欄位數是7個，理論上會有6個comma和兩個byte的0x0D 0X0A */
#define _SIZE_EST_COMMA_0D0A_           8
#define _SIZE_EST_REC_			(sizeof(EST_REC))		/*一個record不含comma和0D0A的長度*/
#define _EST_FILE_NAME_			"EST.dat"			/*file name*/
#define _EST_FILE_NAME_BAK_		"EST.bak"			/*bak name*/


/* Load & Save function */
int inLoadESTRec(int inESTRec);
int inSaveESTRec(int inESTRec);

/*Set function*/
int inSetEMVCAPKIndex(char* szEMVCAPKIndex);
int inSetCAPKApplicationId(char* szCAPKApplicationId);
int inSetCAPKIndex(char* szCAPKIndex);
int inSetCAPKKeyExpireDate(char* szCAPKKeyExpireDate);
int inSetCAPKKeyLength(char* szCAPKKeyLength);
int inSetCAPKKeyModulus(char* szCAPKKeyModulus);
int inSetCAPKExponent(char* szCAPKExponent);


/*Get function*/
int inGetEMVCAPKIndex(char* szEMVCAPKIndex);
int inGetCAPKApplicationId(char* szCAPKApplicationId);
int inGetCAPKIndex(char* szCAPKIndex);
int inGetCAPKKeyExpireDate(char* szCAPKKeyExpireDate);
int inGetCAPKKeyLength(char* szCAPKKeyLength);
int inGetCAPKKeyModulus(char* szCAPKKeyModulus);
int inGetCAPKExponent(char* szCAPKExponent);
