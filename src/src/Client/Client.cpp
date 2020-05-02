#include "../../include/Client/Client.h"

Client::Client(const char* ip_addr)
{
    this->ip_addr = new char[20];
    strcpy(this->ip_addr, ip_addr);
}

Client::~Client()
{

}

int Client::send_cmd(const char* cmd)
{
    int res = -1;
    cout<<cmd<<endl;
    res = sendto(cmd_sock, cmd, strlen(cmd), 0, (struct sockaddr*)(&serv_addr_cmd), sizeof(serv_addr_cmd));
    return res;
}

int Client::recv_cmd(char* buf, int len, int usec)
{
    #ifdef __linux__
    socklen_t nSize = sizeof(sockaddr);
    #endif
    #ifdef WIN32
    int nSize = sizeof(sockaddr);
    #endif

    struct timeval timeout;
    timeout.tv_sec = usec / 1000;
    timeout.tv_usec = usec % 1000;
    if (setsockopt(cmd_sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) == -1) {
        perror("setsockopt failed:");
    }

    int res = recvfrom(cmd_sock, buf, len, 0, (struct sockaddr*) & serv_addr_cmd, &nSize);
    return res;
}

bool Client::send_file(const char* input_file_name, int path_offs)
{
    file = new File(input_file_name, MAX_PACKET_DATA_BYTE_LENGTH, 0);
    char* cmd;
    if ((cmd = (char*)malloc(sizeof(char) * MAX_PACKET_DATA_BYTE_LENGTH)) == nullptr)
    {
        return false;
    }

    sprintf(cmd, "SEND %s %d %d", input_file_name + path_offs, file->get_tot_num(), file->get_file_len());
    send_cmd(cmd);
    int offs = -1;
    offs = get_offset();
    if(offs == -1) return false;

    sprintf(cmd, "PORT");
    send_cmd(cmd);
    int port = -1;
    port = get_port();
    if(port == -1) return false;

    #ifdef _WIN32
    data_sock = new SOCKET;
    #endif
    #ifdef __linux__
    data_sock = new int;
    #endif
    sock_init(data_sock, port, 0);

    file->get_send_rec();

    while (!file->eof())
    {
        read_file_slice(input_file_name + path_offs);
        int slice_len = data->get_slice_len();
        if(!send_packet(slice_len)) return false;
        file->pkt_send(data->get_slice_num() - 1);
    }

    return true;
}

bool Client::read_file_slice(const char* input_file_name)
{
    if(data) delete data;//data equals nullptr the first time, but not after
    data = new pkt_load();

    if(!data->create_file_slice(MAX_PACKET_DATA_BYTE_LENGTH))
        return false;

    int res;
    int slice_num = file->get_pkt_num();
    data->set_slice_num(slice_num);
    
    int header_len = strlen(input_file_name) + 5;
    sprintf(data->get_file_slice(), "%s@%03d&", input_file_name,  slice_num);
    
    //continue from previous point
    FILE* file_ptr = file->get_file();
    int file_offset = file->get_base_offset() + (slice_num - 1) * file->get_slice_len();
    fseek(file_ptr, file_offset, SEEK_SET);

    int header_offset = file->get_offset();
    res = fread(data->get_file_slice() + header_offset, 1, MAX_PACKET_DATA_BYTE_LENGTH - header_offset, file_ptr);
    data->set_slice_len(res + header_len);
    return res;
}

#ifdef _WIN32
bool Client::sock_init(SOCKET* sock, int port, int is_cmd)
#endif
#ifdef __linux__
bool Client::sock_init(int* sock, int port, int is_cmd)
#endif
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    *sock = socket(AF_INET, SOCK_DGRAM, 0);
#endif

#ifdef __linux__
    *sock = socket(AF_INET, SOCK_DGRAM, 0);
#endif

    if (is_cmd)
    {
        memset(&serv_addr_cmd, 0, sizeof(serv_addr_cmd));
        serv_addr_cmd.sin_family = AF_INET;
        serv_addr_cmd.sin_addr.s_addr = inet_addr(ip_addr);
        serv_addr_cmd.sin_port = htons(port);
    }
    else
    {
        memset(&serv_addr_data, 0, sizeof(serv_addr_data));
        serv_addr_data.sin_family = AF_INET;
        serv_addr_data.sin_addr.s_addr = inet_addr(ip_addr);
        serv_addr_data.sin_port = htons(port);
    }

    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
