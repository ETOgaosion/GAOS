#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <fs.h>

#include <os.h>

static char buff[64];

int main(void)
{
    int i, j;
    int fd = fopen("1.txt", O_RD | O_WR);

    // write 'hello world!' * 10
    for (i = 0; i < 10; i++)
    {
        fwrite(fd, "hello world!\n", 13);
    }

    // read
    for (i = 0; i < 10; i++)
    {
        fread(fd, buff, 13);
        for (j = 0; j < 13; j++)
        {
            printf("%c", buff[j]);
        }
    }

    fclose(fd);
}