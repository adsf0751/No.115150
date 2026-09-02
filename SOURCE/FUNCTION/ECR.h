/* 
 * File:   ECR.h
 * Author: user
 *
 * Created on 2017年6月1日, 下午 5:33
 */

/* 交易別 */
#define _ECR_8N1_MIRROR_			"00"    /* 映射訊息 */
#define _ECR_8N1_SALE_				"01"
#define _ECR_8N1_VOID_				"30"
#define _ECR_8N1_REFUND_			"02"
#define _ECR_8N1_OFFLINE_			"03"
#define _ECR_8N1_INSTALLMENT_			"04"
#define _ECR_8N1_REDEEM_			"05"
#define _ECR_8N1_INSTALLMENT_REFUND_		"06"
#define _ECR_8N1_REDEEM_REFUND_			"07"
#define _ECR_8N1_HOST_CHECK_			"08"	/* TK3C 使用 */
#define _ECR_8N1_INSTALLMENT_ADJUST_		"97"
#define _ECR_8N1_REDEEM_ADJUST_			"96"
#define _ECR_8N1_PREAUTH_			"19"
#define _ECR_8N1_PREAUTH_CANCEL_		"21"
#define _ECR_8N1_PREAUTH_COMPLETE_		"20"
#define _ECR_8N1_PREAUTH_COMPLETE_CANCEL_	"22"
#define _ECR_8N1_TIP_				"28"
#define _ECR_8N1_SETTLEMENT_			"50"
#define _ECR_8N1_START_CARD_NO_INQUIRY_		"60"
#define _ECR_8N1_INQUIRY_LAST_TRANSACTION_	"62"
#define _ECR_8N1_EI_TRANSACTION_                "63"
#define _ECR_8N1_END_CARD_NO_INQUIRY_		"70"
#define _ECR_8N1_REPRINT_RECEIPT_		"91"
#define _ECR_8N1_MENU_REVIEW_DETAIL_		"92"
#define _ECR_8N1_MENU_REVIEW_TOTAL_		"93"
#define _ECR_8N1_MENU_REPORT_DETAIL_		"94"
#define _ECR_8N1_MENU_REPORT_TOTAL_		"95"
#define _ECR_8N1_EDC_REBOOT_			"99"
#define _ECR_8N1_AWARD_REDEEM_			"40"
#define _ECR_8N1_VOID_AWARD_REDEEM_		"41"
#define _ECR_8N1_ESVC_TOP_UP_			"65"	/* ESVC加值(現金加值) */
#define _ECR_8N1_ESVC_BALANCE_INQUIRY_		"66"	/* ESVC餘額查詢 */
#define _ECR_8N1_ESVC_VOID_TOP_UP_		"67"	/* ESVC加值取消(現金加值) */
#define _ECR_8N1_HG_VOID_			"30"	/* 快樂購取消 */
#define _ECR_8N1_HG_REWARD_SALE_		"81"	/* 快樂購紅利積點(一般交易) */
#define _ECR_8N1_HG_REWARD_INSTALLMENT_		"82"	/* 快樂購紅利積點 + 信用卡分期付款 */
#define _ECR_8N1_HG_REWARD_REDEMPTION_		"83"	/* 快樂購紅利積點 + 信用卡紅利扣抵 */
#define _ECR_8N1_HG_ONLINE_REDEEMPTION_		"84"	/* 快樂購點數扣抵 */
#define _ECR_8N1_HG_POINT_CERTAIN_		"85"	/* 快樂購加價購 */
#define _ECR_8N1_HG_FULL_REDEEMPTION_		"86"	/* 快樂購點數兌換 */
#define _ECR_8N1_HG_REDEEM_REFUND_		"87"	/* 快樂購扣抵退貨 */
#define _ECR_8N1_HG_REWARD_REFUND_		"88"	/* 快樂購回饋退貨 */
#define _ECR_8N1_HG_POINT_INQUIRY_		"89"	/* 快樂購點數查詢 */
#define _ECR_8N1_ECHO_                          "98"	/* Echo Test */
#define _ECR_8N1_EW_INQUIRY_TRANSACTION_	"68"	/* 電子錢包交易查詢 */

#define _ECR_8N1_AWARD_REDEEM_LEN_		2
#define _ECR_8N1_VOID_AWARD_REDEEM_LEN_		2

#define _ECR_8N1_SALE_NO_			1	/* 一般交易	/SmartPay消費扣款	*/
#define _ECR_8N1_VOID_NO_			30	/* 取消		/SmartPay消費扣款沖正	*/
#define _ECR_8N1_REFUND_NO_			2	/* 退貨		/SmartPay消費扣款退貨	*/
#define _ECR_8N1_OFFLINE_NO_			3	/* 交易補登 */
#define _ECR_8N1_INSTALLMENT_NO_		4	/* 分期付款 */
#define _ECR_8N1_REDEEM_NO_			5	/* 紅利扣抵 */
#define _ECR_8N1_INSTALLMENT_REFUND_NO_		6	/* 分期退貨 */
#define _ECR_8N1_REDEEM_REFUND_NO_		7	/* 紅利退貨 */
#define _ECR_8N1_HOST_CHECK_NO_			8	/* TK3C 使用 */
#define _ECR_8N1_INSTALLMENT_ADJUST_NO_		97	/* 分期調帳 */
#define _ECR_8N1_REDEEM_ADJUST_NO_		96	/* 紅利調帳 */
#define _ECR_8N1_PREAUTH_NO_			19	/* 預先授權 */
#define _ECR_8N1_PREAUTH_CANCEL_NO_		21	/* 預先授權取消(保留未來銀聯卡交易使用) */
#define _ECR_8N1_PREAUTH_COMPLETE_NO_		20	/* 預先授權完成 */
#define _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_	22	/* 預先授權完成取消(保留未來銀聯卡交易使用) */
#define _ECR_8N1_TIP_NO_			28	/* 小費交易 */
#define _ECR_8N1_SETTLEMENT_NO_			50	/* 結帳 */
#define _ECR_8N1_START_CARD_NO_INQUIRY_NO_	60	/* 啟動卡號查詢 */
#define _ECR_8N1_INQUIRY_LAST_TRANSACTION_NO_	62	/* 查詢上一筆 */
#define _ECR_8N1_EI_TRANSACTION_NO_             63	/* 電子發票 */
#define _ECR_8N1_END_CARD_NO_INQUIRY_NO_	70	/* 結束卡號查詢 */
#define _ECR_8N1_REPRINT_RECEIPT_NO_		91	/* 重印簽單 */
#define _ECR_8N1_MENU_REVIEW_DETAIL_NO_		92	/* 交易明細查詢 */
#define _ECR_8N1_MENU_REVIEW_TOTAL_NO_		93	/* 交易總額查詢 */
#define _ECR_8N1_MENU_REPORT_DETAIL_NO_		94	/* 交易明細列印 */
#define _ECR_8N1_MENU_REPORT_TOTAL_NO_		95	/* 交易總額列印 */
#define _ECR_8N1_EDC_REBOOT_NO_			99	/* EDC重新開機 */
#define _ECR_8N1_AWARD_REDEEM_NO_		40	/* 優惠兌換 */
#define _ECR_8N1_VOID_AWARD_REDEEM_NO_		41	/* 取消優惠兌換 */
#define _ECR_8N1_ESVC_TOP_UP_NO_		65	/* ESVC加值(現金加值) */
#define _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_	66	/* ESVC餘額查詢 */
#define _ECR_8N1_ESVC_VOID_TOP_UP_NO_		67	/* ESVC加值取消(現金加值) */
#define _ECR_8N1_HG_VOID_NO_			30	/* 快樂購取消(同一般取消) */
#define _ECR_8N1_HG_REWARD_SALE_NO_		81	/* 快樂購紅利積點(一般交易) */
#define _ECR_8N1_HG_REWARD_INSTALLMENT_NO_	82	/* 快樂購紅利積點 + 信用卡分期付款 */
#define _ECR_8N1_HG_REWARD_REDEMPTION_NO_	83	/* 快樂購紅利積點 + 信用卡紅利扣抵 */
#define _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_	84	/* 快樂購點數扣抵 */
#define _ECR_8N1_HG_POINT_CERTAIN_NO_		85	/* 快樂購加價購 */
#define _ECR_8N1_HG_FULL_REDEEMPTION_NO_	86	/* 快樂購點數兌換 */
#define _ECR_8N1_HG_REDEEM_REFUND_NO_		87	/* 快樂購扣抵退貨 */
#define _ECR_8N1_HG_REWARD_REFUND_NO_		88	/* 快樂購回饋退貨 */
#define _ECR_8N1_HG_POINT_INQUIRY_NO_		89	/* 快樂購點數查詢 */
#define _ECR_8N1_ECHO_NO_                       98	/* Echo */
#define _ECR_8N1_EW_INQUIRY_TRANSACTION_NO_	68	/* 電子錢包交易查詢 */

