#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"

typedef struct stu
{
	char sno[12];
	char name[32];
	int age;
} stu_t ;

typedef struct stu1
{
	int sno;
	char name[32];
	int age;
} stu_t1 ;

unsigned int hash_str(unsigned int buckets,void *key)
{
	char *str = (char *)key;
	unsigned int index = 0;

	while( *str )
	{
		index = *str + 4*index;
		++str;
	}
	return index % buckets;
}

unsigned int hash_int(unsigned int buckets,void *key)
{
	int *num = (int *)key;
	return (*num) % buckets;
}

int main(int argc,char *argv[])
{
	/*
	stu_t stu_array[] = 
	{
		{ "1234","AAAA",20 },
		{ "4567","BBBB",19 },
		{ "6789","CCCC",18 },
	};
	
	hash_t *hash = hash_alloc(256,hash_str);

	int size = sizeof(stu_array) / sizeof(stu_array[0]);
	int i;
	for( i = 0; i < size; ++i )
	{
		hash_add_entry(hash,stu_array[i].sno,strlen(stu_array[i].sno)
			,&stu_array[i],sizeof(stu_array[i]));
	}

	stu_t *s = (stu_t *)hash_lookup_entry(hash,"4567",strlen("4567"));
	if( s )
	{
		printf("%s %s %d\n", s->sno,s->name,s->age);
	}
	else
	{
		printf("not found\n");
	}

	hash_free_entry(hash,"1234",strlen("1234"));

	s = (stu_t *)hash_lookup_entry(hash,"1234",strlen("1234"));

	if( s )
	{
		printf("%s %s %d\n", s->sno,s->name,s->age);
	}
	else
	{
		printf("not found\n");
	}
	*/

	stu_t1 stu_array[] = 
	{
		{ 1234,"AAAA",20 },
		{ 4567,"BBBB",19 },
		{ 6789,"CCCC",18 },
	};

	hash_t *hash = hash_alloc(256,hash_int);

	int size = sizeof(stu_array) / sizeof(stu_array[0]);
	int i;
	for( i = 0; i < size; ++i )
	{
		hash_add_entry(hash,&(stu_array[i].sno),sizeof(stu_array[i].sno)
			,&stu_array[i],sizeof(stu_array[i]));
	}

	int sno = 4567;
	stu_t1 *s = (stu_t1 *)hash_lookup_entry(hash,&sno,sizeof(sno));
	if( s )
	{
		printf("%d %s %d\n", s->sno,s->name,s->age);
	}
	else
	{
		printf("not found\n");
	}

	sno = 1234;
	hash_free_entry(hash,&sno,sizeof(sno));

	s = (stu_t1 *)hash_lookup_entry(hash,&sno,sizeof(sno));

	if( s )
	{
		printf("%d %s %d\n", s->sno,s->name,s->age);
	}
	else
	{
		printf("not found\n");
	}

	return 0;
}

