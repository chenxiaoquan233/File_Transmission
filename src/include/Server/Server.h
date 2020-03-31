#include "../base.h"
#include "../packet_load.h"

class Server : public base
{
private:
	packet_load* data;
	sockaddr_in serv_addr;
	FILE* output_file = nullptr;
#ifdef _WIN32
    SOCKET sock;
#endif
#ifdef __linux__
	int sock;
#endif
public:
	//constructor
	Server();
	//destruuctor
	~Server();
	
	//set up listen on specified port
	//parse data and store in file_slice
	//return status
	bool set_listen(int port);

	//start listen
	bool recv_packet();

	//read the information of file head(FILE path & FILE length)
	//return length of infomation
	int read_FILEinformation(FILE* &output_file, char* origin_data, int &data_length);

	//write file slice to file
	//return status
	bool write_file(FILE* output_file,char * data,int data_length);
};