#define _EMV_CONFIG_FILENAME_       		"emv_config.xml"
#define _EMVCL_CONFIG_FILENAME_       		"CTLS_config.xml"

int inEMVXML_Create_EMVConfigXml(char* szConfigFileName);
int inEMVXML_Create_CTLSConfigXml(char* szConfigFileName);
void vdEMVXML_write_test(void);
int inEMVXML_LoadXML(void);
int inEMVXML_Update_EMV_XML(char* szConfigFileName, char* szDestination);
int inEMVXML_Update_CTLS_XML(char* szConfigFileName, char* szDestination);
