# HTTP Server in C

A custom HTTP server built from scratch in C using low-level POSIX networking APIs.

The goal of this project is to understand how web servers work internally by implementing the networking stack, request handling, and server architecture without relying on existing HTTP frameworks.

This project focuses on:

- Operating system concepts
- TCP/IP networking
- Socket programming
- Concurrent programming
- Memory management
- HTTP protocol internals
- Clean systems architecture

---

# Project Motivation

Modern web servers hide a large amount of complexity behind frameworks and libraries.

This project explores what happens underneath:

```
Client

  |
  |
TCP Connection

  |
  |
HTTP Request

  |
  |
Server Processing

  |
  |
HTTP Response
```

Instead of using existing abstractions, this server is built directly on top of Unix system calls.

The objective is not to create a production replacement for Nginx or Apache, but to develop a deep understanding of how network servers operate internally.

---

# Current Status

## Completed

### TCP Foundation

- [x] Create IPv4 TCP socket
- [x] Configure socket address
- [x] Bind socket to IP address and port
- [x] Proper error handling
- [x] Modular server architecture

Current flow:

```
main.c

    |
    v

start_server()

    |
    +---- socket()

    |
    +---- sockaddr_in setup

    |
    +---- bind()

    |
    v

Server socket created
```

---

## In Progress

TCP connection handling:

- [ ] listen()
- [ ] accept()
- [ ] Client connection lifecycle
- [ ] Connection cleanup

---

## Planned Features

### HTTP Layer

- [ ] HTTP request parsing
- [ ] HTTP method handling
- [ ] Header parsing
- [ ] HTTP response generation
- [ ] Status codes

---

### File Serving

Support serving static files:

Example:

```
GET /index.html
```

Response:

```
HTTP/1.1 200 OK

<html>
    ...
</html>
```

---

### Routing System

Implement a lightweight routing mechanism:

Example:

```
GET /about

GET /api/users

POST /login
```

---

### Logging System

Server logging:

- Client requests
- Connection events
- Errors
- Server activity

---

### Concurrency

Implement concurrent request handling:

Options:

- Thread pool
- Worker threads
- Connection management

Architecture:

```
              Server

                 |
                 |

        Connection Queue

                 |
        -----------------

        Worker  Worker  Worker

          |       |       |

       Client  Client  Client
```

---

# Architecture

Current structure:

```
http-server/

├── src/
│
│   ├── main.c
│   │
│   └── server/
│       │
│       ├── server.c
│       └── server.h
│
├── docs/
│   │
│   └── session-01-tcp-foundation.md
│
├── build/
│
├── Makefile
│
└── README.md
```

The project follows a modular design where:

```
main.c

Application control

        |

        v

server/

Networking implementation
```

Responsibilities are separated to keep the code maintainable as new features are added.

---

# Technologies Used

## Language

```
C
```

---

## Networking

POSIX socket APIs:

- socket()
- bind()
- listen()
- accept()
- send()
- recv()

---

## Operating System Concepts

- File descriptors
- Kernel resources
- Process communication
- Memory management
- System calls

---

## Build System

```
Makefile
```

Compiler:

```
gcc
```

Compilation flags:

```
-Wall
-Wextra
-g
```

---

# How It Works

## Server Lifecycle

The final architecture will follow:

```
Create Socket

      |

Bind Address

      |

Listen For Connections

      |

Accept Client

      |

Receive HTTP Request

      |

Parse Request

      |

Generate Response

      |

Send Response

      |

Close Connection
```

---

# Build Instructions

Clone repository:

```bash
git clone <repository-url>
```

Navigate:

```bash
cd http-server
```

Build:

```bash
make
```

Run:

```bash
./build/http-server
```

---

# Development Approach

This project is developed incrementally.

Each milestone focuses on:

1. Understanding the underlying operating system concept.
2. Implementing the feature manually.
3. Testing behavior.
4. Documenting the learning.

Development notes are maintained inside:

```
docs/
```

Example:

```
session-01-tcp-foundation.md
```

---

# Engineering Principles

## Understand Before Implementing

The goal is not to copy existing implementations.

Each component is studied:

```
Theory

   |

Implementation

   |

Testing

   |

Documentation
```

---

## Resource Management

All operating system resources should have clear ownership.

Examples:

```
socket()

    |

close()
```

Similar patterns:

```
malloc()

    |

free()
```

```
open()

    |

close()
```

---

## Clean Architecture

The project avoids putting everything inside one file.

Responsibilities are separated:

```
Application Logic

        |

Server Logic

        |

Networking Layer

        |

Operating System APIs
```

---

# Learning Goals

By completing this project, the goal is to understand:

- How TCP servers work internally
- How operating systems manage sockets
- How HTTP communication happens
- How concurrent servers are designed
- How low-level networking APIs work
- How production systems are built from simpler primitives

---

# Future Improvements

Possible future additions:

- HTTP/1.1 keep-alive support
- Configuration files
- Better logging system
- Request limits
- Memory profiling
- Performance testing
- Load testing
- Security improvements

---

# Related Projects

This project follows a systems programming learning path:

Previous:

## Unix-like Shell in C

Concepts explored:

- fork()
- execvp()
- pipes
- redirection
- signals
- process management

Current:

## HTTP Server in C

Concepts explored:

- TCP/IP
- sockets
- networking
- concurrency
- server architecture

---

# Author Notes

This project is built as a deep dive into computer science fundamentals.

The objective is not only to make a working HTTP server, but to understand the layers underneath modern internet infrastructure.