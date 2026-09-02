#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>
#include <unistd.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Transaction.h"
#include "../DISPLAY/Display.h"
#include "File.h"
#include "Function.h"
#include "XML.h"

extern int	ginDebug;

/*
Function        :inXML_ParseFile
Date&Time       :2018/3/27 下午 3:36
Describe        :解析失敗回NULL
*/
int inXML_ParseFile(char* szFileName, xmlDocPtr* srDoc)
{
	*srDoc = xmlParseFile(szFileName);
	if (*srDoc == NULL)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inXML_SaveFile
Date&Time       :2018/3/27 下午 3:00
Describe        :返回值：写入文件中的字节数量
*/
int inXML_SaveFile(char *szFilename, xmlDocPtr* srDoc, char *szEncoding, int inFormat)
{
	int	inRetVal = -1;
	int	inEncodingLen = 0;
	char	szEncodingDefault[20 + 1] = "utf-8";
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inXML_SaveFile() START !");
	}
	
	inEncodingLen = strlen(szEncoding);
	if (inEncodingLen > 0)
	{
		memset(szEncodingDefault, 0x00, sizeof(szEncodingDefault));
		if (inEncodingLen >= sizeof(szEncodingDefault))
		{
			memcpy(szEncodingDefault, szEncoding, sizeof(szEncodingDefault));
		}
		else
		{
			memcpy(szEncodingDefault, szEncoding, strlen(szEncoding));
		}
	}
	
	inRetVal = xmlSaveFormatFileEnc(szFilename, *srDoc, szEncodingDefault, inFormat);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inXML_SaveFile() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	if (inRetVal < 0)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inXML_Get_RootElement
Date&Time       :2018/3/27 下午 4:32
Describe        :
*/
int inXML_Get_RootElement(xmlDocPtr* srDoc, xmlNodePtr* srCur)
{
	char	szDebugMsg[100 + 1] = {0};
	
	*srCur = xmlDocGetRootElement(*srDoc);
	if (*srCur == NULL)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Get Root Failed");
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}
/*
Function        :inXML_FreeDOC
Date&Time       :2018/3/27 下午 3:11
Describe        :釋放結構
*/
int inXML_FreeDOC(xmlDocPtr* srdoc)
{
	xmlFreeDoc(*srdoc);
	
	return (VS_SUCCESS);
}

/*
Function        :inXML_CleanParser
Date&Time       :2018/3/27 下午 3:13
Describe        :只有在離開程式實需調用，即不用使用
*/
int inXML_CleanParser(void)
{
	xmlCleanupParser();
	
	return (VS_SUCCESS);
}

/*
Function        :inXML_MemoryDump
Date&Time       :2018/3/27 下午 3:14
Describe        :
*/
int inXML_MemoryDump(void)
{
	xmlMemoryDump();
	
	return (VS_SUCCESS);
}

/*
Function        :inXML_End
Date&Time       :2018/3/27 下午 3:16
Describe        :
*/
int inXML_End(xmlDocPtr* srdoc)
{
	inXML_FreeDOC(srdoc);

	return (VS_SUCCESS);
}

/*
Function        :直接修改xml裡面特定node裡面的值
Date&Time       :2024/8/7 下午 2:26
Describe        :只要給定parent的name和要修改node的name就可以修改
*/
int inXML_Edit_Properties(char* szXMLFilePath, char* szXMLFileName, xmlNodePtr srTargetNode, char* szEditValue)
{
        int             inRetVal = VS_SUCCESS;
        char            szPathName[200 + 1] = {0};
        xmlDocPtr       srDoc = NULL;
        xmlNodePtr	srRoot = NULL;		/* root */
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inXML_Edit_Properties() START !");
                inLogPrintf(AT, "FN:\"%s\",Value:\"%s\",Tag Name:\"%s\"", szXMLFileName, szEditValue, srTargetNode->name);
	}
        
        if (srTargetNode == NULL)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "srTargetNode == NULL");
                }
                return (VS_ERROR);
        }
        
        memset(szPathName, 0x00, sizeof(szPathName));
        sprintf(szPathName, "%s%s", szXMLFilePath, szXMLFileName);
	
	if (inFile_Linux_Check_File_Exist(szXMLFilePath, szXMLFileName) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "XML File Not Exist");
		}
		return (VS_ERROR);
	}
	
	
	inRetVal = inXML_ReadFile(szPathName, &srDoc, NULL, XML_PARSE_RECOVER | XML_PARSE_PEDANTIC);
	/* 異常 */
	if (inRetVal != VS_SUCCESS)
	{
                if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "XML Read Fail");
		}
		return (VS_ERROR);
	}
        
        inRetVal = inXML_Get_RootElement(&srDoc, &srRoot);
	/* 異常 */
	if (inRetVal != VS_SUCCESS)
	{
                if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "XML Get_RootElement Fail");
		}
                
		inXML_End(&srDoc);
		
		return (VS_ERROR);
	}
        
        /* 查找node */
        vdTraverse_and_edit(srRoot, srTargetNode, szEditValue);
        
        /* 儲存xml檔 xml檔現在會自動換行了 */
        inXML_SaveFile(szPathName, &srDoc, "utf-8", 1);
        /* free */
	inXML_End(&srDoc);
	
	return (VS_SUCCESS);
}

