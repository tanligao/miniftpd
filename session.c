#include "session.h"

void begin_session(int connfd)
{
	pid_t pid;
	// 创建服务进程，父进程为nobody进程
	pid = fork();

	switch(pid)
	{
		case -1:
			ERR_EXIT("fork service");
			break;
		case 0:
			// service进程
			
			break;
		default:

			break;
	}

}
