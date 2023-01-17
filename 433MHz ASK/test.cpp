#include <stdio.h>
#include <thread>
#include <random>
#include <stdint.h>
#include <atomic>

float READ_NOISE_RATIO = 0.0;
int32_t swap_write_count = 0;
int32_t swap_read_count = 0;
int32_t swap_read_attempts = 0;

// The issue with syscall sleep is that there was always
// about a 70us minimum extra sleep duration.
// usleep(10);
// std::this_thread::sleep_for(sleep_duration);

// Use the custom spin_sleep_for for precision sleeping at
// us down to 10ns accuracy.
#define SLEEP_FUNC spin_sleep_for // std::this_thread::sleep_for

struct args
{
    uint32_t write_sleep_ns;
    uint32_t read_sleep_ns;
    volatile int32_t *channel;
};

void spin_sleep_for(std::chrono::nanoseconds dura)
{
    // auto start = std::chrono::high_resolution_clock::now();
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start;
    auto elapsed = end - start;

    while (elapsed < dura)
    {
        end = std::chrono::high_resolution_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    }
    if (elapsed > (1.5 * dura))
    {
        printf("%d\n", elapsed);
    }
}

void writer(void *args_v)
{
    struct args *args = (struct args *)args_v;
    std::chrono::nanoseconds sleep_duration(args->write_sleep_ns);

    while (true)
    {
        swap_write_count++;
        (*args->channel)++;

        SLEEP_FUNC(sleep_duration);
    }
}

void reader(void *args_v)
{
    struct args *args = (struct args *)args_v;
    std::chrono::nanoseconds sleep_duration(args->read_sleep_ns);
    int32_t last_val = *(args->channel);
    int32_t curr_val = *(args->channel);

    while (true)
    {
        swap_read_attempts++;
        curr_val = *(args->channel);

        // Check to see if this bit read is subject to noise
        if (((1.0f + std::rand()) / RAND_MAX) < READ_NOISE_RATIO)
        {
            curr_val = std::rand();
        }

        if (curr_val == (last_val + 1))
        {
            swap_read_count++;
        }

        last_val = curr_val;

        // SLEEP_FUNC(sleep_duration);
    }
}

int main(int argc, char **argv)
{
    volatile int32_t channel = 0;
    struct args t_args = {100000, 1, &channel};

    std::thread read_thread = std::thread(reader, &t_args);
    std::thread write_thread = std::thread(writer, &t_args);
    std::chrono::milliseconds print_sleep(100);
    int32_t test = 0;

    while (true)
    {
        printf("%d, %d, %d, %d, %0.2f%%, %0.2fus, %0.2fus\n",
               swap_read_attempts,
               swap_read_count,
               swap_write_count,
               swap_write_count - swap_read_count,
               100.0 * (1.0 - (1.0 * swap_read_count) / swap_write_count),
               1000000 * 0.1 * test / swap_write_count,
               1000000 * 0.1 * test / swap_read_attempts);
        test++;
        std::this_thread::sleep_for(print_sleep);
    }
}
