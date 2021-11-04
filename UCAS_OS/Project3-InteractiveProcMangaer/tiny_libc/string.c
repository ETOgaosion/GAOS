#include <string.h>
#include <stdint.h>
#define ALIGN_DOWN(base, size)        ((base) & -((__typeof__ (base)) (size)))

#define PTR_ALIGN_DOWN(base, size) \
  ((__typeof__ (base)) ALIGN_DOWN ((uintptr_t) (base), (size)))


int strlen(const char *src)
{
    int i;
    for (i = 0; src[i] != '\0'; i++) {
    }
    return i;
}

void *memcpy(uint8_t *dest, const uint8_t *src, uint32_t len)
{
    for (; len != 0; len--) {
        *dest++ = *src++;
    }
    return dest;
}

void *memset(void *dest, uint8_t val, size_t len)
{
    uint8_t *dst = (uint8_t *)dest;

    for (; len != 0; len--) {
        *dst++ = val;
    }
    return dest;
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

char *strtok(char *substr, char *str, const char delim, int length)
{
    int len = strlen(str);
    int i;
    if (len == 0)
        return NULL;
    for (i = 0; i < len; i++){
        if (str[i] != delim){
            if(i < length){
                substr[i] = str[i];
            }
        }
        else{
            substr[i] = 0;
            return &str[i + 1];
        }
    }
    return &str[i + 1];
}