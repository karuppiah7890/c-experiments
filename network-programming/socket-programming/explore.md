How to handle HTTP Clients

```c
char *response = "HTTP/1.1 200 OK\r\n\r\n";
```

```c
char *response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
```

---

`shutdown()` with `SHUT_RDWR`, `SHUT_WR`, `SHUT_RD`

---

Different errors:
- `curl: (52) Empty reply from server`
- `curl: (1) Received HTTP/0.9 when not allowed`
- `curl: (56) Recv failure: Connection reset by peer`
- In thread-per-client server side - `error closing client socket file descriptor in child process: Bad file descriptor` - This was because the socket was closed using `close()` before reading from it or writing to it
    - Not able to read on server side or client side about what the other said
        - This was because the socket was closed using `close()` before reading from it or writing to it

---

Server handling multiple clients:

1. One client only at a time - Tried one version
2. fork() per client - handle multiple clients simultaneously / concurrently - Tried one version
3. thread per client - handle multiple clients simultaneously / concurrently
4. select()/poll() - handle multiple clients simultaneously / concurrently
5. epoll/kqueue - handle multiple clients simultaneously / concurrently
6. async runtimes - handle multiple clients simultaneously / concurrently
