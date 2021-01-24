#ifndef __EVENT_H
#define __EVENT_H

/**
高效的多路复用接口
*/


#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/* 支持的最大事件数*/
#define EVENT_SETSIZE   (10)

#define EVENT_OK    0
#define EVENT_ERR   -1

#define EVENT_NONE      0
#define EVENT_READABLE  1
#define EVENT_WRITABLE  2

#define EVENT_FILE_EVENTS   1
#define EVENT_TIME_EVENTS   2
#define EVENT_ALL_EVENTS    (EVENT_FILE_EVENTS | EVENT_TIME_EVENTS)

/** 该宏会使 poll 立即返回 */
#define EVENT_DONT_WAIT     4

/** 如果time 回调中返回 该宏 就删除对应的time事件*/
#define EVENT_NOMORE    -1
#define EVENT_NOTUSED(V)    ((void)V)

struct evEventLoop;

struct event_ops
{
    int (*evops_ApiCreate)(struct evEventLoop *);
    void (*evops_ApiFree)(struct evEventLoop *l);
    int (*evops_ApiAddEvent)(struct evEventLoop *eventLoop, int fd, int mask);
    void (*evops_ApiDelEvent)(struct evEventLoop *eventLoop, int fd, int delmask);
    int (*evops_ApiPoll)(struct evEventLoop *eventLoop, struct timeval *tvp);
    char *(*evops_ApiName)(void);
};


/* Types and data structures */
/**
 * @brief evFileProc
 * @param eventLoop
 * @param fd
 * @param clientData    私有数据
 * @param mask      event_none event_readable event_writable
 */
typedef void evFileProc(struct evEventLoop *eventLoop, int fd, void *clientData, int mask);
/**
 * @brief evTimeProc
 * @param eventLoop
 * @param id    该id是内部自动添加
 * @param clientData    私有数据
 * @return  返回下一次超时时间
 */
typedef int evTimeProc(struct evEventLoop *eventLoop, long long id, void *clientData);
/**
 * @brief evEventFinalizerProc   当timer事件被删除时调用
 * @param eventLoop
 * @param clientData
 */
typedef void evEventFinalizerProc(struct evEventLoop *eventLoop, void *clientData);
/**
 * @brief aeBeforeSleepProc  进入poll监听前处理的事件
 * @param eventLoop
 */
typedef void evBeforeSleepProc(struct evEventLoop *eventLoop);

/* File event structure */
typedef struct evFileEvent {
    int mask; /* one of AE_(READABLE|WRITABLE) */
    evFileProc *rfileProc;
    evFileProc *wfileProc;
    void *clientData;
} evFileEvent;

/* Time event structure */
typedef struct evTimeEvent {
    long long id; /* time event identifier. */
    long when_sec; /* seconds */
    long when_ms; /* milliseconds */
    evTimeProc *timeProc;
    evEventFinalizerProc *finalizerProc;
    void *clientData;
    struct evTimeEvent *next;
} evTimeEvent;

/* A fired event */
/**
  poll 时 发生了的事件
  */
typedef struct evFiredEvent {
    int fd;
    int mask;
} evFiredEvent;

/* State of an event based program */
typedef struct evEventLoop {
    int maxfd;
    long long timeEventNextId;
    evFileEvent events[EVENT_SETSIZE]; /* Registered events */
    evFiredEvent fired[EVENT_SETSIZE]; /* Fired events */
    evTimeEvent *timeEventHead;
    int stop;
    void *apidata; /* This is used for polling API specific data */ /*用于指定具体的epoll select的私有数据*/
    evBeforeSleepProc *beforesleep;
} evEventLoop;

/* Prototypes */
/**
 * @brief evCreateEventLoop 创建一个loop
 * @return
 */
evEventLoop *evCreateEventLoop(void);
void evDeleteEventLoop(evEventLoop *eventLoop);

/**
 * @brief evStop  停止监听
 * @param eventLoop
 */
void evStop(evEventLoop *eventLoop);
/**
 * @brief evCreateFileEvent  创建fd事件
 * @param eventLoop
 * @param fd
 * @param mask  event_readable event_none event_writable
 * @param proc
 * @param clientData  私有数据
 * @return
 */
int evCreateFileEvent(evEventLoop *eventLoop, int fd, int mask,
        evFileProc *proc, void *clientData);

void evDeleteFileEvent(evEventLoop *eventLoop, int fd, int mask);

long long evCreateTimeEvent(evEventLoop *eventLoop, long long milliseconds,
        evTimeProc *proc, void *clientData,
        evEventFinalizerProc *finalizerProc);

int evDeleteTimeEvent(evEventLoop *eventLoop, long long id);
/**处理事件*/
int evProcessEvents(evEventLoop *eventLoop, int flags);

int evWait(int fd, int mask, long long milliseconds);
/** 监听函数 主函数*/
void evMain(evEventLoop *eventLoop);
/** 获取epoll select 名字*/
char *evGetApiName(void);
/** 在处理事件前执行 */
void evSetBeforeSleepProc(evEventLoop *eventLoop, evBeforeSleepProc *beforesleep);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __EVENT_h
