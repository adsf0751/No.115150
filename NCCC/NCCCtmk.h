/* 
 * File:   NCCCtmk.h
 * Author: bai
 *
 * Created on 2016年1月5日, 上午 9:25
 */

/* TMK Key Card AID */
#define	_TMK_KEY_CARD_AID_	"\xA0\x00\x00\x01\x53\xF0\x01\x02"

/* Test Key data */
#define _MULTI_TMK_DATA_1_	"0123456789ABCDEF0123456789ABCDEF"
#define _MULTI_TMK_DATA_2_	"67310167C75EDA4034401064B38A40AE"
#define _MULTI_TMK_DATA_3_	"83625BC297AE57DFA238B0D679EA32D9"
#define _MULTI_TMK_DATA_4_	"494C70A78357C201C73B9B4A73F77A1A"
#define _MULTI_TMK_DATA_5_	"58AEA26E7008BCE37F97BFCD08BF4A92"
#define _MULTI_TMK_DATA_6_	"89B0250E3B3B9B0285086DF8CB2623F4"
#define _MULTI_TMK_DATA_7_	"C4C7924C58153D94E9F49D51ADAD683D"
#define _MULTI_TMK_DATA_8_	"4FE94CAD0410A4C152460DF2C8FE8F64"
#define _MULTI_TMK_DATA_9_	"DA547FE3152A29F82C19DF622C3BBAFB"
#define _MULTI_TMK_DATA_10_	"6B2F02DAA76DD01C40041694D316E07F"
#define _MULTI_TMK_DATA_11_	"23C73D3D970E5768C83E250B4FC2162F"
#define _MULTI_TMK_DATA_12_	"6E0473B6C746C8F4CB2F0E07326B2ACD"
#define _MULTI_TMK_DATA_13_	"5BFB57A27CC831438A51ABF2CE9D5DA1"
#define _MULTI_TMK_DATA_14_	"7A7F38624AEAC4B923329E97761A0837"
#define _MULTI_TMK_DATA_15_	"49F2CD8010A7EA4FC1526BBCBA4583AE"

/* 自購default */
#define _SELF_TRANS_MULTI_TMK_DATA_1_	"7C85753D0D5B0768D570CB1576E63EE5"
#define _SELF_TRANS_MULTI_TMK_DATA_2_	"EC2F370D2952C449E3160B831F8FDCAD"
#define _SELF_TRANS_MULTI_TMK_DATA_3_	"E60B8C9B6E37292392C1EABA83AB0BC7"
#define _SELF_TRANS_MULTI_TMK_DATA_4_	"38E3E95DFBE5571080F88C5E15EA15C4"
#define _SELF_TRANS_MULTI_TMK_DATA_5_	"57A71AF210942ABA7A19D32C10C2C86B"
#define _SELF_TRANS_MULTI_TMK_DATA_6_	"9BF2796734C7585840677ABC544049B5"
#define _SELF_TRANS_MULTI_TMK_DATA_7_	"37D638C780F2624C620BBAC40726571F"
#define _SELF_TRANS_MULTI_TMK_DATA_8_	"2FCED31A20C86D673B62F1B9ADB99E97"
#define _SELF_TRANS_MULTI_TMK_DATA_9_	"07D9AE293208AD86CDF7A17067C4C757"
#define _SELF_TRANS_MULTI_TMK_DATA_10_	"0176F25BF145AE62C26470160B57D0AB"
#define _SELF_TRANS_MULTI_TMK_DATA_11_	"572F758C0BA11F7067EA61B61A5E982F"
#define _SELF_TRANS_MULTI_TMK_DATA_12_	"9885C186AD8C0B1A256191204C924089"
#define _SELF_TRANS_MULTI_TMK_DATA_13_	"B6494CCD08B9988F01DAA8685EAD52FE"
#define _SELF_TRANS_MULTI_TMK_DATA_14_	"FE5E85A8101F2F3EA731232C6BD54358"
#define _SELF_TRANS_MULTI_TMK_DATA_15_	"25A4E0EC3BE63E7A29E3134C07916D58"

/* NCCC共15把 Key*/
#define _KEY_TOTAL_COUNT_	15

/* Test Key KCV */
#define _MULTI_TMK_KCV_1_	"D5D44FF72068"
#define _MULTI_TMK_KCV_2_	"5DDB28888304"
#define _MULTI_TMK_KCV_3_	"BB48C52B4161"
#define _MULTI_TMK_KCV_4_	"CE155617D835"
#define _MULTI_TMK_KCV_5_	"77EF9820083B"
#define _MULTI_TMK_KCV_6_	"E0C843A4D20E"
#define _MULTI_TMK_KCV_7_	"C0AC2CE7368C"
#define _MULTI_TMK_KCV_8_	"F5B6737268B4"
#define _MULTI_TMK_KCV_9_	"CE532F37192E"
#define _MULTI_TMK_KCV_10_	"4D7F471FE083"
#define _MULTI_TMK_KCV_11_	"4500AB40063B"
#define _MULTI_TMK_KCV_12_	"63D29E3CAE7E"
#define _MULTI_TMK_KCV_13_	"08AD8D5DD798"
#define _MULTI_TMK_KCV_14_	"4ECC1C165483"
#define _MULTI_TMK_KCV_15_	"792658B2E53A"

