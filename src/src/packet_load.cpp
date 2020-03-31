#include "../include/packet_load.h"

packet_load::packet_load()
{
}

packet_load::~packet_load()
{
}

char* packet_load::get_file_slice()
{
    return file_slice;
}

bool packet_load::create_file_slice(int length)
{
    file_slice = new char[length];
    memset(file_slice, 0, length);
    return file_slice!=nullptr;
}

int packet_load::get_slice_num()
{
    return slice_num;
}

void packet_load::set_slice_num(int _num)
{
    slice_num = _num;
}