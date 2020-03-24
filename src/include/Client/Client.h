#pragma once

#include "../base.h"

class Client : public base
{
private:
	char* file_slice;
	FILE* input_file = nullptr;
	bool isfirstread = true;
	sockaddr_in serv_addr;
#ifdef _WIN32
	SOCKET sock;
#endif
#ifdef __linux__
	int sock;
#endif

	
public:
	//constructor
	Client();
	Client(const char* ip_addr, int port);

	//destructor
	~Client();

	//Incomplete parameters 
	int client(char* input_file_name);

	//read file slice from a specified file
	//stored in file_slice
	//length constrainted by MAX_PACKET_DATA_BYTE_LENGTH in base.h
	//return status
	bool read_file(char* input_file_name);

	//set up ip connection to a server
	//return status
	bool set_up_connection();

	//send packet on connection
	//return status
	bool send_packet(int len);
	int filesize(char* input_file_name);
};