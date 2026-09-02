/* 
 * File:   NCCCEWiso.h
 * Author: RussellBai
 *
 * Created on 2023年2月21日, 下午 2:56
 */

#ifndef NCCCEWISO_H
#define	NCCCEWISO_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* NCCCEWISO_H */

#define _NCCC_EW_CHECK_BIT_MAP_(x, b)	((x) & (1U<<(b)))

#define _NCCC_EW_NULL_TX_                  0
#define _NCCC_EW_MAX_BIT_MAP_CNT_          40
#define _NCCC_EW_MTI_SIZE_                 2
#define _NCCC_EW_PCODE_SIZE_               3
#define _NCCC_EW_TPDU_SIZE_                5
#define _NCCC_EW_BIT_MAP_SIZE_             8
#define _NCCC_EW_RRN_SIZE_                 12 /* RRN */
#define _NCCC_EW_ISO_SEND_                 1536
#define _NCCC_EW_ISO_RECV_                 1536
#define _NCCC_EW_ISO_ASC_                  1 /* a */
#define _NCCC_EW_ISO_BCD_                  2
#define _NCCC_EW_ISO_NIBBLE_2_             3 /* ..nibble */
#define _NCCC_EW_ISO_NIBBLE_3_             4 /* ...nibble */
#define _NCCC_EW_ISO_BYTE_2_               5 /* ..ans */
#define _NCCC_EW_ISO_BYTE_3_               6 /* ...ans */
#define _NCCC_EW_ISO_BYTE_2_H_             7 /* ..ans */
#define _NCCC_EW_ISO_BYTE_3_H_             8 /* ...ans */
#define _NCCC_EW_ISO_ASC_1_                9 /* a */
#define _NCCC_EW_ISO_BYTE_1_               10/* ..ans */

typedef struct
{
        int inFieldNum; /* Field Number */
        int (*inISOLoad)(TRANSACTION_OBJECT *, unsigned char *); /* 組 Field 的功能 */
} ISO_FIELD_NCCC_EW_TABLE;

typedef struct
{
        int inFieldNum; /* Field Number */
        int (*inISOCheck)(TRANSACTION_OBJECT *, unsigned char *, unsigned char *); /* 檢查 Field 的功能 */
} ISO_CHECK_NCCC_EW_TABLE;

typedef struct
{
        int inFieldNum; /* Field Number */
        int inFieldType; /* Field Type */
        unsigned char uszDispAscii; /* 是否顯示其 ASCII 字元 */
        int inFieldLen; /* Field Len */
} ISO_FIELD_TYPE_NCCC_EW_TABLE;

typedef struct
{
        int inTxnID; /* 交易類別 */
        int *inBitMap; /* Bit Map */
        char szMTI[_NCCC_EW_MTI_SIZE_ * 2 + 1]; /* Message Type */
        char szPCode[_NCCC_EW_PCODE_SIZE_ * 2 + 1]; /* Processing Code */
} BIT_MAP_NCCC_EW_TABLE;

typedef struct
{
        ISO_FIELD_NCCC_EW_TABLE *srPackISO; /* 組封包的功能結構 */
        ISO_FIELD_NCCC_EW_TABLE *srUnPackISO; /* 解封包的功能結構 */
        ISO_CHECK_NCCC_EW_TABLE *srCheckISO; /* 檢查封包的功能結構 */
        ISO_FIELD_TYPE_NCCC_EW_TABLE *srISOFieldType; /* ISO Field 型態結構 */
        BIT_MAP_NCCC_EW_TABLE *srBitMap; /* Bit Map的陣列 */
        int (*inPackMTI)(TRANSACTION_OBJECT *, int , unsigned char *, char *); /* 組 Message Type 的功能 */
        int (*inModifyBitMap)(TRANSACTION_OBJECT *, int , int *);/* 組封包前修改 Bit Map 的功能 */
        int (*inModifyPackData)(TRANSACTION_OBJECT *, unsigned char *, int *); /* 組封包後修改整個封包的功能 */
        int (*inCheckISOHeader)(TRANSACTION_OBJECT *, char *, char *); /* 檢查 Message Type 的功能 */
        int (*inOnAnalyse)(TRANSACTION_OBJECT *); /* Online交易分析 */
        int (*inAdviceAnalyse)(TRANSACTION_OBJECT *, unsigned char *); /* 收到 Advice 後的分析 */
} ISO_TYPE_NCCC_EW_TABLE;

