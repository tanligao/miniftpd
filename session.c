#include "session.h"
#include "ftpproto.h"
#include "privparent.h"

void begin_session(session_t *sess)
{
	// 创建一对套接字，用于父子进程通信
	int sockfds[2];
	if( socketpair(AF_LOCAL,SOCK_STREAM,0,sockfds) < 0 )
	{
		ERR_EXIT("setsockpair");
	}

	pid_t pid;
	// 创建服务进程，父进程为nobody进程
	pid = fork();

	switch(pid)
	{
		case -1:
			ERR_EXIT("fork service");
			break;
		case 0:
			// service进程，使用sockfds[1]通信
			close(sockfds[0]);
			sess->child_fd = sockfds[1];
			handle_child(sess);		
			break;
		default:
		{
			struct passwd *pw = getpwnam("nobody");
			if( pw == NULL )
				return;

			if( setegid(pw->pw_gid) < 0 )
			{
				ERR_EXIT("setegid");
			}

			if( seteuid(pw->pw_uid) < 0 )
			{
				ERR_EXIT("seteuid");
			}
			// nobody进程，使用sockfds[0]与子进程通信
			close(sockfds[1]);
			sess->parent_fd = sockfds[0];
			handle_parent(sess);
			break;
		}
	}

}
