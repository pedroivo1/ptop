#ifndef UTILS_H
#define UTILS_H

char *r_file(const char *fpath);
int count_subdirs(const char *drpath);
int ls_subdirs(const char *drpath, char **subfs, int size);

int p_cpuc_ids(const char *fpath, int *core_ids, int size);
void cpu_monitor();
void handle_exit(int sig);

#endif
