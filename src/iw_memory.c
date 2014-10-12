// --------------------------------------------------------------------------
///
/// @file iw_memory.c
///
/// The memory chunks are laid out as follows:
/// +----+----+----+---------------------+----+
/// | C  | L  | Pr |  Memory             | Po |
/// +----+----+----+---------------------+----+
/// Where
/// C = Cookie value
/// L = Length of memory
/// Pr = Pre-guard value
/// Mem = Allocated memory
/// Po = Post-guard value
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_memory.h"

#include "iw_htable.h"
#include "iw_list.h"
#include "iw_log.h"
#include "iw_settings.h"
#include "iw_util.h"

#include <arpa/inet.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

// --------------------------------------------------------------------------
//
// Defines
//
// --------------------------------------------------------------------------

/// The magic cookie value to use for memory chunks
#define COOKIE              0xABCDABCD

/// The size of the magic cookie field
#define COOKIE_SIZE         4

/// The size of the length field
#define LEN_SIZE            4

/// The pre-memory chunk guard
#define PRE_GUARD           0xFEFEFEFE

/// The size of the pre-memory chunk guard field
#define PRE_GUARD_SIZE      4

/// The post-memory chunk guard
#define POST_GUARD          0xFEFEFEFE

/// The size of the post-memory chunk guard field
#define POST_GUARD_SIZE     4

// --------------------------------------------------------------------------

/// The type of memory dump
typedef enum {
    IW_MEM_DUMP_ALL,
    IW_MEM_DUMP_SUMMARY,
    IW_MEM_DUMP_BRIEF
} IW_MEM_DUMP;

// --------------------------------------------------------------------------
//
// Internal variables
//
// --------------------------------------------------------------------------

/// The number of allocations made so far.
static unsigned int s_mem_allocs = 0;

/// The number of frees made so far.
static unsigned int s_mem_frees = 0;

/// The number of current outstanding allocated bytes.
static unsigned int s_mem_cur_bytes = 0;

/// The number of total allocated bytes so far (including freed memory).
static unsigned int s_mem_acc_bytes = 0;

/// Number of memory corruption detected.
static unsigned int s_mem_corrupt  = 0;

/// Number of pre-memory guard corruptions detected.
static unsigned int s_pre_corrupt  = 0;

/// Number of post-memory guard corruptions detected.
static unsigned int s_post_corrupt = 0;

/// The global mutex hash.
static iw_htable s_memory;

/// The internal rwlock for access to the mutex hash.
static pthread_rwlock_t s_memory_lock;

// --------------------------------------------------------------------------
//
// Memory tracking helper functions.
//
// --------------------------------------------------------------------------

/// @brief The memory location info structure.
typedef struct _iw_memory_loc {
    char         file[32];  ///< The file that the memory was allocated in.
    unsigned int line;      ///< The line that the memory was allocated in.
    unsigned int size;      ///< The size of the allocated memory chunk.
} iw_memory_loc;

// --------------------------------------------------------------------------

/// @brief The memory info structure.
typedef struct _iw_memory_info {
    iw_memory_loc loc;      ///< The location of the allocation.
    void         *address;  ///< The address of the allocated memory.
} iw_memory_info;

// --------------------------------------------------------------------------

/// @brief The memory report structure.
typedef struct _iw_memory_report {
    iw_list_node  node;     ///< The list node.
    iw_memory_loc loc;      ///< The location of the allocation.
    unsigned int  num;      ///< The number of allocations at this location.
} iw_memory_report;

// --------------------------------------------------------------------------

/// @brief Add a memory chunk to the tracking table.
/// @param file The file that the memory chunk was allocated at.
/// @param line The line that the memory chunk was allocated at.
/// @param address The address of the memory chunk.
/// @param size The size of the memory chunk.
static void iw_memory_add_chunk(
    const char *file,
    unsigned int line,
    void *address,
    unsigned int size)
{
    iw_memory_info *info = (iw_memory_info *)calloc(1, sizeof(iw_memory_info));
    snprintf(info->loc.file, sizeof(info->loc.file), "%s", file);
    info->loc.line = line;
    info->loc.size = size;
    info->address = address;
    iw_htable_insert(&s_memory, sizeof(&address), &address, info);
}

