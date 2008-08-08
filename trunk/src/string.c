/*
	String Operations
	Author: Roy Reznik

	Modified by:
		Roy Reznik 08/08/08
*/

#include "common.h"
#include "string.h"

/* strcmp consts */
#define STRINGS_EQUAL (0)
#define STRING1_GREATER (1)
#define STRING2_GREATER (-1)

size_t strlen(const char *str)
{
	unsigned int length = 0;

	while (0 != *str)
	{
		length++;
		str++;
	}

	return length;
}

char * strcpy(char *strDestination, const char *strSource)
{
	size_t i = 0;

	for (i = 0;'\0' != *strSource;i++)
	{
		strDestination[i] = strSource[i];
	}

	strDestination[i] = '\0';

	return strDestination;
}

char * strcat(char *strDestination, const char *strSource)
{
	size_t uiDestLength = 0;
	char * destOriginal = strDestination;

	uiDestLength = strlen(strDestination);

	strDestination += uiDestLength;

	while (0 != *strSource)
	{
		*strDestination = *strSource;
		strDestination++;
		strSource++;
	}

	*strDestination = '\0';

	return destOriginal;
}

char * strchr(const char *str, int c)
{
	while (((char)(c) != *str) && (0 != *str))
	{
		str++;
	}

	if ((char)(c) == *str)
	{
		return str;
	}

	return NULL;
}

int strcmp(const char *string1, const char *string2)
{
	while ((*string1 == *string2) && (0 != *string1) && (0 != *string2))
	{
		string1++;
		string2++;
	}

	if (*string1 == *string2)
	{
		return STRINGS_EQUAL;
	}

	if (*string1 > *string2)
	{
		return STRING1_GREATER;
	}

	return STRING2_GREATER;
}

int strncmp(const char *string1, const char *string2, size_t count)
{
	size_t counter = 0;

	while ((*string1 == *string2) && (0 != *string1) && (0 != *string2) && (counter < count))
	{
		string1++;
		string2++;
		counter++;
	}

	if (*string1 == *string2)
	{
		return STRINGS_EQUAL;
	}

	if (*string1 > *string2)
	{
		return STRING1_GREATER;
	}

	return STRING2_GREATER;
}


char * strstr(const char *str, const char *strSearch)
{
	size_t strSearchLength = strlen(strSearch);

	while (0 != *(str + strSearchLength))
	{
		if (0 == strncmp(str, strSearch, strSearchLength))
		{
			return str;
		}

		str++;
	}

	return NULL;
}