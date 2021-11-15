#include <time.h>
#include <test3.h>
#include <mthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <mailbox.h>
#include <sys/syscall.h>

#define clear printf("                                      ");

// struct task_info strserver_task = {(uintptr_t)&strServer, USER_PROCESS};
// struct task_info strgenerator_task = {(uintptr_t)&strGenerator, USER_PROCESS};

static const char initReq[] = "clientInitReq";
static const int initReqLen = sizeof(initReq);
static const char names[3][20] = {"Blue Space", "Natual Selection", "Bronze Age"};
struct MsgHeader
{
    int length;
    uint32_t checksum;
    pid_t sender;
};

const uint32_t MOD_ADLER = 65521;

/* adler32 algorithm implementation from: https://en.wikipedia.org/wiki/Adler-32 */
uint32_t adler32(unsigned char *data, size_t len)
{
    uint32_t a = 1, b = 0;
    size_t index;

    // Process each byte of the data in order
    for (index = 0; index < len; ++index)
    {
        a = (a + data[index]) % MOD_ADLER;
        b = (b + a) % MOD_ADLER;
    }

    return (b << 16) | a;
}

int clientInitReq(const char* buf, int length)
{
    if (length != initReqLen) return 0;
    for (int i = 0; i < initReqLen; ++i) {
        if (buf[i] != initReq[i]) return 0;
    }
    return 1;
}

void strServer(void)
{
    char msgBuffer[MAX_MBOX_LENGTH];
    struct MsgHeader header;
    int64_t correctRecvBytes = 0;
    int64_t errorRecvBytes = 0;
    int64_t blockedCount = 0;
    int clientPos = 2;

    mailbox_t mq_m = mbox_open("str-message-queue");
    mailbox_t *mq = &mq_m;
    mailbox_t posmq_m = mbox_open("pos-message-queue");
    mailbox_t *posmq = &posmq_m;
    sys_move_cursor(1, 1);
    printf("[Server] server started");
    sys_sleep(1);

    for (;;)
    {
        blockedCount += mbox_recv(mq, &header, sizeof(struct MsgHeader),0);
        blockedCount += mbox_recv(mq, msgBuffer, header.length,0);

        uint32_t checksum = adler32(msgBuffer, header.length);
        if (checksum == header.checksum) {
            correctRecvBytes += header.length;
        } else {
            errorRecvBytes += header.length;
        }

        sys_move_cursor(1, 1);
        printf("[Server]: recved msg from %d (blocked: %ld, correctBytes: %ld, errorBytes: %ld)",
              header.sender, blockedCount, correctRecvBytes, errorRecvBytes);

        if (clientInitReq(msgBuffer, header.length)) {
            mbox_send(posmq, &clientPos, sizeof(int),0);
            ++clientPos;
        }

        sys_sleep(1);
    }
}

int clientSendMsg(mailbox_t *mq, const char* content, int length, int sleep_operation)
{
    int i;
    char msgBuffer[MAX_MBOX_LENGTH] = {0};
    struct MsgHeader* header = (struct MsgHeader*)msgBuffer;
    char* _content = msgBuffer + sizeof(struct MsgHeader);
    header->length = length;
    header->checksum = adler32(content, length);
    header->sender = sys_getpid();

    for (i = 0; i < length; ++i) {
        _content[i] = content[i];
    }
    return mbox_send(mq, msgBuffer, length + sizeof(struct MsgHeader), sleep_operation);
}

