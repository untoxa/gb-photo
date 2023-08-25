# Photo!

**[The latest release version may be downloaded here](https://github.com/untoxa/gb-photo/releases/latest/)**.

**[More recent builds may be downloaded from the CI](https://github.com/untoxa/gb-photo/actions)** under the "Build and package ROM" link (you must be logged into github to download the build artifacts). Please note, that those are development builds and may not be properly tested and thus contain imcomplete features or bugs, use them on your own risk.

This project is a homebrew enhancing the capabilities of the Game Boy Camera by allowing access to all the possible parameters of the sensor and improving the printing speed and the camera ergonomics in general. This project is compatible with all known models of Game Boy printer and Game Boy printer emulator. The project features Game Boy And Game Boy Color versions.

This project requires a reflashable Game Boy Camera cart. PCB's and building instructions may be found [in this first project](https://github.com/HDR/Gameboy-Camera-Flashcart/) or [in this second one](https://github.com/2BitWizard/GB_Mini_Camera). The two projects are not related and not covered by the same license so read carefully the disclaimers before choosing one. Both require advanced skill in soldering and are not recommended for beginners. [GBxCart flasher from InsideGadget](https://www.gbxcart.com/) is the only device compatible with these flashcarts. A generic flashable cartridge is also necessary if you plan to remote control the camera via serial cable.

The save format used here is retro-compatible with any version of the Game Boy Camera rom. This means that you can switch between roms while keeping the same save without loosing your precious images and data. Technically all camera parameters are stored in various empty area of the camera working ram. They do not interfere with a regular camera rom as all checksums are systematically verified.

Photo! is also compatible with [mBGA](https://mgba.io/), [SameBoy](https://sameboy.github.io/) and [PizzaBoy](https://play.google.com/store/apps/details?id=it.dbtecno.pizzaboypro&hl=fr&gl=US) among other non yet tested emulators.

# Photo! with its serial remote controller
![Showcase](doc/Images/Showcase.jpg)

Example of use: Photo! working on a GB Boy Colour and remotely controlled from a Game Boy Color with a serial cable.

# User Manual in brief

The repo contains two sets of roms. One set is compiled for DMG Game boy and Game Boy Pocket and basically features same slow transfer rate and a default "color" palette, as the original camera. The set compiled for Game Boy Color is able to feature Double Speed Mode, very high baudrate transfer mode and features several palettes. The two sets come with a remote control rom allowing to use another Game Boy as controller via the serial cable. You can flash the remote control rom on any flashable cart, it will be compatible. 

## Main hub

![Main Hub](doc/Images/Main_Hub.png)

The **main hub** brings you to the 5 main menus by pressing START after boot. You can skip it and going directly to **Camera Mode** by pressing A/B/SELECT.

- The **Camera mode** is where you take photos. It features 3 different mode for image capturing (see next section).
- The **Gallery** is your main image viewer/printer/eraser, dealing with images stored into ram. It features the same options as the Gallery of the Game Boy Camera. It is natively limited to 30 images.
- The **Flash storage** is a local mass storage system for images. Photo! is a 128kB rom and the MAC-GBD allows dealing with 1MB roms. All space after the 128kB of rom are writable by Photo! to store camera rolls. It allows storing 7 rolls of 30 images into flash memory, increasing the total number of images stored in Photo! to 240 (30 in ram, 210 in flash memory).
- The **Settings** menu features general parameters like palettes, Double Speed Mode, SGB borders, etc.
- The **Credits** features... the credits to the Photo! authors, artists, coders, debuggers.

## Camera Mode

![Camera Mode](doc/Images/Camera_Mode.png)

The **camera mode** was created to propose many more features than a regular camera like infinite time-lapse, ultra-fast transmission, full manual mode, support of third party devices, etc. It was polished by people passionate by the Game Boy Camera.

### Capture Mode menu
- The **Auto Mode** is a mode that mimics the behavior of the stock Game Boy Camera, it modifies automatically the **Exposure time**, and sets the **Gain**, **Sensor Voltage Out** and **Edge enhancement mode** according to rules assessed by datalogging the [MAC-GBD/sensor protocol](src/state_camera.c#L147). All other camera registers used are fixed.
- The **Assisted Mode** uses the same strategy than Auto Mode with a manual setting of the exposure time.
- The **Manual Mode** allows **modifying all the parameters** of the camera sensor that can be sent to the MAC-GBD. Glitches in perspective !

### Trigger menu
- **A Button** is the most simple action: press A once to get an **Action**.
- **Timer** launch a remote timer between 1 and 99 seconds and trigger an **Action**.
- **Repeat** allows triggering an **Action** repetitively, it is cumulative with **Timer**.
- **EAB mode** (for Automatic Exposure Bracketing) is an automatic burst mode were each image has a different exposure. You can choose the number of **Steps** (for example 14 steps means 14 images gradually under-exposed + 14 images gradually over-exposed + a central image, so 29 images taken in total) and the maximal possible **Range** possible in EV (indicated here for 29 images). The more you take images, the farther it goes in EV. Images can be processed together to make and HDR image (**H**igh **D**efinition **R**ange) by using the [pico-gb-printer](https://github.com/untoxa/pico-gb-printer), the [Game Boy Camera Gallery](https://herrzatacke.github.io/gb-printer-web/#/home) or the [GB Camera Android Manager](https://github.com/Mraulio/GBCamera-Android-Manager).

**Timer**, **Repeat** and **EAB mode** can be cancelled by pressing B button.

### Action menu
- **Save** just saves image in one of the 30 memory slots of the Camera save ram. If memory is full, it will play an error sound but continues to loop.
- **Print** sends the image with the chosen border directly to the printer without saving.
- **Save & Print** cumulates the two features but continues to print if the memory slots are all occupied.
- **Transfer** allows fast transmission of one image to the [pico-gb-printer](https://github.com/untoxa/pico-gb-printer).

The **Transfer** protocol is very similar to standard printing. Only two packets are used, and the Game Boy does not expect receiving anything in response, we just send the raw image data as quick as possible. First, the Game Boy sends the standard printer INIT packet, and then sends the new `0x10` packet that is the same as DATA, but the data length is always 3584 bytes (16x14 tiles) and CRC bytes are always 0. On CGB the transfer rate is 32KB/s, on the DMG the transfer rate is 1KB/s.

- **Transfer REC** allows continuous transmission to the [pico-gb-printer](https://github.com/untoxa/pico-gb-printer).
- **Save & Transfer** allows fast transmission of one image to the [pico-gb-printer](https://github.com/untoxa/pico-gb-printer) and saving to a memory slot.
- **Pic'n'rec** allows recording one picture on the [InsideGadget's Pic'n'Rec device](https://shop.insidegadgets.com/product/gameboy-camera-picnrec/). The Double Speed Mode (see Settings section) is not supported by the Pic'n'rec device at the moment with this action.
- **Pic'n'rec REC** allows continuous transmission to [InsideGadget's Pic'n'Rec device](https://shop.insidegadgets.com/product/gameboy-camera-picnrec/). The Double Speed Mode (see Settings section) is not supported by the Pic'n'rec device at the moment with this action.

### Exp. area menu
- Allows varying the area of image where exposure is calculated in **Auto Mode**. Can be centred, left, right, top or bottom. 

### Restore Default menu
- Allows to come back to factory settings, in particular if situation is becoming messy in **Manual Mode**.

## Gallery

![Gallery](doc/Images/Gallery.png)

The **Gallery** acts as the regular camera gallery but proposes batch image erase/unerase, displaying the image parameters and many printing options compatible with basically all the printer emulators available on the market. It of course can print on the original one !

- **Info** displays a thumbnail and the camera registers used. It allows printing these informations too.
- **Print** prints the current displayed image with the chosen border at a clock frequency of 8 kHz (1kB/s) or 256 kHz (32 kB/s) in **Fast Printing** mode (GBC only).
- **Print All** prints all the images in memory with the chosen border at a clock frequency of 8 kHz (1kB/s) or 256 kHz (32 kB/s) in **Fast Printing** mode (GBC only).
- **Transfer** prints the current displayed image without border at a clock frequency of 256 kHz (32kB/s) by default. This mode is supported by the [pico-gb-printer](https://github.com/untoxa/pico-gb-printer). **Compatible with Game Boy Color only !**
- **Transfer All** prints all the images in memory without border and with a clock frequency of 256 kHz (32kB/s). This mode is supported by the [pico-gb-printer](https://github.com/untoxa/pico-gb-printer). **Compatible with Game Boy Color only !**
- **Delete** declares the memory slot of the displayed image as free (without erasing data).
- **Delete All** declares all the memory slots as free (without erasing data).
- **Undelete All** declares all the memory slots as containing an image (whatever the slots content).

![multiple selection mode](doc/Images/Gallery_2.png)

You can access a thumbnail viewer by pressing A when viewing an image. It allows batch selecting, erasing, printing and transfering images.

## Flash Storage

![Flash Storage](doc/Images/Flash_storage.png)

The **flash storage** of images is a unique feature from Photo! It allows dumping the whole ram (a "roll") to dedicated areas (the "slots") in the flash memory after the 128 kB of rom. It is intended to be used as film storage in case you do not have a printer with you. 7 storage slots of 30 images capacity are available in total. In the main menu, just choosing a full slot by pushing A allows browsing, printing, restoring to ram and transfering its images. The [Game Boy Camera Gallery](https://herrzatacke.github.io/gb-printer-web/#/home) and the [GB Camera Android Manager](https://github.com/Mraulio/GBCamera-Android-Manager) are able to directly get the images from a rom dump without moving them into ram. The [pico-gb-printer](https://github.com/untoxa/pico-gb-printer) is also able to get a roll directly without moving it to ram.

- **Save roll to slot** is the action of dumping the whole ram (roll) to a flash memory slot. **Its does not erase the pictures from ram.**
- **Move roll to slot** do the same as **Save roll to slot** but **it wipes all images from ram.**
- **Load from slot** moves all the images contained in one slected slot to the sram. Beware, it wipes all the preceding images on ram !
- **Erase slot** well, erases a selected slot. Image are still recoverable with a dedicated tool in case of mistake.
- **Print slot** sends the whole roll in slot to any GB printer compatible device.
- **Transfer slot** sends the whole roll in slot to the [pico-gb-printer](https://github.com/untoxa/pico-gb-printer) at high baudrate. **Compatible with Game Boy Color only !**
- **Clear roll** erases the current roll (the ram). So **Save roll to slot** + **Clear roll** = **Move roll to slot.**

## Settings

![Settings](doc/Images/Settings.png)

Some fancy general parameters like palette, Super Game Boy borders, regular and wild borders, GBA SP mode, overclocking, etc.

- **Frame** allows selecting no frame, normal frame or wild frames. Made by passionate pixel artists. Try'm all !
- **Palette** proposes a choice of fancy palettes for the Game Boy Color and Game Boy Advance.
- **Fast printing** allows switching all print mode from 8 kHz (1 kB/s) to 256 kHz (32 kB/s) by default. **Compatible witn Game Boy Color only !**. This mode is currently supported by the [pico-gb-printer](https://github.com/untoxa/pico-gb-printer) and the [BitBoy](https://gameboyphoto.bigcartel.com/product/bitboy).
- **Alt. SGB borders** allows switching between two Super Game Boy borders. Compatible with PAL and NTSC Super NES !
- **Show screen grid** displays crosses following the rule of thirds.
- **Save confirmation** adds or removes the double A input to save an image. Without save confirmation, camera acts like a burst mode. With save confirmation, like a real Game Boy Camera.
- **IR remote** enables using the IR port to trigger image capture. Any IR emitting device (TV remote control, other Game Boy Color, IR LED) does the job !
- **Quick Boot** shunts any boot screen and drive you immediately to the **Camera mode**. Ideal for sneak attack !
- **Flip live image** reverses the screen so that image appears in correct orientation when using a GBA SP, without the need for any additionnal device.
- **Double speed** enables the Game Boy Color Double Speed Mode where everything goes twice faster (internal clock, printing, screen refreshing, inputs, etc.). This mode draws more current, avoid this when using a modded Game Boy Pocket. This mode also drives the sensor twice faster, so exposure times are all divided by 2. **Important note: the Double Speed Mode is not reliably supported by the Pic'n'rec REC at the moment**.
- **Display exposure** indicates the exposure time of the sensor in **Auto mode**.

## Some technical considerations
The Mitsubishi M64282FP artificial retina is one of the first mass produced CMOS light sensor. This kind of sensor is known for its good behavior in low light conditions and low power consumption. Basically each pixel of the sensor converts the quantity of photons received during an exposure time into a voltage. The sensor is able to perform some basic arithmetics on the voltage values before transfering them to an analog output (inversion, offsetting, 2D operations, multiplication, etc.). This sensor contains 128x128 pixels but only 123 lines returns image information as the first 5 lines are just composed of masked pixels uses to measure the voltage response of sensor in full darkness. The [sensor documentation](doc/M64282FP-datasheet.pdf) is notorious for being crappy and some informations are deduced from the much better documentation of the [Mitsubishi M64283FP sensor](doc/Mitsubishi%20Integrated%20Circuit%20M64283FP%20Image%20Sensor.pdf) which is an upgrade.

## Effect of the main adressable parameters
The M64282FP working parameters are set by 8 bytes (plus their 3-bits addresses) containing various registers. The MAC-GBD itself, mapper of the Game Boy Camera, can only receive 5 bytes of data to tune the sensor so 3 equivalent-bytes of data are not modifiable on the Game Boy Camera (more precisely P, M and X, 3x8 bits, registers also called **Filtering Kernels** are never modified). The mapping between sensor registers and MAC-GBD registers is given [as comment in the project code](include/gbcamera.h#L86). Here is a detailed description of the registers that can be modified:

- The **Exposure Time** (registers C, 16 bits) is the time that each pixel of the sensor will receive photons and convert the integral photon quantity to voltage. The longer the exposure time, the higher the output voltage, the higher the signal to noise ratio, but the higher the motion blur. Sensor can saturate for too long exposure time/too high flux of photons. This sensor allows exposure time from 16 µseconds to 1.044 seconds. Exposure times below 256 µseconds lead to strong vertical artifacts. Using varying exposure time creates vertical (low exposure times) and horizontal (high exposure times) artifacts which are intrinsic to the sensor. The total voltage range between dark and saturated sensor is about 2 volts.
- The **Sensor Gain** (register G, 5 bits) is a multiplier applied between the quantity of photons received and the output voltage. To make an analogy with film camera, gain is similar to the ISOs of the film. However calculating the real corresponding ISOs for each gain value is out of reach with the current documentation of the sensor. Like film cameras, high gains (ISOs) and low exposure times gives noisy images, low gains (ISOs) and high exposure times gives smooth images. The gain used in the Game Boy camera rom varies very little compared to what the sensor is able to in Manual Mode.
- The **Sensor Voltage Out** (register O, 6 bits) is a fine bias applied to the output pin. It allows increasing the sensor dynamic. Basically this voltage should exactly compensate the voltage reading of dark pixels so that the sensor output is 0 volts in total darkness. Game Boy Camera just uses it as a supplementary degree of freedom.
- The **Sensor Voltage Reference** (register V, 3 bits) is a crude voltage bias applied to the output. It is not modified by the Game Boy Camera (and set by default to 1.5 volts). It typically allows to have a match between the min/max output voltage and the min/max input voltage allowed by the external ADC converter used with the sensor. The M64282FP is able to automatically set the the voltage reading of dark pixels at the **Sensor Voltage Reference** via the **Sensor Zero Point** register Z.
- The **Inverse Output** (register I, 1 bit) performs an hardware negative image.
- The **Edge enhancement** is performed by playing on the **Sensor Edge Operation** (register VH, 2 bits), the **Sensor Edge Ratio** (register E, 4 bits) and the **Sensor Edge Exclusive** (register N, 1 bit). It drastically improves the sharpness of the sensor image which is natively very soft without this feature.
Surprisingly, the **Contrast** is not modified by the sensor itself but is set by the MAC-GBD by sending [dithering matrices](src/dither_patterns.c) derived from [Bayer matrices](https://en.wikipedia.org/wiki/Ordered_dithering).

## Remote control packet format

The packet format is very simple and consist of one byte.
```
0bS0IPXXXX  
    S - stop, I - identifier, P - parity, XXXX - 4 Button or D-Pad bits
    Stop bit is always 1.
    Identifier is 1 for upper (buttons) and 0 for lower (D-Pad)
    Parity bit is 1 when the count of 1's in XXXX bits is odd, 0 when even.
```
Sender is a master device for the game boy.

# Fast compiling guide for Windows users

Basic requirement: 20 minutes without any disturbance.

Download `GBDK-2020` from [here](https://github.com/gbdk-2020/gbdk-2020/actions) under the `GBDK Build and Package` link and unzip it into the desired directory.

Download `Cygwin` built for Windows from [here](https://www.cygwin.com/). Install it with the additional `make` package from `Devel` category.

Download and install `Python 3` built for Windows from [here](https://www.python.org/downloads/).

Download and install `Git` built for Windows from [here](https://git-scm.com/download/win).

Download a Python script from [here](https://bootstrap.pypa.io/get-pip.py), then run from Cygwin in the directory where `get-pip.py` is placed:

    python get-pip.py
    python -m pip install --upgrade pip

If `python` command is not recognized, you will have to manually create its own environment variable. Create a `Path` environment variable which targets your Python installation folder (the one that contains `python.exe`) or if `Path` exists, edit it and copy your Python installation path. Close and reopen Cygwin and get back to previous steps. Using `py` command (more generic call to Python under Windows) instead of `python` will aparently fix the issue but compiler will not be compatible.

Then install Pillow and wave libraries:

    python -m pip install --upgrade Pillow
    python -m pip install --upgrade wave

Now from the Cygwin terminal, clone this git repository to its desired destination (default is `/home/USER`), but may be any:

    git clone https://github.com/untoxa/gb-photo

Create a `GBDK_HOME` environment variable which targets your GBDK-2020 folder in the Windows system settings, close and reopen Cygwin.

Run make from the Cygwin terminal in the project folder containing the `Makefile`:

    make

Your roms will be in `./build` folders, enjoy !

# Resources
- Mitsubishi M64282FP [Sensor Datasheet](doc/M64282FP-datasheet.pdf)
- Mitsubishi M64283FP [Sensor Datasheet](doc/Mitsubishi%20Integrated%20Circuit%20M64283FP%20Image%20Sensor.pdf)
- Game Boy [Pan Docs](https://gbdev.io/pandocs/)
- [AntonioND](https://github.com/AntonioND) for the outstanding [Documentation regarding the camera's sensor](https://github.com/AntonioND/gbcam-rev-engineer) / [PDF](https://github.com/AntonioND/gbcam-rev-engineer/blob/master/doc/gb_camera_doc_v1_1_1.pdf)

# Author contribution
- [Andreas Hahn](https://github.com/HerrZatacke) - original idea. 
- [Toxa](https://github.com/untoxa) lead developer.
- [@rembrandx](https://www.instagram.com/rembrandx/) as well as [NeoRame](https://github.com/NeoRame) and 2bitpit for the logo/splashcreen/artworks. THX [Hatch](https://github.com/hatchman) for your frames and the permission to use and change your graphics (Zebra and PCB)
- [Raphaël Boichot](https://github.com/Raphael-Boichot/) for the in-depth analysis of the [Game Boy Camera's RAM structure](https://github.com/Raphael-Boichot/Inject-pictures-in-your-Game-Boy-Camera-saves).
- [Tronimal](https://tronimal.de/game-boy-music-software/) for the original SFX effects.
- [Cristofer Cruz](https://github.com/cristofercruz) for debugging and feature ideas.
- [bbbbbr](https://github.com/bbbbbr) for support in programming.
- [Christian Reinbacher](https://github.com/reini1305) for the [print function](https://github.com/HerrZatacke/custom-camera-rom/commit/5976b47e6b6d577c954e2b678affa9925824f5b5) and general help with some C concepts.
- [Alex (insidegadgets)](https://github.com/insidegadgets) for figuring out the flickering issue and more.
- [All the folks from Game Boy Gamera Club Discord](https://discord.gg/C7WFJHG) for their support and ideas.
