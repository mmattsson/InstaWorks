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
/// The input criteria can be either a callback validation function or a
/// regular expression. If the value is a number or an address, then the value
/// is converted to a string before matched against the regular expression.
///
/// Note that numeric ranges are difficult to express as regular expressions so
/// an automatic generator is recommended, e.g.:
/// http://utilitymill.com/utility/Regex_For_Range
///
/// To set a range for a port number, the following code would be used:
/// <code>
///     iw_val_store_add_name_regexp(&store, "port", IW_VAL_TYPE_NUMBER,
///        "^([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$");
/// </code>
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
#include <sys/types.h>
#include <regex.h>

// --------------------------------------------------------------------------

/// The regular expression to use to specify a boolean (zero or one).
#define IW_VAL_CRIT_BOOL "^[0-1]$"

/// The regular expression to use to specify a port number (0-65535).
#define IW_VAL_CRIT_PORT "^([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$"

/// The regular expression to use to specify one character.
#define IW_VAL_CRIT_CHAR "^.$"
    
// --------------------------------------------------------------------------

/// @brief The value types that can be inserted into the value store.
typedef enum {
    IW_VAL_TYPE_NONE    = 0,
    IW_VAL_TYPE_NUMBER  = 1,
    IW_VAL_TYPE_STRING  = 2,
    IW_VAL_TYPE_ADDRESS = 3
} IW_VAL_TYPE;

// --------------------------------------------------------------------------

/// @brief The value object that can be inserted into the value store.
typedef struct _iw_val {
    char       *name; ///< The name of the value.
    IW_VAL_TYPE type; ///< The type of the value.
    union {
        int   number;   ///< The number representation.
        char *string;   ///< The string representation.
        iw_ip address;  ///< The IP address representation.
    } v;    ///< The union structure.
} iw_val;

// --------------------------------------------------------------------------

/// @brief A value criteria callback function.
/// A callback function to be called to validate values when being set.
typedef bool (*IW_VAL_CRITERIA_FN)(const char *name, iw_val *);

// --------------------------------------------------------------------------

/// @brief A criteria definition for a given value name.
typedef struct _iw_val_criteria {
    IW_VAL_TYPE        type;   ///< The type of the value.
    regex_t            regexp; ///< A regexp defining the criteria for the value.
    bool               regset; ///< True if the regexp is set.
    IW_VAL_CRITERIA_FN fn;     ///< A callback function defining the criteria.
} iw_val_criteria;

// --------------------------------------------------------------------------

/// @brief A value store object.
typedef struct _iw_val_store {
    iw_htable table;     ///< The hash table containing the value store values.
    iw_htable names;     ///< The table of pre-defined names for controlled stores.
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
extern iw_val *iw_val_create_number(const char *name, int num);

// --------------------------------------------------------------------------

/// @brief Create a string value that can be stored in a value store.
/// @param name The name of the value to create.
/// @param str The string value of the value.
/// @return The created value or NULL for failure.
extern iw_val *iw_val_create_string(const char *name, const char *str);

// --------------------------------------------------------------------------

/// @brief Create an IP address value that can be stored in a value store.
/// @param name The name of the value to create.
/// @param address The IP address value of the value.
/// @return The created value or NULL for failure.
extern iw_val *iw_val_create_address(const char *name, const iw_ip *address);

// --------------------------------------------------------------------------

/// @brief Convert a value to a string representation.
/// @param value The value to convert.
/// @param buff [out] The buffer to save the string in.
/// @param buff_len The length of the buffer.
/// @return True if the value was successfully converted.
extern bool iw_val_to_str(iw_val *value, char *buff, int buff_len);

// --------------------------------------------------------------------------

/// @brief Destroy a value.
/// @param val The value to destroy.
extern void iw_val_destroy(iw_val *val);

// --------------------------------------------------------------------------
//
// Value store creation/destruction
//
// --------------------------------------------------------------------------

/// @brief Initialize the given value store
/// @param store The store to initialize.
/// @param controlled True if only pre-defined names can be set.
/// @return True if the store was successfully initialized.
extern bool iw_val_store_initialize(iw_val_store *store, bool controlled);

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
    iw_val *value);

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
extern iw_val *iw_val_store_get(
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
extern void *iw_val_store_get_first(iw_val_store *store, unsigned long *token);

// --------------------------------------------------------------------------

/// @brief Get the next element of the value store.
/// Each successive call will pass in the previously returned token.
/// @param store The value store to get the next element from.
/// @param token [in/out] A variable to store the token of the found element.
/// @return The value of the next element in the value store or NULL at the end.
extern void *iw_val_store_get_next(iw_val_store *store, unsigned long *token);

// --------------------------------------------------------------------------
//
// Functions for adding pre-defined names to controlled value store.
//
// --------------------------------------------------------------------------

/// @brief Add a name to the controlled list of names that can be set.
/// @param store The controlled value store to add the name for.
/// @param type The type of the value that can be added.
/// @param name The name to add.
/// @return True if the name was successfully added.
extern bool iw_val_store_add_name(
    iw_val_store *store,
    const char *name,
    IW_VAL_TYPE type);

// --------------------------------------------------------------------------

/// @brief Add a name to the controlled list of names that can be set.
/// @param store The controlled value store to add the name for.
/// @param name The name to add.
/// @param type The type of the value that can be added.
/// @param fn The callback function for validation of values.
/// @return True if the name was successfully added.
extern bool iw_val_store_add_name_callback(
    iw_val_store *store,
    const char *name,
    IW_VAL_TYPE type,
    IW_VAL_CRITERIA_FN fn);

// --------------------------------------------------------------------------

/// @brief Add a name to the controlled list of names that can be set.
/// Uses a regular expression to validate the values being set for this name.
/// If the name is of type 'number' or 'address', then the value is converted
/// to a string before the regular expression is applied.
/// @param store The controlled value store to add the name for.
/// @param name The name to add.
/// @param type The type of the value that can be added.
/// @param regexp The regular expression to use for validation for this value.
/// @return True if the name was successfully added.
extern bool iw_val_store_add_name_regexp(
    iw_val_store *store,
    const char *name,
    IW_VAL_TYPE type,
    const char *regexp);

// --------------------------------------------------------------------------

/// @brief Delete a name from the controlled value store.
/// @param store The controlled value store to delete the name for.
/// @param name The name to delete.
extern void iw_val_store_delete_name(iw_val_store *store, const char *name);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_VAL_STORE_H_

// --------------------------------------------------------------------------
