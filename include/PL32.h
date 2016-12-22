#include <windows.h>
extern "C" int WINAPI pl32DLLVersionGet(void);
extern "C" int WINAPI pl32ComStatus( int nComNo );
extern "C" int WINAPI pl32ComOpen( int nComNo, int nBaudRate );
extern "C" int WINAPI pl32ComClose(void);
extern "C" int WINAPI pl32DevProtocolSet( int nDevPtcl );
extern "C" int WINAPI pl32DevCmdSend( int nDevId, char *szCmdReply );
extern "C" int WINAPI pl32DevReplyGet( int *nDevStatusCode, char *szCmdReply );
extern "C" int WINAPI pl32DevCmdSendReplyGet( int nDevId, int *nDevStatusCode, char *szCmdReply );
extern "C" int WINAPI pl32DevStatus( int nDevId );
extern "C" int WINAPI pl32DevRetrySet( int nDevRetries );
extern "C" int WINAPI FlushTransmitBuffer();
extern "C" int WINAPI pl32DevRetrySet( int nDevRetries );
extern "C" int WINAPI ser_rs232_flush();     