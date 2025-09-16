#!/usr/bin/env python3
import sys
from pathlib import Path
from optparse import OptionParser

def gbcompress(data):
    outbuf = bytearray()
    inbuf = bytearray(data)
    
    position = 0

    size_in = len(data)
    if size_in >= 0x10000:
        return None
    
    trash_len = 0;
        
    while (position < size_in):

        # Check for rle8
        rle_u8_match = data[position];
        rle_u8_len = 1;
        while ((position + rle_u8_len) < size_in):
            if ((data[position + rle_u8_len] == rle_u8_match) and (rle_u8_len < 64)):
                rle_u8_len += 1;
            else: 
                break;

        # Check for rle16
        if ((position + 1) < size_in):
            rle_u16_match = (data[position] << 8) | data[position + 1]
            rle_u16_len = 1;
            while ((position + (rle_u16_len * 2) + 1) < size_in):
                temp_u16 = (data[position + (rle_u16_len * 2)] << 8) | data[position + (rle_u16_len * 2) + 1]
                if ((temp_u16 == rle_u16_match) and (rle_u16_len < 64)):
                    rle_u16_len += 1;
                else: 
                    break;
        else:
            rle_u16_len = 0;
        
        # Check for matching sequences starting at current position against all previous data beginning at start up to 63 bytes max
        rle_str_back_offset = 0;
        rle_str_len = 0;
        rle_str_start = 0;
        
        while (rle_str_start < position):
            rle_str_len_work = 0;

            while ((position + rle_str_len_work) < size_in):
                if ((data[rle_str_start + rle_str_len_work] == data[position + rle_str_len_work]) and ((rle_str_start + rle_str_len_work) < position) and (rle_str_len_work < 64)):
                    rle_str_len_work += 1;
                else: 
                    break;

            if (rle_str_len_work > rle_str_len):
                rle_str_back_offset = position - rle_str_start;
                rle_str_len = rle_str_len_work;
                
            rle_str_start += 1;


        # Write out any rle data if it's ready
        if ((rle_u8_len > 2) and (rle_u8_len > rle_u16_len) and (rle_u8_len > rle_str_len)):
            if (trash_len > 0):
                outbuf.append(((trash_len - 1) & 0x3F) | 0xC0)
                outbuf.extend(bytearray(data[position-trash_len:position]))
                trash_len = 0

            outbuf.append((rle_u8_len - 1) & 0x3F)
            outbuf.append(rle_u8_match)

            position += rle_u8_len
        elif ((rle_u16_len > 2) and ((rle_u16_len*2) > rle_str_len)):
            if (trash_len > 0):
                outbuf.append(((trash_len - 1) & 0x3F) | 0xC0)
                outbuf.extend(bytearray(data[position-trash_len:position]))
                trash_len = 0

            outbuf.append(((rle_u16_len - 1) & 0x3F) | 0x40)
            outbuf.append((rle_u16_match >> 8) & 0xff)
            outbuf.append(rle_u16_match & 0xff)

            position += rle_u16_len * 2
        elif (rle_str_len > 3):
            if (trash_len > 0):
                outbuf.append(((trash_len - 1) & 0x3F) | 0xC0)
                outbuf.extend(bytearray(data[position-trash_len:position]))
                trash_len = 0

            tmpofs = (rle_str_back_offset ^ 0xFFFF) + 1;
            outbuf.append(((rle_str_len - 1) & 0x3F) | 0x80)            
            outbuf.append(tmpofs & 0xff)
            outbuf.append((tmpofs >> 8) & 0xff)

            position += rle_str_len
        elif (trash_len >= 64):
            outbuf.append(((trash_len - 1) & 0x3F) | 0xC0)
            outbuf.extend(bytearray(data[position-trash_len:position]))
            trash_len = 0
        else:
            trash_len += 1
            position += 1

    # Write ramaining uncompressed data
    if (trash_len > 0):
        outbuf.append(((trash_len - 1) & 0x3F) | 0xC0)
        outbuf.extend(bytearray(data[position-trash_len:position]))
        trash_len = 0
        
    # Write terminator
    outbuf.append(0x00)
    
    return outbuf

def write_array(f, ident, data):
    f.write(bytes("const uint8_t {:s}[{:d}] = {{\n".format(ident, len(data)), "ascii"))
    j = 0
    for i in data:
        f.write(bytes("0x{:02x},".format(i), "ascii"))
        if j == 15:
            f.write(bytes("\n", "ascii"))
            j = 0
        else: 
            j = j + 1
    if j != 0:
        f.write(bytes("\n", "ascii"))    
    f.write(bytes("};\n", "ascii"))

def chunks(lst, n):
    for i in range(0, len(lst), n):
        yield lst[i:i + n]

def main(argv=None):
    parser = OptionParser("Usage: sgbborder_comp.py [options] PALS.BIN TILES.BIN MAP.BIN")
    parser.add_option("-o", '--out',        dest='outfilename',                                        help='output file name')
    parser.add_option("-i", '--identifier', dest='identifier',                                         help='source identifier')
    parser.add_option("-b", '--bank',       dest='bank',                               default='255',  help='BANK number (default 255)')    

    (options, args) = parser.parse_args()

    if (len(args) == 0):
        parser.print_help()
        parser.error("Input file name required\n")

    if (len(args) > 3):
        parser.print_help()
        parser.error("Too many input files\n")

    outfilename = Path(options.outfilename)
    
    with open(str(outfilename), "wb") as outf:
        with open(str(outfilename.with_suffix('.h')), "wb") as outh:
            outh.write(bytes(("#pragma once\n\n"
                              "#include <gbdk/platform.h>\n"
                              "#include <stdint.h>\n\n"
                              "BANKREF_EXTERN({0:s})\n\n").format(options.identifier), "ascii"))
            outf.write(bytes(("#pragma bank {1:s}\n\n"
                              "#include <gbdk/platform.h>\n"
                              "#include <stdint.h>\n\n"
                              "BANKREF({0:s})\n\n").format(options.identifier, options.bank), "ascii"))
            narg = 0
            for arg in args:
                infilename = Path(arg)

                with open(str(infilename), mode="rb") as f:
                    data = f.read()

                i = 0
                for block in chunks(data, 0x1000):
                    decomp_size = len(block)
                    compressed_data = gbcompress(block)

                    write_array(outf, "{:s}{:d}".format(infilename.stem, i), compressed_data)
                    outh.write(bytes("extern const uint8_t {:s}{:d}[{:d}];\n".format(infilename.stem, i, len(compressed_data)), "ascii"))
                    outh.write(bytes("#define size_{:s}{:d} {:d}\n\n".format(infilename.stem, i, decomp_size), "ascii"))

                    i += 1

                if (narg == 1) and (i == 1):
                    outh.write(bytes("#define {:s}{:d} NULL\n".format(infilename.stem, i), "ascii"))
                    outh.write(bytes("#define size_{:s}{:d} 0\n\n".format(infilename.stem, i), "ascii"))
                
                narg += 1
if __name__=='__main__':
    main()