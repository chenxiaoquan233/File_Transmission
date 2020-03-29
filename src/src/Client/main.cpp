#include "../../include/Client/Client.h"

int main(int argc, char** argv)
{
    Client* client = new Client();
    client->set_up_connection("127.0.0.1", 8080);
    client->send_file(argv[1]);
    return 0;
}