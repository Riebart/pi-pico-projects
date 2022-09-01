import math
import utime
import picokeypad as keypad
from random import randint

keypad.init()
keypad.set_brightness(1.0)

FADE_TIME = 2500  # Time in milliseconds to fade the button back to 0 over.

COLOURS = ((255, 0, 0), (255, 38, 0), (255, 77, 0), (255, 115, 0),
           (255, 153, 0), (255, 191, 0), (255, 230, 0), (242, 255, 0),
           (204, 255, 0), (166, 255, 0), (128, 255, 0), (89, 255, 0),
           (51, 255, 0), (13, 255, 0), (0, 255, 26), (0, 255, 64),
           (0, 255, 102), (0, 255, 140), (0, 255, 179), (0, 255, 217),
           (0, 255, 255), (0, 217, 255), (0, 178, 255), (0, 140, 255),
           (0, 102, 255), (0, 64, 255), (0, 25, 255), (13, 0, 255),
           (51, 0, 255), (89, 0, 255), (128, 0, 255), (166, 0, 255),
           (204, 0, 255), (242, 0, 255), (255, 0, 229), (255, 0, 191),
           (255, 0, 153), (255, 0, 115), (255, 0, 76), (255, 0, 38), (255, 0,
                                                                      0))


def uint16_to_bits(i):
    return tuple((i >> m) & 1 for m in range(16))


def main():
    num_buttons = keypad.get_num_pads()

    last_button_states = uint16_to_bits(keypad.get_button_states())
    colour_states = [(0, (0, 0, 0)) for _ in range(num_buttons)]
    num_presses = 0

    while True:
        # t0 = utime.ticks_ms()

        button_states = uint16_to_bits(keypad.get_button_states(
        ))  # 16-bit integer indicating which button is pressed

        # For any button pressed, set it to have a new colour and refresh the press time
        for button_id in range(num_buttons):
            button = button_states[button_id]

            if button == 1:
                if button != last_button_states[button_id]:
                    colour_states[button_id] = (utime.ticks_ms(),
                                                COLOURS[num_presses %
                                                        len(COLOURS)])
                    num_presses += 1
                else:
                    colour_states[button_id] = (utime.ticks_ms(),
                                                colour_states[button_id][1])

        for button_id in range(num_buttons):
            colour = colour_states[button_id][1]
            last_press_time = colour_states[button_id][0]
            dt = utime.ticks_diff(utime.ticks_ms(), last_press_time)
            t = min(1, max(0, 1 - dt / FADE_TIME))
            t = t * t * t
            keypad.illuminate(button_id, int(colour[0] * t),
                              int(colour[1] * t), int(colour[2] * t))

        last_button_states = button_states
        keypad.update()
        # dt = utime.ticks_diff(utime.ticks_ms(), t0)
        # print(dt)


if __name__ == "__main__":
    main()
