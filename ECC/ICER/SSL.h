#ifndef __SSL_H__
#define __SSL_H__

#ifdef __cplusplus
		 extern "C" {
#endif

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
	#include "openssl\err.h"
	#include "openssl\evp.h"
	#include "openssl\ssl.h"

	typedef struct ecrObject
	{
		int ( *ecrRun)(void *);
		int ( *settingInterface)(void *);
		int ( *isDataIn)(void*);

	} SSL_OBJ;

	typedef void (*SSL_FUNC_TYPE_1)(void);
	typedef int (*SSL_FUNC_TYPE_2)(void);
	typedef const SSL_METHOD *(*SSL_FUNC_TYPE_3)(void);
	typedef SSL_CTX *(*SSL_FUNC_TYPE_4)( SSL_METHOD *);
	typedef BIO *(*SSL_FUNC_TYPE_5)(SSL_CTX *);
	typedef long (*SSL_FUNC_TYPE_6)(BIO *, int , long , void *);
	//#define BIO_get_ssl(b,sslp)			BIO_ctrl(b,BIO_C_GET_SSL,0,(char *)sslp)
	//#define BIO_set_conn_ip(b,ip) 		BIO_ctrl(b,BIO_C_SET_CONNECT,2,(char *)ip)
	//#define BIO_set_conn_int_port(b,port) BIO_ctrl(b,BIO_C_SET_CONNECT,3,(char *)port)

	typedef long (*SSL_FUNC_TYPE_7)(SSL *, int , long , void *);
	//# define SSL_set_mode(ssl,op)  SSL_ctrl((ssl),SSL_CTRL_MODE,(op),NULL)

	//#define BIO_do_connect(b) 	 BIO_do_handshake(b)
	//#define BIO_do_accept(b)		 BIO_do_handshake(b)
	//#define BIO_do_handshake(b)	 BIO_ctrl(b,BIO_C_DO_STATE_MACHINE,0,NULL)

	typedef unsigned long (*SSL_FUNC_TYPE_8)(void);
	typedef char *(*SSL_FUNC_TYPE_9)(unsigned long , char *);
	typedef int (*SSL_FUNC_TYPE_10)(BIO *, const void *, int );
	typedef int (*SSL_FUNC_TYPE_11)(BIO *, void *, int );
	typedef void (*SSL_FUNC_TYPE_12)(BIO *);
	typedef void (*SSL_FUNC_TYPE_13)(SSL_CTX *);
	typedef int  (*SSL_FUNC_TYPE_14)(SSL_CTX *, const char*, int);
	typedef int  (*SSL_FUNC_TYPE_15)(SSL_CTX *);
	typedef int  (*SSL_FUNC_TYPE_16)(SSL *, int);
	typedef SSL * (*SSL_FUNC_TYPE_17)(SSL_CTX *);
	typedef int (*SSL_FUNC_TYPE_18)(SSL *);
	typedef int (*SSL_FUNC_TYPE_19)(SSL *, const void *, int);
	typedef int (*SSL_FUNC_TYPE_20)(SSL *, void *, int);
	typedef long (*SSL_FUNC_TYPE_21)(SSL_CTX *,  long);
	typedef int (*SSL_FUNC_TYPE_22)(SSL *,  int);
#endif

int inSSLInitial(void);
void inSSLFinalizer(void);
void vdSSLFreeLibrary(void);
int inSSLSendData(unsigned char *IP,unsigned short Port,BYTE *bSendData,int inSendLen);
int inSSLRecvData(BYTE *bRecvData,int inTCPHeaderLen);
void vdSSLCloseSocket(void);

#if READER_MANUFACTURERS==ANDROID_API
int inSSL_Read(BYTE *bOutData);
#endif

#ifdef __cplusplus
		 }
#endif // __cplusplus

#endif
