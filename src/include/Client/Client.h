#pragma once

#include "../base.h"

class Client : public base
{
private:
	char* file_slice;
	sockaddr_in serv_addr;
#ifdef _WIN32
	SOCKET sock;
#endif
#ifdef __linux__
	int sock;
#endif

	
public:
	//constructor
	Client(const char* ip_addr, int port);

	//destructor
	~Client();

	//read file slice from a specified file
	//stored in file_slice
	//length constrainted by MAX_PACKET_DATA_BYTE_LENGTH in base.h
	//return status
	bool read_file(FILE* input_file);

	//set up ip connection to a server
	//return status
	bool set_up_connection();

	//send packet on connection
	//return status
	bool send_packet(int len);
};