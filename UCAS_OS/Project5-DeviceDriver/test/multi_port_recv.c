#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <test.h>
#include <mthread.h>
#include <os.h>

#define MAX_RECV_CNT 32
#define MAX_RECV_SIZE 65
char recv_buffer[2][MAX_RECV_CNT * sizeof(EthernetFrame)];
size_t recv_length[2][MAX_RECV_CNT];

void multi_port_recver(int argc, long *argv[])
{
    int size = 3, position = 2, port = 0;
    int *real_arg = *argv;
    int my_tid;
    my_tid = real_arg[0];
    position = real_arg[1];
    port = real_arg[2];
    size = real_arg[3];
    
    sys_move_cursor(1, position);
    
    printf("[RECV TASK] start recv(num: %d, port: %x):                    ", size,port);

    sys_net_irq_mode(1);
    int ret = 0;
    ret = sys_net_recv(recv_buffer[my_tid], size * sizeof(EthernetFrame), size, recv_length[my_tid], port);
    sys_move_cursor(1, position + 1);
    printf("%d\n", ret);
    char *curr = recv_buffer[my_tid];
    for (int i = 0; i < 1; ++i) {
        for (int j = 0; j < (recv_length[my_tid][i] + 15) / 16; ++j) {
            for (int k = 0; k < 16 && (j * 16 + k < recv_length[my_tid][i]); ++k) {
                printf("%02x ", (uint32_t)(*(uint8_t*)curr));
                ++curr;
            }
            printf("\n");
        }
    }
    sys_exit();
}

int main(int argc, char *argv[])
{
    int print_location = 1, max_recv_size = 3;
    if(argc >= 1){
        max_recv_size = (int)atoi((char *)argv);
    }

    mthread_t recver[2];
    int args[2][4] = {{0,2,50001,max_recv_size},{1,10,58688,max_recv_size}};
    for(int i = 0; i < 2; i++){
        mthread_create(&recver[i], multi_port_recver, &args[i]);
    }

    return 0;
}