// --------------------------------------------------------------------------

/// @brief Delete the memory block with the given address.
/// @param address The address of the memory block to delete.
/// @return The memory info structure or NULL if no match was found.
static void iw_memory_delete_chunk(void *address) {
    iw_memory_info *info = (iw_memory_info *)iw_htable_remove(&s_memory,
                                                              sizeof(address),
                                                              &address);
    if(info != NULL) {
        free(info);
    }
}

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

void iw_memory_init() {
    if(iw_stg.iw_enable_memtrack) {
        iw_htable_init(&s_memory, 1024, false, NULL);
    }
}

// --------------------------------------------------------------------------

void iw_memory_exit() {
    if(iw_stg.iw_enable_memtrack) {
        iw_htable_destroy(&s_memory, free);
    }
}

// --------------------------------------------------------------------------

void *iw_malloc(const char *file, unsigned int line, int size) {
    if(!iw_stg.iw_enable_memtrack) {
        return malloc(size);
    }

    int new_size = size + COOKIE_SIZE + LEN_SIZE + PRE_GUARD_SIZE + POST_GUARD_SIZE;
    void *chunk = malloc(new_size);

    if(chunk == NULL) {
        return NULL;
    }

    *(unsigned int *)(chunk) = htonl(COOKIE);
    *(unsigned int *)(chunk + COOKIE_SIZE) = htonl(size);
    *(unsigned int *)(chunk + COOKIE_SIZE + LEN_SIZE) = htonl(PRE_GUARD);
    *(unsigned int *)(chunk + COOKIE_SIZE + LEN_SIZE + PRE_GUARD_SIZE + size) = htonl(POST_GUARD);

    void *ptr = chunk + COOKIE_SIZE + LEN_SIZE + PRE_GUARD_SIZE;

    iw_memory_add_chunk(file, line, ptr, size);

    s_mem_allocs++;
    s_mem_cur_bytes += size;
    s_mem_acc_bytes += size;

    return ptr;
}

// --------------------------------------------------------------------------

void *iw_calloc(const char *file, unsigned int line, int elems, int size) {
    if(!iw_stg.iw_enable_memtrack) {
        return calloc(elems, size);
    }

    void *chunk = iw_malloc(file, line, elems * size);
    if(chunk == NULL) {
        return NULL;
    }

    memset(chunk, 0, size);
    return chunk;
}

// --------------------------------------------------------------------------

void *iw_realloc(const char *file, unsigned int line, void *ptr, int size) {
    if(!iw_stg.iw_enable_memtrack) {
        return realloc(ptr, size);
    }

    void *new_chunk = NULL;

    // TODO: To simplify realloc we just allocate a new memory chunk and free the
    // old one.
    if(size != 0) {
        new_chunk = iw_malloc(file, line, size);
        if(new_chunk == NULL) {
            // Realloc does not free or move the old memory if allocation fails.
            return NULL;
        }
    }

    if(ptr != NULL) {
        if(new_chunk != NULL) {
            void *len_ptr = ptr - PRE_GUARD_SIZE - LEN_SIZE;
            unsigned int len = ntohl(*(unsigned int *)len_ptr);
            memcpy(new_chunk, ptr, len);
        }
        iw_free(ptr);
    }

    return new_chunk;
}

// --------------------------------------------------------------------------

char *iw_strdup(const char *file, unsigned int line, const char *ptr) {
    if(!iw_stg.iw_enable_memtrack) {
        return strdup(ptr);
    }

    int len = strlen(ptr) + 1;
    void *chunk = iw_malloc(file, line, len);
    if(chunk == NULL) {
        return NULL;
    }
    memcpy(chunk, ptr, len + 1);
    return chunk;
}

// --------------------------------------------------------------------------

