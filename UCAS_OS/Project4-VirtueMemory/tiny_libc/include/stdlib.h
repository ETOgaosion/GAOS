#ifndef STDLIB_H
#define STDLIB_H

#include <stdint.h>
#include <stddef.h>

#define RAND_MAX (INT32_MAX)

void srand(unsigned seed);
int rand();

static inline int isspace(int x)
{
    if(x==' '||x=='\t'||x=='\n'||x=='\f'||x=='\b'||x=='\r')
    return 1;
    else 
    return 0;
}
static inline int isdigit(int x)
{
    if(x<='9'&&x>='0')        
    return 1;
    else
    return 0;

}

static inline long atol(const char *nptr)
{
	int c; /* current char */
	long total; /* current total */
	int sign; /* if ''-'', then negative, otherwise positive */
 
	/* skip whitespace */
	while (isspace((int)(unsigned char)*nptr) )
		++nptr;
 
	c = (int)(unsigned char)*nptr++;
	sign = c; /* save sign indication */
 
	if (c == '-' || c == '+')
		c = (int)(unsigned char)*nptr++; /* skip sign */
 
	total = 0;
	while (isdigit(c)) 
	{
		total = 10 * total + (c - '0'); /* accumulate digit */
		c = (int)(unsigned char)*nptr++; /* get next char */
	}
	if (sign == '-')
		return -total;
	else
		return total; /* return result, negated if necessary */
}

static inline int atoi(const char *nptr)
{
	return (int)atol(nptr);
}

#endif /* STDLIB_H */