/* ECR_NCCC_HOSTID */
#define	_ECR_8N1_NCCC_HOSTID_NCCC_			"03"	/* U CARD、VISA、MASTER、JCB、CUP、SMART PAY*/
#define	_ECR_8N1_NCCC_HOSTID_DCC_			"04"	/* VISA、MASTER之外幣交易 */
#define	_ECR_8N1_NCCC_HOSTID_DINERS_			"00"	/* DINERS */
#define	_ECR_8N1_NCCC_HOSTID_LOYALTY_REDEEM_		"05"	/* 優惠兌換主機(ASM) */
#define	_ECR_8N1_NCCC_HOSTID_HG_			"13"	/* HappyGo 規格沒寫，照Code抄 */
#define	_ECR_8N1_NCCC_HOSTID_ESVC_			"06"	/* 悠遊卡、一卡通、愛金卡及有錢卡 */
#define _ECR_8N1_NCCC_HOSTID_EW_			"08"	/* 電子錢包 LINE Pay 、悠遊付 、 i c ash Pay 、全盈支付 、 全支付、 Pi 錢包 */
#define _ECR_8N1_NCCC_HOSTID_TRUST_			"09"	/* 信託交易 */

#define	_ECR_8N1_NCCC_HOSTID_CUS_005_FTC_HG_		"09"

/* ECR_NCCC_CARDTYPE */
#define	_ECR_8N1_NCCC_CARDTYPE_UNKNOWN_			"  "	/* 未知 */
#define	_ECR_8N1_NCCC_CARDTYPE_UNKNOWN_LEN_		2
#define	_ECR_8N1_NCCC_CARDTYPE_UCARD_			"01"	/* U CARD*/
#define	_ECR_8N1_NCCC_CARDTYPE_VISA_			"02"	/* VISA */
#define	_ECR_8N1_NCCC_CARDTYPE_MASTERCARD_		"03"	/* MASTERCARD */
#define	_ECR_8N1_NCCC_CARDTYPE_JCB_			"04"	/* JCB */
#define	_ECR_8N1_NCCC_CARDTYPE_AMEX_			"05"	/* AMEX */
#define	_ECR_8N1_NCCC_CARDTYPE_CUP_			"06"	/* CUP */
#define	_ECR_8N1_NCCC_CARDTYPE_DINERS_			"07"	/* DINERS */
#define	_ECR_8N1_NCCC_CARDTYPE_SMARTPAY_		"08"	/* SMART PAY */
#define	_ECR_8N1_NCCC_CARDTYPE_ECC_			"11"	/* ECC(悠遊卡) */
#define	_ECR_8N1_NCCC_CARDTYPE_IPASS_			"12"	/* iPASS(一卡通) */
#define	_ECR_8N1_NCCC_CARDTYPE_ICASH_			"13"	/* iCash(愛金卡) */

#define	_ECR_8N1_NCCC_CARDTYPE_CUS_005_FTC_CREDIT_	"0"	/* CREDIT CARD */
#define	_ECR_8N1_NCCC_CARDTYPE_CUS_005_FTC_COMBO_	"1"	/* 聯名卡 */
#define	_ECR_8N1_NCCC_CARDTYPE_CUS_005_FTC_HG_		"2"	/* HG */
#define	_ECR_8N1_NCCC_CARDTYPE_CUS_005_FTC_FETC_	"3"	/* FETC */
#define	_ECR_8N1_NCCC_CARDTYPE_CUS_005_FTC_CUP_		"4"	/* CUP */

