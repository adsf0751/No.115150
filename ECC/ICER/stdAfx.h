// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#ifdef __cplusplus
	extern "C" {
#endif

#if !defined(AFX_STDAFX_H__674FD015_9ED4_4AC0_B480_3C9F5CAF4C42__INCLUDED_)
#define AFX_STDAFX_H__674FD015_9ED4_4AC0_B480_3C9F5CAF4C42__INCLUDED_
		
#if defined _MSC_VER
	#if _MSC_VER > 1000
	#pragma once
	#endif // _MSC_VER > 1000
#endif

// TODO: reference additional headers your program requires here
//#pragma comment(lib, "libeay32.lib")
//#pragma comment(lib, "ssleay32.lib")

//#pragma comment(lib, "comsupp.lib")

#define EXTERNAL_READER 0
#define INTERNAL_READER 1

#define WINDOWS_API		1
#define LINUX_API		2
#define ANDROID_API		3
//#define NE_MICROPROGRAM				2
//#define NE_CASTLES					3
//#define NE_BAORUH					4
//#define NE_POSIFLEX					5
#define NE_PAXSXX					6
#define NE_VX520					9
#define CS_PAXSXX					10
#define SYSTEX_CASTLES     11
#define NE_CASTLES_API		15
#define NE_SYMLINK_API		16
#define WINDOWS_CE_API		17
#define UNI_TS890               18
#define CS_PAXPROLIN            19
#define BINODATA_API		20
#define ING_TERMINAL	             21       // airlink ingenic desk5000
#define NEXSYS_CASTLES		22

#define	NE_X86				0
#define NE_MF1000         	1
#define NE_FEP_V3			2
#define NE_RPI_3			3
#define NE_NEXSYS_V3		4
#define NE_DESK5000			5
//#define	FLASH_SYSTEM

#if READER_MANUFACTURERS==NE_PAXSXX
	#include <LibApiAirlink.h>
	//#include <TRACE.h>
	//#include <EasyCard.h>
	#include <LIB.h>
	#include <LibECC_LibApiAirlink.h>
	#include <stdarg.h>
	#include <PAX.h>
	#define	clock_t		unsigned long

#elif READER_MANUFACTURERS==CS_PAXSXX

	#include "posapi.h"
	#include "../Source/libs80.h"
	#include <stdarg.h>
	#define	clock_t		unsigned long

#elif  READER_MANUFACTURERS==SYSTEX_CASTLES

	#include <string.h>
	#include <stdio.h>
	#include <stdarg.h>
	#include "icerdefine.h"

	#include "ctosapi.h"

	//
	#include "libicer.h"

#elif  READER_MANUFACTURERS==WINDOWS_API

	#pragma comment(lib, "wsock32.lib")

	//#include <iostream.h>
	//#include <winsock2.h>
	#include <stdio.h>
	#include <winsock.h>
	#include <direct.h>
	#include <windows.h>
	#include <io.h>
	#include <time.h>

  #ifdef CTOS_USB
	#include "usblib.h"
  #endif

#elif  READER_MANUFACTURERS==LINUX_API

	#include <syslog.h>
	#include <stdint.h>
	#include <stdarg.h>
	#include <poll.h>
	#include <time.h>
	#include <limits.h>
	#include <execinfo.h>
	#include <dlfcn.h>
	#include <dirent.h>
	#include <regex.h>
	#include <sys/mman.h>
	#include <sys/uio.h>
	#include <sys/ioctl.h>
	#include <sys/time.h>


	#include <unistd.h>
	#include <assert.h>
	#include <errno.h>
	#include <strings.h>
	#include <signal.h>
	#include <getopt.h>
	#include <pthread.h>
	#include <fcntl.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <termios.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <netinet/in.h>

	#include <string.h>
	#include <stdlib.h>
	#include <stdio.h>

	#include <ctype.h>
	#include <netdb.h>

	#if RDEVN==NE_MF1000
		#include "ecc_hal_api.h"

	#elif RDEVN==NE_FEP_V3

		#include <ctype.h>

		#include <ctosapi.h>
		#include "ECC_API.h"
	#elif RDEVN == NE_NEXSYS_V3
		#include <ctosapi.h>
		#include <netinet/tcp.h>
		#include "LibEccHal.h"
	#endif

#elif READER_MANUFACTURERS==ANDROID_API

	#include <syslog.h>
	#include <stdint.h>
	#include <stdarg.h>
	#include <poll.h>
	#include <limits.h>
	#include <dlfcn.h>
	#include <dirent.h>
	#include <sys/mman.h>
	#include <sys/uio.h>


	#include <unistd.h>
	#include <assert.h>
	#include <errno.h>
	#include <strings.h>
	#include <signal.h>
	#include <getopt.h>
	#include <pthread.h>
	#include <fcntl.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <termios.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <netinet/in.h>
	#include <regex.h>
	#include <stdlib.h>

	#include<icerapi_icerapi_ICERAPI.h>
	#include <android/log.h>

#elif READER_MANUFACTURERS==NE_CASTLES_API
	#include <stdarg.h>
	#include <time.h>
	#include <ctype.h>

	#include <ctosapi.h>

	#include "libecc.h"

#elif READER_MANUFACTURERS==NE_SYMLINK_API
	#include <stdarg.h>
	#include <ctype.h>

	#include "posapi.h"
	#include "ECC_Define.h"
	#include "ECC_Function.h"
	#include "POSAPI.H"

	#include <string.h>
	#include <stdlib.h>
	#include <stdio.h>

#elif READER_MANUFACTURERS==NE_VX520

	//#include "LIB.h"
	#include <CMASlib.h>
	#include <CmasApLib.h>
	#include <CMASfilelib.h>
	#include <stdarg.h>
	#include <time.h>
	#include <ctype.h>
	#include <stddef.h>
	//#include "regex.h"

#elif  READER_MANUFACTURERS==WINDOWS_CE_API

//	#include <iostream.h>
	#include <winsock2.h>
//	#include <direct.h>
	#include <windows.h>
//	#include <io.h>
	#include <time.h>
	#include <Winbase.h>

//	#include "usblib.h"
	#pragma comment(lib,"wsock32.lib ")
	#pragma comment(lib, "Ws2.lib")


#elif  READER_MANUFACTURERS==UNI_TS890
        #include <../../libecc/include/libecc/libecc.h>
        #include <time.h>
        #include <stdarg.h>
	#include <string.h>
	#include <stdio.h>
	#include <stdlib.h>
#elif READER_MANUFACTURERS==CS_PAXPROLIN

	#include <string.h>
	#include <stdio.h>
	#include <stdarg.h>
	#include <time.h>
	#include <ctype.h>

	#include "libecc.h"
#elif READER_MANUFACTURERS==BINODATA_API
	#include <time.h>
	#include <ctype.h>
	
	#include <stdio.h>
	#include <stdarg.h>
	
	#include "DataStructDefine.h"
	#include "ECC_DataTransfer.h"
	#include "ECC_File.h"
	#include "ECC_RS232.h"
	#include "ECC_SSL.h"
	#include "ECC_TCPIP.h"
	#include "ECC_Timer.h"
#elif READER_MANUFACTURERS==BINODATA_API
#elif READER_MANUFACTURERS==ING_TERMINAL // INGENICO DESK5000
	#include <stdio.h>
      #include <stdlib.h>
      #include <stdarg.h>
      #include <string.h>
      #include <time.h>
      #include <ctype.h>
      #include <ECCBASE_Types.h>
      #include <ECCBASE_API.h>
#elif READER_MANUFACTURERS==NEXSYS_CASTLES
	#include <stdarg.h>
	#include <time.h>

	#include <string.h>
	#include <stdio.h>
	#include <stdlib.h>
	
	#include <ctype.h>

	#include <ctosapi.h>
	#include <netinet/tcp.h>
	#include "ECCLib.h"
	#include "CMASlib.h"
#endif

#if READER_MANUFACTURERS==NE_PAXSXX
	//#define	BOOL			char
	//#define	BYTE			unsigned char

	#define	TRUE			1
	#define	FALSE			0

	#define __packed_back __attribute__((packed))

	//#define strncasecmp		strncmp
	#define calloc(Par1,Par2)			malloc(Par1*Par2)
	#define FILE_NAME_LIMIT				16

#elif  READER_MANUFACTURERS==CS_PAXSXX
	#define	TRUE			1
	#define	FALSE			0
	#ifdef WIN32
		#define __packed_back
	#else
		#define __packed_back __attribute__((packed))
	#endif

	//#define strncasecmp		strncmp
	#define calloc(Par1,Par2)			malloc(Par1*Par2)
	#define	BOOL			char
	#define	BYTE			unsigned char

	#define FILE_NAME_LIMIT 			16

#elif  READER_MANUFACTURERS==LINUX_API

	#define	BYTE			unsigned char

	#if RDEVN != NE_NEXSYS_V3
		#define	BOOL		char
		typedef unsigned long DWORD;
		typedef unsigned long ULONG;
	#endif

    #if RDEVN==NE_MF1000
        //#define TRUE		1
        //#define FALSE		0

        #define __packed_back __attribute__((packed))
        #define __packed

	#elif RDEVN==NE_FEP_V3

	    #define __packed
		#define __packed_back __attribute__((packed))

	#elif RDEVN==NE_RPI_3

		#define __packed
		#define __packed_back __attribute__((packed))
	#elif RDEVN == NE_NEXSYS_V3
		#define __packed
		#define __packed_back __attribute__((packed))
	#else

		#define __packed_back

	#ifdef CTOS_USB
        #include "xproxusb.h"
	#endif

    #endif
	//#include "qp1000usb.h"

	#define TRUE		1
	#define FALSE		0

	//#define __packed_back

#elif  READER_MANUFACTURERS==ANDROID_API

	#define	BOOL			char
	#define	BYTE			unsigned char

	typedef unsigned long DWORD;

	#define TRUE		1
	#define FALSE		0

	#define __packed_back

#elif READER_MANUFACTURERS==NE_CASTLES_API
	#define __packed
	#define __packed_back __attribute__((packed))

	#include <LIB.h>
#elif READER_MANUFACTURERS==NE_SYMLINK_API

	#define	BOOL			char

	#define TRUE		1
	#define FALSE		0

	#define __packed_back	__attribute__((packed))
	#define __packed

	#define FILE_NAME_LIMIT 			16
	//#define FLASH_SYSTEM

#elif READER_MANUFACTURERS==NE_VX520

	#define	BOOL			char
	#define	BYTE			unsigned char
	#define	clock_t			unsigned long
	#define TRUE			1
	#define FALSE			0
	#define uchar			unsigned char

	#define __packed
	#define __packed_back
	#define LIB_DEFINE
	#define LIB_DEFINE2
	#define ECC_BIG_ENDIAN

#elif  READER_MANUFACTURERS==WINDOWS_CE_API

	#define	BOOL			char
	#define	BYTE			unsigned char

	#define TRUE		1
	#define __packed_back
	//#include <atlconv.h>

#elif  READER_MANUFACTURERS==UNI_TS890
        #define BOOL                    char
        #define BYTE                    unsigned char
        #define __packed_back
        #define __packed
        #define FALSE                   0
        #define TRUE                    1
#elif READER_MANUFACTURERS==CS_PAXPROLIN

	#define	BOOL			char
	#define	BYTE			unsigned char

	#define TRUE			1
	#define FALSE			0

	#define __packed
	#define __packed_back __attribute__((packed))
	#define LIB_DEFINE

#elif READER_MANUFACTURERS==BINODATA_API
	#define __packed_back
#elif READER_MANUFACTURERS==ING_TERMINAL
#elif READER_MANUFACTURERS==NEXSYS_CASTLES
	#define __packed
	#define __packed_back __attribute__((packed))
#else

	#define __packed_back

#endif

#ifdef LIB_DEFINE
	#include "LIB.h"
#endif

#include "CallAPI.h"
#include "Function.h"
#include "Error.h"
#include "TCPIP.h"
#include "ICERAPI.h"
#include "XMLFunc.h"
#include "TM.h"
#include "Com.h"
#include "Global.h"
#include "APDU.h"
#include "CMAS_TM.h"
#include "CMAS_APDU.h"
#include "SSL.h"
#include "SIS2.h"
#include "SIS2_2.h"
#include "BLC.h"
#include "ICERLib.h"
#include "BankData.h"
#include "ICERAes.h"

//#import "MSXML6.dll" rename_namespace("MSXML6") named_guids

extern BYTE gCurrentFolder[MAX_PATH];

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__674FD015_9ED4_4AC0_B480_3C9F5CAF4C42__INCLUDED_)

#ifdef __cplusplus
	}
#endif // __cplusplus
