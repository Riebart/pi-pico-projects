# This example takes the temperature from the Pico's onboard temperature sensor, and displays it on Pico Display Pack, along with a little pixelly graph.
# It's based on the thermometer example in the "Getting Started with MicroPython on the Raspberry Pi Pico" book, which is a great read if you're a beginner!

import machine
import utime

import st7789
from pimoroni import RGBLED, Button

# Duration, in milliseconds, to fade from the current colour to the new one
FADE_TIME = 5000
LOOP_SLEEP_TIME = 0.1

# User config for colours to use, and when.
COLOUR_CONFIG = {
    0: [((0, 0), (0, 0, 0)), ((7, 15), (0, 128, 0)), ((7, 30), (0, 255, 0)),
        ((8, 0), (0, 0, 255)), ((12, 30), (0, 0, 0)), ((15, 0), (0, 255, 0)),
        ((19, 15), (0, 0, 0))],
    1: [((0, 0), (0, 0, 0)), ((6, 45), (0, 128, 0)), ((6, 55), (0, 255, 0)),
        ((7, 0), (0, 0, 255)), ((12, 30), (0, 0, 0)), ((15, 0), (0, 255, 0)),
        ((19, 15), (0, 0, 0))],
    2: [((0, 0), (0, 0, 0)), ((6, 45), (0, 128, 0)), ((6, 55), (0, 255, 0)),
        ((7, 0), (0, 0, 255)), ((12, 30), (0, 0, 0)), ((15, 0), (0, 255, 0)),
        ((19, 15), (0, 0, 0))],
    3: [((0, 0), (0, 0, 0)), ((6, 45), (0, 128, 0)), ((6, 55), (0, 255, 0)),
        ((7, 0), (0, 0, 255)), ((12, 30), (0, 0, 0)), ((15, 0), (0, 255, 0)),
        ((19, 15), (0, 0, 0))],
    4: [((0, 0), (0, 0, 0)), ((6, 45), (0, 128, 0)), ((6, 55), (0, 255, 0)),
        ((7, 0), (0, 0, 255)), ((12, 30), (0, 0, 0)), ((15, 0), (0, 255, 0)),
        ((19, 15), (0, 0, 0))],
    5: [((0, 0), (0, 0, 0)), ((6, 45), (0, 128, 0)), ((6, 55), (0, 255, 0)),
        ((7, 0), (0, 0, 255)), ((12, 30), (0, 0, 0)), ((15, 0), (0, 0, 255)),
        ((19, 15), (0, 0, 0))],
    6: [((0, 0), (0, 0, 0)), ((7, 15), (0, 128, 0)), ((7, 30), (0, 255, 0)),
        ((8, 0), (0, 0, 255)), ((12, 30), (0, 0, 0)), ((15, 0), (0, 255, 0)),
        ((19, 15), (0, 0, 0))]
}


class BackgroundFader(object):
    def __init__(self, colour_scheme, transition_duration):
        self.colour_scheme = {
            k: sorted([(3600 * i[0][0] + 60 * i[0][1], i[1]) for i in v])
            for k, v in colour_scheme.items()
        }
        self.fade_start_ms = None
        self.transition_duration = transition_duration
        self.transition_duration_sec = transition_duration / 1000

    @staticmethod
    def lerp(ta, tb, t):
        return (int(ta[0] * (1 - t) + t * tb[0]),
                int(ta[1] * (1 - t) + t * tb[1]),
                int(ta[2] * (1 - t) + t * tb[2]))

    def get_bg_colour(self, localtime):
        # Determine which colour profile to use based on
        # day of the week
        dow = day_of_week(utime.mktime(localtime))
        tod = 3600 * localtime[3] + 60 * localtime[4] + localtime[5]
        colours = self.colour_scheme[dow]

        # Determine which colour swatch we're on based on
        # time of day
        colour = None
        for i in range(len(colours)):
            if tod >= colours[i][0]:
                colour_index = i

        colour = colours[colour_index]
        previous_colour = colours[colour_index - 1]

        # If we're within the fade duration, assume we're
        # at the start of the second, and start counting
        # milliseconds
        if self.fade_start_ms is None and tod < colour[
                0] + self.transition_duration_sec:
            self.fade_start_ms = utime.ticks_ms()

        if self.fade_start_ms is not None:
            ms = utime.ticks_diff(utime.ticks_ms(), self.fade_start_ms)

            # If we've completed the transition, then just return the new colour and remove
            # our start transition ms point
            if ms > self.transition_duration:
                self.fade_start_ms = None
                return colour[1]

            # Convert the time of day to milliseconds, and add the subsecond part of the ms
            # counter
            tod_ms = 1000 * tod + (ms % 1000)
            print(tod, ms, tod_ms, 1000 * colour[0], self.transition_duration)
            print(
                self.lerp(previous_colour[1], colour[1],
                          (tod_ms - 1000 * colour[0]) /
                          self.transition_duration))

            return self.lerp(previous_colour[1], colour[1],
                             (tod_ms - 1000 * colour[0]) /
                             self.transition_duration)
        else:
            return colour[1]


