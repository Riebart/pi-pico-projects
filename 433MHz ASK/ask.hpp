#ifndef ASK_HPP
#define ASK_HPP

#include <stdint.h>
#include <chrono>
#include <thread>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define DIV_PER_BIT 8

typedef uint32_t preamble_t;
#define FRAME_PREAMBLE 0xd31f26e7

// [sum([int(b) for b in list(("%8s"%bin(i)[2:]).replace(' ', '0'))]) for i in range(256)]
static const uint8_t ONES_PER_BYTE[] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 
    3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4,
    3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2,
    2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5,
    3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5,
    5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 1, 2, 2, 3,
    2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4,
    4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 2, 3, 3, 4, 3, 4,
    4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6,
    5, 6, 6, 7, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 4, 5,
    5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};


typedef int32_t ask_len_t;
typedef uint8_t checksum_t;

enum ENCODING {
    UNBALANCED_REPEATED = 0x0,
    BALANCED_REPEATED = 0x1,
    MANCHESTER = 0x2
};

enum PACKET_READ_STAGE {
    PREAMBLE_SCAN,
    PREAMBLE_SCAN_COMPLETE,
    PAYLOAD_LENGTH_READ,
    PAYLOAD_LENGTH_READ_COMPLETE,
    PAYLOAD_READ,
    PAYLOAD_READ_COMPLETE,
    FCS_READ,
    FCS_READ_COMPLETE,
    FCS_CHECK,
    FCS_CHECK_COMPLETE,
    FCS_CHECK_ERROR,
    PACKET_READ_COMPLETE,
    PACKET_OK,
    PACKET_ERROR,
    PACKET_DISCARD
};

static const uint8_t SYMBOLS46[] =
{
    0xd,  0xe,  0x13, 0x15, 0x16, 0x19, 0x1a, 0x1c, 
    0x23, 0x25, 0x26, 0x29, 0x2a, 0x2c, 0x32, 0x34
};

static const uint8_t SYMBOLS64[] = 
{
    240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
    240, 240, 0, 1, 240, 240, 240, 240, 2, 240, 3, 4, 240,
    240, 5, 6, 240, 7, 240, 240, 240, 240, 240, 240, 8, 240,
    9, 10, 240, 240, 11, 12, 240, 13, 240, 240, 240, 240,
    240, 14, 240, 15, 240, 240, 240, 240, 240, 240, 240, 240,
    240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
    240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
    240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
    240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
    240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
    240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
    240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
    240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
    240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
    240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
    240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
    240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
    240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
    240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
    240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
    240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
    240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
    240, 240, 240, 240, 240, 240, 240, 240
};

struct ask_writer_params {
    void(*write)(uint8_t);
    uint32_t us_per_div;
};

struct ask_frame {
    preamble_t preamble;
    ask_len_t payload_byte_count;
    uint8_t* data;
    checksum_t checksum;
};

struct ask_writer {
    struct ask_writer_params params;
    ask_len_t num_bits;
    uint8_t* bit_stream;
    ask_len_t bit_cursor;
    volatile bool channel_ready;
    volatile bool data_ready;
};

struct ask_reader_params {
    uint8_t(*read)();
    void(*datagram_ready)(uint8_t* data, ask_len_t datalen);
    uint32_t us_per_div;
};

struct ask_symbol_read_state {
    uint8_t* output;
    ask_len_t num_symbols;
    //TODO Efficiency gains here of 5 bytes, since we can process
    // each symbol as it completes, instead of needing them all stored
    uint8_t pulses[6];
    ask_len_t num_pulses_read;
};

struct ask_preamble_read_state {
    uint64_t frame_preamble_pulses[sizeof(preamble_t)];
};

struct ask_reader {
    struct ask_reader_params params;
    struct ask_frame frame;
    PACKET_READ_STAGE stage;

    struct ask_preamble_read_state preamble_state;
    struct ask_symbol_read_state symbol_state;
};

struct ask_writer ask_writer_init(struct ask_writer_params params)
{
    struct ask_writer writer;
    
    writer.params = params;
    writer.num_bits = 0;
    writer.bit_stream = NULL;
    writer.bit_cursor = 0;
    writer.channel_ready = true; // Determines whether or not the channel is ready to accept a new packet. Set to false upon receipt, and blocks teh channel while the payload is prepared.
    writer.data_ready = false; // Determines whether or not the channel has all necessary data prepared to begin sending.
    
    return writer;
}

struct ask_reader ask_reader_init(struct ask_reader_params params)
{
    struct ask_reader reader;

