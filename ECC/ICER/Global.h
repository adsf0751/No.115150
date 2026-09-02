#ifndef __GLOBAL_H__
#define __GLOBAL_H__

//extern int gDLLVersion;//1=一代API / 2=二代API
//extern int gLogFlag;//1=Log On / 0=Log Off
//extern int gTCPIPTimeOut;//主機連線TimeOut秒數
//extern int gLogCnt;//Log保留個數(一天一個)
extern STRUCT_TXN_DATA srTxnData;
extern STRUCT_XML_ACTION_DATA srXMLActionData;
extern STRUCT_XML_DOC srXMLData;
extern Store_Data stStore_DataInfo_t;
extern STRUCT_XML_DOC srBankXMLData;
extern STRUCT_TMP_DATA gTmpData;

//extern BYTE bXMLFileData[MAX_XML_FILE_SIZE];
//extern int inXMLFileSize;

#endif
