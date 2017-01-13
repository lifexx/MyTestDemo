#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#define SERVER_IP "172.16.75.103"

#define CLIENT_PORT 12345
#define SERVER_PORT 12346

int main(int argc, char* argv[])
{
	int sockfd;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in sockaddr;
	memset(&sockaddr, 0, sizeof(sockaddr));

	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(CLIENT_PORT);
	if (!inet_aton(SERVER_IP, &sockaddr.sin_addr))
	{
		printf("error ip\n");
		return 0;
	}

	int ret = bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
	if (ret == -1)
	{
		printf("bind failed, err=%d(%s)\n", errno, strerror(errno));
		exit(0);
	}


	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	if (!inet_aton(SERVER_IP, &serverAddr.sin_addr))
	{
		printf("error ip\n");
		return 0;
	}

	connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	char *sendata = "hello, i'm client";
	send(sockfd, sendata, strlen(sendata), 0);

	char buf[256] = { 0 };
	while (gets(buf))
	{
		if (buf == "exit")
		{
			printf("exit\n");
			break;
		}
	
		

	}

	return 0;
}