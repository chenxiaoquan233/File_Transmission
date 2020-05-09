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
	if (argc==1)
	{
		char bin[20] = "127.0.0.1";
		char p[30] = "C://test/test/";
		Server* server = new Server(bin, 8080);
		server->set_path(p);
		puts("listen");
		if (server->set_listen())
		{

			while (1)
			{
				server->parse_cmd();
			}
		}
		else
		{
			puts("Inital Failed!\nPlease Check Your Port!");
		}
	}
	else {

		parse_arg(argc, argv);
		Server* server = new Server(argv[2], atoi(argv[3]));
		server->set_path(argv[1]);
		puts("listen");
		if (server->set_listen())
		{
			while (1)
			{
				server->parse_cmd();
			}
		}
		else
		{
			puts("Inital Failed!\nPlease Check Your Port!");
		}
	}
}