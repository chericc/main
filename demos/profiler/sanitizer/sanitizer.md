# sanitizer

## docs

https://github.com/google/sanitizers

## classes

AddressSanitizer: detects addressability issues;

LeakSanitizer: detects memory leaks;

ThreadSanitizer: detects data races and dead locks;

MemorySanitizer: detects use of uninitialized memory;

## AddressSanitizer

### asan

Abbreviation: asan

### detects

- Use after free
- Heap buffer overflow
- Stack buffer overflow
- Global buffer overflow
- Use after return
- Use after scope
- Initialization order bugs
- Memory leaks

### performance

~2x;

### technique

#### general

The run-time library replaces the `malloc` and `free` functions. 

Some memory is poisoned and every memory access is transformed in the following way:

```c
*address = ... 
... = *address

--> 

if (is_poisoned(address))
{
    report error;
}
*address = ... 
... = *address
```

#### memory mapping and instrumentation

- Main application memory
- Shadow memory

Poisoning a byte in the main memory means writing some special value into the corresponding shadow memory.

#### run-time library(replaces malloc);

Replaces malloc / free and provides error reporting function.

### steps

```c++
int main()
{
    int *p = new int;
    p = nullptr;
    return 0;
}
```

```bash
g++ test.cpp -fsanitize=address -fno-omit-frame-pointer
```

### options

## LeakSanitizer

### about

LeakSanitizer is integrated into AddressSanitizer.

### stand-alone mode

```bash
-fsanitize=leak
```

This will link your program against a runtime library containing just the bare necessities required for LeakSanitizer to work. No compile-time instrumentation will be applied.

## verbose 

```bash
redzone=16
max_redzone=2048
quarantine_size_mb=4M
malloc_context_size=2
SHADOW_SCALE: 3
SHADOW_GRANULARITY: 8
```

```bash
# print options
ASAN_OPTIONS=help=1:log_path=asan_log ./a.out

# this works
allocator_release_to_os=true
export ASAN_OPTIONS=log_path=asan_log:verbosity=1:quarantine_size_mb=1:malloc_context_size=1:allocator_release_to_os=true:max_redzone=32
export ASAN_OPTIONS=log_path=asan_log:verbosity=1:quarantine_size_mb=1:malloc_context_size=1:max_redzone=32
# 
```

## region control

```c
#if defined(__clang__) || defined (__GNUC__)
# define ATTRIBUTE_NO_SANITIZE_ADDRESS __attribute__((no_sanitize_address))
#else
# define ATTRIBUTE_NO_SANITIZE_ADDRESS
#endif
...
ATTRIBUTE_NO_SANITIZE_ADDRESS
void ThisFunctionWillNotBeInstrumented() {...}
```

### memory usage

```bash
# Disable UAR error detection (reduces code and heap size)
CFLAGS+='-fsanitize-address-use-after-return=never -fno-sanitize-address-use-after-scope'
export ASAN_OPTIONS="$ASAN_OPTIONS:detect_stack_use_after_return=1"

# Disable inline instrumentation (slower but saves code size)
CFLAGS+='-fsanitize-address-outline-instrumentation'

# Reduce heap quarantine (reduces heap consumption but also lowers chance of UAF detection)
export ASAN_OPTIONS="$ASAN_OPTIONS:quarantine_size_mb=16"

# Do not keep full backtrace of malloc origin (slightly complicates debugging but reduces heap size)
export ASAN_OPTIONS="$ASAN_OPTIONS:malloc_context_size=5"

# for implement bugs, can also ulimit -s to tweak memory usage by threads' fake stack memory
ulimit -c 4096;
```

### help

