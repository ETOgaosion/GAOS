#include <mailbox.h>
#include <string.h>
#include <sys/syscall.h>

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

int mbox_send(mailbox_t *mailbox, void *msg, int msg_length, int sleep_operation)
{
    // TODO:
    mbox_arg_t mbox_args;
    mbox_args.msg = msg;
    mbox_args.msg_length = msg_length;
    mbox_args.sleep_operation = sleep_operation;
    return mbox_op(mailbox,&mbox_args,2);
}

int mbox_recv(mailbox_t *mailbox, void *msg, int msg_length, int sleep_operation)
{
    // TODO:
    mbox_arg_t mbox_args;
    mbox_args.msg = msg;
    mbox_args.msg_length = msg_length;
    mbox_args.sleep_operation = sleep_operation;
    return mbox_op(mailbox,&mbox_args,3);
}

int mbox_try_send(mailbox_t *mailbox, void *msg, int msg_length, int sleep_operation)
{
    // TODO:
    mbox_arg_t mbox_args;
    mbox_args.msg = msg;
    mbox_args.msg_length = msg_length;
    mbox_args.sleep_operation = sleep_operation;
    return mbox_op(mailbox,&mbox_args,4);
}

int mbox_try_recv(mailbox_t *mailbox, void *msg, int msg_length, int sleep_operation)
{
    // TODO:
    mbox_arg_t mbox_args;
    mbox_args.msg = msg;
    mbox_args.msg_length = msg_length;
    mbox_args.sleep_operation = sleep_operation;
    return mbox_op(mailbox,&mbox_args,5);
}
