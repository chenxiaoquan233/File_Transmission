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

int Client::read_path(char* path, char* buffer[])
{
    CString filenamelist;
    static int foldernumber = 0;
    CString cpath = path;
    char* temp = new char[1000];
    char* temp1 = new char[1000];
    char* temp2 = new char[1000];
    char* temp3 = new char[1000];
    memset(temp, 0, 1000);
    memset(temp1, 0, 1000);
    memset(temp2, 0, 1000);
    memset(temp3, 0, 1000);
    char* s1 = "d ";
    char* s2 = "\n";
    CFileFind finder;
    BOOL working = finder.FindFile(cpath + "\\*.*");
    while (working)
    {
        working = finder.FindNextFile();
        if (finder.IsDots())
            continue;
        if (finder.IsDirectory())
        {
            USES_CONVERSION;
            temp = T2A(finder.GetFilePath());
            int i = 0;
            int j = 0;
            int k = 0;
            for (i = strlen(temp)-1; i > 0; i--)
            {
                if (temp[i] == '\\')
                    break;
            }
            for (j = i; j < strlen(temp); j++)
            {
                temp2[k++] = temp[j];           //temp2形如“\s1”，即找出当前找到的子文件夹
            }
            temp1 = strcat(path, temp2);        //将原相对路径与之连接
            sprintf(temp3, "%s%s%s", s1, temp1, s2);        //标准化格式为“d floderpath\n”
            strcpy(buffer[foldernumber++], temp3);
            read_path(temp1, buffer);
        }
        else
        {
            CString filename = (finder.GetFileName());
            filenamelist = "d "+cpath+"\\"+filename+"\n";
            USES_CONVERSION;
            temp = T2A(filenamelist);
            //strcpy(buffer[foldernumber++], temp);     //去掉此条语句的注释可将文件路径加入数组
        }
    }
    return foldernumber;
}