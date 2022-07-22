# 2bit-pxlr-studio-next

# Fast compiling guide for Windows users 


Download GBDK-2020 from [here](https://github.com/gbdk-2020/gbdk-2020/actions) under the GBDK Build and Package link.

Download and install MSYS2 built for Windows from [here](https://www.msys2.org/).

Then run in MSYS2 command line: 

    pacman -S python
    pacman -S make
    pacman -S nano
   
It installs Python 3, make and nano (text editor). For whatever the reason, installing pip is always a pain, so get this little script and run it:

    wget https://bootstrap.pypa.io/get-pip.py
    python3 get-pip.py
    
Install few other dependancies:

    python3 -m pip install --upgrade Pillow
    python3 -m pip install --upgrade wave
    
Clone the git to its desired destination: 
    
    cd (root of your git directory)
    git clone https://github.com/untoxa/2bit-pxlr-studio-next

Change the Home directory into the git Makefile:

    cd (your git directory)
    nano Makefile

Modify GBDK_HOME entry, save, then run in MSYS2 command line: 

    make

Your roms will be in /build folders, enjoy ! 
