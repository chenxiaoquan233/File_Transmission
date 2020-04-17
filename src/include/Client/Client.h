#pragma once

#include "../base.h"
#include "../packet_load.h"
#include "../file.h"

class Client : public base
{
private:
	File* file;
	pkt_load* data = nullptr;
	sockaddr_in serv_addr_cmd;
	sockaddr_in serv_addr_data;
	char* ip_addr;
#ifdef _WIN32
	SOCKET cmd_sock;
	SOCKET* data_sock;
#endif
#ifdef __linux__
	int cmd_sock;
	int* data_sock;
#endif

	
public:
	//constructor
	Client(char* ip_addr);

	//destructor
	~Client();

	//Incomplete parameters 
	int send_file(char* input_file_name);

	//read file slice from a specified file
	//stored in file_slice
	//length constrainted by MAX_PACKET_DATA_BYTE_LENGTH in base.h
	//return status
	bool read_file_slice(char* input_file_name);

	//set up ip connection to a server
	//return status
	#ifdef _WIN32
	bool sock_init(SOCKET* sock, const char* ip_addr, int port);
	#endif
	#ifdef __linux__
	bool sock_init(int* sock, int port, int is_cmd);
	#endif

	//change port id
	//return status
	#ifdef _WIN32
	bool set_port(SOCKET* sock, int port);
	#endif
	#ifdef __linux__
	bool set_port(int* sock, int port, int is_cmd);
	#endif
	
	//send packet on connection
	//return status
	bool send_packet(int len);

	bool get_ack();

	//rev offset from server
	int get_offset();

	//rev port id from server
	int get_port();

	int read_path(const char* path, char* path_info_buf, char** file_info_buf);
    
	void send_cmd(char* cmd);
	
	bool send_path_info(char* buffer);

	#ifdef _WIN32
	SOCKET* get_cmd_sock();
	#endif
	#ifdef __linux__
	int* get_cmd_sock();
	#endif

};