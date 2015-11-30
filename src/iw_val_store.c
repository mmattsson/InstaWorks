// --------------------------------------------------------------------------
///
/// @file iw_val_store.c
///
/// Copyright (c) 2014-2015 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_val_store.h"

#include "iw_memory.h"
#include "iw_util.h"

#include <stdlib.h>
#include <string.h>

// --------------------------------------------------------------------------
//
// Internal structures.
//
// --------------------------------------------------------------------------

/// @brief A criteria definition for a given value name.
typedef struct _iw_val_criteria {
    IW_VAL_TYPE        type;   ///< The type of the value.
    bool               persist;///< True if the value should be persisted.
    regex_t            regexp; ///< A regexp defining the criteria for the value.
    bool               regset; ///< True if the regexp is set.
    IW_VAL_CRITERIA_FN fn;     ///< A callback function defining the criteria.
    char              *msg;    ///< The message to display regarding valid range.
} iw_val_criteria;

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

iw_val *iw_val_create_number(const char *name, int num) {
    iw_val *value = calloc(1, sizeof(iw_val));
    if(value == NULL) {
        return NULL;
    }
    value->name = strdup(name);
    value->type = IW_VAL_TYPE_NUMBER;
    value->v.number = num;
    if(value->name == NULL) {
        iw_val_destroy(value);
    }
    return value;
}

// --------------------------------------------------------------------------

iw_val *iw_val_create_string(const char *name, const char *str) {
    iw_val *value = calloc(1, sizeof(iw_val));
    if(value == NULL) {
        return NULL;
    }
    value->name = strdup(name);
    value->type = IW_VAL_TYPE_STRING;
    value->v.string = strdup(str);
    if(value->name == NULL || value->v.string == NULL) {
        iw_val_destroy(value);
    }
    return value;
}

// --------------------------------------------------------------------------

iw_val *iw_val_create_address(const char *name, const iw_ip *address) {
    iw_val *value = calloc(1, sizeof(iw_val));
    if(value == NULL) {
        return NULL;
    }
    value->name = strdup(name);
    value->type = IW_VAL_TYPE_ADDRESS;
    memcpy(&value->v.address, address, sizeof(iw_ip));
    if(value->name == NULL || value->v.string == NULL) {
        iw_val_destroy(value);
    }
    return value;
}

// --------------------------------------------------------------------------

void iw_val_destroy(iw_val *value) {
    free(value->name);
    if(value->type == IW_VAL_TYPE_STRING) {
        free(value->v.string);
    }
    free(value);
}

// --------------------------------------------------------------------------

bool iw_val_to_str(iw_val *value, char *buff, size_t buff_len) {
    switch(value->type) {
    case IW_VAL_TYPE_NUMBER :
        snprintf(buff, buff_len, "%d", value->v.number);
        return true;
    case IW_VAL_TYPE_STRING :
        snprintf(buff, buff_len, "%s", value->v.string);
        return true;
    case IW_VAL_TYPE_ADDRESS :
        return iw_ip_addr_to_str(&value->v.address, true,
                                 buff, buff_len) != NULL;
    default :
        return false;
    }
}

// --------------------------------------------------------------------------
//
// Value store creation/destruction
//
// --------------------------------------------------------------------------

static void iw_val_destroy_value(void *value) {
    iw_val_destroy((iw_val *)value);
}

// --------------------------------------------------------------------------

bool iw_val_store_initialize(iw_val_store *store, bool controlled) {
    if(!iw_htable_init(&store->table, 1024, false, NULL)) {
        return false;
    }
    store->controlled = controlled;
    if(controlled) {
        if(!iw_htable_init(&store->names, 1024, false, NULL)) {
            iw_htable_destroy(&store->table, iw_val_destroy_value);
            return false;
        }
    }
    return true;
}

// --------------------------------------------------------------------------

void iw_val_store_destroy(iw_val_store *store) {
    iw_htable_destroy(&store->table, iw_val_destroy_value);
}

