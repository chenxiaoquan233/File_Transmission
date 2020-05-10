#include "../../include/Server/Server.h"
Server::Server(char* addr, int port)
{
	listen_addr = addr;
	cmd_port = port;
}

Server::~Server()
{
	closesocket(client_sock);
	closesocket(cmd_sock);
}

bool Server::set_listen()
{
	#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    cmd_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	#endif
	#ifdef __linux__
    cmd_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	#endif
    memset(&serv_addr_cmd, 0, sizeof(serv_addr_cmd));
    serv_addr_cmd.sin_family = AF_INET;
    serv_addr_cmd.sin_addr.s_addr = inet_addr(listen_addr);
	//serv_addr_cmd.sin_addr.s_addr = INADDR_ANY;
    serv_addr_cmd.sin_port = htons(cmd_port);

    if(bind(cmd_sock, (struct sockaddr*)&serv_addr_cmd, sizeof(sockaddr)) == -1)
	{
		perror("bind failed!");
		return false;
	}


	listen(cmd_sock, 5);

	while (1)
	{
		struct sockaddr_in addr;
		int len = sizeof(SOCKADDR);

		char buf[1024] = { 0 };

		//接受客户端连接
		if ((client_sock = accept(cmd_sock, (struct sockaddr*) & addr, &len)) != -1)break;
	}

    return true;
}

bool Server::check_port() 
{
	if(start_port == -1)
		start_port = cmd_port > 1024 ? cmd_port + 1 : 1024;

	char answer_port[12];
	sprintf(answer_port, "PORT %d", start_port);
	
	if(send(client_sock, answer_port, strlen(answer_port), 0) < 0)
	{
		perror("send");
		return false;
	}
	return true;
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
	int already_recv = 0;
	int pkt_num = 0;
	
	while(already_recv < MAX_PACKET_DATA_BYTE_LENGTH)
	{

		bool flg = 0;
		int res = recv(client_sock, data->get_file_slice() + already_recv, MAX_UDP_PACKET_LEN, 0);
		if (res == -1)
			cout << WSAGetLastError() << endl;
		if(!already_recv)//the first packet of the file slice
		{
			pkt_num = get_ack(data->get_file_slice());
		}
		already_recv += res;
		send_ack(pkt_num);
		if (res < MAX_UDP_PACKET_LEN)
		{

 			break;// the last packet may not meet the max size
		}
	}
	data->set_slice_len(already_recv);
	cout<<already_recv<<endl;
    return true;
}

bool Server::recv_whole_file()
{
	clock_t start_t, end_t;
	start_t = clock();
	
	if(buffer) delete(buffer);//clear the buffer

	int total_length; // the whole length of the received file

	int slice_num = 0;
	char* file_path = new char[500];
	char* file_slice;
	int data_len = 0;
	char* file_name = new char[500];
	bool checked = 0;
	while(!(file->eof()))
	{
		slice_num = 0;
		data_len = 0;
		memset(file_path, 0, 500 * sizeof(char));
		file_slice = new char[MAX_PACKET_DATA_BYTE_LENGTH];
		if(recv_packet())
		{
			parse_param(data->get_file_slice(), file_path, &slice_num, file_slice, data->get_slice_len(), &data_len);

			// record as recieved
			file->pkt_send(slice_num - 1);

			// record on log
			//write_logfile(file_path, (short)slice_num, sizeof(short));

			// write file slice
			sprintf(file_name, "%s/%s.%03d", path, file_path, slice_num);
			checked = 1;
			brute_create_folder(file_name);
			FILE* output_file_slice = fopen(file_name, "wb");
			fwrite(file_slice, 1, data_len, output_file_slice);
			fclose(output_file_slice);
		}
		else
		{
			return false;
		}
		delete file_slice;
	}

	puts("here");
	end_t = clock();
	printf("recv time: %f\n", ((double)(end_t - start_t) / CLOCKS_PER_SEC));
	if (checked)
	{
		sprintf(file_name, "%s/%s", path, file_path);
		puts(file_name);
		thread* new_thread = new thread(mergeFile, file_name, file->get_tot_num(), MAX_PACKET_DATA_BYTE_LENGTH);
		threads.push_back(new_thread);
		
	}
	else 
	{
		sprintf(file_name, "%s", file->get_file_path());
		thread* new_thread = new thread(mergeFile, file_name, file->get_tot_num(), MAX_PACKET_DATA_BYTE_LENGTH);
		threads.push_back(new_thread);
			

	}
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
	//printf("packet %d get\n", num);
	sprintf(ack, "%d", num);
	int res = send(client_sock, ack, strlen(ack), 0);
	return res != -1;
}

