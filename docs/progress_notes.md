# Session 01 – Building a Minimal HTTP Server

## Objective

Understand how a TCP server works from the operating system level and build a minimal HTTP server without using external libraries.

---

## Concepts Learned

### TCP Socket Lifecycle

Implemented the complete server lifecycle:

```
socket()
    ↓
bind()
    ↓
listen()
    ↓
accept()
```

Learned:

- Sockets are kernel-managed communication endpoints.
- File descriptors reference kernel socket objects.
- `bind()` assigns an IP address and port.
- `listen()` places the socket into passive mode.
- `accept()` creates a new connected socket for each client while the listening socket continues accepting future connections.

---

### Client Connections

Learned the difference between:

- **Listening socket** → waits for incoming connections.
- **Connected socket** → communicates with a single client.

```
Browser
    │
connect()
    │
    ▼
Listening Socket
        │
accept()
        │
        ▼
Connected Socket
```

---

### Receiving and Sending Data

Implemented:

- `recv()`
- `send()`

Used `netcat` and a web browser to exchange data with the server.

---

### HTTP Request Parsing

Created an `http_request` structure and parsed the request line using `strtok()`.

Example:

```
GET / HTTP/1.1
```

↓

```
Method  : GET
Path    : /
Version : HTTP/1.1
```

---

### HTTP Response

Created an `http_response` structure to represent server responses.

Implemented a serializer that converts the response structure into a valid HTTP message.

Example:

```
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 33

<html><h1>Hello world</h1></html>
```

---

## Architecture

```
Browser
    │
    ▼
recv()
    │
    ▼
parse_http_request()
    │
    ▼
create_http_response()
    │
    ▼
serialize_http_response()
    │
    ▼
send()
```

---

## Project Structure

```
src/
├── server/
├── http/
├── response/
└── main.c
```

Each module has a single responsibility:

- **server/** → TCP networking
- **http/** → HTTP request parsing
- **response/** → HTTP response creation and serialization

---

## Milestone Achieved

- TCP server implemented
- Accepts browser connections
- Parses HTTP requests
- Generates HTTP responses
- Serializes responses into valid HTTP format
- Sends responses back to the browser
- Browser successfully renders HTML

---

## Next Session

Implement static file serving:

- Routing
- Serve `index.html`
- 404 responses
- MIME type detection