/* 
 * File:   TSAM.h
 * Author: user
 *
 * Created on 2016年1月5日, 下午 5:14
 */

#define _TSAM_SLOT_NCCC_			_SAM_SLOT_1_

/* 組 TSAM 資料
	SAM_ISO_CLA_COMMAND = 0x00
	SAM_INS_SELECT = 0xA4
	SAM_P1_SELETE = 0x04
	SAM_P2_00 = 0x00

	NCCC_TSAM	    A0 00 00 01 53 F0 01 01
	iPasss-ADD-SAM	41 44 44 53 41 4D
	ECC2-SAM_	    A0 00 00 03 22 10 08 01
	iCash2.0	        A0 00 00 01 53 F0 03 01
*/
#define _AID_SAM_NCCC_				"\xA0\x00\x00\x01\x53\xF0\x01\x01"
#define _AID_SAM_IPASS_				"\x41\x44\x44\x53\x41\x4D"
/*[需求單-115011] 新規格MSAM悠遊卡載入AID調整專案
 *[20260317] 刪除 _AID_SAM_ECC_ 並新增0x0501 AID,以利新mSAM Java Card 讀取 */
#define _AID_SAM_ECC_0501_			"\xA0\x00\x00\x03\x22\x10\x05\x01"
#define _AID_SAM_ECC_0801_			"\xA0\x00\x00\x03\x22\x10\x08\x01"
#define _AID_0501_	501
#define _AID_0801_	801

#define _AID_SAM_ICASH_				"\xA0\x00\x00\x01\x53\xF0\x03\x01"

/* tSAM Command */
/* For tSAM Encrypt (START) */
#define SAM_CLA_ENCRYPTION			0x80			/* CLA */
#define SAM_INS_ENCRYPTION			0x82			/* INS */
#define SAM_P1_ENCRYPTION			0x00			/* P1 */
#define SAM_P2_ENCRYPTION			0x00			/* P2 */
#define SAM_FUNC_ENCRYPTION                     0x00			/* FUNC */
#define SAM_MODE_ENCRYPTION			0x81			/* MODE */
/* For tSAM Encrypt (END) */

/* For tSAM Register TID (START) */
#define SAM_CLA_REGISTER_TID			0x80			/* CLA */
#define SAM_INS_REGISTER_TID			0xEA			/* INS */
#define SAM_P1_REGISTER_TID			0x00			/* P1 */
#define SAM_P2_REGISTER_TID			0x00			/* P2 */
#define SAM_LC_REGISTER_TID			0x04			/* LC */
/* For tSAM Register TID (END) */

/* For tSAM Get Serier Number (START) */
#define SAM_CLA_GET_SN				0x80			/* CLA */
#define SAM_INS_GET_SN				0xCA			/* INS */
#define SAM_P1_GET_SN				0x9F			/* P1 */
#define SAM_P2_GET_SN				0x7F			/* P2 */
#define SAM_LE_GET_SN				0x2D			/* LE */
/* For tSAM Get Serier Number (END) */

/* For tSAM Delete Key (START) */
#define SAM_CLA_DELETE_ALL_KEY		0x80			/* CLA */
#define SAM_INS_DELETE_ALL_KEY		0xEE			/* INS */
#define SAM_P1_DELETE_ALL_KEY			0x00			/* P1 */
#define SAM_P2_DELETE_ALL_KEY			0x00			/* P2 */
#define SAM_LC_DELETE_ALL_KEY			0x05			/* LC */
#define SAM_LE_DELETE_ALL_KEY			0x00			/* LE */
/* For tSAM Delete Key (END) */

/* For tSAM Download Key (START) */
#define SAM_CLA_DOWNLOAD_KEY			0x80			/* CLA */
#define SAM_INS_DOWNLOAD_KEY			0xF0			/* INS */
#define SAM_P1_DOWNLOAD_KEY			0x00			/* P1 */
#define SAM_P2_DOWNLOAD_KEY			0x00			/* P2 */
#define SAM_LC_DOWNLOAD_KEY			0x2A			/* LC */
#define SAM_LE_DOWNLOAD_KEY			0x00			/* LE */
/* For tSAM Download Key (END) */

