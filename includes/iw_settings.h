// --------------------------------------------------------------------------
///
/// @file iw_settings.h
///
/// A module for handling values and keeping the values in a value store.
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _IW_SETTINGS_H_
#define _IW_SETTINGS_H_
#ifdef _cplusplus
extern "C" {
#endif

#include <stdbool.h>

// --------------------------------------------------------------------------

typedef enum {
    CFG_TYPE_NONE   = 0,
    CFG_TYPE_NUMBER = 1,
    CFG_TYPE_STRING = 2
} IW_VALUE_TYPE;

// --------------------------------------------------------------------------

typedef struct _iw_value {
    IW_VALUE_TYPE m_type;
    union {
        int num;
        char *str;
    } v;
} iw_value;

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

iw_value *iw_val_create_num(int num);

// --------------------------------------------------------------------------

iw_value *iw_val_create_str(char *str);

// --------------------------------------------------------------------------

void iw_val_destroy(iw_value *val);

// --------------------------------------------------------------------------

bool iw_val_init_store();

// --------------------------------------------------------------------------

void iw_val_destroy_store(iw_store *store);

// --------------------------------------------------------------------------

bool iw_val_set(iw_store *store, const char *name, iw_value *value);

// --------------------------------------------------------------------------

bool iw_val_set_num(iw_store *store, const char *name, int num);

// --------------------------------------------------------------------------

bool iw_val_set_str(iw_store *store, const char *name, const char *str);

// --------------------------------------------------------------------------

/// @brief Get the first element of the hash table.
/// Starts an iteration of the elements in the hash table. The \p hash
/// parameter is used to return the hash of the element returned. This will
/// then be passed to the iw_htable_get_next() function.
/// @param table The hash table to get the first element from.
/// @param hash [out] A variable to store the hash of the found element.
/// @return The data in the first element in the hash table.
extern void *iw_val_get_first(iw_store *store, unsigned long *hash);

// --------------------------------------------------------------------------

/// @brief Get the next element of the hash table.
/// @param table The hash table to get the next element from.
/// @param hash [in/out] A variable to store the hash of the found element.
/// @return The data in the next element in the hash table or NULL at the end.
extern void *iw_val_get_next(iw_store *store, unsigned long *hash);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_SETTINGS_H_

// --------------------------------------------------------------------------
