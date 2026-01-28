#ifndef UTILS_H
#define UTILS_H

char *r_file(const char *fpath);
char *r_first_n_rows(const char *fpath, const int frow);
long read_int_from_file(const char* path);
int count_subdirs(const char *drpath);
int ls_subdirs(const char *drpath, char **subfs, int size);

int p_cpuc_ids(const char *fpath, int *core_ids, int size);
void cpu_monitor();
void handle_exit(int sig);

#endif
