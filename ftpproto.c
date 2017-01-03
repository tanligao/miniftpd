#include "ftpproto.h"
#include "common.h"
#include "sysutil.h"
#include "str.h"
#include "ftpcodes.h"

// 访问控制命令
static void do_user(session_t *sess);
static void do_pass(session_t *sess);
static void do_cwd(session_t *sess);
static void do_cdup(session_t *sess);
static void do_quit(session_t *sess);

// 传输参数命令
static void do_port(session_t *sess);
static void do_pasv(session_t *sess);
static void do_type(session_t *sess);
static void do_stru(session_t *sess);
static void do_mode(session_t *sess);

// 服务命令
static void do_retr(session_t *sess);
static void do_stor(session_t *sess);
static void do_appe(session_t *sess);
static void do_list(session_t *sess);
static void do_nlst(session_t *sess);
static void do_rest(session_t *sess);
static void do_abor(session_t *sess);
static void do_pwd(session_t *sess);
static void do_mkd(session_t *sess);
static void do_rmd(session_t *sess);
static void do_dele(session_t *sess);
static void do_rnfr(session_t *sess);
static void do_rnto(session_t *sess);
static void do_site(session_t *sess);
static void do_syst(session_t *sess);
static void do_feat(session_t *sess);
static void do_size(session_t *sess);
static void do_stat(session_t *sess);
static void do_noop(session_t *sess);
static void do_help(session_t *sess);

static ftpcmd_t ctrl_cmds_map[] =
{
	// 访问控制映射
	{ "USER",	do_user },
	{ "PASS",	do_pass },
	{ "CWD",	do_cwd },
	{ "XCWD",	do_cwd },
	{ "CDUP",	do_cdup },
	{ "XDUP",	do_cdup},
	{ "QUIT",	do_quit },
	{ "ACCT",	NULL },
	{ "SMNT",	NULL },
	{ "REIN",	NULL },
	
	// 传输参数命令
	{ "PORT",	do_port },
	{ "PASV",	do_pasv },
	{ "TYPE",	do_type },
	{ "STRU",	do_stru },
	{ "MODE",	do_mode },

	// 服务命令
	{ "RETR",	do_retr },
	{ "STOR",	do_stor },
	{ "APPE",	do_appe },
	{ "LIST",		do_list },
	{ "NLST",	do_nlst },
	{ "REST",	do_rest },
	{ "ABOR",	do_abor },
	{ "\377\364\377\362ABOR",do_abor },
	{ "PWD",	do_pwd },
	{ "XPWD",	do_pwd },
	{ "MKD",	do_mkd },
	{ "XMKD",	do_mkd },
	{ "RMD",	do_rmd },
	{ "XRMD",	do_rmd },
	{ "DELE",	do_dele },
	{ "RNFR",	do_rnfr },
	{ "RNTO",	do_rnto },
	{ "SITE",		do_site },
	{ "SYST",	do_syst },
	{ "FEAT",	do_feat },
	{ "SIZE",	do_size },
	{ "STAT",	do_stat },
	{ "NOOP",	do_noop },
	{ "HELP",	do_help },
	{ "STOU",	NULL },
	{ "ALLO",	NULL }
};

void ftp_relply(session_t *sess,int status,const char *text);
void ftp_lrelply(session_t *sess,int status,const char *text);

