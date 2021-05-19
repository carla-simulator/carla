import mmap
import os
import time

fname = './pods.txt'
if not os.path.isfile(fname):
    # create initial file
    with open(fname, "w+b") as fd:
         fd.write(b'\x01\x00\x00\x00\x00\x00\x00\x00')

# at this point, file exists, so memory map it
with open(fname, "r+b") as fd:
    mm = mmap.mmap(fd.fileno(), 8, access=mmap.ACCESS_WRITE, offset=0)

    # set one of the pods to true (== 0x01) all the rest to false
    posn = 0
    while True:
         print(f'writing posn:{posn}')

         # reset to the start of the file
         mm.seek(0)

         # write the true/false values, only one is true
         for count in range(8):
             curr = b'\x01' if count == posn else b'\x00'
             mm.write(curr)

         # admire the view
         time.sleep(2)

         # set up for the next position in the next loop
         posn = (posn + 1) % 8

    mm.close()
    fd.close()
