#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#include <winsock.h>
#pragma comment (lib, "ws2_32.lib")
#endif

#ifdef _linux_
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

//set common prefixes here
class base
{
	const int MAX_PACKET_DATA_BYTE_LENGTH = 0;//to be modified
};