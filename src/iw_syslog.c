// --------------------------------------------------------------------------
///
/// @file iw_syslog.c
///
/// The syslog module saves syslog entries in a memory buffer. To save space
/// we use a ring buffer rather than a two-dimensional array.
///
/// Each entry is saved with a starting length value, a timestamp, and then
/// the string itself. The length is the total length of the entry, including
/// the length value itself, the timestamp, the string as well as the
/// terminating NUL-byte.
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_syslog.h"

#include "iw_log.h"
#include "iw_memory.h"

#include <arpa/inet.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/time.h>
#include <time.h>

// --------------------------------------------------------------------------

/// The default buffer size.
#define DEF_BUFF_SIZE   10000

// --------------------------------------------------------------------------
//
// Data structure variable.
//
// --------------------------------------------------------------------------

/// The pointer to the syslog buffer.
static char *s_msg_buff = NULL;

/// The size of the allocated buffer.
static int s_buff_size = 0;

/// The end of the allocated buffer.
static char *s_buff_end = NULL;

/// The current read position. This is where we will start reading entries.
char *s_read_pos  = NULL;

/// The current write position. This is where we will start writing entries.
static char *s_write_pos    = NULL;

/// This variable keeps track of whether any messages have been written or
/// not. This is important since for the first message, s_read_pos and
/// s_write_pos will be the same and in this case it signifies an empty buffer.
/// In all other cases (since we never remove messages from the buffer)
/// having s_read_pos and s_write_pos be the same means that the queue is full.
static bool s_first_msg = true;

// --------------------------------------------------------------------------
//
// Internal helpers
//
// --------------------------------------------------------------------------

/// @brief Write an entry to the message buffer.
/// @param buff_start The start of the buffer to write to.
/// @param buff_len The length of the available buffer.
/// @param tv The time-stamp to write to the entry.
/// @param msg The message to write.
/// @param ap The variable argument list for the message.
/// @return The length of the message to write. This number is positive if
///         the message could be written, and negative if it did not fit and
///         the write needs to be retried.
static int iw_syslog_write_entry(
    char *buff_start,
    int buff_len,
    struct timeval *tv,
    const char *msg,
    va_list ap)
{
    int length;
    unsigned int hdr_len = sizeof(length) + sizeof(*tv);
    int buff_avail = buff_len - hdr_len;

    if(buff_avail < 0) {
        // If available buffer is less than zero, then set it to zero.
        // This will allow us to call vsnprintf and find out how much
        // space the message actually will take.
        buff_avail = 0;
    }

    length = vsnprintf(buff_start + hdr_len, buff_avail, msg, ap);
    if(length > buff_avail) {
        length += 1 + sizeof(length) + sizeof(*tv);
        return -length;
    } else {
        // Write length field and timestamp
        s_first_msg = false;

        // First update the length record to include the NUL byte, the size
        // of the length field itself, and the size of the timestamp.
        length += 1 + sizeof(length) + sizeof(*tv);

        *(int *)buff_start = htonl(length);
        buff_start += sizeof(length);
        memcpy(buff_start, tv, sizeof(*tv));
        return length;
    }
}
// --------------------------------------------------------------------------

static void iw_syslog_add(const char *msg, va_list ap) {
    // We add entries to the s_write_pos pointer. First we need to find out
    // if the end is after the start or vice versa.
    int length;
    int remainder;
    struct timeval cur_time;

    gettimeofday(&cur_time, NULL);

    // Check if s_write_pos is after s_read_pos.
    if(s_write_pos > s_read_pos || s_first_msg) {
        // Our available space is from cur_end to the end of the buffer.
        remainder = s_buff_end - s_write_pos;
    } else {
        // Our available space is from the beginning of the buffer to
        // the cur_start pointer.
        remainder = s_read_pos - s_write_pos;
    }

    // Try to write the message in the available space.
    length = iw_syslog_write_entry(s_write_pos, remainder, &cur_time, msg, ap);

    if(length > 0) {
        // The message fit in the buffer, update the pointers
        s_write_pos += length;
    } else {
        // The message didn't fit in the buffer.
        length = -length;

        // First check to make sure the message size isn't bigger than the
        // whole buffer. If it is, we can't fit it and might as well return.
        if(length >= s_buff_size) {
            LOG(IW_LOG_IW, "Message too large to fit in buffer.");
            return;
        }
        if(s_write_pos + length > s_buff_end) {
            // There wasn't enough space in the end of the buffer,
            // move the write end to the beginning of the buffer.
            // Before moving the position we write zero into the next
            // four bytes to signify the end for the reader. We don't
            // need to write more than four bytes since the length field
            // is only four bytes, but we also cannot write more than
            // remains in the buffer.
            int end_marker = s_buff_end - s_write_pos;
            if(end_marker > 4) {
                end_marker = 4;
            }
            memset(s_write_pos, 0, end_marker);
            s_write_pos = s_msg_buff;
            s_read_pos = s_msg_buff;
        }

        // Keep clearing old messages as long as we don't have enough space
        // to fit this message.
        while(s_read_pos - s_write_pos < length) {
            if(s_read_pos + 4 > s_buff_end) {
                // There's less than 4 bytes left in the buffer space.
                // This means that we should just go to the start.
                s_read_pos = s_msg_buff;
                break;
            }
            int msg_len = ntohl(*(unsigned int *)s_read_pos);
            if(msg_len == 0) {
                // We've reached the end, there are no more old messages.
                s_read_pos = s_msg_buff;
                break;
            }
            if(s_read_pos + msg_len <= s_buff_end) {
                s_read_pos += msg_len;
            } else {
                break;
            }
        }

        // Now we should have enough space to write the message.
        if(s_read_pos == s_msg_buff) {
            // We've moved s_read_pos back to the start
            remainder = s_buff_end - s_write_pos;
        } else {
            remainder = s_read_pos - s_write_pos;
        }
        length = iw_syslog_write_entry(s_write_pos, remainder, &cur_time, msg, ap);
        if(length > 0) {
            s_write_pos += length;
        }
    }
}

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

