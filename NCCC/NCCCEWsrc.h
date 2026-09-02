/* 
 * File:   NCCCEWsrc.h
 * Author: RussellBai
 *
 * Created on 2023年2月24日, 下午 4:59
 */

#ifndef NCCCEWSRC_H
#define	NCCCEWSRC_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* NCCCEWSRC_H */

/*
 * LINE Pay 21
 * 悠遊付 22
 * icashPay 23
 * 全盈+PAY 24
 * 全支付 25
 * Pi 錢包 26
 */
#define _EW_ISSUERID_LINEPAY_		"21"
#define _EW_ISSUERID_EASYWALLET_	"22"
#define _EW_ISSUERID_ICASHPAY_		"23"
#define _EW_ISSUERID_PLUSPAY_		"24"
#define _EW_ISSUERID_PXPAYPLUS_		"25"
#define _EW_ISSUERID_PIAPP_		"26"
#define	_EW_ISSUERID_LEN_		2

/*1：以信用卡扣款（CREDIT CARD）
2：以電支帳戶餘額扣款（BALANCE）
3：以銀行帳戶連結扣款（ACCOUNT LINK）
4：其他
5：異常
 */
#define _EW_PAY_METHOD_CREDIT_CARD_	"1"
#define _EW_PAY_METHOD_BALANCE_		"2"
#define _EW_PAY_METHOD_ACCOUNT_LINK_	"3"
#define _EW_PAY_METHOD_OTHER_		"4"
#define _EW_PAY_METHOD_UNUSUAL_		"5"
#define _EW_PAY_METHOD_LEN_		1

typedef struct
{
	int	incode;
	long	lnBatchNum;
	long	lnInvNum;
	EW_REC	srEWRec;
} DEMO_EW_REC;

int inNCCC_EW_RunTRT(TRANSACTION_OBJECT *pobTran, int inTRTCode);
int inNCCC_EW_Func_Check_Transaction_Function(int inCode);
int inNCCC_EW_Func_Check_Transaction_Function_Flow(TRANSACTION_OBJECT *pobTran);