#!/usr/bin/env python3
import sys
from pathlib import Path
from optparse import OptionParser

def emit_object_chunk(ofile, area, data):
    addr = 0
    buf = ""
    i = 0
    for b in data:
        buf = "{:s} {:02X}".format(buf, b)
        if (i == 7):
            ofile.write(bytes(("T {:02X} {:02X} {:02X}{:s}\n"
                               "R 00 00 {:02X} {:02X}\n").format(addr & 0xff, (addr >> 8) & 0xff, (addr >> 16) & 0xff, buf, area & 0xff, (area >> 8) & 0xff), "ascii"))            
            addr += 8
            buf = ""
            i = 0
        else:
            i += 1

    if len(buf) > 0:
        ofile.write(bytes(("T {:02X} {:02X} {:02X}{:s}\n"
                           "R 00 00 {:02X} {:02X}\n").format(addr & 0xff, (addr >> 8) & 0xff, (addr >> 16) & 0xff, buf, area & 0xff, (area >> 8) & 0xff), "ascii"))            
        
    return

def main(argv=None):
    parser = OptionParser("Usage: bin2obj.py [options] INPUT_FILE_NAME.BIN")
    parser.add_option("-o", '--out',        dest='outfilename',                                        help='output file name')
    parser.add_option("-i", '--identifier', dest='identifier',                                         help='source identifier')

    parser.add_option("-b", '--bank',       dest='bank',                               default='1',    help='BANK number (default 1)')    

    (options, args) = parser.parse_args()

    if (len(args) == 0):
        parser.print_help()
        parser.error("Input file name required\n")
    
    infilename = Path(args[0])
    
    if options.outfilename == None:
        outfilename = infilename.with_suffix('.o')
    else:
        outfilename = Path(options.outfilename)
        
    if options.identifier == None:
        identifier = "_" + str(Path(infilename.name).with_suffix(''))
    else: 
        identifier = "_" + options.identifier       

    with open(str(infilename), mode="rb") as f:
        data = f.read()

    sbank = int(options.bank)
    tsize = len(data)
    nbanks = (tsize // 0x4000) + 1 if ((tsize % 0x4000) != 0) else (tsize // 0x4000) 

    ofile = open(str(outfilename), "wb")
    ofile.write(bytes(("XL3\n"
                       "H {:X} areas {:X} global symbols\n").format(nbanks, 3), "ascii"))

    ofile.write(bytes(("S ___bank{:s} Def{:06X}\n").format(identifier, sbank), "ascii"))        
    ofile.write(bytes(("S ___size{:s} Def{:06X}\n").format(identifier, tsize), "ascii"))        

    tmp = tsize
    for bank in range(sbank, sbank + nbanks):
        ofile.write(bytes(("A _CODE_{:d} size {:X} flags 0 addr 0\n").format(bank, 0x4000 if tmp > 0x4000 else tmp), "ascii"))
        if bank == sbank:
            ofile.write(bytes(("S {:s} Def000000\n").format(identifier), "ascii"))        
        tmp = tmp - 0x4000
    
    for area in range(0, nbanks):
        if (len(data) > 0x4000):
            chunk = data[0:0x4000]
            data = data[0x4000:]
        else:
            chunk = data
        emit_object_chunk(ofile, area, chunk)    
    
    ofile.write(b"\n")

    ofile.close()

if __name__=='__main__':
    main()