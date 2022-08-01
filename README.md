# 2bit-pxlr-studio-next

# Fast compiling guide for Windows users 


Download `GBDK-2020` from [here](https://github.com/gbdk-2020/gbdk-2020/actions) under the `GBDK Build and Package` link.

Download `Cigwin` built for Windows from [here](https://www.cygwin.com/).
Install with the additional `make` package from `Devel` category.

Download and install `Python 3` built for Windows from [here](https://www.python.org/downloads/).

Download and install `Git` built for Windows from [here](https://git-scm.com/download/win).

Download `get-pip.py` from [here](https://bootstrap.pypa.io/get-pip.py)

From Powershell or Cigwin, run Python and execute `get-pip.py` to install pip.

Then install Pillow and wave libraries : 

    pip install Pillow
    pip install wave

Now from Cigwin terminal, clone this git repository to its desired destination: 
    
    cd (root of your repository directory)
    git clone https://github.com/untoxa/2bit-pxlr-studio-next

Change the Home directory into the git Makefile:

    cd (your cloned repository directory)

Set GBDK_HOME environment variable from the `Makefile` into the git folder, or set it permenently in the Windows system settings.

Run make in the Cigwin terminal: 

    make

Your roms will be in `./build` folders, enjoy ! 
