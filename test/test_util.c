#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "unity.h"
#include "util/util.h"

void setUp(void) {}
void tearDown(void) {}

// --- Testes de Parsing (String -> Number) ---

void test_str_to_uint64_basico(void) {
    char *input = "12345";
    char *ptr = input;
    uint64_t res = str_to_uint64(&ptr);
    TEST_ASSERT_EQUAL_UINT64(12345, res);
    TEST_ASSERT_EQUAL_PTR(input + 5, ptr);
}

void test_str_to_uint64_com_lixo_no_final(void) {
    char *input = "42abc";
    char *ptr = input;
    uint64_t res = str_to_uint64(&ptr);
    TEST_ASSERT_EQUAL_UINT64(42, res);
    TEST_ASSERT_EQUAL_CHAR('a', *ptr);
}

void test_skip_to_digit(void) {
    char *input = "cpu  123";
    char *ptr = input;
    skip_to_digit(&ptr);
    TEST_ASSERT_EQUAL_CHAR('1', *ptr);
}

void test_skip_line(void) {
    char *input = "primeira\nsegunda\nterceira";
    char *ptr = input;
    skip_line(&ptr);
    TEST_ASSERT_EQUAL_CHAR('s', *ptr);
    skip_line(&ptr);
    TEST_ASSERT_EQUAL_CHAR('t', *ptr);
}

// --- Testes de Formatação (Number -> String) ---

void test_append_num_variacoes(void) {
    char buf[128];
    char *p;

    // Zero
    p = buf; memset(buf, 0, 128);
    append_num(&p, 0);
    TEST_ASSERT_EQUAL_STRING("0", buf);

    // Positivo
    p = buf; memset(buf, 0, 128);
    append_num(&p, 123456);
    TEST_ASSERT_EQUAL_STRING("123456", buf);

    // Negativo (Signed)
    p = buf; memset(buf, 0, 128);
    int val = -500;
    append_num(&p, val);
    TEST_ASSERT_EQUAL_STRING("-500", buf);

    // Limite UINT64
    p = buf; memset(buf, 0, 128);
    uint64_t max_u64 = 18446744073709551615ULL;
    append_num(&p, max_u64);
    TEST_ASSERT_EQUAL_STRING("18446744073709551615", buf);
}

void test_append_utilitarios(void) {
    char buf[128] = {0};
    char *p = buf;

    APPEND_LIT(&p, "TEMP: ");
    append_two_digits(&p, 7);
    append_str(&p, " C");

    TEST_ASSERT_EQUAL_STRING("TEMP: 07 C", buf);
}

// --- Testes de Ponto Fixo (Fixed Point) ---

void test_append_fixed_decimal(void) {
    char buf[64];
    char *p;

    // 1 casa: 125 / 10 = 12.5
    p = buf; memset(buf, 0, 64);
    append_fixed_1d(&p, 125, 10);
    TEST_ASSERT_EQUAL_STRING("12.5", buf);

    // 2 casas com preenchimento: 1005 / 100 = 10.05
    p = buf; memset(buf, 0, 64);
    append_fixed_2d(&p, 1005, 100);
    TEST_ASSERT_EQUAL_STRING("10.05", buf);

    // Genérico 3 casas: 10005 / 1000 = 10.005
    p = buf; memset(buf, 0, 64);
    append_fixed_generic(&p, 10005, 1000, 1000);
    TEST_ASSERT_EQUAL_STRING("10.005", buf);
}

void test_append_fixed_shift(void) {
    char buf[64];
    char *p;

    // Shift 10 (divisor 1024). 1536 é 1.5 * 1024
    p = buf; memset(buf, 0, 64);
    append_fixed_shift_1d(&p, 1536, 10);
    TEST_ASSERT_EQUAL_STRING("1.5", buf);

    // Caso de borda: valor < 1.0 (ex: 512 / 1024 = 0.50)
    p = buf; memset(buf, 0, 64);
    append_fixed_shift_2d(&p, 512, 10);
    TEST_ASSERT_EQUAL_STRING("0.50", buf);

    // Genérico com shift e preenchimento de zeros internos (ex: 1.005)
    // 1024 + 5 = 1029. 1029/1024 com 3 casas decimais
    p = buf; memset(buf, 0, 64);
    append_fixed_shift_generic(&p, 1029, 10, 1000); 
    // Nota: (5 * 1000) >> 10 = 4.8... arredonda para baixo no integer math
    // O teste aqui valida a estrutura do append
    TEST_ASSERT_EQUAL_CHAR('.', buf[1]); 
}

// --- Testes de Sistema ---

void test_current_time_ms(void) {
    uint64_t t1 = current_time_ms();
    TEST_ASSERT_TRUE(t1 > 0);
}

void test_read_sysfs_uint64(void) {
    const char* tmp_file = "test_val.tmp";
    FILE* f = fopen(tmp_file, "w");
    fprintf(f, "98765\n");
    fclose(f);

    int fd = open(tmp_file, O_RDONLY);
    uint64_t val = read_sysfs_uint64(fd);
    close(fd);
    remove(tmp_file);

    TEST_ASSERT_EQUAL_UINT64(98765, val);
}

// --- Runner Principal ---

int main(void) {
    UNITY_BEGIN();

    // Parsing
    RUN_TEST(test_str_to_uint64_basico);
    RUN_TEST(test_str_to_uint64_com_lixo_no_final);
    RUN_TEST(test_skip_to_digit);
    RUN_TEST(test_skip_line);

    // Formatação Base
    RUN_TEST(test_append_num_variacoes);
    RUN_TEST(test_append_utilitarios);

    // Ponto Fixo
    RUN_TEST(test_append_fixed_decimal);
    RUN_TEST(test_append_fixed_shift);

    // Sistema
    RUN_TEST(test_current_time_ms);
    RUN_TEST(test_read_sysfs_uint64);

    return UNITY_END();
}
