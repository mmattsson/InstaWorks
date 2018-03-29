// --------------------------------------------------------------------------
///
/// @file tests.h
///
/// Copyright (c) 2014-2018 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _TESTS_H_
#define _TESTS_H_
#ifdef _cplusplus
extern "C" {
#endif

#include "test_main.h"

// --------------------------------------------------------------------------

/// The array containing all the tests.
extern test_info s_tests[];

// --------------------------------------------------------------------------
//
// Tests
//
// --------------------------------------------------------------------------

/// @brief The buffer test suite.
/// @param result The result of the test.
extern void test_buff(test_result *result);

/// @brief The hash table test suite.
/// @param result The result of the test.
extern void test_hash_table(test_result *result);

/// @brief The IP address utility test suite.
/// @param result The result of the test.
extern void test_ip(test_result *result);

/// @brief The list test suite.
/// @param result The result of the test.
extern void test_list(test_result *result);

/// @brief The command-line options test suite.
/// @param result The result of the test.
extern void test_opts(test_result *result);

/// @brief The syslog test suite.
/// @param result The result of the test.
extern void test_syslog(test_result *result);

/// @brief The utilities test suite.
/// Tests miscellaneous functions in the iw_util.c module.
/// @param result The result of the test.
extern void test_util(test_result *result);

/// @brief The value store test suite.
/// @param result The result of the test.
extern void test_value_store(test_result *result);

/// @brief The web server test suite.
/// @param result The result of the test.
extern void test_web_srv(test_result *result);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _TESTS_H_

// --------------------------------------------------------------------------