    reader.params = params;
    reader.frame = {0,0,0,0};
    for (int i = 0 ; i < sizeof(preamble_t) ; i++)
    {
        reader.preamble_state.frame_preamble_pulses[i] = 0;
    }

    reader.stage = PREAMBLE_SCAN;
    reader.symbol_state = {0,0,{0,0,0,0,0,0},0};

    return reader;
}

ask_len_t ask_encode_bytes(struct ask_writer* writer, uint8_t* bytes_in, ask_len_t numbytes, uint8_t* bits_out, bool use6bitsymbols = true)
{
    ask_len_t bit_cursor = 0;

    for (int b = numbytes - 1 ; b >= 0; b--)
    {
        for (int n = 1; n >= 0 ; n--)
        {
            uint8_t nybble = (bytes_in[b] & (0xf << (n * 4))) >> (n * 4);
            uint8_t bits = (use6bitsymbols ? SYMBOLS46[nybble] : nybble);
            for (int i = (use6bitsymbols ? 6 : 4) - 1; i >= 0 ; i--)
            {
                uint8_t bit = (bits & (1 << i)) >> i;
                for (int d = 0 ; d < DIV_PER_BIT; d++)
                {
                    bits_out[bit_cursor] = bit;
                    bit_cursor++;
                }
            }
        }
    }

    return bit_cursor;
}

uint8_t* ask_encode_frame(struct ask_writer* writer, struct ask_frame* frame, ask_len_t* numbits_out)
{
    // TODO There's an 8x memory overhead here since we're storing
    // each pulse as a uint8_t, and not as a bit in an integer.

    ask_len_t numbits = (
        sizeof(preamble_t) + sizeof(ask_len_t) + sizeof(checksum_t) + 
        frame->payload_byte_count) * 2 * 6 * DIV_PER_BIT;
    
    *numbits_out = numbits;
    uint8_t* bit_stream = (uint8_t*)malloc(sizeof(uint8_t) * numbits);
    
    ask_len_t bit_cursor = 0;

    bit_cursor += ask_encode_bytes(writer,
        (uint8_t*)&frame->preamble, sizeof(preamble_t), bit_stream + bit_cursor, false);
    bit_cursor += ask_encode_bytes(writer,
        (uint8_t*)&frame->payload_byte_count, sizeof(ask_len_t), bit_stream + bit_cursor);
    bit_cursor += ask_encode_bytes(writer,
        frame->data, frame->payload_byte_count, bit_stream + bit_cursor);
    bit_cursor += ask_encode_bytes(writer,
        (uint8_t*)&frame->checksum, sizeof(checksum_t), bit_stream + bit_cursor);

    return bit_stream;
}

checksum_t __ask_fcs_calculate(struct ask_frame* frame)
{
    checksum_t fcs = 0;
    for (int i = 0 ; i < sizeof(preamble_t) ; i++)
    {
        fcs ^= ((uint8_t*)(&frame->preamble))[i];
    }

    for (int i = 0 ; i < sizeof(ask_len_t); i++)
    {
        fcs ^= ((uint8_t*)(&frame->payload_byte_count))[i];
    }

    for (int i = 0 ; i < frame->payload_byte_count; i++)
    {
        fcs ^= frame->data[i];
    }

    return fcs;
}

struct ask_frame ask_encap_payload(struct ask_writer* writer, uint8_t* data, ask_len_t datalen)
{
    struct ask_frame frame;
    frame.preamble = FRAME_PREAMBLE;
    frame.payload_byte_count = datalen;
    frame.data = data;
    frame.checksum = __ask_fcs_calculate(&frame);

    return frame;
}

int32_t ask_write(struct ask_writer* writer, uint8_t* data, ask_len_t datalen, bool async = false)
{
    if (!writer->channel_ready)
    {
        return -1;
    }
    else
    {
        // Precompute the bit stream to send, and store that.
        // Saves time spent in the interrupt handler which needs to be as lean as possible.

        // Mark the channel as not ready, prevents anyone else sending data.
        writer->channel_ready = false;

        struct ask_frame frame = ask_encap_payload(writer, data, datalen);
        writer->bit_stream = ask_encode_frame(writer, &frame, &writer->num_bits);
        writer->bit_cursor = 0;
        writer->data_ready = true;

        if (async)
        {
            return 0;
        }
        else
        {
            // Estimate the time to transmit the packet based on the length
            // and writer parameters.
            uint32_t us_to_transmit = writer->num_bits * writer->params.us_per_div;
            
            while (!writer->channel_ready)
            {
                std::this_thread::sleep_for(std::chrono::microseconds(us_to_transmit / 10));
            }

            return datalen;
        }
    }
}

