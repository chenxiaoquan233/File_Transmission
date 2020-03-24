#include "../../include/Client/Client.h"

Client::Client(const char* ip_addr, int port)
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    sock = socket(PF_INET, SOCK_DGRAM, 0);
#endif
#ifdef __linux__
    sock = socket(AF_INET, SOCK_DGRAM, 0);
#endif

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = PF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip_addr);
    serv_addr.sin_port = htons(port);
}

Client::~Client()
{

}

bool Client::read_file(FILE* input_file)
{

}

bool Client::set_up_connection()
{

}

bool Client::send_packet(int len)
{
    int res=sendto(sock, file_slice, len, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    return res!=-1;
}