/* 交易別 */
#define _ECR_7E1_SALE_				"01"
#define _ECR_7E1_REFUND_			"02"
#define _ECR_7E1_OFFLINE_			"03"
#define _ECR_7E1_PREAUTH_			"19"
#define _ECR_7E1_INSTALLMENT_			"04"
#define _ECR_7E1_REDEEM_			"05"
#define _ECR_7E1_INSTALLMENT_REFUND_		"22"
#define _ECR_7E1_REDEEM_REFUND_			"32"
#define _ECR_7E1_VOID_				"30"
#define _ECR_7E1_SETTLEMENT_			"50"
#define _ECR_7E1_START_CARD_NO_INQUIRY_		"60"
#define _ECR_7E1_END_CARD_NO_INQUIRY_		"70"
#define _ECR_7E1_INSTALLMENT_ADJUST_		"97"
#define _ECR_7E1_REDEEM_ADJUST_			"96"
#define _ECR_7E1_HG_POINT_INQUIRY_		"65"	/* 點數查詢 */
#define _ECR_7E1_HG_REWARD_SALE_		"81"	/* 紅利積點 */
#define _ECR_7E1_HG_REWARD_INSTALLMENT_		"84"	/* 紅利積點-分期 */
#define _ECR_7E1_HG_REWARD_REDEMPTION_		"85"	/* 紅利積點-紅利 */
#define _ECR_7E1_HG_ONLINE_REDEEMPTION_		"86"	/* 點數扣抵 */
#define _ECR_7E1_HG_POINT_CERTAIN_		"87"	/* 加價購 */
#define _ECR_7E1_HG_FULL_REDEEMPTION_		"88"	/* 點數扣抵 */
#define _ECR_7E1_HG_REDEEM_REFUND_		"91"	/* 扣抵退貨 */
#define _ECR_7E1_HG_REWARD_REFUND_		"92"	/* 回饋退貨 */
#define _ECR_7E1_CASH_SALE_			"141"	/* 預借現金 */

/* CUSTOMER_002 */
#define _ECR_7E1_CUSTOMER_002_INSTALLMENT_REFUND_	"06"	/* 分期退貨 */
#define _ECR_7E1_CUSTOMER_002_REDEEM_REFUND_		"07"	/* 紅利退貨 */
#define _ECR_7E1_CUSTOMER_002_INQUIRY_			"38"	/* 查詢會員編號 */

/* CUSTOMER_027 */
#define _ECR_7E1_CUSTOMER_027_CHUNGHWA_TELECOM_TIP_			"40"	/* 中華電信收銀機規格定義小費 */

/* CUSTOMER_035 */
#define _ECR_7E1_CUSTOMER_035_MIRAMAR_CINEMAS_SETTLEMENT_	"55"

#define _ECR_7E1_SALE_NO_			1	/* 一般交易	/SmartPay消費扣款	*/
#define _ECR_7E1_REFUND_NO_			2	/* 退貨		/SmartPay消費扣款退貨	*/
#define _ECR_7E1_OFFLINE_NO_			3	/* 交易補登 */
#define _ECR_7E1_PREAUTH_NO_			19	/* 預先授權 */
#define _ECR_7E1_INSTALLMENT_NO_		4	/* 分期付款 */
#define _ECR_7E1_REDEEM_NO_			5	/* 紅利扣抵 */
#define _ECR_7E1_INSTALLMENT_REFUND_NO_		22	/* 分期退貨 */
#define _ECR_7E1_REDEEM_REFUND_NO_		32	/* 紅利退貨 */
#define _ECR_7E1_VOID_NO_			30	/* 取消		/SmartPay消費扣款沖正	*/
#define _ECR_7E1_SETTLEMENT_NO_			50	/* 結帳 */
#define _ECR_7E1_START_CARD_NO_INQUIRY_NO_	60	/* 啟動卡號查詢 */
#define _ECR_7E1_END_CARD_NO_INQUIRY_NO_	70	/* 結束卡號查詢 */
#define _ECR_7E1_INSTALLMENT_ADJUST_NO_		97	/* 分期調帳 */
#define _ECR_7E1_REDEEM_ADJUST_NO_		96	/* 紅利調帳 */
#define _ECR_7E1_HG_POINT_INQUIRY_NO_		65	/* 點數查詢 */
#define _ECR_7E1_HG_REWARD_SALE_NO_		81	/* 紅利積點 */
#define _ECR_7E1_HG_REWARD_INSTALLMENT_NO_	84	/* 紅利積點-分期 */
#define _ECR_7E1_HG_REWARD_REDEMPTION_NO_	85	/* 紅利積點-紅利 */
#define _ECR_7E1_HG_ONLINE_REDEEMPTION_NO_	86	/* 點數扣抵 */
#define _ECR_7E1_HG_POINT_CERTAIN_NO_		87	/* 加價購 */
#define _ECR_7E1_HG_FULL_REDEEMPTION_NO_	88	/* 點數兌換 */
#define _ECR_7E1_HG_REDEEM_REFUND_NO_		91	/* 扣抵退貨 */
#define _ECR_7E1_HG_REWARD_REFUND_NO_		92	/* 回饋退貨 */
#define _ECR_7E1_CASH_SALE_NO_			141	/* 預借現金 */

/* CUSTOMER_002 */
#define _ECR_7E1_CUSTOMER_002_INSTALLMENT_REFUND_NO_	6	/* 分期退貨 */
#define _ECR_7E1_CUSTOMER_002_REDEEM_REFUND_NO_		7	/* 紅利退貨 */
#define _ECR_7E1_CUSTOMER_002_INQUIRY_NO_		38	/* 查詢會員編號 */

/* CUSTOMER_027 */
#define _ECR_7E1_CUSTOMER_027_CHUNGHWA_TELECOM_TIP_NO_		40	/* 中華電信收銀機規格定義小費 */

/* CUSTOMER_035 */
#define _ECR_7E1_CUSTOMER_035_MIRAMAR_CINEMAS_SETTLEMENT_NO_	55

/* ECR_NCCC_HOSTID */
#define	_ECR_7E1_NCCC_HOSTID_NCCC_			"03"	/* U CARD、VISA、MASTER、JCB、CUP、SMART PAY*/
#define	_ECR_7E1_NCCC_HOSTID_DCC_			"04"	/* VISA、MASTER之外幣交易 */
#define	_ECR_7E1_NCCC_WELLCOME_HOSTID_DCC_		"05"    /* VISA、MASTER之外幣交易 */
#define	_ECR_7E1_NCCC_HOSTID_DINERS_			"00"	/* DINERS */
#define	_ECR_7E1_NCCC_HOSTID_HG_			"13"

#define	_ECR_7E1_NCCC_CUS027_CHUNGHWA_TELECOM_HOSTID_NCCC_	"08"	/* U CARD、VISA、MASTER、JCB、CUP、SMART PAY*/

#define	_ECR_7E1_NCCC_HOSTID_CUS_082_IKEA_CREDIT_	"01"	
#define	_ECR_7E1_NCCC_HOSTID_CUS_082_IKEA_REDEEM_	"02"
#define	_ECR_7E1_NCCC_HOSTID_CUS_082_IKEA_INST_		"03"
#define	_ECR_7E1_NCCC_HOSTID_CUS_082_IKEA_CUP_		"04"
#define	_ECR_7E1_NCCC_HOSTID_CUS_082_IKEA_HG_		"13"
#define	_ECR_7E1_NCCC_HOSTID_CUS_082_IKEA_AMEX_		"11"

#define	_ECR_7E1_NCCC_HOSTID_CUS_026_DCC_		"05"
#define	_ECR_7E1_NCCC_HOSTID_CUS_026_TAKA_		"04"

