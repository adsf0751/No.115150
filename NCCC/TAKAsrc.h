/* 
 * File:   TAKAsrc.h
 * Author: RussellBai
 *
 * Created on 2022年10月14日, 上午 11:46
 */

#ifndef TAKASRC_H
#define	TAKASRC_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* TAKASRC_H */

#define _HARDCODE_STORE_ID_CUS_021_TAKAWEL_	"150002000000"

#define	_TAKA_COMM_PRESERVE_XML_FILE_NAME_	"TAKA_COMM_PRESERVE.xml"

#define _TAKA_COMM_PRESERVE_TAG_NEXSYS_			"NEXSYS"
#define _TAKA_COMM_PRESERVE_TAG_NEXSYS_LEN_		6
#define	_TAKA_COMM_PRESERVE_TAG_CommIndex_		"CommIndex"
#define _TAKA_COMM_PRESERVE_TAG_CommIndex_LEN_		9
#define	_TAKA_COMM_PRESERVE_TAG_TPDU_			"TPDU"
#define _TAKA_COMM_PRESERVE_TAG_TPDU_LEN_		4
#define	_TAKA_COMM_PRESERVE_TAG_NII_			"NII"
#define _TAKA_COMM_PRESERVE_TAG_NII_LEN_		3
#define	_TAKA_COMM_PRESERVE_TAG_HostTelPrimary_		"HostTelPrimary"
#define _TAKA_COMM_PRESERVE_TAG_HostTelPrimary_LEN_	14
#define	_TAKA_COMM_PRESERVE_TAG_HostTelSecond_		"HostTelSecond"
#define _TAKA_COMM_PRESERVE_TAG_HostTelSecond_LEN_	13
#define	_TAKA_COMM_PRESERVE_TAG_ReferralTel_		"ReferralTel"
#define _TAKA_COMM_PRESERVE_TAG_ReferralTel_LEN_	11
#define	_TAKA_COMM_PRESERVE_TAG_HostIPPrimary_		"HostIPPrimary"
#define _TAKA_COMM_PRESERVE_TAG_HostIPPrimaryLEN_	13
#define	_TAKA_COMM_PRESERVE_TAG_HostPortNoPrimary_	"HostPortNoPrimary"
#define _TAKA_COMM_PRESERVE_TAG_HostPortNoPrimary_LEN_	17
#define	_TAKA_COMM_PRESERVE_TAG_HostIPSecond_		"HostIPSecond"
#define _TAKA_COMM_PRESERVE_TAG_HostIPSecond_LEN_	12
#define	_TAKA_COMM_PRESERVE_TAG_HostPortNoSecond_	"HostPortNoSecond"
#define _TAKA_COMM_PRESERVE_TAG_HostPortNoSecond_LEN_	16
#define	_TAKA_COMM_PRESERVE_TAG_TCPHeadFormat_		"TCPHeadFormat"
#define _TAKA_COMM_PRESERVE_TAG_TCPHeadFormat_LEN_	13
#define	_TAKA_COMM_PRESERVE_TAG_CarrierTimeOut_		"CarrierTimeOut"
#define _TAKA_COMM_PRESERVE_TAG_CarrierTimeOut_LEN_	14
#define	_TAKA_COMM_PRESERVE_TAG_HostResponseTimeOut_		"HostResponseTimeOut"
#define _TAKA_COMM_PRESERVE_TAG_HostResponseTimeOut_LEN_	19

int inTAKA_Func_RunTRT(TRANSACTION_OBJECT *pobTran, int inTRTCode);
int inTAKA_Func_Check_Transaction_Function(int inCode);
int inTAKA_Func_SwitchToTAKA_Host(int inOrgHDTIndex);
int inTAKA_Func_Check_Transaction_Function_Flow(TRANSACTION_OBJECT *pobTran);
int inTAKA_Func_Must_SETTLE(TRANSACTION_OBJECT *pobTran);
int inTAKA_Func_Find_Comm_Index(int inOrgHDTIndex, int *inCommIndex);
int inTAKA_TAKA_Comm_Preserve(void);
int inTAKA_TAKA_Comm_Get_In_Record(void);