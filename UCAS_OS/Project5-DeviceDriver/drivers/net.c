#include <net.h>
#include <os/string.h>
#include <screen.h>

#include <os/sched.h>
#include <os/mm.h>
#include <tasks.h>

EthernetFrame rx_buffers[RXBD_CNT];
EthernetFrame tx_buffer;
uint32_t rx_len[RXBD_CNT];

int rx_allocated = 0;

int net_poll_mode;

volatile int rx_curr = 0, rx_tail = 0;

long k_net_recv(uintptr_t addr, size_t length, int num_packet, size_t* frLength, int port)
{
    // TODO: 
    // receive packet by calling network driver's function
    // wait until you receive enough packets(`num_packet`).
    // maybe you need to call drivers' receive function multiple times ?
    #ifdef LISTEN_PORT
    (*current_running)->listen_port = port;
    #endif
    while(num_packet > 0)
    {
        int num = (num_packet > 32) ? 32 : num_packet;
        if(!rx_allocated){
            EmacPsRecv(&EmacPsInstance, kva2pa(rx_buffers), num);
            rx_allocated = 1;
        }
        EmacPsWaitRecv(&EmacPsInstance, num, rx_len);
        rx_allocated = 0;
        // Copy to user
        for (int i = 0; i < num; i++){
            memcpy(addr, rx_buffers + i, rx_len[i]);
            *frLength = rx_len[i];
            frLength ++;
            addr += rx_len[i];
        }
        num_packet -= 32;
    }
    return 1;
}

void k_net_send(uintptr_t addr, size_t length)
{
    // TODO:
    // send all packet
    // maybe you need to call drivers' send function multiple times ?
    // Copy to `buffer'
    memcpy(&tx_buffer, addr, length);
    // send packet
    EmacPsSend(&EmacPsInstance, kva2pa(&tx_buffer), length);
    EmacPsWaitSend(&EmacPsInstance);
}

void k_net_irq_mode(int mode)
{
    // TODO:
    // turn on/off network driver's interrupt mode
    EmacPsSetIRQMode(&EmacPsInstance, mode);
}
