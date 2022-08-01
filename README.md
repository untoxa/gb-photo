# 2bit-pxlr-studio-next

# Fast compiling guide for Windows users 

Basic requirement: 20 minutes without any disturbance.

Download `GBDK-2020` from [here](https://github.com/gbdk-2020/gbdk-2020/actions) under the `GBDK Build and Package` link an unzip it into the desired directory.

Download `Cygwin` built for Windows from [here](https://www.cygwin.com/). Install it with the additional `make` package from `Devel` category.

Download and install `Python 3` built for Windows from [here](https://www.python.org/downloads/).

Download and install `Git` built for Windows from [here](https://git-scm.com/download/win).

Download a Python script from [here](https://bootstrap.pypa.io/get-pip.py), then run from Cygwin in the directory where `get-pip.py` is placed:

    python get-pip.py
    python -m pip install --upgrade pip

Then install Pillow and wave libraries: 

    python -m pip install --upgrade Pillow
    python -m pip install --upgrade wave

Now from Cygwin terminal, clone this git repository to its desired destination (default is `/home/USER`): 

    git clone https://github.com/untoxa/2bit-pxlr-studio-next

Change GBDK_HOME environment variable in the `Makefile` into the git folder, or set it permanently in the Windows system settings.

Run make from the Cygwin terminal in the project folder containing the `Makefile`: 

    make

Your roms will be in `./build` folders, enjoy ! 
