#include "../../include/Server/Server.h"

bool parse_arg(int argc, char** argv)
{
	if (argc != 4)
	{
		puts("Wrong arg number!");
		exit(0);
	}
	if ((access(argv[1], 0)) != -1)
	{
		if ((access(argv[1], 6)) == -1)
		{
			puts("Read or Write Permission Denied!");
			exit(0);
		}
		return true;
	}
	else
	{
		puts("File does not exist!");
		exit(0);
	}
	exit(0);
}

int main(int argc, char** argv)
{
    parse_arg(argc, argv);
    Server* server = new Server();
    if(server->set_listen(8080))
        while(1)
        {
            server->parse_cmd();
        }
}