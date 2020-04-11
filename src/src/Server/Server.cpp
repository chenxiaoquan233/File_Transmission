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
#ifdef __linux__
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
#endif
#ifdef WIN32
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on)) == -1)
#endif
		{
			return false;
		}
    if(bind(sock, (struct sockaddr*)&serv_addr, sizeof(sockaddr)) == -1)
	{
		return false;
	}
    return true;
	//printf("%d\n",check_port(8079));
}

int Server::check_port(int cmd_port) {
	static int start_port = cmd_port > 1024 ? cmd_port + 1 : 1024;
#ifdef _WIN32
	data_sock = socket(AF_INET, SOCK_DGRAM, 0);
#endif
#ifdef __linux__
	sock = socket(AF_INET, SOCK_DGRAM, 0);
#endif
	memset(&serv_addr2, 0, sizeof(serv_addr2));
	serv_addr2.sin_family = AF_INET;
	serv_addr2.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr2.sin_port = htons(start_port);
#ifdef _WIN32
	bind(data_sock, (LPSOCKADDR)&serv_addr2, sizeof(serv_addr2));
	while (WSAGetLastError() == WSAEADDRINUSE)
	{
		//端口已被占用
		//printf("占用");
		start_port++;
		memset(&serv_addr2, 0, sizeof(serv_addr2));
		serv_addr2.sin_family = AF_INET;
		serv_addr2.sin_addr.s_addr = htonl(INADDR_ANY);
		serv_addr2.sin_port = htons(start_port);
		bind(data_sock, (LPSOCKADDR)&serv_addr2, sizeof(serv_addr2));
	}
#endif
#ifdef __linux__
	while (bind(sock, (struct sockaddr*) & serv_addr2, sizeof(sockaddr)) == -1)
	{
		start_port++;
		memset(&serv_addr2, 0, sizeof(serv_addr2));
		serv_addr2.sin_family = AF_INET;
		serv_addr2.sin_addr.s_addr = htonl(INADDR_ANY);
		serv_addr2.sin_port = htons(start_port);
	}
#endif
	char answer_port[10];
	sprintf(answer_port, "PORT %d", start_port);
	sendto(sock, answer_port, strlen(answer_port), 0, (struct sockaddr*) & serv_addr, sizeof(serv_addr));
	return start_port;
}

bool Server::recv_packet()
{
    if(data) delete data;//data equals nullptr the first time, but not after
    data = new pkt_load();
#ifdef __linux__
    socklen_t nSize = sizeof(sockaddr);
#endif
#ifdef WIN32
	int nSize = sizeof(sockaddr);
#endif
    data->create_file_slice(MAX_PACKET_DATA_BYTE_LENGTH);
    int res = recvfrom(sock, data->get_file_slice(), MAX_PACKET_DATA_BYTE_LENGTH, 0, (struct sockaddr*)&serv_addr, &nSize);
	data->set_slice_len(res);
    return res != -1;
}

bool Server::recv_whole_file()
{
	//if(output_file) output_file = nullptr;//clear the output_file
	if(buffer) delete(buffer);//clear the buffer

	int file_length = 0; // count the received file length
	int total_length; // the whole length of the received file

	while(1)
	{
		int slice_num = 0;
		char file_path[50];
		char* file_slice = new char[MAX_PACKET_DATA_BYTE_LENGTH];
		int data_len = 0;

		if (recv_packet())
		{
			parse_param(data->get_file_slice(), file_path, &slice_num, file_slice, data->get_slice_len(), &data_len);
			send_ack(slice_num);
			char file_name[100];
			printf("%s,%d,%d\n", file_path, slice_num, data->get_slice_len());
			write_logfile(file_path,slice_num);
			sprintf(file_name, "%s.%03d", file_path, slice_num);
			FILE* output_file_slice = fopen(file_name, "wb");
			fwrite(file_slice, 1, data_len, output_file_slice);
			fclose(output_file_slice);
			//write_file(output_file_slice, file_slice, data_len);
		}

		delete file_slice;
	}

	/*while (1)
	{
		while (1)//receive the next file slice
			if (recv_packet()) break;
		*/


		//int pkt_num = return_packet_serial_number();
		//send_ack(pkt_num);//send the ack to client
		
		/*update information of file and create new buffer*/
		/*if (!output_file)//head of file
		{
			read_FILEinformation(output_file, data->get_file_slice(), total_length);
			buffer = new char(total_length + 1);
		}*/

		/*read this slice into buffer and update file_length*/
		//file_length += get_file_data(data->get_file_slice(), buffer + file_length, total_length - file_length);


		/*already received the whole file*/
		/*if (file_length == total_length)
		{
			break;
		}
	}*/
	//puts(buffer);
	//write_file(output_file, buffer, total_length);
	return true;
}

