/*
 * File:   Transaction.h
 * Author: user
 *
 * Created on 2015年7月27日, 下午 4:31
 */

#define _AUTH_CODE_SIZE_		6		/* Authorization code size */
#define _PAN_SIZE_			19              /* Primary account number */
#define _PAN_UCARD_SIZE_		21		/* 加兩個槓槓 */
#define _EXP_DATE_SIZE_			4               /* Expiry date MMYY */
#define _SVC_CODE_SIZE_			4               /* Size of service code */
#define _CARD_HOLDER_NAME_SIZE_		40              /* CardHolderName */
#define _MAX_TRACK1_NAME_SIZE_		48		/* CardHolderName最大長度 */
#define _DATE_SIZE_			6               /* Date in YYYYMMDD format */
#define _TIME_SIZE_			6               /* Time in HHMMSS format */
#define _DEBIT_ISSUER_ID_SIZE_          8
#define _DEBIT_CARD_COMMENT_SIZE_       30
#define _DEBIT_ACCOUNT_SIZE_            16
#define _FISC_ISSUER_ID_SIZE_           8               /* FISC 發卡單位代號長度*/
#define _FISC_CARD_COMMENT_SIZE_        30              /* FISC 備註欄長度*/
#define _FISC_ACCOUNT_SIZE_             16
#define _FISC_TCC_SIZE_			8
#define _FISC_MCC_SIZE_			15
#define _FISC_DATE_AND_TIME_SIZE_	14
#define _FISC_REFUND_DATE_		8
#define _ISSUER_NUM_SIZE_	        2
#define _PIN_SIZE_			8		/* Size of PIN data*/
#define _MAC_SIZE_			8
#define	_ENC_PIN_LENGTH_		8
#define _VSS_SMALL_BUFF_SIZE_		64
#define _VSS_MED_BUFF_SIZE_		128
#define _UID_SIZE_			30

#define _TRANSACTION_OBJECT_SIZE_       sizeof(TRANSACTION_OBJECT)
#define _BATCH_REC_SIZE_		sizeof(BATCH_REC)
#define _BATCH_KEY_SIZE_		sizeof(TRANS_BATCH_KEY)
#define _EMV_REC_SIZE_			sizeof(EMV_REC)

#define _ADV_FIELD_SIZE_		6

#include "Transaction_struct.h"