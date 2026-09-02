#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <unistd.h>
#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/AllStruct.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/DISPLAY/DispMsg.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/File.h"
#include "../SOURCE/FUNCTION/MVT.h"
#include "../SOURCE/FUNCTION/EST.h"
#include "../SOURCE/FUNCTION/VWT.h"
#include "../SOURCE/FUNCTION/XML.h"
#include "../SOURCE/FUNCTION/NexsysSDK.h"
#include "../SOURCE/FUNCTION/Utility.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../CTLS/CTLS.h"
#include "EMVsrc.h"
#include "EMVxml.h"

extern  int     ginDebug;			/* Debug使用 extern */
extern	int	ginMachineType;
extern	char	gszTermVersionID[16 + 1];
extern	char	gszTermVersionDate[16 + 1];

/*
Function        :inEMVXML_Create_EMVConfigXml
Date&Time       :2016/3/22 下午 5:35
Describe        :組EMV使用的xml檔案
*/
int inEMVXML_Create_EMVConfigXml(char* szConfigFileName)
{
        xmlDocPtr       srDoc = NULL;
        xmlNodePtr      srRoot_Node = NULL, srConfig_Node = NULL, srCAPK_Node = NULL, 
                        srAppList_Node = NULL, srTerminalConfig_Node = NULL, srAppConfig_Node = NULL,
                        srGroup_Node = NULL, srItem_Node = NULL;
        SHA_CTX         srSHA;
        int		i, inRetVal;
        char		szTemplate[64 + 1];
	char		szSerialNumber[16 + 1];
        unsigned char   uszBCD[512 + 1];
        /* CAPK用 */
        int     inBCDLen, inCAPK_DataLen;
        char    szCAPKId[10 + 1], szCAPKId_BAK[10 + 1];
        char    szCAPKIndex[2 + 1];
        char    szCAPKKeyModulus[496 + 1];
        char    szCAPKExponent[6 + 1];
        char    szCAPKIdIndex[20 + 1];
        char    szCAPKData[512 + 1];
        char    szCAPKHash[40 + 1];
        unsigned char   uszHash[32 + 1];
        /* MVT參數用 */
        int     inAPPIndex = 0;
        char    szAPPIndex[2 + 1];
        char    szMVTApplicationId[16 + 1];
        char    szTerminalType[2 + 1];
        char    szTerminalCapabilities[6 + 1];
        char    szAdditionalTerminalCapabilities[10 + 1];
        char    szTerminalCountryCode[4 + 1];
        char    szTransactionCurrencyCode[4 + 1];
        char    szDefaultTAC[10 + 1];
        char    szOnlineTAC[10 + 1];
        char    szDenialTAC[10 + 1];
        char    szDefaultTDOL[6 + 1];
        char    szDefaultDDOL[6 + 1];
        char    szEMVFloorLimit[12 + 1];
        char    szRandomSelectionThreshold[12 + 1];
        char    szTargetPercentforRandomSelection[2 + 1];
        char    szMaxTargetPercentforRandomSelection[2 + 1];
        char    szMerchantCategoryCode[4 + 1];
        char    szTransactionCategoryCode[1 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inEMVXML_Create_EMVConfigXml()_START");
        
        /* 先預LOAD MVT及EST 失敗 Return Error */
        if (inLoadMVTRec(0) < 0)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inLoadMVTRec(0)Error!!");
                
                return (VS_ERROR);
        }
        
        if (inLoadESTRec(0) < 0)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inLoadESTRec(0)Error!!");
                
                return (VS_ERROR);
        }
        
        /* 建立一個新的檔案 */
        srDoc = xmlNewDoc(BAD_CAST "1.0");
        srRoot_Node = xmlNewNode(NULL, BAD_CAST "configurationDescriptor");
        xmlNewProp(srRoot_Node, BAD_CAST "version", BAD_CAST "01");    
        xmlDocSetRootElement (srDoc, srRoot_Node);
        
        /* 目前只有一個Config 預設true */
        srConfig_Node = xmlNewNode(NULL, BAD_CAST "Config");
        xmlNewProp(srConfig_Node, BAD_CAST "index", BAD_CAST "01");
        xmlNewProp(srConfig_Node, BAD_CAST "active", BAD_CAST "true" );
        xmlAddChild(srRoot_Node, srConfig_Node);
    
        /* CAPKConfig */
        srCAPK_Node = xmlNewNode(NULL, BAD_CAST "CAPKConfig");
        xmlAddChild(srConfig_Node, srCAPK_Node);
        
        memset(szCAPKId_BAK, 0x00, sizeof(szCAPKId_BAK));
        
        for (i = 0 ;; i ++)
        {
                if (inLoadESTRec(i) < 0)
		{
			if (i == 0)
			{
				/* 完全沒有CAPK，會導致虹堡kernel回傳錯誤 */
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "NO CAPK ERROR");
				}
				inUtility_StoreTraceLog_OneStep("NO CAPK ERROR");
				
				return (VS_ERROR);
			}
			else
			{
				break;
			}
		}
      
                memset(szCAPKId, 0x00, sizeof(szCAPKId));
                inGetCAPKApplicationId(szCAPKId);
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "RID: %s", szCAPKId);
                        inLogPrintf(AT, szTemplate);
                }
		
                /* 比對前一次的CAPK 相同的話用同一個父標籤 */
                if (!memcmp(&szCAPKId_BAK[0], &szCAPKId[0], 10))
                {
                        /* 相同CAPK ID */
                }
                else
                {
                        /* Copy現在的CAPK到CAPK_BAK */
                        memset(szCAPKId_BAK, 0x00 ,sizeof(szCAPKId_BAK));
                        memcpy(&szCAPKId_BAK[0], &szCAPKId[0], 10);
                        
                        srGroup_Node = xmlNewNode(NULL, BAD_CAST "Group");
                        xmlNewProp(srGroup_Node, BAD_CAST "RID", BAD_CAST szCAPKId);
                        xmlAddChild(srCAPK_Node, srGroup_Node);
                }

                /* index */
                memset(szCAPKIndex, 0x00, sizeof(szCAPKIndex));
                inGetCAPKIndex(szCAPKIndex);
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "Index: %s", szCAPKIndex);
                        inLogPrintf(AT, szTemplate);
                }
                
                srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                xmlNewProp(srItem_Node, BAD_CAST "index", BAD_CAST szCAPKIndex);
                xmlAddChild(srGroup_Node, srItem_Node);

                /* modules */
                memset(szCAPKKeyModulus, 0x00, sizeof(szCAPKKeyModulus));
                inGetCAPKKeyModulus(szCAPKKeyModulus);
                xmlNewChild(srItem_Node, NULL, BAD_CAST "modules", BAD_CAST szCAPKKeyModulus);
                
                /* exponent */
                memset(szCAPKExponent, 0x00, sizeof(szCAPKExponent));
                inGetCAPKExponent(szCAPKExponent);
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "Exponent: %s", szCAPKExponent);
                        inLogPrintf(AT, szTemplate);
                }
                
                xmlNewChild(srItem_Node, NULL, BAD_CAST "exponent", BAD_CAST szCAPKExponent);

                /* expirydata */
                /* CAPK Key值有效期(端末機暫不參考CAPK有效期限) by 聯合TMS規格 */
                xmlNewChild(srItem_Node, NULL, BAD_CAST "expirydata", NULL);
                
                /* hash */
                /* 組CAPK Data CAPK_ID + Index + Modulus + Exponet */
                /* 第一部份 ID + Index */
                memset(szCAPKData, 0x00, sizeof(szCAPKData));
                memset(uszBCD, 0x00, sizeof(uszBCD));
                memset(szCAPKIdIndex, 0x00, sizeof(szCAPKIdIndex));
                strcpy(szCAPKIdIndex, szCAPKId);
                strcat(szCAPKIdIndex, szCAPKIndex);
                inBCDLen = strlen(szCAPKIdIndex) / 2;
                inFunc_ASCII_to_BCD(&uszBCD[0], &szCAPKIdIndex[0], inBCDLen);
                memcpy(&szCAPKData[0], &uszBCD[0], inBCDLen);
                inCAPK_DataLen = inBCDLen;

                /* 第二部份 Modulus */
                memset(uszBCD, 0x00, sizeof(uszBCD));
                inBCDLen = strlen(szCAPKKeyModulus) / 2;
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "CAPK Key Length: %d", inBCDLen);
                        inLogPrintf(AT, szTemplate);
                }
                
                inFunc_ASCII_to_BCD(&uszBCD[0], &szCAPKKeyModulus[0], inBCDLen);
                memcpy(&szCAPKData[inCAPK_DataLen], &uszBCD[0], inBCDLen);
                inCAPK_DataLen = inCAPK_DataLen + inBCDLen;
                
                /* 第三部份 exponent */
                memset(uszBCD, 0x00, sizeof(uszBCD));
                inBCDLen = strlen(szCAPKExponent) / 2;
                inFunc_ASCII_to_BCD(&uszBCD[0], &szCAPKExponent[0], inBCDLen);
                memcpy(&szCAPKData[inCAPK_DataLen], &uszBCD[0], inBCDLen);
                inCAPK_DataLen = inCAPK_DataLen + inBCDLen;

                /* 計算Hash */
                memset(&srSHA, 0x00, sizeof(SHA_CTX));
                /* Initialize the SHA_CTX structure and perpart for the SHA1 operation */
                CTOS_SHA1Init(&srSHA);
                /* Perform the SHA1 algorithm with the input data */
                CTOS_SHA1Update(&srSHA, (unsigned char *)szCAPKData, inCAPK_DataLen);
                /* Finalize the SA1 operation and retrun the result */
                memset(uszHash, 0x00, sizeof(uszHash));
                CTOS_SHA1Final(uszHash, &srSHA);
                
                /* 20Bytes的HASH須轉成ASCII存入 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inFunc_BCD_to_ASCII(szTemplate, uszHash, 40);
                memset(szCAPKHash, 0x00, sizeof(szCAPKHash));
                memcpy(&szCAPKHash[0], &szTemplate[0], 40);
       
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "Hash");
                        inLogPrintf(AT, szCAPKHash);
                }

                xmlNewChild(srItem_Node, NULL, BAD_CAST "hash", BAD_CAST szCAPKHash);
        }
        
        /* AppList */
        srAppList_Node = xmlNewNode(NULL, BAD_CAST "AppList");
        xmlAddChild(srConfig_Node, srAppList_Node);
        
        for (i = 0 ; ; i ++)
        {
                if (inLoadMVTRec(i) < 0)
                        break;
                
                /* Item */
                inAPPIndex ++;
                memset(szAPPIndex, 0x00, sizeof(szAPPIndex));
                sprintf(szAPPIndex, "%02d", inAPPIndex);
                
                srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                xmlNewProp(srItem_Node, BAD_CAST "index", BAD_CAST szAPPIndex);
                
                /* Name AID ASI */
                memset(szMVTApplicationId, 0x00, sizeof(szMVTApplicationId));
                inGetMVTApplicationId(szMVTApplicationId);
		               
                if (!memcmp(&szMVTApplicationId[0], "A000000003", 10))
                {
                        xmlNewChild(srItem_Node, NULL, BAD_CAST "Name", BAD_CAST "VISA");
                        xmlNewChild(srItem_Node, NULL, BAD_CAST "AID", BAD_CAST szMVTApplicationId);
                        xmlNewChild(srItem_Node, NULL, BAD_CAST "ASI", BAD_CAST "00");
                        xmlAddChild(srAppList_Node, srItem_Node);
                }
                else if (!memcmp(&szMVTApplicationId[0], "A000000004", 10))
                {
                        xmlNewChild(srItem_Node, NULL, BAD_CAST "Name", BAD_CAST "MasterCard");
                        xmlNewChild(srItem_Node, NULL, BAD_CAST "AID", BAD_CAST szMVTApplicationId);
                        xmlNewChild(srItem_Node, NULL, BAD_CAST "ASI", BAD_CAST "00");
                        xmlAddChild(srAppList_Node, srItem_Node);
                }
                else if (!memcmp(&szMVTApplicationId[0], "A000000065", 10))
                {
                        xmlNewChild(srItem_Node, NULL, BAD_CAST "Name", BAD_CAST "JCB");
                        xmlNewChild(srItem_Node, NULL, BAD_CAST "AID", BAD_CAST szMVTApplicationId);
                        xmlNewChild(srItem_Node, NULL, BAD_CAST "ASI", BAD_CAST "00");
                        xmlAddChild(srAppList_Node, srItem_Node);
                }
                else if (!memcmp(&szMVTApplicationId[0], "A000000333", 10))
                {
                        xmlNewChild(srItem_Node, NULL, BAD_CAST "Name", BAD_CAST "UICC");
                        xmlNewChild(srItem_Node, NULL, BAD_CAST "AID", BAD_CAST szMVTApplicationId);
                        xmlNewChild(srItem_Node, NULL, BAD_CAST "ASI", BAD_CAST "00");
                        xmlAddChild(srAppList_Node, srItem_Node);
                }
		else if (!memcmp(&szMVTApplicationId[0], "A000000025", 10))
                {
                        xmlNewChild(srItem_Node, NULL, BAD_CAST "Name", BAD_CAST "AMEX");
                        xmlNewChild(srItem_Node, NULL, BAD_CAST "AID", BAD_CAST szMVTApplicationId);
                        xmlNewChild(srItem_Node, NULL, BAD_CAST "ASI", BAD_CAST "00");
                        xmlAddChild(srAppList_Node, srItem_Node);
                }
		else if (!memcmp(&szMVTApplicationId[0], _EMV_AID_TWIN_, _EMV_AID_TWIN_LEN_))
                {
                        xmlNewChild(srItem_Node, NULL, BAD_CAST "Name", BAD_CAST "NCCC");
                        xmlNewChild(srItem_Node, NULL, BAD_CAST "AID", BAD_CAST szMVTApplicationId);
                        xmlNewChild(srItem_Node, NULL, BAD_CAST "ASI", BAD_CAST "00");
                        xmlAddChild(srAppList_Node, srItem_Node);
                }
		else if (!memcmp(&szMVTApplicationId[0], "A00000015230", 12))
                {
                        xmlNewChild(srItem_Node, NULL, BAD_CAST "Name", BAD_CAST "DPAS");
                        xmlNewChild(srItem_Node, NULL, BAD_CAST "AID", BAD_CAST szMVTApplicationId);
                        xmlNewChild(srItem_Node, NULL, BAD_CAST "ASI", BAD_CAST "00");
                        xmlAddChild(srAppList_Node, srItem_Node);
                }
                else
                {
                        inAPPIndex --;
                        continue;
                }
        }
        
        /* TerminalConfig */
        /* 此為預設值 實際會先抓AppConfig */
        srTerminalConfig_Node = xmlNewNode(NULL, BAD_CAST "TerminalConfig");
        xmlAddChild(srConfig_Node, srTerminalConfig_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "TERMINAL CAPABILITIES");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "9F33");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
        xmlNodeAddContent(srItem_Node, BAD_CAST "E0F1C8");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "ADDITIONAL TERMINAL CAPABILITIES");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "9F40");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
        xmlNodeAddContent(srItem_Node, BAD_CAST "F000F0A001");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
	/* 9F1E "INTERFACE DEVICE (IFD) SERIAL NUMBER" 從xml檔中刪除，改在OnTerminalDataGet塞值 */
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "INTERFACE DEVICE (IFD) SERIAL NUMBER");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "9F1E");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "asc");
	memset(szSerialNumber, 0x00, sizeof(szSerialNumber));
	inFunc_GetSeriaNumber(szSerialNumber);
	
	/* 只取8到15位 第16位為檢查碼 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &szSerialNumber[7], 8);
		
        xmlNodeAddContent(srItem_Node, BAD_CAST szTemplate);
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "TRANSACTION CURRENCY CODE");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "5F2A");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
        xmlNodeAddContent(srItem_Node, BAD_CAST "0901");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "TERMINAL COUNTRY CODE");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "9F1A");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
        xmlNodeAddContent(srItem_Node, BAD_CAST "0158");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "TERMINAL TYPE");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "9F35");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
        xmlNodeAddContent(srItem_Node, BAD_CAST "22");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);

        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "Default TDOL");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC0");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
        xmlNodeAddContent(srItem_Node, BAD_CAST "9F0206");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "Default DDOL");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC1");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
        xmlNodeAddContent(srItem_Node, BAD_CAST "9F3704");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "App Version");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "9F09");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
        xmlNodeAddContent(srItem_Node, BAD_CAST "0001");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "Termian Floor Limit");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "9F1B");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
        xmlNodeAddContent(srItem_Node, BAD_CAST "000003E8");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "Threshold Value");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC4");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
        xmlNodeAddContent(srItem_Node, BAD_CAST "00000005");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "Target Percent");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC2");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
        xmlNodeAddContent(srItem_Node, BAD_CAST "20");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "Max Target Percent");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC3");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
        xmlNodeAddContent(srItem_Node, BAD_CAST "40");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "TAC Denial");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC6");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
        xmlNodeAddContent(srItem_Node, BAD_CAST "0000000000");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
       
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "TAC Online");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC7");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
        xmlNodeAddContent(srItem_Node, BAD_CAST "0000000000");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "TAC Default");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC8");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
        xmlNodeAddContent(srItem_Node, BAD_CAST "0000000000");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
        /* AppConfig */
        srAppConfig_Node = xmlNewNode(NULL, BAD_CAST "AppConfig");
        xmlAddChild(srConfig_Node, srAppConfig_Node);
        
        for (i = 0 ; ; i ++)
        {
                if (inLoadMVTRec(i) < 0)
                        break;

                memset(szMVTApplicationId, 0x00, sizeof(szMVTApplicationId));
                inGetMVTApplicationId(szMVTApplicationId);
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "AID: %s", szMVTApplicationId);
                        inLogPrintf(AT, szTemplate);
                }
		             
                if (!memcmp(&szMVTApplicationId[0], "A000000003", 10))
                {
                        srGroup_Node = xmlNewNode(NULL, BAD_CAST "Group");
                        xmlNewProp(srGroup_Node, BAD_CAST "name", BAD_CAST "VISA");
                        xmlNewProp(srGroup_Node, BAD_CAST "AID", BAD_CAST szMVTApplicationId);
                        xmlNewProp(srGroup_Node, BAD_CAST "ASI", BAD_CAST "00");
                        xmlAddChild(srAppConfig_Node, srGroup_Node);
			
			srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
			xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "App Version");
			xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "9F09");
			xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
			xmlNodeAddContent(srItem_Node, BAD_CAST "0084");
			xmlAddChild(srGroup_Node, srItem_Node);
                }
                else if (!memcmp(&szMVTApplicationId[0], "A000000004", 10))
                {
                        srGroup_Node = xmlNewNode(NULL, BAD_CAST "Group");
                        xmlNewProp(srGroup_Node, BAD_CAST "name", BAD_CAST "MasterCard");
                        xmlNewProp(srGroup_Node, BAD_CAST "AID", BAD_CAST szMVTApplicationId);
                        xmlNewProp(srGroup_Node, BAD_CAST "ASI", BAD_CAST "00");
                        xmlAddChild(srAppConfig_Node, srGroup_Node);
			
			srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
			xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "App Version");
			xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "9F09");
			xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
			xmlNodeAddContent(srItem_Node, BAD_CAST "0002");
			xmlAddChild(srGroup_Node, srItem_Node);
                }
                else if (!memcmp(&szMVTApplicationId[0], "A000000065", 10))
                {
                        srGroup_Node = xmlNewNode(NULL, BAD_CAST "Group");
                        xmlNewProp(srGroup_Node, BAD_CAST "name", BAD_CAST "JCB");
                        xmlNewProp(srGroup_Node, BAD_CAST "AID", BAD_CAST szMVTApplicationId);
                        xmlNewProp(srGroup_Node, BAD_CAST "ASI", BAD_CAST "00");
                        xmlAddChild(srAppConfig_Node, srGroup_Node);
			
			srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
			xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "App Version");
			xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "9F09");
			xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
			xmlNodeAddContent(srItem_Node, BAD_CAST "0200");
			xmlAddChild(srGroup_Node, srItem_Node);
                }
                else if (!memcmp(&szMVTApplicationId[0], "A000000333", 10))
                {
                        srGroup_Node = xmlNewNode(NULL, BAD_CAST "Group");
                        xmlNewProp(srGroup_Node, BAD_CAST "name", BAD_CAST "UICC");
                        xmlNewProp(srGroup_Node, BAD_CAST "AID", BAD_CAST szMVTApplicationId);
                        xmlNewProp(srGroup_Node, BAD_CAST "ASI", BAD_CAST "00");
                        xmlAddChild(srAppConfig_Node, srGroup_Node);
			
			srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
			xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "App Version");
			xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "9F09");
			xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
			xmlNodeAddContent(srItem_Node, BAD_CAST "0020");
			xmlAddChild(srGroup_Node, srItem_Node);
                }
		else if (!memcmp(&szMVTApplicationId[0], "A000000025", 10))
                {
                        srGroup_Node = xmlNewNode(NULL, BAD_CAST "Group");
                        xmlNewProp(srGroup_Node, BAD_CAST "name", BAD_CAST "AMEX");
                        xmlNewProp(srGroup_Node, BAD_CAST "AID", BAD_CAST szMVTApplicationId);
                        xmlNewProp(srGroup_Node, BAD_CAST "ASI", BAD_CAST "00");
                        xmlAddChild(srAppConfig_Node, srGroup_Node);
                }
		else if (!memcmp(&szMVTApplicationId[0], _EMV_AID_TWIN_, _EMV_AID_TWIN_LEN_))
                {
                        srGroup_Node = xmlNewNode(NULL, BAD_CAST "Group");
                        xmlNewProp(srGroup_Node, BAD_CAST "name", BAD_CAST "NCCC");
                        xmlNewProp(srGroup_Node, BAD_CAST "AID", BAD_CAST szMVTApplicationId);
                        xmlNewProp(srGroup_Node, BAD_CAST "ASI", BAD_CAST "00");
                        xmlAddChild(srAppConfig_Node, srGroup_Node);
                }
		else if (!memcmp(&szMVTApplicationId[0], "A00000015230", 12))
                {
                        srGroup_Node = xmlNewNode(NULL, BAD_CAST "Group");
                        xmlNewProp(srGroup_Node, BAD_CAST "name", BAD_CAST "DPAS");
                        xmlNewProp(srGroup_Node, BAD_CAST "AID", BAD_CAST szMVTApplicationId);
                        xmlNewProp(srGroup_Node, BAD_CAST "ASI", BAD_CAST "00");
                        xmlAddChild(srAppConfig_Node, srGroup_Node);
                }
                else
                {
                        continue;
                }

                
                /* 以下參數按照聯合EMVDef.txt排序 */
                /* EMV tag 塞空值會system Error 須判斷是否有值 */
                
                /* Terminal Capabilities */
                memset(szTerminalCapabilities, 0x00, sizeof(szTerminalCapabilities));
                inGetTerminalCapabilities(szTerminalCapabilities);
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "Terminal Capabilities: %s", szTerminalCapabilities);
                        inLogPrintf(AT, szTemplate);
                }
                
                if (strlen(szTerminalCapabilities) > 0)
                {
                        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "TERMINAL CAPABILITIES");
                        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "9F33");
                        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
                        xmlNodeAddContent(srItem_Node, BAD_CAST szTerminalCapabilities);
                        xmlAddChild(srGroup_Node, srItem_Node);
                }
                
                /* Terminal Type */
                memset(szTerminalType, 0x00, sizeof(szTerminalType));
                inGetMVTTerminalType(szTerminalType);
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "Terminal Type: %s", szTerminalType);
                        inLogPrintf(AT, szTemplate);
                }
                
                if (strlen(szTerminalType) > 0)
                {
                        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "TERMINAL TYPE");
                        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "9F35");
                        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
                        xmlNodeAddContent(srItem_Node, BAD_CAST szTerminalType);
                        xmlAddChild(srGroup_Node, srItem_Node);
                }

                /* Additional Terminal Capabilities */
                memset(szAdditionalTerminalCapabilities, 0x00, sizeof(szAdditionalTerminalCapabilities));
                inGetAdditionalTerminalCapabilities(szAdditionalTerminalCapabilities);
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "Additional Terminal Capabilities: %s", szAdditionalTerminalCapabilities);
                        inLogPrintf(AT, szTemplate);
                }
                
                if (strlen(szAdditionalTerminalCapabilities) > 0)
                {
                        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "ADDITIONAL TERMINAL CAPABILITIES");
                        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "9F40");
                        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
                        xmlNodeAddContent(srItem_Node, BAD_CAST szAdditionalTerminalCapabilities);
                        xmlAddChild(srGroup_Node, srItem_Node);
                }
                
                /* Terminal Country Code */
                memset(szTerminalCountryCode, 0x00, sizeof(szTerminalCountryCode));
                inGetMVTTerminalCountryCode(szTerminalCountryCode);
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "Terminal Country Code: %s", szTerminalCountryCode);
                        inLogPrintf(AT, szTemplate);
                }
                
                if (strlen(szTerminalCountryCode) > 0)
                {
                        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "TERMINAL COUNTRY CODE");
                        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "9F1A");
                        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
                        xmlNodeAddContent(srItem_Node, BAD_CAST szTerminalCountryCode);
                        xmlAddChild(srGroup_Node, srItem_Node);
                }
                
                /* Transaction Currency Code */
                memset(szTransactionCurrencyCode, 0x00, sizeof(szTransactionCurrencyCode));
                inGetMVTTransactionCurrencyCode(szTransactionCurrencyCode);
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "Transaction Currency Code: %s", szTransactionCurrencyCode);
                        inLogPrintf(AT, szTemplate);
                }
                
                if (strlen(szTransactionCurrencyCode) > 0)
                {
                        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "TRANSACTION CURRENCY CODE");
                        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "5F2A");
                        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
                        xmlNodeAddContent(srItem_Node, BAD_CAST szTransactionCurrencyCode);
                        xmlAddChild(srGroup_Node, srItem_Node);
                }
                
                /* Default TAC */
                memset(szDefaultTAC, 0x00, sizeof(szDefaultTAC));
                inGetDefaultTAC(szDefaultTAC);
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "Default TAC: %s", szDefaultTAC);
                        inLogPrintf(AT, szTemplate);
                }
                
                if (strlen(szDefaultTAC) > 0)
                {
                        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "TAC Default");
                        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC8");
                        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
                        xmlNodeAddContent(srItem_Node, BAD_CAST szDefaultTAC);
                        xmlAddChild(srGroup_Node, srItem_Node);
                }
                
                /* Online TAC  */
                memset(szOnlineTAC, 0x00, sizeof(szOnlineTAC));
                inGetOnlineTAC(szOnlineTAC);
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "Online TAC: %s", szOnlineTAC);
                        inLogPrintf(AT, szTemplate);
                }
                
                if (strlen(szOnlineTAC) > 0)
                {
                        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "TAC Online");
                        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC7");
                        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
                        xmlNodeAddContent(srItem_Node, BAD_CAST szOnlineTAC);
                        xmlAddChild(srGroup_Node, srItem_Node);
                }
                
                /* Denial TAC */
                memset(szDenialTAC, 0x00, sizeof(szDenialTAC));
                inGetDenialTAC(szDenialTAC);
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "Denial TAC: %s", szDenialTAC);
                        inLogPrintf(AT, szTemplate);
                }
                
                if (strlen(szDenialTAC) > 0)
                {
                        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "TAC Denial");
                        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC6");
                        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
                        xmlNodeAddContent(srItem_Node, BAD_CAST szDenialTAC);
                        xmlAddChild(srGroup_Node, srItem_Node);
                }
                
                /* Default TDOL */
                memset(szDefaultTDOL, 0x00, sizeof(szDefaultTDOL));
                inGetDefaultTDOL(szDefaultTDOL);
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "Default TDOL: %s", szDefaultTDOL);
                        inLogPrintf(AT, szTemplate);
                }
                
                if (strlen(szDefaultTDOL) > 0)
                {
                        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "Default TDOL");
                        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC0");
                        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
                        xmlNodeAddContent(srItem_Node, BAD_CAST szDefaultTDOL);
                        xmlAddChild(srGroup_Node, srItem_Node);
                }
                
                /* Default DDOL */
                memset(szDefaultDDOL, 0x00, sizeof(szDefaultDDOL));
                inGetDefaultDDOL(szDefaultDDOL);
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "Default DDOL: %s", szDefaultDDOL);
                        inLogPrintf(AT, szTemplate);
                }
                
                if (strlen(szDefaultDDOL) > 0)
                {
                        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "Default DDOL");
                        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC1");
                        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
                        xmlNodeAddContent(srItem_Node, BAD_CAST szDefaultDDOL);
                        xmlAddChild(srGroup_Node, srItem_Node);
                }
                
                /* EMV Floor Limit (4 Bytes)*/
                memset(szEMVFloorLimit, 0x00, sizeof(szEMVFloorLimit));
                inGetEMVFloorLimit(szEMVFloorLimit);
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &szEMVFloorLimit[2], 8);
                memset(szEMVFloorLimit, 0x00, sizeof(szEMVFloorLimit));
                sprintf(szEMVFloorLimit, "%08X", atoi(szTemplate));
                        
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "EMV Floor Limit: %s", szEMVFloorLimit);
                        inLogPrintf(AT, szTemplate);
                }
                
                if (strlen(szEMVFloorLimit) > 0)
                {
                        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "Termian Floor Limit");
                        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "9F1B");
                        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
                        xmlNodeAddContent(srItem_Node, BAD_CAST szEMVFloorLimit);
                        xmlAddChild(srGroup_Node, srItem_Node);
                }
                
                /* Random Selection Threshold */
                memset(szRandomSelectionThreshold, 0x00, sizeof(szRandomSelectionThreshold));
                inGetRandomSelectionThreshold(szRandomSelectionThreshold);
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &szRandomSelectionThreshold[2], 8);
                memset(szRandomSelectionThreshold, 0x00, sizeof(szRandomSelectionThreshold));
                sprintf(szRandomSelectionThreshold, "%08X", atoi(szTemplate));
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "Random Selection Threshold: %s", szRandomSelectionThreshold);
                        inLogPrintf(AT, szTemplate);
                }
                
                if (strlen(szRandomSelectionThreshold) > 0)
                {
                        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "Threshold Value");
                        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC4");
                        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
                        xmlNodeAddContent(srItem_Node, BAD_CAST szRandomSelectionThreshold);
                        xmlAddChild(srGroup_Node, srItem_Node);
                }
                
                /* Target Percent for Random Selection */
                memset(szTargetPercentforRandomSelection, 0x00, sizeof(szTargetPercentforRandomSelection));
                inGetTargetPercentforRandomSelection(szTargetPercentforRandomSelection);
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "Target Percent for Random Selection: %s", szTargetPercentforRandomSelection);
                        inLogPrintf(AT, szTemplate);
                }
                
                if (strlen(szTargetPercentforRandomSelection) > 0)
                {
                        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "Target Percent");
                        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC2");
                        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
                        xmlNodeAddContent(srItem_Node, BAD_CAST szTargetPercentforRandomSelection);
                        xmlAddChild(srGroup_Node, srItem_Node);
                }
                
                /* Max Target Percent for Random Selection */
                memset(szMaxTargetPercentforRandomSelection, 0x00, sizeof(szMaxTargetPercentforRandomSelection));
                inGetMaxTargetPercentforRandomSelection(szMaxTargetPercentforRandomSelection);
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "Max Target Percent for Random Selection: %s", szMaxTargetPercentforRandomSelection);
                        inLogPrintf(AT, szTemplate);
                }
                
                if (strlen(szMaxTargetPercentforRandomSelection) > 0)
                {
                        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "Max Target Percent");
                        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC3");
                        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
                        xmlNodeAddContent(srItem_Node, BAD_CAST szMaxTargetPercentforRandomSelection);
                        xmlAddChild(srGroup_Node, srItem_Node);
                }
                
                /* Merchant Category Code */
                if (!memcmp(&szMVTApplicationId[0], "A000000004", 10))
                {
                        memset(szMerchantCategoryCode, 0x00, sizeof(szMerchantCategoryCode));
                        inGetMVTMerchantCategoryCode(szMerchantCategoryCode);
                        
                        if (ginDebug == VS_TRUE)
                        {
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                sprintf(szTemplate, "Merchant Category Code: %s", szMerchantCategoryCode);
                                inLogPrintf(AT, szTemplate);
                        }
                        
                        if (strlen(szMerchantCategoryCode) > 0)
                        {
                                srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                                xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "MCC");
                                xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "9F15");
                                xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
                                xmlNodeAddContent(srItem_Node, BAD_CAST szMerchantCategoryCode);
                                xmlAddChild(srGroup_Node, srItem_Node);
                        }
                }
                
                /* Transaction Category Code */
                if (!memcmp(&szMVTApplicationId[0], "A000000004", 10))
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetMVTTransactionCategoryCode(szTemplate);
                        /* TMS帶下來是字元 須轉換 */        
                        memset(szTransactionCategoryCode, 0x00, sizeof(szTransactionCategoryCode));
                        sprintf(szTransactionCategoryCode, "%X", szTemplate[0]);
                        
                        if (ginDebug == VS_TRUE)
                        {
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                sprintf(szTemplate, "Transaction Category Code: %s", szTransactionCategoryCode);
                                inLogPrintf(AT, szTemplate);
                        }
                        
                        if (strlen(szTransactionCategoryCode) > 0)
                        {
                                srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                                xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "TCC");
                                xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "9F53");
                                xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
                                xmlNodeAddContent(srItem_Node, BAD_CAST szTransactionCategoryCode);
                                xmlAddChild(srGroup_Node, srItem_Node);
                        }
                }
        }
        
        /* 儲存xml檔 xml檔現在會自動換行了 */
        inRetVal = inXML_SaveFile(szConfigFileName, &srDoc, "utf-8", 1);
        
        if (ginDebug == VS_TRUE)
        {
                sprintf(szTemplate, "inRetVal = %d", inRetVal);
                inLogPrintf(AT, szTemplate);
        }
        
        
        /* 清空佔用記憶體 */
	inXML_End(&srDoc);

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inEMVXML_Create_EMVConfigXml()_END");
        
        return (VS_SUCCESS);
}