void ask_writer_callback(struct ask_writer* writer)
{
    if (writer->data_ready)
    {
        uint8_t bit = writer->bit_stream[writer->bit_cursor];
        writer->params.write(bit);
        
        writer->bit_cursor++;

        if (writer->bit_cursor == writer->num_bits)
        {
            writer->data_ready = false;
            writer->num_bits = 0;
            writer->bit_cursor = 0;
            free(writer->bit_stream);
            writer->channel_ready = true;
        }
    }
}

inline void __ask_read_pulse(struct ask_symbol_read_state* state, uint8_t pulse)
{
    state->pulses[state->num_pulses_read / DIV_PER_BIT] += 
        (pulse << (state->num_pulses_read % DIV_PER_BIT));
    state->num_pulses_read++;
}

inline uint8_t __ask_process_symbol(struct ask_symbol_read_state* state)
{
    if (state->num_pulses_read == (6 * DIV_PER_BIT))
    {
        uint8_t symbol = 0;
        for (int i = 0 ; i < 6 ; i++)
        {
            uint8_t bit = (ONES_PER_BYTE[state->pulses[i]] >= 5);
            symbol += (bit << (5-i));
        }
        return SYMBOLS64[symbol];
    }
    else
    {
        // A specific value indicating that insufficient reads have
        // completed to fill the symbol.
        return 0xff;
    }
}

void __ask_symbol_state_reset(struct ask_symbol_read_state* state)
{
    state->num_pulses_read = 0;
    for(int i = 0 ; i < 6; i++)
    {
        state->pulses[i] = 0;
    }
}

void ask_read_symbols(struct ask_reader* reader, uint8_t pulse)
{
    // For each pulse, first read the pulse into the right buffer.
    __ask_read_pulse(&reader->symbol_state, pulse);

    // Then check to see if the buffer is ready for processing.
    // Then process the symbol, as 6-bit or 4-bit,
    uint8_t nybble = __ask_process_symbol(&reader->symbol_state);
    // As long as the nybble is invalid, do not process.
    if (nybble >= 0xf0)
    {
        // The sentinel value for "not yet enough bits" is 0xff
        //
        // If 0xf0 is received:
        // The symbol was complete, but did not decode correctly.
        // At this point, abandon the packet, and go back to scanning
        // for the preamble for the next packet.
        if (nybble == 0xf0)
        {
            reader->params.datagram_ready(NULL, 0);
            // The only dynamic memory we might have allocated is the 
            // frame payload, so free that.
            if (reader->frame.data != NULL)
            {
                free(reader->frame.data);
            }
            // Return the reader back to preamble scanning.
            *reader = ask_reader_init(reader->params);
        }

        return;
    }

    // Then write the nybble to the output
    reader->symbol_state.output[((reader->symbol_state.num_symbols - 1) / 2)] +=
        nybble << 4 * ((reader->symbol_state.num_symbols - 1) % 2);
    reader->symbol_state.num_symbols--;
    __ask_symbol_state_reset(&reader->symbol_state);

    // Loop until the output is full.
    // As long as there are symbols to read, do not continue the logic.
    if (reader->symbol_state.num_symbols > 0)
    {
        return;
    }

    // Then handle stage completion.
    if (reader->stage == PAYLOAD_LENGTH_READ)
    {
        fprintf(stderr, "PAYLOAD_LENGTH %d\n", reader->frame.payload_byte_count);
        reader->stage = PAYLOAD_LENGTH_READ_COMPLETE;
        reader->frame.data = (uint8_t*)calloc(reader->frame.payload_byte_count, sizeof(uint8_t*));
        reader->stage = PAYLOAD_READ;

        __ask_symbol_state_reset(&reader->symbol_state);
        reader->symbol_state.num_symbols = 2 * reader->frame.payload_byte_count;
        reader->symbol_state.output = reader->frame.data;
    }
    else if (reader->stage == PAYLOAD_READ)
    {
        fprintf(stderr, "PAYLOAD %s\n", reader->frame.data);
        reader->stage = PAYLOAD_READ_COMPLETE;
        reader->stage = FCS_READ;

        __ask_symbol_state_reset(&reader->symbol_state);
        reader->symbol_state.num_symbols = 2 * sizeof(checksum_t);
        reader->symbol_state.output = &reader->frame.checksum;
    }
    else if (reader->stage = FCS_READ)
    {
        fprintf(stderr, "FCS %#x\n", reader->frame.checksum);
        reader->stage = FCS_READ_COMPLETE;
    }
}

