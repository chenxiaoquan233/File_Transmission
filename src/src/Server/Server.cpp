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
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
        return false;
    if(bind(sock, (struct sockaddr*)&serv_addr, sizeof(sockaddr)) == -1)
        return false;
    return true;
}

bool Server::recv_packet()
{
    if(data) delete data;//data equals nullptr the first time, but not after
    data = new packet_load();

    socklen_t nSize = sizeof(sockaddr);
    data->create_file_slice(MAX_PACKET_DATA_BYTE_LENGTH);
    if(recvfrom(sock, data->get_file_slice(), MAX_PACKET_DATA_BYTE_LENGTH, 0, (struct sockaddr*)&serv_addr, &nSize) == -1)
        return false;
    return true;
}

bool Server::recv_whole_file()
{
	if (output_file)output_file = nullptr;//clear the output_file
	if (buffer)delete(buffer);//clear the buffer
	int file_length = 0; // count the received file length
	int total_length; // the whole length of the received file
	while (1)
	{
		while (1)//receive the next file slice
		{
			if (recv_packet())break;
		}
		int packet_serial_number = return_packet_serial_number();
		send_ack(packet_serial_number);//send the ack to client
		
	    /*update information of file and create new buffer*/
		if (!output_file)//head of file
		{
			read_FILEinformation(output_file, data->get_file_slice(), total_length);
			buffer = new char(total_length + 1);
		}	
		
		/*read this slice into buffer and update file_length*/
		file_length += get_file_data(data->get_file_slice(), buffer + file_length, total_length - file_length);
		
		/*already received the whole file*/
		if (file_length == total_length)
		{
			break;
		}
	}
	write_file(output_file, buffer, total_length);
	return true;
}
int Server::get_file_data(char* src, char* dest, int maxlen)
{
	int srctot = 0,dsttot = 0;
	while (src[srctot] != '@')++srctot;//find @
	++srctot;
	while (maxlen && src[srctot] != '\0')
	{
		--maxlen;
		dest[dsttot++] = src[srctot];
		++srctot;
	}
	return srctot;
}
bool Server::send_ack(int num)
{
	int packet_serial_number = num;//get packet serial number
	char ack[4];
	itoa(packet_serial_number, ack, 10);//transform packet_serial_number to char*
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

bool Server::write_file(FILE* output_file,char * data,int data_length)
{
	for (int data_i = 0; data_i < data_length; data_i++)
		fputc(data[data_i],output_file);
	return true;
}
int Server::return_packet_serial_number()
{
	int position = 0, packet_serial_number = 0, position_first_file, position_first_file_flag = 0;
	while (data->get_file_slice[position] != '?')
	{
		position++;
	}
	position_first_file = position;
	while (data->get_file_slice[position_first_file] != '&')
	{
		if (data->get_file_slice[position_first_file] == '@')
		{
			position_first_file_flag = 1;
			break;
		}
		position_first_file++;
	}
	if (position_first_file_flag == 1)
	{
		position_first_file++;
		while (data->get_file_slice[position_first_file] != '&')
		{
			packet_serial_number *= 10;
			packet_serial_number += (data->get_file_slice[position_first_file] - '0');
			position_first_file++;
		}
		return packet_serial_number;

	}
	else
	{
		position++;
		while (data->get_file_slice[position] != '&')
		{
			packet_serial_number *= 10;
			packet_serial_number += (data->get_file_slice[position] - '0');
			position++;
		}
		return packet_serial_number;
	}
}