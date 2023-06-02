#!/usr/bin/env python3
import sys
from pathlib import Path
from optparse import OptionParser

def checksum(data):
    bsum = 0x2f;
    bxor = 0x15;
    for i in range(len(data)):
        bsum += data[i]
        bxor ^= data[i]
    return bytearray([bsum & 0xFF, bxor & 0xFF])

def patch(data, offset, bytes):
    for i in range(len(bytes)):
        data[offset] = bytes[i]
        offset += 1
        
def protect(data, address, length):
    crc_data = b'Magic' + checksum(data[address:(address + length)])
    patch(data, address + length, crc_data) 
    patch(data, address + length + len(crc_data), data[address:(address + length + len(crc_data))])

def check(data, offset, bytes):
    for i in range(len(bytes)):
        if data[offset] != bytes[i]:
            return False
        offset += 1
    return True

def validate(data, address, length):
    crc_data = b'Magic' + checksum(data[address:(address + length)])
    return check(data, address + length, crc_data) and check(data, address + length + len(crc_data), data[address:(address + length + len(crc_data))])

def main():
    parser = OptionParser("Usage: gbcamerafix.py [options] INPUT_FILE_NAME.SAV")
    parser.add_option("-o", '--out',        dest='outfilename',                                      help='output file name')

    (options, args) = parser.parse_args()

    if (len(args) == 0):
        parser.print_help()
        parser.error("Input file name required\n")
    
    infilename = Path(args[0])
    
    if options.outfilename == None:
        outfilename = infilename.with_suffix('.fixed')
    else:
        outfilename = Path(options.outfilename)


    sys.stdout.write("\nGenerating the camera save file...\n")

    # load data from file
    with open(str(infilename), "rb") as f:
        SRAMSIZE = 1024 * 128
        data = bytearray(f.read())
        if (len(data) >= SRAMSIZE):
            data = data[0:SRAMSIZE]
        elif (len(data) < SRAMSIZE):
            data += bytes(SRAMSIZE - len(data)) 
    
    fixed = False    
    
    # album
    if not validate(data, 0x01000, 0xD2):
        protect(data, 0x01000, 0xD2)
        sys.stdout.write("Album checksum fixed\n")
        fixed = True
    # vector
    if not validate(data, 0x011B2, 0x1E):
        protect(data, 0x011B2, 0x1E)
        sys.stdout.write("Vector checksum fixed\n")
        fixed = True
    # camera owner
    if not validate(data, 0x02FB8, 0x12):
        protect(data, 0x02FB8, 0x12)
        sys.stdout.write("Camera owner checksum fixed\n")
        fixed = True
    # picture owner
    for address in range(0x02000, 0x20000, 0x1000):
        if not validate(data, address + 0x00F00, 0x55):
            protect(data, address + 0x00F00, 0x55)
            sys.stdout.write("Camera image {:d} fixed\n".format((address - 0x2000) // 0x1000))
            fixed = True

    # save data to file
    if fixed:
        with open(str(outfilename), "wb") as f:
            f.write(data)

    sys.stdout.write("Done!\n")

if __name__=='__main__':
    main()