// --------------------------------------------------------------------------
//
// Value access functions
//
// --------------------------------------------------------------------------

IW_VAL_RET iw_val_store_set(
    iw_val_store *store,
    const char *name,
    iw_val *value,
    char *err_buff,
    int buff_size)
{
    if(err_buff != NULL && buff_size > 0) {
        err_buff[0] = '\0';
    }
    if(store->controlled) {
        // We must check whether the name is allowed to be set and if the
        // provided value fits the given criteria.
        iw_val_criteria *crit = (iw_val_criteria *)iw_htable_get(&store->names,
                                                                 strlen(name),
                                                                 name);
        if(crit == NULL) {
            // No pre-defined name, cannot set this value.
            if(err_buff != NULL) {
                snprintf(err_buff, buff_size, "No such value");
            }
            return IW_VAL_RET_NO_SUCH_VALUE;
        }
        if(crit->type != value->type) {
            // The value is of the wrong type, cannot set this value.
            if(err_buff != NULL) {
                snprintf(err_buff, buff_size, "Incorrect type for value");
            }
            return IW_VAL_RET_INCORRECT_TYPE;
        }
        if(crit->fn != NULL) {
            bool ret = (crit->fn)(name, value);
            if(!ret) {
                // The validation function failed, cannot set this value.
                if(err_buff != NULL) {
                    snprintf(err_buff, buff_size, "Invalid value format");
                }
                return IW_VAL_RET_FAILED_CALLBACK;
            }
        }
        if(crit->regset) {
            char buff[IW_IP_BUFF_LEN];
            char *buffer = NULL;
            switch(value->type) {
            case IW_VAL_TYPE_STRING :
                buffer = value->v.string;
                break;
            case IW_VAL_TYPE_NUMBER :
                snprintf(buff, sizeof(buff), "%d", value->v.number);
                buffer = buff;
                break;
            case IW_VAL_TYPE_ADDRESS :
                iw_ip_addr_to_str(&value->v.address, true, buff, sizeof(buff));
                buffer = buff;
                break;
            default :
                break;
            }
            if(buffer != NULL &&
               regexec(&crit->regexp, buffer, 0, NULL, 0) != 0)
            {
                // The regexp did not match, cannot set this value.
                if(err_buff != NULL) {
                    snprintf(err_buff, buff_size, "Invalid value format");
                }
                return IW_VAL_RET_FAILED_REGEXP;
            }
        }
    }
    return iw_htable_replace(&store->table, strlen(name), name, value,
                             iw_val_destroy_value) ?
                                IW_VAL_RET_OK : IW_VAL_RET_FAILED_TO_CREATE;
}

// --------------------------------------------------------------------------

IW_VAL_RET iw_val_store_set_number(
    iw_val_store *store,
    const char *name,
    int num,
    char *err_buff,
    int buff_size)
{
    IW_VAL_RET ret;
    iw_val *value = iw_val_create_number(name, num);
    if(err_buff != NULL && buff_size > 0) {
        err_buff[0] = '\0';
    }
    if(value == NULL) {
        if(err_buff != NULL) {
            snprintf(err_buff, buff_size, "Failed to create number");
        }
        return IW_VAL_RET_FAILED_TO_CREATE;
    }
    if((ret = iw_val_store_set(store, name, value, err_buff, buff_size)) != IW_VAL_RET_OK) {
        iw_val_destroy(value);
        return ret;
    }
    return IW_VAL_RET_OK;
}

// --------------------------------------------------------------------------

IW_VAL_RET iw_val_store_set_string(
    iw_val_store *store,
    const char *name,
    const char *str,
    char *err_buff,
    int buff_size)
{
    IW_VAL_RET ret;
    iw_val *value = iw_val_create_string(name, str);
    if(err_buff != NULL && buff_size > 0) {
        err_buff[0] = '\0';
    }
    if(value == NULL) {
        if(err_buff != NULL) {
            snprintf(err_buff, buff_size, "Failed to create string");
        }
        return IW_VAL_RET_FAILED_TO_CREATE;
    }
    if((ret = iw_val_store_set(store, name, value, err_buff, buff_size)) != IW_VAL_RET_OK) {
        iw_val_destroy(value);
        return ret;
    }
    return IW_VAL_RET_OK;
}

