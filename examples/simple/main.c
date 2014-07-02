// --------------------------------------------------------------------------
///
/// @file main.c
///
/// A simple example program using the InstaWorks framework. This program
/// will create a server socket and will listen for incoming TCP connections.
/// It will read data on each TCP connection and write the received data on
/// the other TCP connections.
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include <iw_cmds.h>
#include <iw_ip.h>
#include <iw_log.h>
#include <iw_list.h>
#include <iw_main.h>
#include <iw_memory.h>
#include <iw_mutex.h>
#include <iw_settings.h>
#include <iw_syslog.h>

#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

// --------------------------------------------------------------------------
//
// Helper data structures
//
// --------------------------------------------------------------------------

#define TIMEOUT_SEC     1
#define TIMEOUT_USEC    0

#define SIMPLE_LOG      8

/// @brief The TCP connection object.
/// Each client connection that is received will be represented by a TCP
/// connection object. This object will keep track of the client socket file
/// descriptor, the number of received bytes, the number of sent bytes, and
/// the address of the client.
typedef struct _tcp_conn {
    iw_list_node node;  ///< The base list object.
    int          fd;    ///< The TCP socket file descriptor.
    unsigned int rx;    ///< The number of bytes received on the connection.
    unsigned int tx;    ///< The number of bytes sent on the connection.
    struct sockaddr_storage address; ///< The peer address.
} tcp_conn;

// --------------------------------------------------------------------------
//
// Local data
//
// --------------------------------------------------------------------------

static int      s_sock = -1;            ///< The server socket.
static iw_list  s_list = IW_LIST_INIT;  ///< The TCP socket list.
static IW_MUTEX s_mutex;                ///< The mutex to protect the sockets.
static unsigned short s_port = 1234;    ///< The port number to use.

// --------------------------------------------------------------------------
//
// Helper functions
//
// --------------------------------------------------------------------------

/// @brief Create a TCP connection object.
/// @param fd The file descriptor for the TCP connection.
/// @return The TCP connection object.
static tcp_conn *create_tcp_conn(int fd, struct sockaddr_storage *address) {
    tcp_conn *conn = (tcp_conn *)IW_CALLOC(1, sizeof(tcp_conn));
    conn->fd = fd;
    conn->address = *address;
    return conn;
}

// --------------------------------------------------------------------------

/// @brief Delete a TCP connection object.
/// @param node The TCP connection object to delete.
static void delete_tcp_conn(iw_list_node *node) {
    tcp_conn *conn = (tcp_conn *)node;
    IW_FREE(conn);
}

// --------------------------------------------------------------------------

/// @brief List all peers currently connected to the server.
/// @param out The output file stream to display the connections on.
/// @param cmd The command request being processed.
/// @param info The parse info to use when parsing the request.
static bool list_connections(FILE *out, const char *cmd, iw_cmd_parse_info *info) {
    int cnt = 0;
    iw_mutex_lock(s_mutex);
    tcp_conn *conn = (tcp_conn *)s_list.head;
    if(conn == NULL) {
        fprintf(out, "<no connections>\n");
    } else {
        for(;conn != NULL;cnt++) {
            char ipbuff[IW_IP_BUFF_LEN];
            unsigned short port = 0;
            iw_ip_addr_to_str(&conn->address, ipbuff, sizeof(ipbuff));
            fprintf(out, "Connection %-3d: FD=%d Client=%s/%d, RX=%d bytes, TX=%d bytes\n",
                    cnt, conn->fd, ipbuff,
                    iw_ip_get_port(&conn->address),
                    conn->rx, conn->tx);
            conn = (tcp_conn *)conn->node.next;
        }
    }
    iw_mutex_unlock(s_mutex);
    return true;
}

// --------------------------------------------------------------------------

