# 2bit-pxlr-studio-next

# Fast compiling guide for Windows users 

Install MSYS2 build for windows from https://www.msys2.org/
Install last built GBDK for windows from https://github.com/gbdk-2020/gbdk-2020/actions/runs/2672801001
Then run in MSYS2 command line: 

    pacman -S python
	  python3 get-pip.py
	python3 -m pip install --upgrade Pillow
	python3 -m pip install --upgrade wave
	git clone https://github.com/untoxa/2bit-pxlr-studio

Change the Home directory in the git Makefile
Then run in MSYS2 command line: 

	make
