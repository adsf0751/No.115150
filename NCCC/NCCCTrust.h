/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   NCCCTrust.h
 * Author: 1500414
 *
 * Created on 2025年10月3日, 上午 10:40
 */

#ifndef NCCCTRUST_H
#define NCCCTRUST_H

#ifdef __cplusplus
extern "C" {
#endif




#ifdef __cplusplus
}
#endif

#endif /* NCCCTRUST_H */

#define _TRUST_QRCODE_TYPE_LEN_		2
#define _TRUST_QRCODE_TYPE_STATIC_	"02"
#define _TRUST_QRCODE_TYPE_DYNAMIC_	"03"
/* [114261]信託模式一~三新增機構自行產QR Code 
 [260316] 新增動態QR (由信託機構自行加密產製) 的參數 */
#define _TRUST_QRCODE_TYPE_DYNAMIC_FORM_HOST_	"05"

int inNCCC_Trust_Deal_Trust_Txn_Parameter_OPT(TRANSACTION_OBJECT *pobTran);
int inNCCC_Trust_Table_Link_TrustRec(TRANSACTION_OBJECT *pobTran, SQLITE_ALL_TABLE *srAll, int inLinkState);
int inNCCC_Trust_VOID_Confirm(TRANSACTION_OBJECT *pobTran);
int inNCCC_Trust_RunTRT(TRANSACTION_OBJECT *pobTran, int inTRTCode);
int inNCCC_Trust_Check_Transaction_Function_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCC_Trust_Check_Transaction_Function(int inCode);
int inNCCC_Trust_Func_Must_SETTLE(TRANSACTION_OBJECT *pobTran);
int inNCCC_Trust_Func_Unpack_QRCode(TRANSACTION_OBJECT *pobTran);
int inNCCC_Trust_Func_GenMAC(TRANSACTION_OBJECT *pobTran, char *szF_03, char *szF_04, char *szF_11, char *szF_41);
int inNCCC_Trust_SendReversal(TRANSACTION_OBJECT *pobTran);
int inNCCC_Func_Display_Review_Settle_TRUST(TRANSACTION_OBJECT *pobTran);