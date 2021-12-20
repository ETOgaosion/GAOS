#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <test.h>

#include <os.h>

#define MAX_RECV_CNT 32
char recv_buffer[MAX_RECV_CNT * sizeof(EthernetFrame)];
size_t recv_length[MAX_RECV_CNT];

int main(int argc, char *argv[])
{
    int mode = 0;
    int size = 1;
    sys_move_cursor(1, 1);
    if(argc > 0) {
        size = atol((char *)argv);
        printf("%d \n", size);
    }
    if(argc > 1) {        
        if (strcmp((char *)argv + SHELL_ARG_MAX_LENGTH, "1") == 0) {
            mode = 1;
        }
    }

    sys_net_irq_mode(mode);

    printf("[RECV TASK] start recv(%d):                    ", size);

    int ret = sys_net_recv(recv_buffer, size * sizeof(EthernetFrame), size, recv_length,0);
    sys_move_cursor(1, 3);
    char *curr = recv_buffer;
    for (int j = 0; j < recv_length[31] / 16 + 1; ++j) {
        for (int k = 0; k < 16 && (j * 16 + k < recv_length[size - 1]); ++k) {
            printf("%02x ", (uint32_t)(*(uint8_t*)curr));
            ++curr;
        }
        printf("\n");
    }

    return 0;
}