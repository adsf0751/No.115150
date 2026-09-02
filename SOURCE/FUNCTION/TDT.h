/* 
 * File:   TDT.h
 * Author: user
 *
 * Created on 2017年12月19日, 下午 3:46
 */

typedef struct
{
	int	inRecordRowID;			/* SQLite使用，用於暫存Table的Rowid */
	char	szTicket_HostIndex[2 + 1];
	char	szTicket_HostName[12 + 1];
	char	szTicket_HostTransFunc[20 + 1];
	char	szTicket_HostEnable[2 + 1];
	char	szTicket_LogOnOK[2 + 1];
	char	szTicket_SAM_Slot[2 + 1];
	char	szTicket_ReaderID[4 + 1];		/* For IPASS */
	char	szTicket_STAN[6 + 1];			/* FOR ECC */
	char	szTicket_LastTransDate[8 + 1];		/* FOR ECC 上次交易日期 YYYYMMDD */
	char	szTicket_LastRRN[15 + 1];		/* FOR ECC 上次交易RRN */
	char	szTicket_Device1[20 + 1];		/* FOR ECC 一代設備編號 */
	char	szTicket_Device2[20 + 1];		/* FOR ECC 二代設備編號 */
	char	szTicket_Batch[8 + 1];			/* FOR ECC 批次號碼之規則為yymmddxx(年月日(6) + 流水號(2)) */
	char	szTicket_NeedNewBatch[2 + 1];		/* FOR ECC 是否要更新批號(Y/N) */
	char	szTicket_Device3[20 + 1];		/* FOR ECC 悠遊卡Dongle Device ID */
	char	szTicket_ReversalBit[2 + 1];		/* 各票證是否要送Reversal */
	char	szTicket_LastAutoSignOnDate[6 + 1];	/* 最後一次自動SignOn日期，用來分辨本日是否已自動SignOn過 */
	char	szTicket_PIN[4 + 1];			/* For ICASH */
	char	szTicket_API_Version[20 + 1];		/* 各票證的API版本 */
} TDT_REC;

/* IPASS
 * 扣款(1Byte)+扣款取消(1Byte)+退貨(1Byte)+自動加值(1Byte)+現金加值(1Byte)+
 * 現金加值取消(1Byte)+保留(1Byte)+餘額查詢(1 Byte)+結帳(1 Byte)+補足15個Bytes。(Byte map=Y，表示功能開啟。Byte map=N，表示功能關閉) 
 */

/* 以欄位數決定 ex:欄位數是19個，理論上會有18個comma和兩個byte的0x0D 0X0A */
#define _SIZE_TDT_COMMA_0D0A_		20
#define _SIZE_TDT_REC_			(sizeof(TDT_REC))			/* 一個record不含comma和0D0A的長度 */
#define _TDT_FILE_NAME_			"TDT.dat"				/* File name */
#define _TDT_FILE_NAME_BAK_		"TDT.bak"				/* Bak name */
#define _TDT_TABLE_NAME_		"TDT"

#define _TDT_INDEX_00_IPASS_		0
#define _TDT_INDEX_01_ECC_		1
#define _TDT_INDEX_02_ICASH_		2
#define _TDT_INDEX_MAX_			3

#define _TDT_HOST_MAX_NUM_		_TDT_INDEX_MAX_

/* Load & Save function */
int inLoadTDTRec(int inTDTRec);
int inLoadTDTRec_SQLite(int inTDTRec);
int inLoadTDTRec_CTOS(int inTDTRec);
int inSaveTDTRec(int inTDTRec);
int inSaveTDTRec_CTOS(int inTDTRec);
int inSaveTDTRec_SQLite(int inTDTRec);
int inTDT_Table_Link_TDTRec(SQLITE_ALL_TABLE *srAll, int inLinkState);
int inTDT_Initial_AllRercord(char* szDBName, char* szTableName);
int inTDT_Edit_TDT_Table(void);

/* Get function */
int inGetTicket_HostIndex(char* szTicket_HostIndex);
int inGetTicket_HostName(char* szTicket_HostName);
int inGetTicket_HostTransFunc(char* szTicket_HostTransFunc);
int inGetTicket_HostEnable(char* szTicket_HostEnable);
int inGetTicket_LogOnOK(char* szTicket_LogOnOK);
int inGetTicket_SAM_Slot(char* szTicket_SAM_Slot);
int inGetTicket_ReaderID(char* szTicket_ReaderID);
int inGetTicket_STAN(char* szTicket_STAN);
int inGetTicket_LastTransDate(char* szTicket_LastTransDate);
int inGetTicket_LastRRN(char* szTicket_LastRRN);
int inGetTicket_Device1(char* szTicket_Device1);
int inGetTicket_Device2(char* szTicket_Device2);
int inGetTicket_Batch(char* szTicket_Batch);
int inGetTicket_NeedNewBatch(char* szTicket_NeedNewBatch);
int inGetTicket_Device3(char* szTicket_Device3);
int inGetTicket_ReversalBit(char* szTicket_ReversalBit);
int inGetTicket_LastAutoSignOnDate(char* szTicket_LastAutoSignOnDate);
int inGetTicket_PIN(char* szTicket_PIN);
int inGetTicket_API_Version(char* szTicket_API_Version);

/* Set function */
int inSetTicket_HostIndex(char* szTicket_HostIndex);
int inSetTicket_HostName(char* szTicket_HostName);
int inSetTicket_HostTransFunc(char* szTicket_HostTransFunc);
int inSetTicket_HostEnable(char* szTicket_HostEnable);
int inSetTicket_LogOnOK(char* szTicket_LogOnOK);
int inSetTicket_SAM_Slot(char* szTicket_SAM_Slot);
int inSetTicket_ReaderID(char* szTicket_ReaderID);
int inSetTicket_STAN(char* szTicket_STAN);
int inSetTicket_LastTransDate(char* szTicket_LastTransDate);
int inSetTicket_LastRRN(char* szTicket_LastRRN);
int inSetTicket_Device1(char* szTicket_Device1);
int inSetTicket_Device2(char* szTicket_Device2);
int inSetTicket_Batch(char* szTicket_Batch);
int inSetTicket_NeedNewBatch(char* szTicket_NeedNewBatch);
int inSetTicket_Device3(char* szTicket_Device3);
int inSetTicket_ReversalBit(char* szTicket_ReversalBit);
int inSetTicket_LastAutoSignOnDate(char* szTicket_LastAutoSignOnDate);
int inSetTicket_PIN(char* szTicket_PIN);
int inSetTicket_API_Version(char* szTicket_API_Version);
