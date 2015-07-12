// --------------------------------------------------------------------------
///
/// @file iw_list.h
///
/// A basic list implementation.
///
/// Copyright (c) 2014-2015 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _IW_LIST_H_
#define _IW_LIST_H_
#ifdef _cplusplus
extern "C" {
#endif

#include <stdbool.h>

// --------------------------------------------------------------------------

/// @brief Initialize a list structure.
/// Do not use memory tracking for this list.
#define IW_LIST_INIT { 0, 0, 0, 0 };

/// @brief Initialize a list structure.
/// Use memory tracking for this list.
#define IW_LIST_INIT_MEM { 0, 0, 0, 1 };

// --------------------------------------------------------------------------

/// @brief The main list node structure.
typedef struct _iw_list_node {
    struct _iw_list_node *prev; ///< A pointer to the previous node.
    struct _iw_list_node *next; ///< A pointer to the next node.
} iw_list_node;

// --------------------------------------------------------------------------

/// @brief The generic list node structure.
/// This structure is used for a generic list containing pointers to data
/// to store. Note that the pointer can be used to store an integer as well.
typedef struct _iw_list_gen_node {
    iw_list_node node;  ///< The main node structure.
    void  *data;        ///< The data pointer.
} iw_list_gen_node;

// --------------------------------------------------------------------------

/// @brief The list structure.
typedef struct _iw_list {
    iw_list_node *head; ///< A pointer to the head of the list.
    iw_list_node *tail; ///< A pointer to the tail of the list.
    bool iw_mem_alloc;  ///< True if the IW memory allocation should be used.
    unsigned int num_elems; ///< The number of elements in the list.
} iw_list;

// --------------------------------------------------------------------------

/// @brief A list node deletion function.
/// A typedef of a list node deletion function to provide.
typedef void (*IW_LIST_DEL_FN)(iw_list_node *);

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// @brief Initialize a list structure.
/// If the memory allocator is used, any memory allocated by the list functions
/// will be allocated using the InstaWorks memory tracking feature, otherwise
/// the normal C API will be used.
/// @param list The list to initialize.
/// @param iw_mem_alloc True if the IW memory allocator should be used.
extern void iw_list_init(iw_list *list, bool iw_mem_alloc);

// --------------------------------------------------------------------------

/// @brief Return the number of elements in the list.
/// @param list The list to query for the number of elements.
/// @return The number of elements in the list.
extern int iw_list_num_elems(iw_list *list);

// --------------------------------------------------------------------------

/// @brief Add a node to a list.
/// @param list The list to add the node to.
/// @param node The node to add to the list.
/// @return The node that was added.
extern iw_list_node *iw_list_add(iw_list *list, iw_list_node *node);

// --------------------------------------------------------------------------

/// @brief Adds data to a list.
/// When adding the data, the iw_list_gen_node node structure will be used.
/// @param list The list to add the data to.
/// @param data The data to add.
/// @return The node that was added.
extern iw_list_node *iw_list_add_data(iw_list *list, void *data);

// --------------------------------------------------------------------------

/// @brief Insert a node into a list.
/// Inserts the element before the given element in the list. If \a insert
/// is NULL, the element is inserted first in the list.
/// @param list The list to add the node to.
/// @param insert The node to insert the new node before.
/// @param node The new node to insert into the list.
/// @return The node that was added.
extern iw_list_node *iw_list_insert_before(
    iw_list *list,
    iw_list_node *insert,
    iw_list_node *node);

// --------------------------------------------------------------------------

/// @brief Insert data into a list.
/// Inserts the element before the given element in the list. If \a insert
/// is NULL, the element is inserted first in the list.
/// @param list The list to add the node to.
/// @param insert The node to insert the new data before.
/// @param data The data to insert into the list.
/// @return The node that was added.
extern iw_list_node *iw_list_insert_before_data(
    iw_list *list,
    iw_list_node *insert,
    void *data);

// --------------------------------------------------------------------------

/// @brief Insert a node into a list.
/// Inserts the element after the given element in the list. If \a insert
/// is NULL, the element is inserted last in the list.
/// @param list The list to add the node to.
/// @param insert The node to insert the new node after.
/// @param node The new node to insert into the list.
/// @return The node that was added.
extern iw_list_node *iw_list_insert_after(
    iw_list *list,
    iw_list_node *insert,
    iw_list_node *node);

// --------------------------------------------------------------------------

/// @brief Insert data into a list.
/// Inserts the element after the given element in the list. If \a insert
/// is NULL, the element is inserted last in the list.
/// @param list The list to add the node to.
/// @param insert The node to insert the new data after.
/// @param data The data to insert into the list.
/// @return The node that was added.
extern iw_list_node *iw_list_insert_after_data(
    iw_list *list,
    iw_list_node *insert,
    void *data);

// --------------------------------------------------------------------------

/// @brief Remove a node from the list.
/// The node is removed but the memory is not freed. It is the callers
/// responsibility to free the memory.
/// @param list The list to remove the node from.
/// @param node The node to remove from the list.
/// @return The next node in the list for simplified deletion during iteration.
extern iw_list_node *iw_list_remove(iw_list *list, iw_list_node *node);

// --------------------------------------------------------------------------

/// @brief Delete a node from the list.
/// The node is removed and the memory it uses is freed. If no delete function
/// is given (i.e. if NULL is passed in for \a fn), the nodes themselves are
/// freed using free().
/// @param list The list to delete the node from.
/// @param node The node to delete.
/// @param fn The delete function to call to delete the node.
/// @return The next node in the list for simplified deletion during iteration.
extern iw_list_node *iw_list_delete(
    iw_list *list, 
    iw_list_node *node, 
    IW_LIST_DEL_FN fn);

// --------------------------------------------------------------------------

/// @brief Destroys a whole list.
/// The list and all the memory it uses is freed. If no delete function is
/// given (i.e. if NULL is passed in for \a fn), the nodes themselves are
/// freed using free().
/// @param list The list to destroy.
/// @param fn The delete function to call to delete the node.
extern void iw_list_destroy(iw_list *list, IW_LIST_DEL_FN fn);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_LIST_H_

// --------------------------------------------------------------------------
