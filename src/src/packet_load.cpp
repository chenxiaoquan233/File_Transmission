#include "../include/packet_load.h"

pkt_load::pkt_load()
{
}

pkt_load::~pkt_load()
{
}

char* pkt_load::get_file_slice()
{
    return file_slice;
}

bool pkt_load::create_file_slice(int length)
{
    file_slice = new char[length];
    memset(file_slice, 0, length);
    return file_slice!=nullptr;
}

int pkt_load::get_slice_num()
{
    return slice_num;
}

void pkt_load::set_slice_num(int _num)
{
    slice_num = _num;
}

int pkt_load::get_slice_len()
{
    return slice_len;
}

void pkt_load::set_slice_len(int len)
{
    slice_len = len;
}