// --------------------------------------------------------------------------

IW_VAL_RET iw_val_store_set_address(
    iw_val_store *store,
    const char *name,
    const iw_ip *address,
    char *err_buff,
    int buff_size)
{
    IW_VAL_RET ret;
    iw_val *value = iw_val_create_address(name, address);
    if(err_buff != NULL && buff_size > 0) {
        err_buff[0] = '\0';
    }
    if(value == NULL) {
        if(err_buff != NULL) {
            snprintf(err_buff, buff_size, "Failed to create address");
        }
        return IW_VAL_RET_FAILED_TO_CREATE;
    }
    if((ret = iw_val_store_set(store, name, value, err_buff, buff_size)) != IW_VAL_RET_OK) {
        iw_val_destroy(value);
        return false;
    }
    return IW_VAL_RET_OK;
}

// --------------------------------------------------------------------------

IW_VAL_RET iw_val_store_set_existing_value(
    iw_val_store *store,
    const char *name,
    const char *value,
    char *err_buff,
    int buff_size)
{
    iw_val *val = iw_val_store_get(store, name);
    if(err_buff != NULL && buff_size > 0) {
        err_buff[0] = '\0';
    }
    if(val == NULL) {
        if(err_buff != NULL) {
            snprintf(err_buff, buff_size, "No such value");
        }
        return IW_VAL_RET_NO_SUCH_VALUE;
    }
    switch(val->type) {
    case IW_VAL_TYPE_STRING :
        return iw_val_store_set_string(store, name, value, err_buff, buff_size);
    case IW_VAL_TYPE_NUMBER : {
        long long num;
        if(!iw_util_strtoll(value, &num, 0)) {
            snprintf(err_buff, buff_size, "Invalid number");
            return IW_VAL_RET_FAILED_REGEXP;
        }
        return iw_val_store_set_number(store, name, num, err_buff, buff_size);
        }
    case IW_VAL_TYPE_ADDRESS : {
        iw_ip address;
        if(!iw_ip_str_to_addr(value, true, &address)) {
            if(err_buff != NULL) {
                snprintf(err_buff, buff_size, "Invalid IP address format");
            }
            return IW_VAL_RET_FAILED_REGEXP;
        }
        return iw_val_store_set_address(store, name, &address, err_buff, buff_size);
        }
    case IW_VAL_TYPE_NONE :
        if(err_buff != NULL) {
            snprintf(err_buff, buff_size, "No value type set");
        }
        return IW_VAL_RET_INCORRECT_TYPE;
    }
    if(err_buff != NULL) {
        snprintf(err_buff, buff_size, "Invalid value type");
    }
    return IW_VAL_RET_INCORRECT_TYPE;
}

// --------------------------------------------------------------------------

bool iw_val_store_get_persist(iw_val_store *store, const char *name) {
    iw_val_criteria *crit = (iw_val_criteria *)iw_htable_get(&store->names,
                                                             strlen(name),
                                                             name);
    return (crit != NULL && crit->persist);
}

// --------------------------------------------------------------------------

iw_val *iw_val_store_get(
    iw_val_store *store,
    const char *name)
{
    return iw_htable_get(&store->table, strlen(name), name);
}

// --------------------------------------------------------------------------

int *iw_val_store_get_number(
    iw_val_store *store,
    const char *name)
{
    iw_val *value = iw_val_store_get(store, name);
    if(value == NULL || value->type != IW_VAL_TYPE_NUMBER) {
        return NULL;
    }
    return &value->v.number;
}

// --------------------------------------------------------------------------

char *iw_val_store_get_string(
    iw_val_store *store,
    const char *name)
{
    iw_val *value = iw_val_store_get(store, name);
    if(value == NULL || value->type != IW_VAL_TYPE_STRING) {
        return NULL;
    }
    return value->v.string;
}