/*
Function        :inEMVXML_Create_CTLSConfigXml
Date&Time       :2016/3/22 下午 5:35
Describe        :組感應使用的xml檔案
*/
int inEMVXML_Create_CTLSConfigXml(char* szConfigFileName)
{
        xmlDocPtr       srDoc = NULL;
        xmlNodePtr      srRoot_Node = NULL, srCLConfig_Node = NULL, srTagCombination_Node = NULL, srCAPK_Node = NULL, 
                        srParametersConfig_Node = NULL, srGroup_Node = NULL, srItem_Node = NULL;
        SHA_CTX         srSHA;
        int		i, j, inRetVal, inPacketSizes = 0, inOWSize = 0;
	int		inTagLen = 0;
        char		szTemplate[64 + 1], szTemplate1[64 + 1], szBuffer[256 + 1], szBuffer_2[256 + 1];
        unsigned char   uszBCD[512 + 1];
        /* CAPK用 */
        int		inBCDLen, inCAPK_DataLen;
        char		szCAPKId[10 + 1], szCAPKId_BAK[10 + 1];
        char		szCAPKIndex[2 + 1];
        char		szCAPKKeyModulus[496 + 1];
        char		szCAPKExponent[6 + 1];
        char		szCAPKIdIndex[20 + 1];
        char		szCAPKData[512 + 1];
        char		szCAPKHash[40 + 1];
        unsigned char   uszHash[32 + 1];
        /* VWT參數用 */
        long		lnCVMRequireLimit, lnContactlessFloorLimit, lnContactlessAPIDFloorLimit;
        char		szTagContent[1024 + 1];
	char		szDF8F4FData[300 + 1] = {0};
        char		szTerminalCapabilities[6 + 1]; 
        char		szAdditionalTerminalCapabilities[10 + 1];
        char		szMVTApplicationId[16 + 1];
        char		szTerminalType[2 + 1];
        char		szCVMRequirement[2 + 1];
//        char		szVLPSupportIndicator[2 + 1];
        char		szTerminalCountryCode[4 + 1];
        char		szTransactionCurrencyCode[4 + 1];
        char		szTransactionType[2 + 1];
        char		szContactlessTransactionLimit[12 + 1];
        char		szCVMRequiredLimit[12 + 1];
        char		szContactlessFloorLimit[12 + 1];
//        char		szEnhancedDDAVersionNum[2 + 1];
        char		szDisplayOfflineFunds[2 + 1];
        char		szTerminalTransactionQualifier[8 + 1];
//        char		szPaypassTerminalCapabilities[6 + 1];
//        char		szPaypassTerminalCountryCode[4 + 1];
//        char		szPaypassTransactionCurrencyCode[4 + 1];
        char		szPaypassDefaultTAC[10 + 1];
        char		szPaypassDenialTAC[10 + 1];
        char		szPaypassOnlineTAC[10 + 1];
        char		szPaypassDefaultTDOL[6 + 1];
        char		szDefaultDDOL[6 + 1];
        char		szPaypassEMVFloorLimit[12 + 1];
        char		szPaypassRandomSelectionThreshold[12 + 1];
//        char		szPaypassTargetPercentforRandomSelection[2 + 1];
//        char		szPaypassMaxTargetPercentforRandomSelection[2 + 1];
        char		szPaypassCVMRequiredLimit[12 + 1];
        char		szMerchantCategoryCode[4 + 1];
//        char		szTransactionCategoryCode[2 + 1];
        char		szCombinationOption[4 + 1];
        char		szTerminalInterchangeProfile[6 + 1];
        char		szPayWaveAPID[32 + 1];
        char		szPayWaveAPIDContactlessTransactionLimit[12 + 1];
        char		szPayWaveAPIDCVMRequiredLimit[12 + 1];
        char		szPayWaveAPIDContactlessFloorLimit[12 + 1];
	char		szBCD[20 + 1] = {0};
	char		szMVTIndex[2 + 1] = {0};
        char            szCustomerIndicator[3 + 1] = {0};
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inEMVXML_Create_CTLSConfigXml()_START");
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
        inGetCustomIndicator(szCustomerIndicator);

        /* 先預LOAD VWT及EST 失敗 Return Error */
        if (inLoadVWTRec(0) < 0)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inLoadVWTRec(0)Error!!");
                
                return (VS_ERROR);
        }
        
        if (inLoadESTRec(0) < 0)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inLoadESTRec(0)Error!!");
                
                return (VS_ERROR);
        }
        
        /* 建立一個新的檔案 */
        srDoc = xmlNewDoc(BAD_CAST "1.0");
        srRoot_Node = xmlNewNode(NULL, BAD_CAST "configurationDescriptor");
        xmlNewProp(srRoot_Node, BAD_CAST "version", BAD_CAST "01");    
        xmlDocSetRootElement (srDoc, srRoot_Node);
        
        /* 目前只有一個Config 預設true */
        srCLConfig_Node = xmlNewNode(NULL, BAD_CAST "CLConfig");
        xmlNewProp(srCLConfig_Node, BAD_CAST "index", BAD_CAST "01");
        xmlNewProp(srCLConfig_Node, BAD_CAST "active", BAD_CAST "true" );
        xmlAddChild(srRoot_Node, srCLConfig_Node);
        
        /* TagCombination */
        srTagCombination_Node = xmlNewNode(NULL, BAD_CAST "TagCombination");
        xmlAddChild(srCLConfig_Node, srTagCombination_Node);
        
        for (i = 0 ;; i ++)
        {
                /* (00=Paywave，01=JSpeedy，02=Paypass，03=QP_Debit，04=QP_Credit，05=QP_QCredit, 06=AE ExpressPay, 07=NCCC, 08=D-PAS) */
                if (inLoadVWTRec(i) < 0)
                        break;
				
                for (j = 0 ;; j ++)
                {
                        if (inLoadMVTRec(j) < 0)
			{
				memset(szMVTApplicationId, 0x00, sizeof(szMVTApplicationId));
                                break;
			}
                        
                        memset(szMVTApplicationId, 0x00, sizeof(szMVTApplicationId));
                        inGetMVTApplicationId(szMVTApplicationId);
			memset(szMVTIndex, 0x00, sizeof(szMVTIndex));
			inGetApplicationIndex(szMVTIndex);
			
                        /* 依照聯合規格Contactless Scheme Index順序是固定的 */
                        if (i == _PAYWAVE_VWT_INDEX_)
                        {
                                /* Paywave */
                                if (memcmp(szMVTIndex, _MVT_VISA_NCCC_INDEX_, strlen(_MVT_VISA_NCCC_INDEX_)) == 0)
                                {
                                        srGroup_Node = xmlNewNode(NULL, BAD_CAST "Group");
                                        xmlNewProp(srGroup_Node, BAD_CAST "AID", BAD_CAST szMVTApplicationId);
                                        xmlNewProp(srGroup_Node, BAD_CAST "KernelID", BAD_CAST "03");
                                        xmlNewProp(srGroup_Node, BAD_CAST "TxnType", BAD_CAST "00");
                                        xmlAddChild(srTagCombination_Node, srGroup_Node);
                                        break;
                                }
                                else
                                        continue;
                        }
                        else if (i == _JSPEEDY_VWT_INDEX_)
                        {
                                /* JSpeedy */
				if (memcmp(szMVTIndex, _MVT_JSMART_NCCC_INDEX_, strlen(_MVT_JSMART_NCCC_INDEX_)) == 0)
                                {
                                        srGroup_Node = xmlNewNode(NULL, BAD_CAST "Group");
                                        xmlNewProp(srGroup_Node, BAD_CAST "AID", BAD_CAST szMVTApplicationId);
                                        /* JCB的Kernel ID 未知 */
                                        xmlNewProp(srGroup_Node, BAD_CAST "KernelID", BAD_CAST "05");
                                        xmlNewProp(srGroup_Node, BAD_CAST "TxnType", BAD_CAST "00");
                                        xmlAddChild(srTagCombination_Node, srGroup_Node);
                                        break;
                                }
                                else
                                        continue;
                        }
                        else if (i == _PAYPASS_VWT_INDEX_)
                        {
                                /* Paypass */
				if (memcmp(szMVTIndex, _MVT_MCHIP_NCCC_INDEX_, strlen(_MVT_MCHIP_NCCC_INDEX_)) == 0)
                                {
                                        srGroup_Node = xmlNewNode(NULL, BAD_CAST "Group");
                                        xmlNewProp(srGroup_Node, BAD_CAST "AID", BAD_CAST szMVTApplicationId);
                                        xmlNewProp(srGroup_Node, BAD_CAST "KernelID", BAD_CAST "02");
                                        xmlNewProp(srGroup_Node, BAD_CAST "TxnType", BAD_CAST "00");
                                        xmlAddChild(srTagCombination_Node, srGroup_Node);
                                        break;
                                }
                                else
                                        continue;
                        }
                        else if (i == _QP_DEBIT_VWT_INDEX_)
                        {
                                /* QuickPass_Debit */
				if (memcmp(szMVTIndex, _MVT_CUP_DEBIT_NCCC_INDEX_, strlen(_MVT_CUP_DEBIT_NCCC_INDEX_)) == 0)
                                {
                                        srGroup_Node = xmlNewNode(NULL, BAD_CAST "Group");
                                        xmlNewProp(srGroup_Node, BAD_CAST "AID", BAD_CAST szMVTApplicationId);
                                        xmlNewProp(srGroup_Node, BAD_CAST "KernelID", BAD_CAST "07");
                                        xmlNewProp(srGroup_Node, BAD_CAST "TxnType", BAD_CAST "00");
                                        xmlAddChild(srTagCombination_Node, srGroup_Node);
                                        break;
                                }
                                else
                                        continue;
                        }
                        else if (i == _QP_CREDIT_VWT_INDEX_)
                        {
                                /* QuickPass_Credit */
				if (memcmp(szMVTIndex, _MVT_CUP_CREDIT_NCCC_INDEX_, strlen(_MVT_CUP_CREDIT_NCCC_INDEX_)) == 0)
                                {
                                        srGroup_Node = xmlNewNode(NULL, BAD_CAST "Group");
                                        xmlNewProp(srGroup_Node, BAD_CAST "AID", BAD_CAST szMVTApplicationId);
                                        xmlNewProp(srGroup_Node, BAD_CAST "KernelID", BAD_CAST "07");
                                        xmlNewProp(srGroup_Node, BAD_CAST "TxnType", BAD_CAST "00");
                                        xmlAddChild(srTagCombination_Node, srGroup_Node);
                                        break;
                                }
                                else
                                        continue;
                        }
                        else if (i == _QP_QUASI_CREDIT_VWT_INDEX_)
                        {
                                /* QuickPass_QCredit */
				if (memcmp(szMVTIndex, _MVT_CUP_QUASI_CREDIT_NCCC_INDEX_, strlen(_MVT_CUP_QUASI_CREDIT_NCCC_INDEX_)) == 0)
                                {
                                        srGroup_Node = xmlNewNode(NULL, BAD_CAST "Group");
                                        xmlNewProp(srGroup_Node, BAD_CAST "AID", BAD_CAST szMVTApplicationId);
                                        xmlNewProp(srGroup_Node, BAD_CAST "KernelID", BAD_CAST "07");
                                        xmlNewProp(srGroup_Node, BAD_CAST "TxnType", BAD_CAST "00");
                                        xmlAddChild(srTagCombination_Node, srGroup_Node);
                                        break;
                                }
                                else
                                        continue;
                        }
			else if (i == _AE_EXPRESSPAY_VWT_INDEX_)
                        {
                                /* Expresspay */
				if (memcmp(szMVTIndex, _MVT_AEIPS_NCCC_INDEX_, strlen(_MVT_AEIPS_NCCC_INDEX_)) == 0)
                                {
                                        srGroup_Node = xmlNewNode(NULL, BAD_CAST "Group");
                                        xmlNewProp(srGroup_Node, BAD_CAST "AID", BAD_CAST szMVTApplicationId);
                                        xmlNewProp(srGroup_Node, BAD_CAST "KernelID", BAD_CAST "04");
                                        xmlNewProp(srGroup_Node, BAD_CAST "TxnType", BAD_CAST "00");
                                        xmlAddChild(srTagCombination_Node, srGroup_Node);
                                        break;
                                }
                                else
                                        continue;
                        }
			else if (i == _NCCC_VWT_INDEX_)
                        {
                                /* NCCC */
				if (memcmp(szMVTIndex, _MVT_NCCC_NCCC_INDEX_, strlen(_MVT_NCCC_NCCC_INDEX_)) == 0)
                                {
                                        srGroup_Node = xmlNewNode(NULL, BAD_CAST "Group");
                                        xmlNewProp(srGroup_Node, BAD_CAST "AID", BAD_CAST szMVTApplicationId);
                                        xmlNewProp(srGroup_Node, BAD_CAST "KernelID", BAD_CAST "06");
                                        xmlNewProp(srGroup_Node, BAD_CAST "TxnType", BAD_CAST "00");
                                        xmlAddChild(srTagCombination_Node, srGroup_Node);
                                        break;
                                }
                                else
                                        continue;
                        }
			else if (i == _D_PAS_VWT_INDEX_)
                        {
                                /* DISCOVER */
				if (memcmp(szMVTIndex, _MVT_D_PAS_NCCC_INDEX_, strlen(_MVT_D_PAS_NCCC_INDEX_)) == 0)
                                {
                                        srGroup_Node = xmlNewNode(NULL, BAD_CAST "Group");
                                        xmlNewProp(srGroup_Node, BAD_CAST "AID", BAD_CAST szMVTApplicationId);
                                        xmlNewProp(srGroup_Node, BAD_CAST "KernelID", BAD_CAST "06");
                                        xmlNewProp(srGroup_Node, BAD_CAST "TxnType", BAD_CAST "00");
                                        xmlAddChild(srTagCombination_Node, srGroup_Node);
                                        break;
                                }
                                else
                                        continue;
                        }
                        else
                        {
                                memset(szMVTApplicationId, 0x00, sizeof(szMVTApplicationId));
                                break;
                        }
                }
                
                /* 組感應的tag 各AID不同 分開組 */
                /* VISA Paywave */
                if (!memcmp(&szMVTApplicationId[0], "A000000003", 10))
                {
                        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "tlv");
                        memset(szTagContent, 0x00, sizeof(szTagContent));
                        inPacketSizes = 0;

                        /* 9F66-04 Terminal Transaction Qualifier */
                        strcat(szTagContent, "9F66");
                        inPacketSizes += 4;
                        memset(szTerminalTransactionQualifier, 0x00, sizeof(szTerminalTransactionQualifier));
                        inGetTerminalTransactionQualifier(szTerminalTransactionQualifier);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalTransactionQualifier) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTerminalTransactionQualifier);
                        inPacketSizes += strlen(szTerminalTransactionQualifier);
                        
			/* 9F33 Terminal Capabilities */
                        strcat(szTagContent, "9F33");
                        inPacketSizes += 4;
                        memset(szTerminalCapabilities, 0x00, sizeof(szTerminalCapabilities));
			inGetPaypassTerminalCapabilities(szTerminalCapabilities);
			/* 沒值或空值(判斷第一個為空白) */
			if (strlen(szTerminalCapabilities) == 0	|| memcmp(" ", szTerminalCapabilities, 1) == 0)
			{
				/* 空值抓接觸式 */
				inGetTerminalCapabilities(szTerminalCapabilities);
			}
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalCapabilities) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTerminalCapabilities);
                        inPacketSizes += strlen(szTerminalCapabilities);
			
                        /* 9F35-01 Terminal Type */
			memset(szTerminalType, 0x00, sizeof(szTerminalType));
			inGetPaypassTerminalType(szTerminalType);
			if (strlen(szTerminalType) == 0	|| memcmp(" ", szTerminalType, 1) == 0)
			{
				/* VWT沒值改抓回MVT，沒填Terminal Type Kernel會無法運行 */
				memset(szTerminalType, 0x00, sizeof(szTerminalType));
				inGetMVTTerminalType(szTerminalType);
				
				strcat(szTagContent, "9F35");
				inPacketSizes += 4;
				
				sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalType) / 2));
				inPacketSizes += 2;
				
				if (strlen(szTerminalType) > 0)
				{
					strcat(szTagContent, szTerminalType);
					inPacketSizes += strlen(szTerminalType);
				}
			}
			else
			{
				strcat(szTagContent, "9F35");
				inPacketSizes += 4;
				strcat(szTagContent, "01");
				inPacketSizes += 2;

				/* 強制縮減長度為2，避免意外 */
				szTerminalType[2] = 0x00;
				strcat(szTagContent, szTerminalType);
				inPacketSizes += strlen(szTerminalType);
			}

                        /* 9F40 Additional Terminal Capabilities */
                        strcat(szTagContent, "9F40");
                        inPacketSizes += 4;
                        memset(szAdditionalTerminalCapabilities, 0x00, sizeof(szAdditionalTerminalCapabilities));
                        inGetAdditionalTerminalCapabilities(szAdditionalTerminalCapabilities);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szAdditionalTerminalCapabilities) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szAdditionalTerminalCapabilities);
                        inPacketSizes += strlen(szAdditionalTerminalCapabilities);

                        /* 9F09 AppVersion */
                        
                        /* 9F1A-02 Terminal Country Code */
                        strcat(szTagContent, "9F1A");
                        inPacketSizes += 4;
                        memset(szTerminalCountryCode, 0x00, sizeof(szTerminalCountryCode));
                        inGetTerminalCountryCode(szTerminalCountryCode);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalCountryCode) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTerminalCountryCode);
                        inPacketSizes += strlen(szTerminalCountryCode);
			
			/* 9F1B EMV Floor Limit 4Bytes */
                        memset(szContactlessFloorLimit, 0x00, sizeof(szContactlessFloorLimit));
                        inGetContactlessFloorLimit(szContactlessFloorLimit);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        
                        // Tusin_071
                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_071_FORCE_ONLINE_, _CUSTOMER_INDICATOR_SIZE_))
                            memcpy(&szTemplate[0], "00000000", 8);
                        else
                            memcpy(&szTemplate[0], &szContactlessFloorLimit[2], 8);
                        
                        memset(szTemplate1, 0x00, sizeof(szTemplate1));
			/* the currency exponent is 2，所以需要乘上100 */
                        sprintf(szTemplate1, "%X", atoi(szTemplate) * 100);
                        inFunc_PAD_ASCII(szTemplate1, szTemplate1, '0', 8, _PADDING_LEFT_);
                        strcat(szTagContent, "9F1B");
                        inPacketSizes += 4;
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen(szTemplate1) / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, szTemplate1);
                        inPacketSizes += strlen(szTemplate1);

                        /* 5F2A-02 Transaction Currcncy Code */
                        strcat(szTagContent, "5F2A");
                        inPacketSizes += 4;
                        memset(szTransactionCurrencyCode, 0x00, sizeof(szTransactionCurrencyCode));
                        inGetVWTTransactionCurrencyCode(szTransactionCurrencyCode);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTransactionCurrencyCode) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTransactionCurrencyCode);
                        inPacketSizes += strlen(szTransactionCurrencyCode);

                        /* DF00-06 Contactless Transaction Limit */
                        strcat(szTagContent, "DF00");
                        inPacketSizes += 4;
                        memset(szContactlessTransactionLimit, 0x00, sizeof(szContactlessTransactionLimit));
                        inGetContactlessTransactionLimit(szContactlessTransactionLimit);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szContactlessTransactionLimit) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szContactlessTransactionLimit);
                        inPacketSizes += strlen(szContactlessTransactionLimit);

			/* If the amount is equal to or large than 2000, this this transaction shall perform the CVM. 
			 FFh - Bypass this check
			 Others - Ex : 000000200000 */
                        /* DF01-06 Reader CVM Required Limit */
                        memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
                        inGetCVMRequiredLimit(szCVMRequiredLimit);
                        lnCVMRequireLimit = atoi(szCVMRequiredLimit);
                        
                        memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
                        sprintf(szCVMRequiredLimit, "%ld", lnCVMRequireLimit);
                        inFunc_PAD_ASCII(szCVMRequiredLimit, szCVMRequiredLimit, '0', 12, _PADDING_LEFT_);
                        strcat(szTagContent, "DF01");
                        inPacketSizes += 4;
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szCVMRequiredLimit) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szCVMRequiredLimit);
                        inPacketSizes += strlen(szCVMRequiredLimit);

			/* If the amount is equal to or large than 1000, this transaction shall go online. 
			 FFh - Bypass this check
			 Others - Ex : 000000100000 */
                        /* DF02-06 Reader Contactless Floor Limit */
                        memset(szContactlessFloorLimit, 0x00, sizeof(szContactlessFloorLimit));
                        inGetContactlessFloorLimit(szContactlessFloorLimit);
                        
                        // Tusin_071
                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_071_FORCE_ONLINE_, _CUSTOMER_INDICATOR_SIZE_))
                            lnContactlessFloorLimit = 0;
                        else
                            lnContactlessFloorLimit = atoi(szContactlessFloorLimit);
                        
			/* 條件是larger，所以要減1元 */
                        if (lnContactlessFloorLimit > 0)
                        {
                                /* 如果金額已經為0時不需再減1元 */
                                lnContactlessFloorLimit -= 100;
                        }
                        
                        memset(szContactlessFloorLimit, 0x00, sizeof(szContactlessFloorLimit));
                        sprintf(szContactlessFloorLimit, "%ld", lnContactlessFloorLimit);
                        inFunc_PAD_ASCII(szContactlessFloorLimit, szContactlessFloorLimit, '0', 12, _PADDING_LEFT_);
                        strcat(szTagContent, "DF02");
                        inPacketSizes += 4;
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szContactlessFloorLimit) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szContactlessFloorLimit);
                        inPacketSizes += strlen(szContactlessFloorLimit);

                        /* DF25 Status Check
                           FFh - Status Check is disable.
                        */
                        strcat(szTagContent, "DF2501FF");
                        inPacketSizes += strlen("DF2501FF");
                        
                        /* DF8F4B Amount 0 Check
                           FFh - Amount 0 Check is disable.
                        */
                        strcat(szTagContent, "DF8F4B01FF");
                        inPacketSizes += strlen("DF8F4B01FF");
                        
                        /* DF05-01 Display Offline Available Fund */
                        strcat(szTagContent, "DF05");
                        inPacketSizes += 4;
                        memset(szDisplayOfflineFunds, 0x00, sizeof(szDisplayOfflineFunds));
                        inGetDisplayOfflineFunds(szDisplayOfflineFunds);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szDisplayOfflineFunds) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szDisplayOfflineFunds);
                        inPacketSizes += strlen(szDisplayOfflineFunds);
                        
                        /* DF21 Visa MSD Track1 Enable Indicator */
                        strcat(szTagContent, "DF210101");
                        inPacketSizes += strlen("DF210101");
                        
                        /* DF22 Visa MSD Track2 Enable Indicator */
                        strcat(szTagContent, "DF220101");
                        inPacketSizes += strlen("DF220101");
                        
                        /* DF29 Visa MSD CVN17 Enable Indicator */
                        strcat(szTagContent, "DF290101");
                        inPacketSizes += strlen("DF290101");
                        
                        
                        /* DRL overwrite tag */       
                        /* DF24 DRL Enable/Disable Indicator */
                        /*
                           set DF24 to 01h : Enable DRL function
                           set DF24 to 00h : Disable DRL function (Default)
                         */
                        strcat(szTagContent, "DF24");
                        inPacketSizes += 4;
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen("00") / 2));
                        inPacketSizes += 2;
                        
                        memset(szPayWaveAPID, 0x00, sizeof(szPayWaveAPID));
                        inGetPayWaveAPID(szPayWaveAPID);

                        if (strlen(szPayWaveAPID) > 0)
                                strcat(szTagContent, "01");
                        else
                                strcat(szTagContent, "00");
                        
                        inPacketSizes += strlen("00");
                        
                        if (strlen(szPayWaveAPID) > 0)
                        {
                                /* DF23 DRL overwrite tag */
                                /* The format of DRL overwrite tag : APID Len 1 + APID 1 + OWdata len 1 + OWdata 1 + APID Len 2 + APID 2 + OWdata len 2 + OWdata 2 + ....+ APID Len n + APID n + OWdata len n + OWdata n */
                                memset(szBuffer, 0x00, sizeof(szBuffer));
                                /* APID Len */
                                sprintf(&szBuffer[0], "%02X", (strlen(szPayWaveAPID) / 2));
                                inOWSize += 2;
                                /* APID */
                                strcat(szBuffer, szPayWaveAPID);
                                inOWSize += strlen(szPayWaveAPID);
                                /* 先組OWdata */
                                memset(szBuffer_2, 0x00, sizeof(szBuffer_2));
                                /* DF25 Status Check
                                   FFh - Status Check is disable.
                                */
                                strcat(szBuffer_2, "DF2501FF");
                                /* DF8F4B Amount 0 Check
                                   FFh - Amount 0 Check is disable.
                                */
                                strcat(szBuffer_2, "DF8F4B01FF");
                                /* DF00-06 Contactless Transaction Limit */
                                strcat(szBuffer_2, "DF0006");
                                memset(szPayWaveAPIDContactlessTransactionLimit, 0x00, sizeof(szPayWaveAPIDContactlessTransactionLimit));
                                inGetPayWaveAPIDContactlessTransactionLimit(szPayWaveAPIDContactlessTransactionLimit);
                                strcat(szBuffer_2, szPayWaveAPIDContactlessTransactionLimit);
                                /* DF01-06 Reader CVM Required Limit */
                                strcat(szBuffer_2, "DF0106");
                                memset(szPayWaveAPIDCVMRequiredLimit, 0x00, sizeof(szPayWaveAPIDCVMRequiredLimit));
                                inGetPayWaveAPIDCVMRequiredLimit(szPayWaveAPIDCVMRequiredLimit);
                                strcat(szBuffer_2, szPayWaveAPIDCVMRequiredLimit);
                                /* DF02-06 Reader Contactless Floor Limit */
                                strcat(szBuffer_2, "DF0206");
                                memset(szPayWaveAPIDContactlessFloorLimit, 0x00, sizeof(szPayWaveAPIDContactlessFloorLimit));
                                inGetPayWaveAPIDContactlessFloorLimit(szPayWaveAPIDContactlessFloorLimit);
                                lnContactlessAPIDFloorLimit = atoi(szPayWaveAPIDContactlessFloorLimit);
                                
                                if (lnContactlessAPIDFloorLimit > 0)
                                {
                                        /* 如果金額已經為0時不需再減1元 */
                                        lnContactlessAPIDFloorLimit -= 100;
                                }
                                
                                memset(szPayWaveAPIDContactlessFloorLimit, 0x00, sizeof(szPayWaveAPIDContactlessFloorLimit));
                                sprintf(szPayWaveAPIDContactlessFloorLimit, "%ld", lnContactlessAPIDFloorLimit);
                                inFunc_PAD_ASCII(szPayWaveAPIDContactlessFloorLimit, szPayWaveAPIDContactlessFloorLimit, '0', 12, _PADDING_LEFT_);
                                strcat(szBuffer_2, szPayWaveAPIDContactlessFloorLimit);
                                /* OWdata len */
                                sprintf(&szBuffer[inOWSize], "%02X", (strlen(szBuffer_2) / 2));
                                inOWSize += 2;
                                /* 算好長度後再把szBuffer_2組到OWdata */
                                strcat(szBuffer, szBuffer_2);
                                /* 組DF23 DRL overwrite tag*/
                                strcat(szTagContent, "DF23");
                                inPacketSizes += 4;
                                sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szBuffer) / 2));
                                inPacketSizes += 2;
                                strcat(szTagContent, szBuffer);
                                inPacketSizes += strlen(szBuffer);
                        }
			
			/* DF8F49 Upload DOL */
			/* The format of Tag Upload DOL is (FFC3 + DOL Data Length + DOL Data */
			/* Tag1 + 0x00 + Tag2 + 0x00 + .... + Tagn + 0x00 */
			strcat(szTagContent, "DF8F49");
			inPacketSizes += 6;
			
			/* DF8F49內容 */
			/* 【需求單 - 106252】	DCC交易新增感應及BIN11碼需求 新增5F28 9F42 by Russell 2020/2/20 上午 11:56 */
			/* 新增9F5A，用來判斷APID用 by Russell 2022/1/27 下午 1:56 */
			memset(szDF8F4FData, 0x00, sizeof(szDF8F4FData));
			strcpy(szDF8F4FData, "50005A005F20005F2A005F28005F3400820084008A0095009A009B009C009F02009F03009F10009F1A009F1E009F26009F27009F33009F34009F35009F36009F37009F42009F5A009F6E00");
			
			/* 全部長度(含FFC3) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%02X", ((strlen("FFC3") + 2 + strlen(szDF8F4FData)) / 2));
			strcat(szTagContent, szTemplate);
			inPacketSizes += 2;
			
			strcat(szTagContent, "FFC3");
			inPacketSizes += 4;
			
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%02X", (strlen(szDF8F4FData) / 2));
			strcat(szTagContent, szTemplate);
			inPacketSizes += 2;
                        
			strcat(szTagContent, szDF8F4FData);
			inPacketSizes += strlen(szDF8F4FData);

                        xmlNodeAddContent(srItem_Node, BAD_CAST szTagContent);
                        xmlAddChild(srGroup_Node, srItem_Node);
                }
                /* JSpeedy */
                else if (!memcmp(&szMVTApplicationId[0], "A000000065", 10))
                {
                        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "tlv");
                        memset(szTagContent, 0x00, sizeof(szTagContent));
                        inPacketSizes = 0;
                        
			/* 9F33-03 Terminal Capability */
                        strcat(szTagContent, "9F33");
                        inPacketSizes += 4;
                        memset(szTerminalCapabilities, 0x00, sizeof(szTerminalCapabilities));
                        inGetPaypassTerminalCapabilities(szTerminalCapabilities);
			/* 沒值或空值(判斷第一個為空白) */
			if (strlen(szTerminalCapabilities) == 0	|| memcmp(" ", szTerminalCapabilities, 1) == 0)
			{
				/* 空值抓接觸式 */
				inGetTerminalCapabilities(szTerminalCapabilities);
			}
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen(szTerminalCapabilities) / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, szTerminalCapabilities);
                        inPacketSizes += strlen(szTerminalCapabilities);
			
			/* 9F35-01 Terminal Type */
			memset(szTerminalType, 0x00, sizeof(szTerminalType));
			inGetPaypassTerminalType(szTerminalType);
                        if (strlen(szTerminalType) == 0	|| memcmp(" ", szTerminalType, 1) == 0)
			{
				/* VWT沒值改抓回MVT，沒填Terminal Type Kernel會無法運行 */
				memset(szTerminalType, 0x00, sizeof(szTerminalType));
				inGetMVTTerminalType(szTerminalType);
				
				strcat(szTagContent, "9F35");
				inPacketSizes += 4;
				
				sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalType) / 2));
				inPacketSizes += 2;
				
				if (strlen(szTerminalType) > 0)
				{
					strcat(szTagContent, szTerminalType);
					inPacketSizes += strlen(szTerminalType);
				}
			}
			else
			{
				strcat(szTagContent, "9F35");
				inPacketSizes += 4;
				strcat(szTagContent, "01");
				inPacketSizes += 2;

				/* 強制縮減長度為2，避免意外 */
				szTerminalType[2] = 0x00;
				strcat(szTagContent, szTerminalType);
				inPacketSizes += strlen(szTerminalType);
			}
                        
                       /* 9F40 Additional Terminal Capabilities */
                        strcat(szTagContent, "9F40");
                        inPacketSizes += 4;
                        memset(szAdditionalTerminalCapabilities, 0x00, sizeof(szAdditionalTerminalCapabilities));
                        inGetAdditionalTerminalCapabilities(szAdditionalTerminalCapabilities);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen(szAdditionalTerminalCapabilities) / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, szAdditionalTerminalCapabilities);
                        inPacketSizes += strlen(szAdditionalTerminalCapabilities);
                        
                        /* 9F09 AppVersion */
                        
                        /* 9F1A-02 Terminal Country Code */
                        strcat(szTagContent, "9F1A");
                        inPacketSizes += 4;
                        memset(szTerminalCountryCode, 0x00, sizeof(szTerminalCountryCode));
                        inGetTerminalCountryCode(szTerminalCountryCode);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen(szTerminalCountryCode) / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, szTerminalCountryCode);
                        inPacketSizes += strlen(szTerminalCountryCode);
                        
                        /* 5F2A-02 Transaction Currcncy Code */
                        strcat(szTagContent, "5F2A");
                        inPacketSizes += 4;
                        memset(szTransactionCurrencyCode, 0x00, sizeof(szTransactionCurrencyCode));
                        inGetVWTTransactionCurrencyCode(szTransactionCurrencyCode);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen(szTransactionCurrencyCode) / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, szTransactionCurrencyCode);
                        inPacketSizes += strlen(szTransactionCurrencyCode);
                       
                        /* 5F36 Transaction Currency Exponent */
                        strcat(szTagContent, "5F36");
                        inPacketSizes += 4;
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", 1);
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, "02");
                        inPacketSizes += strlen("02");
                        
                        /* DF8F40 Default TDOL */
                        strcat(szTagContent, "DF8F40");
                        inPacketSizes += 6;
                        memset(szPaypassDefaultTDOL, 0x00, sizeof(szPaypassDefaultTDOL));
                        inGetPaypassDefaultTDOL(szPaypassDefaultTDOL);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen(szPaypassDefaultTDOL) / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, szPaypassDefaultTDOL);
                        inPacketSizes += strlen(szPaypassDefaultTDOL);
                        
                        /* DF8F41 Default DDOL */
                        strcat(szTagContent, "DF8F41");
                        inPacketSizes += 6;
                        memset(szDefaultDDOL, 0x00, sizeof(szDefaultDDOL));
                        inGetDefaultDDOL(szDefaultDDOL);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen(szDefaultDDOL) / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, szDefaultDDOL);
                        inPacketSizes += strlen(szDefaultDDOL);
                        
                        /* DF8F42 Target Percent */
                        strcat(szTagContent, "DF8F42");
                        inPacketSizes += 6;
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen("00") / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, "00");
                        inPacketSizes += strlen("00");
                        
                        /* DF8F43 Max Target Percent */
                        strcat(szTagContent, "DF8F43");
                        inPacketSizes += 6;
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen("00") / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, "00");
                        inPacketSizes += strlen("00");
                        
                        /* DF8F44 Threshold value */
                        strcat(szTagContent, "DF8F44");
                        inPacketSizes += 6;
                        memset(szPaypassRandomSelectionThreshold, 0x00, sizeof(szPaypassRandomSelectionThreshold));
                        inGetPaypassRandomSelectionThreshold(szPaypassRandomSelectionThreshold);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memcpy(&szTemplate[0], &szPaypassRandomSelectionThreshold[0], 10);
                        sprintf(szTemplate1, "%08X", atoi(szTemplate));
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen(szTemplate1) / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, szTemplate1);
                        inPacketSizes += strlen(szTemplate1);
                        
                        /* DF8121 TAC Denial */
                        strcat(szTagContent, "DF8121");
                        inPacketSizes += 6;
                        memset(szPaypassDenialTAC, 0x00, sizeof(szPaypassDenialTAC));
                        inGetPaypassDenialTAC(szPaypassDenialTAC);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen(szPaypassDenialTAC) / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, szPaypassDenialTAC);
                        inPacketSizes += strlen(szPaypassDenialTAC);
                        
                        /* DF8122 TAC Online */
                        strcat(szTagContent, "DF8122");
                        inPacketSizes += 6;
                        memset(szPaypassOnlineTAC, 0x00, sizeof(szPaypassOnlineTAC));
                        inGetPaypassOnlineTAC(szPaypassOnlineTAC);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen(szPaypassOnlineTAC) / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, szPaypassOnlineTAC);
                        inPacketSizes += strlen(szPaypassOnlineTAC);
                        
                        /* DF8120 TAC Default */
                        strcat(szTagContent, "DF8120");
                        inPacketSizes += 6;
                        memset(szPaypassDefaultTAC, 0x00, sizeof(szPaypassDefaultTAC));
                        inGetPaypassDefaultTAC(szPaypassDefaultTAC);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen(szPaypassDefaultTAC) / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, szPaypassDefaultTAC);
                        inPacketSizes += strlen(szPaypassDefaultTAC);
                        
                        /* DF00 CL Transaction Limit */
                        strcat(szTagContent, "DF00");
                        inPacketSizes += 4;
                        memset(szContactlessTransactionLimit, 0x00, sizeof(szContactlessTransactionLimit));
                        inGetContactlessTransactionLimit(szContactlessTransactionLimit);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen(szContactlessTransactionLimit) / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, szContactlessTransactionLimit);
                        inPacketSizes += strlen(szContactlessTransactionLimit);
                        
                        /* DF01 CVM Limit */
                        memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
                        inGetCVMRequiredLimit(szCVMRequiredLimit);
                        lnCVMRequireLimit = atol(szCVMRequiredLimit);
                        
                        if (lnCVMRequireLimit > 0)
                        {
                                /* 如果金額已經為0時不需再減1元 */
                                lnCVMRequireLimit -= 100;
                        }
                        
                        memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
                        sprintf(szCVMRequiredLimit, "%ld", lnCVMRequireLimit);
                        inFunc_PAD_ASCII(szCVMRequiredLimit, szCVMRequiredLimit, '0', 12, _PADDING_LEFT_);
                        strcat(szTagContent, "DF01");
                        inPacketSizes += 4;
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen(szCVMRequiredLimit) / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, szCVMRequiredLimit);
                        inPacketSizes += strlen(szCVMRequiredLimit);
                        
                        /* DF02 CL Floor Limit */
                        strcat(szTagContent, "DF0206");
                        inPacketSizes += 6;
                        memset(szPaypassEMVFloorLimit, 0x00, sizeof(szPaypassEMVFloorLimit));
                        inGetPaypassEMVFloorLimit(szPaypassEMVFloorLimit);
                        lnContactlessFloorLimit = atoi(szPaypassEMVFloorLimit);
                        
                        // Tusin_071
                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_071_FORCE_ONLINE_, _CUSTOMER_INDICATOR_SIZE_))
                            lnContactlessFloorLimit = 0;
                        else
                            lnContactlessFloorLimit = atoi(szPaypassEMVFloorLimit);
                        
                        inFunc_PAD_ASCII(szPaypassEMVFloorLimit, szPaypassEMVFloorLimit, '0', 12, _PADDING_LEFT_);
                        strcat(szTagContent, szPaypassEMVFloorLimit);
                        inPacketSizes += strlen(szPaypassEMVFloorLimit);
                        			
                        /* DF68 Combination options */
                        /*
                                =============================================
                                DF68 Combination options , 2 bytes data
                                Byte 1
                                  bit 8 - RFU
                                  bit 7 - Status Check supported
                                  bit 6 - Offline Data Authentication supported
                                  bit 5 - Exception File Check required
                                  bit 4 - Random Transaction Selection supported
                                  bit 3 - RFU
                                  bit 2 - EMV Mode Supported
                                  bit 1 - Legacy Mode Supported
                                Byte 2
                                  bit 8-1 - RFU
                        
                                =============================================
                         */
                        strcat(szTagContent, "DF6802");
                        inPacketSizes += 6;
                        memset(szCombinationOption, 0x00, sizeof(szCombinationOption));
                        inGetCombinationOption(szCombinationOption);
                        strcat(szTagContent, szCombinationOption);
                        inPacketSizes += strlen(szCombinationOption);
                        
                        /* 9F52 Terminal Compatibility Indicator */
                        /*
                                =============================================
                                9F52 Terminal Compatibility Indicator, 1 byte data


                                Byte 1
                                  bit 8-3 - RFU
                                  bit 2 - EMV Mode Supported
                                  bit 1 - Magstripe Mode Supported

                                ============================================
                        */
                        strcat(szTagContent, "9F520103");
                        inPacketSizes += 8;
                                
                        /* 9F53 Terminal Interchange Profile */
                        /*
                                ============================================
                                9F53 Terminal Interchange Profile (static), 3 bytes data
                                Byte 1
                                  bit 8 - N/A, please set this bit to 0 (This bit will be dynamically set by Kernel to indicate CVM required by reader)
                                  bit 7 - Signature supported
                                  bit 6 - Online PIN supported
                                  bit 5 - On-Device CVM supported
                                  bit 4 - RFU
                                  bit 3 - Reader is a Transit Reader, please set to 0
                                  bit 2 - EMV contact chip supported
                                  bit 1 - (Contact Chip) Offline PIN supported
                                Byte 2
                                  bit 8 - Issuer Update supported
                                  bit 7-1 - RFU
                                Byte 3
                                  bit 8-1 - RFU
                                ============================================
                         */
                        strcat(szTagContent, "9F5303");
                        inPacketSizes += 6;
                        memset(szTerminalInterchangeProfile, 0x00, sizeof(szTerminalInterchangeProfile));
                        inGetTerminalInterchangeProfile(szTerminalInterchangeProfile);
                        strcat(szTagContent, szTerminalInterchangeProfile);
                        inPacketSizes += strlen(szTerminalInterchangeProfile);
                        
                        /* JCB感應的tag不會自己存到chip Data需要加下面的參數 */
			/* DF8F49 Upload DOL */
			/* The format of Tag Upload DOL is (FFC3 + DOL Data Length + DOL Data */
			/* Tag1 + 0x00 + Tag2 + 0x00 + .... + Tagn + 0x00 */
			strcat(szTagContent, "DF8F49");
			inPacketSizes += 6;
			
			/* DF8F4F內容 */
			/* 【需求單 - 106252】	DCC交易新增感應及BIN11碼需求 新增5F28 9F42 by Russell 2020/2/20 上午 11:56 */
			memset(szDF8F4FData, 0x00, sizeof(szDF8F4FData));
			strcpy(szDF8F4FData, "50005A005F20005F2A005F28005F3400820084008A0095009A009B009C009F02009F03009F10009F1A009F1E009F26009F27009F33009F34009F35009F36009F37009F42009F6E009F7C00");
			
			/* 全部長度(含FFC3) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%02X", ((strlen("FFC3") + 2 + strlen(szDF8F4FData)) / 2));
			strcat(szTagContent, szTemplate);
			inPacketSizes += 2;
			
			strcat(szTagContent, "FFC3");
			inPacketSizes += 4;
			
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%02X", (strlen(szDF8F4FData) / 2));
			strcat(szTagContent, szTemplate);
			inPacketSizes += 2;
                        
			strcat(szTagContent, szDF8F4FData);
			inPacketSizes += strlen(szDF8F4FData);
                                
                        xmlNodeAddContent(srItem_Node, BAD_CAST szTagContent);
                        xmlAddChild(srGroup_Node, srItem_Node);
                }
                /* Paypass */
                else if (!memcmp(&szMVTApplicationId[0], "A000000004", 10))
                {
                        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "tlv");
                        memset(szTagContent, 0x00, sizeof(szTagContent));
                        inPacketSizes = 0;

                        /* 5F57 Account Type */
                        strcat(szTagContent, "5F57");
                        inPacketSizes += 4;
                        strcat(szTagContent, "00");
                        inPacketSizes += 2;
                        
                        /* 9F01 Acquirer Identifier */
                        strcat(szTagContent, "9F01");
                        inPacketSizes += 4;
                        strcat(szTagContent, "00");
                        inPacketSizes += 2;
                        
                        /* 9F40 Additional Terminal Capabilities */
                        strcat(szTagContent, "9F40");
                        inPacketSizes += 4;
                        memset(szAdditionalTerminalCapabilities, 0x00, sizeof(szAdditionalTerminalCapabilities));
                        inGetAdditionalTerminalCapabilities(szAdditionalTerminalCapabilities);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen(szAdditionalTerminalCapabilities) / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, szAdditionalTerminalCapabilities);
                        inPacketSizes += strlen(szAdditionalTerminalCapabilities);
                        
			/* Ex: DF8117 = 00,  DF8118 = 60,  DF8119 = 08, DF811F = 08
			   when amount > Reader CVM Required Limit, the 9F33 will be 006008
			   when amount <= Reader CVM Required Limit, the 9F33 will be 000808
			 */
			memset(szTerminalCapabilities, 0x00, sizeof(szTerminalCapabilities));
                        inGetPaypassTerminalCapabilities(szTerminalCapabilities);
			/* 沒值或空值(判斷第一個為空白) */
			if (strlen(szTerminalCapabilities) == 0	|| memcmp(" ", szTerminalCapabilities, 1) == 0)
			{
				/* 空值抓接觸式 */
				inGetTerminalCapabilities(szTerminalCapabilities);
			}
			
                        /* DF8117 Card Data Input Capability */
			strcat(szTagContent, "DF8117");
                        inPacketSizes += 6;
                        strcat(szTagContent, "01");
                        inPacketSizes += 2;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &szTerminalCapabilities[0], 2);
			strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        
                        /* DF8118 CVM Capability – CVM Required */
                        strcat(szTagContent, "DF8118");
                        inPacketSizes += 6;
                        strcat(szTagContent, "01");
                        inPacketSizes += 2;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &szTerminalCapabilities[2], 2);
			strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        
                        /* DF8119 CVM Capability – No CVM Required */
                        strcat(szTagContent, "DF8119");
                        inPacketSizes += 6;
                        strcat(szTagContent, "01");
                        inPacketSizes += 2;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, "08", 2);
			strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
			
			/* DF811F Security Capability */
                        strcat(szTagContent, "DF811F");
                        inPacketSizes += 6;
                        strcat(szTagContent, "01");
                        inPacketSizes += 2;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &szTerminalCapabilities[4], 2);
			strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        
                        /* DF811A Default UDOL */
                        strcat(szTagContent, "DF811A039F6A04");
                        inPacketSizes += 14;
			
			/* DF811B Kernel Cofiguration */
			/* bit 8 - Mag-stripe mode contactless transactions not supported
			   bit 7 - EMV mode contactless transactions not supported
			   bit 6 - On device cardholder verification supported(CDCVM)
			   bit 5 - Relay resistance protocol supported  --> 1 : support ; 0 : not support
			   bit 4-1 - RFU
			 */
			/* (需求單-110175)-端末設備配合實務優化流程需求 
			 * 因應MASTER CARD規範「端末設備Contactless Reader版本為V3.1.1或以上需支援RRP (Relay Resistance Protocol) 功能」，開啟RRP功能。
			 * by Russell 2021/12/20 下午 4:05 */
                        strcat(szTagContent, "DF811B");
                        inPacketSizes += 6;
			
			strcat(szTagContent, "01");
                        inPacketSizes += 2;
			
			if (memcmp(gszTermVersionID, _CEE_UPT1000_TERMINAL_VERSION_, _CEE_UPT1000_TERMINAL_VERSION_LEN_) == 0)
			{
				strcat(szTagContent, "10");
				inPacketSizes += 2;
			}
			else
			{
				strcat(szTagContent, "30");
				inPacketSizes += 2;
			}
                        
			/* 9F1D Terminal Risk Management Data */
			if (memcmp(gszTermVersionID, _CEE_UPT1000_TERMINAL_VERSION_, _CEE_UPT1000_TERMINAL_VERSION_LEN_) == 0)
			{
				strcat(szTagContent, "9F1D080800800000000000");
				inPacketSizes += 22;
			}
			else
			{
				strcat(szTagContent, "9F1D082C32800000000000");
				inPacketSizes += 22;
			}
			
                        /* 9F1E Interface Device(IFD) Serial Number */
                        
                        /* DF810C Kernel ID */
                        strcat(szTagContent, "DF810C0102");
                        inPacketSizes += 10;
                        
                        /* 9F6D Mag-srtipe Application Version Number (Reader) */
                        strcat(szTagContent, "9F6D020001");
                        inPacketSizes += 10;
                        
                        /* DF811E Mag-stripe CVM Capability – CVM Required */
                        strcat(szTagContent, "DF811E0110");
                        inPacketSizes += 10;
                        
                        /* DF812C Mag-stripe CVM Capability – No CVM Required */
                        strcat(szTagContent, "DF812C0100");
                        inPacketSizes += 10;
                        
                        /* DF811C Max Lifetime of Torn Transaction Log Record */
                        strcat(szTagContent, "DF811C020000");
                        inPacketSizes += 12;
                        
                        /* DF811D Max Number of Torn Transaction Log Record */
                        strcat(szTagContent, "DF811D0100");
                        inPacketSizes += 10;

                        /* 9F15 Merchant Category Code */
                        strcat(szTagContent, "9F1502");
                        inPacketSizes += 6;
                        memset(szMerchantCategoryCode, 0x00, sizeof(szMerchantCategoryCode));
                        inGetVWTMerchantCategoryCode(szMerchantCategoryCode);
                        strcat(szTagContent, szMerchantCategoryCode);
                        inPacketSizes += 4;
                        
                        /* 9F16 Merchant Identifier */
