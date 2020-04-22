#include "../../interface/mainwindow.h"
#include <QApplication>
#include "../../include/Client/Client.h"

bool parse_arg(int argc, char** argv, char** file_path, char** ip_addr, int* port)
{
    bool dir_flag = false;
    if(argc == 4)
    {
        //judge arg is dir or file
        #ifdef __linux__
        struct stat s_buf;
        stat(argv[1], &s_buf);

        if(S_ISDIR(s_buf.st_mode)) // is directory
        {
            DIR* dir = opendir(argv[1]);
            if(dir == nullptr) // dir not exists
            {
                puts("Directory not exists!");
                exit(0);
            }
            closedir(dir);
            int read_mask = 0x124; // r--r--r-- 0x 100 100 100
            if(!(s_buf.st_mode & read_mask)) 
            {
                puts("Directory Read Permission Denied!");
                exit(0);
            }
            dir_flag = true;
        }
        else if(S_ISREG(s_buf.st_mode)) // is file
        {
            FILE* file = fopen(argv[1], "r");
            if(file == nullptr)
            {
                puts("File not exists!");
                exit(0);
            }
            fclose(file);
            dir_flag = false;
        }
        #endif

        #ifdef _WIN32
        if(GetFileAttributesA(argv[1])&FILE_ATTRIBUTE_DIRECTORY)// is directory
        {
            if(_access(argv[1], 2) < 0)
            {
                puts("Directory not exists or Read Permission Denied!");
                exit(0);
            }
            dir_flag = true;
        }
        else // is file
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
        *file_path = argv[1];
        *ip_addr = argv[2];
        *port = atoi(argv[3]);
    }
    else
    {
        puts("Wrong arg number!");
        exit(0);
    }
    return dir_flag;
}

int main(int argc, char** argv)
{
    int port;
    char* file_path = nullptr;
    char* ip_addr = nullptr;
    bool dir_flag;

    if(argc > 1)
    {
        Client* client;
        dir_flag = parse_arg(argc, argv, &file_path, &ip_addr, &port);
        if(init_connect(client, ip_addr, port))
            start_send(client, file_path, dir_flag);
    }
    else
    {
        QApplication app(argc,argv);
        MainWindow m;
        m.show();
        return app.exec();
    }

    return 0;
}
