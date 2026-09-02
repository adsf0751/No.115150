/* 
 * File:   TMSIPDT.h
 * Author: RussellBai
 *
 * Created on 2024年4月1日, 上午 10:33
 */

#ifndef TMSIPDT_H
#define	TMSIPDT_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* TMSIPDT_H */

typedef struct
{
	int	inRecordRowID;			/* SQLite使用，用於暫存Table的Rowid */
	char	szTMS_IP_Primary[15 + 1];
	char	szTMS_PortNo_Primary[5 + 1];
	char	szTMS_IP_Second[15 + 1];
	char	szTMS_PortNo_Second[5 + 1];
} TMSIPDT_REC;

/* 以欄位數決定 ex:欄位數是4個，理論上會有3個comma和兩個byte的0x0D 0X0A */
#define _SIZE_TMSIPDT_COMMA_0D0A_		5
#define _SIZE_TMSIPDT_REC_			(sizeof(TMSIPDT_REC))			/* 一個record不含comma和0D0A的長度 */
#define _TMSIPDT_FILE_NAME_			"TMSIPDT.dat"				/* File name */
#define _TMSIPDT_FILE_NAME_BAK_			"TMSIPDT.bak"				/* Bak name */
#define _TMSIPDT_TABLE_NAME_			"TMSIPDT"
#define _TMSIPDT_ORG_TMS_FILE_NAME_		"TMSIP.txt"				
#define _TMSIPDT_ORG_TMS_FILE_NAME_LEN_		9

/* Load & Save function */
int inLoadTMSIPDTRec(int inTMSIPDTRec);
int inLoadTMSIPDTRec_CTOS(int inTMSIPDTRec);
int inSaveTMSIPDTRec(int inTMSIPDTRec);
int inSaveTMSIPDTRec_CTOS(int inTMSIPDTRec);
int inTMSIPDT_Edit_TDT_Table(void);

int inGetTMS_IP_Primary(char* szTMS_IP_Primary);
int inSetTMS_IP_Primary(char* szTMS_IP_Primary);
int inGetTMS_PortNo_Primary(char* szTMS_PortNo_Primary);
int inSetTMS_PortNo_Primary(char* szTMS_PortNo_Primary);
int inGetTMS_IP_Second(char* szTMS_IP_Second);
int inSetTMS_IP_Second(char* szTMS_IP_Second);
int inGetTMS_PortNo_Second(char* szTMS_PortNo_Second);
int inSetTMS_PortNo_Second(char* szTMS_PortNo_Second);