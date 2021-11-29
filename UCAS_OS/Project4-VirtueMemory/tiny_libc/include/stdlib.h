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

static inline int isalpha(int x)
{
    if((x<='f'&&x>='a') || (x<='F'&&x>='A'))        
    return 1;
    else
    return 0;

}

static inline int isupper(int x)
{
	if(x>='A' && x<='Z'){
		return 1;
	}
	return 0;
}

static inline long atol(const char* str)
{
    int base = 10;
    if ((str[0] == '0' && str[1] == 'x') ||
        (str[0] == '0' && str[1] == 'X')) {
        base = 16;
        str += 2;
    }
    long ret = 0;
    while (*str != '\0') {
        if ('0' <= *str && *str <= '9') {
            ret = ret * base + (*str - '0');
        } else if (base == 16) {
            if ('a' <= *str && *str <= 'f'){
                ret = ret * base + (*str - 'a' + 10);
            } else if ('A' <= *str && *str <= 'F') {
                ret = ret * base + (*str - 'A' + 10);
            } else {
                return 0;
            }
        } else {
            return 0;
        }
        ++str;
    }
    return ret;
}



static inline int atoi(const char *nptr)
{
	int c; /* current char */
	int total; /* current total */
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

#endif /* STDLIB_H */
