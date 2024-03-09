# select

## Select-like calls

There are three select-like calls: `select`, `poll`, `epoll`.

## About `select`

- `select` needs to copy `fd_set` to and from kernel. `fd_set` is a bit map of fds. Macros `FD_SET`, `FD_CLR` just modifies this fd-bit-map.
- As the bitmap size is an kernel compile-time value, so it can not change. It's usually 1024. A bigger size is not a good idea because of performance problem.
- Kernel has to go through the fd map to decide the state of each fd. It takes time.
- Note: `select` call needs nfds to save time for not going through whole bit map.
- `select` works fine when fd-set is small, that is, when nfds is small.

## About `poll`

- In linux, `poll` is just a modified version of `select`. It fixed the problem of nfds. It can select fds with bigger number(not limited by 1024).

## About `epoll`

- Support edge-triggered and level-triggered interface.
- Support large numbers of fds.
- Unlike `select` and `poll`, `epoll` manages the fds inside kernel, not in user space. Performance problem caused by copy is thus solved.

## How does kernel decide that a fd has something to do?

### reference

https://www.cnblogs.com/Hijack-you/p/13057792.html

### Begin with recv

Think about what happened of the code next.

```c
recv(fd);
```

If there is no data in the associated buffer, then this call will make the process blocked. And this process will be added to the wait-queue of the socket.

When net-card receives data, it will generate a hardware interrupt request(hard-IRQ). OS handles this IRQ, and data will be transfered to the buffer of fd, the blocking process will be waked(moved to the waked-queue).

### select

The calling process will be added to the waiting queue of all the file descriptors. Each of the fd has data will wake the process, and select just go through all fds to check if they have data.

Performance problem:

- When select calls, add process to all fds's wait-queue. When select ends, this process need to be removed from all fds's wait-queue.
- Bitmap need to transfer between user space and kernel.
- Both user and kernel has to go through the bitmap to check fds's state.

### epoll

Epoll seperate fd-set operation and wait operation, as fd-set does not often change a lot. 

Epoll also introduced ready-list into kernel. One process is waken up, it just need to get the ready-list to know all fds that have data. This avoids going through.

