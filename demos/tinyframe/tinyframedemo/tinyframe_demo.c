

#include <pthread.h>

#include "TinyFrame.h"
#include "demo_util.h"

static pthread_mutex_t s_mutex = PTHREAD_MUTEX_INITIALIZER;

void TF_WriteImpl(TinyFrame *tf, const uint8_t *buff, uint32_t len) {
    // send to UART

    hex_dump("sending msg to uart", buff, len);
    hex_dump("simulating accepting msg", buff, len);

    
    
    TF_Accept(tf, buff, len);
}

// --------- Mutex callbacks ----------
// Needed only if TF_USE_MUTEX is 1 in the config file.
// DELETE if mutex is not used

/** Claim the TX interface before composing and sending a frame */
bool TF_ClaimTx(TinyFrame *tf) {
    // take mutex

    pthread_mutex_lock(&s_mutex);

    return true;  // we succeeded
}

/** Free the TX interface after composing and sending a frame */
void TF_ReleaseTx(TinyFrame *tf) {
    // release mutex

    pthread_mutex_unlock(&s_mutex);
}

TF_Result generic_listener(TinyFrame *tf, TF_Msg *msg) {
    APP_LOGW(
        "msg: \n"
        "frame_id: %x\n"
        "type: %x\n"
        "data: %s\n"
        "len: %u\n",
        msg->frame_id, msg->type, msg->data, msg->len);
    return TF_STAY;
}

void demo1() {
    TF_Msg msg = {0};

    // 初始化
    TinyFrame *tf = TF_Init(TF_MASTER);

    TF_AddGenericListener(tf, generic_listener);

    // 模拟给底板发消息
    char cmd[] = "cmd: move left";
    TF_ClearMsg(&msg);
    msg.type = 0x56;
    msg.data = (uint8_t *)cmd;
    msg.len = sizeof(cmd);
    TF_Send(tf, &msg);

    // 模拟底板回复消息
    // 在TF_WriteImpl中模拟，直接回复发送的消息
    //

    // 处理底板回复的消息
    // 在generic_listener中处理

    TF_DeInit(tf);
    tf = NULL;
    return;
}

TF_Result id_listener(TinyFrame *tf, TF_Msg *msg) {
    APP_LOGW("listen msg(id=%#x,type=%#x): %s\n", msg->frame_id, msg->type, msg->data);
    return TF_CLOSE;
}

void demo2() {
    TinyFrame *tf = TF_Init(TF_MASTER);
    TF_Msg msg = {0};
    TF_ClearMsg(&msg);
    char data[] = "query";
    msg.data = data;
    msg.len = sizeof(data);
    msg.type = 0x56;
    TF_Query(tf, &msg, id_listener, 5);
    APP_LOGW("sending msg id: %#x\n", msg.frame_id);
    TF_Query(tf, &msg, id_listener, 5);
    APP_LOGW("sending msg id: %#x\n", msg.frame_id);
    TF_DeInit(tf);
}

int main() {
    // demo1();
    demo2();
    return 0;
}