void handle_child(session_t *sess)
{
	ftp_relply(sess,FTP_GREET,"(miniftpd 0.1)");
	int ret;
	while(1)
	{
		memset(sess->cmdline,0,MAX_COMMAND_LINE);
		memset(sess->cmd,0,MAX_COMMAND);
		memset(sess->cmd_arg,0,MAX_ARG);
		ret = readline(sess->ctrl_fd,sess->cmdline,MAX_COMMAND_LINE);

		if( ret == -1 )
			ERR_EXIT("readline");
		else if( ret == 0 )
			exit(EXIT_SUCCESS);
		// 解析读取到的FTP命令与参数，处理FTP命令，然后发送给父进程
		//printf("%s", sess->cmdline);
		str_trim_crlf(sess->cmdline);
		//printf("%s\n", sess->cmdline);
		
		// 将命令与参数分割
		str_split(sess->cmdline,sess->cmd,sess->cmd_arg,' ');
		//printf("%s %s\n",sess->cmd,sess->cmd_arg);
		// 将命令全部转化为大写
		str_upper(sess->cmd);

		int i;
		int map_size = sizeof(ctrl_cmds_map)/sizeof(ctrl_cmds_map[0]);
		for( i =0; i < map_size; ++i )
		{
			if( strcmp(ctrl_cmds_map[i].cmd,sess->cmd) == 0 )
			{
				if( ctrl_cmds_map[i].cmd_func != NULL )
				{
					ctrl_cmds_map[i].cmd_func(sess);
				}
				else
				{
					ftp_relply(sess,FTP_COMMANDNOTIMPL,"Unimplement command.");
				}
				break;
			}
		}
		if( i == map_size )
		{
			ftp_relply(sess,FTP_BADCMD,"Unknown command.");
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

void do_cwd(session_t *sess)
{

}

void do_cdup(session_t *sess)
{

}

void do_quit(session_t *sess)
{

}

void do_port(session_t *sess)
{

}

void do_pasv(session_t *sess)
{

}

void do_type(session_t *sess)
{
	// switch to ascii mode
	if( strcmp(sess->cmd_arg,"A") == 0 )
	{
		sess->is_ascii = 1;
		ftp_relply(sess,FTP_TYPEOK,"Switch to ASCII mode");
	}
	else if( strcmp(sess->cmd_arg,"I") == 0 )
	{
		sess->is_ascii = 0;
		ftp_relply(sess,FTP_TYPEOK,"Switch to Binary mode");
	}
	else
	{
		ftp_relply(sess,FTP_BADCMD,"Unrecognised command.");
	}
}

void do_stru(session_t *sess)
{

}

void do_mode(session_t *sess)
{

}

void do_retr(session_t *sess)
{

}

void do_stor(session_t *sess)
{

}

void do_appe(session_t *sess)
{

}

void do_list(session_t *sess)
{

}

void do_nlst(session_t *sess)
{

}

void do_rest(session_t *sess)
{

}

void do_abor(session_t *sess)
{

}

void do_pwd(session_t *sess)
{
	char cur_dir[MAX_LINE+1] = {0};
	char text[MAX_LINE] = {0};
	getcwd(cur_dir,MAX_LINE);
	sprintf(text,"\"%s\"",cur_dir);
	ftp_relply(sess,FTP_PWDOK,text);
}

void do_mkd(session_t *sess)
{

}

void do_rmd(session_t *sess)
{

}

void do_dele(session_t *sess)
{

}

void do_rnfr(session_t *sess)
{

}

void do_rnto(session_t *sess)
{

}

void do_site(session_t *sess)
{

}

void do_syst(session_t *sess)
{
	ftp_relply(sess,FTP_SYSTOK,"UNIX Type: L8");
}

void do_feat(session_t *sess)
{
	ftp_lrelply(sess,FTP_FEAT,"Features:");
	writen(sess->ctrl_fd,"EPRT\r\n",strlen("EPRT\r\n"));
	writen(sess->ctrl_fd,"EPSV\r\n",strlen("EPSV\r\n"));
	writen(sess->ctrl_fd,"MDTM\r\n",strlen("MDTM\r\n"));
	writen(sess->ctrl_fd,"PASV\r\n",strlen("PASV\r\n"));
	writen(sess->ctrl_fd,"REST STREAM\r\n",strlen("REST STREAM\r\n"));
	writen(sess->ctrl_fd,"SIZE\r\n",strlen("SIZE\r\n"));
	writen(sess->ctrl_fd,"TVFS\r\n",strlen("TVFS\r\n"));
	writen(sess->ctrl_fd,"UTF8\r\n",strlen("UTF8\r\n"));

	ftp_relply(sess,FTP_FEAT,"End");
}

void do_size(session_t *sess)
{

}

void do_stat(session_t *sess)
{

}

void do_noop(session_t *sess)
{

}

void do_help(session_t *sess)
{

}

void ftp_relply(session_t *sess,int status,const char *text)
{
	char buf[MAX_LINE] = {0};
	sprintf(buf,"%d %s\r\n",status,text);
	writen(sess->ctrl_fd,buf,strlen(buf));
}

void ftp_lrelply(session_t *sess,int status,const char *text)
{
	char buf[MAX_LINE] = {0};
	sprintf(buf,"%d-%s\r\n",status,text);
	writen(sess->ctrl_fd,buf,strlen(buf));
}

int list_common(void)
{
	DIR *dir = opendir(".");
	if( dir == NULL )
	{
		return 0;
	}
	struct dirent *dt;
	struct stat sbuf;
	while( (dt = readdir(dir)) != NULL )
	{
		if( lstat(dt->d_name,&sbuf) < 0 )
		{
			continue;
		}
		if( dt->d_name[0] == '.' )
		{
			continue;
		}
		char perms[ ] = "----------";
		mode_t mode = sbuf.st_mode;
		perms[0] = '?';
		switch( mode &  S_IFMT )
		{
		case S_IFREG:
			perms[0] = '-';
			break;
		case S_IFDIR:
			perms[0] = 'd';			
			break;
		case S_IFBLK:
			perms[0] = 'b';
			break;
		case S_IFLNK:
			perms[0] = 'l';
			break;
		case S_IFCHR:
			perms[0] = 'c';
			break;
		case S_IFSOCK:
			perms[0] = 's';
			break;
		case S_IFIFO:
			perms[0] = 'p';
			break;
		default:
			break;
		}

           		if( mode & S_IRUSR )
           		{
           			perms[1] = 'r';
           		}
           		if( mode & S_IWUSR )
           		{
           			perms[2] = 'w';
           		}
           		if( mode & S_IXUSR )
           		{
           			perms[3] = 'x';
           		}
           		if( mode & S_IRGRP )
           		{
           			perms[4] = 'r';
           		}
           		if( mode & S_IWGRP )
           		{
           			perms[5] = 'w';
           		}
           		if( mode & S_IXGRP )
           		{
           			perms[6] = 'x';
           		}
           		if( mode & S_IROTH )
           		{
           			perms[7] = 'r';
           		}
           		if( mode & S_IWOTH )
           		{
           			perms[8] = 'w';
           		}
           		if( mode & S_IXOTH )
           		{
           			perms[9] = 'x';
           		}
           		// special perms
           		if( mode & S_ISUID )
           		{
           			perms[3] = (perms[3] == 'x' ? 's' : 'S');
           		}
           		if( mode & S_ISGID )
           		{
           			perms[6] = (perms[6] == 'x' ? 's' : 'S');
           		}
           		if( mode & S_ISVTX )
           		{
           			perms[9] = (perms[9] == 'x' ? 's' : 'S');
           		}

           		char buf[MAX_LINE] = {0};
           		int off = 0;
           		off += sprintf(buf,"%s ",perms);
           		off += sprintf(buf + off,"%3d %-8d  %-8d",(unsigned int)sbuf.st_nlink,(unsigned int)sbuf.st_uid,(unsigned int)sbuf.st_gid);
           		off += sprintf(buf + off, "%8lu ",(unsigned long)sbuf.st_size);

           		const char *p_data_format = "%b %e %H:%M";
           		struct timeval tv;
           		gettimeofday(&tv,NULL);
           		time_t local_time = tv.tv_sec;
           		if( sbuf.st_mtime > local_time || (local_time - sbuf.st_mtime) > HALF_YEAR_SEC)
           		{
           			p_data_format = "%b %e    %Y";
           		}
           		char databuf[64] = {0};
           		struct tm* p_tm = localtime(&local_time);
           		strftime(databuf,sizeof(databuf),p_data_format,p_tm);
		
		off += sprintf(buf + off,"%s ",databuf);
		// link file
		if( S_ISLNK(sbuf.st_mode) )
		{
			char tmp[MAX_LINE] = {0};
			readlink(dt->d_name,tmp,sizeof(tmp));
			sprintf(buf + off,"%s -> %s\r\n",dt->d_name,tmp);
		}
		else
		{
			sprintf(buf + off,"%s\r\n",dt->d_name);
		}
		printf("%s", buf);
	}

	closedir(dir);

	return 1;
}