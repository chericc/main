# kernel malloc

## topic

本文主要讨论以下几个问题：

- 内核主要有哪些内存分配方式？如何查看内核占用的内存？
- 用户内存分配和内核内存分配的区别？
- 是否存在内核内存分配失败但是还有可用内存的情况？

## 内核内存分配方式及查询

内核中内存分配方式有：

1. static
2. kmalloc, kzalloc
3. alloc_pages
4. kmap, kmap_atomic
5. kmem_cache_create, kmem_cache_alloc
6. vmalloc
7. alloc_percpu, this_cpu_ptr
8. stack
9. get_free_page

- **size**: small(use `kmalloc`), large(use `vmalloc`)
- **contiguity**: contiguous(`kmalloc`), non-contiguous(`vmalloc`)
- **context**: sleeping allowed(`GFP_KERNEL`), interrupt context(`GFP_ATOMIC`)

查询方式有：

`cat /proc/slabinfo`

- core kernel data structures
- network-related structures
- filesystem and block layer structures
- device-related caches
- kmalloc-64, kmalloc-128, ...

### 内核内存分配和用户空间内存分配的外部区别

从 `/proc/meminfo` 来看，部分项和内核内存分配有关，部分项和用户空间内存分配有关，部分项则是两者皆有关的。

内核相关的项：

- `Buffers`: Memory used by the kernel fo rblock I/O buffers, such as data to/from storage devices.
- `Cached`: Memory used by the kernel to cache files for faster access. This is user-accessible but technically managed by the kernel.
- `Slab`: Memory used by the kernel's slab allocator for managing kernel objects.
- `SReclaimable`: Slab memory that can be reclaimed(e.g., if memory is low).
- `SUnreclaim`: Slab memory that cannot be reclaimed.

用户空间相关的项：

- `MemTotal`: Total physical memory in the system.
- `MemFree`: Memory currently available and not used by the kernel or applications.
- `Active`: Memory actively used by applications(recently accessed).
- `Inactive`: Memory not recently used but still allcated(e.g., cached program data).
- `AnonPages`: Memory used by anonymous(non-file-backed) pages, like heap and stack memory.
- `Mapped`: Memory mapped into process addresss spaces(e.g., shared libraries).
- `Shmem`: Memory used for shared memory segments.

### 内核分配和用户分配的一些细节上的区别

- `kmalloc`分配的物理内存要求有连续空间，因此如果出现了碎片，则分配大的物理内存会出现失败，即使是由大量剩余内存的情况。
- 内存区域：内核、用户空间的物理内存地址可能被限制到特定的地址段，如果地址段被用完了，也会出现失败；
- 连续物理内存分配失败的问题可以通过 `/proc/buddyinfo` 来调试；

### `/proc/buddyinfo`

对应于内核中的 **buddy memory allocator**，用于管理系统中的可用物理内存；

`min_free_kbytes` parameters can help to reserve memory. It specifies the minimum amount of free memory that the system tries to maintain. This prevents critical memory zones from running out of memory. A higher value reserves more memory for the buddy system, reducing fragmentation but potentially limiting available memory for use processes.

The slab allocator works with the buddy system to allocate memory for kernel objects.

### slab

Modern Linux systems support multiple slab allocators:

- `slab`
- `slub`(default in most distros)
- `slob`(optimized for low memory systems)