#include "../../include/Client/Client.h"

int main(int argc, char** argv)
{
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
    for (int i=0; i < folder_number; i++)
    {
        std::cout << test[i];
    }
    */
    return 0;
}