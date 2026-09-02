typedef struct
{
        char    szSCDTCardIndex[2 + 1];          /* 卡別索引 */
        char    szSCDTLowBinRange[11 + 1];       /* 低卡號範圍 */
        char    szSCDTHighBinRange[11 + 1];      /* 高卡號範圍 */
        char    szCampaignNumber[2 + 1];         /* 活動代碼 */
        char    szCampaignStartDate[8 + 1];      /* 活動起始日(YYYYMMDD) */
        char    szCampaignEndDate[8 + 1];        /* 活動結束日(YYYYMMDD) */
        char    szCampaignAmount[12 + 1];        /* 活動限額 */
} SCDT_REC;


/* 以欄位數決定 ex:欄位數是7個，理論上會有6個comma和兩個byte的0x0D 0X0A */
#define _SIZE_SCDT_COMMA_0D0A_                  8
#define _SIZE_SCDT_REC_				(sizeof(SCDT_REC))		/*一個record不含comma和0D0A的長度*/
#define _SCDT_FILE_NAME_			"SCDT.dat"			/*file name*/
#define _SCDT_FILE_NAME_BAK_			"SCDT.bak"			/*bak name*/

/*	
 *	00=符合此活動代碼之BIN，於交易時列印免簽名。
 *	01=符合此活動代碼之BIN，於收銀機連線時回傳明碼卡號給收銀機。
 */

#define	_CAMPAIGNNUMBER_LEN_			2
#define	_CAMPAIGNNUMBER_00_NO_SIGNATURE_	"00"
#define	_CAMPAIGNNUMBER_01_PLAIN_PAN_		"01"
#define	_CAMPAIGNNUMBER_05_EVER_RICH_COMBO_	"05"

/* Load & Save function */
int inLoadSCDTRec(int inSCDTRec);
int inSaveSCDTRec(int inSCDTRec);
int inSCDT_Edit_SCDT_Table(void);

/* Set function */
int inSetSCDTCardIndex(char* szSCDTCardIndex);
int inSetSCDTLowBinRange(char* szSCDTLowBinRange);
int inSetSCDTHighBinRange(char* szSCDTHighBinRange);
int inSetCampaignNumber(char* szCampaignNumber);
int inSetCampaignStartDate(char* szCampaignStartDate);
int inSetCampaignEndDate(char* szCampaignEndDate);
int inSetCampaignAmount(char* szCampaignAmount);


/* Get function */
int inGetSCDTCardIndex(char* szSCDTCardIndex);
int inGetSCDTLowBinRange(char* szSCDTLowBinRange);
int inGetSCDTHighBinRange(char* szSCDTHighBinRange);
int inGetCampaignNumber(char* szCampaignNumber);
int inGetCampaignStartDate(char* szCampaignStartDate);
int inGetCampaignEndDate(char* szCampaignEndDate);
int inGetCampaignAmount(char* szCampaignAmount);
