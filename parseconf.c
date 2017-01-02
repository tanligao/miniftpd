#include "parseconf.h"
#include "common.h"
#include "tunable.h"
#include "str.h"

static struct parseconf_bool_setting parseconf_bool_array[] = 
{
	{ "pasv_enable",	&tunable_pasv_enable },
	{ "port_enable",		&tunable_port_enable },
	{  NULL,		NULL }
};

static struct parseconf_uint_setting parseconf_uint_array[] = 
{
	{ "listen_port",		&tunable_listen_port },
	{ "max_clients",		&tunable_max_clients },
	{ "max_per_ip",		&tunable_max_per_ip },
	{ "accept_timeout",	&tunable_accept_timeout },
	{ "connect_timeout",	&tunable_connect_timeout },
	{ "idle_session_timeout",&tunable_idle_session_timeout},
	{ "data_connection_timeout",&tunable_data_connection_timeout },
	{ "local_umask",	&tunable_local_umask },
	{ "upload_max_rate",	&tunable_upload_max_rate},
	{ "download_max_rate",&tunable_download_max_rate},
	{ NULL,			NULL }
};

static struct parseconf_str_setting parseconf_str_array[] = 
{
	{ "listen_adress",	&tunable_listen_adress},
	{ NULL,			NULL }
};

void parseconf_load_file(const char *path)
{
	FILE *fp = fopen(path,"r");
	if( fp == NULL )
		ERR_EXIT("fopen");

	char setting_line[MAX_LINE];
	while( fgets(setting_line,MAX_LINE,fp) != NULL )
	{
		if( strlen(setting_line) == 0 || setting_line[0] == '#' || str_all_space(setting_line) )
			continue;

		str_trim_crlf(setting_line);
		parseconf_load_setting(setting_line);
		memset(setting_line,0,MAX_LINE);
	}
}

void parseconf_load_setting(const char *setting)
{
	char setting_name[MAX_SET_NAME_LEN] = {0};
	char setting_value[MAX_SET_VALUE_LEN] = {0};

	// jump space
	while( isspace(*setting) )
		++setting;

	str_split(setting,setting_name,setting_value,'=');

	if( strlen(setting_value) == 0 )
	{
		fprintf(stderr, "miss value in config file for: %s\n", setting_name);
		exit(EXIT_FAILURE);
	}

	struct parseconf_str_setting *p_str_setting = parseconf_str_array;
	while( p_str_setting->p_setting_name != NULL )
	{
		if( strcmp(setting_name,p_str_setting->p_setting_name) == 0 )
		{
			const char **p_cur_setting = p_str_setting->p_variable;
			if( *p_cur_setting )
				free((char*)*p_cur_setting);
			*p_cur_setting = strdup(setting_value);
			return;
		}
		++p_str_setting;
	}
	
	struct parseconf_bool_setting *p_bool_setting = parseconf_bool_array;
	while( p_bool_setting->p_setting_name != NULL )
	{
		if( strcmp(setting_name,p_bool_setting->p_setting_name) == 0 )
		{
			str_upper(setting_value);
			if( strcmp(setting_value,"YES") == 0 || 
				strcmp(setting_value,"TRUE") == 0 ||
				strcmp(setting_value,"1") == 0 )
			{
				*(p_bool_setting->p_variable) = 1;
			}
			else if( strcmp(setting_value,"NO") == 0 ||
				strcmp(setting_value,"FALSE") || 
				strcmp(setting_value,"0") == 0 )
			{
				*(p_bool_setting->p_variable) = 0;	
			}
			else
			{
				fprintf(stderr, "miss value in config file for: %s\n", setting_name);
				exit(EXIT_FAILURE);
			}
			return;
		}
		++p_bool_setting;
	}
	
	struct parseconf_uint_setting *p_uint_setting = parseconf_uint_array;
	while( p_uint_setting->p_setting_name != NULL )
	{
		if( strcmp(setting_name,p_uint_setting->p_setting_name) == 0 )
		{
			if( setting_value[0] == '0' )
			{
				*(p_uint_setting->p_variable) = str_octal_to_uint(setting_value);
			}
			else
			{
				*(p_uint_setting->p_variable) = atoi(setting_value);
			}
			return;
		}
		++p_uint_setting;
	}	
}