void iw_syslog_reinit(int buff_size) {
    if(s_msg_buff != NULL) {
        iw_syslog_exit();
    }
    if(buff_size == 0) {
        buff_size = DEF_BUFF_SIZE;
    }
    s_buff_size  = buff_size;
    s_msg_buff   = IW_CALLOC(1, s_buff_size);
    s_buff_end   = s_msg_buff + s_buff_size;
    s_read_pos   = s_msg_buff;
    s_write_pos  = s_msg_buff;
    s_first_msg  = true;
}

// --------------------------------------------------------------------------

void iw_syslog_exit() {
    IW_FREE(s_msg_buff);
}

// --------------------------------------------------------------------------

void iw_syslog_display(FILE *out) {
    // Start at the current pointer and display all messages in the buffer
    bool buff_empty = true;
    char *ptr = s_read_pos;
    char *end;
    int cnt=0, max=1;

    if(ptr > s_msg_buff) {
        // If s_read_pos is the same as s_msg_buff, we only have to
        // go through the buffer once from the start to the end. If
        // s_read_pos is in the middle of the buffer, we have to first go
        // from s_read_pos to the end of the buffer, then from the start
        // of the buffer to s_read_pos.
        max = 2;
    }

    for(cnt=0;cnt < max;cnt++) {
        int length;
        struct timeval tv;
        unsigned int hdr_len = sizeof(length) + sizeof(tv);

        // If the end is after the current position, then use the end,
        // otherwise use the end of the buffer as the end.
        if(s_write_pos > ptr) {
            end = s_write_pos;
        } else {
            end = s_buff_end;
        }

        // There must be enough space for the length and timestamp fields at
        // least before we consider the entry.
        while(ptr < end - hdr_len) {
            time_t nowtime;
            struct tm *nowtm;
            char buff[64];
            int offset;
            int length = ntohl(*(int *)ptr);
            char *start = ptr;
            if(length == 0) {
                // No more records until the end of the buffer, break out
                break;
            }

            ptr += sizeof(length);
            memcpy(&tv, ptr, sizeof(tv));
            ptr += sizeof(tv);
            nowtime = tv.tv_sec;
            nowtm = localtime(&nowtime);
            offset = strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", nowtm);
            snprintf(buff + offset, sizeof(buff) - offset, "%06ld", tv.tv_usec);

            fprintf(out, "LOG: [%s] %s\n", buff, ptr);
            buff_empty = false;
            ptr = start + length;
        }
        ptr = s_msg_buff;
    }
    if(buff_empty) {
        fprintf(out, "<no messages>\n");
    }
}

// --------------------------------------------------------------------------

void iw_syslog_clear() {
    s_read_pos   = s_msg_buff;
    s_write_pos  = s_msg_buff;
    s_first_msg  = true;
    memset(s_msg_buff, 0, s_buff_size);
}

// --------------------------------------------------------------------------

void iw_syslog(int priority, const char *fmt, ...) {
    {
        va_list ap;
        va_start(ap, fmt);
        vsyslog(priority, fmt, ap);
        va_end(ap);
    }

    {
        va_list ap;
        va_start(ap, fmt);
        iw_syslog_add(fmt, ap);
        va_end(ap);
    }
}

// --------------------------------------------------------------------------