//                        strcat(szTagContent, "9F1600");
//                        inPacketSizes += 6;
                        
                        /* 9F4E Merchant Nameand Location */
			/* 要嗎填值，要嗎就不填 */
//                        strcat(szTagContent, "9F4E00");
//                        inPacketSizes += 6;
                        
                        /* 9F7E Mobile Support Indicator */
//                        strcat(szTagContent, "9F7E00");
//                        inPacketSizes += 6;

			/* Indicaates the transaction amount above which transactions must be authorized online */
                        /* DF8123 PaypassEMVFloorLimit */
                        memset(szPaypassEMVFloorLimit, 0x00, sizeof(szPaypassEMVFloorLimit));
                        inGetPaypassEMVFloorLimit(szPaypassEMVFloorLimit);
                        
                        // Tusin_071
                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_071_FORCE_ONLINE_, _CUSTOMER_INDICATOR_SIZE_))
                            lnContactlessFloorLimit = 0;
                        else
                            lnContactlessFloorLimit = atoi(szPaypassEMVFloorLimit);
                        
                        if (lnContactlessFloorLimit > 0)
                        {
                                /* 如果金額已經為0時不需再減1元 */
                                lnContactlessFloorLimit -= 100;
                        }
                        
                        memset(szPaypassEMVFloorLimit, 0x00, sizeof(szPaypassEMVFloorLimit));
                        sprintf(szPaypassEMVFloorLimit, "%ld", lnContactlessFloorLimit);
                        inFunc_PAD_ASCII(szPaypassEMVFloorLimit, szPaypassEMVFloorLimit, '0', 12, _PADDING_LEFT_);
                        strcat(szTagContent, "DF8123");
                        inPacketSizes += 6;
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen(szPaypassEMVFloorLimit) / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, szPaypassEMVFloorLimit);
                        inPacketSizes += strlen(szPaypassEMVFloorLimit);
                        
                        /* DF8124 MC CL Trans Limit-No on device CVM */
                        strcat(szTagContent, "DF8124");
                        inPacketSizes += 6;
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen("001000000000") / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, "001000000000");
                        inPacketSizes += 12;
                        
                        /* DF8125 MC CL Trans Limit-on device CVM */
                        strcat(szTagContent, "DF8125");
                        inPacketSizes += 6;
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen("001000000000") / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, "001000000000");
                        inPacketSizes += 12;
			
			/* Indicates the transaction amount above which the Kernel instantiates the CVM capabilities field in 
			 * Terminal Capabilities with CVM Capability ─ CVM Required */
			/* DF8126 PaypassCVMRequiredLimit */
                        memset(szPaypassCVMRequiredLimit, 0x00, sizeof(szPaypassCVMRequiredLimit));
                        inGetPaypassCVMRequiredLimit(szPaypassCVMRequiredLimit);
                        lnCVMRequireLimit = atoi(szPaypassCVMRequiredLimit);
                        
                        if (lnCVMRequireLimit > 0)
                        {
                                /* 如果金額已經為0時不需再減1元 */
                                lnCVMRequireLimit -= 100;
                        }
                        
                        memset(szPaypassCVMRequiredLimit, 0x00, sizeof(szPaypassCVMRequiredLimit));
                        sprintf(szPaypassCVMRequiredLimit, "%ld", lnCVMRequireLimit);
                        inFunc_PAD_ASCII(szPaypassCVMRequiredLimit, szPaypassCVMRequiredLimit, '0', 12, _PADDING_LEFT_);
                        strcat(szTagContent, "DF8126");
                        inPacketSizes += 6;
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szPaypassCVMRequiredLimit) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szPaypassCVMRequiredLimit);
                        inPacketSizes += strlen(szPaypassCVMRequiredLimit);
                        
                        /* DF8121 Paypass Denial_TAC */
                        strcat(szTagContent, "DF8121");
                        inPacketSizes += 6;
                        memset(szPaypassDenialTAC, 0x00, sizeof(szPaypassDenialTAC));
                        inGetPaypassDenialTAC(szPaypassDenialTAC);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen(szPaypassDenialTAC) / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, szPaypassDenialTAC);
                        inPacketSizes += strlen(szPaypassDenialTAC);
                        
                        /* DF8122 Paypass Online_TAC */
                        strcat(szTagContent, "DF8122");
                        inPacketSizes += 6;
                        memset(szPaypassOnlineTAC, 0x00, sizeof(szPaypassOnlineTAC));
                        inGetPaypassOnlineTAC(szPaypassOnlineTAC);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen(szPaypassOnlineTAC) / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, szPaypassOnlineTAC);
                        inPacketSizes += strlen(szPaypassOnlineTAC);
                
                        /* DF8120 Paypass Default_TAC */
                        strcat(szTagContent, "DF8120");
                        inPacketSizes += 6;
                        memset(szPaypassDefaultTAC, 0x00, sizeof(szPaypassDefaultTAC));
                        inGetPaypassDefaultTAC(szPaypassDefaultTAC);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen(szPaypassDefaultTAC) / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, szPaypassDefaultTAC);
                        inPacketSizes += strlen(szPaypassDefaultTAC);
                
                        /* 9F33 Terminal Capability */
                        strcat(szTagContent, "9F33");
                        inPacketSizes += 4;
                        memset(szTerminalCapabilities, 0x00, sizeof(szTerminalCapabilities));
                        inGetPaypassTerminalCapabilities(szTerminalCapabilities);
			/* 沒值或空值(判斷第一個為空白) */
			if (strlen(szTerminalCapabilities) == 0	|| memcmp(" ", szTerminalCapabilities, 1) == 0)
			{
				/* 空值抓接觸式 */
				inGetTerminalCapabilities(szTerminalCapabilities);
			}
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen(szTerminalCapabilities) / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, szTerminalCapabilities);
                        inPacketSizes += strlen(szTerminalCapabilities);

                        /* 5F2A-02 Transaction Currcncy Code */
                        strcat(szTagContent, "5F2A");
                        inPacketSizes += 4;
                        memset(szTransactionCurrencyCode, 0x00, sizeof(szTransactionCurrencyCode));
                        inGetPaypassTransactionCurrencyCode(szTransactionCurrencyCode);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTransactionCurrencyCode) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTransactionCurrencyCode);
                        inPacketSizes += strlen(szTransactionCurrencyCode);
                        
                        /* 9F1A Terminal Country Code */
                        strcat(szTagContent, "9F1A");
                        inPacketSizes += 4;
                        memset(szTerminalCountryCode, 0x00, sizeof(szTerminalCountryCode));
                        inGetPaypassTerminalCountryCode(szTerminalCountryCode);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen(szTerminalCountryCode) / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, szTerminalCountryCode);
                        inPacketSizes += strlen(szTerminalCountryCode);

                        /* 9F1C Terminal Identification */
                        strcat(szTagContent, "9F1C00");
                        inPacketSizes += 6;
                        
                        /* 9F35 */
			memset(szTerminalType, 0x00, sizeof(szTerminalType));
			inGetPaypassTerminalType(szTerminalType);
                        if (strlen(szTerminalType) == 0	|| memcmp(" ", szTerminalType, 1) == 0)
			{
				/* VWT沒值改抓回MVT，沒填Terminal Type Kernel會無法運行 */
				memset(szTerminalType, 0x00, sizeof(szTerminalType));
				inGetMVTTerminalType(szTerminalType);
				
				strcat(szTagContent, "9F35");
				inPacketSizes += 4;
				
				sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalType) / 2));
				inPacketSizes += 2;
				
				if (strlen(szTerminalType) > 0)
				{
					strcat(szTagContent, szTerminalType);
					inPacketSizes += strlen(szTerminalType);
				}
			}
			else
			{
				strcat(szTagContent, "9F35");
				inPacketSizes += 4;
				strcat(szTagContent, "01");
				inPacketSizes += 2;

				/* 強制縮減長度為2，避免意外 */
				szTerminalType[2] = 0x00;
				strcat(szTagContent, szTerminalType);
				inPacketSizes += strlen(szTerminalType);
			}
			
			/* DF8F49 Upload DOL */
			/* The format of Tag Upload DOL is (FFC3 + DOL Data Length + DOL Data */
			/* Tag1 + 0x00 + Tag2 + 0x00 + .... + Tagn + 0x00 */
			strcat(szTagContent, "DF8F49");
			inPacketSizes += 6;
			
			/* DF8F4F內容 */
			/* 【需求單 - 106252】	DCC交易新增感應及BIN11碼需求 新增5F28 9F42 by Russell 2020/2/20 上午 11:56 */
			memset(szDF8F4FData, 0x00, sizeof(szDF8F4FData));
			strcpy(szDF8F4FData, "50005A005F20005F2A005F28005F3400820084008A0095009A009B009C009F02009F03009F10009F1A009F1E009F26009F27009F33009F34009F35009F36009F37009F42009F6E00");
			
			/* 全部長度(含FFC3) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%02X", ((strlen("FFC3") + 2 + strlen(szDF8F4FData)) / 2));
			strcat(szTagContent, szTemplate);
			inPacketSizes += 2;
			
			strcat(szTagContent, "FFC3");
			inPacketSizes += 4;
			
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%02X", (strlen(szDF8F4FData) / 2));
			strcat(szTagContent, szTemplate);
			inPacketSizes += 2;
                        
			strcat(szTagContent, szDF8F4FData);
			inPacketSizes += strlen(szDF8F4FData);
                        
                        xmlNodeAddContent(srItem_Node, BAD_CAST szTagContent);
                        xmlAddChild(srGroup_Node, srItem_Node);
                }
                /* QuickPass_Debit */
                else if (!memcmp(&szMVTApplicationId[0], "A000000333010101", 16))
                {
                        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "tlv");
                        memset(szTagContent, 0x00, sizeof(szTagContent));
                        inPacketSizes = 0;
                        
                        /* 9F33 */
                        strcat(szTagContent, "9F33");
                        inPacketSizes += 4;
                        memset(szTerminalCapabilities, 0x00, sizeof(szTerminalCapabilities));
                        inGetPaypassTerminalCapabilities(szTerminalCapabilities);
			/* 沒值或空值(判斷第一個為空白) */
			if (strlen(szTerminalCapabilities) == 0	|| memcmp(" ", szTerminalCapabilities, 1) == 0)
			{
				/* 空值抓接觸式 */
				inGetTerminalCapabilities(szTerminalCapabilities);
			}
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalCapabilities) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTerminalCapabilities);
                        inPacketSizes += strlen(szTerminalCapabilities);
                        
			/* 9F35-01 Terminal Type */
			memset(szTerminalType, 0x00, sizeof(szTerminalType));
			inGetPaypassTerminalType(szTerminalType);
			if (strlen(szTerminalType) == 0	|| memcmp(" ", szTerminalType, 1) == 0)
			{
				/* VWT沒值改抓回MVT，沒填Terminal Type Kernel會無法運行 */
				memset(szTerminalType, 0x00, sizeof(szTerminalType));
				inGetMVTTerminalType(szTerminalType);
				
				strcat(szTagContent, "9F35");
				inPacketSizes += 4;
				
				sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalType) / 2));
				inPacketSizes += 2;
				
				if (strlen(szTerminalType) > 0)
				{
					strcat(szTagContent, szTerminalType);
					inPacketSizes += strlen(szTerminalType);
				}
			}
			else
			{
				strcat(szTagContent, "9F35");
				inPacketSizes += 4;
				strcat(szTagContent, "01");
				inPacketSizes += 2;

				/* 強制縮減長度為2，避免意外 */
				szTerminalType[2] = 0x00;
				strcat(szTagContent, szTerminalType);
				inPacketSizes += strlen(szTerminalType);
			}
			
                        /* DF04-01 CVM Required */
                        strcat(szTagContent, "DF04");
                        inPacketSizes += 4;
                        memset(szCVMRequirement, 0x00, sizeof(szCVMRequirement));
                        inGetCVMRequirement(szCVMRequirement);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szCVMRequirement) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szCVMRequirement);
                        inPacketSizes += strlen(szCVMRequirement);
                        
                        /* 5F2A-02 Transaction Currcncy Code */
                        strcat(szTagContent, "5F2A");
                        inPacketSizes += 4;
                        memset(szTransactionCurrencyCode, 0x00, sizeof(szTransactionCurrencyCode));
                        inGetVWTTransactionCurrencyCode(szTransactionCurrencyCode);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTransactionCurrencyCode) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTransactionCurrencyCode);
                        inPacketSizes += strlen(szTransactionCurrencyCode);
                        
                        /* 9C-01 Transaction Type */
                        strcat(szTagContent, "9C");
                        inPacketSizes += 2;
                        memset(szTransactionType, 0x00, sizeof(szTransactionType));
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
			{
				/* 0x01: cash */
				sprintf(szTransactionType, "01");
			}
			else
			{
				inGetTransactionType(szTransactionType);
			}
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTransactionType) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTransactionType);
                        inPacketSizes += strlen(szTransactionType);
                        
                        /* 9F66-04 Terminal Transaction Qualifier */
                        strcat(szTagContent, "9F66");
                        inPacketSizes += 4;
                        memset(szTerminalTransactionQualifier, 0x00, sizeof(szTerminalTransactionQualifier));
                        inGetTerminalTransactionQualifier(szTerminalTransactionQualifier);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalTransactionQualifier) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTerminalTransactionQualifier);
                        inPacketSizes += strlen(szTerminalTransactionQualifier);
                        
                        /* 9F1A */
                        strcat(szTagContent, "9F1A");
                        inPacketSizes += 4;
                        memset(szTerminalCountryCode, 0x00, sizeof(szTerminalCountryCode));
                        inGetTerminalCountryCode(szTerminalCountryCode);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalCountryCode) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTerminalCountryCode);
                        inPacketSizes += strlen(szTerminalCountryCode);
                        
                        /* DF00 */
                        strcat(szTemplate, "DF0006");
                        strcat(szTemplate, "001000000000");
                        
                        /* DF01 */
                        memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
                        inGetCVMRequiredLimit(szCVMRequiredLimit);
                        lnCVMRequireLimit = atoi(szCVMRequiredLimit);
                        
                        if (lnCVMRequireLimit > 0)
                        {
                                /* 如果金額已經為0時不需再減1元 */
                                lnCVMRequireLimit -= 100;
                        }
                        
                        memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
                        sprintf(szCVMRequiredLimit, "%ld", lnCVMRequireLimit);
                        inFunc_PAD_ASCII(szCVMRequiredLimit, szCVMRequiredLimit, '0', 12, _PADDING_LEFT_);
                        strcat(szTagContent, "DF01");
                        inPacketSizes += 4;
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szCVMRequiredLimit) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szCVMRequiredLimit);
                        inPacketSizes += strlen(szCVMRequiredLimit);
                        
                        /* DF02 */
                        memset(szContactlessFloorLimit, 0x00, sizeof(szContactlessFloorLimit));
                        inGetContactlessFloorLimit(szContactlessFloorLimit);
                        
                        // Tusin_071
                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_071_FORCE_ONLINE_, _CUSTOMER_INDICATOR_SIZE_))
                            lnContactlessFloorLimit = 0;
                        else
                            lnContactlessFloorLimit = atoi(szContactlessFloorLimit);
                        
                        if (lnContactlessFloorLimit > 0)
                        {
                                /* 如果金額已經為0時不需再減1元 */
                                lnContactlessFloorLimit -= 100;
                        }
                        
                        memset(szContactlessFloorLimit, 0x00, sizeof(szContactlessFloorLimit));
                        sprintf(szContactlessFloorLimit, "%ld", lnContactlessFloorLimit);
                        inFunc_PAD_ASCII(szContactlessFloorLimit, szContactlessFloorLimit, '0', 12, _PADDING_LEFT_);
                        strcat(szTagContent, "DF02");
                        inPacketSizes += 4;
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szContactlessFloorLimit) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szContactlessFloorLimit);
                        inPacketSizes += strlen(szContactlessFloorLimit);
                        
                        /* DF05-01 Display Offline Available Fund */
                        strcat(szTagContent, "DF05");
                        inPacketSizes += 4;
                        memset(szDisplayOfflineFunds, 0x00, sizeof(szDisplayOfflineFunds));
                        inGetDisplayOfflineFunds(szDisplayOfflineFunds);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szDisplayOfflineFunds) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szDisplayOfflineFunds);
                        inPacketSizes += strlen(szDisplayOfflineFunds);
                        
                        /* DF8F4D OverWrite */
			
			/* CUP感應的tag不會自己存到chip Data需要加下面的參數 */
			/* DF8F49 Upload DOL */
			/* The format of Tag Upload DOL is (FFC3 + DOL Data Length + DOL Data */
			/* Tag1 + 0x00 + Tag2 + 0x00 + .... + Tagn + 0x00 */
			strcat(szTagContent, "DF8F49");
			inPacketSizes += 6;
			
			/* DF8F49內容 */
			/* 【需求單 - 106252】	DCC交易新增感應及BIN11碼需求 新增5F28 9F42 by Russell 2020/2/20 上午 11:56 */
			memset(szDF8F4FData, 0x00, sizeof(szDF8F4FData));
			strcpy(szDF8F4FData, "50005A005F20005F2A005F28005F3400820084008A0095009A009B009C009F02009F03009F10009F1A009F1E009F26009F27009F33009F34009F35009F36009F37009F42009F6E00");
			
			/* 全部長度(含FFC3) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%02X", ((strlen("FFC3") + 2 + strlen(szDF8F4FData)) / 2));
			strcat(szTagContent, szTemplate);
			inPacketSizes += 2;
			
			strcat(szTagContent, "FFC3");
			inPacketSizes += 4;
			
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%02X", (strlen(szDF8F4FData) / 2));
			strcat(szTagContent, szTemplate);
			inPacketSizes += 2;
                        
			strcat(szTagContent, szDF8F4FData);
			inPacketSizes += strlen(szDF8F4FData);
                        
                        xmlNodeAddContent(srItem_Node, BAD_CAST szTagContent);
                        xmlAddChild(srGroup_Node, srItem_Node);
                }
                /* QuickPass_Credit */
                else if (!memcmp(&szMVTApplicationId[0], "A000000333010102", 16))
                {
                        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "tlv");
                        memset(szTagContent, 0x00, sizeof(szTagContent));
                        inPacketSizes = 0;
                        
                        /* 9F33 */
                        strcat(szTagContent, "9F33");
                        inPacketSizes += 4;
                        memset(szTerminalCapabilities, 0x00, sizeof(szTerminalCapabilities));
                        inGetPaypassTerminalCapabilities(szTerminalCapabilities);
			/* 沒值或空值(判斷第一個為空白) */
			if (strlen(szTerminalCapabilities) == 0	|| memcmp(" ", szTerminalCapabilities, 1) == 0)
			{
				/* 空值抓接觸式 */
				inGetTerminalCapabilities(szTerminalCapabilities);
			}
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalCapabilities) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTerminalCapabilities);
                        inPacketSizes += strlen(szTerminalCapabilities);
			
			/* 9F35-01 Terminal Type */
			memset(szTerminalType, 0x00, sizeof(szTerminalType));
			inGetPaypassTerminalType(szTerminalType);
			if (strlen(szTerminalType) == 0	|| memcmp(" ", szTerminalType, 1) == 0)
			{
				/* VWT沒值改抓回MVT，沒填Terminal Type Kernel會無法運行 */
				memset(szTerminalType, 0x00, sizeof(szTerminalType));
				inGetMVTTerminalType(szTerminalType);
				
				strcat(szTagContent, "9F35");
				inPacketSizes += 4;
				
				sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalType) / 2));
				inPacketSizes += 2;
				
				if (strlen(szTerminalType) > 0)
				{
					strcat(szTagContent, szTerminalType);
					inPacketSizes += strlen(szTerminalType);
				}
			}
			else
			{
				strcat(szTagContent, "9F35");
				inPacketSizes += 4;
				strcat(szTagContent, "01");
				inPacketSizes += 2;

				/* 強制縮減長度為2，避免意外 */
				szTerminalType[2] = 0x00;
				strcat(szTagContent, szTerminalType);
				inPacketSizes += strlen(szTerminalType);
			}
                        
                        /* DF04-01 CVM Required */
                        strcat(szTagContent, "DF04");
                        inPacketSizes += 4;
                        memset(szCVMRequirement, 0x00, sizeof(szCVMRequirement));
                        inGetCVMRequirement(szCVMRequirement);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szCVMRequirement) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szCVMRequirement);
                        inPacketSizes += strlen(szCVMRequirement);
                        
                        /* 5F2A-02 Transaction Currcncy Code */
                        strcat(szTagContent, "5F2A");
                        inPacketSizes += 4;
                        memset(szTransactionCurrencyCode, 0x00, sizeof(szTransactionCurrencyCode));
                        inGetVWTTransactionCurrencyCode(szTransactionCurrencyCode);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTransactionCurrencyCode) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTransactionCurrencyCode);
                        inPacketSizes += strlen(szTransactionCurrencyCode);
                        
                        /* 9C-01 Transaction Type */
                        strcat(szTagContent, "9C");
                        inPacketSizes += 2;
                        memset(szTransactionType, 0x00, sizeof(szTransactionType));
                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
			{
				/* 0x01: cash */
				sprintf(szTransactionType, "01");
			}
			else
			{
				inGetTransactionType(szTransactionType);
			}
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTransactionType) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTransactionType);
                        inPacketSizes += strlen(szTransactionType);
                        
                        /* 9F66-04 Terminal Transaction Qualifier */
                        strcat(szTagContent, "9F66");
                        inPacketSizes += 4;
                        memset(szTerminalTransactionQualifier, 0x00, sizeof(szTerminalTransactionQualifier));
                        inGetTerminalTransactionQualifier(szTerminalTransactionQualifier);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalTransactionQualifier) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTerminalTransactionQualifier);
                        inPacketSizes += strlen(szTerminalTransactionQualifier);
                        
                        /* 9F1A */
                        strcat(szTagContent, "9F1A");
                        inPacketSizes += 4;
                        memset(szTerminalCountryCode, 0x00, sizeof(szTerminalCountryCode));
                        inGetTerminalCountryCode(szTerminalCountryCode);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalCountryCode) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTerminalCountryCode);
                        inPacketSizes += strlen(szTerminalCountryCode);
                        
                        /* DF00 */
                        strcat(szTemplate, "DF0006");
                        strcat(szTemplate, "001000000000");
                        
                        /* DF01 */
                        memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
                        inGetCVMRequiredLimit(szCVMRequiredLimit);
                        lnCVMRequireLimit = atoi(szCVMRequiredLimit);
                        
                        if (lnCVMRequireLimit > 0)
                        {
                                /* 如果金額已經為0時不需再減1元 */
                                lnCVMRequireLimit -= 100;
                        }
                        
                        memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
                        sprintf(szCVMRequiredLimit, "%ld", lnCVMRequireLimit);
                        inFunc_PAD_ASCII(szCVMRequiredLimit, szCVMRequiredLimit, '0', 12, _PADDING_LEFT_);
                        strcat(szTagContent, "DF01");
                        inPacketSizes += 4;
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szCVMRequiredLimit) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szCVMRequiredLimit);
                        inPacketSizes += strlen(szCVMRequiredLimit);
                        
                        /* DF02 */
                        memset(szContactlessFloorLimit, 0x00, sizeof(szContactlessFloorLimit));
                        inGetContactlessFloorLimit(szContactlessFloorLimit);
                        
                        // Tusin_071
                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_071_FORCE_ONLINE_, _CUSTOMER_INDICATOR_SIZE_))
                            lnContactlessFloorLimit = 0;
                        else
                            lnContactlessFloorLimit = atoi(szContactlessFloorLimit);
                        
                        if (lnContactlessFloorLimit > 0)
                        {
                                /* 如果金額已經為0時不需再減1元 */
                                lnContactlessFloorLimit -= 100;
                        }
                        
                        memset(szContactlessFloorLimit, 0x00, sizeof(szContactlessFloorLimit));
                        sprintf(szContactlessFloorLimit, "%ld", lnContactlessFloorLimit);
                        inFunc_PAD_ASCII(szContactlessFloorLimit, szContactlessFloorLimit, '0', 12, _PADDING_LEFT_);
                        strcat(szTagContent, "DF02");
                        inPacketSizes += 4;
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szContactlessFloorLimit) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szContactlessFloorLimit);
                        inPacketSizes += strlen(szContactlessFloorLimit);
                        
                        /* DF05-01 Display Offline Available Fund */
                        strcat(szTagContent, "DF05");
                        inPacketSizes += 4;
                        memset(szDisplayOfflineFunds, 0x00, sizeof(szDisplayOfflineFunds));
                        inGetDisplayOfflineFunds(szDisplayOfflineFunds);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szDisplayOfflineFunds) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szDisplayOfflineFunds);
                        inPacketSizes += strlen(szDisplayOfflineFunds);
                        
                        /* DF8F4D OverWrite */
			
			/* CUP感應的tag不會自己存到chip Data需要加下面的參數 */
			/* DF8F49 Upload DOL */
			/* The format of Tag Upload DOL is (FFC3 + DOL Data Length + DOL Data */
			/* Tag1 + 0x00 + Tag2 + 0x00 + .... + Tagn + 0x00 */
			strcat(szTagContent, "DF8F49");
			inPacketSizes += 6;
			
			/* DF8F4F內容 */
			/* 【需求單 - 106252】	DCC交易新增感應及BIN11碼需求 新增5F28 9F42 by Russell 2020/2/20 上午 11:56 */
			memset(szDF8F4FData, 0x00, sizeof(szDF8F4FData));
			strcpy(szDF8F4FData, "50005A005F20005F2A005F28005F3400820084008A0095009A009B009C009F02009F03009F10009F1A009F1E009F26009F27009F33009F34009F35009F36009F37009F42009F6E00");
			
			/* 全部長度(含FFC3) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%02X", ((strlen("FFC3") + 2 + strlen(szDF8F4FData)) / 2));
			strcat(szTagContent, szTemplate);
			inPacketSizes += 2;
			
			strcat(szTagContent, "FFC3");
			inPacketSizes += 4;
			
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%02X", (strlen(szDF8F4FData) / 2));
			strcat(szTagContent, szTemplate);
			inPacketSizes += 2;
                        
			strcat(szTagContent, szDF8F4FData);
			inPacketSizes += strlen(szDF8F4FData);

                        xmlNodeAddContent(srItem_Node, BAD_CAST szTagContent);
                        xmlAddChild(srGroup_Node, srItem_Node);
                }
                /* QuickPass_QCredit */
                else if (!memcmp(&szMVTApplicationId[0], "A000000333010103", 16))
                {
                        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "tlv");
                        memset(szTagContent, 0x00, sizeof(szTagContent));
                        inPacketSizes = 0;
                        
                        /* 9F33 */
                        strcat(szTagContent, "9F33");
                        inPacketSizes += 4;
                        memset(szTerminalCapabilities, 0x00, sizeof(szTerminalCapabilities));
                        inGetPaypassTerminalCapabilities(szTerminalCapabilities);
			/* 沒值或空值(判斷第一個為空白) */
			if (strlen(szTerminalCapabilities) == 0	|| memcmp(" ", szTerminalCapabilities, 1) == 0)
			{
				/* 空值抓接觸式 */
				inGetTerminalCapabilities(szTerminalCapabilities);
			}
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalCapabilities) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTerminalCapabilities);
                        inPacketSizes += strlen(szTerminalCapabilities);
			
			/* 9F35-01 Terminal Type */
			memset(szTerminalType, 0x00, sizeof(szTerminalType));
			inGetPaypassTerminalType(szTerminalType);
			if (strlen(szTerminalType) == 0	|| memcmp(" ", szTerminalType, 1) == 0)
			{
				/* VWT沒值改抓回MVT，沒填Terminal Type Kernel會無法運行 */
				memset(szTerminalType, 0x00, sizeof(szTerminalType));
				inGetMVTTerminalType(szTerminalType);
				
				strcat(szTagContent, "9F35");
				inPacketSizes += 4;
				
				sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalType) / 2));
				inPacketSizes += 2;
				
				if (strlen(szTerminalType) > 0)
				{
					strcat(szTagContent, szTerminalType);
					inPacketSizes += strlen(szTerminalType);
				}
			}
			else
			{
				strcat(szTagContent, "9F35");
				inPacketSizes += 4;
				strcat(szTagContent, "01");
				inPacketSizes += 2;

				/* 強制縮減長度為2，避免意外 */
				szTerminalType[2] = 0x00;
				strcat(szTagContent, szTerminalType);
				inPacketSizes += strlen(szTerminalType);
			}
                        
                        /* DF04-01 CVM Required */
                        strcat(szTagContent, "DF04");
                        inPacketSizes += 4;
                        memset(szCVMRequirement, 0x00, sizeof(szCVMRequirement));
                        inGetCVMRequirement(szCVMRequirement);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szCVMRequirement) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szCVMRequirement);
                        inPacketSizes += strlen(szCVMRequirement);
                        
                        /* 5F2A-02 Transaction Currcncy Code */
                        strcat(szTagContent, "5F2A");
                        inPacketSizes += 4;
                        memset(szTransactionCurrencyCode, 0x00, sizeof(szTransactionCurrencyCode));
                        inGetVWTTransactionCurrencyCode(szTransactionCurrencyCode);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTransactionCurrencyCode) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTransactionCurrencyCode);
                        inPacketSizes += strlen(szTransactionCurrencyCode);
                        
                        /* 9C-01 Transaction Type */
                        strcat(szTagContent, "9C");
                        inPacketSizes += 2;
                        memset(szTransactionType, 0x00, sizeof(szTransactionType));
                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
			{
				/* 0x01: cash */
				sprintf(szTransactionType, "01");
			}
			else
			{
				inGetTransactionType(szTransactionType);
			}
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTransactionType) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTransactionType);
                        inPacketSizes += strlen(szTransactionType);
                        
                        /* 9F66-04 Terminal Transaction Qualifier */
                        strcat(szTagContent, "9F66");
                        inPacketSizes += 4;
                        memset(szTerminalTransactionQualifier, 0x00, sizeof(szTerminalTransactionQualifier));
                        inGetTerminalTransactionQualifier(szTerminalTransactionQualifier);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalTransactionQualifier) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTerminalTransactionQualifier);
                        inPacketSizes += strlen(szTerminalTransactionQualifier);
                        
                        /* 9F1A */
                        strcat(szTagContent, "9F1A");
                        inPacketSizes += 4;
                        memset(szTerminalCountryCode, 0x00, sizeof(szTerminalCountryCode));
                        inGetTerminalCountryCode(szTerminalCountryCode);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalCountryCode) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTerminalCountryCode);
                        inPacketSizes += strlen(szTerminalCountryCode);
                        
                        /* DF00 */
                        strcat(szTemplate, "DF0006");
                        strcat(szTemplate, "001000000000");
                        
                        /* DF01 */
                        memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
                        inGetCVMRequiredLimit(szCVMRequiredLimit);
                        lnCVMRequireLimit = atoi(szCVMRequiredLimit);
                        
                        if (lnCVMRequireLimit > 0)
                        {
                                /* 如果金額已經為0時不需再減1元 */
                                lnCVMRequireLimit -= 100;
                        }
                        
                        memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
                        sprintf(szCVMRequiredLimit, "%ld", lnCVMRequireLimit);
                        inFunc_PAD_ASCII(szCVMRequiredLimit, szCVMRequiredLimit, '0', 12, _PADDING_LEFT_);
                        strcat(szTagContent, "DF01");
                        inPacketSizes += 4;
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szCVMRequiredLimit) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szCVMRequiredLimit);
                        inPacketSizes += strlen(szCVMRequiredLimit);
                        
                        /* DF02 */
                        memset(szContactlessFloorLimit, 0x00, sizeof(szContactlessFloorLimit));
                        inGetContactlessFloorLimit(szContactlessFloorLimit);
                        
                        // Tusin_071
                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_071_FORCE_ONLINE_, _CUSTOMER_INDICATOR_SIZE_))
                            lnContactlessFloorLimit = 0;
                        else
                            lnContactlessFloorLimit = atoi(szContactlessFloorLimit);
                        
                        if (lnContactlessFloorLimit > 0)
                        {
                                /* 如果金額已經為0時不需再減1元 */
                                lnContactlessFloorLimit -= 100;
                        }
                        
                        memset(szContactlessFloorLimit, 0x00, sizeof(szContactlessFloorLimit));
                        sprintf(szContactlessFloorLimit, "%ld", lnContactlessFloorLimit);
                        inFunc_PAD_ASCII(szContactlessFloorLimit, szContactlessFloorLimit, '0', 12, _PADDING_LEFT_);
                        strcat(szTagContent, "DF02");
                        inPacketSizes += 4;
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szContactlessFloorLimit) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szContactlessFloorLimit);
                        inPacketSizes += strlen(szContactlessFloorLimit);
                        
                        /* DF05-01 Display Offline Available Fund */
                        strcat(szTagContent, "DF05");
                        inPacketSizes += 4;
                        memset(szDisplayOfflineFunds, 0x00, sizeof(szDisplayOfflineFunds));
                        inGetDisplayOfflineFunds(szDisplayOfflineFunds);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szDisplayOfflineFunds) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szDisplayOfflineFunds);
                        inPacketSizes += strlen(szDisplayOfflineFunds);
                        
                        /* DF8F4D OverWrite */
			
			/* CUP感應的tag不會自己存到chip Data需要加下面的參數 */
			/* DF8F49 Upload DOL */
			/* The format of Tag Upload DOL is (FFC3 + DOL Data Length + DOL Data */
			/* Tag1 + 0x00 + Tag2 + 0x00 + .... + Tagn + 0x00 */
			strcat(szTagContent, "DF8F49");
			inPacketSizes += 6;
			
			/* DF8F4F內容 */
			/* 【需求單 - 106252】	DCC交易新增感應及BIN11碼需求 新增5F28 9F42 by Russell 2020/2/20 上午 11:56 */
			memset(szDF8F4FData, 0x00, sizeof(szDF8F4FData));
			strcpy(szDF8F4FData, "50005A005F20005F2A005F28005F3400820084008A0095009A009B009C009F02009F03009F10009F1A009F1E009F26009F27009F33009F34009F35009F36009F37009F42009F6E00");
			
			/* 全部長度(含FFC3) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%02X", ((strlen("FFC3") + 2 + strlen(szDF8F4FData)) / 2));
			strcat(szTagContent, szTemplate);
			inPacketSizes += 2;
			
			strcat(szTagContent, "FFC3");
			inPacketSizes += 4;
			
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%02X", (strlen(szDF8F4FData) / 2));
			strcat(szTagContent, szTemplate);
			inPacketSizes += 2;
                        
			strcat(szTagContent, szDF8F4FData);
			inPacketSizes += strlen(szDF8F4FData);

                        xmlNodeAddContent(srItem_Node, BAD_CAST szTagContent);
                        xmlAddChild(srGroup_Node, srItem_Node);
                }
		else if (!memcmp(&szMVTApplicationId[0], "A00000002501", 12))
                {
			srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "tlv");
                        memset(szTagContent, 0x00, sizeof(szTagContent));
                        inPacketSizes = 0;

			/* 5F2A-02 Transaction Currcncy Code */
                        strcat(szTagContent, "5F2A");
                        inPacketSizes += 4;
                        memset(szTransactionCurrencyCode, 0x00, sizeof(szTransactionCurrencyCode));
                        inGetVWTTransactionCurrencyCode(szTransactionCurrencyCode);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTransactionCurrencyCode) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTransactionCurrencyCode);
                        inPacketSizes += strlen(szTransactionCurrencyCode);
			
			/* 9C-01 Transaction Type */
                        strcat(szTagContent, "9C");
                        inPacketSizes += 2;
                        memset(szTransactionType, 0x00, sizeof(szTransactionType));
                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
			{
				/* 0x01: cash */
				sprintf(szTransactionType, "01");
			}
			else
			{
				inGetTransactionType(szTransactionType);
			}
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTransactionType) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTransactionType);
                        inPacketSizes += strlen(szTransactionType);
			
			/* 9F1A-02 Terminal Country Code */
                        strcat(szTagContent, "9F1A");
                        inPacketSizes += 4;
                        memset(szTerminalCountryCode, 0x00, sizeof(szTerminalCountryCode));
                        inGetTerminalCountryCode(szTerminalCountryCode);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalCountryCode) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTerminalCountryCode);
                        inPacketSizes += strlen(szTerminalCountryCode);
			
			/* 9F33 */
                        strcat(szTagContent, "9F33");
                        inPacketSizes += 4;
                        memset(szTerminalCapabilities, 0x00, sizeof(szTerminalCapabilities));
                        inGetPaypassTerminalCapabilities(szTerminalCapabilities);
			/* 沒值或空值(判斷第一個為空白) */
			if (strlen(szTerminalCapabilities) == 0	|| memcmp(" ", szTerminalCapabilities, 1) == 0)
			{
				/* 空值抓接觸式 */
				inGetTerminalCapabilities(szTerminalCapabilities);
			}
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalCapabilities) / 2));
                        inPacketSizes += 2;
			/* 虹堡AE是否支援mobileCVM，要使用9F33的byte 2 bit 8來啟用 */
			/* NCCC TTQ byte8~5恰好能對應9F33
			 * Byte 8 (AEIPS:Mobile CVM supported)
			 * Byte 7 (AEIPS:Online PIN supported)
			 * Byte 6 (AEIPS:Signature)
			 * Byte 5 (AEIPS:Plaintext offline PIN)
			  */
                        strcat(szTagContent, szTerminalCapabilities);
                        inPacketSizes += strlen(szTerminalCapabilities);
			
			/* 9F35 Terminal_Type */
			memset(szTerminalType, 0x00, sizeof(szTerminalType));
			inGetPaypassTerminalType(szTerminalType);
                        if (strlen(szTerminalType) == 0	|| memcmp(" ", szTerminalType, 1) == 0)
			{
				/* VWT沒值改抓回MVT，沒填Terminal Type Kernel會無法運行 */
				memset(szTerminalType, 0x00, sizeof(szTerminalType));
				inGetMVTTerminalType(szTerminalType);
				
				strcat(szTagContent, "9F35");
				inPacketSizes += 4;
				
				sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalType) / 2));
				inPacketSizes += 2;
				
				if (strlen(szTerminalType) > 0)
				{
					strcat(szTagContent, szTerminalType);
					inPacketSizes += strlen(szTerminalType);
				}
			}
			else
			{
				strcat(szTagContent, "9F35");
				inPacketSizes += 4;
				strcat(szTagContent, "01");
				inPacketSizes += 2;

				/* 強制縮減長度為2，避免意外 */
				szTerminalType[2] = 0x00;
				strcat(szTagContent, szTerminalType);
				inPacketSizes += strlen(szTerminalType);
			}
						
			/* DF50-06 Contactless Transaction Limit(虹堡才使用DF50) */
                        strcat(szTagContent, "DF50");
                        inPacketSizes += 4;
                        memset(szContactlessTransactionLimit, 0x00, sizeof(szContactlessTransactionLimit));
                        inGetContactlessTransactionLimit(szContactlessTransactionLimit);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szContactlessTransactionLimit) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szContactlessTransactionLimit);
                        inPacketSizes += strlen(szContactlessTransactionLimit);
			
			/* DF53-06 Reader CVM Required Limit(虹堡才使用DF53) */
			/* If the amount is equal to or large than 2000, this this transaction shall perform the CVM. 
			 FFh - Bypass this check
			 Others - Ex : 000000200000 */
                        memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
                        inGetCVMRequiredLimit(szCVMRequiredLimit);
                        lnCVMRequireLimit = atoi(szCVMRequiredLimit);
                        
                        memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
                        sprintf(szCVMRequiredLimit, "%ld", lnCVMRequireLimit);
                        inFunc_PAD_ASCII(szCVMRequiredLimit, szCVMRequiredLimit, '0', 12, _PADDING_LEFT_);
                        strcat(szTagContent, "DF53");
                        inPacketSizes += 4;
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szCVMRequiredLimit) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szCVMRequiredLimit);
                        inPacketSizes += strlen(szCVMRequiredLimit);

			/* DF52-06 Reader Contactless Floor Limit(虹堡才使用DF52) */
			/* If the amount is equal to or large than 1000, this transaction shall go online. 
			 FFh - Bypass this check
			 Others - Ex : 000000100000 */
                        memset(szContactlessFloorLimit, 0x00, sizeof(szContactlessFloorLimit));
                        inGetContactlessFloorLimit(szContactlessFloorLimit);
                        
                        // Tusin_071
                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_071_FORCE_ONLINE_, _CUSTOMER_INDICATOR_SIZE_))
                            lnContactlessFloorLimit = 0;
                        else
                            lnContactlessFloorLimit = atoi(szContactlessFloorLimit);
                        
			/* 條件是larger，所以要減1元 */
                        if (lnContactlessFloorLimit > 0)
                        {
                                /* 如果金額已經為0時不需再減1元 */
                                lnContactlessFloorLimit -= 100;
                        }
                        
                        memset(szContactlessFloorLimit, 0x00, sizeof(szContactlessFloorLimit));
                        sprintf(szContactlessFloorLimit, "%ld", lnContactlessFloorLimit);
                        inFunc_PAD_ASCII(szContactlessFloorLimit, szContactlessFloorLimit, '0', 12, _PADDING_LEFT_);
                        strcat(szTagContent, "DF52");
                        inPacketSizes += 4;
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szContactlessFloorLimit) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szContactlessFloorLimit);
                        inPacketSizes += strlen(szContactlessFloorLimit);
			
			/* DF57-01 AE Expresspay Terminal Capability(虹堡才使用DF57)
			 * EnhancedDDAVersionNum的欄位當AE Expresspay Terminal Capability */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetEnhancedDDAVersionNum(szTemplate);
			inTagLen = strlen(szTemplate);
			
                        strcat(szTagContent, "DF57");
                        inPacketSizes += 4;
                        sprintf(&szTagContent[inPacketSizes], "%02X", (inTagLen / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += inTagLen;
			
			/* DF8120 Paypass Default_TAC */
                        strcat(szTagContent, "DF8120");
                        inPacketSizes += 6;
                        memset(szPaypassDefaultTAC, 0x00, sizeof(szPaypassDefaultTAC));
                        inGetPaypassDefaultTAC(szPaypassDefaultTAC);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen(szPaypassDefaultTAC) / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, szPaypassDefaultTAC);
                        inPacketSizes += strlen(szPaypassDefaultTAC);
			
			/* DF8121 Paypass Denial_TAC */
                        strcat(szTagContent, "DF8121");
                        inPacketSizes += 6;
                        memset(szPaypassDenialTAC, 0x00, sizeof(szPaypassDenialTAC));
                        inGetPaypassDenialTAC(szPaypassDenialTAC);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen(szPaypassDenialTAC) / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, szPaypassDenialTAC);
                        inPacketSizes += strlen(szPaypassDenialTAC);
                        
                        /* DF8122 Paypass Online_TAC */
                        strcat(szTagContent, "DF8122");
                        inPacketSizes += 6;
                        memset(szPaypassOnlineTAC, 0x00, sizeof(szPaypassOnlineTAC));
                        inGetPaypassOnlineTAC(szPaypassOnlineTAC);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (strlen(szPaypassOnlineTAC) / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
                        strcat(szTagContent, szPaypassOnlineTAC);
                        inPacketSizes += strlen(szPaypassOnlineTAC);
			
			/*  Tag DF9F57 (AMEX Mobile CVM Supported) 來設定是否支援 Mobile CVM，此功能至少須EMCVL RAB16才有用 */
                        strcat(szTagContent, "DF9F57");
                        inPacketSizes += 6;
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02X", (2 / 2));
                        strcat(szTagContent, szTemplate);
                        inPacketSizes += 2;
			/* 確認Byte2 Bit8有沒有On*/
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetTerminalTransactionQualifier(szTemplate);
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			szTemplate1[0] = '0';
			szTemplate1[1] = szTemplate[2];
			memset(szBCD, 0x00, sizeof(szBCD));
			inFunc_ASCII_to_BCD((unsigned char*)szBCD, szTemplate1, 1);
			if ((szBCD[0] & 8) == 8)
			{
				strcat(szTagContent, "01");
			}
			else
			{
				strcat(szTagContent, "00");
			}
                        inPacketSizes += 2;
			
			/* 感應的tag不會自己存到chip Data需要加下面的參數 */
			/* DF8F49 Upload DOL */
			/* The format of Tag Upload DOL is (FFC3 + DOL Data Length + DOL Data */
			/* Tag1 + 0x00 + Tag2 + 0x00 + .... + Tagn + 0x00 */
			strcat(szTagContent, "DF8F49");
			inPacketSizes += 6;
			
			/* DF8F4F內容 */
			/* 【需求單 - 106252】	DCC交易新增感應及BIN11碼需求 新增5F28 9F42 by Russell 2020/2/20 上午 11:56 */
			memset(szDF8F4FData, 0x00, sizeof(szDF8F4FData));
			strcpy(szDF8F4FData, "50005A005F20005F2A005F28005F3400820084008A0095009A009B009C009F02009F03009F10009F1A009F1E009F26009F27009F33009F34009F35009F36009F37009F42009F6E00");
			
			/* 全部長度(含FFC3) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%02X", ((strlen("FFC3") + 2 + strlen(szDF8F4FData)) / 2));
			strcat(szTagContent, szTemplate);
			inPacketSizes += 2;
			
			strcat(szTagContent, "FFC3");
			inPacketSizes += 4;
			
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%02X", (strlen(szDF8F4FData) / 2));
			strcat(szTagContent, szTemplate);
			inPacketSizes += 2;
                        
			strcat(szTagContent, szDF8F4FData);
			inPacketSizes += strlen(szDF8F4FData);
                
                        xmlNodeAddContent(srItem_Node, BAD_CAST szTagContent);
                        xmlAddChild(srGroup_Node, srItem_Node);
		}
		/* NCCC */
		else if (!memcmp(&szMVTApplicationId[0], _EMV_AID_TWIN_, _EMV_AID_TWIN_LEN_))
		{
			srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "tlv");
                        memset(szTagContent, 0x00, sizeof(szTagContent));
                        inPacketSizes = 0;

			/* 5F2A-02 Transaction Currcncy Code */
                        strcat(szTagContent, "5F2A");
                        inPacketSizes += 4;
                        memset(szTransactionCurrencyCode, 0x00, sizeof(szTransactionCurrencyCode));
                        inGetVWTTransactionCurrencyCode(szTransactionCurrencyCode);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTransactionCurrencyCode) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTransactionCurrencyCode);
                        inPacketSizes += strlen(szTransactionCurrencyCode);
			
			/* 9C-01 Transaction Type */
                        strcat(szTagContent, "9C");
                        inPacketSizes += 2;
                        memset(szTransactionType, 0x00, sizeof(szTransactionType));
                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
			{
				/* 0x01: cash */
				sprintf(szTransactionType, "01");
			}
			else
			{
				inGetTransactionType(szTransactionType);
			}
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTransactionType) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTransactionType);
                        inPacketSizes += strlen(szTransactionType);
			
			/* 9F1A-02 Terminal Country Code */
                        strcat(szTagContent, "9F1A");
                        inPacketSizes += 4;
                        memset(szTerminalCountryCode, 0x00, sizeof(szTerminalCountryCode));
                        inGetTerminalCountryCode(szTerminalCountryCode);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalCountryCode) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTerminalCountryCode);
                        inPacketSizes += strlen(szTerminalCountryCode);
						
			/* 9F33 */
                        strcat(szTagContent, "9F33");
                        inPacketSizes += 4;
                        memset(szTerminalCapabilities, 0x00, sizeof(szTerminalCapabilities));
                        inGetPaypassTerminalCapabilities(szTerminalCapabilities);
			/* 沒值或空值(判斷第一個為空白) */
			if (strlen(szTerminalCapabilities) == 0	|| memcmp(" ", szTerminalCapabilities, 1) == 0)
			{
				/* 空值抓接觸式 */
				inGetTerminalCapabilities(szTerminalCapabilities);
			}
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalCapabilities) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTerminalCapabilities);
                        inPacketSizes += strlen(szTerminalCapabilities);
			
			/* 9F35 Terminal_Type */
			memset(szTerminalType, 0x00, sizeof(szTerminalType));
			inGetPaypassTerminalType(szTerminalType);
                        if (strlen(szTerminalType) == 0	|| memcmp(" ", szTerminalType, 1) == 0)
			{
				/* VWT沒值改抓回MVT，沒填Terminal Type Kernel會無法運行 */
				memset(szTerminalType, 0x00, sizeof(szTerminalType));
				inGetMVTTerminalType(szTerminalType);
				
				strcat(szTagContent, "9F35");
				inPacketSizes += 4;
				
				sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalType) / 2));
				inPacketSizes += 2;
				
				if (strlen(szTerminalType) > 0)
				{
					strcat(szTagContent, szTerminalType);
					inPacketSizes += strlen(szTerminalType);
				}
			}
			else
			{
				strcat(szTagContent, "9F35");
				inPacketSizes += 4;
				strcat(szTagContent, "01");
				inPacketSizes += 2;

				/* 強制縮減長度為2，避免意外 */
				szTerminalType[2] = 0x00;
				strcat(szTagContent, szTerminalType);
				inPacketSizes += strlen(szTerminalType);
			}
			
			/* 9F66-04 Terminal Transaction Qualifier */
                        strcat(szTagContent, "9F66");
                        inPacketSizes += 4;
                        memset(szTerminalTransactionQualifier, 0x00, sizeof(szTerminalTransactionQualifier));
                        inGetTerminalTransactionQualifier(szTerminalTransactionQualifier);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalTransactionQualifier) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTerminalTransactionQualifier);
                        inPacketSizes += strlen(szTerminalTransactionQualifier);
			
			/* DFAF32-01 Status Check Support flag (1 byte)
			 * (= 0x01 , Status Check is Supported 
			 *  = Other, Status Check is NOT Supported)
			 */
			strcat(szTagContent, "DFAF320100");
			inPacketSizes += 10;
			
			/* DFAF33-01 Zero Amount Allowed flag
			 * (= 0x00, Zero Amount is NOT Allowed
			 *  = Other, Zero Amount is Allowed)
			 */
			strcat(szTagContent, "DFAF330101");
			inPacketSizes += 10;
			
			/* DFAF34-06 Contactless Transaction Limit(虹堡才使用DFAF34) */
                        strcat(szTagContent, "DFAF34");
                        inPacketSizes += 6;
                        memset(szContactlessTransactionLimit, 0x00, sizeof(szContactlessTransactionLimit));
                        inGetContactlessTransactionLimit(szContactlessTransactionLimit);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szContactlessTransactionLimit) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szContactlessTransactionLimit);
                        inPacketSizes += strlen(szContactlessTransactionLimit);
			
			/* DFAF36-06 Reader CVM Required Limit(虹堡才使用DFAF36) */
			/* If the amount is equal to or large than 2000, this this transaction shall perform the CVM. 
			 FFh - Bypass this check
			 Others - Ex : 000000200000 */
                        memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
                        inGetCVMRequiredLimit(szCVMRequiredLimit);
                        lnCVMRequireLimit = atoi(szCVMRequiredLimit);
                        
                        memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
                        sprintf(szCVMRequiredLimit, "%ld", lnCVMRequireLimit);
                        inFunc_PAD_ASCII(szCVMRequiredLimit, szCVMRequiredLimit, '0', 12, _PADDING_LEFT_);
                        strcat(szTagContent, "DFAF36");
                        inPacketSizes += 6;
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szCVMRequiredLimit) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szCVMRequiredLimit);
                        inPacketSizes += strlen(szCVMRequiredLimit);

			/* DFAF35-06 Reader Contactless Floor Limit(虹堡才使用DFAF35) */
			/* If the amount is equal to or large than 1000, this transaction shall go online. 
			 FFh - Bypass this check
			 Others - Ex : 000000100000 */
                        memset(szContactlessFloorLimit, 0x00, sizeof(szContactlessFloorLimit));
                        inGetContactlessFloorLimit(szContactlessFloorLimit);
                        
                        // Tusin_071
                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_071_FORCE_ONLINE_, _CUSTOMER_INDICATOR_SIZE_))
                            lnContactlessFloorLimit = 0;
                        else
                            lnContactlessFloorLimit = atoi(szContactlessFloorLimit);
                        
			/* 條件是larger，所以要減1元 */
                        if (lnContactlessFloorLimit > 0)
                        {
                                /* 如果金額已經為0時不需再減1元 */
                                lnContactlessFloorLimit -= 100;
                        }
                        
                        memset(szContactlessFloorLimit, 0x00, sizeof(szContactlessFloorLimit));
                        sprintf(szContactlessFloorLimit, "%ld", lnContactlessFloorLimit);
                        inFunc_PAD_ASCII(szContactlessFloorLimit, szContactlessFloorLimit, '0', 12, _PADDING_LEFT_);
                        strcat(szTagContent, "DFAF35");
                        inPacketSizes += 6;
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szContactlessFloorLimit) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szContactlessFloorLimit);
                        inPacketSizes += strlen(szContactlessFloorLimit);
			
			/* DF8F49 Upload DOL */
			/* The format of Tag Upload DOL is (FFC3 + DOL Data Length + DOL Data */
			/* Tag1 + 0x00 + Tag2 + 0x00 + .... + Tagn + 0x00 */
			strcat(szTagContent, "DF8F49");
			inPacketSizes += 6;
			
			/* DF8F4F內容 */
			/* 【需求單 - 106252】	DCC交易新增感應及BIN11碼需求 新增5F28 9F42 by Russell 2020/2/20 上午 11:56 */
			memset(szDF8F4FData, 0x00, sizeof(szDF8F4FData));
			strcpy(szDF8F4FData, "50005A005F20005F2A005F28005F3400820084008A0095009A009B009C009F02009F03009F10009F1A009F1E009F26009F27009F33009F34009F35009F36009F37009F42009F6E00");
			
			/* 全部長度(含FFC3) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%02X", ((strlen("FFC3") + 2 + strlen(szDF8F4FData)) / 2));
			strcat(szTagContent, szTemplate);
			inPacketSizes += 2;
			
			strcat(szTagContent, "FFC3");
			inPacketSizes += 4;
			
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%02X", (strlen(szDF8F4FData) / 2));
			strcat(szTagContent, szTemplate);
			inPacketSizes += 2;
                        
			strcat(szTagContent, szDF8F4FData);
			inPacketSizes += strlen(szDF8F4FData);
			
                        xmlNodeAddContent(srItem_Node, BAD_CAST szTagContent);
                        xmlAddChild(srGroup_Node, srItem_Node);
		}
		/* D-PAS*/
		else if (!memcmp(&szMVTApplicationId[0], "A00000015230", 12))
                {
			srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "tlv");
                        memset(szTagContent, 0x00, sizeof(szTagContent));
                        inPacketSizes = 0;

			/* 5F2A-02 Transaction Currcncy Code */
                        strcat(szTagContent, "5F2A");
                        inPacketSizes += 4;
                        memset(szTransactionCurrencyCode, 0x00, sizeof(szTransactionCurrencyCode));
                        inGetVWTTransactionCurrencyCode(szTransactionCurrencyCode);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTransactionCurrencyCode) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTransactionCurrencyCode);
                        inPacketSizes += strlen(szTransactionCurrencyCode);
			
			/* 9C-01 Transaction Type */
                        strcat(szTagContent, "9C");
                        inPacketSizes += 2;
                        memset(szTransactionType, 0x00, sizeof(szTransactionType));
                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
			{
				/* 0x01: cash */
				sprintf(szTransactionType, "01");
			}
			else
			{
				inGetTransactionType(szTransactionType);
			}
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTransactionType) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTransactionType);
                        inPacketSizes += strlen(szTransactionType);
			
			/* 9F1A-02 Terminal Country Code */
                        strcat(szTagContent, "9F1A");
                        inPacketSizes += 4;
                        memset(szTerminalCountryCode, 0x00, sizeof(szTerminalCountryCode));
                        inGetTerminalCountryCode(szTerminalCountryCode);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalCountryCode) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTerminalCountryCode);
                        inPacketSizes += strlen(szTerminalCountryCode);
						
			/* 9F33 */
                        strcat(szTagContent, "9F33");
                        inPacketSizes += 4;
                        memset(szTerminalCapabilities, 0x00, sizeof(szTerminalCapabilities));
                        inGetPaypassTerminalCapabilities(szTerminalCapabilities);
			/* 沒值或空值(判斷第一個為空白) */
			if (strlen(szTerminalCapabilities) == 0	|| memcmp(" ", szTerminalCapabilities, 1) == 0)
			{
				/* 空值抓接觸式 */
				inGetTerminalCapabilities(szTerminalCapabilities);
			}
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalCapabilities) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTerminalCapabilities);
                        inPacketSizes += strlen(szTerminalCapabilities);
			
			/* 9F35 Terminal_Type */
			memset(szTerminalType, 0x00, sizeof(szTerminalType));
			inGetPaypassTerminalType(szTerminalType);
                        if (strlen(szTerminalType) == 0	|| memcmp(" ", szTerminalType, 1) == 0)
			{
				/* VWT沒值改抓回MVT，沒填Terminal Type Kernel會無法運行 */
				memset(szTerminalType, 0x00, sizeof(szTerminalType));
				inGetMVTTerminalType(szTerminalType);
				
				strcat(szTagContent, "9F35");
				inPacketSizes += 4;
				
				sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalType) / 2));
				inPacketSizes += 2;
				
				if (strlen(szTerminalType) > 0)
				{
					strcat(szTagContent, szTerminalType);
					inPacketSizes += strlen(szTerminalType);
				}
			}
			else
			{
				strcat(szTagContent, "9F35");
				inPacketSizes += 4;
				strcat(szTagContent, "01");
				inPacketSizes += 2;

				/* 強制縮減長度為2，避免意外 */
				szTerminalType[2] = 0x00;
				strcat(szTagContent, szTerminalType);
				inPacketSizes += strlen(szTerminalType);
			}
			
			/* 9F66-04 Terminal Transaction Qualifier */
                        strcat(szTagContent, "9F66");
                        inPacketSizes += 4;
                        memset(szTerminalTransactionQualifier, 0x00, sizeof(szTerminalTransactionQualifier));
                        inGetTerminalTransactionQualifier(szTerminalTransactionQualifier);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szTerminalTransactionQualifier) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szTerminalTransactionQualifier);
                        inPacketSizes += strlen(szTerminalTransactionQualifier);
			
			/* DFAF32-01 Status Check Support flag (1 byte)
			 * (= 0x01 , Status Check is Supported 
			 *  = Other, Status Check is NOT Supported)
			 */
			strcat(szTagContent, "DFAF320100");
			inPacketSizes += 10;
			
			/* DFAF33-01 Zero Amount Allowed flag
			 * (= 0x00, Zero Amount is NOT Allowed
			 *  = Other, Zero Amount is Allowed)
			 */
			strcat(szTagContent, "DFAF330101");
			inPacketSizes += 10;
			
			/* DFAF34-06 Contactless Transaction Limit(虹堡才使用DFAF34) */
                        strcat(szTagContent, "DFAF34");
                        inPacketSizes += 6;
                        memset(szContactlessTransactionLimit, 0x00, sizeof(szContactlessTransactionLimit));
                        inGetContactlessTransactionLimit(szContactlessTransactionLimit);
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szContactlessTransactionLimit) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szContactlessTransactionLimit);
                        inPacketSizes += strlen(szContactlessTransactionLimit);
			
			/* DFAF36-06 Reader CVM Required Limit(虹堡才使用DFAF36) */
			/* If the amount is equal to or large than 2000, this this transaction shall perform the CVM. 
			 FFh - Bypass this check
			 Others - Ex : 000000200000 */
                        memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
                        inGetCVMRequiredLimit(szCVMRequiredLimit);
                        lnCVMRequireLimit = atoi(szCVMRequiredLimit);
                        
                        memset(szCVMRequiredLimit, 0x00, sizeof(szCVMRequiredLimit));
                        sprintf(szCVMRequiredLimit, "%ld", lnCVMRequireLimit);
                        inFunc_PAD_ASCII(szCVMRequiredLimit, szCVMRequiredLimit, '0', 12, _PADDING_LEFT_);
                        strcat(szTagContent, "DFAF36");
                        inPacketSizes += 6;
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szCVMRequiredLimit) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szCVMRequiredLimit);
                        inPacketSizes += strlen(szCVMRequiredLimit);

			/* DFAF35-06 Reader Contactless Floor Limit(虹堡才使用DFAF35) */
			/* If the amount is equal to or large than 1000, this transaction shall go online. 
			 FFh - Bypass this check
			 Others - Ex : 000000100000 */
                        memset(szContactlessFloorLimit, 0x00, sizeof(szContactlessFloorLimit));
                        inGetContactlessFloorLimit(szContactlessFloorLimit);
                        
                        // Tusin_071
                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_071_FORCE_ONLINE_, _CUSTOMER_INDICATOR_SIZE_))
                            lnContactlessFloorLimit = 0;
                        else
                            lnContactlessFloorLimit = atoi(szContactlessFloorLimit);
                        
			/* 條件是larger，所以要減1元 */
                        if (lnContactlessFloorLimit > 0)
                        {
                                /* 如果金額已經為0時不需再減1元 */
                                lnContactlessFloorLimit -= 100;
                        }
