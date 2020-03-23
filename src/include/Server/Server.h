#include "../base.h"

class Server : public base
{
private:
	char* file_slice;

public:
	//constructor
	Server();

	//destruuctor
	~Server();
	
	//set up listen on specified port
	//parse data and store in file_slice
	//return status
	bool set_listen();

	//write file slice to file
	//return status
	bool write_file(FILE* output_file);
};