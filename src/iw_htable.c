// --------------------------------------------------------------------------
///
/// @file iw_htable.c
///
/// Copyright (c) 2014-2015 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_htable.h"

#include "iw_log.h"
#include "iw_memory.h"
#include "iw_memory_int.h"

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

bool iw_htable_init(
    iw_htable *table,
    unsigned int table_size,
    bool iw_mem_alloc,
    IW_HASH_FN fn)
{
    INT_CALLOC(iw_mem_alloc, table->table, table_size, iw_hash_node *);
    if(table->table == NULL) {
        LOG(IW_LOG_IW, "Failed to allocate memory for table size=%d", table_size);
        return false;
    }
    table->iw_mem_alloc = iw_mem_alloc;
    table->fn           = fn == NULL ? iw_hash_data : fn;
    table->size         = table_size;
    table->num_elems    = 0;
    table->collisions   = 0;
    return true;
}

// --------------------------------------------------------------------------

bool iw_htable_replace(
    iw_htable *table,
    unsigned int key_len,
    const void *key,
    void *data,
    IW_HASH_DEL_FN fn)
{
    unsigned long hash = iw_hash_data(key_len, key);
    unsigned int index = hash % table->size;
    iw_hash_node *node = table->table[index];

    if(fn != NULL) {
        iw_htable_delete(table, key_len, key, fn);
    } else {
        // See if the bucket already contains the value
        if(node != NULL) {
            while(node != NULL) {
                if(node->hash == hash) {
                    LOG(IW_LOG_IW, "Hash table already contains the value");
                    return false;
                }
                node = node->next;
            }
            table->collisions++;
        }
    }

    // Bucket did not contain the value, let's add it at the start of the list
    iw_hash_node *new_node;
    INT_CALLOC(table->iw_mem_alloc, new_node, 1, iw_hash_node);
    if(new_node == NULL) {
        LOG(IW_LOG_IW, "Failed to allocate memory for node");
        return false;
    }
    new_node->next = table->table[index];
    table->table[index] = new_node;
    new_node->hash = hash;
    new_node->data = data;
    table->num_elems++;

    return true;
}

// --------------------------------------------------------------------------

bool iw_htable_insert(
    iw_htable *table,
    unsigned int key_len,
    const void *key,
    void *data)
{
    return iw_htable_replace(table, key_len, key, data, NULL);
}

// --------------------------------------------------------------------------

void *iw_htable_get(
    iw_htable *table,
    unsigned int key_len,
    const void *key)
{
    unsigned long hash = table->fn(key_len, key);
    unsigned int index = hash % table->size;
    iw_hash_node *node = table->table[index];

    while(node != NULL) {
        if(node->hash == hash) {
            return node->data;
        }
        node = node->next;
    }
    return NULL;
}

// --------------------------------------------------------------------------

void *iw_htable_remove(
    iw_htable *table,
    unsigned int key_len,
    const void *key)
{
    unsigned long hash = table->fn(key_len, key);
    unsigned int index = hash % table->size;
    iw_hash_node *node = table->table[index];
    iw_hash_node *prev;

    if(node == NULL) {
        return NULL;
    }

    if(node->hash == hash) {
        table->table[index] = node->next;
        void *data = node->data;
        table->num_elems--;
        if(node->next != NULL) {
            table->collisions--;
        }
        INT_FREE(table->iw_mem_alloc, node);
        return data;
    }

    prev = node;
    node = node->next;
    while(node != NULL) {
        if(node->hash == hash) {
            prev->next = node->next;
            void *data = node->data;
            INT_FREE(table->iw_mem_alloc, node);
            table->num_elems--;
            table->collisions--;
            return data;
        }
        prev = node;
        node = node->next;
    }
    return NULL;
}

// --------------------------------------------------------------------------

bool iw_htable_delete(
    iw_htable *tb,
    unsigned int key_len,
    const void *key,
    IW_HASH_DEL_FN fn)
{
    void *data = iw_htable_remove(tb, key_len, key);
    if(data != NULL) {
        if(fn != NULL) {
            fn(data);
        }
        return true;
    }
    return false;
}

// --------------------------------------------------------------------------

void iw_htable_destroy(iw_htable *table, IW_HASH_DEL_FN fn) {
    int index, size = table->size;
    for(index=0;index < size;index++) {
        iw_hash_node *node = table->table[index];
        while(node != NULL) {
            iw_hash_node *tmp = node->next;
            if(fn != NULL) {
                fn(node->data);
            }
            INT_FREE(table->iw_mem_alloc, node);
            node = tmp;
        }
    }
    INT_FREE(table->iw_mem_alloc, table->table);
    table->table = NULL;
    table->num_elems = 0;
}

// --------------------------------------------------------------------------

