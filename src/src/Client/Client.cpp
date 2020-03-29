#include "../../include/Client/Client.h"

Client::Client()
{

}
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
int Client::client(char* input_file_name)
{
    input_file = fopen(input_file_name, "rb");
    while (!feof(input_file))
    {
        read_file(input_file_name);
        std::cout << file_slice << " !! ";    //内容测试
    }
    return 0;
}
bool Client::read_file(char* input_file_name)
{
    file_slice = new char[MAX_PACKET_DATA_BYTE_LENGTH];
    memset(file_slice, 0, MAX_PACKET_DATA_BYTE_LENGTH);
    int res;
    if (isfirstread == true)
    {
        int len_of_text = filesize(input_file_name);
        char start = 64;//符号@
        char start2 = 63;
        char* len_text = new char[MAX_PACKET_DATA_BYTE_LENGTH];
        sprintf(len_text, "%s%c%d%c",input_file_name,start2, len_of_text, start);		//将“文件名$长度@”加入数组
        res = fread(file_slice, 1, MAX_PACKET_DATA_BYTE_LENGTH - strlen(len_text), input_file);
        res += strlen(len_text);
        int len_tmp = strlen(len_text);
        for (int ii = strlen(len_text); ii < MAX_PACKET_DATA_BYTE_LENGTH; ii++) 
        {
            len_text[ii] = file_slice[ii - len_tmp];
        }
        file_slice = len_text;				//将“文件名长度@”添加到正文前面
        isfirstread = false;
    }
    else
    {
        res = fread(file_slice, 1, MAX_PACKET_DATA_BYTE_LENGTH, input_file);
    }
    return true;
}

bool Client::set_up_connection()
{
    return true;
}

bool Client::send_packet(int len)
{
    int res=sendto(sock, file_slice, len, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
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