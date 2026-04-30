# Circular Buffer Library (Thread-Safe)

A lightweight, thread-safe circular buffer implementation in C — built for embedded systems and real-time producers/consumers.

```
    ┌───┬───┬───┬───┬───┬───┬───┬───┐
    │ 7 │ 0 │ 1 │ 2 │ 3 │ 4 │ 5 │ 6 │
    └───┴───┴───┴───┴───┴───┴───┴───┘
      ▲                           ▲
     HEAD                        TAIL
     (write)                    (read)
```

## Origin

The V1 implementation is based on the excellent [Embedded Artistry article](https://embeddedartistry.com/blog/2017/05/17/creating-a-circular-buffer-in-c-and-c/) by Phillip Johnston. This version extends it with **mutex-based thread safety** using the Windows API, making it safe for concurrent multi-producer/multi-consumer workloads.

## Features

- **Opaque handle API** — clean separation between interface and internals
- **Two write modes:**
  - `circular_buf_put` — overwrites oldest data when full (lossy)
  - `circular_buf_put2` — rejects writes when full (lossless)
- **Thread-safe** — every read/write/status operation is protected by a Windows mutex
- **Zero-copy design** — user owns the backing buffer, library manages the pointers
- **Minimal footprint** — no dynamic allocation for the data buffer itself

## API Reference

| Function | Description |
|----------|-------------|
| `circular_buf_init(buffer, size)` | Initialize buffer, returns handle |
| `circular_buf_free(handle)` | Destroy handle (does **not** free backing buffer) |
| `circular_buf_reset(handle)` | Reset to empty state |
| `circular_buf_put(handle, data)` | Write byte, overwrite if full |
| `circular_buf_put2(handle, data)` | Write byte, reject if full (returns `-1`) |
| `circular_buf_get(handle, &data)` | Read byte (returns `-1` if empty) |
| `circular_buf_empty(handle)` | Check if empty |
| `circular_buf_full(handle)` | Check if full |
| `circular_buf_capacity(handle)` | Max capacity |
| `circular_buf_size(handle)` | Current number of elements |

## Quick Start

```c
#include "c_buf.h"

#define BUF_SIZE 64

int main(void)
{
    uint8_t backing_store[BUF_SIZE];
    cbuf_handle_t cbuf = circular_buf_init(backing_store, BUF_SIZE);

    // Write
    circular_buf_put(cbuf, 0xAA);
    circular_buf_put(cbuf, 0xBB);

    // Read
    uint8_t val;
    circular_buf_get(cbuf, &val); // val = 0xAA

    circular_buf_free(cbuf);
    return 0;
}
```

## Building

Compile with any C compiler on Windows (requires `windows.h` for mutex support):

```bash
# Basic test
gcc -o test main.c c_buf.c -Wall -Wextra

# Threaded stress test
gcc -o test_threaded test_threaded.c c_buf.c -Wall -Wextra
```

## Thread Safety

Every public API function acquires the buffer's mutex before accessing shared state and releases it on exit. This allows safe concurrent access from multiple threads without external synchronization:

```c
// Safe to call from any thread simultaneously
circular_buf_put(cbuf, sensor_reading);   // Producer thread
circular_buf_get(cbuf, &command);          // Consumer thread
```

The included `test_threaded.c` hammers the buffer with **4 producers and 4 consumers** running concurrently, pushing 4000 items through a 16-byte buffer — verifying zero data loss.

```
=== Thread Safety Test ===
Buffer size: 16
Producers: 4, each writing 1000 items
Consumers: 4

  PASS: All items accounted for!
```

## Project Structure

```
c_buf_lib/
├── c_buf.h            — Public API header
├── c_buf.c            — Implementation (thread-safe)
├── main.c             — Functional test suite
└── test_threaded.c    — Multi-threaded stress test
```

## License

Do whatever you want with it.
