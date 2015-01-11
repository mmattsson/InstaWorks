// --------------------------------------------------------------------------
///
/// @file iw_val_store.h
///
/// A module for handling values and keeping the values in a value store. A
/// value store is an object to keep track of name-value pairs for pre-defined
/// types, such as numbers, strings, or IP addresses. Values can be set or
/// accessed using the name of a value pair.
///
/// In its basic mode, a value store accepts any name to be inserted or
/// accessed. In the controlled mode, a value store only accepts pre-defined
/// names to be set or accessed. The pre-defined names must be defined by
/// function calls adding the name (and possibly an input validation criteria)
/// to the value store. Once the names are defined, they can be set just like
/// the basic mode value store. If the input validation criteria is set the
/// value must conform to the criteria in order to be set.
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _IW_VAL_STORE_H_
#define _IW_VAL_STORE_H_
#ifdef _cplusplus
extern "C" {
#endif

#include "iw_ip.h"
#include "iw_htable.h"

#include <stdbool.h>

// --------------------------------------------------------------------------

/// @brief The value types that can be inserted into the value store.
typedef enum {
    IW_VALUE_TYPE_NONE    = 0,
    IW_VALUE_TYPE_NUMBER  = 1,
    IW_VALUE_TYPE_STRING  = 2,
    IW_VALUE_TYPE_ADDRESS = 3
} IW_VALUE_TYPE;

// --------------------------------------------------------------------------

/// @brief The value object that can be inserted into the value store.
typedef struct _iw_value {
    char         *name; ///< The name of the value.
    IW_VALUE_TYPE type; ///< The type of the value.
    union {
        int   number;   ///< The number representation.
        char *string;   ///< The string representation.
        iw_ip address;  ///< The IP address representation.
    } v;    ///< The union structure.
} iw_value;

// --------------------------------------------------------------------------

/// @brief A value store object.
typedef struct _iw_val_store {
    iw_htable table;     ///< The hash table containing the value store values.
    bool      controlled;///< True if the value store is controlled.
} iw_val_store;

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// @brief Create a number value that can be stored in a value store.
/// @param name The name of the value to create.
/// @param num The number value of the value.
/// @return The created value or NULL for failure.
extern iw_value *iw_val_create_number(const char *name, int num);

// --------------------------------------------------------------------------

/// @brief Create a string value that can be stored in a value store.
/// @param name The name of the value to create.
/// @param str The string value of the value.
/// @return The created value or NULL for failure.
extern iw_value *iw_val_create_string(const char *name, const char *str);

// --------------------------------------------------------------------------

/// @brief Create an IP address value that can be stored in a value store.
/// @param name The name of the value to create.
/// @param address The IP address value of the value.
/// @return The created value or NULL for failure.
extern iw_value *iw_val_create_address(const char *name, const iw_ip *address);

// --------------------------------------------------------------------------

/// @brief Destroy a value.
/// @param val The value to destroy.
extern void iw_val_destroy(iw_value *val);

// --------------------------------------------------------------------------
//
// Value store creation/destruction
//
// --------------------------------------------------------------------------

/// @brief Initialize the given value store
/// @param store The store to initialize.
/// @return True if the store was successfully initialized.
extern bool iw_val_store_initialize(iw_val_store *store);

// --------------------------------------------------------------------------

/// @brief Destroy the given value store and all values stored in it.
/// @param store The store to destroy.
extern void iw_val_store_destroy(iw_val_store *store);

// --------------------------------------------------------------------------
//
// Value access functions
//
// --------------------------------------------------------------------------

/// @brief Set the value of the given value name.
/// @param store The store to set the value for.
/// @param name The name of the value to set.
/// @param value The value to set.
/// @return True if the value was successfully set.
extern bool iw_val_store_set(
    iw_val_store *store,
    const char *name,
    iw_value *value);

// --------------------------------------------------------------------------

/// @brief Set the number value of the given value name.
/// @param store The store to set the value for.
/// @param name The name of the value to set.
/// @param num The number value to set.
/// @return True if the value was successfully set.
extern bool iw_val_store_set_number(
    iw_val_store *store,
    const char *name,
    int num);

// --------------------------------------------------------------------------

/// @brief Set the string value of the given value name.
/// @param store The store to set the value for.
/// @param name The name of the value to set.
/// @param str The string value to set.
/// @return True if the value was successfully set.
extern bool iw_val_store_set_string(
    iw_val_store *store,
    const char *name,
    const char *str);

// --------------------------------------------------------------------------

/// @brief Set the IP address value of the given value name.
/// @param store The store to set the value for.
/// @param name The name of the value to set.
/// @param address The IP address value to set.
/// @return True if the value was successfully set.
extern bool iw_val_store_set_address(
    iw_val_store *store,
    const char *name,
    const iw_ip *address);

// --------------------------------------------------------------------------

/// @brief Get the value of the given value name.
/// @param store The store to get the value from.
/// @param name The name of the value to get.
/// @return The value of the given name.
extern iw_value *iw_val_store_get(
    iw_val_store *store,
    const char *name);

// --------------------------------------------------------------------------

/// @brief Get the number value of the given value name.
/// @param store The store to get the value from.
/// @param name The name of the value to get.
/// @return The number value of the given name.
extern int *iw_val_store_get_number(
    iw_val_store *store,
    const char *name);

// --------------------------------------------------------------------------

/// @brief Get the string value of the given value name.
/// @param store The store to get the value from.
/// @param name The name of the value to get.
/// @return The string value of the given name.
extern char *iw_val_store_get_string(
    iw_val_store *store,
    const char *name);

// --------------------------------------------------------------------------

/// @brief Get the IP address value of the given value name.
/// @param store The store to get the value from.
/// @param name The name of the value to get.
/// @return The IP address value of the given name.
extern iw_ip *iw_val_store_get_address(
    iw_val_store *store,
    const char *name);

// --------------------------------------------------------------------------
//
// Iteration through values in store
//
// --------------------------------------------------------------------------

/// @brief Get the first element of the value store.
/// Starts an iteration of the elements in the value store. The \p token
/// parameter is used to return a token of the element returned. This will
/// then be passed to the iw_store_val_get_next() function.
/// @param store The value store to get the next element from.
/// @param token [out] A variable to store the token of the found element.
/// @return The value of the first element in the value store or NULL at the end.
extern void *iw_store_val_get_first(iw_val_store *store, unsigned long *token);

// --------------------------------------------------------------------------

/// @brief Get the next element of the value store.
/// Each successive call will pass in the previously returned token.
/// @param store The value store to get the next element from.
/// @param token [in/out] A variable to store the token of the found element.
/// @return The value of the next element in the value store or NULL at the end.
extern void *iw_store_val_get_next(iw_val_store *store, unsigned long *token);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_VAL_STORE_H_

// --------------------------------------------------------------------------
