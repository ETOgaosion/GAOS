#ifndef INCLUDE_MAIL_BOX_
#define INCLUDE_MAIL_BOX_

#define MAX_MBOX_LENGTH 64
#define MAX_MBOX_OP_NUM 16

// TODO: please define mailbox_t;
// mailbox_t is just an id of kernel's mail box.
typedef int mailbox_t;

typedef struct mbox_arg{
    int valid;
    void *msg;
    int msg_length;
} mbox_arg_t;

int mbox_op(mailbox_t *handle, void *arg, int op);
mailbox_t mbox_open(char *name);
void mbox_close(mailbox_t *mailbox);
int mbox_send(mailbox_t *mailbox, void *msg, int msg_length);
int mbox_recv(mailbox_t *mailbox, void *msg, int msg_length);

#endif
