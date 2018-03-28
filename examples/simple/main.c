// --------------------------------------------------------------------------
///
/// @file main.c
///
/// A simple example program using the InstaWorks framework. This program
/// will create a server socket and will listen for incoming TCP connections.
/// It will read data on each TCP connection and write the received data on
/// the other TCP connections.
///
/// Copyright (c) 2014-2015 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include <iw_cfg.h>
#include <iw_cmds.h>
#include <iw_cmdline.h>
#include <iw_ip.h>
#include <iw_log.h>
#include <iw_list.h>
#include <iw_main.h>
#include <iw_memory.h>
#include <iw_mutex.h>
#include <iw_syslog.h>
#include <iw_thread.h>
#include <iw_util.h>

#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

// --------------------------------------------------------------------------
//
// Helper data structures
//
// --------------------------------------------------------------------------

#define TIMEOUT_SEC     1
#define TIMEOUT_USEC    0

#define SIMPLE_LOG      8

#define DEFAULT_PORT    1234

#define SIMPLE_CFG      "/tmp/simple.cfg"

/// @brief The TCP connection object.
/// Each client connection that is received will be represented by a TCP
/// connection object. This object will keep track of the client socket file
/// descriptor, the number of received bytes, the number of sent bytes, and
/// the address of the client.
typedef struct _tcp_conn {
    iw_list_node node;    ///< The base list object.
    int          fd;      ///< The TCP socket file descriptor.
    unsigned int rx;      ///< The number of bytes received on the connection.
    unsigned int tx;      ///< The number of bytes sent on the connection.
    iw_ip        address; ///< The peer address.
    bool         do_log;  ///< True if logging should be done for this connection.
} tcp_conn;

// --------------------------------------------------------------------------
//
// Local data
//
// --------------------------------------------------------------------------

static int      s_sock = -1;               ///< The server socket.
static iw_list  s_list = IW_LIST_INIT_MEM; ///< The TCP socket list.
static IW_MUTEX s_mutex;                   ///< The mutex to protect the sockets.
static unsigned short s_port = DEFAULT_PORT;///< The port number to use.
static bool     s_keep_going = true;       ///< True as long as the program should execute.

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
    conn->do_log = true;
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
/// @param html True if the output should be HTML formatted.
static bool list_connections(FILE *out, bool html) {
    int cnt = 0;
    iw_mutex_lock(s_mutex);
    tcp_conn *conn = (tcp_conn *)s_list.head;
    if(conn == NULL) {
        fprintf(out, "no connections\n");
    } else {
        for(;conn != NULL;cnt++) {
            char ipbuff[IW_IP_BUFF_LEN];
            if(html) {
                fprintf(out,
                    "<h2>Connection %-3d</h2>\n"
                    "<table class='data'>\n"
                    "<tr><td>File Descriptor</td><td>%d</td></tr>\n"
                    "<tr><td>Logging Enabled</td><td>%s</td></tr>\n"
                    "<tr><td>Client Address</td><td>%s</td></tr>\n"
                    "<tr><td>Received bytes</td><td>%d</td></tr>\n"
                    "<tr><td>Sent bytes</td><td>%d</td></tr>\n"
                    "</table>\n",
                    cnt, conn->fd,
                    conn->do_log ? "on " : "off",
                    iw_ip_addr_to_str(&conn->address, true, ipbuff, sizeof(ipbuff)),
                    conn->rx, conn->tx);
            } else {
                fprintf(out,
                    "Connection %-3d: FD=%d log=%s Client=%s, RX=%d bytes, TX=%d bytes\n",
                    cnt, conn->fd,
                    conn->do_log ? "on " : "off",
                    iw_ip_addr_to_str(&conn->address, true, ipbuff, sizeof(ipbuff)),
                    conn->rx, conn->tx);
            }
            conn = (tcp_conn *)conn->node.next;
        }
    }
    iw_mutex_unlock(s_mutex);
    return true;
}

// --------------------------------------------------------------------------

/// @brief List all peers currently connected to the server.
/// @param out The output file stream to display the connections on.
static bool list_conn_gui(FILE *out) {
    return list_connections(out, true);
}

// --------------------------------------------------------------------------

/// @brief List all peers currently connected to the server.
/// @param out The output file stream to display the connections on.
/// @param cmd The command request being processed.
/// @param info The parse info to use when parsing the request.
static bool list_conn_cmd(FILE *out, const char *cmd, iw_cmd_parse_info *info) {
    return list_connections(out, false);
}

// --------------------------------------------------------------------------

/// @brief Enable or disable client logging.
static bool log_client(FILE *out, const char *cmd, iw_cmd_parse_info *info) {
    iw_ip address;
    char *ipstr    = iw_cmd_get_token(info);
    char *onoffstr = iw_cmd_get_token(info);
    if(ipstr == NULL || !iw_ip_str_to_addr(ipstr, true, &address)) {
        fprintf(out, "\nInvalid address\n");
        return false;
    }
    if(onoffstr == NULL || (strcmp(onoffstr, "on") != 0 && strcmp(onoffstr, "off") != 0)) {
        fprintf(out, "\nInvalid value, should be either on of off\n");
        return false;
    }
    bool log_on = false;
    if(strcmp(onoffstr, "on") == 0) {
        log_on = true;
    }

    iw_mutex_lock(s_mutex);
    tcp_conn *conn = (tcp_conn *)s_list.head;
    while(conn != NULL) {
        if(iw_ip_equal(&address, &conn->address, true)) {
            conn->do_log = log_on;
            break;
        }
        conn = (tcp_conn *)conn->node.next;
    }
    if(conn == NULL) {
        char ipbuff[IW_IP_BUFF_LEN];
        fprintf(out, "\nAddress %s was not found\n",
                iw_ip_addr_to_str(&address, true, ipbuff, sizeof(ipbuff)));
    }
    iw_mutex_unlock(s_mutex);
    return true;
}