```bash
# Available flags for AddressSanitizer:
        quarantine_size
                - Deprecated, please use quarantine_size_mb.
        quarantine_size_mb
                - Size (in Mb) of quarantine used to detect use-after-free errors. Lower value may reduce memory usage but increase the chance of false negatives.
        redzone
                - Minimal size (in bytes) of redzones around heap objects. Requirement: redzone >= 16, is a power of two.
        max_redzone
                - Maximal size (in bytes) of redzones around heap objects.
        debug
                - If set, prints some debugging information and does additional checks.
        report_globals
                - Controls the way to handle globals (0 - don't detect buffer overflow on globals, 1 - detect buffer overflow, 2 - print data about registered globals).
        check_initialization_order
                - If set, attempts to catch initialization order issues.
        replace_str
                - If set, uses custom wrappers and replacements for libc string functions to find more errors.
        replace_intrin
                - If set, uses custom wrappers for memset/memcpy/memmove intrinsics.
        detect_stack_use_after_return
                - Enables stack-use-after-return checking at run-time.
        min_uar_stack_size_log
                - Minimum fake stack size log.
        max_uar_stack_size_log
                - Maximum fake stack size log.
        uar_noreserve
                - Use mmap with 'noreserve' flag to allocate fake stack.
        max_malloc_fill_size
                - ASan allocator flag. max_malloc_fill_size is the maximal amount of bytes that will be filled with malloc_fill_byte on malloc.
        malloc_fill_byte
                - Value used to fill the newly allocated memory.
        allow_user_poisoning
                - If set, user may manually mark memory regions as poisoned or unpoisoned.
        sleep_before_dying
                - Number of seconds to sleep between printing an error report and terminating the program. Useful for debugging purposes (e.g. when one needs to attach gdb).
        check_malloc_usable_size
                - Allows the users to work around the bug in Nvidia drivers prior to 295.*.
        unmap_shadow_on_exit
                - If set, explicitly unmaps the (huge) shadow at exit.
        protect_shadow_gap
                - If set, mprotect the shadow gap
        print_stats
                - Print various statistics after printing an error message or if atexit=1.
        print_legend
                - Print the legend for the shadow bytes.
        print_scariness
                - Print the scariness score. Experimental.
        atexit
                - If set, prints ASan exit stats even after program terminates successfully.
        print_full_thread_history
                - If set, prints thread creation stacks for the threads involved in the report and their ancestors up to the main thread.
        poison_heap
                - Poison (or not) the heap memory on [de]allocation. Zero value is useful for benchmarking the allocator or instrumentator.
        poison_partial
                - If true, poison partially addressable 8-byte aligned words (default=true). This flag affects heap and global buffers, but not stack buffers.
        poison_array_cookie
                - Poison (or not) the array cookie after operator new[].
        alloc_dealloc_mismatch
                - Report errors on malloc/delete, new/free, new/delete[], etc.
        new_delete_type_mismatch
                - Report errors on mismatch between size of new and delete.
        strict_init_order
                - If true, assume that dynamic initializers can never access globals from other modules, even if the latter are already initialized.
        start_deactivated
                - If true, ASan tweaks a bunch of other flags (quarantine, redzone, heap poisoning) to reduce memory consumption as much as possible, and restores them to original values when the first instrumented module is loaded into the process. This is mainly intended to be used on Android. 
        detect_invalid_pointer_pairs
                - If non-zero, try to detect operations like <, <=, >, >= and - on invalid pointer pairs (e.g. when pointers belong to different objects). The bigger the value the harder we try.
        detect_container_overflow
                - If true, honor the container overflow annotations. See https://github.com/google/sanitizers/wiki/AddressSanitizerContainerOverflow
        detect_odr_violation
                - If >=2, detect violation of One-Definition-Rule (ODR); If ==1, detect ODR-violation only if the two variables have different sizes
        dump_instruction_bytes
                - If true, dump 16 bytes starting at the instruction that caused SEGV
        suppressions
                - Suppressions file name.
        halt_on_error
                - Crash the program after printing the first error report (WARNING: USE AT YOUR OWN RISK!)
        use_odr_indicator
                - Use special ODR indicator symbol for ODR violation detection
        symbolize
                - If set, use the online symbolizer from common sanitizer runtime to turn virtual addresses to file/line locations.
        external_symbolizer_path
                - Path to external symbolizer. If empty, the tool will search $PATH for the symbolizer.
        allow_addr2line
                - If set, allows online symbolizer to run addr2line binary to symbolize stack traces (addr2line will only be used if llvm-symbolizer binary is unavailable.
        strip_path_prefix
                - Strips this prefix from file paths in error reports.
        fast_unwind_on_check
                - If available, use the fast frame-pointer-based unwinder on internal CHECK failures.
        fast_unwind_on_fatal
                - If available, use the fast frame-pointer-based unwinder on fatal errors.
        fast_unwind_on_malloc
                - If available, use the fast frame-pointer-based unwinder on malloc/free.
        handle_ioctl
                - Intercept and handle ioctl requests.
        malloc_context_size
                - Max number of stack frames kept for each allocation/deallocation.
        log_path
                - Write logs to "log_path.pid". The special values are "stdout" and "stderr". The default is "stderr".
        log_exe_name
                - Mention name of executable when reporting error and append executable name to logs (as in "log_path.exe_name.pid").
        log_to_syslog
                - Write all sanitizer output to syslog in addition to other means of logging.
        verbosity
                - Verbosity level (0 - silent, 1 - a bit of output, 2+ - more output).
        detect_leaks
                - Enable memory leak detection.
        leak_check_at_exit
                - Invoke leak checking in an atexit handler. Has no effect if detect_leaks=false, or if __lsan_do_leak_check() is called before the handler has a chance to run.
        allocator_may_return_null
                - If false, the allocator will crash instead of returning 0 on out-of-memory.
        print_summary
                - If false, disable printing error summaries in addition to error reports.
        check_printf
                - Check printf arguments.
        handle_segv
                - If set, registers the tool's custom SIGSEGV/SIGBUS handler.
        handle_abort
                - If set, registers the tool's custom SIGABRT handler.
        handle_sigill
                - If set, registers the tool's custom SIGILL handler.
        handle_sigfpe
                - If set, registers the tool's custom SIGFPE handler.
        allow_user_segv_handler
                - If set, allows user to register a SEGV handler even if the tool registers one.
        use_sigaltstack
                - If set, uses alternate stack for signal handling.
        detect_deadlocks
                - If set, deadlock detection is enabled.
        clear_shadow_mmap_threshold
                - Large shadow regions are zero-filled using mmap(NORESERVE) instead of memset(). This is the threshold size in bytes.
        color
                - Colorize reports: (always|never|auto).
        legacy_pthread_cond
                - Enables support for dynamic libraries linked with libpthread 2.2.5.
        intercept_tls_get_addr
                - Intercept __tls_get_addr.
        help
                - Print the flag descriptions.
        mmap_limit_mb
                - Limit the amount of mmap-ed memory (excluding shadow) in Mb; not a user-facing flag, used mosly for testing the tools
        hard_rss_limit_mb
                - Hard RSS limit in Mb. If non-zero, a background thread is spawned at startup which periodically reads RSS and aborts the process if the limit is reached
        soft_rss_limit_mb
                - Soft RSS limit in Mb. If non-zero, a background thread is spawned at startup which periodically reads RSS. If the limit is reached all subsequent malloc/new calls will fail or return NULL (depending on the value of allocator_may_return_null) until the RSS goes below the soft limit. This limit does not affect memory allocations other than malloc/new.
        heap_profile
                - Experimental heap profiler, asan-only
        allocator_release_to_os
                - Experimental. If true, try to periodically release unused memory to the OS.

        can_use_proc_maps_statm
                - If false, do not attempt to read /proc/maps/statm. Mostly useful for testing sanitizers.
        coverage
                - If set, coverage information will be dumped at program shutdown (if the coverage instrumentation was enabled at compile time).
        coverage_pcs
                - If set (and if 'coverage' is set too), the coverage information will be dumped as a set of PC offsets for every module.
        coverage_order_pcs
                - If true, the PCs will be dumped in the order they've appeared during the execution.
        coverage_bitset
                - If set (and if 'coverage' is set too), the coverage information will also be dumped as a bitset to a separate file.
        coverage_counters
                - If set (and if 'coverage' is set too), the bitmap that corresponds to coverage counters will be dumped.
        coverage_direct
                - If set, coverage information will be dumped directly to a memory mapped file. This way data is not lost even if the process is suddenly killed.
        coverage_dir
                - Target directory for coverage dumps. Defaults to the current directory.
        full_address_space
                - Sanitize complete address space; by default kernel area on 32-bit platforms will not be sanitized
        print_suppressions
                - Print matched suppressions at exit.
        disable_coredump
                - Disable core dumping. By default, disable_coredump=1 on 64-bit to avoid dumping a 16T+ core file. Ignored on OSes that don't dump core by default and for sanitizers that don't reserve lots of virtual memory.
        use_madv_dontdump
                - If set, instructs kernel to not store the (huge) shadow in core file.
        symbolize_inline_frames
                - Print inlined frames in stacktraces. Defaults to true.
        symbolize_vs_style
                - Print file locations in Visual Studio style (e.g:  file(10,42): ...
        dedup_token_length
                - If positive, after printing a stack trace also print a short string token based on this number of frames that will simplify deduplication of the reports. Example: 'DEDUP_TOKEN: foo-bar-main'. Default is 0.
        stack_trace_format
                - Format string used to render stack frames. See sanitizer_stacktrace_printer.h for the format description. Use DEFAULT to get default format.
        no_huge_pages_for_shadow
                - If true, the shadow is not allowed to use huge pages. 
        strict_string_checks
                - If set check that string arguments are properly null-terminated
        intercept_strstr
                - If set, uses custom wrappers for strstr and strcasestr functions to find more errors.
        intercept_strspn
                - If set, uses custom wrappers for strspn and strcspn function to find more errors.
        intercept_strpbrk
                - If set, uses custom wrappers for strpbrk function to find more errors.
        intercept_strlen
                - If set, uses custom wrappers for strlen and strnlen functions to find more errors.
        intercept_strchr
                - If set, uses custom wrappers for strchr, strchrnul, and strrchr functions to find more errors.
        intercept_memcmp
                - If set, uses custom wrappers for memcmp function to find more errors.
        strict_memcmp
                - If true, assume that memcmp(p1, p2, n) always reads n bytes before comparing p1 and p2.
        intercept_memmem
                - If set, uses a wrapper for memmem() to find more errors.
        intercept_intrin
                - If set, uses custom wrappers for memset/memcpy/memmove intrinsics to find more errors.
        intercept_stat
                - If set, uses custom wrappers for *stat functions to find more errors.
        intercept_send
                - If set, uses custom wrappers for send* functions to find more errors.
        decorate_proc_maps
                - If set, decorate sanitizer mappings in /proc/self/maps with user-readable names
        exitcode
                - Override the program exit status if the tool found an error
        abort_on_error
                - If set, the tool calls abort() instead of _exit() after printing the error report.
        suppress_equal_pcs
                - Deduplicate multiple reports for single source location in halt_on_error=false mode (asan only).
        print_cmdline
                - Print command line on crash (asan only).
        html_cov_report
                - Generate html coverage report.
        sancov_path
                - Sancov tool location.
        include
                - read more options from the given file
        include_if_exists
                - read more options from the given file (if it exists)
```