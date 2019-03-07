/*
 * Copyright(C) 2017 Victor Wu. All rights reserved.
 */
/*
 * inotify.c
 * Original Author: hxyjxxx@gmail.com , 2017.6.15.-11:49:33am
 *
 * inotify inotify.c
 *
 * History
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#ifdef __linux__
#include <sys/inotify.h> /* FOR LINUX */
#endif
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include "optarg.h"

/**
 * inotify_create_nonblock
 * @return if fail return -1
 */
int inotify_create_nonblock(void)
{
    int fd;

    fd = inotify_init1(IN_NONBLOCK);
    if (fd == -1) {
        printf("%s create inotify fail: %s, %d\r\n", __func__, strerror(errno), errno);
    }

    return fd;
}

#define EVENT_RET_STR(mask, flag)                   \
    do {                                            \
        if (mask & flag) {                          \
            return #flag;                           \
        }                                           \
    } while(0)


static char *inotify_event2str(uint32_t mask)
{
    EVENT_RET_STR(mask, IN_ACCESS);
    EVENT_RET_STR(mask, IN_ATTRIB);
    EVENT_RET_STR(mask, IN_CREATE);
    EVENT_RET_STR(mask, IN_DELETE);
    EVENT_RET_STR(mask, IN_DELETE_SELF);
    EVENT_RET_STR(mask, IN_MOVE_SELF);
    EVENT_RET_STR(mask, IN_OPEN);

    return "no-match";
}

void inotify_handle(int fd, int wd, int argc, char **argv)
{
    char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event)))), *ptr;
    const struct inotify_event *event;
    int len;

    while (1) {
        len = read(fd, buf, sizeof(buf));
        if (len == -1 && errno != EAGAIN) {
            printf("%s read fail: %s, %d\r\n", __func__, strerror(errno), errno);
            exit(len);
        }
        if (len <= 0) {
            break;
        }
        for (ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {
            event = (const struct inotify_event *)ptr;
            /* we only focus on create event. */
            printf("event name: %s mask: 0x%x, mask means: %s\r\n", event->name, event->mask, inotify_event2str(event->mask));
            if ((event->mask & IN_CREATE ) && wd == event->wd) {
                if (event->len) {
                    printf("event %s \r\n", event->name);
                }
            }
            if (event->mask & IN_ISDIR) {
                printf(" [directory]\n");
            } else {
                printf(" [file]\n");
            }
        }
    }
}

static int add_dir_watch_under_your_path(int fd, int *wd)
{
    /* TODO: should create a list for watch path. */
    return 0;
}

static int daemonize(void)
{
    int fd;
    pid_t pid;

    pid = fork();
    switch (pid) {
    case 0:
        /* child */
        break;
    case -1:
        return -1;
    default:
        exit(0);
        break;
    }
    pid = setsid();
    if (pid == -1) {
        printf("%s setsid fail, %s, %d\r\n.", __func__, strerror(errno), errno);
        return -1;
    }
    if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > STDERR_FILENO) {
            close(fd);
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int fd, wd, poll_num;
    struct pollfd fds[2];
    nfds_t nfds; /* Type used for the number of file descriptors, unsigned long int. */

    if (argc <= 2) {
        usage(argc, argv);
        return 0;
    }
    get_usage_option(argc, argv);
    //deamonize();
    fd = inotify_create_nonblock();
    if (fd == -1) {
        exit(fd);
    }

    wd = inotify_add_watch(fd, get_watch_path(), IN_CREATE);
    if (wd == -1) {
        close(fd);
        printf("%s add inotify watch fail: %s, %d\r\n", __func__, strerror(errno),
            errno);
        return wd;
    }

    fds[0].fd = fd;
    fds[0].events = POLLIN;

    nfds = 1;

    while (1) {
        poll_num = poll(fds, nfds, -1);
        if (poll_num == -1) {
            if (errno == EINTR) {
                continue;
            }
            printf("poll fail: %s, %d, %s\r\n", __func__, errno, strerror(errno));
            close(fd);
            exit(poll_num);
        }
        if (poll_num > 0) {
            if (fds[0].revents & POLLIN) {
                inotify_handle(fd, wd, argc, argv);
            }
        }
    }

    return 0;
}