//			else
//			{
//				lnContactlessFloorLimit = 100;
//			}
                        
                        memset(szContactlessFloorLimit, 0x00, sizeof(szContactlessFloorLimit));
                        sprintf(szContactlessFloorLimit, "%ld", lnContactlessFloorLimit);
                        inFunc_PAD_ASCII(szContactlessFloorLimit, szContactlessFloorLimit, '0', 12, _PADDING_LEFT_);
                        strcat(szTagContent, "DFAF35");
                        inPacketSizes += 6;
                        sprintf(&szTagContent[inPacketSizes], "%02X", (strlen(szContactlessFloorLimit) / 2));
                        inPacketSizes += 2;
                        strcat(szTagContent, szContactlessFloorLimit);
                        inPacketSizes += strlen(szContactlessFloorLimit);
			
			/* DF8F49 Upload DOL */
			/* The format of Tag Upload DOL is (FFC3 + DOL Data Length + DOL Data */
			/* Tag1 + 0x00 + Tag2 + 0x00 + .... + Tagn + 0x00 */
			strcat(szTagContent, "DF8F49");
			inPacketSizes += 6;
			
			/* DF8F4F內容 */
			/* 【需求單 - 106252】	DCC交易新增感應及BIN11碼需求 新增5F28 9F42 by Russell 2020/2/20 上午 11:56 */
			memset(szDF8F4FData, 0x00, sizeof(szDF8F4FData));
			strcpy(szDF8F4FData, "50005A005F20005F2A005F28005F3400820084008A0095009A009B009C009F02009F03009F10009F1A009F1E009F26009F27009F33009F34009F35009F36009F37009F42009F6E00");
			
			/* 全部長度(含FFC3) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%02X", ((strlen("FFC3") + 2 + strlen(szDF8F4FData)) / 2));
			strcat(szTagContent, szTemplate);
			inPacketSizes += 2;
			
			strcat(szTagContent, "FFC3");
			inPacketSizes += 4;
			
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%02X", (strlen(szDF8F4FData) / 2));
			strcat(szTagContent, szTemplate);
			inPacketSizes += 2;
                        
			strcat(szTagContent, szDF8F4FData);
			inPacketSizes += strlen(szDF8F4FData);
			
                        xmlNodeAddContent(srItem_Node, BAD_CAST szTagContent);
                        xmlAddChild(srGroup_Node, srItem_Node);
		}
                else
		{
                        continue;
		}
        } /* end for */
        
        /* CAPKConfig */
        srCAPK_Node = xmlNewNode(NULL, BAD_CAST "CAPKConfig");
        xmlAddChild(srCLConfig_Node, srCAPK_Node);
        
        memset(szCAPKId_BAK, 0x00, sizeof(szCAPKId_BAK));
        
        for (i = 0 ;; i ++)
        {
                if (inLoadESTRec(i) < 0)
                        break;
      
                memset(szCAPKId, 0x00, sizeof(szCAPKId));
                inGetCAPKApplicationId(szCAPKId);
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "RID: %s", szCAPKId);
                        inLogPrintf(AT, szTemplate);
                }
	
                /* 比對前一次的CAPK 相同的話用同一個父標籤 */
                if (!memcmp(&szCAPKId_BAK[0], &szCAPKId[0], 10))
                {
                        /* 相同CAPK ID */
                }
                else
                {
                        /* Copy現在的CAPK到CAPK_BAK */
                        memset(szCAPKId_BAK, 0x00 ,sizeof(szCAPKId_BAK));
                        memcpy(&szCAPKId_BAK[0], &szCAPKId[0], 10);
                        
                        srGroup_Node = xmlNewNode(NULL, BAD_CAST "Group");
                        xmlNewProp(srGroup_Node, BAD_CAST "RID", BAD_CAST szCAPKId);
                        xmlAddChild(srCAPK_Node, srGroup_Node);
                }

                /* index */
                memset(szCAPKIndex, 0x00, sizeof(szCAPKIndex));
                inGetCAPKIndex(szCAPKIndex);
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "Index: %s", szCAPKIndex);
                        inLogPrintf(AT, szTemplate);
                }
                
                srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
                xmlNewProp(srItem_Node, BAD_CAST "index", BAD_CAST szCAPKIndex);
                xmlAddChild(srGroup_Node, srItem_Node);

                /* modules */
                memset(szCAPKKeyModulus, 0x00, sizeof(szCAPKKeyModulus));
                inGetCAPKKeyModulus(szCAPKKeyModulus);
                xmlNewChild(srItem_Node, NULL, BAD_CAST "modules", BAD_CAST szCAPKKeyModulus);
                
                /* exponent */
                memset(szCAPKExponent, 0x00, sizeof(szCAPKExponent));
                inGetCAPKExponent(szCAPKExponent);
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "Exponent: %s", szCAPKExponent);
                        inLogPrintf(AT, szTemplate);
                }
                
                xmlNewChild(srItem_Node, NULL, BAD_CAST "exponent", BAD_CAST szCAPKExponent);

                /* expirydata */
                /* CAPK Key值有效期(端末機暫不參考CAPK有效期限) by 聯合TMS規格 */
                xmlNewChild(srItem_Node, NULL, BAD_CAST "expirydata", NULL);
                
                /* hash */
                /* 組CAPK Data CAPK_ID + Index + Modulus + Exponet */
                /* 第一部份 ID + Index */
                memset(szCAPKData, 0x00, sizeof(szCAPKData));
                memset(uszBCD, 0x00, sizeof(uszBCD));
                memset(szCAPKIdIndex, 0x00, sizeof(szCAPKIdIndex));
                strcpy(szCAPKIdIndex, szCAPKId);
                strcat(szCAPKIdIndex, szCAPKIndex);
                inBCDLen = strlen(szCAPKIdIndex) / 2;
                inFunc_ASCII_to_BCD(&uszBCD[0], &szCAPKIdIndex[0], inBCDLen);
                memcpy(&szCAPKData[0], &uszBCD[0], inBCDLen);
                inCAPK_DataLen = inBCDLen;

                /* 第二部份 Modulus */
                memset(uszBCD, 0x00, sizeof(uszBCD));
                inBCDLen = strlen(szCAPKKeyModulus) / 2;
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "CAPK Key Length: %d", inBCDLen);
                        inLogPrintf(AT, szTemplate);
                }
                
                inFunc_ASCII_to_BCD(&uszBCD[0], &szCAPKKeyModulus[0], inBCDLen);
                memcpy(&szCAPKData[inCAPK_DataLen], &uszBCD[0], inBCDLen);
                inCAPK_DataLen = inCAPK_DataLen + inBCDLen;
                
                /* 第三部份 exponent */
                memset(uszBCD, 0x00, sizeof(uszBCD));
                inBCDLen = strlen(szCAPKExponent) / 2;
                inFunc_ASCII_to_BCD(&uszBCD[0], &szCAPKExponent[0], inBCDLen);
                memcpy(&szCAPKData[inCAPK_DataLen], &uszBCD[0], inBCDLen);
                inCAPK_DataLen = inCAPK_DataLen + inBCDLen;

                /* 計算Hash */
                memset(&srSHA, 0x00, sizeof(SHA_CTX));
                /* Initialize the SHA_CTX structure and perpart for the SHA1 operation */
                CTOS_SHA1Init(&srSHA);
                /* Perform the SHA1 algorithm with the input data */
                CTOS_SHA1Update(&srSHA, (unsigned char *)szCAPKData, inCAPK_DataLen);
                /* Finalize the SA1 operation and retrun the result */
                memset(uszHash, 0x00, sizeof(uszHash));
                CTOS_SHA1Final(uszHash, &srSHA);
                
                /* 20Bytes的HASH須轉成ASCII存入 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inFunc_BCD_to_ASCII(szTemplate, uszHash, 40);
                memset(szCAPKHash, 0x00, sizeof(szCAPKHash));
                memcpy(&szCAPKHash[0], &szTemplate[0], 40);
                
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "Hash");
                        inLogPrintf(AT, szCAPKHash);
                }
                
                xmlNewChild(srItem_Node, NULL, BAD_CAST "hash", BAD_CAST szCAPKHash);
        }
        
        /* ParametersConfig */
        srParametersConfig_Node = xmlNewNode(NULL, BAD_CAST "ParametersConfig");
        xmlAddChild(srCLConfig_Node, srParametersConfig_Node);
        
        /* Sale Timeout (ms) */
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "ParaIndex", BAD_CAST "0002");	/* 新library此Tag最大支援到4 Bytes */
        xmlNodeAddContent(srItem_Node, BAD_CAST "FFFFFFFF");		/* 設定最大秒數，由程式流程來控制是否Timeout */
        xmlAddChild(srParametersConfig_Node, srItem_Node);
        
        /* UI Type */
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "ParaIndex", BAD_CAST "100A");
        xmlNodeAddContent(srItem_Node, BAD_CAST "00");
        xmlAddChild(srParametersConfig_Node, srItem_Node);
        
        /* Visa EUR CL TIG Follow */
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "ParaIndex", BAD_CAST "100B");
        xmlNodeAddContent(srItem_Node, BAD_CAST "00");
        xmlAddChild(srParametersConfig_Node, srItem_Node);
	
	/* 加入這個Tag才可以使用AGOGO icash */
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "ParaIndex", BAD_CAST "3004");
        xmlNodeAddContent(srItem_Node, BAD_CAST "01");
        xmlAddChild(srParametersConfig_Node, srItem_Node);
	
	/* 加入這個Tag才會回傳d_EMVCL_RC_SEE_PHONE */
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "ParaIndex", BAD_CAST "3008");
        xmlNodeAddContent(srItem_Node, BAD_CAST "01");
        xmlAddChild(srParametersConfig_Node, srItem_Node);
        
        /* 儲存xml檔 xml檔現在會自動換行了*/
        inRetVal = inXML_SaveFile(szConfigFileName, &srDoc, "utf-8", 1);
        
        if (ginDebug == VS_TRUE)
        {
                sprintf(szTemplate, "inRetVal = %d", inRetVal);
                inLogPrintf(AT, szTemplate);
        }
        
        /* 清空佔用記憶體 */
	inXML_End(&srDoc);
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inEMVXML_Create_CTLSConfigXml()_END");

        return (VS_SUCCESS);
}

