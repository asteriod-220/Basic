# Low-Level Unix Socket Streaming Engine

A lightweight command line data transfer tool written in C using ("sys.socket.h").

# Features

**TCP and UDP Protocol:** Supports stream oriented TCP and connectionless UDP pipelines via standard CLI options.
**UDP Pacing Engine:** Includes a 'usleep(500)' rate limittin gmechanism in the UDP file loop to prevent local network interface buffer overruns and eliminate silent packet drops during large file transfers.
**Zero Overhead Alignment:** Built with zero high-level library dependencies and structured entirely on standard POSIX blocks. It safely streams heavy payloads (validated up to 2GB+ complete byte-for-byte identical replication) via a clean 4096-byte memory layout buffer.

## Installation & Compilation

Compile using 'gcc'. No external library linkages or configs are required.

'''bash
gcc simple-socket.c -o socket

# Usage

Usage: ./socket -i <ip_addr> -p <port> [options]
  -i <addr>       IP address of target destination (Required=Yes)
  -p <port>       Port number of target (Default=80)
  -t <string>     Send a raw text string payload
  -f <file>       Stream a physical file payload across the network
  -u              Switch communication transport mode to UDP (Defaults=TCP)
  -T <seconds>    Socket connection timeout window in seconds (Defaults=3s)
  -c              Check if the target port/host is up and exit immediately

  # Example

./socket -i 0.0.0.0 -p 8888 -c

./socket -i 8.8.8.8 -p 8725 -f <file> -T 5

./socket -i 8.8.8.8 -t <string> -u
