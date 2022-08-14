# 2bit PXLR Studio Next

**[The latest release version may be downloaded here](https://github.com/untoxa/2bit-pxlr-studio-next/releases/latest/)**
This project requires the reflashable Game Boy Camera cart, PCB's and build instructions may be found [here](https://github.com/HDR/Gameboy-Camera-Flashcart/)

# User Manual

## Some technical configurations##
The Mitsubishi M64282FP artificial retina is a one of the first mass produced CMOS light sensor. This kind of sensor is known for its good behavior in low light conditions and low power consumption. Basically each pixel of the sensor converts the quantity of photons received during an exposure time into a voltage. The sensor is able to perform some basic arithmetics on the voltage values before transfering them to an analog output (inversion, offsetting, 2D operations, multiplication, etc.). This sensor contains 128x128 pixels but only 123 lines returns image information as the first 5 lines are just composed of masked pixels uses to measure the voltage response of sensor in full darkness. The sensor documentation is notorious for being unfinished and fuzzy and some informations are deduced from the much better documentation of the Mitsubishi M64283FP sensor which is an upgrade.

The project addresses the sensor in two ways:
- In **Assisted Mode**, the 2bit PXLR Studio rom mimics the behavior of the Game Boy Camera, it modifies only the **Exposure time, gain and output reference voltage**. All other parameters used are fixed. 
- In **Manual Mode**, the 2bit PXLR Studio rom allows **modifying all the parameters** (or registers) of the sensor. This could lead to unexpected results as certain parameter configurations are not expected to give a clear image or to be compatible with the scale of the ADC (analog to digital converter) of the camera.

# Effect of parameters
- The **exposure time** is the time each pixel of the sensor will receive photons and convert the integral photon quantity to voltage. The longer the exposure time, the higher the output voltage, the higher the signal to noise ratio, but the higher the motion blur. Sensor can saturate for too long exposure time/too high flux of photons. This sensor allows exposure time from 16 µseconds to 1.044 seconds. Exposure times below 256 µseconds lead to strong vertical artifacts. Using varying exposure time creates vertical (low exposure times) and horizontal (high exposure times) artifacts which are intrinsic to the sensor. The total voltage range between dark and saturated sensor is about 2 volts.
- The **gain** is a multiplier applied between the quantity of photons received and the output voltage. To make an analogy with film camera, gain is similar to the ISOs of the film. However calculating the real corresponding ISOs for each gain value is out of reach with the current documentation of the sensor. Like film cameras, high gains (ISOs) and low exposure times gives noisy images, low gains (ISOs) and high exposure times gives smooth images. The gain used in the Game Boy camera rom varies very little compared to what the sensor is able to in Manual Mode.
- The **Output reference voltage** is a fine bias applied to the output pin. It allows increasing the sensor dynamic. Basically this voltage should exactly compensate the voltage reading of dark pixels so that the sensor output is 0 volts in total darkness. 
- The **Voltage reference** or **Output node bias voltage** is a crude voltage bias applied to the output. It is not modified by the Game Boy Camera (and set by default to 1.5 volts). It typically allows to have a match between the min/max output voltage and the min/max input voltage allowed by the external ADC converter used with the sensor. The M64282FP is able to automatically set the the voltage reading of dark pixels at the **Voltage reference** via the register Z.


# Credits
Thanks to the following people for direct and indirect help with this project:
* [Toxa](https://github.com/untoxa) for a lot of help while refactoring, and for the GBDK in general. 
* [@rembrandx](https://www.instagram.com/rembrandx/) for the Logo/Splashcreen 
* [Raphaël Boichot](https://github.com/Raphael-Boichot/) for the in-depth analysis of the [Game Boy Camera's RAM structure](https://funtography.online/wiki/Cartridge_RAM)
* [AntonioND](https://github.com/AntonioND) for the outstanding [Documentation regarding the camera's sensor](https://github.com/AntonioND/gbcam-rev-engineer) / [PDF](https://github.com/AntonioND/gbcam-rev-engineer/blob/master/doc/gb_camera_doc_v1_1_1.pdf)
* [reini1305](https://github.com/reini1305) for the [print function](https://github.com/HerrZatacke/custom-camera-rom/commit/5976b47e6b6d577c954e2b678affa9925824f5b5) and general help with some C concepts
* [Alex (insidegadgets)](https://github.com/insidegadgets) for figuring out the flickering issue and more
* [All the folks from Game Boy Gamera Club Discord](https://discord.gg/C7WFJHG) for their support and ideas

# Other resources
- Mitsubishi [Sensor Datasheet](https://pdf1.alldatasheet.com/datasheet-pdf/view/146598/MITSUBISHI/M64282FP.html)


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
