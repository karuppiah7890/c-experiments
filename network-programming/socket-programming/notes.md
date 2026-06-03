Learnings from ChatGPT AI :P -

When forking using `fork()`, a copy of the file descriptor (FD) table of the parent is made for the child. The file descriptor table is maintained at the kernel side, and the programs only have the index of the file descriptor table, and the index is the file descriptor integer that the program has. The index is used to make system calls and the kernel finds out the actual reference using the index and the file descriptor table to get access to the file. If the file descriptor refers to a socket, the file descriptor table has references to the kernel socket object. When forking using `fork()`, the child process gets a copy of the file descriptor (FD) table of the parent process and any socket references are shared - so, the file descriptor tables are separate but are copies (same copy) but the references to the socket are the same - so, they refer to the same kernel socket object reference. Only if all the file descriptor to a kernel socket object references are removed, only then the kernel destroy the socket, or else the socket remains till there's at least one kernel socket object reference. The kernel keeps track of the reference count on the socket for the kernel socket object references. When reference count becomes 0, it does the cleanup and destroys the socket

---

When sockets are not explicitly closed using `close()`, the kernel still closes them when the process exits. This is kinda like garbage collection I guess. And when there are no references remaining for a socket, the kernel destroys the socket

---

The max value of somaxconn (SOMAXCONN) is `2147483647` or 2,147,483,647 and that's around 2 Billion

The second argument of `listen()` system call is called `backlog` and the max value of `backlog` is SOMAXCONN

Read more about `backlog` in `man listen`

Check it out using `sysctl` or `cat`

```bash
sysctl net.core.somaxconn

cat /proc/sys/net/core/somaxconn
```

And update it using `sysctl` or `echo`

```bash
sysctl net.core.somaxconn

cat /proc/sys/net/core/somaxconn
```

And these changes reset on reboot. For permanent changes to SOMAXCONN value - you need to write the configuration to `/etc/sysctl.conf` file with the line `net.core.somaxconn = 1024` for example

