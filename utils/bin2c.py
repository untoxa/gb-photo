#!/usr/bin/env python3

import sys

def bin2c(filename, name):
    with open(filename, "rb") as f:
        data = f.read()

        sys.stdout.write("const unsigned char {:s}[] = {{\n".format(name))
        j = 0
        for i in data:
            sys.stdout.write("0x{:02x},".format(i))
            if j == 15:
                sys.stdout.write("\n")
                j = 0
            else: 
                j = j + 1
        if j != 0:
            sys.stdout.write("\n")    
        sys.stdout.write("};\n")

        sys.stdout.write("const unsigned int {:s}_size = {:d};\n".format(name, f.tell()))

    return

if __name__=='__main__':
    bin2c(sys.argv[1], sys.argv[2])