#define	_ECR_7E1_NCCC_HOSTID_CUS_026_DCC_		"05"
#define	_ECR_7E1_NCCC_HOSTID_CUS_026_TAKA_		"04"

#define _ECR_LOAD_TMK_FROM_520_Standard_Data_Size_	782	/* (2 + 15 * (2 + 2 + 48))*/

/* For ECR Transaction Response Code */
/* 錯誤碼版本00 */
#define _ECR_RESPONSE_CODE_NOT_SET_ERROR_		0	/* 沒設定ECR錯誤碼 */
#define _ECR_RESPONSE_CODE_ERROR_			1	/* 交易失敗 / 主機拒絕或卡片拒絕 */
#define _ECR_RESPONSE_CODE_CALLBANK_			2	/* 請連絡銀行/電子錢包業者 */
#define _ECR_RESPONSE_CODE_TIMEOUT_			3	/* 交易逾時 */
#define _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_		4	/* 操作錯誤 */
#define _ECR_RESPONSE_CODE_COMM_ERROR_			5	/* 通訊失敗 */
#define _ECR_RESPONSE_CODE_USER_TERMINATE_ERROR_	6	/* 使用者終止交易 */
#define _ECR_RESPONSE_CODE_READER_CARD_ERROR_		7	/* 讀卡失敗 */
#define _ECR_RESPONSE_CODE_END_INQUIRY_SUCCESS_         8
#define _ECR_RESPONSE_CODE_BYPASS_			9
#define _ECR_RESPONSE_CODE_NOT_MEMBER_CARD_		10
#define _ECR_RESPONSE_CODE_VOID_CHECK_ORG_AMT_ERROR_	11
#define _ECR_RESPONSE_CODE_CTLS_HOST_CANCEL_		12
#define _ECR_RESPONSE_CODE_CTLS_TWO_TAP_		13
#define _ECR_RESPONSE_CODE_SETTLEMENT_ERROR_		14
#define _ECR_RESPONSE_CODE_PIN_BYPASS_			15
#define _ECR_RESPONSE_CODE_AMOUNT_ERROR_		16      /* SKM - 金額檢核錯誤 */
#define _ECR_RESPONSE_CODE_OTHER_ERROR_			19
#define _ECR_RESPONSE_CODE_SMARTPAY_ERROR_		20	/* 20151014浩瑋新增 *//* 假error，只是標明是 Smartpay */
#define _ECR_RESPONSE_CODE_CTLS_ERROR_			21	
#define _ECR_RESPONSE_CODE_SELF_TRANS_SETTLE_ERROR_	22
#define _ECR_RESPONSE_CODE_ESVC_OVER_LIMIT_		23	/* 電票金額超過上限 */
#define _ECR_RESPONSE_CODE_ESVC_LOCK_CARD_		24	/* 票卡已鎖 */
#define _ECR_RESPONSE_CODE_ESVC_CANT_FIND_ORG_TRAS_	25	/* 無法比對原始電票交易 */
#define _ECR_RESPONSE_CODE_ESVC_UNEBLE_AUTO_TOP_UP_	26	/* 票卡未開啟自動加值 */
#define _ECR_RESPONSE_CODE_ESVC_INSUFFICIENT_BALANCE_	27	/* 電票餘額不足 */
#define _ECR_RESPONSE_CODE_MULTI_CARD_			28	/* 多張票卡讀取失敗 */
#define _ECR_RESPONSE_CODE_ESVC_RE_TAP_CARD_		29	/* 請重新感應卡片/重新掃碼/重試交易 */
#define _ECR_RESPONSE_CODE_VOID_INVOICE_ERROR_		30	/* 無法調閱原交易，請改用退貨 */
#define _ECR_RESPONSE_CODE_UNY_BARCODE_DATA_ERROR_	31	/* 支付條碼查核錯誤，請重新取得條碼 */
#define _ECR_RESPONSE_CODE_INDICATOR_ERROR_             32      /* Indicator errer */
#define _ECR_RESPONSE_CODE_PACKET_SAME_ERROR_           33	/* 重複封包 */
#define _ECR_RESPONSE_CODE_TOTAL_PACKET_ERROR_          34	/* 封包異常 */
#define _ECR_RESPONSE_CODE_CURRENT_PACKET_ERROR_        35	/* 封包遺失 */
#define _ECR_RESPONSE_CODE_PACKET_SIZE_ERROR_           36	/* 大小不符 */
#define _ECR_RESPONSE_CODE_TEXT_SIZE_ERROR_             37	/* 大小不符 */
#define _ECR_RESPONSE_CODE_QR_SIZE_ERROR_               38	/* 大小不符 */
#define _ECR_RESPONSE_CODE_39_SIZE_ERROR_               39	/* 大小不符 */
#define _ECR_RESPONSE_CODE_QR_DATA_ERROR_               40	/* 資料異常 */
#define _ECR_RESPONSE_CODE_39_DATA_ERROR_               41	/* 資料異常 */
#define _ECR_RESPONSE_CODE_END_ERROR_                   42	/* 資料異常 */
#define _ECR_RESPONSE_CODE_NOT_EVERRICH_CO_BRAND_CARD_	43	/* 非昇恆昌聯名卡錯誤 */
#define _ECR_RESPONSE_CODE_CHUNGHWA_TELECOM_IDENTIFY_CODE_	44	/* 中華電信客製化，識別碼錯誤 */
#define _ECR_RESPONSE_CODE_CARD_TYPR_NOT_MATCHED		45  /* 卡別不合 (客製化005專用)*/
#define _ECR_RESPONSE_CODE_EMV_ERROR				46  /* EMV 交易拒絕(客製化005專用) */
#define _ECR_RESPONSE_CODE_MAGENTIC_STRIPE_ERROR		47  /* 磁條交易拒絕(客製化005專用) */
#define _ECR_RESPONSE_CODE_BALANCE_NOT_ENOUGH			48  /* 餘額不足(客製化005專用) */
#define _ECR_RESPONSE_CODE_CARD_NUMBER_NOT_MATCH		49  /* 卡號不一致(客製化005專用) */
#define _ECR_RESPONSE_CODE_UPDATE_CARD_FAIL			50  /* 更正卡片資料失敗(客製化005專用) */
#define _ECR_RESPONSE_CODE_RECORD_NOT_FOUND			51 /* 取消資料不符(客製化005專用) */
#define _ECR_RESPONSE_CODE_READ_CARD_ERROR			52 /* 讀卡失敗 (Fall Back)(客製化005專用) */
#define _ECR_RESPONSE_CODE_CARD_NUMBER_ERROR			53 /* 卡號錯誤 or 過期(客製化005專用) */
#define _ECR_RESPONSE_CODE_HG_REDEEM_POINT_ERROR		54 /* 兌換點數不足(客製化005專用) */
#define	_ECR_RESPONSE_CODE_FORMAT_CHECK_ERROR_		55	/* 電文錯誤/格式錯誤 */
#define _ECR_RESPONSE_CODE_EW_HOST_TIMEOUT_		56	/* 電子錢包Timeout */
#define _ECR_RESPONSE_CODE_EW_INSUFFICIENT_BALANCE_	57	/* 電子錢包餘額不足 */
#define _ECR_RESPONSE_CODE_COUPON_USED_			58	/* 優惠券已被使用 */
#define _ECR_RESPONSE_CODE_COUPON_NOT_EXIST_		59	/* 優惠券不存在 */
#define _ECR_RESPONSE_CODE_NOT_MEET_RULE_		60	/* 不符合優惠規則 */
#define _ECR_RESPONSE_CODE_OFFER_ENDED_			61	/* 優惠活動已結束 */
#define _ECR_RESPONSE_CODE_COUPON_EXPIRED_		62	/* 優惠券已過期 */
#define _ECR_RESPONSE_CODE_NOT_ENTER_PATIENTID_		63	/* 未帶病患或入住者ID */
#define _ECR_RESPONSE_CODE_HOST_OK_                     98	/* Host Match */
#define _ECR_RESPONSE_CODE_SUCCESS_			99	/* 成功 */

