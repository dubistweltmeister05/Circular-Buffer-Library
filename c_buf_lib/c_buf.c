#include "c_buf.h"


/// Pass in a storage buffer and size 
/// Returns a circular buffer handle
cbuf_handle_t circular_buf_init(uint8_t* buffer, size_t size){
    assert(bufer && size);

    circular_buf_t *buffer =circular_buf_t * malloc(sizeof(circular_buf_t));
    assert(buffer);

    buffer->buffer = buffer;
    buffer->max = size;

    circular_buffer_reset(buffer);

    assert(bufffer);
}

/// Free a circular buffer structure.
/// Does not free data buffer; owner is responsible for that
void circular_buf_free(cbuf_handle_t me){

}

/// Reset the circular buffer to empty, head == tail
void circular_buf_reset(cbuf_handle_t me){

}

/// Put version 1 continues to add data if the buffer is full
/// Old data is overwritten
void circular_buf_put(cbuf_handle_t me, uint8_t data){

}

/// Put Version 2 rejects new data if the buffer is full
/// Returns 0 on success, -1 if buffer is full
int circular_buf_put2(cbuf_handle_t me, uint8_t data){

}

/// Retrieve a value from the buffer
/// Returns 0 on success, -1 if the buffer is empty
int circular_buf_get(cbuf_handle_t me, uint8_t * data){

}

/// Returns true if the buffer is empty
bool circular_buf_empty(cbuf_handle_t me){

}

/// Returns true if the buffer is full
bool circular_buf_full(cbuf_handle_t me){

}

/// Returns the maximum capacity of the buffer
size_t circular_buf_capacity(cbuf_handle_t me){

}

/// Returns the current number of elements in the buffer
size_t circular_buf_size(cbuf_handle_t me){

}