// --------------------------------------------------------------------------

iw_ip *iw_val_store_get_address(
    iw_val_store *store,
    const char *name)
{
    iw_val *value = iw_val_store_get(store, name);
    if(value == NULL || value->type != IW_VAL_TYPE_ADDRESS) {
        return NULL;
    }
    return &value->v.address;
}

// --------------------------------------------------------------------------
//
// Iteration through values in store
//
// --------------------------------------------------------------------------

void *iw_val_store_get_first(iw_val_store *store, unsigned long *token) {
    return (iw_val *)iw_htable_get_first(&store->table, token);
}

// --------------------------------------------------------------------------

void *iw_val_store_get_next(iw_val_store *store, unsigned long *token) {
    return (iw_val *)iw_htable_get_next(&store->table, token);
}

// --------------------------------------------------------------------------
//
// Add a pre-defined value to the value store.
//
// --------------------------------------------------------------------------

static iw_val_criteria *iw_val_store_create_criteria(
    IW_VAL_TYPE type,
    bool persist,
    const char *msg,
    IW_VAL_CRITERIA_FN fn,
    const char *regexp)
{
    iw_val_criteria *crit = calloc(1, sizeof(iw_val_criteria));
    if(crit == NULL) {
        return NULL;
    }
    crit->type    = type;
    crit->persist = persist;
    crit->fn      = fn;
    crit->regset  = false;
    if(msg != NULL) {
        crit->msg    = strdup(msg);
    }
    return crit;
}

// --------------------------------------------------------------------------

static void iw_val_store_destroy_criteria(void *node) {
    iw_val_criteria *crit = (iw_val_criteria *)node;
    if(crit->regset) {
        regfree(&crit->regexp);
    }
    free(crit->msg);
    free(crit);
}

// --------------------------------------------------------------------------

static bool iw_val_store_add_name_internal(
    iw_val_store *store,
    const char *name,
    iw_val_criteria *crit)
{
    return iw_htable_insert(&store->names, strlen(name), name, crit);
}

// --------------------------------------------------------------------------

bool iw_val_store_add_name(
    iw_val_store *store,
    const char *name,
    const char *msg,
    IW_VAL_TYPE type,
    bool persist)
{
    iw_val_criteria *crit = iw_val_store_create_criteria(type, persist,
                                                         msg, NULL, NULL);
    if(crit == NULL) {
        return false;
    }
    return iw_val_store_add_name_internal(store, name, crit);
}

// --------------------------------------------------------------------------

bool iw_val_store_add_name_callback(
    iw_val_store *store,
    const char *name,
    const char *msg,
    IW_VAL_TYPE type,
    IW_VAL_CRITERIA_FN fn,
    bool persist)
{
    iw_val_criteria *crit = iw_val_store_create_criteria(type, persist,
                                                         msg, fn, NULL);
    if(crit == NULL) {
        return false;
    }
    return iw_val_store_add_name_internal(store, name, crit);
}

// --------------------------------------------------------------------------

bool iw_val_store_add_name_regexp(
    iw_val_store *store,
    const char *name,
    const char *msg,
    IW_VAL_TYPE type,
    const char *regexp,
    bool persist)
{
    iw_val_criteria *crit = iw_val_store_create_criteria(type, persist,
                                                         msg, NULL, regexp);
    if(crit == NULL) {
        return false;
    }
    if(regcomp(&crit->regexp, regexp, REG_EXTENDED) != 0) {
        iw_val_store_destroy_criteria(crit);
        return false;
    }
    crit->regset = true;
    return iw_val_store_add_name_internal(store, name, crit);
}

// --------------------------------------------------------------------------

void iw_val_store_delete_name(iw_val_store *store, const char *name) {
    iw_htable_delete(&store->names, strlen(name), name,
                     iw_val_store_destroy_criteria);

    // Make sure to delete any value that was set for this name as well
    iw_htable_delete(&store->table, strlen(name), name, iw_val_destroy_value);
}

// --------------------------------------------------------------------------