int Server::read_FILEinformation(FILE*& output_file, char* origin_data, int& data_length)
{
    int info_length = 0;
    char FILE_path[200];
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
	char* tmpDirPath = new char[200];
	memset(tmpDirPath, 0, 200);
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
		if (access(path, 0) == -1)
		{
			int ret = mkdir(path, 0777);
				if (ret == -1)
				{
					printf("%s create failed\n", path);
					return ret;//Create failure
				}
		}
	}
	#endif
	return 0;//Create success
}

bool Server::parse_path()
{
	char buf[1000];
	char* r_cmd = new char[4];
	memset(r_cmd, 0, 4);
	memset(buf, 0, 1000);

	#ifdef __linux__
	socklen_t nSize = sizeof(sockaddr);
	#endif
	#ifdef WIN32
	int nSize = sizeof(sockaddr);
	#endif

	recv(client_sock, buf, 1000 * sizeof(char), 0);

	/*int pos = 0;
	while(pos < strlen(buf))
	{
		char addr[1000];
		sscanf(buf + pos, "%s", addr);
		pos += strlen(addr) + sizeof('\n');
		char * abs_path = new char[1000];
		sprintf(abs_path, "%s/%s", path, addr);
		//set_dir(abs_path);
	}*/
	
	sprintf(r_cmd, "%s", "INFO");
	int res = send(client_sock, r_cmd, 4, 0);
	return res != -1;
}

void Server::parse_cmd()
{
	char* cmd = new char[1000];
	memset(cmd, 0, 1000);

	#ifdef __linux__
	socklen_t nSize = sizeof(sockaddr);
	#endif

	#ifdef WIN32
	int nSize = sizeof(sockaddr);
	#endif

	int res = -1;

	res = recv(client_sock, cmd, 1000 * sizeof(char), 0);
	//res = recvfrom(cmd_sock, cmd, 256 * sizeof(char), 0, (struct sockaddr*) & serv_addr_cmd, &nSize);
	if(res == -1) return;
	puts("CMD:");
    puts(cmd);
	if (cmd[0] == 'I' && cmd[1] == 'N' && cmd[2] == 'F' && cmd[3] == 'O')
	{
		if(check_port())
			parse_path();
	}
	else if (cmd[0] == 'S' && cmd[1] == 'E' && cmd[2] == 'N' && cmd[3] == 'D')
	{
		int i = 5;
		int file_len = 0;
		int tot_pkt_num = 0;
		char* file_name = new char[1000];
		memset(file_name, 0, 1000);

		// SEND filename pkt_num file_length
		i = strlen(cmd) - 1;
		int base = 1;
		while (cmd[i] != ' ')
		{
			file_len += base * (cmd[i--] - '0');
			base *= 10;
		}
		i--;
		base = 1;
		while (cmd[i] != ' ')
		{
			tot_pkt_num += base * (cmd[i--] - '0');
			base *= 10;
		}
		i--;
		for (int j = 5; j <= i; ++j)
		{
			file_name[j - 5] = cmd[j];
		}



		/*while(i < strlen(cmd) && cmd[i] != ' ')
			file_name[i - 5] = cmd[i], i++;
		i++;
		while(i < strlen(cmd) && cmd[i] != ' ')
			tot_pkt_num *= 10, tot_pkt_num += cmd[i++] - '0';
		i++;
		while(i < strlen(cmd) && cmd[i] != ' ')
		{
			file_len *= 10, file_len += cmd[i++] - '0';
		}*/

		char* send_file_name = new char[1000];
		memset(send_file_name, 0, 1000);
		sprintf(send_file_name, "%s/%s", path, file_name);
		file = new File(send_file_name, tot_pkt_num);
		if(!check_file(send_file_name, file_len, tot_pkt_num)) 
		{
			//write_logfile(send_file_name, (short)tot_pkt_num, sizeof(short));
			//write_logfile(send_file_name, file_len, sizeof(int));
		}
		recv_whole_file();
	}
	else if (cmd[0] == 'P' && cmd[1] == 'O' && cmd[2] == 'R' && cmd[3] == 'T')
	{
		check_port();
		//if(check_port())
			//recv_whole_file();
	}
	else if (cmd[0] == 'O' && cmd[1] == 'N')
	{
		send(client_sock, "1", 1, 0);
	}
	return;
}

