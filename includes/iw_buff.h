// --------------------------------------------------------------------------
///
/// @file iw_buff.h
///
/// The buffer facility allows you to write data into a buffer than can
/// automatically resize if needed. When reading data, all unread data is
/// moved to the beginning of the buffer. This is done to guarantee an
/// unbroken memory area.
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _IW_BUFF_H_
#define _IW_BUFF_H_
#ifdef _cplusplus
extern "C" {
#endif

#include <stdbool.h>

// --------------------------------------------------------------------------
//
// Datastructures
//
// --------------------------------------------------------------------------

/// @brief The buffer structure.
typedef struct _iw_buff {
    char *buff;                 ///< A pointer to the beginning of the buffer.
    unsigned int end;           ///< The end of the actual data.
    unsigned int size;          ///< The size of the complete buffer memory.
    unsigned int max_size;      ///< The maximum size of the buffer.
} iw_buff;

//---------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// @brief Create a buffer object.
/// @param buff A pointer to a buffer structure.
/// @param initial_size The intial size of the buffer.
/// @param max_size The maximum size of the buffer.
/// @return True if the buffer was successfully created.
extern bool iw_buff_create(
    iw_buff *buff,
    unsigned int initial_size,
    unsigned int max_size);

// --------------------------------------------------------------------------

/// @brief Destroy a buffer object.
/// Frees the memory used by the buffer object.
/// @param buff The buffer object to destroy.
extern void iw_buff_destroy(iw_buff *buff);

// --------------------------------------------------------------------------

/// @brief Add data to the buffer.
/// @param buff A pointer to the buffer structure.
/// @param data The data to add.
/// @param size The size of the data to add.
/// @return True if the data was successfully added.
extern bool iw_buff_add_data(iw_buff *buff, char *data, unsigned int size);

// --------------------------------------------------------------------------

/// @brief Reserve data space in the buffer.
/// This should be used to allocate space for a system call (such as recv()) to
/// write data into the buffer. Once the data has been written by the system
/// call, you should call iw_buff_commit_data() to commit the amount of data
/// that was actually written.
/// @param buff A pointer to the buffer structure.
/// @param data [out] A pointer to the data space reserved.
/// @param size The size of the buffer being reserved.
/// @return True if the data was successfully reserved.
extern bool iw_buff_reserve_data(iw_buff *buff, char **data, unsigned int size);

// --------------------------------------------------------------------------

/// @param buff A pointer to the buffer structure.
/// @param size The size of the data being committed as written.
extern void iw_buff_commit_data(iw_buff *buff, unsigned int size);

// --------------------------------------------------------------------------

/// @brief Remove data from the buffer.
/// The data in the beginning of the buffer is removed and all non-removed
/// data is moved to the beginning of the buffer.
/// @param buff A pointer to the buffer structure.
/// @param size The size of the data to remove.
extern void iw_buff_remove_data(iw_buff *buff, unsigned int size);

// --------------------------------------------------------------------------

/// @brief Return the remainding amount of space in the buffer.
/// Note that this is the remainding amount of space in the current buffer.
/// If more than the remainder is requested, the buffer may be reallocated
/// to provide that space.
/// @param buff A pointer to the buffer structure.
/// @return The number of bytes that remains in the current buffer.
extern int iw_buff_remainder(iw_buff *buff);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_BUFF_H_

// --------------------------------------------------------------------------
