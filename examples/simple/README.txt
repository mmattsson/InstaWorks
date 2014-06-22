Simple
=============================================================================
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


Implementation notes
=============================================================================
The simple example lets the InstaWorks framework handle the parsing of the
command line options.

The first thing that is done in the main() function is to set the desired
InstaWorks settings by assigning values to the members of iw_stg.

No log levels or run-time queries can be registered before the call to
iw_main(). These has to be registered after the callback is done so this
is done in main_callback().

