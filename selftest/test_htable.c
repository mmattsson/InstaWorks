// --------------------------------------------------------------------------
///
/// @file test_htable.c
///
/// Copyright (C) Mattias Mattsson - 2014
///
// --------------------------------------------------------------------------

#include "iw_htable.h"

#include "iw_hash.h"

#include "tests.h"

#include <stdlib.h>
#include <string.h>

// --------------------------------------------------------------------------

static void test_hash_node_delete(void *data) {
    free(data);
}

// --------------------------------------------------------------------------

void test_hash_table(test_result *result) {
    iw_htable table;
    char *data;

    test_display("Initializing hash table");
    iw_htable_init(&table, 4, iw_hash_data);
    test(result, table.num_elems == 0, "Initalized table has zero elements");
    iw_htable_delete(&table, 4, "abcd", test_hash_node_delete);
    test(result, table.num_elems == 0, "Removing element from empty table");

    test_display("Adding elements to hash table");
    iw_htable_insert(&table, 4, "abcd", strdup("1001"));
    test(result, table.num_elems == 1, "Added one element to table");

    iw_htable_delete(&table, 4, "abcd", test_hash_node_delete);
    test(result, table.num_elems == 0, "Removing last element from table");

    iw_htable_insert(&table, 4, "abcd", strdup("1001"));
    test_display("Added abcd->1001");
    iw_htable_insert(&table, 4, "efgh", strdup("1002"));
    test_display("Added efgh->1002");
    iw_htable_insert(&table, 4, "ijkl", strdup("1003"));
    test_display("Added ijkl->1003");
    iw_htable_insert(&table, 4, "mnop", strdup("1004"));
    test_display("Added mnop->1004");
    test(result, table.num_elems == 4, "Added 4 elements to table");

    test_display("Accessing elements from hash table");
    data = (char *)iw_htable_get(&table, 4, "abcd");
    test(result, data != NULL && strcmp("1001", data) == 0, "Accessing element 1 (abcd->1001)");
    data = (char *)iw_htable_get(&table, 4, "efgh");
    test(result, data != NULL && strcmp("1002", data) == 0, "Accessing element 2 (efgh->1002)");
    data = (char *)iw_htable_get(&table, 4, "ijkl");
    test(result, data != NULL && strcmp("1003", data) == 0, "Accessing element 3 (ijkl->1003)");
    data = (char *)iw_htable_get(&table, 4, "mnop");
    test(result, data != NULL && strcmp("1004", data) == 0, "Accessing element 4 (mnop->1004)");
    data = (char *)iw_htable_get(&table, 4, "qrst");
    test(result, data == NULL, "Accessing non-existent element");
    test_display("Number of collisions: %d", table.collisions);

    test_display("Adding existing value (abcd->1005)");
    char *value = strdup("1005");
    bool retval = iw_htable_insert(&table, 4, "abcd", value);
    if(!retval) {
        free(value);
    }
    test(result, retval == false, "Failed to add existing value?");

    test_display("Adding one more element (qrst->1005)");
    iw_htable_insert(&table, 4, "qrst", strdup("1005"));
    test(result, table.num_elems == 5, "Added 5th element to table");
    data = (char *)iw_htable_get(&table, 4, "qrst");
    test(result, data != NULL && strcmp("1005", data) == 0, "Accessing element 5 (qrst->1005)");
    test_display("Number of collisions: %d", table.collisions);

    test_display("Removing elements");
    iw_htable_delete(&table, 4, "efgh", test_hash_node_delete);
    test(result, table.num_elems == 4, "Deleted 2nd element from table (efgh)");
    data = (char *)iw_htable_get(&table, 4, "efgh");
    test(result, data == NULL, "Accessing element 2 (efgh->1002)");
    data = (char *)iw_htable_remove(&table, 4, "mnop");
    test(result, table.num_elems == 3, "Removed 4th element from table");
    test(result, data != NULL && strcmp("1004", data) == 0, "Removed element (mnop->1004)");
    test_hash_node_delete(data);
    data = (char *)iw_htable_get(&table, 4, "mnop");
    test(result, data == NULL, "Fail to access removed element 2 (mnop)");
    test_display("Number of collisions: %d", table.collisions);

    test_display("Destroying hash table");
    iw_htable_destroy(&table, test_hash_node_delete);
    test(result, table.num_elems == 0, "Destroyed table has zero elements");
}

// --------------------------------------------------------------------------
