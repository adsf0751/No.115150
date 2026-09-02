typedef struct
{
        char szHostIndex[2 + 1];               /* 主機索引 */
        char szHostEnable[1 + 1];              /* 主機功能開關 */
        char szHostLabel[8 + 1];               /* 主機名稱 */
        char szMerchantID[15 + 1];             /* 商店代號 */
        char szTerminalID[8 + 1];              /* 端末機代號 */
        char szCommunicationIndex[2 + 1];      /* 通訊參數索引 */
        char szTransFunc[20 + 1];              /* 交易功能參數 */
        char szManualKeyin[1 + 1];             /* 人工輸入卡號功能 */
        char szCallBankEnable[1 + 1];          /* Call Bank功能 */
        char szTipPercent[3 + 1];              /* 小費檢合百分比 */
} HDT_REC;


/* 以欄位數決定 ex:欄位數是10個，理論上會有9個comma和兩個byte的0x0D 0X0A */
#define	_SIZE_HDT_COMMA_0D0A_           11
#define _SIZE_HDT_REC_                  (sizeof(HDT_REC))
#define _HDT_FILE_NAME_			"HDT.dat"
#define _HDT_FILE_NAME_BAK_             "HDT.bak"

/* Load & Save function */
int inSaveHDTRec(int inHDTRec);
int inLoadHDTRec(int inHDTRec);
int inHDT_Edit_HDT_Table(void);

/* Set function */
int inSetHostIndex(char* szHostIndex);
int inSetHostEnable(char* szHostEnable);
int inSetHostLabel(char* szHostLabel);
int inSetMerchantID(char* szMerchantID);
int inSetTerminalID(char* szTerminalID);
int inSetCommunicationIndex(char* szCommunicationInde);
int inSetTransFunc(char* szTransFunc);
int inSetManualKeyin(char* szManualKeyin);
int inSetCallBankEnable(char* szCallBankEnable);
int inSetTipPercent(char* szTipPercent);

/* Get function */
int inGetHostIndex(char* szHostIndex);
int inGetHostEnable(char* szHostEnable);
int inGetHostLabel(char* szHostLabel);
int inGetMerchantID(char* szMerchantID);
int inGetTerminalID(char* szTerminalID);
int inGetCommunicationIndex(char* szCommunicationInde);
int inGetTransFunc(char* szTransFunc);
int inGetManualKeyin(char* szManualKeyin);
int inGetCallBankEnable(char* szCallBankEnable);
int inGetTipPercent(char* szTipPercent);

int inHDT_Test1(void);

/*
 *	01=NCCC(MPAS)	"銷售(1Byte)+取消(1Byte)+結帳(1Byte)+退貨(1Byte)+交易補登(1Byte)+預先授權(1Byte)+小費(1Byte)
 *			 +分期付款(1Byte)+紅利扣抵(1Byte)+分期調帳(1Byte)+紅利調帳(1Byte)+郵購(1Byte)+補足 20 Bytes"
 * 
 *	02=DINERS	"銷售(1Byte)+取消(1Byte)+結帳(1Byte)+退貨(1Byte)+交易補登(1Byte)+預先授權(1Byte)+小費(1Byte)+調帳(1Byte)+補足 20 Bytes"
 * 				
 *	03=HG		"紅利積點(1Byte)+點數扣抵(1Byte)+加價購(1Byte)+點數兌換(1Byte)+點數查詢(1Byte)+回饋退貨(1Byte)+
 *			扣抵退貨(1Byte)+紅利積點人工輸入卡號(1Byte)+紅利積點列印簽單(1Byte)+補足 20 Bytes"
 * 
 *	04=VAS		"VAS主機的功能參數及主機名稱，請參閱加值系統參數檔。
 *			本欄位 20 Bytes 全部預設為關閉。"	
 * 			
 *	05=CLM		"支援CLM晶片功能(1Byte)+支援CLM磁條功能(1Byte)+支援CLM晶片及磁條功能(1Byte)+餘額轉置(1Byte)+
 *			後台調帳(1Byte)+後台退貨(1Byte)+補足 20 Bytes"
 * 
 *	06=AMEX		"銷售(1Byte)+取消(1Byte)+結帳(1Byte)+退貨(1Byte)+交易補登(1Byte)+預先授權(1Byte)+小費(1Byte)+
 *			調帳(1Byte)+補足 20 Bytes"
 * 
 *	07=DCC		"銷售(1Byte)+取消(1Byte)+結帳(1Byte)+退貨(1Byte)+交易補登保留(1Byte)+預先授權(1Byte)+小費(1Byte)+
 *			調帳(1Byte)+補足 20 Bytes"
 * 
 *	08=ESC		所有功能比照 01=NCCC
 * 
 *	09=ESVC	"	所有功能比照 01=NCCC請忽略HostDef.txt中的交易功能參數，要以各電票檔案的交易功能參數為主。"	
 * 
 *	10=TAKA(此為自行新增，若NCCC有新增其他Host，此Host要往後遞延)
 */			
