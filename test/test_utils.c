#include "unity/unity.h"
#include "../src/utils.h"

void setUp(void) {}

void tearDown(void) {}

// --- Parsing Tests (String -> Number) ---
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

// --- Formatting Tests (Number -> String) ---
void test_append_num_zero(void) {
    char buf[100];
    memset(buf, 0, 100);
    
    char *end_ptr = append_num(buf, 0);
    
    TEST_ASSERT_EQUAL_STRING("0", buf);
    TEST_ASSERT_EQUAL_PTR(buf + 1, end_ptr);
}

void test_append_num_positivo(void) {
    char buf[100];
    memset(buf, 0, 100);
    
    char *end_ptr = append_num(buf, 123456);
    
    TEST_ASSERT_EQUAL_STRING("123456", buf);
    TEST_ASSERT_EQUAL_INT(6, end_ptr - buf);
}

void test_append_num_negativo(void) {
    char buf[100];
    memset(buf, 0, 100);
    
    int val = -500;
    append_num(buf, val);
    
    TEST_ASSERT_EQUAL_STRING("-500", buf);
}

void test_append_num_max_uint64(void) {
    char buf[100];
    memset(buf, 0, 100);
    
    uint64_t val = 18446744073709551615ULL; // UINT64_MAX
    append_num(buf, val);
    
    TEST_ASSERT_EQUAL_STRING("18446744073709551615", buf);
}

// --- Concatenation Test (Chaining) ---
void test_chaining(void) {
    char buf[100];
    memset(buf, 0, 100);
    char *p = buf;

    p = append_str(p, "CPU: ");
    p = append_num(p, 99);
    p = append_str(p, "%");

    TEST_ASSERT_EQUAL_STRING("CPU: 99%", buf);
}

int main(void) {
    UNITY_BEGIN();

    // Parsing
    RUN_TEST(test_str_to_uint64_basico);
    RUN_TEST(test_str_to_uint64_com_lixo_no_final);
    RUN_TEST(test_skip_to_digit);

    // Formatting
    RUN_TEST(test_append_num_zero);
    RUN_TEST(test_append_num_positivo);
    RUN_TEST(test_append_num_negativo);
    RUN_TEST(test_append_num_max_uint64);
    RUN_TEST(test_chaining);

    return UNITY_END();
}
