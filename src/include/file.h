class File
{
private:
    char* file_path = nullptr;
    FILE* file_ptr = nullptr;
    int file_len = -1;
    int pkt_num = -1;
    bool* send_rec;
    int offset = -1;
    const int pkt_num_len = 3;
    int base_offset = -1;
    int slice_len = -1;
public:
    //constructor
    File(char* input_file, int pkt_len, int offset);
    File(char* input_file, int pkt_num);

    //deconstructor
    ~File();

    //aquire a packet num which is not sent
    int get_pkt_num();
    int get_file_len();
    void set_file_len(int len);
    bool eof();
    FILE* get_file();
    void cal_file_size();
    void cal_pkt_num(int pkt_len);
    void cal_offset();
    int get_offset();
    void pkt_send(int num);
    void cal_slice_len(int pkt_len);
    int get_slice_len();
    int get_base_offset();
    void get_send_rec();
    int  get_tot_num();
};