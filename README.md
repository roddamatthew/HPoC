# Heaping Pile of Crap (HPoC)

Implementing dynamic memory allocator for learning.
- Below are notes building from a simple allocator to something resembling the `glibc` heap.
- Each version of the heap is within its own directory:
    - Can build each with `make`
    - Implementations increasing in complexity are:
        - [bump](bump/)
        - [free-list](free-list/)
        - [coal](coal/)

# Notes:

## Allocating Memory
- When implementing the heap we want to be able to allocate large regions of contiguous memory.
    - The memory being contiguous is important for consolidation
    - We want it to be large to support big allocations
    - We want to be able to extend our region later
- There's two reasonable ways to do this:
    - `brk()`
    - `mmap()`

## `brk()`:
- The heap starts at a given (random) address
- The program break is above the heap, and marks the end of heap memory.
- We can extend the heap by calling `brk()` or `sbrk()`.
    - This gives us a new region of contigous uninitialized memory.
    - This is the default implementation of the `main_arena` in `glibc`.

## `mmap()`:
- `mmap()` allows a user to create a new virtual memory mapping
- The user can define permissions (RWX)
- The user can also define important properties:
    - `MAP_PRIVATE`: Is this memory private to this process
    - `MAP_SHARED`: Is this memory associated with a shared file
    - `MAP_ANONYMOUS`: This memory isn't backed by a file and is instead initialized as read only.
        - Isn't that slow?! We have to zero our memory on first allocation
        - I don't think so! Since the memory is `MAP_PRIVATE` it is copy-on-write.

## Private (copy-on-write) memory:
- Property of virtual memory pages
- Allows the kernel to back all virtual memory pages with the same contents by the same physical page:
    - Also called memory deduplication
- While we're only reading a page this is fine
- Once we try to write to a deduplicated page now we have an issue; the shared physical page would be corrupted to any other process being shared with.
- So, upon attempting to write to this page, the kernel must copy it to a new physical page.

## chunks
- The fundamental unit of the heap is a chunk
- A chunk is what stores your variable length data
    - It also stores some necessary metadata to manage the heap

```C
typedef struct {
    uint64_t prev_size;
    uint64_t size : 61;
    uint64_t NON_MAIN_ARENA : 1;
    uint64_t IS_MMAPED : 1;
    uint64_t PREV_INUSE : 1;
    // your data ....
}chunk;
```

- Every malloc call returns a chunk, where this header data immediately precedes the pointer returned:
    - The header is intentionally small
    - Think of some of the fields you might add if you were more concerned with security than performance?
    - `isfree`? `checksum`?

## Bump allocator: the `hello world` of heaps:
- Perhaps the simplest implementation of the heap you could imagine is the following:
    - You have some contiguous memory with a pointer to the end of it, `end_ptr`
    - Every allocation is made at `end_ptr`
    - `end_ptr` is then incremented by the size of the chunk that was allocated.
    - If `end_ptr > end_of_heap`, allocate more heap memory.
    - Frees do nothing

## `free`
- We want to allow `free`'d memory to be reused.
- Imagine we have our bump allocator:
    - When free is called, put this chunk in some list
    - When we next allocate, first check the free list
    - If we find a chunk of sufficient size, use that
    - Otherwise we eventually hit our end pointer and allocate there.
- Strictly better than the bump allocator

## Defragmentation/Consolidation/Coalescing:
- Imagine we now have our free-list implementation
- One failure case is that we free many small neighboring chunks
- We then make a large allocation, which could've fit in those unused chunks
- But individually it seems that these chunks were too small
- So we allocated at our end pointer
- We could combine these free chunks to consolidate the heap.
    - In general we want to optimize for the largest contiguous free regions.
    - Can you think of reasons why you wouldn't want to move allocated memory? To consolidate even further.
        - This would be slow (deep copy)
        - The user manages allocated pointers. (Would also require storing some kind of lookup to abstract away actual memory locations.)

## Consolidation:
- When is a good time to consolidate?
- Just before allocation
    - Doing it on free might be wasteful if we free a bunch of memory consecutively.
- Algorithm with our free-list is pretty simple:
    - Iterate through the free-list
    - Peek at the next chunk
    - If we're both free, consolidate:
        - Extend current chunk by `fd.size`
        - Update pointers
- Now we have an entirely respectable heap implementation.

## Performance improvements:
- We now delve more into the `glibc` implementation of the heap
- In my opinion, the goal of this implementation is to minimise the number of O(n) traversals of the free list.
- This is done by implementing a number of smaller caches which are preferentially freed into and allocated out of, since they're O(1)-O(log(n)).

## Thread safety:
- Modern `libc` implementations of the heap are usually thread safe.
- Recall that threads share the heap (memory region).
- Allocations and frees require mutex's for this to be achieved without race conditions.
- If you have a multithreaded program, this has massive performance implications.

## Arenas
- To allow thread safety without being too slow, typically many "heaps" are created, each called an `arena`.
- Careful with the terminology here:
    - A `arena` stores one or more `heaps` which store many `chunks`
- The "heap" that we often think about is stored within the `main_arena`:
    - Extended with `brk()`
    - Contains only one heap
    - Associated with the main thread
- Each additional thread may also have a `thread_arena`:
    - Usually a maximum of 8 times the number of CPUs arenas total
    - These are allocated with `mmap()`
    - Can contain many heaps.
- The intuition of this is that usually your main thread would setup some shared data that different threads might act on.
- Threads you spin up later will probably just reference their own data so that can be made faster.

## Thread ~~safety~~ speedy!
- Threads also have their own small free-list with some predetermined sizes.
- Implemented with thread-local storage.
- The intuition is that threads might make many small allocations that could block eachother.
    - So tradeoff some memory efficiency for speed.
- This free-list is called the `tcache`:
    - Stores up to 7 chunks of sizes between 0x20 to 0x410
    - Stored as an array of singley-linked lists
    - Chunks in the tcache are never consolidated (always set PREV_INUSE)

## fastbin

## smallbins

## largebins

## unsortedbin