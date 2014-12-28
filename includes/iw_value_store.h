// --------------------------------------------------------------------------
///
/// @file iw_value_store.h
///
/// A module for handling values and keeping the values in a value store.
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _IW_VALUE_STORE_H_
#define _IW_VALUE_STORE_H_
#ifdef _cplusplus
extern "C" {
#endif

#include "iw_ip.h"
#include "iw_htable.h"

#include <stdbool.h>

// --------------------------------------------------------------------------

typedef enum {
    IW_VALUE_TYPE_NONE    = 0,
    IW_VALUE_TYPE_NUMBER  = 1,
    IW_VALUE_TYPE_STRING  = 2,
    IW_VALUE_TYPE_ADDRESS = 3
} IW_VALUE_TYPE;

// --------------------------------------------------------------------------

typedef struct _iw_value {
    char         *name;
    IW_VALUE_TYPE type;
    union {
        int   number;
        char *string;
        iw_ip address;
    } v;
} iw_value;

// --------------------------------------------------------------------------

typedef struct _iw_store {
    iw_htable table;
} iw_store;

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

extern iw_value *iw_val_create_number(const char *name, int num);

// --------------------------------------------------------------------------

extern iw_value *iw_val_create_string(const char *name, char *str);

// --------------------------------------------------------------------------

extern iw_value *iw_val_create_address(const char *name, iw_ip *address);

// --------------------------------------------------------------------------

extern void iw_val_destroy(iw_value *val);

// --------------------------------------------------------------------------
//
// Value store creation/destruction
//
// --------------------------------------------------------------------------

extern bool iw_val_store_init();

// --------------------------------------------------------------------------

extern void iw_val_store_destroy(iw_store *store);

// --------------------------------------------------------------------------
//
// Value access functions
//
// --------------------------------------------------------------------------

extern bool iw_val_store_set(
    iw_store *store,
    const char *name,
    iw_value *value);

// --------------------------------------------------------------------------

extern bool iw_val_store_set_number(
    iw_store *store,
    const char *name,
    int num);

// --------------------------------------------------------------------------

extern bool iw_val_store_set_string(
    iw_store *store,
    const char *name,
    const char *str);

// --------------------------------------------------------------------------

extern bool iw_val_store_set_address(
    iw_store *store,
    const char *name,
    const iw_ip *address);

// --------------------------------------------------------------------------

extern iw_value *iw_val_store_get(
    iw_store *store,
    const char *name);

// --------------------------------------------------------------------------

extern int *iw_val_store_get_number(
    iw_store *store,
    const char *name);

// --------------------------------------------------------------------------

extern char *iw_val_store_get_string(
    iw_store *store,
    const char *name);

// --------------------------------------------------------------------------

extern iw_ip *iw_val_store_get_address(
    iw_store *store,
    const char *name);

// --------------------------------------------------------------------------
//
// Iteration through values in store
//
// --------------------------------------------------------------------------

/// @brief Get the first element of the hash table.
/// Starts an iteration of the elements in the hash table. The \p hash
/// parameter is used to return the hash of the element returned. This will
/// then be passed to the iw_htable_get_next() function.
/// @param table The hash table to get the first element from.
/// @param hash [out] A variable to store the hash of the found element.
/// @return The data in the first element in the hash table.
extern void *iw_store_val_get_first(iw_store *store, unsigned long token);

// --------------------------------------------------------------------------

/// @brief Get the next element of the hash table.
/// @param table The hash table to get the next element from.
/// @param hash [in/out] A variable to store the hash of the found element.
/// @return The data in the next element in the hash table or NULL at the end.
extern void *iw_store_val_get_next(iw_store *store, unsigned long token);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_VALUE_STORE_H_

// --------------------------------------------------------------------------
