# 2bit-pxlr-studio-next

# Fast compiling guide for Windows users 


Download `GBDK-2020` from [here](https://github.com/gbdk-2020/gbdk-2020/actions) under the `GBDK Build and Package` link.

Download and install `MSYS2` built for Windows from [here](https://www.msys2.org/).

Then run in MSYS2 command line: 

    pacman -S python
    pacman -S make
   
It installs `Python 3`, make. For whatever the reason, installing pip is always a pain, so get this little script and run it:

    wget https://bootstrap.pypa.io/get-pip.py
    python3 get-pip.py
    
Install few other dependancies:

    python3 -m pip install --upgrade Pillow
    python3 -m pip install --upgrade wave
    
Clone this git repository to its desired destination: 
    
    cd (root of your repository directory)
    git clone https://github.com/untoxa/2bit-pxlr-studio-next

Change the Home directory into the git Makefile:

    cd (your cloned repository directory)

Set GBDK_HOME environment variable:

    set GBDK_HOME=(path to GBDK-2020 directory)

or set it permenently in the Windows system settings

Run make in the MSYS2 command line: 

    make

Your roms will be in `./build` folders, enjoy ! 
