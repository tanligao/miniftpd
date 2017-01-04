#ifndef __SESSION_H__
#define __SESSION_H__

#include "common.h"

typedef struct session
{
	int uid;
	// 控制连接
	int ctrl_fd;
	char cmdline[MAX_COMMAND_LINE];
	char cmd[MAX_COMMAND];
	char cmd_arg[MAX_ARG];
	// 进程通信fd
	int parent_fd;
	int child_fd;
	// ftpproto state
	int is_ascii;
	// port_addr
	struct sockaddr_in *port_addr;
	// data fd
	int data_fd;
	int pasv_listen_fd;
} session_t;

void begin_session(session_t *sess);

#endif /* __SESSION_H__*/
