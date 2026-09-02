/* 
 * File:   XML.h
 * Author: user
 *
 * Created on 2018年3月27日, 下午 2:10
 */

#define _XML_ESCAPE_AMPERSAND_		"&amp;"
#define _XML_ESCAPE_LESS_THAN_		"&lt;"
#define _XML_ESCAPE_GREATER_THAN_	"&gt;"
#define _XML_ESCAPE_QUOTES_		"&quot;"
#define _XML_ESCAPE_APOSTROPHE_		"&apos;"

int inXML_ParseFile(char* szFileName, xmlDocPtr* srDoc);
int inXML_SaveFile(char *szFilename, xmlDocPtr* srDoc, char *szEncoding, int inFormat);
int inXML_Get_RootElement(xmlDocPtr* srDoc, xmlNodePtr* srCur);
int inXML_FreeDOC(xmlDocPtr* srdoc);
int inXML_CleanParser(void);
int inXML_MemoryDump(void);
int inXML_End(xmlDocPtr* srdoc);
int inXML_Edit_Properties(char* szXMLFilePath, char* szXMLFileName, xmlNodePtr srTargetNode, char* szEditValue);
int inXML_ReadFile(char* szFileName, xmlDocPtr* srDoc, char* szEncoding, int inOption);
int inXML_Escape_Character_Transform(char* szInData, int inDataSize, char* szOutData);
int inXML_Escape_Character_Recover_By_File(char* szFileName, char* szPath);
void vdTraverse_and_print(xmlNode *node);
void vdTraverse_and_edit(xmlNode *node, xmlNode *srTargetNode, char* szEditValue);
int inXML_Init(void);

