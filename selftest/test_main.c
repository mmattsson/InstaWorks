// --------------------------------------------------------------------------
///
/// @file test_main.c
///
/// Copyright (c) 2014-2015 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "tests.h"

#include <iw_main.h>
#include <iw_cfg.h>

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

static void run_test(TEST_FN fn, const char *name, const char *desc) {
    test_result results = { 0, 0 };
    printf("  -- Running test -------------------------------\n");
    printf("    Running test \"%s: %s\"\n", name, desc);
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

static void run_tests(const char *test) {
    int cnt;
    bool did_run = false;
    for(cnt=0;s_tests[cnt].fn != NULL;cnt++) {
        if(test == NULL || strcmp(test, s_tests[cnt].name) == 0) {
            did_run = true;
            run_test(s_tests[cnt].fn, s_tests[cnt].name, s_tests[cnt].desc);
        }
    }
    if(test != NULL && !did_run) {
        printf(" No such test \'%s\'\n", test);
    } else {
        printf(" == Total Test Summary ==============================\n");
        if(s_results.failed > 0) {
            printf(" THERE WERE FAILED TESTS!\n");
        }
        printf(" Failed tests: %d\n", s_results.failed);
        printf(" Passed tests: %d\n", s_results.passed);
        printf(" Total tests:  %d\n", s_results.failed + s_results.passed);
    }
}

// --------------------------------------------------------------------------

static void print_tests() {
    int cnt;
    printf(" == Available Tests =================================\n");
    for(cnt=0;s_tests[cnt].fn != NULL;cnt++) {
        printf(" %-10s : %s\n", s_tests[cnt].name, s_tests[cnt].desc);
    }
    printf("\n");
}

// --------------------------------------------------------------------------

static void print_help() {
    printf("Usage: selftest [options] <cmd>\n"
            "Options can be:\n"
            "-v : Verbose, show all debug logs.\n"
            "\n"
            "Command can be:\n"
            "all   : Run all tests.\n"
            "show  : Show what tests are available.\n"
            "<test>: Run only this particular test.\n"
            "\n");
    exit(0);
}

// --------------------------------------------------------------------------

/// @brief The selftest main entrypoint.
/// @param argc The argument count.
/// @param argv The arguments.
int main(int argc, char **argv) {
    iw_cfg_init();

    int opt;
    while((opt = getopt(argc, argv, ":v")) != -1) {
        switch(opt) {
        case 'v':
            iw_val_store_set_number(&iw_cfg, IW_CFG_LOGLEVEL, 0xFF, NULL, 0);
            break;
        default :
            print_help();
            return 0;
        }
    }
    if(optind != argc - 1) {
        // Only one argument expected in addition to options.
        print_help();
        return 0;
    }

    char *test = NULL;
    if(strcmp(argv[optind], "show") == 0) {
        print_tests();
        return 0;
    } else if(strcmp(argv[optind], "all") != 0) {
        test = argv[optind];
    }

    // Disable memory tracking and health check thread to avoid false
    // positives and avoid hiding real issues in valgrind.
    iw_val_store_set_number(&iw_cfg, IW_CFG_MEMTRACK_ENABLE, 0, NULL, 0);
    iw_val_store_set_number(&iw_cfg, IW_CFG_HEALTHCHECK_ENABLE, 0, NULL, 0);
    iw_val_store_set_number(&iw_cfg, IW_CFG_WEBGUI_ENABLE, 0, NULL, 0);

    iw_init();
    printf(" == Running self-test ===============================\n");
    run_tests(test);
    printf(" == Completed self-test =============================\n");
    printf("\n");
    iw_exit();
    return (s_results.failed != 0) ? -1 : 0;
}

// --------------------------------------------------------------------------
