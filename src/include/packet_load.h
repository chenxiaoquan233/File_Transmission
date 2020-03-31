#include "base.h"

class packet_load
{
private:
    char* file_slice = nullptr;
    int slice_num = -1;
    
public:
    packet_load();
    ~packet_load();

    char* get_file_slice();
    bool create_file_slice(int length);

    int get_slice_num();
    void set_slice_num(int num);
};