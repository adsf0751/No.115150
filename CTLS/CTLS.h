/* 
 * File:   CTLS.h
 * Author: user
 *
 * Created on 2016年1月26日, 上午 11:34
 */
#include <emv_cl.h>

#define _DEFAULT_CTLS_AUTO_OFF_TIME_		5000		/* (ms) */
#define _ADJUSTED_CTLS_AUTO_OFF_TIME_		20000		/* (ms) */

// Scheme ID
#define _SID_VISA_OLD_US_			0x13
//#define _SID_VISA_WAVE_2_			0x16
//#define _SID_VISA_WAVE_QVSDC_                   0x17
#define _SID_VISA_WAVE_MSD_			0x18
//#define _SID_PAYPASS_MAG_STRIPE_		0x20
//#define _SID_PAYPASS_MCHIP_			0x21
//#define _SID_JCB_WAVE_2_			0x61
#define _SID_JCB_WAVE_QVSDC_			0x62
//#define _SID_NEW_JSPEEDY_                       0x63
#define _SID_AE_EMV_				0x50
#define _SID_AE_MAG_STRIPE_			0x52
//#define _SID_QUICKPASS_                         0x91

#define SCHEME_ID_16_WAVE1			0x16
#define SCHEME_ID_17_WAVE3			0x17
#define SCHEME_ID_42_DPAS			0x42
#define SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE	0x43
#define SCHEME_ID_50_EXPRESSSPAY		0x50
#define SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE	0x52
#define SCHEME_ID_61_JSPEEDY			0x61	/* OLD JSpeedy */
#define SCHEME_ID_63_NEWJSPEEDY_EMV		0x63	/* NEW JSpeedy A卡 */
#define SCHEME_ID_64_NEWJSPEEDY_MSD		0x64	/* NEW JSpeedy B卡 */
#define SCHEME_ID_65_NEWJSPEEDY_LEGACY		0x65	/* NEW JSpeedy C卡 */
#define SCHEME_ID_20_PAYPASS_MAG_STRIPE		0x20
#define SCHEME_ID_21_PAYPASS_MCHIP		0x21
#define SCHEME_ID_91_QUICKPASS			0x91


#define SCHEME_ID_90_SMARTPAY			0x90

#define _TIMER_CTLS_			_TIMER_NEXSYS_4_

#define _CTLS_LIGHT_NONE_		0
#define _CTLS_LIGHT_RED_		1
#define _CTLS_LIGHT_GREEN_		2
#define _CTLS_LIGHT_YELLOW_		4
#define _CTLS_LIGHT_BLUE_		8

typedef struct
{
	unsigned long	lnSaleRespCode;
        unsigned char   uszSchemeID[2 + 1];
} CTLS_OBJECT;



int inCTLS_InitReader_Flow(void);
int inCTLS_InitReader_Internal(void);
int inCTLS_DeInitReader_Flow(void);
int inCTLS_DeInitReader_Internal(void);
int inCTLS_SetUIType(void);
int inCTLS_DevicePooling_Flow(TRANSACTION_OBJECT *pobTran);
int inCTLS_DevicePooling_Internal(TRANSACTION_OBJECT *pobTran);
int inCTLS_SendReadyForSale_Flow(TRANSACTION_OBJECT *pobTran);
int inCTLS_SendReadyForSale_Internal(TRANSACTION_OBJECT *pobTran);
int inCTLS_ReceiveReadyForSales_Flow(TRANSACTION_OBJECT *pobTran);
unsigned long ulCTLS_ReceiveReadyForSales_Internal(TRANSACTION_OBJECT *pobTran);
int inCTLS_CancelTransacton_Flow(void);
int inCTLS_CancelTransacton_Internal(void);
unsigned long ulCTLS_CheckResponseCode_SALE(TRANSACTION_OBJECT *pobTran);
int inCTLS_UnPackReadyForSale_Flow(TRANSACTION_OBJECT *pobTran);
int inCTLS_UnPackReadyForSale_Internal(TRANSACTION_OBJECT *pobTran);
int inCTLS_ProcessChipData(TRANSACTION_OBJECT *pobTran);
int inCTLS_ISOFormatDebug_DISP(char *szTag, unsigned short ushTaglen, char *szTagData);
int inCTLS_ISOFormatDebug_PRINT(char *szTag, unsigned short ushTaglen, char *szTagData);
int inCTLS_Wave2TransactionEnable(void);
int inCTLS_ExceptionCheck(TRANSACTION_OBJECT *pobTran);
int inCTLS_SendReadyForRefund_Flow(TRANSACTION_OBJECT *pobTran);
int inCTLS_SendReadyForRefund_Internal(TRANSACTION_OBJECT *pobTran);
int inCTLS_ReceiveReadyForRefund_Flow(TRANSACTION_OBJECT *pobTran);
unsigned long ulCTLS_ReceiveReadyForRefund_Internal(TRANSACTION_OBJECT *pobTran);
unsigned long ulCTLS_CheckResponseCode_Refund(TRANSACTION_OBJECT *pobTran);
int inCTLS_UnPackReadyForRefund_Flow(TRANSACTION_OBJECT *pobTran);
int inCTLS_UnPackReadyForRefund_Internal(TRANSACTION_OBJECT *pobTran);
int inCTLS_ExceptionCheck_Refund(TRANSACTION_OBJECT *pobTran);
void vdCTLS_EVENT_EMVCL_PRE_NON_EMV_CARD(OUT BYTE *pbNonEMVCard);
int inCTLS_Set_LED(int inColor);
int inCTLS_Clear_LED(void);
int inCTLS_Decide_Display_Image(TRANSACTION_OBJECT *pobTran);
int inCTLS_Check_TypeACard(void);
int inCTLS_Check_ISO14443_4_Card(void);
int inCTLS_IsCard_Still_Exist(void);
int inCTLS_Check_Mifare_Card(void);
int inCTLS_CheckRemoveCard(TRANSACTION_OBJECT *pobTran, int inIsError);
int inCTLS_LED_Wait_Start(void);
int inCTLS_LED_Wait_STOP(void);
int inCTLS_Mifare_LoadKey(unsigned char *uszKey);
int inCTLS_Mifare_Auth(unsigned char uszKeyType, unsigned char uszBlockIndex, unsigned char *uszCardSN, unsigned char uszCardSNLen);
int inCTLS_Get_TypeACardSN(char *szUID);
int inCTLS_ReActive_TypeACard(void);
int inCTLS_Power_On(void);
int inCTLS_Power_Off(void);
int inCTLS_Mifare_Read_Block(unsigned char usBlockNum, unsigned char *uszBlockContent);
int inCTLS_RATS(void);
int inCTLS_SetDebug(void);
USHORT OnEVENT_EMVCL_APP_LIST_V2(IN BYTE bAppNum, IN EMVCL_APP_LIST_DATA_V2 *pstAppListExData, OUT BYTE *pbAppSelectedIndex);