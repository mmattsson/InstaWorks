// --------------------------------------------------------------------------
///
/// @file iw_htable.h
///
/// Copyright (C) Mattias Mattsson - 2014
///
// --------------------------------------------------------------------------

#ifndef _IW_HTABLE_H_
#define _IW_HTABLE_H_
#ifdef _cplusplus
extern "C" {
#endif

#include "iw_hash.h"

#include <stdbool.h>
#include <stdio.h>

// --------------------------------------------------------------------------
//
// Typedefs
//
// --------------------------------------------------------------------------

/// @brief A hash data deletion function.
/// A typedef of a hash data deletion function to provide.
typedef void (*IW_HASH_DEL_FN)(void *);

// --------------------------------------------------------------------------

/// @brief The hash table node data structure
typedef struct _iw_hash_node {
    struct _iw_hash_node *next; ///< The pointer to the next node.
    unsigned long hash;         ///< The hash key for this value.
    void         *data;         ///< The data stored for this key.
} iw_hash_node;

// --------------------------------------------------------------------------

/// @brief The hash table data structure
typedef struct _iw_hash_table {
    IW_HASH_FN     fn;          ///< The hash function to use for this table.
    unsigned int   size;        ///< The size of the hash table.
    unsigned int   num_elems;   ///< The current number of elements.
    unsigned int   collisions;  ///< The number of collisions in the table.
    iw_hash_node **table;       ///< The allocated array of buckets.
} iw_htable;

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// @brief Initialize a hash table.
/// The size of the hash table refers to the number of buckets that will be
/// used. The table can contain more elements than the size specifies but
/// these extra elements will be stored in a list in the same buckets. This
/// will degrade the performance of the hash table so that it degrades to
/// a linear list.
/// @param table The hash table to initialize.
/// @param table_size The size of the hash table.
/// @param hash_fn The hash function to use or NULL for the default.
/// @return True if the hash table was successfully created.
extern bool iw_htable_init(
    iw_htable *table,
    unsigned int table_size,
    IW_HASH_FN hash_fn);

// --------------------------------------------------------------------------

/// @brief Insert an element into the hash table.
/// @param table The table to insert the element into.
/// @param key_size The length of the key.
/// @param key The hash key to use.
/// @param data The data to insert.
/// @return True if the element was successfully inserted.
extern bool iw_htable_insert(
    iw_htable *table, 
    unsigned int key_size,
    void *key,
    void *data);

// --------------------------------------------------------------------------

/// @brief Get an element from a hash table.
/// This returns the element without removing it from the hash table.
/// @param table The table to get the element from.
/// @param key_len The length of the key.
/// @param key The hash key to use.
/// @return The data pointed to by the key or NULL if no match was found.
extern void *iw_htable_get(
    iw_htable *table,
    unsigned int key_len,
    const void *key);

// --------------------------------------------------------------------------

/// @brief Remove an element from a hash table.
/// This removes the element from the hash table.
/// @param table The table to remove the element from.
/// @param key_len The length of the key.
/// @param key The hash key to use.
/// @return The data pointed to by the key or NULL if no match was found.
extern void *iw_htable_remove(
    iw_htable *table,
    unsigned int key_len,
    const void *key);

// --------------------------------------------------------------------------

/// @brief Delete an element from a hash table.
/// This deletes the element from the hash table.
/// @param table The table to delete the element from.
/// @param key_len The length of the key.
/// @param key The hash key to use.
/// @param fn The entry deletion function to use or NULL if free() can be used.
/// @return True if the entry was found and deleted.
extern bool iw_htable_delete(
    iw_htable *table,
    unsigned int key_len,
    void *key,
    IW_HASH_DEL_FN fn);

// --------------------------------------------------------------------------

/// @brief Destroy a hash table.
/// @param table The table to destory.
/// @param fn The hash table data deletion function.
extern void iw_htable_destroy(iw_htable *table, IW_HASH_DEL_FN fn);

// --------------------------------------------------------------------------

/// @brief Get the first element of the hash table.
/// Starts an iteration of the elements in the hash table. The \p hash 
/// parameter is used to return the hash of the element returned. This will
/// then be passed to the iw_htable_get_next() function.
/// @param table The hash table to get the first element from.
/// @param hash [out] A variable to store the hash of the found element.
/// @return The data in the first element in the hash table.
extern void *iw_htable_get_first(iw_htable *table, unsigned long *hash);

// --------------------------------------------------------------------------

/// @brief Get the next element of the hash table.
/// @param table The hash table to get the next element from.
/// @param hash [in/out] A variable to store the hash of the found element.
/// @return The data in the next element in the hash table or NULL at the end.
extern void *iw_htable_get_next(iw_htable *table, unsigned long *hash);

// --------------------------------------------------------------------------

/// @brief Print a report on the given hash table.
/// @param table The table to print the report on.
/// @param out The file stream to print the report on.
extern void iw_htable_report(iw_htable *table, FILE *out);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_HTABLE_H_

// --------------------------------------------------------------------------