// --------------------------------------------------------------------------

/// @brief Print help for program usage.
/// @param error The error to print, if any.
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
           "    The port number to use for the server (default is %d)\n"
           "\n", DEFAULT_PORT);
    iw_cmdline_print_help();
    printf("\n"
           "If the program is started without any command line options it will\n"
           "run in client mode and send control commands to a running server.\n"
           "Run 'simple help' once the server is running for more help on this.\n"
           "\n");
}

// --------------------------------------------------------------------------
//
// Main program
//
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
    while(s_keep_going && (retval = select(maxfd + 1,
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
                IW_SYSLOG(LOG_INFO, SIMPLE_LOG, "Accepted socket FD=%d from client %s",
                        sock, iw_ip_addr_to_str(&address, true, ipbuff, sizeof(ipbuff)));
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
                // First check if logging should be done for this connection
                bool do_log = iw_thread_get_log(0);
                iw_thread_set_log(0, conn->do_log);

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
                    IW_SYSLOG(LOG_INFO, SIMPLE_LOG, "Socket FD=%d, client %s, is closed",
                            conn->fd, iw_ip_addr_to_str(&conn->address, true, ipbuff, sizeof(ipbuff)));
                    close(conn->fd);
                    conn = (tcp_conn *)iw_list_delete(&s_list,
                                                      (iw_list_node *)conn,
                                                      delete_tcp_conn);
                    continue;
                }

                // Revert logging after we're done
                iw_thread_set_log(0, do_log);
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

    // Free up all allocated resources still in use at exit
    iw_mutex_lock(s_mutex);
    conn = (tcp_conn *)s_list.head;
    while(conn != NULL) {
        close(conn->fd);
        tcp_conn *tmp = (tcp_conn *)conn->node.next;
        conn = (tcp_conn *)iw_list_delete(&s_list,
                                            (iw_list_node *)conn,
                                            delete_tcp_conn);
        conn = tmp;
    }
    iw_mutex_unlock(s_mutex);

    if(retval == -1) {
        LOG(SIMPLE_LOG, "Select failed, exiting (%d:%s)", errno, strerror(errno));
        return false;
    }
    return true;
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
        if(!iw_ip_str_to_port(argv[0], &s_port)) {
            print_help("Invalid port number");
            return false;
        }
        printf("Using port number %d\n", s_port);
    }

    // Set the callback for displaying run-time statistics in the Web GUI.
    iw_cb.runtime = list_conn_gui;

    // Add commands to display the currently connected clients and to set
    // log level based on client connections.
    iw_cmd_add(NULL, "connections", list_conn_cmd,
            "List currently connected clients",
            "Displays information regarding all currently connected clients\n"
            "such as the file descriptor for the socket connection.\n");
    iw_cmd_add("log", "client", log_client,
            "Enable or disable logging for a given client",
            "Used to enable or disable logging for a given client by specifying\n"
            "the peer IP address and port, e.g. 'log client 1.1.1.1:" IW_STR(DEFAULT_PORT) " on'.\n");

    // Create the mutex to protect the TCP connection list.
    s_mutex = iw_mutex_create("TCP Connections");

    // Open the server socket
    LOG(SIMPLE_LOG, "Starting the simple server.");
    iw_ip address;
    if(!iw_ip_ipv6_to_addr(in6addr_any, &address) ||
       !iw_ip_set_port(&address, s_port)  ||
       ((s_sock = iw_ip_open_server_socket(SOCK_STREAM, &address, true)) == -1))
    {
        LOG(SIMPLE_LOG, "Failed to open server socket");
        return false;
    }

    printf("Opened simple server on TCP port %d!\n\n"
           "Connect in using 'telnet localhost %d' from a couple of terminals\n"
           "and then type text into the telnet sessions.\n\n"
           "You can use the web GUI by pointing your browser to http://localhost:8080\n"
           "and look at the programs run-time data and configuration settings.\n\n",
           s_port, s_port);

    // Start serving clients
    if(!serve_data()) {
        return false;
    }

    return true;
}

// --------------------------------------------------------------------------

/// @brief The termination notification callback.
/// Called when the program should terminate.
void main_term() {
    s_keep_going = false;
}

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
    // are set before they are accessed and to do so, iw_cfg_init() must be
    // called.
    iw_cfg_init(SIMPLE_CFG);
    iw_val_store_set_number(&iw_cfg, IW_CFG_ALLOW_QUIT, 1, NULL, 0);

    // Add the simple server port number as a configurable value
    iw_cfg_add_number("cfg.port", true, "The port number must be between 1025 and 65535",
                      "^(102[4-9]|10[3-9][0-9]|1[1-9][0-9]{2}|[2-9][0-9]{3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$",
                      DEFAULT_PORT);

    // Also, adding log levels should be done before the iw_main() call
    // so that the log levels can be displayed in the program usage text.
    iw_log_add_level(SIMPLE_LOG, "The simple application general log level");

    // Loading any saved configuration
    iw_cfg_load(SIMPLE_CFG);

    // Calling iw_main(). The exit code tells us if the program was invoked
    // as a client or server, and whether the command line parameters
    // were invalid or not. If the command line parameters are invalid, the
    // program can print out help for the user.
    IW_MAIN_EXIT retval = iw_main(main_callback, main_term, true, argc, argv);

    switch(retval) {
    case IW_MAIN_SRV_INVALID_PARAMETER :
        print_help("Invalid command-line options");
        exit_code = 0;
        break;
    case IW_MAIN_SRV_NO_OPTS :
        print_help(NULL);
        exit_code = 0;
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
