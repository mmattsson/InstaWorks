// --------------------------------------------------------------------------
///
/// @file test_value_store.c
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_val_store.h"

#include "iw_util.h"

#include "tests.h"

#include <stdlib.h>
#include <string.h>

// --------------------------------------------------------------------------

static iw_val_store store;

// --------------------------------------------------------------------------

static void test_get_value_failure(
    test_result *result,
    char *name,
    IW_VALUE_TYPE type)
{
    int *num;
    char *str;
    switch(type) {
    case IW_VALUE_TYPE_NUMBER :
        num = iw_val_store_get_number(&store, name);
        test(result, num == NULL,
            "Access '%s' as number, expected NULL", name);
        break;
    case IW_VALUE_TYPE_STRING :
        str = iw_val_store_get_string(&store, name);
        test(result, str == NULL,
            "Access '%s' as string, expected NULL", name);
        break;
    default :
        test(result, false, "Test not implemented");
        break;
    }
}

// --------------------------------------------------------------------------

static void test_get_num_value(test_result *result, char *name, int value) {
    int *num = iw_val_store_get_number(&store, name);
    if(num == NULL) {
        test(result, num != NULL,
            "Access '%s' as number, expected %d, got NULL",
            name, value);
    } else {
        test(result, *num == value,
            "Access '%s' as number, expected %d, got %d",
            name, value, *num);
    }
}

// --------------------------------------------------------------------------

static void test_get_str_value(test_result *result, char *name, char *value) {
    char *str = iw_val_store_get_string(&store, name);
    if(str == NULL) {
        test(result, str != NULL,
            "Access '%s' as string, expected %d, got NULL",
            name, value);
    } else {
        test(result, strcmp(str, value) == 0,
            "Access '%s' as string, expected '%s', got '%s'",
            name, value, str);
    }
}

// --------------------------------------------------------------------------

static void test_insert_values(
    test_result *result,
    int num_values,
    IW_VALUE_TYPE type)
{
    int cnt;
    char name_buff[16];
    char value_buff[16];
    for(cnt=0;cnt < num_values;cnt++) {
        if(type == IW_VALUE_TYPE_NUMBER) {
            snprintf(name_buff, sizeof(name_buff), "num_%d", cnt);
            iw_value *value = iw_val_create_number(name_buff, cnt);
            bool ret = iw_val_store_set(&store, name_buff, value);
            test(result, ret, "Successfully inserted '%s'->%d",
                 name_buff, cnt);
        } else if(type == IW_VALUE_TYPE_STRING) {
            snprintf(name_buff, sizeof(name_buff), "str_%d", cnt);
            snprintf(value_buff, sizeof(value_buff), "str_%d", cnt);
            iw_value *value = iw_val_create_string(name_buff, value_buff);
            bool ret = iw_val_store_set(&store, name_buff, value);
            test(result, ret, "Successfully inserted '%s'->'%s'",
                 name_buff, value_buff);
        }
    }
}

// --------------------------------------------------------------------------

void test_value_store(test_result *result) {
    test_display("Initializing value store");

    iw_val_store_initialize(&store);

    test_insert_values(result, 6, IW_VALUE_TYPE_NUMBER);
    test_insert_values(result, 6, IW_VALUE_TYPE_STRING);
    test_get_num_value(result, "num_1", 1);
    test_get_num_value(result, "num_2", 2);
    test_get_num_value(result, "num_3", 3);
    test_get_value_failure(result, "num_7", IW_VALUE_TYPE_NUMBER);
    test_get_value_failure(result, "str_1", IW_VALUE_TYPE_NUMBER);
    test_get_str_value(result, "str_1", "str_1");
    test_get_str_value(result, "str_2", "str_2");
    test_get_str_value(result, "str_3", "str_3");
    test_get_value_failure(result, "str_7", IW_VALUE_TYPE_STRING);
    test_get_value_failure(result, "num_3", IW_VALUE_TYPE_STRING);

    test_display("Re-initializing value store");
    iw_val_store_destroy(&store);
    iw_val_store_initialize(&store);
    test_get_value_failure(result, "num_1", IW_VALUE_TYPE_NUMBER);
    test_get_value_failure(result, "str_1", IW_VALUE_TYPE_STRING);
}

// --------------------------------------------------------------------------
