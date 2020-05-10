#include "../../include/Client/Client.h"

Client::Client(const char* ip_addr)
{
    this->ip_addr = new char[20];
    strcpy(this->ip_addr, ip_addr);
}

Client::~Client()
{
    closesocket(this->cmd_sock);
}

int Client::send_cmd(const char* cmd)
{
    int res = -1;
    cout<<cmd<<endl;
    res = send(cmd_sock, cmd, strlen(cmd), 0);
    return res;
}

int Client::recv_cmd(char* buf, int len, int usec)
{
    int res = recv(cmd_sock, buf, len, 0);
    return res;
}

bool Client::send_file(const char* input_file_name, int path_offs)
{
	if (file) file->~File();
    file = new File(input_file_name, MAX_PACKET_DATA_BYTE_LENGTH, 0);
    char* cmd = nullptr;
	cmd = new char[MAX_PACKET_DATA_BYTE_LENGTH];
    while (cmd == nullptr)
    {
		cmd = new char[MAX_PACKET_DATA_BYTE_LENGTH];
    }
    sprintf(cmd, "SEND %s %d %lld", input_file_name + path_offs, file->get_tot_num(), file->get_file_len());
    send_cmd(cmd);
    int offs = -1;
    offs = get_offset();
    if(offs == -1) return false;

    file->get_send_rec();

    while (!file->eof())
    {
        read_file_slice(input_file_name + path_offs);
        int slice_len = data->get_slice_len();
        if(!send_packet(slice_len)) return false;
        file->pkt_send(data->get_slice_num() - 1);
    }
	delete []cmd;
    return true;
}

bool Client::read_file_slice(const char* input_file_name)
{
    if(data) delete data;//data equals nullptr the first time, but not after
    data = new pkt_load();
	cout << "a slice" << endl;
    if(!data->create_file_slice(MAX_PACKET_DATA_BYTE_LENGTH))
        return false;

    int res;
    int slice_num = file->get_pkt_num();
    data->set_slice_num(slice_num);
    
    int header_len = strlen(input_file_name) + 5;
    sprintf(data->get_file_slice(), "%s@%03d&", input_file_name,  slice_num);
    
    //continue from previous point
    FILE* file_ptr = file->get_file();
    //fpos_t file_offset = file->get_base_offset() + static_cast<long long>((slice_num - 1)) * file->get_slice_len();
    fpos_t file_offset = file->get_base_offset() + static_cast<long long>((slice_num - 1)) * (MAX_PACKET_DATA_BYTE_LENGTH - header_len);
	//if (file_offset != 0) file_offset += header_len;
    //fseek(file_ptr, file_offset, SEEK_SET);
	fsetpos(file_ptr, &file_offset);
	cout << file_offset << endl;

    int header_offset = file->get_offset();
    
    //res = fread(data->get_file_slice() + header_offset, 1, MAX_PACKET_DATA_BYTE_LENGTH - header_offset, file_ptr);
    res = fread(data->get_file_slice() + header_len, 1, MAX_PACKET_DATA_BYTE_LENGTH - header_len, file_ptr);
    data->set_slice_len(res + header_len);
	cout << res << endl;
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
    *sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#endif

#ifdef __linux__
    *sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
    long long already_send = 0;
    while(already_send < len)
    {
        int send_time = 0;
        int send_len;

            send_len = getmin(len - already_send, MAX_UDP_PACKET_LEN);
            int res = send(cmd_sock, data->get_file_slice() + already_send, send_len, 0);
            if (res == -1)
            {
                perror("send");
                exit(0);
            }

			if (!get_ack()) return false;

        already_send += send_len;
    }
    return true;
}

int Client::get_offset()
{
    char buffer[1200];

    int recv_len = -1;
	recv_len = recv_cmd(buffer, 1200, 5000);

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
        memset(need_rec, 0, tot_num * sizeof(bool));

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
            need_rec[tmp] = true;
        }
        for(int i = 0; i < file->get_tot_num(); ++i)
            if(!need_rec[i])
                file->pkt_send(i);

		delete[]need_rec;
    }

    return value;
}