/* For tSAM Select AID NCCC(START) */
#define SAM_CLA_SELECT_NCCC_AID			0x00			/* CLA */
#define SAM_INS_SELECT_NCCC_AID			0xA4			/* INS */
#define SAM_P1_SELECT_NCCC_AID			0x04			/* P1 */
#define SAM_P2_SELECT_NCCC_AID			0x00			/* P2 */
#define SAM_LC_SELECT_NCCC_AID			0x08			/* LC */
/* For tSAM Select AID NCCC(END) */

/* For tSAM Select AID IPASS(START) */
#define SAM_CLA_SELECT_IPASS_AID		0x00			/* CLA */
#define SAM_INS_SELECT_IPASS_AID		0xA4			/* INS */
#define SAM_P1_SELECT_IPASS_AID			0x04			/* P1 */
#define SAM_P2_SELECT_IPASS_AID			0x00			/* P2 */
#define SAM_LC_SELECT_IPASS_AID			0x06			/* LC */
/* For tSAM Select AID IPASS(END) */

/* For tSAM Select AID ECC(START) */
#define SAM_CLA_SELECT_ECC_AID			0x00			/* CLA */
#define SAM_INS_SELECT_ECC_AID			0xA4			/* INS */
#define SAM_P1_SELECT_ECC_AID			0x04			/* P1 */
#define SAM_P2_SELECT_ECC_AID			0x00			/* P2 */
#define SAM_LC_SELECT_ECC_AID			0x08			/* LC */
/* For tSAM Select AID ECC(END) */

/* For tSAM Select AID ICASH(START) */
#define SAM_CLA_SELECT_ICASH_AID		0x00			/* CLA */
#define SAM_INS_SELECT_ICASH_AID		0xA4			/* INS */
#define SAM_P1_SELECT_ICASH_AID			0x04			/* P1 */
#define SAM_P2_SELECT_ICASH_AID			0x00			/* P2 */
#define SAM_LC_SELECT_ICASH_AID			0x08			/* LC */
/* For tSAM Select AID ICASH(END) */

#define TPD_SIZE                                5
#define MESS_SIZE                               2
#define BMAP_SIZE                               8
#define F02_LSIZE                               1 	//Field2 帶的L長度
#define F03_SIZE                                3
#define F04_SIZE                                6
#define F11_SIZE                                3
#define F12_SIZE                                3
#define F13_SIZE                                2
#define F14_SIZE                                2
#define F15_SIZE                                2
#define F22_SIZE                                2
#define F24_SIZE                                2
#define F25_SIZE                                1
#define F35_LSIZE                               1
#define F37_SIZE                                12
#define F38_SIZE                                6
#define F39_SIZE                                2
#define F41_SIZE                                8
#define F42_SIZE                                15
#define F44_LSIZE                               1
#define F45_LSIZE                               1
#define F48_LSIZE                               2
#define F54_LSIZE                               2
#define F55_LSIZE                               2
#define F55_TSIZE                               1		//Field55 tag57 or tag5A的長度(其它tag的長度為2個byte)
#define F55_TLSIZE                              1		//Field55 各tag帶的L長度
#define F56_LSIZE                               2
#define F57_LSIZE                               2
#define F58_LSIZE                               2
#define F59_LSIZE                               2
#define F60_LSIZE                               2
#define F61_LSIZE                               2
#define F62_LSIZE                               2
#define F63_LSIZE                               2
#define F64_LSIZE                               2
#define LIMITSIZE                               8		//加密至少所需要的長度(8個byte的倍數)
#define CHECKSUMSIZE                            4
#define TIDSIZE                                 4
#define DIVSIZE                                 8
#define KEY_SESSION_DATA_SIZE			16

#define APDU_LENGTH                             sizeof(APDU_COMMAND)

