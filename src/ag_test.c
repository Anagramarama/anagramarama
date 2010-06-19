#ifndef UNIT_TEST
#define UNIT_TEST
#endif
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
    char *s = shiftLeftKill(a);
    test_equals_str("shiftLeftKill", "bcdef", s);
    free(s);
    s = shiftLeftKill("abcdef");
    test_equals_str("shiftLeftKill const str", "bcdef", s);
    free(s);
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
    {NULL, test_whereinstr, NULL},
};

int
main(void)
{
    run_tests(unit_tests);
    return 0;
}
