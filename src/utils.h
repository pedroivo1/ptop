#ifndef UTILS_H
#define UTILS_H

int print_file(const char *fpath);
int ls_subdirs(const char *drpath, char subfs[][16], int size);

int parse_cpu_core_ids(const char *fpath, int *core_ids, int size);
void cpu_monitor();
void handle_exit(int sig);

#endif
