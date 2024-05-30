# gdb

## array

int arr[5] = { 0, 1, 2, 3, 4 };
int *p = arr;

```gdb
p *p@5
```

## server

```bash
./gdbserver 0.0.0.0:8008 ./a.out

# client side
gdb ./a.out
target remote 10.0.0.101:8008

```