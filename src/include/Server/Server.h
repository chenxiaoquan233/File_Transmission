#include "../base.h"
#include "../packet_load.h"

class Server : public base
{
private:
	packet_load* data;
	sockaddr_in serv_addr;
	FILE* output_file = nullptr;
	//shore the received data for a short time
	char* buffer;
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
	bool write_file(FILE* output_file,char * data,int data_length);

	//return packet serial number
	int return_packet_serial_number();
};