/* 交易別 */
#define _ECR_8N1_TSB_KIOSK_SALE_MULTI_			"01"
#define _ECR_8N1_TSB_KIOSK_SALE_SINGLE_			"11"
#define _ECR_8N1_TSB_KIOSK_REFUND_			"02"	/* 目前不開放 */
#define _ECR_8N1_TSB_KIOSK_OFFLINE_			"03"	/* 目前不開放 */
#define _ECR_8N1_TSB_KIOSK_INSTALLMENT_			"04"
#define _ECR_8N1_TSB_KIOSK_REDEEM_			"05"
#define _ECR_8N1_TSB_KIOSK_VOID_			"30"
#define _ECR_8N1_TSB_KIOSK_SETTLEMENT_			"50"
#define _ECR_8N1_TSB_KIOSK_INQUIRE_TRANSACTION_STAUS_	"62"
#define _ECR_8N1_TSB_KIOSK_REPRINT_RECEIPT_		"63"
#define _ECR_8N1_TSB_KIOSK_SALE_FALLBACK_		"91"
#define _ECR_8N1_TSB_KIOSK_INSTALLMENT_FALLBACK_	"94"
#define _ECR_8N1_TSB_KIOSK_REDEEM_FALLBACK_		"95"


#define _ECR_8N1_TSB_KIOSK_SALE_MULTI_NO_			1	/* 一般交易(多合一)		*/
#define _ECR_8N1_TSB_KIOSK_SALE_SINGLE_NO_			11	/* 一般交易(僅只有一般交易)	*/
#define _ECR_8N1_TSB_KIOSK_REFUND_NO_				2	/* 退貨		目前不開放	*/
#define _ECR_8N1_TSB_KIOSK_OFFLINE_NO_				3	/* 交易補登	目前不開放	*/
#define _ECR_8N1_TSB_KIOSK_INSTALLMENT_NO_			4	/* 分期付款			*/
#define _ECR_8N1_TSB_KIOSK_REDEEM_NO_				5	/* 紅利扣抵			*/
#define _ECR_8N1_TSB_KIOSK_VOID_NO_				30	/* 取消				*/
#define _ECR_8N1_TSB_KIOSK_SETTLEMENT_NO_			50	/* 結帳				*/
#define _ECR_8N1_TSB_KIOSK_INQUIRE_TRANSACTION_STAUS_NO_	62	/* 詢問當筆交易狀態		*/
#define _ECR_8N1_TSB_KIOSK_REPRINT_RECEIPT_NO_			63	/* 重印簽單			*/
#define _ECR_8N1_TSB_KIOSK_SALE_FALLBACK_NO_			91	/* 銷售(Fall Back)		*/
#define _ECR_8N1_TSB_KIOSK_INSTALLMENT_FALLBACK_NO_		94	/* 分期付款(Fall Back)		*/
#define _ECR_8N1_TSB_KIOSK_REDEEM_FALLBACK_NO_			95	/* 紅利抵用(Fall Back)		*/

/* ECR_NCCC_HOSTID */
#define	_ECR_8N1_TSB_KIOSK_HOSTID_TSB_				"03"	/* U CARD、VISA、MASTER、JCB、CUP */
#define	_ECR_8N1_TSB_KIOSK_HOSTID_INST_				"04"	/* 分期 */
#define	_ECR_8N1_TSB_KIOSK_HOSTID_REDEEM_			"05"	/* 紅利 */
#define	_ECR_8N1_TSB_KIOSK_HOSTID_SMARTPAY_			"06"	/* Samrtpay */
#define	_ECR_8N1_TSB_KIOSK_HOSTID_DCC_				"07"	/* VISA、MASTER之外幣交易 */
#define	_ECR_8N1_TSB_KIOSK_HOSTID_DINERS_			"00"	/* DINERS */
#define	_ECR_8N1_TSB_KIOSK_HOSTID_AMEX_				"01"	/* AMEX */

/* ECR_NCCC_CARDTYPE */
#define	_ECR_8N1_TSB_KIOSK_NCCC_CARDTYPE_VISA_			"04"	/* VISA */
#define	_ECR_8N1_TSB_KIOSK_NCCC_CARDTYPE_MASTERCARD_		"05"	/* MASTERCARD */
#define	_ECR_8N1_TSB_KIOSK_NCCC_CARDTYPE_JCB_			"03"	/* JCB */
#define	_ECR_8N1_TSB_KIOSK_NCCC_CARDTYPE_SMARTPAY_		"09"	/* SMART PAY */
#define	_ECR_8N1_TSB_KIOSK_NCCC_CARDTYPE_UCARD_			"08"	/* U CARD*/
#define	_ECR_8N1_TSB_KIOSK_NCCC_CARDTYPE_AMEX_			"06"	/* AMEX */
#define	_ECR_8N1_TSB_KIOSK_NCCC_CARDTYPE_DINERS_		"07"	/* DINERS */
#define	_ECR_8N1_TSB_KIOSK_NCCC_CARDTYPE_CUP_			"01"	/* CUP */

#define	_ECR_LAST_SEND_FILENAME_				"ECRLASTSEND.txt"	/* POS組的電文 */
#define	_ECR_LAST_RECE_FILENAME_				"ECRLASTRECE.txt"	/* 回傳給POS的電文 */

