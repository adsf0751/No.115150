/* 
 * File:   Event_Struct.h
 * Author: RussellBai
 *
 * Created on 2022年7月19日, 下午 5:58
 */

#ifndef EVENT_STRUCT_H
#define	EVENT_STRUCT_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef	struct	EventMenuItem
{
        int		inEventCode;            /* 儲存按鍵 */
	int		inPasswordLevel;	/* 輸入密碼的層級 */
	int		inCode;			/* 交易的類型 */
	int		inRunOperationID;	/* 執行 OPT.txt */
	int		inRunTRTID;		/* 執行 xxxxTRT.txt */
	int		inErrorMsg;		/* 錯誤訊息 */
        long            lnHGTransactionType;	/* 聯合_HAPPY GO_交易類別 */
	unsigned char	uszCUPTransBit;		/* 標示是否做銀聯交易 */
	unsigned char	uszInstallmentBit;	/* 標示是否為分期交易 */
	unsigned char	uszRedeemBit;		/* 標示是否為紅利交易 */
	unsigned char	uszECRBit;		/* 標示是否為收銀機交易 */
	unsigned char	uszAutoSettleBit;	/* 標示是否為連動結帳 */
        unsigned char   uszFISCTransBit;        /* 標示是否為SmartPay交易 */
	unsigned char   uszMailOrderBit;        /* 標示是否為郵購交易 */
	unsigned char	uszMultiFuncSlaveBit;	/* 標示是被外接設備 */
	unsigned char	uszESVCTransBit;	/* 標示是否是電票交易 */
	unsigned char	uszESVCLogOnDispBit;	/* 標示是否已顯示過LogOn畫面 */
	unsigned char	uszTrustTransBit;	/* 標示是否是信託交易 */
} EventMenuItem;


#ifdef	__cplusplus
}
#endif

#endif	/* EVENT_STRUCT_H */