void iw_free(void *ptr) {
    if(!iw_stg.iw_enable_memtrack) {
        free(ptr);
        return;
    }

    // Check cookie, pre- and post-guards to ensure no corruption has
    // occurred.
    void *pre_ptr    = ptr - PRE_GUARD_SIZE;
    void *len_ptr    = pre_ptr - LEN_SIZE;
    void *cookie_ptr = len_ptr - COOKIE_SIZE;
    void *post_ptr;

    unsigned int cookie = ntohl(*(unsigned int *)cookie_ptr);
    unsigned int pre    = ntohl(*(unsigned int *)pre_ptr);
    unsigned int len    = ntohl(*(unsigned int *)len_ptr);
    unsigned int post;

    if(cookie != COOKIE) {
        LOG(IW_LOG_IW, "Memory corruption detected");
        s_mem_corrupt++;

        // Cannot continue memory check. It is possible that this memory has
        // been allocated outside our normal allocation framework so just
        // free it as normal.
        free(ptr);
        return;
    }

    if(pre != PRE_GUARD) {
        LOG(IW_LOG_IW, "Pre-guard memory corruption detected");
        s_pre_corrupt++;
    }
    post_ptr = ptr + len;
    post = ntohl(*(unsigned int *)post_ptr);
    if(post != POST_GUARD) {
        LOG(IW_LOG_IW, "Post-guard memory corruption detected");
        s_post_corrupt++;
    }

    // Remove memory chunk from hash table
    iw_memory_delete_chunk(ptr);

    int org_size = len - COOKIE_SIZE - LEN_SIZE - PRE_GUARD_SIZE - POST_GUARD_SIZE;

    s_mem_frees++;
    s_mem_cur_bytes -= org_size;

    // Free the actual memory pointer.
    free(cookie_ptr);
}

// --------------------------------------------------------------------------

char *iw_memory_display_str(unsigned int len, char *buff, unsigned int bytes) {
    static char *unit[] = { "Bytes", "KBytes", "MBytes", "GBytes" };
    int cnt=0;
    for(;cnt < IW_ARR_LEN(unit);cnt++) {
        if(bytes < 1024) {
            snprintf(buff, len, "%d %s", bytes, unit[cnt]);
            return buff;
        }
        bytes = bytes / 1024;
    }
    snprintf(buff, len, "%d %s", bytes, unit[cnt-1]);
    return buff;
}

// --------------------------------------------------------------------------

