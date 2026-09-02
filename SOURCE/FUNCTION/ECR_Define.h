/* 
 * File:   ECR_Define.h
 * Author: RussellBai
 *
 * Created on 2022年8月12日, 下午 3:58
 */

#define _SOH_				0x01
#define _STX_				0x02
#define _ETX_				0x03
#define _EOT_				0x04
#define _ACK_				0x06
#define _NAK_				0x15

#define _ECR_RETRYTIMES_		3
#define _ECR_RECEIVE_TIMER_		_TIMER_NEXSYS_2_	/* 1 */
#define _ECR_RECEIVE_REQUEST_TIMEOUT_	10
#define _ECR_RECEIVE_EI_TIMEOUT_	5
#define _ECR_RECEIVE_CUS_096_IPASS_DIRECT_TIMEOUT_	35
#define _ECR_GET_CARD_TIMEOUT_		13			/* 根據端末設備交易流程使用者介面(UI)規格書 改為13秒 */
#define _ECR_SELECT_TIMEOUT_		20
#define _ECR_RECEIVE_ACK_TIMEOUT_LONG_		150			/* 避免POS收太慢，由1秒改為1.5秒(10ms) */
#define _ECR_RECEIVE_ACK_TIMEOUT_NORMAL_	100			/* (10ms) */
#define _ECR_RECEIVE_ACK_TIMEOUT_144_		200			/* 長度144用2秒 (10ms) */
#define _ECR_BUFF_SIZE_			5120 + 1
#define _ECR_UDP_DEFAULT_PORT_		50002
#define _ECR_UDP_HEADER_SIZE_		16
#define _ECR_RS232_STX_SIZE_		1
#define _ECR_RS232_ETX_SIZE_		1
#define _ECR_RS232_LRC_SIZE_		1
#define _ECR_UDP_SOH_SIZE_		1
#define _ECR_UDP_EOT_SIZE_		1
#define _ECR_UDP_PACKET_MAX_LIMIT_	5

#define _ECR_HOST_RESPONSE_STATUS_NONE_		0	/* 主機沒回 */
#define _ECR_HOST_RESPONSE_STATUS_DECLINE_	1	/* 主機有回錯誤碼 */

#define _ECR_8N1_1000_CUPQRCODE_Size_			1000
#define _ECR_8N1_Customer_034_TK3C_EInvoice_Data_Size_  144
#define _ECR_8N1_2044_EI_Receive_Size_                  2044
#define _ECR_8N1_100_EI_Send_Size_                      100
#define _ECR_IPASS_Receive_Size_			512

/* 8N1_Standard 長度 */
#define _ECR_8N1_Standard_Data_Size_			400
#define _ECR_8N1_CUSTOMER_005_FPG_Data_Size_		480
	
/* 7E1_Standard 長度 */
#define _ECR_7E1_Standard_Data_Size_			144
#define _ECR_7E1_CUSTOMER_002_NICE_PLAZA_Data_Size_	247
#define _ECR_7E1_CUSTOMER_079_CINEMARK_Data_Size_	98


/* 台塑生醫生技客製化專屬【005】　台亞福懋加油站客製化專屬【006】 (START) */
#define FPG_FTC_EFID_SIZE			2
#define FPG_FTC_MEMBER_ID_SIZE			10	/* FE01_會員編號 */
#define FPG_FTC_CARD_TYPE_SIZE			2       /* FE01_卡別 */
#define FPG_FTC_MEMBER_CODE_SIZE		2       /* FE01_身分代號 */
#define FPG_FTC_RANGE_SIZE			2       /* FE01_等級 */
#define FPG_FTC_INVOICE_NO_SIZE			10      /* FE01_發票列印統編 */
#define FPG_FTC_CARD_NUM_SIZE			10      /* FE01_車號 */
#define FPG_FTC_OIL_CODE_SIZE			2       /* FE01_油品代號 */
#define FPG_FTC_BASIC_DATA_SIZE			25      /* FE01_Basic_Data_1 */
#define FPG_FTC_FE02_CUSTOMER_NAME_SIZE		10      /* FE02_姓名 */
#define FPG_FTC_BIRTHDAY_SIZE			8       /* FE02_生日 */

#define SIGNTURE_SIZE				8
#define FPG_FTC_MAX_EFID_REC			5
#define FPG_FTC_CARD_NUM_SIZE 			10
#define FPG_FTC_ICV_SIZE			8
#define FETC_CSN_SIZE				8
#define FPG_FTC_PID_SIZE			8
