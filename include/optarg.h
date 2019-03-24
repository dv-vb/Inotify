#ifndef _OPTARG_H_
#define _OPTARG_H_

#define ARG_WITH_DARMON (0x1 << 0)

void usage(int argc, char **argv);

void get_usage_option(int argc, char **argv);

char *get_watch_path(void);

char *get_target_path(void);

#endif
