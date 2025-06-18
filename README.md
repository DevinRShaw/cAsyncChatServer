# Architecture and `uthash` Usage Overview

## Architecture Overview

This project implements a scalable server-client system using the following key components:

### 1. Server Core

- The server uses **epoll**, a Linux-specific I/O event notification facility, to efficiently handle multiple simultaneous client connections in a non-blocking manner.
- The main server loop listens for incoming connections and data, dispatching client handling to dedicated functions.
- Non-blocking sockets and edge-triggered epoll events ensure responsiveness and scalability.

### 2. Client Management Module

- Client management (in `client_handling.c` and `.h`) abstracts the logic of tracking connected clients, verifying unique usernames, and managing client lifecycle.
- It exposes functions such as:
  - `is_client_new(int user_id)` — checks if a client with given ID is already known.
  - `is_username_unique(char* name)` — verifies if a username is not already taken.
  - `add_user(int user_id, char* name)` — adds a new client to internal data structures.

### 3. Communication Protocol

- Clients send their username as the first message after connection.
- The server verifies username uniqueness, adding the client if unique.
- Basic commands such as `/quit` are handled to allow clients to disconnect gracefully.
- Message receipt and transmission happen on non-blocking sockets, integrated with the epoll event loop.

---

## `uthash` Usage

### Purpose

`uthash` is a lightweight, single-header hash table implementation for C, used here to efficiently manage client records indexed by different keys.

### Data Structure

The client records are stored in a struct:

```c
struct client_lookup {
    char name[MAXDATASIZE];  // Client username (key for name hash)
    int id;                  // Client file descriptor or ID (key for ID hash)
    UT_hash_handle hh1;      // Hash handle for id-based hash table
    UT_hash_handle hh2;      // Hash handle for name-based hash table
};
