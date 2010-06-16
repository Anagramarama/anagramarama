#define UNIT_TEST
#include "unittest.h"
#define main ag_main
#include "ag.c"
#undef main

static int test_nextBlank()
{
    char a[7] = {'a','b',SPACE_CHAR,'c',SPACE_CHAR,'d', 0};
    test_equals_int("nextBlank", 3, nextBlank(a));
    test_equals_int("nextBlank substr", 2, nextBlank(a+3));
    test_equals_int("nextBlank no-blanks", 0, nextBlank("abcdef"));
    test_equals_int("nextBlank zero-length", 0, nextBlank(""));
    return 0;
}

static int test_shiftLeftKill()
{
    char a[7] = { 'a','b','c','d','e','f', 0 };
    test_equals_str("shiftLeftKill", "bcdef", shiftLeftKill(a));
    test_equals_str("shiftLeftKill const str", "bcdef", shiftLeftKill("abcdef"));
    return 0;
}

static int test_shiftLeft()
{
    char a[7] = { 'a','b','c','d','e','f', 0 };
    char b[2] = { 'a', 0 };
    test_equals_str("shiftLeft string", "bcdefa", shiftLeft(a));
    test_equals_str("shiftLeft short string", "a", shiftLeft(b));
    return 0;
}

static int test_swapChars()
{
    char a[7] = { 'a','b','c','d','e','f', 0 };
    const char *p = a, *q = NULL;
    test_equals_str("swapChars end", "fbcdea", swapChars(0, 5, a));
    q = swapChars(2, 3, a);
    test_equals_str("swapChars inner", "fbdcea", q);
    test_equals_ptr("swapChars ptr equiv", p, q);
    return 0;
}

static int test_revFirstNonSpace()
{
    char a[7] = { 'a','b','c','d','e','f', 0 };
    char b[7] = { 'a','b',SPACE_CHAR,'d','e',SPACE_CHAR, 0 };
    test_equals_int("rev no space", 6, revFirstNonSpace(a));
    test_equals_int("rev find space", 5, revFirstNonSpace(b));
    return 0;
}
 
static int test_whereinstr()
{
    test_equals_int("where is b", 1, whereinstr("abcdef",'b'));
    test_equals_int("where is f", 5, whereinstr("abcdef",'f'));
    test_equals_int("where is x", 0, whereinstr("abcdef",'x'));
    return 0;
}

struct unit_test_t unit_tests[] = {
    {NULL, test_shiftLeftKill, NULL},
    {NULL, test_shiftLeft, NULL},
    {NULL, test_nextBlank, NULL},
    {NULL, test_swapChars, NULL},
    {NULL, test_revFirstNonSpace, NULL},
    {NULL, test_whereinstr, NULL},
};

int
main(void)
{
    size_t n;
    for (n = 0; n < sizeof(unit_tests)/sizeof(unit_tests[0]); ++n) {
        run_tests(&unit_tests[n]);
    }
    test_print_results();
    return 0;
}