void vdEMVXML_write_test(void)
{
        int inRetVal;
        char szTemplate[64 + 1];
        xmlDocPtr srDoc = NULL;
        xmlNodePtr srRoot_Node = NULL, srConfig_Node = NULL, srCAPK_Node = NULL, 
                   srAppList_Node = NULL, srTerminalConfig_Node = NULL, srAppConfig_Node = NULL,
                   srGroup_Node = NULL, srItem_Node = NULL;
//        xmlNodePtr        pModules_Node = NULL, pExponent_Node = NULL,
//                   pExpirydata_Node = NULL, pHash_Node = NULL;
//        xmlNodePtr content = NULL;
//        int     i;
//        char    szApplicationId[10 + 1];        /* 應用程式 ID */
//        char    szCAPKIndex[2 + 1];             /* CAPK索引值(16進位) */
//        char    szCAPKKeyModulus[496 + 1];      /* CAPK Key值 (原資料為 Binary 248 Bytes，故轉成 ASC 496 bytes) */
//        char    szCAPKExponent[6 + 1];          /* CAPK Key值 Exponent (原資料為 Binary 3 Bytes，故轉成 ASC 6 bytes) */
        
        /* 
         * Creates a new document, a node and set it as a root node
         */
        srDoc = xmlNewDoc(BAD_CAST"1.0");
        srRoot_Node = xmlNewNode(NULL, BAD_CAST "configurationDescriptor");
        xmlNewProp(srRoot_Node, BAD_CAST "version", BAD_CAST "01");    
        xmlDocSetRootElement (srDoc, srRoot_Node);
        
        
        srConfig_Node = xmlNewNode(NULL, BAD_CAST "Config");
        xmlNewProp(srConfig_Node, BAD_CAST "index", BAD_CAST "01");
        xmlNewProp(srConfig_Node, BAD_CAST "active", BAD_CAST "true" );
        //add Config in configurationDescriptor
        xmlAddChild(srRoot_Node, srConfig_Node);
        
        /* CAPKConfig */
        srCAPK_Node = xmlNewNode(NULL, BAD_CAST "CAPKConfig");
        
        
        srGroup_Node = xmlNewNode(NULL, BAD_CAST "Group");
        xmlNewProp(srGroup_Node, BAD_CAST "RID", BAD_CAST "A000000003");
        xmlAddChild(srCAPK_Node, srGroup_Node);
        xmlAddChild(srConfig_Node, srCAPK_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "index", BAD_CAST "50");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"modules", BAD_CAST"D11197590057B84196C2F4D11A8F3C05408F422A35D702F90106EA5B019BB28AE607AA9CDEBCD0D81A38D48C7EBB0062D287369EC0C42124246AC30D80CD602AB7238D51084DED4698162C59D25EAC1E66255B4DB2352526EF0982C3B8AD3D1CCE85B01DB5788E75E09F44BE7361366DEF9D1E1317B05E5D0FF5290F88A0DB47");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"exponent", BAD_CAST"010001");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"expirydata", BAD_CAST"");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"hash", BAD_CAST"B4538DC5FD3416852EAE56154D07341D21E3F4CE");
        xmlAddChild(srGroup_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "index", BAD_CAST "51");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"modules", BAD_CAST"DB5FA29D1FDA8C1634B04DCCFF148ABEE63C772035C79851D3512107586E02A917F7C7E885E7C4A7D529710A145334CE67DC412CB1597B77AA2543B98D19CF2CB80C522BDBEA0F1B113FA2C86216C8C610A2D58F29CF3355CEB1BD3EF410D1EDD1F7AE0F16897979DE28C6EF293E0A19282BD1D793F1331523FC71A228800468C01A3653D14C6B4851A5C029478E757F");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"exponent", BAD_CAST"03");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"expirydata", BAD_CAST"");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"hash", BAD_CAST"AF08EFF20DA86FDA1259519B592372A9E0A71E2B");
        xmlAddChild(srGroup_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "index", BAD_CAST "53");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"modules", BAD_CAST"BCD83721BE52CCCC4B6457321F22A7DC769F54EB8025913BE804D9EABBFA19B3D7C5D3CA658D768CAF57067EEC83C7E6E9F81D0586703ED9DDDADD20675D63424980B10EB364E81EB37DB40ED100344C928886FF4CCC37203EE6106D5B59D1AC102E2CD2D7AC17F4D96C398E5FD993ECB4FFDF79B17547FF9FA2AA8EEFD6CBDA124CBB17A0F8528146387135E226B005A474B9062FF264D2FF8EFA36814AA2950065B1B04C0A1AE9B2F69D4A4AA979D6CE95FEE9485ED0A03AEE9BD953E81CFD1EF6E814DFD3C2CE37AEFA38C1F9877371E91D6A5EB59FDEDF75D3325FA3CA66CDFBA0E57146CC789818FF06BE5FCC50ABD362AE4B80996D");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"exponent", BAD_CAST"03");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"expirydata", BAD_CAST"");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"hash", BAD_CAST"BDE69CBE66F27879C11E15E7628A3B126F34F2EE");
        xmlAddChild(srGroup_Node, srItem_Node);

        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "index", BAD_CAST "96");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"modules", BAD_CAST"B74586D19A207BE6627C5B0AAFBC44A2ECF5A2942D3A26CE19C4FFAEEE920521868922E893E7838225A3947A2614796FB2C0628CE8C11E3825A56D3B1BBAEF783A5C6A81F36F8625395126FA983C5216D3166D48ACDE8A431212FF763A7F79D9EDB7FED76B485DE45BEB829A3D4730848A366D3324C3027032FF8D16A1E44D8D");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"exponent", BAD_CAST"03");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"expirydata", BAD_CAST"");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"hash", BAD_CAST"27072A41BE8A13DCBE3C3ACD3087DCDF130A02CE");
        xmlAddChild(srGroup_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "index", BAD_CAST "94");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"modules", BAD_CAST"D1BE39615F395AC9337E3307AA5A7AC35EAE0036BF20B92F9A45D190B2F4616ABF9D340CBF5FBB3A2B94BD8F2F977C0A10B90E59D4201AA32669E8CBE753F536119DF4FB5E63CED87F1153CE914B124F3E6B648CD5C97655F7AB4DF62607C95DA50517AB8BE3836672D1C71BCDE9BA7293FF3482F124F86691130AB08177B02F459C025A1F3DFFE0884CE78122542EA1C8EA092B552B586907C83AD65E0C6F91A400E485E11192AA4C171C5A1EF56381F4D091CC7EF6BD8604CBC4C74D5D77FFA07B641D53998CDB5C21B7BC65E082A6513F424A4B252E0D77FA4056986A0AB0CDA6155ED9A883C69CC2992D49ECBD4797DD2864FFC96B8D");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"exponent", BAD_CAST"010001");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"expirydata", BAD_CAST"");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"hash", BAD_CAST"08531878494DE233304AEDD0836B4A25244207AA");
        xmlAddChild(srGroup_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "index", BAD_CAST "97");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"modules", BAD_CAST"AF0754EAED977043AB6F41D6312AB1E22A6809175BEB28E70D5F99B2DF18CAE73519341BBBD327D0B8BE9D4D0E15F07D36EA3E3A05C892F5B19A3E9D3413B0D97E7AD10A5F5DE8E38860C0AD004B1E06F4040C295ACB457A788551B6127C0B29");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"exponent", BAD_CAST"03");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"expirydata", BAD_CAST"");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"hash", BAD_CAST"48176F47CE1F3E77C44C3548BFFEFF8682F4B39F");
        xmlAddChild(srGroup_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "index", BAD_CAST "95");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"modules", BAD_CAST"BE9E1FA5E9A803852999C4AB432DB28600DCD9DAB76DFAAA47355A0FE37B1508AC6BF38860D3C6C2E5B12A3CAAF2A7005A7241EBAA7771112C74CF9A0634652FBCA0E5980C54A64761EA101A114E0F0B5572ADD57D010B7C9C887E104CA4EE1272DA66D997B9A90B5A6D624AB6C57E73C8F919000EB5F684898EF8C3DBEFB330C62660BED88EA78E909AFF05F6DA627B");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"exponent", BAD_CAST"03");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"expirydata", BAD_CAST"");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"hash", BAD_CAST"EE1511CEC71020A9B90443B37B1D5F6E703030F6");
        xmlAddChild(srGroup_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "index", BAD_CAST "57");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"modules", BAD_CAST"942B7F2BA5EA307312B63DF77C5243618ACC2002BD7ECB74D821FE7BDC78BF28F49F74190AD9B23B9713B140FFEC1FB429D93F56BDC7ADE4AC075D75532C1E590B21874C7952F29B8C0F0C1CE3AEEDC8DA25343123E71DCF86C6998E15F756E3");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"exponent", BAD_CAST"010001");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"expirydata", BAD_CAST"");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"hash", BAD_CAST"F9862BDAB2D788E622DA7F0B701C04BE97DC3631");
        xmlAddChild(srGroup_Node, srItem_Node);
        
        /* AppList */
        srAppList_Node = xmlNewNode(NULL, BAD_CAST "AppList");
        xmlAddChild(srConfig_Node, srAppList_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "index", BAD_CAST "01");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"Name", BAD_CAST"VISA");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"AID", BAD_CAST"A0000000031010");
        xmlNewChild(srItem_Node, NULL, BAD_CAST"ASI", BAD_CAST"00");
        xmlAddChild(srAppList_Node, srItem_Node);
        
        
        
        /* TerminalConfig */
        srTerminalConfig_Node = xmlNewNode(NULL, BAD_CAST "TerminalConfig");
        xmlAddChild(srConfig_Node, srTerminalConfig_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "TERMINAL CAPABILITIES");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "9F33");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
