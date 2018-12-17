# Pelion Device Ready example - template application

(aka Simple Mbed Cloud Client template)

## Overview

This is a template application for platform vendors. It demonstrates how to create a simple application that can connect to the Pelion IoT Platform service, register resources and get ready to receive a firmware update.

It's intended to be forked and customized to add platform-specific features (such as sensors and actuators) and configure the connectivity and storage to work **out-of-the-box**. The template application works in **developer mode** by default.

There is a mirror version of the stable (master) template application on [this location](https://os.mbed.com/teams/mbed-os-examples/code/mbed-cloud-example) to facilitate the fork and publish on os.mbed.com.

## Table of Contents

1. [Getting started with the application](#getting-started-with-the-application)
2. [Porting to a new platform](#porting-to-a-new-platform)
3. [Enabling firmware updates](#enabling-firmware-updates)
4. [Automated testing](#automated-testing)
5. [Known issues](#known-issues)

## Getting started with the application

This is a summary of the process for developers to get started and get a device connected to Pelion Device Management.

### Mbed Online IDE

1. Import the application into the Online IDE.
2. Add the API key to establish connection.
3. Install the developer certificate.
4. Compile and program.

### Mbed CLI tools

1. Import the application into your desktop:

    ```
    mbed import https://os.mbed.com/teams/mbed-os-examples/code/mbed-cloud-example
    cd mbed-cloud-example
    ```

2. Download the developer certificate from the Pelion Device Management [portal](https://portal.mbedcloud.com).
3. Compile and program:

    ```
    mbed compile -t <toolchain> -m <target> -f
    ```

## Porting to a new platform

### Requirements

The hardware requirements for Mbed OS platforms to support Pelion Device Management Client are [here](https://cloud.mbed.com/docs/current/cloud-requirements/index.html).

In general, to start creating a secure connected product, you need a microcontroller that has the following features:

* RAM: 96K or more
* Flash: 512K or more
* True Random Number Generator (TRNG)
* Real Time Clock (RTC)

Additionally, to use the Pelion Device Management Client, the microcontroller needs to support the following in Mbed OS (latest version preferred) or in a compatible driver library:

* A storage device (SDcard, SPI Flash, QSPI Flash, Data Flash)
* IP connectivity (Ethernet, Wi-Fi, Cellular, 6LoWPAN, Thread)

For the Firmware update over the air (FOTA), you need the following:

* [FlashIAP](https://github.com/ARMmbed/mbed-os/blob/master/drivers/FlashIAP.h) - Flash In-Application Programming (IAP).
* [Mbed Bootloader](https://github.com/ARMmbed/mbed-bootloader) or a compatible bootloader.
* TCP connection - the current Firmware Download client only supports HTTP download over TCP (this will be resolved in a future release, so that CoAP blockwise transfer will be used for UDP connections).

### References

* Check which Mbed OS platforms are supported in the [Pelion Device Management quick-start guide](https://cloud.mbed.com/quick-start).
* Check which storage options are available [here](https://os.mbed.com/docs/latest/reference/storage.html).
* Check which network options are available [here](https://os.mbed.com/docs/latest/reference/network-socket.html).

### Adding a new target

#### 1. Create new target entry
Edit the `mbed_app.json` and create a new entry under `target_overrides`:
   * **Connectivity** - specify default connectivity type for your target. It's essential with targets that lack default connectivity set in targets.json or for targets that support multiple connectivity options. Example:
   ```
            "target.network-default-interface-type" : "ETHERNET",
   ```
   At the time of this writing, the possible options are `ETHERNET`, `WIFI`, `CELLULAR`.
   
   Depending on connectivity type, you might have to specify more config options, e.g. see already defined `CELLULAR` targets in `mbed_app.json`.

   * **Storage** - specify storage blockdevice type, which dynamically adds the blockdevice driver you specified at compile time. Example:
   ```
            "target.components_add" : ["SD"],
   ```
   Valud options are `SD`, `SPIF`, `QSPIF`, `FLASHIAP` (not recommended). Check more available options under https://github.com/ARMmbed/mbed-os/tree/master/components/storage/blockdevice

   You will also have to specify blockdevice pin configuration, which may be very different from one blockdevice type to another. Here's an example for `SD`:
   ```
            "sd.SPI_MOSI"                      : "PE_6",
            "sd.SPI_MISO"                      : "PE_5",
            "sd.SPI_CLK"                       : "PE_2",
            "sd.SPI_CS"                        : "PE_4",
   ```
   * **Flash** - define the basics for the microcontroller flash, e.g.:
   ```
            "flash-start-address"              : "0x08000000",
            "flash-size"                       : "(2048*1024)",
   ```
   * **SOTP** - define 2 SOTP/NVStore regions which will be used for Mbed OS Device Management to store it's special keys which are used to encrypt the data stored on the storage. Use the last 2 Flash sectors (if possible) to ensure that they don't get overwritten when new firmware is applied. Example:
   ```
            "sotp-section-1-address"            : "(MBED_CONF_APP_FLASH_START_ADDRESS + MBED_CONF_APP_FLASH_SIZE - 2*(128*1024))",
            "sotp-section-1-size"               : "(128*1024)",
            "sotp-section-2-address"            : "(MBED_CONF_APP_FLASH_START_ADDRESS + MBED_CONF_APP_FLASH_SIZE - 1*(128*1024))",
            "sotp-section-2-size"               : "(128*1024)",
            "sotp-num-sections" : 2
   ```
   `*-address` defines the start of the Flash sector and `*-size` defines the actual sector size. Currently `sotp-num-sections` should always be set to `2`.

   Note that these SOTP regions will be used for the next step...

#### 2. Compile bootloader

1. Edit the `bootloader/bootloader_app.json` and specify:

   * **Flash** - define the basics for the microcontroller flash (the same as in `mbed_app.json`), e.g.:
    ```
            "flash-start-address"              : "0x08000000",
            "flash-size"                       : "(2048*1024)",
    ```

   * **SOTP** - similar to the **SOTP** step above, specify the location of the SOTP key storage. Note that in the bootloader, the variables are named differently. We should try to use the last 2 Flash sectors (if possible) to ensure that they don't get overwritten when new firmware is applied Example:
    ```
            "nvstore.area_1_address"           : "(MBED_CONF_APP_FLASH_START_ADDRESS + MBED_CONF_APP_FLASH_SIZE - 2*(128*1024))",
            "nvstore.area_1_size"              : "(128*1024)",
            "nvstore.area_2_address"           : "(MBED_CONF_APP_FLASH_START_ADDRESS + MBED_CONF_APP_FLASH_SIZE - 1*(128*1024))", "nvstore.area_2_size" : "(128*1024)",
    ```

    * **Application offset** - specify start address for the application and also the update-client meta info. As these are automatically calculated, you could copy the ones below:
    ```
            "update-client.application-details": "(MBED_CONF_APP_FLASH_START_ADDRESS + 64*1024)",
            "application-start-address"        : "(MBED_CONF_APP_FLASH_START_ADDRESS + 65*1024)",
            "max-application-size"             : "DEFAULT_MAX_APPLICATION_SIZE",
    ```
    
    * **Storage** - specify blockdevice pin configuration, exactly as you defined it in the `mbed_app.json` file. Example:
    ```
            "target.components_add"            : ["SD"],
            "sd.SPI_MOSI"                      : "PE_6",
            "sd.SPI_MISO"                      : "PE_5",
            "sd.SPI_CLK"                       : "PE_2",
            "sd.SPI_CS"                        : "PE_4"
    ```

2. Import the official [mbed-bootloader](https://github.com/ARMmbed/mbed-bootloader/) repository or the [mbed-bootloader-extended](https://github.com/ARMmbed/mbed-bootloader-extended/) repository that builds on top of `mbed-bootloader` and extends the support for filesystems and storage drivers. You can do this with ```mbed import mbed-bootloader-extended``` and change your commandline working dir, e.g. `cd mbed-bootloader-extended`.

3. Compile the bootloader using the `bootloader_app.json` configuration you just editted:
   ```
   mbed compile -t <TOOLCHAIN> -m <TARGET> --profile=tiny.json --app-config=<path to pelion-enablement/bootloader/bootloader_app.json>
   ```

   Note the following:
   * `mbed-bootloader` is primarily optimized for `GCC_ARM` and therefore you might want to compile it with that toolchain.
   * Before jumping to the next step, you should compile and flash the bootloader, and then connect over the virtual comport to ensure that the bootloader is running correctly. You can ignore errors related to checksum verification or falure to jump to application - these are expected at this stage.

#### 3. Include the bootloader
1. Copy the compiled bootloader from `mbed-bootloader/BUILDS/<TARGET>/<TOOLCHAIN>-TINY/mbed-bootloader.bin` to `pelion-enablement/bootloader/mbed-bootloader-<TARGET>.bin`.

2. Edit `pelion-enablement/mbed_app.json` and modify the target entry to include:
  ```
            "target.features_add"              : ["BOOTLOADER"],
            "target.bootloader_img"            : "bootloader/mbed-bootloader-<TARGET>.bin",
            "target.app_offset"                : "0x10400",
            "target.header_offset"             : "0x10000",
            "update-client.application-details": "(MBED_CONF_APP_FLASH_START_ADDRESS + 64*1024)",
   ```
 
   Note that:
   * `update-client.application-details` should be identical in both `bootloader_app.json` and `mbed_app.json`
   * `target.app_offset` is relative offset to `flash-start-address` you specified in the `mbed_app.json` and `bootloader_app.json`, and is the hex value of the offset specified by `application-start-address` in `bootloader_app.json`, e.g. `(MBED_CONF_APP_FLASH_START_ADDRESS+65*1024)` dec equals `0x10400` hex.
   * `target.header_offset` is also relative offset to the `flash-start-address` you specified in the `bootloader_app.json`, and is the hex value of the offset specified by `update-client.application-details`, e.g. `(MBED_CONF_APP_FLASH_START_ADDRESS+64*1024)` dec equals `0x10000` hex.

7. Finally, re-run all tests with:
```
mbed test -t <TOOLCHAIN> -m <TARGET> -n simple-mbed-cloud-client-tests-dev_mgmt*
```

#### Creating a custom target board

Read the Mbed OS [Contributing](https://os.mbed.com/docs/latest/reference/porting-targets.html) documentation on how to add a new target.

#### Update the application logic

The template example uses a ticker object to periodically fire a software interrupt to simulate button presses. Let’s say you want to make an actual button press.

By default, there is a Ticker object, which fires every five seconds and invokes a callback function:

```cpp
Ticker timer;
timer.attach(eventQueue.event(&fake_button_press), 5.0);
```

This callback function changes the `button_res` resource:

```cpp
void fake_button_press() {
    int v = button_res->get_value_int() + 1;

    button_res->set_value(v);

    printf("Simulated button clicked %d times\n", v);
}
```

If you want to change this to an actual button, here is how to do it:

1. Remove:

    ```cpp
    Ticker timer;
    timer.attach(eventQueue.event(&fake_button_press), 5.0);
    ```

2. Declare an `InterruptIn` object on the button, and attach the callback function to the `fall` handler:

    ```cpp
    InterruptIn btn(BUTTON1);
    btn.fall(eventQueue.event(&fake_button_press), 5.0);
    ```

3. Rename `fake_button_press` to `real_button_press`.

## Enabling firmware updates

Mbed OS 5.10 and Mbed CLI 1.8 simplifies the process to enable and perform Firmware Updates. Here is a summary on how to configure the device and verify its correct behaviour.

For full documentation about bootloaders and firmware update, read the following documents:

- [Introduccion to bootloaders](https://os.mbed.com/docs/latest/porting/bootloader.html)
- [Creating and using a bootloader](https://os.mbed.com/docs/latest/tutorials/bootloader.html)
- [Bootloader configuration in Mbed OS](https://os.mbed.com/docs/latest/tools/configuring-tools.html)
- [Mbed Bootloader for Pelion Device Management Client](https://github.com/ARMmbed/mbed-bootloader)
- [Updating devices with Arm Mbed CLI](https://os.mbed.com/docs/latest/tools/cli-update.html)

This is a summary to use Arm Mbed OS managed bootloaders.

#### Verifying that firmware update works

Follow these steps to generate a manifest, compile and perform a firmware update of your device:

1. Configure the API key for your Pelion account.

     If you don't have an API key available, then login in [Pelion IoT Platform portal](https://portal.mbedcloud.com/), navigate to 'Access Management', 'API keys' and create a new one. Then specify the API key as global `mbed` configuration:

    ```
    mbed config -G CLOUD_SDK_API_KEY <your-api-key>
    ```

2. Initialize the device management feature:

    ```
    mbed dm init -d "company.com" --model-name "product-model" -q --force
    ```

3. Compile the application, include the firware update credentials generated before, merge with the bootloader and program the device:

    ```
    mbed compile -t <toolchain> -m <target> -c -f
    ```

4. Open a serial terminal, verify the application boots and is able to register to the Device Management service. Write down the `<endpoint ID>`, as it's required to identify the device to perform a firmware update.

5. Update the firmware of the device through Mbed CLI:

    ```
    mbed dm update device -D <device ID> -t <toolchain> -m <target>
    ```

    Inspect the logs on the device to see the update progress. It should look similar to:

    ```
    Firmware download requested
    Authorization granted
    Downloading: [+++- ] 6 %
    ```

    When the download completes, the firmware is verified. If everything is OK, the firmware update is applied, the device reboots and attemps to connect to the Device Management service again. The `<endpoint ID>` should be preserved.

## Automated testing

The Simple Pelion Client provides Greentea tests to confirm your platform works as expected. The network and storage configuration is already defined in Mbed OS 5.10, but you may want to override the configuration in `mbed_app.json`.

For details on Simple Pelion Client testing, refer to the documentation [here](https://github.com/ARMmbed/simple-mbed-cloud-client#testing).

This template application contains a working application and tests passing for the `K64F` and `K66F` platforms.

## Known issues

Please check the issues reported on github.