static iw_hash_node *iw_htable_find_hash(iw_htable *table, unsigned long *hash)
{
    int index, size = table->size;
    for(index=0;index < size;index++) {
        iw_hash_node *node = table->table[index];
        while(node != NULL) {
            if(node->hash == *hash) {
                // Found the element with the given hash
                return node;
            }
            node = node->next;
        }
    }
    // We did not find anything, this means we've gone through the whole table.
    return NULL;
}

// --------------------------------------------------------------------------

static iw_hash_node *iw_htable_get_first_hash(iw_htable *table, unsigned long *hash) {
    int index, size = table->size;
    for(index=0;index < size;index++) {
        iw_hash_node *node = table->table[index];
        if(node != NULL) {
            *hash = node->hash;
            return node;
        }
    }
    return NULL;
}

// --------------------------------------------------------------------------

static void *iw_htable_get_next_hash(iw_htable *table, unsigned long *hash) {
    bool found_last = false;
    int index, size = table->size;
    for(index=0;index < size;index++) {
        iw_hash_node *node = table->table[index];
        while(node != NULL) {
            if(found_last) {
                // We already found the last hash element, now return this one.
                *hash = node->hash;
                return node;
            }
            if(node->hash == *hash) {
                // Found the last element that was returned, now return the
                // next element
                found_last = true;
            }
            node = node->next;
        }
    }
    // We did not find anything, this means we've gone through the whole table.
    return NULL;
}

// --------------------------------------------------------------------------

void *iw_htable_get_first(iw_htable *table, unsigned long *hash) {
    iw_hash_node *node = iw_htable_get_first_hash(table, hash);

    return node != NULL ? node->data : NULL;
}

// --------------------------------------------------------------------------

void *iw_htable_get_next(iw_htable *table, unsigned long *hash) {
    iw_hash_node *node = iw_htable_get_next_hash(table, hash);

    return node != NULL ? node->data : NULL;
}

// --------------------------------------------------------------------------

void *iw_htable_get_first_ordered(
    iw_htable *table,
    int (*compare)(const void *, const void *),
    unsigned long *hash)
{
    // Find the lowest node using the given comparison function
    unsigned long hash_cur = 0;
    iw_hash_node *cur = iw_htable_get_first_hash(table, &hash_cur);
    *hash = 0;
    while(cur != NULL) {
        iw_hash_node *next = iw_htable_get_next_hash(table, &hash_cur);
        if(next == NULL) {
            break;
        }
        if((*compare)(cur->data, next->data) > 0) {
            cur = next;
        }
    }

    // Now we've found the first element given the order to use
    if(cur != NULL) {
        *hash = cur->hash;
        return cur->data;
    }
    return NULL;
}

// --------------------------------------------------------------------------

void *iw_htable_get_next_ordered(
    iw_htable *table,
    int (*compare)(const void *, const void *),
    unsigned long *hash)
{
    unsigned long hash_cur;
    iw_hash_node *prev = iw_htable_find_hash(table, hash);
    iw_hash_node *cur  = NULL;
    iw_hash_node *next = iw_htable_get_first_hash(table, &hash_cur);
    if(prev == NULL || next == NULL) {
        // The previous returned node is NULL or the whole table is empty,
        // either way, we can't continue
        return NULL;
    }

    // We're searching for the lowest value that is still higher than the
    // previous value. We need to check if the next value is lower than
    // current, but higher than previous. If so, this is the new current value.
    while(next != NULL) {
        if((*compare)(prev->data, next->data) < 0) {
            // 'next' node is 'higher' than 'prev' node, also make sure it
            // is lower than current node if any
            if(cur == NULL || (*compare)(cur->data, next->data) > 0) {
                // Either there were no old 'cur' value, or there was an old
                // 'cur' value, but the 'next' value is lower, so now this is
                // the new 'cur' value.
                *hash = hash_cur;
                cur = next;
            }
        }
        next = iw_htable_get_next_hash(table, &hash_cur);
    }

    // Now we've found the first element given the order to use
    return cur != NULL ? cur->data : NULL;
}

// --------------------------------------------------------------------------

void iw_htable_report(iw_htable *table, FILE *out) {
    int index, size = table->size;
    int tot_elems = 0;
    int tot_collisions = 0;
    fprintf(out, " v-- Hash Table 0x%p --v\n", table);
    for(index=0;index < size;index++) {
        fprintf(out, "  Bucket %d:\n", index);
        iw_hash_node *node = table->table[index];
        int num_elems = 0;
        while(node != NULL) {
            fprintf(out, "   Key[%08lX] --> %p\n", node->hash, node->data);
            num_elems++;
            node = node->next;
        }
        if(num_elems > 1) {
            tot_collisions += num_elems - 1;
        }
        tot_elems += num_elems;
    }
    fprintf(out, "  -- Summary --\n");
    fprintf(out, "   Number of Elements:   %d\n", tot_elems);
    fprintf(out, "   Number of Collisions: %d\n", table->collisions);
    fprintf(out, " ^-- Hash Table 0x%p --^\n", table);
}

// --------------------------------------------------------------------------