/* HG Payment Tool Type */
#define _ECR_8N1_HG_PAYMENT_TOOL_TYPE_EDC_CHOOSE_	"00"
#define _ECR_8N1_HG_PAYMENT_TOOL_TYPE_CREDIT_CARD_	"01"
#define _ECR_8N1_HG_PAYMENT_TOOL_TYPE_CASH_		"02"
#define _ECR_8N1_HG_PAYMENT_TOOL_TYPE_GIFT_PAPER_	"03"
#define _ECR_8N1_HG_PAYMENT_TOOL_TYPE_HGI_		"04"
#define _ECR_8N1_HG_PAYMENT_TOOL_TYPE_CUP_CARD_		"05"

/* 麥當勞使用的Mirror Message */
#define _MIRROR_MSG_GET_CARD_			1	/* 請在刷卡機上操作刷卡、插卡或感應卡片 */
#define _MIRROR_MSG_GET_CARD_REFUND_		2	/* 請在刷卡機上操作刷卡或感應卡片 */
#define _MIRROR_MSG_GET_CARD_REDEEM_REFUND_	3	/* 請在刷卡機上操作刷卡 */
#define _MIRROR_MSG_GET_CARD_REDEEM_		4	/* 請在刷卡機上操作刷卡或插卡 */
#define _MIRROR_MSG_ENTER_PW_			5	/* 請顧客在刷卡機上輸入密碼 */
#define _MIRROR_MSG_CONNECT_HOST_		6	/* 刷卡機與主機連線中 */
#define _MIRROR_MSG_GET_CARD_RETRY_		7	/* 刷卡失敗，請再操作一次 */
#define _MIRROR_MSG_NOT_USE_IC_CARD_		8	/* 本交易不接受晶片卡，請重試交易！請按刷卡機清除鍵 */
#define _MIRROR_MSG_DECLINED_			9	/* 拒絕交易，請按刷卡機清除鍵 */
#define _MIRROR_MSG_CTLS_ERROR_			10	/* 感應失敗，請按刷卡機清除鍵 */
#define _MIRROR_MSG_CARD_NO_ERROR_		11	/* 卡號錯誤，請按刷卡機清除鍵 */
#define _MIRROR_MSG_WRONG_CARD_ERROR_		12	/* 請依正確卡別，選擇功能鍵操作，請按刷卡機清除鍵 */
#define _MIRROR_MSG_CHECK_CARD_ERROR_		13	/* 檢核錯誤，請按刷卡機清除鍵 */
#define _MIRROR_MSG_NOT_SUP_CARD_		14	/* 不支持此卡，請按刷卡機清除鍵 */
#define _MIRROR_MSG_AMOUNT_ERROR_		15	/* 金額錯誤，請按刷卡機清除鍵 */
#define _MIRROR_MSG_OPER_ERROR_			16	/* 操作錯誤，請按刷卡機清除鍵 */
#define _MIRROR_MSG_PLS_CLEAR_KEY_		17	/* 請按刷卡機清除鍵 */
#define _MIRROR_MSG_COMM_ERROR_			18	/* 主機連線失敗，請按刷卡機清除鍵 */
#define _MIRROR_MSG_ISO_ERROR_			19	/* 交易電文錯誤，請按刷卡機清除鍵 */
#define _MIRROR_MSG_EXP_CARD_ERROR_		20	/* 卡片有效期錯誤，請按刷卡機清除鍵 */
#define _MIRROR_MSG_EMV_USE_CHIP_ERROR_		21	/* 請改讀晶片卡，請按刷卡機清除鍵 */
#define _MIRROR_MSG_ESVC_GET_CARD_		22	/* 請在刷卡機上感應電票卡片 */
#define _MIRROR_MSG_ESVC_GET_CARD_RETRY_	23	/* 電票感應失敗，請再操作一次 */
#define _MIRROR_MSG_ESVC_KEEP_CARD_		24	/* 請勿移動票卡 */
#define _MIRROR_MSG_ESVC_TAP_AGAIN_		25	/* 請重新感應卡片 */
#define _MIRROR_MSG_8N1_Standard_OPER_ERR_                  26       /* 操作錯誤(0004) *//* 標準版使用Mirror Message *//* 前25個為麥當勞客製化專用，理論上要判斷客製化再傳進來*/
#define _MIRROR_MSG_8N1_Standard_PLS_ENTER_PATIENT_ID_      27       /* 請於EDC輸入病患或入住者ID */
#define _MIRROR_MSG_8N1_Standard_NOT_ENTER_PATIENT_ID_      28       /* 未帶病患或入住者ID */


/* 096義大一卡通直連交易別 */
#define _F_INIT_        "01"
#define _F_QUERY_       "11"
#define _F_DEDUCT_      "21"
#define _F_ADD_         "31"
#define _F_VOID_ADD_    "41"
#define _F_REGI_        "71"

#define _S_INIT_        "02"
#define _S_QUERY_       "12"
#define _S_DEDUCT_      "22"
#define _S_ADD_         "32"
#define _S_VOID_ADD_    "42"
#define _S_END_         "52"
#define _S_LOCK_        "62"

#define _ECR_IPASS_CUS096_ERR_NO_	0
#define _ECR_IPASS_CUS096_ERR_TIMEOUT_	1
#define _ECR_IPASS_CUS096_ERR_CANCEL_	2

/* 台塑生醫生技客製化專屬【005】　台亞福懋加油站客製化專屬【006】 (START) */
#define _ECR_8N1_FPG_FTC_HG_REDEEM_			"07" /* HG 點數扣抵【信用卡、現金】 */
#define _ECR_8N1_FPG_FTC_HGI_ONLINE_REDEEM_		"08" /* HGI 點數扣抵【HGI 信用卡】 */
#define _ECR_8N1_FPG_FTC_HG_POINT_CERTAIN_		"09" /* HG 加價購【信用卡、現金】 */
//#define _ECR_8N1_FPG_FTC_HGI_POINT_CERTAIN_		"10" /* HGI 加價購 */
#define _ECR_8N1_FPG_FTC_INSTALLMENT_REFUND_		"22" /* 分期付款退貨 */
#define _ECR_8N1_FPG_FTC_HG_REDEEM_REFUND_		"27" /* HAPPY GO 扣抵退貨【點數】 */
#define _ECR_8N1_FPG_FTC_HG_POINT_REDEEM_		"29" /* HAPPY GO 點數兌換【HG 商品券兌換】 */
#define _ECR_8N1_FPG_FTC_REDEMPTION_REFUND_		"32" /* 紅利扣抵退貨 */
//#define _ECR_8N1_FPG_FTC_STORE_VALUE_			"53" /* 聯名卡儲值金加值【現金】 */
//#define _ECR_8N1_FPG_FTC_SVC_SALE_			"55" /* 聯名卡儲值金消費扣款 */
//#define _ECR_8N1_FPG_FTC_SVC_REFUND_			"56" /* 聯名卡儲值金消費退貨 */
#define _ECR_8N1_FPG_FTC_EXCHANGE_COUPON_		"62" /* 聯名卡商品券兌換 */
#define _ECR_8N1_FPG_FTC_REPRINT_RECEIPT_		"63" /* 重印交易簽單 */
#define _ECR_8N1_FPG_FTC_REPRINT_COUPON_		"64" /* 重印商品券簽單【FPG & HG】 */
#define _ECR_8N1_FPG_FTC_VERSION_CHECK_			"98" /* 版本檢查 */
#define _ECR_8N1_FPG_FTC_TRANS_TERMINATE_		"99" /* 交易終止【不回覆POS】*/