int inNCCC_EW_Func_SetTxnOnlineOffline(TRANSACTION_OBJECT *pobTran);
int inNCCC_EW_Func_BuildAndSendPacket(TRANSACTION_OBJECT *pobTran);
int inNCCC_EW_GetSTAN(TRANSACTION_OBJECT *pobTran);
int inNCCC_EW_SetSTAN(TRANSACTION_OBJECT *pobTran);
int inNCCC_EW_ProcessOnline(TRANSACTION_OBJECT *pobTran);
int inNCCC_EW_SendPackRecvUnPack(TRANSACTION_OBJECT *pobTran);
int inNCCC_EW_PackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, int inTxnCode);
int inNCCC_EW_GetBitMapMessagegTypeField03(TRANSACTION_OBJECT *pobTran, ISO_TYPE_NCCC_EW_TABLE *srISOFunc, int inIsoType, int *inTxnBitMap, unsigned char *uszSendBuf);
int inNCCC_EW_GetBitMapTableIndex(ISO_TYPE_NCCC_EW_TABLE *srISOFunc, int inBitMapTxnCode);
int inNCCC_EW_CopyBitMap(int *inBitMap, int *inSearchBitMap);
int inNCCC_EW_ISOPackMessageType(TRANSACTION_OBJECT *pobTran, int inISOTxnCode, unsigned char *uszPackData, char *szMTI);
int inNCCC_EW_ISOModifyBitMap(TRANSACTION_OBJECT *pobTran, int inISOTxnCode, int *inBitMap);
int inNCCC_EW_BitMapSet(int *inBitMap, int inFeild);
int inNCCC_EW_BitMapReset(int *inBitMap, int inFeild);
int inNCCC_EW_ISOModifyPackData(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackData, int *inPackLen);
int inNCCC_EW_CommSendRecvToHost(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, int inSendLen, unsigned char *uszRecvPacket);
void vdNCCC_EW_ISO_FormatDebug_DISP(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen);
void vdNCCC_EW_ISO_FormatDebug_DISP_59(unsigned char *uszDebugBuf, int inFieldLen);
void vdNCCC_EW_ISO_FormatDebug_PRINT(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen);
void vdNCCC_EW_ISO_FormatDebug_PRINT_59(unsigned char *uszDebugBuf, int inFieldLen, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inNCCC_EW_BitMapCheck(unsigned char *inBitMap, int inFeild);
int inNCCC_EW_GetFieldIndex(int inField, ISO_FIELD_TYPE_NCCC_EW_TABLE *srFieldType);
int inNCCC_EW_GetFieldLen(int inField, unsigned char *uszSendData, ISO_FIELD_TYPE_NCCC_EW_TABLE *srFieldType);
int inNCCC_EW_UnPackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, unsigned char *uszRecvBuf);
int inNCCC_EW_GetCheckField(int inField, ISO_CHECK_NCCC_EW_TABLE *ISOFieldCheck);
int inNCCC_EW_CheckUnPackField(int inField, ISO_FIELD_NCCC_EW_TABLE *srCheckUnPackField);
int inNCCC_EW_ISOCheckHeader(TRANSACTION_OBJECT *pobTran, char *szSendISOHeader, char *szReceISOHeader);
int inNCCC_EW_CheckRespCode(TRANSACTION_OBJECT *pobTran);
int inNCCC_EW_CheckAuthCode(TRANSACTION_OBJECT *pobTran);
int inNCCC_EW_AnalysePacket(TRANSACTION_OBJECT *pobTran);
int inNCCC_EW_ISOOnlineAnalyse(TRANSACTION_OBJECT *pobTran);
int inNCCC_EW_OnlineAnalyseNormal(TRANSACTION_OBJECT *pobTran);
int inNCCC_EW_SyncHostTerminalDateTime(TRANSACTION_OBJECT *pobTran);
int inNCCC_EW_DispHostResponseCode(TRANSACTION_OBJECT *pobTran);
/* Pack */
int inNCCC_EW_Pack03(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_EW_Pack04(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_EW_Pack11(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_EW_Pack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_EW_Pack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_EW_Pack22(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_EW_Pack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_EW_Pack25(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_EW_Pack32(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_EW_Pack35(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_EW_Pack35_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_EW_Pack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_EW_Pack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_EW_Pack41(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_EW_Pack42(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_EW_Pack48(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_EW_Pack57_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_EW_Pack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_EW_Pack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_EW_Pack62(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_EW_Pack64(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);

/* Check */
int inNCCC_EW_Check03(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket);
int inNCCC_EW_Check04(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket);
int inNCCC_EW_Check41(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket);

/* UnPack */
int inNCCC_EW_UnPack04(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_EW_UnPack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_EW_UnPack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_EW_UnPack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_EW_UnPack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_EW_UnPack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_EW_UnPack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_EW_UnPack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_EW_UnPack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_EW_UnPack62(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);