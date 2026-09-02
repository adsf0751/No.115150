
#ifdef EZ_AIRLINKEDC
	#include "_stdAfx.h"
#else
	#include "stdAfx.h"
#endif

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API

	#include "openssl\err.h"
	#include "openssl\evp.h"
	#include "openssl\ssl.h"

	HMODULE hSSL1,hSSL2;
	SSL* m_pSSL;

	//#define 	SSL_LIB

	#ifdef SSL_LIB

		#pragma comment(lib, "lib\\libeay32.lib")
		#pragma comment(lib, "lib\\ssleay32.lib")

		#define ECC_OpenSSL_add_all_algorithms		OpenSSL_add_all_algorithms
		#define ECC_ERR_load_BIO_strings			ERR_load_BIO_strings
		#define ECC_SSL_load_error_strings			SSL_load_error_strings
		#define ECC_SSL_library_init				SSL_library_init
		#define ECC_SSLv23_client_method			SSLv23_client_method
		#define ECC_SSL_CTX_new						SSL_CTX_new
		#define ECC_BIO_new_ssl_connect				BIO_new_ssl_connect
		#define ECC_BIO_ctrl						BIO_ctrl
		#define ECC_SSL_ctrl						SSL_ctrl
		#define ECC_ERR_get_error					ERR_get_error
		#define ECC_ERR_error_string				ERR_error_string
		#define ECC_BIO_write						BIO_write
		#define ECC_BIO_read						BIO_read
		#define ECC_BIO_free_all					BIO_free_all
		#define ECC_SSL_CTX_free					SSL_CTX_free
		#define ECC_SSL_CTX_use_certificate_file	SSL_CTX_use_certificate_file
		#define ECC_SSL_CTX_use_PrivateKey_file 	SSL_CTX_use_PrivateKey_file
		#define ECC_SSL_CTX_check_private_key	 	SSL_CTX_check_private_key
		/*#define ECC_SSL_set_fd;					SSL_set_fd
		#define ECC_SSL_new;						SSL_new	
		#define ECC_SSL_connect;					SSL_connect
		#define ECC_SSL_write;						SSL_write
		#define ECC_SSL_read;						SSL_read*/

	#else

		SSL_FUNC_TYPE_1 ECC_OpenSSL_add_all_algorithms,ECC_ERR_load_BIO_strings,ECC_SSL_load_error_strings;
		SSL_FUNC_TYPE_2 ECC_SSL_library_init;
		SSL_FUNC_TYPE_3 ECC_SSLv23_client_method;
		SSL_FUNC_TYPE_4 ECC_SSL_CTX_new;
		SSL_FUNC_TYPE_5 ECC_BIO_new_ssl_connect;
		SSL_FUNC_TYPE_6 ECC_BIO_ctrl;
		SSL_FUNC_TYPE_7 ECC_SSL_ctrl;
		SSL_FUNC_TYPE_8 ECC_ERR_get_error;
		SSL_FUNC_TYPE_9 ECC_ERR_error_string;
		SSL_FUNC_TYPE_10 ECC_BIO_write;
		SSL_FUNC_TYPE_11 ECC_BIO_read;
		SSL_FUNC_TYPE_12 ECC_BIO_free_all;
		SSL_FUNC_TYPE_13 ECC_SSL_CTX_free;
		SSL_FUNC_TYPE_14 ECC_SSL_CTX_use_certificate_file,ECC_SSL_CTX_use_PrivateKey_file;
		SSL_FUNC_TYPE_15 ECC_SSL_CTX_check_private_key;
		SSL_FUNC_TYPE_16 ECC_SSL_set_fd;
		SSL_FUNC_TYPE_17 ECC_SSL_new;
		SSL_FUNC_TYPE_18 ECC_SSL_connect;
		SSL_FUNC_TYPE_19 ECC_SSL_write;
		SSL_FUNC_TYPE_20 ECC_SSL_read;
		SSL_FUNC_TYPE_21 ECC_SSL_CTX_set_timeout;
		SSL_FUNC_TYPE_22 ECC_SSL_get_error;

	#endif

	BIO* m_pSockBIO;
	SSL_CTX* m_pSSLCtx;

#elif READER_MANUFACTURERS==LINUX_API//this is mean define LINUX_API

	#include <openssl/bio.h>
	#include <openssl/err.h>
	#include <openssl/evp.h>
	#include <openssl/ssl.h>

	#define ECC_OpenSSL_add_all_algorithms		OpenSSL_add_all_algorithms
	#define ECC_ERR_load_BIO_strings			ERR_load_BIO_strings
	#define ECC_SSL_load_error_strings			SSL_load_error_strings
	#define ECC_SSL_library_init				SSL_library_init
	#define ECC_SSLv23_client_method			SSLv23_client_method
	#define ECC_SSL_CTX_new						SSL_CTX_new
	#define ECC_BIO_new_ssl_connect				BIO_new_ssl_connect
	#define ECC_BIO_ctrl						BIO_ctrl
	#define ECC_SSL_ctrl						SSL_ctrl
	#define ECC_ERR_get_error					ERR_get_error
	#define ECC_ERR_error_string				ERR_error_string
	#define ECC_BIO_write						BIO_write
	#define ECC_BIO_read						BIO_read
	#define ECC_BIO_free_all					BIO_free_all
	#define ECC_SSL_CTX_free					SSL_CTX_free
	#define ECC_SSL_CTX_use_certificate_file	SSL_CTX_use_certificate_file
	#define ECC_SSL_CTX_use_PrivateKey_file		SSL_CTX_use_PrivateKey_file
	#define ECC_SSL_CTX_check_private_key	 	SSL_CTX_check_private_key
	#define ECC_SSL_set_fd						SSL_set_fd

	SSL_CTX* m_pSSLCtx;

	SSL* ssl;
	int sockfd = 0;

#elif READER_MANUFACTURERS==ANDROID_API

	#include<icerapi_icerapi_Api.h>
	#include <android/log.h>
	extern JNIEnv *Jenv;
	extern jobject Jni_usbManager,Jni_context;

	jobject obj;
	jmethodID SSLInit,AndroidSSLSend,AndroidSSLRead,AndroidSSLClose;

#else

#endif

int socketClient;

