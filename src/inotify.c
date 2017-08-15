/*
 * Copyright(C) 2017 Ruijie Network. All rights reserved.
 */
/*
 * inotify.c
 * Original Author: wujincheng@ruijie.com.cn , 2017年6月15日-上午11:49:33
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <stdlib.h>

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
            if ((event->mask | IN_CREATE ) && wd == event->wd) {
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

static void usage(int argc, char **argv)
{
    printf("[%s]Usage: \r\n", argv[0]);
    printf("%s path dstpath\r\n", argv[0]);
    printf("--deamon    That the %s run back.\r\n", argv[0]);
    printf("--dirdetect    If there have direction on your path which you care about,"
        " will detect it also.\r\n");
    printf("--allyes    All file when finished created on you care about, will copy"
        " to dstpath\r\n");
    printf("--move      All file when finished created on you care about, will move"
        " to dstpath, default just copy.\r\n");
}

static int add_dir_watch_under_your_path(int fd, int *wd)
{
    /* TODO: should create a list for watch path. */
    return 0;
}

static int deamonize(void)
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
    deamonize();
    fd = inotify_create_nonblock();
    if (fd == -1) {
        exit(fd);
    }

    wd = inotify_add_watch(fd, argv[1], IN_CREATE);
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

