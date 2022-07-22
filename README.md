# 2bit-pxlr-studio-next

# Fast compiling guide for Windows users 

Install MSYS2 built for windows from [https://www.msys2.org/]

Install latest GBDK built for windows from [https://github.com/gbdk-2020/gbdk-2020/actions/runs/2672801001]

Then run in MSYS2 command line: 

    pacman -S python
   
It installs Python 3. For whatever the reason, installing pip is a pain under Windows, so get this script and run it:

    wget https://bootstrap.pypa.io/get-pip.py
    python3 get-pip.py
    
Install few other dependancies:

    python3 -m pip install --upgrade Pillow
    python3 -m pip install --upgrade wave
    
Clone the git to you local home: 
    
    git clone https://github.com/untoxa/2bit-pxlr-studio

Change the Home directory in the git Makefile
Then run in MSYS2 command line: 

    make

Your roms will be in /build folders, enjoy ! 