int inSSLInitial(void)
{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
int i;
DWORD		  err      ;//錯誤函數回傳值

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		WCHAR szBuf[50];
	#endif

	#ifndef SSL_LIB

		char chAllFileName[200];

		memset(chAllFileName,0x00,sizeof(chAllFileName));
		sprintf(chAllFileName,"%s\\libeay32.dll",gCurrentFolder);
		i = inFileGetSize("libeay32.dll",FALSE);
	#if READER_MANUFACTURERS==WINDOWS_CE_API
		memset(szBuf,0x00,sizeof(szBuf));
		MultiByteToWideChar(CP_UTF8, 0, (char *)chAllFileName, 100, szBuf, 100);
		log_msg(LOG_LEVEL_FLOW,"inSSLInitial 1(%d) %S\n",i,szBuf);
		hSSL1 = LoadLibrary(szBuf);
	#else
		hSSL1 = LoadLibraryEx(chAllFileName, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	#endif
		//hSSL1 = LoadLibrary(chAllFileName);

		if(hSSL1 == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial SSL DLL FAIL 1 (%s)",chAllFileName);
			return ECC_SSL_LOAD_FAIL;
		}

		memset(chAllFileName,0x00,sizeof(chAllFileName));
		sprintf(chAllFileName,"%s\\ssleay32.dll",gCurrentFolder);
	#if READER_MANUFACTURERS==WINDOWS_CE_API
		memset(szBuf,0x00,sizeof(szBuf));
		MultiByteToWideChar(CP_UTF8, 0, (char *)chAllFileName, 100, szBuf, 100);
		log_msg(LOG_LEVEL_FLOW,"inSSLInitial 2 %S\n",szBuf);
		hSSL2 = LoadLibrary(szBuf);
	#else
		hSSL2 = LoadLibraryEx(chAllFileName, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	#endif
		//hSSL2 = LoadLibrary(chAllFileName);

		if(hSSL2 == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial SSL DLL FAIL 2 (%s)",chAllFileName);
			return ECC_SSL_LOAD_FAIL;
		}

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		ECC_OpenSSL_add_all_algorithms = (SSL_FUNC_TYPE_1)GetProcAddress(hSSL1,L"OPENSSL_add_all_algorithms_noconf");
	#else
		ECC_OpenSSL_add_all_algorithms = (SSL_FUNC_TYPE_1)GetProcAddress(hSSL1,"OPENSSL_add_all_algorithms_noconf");
	#endif
		if(ECC_OpenSSL_add_all_algorithms == NULL)
		{
			err = GetLastError();
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial Fail 3(%d)(%d)!!",hSSL1,err);
			return ECC_SSL_LOAD_FAIL;
		}

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		ECC_ERR_load_BIO_strings = (SSL_FUNC_TYPE_1)GetProcAddress(hSSL1,L"ERR_load_BIO_strings");
	#else
		ECC_ERR_load_BIO_strings = (SSL_FUNC_TYPE_1)GetProcAddress(hSSL1,"ERR_load_BIO_strings");
	#endif
		//ECC_ERR_load_BIO_strings = (SSL_FUNC_TYPE_1)GetProcAddress(hSSL1,"ERR_load_BIO_strings");
		if(ECC_ERR_load_BIO_strings == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial Fail 4!!");
			return ECC_SSL_LOAD_FAIL;
		}

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		ECC_SSL_load_error_strings = (SSL_FUNC_TYPE_1)GetProcAddress(hSSL1,L"ERR_load_BIO_strings");
	#else
		ECC_SSL_load_error_strings = (SSL_FUNC_TYPE_1)GetProcAddress(hSSL1,"ERR_load_BIO_strings");
	#endif
		//ECC_SSL_load_error_strings = (SSL_FUNC_TYPE_1)GetProcAddress(hSSL1,"ERR_load_BIO_strings");
		if(ECC_SSL_load_error_strings == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial Fail 5!!");
			return ECC_SSL_LOAD_FAIL;
		}

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		ECC_SSL_library_init = (SSL_FUNC_TYPE_2)GetProcAddress(hSSL2,L"SSL_library_init");
	#else
		ECC_SSL_library_init = (SSL_FUNC_TYPE_2)GetProcAddress(hSSL2,"SSL_library_init");
	#endif
		//ECC_SSL_library_init = (SSL_FUNC_TYPE_2)GetProcAddress(hSSL2,"SSL_library_init");
		if(ECC_SSL_library_init == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial Fail 6!!");
			return ECC_SSL_LOAD_FAIL;
		}

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		ECC_SSLv23_client_method = (SSL_FUNC_TYPE_3)GetProcAddress(hSSL2,L"SSLv23_client_method");
	#else
		ECC_SSLv23_client_method = (SSL_FUNC_TYPE_3)GetProcAddress(hSSL2,"SSLv23_client_method");
	#endif
		//ECC_SSLv23_client_method = (SSL_FUNC_TYPE_3)GetProcAddress(hSSL2,"SSLv23_client_method");
		if(ECC_SSLv23_client_method == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial Fail 7!!");
			return ECC_SSL_LOAD_FAIL;
		}

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		ECC_SSL_CTX_new = (SSL_FUNC_TYPE_4)GetProcAddress(hSSL2,L"SSL_CTX_new");
	#else
		ECC_SSL_CTX_new = (SSL_FUNC_TYPE_4)GetProcAddress(hSSL2,"SSL_CTX_new");
	#endif
		//ECC_SSL_CTX_new = (SSL_FUNC_TYPE_4)GetProcAddress(hSSL2,"SSL_CTX_new");
		if(ECC_SSL_CTX_new == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial Fail 8!!");
			return ECC_SSL_LOAD_FAIL;
		}

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		ECC_BIO_new_ssl_connect = (SSL_FUNC_TYPE_5)GetProcAddress(hSSL2,L"BIO_new_ssl_connect");
	#else
		ECC_BIO_new_ssl_connect = (SSL_FUNC_TYPE_5)GetProcAddress(hSSL2,"BIO_new_ssl_connect");
	#endif
		//ECC_BIO_new_ssl_connect = (SSL_FUNC_TYPE_5)GetProcAddress(hSSL2,"BIO_new_ssl_connect");
		if(ECC_BIO_new_ssl_connect == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial Fail 9!!");
			return ECC_SSL_LOAD_FAIL;
		}

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		ECC_BIO_ctrl = (SSL_FUNC_TYPE_6)GetProcAddress(hSSL1,L"BIO_ctrl");
	#else
		ECC_BIO_ctrl = (SSL_FUNC_TYPE_6)GetProcAddress(hSSL1,"BIO_ctrl");
	#endif
		//ECC_BIO_ctrl = (SSL_FUNC_TYPE_6)GetProcAddress(hSSL1,"BIO_ctrl");
		if(ECC_BIO_ctrl == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial Fail 10!!");
			return ECC_SSL_LOAD_FAIL;
		}

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		ECC_SSL_ctrl = (SSL_FUNC_TYPE_7)GetProcAddress(hSSL2,L"SSL_ctrl");
	#else
		ECC_SSL_ctrl = (SSL_FUNC_TYPE_7)GetProcAddress(hSSL2,"SSL_ctrl");
	#endif
		//ECC_SSL_ctrl = (SSL_FUNC_TYPE_7)GetProcAddress(hSSL2,"SSL_ctrl");
		if(ECC_SSL_ctrl == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial Fail 11!!");
			return ECC_SSL_LOAD_FAIL;
		}

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		ECC_ERR_get_error = (SSL_FUNC_TYPE_8)GetProcAddress(hSSL1,L"ERR_get_error");
	#else
		ECC_ERR_get_error = (SSL_FUNC_TYPE_8)GetProcAddress(hSSL1,"ERR_get_error");
	#endif
		//ECC_ERR_get_error = (SSL_FUNC_TYPE_8)GetProcAddress(hSSL1,"ERR_get_error");
		if(ECC_ERR_get_error == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial Fail 12!!");
			return ECC_SSL_LOAD_FAIL;
		}

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		ECC_ERR_error_string = (SSL_FUNC_TYPE_9)GetProcAddress(hSSL1,L"ERR_error_string");
	#else
		ECC_ERR_error_string = (SSL_FUNC_TYPE_9)GetProcAddress(hSSL1,"ERR_error_string");
	#endif
		//ECC_ERR_error_string = (SSL_FUNC_TYPE_9)GetProcAddress(hSSL1,"ERR_error_string");
		if(ECC_ERR_error_string == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial Fail 13!!");
			return ECC_SSL_LOAD_FAIL;
		}

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		ECC_BIO_write = (SSL_FUNC_TYPE_10)GetProcAddress(hSSL1,L"BIO_write");
	#else
		ECC_BIO_write = (SSL_FUNC_TYPE_10)GetProcAddress(hSSL1,"BIO_write");
	#endif
		//ECC_BIO_write = (SSL_FUNC_TYPE_10)GetProcAddress(hSSL1,"BIO_write");
		if(ECC_BIO_write == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial Fail 14!!");
			return ECC_SSL_LOAD_FAIL;
		}

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		ECC_BIO_read = (SSL_FUNC_TYPE_11)GetProcAddress(hSSL1,L"BIO_read");
	#else
		ECC_BIO_read = (SSL_FUNC_TYPE_11)GetProcAddress(hSSL1,"BIO_read");
	#endif
		//ECC_BIO_read = (SSL_FUNC_TYPE_11)GetProcAddress(hSSL1,"BIO_read");
		if(ECC_BIO_read == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial Fail 15!!");
			return ECC_SSL_LOAD_FAIL;
		}

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		ECC_BIO_free_all = (SSL_FUNC_TYPE_12)GetProcAddress(hSSL1,L"BIO_free_all");
	#else
		ECC_BIO_free_all = (SSL_FUNC_TYPE_12)GetProcAddress(hSSL1,"BIO_free_all");
	#endif
		//ECC_BIO_free_all = (SSL_FUNC_TYPE_12)GetProcAddress(hSSL1,"BIO_free_all");
		if(ECC_BIO_free_all == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial Fail 16!!");
			return ECC_SSL_LOAD_FAIL;
		}

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		ECC_SSL_CTX_free = (SSL_FUNC_TYPE_13)GetProcAddress(hSSL2,L"SSL_CTX_free");
	#else
		ECC_SSL_CTX_free = (SSL_FUNC_TYPE_13)GetProcAddress(hSSL2,"SSL_CTX_free");
	#endif
		//ECC_SSL_CTX_free = (SSL_FUNC_TYPE_13)GetProcAddress(hSSL2,"SSL_CTX_free");
		if(ECC_SSL_CTX_free == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial Fail 17!!");
			return ECC_SSL_LOAD_FAIL;
		}

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		ECC_SSL_CTX_use_certificate_file = (SSL_FUNC_TYPE_14)GetProcAddress(hSSL2,L"SSL_CTX_use_certificate_file");
	#else
		ECC_SSL_CTX_use_certificate_file = (SSL_FUNC_TYPE_14)GetProcAddress(hSSL2,"SSL_CTX_use_certificate_file");
	#endif
		//ECC_SSL_CTX_use_certificate_file = (SSL_FUNC_TYPE_14)GetProcAddress(hSSL2,"SSL_CTX_use_certificate_file");
		if(ECC_SSL_CTX_use_certificate_file == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial Fail 18!!");
			return ECC_SSL_LOAD_FAIL;
		}

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		ECC_SSL_CTX_use_PrivateKey_file = (SSL_FUNC_TYPE_14)GetProcAddress(hSSL2,L"SSL_CTX_use_PrivateKey_file");
	#else
		ECC_SSL_CTX_use_PrivateKey_file = (SSL_FUNC_TYPE_14)GetProcAddress(hSSL2,"SSL_CTX_use_PrivateKey_file");
	#endif
		//ECC_SSL_CTX_use_PrivateKey_file = (SSL_FUNC_TYPE_14)GetProcAddress(hSSL2,"SSL_CTX_use_PrivateKey_file");
		if(ECC_SSL_CTX_use_PrivateKey_file == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial Fail 19!!");
			return ECC_SSL_LOAD_FAIL;
		}

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		ECC_SSL_CTX_check_private_key = (SSL_FUNC_TYPE_15)GetProcAddress(hSSL2,L"SSL_CTX_check_private_key");
	#else
		ECC_SSL_CTX_check_private_key = (SSL_FUNC_TYPE_15)GetProcAddress(hSSL2,"SSL_CTX_check_private_key");
	#endif
		//ECC_SSL_CTX_check_private_key = (SSL_FUNC_TYPE_15)GetProcAddress(hSSL2,"SSL_CTX_check_private_key");
		if(ECC_SSL_CTX_check_private_key == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial Fail 20!!");
			return ECC_SSL_LOAD_FAIL;
		}

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		ECC_SSL_set_fd = (SSL_FUNC_TYPE_16)GetProcAddress(hSSL2,L"SSL_set_fd");
	#else
		ECC_SSL_set_fd = (SSL_FUNC_TYPE_16)GetProcAddress(hSSL2,"SSL_set_fd");
	#endif

		if(ECC_SSL_set_fd == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial Fail 21!!");
			return ECC_SSL_LOAD_FAIL;
		}

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		ECC_SSL_new = (SSL_FUNC_TYPE_17)GetProcAddress(hSSL2,L"SSL_new");
	#else
		ECC_SSL_new = (SSL_FUNC_TYPE_17)GetProcAddress(hSSL2,"SSL_new");
	#endif
		//ECC_SSL_CTX_check_private_key = (SSL_FUNC_TYPE_15)GetProcAddress(hSSL2,"SSL_CTX_check_private_key");
		if(ECC_SSL_new == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial Fail 22!!");
			return ECC_SSL_LOAD_FAIL;
		}

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		ECC_SSL_connect = (SSL_FUNC_TYPE_18)GetProcAddress(hSSL2,L"SSL_connect");
	#else
		ECC_SSL_connect = (SSL_FUNC_TYPE_18)GetProcAddress(hSSL2,"SSL_connect");
	#endif
		//ECC_SSL_CTX_check_private_key = (SSL_FUNC_TYPE_15)GetProcAddress(hSSL2,"SSL_CTX_check_private_key");
		if(ECC_SSL_connect == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial Fail 23!!");
			return ECC_SSL_LOAD_FAIL;
		}

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		ECC_SSL_write = (SSL_FUNC_TYPE_19)GetProcAddress(hSSL2,L"SSL_write");
	#else
		ECC_SSL_write = (SSL_FUNC_TYPE_19)GetProcAddress(hSSL2,"SSL_write");
	#endif
		//ECC_SSL_CTX_check_private_key = (SSL_FUNC_TYPE_15)GetProcAddress(hSSL2,"SSL_CTX_check_private_key");
		if(ECC_SSL_write == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial Fail 24!!");
			return ECC_SSL_LOAD_FAIL;
		}

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		ECC_SSL_read = (SSL_FUNC_TYPE_20)GetProcAddress(hSSL2,L"SSL_read");
	#else
		ECC_SSL_read = (SSL_FUNC_TYPE_20)GetProcAddress(hSSL2,"SSL_read");
	#endif
		//ECC_SSL_CTX_check_private_key = (SSL_FUNC_TYPE_15)GetProcAddress(hSSL2,"SSL_CTX_check_private_key");
		if(ECC_SSL_read == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial Fail 24!!");
			return ECC_SSL_LOAD_FAIL;
		}

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		ECC_SSL_CTX_set_timeout= (SSL_FUNC_TYPE_21)GetProcAddress(hSSL2,L"SSL_CTX_set_timeout");
	#else
		ECC_SSL_CTX_set_timeout = (SSL_FUNC_TYPE_21)GetProcAddress(hSSL2,"SSL_CTX_set_timeout");

	#endif
		//ECC_SSL_CTX_check_private_key = (SSL_FUNC_TYPE_15)GetProcAddress(hSSL2,"SSL_CTX_check_private_key");
		if(ECC_SSL_CTX_set_timeout == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial Fail 25!!");
			return ECC_SSL_LOAD_FAIL;
		}

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		ECC_SSL_get_error= (SSL_FUNC_TYPE_22)GetProcAddress(hSSL2,L"SSL_get_error");
	#else
		ECC_SSL_get_error = (SSL_FUNC_TYPE_22)GetProcAddress(hSSL2,"SSL_get_error");
	#endif
		//ECC_SSL_CTX_check_private_key = (SSL_FUNC_TYPE_15)GetProcAddress(hSSL2,"SSL_CTX_check_private_key");
		if(ECC_SSL_get_error == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inSSLInitial Fail 26!!");
			return ECC_SSL_LOAD_FAIL;
		}
	#endif

#elif READER_MANUFACTURERS==LINUX_API//this is mean define LINUX_API

#elif READER_MANUFACTURERS==ANDROID_API

	jclass clazz;
//jmethodID SSLInit,AndroidSSLSend,AndroidSSLRead,AndroidSSLClose;

	//log_msg(LOG_LEVEL_ERROR,"inSSLInitial Start!!");
	clazz = (*Jenv) -> FindClass(Jenv,"icerapi/icerapi/SSLClient");
	//no ca
	//AndroidSSLSend = (*Jenv) -> GetMethodID(Jenv,clazz,"SSLSend","([BLjava/lang/String;Ljava/lang/String;)I");
	//ca
	AndroidSSLSend = (*Jenv) -> GetMethodID(Jenv,clazz,"SSLSend","([BLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;II)I");
	AndroidSSLRead = (*Jenv) -> GetMethodID(Jenv,clazz,"SSLReceive","(I)[B");
	AndroidSSLClose = (*Jenv) -> GetMethodID(Jenv,clazz,"SSLClose","()V");
	SSLInit = (*Jenv) -> GetMethodID(Jenv,clazz,"<init>","()V");
	obj = (*Jenv) -> NewObject(Jenv,clazz,SSLInit);

#else

#endif

	return SUCCESS;
}

void inSSLFinalizer()
{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API

	vdSSLFreeLibrary();

#elif READER_MANUFACTURERS==LINUX_API//this is mean define LINUX_API

#elif READER_MANUFACTURERS==ANDROID_API

#else

#endif
}

void vdSSLFreeLibrary()
{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API

	FreeLibrary(hSSL1);
	FreeLibrary(hSSL2);

#endif
}

//#if READER_MANUFACTURERS != ANDROID_API
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API || READER_MANUFACTURERS==LINUX_API
int ssl_ecc_connect(int sockfd, SSL* ssl) 
{
	fd_set rw_fds;
	int result = 0, retval;
	
	//printf("calling SSL_connect\n");

	while (result != 1)
	{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
		result = ECC_SSL_connect(ssl);
#else   // LINUX_API
	    result = SSL_connect(ssl);
#endif			
		if (result != 1)
		{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API		
			if ((ECC_SSL_get_error(ssl, result) != SSL_ERROR_WANT_READ) && (ECC_SSL_get_error(ssl, result) != SSL_ERROR_WANT_WRITE))
#else   // LINUX_API
			if ((SSL_get_error(ssl, result) != SSL_ERROR_WANT_READ) && (SSL_get_error(ssl, result) != SSL_ERROR_WANT_WRITE))
#endif
				return -1; // connect error
		}
		
		FD_ZERO(&rw_fds);
		FD_SET(sockfd, &rw_fds);
    
		//struct timeval timeout = {5, 0 };
		struct timeval timeout = {srTxnData.srParameter.gTCPIPTimeOut, 0 };
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
		if (ECC_SSL_get_error(ssl, result) == SSL_ERROR_WANT_READ)
#else
		if (SSL_get_error(ssl, result) == SSL_ERROR_WANT_READ)
#endif
		{    	
			retval = select(sockfd + 1, &rw_fds, NULL, NULL, &timeout);
 		
 			if (retval == -1)
 				return -1; // error
 			else if (retval == 0)
 				return -2; // timeout
		}
 		else
 		{
 			retval = select(sockfd + 1, NULL, &rw_fds, NULL, &timeout);
 		
 			if (retval == -1)
 				return -1; // error
 			else if (retval == 0)
 				return -2; // timeout
 		}
	}

  return 0;
}
#endif

//#if READER_MANUFACTURERS != ANDROID_API
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API || READER_MANUFACTURERS==LINUX_API 
int ssl_ecc_read(int sockfd, SSL* ssl, unsigned char* recv_data, int max_len)
{
	fd_set rw_fds;
	int real_len = 0, retval;
	
  	//printf("calling SSL_read\n");

	while (1)
	{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
		real_len = ECC_SSL_read(ssl, recv_data, max_len);
#else
		real_len = SSL_read(ssl, recv_data, max_len);
#endif
		
		if (real_len == 0)
		{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API		
			if ((ECC_SSL_get_error(ssl, real_len) != SSL_ERROR_WANT_READ) && (ECC_SSL_get_error(ssl, real_len) != SSL_ERROR_WANT_WRITE))
#else
			if ((SSL_get_error(ssl, real_len) != SSL_ERROR_WANT_READ) && (SSL_get_error(ssl, real_len) != SSL_ERROR_WANT_WRITE))
#endif
				return -1; // read error
		}
		else if (real_len < 0)
		{
		
			FD_ZERO(&rw_fds);
	    		FD_SET(sockfd, &rw_fds);
	    
	   		//struct timeval timeout = { 3, 0 };
	   		struct timeval timeout = {srTxnData.srParameter.gTCPIPTimeOut, 0 };
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
	    		if (ECC_SSL_get_error(ssl, real_len) == SSL_ERROR_WANT_READ)
#else
			if (SSL_get_error(ssl, real_len) == SSL_ERROR_WANT_READ)
#endif
	    		{    	
				retval = select(sockfd + 1, &rw_fds, NULL, NULL, &timeout);
	 		
				if (retval == -1)
	 				return -1; // error
		 		else if (retval == 0)
	 				return -2; // timeout
	    		}
			else
	 		{
	 			retval = select(sockfd + 1, NULL, &rw_fds, NULL, &timeout);

				if (retval == -1)
		 			return -1; // error
	 			else if (retval == 0)
	 				return -2; // timeout
	 		}   
	 	}
	 	else
	 		return real_len; // success
	}
}
#endif

int inSSLSendData(unsigned char *IP,unsigned short Port,BYTE *bSendData,int inSendLen)
{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API// || READER_MANUFACTURERS==LINUX_API

char szURL[200] = {'\0'};
char m_szError[256];

char szData[1024*3] = {'\0'};
int nPos;// = 0;
int nCmdLen;// = (int)strlen(pCmd);
int nLen;
int inPort = (int)Port;
unsigned long ulIP = 0L;
//int 	verify_client = FALSE;
//SSL_CTX* ctx=NULL;
const char bPath[100] = {"D:\\Office_Work\\ICERAPI\\Windows\\ICERAPI_10017AB\\ICERAPI\\ck1.cer"},bCA[20] = {"ck1.cer"};
SSL_METHOD *SSLRet;

	int inRetVal;
	WSADATA wsaData;
	struct sockaddr_in addr;
	struct hostent *host;


	//初始化
	inRetVal = WSAStartup(MAKEWORD(2,2),&wsaData);
	if(inRetVal != NO_ERROR)
	{
		log_msg(LOG_LEVEL_ERROR,"WSAStartup failed with error: %d",inRetVal);
		return ICER_ERROR;
		//return TCP_SOCKET_FAIL;
	}

	if ((host=gethostbyname((char *)IP)) == NULL)
	{
		log_msg(LOG_LEVEL_ERROR,"gethostbyname(%s) fail.", IP);
		return ICER_ERROR;
	}

	socketClient = socket(AF_INET,SOCK_STREAM,0);

	if (socketClient < 0)
	{
		log_msg(LOG_LEVEL_ERROR,"inSSLSendData ERROR 01");
		return ICER_ERROR;
    }
	
	memset(&addr, 0, sizeof(addr));
	memcpy(&addr.sin_addr, host->h_addr,host->h_length);
	addr.sin_family =host->h_addrtype;
   	addr.sin_port = htons(Port);   	
	
   	if (connect(socketClient, (struct sockaddr *)&addr, sizeof(addr)) == -1) 
	{
		close(socketClient);
		log_msg(LOG_LEVEL_ERROR,"inSSLSendData ERROR 02");
		return ICER_ERROR;
   	}

	// Set the socket to be non blocking.
  	unsigned long flags =1;
	if (ioctlsocket(socketClient, FIONBIO, &flags) != 0) 
  	{
   		log_msg(LOG_LEVEL_ERROR,"inSSLSendData ERROR 03");
    	close(socketClient);
		return ICER_ERROR;
  	}


	ECC_SSL_library_init();
	ECC_OpenSSL_add_all_algorithms();
	ECC_SSL_load_error_strings();


	SSLRet = (SSL_METHOD *)ECC_SSLv23_client_method();
	
	m_pSSLCtx = ECC_SSL_CTX_new(SSLRet);
	
	if (m_pSSLCtx== NULL)
    {
		log_msg(LOG_LEVEL_ERROR,"inSSLSendData ERROR 04");
		abort();
		return ICER_ERROR;
    }

	if(strlen((char *)srTxnData.srParameter.bCRT_FileName) > 0)
   	{
   		char chAllFileName[2000];

		memset(chAllFileName,0x00,sizeof(chAllFileName));
		sprintf(chAllFileName,"%s\\%s",gCurrentFolder,srTxnData.srParameter.bCRT_FileName);
    	/* Load the client certificate into the SSL_CTX structure */
        //if ( SSL_CTX_use_certificate_file(m_pSSLCtx, "ck1.crt",  SSL_FILETYPE_PEM) <= 0 )//0906D06C:PEM routines:PEM_read_bio:no start line.
        //if ( SSL_CTX_use_certificate_file(m_pSSLCtx, "ck1.cer",  SSL_FILETYPE_ASN1) <= 0 )//error:0906D06C:PEM routines:PEM_read_bio:no start line.

		//if ( SSL_CTX_use_certificate_file(m_pSSLCtx, "ck1.crt",  SSL_FILETYPE_ASN1) <= 0 )//error:0D0680A8:asn1 encoding routines:ASN1_CHECK_TLEN:wrong tag.
		if ( ECC_SSL_CTX_use_certificate_file(m_pSSLCtx, chAllFileName,  SSL_FILETYPE_PEM) <= 0 )//error:0906D06C:PEM routines:PEM_read_bio:no start line.
       	{
	   		ECC_ERR_error_string(ECC_ERR_get_error(), (char* )m_szError);
			log_msg(LOG_LEVEL_ERROR,"SSL_CTX_use_certificate_file ERROR: %s.", m_szError);
			vdSSLCloseSocket();
			return ICER_ERROR;
        }
	}

	if(strlen((char *)srTxnData.srParameter.bKey_FileName) > 0)
	{
    	char chAllFileName[2000];

		memset(chAllFileName,0x00,sizeof(chAllFileName));
		sprintf(chAllFileName,"%s\\%s",gCurrentFolder,srTxnData.srParameter.bKey_FileName);
        /* Load the private-key corresponding to the client certificate */
        if ( ECC_SSL_CTX_use_PrivateKey_file( m_pSSLCtx, chAllFileName, SSL_FILETYPE_PEM ) <= 0 )
        {
            ECC_ERR_error_string(ECC_ERR_get_error(), (char* )m_szError);
			log_msg(LOG_LEVEL_ERROR,"SSL_CTX_use_PrivateKey_file ERROR: %s.", m_szError);
			vdSSLCloseSocket();
			return ICER_ERROR;
        }

        /* Check if the client certificate and private-key matches */
        if ( !ECC_SSL_CTX_check_private_key(m_pSSLCtx))
        {
            ECC_ERR_error_string(ECC_ERR_get_error(), (char* )m_szError);
			log_msg(LOG_LEVEL_ERROR,"SSL_CTX_check_private_key ERROR: %s.", m_szError);
			vdSSLCloseSocket();
			return ICER_ERROR;
        }
    }

	m_pSSL = ECC_SSL_new(m_pSSLCtx);

	if (!m_pSSL)
	{
		log_msg(LOG_LEVEL_ERROR,"inSSLSendData ERROR 05");
		return ICER_ERROR;
	}

	ECC_SSL_set_fd(m_pSSL, socketClient);
	
	if (ssl_ecc_connect(socketClient, m_pSSL) < 0)
	{
		ECC_ERR_error_string(ECC_ERR_get_error(), (char* )m_szError);
		log_msg(LOG_LEVEL_ERROR,"[CSSLConnect::Connect] SSL_CTX_use_certificate ERROR: %s.", m_szError);
		vdSSLCloseSocket();
		return ICER_ERROR;
	}
	//log_msg(LOG_LEVEL_ERROR,"BIO_ctrl OK");

	/* Check the certificate
	if(SSL_get_verify_result(m_pSSL) != X509_V_OK)
	{
		fprintf(stderr, "Certificate verification error: %i\n", SSL_get_verify_result(ssl));
		BIO_free_all(m_pSockBIO);
		SSL_CTX_free(ctx);
		return 0;
	}*/

	nPos = 0;
	//nCmdLen = 76;
	nCmdLen = inSendLen;
	//nCmdLen = (int)strlen(pCmd);
	//printf("ready to send[%s]....\n",pCmd);
	//send
	while(1)
	{
		if(nPos == nCmdLen || nCmdLen <= 0)
		{
			break;
		}

		//nLen = ECC_BIO_write(m_pSockBIO, "<TransXML><TRANS><T0100>0200</T0100><T0300>881999</T0300></TRANS></TransXML>", 76);
		//nLen = ECC_BIO_write(m_pSockBIO, &bSendData[nPos], nCmdLen);
		//nLen = SSL_write(m_pSockBIO, &bSendData[nPos], nCmdLen);

		nLen = ECC_SSL_write(m_pSSL, &bSendData[nPos], nCmdLen);
		if(nLen <= 0)
		{
			//printf("write break\n");
			break;
		}
		else
		{
			nPos += nLen;
			nCmdLen -= nLen;
		}
		//log_msg(LOG_LEVEL_ERROR,"BIO_write(%d)(%d)(%d) OK",nCmdLen,nPos,nLen);
	}

	if(nPos != nCmdLen && nCmdLen != 0)
	{
		log_msg(LOG_LEVEL_ERROR,"inSSLSendData failed nPos = %d,nCmdLen = %d",nPos,nCmdLen);
		//BIO_free_all(m_pSockBIO);
		//SSL_CTX_free(m_pSSLCtx);
		vdSSLCloseSocket();
		return ICER_ERROR;
	}
	//log_msg(LOG_LEVEL_ERROR,"inSSLSendData OK");

#elif READER_MANUFACTURERS==LINUX_API//this is mean define LINUX_API

//	int nLen;
	struct sockaddr_in addr;
	struct hostent *host;
	int num;
	char m_szError[256];

	log_msg(LOG_LEVEL_FLOW,"inSSLSendData 1 Start(%s:%d)%d",IP,Port,inSendLen);


	if ((host=gethostbyname((char *)IP)) == NULL)
	{
		log_msg(LOG_LEVEL_ERROR,"gethostbyname(%s) fail.", IP);
		return ICER_ERROR;
	}
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    	if (sockfd < 0)
    	{
      		log_msg(LOG_LEVEL_ERROR,"could not create socket\n");
	      return ICER_ERROR;
    	}

	memset(&addr, 0, sizeof(addr));
	memcpy(&addr.sin_addr, host->h_addr,host->h_length);
	addr.sin_family =host->h_addrtype;
	addr.sin_port = htons(Port);


	if (connect(sockfd, (struct sockaddr*)(&addr), sizeof(addr)) && errno != EINPROGRESS)
	{
		log_msg(LOG_LEVEL_ERROR,"could not connect\n");
		return ICER_ERROR;
	}

	// Set the socket to be non blocking.
  	int flags = fcntl(sockfd, F_GETFL, 0);
	if (fcntl(sockfd, F_SETFL, flags |= O_NONBLOCK) != 0) 
  	{
    		log_msg(LOG_LEVEL_ERROR,"inSSLSendData ERROR 03");
	    	vdSSLCloseSocket();
		return ICER_ERROR;
  	}

	SSL_library_init();
	SSL_load_error_strings();

	m_pSSLCtx = SSL_CTX_new(SSLv23_client_method());
	if (!m_pSSLCtx)
	{
		log_msg(LOG_LEVEL_ERROR,"could not SSL_CTX_new\n");
		return ICER_ERROR;
	}

	
	if(strlen((char *)srTxnData.srParameter.bCRT_FileName) > 0)
    	{
    		char chAllFileName[2000];

		memset(chAllFileName,0x00,sizeof(chAllFileName));
		sprintf(chAllFileName,"%s/%s",gCurrentFolder,srTxnData.srParameter.bCRT_FileName);

		// * Load the client certificate into the SSL_CTX structure * /
		if ( SSL_CTX_use_certificate_file(m_pSSLCtx, chAllFileName,  SSL_FILETYPE_PEM) <= 0 )//SSL_FILETYPE_PEM SSL_FILETYPE_ASN1
		{
			ECC_ERR_error_string(ECC_ERR_get_error(), (char* )m_szError);
			log_msg(LOG_LEVEL_ERROR,"SSL_CTX_use_certificate_file ERROR(%s): %s.",chAllFileName, m_szError);
			vdSSLCloseSocket();
			return ICER_ERROR;
		}
	}

	if(strlen((char *)srTxnData.srParameter.bKey_FileName) > 0)
	{
		char chAllFileName[2000];

		memset(chAllFileName,0x00,sizeof(chAllFileName));
		sprintf(chAllFileName,"%s/%s",gCurrentFolder,srTxnData.srParameter.bKey_FileName);

 		if ( SSL_CTX_use_PrivateKey_file( m_pSSLCtx, chAllFileName, SSL_FILETYPE_PEM ) <= 0 )//SSL_FILETYPE_PEM SSL_FILETYPE_ASN1
		{
			ECC_ERR_error_string(ECC_ERR_get_error(), (char* )m_szError);
			log_msg(LOG_LEVEL_ERROR,"SSL_CTX_use_PrivateKey_file ERROR(%s): %s.",chAllFileName, m_szError);
			vdSSLCloseSocket();
			return ICER_ERROR;
		}

		if ( !SSL_CTX_check_private_key(m_pSSLCtx))
		{
			ECC_ERR_error_string(ECC_ERR_get_error(), (char* )m_szError);
			log_msg(LOG_LEVEL_ERROR,"SSL_CTX_check_private_key ERROR: %s.", m_szError);
			vdSSLCloseSocket();
			return ICER_ERROR;
		}
	}

	ssl = SSL_new(m_pSSLCtx);
	if (!ssl)
	{
		log_msg(LOG_LEVEL_ERROR,"could not SSL_new\n");
		return ICER_ERROR;
	}
	int ret=0;
	ret = SSL_set_fd(ssl, sockfd);
	if (ret != TRUE)
	{
	    close(sockfd);
	    log_msg(LOG_LEVEL_ERROR,"could not SSL_set_fd : %d\n", ret);
	    return ICER_ERROR;
	}

	//num = SSL_connect(ssl);
	num = ssl_ecc_connect(sockfd, ssl);
	//if(num <= 0)
	if(num < 0)
	{
		ret = SSL_get_error(ssl, num);
	    close(sockfd);
	    log_msg(LOG_LEVEL_ERROR,"could not SSL_connect : %d\n", num);
	    return ICER_ERROR;
	}

	num = SSL_write(ssl, bSendData, inSendLen);
	if(num <= 0)
	{
		ret = SSL_get_error(ssl, num);
	    close(sockfd);
	    log_msg(LOG_LEVEL_ERROR,"could not SSL_write : %d\n", ret);
	    return ICER_ERROR;
	}

#elif READER_MANUFACTURERS==ANDROID_API
jint i;
const char *jBuf;
char /*bIP[20],*/bPort[5];
jstring jstMSG1,jstMSG2,jstMSG3,jstMSG4,jstMSG5,jstMSG6;
jint resultI;
//path fileName1 filleName2 caFlag
extern BYTE ANDROID_FILE_PATH[100];

//jbyte *jy;

	//memset(bIP,0x00,sizeof(bIP));
	memset(bPort,0x00,sizeof(bPort));
	//memcpy(bIP,IP,strlen(IP));
	sprintf(bPort,"%d",Port);
	//ip port caPath file1 file2 caFlag
	jstMSG2 = (*Jenv)->NewStringUTF(Jenv,IP);
	jstMSG3 = (*Jenv)->NewStringUTF(Jenv,bPort);
	jstMSG4 = (*Jenv)->NewStringUTF(Jenv,ANDROID_FILE_PATH);
	//test
	char *fileName1 = srTxnData.srParameter.bCRT_FileName;
	//log_msg(LOG_LEVEL_ERROR,"fileName1: %s,%s",fileName1,srTxnData.srParameter.bCRT_FileName);
	jstMSG5 = (*Jenv)->NewStringUTF(Jenv,fileName1);
	char *fileName2 = srTxnData.srParameter.bKey_FileName;
	jstMSG6 = (*Jenv)->NewStringUTF(Jenv,fileName2);
    int caFlag;
    if(strlen(srTxnData.srParameter.bCRT_FileName) > 0)
        caFlag = 1;
    else
       caFlag = 0;

	jbyteArray result=(*Jenv)->NewByteArray(Jenv, inSendLen);
	jbyte *jy = (jbyte*)bSendData;
	(*Jenv)->SetByteArrayRegion(Jenv, result, 0, inSendLen, jy);

	resultI = (*Jenv) -> CallIntMethod(Jenv,obj,AndroidSSLSend,result,jstMSG2,jstMSG3,jstMSG4,jstMSG5,jstMSG6,caFlag,srTxnData.srParameter.gTCPIPTimeOut);
	if(resultI != SUCCESS)
	{
		vdSSLCloseSocket();
		log_msg(LOG_LEVEL_ERROR,"AndroidSSLSend fail: %d",resultI);
		return ICER_ERROR;
	}
#else
int inRetVal;

	inRetVal = ECC_SSLConnect(IP,Port,10);
	if(inRetVal != 0)
	{
		log_msg(LOG_LEVEL_ERROR,"ECC_SSLConnect failed(%s)(%d)",IP,Port);
		return ICER_ERROR;
	}

	inRetVal = ECC_SSLSend(bSendData,inSendLen,srTxnData.srParameter.gTCPIPTimeOut);
	if(inRetVal != 0)
	{
		ECC_SSLClose();
		log_msg(LOG_LEVEL_ERROR,"ECC_SSLSend failed(%d)",inRetVal);
		return ICER_ERROR;
	}

#endif

	return SUCCESS;
}

int inSSLRecvData(BYTE *bRecvData,int inTCPHeaderLen)
{
int totalRecvLen = 0;
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==WINDOWS_CE_API
int inRetVal;
unsigned short inTmpLen = 0;
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
clock_t lnStartTime, lnEndTime;
#else
time_t lnStartTime, lnEndTime;
#endif

  #if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
	lnStartTime = GetTickCount();
  #else
    lnStartTime = time((time_t*)0);
  #endif

	totalRecvLen=0;

  #if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
	//inRetVal = ECC_BIO_read(m_pSockBIO, &bRecvData[totalRecvLen], MAX_XML_FILE_SIZE);
	//inRetVal = ECC_SSL_read(m_pSSL, &bRecvData[totalRecvLen], MAX_XML_FILE_SIZE);
	inRetVal = ssl_ecc_read(socketClient, m_pSSL, &bRecvData[totalRecvLen], MAX_XML_FILE_SIZE);
  #else
	//inRetVal = SSL_read(ssl, &bRecvData[totalRecvLen], MAX_XML_FILE_SIZE);
	inRetVal = ssl_ecc_read(sockfd, ssl,  &bRecvData[totalRecvLen], MAX_XML_FILE_SIZE);
  #endif

	totalRecvLen += inRetVal;

	if(inRetVal <= 0)
	{
		vdSSLCloseSocket();
		log_msg(LOG_LEVEL_ERROR,"inSSLRecvData failed nLen = %d,totalRecvLen = %d",inRetVal,totalRecvLen);
		return ICER_ERROR;
	}

	if(inTCPHeaderLen == 0)
	{
		vdSSLCloseSocket();
		return totalRecvLen;
	}

	totalRecvLen = inRetVal;
	if(totalRecvLen >= inTCPHeaderLen)
		inTmpLen = bRecvData[0] * 256 + bRecvData[1];

	while(inTmpLen > 0 && inTmpLen > totalRecvLen)
	{
		if(inTmpLen == 0 && totalRecvLen >= inTCPHeaderLen)
			inTmpLen = bRecvData[0] * 256 + bRecvData[1];


  #if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
		//inRetVal = ECC_BIO_read(m_pSockBIO, &bRecvData[totalRecvLen], MAX_XML_FILE_SIZE);
		inRetVal = ECC_SSL_read(m_pSSL, &bRecvData[totalRecvLen], MAX_XML_FILE_SIZE);
  #else
		inRetVal = SSL_read(ssl, &bRecvData[totalRecvLen], MAX_XML_FILE_SIZE);
  #endif
		if(inRetVal < 0)
		{
			vdSSLCloseSocket();
			log_msg(LOG_LEVEL_ERROR,"ECC_SSLRecv error 2: %d",inRetVal);
			return ICER_ERROR;
		}

		totalRecvLen += inRetVal;

  #if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
		lnEndTime = GetTickCount();
  #else
		lnEndTime = time((time_t*)0);
  #endif

		if(lnEndTime < lnStartTime)
			lnStartTime &= 0x80000000;
  #if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
        if((lnEndTime - lnStartTime) >= srTxnData.srParameter.gTCPIPTimeOut * 1000)
  #else
  		if((lnEndTime - lnStartTime) >= srTxnData.srParameter.gTCPIPTimeOut)
  #endif
		{
			vdSSLCloseSocket();
			log_msg(LOG_LEVEL_ERROR,"%ld(lnEndTime) - %ld(lnStartTime) = %ld",lnEndTime,lnStartTime,lnEndTime - lnStartTime);
			return PORT_READTIMEOUT_ERROR ;
		}
	}

	vdSSLCloseSocket();

/*#elif READER_MANUFACTURERS==LINUX_API//this is mean define LINUX_API

int nLen;
time_t prev_time, curr_time;

	totalRecvLen=0;
	prev_time = time((time_t*)0);

	do
	{
		nLen = SSL_read(ssl, bRecvData+totalRecvLen, MAX_XML_FILE_SIZE);
		totalRecvLen += nLen;

		if(nLen > 0)
			break;

		curr_time = time((time_t*)0);
		if(curr_time - curr_time > 10)
		{
			vdSSLCloseSocket();
			log_msg(LOG_LEVEL_ERROR,"inSSLRecvData 1 failed nLen = %d,totalRecvLen = %d",nLen,totalRecvLen);
			return ICER_ERROR;
		}
	}
	while(1);
	vdSSLCloseSocket();
*/
#elif READER_MANUFACTURERS==ANDROID_API
int inRetVal;
unsigned short inTmpLen = 0;

	totalRecvLen=0;

	inRetVal = inSSL_Read(&bRecvData[totalRecvLen]);

	totalRecvLen += inRetVal;

	if(inRetVal <= 0)
	{
		vdSSLCloseSocket();
		log_msg(LOG_LEVEL_ERROR,"inSSLRecvData failed nLen = %d,totalRecvLen = %d",inRetVal,totalRecvLen);
		return ICER_ERROR;
	}

	if(inTCPHeaderLen == 0)
	{
		vdSSLCloseSocket();
		return totalRecvLen;
	}

	totalRecvLen = inRetVal;
	if(totalRecvLen >= inTCPHeaderLen)
		inTmpLen = bRecvData[0] * 256 + bRecvData[1];

	while(inTmpLen > 0 && inTmpLen > totalRecvLen)
	{
		if(inTmpLen == 0 && totalRecvLen >= inTCPHeaderLen)
			inTmpLen = bRecvData[0] * 256 + bRecvData[1];

		inRetVal = inSSL_Read(&bRecvData[totalRecvLen]);

		if(inRetVal < 0)
		{
			vdSSLCloseSocket();
			log_msg(LOG_LEVEL_ERROR,"ECC_SSLRecv error 2: %d",inRetVal);
			return ICER_ERROR;
		}

		totalRecvLen += inRetVal;
	}

	vdSSLCloseSocket();

#else

int inRetVal;
#if READER_MANUFACTURERS==NE_VX520
unsigned short inRecvLen = (unsigned short)inTCPHeaderLen;
#else
unsigned short inRecvLen = inTCPHeaderLen;
#endif
unsigned short inTmpLen = 0,inStartLen = 0;


	inRetVal = ECC_SSLRecv(&bRecvData[inStartLen],&inRecvLen,srTxnData.srParameter.gTCPIPTimeOut);
	if(inRetVal != SUCCESS || (int)inRecvLen < 0)
	{
		ECC_SSLClose();
		log_msg(LOG_LEVEL_ERROR,"ECC_SSLRecv error 1: %d,inRecvLen = %d",inRetVal,inRecvLen);
		return ICER_ERROR;
	}

	if(inTCPHeaderLen == 0)
	{
		ECC_SSLClose();
		return inRecvLen;
	}

	inStartLen = inRecvLen;
	if(inStartLen >= inTCPHeaderLen)
		inRecvLen = inTmpLen = bRecvData[0] * 256 + bRecvData[1] + inTCPHeaderLen;

	while(inTmpLen > 0 && inTmpLen > inStartLen)
	{
		if(inTmpLen == 0 && inStartLen >= inTCPHeaderLen)
			inRecvLen = inTmpLen = bRecvData[0] * 256 + bRecvData[1] + inTCPHeaderLen;

		inRetVal = ECC_SSLRecv(&bRecvData[inStartLen],&inRecvLen,srTxnData.srParameter.gTCPIPTimeOut);
		if(inRetVal != SUCCESS || (int)inRecvLen < 0)
		{
			ECC_SSLClose();
			log_msg(LOG_LEVEL_ERROR,"ECC_SSLRecv error 2: %d,inRecvLen = %d",inRetVal,inRecvLen);
			return ICER_ERROR;
		}

		inStartLen += inRecvLen;
	}

	ECC_SSLClose();
	totalRecvLen = inRecvLen = inStartLen;

#endif

	return totalRecvLen;
}

void vdSSLCloseSocket()
{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API// || READER_MANUFACTURERS==LINUX_API

	ECC_BIO_free_all(m_pSockBIO);
	ECC_SSL_CTX_free(m_pSSLCtx);

#elif READER_MANUFACTURERS==LINUX_API//this is mean define LINUX_API

	SSL_CTX_free ( m_pSSLCtx ) ;
	SSL_shutdown ( ssl ) ;
	SSL_free ( ssl ) ;
    close(sockfd);

#elif READER_MANUFACTURERS==ANDROID_API
	//log_msg(LOG_LEVEL_ERROR,"vdSSLCloseSocket Start:");
	(*Jenv) -> CallVoidMethod(Jenv,obj,AndroidSSLClose);
#else

#endif
}

#if READER_MANUFACTURERS==ANDROID_API
int inSSL_Read(BYTE *bOutData)
{
int inRetVat;
jint i;
const char *jBuf;
char bTest[4101];
jstring jstMSG1,jstMSG2,jstMSG3;
jbyteArray readArray;
BYTE request[4000];

	readArray = (*Jenv) -> CallObjectMethod(Jenv,obj,AndroidSSLRead,srTxnData.srParameter.gTCPIPTimeOut);
	if(readArray != NULL)
	{
		jsize len = (*Jenv) -> GetArrayLength(Jenv,readArray);
		jbyte* jbarray = (jbyte*) ECC_calloc(len,sizeof(jbyte));
		(*Jenv) -> GetByteArrayRegion(Jenv,readArray,0,len,jbarray);
		BYTE *buf = (BYTE*)jbarray;
		memcpy(bOutData,jbarray,len);
		ECC_free(jbarray);
		inRetVat = len;
	}
	else
	{
		log_msg(LOG_LEVEL_ERROR,"inSSLRecvData fail:");
		return ICER_ERROR ;
	}

	return inRetVat;
}

#endif

