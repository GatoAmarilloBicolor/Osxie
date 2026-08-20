#include "CGSEvent.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define EVENT_QUEUE_SIZE 256

static CGSEventRecord event_queue[EVENT_QUEUE_SIZE];
static int queue_head = 0;
static int queue_tail = 0;
static pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  queue_cond  = PTHREAD_COND_INITIALIZER;

static CGSConnectionID main_connection = 1;

CGSConnectionID CGSMainConnectionID(void) {
    return main_connection;
}

CGError CGSPostEventRecord2(const CGSEventRecord *record) {
    if (!record) return kCGErrorIllegalArgument;

    pthread_mutex_lock(&queue_mutex);
    int next = (queue_tail + 1) % EVENT_QUEUE_SIZE;
    if (next == queue_head) {
        pthread_mutex_unlock(&queue_mutex);
        return kCGErrorFailure;
    }
    memcpy(&event_queue[queue_tail], record, sizeof(CGSEventRecord));
    queue_tail = next;
    pthread_cond_signal(&queue_cond);
    pthread_mutex_unlock(&queue_mutex);

    return kCGErrorSuccess;
}

int CGSGetNextEventRecord(CGSEventRecord *out) {
    pthread_mutex_lock(&queue_mutex);
    while (queue_head == queue_tail) {
        pthread_cond_wait(&queue_cond, &queue_mutex);
    }
    memcpy(out, &event_queue[queue_head], sizeof(CGSEventRecord));
    queue_head = (queue_head + 1) % EVENT_QUEUE_SIZE;
    pthread_mutex_unlock(&queue_mutex);
    return 0;
}

int CGSPeekEventRecord(CGSEventRecord *out) {
    pthread_mutex_lock(&queue_mutex);
    if (queue_head == queue_tail) {
        pthread_mutex_unlock(&queue_mutex);
        return -1;
    }
    memcpy(out, &event_queue[queue_head], sizeof(CGSEventRecord));
    pthread_mutex_unlock(&queue_mutex);
    return 0;
}

void CGSInitEventSystem(void) {
    queue_head = 0;
    queue_tail = 0;
}
