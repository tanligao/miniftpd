#include "common.h"
#include "sysutil.h"


int main(int argc,char *argv[])
{
	if( getuid() != 0 )
	{
		printf("run miniftpd must be root\n");
		exit(EXIT_FAILURE);
	}	
	int listenfd = tcp_server(NULL,8888);

	pid_t pid;
	for( ; ; )
	{
		// 时间设置为0,阻塞接收连接
		int connfd = accept_timeout(listenfd,NULL,0);
		if( connfd == -1 )
			continue;
		// 创建子进程
		pid = fork();
		switch(pid)
		{
			case 0:
				// 子进程关闭listenfd，避免出现“惊群效应”
				close(listenfd);
				break;
			case -1:
				ERR_EXIT("fork");
				break;
			default:
				// 父进程关闭connfd
				close(connfd);
				break;
		}
	}

	return EXIT_SUCCESS;
}
