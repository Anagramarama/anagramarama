/*
 * Simple C unit testing framework.
 *
 * Declare a table containing your tests and call the run_test function
 * passing in this table. You can declare a setup and teardown function
 * for each test and provide per-test custom data if needed.
 *
 * A number of test helpers are provided to check values. They all print
 * to stdout and increment the pass/fail counters.
 *
 * For instance:
 *
 *   #ifdef UNIT_TEST
 *   #include "unittest.h"
 *
 *   static int testHashInit(void *clientData) 
 *   {
 *       struct hash_table_t ht;
 *   
 *       hash_init(&ht, hash_algorithm_jim);
 *       test_equals_int("new table has size 0", ht.size, 0);
 *       test_equals_int("new table has usage 0", ht.used, 0);
 *       test_equals_ptr("new table using jim hash", ht.hashfunc, hash_algorithm_jim);
 *       
 *       hash_init(&ht, hash_algorithm_jenkins);
 *       test_equals_ptr("new table using jenkins hash", ht.hashfunc, hash_algorithm_jenkins);
 *       
 *       return 0;
 *   }
 *
 *   struct unit_test_t unit_tests[] = {
 *       {NULL, testHashInit, NULL},
 *   };
 *
 *   int
 *   main(void)
 *   {
 *       size_t n = 0;
 *       for (n = 0; n < sizeof(unit_tests)/sizeof(unit_tests[0]); ++n) {
 *           run_tests(&unit_tests[n]);
 *       }
 *       test_print_results();
 *   }
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _MSC_VER
#ifndef FAKE_STDINT
#define FAKE_STDINT
typedef   signed __int8  int8_t;
typedef   signed __int16 int16_t;
typedef   signed __int32 int32_t;
typedef   signed __int64 int64_t;
typedef unsigned __int8  uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#endif /* FAKE_STDINT*/
#endif /* _MSC_VER */
#ifndef FAKE_STDINT
#include <stdint.h>
#endif

#ifdef __GNUC__
#define NOWARN __attribute__((unused))
#else
#define NOWARN
#endif
static void test_equals_int(const char *what, const int a, const int b) NOWARN;
static void test_equals_wide(const char *what, const uint64_t a, const uint64_t b) NOWARN;
static void test_equals_ptr(const char *what, const void *a, const void *b) NOWARN;
static void test_equals_str(const char *what, const char *a, const char *b) NOWARN;
static void test_equals_strn(const char *what, const char *a, const char *b, size_t n) NOWARN;
static void test_non_null_ptr(const char *what, const void *a) NOWARN;

typedef void * (unit_test_setup_t)();
typedef int    (unit_test_run_t)(void *);
typedef void   (unit_test_teardown_t)(void *);

struct unit_test_t {
    unit_test_setup_t *setUp;
    unit_test_run_t *runTest;
    unit_test_teardown_t *tearDown;
};

static int test_count = 0;
static int fail_count = 0;

#define RUN(what) printf("\t%s\n", what); test_count++;

static void test_fail(const char *what)
{
    printf("\tFAIL: %s\n", what);
    fflush(stdout);
    fail_count++;
}
static void test_equals_int(const char *what, const int a, const int b)
{
    RUN(what);
    if (a != b) test_fail(what);
}
static void test_equals_wide(const char *what, const uint64_t a, const uint64_t b)
{
    RUN(what);
    if (a != b) test_fail(what);
}
static void test_equals_ptr(const char *what, const void *a, const void *b)
{
    RUN(what);
    if (a != b) test_fail(what);
}
static void test_equals_str(const char *what, const char *a, const char *b)
{
    RUN(what);
    if (strcmp(a,b)) test_fail(what);
}
static void test_equals_strn(const char *what, const char *a, const char *b, size_t n)
{
    RUN(what);
    if (strncmp(a,b,n)) test_fail(what);
}
static void test_non_null_ptr(const char *what, const void *a)
{
    RUN(what);
    if (NULL == a) test_fail(what);
}

static void
test_print_results()
{
    printf("\t%d / %d pass\n", (test_count-fail_count),test_count);
}

static int
run_test(const struct unit_test_t *testPtr)
{
    int r = 0;
    void *clientData = NULL;
    if (testPtr->setUp)
        clientData = testPtr->setUp();
    r = testPtr->runTest(clientData);
    if (testPtr->tearDown)
        testPtr->tearDown(clientData);
    return r;
}

#define run_tests(tests) do {                                           \
    size_t n;                                                           \
    for (n = 0; n < sizeof(tests)/sizeof(tests[0]); ++n) {              \
        run_test(&tests[n]);                                            \
    }                                                                   \
    test_print_results();                                               \
 } while(0);
