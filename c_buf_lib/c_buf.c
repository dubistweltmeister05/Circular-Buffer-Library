#include "c_buf.h"

static void advance_pointer(cbuf_handle_t buf);
static void retreat_pointer(cbuf_handle_t buf);

/// Pass in a storage buffer and size
/// Returns a circular buffer handle
cbuf_handle_t circular_buf_init(uint8_t *buffer, size_t size)
{
    assert(buffer && size);

    cbuf_handle_t buf = (cbuf_handle_t)malloc(sizeof(circular_buf_t));
    assert(buf);

    // Acquire  a mutex entity
    buf->mtx = CreateMutex(NULL, FALSE, NULL);
    assert(buf->mtx);

    buf->buffer = buffer;
    buf->max = size;

    circular_buf_reset(buf);

    assert(buf);

    return buf;
}

/// Free a circular buffer structure.
/// Does not free data buffer; owner is responsible for that
void circular_buf_free(cbuf_handle_t buf)
{
    assert(buf);
    CloseHandle(buf->mtx);
    free(buf);
}

/// Reset the circular buffer to empty, head == tail
void circular_buf_reset(cbuf_handle_t buf)
{
    assert(buf);
    WaitForSingleObject(buf->mtx, INFINITE);
    buf->head = 0;
    buf->tail = 0;
    buf->full = false;
    ReleaseMutex(buf->mtx);
}

/// Put version 1 continues to add data if the buffer is full
/// Old data is overwritten
void circular_buf_put(cbuf_handle_t buf, uint8_t data)
{
    assert(buf && buf->buffer);
    WaitForSingleObject(buf->mtx, INFINITE);

    buf->buffer[buf->head] = data;
    advance_pointer(buf);

    ReleaseMutex(buf->mtx);
}

/// Put Version 2 rejects new data if the buffer is full
/// Returns 0 on success, -1 if buffer is full
int circular_buf_put2(cbuf_handle_t buf, uint8_t data)
{
    int r = -1;
    assert(buf && buf->buffer);
    WaitForSingleObject(buf->mtx, INFINITE);

    if (!circular_buf_full(buf))
    {
        buf->buffer[buf->head] = data;
        advance_pointer(buf);
        r = 0;
    }

    ReleaseMutex(buf->mtx);
    return r;
}

/// Retrieve a value from the buffer
/// Returns 0 on success, -1 if the buffer is empty
int circular_buf_get(cbuf_handle_t buf, uint8_t *data)
{
    assert(buf && data);
    WaitForSingleObject(buf->mtx, INFINITE);
    int r = -1;

    if (!circular_buf_empty(buf))
    {
        r = 0;
        *data = buf->buffer[buf->tail];
        retreat_pointer(buf);
    }

    ReleaseMutex(buf->mtx);
    return r;
}

/// Returns true if the buffer is empty
bool circular_buf_empty(cbuf_handle_t buf)
{
    assert(buf);
    WaitForSingleObject(buf->mtx, INFINITE);

    bool empty = (!buf->full && (buf->head == buf->tail));

    ReleaseMutex(buf->mtx);
    return empty;
}

/// Returns true if the buffer is full
bool circular_buf_full(cbuf_handle_t buf)
{
    assert(buf);
    WaitForSingleObject(buf->mtx, INFINITE);

    bool full = (buf->full && (buf->head == buf->tail));

    ReleaseMutex(buf->mtx);
    return full;
}

/// Returns the maximum capacity of the buffer
size_t circular_buf_capacity(cbuf_handle_t buf)
{
    assert(buf);

    return buf->max;
}

/// Returns the current number of elements in the buffer
size_t circular_buf_size(cbuf_handle_t buf)
{
    assert(buf);
    WaitForSingleObject(buf->mtx, INFINITE);

    size_t size = buf->max;

    if (!buf->full)
    {
        if (buf->head >= buf->tail)
        {
            size = buf->head - buf->tail;
        }
        else
        {
            size = buf->max + buf->head - buf->tail;
        }
    }

    ReleaseMutex(buf->mtx);
    return size;
}

static void advance_pointer(cbuf_handle_t buf)
{
    assert(buf);

    if (buf->full)
    {
        buf->tail = (buf->tail + 1) % buf->max;
    }
    buf->head = (buf->head + 1) % buf->max;
    buf->full = (buf->head == buf->tail ? 1 : 0);
}

static void retreat_pointer(cbuf_handle_t buf)
{
    assert(buf);

    buf->full = false;
    if (++(buf->tail) == buf->max)
    {
        buf->tail = 0;
    }
}