#define _ECR_8N1_FPG_FTC_HG_REDEEM_NO_			7  /* HG 點數扣抵【信用卡、現金】 */
#define _ECR_8N1_FPG_FTC_HGI_ONLINE_REDEEM_NO_		8  /* HGI 點數扣抵【HGI 信用卡】 */
#define _ECR_8N1_FPG_FTC_HG_POINT_CERTAIN_NO_		9  /* HG 加價購【信用卡、現金】 */
//#define _ECR_8N1_FPG_FTC_HGI_POINT_CERTAIN_NO_	10 /* HGI 加價購 */
#define _ECR_8N1_FPG_FTC_INSTALLMENT_REFUND_NO_		22 /* 分期付款退貨 */
#define _ECR_8N1_FPG_FTC_HG_REDEEM_REFUND_NO_		27 /* HAPPY GO 扣抵退貨【點數】 */
#define _ECR_8N1_FPG_FTC_HG_POINT_REDEEM_NO_		29 /* HAPPY GO 點數兌換【HG 商品券兌換】 */
#define _ECR_8N1_FPG_FTC_REDEMPTION_REFUND_NO_		32 /* 紅利扣抵退貨 */
//#define _ECR_8N1_FPG_FTC_STORE_VALUE_NO_		53 /* 聯名卡儲值金加值【現金】 */
//#define _ECR_8N1_FPG_FTC_SVC_SALE_NO_			55 /* 聯名卡儲值金消費扣款 */
//#define _ECR_8N1_FPG_FTC_SVC_REFUND_NO_		56 /* 聯名卡儲值金消費退貨 */
#define _ECR_8N1_FPG_FTC_EXCHANGE_COUPON_NO_		62 /* 聯名卡商品券兌換 */
#define _ECR_8N1_FPG_FTC_REPRINT_RECEIPT_NO_		63 /* 重印交易簽單 */
#define _ECR_8N1_FPG_FTC_REPRINT_COUPON_NO_		64 /* 重印商品券簽單【FPG & HG】 */
#define _ECR_8N1_FPG_FTC_VERSION_CHECK_NO_		98 /* 版本檢查 */
#define _ECR_8N1_FPG_FTC_TRANS_TERMINATE_NO_		99 /* 交易終止【不回覆POS】*/
/* 台塑生醫生技客製化專屬【005】　台亞福懋加油站客製化專屬【006】 (END) */

#define _ECR_EW_INDICATOR_	"W"
#define _ECR_TRUST_INDICATOR_	"T"

int inECR_Initial(void);
int inECR_FlushTxBuffer(void);
int inECR_FlushRxBuffer(void);
int inECR_Send_Check(void);
int inECR_Receive_Check(unsigned short* usLen);
int inECR_Receive_Transaction(TRANSACTION_OBJECT *pobTran);
int inECR_Receive_Second_Transaction(TRANSACTION_OBJECT *pobTran);
int inECR_Send_Transaction_Result(TRANSACTION_OBJECT *pobTran);
int inECR_Send_Inquiry_Result(TRANSACTION_OBJECT *pobTran);
int inECR_Send_Transaction(TRANSACTION_OBJECT *pobTran);
int inECR_SendError(TRANSACTION_OBJECT * pobTran, int inErrorType);
int inECR_SendMirror(TRANSACTION_OBJECT * pobTran, int inErrorType);
int inECR_ReceiveEI(TRANSACTION_OBJECT * pobTran);
int inECR_SendEI(TRANSACTION_OBJECT * pobTran, int inErrorType);
int inECR_DeInitial(void);
int inECR_NCCC_144_To_400(ECR_TABLE* srECROb, unsigned char* uszReceiveBuffer);
int inECR_Print_Receive_ISODeBug(char *szDataBuffer, unsigned short usReceiveBufferSize, int inDataSize);
int inECR_Print_Send_ISODeBug(char *szDataBuffer, unsigned short usReceiveBufferSize, int inDataSize);
int inECR_CardNoTruncateDecision(TRANSACTION_OBJECT * pobTran);
int inECR_CardNoTruncateDecision_HG(TRANSACTION_OBJECT * pobTran);
int inECR_Customer_Flow(TRANSACTION_OBJECT* pobTran);
int inECR_Save_Request(char* szBuffer, int inBufferLen);
int inECR_Save_Response(char* szBuffer, int inBufferLen);
int inECR_Inquiry_Last_Transction(TRANSACTION_OBJECT * pobTran, ECR_TABLE *srECROb, char *szBuffer, int inBufferLen);
int inECR_No_TRT_Return_Flow(TRANSACTION_OBJECT * pobTran);
int inECR_Load_TMK_Initial(void);
int inECR_Load_TMK(TRANSACTION_OBJECT *pobTran);
int inECR_Send_TMK(TRANSACTION_OBJECT *pobTran);
int inECR_Load_TMK_DeInitial(void);
int inECR_Check_Exception(TRANSACTION_OBJECT *pobTran);
int inECR_Transform_8N1_No_To_String(int inNo, char *szTransType);
int inECR_SelectTransType(ECR_TABLE * srECROb);
int inECR_SelectOtherCardType(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inECR_SelectAwardRedeemMode(ECR_TABLE * srECROb, char *szAwardType);
int inECR_Choose_Way_Flow(TRANSACTION_OBJECT *pobTran);
int inECR_PackEsgData(TRANSACTION_OBJECT *pobTran, int *inPackLen, char *szPackData);

/* EI */
int inECR_EI_Flow(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);
int inECR_EI_Unpack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer);
int inECR_EI_Unpack_Data(long lnSize, char *szData);
int inECR_8N1_EInvoice_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);