SNAPSHOT_TIME_TO_FLASH_INTERVAl = 300

#### CALIBRATE THE CLOCK

# Note the boot time, and then assume that the offset we're going to use from boot is just that
BASELINE_DOW_TS = utime.mktime((2022, 1, 1, 0, 0, 0, 0, 0))
BASELINE_DOW = 6  # 0 = Sunday, 6 = Saturday
BOOT_TIME = utime.time()
TIMESTAMP_OFFSET = BOOT_TIME

# Try to find a coarse timestamp from the flash, say if one was written whlie connected to Thonny
try:
    with open("timestamp", "r") as fp:
        TIMESTAMP_OFFSET = int(fp.read())
    # If we read a timestamp from the flash that's older than now, rewrite and update it
    if TIMESTAMP_OFFSET < BOOT_TIME:
        with open("timestamp", "w") as fp:
            fp.write(str(BOOT_TIME))
except:
    with open("timestamp", "w") as fp:
        fp.write(str(TIMESTAMP_OFFSET))

LAST_TS_UPDATE = TIMESTAMP_OFFSET


def get_time():
    return utime.time() - BOOT_TIME + TIMESTAMP_OFFSET


def day_of_week(time_ts):
    return ((time_ts - BASELINE_DOW_TS) // 86400 + BASELINE_DOW) % 7


def check_buttons():
    global TIMESTAMP_OFFSET
    button_hr_up = Button(12)  # A
    button_hr_down = Button(13)  # B
    button_min_up = Button(14)  # X
    button_min_down = Button(15)  # Y

    if button_hr_down.read():
        TIMESTAMP_OFFSET -= 3600
    if button_hr_up.read():
        TIMESTAMP_OFFSET += 3600
    if button_min_down.read():
        TIMESTAMP_OFFSET -= 60
    if button_min_up.read():
        TIMESTAMP_OFFSET += 60


#### CLOCK CALIBRATED

# Set the display resolution
# in most cases you can swap WIDTH weith HEIGHT for portrait mode
WIDTH, HEIGHT = 320, 240  # Pico Display 2.0
display = st7789.ST7789(WIDTH, HEIGHT, rotate180=False)
display.set_backlight(1.0)
led = RGBLED(6, 7, 8)

# reads from Pico's temp sensor and converts it into a more manageable number
sensor_temp = machine.ADC(4)
conversion_factor = 3.3 / (65535)


def main(bg_fader):
    last_ts_update = TIMESTAMP_OFFSET
    while True:
        time_ts = get_time()
        localtime = utime.localtime(time_ts)
        time_of_day = (localtime[3], localtime[4])

        # If more than the write-out interval has passed since the last time we updated
        # the flash-stored offset, write the calibration TS back out to flash
        if time_ts - last_ts_update >= SNAPSHOT_TIME_TO_FLASH_INTERVAl:
            with open("timestamp", "w") as fp:
                fp.write(str(time_ts))
            last_ts_update = time_ts
            # Pulse the LED to show a write
            # led.set_rgb(255, 255, 255)
            # utime.sleep(0.1)
            # led.set_rgb(0, 0, 0)

        check_buttons()

        # fills the screen with a time-aware background colour
        bg_colour = bg_fader.get_bg_colour(localtime)
        display.set_pen(*bg_colour)
        led.set_rgb(bg_colour[0] / 4, bg_colour[1] / 4, bg_colour[2] / 4)
        backlight_level = min(0.25 + 0.75 * sum(bg_colour) / 255, 1)
        display.set_backlight(backlight_level)
        display.clear()

        # the following two lines do some maths to convert the number from the temp sensor into
        # celsius
        reading = sensor_temp.read_u16() * conversion_factor
        temperature = 27 - (reading - 0.706) / 0.001721

        # Writes the temperature reading as text in the white rectangle
        display.set_pen(64, 0, 0)
        display.text("{:.2f}".format(temperature) + "c", 3, 3, 0, 3)

        display.set_pen(64, 64, 64)
        display.text("%04d-%02d-%02d %02d:%02d:%02d" % (localtime[:6]), 5, 30,
                     320, 3)
        display.set_pen(255, 255, 255)
        display.text("%02d:%02d" % time_of_day, 5, 60, 320, 12)

        # time to update the display
        display.update()

        # Sleep a bit, bit since we'll just polling buttons, we need ot poll often enough that we\
        # capture button presses reliably, so about 10Hz
        utime.sleep(LOOP_SLEEP_TIME)


if __name__ == "__main__":
    bg = BackgroundFader(COLOUR_CONFIG, FADE_TIME)
    del COLOUR_CONFIG
    main(bg)
