#ifndef UNIT_TEST
#define UNIT_TEST
#endif
#include "unittest.h"

#define main ag_main
#include "ag.c"
#undef main

static int test_whereinstr()
{
    test_equals_int("where is b", 1, whereinstr("abcdef",'b'));
    test_equals_int("where is f", 5, whereinstr("abcdef",'f'));
    test_equals_int("where is x", 0, whereinstr("abcdef",'x'));
    return 0;
}

struct unit_test_t unit_tests[] = {
    {NULL, test_whereinstr, NULL},
};

int
main(void)
{
    run_tests(unit_tests);
    return 0;
}
