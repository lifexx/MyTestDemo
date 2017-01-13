#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <sys/epoll.h>
#include <malloc.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <stdlib.h>

#define CLIENT_PORT 12345
#define SERVER_PORT 12346
#define MAX_EPOLL_EVENTS  10
#define RECV_BUF_SIZE (8*1024)
#define SERVER_IP "172.16.75.103"



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
	bool bReuseAddr = false;
	int sockfd;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	if (!inet_aton(SERVER_IP, &serverAddr.sin_addr))
	{
		printf("error ip\n");
		return 0;
	}

	if (argc == 2 && argv[0] == "A")
	{
		bReuseAddr = true;
	}
	int opt = 1;
	if (bReuseAddr)
	{
		printf("resuse addr %s:%d", SERVER_IP, SERVER_PORT);
		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	}



	if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
	{
		printf("bind faild, err=%d(%s)\n", errno, strerror(errno));
		exit(0);
	}

	MakeSocketNonblocking(sockfd);

	listen(sockfd, 5);

	struct epoll_event ev;
	struct epoll_event events[MAX_EPOLL_EVENTS] = {0};
	ev.data.fd = sockfd;
	ev.events = EPOLLIN;

	int epfd = epoll_create(MAX_EPOLL_EVENTS);
	if (epfd == -1)
	{
		printf("epoll_create failed, err=%d(%s)\n", errno, strerror(errno));
		return 0;
	}

	int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
	if (ret == -1)
	{
		printf("epoll_ctl failed, err=%d(%s)\n", errno, strerror(errno));
		return 0;
	}

	int len = 0;
	//not free
	char* pRecvBuf = (char*)calloc(RECV_BUF_SIZE, 1);
	struct sockaddr_in clientAddr;
	memset(&clientAddr, 0, sizeof(clientAddr));
	while (1)
	{
		int size = epoll_wait(epfd, events, MAX_EPOLL_EVENTS, 500);
		if (size == -1)
		{
			printf("epoll_wait failed, err=%d(%s)\n", errno, strerror(errno));
			return 0;
		}

		
		for (int i = 0; i < size;i++)
		{
			if (events[i].data.fd == sockfd)
			{
				socklen_t socklen = sizeof(clientAddr);
				int newFd = accept(sockfd, (struct sockaddr*)&clientAddr, &socklen);
				if (newFd == -1)
				{
					printf("accept failed, err=%d(%s)\n", errno, strerror(errno));
					continue;
				}
				ev.data.fd = newFd;
				ev.events = EPOLLIN;
				MakeSocketNonblocking(newFd);
				epoll_ctl(epfd, EPOLL_CTL_ADD, newFd, &ev);
				continue;
			}

			if (events[i].events & EPOLLIN)
			{
				len = recv(events[i].data.fd, pRecvBuf, RECV_BUF_SIZE, 0);
				if (len == -1)
				{
					if (errno == EAGAIN)
					{
						continue;
					}

					printf("recv failed, err=%d(%s)\n", errno, strerror(errno));
					epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, 0);
					close(events[i].data.fd);
					continue;
				}
				else if (len == 0)
				{
					epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, 0);
					close(events[i].data.fd);
					continue;
				}

				printf("recv fd=%d, buf=%s\n", events[i].data.fd, pRecvBuf);
				memset(&pRecvBuf, 0, sizeof(pRecvBuf));
			}
			else
			{
				/* An error has occured on this fd, or the socket is not
				ready for reading (why were we notified then?) */
				printf("sockfd=%d err, err notified, err=%d(%s), events[i].events=%#x",
					events[i].data.fd, errno, strerror(errno), events[i].events);
				close(events[i].data.fd);
				continue;
			}

		}
		

	}
	

	return 0;
}


