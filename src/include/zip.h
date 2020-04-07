#pragma once
#include"zlib/zlib.h"
#include<cstdio>
#include<cstdlib>
//return length of dst string
int zip(char* src, char* &dest, int src_len);
bool unzip(char* src, char* &dest, int src_len, int dst_len);
