#include <net.h>
#include <os/string.h>
#include <screen.h>

#include <os/sched.h>
#include <os/mm.h>
#include <tasks.h>

EthernetFrame rx_buffers[RXBD_CNT];
int rx_ports[RXBD_CNT];
uint32_t rx_len[RXBD_CNT];
uint32_t rx_len_cpy[RXBD_CNT];
EthernetFrame tx_buffer;

int rx_owner = 0;

int net_poll_mode;

volatile int rx_curr = 0, rx_tail = 0;

long k_net_recv(uintptr_t addr, size_t length, int num_packet, size_t* frLength, int port)
{
    // TODO: 
    // receive packet by calling network driver's function
    // wait until you receive enough packets(`num_packet`).
    // maybe you need to call drivers' receive function multiple times ?
    uintptr_t addr_tmp = addr;
    size_t *frLength_tmp = frLength;
    #ifdef LISTEN_PORT
    (*current_running)->listen_port = port;
    #endif
    if(!rx_owner){
        rx_owner = (*current_running)->pid;
        EmacPsRecv(&EmacPsInstance, kva2pa(rx_buffers),/* num_packet <= 32 ? num_packet :*/ 32);
    }
    addr = addr_tmp;
    frLength = frLength_tmp;
    if(rx_owner == (*current_running)->pid){
        EmacPsWaitRecv(&EmacPsInstance, num_packet, rx_len,rx_ports);
    }
    else{
        EmacPsWaitRecv(&EmacPsInstance, num_packet, rx_len_cpy,rx_ports);
    }
    // Copy to user
    int i = 0, j = 0;
    while(i < (num_packet >= 32 ? 32 : num_packet)){
        if((*current_running)->listen_port > 0){
            if(rx_ports[i] == (*current_running)->listen_port){
                memcpy((uint8_t *)addr, rx_buffers + i, rx_len[j]);
                if(rx_owner == (*current_running)->pid){
                    *frLength = rx_len[j];
                    frLength ++;
                    addr += rx_len[j];
                }
                else{
                    *frLength = rx_len_cpy[j];
                    frLength ++;
                    addr += rx_len_cpy[j];
                }
                i++,j++;
            }
            else{
                i++;
            }
        }
        else{
            memcpy((uint8_t *)addr, rx_buffers + i, rx_len[j]);
            *frLength = rx_len[j];
            frLength ++;
            addr += rx_len[j];
            i++, j++;
        }
    }
    return 1;
}

void k_net_send(uintptr_t addr, size_t length)
{
    // TODO:
    // send all packet
    // maybe you need to call drivers' send function multiple times ?
    // Copy to `buffer'
    memcpy((uint8_t *)&tx_buffer, addr, length);
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
