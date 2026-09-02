/* 
 * File:   TLS.h
 * Author: user
 *
 * Created on 2017年7月28日, 下午 4:53
 */

#ifdef _TEST_CERTIFICATE_
	#define _PEM_IFES_FILE_NAME_		"nccc.com.tw.pem"
	#define _PEM_LDSS_FILE_NAME_		"nccc.com.tw.pem"
	#define _PEM_IFES_DEMO_FILE_NAME_	"nccc.com.tw.pem"
	#define _PEM_LDSS_DEMO_FILE_NAME_	"nccc.com.tw.pem"
	#define _PEM_NEW_TLS_FILE_NAME_		"TMS_new.pem"
	#define _PEM_NEW_TLS_BACKUP_FILE_NAME_	"TMS_new.pem.bak"
	#define _PEM_PRESERVE_TLS_FILE_NAME_	"TMS_preserver.pem"
#else
	#define _PEM_IFES_FILE_NAME_		"ifes.nccc.com.tw.pem"//20260206 這參數已沒在使用
	#define _PEM_LDSS_FILE_NAME_		"ldss.nccc.com.tw.pem"//20260206 這參數已沒在使用
	#define _PEM_IFES_DEMO_FILE_NAME_	"nccc.com.tw.pem"
	#define _PEM_LDSS_DEMO_FILE_NAME_	"nccc.com.tw.pem"
	#define _PEM_NEW_TLS_FILE_NAME_		"TMS_new.pem"
	#define _PEM_NEW_TLS_BACKUP_FILE_NAME_	"TMS_new.pem.bak"
	#define _PEM_PRESERVE_TLS_FILE_NAME_	"TMS_preserver.pem"
#endif

#define _CER_UCA_FILE_NAME_			"UCA.CER"
#define _CER_UCA_FILE_NAME_LEN_		7
#define _CER_ROOT_FILE_NAME_			"ROOT.CER"
#define _CER_ROOT_FILE_NAME_LEN_		8

/*[260206][114215-TWCA憑證升級] 新增憑證路徑 */
#define _CER_UCA2_FILE_NAME_			"UCA2.CER"
#define _CER_UCA2_FILE_NAME_LEN_		8
/*[260305] 固定三、四層PEM內容 */
#define _CER_FIX_PEM_FILE_NAME_		"FIX_CRT.pem"
#define _CER_FIX_PEM_FILE_NAME_LEN_	11

#define _PEM_CLIENT_CRT_FILE_NAME_		"client_crt.pem"
#define _PEM_CLIENT_PRIVATE_KEY_FILE_NAME_	"client_pri_key.pem"

#define _TLS_API_WAY_CTOS_      1
#define _TLS_API_WAY_OPENSSL_   2
#define _TLS_API_WAY_           _TLS_API_WAY_OPENSSL_           

int inTLS_Init(void);
int inTLS_OPENSSL_TLS_Init(void);
int inTLS_CTOS_TLS_Init(void);

