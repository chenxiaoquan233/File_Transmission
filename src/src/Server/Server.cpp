#include "../../include/Server/Server.h"
Server::Server()
{

}
Server::Server(const char* ip_addr, int port)
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    //创建套接字
    sock = socket(PF_INET, SOCK_DGRAM, 0);
#endif
#ifdef __linux__
    sock = socket(AF_INET, SOCK_DGRAM, 0);
#endif
    //服务器地址信息
    memset(&serv_addr, 0, sizeof(serv_addr));  //每个字节都用0填充
    serv_addr.sin_family = PF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip_addr);
    serv_addr.sin_port = htons(port);
}
Server::~Server()
{

}

bool Server::set_listen()
{
    int res = sendto(sock, file_slice, MAX_PACKET_DATA_BYTE_LENGTH, 0, (struct sockaddr*) & serv_addr, sizeof(serv_addr));
    return res != -1;
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