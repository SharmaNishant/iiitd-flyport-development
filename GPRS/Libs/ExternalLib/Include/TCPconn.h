#include "TCPlib.h"

void TCPConnect(TCP_SOCKET *sockTcp, char *IP, char *Port);

int TCPSend(TCP_SOCKET *sockTcp, char *msg);

int lTCPSend(TCP_SOCKET *sockTcp, char *msg);

int TCPRecieve(TCP_SOCKET *sockTCP, char inBuff[]);

int lTCPRecieve(TCP_SOCKET *sockTcp, char inBuff[]);

int TCPClose(TCP_SOCKET *sockTcp);
