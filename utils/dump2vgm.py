#!/usr/bin/env python3
import sys
from struct import pack

from pathlib import Path
from optparse import OptionParser

def create_vgm(filename):
    outfile = open(filename, "wb")
    outfile.write(pack('IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII', 0x206D6756, 0, 0x161, 0,0,0,0,0,0,0,0,0,0,0xc0-0x34,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x400000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0))
    return outfile

def finalize_vgm(outfile, cut):
    if (cut):
        outfile.write(b'\x62')
        outfile.write(pack('BBBBBBBBBBBB', 0xb3, 0xff12 - 0xff10, 0x00, 0xb3, 0xff17 - 0xff10, 0x00, 0xb3, 0xff1c - 0xff10, 0x00, 0xb3, 0xff21 - 0xff10, 0x00))
        outfile.write(pack('BBBBBBBBB',    0xb3, 0xff14 - 0xff10, 0xc0, 0xb3, 0xff19 - 0xff10, 0xc0,                              0xb3, 0xff23 - 0xff10, 0xc0))
    outfile.write(b'\x66')

def main(argv=None):
    parser = OptionParser("Usage: dump2vgm.py [options] INPUT_FILE_NAME.TXT")

    parser.add_option("-c", "--cut", dest="cut_sound", action="store_true", default=False, help='cut all sound channels')

    (options, args) = parser.parse_args()

    if (len(args) == 0):
        parser.print_help()
        parser.error("Input file name required\n")
    
    infilename = Path(args[0])
    
    outfilename = infilename.with_suffix('.c')

    line = ""
    song_no = 0;
    outfile = None
    reset = True

    with open(str(infilename), 'r') as f:
        line = f.readline()
        while (line):
            decoded_line = [x.strip() for x in line.split()]
            if len(decoded_line) == 0:
                if (outfile):
                    finalize_vgm(outfile, options.cut_sound)
                    outfile = None
                song_no += 1
                print(song_no)
            elif len(decoded_line) == 2:
                if (decoded_line[0] == "subsong"):
                    pass;
                else:
                    if (not outfile):
                        outfile = create_vgm(str(infilename.with_suffix('')) + ".{}.vgm".format(song_no))
                        reset = True
                    reg, value = [int(x.strip(), base = 16) for x in decoded_line[1].split(sep = '=')]

                    if (reg in range(0xFF10, 0xFF16) or reg in range(0xFF16, 0xFF20) or 
                        reg in range(0xFF1A, 0xFF1F) or reg in range(0xFF20, 0xFF24) or 
                        reg in range(0xFF24, 0xFF27) or reg in range(0xFF30, 0xFF40)):
                        delay = round(int(decoded_line[0], base=16) / 70224)
                        if (not reset) and (delay > 0):
                            for i in range(0, delay):
                                outfile.write(b'\x62')
                        outfile.write(pack('BBB', 0xb3, reg - 0xFF10, value))
                    else:
                        print("Skipped: 0x{:04x}".format(reg))

                    reset = False
            
            line = f.readline()
    
    if (outfile):
        finalize_vgm(outfile, options.cut_sound)
        outfile = None
    
    return

if __name__=='__main__':
    main()