int Server::get_file_data(char* src, char* dest, int maxlen)
{
	int src_pos = 0,dst_pos = 0, tot_len = 0;

	while(src[src_pos] != '?') ++src_pos;//find ?
	++ src_pos;

	sscanf(src + src_pos, "%d", &tot_len);

	while (maxlen && src[src_pos] != '\0')
	{
		--maxlen;
		dest[dst_pos++] = src[src_pos];
		++src_pos;
	}
	return tot_len;
}

bool Server::send_ack(int num)
{
	int packet_serial_number = num;//get packet serial number
	char ack[4];
	printf("packet %d get\n", num);
	sprintf(ack, "%d", packet_serial_number);
	int res = sendto(sock, ack, strlen(ack), 0, (struct sockaddr*) & serv_addr, sizeof(serv_addr));
	return res != -1;

}

int Server::read_FILEinformation(FILE*& output_file, char* origin_data, int& data_length)
{
    int info_length = 0;
    char FILE_path[50];
    data_length = 0;
	int tot = 0;
    while (origin_data[info_length] != '?') {
        if(origin_data[info_length] != '$')
		FILE_path[tot++] = origin_data[info_length];
        info_length++;
    }
	FILE_path[tot] = '\0';
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

bool Server::write_file(FILE* output_file,char* data,int data_length)
{
	for (int data_i = 0; data_i < data_length; data_i++)
		fputc(data[data_i],output_file);
	return true;
}

int Server::return_packet_serial_number()
{
	int position = 0, packet_serial_number = 0, position_first_file, position_first_file_flag = 0;

	while (data->get_file_slice()[position] != '?')
		position++;

	
	position_first_file = position;
	while (data->get_file_slice()[position_first_file] != '&')
	{
		if (data->get_file_slice()[position_first_file] == '@')
		{
			position_first_file_flag = 1;
			break;
		}
		position_first_file++;
	}
	if (position_first_file_flag == 1)
	{
		position_first_file++;
		while (data->get_file_slice()[position_first_file] != '&')
		{
			packet_serial_number *= 10;
			packet_serial_number += (data->get_file_slice()[position_first_file] - '0');
			position_first_file++;
		}
		return packet_serial_number;

	}
	else
	{
		position++;
		while (data->get_file_slice()[position] != '&')
		{
			packet_serial_number *= 10;
			packet_serial_number += (data->get_file_slice()[position] - '0');
			position++;
		}
		return packet_serial_number;
	}
}

void Server::parse_param(char* src, char* path, int* slice_num, char* data, int len, int* data_len)
{
	int si = 0, di = 0;

	//get file_name
	while(src[si] != '@') 
		path[di++] = src[si++];

	si++;//skip '@'

	//get slice_num
	while(src[si] != '&')
	{
		*slice_num *= 10;
		*slice_num += src[si++] - '0';
	}

	si++;//skip '&'

	*data_len = len - si;
	for(di = 0; di < *data_len; ++di)
		data[di] = src[si + di];
}
int Server::set_dir(char* path)
{
	int len = strlen(path);
	char* tmpDirPath = new char[100];
	memset(tmpDirPath, 0, 100);
#ifdef _WIN32
	for (int i = 0; i < len; i++)
	{
		tmpDirPath[i] = path[i];
		if (tmpDirPath[i] == '\\' || tmpDirPath[i] == '/')
		{
			if (_access(tmpDirPath, 0) == -1)
			{
				int ret = _mkdir(tmpDirPath);
				if (ret == -1) return ret;//Create failure
			}
		}
	}
#endif
#ifdef __linux__
	for (int i = 0; i < len; i++)
	{
		tmpDirPath[i] = path[i];
		if (tmpDirPath[i] == '\\' || tmpDirPath[i] == '/')
		{
			if (access(tmpDirPath, 0) == -1)
			{
				int ret = mkdir(tmpDirPath, 0777);
				if (ret == -1) return ret;//Create failure
			}
		}
	}
#endif
	return 0;//Create success
}
bool Server::parse_path()
{
	char* address = new char[256];
	char* r_cmd = new char[4];
	memset(r_cmd, 0, 4);
	memset(address, 0, 256);
#ifdef __linux__
	socklen_t nSize = sizeof(sockaddr);
#endif
#ifdef WIN32
	int nSize = sizeof(sockaddr);
#endif
	int res = recvfrom(sock, address, sizeof(address), 0, (struct sockaddr*) & serv_addr, &nSize);
	sprintf(r_cmd, "%s", "INFO");
	res = sendto(sock, r_cmd, 4, 0, (struct sockaddr*) & serv_addr, sizeof(serv_addr));
	set_dir(address);
	return res != -1;
}
void Server::parse_cmd()
{
	char* cmd = new char[256];
	memset(cmd, 0, 256);
#ifdef __linux__
	socklen_t nSize = sizeof(sockaddr);
#endif
#ifdef WIN32
	int nSize = sizeof(sockaddr);
#endif
	int res = recvfrom(sock, cmd, sizeof(cmd), 0, (struct sockaddr*) & serv_addr, &nSize);
	if (cmd[0] == 'I' && cmd[1] == 'N' && cmd[2] == 'F' && cmd[3] == 'O')
	{
		parse_path();
	}
	else if (cmd[0] == 'S' && cmd[1] == 'E' && cmd[2] == 'N' && cmd[3] == 'D')
	{
		char* file_name = new char[100];
		int i = 5;
		memset(file_name, 0, 100);
		while (cmd[i] != ' ' && i != strlen(cmd))
		{
			file_name[i - 5] = cmd[i];
			i++;
		}
		if (!check_file(file_name,file_length)) {
			write_logfile(file_name,total_packet_num);
			write_logfile(file_name, file_length);
		}
	}
	else if (cmd[0] == 'P' && cmd[1] == 'O' && cmd[2] == 'R' && cmd[3] == 'T' && cmd[4] == ' ')
	{
		int port = 0;
		for (int i = 5; i < strlen(cmd); i++)
		{
			port *= 10;
			port += (cmd[i] - '0');
		}
		check_port(port);
	}
	else
	{
		printf("Function parse_cmd() cmd error");
	}
}
bool Server::parse_arg(int argc, char** argv)
{
	if (argc != 4)
	{
		printf("wrong number of parameters\n");
		return false;
	}
	if ((access(argv[1], 0)) != -1)
	{
		if ((access(argv[1], 6)) == -1)
		{
			printf("File does not have read-write permission\n");
			return false;
		}
		return true;
	}
	else
	{
		printf("File does not exist\n");
		return false;
	}
	return false;
}

bool Server::write_logfile(char* path, int number) {
	char logfile_path[50];
	sprintf(logfile_path, "%s.FTlog", path);
	FILE* logfile = fopen(logfile_path, "ab");
	fwrite(&number, 4, 1, logfile);
	fclose(logfile);
	return true;
}

bool Server::check_file(char* file_name, int file_length) {
	char logfile_path[50];
	sprintf(logfile_path, "%s.FTlog", file_name);
	//FILE * file=fopen(file_name, "rb");
	FILE* logfile;
	logfile = fopen(logfile_path, "rb");
	//if(file)
	if (fopen(logfile_path, "rb")) {
		int* total_packet_num = new int;
		int* log_file_length = new int;
		fread(total_packet_num, 4, 1, logfile);
		fread(log_file_length, 4, 1, logfile);
		if (*log_file_length == file_length) {
			bool* loaded_pack_num = new bool[*total_packet_num + 1];
			for (int i = 1; i <= *total_packet_num; i++)loaded_pack_num[i] = false;
			int total_loaded_pack_num = 1;
			int* temp = new int;
			for (; !feof(logfile); total_loaded_pack_num++) {
				fread(temp, 4, 1, logfile);
				if (loaded_pack_num[*temp] == true)total_loaded_pack_num--;
				loaded_pack_num[*temp] = true;
			}
			total_loaded_pack_num--;
			void* need_send = malloc(sizeof(int) * (*total_packet_num - total_loaded_pack_num + 1));
			((int*)need_send)[0] = *total_packet_num - total_loaded_pack_num;
			for (int i = 1, j = 1; i <= *total_packet_num; i++) {
				if (loaded_pack_num[i] == false) {
					((int*)need_send)[j] = i;
					j++;
				}
			}
			//for(int i=0;i<sizeof(int)*(*total_packet_num-total_loaded_pack_num+1);i++)
			  //  cout<<(int)(((char *)need_send)[i])<<endl;
			sendto(sock, (char *)need_send, sizeof(int)*(*total_packet_num-total_loaded_pack_num+1), 0,(struct sockaddr*) & serv_addr, sizeof(serv_addr));
			fclose(logfile);
			return true;
		}
	}

	void* need_send = new int;
	((int*)need_send)[0] = 0;
	sendto(sock, (char*)need_send, 4, 0,(struct sockaddr*) & serv_addr, sizeof(serv_addr));
	fclose(logfile);
	remove(logfile_path);
	return false;
}