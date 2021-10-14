#ifndef STDLIB_H
#define STDLIB_H

/***
*long atol(char *nptr) - Convert string to long
*
*Purpose:
* Converts ASCII string pointed to by nptr to binary.
* Overflow is not detected.
*
*Entry:
* nptr = ptr to string to convert
*
*Exit:
* return long int value of the string
*
*Exceptions:
* None - overflow is not detected.
*
*******************************************************************************/
int isspace(int x)
{
    if(x==' '||x=='\t'||x=='\n'||x=='\f'||x=='\b'||x=='\r')
    return 1;
    else 
    return 0;
}
int isdigit(int x)
{
    if(x<='9'&&x>='0')        
    return 1;
    else
    return 0;

}

long atol(const char *nptr)
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

int atoi(const char *nptr)
{
	return (int)atol(nptr);
}

#endif