#ifdef _WIN32
    if (setsockopt(*sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) == -1)
#endif
#ifdef __linux__
    if (setsockopt(*sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
#endif
    {
        perror("setsockopt timeout failed:");
        return false;
    }

    return set_port(sock, port, is_cmd);
}

#ifdef _WIN32
bool Client::set_port(SOCKET* sock, int port,int  is_cmd)
#endif
#ifdef __linux__
bool Client::set_port(int* sock, int port, int is_cmd)
#endif
{
    if (is_cmd)
    {
        serv_addr_cmd.sin_port = htons(port);
    }
    else
    {
        serv_addr_data.sin_port = htons(port);
    }
    return true;
}
bool Client::send_packet(int len)
{
    int on=1;
#ifdef _WIN32
    if(setsockopt(*data_sock, SOL_SOCKET, SO_BROADCAST, (const char*)&on, sizeof(on)) < 0)
#endif
#ifdef __linux__
    if(setsockopt(*data_sock, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) < 0)
#endif
    {
        perror("broadcast");
        exit(0);
    }
    int already_send = 0;
    while(already_send < len)
    {
        int send_len = getmin(len - already_send, MAX_UDP_PACKET_LEN);
        int res=sendto(*data_sock, data->get_file_slice() + already_send, send_len, 0, (struct sockaddr*)&serv_addr_data, sizeof(serv_addr_data));
        if(res == -1)
        {
            perror("send");
            exit(0);
        }
        already_send += send_len;
        if(!get_ack()) return false;
    }
    return true;
}

int Client::get_offset()
{
    char buffer[3000];
    #ifdef __linux__
    socklen_t nSize = sizeof(sockaddr);
    #endif
    #ifdef WIN32
    int nSize = sizeof(sockaddr);
    #endif

    int recv_len = recvfrom(cmd_sock, buffer, 12, 0, (struct sockaddr*) & serv_addr_cmd, &nSize);
    if(recv_len == -1) return -1;

    if (buffer[0] != 'O' || buffer[1] != 'F' || buffer[2] != 'F' || buffer[3] != 'S') return -1;
    int value = 0;
    int tot = strlen("OFFS ");

    // get total packet sum
    while(tot < recv_len && buffer[tot] != ' ')
    {
        value *= 10;
        value += buffer[tot] - '0';
        ++tot; 
    }

    // some packet need redo
    int tot_num = file->get_tot_num();
    if(value != tot_num)
    {
        bool* need_rec=new bool[tot_num];
        memset(need_rec, 0, sizeof(need_rec));

        while(tot < recv_len)
        {
            // skip space
            ++tot;

            // get packet number
            int tmp = 0;
            while(tot < recv_len && buffer[tot] != ' ')
            {
                tmp *= 10;
                tmp += buffer[tot] - '0';
                ++tot; 
            }
            need_rec[tmp - 1] = true;
        }
        for(int i = 0; i < file->get_tot_num(); ++i)
            if(!need_rec[tot_num])
                file->pkt_send(i);
    }

    return value;
}

int Client::get_port()
{
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
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

    if(recvfrom(cmd_sock, buffer, 12, 0, (struct sockaddr*) & serv_addr_cmd, &nSize) < 0)
        return -1;

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
    timeout.tv_sec = 15;
    timeout.tv_usec = 0;

    #ifdef __linux__
    if (setsockopt(cmd_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
        return false;
    #endif
    #ifdef WIN32
	if (setsockopt(cmd_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) == -1)
		return false;
    #endif

    char num_buffer[4];
    memset(num_buffer, 0, sizeof(num_buffer));

    #ifdef __linux__
    socklen_t nSize = sizeof(sockaddr);
    #endif
    #ifdef WIN32
	int nSize = sizeof(sockaddr);
    #endif
    if(recvfrom(cmd_sock, num_buffer, 4, 0, (struct sockaddr*)&serv_addr_cmd, &nSize) == -1)
        return false;

    return atoi(num_buffer) == data->get_slice_num();
}

int Client::read_path(const char* path, char* path_info_buf, char** file_info_buf, int& file_num)
{
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
            std::string subdir(path);
            subdir += ("/" + std::string(findData.name));
            std::string standardization(subdir);
            standardization.insert(0, "d ");
            standardization += "\n";
            sprintf(path_info_buf, "%s%s", path_info_buf, standardization.c_str());
            read_path(subdir.c_str(), path_info_buf, file_info_buf, file_num);
        }
        else if (strcmp(findData.name, ".") != 0 && strcmp(findData.name, "..") != 0)
        {
            
            std::string filepath(path);
            filepath+= ("/" + std::string(findData.name));
            std::string standardization(filepath);
            strcpy(file_info_buf[file_num], standardization.c_str());
            file_num++;
        }
    } while (_findnext(handle, &findData) == 0);
    _findclose(handle);
    #endif

    #ifdef __linux__
    struct dirent* ent = nullptr;
    DIR* pDir;
    pDir = opendir(path);
    while (nullptr != (ent = readdir(pDir)))
    {
        if (1 || ent->d_reclen == 24)
        {
            if (ent->d_type == DT_REG)
            {
                //for file
                std::string filepath(path);
                filepath += ("/" + std::string(ent->d_name));
                std::string standardization(filepath);
                strcpy(file_info_buf[file_num], standardization.c_str());
                file_num++;
            }
            else
            {
                //for floder
                if(strcmp(ent->d_name, ".") && strcmp(ent->d_name, ".."))
                {
                    std::string subdir(path);
                    subdir += ("/" + std::string(ent->d_name));
                    std::string standardization(subdir);
                    standardization += "/\n";
                    sprintf(path_info_buf, "%s%s", path_info_buf, standardization.c_str());
                    read_path(subdir.c_str(), path_info_buf, file_info_buf, file_num);
                }
            }
        }
    }
    #endif

    return file_num;
}

bool Client::send_path_info(char* buffer)
{
    if(strlen(buffer))
    {
        char info[6] = "INFO";
        send_cmd(info);
        int port = -1;
        port = get_port();
        if(port == -1) return false;

        #ifdef _WIN32
        data_sock = new SOCKET;
        #endif
        #ifdef __linux__
        data_sock = new int;
        #endif
        sock_init(data_sock, port, 0);

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
                    int res = sendto(*data_sock, paths, strlen(paths) + UPD_HEADER_LENGTH, 0, (struct sockaddr*) & serv_addr_cmd, sizeof(serv_addr_cmd));
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
                int res = sendto(*data_sock, buffer, strlen(buffer), 0, (struct sockaddr*) & serv_addr_data, sizeof(serv_addr_data));
                char ret[64];
                memset(ret, 0, sizeof(ret));

                #ifdef __linux__
                socklen_t nSize = sizeof(sockaddr);
                #endif
                #ifdef WIN32
                int nSize = sizeof(sockaddr);
                #endif
                recvfrom(cmd_sock, ret, 64, 0, (struct sockaddr*) & serv_addr_cmd, &nSize);
                return !strcmp(ret, "INFO");
            }
            std::cout << "Transmission failed, retransmission limit reached" << std::endl;
            return false;
        }
    }
    else
    {
        return true;
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

/*bool init_connect(Client*& client, const char* ip_addr, int port)
{
    client = new Client(ip_addr);
    client->sock_init(client->get_cmd_sock(), port, 1);
    client->send_cmd("ON");
    char buf[5];
    memset(buf, 0, 5 * sizeof(char));
    if(!client->recv_cmd(buf, 1, 500))
        return false;
    return !strcmp(buf, "1");
}

bool start_send(Client*& client, const char* file_path, bool dir_flag)
{
    if(dir_flag)
    {
        char* file_info[100];
        for (int i = 0; i < 100; i++)
        {
            file_info[i] = new char[1000];
            memset(file_info[i], 0, 1000);
        }
        char* path_info = new char[10000];
        memset(path_info, 0, 10000 * sizeof(char));
        int file_number = 0;
        client->read_path(file_path, path_info, file_info, file_number);
        cout<<file_number<<endl;
        if(client->send_path_info(path_info))
        {
            int len = strlen(file_path) + 1;
            for(int i = 0; i < file_number; ++i)
            {
                if(!client->send_file(file_info[i], len))
                    return false;
            }
        }
        else return false;
    }
    else
    {
        if(!client->send_file(file_path, 0))
            return false;
    }
    return true;
}*/
