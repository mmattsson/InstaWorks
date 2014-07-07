    // --------------------------------------------------------------------------
///
/// @file test_list.c
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_list.h"
#include "iw_util.h"

#include "tests.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// --------------------------------------------------------------------------

static void validate_list(
    test_result *result, iw_list *list,
    unsigned int len, unsigned int *values)
{
    int cnt;
    test_disp_start("Validating list {");
    for(cnt=0;cnt < len;cnt++) {
        if(cnt > 0) {
            test_disp_msg(", ");
        }
        test_disp_msg("%d", values[cnt]);
    }
    test_disp_end("}");
    iw_list_gen_node *node = (iw_list_gen_node *)list->head;
    if(list->num_elems != len) {
        test(result, false, "List size is %d, expected %d",
             list->num_elems, len);
    }
    // Testing the list of elements
    for(cnt=0,node=(iw_list_gen_node *)list->head;
        cnt < len;
        cnt++, node=(iw_list_gen_node *)node->node.next)
    {
        if(node == NULL || (uintptr_t)node->data != values[cnt]) {
            test(result, false, "Node %d has value %d, expected %d",
                 cnt, node->data, values[cnt]);
            return;
        }
    }
    // Testing the tail pointer separately, the head pointer was tested
    // by the above loop.
    if((uintptr_t)((iw_list_gen_node *)list->tail)->data != values[len - 1]) {
        test(result, false, "List tail is pointing to value %d, expected %d",
             ((iw_list_gen_node *)list->tail)->data,
             values[len - 1]);
    }
    test(result, true, "List validated OK");
}

// --------------------------------------------------------------------------

void test_list(test_result *result) {
    iw_list list = IW_LIST_INIT;

    test(result, list.num_elems == 0, "Initalized list has zero elements");
    iw_list_remove(&list, NULL);
    test(result, list.num_elems == 0, "Removing element from empty list");

    test_display("Adding elements 1, 2, 3, and 4");
    iw_list_node *node1 = iw_list_add_data(&list, (void *)1);
    iw_list_node *node2 = iw_list_add_data(&list, (void *)2);
    iw_list_node *node3 = iw_list_add_data(&list, (void *)3);
    iw_list_node *node4 = iw_list_add_data(&list, (void *)4);
    unsigned int val_list1[] = { 1, 2, 3, 4 };
    validate_list(result, &list, IW_ARR_LEN(val_list1), val_list1);

    test_display("Deleting element 3 and 4");
    iw_list_delete(&list, node3, NULL);
    iw_list_delete(&list, node4, NULL);
    unsigned int val_list2[] = { 1, 2 };
    validate_list(result, &list, IW_ARR_LEN(val_list2), val_list2);

    test_display("Adding element 5");
    iw_list_add_data(&list, (void *)5);
    unsigned int val_list3[] = { 1, 2, 5 };
    validate_list(result, &list, IW_ARR_LEN(val_list3), val_list3);

    test_display("Removing element 1");
    iw_list_node *next = iw_list_remove(&list, node1);
    test(result, next == list.head && list.num_elems == 2, "Removed 1 element from list");
    free(node1);
    unsigned int val_list4[] = { 2, 5 };
    validate_list(result, &list, IW_ARR_LEN(val_list4), val_list4);

    iw_list_destroy(&list, NULL);
    test(result, list.num_elems == 0, "Destroyed list is empty");
    iw_list_init(&list, false);
    test(result, list.num_elems == 0, "Re-initialized list is empty");

    test_display("insert element");
    node2 = iw_list_insert_before_data(&list, NULL, (void *)2);
    unsigned int val_list5a[] = { 2 };
    validate_list(result, &list, IW_ARR_LEN(val_list5a), val_list5a);
    node1 = iw_list_insert_before_data(&list, node2, (void *)1);
    unsigned int val_list5b[] = { 1, 2 };
    validate_list(result, &list, IW_ARR_LEN(val_list5b), val_list5b);
    node3 = iw_list_insert_after_data(&list, node2, (void *)3);
    unsigned int val_list5c[] = { 1, 2, 3 };
    validate_list(result, &list, IW_ARR_LEN(val_list5c), val_list5c);

    iw_list_destroy(&list, NULL);
    test(result, list.num_elems == 0, "Destroyed list is empty");
    iw_list_init(&list, false);
    test(result, list.num_elems == 0, "Re-initialized list is empty");

    test_display("insert element 2 after NULL");
    node2 = iw_list_insert_after_data(&list, NULL, (void *)2);
    unsigned int val_list6a[] = { 2 };
    validate_list(result, &list, IW_ARR_LEN(val_list6a), val_list6a);
    test_display("insert element 3 after element 2");
    node3 = iw_list_insert_after_data(&list, node2, (void *)3);
    unsigned int val_list6b[] = { 2, 3 };
    validate_list(result, &list, IW_ARR_LEN(val_list6b), val_list6b);
    test_display("insert element 1 before element 2");
    node1 = iw_list_insert_before_data(&list, node2, (void *)1);
    unsigned int val_list6c[] = { 1, 2, 3 };
    validate_list(result, &list, IW_ARR_LEN(val_list6c), val_list6c);

    iw_list_destroy(&list, NULL);
    test(result, list.num_elems == 0, "Destroyed list is empty");
}

// --------------------------------------------------------------------------
