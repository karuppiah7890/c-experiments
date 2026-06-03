```bash
sudo apt update

sudo apt install -y gdb apache2-utils gcc telnet forkstat graphviz strace ltrace tcpdump tcptrace tshark wireshark

sudo apt install -y linux-tools-common

sudo apt install -y linux-tools-$(uname -r)
```

```bash
sysctl net.core.somaxconn

cat /proc/sys/net/core/somaxconn
```

```bash
while true; do ./client; done;
```

```bash
while true; do ./client & done;
```
