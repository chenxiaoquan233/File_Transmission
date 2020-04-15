#include "../../include/Server/Server.h"

Server::Server(int port)
{
	cmd_port = port;
}

Server::~Server()
{

}

bool Server::set_listen()
{
	#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    cmd_sock = socket(AF_INET, SOCK_DGRAM, 0);
	#endif
	#ifdef __linux__
    cmd_sock = socket(AF_INET, SOCK_DGRAM, 0);
	#endif
    memset(&serv_addr_cmd, 0, sizeof(serv_addr_cmd));
    serv_addr_cmd.sin_family = AF_INET;
    serv_addr_cmd.sin_addr.s_addr = INADDR_ANY;
    serv_addr_cmd.sin_port = htons(cmd_port);
    int on = 1;

	#ifdef WIN32
	if (setsockopt(cmd_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on)) == -1)
	#endif
	#ifdef __linux__
    if(setsockopt(cmd_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
	#endif
	{
		perror("reuse");
		return false;
	}
    if(bind(cmd_sock, (struct sockaddr*)&serv_addr_cmd, sizeof(sockaddr)) == -1)
	{
		perror("bind");
		return false;
	}
    return true;
}

int Server::check_port() 
{
	static int start_port = cmd_port > 1024 ? cmd_port + 1 : 1024;
	#ifdef _WIN32
	data_sock = socket(AF_INET, SOCK_DGRAM, 0);
	#endif
	#ifdef __linux__
	data_sock = socket(AF_INET, SOCK_DGRAM, 0);
	#endif
	memset(&serv_addr_data, 0, sizeof(serv_addr_data));
	serv_addr_data.sin_family = AF_INET;
	serv_addr_data.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr_data.sin_port = htons(start_port);
	#ifdef _WIN32
	bind(data_sock, (LPSOCKADDR)&serv_addr_data, sizeof(serv_addr_data));
	while (WSAGetLastError() == WSAEADDRINUSE)
	{
		//�˿��ѱ�ռ��
		//printf("ռ��");
		start_port++;
		memset(&serv_addr2, 0, sizeof(serv_addr2));
		serv_addr2.sin_family = AF_INET;
		serv_addr2.sin_addr.s_addr = htonl(INADDR_ANY);
		serv_addr2.sin_port = htons(start_port);
		bind(data_sock, (LPSOCKADDR)&serv_addr_data, sizeof(serv_addr_data));
	}
	#endif
	#ifdef __linux__
	while (bind(data_sock, (struct sockaddr*) & serv_addr_data, sizeof(sockaddr)) == -1)
	{
		start_port++;
		memset(&serv_addr_data, 0, sizeof(serv_addr_data));
		serv_addr_data.sin_family = AF_INET;
		serv_addr_data.sin_addr.s_addr = htonl(INADDR_ANY);
		serv_addr_data.sin_port = htons(start_port);
	}
	#endif
	char answer_port[10];
	sprintf(answer_port, "PORT %d", start_port);
	sendto(cmd_sock, answer_port, strlen(answer_port), 0, (struct sockaddr*) & serv_addr_cmd, sizeof(serv_addr_cmd));
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
    int res = recvfrom(data_sock, data->get_file_slice(), MAX_PACKET_DATA_BYTE_LENGTH, 0, (struct sockaddr*)&serv_addr_data, &nSize);
	data->set_slice_len(res);
    return res != -1;
}

bool Server::recv_whole_file()
{
	//if(output_file) output_file = nullptr;//clear the output_file
	if(buffer) delete(buffer);//clear the buffer

	int file_length = 0; // count the received file length
	int total_length; // the whole length of the received file

	file->get_send_rec();

	while(!(file->eof()))
	{
		int slice_num = 0;
		char file_path[50];
		char* file_slice = new char[MAX_PACKET_DATA_BYTE_LENGTH];
		int data_len = 0;

		if (recv_packet())
		{
			parse_param(data->get_file_slice(), file_path, &slice_num, file_slice, data->get_slice_len(), &data_len);
			send_ack(slice_num);
			file->pkt_send(slice_num - 1);
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

	puts("tot file transmission finished");

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
	char ack[4];
	printf("packet %d get\n", num);
	sprintf(ack, "%d", num);
	int res = sendto(cmd_sock, ack, strlen(ack), 0, (struct sockaddr*) & serv_addr_cmd, sizeof(serv_addr_cmd));
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
	int res = recvfrom(data_sock, address, sizeof(address), 0, (struct sockaddr*) & serv_addr_data, &nSize);
	sprintf(r_cmd, "%s", "INFO");
	res = sendto(cmd_sock, r_cmd, 4, 0, (struct sockaddr*) & serv_addr_cmd, sizeof(serv_addr_cmd));
	set_dir(address);
	return res != -1;
}

bool Server::parse_cmd()
{
	char* cmd = new char[256];
	memset(cmd, 0, 256);

	#ifdef __linux__
	socklen_t nSize = sizeof(sockaddr);
	#endif

	#ifdef WIN32
	int nSize = sizeof(sockaddr);
	#endif

	recvfrom(cmd_sock, cmd, 256 * sizeof(char), 0, (struct sockaddr*) & serv_addr_cmd, &nSize);

	if (cmd[0] == 'I' && cmd[1] == 'N' && cmd[2] == 'F' && cmd[3] == 'O')
	{
		parse_path();
	}
	else if (cmd[0] == 'S' && cmd[1] == 'E' && cmd[2] == 'N' && cmd[3] == 'D')
	{
		int i = 5;
		int file_len = 0;
		int tot_pkt_num = 0;
		char* file_name = new char[100];
		memset(file_name, 0, 100);

		// SEND filename pkt_num file_length
		while(i < strlen(cmd) && cmd[i] != ' ')
			file_name[i - 5] = cmd[i], i++;
		i++;
		while(i < strlen(cmd) && cmd[i] != ' ')
			tot_pkt_num *= 10, tot_pkt_num += cmd[i++] - '0';
		i++;
		while(i < strlen(cmd) && cmd[i] != ' ')
			file_len *= 10, file_len += cmd[i++] - '0';

		file = new File(file_name, tot_pkt_num);
		if(!check_file(file_name, file_len, tot_pkt_num)) 
		{
			write_logfile(file_name, tot_pkt_num);
			write_logfile(file_name, file_len);
		}
	}
	else if (cmd[0] == 'P' && cmd[1] == 'O' && cmd[2] == 'R' && cmd[3] == 'T')
	{
		check_port();
		recv_whole_file();
	}
	else
	{
		return false;
	}
	return true;
}


bool Server::write_logfile(char* path, int number) // to be finished
{
	/*char logfile_path[50];
	sprintf(logfile_path, "%s.FTlog", path);
	FILE* logfile = fopen(logfile_path, "ab");
	fwrite(&number, 4, 1, logfile);
	fclose(logfile);
	return true;*/
}

bool Server::check_file(char* file_name, int file_len, int pkt_num) 
{
	char logfile_path[50];
	sprintf(logfile_path, "%s.FTlog", file_name);
	FILE* logfile;
	logfile = fopen(logfile_path, "rb");
	if (logfile) 
	{
		int total_packet_num = 0;
		int log_file_length = 0;

		fread(&total_packet_num, 4, 1, logfile);
		fread(&log_file_length, 4, 1, logfile);
		
		if (log_file_length == file_len) 
		{
			bool* loaded_pack_num = new bool[total_packet_num];
			for (int i = 0; i < total_packet_num; i++)
				loaded_pack_num[i] = false;
				
			int total_loaded_pack_num = 0;
			int temp = 0;
			for (; !feof(logfile); total_loaded_pack_num++) 
			{
				fread(&temp, 4, 1, logfile);
				//if(loaded_pack_num[temp] == true) total_loaded_pack_num--;
				loaded_pack_num[temp] = true;
			}
			
			int need_send = total_packet_num - total_loaded_pack_num;
			int need_send_num[need_send];
			printf("need: %d\n", need_send);
			for (int i = 0, j = 0; i < total_packet_num; i++) 
			{
				if (loaded_pack_num[i] == false) 
				{
					printf("%d ", i + 1);
					need_send_num[j] = i;
					j++;
				}
			}
			
			char offset[256];
			sprintf(offset, "OFFS %d", need_send);
			for(int i = 0; i < need_send; ++i)
				sprintf(offset, "%s %d", offset, need_send_num[i]);
			puts(offset);
			sendto(cmd_sock, (char *)need_send, sizeof(int) * (total_packet_num-total_loaded_pack_num+1), 0,(struct sockaddr*) & serv_addr_cmd, sizeof(serv_addr_cmd));
			fclose(logfile);
			return true;
		}
	}

	char offset[256];
	sprintf(offset, "OFFS %d", pkt_num);
	sendto(cmd_sock, offset, strlen(offset), 0,(struct sockaddr*) & serv_addr_cmd, sizeof(serv_addr_cmd));
	remove(logfile_path);
	return false;
}