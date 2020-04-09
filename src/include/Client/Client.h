#pragma once

#include "../base.h"
#include "../packet_load.h"
#include "../file.h"

class Client : public base
{
private:
	File* file;
	pkt_load* data = nullptr;
	sockaddr_in serv_addr;
#ifdef _WIN32
	SOCKET cmd_sock;
#endif
#ifdef __linux__
	int cmd_sock;
#endif

	
public:
	//constructor
	Client();

	//destructor
	~Client();

	//Incomplete parameters 
	int send_file(char* input_file_name);

	//read file slice from a specified file
	//stored in file_slice
	//length constrainted by MAX_PACKET_DATA_BYTE_LENGTH in base.h
	//return status
	bool read_file(char* input_file_name);

	//set up ip connection to a server
	//return status
	bool set_up_connection(const char* ip_addr, int port);

	//send packet on connection
	//return status
	bool send_packet(int len);

	bool get_ack();

	int read_path(const char* path, char* buffer[]);
};