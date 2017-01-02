#include "ftpproto.h"
#include "common.h"
#include "sysutil.h"
#include "str.h"
#include "ftpcodes.h"

static void do_user(session_t *sess);
static void do_pass(session_t *sess);
void ftp_relply(session_t *sess,int status,const char *text);

void handle_child(session_t *sess)
{
	ftp_relply(sess,FTP_GREET,"(miniftpd 0.1)");
	while(1)
	{
		memset(sess->cmdline,0,MAX_COMMAND_LINE);
		memset(sess->cmd,0,MAX_COMMAND);
		memset(sess->cmd_arg,0,MAX_ARG);
		readline(sess->ctrl_fd,sess->cmdline,MAX_COMMAND_LINE);

		// 解析读取到的FTP命令与参数，处理FTP命令，然后发送给父进程
		printf("%s", sess->cmdline);
		str_trim_crlf(sess->cmdline);
		printf("%s\n", sess->cmdline);
		
		// 将命令与参数分割
		str_split(sess->cmdline,sess->cmd,sess->cmd_arg,' ');
		printf("%s %s\n",sess->cmd,sess->cmd_arg);
		// 将命令全部转化为大写
		str_upper(sess->cmd);

		if( strcmp("USER",sess->cmd) == 0 )
		{
			do_user(sess);
		}
		else if( strcmp("PASS",sess->cmd) == 0 )
		{
			do_pass(sess);
		}
	}
}

static void do_user(session_t *sess)
{
	struct passwd *pw = getpwnam(sess->cmd_arg);
	if( pw == NULL )
	{
		// user is not exist
		ftp_relply(sess,FTP_LOGINERR,"Login incorrect.");
		return;
	}
	sess->uid = pw->pw_uid;
	ftp_relply(sess,FTP_GIVEPWORD,"Please specify the password.");
}

static void do_pass(session_t *sess)
{
	struct passwd *pw = getpwuid(sess->uid);
	if( pw == NULL )
	{
		ftp_relply(sess,FTP_LOGINERR,"Login incorrect.");
		return;
	}
	// only root can do this
	struct spwd *sp = getspnam(pw->pw_name);
	if( sp == NULL )
	{
		ftp_relply(sess,FTP_LOGINERR,"Login incorrect.");
		return;	
	}

	// encrypt the sess passwd
	char *encrypt_pass = crypt(sess->cmd_arg, sp->sp_pwdp);
	if( strcmp(encrypt_pass,sp->sp_pwdp) != 0 )
	{
		ftp_relply(sess,FTP_LOGINERR,"Password incorrect.");	
		return;	
	}
	// login successful,set process egid and euid
	if( setegid(pw->pw_gid) < 0 )
	{
		ERR_EXIT("setegid");
	}

	if( seteuid(pw->pw_uid) < 0 )
	{
		ERR_EXIT("seteuid");
	}
	chdir(pw->pw_dir);
	ftp_relply(sess,FTP_LOGINOK,"Login successful.");
}

void ftp_relply(session_t *sess,int status,const char *text)
{
	char buf[MAX_LINE] = {0};
	sprintf(buf,"%d %s\r\n",status,text);
	writen(sess->ctrl_fd,buf,strlen(buf));
}