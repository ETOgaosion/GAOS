#ifndef STRING_H
#define STRING_H

#include <stdint.h>
#include <stddef.h>

void *memcpy(uint8_t *dest, const uint8_t *src, uint32_t len);
void *memset(void *dest, uint8_t val, size_t len);
int memcmp(const void *ptr1, const void* ptr2, uint32_t num);

int strcmp(const char *str1, const char *str2);
char *strcpy(char *dest, const char *src);
char *strcat(char *dest, const char *src);
int strlen(const char *src);
char *strtok(char *substr, char *str, const char delim, int length);

long strtol(const char *nptr, char **endptr, register int base);

#endif /* STRING_H */
