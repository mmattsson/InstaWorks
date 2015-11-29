// --------------------------------------------------------------------------
///
/// @file test_value_store.c
///
/// Copyright (c) 2014-2015 Mattias Mattsson. All rights reserved.
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
    IW_VAL_TYPE type)
{
    int *num;
    char *str;
    switch(type) {
    case IW_VAL_TYPE_NUMBER :
        num = iw_val_store_get_number(&store, name);
        test(result, num == NULL,
            "Access '%s' as number, expected NULL", name);
        break;
    case IW_VAL_TYPE_STRING :
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

static void test_insert_value(
    test_result *result,
    const char *name,
    int value_index,
    IW_VAL_TYPE type,
    bool should_succeed)
{
    char value_buff[16];
    if(type == IW_VAL_TYPE_NUMBER) {
        IW_VAL_RET ret = iw_val_store_set_number(&store, name, value_index, NULL, 0);
        test(result, ((ret == IW_VAL_RET_OK) && should_succeed) ||
                     ((ret != IW_VAL_RET_OK) && !should_succeed),
             "%s to insert '%s'->%d",
             should_succeed ? "Succeeded" : "Failed",
             name, value_index);
    } else if(type == IW_VAL_TYPE_STRING) {
        snprintf(value_buff, sizeof(value_buff), "str_%d", value_index);
        IW_VAL_RET ret = iw_val_store_set_string(&store, name, value_buff, NULL, 0);
        test(result, ((ret == IW_VAL_RET_OK) && should_succeed) ||
                     ((ret != IW_VAL_RET_OK) && !should_succeed),
             "%s to insert '%s'->'%s'",
             should_succeed ? "Succeeded" : "Failed",
             name, value_buff);
    }
}

// --------------------------------------------------------------------------

static void test_insert_values(
    test_result *result,
    int num_values,
    IW_VAL_TYPE type,
    bool should_succeed)
{
    int cnt;
    for(cnt=0;cnt < num_values;cnt++) {
        char buff[128];
        switch(type) {
        case IW_VAL_TYPE_NUMBER :
            snprintf(buff, sizeof(buff), "num_%d", cnt);
            break;
        case IW_VAL_TYPE_STRING :
            snprintf(buff, sizeof(buff), "str_%d", cnt);
            break;
        default :
            return;
        }
        test_insert_value(result, buff, cnt, type, should_succeed);
    }
}

// --------------------------------------------------------------------------

void test_value_store(test_result *result) {
    test_display("Initializing value store");

    iw_val_store_initialize(&store, false);

    test_insert_values(result, 6, IW_VAL_TYPE_NUMBER, true);
    test_insert_values(result, 6, IW_VAL_TYPE_STRING, true);
    test_get_num_value(result, "num_1", 1);
    test_get_num_value(result, "num_2", 2);
    test_get_num_value(result, "num_3", 3);
    test_get_value_failure(result, "num_7", IW_VAL_TYPE_NUMBER);
    test_get_value_failure(result, "str_1", IW_VAL_TYPE_NUMBER);
    test_display("Testing overwriting values");
    test_insert_value(result, "num_4", 4, IW_VAL_TYPE_NUMBER, true);
    test_insert_value(result, "num_5", 5, IW_VAL_TYPE_NUMBER, true);
    test_get_str_value(result, "str_1", "str_1");
    test_get_str_value(result, "str_2", "str_2");
    test_get_str_value(result, "str_3", "str_3");
    test_get_value_failure(result, "str_7", IW_VAL_TYPE_STRING);
    test_get_value_failure(result, "num_3", IW_VAL_TYPE_STRING);
    test_insert_value(result, "num_4", 4, IW_VAL_TYPE_STRING, true);
    test_insert_value(result, "num_5", 5, IW_VAL_TYPE_STRING, true);

    test_display("Re-initializing value store as 'controlled'");
    iw_val_store_destroy(&store);
    iw_val_store_initialize(&store, true);
    test_get_value_failure(result, "num_1", IW_VAL_TYPE_NUMBER);
    test_get_value_failure(result, "str_1", IW_VAL_TYPE_STRING);

    test_display("Inserting non-pre-defined values");
    test_insert_values(result, 3, IW_VAL_TYPE_NUMBER, false);

    test_display("Adding a value (num_1) that can be between 0..65535 (a port number)");
    iw_val_store_add_name_regexp(&store, "num_1", NULL, IW_VAL_TYPE_NUMBER,
        "^([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$", false);
    test_insert_value(result, "num_1", 67000, IW_VAL_TYPE_STRING, false);
    test_insert_value(result, "num_1", -1, IW_VAL_TYPE_NUMBER, false);
    test_insert_value(result, "num_1", 67000, IW_VAL_TYPE_NUMBER, false);
    test_insert_value(result, "num_1", 1234, IW_VAL_TYPE_NUMBER, true);
    test_get_num_value(result, "num_1", 1234);
    test_insert_value(result, "num_1", 65535, IW_VAL_TYPE_NUMBER, true);
    test_get_num_value(result, "num_1", 65535);
    test_insert_value(result, "num_1", 65536, IW_VAL_TYPE_NUMBER, false);
    test_get_num_value(result, "num_1", 65535);
}

// --------------------------------------------------------------------------
