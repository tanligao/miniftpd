#include "ftpproto.h"
#include "common.h"
#include "sysutil.h"

void handle_child(session_t *sess)
{
	char *welcome_str = "220 (miniftpd 0.1)\r\n";
	writen(sess->ctrl_fd,welcome_str,strlen(welcome_str));
	while(1)
	{
		memset(sess->cmdline,0,MAX_COMMAND_LINE);
		memset(sess->cmd,0,MAX_COMMAND);
		memset(sess->cmd_arg,0,MAX_ARG);
		readline(sess->ctrl_fd,sess->cmdline,MAX_COMMAND_LINE);

		// 解析读取到的FTP命令与参数，处理FTP命令，然后发送给父进程
		
	}
}
