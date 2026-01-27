#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "utils.h"

void clean_file(const char *path) {
    unlink(path);
}

void clean_dir(const char *path) {
    rmdir(path);
}

int saved_stdout, saved_stderr;

void suppress_output() {
    saved_stdout = dup(STDOUT_FILENO);
    saved_stderr = dup(STDERR_FILENO);

    int dev_null = open("/dev/null", O_WRONLY);

    dup2(dev_null, STDOUT_FILENO);
    dup2(dev_null, STDERR_FILENO);

    close(dev_null);
}

void restore_output() {
    dup2(saved_stdout, STDOUT_FILENO);
    dup2(saved_stderr, STDERR_FILENO);

    close(saved_stdout);
    close(saved_stderr);
}

void test_print_file() {
    printf("Running test_print_file... ");
    fflush(stdout);

    const char *fname = "test_print.txt";
    
    FILE *f = fopen(fname, "w");
    if (!f) { perror("Test Setup Failed"); return; }
    fprintf(f, "Hello World\n");
    fclose(f);

    suppress_output();
    int res_success = print_file(fname);
    int res_fail = print_file("ghost_file.txt");
    restore_output();

    assert(res_success == 0);
    assert(res_fail != 0);

    clean_file(fname);
    printf("PASSED\n");
}

void test_parse_cpu_core_ids() {
    printf("Running test_parse_cpu_core_ids... ");
    fflush(stdout); 

    const char *fname = "test_cpuinfo.txt";
    int cores[10];

    FILE *f = fopen(fname, "w");
    if (!f) { perror("Test Setup Failed"); return; }
    fprintf(f, "processor\t: 0\nmodel name\t: Test CPU\n\n");
    fprintf(f, "processor\t: 1\nmodel name\t: Test CPU\n\n");
    fprintf(f, "flags\t\t: fpu vme de pse\n\n");
    fprintf(f, "processor\t: 5\nmodel name\t: Test CPU\n\n");
    fclose(f);

    parse_cpu_core_ids(fname, cores, 10);

    assert(cores[0] == 0);
    assert(cores[1] == 1);
    assert(cores[2] == 5);
    assert(cores[3] == -1);

    clean_file(fname);
    printf("PASSED\n");
}

void test_ls_subdirs() {
    printf("Running test_ls_subdirs... ");
    fflush(stdout);

    mkdir("test_env", 0777);
    mkdir("test_env/A", 0777);
    mkdir("test_env/B", 0777);

    char result_buffer[8][16];
    memset(result_buffer, 0, sizeof(result_buffer));

    suppress_output();
    int count = ls_subdirs("test_env", result_buffer, 8);
    restore_output();

    assert(count == 2);

    int found_A = 0;
    int found_B = 0;

    for(int i = 0; i < count; i++) {
        if (strcmp(result_buffer[i], "A") == 0) found_A = 1;
        if (strcmp(result_buffer[i], "B") == 0) found_B = 1;
    }

    assert(found_A == 1);
    assert(found_B == 1);

    clean_dir("test_env/A");
    clean_dir("test_env/B");
    clean_dir("test_env");
    printf("PASSED\n");
}

int main() {
    printf("=== Starting Linux Test Suite ===\n");
    
    test_print_file();
    test_parse_cpu_core_ids();
    test_ls_subdirs();

    printf("=== All Tests Passed ===\n");
    return 0;
}