#define _KCV_TOTAL_COUNT_	15

#define _KEY_GROUP_ID_CEE_	0x05

typedef struct
{
	unsigned char	uszTMK_AID[22 + 1];				/* AID */
	unsigned char	uszTMK_KGI[1 + 1];				/* Key Group ID */
	unsigned char	uszTMK_KCT[1 + 1];				/* Key Count Total*/
	unsigned char	uszTMK_KID[_KCV_TOTAL_COUNT_][1 + 1];		/* TMK Key Index ‘01’~'0F'*/
	unsigned char	uszTMK_KCC[_KCV_TOTAL_COUNT_][2 + 1];		/* TMK 驗證碼 ‘01’ */
	unsigned char	uszTMK_KLC[2 + 1];				/* 讀取限制次數 */
	unsigned char	uszTMK_KUC[2 + 1];				/* 已讀取次數 */
	unsigned char	uszOTP_KCC[2 + 1];				/* OTP 驗證碼 ‘01’ */
	unsigned char	uszOTP_KTC[1 + 1];				/* OTP-key尚可嘗試錯誤次數 */
	unsigned char	uszStatusWord[2 + 1];				/* Status word */
	unsigned char	uszTMK_CRN[8 + 1];				/* 8個0~9的亂數 */
	unsigned char	uszGET_KGI[1 + 1];				/* get tmk的KGI */
	unsigned char	uszGET_KCT[1 + 1];				/* get Key Count Total*/
	unsigned char	uszGET_KID[_KCV_TOTAL_COUNT_][1 + 1];		/* TMK Key Index ‘01’~'0F'*/
	unsigned char	uszGET_KLEN[_KCV_TOTAL_COUNT_][1 + 1];		/* TMK Key Length ‘08' or '10' or '18' */
	unsigned char	uszTMK_DES[_KEY_TOTAL_COUNT_][24 + 1];		/* tmk DES key */
	unsigned char	uszTMK_DES_Ascii[_KEY_TOTAL_COUNT_][48 + 1];	/* tmk DES key */
	unsigned char	uszGET_KFC[2 + 1];				/* get tmk的KFC 剩下的讀取次數 */
} NCCC_TMK_COMMAND;

int inNCCC_TMK_Write_Test_TMK_By_Terminal(void);
int inNCCC_TMK_Write_Test_TMK_By_Terminal_Self_Trans(void);
int inNCCC_TMK_Write_TMK(int inKeyCnt, char *szKeyDataAscii, int inKeyDataAsciiLen);
int inNCCC_TMK_Write_TMK_By_KeyCard_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCC_TMK_Write_PINKey(unsigned short usTMKindex, unsigned short usPINLen, char *szPINKey, char* szKeyCheckValueAscii);
int inNCCC_TMK_Write_MACKey(unsigned short usTMKindex, unsigned short usMACKeyLen, char *szMACKeyAscii, char* szKeyCheckValueAscii);
int inNCCC_TMK_Write_ESCKey(TRANSACTION_OBJECT *pobTran);
int inNCCC_TMK_ESC_3DES_Encrypt(char* szInPlaindata, int inInPlaindataLen, char *szResult);
int inNCCC_TMK_CalculatePINBlock(TRANSACTION_OBJECT* pobTran, char *szOutputPINBlock);
int inNCCC_TMK_CalculateMac(char* szInPlaindata, char *szMACdata);
int inNCCC_TMK_GetKeyInfo_LookUp_Default(void);
int inNCCC_TMK_ProductionKey_Swap_To_Temp(void);
int inNCCC_TMK_Check_Test_Key(unsigned short usKeySet, unsigned short usKeyIndex, char *szTest);
int inNCCC_TMK_Select_AID(TRANSACTION_OBJECT *pobTran);
int inNCCC_TMK_GetAppletInfo(TRANSACTION_OBJECT *pobTran);
int inNCCC_TMK_GetKeyInfo_MultiKey(TRANSACTION_OBJECT *pobTran);
int inNCCC_TMK_GenerateRandomNumber(TRANSACTION_OBJECT *pobTran);
int inNCCC_TMK_VerifyOTP(TRANSACTION_OBJECT *pobTran);
int inNCCC_TMK_GetTMK_Multi(TRANSACTION_OBJECT *pobTran);
int inNCCC_TMK_Select_Device(TRANSACTION_OBJECT *pobTran);

int inNCCC_TMK_Test(void);
int inNCCC_TMK_ESCKey_Test(void);
int inNCCC_TMK_MFES_Test(void);
