#include <stdio.h>
#include <stdint.h>
#include "c_buf.h"

#define BUFFER_SIZE 10

static void print_buf_status(cbuf_handle_t cbuf)
{
    printf("  Full: %d, Empty: %d, Size: %zu, Capacity: %zu\n",
           circular_buf_full(cbuf),
           circular_buf_empty(cbuf),
           circular_buf_size(cbuf),
           circular_buf_capacity(cbuf));
}

int main(void)
{
    uint8_t buffer[BUFFER_SIZE];
    cbuf_handle_t cbuf = circular_buf_init(buffer, BUFFER_SIZE);

    printf("=== Circular Buffer Library Test ===\n\n");

    // Test 1: Initial state
    printf("Test 1: Initial state after init\n");
    print_buf_status(cbuf);
    printf("  Expected: Full=0, Empty=1, Size=0, Capacity=10\n\n");

    // Test 2: Put some data (version 1 - overwrite mode)
    printf("Test 2: Adding 5 items with circular_buf_put\n");
    for (uint8_t i = 0; i < 5; i++)
    {
        circular_buf_put(cbuf, i * 10);
        printf("  Put: %d\n", i * 10);
    }
    print_buf_status(cbuf);
    printf("  Expected: Full=0, Empty=0, Size=5, Capacity=10\n\n");

    // Test 3: Read data back
    printf("Test 3: Reading 3 items with circular_buf_get\n");
    for (int i = 0; i < 3; i++)
    {
        uint8_t data;
        int ret = circular_buf_get(cbuf, &data);
        printf("  Get: %d (ret=%d)\n", data, ret);
    }
    print_buf_status(cbuf);
    printf("  Expected: Full=0, Empty=0, Size=2, Capacity=10\n\n");

    // Test 4: Fill the buffer completely
    printf("Test 4: Filling buffer to capacity\n");
    circular_buf_reset(cbuf);
    for (uint8_t i = 0; i < BUFFER_SIZE; i++)
    {
        circular_buf_put(cbuf, i + 100);
    }
    print_buf_status(cbuf);
    printf("  Expected: Full=1, Empty=0, Size=10, Capacity=10\n\n");

    // Test 5: Overwrite behavior (put version 1)
    printf("Test 5: Overwrite when full (put version 1)\n");
    printf("  Adding value 255 to a full buffer...\n");
    circular_buf_put(cbuf, 255);
    print_buf_status(cbuf);
    printf("  Buffer should still be full, oldest item overwritten\n");
    uint8_t data;
    circular_buf_get(cbuf, &data);
    printf("  First item now: %d (expected 101, since 100 was overwritten)\n\n", data);

    // Test 6: Put version 2 - reject when full
    printf("Test 6: Reject when full (put version 2)\n");
    circular_buf_reset(cbuf);
    for (uint8_t i = 0; i < BUFFER_SIZE; i++)
    {
        circular_buf_put(cbuf, i);
    }
    int ret = circular_buf_put2(cbuf, 99);
    printf("  Attempted put2 on full buffer, ret=%d (expected -1)\n", ret);
    print_buf_status(cbuf);
    printf("\n");

    // Test 7: Get from empty buffer
    printf("Test 7: Get from empty buffer\n");
    circular_buf_reset(cbuf);
    uint8_t val = 0xFF;
    ret = circular_buf_get(cbuf, &val);
    printf("  Get from empty: ret=%d (expected -1), val unchanged=%d\n", ret, val);
    print_buf_status(cbuf);
    printf("\n");

    // Test 8: Reset
    printf("Test 8: Reset buffer\n");
    circular_buf_put(cbuf, 42);
    circular_buf_put(cbuf, 43);
    printf("  Before reset:\n");
    print_buf_status(cbuf);
    circular_buf_reset(cbuf);
    printf("  After reset:\n");
    print_buf_status(cbuf);
    printf("  Expected: Full=0, Empty=1, Size=0\n\n");

    // Test 9: Wraparound behavior
    printf("Test 9: Wraparound behavior\n");
    circular_buf_reset(cbuf);
    // Fill halfway, drain, then fill past the end
    for (uint8_t i = 0; i < 7; i++)
        circular_buf_put(cbuf, i);
    for (int i = 0; i < 5; i++)
    {
        uint8_t d;
        circular_buf_get(cbuf, &d);
    }
    // Now head is at 7, tail is at 5, add more to wrap
    for (uint8_t i = 20; i < 28; i++)
        circular_buf_put(cbuf, i);
    printf("  After wraparound fill:\n");
    print_buf_status(cbuf);
    printf("  Reading all data: ");
    while (!circular_buf_empty(cbuf))
    {
        uint8_t d;
        circular_buf_get(cbuf, &d);
        printf("%d ", d);
    }
    printf("\n\n");

    // Cleanup
    circular_buf_free(cbuf);
    printf("=== All tests complete ===\n");

    return 0;
}
