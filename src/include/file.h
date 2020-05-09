#include <stdio.h>

class File
{
private:
    const char* file_path = nullptr;
    FILE* file_ptr = nullptr;
    long long file_len = -1;
    int pkt_num = -1;
    bool* send_rec;
    long long offset = -1;
    const int pkt_num_len = 3;
    int base_offset = -1;
    int slice_len = -1;
public:
    //constructor
    File(const char* input_file, int pkt_len, int offset);
    File(const char* input_file, int pkt_num);

    //deconstructor
    ~File();

    //aquire a packet num which is not sent
    int get_pkt_num();
    long long get_file_len();
    bool eof();
    FILE* get_file();
    void cal_file_size();
    void cal_pkt_num(int pkt_len);
    void cal_offset();
    long long get_offset();
    void pkt_send(int num);
    void cal_slice_len(int pkt_len);
    int get_slice_len();
    int get_base_offset();
    void get_send_rec();
    int  get_tot_num();
};
