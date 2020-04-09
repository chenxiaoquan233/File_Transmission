#include "../../include/Client/Client.h"

Client::Client()
{

}

Client::~Client()
{

}

int Client::send_file(char* input_file_name)
{
    file = new File(input_file_name, MAX_PACKET_DATA_BYTE_LENGTH, 0);
    while (!file->eof())
    {
        read_file(input_file_name);
        int slice_len = data->get_slice_len();
        bool res = send_packet(slice_len);
        //while(!get_ack()) send_packet(slice_len());
        printf("%d\n", data->get_slice_num());
        file->pkt_send(data->get_slice_num() - 1);

    }
    return 0;
}

bool Client::read_file(char* input_file_name)
{
    if(data) delete data;//data equals nullptr the first time, but not after
    data = new pkt_load();

    if(!data->create_file_slice(MAX_PACKET_DATA_BYTE_LENGTH))
        return false;

    int res;
    int slice_num = file->get_pkt_num();
    data->set_slice_num(slice_num);
    
    sprintf(data->get_file_slice(), "%s@%03d&", input_file_name,  slice_num);
    
    //continue from previous point
    FILE* file_ptr = file->get_file();
    int file_offset = file->get_base_offset() + (slice_num - 1) * file->get_slice_len();
    printf("%d\n", file_offset);
    fseek(file_ptr, file_offset, SEEK_SET);

    int header_offset = file->get_offset();
    res = fread(data->get_file_slice() + header_offset, 1, MAX_PACKET_DATA_BYTE_LENGTH - header_offset, file_ptr);
    data->set_slice_len(res);
    return res;
}

bool Client::set_up_connection(const char* ip_addr, int port)
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    cmd_sock = socket(AF_INET, SOCK_DGRAM, 0);
#endif
#ifdef __linux__
    cmd_sock = socket(AF_INET, SOCK_DGRAM, 0);
#endif
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip_addr);
    serv_addr.sin_port = htons(port);
    if(bind(cmd_sock, (struct sockaddr*)&serv_addr, sizeof(sockaddr)) == -1)
        return false;
    return true;
}

bool Client::send_packet(int len)
{
    int res=sendto(cmd_sock, data->get_file_slice(), len + UPD_HEADER_LENGTH, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    return res!=-1;
}

bool Client::get_ack()
{
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 200;
#ifdef __linux__
    if (setsockopt(cmd_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
        return false;
#endif
#ifdef WIN32
	if (setsockopt(cmd_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) == -1)
		return false;
#endif
    char num_buffer[4];
#ifdef __linux__
    socklen_t nSize = sizeof(sockaddr);
#endif
#ifdef WIN32
	int nSize = sizeof(sockaddr);
#endif
    recvfrom(cmd_sock, num_buffer, 4, 0, (struct sockaddr*)&serv_addr, &nSize);

    return atoi(num_buffer) == data->get_slice_num();
}

int Client::read_path(const char* path, char* buffer[])
{
    static int foldernumber = 0;
    intptr_t handle;
    _finddata_t findData;
    char* dir = new char[1000];
    strcpy(dir, path);
    dir = strcat(dir, "/*.*");
    handle = _findfirst(dir, &findData);
    if (handle == -1)
    {
        return 0;
    }
    do
    {
        if (findData.attrib & _A_SUBDIR && strcmp(findData.name, ".") != 0 && strcmp(findData.name, "..") != 0)
        {
            //for floder
            foldernumber++;
            std::string subdir(path);
            subdir += ("/" + std::string(findData.name));
            std::string standardization(subdir);
            standardization.insert(0, "d ");
            standardization += "\n";
            strcpy(buffer[foldernumber], standardization.c_str());
            read_path(subdir.c_str(), buffer);
        }
        else if (strcmp(findData.name, ".") != 0 && strcmp(findData.name, "..") != 0)
        {
            //for file,remove the comment here to add the file path to the array
            /*
            foldernumber++;
            std::string filepath(path);
            filepath+= ("/" + std::string(findData.name));
            std::string standardization(filepath);
            standardization.insert(0, "d ");
            standardization += "\n";
            strcpy(buffer[foldernumber], standardization.c_str());
            */
        }
    } while (_findnext(handle, &findData) == 0);
    _findclose(handle);
    return foldernumber;
}