/*
Function        :inXML_ReadFile
Date&Time       :2018/3/27 下午 3:36
Describe        :*url:可填網址或文件名
 *		 *encoding:編碼方式
*/
int inXML_ReadFile(char* szFileName, xmlDocPtr* srDoc, char* szEncoding, int inOption)
{
	*srDoc = xmlReadFile(szFileName, szEncoding, inOption);
	if (*srDoc == NULL)
	{
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "inXML_ReadFile\"%s\" Fail", szFileName);
                }
		return (VS_ERROR);
	}
	else
	{
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "inXML_ReadFile\"%s\" success", szFileName);
                }
		return (VS_SUCCESS);
	}
}

/*
Function        :inXML_Escape_Character_Transform
Date&Time       :2020/7/24 下午 4:08
Describe        :因xml有跳脫字元，故須轉換以下字元
 *		
		特殊字元		替代字元
		<		&lt;
		>		&gt;
		&		&amp;
		'		&apos;
		"		&quot;
*/
int inXML_Escape_Character_Transform(char* szInData, int inDataSize, char* szOutData)
{
	int	i = 0;
	int	j = 0;
	int	inLen = 0;
	
	for (i = 0; i <= inDataSize; i++)
	{
		if (szInData[i] == '<')
		{
			inLen = strlen(_XML_ESCAPE_LESS_THAN_);
			memcpy(&szOutData[j], _XML_ESCAPE_LESS_THAN_, inLen);
			j += inLen;
		}
		else if (szInData[i] == '>')
		{
			inLen = strlen(_XML_ESCAPE_GREATER_THAN_);
			memcpy(&szOutData[j], _XML_ESCAPE_GREATER_THAN_, inLen);
			j += inLen;
		}
		else if (szInData[i] == '&')
		{
			inLen = strlen(_XML_ESCAPE_AMPERSAND_);
			memcpy(&szOutData[j], _XML_ESCAPE_AMPERSAND_, inLen);
			j += inLen;
		}
		else if (szInData[i] == '\'')
		{
			inLen = strlen(_XML_ESCAPE_APOSTROPHE_);
			memcpy(&szOutData[j], _XML_ESCAPE_APOSTROPHE_, inLen);
			j += inLen;
		}
		else if (szInData[i] == '\"')
		{
			inLen = strlen(_XML_ESCAPE_QUOTES_);
			memcpy(&szOutData[j], _XML_ESCAPE_QUOTES_, inLen);
			j += inLen;
		}
		else
		{
			szOutData[j] = szInData[i];
			j++;
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inXML_Escape_Character_Recover_By_File
Date&Time       :2020/7/27 下午 3:55
Describe        :因為悠遊卡不會自動轉換XML跳脫字元，所以要幫忙轉換。
*/
int inXML_Escape_Character_Recover_By_File(char* szFileName, char* szPath)
{
	int	inFd1 = 0;
	int	inFd2 = 0;
	int	i, j = 0;
	int	inFileLen = 0;
	int	inReplaceLen = 0;
	int	inReadCnt = 0;
	char	szFileName1[100 + 1] = {0};
	char	szFileName2[100 + 1] = {0};
	char	szTempName[50 + 1] = {"ECC_AFTER.xml"};
	char	szTempBuffer[10 + 1] = {0};
	
	memset(szFileName1, 0x00, sizeof(szFileName1));
	sprintf(szFileName1, "%s%s", szPath, szFileName);
	memset(szFileName2, 0x00, sizeof(szFileName2));
	sprintf(szFileName2, "%s%s", _AP_ROOT_PATH_, szTempName);
	
	inFile_Unlink_File(szTempName, _AP_ROOT_PATH_);
	inFile_Linux_Get_FileSize_By_Stat(szFileName1, &inFileLen);
	
	inFile_Linux_Open(&inFd1, szFileName1);
	inFile_Linux_Create(&inFd2, szFileName2);
	inFile_Linux_Seek(inFd1, 0, _SEEK_BEGIN_);
	inFile_Linux_Seek(inFd2, 0, _SEEK_BEGIN_);
	
	for (i = 0; i < inFileLen; i++)
	{
		inReplaceLen = 0;
		memset(szTempBuffer, 0x00, sizeof(szTempBuffer));
		inReadCnt = 1;
		inFile_Linux_Read(inFd1, &szTempBuffer[0], &inReadCnt);
		inReplaceLen += 1;
		if (szTempBuffer[0] == '&')
		{
			do
			{
				inReadCnt = 1;
				inFile_Linux_Read(inFd1, &szTempBuffer[inReplaceLen], &inReadCnt);
				inReplaceLen += 1;
				i++;
				if (szTempBuffer[inReplaceLen - 1] == ';')
				{
					break;
				}
			} while (i < inFileLen);
			
			if (memcmp(szTempBuffer, _XML_ESCAPE_AMPERSAND_, strlen(_XML_ESCAPE_AMPERSAND_)) == 0)
			{
				inReadCnt = 1;
				inFile_Linux_Write(inFd2, "&", &inReadCnt);
				j++;
			}
			else if (memcmp(szTempBuffer, _XML_ESCAPE_LESS_THAN_, strlen(_XML_ESCAPE_LESS_THAN_)) == 0)
			{
				inReadCnt = 1;
				inFile_Linux_Write(inFd2, "<", &inReadCnt);
				j++;
			}
			else if (memcmp(szTempBuffer, _XML_ESCAPE_GREATER_THAN_, strlen(_XML_ESCAPE_GREATER_THAN_)) == 0)
			{
				inReadCnt = 1;
				inFile_Linux_Write(inFd2, ">", &inReadCnt);
				j++;
			}
			else if (memcmp(szTempBuffer, _XML_ESCAPE_QUOTES_, strlen(_XML_ESCAPE_QUOTES_)) == 0)
			{
				inReadCnt = 1;
				inFile_Linux_Write(inFd2, "\"", &inReadCnt);
				j++;
			}
			else if (memcmp(szTempBuffer, _XML_ESCAPE_APOSTROPHE_, strlen(_XML_ESCAPE_APOSTROPHE_)) == 0)
			{
				inReadCnt = 1;
				inFile_Linux_Write(inFd2, "'", &inReadCnt);
				j++;
			}
			
		}
		else
		{
			inReadCnt = 1;
			inFile_Linux_Write(inFd2, szTempBuffer, &inReadCnt);
			j++;
		}
	}
	
	inFile_Linux_Close(inFd1);
	inFile_Linux_Close(inFd2);
	inFile_Unlink_File(szFileName, szPath);
	inFunc_Data_Rename(szTempName, _AP_ROOT_PATH_, szFileName, szPath);
	/* 會有權限問題，要先改權限 */
	inFunc_Data_Chmod("u+rwx", szFileName, szPath);
	
	return (VS_SUCCESS);
}

/*
 Function        :vdTraverse_and_print
 Date&Time       :2024/8/7 下午 4:57
 Describe        :遞迴函數遍歷所有節點
 */
void vdTraverse_and_print(xmlNode *node)
{
        xmlNode *ptrCur = NULL;
                
        for (ptrCur = node; ptrCur; ptrCur = ptrCur->next)
        {
            if (ptrCur->type == XML_ELEMENT_NODE)
            {
                    if (ginDebug == VS_TRUE)
                    {
                            inLogPrintf(AT, "節點名稱: %s\n", ptrCur->name);
                    }
                    if (ptrCur->children && ptrCur->children->content)
                    {
                            if (ginDebug == VS_TRUE)
                            {
                                    inLogPrintf(AT, "節點內容: %s\n", (char *)ptrCur->children->content);
                            }
                    }
            }
            //遞迴遍歷子節點
            vdTraverse_and_print(ptrCur->children);
        }
}

/*
 Function        :vdTraverse_and_edit
 Date&Time       :2024/8/7 下午 4:57
 Describe        :遞迴函數遍歷所有節點並修改特定節點
 */
void vdTraverse_and_edit(xmlNode *node, xmlNode *srTargetNode, char* szEditValue)
{
        xmlNode *ptrCur = NULL;
        
        for (ptrCur = node; ptrCur != NULL; ptrCur = ptrCur->next)
        {
                if (ptrCur->type == XML_ELEMENT_NODE)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "節點名稱: %s\n", ptrCur->name);
                        }

                        if (ptrCur->children && ptrCur->children->content)
                        {
                                if (ginDebug == VS_TRUE)
                                {
                                        inLogPrintf(AT, "節點內容: %s\n", (char *)ptrCur->children->content);
                                }
                        }
                        
                        /* 用NodeName、parent Name、content做搜索條件 */
                        if(xmlStrcmp(ptrCur->name, srTargetNode->name) == 0                                                         &&
                          (ptrCur->parent->name && xmlStrcmp(ptrCur->parent->name, srTargetNode->parent->name) == 0))
                        {
                                xmlNodeSetContent(ptrCur, (const xmlChar *)szEditValue);
                                if (ginDebug == VS_TRUE)
                                {
                                        inLogPrintf(AT, "Node content changed to: %s\n", ptrCur->children->content);
                                }
                        }
                }

                //遞迴遍歷子節點
                vdTraverse_and_edit(ptrCur->children, srTargetNode, szEditValue);
        }
}

/*
Function        :inXML_Escape_Character_Recover_By_File
Date&Time       :2020/7/27 下午 3:55
Describe        :因為悠遊卡不會自動轉換XML跳脫字元，所以要幫忙轉換。
*/
int inXML_Init(void)
{
        LIBXML_TEST_VERSION;
        
        /* 配置全域變數 */
        xmlKeepBlanksDefault(0);   // 不保留空白節點
        xmlIndentTreeOutput = 1;   // 輸出 XML 時使用縮排格式
        
        xmlInitParser();
        
        return (VS_SUCCESS);
}