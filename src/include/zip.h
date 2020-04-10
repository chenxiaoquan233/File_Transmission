#pragma once
#ifdef _WIN32
#include"zlib/zlib.h"
#endif
#ifdef __linux__
#include <zlib.h>
#endif
#include<cstdio>
#include<cstdlib>
//return length of dst string
int zip(char* src, char* &dest, int src_len);
bool unzip(char* src, char* &dest, int src_len, int dst_len);
