// --------------------------------------------------------------------------
///
/// @file test_main.h
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _TEST_MAIN_H_
#define _TEST_MAIN_H_
#ifdef _cplusplus
extern "C" {
#endif

#include <stdbool.h>

// --------------------------------------------------------------------------

/// @brief The global test result structure.
typedef struct _test_result {
    unsigned int failed;    ///< The number of failed tests.
    unsigned int passed;    ///< The number of passed tests.
} test_result;

// --------------------------------------------------------------------------

/// @brief The test function definition.
typedef void (*TEST_FN)(test_result *result);

// --------------------------------------------------------------------------

/// @brief The test information object.
/// Contains information about the available tests.
typedef struct _test_info {
    TEST_FN fn; ///< A function pointer to the test to run.
    char *name; ///< The name of the function to run.
} test_info;

// --------------------------------------------------------------------------

/// @brief Test result function.
/// Called to validate test results.
/// @param result A pointer to the result object to be updated.
/// @param passed True if the test passed.
/// @param name The test being performed.
extern void test(test_result *result, bool passed, const char *name, ...);

// --------------------------------------------------------------------------

/// @brief Displays information about the test without updating results.
/// Displays test information, e.g. the test operation being performed, without
/// updating the results.
/// @param info The test information to display.
extern void test_display(const char *info, ...);

// --------------------------------------------------------------------------

/// @brief Displays information about the test without updating results.
/// Displays only the start of the test information entry. Can be followed
/// by multiple calls to test_disp_msg() and finalized with a call to
/// test_disp_end().
/// @param info The test information to display.
extern void test_disp_start(const char *info, ...);

// --------------------------------------------------------------------------

/// @brief Displays information about the test without updating results.
/// Displays information without finalizing the entry.
/// @param info The test information to display.
extern void test_disp_msg(const char *info, ...);

// --------------------------------------------------------------------------

/// @brief Ends the test display entry.
/// @param info The test information to display.
extern void test_disp_end(const char *info, ...);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _TESTS_H_

// --------------------------------------------------------------------------
