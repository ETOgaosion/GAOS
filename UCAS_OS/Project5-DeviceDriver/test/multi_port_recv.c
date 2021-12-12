#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <test.h>
#include <mthread.h>
#include <os.h>

#define MAX_RECV_CNT 32
#define MAX_RECV_SIZE 1024
char recv_buffer[MAX_RECV_CNT * sizeof(EthernetFrame)];
size_t recv_length[MAX_RECV_CNT];

void multi_port_recver(int argc, long *argv[])
{
    int size = MAX_RECV_SIZE, position = 2, port = 0;
    int *real_arg = *argv;

    position = real_arg[0];
    port = real_arg[1];
    
    sys_move_cursor(1, position);
    
    printf("[RECV TASK] start recv(%d):                    ", size);

    sys_net_irq_mode(1);
    int ret = sys_net_recv(recv_buffer, size * sizeof(EthernetFrame), size, recv_length, port);
    sys_move_cursor(1, position + 1);
    printf("%d\n", ret);
    char *curr = recv_buffer;
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < (recv_length[i] + 15) / 16; ++j) {
            for (int k = 0; k < 16 && (j * 16 + k < recv_length[i]); ++k) {
                printf("%02x ", (uint32_t)(*(uint8_t*)curr));
                ++curr;
            }
            printf("\n");
        }
    }
}

int main(int argc, char *argv[])
{
    int print_location = 1;
    if (argc >= 1) {
        print_location = (int) atoi((char *)argv);
    }

    mthread_t recver[2];
    int args[2][2] = {{2,50001},{10,58688}};
    for(int i = 0; i < 2; i++){
        mthread_create(&recver[i], multi_port_recver, &args[i]);
    }

    return 0;
}