/* context 相關 */
int inTLS_CTX_New(int inMethod, unsigned int *inCTX_ID);
int inTLS_CTX_LoadCACertificationFromFile(unsigned int uiCTX_ID,unsigned char *uszFileName);
int inTLS_CTX_LoadCertificationFromFile(unsigned int uiCTX_ID,unsigned char *uszFileName, int inFiletype);
int inTLS_CTX_LoadCertificationFromFile(unsigned int uiCTX_ID,unsigned char *uszFileName, int inFiletype);
int inTLS_CTX_LoadPrivateKeyFromFile(unsigned int uiCTX_ID,unsigned char *uszFileName, int inFiletype);
int inTLS_CTX_SetVerificationMode(unsigned int uiCTX_ID,unsigned int uiMode);
int inTLS_CTX_Free(unsigned int *uiCTX_ID);
int inTLS_OPENSSL_CTX_New(SSL_CTX **ctx);
int inTLS_OPENSSL_CTX_LoadCACertificationFromFile(SSL_CTX **ctx, char* szFileName, char* szCAPATH);
int inTLS_OPENSSL_CTX_LoadCertificationFromFile(SSL_CTX **ctx, char* szFileName, int inType);
int inTLS_OPENSSL_CTX_LoadPrivateKeyFromFile(SSL_CTX **ctx, char* szFileName, int inType);
void vdTLS_OPENSSL_CTX_SetVerificationMode(SSL_CTX **ctx, int inMode, SSL_verify_cb callback);
void vdTLS_OPENSSL_CTX_Free(SSL_CTX **ctx);
int inTLS_CTOS_CTX_New(int inMethod, unsigned int *inCTX_ID);
int inTLS_OPENSSL_CTX_SetCipherList(SSL_CTX **ctx, char *szList);
int inTLS_OPENSSL_CTX_Set_Max_Protocol_Version(SSL_CTX **ctx, int inVersion);
int inTLS_OPENSSL_CTX_Set_Min_Protocol_Version(SSL_CTX **ctx, int inVersion);
int inTLS_OPENSSL_CTX_Set_Timeout(SSL_CTX **ctx, int inTimeout);
void vdTLS_OPENSSL_SSL_INFO_CALLBACK(const SSL *ssl, int inWhere, int inRet);
void vdTLS_OPENSSL_SSL_Get_Error(char* szlocation, SSL **ssl, int *inRet);

/* 連線相關 */
int inTLS_TLS_New(unsigned int uiCTX, unsigned int *uiSSLID);
int inTLS_SetVerificationMode(unsigned int uiSSLID, unsigned int uiMode);
int inTLS_SetCipherList(unsigned int uiSSLID,unsigned char *uszList);
int inTLS_SetSocket(unsigned int uiSSL_ID, int inSocketHandle);
int inTLS_SetProtocolVersion(unsigned int uiSSL_ID, unsigned int uiProtocolVersion);
int inTLS_TLS2_Connect(unsigned int uiSSL_ID);
int inTLS_Send_Data(unsigned int uiSSL_ID, unsigned char *uszData, int *inDataLen);
int inTLS_Read_Data(unsigned int uiSSL_ID, unsigned char *uszData, int *inDataLen);
int inTLS_TLS2_Disconnect(unsigned int uiSSL_ID, unsigned int uiFlag);
int inTLS_TLS2_Free(unsigned int *uiSSL_ID);
int inTLS_TLS2_GetSession(unsigned int uiSSL_ID);
int inTLS_TLS2_GetErrorMessage(unsigned int* uiSSLID, unsigned char* uszDebugMsg);
int inTLS_OPENSSL_SSL_New(SSL_CTX **ctx, SSL **ssl);
void vdTLS_OPENSSL_SSL_SetVerificationMode(SSL **ssl, int inMode, SSL_verify_cb callback);
int inTLS_OPENSSL_SSL_SetCipherList(SSL **ssl, char *szList);
int inTLS_OPENSSL_SSL_SetSocket(SSL **ssl, int inSocketHandle);
int inTLS_OPENSSL_TLS_Disconnect(SSL **ssl);
void vdTLS_OPENSSL_TLS_Free(SSL **ssl);
int inTLS_OPENSSL_SSL_Connect(SSL **ssl);
int inTLS_OPENSSL_SSL_Send_Data(SSL **ssl, unsigned char *uszData, int *inDataLen);
int inTLS_OPENSSL_SSL_Read_Data(SSL **ssl, unsigned char *uszData, int *inDataLen);

/* 流程 */
int inTLS_Process_CTX_Flow(unsigned int *uiCTX_ID);
int inTLS_Process_TLS_Flow(unsigned int *uiCTX_ID, unsigned int *uiSSLID);
int inTLS_OPENSSL_Process_CTX_Flow(SSL_CTX **ctx);
int inTLS_OPENSSL_Process_SSL_Flow(SSL_CTX **ctx, SSL **ssl);

/* DEBUG */
void vdTLS_print_certificate_info(const char* cert_path);
void vdTLS_print_in_32byte_blocks(const char* input_str);
void vdTLS_print_time_32bytes_fixed(const ASN1_TIME *atime, const char* label);

/* Test */
int inTLS_Test(void);