//        content = xmlNewText(BAD_CAST "E0F1C8");
//        xmlAddChild(srItem_Node, content);
        xmlNodeAddContent(srItem_Node, BAD_CAST "E0F1C8");
//        xmlAddChild(pnode1,xmlNewText (BAD_CAST "這是更低的節點,子子子節點1" ));
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "ADDITIONAL TERMINAL CAPABILITIES");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "9F40");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
//        content = xmlNewText(BAD_CAST "F000F0A001");
//        xmlAddChild(srItem_Node, content);
        xmlNodeAddContent(srItem_Node, BAD_CAST "F000F0A001");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "INTERFACE DEVICE (IFD) SERIAL NUMBER");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "9F1E");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "asc");
//        content = xmlNewText(BAD_CAST "12345678");
//        xmlAddChild(srItem_Node, content);
        xmlNodeAddContent(srItem_Node, BAD_CAST "12345678");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "TRANSACTION CURRENCY CODE");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "5F2A");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
//        content = xmlNewText(BAD_CAST "0949");
//        xmlAddChild(srItem_Node, content);
        xmlNodeAddContent(srItem_Node, BAD_CAST "0949");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "TERMINAL COUNTRY CODE");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "9F1A");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
//        content = xmlNewText(BAD_CAST "0792");
//        xmlAddChild(srItem_Node, content);
        xmlNodeAddContent(srItem_Node, BAD_CAST "0792");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);

        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "Default TDOL");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC0");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
