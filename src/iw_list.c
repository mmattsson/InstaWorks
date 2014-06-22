// --------------------------------------------------------------------------
///
/// @file iw_list.c
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_list.h"

#include "iw_memory.h"

#include <stdlib.h>
#include <string.h>

// TODO: Make the allocator/free functions configurable so that we can use
// the same list code for internal use as well as for users of the library.

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

void iw_list_init(iw_list *list) {
    memset(list, 0, sizeof(iw_list));
}

// --------------------------------------------------------------------------

iw_list_node *iw_list_add(iw_list *list, iw_list_node *node) {
    if(list->tail == NULL) {
        list->head = list->tail = node;
    } else {
        list->tail->next = node;
        node->prev = list->tail;
        node->next = NULL;
        list->tail = node;
    }
    list->num_elems++;
    return node;
}

// --------------------------------------------------------------------------

iw_list_node *iw_list_add_data(iw_list *list, void *data) {
    iw_list_gen_node *node = (iw_list_gen_node *)calloc(1, sizeof(iw_list));
    if(node == NULL) {
        return false;
    }
    node->data = data;
    return iw_list_add(list, (iw_list_node *)node);
}

// --------------------------------------------------------------------------

iw_list_node *iw_list_insert_before(
    iw_list *list,
    iw_list_node *insert,
    iw_list_node *node)
{
    if(insert == NULL) {
        node->next = list->head;
        list->head = node;
        if(list->tail == NULL) {
            list->tail = node;
        }
    } else {
        node->next = insert;
        node->prev = insert->prev;
        insert->prev = node;
        if(node->prev != NULL) {
            node->prev->next = node;
        } else {
            list->head = node;
        }
    }
    list->num_elems++;
    return node;
}

// --------------------------------------------------------------------------

iw_list_node *iw_list_insert_before_data(
    iw_list *list,
    iw_list_node *insert,
    void *data)
{
    iw_list_gen_node *node = (iw_list_gen_node *)calloc(1, sizeof(iw_list));
    if(node == NULL) {
        return false;
    }
    node->data = data;
    return iw_list_insert_before(list, insert, (iw_list_node *)node);
}

// --------------------------------------------------------------------------

iw_list_node *iw_list_insert_after(
    iw_list *list,
    iw_list_node *insert,
    iw_list_node *node)
{
    if(insert == NULL) {
        node->prev = list->tail;
        list->tail = node;
        if(list->head == NULL) {
            list->head = node;
        }
    } else {
        node->next = insert->next;
        insert->next = node;
        node->prev = insert;
        if(node->next != NULL) {
            node->next->prev = node;
        } else {
            list->tail = node;
        }
    }
    list->num_elems++;
    return node;
}

// --------------------------------------------------------------------------

iw_list_node *iw_list_insert_after_data(
    iw_list *list,
    iw_list_node *insert,
    void *data)
{
    iw_list_gen_node *node = (iw_list_gen_node *)calloc(1, sizeof(iw_list));
    if(node == NULL) {
        return false;
    }
    node->data = data;
    return iw_list_insert_after(list, insert, (iw_list_node *)node);
}

// --------------------------------------------------------------------------

iw_list_node *iw_list_remove(iw_list *list, iw_list_node *node) {
    if(node == NULL || list->head == NULL) {
        return NULL;
    }

    if(node->prev == NULL) {
        // This means node was the head of the list, we need to update
        // the head pointer
        list->head = node->next;
    } else {
        // The node was not the head of the list, just update the previous
        // node to point to the next node
        node->prev->next = node->next;
    }
    if(node->next == NULL) {
        // This means node was the tail of the list, we need to update
        // the tail pointer
        list->tail = node->prev;
    } else {
        // The node was not the tail of the list, just update the next
        // node to point to the previous node
        node->next->prev = node->prev;
    }
    iw_list_node *next = node->next;
    node->prev = NULL;
    node->next = NULL;
    list->num_elems--;
    return next;
}

// --------------------------------------------------------------------------

iw_list_node *iw_list_delete(
    iw_list *list,
    iw_list_node *node,
    IW_LIST_DEL_FN fn) 
{
    iw_list_node *next = iw_list_remove(list, node);
    if(fn != NULL) {
        fn(node);
    } else {
        free(node);
    }
    return next;
}

// --------------------------------------------------------------------------

void iw_list_destroy(iw_list *list, IW_LIST_DEL_FN fn) {
    iw_list_node *node = list->head;
    while(node != NULL) {
        iw_list_node *tmp = node->next;
        if(fn != NULL) {
            fn(node);
        } else {
            free(node);
        }
        node = tmp;
    }
    list->num_elems = 0;
}

// --------------------------------------------------------------------------
