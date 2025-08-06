#!/usr/bin/env python3
import sys
from pathlib import Path
from optparse import OptionParser

class RomFile:
    ROMFILE_PATH = None
    ROMFILE = bytearray()
    
    def __init__(self, file=None):
        if isinstance(file, str):
            self.Open(file)
        elif isinstance(file, bytearray):
            self.ROMFILE = file
    
    def Open(self, file):
        self.ROMFILE_PATH = file
        self.Load()
    
    def Load(self):
        with open(self.ROMFILE_PATH, "rb") as f:
            self.ROMFILE = bytearray(f.read())

    def Save(self):
        with open(self.ROMFILE_PATH, "wb") as f:
            f.write(self.ROMFILE)
    
    def CalcChecksumHeader(self, fix=False):
        checksum = 0
        for i in range(0x134, 0x14D):
            checksum = checksum - self.ROMFILE[i] - 1
        checksum = checksum & 0xFF
        
        repair = not (self.ROMFILE[0x14D] == checksum) and fix
        if repair: self.ROMFILE[0x14D] = checksum
        return repair
    
    def CalcChecksumGlobal(self, fix=False):
        temp1 = self.ROMFILE[0x14E]
        temp2 = self.ROMFILE[0x14F]
        old_sum = (temp1 << 8) | temp2
        self.ROMFILE[0x14E] = 0
        self.ROMFILE[0x14F] = 0
        checksum = sum(self.ROMFILE) & 0xFFFF
        repair = not (old_sum == checksum) and fix
        if repair:
            self.ROMFILE[0x14E] = checksum >> 8
            self.ROMFILE[0x14F] = checksum & 0xFF
        else:
            self.ROMFILE[0x14E] = temp1
            self.ROMFILE[0x14F] = temp2
        return repair

    def Fix(self):
        self.Load()
        check1, check2 = self.CalcChecksumHeader(True), self.CalcChecksumGlobal(True)
        if check1 or check2:
            self.Save()
            print("ROM checksums fixed")

def main(argv=None):
    parser = OptionParser("Usage: romfixer.py [options] INPUT_FILE_NAME")
    parser.add_option("-e", '--ext',        dest='suffix',                                        help='check for suffix')

    (options, args) = parser.parse_args()

    if (len(args) == 0):
        parser.print_help()
        parser.error("Input file name required\n")
    
    infilename = Path(args[0])

    if options.suffix != None:
        if infilename.suffix.lower() != ".{}".format(options.suffix).lower():
            return

    if infilename.is_file():
        ROM = RomFile(str(infilename))
        ROM.Fix()
    else:
        parser.error("Input file name not found\n")

if __name__=='__main__':
    main()