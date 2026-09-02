#ifndef __SIS2_2_H__
#define __SIS2_2_H__

#ifdef __cplusplus
	extern "C" {
#endif

int inBuildSIS2Body_FareSale(BYTE *bOut_Data,BYTE *ucLoyaltyCounter,
										  BYTE *ucAgentNumber,
										  BYTE ucFareProductType,
										  BYTE ucGroupsize,
										  BYTE *ucEntryStation,
										  BYTE *ucExitStation,
										  BYTE *ucJourneyNumber,
										  BYTE *ucFirstDate,
										  BYTE *ucLastDate,
										  BYTE *ucCarNo,
										  BYTE *ucDuration,
										  BYTE *ucAuthoriseGroup,
										  BYTE *ucRFU,
										  BYTE ucTxnMode,
										  BYTE ucTxnQuqlifier,
										  BYTE ucSignatureKeyKVN,
										  BYTE *ucSignature);
int inBuildSIS2Body_VoidFare(BYTE *bOut_Data,BYTE *ucLoyaltyCounter,
										  BYTE *ucAgentNumber,
										  BYTE ucFareProductType,
										  BYTE *ucRFU,
										  BYTE ucTxnMode,
										  BYTE ucTxnQuqlifier,
										  BYTE ucSignatureKeyKVN,
										  BYTE *ucSignature);
int inBuildSIS2Body_RefundFare(BYTE *bOut_Data,BYTE *ucLoyaltyCounter,
										  BYTE ucFareProductType,
										  BYTE *ucRFU,
										  BYTE ucTxnMode,
										  BYTE ucTxnQuqlifier,
										  BYTE ucSignatureKeyKVN,
										  BYTE *ucSignature);
int inBuildSIS2Body_RefundCard(BYTE *bOut_Data,BYTE *ucAgentNumber,
											BYTE *ucDepost,
											BYTE *ucCardRefundFee,
											BYTE *ucCPUBrokenFee,
											BYTE *ucCardCustomerFee,
											BYTE *ucRefFee,
											BYTE *ucRFU,
											BYTE ucTxnMode,
											BYTE ucTxnQuqlifier,
											BYTE ucSignatureKeyKVN,
											BYTE *ucSignature);

int inBuildFareSaleSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildVoidFareSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildRefundFareSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildTRTCSetValueSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildSetValueOfflineSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildAuthTxnOnlineSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildAuthRefundCardSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildETxnReqOnlineSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildCBikeDeductSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildEDCADeductSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildICERETxnSIS2(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);

#ifdef __cplusplus
		}
#endif // __cplusplus

#endif

