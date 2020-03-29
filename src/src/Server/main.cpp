#include "../../include/Server/Server.h"

int main()
{
    Server* server = new Server();
    if(server->set_listen(8080))
        while(1)
        {
            server->recv_packet();
        }
}