/* 整合function */
int inECR_Receive(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer, int inDataSize);
int inECR_Receive_UDP(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer, int inDataSize);
int inECR_Not_Check_Sizes_Receive(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer, int inDataSize);
int inECR_Send(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer, int inDataSize);
int inECR_Send_NotACK(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer, int inDataSize);
int inECR_Data_Receive(unsigned char *uszReceBuff, unsigned short *usReceSize);
int inECR_Send_ACKorNAK(ECR_TABLE * srECROb, int inAckorNak);

int inECR_8N1_Standard_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer);
int inECR_8N1_Standard_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);
int inECR_8N1_Standard_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf);
int inECR_8N1_Standard_Pack_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);
int inECR_8N1_Standard_Parse_Data(ECR_TABLE* srECROb, char *szDataBuffer);
int inECR_8N1_Standard_Mirror_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);

int inECR_7E1_Standard_Unpack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Standard_Pack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Standard_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf);
int inECR_7E1_Standard_Pack_Error(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Standard_Parse_Data(ECR_TABLE* srECROb, char *szDataBuffer);

int inECR_LOAD_TMK_FROM_520_Standard_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer);
int inECR_LOAD_TMK_FROM_520_Standard_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);

int inECR_8N1_TSB_KIOSK_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer);
int inECR_8N1_TSB_KIOSK_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);
int inECR_8N1_TSB_KIOSK_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf);

int inECR_8N1_Customer_107_Bumper_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer);
int inECR_8N1_Customer_Self_Trans_Settle_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);
int inECR_8N1_Inquiry_Last_Transaction_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);
int inECR_8N1_Customer_107_Bumper_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf);
int inECR_8N1_Customer_111_Kiosk_Standard_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer);
int inECR_8N1_Customer_039_SKYKAND_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);
int inECR_8N1_Customer_039_SKYKAND_Pack_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);

int inECR_8N1_Customer_098_Mcdonalds_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer);
int inECR_8N1_Customer_098_Mcdonalds_Remove_Card_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer);
int inECR_8N1_Customer_098_Mcdonalds_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf);
int inECR_8N1_Customer_098_Mcdonalds_Mirror_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);
int inECR_8N1_Customer_098_Mcdonalds_Pack_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);
int inECR_8N1_Customer_098_Mcdonalds_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);
int inECR_8N1_Customer_098_Mcdonalds_Parse_Data(ECR_TABLE* srECROb, char *szDataBuffer);
int inECR_098_Inquiry_Last_Transction(TRANSACTION_OBJECT * pobTran, ECR_TABLE *srECROb, char *szBuffer, int inBufferLen);

int inECR_8N1_Customer_034_TK3C_EInvoice_Unpack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_8N1_Customer_034_TK3C_EInvoice_Pack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_8N1_Customer_034_TK3C_EInvoice_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf);
int inECR_8N1_Customer_034_TK3C_EInvoice_Pack_Error(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_8N1_Customer_034_TK3C_EInvoice_Parse_Data(ECR_TABLE* srECROb, char *szDataBuffer);
int inECR_8N1_Customer_034_TK3C_EInvoice_SelectTransType(TRANSACTION_OBJECT *pobTran);

int inECR_7E1_Customer_002_NICE_PLAZA_Unpack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_002_NICE_PLAZA_Pack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_002_NICE_PLAZA_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf);
int inECR_7E1_Customer_002_NICE_PLAZA_Pack_Error(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_002_NICE_PLAZA_Parse_Data(ECR_TABLE* srECROb, char *szDataBuffer);
int inECR_7E1_Customer_002_NICE_PLAZA_SelectTransType(ECR_TABLE * srECROb);

int inECR_7E1_Customer_003_WELLCOME_Unpack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_003_WELLCOME_Pack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_003_WELLCOME_Pack_Error(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_003_WELLCOME_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf);

int inECR_7E1_Customer_027_CHUNGHWA_TELECOM_Unpack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_027_CHUNGHWA_TELECOM_Pack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_027_CHUNGHWA_TELECOM_Pack_Error(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_027_CHUNGHWA_TELECOM_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf);
int inECR_7E1_Customer_027_CHUNGHWA_TELECOM_Parse_Data(ECR_TABLE* srECROb, char *szDataBuffer);

int inECR_8N1_Customer_033_UNIAIR_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer);
int inECR_8N1_Customer_033_UNIAIR_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);
int inECR_8N1_Customer_033_UNIAIR_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf);

int inECR_7E1_Customer_053_TAICHUNG_SOGO_Unpack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_053_TAICHUNG_SOGO_Pack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_053_TAICHUNG_SOGO_Pack_Error(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_053_TAICHUNG_SOGO_Parse_Data(ECR_TABLE* srECROb, char *szDataBuffer);

int inECR_7E1_Customer_035_MIRAMAR_CINEMAS_Unpack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_035_MIRAMAR_CINEMAS_Pack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_035_MIRAMAR_CINEMAS_Pack_Error(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_035_MIRAMAR_CINEMAS_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf);

int inECR_Receive_Cus_096(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer, int inDataSize);
int inECR_Cus_096_Ipass_Parse_Data(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer);
int inECR_Cus_096_Ipass_Transaction_Flow(TRANSACTION_OBJECT *pobTran);

int inECR_ReCheck_144_TRT_Flow(TRANSACTION_OBJECT *pobTran);

int inECR_7E1_Customer_079_CINEMARK_Unpack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_079_CINEMARK_SelectTransType(TRANSACTION_OBJECT *pobTran);
int inECR_7E1_Customer_079_CINEMARK_Pack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_079_CINEMARK_Pack_Error(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_079_CINEMARK_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf);

int inECR_7E1_Customer_082_IKEA_Unpack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_082_IKEA_Pack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_082_IKEA_Pack_Error(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_082_IKEA_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf);
int inECR_SelectTransType_Customer_082_IKEA(ECR_TABLE * srECROb);

int inECR_8N1_Customer_038_Bellavita_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer);

int inECR_8N1_Customer_005_FPG_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer);
int inECR_8N1_Customer_005_FPG_Parse_Data(ECR_TABLE* srECROb, char *szDataBuffer);
int inECR_8N1_Customer_005_FPG_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);
int inECR_8N1_Customer_005_FPG_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf);

int inECR_7E1_Customer_026_TAKA_Unpack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_026_TAKA_Pack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_026_TAKA_Pack_Error(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_026_TAKA_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf);

int inECR_7E1_Customer_021_TAKAWEL_Unpack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_021_TAKAWEL_Pack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_021_TAKAWEL_Pack_Error(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb);
int inECR_7E1_Customer_021_TAKAWEL_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf);

int inECR_8N1_Customer_090_TAIPEI_101_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);
int inECR_8N1_Customer_090_TAIPEI_101_Pack_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer);