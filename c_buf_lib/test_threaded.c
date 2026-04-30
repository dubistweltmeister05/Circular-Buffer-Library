#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include "c_buf.h"

#define BUFFER_SIZE 16
#define NUM_PRODUCERS 4
#define NUM_CONSUMERS 4
#define ITEMS_PER_PRODUCER 1000

static cbuf_handle_t cbuf;
static volatile LONG total_produced = 0;
static volatile LONG total_consumed = 0;

DWORD WINAPI producer_thread(LPVOID param)
{
    int id = (int)(intptr_t)param;
    for (int i = 0; i < ITEMS_PER_PRODUCER; i++)
    {
        uint8_t val = (uint8_t)((id * ITEMS_PER_PRODUCER + i) & 0xFF);
        circular_buf_put(cbuf, val);
        InterlockedIncrement(&total_produced);
    }
    printf("  Producer %d finished\n", id);
    return 0;
}

DWORD WINAPI consumer_thread(LPVOID param)
{
    int id = (int)(intptr_t)param;
    int consumed = 0;
    while (1)
    {
        uint8_t data;
        if (circular_buf_get(cbuf, &data) == 0)
        {
            consumed++;
            InterlockedIncrement(&total_consumed);
        }
        else
        {
            // Buffer empty — check if all producers are done
            if (total_produced >= NUM_PRODUCERS * ITEMS_PER_PRODUCER)
                break;
            // Yield to let producers catch up
            Sleep(0);
        }
    }
    printf("  Consumer %d finished, consumed %d items\n", id, consumed);
    return 0;
}

int main(void)
{
    uint8_t buffer[BUFFER_SIZE];
    cbuf = circular_buf_init(buffer, BUFFER_SIZE);

    printf("=== Thread Safety Test ===\n");
    printf("Buffer size: %d\n", BUFFER_SIZE);
    printf("Producers: %d, each writing %d items\n", NUM_PRODUCERS, ITEMS_PER_PRODUCER);
    printf("Consumers: %d\n\n", NUM_CONSUMERS);

    HANDLE threads[NUM_PRODUCERS + NUM_CONSUMERS];

    // Start consumers first so they're waiting
    for (int i = 0; i < NUM_CONSUMERS; i++)
    {
        threads[NUM_PRODUCERS + i] = CreateThread(
            NULL, 0, consumer_thread, (LPVOID)(intptr_t)i, 0, NULL);
    }

    // Start producers
    for (int i = 0; i < NUM_PRODUCERS; i++)
    {
        threads[i] = CreateThread(
            NULL, 0, producer_thread, (LPVOID)(intptr_t)i, 0, NULL);
    }

    // Wait for all threads to finish
    WaitForMultipleObjects(NUM_PRODUCERS + NUM_CONSUMERS, threads, TRUE, INFINITE);

    // Close thread handles
    for (int i = 0; i < NUM_PRODUCERS + NUM_CONSUMERS; i++)
    {
        CloseHandle(threads[i]);
    }

    printf("\nResults:\n");
    printf("  Total produced: %ld\n", total_produced);
    printf("  Total consumed: %ld\n", total_consumed);
    printf("  Buffer size remaining: %zu\n", circular_buf_size(cbuf));

    // Drain remaining items
    long remaining = 0;
    uint8_t d;
    while (circular_buf_get(cbuf, &d) == 0)
        remaining++;

    printf("  Drained remaining: %ld\n", remaining);
    printf("  Total accounted for: %ld\n", total_consumed + remaining);

    if (total_consumed + remaining == total_produced)
        printf("\n  PASS: All items accounted for!\n");
    else
        printf("\n  FAIL: Item count mismatch!\n");

    circular_buf_free(cbuf);
    return 0;
}
