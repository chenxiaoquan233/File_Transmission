#include "../../include/Client/Client.h"

bool parse_arg(int argc, char** argv, int port, char* file_path, char* ip_addr)
{
    bool dir_flag;
    if(argc == 4) // USE CMD ARGS, NO GUI 
    {
        //judge arg is dir or file
#ifdef __linux__
        struct stat s_buf;
        stat(argv[1], &s_buf);
        if(S_ISDIR(s_buf.st_mode))
        {
            DIR* dir = opendir(argv[1]);
            if(dir == NULL)
            {
                puts("Directory not exists!");
                exit(0);
            }
            closedir(dir);
            dir_flag = true;
        }
        else if(S_ISREG(s_buf.st_mode))
        {
            FILE* file = fopen(argv[1], "r");
            if(file == NULL)
            {
                puts("File not exists!");
                exit(0);
            }
            fclose(file);
            dir_flag = false;
        }
#endif
#ifdef _WIN32
        int access(const char *filename, int mode);
#endif

    }
    else if(argc == 1)
    {
        // GUI
    }
    return dir_flag;
}


int main(int argc, char** argv)
{
    int port;
    char* file_path;
    char* ip_addr;
    parse_arg(argc, argv, port, file_path, ip_addr);
    Client* client = new Client();
    client->set_up_connection("127.0.0.1", 8080);
    client->send_file(argv[1]);
    char* test[100];
    for (int i = 0; i < 100; i++)
    {
        test[i] = new char[1000];
        memset(test[i], 0, 1000);
    }
    int folder_number=client->read_path(argv[2], test);
    /*
    for (int i=0; i <= folder_number; i++)
    {
        std::cout << test[i];
    }
    */
    return 0;
}