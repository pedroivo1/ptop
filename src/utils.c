#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "utils.h"
#include "config.h"


char *r_file(const char *fpath)
{
    FILE *fptr = fopen(fpath, "r");
    if (fptr == NULL)
    {
        fprintf(stderr, "Error: Could not open file \"%s\"\n", fpath);
        return NULL;
    }

    char *text = malloc(sizeof(char));
    if (text == NULL)
    {
        fclose(fptr);
        return NULL;
    }
    text[0] = '\0';

    int text_len = 0;
    char buff[MAX_LINE_LENGTH];
    while(fgets(buff, sizeof(buff), fptr))
    {
        int chunk_len = strlen(buff);
        char *temp = (char*) realloc(text, text_len + chunk_len + 1);
        if(temp == NULL)
        {
            free(text);
            fclose(fptr);
            return NULL;
        }
        text = temp;

        strcpy(text + text_len, buff);
        text_len += chunk_len;
    }
    fclose(fptr);

    return text;
}


long read_int_from_file(const char* path)
{
    FILE* fptr = fopen(path, "r");
    if(!fptr) return -1;

    long value;
    if (fscanf(fptr, "%ld", &value) != 1) value = -1;

    fclose(fptr);
    return value;
}


int count_subdirs(const char *drpath)
{
    struct dirent *de;
    DIR *dr = opendir(drpath);
    if (dr == NULL)
    {
        fprintf(stderr, "Error: Could not open directory \"%s\"\n", drpath);
        return 0;
    }

    int i = 0;
    while((de = readdir(dr)))
    {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            continue;
        i++;
    }
    closedir(dr);

    return i;
}


int ls_subdirs(const char *drpath, char **subfs, int size)
{
    struct dirent *de;
    DIR *dr = opendir(drpath);
    if (dr == NULL)
    {
        fprintf(stderr, "Error: Could not open directory \"%s\"\n", drpath);
        return 0;
    }

    int i = 0;
    while((de = readdir(dr)) != NULL && i < size)
    {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            continue;

        subfs[i] = (char*) malloc(strlen(de->d_name) + 1);
        strcpy(subfs[i], de->d_name);
        i++;
    }
    closedir(dr);

    return i;
}

// ======= CPU =======
int p_cpuc_ids(const char *fpath, int *core_ids, int size)
{
    FILE *fptr = fopen(fpath, "r");
    if (fptr == NULL)
    {
        fprintf(stderr, "Error: Could not open file \"%s\"\n", fpath);
        return 1;
    }

    char buff[MAX_LINE_LENGTH];
    int i = 0;
    while(fgets(buff, sizeof(buff), fptr) && i < size - 1)
    {
        int match = strncmp(buff, "processor", 9);
        if(match == 0)
        {
            char *c_id = strstr(buff, ":");
            if(c_id)
            {
                int core_id = atoi(c_id+1);
                core_ids[i++] = core_id;
            }
        }
    }
    core_ids[i] = -1;

    fclose(fptr);
    return 0;
}


int get_hardwares(char **hw_paths)
{
    return 0;
}


int p_cpuc_temp()
{
    return 0;
}


void cpu_monitor()
{
    int cores[MAX_CORES];
    p_cpuc_ids("/proc/cpuinfo", cores, MAX_CORES);

    int i = 0;
    while(cores[i] != -1)
    {
        printf("%d\n", cores[i++]);
    }
}


void handle_exit(int sig)
{
    printf("\033[?1049l");
    printf("\033[?25h");
    exit(0);
}
