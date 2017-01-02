#ifndef __PARSECONF_H__
#define __PARSECONF_H__

struct  parseconf_bool_setting
{
	const char *p_setting_name;
	int *p_variable;
};

struct parseconf_uint_setting
{
	const char *p_setting_name;
	unsigned int *p_variable;
};

struct parseconf_str_setting
{
	const char *p_setting_name;
	const char **p_variable;
};

void parseconf_load_file(const char *path);

void parseconf_load_setting(const char *setting);

#endif /* __PARSECONF_H__ */