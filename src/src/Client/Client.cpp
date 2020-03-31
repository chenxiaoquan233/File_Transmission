#include "../../include/Client/Client.h"

Client::Client()
{

}

Client::~Client()
{

}

int Client::send_file(char* input_file_name)
{
    input_file = fopen(input_file_name, "rb");
    while (!feof(input_file))
    {
        read_file(input_file_name);
        get_ack();
        bool res = send_packet(MAX_PACKET_DATA_BYTE_LENGTH);
    }
    return 0;
}

bool Client::read_file(char* input_file_name)
{
    if(data) delete data;//data equals nullptr the first time, but not after
    data = new packet_load();

    if(!data->create_file_slice(MAX_PACKET_DATA_BYTE_LENGTH))
        return false;

    int res;
    if (isfirstread == true)
    {
        int text_len = filesize(input_file_name);
        int offset = strlen(input_file_name) + sizeof(char) + ceil(log10(text_len)) + 1;

        sprintf(data->get_file_slice(), "%s?%d@", input_file_name, text_len);
        res = fread(data->get_file_slice() + offset, 1, MAX_PACKET_DATA_BYTE_LENGTH - offset, input_file);
        isfirstread = false;
    }
    else
        res = fread(data->get_file_slice(), 1, MAX_PACKET_DATA_BYTE_LENGTH, input_file);
    return res;
}

bool Client::set_up_connection(const char* ip_addr, int port)
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
    serv_addr.sin_addr.s_addr = inet_addr(ip_addr);
    serv_addr.sin_port = htons(port);
    if(bind(sock, (struct sockaddr*)&serv_addr, sizeof(sockaddr)) == -1)
        return false;
    return true;
}

bool Client::send_packet(int len)
{
    int res=sendto(sock, data->get_file_slice(), len, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    return res!=-1;
}

int Client::filesize(char* input_file_name)
{
    int length = 0;
    FILE* fp;
    fp = fopen(input_file_name, "rb");
    if (fp == NULL)
    {
        return -1;
    }
    fseek(fp, 0L, SEEK_END);
    length = ftell(fp);
    fclose(fp);
    return length;
}

bool Client::get_ack()
{
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 200;
#ifdef __linux__
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
        return false;
#endif
#ifdef WIN32
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) == -1)
		return false;
#endif
    
    char num_buffer[4];
#ifdef __linux__
    socklen_t nSize = sizeof(sockaddr);
#endif
#ifdef WIN32
	int nSize = sizeof(sockaddr);
#endif
    recvfrom(sock, num_buffer, 4, 0, (struct sockaddr*)&serv_addr, &nSize);

    return atoi(num_buffer) == data->get_slice_num();
}