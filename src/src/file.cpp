#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <iostream>
#include <errno.h>
#include "../include/file.h"

using namespace std;

File::File(const char* input_file, int _pkt_num)
{
    file_path = input_file;
    pkt_num = _pkt_num;
    send_rec = new bool[_pkt_num];
    memset(send_rec, 0, _pkt_num * sizeof(bool));
}

File::File(const char* input_file, int pkt_len, int offset)
{
    file_path = input_file;
    file_ptr = fopen(file_path, "rb");
    cal_file_size();
    cal_offset();
    cal_slice_len(pkt_len);
    cal_pkt_num(pkt_len);
    
    send_rec = new bool[pkt_num];
    memset(send_rec, 0, pkt_num * sizeof(bool));
    base_offset = offset;
}

File::~File()
{
	if (file_ptr)
		fclose(file_ptr);
	if (send_rec)
		delete(send_rec);
	if (file_path)
		delete(file_path);
}

int File::get_tot_num()
{
    return pkt_num;
}

int File::get_pkt_num()
{
    for(int i = 0; i < pkt_num; ++i)
        if(!send_rec[i])
            return i + 1;
    return 0;
}

long long File::get_file_len()
{
    return file_len;
}

bool File::eof()
{
    return get_pkt_num() == 0;
}

FILE* File::get_file()
{
    return file_ptr;
}

void File::cal_file_size()
{
#ifdef __linux__
    struct stat64 st;
    stat64(file_path, &st );
    file_len = st.st_size;
#endif

#ifdef _WIN32
    struct __stat64 st;
    __stat64(file_path, &st );
    file_len = st.st_size;
#endif
    /*
    int length = 0;
    FILE* fp = fopen(file_path, "rb");
    if (fp == nullptr)
    {
        file_len =  -1;
        return;
    }
    fseek(fp, 0L, SEEK_END);
    length = static_cast<int>(ftell(fp));
    fclose(fp);
    file_len = length;*/
}

void File::cal_offset()
{
    offset = strlen(file_path) + 2 * sizeof(char) + pkt_num_len;
}

void File::cal_pkt_num(int pkt_len)
{
    pkt_num = ceil((double)file_len / slice_len);
}

long long File::get_offset()
{
    return offset;
}

void File::pkt_send(int num)
{
    send_rec[num] = 1;
}

void File::cal_slice_len(int pkt_len)
{
    slice_len = pkt_len - offset;
}

int File::get_slice_len()
{
    return slice_len;
}

int File::get_base_offset()
{
    return base_offset;
}

void File::get_send_rec()
{
    for(int i=0;i<pkt_num;++i)
        printf("%d ", send_rec[i]);
    puts("");
}
const char* File::get_file_path()
{
    return this->file_path;
}