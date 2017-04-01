/*
 *  linux/lib/string.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#ifndef __GNUC__
#error I want gcc!
#endif

#include <linux/types.h>

#define extern
#define inline
#define __LIBRARY__
#include <linux/string.h>

extern char * ___strtok;

// From 2.4 kernel
size_t strspn(const char *s, const char *accept)
{
	const char *p;
	const char *a;
	size_t count = 0;

	for (p = s; *p != '\0'; ++p) {
		for (a = accept; *a != '\0'; ++a) {
			if (*p == *a)
				break;
		}
		if (*a == '\0')
			return count;
		++count;
	}

	return count;
}

// From 2.4 kernel
char * strpbrk(const char * cs,const char * ct)
{
	const char *sc1,*sc2;

	for( sc1 = cs; *sc1 != '\0'; ++sc1) {
		for( sc2 = ct; *sc2 != '\0'; ++sc2) {
			if (*sc1 == *sc2)
				return (char *) sc1;
		}
	}
	return NULL;
}

// From 2.4 kernel
char * strtok(char * s,const char * ct)
{
	char *sbegin, *send;

	sbegin  = s ? s : ___strtok;
	if (!sbegin) {
		return NULL;
	}
	sbegin += strspn(sbegin,ct);
	if (*sbegin == '\0') {
		___strtok = NULL;
		return( NULL );
	}
	send = strpbrk( sbegin, ct);
	if (send && *send != '\0')
		*send++ = '\0';
	___strtok = send;
	return (sbegin);
}

// From 2.4 kernel
char * strsep(char **s, const char * ct)
{
	char *sbegin=*s;
	if (!sbegin) 
		return NULL;
	
	sbegin += strspn(sbegin,ct);
	if (*sbegin == '\0') 
		return NULL;
	
	*s = strpbrk( sbegin, ct);
	if (*s && **s != '\0')
		*(*s)++ = '\0';
	return (sbegin);
}
