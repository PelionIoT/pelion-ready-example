# Simple Mbed Cloud Client template application

## Overview

This is a template application for platform vendors. It demonstrates how to create a simple Mbed Cloud Client application that can connect to Mbed Cloud, register resources and get ready to receive a firmware update.

It's intended to be customized to add platform-specific features (such as sensors and actuators) and configure the connectivity and storage to work **out-of-the-box**. The Simple Mbed Cloud Client template application works in **developer mode** by default.

## Setup process

This is a summary of the process for developers to get started and get a device connected to Mbed Cloud.

### Mbed Online IDE

1. Import the application into the Online IDE.
2. Add the API key to establish connection to Mbed Cloud.
3. Install the developer certificate.
4. Compile and program.

### Mbed CLI tools

1. Import the application in developer's desktop:

    ```
    mbed import https://os.mbed.com/teams/mbed-os-examples/code/mbed-cloud-example
    cd mbed-cloud-example
    ```

2. Download the developer certificate from Mbed Cloud.
3. Compile and program:

    ```
    mbed compile -t <toolchain> -m <target> -f
    ```

## Porting to a new platform

### Requirements

The hardware requirements for Mbed OS platforms to support Mbed Cloud Client are [here](https://cloud.mbed.com/docs/current/cloud-requirements/index.html).

In general, to start creating a secure connected product, you need a microcontroller that has the following features.
* RAM: 96K or more
* Flash: 512K or more
* True Random Number Generator (TRNG)
* Real Time Clock (RTC)

Additionally, to use Mbed Cloud Client, the microcontroller needs to support the following in Mbed OS (latest version preferred) or in a compatible driver library:
* A storage device (SDcard, SPI Flash, Data Flash)
* IP connectivity (Ethernet, WiFi, Cellular, 6LoWPAN, Thread)

For the Firmware update over the air (FOTA), you need the following:
* [FlashIAP](https://github.com/ARMmbed/mbed-os/blob/master/drivers/FlashIAP.h) - Flash In-Application Programming (IAP)
* [Mbed Bootloader](https://github.com/ARMmbed/mbed-bootloader) or a bootloader compatible with Mbed Cloud Client.

### References

* Check which Mbed OS platforms are supported in the [Mbed Cloud quick-start guide](https://cloud.test.mbed.com/quick-start).
* Check which storage options are available [here](https://os.mbed.com/docs/v5.8/reference/storage.html).
* Check which network options are available [here](https://os.mbed.com/docs/v5.8/reference/network-socket.html).


### Porting steps

Supporting a new derivative platform requires the following steps:

* Fork the template and create an example application for your platform in https://os.mbed.com.
* (Optional) Change the connectivity interface. Ethernet is the default - see `main.cpp`.
* (Optional) Change the filesystem and/or the block device for storage. FAT filesystem over SD card is the default. See `main.cpp`.
* (Optional) Make minor changes in `mbed_app.json` to support multiple platforms with same connectivity and storage.

<span class="notes">**Note:** Make sure that the application works out-of-the-box and no changes are required in the `main.cpp` file. The goal is to deliver a great UX to our developers.</span>

### Porting example

In this example, an app with an SD card and on-chip Ethernet is taken to a custom board that has an MCU + Wi-Fi module.

#### Changing the storage option

##### For an SD card

1. Add the SD card driver (`sd-driver.lib`) if it is not already added.

    On the command line:

    ```
    mbed add https://github.com/armmbed/sd-driver
    ```

2. In the online compiler, click **Import**, then click here to import from URL.
3. Then enter https://github.com/armmbed/sd-driver for the **Source URL** and **Import As:** Library.
4. Include the header files for the SD driver and FAT file system:

    ```cpp
    #include "SDBlockDevice.h"
    #include "FATFileSystem.h"
    ```

5. Declare the global objects for the SD card and file system.

    ```cpp
    SDBlockDevice bd(SPI_MOSI, SPI_MISO, SPI_CLK, SPI_CS);
    FATFileSystem fs("sd", &sd);
    ```

<span class="notes">**Note:** The `SPI_*` macros represent the pin names. The names can be defined in a variety of places including the sd-driver, your project’s configuration file (`mbed_app.json`) or the `pinnames.h` file for the target that defines the default pin names. You can use other pin names depending on the platform and the connections.</span>

For example, if the SPI signals for the SD card interface are connected on an Arduino compatible shield, you may define them like this:

```cpp
SDBlockDevice sd(D11, D12, D13, D10);
```

##### For SPI Flash (devices that support SFDP)

<Please note that this section of the document is under construction.  More information is needed.>

1. Add the SPI Flash driver (`spif-driver`) if it is not already added.

    ```
    mbed add https://github.com/ARMmbed/spif-driver
    ```

2. Include the header files for the SPI Flash driver and LitteFS file system. For SPI Flash, we recommend LittleFS file system which supports wear leveling.

    ```cpp
    #include "SPIFBlockDevice.h"
    #include "LittleFileSystem.h"
    ```

3. Declare the global objects for the SD card and file system:

    ```cpp
    SPIFBlockDevice spif(SPI_MOSI, SPI_MISO, SPI_CLK, SPI_CS);
    LittleFileSystem fs("fs", &spif);
    ```

4. Update the construction of the `SimpleMbedCloudClient` object to pass in the file system and block device:

    ```cpp
    SimpleMbedCloudClient client(&net, &spif, &fs);
    ```

#### Changing the network interface

##### For Ethernet

The Ethernet interface is included within Mbed OS, so you do not need to add a library.

1. Include the header file for the interface.

    ```
    #include "EthernetInterface.h"
    ```

2. Declare the network interface object.

    ```
    EthernetInterface net;
    ```

3. Connect the interface.

    ```
    status = net.connect();
    ```

4. When the Mbed Cloud Client is started, pass the network interface.

    ```
    SimpleMbedCloudClient client(&net, &sd, &fs);
    ```

##### For WiFi

This example references the ESP8266 WiFi module, but the instructions are applicable to other modules.

1. Add the ESP8266 WiFi interface driver (esp8266-driver) if it is not already added.

    ```
    Mbed add https://github.com/ARMmbed/esp8266-driver
    ```

    <span class="notes">**Note:** You may have to update the firmware inside the ESP8266 module.</span>

2. Include the header file for the interface.

    ```cpp
    #include "ESP8266Interface.h"
    ```

3. Add the driver configuration information in `mbed_app.json` (located at the top level of the Mbed Cloud Connect example project).

    ```json
        "config": {
            "wifi-ssid": {
                "help": "WiFi SSID",
                "value": "\"SSID\""
            },
            "wifi-password": {
                "help": "WiFi Password",
                "value": "\"PASSWORD\""
            }
        }
    ```

4. Declare the network interface object.

    ```cpp
    ESP8266Interface net(D1, D0);
    ```

5. Connect the interface.

    ```cpp
    nsapi_error_t status = net.connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    ```

6. When the Mbed Cloud Client is started, pass the network interface.

    ```cpp
    SimpleMbedCloudClient client(&net, &sd, &fs);
    ```

#### Changing the target MCU

To change the target board to another board that is supported by Mbed OS, simply change the target name.

##### Using the command line

Use the -m option.

```
mbed compile -m Hexiwear -t GCC_ARM
```

##### Using the online compiler

Click the platform name on the top right corner, then select another platform.

##### Using an IDE

First re-export (create project files) for the target with the command line.

```
mbed export -m Hexiwear -i uvision
```

#### Creating a custom target board

Read the Mbed OS [Contributing](https://os.mbed.com/docs/latest/reference/porting-targets.html) documentation on how to add a new target.

#### Update the application logic

The template example uses a ticker object to periodically fire a software interrupt to simulate button presses. Let’s say you want to make an actual button press.

By default, there is a Ticker object, which fires every five seconds and invokes a callback function.

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

1. Declare an `InterruptIn` object on the button, and attach the callback function to the `fall` handler:

    ```cpp
    InterruptIn btn(BUTTON1);
    btn.fall(eventQueue.event(&fake_button_press), 5.0);
    ```

1. Rename `fake_button_press` to `real_button_press`.


#### Updating the LwM2M objects

See guide at [TODO]

#### Mbed Cloud Client v1.3.x SOTP specific changes

Mbed Cloud Client v1.3.x introduces a new feature called SOTP that makes use of the internal flash of the MCU as a One-Time-Programmable section in order to store the keys required to decrypt the credentials stored in the persistent storage. More information on this can be found [here](https://cloud.mbed.com/docs/current/porting/changing-a-customized-porting-layer.html#rtos-module) under the RTOS module.

Assuming the flash is divided into 2 sections for your target, the following changes are required to mbed_app.json
1. Add a section to the target_overrides with sotp addresses and sizes.

For example, for the NUCLEO_L476RG
    ```json
        "NUCLEO_L476RG": {
            "sotp-section-1-address"           : "(0x08000000+((1024-32)*1024))",
            "sotp-section-1-size"              : "(16*1024)",
            "sotp-section-2-address"           : "(0x08000000+((1024-16)*1024))",
            "sotp-section-2-size"              : "(16*1024)",
            "sotp-num-sections"                : 2
        }
    ```
2. Add the macro definition to the "config" section. Note that the address and size macros are already provided. You only have to add the macro for the number of sections
    ```json
        "sotp-num-sections": {
            "help": "Number of SOTP sections",
            "macro_name": "PAL_INT_FLASH_NUM_SECTIONS",
            "value": null
        }
    ```

## Enabling firmware updates

To enable firmware updates, a compatible bootloader needs to be added in the `tools/` folder. The process to merge the application with the bootloader currently only works when building with Mbed CLI. In the future, this combine process will be done automatically by Mbed tools.

1. Compile [mbed-bootloader](https://github.com/armmbed/mbed-bootloader) for the platform and storage configuration used in this application. Place the binary in the tools folder.

1. Add a section to `mbed_app.json` under `target_overrides` with the bootloader configuration. For example:

    ```json
        "K64F": {
            "target.mbed_app_start"            : "0x0000a400",
            "update-client.bootloader-details" : "0x00007188",
            "sotp-section-1-address"           : "(32*1024)",
            "sotp-section-1-size"              : "( 4*1024)",
            "sotp-section-2-address"           : "(36*1024)",
            "sotp-section-2-size"              : "( 4*1024)",
            "update-client.application-details": "(40*1024)"
        }
    ```

Next, instruct your users to do the following:

1. Install the [manifest tool](https://github.com/armmbed/manifest-tool).
1. Generate an update certificate:

    ```
    $ manifest-tool init -a YOUR_MBED_CLOUD_API_KEY -d yourdomain.com -m device-model-id -q --force
    ```

    <span class="notes">**Note:** Make sure to replace `YOUR_MBED_CLOUD_API_KEY` with an Mbed Cloud API key.

1. Build the application and combine it with the bootloader:

    ```
    $ mbed compile -m YOUR_TARGET -t GCC_ARM
    $ tools/combine_bootloader_with_app.py -m YOUR_TARGET -a BUILD/YOUR_TARGET/GCC_ARM/simple-mbed-cloud-client-example_application.bin -o combined.bin
    ```

1. Flash `combined.bin` to the development board.
1. Write down the endpoint ID of the board. You need it to start the update.

Now, a firmware update can be scheduled as explained in the [Mbed Cloud documentation](https://cloud.mbed.com/docs/current/updating-firmware/index.html). You can do it with the manifest tool itself or via the Mbed Cloud portal. Here we explain how to do it with the manifest tool.

1. Change the application, for example by changing some strings in `main.cpp`.
1. Compile the application:

    ```
    $ mbed compile -m YOUR_TARGET -t GCC_ARM
    ```

1. The manifest tool can both sign the update - using the private key generated earlier - and upload it to Mbed Cloud in a single command. Run:

    ```
    $ manifest-tool update device -p BUILD/YOUR_BOARD_NAME/GCC_ARM/simple-mbed-cloud-client-example_application.bin -D YOUR_ENDPOINT_NAME
    ```

    Replace `YOUR_BOARD_NAME` with the name of your development board, and replace `YOUR_ENDPOINT_NAME` with the endpoint name in Mbed Cloud.

1. Inspect the logs on the device to see the update progress. It looks similar to:

    ```
    Firmware download requested
    Authorization granted
    Downloading: [+++- ] 6 %
    ```

1. When the download completes, the firmware is verified. If everything is OK, the firmware update is applied.

## Known issues

Please check the issues reported on github.