//        content = xmlNewText(BAD_CAST "9F02065F2A029A039C0195059F3704");
//        xmlAddChild(srItem_Node, content);
        xmlNodeAddContent(srItem_Node, BAD_CAST "9F0206");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "Default DDOL");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC1");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
//        content = xmlNewText(BAD_CAST "9F3704");
//        xmlAddChild(srItem_Node, content);
        xmlNodeAddContent(srItem_Node, BAD_CAST "9F3704");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "App Version");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "9F09");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
//        content = xmlNewText(BAD_CAST "0084");
//        xmlAddChild(srItem_Node, content);
        xmlNodeAddContent(srItem_Node, BAD_CAST "0084");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "Termian Floor Limit");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "9F1B");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
//        content = xmlNewText(BAD_CAST "000003E8");
//        xmlAddChild(srItem_Node, content);
        xmlNodeAddContent(srItem_Node, BAD_CAST "000003E8");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "Threshold Value");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC4");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
//        content = xmlNewText(BAD_CAST "00000005");
//        xmlAddChild(srItem_Node, content);
        xmlNodeAddContent(srItem_Node, BAD_CAST "00000005");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "Target Percent");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC2");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
//        content = xmlNewText(BAD_CAST "20");
//        xmlAddChild(srItem_Node, content);
        xmlNodeAddContent(srItem_Node, BAD_CAST "20");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "Max Target Percent");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC3");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
