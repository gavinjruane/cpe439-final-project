# cpe439-final-project

Joshua Gonzalez, Jeremy Macchiarella, Gavin Ruane, and Michael Wilson &bullet; CPE 439-01

## Notes and Resources

- [Meeting 1 Notes](https://github.com/gavinjruane/cpe439-final-project/blob/main/notes/initial-meeting-may14.md)
- [Project Proposal](https://docs.google.com/document/d/1telajZnM3WR28OJqXH92RLEYiQae_iakEk_JQqzX1Ic/edit?usp=sharing)

## Setting up ESP-IDF with Visual Studio Code

I never thought I would say this, but STM32CubeIDE is actually somewhat *easy* to set up compared to ESP-IDF&mdash;but it is possible. Here are the steps I followed on my **Arch-based Linux machine**. It should be similar but possibly easier on Windows.

1. From Visual Studio Code, install the ESP-IDF extension.
2. Once the extension is installed, open it and click on the *ESP-IDF Installation Manager* in the list.
3. Here, you can install the development framework for ESP32. You could probably get away with the Easy install, but I chose the *Custom* install to see if I could add any options to make development easier.
    1. Select *All Targets*.
    2. Select the latest IDF version (should be 6.0.1).
    3. Use the default mirrors.
    4. Get the required features *and the optional **IDE** feature*.
    5. Don't choose any additional tools unless you don't have CMake installed. (I'm sorry, I'm not sure how CMake works on Windows.)
    6. Install ESP-IDF and wait for it to complete.

After this point, ESP-IDF should be installed for basic usage. 

### Installing OpenOCD Rules (likely Linux only)

Before I could do anything useful with the dev board, I needed to install the OpenOCD rules so I could communicate with it. As far as I'm aware, this isn't necessary for Windows, but I'm going to include it here in case it's useful to you.

1. First, you might need to be a member of the `uucp` or `dialout` groups. (Replace `uucp` with `dialout` if you're on a Debian or Ubuntu distro.)

	`sudo usermod -a -G uucp $USER`
2. Restart your computer to apply the group changes.
2. Copy the udev rules provided with ESP-IDF to the system udev config directory.

	`sudo cp ~/.espressif/tools/openocd-esp32/v*/openocd-esp32/share/openocd/contrib/60-openocd.rules /etc/udev/rules.d/`
2. Apply the necessary changes.

	```
	sudo udevadm control --reload-rules
	sudo udevadm trigger
	```

### Creating a Project

1. Open Visual Studio Code and launch the Command Palette (`F1`).
2. Type `ESP-IDF: New Project` to create an ESP32 project.
3. Set the serial port used to communicate with the dev board using the "plug" icon on the bottom strip of Visual Studio Code.
    1. If you have your dev board plugged in, it might auto-detect it. You can also try the `detect` option.
    2. If not, I think you can use Device Manager to find what serial port it is connected to. (On Linux, you can run `ls /dev/tty*` to see the available serial ports, and it will likely be `/dev/ttyUSB0`.)
4. Set the target device with the "chip" icon in the bottom strip. The dev board I borrowed from Michael is an esp32s3 for example.

### Building, Flashing, and Monitoring a Project

1. Click the "wrench" icon in the bottom strip to build the project.
2. Once the build completes, click the "lightning bolt" icon to flash the project to the board.
    1. I have just been using UART to flash it, and that has worked well. I think you can use JTAG too, but that needs some more setup.
    2. You can configure whether you're using UART or JTAG with the "star" icon in the bottom strip.
3. Once the program is flashed, you can view the serial console with the "display" icon next to the "lightning bolt."

### FreeRTOS on ESP32

As far as I can tell, FreeRTOS seems to be very tightly integrated with ESP32. The sample projects all include it by default, and, I might be wrong, but all programs require an entry FreeRTOS task (function) called `app_main`.

I've been looking at [this web page](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/freertos.html) for information.

Also see my [sample main.c](https://github.com/gavinjruane/cpe439-final-project/tree/main/notes/sample-main.c) file if you're curious how the basic structure works. (Yes, printf() is a valid call!)

#### FreeRTOS Quirks

Here are some immediate differences I have noticed:

- Stack sizes in `xTaskCreate()` are measured in *bytes*, not words. If you give it too small of a stack size, it will yell at you on the serial console about random errors.
    * So far, 4096 B seems to work fine for very small tasks.
- The default `app_main()` function that is required is some kind of FreeRTOS task.
- You are **not** supposed to edit the `FreeRTOSConfig.h` file like we have done for STM32. Instead, you are supposed to make any config changes through the SDK Configuration Editor.
    * You can access this with the "gear" icon on the bottom strip. From here, click on "Component config" and then "FreeRTOS." There are some familiar options in there.