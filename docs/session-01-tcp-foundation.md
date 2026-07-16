# Session 01 – TCP Server Foundation

## Objective

Before writing networking code, the goal of this session was to understand how a TCP server works internally instead of memorizing socket API calls.

The focus was understanding the operating system's role in creating, managing, and exposing network communication through system calls.

---

# Concepts Learned

## 1. What is a Socket?

A socket is a communication endpoint created and managed by the operating system.

It is **not a connection itself**.

A socket represents a resource that allows a process to communicate using a network protocol.

Similar to files and pipes in Unix, sockets are represented using **file descriptors**.

Conceptually:

```
Application

      |
      v

Socket File Descriptor

      |
      v

Kernel Socket Object
```

The application interacts with the socket through the file descriptor while the kernel manages the actual networking operations.

---

# 2. Why Does `socket()` Return an Integer?

Unix systems represent resources using file descriptors.

The operating system maintains a table that maps file descriptors to kernel objects.

Example:

```
Process File Descriptor Table

0  -> stdin
1  -> stdout
2  -> stderr
3  -> file
4  -> pipe
5  -> socket
```

When `socket()` is called:

```
Application

      |
      v

socket()

      |
      v

Kernel creates socket object

      |
      v

Returns file descriptor
```

The integer is not the socket itself.

It is a reference that the process uses to access the kernel-managed socket.

---

# 3. Why Are `socket()` and `bind()` Separate?

Creating a socket does not make it reachable over the network.

After:

```
socket()
```

the socket has:

- no IP address
- no port number
- no network identity

It is only an endpoint created inside the operating system.

`bind()` assigns an address to that socket.

Example:

```
Before bind:

Socket FD 5

(no address assigned)


After bind:

127.0.0.1:8080

        |

     Socket FD 5
```

The kernel now knows where incoming traffic for this endpoint should be delivered.

---

# 4. Why Can Only One Server Normally Use the Same Port?

The operating system maintains a mapping between:

```
IP Address
      +
Port Number
      +
Socket
```

Example:

```
127.0.0.1:8080

        |

   HTTP Server
```

Normally, another process cannot bind to the same IP and port combination because the kernel would not know which process should receive incoming traffic.

The OS prevents multiple processes from competing for the same endpoint.

---

# 5. Why Must Sockets Be Closed?

Sockets consume operating system resources:

- file descriptor entries
- network buffers
- socket state information

When a socket is no longer needed, it should be released using:

```c
close()
```

This follows the Unix resource management pattern:

```
malloc()  -> free()

open()    -> close()

pipe()    -> close()

socket()  -> close()
```

Every acquired resource should eventually be released.

---

# 6. Understanding Network Interfaces

A computer can have multiple network interfaces.

Each interface can have its own IP address.

## Loopback Interface

```
127.0.0.1
```

Used for communication inside the same machine.

Example:

```
Browser
   |
   |
localhost
   |
   |
Local Server
```

Only programs running on the same computer can access it.

---

## Local Network Interface

Example:

```
192.168.x.x
```

Used for communication inside a local network.

Other devices connected to the same LAN can communicate using this address.

---

## All Interfaces

```
0.0.0.0
```

This is a special address meaning:

```
Listen on every available IPv4 interface
```

Instead of binding separately to every network interface, the server can bind once and accept traffic arriving through any interface.

---

# 7. IPv4, TCP, and Protocol Selection

A TCP socket is created using:

```
socket(AF_INET, SOCK_STREAM, 0)
```

The arguments describe the communication type.

## AF_INET

Specifies the address family:

```
IPv4
```

Example:

```
192.168.1.10
```

---

## SOCK_STREAM

Specifies a stream-based communication method:

```
TCP
```

TCP provides:

- reliable delivery
- ordered data transfer
- retransmission of lost packets
- connection-based communication

---

## Protocol = 0

The value `0` tells the operating system:

```
Choose the default protocol for this socket type.
```

For:

```
AF_INET + SOCK_STREAM
```

the kernel selects TCP.

---

# 8. sockaddr_in Structure

IPv4 address information is stored using:

```c
struct sockaddr_in
```

The structure contains:

```
sin_family

    |
    +-- Address family (IPv4)


sin_addr

    |
    +-- IPv4 address


sin_port

    |
    +-- Port number
```

Example configuration:

```
Address Family: IPv4

IP Address: 0.0.0.0

Port: 8080
```

---

# 9. Network Byte Order

Different computer architectures may store numbers differently in memory.

The internet uses a standard representation:

```
Network Byte Order
```

Conversion functions are used before storing values inside networking structures.

## htons()

Host To Network Short

Used for:

```
Port numbers
```

Example:

```
8080

Host byte order

        |

      htons()

        |

Network byte order
```

---

## htonl()

Host To Network Long

Used for larger integer values.

Example:

```
INADDR_ANY
```

---

# 10. bind()

The `bind()` system call connects:

```
Socket File Descriptor

        +

IP Address

        +

Port Number
```

Before bind:

```
Socket FD 5

No address assigned
```

After bind:

```
Socket FD 5

owns

0.0.0.0:8080
```

The operating system now knows:

```
Traffic arriving at this endpoint belongs to this process.
```

---

# Implementation Completed

After understanding the theory, the following functionality was implemented:

- Created a TCP IPv4 socket using `socket()`
- Created and initialized `struct sockaddr_in`
- Configured:
  - IPv4 address family (`AF_INET`)
  - All network interfaces (`INADDR_ANY`)
  - Port number using network byte order (`htons`)
- Bound the socket using `bind()`
- Returned the server socket file descriptor to `main()`

Current implementation flow:

```
main.c

    |
    |
    v

start_server(port)

    |
    |
    +------ socket()

    |
    |
    +------ Create sockaddr_in

    |
    |
    +------ Configure IP + Port

    |
    |
    +------ bind()

    |
    |
    v

Return server file descriptor
```

---

# Important Design Decisions

## Separation Between main.c and server.c

The port configuration belongs to the application layer.

Example:

```
main.c

port = 8080

      |

      v

server.c

creates networking resources
```

This allows future configuration changes without modifying server implementation.

Example:

```
./http-server --port 3000
```

---

# Current Server State

Completed:

✅ Create TCP socket  
✅ Configure IPv4 address  
✅ Assign IP and port  
✅ Bind socket to endpoint  

Not implemented yet:

❌ Listen for incoming connections  
❌ Accept clients  
❌ Parse HTTP requests  
❌ Generate HTTP responses  
❌ Serve static files  
❌ Routing system  
❌ Logging  
❌ Thread pool  

---

# Current Server Lifecycle

```
socket()

    |

bind()

    |

listen()

    |

accept()

    |

Client Connection
```

At the current stage, the server owns an address and port but is not accepting clients yet.

---

# Key Takeaways

- A socket is a kernel-managed communication endpoint.
- Socket descriptors are references into the process file descriptor table.
- `socket()` creates the communication endpoint.
- `bind()` assigns an IP address and port.
- `0.0.0.0` allows listening on all IPv4 interfaces.
- TCP sockets provide reliable, ordered communication.
- Network byte order is required for network communication.
- Resources created by the operating system must be released.
- Understanding the operating system is more important than memorizing API calls.

---

# Next Session

## TCP Listening State

The next milestone will implement:

```
socket()

    |

bind()

    |

listen()

    |

accept()
```

Topics:

- How a socket enters listening state
- What the backlog queue represents
- How the kernel stores pending client connections
- Difference between listening sockets and connected sockets
- Why `accept()` creates a new socket