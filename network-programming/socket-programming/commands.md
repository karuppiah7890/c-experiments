```bash
sudo apt update

sudo apt install -y wrk gdb apache2-utils gcc telnet forkstat graphviz strace ltrace tcpdump tcptrace tshark wireshark

sudo apt install -y linux-tools-common

sudo apt install -y linux-tools-$(uname -r)
```

```bash
sysctl net.core.somaxconn

cat /proc/sys/net/core/somaxconn
```

```bash
sudo su
ulimit -a
ulimit -n
ulimit -n unlimited # it might not work
ulimit -n 200000
ulimit -n 999999
ulimit -n 1000000
ulimit -n 1009999
```

```bash
gcc server.c -o server
gcc client.c -o client
```

```bash
./server
```

```bash
while true; do ./client; done;
```

```bash
while true; do ./client & done;
```

```bash
sudo apt update
sudo apt install -y wrk
wrk -t8 -c1000 -d30s http://box-01:8080/
```

```bash
sudo apt update
sudo apt install -y apache2-utils
ab -n 100000 -c 1000 http://box-01:8080/
```

