#ifndef NET_H
#define NET_H

#include <type.h>
#include <emacps/xemacps_example.h>
#include <emacps/xemacps.h>

extern EthernetFrame rx_buffers[RXBD_CNT];
extern EthernetFrame tx_buffer;
extern uint32_t rx_len[RXBD_CNT];

long k_net_recv(uintptr_t addr, size_t length, int num_packet, size_t* frLength);
void k_net_send(uintptr_t addr, size_t length);
void k_net_irq_mode(int mode);

extern int net_poll_mode;

#endif //NET_H
