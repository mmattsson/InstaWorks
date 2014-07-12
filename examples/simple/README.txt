Simple
=============================================================================

Program usage
-----------------------------------------------------------------------------
This example shows a simple TCP server. The server accepts connections on
the listening port. If data is received on any connection, this data is sent
on all other connections. This program can be tested by connecting by telnet
to the listening port, e.g.:
 $ examples/simple/simple -f -l 0xF
And in other terminals:
 $ telnet localhost 1234
or
 $ telnet :: 1234
When you have multiple terminals connected you can simply type text into
one terminal and it will show up in all other terminals.


Control commands
-----------------------------------------------------------------------------
Once the program is running with a few connections, you can issue control
commands to query the current state of the program.
-----------------------------------------------------------------------------
 $ examples/simple/simple syslog show
Received request: syslog show
LOG: [2014-06-19 06:11:16314400] Opened server socket on port 1234!
LOG: [2014-06-19 06:11:27621542] Accepted socket FD=7 from client ::1:45901
LOG: [2014-06-19 06:11:36075297] Accepted socket FD=8 from client ::1:45902
LOG: [2014-06-19 06:11:56675675] Accepted socket FD=9 from client ::ffff:192.168.2.1:39882

  $ examples/simple/simple memory show
Received request: memory show
== Memory summary ==
Number allocations:      6
Number frees:            1
Outstanding allocations: 5
Outstanding allocations: 2 KBytes
Accumulated allocations: 3 KBytes
Memory corruptions:      0
Pre-guard corruptions:   0
Post-guard corruptions:  0

== Allocated Memory ==
Memory[0847fd54]: main.c:75 (148 Bytes)
Memory[0847fc6c]: main.c:75 (148 Bytes)
Memory[08481e44]: main.c:75 (148 Bytes)
Memory[b6600484]: src/iw_buff.c:31 (1 KBytes)
Memory[0847e224]: src/iw_syslog.c:219 (1000 Bytes)

 $ examples/simple/simple connections
Received request: connections
Connection 0  : FD=7 Client=::1/45901, RX=13 bytes, TX=12 bytes
Connection 1  : FD=8 Client=::1/45902, RX=6 bytes, TX=19 bytes
Connection 2  : FD=9 Client=::ffff:192.168.2.1/39882, RX=6 bytes, TX=19 bytes
-----------------------------------------------------------------------------
Out of these commands, "syslog show" and "memory show" are built into the
InstaWorks framework. The "connections" command is a custom command added
by the simple server.


Thread-specific logging
-----------------------------------------------------------
It is also possible to set the logging on a per thread basis. To only see
logs related to the main thread (the thread serving connections) in this
example, you can first turn off logs for all threads, then enable them for
just the main thread.
-----------------------------------------------------------------------------
$ examples/simple/simple log thread all off
Received request: log thread all off

$ examples/simple/simple threads
Received request: threads
== Thread Information ==
Thread-ID  Log Mutex  Thread-name
---------------------------------
[B6F66B40] off 0000 : "CMD Server"
[B7767B40] off 0000 : "Health Check"
[B7768940] off 0000 : "Main"

$ examples/simple/simple log thread 0xB7768940 on
Received request: log thread 0xB7768940 on

$ examples/simple/simple log lvl 0xF `tty`
Received request: log lvl 0xF /dev/pts/0

[B7768940]main.c(157): Accepted socket FD=9 from client ::1:49725
[B7768940]main.c(79): ++ calloc(1,148)

-----------------------------------------------------------------------------
From this point on, only the Main thread's log output is shown so that we can
concentrate on debugging the client connections.


Client-specific logging
-----------------------------------------------------------------------------
In addition to the built-in thread-specific logging, the simple example
adds client-specific logging. The simple server is registering a 'client'
sub-command under the 'log' command. This command takes an IP address
and the word 'on' or 'off'. When the command callback is called, the client
list is searched for a client with the given IP address and port. If this
client is found, the logging for that client is enabled or disabled.

This command is implemented in a simplistic fashion to keep the simple 
server example straight-forward, but could be improved by having a separate
list of IP addresses that logging should be done for. When a new connection
is established, the logging for that connection would be set if the peer
matches an IP address in the list.
-----------------------------------------------------------------------------
 $ examples/simple/simple connections
 Received request: connections
Connection 0  : FD=8 log=on  Client=[::1]:49870, RX=0 bytes, TX=0 bytes
Connection 1  : FD=9 log=on  Client=[::1]:49871, RX=0 bytes, TX=0 bytes

 $ examples/simple/simple log client [::1]:49870 off
 $ examples/simple/simple connections
 Received request: connections
Connection 0  : FD=8 log=off Client=[::1]:49870, RX=0 bytes, TX=0 bytes
Connection 1  : FD=9 log=on  Client=[::1]:49871, RX=0 bytes, TX=0 bytes


Implementation notes
=============================================================================
The simple example lets the InstaWorks framework handle the parsing of the
command line options.

The first thing that is done in the main() function is to set the desired
InstaWorks settings by assigning values to the members of iw_stg.

No log levels or control commands can be registered before the call to
iw_main(). These have to be registered after the callback is done so this
is done in main_callback().

The simple server registers two commands, 'connections' and 'log client'.
'connections' is registered as a top-level command with no children and
'log client is registered as an extension to the existing 'log' command.
