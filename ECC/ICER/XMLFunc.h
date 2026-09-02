#ifndef __XMLFUNC_H__
#define __XMLFUNC_H__

#ifdef __cplusplus
	extern "C" {
#endif

#define MAX_TEXT_LEN			30
#define MAX_XML_ELEMENT_CNT		400

typedef struct
{
	BYTE fNodeFlag;
	BYTE fXMLSendFlag	:1;
	BYTE fNoResponseTM	:1;
	BYTE fRFU			:6;
	int inNodeNumber;
	short shTagLen;
	short shValueLen;
	char *chTag;
	char *chValue;
}STRUCT_XML_ELEMENT;

typedef struct
{
	int inXMLElementCnt;
	STRUCT_XML_ELEMENT srXMLElement[MAX_XML_ELEMENT_CNT];
}STRUCT_XML_DOC;

#define NODE_NO_SAME		0
#define NODE_NO_ADD			1
#define NODE_NO_DEC			2

typedef struct
{
	int inNodeNoType;
	char chTagName[30];
	char *chValue;
	BOOL fNodeFlag;
	BOOL fOK;
	BYTE fXMLSendFlag	:1;
	BYTE fNoResponseTM	:1;
	BYTE fRFU			:6;
}STRUCT_XML_DETAIL;

typedef struct
{
	int inParseCnt;
	STRUCT_XML_DETAIL srXMLDetail[20];
}STRUCT_XML_ACTION_DATA;

#define MAX_XML_CHECK_CNT		20

typedef struct
{
	char chTagName[MAX_XML_CHECK_CNT][10];
}STRUCT_XML_CHECK_DATA;

typedef struct
{
	long lnTag;
	int inXMLCheckType;
}STRUCT_XML_FORMAT_CHECK;

#define TAG_NAME_START		0
#define VALUE_NAME			1
#define TAG_NAME_END		2

#define TAG_TRANS_XML_HEADER	"TransXML"
#define TAG_TRANS_XML_END	"/TransXML"
#define TAG_TRANS_HEADER	"TRANS"
#define TAG_TRANS_END		"/TRANS"
#define TAG_NAME_0100		"T0100"
#define TAG_NAME_0200		"T0200"
#define TAG_NAME_0206		"T0206"
#define TAG_NAME_0211		"T0211"
#define TAG_NAME_0212		"T0212"
#define TAG_NAME_0213		"T0213"
#define TAG_NAME_0214		"T0214"
#define TAG_NAME_0215		"T0215"
#define TAG_NAME_0216		"T0216"
#define TAG_NAME_0217		"T0217"
#define TAG_NAME_0218		"T0218"
#define TAG_NAME_0221		"T0221"
#define TAG_NAME_0222		"T0222"
#define TAG_NAME_0223		"T0223"
#define TAG_NAME_0231		"T0231"
#define TAG_NAME_0232		"T0232"
#define TAG_NAME_0220		"T0220"
#define TAG_NAME_0300		"T0300"
#define TAG_NAME_0400		"T0400"
#define TAG_NAME_0401		"T0401"
#define TAG_NAME_0403		"T0403"
#define TAG_NAME_0404		"T0404"
#define TAG_NAME_0406		"T0406"
#define TAG_NAME_0407		"T0407"
#define TAG_NAME_0408		"T0408"
#define TAG_NAME_0409		"T0409"
#define TAG_NAME_0410		"T0410"
#define TAG_NAME_0411		"T0411"
//#define TAG_NAME_0412		"T0412"
//#define TAG_NAME_0414		"T0414"
#define TAG_NAME_0415 		"T0415"
#define TAG_NAME_0416		"T0416"
#define TAG_NAME_0417		"T0417"
#define TAG_NAME_0418		"T0418"
#define TAG_NAME_0437		"T0437"
#define TAG_NAME_0438		"T0438"
#define TAG_NAME_0439		"T0439"
#define TAG_NAME_0440		"T0440"
#define TAG_NAME_0442		"T0442"
#define TAG_NAME_0443		"T0443"
#define TAG_NAME_0444		"T0444"
#define TAG_NAME_0445		"T0445"
#define TAG_NAME_0446		"T0446"
#define TAG_NAME_1100		"T1100"
#define TAG_NAME_1101		"T1101"
#define TAG_NAME_1102		"T1102"
#define TAG_NAME_1103		"T1103"
#define TAG_NAME_1107		"T1107"
#define TAG_NAME_1200		"T1200"
#define TAG_NAME_1300		"T1300"
#define TAG_NAME_1400		"T1400"
#define TAG_NAME_1402		"T1402"
#define TAG_NAME_1403		"T1403"
#define TAG_NAME_1404		"T1404"
#define TAG_NAME_3700		"T3700"
#define TAG_NAME_3900		"T3900"
#define TAG_NAME_3908		"T3908"
#define TAG_NAME_4100		"T4100"
#define TAG_NAME_4101		"T4101"
#define TAG_NAME_4103		"T4103"
#define TAG_NAME_4104		"T4104"
#define TAG_NAME_4105		"T4105"
#define TAG_NAME_4107		"T4107"
#define TAG_NAME_4108		"T4108"
#define TAG_NAME_4109		"T4109"
#define TAG_NAME_4110		"T4110"
#define TAG_NAME_4117		"T4117"
#define TAG_NAME_4200		"T4200"
#define TAG_NAME_4213		"T4213"
#define TAG_NAME_4214		"T4214"
#define TAG_NAME_4800		"T4800"
#define TAG_NAME_4803		"T4803"
#define TAG_NAME_4804		"T4804"
#define TAG_NAME_4805		"T4805"
#define TAG_NAME_4806		"T4806"
#define TAG_NAME_4807		"T4807"
#define TAG_NAME_4808		"T4808"
#define TAG_NAME_4814		"T4814"
#define TAG_NAME_4818		"T4818"
#define TAG_NAME_4830		"T4830"
#define TAG_NAME_4831		"T4831"
#define TAG_NAME_483100		"T483100"
#define TAG_NAME_483101		"T483101"
#define TAG_NAME_483102		"T483102"
#define TAG_NAME_483103		"T483103"
#define TAG_NAME_483104		"T483104"
#define TAG_NAME_483105		"T483105"
#define TAG_NAME_483106		"T483106"
#define TAG_NAME_483107		"T483107"
#define TAG_NAME_483108		"T483108"
#define TAG_NAME_483109		"T483109"
#define TAG_NAME_483110		"T483110"
#define TAG_NAME_483111		"T483111"
#define TAG_NAME_4832	 	"T4832"
#define TAG_NAME_4833	 	"T4833"
#define TAG_NAME_4834	 	"T4834"
#define TAG_NAME_4835	 	"T4835"
#define TAG_NAME_4836	 	"T4836"
#define TAG_NAME_4837	 	"T4837"
//#define TAG_NAME_4838	 	"T4838"
#define TAG_NAME_4840	 	"T4840"
#define TAG_NAME_4841	 	"T4841"
#define TAG_NAME_4842	 	"T4842"
#define TAG_NAME_484200	 	"T484200"
#define TAG_NAME_484201	 	"T484201"
#define TAG_NAME_484202	 	"T484202"
#define TAG_NAME_484203	 	"T484203"
#define TAG_NAME_4842_END 	"/T4842"
#define TAG_NAME_4843	 	"T4843"
#define TAG_NAME_4844	 	"T4844"
#define TAG_NAME_4845	 	"T4845"
#define TAG_NAME_484500	 	"T484500"
#define TAG_NAME_484501	 	"T484501"
#define TAG_NAME_484502	 	"T484502"
#define TAG_NAME_484503	 	"T484503"
#define TAG_NAME_484504	 	"T484504"
#define TAG_NAME_484505	 	"T484505"
#define TAG_NAME_484506	 	"T484506"
#define TAG_NAME_484507	 	"T484507"
#define TAG_NAME_4845_END 	"/T4845"
#define TAG_NAME_4846	 	"T4846"
#define TAG_NAME_4847	 	"T4847"
#define TAG_NAME_4848	 	"T4848"
#define TAG_NAME_4849	 	"T4849"
#define TAG_NAME_4852	 	"T4852"
#define TAG_NAME_4853	 	"T4853"
#define TAG_NAME_4900		"T4900"
#define TAG_NAME_4901		"T4901"
#define TAG_NAME_4902		"T4902"
#define TAG_NAME_4903		"T4903"
#define TAG_NAME_4904		"T4904"
#define TAG_NAME_4905		"T4905"
#define TAG_NAME_4906		"T4906"
#define TAG_NAME_5000		"T5000"
#define TAG_NAME_5001		"T5001"
#define TAG_NAME_5002		"T5002"
#define TAG_NAME_5003		"T5003"
#define TAG_NAME_5100		"T5100"
#define TAG_NAME_5101		"T5101"
#define TAG_NAME_5102		"T5102"
#define TAG_NAME_5501		"T5501"
#define TAG_NAME_5503		"T5503"
#define TAG_NAME_5509		"T5509"
#define TAG_NAME_5531		"T5531"
#define TAG_NAME_5532		"T5532"
#define TAG_NAME_553202		"T553202"
#define TAG_NAME_553204		"T553204"
#define TAG_NAME_5533		"T5533"
#define TAG_NAME_553301		"T553301"
#define TAG_NAME_553304		"T553304"
#define TAG_NAME_553307		"T553307"
#define TAG_NAME_553309		"T553309"
#define TAG_NAME_553311		"T553311"
#define TAG_NAME_5534		"T5534"
#define TAG_NAME_5535		"T5535"
#define TAG_NAME_5536		"T5536"
#define TAG_NAME_5539		"T5539"
#define TAG_NAME_5548		"T5548"
#define TAG_NAME_5548_END	"/T5548"
#define TAG_NAME_5532		"T5532"
#define TAG_NAME_554801		"T554801"
#define TAG_NAME_554802		"T554802"
#define TAG_NAME_554803		"T554803"
#define TAG_NAME_554804		"T554804"
#define TAG_NAME_554805		"T554805"
#define TAG_NAME_554806		"T554806"
#define TAG_NAME_554807		"T554807"
#define TAG_NAME_554808		"T554808"
#define TAG_NAME_554809		"T554809"
#define TAG_NAME_554810		"T554810"
#define TAG_NAME_5572		"T5572"
#define TAG_NAME_5574		"T5574"
#define TAG_NAME_5581		"T5581"
#define TAG_NAME_5582		"T5582"
#define TAG_NAME_5583		"T5583"
#define TAG_NAME_5588		"T5588"
#define TAG_NAME_558801		"T558801"
#define TAG_NAME_558802		"T558802"
#define TAG_NAME_558803		"T558803"
#define TAG_NAME_5589		"T5589"
#define TAG_NAME_558901		"T558901"
#define TAG_NAME_5593		"T5593"
#define TAG_NAME_5594		"T5594"
#define TAG_NAME_5595		"T5595"
#define TAG_NAME_5596		"T5596"
#define TAG_NAME_5597		"T5597"
#define TAG_NAME_5599		"T5599"
#define TAG_NAME_6400		"T6400"
#define TAG_NAME_6401		"T6401"
#define TAG_NAME_6404		"T6404"
#define TAG_NAME_6406		"T6406"
#define TAG_NAME_6407		"T6407"
#define TAG_NAME_6409		"T6409"

#define XML_CHECK_NUMERIC_ALPHA			1//文字+數字
#define XML_CHECK_NUMERIC				2//數字
#define XML_CHECK_NUMERIC_AMOUNT		3//數字(含2位小數)
#define XML_CHECK_NUMERIC_TIME			4//數字(時間格式)
#define XML_CHECK_NUMERIC_DATE			5//數字(日期格式)
#define XML_CHECK_NUMERIC_DATE_TIME		6//數字(日期+時間格式)

void vdSetXMLActionData(STRUCT_XML_ACTION_DATA *srXMLAction,char *chXMLName,char *chValue,BOOL fInitial,int inNodeNoType,int inNodeType,BOOL fXMLSendFlag);
int inParseXML(char *bFileName,BYTE *bXMLInData,BOOL fParseByFile,int inXMLInLen,STRUCT_XML_DOC *srXML,int inFileOffset);
int inInitialSystemXML(char *bFileName);
int inGetTag(BYTE *bInData,int inCnt,int inFileSize,STRUCT_XML_DOC *srXML,int inNodeNumber);
int inGetValue(BYTE *bInData,int inCnt,int inFileSize,STRUCT_XML_DOC *srXML);
int inSkipLine(BYTE *bInData,int inCnt,int inFileSize);
int inGetEndTag(BYTE *bInData,int inCnt,int inFileSize,STRUCT_XML_DOC *srXML);
int inXMLGetParseData(STRUCT_XML_ACTION_DATA *srXMLAction,STRUCT_XML_DOC *srXML);
void vdFreeXMLDOC(STRUCT_XML_DOC *srXML);
int inXMLGetData(STRUCT_XML_DOC *srXML,char *chGetName,char *chValue,int inValueLen,int inStartIndex);
int inXMLGetAmt(STRUCT_XML_DOC *srXML,char *chGetName,long *lnValue,int inStartIndex);
int inXMLInsertData(STRUCT_XML_DOC *srXML,STRUCT_XML_ACTION_DATA *srXMLAction,char *chInsertTagName,BOOL fXMLSendFlag);
int inXMLMoveMultipleData(STRUCT_XML_DOC *srXML,int inMoveStart,int inMoveCnt);
int inXMLMoveMultipleData2(STRUCT_XML_DOC *srXML,int inMoveSour,int inMoveDest,int inMoveCnt);
int inXMLMoveData(STRUCT_XML_ELEMENT *srSourElement,STRUCT_XML_ELEMENT *srDestElement);
int inXMLCopyMultipleData(STRUCT_XML_DOC *srXML,int inCopytart,STRUCT_XML_ACTION_DATA *srXMLAction);
int inXMLCopyData(int inNodeNo,STRUCT_XML_DETAIL *srSourElement,STRUCT_XML_ELEMENT *srDestElement);
int inXMLCheckTMInputData(int inTxnType,STRUCT_XML_DOC *srXML);
int inXMLSearchValueByTag(STRUCT_XML_DOC *srXML,char *chTag,int *inStart,int *inEnd,int inInStartCnt);
int inXMLCheckTagDuplicated(STRUCT_XML_DOC *srXML,char *chTag);
void vdBuildXMLHeaderMsg(BYTE *bOutData,STRUCT_XML_DOC *bInData);
void vdBuildXMLResponse(int inTxnType,BYTE *bOutData,int inOutDataStart,STRUCT_XML_DOC *srXMLRes,BOOL bResponseAll);
int inXMLUpdateData(STRUCT_XML_DOC *srXML,char *chInsterTagName,char *chUpdateTagName,BYTE *bUpdateValue,BOOL fXMLSendFlag);
int inXMLCheckTMCanNotInputData(int inTxnType,STRUCT_XML_DOC *srXML);
//int inXMLModifyData(STRUCT_XML_DOC *srXML,char *chModiftName,char *chModifyValue);
int inCheckXMLDataFormat(STRUCT_XML_DOC *srXML);
BOOL fIsXMLTAGFormat(STRUCT_XML_ELEMENT *srElement);
int inXMLFormatCheck(char *chTag,short shTagLen,int inFormatType);
int inGetXMLCheckType(long lnTag);
int inXMLCompareData(STRUCT_XML_DOC *srXML,char *chTagName,char *chCompareValue,int inStartIndex);
int inXMLDeleteData(STRUCT_XML_DOC *srXML,int inDeleteStart,int inDeleteCnt);
int inXMLAppendData(STRUCT_XML_DOC *srXML,char *chTagName,int inTagLen,BYTE *chValueName,int inValueLen,int inNodeType,BOOL fXMLSendFlag);
void vdSetConfigFlag(STRUCT_XML_DOC *srXML,int inSetType,char *chTagName);
void vdUnPackToXMLData(BYTE *BINData,int BINSize,STRUCT_XML_DOC *srXML,char *chTagName,int inTagLen,int inNodeType,BOOL fXMLSendFlag);
void vdXMLBackUpData(STRUCT_XML_DOC *srXMLIn,STRUCT_XML_DOC *srXMLOut,BOOL fFreeData);
void vdModifySAmtTag(STRUCT_XML_DOC *srXML,int inMainRetVal);
void vdXMLChangeTagName(STRUCT_XML_DOC *srXML,char *bOrgTagName,char *bNewTagName,char *bAppendData);
void vdBuildRespData(int inRet,STRUCT_XML_DOC *bInData);
void vdMaskCMASCardID(STRUCT_XML_DOC *srXML);
void vdBuildAutolaodData(STRUCT_XML_DOC *srXML);

#ifdef __cplusplus
	}
#endif // __cplusplus

#endif
