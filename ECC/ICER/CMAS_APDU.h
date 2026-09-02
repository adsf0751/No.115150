#ifndef __CMAS_APDU_H__
#define __CMAS_APDU_H__

#ifdef __cplusplus
		 extern "C" {
#endif

#include "XMLFunc.h"

int inBuildCMSSignOnAPDU(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildCMSAuthTxnOnlineAPDU(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildCMSAuthTxnOfflineAPDU(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildCMSTaxiDeductCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildCMSAuthSetValueAPDU(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inCMSVerifyHCryptCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildCMASCBikeDeductCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildCMASEDCADeductCommand(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildICERAuthTxnOnlineAPDU(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);

#ifdef __cplusplus
		 }
#endif // __cplusplus

#endif
