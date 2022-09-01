import machine
import sdcard

READ_BENCH_DATA_SIZE = 65536
WRITE_BENCH_TOTAL_SIZE = 1048576
WRITE_BENCH_BLOCK_SIZE = 4096

def write_bench():
    print("Write benchmark")
    t0 = utime.ticks_ms()
    total_len = WRITE_BENCH_TOTAL_SIZE
    with open("/sd/wb","wb") as fp:
        data = bytes([random.randint(0,255) for _ in range(WRITE_BENCH_BLOCK_SIZE)])
        while total_len > 0:
            _ = fp.write(data)
            total_len -= WRITE_BENCH_BLOCK_SIZE
    t1 = utime.ticks_ms()
    dt = utime.ticks_diff(t1, t0)
    print("Wrote %d bytes in %d ms, %f b/s" % (WRITE_BENCH_TOTAL_SIZE, dt, WRITE_BENCH_TOTAL_SIZE * 1000.0 / dt))



def read_bench():
    #import hashlib
    #h = hashlib.sha256()
    print("Read benchmark")
    t0 = utime.ticks_ms()
    total_len = 0
    with open("/sd/Geekbench 5/cpuidsdk64.dll","rb") as fp:
        total_len = 0
        data = fp.read(READ_BENCH_DATA_SIZE)
        while len(data) == READ_BENCH_DATA_SIZE:
            #h.update(data)
            total_len += len(data)
            data = fp.read(READ_BENCH_DATA_SIZE)
        #h.update(data)
        total_len += len(data)
    t1 = utime.ticks_ms()
    dt = utime.ticks_diff(t1, t0)
    print("Read %d bytes in %d ms, %f b/s" % (total_len, dt, total_len * 1000.0 / dt))
    #print(h.digest())
        
# Assign chip select (CS) pin (and start it high)
cs = machine.Pin(17, machine.Pin.OUT)

# Initialize SPI
spi = machine.SPI(0,
                  baudrate=50000000,
                  polarity=0,
                  phase=0,
                  bits=8,
                  firstbit=machine.SPI.MSB,
                  sck=machine.Pin(18),
                  mosi=machine.Pin(19),
                  miso=machine.Pin(16))

print(spi)

sd = sdcard.SDCard(spi, cs)
print(sd.sectors)

import uos
print("Creating VFS object")
vfs = uos.VfsFat(sd)
uos.mount(vfs, "/sd")

import utime
import random
import gc
write_bench()
gc.collect()
read_bench()