bool Server::write_logfile(char* path, int number, int size)
{
	char logfile_path[200];
	sprintf(logfile_path, "%s.FTlog", path);

	FILE* logfile = fopen(logfile_path, "ab");
	fwrite(&number, size, 1, logfile);
	fclose(logfile);
	return true;
}

bool Server::check_file(char* file_name, int file_len, int pkt_num) 
{
	/*
	char* logfile_path = new char[200];
	sprintf(logfile_path, "%s.FTlog", file_name);
	FILE* logfile;

	logfile = fopen(logfile_path, "rb");
	if (logfile) 
	{
		short total_packet_num = 0;
		int log_file_length = 0;

		fread(&total_packet_num, sizeof(short), 1, logfile);
		fread(&log_file_length, sizeof(int), 1, logfile);

		if (log_file_length == file_len) 
		{
			bool* loaded_pack_num = new bool[total_packet_num];
			for (int i = 0; i < total_packet_num; i++)
				loaded_pack_num[i] = false;
				
			int total_loaded_pack_num = 0;
			int temp = 0;
			while(!feof(logfile))
			{
				fread(&temp, sizeof(short), 1, logfile);
				if(!loaded_pack_num[temp])
				{
					loaded_pack_num[temp] = true;
					total_loaded_pack_num++;
				}
			}
			int need_send = total_packet_num - total_loaded_pack_num;
			printf("need_send:%d\n",need_send);
			int *need_send_num = new int[need_send];
			for (int i = 0, j = 0; i < total_packet_num; i++) 
			{
				if (loaded_pack_num[i] == false) 
				{
					need_send_num[j] = i;
					j++;
				}
				else file->pkt_send(i);
			}
			
			char* offset = new char[3000];
			sprintf(offset, "OFFS %d", need_send);
			for(int i = 0; i < need_send; ++i)
				sprintf(offset, "%s %d", offset, need_send_num[i]);
			if(sendto(cmd_sock, offset, strlen(offset), 0,(struct sockaddr*) & serv_addr_cmd, sizeof(serv_addr_cmd))<0)
				perror("offset");
			fclose(logfile);
			delete(need_send_num);
			return true;
		}
	}
	char offset[256];
	sprintf(offset, "OFFS %d", pkt_num);
	sendto(cmd_sock, offset, strlen(offset), 0,(struct sockaddr*) & serv_addr_cmd, sizeof(serv_addr_cmd));
	remove(logfile_path);
	return false;
	*/


	bool* loaded_pack_num = new bool[pkt_num];
	for (int i = 0; i < pkt_num; i++)
		loaded_pack_num[i] = false;
	char* logfile_path = new char[200];
	int total_loaded_pack_num = 0;

	sprintf(logfile_path, "%s", file_name);
	
#ifdef __linux__
	if (access(logfile_path, 0) == 0)
#endif
#ifdef WIN32
	if (_access(logfile_path, 0) == 0)
#endif
	{
		for (int i = 0; i < pkt_num; i++)
		{
			if (!loaded_pack_num[i])loaded_pack_num[i] = true, ++total_loaded_pack_num;
		}
	}
	for (int i = 0; i < pkt_num; i++)
	{
		sprintf(logfile_path, "%s.%03d", file_name,i+1);
#ifdef __linux__
		    if (access(logfile_path, 0) == 0)
#endif
#ifdef WIN32
			if (_access(logfile_path, 0) == 0)
#endif
		{
			if (!loaded_pack_num[i])loaded_pack_num[i] = true, ++total_loaded_pack_num;
		}
	}
	if (!total_loaded_pack_num)
	{
		char offset[256];
		sprintf(offset, "OFFS %d", pkt_num);
		send(client_sock, offset, strlen(offset), 0);
		remove(logfile_path);
		return false;
	}
	int need_send = pkt_num - total_loaded_pack_num;
	printf("need_send:%d\n", need_send);
	int* need_send_num = new int[need_send];
	for (int i = 0, j = 0; i < pkt_num; i++)
	{
		if (loaded_pack_num[i] == false)
		{
			need_send_num[j] = i;
			j++;
		}
		else file->pkt_send(i);
	}
	char* offset = new char[3000];
	sprintf(offset, "OFFS %d", need_send);
	for (int i = 0; i < need_send; ++i)
		sprintf(offset, "%s %d", offset, need_send_num[i]);
	if (send(client_sock, offset, strlen(offset), 0) < 0)
		perror("offset");
	delete(need_send_num);
	return true;

}