/// @brief Serve the TCP server socket and connections.
/// This function opens a server socket and starts listening on all open
/// socket file descriptors. If a client connection is received, it is
/// accepted and saved in the list of TCP connections. If data is received
/// on a TCP connection, it is read and the data is written on all other
/// TCP connections.
/// @return False if the server exited prematurely.
static bool serve_data() {
    tcp_conn *conn;
    int retval = 0;
    int maxfd = s_sock;
    fd_set readfds;
    struct timeval timeout = { TIMEOUT_SEC, TIMEOUT_USEC };

    // Initial setup
    FD_ZERO(&readfds);
    FD_SET(s_sock, &readfds);
    while((retval = select(maxfd + 1,
                           &readfds, NULL, NULL, &timeout) >= 0)) 
    {
        // Check the server socket for events
        if(FD_ISSET(s_sock, &readfds)) {
            // We received a TCP connection attempt, accept the socket
            struct sockaddr_storage address;
            socklen_t len = sizeof(address);
            int sock = accept(s_sock, (struct sockaddr *)&address, &len);
            if(sock == -1) {
                IW_SYSLOG(LOG_INFO, SIMPLE_LOG, "Failed to accept the connection (%d:%s)",
                    errno, strerror(errno));
            } else {
                char ipbuff[INET6_ADDRSTRLEN];
                iw_ip_addr_to_str(&address, ipbuff, sizeof(ipbuff));
                IW_SYSLOG(LOG_INFO, SIMPLE_LOG, "Accepted socket FD=%d from client %s:%d",
                        sock, ipbuff, iw_ip_get_port(&address));
                conn = create_tcp_conn(sock, &address);
                iw_list_add(&s_list, (iw_list_node *)conn);
            }
        }

        // Check the TCP connection sockets for events.
        iw_mutex_lock(s_mutex);
        conn = (tcp_conn *)s_list.head;
        while(conn != NULL) {
            if(FD_ISSET(conn->fd, &readfds)) {
                // We received data on the TCP connection
                char buff[1024];
                int bytes = recv(conn->fd, buff, sizeof(buff), 0);
                if(bytes > 0) {
                    conn->rx += bytes;

                    // Write the data on all TCP connections except the one
                    // that it was received on.
                    buff[bytes] = '\0';
                    LOG(SIMPLE_LOG, "Received data \"%s\"", buff);
                    tcp_conn *conn2 = (tcp_conn *)s_list.head;
                    while(conn2 != NULL) {
                        if(conn2->fd != conn->fd) {
                            send(conn2->fd, ">", 1, 0);
                            send(conn2->fd, buff, bytes, 0);
                            conn2->tx += bytes;
                        }
                        conn2 = (tcp_conn *)conn2->node.next;
                    }
                } else if(bytes == 0) {
                    // Received a socket disconnect, remove socket from list.
                    char ipbuff[IW_IP_BUFF_LEN];
                    iw_ip_addr_to_str(&conn->address, ipbuff, sizeof(ipbuff));
                    IW_SYSLOG(LOG_INFO, SIMPLE_LOG, "Socket FD=%d, client %s:%d, is closed",
                            conn->fd, ipbuff, iw_ip_get_port(&conn->address));
                    close(conn->fd);
                    conn = (tcp_conn *)iw_list_delete(&s_list, 
                                                      (iw_list_node *)conn,
                                                      delete_tcp_conn);
                    continue;
                }
            }
            conn = (tcp_conn *)conn->node.next;
        }
        iw_mutex_unlock(s_mutex);

        // Set up socket sets for next select.
        maxfd = s_sock;
        FD_ZERO(&readfds);
        FD_SET(s_sock, &readfds);
        timeout.tv_sec  = TIMEOUT_SEC;
        timeout.tv_usec = TIMEOUT_USEC;

        iw_mutex_lock(s_mutex);
        conn = (tcp_conn *)s_list.head;
        while(conn != NULL) {
            FD_SET(conn->fd, &readfds);
            maxfd = conn->fd > maxfd ? conn->fd : maxfd;
            conn = (tcp_conn *)conn->node.next;
        }
        iw_mutex_unlock(s_mutex);
    }
    if(retval == -1) {
        LOG(SIMPLE_LOG, "Select failed, exiting (%d:%s)", errno, strerror(errno));
        return false;
    }
    return true;
}

// --------------------------------------------------------------------------

