#include "../../include/Client/Client.h"

int main(int argc, char** argv)
{
    Client* client = new Client();
    client->client(argv[1]);
}