//        content = xmlNewText(BAD_CAST "40");
//        xmlAddChild(srItem_Node, content);
        xmlNodeAddContent(srItem_Node, BAD_CAST "40");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "TAC Denial");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC6");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
//        content = xmlNewText(BAD_CAST "0000000000");
//        xmlAddChild(srItem_Node, content);
        xmlNodeAddContent(srItem_Node, BAD_CAST "0000000000");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
       
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "TAC Online");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC7");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
//        content = xmlNewText(BAD_CAST "0000000000");
//        xmlAddChild(srItem_Node, content);
        xmlNodeAddContent(srItem_Node, BAD_CAST "0000000000");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "TAC Default");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC8");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
//        content = xmlNewText(BAD_CAST "0000000000");
//        xmlAddChild(srItem_Node, content);
        xmlNodeAddContent(srItem_Node, BAD_CAST "0000000000");
        xmlAddChild(srTerminalConfig_Node, srItem_Node);
        
        /* AppConfig */
        srAppConfig_Node = xmlNewNode(NULL, BAD_CAST "AppConfig");
        xmlAddChild(srConfig_Node, srAppConfig_Node);
        
        srGroup_Node = xmlNewNode(NULL, BAD_CAST "Group");
        xmlNewProp(srGroup_Node, BAD_CAST "name", BAD_CAST "VISA");
        xmlNewProp(srGroup_Node, BAD_CAST "AID", BAD_CAST "A0000000031010");
        xmlNewProp(srGroup_Node, BAD_CAST "ASI", BAD_CAST "00");
        xmlAddChild(srAppConfig_Node, srGroup_Node);
        
        

        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "App Version");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "9F09");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
//        content = xmlNewText(BAD_CAST "008C");
//        xmlAddChild(srItem_Node, content);
        xmlNodeAddContent(srItem_Node, BAD_CAST "008C");
        xmlAddChild(srGroup_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "Termian Floor Limit");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "9F1B");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
//        content = xmlNewText(BAD_CAST "00002710");
//        xmlAddChild(srItem_Node, content);
        xmlNodeAddContent(srItem_Node, BAD_CAST "00002710");
        xmlAddChild(srGroup_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "Threshold Value");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC4");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
//        content = xmlNewText(BAD_CAST "00000005");
//        xmlAddChild(srItem_Node, content);
        xmlNodeAddContent(srItem_Node, BAD_CAST "00000005");
        xmlAddChild(srGroup_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "Target Percent");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC2");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
//        content = xmlNewText(BAD_CAST "50");
//        xmlAddChild(srItem_Node, content);
        xmlNodeAddContent(srItem_Node, BAD_CAST "50");
        xmlAddChild(srGroup_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "Max Target Percent");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC3");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
//        content = xmlNewText(BAD_CAST "99");
//        xmlAddChild(srItem_Node, content);
        xmlNodeAddContent(srItem_Node, BAD_CAST "99");
        xmlAddChild(srGroup_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "TAC Denial");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC6");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
//        content = xmlNewText(BAD_CAST "0010000000");
//        xmlAddChild(srItem_Node, content);
        xmlNodeAddContent(srItem_Node, BAD_CAST "0010000000");
        xmlAddChild(srGroup_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "TAC Online");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC7");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
//        content = xmlNewText(BAD_CAST "DC4004F800");
//        xmlAddChild(srItem_Node, content);
        xmlNodeAddContent(srItem_Node, BAD_CAST "DC4004F800");
        xmlAddChild(srGroup_Node, srItem_Node);
        
        srItem_Node = xmlNewNode(NULL, BAD_CAST "Item");
        xmlNewProp(srItem_Node, BAD_CAST "name", BAD_CAST "TAC Default");
        xmlNewProp(srItem_Node, BAD_CAST "tag", BAD_CAST "DFC8");
        xmlNewProp(srItem_Node, BAD_CAST "attribute", BAD_CAST "hex");
//        content = xmlNewText(BAD_CAST "DC4000A800");
//        xmlAddChild(srItem_Node, content);
        xmlNodeAddContent(srItem_Node, BAD_CAST "DC4000A800");
        xmlAddChild(srGroup_Node, srItem_Node);
        

        
        
//        inXML_SaveFile ("emv_config.xml", &srDoc, "UTF-8", 1);
//        inRetVal = inXML_SaveFile("emv_config.xml", &srDoc, "UTF-8");
        inRetVal = inXML_SaveFile("emv_config.xml", &srDoc, "utf-8", 1);
        sprintf(szTemplate, "inRetVal = %d", inRetVal);
        inLogPrintf(AT, szTemplate);
        
        
        /* 清空佔用記憶體 */
	inXML_End(&srDoc);

        system("cd home/ap/pub/" );
        system("cp emv_config.xml /media/mdisk/");
}

int inEMVXML_LoadXML(void)
{
	inFunc_Data_Copy("emv_config.xml", _AP_ROOT_PATH_, "", _SD_PATH_);
	inFunc_Data_Copy("CTLS_config.xml", _AP_ROOT_PATH_, "", _SD_PATH_);
	
	return (VS_SUCCESS);
}

/*
Function        :inEMVXML_Update_EMV_XML
Date&Time       :2019/12/6 上午 11:20
Describe        :
*/
int inEMVXML_Update_EMV_XML(char* szConfigFileName, char* szDestination)
{
	int	inRetVal = VS_ERROR;
	
	do
	{
		inRetVal = inEMVXML_Create_EMVConfigXml(szConfigFileName);
		if (inRetVal != VS_SUCCESS)
		{
			break;
		}
		inFile_Unlink_File(szConfigFileName, szDestination);

		inRetVal = inFile_Move_File(szConfigFileName, _AP_ROOT_PATH_, "", szDestination);
		if (inRetVal != VS_SUCCESS)
		{
			break;
		}
		break;
	}while(1);
	
	return (inRetVal);
}

/*
Function        :inEMVXML_Update_CTLS_XML
Date&Time       :2019/12/6 上午 11:20
Describe        :
*/
int inEMVXML_Update_CTLS_XML(char* szConfigFileName, char* szDestination)
{
	int	inRetVal = VS_ERROR;
	
	do
	{
		inRetVal = inEMVXML_Create_CTLSConfigXml(szConfigFileName);
		if (inRetVal != VS_SUCCESS)
		{
			break;
		}
		inFile_Unlink_File(szConfigFileName, szDestination);

		inRetVal = inFile_Move_File(szConfigFileName, _AP_ROOT_PATH_, "", szDestination);
		if (inRetVal != VS_SUCCESS)
		{
			break;
		}
		break;
	}while(1);
	
	return (inRetVal);
}
