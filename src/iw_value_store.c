// --------------------------------------------------------------------------
///
/// @file iw_value_store.c
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_value_store.h"

#include "iw_memory.h"

#include <stdlib.h>
#include <string.h>

// --------------------------------------------------------------------------

iw_value *iw_val_create_number(const char *name, int num) {
    iw_value *value = IW_CALLOC(1, sizeof(iw_value));
    if(value == NULL) {
        return NULL;
    }
    value->name = IW_STRDUP(name);
    value->type = IW_VALUE_TYPE_NUMBER;
    value->v.number = num;
    if(value->name == NULL) {
        iw_val_destroy(value);
    }
    return value;
}

// --------------------------------------------------------------------------

iw_value *iw_val_create_string(const char *name, char *str) {
    iw_value *value = IW_CALLOC(1, sizeof(iw_value));
    if(value == NULL) {
        return NULL;
    }
    value->name = IW_STRDUP(name);
    value->type = IW_VALUE_TYPE_STRING;
    value->v.string = IW_STRDUP(str);
    if(value->name == NULL || value->v.string == NULL) {
        iw_val_destroy(value);
    }
    return value;
}

// --------------------------------------------------------------------------

iw_value *iw_val_create_address(const char *name, iw_ip *address) {
    iw_value *value = IW_CALLOC(1, sizeof(iw_value));
    if(value == NULL) {
        return NULL;
    }
    value->name = IW_STRDUP(name);
    value->type = IW_VALUE_TYPE_ADDRESS;
    memcpy(&value->v.address, address, sizeof(iw_ip));
    if(value->name == NULL || value->v.string == NULL) {
        iw_val_destroy(value);
    }
    return value;
}

// --------------------------------------------------------------------------

void iw_val_destroy(iw_value *value) {
    IW_FREE(value->name);
    if(value->type == IW_VALUE_TYPE_STRING) {
        IW_FREE(value->v.string);
    }
    IW_FREE(value);
}

// --------------------------------------------------------------------------
//
// Value store creation/destruction
//
// --------------------------------------------------------------------------

bool iw_val_store_init(iw_store *store) {
    iw_htable_init(&store->table, 1024, false, NULL);
}

// --------------------------------------------------------------------------

void iw_val_store_destroy(iw_store *store) {
    iw_htable_destroy(&store->table, iw_val_destroy);
}

// --------------------------------------------------------------------------
//
// Value access functions
//
// --------------------------------------------------------------------------

bool iw_val_store_set(
    iw_store *store,
    const char *name,
    iw_value *value)
{
    return iw_htable_insert(&store->table, strlen(name), name, value);
}

// --------------------------------------------------------------------------

bool iw_val_store_set_number(
    iw_store *store,
    const char *name,
    int num)
{
    iw_value *value = iw_val_create_number(name, num);
    if(value == NULL) {
        return false;
    }
    if(!iw_val_store_set(store, name, value)) {
        iw_val_destroy(value);
        return false;
    }
    return true;
}

// --------------------------------------------------------------------------

bool iw_val_store_set_string(
    iw_store *store,
    const char *name,
    const char *str)
{
    iw_value *value = iw_val_create_string(name, str);
    if(value == NULL) {
        return false;
    }
    if(!iw_val_store_set(store, name, value)) {
        iw_val_destroy(value);
        return false;
    }
    return true;
}

// --------------------------------------------------------------------------

bool iw_val_store_set_address(
    iw_store *store,
    const char *name,
    const iw_ip *address)
{
    iw_value *value = iw_val_create_address(name, address);
    if(value == NULL) {
        return false;
    }
    if(!iw_val_store_set(store, name, value)) {
        iw_val_destroy(value);
        return false;
    }
    return true;
}

// --------------------------------------------------------------------------

iw_value *iw_val_store_get(
    iw_store *store,
    const char *name)
{
    return iw_htable_get(&store->table, strlen(name), name);
}

// --------------------------------------------------------------------------

int *iw_val_store_get_number(
    iw_store *store,
    const char *name)
{
}

// --------------------------------------------------------------------------

char *iw_val_store_get_string(
    iw_store *store,
    const char *name)
{
}

// --------------------------------------------------------------------------

iw_ip *iw_val_store_get_address(
    iw_store *store,
    const char *name)
{
}

// --------------------------------------------------------------------------
//
// Iteration through values in store
//
// --------------------------------------------------------------------------

void *iw_store_val_get_first(iw_store *store, unsigned long token) {
}

// --------------------------------------------------------------------------

void *iw_store_val_get_next(iw_store *store, unsigned long token) {
}

// --------------------------------------------------------------------------
