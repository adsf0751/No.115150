#ifndef __CMAS_TM_H__
#define __CMAS_TM_H__

#ifdef __cplusplus
		 extern "C" {
#endif

void vdGetLocalIP(BYTE *bBuf);
void vdBuildCMASCardAVR(BYTE *bCardAVRData,int inCardAVRDataLen,STRUCT_XML_DOC *srXML,char *chTagName,int inTagLen,int inNodeType,BOOL fXMLSendFlag);
int inBuildCMSResetTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildCMSSignOnTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildCMSTxnReqOnlineTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildCMSAuthTxnOnlineTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildCMSTxnReqOfflineTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildCMSTaxiReadTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildCMSTaxiDeductTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
void vdSetPointDetail(int inTxnType,STRUCT_XML_DOC *srXML);
int inBuildCMASSettleTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildCMASLockTM1(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildCMASLockTM2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildCMSReadDongleDeductTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildCMSAuthTxnOfflineTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildCMSSetStudentValueTM(int inTxnType, STRUCT_XML_DOC * srXML, BYTE * API_Out, BYTE * Dongle_In, BYTE * Dongle_Out, BYTE * Dongle_ReqOut);
int inBuildCMSSetValueTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);

int inBuildICERTxnReqOnlineTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
void vdBuildICERAData(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *Dongle_In,BYTE *Dongle_Out);
void vdBuildICERADataAuth(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *Dongle_In,BYTE *DongleReq_Out,BYTE *Dongle_Out);
int inBuildICERQRTxnTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildCMSCBikeReadTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildCMSCBikeDeductTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildCMASMMSelectCard2Data(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildCMASEDCAReadTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildCMASEDCADeductTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildICERAuthTxnOnlineTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);

#ifdef __cplusplus
		 }
#endif // __cplusplus

#endif
