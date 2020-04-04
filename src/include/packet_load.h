#include "base.h"

class pkt_load
{
private:
    char* file_slice = nullptr;
    int slice_num = -1;
    int slice_len;
public:
    pkt_load();
    ~pkt_load();

    char* get_file_slice();
    bool create_file_slice(int length);

    int get_slice_num();
    void set_slice_num(int num);
    int get_slice_len();
    void set_slice_len(int len);
};