#include "../../include/Server/Server.h"

Server::Server()
{

}

Server::~Server()
{

}

bool Server::set_listen(int port)
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    sock = socket(AF_INET, SOCK_DGRAM, 0);
#endif
#ifdef __linux__
    sock = socket(AF_INET, SOCK_DGRAM, 0);
#endif
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);
    int on = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
        return false;
    if(bind(sock, (struct sockaddr*)&serv_addr, sizeof(sockaddr)) == -1)
        return false;
    return true;
}

bool Server::recv_packet()
{
    if(data) delete data;//data equals nullptr the first time, but not after
    data = new packet_load();

    socklen_t nSize = sizeof(sockaddr);
    data->create_file_slice(MAX_PACKET_DATA_BYTE_LENGTH);
    if(recvfrom(sock, data->get_file_slice(), MAX_PACKET_DATA_BYTE_LENGTH, 0, (struct sockaddr*)&serv_addr, &nSize) == -1)
        return false;
    return true;
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