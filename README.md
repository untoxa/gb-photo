# 2bit-pxlr-studio-next

# Fast compiling guide for Windows users 


Download `GBDK-2020` from [here](https://github.com/gbdk-2020/gbdk-2020/actions) under the `GBDK Build and Package` link.

Download `Cigwin` built for Windows from [here](https://www.cygwin.com/).
Install with the additional `make` package from `Devel` category.

Download and install `Python 3` built for Windows from [here](https://www.python.org/downloads/).

Download and install `Git` built for Windows from [here](https://git-scm.com/download/win).

Download `get-pip.py` from [here](https://bootstrap.pypa.io/get-pip.py)

From Powershell or Cigwin, run Python command line and execute `get-pip.py` to install pip installer.

Then install Pillow and wave libraries under Python command line: 

    pip install Pillow
    pip install wave

Now from Cigwin terminal, clone this git repository to its desired destination (default is `/home/USER`): 

    git clone https://github.com/untoxa/2bit-pxlr-studio-next

Change GBDK_HOME environment variable in the `Makefile` into the git folder, or set it permanently in the Windows system settings.

Run make from the Cigwin terminal in the project folder containing the `Makefile`: 

    make

Your roms will be in `./build` folders, enjoy ! 
