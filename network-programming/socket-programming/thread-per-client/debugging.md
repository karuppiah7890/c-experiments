Use the following for debugging

```bash
sudo apt update
sudo apt install strace ltrace

strace -h
# OR
strace --help

strace -c ./server

ltrace -h
# OR
ltrace --help

ltrace -c ./server
```

```bash
ps aux | rg server
```

```bash
# Look at the Recv-Q and Send-Q columns
# Recv-Q column value shouldn't be growing
# TODO: Read more on this and how this works and what this shows
watch "ss -lnt"
```
