#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <iostream>
#include <fcntl.h>

using namespace std;

#define SERVER_IP "192.168.114.129"

#define CLIENT_PORT 12345
#define SERVER_PORT 12346

int MakeSocketNonblocking(int sockfd)
{
	int flags = 0;
	int ret = 0;
	flags = fcntl(sockfd, F_GETFL, 0);
	if (flags == -1)
	{
		printf("%s F_GETFL failed , sockfd=%d", __FUNCTION__, sockfd);
		return -1;
	}

	flags |= O_NONBLOCK;
	ret = fcntl(sockfd, F_SETFL, flags);
	if (ret == -1)
	{
		printf("%s F_GETFL failed , sockfd=%d", __FUNCTION__, sockfd);
		return -1;
	}

	return 0;
}




int main(int argc, char* argv[])
{
	bool bResueAddr = false;


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

	MakeSocketNonblocking(sockfd);

	char buf[256] = { 0 };

	char cmd;
	while (1)
	{
		if (recv(sockfd, buf, sizeof(buf), 0) == 0)
		{
			printf("recv 0 byte, exit");
			close(sockfd);
			break;
		}
		sleep(1);


// 		if (cmd == 'q')
// 		{
// 			printf("exit\n");
// 			break;
// 		}
// 		else if (cmd == 'f')
// 		{
// 			close(sockfd);
// 		}
// 		else if (cmd == 's')
// 		{
// 			send(sockfd, sendata, strlen(sendata), 0);
// 		}
// 		
// 
// 		printf("input cmd:  q-exit; f-closesocket; s-senddata;");

	}

	return 0;
}