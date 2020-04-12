#include "../../include/Client/Client.h"

Client::Client()
{

}

Client::~Client()
{

}

void Client::send_cmd(char* cmd)
{
    puts(cmd);
    int res = sendto(cmd_sock, cmd, strlen(cmd), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
}


int Client::send_file(char* input_file_name)
{
    file = new File(input_file_name, MAX_PACKET_DATA_BYTE_LENGTH, 0);
    char* cmd;
    if ((cmd = (char*)malloc(sizeof(char) * MAX_PACKET_DATA_BYTE_LENGTH)) == NULL)
    {
        return -1;
    }
    while (!file->eof())
    {
        read_file_slice(input_file_name);
        int slice_len = data->get_slice_len();
        
        sprintf(cmd, "SEND %s %d %d", input_file_name, file->get_pkt_num(), file->get_file_len());
        send_cmd(cmd);
        int offs = -1;
        while (offs == -1)offs = get_offset();

        sprintf(cmd, "PORT");
        send_cmd(cmd);
        int port = -1;
        while (port == -1)port = get_port();

        set_port(&cmd_sock, port);

        bool res = send_packet(slice_len);
    }
    /*
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
    */
}

bool Client::read_file_slice(char* input_file_name)
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
    fseek(file_ptr, file_offset, SEEK_SET);

    int header_offset = file->get_offset();
    res = fread(data->get_file_slice() + header_offset, 1, MAX_PACKET_DATA_BYTE_LENGTH - header_offset, file_ptr);
    data->set_slice_len(res);
    return res;
}

#ifdef _WIN32
bool Client::sock_init(SOCKET* sock, const char* ip_addr, int port)
#endif
#ifdef __linux__
bool Client::sock_init(int* sock, const char* ip_addr, int port)
#endif
{
    #ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    cmd_sock = socket(AF_INET, SOCK_DGRAM, 0);
    #endif

    #ifdef __linux__
    *sock = socket(AF_INET, SOCK_DGRAM, 0);
    #endif

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip_addr);
    serv_addr.sin_port = htons(port);
    return set_port(sock, port);
}

#ifdef _WIN32
bool Client::set_port(SOCKET* sock, int port)
#endif
#ifdef __linux__
bool Client::set_port(int* sock, int port)
#endif
{
    serv_addr.sin_port = htons(port);
    if (bind(*sock, (struct sockaddr*) & serv_addr, sizeof(sockaddr)) == -1)
        return false;
    return true;
}

bool Client::send_packet(int len)
{
    int res=sendto(data_sock, data->get_file_slice(), len + UPD_HEADER_LENGTH, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    return res!=-1;
}


int Client::get_offset()
{
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 200;
#ifdef __linux__
    if (setsockopt(cmd_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
        return -1;
#endif
#ifdef WIN32
    if (setsockopt(cmd_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) == -1)
        return -1;
#endif
    char buffer[12];
#ifdef __linux__
    socklen_t nSize = sizeof(sockaddr);
#endif
#ifdef WIN32
    int nSize = sizeof(sockaddr);
#endif
    recvfrom(cmd_sock, buffer, 12, 0, (struct sockaddr*) & serv_addr, &nSize);

    if (buffer[0] != 'O' || buffer[1] != 'F' || buffer[2] != 'F' || buffer[3] != 'S')return -1;
    int value = 0;
    int tot = 4;
    while (tot < 12&&!isdigit(buffer[tot]))++tot;
    while (tot < 12&&isdigit(buffer[tot])) { value *= 10; value += buffer[tot] - '0'; ++tot; }
    return value;
}

int Client::get_port()
{
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 200;
#ifdef __linux__
    if (setsockopt(cmd_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
        return -1;
#endif
#ifdef WIN32
    if (setsockopt(cmd_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) == -1)
        return -1;
#endif
    char buffer[12];
#ifdef __linux__
    socklen_t nSize = sizeof(sockaddr);
#endif
#ifdef WIN32
    int nSize = sizeof(sockaddr);
#endif
    recvfrom(cmd_sock, buffer, 12, 0, (struct sockaddr*) & serv_addr, &nSize);

    if (buffer[0] != 'P' || buffer[1] != 'O' || buffer[2] != 'R' || buffer[3] != 'T')return -1;
    int value = 0;
    int tot = 4;
    while (tot < 12 && !isdigit(buffer[tot]))++tot;
    while (tot < 12 && isdigit(buffer[tot])) { value *= 10; value += buffer[tot] - '0'; ++tot; }
    return value;
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
    #ifdef WIN32
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
    #endif

#ifdef __linux__
    struct dirent* ent = NULL;
    DIR* pDir;
    pDir = opendir(path);
    while (NULL != (ent = readdir(pDir)))
    {
        if (ent->d_reclen == 24)
        {
            if (ent->d_type == 8)
            {
                //for file,remove the comment here to add the file path to the array
                /*
                foldernumber++;
                std::string filepath(path);
                filepath += ("/" + std::string(ent->d_name));
                std::string standardization(filepath);
                standardization.insert(0, "d ");
                standardization += "\n";
                strcpy(buffer[foldernumber], standardization.c_str());
                */
            }
            else
            {
                //for floder
                foldernumber++;
                std::string subdir(path);
                subdir += ("/" + std::string(ent->d_name));
                std::string standardization(subdir);
                standardization.insert(0, "d ");
                standardization += "\n";
                strcpy(buffer[foldernumber], standardization.c_str());
                read_path(subdir.c_str(), buffer);
            }
        }
    }
#endif
    return foldernumber;
}

bool Client::send_path_info(char* buffer)
{
    char* info = "INFO";
    send_cmd(info);
    int port = -1;
    while (port == -1)port = get_port();
    set_port(&cmd_sock, port);
    if (strlen(buffer) > MAX_PACKET_DATA_BYTE_LENGTH)
    {
        for (int i = 0; i < strlen(buffer) / MAX_PACKET_DATA_BYTE_LENGTH + 1; i++)
        {
            char* paths = new char[MAX_PACKET_DATA_BYTE_LENGTH];
            for (int j = 0; j < MAX_PACKET_DATA_BYTE_LENGTH; j++)
            {
                paths[j] = buffer[i * MAX_PACKET_DATA_BYTE_LENGTH + j];
            }
            for(int k=0;k< SEND_FREQ;k++)
            {
                int res = sendto(cmd_sock, paths, strlen(paths) + UPD_HEADER_LENGTH, 0, (struct sockaddr*) & serv_addr, sizeof(serv_addr));
                //UNCOMPLETED!!!Need to determine whether the return is "info"
                if (res != -1)
                {
                    return true;
                }
            }
            std::cout << "Transmission failed, retransmission limit reached" << std::endl;
            return false;
        }
    }
    else
    {
        for (int k = 0; k < SEND_FREQ; k++)
        {
            int res = sendto(cmd_sock, buffer, strlen(buffer) + UPD_HEADER_LENGTH, 0, (struct sockaddr*) & serv_addr, sizeof(serv_addr));
            //UNCOMPLETED!!!Need to determine whether the return is "info"
            if (res != -1)
            {
                return true;
            }
        }
        std::cout << "Transmission failed, retransmission limit reached" << std::endl;
        return false;
    }

}

#ifdef _WIN32
SOCKET* Client::get_cmd_sock()
#endif
#ifdef __linux__
int* Client::get_cmd_sock()
#endif
{
    return &cmd_sock;
}