/*
	String Operations
	Author: Roy Reznik

	Modified by:
		Roy Reznik 08/08/08
*/
#include "common.h"

size_t strlen(const char *str);
char * strcpy(char *strDestination, const char *strSource);
char * strcat(char *strDestination, const char *strSource);
char * strchr(const char *str, int c);
int strcmp(const char *string1, const char *string2);
int strncmp(const char *string1, const char *string2, size_t count);
char * strstr(const char *str, const char *strSearch);