void hex_print_preamble_buffer(struct ask_reader* reader)
{
    for (int i = sizeof(preamble_t) - 1 ; i >= 0 ; i--)
    {
        fprintf(stderr, "%#0.16lx ", reader->preamble_state.frame_preamble_pulses[i]);
    }
    fprintf(stderr, "\n");
}

preamble_t __ask_pulses_to_bytes(uint8_t* bytes, ask_len_t num_bytes)
{
    uint64_t output = 0;

    for (int i = 0 ; i < 8 * num_bytes ; i++)
    {
        output += ((uint64_t)(ONES_PER_BYTE[bytes[i]] >= 6) << i);
    }

    return output;
}

void ask_read_preamble(struct ask_reader* reader, uint8_t pulse)
{   
    // hex_print_preamble_buffer(reader);
    // Step 1: shuffle the preamble bit buffer left to make room at the
    // LSB to put the new incoming pulse.
    uint8_t overflow = pulse;
    uint8_t incoming = pulse;
    for (int i = 0 ; i < sizeof(preamble_t) ; i++)
    {
        uint64_t v = reader->preamble_state.frame_preamble_pulses[i];
        overflow = v >> 63;
        v <<= 1;
        v += incoming;
        incoming = overflow;
        reader->preamble_state.frame_preamble_pulses[i] = v;
    }

    reader->frame.preamble = __ask_pulses_to_bytes((uint8_t*)reader->preamble_state.frame_preamble_pulses, sizeof(preamble_t));

    // TODO Check to see if adding another bit would improve the
    // XOR autocorrelation with the target value. If so, delay
    // moving to the next stage of processing.
    //
    // Notably, a perfect read will always need 3 more pulses
    // to perfectly synchronize.
    
    if (reader->frame.preamble == FRAME_PREAMBLE)
    {
        hex_print_preamble_buffer(reader);
        reader->stage = PREAMBLE_SCAN_COMPLETE;
    }
}

void __ask_fcs_validate(struct ask_reader reader)
{
    checksum_t computed_fcs = __ask_fcs_calculate(&reader.frame);
    if (computed_fcs == reader.frame.checksum)
    {
        reader.stage = FCS_CHECK_COMPLETE;
        reader.stage = PACKET_OK;
        reader.params.datagram_ready(reader.frame.data, reader.frame.payload_byte_count);
    }
    else
    {
        printf("FCS CHECK FAILURE %u %u\n", computed_fcs, reader.frame.checksum);
        reader.stage = FCS_CHECK_ERROR;
        reader.stage = PACKET_ERROR;
    }
}

void ask_reader_callback(struct ask_reader* reader)
{
    // The idea here is to watch for a preamble by:
    // - Reading in pulses into a pulse buffer equal to sizeof(preamble) bytes long FIFO
    //  > For every bit we read, shuffle everything over 1 pulse
    // - On each pulse, attempt to synchronize by seeing if there's a preamble in the
    //   pulse buffer
    // - On successful detection of a preamble, mark a datagram as incoming, and
    //   begin reading in the data length, data, and checksum.
    // - If a nonsense symbol (e.g., 6 bits that doesn't decode to a valid nybble)
    //   is detected, throw away the remaining pulses entirely (ignore them).
    // - Otherwise, 
    // - And then 

    // Read a pulse
    uint8_t pulse = reader->params.read();
    
    // Reading the preamble is different from reading a symbol, as
    // during the preamble scan, we do not yet have a synchronzied
    // clock with the sender. This requires storing every pulse for the
    // entire set of bytes until it lines up and decodes correctly.
    if (reader->stage == PREAMBLE_SCAN)
    {
        ask_read_preamble(reader, pulse);

        if (reader->stage == PREAMBLE_SCAN_COMPLETE)
        {
            reader->symbol_state.num_symbols = 2 * sizeof(ask_len_t);
            reader->symbol_state.output = (uint8_t*)&reader->frame.payload_byte_count;
            __ask_symbol_state_reset(&reader->symbol_state);
            reader->stage = PAYLOAD_LENGTH_READ;
        }
    }
    // Once the preamble scan is complete, we can move onto synchronized
    // reading of symbols. This continues until the FCS read is complete.
    else if (reader->stage < FCS_READ_COMPLETE)
    {
        ask_read_symbols(reader, pulse);
    }
    else if (reader->stage == FCS_READ_COMPLETE)
    {
        printf("FCS_READ Complete, detaching validation and callback task\n");
        std::thread* t = new std::thread(__ask_fcs_validate, *reader);
        t->detach();
        
        // Return the reader back to preamble scanning.
        *reader = ask_reader_init(reader->params);
    }
}

#endif