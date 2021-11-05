#include <mailbox.h>
#include <string.h>
#include <sys/syscall.h>

mbox_arg_t mbox_args[MAX_MBOX_OP_NUM] = {{.valid = 0, .msg = (void*)0, .msg_length = 0}};

int mbox_op(mailbox_t *handle, void *arg, int op){
    return sys_commop(handle,(int *)arg,op + 7);
}

mailbox_t mbox_open(char *name)
{
    // TODO:
    return mbox_op((mailbox_t *)name,NULL,0);
}

void mbox_close(mailbox_t *mailbox)
{
    // TODO:
    mbox_op(mailbox,NULL,1);
}

static int find_arg_t(){
    for (int i = 0; i < MAX_MBOX_OP_NUM; i++)
    {
        if(mbox_args[i].valid == 0){
            return i;
        }
    }
    return -1;
}

int mbox_send(mailbox_t *mailbox, void *msg, int msg_length)
{
    // TODO:
    int arg_i = find_arg_t();
    mbox_args[arg_i].valid = 1;
    mbox_args[arg_i].msg = msg;
    mbox_args[arg_i].msg_length = msg_length;
    return mbox_op(mailbox,&mbox_args[arg_i],2);
}

int mbox_recv(mailbox_t *mailbox, void *msg, int msg_length)
{
    // TODO:
    int arg_i = find_arg_t();
    mbox_args[arg_i].valid = 1;
    mbox_args[arg_i].msg = msg;
    mbox_args[arg_i].msg_length = msg_length;
    return mbox_op(mailbox,&mbox_args[arg_i],3);
}