static void print_help(const char *error) {
    printf("simple - A simple server\n"
           "\n");
    if(error != NULL) {
        printf("Error: %s\n\n", error);
    }
    printf("A simple TCP server that listens for connections on the specified port\n"
           "number. Any data received on a connection is forwarded to all other\n"
           "connections.\n"
           "\n"
           "Usage: simple [options] [port number]\n"
           "\n"
           "[port number]\n"
           "    The port number to use for the server (default is 1234)\n"
           "\n");
    iw_cmdline_print_help();
    printf("\n"
           "If the program is started without any command line options it will\n"
           "run in client mode and send control commands to a running server.\n"
           "Run 'simple help' once the server is running for more help on this.\n"
           "\n");
}

// --------------------------------------------------------------------------

/// @brief The main callback function.
/// @param argc The number of arguments.
/// @param argv The arguments.
/// @return False if the program exited due to an error.
bool main_callback(int argc, char **argv) {
    // At this point it is safe to add log levels and control commands.

    // Start with processing any remaining non-option arguments. If there
    // is an argument, we require it to be a port number to bind to.
    if(argc > 1) {
        print_help("Invalid number of arguments");
        return false;
    } else if(argc == 1) {
        long int port;
        if(!iw_strtol(argv[0], &port, 10) ||
           port < 0 || port > 65535)
        {
            print_help("Invalid port number");
            return false;
        }
        s_port = port;
        printf("Using port number %d\n", s_port);
    }

    // Add a log level for simple server specific logs.
    iw_log_add_level(SIMPLE_LOG, "The simple application general log level");

    // Add a command to display the currently connected clients.
    iw_cmd_add(NULL, "connections", list_connections,
            "List currently connected clients",
            "Displays information regarding all currently connected clients such as\n"
            "the file descriptor for the socket connection.\n"
            );

    // Create the mutex to protect the TCP connection list.
    s_mutex = iw_mutex_create("TCP Connections");

    // Open the server socket
    LOG(SIMPLE_LOG, "Starting the simple server.");
    iw_ip address;
    if(!iw_ip_str_to_addr("::", &address) ||
       !iw_ip_set_port(&address, s_port)  ||
       ((s_sock = iw_ip_open_server_socket(SOCK_STREAM, &address)) == -1))
    {
        LOG(SIMPLE_LOG, "Failed to open server socket");
        return false;
    }

    IW_SYSLOG(LOG_INFO, SIMPLE_LOG, "Opened server socket on port %d!",
              s_port);

    // Start serving clients
    if(!serve_data()) {
        return false;
    }

    return true;
}

// --------------------------------------------------------------------------
//
// Main program
//
// --------------------------------------------------------------------------

/// @brief The program main entry-point.
/// @param argc The number of arguments.
/// @param argv The arguments.
/// @return -1 if an error occurred.
int main(int argc, char **argv) {
    unsigned int exit_code = -1;

    // No calls to the instaworks framework should be done before calling
    // iw_init() or before iw_main() calls the provided callback function.

    // However, default settings should be changed before the call to
    // iw_main() to make sure that settings that are processed by iw_main()
    // are set before they are accessed.
    iw_stg.iw_syslog_size = 1000;
    iw_stg.iw_cmd_port    = 10001;

    // Calling iw_main(). The exit code tells us if the program was invoked
    // as a client or server, and whether the command line parameters
    // were invalid or not. If the command line parameters are invalid, the
    // program can print out help for the user.
    IW_MAIN_EXIT retval = iw_main(main_callback, true, argc, argv);

    switch(retval) {
    case IW_MAIN_SRV_INVALID_PARAMETER :
        print_help("Invalid command-line options");
        break;
    case IW_MAIN_SRV_NO_OPTS :
        print_help(NULL);
        break;
    case IW_MAIN_SRV_OK :
    case IW_MAIN_CLNT_OK :
        exit_code = 0;
        break;
    case IW_MAIN_SRV_FAILED :
        printf("Failed to start program!\n");
        break;
    default :
        break;
    }

    return exit_code;
}

// --------------------------------------------------------------------------
