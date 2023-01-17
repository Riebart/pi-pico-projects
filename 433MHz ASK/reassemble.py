#!/usr/bin/env python3

import sys

def str_reverse(s):
    return "".join(reversed(s))

syms = [0xd,  0xe,  0x13, 0x15, 0x16, 0x19, 0x1a, 0x1c, 
    0x23, 0x25, 0x26, 0x29, 0x2a, 0x2c, 0x32, 0x34]
bs = [("%6s" % bin(s)[2:]).replace(" ", "0") for s in syms]
symbolmap = dict([(bs[i],i) for i in range(len(bs))])

instring = sys.argv[1] if len(sys.argv) > 1 else sys.stdin.read().strip()

bits1 = [instring[i:i+8] for i in range(0, len(instring), 8)]
bits2 = "".join([b[0] for b in bits1 if len(set(list(b))) == 1])
print(bits2)
symbols = [bits2[i:i+6] for i in range(0, len(bits2), 6)]
nybbles = [symbolmap[str_reverse(s)] for s in symbols]
bytes = [b2[0] + 16 * b2[1] for b2 in 
    [nybbles[i:i+2] for i in range(0, len(nybbles), 2)]]

print(bytes)

preamble = bytes[0:4]
payload_byte_count = bytes[4] + 256 * bytes[5]
data = bytes[6:-1]
checksum = bytes[-1]

checksum_calc = 0
for b in bytes:
    checksum_calc ^= b
checksum_calc ^= checksum

print("".join([chr(b) for b in data]))
print(checksum, checksum_calc)