static void iw_memory_dump(FILE *out, IW_MEM_DUMP dump) {
    char buff1[64];
    char buff2[64];

    if(!iw_stg.iw_enable_memtrack) {
        fprintf(out, "Memory tracking is disabled.\n");
        return;
    }

    // Lock the mutex lock
    pthread_rwlock_rdlock(&s_memory_lock);

    // Print memory statistics
    fprintf(out,
            "== Memory summary ==\n"
            "Number allocations:      %d\n"
            "Number frees:            %d\n"
            "Outstanding allocations: %d\n"
            "Outstanding allocations: %s\n"
            "Accumulated allocations: %s\n"
            "Memory corruptions:      %d\n"
            "Pre-guard corruptions:   %d\n"
            "Post-guard corruptions:  %d\n"
            "\n",
            s_mem_allocs, s_mem_frees,
            s_mem_allocs - s_mem_frees,
            iw_memory_display_str(sizeof(buff1), buff1, s_mem_cur_bytes),
            iw_memory_display_str(sizeof(buff2), buff2, s_mem_acc_bytes),
            s_mem_corrupt, s_pre_corrupt, s_post_corrupt);

    iw_htable sum;
    iw_htable_init(&sum, s_memory.size, false, NULL);

    if(dump == IW_MEM_DUMP_ALL) {
        // Print out every single memory allocation.
        unsigned long hash;
        iw_memory_info *minfo = (iw_memory_info *)iw_htable_get_first(&s_memory,
                                                                      &hash);
        fprintf(out, "== Allocated Memory ==\n");
        while(minfo != NULL) {
            fprintf(out, "Memory[%08" PRIxPTR "]: %s:%d (%s)\n",
                (uintptr_t)minfo->address,
                minfo->loc.file, minfo->loc.line,
                iw_memory_display_str(sizeof(buff1), buff1, minfo->loc.size));
            minfo = (iw_memory_info *)iw_htable_get_next(&s_memory, &hash);
        }
    } else if(dump == IW_MEM_DUMP_SUMMARY || dump == IW_MEM_DUMP_BRIEF) {
        // Summarize the memory allocations and print out the largest blocks first
        unsigned long hash;
        fprintf(out, "== Allocated Memory Summary ==\n");
        iw_memory_info *minfo = (iw_memory_info *)iw_htable_get_first(&s_memory, &hash);
        while(minfo != NULL) {
            // Use the location info as a hash key into the summary table
            iw_memory_report *report =
                    (iw_memory_report *)iw_htable_get(&sum,
                                                      sizeof(iw_memory_loc),
                                                      &minfo->loc);

            // If we have an existing entry for this location, then increase
            // the count, otherwise create an entry.
            if(report != NULL) {
                report->num++;
            } else {
                report = (iw_memory_report *)calloc(1, sizeof(iw_memory_report));
                if(report != NULL) {
                    memcpy(&report->loc, &minfo->loc, sizeof(iw_memory_loc));
                    report->num = 1;
                    iw_htable_insert(&sum, sizeof(iw_memory_loc), &report->loc, report);
                }
            }

            minfo = (iw_memory_info *)iw_htable_get_next(&s_memory, &hash);
        }
    }

    // Now we should have a table with all memory allocations summarized.
    // Go ahead and unlock the mutex lock at this point since we can
    // work on the summary table from this point on.
    pthread_rwlock_unlock(&s_memory_lock);

    if(dump == IW_MEM_DUMP_SUMMARY || dump == IW_MEM_DUMP_BRIEF) {
        // Create a sorted linked list by taking one element at the time and
        // insert it in the correct position in the list.
        unsigned long hash;
        iw_list rlist = IW_LIST_INIT;
        iw_memory_report *report = (iw_memory_report *)iw_htable_get_first(&sum, &hash);
        while(report != NULL) {
            if(rlist.num_elems == 0) {
                iw_list_add(&rlist, (iw_list_node *)report);
            } else {
                iw_list_node *node = rlist.head;
                iw_memory_report *rnode = (iw_memory_report *)node;
                while(node->next != NULL &&
                      (rnode->num > report->num ||
                       (rnode->num == report->num &&
                        rnode->loc.size > report->loc.size)))
                {
                    node = node->next;
                }

                // Insert the entry into the list at this point
                iw_list_insert_before(&rlist, node, (iw_list_node *)report);
            }

            // Get the next entry
            report = (iw_memory_report *)iw_htable_get_next(&sum, &hash);
        }

        // Print out the summarized report
        iw_list_node *node = rlist.head;
        int cnt;
        for(cnt=0;node != NULL && (dump != IW_MEM_DUMP_BRIEF || cnt < 20);cnt++) {
            iw_memory_report *report = (iw_memory_report *)node;
            fprintf(out, "Memory Allocation: %s:%d (%d * %s => Total %s)\n",
                report->loc.file,
                report->loc.line,
                report->num,
                iw_memory_display_str(sizeof(buff1), buff1, report->loc.size),
                iw_memory_display_str(sizeof(buff2), buff2, report->num * report->loc.size));
            node = node->next;
        }
    }

    // Finally delete all allocated structures.
    iw_htable_destroy(&sum, free);
}

// --------------------------------------------------------------------------

void iw_memory_show(FILE *out) {
    iw_memory_dump(out, IW_MEM_DUMP_ALL);
}

// --------------------------------------------------------------------------

void iw_memory_summary(FILE *out) {
    iw_memory_dump(out, IW_MEM_DUMP_SUMMARY);
}

// --------------------------------------------------------------------------

void iw_memory_brief(FILE *out) {
    iw_memory_dump(out, IW_MEM_DUMP_BRIEF);
}

// --------------------------------------------------------------------------
