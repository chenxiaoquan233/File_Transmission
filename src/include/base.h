#pragma once

// MSVC DEFINES
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#define _XKEYCHECK_H
#endif

// COMMON INCLUDE
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

// WINDOWS INCLUDE
#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include <winsock.h>
#pragma comment (lib, "ws2_32.lib")
#endif

//LINUX INCLUDE
#ifdef __linux__
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#endif

//set common prefixes here
class base
{
	protected:
		const int MAX_PACKET_DATA_BYTE_LENGTH = 528;//adviced length
		const int UPD_HEADER_LENGTH = 19;
		const int SEND_FREQ = 10;
};