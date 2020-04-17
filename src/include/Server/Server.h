#include "../base.h"
#include "../file.h"
#include "../packet_load.h"

class Server : public base
{
private:
	pkt_load* data;
	sockaddr_in serv_addr_cmd;
	sockaddr_in serv_addr_data;
	int cmd_port;
	File* file;
	//shore the received data for a short time
	char* buffer = nullptr;
#ifdef _WIN32
    SOCKET cmd_sock;
	SOCKET data_sock;
#endif
#ifdef __linux__
	int cmd_sock;
	int data_sock;
#endif
public:
	//constructor
	Server(int port);
	//destruuctor
	~Server();
	
	//set up listen on specified port
	//parse data and store in file_slice
	//return status
	bool set_listen();

		
    //start listen,reveive packet to file_slice
	bool recv_packet();

	
	//receive a whole file(same path)
	bool recv_whole_file();

	//send current slice num to client
	bool send_ack(int num);
	
	//read data from file_slice and return the length of data
	int get_file_data(char* src, char* dest, int maxlen);
	
	//read the information of file head(FILE path & FILE length)
	//return length of infomation
	int read_FILEinformation(FILE* &output_file, char* origin_data, int &data_length);

	//write file slice to file
	//return status
	bool write_file(FILE* output_file,char* data,int data_length);

	//return packet serial number
	int return_packet_serial_number();

	//get params
	void parse_param(char*, char*, int*, char*, int, int*);

	//Check that the folder path does not create a folder
	int set_dir(char* path);

	//find a usable  port
	//return port number
	int check_port();

	//read in path information and return info and use function set_dir(char* path)
	bool parse_path();

	//analytical command
	bool parse_cmd();

	//parameter checking
	bool parse_arg(int argc, char** argv);

	//when receive a info,first use it to write total packet number,then write file leghth
	//when receive a packet,use it write packet No.
	bool write_logfile(char* path, int number, int size);

	//check whether a file need to re-upload
	bool check_file(char* file_name, int file_len, int pkt_num);

	//combine file slice
	bool mergeFile(char* file_name, int pkt_num);

	int get_ack(char* data);
};