#include <os/string.h>
#include <sbi.h>

int strlen(const char *src)
{
    int i;
    for (i = 0; src[i] != '\0'; i++) {
    }
    return i;
}

void memcpy(uint8_t *dest, const uint8_t *src, uint32_t len)
{
    for (; len != 0; len--) {
        *dest++ = *src++;
    }
}

void memset(void *dest, uint8_t val, uint32_t len)
{
    uint8_t *dst = (uint8_t *)dest;

    for (; len != 0; len--) {
        *dst++ = val;
    }
}

void bzero(void *dest, uint32_t len) { memset(dest, 0, len); }

int strcmp(const char *str1, const char *str2)
{
    while (*str1 && *str2) {
        if (*str1 != *str2) {
            return (*str1) - (*str2);
        }
        ++str1;
        ++str2;
    }
    return (*str1) - (*str2);
}

char *strcpy(char *dest, const char *src)
{
    char *tmp = dest;

    while (*src) {
        *dest++ = *src++;
    }

    *dest = '\0';

    return tmp;
}

char *strcat(char *dest, const char *src)
{
    char *tmp = dest;

    while (*dest != '\0') { dest++; }
    while (*src) {
        *dest++ = *src++;
    }

    *dest = '\0';

    return tmp;
}

void itos(int i,int base, char *dest){
    *dest-- = '\0';
    while (i != 0)
    {
        if(base > 10 && (i % base > 10)){
            *dest-- = 'a' + i % base - 10;
        }
        else{
            *dest-- = '0' + i % base;
        }
        i /= base;
    }  
}

void outputstr(uint64_t src){
    uint64_t a = src;
    if(a == 0){
        sbi_console_putchar('0');
    }
    while (a > 0)
    {
        /* code */
        if(a%16 >= 10){
            sbi_console_putchar('a'+a%16 - 10);
        }
        else{
            sbi_console_putchar('0' + a%16);
        }
        a /= 16;
    }
}