int Client::get_port()
{

	char buffer[12];
	memset(buffer, 0, 12 * sizeof(char));
	recv_cmd(buffer, 12, 5000);

    if (buffer[0] != 'P' || buffer[1] != 'O' || buffer[2] != 'R' || buffer[3] != 'T')return -1;
    int value = 0;
    int tot = 4;
    while (tot < 12 && !isdigit(buffer[tot]))++tot;
    while (tot < 12 && isdigit(buffer[tot])) { value *= 10; value += buffer[tot] - '0'; ++tot; }
    return value;
}

bool Client::get_ack()
{
    char num_buffer[4];
    memset(num_buffer, 0, sizeof(num_buffer));

	if (recv_cmd(num_buffer, 4, 10000))
		return atoi(num_buffer) == data->get_slice_num();
	else return false;
   
}

int Client::read_path(const char* path, char* path_info_buf, char** file_info_buf, int& file_num, const int abs_path_offs)
{
    #ifdef WIN32
    intptr_t handle;
    _finddata_t findData;
    char* dir = new char[2000];
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
            sprintf(path_info_buf, "%s%s", path_info_buf, standardization.c_str() + abs_path_offs);
            read_path(subdir.c_str(), path_info_buf, file_info_buf, file_num, abs_path_offs);
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
	delete[]dir;
    return file_num;
}

bool Client::send_path_info(char* buffer)
{
    delete[]buffer;
    buffer = new char[3];
    buffer[0] = 't'; buffer[1] = '\0';
    if(strlen(buffer))
    {
        int p = strlen(buffer);
        char info[6] = "INFO";
        send_cmd(info);
        int port = -1;
        port = get_port();
        if(port == -1) return false;

        if (p > MAX_PACKET_DATA_BYTE_LENGTH)
        {
            for (int i = 0; i < p / MAX_PACKET_DATA_BYTE_LENGTH + 1; i++)
            {
                char* paths = new char[MAX_PACKET_DATA_BYTE_LENGTH];
                for (int j = 0; j < MAX_PACKET_DATA_BYTE_LENGTH; j++)
                {
                    paths[j] = buffer[i * MAX_PACKET_DATA_BYTE_LENGTH + j];
                }
                    int res = send(cmd_sock, paths, strlen(paths) + UPD_HEADER_LENGTH, 0);
                    delete(paths);
                    if (res != -1)
                    {
                        return true;
                    }
                std::cout << "Transmission failed, retransmission limit reached" << std::endl;
                return false;
            }
        }
        else
        {

                int res = send(cmd_sock, buffer, strlen(buffer), 0);
                char ret[64];
                memset(ret, 0, sizeof(ret));

                #ifdef __linux__
                socklen_t nSize = sizeof(sockaddr);
                #endif
                #ifdef WIN32
                int nSize = sizeof(sockaddr);
                #endif
                //recvfrom(cmd_sock, ret, 64, 0, (struct sockaddr*) & serv_addr_cmd, &nSize);
				recv_cmd(ret, 64, 5000);
                delete(buffer);
                return !strcmp(ret, "INFO");
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

bool Client::set_data_port()
{
	char* cmd = nullptr;
	cmd = new char[MAX_PACKET_DATA_BYTE_LENGTH];
	while (cmd == nullptr)
	{
		cmd = new char[MAX_PACKET_DATA_BYTE_LENGTH];
	}

	sprintf(cmd, "PORT");
	send_cmd(cmd);
	int port = -1;
	port = get_port();
	if (port == -1) return false;

	delete[]cmd;
	return true;
}
bool Client::tcp_connection()
{
    return connect(cmd_sock, (struct sockaddr*) & serv_addr_cmd, sizeof(serv_addr_cmd)) != -1;
}