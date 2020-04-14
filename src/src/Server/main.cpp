#include "../../include/Server/Server.h"

bool parse_arg(int argc, char** argv)
{
	if (argc != 4)
	{
		puts("Wrong arg number!");
		exit(0);
	}
	#ifdef _WIN32
	if ((_access(argv[1], 0)) != -1)
	#endif
	#ifdef __linux__
	if ((access(argv[1], 0)) != -1)
	#endif
	{
	#ifdef _WIN32
		if ((_access(argv[1], 6)) == -1)
	#endif
	#ifdef __linux__
		if ((access(argv[1], 6)) == -1)
	#endif
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
}

int main(int argc, char** argv)
{
    parse_arg(argc, argv);
    Server* server = new Server(atoi(argv[3]));
    if(server->set_listen())
        while(1)
        {
            server->parse_cmd();
        }
}