int inNCCC_tSAM_APDUTransmit(unsigned char uszSlot, unsigned char *uszSendBuf, unsigned short usSendLen, unsigned char *uszReceBuf, int *usReceLen);
int inNCCC_tSAM_APDUEncrypt(unsigned char uszSlot, int	inLength, unsigned char	*uszData, unsigned char	*uszRRN, unsigned char *uszTID, unsigned char *uszKeyindex, unsigned char *uszCheckSum);
int inNCCC_tSAM_Encrypt(unsigned char uszSlot, int inDataLength, char *szInOutEncryptData, unsigned char *uszRRN, unsigned char *uszTID, unsigned char *uszKeyIndex, unsigned char *uszCheckSum);
int inNCCC_tSAM_Encrypt_ESVC(unsigned char uszSlot, int inDataLength, char *szInOutEncryptData, unsigned char *uszRRN, unsigned char *uszTID, unsigned char *uszKeyIndex, unsigned char *uszCheckSum);
int inNCCC_tSAM_InitialSLOT(TRANSACTION_OBJECT * pobTran);
int inNCCC_tSAM_SelectAID_NCCC_Flow(void);
int inNCCC_tSAM_SelectAID_NCCC(unsigned char uszSlot);
int inNCCC_tSAM_PowerOn_IPASS_Flow(void);
int inNCCC_tSAM_SelectAID_IPASS_Flow(void);
int inNCCC_tSAM_SelectAID_IPASS(unsigned char uszSlot);
int inNCCC_tSAM_PowerOn_ECC_Flow(void);
int inNCCC_tSAM_SelectAID_ECC_Flow(void);
/*[需求單-115011] 新規格MSAM悠遊卡載入AID調整專案 [20260317] 
  [20260317] 修改函式多傳入指定AID的參數 */
int inNCCC_tSAM_SelectAID_ECC(unsigned char uszSlot, int inAidValue);
int inNCCC_tSAM_PowerOn_ICASH_Flow(void);
int inNCCC_tSAM_SelectAID_ICASH_Flow(void);
int inNCCC_tSAM_SelectAID_ICASH(unsigned char uszSlot);
int inNCCC_tSAM_DeleteAllKey(unsigned char uszSlot);
int inNCCC_tSAM_DownloadKey(unsigned char uszSlot);
int inNCCC_tSAM_GetSlotSN1_Flow(void);
int inNCCC_tSAM_GetSlotSN2_Flow(void);
int inNCCC_tSAM_GetSlotSN(unsigned char uszSlot, char *szSlotSN);
int inNCCC_tSAM_Status_IsActive(unsigned char uszSlotNum);
int inNCCC_tSAM_Status_IsPresented(unsigned char uszSlotNum);
int inNCCC_tSAM_Slot_PowerOn(unsigned char uszSlot);
int inNCCC_tSAM_Slot_PowerOff(unsigned char uszSlot);
int inNCCC_tSAM_Slot_PowerOn_Warm(unsigned char uszSlot);
int inNCCC_tSAM_Check_SAM_In_Slot(unsigned char uszSlot);
int inNCCC_tSAM_Check_SAM_In_Slot_By_Global_Variable(unsigned char uszSlot);
int inNCCC_tSAM_Decide_tSAM_Slot(unsigned char *uszSlot);
int inNCCC_tSAM_PowerOn_Flow(unsigned char *uszSlot);
int inNCCC_tSAM_SAM_Encrypt_Fail_Remedy(unsigned char uszSlot);

/* 註冊SAM卡專用的function */
int inNCCC_tSAM_Register_Menu(TRANSACTION_OBJECT *pobTran);
int inNCCC_tSAM_Register_PowerOn(void);
int inNCCC_tSAM_SLOT1RegisterTID(void);
int inNCCC_tSAM_APDURegisterTID(unsigned char uszSlot, BYTE *pucTID);
int inNCCC_tSAM_CheckRegisterTID(unsigned char uszSlot, APDU_COMMAND *srAPDUData, BYTE *pucTID);

/*[需求單-115011] 新規格MSAM悠遊卡載入AID調整專案 [20260317] */
int inNCCC_tSAM_PselectSamAidMulti_ECC(unsigned char uszSlot);

/* castle sample code */
int intSAMTest(void);
