# 2bit-pxlr-studio-next

**[The latest release version may be downloaded here](https://github.com/untoxa/2bit-pxlr-studio-next/releases/latest/)**

This project requires the reflashable Game Boy Camera cart, PCB's and build instructions may be found [here](https://github.com/HDR/Gameboy-Camera-Flashcart/)

# Fast compiling guide for Windows users 

Basic requirement: 20 minutes without any disturbance.

Download `GBDK-2020` from [here](https://github.com/gbdk-2020/gbdk-2020/actions) under the `GBDK Build and Package` link and unzip it into the desired directory.

Download `Cygwin` built for Windows from [here](https://www.cygwin.com/). Install it with the additional `make` package from `Devel` category.

Download and install `Python 3` built for Windows from [here](https://www.python.org/downloads/).

Download and install `Git` built for Windows from [here](https://git-scm.com/download/win).

Download a Python script from [here](https://bootstrap.pypa.io/get-pip.py), then run from Cygwin in the directory where `get-pip.py` is placed:

    python get-pip.py
    python -m pip install --upgrade pip

Then install Pillow and wave libraries: 

    python -m pip install --upgrade Pillow
    python -m pip install --upgrade wave

Now from the Cygwin terminal, clone this git repository to its desired destination (default is `/home/USER`), but may be any: 

    git clone https://github.com/untoxa/2bit-pxlr-studio-next

Set the `GBDK_HOME` environment variable, which points to the GBDK-2020 folder in the Windows system settings (recommended) or edit the path in the beginning of the `Makefile` in the root of the `2bit-pxlr-studio-next` repository.

Run make from the Cygwin terminal in the project folder containing the `Makefile`: 

    make

Your roms will be in `./build` folders, enjoy ! 

# Remote control packet format

The packet format is very simple and consist of one byte.
```
0bS0IPXXXX  
    S - stop, I - identifier, P - parity, XXXX - 4 Button or D-Pad bits
    Stop bit is always 1.
    Identifier is 1 for upper (buttons) and 0 for lower (D-Pad)
    Parity bit is 1 when the count of 1's in XXXX bits is odd, 0 when even.
```
Sender is a master device for the game boy.

# Transfer image protocol

"Transfer Image" feature protocol is very similar to printing. Only two packets are used, and the game boy does not expect receiving anything in response, we just send the raw image data as quick as possible. 

First, the game boy sends the standard printer INIT packet, and then sends the new `0x10` packet that is the same as DATA, but the data length is always 3585 bytes (16x14 tiles) and CRC bytes are always 0. On CGB the transfer rate is 32KB/s, on the DMG the transfer rate is 1KB/s.
