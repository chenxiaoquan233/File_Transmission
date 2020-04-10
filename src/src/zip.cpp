#include"../include/zip.h"
int zip(char* src, char* &dest, int src_len)
{
	delete(dest);//clear ptr

	uLong tlen = src_len,blen;
	/* cal the bound of length */
	blen = compressBound(tlen);	
	if ((dest = (char*)malloc(sizeof(char) * blen)) == NULL)
	{
		return -1;
	}
	/* compress */
	if (compress((Bytef*)dest, &blen, (Bytef*)src, tlen) != Z_OK)
	{
		return -1;
	}
	return blen;
}
bool unzip(char* src, char* &dest, int src_len, int dst_len)
{
	if ((dest = (char*)malloc(sizeof(char) * dst_len)) == NULL)
	{
		return -1;
	}
	uLong tlen = dst_len;
	if (uncompress((Bytef*)dest, &tlen, (Bytef*)src, src_len) != Z_OK)
	{
		return 0;
	}
	return 1;
}