void generateRandomString(char* buf, int len)
{
    static const char alpha[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-={};|[],./<>?!@#$%^&*";
    static const int alphaSz = sizeof(alpha) / sizeof(char);
    int i = len - 2;
    buf[len - 1] = '\0';
    while (i >= 0) {
        buf[i] = alpha[rand() % alphaSz];
        --i;
    }
}

void strGenerator(void)
{
    mailbox_t mq_m = mbox_open("str-message-queue");
    mailbox_t *mq = &mq_m;
    mailbox_t posmq_m = mbox_open("pos-message-queue");
    mailbox_t *posmq = &posmq_m;

    int len = 0;
    int strBuffer[MAX_MBOX_LENGTH - sizeof(struct MsgHeader)];
    clientSendMsg(mq, initReq, initReqLen, 0);
    int position = 1;
    mbox_recv(posmq, &position, sizeof(int),0);
    int blocked = 0;
    int64_t bytes = 0;

    sys_move_cursor(1, position);
    printf("[Client %d] server started", position);
    sys_sleep(1);
    for (;;)
    {
        len = (rand() % ((MAX_MBOX_LENGTH - sizeof(struct MsgHeader))/2)) + 1;
        generateRandomString(strBuffer, len);
        blocked += clientSendMsg(mq, strBuffer, len, 0);
        bytes += len;

        sys_move_cursor(1, position);
        printf("[Client %d] send bytes: %ld, blocked: %d", position,
            bytes, blocked);
        sys_sleep(1);
    }
}

void send_recver(int idx){
    mailbox_t mq_m;
    mailbox_t *mq = &mq_m;
    mailbox_t mq_m_recv = mbox_open(names[idx]);
    mailbox_t *mq_recv = &mq_m_recv;
    int flow = 1;
    // sender
    int len = 0;
    int raw_rand = idx;
    int send_to = idx;
    int strBuffer[MAX_MBOX_LENGTH - sizeof(struct MsgHeader)];
    int send_res = 0;
    int blocked = 0;
    int64_t bytes = 0;
    // recver
    char msgBuffer[MAX_MBOX_LENGTH];
    int64_t correctRecvBytes = 0;
    int64_t errorRecvBytes = 0;
    int recv_res = 0;
    int64_t blockedCount = 0;
    struct MsgHeader header;

    srand(clock());
    sys_move_cursor(1 , 5*idx + 1);
    printf("This is %s",names[idx]);
    for(;;){
        srand(clock());
        if((flow == 1) || (flow == 0 && rand()%2 == 0)){
            srand(clock());
            raw_rand = rand() % 2;
            if(raw_rand >= idx){
                send_to = raw_rand + 1;
            }
            else{
                send_to = raw_rand;
            }
            mq_m = mbox_open(names[send_to]);
            len = (rand() % ((MAX_MBOX_LENGTH - sizeof(struct MsgHeader))/2)) + 1;
            generateRandomString(strBuffer, len);
            send_res = clientSendMsg(mq, strBuffer, len, 1);
            if(send_res < 0){
                flow = 2;
                continue;
            }
            else{
                blocked += send_res;
            }
            bytes += len;
            sys_move_cursor(1, 5 * idx + raw_rand + 2);
            printf("[%s Sender] send to %s, send bytes: %ld, blocked: %d", names[idx], names[send_to], bytes, blocked);
            sys_move_cursor(1, 5 * idx + 5);
            printf("[%s] operation:send msg to %s", names[idx], names[send_to]);
            flow = 0;
            sys_sleep(1);
        }
        if((flow == 2) || (flow == 0 && rand()%2 == 1)){
            recv_res = mbox_recv(mq_recv, &header, sizeof(struct MsgHeader),1);
            if(recv_res < 0){
                flow = 1;
                continue;
            }
            else{
                blockedCount += recv_res;
            }
            recv_res = mbox_recv(mq_recv, msgBuffer, header.length,1);
            if(recv_res < 0){
                flow = 1;
                continue;
            }
            else{
                blockedCount += recv_res;
            }
            uint32_t checksum = adler32(msgBuffer, header.length);
            if (checksum == header.checksum) {
                correctRecvBytes += header.length;
            } else {
                errorRecvBytes += header.length;
            }
            sys_move_cursor(1, 5 * idx + 4);
            printf("[%s Receiver]: receive msg from %s (blocked: %ld, correctBytes: %ld, errorBytes: %ld)", names[idx],
                names[header.sender - 3], blockedCount, correctRecvBytes, errorRecvBytes);
            sys_move_cursor(1, 5 * idx + 5);
            printf("[%s] operation:recv msg from %s", names[idx], names[header.sender - 3]);
            flow = 0;
            sys_sleep(1);
        }
    }
}

void test_mailbox_multicore(void)
{
    struct task_info Send_Recver = {(uintptr_t)&send_recver, USER_PROCESS};
    int idxes[] = {0,1,2};
    pid_t pids[3] = {0};
    for(int i = 0; i < 3; i++){
        pids[i] = sys_spawn(&Send_Recver, &idxes[i], ENTER_ZOMBIE_ON_EXIT);
    }
    for(int i = 0; i < 3; i++){
        sys_waitpid(pids[i]);
    }
    sys_exit();
}