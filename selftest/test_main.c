// --------------------------------------------------------------------------
///
/// @file test_main.c
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "tests.h"

#include <iw_main.h>
#include <iw_settings.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// --------------------------------------------------------------------------

/// The number of space characters to use in the test information column.
#define TEST_SPACE      60

// --------------------------------------------------------------------------

static test_result s_results = { 0, 0 };

// --------------------------------------------------------------------------

void test(test_result *result, bool passed, const char *test, ...) {
    printf("    Test: ");
    va_list ap;
    va_start(ap, test);
    int length = vprintf(test, ap);
    va_end(ap);

    int cnt = 0;
    for(;cnt < TEST_SPACE - length;cnt++) {
        printf(" ");
    }
    printf(" : %s\n", passed ? "PASS" : "FAIL");
    if(passed) {
        result->passed++;
    } else {
        result->failed++;
    }
}

// --------------------------------------------------------------------------

void test_display(const char *info, ...) {
    printf("    Info: ");
    va_list ap;
    va_start(ap, info);
    vprintf(info, ap);
    va_end(ap);
    printf("\n");
}

// --------------------------------------------------------------------------

void test_disp_start(const char *info, ...) {
    printf("    Info: ");
    va_list ap;
    va_start(ap, info);
    vprintf(info, ap);
    va_end(ap);
}

// --------------------------------------------------------------------------

void test_disp_msg(const char *info, ...) {
    va_list ap;
    va_start(ap, info);
    vprintf(info, ap);
    va_end(ap);
}

// --------------------------------------------------------------------------

void test_disp_end(const char *info, ...) {
    va_list ap;
    va_start(ap, info);
    vprintf(info, ap);
    va_end(ap);
    printf("\n");
}

// --------------------------------------------------------------------------

static void run_test(TEST_FN fn, const char *name) {
    test_result results = { 0, 0 };
    printf("  -- Running test -------------------------------\n");
    printf("    Running test \"%s\"\n", name);
    (*fn)(&results);
    printf("  -- Summary ------------------------------------\n");
    printf("    Failed tests: %d\n", results.failed);
    printf("    Passed tests: %d\n", results.passed);
    printf("    Total tests:  %d\n", results.failed + results.passed);
    printf("  -- Done running test --------------------------\n");
    printf("\n");
    s_results.failed += results.failed;
    s_results.passed += results.passed;
}

// --------------------------------------------------------------------------

static void run_tests() {
    int cnt;
    for(cnt=0;s_tests[cnt].fn != NULL;cnt++) {
        run_test(s_tests[cnt].fn, s_tests[cnt].name);
    }
    printf(" == Total Test Summary ==============================\n");
    if(s_results.failed > 0) {
        printf(" THERE WERE FAILED TESTS!\n");
    }
    printf(" Failed tests: %d\n", s_results.failed);
    printf(" Passed tests: %d\n", s_results.passed);
    printf(" Total tests:  %d\n", s_results.failed + s_results.passed);
}

// --------------------------------------------------------------------------

static void print_help() {
    printf("Usage: selftest [options]\n"
            "Options can be:\n"
            "-v : Verbose, show all debug logs\n"
            "\n");
    exit(0);
}

// --------------------------------------------------------------------------

/// @brief The selftest main entrypoint.
/// @param argc The argument count.
/// @param argv The arguments.
int main(int argc, char **argv) {
    if(argc > 2 || (argc == 2 && strcmp(argv[1], "-v") != 0)) {
        print_help();
    }

    int opt;
    while((opt = getopt(argc, argv, ":v")) != -1) {
        switch(opt) {
        case 'v':
            iw_stg.iw_log_level = 0xFF;
            break;
        default :
            print_help();
            break;
        }
    }
    if(optind < argc) {
        // No arguments expected in addition to options.
        print_help();
    }

    // Disable memory tracking and health check thread to avoid false
    // positives and avoid hiding real issues in valgrind.
    iw_stg.iw_enable_memtrack    = false;
    iw_stg.iw_enable_healthcheck = false;
    iw_stg.iw_enable_web_server  = false;

    iw_init();
    printf(" == Running self-test ===============================\n");
    run_tests();
    printf(" == Completed self-test =============================\n");
    printf("\n");
    iw_exit();
    return (s_results.failed != 0) ? -1 : 0;
}

// --------------------------------------------------------------------------
