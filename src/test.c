#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "utils.h"
#include "config.h"

void clean_file(const char *path)
{
    unlink(path);
}

void clean_dir(const char *path)
{
    rmdir(path);
}

void passed()
{
    printf("%s%sPASSED%s\n", BOLD, GREEN, RESET);
}

int saved_stdout, saved_stderr;

void suppress_output()
{
    saved_stdout = dup(STDOUT_FILENO);
    saved_stderr = dup(STDERR_FILENO);

    int dev_null = open("/dev/null", O_WRONLY);

    dup2(dev_null, STDOUT_FILENO);
    dup2(dev_null, STDERR_FILENO);

    close(dev_null);
}

void restore_output()
{
    dup2(saved_stdout, STDOUT_FILENO);
    dup2(saved_stderr, STDERR_FILENO);

    close(saved_stdout);
    close(saved_stderr);
}

void test_r_file()
{
    printf("test_r_file\t\t");
    fflush(stdout);

    const char *fname = "test_read.txt";
    const char *expected_content = "Hello World\n";
    
    FILE *f = fopen(fname, "w");
    if (!f) { perror("Test Setup Failed"); return; }
    fprintf(f, "%s", expected_content);
    fclose(f);

    // Test Success
    char *content = r_file(fname);
    assert(content != NULL);
    assert(strcmp(content, expected_content) == 0);
    free(content);

    // Test Failure (Suppress stderr error message)
    suppress_output();
    char *ghost = r_file("ghost_file.txt");
    restore_output();

    assert(ghost == NULL);

    clean_file(fname);
    passed();

}

void test_p_cpuc_ids()
{
    printf("test_p_cpuc_ids\t\t");
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

    p_cpuc_ids(fname, cores, 10);

    assert(cores[0] == 0);
    assert(cores[1] == 1);
    assert(cores[2] == 5);
    assert(cores[3] == -1);

    clean_file(fname);
    passed();
}

void test_ls_subdirs()
{
    printf("test_ls_subdirs\t\t");
    fflush(stdout);

    mkdir("test_env", 0777);
    mkdir("test_env/A", 0777);
    mkdir("test_env/B", 0777);

    char *result_buffer[8];
    memset(result_buffer, 0, sizeof(result_buffer));

    suppress_output();
    int count = ls_subdirs("test_env", result_buffer, 8);
    restore_output();

    assert(count == 2);

    int found_A = 0;
    int found_B = 0;

    for(int i = 0; i < count; i++)
    {
        if (strcmp(result_buffer[i], "A") == 0) found_A = 1;
        if (strcmp(result_buffer[i], "B") == 0) found_B = 1;
    }

    for(int i = 0; i < 8; i++)
    {
        free(result_buffer[i]);
    }

    assert(found_A == 1);
    assert(found_B == 1);

    clean_dir("test_env/A");
    clean_dir("test_env/B");
    clean_dir("test_env");
    passed();
}

void test_count_subdirs()
{
    printf("test_count_subdirs\t");
    fflush(stdout);

    mkdir("test_count", 0777);
    mkdir("test_count/X", 0777);
    mkdir("test_count/Y", 0777);
    mkdir("test_count/Z", 0777);

    int count = count_subdirs("test_count");
    assert(count == 3);

    clean_dir("test_count/X");
    clean_dir("test_count/Y");
    clean_dir("test_count/Z");
    clean_dir("test_count");
    passed();
}

int main()
{
    printf("=== Starting Linux Test Suite ===\n");
    
    test_r_file();
    test_p_cpuc_ids();
    test_ls_subdirs();
    test_count_subdirs();

    printf("%s%s=== All Tests Passed ===%s\n\n", BOLD, GREEN, RESET);
    return 0;
}
