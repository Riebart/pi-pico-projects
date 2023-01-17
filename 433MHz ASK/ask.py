#!/usr/bin/env python3
"""
Implement an ASK communication channel over 4 lanes.

This is intended to be used with 433MHz coded transmitters/receiver pairs,
for example the TX118SA-4 which uses an EV1527 rolling code generator, so the
bitrate is low, but the error types are omission-only. These have an undesirable
characteristic that a high pulse has a minimum duration of about 200ms, resulting
in a frequency for transmission in the single-digit Hz.

There are other undesirable characteristics of these coding transceivers in that
the duration of a highpulse is often (enough) longer than expected by a significant
amount (20-40ms, or 10%), making timing-based approaches infeasible.

Each lane should support a .value() call for reading, and a .on() and .off()
for writing (as micropython machine.Pin() objects do)

The approach is to use the 4 lanes to read symbols across 3 data lanes and
one sync lane. The sync lane is used to mark the start and end of a packet,
and since the duration of a high pulse is so long, no synchronization is needed
inside of even very long packets.

At the start of a payload, the sync lane is pulled high, and then low, marking
the end of the sync pulse, and start of data.

To read data, we use all data lanes and the data is encoded as a high pulse with
a short blank between on each of the data lanes. This allows one trit per ~300ms,
or about 5.28 bits per second.

The end of the data frame is marked by a pulse on the sync lane, at which point
the trits are encoded as bits

NOTE: The assumption here is that the receiver is in multi-channel-lock/latching
mode, so that the sync lane can be held high for the duration of a frame.
"""

import machine
import utime


class ASK4ChannelSimple(object):
    HYSTERESIS_MS = 110

    def __init__(self,
                 start_lane,
                 zero_lane,
                 one_lane,
                 end_lane,
                 write_ms=50,
                 recovery_ms=250):
        self.start_lane = start_lane
        self.data_lanes = (zero_lane, one_lane)
        self.end_lane = end_lane
        self.write_ms = write_ms
        self.recovery_ms = recovery_ms

    def __toggle_lane(self, lane):
        lane.toggle()
        utime.sleep_ms(self.write_ms)
        lane.toggle()
        utime.sleep_ms(self.recovery_ms)

    def __write_byte(self, byte):
        # Write the byte by writing the bits LSB first
        for i in range(8):
            self.__toggle_lane(self.data_lanes[(byte >> i) & 1])

    def write_bytes(self, data):
        self.__toggle_lane(self.start_lane)
        for byte in data:
            self.__write_byte(byte)
        self.__toggle_lane(self.end_lane)

    def __sync_lane(self, lane, timeout_ms):
        """
        Wait timeout_ms for a pulse on the specified lane, and if it arrives,
        wait until it ends.

        If no pulse is found in timeout_ms, then return False, otherwise
        return True and control will be closely synchronized with the
        falling edge of the pulse.
        """
        ret = machine.time_pulse_us(lane, 1, 1000 * timeout_ms)

        if ret == -2:  # Timed out waiting for the pulse
            return False
        elif ret == -1:
            machine.time_pulse_us(lane, 1, 1000 * self.recovery_ms)

        return True

    def __read_bit(self):
        """
        Read a bit from one of the two data lanes.

        Control is expected to be here very shortly after the rising edge
        of a bit on one of the two data lanes. This is due to the start sync
        placing control within __read_bytes() near the falling edge of the
        start pulse, at which point the end-pulse search is immediately called.

        The end-pulse search works by eating the inter-pulse duration (~90ms)
        and then some, placing the control comfortably inside the next pulse.
        """
        bit = None
        for i in range(len(self.data_lanes)):
            if self.data_lanes[i].value() == 1:
                bit = i
                machine.time_pulse_us(self.data_lanes[i], 1,
                                      1000 * self.recovery_ms)
                break
        return bit

    def __read_byte(self):
        byte = 0
        for i in range(8):
            bit = self.__read_bit()
            if bit is None:
                return None
            byte += (bit << i)
            utime.sleep_ms(ASK4ChannelSimple.HYSTERESIS_MS)
        return byte

    def __read_bytes(self):
        end_lane_value = self.__sync_lane(self.end_lane,
                                          ASK4ChannelSimple.HYSTERESIS_MS)

        ret = list()
        while not end_lane_value:
            # Read a byte
            b = self.__read_byte()
            if b is None:
                return None
            ret.append(b)

            # Check for the end of frame signal by waiting
            # at least the 90ms that will elapse between the end of the
            # previous pulse, and the end-of-frame pulse.
            end_lane_value = self.__sync_lane(self.end_lane,
                                              ASK4ChannelSimple.HYSTERESIS_MS)

        return ret

    def read_frame(self, frame_start_timeout_ms=10000):
        sync_result = self.__sync_lane(self.start_lane, frame_start_timeout_ms)

        if not sync_result:
            return None
        ret = self.__read_bytes()
        return ret if ret is not None else []
