// --------------------------------------------------------------------------
///
/// @file tests.h
///
/// Copyright (C) Mattias Mattsson - 2014
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

/// @brief The list test suite.
/// @param result The result of the test.
extern void test_list(test_result *result);

/// @brief The buffer test suite.
/// @param result The result of the test.
extern void test_buff(test_result *result);

/// @brief The hash table test suite.
/// @param result The result of the test.
extern void test_hash_table(test_result *result);

/// @brief The syslog test suite.
/// @param result The result of the test.
extern void test_syslog(test_result *result);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _TESTS_H_

// --------------------------------------------------------------------------
