#include "../../include/Server/Server.h"

Server::Server()
{

}

Server::~Server()
{

}

bool Server::set_listen()
{
	
}

int Server::read_FILEinformation(FILE*& output_file, char* origin_data, int& data_length)
{
    int info_length = 0;
    char FILE_path[50];
    data_length = 0;
    while (origin_data[info_length] != '?') {
        FILE_path[info_length] = origin_data[info_length];
        FILE_path[info_length + 1] = '\0';
        info_length++;
    }
    info_length++;
    while (origin_data[info_length] != '@')
    {
        data_length *= 10;
        data_length += (origin_data[info_length] - '0');
        info_length++;
    }
    output_file = fopen(FILE_path, "wb");
    return info_length + 1;
}

bool Server::write_file(FILE* output_file,char * data,int data_length)
{
	for (int data_i = 0; data_i < data_length; data_i++)
		fputc(data[data_i],output_file);
	return true;
}