void mergeFile(char* fileaddress, int package,int max_pck_sze)
{
    
	int filelen = strlen(fileaddress);
	cout<<fileaddress<<' '<<filelen<<endl;

    char* buffaddress,*buf;
	if ((buffaddress = (char*)malloc(sizeof(char) * (filelen + 5))) == NULL)
	{
		puts("memory not available"); return;
	}
	if ((buf = (char*)malloc(sizeof(char) * (max_pck_sze + 5))) == NULL)
	{
		puts("memory not available"); return;
	}
    FILE* dst, * src;

#ifdef WIN32
	if (_access(fileaddress, 0) == 0) { delete(fileaddress); return; }
#endif
#ifdef __linux__
	if (access(fileaddress, 0) == 0) { delete(fileaddress); return; }
#endif

	if ((dst = fopen(fileaddress, "wb+")) == NULL)
	{
		puts("cannot create file"); return;
	}
	for (int i = 1; i <= package; i++)
    {
        sprintf(buffaddress, "%s.%03d", fileaddress, i);
		puts(buffaddress);
		if ((src = fopen(buffaddress, "rb")) == NULL)
		{
			puts("file not existss"); return;
		}
		int sze = get_filesize(buffaddress);
		fread(buf, sze, 1, src);
		fwrite(buf, sze, 1, dst);
        fclose(src);
        remove(buffaddress);
    }
    fclose(dst);
	delete(buffaddress);
	delete(buf);
	delete(fileaddress);
	return;
}
int get_filesize(char* filename)
{
		FILE* fp = fopen(filename, "r");
		if (!fp) return -1;
		fseek(fp, 0L, SEEK_END);
		int size = ftell(fp);
		fclose(fp);

		return size;
}
int Server::get_ack(char* data)
{
	//puts(data);
	int ack_num = 0;
	int i = 0;
	while(data[++i]!='@');
	++i;

	while(data[i] != '&')
	{
		ack_num *= 10;
		ack_num += data[i++] - '0';
	}
	return ack_num;
}

void Server::set_path(char* path)
{
	this->path = path;
}
void Server::brute_create_folder(string a)
{
	string b = a;
	while (b.back() != '/')b.pop_back();
	b.pop_back();
	
#ifdef WIN32
	if (_access(b.c_str(), 0) == 0)return;
#endif
#ifdef __linux__
	if (access(b.c_str(), 0) == 0)return;
#endif
	brute_create_folder(b);
#ifdef WIN32
	_mkdir(b.c_str());
#endif
#ifdef __linux__
	mkdir(b.c_str(), 777);
#endif
}