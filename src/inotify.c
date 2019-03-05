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

#define OPT_NAME_LEN                    (20)
#define OPT_DESCRIPTION_LEN             (1024)

typedef enum optype_s {
    NO_ARGUMENT = 1,
    OPTION_ARGUMENT,
    REQUIRED_ARGUMENT,
} optype_t;

typedef struct opt_s {
    char acronym;
    char name[OPT_NAME_LEN];
    optype_t type;
    char descript[OPT_DESCRIPTION_LEN];
} opt_t;

static opt_t g_opt[] = {
        {'d', "deamon", NO_ARGUMENT, "Let process run in back"},
        {'f', "folder", REQUIRED_ARGUMENT, "Detect the folder"},
        {'t', "target", REQUIRED_ARGUMENT, "Copy or mv new file to target dir"},
        {'s', "sub-folder", NO_ARGUMENT, "Detect the sub-folder"},
        {'a', "action", REQUIRED_ARGUMENT, "action: copy,move,logging"},
        {'j', "joke-test", OPTION_ARGUMENT, "joke-testing"},
        {'h', "help", NO_ARGUMENT, "help paramaters"},
};

typedef struct n_opt_s {
    char desc[OPT_NAME_LEN];
    struct option opt;
} n_opt_t;

static struct option g_opts[] = {
    ,
    {"joke-test", optional_argument, 0, 'j'},
    {"help", no_argument, 0, 'h'},
};

static n_opt_t g_optss[] = {
    {"Let process run in back", {"daemon", no_argument, 0, 'd'}},
    {"Inotify the folder", {"folder", required_argument, 0, 'f'}},
    {"Copy or mv new file to target dir", {"target-folder", required_argument, 0, 't'}},
    {"Detect the sub-folder", {"sub-folder", no_argument, 0, 's'}},
    {"Action: copy, move, logging", {"action", required_argument, 0, 'a'}},
    {"Joke-tesing", {"joket", optional_argument, 0, 'j'}},
    {"Get Help", {"help", no_argument, 0, 'h'}},
};

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
            /* we only focus on create event. */
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

static void parse_option(char *str)
{
    int total, i;
    n_opt_t *pos;
    char *ret_str;

    total = sizeof(g_opt) / sizeof(opt_t);
    for (i = 0; i < total; i++) {
        pos = &g_optss[i];
        switch (pos->opt.has_arg) {
        case NO_ARGUMENT:
            ret_str = strncat(str, &pos->acronym, 1);
            break;
        case REQUIRED_ARGUMENT:
            (void)strncat(str, &pos->acronym, 1);
            ret_str = strcat(str, ":");
            break;
        case OPTION_ARGUMENT:
            strncat(str, &pos->acronym, 1);
            ret_str = strcat(str, "::");
            break;
        default:
            /* do nothing. */
            ret_str = str;
            break;
        }
        printf("currently optstring: %s\r\n", ret_str);
    }
}

static void usage(int argc, char **argv)
{
    int total, i;
    opt_t *pos;

    printf("====%s Usage====\r\n", argv[0]);
    total = sizeof(g_opt) / sizeof(opt_t);
    for (i = 0; i < total; i++) {
        pos = &g_opt[i];
        switch (pos->type) {
        case NO_ARGUMENT:
            printf("-%c %s : (no_argument) %s\r\n", pos->acronym, pos->name, pos->descript);
            break;
        case REQUIRED_ARGUMENT:
            printf("-%c %s : (required_argument) %s\r\n", pos->acronym, pos->name, pos->descript);
            break;
        case OPTION_ARGUMENT:
            printf("-%c %s : (option_argument) %s\r\n", pos->acronym, pos->name, pos->descript);
            break;
        default:
            /* do nothing */
            break;
        }
    } /* end of for */

    /**
    printf("[%s]Usage: \r\n", argv[0]);
    printf("%s path dstpath\r\n", argv[0]);
    printf("--deamon    That the %s run back.\r\n", argv[0]);
    printf("--dirdetect    If there have direction on your path which you care about,"
        " will detect it also.\r\n");
    printf("--allyes    All file when finished created on you care about, will copy"
        " to dstpath\r\n");
    printf("--move      All file when finished created on you care about, will move"
        " to dstpath, default just copy.\r\n");
    */
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

static void get_usage_option(int argc, char **argv)
{
#define TMP_LEN 200
    int opt;
    char optstring[TMP_LEN] = {0};

    parse_option(optstring);
    while ((opt = getopt(argc, argv, optstring) != -1)) {
        printf("currently opt: %c %d\r\n", (char)opt, opt);
        switch (opt) {
        case 'h':
        case '?':
            usage(argc, argv);
            exit(0);
            break;
        default:
            break;
        }
    }
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

