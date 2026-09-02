#ifndef __LIB_ECC_HAL_EXTERNAL_H__
#define __LIB_ECC_HAL_EXTERNAL_H__

#include "LibEccHal.h"

#ifdef __cplusplus
extern "C"
{
#endif

/********************************************************************
 * TCP/IP EXTERNAL FUNCTIONS                                                 *
 ********************************************************************/
typedef short (*ECC_TCPIPConnectEx_Lib)(IN unsigned char *bIP, IN unsigned short usPort, IN unsigned short usTimeOut);
typedef short (*ECC_TCPIPSendEx_Lib)(IN unsigned char *bSendData, IN unsigned short usSendLen, IN unsigned short usTimeOut);
typedef short (*ECC_TCPIPRecvEx_Lib)(OUT unsigned char *bRecvData, OUT unsigned short *usRecvLen, IN unsigned short usTimeOut);
typedef short (*ECC_TCPIPCloseEx_Lib)(void);
typedef struct {
    ECC_TCPIPConnectEx_Lib cbEccTcpIpConnect;
    ECC_TCPIPSendEx_Lib cbEccTcpIpSend;
    ECC_TCPIPRecvEx_Lib cbEccTcpIpRecv;
    ECC_TCPIPCloseEx_Lib cbEccTcpIpClose;
} stEccTcpIpExFuns;
extern short ECC_RegTCPIP_External_Lib(IN void *pConnect, IN void *pSend, IN void *pRecv, IN void *pClose);

#ifdef __cplusplus
}
#endif

#endif // __LIB_ECC_HAL_EXTERNAL_H__
