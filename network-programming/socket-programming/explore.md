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

Server handling multiple clients:

1. One client only at a time - Tried one version
2. fork() per client - handle multiple clients simultaneously / concurrently - Tried one version
3. thread per client - handle multiple clients simultaneously / concurrently
4. select()/poll() - handle multiple clients simultaneously / concurrently
5. epoll/kqueue - handle multiple clients simultaneously / concurrently
6. async runtimes - handle multiple clients simultaneously / concurrently
