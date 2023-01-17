#include <stdio.h>
#include <thread>
#include <random>
#include <stdint.h>
#include <utime.h>
#include <unistd.h>

#include "pico/stdlib.h"

#include "ask.hpp"
#include "repeater.hpp"

#define US_PER_DIV 50

#ifdef PICO_DEFAULT_LED_PIN
#define SLEEP(x) sleep_ms(x)
#else
#define SLEEP(x) std::this_thread::sleep_for(x)
#endif


volatile uint32_t successes = 0;
volatile uint32_t channel = 0;

void bit_writer(uint8_t bit)
{
    channel = bit;
}

uint8_t bit_reader()
{
    return channel;
}

void datagram(uint8_t* data, ask_len_t datalen)
{
    if (data == NULL)
    {
        fprintf(stderr, "INVALID SYMBOL READ, DISCARDING FRAME\n");
    }
    else
    {
        printf("DATAGRAM: %s\n", data);
        free(data);
        successes++;
    }
}

void test()
{
    // std::this_thread::sleep_for(std::chrono::milliseconds(500));
    channel++;
    printf("TEST %u\n", channel);
}

int main(int argc, char** argv)
{
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    while (true) {
        gpio_put(LED_PIN, 1);
        sleep_ms(250);
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
    }
    
    // To construct the writer, we need to know:
    // - the callback to write a bit
    // - The number of microsecons per division, which controls the period of the timer.
    struct ask_writer_params writer_params;
    writer_params.write = &bit_writer;
    writer_params.us_per_div = US_PER_DIV;
    struct ask_writer writer = ask_writer_init(writer_params);

    // There is a manual post-initialization step to add the writer to
    // a strict timer process
    //
    // On a Pico, you'd call add_repeating_timer_us, but here we need
    // to invoke the Repeater class to do this.
    Repeater* rw = new Repeater(writer_params.us_per_div, true, false, &ask_writer_callback, &writer);

    // To construct the reader, we need to have one more parameter than
    // a writer:
    // - The callback to read a bit
    // - The callback when a datagram is ready for processing
    // - The time per division.
    struct ask_reader_params reader_params;
    reader_params.read = &bit_reader;
    reader_params.datagram_ready = &datagram;
    reader_params.us_per_div = US_PER_DIV;
    struct ask_reader reader = ask_reader_init(reader_params);
    Repeater* rr = new Repeater(reader_params.us_per_div, true, false, &ask_reader_callback, &reader);

    uint8_t* data1 = (uint8_t*)"This is a test string\0This is a test string\0This is a test string\0This is a test string\0";
    uint32_t datalen = 22 * 4;
    uint32_t numsent = 0;

    #ifdef PICO_DEFAULT_LED_PIN
    uint16_t inter_send_wait = 10;
    #else
    std::chrono::microseconds inter_send_wait(10);
    #endif;

    for (int i = 0 ; i < 100 ; i++)
    {
        numsent += ask_write(&writer, data1, datalen, false);
        SLEEP(inter_send_wait);
        channel = 0;
        SLEEP(inter_send_wait);
    }

    fprintf(stderr, "%u %u\n", numsent / datalen, successes);

    delete rr;
    delete rw;

    return 0;
}
