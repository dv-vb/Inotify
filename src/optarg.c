#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "optarg.h"

#define OPT_NAME_LEN                    (20)
#define OPT_DESCRIPTION_LEN             (1024)

typedef struct n_opt_s {
    char desc[OPT_DESCRIPTION_LEN];
    struct option opt;
} n_opt_t;

static n_opt_t g_optss[] = {
    {"Let process run in back, default is no.", {"daemon", no_argument, 0, 'd'}},
    {"*Inotify the folder, should set this paramaters.", {"folder", required_argument, 0, 'f'}},
    {"*Copy or mv new file to target dir, should set this paramaters.", {"target-folder", required_argument, 0, 't'}},
    {"Detect the sub-folder", {"sub-folder", no_argument, 0, 's'}},
    {"*Action: copy|move|logging, should set this paramaters.", {"action", required_argument, 0, 'a'}},
    {"Joke-tesing", {"joket", optional_argument, 0, 'j'}},
    {"Get Help", {"help", no_argument, 0, 'h'}},
};

static char g_watch_path[OPT_DESCRIPTION_LEN];
static char g_target_path[OPT_DESCRIPTION_LEN];
static uint32_t g_arg_flags;

char *get_watch_path(void)
{
    return g_watch_path;
}

char *get_target_path(void)
{
    return g_target_path;
}

static void parse_option(char *str)
{
    int total, i;
    n_opt_t *pos;

    total = sizeof(g_optss) / sizeof(n_opt_t);
    for (i = 0; i < total; i++) {
        pos = &g_optss[i];
        switch (pos->opt.has_arg) {
        case no_argument:
            sprintf(str, "%s%c", str, pos->opt.val);
            break;
        case required_argument:
            sprintf(str, "%s%c:", str, pos->opt.val);
            break;
        case optional_argument:
            sprintf(str, "%s%c::", str, pos->opt.val);
            break;
        default:
            /* do nothing. */
            break;
        } /* end of switch */
        /* printf("currently optstring: %s\r\n", str); */
    } /* end of for */
}

void usage(int argc, char **argv)
{
    int total, i;
    n_opt_t *pos;

    printf("====%s Usage====\r\n", argv[0]);
    total = sizeof(g_optss) / sizeof(n_opt_t);
    for (i = 0; i < total; i++) {
        pos = &g_optss[i];
        switch (pos->opt.has_arg) {
        case no_argument:
            printf("-%c|--%s, (no_argument): %s\r\n", pos->opt.val, pos->opt.name, pos->desc);
            break;
        case required_argument:
            printf("-%c|--%s, (required_argument):%s\r\n", pos->opt.val, pos->opt.name, pos->desc);
            break;
        case optional_argument:
            printf("-%c|--%s, (option_argument): %s\r\n", pos->opt.val, pos->opt.name, pos->desc);
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

static struct option *alloc_option(void)
{
    struct option *opts;
    int i, total;

    total = sizeof(g_optss) / sizeof(n_opt_t);
    opts = (struct option *)calloc((sizeof(struct option) * total), 1);
    assert(opts);
    for (i = 0; i < total; i++) {
        memcpy(&opts[i], &g_optss[i].opt, sizeof(struct option));
    }

    return opts;
}

void get_usage_option(int argc, char **argv)
{
#define TMP_LEN 200
    int opt, opt_index, result;
    char optstring[TMP_LEN] = {0};
    struct option *opts;

    parse_option(optstring);
    opt_index = 0;
    opts = alloc_option();
    result = 0;
    while (1) {
        opt = getopt_long(argc, argv, optstring, opts, &opt_index);
        if (opt == -1) {
            break;
        }
        switch (opt) {
        case 'h':
        case '?':
            usage(argc, argv);
            exit(0);
            break;
        case 'd':
            /* we need set daemonaize flag first. */
            g_arg_flags | ARG_WITH_DARMON;
            break;
        case 't':
            printf("action target folder is (%s)\r\n", optarg);
            result++;
            break;
        case 'f':
            printf("detect folder is (%s)\r\n", optarg);
            memset(g_watch_path, 0, OPT_DESCRIPTION_LEN);
            snprintf(g_watch_path, OPT_DESCRIPTION_LEN, "%s", optarg);
            result++;
            break;
        case 'a':
            printf("action with value (%s)\r\n", optarg);
            result++;
            break;
        default:
            break;
        } /* end of switch */
    } /* end of while */
    free(opts);
    /* should have 't' 'f' 'a' */
    if (result != 3) {
        usage(argc, argv);
        exit(0);
    }
}

